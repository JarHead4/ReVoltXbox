//-----------------------------------------------------------------------------
// File: model.h
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef MODEL_H
#define MODEL_H

#include "NewColl.h"
#include "object.h"         // only needed for struct renderflags...

#include "XBResource.h"  //$ADDITION(jedl) - graphics resources

// macros

#define MODEL_PLAIN 0
#define MODEL_FOG 1
#define MODEL_LIT 2
#define MODEL_ENV 4
#define MODEL_DONOTCLIP 8
#define MODEL_MIRROR 16
#define MODEL_GHOST 32
#define MODEL_GLARE 64
#define MODEL_USENEWVERTS 128
#define MODEL_ADDLIT 256
#define MODEL_SCALE 512
#define MODEL_ENVGOOD 1024
#define MODEL_NODRAW 2048
#define MODEL_NEARCLIP 4096

#define LOADMODEL_FORCE_TPAGE 1
#define LOADMODEL_OFFSET_TPAGE 2

#ifndef _PSX
#define MAX_LEVEL_MODELS 64
#else
#define MAX_LEVEL_MODELS 32
#endif


enum {
    LEVEL_MODEL_BARREL,
    LEVEL_MODEL_BEACHBALL,
    LEVEL_MODEL_MERCURY,
    LEVEL_MODEL_VENUS,
    LEVEL_MODEL_EARTH,
    LEVEL_MODEL_MARS,
    LEVEL_MODEL_JUPITER,
    LEVEL_MODEL_SATURN,
    LEVEL_MODEL_URANUS,
    LEVEL_MODEL_NEPTUNE,
    LEVEL_MODEL_PLUTO,
    LEVEL_MODEL_MOON,
    LEVEL_MODEL_RINGS,
    LEVEL_MODEL_PLANE,
    LEVEL_MODEL_PLANE_PROPELLOR,
    LEVEL_MODEL_COPTER,
    LEVEL_MODEL_COPTER_BLADE1,
    LEVEL_MODEL_COPTER_BLADE2,
    LEVEL_MODEL_DRAGON1,
    LEVEL_MODEL_DRAGON2,
    LEVEL_MODEL_WATER,
    LEVEL_MODEL_BOAT1,
    LEVEL_MODEL_BOAT2,
    LEVEL_MODEL_SPEEDUP,
    LEVEL_MODEL_RADAR,
    LEVEL_MODEL_BALLOON,
    LEVEL_MODEL_HORSE,
    LEVEL_MODEL_TRAIN,
    LEVEL_MODEL_TRAIN2,
    LEVEL_MODEL_TRAIN3,
    LEVEL_MODEL_LIGHT1,
    LEVEL_MODEL_LIGHT2,
    LEVEL_MODEL_LIGHT3,
    LEVEL_MODEL_FOOTBALL,
    LEVEL_MODEL_SPACEMAN,
    LEVEL_MODEL_PICKUP,
    LEVEL_MODEL_FLAP,
    LEVEL_MODEL_LASER,
    LEVEL_MODEL_FIREWORK,
    LEVEL_MODEL_CHROMEBALL,
    LEVEL_MODEL_WATERBOMB,
    LEVEL_MODEL_BOMBBALL,
    LEVEL_MODEL_WEEBEL,
    LEVEL_MODEL_PROBELOGO,
    LEVEL_MODEL_SPRINKLER_BASE,
    LEVEL_MODEL_SPRINKLER_HEAD,
    LEVEL_MODEL_SPRINKLER_HOSE,
    LEVEL_MODEL_BASKETBALL,
    LEVEL_MODEL_TRACKSCREEN,
    LEVEL_MODEL_CLOCKBODY,
    LEVEL_MODEL_CLOCKHANDLARGE,
    LEVEL_MODEL_CLOCKHANDSMALL,
    LEVEL_MODEL_CLOCKDISC,
    LEVEL_MODEL_CARBOX,
    LEVEL_MODEL_PLAINBOX,
    LEVEL_MODEL_NAMESTAND,
    LEVEL_MODEL_NAMEWHEEL,
    LEVEL_MODEL_STREAM,
    LEVEL_MODEL_CUP1,
    LEVEL_MODEL_CUP2,
    LEVEL_MODEL_CUP3,
    LEVEL_MODEL_CUP4,
    LEVEL_MODEL_STAR,
    LEVEL_MODEL_TUMBLEWEED,
    LEVEL_MODEL_SMALLSCREEN,
    LEVEL_MODEL_LANTERN,
    LEVEL_MODEL_SLIDER,
    LEVEL_MODEL_BOTTLE,
    LEVEL_MODEL_SHIP_POOL,
    LEVEL_MODEL_GARDEN_WATER1,
    LEVEL_MODEL_GARDEN_WATER2,
    LEVEL_MODEL_GARDEN_WATER3,
    LEVEL_MODEL_GARDEN_WATER4,
    LEVEL_MODEL_BUCKET,
    LEVEL_MODEL_CONE,
    LEVEL_MODEL_CONE2,
    LEVEL_MODEL_CAN,
    LEVEL_MODEL_LILO,
    LEVEL_MODEL_SHIPLIGHT,
    LEVEL_MODEL_PACKET,
    LEVEL_MODEL_PACKET1,
    LEVEL_MODEL_ABC,
    LEVEL_MODEL_SPRINKLER_BASE_GARDEN,
    LEVEL_MODEL_SPRINKLER_HEAD_GARDEN,
};

#define ModelAddGouraud(_a, _b, _c) \
{ \
    long _i; \
    _i = (_b)[0] + (long)(_a)->r; \
    if (_i > 255) (_c)->r = 255; \
    else if (_i < 0) (_c)->r = 0; \
    else (_c)->r = (unsigned char)_i; \
    _i = (_b)[1] + (long)(_a)->g; \
    if (_i > 255) (_c)->g = 255; \
    else if (_i < 0) (_c)->g = 0; \
    else (_c)->g = (unsigned char)_i; \
    _i = (_b)[2] + (long)(_a)->b; \
    if (_i > 255) (_c)->b = 255; \
    else if (_i < 0) (_c)->b = 0; \
    else (_c)->b = (unsigned char)_i; \
}

#define ModelAddGouraudAlpha(_a, _b, _c) \
{ \
    long _i; \
    _i = (_b)[0] + (long)(_a)->r; \
    if (_i > 255) (_c)->r = 255; \
    else if (_i < 0) (_c)->r = 0; \
    else (_c)->r = (unsigned char)_i; \
    _i = (_b)[1] + (long)(_a)->g; \
    if (_i > 255) (_c)->g = 255; \
    else if (_i < 0) (_c)->g = 0; \
    else (_c)->g = (unsigned char)_i; \
    _i = (_b)[2] + (long)(_a)->b; \
    if (_i > 255) (_c)->b = 255; \
    else if (_i < 0) (_c)->b = 0; \
    else (_c)->b = (unsigned char)_i; \
    (_c)->a = (_a)->a; \
}

#define ModelChangeGouraud(c, p) \
{ \
    if (p != 100) \
    { \
        (c)->r = (unsigned char)(((c)->r + 1) * p / 100); \
        (c)->g = (unsigned char)(((c)->g + 1) * p / 100); \
        (c)->b = (unsigned char)(((c)->b + 1) * p / 100); \
    } \
}

#define Grayscale(rgb) \
{ \
    long bri = (((*(rgb) & 0xff0000) >> 16) * 77 + ((*(rgb) & 0x00ff00) >> 8) * 150 + (*(rgb) & 0x0000ff) * 29) >> 8; \
    *(rgb) = bri | bri << 8 | bri << 16; \
}

#define ModelChangeGouraudAlpha(c, p) \
{ \
    if (p != 100) \
    { \
        (c)->a = ((c)->a + 1) * p / 100; \
        (c)->r = ((c)->r + 1) * p / 100; \
        (c)->g = ((c)->g + 1) * p / 100; \
        (c)->b = ((c)->b + 1) * p / 100; \
    } \
}

#define REJECT_MODEL_ENV_POLY() \
    if (mp->Type & POLY_NOENV) continue

#define REJECT_MODEL_POLY() \
{ \
    if (!(mp->Type & POLY_DOUBLE)) if (Cull(mp->v0->sx, mp->v0->sy, mp->v1->sx, mp->v1->sy, mp->v2->sx, mp->v2->sy) > 0) continue; \
}

#define REJECT_MODEL_POLY_MIRROR() \
{ \
    if (!(mp->Type & POLY_DOUBLE)) if (Cull(mp->v0->sx, mp->v0->sy, mp->v1->sx, mp->v1->sy, mp->v2->sx, mp->v2->sy) < 0) continue; \
}

#define REJECT_MODEL_POLY_3D() \
{ \
    if (!(mp->Type & POLY_DOUBLE)) if (PlaneDist(&mp->Plane, &ModelSpaceCameraPos) > 0.0f) continue; \
}

#define COPY_MODEL_TRI_COLOR(_v) \
{ \
    (_v)[0].color = *(long*)&mrgb->rgb[0]; \
    (_v)[1].color = *(long*)&mrgb->rgb[1]; \
    (_v)[2].color = *(long*)&mrgb->rgb[2]; \
}

#define COPY_MODEL_QUAD_COLOR(_v) \
{ \
    (_v)[0].color = *(long*)&mrgb->rgb[0]; \
    (_v)[1].color = *(long*)&mrgb->rgb[1]; \
    (_v)[2].color = *(long*)&mrgb->rgb[2]; \
    (_v)[3].color = *(long*)&mrgb->rgb[3]; \
}

#define COPY_MODEL_TRI_COLOR_LIT(_v) \
{ \
    ModelAddGouraudAlpha(&mrgb->rgb[0], &mp->v0->r, (MODEL_RGB*)&(_v)[0].color); \
    ModelAddGouraudAlpha(&mrgb->rgb[1], &mp->v1->r, (MODEL_RGB*)&(_v)[1].color); \
    ModelAddGouraudAlpha(&mrgb->rgb[2], &mp->v2->r, (MODEL_RGB*)&(_v)[2].color); \
}

#define COPY_MODEL_QUAD_COLOR_LIT(_v) \
{ \
    ModelAddGouraudAlpha(&mrgb->rgb[0], &mp->v0->r, (MODEL_RGB*)&(_v)[0].color); \
    ModelAddGouraudAlpha(&mrgb->rgb[1], &mp->v1->r, (MODEL_RGB*)&(_v)[1].color); \
    ModelAddGouraudAlpha(&mrgb->rgb[2], &mp->v2->r, (MODEL_RGB*)&(_v)[2].color); \
    ModelAddGouraudAlpha(&mrgb->rgb[3], &mp->v3->r, (MODEL_RGB*)&(_v)[3].color); \
}

#define COPY_MODEL_TRI_COLOR_GHOST(_v) \
{ \
    (_v)[0].color = *(long*)&mrgb->rgb[0] | mp->v0->a; \
    (_v)[1].color = *(long*)&mrgb->rgb[1] | mp->v1->a; \
    (_v)[2].color = *(long*)&mrgb->rgb[2] | mp->v2->a; \
}

#define COPY_MODEL_QUAD_COLOR_GHOST(_v) \
{ \
    (_v)[0].color = *(long*)&mrgb->rgb[0] | mp->v0->a; \
    (_v)[1].color = *(long*)&mrgb->rgb[1] | mp->v1->a; \
    (_v)[2].color = *(long*)&mrgb->rgb[2] | mp->v2->a; \
    (_v)[3].color = *(long*)&mrgb->rgb[3] | mp->v3->a; \
}

// structures

typedef struct {
    REAL x, y, z;
    REAL nx, ny, nz;
} MODEL_VERTEX_LOAD;

typedef struct {
    short Type, Tpage;
    short vi0, vi1, vi2, vi3;
    long c0, c1, c2, c3;
    REAL u0, v0, u1, v1, u2, v2, u3, v3;
} MODEL_POLY_LOAD;

typedef struct {
    unsigned char b, g, r, a;
} MODEL_RGB;

typedef struct {
    MODEL_RGB rgb[4];
} POLY_RGB;

typedef struct {
    REAL x, y, z;
    REAL x2, y2, z2;
    REAL nx, ny, nz;
    REAL sx, sy, sz, rhw;
    long color, specular;
    REAL tu, tv;
    long r, g, b, a;
    unsigned long Clip;
} MODEL_VERTEX;

typedef struct {
    REAL x, y, z;
    REAL nx, ny, nz;
} MODEL_VERTEX_MORPH;

typedef struct {
    short Type, Tpage;
    REAL tu0, tv0;
    REAL tu1, tv1;
    REAL tu2, tv2;
    REAL tu3, tv3;
    PLANE Plane;
    MODEL_VERTEX *v0, *v1, *v2, *v3;
} MODEL_POLY;

typedef struct {
    REAL Radius;
    REAL Xmin, Xmax;
    REAL Ymin, Ymax;
    REAL Zmin, Zmax;
    void *AllocPtr;
    short PolyNum, VertNum;
    short QuadNumTex, TriNumTex, QuadNumRGB, TriNumRGB;
    POLY_RGB *PolyRGB;
    MODEL_POLY *PolyPtr;
    MODEL_VERTEX *VertPtr;
    MODEL_VERTEX_MORPH *VertPtrMorph;
//$ADDITION(jedl)
    // New model stucture uses vertex buffers, index buffers,
    // vertex shader, pixel shaders, and draw primitive calls.
    Effect *m_pEffect;
//$END_ADDITION
} MODEL;

typedef struct {
    short PolyNum, VertNum;
} MODEL_HEADER;

typedef struct {
    long ID, RefCount;
    MODEL Model;
//  BBOX BBox;
    COLLSKIN CollSkin;
} LEVEL_MODEL;

// prototypes

extern long LoadModel(char *file, MODEL *m, char tpage, char prmlevel, char loadflag, long RgbPer);
//$ADDITION
extern long LoadModelGPU(char *file,        // name of model file
                         MODEL *m,          // model to load
                         INT prmlevel,      // max number of level-of-detail models
                         XBResource *pXBR); // resource to look for models
//$END_ADDITION
extern void FreeModel(MODEL *m, long prmlevel);
extern void DrawModel(MODEL *m, MAT *worldmat, VEC *worldpos, short flag);
extern void DrawModelGPU(MODEL *m, MAT *worldmat, VEC *worldpos, short flag); //$ADDITION(jedl) - draw model using GPU
extern void TransModelVertsFogClip(MODEL *m, MAT *mat, VEC *trans);
extern void TransModelVertsPlainClip(MODEL *m, MAT *mat, VEC *trans);
extern void TransModelVertsPlain(MODEL *m, MAT *mat, VEC *trans);
extern void TransModelVertsFog(MODEL *m, MAT *mat, VEC *trans);
extern void TransModelVertsFogClipNewVerts(MODEL *m, MAT *mat, VEC *trans);
extern void TransModelVertsPlainClipNewVerts(MODEL *m, MAT *mat, VEC *trans);
extern void TransModelVertsPlainNewVerts(MODEL *m, MAT *mat, VEC *trans);
extern void TransModelVertsFogNewVerts(MODEL *m, MAT *mat, VEC *trans);
extern void TransModelVertsMirror(MODEL *m, MAT *mat, VEC *trans, MAT *worldmat, VEC *worldpos);
extern void TransModelVertsMirrorNewVerts(MODEL *m, MAT *mat, VEC *trans, MAT *worldmat, VEC *worldpos);
extern void SetModelVertsEnvPlain(MODEL *m);
extern void SetModelVertsEnvLit(MODEL *m);
extern void SetModelVertsEnvGoodPlain(MODEL *m, VEC *campos, VEC *up);
extern void SetModelVertsEnvGoodLit(MODEL *m, VEC *campos, VEC *up);
extern void SetModelVertsGhost(MODEL *m);
extern void SetModelVertsGlare(MODEL *m, VEC *pos, MAT *mat, short flag);
extern void DrawModelPolysNearClip(MODEL *m, long lit, long env);
extern void DrawModelPolysClip(MODEL *m, long lit, long env);
extern void DrawModelPolys(MODEL *m, long lit, long env);
extern void DrawModelPolysMirror(MODEL *m, long lit);
extern void SetEnvStatic(VEC *pos, MAT *mat, long rgb, REAL xoff, REAL yoff, REAL scale);
extern void SetEnvActive(VEC *pos, MAT *mat, MAT *envmat, long rgb, REAL xoff, REAL yoff, REAL scale);
extern void SetEnvGood(long rgb, REAL xoff, REAL yoff, REAL scale);
extern void InitLevelModels(void);
extern void FreeLevelModels(void);
extern long LoadOneLevelModel(long id, long flag, struct renderflags renderflag, long tpage);
extern void FreeOneLevelModel(long slot);
extern void SetModelFrames(MODEL *model, char **files, long count);
extern void SetModelMorph(MODEL *m, long frame1, long frame2, REAL time);
extern void CheckModelMeshFx(MODEL *model, MAT *mat, VEC *pos, short *flag);
extern long CopyModel(MODEL *src, MODEL *dest);

// globals

extern REAL ModelVertFog;
extern short ModelPolyCount, EnvTpage;
extern MAT EnvMatrix;
extern MODEL_RGB EnvRgb;
extern long ModelAddLit;
extern REAL ModelScale;
extern REAL ModelGhostSineOffset, ModelGhostSinePos, ModelGhostFadeMul, GhostSineCount, GhostSinePos;
extern LEVEL_MODEL LevelModel[];
extern MODEL *ModelMeshModel;
extern MAT *ModelMeshMat;
extern VEC *ModelMeshPos;
extern short *ModelMeshFlag;

#endif // MODEL_H

