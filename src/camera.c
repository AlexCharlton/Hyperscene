#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <float.h>
#include "scene.h"
#define HALF_PI 1.57079631

typedef enum {
    RIGHT, LEFT, TOP, BOTTOM, NEAR, FAR
} Faces;

static HPSvector cameraList, activeCameras, renderQueue, alphaQueue;

static HPScamera currentCamera;
static float currentInverseTransposeModel[16];

HPScamera *hpsCurrentCamera(){ return &currentCamera; }

float *hpsCurrentCameraPosition(){
    return (float *) &currentCamera.position;
}

float *hpsCurrentCameraProjection(){
    return currentCamera.projection;
}

float *hpsCurrentCameraView(){
    return currentCamera.view;
}

float *hpsCurrentCameraViewProjection(){
    return currentCamera.viewProjection;
}

float *hpsCurrentCameraModelViewProjection(){
    return currentCamera.modelViewProjection;
}

float *hpsCurrentInverseTransposeModel(){
    return currentInverseTransposeModel;
}

static void addToQueue(Node *node){
    HPSnode *n = (HPSnode *) node->data;
    if (n->pipeline){
        if (n->pipeline->isAlpha){
            hpsPush(&alphaQueue, n);
        } else {
            hpsPush(&renderQueue, n);
        }
    }
    if (n->extension){
        hpsVisibleExtensionNode(n);
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

static void xPositive(const HPMpoint *a, const HPMpoint *b, float *m, float *n){
    *m = a->x; *n = b->x;
}

static void xNegative(const HPMpoint *a, const HPMpoint *b, float *m, float *n){
    *m = -a->x; *n = -b->x;
}

static void yPositive(const HPMpoint *a, const HPMpoint *b, float *m, float *n){
    *m = a->y; *n = b->y;
}

static void yNegative(const HPMpoint *a, const HPMpoint *b, float *m, float *n){
    *m = -a->y; *n = -b->y;
}

static void zPositive(const HPMpoint *a, const HPMpoint *b, float *m, float *n){
    *m = a->z; *n = b->z;
}

static void zNegative(const HPMpoint *a, const HPMpoint *b, float *m, float *n){
    *m = -a->z; *n = -b->z;
}

static int programSort(const void *a, const void *b){
    HPSpipeline *pa = (*((HPSnode **) a))->pipeline;
    HPSpipeline *pb = (*((HPSnode **) b))->pipeline;
    if (pa < pb) return -1;
    else if (pa > pb) return 1;
    return 0;
}

int hpsCloserToCamera(const HPScamera *camera, const float *a, const float *b){
    float m, n;
    camera->sort((HPMpoint *) a, (HPMpoint *) b, &m, &n);
    if (m < n) return -1;
    else if (m > n) return 1;
    return 0;
}

int hpsFurtherFromCameraRough(const HPScamera *camera, const float *a, const float *b){
    float m, n;
    camera->sort((HPMpoint *) a, (HPMpoint *) b, &m, &n);
    if (m > n) return -1;
    else if (m < n) return 1;
    return 0;
}

int hpsFurtherFromCamera(const HPScamera *camera, const float *a, const float *b){
    float m, n, mx, my, mz, nx, ny, nz;
    HPMpoint c = camera->position;
    mx = a[0] - c.x;
    my = a[1] - c.y;
    mz = a[2] - c.z;
    m =  mx*mx + my*my + mz*mz;
    nx = b[0] - c.x;
    ny = b[1] - c.y;
    nz = b[2] - c.z;
    n =  nx*nx + ny*ny + nz*nz;
    if (m > n) return -1;
    else if (m < n) return 1;
    return 0;
}

int hpsBSCloserToCamera(const HPScamera *camera, const float *a, const float *b){
    float m, n;
    camera->sort((HPMpoint *) a, (HPMpoint *) b, &m, &n);
    float mr = m - a[3];
    float nr = n - b[3];
    if (mr < nr) return -1;
    else if (mr > nr) return 1;
    return 0;
}

int hpsBSFurtherFromCameraRough(const HPScamera *camera, const float *a, const float *b){
    float m, n;
    camera->sort((HPMpoint *) a, (HPMpoint *) b, &m, &n);
    float mr = m - a[3];
    float nr = n - b[3];
    if (mr > nr) return -1;
    else if (mr < nr) return 1;
    return 0;
}

int hpsBSFurtherFromCamera(const HPScamera *camera, const float *a, const float *b){
    float m, n, mx, my, mz, nx, ny, nz, mr, nr;
    HPMpoint c = camera->position;
    mr = a[3];
    nr = b[3];
    mx = a[0] - c.x - mr;
    my = a[1] - c.y - mr;
    mz = a[2] - c.z - mr;
    m =  mx*mx + my*my + mz*mz;
    nx = b[0] - c.x - nr;
    ny = b[1] - c.y - nr;
    nz = b[2] - c.z - nr;
    n =  nx*nx + ny*ny + nz*nz;
    if (m > n) return -1;
    else if (m < n) return 1;
    return 0;
}

static void setCameraSort(HPScamera *camera){
    Plane *plane = &camera->planes[NEAR];
    float aa, ab, ac;
    aa = abs(plane->a); 
    ab = abs(plane->b); 
    ac = abs(plane->c); 

    if ((aa > ab) && (aa > ac)){
        if (plane->a > 0.0) { camera->sort = &xPositive; }
        else                { camera->sort = &xNegative; }
    } else if ((ab > ac)){
        if (plane->b > 0.0) { camera->sort = &yPositive; }
        else                { camera->sort = &yNegative; }
    } else {
        if (plane->c > 0.0) { camera->sort = &zPositive; }
        else                { camera->sort = &zNegative; }
    }
}

static int alphaSort(const void *a, const void *b){
    BoundingSphere *ba = (*((HPSnode **) a))->partitionData.boundingSphere;
    BoundingSphere *bb = (*((HPSnode **) b))->partitionData.boundingSphere;

#ifdef ROUGH_ALPHA

#ifdef VOLUMETRIC_ALPHA
    return hpsBSFurtherFromCameraRough(&currentCamera, ba, bb);
#else
    return hpsFurtherFromCameraRough(&currentCamera, (float *) ba, (float *) bb);
#endif

#else // not ROUGH_ALPHA

#ifdef VOLUMETRIC_ALPHA
    return hpsBSFurtherFromCamera(&currentCamera, ba, bb);
#else
    return hpsFurtherFromCamera(&currentCamera, (float *) ba, (float *) bb);
#endif

#endif // ROUGH_ALPHA
}

static int renderSort(const void *a, const void *b){
    BoundingSphere *ba = (*((HPSnode **) a))->partitionData.boundingSphere;
    BoundingSphere *bb = (*((HPSnode **) b))->partitionData.boundingSphere;
    return hpsBSCloserToCamera(&currentCamera, (float *) ba, (float *) bb);
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
    qsort(renderQueue.data, renderQueue.size, sizeof(void *), &programSort);
    HPSnode **nodes = (HPSnode **) renderQueue.data;
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
            if ((*m)->pipeline == p){
                m++;
                count++;
            } else {
                break;
            }
        }
        qsort(nodes, count, sizeof(void *), renderSort);

        for (j = 0; j < count; j++){
            renderNode(*nodes, camera);
            nodes++;
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

void hpsUpdateCamera(HPScamera *camera){
    HPScamera *c = camera;
    float cameraMat[16];
    switch (camera->style){
    case HPS_ORBIT:
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
    case HPS_LOOK_AT:
        hpmLookAt((float *) &c->position, (float *) &c->object, (float *) &c->up, 
                  c->view);
        break;
    case HPS_POSITION:
        hpmQuaternionRotation((float *) &c->rotation, cameraMat);
        hpmTranslate((float *) &c->position, cameraMat);
        hpmCameraInverse(cameraMat, c->view);
        break;
    case HPS_FIRST_PERSON:
        hpmYPRRotation(-c->rotation.x, c->rotation.y, c->rotation.z, cameraMat);
        hpmTranslate((float *) &c->position, cameraMat);
        hpmCameraInverse(cameraMat, c->view);
        break;
    default:
        fprintf(stderr, "Camera does not have a valid style\n");
        exit(EXIT_FAILURE);
    }
    hpmMultMat4(c->projection, c->view, c->viewProjection);
}

void hpsRenderCamera(HPScamera *camera){
    currentCamera = *camera; // Set current camera to this one
    HPScamera *c = &currentCamera;
    clearQueues();
    computePlanes(c);
    c->scene->partitionInterface->doVisible(c->scene->partitionStruct,
                                            c->planes, &addToQueue);
    setCameraSort(c);
    hpsPreRenderExtensions(c->scene);
    renderQueues(c);
    hpsPostRenderExtensions(c->scene);
    *camera = currentCamera; // Copy currentCamera back into camera
}

static void hpsOrthoCamera(HPScamera *camera){
    float width = camera->vw * camera->vwRatio;
    float height = camera->vh * camera->vhRatio;
    float r = width * (0.5 + camera->vx);
    float l = r - width;
    float t = height * (0.5 + camera->vy);
    float b = t - height;
    hpmOrthoViewport(l, r, b, t, camera->n, camera->f, 
                     camera->vl, camera->vr, camera->vb, camera->vt,
                     camera->projection);
}

static void hpsPerspectiveCamera(HPScamera *camera){
    float width = camera->vw * camera->vwRatio;
    float height = camera->vh * camera->vhRatio;
    float scale = tan(hpmDegreesToRadians(camera->viewAngle * 0.5)) 
        * camera->n * 2.0;
    width = ((float) width / (float) height) * scale;
    height = scale;
    float r = width * (0.5 + camera->vx);
    float l = r - width;
    float t = height * (0.5 + camera->vy);
    float b = t - height;
    hpmFrustumViewport(l, r, b, t, camera->n, camera->f,
                       camera->vl, camera->vr, camera->vb, camera->vt,
                       camera->projection);
}

HPScamera *hpsMakeCamera(HPScameraType type, HPScameraStyle style, HPSscene *scene, float width, float height){
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
    camera->vw = width; camera->vh = height;
    camera->vwRatio = 1.0; camera->vhRatio = 1.0;
    camera->vl = -1.0; camera->vr = 1.0;
    camera->vb = -1.0; camera->vt = 1.0;
    camera->vx = 0.0; camera->vy = 0.0;
    camera->viewportIsStatic = false;
    if (type == HPS_ORTHO)
        camera->update = &hpsOrthoCamera;
    else
        camera->update = &hpsPerspectiveCamera;
    camera->style = style;
    camera->scene = scene;
    hpsPush(&cameraList, (void *) camera);
    hpsPush(&activeCameras, (void *) camera);
    camera->update(camera);
    return camera;
}

void hpsSetCameraClipPlanes(HPScamera *camera, float near, float far){
    camera->n = near;
    camera->f = far;
    camera->update(camera);
}

void hpsSetCameraViewAngle(HPScamera *camera, float angle){
    camera->viewAngle = angle;
    camera->update(camera);
}

void hpsSetCameraViewportRatio(HPScamera *camera, float width, float height){
    camera->vwRatio = width;
    camera->vhRatio = height;
    camera->update(camera);
}

void hpsSetCameraViewportDimensions(HPScamera *camera, float width, float height){
    camera->vw = width;
    camera->vh = height;
    camera->viewportIsStatic = true;
    camera->update(camera);
}

void hpsSetCameraViewportScreenPosition(HPScamera *camera, float left, float right, float bottom, float top){
    if ((left < -1.0) || ( left > 1.0) ||
        (right < -1.0) || ( right > 1.0) ||
        (bottom < -1.0) || ( bottom > 1.0) ||
        (top < -1.0) ||( top > 1.0)){
        fprintf(stderr, "Camera viewport screen position values should be between -1.0 and 1.0.\n");
        return;
    }
    camera->vl = left; camera->vr = right;
    camera->vb = bottom; camera->vt = top;
    camera->update(camera);
}

void hpsSetCameraViewportOffset(HPScamera *camera, float x, float y){
    camera->vx = x; camera->vy = y;
    camera->update(camera);
}

void hpsDeleteCamera(HPScamera *camera){
    hpsDeactivateCamera(camera);
    hpsRemove(&cameraList, (void *) camera);
    free(camera);
}

void hpsMoveCamera(HPScamera *camera, float *vec){
    if (camera->style == HPS_ORBIT){
        fprintf(stderr, "Can't move an HPS_ORBIT camera\n");
        return;
    }
    camera->position.x += vec[0];
    camera->position.y += vec[1];
    camera->position.z += vec[2];
}

void hpsSetCameraPosition(HPScamera *camera, float *vec){
    if (camera->style == HPS_ORBIT){
        fprintf(stderr, "Can't move an HPS_ORBIT camera\n");
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
    if (camera->style != HPS_POSITION){
        fprintf(stderr, "Can't rotation a non HPS_POSITION camera\n");
        return NULL;
    }
    return (float *) &camera->rotation;
}

void hpsSetCameraUp(HPScamera *camera, float *up){
    if (camera->style != HPS_LOOK_AT){
        fprintf(stderr, "Can't set up on a non HPS_LOOK_AT camera\n");
        return;
    }
    camera->up.x = up[0];
    camera->up.y = up[1];
    camera->up.z = up[2];
}

void hpsCameraLookAt(HPScamera *camera, float *p){
    if ((camera->style != HPS_ORBIT) && (camera->style != HPS_LOOK_AT)) {
        fprintf(stderr, "Can't set object to look at for a non HPS_LOOK_AT or HPS_ORBIT camera\n");
        return;
    }
    camera->object.x = p[0];
    camera->object.y = p[1];
    camera->object.z = p[2];
}

void hpsYawCamera(HPScamera *camera, float angle){
    if ((camera->style != HPS_ORBIT) && (camera->style != HPS_FIRST_PERSON)) {
        fprintf(stderr, "Can't yaw a non HPS_ORBIT or HPS_FIRST_PERSON camera\n");
        return;
    }
    camera->rotation.x += angle;
}

void hpsSetCameraYaw(HPScamera *camera, float angle){
    if ((camera->style != HPS_ORBIT) && (camera->style != HPS_FIRST_PERSON)) {
        fprintf(stderr, "Can't yaw a non HPS_ORBIT or HPS_FIRST_PERSON camera\n");
        return;
    }
    camera->style = HPS_ORBIT;
    camera->rotation.x = angle;
}

void hpsPitchCamera(HPScamera *camera, float angle){
    if ((camera->style != HPS_ORBIT) && (camera->style != HPS_FIRST_PERSON)) {
        fprintf(stderr, "Can't pitch a non HPS_ORBIT or HPS_FIRST_PERSON camera\n");
        return;
    }
    camera->rotation.y += angle;
    camera->rotation.y = fmin(HALF_PI, fmax(-HALF_PI, camera->rotation.y));
}

void hpsSetCameraPitch(HPScamera *camera, float angle){
    if ((camera->style != HPS_ORBIT) && (camera->style != HPS_FIRST_PERSON)) {
        fprintf(stderr, "Can't pitch a non HPS_ORBIT or HPS_FIRST_PERSON camera\n");
        return;
    }
    camera->rotation.y = fmin(HALF_PI, fmax(-HALF_PI, angle));
}

void hpsZoomCamera(HPScamera *camera, float distance){
    if (camera->style != HPS_ORBIT){
        fprintf(stderr, "Can't zoom a non HPS_ORBIT camera\n");
        return;
    }
    camera->rotation.w += distance;
    camera->rotation.w = (camera->rotation.w < FLT_MIN) ? FLT_MIN : camera->rotation.w;
}

void hpsSetCameraZoom(HPScamera *camera, float distance){
    if (camera->style != HPS_ORBIT){
        fprintf(stderr, "Can't zoom a non HPS_ORBIT camera\n");
        return;
    }
    camera->rotation.w = (distance < 0.0) ? 0.0 : distance;
}

void hpsRollCamera(HPScamera *camera, float angle){
    if ((camera->style != HPS_ORBIT) && (camera->style != HPS_FIRST_PERSON)) {
        fprintf(stderr, "Can't roll a non HPS_ORBIT or HPS_FIRST_PERSON camera\n");
        return;
    }
    camera->rotation.z += angle;
}

void hpsSetCameraRoll(HPScamera *camera, float angle){
    if ((camera->style != HPS_ORBIT) && (camera->style != HPS_LOOK_AT)) {
        fprintf(stderr, "Can't roll a non HPS_ORBIT or HPS_LOOK_AT camera\n");
        return;
    }
    camera->rotation.z = angle;
}

void hpsResizeCameras(float width, float height){
    int i;
    for (i = 0; i < cameraList.size; i++){
	HPScamera *camera = (HPScamera *) cameraList.data[i];
        if (!camera->viewportIsStatic){
            camera->vw = width;
            camera->vh = height;
            camera->update(camera);
        }
    }
}

void hpsMoveCameraForward(HPScamera *camera, float dist){
    if ((camera->style != HPS_FIRST_PERSON)) {
        fprintf(stderr, "Can't move a non HPS_FIRST_PERSON camera forward\n");
        return;
    }
    float sinYaw = sin(camera->rotation.x);
    float cosYaw = cos(camera->rotation.x);
    camera->position.x += dist * sinYaw;
    camera->position.z -= dist * cosYaw;
}

void hpsMoveCameraUp(HPScamera *camera, float dist){
    if ((camera->style != HPS_FIRST_PERSON)) {
        fprintf(stderr, "Can't move a non HPS_FIRST_PERSON camera up\n");
        return;
    }
    camera->position.y += dist;
}

void hpsStrafeCamera(HPScamera *camera, float dist){
    if ((camera->style != HPS_FIRST_PERSON)) {
        fprintf(stderr, "Can't strafe a non HPS_FIRST_PERSON camera\n");
        return;
    }
    float sinYaw = sin(camera->rotation.x);
    float cosYaw = cos(camera->rotation.x);
    camera->position.x += dist * cosYaw;
    camera->position.z += dist * sinYaw;
}

float *hpsCameraProjection(HPScamera *camera){
    return camera->projection;
}

float *hpsCameraView(HPScamera *camera){
    return camera->view;
}

float *hpsCameraViewProjection(HPScamera *camera){
    return camera->viewProjection;
}

void hpsUpdateCameras(){
    int i;
    for (i = 0; i < activeCameras.size; i++)
	hpsUpdateCamera((HPScamera *) activeCameras.data[i]);
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
