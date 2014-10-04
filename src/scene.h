#include <hypermath.h>
#include <hyperscene.h>
#include "memory.h"
#include "partition.h"

struct pipeline {
    long hasAlpha; // A boolean value expressed as a long so that pipeline can't be mixed up with an extension
    void (*preRender)(void *);
    void (*render)(void *);
    void (*postRender)();
};

struct node {
    struct node *parent;
    Node partitionData;
    HPGscene *scene;
    HPGvector children;
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
    HPGvector topLevelNodes;
    PartitionInterface *partitionInterface;
    void *partitionStruct;
    HPGpool nodePool, boundingSpherePool, transformPool, partitionPool;
    HPGvector extensions;
};

struct camera {
    HPGscene *scene;
    HPGcameraStyle style;
    HPMpoint position, up, object;
    float n, f, viewAngle;
    HPMquat rotation; // pan, tilt, roll, distance for ORBIT camera
    float view[16];
    float projection[16];
    float viewProjection[16];
    float modelViewProjection[16];
    Plane planes[6];
    HPGcameraUpdateFun update;
};

void hpgInitCameras();
void hpgSetWindowSizeFun(HPGwindowSizeFun fun);

/* Extensions */
void hpgPreRenderExtensions(HPGscene *scene);
void hpgPostRenderExtensions(HPGscene *scene);
void hpgVisibleNodeExtensions(HPGscene *scene, HPGnode *node);
void hpgUpdateExtensions(HPGscene *scene);
void hpgDeleteExtensions(HPGscene *scene);
