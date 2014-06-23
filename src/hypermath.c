#include <stdio.h>
#include <math.h>
#include <string.h>
#include <hypermath.h>

// Angle operations
float hpmDegreesToRadians(float deg){
    return deg * 0.0174532925;
}

float hpmRadiansToDegrees(float rad){
    return rad * 57.2957795;
}

// Vector operations
void hpmCross(float ax, float ay, float az, float bx, float by, float bz,
              float *rx, float *ry, float *rz){
    *rx = ay*bz - az*by;
    *ry = az*bx - ax*bz;
    *rz = ax*by - ay*bx;
}

void hpmNormalize(float x, float y, float z, float *rx, float *ry, float *rz){
    float len = sqrt(x*x + y*y + z*z);
    *rx = x / len;
    *ry = y / len;
    *rz = z / len;
}

float hpmDot(float ax, float ay, float az, float bx, float by, float bz){
    return ax*bx + ay*by + az*bz;
}

// Matrix operations
static void initMat4(HPMmat4 *m){
  memset(m, 0, sizeof(HPMmat4));
}

static void copyMat4(float *dest, const float *source){
    memcpy(dest, source, sizeof(float) * 16);
}

void hpmPrintMat4(const float *mat){
    HPMmat4 *m = (HPMmat4 *) mat;
    printf("[%f, %f, %f, %f,\n",   m->_11, m->_12, m->_13, m->_14);
    printf(" %f, %f, %f, %f,\n",   m->_21, m->_22, m->_23, m->_24);
    printf(" %f, %f, %f, %f,\n",   m->_31, m->_32, m->_33, m->_34);
    printf(" %f, %f, %f, %f]\n\n", m->_41, m->_42, m->_43, m->_44);
}

void hpmIdentityMat4(float *mat){
    HPMmat4 *m = (HPMmat4 *) mat;
    memset(m, 0, sizeof(HPMmat4));
    m->_11 = 1.0;
    m->_22 = 1.0;
    m->_33 = 1.0;
    m->_44 = 1.0;
}

void hpmMultMat4(const float *matA, const float *matB, float *result){
    HPMmat4 *a = (HPMmat4 *) matA;
    HPMmat4 *b = (HPMmat4 *) matB;
    HPMmat4 *r = (HPMmat4 *) result;
    r->_11 = a->_11*b->_11 + a->_12*b->_21 + a->_13*b->_31 + a->_14*b->_41;
    r->_12 = a->_11*b->_12 + a->_12*b->_22 + a->_13*b->_32 + a->_14*b->_42;
    r->_13 = a->_11*b->_13 + a->_12*b->_23 + a->_13*b->_33 + a->_14*b->_43;
    r->_14 = a->_11*b->_14 + a->_12*b->_24 + a->_13*b->_34 + a->_14*b->_44;

    r->_21 = a->_21*b->_11 + a->_22*b->_21 + a->_23*b->_31 + a->_24*b->_41;
    r->_22 = a->_21*b->_12 + a->_22*b->_22 + a->_23*b->_32 + a->_24*b->_42;
    r->_23 = a->_21*b->_13 + a->_22*b->_23 + a->_23*b->_33 + a->_24*b->_43;
    r->_24 = a->_21*b->_14 + a->_22*b->_24 + a->_23*b->_34 + a->_24*b->_44;

    r->_31 = a->_31*b->_11 + a->_32*b->_21 + a->_33*b->_31 + a->_34*b->_41;
    r->_32 = a->_31*b->_12 + a->_32*b->_22 + a->_33*b->_32 + a->_34*b->_42;
    r->_33 = a->_31*b->_13 + a->_32*b->_23 + a->_33*b->_33 + a->_34*b->_43;
    r->_34 = a->_31*b->_14 + a->_32*b->_24 + a->_33*b->_34 + a->_34*b->_44;

    r->_41 = a->_41*b->_11 + a->_42*b->_21 + a->_43*b->_31 + a->_44*b->_41;
    r->_42 = a->_41*b->_12 + a->_42*b->_22 + a->_43*b->_32 + a->_44*b->_42;
    r->_43 = a->_41*b->_13 + a->_42*b->_23 + a->_43*b->_33 + a->_44*b->_43;
    r->_44 = a->_41*b->_14 + a->_42*b->_24 + a->_43*b->_34 + a->_44*b->_44;
}

void hpmTranslation(float x, float y, float z, float *mat){
    hpmIdentityMat4(mat);
    HPMmat4 *m = (HPMmat4 *) mat;
    m->_14 += x;
    m->_24 += y;
    m->_34 += z;
}

void hpmTranslate(float x, float y, float z, float *mat){
    float trans[16], r[16];
    hpmTranslation(x, y, z, trans);
    hpmMultMat4(trans, mat, r);
    copyMat4(mat, r);
}

void hpmXRotation(float rotation, float *mat){
    HPMmat4 *m = (HPMmat4 *) mat;
    initMat4(m);
    float c = cos(rotation);
    float s = sin(rotation);
    m->_11 = 1.0;
    m->_22 = c;
    m->_33 = c;
    m->_23 = -s;
    m->_32 = s;
    m->_44 = 1.0;
}

void hpmRotateX(float rotation, float *mat){
    if (rotation == 0.0) return;
    float rot[16], r[16];
    hpmXRotation(rotation, rot);
    hpmMultMat4(rot, mat, r);
    copyMat4(mat, r);
}

void hpmYRotation(float rotation, float *mat){
    HPMmat4 *m = (HPMmat4 *) mat;
    initMat4(m);
    float c = cos(rotation);
    float s = sin(rotation);
    m->_11 = c;
    m->_22 = 1.0;
    m->_33 = c;
    m->_13 = s;
    m->_31 = -s;
    m->_44 = 1.0;
}

void hpmRotateY(float rotation, float *mat){
    if (rotation == 0.0) return;
    float rot[16], r[16];
    hpmYRotation(rotation, rot);
    hpmMultMat4(rot, mat, r);
    copyMat4(mat, r);
}

void hpmZRotation(float rotation, float *mat){
    HPMmat4 *m = (HPMmat4 *) mat;
    initMat4(m);
    float c = cos(rotation);
    float s = sin(rotation);
    m->_11 = c;
    m->_22 = c;
    m->_12 = -s;
    m->_21 = s;
    m->_33 = 1.0;
    m->_44 = 1.0;
}

void hpmRotateZ(float rotation, float *mat){
    if (rotation == 0.0) return;
    float rot[16], r[16];
    hpmZRotation(rotation, rot);
    hpmMultMat4(rot, mat, r);
    copyMat4(mat, r);
}

void hpmRotation(float x, float y, float z, float angle, float *mat){
    HPMmat4 *m = (HPMmat4 *) mat;
    initMat4(m);
    float c = cos(angle);
    float s = sin(angle);
    float C = 1 - c;
    float xx, xy, xz, yy, yz, zz, xs, ys, zs;
    hpmNormalize(x, y, z, &x, &y, &z);
    xx = x * x;
    xy = x * y;
    xz = x * z;
    yy = y * y;
    yz = y * z;
    zz = z * z;
    xs = x * s;
    ys = y * s;
    zs = z * s;
    
    m->_11 = xx*C + c;
    m->_12 = xy*C - zs;
    m->_13 = xz*C + ys;

    m->_21 = xy*C + zs;
    m->_22 = yy*C + c;
    m->_23 = yz*C - xs;

    m->_31 = xz*C - ys;
    m->_32 = yz*C + xs;
    m->_33 = zz*C + c;
    
    m->_44 = 1.0;
}

void hpmRotate(float x, float y, float z, float angle, float *mat){
    if (angle == 0.0) return;
    float rot[16], r[16];
    hpmRotation(x, y, z, angle, rot);
    hpmMultMat4(rot, mat, r);
    copyMat4(mat, r);
}

void hpmQuaternionRotation(float x, float y, float z, float w, float *mat){
    HPMmat4 *m = (HPMmat4 *) mat;
    initMat4(m);
    float xx, xy, xz, xw, yy, yz, yw, zz, zw;
    xx = x * x;
    xy = x * y;
    xz = x * z;
    xw = x * w;
    yy = y * y;
    yz = y * z;
    yw = y * w;
    zz = z * z;
    zw = z * w;
    m->_11 = 1.0 - (2.0 * (zz + yy));
    m->_12 = 2.0 * (xy - zw);
    m->_13 = 2.0 * (xz + yw);

    m->_21 = 2.0 * (xy + zw);
    m->_22 = 1.0 - (2.0 * (xx + zz));
    m->_23 = 2.0 * (yz - xw);

    m->_31 = 2.0 * (xz - yw);
    m->_32 = 2.0 * (yz + xw);
    m->_33 = 1.0 - (2.0 * (xx + yy));
    
    m->_44 = 1.0;
}

void hpmRotateQuaternion(float x, float y, float z, float w, float *mat){
    float rot[16], r[16];
    hpmQuaternionRotation(x, y, z, w, rot);
    hpmMultMat4(rot, mat, r);
    copyMat4(mat, r);
}

void hpmYPRRotation(float yaw, float pitch, float roll, float *mat){
    HPMmat4 *m = (HPMmat4 *) mat;
    initMat4(m);
    float sy, cy, sp, cp, sr, cr;
    sy = sin(yaw); cy = cos(yaw);
    sp = sin(pitch); cp = cos(pitch);
    sr = sin(roll); cr = cos(roll);

    m->_11 = sp*sy*sr + cy*cr;
    m->_12 = sp*sy*cr - cy*sr;
    m->_13 = cp*sy;

    m->_21 = cp*sr;
    m->_22 = cp*cr;
    m->_23 = -sp;

    m->_31 = sp*cy*sr - sy*cr;
    m->_32 = sp*cy*cr + sy*sr;
    m->_33 = cp*cy;

    m->_44 = 1.0;
}

void hpmRotateYPR(float yaw, float pitch, float roll, float *mat){
    float rot[16], r[16];
    hpmYPRRotation(yaw, pitch, roll, rot);
    hpmMultMat4(rot, mat, r);
    copyMat4(mat, r);
}

void hpm2DScaling(float scaleX, float scaleY, float *mat){
    HPMmat4 *m = (HPMmat4 *) mat;
    initMat4(m);
    m->_11 = scaleX;
    m->_22 = scaleY;
    m->_33 = 1.0;
    m->_44 = 1.0;
}
void hpmScale2D(float scaleX, float scaleY, float *mat){
    float scale[16], r[16];
    hpm2DScaling(scaleX, scaleY, scale);
    hpmMultMat4(scale, mat, r);
    copyMat4(mat, r);
}

void hpm3DScaling(float scaleX, float scaleY, float scaleZ, float *mat){
    HPMmat4 *m = (HPMmat4 *) mat;
    initMat4(m);
    m->_11 = scaleX;
    m->_22 = scaleY;
    m->_33 = scaleZ;
    m->_44 = 1.0;
}
void hpmScale3D(float scaleX, float scaleY, float scaleZ, float *mat){
    float scale[16], r[16];
    hpm3DScaling(scaleX, scaleY, scaleZ, scale);
    hpmMultMat4(scale, mat, r);
    copyMat4(mat, r);
}

void hpmScaling(float factor, float *mat){
    HPMmat4 *m = (HPMmat4 *) mat;
    initMat4(m);
    m->_11 = factor;
    m->_22 = factor;
    m->_33 = factor;
    m->_44 = 1.0;
}

void hpmScale(float factor, float *mat){
    float scale[16], r[16];
    hpmScaling(factor, scale);
    hpmMultMat4(scale, mat, r);
    copyMat4(mat, r);
}

void hpmFlipX(float *mat){
    HPMmat4 *m = (HPMmat4 *) mat;
    m->_22 = -m->_22;
}

void hpmFlipY(float *mat){
    HPMmat4 *m = (HPMmat4 *) mat;
    m->_11 = -m->_11;
}

void hpmFlipZ(float *mat){
    HPMmat4 *m = (HPMmat4 *) mat;
    m->_33 = -m->_33;
}

void hpmTranslateRotateScale2D(float x, float y, float z, float angle, float scale,
			        float *mat){
    HPMmat4 *m = (HPMmat4 *) mat;
    initMat4(m);
    float c = scale * cos(angle);
    float s = scale * sin(angle);
    m->_11 = c;
    m->_22 = c;
    m->_12 = -s;
    m->_21 = s;
    m->_33 = 1.0;
    m->_44 = 1.0;
    m->_14 = x;
    m->_24 = y;
    m->_34 = z;
}

void hpmTranspose(const float *mat, float *result){
    HPMmat4 *m = (HPMmat4 *) mat;
    HPMmat4 *r = (HPMmat4 *) result;
    r->_11 = m->_11;
    r->_12 = m->_21;
    r->_13 = m->_31;
    r->_14 = m->_41;

    r->_21 = m->_12;
    r->_22 = m->_22;
    r->_23 = m->_32;
    r->_24 = m->_42;

    r->_31 = m->_13;
    r->_32 = m->_23;
    r->_33 = m->_33;
    r->_34 = m->_43;

    r->_41 = m->_14;
    r->_42 = m->_24;
    r->_43 = m->_34;
    r->_44 = m->_44;
}

void hpmInverse(const float *mat, float *result){
    HPMmat4 *m = (HPMmat4 *) mat;
    HPMmat4 inv;
    float det;
    int i;

    inv._11 = m->_22  * m->_33 * m->_44 - 
        m->_22  * m->_34 * m->_43 - 
        m->_32  * m->_23  * m->_44 + 
        m->_32  * m->_24  * m->_43 +
        m->_42 * m->_23  * m->_34 - 
        m->_42 * m->_24  * m->_33;

    inv._21 = -m->_21  * m->_33 * m->_44 + 
        m->_21  * m->_34 * m->_43 + 
        m->_31  * m->_23  * m->_44 - 
        m->_31  * m->_24  * m->_43 - 
        m->_41 * m->_23  * m->_34 + 
        m->_41 * m->_24  * m->_33;

    inv._31 = m->_21  * m->_32 * m->_44 - 
        m->_21  * m->_34 * m->_42 - 
        m->_31  * m->_22 * m->_44 + 
        m->_31  * m->_24 * m->_42 + 
        m->_41 * m->_22 * m->_34 - 
        m->_41 * m->_24 * m->_32;

    inv._41 = -m->_21  * m->_32 * m->_43 + 
        m->_21  * m->_33 * m->_42 +
        m->_31  * m->_22 * m->_43 - 
        m->_31  * m->_23 * m->_42 - 
        m->_41 * m->_22 * m->_33 + 
        m->_41 * m->_23 * m->_32;

    inv._12 = -m->_12  * m->_33 * m->_44 + 
        m->_12  * m->_34 * m->_43 + 
        m->_32  * m->_13 * m->_44 - 
        m->_32  * m->_14 * m->_43 - 
        m->_42 * m->_13 * m->_34 + 
        m->_42 * m->_14 * m->_33;

    inv._22 = m->_11  * m->_33 * m->_44 - 
        m->_11  * m->_34 * m->_43 - 
        m->_31  * m->_13 * m->_44 + 
        m->_31  * m->_14 * m->_43 + 
        m->_41 * m->_13 * m->_34 - 
        m->_41 * m->_14 * m->_33;

    inv._32 = -m->_11  * m->_32 * m->_44 + 
        m->_11  * m->_34 * m->_42 + 
        m->_31  * m->_12 * m->_44 - 
        m->_31  * m->_14 * m->_42 - 
        m->_41 * m->_12 * m->_34 + 
        m->_41 * m->_14 * m->_32;

    inv._42 = m->_11  * m->_32 * m->_43 - 
        m->_11  * m->_33 * m->_42 - 
        m->_31  * m->_12 * m->_43 + 
        m->_31  * m->_13 * m->_42 + 
        m->_41 * m->_12 * m->_33 - 
        m->_41 * m->_13 * m->_32;

    inv._13 = m->_12  * m->_23 * m->_44 - 
        m->_12  * m->_24 * m->_43 - 
        m->_22  * m->_13 * m->_44 + 
        m->_22  * m->_14 * m->_43 + 
        m->_42 * m->_13 * m->_24 - 
        m->_42 * m->_14 * m->_23;

    inv._23 = -m->_11  * m->_23 * m->_44 + 
        m->_11  * m->_24 * m->_43 + 
        m->_21  * m->_13 * m->_44 - 
        m->_21  * m->_14 * m->_43 - 
        m->_41 * m->_13 * m->_24 + 
        m->_41 * m->_14 * m->_23;

    inv._33 = m->_11  * m->_22 * m->_44 - 
        m->_11  * m->_24 * m->_42 - 
        m->_21  * m->_12 * m->_44 + 
        m->_21  * m->_14 * m->_42 + 
        m->_41 * m->_12 * m->_24 - 
        m->_41 * m->_14 * m->_22;

    inv._43 = -m->_11  * m->_22 * m->_43 + 
        m->_11  * m->_23 * m->_42 + 
        m->_21  * m->_12 * m->_43 - 
        m->_21  * m->_13 * m->_42 - 
        m->_41 * m->_12 * m->_23 + 
        m->_41 * m->_13 * m->_22;

    inv._14 = -m->_12 * m->_23 * m->_34 + 
        m->_12 * m->_24 * m->_33 + 
        m->_22 * m->_13 * m->_34 - 
        m->_22 * m->_14 * m->_33 - 
        m->_32 * m->_13 * m->_24 + 
        m->_32 * m->_14 * m->_23;

    inv._24 = m->_11 * m->_23 * m->_34 - 
        m->_11 * m->_24 * m->_33 - 
        m->_21 * m->_13 * m->_34 + 
        m->_21 * m->_14 * m->_33 + 
        m->_31 * m->_13 * m->_24 - 
        m->_31 * m->_14 * m->_23;

    inv._34 = -m->_11 * m->_22 * m->_34 + 
        m->_11 * m->_24 * m->_32 + 
        m->_21 * m->_12 * m->_34 - 
        m->_21 * m->_14 * m->_32 - 
        m->_31 * m->_12 * m->_24 + 
        m->_31 * m->_14 * m->_22;

    inv._44 = m->_11 * m->_22 * m->_33 - 
        m->_11 * m->_23 * m->_32 - 
        m->_21 * m->_12 * m->_33 + 
        m->_21 * m->_13 * m->_32 + 
        m->_31 * m->_12 * m->_23 - 
        m->_31 * m->_13 * m->_22;

    det = m->_11 * inv._11 + m->_12 * inv._21 + m->_13 * inv._31 + m->_14 * inv._41;
    det = 1.0 / det;

    float *in = (float *) &inv;

    if (det == 0){
        for (i = 0; i < 16; i++)
            result[i] = 0;
    } else {
        for (i = 0; i < 16; i++)
            result[i] = in[i] * det;
    }
}

//
// Matrix creation
//

// Projection
void hpmOrtho(int width, int height, float near, float far, float *mat){
    HPMmat4 *m = (HPMmat4 *) mat;
    initMat4(m);
    m->_11 = 2.0 / (float) width;
    m->_22 = 2.0 / (float) height;
    m->_33 = -2 / (far - near);
    m->_34 = -(far + near) / (far - near);
    m->_44 = 1.0;
}

void hpmFrustum(float left, float right, float bottom, float top,
		float near, float far, float *mat){
    HPMmat4 *m = (HPMmat4 *) mat;
    initMat4(m);
    m->_11 = 2.0 * near / (right - left);
    m->_13 =  (right + left) / (right - left);
    m->_22 = 2.0 * near / (top - bottom);
    m->_23 = (top + bottom) / (top - bottom);
    m->_33 = -(far + near) / (far - near);
    m->_34 = - (2.0 * far * near) / (far - near);
    m->_43 = -1.0;
}

void hpmPerspective(int width, int height, float near, float far, float angle,
		    float *mat){
    float scale = tan(hpmDegreesToRadians(angle * 0.5)) * near;
    float r = ((float) width / (float) height) * scale;
    float l = -r;
    float t = scale;
    float b = -t;
    hpmFrustum(l, r, b, t, near, far, mat);
}

// Camera
void hpmLookAt(float eyeX, float eyeY, float eyeZ, float x, float y, float z, float upX, float upY, float upZ, float *mat){
    float fx, fy, fz, ux, uy, uz, rx, ry, rz;
    HPMmat4 *m = (HPMmat4 *) mat;
    initMat4(m);
    hpmNormalize(eyeX - x, eyeY - y, eyeZ - z, &fx, &fy, &fz);
    hpmCross(upX, upY, upZ, fx, fy, fz, &rx, &ry, &rz);
    hpmNormalize(rx, ry, rz, &rx, &ry, &rz);
    hpmCross(fx, fy, fz, rx, ry, rz, &ux, &uy, &uz);

    m->_11 = rx;
    m->_12 = ry;
    m->_13 = rz;
    m->_21 = ux;
    m->_22 = uy;
    m->_23 = uz;
    m->_31 = fx;
    m->_32 = fy;
    m->_33 = fz;
    m->_14 =-hpmDot(rx, ry, rz, eyeX, eyeY, eyeZ);
    m->_24 =-hpmDot(ux, uy, uz, eyeX, eyeY, eyeZ);
    m->_34 =-hpmDot(fx, fy, fz, eyeX, eyeY, eyeZ);
    m->_44 = 1.0;
}

/*
  http://ksimek.github.io/2012/08/22/extrinsic/
*/
void hpmCameraInverse(const float *camera, float *inverse){
    HPMmat4 *cam = (HPMmat4 *) camera;
    HPMmat4 *inv = (HPMmat4 *) inverse;
    // Rotation component
    inv->_11 = cam->_11;
    inv->_12 = cam->_21;
    inv->_13 = cam->_31;

    inv->_21 = cam->_12;
    inv->_22 = cam->_22;
    inv->_23 = cam->_32;

    inv->_31 = cam->_13;
    inv->_32 = cam->_23;
    inv->_33 = cam->_33;
    // Translation component
    inv->_14 = -(cam->_11*cam->_14 + cam->_21*cam->_24 + cam->_31*cam->_34);
    inv->_24 = -(cam->_12*cam->_14 + cam->_22*cam->_24 + cam->_32*cam->_34);
    inv->_34 = -(cam->_13*cam->_14 + cam->_23*cam->_24 + cam->_33*cam->_34);
    // Last row
    inv->_41 = 0;
    inv->_42 = 0;
    inv->_43 = 0;
    inv->_44 = 1;
}
