#include <hypermath.h>
#include <hyperscene.h>
#include "memory.h"
#include "partition.h"

struct pipeline {
    bool hasAlpha;
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
};

struct camera {
    HPGscene *scene;
    HPGcameraStyle style;
    HPMpoint position, up, object;
    float n, f, viewAngle;
    HPMquat rotation; // pan, tilt, roll, distance for ORBIT camera
    float projection[16];
    float viewProjection[16];
    float modelViewProjection[16];
    Plane planes[6];
    HPGcameraUpdateFun update;
};

void hpgInitCameras();
void hpgSetWindowSizeFun(HPGwindowSizeFun fun);
