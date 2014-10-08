#include <stdlib.h>
#include <string.h>
#include "scene.h"

unsigned int hpsNodePoolSize = 4096, hpsBoundingSpherePoolSize = 4096, hpsTransformPoolSize = 4096, hpsPartitionPoolSize = 4096;

HPSpartitionInterface *hpsPartitionInterface;

static HPSvector activeScenes, freeScenes;

void hpsInit(HPSwindowSizeFun windowSizeFun){
    hpsInitCameras();
    hpsInitVector(&activeScenes, 16);
    hpsInitVector(&freeScenes, 16);
    hpsSetWindowSizeFun(windowSizeFun);
    hpsPartitionInterface = hpsAABBpartitionInterface;
}

/* Nodes */
static void freeNode(HPSnode *node, HPSscene *scene){
    int i;
    if (node->delete) node->delete(node->data);
    if (node->children.capacity){
	HPSvector *v = &node->children;
	for (i = 0; i < v->size; i++)
	    freeNode(v->data[i], scene);
	hpsDeleteVector(v);
    }
}

static void updateNode(HPSnode *node, HPSscene *scene){
    int i;
    if (node->needsUpdate){
        if ((HPSscene *) node->parent == scene){
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
            HPSnode *child = node->children.data[i];
            child->needsUpdate = true;
            updateNode(child, scene);
        }
        node->needsUpdate = false;
    } else {
        for (i = 0; i < node->children.size; i++)
            updateNode(node->children.data[i], scene);
    }
}

static void initBoundingSphere(BoundingSphere *bs){
    memset(bs, 0, 3 * sizeof(float));
    bs->r = 1;
}

static HPSscene *getScene(HPSnode *node){
    if (!node->parent)
        return (HPSscene *) node;
    return getScene(node->parent);
}

HPSnode *hpsAddNode(HPSnode *parent, void *data,
                    HPSpipeline *pipeline,
                    void (*deleteFunc)(void *)){
    HPSscene *scene = getScene(parent);
    HPSnode *node = hpsAllocateFrom(scene->nodePool);
    node->transform = hpsAllocateFrom(scene->transformPool);
    node->partitionData.data = node;
    node->partitionData.boundingSphere = hpsAllocateFrom(scene->boundingSpherePool);
    hpmIdentityMat4(node->transform);
    initBoundingSphere(node->partitionData.boundingSphere);
    node->position.x = 0.0; node->position.y = 0.0; node->position.z = 0.0;
    node->rotation.x = 0.0; node->rotation.y = 0.0; node->rotation.z = 0.0; 
    node->rotation.w = 1.0;
    node->data = data;
    node->pipeline = pipeline;
    node->parent = parent;
    node->delete = deleteFunc;
    node->needsUpdate = true;
    hpsInitVector(&node->children, 0);
    scene->partitionInterface->addNode(&node->partitionData, scene->partitionStruct);
    if ((HPSscene *) parent == scene)
        hpsPush(&scene->topLevelNodes, node);
    else
        hpsPush(&parent->children, node);
    return node;
}

static void deleteNode(HPSnode *node, HPSscene *scene){
    int i;
    scene->partitionInterface->removeNode(&node->partitionData);
    hpsDeleteFrom(node->partitionData.boundingSphere, scene->boundingSpherePool);
    hpsDeleteFrom(node->transform, scene->transformPool);
    for (i = 0; i < node->children.size; i++)
        deleteNode(node->children.data[i], scene);
    if ((HPSscene *) node->parent == scene)
        hpsRemove(&scene->topLevelNodes, node);
    else
        hpsRemove(&node->parent->children, node);
    freeNode(node, scene);
}

void hpsDeleteNode(HPSnode *node){
    deleteNode(node, getScene(node));
}

void hpsSetNodeBoundingSphere(HPSnode *node, float radius){
    node->partitionData.boundingSphere->r = radius;
    node->needsUpdate = true;
}

float *hpsNodeBoundingSphere(HPSnode *node){
    return (float *) node->partitionData.boundingSphere;
}

void hpsMoveNode(HPSnode *node, float *vec){
    node->position.x += vec[0];
    node->position.y += vec[1];
    node->position.z += vec[2];
    node->needsUpdate = true;
}

void hpsSetNodePosition(HPSnode *node, float *p){
    node->position.x = p[0];
    node->position.y = p[1];
    node->position.z = p[2];
    node->needsUpdate = true;
}

float* hpsNodeRotation(HPSnode *node){
    return (float *) &node->rotation;
}

float* hpsNodePosition(HPSnode *node){
    return (float *) &node->position;
}

float* hpsNodeTransform(HPSnode *node){
    return node->transform;
}

float* hpsNodeData(HPSnode *node){
    return node->data;
}

/* Scenes */
HPSscene *hpsMakeScene(){
    HPSscene *scene = (freeScenes.size) ?
	hpsPop(&freeScenes) : malloc(sizeof(HPSscene));
    scene->partitionInterface = hpsPartitionInterface;
    scene->nodePool = hpsMakePool(sizeof(HPSnode), hpsNodePoolSize, "Node pool");
    scene->transformPool = hpsMakePool(sizeof(float) * 16, hpsTransformPoolSize,
				       "Transform pool");
    scene->boundingSpherePool = hpsMakePool(sizeof(BoundingSphere),
					    hpsBoundingSpherePoolSize,
					    "Bounding sphere pool");
    scene->partitionPool = hpsMakePool(scene->partitionInterface->structSize,
                                       hpsPartitionPoolSize,
				      "Spatial partition pool");
    scene->partitionStruct = scene->partitionInterface->new(scene->partitionPool);
    scene->null = NULL;
    hpsInitVector(&scene->topLevelNodes, 1024);
    hpsInitVector(&scene->extensions, 4);
    hpsPush(&activeScenes, (void *) scene);
    return scene;
}

void hpsDeleteScene(HPSscene *scene){
    int i;
    for (i = 0; i < scene->topLevelNodes.size; i++)
        freeNode(scene->topLevelNodes.data[i], scene);
    hpsDeleteExtensions(scene);
    hpsClearPool(scene->nodePool);
    hpsClearPool(scene->transformPool);
    hpsClearPool(scene->boundingSpherePool);
    hpsRemove(&activeScenes, (void *) scene);
    hpsPush(&freeScenes, (void *) scene);
}

void hpsActivateScene(HPSscene *s){
    hpsRemove(&activeScenes, (void *) s);
    hpsPush(&activeScenes, (void *) s);
}

void hpsDeactivateScene(HPSscene *s){
    hpsRemove(&activeScenes, (void *) s);
}

static void hpsUpdateScene(HPSscene *scene){
    int i;
    for (i = 0; i < scene->topLevelNodes.size; i++)
        updateNode(scene->topLevelNodes.data[i], scene);
    hpsUpdateExtensions(scene);
}

void hpsUpdateScenes(){
    int i;
    for (i = 0; i < activeScenes.size; i++)
	hpsUpdateScene((HPSscene *) activeScenes.data[i]);
}



/* Pipelines */
HPSpipeline *hpsAddPipeline(void (*preRender)(void *),
			    void (*render)(void *),
			    void (*postRender)(),
                            bool isAlpha){
    HPSpipeline *pipeline = malloc(sizeof(HPSpipeline));
    pipeline->isAlpha = isAlpha;
    pipeline->preRender = preRender;
    pipeline->render = render;
    pipeline->postRender = postRender;
    return pipeline;
}

void hpsDeletePipeline(HPSpipeline *pipeline){
    free(pipeline);
}

/* Extensions */
// Add a node with the extension as the pipeline to have it passed to visibleNode when rendering

void hpsActivateExtension(HPSscene *scene, HPSextension *extension){
    hpsPush(&scene->extensions, (void *) extension);
    hpsPush(&scene->extensions, NULL);
    extension->init(&scene->extensions.data[scene->extensions.size-1]);
}

void *hpsExtensionData(HPSscene *scene, HPSextension *extension){
    int i;
    for (i = 0; i < scene->extensions.size; i += 2){
        HPSextension *e = (HPSextension *) scene->extensions.data[i];
        if (e == extension) return scene->extensions.data[i+1];
    }
}

void hpsPreRenderExtensions(HPSscene *scene){
    int i;
    for (i = 0; i < scene->extensions.size; i += 2){
        HPSextension *e = (HPSextension *) scene->extensions.data[i];
        e->preRender(scene->extensions.data[i+1]);
    }
}

void hpsPostRenderExtensions(HPSscene *scene){
    int i;
    for (i = 0; i < scene->extensions.size; i += 2){
        HPSextension *e = (HPSextension *) scene->extensions.data[i];
        e->postRender(scene->extensions.data[i+1]);
    }
}

void hpsVisibleNodeExtensions(HPSscene *scene, HPSnode *node){
    int i;
    for (i = 0; i < scene->extensions.size; i += 2){
        HPSextension *e = (HPSextension *) scene->extensions.data[i];
        if ((void*) node->pipeline == (void*) e)
            e->visibleNode(scene->extensions.data[i+1], node);
    }
}

void hpsUpdateExtensions(HPSscene *scene){
    int i;
    for (i = 0; i < scene->extensions.size; i += 2){
        HPSextension *e = (HPSextension *) scene->extensions.data[i];
        e->update(scene->extensions.data[i+1]);
    }
}

void hpsDeleteExtensions(HPSscene *scene){
    int i;
    for (i = 0; i < scene->extensions.size; i += 2){
        HPSextension *e = (HPSextension *) scene->extensions.data[i];
        e->delete(scene->extensions.data[i+1]);
    }
}
