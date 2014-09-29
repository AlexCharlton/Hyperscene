#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <float.h>
#include "scene.h"
#define HALF_PI 1.57079631

// TODO cameras render to different viewport areas

typedef enum {
    RIGHT, LEFT, TOP, BOTTOM, NEAR, FAR
} Faces;

static HPGvector cameraList, activeCameras, renderQueue, alphaQueue;
static HPGwindowSizeFun windowSizefun;

static HPGcamera currentCamera;

float *hpgCurrentCameraPosition(){
    return (float *) &currentCamera.position;
}

float *hpgCurrentCameraProjection(){
    return currentCamera.projection;
}

float *hpgCurrentCameraView(){
    return currentCamera.view;
}

float *hpgCurrentCameraViewProjection(){
    return currentCamera.viewProjection;
}

float *hpgCurrentCameraModelViewProjection(){
    return currentCamera.modelViewProjection;
}

void hpgSetWindowSizeFun(HPGwindowSizeFun fun){ windowSizefun = fun; }

static void addToQueue(Node *node){
    HPGnode *n = (HPGnode *) node->data;
    if(!n->pipeline) return;
    if (n->pipeline->hasAlpha)
	hpgPush(&alphaQueue, n);
    else
	hpgPush(&renderQueue, n);
}

static void renderNode(HPGnode *node, HPGcamera *camera){
    hpmMultMat4(camera->viewProjection, node->transform, 
                camera->modelViewProjection);
    node->pipeline->render(node->data);
}

static void clearQueues(){
    renderQueue.size = 0;
    alphaQueue.size = 0;
}

static int xNegative(const void *a, const void *b){
    BoundingSphere *ba = ((HPGnode *) a)->partitionData.boundingSphere;
    BoundingSphere *bb = ((HPGnode *) b)->partitionData.boundingSphere;
    float xa = ba->x + ba->r;
    float xb = bb->x + bb->r;
    if (xa < xb) return 1;
    else if (xa > xb) return -1;
    return 0;
}
static int xPositive(const void *a, const void *b){
    BoundingSphere *ba = ((HPGnode *) a)->partitionData.boundingSphere;
    BoundingSphere *bb = ((HPGnode *) b)->partitionData.boundingSphere;
    float xa = ba->x + ba->r;
    float xb = bb->x + bb->r;
    if (xa > xb) return 1;
    else if (xa < xb) return -1;
    return 0;
}
static int yNegative(const void *a, const void *b){
    BoundingSphere *ba = ((HPGnode *) a)->partitionData.boundingSphere;
    BoundingSphere *bb = ((HPGnode *) b)->partitionData.boundingSphere;
    float ya = ba->y + ba->r;
    float yb = bb->y + bb->r;
    if (ya < yb) return 1;
    else if (ya > yb) return -1;
    return 0;
}
static int yPositive(const void *a, const void *b){
    BoundingSphere *ba = ((HPGnode *) a)->partitionData.boundingSphere;
    BoundingSphere *bb = ((HPGnode *) b)->partitionData.boundingSphere;
    float ya = ba->y + ba->r;
    float yb = bb->y + bb->r;
    if (ya > yb) return 1;
    else if (ya < yb) return -1;
    return 0;
}
static int zNegative(const void *a, const void *b){
    BoundingSphere *ba = ((HPGnode *) a)->partitionData.boundingSphere;
    BoundingSphere *bb = ((HPGnode *) b)->partitionData.boundingSphere;
    float za = ba->z + ba->r;
    float zb = bb->z + bb->r;
    if (za < zb) return 1;
    else if (za > zb) return -1;
    return 0;
}
static int zPositive(const void *a, const void *b){
    BoundingSphere *ba = ((HPGnode *) a)->partitionData.boundingSphere;
    BoundingSphere *bb = ((HPGnode *) b)->partitionData.boundingSphere;
    float za = ba->z + ba->r;
    float zb = bb->z + bb->r;
    if (za > zb) return 1;
    else if (za < zb) return -1;
    return 0;
}

static int xGreaterThan(const void *a, const void *b){
    float xa = ((HPGnode *) a)->partitionData.boundingSphere->x;
    float xb = ((HPGnode *) b)->partitionData.boundingSphere->x;
    if (xa < xb) return 1;
    else if (xa > xb) return -1;
    return 0;
}
static int xLessThan(const void *a, const void *b){
    float xa = ((HPGnode *) a)->partitionData.boundingSphere->x;
    float xb = ((HPGnode *) b)->partitionData.boundingSphere->x;
    if (xa > xb) return 1;
    else if (xa < xb) return -1;
    return 0;
}
static int yGreaterThan(const void *a, const void *b){
    float ya = ((HPGnode *) a)->partitionData.boundingSphere->y;
    float yb = ((HPGnode *) b)->partitionData.boundingSphere->y;
    if (ya < yb) return 1;
    else if (ya > yb) return -1;
    return 0;
}
static int yLessThan(const void *a, const void *b){
    float ya = ((HPGnode *) a)->partitionData.boundingSphere->y;
    float yb = ((HPGnode *) b)->partitionData.boundingSphere->y;
    if (ya > yb) return 1;
    else if (ya < yb) return -1;
    return 0;
}
static int zGreaterThan(const void *a, const void *b){
    float za = ((HPGnode *) a)->partitionData.boundingSphere->z;
    float zb = ((HPGnode *) b)->partitionData.boundingSphere->z;
    if (za < zb) return 1;
    else if (za > zb) return -1;
    return 0;
}
static int zLessThan(const void *a, const void *b){
    float za = ((HPGnode *) a)->partitionData.boundingSphere->z;
    float zb = ((HPGnode *) b)->partitionData.boundingSphere->z;
    if (za > zb) return 1;
    else if (za < zb) return -1;
    return 0;
}

static int programSort(const void *a, const void *b){
    unsigned int pa = (int) (((HPGnode *) a)->pipeline);
    unsigned int pb = (int) (((HPGnode *) b)->pipeline);
    if (pa < pb) return -1;
    else if (pa > pb) return 1;
    return 0;
}

// TODO testing!
static void setSortFuns(Plane *plane, 
                        int (**alphaSort)(const void*, const void*),
                        int (**renderSort)(const void*, const void*)){
    float aa, ab, ac;
    aa = abs(plane->a); 
    ab = abs(plane->b); 
    ac = abs(plane->c); 

    if ((aa > ab) && (aa > ac)){
        if (plane->a < 0.0) { *alphaSort = &xLessThan; *renderSort = &xNegative; } 
        else                { *alphaSort = &xGreaterThan; *renderSort = &xPositive; }
    } else if ((ab > ac)){
        if (plane->b < 0.0) { *alphaSort = &yLessThan; *renderSort = &yNegative; } 
        else                { *alphaSort = &yGreaterThan; *renderSort = &yPositive; }
    } else {
        if (plane->c < 0.0) { *alphaSort = &zLessThan; *renderSort = &zNegative; } 
        else                { *alphaSort = &zGreaterThan; *renderSort = &zPositive; }
    }
}

static void renderQueues(HPGcamera *camera){
    int i, j, count;
    struct pipeline *p = NULL;
    int (*alphaSort)(const void*, const void*) = NULL;
    int (*renderSort)(const void*, const void*) = NULL;
    setSortFuns(&camera->planes[NEAR], &alphaSort, &renderSort);
    qsort(renderQueue.data, renderQueue.size, sizeof(void *), &programSort);
    HPGnode **nodes = (HPGnode **) &renderQueue.data[0];
    for (i = 0; i < renderQueue.size;){
        HPGnode *n = *nodes;
        p = n->pipeline;
        p->preRender(n->data);
#ifdef NO_REVERSE_PAINTER
        renderNode(n, camera);
        for (count = 1; count < (renderQueue.size - i);){
            HPGnode *m = *(++nodes);
            if (m->pipeline == p){
                renderNode(m, camera);
                count++;
            } else break;
        }
#else
        HPGnode **m = nodes + 1;
        for (count = 1; count < (renderQueue.size - i);){
            if ((*m++)->pipeline == p){
                count++;
            } else {
                qsort(nodes, count, sizeof(void *), renderSort);
                break;
            }
        }
        for (j = 0; j < count; j++){
            renderNode(*nodes++, camera);
        }
#endif
        p->postRender();
        i += count;
    }
    qsort(alphaQueue.data, alphaQueue.size, sizeof(void *), alphaSort);
    nodes = (HPGnode **) &alphaQueue.data[0];
    for (i = 0; i < alphaQueue.size;){
        HPGnode *n = *nodes;
        p = n->pipeline;
        p->preRender(n->data);
        renderNode(n, camera);
        for (count = 1; count < (alphaQueue.size - i);){
            HPGnode *m = *(++nodes);
            if (m->pipeline == p){
                renderNode(m, camera);
                count++;
            } else break;
        }
        p->postRender();
        i += count;
    }
}

/*
http://web.archive.org/web/20120531231005/http://crazyjoke.free.fr/doc/3D/plane%20extraction.pdf
*/
/* Normals (pointing in) are (a, b, c) */
static void computePlanes(HPGcamera *camera){
    Plane *ps = camera->planes;
    HPMmat4 *m = (HPMmat4 *) camera->viewProjection;
    ps[LEFT].a   = m->_41 + m->_11; ps[LEFT].b   = m->_42 + m->_12;
    ps[LEFT].c   = m->_43 + m->_13; ps[LEFT].d   = m->_44 + m->_14;
    ps[RIGHT].a  = m->_41 - m->_11; ps[RIGHT].b  = m->_42 - m->_12;
    ps[RIGHT].c  = m->_43 - m->_13; ps[RIGHT].d  = m->_44 - m->_14;
    ps[BOTTOM].a = m->_41 + m->_21; ps[BOTTOM].b = m->_42 + m->_22;
    ps[BOTTOM].c = m->_43 + m->_23; ps[BOTTOM].d = m->_44 + m->_24;
    ps[TOP].a    = m->_41 - m->_21; ps[TOP].b    = m->_42 - m->_22;
    ps[TOP].c    = m->_43 - m->_23; ps[TOP].d    = m->_44 - m->_24;
    ps[NEAR].a   = m->_41 + m->_31; ps[NEAR].b   = m->_42 + m->_32;
    ps[NEAR].c   = m->_43 + m->_33; ps[NEAR].d   = m->_44 + m->_34;
    ps[FAR].a    = m->_41 - m->_31; ps[FAR].b    = m->_42 - m->_32;
    ps[FAR].c    = m->_43 - m->_33; ps[FAR].d    = m->_44 - m->_34;
}

void hpgRenderCamera(HPGcamera *camera){
    currentCamera = *camera; // Set current camera to this one
    HPGcamera *c = &currentCamera;
    float cameraMat[16];
    clearQueues();
    switch (camera->style){
    case ORBIT:
    {
        float cosTilt = cos(c->rotation.y);
        float sinTilt = sin(c->rotation.y);
        float sinPan = sin(c->rotation.x);
        float cosPan = cos(c->rotation.x);
        c->position.x = c->object.x + c->rotation.w * cosTilt * sinPan;
        c->position.y = c->object.y + c->rotation.w * sinTilt;
        c->position.z = c->object.z + c->rotation.w * cosTilt * cosPan;
        hpmYPRRotation(c->rotation.x, -c->rotation.y, c->rotation.z, cameraMat);
        hpmTranslate((float *) &c->position, cameraMat);
        hpmCameraInverse(cameraMat, c->view);
        break;
    }
    case LOOK_AT:
        hpmLookAt((float *) &c->position, (float *) &c->object, (float *) &c->up, 
                  c->view);
        break;
    case POSITION:
        hpmQuaternionRotation((float *) &c->rotation, cameraMat);
        hpmTranslate((float *) &c->position, cameraMat);
        hpmCameraInverse(cameraMat, c->view);
        break;
    case FIRST_PERSON:
        hpmYPRRotation(-c->rotation.x, c->rotation.y, c->rotation.z, cameraMat);
        hpmTranslate((float *) &c->position, cameraMat);
        hpmCameraInverse(cameraMat, c->view);
        break;
    default:
        fprintf(stderr, "Camera does not have a valid style\n");
        exit(EXIT_FAILURE);
    }
    hpmMultMat4(c->projection, c->view, c->viewProjection);
    computePlanes(c);
    camera->scene->partitionInterface->doVisible(c->scene->partitionStruct,
                                                 c->planes, &addToQueue);
    renderQueues(c);
    *camera = currentCamera; // Copy currentCamera back into camera
}

static void hpgOrthoCamera(int width, int height, HPGcamera *camera){
    hpmOrtho(width, height, camera->n, camera->f, camera->projection);
}

static void hpgPerspectiveCamera(int width, int height, HPGcamera *camera){
    hpmPerspective(width, height, camera->n, camera->f, camera->viewAngle,
		   camera->projection);
}

HPGcamera *hpgMakeCamera(HPGcameraType type, HPGcameraStyle style, HPGscene *scene){
    HPGcamera *camera = malloc(sizeof(struct camera));
    camera->n = HPG_DEFAULT_NEAR_PLANE;
    camera->f = HPG_DEFAULT_FAR_PLANE;
    camera->viewAngle = HPG_DEFAULT_VIEW_ANGLE;
    camera->position.x = 0.0;
    camera->position.y = 0.0;
    camera->position.z = 0.0;
    camera->up.x = 0.0;
    camera->up.y = 1.0;
    camera->up.z = 0.0;
    camera->object.x = 0.0;
    camera->object.y = 0.0;
    camera->object.z = 0.0;
    camera->rotation.x = 0;
    camera->rotation.y = 0;
    camera->rotation.z = 0;
    camera->rotation.w = 1;
    if (type == HPG_ORTHO)
        camera->update = &hpgOrthoCamera;
    else
        camera->update = &hpgPerspectiveCamera;
    camera->style = style;
    camera->scene = scene;
    hpgPush(&cameraList, (void *) camera);
    hpgPush(&activeCameras, (void *) camera);
    int w, h;
    windowSizefun(&w, &h);
    camera->update(w, h, camera);
    return camera;
}

void hpgSetCameraClipPlanes(HPGcamera *camera, float near, float far){
    camera->n = near;
    camera->f = far;
    int w, h;
    windowSizefun(&w, &h);
    camera->update(w, h, camera);
}

void hpgSetCameraViewAngle(HPGcamera *camera, float angle){
    camera->viewAngle = angle;
    int w, h;
    windowSizefun(&w, &h);
    camera->update(w, h, camera);
}

void hpgDeleteCamera(HPGcamera *camera){
    hpgRemove(&cameraList, (void *) camera);
    free(camera);
}

void hpgMoveCamera(HPGcamera *camera, float *vec){
    if (camera->style == ORBIT){
        fprintf(stderr, "Can't move an orbit camera\n");
        return;
    }
    camera->position.x += vec[0];
    camera->position.y += vec[1];
    camera->position.z += vec[2];
}

void hpgSetCameraPosition(HPGcamera *camera, float *vec){
    if (camera->style == ORBIT){
        fprintf(stderr, "Can't move an ORBIT camera\n");
        return;
    }
    camera->position.x = vec[0];
    camera->position.y = vec[1];
    camera->position.z = vec[2];
}

float *hpgCameraRotation(HPGcamera *camera){
    if (camera->style != POSITION){
        fprintf(stderr, "Can't rotation a non POSITION camera\n");
        return NULL;
    }
    return (float *) &camera->rotation;
}

void hpgSetCameraUp(HPGcamera *camera, float *up){
    if (camera->style != LOOK_AT){
        fprintf(stderr, "Can't set up on a non LOOK_AT camera\n");
        return;
    }
    camera->up.x = up[0];
    camera->up.y = up[1];
    camera->up.z = up[2];
}

void hpgCameraLookAt(HPGcamera *camera, float *p){
    if ((camera->style != ORBIT) && (camera->style != LOOK_AT)) {
        fprintf(stderr, "Can't set object to look at for a non LOOK_AT or ORBIT camera\n");
        return;
    }
    camera->object.x = p[0];
    camera->object.y = p[1];
    camera->object.z = p[2];
}

void hpgPanCamera(HPGcamera *camera, float angle){
    if ((camera->style != ORBIT) && (camera->style != FIRST_PERSON)) {
        fprintf(stderr, "Can't pan a non ORBIT or FIRST_PERSON camera\n");
        return;
    }
    camera->rotation.x += angle;
}

void hpgSetCameraPan(HPGcamera *camera, float angle){
    if ((camera->style != ORBIT) && (camera->style != FIRST_PERSON)) {
        fprintf(stderr, "Can't pan a non ORBIT or FIRST_PERSON camera\n");
        return;
    }
    camera->style = ORBIT;
    camera->rotation.x = angle;
}

void hpgTiltCamera(HPGcamera *camera, float angle){
    if ((camera->style != ORBIT) && (camera->style != FIRST_PERSON)) {
        fprintf(stderr, "Can't tilt a non ORBIT or FIRST_PERSON camera\n");
        return;
    }
    camera->rotation.y += angle;
    camera->rotation.y = fmin(HALF_PI, fmax(-HALF_PI, camera->rotation.y));
}

void hpgSetCameraTilt(HPGcamera *camera, float angle){
    if ((camera->style != ORBIT) && (camera->style != FIRST_PERSON)) {
        fprintf(stderr, "Can't tilt a non ORBIT or FIRST_PERSON camera\n");
        return;
    }
    camera->rotation.y = fmin(HALF_PI, fmax(-HALF_PI, angle));
}

void hpgZoomCamera(HPGcamera *camera, float distance){
    if (camera->style != ORBIT){
        fprintf(stderr, "Can't zoom a non ORBIT camera\n");
        return;
    }
    camera->rotation.w += distance;
    camera->rotation.w = (camera->rotation.w < FLT_MIN) ? FLT_MIN : camera->rotation.w;
}

void hpgSetCameraZoom(HPGcamera *camera, float distance){
    if (camera->style != ORBIT){
        fprintf(stderr, "Can't zoom a non ORBIT camera\n");
        return;
    }
    camera->rotation.w = (distance < 0.0) ? 0.0 : distance;
}

void hpgRollCamera(HPGcamera *camera, float angle){
    if ((camera->style != ORBIT) && (camera->style != FIRST_PERSON)) {
        fprintf(stderr, "Can't roll a non ORBIT or FIRST_PERSON camera\n");
        return;
    }
    camera->rotation.z += angle;
}

void hpgSetCameraRoll(HPGcamera *camera, float angle){
    if ((camera->style != ORBIT) && (camera->style != LOOK_AT)) {
        fprintf(stderr, "Can't roll a non ORBIT or LOOK_AT camera\n");
        return;
    }
    camera->rotation.z = angle;
}

void hpgResizeCameras(int width, int height){
    int i;
    for (i = 0; i < cameraList.size; i++){
	HPGcamera *camera = (HPGcamera *) hpgVectorValue(&cameraList, i);
	camera->update(width, height, camera);
    }
}

void hpgMoveCameraForward(HPGcamera *camera, float dist){
    if ((camera->style != FIRST_PERSON)) {
        fprintf(stderr, "Can't move a non FIRST_PERSON camera forward\n");
        return;
    }
    float sinPan = sin(camera->rotation.x);
    float cosPan = cos(camera->rotation.x);
    camera->position.x += dist * sinPan;
    camera->position.z -= dist * cosPan;
}

void hpgMoveCameraUp(HPGcamera *camera, float dist){
    if ((camera->style != FIRST_PERSON)) {
        fprintf(stderr, "Can't move a non FIRST_PERSON camera up\n");
        return;
    }
    float sinPan = sin(camera->rotation.x);
    float cosPan = cos(camera->rotation.x);
    camera->position.y += dist;
}

void hpgStrafeCamera(HPGcamera *camera, float dist){
    if ((camera->style != FIRST_PERSON)) {
        fprintf(stderr, "Can't strafe a non FIRST_PERSON camera\n");
        return;
    }
    float sinPan = sin(camera->rotation.x);
    float cosPan = cos(camera->rotation.x);
    camera->position.x += dist * cosPan;
    camera->position.z += dist * sinPan;
}


void hpgRenderCameras(){
    int i;
    for (i = 0; i < activeCameras.size; i++)
	hpgRenderCamera((HPGcamera *) hpgVectorValue(&activeCameras, i));
}

void hpgActiveateCamera(HPGcamera *c){
    hpgPush(&activeCameras, (void *) c);
}

void hpgDeactiveateCamera(HPGcamera *c){
    hpgRemove(&activeCameras, (void *) c);
}

void hpgInitCameras(){
    hpgInitVector(&cameraList, 16);
    hpgInitVector(&activeCameras, 16);
    hpgInitVector(&renderQueue, 4096);
    hpgInitVector(&alphaQueue, 1024);
}
