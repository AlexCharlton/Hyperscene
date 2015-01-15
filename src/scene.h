#include <hypermath.h>
#include <hyperscene.h>
#include "memory.h"
#include "partition.h"

typedef void (*cameraUpdateFun)(int, int, HPScamera*);

struct pipeline {
    bool isAlpha;
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
    void **extension;
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
    void (*sort)(const HPMpoint*, const HPMpoint*, float *, float*); // used to sort points based on camera positioning
};

void hpsInitCameras();
void hpsSetWindowSizeFun(HPSwindowSizeFun fun);

/* Extensions */
void hpsPreRenderExtensions(HPSscene *scene);
void hpsPostRenderExtensions(HPSscene *scene);
void hpsDeleteExtensions(HPSscene *scene);
void hpsVisibleExtensionNode(HPSnode *node);
void hpsUpdateExtensionNode(HPSnode *node);
