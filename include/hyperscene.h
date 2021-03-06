#include <stdbool.h>

#define HPS_DEFAULT_NEAR_PLANE 1.0
#define HPS_DEFAULT_FAR_PLANE 10000.0
#define HPS_DEFAULT_VIEW_ANGLE 70.0

typedef enum {
    HPS_ORTHO, HPS_PERSPECTIVE
} HPScameraType;

typedef enum {
    HPS_POSITION, HPS_LOOK_AT, HPS_ORBIT, HPS_FIRST_PERSON
} HPScameraStyle;

typedef struct node HPSnode;
typedef struct scene HPSscene;
typedef struct camera HPScamera;
typedef struct pipeline HPSpipeline;
typedef struct partitionInterface HPSpartitionInterface;

typedef struct HPSextension {
    void (*init)(void **);
    void (*preRender)(void *);
    void (*postRender)(void *);
    void (*visibleNode)(void *, HPSnode *node);
    void (*updateNode)(void *, HPSnode *node);
    void (*delete)(void *);
} HPSextension;

extern unsigned int hpsNodePoolSize;

extern HPSpartitionInterface *hpsPartitionInterface;

void hpsInit();

HPSscene *hpsGetScene(HPSnode *node);

HPSnode *hpsAddNode(HPSnode *parent, void *data,
                    HPSpipeline *pipeline,
                    void (*deleteFunc)(void *));

void hpsDeleteNode(HPSnode *node);

void hpsSetNodeBoundingSphere(HPSnode *node, float radius);

float *hpsNodeBoundingSphere(HPSnode *node);

void hpsMoveNode(HPSnode *node, float *vec);

void hpsSetNodePosition(HPSnode *node, float *p);

void hpsNodeNeedsUpdate(HPSnode *node);

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
HPScamera *hpsCurrentCamera();

float *hpsCurrentInverseTransposeModel();

float *hpsCurrentCameraPosition();

float *hpsCurrentCameraView();

float *hpsCurrentCameraProjection();

float *hpsCurrentCameraViewProjection();

float *hpsCurrentCameraModelViewProjection();

void hpsUpdateCamera(HPScamera *camera);

void hpsRenderCamera(HPScamera *camera);

HPScamera *hpsMakeCamera(HPScameraType type, HPScameraStyle style, HPSscene *scene, float width, float height);

void hpsSetCameraClipPlanes(HPScamera *camera, float near, float far);

void hpsSetCameraViewAngle(HPScamera *camera, float angle);

void hpsSetCameraViewportRatio(HPScamera *camera, float width, float height);

void hpsSetCameraViewportDimensions(HPScamera *camera, float width, float height);

void hpsSetCameraViewportScreenPosition(HPScamera *camera, float left, float right, float bottom, float top);

void hpsSetCameraViewportOffset(HPScamera *camera, float x, float y);

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

float *hpsCameraProjection(HPScamera *camera);

float *hpsCameraView(HPScamera *camera);

float *hpsCameraViewProjection(HPScamera *camera);

void hpsResizeCameras(float width, float height);

void hpsRenderCameras();

void hpsUpdateCameras();

void hpsActivateCamera(HPScamera *c);

void hpsDeactivateCamera(HPScamera *c);

/* Spatial partitioning interfaces */
extern void *hpsAABBpartitionInterface;

extern unsigned int hpsAABBpartitionPoolSize;

/* Extensions */
void hpsActivateExtension(HPSscene *scene, HPSextension *extension);

void *hpsExtensionData(HPSscene *scene, HPSextension *extension);

void *hpsNodeExtensionData(HPSnode *node);

void hpsSetNodeExtension(HPSnode *node, HPSextension *extension);

/* Sorting */
int hpsCloserToCamera(const HPScamera *camera, const float *a, const float *b);

int hpsFurtherFromCamera(const HPScamera *camera, const float *a, const float *b);

int hpsFurtherFromCameraRough(const HPScamera *camera, const float *a, const float *b);

int hpsBSCloserToCamera(const HPScamera *camera, const float *a, const float *b);

int hpsBSFurtherFromCamera(const HPScamera *camera, const float *a, const float *b);

int hpsBSFurtherFromCameraRough(const HPScamera *camera, const float *a, const float *b);
