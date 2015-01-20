#ifndef HYPERMATH
#define HYPERMATH 1

typedef struct {
    float _11, _21, _31, _41,
          _12, _22, _32, _42,
	  _13, _23, _33, _43,
	  _14, _24, _34, _44;
} HPMmat4;

typedef struct {
    float x, y, z, w;
} HPMquat;

typedef struct {
    float x, y, z;
} HPMpoint;

// Angle operations
float hpmDegreesToRadians(float deg);

float hpmRadiansToDegrees(float rad);

// Vector operations
void hpmCopyVec(const float *source, float *dest);

void hpmMultVec(const float *pointA, float m, float *result);

void hpmAddVec(const float *pointA, const float *pointB, float *result);

void hpmSubVec(const float *pointA, const float *pointB, float *result);

void hpmCross(const float *pointA, const float *pointB, float *result);

float hpmMagnitude(const float *point);

void hpmNormalize(float *point);

float hpmDot(const float *pointA, const float *pointB);

void hpmLerp(const float *pointA, const float *pointB, float t, float *result);

void hpmMat4VecMult(const float *mat, float *point);

void hpmMat4VecArrayMult(const float *mat, float *vec, size_t length, size_t stride);


// Quaternion operations
void hpmCopyQuat(const float *source, float *dest);

void hpmQuatNormalize(float *quat);

void hpmQuatInverse(const float *quat, float *inv);

void hpmQuatCross(const float *quatA, const float *quatB, float *result);

void hpmQuatVecRotate(const float *quat, float *point);

void hpmAxisAngleQuatRotation(float *axis, float angle, float *quat);

void hpmRotateQuatAxisAngle(float *axis, float angle, float *quat);

void hpmXQuatRotation(float angle, float *quat);

void hpmRotateQuatX(float angle, float *quat);

void hpmYQuatRotation(float angle, float *quat);

void hpmRotateQuatY(float angle, float *quat);

void hpmZQuatRotation(float angle, float *quat);

void hpmRotateQuatZ(float angle, float *quat);

void hpmYPRQuatRotation(float yaw, float pitch, float roll, float *quat);

void hpmRotateQuatYPR(float yaw, float pitch, float roll, float *quat);

void hpmSlerp(const float *quatA, const float *quatB, float t, float *result);

// Matrix operations
void hpmCopyMat4(const float *source, float *dest);

void hpmMultMat4(const float *matA, const float *matB, float *result);

void hpmPrintMat4(const float *m);

void hpmIdentityMat4(float *m);

void hpmTranslation(float *vector, float *mat);

void hpmTranslate(float *vector, float *mat);

void hpmXRotation(float rotation, float *mat);

void hpmRotateX(float rotation, float *mat);

void hpmYRotation(float rotation, float *mat);

void hpmRotateY(float rotation, float *mat);

void hpmZRotation(float rotation, float *mat);

void hpmRotateZ(float rotation, float *mat);

void hpmAxisAngleRotation(float *axis, float angle, float *mat);

void hpmRotateAxisAngle(float *axis, float angle, float *mat);

void hpmQuaternionRotation(float* quat, float *mat);

void hpmRotateQuaternion(float *quat, float *mat);

void hpmYPRRotation(float yaw, float pitch, float roll, float *mat);

void hpmRotateYPR(float yaw, float pitch, float roll, float *mat);

void hpm2DScaling(float scaleX, float scaleY, float *mat);

void hpmScale2D(float scaleX, float scaleY, float *mat);

void hpm3DScaling(float scaleX, float scaleY, float scaleZ, float *mat);

void hpmScale3D(float scaleX, float scaleY, float scaleZ, float *mat);

void hpmScaling(float factor, float *mat);

void hpmScale(float factor, float *mat);

void hpmFlipX(float *mat);

void hpmFlipY(float *mat);

void hpmFlipZ(float *mat);

void hpmTranslateRotateScale2D(float *point, float angle, float scale, float *mat);

void hpmTranspose(const float *mat, float *result);

void hpmInverse(const float *mat, float *result);

void hpmFastInverseTranspose(const float *mat, float *result);

// Projection
void hpmOrtho(int width, int height, float near, float far, float *mat);

void hpmOrthoViewport(float left, float right, float bottom, float top,
                      float near, float far,
                      float vLeft, float vRight, float vBottom, float vTop,
                      float *mat);

void hpmPerspective(int width, int height, float near, float far, float angle, float *mat);

void hpmFrustum(float left, float right, float bottom, float top,
		float near, float far, float *mat);

void hpmFrustumViewport(float left, float right, float bottom, float top,
                        float near, float far,
                        float vLeft, float vRight, float vBottom, float vTop,
                        float *mat);

// Camera
void hpmLookAt(float *eye, float *obj, float *up, float *mat);

void hpmCameraInverse(const float *camera, float *inverse);

#endif
