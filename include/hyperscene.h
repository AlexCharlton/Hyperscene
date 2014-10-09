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
typedef struct partitionInterface HPSpartitionInterface;
typedef void (*HPSwindowSizeFun)(int *, int *);

typedef struct HPSextension {
    void (*init)(void **);
    void (*preRender)(void *);
    void (*postRender)(void *);
    void (*visibleNode)(void *, HPSnode *node);
    void (*update)(void *);
    void (*delete)(void *);
} HPSextension;

extern unsigned int hpsNodePoolSize;

extern HPSpartitionInterface *hpsPartitionInterface;

void hpsInit(HPSwindowSizeFun windowSizeFun);

HPSscene *hpsGetScene(HPSnode *node);

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

void* hpsNodeData(HPSnode *node);

HPSscene *hpsMakeScene();

void hpsDeleteScene(HPSscene *scene);

void hpsActivateScene(HPSscene *s);

void hpsDeactivateScene(HPSscene *s);

void hpsUpdateScenes();

/* Pipelines */
HPSpipeline *hpsAddPipeline(void (*preRender)(void *),
			    void (*render)(void *),
			    void (*postRender)(),
                            bool hasAlpha);

void hpsDeletePipeline(HPSpipeline *pipeline);

/* Cameras */

float *hpsCurrentInverseTransposeModel;

float *hpsCurrentCameraPosition;

float *hpsCurrentCameraView;

float *hpsCurrentCameraProjection;

float *hpsCurrentCameraViewProjection;

float *hpsCurrentCameraModelViewProjection;

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

void hpsYawCamera(HPScamera *camera, float angle);

void hpsSetCameraYaw(HPScamera *camera, float angle);

void hpsPitchCamera(HPScamera *camera, float angle);

void hpsSetCameraPitch(HPScamera *camera, float angle);

void hpsZoomCamera(HPScamera *camera, float distance);

void hpsSetCameraZoom(HPScamera *camera, float distance);

void hpsRollCamera(HPScamera *camera, float angle);

void hpsSetCameraRoll(HPScamera *camera, float angle);

void hpsMoveCameraForward(HPScamera *camera, float dist);

void hpsMoveCameraUp(HPScamera *camera, float dist);

void hpsStrafeCamera(HPScamera *camera, float dist);

void hpsResizeCameras();

void hpsRenderCameras();

void hpsActivateCamera(HPScamera *c);

void hpsDeactivateCamera(HPScamera *c);

/* Spatial partitioning interfaces */
extern void *hpsAABBpartitionInterface;

extern unsigned int hpsAABBpartitionPoolSize;

/* Extensions */
void hpsActivateExtension(HPSscene *scene, HPSextension *extension);

void *hpsExtensionData(HPSscene *scene, HPSextension *extension);

