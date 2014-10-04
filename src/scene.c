#include <stdlib.h>
#include <string.h>
#include "scene.h"

unsigned int hpgNodePoolSize = 4096, hpgBoundingSpherePoolSize = 4096, hpgTransformPoolSize = 4096, hpgPartitionPoolSize = 4096;

static HPGpool *pipelinePool;
static HPGvector activeScenes, freeScenes;

void hpgInitScenes(HPGwindowSizeFun windowSizeFun){
    hpgInitCameras();
    hpgInitVector(&activeScenes, 16);
    hpgInitVector(&freeScenes, 16);
    hpgSetWindowSizeFun(windowSizeFun);
    pipelinePool = hpgMakePool(sizeof(struct pipeline), 256, "Pipeline pool");
}

/* Nodes */
static void freeNode(HPGnode *node, HPGscene *scene){
    int i;
    node->delete(node->data);
    if (node->children.capacity){
	HPGvector *v = &node->children;
	for (i = 0; i < node->children.size; i++)
	    freeNode(hpgVectorValue(v, i), scene);
	hpgDeleteVector(v);
    }
}

static void updateNode(HPGnode *node, HPGscene *scene){
    int i;
    if (node->needsUpdate){
        if ((HPGscene *) node->parent == scene){
            hpmQuaternionRotation((float *) &node->rotation, node->transform);
            hpmTranslate((float *) &node->position, node->transform);
        } else {
            float trans[16];
            hpmQuaternionRotation((float *) &node->rotation, trans);
            hpmTranslate((float *) &node->position, trans);
            hpmMultMat4(trans, node->parent->transform, node->transform);
        }
        BoundingSphere *bs = node->partitionData.boundingSphere;
        bs->x = 0;
        bs->y = 0;
        bs->z = 0;
        hpmMat4VecMult(node->transform, (float*) bs);
	scene->partitionInterface->updateNode(&node->partitionData);
        for (i = 0; i < node->children.size; i++){
            HPGnode *child = hpgVectorValue(&node->children, i);
            child->needsUpdate = true;
            updateNode(child, scene);
        }
        node->needsUpdate = false;
    } else {
        for (i = 0; i < node->children.size; i++)
            updateNode(hpgVectorValue(&node->children, i), scene);
    }
}

static void initBoundingSphere(BoundingSphere *bs){
    memset(bs, 0, 3 * sizeof(float));
    bs->r = 1;
}

static HPGscene *getScene(HPGnode *node){
    if (!node->parent)
        return (HPGscene *) node;
    return getScene(node->parent);
}

HPGnode *hpgAddNode(HPGnode *parent, void *data,
                    HPGpipeline *pipeline,
                    void (*deleteFunc)(void *)){
    HPGscene *scene = getScene(parent);
    HPGnode *node = hpgAllocateFrom(scene->nodePool);
    node->transform = hpgAllocateFrom(scene->transformPool);
    node->partitionData.data = node;
    node->partitionData.boundingSphere = hpgAllocateFrom(scene->boundingSpherePool);
    hpmIdentityMat4(node->transform);
    initBoundingSphere(node->partitionData.boundingSphere);
    node->position.x = 0.0; node->position.y = 0.0; node->position.z = 0.0;
    node->rotation.x = 0.0; node->rotation.y = 0.0; node->rotation.z = 0.0; 
    node->rotation.w = 1.0;
    node->data = data;
    node->pipeline = pipeline;
    node->parent = parent;
    node->delete = (deleteFunc) ? deleteFunc : &free;;
    node->needsUpdate = true;
    hpgInitVector(&node->children, 0);
    scene->partitionInterface->addNode(&node->partitionData, scene->partitionStruct);
    if ((HPGscene *) parent == scene)
        hpgPush(&scene->topLevelNodes, node);
    else
        hpgPush(&parent->children, node);
    return node;
}

static void deleteNode(HPGnode *node, HPGscene *scene){
    int i;
    scene->partitionInterface->removeNode(&node->partitionData);
    hpgDeleteFrom(node->partitionData.boundingSphere, scene->boundingSpherePool);
    hpgDeleteFrom(node->transform, scene->transformPool);
    for (i = 0; i < node->children.size; i++)
        deleteNode(hpgVectorValue(&node->children, i), scene);
    if ((HPGscene *) node->parent == scene)
        hpgRemove(&scene->topLevelNodes, node);
    else
        hpgRemove(&node->parent->children, node);
    freeNode(node, scene);
}

void hpgDeleteNode(HPGnode *node){
    deleteNode(node, getScene(node));
}

void hpgSetNodeBoundingSphere(HPGnode *node, float radius){
    node->partitionData.boundingSphere->r = radius;
    node->needsUpdate = true;
}

float *hpgNodeBoundingSphere(HPGnode *node){
    return (float *) node->partitionData.boundingSphere;
}

void hpgMoveNode(HPGnode *node, float *vec){
    node->position.x += vec[0];
    node->position.y += vec[1];
    node->position.z += vec[2];
    node->needsUpdate = true;
}

void hpgSetNodePosition(HPGnode *node, float *p){
    node->position.x = p[0];
    node->position.y = p[1];
    node->position.z = p[2];
    node->needsUpdate = true;
}

float* hpgNodeRotation(HPGnode *node){
    return (float *) &node->rotation;
}

float* hpgNodePosition(HPGnode *node){
    return (float *) &node->position;
}

float* hpgNodeTransform(HPGnode *node){
    return node->transform;
}

float* hpgNodeData(HPGnode *node){
    return node->data;
}

/* Scenes */
HPGscene *hpgMakeScene(void *partitionInterface){
    HPGscene *scene = (freeScenes.size) ?
	hpgPop(&freeScenes) : malloc(sizeof(HPGscene));
    scene->partitionInterface = (PartitionInterface *) partitionInterface;
    scene->nodePool = hpgMakePool(sizeof(HPGnode), hpgNodePoolSize, "Node pool");
    scene->transformPool = hpgMakePool(sizeof(float) * 16, hpgTransformPoolSize,
				       "Transform pool");
    scene->boundingSpherePool = hpgMakePool(sizeof(BoundingSphere),
					    hpgBoundingSpherePoolSize,
					    "Bounding sphere pool");
    scene->partitionPool = hpgMakePool(scene->partitionInterface->structSize,
                                       hpgPartitionPoolSize,
				      "Spatial partition pool");
    scene->partitionStruct = scene->partitionInterface->new(scene->partitionPool);
    scene->null = NULL;
    hpgInitVector(&scene->topLevelNodes, 1024);
    hpgInitVector(&scene->extensions, 4);
    hpgPush(&activeScenes, (void *) scene);
    return scene;
}

void hpgDeleteScene(HPGscene *scene){
    int i;
    for (i = 0; i < scene->topLevelNodes.size; i++)
        freeNode(hpgVectorValue(&scene->topLevelNodes, i), scene);
    hpgDeleteExtensions(scene);
    hpgClearPool(scene->nodePool);
    hpgClearPool(scene->transformPool);
    hpgClearPool(scene->boundingSpherePool);
    hpgClearPool(scene->partitionPool);
    hpgRemove(&activeScenes, (void *) scene);
    hpgPush(&freeScenes, (void *) scene);
}

void hpgActiveateScene(HPGscene *s){
    hpgPush(&activeScenes, (void *) s);
}

void hpgDeactiveateScene(HPGscene *s){
    hpgRemove(&activeScenes, (void *) s);
}

static void hpgUpdateScene(HPGscene *scene){
    int i;
    for (i = 0; i < scene->topLevelNodes.size; i++)
        updateNode(hpgVectorValue(&scene->topLevelNodes, i), scene);
    hpgUpdateExtensions(scene);
}

void hpgUpdateScenes(){
    int i;
    for (i = 0; i < activeScenes.size; i++)
	hpgUpdateScene((HPGscene *) hpgVectorValue(&activeScenes, i));
}



/* Pipelines */
HPGpipeline *hpgAddPipeline(void (*preRender)(void *),
			    void (*render)(void *),
			    void (*postRender)(),
                            bool hasAlpha){
    HPGpipeline *pipeline = hpgAllocateFrom(pipelinePool);
    pipeline->hasAlpha = hasAlpha;
    pipeline->preRender = preRender;
    pipeline->render = render;
    pipeline->postRender = postRender;
    return pipeline;
}

void hpgPipelineAlpha(HPGpipeline *pipeline, bool hasAlpha){
    pipeline->hasAlpha = hasAlpha;
}

void hpgDeletePipeline(HPGpipeline *pipeline){
    hpgDeleteFrom(pipeline, pipelinePool);
}

/* Extensions */
// Add a node with the extension as the pipeline to have it passed to visibleNode when rendering

void hpgActivateExtension(HPGscene *scene, HPGextension *extension){
    hpgPush(&scene->extensions, (void *) extension);
    hpgPush(&scene->extensions, NULL);
    extension->init(&scene->extensions.data[scene->extensions.size-1]);
}

void *hpgExtensionData(HPGscene *scene, HPGextension *extension){
    int i;
    for (i = 0; i < scene->extensions.size; i += 2){
        HPGextension *e = (HPGextension *) scene->extensions.data[i];
        if (e == extension) return scene->extensions.data[i+1];
    }
}

void hpgPreRenderExtensions(HPGscene *scene){
    int i;
    for (i = 0; i < scene->extensions.size; i += 2){
        HPGextension *e = (HPGextension *) scene->extensions.data[i];
        e->preRender(scene->extensions.data[i+1]);
    }
}

void hpgPostRenderExtensions(HPGscene *scene){
    int i;
    for (i = 0; i < scene->extensions.size; i += 2){
        HPGextension *e = (HPGextension *) scene->extensions.data[i];
        e->postRender(scene->extensions.data[i+1]);
    }
}

void hpgVisibleNodeExtensions(HPGscene *scene, HPGnode *node){
    int i;
    for (i = 0; i < scene->extensions.size; i += 2){
        HPGextension *e = (HPGextension *) scene->extensions.data[i];
        if ((void*) node->pipeline == (void*) e)
            e->visibleNode(scene->extensions.data[i+1], node);
    }
}

void hpgUpdateExtensions(HPGscene *scene){
    int i;
    for (i = 0; i < scene->extensions.size; i += 2){
        HPGextension *e = (HPGextension *) scene->extensions.data[i];
        e->update(scene->extensions.data[i+1]);
    }
}

void hpgDeleteExtensions(HPGscene *scene){
    int i;
    for (i = 0; i < scene->extensions.size; i += 2){
        HPGextension *e = (HPGextension *) scene->extensions.data[i];
        e->delete(scene->extensions.data[i+1]);
    }
}
