#include <hypermath.h>
#include <hyperscene.h>
#include "memory.h"
#include "partition.h"

typedef enum {
    POSITION, LOOK_AT, ORBIT
} CameraStyle;


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
    float x, y, z, rx, ry, rz, angle;
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
    CameraStyle style;
    HPMpoint position, up, object;
    float pan, tilt, roll, distance;
    float n, f, viewAngle;
    float rotation[16];
    float projection[16];
    float viewProjection[16];
    float modelViewProjection[16];
    Plane planes[6];
    HPGcameraUpdateFun update;
};

void hpgInitCameras();
void hpgSetWindowSizeFun(HPGwindowSizeFun fun);
