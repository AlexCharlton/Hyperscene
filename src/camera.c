#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <float.h>
#include "scene.h"
#define HALF_PI 1.57079631

// TODO cameras render to different viewport areas

typedef enum {
    RIGHT, LEFT, TOP, BOTTOM, NEAR, FAR
} Faces;

static HPSvector cameraList, activeCameras, renderQueue, alphaQueue;
static HPSwindowSizeFun windowSizefun;

static HPScamera currentCamera;
static float currentInverseTransposeModel[16];

float *hpsCurrentCameraPosition = (float *) &currentCamera.position;

float *hpsCurrentCameraProjection = currentCamera.projection;

float *hpsCurrentCameraView = currentCamera.view;

float *hpsCurrentCameraViewProjection = currentCamera.viewProjection;

float *hpsCurrentCameraModelViewProjection = currentCamera.modelViewProjection;

float *hpsCurrentInverseTransposeModel = currentInverseTransposeModel;

void hpsSetWindowSizeFun(HPSwindowSizeFun fun){ windowSizefun = fun; }

static void addToQueue(Node *node){
    HPSnode *n = (HPSnode *) node->data;
    if (!n->pipeline) return;
    if (n->pipeline->isAlpha == 1){
	hpsPush(&alphaQueue, n);
    } else if (n->pipeline->isAlpha == 0) {
	hpsPush(&renderQueue, n);
    } else {
        hpsVisibleNodeExtensions(currentCamera.scene, node->data);
    }
}

static void renderNode(HPSnode *node, HPScamera *camera){
    hpmMultMat4(camera->viewProjection, node->transform, 
                camera->modelViewProjection);
#ifndef NO_INVERSE_TRANSPOSE
    hpmFastInverseTranspose(node->transform, currentInverseTransposeModel);
#endif
    node->pipeline->render(node->data);
}

static void clearQueues(){
    renderQueue.size = 0;
    alphaQueue.size = 0;
}

static int xNegative(const void *a, const void *b){
    BoundingSphere *ba = ((HPSnode *) a)->partitionData.boundingSphere;
    BoundingSphere *bb = ((HPSnode *) b)->partitionData.boundingSphere;
    float xa = ba->x + ba->r;
    float xb = bb->x + bb->r;
    if (xa < xb) return 1;
    else if (xa > xb) return -1;
    return 0;
}
static int xPositive(const void *a, const void *b){
    BoundingSphere *ba = ((HPSnode *) a)->partitionData.boundingSphere;
    BoundingSphere *bb = ((HPSnode *) b)->partitionData.boundingSphere;
    float xa = ba->x + ba->r;
    float xb = bb->x + bb->r;
    if (xa > xb) return 1;
    else if (xa < xb) return -1;
    return 0;
}
static int yNegative(const void *a, const void *b){
    BoundingSphere *ba = ((HPSnode *) a)->partitionData.boundingSphere;
    BoundingSphere *bb = ((HPSnode *) b)->partitionData.boundingSphere;
    float ya = ba->y + ba->r;
    float yb = bb->y + bb->r;
    if (ya < yb) return 1;
    else if (ya > yb) return -1;
    return 0;
}
static int yPositive(const void *a, const void *b){
    BoundingSphere *ba = ((HPSnode *) a)->partitionData.boundingSphere;
    BoundingSphere *bb = ((HPSnode *) b)->partitionData.boundingSphere;
    float ya = ba->y + ba->r;
    float yb = bb->y + bb->r;
    if (ya > yb) return 1;
    else if (ya < yb) return -1;
    return 0;
}
static int zNegative(const void *a, const void *b){
    BoundingSphere *ba = ((HPSnode *) a)->partitionData.boundingSphere;
    BoundingSphere *bb = ((HPSnode *) b)->partitionData.boundingSphere;
    float za = ba->z + ba->r;
    float zb = bb->z + bb->r;
    if (za < zb) return 1;
    else if (za > zb) return -1;
    return 0;
}
static int zPositive(const void *a, const void *b){
    BoundingSphere *ba = ((HPSnode *) a)->partitionData.boundingSphere;
    BoundingSphere *bb = ((HPSnode *) b)->partitionData.boundingSphere;
    float za = ba->z + ba->r;
    float zb = bb->z + bb->r;
    if (za > zb) return 1;
    else if (za < zb) return -1;
    return 0;
}

static int xGreaterThan(const void *a, const void *b){
    float xa = ((HPSnode *) a)->partitionData.boundingSphere->x;
    float xb = ((HPSnode *) b)->partitionData.boundingSphere->x;
    if (xa < xb) return 1;
    else if (xa > xb) return -1;
    return 0;
}
static int xLessThan(const void *a, const void *b){
    float xa = ((HPSnode *) a)->partitionData.boundingSphere->x;
    float xb = ((HPSnode *) b)->partitionData.boundingSphere->x;
    if (xa > xb) return 1;
    else if (xa < xb) return -1;
    return 0;
}
static int yGreaterThan(const void *a, const void *b){
    float ya = ((HPSnode *) a)->partitionData.boundingSphere->y;
    float yb = ((HPSnode *) b)->partitionData.boundingSphere->y;
    if (ya < yb) return 1;
    else if (ya > yb) return -1;
    return 0;
}
static int yLessThan(const void *a, const void *b){
    float ya = ((HPSnode *) a)->partitionData.boundingSphere->y;
    float yb = ((HPSnode *) b)->partitionData.boundingSphere->y;
    if (ya > yb) return 1;
    else if (ya < yb) return -1;
    return 0;
}
static int zGreaterThan(const void *a, const void *b){
    float za = ((HPSnode *) a)->partitionData.boundingSphere->z;
    float zb = ((HPSnode *) b)->partitionData.boundingSphere->z;
    if (za < zb) return 1;
    else if (za > zb) return -1;
    return 0;
}
static int zLessThan(const void *a, const void *b){
    float za = ((HPSnode *) a)->partitionData.boundingSphere->z;
    float zb = ((HPSnode *) b)->partitionData.boundingSphere->z;
    if (za > zb) return 1;
    else if (za < zb) return -1;
    return 0;
}

static int programSort(const void *a, const void *b){
    HPSpipeline *pa = ((HPSnode *) a)->pipeline;
    HPSpipeline *pb = ((HPSnode *) b)->pipeline;
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

#ifdef DEBUG
int nonAlphaObjects = 0;
int alphaObjects = 0;
#endif 

static void renderQueues(HPScamera *camera){
#ifdef DEBUG
    int nonAlpha = renderQueue.size;
    int alpha = alphaQueue.size;
    if ((alpha != alphaObjects) || (nonAlpha != nonAlphaObjects)){
        printf("Rendering %d non-alpha objects, %d alpha objects\n", nonAlpha, alpha);
        nonAlphaObjects = nonAlpha;
        alphaObjects = alpha;
    }
#endif 
    int i, j, count;
    struct pipeline *p = NULL;
    int (*alphaSort)(const void*, const void*) = NULL;
    int (*renderSort)(const void*, const void*) = NULL;
    setSortFuns(&camera->planes[NEAR], &alphaSort, &renderSort);
    qsort(renderQueue.data, renderQueue.size, sizeof(void *), &programSort);
    HPSnode **nodes = (HPSnode **) &renderQueue.data[0];
    for (i = 0; i < renderQueue.size;){
        HPSnode *n = *nodes;
        p = n->pipeline;
        p->preRender(n->data);
#ifdef NO_REVERSE_PAINTER
        renderNode(n, camera);
        for (count = 1; count < (renderQueue.size - i);){
            HPSnode *m = *(++nodes);
            if (m->pipeline == p){
                renderNode(m, camera);
                count++;
            } else break;
        }
#else
        HPSnode **m = nodes + 1;
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
    nodes = (HPSnode **) &alphaQueue.data[0];
    for (i = 0; i < alphaQueue.size;){
        HPSnode *n = *nodes;
        p = n->pipeline;
        p->preRender(n->data);
        renderNode(n, camera);
        for (count = 1; count < (alphaQueue.size - i);){
            HPSnode *m = *(++nodes);
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
static void computePlanes(HPScamera *camera){
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

void hpsRenderCamera(HPScamera *camera){
    currentCamera = *camera; // Set current camera to this one
    HPScamera *c = &currentCamera;
    float cameraMat[16];
    clearQueues();
    switch (camera->style){
    case ORBIT:
    {
        float cosPitch = cos(c->rotation.y);
        float sinPitch = sin(c->rotation.y);
        float sinYaw = sin(c->rotation.x);
        float cosYaw = cos(c->rotation.x);
        c->position.x = c->object.x + c->rotation.w * cosPitch * sinYaw;
        c->position.y = c->object.y + c->rotation.w * sinPitch;
        c->position.z = c->object.z + c->rotation.w * cosPitch * cosYaw;
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
    hpsPreRenderExtensions(c->scene);
    renderQueues(c);
    hpsPostRenderExtensions(c->scene);
    *camera = currentCamera; // Copy currentCamera back into camera
}

static void hpsOrthoCamera(int width, int height, HPScamera *camera){
    hpmOrtho(width, height, camera->n, camera->f, camera->projection);
}

static void hpsPerspectiveCamera(int width, int height, HPScamera *camera){
    hpmPerspective(width, height, camera->n, camera->f, camera->viewAngle,
		   camera->projection);
}

HPScamera *hpsMakeCamera(HPScameraType type, HPScameraStyle style, HPSscene *scene){
    HPScamera *camera = malloc(sizeof(struct camera));
    camera->n = HPS_DEFAULT_NEAR_PLANE;
    camera->f = HPS_DEFAULT_FAR_PLANE;
    camera->viewAngle = HPS_DEFAULT_VIEW_ANGLE;
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
    if (type == HPS_ORTHO)
        camera->update = &hpsOrthoCamera;
    else
        camera->update = &hpsPerspectiveCamera;
    camera->style = style;
    camera->scene = scene;
    hpsPush(&cameraList, (void *) camera);
    hpsPush(&activeCameras, (void *) camera);
    int w, h;
    windowSizefun(&w, &h);
    camera->update(w, h, camera);
    return camera;
}

void hpsSetCameraClipPlanes(HPScamera *camera, float near, float far){
    camera->n = near;
    camera->f = far;
    int w, h;
    windowSizefun(&w, &h);
    camera->update(w, h, camera);
}

void hpsSetCameraViewAngle(HPScamera *camera, float angle){
    camera->viewAngle = angle;
    int w, h;
    windowSizefun(&w, &h);
    camera->update(w, h, camera);
}

void hpsDeleteCamera(HPScamera *camera){
    hpsRemove(&cameraList, (void *) camera);
    free(camera);
}

void hpsMoveCamera(HPScamera *camera, float *vec){
    if (camera->style == ORBIT){
        fprintf(stderr, "Can't move an orbit camera\n");
        return;
    }
    camera->position.x += vec[0];
    camera->position.y += vec[1];
    camera->position.z += vec[2];
}

void hpsSetCameraPosition(HPScamera *camera, float *vec){
    if (camera->style == ORBIT){
        fprintf(stderr, "Can't move an ORBIT camera\n");
        return;
    }
    camera->position.x = vec[0];
    camera->position.y = vec[1];
    camera->position.z = vec[2];
}

float *hpsCameraPosition(HPScamera *camera){
    return (float *) &camera->position;
}

float *hpsCameraRotation(HPScamera *camera){
    if (camera->style != POSITION){
        fprintf(stderr, "Can't rotation a non POSITION camera\n");
        return NULL;
    }
    return (float *) &camera->rotation;
}

void hpsSetCameraUp(HPScamera *camera, float *up){
    if (camera->style != LOOK_AT){
        fprintf(stderr, "Can't set up on a non LOOK_AT camera\n");
        return;
    }
    camera->up.x = up[0];
    camera->up.y = up[1];
    camera->up.z = up[2];
}

void hpsCameraLookAt(HPScamera *camera, float *p){
    if ((camera->style != ORBIT) && (camera->style != LOOK_AT)) {
        fprintf(stderr, "Can't set object to look at for a non LOOK_AT or ORBIT camera\n");
        return;
    }
    camera->object.x = p[0];
    camera->object.y = p[1];
    camera->object.z = p[2];
}

void hpsYawCamera(HPScamera *camera, float angle){
    if ((camera->style != ORBIT) && (camera->style != FIRST_PERSON)) {
        fprintf(stderr, "Can't yaw a non ORBIT or FIRST_PERSON camera\n");
        return;
    }
    camera->rotation.x += angle;
}

void hpsSetCameraYaw(HPScamera *camera, float angle){
    if ((camera->style != ORBIT) && (camera->style != FIRST_PERSON)) {
        fprintf(stderr, "Can't yaw a non ORBIT or FIRST_PERSON camera\n");
        return;
    }
    camera->style = ORBIT;
    camera->rotation.x = angle;
}

void hpsPitchCamera(HPScamera *camera, float angle){
    if ((camera->style != ORBIT) && (camera->style != FIRST_PERSON)) {
        fprintf(stderr, "Can't pitch a non ORBIT or FIRST_PERSON camera\n");
        return;
    }
    camera->rotation.y += angle;
    camera->rotation.y = fmin(HALF_PI, fmax(-HALF_PI, camera->rotation.y));
}

void hpsSetCameraPitch(HPScamera *camera, float angle){
    if ((camera->style != ORBIT) && (camera->style != FIRST_PERSON)) {
        fprintf(stderr, "Can't pitch a non ORBIT or FIRST_PERSON camera\n");
        return;
    }
    camera->rotation.y = fmin(HALF_PI, fmax(-HALF_PI, angle));
}

void hpsZoomCamera(HPScamera *camera, float distance){
    if (camera->style != ORBIT){
        fprintf(stderr, "Can't zoom a non ORBIT camera\n");
        return;
    }
    camera->rotation.w += distance;
    camera->rotation.w = (camera->rotation.w < FLT_MIN) ? FLT_MIN : camera->rotation.w;
}

void hpsSetCameraZoom(HPScamera *camera, float distance){
    if (camera->style != ORBIT){
        fprintf(stderr, "Can't zoom a non ORBIT camera\n");
        return;
    }
    camera->rotation.w = (distance < 0.0) ? 0.0 : distance;
}

void hpsRollCamera(HPScamera *camera, float angle){
    if ((camera->style != ORBIT) && (camera->style != FIRST_PERSON)) {
        fprintf(stderr, "Can't roll a non ORBIT or FIRST_PERSON camera\n");
        return;
    }
    camera->rotation.z += angle;
}

void hpsSetCameraRoll(HPScamera *camera, float angle){
    if ((camera->style != ORBIT) && (camera->style != LOOK_AT)) {
        fprintf(stderr, "Can't roll a non ORBIT or LOOK_AT camera\n");
        return;
    }
    camera->rotation.z = angle;
}

void hpsResizeCameras(){
    int i, w, h;
    windowSizefun(&w, &h);
    for (i = 0; i < cameraList.size; i++){
	HPScamera *camera = (HPScamera *) cameraList.data[i];
	camera->update(w, h, camera);
    }
}

void hpsMoveCameraForward(HPScamera *camera, float dist){
    if ((camera->style != FIRST_PERSON)) {
        fprintf(stderr, "Can't move a non FIRST_PERSON camera forward\n");
        return;
    }
    float sinYaw = sin(camera->rotation.x);
    float cosYaw = cos(camera->rotation.x);
    camera->position.x += dist * sinYaw;
    camera->position.z -= dist * cosYaw;
}

void hpsMoveCameraUp(HPScamera *camera, float dist){
    if ((camera->style != FIRST_PERSON)) {
        fprintf(stderr, "Can't move a non FIRST_PERSON camera up\n");
        return;
    }
    float sinYaw = sin(camera->rotation.x);
    float cosYaw = cos(camera->rotation.x);
    camera->position.y += dist;
}

void hpsStrafeCamera(HPScamera *camera, float dist){
    if ((camera->style != FIRST_PERSON)) {
        fprintf(stderr, "Can't strafe a non FIRST_PERSON camera\n");
        return;
    }
    float sinYaw = sin(camera->rotation.x);
    float cosYaw = cos(camera->rotation.x);
    camera->position.x += dist * cosYaw;
    camera->position.z += dist * sinYaw;
}


void hpsRenderCameras(){
    int i;
    for (i = 0; i < activeCameras.size; i++)
	hpsRenderCamera((HPScamera *) activeCameras.data[i]);
}

void hpsActivateCamera(HPScamera *c){
    hpsRemove(&activeCameras, (void *) c);
    hpsPush(&activeCameras, (void *) c);
}

void hpsDeactivateCamera(HPScamera *c){
    hpsRemove(&activeCameras, (void *) c);
}

void hpsInitCameras(){
    hpsInitVector(&cameraList, 16);
    hpsInitVector(&activeCameras, 16);
    hpsInitVector(&renderQueue, 4096);
    hpsInitVector(&alphaQueue, 1024);
}
