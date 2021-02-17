
#ifndef MODEL_H
#define MODEL_H

#include "newcoll.h"

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

#define LOADMODEL_FORCE_TPAGE 1
#define LOADMODEL_OFFSET_TPAGE 2

#define MAX_LEVEL_MODELS 64

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
	LEVEL_MODEL_FOOTBALL,
	LEVEL_MODEL_SPACEMAN,
	LEVEL_MODEL_PICKUP,
	LEVEL_MODEL_FLAP,
	LEVEL_MODEL_LASER,
	LEVEL_MODEL_FIREWORK,
	LEVEL_MODEL_CHROMEBALL,
	LEVEL_MODEL_WATERBOMB,
	LEVEL_MODEL_BOMBBALL,
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
	float x, y, z;
	float nx, ny, nz;
} MODEL_VERTEX_LOAD;

typedef struct {
	short Type, Tpage;
	short vi0, vi1, vi2, vi3;
	long c0, c1, c2, c3;
	float u0, v0, u1, v1, u2, v2, u3, v3;
} MODEL_POLY_LOAD;

typedef struct {
	unsigned char b, g, r, a;
} MODEL_RGB;

typedef struct {
	MODEL_RGB rgb[4];
} POLY_RGB;

typedef struct {
	float x, y, z;
	float x2, y2, z2;
	float nx, ny, nz;
	float sx, sy, sz, rhw;
	long color, specular;
	float tu, tv;
	long r, g, b, a;
	unsigned char Clip;
	unsigned char pad0, pad1, pad2;
} MODEL_VERTEX;

typedef struct {
	float x, y, z;
	float nx, ny, nz;
} MODEL_VERTEX_MORPH;

typedef struct {
	short Type, Tpage;
	float tu0, tv0;
	float tu1, tv1;
	float tu2, tv2;
	float tu3, tv3;
	MODEL_VERTEX *v0, *v1, *v2, *v3;
} MODEL_POLY;

typedef struct {
	float Radius;
	float Xmin, Xmax;
	float Ymin, Ymax;
	float Zmin, Zmax;
	void *AllocPtr;
	short PolyNum, VertNum;
	short QuadNumTex, TriNumTex, QuadNumRGB, TriNumRGB;
	POLY_RGB *PolyRGB;
	MODEL_POLY *PolyPtr;
	MODEL_VERTEX *VertPtr;
	MODEL_VERTEX_MORPH *VertPtrMorph;
} MODEL;

typedef struct {
	short PolyNum, VertNum;
} MODEL_HEADER;

typedef struct {
	long ID, RefCount;
	MODEL Model;
//	BBOX BBox;
	COLLSKIN CollSkin;
} LEVEL_MODEL;

// prototypes

extern long LoadModel(char *file, MODEL *m, char tpage, char prmlevel, char loadflag, long RgbPer);
extern void FreeModel(MODEL *m, long prmlevel);
extern void DrawModel(MODEL *m, MAT *worldmat, VEC *worldpos, short flag);
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
extern void SetModelVertsGhost(MODEL *m);
extern void SetModelVertsGlare(MODEL *m, VEC *pos, MAT *mat, short flag);
extern void DrawModelPolysClip(MODEL *m, long lit, long env);
extern void DrawModelPolys(MODEL *m, long lit, long env);
extern void DrawModelPolysMirror(MODEL *m, long lit);
extern void SetEnvStatic(VEC *pos, MAT *mat, long rgb, float xoff, float yoff, float scale);
extern void SetEnvActive(VEC *pos, MAT *mat, MAT *envmat, long rgb, float xoff, float yoff, float scale);
extern void InitLevelModels(void);
extern void FreeLevelModels(void);
extern long LoadOneLevelModel(long id, long flag, struct renderflags renderflag, long tpage);
extern void FreeOneLevelModel(long slot);
extern void SetModelFrames(MODEL *model, char **files, long count);
extern void SetModelMorph(MODEL *m, long frame1, long frame2, float time);
extern void CheckModelMeshFx(MODEL *model, MAT *mat, VEC *pos, short *flag);

// globals

extern float ModelVertFog;
extern short ModelPolyCount, ModelDrawnCount, EnvTpage;
extern MAT EnvMatrix;
extern MODEL_RGB EnvRgb;
extern long ModelAddLit;
extern REAL ModelScale;
extern float GhostSineCount, GhostSinePos, GhostSineOffset;
extern LEVEL_MODEL LevelModel[];
extern MODEL *ModelMeshModel;
extern MAT *ModelMeshMat;
extern VEC *ModelMeshPos;
extern short *ModelMeshFlag;

#endif
