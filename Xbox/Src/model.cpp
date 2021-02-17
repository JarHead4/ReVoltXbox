//-----------------------------------------------------------------------------
// File: model.cpp
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"
#include "main.h"
#include "dx.h"
#include "geom.h"
#include "model.h"
#include "text.h"
#include "light.h"
#include "draw.h"
#include "network.h"
#include "particle.h"
#include "aerial.h"
#include "NewColl.h"
#include "body.h"
#include "car.h"
#include "camera.h"
#include "LevelLoad.h"
#include "settings.h"
#include "mirror.h"
#include "ctrlread.h"
#include "object.h"
#include "control.h"
#include "player.h"
#include "timing.h"

// globals

float ModelVertFog;
short ModelPolyCount, EnvTpage;
MAT EnvMatrix;
MODEL_RGB EnvRgb;
long ModelAddLit;
REAL ModelScale;
float EnvXoffset, EnvYoffset, EnvScale;
float ModelGhostSineOffset, ModelGhostSinePos, ModelGhostFadeMul, GhostSineCount, GhostSinePos;
LEVEL_MODEL LevelModel[MAX_LEVEL_MODELS];
MODEL *ModelMeshModel;
MAT *ModelMeshMat;
VEC *ModelMeshPos;
short *ModelMeshFlag;

static VEC ModelSpaceCameraPos;
static long EnvPer;
//$MODIFIED
//static char  AerialSectionName[] = "CARS\\misc\\Aerial.m";
//static char  AerialTopName[] = "CARS\\misc\\Aerialt.m";
static char  AerialSectionName[] = "D:\\CARS\\misc\\Aerial.m";
static char  AerialTopName[]     = "D:\\CARS\\misc\\Aerialt.m";
//$END_MODIFICATIONS

static short ModelFog;
static BUCKET_TEX0 *ModelBucketHeadRGB, *ModelBucketHeadClipRGB;
static BUCKET_TEX1 *ModelBucketHead, *ModelBucketHeadClip;
static BUCKET_ENV *ModelBucketHeadEnv, *ModelBucketHeadEnvClip;

// level model list

/*static*/ char *LevelModelList[] = {  //$MODIFIED(jedl) - removed 'static' b/c exporting is easier if this is global
//$MODIFIED
//    "models\\barrel",
//    "models\\beachball",
//    "models\\mercury",
//    "models\\venus",
//    "models\\earth",
//    "models\\mars",
//    "models\\jupiter",
//    "models\\saturn",
//    "models\\uranus",
//    "models\\neptune",
//    "models\\pluto",
//    "models\\moon",
//    "models\\rings",
//    "models\\plane",
//    "models\\plane2",
//    "models\\copter",
//    "models\\copter2",
//    "models\\copter3",
//    "models\\dragon1",
//    "models\\dragon2",
//    "models\\water",
//    "models\\boat1",
//    "models\\boat2",
//    "models\\speedup",
//    "models\\radar",
//    "models\\balloon",
//    "models\\horse",
//    "models\\train",
//    "models\\train2",
//    "models\\train3",
//    "models\\light1",
//    "models\\light2",
//    "models\\light3",
//    "models\\football",
//    "models\\spaceman",
//    "models\\pickup",
//    "models\\flap",
//    //"models\\laser",
//    "edit\\spot",
//    "models\\firework",
//    "models\\ball",
//    "models\\wbomb",
//    "models\\ball",
//    "models\\cone",
//    "models\\Probe",
//    "models\\spbase",
//    "models\\sphead",
//    "models\\sphose",
//    "models\\basketball",
//    "levels\\frontend\\BigScreen",
//    "levels\\frontend\\ClockBody",
//    "levels\\frontend\\ClockHandLarge",
//    "levels\\frontend\\ClockHandSmall",
//    "levels\\frontend\\ClockDisc",
//    "levels\\frontend\\CarBox",
//    "levels\\frontend\\PlainBox",
//    "levels\\frontend\\namestand",
//    "levels\\frontend\\namewheel",
//    "models\\stream",
//    "levels\\frontend\\cup04",
//    "levels\\frontend\\cup02",
//    "levels\\frontend\\cup01",
//    "levels\\frontend\\cup03",
//    "models\\star",
//    "models\\tumble_weed",
//    "levels\\frontend\\smallscreen",
//    "models\\lantern",
//    "models\\slider",
//    "models\\bottle",
//    "models\\poolwater",
//    "models\\gardenwater1",
//    "models\\gardenwater2",
//    "models\\gardenwater3",
//    "models\\gardenwater4",
//    "models\\bucket",
//    "models\\trafficcone",
//    "models\\n2trafficcone",
//    "models\\lantern",//"models\\can",
//    "models\\lantern",//"models\\lilo",
//    "models\\shiplight",
//    "models\\packet",
//    "models\\packet1",
//    "models\\abcblock",
//    "models\\spbasegarden",
//    "models\\spheadgarden",
    "D:\\models\\barrel",
    "D:\\models\\beachball",
    "D:\\models\\mercury",
    "D:\\models\\venus",
    "D:\\models\\earth",
    "D:\\models\\mars",
    "D:\\models\\jupiter",
    "D:\\models\\saturn",
    "D:\\models\\uranus",
    "D:\\models\\neptune",
    "D:\\models\\pluto",
    "D:\\models\\moon",
    "D:\\models\\rings",
    "D:\\models\\plane",
    "D:\\models\\plane2",
    "D:\\models\\copter",
    "D:\\models\\copter2",
    "D:\\models\\copter3",
    "D:\\models\\dragon1",
    "D:\\models\\dragon2",
    "D:\\models\\water",
    "D:\\models\\boat1",
    "D:\\models\\boat2",
    "D:\\models\\speedup",
    "D:\\models\\radar",
    "D:\\models\\balloon",
    "D:\\models\\horse",
    "D:\\models\\train",
    "D:\\models\\train2",
    "D:\\models\\train3",
    "D:\\models\\light1",
    "D:\\models\\light2",
    "D:\\models\\light3",
    "D:\\models\\football",
    "D:\\models\\spaceman",
    "D:\\models\\pickup",
    "D:\\models\\flap",
    //"D:\\models\\laser",
    "D:\\edit\\spot",
    "D:\\models\\firework",
    "D:\\models\\ball",
    "D:\\models\\wbomb",
    "D:\\models\\ball",
    "D:\\models\\cone",
    "D:\\models\\Probe",
    "D:\\models\\spbase",
    "D:\\models\\sphead",
    "D:\\models\\sphose",
    "D:\\models\\basketball",
    "D:\\levels\\frontend\\BigScreen",
    "D:\\levels\\frontend\\ClockBody",
    "D:\\levels\\frontend\\ClockHandLarge",
    "D:\\levels\\frontend\\ClockHandSmall",
    "D:\\levels\\frontend\\ClockDisc",
    "D:\\levels\\frontend\\CarBox",
    "D:\\levels\\frontend\\PlainBox",
    "D:\\levels\\frontend\\namestand",
    "D:\\levels\\frontend\\namewheel",
    "D:\\models\\stream",
    "D:\\levels\\frontend\\cup04",
    "D:\\levels\\frontend\\cup02",
    "D:\\levels\\frontend\\cup01",
    "D:\\levels\\frontend\\cup03",
    "D:\\models\\star",
    "D:\\models\\tumble_weed",
    "D:\\levels\\frontend\\smallscreen",
    "D:\\models\\lantern",
    "D:\\models\\slider",
    "D:\\models\\bottle",
    "D:\\models\\poolwater",
    "D:\\models\\gardenwater1",
    "D:\\models\\gardenwater2",
    "D:\\models\\gardenwater3",
    "D:\\models\\gardenwater4",
    "D:\\models\\bucket",
    "D:\\models\\trafficcone",
    "D:\\models\\n2trafficcone",
    "D:\\models\\lantern",//"D:\\models\\can",
    "D:\\models\\lantern",//"D:\\models\\lilo",
    "D:\\models\\shiplight",
    "D:\\models\\packet",
    "D:\\models\\packet1",
    "D:\\models\\abcblock",
    "D:\\models\\spbasegarden",
    "D:\\models\\spheadgarden",
//$END_MODIFICATIONS
};



//$ADDITION(jedl) - export / import rewrite.
//-----------------------------------------------------------------------------
// Name: LoadModelGPU
// Desc: Loads a model from a XBResource.
//-----------------------------------------------------------------------------
long LoadModelGPU(char *file,       // name of model file
                  MODEL *m,
                  INT prmlevel,     // max number of level-of-detail models
                  XBResource *pXBR) // resource to look for models
{
    INT count = 0;
    CHAR drive[_MAX_DRIVE];
    CHAR dir[_MAX_DIR];
    CHAR name[_MAX_PATH];
    CHAR ext[_MAX_EXT];
    _splitpath( file, drive, dir, name, ext );

    // Look for effect with the right name
    if (prmlevel == 1)
    {
        m->m_pEffect = pXBR->GetEffect(name);
        if (m->m_pEffect != NULL)
            return 1;
        else 
            return 0;
    }

    // Look for multiple level of detail effects
    for (INT iModel = 0; iModel < prmlevel; iModel++, m++)
    {
        const int maxName = 1000;
        CHAR strModelName[maxName];
        _snprintf(strModelName, maxName, "%s%d", name, iModel);
        m->m_pEffect = pXBR->GetEffect(strModelName);
        if (m->m_pEffect == NULL)
        {
            if (iModel == 0)
            {
                // Try looking for name without appended 0, since there 
                // might be just one model
                m->m_pEffect = pXBR->GetEffect(name);
                if (m->m_pEffect == NULL)
                {
                    CHAR strMessage[1000];
                    sprintf(strMessage, "Model \"%s\" resource not found.", strModelName);
                    DumpMessage("Warning", strMessage);
                }
            }
            else
            {
                // No more levels-of-detail found, so fill in zeros
                for ( ; iModel < prmlevel ; iModel++, m++)
                    m->m_pEffect = NULL;
                break;
            }
        }
        count++;
    }   
    return count;
}
//$END_ADDITION



//////////////////
// load a model //
//////////////////

long LoadModel(char *file, MODEL *m, char tpage, char prmlevel, char loadflag, long RgbPer)
{
//$ADDITION(jedl) - Eventually all the loader code will go away. For
// now, we keep it around to do a visual comparison with the
// exported and then re-imported resource drawing.
    m->m_pEffect = NULL;
//$END_ADDITION
    
    FILE *fp;
    MODEL_HEADER mh;
    MODEL_POLY_LOAD mpl;
    MODEL_VERTEX_LOAD mvl;
    POLY_RGB *mrgb, rgb;
    MODEL_POLY *mp, poly;
    float rad;
    long size, i, j, k, a, b, count;
    char buf[128];
    char prm;

// open file for reading

    if (file == NULL || file[0] == '\0')
        return FALSE;

    fp = fopen(file, "rb");
    if (fp == NULL)
    {
        sprintf(buf, "Can't load model %s", file);
        DumpMessage(NULL, buf);
        return FALSE;
    }

// loop thru prm count

    count = 0;

    for (prm = 0 ; prm < prmlevel ; prm++, m++, count++)
    {

// get header info

        fread(&mh, sizeof(mh), 1, fp);

// NULL remaining models if end of file

        if (feof(fp))
        {
            for ( ; prm < prmlevel ; prm++, m++)
            {
                m->PolyNum = 0;
                m->VertNum = 0;
                m->AllocPtr = NULL;
                m->VertPtrMorph = NULL;
            }
            break;
        }

// alloc model memory

        m->PolyNum = mh.PolyNum;
        m->VertNum = mh.VertNum;

        size = sizeof(POLY_RGB) * m->PolyNum;
        size += sizeof(MODEL_POLY) * m->PolyNum;
        size += sizeof(MODEL_VERTEX) * m->VertNum;

        SetMallocString(file);

        m->AllocPtr = malloc(size);
        if (m->AllocPtr == NULL)
        {
            sprintf(buf, "Can't alloc memory for %s (%d)", file, prm);
            DumpMessage("ERROR", buf);
            return FALSE;
        }

        SetMallocString("");

        m->PolyRGB = (POLY_RGB*)(m->AllocPtr);
        m->PolyPtr = (MODEL_POLY*)(m->PolyRGB + m->PolyNum);
        m->VertPtr = (MODEL_VERTEX*)(m->PolyPtr + m->PolyNum);
        m->VertPtrMorph = NULL;

// load polys - count textured / rgb + quads / tris's

        mrgb = m->PolyRGB;
        mp = m->PolyPtr;

        m->QuadNumTex = 0;
        m->TriNumTex = 0;
        m->QuadNumRGB = 0;
        m->TriNumRGB = 0;

        for (i = 0 ; i < m->PolyNum ; i++, mrgb++, mp++)
        {
            fread(&mpl, sizeof(mpl), 1, fp);

            mp->Type = mpl.Type;

            if ((mp->Tpage = mpl.Tpage) != -1)
            {
                if (loadflag & LOADMODEL_FORCE_TPAGE) mp->Tpage = tpage;
                if (loadflag & LOADMODEL_OFFSET_TPAGE) mp->Tpage += tpage;
            }

            if (mp->Tpage < -1 || mp->Tpage > MAX_POLY_BUCKETS - 1)
            {
                sprintf(buf, "Dodgy poly texture num %d in %s", mp->Tpage, file);
                DumpMessage(NULL, buf);
                mp->Tpage = -1;
            }

            if (GameSettings.Mirrored)
            {
                if (mp->Type & POLY_QUAD)
                {
                    *(long*)&mrgb->rgb[0] = mpl.c3;
                    *(long*)&mrgb->rgb[1] = mpl.c2;
                    *(long*)&mrgb->rgb[2] = mpl.c1;
                    *(long*)&mrgb->rgb[3] = mpl.c0;

                    mp->tu0 = mpl.u3;
                    mp->tv0 = mpl.v3;
                    mp->tu1 = mpl.u2;
                    mp->tv1 = mpl.v2;
                    mp->tu2 = mpl.u1;
                    mp->tv2 = mpl.v1;
                    mp->tu3 = mpl.u0;
                    mp->tv3 = mpl.v0;

                    mp->v0 = m->VertPtr + mpl.vi3;
                    mp->v1 = m->VertPtr + mpl.vi2;
                    mp->v2 = m->VertPtr + mpl.vi1;
                    mp->v3 = m->VertPtr + mpl.vi0;
                }
                else
                {
                    *(long*)&mrgb->rgb[0] = mpl.c2;
                    *(long*)&mrgb->rgb[1] = mpl.c1;
                    *(long*)&mrgb->rgb[2] = mpl.c0;

                    mp->tu0 = mpl.u2;
                    mp->tv0 = mpl.v2;
                    mp->tu1 = mpl.u1;
                    mp->tv1 = mpl.v1;
                    mp->tu2 = mpl.u0;
                    mp->tv2 = mpl.v0;

                    mp->v0 = m->VertPtr + mpl.vi2;
                    mp->v1 = m->VertPtr + mpl.vi1;
                    mp->v2 = m->VertPtr + mpl.vi0;
                }
            }
            else
            {
                *(long*)&mrgb->rgb[0] = mpl.c0;
                *(long*)&mrgb->rgb[1] = mpl.c1;
                *(long*)&mrgb->rgb[2] = mpl.c2;
                *(long*)&mrgb->rgb[3] = mpl.c3;

                mp->tu0 = mpl.u0;
                mp->tv0 = mpl.v0;
                mp->tu1 = mpl.u1;
                mp->tv1 = mpl.v1;
                mp->tu2 = mpl.u2;
                mp->tv2 = mpl.v2;
                mp->tu3 = mpl.u3;
                mp->tv3 = mpl.v3;

                mp->v0 = m->VertPtr + mpl.vi0;
                mp->v1 = m->VertPtr + mpl.vi1;
                mp->v2 = m->VertPtr + mpl.vi2;
                mp->v3 = m->VertPtr + mpl.vi3;
            }

            ModelChangeGouraud(&mrgb->rgb[0], RgbPer);
            ModelChangeGouraud(&mrgb->rgb[1], RgbPer);
            ModelChangeGouraud(&mrgb->rgb[2], RgbPer);
            ModelChangeGouraud(&mrgb->rgb[3], RgbPer);

            if (RenderSettings.Sepia)
            {
                Grayscale((long*)&mrgb->rgb[0]);
                Grayscale((long*)&mrgb->rgb[1]);
                Grayscale((long*)&mrgb->rgb[2]);
                Grayscale((long*)&mrgb->rgb[3]);
            }

            if (mp->Tpage == -1)
            {
                mp->Tpage = TPAGE_FX1;

                mp->tu0 = 220.0f / 256.0f;
                mp->tv0 = 156.0f / 256.0f;
                mp->tu1 = 228.0f / 256.0f;
                mp->tv1 = 156.5f / 256.0f;
                mp->tu2 = 228.0f / 256.0f;
                mp->tv2 = 164.5f / 256.0f;
                mp->tu3 = 220.0f / 256.0f;
                mp->tv3 = 164.5f / 256.0f;
            }

            if (mp->Tpage != -1)
            {
                if (mp->Type & POLY_QUAD) m->QuadNumTex++;
                else m->TriNumTex++;
            }
            else
            {
                if (mp->Type & POLY_QUAD) m->QuadNumRGB++;
                else m->TriNumRGB++;
            }
        }

// sort polys into textured / untextured + quads / tri's

        mp = m->PolyPtr;

        for (j = m->PolyNum - 1; j ; j--) for (k = 0 ; k < j ; k++)
        {
            a = mp[k].Type & POLY_QUAD;
            if (mp[k].Tpage != -1) a += 256;

            b = mp[k + 1].Type & POLY_QUAD;
            if (mp[k + 1].Tpage != -1) b += 256;

            if (b > a)
            {
                poly = mp[k];
                mp[k] = mp[k + 1];
                mp[k + 1] = poly;

                rgb = m->PolyRGB[k];
                m->PolyRGB[k] = m->PolyRGB[k + 1];
                m->PolyRGB[k + 1] = rgb;
            }
        }

// load verts, set radius / bounding box

        m->Radius = 0;
        m->Xmin = m->Ymin = m->Zmin = 999999;
        m->Xmax = m->Ymax = m->Zmax = -999999;

        for (i = 0 ; i < m->VertNum ; i++)
        {
            fread(&mvl, sizeof(mvl), 1, fp);

            m->VertPtr[i].x = mvl.x;
            m->VertPtr[i].y = mvl.y;
            m->VertPtr[i].z = mvl.z;

            m->VertPtr[i].nx = mvl.nx;
            m->VertPtr[i].ny = mvl.ny;
            m->VertPtr[i].nz = mvl.nz;

            rad = Length((VEC*)&m->VertPtr[i].x);
            if (rad > m->Radius) m->Radius = rad;

            if (m->VertPtr[i].x < m->Xmin) m->Xmin = m->VertPtr[i].x;
            if (m->VertPtr[i].x > m->Xmax) m->Xmax = m->VertPtr[i].x;
            if (m->VertPtr[i].y < m->Ymin) m->Ymin = m->VertPtr[i].y;
            if (m->VertPtr[i].y > m->Ymax) m->Ymax = m->VertPtr[i].y;
            if (m->VertPtr[i].z < m->Zmin) m->Zmin = m->VertPtr[i].z;
            if (m->VertPtr[i].z > m->Zmax) m->Zmax = m->VertPtr[i].z;
        }

// build poly planes

        mp = m->PolyPtr;
        for (i = 0 ; i < m->PolyNum ; i++, mp++)
        {
            if (GameSettings.Mirrored)
                BuildPlane((VEC*)&mp->v2->x, (VEC*)&mp->v1->x, (VEC*)&mp->v0->x, &mp->Plane);
            else
                BuildPlane((VEC*)&mp->v0->x, (VEC*)&mp->v1->x, (VEC*)&mp->v2->x, &mp->Plane);
        }
    }

// close file

    fclose(fp);
    return count;
}

//////////////////
// free a model //
//////////////////

void FreeModel(MODEL *m, long prmlevel)
{
    long i;

    for (i = 0 ; i < prmlevel ; i++, m++)
    {
        if (m->VertPtrMorph)
            free(m->VertPtrMorph);

        if (m->AllocPtr)
            free(m->AllocPtr);

        m->AllocPtr = NULL;
        m->VertPtrMorph = NULL;
    }
}

//$ADDITION(jedl)
//-----------------------------------------------------------------------------
// Name: DrawModelGPU
// Desc: Draws a model using packed resources and the GPU.
//-----------------------------------------------------------------------------
void DrawModelGPU(MODEL *m, MAT *worldmat, VEC *worldpos, short flag)
{
	// Fill in world matrix
    XGMATRIX mat;
    mat._11 = worldmat->m[RX];
    mat._12 = worldmat->m[UX];
    mat._13 = worldmat->m[LX];
    mat._14 = worldpos->v[0];

    mat._21 = worldmat->m[RY];
    mat._22 = worldmat->m[UY];
    mat._23 = worldmat->m[LY];
    mat._24 = worldpos->v[1];

    mat._31 = worldmat->m[RZ];
    mat._32 = worldmat->m[UZ];
    mat._33 = worldmat->m[LZ];
    mat._34 = worldpos->v[2];

    mat._41 = 0.f;
    mat._42 = 0.f;
    mat._43 = 0.f;
    mat._44 = 1.f;
	
    // Concatenate world matrix to projection matrix
    // $TODO: communication between app and effect is still being worked out
    extern XGMATRIX g_Proj;
    XGMATRIX matProj = g_Proj * mat;
        
    // Draw the effect
	if (m->m_pEffect == NULL) return; // $TODO: this should be    Assert(m->m_pEffect != NULL);
    Effect::BeginDraw();    // $TODO: this should not be needed, but just in case we're not in a BeginDraw .. EndDraw block
    m->m_pEffect->DrawEffect((VOID *)&matProj, 4);
    Effect::EndDraw();
 }
//$END_ADDITION



//////////////////
// draw a model //
//////////////////

void DrawModel(MODEL *m, MAT *worldmat, VEC *worldpos, short flag)
{
//$ADDITION(jedl) - New rendering pipeline.
    if ( RegistrySettings.bUseGPU )
    {
        if ( m->m_pEffect != NULL )			    // $TODO: make this an assert when all the art is in the tree
			DrawModelGPU(m, worldmat, worldpos, flag);
        return;
    }
//$END_ADDITION

    long i;
    MAT eyematrix;
    VEC mirrorpos, eyetrans, vec, vec2;
    float mirroradd, z;
    MODEL_VERTEX *mv;

// kill env flag if env off or no draw

    if (!RenderSettings.Env || flag & MODEL_NODRAW)
        flag &= ~(MODEL_ENV | MODEL_ENVGOOD);

// kill env if out of range

    if (flag & (MODEL_ENV | MODEL_ENVGOOD))
    {
        z = worldpos->v[X] * ViewMatrix.m[RZ] + worldpos->v[Y] * ViewMatrix.m[UZ] + worldpos->v[Z] * ViewMatrix.m[LZ] + ViewTrans.v[Z];
        if (z > 3000.0f)
        {
            flag &= ~(MODEL_ENV | MODEL_ENVGOOD);
        }
        else
        {
            FTOL((3000.0f - z) * 0.1f, EnvPer);
            if (EnvPer > 100) EnvPer = 100;
        }
    }

// set fog flag

    ModelFog = flag & MODEL_FOG;

// set bucket heads

    if (!(flag & MODEL_NODRAW))
    {
        if (ModelFog)
        {
            FOG_ON();

            ModelBucketHead = BucketFog;
            ModelBucketHeadRGB = &BucketFogRGB;
            ModelBucketHeadClip = BucketClipFog;
            ModelBucketHeadClipRGB = &BucketClipFogRGB;
        }
        else
        {
            ModelBucketHead = Bucket;
            ModelBucketHeadRGB = &BucketRGB;
            ModelBucketHeadClip = BucketClip;
            ModelBucketHeadClipRGB = &BucketClipRGB;
        }
    }

// set eye mat + trans

    MulMatrix(&ViewMatrixScaled, worldmat, &eyematrix);
    RotTransVector(&ViewMatrixScaled, &ViewTransScaled, worldpos, &eyetrans);

// set model space camera pos if near clipping

    if (flag & MODEL_NEARCLIP)
    {
        SubVector(&ViewCameraPos, worldpos, &vec);
        TransposeRotVector(worldmat, &vec, &ModelSpaceCameraPos);
    }

// ghost?

    if (flag & MODEL_GHOST)
    {
        SetModelVertsGhost(m);
    }

// env?

    if (flag & MODEL_ENV)
    {
        if (ModelFog)
        {
            if (EnvTpage == TPAGE_ENVSTILL)
            {
                ModelBucketHeadEnv = &BucketEnvStillFog;
                ModelBucketHeadEnvClip = &BucketEnvStillClipFog;
            }
            else
            {
                ModelBucketHeadEnv = &BucketEnvRollFog;
                ModelBucketHeadEnvClip = &BucketEnvRollClipFog;
            }
        }
        else
        {
            if (EnvTpage == TPAGE_ENVSTILL)
            {
                ModelBucketHeadEnv = &BucketEnvStill;
                ModelBucketHeadEnvClip = &BucketEnvStillClip;
            }
            else
            {
                ModelBucketHeadEnv = &BucketEnvRoll;
                ModelBucketHeadEnvClip = &BucketEnvRollClip;
            }
        }

        if (flag & MODEL_LIT) SetModelVertsEnvLit(m);
        else SetModelVertsEnvPlain(m);
    }

// env good?

    if (flag & MODEL_ENVGOOD)
    {
        flag |= MODEL_ENV;

        if (ModelFog)
        {
            ModelBucketHeadEnv = &BucketEnvStillFog;
            ModelBucketHeadEnvClip = &BucketEnvStillClipFog;
        }
        else
        {
            ModelBucketHeadEnv = &BucketEnvStill;
            ModelBucketHeadEnvClip = &BucketEnvStillClip;
        }

        SubVector(&ViewCameraPos, worldpos, &vec2);
        TransposeRotVector(worldmat, &vec2, &vec);
        TransposeRotVector(worldmat, &ViewCameraMatrix.mv[U], &vec2);

        if (flag & MODEL_LIT) SetModelVertsEnvGoodLit(m, &vec, &vec2);
        else SetModelVertsEnvGoodPlain(m, &vec, &vec2);
    }

// add lit?

    if (flag & MODEL_ADDLIT)
    {
        mv = m->VertPtr;

        if (flag & MODEL_LIT)
        {
            for (i = m->VertNum ; i ; i--, mv++)
            {
                mv->r += ModelAddLit;
                mv->g += ModelAddLit;
                mv->b += ModelAddLit;
            }
        }
        else
        {
            for (i = m->VertNum ; i ; i--, mv++)
            {
                mv->r = ModelAddLit;
                mv->g = ModelAddLit;
                mv->b = ModelAddLit;
            }

            flag |= MODEL_LIT;
        }
    }

// scale?

    if (flag & MODEL_SCALE)
    {
        VecMulScalar(&eyematrix.mv[R], ModelScale);
        VecMulScalar(&eyematrix.mv[U], ModelScale);
        VecMulScalar(&eyematrix.mv[L], ModelScale);
    }

// draw

    if (!(flag & MODEL_NODRAW))
    {
        if (flag & MODEL_USENEWVERTS)
        {
            if (flag & MODEL_DONOTCLIP)
            {
                if (ModelFog) TransModelVertsFogNewVerts(m, &eyematrix, &eyetrans);
                else TransModelVertsPlainNewVerts(m, &eyematrix, &eyetrans);

                DrawModelPolys(m, flag & MODEL_LIT, flag & MODEL_ENV);
            }
            else
            {
                if (ModelFog) TransModelVertsFogClipNewVerts(m, &eyematrix, &eyetrans);
                else TransModelVertsPlainClipNewVerts(m, &eyematrix, &eyetrans);

                if (flag & MODEL_NEARCLIP) DrawModelPolysNearClip(m, flag & MODEL_LIT, flag & MODEL_ENV);
                else DrawModelPolysClip(m, flag & MODEL_LIT, flag & MODEL_ENV);
            }
        }
        else
        {
            if (flag & MODEL_DONOTCLIP)
            {
                if (ModelFog) TransModelVertsFog(m, &eyematrix, &eyetrans);
                else TransModelVertsPlain(m, &eyematrix, &eyetrans);

                DrawModelPolys(m, flag & MODEL_LIT, flag & MODEL_ENV);
            }
            else
            {
                if (ModelFog) TransModelVertsFogClip(m, &eyematrix, &eyetrans);
                else TransModelVertsPlainClip(m, &eyematrix, &eyetrans);

                if (flag & MODEL_NEARCLIP) DrawModelPolysNearClip(m, flag & MODEL_LIT, flag & MODEL_ENV);
                else DrawModelPolysClip(m, flag & MODEL_LIT, flag & MODEL_ENV);
            }
        }
    }

// mirrored?

    if (flag & MODEL_MIRROR)
    {
        mirroradd = MirrorHeight - worldpos->v[Y];
        if (mirroradd > -MIRROR_OVERLAP_TOL)
        {
            if (!ModelFog)
            {
                FOG_ON();

                ModelBucketHead = BucketFog;
                ModelBucketHeadRGB = &BucketFogRGB;
                ModelBucketHeadClip = BucketClipFog;
                ModelBucketHeadClipRGB = &BucketClipFogRGB;
            }

            mirrorpos.v[X] = worldpos->v[X];
            mirrorpos.v[Y] = MirrorHeight + mirroradd;
            mirrorpos.v[Z] = worldpos->v[Z];

            MulMatrix(&ViewMatrixScaledMirrorY, worldmat, &eyematrix);
            RotTransVector(&ViewMatrixScaled, &ViewTransScaled, &mirrorpos, &eyetrans);

            if (flag & MODEL_SCALE)
            {
                VecMulScalar(&eyematrix.mv[R], ModelScale);
                VecMulScalar(&eyematrix.mv[U], ModelScale);
                VecMulScalar(&eyematrix.mv[L], ModelScale);
            }

            if (flag & MODEL_USENEWVERTS)
            {
                TransModelVertsMirrorNewVerts(m, &eyematrix, &eyetrans, worldmat, worldpos);
                DrawModelPolysMirror(m, flag & MODEL_LIT);
            }
            else
            {
                TransModelVertsMirror(m, &eyematrix, &eyetrans, worldmat, worldpos);
                DrawModelPolysMirror(m, flag & MODEL_LIT);
            }
        }
    }

// glare?

    if (flag & MODEL_GLARE)
    {
        SetModelVertsGlare(m, worldpos, worldmat, flag);
    }

// fog off

    FOG_OFF();
}

///////////////////////
// trans model verts //
///////////////////////

void TransModelVertsPlainClip(MODEL *m, MAT *mat, VEC *trans)
{
    short i;
    float z;
    float nearclip = RenderSettings.NearClip;
    MODEL_VERTEX *mv;

    mv = m->VertPtr;

    for (i = 0 ; i < m->VertNum ; i++, mv++)
    {
        z = mv->x * mat->m[RZ] + mv->y * mat->m[UZ] + mv->z * mat->m[LZ] + trans->v[Z];
        mv->sx = (mv->x * mat->m[RX] + mv->y * mat->m[UX] + mv->z * mat->m[LX] + trans->v[X]) / z + RenderSettings.GeomCentreX;
        mv->sy = (mv->x * mat->m[RY] + mv->y * mat->m[UY] + mv->z * mat->m[LY] + trans->v[Y]) / z + RenderSettings.GeomCentreY;

        mv->rhw = 1 / z;
        mv->sz = GET_ZBUFFER(z);

        mv->Clip = 0;
        if (mv->sx < ScreenLeftClipGuard) mv->Clip |= CLIP_LEFT;
        else if (mv->sx > ScreenRightClipGuard) mv->Clip |= CLIP_RIGHT;
        if (mv->sy < ScreenTopClipGuard) mv->Clip |= CLIP_TOP;
        else if (mv->sy > ScreenBottomClipGuard) mv->Clip |= CLIP_BOTTOM;
//      if (mv->sz < 0) mv->Clip |= CLIP_NEAR;
        if (z < nearclip) mv->Clip |= CLIP_NEAR;
        else if (mv->sz >= 1) mv->Clip |= CLIP_FAR;
    } 
}

///////////////////////
// trans model verts //
///////////////////////

void TransModelVertsFogClip(MODEL *m, MAT *mat, VEC *trans)
{
    short i;
    float z;
    float fog;
    float nearclip = RenderSettings.NearClip;
    MODEL_VERTEX *mv;

    mv = m->VertPtr;

    for (i = 0 ; i < m->VertNum ; i++, mv++)
    {
        z = mv->x * mat->m[RZ] + mv->y * mat->m[UZ] + mv->z * mat->m[LZ] + trans->v[Z];
        mv->sx = (mv->x * mat->m[RX] + mv->y * mat->m[UX] + mv->z * mat->m[LX] + trans->v[X]) / z + RenderSettings.GeomCentreX;
        mv->sy = (mv->x * mat->m[RY] + mv->y * mat->m[UY] + mv->z * mat->m[LY] + trans->v[Y]) / z + RenderSettings.GeomCentreY;

        mv->rhw = 1 / z;
        mv->sz = GET_ZBUFFER(z);

        fog = (RenderSettings.FarClip - z) * RenderSettings.FogMul;
        if (fog > 255) fog = 255;
        fog -= ModelVertFog;
        if (fog < 0) fog = 0;
        mv->specular = FTOL3(fog) << 24;

        mv->Clip = 0;
        if (mv->sx < ScreenLeftClipGuard) mv->Clip |= CLIP_LEFT;
        else if (mv->sx > ScreenRightClipGuard) mv->Clip |= CLIP_RIGHT;
        if (mv->sy < ScreenTopClipGuard) mv->Clip |= CLIP_TOP;
        else if (mv->sy > ScreenBottomClipGuard) mv->Clip |= CLIP_BOTTOM;
//      if (mv->sz < 0) mv->Clip |= CLIP_NEAR;
        if (z < nearclip) mv->Clip |= CLIP_NEAR;
        else if (mv->sz >= 1) mv->Clip |= CLIP_FAR;
    }
}

///////////////////////
// trans model verts //
///////////////////////

void TransModelVertsPlain(MODEL *m, MAT *mat, VEC *trans)
{
    short i;
    float z;
    MODEL_VERTEX *mv;

    mv = m->VertPtr;

    for (i = 0 ; i < m->VertNum ; i++, mv++)
    {
        z = mv->x * mat->m[RZ] + mv->y * mat->m[UZ] + mv->z * mat->m[LZ] + trans->v[Z];
        mv->rhw = 1 / z;

        mv->sx = (mv->x * mat->m[RX] + mv->y * mat->m[UX] + mv->z * mat->m[LX] + trans->v[X]) / z + RenderSettings.GeomCentreX;
        mv->sy = (mv->x * mat->m[RY] + mv->y * mat->m[UY] + mv->z * mat->m[LY] + trans->v[Y]) / z + RenderSettings.GeomCentreY;

        mv->sz = GET_ZBUFFER(z);
    } 
}

///////////////////////
// trans model verts //
///////////////////////

void TransModelVertsFog(MODEL *m, MAT *mat, VEC *trans)
{
    short i;
    float z;
    float fog;
    MODEL_VERTEX *mv;

    mv = m->VertPtr;

    for (i = 0 ; i < m->VertNum ; i++, mv++)
    {
        z = mv->x * mat->m[RZ] + mv->y * mat->m[UZ] + mv->z * mat->m[LZ] + trans->v[Z];
        mv->rhw = 1 / z;

        mv->sx = (mv->x * mat->m[RX] + mv->y * mat->m[UX] + mv->z * mat->m[LX] + trans->v[X]) / z + RenderSettings.GeomCentreX;
        mv->sy = (mv->x * mat->m[RY] + mv->y * mat->m[UY] + mv->z * mat->m[LY] + trans->v[Y]) / z + RenderSettings.GeomCentreY;

        mv->sz = GET_ZBUFFER(z);

        fog = (RenderSettings.FarClip - z) * RenderSettings.FogMul;
        if (fog > 255) fog = 255;
        fog -= ModelVertFog;
        if (fog < 0) fog = 0;
        mv->specular = FTOL3(fog) << 24;
    }
}

///////////////////////
// trans model verts //
///////////////////////

void TransModelVertsPlainClipNewVerts(MODEL *m, MAT *mat, VEC *trans)
{
    short i;
    float z;
    float nearclip = RenderSettings.NearClip;
    MODEL_VERTEX *mv;

    mv = m->VertPtr;

    for (i = 0 ; i < m->VertNum ; i++, mv++)
    {
        z = mv->x2 * mat->m[RZ] + mv->y2 * mat->m[UZ] + mv->z2 * mat->m[LZ] + trans->v[Z];
        mv->sx = (mv->x2 * mat->m[RX] + mv->y2 * mat->m[UX] + mv->z2 * mat->m[LX] + trans->v[X]) / z + RenderSettings.GeomCentreX;
        mv->sy = (mv->x2 * mat->m[RY] + mv->y2 * mat->m[UY] + mv->z2 * mat->m[LY] + trans->v[Y]) / z + RenderSettings.GeomCentreY;

        mv->rhw = 1 / z;
        mv->sz = GET_ZBUFFER(z);

        mv->Clip = 0;
        if (mv->sx < ScreenLeftClipGuard) mv->Clip |= CLIP_LEFT;
        else if (mv->sx > ScreenRightClipGuard) mv->Clip |= CLIP_RIGHT;
        if (mv->sy < ScreenTopClipGuard) mv->Clip |= CLIP_TOP;
        else if (mv->sy > ScreenBottomClipGuard) mv->Clip |= CLIP_BOTTOM;
//      if (mv->sz < 0) mv->Clip |= CLIP_NEAR;
        if (z < nearclip) mv->Clip |= CLIP_NEAR;
        else if (mv->sz >= 1) mv->Clip |= CLIP_FAR;
    } 
}

///////////////////////
// trans model verts //
///////////////////////

void TransModelVertsFogClipNewVerts(MODEL *m, MAT *mat, VEC *trans)
{
    short i;
    float z;
    float fog;
    float nearclip = RenderSettings.NearClip;
    MODEL_VERTEX *mv;

    mv = m->VertPtr;

    for (i = 0 ; i < m->VertNum ; i++, mv++)
    {
        z = mv->x2 * mat->m[RZ] + mv->y2 * mat->m[UZ] + mv->z2 * mat->m[LZ] + trans->v[Z];
        mv->sx = (mv->x2 * mat->m[RX] + mv->y2 * mat->m[UX] + mv->z2 * mat->m[LX] + trans->v[X]) / z + RenderSettings.GeomCentreX;
        mv->sy = (mv->x2 * mat->m[RY] + mv->y2 * mat->m[UY] + mv->z2 * mat->m[LY] + trans->v[Y]) / z + RenderSettings.GeomCentreY;

        mv->rhw = 1 / z;
        mv->sz = GET_ZBUFFER(z);

        fog = (RenderSettings.FarClip - z) * RenderSettings.FogMul;
        if (fog > 255) fog = 255;
        fog -= ModelVertFog;
        if (fog < 0) fog = 0;
        mv->specular = FTOL3(fog) << 24;

        mv->Clip = 0;
        if (mv->sx < ScreenLeftClipGuard) mv->Clip |= CLIP_LEFT;
        else if (mv->sx > ScreenRightClipGuard) mv->Clip |= CLIP_RIGHT;
        if (mv->sy < ScreenTopClipGuard) mv->Clip |= CLIP_TOP;
        else if (mv->sy > ScreenBottomClipGuard) mv->Clip |= CLIP_BOTTOM;
//      if (mv->sz < 0) mv->Clip |= CLIP_NEAR;
        if (z < nearclip) mv->Clip |= CLIP_NEAR;
        else if (mv->sz >= 1) mv->Clip |= CLIP_FAR;
    }
}

///////////////////////
// trans model verts //
///////////////////////

void TransModelVertsPlainNewVerts(MODEL *m, MAT *mat, VEC *trans)
{
    short i;
    float z;
    MODEL_VERTEX *mv;

    mv = m->VertPtr;

    for (i = 0 ; i < m->VertNum ; i++, mv++)
    {
        z = mv->x2 * mat->m[RZ] + mv->y2 * mat->m[UZ] + mv->z2 * mat->m[LZ] + trans->v[Z];
        mv->rhw = 1 / z;

        mv->sx = (mv->x2 * mat->m[RX] + mv->y2 * mat->m[UX] + mv->z2 * mat->m[LX] + trans->v[X]) / z + RenderSettings.GeomCentreX;
        mv->sy = (mv->x2 * mat->m[RY] + mv->y2 * mat->m[UY] + mv->z2 * mat->m[LY] + trans->v[Y]) / z + RenderSettings.GeomCentreY;

        mv->sz = GET_ZBUFFER(z);
    } 
}

///////////////////////
// trans model verts //
///////////////////////

void TransModelVertsFogNewVerts(MODEL *m, MAT *mat, VEC *trans)
{
    short i;
    float z;
    float fog;
    MODEL_VERTEX *mv;

    mv = m->VertPtr;

    for (i = 0 ; i < m->VertNum ; i++, mv++)
    {
        z = mv->x2 * mat->m[RZ] + mv->y2 * mat->m[UZ] + mv->z2 * mat->m[LZ] + trans->v[Z];
        mv->rhw = 1 / z;

        mv->sx = (mv->x2 * mat->m[RX] + mv->y2 * mat->m[UX] + mv->z2 * mat->m[LX] + trans->v[X]) / z + RenderSettings.GeomCentreX;
        mv->sy = (mv->x2 * mat->m[RY] + mv->y2 * mat->m[UY] + mv->z2 * mat->m[LY] + trans->v[Y]) / z + RenderSettings.GeomCentreY;

        mv->sz = GET_ZBUFFER(z);

        fog = (RenderSettings.FarClip - z) * RenderSettings.FogMul;
        if (fog > 255) fog = 255;
        fog -= ModelVertFog;
        if (fog < 0) fog = 0;
        mv->specular = FTOL3(fog) << 24;
    }
}

///////////////////////
// trans model verts //
///////////////////////

void TransModelVertsMirror(MODEL *m, MAT *mat, VEC *trans, MAT *worldmat, VEC *worldpos)
{
    short i;
    float z;
    float fog, mirrorfog;
    float nearclip = RenderSettings.NearClip;
    MODEL_VERTEX *mv;

    mv = m->VertPtr;

    for (i = 0 ; i < m->VertNum ; i++, mv++)
    {
        z = mv->x * mat->m[RZ] + mv->y * mat->m[UZ] + mv->z * mat->m[LZ] + trans->v[Z];
        mv->sx = (mv->x * mat->m[RX] + mv->y * mat->m[UX] + mv->z * mat->m[LX] + trans->v[X]) / z + RenderSettings.GeomCentreX;
        mv->sy = (mv->x * mat->m[RY] + mv->y * mat->m[UY] + mv->z * mat->m[LY] + trans->v[Y]) / z + RenderSettings.GeomCentreY;

        mv->rhw = 1 / z;
        mv->sz = GET_ZBUFFER(z);

        mv->Clip = 0;
        if (mv->sx < ScreenLeftClipGuard) mv->Clip |= CLIP_LEFT;
        else if (mv->sx > ScreenRightClipGuard) mv->Clip |= CLIP_RIGHT;
        if (mv->sy < ScreenTopClipGuard) mv->Clip |= CLIP_TOP;
        else if (mv->sy > ScreenBottomClipGuard) mv->Clip |= CLIP_BOTTOM;
//      if (mv->sz < 0) mv->Clip |= CLIP_NEAR;
        if (z < nearclip) mv->Clip |= CLIP_NEAR;
        else if (mv->sz >= 1) mv->Clip |= CLIP_FAR;

        mirrorfog = mv->x * worldmat->m[RY] + mv->y * worldmat->m[UY] + mv->z * worldmat->m[LY] + worldpos->v[Y];
        mirrorfog = GET_MIRROR_FOG(MirrorHeight - mirrorfog);
        if (mirrorfog < 0) mirrorfog = 0;

        if (mirrorfog >= 255)
        {
            mv->Clip |= CLIP_MAXFOG;
            mv->specular = 0;
        }
        else
        {
            fog = (RenderSettings.FarClip - z) * RenderSettings.FogMul;
            if (fog > 255) fog = 255;
            fog -= mirrorfog;
            if (fog < 0) fog = 0;
            mv->specular = FTOL3(fog) << 24;
        }
    }
}

///////////////////////
// trans model verts //
///////////////////////

void TransModelVertsMirrorNewVerts(MODEL *m, MAT *mat, VEC *trans, MAT *worldmat, VEC *worldpos)
{
    short i;
    float z;
    float fog, mirrorfog;
    float nearclip = RenderSettings.NearClip;
    MODEL_VERTEX *mv;

    mv = m->VertPtr;

    for (i = 0 ; i < m->VertNum ; i++, mv++)
    {
        z = mv->x2 * mat->m[RZ] + mv->y2 * mat->m[UZ] + mv->z2 * mat->m[LZ] + trans->v[Z];
        mv->sx = (mv->x2 * mat->m[RX] + mv->y2 * mat->m[UX] + mv->z2 * mat->m[LX] + trans->v[X]) / z + RenderSettings.GeomCentreX;
        mv->sy = (mv->x2 * mat->m[RY] + mv->y2 * mat->m[UY] + mv->z2 * mat->m[LY] + trans->v[Y]) / z + RenderSettings.GeomCentreY;

        mv->rhw = 1 / z;
        mv->sz = GET_ZBUFFER(z);

        mv->Clip = 0;
        if (mv->sx < ScreenLeftClipGuard) mv->Clip |= CLIP_LEFT;
        else if (mv->sx > ScreenRightClipGuard) mv->Clip |= CLIP_RIGHT;
        if (mv->sy < ScreenTopClipGuard) mv->Clip |= CLIP_TOP;
        else if (mv->sy > ScreenBottomClipGuard) mv->Clip |= CLIP_BOTTOM;
//      if (mv->sz < 0) mv->Clip |= CLIP_NEAR;
        if (z < nearclip) mv->Clip |= CLIP_NEAR;
        else if (mv->sz >= 1) mv->Clip |= CLIP_FAR;

        mirrorfog = mv->x * worldmat->m[RY] + mv->y * worldmat->m[UY] + mv->z * worldmat->m[LY] + worldpos->v[Y];
        mirrorfog = GET_MIRROR_FOG(MirrorHeight - mirrorfog);
        if (mirrorfog < 0) mirrorfog = 0;

        if (mirrorfog >= 255)
        {
            mv->Clip |= CLIP_MAXFOG;
            mv->specular = 0;
        }
        else
        {
            fog = (RenderSettings.FarClip - z) * RenderSettings.FogMul;
            if (fog > 255) fog = 255;
            fog -= mirrorfog;
            if (fog < 0) fog = 0;
            mv->specular = FTOL3(fog) << 24;
        }
    }
}

/////////////////////////////////////
// set model verts for env mapping //
/////////////////////////////////////

void SetModelVertsEnvPlain(MODEL *m)
{
    short i;
    MODEL_VERTEX *mv;

    mv = m->VertPtr;

    for (i = 0 ; i < m->VertNum ; i++, mv++)
    {
        mv->tu = (mv->nx * EnvMatrix.m[RX] + mv->ny * EnvMatrix.m[UX] + mv->nz * EnvMatrix.m[LX]) + EnvXoffset;
        mv->tv = (mv->nx * EnvMatrix.m[RY] + mv->ny * EnvMatrix.m[UY] + mv->nz * EnvMatrix.m[LY]) + EnvYoffset;
        mv->color = *(long*)&EnvRgb;
        ModelChangeGouraud((MODEL_RGB*)&mv->color, EnvPer);
    } 
}

/////////////////////////////////////
// set model verts for env mapping //
/////////////////////////////////////

void SetModelVertsEnvLit(MODEL *m)
{
    short i;
    MODEL_VERTEX *mv;

    mv = m->VertPtr;

    for (i = 0 ; i < m->VertNum ; i++, mv++)
    {
        mv->tu = (mv->nx * EnvMatrix.m[RX] + mv->ny * EnvMatrix.m[UX] + mv->nz * EnvMatrix.m[LX]) + EnvXoffset;
        mv->tv = (mv->nx * EnvMatrix.m[RY] + mv->ny * EnvMatrix.m[UY] + mv->nz * EnvMatrix.m[LY]) + EnvYoffset;
        ModelAddGouraud(&EnvRgb, &mv->r, (MODEL_RGB*)&mv->color);
        ModelChangeGouraud((MODEL_RGB*)&mv->color, EnvPer);
    }
}

/////////////////////////////////////
// set model verts for env mapping //
/////////////////////////////////////

void SetModelVertsEnvGoodPlain(MODEL *m, VEC *campos, VEC *up)
{
    short i;
    MODEL_VERTEX *mv;
    VEC vecx, vecy, vecz;

    mv = m->VertPtr;

    for (i = 0 ; i < m->VertNum ; i++, mv++)
    {
        SubVector((VEC*)&mv->x, campos, &vecz);
        NormalizeVector(&vecz);
        CrossProduct(up, &vecz, &vecx)
        CrossProduct(&vecz, &vecx, &vecy);

        mv->tu = DotProduct((VEC*)&mv->nx, &vecx) * EnvScale + EnvXoffset;
        mv->tv = DotProduct((VEC*)&mv->nx, &vecy) * EnvScale + EnvYoffset;

        mv->color = *(long*)&EnvRgb;
        ModelChangeGouraud((MODEL_RGB*)&mv->color, EnvPer);
    } 
}

/////////////////////////////////////
// set model verts for env mapping //
/////////////////////////////////////

void SetModelVertsEnvGoodLit(MODEL *m, VEC *campos, VEC *up)
{
    short i;
    MODEL_VERTEX *mv;
    VEC vecx, vecy, vecz;

    mv = m->VertPtr;

    for (i = 0 ; i < m->VertNum ; i++, mv++)
    {
        SubVector((VEC*)&mv->x, campos, &vecz);
        NormalizeVector(&vecz);
        CrossProduct(up, &vecz, &vecx)
        CrossProduct(&vecz, &vecx, &vecy);

        mv->tu = DotProduct((VEC*)&mv->nx, &vecx) * EnvScale + EnvXoffset;
        mv->tv = DotProduct((VEC*)&mv->nx, &vecy) * EnvScale + EnvYoffset;

        ModelAddGouraud(&EnvRgb, &mv->r, (MODEL_RGB*)&mv->color);
        ModelChangeGouraud((MODEL_RGB*)&mv->color, EnvPer);
    }
}

/////////////////////////////////////
// set model verts for ghost model //
/////////////////////////////////////

void SetModelVertsGhost(MODEL *m)
{
    short i;
    MODEL_VERTEX *mv;
    MODEL_POLY *mp;
    POLY_RGB *mrgb;
    float fz, pos;

// set vert alpha

    pos = ModelGhostSineOffset - ModelGhostSinePos;
    mv = m->VertPtr;

    for (i = 0 ; i < m->VertNum ; i++, mv++)
    {
        fz = abs((mv->z + pos) * ModelGhostFadeMul);
        mv->a = -FTOL2(fz) + 255;
        if (mv->a < 0) mv->a = 0;
    }

// copy to polys

    mp = m->PolyPtr;
    mrgb = m->PolyRGB;

    for (i = 0 ; i < m->PolyNum ; i++, mp++, mrgb++)
    {
        mrgb->rgb[0].a = (unsigned char)mp->v0->a;
        mrgb->rgb[1].a = (unsigned char)mp->v1->a;
        mrgb->rgb[2].a = (unsigned char)mp->v2->a;
        if(mp->Type & POLY_QUAD)
        {
            mrgb->rgb[3].a = (unsigned char)mp->v3->a;
        }
    }
}

///////////////////////////
// set model verts glare //
///////////////////////////

void SetModelVertsGlare(MODEL *m, VEC *pos, MAT *mat, short flag)
{
    FACING_POLY poly;
    MAT spinmat1, spinmat2;
    REAL dot;
    VEC look, vec, vec2;
    long i, col;

// set spin mat

    RotMatrixZ(&spinmat1, TIME2MS(CurrentTimer()) / 5000.0f);
    RotMatrixZ(&spinmat2, TIME2MS(CurrentTimer()) / 2500.0f);

// set look vector

    TransposeRotVector(mat, &ViewCameraMatrix.mv[L], &look);
    NormalizeVector(&look);

// set facing poly

    poly.U = 192.0f / 256.0f;
    poly.V = 64.0f / 256.0f;
    poly.Usize = 64.0f / 256.0f;
    poly.Vsize = 64.0f / 256.0f;
    poly.Tpage = TPAGE_FX1;

// loop thru verts

    MODEL_POLY *mp;
    MODEL_VERTEX **mvp, *v0, *v1;
    long j, vnum;
    REAL time;

    mp = m->PolyPtr;
    for (i = 0 ; i < m->PolyNum ; i++, mp++) if (mp->Type & POLY_ENV)
    {
        mvp = &mp->v0;
        vnum = 3 + (mp->Type & 1);

        for (j = 0 ; j < vnum ; j++)
        {
            v0 = mvp[j];
            v1 = mvp[(j + 1) % vnum];

            SubVector((VEC*)&v1->x, (VEC*)&v0->x, &vec2);
            NormalizeVector(&vec2);
            dot = DotProduct(&look, &vec2);
            time = dot * 0.75f + 0.5f;
            if (time < 0.0f) continue;
            if (time > 1.0f) continue;

            vec2.v[X] = v0->nx + (v1->nx - v0->nx) * time;
            vec2.v[Y] = v0->ny + (v1->ny - v0->ny) * time;
            vec2.v[Z] = v0->nz + (v1->nz - v0->nz) * time;
            NormalizeVector(&vec2);

            dot = DotProduct(&look, (VEC*)&vec2);
            if (dot < -0.8f)
            {
                poly.Xsize = poly.Ysize = (-0.8f - dot) * 48.0f;

                FTOL((-0.8f - dot) * 5.0f * 255, col);
                poly.RGB = (col * EnvRgb.b / 255) | (col * EnvRgb.g / 255) << 8 | (col * EnvRgb.r / 255) << 16;

                if (flag & MODEL_USENEWVERTS)
                {
                    vec2.v[X] = v0->x2 + (v1->x2 - v0->x2) * time;
                    vec2.v[Y] = v0->y2 + (v1->y2 - v0->y2) * time;
                    vec2.v[Z] = v0->z2 + (v1->z2 - v0->z2) * time;
                }
                else
                {
                    vec2.v[X] = v0->x + (v1->x - v0->x) * time;
                    vec2.v[Y] = v0->y + (v1->y - v0->y) * time;
                    vec2.v[Z] = v0->z + (v1->z - v0->z) * time;
                }
                RotTransVector(mat, pos, &vec2, &vec);

                DrawFacingPolyRot(&vec, &spinmat1, &poly, 1, -24.0f);
                DrawFacingPolyRot(&vec, &spinmat2, &poly, 1, -24.0f);
            }
        }
    }
}

//////////////////////
// draw model polys //
//////////////////////

void DrawModelPolysNearClip(MODEL *m, long lit, long env)
{
    long i, clip;
    POLY_RGB *mrgb;
    MODEL_POLY *mp;
    VERTEX_TEX1 *vert;
    BUCKET_TEX1 *bucket;
    VERTEX_TEX0 *vertrgb;
    BUCKET_TEX0 *bucketrgb;
    BUCKET_ENV *envbucket;
    short count;

// add to poly count

    if (Version == VERSION_DEV)
    {
        ModelPolyCount += (m->QuadNumTex + m->QuadNumRGB) * 2;
        ModelPolyCount += (m->TriNumTex + m->TriNumRGB);
        if (env)
        {
            ModelPolyCount += (m->QuadNumTex + m->QuadNumRGB) * 2;
            ModelPolyCount += (m->TriNumTex + m->TriNumRGB);
        }
    }

// draw textured quads

    mrgb = m->PolyRGB;
    mp = m->PolyPtr;

    for (i = m->QuadNumTex ; i ; i--, mrgb++, mp++)
    {

// reject?

        REJECT_MODEL_POLY_3D();
        CLIP_QUAD_NEAR();

// get vert ptr

        if (mp->Type & POLY_SEMITRANS)
        {
            if (!SEMI_POLY_FREE()) continue;
            SEMI_POLY_SETUP(vert, ModelFog, 4, mp->Tpage, clip, (mp->Type & POLY_SEMITRANS_ONE) != 0);
        }
        else
        {
            if (clip) bucket = &ModelBucketHeadClip[mp->Tpage];
            else bucket = &ModelBucketHead[mp->Tpage];
            count = (short)(bucket->CurrentVerts - bucket->Verts);

            if (count > BUCKET_VERT_END)
            {
                SET_TPAGE(mp->Tpage);
                FlushOneBucketTEX1(bucket, clip);
                count = 0;
            }

            bucket->CurrentIndex[0] = count;
            bucket->CurrentIndex[1] = count + 1;
            bucket->CurrentIndex[2] = count + 2;
            bucket->CurrentIndex[3] = count;
            bucket->CurrentIndex[4] = count + 2;
            bucket->CurrentIndex[5] = count + 3;
            bucket->CurrentIndex += 6;

            vert = bucket->CurrentVerts;
            bucket->CurrentVerts += 4;
        }

// copy vert info

        COPY_QUAD_XYZRHW(vert);
        COPY_QUAD_UV(vert);

        if (ModelFog)
            COPY_QUAD_SPECULAR(vert);

        if (lit)
        {
            COPY_MODEL_QUAD_COLOR_LIT(vert);
        }
        else
        {
            COPY_MODEL_QUAD_COLOR(vert);
        }

// env?

        if (env)
        {
            REJECT_MODEL_ENV_POLY();

// get env vert ptr

            if (clip) envbucket = ModelBucketHeadEnvClip;
            else envbucket = ModelBucketHeadEnv;
            count = (short)(envbucket->CurrentVerts - envbucket->Verts);

            if (count > ENV_VERT_END)
                continue;

            envbucket->CurrentIndex[0] = count;
            envbucket->CurrentIndex[1] = count + 1;
            envbucket->CurrentIndex[2] = count + 2;
            envbucket->CurrentIndex[3] = count;
            envbucket->CurrentIndex[4] = count + 2;
            envbucket->CurrentIndex[5] = count + 3;
            envbucket->CurrentIndex += 6;

            vert = envbucket->CurrentVerts;
            envbucket->CurrentVerts += 4;

// copy env vert info

            *(MEM32*)&vert[0] = *(MEM32*)&mp->v0->sx;
            *(MEM32*)&vert[1] = *(MEM32*)&mp->v1->sx;
            *(MEM32*)&vert[2] = *(MEM32*)&mp->v2->sx;
            *(MEM32*)&vert[3] = *(MEM32*)&mp->v3->sx;
        }
    }

// draw textured tri's

    for (i = m->TriNumTex ; i ; i--, mrgb++, mp++)
    {

// reject?

        REJECT_MODEL_POLY_3D();
        CLIP_TRI_NEAR();

// get vert ptr

        if (mp->Type & POLY_SEMITRANS)
        {
            if (!SEMI_POLY_FREE()) continue;
            SEMI_POLY_SETUP(vert, ModelFog, 3, mp->Tpage, clip, (mp->Type & POLY_SEMITRANS_ONE) != 0);
        }
        else
        {
            if (clip) bucket = &ModelBucketHeadClip[mp->Tpage];
            else bucket = &ModelBucketHead[mp->Tpage];
            count = (short)(bucket->CurrentVerts - bucket->Verts);

            if (count > BUCKET_VERT_END)
            {
                SET_TPAGE(mp->Tpage);
                FlushOneBucketTEX1(bucket, clip);
                count = 0;
            }

            bucket->CurrentIndex[0] = count;
            bucket->CurrentIndex[1] = count + 1;
            bucket->CurrentIndex[2] = count + 2;
            bucket->CurrentIndex += 3;

            vert = bucket->CurrentVerts;
            bucket->CurrentVerts += 3;
        }

// copy vert info

        COPY_TRI_XYZRHW(vert);
        COPY_TRI_UV(vert);

        if (ModelFog)
            COPY_TRI_SPECULAR(vert);

        if (lit)
        {
            COPY_MODEL_TRI_COLOR_LIT(vert);
        }
        else
        {
            COPY_MODEL_TRI_COLOR(vert);
        }

// env?

        if (env)
        {
            REJECT_MODEL_ENV_POLY();

// get env vert ptr

            if (clip) envbucket = ModelBucketHeadEnvClip;
            else envbucket = ModelBucketHeadEnv;
            count = (short)(envbucket->CurrentVerts - envbucket->Verts);

            if (count > ENV_VERT_END)
                continue;

            envbucket->CurrentIndex[0] = count;
            envbucket->CurrentIndex[1] = count + 1;
            envbucket->CurrentIndex[2] = count + 2;
            envbucket->CurrentIndex += 3;

            vert = envbucket->CurrentVerts;
            envbucket->CurrentVerts += 3;

// copy env vert info

            *(MEM32*)&vert[0] = *(MEM32*)&mp->v0->sx;
            *(MEM32*)&vert[1] = *(MEM32*)&mp->v1->sx;
            *(MEM32*)&vert[2] = *(MEM32*)&mp->v2->sx;
        }
    }

// draw rgb quads

    for (i = m->QuadNumRGB ; i ; i--, mrgb++, mp++)
    {

// reject?

        REJECT_MODEL_POLY_3D();
        CLIP_QUAD_NEAR();

// get vert ptr

        if (mp->Type & POLY_SEMITRANS)
        {
            if (!SEMI_POLY_FREE()) continue;
            SEMI_POLY_SETUP_RGB(vertrgb, ModelFog, 4, clip, (mp->Type & POLY_SEMITRANS_ONE) != 0);
        }
        else
        {
            if (clip) bucketrgb = ModelBucketHeadClipRGB;
            else bucketrgb = ModelBucketHeadRGB;
            count = (short)(bucketrgb->CurrentVerts - bucketrgb->Verts);

            if (count > BUCKET_VERT_END)
            {
                SET_TPAGE(-1);
                FlushOneBucketTEX0(bucketrgb, clip);
                count = 0;
            }

            bucketrgb->CurrentIndex[0] = count;
            bucketrgb->CurrentIndex[1] = count + 1;
            bucketrgb->CurrentIndex[2] = count + 2;
            bucketrgb->CurrentIndex[3] = count;
            bucketrgb->CurrentIndex[4] = count + 2;
            bucketrgb->CurrentIndex[5] = count + 3;
            bucketrgb->CurrentIndex += 6;

            vertrgb = bucketrgb->CurrentVerts;
            bucketrgb->CurrentVerts += 4;
        }

// copy vert info

        COPY_QUAD_XYZRHW(vertrgb);

        if (ModelFog)
            COPY_QUAD_SPECULAR(vertrgb);

        if (lit)
        {
            COPY_MODEL_QUAD_COLOR_LIT(vertrgb);
        }
        else
        {
            COPY_MODEL_QUAD_COLOR(vertrgb);
        }

// env?

        if (env)
        {
            REJECT_MODEL_ENV_POLY();

// get env vert ptr

            if (clip) envbucket = ModelBucketHeadEnvClip;
            else envbucket = ModelBucketHeadEnv;
            count = (short)(envbucket->CurrentVerts - envbucket->Verts);

            if (count > ENV_VERT_END)
                continue;

            envbucket->CurrentIndex[0] = count;
            envbucket->CurrentIndex[1] = count + 1;
            envbucket->CurrentIndex[2] = count + 2;
            envbucket->CurrentIndex[3] = count;
            envbucket->CurrentIndex[4] = count + 2;
            envbucket->CurrentIndex[5] = count + 3;
            envbucket->CurrentIndex += 6;

            vert = envbucket->CurrentVerts;
            envbucket->CurrentVerts += 4;

// copy env vert info

            *(MEM32*)&vert[0] = *(MEM32*)&mp->v0->sx;
            *(MEM32*)&vert[1] = *(MEM32*)&mp->v1->sx;
            *(MEM32*)&vert[2] = *(MEM32*)&mp->v2->sx;
            *(MEM32*)&vert[3] = *(MEM32*)&mp->v3->sx;
        }
    }

// draw rgb tri's

    for (i = m->TriNumRGB ; i ; i--, mrgb++, mp++)
    {

// reject?

        REJECT_MODEL_POLY_3D();
        CLIP_TRI_NEAR();

// get vert ptr

        if (mp->Type & POLY_SEMITRANS)
        {
            if (!SEMI_POLY_FREE()) continue;
            SEMI_POLY_SETUP_RGB(vertrgb, ModelFog, 3, clip, (mp->Type & POLY_SEMITRANS_ONE) != 0);
        }
        else
        {
            if (clip) bucketrgb = ModelBucketHeadClipRGB;
            else bucketrgb = ModelBucketHeadRGB;
            count = (short)(bucketrgb->CurrentVerts - bucketrgb->Verts);

            if (count > BUCKET_VERT_END)
            {
                SET_TPAGE(-1);
                FlushOneBucketTEX0(bucketrgb, clip);
                count = 0;
            }

            bucketrgb->CurrentIndex[0] = count;
            bucketrgb->CurrentIndex[1] = count + 1;
            bucketrgb->CurrentIndex[2] = count + 2;
            bucketrgb->CurrentIndex += 3;

            vertrgb = bucketrgb->CurrentVerts;
            bucketrgb->CurrentVerts += 3;
        }

// copy vert info

        COPY_TRI_XYZRHW(vertrgb);

        if (ModelFog)
            COPY_TRI_SPECULAR(vertrgb);

        if (lit)
        {
            COPY_MODEL_TRI_COLOR_LIT(vertrgb);
        }
        else
        {
            COPY_MODEL_TRI_COLOR(vertrgb);
        }

// env?

        if (env)
        {
            REJECT_MODEL_ENV_POLY();

// get env vert ptr

            if (clip) envbucket = ModelBucketHeadEnvClip;
            else envbucket = ModelBucketHeadEnv;
            count = (short)(envbucket->CurrentVerts - envbucket->Verts);

            if (count > ENV_VERT_END)
                continue;

            envbucket->CurrentIndex[0] = count;
            envbucket->CurrentIndex[1] = count + 1;
            envbucket->CurrentIndex[2] = count + 2;
            envbucket->CurrentIndex += 3;

            vert = envbucket->CurrentVerts;
            envbucket->CurrentVerts += 3;

// copy env vert info

            *(MEM32*)&vert[0] = *(MEM32*)&mp->v0->sx;
            *(MEM32*)&vert[1] = *(MEM32*)&mp->v1->sx;
            *(MEM32*)&vert[2] = *(MEM32*)&mp->v2->sx;
        }
    }
}

//////////////////////
// draw model polys //
//////////////////////

void DrawModelPolysClip(MODEL *m, long lit, long env)
{
    long i, clip;
    POLY_RGB *mrgb;
    MODEL_POLY *mp;
    VERTEX_TEX1 *vert;
    BUCKET_TEX1 *bucket;
    VERTEX_TEX0 *vertrgb;
    BUCKET_TEX0 *bucketrgb;
    BUCKET_ENV *envbucket;
    short count;

// add to poly count

    if (Version == VERSION_DEV)
    {
        ModelPolyCount += (m->QuadNumTex + m->QuadNumRGB) * 2;
        ModelPolyCount += (m->TriNumTex + m->TriNumRGB);
        if (env)
        {
            ModelPolyCount += (m->QuadNumTex + m->QuadNumRGB) * 2;
            ModelPolyCount += (m->TriNumTex + m->TriNumRGB);
        }
    }

// draw textured quads

    mrgb = m->PolyRGB;
    mp = m->PolyPtr;

    for (i = m->QuadNumTex ; i ; i--, mrgb++, mp++)
    {

// reject?

        REJECT_MODEL_POLY();
        CLIP_QUAD();

// get vert ptr

        if (mp->Type & POLY_SEMITRANS)
        {
            if (!SEMI_POLY_FREE()) continue;
            SEMI_POLY_SETUP(vert, ModelFog, 4, mp->Tpage, clip, (mp->Type & POLY_SEMITRANS_ONE) != 0);
        }
        else
        {
            if (clip) bucket = &ModelBucketHeadClip[mp->Tpage];
            else bucket = &ModelBucketHead[mp->Tpage];
            count = (short)(bucket->CurrentVerts - bucket->Verts);

            if (count > BUCKET_VERT_END)
            {
                SET_TPAGE(mp->Tpage);
                FlushOneBucketTEX1(bucket, clip);
                count = 0;
            }

            bucket->CurrentIndex[0] = count;
            bucket->CurrentIndex[1] = count + 1;
            bucket->CurrentIndex[2] = count + 2;
            bucket->CurrentIndex[3] = count;
            bucket->CurrentIndex[4] = count + 2;
            bucket->CurrentIndex[5] = count + 3;
            bucket->CurrentIndex += 6;

            vert = bucket->CurrentVerts;
            bucket->CurrentVerts += 4;
        }

// copy vert info

        COPY_QUAD_XYZRHW(vert);
        COPY_QUAD_UV(vert);

        if (ModelFog)
            COPY_QUAD_SPECULAR(vert);

        if (lit)
        {
            COPY_MODEL_QUAD_COLOR_LIT(vert);
        }
        else
        {
            COPY_MODEL_QUAD_COLOR(vert);
        }

// env?

        if (env)
        {
            REJECT_MODEL_ENV_POLY();

// get env vert ptr

            if (clip) envbucket = ModelBucketHeadEnvClip;
            else envbucket = ModelBucketHeadEnv;
            count = (short)(envbucket->CurrentVerts - envbucket->Verts);

            if (count > ENV_VERT_END)
                continue;

            envbucket->CurrentIndex[0] = count;
            envbucket->CurrentIndex[1] = count + 1;
            envbucket->CurrentIndex[2] = count + 2;
            envbucket->CurrentIndex[3] = count;
            envbucket->CurrentIndex[4] = count + 2;
            envbucket->CurrentIndex[5] = count + 3;
            envbucket->CurrentIndex += 6;

            vert = envbucket->CurrentVerts;
            envbucket->CurrentVerts += 4;

// copy env vert info

            *(MEM32*)&vert[0] = *(MEM32*)&mp->v0->sx;
            *(MEM32*)&vert[1] = *(MEM32*)&mp->v1->sx;
            *(MEM32*)&vert[2] = *(MEM32*)&mp->v2->sx;
            *(MEM32*)&vert[3] = *(MEM32*)&mp->v3->sx;
        }
    }

// draw textured tri's

    for (i = m->TriNumTex ; i ; i--, mrgb++, mp++)
    {

// reject?

        REJECT_MODEL_POLY();
        CLIP_TRI();

// get vert ptr

        if (mp->Type & POLY_SEMITRANS)
        {
            if (!SEMI_POLY_FREE()) continue;
            SEMI_POLY_SETUP(vert, ModelFog, 3, mp->Tpage, clip, (mp->Type & POLY_SEMITRANS_ONE) != 0);
        }
        else
        {
            if (clip) bucket = &ModelBucketHeadClip[mp->Tpage];
            else bucket = &ModelBucketHead[mp->Tpage];
            count = (short)(bucket->CurrentVerts - bucket->Verts);

            if (count > BUCKET_VERT_END)
            {
                SET_TPAGE(mp->Tpage);
                FlushOneBucketTEX1(bucket, clip);
                count = 0;
            }

            bucket->CurrentIndex[0] = count;
            bucket->CurrentIndex[1] = count + 1;
            bucket->CurrentIndex[2] = count + 2;
            bucket->CurrentIndex += 3;

            vert = bucket->CurrentVerts;
            bucket->CurrentVerts += 3;
        }

// copy vert info

        COPY_TRI_XYZRHW(vert);
        COPY_TRI_UV(vert);

        if (ModelFog)
            COPY_TRI_SPECULAR(vert);

        if (lit)
        {
            COPY_MODEL_TRI_COLOR_LIT(vert);
        }
        else
        {
            COPY_MODEL_TRI_COLOR(vert);
        }

// env?

        if (env)
        {
            REJECT_MODEL_ENV_POLY();

// get env vert ptr

            if (clip) envbucket = ModelBucketHeadEnvClip;
            else envbucket = ModelBucketHeadEnv;
            count = (short)(envbucket->CurrentVerts - envbucket->Verts);

            if (count > ENV_VERT_END)
                continue;

            envbucket->CurrentIndex[0] = count;
            envbucket->CurrentIndex[1] = count + 1;
            envbucket->CurrentIndex[2] = count + 2;
            envbucket->CurrentIndex += 3;

            vert = envbucket->CurrentVerts;
            envbucket->CurrentVerts += 3;

// copy env vert info

            *(MEM32*)&vert[0] = *(MEM32*)&mp->v0->sx;
            *(MEM32*)&vert[1] = *(MEM32*)&mp->v1->sx;
            *(MEM32*)&vert[2] = *(MEM32*)&mp->v2->sx;
        }
    }

// draw rgb quads

    for (i = m->QuadNumRGB ; i ; i--, mrgb++, mp++)
    {

// reject?

        REJECT_MODEL_POLY();
        CLIP_QUAD();

// get vert ptr

        if (mp->Type & POLY_SEMITRANS)
        {
            if (!SEMI_POLY_FREE()) continue;
            SEMI_POLY_SETUP_RGB(vertrgb, ModelFog, 4, clip, (mp->Type & POLY_SEMITRANS_ONE) != 0);
        }
        else
        {
            if (clip) bucketrgb = ModelBucketHeadClipRGB;
            else bucketrgb = ModelBucketHeadRGB;
            count = (short)(bucketrgb->CurrentVerts - bucketrgb->Verts);

            if (count > BUCKET_VERT_END)
            {
                SET_TPAGE(-1);
                FlushOneBucketTEX0(bucketrgb, clip);
                count = 0;
            }

            bucketrgb->CurrentIndex[0] = count;
            bucketrgb->CurrentIndex[1] = count + 1;
            bucketrgb->CurrentIndex[2] = count + 2;
            bucketrgb->CurrentIndex[3] = count;
            bucketrgb->CurrentIndex[4] = count + 2;
            bucketrgb->CurrentIndex[5] = count + 3;
            bucketrgb->CurrentIndex += 6;

            vertrgb = bucketrgb->CurrentVerts;
            bucketrgb->CurrentVerts += 4;
        }

// copy vert info

        COPY_QUAD_XYZRHW(vertrgb);

        if (ModelFog)
            COPY_QUAD_SPECULAR(vertrgb);

        if (lit)
        {
            COPY_MODEL_QUAD_COLOR_LIT(vertrgb);
        }
        else
        {
            COPY_MODEL_QUAD_COLOR(vertrgb);
        }

// env?

        if (env)
        {
            REJECT_MODEL_ENV_POLY();

// get env vert ptr

            if (clip) envbucket = ModelBucketHeadEnvClip;
            else envbucket = ModelBucketHeadEnv;
            count = (short)(envbucket->CurrentVerts - envbucket->Verts);

            if (count > ENV_VERT_END)
                continue;

            envbucket->CurrentIndex[0] = count;
            envbucket->CurrentIndex[1] = count + 1;
            envbucket->CurrentIndex[2] = count + 2;
            envbucket->CurrentIndex[3] = count;
            envbucket->CurrentIndex[4] = count + 2;
            envbucket->CurrentIndex[5] = count + 3;
            envbucket->CurrentIndex += 6;

            vert = envbucket->CurrentVerts;
            envbucket->CurrentVerts += 4;

// copy env vert info

            *(MEM32*)&vert[0] = *(MEM32*)&mp->v0->sx;
            *(MEM32*)&vert[1] = *(MEM32*)&mp->v1->sx;
            *(MEM32*)&vert[2] = *(MEM32*)&mp->v2->sx;
            *(MEM32*)&vert[3] = *(MEM32*)&mp->v3->sx;
        }
    }

// draw rgb tri's

    for (i = m->TriNumRGB ; i ; i--, mrgb++, mp++)
    {

// reject?

        REJECT_MODEL_POLY();
        CLIP_TRI();

// get vert ptr

        if (mp->Type & POLY_SEMITRANS)
        {
            if (!SEMI_POLY_FREE()) continue;
            SEMI_POLY_SETUP_RGB(vertrgb, ModelFog, 3, clip, (mp->Type & POLY_SEMITRANS_ONE) != 0);
        }
        else
        {
            if (clip) bucketrgb = ModelBucketHeadClipRGB;
            else bucketrgb = ModelBucketHeadRGB;
            count = (short)(bucketrgb->CurrentVerts - bucketrgb->Verts);

            if (count > BUCKET_VERT_END)
            {
                SET_TPAGE(-1);
                FlushOneBucketTEX0(bucketrgb, clip);
                count = 0;
            }

            bucketrgb->CurrentIndex[0] = count;
            bucketrgb->CurrentIndex[1] = count + 1;
            bucketrgb->CurrentIndex[2] = count + 2;
            bucketrgb->CurrentIndex += 3;

            vertrgb = bucketrgb->CurrentVerts;
            bucketrgb->CurrentVerts += 3;
        }

// copy vert info

        COPY_TRI_XYZRHW(vertrgb);

        if (ModelFog)
            COPY_TRI_SPECULAR(vertrgb);

        if (lit)
        {
            COPY_MODEL_TRI_COLOR_LIT(vertrgb);
        }
        else
        {
            COPY_MODEL_TRI_COLOR(vertrgb);
        }

// env?

        if (env)
        {
            REJECT_MODEL_ENV_POLY();

// get env vert ptr

            if (clip) envbucket = ModelBucketHeadEnvClip;
            else envbucket = ModelBucketHeadEnv;
            count = (short)(envbucket->CurrentVerts - envbucket->Verts);

            if (count > ENV_VERT_END)
                continue;

            envbucket->CurrentIndex[0] = count;
            envbucket->CurrentIndex[1] = count + 1;
            envbucket->CurrentIndex[2] = count + 2;
            envbucket->CurrentIndex += 3;

            vert = envbucket->CurrentVerts;
            envbucket->CurrentVerts += 3;

// copy env vert info

            *(MEM32*)&vert[0] = *(MEM32*)&mp->v0->sx;
            *(MEM32*)&vert[1] = *(MEM32*)&mp->v1->sx;
            *(MEM32*)&vert[2] = *(MEM32*)&mp->v2->sx;
        }
    }
}

//////////////////////
// draw model polys //
//////////////////////

void DrawModelPolys(MODEL *m, long lit, long env)
{
    long i;
    POLY_RGB *mrgb;
    MODEL_POLY *mp;
    VERTEX_TEX1 *vert;
    BUCKET_TEX1 *bucket;
    VERTEX_TEX0 *vertrgb;
    BUCKET_TEX0 *bucketrgb;
    BUCKET_ENV *envbucket = ModelBucketHeadEnv;
    short count;

// add to poly count

    if (Version == VERSION_DEV)
    {
        ModelPolyCount += (m->QuadNumTex + m->QuadNumRGB) * 2;
        ModelPolyCount += (m->TriNumTex + m->TriNumRGB);
        if (env)
        {
            ModelPolyCount += (m->QuadNumTex + m->QuadNumRGB) * 2;
            ModelPolyCount += (m->TriNumTex + m->TriNumRGB);
        }
    }

// draw textured quads

    mrgb = m->PolyRGB;
    mp = m->PolyPtr;

    for (i = m->QuadNumTex ; i ; i--, mrgb++, mp++)
    {

// reject?

        REJECT_MODEL_POLY();

// get vert ptr

        if (mp->Type & POLY_SEMITRANS)
        {
            if (!SEMI_POLY_FREE()) continue;
            SEMI_POLY_SETUP(vert, ModelFog, 4, mp->Tpage, FALSE, (mp->Type & POLY_SEMITRANS_ONE) != 0);
        }
        else
        {
            bucket = &ModelBucketHead[mp->Tpage];
            count = (short)(bucket->CurrentVerts - bucket->Verts);

            if (count > BUCKET_VERT_END)
            {
                SET_TPAGE(mp->Tpage);
                FlushOneBucketTEX1(bucket, FALSE);
                count = 0;
            }

            bucket->CurrentIndex[0] = count;
            bucket->CurrentIndex[1] = count + 1;
            bucket->CurrentIndex[2] = count + 2;
            bucket->CurrentIndex[3] = count;
            bucket->CurrentIndex[4] = count + 2;
            bucket->CurrentIndex[5] = count + 3;
            bucket->CurrentIndex += 6;

            vert = bucket->CurrentVerts;
            bucket->CurrentVerts += 4;
        }

// copy vert info

        COPY_QUAD_XYZRHW(vert);
        COPY_QUAD_UV(vert);

        if (ModelFog)
            COPY_QUAD_SPECULAR(vert);

        if (lit)
        {
            COPY_MODEL_QUAD_COLOR_LIT(vert);
        }
        else
        {
            COPY_MODEL_QUAD_COLOR(vert);
        }

// env?

        if (env)
        {
            REJECT_MODEL_ENV_POLY();

// get env vert ptr

            count = (short)(envbucket->CurrentVerts - envbucket->Verts);

            if (count > ENV_VERT_END)
                continue;

            envbucket->CurrentIndex[0] = count;
            envbucket->CurrentIndex[1] = count + 1;
            envbucket->CurrentIndex[2] = count + 2;
            envbucket->CurrentIndex[3] = count;
            envbucket->CurrentIndex[4] = count + 2;
            envbucket->CurrentIndex[5] = count + 3;
            envbucket->CurrentIndex += 6;

            vert = envbucket->CurrentVerts;
            envbucket->CurrentVerts += 4;

// copy env vert info

            *(MEM32*)&vert[0] = *(MEM32*)&mp->v0->sx;
            *(MEM32*)&vert[1] = *(MEM32*)&mp->v1->sx;
            *(MEM32*)&vert[2] = *(MEM32*)&mp->v2->sx;
            *(MEM32*)&vert[3] = *(MEM32*)&mp->v3->sx;
        }
    }

// draw textured tri's

    for (i = m->TriNumTex ; i ; i--, mrgb++, mp++)
    {

// reject?

        REJECT_MODEL_POLY();

// get vert ptr

        if (mp->Type & POLY_SEMITRANS)
        {
            if (!SEMI_POLY_FREE()) continue;
            SEMI_POLY_SETUP(vert, ModelFog, 3, mp->Tpage, FALSE, (mp->Type & POLY_SEMITRANS_ONE) != 0);
        }
        else
        {
            bucket = &ModelBucketHead[mp->Tpage];
            count = (short)(bucket->CurrentVerts - bucket->Verts);

            if (count > BUCKET_VERT_END)
            {
                SET_TPAGE(mp->Tpage);
                FlushOneBucketTEX1(bucket, FALSE);
                count = 0;
            }

            bucket->CurrentIndex[0] = count;
            bucket->CurrentIndex[1] = count + 1;
            bucket->CurrentIndex[2] = count + 2;
            bucket->CurrentIndex += 3;

            vert = bucket->CurrentVerts;
            bucket->CurrentVerts += 3;
        }

// copy vert info

        COPY_TRI_XYZRHW(vert);
        COPY_TRI_UV(vert);

        if (ModelFog)
            COPY_TRI_SPECULAR(vert);

        if (lit)
        {
            COPY_MODEL_TRI_COLOR_LIT(vert);
        }
        else
        {
            COPY_MODEL_TRI_COLOR(vert);
        }

// env?

        if (env)
        {
            REJECT_MODEL_ENV_POLY();

// get env vert ptr

            count = (short)(envbucket->CurrentVerts - envbucket->Verts);

            if (count > ENV_VERT_END)
                continue;

            envbucket->CurrentIndex[0] = count;
            envbucket->CurrentIndex[1] = count + 1;
            envbucket->CurrentIndex[2] = count + 2;
            envbucket->CurrentIndex += 3;

            vert = envbucket->CurrentVerts;
            envbucket->CurrentVerts += 3;

// copy env vert info

            *(MEM32*)&vert[0] = *(MEM32*)&mp->v0->sx;
            *(MEM32*)&vert[1] = *(MEM32*)&mp->v1->sx;
            *(MEM32*)&vert[2] = *(MEM32*)&mp->v2->sx;
        }
    }

// draw rgb quads

    for (i = m->QuadNumRGB ; i ; i--, mrgb++, mp++)
    {

// reject?

        REJECT_MODEL_POLY();

// get vert ptr

        if (mp->Type & POLY_SEMITRANS)
        {
            if (!SEMI_POLY_FREE()) continue;
            SEMI_POLY_SETUP_RGB(vertrgb, ModelFog, 4, FALSE, (mp->Type & POLY_SEMITRANS_ONE) != 0);
        }
        else
        {
            bucketrgb = ModelBucketHeadRGB;
            count = (short)(bucketrgb->CurrentVerts - bucketrgb->Verts);

            if (count > BUCKET_VERT_END)
            {
                SET_TPAGE(-1);
                FlushOneBucketTEX0(bucketrgb, FALSE);
                count = 0;
            }

            bucketrgb->CurrentIndex[0] = count;
            bucketrgb->CurrentIndex[1] = count + 1;
            bucketrgb->CurrentIndex[2] = count + 2;
            bucketrgb->CurrentIndex[3] = count;
            bucketrgb->CurrentIndex[4] = count + 2;
            bucketrgb->CurrentIndex[5] = count + 3;
            bucketrgb->CurrentIndex += 6;

            vertrgb = bucketrgb->CurrentVerts;
            bucketrgb->CurrentVerts += 4;
        }

// copy vert info

        COPY_QUAD_XYZRHW(vertrgb);

        if (ModelFog)
            COPY_QUAD_SPECULAR(vertrgb);

        if (lit)
        {
            COPY_MODEL_QUAD_COLOR_LIT(vertrgb);
        }
        else
        {
            COPY_MODEL_QUAD_COLOR(vertrgb);
        }

// env?

        if (env)
        {
            REJECT_MODEL_ENV_POLY();

// get env vert ptr

            count = (short)(envbucket->CurrentVerts - envbucket->Verts);

            if (count > ENV_VERT_END)
                continue;

            envbucket->CurrentIndex[0] = count;
            envbucket->CurrentIndex[1] = count + 1;
            envbucket->CurrentIndex[2] = count + 2;
            envbucket->CurrentIndex[3] = count;
            envbucket->CurrentIndex[4] = count + 2;
            envbucket->CurrentIndex[5] = count + 3;
            envbucket->CurrentIndex += 6;

            vert = envbucket->CurrentVerts;
            envbucket->CurrentVerts += 4;

// copy env vert info

            *(MEM32*)&vert[0] = *(MEM32*)&mp->v0->sx;
            *(MEM32*)&vert[1] = *(MEM32*)&mp->v1->sx;
            *(MEM32*)&vert[2] = *(MEM32*)&mp->v2->sx;
            *(MEM32*)&vert[3] = *(MEM32*)&mp->v3->sx;
        }
    }

// draw rgb tri's

    for (i = m->TriNumRGB ; i ; i--, mrgb++, mp++)
    {

// reject?

        REJECT_MODEL_POLY();

// get vert ptr

        if (mp->Type & POLY_SEMITRANS)
        {
            if (!SEMI_POLY_FREE()) continue;
            SEMI_POLY_SETUP_RGB(vertrgb, ModelFog, 3, FALSE, (mp->Type & POLY_SEMITRANS_ONE) != 0);
        }
        else
        {
            bucketrgb = ModelBucketHeadRGB;
            count = (short)(bucketrgb->CurrentVerts - bucketrgb->Verts);

            if (count > BUCKET_VERT_END)
            {
                SET_TPAGE(-1);
                FlushOneBucketTEX0(bucketrgb, FALSE);
                count = 0;
            }

            bucketrgb->CurrentIndex[0] = count;
            bucketrgb->CurrentIndex[1] = count + 1;
            bucketrgb->CurrentIndex[2] = count + 2;
            bucketrgb->CurrentIndex += 3;

            vertrgb = bucketrgb->CurrentVerts;
            bucketrgb->CurrentVerts += 3;
        }

// copy vert info

        COPY_TRI_XYZRHW(vertrgb);

        if (ModelFog)
            COPY_TRI_SPECULAR(vertrgb);

        if (lit)
        {
            COPY_MODEL_TRI_COLOR_LIT(vertrgb);
        }
        else
        {
            COPY_MODEL_TRI_COLOR(vertrgb);
        }

// env?

        if (env)
        {
            REJECT_MODEL_ENV_POLY();
            INC_POLY_COUNT(ModelDrawnCount, 1);

// get env vert ptr

            count = (short)(envbucket->CurrentVerts - envbucket->Verts);

            if (count > ENV_VERT_END)
                continue;

            envbucket->CurrentIndex[0] = count;
            envbucket->CurrentIndex[1] = count + 1;
            envbucket->CurrentIndex[2] = count + 2;
            envbucket->CurrentIndex += 3;

            vert = envbucket->CurrentVerts;
            envbucket->CurrentVerts += 3;

// copy env vert info

            *(MEM32*)&vert[0] = *(MEM32*)&mp->v0->sx;
            *(MEM32*)&vert[1] = *(MEM32*)&mp->v1->sx;
            *(MEM32*)&vert[2] = *(MEM32*)&mp->v2->sx;
        }
    }
}

//////////////////////
// draw model polys //
//////////////////////

void DrawModelPolysMirror(MODEL *m, long lit)
{
    long i, clip;
    POLY_RGB *mrgb;
    MODEL_POLY *mp;
    VERTEX_TEX1 *vert;
    BUCKET_TEX1 *bucket;
    VERTEX_TEX0 *vertrgb;
    BUCKET_TEX0 *bucketrgb;
    short count;

// add to poly count

    if (Version == VERSION_DEV)
    {
        ModelPolyCount += (m->QuadNumTex + m->QuadNumRGB) * 2;
        ModelPolyCount += (m->TriNumTex + m->TriNumRGB);
    }

// draw textured quads

    mrgb = m->PolyRGB;
    mp = m->PolyPtr;

    for (i = m->QuadNumTex ; i ; i--, mrgb++, mp++)
    {

// reject?

        REJECT_MODEL_POLY_MIRROR();
        CLIP_QUAD_MIRROR();

// get vert ptr

        if (mp->Type & POLY_SEMITRANS)
        {
            if (!SEMI_POLY_FREE()) continue;
            SEMI_POLY_SETUP(vert, ModelFog, 4, mp->Tpage, clip, (mp->Type & POLY_SEMITRANS_ONE) != 0);
        }
        else
        {
            if (clip) bucket = &ModelBucketHeadClip[mp->Tpage];
            else bucket = &ModelBucketHead[mp->Tpage];
            count = (short)(bucket->CurrentVerts - bucket->Verts);

            if (count > BUCKET_VERT_END)
            {
                SET_TPAGE(mp->Tpage);
                FlushOneBucketTEX1(bucket, clip);
                count = 0;
            }

            bucket->CurrentIndex[0] = count;
            bucket->CurrentIndex[1] = count + 1;
            bucket->CurrentIndex[2] = count + 2;
            bucket->CurrentIndex[3] = count;
            bucket->CurrentIndex[4] = count + 2;
            bucket->CurrentIndex[5] = count + 3;
            bucket->CurrentIndex += 6;

            vert = bucket->CurrentVerts;
            bucket->CurrentVerts += 4;
        }

// copy vert info

        COPY_QUAD_XYZRHW(vert);
        COPY_QUAD_UV(vert);
        COPY_QUAD_SPECULAR(vert);

        if (lit)
        {
            COPY_MODEL_QUAD_COLOR_LIT(vert);
        }
        else
        {
            COPY_MODEL_QUAD_COLOR(vert);
        }
    }

// draw textured tri's

    for (i = m->TriNumTex ; i ; i--, mrgb++, mp++)
    {

// reject?

        REJECT_MODEL_POLY_MIRROR();
        CLIP_TRI_MIRROR();

// get vert ptr

        if (mp->Type & POLY_SEMITRANS)
        {
            if (!SEMI_POLY_FREE()) continue;
            SEMI_POLY_SETUP(vert, ModelFog, 3, mp->Tpage, clip, (mp->Type & POLY_SEMITRANS_ONE) != 0);
        }
        else
        {
            if (clip) bucket = &ModelBucketHeadClip[mp->Tpage];
            else bucket = &ModelBucketHead[mp->Tpage];
            count = (short)(bucket->CurrentVerts - bucket->Verts);

            if (count > BUCKET_VERT_END)
            {
                SET_TPAGE(mp->Tpage);
                FlushOneBucketTEX1(bucket, clip);
                count = 0;
            }

            bucket->CurrentIndex[0] = count;
            bucket->CurrentIndex[1] = count + 1;
            bucket->CurrentIndex[2] = count + 2;
            bucket->CurrentIndex += 3;

            vert = bucket->CurrentVerts;
            bucket->CurrentVerts += 3;
        }

// copy vert info

        COPY_TRI_XYZRHW(vert);
        COPY_TRI_UV(vert);
        COPY_TRI_SPECULAR(vert);

        if (lit)
        {
            COPY_MODEL_TRI_COLOR_LIT(vert);
        }
        else
        {
            COPY_MODEL_TRI_COLOR(vert);
        }
    }

// draw rgb quads

    for (i = m->QuadNumRGB ; i ; i--, mrgb++, mp++)
    {

// reject?

        REJECT_MODEL_POLY_MIRROR();
        CLIP_QUAD_MIRROR();

// get vert ptr

        if (mp->Type & POLY_SEMITRANS)
        {
            if (!SEMI_POLY_FREE()) continue;
            SEMI_POLY_SETUP_RGB(vertrgb, ModelFog, 4, clip, (mp->Type & POLY_SEMITRANS_ONE) != 0);
        }
        else
        {
            if (clip) bucketrgb = ModelBucketHeadClipRGB;
            else bucketrgb = ModelBucketHeadRGB;
            count = (short)(bucketrgb->CurrentVerts - bucketrgb->Verts);

            if (count > BUCKET_VERT_END)
            {
                SET_TPAGE(-1);
                FlushOneBucketTEX0(bucketrgb, clip);
                count = 0;
            }

            bucketrgb->CurrentIndex[0] = count;
            bucketrgb->CurrentIndex[1] = count + 1;
            bucketrgb->CurrentIndex[2] = count + 2;
            bucketrgb->CurrentIndex[3] = count;
            bucketrgb->CurrentIndex[4] = count + 2;
            bucketrgb->CurrentIndex[5] = count + 3;
            bucketrgb->CurrentIndex += 6;

            vertrgb = bucketrgb->CurrentVerts;
            bucketrgb->CurrentVerts += 4;
        }

// copy vert info

        COPY_QUAD_XYZRHW(vertrgb);
        COPY_QUAD_SPECULAR(vertrgb);

        if (lit)
        {
            COPY_MODEL_QUAD_COLOR_LIT(vertrgb);
        }
        else
        {
            COPY_MODEL_QUAD_COLOR(vertrgb);
        }
    }

// draw rgb tri's

    for (i = m->TriNumRGB ; i ; i--, mrgb++, mp++)
    {

// reject?

        REJECT_MODEL_POLY_MIRROR();
        CLIP_TRI_MIRROR();

// get vert ptr

        if (mp->Type & POLY_SEMITRANS)
        {
            if (!SEMI_POLY_FREE()) continue;
            SEMI_POLY_SETUP_RGB(vertrgb, ModelFog, 3, clip, (mp->Type & POLY_SEMITRANS_ONE) != 0);
        }
        else
        {
            if (clip) bucketrgb = ModelBucketHeadClipRGB;
            else bucketrgb = ModelBucketHeadRGB;
            count = (short)(bucketrgb->CurrentVerts - bucketrgb->Verts);

            if (count > BUCKET_VERT_END)
            {
                SET_TPAGE(-1);
                FlushOneBucketTEX0(bucketrgb, clip);
                count = 0;
            }

            bucketrgb->CurrentIndex[0] = count;
            bucketrgb->CurrentIndex[1] = count + 1;
            bucketrgb->CurrentIndex[2] = count + 2;
            bucketrgb->CurrentIndex += 3;

            vertrgb = bucketrgb->CurrentVerts;
            bucketrgb->CurrentVerts += 3;
        }

// copy vert info

        COPY_TRI_XYZRHW(vertrgb);
        COPY_TRI_SPECULAR(vertrgb);

        if (lit)
        {
            COPY_MODEL_TRI_COLOR_LIT(vertrgb);
        }
        else
        {
            COPY_MODEL_TRI_COLOR(vertrgb);
        }
    }
}

////////////////////////////////
// setup environment map vars //
////////////////////////////////

void SetEnvStatic(VEC *pos, MAT *mat, long rgb, float xoff, float yoff, float scale)
{
    MAT m, m2;
    float mul;

// set rgb

    *(long*)&EnvRgb = rgb;
    if (RenderSettings.Sepia)
    {
        Grayscale((long*)&EnvRgb);
    }

// quit if env mapping off

    if (!RenderSettings.Env)
        return;

// build camera to object 'look' matrix

    SubVector(pos, &ViewCameraPos, &m.mv[L]);
    NormalizeVector(&m.mv[L]);
    CrossProduct(&m.mv[L], &ViewCameraMatrix.mv[R], &m.mv[U]);
    CrossProduct(&m.mv[U], &m.mv[L], &m.mv[R]);

// transpose for 'view' matrix

    TransposeMatrix(&m, &m2);

// mul with obj matrix for 'env' matrix

    MulMatrix(&m2, mat, &EnvMatrix);

// scale 'env' matrix

    mul = 0.5f * scale;
    EnvMatrix.m[RX] *= mul;
    EnvMatrix.m[UX] *= mul;
    EnvMatrix.m[LX] *= mul;
    EnvMatrix.m[RY] *= mul;
    EnvMatrix.m[UY] *= mul;
    EnvMatrix.m[LY] *= mul;

// set tpage

    EnvTpage = TPAGE_ENVSTILL;

// set xy offsets

    EnvXoffset = xoff + 0.5f;
    EnvYoffset = yoff + 0.5f;
}

////////////////////////////////
// setup environment map vars //
////////////////////////////////

void SetEnvActive(VEC *pos, MAT *mat, MAT *envmat, long rgb, float xoff, float yoff, float scale)
{
    float mul;
    MAT m1, m2;

// set rgb

    *(long*)&EnvRgb = rgb;
    if (RenderSettings.Sepia)
    {
        Grayscale((long*)&EnvRgb);
    }

// quit if env mapping off

    if (!RenderSettings.Env)
        return;

// rotate envmat by 90 degrees on the Y (why???)

    RotMatrixY(&m1, 0.25f);
    MulMatrix(envmat, &m1, &m2);

// mul with object matrix for 'env' matrix

    MulMatrix(&m2, mat, &EnvMatrix);

// scale 'env' matrix

    mul = 0.5f * scale;
    EnvMatrix.m[RX] *= mul;
    EnvMatrix.m[UX] *= mul;
    EnvMatrix.m[LX] *= mul;
    EnvMatrix.m[RY] *= mul;
    EnvMatrix.m[UY] *= mul;
    EnvMatrix.m[LY] *= mul;

// set tpage

    EnvTpage = TPAGE_ENVROLL;

// set xy offsets

    EnvXoffset = xoff + 0.5f;
    EnvYoffset = yoff + 0.5f;
}

////////////////////////////////
// setup environment map vars //
////////////////////////////////

void SetEnvGood(long rgb, float xoff, float yoff, float scale)
{

// set rgb

    *(long*)&EnvRgb = rgb;
    if (RenderSettings.Sepia)
    {
        Grayscale((long*)&EnvRgb);
    }

// set scale

    EnvScale = scale * 0.5f;

// set tpage

    EnvTpage = TPAGE_ENVSTILL;

// set offsets

    EnvXoffset = xoff + 0.5f;
    EnvYoffset = yoff + 0.5f;
}

/////////////////////////////
// init level model system //
/////////////////////////////

void InitLevelModels(void)
{
    long i;

    for (i = 0 ; i < MAX_LEVEL_MODELS ; i++)
    {
        LevelModel[i].ID = -1;
    }
}

///////////////////////////
// free all level models //
///////////////////////////

void FreeLevelModels(void)
{
    long i;

    for (i = 0 ; i < MAX_LEVEL_MODELS ; i++)
    {
        if (LevelModel[i].ID != -1)
        {
            LevelModel[i].RefCount = 1;
            FreeOneLevelModel(i);
        }
    }
}

//////////////////////////
// load one level model //
//////////////////////////

//$REVISIT: function doesn't actually use 'renderflag' param, so should probably remove it eventually...
long LoadOneLevelModel(long id, long flag, struct renderflags renderflag, long tpage)
{
    long i, rgbper;
    char buf[128];
    FILE *fp;

// look for existing model

    for (i = 0 ; i < MAX_LEVEL_MODELS ; i++)
    {
        if (LevelModel[i].ID == id)
        {
            LevelModel[i].RefCount++;
            return i;
        }
    }
// find new slot

    for (i = 0 ; i < MAX_LEVEL_MODELS ; i++)
    {
        if (LevelModel[i].ID == -1)
        {

// load model

            sprintf(buf, "%s.m", LevelModelList[id]);

            if (flag)
                rgbper = CurrentLevelInfo.ModelRGBper;
            else
                rgbper = 100;

            if (!LoadModel(buf, &LevelModel[i].Model, (char)tpage, 1, LOADMODEL_OFFSET_TPAGE, rgbper))
                return -1;
    
//$ADDITION(jedl) - first pass at resource loading
            // get resource pointers from world resource bundle
            if (World.m_pXBR != NULL)
			{
                LoadModelGPU(buf, &LevelModel[i].Model, 1, World.m_pXBR);
			}
#ifdef SHIPPING
// Shipping version does not need exporter code.
#else
            else
			{
				extern HRESULT ExportLevelModel(INT LevelModelIndex, CHAR *file, MODEL *m, INT ModelCount);
				ExportLevelModel(i, buf, &LevelModel[i].Model, 1);
			}
#endif
//$END_ADDITION
    
// load coll skin

            LevelModel[i].CollSkin.Convex = NULL;
            LevelModel[i].CollSkin.Sphere = NULL;
            LevelModel[i].CollSkin.CollPoly = NULL;
            sprintf(buf, "%s.hul", LevelModelList[id]);
            if ((fp = fopen(buf, "rb")) != NULL) 
            {
                if ((LevelModel[i].CollSkin.Convex = LoadConvex(fp, &LevelModel[i].CollSkin.NConvex)) != NULL)
                {
                    if ((LevelModel[i].CollSkin.Sphere = LoadSpheres(fp, &LevelModel[i].CollSkin.NSpheres)) != NULL) 
                    {
                        LevelModel[i].CollSkin.CollType = BODY_COLL_CONVEX;
                        MakeTightLocalBBox(&LevelModel[i].CollSkin);
                    }
                }
                fclose(fp);
            }
            else 
            {
                sprintf(buf, "%s.ncp", LevelModelList[id]);
                if ((fp = fopen(buf, "rb")) != NULL) 
                {
                    LevelModel[i].CollSkin.CollPoly = LoadNewCollPolys(fp, &LevelModel[i].CollSkin.NCollPolys);
                    fclose(fp);
                }
            }



// set ID / ref count

            LevelModel[i].ID = id;
            LevelModel[i].RefCount = 1;
            return i;
        }
    }

// slots full

    return -1;
}

//////////////////////////
// free one level model //
//////////////////////////

void FreeOneLevelModel(long slot)
{

// skip if empty

    if (LevelModel[slot].ID == -1)
        return;

// dec ref count

    LevelModel[slot].RefCount--;

// free model + coll if no owners

    if (LevelModel[slot].RefCount < 1)
    {
        FreeModel(&LevelModel[slot].Model, 1);
        DestroyConvex(LevelModel[slot].CollSkin.Convex, LevelModel[slot].CollSkin.NConvex);
        LevelModel[slot].CollSkin.Convex = NULL;
        LevelModel[slot].CollSkin.NConvex = 0;
        DestroySpheres(LevelModel[slot].CollSkin.Sphere);
        LevelModel[slot].CollSkin.Sphere = NULL;
        LevelModel[slot].CollSkin.NSpheres = 0;
        LevelModel[slot].ID = -1;
        DestroyCollPolys(LevelModel[slot].CollSkin.CollPoly);
        LevelModel[slot].CollSkin.CollPoly = NULL;
        LevelModel[slot].CollSkin.NCollPolys = 0;
    }
}

///////////////////////////////
// set a models morph frames //
///////////////////////////////

void SetModelFrames(MODEL *model, char **files, long count)
{
    long i, j;
    FILE *fp;
    MODEL_HEADER mh;
    MODEL_POLY_LOAD mpl;
    MODEL_VERTEX_LOAD mvl;
    MODEL_VERTEX_MORPH *mvm;
    char buf[128];

// alloc morph ram

    model->VertPtrMorph = (MODEL_VERTEX_MORPH*)malloc(sizeof(MODEL_VERTEX_MORPH) * model->VertNum * count);
    if (!model->VertPtrMorph)
        return;

// load in each frame

    for (i = 0 ; i < count ; i++)
    {
        fp = fopen(files[i], "rb");
        if (fp == NULL)
        {
            sprintf(buf, "Can't load morph frame %s", files[i]);
            DumpMessage("ERROR", buf);
            continue;
        }

// read header

        fread(&mh, sizeof(mh), 1, fp);

// skip polys

        for (j = 0 ; j < mh.PolyNum ; j++)
            fread(&mpl, sizeof(mpl), 1, fp);

// load verts

        mvm = &model->VertPtrMorph[model->VertNum * i];

        for (j = 0 ; j < model->VertNum ; j++)
        {
            fread(&mvl, sizeof(mvl), 1, fp);

            mvm[j].x = mvl.x;
            mvm[j].y = mvl.y;
            mvm[j].z = mvl.z;

            mvm[j].nx = mvl.nx;
            mvm[j].ny = mvl.ny;
            mvm[j].nz = mvl.nz;
        }

// close file

        fclose(fp);
    }
}

/////////////////////////////
// set a model frame morph //
/////////////////////////////

void SetModelMorph(MODEL *m, long frame1, long frame2, float time)
{
    long i;
    MODEL_VERTEX *mv;
    MODEL_VERTEX_MORPH *v1, *v2;

// quit if no morph frames

    if (!m->VertPtrMorph)
        return;

// loop thru each vert

    v1 = &m->VertPtrMorph[m->VertNum * frame1];
    v2 = &m->VertPtrMorph[m->VertNum * frame2];
    mv= m->VertPtr;

    if (time == 0.0f)
    {
        for (i = m->VertNum ; i ; i--, v1++, v2++, mv++)
        {
            mv->x = v1->x;
            mv->y = v1->y;
            mv->z = v1->z;

            mv->nx = v1->nx;
            mv->ny = v1->ny;
            mv->nz = v1->nz;
        }
    }

    else
    {
        for (i = m->VertNum ; i ; i--, v1++, v2++, mv++)
        {
            mv->x = v1->x + (v2->x - v1->x) * time;
            mv->y = v1->y + (v2->y - v1->y) * time;
            mv->z = v1->z + (v2->z - v1->z) * time;

            mv->nx = v1->nx + (v2->nx - v1->nx) * time;
            mv->ny = v1->ny + (v2->ny - v1->ny) * time;
            mv->nz = v1->nz + (v2->nz - v1->nz) * time;

            NormalizeVector((VEC*)&mv->nx);
        }
    }
}

/////////////////////////
// check model mesh fx //
/////////////////////////

void CheckModelMeshFx(MODEL *model, MAT *mat, VEC *pos, short *flag)
{
    long i;

// set checker params

    ModelMeshModel = model;
    ModelMeshMat = mat;
    ModelMeshPos = pos;
    ModelMeshFlag = flag;

// loop thru all

    for (i = 0 ; i < ModelMeshFxCount ; i++)
        ModelMeshFx[i].Checker(ModelMeshFx[i].Data);
}

//////////////////
// copy a model //
//////////////////

long CopyModel(MODEL *src, MODEL *dest)
{
    long i, size, offset;

// copy src contents to dest

    memcpy(dest, src, sizeof(MODEL));

// alloc mem for mesh data

    size = sizeof(POLY_RGB) * dest->PolyNum;
    size += sizeof(MODEL_POLY) * dest->PolyNum;
    size += sizeof(MODEL_VERTEX) * dest->VertNum;

    dest->AllocPtr = malloc(size);
    if (dest->AllocPtr == NULL)
    {
        DumpMessage(NULL,"Can't alloc memory for copy model!");
        return FALSE;
    }

    dest->PolyRGB = (POLY_RGB*)(dest->AllocPtr);
    dest->PolyPtr = (MODEL_POLY*)(dest->PolyRGB + dest->PolyNum);
    dest->VertPtr = (MODEL_VERTEX*)(dest->PolyPtr + dest->PolyNum);
    dest->VertPtrMorph = NULL;

// copy all mesh data from src to dest

    memcpy(dest->AllocPtr, src->AllocPtr, size);

// fix vert indices in polys

    offset = (long)dest->VertPtr - (long)src->VertPtr;

    for (i = 0 ; i < dest->PolyNum ; i++)
    {
        dest->PolyPtr[i].v0 = (MODEL_VERTEX*)((long)dest->PolyPtr[i].v0 + offset);
        dest->PolyPtr[i].v1 = (MODEL_VERTEX*)((long)dest->PolyPtr[i].v1 + offset);
        dest->PolyPtr[i].v2 = (MODEL_VERTEX*)((long)dest->PolyPtr[i].v2 + offset);
        dest->PolyPtr[i].v3 = (MODEL_VERTEX*)((long)dest->PolyPtr[i].v3 + offset);
    }

// return OK

    return TRUE;
}
