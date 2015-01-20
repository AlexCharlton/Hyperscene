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
void hpmCopyVec(const float *source, float *dest){
    memcpy(dest, source, sizeof(float) * 3);
}

void hpmMultVec(const float *pointA, float m, float *result){
    HPMpoint *a = (HPMpoint *) pointA;
    HPMpoint *r = (HPMpoint *) result;
    r->x = a->x * m;
    r->y = a->y * m;
    r->z = a->z * m;
}

void hpmAddVec(const float *pointA, const float *pointB, float *result){
    HPMpoint *a = (HPMpoint *) pointA;
    HPMpoint *b = (HPMpoint *) pointB;
    HPMpoint *r = (HPMpoint *) result;
    r->x = a->x + b->x;
    r->y = a->y + b->y;
    r->z = a->z + b->z;
}

void hpmSubVec(const float *pointA, const float *pointB, float *result){
    HPMpoint *a = (HPMpoint *) pointA;
    HPMpoint *b = (HPMpoint *) pointB;
    HPMpoint *r = (HPMpoint *) result;
    r->x = a->x - b->x;
    r->y = a->y - b->y;
    r->z = a->z - b->z;
}

void hpmCross(const float *pointA, const float *pointB, float *result){
    HPMpoint *a = (HPMpoint *) pointA;
    HPMpoint *b = (HPMpoint *) pointB;
    HPMpoint *r = (HPMpoint *) result;
    r->x = a->y*b->z - a->z*b->y;
    r->y = a->z*b->x - a->x*b->z;
    r->z = a->x*b->y - a->y*b->x;
}

float hpmMagnitude(const float *point){
    HPMpoint *p = (HPMpoint *) point;
    return sqrt(p->x*p->x + p->y*p->y + p->z*p->z);
}

void hpmNormalize(float *point){
    HPMpoint *p = (HPMpoint *) point;
    float len = hpmMagnitude(point);
    p->x = p->x / len;
    p->y = p->y / len;
    p->z = p->z / len;
}

float hpmDot(const float *pointA, const float *pointB){
    HPMpoint *a = (HPMpoint *) pointA;
    HPMpoint *b = (HPMpoint *) pointB;
    return a->x*b->x + a->y*b->y + a->z*b->z;
}

void hpmMat4VecMult(const float *mat, float *point){
    HPMmat4 *m = (HPMmat4 *) mat;
    HPMpoint *p = (HPMpoint *) point;
    float x, y, z, w;
    x = p->x; y = p->y; z = p->z;
    w = m->_41*x + m->_42*y + m->_43*z + m->_44;

    p->x = m->_11*x + m->_12*y + m->_13*z + m->_14;
    p->y = m->_21*x + m->_22*y + m->_23*z + m->_24;
    p->z = m->_31*x + m->_32*y + m->_33*z + m->_34;

    if (w != 1.0){
        w = 1.0/w;
        p->x *= w;
        p->y *= w;
        p->z *= w;
    }
}

void hpmMat4VecArrayMult(const float *mat, float *vec, size_t length, size_t stride){
    int i;
    stride = (stride) ? stride : 3 * sizeof(float);
    for (i = 0; i < length; i++){
        hpmMat4VecMult(mat, vec);
        vec = ((float *) ((char *) vec + stride));
    }
}

void hpmLerp(const float *pointA, const float *pointB, float t, float *result){
    HPMpoint *a = (HPMpoint *) pointA;
    HPMpoint *b = (HPMpoint *) pointB;
    HPMpoint *r = (HPMpoint *) result;
    float t0 = 1 - t;
    r->x = a->x*t0 + b->x*t;
    r->y = a->y*t0 + b->y*t;
    r->z = a->z*t0 + b->z*t;
}

// Quaternion operations
void hpmCopyQuat(const float *source, float *dest){
    memcpy(dest, source, sizeof(float) * 4);
}

void hpmQuatNormalize(float *quat){
    HPMquat *q = (HPMquat *) quat;
    float mag = sqrt(q->x*q->x + q->y*q->y + q->z*q->z + q->w*q->w);
    q->x = q->x / mag;
    q->y = q->y / mag;
    q->z = q->z / mag;
    q->w = q->w / mag;
}

void hpmQuatInverse(const float *quat, float *inv){
    HPMquat *q = (HPMquat *) quat;
    HPMquat *i = (HPMquat *) inv;
    i->x = -q->x;
    i->y = -q->y;
    i->z = -q->z;
    i->w = q->w;
}

void hpmQuatCross(const float *quatA, const float *quatB, float *result){
    HPMquat *a = (HPMquat *) quatA;
    HPMquat *b = (HPMquat *) quatB;
    HPMquat *r = (HPMquat *) result;
    r->x = a->w*b->x + a->x*b->w + a->y*b->z - a->z*b->y;
    r->y = a->w*b->y + a->y*b->w + a->z*b->x - a->x*b->z;
    r->z = a->w*b->z + a->z*b->w + a->x*b->y - a->y*b->x;
    r->w = a->w*b->w - a->x*b->x - a->y*b->y - a->z*b->z;
}

void hpmQuatVecRotate(const float *quat, float *point){
    float r[4], i[4];
    HPMpoint *pt = (HPMpoint *) point;
    HPMquat p;
    p.x = pt->x;
    p.y = pt->y;
    p.z = pt->z;
    p.w = 0;
    hpmQuatInverse(quat, i);
    hpmQuatCross(i, (float *) &p, r);
    hpmQuatCross(r, quat, i);
    pt->x = i[0];
    pt->y = i[1];
    pt->z = i[2];
}

void hpmAxisAngleQuatRotation(float *axis, float angle, float *quat){
    HPMquat *q = (HPMquat *) quat;
    HPMpoint *a = (HPMpoint *) axis;
    float s = sin(angle/2.0);
    q->x = s*a->x;
    q->y = s*a->y;
    q->z = s*a->z;
    q->w = cos(angle/2.0);
}

void hpmRotateQuatAxisAngle(float *axis, float angle, float *quat){
    float rot[4], r[4];
    hpmAxisAngleQuatRotation(axis, angle, rot);
    hpmQuatCross(rot, quat, r);
    hpmCopyQuat(r, quat);
}

void hpmXQuatRotation(float angle, float *quat){
    float a[3];
    a[0] = 1; a[1] = 0; a[2] = 0;
    hpmAxisAngleQuatRotation(a, angle, quat);
}

void hpmRotateQuatX(float angle, float *quat){
    if (angle == 0.0) return;
    float rot[4], r[4];
    hpmXQuatRotation(angle, rot);
    hpmQuatCross(rot, quat, r);
    hpmCopyQuat(r, quat);
}

void hpmYQuatRotation(float angle, float *quat){
    float a[3];
    a[0] = 0; a[1] = 1; a[2] = 0;
    hpmAxisAngleQuatRotation(a, angle, quat);
}

void hpmRotateQuatY(float angle, float *quat){
    if (angle == 0.0) return;
    float rot[4], r[4];
    hpmYQuatRotation(angle, rot);
    hpmQuatCross(rot, quat, r);
    hpmCopyQuat(r, quat);
}

void hpmZQuatRotation(float angle, float *quat){
    float a[3];
    a[0] = 0; a[1] = 0; a[2] = 1;
    hpmAxisAngleQuatRotation(a, angle, quat);
}

void hpmRotateQuatZ(float angle, float *quat){
    if (angle == 0.0) return;
    float rot[4], r[4];
    hpmZQuatRotation(angle, rot);
    hpmQuatCross(rot, quat, r);
    hpmCopyQuat(r, quat);
}

void hpmYPRQuatRotation(float yaw, float pitch, float roll, float *quat){
    hpmZQuatRotation(roll, quat);
    hpmRotateQuatX(pitch, quat);
    hpmRotateQuatY(yaw, quat);
}

void hpmRotateQuatYPR(float yaw, float pitch, float roll, float *quat){
    float rot[4], r[4];
    hpmYPRQuatRotation(yaw, pitch, roll, rot);
    hpmQuatCross(rot, quat, r);
    hpmCopyQuat(r, quat);
}

void hpmSlerp(const float *quatA, const float *quatB, float t, float *result){
    HPMquat *a = (HPMquat *) quatA;
    HPMquat *b = (HPMquat *) quatB;
    HPMquat *r = (HPMquat *) result;
    float cosOmega = a->x*b->x + a->y*b->y + a->z*b->z + a->w*b->w;
    if (cosOmega < 0){
        b->x = -b->x;
        b->y = -b->y;
        b->z = -b->z;
        b->w = -b->w;
        cosOmega = -cosOmega;
    }
    float ka, kb;
    if (cosOmega > 0.9999){ // Linear interpolation when close
        ka = 1-t;
        kb = t;
    } else {
        float sinOmega = sqrt(1 - cosOmega*cosOmega);
        float omega = atan2(sinOmega, cosOmega);
        float oneOverSinOmega = 1/sinOmega;
        ka = sin((1-t) * omega) *oneOverSinOmega;
        kb = sin(t * omega) *oneOverSinOmega;
    }
    r->x = a->x*ka + b->x*kb;
    r->y = a->y*ka + b->y*kb;
    r->z = a->z*ka + b->z*kb;
    r->w = a->w*ka + b->w*kb;
}


// Matrix operations
static void initMat4(HPMmat4 *m){
  memset(m, 0, sizeof(HPMmat4));
}

void hpmCopyMat4(const float *source, float *dest){
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

void hpmTranslation(float *vector, float *mat){
    hpmIdentityMat4(mat);
    HPMmat4 *m = (HPMmat4 *) mat;
    HPMpoint *p = (HPMpoint *) vector;
    m->_14 += p->x;
    m->_24 += p->y;
    m->_34 += p->z;
}

void hpmTranslate(float *vector, float *mat){
    float trans[16], r[16];
    hpmTranslation(vector, trans);
    hpmMultMat4(trans, mat, r);
    hpmCopyMat4(r, mat);
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
    hpmCopyMat4(r, mat);
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
    hpmCopyMat4(r, mat);
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
    hpmCopyMat4(r, mat);
}

void hpmAxisAngleRotation(float *axis, float angle, float *mat){
    HPMmat4 *m = (HPMmat4 *) mat;
    HPMpoint *a = (HPMpoint *) axis;
    initMat4(m);
    float c = cos(angle);
    float s = sin(angle);
    float C = 1 - c;
    float xx, xy, xz, yy, yz, zz, xs, ys, zs;
    hpmNormalize(axis);
    xx = a->x * a->x;
    xy = a->x * a->y;
    xz = a->x * a->z;
    yy = a->y * a->y;
    yz = a->y * a->z;
    zz = a->z * a->z;
    xs = a->x * s;
    ys = a->y * s;
    zs = a->z * s;
    
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

void hpmRotateAxisAngle(float *axis, float angle, float *mat){
    if (angle == 0.0) return;
    float rot[16], r[16];
    hpmAxisAngleRotation(axis, angle, rot);
    hpmMultMat4(rot, mat, r);
    hpmCopyMat4(r, mat);
}

void hpmQuaternionRotation(float* quat, float *mat){
    HPMmat4 *m = (HPMmat4 *) mat;
    HPMquat *q = (HPMquat *) quat;
    initMat4(m);
    float xx, xy, xz, xw, yy, yz, yw, zz, zw;
    xx = q->x * q->x;
    xy = q->x * q->y;
    xz = q->x * q->z;
    xw = q->x * q->w;
    yy = q->y * q->y;
    yz = q->y * q->z;
    yw = q->y * q->w;
    zz = q->z * q->z;
    zw = q->z * q->w;
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

void hpmRotateQuaternion(float *quat, float *mat){
    float rot[16], r[16];
    hpmQuaternionRotation(quat, rot);
    hpmMultMat4(rot, mat, r);
    hpmCopyMat4(r, mat);
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
    hpmCopyMat4(r, mat);
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
    hpmCopyMat4(r, mat);
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
    hpmCopyMat4(r, mat);
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
    hpmCopyMat4(r, mat);
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

void hpmTranslateRotateScale2D(float *point, float angle, float scale, float *mat){
    HPMmat4 *m = (HPMmat4 *) mat;
    HPMpoint *p = (HPMpoint *) point;
    initMat4(m);
    float c = scale * cos(angle);
    float s = scale * sin(angle);
    m->_11 = c;
    m->_22 = c;
    m->_12 = -s;
    m->_21 = s;
    m->_33 = 1.0;
    m->_44 = 1.0;
    m->_14 = p->x;
    m->_24 = p->y;
    m->_34 = p->z;
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

void hpmFastInverseTranspose(const float *mat, float *result){
    HPMmat4 *m = (HPMmat4 *) mat;
    HPMmat4 *r = (HPMmat4 *) result;
    // Rotation component
    r->_11 = m->_11;
    r->_12 = m->_12;
    r->_13 = m->_13;

    r->_21 = m->_21;
    r->_22 = m->_22;
    r->_23 = m->_23;

    r->_31 = m->_31;
    r->_32 = m->_32;
    r->_33 = m->_33;
    // Translation component
    r->_41 = -(m->_11*m->_14 + m->_21*m->_24 + m->_31*m->_34);
    r->_42 = -(m->_12*m->_14 + m->_22*m->_24 + m->_32*m->_34);
    r->_43 = -(m->_13*m->_14 + m->_23*m->_24 + m->_33*m->_34);
    // Last column
    r->_14 = 0;
    r->_24 = 0;
    r->_34 = 0;
    r->_44 = 1;
}


//
// Matrix creation
//

// Projection
void hpmOrthoViewport(float left, float right, float bottom, float top,
                      float near, float far,
                      float vLeft, float vRight, float vBottom, float vTop,
                      float *mat){
    HPMmat4 *m = (HPMmat4 *) mat;
    initMat4(m);
    m->_11 = (vRight - vLeft) / (right - left);
    m->_14 = (right*vLeft - vRight*left) / (right - left);
    m->_22 = (vTop - vBottom) / (top - bottom);
    m->_24 = (top*vBottom - vTop*bottom) / (top - bottom);
    m->_33 = -2 / (far - near);
    m->_34 = -(far + near) / (far - near);
    m->_44 = 1.0;
}

void hpmOrtho(int width, int height, float near, float far, float *mat){
    float r = (float) width / 2;
    float l = -r;
    float t = (float) height / 2;
    float b = -t;
    hpmOrthoViewport(l, r, t, b, near, far, -1.0, 1.0, 1.0, -1.0, mat);
}

void hpmFrustumViewport(float left, float right, float bottom, float top,
                        float near, float far,
                        float vLeft, float vRight, float vBottom, float vTop,
                        float *mat){
    HPMmat4 *m = (HPMmat4 *) mat;
    initMat4(m);
    m->_11 = near * (vRight - vLeft) / (right - left);
    m->_13 = (vRight*left - right*vLeft) / (right - left);
    m->_22 = near * (vTop - vBottom) / (top - bottom);
    m->_23 = (vTop*bottom - top*vBottom) / (top - bottom);
    m->_33 = -(far + near) / (far - near);
    m->_34 = - (2.0 * far * near) / (far - near);
    m->_43 = -1.0;
}

void hpmFrustum(float left, float right, float bottom, float top,
		float near, float far, float *mat){
    hpmFrustumViewport(left, right, bottom, top, near, far, 
                       -1.0, 1.0, 1.0, -1.0, mat);
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
void hpmLookAt(float *eye, float *obj, float *up, float *mat){
    HPMmat4 *m = (HPMmat4 *) mat;
    HPMpoint f, r, u;
    initMat4(m);
    hpmSubVec(eye, obj, (float *) &f);
    hpmNormalize((float *) &f);
    hpmCross(up, (float *) &f, (float *) &r);
    hpmNormalize((float *) &r);
    hpmCross((float *) &f, (float *) &r, (float *) &u);

    m->_11 = r.x;
    m->_12 = r.y;
    m->_13 = r.z;
    m->_21 = u.x;
    m->_22 = u.y;
    m->_23 = u.z;
    m->_31 = f.x;
    m->_32 = f.y;
    m->_33 = f.z;
    m->_14 =-hpmDot((float *) &r, eye);
    m->_24 =-hpmDot((float *) &u, eye);
    m->_34 =-hpmDot((float *) &f, eye);
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
