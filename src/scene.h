#include <hypermath.h>
#include <hyperscene.h>
#include "memory.h"
#include "partition.h"

typedef void (*cameraUpdateFun)(int, int, HPScamera*);

struct pipeline {
    long isAlpha; // A boolean value expressed as a long so that pipeline can't be mixed up with an extension
    void (*preRender)(void *);
    void (*render)(void *);
    void (*postRender)();
};

struct node {
    struct node *parent;
    Node partitionData;
    HPSscene *scene;
    HPSvector children;
    HPMpoint position;
    HPMquat rotation;
    float *transform;
    struct pipeline *pipeline;
    void (*delete)(void *); //(data)
    void *data;
    bool needsUpdate;
};

struct scene {
    void *null; // used to distinguish top-level nodes;
    HPSvector topLevelNodes;
    PartitionInterface *partitionInterface;
    void *partitionStruct;
    HPSpool nodePool, boundingSpherePool, transformPool, partitionPool;
    HPSvector extensions;
};

struct camera {
    HPSscene *scene;
    HPScameraStyle style;
    HPMpoint position, up, object;
    float n, f, viewAngle;
    HPMquat rotation; // yaw, pitch, roll, distance for ORBIT camera
    float view[16];
    float projection[16];
    float viewProjection[16];
    float modelViewProjection[16];
    Plane planes[6];
    cameraUpdateFun update;
};

void hpsInitCameras();
void hpsSetWindowSizeFun(HPSwindowSizeFun fun);

/* Extensions */
void hpsPreRenderExtensions(HPSscene *scene);
void hpsPostRenderExtensions(HPSscene *scene);
void hpsVisibleNodeExtensions(HPSscene *scene, HPSnode *node);
void hpsUpdateNodeExtensions(HPSscene *scene, HPSnode *node);
void hpsDeleteExtensions(HPSscene *scene);
