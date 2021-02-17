//-----------------------------------------------------------------------------
// File: draw.cpp
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"
#include "draw.h"
#include "dx.h"
#include "main.h"
#include "world.h"
#include "camera.h"
#include "geom.h"
#include "model.h"
#include "text.h"
#include "texture.h"
#include "LevelLoad.h"
#include "mirror.h"
#include "timing.h"
#include "input.h"
#include "pickup.h"

extern unsigned long NPhysicsLoops;

// globals

BUCKET_ENV BucketEnvStill, BucketEnvStillFog, BucketEnvStillClip, BucketEnvStillClipFog;
BUCKET_ENV BucketEnvRoll, BucketEnvRollFog, BucketEnvRollClip, BucketEnvRollClipFog;
BUCKET_TEX0 BucketRGB, BucketFogRGB, BucketClipRGB, BucketClipFogRGB;
BUCKET_TEX1 Bucket[MAX_POLY_BUCKETS], BucketFog[MAX_POLY_BUCKETS], BucketClip[MAX_POLY_BUCKETS], BucketClipFog[MAX_POLY_BUCKETS];
VERTEX_TEX0 DrawVertsTEX0[8];
VERTEX_TEX1 DrawVertsTEX1[256];
VERTEX_TEX2 DrawVertsTEX2[8];
DRAW_SEMI_POLY SemiPoly[MAX_SEMI_POLYS];
WORLD_MESH_FX WorldMeshFx[MAX_WORLD_MESH_FX];
MODEL_MESH_FX ModelMeshFx[MAX_MODEL_MESH_FX];
long SemiCount, WorldMeshFxCount, ModelMeshFxCount;

static DRAW_SEMI_POLY *SemiOT[SEMI_POLY_OTSIZE];
static unsigned short ClipVertNum, ClipVertFree;
static unsigned short ClipVertList[2][8];
static VERTEX_TEX1 ClipVert[32];
static long Poly3dCount;
static DRAW_3D_POLY Poly3d[MAX_3D_POLYS];
static VEC JumpSparkOffset[JUMPSPARK_OFFSET_NUM];
static long JumpSparkTime, JumpSparkOff, JumpSparkSinTime;
static float JumpSparkSinDiv;

// fade shit

static VERTEX_TEX0 FadeVert[4];
static long FadeEffect;
static float FadeTime;

// Film lines
#define FILMLINE_MAX    8
t_FilmLine gFilmLines[FILMLINE_MAX];
VERTEX_TEX1*    gpFilmLineVert;
int             gcFilmLineVert;
int             gcFilmLineIndex;
unsigned short  giFilmLineIndex[FILMLINE_MAX * 2 * 3];
unsigned short* gpFilmLineIndex;

// semi shell gaps

static long SemiShellGap[] = {13, 4, 1};

////////////////////////
// init poly buckets //
////////////////////////

void InitPolyBuckets(void)
{
    long i;

// env

    BucketEnvStill.CurrentIndex = BucketEnvStill.Index;
    BucketEnvStill.CurrentVerts = BucketEnvStill.Verts;

    BucketEnvStillFog.CurrentIndex = BucketEnvStillFog.Index;
    BucketEnvStillFog.CurrentVerts = BucketEnvStillFog.Verts;

    BucketEnvStillClip.CurrentIndex = BucketEnvStillClip.Index;
    BucketEnvStillClip.CurrentVerts = BucketEnvStillClip.Verts;

    BucketEnvStillClipFog.CurrentIndex = BucketEnvStillClipFog.Index;
    BucketEnvStillClipFog.CurrentVerts = BucketEnvStillClipFog.Verts;

    BucketEnvRoll.CurrentIndex = BucketEnvRoll.Index;
    BucketEnvRoll.CurrentVerts = BucketEnvRoll.Verts;

    BucketEnvRollFog.CurrentIndex = BucketEnvRollFog.Index;
    BucketEnvRollFog.CurrentVerts = BucketEnvRollFog.Verts;

    BucketEnvRollClip.CurrentIndex = BucketEnvRollClip.Index;
    BucketEnvRollClip.CurrentVerts = BucketEnvRollClip.Verts;

    BucketEnvRollClipFog.CurrentIndex = BucketEnvRollClipFog.Index;
    BucketEnvRollClipFog.CurrentVerts = BucketEnvRollClipFog.Verts;

// rgb

    BucketRGB.CurrentIndex = BucketRGB.Index;
    BucketRGB.CurrentVerts = BucketRGB.Verts;

    BucketFogRGB.CurrentIndex = BucketFogRGB.Index;
    BucketFogRGB.CurrentVerts = BucketFogRGB.Verts;

    BucketClipRGB.CurrentIndex = BucketClipRGB.Index;
    BucketClipRGB.CurrentVerts = BucketClipRGB.Verts;

    BucketClipFogRGB.CurrentIndex = BucketClipFogRGB.Index;
    BucketClipFogRGB.CurrentVerts = BucketClipFogRGB.Verts;

// textured

    for (i = 0 ; i < MAX_POLY_BUCKETS ; i++)
    {
        Bucket[i].CurrentIndex = Bucket[i].Index;
        Bucket[i].CurrentVerts = Bucket[i].Verts;

        BucketFog[i].CurrentIndex = BucketFog[i].Index;
        BucketFog[i].CurrentVerts = BucketFog[i].Verts;

        BucketClip[i].CurrentIndex = BucketClip[i].Index;
        BucketClip[i].CurrentVerts = BucketClip[i].Verts;

        BucketClipFog[i].CurrentIndex = BucketClipFog[i].Index;
        BucketClipFog[i].CurrentVerts = BucketClipFog[i].Verts;
    }
}

///////////////////////
// kill poly buckets //
///////////////////////

void KillPolyBuckets(void)
{
}

////////////////////////
// flush poly buckets //
////////////////////////

void FlushPolyBuckets(void)
{
    short i;

// rgb

    if (BucketRGB.CurrentVerts != BucketRGB.Verts)
    {
        FOG_OFF();
        SET_TPAGE(-1);
        FlushOneBucketTEX0(&BucketRGB, FALSE);
    }

    if (BucketFogRGB.CurrentVerts != BucketFogRGB.Verts)
    {
        FOG_ON();
        SET_TPAGE(-1);
        FlushOneBucketTEX0(&BucketFogRGB, FALSE);
    }

    if (BucketClipRGB.CurrentVerts != BucketClipRGB.Verts)
    {
        FOG_OFF();
        SET_TPAGE(-1);
        FlushOneBucketTEX0(&BucketClipRGB, TRUE);
    }

    if (BucketClipFogRGB.CurrentVerts != BucketClipFogRGB.Verts)
    {
        FOG_ON();
        SET_TPAGE(-1);
        FlushOneBucketTEX0(&BucketClipFogRGB, TRUE);
    }

// textured

    for (i = 0 ; i < MAX_POLY_BUCKETS ; i++)
    {
        if (Bucket[i].CurrentVerts != Bucket[i].Verts)
        {
            FOG_OFF();
            SET_TPAGE(i);
            FlushOneBucketTEX1(&Bucket[i], FALSE);
        }

        if (BucketFog[i].CurrentVerts != BucketFog[i].Verts)
        {
            FOG_ON();
            SET_TPAGE(i);
            FlushOneBucketTEX1(&BucketFog[i], FALSE);
        }

        if (BucketClip[i].CurrentVerts != BucketClip[i].Verts)
        {
            FOG_OFF();
            SET_TPAGE(i);
            FlushOneBucketTEX1(&BucketClip[i], TRUE);
        }

        if (BucketClipFog[i].CurrentVerts != BucketClipFog[i].Verts)
        {
            FOG_ON();
            SET_TPAGE(i);
            FlushOneBucketTEX1(&BucketClipFog[i], TRUE);
        }
    }

// reset states

    FOG_OFF();
}

///////////////////////
// flush env buckets //
///////////////////////

void FlushEnvBuckets(void)
{

// set env render states

    ZWRITE_OFF();
    BLEND_ON();

    BLEND_SRC(D3DBLEND_ONE);
    BLEND_DEST(D3DBLEND_ONE);

// env still

    if (BucketEnvStill.CurrentVerts != BucketEnvStill.Verts)
    {
        FOG_OFF();
        SET_TPAGE(TPAGE_ENVSTILL);
        FlushOneBucketEnv(&BucketEnvStill, FALSE);
    }

    if (BucketEnvStillFog.CurrentVerts != BucketEnvStillFog.Verts)
    {
        FOG_ON();
        SET_TPAGE(TPAGE_ENVSTILL);
        FlushOneBucketEnv(&BucketEnvStillFog, FALSE);
    }

    if (BucketEnvStillClip.CurrentVerts != BucketEnvStillClip.Verts)
    {
        FOG_OFF();
        SET_TPAGE(TPAGE_ENVSTILL);
        FlushOneBucketEnv(&BucketEnvStillClip, TRUE);
    }

    if (BucketEnvStillClipFog.CurrentVerts != BucketEnvStillClipFog.Verts)
    {
        FOG_ON();
        SET_TPAGE(TPAGE_ENVSTILL);
        FlushOneBucketEnv(&BucketEnvStillClipFog, TRUE);
    }

// env roll

    if (BucketEnvRoll.CurrentVerts != BucketEnvRoll.Verts)
    {
        FOG_OFF();
        SET_TPAGE(TPAGE_ENVROLL);
        FlushOneBucketEnv(&BucketEnvRoll, FALSE);
    }

    if (BucketEnvRollFog.CurrentVerts != BucketEnvRollFog.Verts)
    {
        FOG_ON();
        SET_TPAGE(TPAGE_ENVROLL);
        FlushOneBucketEnv(&BucketEnvRollFog, FALSE);
    }

    if (BucketEnvRollClip.CurrentVerts != BucketEnvRollClip.Verts)
    {
        FOG_OFF();
        SET_TPAGE(TPAGE_ENVROLL);
        FlushOneBucketEnv(&BucketEnvRollClip, TRUE);
    }

    if (BucketEnvRollClipFog.CurrentVerts != BucketEnvRollClipFog.Verts)
    {
        FOG_ON();
        SET_TPAGE(TPAGE_ENVROLL);
        FlushOneBucketEnv(&BucketEnvRollClipFog, TRUE);
    }

// reset states

    FOG_OFF();
    BLEND_OFF();
    ZWRITE_ON();
}

//////////////////////
// flush one bucket //
//////////////////////

void FlushOneBucketTEX0(BUCKET_TEX0 *bucket, long clip)
{
//$REMOVED
//    DWORD flag;
//
//    if (clip) flag = D3DDP_DONOTUPDATEEXTENTS;
//    else flag = D3DDP_DONOTUPDATEEXTENTS | D3DDP_DONOTCLIP;
//$END_REMOVAL

    SET_RENDER_STATE(D3DRENDERSTATE_ALPHATESTENABLE, FALSE);
    DRAW_PRIM_INDEX(D3DPT_TRIANGLELIST, FVF_TEX0, bucket->Verts, (DWORD)(bucket->CurrentVerts - bucket->Verts), bucket->Index, (DWORD)(bucket->CurrentIndex - bucket->Index), flag);
    SET_RENDER_STATE(D3DRENDERSTATE_ALPHATESTENABLE, TRUE);

    //$CMP_NOTE: set 'curr' values back to start of the arrays
    bucket->CurrentIndex = bucket->Index;
    bucket->CurrentVerts = bucket->Verts;
}

//////////////////////
// flush one bucket //
//////////////////////

void FlushOneBucketTEX1(BUCKET_TEX1 *bucket, long clip)
{
//$REMOVED
//    DWORD flag;
//
//    if (clip) flag = D3DDP_DONOTUPDATEEXTENTS;
//    else flag = D3DDP_DONOTUPDATEEXTENTS | D3DDP_DONOTCLIP;
//$END_REMOVAL

    DRAW_PRIM_INDEX(D3DPT_TRIANGLELIST, FVF_TEX1, bucket->Verts, (DWORD)(bucket->CurrentVerts - bucket->Verts), bucket->Index, (DWORD)(bucket->CurrentIndex - bucket->Index), flag);

    //$CMP_NOTE: set 'curr' values back to start of the arrays
    bucket->CurrentIndex = bucket->Index;
    bucket->CurrentVerts = bucket->Verts;
}

//////////////////////
// flush one bucket //
//////////////////////

void FlushOneBucketEnv(BUCKET_ENV *bucket, long clip)
{
//$REMOVED
//    DWORD flag;
//
//    if (clip) flag = D3DDP_DONOTUPDATEEXTENTS;
//    else flag = D3DDP_DONOTUPDATEEXTENTS | D3DDP_DONOTCLIP;
//$END_REMOVAL

    DRAW_PRIM_INDEX(D3DPT_TRIANGLELIST, FVF_TEX1, bucket->Verts, (DWORD)(bucket->CurrentVerts - bucket->Verts), bucket->Index, (DWORD)(bucket->CurrentIndex - bucket->Index), flag);

    //$CMP_NOTE: set 'curr' values back to start of the arrays
    bucket->CurrentIndex = bucket->Index;
    bucket->CurrentVerts = bucket->Verts;
}

//////////////////////////
// reset 3D poly list //
//////////////////////////

void Reset3dPolyList(void)
{
    Poly3dCount = 0;
}

///////////////////////
// draw 3D poly list //
///////////////////////

void Draw3dPolyList(void)
{
    long i, j;
    DRAW_3D_POLY *poly;
    VERTEX_TEX1 *verts;

// skip if none

    if( 0 == Poly3dCount )  return;

// leap thru polys

    poly = Poly3d;

    for (i = Poly3dCount ; i ; i--, poly++)
    {

// semi

        if (poly->SemiType != -1)
        {
            if (SEMI_POLY_FREE())
            {
                SEMI_POLY_SETUP(verts, poly->Fog, poly->VertNum, poly->Tpage, TRUE, poly->SemiType);
                for (j = 0 ; j < poly->VertNum ; j++)
                {
                    RotTransPersVector(&ViewMatrixScaled, &ViewTransScaled, &poly->Pos[j], (REAL*)&verts[j]);
                    verts[j].color = poly->Verts[j].color;
                    verts[j].specular = poly->Verts[j].specular;
                    verts[j].tu = poly->Verts[j].tu;
                    verts[j].tv = poly->Verts[j].tv;
                }
            }
        }

// opaque

        else
        {
            for (j = 0 ; j < poly->VertNum ; j++)
                RotTransPersVector(&ViewMatrixScaled, &ViewTransScaled, &poly->Pos[j], (REAL*)&poly->Verts[j]);

            SET_TPAGE((short)poly->Tpage);

            if (poly->Fog)
            {
                FOG_ON();
            }
            else
            {
                FOG_OFF();
            }

            DRAW_PRIM(D3DPT_TRIANGLEFAN, FVF_TEX1, poly->Verts, poly->VertNum, D3DDP_DONOTUPDATEEXTENTS);
        }
    }

// fog off

    FOG_OFF();
}

/////////////////////////////
// add a 3d poly to list //
/////////////////////////////

DRAW_3D_POLY *Get3dPoly(void)
{

// ret if full

    if (Poly3dCount == MAX_3D_POLYS)
        return NULL;

// ret ptr to poly

    return &Poly3d[Poly3dCount++];
}

////////////////////////
// draw nearclip poly //
////////////////////////

void DrawNearClipPolyTEX0(VEC *pos, long *rgb, long vertnum)
{
    long newvertnum, i, lmul, newrgb[8];
    float mul, z0, z1, z[8];
    VEC *vec0, *vec1, newpos[8];
    MODEL_RGB *rgb0, *rgb1, *rgbout;

// clip

    newvertnum = 0;

    for (i = 0 ; i < vertnum ; i++)
        z[i] = pos[i].v[X] * ViewMatrixScaled.m[RZ] + pos[i].v[Y] * ViewMatrixScaled.m[UZ] + pos[i].v[Z] * ViewMatrixScaled.m[LZ] + ViewTransScaled.v[Z];

    for (i = 0 ; i < vertnum ; i++)
    {
        vec0 = &pos[i];
        vec1 = &pos[(i + 1) % vertnum];
        z0 = z[i];
        z1 = z[(i + 1) % vertnum];

        if (z0 >= 0)
        {
            newpos[newvertnum] = pos[i];
            newrgb[newvertnum] = rgb[i];
            newvertnum++;

            if (z1 < 0)
            {
                mul = z0 / (z0 - z1);

                newpos[newvertnum].v[X] = vec0->v[X] + (vec1->v[X] - vec0->v[X]) * mul;
                newpos[newvertnum].v[Y] = vec0->v[Y] + (vec1->v[Y] - vec0->v[Y]) * mul;
                newpos[newvertnum].v[Z] = vec0->v[Z] + (vec1->v[Z] - vec0->v[Z]) * mul;

                FTOL(mul * 256, lmul);

                rgb0 = (MODEL_RGB*)&rgb[i];
                rgb1 = (MODEL_RGB*)&rgb[(i + 1) % vertnum];
                rgbout = (MODEL_RGB*)&newrgb[newvertnum];

                rgbout->a = rgb0->a + (unsigned char)(((rgb1->a - rgb0->a) * lmul) >> 8);
                rgbout->r = rgb0->r + (unsigned char)(((rgb1->r - rgb0->r) * lmul) >> 8);
                rgbout->g = rgb0->g + (unsigned char)(((rgb1->g - rgb0->g) * lmul) >> 8);
                rgbout->b = rgb0->b + (unsigned char)(((rgb1->b - rgb0->b) * lmul) >> 8);

                newvertnum++;
            }
        }
        else
        {
            if (z1 >= RenderSettings.NearClip)
            {
                mul = z1 / (z1 - z0);

                newpos[newvertnum].v[X] = vec1->v[X] + (vec0->v[X] - vec1->v[X]) * mul;
                newpos[newvertnum].v[Y] = vec1->v[Y] + (vec0->v[Y] - vec1->v[Y]) * mul;
                newpos[newvertnum].v[Z] = vec1->v[Z] + (vec0->v[Z] - vec1->v[Z]) * mul;

                FTOL(mul * 256, lmul);

                rgb0 = (MODEL_RGB*)&rgb[(i + 1) % vertnum];
                rgb1 = (MODEL_RGB*)&rgb[i];
                rgbout = (MODEL_RGB*)&newrgb[newvertnum];

                rgbout->a = rgb0->a + (unsigned char)(((rgb1->a - rgb0->a) * lmul) >> 8);
                rgbout->r = rgb0->r + (unsigned char)(((rgb1->r - rgb0->r) * lmul) >> 8);
                rgbout->g = rgb0->g + (unsigned char)(((rgb1->g - rgb0->g) * lmul) >> 8);
                rgbout->b = rgb0->b + (unsigned char)(((rgb1->b - rgb0->b) * lmul) >> 8);

                newvertnum++;
            }
        }
    }

    if (!newvertnum)
        return;

// setup verts

    for (i = 0 ; i < newvertnum ; i++)
    {
        RotTransPersVector(&ViewMatrixScaled, &ViewTransScaled, &newpos[i], &DrawVertsTEX0[i].sx);
        DrawVertsTEX0[i].color = newrgb[i];
    }

// draw

    SET_RENDER_STATE(D3DRENDERSTATE_ALPHATESTENABLE, FALSE);
    DRAW_PRIM(D3DPT_TRIANGLEFAN, FVF_TEX0, DrawVertsTEX0, newvertnum, D3DDP_DONOTUPDATEEXTENTS);
    SET_RENDER_STATE(D3DRENDERSTATE_ALPHATESTENABLE, TRUE);
}

//////////////////////////
// reset semi poly list //
//////////////////////////

void ResetSemiList(void)
{
    SemiCount = 0;
}

/////////////////////////
// draw semi poly list //
/////////////////////////

void DrawSemiList(void)
{
    long i, j, lz;
    float z, otmul;
    DRAW_SEMI_POLY *poly;

// skip if none

    if( 0 == SemiCount )  return;

// setup misc render states

    BLEND_ON();
    ZWRITE_OFF();

// setup ot list + spacing

    otmul = SEMI_POLY_OTSIZE / RenderSettings.FarClip;

    for (i = 0 ; i < SEMI_POLY_OTSIZE ; i++)
    {
        SemiOT[i] = NULL;
    }

// insert each poly into OT slot

    poly = SemiPoly;

    for (i = SemiCount ; i ; i--, poly++)
    {
        z = 0.0f;

        if (poly->Tpage == -1)
            for (j = 0 ; j < poly->VertNum ; j++) z += 1.0f / poly->VertsRGB[j].rhw;
        else
            for (j = 0 ; j < poly->VertNum ; j++) z += 1.0f / poly->Verts[j].rhw;

        lz = (SEMI_POLY_OTSIZE - 1) - (long)((z / j + poly->z) * otmul);
        if (lz > (SEMI_POLY_OTSIZE - 1)) lz = (SEMI_POLY_OTSIZE - 1);
        if (lz >= 0)
        {
            poly->Next = SemiOT[lz];
            SemiOT[lz] = poly;
        }
    }

// draw

    for (i = 0 ; i < SEMI_POLY_OTSIZE ; i++) for (poly = SemiOT[i] ; poly ; poly = poly->Next)
    {

// set render states

        SET_TPAGE((short)poly->Tpage);

        if (poly->Fog)
        {
            FOG_ON();
        }
        else
        {
            FOG_OFF();
        }

        if (!poly->SemiType)
        {
            BLEND_ALPHA();
            BLEND_SRC(D3DBLEND_SRCALPHA);
            BLEND_DEST(D3DBLEND_INVSRCALPHA);
        }
        else if (poly->SemiType == 1)
        {
            BLEND_ON();
            BLEND_SRC(D3DBLEND_ONE);
            BLEND_DEST(D3DBLEND_ONE);
        }
        else
        {
            BLEND_ON();
            BLEND_SRC(D3DBLEND_ZERO);
            BLEND_DEST(D3DBLEND_INVSRCCOLOR);
        }

// draw

        if (poly->Tpage == -1)
        {
            SET_RENDER_STATE(D3DRENDERSTATE_ALPHATESTENABLE, FALSE);
            DRAW_PRIM(D3DPT_TRIANGLEFAN, FVF_TEX0, poly->VertsRGB, poly->VertNum, poly->DrawFlag);
            SET_RENDER_STATE(D3DRENDERSTATE_ALPHATESTENABLE, TRUE);
        }
        else
        {
            DRAW_PRIM(D3DPT_TRIANGLEFAN, FVF_TEX1, poly->Verts, poly->VertNum, poly->DrawFlag);
        }
    }

// fog off

    FOG_OFF();
}

////////////////////////////////////////
// draw a 'face me' poly + reflection //
////////////////////////////////////////

void DrawFacingPolyMirror(VEC *pos, FACING_POLY *poly, long semi, float zbias)
{
    float screen[4];
    float xadd, yadd;
    float z, fog, mirrorfog;
    VERTEX_TEX1 *vert;
    float mirroradd, mirrory;

// draw original

    DrawFacingPoly(pos, poly, semi, zbias);

// reflect?

    if (!RenderSettings.Mirror)
        return;

    if (!GetMirrorPlane(pos))
        return;

    if (ViewCameraPos.v[Y] >= MirrorHeight)
        return;

    mirroradd = MirrorHeight - pos->v[Y];
    if (mirroradd <= -MIRROR_OVERLAP_TOL)
        return;

// yep

    mirrory = MirrorHeight + mirroradd;

// get screen coors

    z = pos->v[X] * ViewMatrixScaled.m[RZ] + mirrory * ViewMatrixScaled.m[UZ] + pos->v[Z] * ViewMatrixScaled.m[LZ] + ViewTransScaled.v[Z];
    if (z < RenderSettings.NearClip || z >= RenderSettings.FarClip)
        return;

    screen[0] = (pos->v[X] * ViewMatrixScaled.m[RX] + mirrory * ViewMatrixScaled.m[UX] + pos->v[Z] * ViewMatrixScaled.m[LX] + ViewTransScaled.v[X]) / z + RenderSettings.GeomCentreX;
    screen[1] = (pos->v[X] * ViewMatrixScaled.m[RY] + mirrory * ViewMatrixScaled.m[UY] + pos->v[Z] * ViewMatrixScaled.m[LY] + ViewTransScaled.v[Y]) / z + RenderSettings.GeomCentreY;

    screen[3] = 1 / z;
    screen[2] = GET_ZBUFFER(z + zbias);

// get verts

    if (semi == -1)
    {
        vert = DrawVertsTEX1;
    }
    else
    {
        if (!SEMI_POLY_FREE()) return;
        SEMI_POLY_SETUP(vert, TRUE, 4, poly->Tpage, TRUE, semi);
    }

// get xy adds

    xadd = (poly->Xsize * RenderSettings.MatScaleX) / z;
    yadd = (poly->Ysize * RenderSettings.MatScaleY) / z;

// build 4 from one

    vert[0].sx = vert[3].sx = screen[0] - xadd;
    vert[1].sx = vert[2].sx = screen[0] + xadd;

    vert[0].sy = vert[1].sy = screen[1] + yadd;
    vert[2].sy = vert[3].sy = screen[1] - yadd;

    vert[0].sz = vert[1].sz = vert[2].sz = vert[3].sz = screen[2];
    vert[0].rhw = vert[1].rhw = vert[2].rhw = vert[3].rhw = screen[3];

// set RGB's

    vert[0].color = vert[1].color = vert[2].color = vert[3].color = poly->RGB;

// set UV's

    vert[0].tu = vert[3].tu = poly->U;
    vert[1].tu = vert[2].tu = poly->U + poly->Usize;

    vert[0].tv = vert[1].tv = poly->V;
    vert[2].tv = vert[3].tv = poly->V + poly->Vsize;

// set fog

    mirrorfog = GET_MIRROR_FOG(mirroradd);
    if (mirrorfog < 0) mirrorfog = 0;

    fog = (RenderSettings.FarClip - z) * RenderSettings.FogMul;
    if (fog > 255) fog = 255;
    fog -= mirrorfog;
    if (fog < 0) fog = 0;
    vert[0].specular = vert[1].specular = vert[2].specular = vert[3].specular = FTOL3(fog) << 24;

// draw

    if (semi == -1)
    {
        SET_TPAGE(poly->Tpage);
        DRAW_PRIM(D3DPT_TRIANGLEFAN, FVF_TEX1, vert, 4, D3DDP_DONOTUPDATEEXTENTS);
    }
}

///////////////////////////
// draw a 'face me' poly //
///////////////////////////

void DrawFacingPoly(VEC *pos, FACING_POLY *poly, long semi, float zbias)
{
    float screen[4];
    float xadd, yadd;
    float z, fog;
    VERTEX_TEX1 *vert;

// get screen coors

    z = pos->v[X] * ViewMatrixScaled.m[RZ] + pos->v[Y] * ViewMatrixScaled.m[UZ] + pos->v[Z] * ViewMatrixScaled.m[LZ] + ViewTransScaled.v[Z];
    if (z < RenderSettings.NearClip || z >= RenderSettings.FarClip)
        return;

    screen[0] = (pos->v[X] * ViewMatrixScaled.m[RX] + pos->v[Y] * ViewMatrixScaled.m[UX] + pos->v[Z] * ViewMatrixScaled.m[LX] + ViewTransScaled.v[X]) / z + RenderSettings.GeomCentreX;
    screen[1] = (pos->v[X] * ViewMatrixScaled.m[RY] + pos->v[Y] * ViewMatrixScaled.m[UY] + pos->v[Z] * ViewMatrixScaled.m[LY] + ViewTransScaled.v[Y]) / z + RenderSettings.GeomCentreY;

    screen[3] = 1 / z;
    screen[2] = GET_ZBUFFER(z + zbias);

// get verts

    if (semi == -1)
    {
        vert = DrawVertsTEX1;
    }
    else
    {
        if (!SEMI_POLY_FREE()) return;
        SEMI_POLY_SETUP(vert, TRUE, 4, poly->Tpage, TRUE, semi);
    }

// get xy adds

    xadd = (poly->Xsize * RenderSettings.MatScaleX) / z;
    yadd = (poly->Ysize * RenderSettings.MatScaleY) / z;

// build 4 from one

    vert[0].sx = vert[3].sx = screen[0] - xadd;
    vert[1].sx = vert[2].sx = screen[0] + xadd;

    vert[0].sy = vert[1].sy = screen[1] - yadd;
    vert[2].sy = vert[3].sy = screen[1] + yadd;

    vert[0].sz = vert[1].sz = vert[2].sz = vert[3].sz = screen[2];
    vert[0].rhw = vert[1].rhw = vert[2].rhw = vert[3].rhw = screen[3];

// set RGB's

    vert[0].color = vert[1].color = vert[2].color = vert[3].color = poly->RGB;

// set UV's

    vert[0].tu = vert[3].tu = poly->U;
    vert[1].tu = vert[2].tu = poly->U + poly->Usize;

    vert[0].tv = vert[1].tv = poly->V;
    vert[2].tv = vert[3].tv = poly->V + poly->Vsize;

// set fog

    fog = (RenderSettings.FarClip - z) * RenderSettings.FogMul;
    if (fog > 255) fog = 255;
    else if (fog < 0) fog = 0;
    vert[0].specular = vert[1].specular = vert[2].specular = vert[3].specular = FTOL3(fog) << 24;

// draw

    if (semi == -1)
    {
        SET_TPAGE(poly->Tpage);
        DRAW_PRIM(D3DPT_TRIANGLEFAN, FVF_TEX1, vert, 4, D3DDP_DONOTUPDATEEXTENTS);
    }
}

//////////////////////////////////////////////////////
// draw a 'face me' poly with rotation + reflection //
//////////////////////////////////////////////////////

void DrawFacingPolyRotMirror(VEC *pos, MAT *mat, FACING_POLY *poly, long semi, float zbias)
{
    float fog, mirrorfog;
    VEC trans;
    VEC v0, v1, v2, v3;
    MAT mat2, mat3, mat4;
    VERTEX_TEX1 *vert;
    float mirroradd, mirrory;

// draw original

    DrawFacingPolyRot(pos, mat, poly, semi, zbias);

// reflect?

    if (!RenderSettings.Mirror)
        return;

    if (!GetMirrorPlane(pos))
        return;

    if (ViewCameraPos.v[Y] >= MirrorHeight)
        return;

    mirroradd = MirrorHeight - pos->v[Y];
    if (mirroradd <= -MIRROR_OVERLAP_TOL)
        return;

// yep

    mirrory = MirrorHeight + mirroradd;

// get vector translation

    trans.v[Z] = pos->v[X] * ViewMatrixScaled.m[RZ] + mirrory * ViewMatrixScaled.m[UZ] + pos->v[Z] * ViewMatrixScaled.m[LZ] + ViewTransScaled.v[Z];
    if (trans.v[Z] < RenderSettings.NearClip - 128 || trans.v[Z] >= RenderSettings.FarClip + 128)
        return;

    trans.v[X] = pos->v[X] * ViewMatrixScaled.m[RX] + mirrory * ViewMatrixScaled.m[UX] + pos->v[Z] * ViewMatrixScaled.m[LX] + ViewTransScaled.v[X];
    trans.v[Y] = pos->v[X] * ViewMatrixScaled.m[RY] + mirrory * ViewMatrixScaled.m[UY] + pos->v[Z] * ViewMatrixScaled.m[LY] + ViewTransScaled.v[Y];

// get verts

    if (semi == -1)
    {
        vert = DrawVertsTEX1;
    }
    else
    {
        if (!SEMI_POLY_FREE()) return;
        SEMI_POLY_SETUP(vert, TRUE, 4, poly->Tpage, TRUE, semi);
    }

// set mat

    CopyMat(mat, &mat4);
    mat4.m[UX] = -mat4.m[UX];
    mat4.m[UY] = -mat4.m[UY];
    mat4.m[UZ] = -mat4.m[UZ];

    BuildLookMatrixForward(pos, &ViewCameraPos, &mat2);
    MulMatrix(&ViewMatrixScaled, &mat2, &mat3);
    MulMatrix(&mat3, &mat4, &mat2);

// setup 4 vectors

    SetVector(&v0, -poly->Xsize, -poly->Ysize, 0);
    SetVector(&v1, poly->Xsize, -poly->Ysize, 0);
    SetVector(&v2, poly->Xsize, poly->Ysize, 0);
    SetVector(&v3, -poly->Xsize, poly->Ysize, 0);

// get screen coors

    RotTransPersVectorZbias(&mat2, &trans, &v0, &vert[0].sx, zbias);
    RotTransPersVectorZbias(&mat2, &trans, &v1, &vert[1].sx, zbias);
    RotTransPersVectorZbias(&mat2, &trans, &v2, &vert[2].sx, zbias);
    RotTransPersVectorZbias(&mat2, &trans, &v3, &vert[3].sx, zbias);

// set RGB's

    vert[0].color = vert[1].color = vert[2].color = vert[3].color = poly->RGB;

// set UV's

    vert[0].tu = vert[3].tu = poly->U;
    vert[1].tu = vert[2].tu = poly->U + poly->Usize;

    vert[0].tv = vert[1].tv = poly->V;
    vert[2].tv = vert[3].tv = poly->V + poly->Vsize;

// set fog

    mirrorfog = GET_MIRROR_FOG(mirroradd);
    if (mirrorfog < 0) mirrorfog = 0;

    fog = (RenderSettings.FarClip - 1 / vert[0].rhw) * RenderSettings.FogMul;
    if (fog > 255) fog = 255;
    fog -= mirrorfog;
    if (fog < 0) fog = 0;
    vert[0].specular = FTOL3(fog) << 24;

    fog = (RenderSettings.FarClip - 1 / vert[1].rhw) * RenderSettings.FogMul;
    if (fog > 255) fog = 255;
    fog -= mirrorfog;
    if (fog < 0) fog = 0;
    vert[1].specular = FTOL3(fog) << 24;

    fog = (RenderSettings.FarClip - 1 / vert[2].rhw) * RenderSettings.FogMul;
    if (fog > 255) fog = 255;
    fog -= mirrorfog;
    if (fog < 0) fog = 0;
    vert[2].specular = FTOL3(fog) << 24;

    fog = (RenderSettings.FarClip - 1 / vert[3].rhw) * RenderSettings.FogMul;
    if (fog > 255) fog = 255;
    fog -= mirrorfog;
    if (fog < 0) fog = 0;
    vert[3].specular = FTOL3(fog) << 24;

// draw

    if (semi == -1)
    {
        SET_TPAGE(poly->Tpage);
        DRAW_PRIM(D3DPT_TRIANGLEFAN, FVF_TEX1, vert, 4, D3DDP_DONOTUPDATEEXTENTS);
    }
}

/////////////////////////////////////////
// draw a 'face me' poly with rotation //
/////////////////////////////////////////

void DrawFacingPolyRot(VEC *pos, MAT *mat, FACING_POLY *poly, long semi, float zbias)
{
    float fog;
    VEC trans;
    VEC v0, v1, v2, v3;
    MAT mat2, mat3;
    VERTEX_TEX1 *vert;

// get vector translation

    trans.v[Z] = pos->v[X] * ViewMatrixScaled.m[RZ] + pos->v[Y] * ViewMatrixScaled.m[UZ] + pos->v[Z] * ViewMatrixScaled.m[LZ] + ViewTransScaled.v[Z];
    if (trans.v[Z] < RenderSettings.NearClip - 128 || trans.v[Z] >= RenderSettings.FarClip + 128)
        return;

    trans.v[X] = pos->v[X] * ViewMatrixScaled.m[RX] + pos->v[Y] * ViewMatrixScaled.m[UX] + pos->v[Z] * ViewMatrixScaled.m[LX] + ViewTransScaled.v[X];
    trans.v[Y] = pos->v[X] * ViewMatrixScaled.m[RY] + pos->v[Y] * ViewMatrixScaled.m[UY] + pos->v[Z] * ViewMatrixScaled.m[LY] + ViewTransScaled.v[Y];

// get verts

    if (semi == -1)
    {
        vert = DrawVertsTEX1;
    }
    else
    {
        if (!SEMI_POLY_FREE()) return;
        SEMI_POLY_SETUP(vert, TRUE, 4, poly->Tpage, TRUE, semi);
    }

// set mat

    BuildLookMatrixForward(pos, &ViewCameraPos, &mat2);
    MulMatrix(&ViewMatrixScaled, &mat2, &mat3);
    MulMatrix(&mat3, mat, &mat2);

// setup 4 vectors

    SetVector(&v0, -poly->Xsize, -poly->Ysize, 0);
    SetVector(&v1, poly->Xsize, -poly->Ysize, 0);
    SetVector(&v2, poly->Xsize, poly->Ysize, 0);
    SetVector(&v3, -poly->Xsize, poly->Ysize, 0);

// get screen coors

    RotTransPersVectorZbias(&mat2, &trans, &v0, &vert[0].sx, zbias);
    RotTransPersVectorZbias(&mat2, &trans, &v1, &vert[1].sx, zbias);
    RotTransPersVectorZbias(&mat2, &trans, &v2, &vert[2].sx, zbias);
    RotTransPersVectorZbias(&mat2, &trans, &v3, &vert[3].sx, zbias);

// set RGB's

    vert[0].color = vert[1].color = vert[2].color = vert[3].color = poly->RGB;

// set UV's

    vert[0].tu = vert[3].tu = poly->U;
    vert[1].tu = vert[2].tu = poly->U + poly->Usize;

    vert[0].tv = vert[1].tv = poly->V;
    vert[2].tv = vert[3].tv = poly->V + poly->Vsize;

// set fog

    fog = (RenderSettings.FarClip - 1 / vert[0].rhw) * RenderSettings.FogMul;
    if (fog > 255) fog = 255;
    else if (fog < 0) fog = 0;
    vert[0].specular = FTOL3(fog) << 24;

    fog = (RenderSettings.FarClip - 1 / vert[1].rhw) * RenderSettings.FogMul;
    if (fog > 255) fog = 255;
    else if (fog < 0) fog = 0;
    vert[1].specular = FTOL3(fog) << 24;

    fog = (RenderSettings.FarClip - 1 / vert[2].rhw) * RenderSettings.FogMul;
    if (fog > 255) fog = 255;
    else if (fog < 0) fog = 0;
    vert[2].specular = FTOL3(fog) << 24;

    fog = (RenderSettings.FarClip - 1 / vert[3].rhw) * RenderSettings.FogMul;
    if (fog > 255) fog = 255;
    else if (fog < 0) fog = 0;
    vert[3].specular = FTOL3(fog) << 24;

// draw

    if (semi == -1)
    {
        SET_TPAGE(poly->Tpage);
        DRAW_PRIM(D3DPT_TRIANGLEFAN, FVF_TEX1, vert, 4, D3DDP_DONOTUPDATEEXTENTS);
    }
}

//////////////////////////////////
// init main render state stuff //
//////////////////////////////////

void InitRenderStates(void)
{

// set misc render states

    RenderTP = -2;
    RenderTP2 = -2;

    RenderBlend = 2;
    RenderBlendSrc = -1;
    RenderBlendDest = -1;
    BLEND_OFF();

    RenderFog = TRUE;
    FOG_OFF();
    FOG_COLOR(BackgroundColor);

    RenderZbuffer = D3DZB_FALSE;
    RenderZwrite = FALSE;
    RenderZcmp = D3DCMP_ALWAYS;
    ZBUFFER_ON();
    ZWRITE_ON();
    ZCMP(D3DCMP_LESSEQUAL);

// reset screen debug misc

    if (Version == VERSION_DEV)
    {
        WorldPolyCount = 0;
        ModelPolyCount = 0;
    }

    RenderStateChange = 0;
    TextureStateChange = 0;
}

//////////////////////////////
// set near / far clip vars //
//////////////////////////////

void SetNearFar(REAL n, REAL f)
{
    RenderSettings.NearClip = n;
    RenderSettings.FarClip = f;

    RenderSettings.ZedDrawDist = RenderSettings.FarClip - RenderSettings.NearClip;
    RenderSettings.ZedFarDivDist = RenderSettings.FarClip / RenderSettings.ZedDrawDist;
    RenderSettings.ZedFarMulNear = RenderSettings.FarClip * RenderSettings.NearClip;

//  RenderSettings.ZedSub = 0.0f;

//  float off = RenderSettings.NearClip / RenderSettings.FarClip;
//  float drawdist = RenderSettings.FarClip - RenderSettings.NearClip;

//  RenderSettings.ZedDrawDist = (RenderSettings.FarClip - RenderSettings.NearClip) * 0.5f;
//  RenderSettings.ZedFarDivDist = RenderSettings.FarClip / drawdist + off;
//  RenderSettings.ZedFarMulNear = (RenderSettings.FarClip * RenderSettings.NearClip) * (1.0f + off);
//  RenderSettings.ZedSub = (RenderSettings.NearClip * 0.5f - RenderSettings.NearClip) * drawdist;
}

//////////////////
// set fog vars //
//////////////////

void SetFogVars(REAL fogstart, REAL vertstart, REAL vertend)
{

// set fog vars

    RenderSettings.FogStart = fogstart;
    RenderSettings.FogDist = RenderSettings.FarClip - RenderSettings.FogStart;
    if (RenderSettings.FogDist <= 0.0f) RenderSettings.FogMul = 256.0f, RenderSettings.FogStart += 65535.0f;
    else RenderSettings.FogMul = 256.0f / RenderSettings.FogDist;

    RenderSettings.VertFogStart = vertstart;
    RenderSettings.VertFogEnd = vertend;
    if (RenderSettings.VertFogStart == RenderSettings.VertFogEnd) RenderSettings.VertFogMul = 0;
    else RenderSettings.VertFogMul = 256.0f / (RenderSettings.VertFogEnd - RenderSettings.VertFogStart);
    if (RenderSettings.VertFogMul) RenderSettings.FogStart = 0.0f;
}

///////////////////
// draw XYZ axis //
///////////////////

void DrawAxis(MAT *mat, VEC *pos)
{
    FACING_POLY fp;
    VEC in, out;

// init facing misc

    fp.Tpage = TPAGE_FONT;
    fp.Xsize = 8;
    fp.Ysize = 8;
    fp.Usize = FONT_UWIDTH / 256.0f;
    fp.Vsize = FONT_VHEIGHT / 256.0f;
    fp.RGB = 0xffffff;

// X

    SetVector(&in, 64, 0, 0);
    RotVector(mat, &in, &out);
    AddVector(&out, pos, &out);

    fp.U = 184.0f / 256.0f;
    fp.V = 16.0f / 256.0f;
    DrawFacingPoly(&out, &fp, -1, 0);

    DrawLine(pos, &out, 0xff0000, 0xff0000);

// Y

    SetVector(&in, 0, 64, 0);
    RotVector(mat, &in, &out);
    AddVector(&out, pos, &out);

    fp.U = 192.0f / 256.0f;
    fp.V = 16.0f / 256.0f;
    DrawFacingPoly(&out, &fp, -1, 0);

    DrawLine(pos, &out, 0x00ff00, 0x00ff00);

// Z

    SetVector(&in, 0, 0, 64);
    RotVector(mat, &in, &out);
    AddVector(&out, pos, &out);

    fp.U = 200.0f / 256.0f;
    fp.V = 16.0f / 256.0f;
    DrawFacingPoly(&out, &fp, -1, 0);

    DrawLine(pos, &out, 0x0000ff, 0x0000ff);
}

//////////////////////////
// dump image on screen //
//////////////////////////

void DumpImage(char handle, float x, float y, float w, float h, float u, float v, float tw, float th, unsigned long col)
{
    long i;
    float xstart, ystart, xsize, ysize;

// scale

    xstart = x * RenderSettings.GeomScaleX + ScreenLeftClip;
    ystart = y * RenderSettings.GeomScaleY + ScreenTopClip;

    xsize = w * RenderSettings.GeomScaleX;
    ysize = h * RenderSettings.GeomScaleY;

// set render states

    SET_TPAGE(handle);

// init vert misc

    for (i = 0 ; i < 4 ; i++)
    {
        DrawVertsTEX1[i].color = col;
        DrawVertsTEX1[i].rhw = 1;
    }

// set screen coors

    DrawVertsTEX1[0].sx = DrawVertsTEX1[3].sx = xstart;
    DrawVertsTEX1[1].sx = DrawVertsTEX1[2].sx = xstart + xsize;
    DrawVertsTEX1[0].sy = DrawVertsTEX1[1].sy = ystart;
    DrawVertsTEX1[2].sy = DrawVertsTEX1[3].sy = ystart + ysize;

// set uv's

    DrawVertsTEX1[0].tu = DrawVertsTEX1[3].tu = u;
    DrawVertsTEX1[1].tu = DrawVertsTEX1[2].tu = u + tw;
    DrawVertsTEX1[0].tv = DrawVertsTEX1[1].tv = v;
    DrawVertsTEX1[2].tv = DrawVertsTEX1[3].tv = v + th;

// draw

    DRAW_PRIM(D3DPT_TRIANGLEFAN, FVF_TEX1, DrawVertsTEX1, 4, D3DDP_DONOTUPDATEEXTENTS);
}

//////////////////////
// draw a mouse ptr //
//////////////////////

void DrawMousePointer(unsigned long color)
{
    char i;
    float x, y, xs, ys, tu, tv;

// init verts

    for (i = 0 ; i < 4 ; i++)
    {
        DrawVertsTEX1[i].color = color;
        DrawVertsTEX1[i].rhw = 1;
    }

// set screen coors

    x = MouseXpos * RenderSettings.GeomScaleX + ScreenLeftClip;
    y = MouseYpos * RenderSettings.GeomScaleY + ScreenTopClip;
    xs = 8 * RenderSettings.GeomScaleX;
    ys = 16 * RenderSettings.GeomScaleY;

    DrawVertsTEX1[0].sx = x;
    DrawVertsTEX1[0].sy = y;

    DrawVertsTEX1[1].sx = x + xs;
    DrawVertsTEX1[1].sy = y;

    DrawVertsTEX1[2].sx = x + xs;
    DrawVertsTEX1[2].sy = y + ys;

    DrawVertsTEX1[3].sx = x;
    DrawVertsTEX1[3].sy = y + ys;

// set uv's

    tu = 241.0f;
    tv = 32.0f;

    DrawVertsTEX1[0].tu = tu / 256;
    DrawVertsTEX1[0].tv = tv / 256;

    DrawVertsTEX1[1].tu = (tu + 8) / 256;
    DrawVertsTEX1[1].tv = tv / 256;

    DrawVertsTEX1[2].tu = (tu + 8) / 256;
    DrawVertsTEX1[2].tv = (tv + 16) / 256;

    DrawVertsTEX1[3].tu = tu / 256;
    DrawVertsTEX1[3].tv = (tv + 16) / 256;

// draw

    DRAW_PRIM(D3DPT_TRIANGLEFAN, FVF_TEX1, DrawVertsTEX1, 4, D3DDP_DONOTUPDATEEXTENTS);
}

///////////////////
// load a bitmap //
///////////////////

//$MODIFIED
//BOOL LoadBitmap(char *bitmap, HBITMAP *hbm)
//{
//    *hbm = (HBITMAP)LoadImage(NULL, bitmap, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
//    return (BOOL)*hbm;
//}
BOOL LoadBitmap(char* strFilename, D3DTexture** ppBitmap)
{
    //$PERF: should not be using D3DXCreateTextureFromFile at runtime!
    HRESULT hr = D3DXCreateTextureFromFileExA( D3Ddevice, strFilename,
                                               D3DX_DEFAULT, D3DX_DEFAULT, 1, // dimensions
                                               0, D3DFMT_LIN_A8R8G8B8, NULL, D3DX_DEFAULT, D3DX_DEFAULT,
                                               0x00000000, NULL, NULL, ppBitmap );
    return( SUCCEEDED(hr) );
}
//$END_MODIFICATIONS

/////////////////
// free bitmap //
/////////////////

//$MODIFIED
//BOOL FreeBitmap(HBITMAP hbm)
//{
//    return (BOOL)DeleteObject(hbm);
//}
BOOL FreeBitmap(D3DTexture* pBitmap)
{
    RELEASE( pBitmap );
    return TRUE; // success
}
//$END_MODIFICATIONS

////////////////////////////////////////////
// blit from a bitmap handle to a surface //
////////////////////////////////////////////

//$MODIFIED
//BOOL BlitBitmap(HBITMAP hbm, IDirectDrawSurface4 **surface)
//{
//    HRESULT r;
//    BITMAP bm;
//    HDC dcimage, dc;
//
//// get bitmap info
//
//    GetObject(hbm, sizeof(bm), &bm);
//
//// get  dc's
//
//    dcimage = CreateCompatibleDC(NULL);
//    SelectObject(dcimage, hbm);
//    r = (*surface)->GetDC(&dc);
//
//// blit
//
//    if (r == DD_OK)
//        r = StretchBlt(dc, 0, 0, ScreenXsize, ScreenYsize, dcimage, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
//
//// free dc's
//
//    (*surface)->ReleaseDC(dc);
//    DeleteDC(dcimage);
//
//// return
//
//    return (r == DD_OK);
//}
BOOL BlitBitmap(D3DTexture* pBitmap)
{
    struct VERTEX { D3DXVECTOR4 p; FLOAT tu, tv; };
    VERTEX v[4];
    v[0].p = D3DXVECTOR4(   0 - 0.5f,   0 - 0.5f, 0, 0 );  v[0].tu =   0;  v[0].tv =   0;
    v[1].p = D3DXVECTOR4( 640 - 0.5f,   0 - 0.5f, 0, 0 );  v[1].tu = 1024; v[1].tv =   0;
    v[2].p = D3DXVECTOR4( 640 - 0.5f, 480 - 0.5f, 0, 0 );  v[2].tu = 1024; v[2].tv = 512;
    v[3].p = D3DXVECTOR4(   0 - 0.5f, 480 - 0.5f, 0, 0 );  v[3].tu =   0;  v[3].tv = 512;
    //$BUGBUG: do we really want to assume a 1024x512 texture here ??

    // Set state to render the image
    D3DDevice_SetTexture( 0, pBitmap );
    D3DDevice_SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
    D3DDevice_SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
    D3DDevice_SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );
    D3DDevice_SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE );
    D3DDevice_SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );
    D3DDevice_SetTextureStageState( 0, D3DTSS_ADDRESSU,  D3DTADDRESS_CLAMP );
    D3DDevice_SetTextureStageState( 0, D3DTSS_ADDRESSV,  D3DTADDRESS_CLAMP );
    D3DDevice_SetRenderState( D3DRS_ZENABLE, FALSE );
    D3DDevice_SetVertexShader( D3DFVF_XYZRHW|D3DFVF_TEX1 );

    // Render the image
    D3DDevice_DrawVerticesUP( D3DPT_QUADLIST, 1, v, sizeof(VERTEX) );

    return TRUE; // success
}
//$END_MODIFICATIONS



/////////////////////////////////////////////////////////////////////
//
// DrawCollPoly:
//
/////////////////////////////////////////////////////////////////////

void DrawCollPoly(NEWCOLLPOLY *poly)
{
    long col, vertnum;
    REAL normDotUp;
    VEC pos[4];
    long rgb[4];

    // Get the polygon vertex positions and number of vertices
    vertnum = GetCollPolyVertices(poly, &pos[0], &pos[1], &pos[2], &pos[3]);
    Assert(vertnum >= 3 && vertnum <= 4);

    // calc rgb
    normDotUp = ONE - VecDotVec(&DownVec, PlaneNormal(&poly->Plane));
    col = (long)(127 * normDotUp);
    if (poly->Type & NON_PLANAR) {
    //  col |= col << 24;
    } else {
        col |= col << 8 | col << 16 | col << 24;
    }
//  rgb[0] = rgb[1] = rgb[2] = rgb[3] = col;

    rgb[0] = 0xff0000;
    rgb[1] = 0x00ff00;
    rgb[2] = 0x0000ff;
    rgb[3] = 0xffff00;

    // draw
    DrawNearClipPolyTEX0(pos, rgb, vertnum);
}

////////////////////////////////////////////////////////////////
//
// DrawHorizontalPoly:
//
////////////////////////////////////////////////////////////////

void DrawHorizontalPoly(VEC *pos, FACING_POLY *fpoly, long semi, float zbias)
{

    /*DRAW_3D_POLY *poly;


    poly = Get3dPoly();
    if (poly == NULL) return;

    poly->VertNum = 4;
    poly->Tpage = fpoly->Tpage;
    poly->Fog = FALSE;
    poly->SemiType = 0;

    poly->Verts[0].color = fpoly->RGB;
    poly->Verts[1].color = fpoly->RGB;
    poly->Verts[2].color = fpoly->RGB;
    poly->Verts[3].color = fpoly->RGB;

    poly->Verts[0].tu = fpoly->U;
    poly->Verts[0].tv = fpoly->V;
    poly->Verts[1].tu = fpoly->U + fpoly->Usize;
    poly->Verts[1].tv = fpoly->V;
    poly->Verts[2].tu = fpoly->U + fpoly->Usize;
    poly->Verts[2].tv = fpoly->V + fpoly->Vsize;
    poly->Verts[3].tu = fpoly->U;
    poly->Verts[3].tv = fpoly->V + fpoly->Vsize;

    poly->Pos[0].v[X] = poly->Pos[3].v[X] = pos->v[X] - fpoly->Xsize / 2;
    poly->Pos[1].v[X] = poly->Pos[2].v[X] = pos->v[X] + fpoly->Xsize / 2;

    poly->Pos[0].v[Y] = poly->Pos[1].v[Y] = pos->v[Y];

    poly->Pos[0].v[Z] = poly->Pos[1].v[Z] = pos->v[Z] + fpoly->Ysize / 2;
    poly->Pos[2].v[Z] = poly->Pos[3].v[Z] = pos->v[Z] - fpoly->Ysize / 2;
    */

    VERTEX_TEX1 *vert;
    VEC vertpos[4];

    vertpos[0].v[X] = vertpos[3].v[X] = pos->v[X] - fpoly->Xsize / 2;
    vertpos[1].v[X] = vertpos[2].v[X] = pos->v[X] + fpoly->Xsize / 2;

    vertpos[0].v[Y] = vertpos[1].v[Y] = pos->v[Y];

    vertpos[0].v[Z] = vertpos[1].v[Z] = pos->v[Z] + fpoly->Ysize / 2;
    vertpos[2].v[Z] = vertpos[3].v[Z] = pos->v[Z] - fpoly->Ysize / 2;

    vert = DrawVertsTEX1;

    RotTransPersVector(&ViewMatrixScaled, &ViewTransScaled, &vertpos[0], (REAL*)&vert[0].sx);
    RotTransPersVector(&ViewMatrixScaled, &ViewTransScaled, &vertpos[1], (REAL*)&vert[1].sx);
    RotTransPersVector(&ViewMatrixScaled, &ViewTransScaled, &vertpos[2], (REAL*)&vert[2].sx);
    RotTransPersVector(&ViewMatrixScaled, &ViewTransScaled, &vertpos[3], (REAL*)&vert[3].sx);

    vert[0].color = fpoly->RGB;
    vert[1].color = fpoly->RGB;
    vert[2].color = fpoly->RGB;
    vert[3].color = fpoly->RGB;

    vert[0].tu = fpoly->U;
    vert[0].tv = fpoly->V;
    vert[1].tu = fpoly->U + fpoly->Usize;
    vert[1].tv = fpoly->V;
    vert[2].tu = fpoly->U + fpoly->Usize;
    vert[2].tv = fpoly->V + fpoly->Vsize;
    vert[3].tu = fpoly->U;
    vert[3].tv = fpoly->V + fpoly->Vsize;

    vert[0].specular = vert[1].specular = vert[2].specular = vert[3].specular = 0;

    SET_TPAGE(fpoly->Tpage);
    DRAW_PRIM(D3DPT_TRIANGLEFAN, FVF_TEX1, vert, 4, D3DDP_DONOTUPDATEEXTENTS);
}


/////////////////////////
// draw a bounding box //
/////////////////////////

void DrawBoundingBox(float xmin, float xmax, float ymin, float ymax, float zmin, float zmax, long c0, long c1, long c2, long c3, long c4, long c5)
{
    VEC v0;
    VEC v1;
    VEC v2;
    VEC v3;
    VEC v4;
    VEC v5;
    VEC v6;
    VEC v7;
    VEC pos[4];
    long col[4];

// no textures

    SET_TPAGE(-1);

// set 8 world points

    SetVector(&v0, xmin, ymin, zmin);
    SetVector(&v1, xmax, ymin, zmin);
    SetVector(&v2, xmin, ymax, zmin);
    SetVector(&v3, xmax, ymax, zmin);
    SetVector(&v4, xmin, ymin, zmax);
    SetVector(&v5, xmax, ymin, zmax);
    SetVector(&v6, xmin, ymax, zmax);
    SetVector(&v7, xmax, ymax, zmax);

// draw xmin

    pos[0] = v4;
    pos[1] = v0;
    pos[2] = v2;
    pos[3] = v6;
    col[0] = col[1] = col[2] = col[3] = c0;
    DrawNearClipPolyTEX0(pos, col, 4);

// draw xmax

    pos[0] = v1;
    pos[1] = v5;
    pos[2] = v7;
    pos[3] = v3;
    col[0] = col[1] = col[2] = col[3] = c1;
    DrawNearClipPolyTEX0(pos, col, 4);

// draw ymin

    pos[0] = v4;
    pos[1] = v5;
    pos[2] = v1;
    pos[3] = v0;
    col[0] = col[1] = col[2] = col[3] = c2;
    DrawNearClipPolyTEX0(pos, col, 4);

// draw ymax

    pos[0] = v2;
    pos[1] = v3;
    pos[2] = v7;
    pos[3] = v6;
    col[0] = col[1] = col[2] = col[3] = c3;
    DrawNearClipPolyTEX0(pos, col, 4);

// draw zmin

    pos[0] = v0;
    pos[1] = v1;
    pos[2] = v3;
    pos[3] = v2;
    col[0] = col[1] = col[2] = col[3] = c4;
    DrawNearClipPolyTEX0(pos, col, 4);

// draw zmax

    pos[0] = v4;
    pos[1] = v5;
    pos[2] = v7;
    pos[3] = v6;
    col[0] = col[1] = col[2] = col[3] = c5;
    DrawNearClipPolyTEX0(pos, col, 4);
}

////////////////////////
// draw world normals //
////////////////////////

void DrawCubeNormals(WORLD_MODEL *m)
{
    long i;
    WORLD_VERTEX *v;
    VEC point1, point2;

// loop thru polys

    v = m->VertPtr;
    for (i = 0 ; i < m->VertNum + m->PolyNum ; i++, v++)
    {
        point1.v[X] = v->x;
        point1.v[Y] = v->y;
        point1.v[Z] = v->z;

        point2.v[X] = point1.v[X] + (v->nx * 64);
        point2.v[Y] = point1.v[Y] + (v->ny * 64);
        point2.v[Z] = point1.v[Z] + (v->nz * 64);

        DrawLine(&point1, &point2, 0xffffff, 0);
    }
}

//////////////////
// draw 3d line //
//////////////////

void DrawLine(VEC *v0, VEC *v1, long col0, long col1)
{
    float mul;
    long lmul;
    VEC delta;
    MODEL_RGB *rgb0, *rgb1;

// transform to camera space

    RotTransVector(&ViewMatrixScaled, &ViewTransScaled, v0, (VEC*)&DrawVertsTEX1[0].sx);
    RotTransVector(&ViewMatrixScaled, &ViewTransScaled, v1, (VEC*)&DrawVertsTEX1[1].sx);

// clip verts if < 1

    if (DrawVertsTEX1[0].sz < 1)
    {
        if (DrawVertsTEX1[1].sz < 1)
            return;

        SubVector((VEC*)&DrawVertsTEX1[1].sx, (VEC*)&DrawVertsTEX1[0].sx, &delta);
        mul = (1 - DrawVertsTEX1[0].sz) / (DrawVertsTEX1[1].sz - DrawVertsTEX1[0].sz);
        DrawVertsTEX1[0].sx += delta.v[X] * mul;
        DrawVertsTEX1[0].sy += delta.v[Y] * mul;
        DrawVertsTEX1[0].sz += delta.v[Z] * mul;

        FTOL(mul * 256, lmul);
        rgb0 = (MODEL_RGB*)&col0;
        rgb1 = (MODEL_RGB*)&col1;
        rgb0->r = (unsigned char)((rgb0->r * (256 - lmul) + rgb1->r * lmul) >> 8);
        rgb0->g = (unsigned char)((rgb0->g * (256 - lmul) + rgb1->g * lmul) >> 8);
        rgb0->b = (unsigned char)((rgb0->b * (256 - lmul) + rgb1->b * lmul) >> 8);
    }

    else if (DrawVertsTEX1[1].sz < 1)
    {
        SubVector((VEC*)&DrawVertsTEX1[0].sx, (VEC*)&DrawVertsTEX1[1].sx, &delta);
        mul = (1 - DrawVertsTEX1[1].sz) / (DrawVertsTEX1[0].sz - DrawVertsTEX1[1].sz);
        DrawVertsTEX1[1].sx += delta.v[X] * mul;
        DrawVertsTEX1[1].sy += delta.v[Y] * mul;
        DrawVertsTEX1[1].sz += delta.v[Z] * mul;

        FTOL(mul * 256, lmul);
        rgb0 = (MODEL_RGB*)&col1;
        rgb1 = (MODEL_RGB*)&col0;
        rgb0->r = (unsigned char)((rgb0->r * (256 - lmul) + rgb1->r * lmul) >> 8);
        rgb0->g = (unsigned char)((rgb0->g * (256 - lmul) + rgb1->g * lmul) >> 8);
        rgb0->b = (unsigned char)((rgb0->b * (256 - lmul) + rgb1->b * lmul) >> 8);
    }

// perspectify

    DrawVertsTEX1[0].sx = DrawVertsTEX1[0].sx / DrawVertsTEX1[0].sz + RenderSettings.GeomCentreX;
    DrawVertsTEX1[0].sy = DrawVertsTEX1[0].sy / DrawVertsTEX1[0].sz + RenderSettings.GeomCentreY;
    DrawVertsTEX1[0].rhw = 1 / DrawVertsTEX1[0].sz;
    DrawVertsTEX1[0].sz = GET_ZBUFFER(DrawVertsTEX1[0].sz);

    DrawVertsTEX1[1].sx = DrawVertsTEX1[1].sx / DrawVertsTEX1[1].sz + RenderSettings.GeomCentreX;
    DrawVertsTEX1[1].sy = DrawVertsTEX1[1].sy / DrawVertsTEX1[1].sz + RenderSettings.GeomCentreY;
    DrawVertsTEX1[1].rhw = 1 / DrawVertsTEX1[1].sz;
    DrawVertsTEX1[1].sz = GET_ZBUFFER(DrawVertsTEX1[1].sz);

// draw

    DrawVertsTEX1[0].color = col0;
    DrawVertsTEX1[1].color = col1;

    DrawVertsTEX1[0].tu = 220.0f / 256.0f;
    DrawVertsTEX1[0].tv = 156.0f / 256.0f;
    DrawVertsTEX1[1].tu = 228.0f / 256.0f;
    DrawVertsTEX1[1].tv = 156.5f / 256.0f;

    SET_TPAGE(TPAGE_FX1);
    DRAW_PRIM(D3DPT_LINELIST, FVF_TEX1, DrawVertsTEX1, 2, D3DDP_DONOTUPDATEEXTENTS);
}

///////////////////////
// clip and draw tri //
///////////////////////

void DrawTriClip(VERTEX_TEX1 *v0, VERTEX_TEX1 *v1, VERTEX_TEX1 *v2)
{

// setup misc

    ClipVertNum = 3;
    ClipVertFree = 3;

    ClipVertList[0][0] = 0;
    ClipVertList[0][1] = 1;
    ClipVertList[0][2] = 2;

    ClipVert[0] = *v0;
    ClipVert[1] = *v1;
    ClipVert[2] = *v2;

// clip

    DrawFanClip();
}

////////////////////////
// clip and draw quad //
////////////////////////

void DrawQuadClip(VERTEX_TEX1 *v0, VERTEX_TEX1 *v1, VERTEX_TEX1 *v2, VERTEX_TEX1 *v3)
{

// setup misc

    ClipVertNum = 4;
    ClipVertFree = 4;

    ClipVertList[0][0] = 0;
    ClipVertList[0][1] = 1;
    ClipVertList[0][2] = 2;
    ClipVertList[0][3] = 3;

    ClipVert[0] = *v0;
    ClipVert[1] = *v1;
    ClipVert[2] = *v2;
    ClipVert[3] = *v3;

// clip

    DrawFanClip();
}

//////////////////////////////////
// clip and draw a triangle fan //
//////////////////////////////////

void DrawFanClip(void)
{
    short i, newcount;
    VERTEX_TEX1 *vert0, *vert1;

// top

    for (i = newcount = 0 ; i < ClipVertNum ; i++)
    {
        vert0 = ClipVert + ClipVertList[0][i];
        vert1 = ClipVert + ClipVertList[0][(i + 1) % ClipVertNum];

        if (vert0->sy >= ScreenTopClip)
        {
            ClipVertList[1][newcount++] = ClipVertList[0][i];

            if (vert1->sy < ScreenTopClip)
            {
                ClipLineTEX1(vert0, vert1, (vert0->sy - ScreenTopClip) / (vert0->sy - vert1->sy), ClipVert + ClipVertFree);
                ClipVertList[1][newcount++] = ClipVertFree++;
            }
        }
        else
        {
            if (vert1->sy >= ScreenTopClip)
            {
                ClipLineTEX1(vert1, vert0, (vert1->sy - ScreenTopClip) / (vert1->sy - vert0->sy), ClipVert + ClipVertFree);
                ClipVertList[1][newcount++] = ClipVertFree++;
            }
        }
    }

    ClipVertNum = newcount;

// bottom

    for (i = newcount = 0 ; i < ClipVertNum ; i++)
    {
        vert0 = ClipVert + ClipVertList[1][i];
        vert1 = ClipVert + ClipVertList[1][(i + 1) % ClipVertNum];

        if (vert0->sy <= ScreenBottomClip)
        {
            ClipVertList[0][newcount++] = ClipVertList[1][i];

            if (vert1->sy > ScreenBottomClip)
            {
                ClipLineTEX1(vert0, vert1, (ScreenBottomClip - vert0->sy) / (vert1->sy - vert0->sy), ClipVert + ClipVertFree);
                ClipVertList[0][newcount++] = ClipVertFree++;
            }
        }
        else
        {
            if (vert1->sy <= ScreenBottomClip)
            {
                ClipLineTEX1(vert1, vert0, (ScreenBottomClip - vert1->sy) / (vert0->sy - vert1->sy), ClipVert + ClipVertFree);
                ClipVertList[0][newcount++] = ClipVertFree++;
            }
        }
    }

    ClipVertNum = newcount;

// left

    for (i = newcount = 0 ; i < ClipVertNum ; i++)
    {
        vert0 = ClipVert + ClipVertList[0][i];
        vert1 = ClipVert + ClipVertList[0][(i + 1) % ClipVertNum];

        if (vert0->sx >= ScreenLeftClip)
        {
            ClipVertList[1][newcount++] = ClipVertList[0][i];

            if (vert1->sx < ScreenLeftClip)
            {
                ClipLineTEX1(vert0, vert1, (vert0->sx - ScreenLeftClip) / (vert0->sx - vert1->sx), ClipVert + ClipVertFree);
                ClipVertList[1][newcount++] = ClipVertFree++;
            }
        }
        else
        {
            if (vert1->sx >= ScreenLeftClip)
            {
                ClipLineTEX1(vert1, vert0, (vert1->sx - ScreenLeftClip) / (vert1->sx - vert0->sx), ClipVert + ClipVertFree);
                ClipVertList[1][newcount++] = ClipVertFree++;
            }
        }
    }

    ClipVertNum = newcount;

// right

    for (i = newcount = 0 ; i < ClipVertNum ; i++)
    {
        vert0 = ClipVert + ClipVertList[1][i];
        vert1 = ClipVert + ClipVertList[1][(i + 1) % ClipVertNum];

        if (vert0->sx <= ScreenRightClip)
        {
            ClipVertList[0][newcount++] = ClipVertList[1][i];

            if (vert1->sx > ScreenRightClip)
            {
                ClipLineTEX1(vert0, vert1, (ScreenRightClip - vert0->sx) / (vert1->sx - vert0->sx), ClipVert + ClipVertFree);
                ClipVertList[0][newcount++] = ClipVertFree++;
            }
        }
        else
        {
            if (vert1->sx <= ScreenRightClip)
            {
                ClipLineTEX1(vert1, vert0, (ScreenRightClip - vert1->sx) / (vert0->sx - vert1->sx), ClipVert + ClipVertFree);
                ClipVertList[0][newcount++] = ClipVertFree++;
            }
        }
    }

    ClipVertNum = newcount;

// draw

    DRAW_PRIM_INDEX(D3DPT_TRIANGLEFAN, FVF_TEX1, ClipVert, 32, ClipVertList[0], ClipVertNum, D3DDP_DONOTUPDATEEXTENTS | D3DDP_DONOTCLIP);
}

/////////////////
// clip a line //
/////////////////

void ClipLineTEX0(VERTEX_TEX0 *v0, VERTEX_TEX0 *v1, float mul, VERTEX_TEX0 *out)
{
    long lmul;
    float zout;
    MODEL_RGB *rgb0, *rgb1, *rgbout;

// clip xyz rhw

    out->sx = v0->sx + (v1->sx - v0->sx) * mul;
    out->sy = v0->sy + (v1->sy - v0->sy) * mul;
    out->rhw = v0->rhw + (v1->rhw - v0->rhw) * mul;
    zout = 1.0f / out->rhw;
    out->sz = GET_ZBUFFER(zout);

// clip fog + rgb

    FTOL(mul * 256, lmul);

    out->specular = v0->specular + ((((v1->specular >> 24) - (v0->specular >> 24)) * lmul) << 16);

    rgb0 = (MODEL_RGB*)&v0->color;
    rgb1 = (MODEL_RGB*)&v1->color;
    rgbout = (MODEL_RGB*)&out->color;

    rgbout->r = rgb0->r + (unsigned char)(((rgb1->r - rgb0->r) * lmul) >> 8);
    rgbout->g = rgb0->g + (unsigned char)(((rgb1->g - rgb0->g) * lmul) >> 8);
    rgbout->b = rgb0->b + (unsigned char)(((rgb1->b - rgb0->b) * lmul) >> 8);
}

/////////////////
// clip a line //
/////////////////

void ClipLineTEX1(VERTEX_TEX1 *v0, VERTEX_TEX1 *v1, float mul, VERTEX_TEX1 *out)
{
    long lmul;
    float z0, z1, zout, zmul;
    MODEL_RGB *rgb0, *rgb1, *rgbout;

// clip xyz rhw

    out->sx = v0->sx + (v1->sx - v0->sx) * mul;
    out->sy = v0->sy + (v1->sy - v0->sy) * mul;
    out->rhw = v0->rhw + (v1->rhw - v0->rhw) * mul;
    zout = 1.0f / out->rhw;
    out->sz = GET_ZBUFFER(zout);

// clip uv

    z0 = 1.0f / v0->rhw;
    z1 = 1.0f / v1->rhw;
    zmul = (zout - z0) / (z1 - z0);

    out->tu = v0->tu + (v1->tu - v0->tu) * zmul;
    out->tv = v0->tv + (v1->tv - v0->tv) * zmul;

// clip fog + rgb

    FTOL(mul * 256, lmul);

    out->specular = v0->specular + ((((v1->specular >> 24) - (v0->specular >> 24)) * lmul) << 16);

    rgb0 = (MODEL_RGB*)&v0->color;
    rgb1 = (MODEL_RGB*)&v1->color;
    rgbout = (MODEL_RGB*)&out->color;

    rgbout->r = rgb0->r + (unsigned char)(((rgb1->r - rgb0->r) * lmul) >> 8);
    rgbout->g = rgb0->g + (unsigned char)(((rgb1->g - rgb0->g) * lmul) >> 8);
    rgbout->b = rgb0->b + (unsigned char)(((rgb1->b - rgb0->b) * lmul) >> 8);
}

///////////////////////////
// save out front buffer //
///////////////////////////

void SaveFrontBuffer(char *file)
{
#ifndef XBOX_NOT_YET_IMPLEMENTED
    DDSURFACEDESC2 ddsd;
    BITMAPFILEHEADER bf;
    BITMAPINFOHEADER bi;
    FILE *fp;
    DWORD y, x;
    short *p, r, g, b;
    short outbuf[1600];

// open file

    fp = fopen(file, "wb");
    if (!fp) return;

// lock front buffer

    ddsd.dwSize = sizeof(ddsd);
    while (FrontBuffer->Lock(NULL, &ddsd, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL) != DD_OK);

// write header

    bf.bfType = MAKEWORD((BYTE)'B',(BYTE)'M');
    bf.bfSize = sizeof(bf) + sizeof(bi) + ScreenXsize * ScreenYsize * sizeof(short);
    bf.bfReserved1 = 0;
    bf.bfReserved2 = 0;
    bf.bfOffBits = sizeof(bf) + sizeof(bi);

    fwrite(&bf, sizeof(bf), 1, fp);

    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = ScreenXsize;
    bi.biHeight = ScreenYsize;
    bi.biPlanes = 1;
    bi.biBitCount = 16;
    bi.biCompression = BI_RGB;
    bi.biSizeImage = 0;
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrUsed = 0;
    bi.biClrImportant = 0;

    fwrite(&bi, sizeof(bi), 1, fp);

// write buffer

    p = (short*)ddsd.lpSurface;
    p += ddsd.lPitch * (ScreenYsize - 1) / 2;

    if (ddsd.ddpfPixelFormat.dwGBitMask & 1024)
    {
        for (y = 0 ; y < ScreenYsize ; y++)
        {
            for (x = 0 ; x < ScreenXsize ; x++)
            {
                b = p[x] & 31;
                g = (p[x] >> 6) & 31;
                r = (p[x] >> 11) & 31;
                outbuf[x] = b | (g << 5) | (r << 10);
            }
            fwrite(outbuf, sizeof(short), ScreenXsize, fp);
            p -= ddsd.lPitch / 2;
        }
    }
    else
    {
        for (y = 0 ; y < ScreenYsize ; y++)
        {
            fwrite(p, sizeof(short), ScreenXsize, fp);
            p -= ddsd.lPitch / 2;
        }
    }

// unlock front buffer

    FrontBuffer->Unlock(NULL);

// close file

    fclose(fp);
#endif // ! XBOX_NOT_YET_IMPLEMENTED
}

////////////////////////
// reset mesh fx list //
////////////////////////

void ResetMeshFxList(void)
{
    WorldMeshFxCount = 0;
    ModelMeshFxCount = 0;
}

/////////////////////////
// add mesh special fx //
/////////////////////////
 
void AddWorldMeshFx(void (*checker)(void *data), void *data)
{

// quit if full

    if (WorldMeshFxCount >= MAX_WORLD_MESH_FX)
        return;

// add

    WorldMeshFx[WorldMeshFxCount].Checker = checker;
    WorldMeshFx[WorldMeshFxCount].Data = data;
    WorldMeshFxCount++;
}

/////////////////////////
// add mesh special fx //
/////////////////////////
 
void AddModelMeshFx(void (*checker)(void *data), void *data)
{

// quit if full

    if (ModelMeshFxCount >= MAX_MODEL_MESH_FX)
        return;

// add

    ModelMeshFx[ModelMeshFxCount].Checker = checker;
    ModelMeshFx[ModelMeshFxCount].Data = data;
    ModelMeshFxCount++;
}

/////////////////////////////
// init jump spark offsets //
/////////////////////////////

void InitJumpSparkOffsets(void)
{
    long i;

    for (i = 0 ; i < JUMPSPARK_OFFSET_NUM ; i++)
    {
        SetVector(&JumpSparkOffset[i], frand(4.0f) - 2.0f, frand(4.0f) - 2.0f, frand(4.0f) - 2.0f);
    }
}

/////////////////////
// draw jump spark //
/////////////////////
void DrawJumpSpark(VEC *v1, VEC *v2, VEC *dir1, VEC *dir2, long tex)
{
    long i, steps, offset;
    VEC delta, pos, vec, svec;
    REAL len, fsteps, mul, dx1, dy1, dx2, dy2, s, sadd, smul, tu, tv;
    REAL dx[JUMPSPARK_STEP_MAX], dy[JUMPSPARK_STEP_MAX];
    VERTEX_TEX0 points[JUMPSPARK_STEP_MAX + 1];
    VERTEX_TEX1 *verts;
    long    rgb, sparkSize;
    REAL    scale;
    VEC     bezier[4];
    REAL    t, Dt;
    REAL    t0,t1,t2,t3, a,b;

// get tu / tv

    if (!tex)
    {
        tu = 216.0f;
        tv = 33.0f;
    }
    else
    {
        tu = 209.0f;
        tv = 33.0f;
    }

// calc steps

    SubVector(v2, v1, &delta);
    len = Length(&delta);
    fsteps = len / JUMPSPARK_STEP_LEN;
    if (fsteps > JUMPSPARK_STEP_MAX)
        fsteps = JUMPSPARK_STEP_MAX;

    FTOL(fsteps, steps);
    fsteps = (float)steps;

// build screen xyz's

    CopyVec(v1, &pos);
    mul = 1.0f / fsteps;
    VecMulScalar(&delta, mul);

    i = TIME2MS(TimerCurrent) / 20;
    if (i != JumpSparkTime)
    {
        JumpSparkTime = i;
        JumpSparkOff = rand() % JUMPSPARK_OFFSET_NUM;
    }

    offset = JumpSparkOff;

    CrossProduct(&delta, &ViewCameraMatrix.mv[U], &vec);
    CrossProduct(&vec, &delta, &svec);
    NormalizeVector(&svec);

    s = (float)TIME2MS(TimerCurrent) / 100.0f;

    i = TIME2MS(TimerCurrent) / 500;
    if (i != JumpSparkSinTime)
    {   
        JumpSparkSinTime = i;
        JumpSparkSinDiv = frand(6.0f) + 9.0f;
    }
    sadd = len / fsteps / JumpSparkSinDiv;

    smul = (float)sin((float)TIME2MS(TimerCurrent) / 300.0f) + 3.0f;


// Create Bezier Curve
    CopyVec(v1, &bezier[0]);
    CopyVec(v2, &bezier[3]);
    CopyVec(v1, &bezier[1]);
    CopyVec(v2, &bezier[2]);
    VecPlusEqVec(&bezier[1], dir1);
    VecPlusEqVec(&bezier[2], dir2);

    offset = JumpSparkOff;

    sparkSize = rand() & 7;
    if (!sparkSize)     scale = frand(2.0f) + 3.0f;
    else                scale = frand(4.0f) + 6.0f;

    Dt = Real(1.0) / fsteps;
    for (i = 0, t = 0; i < steps + 1; i++, t += Dt)
    {
        BEZIER_SETUP(t, t0,t1,t2,t3, a,b);

        BEZIER_INTERP(&vec, &bezier[0],
                            &bezier[1],
                            &bezier[2],
                            &bezier[3], t0,t1,t2,t3);

        vec.v[X] += JumpSparkOffset[offset].v[X] * scale;
        vec.v[Y] += JumpSparkOffset[offset].v[Y] * scale;
        vec.v[Z] += JumpSparkOffset[offset].v[Z] * scale;
        mul = (float)sin(s) * smul;
        VecPlusEqScalarVec(&vec, mul, &svec);

        offset++;
        offset %= JUMPSPARK_OFFSET_NUM;

        RotTransPersVector(&ViewMatrixScaled, &ViewTransScaled, &vec, (float*)&points[i]);
    }


// calc delta's for each step

    for (i = 0 ; i < steps ; i++)
    {
        dx1 = points[i + 1].sx - points[i].sx;
        dy1 = points[i + 1].sy - points[i].sy;
        mul = 4.0f / (float)sqrt(dx1 * dx1 + dy1 * dy1) * points[i].rhw * RenderSettings.GeomPers;
        dy[i] = dx1 * mul * RenderSettings.GeomScaleX;
        dx[i] = -dy1 * mul * RenderSettings.GeomScaleY;
    }

// create each poly

    i = (rand() & 255) | 31;
    rgb = i | (i << 8) | (i << 16);

    if (!sparkSize)     scale = frand(5.0f) + 10.0f;
    else                scale = frand(4.0f) + 2.0f;

    for (i = 0 ; i < steps ; i++)
    {

// get semi slot

        if (!SEMI_POLY_FREE()) return;
        SEMI_POLY_SETUP_ZBIAS(verts, FALSE, 4, TPAGE_FX1, TRUE, 1, -128.0f);

// calc dx1, dy1

        if (!i)
        {
            dx1 = dx[i] * 0.75f;
            dy1 = dy[i] * 0.75f;
        }
        else
        {
            dx1 = dx2;
            dy1 = dy2;
        }

// calc dx2, dy2

        if (i == steps - 1)
        {
            dx2 = dx[i] * 0.75f;
            dy2 = dy[i] * 0.75f;
        }
        else
        {
            dx2 = dx[i] + dx[i + 1];
            dy2 = dy[i] + dy[i + 1];
//          mul = 4.0f / (float)sqrt(dx2 * dx2 + dy2 * dy2) * points[i + 1].rhw * RenderSettings.GeomPers;
            mul = scale / (float)sqrt(dx2 * dx2 + dy2 * dy2) * points[i + 1].rhw * RenderSettings.GeomPers;
            dx2 *= mul * RenderSettings.GeomScaleX;
            dy2 *= mul * RenderSettings.GeomScaleY;
        }

// build poly

        verts[0].sx = points[i].sx - dx1;
        verts[0].sy = points[i].sy - dy1;

        verts[1].sx = points[i + 1].sx - dx2;
        verts[1].sy = points[i + 1].sy - dy2;

        verts[2].sx = points[i + 1].sx + dx2;
        verts[2].sy = points[i + 1].sy + dy2;

        verts[3].sx = points[i].sx + dx1;
        verts[3].sy = points[i].sy + dy1;

        verts[0].sz = verts[3].sz = points[i].sz;
        verts[1].sz = verts[2].sz = points[i + 1].sz;
        verts[0].rhw = verts[3].rhw = points[i].rhw;
        verts[1].rhw = verts[2].rhw = points[i + 1].rhw;

        verts[0].tu = verts[3].tu = tu / 256.0f;
        verts[1].tu = verts[2].tu = (tu + 6.0f) / 256.0f;
        verts[0].tv = verts[1].tv = tv / 256.0f;
        verts[2].tv = verts[3].tv = (tv + 14.0f) / 256.0f;

        if (i == 0)
        {
            verts[0].color = verts[3].color = 0x000000;
            verts[1].color = verts[2].color = rgb;
        }
        else if (i == steps-1)
        {
            verts[1].color = verts[2].color = 0x000000;
            verts[0].color = verts[3].color = rgb;
        }
        else
            verts[0].color = verts[1].color = verts[2].color = verts[3].color = rgb;
    }

// draw 'end' flares
#if 0   // start & end aren't setup !!!
    poly.Xsize = poly.Ysize = (float)(TIME2MS(TimerCurrent) % 100) / 25.0f + 4.0f;
    poly.U = 192.0f / 256.0f;
    poly.V = 64.0f / 256.0f;
    poly.Usize = poly.Vsize = 64.0f / 256.0f;
    poly.Tpage = TPAGE_FX1;
    poly.RGB = 0x8080ff;

    ang = (float)TIME2MS(TimerCurrent) / 10000.0f;

    RotMatrixZ(&mat, ang);
    DrawFacingPolyRot(&start, &mat, &poly, 1, -16.0f);
    DrawFacingPolyRot(&end, &mat, &poly, 1, -16.0f);

    RotMatrixZ(&mat, ang * 2.0f);
    DrawFacingPolyRot(&start, &mat, &poly, 1, -16.0f);
    DrawFacingPolyRot(&end, &mat, &poly, 1, -16.0f);
#endif
}

void DrawJumpSpark2(VEC *v1, VEC *v2)
{
    long i, steps, offset;
    VEC delta, pos, vec, start, end, svec;
    REAL len, fsteps, mul, dx1, dy1, dx2, dy2, s, sadd, smul;
    REAL dx[JUMPSPARK_STEP_MAX], dy[JUMPSPARK_STEP_MAX];
    VERTEX_TEX0 points[JUMPSPARK_STEP_MAX + 1];
    VERTEX_TEX1 *verts;

// calc steps

    SubVector(v2, v1, &delta);
    len = Length(&delta);
    fsteps = len / JUMPSPARK_STEP_LEN;
    if (fsteps > JUMPSPARK_STEP_MAX)
        fsteps = JUMPSPARK_STEP_MAX;

    FTOL(fsteps, steps);
    fsteps = (float)steps;

// build screen xyz's

    CopyVec(v1, &pos);
    mul = 1.0f / fsteps;
    VecMulScalar(&delta, mul);

    i = TIME2MS(TimerCurrent) / 20;
    if (i != JumpSparkTime)
    {
        JumpSparkTime = i;
        JumpSparkOff = rand() % JUMPSPARK_OFFSET_NUM;
    }

    offset = JumpSparkOff;

    CrossProduct(&delta, &ViewCameraMatrix.mv[U], &vec);
    CrossProduct(&vec, &delta, &svec);
    NormalizeVector(&svec);

    s = (float)TIME2MS(TimerCurrent) / 100.0f;

    i = TIME2MS(TimerCurrent) / 500;
    if (i != JumpSparkSinTime)
    {   
        JumpSparkSinTime = i;
        JumpSparkSinDiv = frand(6.0f) + 9.0f;
    }
    sadd = len / fsteps / JumpSparkSinDiv;

    smul = (float)sin((float)TIME2MS(TimerCurrent) / 300.0f) + 3.0f;

    for (i = 0 ; i < steps + 1; i++)
    {
        CopyVec(&pos, &vec);

//      if (i && i < steps)
        {
            vec.v[X] += JumpSparkOffset[offset].v[X];
            vec.v[Y] += JumpSparkOffset[offset].v[Y];
            vec.v[Z] += JumpSparkOffset[offset].v[Z];

            mul = (float)sin(s) * smul;
            VecPlusEqScalarVec(&vec, mul, &svec);
        }

        if (!i)
            CopyVec(&vec, &start)

        if (i == steps)
            CopyVec(&vec, &end)

        RotTransPersVector(&ViewMatrixScaled, &ViewTransScaled, &vec, (float*)&points[i]);
        AddVector(&pos, &delta, &pos);

        offset++;
        offset %= JUMPSPARK_OFFSET_NUM;

        s += sadd;
    }

// calc delta's for each step

    for (i = 0 ; i < steps ; i++)
    {
        dx1 = points[i + 1].sx - points[i].sx;
        dy1 = points[i + 1].sy - points[i].sy;
        mul = 4.0f / (float)sqrt(dx1 * dx1 + dy1 * dy1) * points[i].rhw * RenderSettings.GeomPers;
        dy[i] = dx1 * mul  * RenderSettings.GeomScaleX;
        dx[i] = -dy1 * mul  * RenderSettings.GeomScaleY;
    }

// create each poly

    for (i = 0 ; i < steps ; i++)
    {

// get semi slot

        if (!SEMI_POLY_FREE()) return;
        SEMI_POLY_SETUP_ZBIAS(verts, FALSE, 4, TPAGE_FX1, TRUE, 1, -128.0f);

// calc dx1, dy1

        if (!i)
        {
            dx1 = 0;
            dy1 = 0;
        }
        else
        {
            dx1 = dx2;
            dy1 = dy2;
        }

// calc dx2, dy2

        if (i == steps - 1)
        {
            dx2 = 0;
            dy2 = 0;
        }
        else
        {
            dx2 = dx[i] + dx[i + 1];
            dy2 = dy[i] + dy[i + 1];
            mul = 4.0f / (float)sqrt(dx2 * dx2 + dy2 * dy2) * points[i + 1].rhw * RenderSettings.GeomPers;
            dx2 *= mul  * RenderSettings.GeomScaleX;
            dy2 *= mul  * RenderSettings.GeomScaleY;
        }

// build poly

        verts[0].sx = points[i].sx - dx1;
        verts[0].sy = points[i].sy - dy1;

        verts[1].sx = points[i + 1].sx - dx2;
        verts[1].sy = points[i + 1].sy - dy2;

        verts[2].sx = points[i + 1].sx + dx2;
        verts[2].sy = points[i + 1].sy + dy2;

        verts[3].sx = points[i].sx + dx1;
        verts[3].sy = points[i].sy + dy1;

        verts[0].sz = verts[3].sz = points[i].sz;
        verts[1].sz = verts[2].sz = points[i + 1].sz;
        verts[0].rhw = verts[3].rhw = points[i].rhw;
        verts[1].rhw = verts[2].rhw = points[i + 1].rhw;

        verts[0].tu = verts[3].tu = 216.0f / 256.0f;
        verts[1].tu = verts[2].tu = 223.0f / 256.0f;
        verts[0].tv = verts[1].tv = 33.0f / 256.0f;
        verts[2].tv = verts[3].tv = 47.0f / 256.0f;

        verts[0].color = verts[1].color = verts[2].color = verts[3].color = 0xffffff;
    }
}

//////////////////////////////////////
// near clip a bucket textured poly //
//////////////////////////////////////

void NearClipBucketPoly(BUCKET_TEX1 *bucket, long vertnum)
{
    long i, newcount;
    unsigned short baseidx, *destidx;
    VERTEX_TEX1 *destvert, *v0, *v1;

// copy verts

    destvert = bucket->CurrentVerts - vertnum;
    for (i = 0 ; i < vertnum ; i++)
    {
        *(MEM32*)&DrawVertsTEX1[i] = *(MEM32*)&destvert[i];
    }

// clip each line - sutherland / bodgeman!

    newcount = 0;

    for (i = 0 ; i < vertnum ; i++)
    {
        v0 = &DrawVertsTEX1[i];
        v1 = &DrawVertsTEX1[(i + 1) % vertnum];

// 1st vert good?

        if ((1.0f / v0->rhw) >= RenderSettings.NearClip)
        {

// yep, add to new list

            *(MEM32*)&destvert[newcount++] = *(MEM32*)v0;

// 2nd vert neg?

            if ((1.0f / v1->rhw) < RenderSettings.NearClip)
            {

// yep, clip from 1st to 2nd

                NearClipLineTEX1(v0, v1, &destvert[newcount++]);
            }
        }

// 1st was neg, 2nd pos?

        else if ((1.0f / v1->rhw) >= RenderSettings.NearClip)
        {

// yep, clip from 2nd to 1st

            NearClipLineTEX1(v1, v0, &destvert[newcount++]);
        }
    }

// create indices

    baseidx = (unsigned short)(destvert - bucket->Verts);
    destidx = bucket->CurrentIndex - ((vertnum - 2) * 3);

    for (i = 0 ; i < newcount - 2 ; i++)
    {
        *destidx++ = baseidx;
        *destidx++ = baseidx + i + 1;
        *destidx++ = baseidx + i + 2;
    }

// update 'current' ptrs

    bucket->CurrentIndex = destidx;
    bucket->CurrentVerts = destvert + newcount;
}

/////////////////////////////////
// near clip a bucket rgb poly //
/////////////////////////////////

void NearClipBucketPolyRGB(BUCKET_TEX0 *bucket, long vertnum)
{
}

////////////////////////////
// near clip a TEX 1 line //
////////////////////////////

void NearClipLineTEX1(VERTEX_TEX1 *v0, VERTEX_TEX1 *v1, VERTEX_TEX1 *vout)
{
    float mul, x0, y0, z0, x1, y1, z1, xi, yi;
    long lmul;
    MODEL_RGB *rgb0, *rgb1, *rgbout;

// get eyespace 3D coordinates of each point

    z0 = 1.0f / v0->rhw;
    x0 = v0->sx * z0 / RenderSettings.MatScaleX;
    y0 = v0->sy * z0 / RenderSettings.MatScaleY;

    z1 = 1.0f / v1->rhw;
    x1 = v1->sx * z1 / RenderSettings.MatScaleX;
    y1 = v1->sy * z1 / RenderSettings.MatScaleY;

// get xyz intersection point

    mul = (z0 - RenderSettings.NearClip) / (z0 - z1);

    xi = x0 + (x1 - x0) * mul;
    yi = y0 + (y1 - y0) * mul;

// set screen xyzrhw

    vout->sx = xi * RenderSettings.MatScaleX / RenderSettings.NearClip;
    vout->sy = yi * RenderSettings.MatScaleY / RenderSettings.NearClip;
    vout->sz = 0.0f;
    vout->rhw = 1.0f / RenderSettings.NearClip;

// set tex uv

    vout->tu = v0->tu;
    vout->tv = v0->tv;

// set fog / diffuse

    FTOL(mul * 256, lmul);

    vout->specular = v0->specular + ((((v1->specular >> 24) - (v0->specular >> 24)) * lmul) << 16);

    rgb0 = (MODEL_RGB*)&v0->color;
    rgb1 = (MODEL_RGB*)&v1->color;
    rgbout = (MODEL_RGB*)&vout->color;

    rgbout->a = rgb0->a + (unsigned char)(((rgb1->a - rgb0->a) * lmul) >> 8);
    rgbout->r = rgb0->r + (unsigned char)(((rgb1->r - rgb0->r) * lmul) >> 8);
    rgbout->g = rgb0->g + (unsigned char)(((rgb1->g - rgb0->g) * lmul) >> 8);
    rgbout->b = rgb0->b + (unsigned char)(((rgb1->b - rgb0->b) * lmul) >> 8);

*(long*)rgbout = 0xff0000;
}

////////////////////////////
// near clip a TEX 0 line //
////////////////////////////

void NearClipLineTEX0(VERTEX_TEX0 *v0, VERTEX_TEX0 *v1, VERTEX_TEX0 *vout)
{
}

////////////////////
// init fade shit //
////////////////////

void InitFadeShit(void)
{

    FadeVert[0].sx = 0.0f;
    FadeVert[0].sy = 0.0f;
    FadeVert[0].rhw = 1.0f;

    FadeVert[1].sx = (float)ScreenXsize;
    FadeVert[1].sy = 0.0f;
    FadeVert[1].rhw = 1.0f;

    FadeVert[2].sx = (float)ScreenXsize;
    FadeVert[2].sy = (float)ScreenYsize;
    FadeVert[2].rhw = 1.0f;

    FadeVert[3].sx = 0.0f;
    FadeVert[3].sy = (float)ScreenYsize;
    FadeVert[3].rhw = 1.0f;
}

/////////////////////
// set fade effect //
/////////////////////

void SetFadeEffect(long effect)
{
    FadeEffect = effect;

    switch (effect)
    {
        case FADE_UP:
            FadeTime = 0.0f;
            break;

        case FADE_DOWN:
            FadeTime = 1.0f;
            break;
    }
}

/////////////////////
// get fade effect //
/////////////////////

long GetFadeEffect(void)
{
    return FadeEffect;
}

////////////////////
// draw fade shit //
////////////////////

void DrawFadeShit(void)
{
    long i, alpha;

// effect

    switch (FadeEffect)
    {

// quit

        case FADE_NONE:
        case FADE_UP_DONE:
            return;
        case FADE_DOWN_DONE:
            alpha = 0;

            for (i = 0 ; i < 4 ; i++)
            {
                FadeVert[i].color = alpha << 24;
            }

            break;

// fade up / down

        case FADE_UP:
        case FADE_UP_STAY:
        case FADE_DOWN:
        case FADE_DOWN_STAY:

            if (FadeEffect == FADE_UP_STAY)
            {
                FadeTime += TimeStep * 4.0f;
            }
            else if (FadeEffect == FADE_UP)
            {
                FadeTime += TimeStep * 4.0f;
                if (FadeTime > 1.2f)
                {
                    FadeEffect = FADE_UP_DONE;
                    //return;
                }
            }
            else if (FadeEffect == FADE_DOWN_STAY)
            {
                FadeTime -= TimeStep * 4.0f;
            }
            else
            {
                FadeTime -= TimeStep * 4.0f;
                if (FadeTime < -0.2f)
                {
                    FadeEffect = FADE_DOWN_DONE;
                    //return;
                }
            }

            alpha = 255 - (long)(FadeTime * 255.0f);
            if (alpha < 0) alpha = 0;
            if (alpha > 255) alpha = 255;

            for (i = 0 ; i < 4 ; i++)
            {
                FadeVert[i].color = alpha << 24;
            }

            break;
    }

// set viewport

    SetViewport(0.0f, 0.0f, (float)ScreenXsize, (float)ScreenYsize, RenderSettings.GeomPers);

// draw

    FOG_OFF();
    ZBUFFER_OFF();
    SET_TPAGE(-1);
    BLEND_ALPHA();
    BLEND_SRC(D3DBLEND_SRCALPHA);
    BLEND_DEST(D3DBLEND_INVSRCALPHA);

    SET_RENDER_STATE(D3DRENDERSTATE_ALPHATESTENABLE, FALSE);
    DRAW_PRIM(D3DPT_TRIANGLEFAN, FVF_TEX0, FadeVert, 4, D3DDP_DONOTUPDATEEXTENTS | D3DDP_DONOTCLIP);
    SET_RENDER_STATE(D3DRENDERSTATE_ALPHATESTENABLE, TRUE);
}

////////////////////////////////
// render global pickup flash //
////////////////////////////////

void RenderGlobalPickupFlash(void)
{
    long rgb;

// set rgb's

    FTOL((float)sin(GlobalPickupFlash * RAD * 12.0f) * 127.0f + 128.0f, rgb);
//  rgb = rgb | rgb << 8 | rgb << 16;

    FadeVert[0].color = rgb;
    FadeVert[1].color = rgb;
    FadeVert[2].color = rgb;
    FadeVert[3].color = rgb;

// set viewport

    SetViewport(0.0f, 0.0f, (float)ScreenXsize, (float)ScreenYsize, RenderSettings.GeomPers);

// render

    FOG_OFF();
    ZBUFFER_OFF();
    SET_TPAGE(-1);
    BLEND_ON();
    BLEND_SRC(D3DBLEND_ONE);
    BLEND_DEST(D3DBLEND_ONE);

    SET_RENDER_STATE(D3DRENDERSTATE_ALPHATESTENABLE, FALSE);
    DRAW_PRIM(D3DPT_TRIANGLEFAN, FVF_TEX0, FadeVert, 4, D3DDP_DONOTUPDATEEXTENTS | D3DDP_DONOTCLIP);
    SET_RENDER_STATE(D3DRENDERSTATE_ALPHATESTENABLE, TRUE);
}

/////////////////////
// draw sepia shit //
/////////////////////

void DrawSepiaShit(void)
{
    static  REAL delay = 0;
    static  long rgb;
    static  REAL intensity;

// render flicker
    delay -= TimeStep;
    if (delay <= 0)
    {
//      intensity = frand(Real(56)) + Real(200);
        intensity = frand(Real(156)) + Real(100);
        delay = frand(Real(0.05)) + Real(0.05);
    }

    FTOL(intensity, rgb);
    rgb = (256 - rgb) << 24;
    intensity -= TimeStep * Real(64);

    FadeVert[0].color = rgb;
    FadeVert[1].color = rgb;
    FadeVert[2].color = rgb;
    FadeVert[3].color = rgb;

    FOG_OFF();
    ZBUFFER_OFF();
    SET_TPAGE(-1);
    BLEND_ALPHA();
    BLEND_SRC(D3DBLEND_SRCALPHA);
    BLEND_DEST(D3DBLEND_INVSRCALPHA);

    SET_RENDER_STATE(D3DRENDERSTATE_ALPHATESTENABLE, FALSE);
    DRAW_PRIM(D3DPT_TRIANGLEFAN, FVF_TEX0, FadeVert, 4, D3DDP_DONOTUPDATEEXTENTS | D3DDP_DONOTCLIP);
    SET_RENDER_STATE(D3DRENDERSTATE_ALPHATESTENABLE, TRUE);


// Draw film lines
    DrawFilmLines();
}


/////////////////////////////////////////////////
// FilmLines
/////////////////////////////////////////////////
void ProcessFilmLine(t_FilmLine* pFilm, REAL dT)
{
    REAL    x,y;

// Ready to start ?
    if (pFilm->delay >= 0)
    {
        pFilm->delay -= dT;
        if (pFilm->delay <= 0)
        {
            pFilm->gfx = 0;
            pFilm->alpha = ((rand() & 63) + 32) << 24;
            pFilm->lifeSpan = frand(Real(3)) + Real(0.5);
            pFilm->pos = frand(Real(1));
            pFilm->vel = frand(Real(0.2)) + Real(0.1);
//          if (pFilm->pos > HALF)
            if (rand()&1)
                pFilm->vel = -pFilm->vel;
        }

        return;
    }

// Render line
    x = (REAL)ScreenXsize * pFilm->pos;
    if ((x >= 0) && (x < ScreenXsize-1))
    {
        y = (REAL)ScreenYsize-1;

        gpFilmLineVert->sx = x;
        gpFilmLineVert->sy = 0;
        gpFilmLineVert->sz = 0;
        gpFilmLineVert->rhw = 1;
        gpFilmLineVert->tu = 0;
        gpFilmLineVert->tv = 0;
        gpFilmLineVert->color = 0xFFFFFF | pFilm->alpha;
        gpFilmLineVert++;

        gpFilmLineVert->sx = x + 1;
        gpFilmLineVert->sy = 0;
        gpFilmLineVert->sz = 0;
        gpFilmLineVert->rhw = 1;
        gpFilmLineVert->tu = 1;
        gpFilmLineVert->tv = 0;
        gpFilmLineVert->color = 0xFFFFFF | pFilm->alpha;
        gpFilmLineVert++;

        gpFilmLineVert->sx = x + 1;
        gpFilmLineVert->sy = y;
        gpFilmLineVert->sz = 0;
        gpFilmLineVert->rhw = 1;
        gpFilmLineVert->tu = 0;
        gpFilmLineVert->tv = 1;
        gpFilmLineVert->color = 0xFFFFFF | pFilm->alpha;
        gpFilmLineVert++;

        gpFilmLineVert->sx = x;
        gpFilmLineVert->sy = y;
        gpFilmLineVert->sz = 0;
        gpFilmLineVert->rhw = 1;
        gpFilmLineVert->tu = 0;
        gpFilmLineVert->tv = 1;
        gpFilmLineVert->color = 0xFFFFFF | pFilm->alpha;
        gpFilmLineVert++;

        *gpFilmLineIndex++ = gcFilmLineVert + 0;
        *gpFilmLineIndex++ = gcFilmLineVert + 1;
        *gpFilmLineIndex++ = gcFilmLineVert + 2;
        *gpFilmLineIndex++ = gcFilmLineVert + 0;
        *gpFilmLineIndex++ = gcFilmLineVert + 2;
        *gpFilmLineIndex++ = gcFilmLineVert + 3;

        gcFilmLineIndex += 6;
        gcFilmLineVert += 4;
    }

// Move line
    pFilm->pos += pFilm->vel * dT;

// Dead ?
    pFilm->lifeSpan -= dT;
    if ((pFilm->lifeSpan < 0) || (pFilm->pos < 0) || (pFilm->pos > 1))
        pFilm->delay = frand(Real(3)) + Real(1);
}

void InitFilmLines(void)
{
    int i;

    for (i = 0; i < FILMLINE_MAX; i++)
    {
        gFilmLines[i].delay = frand(Real(3));
    }
}

void DrawFilmLines(void)
{
    REAL    dT;
    int     i;

    FOG_OFF();
    ZBUFFER_OFF();
    SET_TPAGE(-1);
    BLEND_ALPHA();
//  BLEND_SRC(D3DBLEND_SRCALPHA);
//  BLEND_DEST(D3DBLEND_INVSRCALPHA);
    BLEND_SRC(D3DBLEND_ZERO);
    BLEND_DEST(D3DBLEND_INVSRCALPHA);

    gpFilmLineVert = DrawVertsTEX1;
    gcFilmLineVert = 0;
    gpFilmLineIndex = giFilmLineIndex;
    gcFilmLineIndex = 0;

    dT = NPhysicsLoops * PHYSICSTIMESTEP/1000.0f;
    for (i = 0; i < FILMLINE_MAX; i++)
    {
        ProcessFilmLine(&gFilmLines[i], TimeStep);
    }

	if( gcFilmLineVert > 0 )
	{
	    SET_RENDER_STATE(D3DRENDERSTATE_ALPHATESTENABLE, FALSE);
		DRAW_PRIM_INDEX(D3DPT_TRIANGLELIST, FVF_TEX1, DrawVertsTEX1, gcFilmLineVert, giFilmLineIndex, gcFilmLineIndex, D3DDP_DONOTUPDATEEXTENTS | D3DDP_DONOTCLIP);
		SET_RENDER_STATE(D3DRENDERSTATE_ALPHATESTENABLE, TRUE);
	}
}

