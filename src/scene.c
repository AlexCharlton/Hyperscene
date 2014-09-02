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

static void updateNode(HPGnode *node, HPGscene *scene, float x, float y, float z){//todo
    int i;
    float n[3];
    n[0] = x + node->x;
    n[1] = y + node->y;
    n[2] = z + node->z;
    if (node->needsUpdate){
        BoundingSphere *bs = node->partitionData.boundingSphere;
        bs->x = n[0];
        bs->y = n[1];
        bs->z = n[2];
        if ((HPGscene *) node->parent == scene){
            hpmAxisAngleRotation(&node->rx, node->angle, node->transform);//todo
            hpmTranslate(n, node->transform);
        } else {
            float trans[16];
            hpmAxisAngleRotation(&node->rx, node->angle, trans); //todo
            hpmTranslate(n, trans);
            hpmMultMat4(trans, node->parent->transform, node->transform);
        }
	scene->partitionInterface->updateNode(&node->partitionData);
        for (i = 0; i < node->children.size; i++){
            HPGnode *child = hpgVectorValue(&node->children, i);
            child->needsUpdate = true;
            updateNode(child, scene, n[0], n[1], n[2]);//todo
        }
        node->needsUpdate = false;
    } else {
        for (i = 0; i < node->children.size; i++)
            updateNode(hpgVectorValue(&node->children, i), scene, n[0], n[1], n[2]);//todo
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
    node->x = 0.0; node->y = 0.0; node->z = 0.0;
    node->rx = 0.0; node->ry = 0.0; node->rz = 1.0; node->angle = 0.0;
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
    freeNode(node, scene);
}

void hpgDeleteNode(HPGnode *node){
    deleteNode(node, getScene(node));
}

void hpgSetBoundingSphere(HPGnode *node, float radius){
    node->partitionData.boundingSphere->r = radius;
    node->needsUpdate = true;
}

void hpgMoveNode(HPGnode *node, float x, float y, float z){//todo
    node->x += x;
    node->y += y;
    node->z += z;
    node->needsUpdate = true;
}

void hpgSetNodePosition(HPGnode *node, float x, float y, float z){//todo
    node->x = x;
    node->y = y;
    node->z = z;
    node->needsUpdate = true;
}

void hpgSetNodeRotation(HPGnode *node, float x, float y, float z, float angle){//todo
    node->rx = x;
    node->ry = y;
    node->rz = z;
    node->angle = angle;
    node->needsUpdate = true;
}

void hpgRotateNode(HPGnode *node, float angle){ //todo
    node->angle = angle;
    node->needsUpdate = true;
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
    hpgPush(&activeScenes, (void *) scene);
    return scene;
}

void hpgDeleteScene(HPGscene *scene){
    int i;
    for (i = 0; i < scene->topLevelNodes.size; i++)
        freeNode(hpgVectorValue(&scene->topLevelNodes, i), scene);
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
        updateNode(hpgVectorValue(&scene->topLevelNodes, i), scene, 0.0, 0.0, 0.0);
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
