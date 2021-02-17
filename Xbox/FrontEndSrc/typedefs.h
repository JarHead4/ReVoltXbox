//-----------------------------------------------------------------------------
// File: typedefs.h
//
// Desc: typedefs for Re-Volt
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef TYPEDEFS_H
#define TYPEDEFS_H

/////////////////////////////////////////////////////////////////////
// PC and N64 floating point types
#ifndef _TRACK_EDITOR
#ifndef INT_MAX
#define INT_MAX     0x7FFFFFFF
#endif
#endif

typedef float REAL;

#ifndef _UNITCONV
#ifndef _TRACK_EDITOR
#undef bool
#define bool long
#endif
#endif

#define SMALL_REAL  (0.00001f)
#define SIMILAR_REAL (0.0001f)
#define ZERO        (0.0f)
#define ONE         (1.0f)
#define HALF        (0.5f)
#define LONGTIME    (100000.0f)
#define QUITELONGTIME (HALF * LONGTIME)
#define LARGEDIST   (1000000.0f)

#define Real(x) ((REAL)(x))
#define Int(r) ((int)(r))

// Return the nearest integer to a positive or negative real number
#define NearestInt(r) (Int((r) + HALF))

// type giving the index of a vertex in a vertex array
typedef short INDEX;

/////////////////////////////////////////////////////////////////////
// NULL TRUE and FALSE
/////////////////////////////////////////////////////////////////////

#ifndef NULL
#define NULL ((void *)0)
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE (!FALSE)
#endif


/////////////////////////////////////////////////////////////////////
// misc types
/////////////////////////////////////////////////////////////////////

enum VectorElements {X, Y, Z};
enum MatrixElements {
    XX, XY, XZ, 
    YX, YY, YZ,
    ZX, ZY, ZZ,
    RX=0, RY, RZ,
    UX, UY, UZ,
    LX, LY, LZ
};
enum MatrixVectors {R, U, L};
enum PlaneElements {A, B, C, D};

enum QuaternionElements {VX, VY, VZ, S};

#ifndef _PC
 typedef unsigned long long VISIMASK;
#else
#ifdef _CARCONV
 typedef long bool;
 typedef long VISIMASK;             // Dummy for CarConv
#else
 typedef unsigned __int64 VISIMASK;
#endif
#endif

typedef struct VectorStruct {
    REAL v[3];
} VEC;

typedef struct PlaneStruct {
    REAL v[4];
} PLANE;

typedef union MatrixUnion {
    REAL m[9];          // individual elements
    REAL mm[3][3];
    VEC mv[3];
    struct {
        REAL r[3];      // right vector
        REAL u[3];      // up vector
        REAL l[3];      // look vector
    } row;
} MAT;

typedef struct ShortVectorStruct {
    short v[3];
    short pad;
} SHORTVEC;

typedef union ShortMatrixUnion {
    short m[9];
    SHORTVEC mv[3];
} SHORTMAT;

typedef struct BoundingBoxStruct {
    REAL Xmin, Xmax;
    REAL Ymin, Ymax;
    REAL Zmin, Zmax;
} BOUNDING_BOX;

typedef struct ShortQuatStruct {
    short v[4];
} SHORTQUAT;

typedef struct CharQuatStruct {
    char v[4];
} CHARQUAT;

typedef struct QuaternionStruct {
    REAL v[4];
} QUATERNION;


typedef struct SphereStruct {
    VEC Pos;
    REAL    Radius;
} SPHERE;

typedef struct {
    long data[2];
} MEM8;

typedef struct {
    long data[3];
} MEM12;

typedef struct {
    long data[4];
} MEM16;

typedef struct {
    long data[5];
} MEM20;

typedef struct {
    long data[6];
} MEM24;

typedef struct {
    long data[7];
} MEM28;

typedef struct {
    long data[8];
} MEM32;

#endif // TYPEDEFS_H

