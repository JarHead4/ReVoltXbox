//-----------------------------------------------------------------------------
// File: world.h
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef WORLD_H
#define WORLD_H

#include "XBResource.h"  //$ADDITION(jedl) - graphics resources

// macros

#define MAX_TEXANIMS 16
#define DRAW_WORLD_NORMALS 0

#define NEAR_CLIP_POLY() \
    (clip & CLIP_NEAR)

#define REJECT_WORLD_POLY() \
{ \
    if (mp->VisiMask & CamVisiMask) continue; \
    if (!(mp->Type & POLY_DOUBLE)) if (Cull(mp->v0->sx, mp->v0->sy, mp->v1->sx, mp->v1->sy, mp->v2->sx, mp->v2->sy) > 0.0f) continue; \
}

#define REJECT_WORLD_POLY_3D() \
{ \
    if (mp->VisiMask & CamVisiMask) continue; \
    if (!(mp->Type & POLY_DOUBLE)) if (PlaneDist(&mp->Plane, &ViewCameraPos) > 0.0f) continue; \
}

#define COPY_WORLD_TRI_COLOR(_v) \
{ \
    (_v)[0].color = mp->rgb0; \
    (_v)[1].color = mp->rgb1; \
    (_v)[2].color = mp->rgb2; \
}

#define COPY_WORLD_QUAD_COLOR(_v) \
{ \
    (_v)[0].color = mp->rgb0; \
    (_v)[1].color = mp->rgb1; \
    (_v)[2].color = mp->rgb2; \
    (_v)[3].color = mp->rgb3; \
}

#define COPY_WORLD_TRI_COLOR_LIT(_v) \
{ \
    ModelAddGouraudAlpha((MODEL_RGB*)&mp->rgb0, &mp->v0->r, (MODEL_RGB*)&(_v)[0].color); \
    ModelAddGouraudAlpha((MODEL_RGB*)&mp->rgb1, &mp->v1->r, (MODEL_RGB*)&(_v)[1].color); \
    ModelAddGouraudAlpha((MODEL_RGB*)&mp->rgb2, &mp->v2->r, (MODEL_RGB*)&(_v)[2].color); \
}

#define COPY_WORLD_QUAD_COLOR_LIT(_v) \
{ \
    ModelAddGouraudAlpha((MODEL_RGB*)&mp->rgb0, &mp->v0->r, (MODEL_RGB*)&(_v)[0].color); \
    ModelAddGouraudAlpha((MODEL_RGB*)&mp->rgb1, &mp->v1->r, (MODEL_RGB*)&(_v)[1].color); \
    ModelAddGouraudAlpha((MODEL_RGB*)&mp->rgb2, &mp->v2->r, (MODEL_RGB*)&(_v)[2].color); \
    ModelAddGouraudAlpha((MODEL_RGB*)&mp->rgb3, &mp->v3->r, (MODEL_RGB*)&(_v)[3].color); \
}

#define COPY_WORLD_TRI_COLOR_LIT_MIRROR(_v) \
{ \
    ModelAddGouraudAlpha((MODEL_RGB*)&mp->rgb0, &mp->v0->RealVertex->r, (MODEL_RGB*)&(_v)[0].color); \
    ModelAddGouraudAlpha((MODEL_RGB*)&mp->rgb1, &mp->v1->RealVertex->r, (MODEL_RGB*)&(_v)[1].color); \
    ModelAddGouraudAlpha((MODEL_RGB*)&mp->rgb2, &mp->v2->RealVertex->r, (MODEL_RGB*)&(_v)[2].color); \
}

#define COPY_WORLD_QUAD_COLOR_LIT_MIRROR(_v) \
{ \
    ModelAddGouraudAlpha((MODEL_RGB*)&mp->rgb0, &mp->v0->RealVertex->r, (MODEL_RGB*)&(_v)[0].color); \
    ModelAddGouraudAlpha((MODEL_RGB*)&mp->rgb1, &mp->v1->RealVertex->r, (MODEL_RGB*)&(_v)[1].color); \
    ModelAddGouraudAlpha((MODEL_RGB*)&mp->rgb2, &mp->v2->RealVertex->r, (MODEL_RGB*)&(_v)[2].color); \
    ModelAddGouraudAlpha((MODEL_RGB*)&mp->rgb3, &mp->v3->RealVertex->r, (MODEL_RGB*)&(_v)[3].color); \
}

// structures

typedef struct {
    long Tpage;
    float Time;
    float u0, v0;
    float u1, v1;
    float u2, v2;
    float u3, v3;
} TEXANIM_FRAME;

typedef struct {
    TEXANIM_FRAME *Frame, *CurrentFrame;
    long FrameNum, CurrentFrameNum;
    float FrameTime;
} TEXANIM_HEADER;

typedef struct {
    float x, y, z;
    float nx, ny, nz;
} WORLD_VERTEX_LOAD;

typedef struct {
    short Type, Tpage;
    short vi0, vi1, vi2, vi3;
    long c0, c1, c2, c3;
    float u0, v0, u1, v1, u2, v2, u3, v3;
} WORLD_POLY_LOAD;

typedef struct {
    float x, y, z;
    float x2, y2, z2;
    float nx, ny, nz;
    float sx, sy, sz, rhw;
    long color, specular;
    float tu, tv;
    long r, g, b, EnvRGB;
    float VertFog;
    unsigned long Clip;
} WORLD_VERTEX;

typedef struct {
    short Type, Tpage;
    VISIMASK VisiMask;
    long rgb0, rgb1, rgb2, rgb3;
    float tu0, tv0;
    float tu1, tv1;
    float tu2, tv2;
    float tu3, tv3;
    PLANE Plane;
    WORLD_VERTEX *v0, *v1, *v2, *v3;
} WORLD_POLY;

typedef struct {
    float x, y, z;
    float x2, y2, z2;
    float sx, sy, sz, rhw;
    float VertFog;
    long specular;
    unsigned char Clip, pad[3];
    WORLD_VERTEX *RealVertex;
} WORLD_MIRROR_VERTEX;

typedef struct {
    short Type, Tpage;
    VISIMASK VisiMask;
    long rgb0, rgb1, rgb2, rgb3;
    float tu0, tv0;
    float tu1, tv1;
    float tu2, tv2;
    float tu3, tv3;
    PLANE Plane;
    WORLD_MIRROR_VERTEX *v0, *v1, *v2, *v3;
} WORLD_MIRROR_POLY;

typedef struct {
    WORLD_POLY *Poly;
    TEXANIM_HEADER *Anim;
} WORLD_ANIM_POLY;

typedef struct {
    void *AllocPtr;
    short PolyNum, VertNum;
    short QuadNumTex, TriNumTex, QuadNumRGB, TriNumRGB;
    short MirrorPolyNum, MirrorVertNum;
    short MirrorQuadNumTex, MirrorTriNumTex, MirrorQuadNumRGB, MirrorTriNumRGB;
    short AnimPolyNum, EnvVertNum;
    WORLD_POLY *PolyPtr;
    WORLD_VERTEX *VertPtr;
    WORLD_MIRROR_POLY *MirrorPolyPtr;
    WORLD_MIRROR_VERTEX *MirrorVertPtr;
    WORLD_ANIM_POLY *AnimPolyPtr;
    WORLD_VERTEX **EnvVertPtr;
} WORLD_MODEL;

typedef struct {
    float CentreX, CentreY, CentreZ, Radius;
    float Xmin, Xmax, Ymin, Ymax, Zmin, Zmax;
    short PolyNum, VertNum;
} CUBE_HEADER_LOAD;

typedef struct {
    float CentreX, CentreY, CentreZ, Radius;
    float Xmin, Xmax, Ymin, Ymax, Zmin, Zmax;
    VISIMASK VisiMask;
    long Clip, z, Lit, MeshFxFlag;
    float MirrorHeight;
    WORLD_MODEL Model;
} CUBE_HEADER;

typedef struct {
    float x, y, z, Radius;
    long CubeNum;
} BIG_CUBE_HEADER_LOAD;

typedef struct {
    float x, y, z, Radius;
    long CubeNum;
    CUBE_HEADER **Cubes;
} BIG_CUBE_HEADER;

typedef struct {
    long CubeNum, BigCubeNum;
    CUBE_HEADER *Cube, **CubeList;
    BIG_CUBE_HEADER *BigCube;
//$ADDITION(jedl)
    // New structure uses vertex buffers, index buffers,
    // vertex shader, pixel shaders, and draw primitive calls.
    XBResource *m_pXBR;
    Effect *m_pEffectDefaultMaterial;
    Effect *m_pEffect;
//$END_ADDITION
} WORLD;

typedef struct {
    long CubeNum;
} WORLD_HEADER;

// prototypes

extern bool LoadWorld(char *file);
//$ADDITION(jedl)
extern HRESULT LoadWorldGPU(char *file);
//$END_ADDITION
extern void FreeWorld(void);
extern void MirrorWorldPolys(void);
extern void SetWorldMirror(void);
extern void DrawWorld(void);
extern void DrawWorldWireframe(void);
extern void DrawWorldCube(CUBE_HEADER *cube);
extern void DrawWorldCubeMirror(CUBE_HEADER *cube);
extern void DrawWorldCubeWireframe(CUBE_HEADER *cube);
extern void TransCubeVertsClip(WORLD_MODEL *m);
extern void TransCubeVertsFogClip(WORLD_MODEL *m);
extern void TransCubeVerts(WORLD_MODEL *m);
extern void TransCubeVertsFog(WORLD_MODEL *m);
extern void TransCubeVertsClipNewVerts(WORLD_MODEL *m);
extern void TransCubeVertsFogClipNewVerts(WORLD_MODEL *m);
extern void TransCubeVertsNewVerts(WORLD_MODEL *m);
extern void TransCubeVertsFogNewVerts(WORLD_MODEL *m);
extern void TransCubeVertsMirror(WORLD_MODEL *m);
extern void TransCubeVertsMirrorNewVerts(WORLD_MODEL *m);
extern void DrawCubePolysNearClip(WORLD_MODEL *m, long lit);
extern void DrawCubePolysClip(WORLD_MODEL *m, long lit);
extern void DrawCubePolys(WORLD_MODEL *m, long lit);
extern void DrawCubePolysMirror(WORLD_MODEL *m, long lit);
extern void ProcessTextureAnimations(void);

// globals

extern WORLD World;
extern long Wireframe;
extern short WorldBigCubeCount, WorldCubeCount, WorldPolyCount, WorldClipCount;

#endif // WORLD_H

