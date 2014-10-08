#include <stdlib.h>
#include <hyperscene.h>
#include <hypersceneLighting.h>
#include "memory.h"

unsigned int hpsLightPoolSize = 1024;
static HPSvector lightQueue;
static bool initialized = false;

typedef struct {
    float r, g, b;
} Color;

typedef struct {
    float x, y, z;
} Point;

typedef struct {
    Color color;
    Point direction;
    float spotAngle;
    float intensity;
    HPSpool *pool;
} Light;

typedef struct {
    Color ambient;
    HPSpool *lightPool;
} SceneLighting;

unsigned int hpsMaxLights = 8;
unsigned int currentLights;
unsigned int *hpsNCurrentLights = &currentLights;
float *hpsCurrentLightPositions;
float *hpsCurrentLightColors;
float *hpsCurrentLightDirections;
float *hpsCurrentLightIntensities;

void hpsInitLighting(void **data){
    if (!initialized){
        hpsInitVector(&lightQueue, 16);
        hpsCurrentLightPositions = malloc(sizeof(float) * hpsMaxLights * 3);
        hpsCurrentLightDirections = malloc(sizeof(float) * hpsMaxLights * 3);
        hpsCurrentLightColors = malloc(sizeof(float) * hpsMaxLights * 3);
        hpsCurrentLightIntensities = malloc(sizeof(float) * hpsMaxLights);
        initialized = true;
    }
    SceneLighting *sLighting = malloc(sizeof(SceneLighting));
    sLighting->lightPool = hpsMakePool(sizeof(Light), hpsLightPoolSize, "Light pool");
    *data = sLighting;
}

void hpsDeleteLighting(void *data){
    SceneLighting *sLighting = (SceneLighting *) data;
    hpsDeletePool(sLighting->lightPool);
}

// TODO: Cache lights?
void hpsLightingPreRender(void *data){
    currentLights = lightQueue.size;
    currentLights = (currentLights > hpsMaxLights) ? hpsMaxLights : currentLights;
    int i;
    for (i = 0; i < currentLights; i++){
        HPSnode *node = (HPSnode *) lightQueue.data[i];
        Light *l = (Light *) hpsNodeData(node);
        float *bs = hpsNodeBoundingSphere(node);
        hpsCurrentLightIntensities[i] = l->intensity;
        hpsCurrentLightPositions[i*3]   = bs[0];
        hpsCurrentLightPositions[i*3+1] = bs[1];
        hpsCurrentLightPositions[i*3+2] = bs[2];
        hpsCurrentLightColors[i*3]   = l->color.r;
        hpsCurrentLightColors[i*3+1] = l->color.g;
        hpsCurrentLightColors[i*3+2] = l->color.b;
        hpsCurrentLightDirections[i*3]   = l->direction.x;
        hpsCurrentLightDirections[i*3+1] = l->direction.y;
        hpsCurrentLightDirections[i*3+2] = l->direction.z;
        hpsCurrentLightDirections[i*3+3] = l->spotAngle;
    }
}

void hpsLightingPostRender(void *data){
    lightQueue.size = 0;
}

void hpsLightingVisibleNode(void *data, HPSnode *node){
    hpsPush(&lightQueue, node);
}

void hpsLightingUpdate(void *data){
    // Nothing to be done
}

HPSextension lighting = {hpsInitLighting,
                         hpsLightingPreRender,
                         hpsLightingPostRender,
                         hpsLightingVisibleNode,
                         hpsLightingUpdate,
                         hpsDeleteLighting};

HPSextension *hpsLighting = &lighting;

void hpsDeleteLight(void *light){
    Light *l = (Light *) light;
    hpsDeleteFrom(l, l->pool);
}

HPSnode *hpsAddLight(HPSscene *scene, float* color, float i, float *direction, float spotAngle){
    SceneLighting *sLighting = (SceneLighting *) hpsExtensionData(scene, &lighting);
    Light *light = hpsAllocateFrom(sLighting->lightPool);
    light->pool = sLighting->lightPool;
    light->color.r = color[0];
    light->color.g = color[1];
    light->color.b = color[2];
    light->intensity = i;
    light->direction.x = direction[0];
    light->direction.y = direction[1];
    light->direction.z = direction[2];
    light->spotAngle = spotAngle;
    return hpsAddNode((HPSnode *) scene, (void *) light, 
                      (HPSpipeline *) &lighting, hpsDeleteLight);
}

void hpsSetLightColor(HPSnode *node, float* color){
    Light *l = (Light *) hpsNodeData(node);
    l->color.r = color[0];
    l->color.g = color[1];
    l->color.b = color[2];
}

float *hpsLightColor(HPSnode *node){
    Light *l = (Light *) hpsNodeData(node);
    return (float *) &l->color;
}

void hpsSetLightIntensity(HPSnode *node, float i){
    Light *l = (Light *) hpsNodeData(node);
    l->intensity = i;
}

float hpsLightIntensity(HPSnode *node){
    Light *l = (Light *) hpsNodeData(node);
    return l->intensity;
}

void hpsSetLightDirection(HPSnode *node, float* dir){
    Light *l = (Light *) hpsNodeData(node);
    l->direction.x = dir[0];
    l->direction.y = dir[1];
    l->direction.z = dir[2];
}
float *hpsLightDirection(HPSnode *node){
    Light *l = (Light *) hpsNodeData(node);
    return (float *) &l->direction;
}

void hpsSetLightSpotAngle(HPSnode *node, float a){
    Light *l = (Light *) hpsNodeData(node);
    l->spotAngle = a;
}

float hpsLightSpotAngle(HPSnode *node){
    Light *l = (Light *) hpsNodeData(node);
    return l->spotAngle;
}

void hpsSetAmbientLight(HPSscene *scene, float* color){
    SceneLighting *sLighting = (SceneLighting *) hpsExtensionData(scene, &lighting);
    sLighting->ambient.r = color[0];
    sLighting->ambient.g = color[1];
    sLighting->ambient.b = color[2];
}

float *hpsAmbientLight(HPSscene *scene){
    SceneLighting *sLighting = (SceneLighting *) hpsExtensionData(scene, &lighting);
    return (float *) &sLighting->ambient;
}
 
