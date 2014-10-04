#include <stdbool.h>

#define HPG_DEFAULT_NEAR_PLANE 1.0
#define HPG_DEFAULT_FAR_PLANE 10000.0
#define HPG_DEFAULT_VIEW_ANGLE 70.0

typedef enum {
    HPG_ORTHO, HPG_PERSPECTIVE
} HPGcameraType;

typedef enum {
    POSITION, LOOK_AT, ORBIT, FIRST_PERSON
} HPGcameraStyle;


typedef struct node HPGnode;
typedef struct scene HPGscene;
typedef struct camera HPGcamera;
typedef struct pipeline HPGpipeline;
typedef void (*HPGwindowSizeFun)(int *, int *);
typedef void (*HPGcameraUpdateFun)(int, int, HPGcamera*);

typedef struct HPGextension {
    void (*init)(void **);
    void (*preRender)(void *);
    void (*postRender)(void *);
    void (*visibleNode)(void *, HPGnode *node);
    void (*update)(void *);
    void (*delete)(void *);
} HPGextension;

extern unsigned int hpgNodePoolSize, hpgBoundingSpherePoolSize, hpgTransformPoolSize,
    hpgPartitionPoolSize;

void hpgInitScenes(HPGwindowSizeFun windowSizeFun);

HPGnode *hpgAddNode(HPGnode *parent, void *data,
                    HPGpipeline *pipeline,
                    void (*deleteFunc)(void *));

void hpgDeleteNode(HPGnode *node);

void hpgSetNodeBoundingSphere(HPGnode *node, float radius);

float *hpgNodeBoundingSphere(HPGnode *node);

void hpgMoveNode(HPGnode *node, float *vec);

void hpgSetNodePosition(HPGnode *node, float *p);

float* hpgNodeRotation(HPGnode *node);

float* hpgNodePosition(HPGnode *node);

float* hpgNodeTransform(HPGnode *node);

float* hpgNodeData(HPGnode *node);

HPGscene *hpgMakeScene(void *partitionInterface);

void hpgDeleteScene(HPGscene *scene);

void hpgActiveateScene(HPGscene *s);

void hpgDeactiveateScene(HPGscene *s);

void hpgUpdateScenes();

/* Pipelines */
HPGpipeline *hpgAddPipeline(void (*preRender)(void *),
			    void (*render)(void *),
			    void (*postRender)(),
                            bool hasAlpha);

void hpgPipelineAlpha(HPGpipeline *pipeline, bool hasAlpha);

void hpgDeletePipeline(HPGpipeline *pipeline);

/* Cameras */

extern float hpgCurrentInverseTransposeModel[16];

float *hpgCurrentCameraPosition();

float *hpgCurrentCameraView();

float *hpgCurrentCameraProjection();

float *hpgCurrentCameraViewProjection();

float *hpgCurrentCameraModelViewProjection();

void hpgRenderCamera(HPGcamera *camera);

HPGcamera *hpgMakeCamera(HPGcameraType type, HPGcameraStyle style, HPGscene *scene);

void hpgSetCameraClipPlanes(HPGcamera *camera, float near, float far);

void hpgSetCameraViewAngle(HPGcamera *camera, float angle);

void hpgDeleteCamera(HPGcamera *camera);

void hpgMoveCamera(HPGcamera *camera, float *vec);

void hpgSetCameraPosition(HPGcamera *camera, float *vec);

float *hpgCameraPosition(HPGcamera *camera);

float *hpgCameraRotation(HPGcamera *camera);

void hpgSetCameraUp(HPGcamera *camera, float *up);

void hpgCameraLookAt(HPGcamera *camera, float *p);

void hpgPanCamera(HPGcamera *camera, float angle);

void hpgSetCameraPan(HPGcamera *camera, float angle);

void hpgTiltCamera(HPGcamera *camera, float angle);

void hpgSetCameraTilt(HPGcamera *camera, float angle);

void hpgZoomCamera(HPGcamera *camera, float distance);

void hpgSetCameraZoom(HPGcamera *camera, float distance);

void hpgRollCamera(HPGcamera *camera, float angle);

void hpgSetCameraRoll(HPGcamera *camera, float angle);

void hpgMoveCameraForward(HPGcamera *camera, float dist);

void hpgMoveCameraUp(HPGcamera *camera, float dist);

void hpgStrafeCamera(HPGcamera *camera, float dist);

void hpgResizeCameras(int width, int height);

void hpgRenderCameras();

void hpgActiveateCamera(HPGcamera *c);

void hpgDeactiveateCamera(HPGcamera *c);

/* Spatial partitioning interfaces */
extern void *hpgAABBpartitionInterface;

/* Extensions */
void hpgActivateExtension(HPGscene *scene, HPGextension *extension);

void *hpgExtensionData(HPGscene *scene, HPGextension *extension);

