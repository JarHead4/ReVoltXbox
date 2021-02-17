
#ifndef DRAW_H
#define DRAW_H

#include "texture.h"
#include "world.h"
#include "newcoll.h"

// macros

#define POLY_QUAD 1
#define POLY_DOUBLE 2
#define POLY_SEMITRANS 4
#define POLY_MIRROR 128
#define POLY_SEMITRANS_ONE 256
#define POLY_TEXANIM 512
#define POLY_NOENV 1024
#define POLY_ENV 2048

#define MAX_SEMI_POLYS 800
#define MAX_3D_POLYS 128
#define MAX_WORLD_MESH_FX 32
#define MAX_MODEL_MESH_FX 32

#define MESHFX_USENEWVERTS 1

#define MAX_POLY_BUCKETS (TPAGE_NUM)
#define BUCKET_VERT_NUM 200
#define BUCKET_VERT_END (BUCKET_VERT_NUM - 4)

#define ENV_VERT_NUM 2000
#define ENV_VERT_END (ENV_VERT_NUM - 4)

#define JUMPSPARK_STEP_LEN 8.0f
#define JUMPSPARK_STEP_MAX 64
#define JUMPSPARK_OFFSET_NUM 32

#define RGB_ALPHA_MASK	0xff000000
#define RGB_RED_MASK	0xff0000
#define RGB_GREEN_MASK	0xff00
#define RGB_BLUE_MASK	0xff

#define FVF_TEX0 (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX0)
#define FVF_TEX1 (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1)
#define FVF_TEX2 (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX2)

// semi poly macros

#define SEMI_POLY_FREE() \
	(SemiCount < MAX_SEMI_POLYS)

#define SEMI_POLY_SETUP(_vert, _fog, _vertnum, _tpage, _clip, _semi) \
{ \
	SemiPoly[SemiCount].Fog = _fog; \
	SemiPoly[SemiCount].VertNum = _vertnum; \
	SemiPoly[SemiCount].Tpage = _tpage; \
	if (_clip) SemiPoly[SemiCount].DrawFlag = D3DDP_DONOTUPDATEEXTENTS; \
	else SemiPoly[SemiCount].DrawFlag = D3DDP_DONOTUPDATEEXTENTS | D3DDP_DONOTCLIP; \
	SemiPoly[SemiCount].SemiType = _semi; \
	SemiPoly[SemiCount].z = 0.0f; \
	_vert = SemiPoly[SemiCount++].Verts; \
}

#define SEMI_POLY_SETUP_RGB(_vert, _fog, _vertnum, _clip, _semi) \
{ \
	SemiPoly[SemiCount].Fog = _fog; \
	SemiPoly[SemiCount].VertNum = _vertnum; \
	SemiPoly[SemiCount].Tpage = -1; \
	if (_clip) SemiPoly[SemiCount].DrawFlag = D3DDP_DONOTUPDATEEXTENTS; \
	else SemiPoly[SemiCount].DrawFlag = D3DDP_DONOTUPDATEEXTENTS | D3DDP_DONOTCLIP; \
	SemiPoly[SemiCount].SemiType = _semi; \
	SemiPoly[SemiCount].z = 0.0f; \
	_vert = SemiPoly[SemiCount++].VertsRGB; \
}

#define SEMI_POLY_SETUP_ZBIAS(_vert, _fog, _vertnum, _tpage, _clip, _semi, _zbias) \
{ \
	SemiPoly[SemiCount].Fog = _fog; \
	SemiPoly[SemiCount].VertNum = _vertnum; \
	SemiPoly[SemiCount].Tpage = _tpage; \
	if (_clip) SemiPoly[SemiCount].DrawFlag = D3DDP_DONOTUPDATEEXTENTS; \
	else SemiPoly[SemiCount].DrawFlag = D3DDP_DONOTUPDATEEXTENTS | D3DDP_DONOTCLIP; \
	SemiPoly[SemiCount].SemiType = _semi; \
	SemiPoly[SemiCount].z = _zbias; \
	_vert = SemiPoly[SemiCount++].Verts; \
}

#define SEMI_POLY_SETUP_RGB_ZBIAS(_vert, _fog, _vertnum, _clip, _semi, _zbias) \
{ \
	SemiPoly[SemiCount].Fog = _fog; \
	SemiPoly[SemiCount].VertNum = _vertnum; \
	SemiPoly[SemiCount].Tpage = -1; \
	if (_clip) SemiPoly[SemiCount].DrawFlag = D3DDP_DONOTUPDATEEXTENTS; \
	else SemiPoly[SemiCount].DrawFlag = D3DDP_DONOTUPDATEEXTENTS | D3DDP_DONOTCLIP; \
	SemiPoly[SemiCount].SemiType = _semi; \
	SemiPoly[SemiCount].z = _zbias; \
	_vert = SemiPoly[SemiCount++].VertsRGB; \
}

// poly macros

#if SCREEN_DEBUG
#define INC_POLY_COUNT(_v, _n) \
	(_v) += (_n)
#else
#define INC_POLY_COUNT(_v, _n)
#endif

#define CLIP_QUAD() \
{ \
	if (mp->v0->Clip & mp->v1->Clip & mp->v2->Clip & mp->v3->Clip) continue; \
	clip = (mp->v0->Clip | mp->v1->Clip | mp->v2->Clip | mp->v3->Clip); \
}

#define CLIP_TRI() \
{ \
	if (mp->v0->Clip & mp->v1->Clip & mp->v2->Clip) continue; \
	clip = (mp->v0->Clip | mp->v1->Clip | mp->v2->Clip); \
}

#define CLIP_QUAD_MIRROR() \
{ \
	if (mp->v0->Clip & mp->v1->Clip & mp->v2->Clip & mp->v3->Clip) continue; \
	clip = (mp->v0->Clip | mp->v1->Clip | mp->v2->Clip | mp->v3->Clip) & 63; \
}

#define CLIP_TRI_MIRROR() \
{ \
	if (mp->v0->Clip & mp->v1->Clip & mp->v2->Clip) continue; \
	clip = (mp->v0->Clip | mp->v1->Clip | mp->v2->Clip) & 63; \
}

#define COPY_TRI_XYZRHW(_v) \
{ \
	*(MEM16*)&(_v)[0].sx = *(MEM16*)&mp->v0->sx; \
	*(MEM16*)&(_v)[1].sx = *(MEM16*)&mp->v1->sx; \
	*(MEM16*)&(_v)[2].sx = *(MEM16*)&mp->v2->sx; \
}

#define COPY_QUAD_XYZRHW(_v) \
{ \
	*(MEM16*)&(_v)[0].sx = *(MEM16*)&mp->v0->sx; \
	*(MEM16*)&(_v)[1].sx = *(MEM16*)&mp->v1->sx; \
	*(MEM16*)&(_v)[2].sx = *(MEM16*)&mp->v2->sx; \
	*(MEM16*)&(_v)[3].sx = *(MEM16*)&mp->v3->sx; \
}

#define COPY_TRI_UV(_v) \
{ \
	*(MEM8*)&(_v)[0].tu = *(MEM8*)&mp->tu0; \
	*(MEM8*)&(_v)[1].tu = *(MEM8*)&mp->tu1; \
	*(MEM8*)&(_v)[2].tu = *(MEM8*)&mp->tu2; \
}

#define COPY_QUAD_UV(_v) \
{ \
	*(MEM8*)&(_v)[0].tu = *(MEM8*)&mp->tu0; \
	*(MEM8*)&(_v)[1].tu = *(MEM8*)&mp->tu1; \
	*(MEM8*)&(_v)[2].tu = *(MEM8*)&mp->tu2; \
	*(MEM8*)&(_v)[3].tu = *(MEM8*)&mp->tu3; \
}

#define COPY_TRI_SPECULAR(_v) \
{ \
	(_v)[0].specular = mp->v0->specular; \
	(_v)[1].specular = mp->v1->specular; \
	(_v)[2].specular = mp->v2->specular; \
}

#define COPY_QUAD_SPECULAR(_v) \
{ \
	(_v)[0].specular = mp->v0->specular; \
	(_v)[1].specular = mp->v1->specular; \
	(_v)[2].specular = mp->v2->specular; \
	(_v)[3].specular = mp->v3->specular; \
}

// structures

typedef struct {
	float sx, sy, sz, rhw;
	DWORD color, specular;
} VERTEX_TEX0;

typedef struct {
	float sx, sy, sz, rhw;
	DWORD color, specular;
	float tu, tv;
} VERTEX_TEX1;

typedef struct {
	float sx, sy, sz, rhw;
	DWORD color, specular;
	float tu, tv;
	float tu2, tv2;
} VERTEX_TEX2;

typedef struct {
	unsigned short Index[ENV_VERT_NUM * 2];
	unsigned short *CurrentIndex;
	VERTEX_TEX1 *CurrentVerts;
	VERTEX_TEX1 Verts[ENV_VERT_NUM];
} BUCKET_ENV;

typedef struct {
	unsigned short Index[BUCKET_VERT_NUM * 2];
	unsigned short *CurrentIndex;
	VERTEX_TEX0 *CurrentVerts;
	VERTEX_TEX0 Verts[BUCKET_VERT_NUM];
} BUCKET_TEX0;

typedef struct {
	unsigned short Index[BUCKET_VERT_NUM * 2];
	unsigned short *CurrentIndex;
	VERTEX_TEX1 *CurrentVerts;
	VERTEX_TEX1 Verts[BUCKET_VERT_NUM];
} BUCKET_TEX1;

typedef struct {
	long DrawFlag, VertNum, Tpage, Fog, SemiType;
	float z;
	union {
		VERTEX_TEX1 Verts[4];
		VERTEX_TEX0 VertsRGB[4];
	};
} DRAW_SEMI_POLY;

typedef struct {
	long VertNum, Tpage, Fog, SemiType;
	VEC Pos[4];
	VERTEX_TEX1 Verts[4];
} DRAW_3D_POLY;

typedef struct {
	float Xsize, Ysize;
	float U, V, Usize, Vsize;
	short Tpage, pad;
	long RGB;
} FACING_POLY;

typedef struct {
	void (*Checker)(void *data);
	void *Data;
} WORLD_MESH_FX;

typedef struct {
	void (*Checker)(void *data);
	void *Data;
} MODEL_MESH_FX;

// prototypes

extern void InitPolyBuckets(void);
extern void KillPolyBuckets(void);
extern void FlushPolyBuckets(void);
extern void FlushEnvBuckets(void);
extern void FlushOneBucketTEX0(BUCKET_TEX0 *bucket, long clip);
extern void FlushOneBucketTEX1(BUCKET_TEX1 *bucket, long clip);
extern void FlushOneBucketEnv(BUCKET_ENV *bucket, long clip);
extern void Reset3dPolyList(void);
extern void Draw3dPolyList(void);
extern DRAW_3D_POLY *Get3dPoly(void);
extern void DrawNearClipPolyTEX0(VEC *pos, long *rgb, long vertnum);
extern void ResetSemiList(void);
extern void DrawSemiList(void);
extern void DrawFacingPolyMirror(VEC *pos, FACING_POLY *poly, long semi, float zbias);
extern void DrawFacingPoly(VEC *pos, FACING_POLY *poly, long semi, float zbias);
extern void DrawFacingPolyRotMirror(VEC *pos, MAT *mat, FACING_POLY *poly, long semi, float zbias);
extern void DrawFacingPolyRot(VEC *pos, MAT *mat, FACING_POLY *poly, long semi, float zbias);
extern void InitRenderStates(void);
extern void SetNearFar(REAL n, REAL f);
extern void SetFogVars(REAL fogstart, REAL vertstart, REAL vertend);
extern void DrawAxis(MAT *mat, VEC *pos);
extern void DumpImage(char handle, float x, float y, float w, float h, float u, float v, float tw, float th, unsigned long col);
extern void DrawMousePointer(unsigned long color);
extern BOOL LoadBitmap(char *bitmap, HBITMAP *hbm);
extern BOOL FreeBitmap(HBITMAP hbm);
extern BOOL BlitBitmap(HBITMAP hbm, IDirectDrawSurface4 **surface);
extern void DrawBoundingBox(float xmin, float xmax, float ymin, float ymax, float zmin, float zmax, long c0, long c1, long c2, long c3, long c4, long c5);
extern void DrawCubeNormals(WORLD_MODEL *m);
extern bool DrawCollSkin(CONVEX *skin, INDEX count);
extern void DrawLine(VEC *v0, VEC *v1, long col0, long col1);
extern void DrawTriClip(VERTEX_TEX1 *v0, VERTEX_TEX1 *v1, VERTEX_TEX1 *v2);
extern void DrawQuadClip(VERTEX_TEX1 *v0, VERTEX_TEX1 *v1, VERTEX_TEX1 *v2, VERTEX_TEX1 *v3);
extern void DrawFanClip(void);
extern void ClipLineTEX0(VERTEX_TEX0 *v0, VERTEX_TEX0 *v1, float mul, VERTEX_TEX0 *out);
extern void ClipLineTEX1(VERTEX_TEX1 *v0, VERTEX_TEX1 *v1, float mul, VERTEX_TEX1 *out);
extern void DrawCollPoly(NEWCOLLPOLY *poly);
extern void SaveFrontBuffer(char *file);
extern void ResetMeshFxList(void);
extern void AddWorldMeshFx(void (*checker)(void *data), void *data);
extern void AddModelMeshFx(void (*checker)(void *data), void *data);
extern void InitJumpSparkOffsets(void);
extern void DrawJumpSpark(VEC *v1, VEC *v2);
extern void DrawJumpSpark2(VEC *v1, VEC *v2);


// globals

extern BUCKET_ENV BucketEnvStill, BucketEnvStillFog, BucketEnvStillClip, BucketEnvStillClipFog;
extern BUCKET_ENV BucketEnvRoll, BucketEnvRollFog, BucketEnvRollClip, BucketEnvRollClipFog;
extern BUCKET_ENV BucketEnv, BucketEnvFog, BucketEnvClip, BucketEnvClipFog;
extern BUCKET_TEX0 BucketRGB, BucketFogRGB, BucketClipRGB, BucketClipFogRGB;
extern BUCKET_TEX1 Bucket[], BucketFog[], BucketClip[], BucketClipFog[];
extern VERTEX_TEX0 DrawVertsTEX0[];
extern VERTEX_TEX1 DrawVertsTEX1[];
extern VERTEX_TEX2 DrawVertsTEX2[];
extern DRAW_SEMI_POLY SemiPoly[];
extern WORLD_MESH_FX WorldMeshFx[];
extern MODEL_MESH_FX ModelMeshFx[];
extern long SemiCount, WorldMeshFxCount, ModelMeshFxCount;


#endif