#include <stdlib.h>
#include <hyperscene.h>
#include <hypersceneLighting.h>
#include "memory.h"

unsigned int hpgLightPoolSize = 1024;
static HPGvector lightQueue;
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
    HPGpool *pool;
} Light;

typedef struct {
    Color ambient;
    HPGpool *lightPool;
} SceneLighting;

unsigned int hpgMaxLights = 8;
unsigned int currentLights;
unsigned int *hpgNCurrentLights = &currentLights;
float *hpgCurrentLightPositions;
float *hpgCurrentLightColors;
float *hpgCurrentLightDirections;
float *hpgCurrentLightIntensities;

void hpgInitLighting(void **data){
    if (!initialized){
        hpgInitVector(&lightQueue, 16);
        hpgCurrentLightPositions = malloc(sizeof(float) * hpgMaxLights * 3);
        hpgCurrentLightDirections = malloc(sizeof(float) * hpgMaxLights * 3);
        hpgCurrentLightColors = malloc(sizeof(float) * hpgMaxLights * 3);
        hpgCurrentLightIntensities = malloc(sizeof(float) * hpgMaxLights);
        initialized = true;
    }
    SceneLighting *sLighting = malloc(sizeof(SceneLighting));
    sLighting->lightPool = hpgMakePool(sizeof(Light), hpgLightPoolSize, "Light pool");
    *data = sLighting;
}

void hpgDeleteLighting(void *data){
    SceneLighting *sLighting = (SceneLighting *) data;
    hpgDeletePool(sLighting->lightPool);
}

// TODO: Cache lights?
void hpgLightingPreRender(void *data){
    currentLights = lightQueue.size;
    int i;
    for (i = 0; i < currentLights; i++){
        HPGnode *node = (HPGnode *) lightQueue.data[i];
        Light *l = (Light *) hpgNodeData(node);
        float *bs = hpgNodeBoundingSphere(node);
        hpgCurrentLightIntensities[i] = l->intensity;
        hpgCurrentLightPositions[i*3]   = bs[0];
        hpgCurrentLightPositions[i*3+1] = bs[1];
        hpgCurrentLightPositions[i*3+2] = bs[2];
        hpgCurrentLightColors[i*3]   = l->color.r;
        hpgCurrentLightColors[i*3+1] = l->color.g;
        hpgCurrentLightColors[i*3+2] = l->color.b;
        hpgCurrentLightDirections[i*3]   = l->direction.x;
        hpgCurrentLightDirections[i*3+1] = l->direction.y;
        hpgCurrentLightDirections[i*3+2] = l->direction.z;
        hpgCurrentLightDirections[i*3+3] = l->spotAngle;
    }
}

void hpgLightingPostRender(void *data){
    lightQueue.size = 0;
}

void hpgLightingVisibleNode(void *data, HPGnode *node){
    hpgPush(&lightQueue, node);
}

void hpgLightingUpdate(void *data){
    // Nothing to be done
}

HPGextension lighting = {hpgInitLighting,
                         hpgLightingPreRender,
                         hpgLightingPostRender,
                         hpgLightingVisibleNode,
                         hpgLightingUpdate,
                         hpgDeleteLighting};

HPGextension *hpgLighting = &lighting;

void hpgDeleteLight(void *light){
    Light *l = (Light *) light;
    hpgDeleteFrom(l, l->pool);
}

HPGnode *hpgAddLight(HPGscene *scene, float* color, float i, float *direction, float spotAngle){
    SceneLighting *sLighting = (SceneLighting *) hpgExtensionData(scene, &lighting);
    Light *light = hpgAllocateFrom(sLighting->lightPool);
    light->pool = sLighting->lightPool;
    light->color.r = color[0];
    light->color.g = color[1];
    light->color.b = color[2];
    light->intensity = i;
    light->direction.x = direction[0];
    light->direction.y = direction[1];
    light->direction.z = direction[2];
    light->spotAngle = spotAngle;
    return hpgAddNode((HPGnode *) scene, (void *) light, 
                      (HPGpipeline *) &lighting, hpgDeleteLight);
}

void hpgSetLightColor(HPGnode *node, float* color){
    Light *l = (Light *) hpgNodeData(node);
    l->color.r = color[0];
    l->color.g = color[1];
    l->color.b = color[2];
}

void hpgSetLightIntensity(HPGnode *node, float i){
    Light *l = (Light *) hpgNodeData(node);
    l->intensity = i;
}

void hpgSetLightDirection(HPGnode *node, float* dir){
    Light *l = (Light *) hpgNodeData(node);
    l->direction.x = dir[0];
    l->direction.y = dir[1];
    l->direction.z = dir[2];
}

void hpgSetLightSpotAngle(HPGnode *node, float a){
    Light *l = (Light *) hpgNodeData(node);
    l->spotAngle = a;
}

void hpgSetAmbientLight(HPGscene *scene, float* color){
    SceneLighting *sLighting = (SceneLighting *) hpgExtensionData(scene, &lighting);
    sLighting->ambient.r = color[0];
    sLighting->ambient.g = color[1];
    sLighting->ambient.b = color[2];
}

float *hpgAmbientLight(HPGscene *scene){
    SceneLighting *sLighting = (SceneLighting *) hpgExtensionData(scene, &lighting);
    return (float *) &sLighting->ambient;
}
 
