#include <stdbool.h>

#define HPS_DEFAULT_NEAR_PLANE 1.0
#define HPS_DEFAULT_FAR_PLANE 10000.0
#define HPS_DEFAULT_VIEW_ANGLE 70.0

typedef enum {
    HPS_ORTHO, HPS_PERSPECTIVE
} HPScameraType;

typedef enum {
    POSITION, LOOK_AT, ORBIT, FIRST_PERSON
} HPScameraStyle;


typedef struct node HPSnode;
typedef struct scene HPSscene;
typedef struct camera HPScamera;
typedef struct pipeline HPSpipeline;
typedef void (*HPSwindowSizeFun)(int *, int *);
typedef void (*HPScameraUpdateFun)(int, int, HPScamera*);

typedef struct HPSextension {
    void (*init)(void **);
    void (*preRender)(void *);
    void (*postRender)(void *);
    void (*visibleNode)(void *, HPSnode *node);
    void (*update)(void *);
    void (*delete)(void *);
} HPSextension;

extern unsigned int hpsNodePoolSize, hpsBoundingSpherePoolSize, hpsTransformPoolSize,
    hpsPartitionPoolSize;

void hpsInitScenes(HPSwindowSizeFun windowSizeFun);

HPSnode *hpsAddNode(HPSnode *parent, void *data,
                    HPSpipeline *pipeline,
                    void (*deleteFunc)(void *));

void hpsDeleteNode(HPSnode *node);

void hpsSetNodeBoundingSphere(HPSnode *node, float radius);

float *hpsNodeBoundingSphere(HPSnode *node);

void hpsMoveNode(HPSnode *node, float *vec);

void hpsSetNodePosition(HPSnode *node, float *p);

float* hpsNodeRotation(HPSnode *node);

float* hpsNodePosition(HPSnode *node);

float* hpsNodeTransform(HPSnode *node);

float* hpsNodeData(HPSnode *node);

HPSscene *hpsMakeScene(void *partitionInterface);

void hpsDeleteScene(HPSscene *scene);

void hpsActiveateScene(HPSscene *s);

void hpsDeactiveateScene(HPSscene *s);

void hpsUpdateScenes();

/* Pipelines */
HPSpipeline *hpsAddPipeline(void (*preRender)(void *),
			    void (*render)(void *),
			    void (*postRender)(),
                            bool hasAlpha);

void hpsDeletePipeline(HPSpipeline *pipeline);

/* Cameras */

float *hpsCurrentInverseTransposeModel();

float *hpsCurrentCameraPosition();

float *hpsCurrentCameraView();

float *hpsCurrentCameraProjection();

float *hpsCurrentCameraViewProjection();

float *hpsCurrentCameraModelViewProjection();

void hpsRenderCamera(HPScamera *camera);

HPScamera *hpsMakeCamera(HPScameraType type, HPScameraStyle style, HPSscene *scene);

void hpsSetCameraClipPlanes(HPScamera *camera, float near, float far);

void hpsSetCameraViewAngle(HPScamera *camera, float angle);

void hpsDeleteCamera(HPScamera *camera);

void hpsMoveCamera(HPScamera *camera, float *vec);

void hpsSetCameraPosition(HPScamera *camera, float *vec);

float *hpsCameraPosition(HPScamera *camera);

float *hpsCameraRotation(HPScamera *camera);

void hpsSetCameraUp(HPScamera *camera, float *up);

void hpsCameraLookAt(HPScamera *camera, float *p);

void hpsPanCamera(HPScamera *camera, float angle);

void hpsSetCameraPan(HPScamera *camera, float angle);

void hpsTiltCamera(HPScamera *camera, float angle);

void hpsSetCameraTilt(HPScamera *camera, float angle);

void hpsZoomCamera(HPScamera *camera, float distance);

void hpsSetCameraZoom(HPScamera *camera, float distance);

void hpsRollCamera(HPScamera *camera, float angle);

void hpsSetCameraRoll(HPScamera *camera, float angle);

void hpsMoveCameraForward(HPScamera *camera, float dist);

void hpsMoveCameraUp(HPScamera *camera, float dist);

void hpsStrafeCamera(HPScamera *camera, float dist);

void hpsResizeCameras(int width, int height);

void hpsRenderCameras();

void hpsActiveateCamera(HPScamera *c);

void hpsDeactiveateCamera(HPScamera *c);

/* Spatial partitioning interfaces */
extern void *hpsAABBpartitionInterface;

/* Extensions */
void hpsActivateExtension(HPSscene *scene, HPSextension *extension);

void *hpsExtensionData(HPSscene *scene, HPSextension *extension);

