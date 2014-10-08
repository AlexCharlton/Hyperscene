# Hypermath
A small math library aimed at gamedev that provides 4x4 float matrix, vector, and quaternion operations. Designed to be easy to interface with other languages.

## Installation
`make install` will install libhypermath in the `PREFIX` defaulting to `/usr/local`.

## Requirements
None

## Documentation
All matrices must be arrays of 16 floats (with sequential numbers representing a column), all vectors are arrays of 3 floats (`(x, y, z)`), and quaternions are arrays of 4 floats (`(x, y, z, q)`). While this setup does not provide the benefits of type checking and makes it awkward to access data if you wish to go through the structs, it was chosen for ease of foreign interfacing.

### Structs

```C
typedef struct {
    float _11, _21, _31, _41,
          _12, _22, _32, _42,
          _13, _23, _33, _43,
          _14, _24, _34, _44;
} HPMmat4;
```

```C
typedef struct {
    float x, y, z, w;
} HPMquat;
```

```C
typedef struct {
    float x, y, z;
} HPMpoint;
```

### Matrix operations
    void hpmCopyMat4(const float *source, float *dest);

Copy the data from matrix `source` into `dest`.

    void hpmPrintMat4(const float *m);
Print the given 4x4 matrix.

    void hpmMultMat4(const float *A, const float *B, float *result);
Multiply matrix `A` and `B` into `results`.

    void hpmIdentityMat4(float *m);
Turn the given matrix into an identity matrix.
    
    void hpmTranslation(float *vector, float *mat);
Create the translation matrix given by vector `(x, y, z)` in the given matrix.

    void hpmTranslate(float *vector, float *mat);
Multiply the given matrix by the translation matrix created with `(x, y, z)`.

    void hpmXRotation(float rotation, float *mat);
Create the rotation matrix of `rotation` radians around the X-axis in the given matrix.

    void hpmRotateX(float rotation, float *mat);
Rotate the given matrix around the X-axis by `rotation` radians.

    void hpmYRotation(float rotation, float *mat);
Create the rotation matrix of `rotation` radians around the Y-axis in the given matrix.

    void hpmRotateY(float rotation, float *mat);
Rotate the given matrix around the Y-axis by `rotation` radians.

    void hpmZRotation(float rotation, float *mat);
Create the rotation matrix of `rotation` radians around the Z-axis in the given matrix.

    void hpmRotateZ(float rotation, float *mat);
Rotate the given matrix around the Z-axis by `rotation` radians.

    void hpmAxisAngleRotation(float *axis, float angle, float *mat);
Create the rotation matrix of `angle` radians around the vector given by `axis` in the given matrix.

    void hpmRotateAxisAngle(float *axis, float angle, float *mat);
Rotate the given matrix around the vector given by `axis` by `angle` radians.

    void hpmQuaternionRotation(float *quat, float *mat);
Create the rotation matrix given by the quaternion `quat` in the given matrix.

    void hpmRotateQuaternion(float *quat, float *mat);
Rotate the given matrix around the quaternion `quat`.

    void hpmYPRRotation(float yaw, float pitch, float roll, float *mat);
Create the rotation matrix given by rotating by `roll` radians around the z-axis followed by `pitch` radians around the x-axis followed by `yaw` radians around the y-axis.

    void hpmRotateYPR(float yaw, float pitch, float roll, float *mat);
Rotate the given matrix by `roll` radians around the z-axis followed by `pitch` radians around the x-axis followed by `yaw` radians around the y-axis.

    void hpm2DScaling(float scaleX, float scaleY, float *mat);
Create the scaling matrix created by multiplying the x and y axis by `scaleX` and `scaleY` in the given matrix.

    void hpmScale2D(float scaleX, float scaleY, float *mat);
Scale the x and y axis of the given matrix by `scaleX` and `scaleY`.

    void hpm3DScaling(float scaleX, float scaleY, float scaleZ, float *mat);
Create the scaling matrix created by multiplying the x, y and z axis by `scaleX`, `scaleY`, and `scaleZ` in the given matrix.

    void hpmScale3D(float scaleX, float scaleY, float scaleZ, float *mat);
Scale the x, y, and z axis of the given matrix by `scaleX`, `scaleY`, and `scaleZ`.

    void hpmScaling(float factor, float *mat);
Create the scaling matrix created by multiplying the x, y and z axis by `factor` in the given matrix.

    void hpmScale(float scale, float *mat);
Scale the x, y, and z axis of the given matrix by `scale`.

    void hpmFlipX(float *mat);
Flip (mirror) the given matrix along the x-axis.

    void hpmFlipY(float *mat);
Flip (mirror) the given matrix along the y-axis.

    void hpmFlipZ(float *mat);
Flip (mirror) the given matrix along the z-axis.

    void hpmTranslateRotateScale2D(float *vec, float angle, float scale, float *mat);
Efficiently create a matrix translated by `vec`, rotated around the z-axis by `angle` then scaled by `scale`. 

    void hpmTranspose(const float *mat, float *result);
Transpose the given matrix into `result`

    void hpmInverse(const float *mat, float *result);
Invert the given matrix into `result`

    void hpmFastInverseTranspose(const float *mat, float *result);
Inverse then transpose the given matrix into `result` much faster than if `hpmInverse` and `hpmTranspose` were used. This will not produce correct results on matrices that have been scaled. Instead `hpmInverse` and `hpmTranspose` should be used.


### Projection
    void hpmOrtho(int width, int height, float near, float far, float *mat);
Create an orthographic projection matrix.

    void hpmPerspective(int width, int height, float near, float far, float angle, float *mat);
Create an perspective projection matrix. 

    void hpmFrustum(float left, float right, float bottom, float top, float near, float far, float *mat);
Create a view-frustum matrix.

### Camera
    void hpmLookAt(float *eye, float *cam, float *up, float *mat);
Create a “look-at” style camera matrix. The camera is positioned at `eye`, pointing towards `obj`. `up` defines the camera’s up vector.

    void hpmCameraInverse(const float *camera, float *inverse);
Invert `camera` in an efficient fashion. This allows the camera to be constructed in an intuitive fashion by translating and rotating before inverting in order to position the scene properly. This function is far faster than the general `hpmInverse` function, but the matrix `camera` must only be a matrix representing a translation and a rotation (no scaling).

### Vector operations
    void hpmCopyVec(const float *source, float *dest);

Copy the vector `source` into `dest`.

    void hpmMultVec(const float *A, float m, float *result);

Multiply the vector `A` by the scalar `m` to produce `result`.

    void hpmAddVec(const float *A, const float *B, float *result);

Add vector `A` and `B` to produce `result`.

    void hpmSubVec(const float *A, const float *B, float *result);

Add vector `B` from `A` to produce `result`.

    void hpmCross(const float *A, const float *B, float *result);

Return the cross product of vector `A` and `B` in `result`.

    float hpmDot(const float *pointA, const float *pointB);

Return the dot product of vector `A` and `B`.

    float hpmMagnitude(const float *vec);

Return the magnitude of the given vector.

    void hpmNormalize(float *vec);

Normalize the given vector.

    void hpmLerp(const float *A, const float *B, float t, float *result);

Linear interpolation between the points `A` and `B` with the interpolation parameter `t` which must be between 0 and 1. Returns `result`.

    void hpmMat4VecMult(const float *matrix, float *vec);
Multiply the given vector by `matrix`, modifying it.

    void hpmMat4VecArrayMult(const float *matrix, float *vectorArray, size_t length, size_t stride);
Multiply each 3 element vector in `vectorArray` by `matrix`. `length` specifies the number of vectors in `vectorArray`. `stride` specifies the number of bytes between the start of two vectors. If `stride` is `0`, the vectors are assumed to be tightly packed.

### Quaternion operations
Quaternions are expected to be normalized before they are used in certain functions (`hpmQuatNormalize` may be used to do so). All the provided functions that create quaternions, create unit quaternions. 

The order of quaternion cross-multiplication is the inverse of the “standard” order, so a quaternion that has undergone a series or rotations will represent the same rotation as a marix that has gone through the same series, in the same order.

    void hpmCopyQuat(const float *source, float *dest);

Copy the quaternion `source` into `dest`.

    void hpmQuatNormalize(float *quat);

Normalize the given quaternion.

    void hpmQuatInverse(const float *quat, float *inv);

Return the inverse of the unit quaternion `quat` in `inv`.

    void hpmQuatCross(const float *A, const float *B, float *result);

Return the cross product of quaternions `A` and `B` in `result`.

    void hpmQuatVecRotate(const float *quat, float *point);

Rotate `point` around the unit quaternion `quat`.

    void hpmAxisAngleQuatRotation(float *axis, float angle, float *quat);

Create the unit quaternion `quat` given by a rotation of `angle` radians around the vector `axis`.

    void hpmRotateQuatAxisAngle(float *axis, float angle, float *quat);

Rotate the quaternion `quat` by `angle` radians around the vector `axis`.

    void hpmXQuatRotation(float angle, float *quat);

Create the unit quaternion `quat` given by a rotation of `angle` radians around the x-axis.

    void hpmRotateQuatX(float angle, float *quat);

Rotate the quaternion `quat` by `angle` radians around the x-axis.

    void hpmYQuatRotation(float angle, float *quat);

Create the unit quaternion `quat` given by a rotation of `angle` radians around the y-axis.

    void hpmRotateQuatY(float angle, float *quat);

Rotate the quaternion `quat` by `angle` radians around the y-axis.

    void hpmZQuatRotation(float angle, float *quat);

Create the unit quaternion `quat` given by a rotation of `angle` radians around the z-axis.

    void hpmRotateQuatZ(float angle, float *quat);

Rotate the quaternion `quat` by `angle` radians around the z-axis.

    void hpmYPRQuatRotation(float yaw, float pitch, float roll, float *quat);

Create the unit quaterion `quat` given by a rotation of `roll` radians around the z-axis followed by `pitch` radians around the x-axis followed by `yaw` radians around the y-axis.

    void hpmRotateQuatYPR(float yaw, float pitch, float roll, float *quat);

Rotate the quaterion `quat` by `roll` radians around the z-axis followed by `pitch` radians around the x-axis followed by `yaw` radians around the y-axis.

    void hpmSlerp(const float *A, const float *B, float t, float *result);

Spherical linear interpolation between the quaternions `A` and `B` with the interpolation parameter `t` which must be between 0 and 1. Returns the result in the quaternion `result`.

### Angle operations
    float hpmDegreesToRadians(float deg);
Convert degrees into radians.

    float hpmRadiansToDegrees(float rad);
Convert radians into degrees.


## Version history
### Version 0.6.0
* Add `hpmFastInverseTranspose`

### Version 0.5.0
* Add quaternion operations
* Expand vector operations, and accept vectors as arrays

### Version 0.4.0
* Export `hpmCopyMat4`

### Version 0.3.0
* Matrix vector multiplication

### Version 0.2.0
* Each transformation function now has two variants: one that initializes a matrix, and one that operates on a matrix
* Provide quaternion and YPR rotation
* Remove unhelpful composite operations
* Fix a bug in `hpmLookAt`

### Version 0.1.0
* Initial release

## Source repository
Source available on [GitHub](https://github.com/AlexCharlton/hypermath).

Bug reports and patches welcome! Bugs can be reported via GitHub or to alex.n.charlton at gmail.

## Author
Alex Charlton

## Licence
BSD
