# Hypermath
A very small math library for gamedev that mostly provides 4x4 float matrix operations.

## Installation
`make install` will install libhypermath in the `PREFIX` defaulting to `/usr/local`.

## Requirements
None

## Documentation
All matrices must be arrays of 16 floats.

### Matrix operations
    void hpmCopyMat4(const float *source, float *dest);

Copy the data from matrix `source` into `dest`.

    void hpmPrintMat4(const float *m);
Print the given 4x4 matrix.

    void hpmMultMat4(const float *A, const float *B, float *result);
Multiply matrix `A` and `B` into `results`.

    void hpmIdentityMat4(float *m);
Turn the given matrix into an identity matrix.
    
    void hpmTranslation(float x, float y, float z, float *mat);
Create the translation matrix given by `x`, `y`, and `z` in the given matrix.

    void hpmTranslate(float x, float y, float z, float *mat);
Multiply the given matrix by the translation matrix created with `x`, `y`, and `z`.

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

    void hpmRotation(float x, float y, float z, float angle, float *mat);
Create the rotation matrix of `angle` radians around the vector `(x, y, z)` in the given matrix.

    void hpmRotate(float x, float y, float z, float angle, float *mat);
Rotate the given matrix around the vector given by `x`, `y`, and `z` by `angle` radians.

    void hpmQuaternionRotation(float x, float y, float z, float w, float *mat);
Create the rotation matrix given by the quaternion `(x, y, z, q)` in the given matrix.

    void hpmRotateQuaternion(float x, float y, float z, float w, float *mat);
Rotate the given matrix around the quaternion `(x, y, z, q)`.

    void hpmYPRRotation(float yaw, float pitch, float roll, float *mat);
Create the rotation matrix given by transforming by `roll` radians followed by `pitch` radians followed by `yaw` radians in the given matrix.

    void hpmRotateYPR(float yaw, float pitch, float roll, float *mat);
Rotate the given matrix by transforming by `roll` radians followed by `pitch` radians followed by `yaw` radians.

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

    void hpmTranslateRotateScale2D(float x, float y, float z, float angle, float scale, float *mat);
Efficiently create a matrix translated by `x`, `y`, and `z`, rotated around the z-axis by `angle` then scaled by `scale`. 

    void hpmTranspose(const float *mat, float *result);
Transpose the given matrix into `result`

    void hpmInverse(const float *mat, float *result);
Invert the given matrix into `result`

### Projection
    void hpmOrtho(int width, int height, float near, float far, float *mat);
Create an orthographic projection matrix.

    void hpmPerspective(int width, int height, float near, float far, float angle, float *mat);
Create an perspective projection matrix. 

    void hpmFrustum(float left, float right, float bottom, float top, float near, float far, float *mat);
Create a view-frustum matrix.

### Camera
    void hpmLookAt(float eyeX, float eyeY, float eyeZ, float x, float y, float z, float upX, float upY, float upZ, float *mat);
Create a “look-at” style camera matrix. The camera is positioned at `(eyeX, eyeY, eyeZ)`, pointing towards `(x, y, z)`. `(upX, upY, upZ)` defines the camera’s up vector.

    void hpmCameraInverse(const float *camera, float *inverse);
Invert `camera` in an efficient fashion. This allows the camera to be constructed in an intuitive fashion by translating and rotating before inverting in order to position the scene properly. This function is far faster than the general `hpmInverse` function, but the matrix `camera` must only be a matrix representing a translation and a rotation (no scaling).

### Vector operations
    void hpmCross(float ax, float ay, float az, float bx, float by, float bz, float *rx, float *ry, float *rz);
Return the result of the cross product between the vectors `(ax, ay, az)` and `(bx, by, bz)`. The result is returned in `rx`, `ry`, and `rz`.

    float hpmDot(float ax, float ay, float az, float bx, float by, float bz);
Return the result of the dot product between the vectors `(ax, ay, az)` and `(bx, by, bz)`.

    void hpmNormalize(float x, float y, float z, float *rx, float *ry, float *rz);
Return the normalized vector `(x, y, z)`. The result is returned in `rx`, `ry`, and `rz`.

    void hpmMat4VecMult(const float *matrix, float *vec);
Multiply the 3 element vector by `matrix`, modifying it.

    void hpmMat4VecArrayMult(const float *matrix, float *vectorArray, size_t length, size_t stride);
Multiply each 3 element vector in `vectorArray` by `matrix`. `length` specifies the number of vectors in `vectorArray`. `stride` specifies the number of bytes between the start of two vectors. If `stride` is `0`, the vectors are assumed to be tightly packed.


### Angle operations
    float hpmDegreesToRadians(float deg);
Convert degrees into radians.

    float hpmRadiansToDegrees(float rad);
Convert radians into degrees.


## Version history
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
