//-----------------------------------------------------------------------------
// File: geom.h
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef GEOM_H
#define GEOM_H

// macros

#define USE_NEW_UNITS   FALSE


#define SLERP_SMALL_ANGLE Real(0.01)

/////////////////////////////////////////////////////////////////////
// Viewport dimensions
/////////////////////////////////////////////////////////////////////

#define REAL_SCREEN_XSIZE 640
#define REAL_SCREEN_YSIZE 480
#define REAL_SCREEN_XHALF 320
#define REAL_SCREEN_YHALF 240

#define DivScalar(numerator, divisor) ((numerator) / (divisor))
#define MulScalar(left, right) ((left) * (right))
#define MulScalar3(a, b, c) ((a) * (b) * (c))
#define MulScalar4(a, b, c, d) ((a) * (b) * (c) * (d))

#ifndef _N64
#define GET_ZBUFFER(z) \
    (RenderSettings.ZedFarDivDist - (RenderSettings.ZedFarMulNear / (RenderSettings.ZedDrawDist * (z))))
#else
#define GET_ZBUFFER(z) \
    (RenderSettings.FarDivDist - (RenderSettings.FarMulNear / (RenderSettings.DrawDist * (z) )))
#endif

#define SetVector(_v, _x, _y, _z) \
{ \
    (_v)->v[X] = _x; \
    (_v)->v[Y] = _y; \
    (_v)->v[Z] = _z; \
}

#define SubVector(a, b, c) \
    (c)->v[X] = (a)->v[X] - (b)->v[X]; \
    (c)->v[Y] = (a)->v[Y] - (b)->v[Y]; \
    (c)->v[Z] = (a)->v[Z] - (b)->v[Z];

#define AddVector(a, b, c) \
    (c)->v[X] = (a)->v[X] + (b)->v[X]; \
    (c)->v[Y] = (a)->v[Y] + (b)->v[Y]; \
    (c)->v[Z] = (a)->v[Z] + (b)->v[Z];

#define FTOL(_f, _i) \
{ \
    float _temp = (_f) + (float)(1L << 23); \
    (_i) = (*(long*)&_temp) & 0xffffff; \
}

#define FTOL2(_f) \
    ((*(long*)&((_f) += (float)(1L << 23))) & 0xffffff)

#define FTOL3(_f) \
    (*(long*)&((_f) += (float)(1L << 23)))

/////////////////////////////////////////////////////////////////////
// Platform independent macros
/////////////////////////////////////////////////////////////////////

#define SetVec(pVec, vx, vy, vz) SetVector(pVec, vx, vy, vz)
#define SetVecZero(vec) SetVec(vec, ZERO, ZERO, ZERO)
#define SetVecUp(vec) SetVec(vec, ZERO, -ONE, ZERO)
#define NegateVec(vec) SetVec(vec, -(vec)->v[X], -(vec)->v[Y], -(vec)->v[Z])
#define VecLen(vec) Length(vec)
#define VecLen2(vec) Length2(vec)       // Length squared
#define NormalizeVec(vec) NormalizeVector(vec)
#define VecDotVec(vLeft, vRight) DotProduct(vLeft, vRight)
#define VecCrossVec(vLeft, vRight, vOut) CrossProduct(vLeft, vRight, vOut)
#define CopyMatrix CopyMat
#define TransposeMatrix TransMat

#define CopyVec(src, dest) \
    { \
    (dest)->v[X] = (src)->v[X]; \
    (dest)->v[Y] = (src)->v[Y]; \
    (dest)->v[Z] = (src)->v[Z]; \
    }

#define VecPlusVec(vecLeft, vecRight, vecOut) \
    { \
    (vecOut)->v[X] = (vecLeft)->v[X] + (vecRight)->v[X]; \
    (vecOut)->v[Y] = (vecLeft)->v[Y] + (vecRight)->v[Y]; \
    (vecOut)->v[Z] = (vecLeft)->v[Z] + (vecRight)->v[Z]; \
    }

#define VecMinusVec(vecLeft, vecRight, vecOut) \
    { \
    (vecOut)->v[X] = (vecLeft)->v[X] - (vecRight)->v[X]; \
    (vecOut)->v[Y] = (vecLeft)->v[Y] - (vecRight)->v[Y]; \
    (vecOut)->v[Z] = (vecLeft)->v[Z] - (vecRight)->v[Z]; \
    }

#define VecPlusEqVec(vecLeft, vecRight) \
    { \
    (vecLeft)->v[X] += (vecRight)->v[X]; \
    (vecLeft)->v[Y] += (vecRight)->v[Y]; \
    (vecLeft)->v[Z] += (vecRight)->v[Z]; \
    }

#define VecMinusEqVec(vecLeft, vecRight) \
    { \
    (vecLeft)->v[X] -= (vecRight)->v[X]; \
    (vecLeft)->v[Y] -= (vecRight)->v[Y]; \
    (vecLeft)->v[Z] -= (vecRight)->v[Z]; \
    }

#define SetPlane(plane, a, b, c, d) \
    { \
    (plane)->v[A] = a; \
    (plane)->v[B] = b; \
    (plane)->v[C] = c; \
    (plane)->v[D] = d; \
    }

#define CopyPlane(src, dest) \
    { \
    (dest)->v[A] = (src)->v[A]; \
    (dest)->v[B] = (src)->v[B]; \
    (dest)->v[C] = (src)->v[C]; \
    (dest)->v[D] = (src)->v[D]; \
    }

#define PlaneNormal(plane) ((VEC *)(plane))

#define FlipPlane(src, dest) \
    { \
    (dest)->v[A] = -(src)->v[A]; \
    (dest)->v[B] = -(src)->v[B]; \
    (dest)->v[C] = -(src)->v[C]; \
    (dest)->v[D] = -(src)->v[D]; \
    }

#define MatPlusEqMat(matLeft, matRight) \
    { \
    (matLeft)->m[XX] += (matRight)->m[XX]; \
    (matLeft)->m[XY] += (matRight)->m[XY]; \
    (matLeft)->m[XZ] += (matRight)->m[XZ]; \
                                         \
    (matLeft)->m[YX] += (matRight)->m[YX]; \
    (matLeft)->m[YY] += (matRight)->m[YY]; \
    (matLeft)->m[YZ] += (matRight)->m[YZ]; \
                                         \
    (matLeft)->m[ZX] += (matRight)->m[ZX]; \
    (matLeft)->m[ZY] += (matRight)->m[ZY]; \
    (matLeft)->m[ZZ] += (matRight)->m[ZZ]; \
    }

/////////////////////////////////////////////////////////////////////
// Platform dependent macros
/////////////////////////////////////////////////////////////////////

#define Set4x4Matrix(_m, _t, _r) \
{ \
    (_r)->_11 = (_m)->m[RX]; \
    (_r)->_12 = (_m)->m[RY]; \
    (_r)->_13 = (_m)->m[RZ]; \
    (_r)->_14 = 0; \
\
    (_r)->_21 = (_m)->m[UX]; \
    (_r)->_22 = (_m)->m[UY]; \
    (_r)->_23 = (_m)->m[UZ]; \
    (_r)->_24 = 0; \
\
    (_r)->_31 = (_m)->m[LX]; \
    (_r)->_32 = (_m)->m[LY]; \
    (_r)->_33 = (_m)->m[LZ]; \
    (_r)->_34 = 0; \
\
    (_r)->_41 = (_t)->v[X]; \
    (_r)->_42 = (_t)->v[Y]; \
    (_r)->_43 = (_t)->v[Z]; \
    (_r)->_44 = 1; \
}

#define CrossProduct(a, b, c) \
{ \
    (c)->v[X] = (a)->v[Y] * (b)->v[Z] - (a)->v[Z] * (b)->v[Y]; \
    (c)->v[Y] = (a)->v[Z] * (b)->v[X] - (a)->v[X] * (b)->v[Z]; \
    (c)->v[Z] = (a)->v[X] * (b)->v[Y] - (a)->v[Y] * (b)->v[X]; \
}

#define CrossProduct3(a, b, c) \
    ((c)->v[X] * ((a)->v[Y] * (b)->v[Z] - (a)->v[Z] * (b)->v[Y]) + \
    (c)->v[Y] * ((a)->v[Z] * (b)->v[X] - (a)->v[X] * (b)->v[Z]) + \
    (c)->v[Z] * ((a)->v[X] * (b)->v[Y] - (a)->v[Y] * (b)->v[X]))

#define DotProduct(a, b) \
    ((a)->v[X] * (b)->v[X] + (a)->v[Y] * (b)->v[Y] + (a)->v[Z] * (b)->v[Z])

#define Cull(x1, y1, x2, y2, x3, y3) \
    (((x1) - (x2)) * ((y3) - (y2)) - ((y1) - (y2)) * ((x3) - (x2)))


#ifdef _N64

#define NormalizeVector(_v) \
{ \
    REAL _mul; \
    REAL _div = (REAL)sqrt((_v)->v[X] * (_v)->v[X] + (_v)->v[Y] * (_v)->v[Y] + (_v)->v[Z] * (_v)->v[Z]); \
    if(_div){ \
    _mul = 1 / _div; \
    (_v)->v[X] *= _mul; \
    (_v)->v[Y] *= _mul; \
    (_v)->v[Z] *= _mul; \
    }else{\
        Assert(0);\
    }\
}

#else

#define NormalizeVector(_v) \
{ \
    REAL _mul = 1 / (REAL)sqrt((_v)->v[X] * (_v)->v[X] + (_v)->v[Y] * (_v)->v[Y] + (_v)->v[Z] * (_v)->v[Z]); \
    (_v)->v[X] *= _mul; \
    (_v)->v[Y] *= _mul; \
    (_v)->v[Z] *= _mul; \
}

#endif

#define NormalizeMatrix(m) \
{ \
    NormalizeVector(&(m)->mv[Y]); \
    NormalizeVector(&(m)->mv[Z]); \
    CrossProduct(&(m)->mv[Y], &(m)->mv[Z], &(m)->mv[X]); \
    CrossProduct(&(m)->mv[Z], &(m)->mv[X], &(m)->mv[Y]); \
}

#define Length(a) \
    ((REAL)sqrt((a)->v[X] * (a)->v[X] + (a)->v[Y] * (a)->v[Y] + (a)->v[Z] * (a)->v[Z]))

#define Length2(a) \
    (((a)->v[X] * (a)->v[X] + (a)->v[Y] * (a)->v[Y] + (a)->v[Z] * (a)->v[Z]))
    
#define VecMulScalar(vec, scalar) \
    { \
    (vec)->v[X] *= (scalar); \
    (vec)->v[Y] *= (scalar); \
    (vec)->v[Z] *= (scalar); \
    }

#define VecDivScalar(vec, scalar) \
    { \
    (vec)->v[X] /= (scalar); \
    (vec)->v[Y] /= (scalar); \
    (vec)->v[Z] /= (scalar); \
    }

#define VecEqScalarVec(vLeft, scalar, vRight) \
    { \
    (vLeft)->v[X] = (scalar) * (vRight)->v[X]; \
    (vLeft)->v[Y] = (scalar) * (vRight)->v[Y]; \
    (vLeft)->v[Z] = (scalar) * (vRight)->v[Z]; \
    }

#define VecPlusScalarVec(vLeft, scalar, vRight, vOut) \
    { \
    (vOut)->v[X] = (vLeft)->v[X] + (scalar) * (vRight)->v[X]; \
    (vOut)->v[Y] = (vLeft)->v[Y] + (scalar) * (vRight)->v[Y]; \
    (vOut)->v[Z] = (vLeft)->v[Z] + (scalar) * (vRight)->v[Z]; \
    }

#define ScalarVecPlusScalarVec(scalarL, vLeft, scalarR, vRight, vOut) \
    { \
    (vOut)->v[X] = (scalarL) * (vLeft)->v[X] + (scalarR) * (vRight)->v[X]; \
    (vOut)->v[Y] = (scalarL) * (vLeft)->v[Y] + (scalarR) * (vRight)->v[Y]; \
    (vOut)->v[Z] = (scalarL) * (vLeft)->v[Z] + (scalarR) * (vRight)->v[Z]; \
    }

#define InterpVec(vLeft, vRight, scalar, vOut) \
    { \
    (vOut)->v[X] = (vLeft)->v[X] + (((vRight)->v[X] - (vLeft)->v[X]) * scalar); \
    (vOut)->v[Y] = (vLeft)->v[Y] + (((vRight)->v[Y] - (vLeft)->v[Y]) * scalar); \
    (vOut)->v[Z] = (vLeft)->v[Z] + (((vRight)->v[Z] - (vLeft)->v[Z]) * scalar); \
    }

#define VecPlusEqScalarVec(vLeft, scalar, vRight) \
    { \
    (vLeft)->v[X] = (vLeft)->v[X] + (scalar) * (vRight)->v[X]; \
    (vLeft)->v[Y] = (vLeft)->v[Y] + (scalar) * (vRight)->v[Y]; \
    (vLeft)->v[Z] = (vLeft)->v[Z] + (scalar) * (vRight)->v[Z]; \
    }

#define VecMinusScalarVec(vLeft, scalar, vRight, vOut) \
    { \
    (vOut)->v[X] = (vLeft)->v[X] - (scalar) * (vRight)->v[X]; \
    (vOut)->v[Y] = (vLeft)->v[Y] - (scalar) * (vRight)->v[Y]; \
    (vOut)->v[Z] = (vLeft)->v[Z] - (scalar) * (vRight)->v[Z]; \
    }

#define VecMinusEqScalarVec(vLeft, scalar, vRight) \
    { \
    (vLeft)->v[X] = (vLeft)->v[X] - (scalar) * (vRight)->v[X]; \
    (vLeft)->v[Y] = (vLeft)->v[Y] - (scalar) * (vRight)->v[Y]; \
    (vLeft)->v[Z] = (vLeft)->v[Z] - (scalar) * (vRight)->v[Z]; \
    }

#define VecDotMulVec(vLeft, vRight, vOut) \
    { \
    REAL dp = VecDotVec(vLeft, vRight); \
    (vOut)->v[X] = dp * (vRight)->v[X]; \
    (vOut)->v[Y] = dp * (vRight)->v[Y]; \
    (vOut)->v[Z] = dp * (vRight)->v[Z]; \
    }

#define VecDotPlane(vec, plane) \
    ( (vec)->v[X] * (plane)->v[A] + (vec)->v[Y] * (plane)->v[B] + (vec)->v[Z] * (plane)->v[C] + (plane)->v[D] )

#define PlaneDist(plane, vec) VecDotPlane((vec), (plane))

#define VecDotPlaneNorm(vec, plane) \
    ( (vec)->v[X] * (plane)->v[A] + (vec)->v[Y] * (plane)->v[B] + (vec)->v[Z] * (plane)->v[C] )

#define VecCrossVecDotVec(vec1, vec2, vec3) \
    ((vec3)->v[X] * ((vec1)->v[Y] * (vec2)->v[Z] - (vec1)->v[Z] * (vec2)->v[Y]) + \
    (vec3)->v[Y] * ((vec1)->v[Z] * (vec2)->v[X] - (vec1)->v[X] * (vec2)->v[Z]) + \
    (vec3)->v[Z] * ((vec1)->v[X] * (vec2)->v[Y] - (vec1)->v[Y] * (vec2)->v[X]))

#define VecCrossVecDotPlaneNorm(vec1, vec2, plane) VecCrossVecDotVec(vec1, vec2, PlaneNormal(plane))
/*  ((plane)->v[A] * ((vec1)->v[Y] * (vec2)->v[Z] - (vec1)->v[Z] * (vec2)->v[Y]) + \
    (plane)->v[B] * ((vec1)->v[Z] * (vec2)->v[X] - (vec1)->v[X] * (vec2)->v[Z]) + \
    (plane)->v[C] * ((vec1)->v[X] * (vec2)->v[Y] - (vec1)->v[Y] * (vec2)->v[X]))*/

#define LineInterp(r0, r1, t, rt) \
    { \
    (rt)->v[X] = (r0)->v[X] + (t) * ((r1)->v[X] - (r0)->v[X]); \
    (rt)->v[Y] = (r0)->v[Y] + (t) * ((r1)->v[Y] - (r0)->v[Y]); \
    (rt)->v[Z] = (r0)->v[Z] + (t) * ((r1)->v[Z] - (r0)->v[Z]); \
    }


/////////////////////////////////////////////////////////////////////
//
// Quaternion stuff
//

#define SetQuat(q, vx, vy, vz, s) \
    { \
    (q)->v[VX] = (vx); \
    (q)->v[VY] = (vy); \
    (q)->v[VZ] = (vz); \
    (q)->v[S] = (s); \
    }

#define CopyQuat(src, dest) \
    { \
    (dest)->v[VX] = (src)->v[VX]; \
    (dest)->v[VY] = (src)->v[VY]; \
    (dest)->v[VZ] = (src)->v[VZ]; \
    (dest)->v[S] = (src)->v[S]; \
    }

#define SetQuatUnit(q) \
    { \
    (q)->v[VX] = ZERO; \
    (q)->v[VY] = ZERO; \
    (q)->v[VZ] = ZERO; \
    (q)->v[S] = ONE; \
    }

#define NegateQuat(quat) \
    { \
    (quat)->v[VX] = -(quat)->v[VX]; \
    (quat)->v[VY] = -(quat)->v[VY]; \
    (quat)->v[VZ] = -(quat)->v[VZ]; \
    (quat)->v[S] = -(quat)->v[S]; \
    }

#define QuatDotQuat(qLeft, qRight) \
    ((qLeft)->v[VX] * (qRight)->v[VX] + \
    (qLeft)->v[VY] * (qRight)->v[VY] + \
    (qLeft)->v[VZ] * (qRight)->v[VZ] + \
    (qLeft)->v[S] * (qRight)->v[S])

#define QuatLen(q) ((REAL)sqrt(QuatDotQuat(q, q)))

#define NormalizeQuat(q) \
    { \
    REAL qLen = QuatLen(q); \
    if (qLen > SMALL_REAL) { \
        qLen = ONE / qLen; \
        (q)->v[VX] *= qLen; \
        (q)->v[VY] *= qLen; \
        (q)->v[VZ] *= qLen; \
        (q)->v[S] *= qLen; \
    } \
    }

    

#define QuatMulQuat(qLeft, qRight, qOut) \
    { \
    (qOut)->v[VX] = (qLeft)->v[S] * (qRight)->v[VX] + (qLeft)->v[VX] * (qRight)->v[S] + (qLeft)->v[VY] * (qRight)->v[VZ] - (qLeft)->v[VZ] * (qRight)->v[VY]; \
    (qOut)->v[VY] = (qLeft)->v[S] * (qRight)->v[VY] + (qLeft)->v[VY] * (qRight)->v[S] + (qLeft)->v[VZ] * (qRight)->v[VX] - (qLeft)->v[VX] * (qRight)->v[VZ]; \
    (qOut)->v[VZ] = (qLeft)->v[S] * (qRight)->v[VZ] + (qLeft)->v[VZ] * (qRight)->v[S] + (qLeft)->v[VX] * (qRight)->v[VY] - (qLeft)->v[VY] * (qRight)->v[VX]; \
    (qOut)->v[S] = (qLeft)->v[S] * (qRight)->v[S] - (qLeft)->v[VX] * (qRight)->v[VX] - (qLeft)->v[VY] * (qRight)->v[VY] - (qLeft)->v[VZ] * (qRight)->v[VZ]; \
    }

#define QuatMulInvQuat(qLeft, qRight, qOut) \
    { \
    (qOut)->v[VX] = (qLeft)->v[VX] * (qRight)->v[S] - (qLeft)->v[S] * (qRight)->v[VX] - (qLeft)->v[VY] * (qRight)->v[VZ] - (qLeft)->v[VZ] * (qRight)->v[VY]; \
    (qOut)->v[VY] = (qLeft)->v[VY] * (qRight)->v[S] - (qLeft)->v[S] * (qRight)->v[VY] - (qLeft)->v[VZ] * (qRight)->v[VX] - (qLeft)->v[VX] * (qRight)->v[VZ]; \
    (qOut)->v[VZ] = (qLeft)->v[VZ] * (qRight)->v[S] - (qLeft)->v[S] * (qRight)->v[VZ] - (qLeft)->v[VX] * (qRight)->v[VY] - (qLeft)->v[VY] * (qRight)->v[VX]; \
    (qOut)->v[S] = (qLeft)->v[S] * (qRight)->v[S] + (qLeft)->v[VX] * (qRight)->v[VX] + (qLeft)->v[VY] * (qRight)->v[VY] + (qLeft)->v[VZ] * (qRight)->v[VZ]; \
    }

#define VecMulQuat(vLeft, qRight, qOut) \
    { \
    (qOut)->v[VX] = (vLeft)->v[VX] * (qRight)->v[S] + (vLeft)->v[VY] * (qRight)->v[VZ] - (vLeft)->v[VZ] * (qRight)->v[VY]; \
    (qOut)->v[VY] = (vLeft)->v[VY] * (qRight)->v[S] + (vLeft)->v[VZ] * (qRight)->v[VX] - (vLeft)->v[VX] * (qRight)->v[VZ]; \
    (qOut)->v[VZ] = (vLeft)->v[VZ] * (qRight)->v[S] + (vLeft)->v[VX] * (qRight)->v[VY] - (vLeft)->v[VY] * (qRight)->v[VX]; \
    (qOut)->v[S] = - (vLeft)->v[VX] * (qRight)->v[VX] - (vLeft)->v[VY] * (qRight)->v[VY] - (vLeft)->v[VZ] * (qRight)->v[VZ]; \
    }

#define ConjQuat(qIn, qOut) \
    { \
    (qOut)->v[VX] = -(qIn)->v[VX]; \
    (qOut)->v[VY] = -(qIn)->v[VY]; \
    (qOut)->v[VZ] = -(qIn)->v[VZ]; \
    (qOut)->v[S] = (qIn)->v[S]; \
    }

#define InvQuat(qIn, qOut) (ConjQuat((qIn), (qOut)))

#define QuatPlusQuat(q1, q2, qOut) \
    { \
    (qOut)->v[VX] = (q1)->v[VX] + (q2)->v[VX]; \
    (qOut)->v[VY] = (q1)->v[VY] + (q2)->v[VY]; \
    (qOut)->v[VZ] = (q1)->v[VZ] + (q2)->v[VZ]; \
    (qOut)->v[S] = (q1)->v[S] + (q2)->v[S]; \
    }

#define QuatMinusQuat(q1, q2, qOut) \
    { \
    (qOut)->v[VX] = (q1)->v[VX] - (q2)->v[VX]; \
    (qOut)->v[VY] = (q1)->v[VY] - (q2)->v[VY]; \
    (qOut)->v[VZ] = (q1)->v[VZ] - (q2)->v[VZ]; \
    (qOut)->v[S] = (q1)->v[S] - (q2)->v[S]; \
    }

#define QuatPlusScalarQuat(q1, s, q2, qOut) \
    { \
    (qOut)->v[VX] = (q1)->v[VX] + s * (q2)->v[VX]; \
    (qOut)->v[VY] = (q1)->v[VY] + s * (q2)->v[VY]; \
    (qOut)->v[VZ] = (q1)->v[VZ] + s * (q2)->v[VZ]; \
    (qOut)->v[S] = (q1)->v[S] + s * (q2)->v[S]; \
    }

#define ScalarQuatPlusScalarQuat(scalarL, vLeft, scalarR, vRight, vOut) \
    { \
    (vOut)->v[VX] = (scalarL) * (vLeft)->v[VX] + (scalarR) * (vRight)->v[VX]; \
    (vOut)->v[VY] = (scalarL) * (vLeft)->v[VY] + (scalarR) * (vRight)->v[VY]; \
    (vOut)->v[VZ] = (scalarL) * (vLeft)->v[VZ] + (scalarR) * (vRight)->v[VZ]; \
    (vOut)->v[S] = (scalarL) * (vLeft)->v[S] + (scalarR) * (vRight)->v[S]; \
    }

#define QuatPlusEqScalarQuat(q1, s, q2) \
    { \
    (q1)->v[VX] += s * (q2)->v[VX]; \
    (q1)->v[VY] += s * (q2)->v[VY]; \
    (q1)->v[VZ] += s * (q2)->v[VZ]; \
    (q1)->v[S] += s * (q2)->v[S]; \
    }

#define VecOfQuat(q) ((VEC *)(q))

#define RemoveComponent(v1, v2) \
    { \
    VecMinusEqScalarVec((v1), VecDotVec((v1), (v2)), (v2)) \
    }

// prototypes

#ifndef _CARCONV
extern void RotMatrixX(MAT *mat, REAL rot);
extern void RotMatrixY(MAT *mat, REAL rot);
extern void RotMatrixZ(MAT *mat, REAL rot);
extern void RotMatrixZYX(MAT *mat, REAL x, REAL y, REAL z);
extern void RotVector(MAT *mat, VEC *in, VEC *out);
extern void TransposeRotVector(MAT *mat, VEC *in, VEC *out);
extern void RotTransVector(MAT *mat, VEC *trans, VEC *in, VEC *out);
extern void RotTransPersVector(MAT *mat, VEC *trans, VEC *in, REAL *out);
extern void RotTransPersVectorZleave(MAT *mat, VEC *trans, VEC *in, REAL *out);
extern void RotTransPersVectorZbias(MAT *mat, VEC *trans, VEC *in, REAL *out, REAL zbias);
extern void MulMatrix(MAT *one, MAT *two, MAT *out);
extern void TransposeMatrix(MAT *in, MAT *out);
extern void BuildLookMatrixForward(VEC *pos, VEC *look, MAT *mat);
extern void BuildLookMatrixDown(VEC *pos, VEC *look, MAT *mat);
extern void BuildMatrixFromLook(MAT *matrix);
extern void BuildMatrixFromUp(MAT *matrix);

// MAT and VEC operator prototypes
extern void SetMat(MAT *mat, REAL xx, REAL xy, REAL xz, REAL yx, REAL yy, REAL yz, REAL zx, REAL zy, REAL zz);
extern void SwapVecs(VEC *a, VEC *b);
extern void CopyMat(MAT *src, MAT *dest);
extern void SetMatZero(MAT *mat);
extern void SetMatUnit(MAT *mat);
extern void MatMulScalar(MAT *mat, REAL scalar);
extern void MatMulVec(MAT *mIn, VEC *vIn, VEC *vOut);
extern void VecMulMat(VEC *vIn, MAT *mIn, VEC *vOut);
extern void MatMulTransMat(MAT *mLeft, MAT *mRight, MAT *mOut);
extern void TransMatMulMat(MAT *mLeft, MAT *mRight, MAT *mOut);
extern void MatMulMat(MAT *mLeft, MAT *mRight, MAT *mOut);
extern void MatPlusMat(MAT *matLeft, MAT *matRight, MAT *matOut);
extern void MatPlusEqScalarMat(MAT *matLeft, REAL scalar, MAT *matRight);
extern void MatMulThisVec(MAT *mIn, VEC *vInOut);
extern void BuildRotation3D(REAL axisX, REAL axisY, REAL axisZ, REAL angle, MAT *mOut);
extern void RotationX(MAT *mat, REAL rot);
extern void RotationY(MAT *mat, REAL rot);
extern void RotationZ(MAT *mat, REAL rot);
extern void BuildMatFromVec(VEC *vec, MAT *mat);
extern void TransMat(MAT *src, MAT *dest) ;
extern void InvertMat(MAT *mat);
extern void BuildCrossMat(VEC *vec, MAT *mat);
extern void MatCrossVec(MAT *matLeft, VEC *vecRight, MAT *matOut);
extern void VecCrossMat(VEC *vecLeft, MAT *matRight, MAT *matOut);
extern void Interpolate3D(VEC *r0, VEC *r1, VEC *r2, REAL t, VEC *rt);
extern void QuadInterpVec(VEC *r0, REAL t0, VEC *r1, REAL t1, VEC *r2, REAL t2, REAL t, VEC *rt);
extern void LInterpVec(VEC *r0, REAL t0, VEC *r1, REAL t1, REAL t, VEC *rt);
extern void BuildPlane(VEC *v1, VEC *v2, VEC *v3, PLANE *plane);
extern void BuildPlane2(VEC *normal, VEC *pt, PLANE *plane);
extern void MovePlane(PLANE *plane, VEC *dR);
extern bool PlaneIntersect3(PLANE *p1, PLANE *p2, PLANE *p3, VEC *r);
extern void RotTransPlane(PLANE *plane, MAT *rotMat, VEC *dR, PLANE *pOut);
extern bool LinePoint(VEC *p, REAL d, VEC *r0, VEC *r1, REAL *t1, REAL *t2);
extern void TestLinePoint(void);
extern REAL NearPointOnLine(VEC *r0, VEC *r1, VEC *p, VEC *rN);
extern void FindIntersection(VEC *point1, REAL dist1, VEC *point2, REAL dist2, VEC *out);
extern void QuadInterpQuat(QUATERNION *r0, REAL t0, QUATERNION *r1, REAL t1, QUATERNION *r2, REAL t2, REAL t, QUATERNION *rt);
extern void ConstrainQuat1(QUATERNION *quat);
extern void ConstrainQuat2(QUATERNION *quat1, QUATERNION *quat2);
extern void QuatToMat(QUATERNION *quat, MAT *mat);
extern void MatToQuat(MAT *mat, QUATERNION *quat);
extern void InterpQuat(QUATERNION *q1, QUATERNION *q2);
extern void LerpQuat(QUATERNION *q1, QUATERNION *q2, REAL t, QUATERNION *qt);
#ifndef _N64
extern void SLerpQuat(QUATERNION *q1, QUATERNION *q2, REAL t, QUATERNION *qt);
#else
#define SLerpQuat LerpQuat
#endif
extern void QuatRotVec(QUATERNION *quat, VEC *vIn, VEC *vOut);
#endif
extern REAL SawTooth(REAL x, REAL f);
extern REAL FalsePower(REAL value, REAL power);
extern void InterpolatePosDir(VEC *r0, VEC *r1, VEC *rd, REAL t, VEC *rt);

// globals

extern REAL BaseGeomPers;
extern REAL ScreenLeftClip, ScreenRightClip, ScreenTopClip, ScreenBottomClip;
extern REAL ScreenLeftClipGuard, ScreenRightClipGuard, ScreenTopClipGuard, ScreenBottomClipGuard;
extern MAT IdentityMatrix;
extern MAT Identity;
extern VEC ZeroVector;
extern VEC DownVec;
extern VEC UpVec;
extern VEC RightVec;
extern VEC LeftVec;
extern VEC LookVec;
extern VEC NegLookVec;
extern QUATERNION IdentityQuat;

#endif // GEOM_H

