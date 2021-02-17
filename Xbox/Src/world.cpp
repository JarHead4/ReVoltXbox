//-----------------------------------------------------------------------------
// File: world.cpp
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"
#include "NewColl.h"
#include "world.h"
#include "geom.h"
#include "dx.h"
#include "light.h"
#include "main.h"
#include "draw.h"
#include "camera.h"
#include "input.h"
#include "visibox.h"
#include "LevelLoad.h"
#include "mirror.h"
#include "settings.h"
#include "panel.h"

// globals

WORLD World;
long Wireframe = FALSE;
short WorldBigCubeCount, WorldCubeCount, WorldPolyCount;
long TexAnimNum;
TEXANIM_HEADER TexAnim[MAX_TEXANIMS];

static short WorldFog;
static BUCKET_TEX0 *WorldBucketHeadRGB, *WorldBucketHeadClipRGB;
static BUCKET_TEX1 *WorldBucketHead, *WorldBucketHeadClip;
static BUCKET_ENV *WorldBucketHeadEnv, *WorldBucketHeadEnvClip;
static short WorldEnvMask;

// shell sort gaps

static long ShellGap[] = {13, 4, 1};



//$ADDITION(jedl) - first pass at world exporting / reloading as compiled resources

//-----------------------------------------------------------------------------
// Name: LoadWorldGPU
// Desc: Loads a world from an XBResource.
//-----------------------------------------------------------------------------
HRESULT LoadWorldGPU(char *file)
{
    World.m_pXBR = NULL;
    World.m_pDefaultWorldCubeMaterial = NULL;
    
    // Check to see if there's a compiled version of the world
    CHAR path_xbr[_MAX_PATH];
    CHAR drive[_MAX_DRIVE];
    CHAR dir[_MAX_DIR];
    CHAR name[_MAX_PATH];
    CHAR ext[_MAX_EXT];
    _splitpath( file, drive, dir, name, ext );
    _makepath( path_xbr, drive, dir, name, ".xbr");
    DWORD FileAttrib = GetFileAttributes( path_xbr );
    if (FileAttrib == 0xffffffff)
        return E_FAIL;

    World.m_pXBR = new XBResource;
    Assert(World.m_pXBR != NULL);
    
    // Start loading resources from file
    World.m_pXBR->StartLoading( path_xbr );

    // Busy-wait.  This should instead be overlapped with the car
    // selection or screens displaying network play state, etc.
    LOADINGSTATE LoadingState = World.m_pXBR->CurrentLoadingState();
    while ( LoadingState != LOADING_DONE )
    {
        Assert(LoadingState != LOADING_FAILED );
        World.m_pXBR->PollLoadingState();
        LoadingState = World.m_pXBR->CurrentLoadingState();
    }

    // Get the resources of interest.
    World.m_pDefaultWorldCubeMaterial = World.m_pXBR->GetEffect( "DefaultWorldCubeMaterial" );

    // $TODO: move all of the stuff that's still in LoadWorld to here
    return S_OK;
}
//$END_ADDITION



//////////////////
// load a world //
//////////////////

bool LoadWorld(char *file)
{
    FILE *fp;
    WORLD_HEADER wh;
    CUBE_HEADER_LOAD chl;
    BIG_CUBE_HEADER_LOAD bchl;
    WORLD_POLY *mp;
    WORLD_POLY_LOAD mpl;
    WORLD_VERTEX_LOAD mvl;
    WORLD_POLY wp;
    WORLD_VERTEX **vert, **vert2;
    long size, i, j, k, l, idx, a, b, rgb;
    char buf[128];
    float vf, rad, maxrad;

// open file for reading

    fp = fopen(file, "rb");
    if (!fp)
    {
        sprintf(buf, "Can't load world file: '%s'", file);
        DumpMessage("ERROR", buf);
        return FALSE;
    }

// get header info, alloc cube header memory

    fread(&wh, sizeof(wh), 1, fp);
    World.CubeNum = wh.CubeNum;
    World.Cube = (CUBE_HEADER*)malloc(World.CubeNum * sizeof(CUBE_HEADER));
    if (!World.Cube)
    {
        DumpMessage("ERROR", "Can't alloc memory for world cubes!");
        QuitGame();
        return FALSE;
    }

// loop thru each cube

    maxrad = 0;
    OutOfBoundsBox.XMin = OutOfBoundsBox.YMin = OutOfBoundsBox.ZMin = FLT_MAX;
    OutOfBoundsBox.XMax = OutOfBoundsBox.YMax = OutOfBoundsBox.ZMax = -FLT_MAX;

    for (i = 0 ; i < World.CubeNum ; i++)
    {

// read load header

        fread(&chl, sizeof(chl), 1, fp);

// update out of bounds box

        AddPosRadToBBox(&OutOfBoundsBox, (VEC*)&chl.CentreX, chl.Radius + Real(20000));

// setup header

        World.Cube[i].CentreY = chl.CentreY;
        World.Cube[i].CentreX = chl.CentreX;
        World.Cube[i].CentreZ = chl.CentreZ;
        World.Cube[i].Radius = chl.Radius;
        World.Cube[i].Xmin = chl.Xmin;
        World.Cube[i].Xmax = chl.Xmax;
        World.Cube[i].Ymin = chl.Ymin;
        World.Cube[i].Ymax = chl.Ymax;
        World.Cube[i].Zmin = chl.Zmin;
        World.Cube[i].Zmax = chl.Zmax;

        World.Cube[i].Model.PolyNum = chl.PolyNum;
        World.Cube[i].Model.VertNum = chl.VertNum;

// alloc memory for polys / verts

        size = sizeof(WORLD_POLY) * World.Cube[i].Model.PolyNum;
        size += sizeof(WORLD_VERTEX) * World.Cube[i].Model.VertNum;

        World.Cube[i].Model.AllocPtr = malloc(size);
        if (World.Cube[i].Model.AllocPtr == NULL)
        {
            DumpMessage("ERROR", "Can't alloc memory for world mesh!");
            QuitGame();
            return FALSE;
        }
        World.Cube[i].Model.PolyPtr = (WORLD_POLY*)World.Cube[i].Model.AllocPtr;
        World.Cube[i].Model.VertPtr = (WORLD_VERTEX*)(World.Cube[i].Model.PolyPtr + World.Cube[i].Model.PolyNum);

// load polys - count textured / rgb + quads / tris's

        mp = World.Cube[i].Model.PolyPtr;

        World.Cube[i].Model.QuadNumTex = 0;
        World.Cube[i].Model.TriNumTex = 0;
        World.Cube[i].Model.QuadNumRGB = 0;
        World.Cube[i].Model.TriNumRGB = 0;

        for (j = 0 ; j < World.Cube[i].Model.PolyNum ; j++, mp++)
        {
            fread(&mpl, sizeof(mpl), 1, fp);

            mp->Type = mpl.Type;
            mp->Tpage = mpl.Tpage;

            if (GameSettings.Mirrored)
            {
                if (mp->Type & POLY_QUAD)
                {
                    mp->rgb0 = mpl.c3;
                    mp->rgb1 = mpl.c2;
                    mp->rgb2 = mpl.c1;
                    mp->rgb3 = mpl.c0;

                    mp->tu0 = mpl.u3;
                    mp->tv0 = mpl.v3;
                    mp->tu1 = mpl.u2;
                    mp->tv1 = mpl.v2;
                    mp->tu2 = mpl.u1;
                    mp->tv2 = mpl.v1;
                    mp->tu3 = mpl.u0;
                    mp->tv3 = mpl.v0;

                    mp->v0 = World.Cube[i].Model.VertPtr + mpl.vi3;
                    mp->v1 = World.Cube[i].Model.VertPtr + mpl.vi2;
                    mp->v2 = World.Cube[i].Model.VertPtr + mpl.vi1;
                    mp->v3 = World.Cube[i].Model.VertPtr + mpl.vi0;
                }
                else
                {
                    mp->rgb0 = mpl.c2;
                    mp->rgb1 = mpl.c1;
                    mp->rgb2 = mpl.c0;

                    mp->tu0 = mpl.u2;
                    mp->tv0 = mpl.v2;
                    mp->tu1 = mpl.u1;
                    mp->tv1 = mpl.v1;
                    mp->tu2 = mpl.u0;
                    mp->tv2 = mpl.v0;

                    mp->v0 = World.Cube[i].Model.VertPtr + mpl.vi2;
                    mp->v1 = World.Cube[i].Model.VertPtr + mpl.vi1;
                    mp->v2 = World.Cube[i].Model.VertPtr + mpl.vi0;
                }
            }
            else
            {
                mp->rgb0 = mpl.c0;
                mp->rgb1 = mpl.c1;
                mp->rgb2 = mpl.c2;
                mp->rgb3 = mpl.c3;

                mp->tu0 = mpl.u0;
                mp->tv0 = mpl.v0;
                mp->tu1 = mpl.u1;
                mp->tv1 = mpl.v1;
                mp->tu2 = mpl.u2;
                mp->tv2 = mpl.v2;
                mp->tu3 = mpl.u3;
                mp->tv3 = mpl.v3;

                mp->v0 = World.Cube[i].Model.VertPtr + mpl.vi0;
                mp->v1 = World.Cube[i].Model.VertPtr + mpl.vi1;
                mp->v2 = World.Cube[i].Model.VertPtr + mpl.vi2;
                mp->v3 = World.Cube[i].Model.VertPtr + mpl.vi3;
            }

            ModelChangeGouraud((MODEL_RGB*)&mp->rgb0, CurrentLevelInfo.WorldRGBper);
            ModelChangeGouraud((MODEL_RGB*)&mp->rgb1, CurrentLevelInfo.WorldRGBper);
            ModelChangeGouraud((MODEL_RGB*)&mp->rgb2, CurrentLevelInfo.WorldRGBper);
            ModelChangeGouraud((MODEL_RGB*)&mp->rgb3, CurrentLevelInfo.WorldRGBper);

            if (RenderSettings.Sepia)
            {
                Grayscale(&mp->rgb0);
                Grayscale(&mp->rgb1);
                Grayscale(&mp->rgb2);
                Grayscale(&mp->rgb3);
            }

            if (mp->Type & POLY_MIRROR)
            {
                mp->rgb0 |= MirrorAlpha;
                mp->rgb1 |= MirrorAlpha;
                mp->rgb2 |= MirrorAlpha;
                mp->rgb3 |= MirrorAlpha;
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
                if (mp->Type & POLY_QUAD) World.Cube[i].Model.QuadNumTex++;
                else World.Cube[i].Model.TriNumTex++;
            }
            else
            {
                if (mp->Type & POLY_QUAD) World.Cube[i].Model.QuadNumRGB++;
                else World.Cube[i].Model.TriNumRGB++;
            }
        }

// sort polys into textured / untextured + quads / tri's

        mp = World.Cube[i].Model.PolyPtr;

        j = World.Cube[i].Model.PolyNum;
        if (j > 1)  //can only sort a list with more than one entry
        {
            while(--j)  //pre-decrement because we only need to scan j - 1 times
            {
                for (k = 0 ; k < j ; k++)
                {
                    a = mp[k].Type & POLY_QUAD;
                    if (mp[k].Tpage != -1) a += 256;

                    b = mp[k + 1].Type & POLY_QUAD;
                    if (mp[k + 1].Tpage != -1) b += 256;

                    if (b > a)
                    {
                        wp = mp[k];
                        mp[k] = mp[k + 1];
                        mp[k + 1] = wp;
                    }
                }
            }
        }

// load verts

        for (j = 0 ; j < World.Cube[i].Model.VertNum ; j++)
        {
            fread(&mvl, sizeof(mvl), 1, fp);

            World.Cube[i].Model.VertPtr[j].x = mvl.x;
            World.Cube[i].Model.VertPtr[j].y = mvl.y;
            World.Cube[i].Model.VertPtr[j].z = mvl.z;

            World.Cube[i].Model.VertPtr[j].nx = mvl.nx;
            World.Cube[i].Model.VertPtr[j].ny = mvl.ny;
            World.Cube[i].Model.VertPtr[j].nz = mvl.nz;

            vf = (mvl.y - RenderSettings.VertFogStart) * RenderSettings.VertFogMul;
            vf -= (float)(rand() & 31);
            if (vf < 0) vf = 0;
            if (vf > 255) vf = 255;
            (World.Cube[i].Model.VertPtr + j)->VertFog = vf;

            rad = (float)sqrt(mvl.x * mvl.x + mvl.y * mvl.y + mvl.z * mvl.z);
            if (rad > maxrad) maxrad = rad;
        }

// build poly planes

        mp = World.Cube[i].Model.PolyPtr;
        for (j = 0 ; j < World.Cube[i].Model.PolyNum ; j++, mp++)
        {
            if (GameSettings.Mirrored)
                BuildPlane((VEC*)&mp->v2->x, (VEC*)&mp->v1->x, (VEC*)&mp->v0->x, &mp->Plane);
            else
                BuildPlane((VEC*)&mp->v0->x, (VEC*)&mp->v1->x, (VEC*)&mp->v2->x, &mp->Plane);
        }

// setup tex anim poly list

        World.Cube[i].Model.AnimPolyNum = 0;
        mp = World.Cube[i].Model.PolyPtr;

        for (j = World.Cube[i].Model.PolyNum ; j ; j--, mp++) if (mp->Type & POLY_TEXANIM)
        {
            World.Cube[i].Model.AnimPolyNum++;
        }

        if (!World.Cube[i].Model.AnimPolyNum)
        {
            World.Cube[i].Model.AnimPolyPtr = NULL;
        }
        else
        {
            World.Cube[i].Model.AnimPolyPtr = (WORLD_ANIM_POLY*)malloc(sizeof(WORLD_ANIM_POLY) * World.Cube[i].Model.AnimPolyNum);
            if (!World.Cube[i].Model.AnimPolyPtr) World.Cube[i].Model.AnimPolyNum = 0;
        }

        if (World.Cube[i].Model.AnimPolyNum)
        {
            mp = World.Cube[i].Model.PolyPtr;
            for (j = 0 ; j < World.Cube[i].Model.AnimPolyNum ; j++)
            {
                while (!(mp->Type & POLY_TEXANIM)) mp++;
                World.Cube[i].Model.AnimPolyPtr[j].Poly = mp;
                World.Cube[i].Model.AnimPolyPtr[j].Anim = &TexAnim[mp->Tpage];
                mp++;
            }
        }

// setup env vert list

        World.Cube[i].Model.EnvVertNum = 0;
        World.Cube[i].Model.EnvVertPtr = NULL;

        vert = (WORLD_VERTEX**)malloc(sizeof(WORLD_VERTEX*) * World.Cube[i].Model.VertNum);
        if (vert)
        {
            mp = World.Cube[i].Model.PolyPtr;

            for (j = 0 ; j < World.Cube[i].Model.PolyNum ; j++) if (mp[j].Type & POLY_ENV)
            {
                vert2 = &mp[j].v0;
                for (k = 0 ; k < 3 + (mp[j].Type & 1) ; k++)
                {
                    for (l = 0 ; l < World.Cube[i].Model.EnvVertNum ; l++)
                    {
                        if (vert[l] == vert2[k])
                            break;
                    }
                    if (l == World.Cube[i].Model.EnvVertNum)
                    {
                        vert[l] = vert2[k];
                        World.Cube[i].Model.EnvVertNum++;
                    }
                }
            }

            if (World.Cube[i].Model.EnvVertNum)
            {
                World.Cube[i].Model.EnvVertPtr = (WORLD_VERTEX**)malloc(sizeof(WORLD_VERTEX*) * World.Cube[i].Model.EnvVertNum);
                if (!World.Cube[i].Model.EnvVertPtr) World.Cube[i].Model.EnvVertNum = 0;
                else memcpy(World.Cube[i].Model.EnvVertPtr, vert, sizeof(WORLD_VERTEX*) * World.Cube[i].Model.EnvVertNum);
            }

            free(vert);
        }
    }

// PSX warning

/*  if (maxrad > 32767)
    {
        sprintf(buf, "'%s' is not PSX friendly by %ld pixels!", file, (long)maxrad - 32767);
        DumpMessage("Warning!", buf);
    }*/

// get big cube header, alloc big cube header memory

    if (!fread(&wh, sizeof(wh), 1, fp))
    {
        DumpMessage("ERROR", "World file has no big cube info!");
        QuitGame();
        return FALSE;
    }

    World.BigCubeNum = wh.CubeNum;
    World.BigCube = (BIG_CUBE_HEADER*)malloc(World.BigCubeNum * sizeof(BIG_CUBE_HEADER));
    if (!World.BigCube)
    {
        DumpMessage("ERROR", "Can't alloc memory for world big cubes!");
        QuitGame();
        return FALSE;
    }

// loop thru each big cube

    for (i = 0 ; i < World.BigCubeNum ; i++)
    {

// setup header

        fread(&bchl, sizeof(bchl), 1, fp);

        World.BigCube[i].x = bchl.x;
        World.BigCube[i].y = bchl.y;
        World.BigCube[i].z = bchl.z;
        World.BigCube[i].Radius = bchl.Radius;
        World.BigCube[i].CubeNum = bchl.CubeNum;

// alloc memory for CUBE_HEADER pointers

        World.BigCube[i].Cubes = (CUBE_HEADER**)malloc(sizeof(CUBE_HEADER*) * World.BigCube[i].CubeNum);
        if (!World.BigCube[i].Cubes)
        {
            DumpMessage("ERROR", "Can't alloc memory for a cube list!");
            return FALSE;
        }

// setup CUBE_HEADER pointers

        for (j = 0 ; j < World.BigCube[i].CubeNum ; j++)
        {
            fread(&idx, sizeof(idx), 1, fp);
            World.BigCube[i].Cubes[j] = &World.Cube[idx];
        }
    }

// alloc sort space

    World.CubeList = (CUBE_HEADER**)malloc(sizeof(CUBE_HEADER*) * World.CubeNum);
    if (!World.CubeList)
    {
        DumpMessage(NULL, "Can't alloc memory for world cube list!");
        QuitGame();
        return FALSE;
    }

// get texture anim num

    if (!fread(&TexAnimNum, sizeof(TexAnimNum), 1, fp))
    {
        TexAnimNum = 0;
    }

// load texture anims

    for (i = 0 ; i < TexAnimNum ; i++)
    {
        fread(&TexAnim[i].FrameNum, sizeof(TexAnim[i].FrameNum), 1, fp);
        TexAnim[i].Frame = (TEXANIM_FRAME*)malloc(sizeof(TEXANIM_FRAME) * TexAnim[i].FrameNum);
        if (!TexAnim[i].Frame)
        {
            DumpMessage(NULL, "Can't alloc memory for texture animation");
            TexAnimNum = 0;
            break;
        }

        fread(TexAnim[i].Frame, sizeof(TEXANIM_FRAME), TexAnim[i].FrameNum, fp);

        TexAnim[i].FrameTime = 0;
        TexAnim[i].CurrentFrameNum = 0;
        TexAnim[i].CurrentFrame = TexAnim[i].Frame;

        if (GameSettings.Mirrored)
        {
            for (j = 0 ; j < TexAnim[i].FrameNum ; j++)
            {
                float f;

                f = TexAnim[i].Frame[j].u0;
                TexAnim[i].Frame[j].u0 = TexAnim[i].Frame[j].u3;
                TexAnim[i].Frame[j].u3 = f;

                f = TexAnim[i].Frame[j].u1;
                TexAnim[i].Frame[j].u1 = TexAnim[i].Frame[j].u2;
                TexAnim[i].Frame[j].u2 = f;

                f = TexAnim[i].Frame[j].v0;
                TexAnim[i].Frame[j].v0 = TexAnim[i].Frame[j].v3;
                TexAnim[i].Frame[j].v3 = f;

                f = TexAnim[i].Frame[j].v1;
                TexAnim[i].Frame[j].v1 = TexAnim[i].Frame[j].v2;
                TexAnim[i].Frame[j].v2 = f;
            }
        }
    }

// load env rgb's

    for (i = 0 ; i < World.CubeNum ; i++)
    {
        mp = World.Cube[i].Model.PolyPtr;
        for (j = 0 ; j < World.Cube[i].Model.PolyNum ; j++, mp++) if (mp->Type & POLY_ENV)
        {
            fread(&rgb, sizeof(long), 1, fp);

            mp->v0->EnvRGB = rgb;
            mp->v1->EnvRGB = rgb;
            mp->v2->EnvRGB = rgb;

            if (mp->Type & POLY_QUAD)
                mp->v3->EnvRGB = rgb;
        }
    }

// close file

    fclose(fp);

//$BEGIN_ADDITION(jedl) - packed resources for world cubes
    if( World.m_pXBR != NULL )
    {
        // $TODO: move this up to LoadWorldGPU
        
        // Get rid of all the malloc'ing and freeing above.  Use
        // user-data resources instead, so that none of the parsing is
        // done at load time.
        World.BigCubeList = (BIG_CUBE_HEADER**)malloc(sizeof(BIG_CUBE_HEADER *) * World.BigCubeNum);
        if (World.BigCubeList == NULL)
        {
            DumpMessage(NULL, "Can't alloc memory for world big-cube list!");
            QuitGame();
            return FALSE;
        }
        
        // Get per-big-cube effect pointers
        for (i = 0; i < World.BigCubeNum; i++)
        {
            BIG_CUBE_HEADER *pBigCube = &World.BigCube[i];
            CHAR buf[EFFECT_IDENTIFIER_SIZE];

            // Opaque cube model
            _snprintf(buf, EFFECT_IDENTIFIER_SIZE, "WorldCube%d_opaque", i);
            buf[EFFECT_IDENTIFIER_SIZE - 1] = 0;
            pBigCube->m_pModelOpaque = World.m_pXBR->GetEffect( buf );

            // Alpha cube model
            _snprintf(buf, EFFECT_IDENTIFIER_SIZE, "WorldCube%d_alpha", i);
            buf[EFFECT_IDENTIFIER_SIZE - 1] = 0;
            pBigCube->m_pModelAlpha = World.m_pXBR->GetEffect( buf );
        }
    }
//$END_ADDITION
    
// return OK

    return TRUE;
}

//////////////////
// free a world //
//////////////////

void FreeWorld(void)
{
    long i;

    for (i = 0 ; i < World.CubeNum ; i++)
    {
        free(World.Cube[i].Model.AllocPtr);
        free(World.Cube[i].Model.MirrorPolyPtr);
        free(World.Cube[i].Model.AnimPolyPtr);
        free(World.Cube[i].Model.EnvVertPtr);
    }

    for (i = 0 ; i < World.BigCubeNum ; i++)
        free(World.BigCube[i].Cubes);

    free(World.Cube);
    free(World.BigCube);
    free(World.CubeList);

    World.Cube = NULL;
    World.BigCube = NULL;
    World.CubeList = NULL;
    World.BigCubeNum = 0;
    World.CubeNum = 0;


    for (i = 0 ; i < TexAnimNum ; i++)
        free(TexAnim[i].Frame);

//$ADDITION(jedl) - packed resources
    if( World.m_pXBR != NULL )
    {
        free(World.BigCubeList);
        
        // Make sure none of the textures are being used
        D3DDevice_SetTexture(0, 0);
        D3DDevice_SetTexture(1, 0);
        D3DDevice_SetTexture(2, 0);
        D3DDevice_SetTexture(3, 0);

        // Unload the resources
        delete World.m_pXBR;
        World.m_pXBR = NULL;
    }
//$END_ADDITION
}

////////////////////////
// mirror world polys //
////////////////////////

void MirrorWorldPolys(void)
{
    short mpolynum, mvertnum;
    long i, j, k, l, vnum, flag, size, offset;
    CUBE_HEADER *cube;
    WORLD_POLY *p;
    WORLD_VERTEX **v;
    WORLD_MIRROR_POLY *mpolys;
    WORLD_MIRROR_VERTEX *mverts, **mv;
    MIRROR_PLANE *mplane, *plane;

// loop thru all world cubes

    cube = World.Cube;
    for (i = 0 ; i < World.CubeNum ; i++, cube++)
    {

// zero misc

        cube->Model.MirrorPolyNum = 0;
        cube->Model.MirrorVertNum = 0;

        cube->Model.MirrorQuadNumTex = 0;
        cube->Model.MirrorTriNumTex = 0;
        cube->Model.MirrorQuadNumRGB = 0;
        cube->Model.MirrorTriNumRGB = 0;

        cube->Model.MirrorPolyPtr = NULL;

        cube->MirrorHeight = -99999;

// skip if no planes

        if (!MirrorPlaneNum)
            continue;

// alloc max polys + verts

        mpolys = (WORLD_MIRROR_POLY*)malloc(sizeof(WORLD_MIRROR_POLY) * cube->Model.PolyNum);
        if (!mpolys)
        {
            continue;
        }
        mverts = (WORLD_MIRROR_VERTEX*)malloc(sizeof(WORLD_MIRROR_VERTEX) * cube->Model.VertNum);
        if (!mverts)
        {
            free(mpolys);
            continue;
        }

        mpolynum = 0;
        mvertnum = 0;

// loop thru cube polys

        p = cube->Model.PolyPtr;
        for (j = 0 ; j < cube->Model.PolyNum ; j++, p++)
        {

// skip if a 'mirror' poly

            if (p->Type & POLY_MIRROR) continue;

// loop thru poly verts checking for a valid reflection against each mirror plane

            v = &p->v0;
            vnum = p->Type & POLY_QUAD ? 4 : 3;
            plane = NULL;
            mplane = MirrorPlanes;

            for (l = 0 ; l < MirrorPlaneNum ; l++, mplane++)
            {
                for (k = 0 ; k < vnum ; k++)
                {
                    if (v[k]->y < mplane->Height + MIRROR_OVERLAP_TOL && v[k]->y > mplane->Height - MirrorDist &&
                        v[k]->x >= mplane->Xmin && v[k]->x <= mplane->Xmax &&
                        v[k]->z >= mplane->Zmin && v[k]->z <= mplane->Zmax)
                    {
                        plane = mplane;
                        break;
                    }
                }
                if (plane) break;
            }

// check lowest vert if got a reflection

            if (plane)
            {
                for (k = 0 ; k < vnum ; k++)
                {
                    if (v[k]->y >= plane->Height + MIRROR_OVERLAP_TOL)
                    {
                        plane = NULL;
                        break;
                    }
                }
            }

// add mirror poly if any verts sucessfully mirrored

            if (plane)
            {

// lowest mirror plane for this cube?

                if (plane->Height > cube->MirrorHeight)
                    cube->MirrorHeight = plane->Height;

// loop thru each vertex

                mv = &mpolys[mpolynum].v0;
                for (k = 0 ; k < vnum ; k++)
                {

// look for existing mirrored vertex match

                    flag = FALSE;
                    for (l = 0 ; l < mvertnum ; l++)
                    {
                        if (v[k] == mverts[l].RealVertex)
                        {
                            mv[k] = &mverts[l];
                            flag = TRUE;
                            break;
                        }
                    }

// no match, create new mirrored vertex

                    if (!flag)
                    {
                        mverts[mvertnum].x = v[k]->x;
                        mverts[mvertnum].y = plane->Height + (plane->Height - v[k]->y);
                        mverts[mvertnum].z = v[k]->z;
                        mverts[mvertnum].VertFog = GET_MIRROR_FOG(mverts[mvertnum].y - plane->Height);
                        mverts[mvertnum].RealVertex = v[k];
                        mv[k] = &mverts[mvertnum];
                        mvertnum++;
                    }
                }

// create mirrored poly

                mpolys[mpolynum].Type = p->Type;
                mpolys[mpolynum].Tpage = p->Tpage;
                mpolys[mpolynum].VisiMask = p->VisiMask;
                mpolys[mpolynum].rgb0 = p->rgb0;
                mpolys[mpolynum].rgb1 = p->rgb1;
                mpolys[mpolynum].rgb2 = p->rgb2;
                mpolys[mpolynum].rgb3 = p->rgb3;
                mpolys[mpolynum].tu0 = p->tu0;
                mpolys[mpolynum].tv0 = p->tv0;
                mpolys[mpolynum].tu1 = p->tu1;
                mpolys[mpolynum].tv1 = p->tv1;
                mpolys[mpolynum].tu2 = p->tu2;
                mpolys[mpolynum].tv2 = p->tv2;
                mpolys[mpolynum].tu3 = p->tu3;
                mpolys[mpolynum].tv3 = p->tv3;

                if (GameSettings.Mirrored)
                    BuildPlane((VEC*)&mpolys[mpolynum].v0->x, (VEC*)&mpolys[mpolynum].v1->x, (VEC*)&mpolys[mpolynum].v2->x, &mpolys[mpolynum].Plane);
                else
                    BuildPlane((VEC*)&mpolys[mpolynum].v2->x, (VEC*)&mpolys[mpolynum].v1->x, (VEC*)&mpolys[mpolynum].v0->x, &mpolys[mpolynum].Plane);

                mpolynum++;
            }
        }

// alloc + copy mirrored verts + polys to real position

        if (mpolynum)
        {
            size = sizeof(WORLD_MIRROR_POLY) * mpolynum;
            size += sizeof(WORLD_MIRROR_VERTEX) * mvertnum;
            cube->Model.MirrorPolyPtr = (WORLD_MIRROR_POLY*)malloc(size);

            if (cube->Model.MirrorPolyPtr)
            {
                cube->Model.MirrorVertPtr = (WORLD_MIRROR_VERTEX*)(cube->Model.MirrorPolyPtr + mpolynum);
                cube->Model.MirrorPolyNum = mpolynum;
                cube->Model.MirrorVertNum = mvertnum;

                offset = ((long)cube->Model.MirrorVertPtr) - ((long)mverts);
                for (j = 0 ; j < mpolynum ; j++)
                {
                    cube->Model.MirrorPolyPtr[j] = mpolys[j];
                    cube->Model.MirrorPolyPtr[j].v0 = (WORLD_MIRROR_VERTEX*)(((long)cube->Model.MirrorPolyPtr[j].v0) + offset);
                    cube->Model.MirrorPolyPtr[j].v1 = (WORLD_MIRROR_VERTEX*)(((long)cube->Model.MirrorPolyPtr[j].v1) + offset);
                    cube->Model.MirrorPolyPtr[j].v2 = (WORLD_MIRROR_VERTEX*)(((long)cube->Model.MirrorPolyPtr[j].v2) + offset);
                    if (mpolys[j].Type & POLY_QUAD)
                        cube->Model.MirrorPolyPtr[j].v3 = (WORLD_MIRROR_VERTEX*)(((long)cube->Model.MirrorPolyPtr[j].v3) + offset);

                    if (mpolys[j].Tpage != -1)
                    {
                        if (mpolys[j].Type & POLY_QUAD) cube->Model.MirrorQuadNumTex++;
                        else cube->Model.MirrorTriNumTex++;
                    }
                    else
                    {
                        if (mpolys[j].Type & POLY_QUAD) cube->Model.MirrorQuadNumRGB++;
                        else cube->Model.MirrorTriNumRGB++;
                    }
                }

                for (j = 0 ; j < mvertnum ; j++)
                {
                    cube->Model.MirrorVertPtr[j] = mverts[j];
                }
            }
        }

// free temp poly + vert ram

        free(mpolys);
        free(mverts);
    }
}

/////////////////////////////
// set world mirror poly's //
/////////////////////////////

void SetWorldMirror(void)
{
    long i, j;
    CUBE_HEADER *cube;
    WORLD_POLY *p;

// loop thru all world cubes

    cube = World.Cube;
    for (i = 0 ; i < World.CubeNum ; i++, cube++)
    {
        p = cube->Model.PolyPtr;
        for (j = 0 ; j < cube->Model.PolyNum ; j++, p++)
        {
            if (p->Type & POLY_MIRROR)
            {
                if (RenderSettings.Mirror)
                {
                    if (MirrorType)
                    {
                        p->Type |= (POLY_SEMITRANS | POLY_SEMITRANS_ONE);
                    }
                    else
                    {
                        p->Type |= POLY_SEMITRANS;
                    }
                }
                else
                {
                    p->Type &= ~(POLY_SEMITRANS | POLY_SEMITRANS_ONE);
                }
            }
        }
    }
}



//$ADDITION(jedl) - New world rendering pipeline. First pass: no visiboxes. Just render the whole damn thing and let the GPU handle it.
// $TODO: Bring back visiboxes.  Add mirrors, lights, and effects.

//-----------------------------------------------------------------------------
// Name: DrawWorldGPU
// Desc: Render the world using the GPU
//       ($$ AS OPPOSED TO THE CPU-SIDE TRANSFORMS/ETC DONE IN THE OLD PC VERSION OF RE-VOLT $$)
//-----------------------------------------------------------------------------

// $TODO: Get rid of these global variables, or move to the right place.  The communication
//        between the app and the effects is not yet worked out.
XGMATRIX g_Proj;
XGVECTOR4 g_vScale;
XGVECTOR4 g_vOffset;

void DrawWorldGPU(void)
{
    long i, j, k, gap;
    float z, cuberad;
    VEC *cubepos;
    BIG_CUBE_HEADER *pBigCube, **rpBigCubeList;

    // loop thru big cubes
    WorldBigCubeCount = 0;
    pBigCube = World.BigCube;
    for (i = 0 ; i < World.BigCubeNum ; i++, pBigCube++)
    {
        // test big cube against camera view planes
        cubepos = (VEC*)&pBigCube->x;
        cuberad = pBigCube->Radius;

        z = cubepos->v[X] * ViewMatrix.m[RZ] + cubepos->v[Y] * ViewMatrix.m[UZ] + cubepos->v[Z] * ViewMatrix.m[LZ] + ViewTrans.v[Z];
        if (z + cuberad < RenderSettings.NearClip) continue;
        if (z - cuberad >= RenderSettings.FarClip) continue;

        if (PlaneDist(&CameraPlaneLeft, cubepos) >= cuberad) continue;
        if (PlaneDist(&CameraPlaneRight, cubepos) >= cuberad) continue;
        if (PlaneDist(&CameraPlaneBottom, cubepos) >= cuberad) continue;
        if (PlaneDist(&CameraPlaneTop, cubepos) >= cuberad) continue;

        // big cube passed.  That's good enough for us.
        World.BigCubeList[WorldBigCubeCount] = pBigCube;
        WorldBigCubeCount++;

        FTOL(z + cuberad, pBigCube->ZScreen); // set screen z for sort below

        // $REVISIT: Add VisiMask to BigCubes?
    }

    // quit if nothing to render
    if (!WorldBigCubeCount) return;

    // shell sort 'in view' cubes
    rpBigCubeList = World.BigCubeList;
    for (k = 0 ; k < 3 ; k++)
    {
        gap = ShellGap[k];
        for (i = gap ; i < WorldBigCubeCount ; i++)
        {
            pBigCube = rpBigCubeList[i];
            for (j = i - gap ; j >= 0 && pBigCube->ZScreen < rpBigCubeList[j]->ZScreen ; j -= gap)
            {
                rpBigCubeList[j + gap] = rpBigCubeList[j];
            }
            rpBigCubeList[j + gap] = pBigCube;
        }
    }

    // Set default state.
    Effect::BeginDraw();
    Assert(World.m_pDefaultWorldCubeMaterial != NULL);
	World.m_pDefaultWorldCubeMaterial->DrawEffect(&g_Proj, 4);

    // Draw opaque cubes in order from front to back
    D3Ddevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
    for (i = 0 ; i < WorldBigCubeCount ; i++)
    {
        BIG_CUBE_HEADER *pBigCube = rpBigCubeList[i];
        if (pBigCube->m_pModelOpaque != NULL)
            pBigCube->m_pModelOpaque->DrawEffect();
    }
    
    // Draw alpha cubes from back to front.
	// $TODO: if we use the original mirroring scheme, these should be drawn last, after the cars and objects
    D3Ddevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    for (i = WorldBigCubeCount - 1 ; i >= 0 ; i--)
    {
        BIG_CUBE_HEADER *pBigCube = rpBigCubeList[i];
        if (pBigCube->m_pModelAlpha != NULL)
            pBigCube->m_pModelAlpha->DrawEffect();
    }
    
    // Restore state
    Effect::EndDraw();
}
//$END_ADDITION



//////////////////
// draw a world //
//////////////////

void DrawWorld(void)
{
//$ADDITION(jedl) - use new world drawing code
    if (RegistrySettings.bUseGPU
        && World.m_pDefaultWorldCubeMaterial != NULL)
    {
        DrawWorldGPU();
        return;
    }
//$END_ADDITION
    long i, j, k, gap;
    float z, cuberad;
    float l, r, t, b, camy;
    VEC *cubepos;
    BIG_CUBE_HEADER *bch;
    CUBE_HEADER *ch, **chp;

// loop thru big cubes

    if (Version == VERSION_DEV)
    {
        WorldBigCubeCount = 0;
    }

    WorldCubeCount = 0;
    bch = World.BigCube;

    for (i = 0 ; i < World.BigCubeNum ; i++, bch++)
    {

// test big cube against camera view planes

        cubepos = (VEC*)&bch->x;
        cuberad = bch->Radius;

        z = cubepos->v[X] * ViewMatrix.m[RZ] + cubepos->v[Y] * ViewMatrix.m[UZ] + cubepos->v[Z] * ViewMatrix.m[LZ] + ViewTrans.v[Z];
        if (z + cuberad < RenderSettings.NearClip) continue;
        if (z - cuberad >= RenderSettings.FarClip) continue;

        if (PlaneDist(&CameraPlaneLeft, cubepos) >= cuberad) continue;
        if (PlaneDist(&CameraPlaneRight, cubepos) >= cuberad) continue;
        if (PlaneDist(&CameraPlaneBottom, cubepos) >= cuberad) continue;
        if (PlaneDist(&CameraPlaneTop, cubepos) >= cuberad) continue;

// big cube passed, test it's sub-cubes

        if (Version == VERSION_DEV)
        {
            WorldBigCubeCount++;
        }

        chp = bch->Cubes;

        for (j = 0 ; j < bch->CubeNum ; j++)
        {

// visibox test

            if (chp[j]->VisiMask & CamVisiMask)
            {
                continue;
            }

// test cube against camera view planes

            cubepos = (VEC*)&chp[j]->CentreX;
            cuberad = chp[j]->Radius;

            z = cubepos->v[X] * ViewMatrix.m[RZ] + cubepos->v[Y] * ViewMatrix.m[UZ] + cubepos->v[Z] * ViewMatrix.m[LZ] + ViewTrans.v[Z];
            if (z + cuberad < RenderSettings.NearClip) continue;
            if (z - cuberad >= RenderSettings.FarClip) continue;

            if ((l = PlaneDist(&CameraPlaneLeft, cubepos)) >= cuberad) continue;
            if ((r = PlaneDist(&CameraPlaneRight, cubepos)) >= cuberad) continue;
            if ((b = PlaneDist(&CameraPlaneBottom, cubepos)) >= cuberad) continue;
            if ((t = PlaneDist(&CameraPlaneTop, cubepos)) >= cuberad) continue;

// cube passed, add to 'in view' list

            World.CubeList[WorldCubeCount] = chp[j];
            chp[j]->Clip = (l > -cuberad || r > -cuberad || b > -cuberad || t > -cuberad || z + cuberad >= RenderSettings.FarClip);
            if (z - cuberad < RenderSettings.NearClip) chp[j]->Clip |= 2;
            FTOL(z + cuberad, chp[j]->z);
            chp[j]->MeshFxFlag = 0;
            WorldCubeCount++;
        }
    }

// quit if nothing to render

    if (!WorldCubeCount) return;

// shell sort 'in view' cubes

    chp = World.CubeList;

    for (k = 0 ; k < 3 ; k++)
    {
        gap = ShellGap[k];
        for (i = gap ; i < WorldCubeCount ; i++)
        {
            ch = chp[i];
            for (j = i - gap ; j >= 0 && ch->z < chp[j]->z ; j -= gap)
            {
                chp[j + gap] = chp[j];
            }
            chp[j + gap] = ch;
        }
    }

// check mesh fx

    for (i = 0 ; i < WorldMeshFxCount ; i++)
        WorldMeshFx[i].Checker(WorldMeshFx[i].Data);

// set env mask

    if (RenderSettings.Env)
        WorldEnvMask = POLY_ENV;
    else
        WorldEnvMask = 0;

// draw unfogged cubes

    WorldFog = FALSE;
    WorldBucketHead = Bucket;
    WorldBucketHeadRGB = &BucketRGB;
    WorldBucketHeadClip = BucketClip;
    WorldBucketHeadClipRGB = &BucketClipRGB;
    WorldBucketHeadEnv = &BucketEnvStill;
    WorldBucketHeadEnvClip = &BucketEnvStillClip;

    for (i = 0 ; i < WorldCubeCount ; i++)
    {
        if (DxState.Fog && chp[i]->z >= RenderSettings.FogStart) break;
        DrawWorldCube(chp[i]);
    }

// draw fogged cubes

    if (i < WorldCubeCount)
    {
        WorldFog = TRUE;
        WorldBucketHead = BucketFog;
        WorldBucketHeadRGB = &BucketFogRGB;
        WorldBucketHeadClip = BucketClipFog;
        WorldBucketHeadClipRGB = &BucketClipFogRGB;
        WorldBucketHeadEnv = &BucketEnvStillFog;
        WorldBucketHeadEnvClip = &BucketEnvStillClipFog;

        FOG_ON();
        for ( ; i < WorldCubeCount ; i++)
        {
            DrawWorldCube(chp[i]);
        }
    }

// draw mirrored cubes?

    if (RenderSettings.Mirror && MirrorPlaneNum)
    {
        FOG_ON();
        camy = ViewCameraPos.v[Y];
        for (i = 0 ; i < WorldCubeCount ; i++)
        {
            if (camy < chp[i]->MirrorHeight)
                DrawWorldCubeMirror(chp[i]);
        }
    }

// fog off

    FOG_OFF();
}

//////////////////////////
// draw world wireframe //
//////////////////////////

void DrawWorldWireframe(void)
{
    long i;

    FlushPolyBuckets();
    SET_TPAGE(-1);
    ZBUFFER_OFF();

    for (i = 0 ; i < WorldCubeCount ; i++)
    {
        DrawWorldCubeWireframe(World.CubeList[i]);
    }

    ZBUFFER_ON();
}

///////////////////////
// draw a world cube //
///////////////////////

void DrawWorldCube(CUBE_HEADER *cube)
{
    long i;
    WORLD_ANIM_POLY *wap;
    WORLD_VERTEX *v;
    VEC vecx, vecy, vecz;

// get lit flag

    cube->Lit = CheckCubeLight(cube);

// set env verts

    if (RenderSettings.Env)
    {
        for (i = 0 ; i < cube->Model.EnvVertNum ; i++)
        {
            v = cube->Model.EnvVertPtr[i];

            SubVector((VEC*)&v->x, &ViewCameraPos, &vecz);
            NormalizeVector(&vecz);
            CrossProduct(&ViewCameraMatrix.mv[U], &vecz, &vecx)
            CrossProduct(&vecz, &vecx, &vecy);

            v->tu = DotProduct((VEC*)&v->nx, &vecx) * 0.5f + 0.5f;
            v->tv = DotProduct((VEC*)&v->nx, &vecy) * 0.5f + 0.5f;

            if (cube->Lit)
            {
                ModelAddGouraud((MODEL_RGB*)&v->EnvRGB, &v->r, (MODEL_RGB*)&v->color);
            }
            else
            {
                v->color = v->EnvRGB;
            }
        }
    }

// get anim poly tpages + uv's

    if (cube->Model.AnimPolyNum)
    {
        wap = cube->Model.AnimPolyPtr;

        for (i = cube->Model.AnimPolyNum ; i ; i--, wap++)
        {
            if ((wap->Anim - TexAnim) < TexAnimNum)
            {
                wap->Poly->Tpage = (short)wap->Anim->CurrentFrame->Tpage;
                *(MEM32*)&wap->Poly->tu0 = *(MEM32*)&wap->Anim->CurrentFrame->u0;
            }
        }
    }

// new verts?

    if (cube->MeshFxFlag & MESHFX_USENEWVERTS)
    {

// clip

        if (cube->Clip)
        {
            if (WorldFog) TransCubeVertsFogClipNewVerts(&cube->Model);
            else TransCubeVertsClipNewVerts(&cube->Model);
            if (cube->Clip & 2) DrawCubePolysNearClip(&cube->Model, cube->Lit);
            else DrawCubePolysClip(&cube->Model, cube->Lit);
        }

// don't clip

        else
        {
            if (WorldFog) TransCubeVertsFogNewVerts(&cube->Model);
            else TransCubeVertsNewVerts(&cube->Model);
            DrawCubePolys(&cube->Model, cube->Lit);
        }
    }

// normal

    else
    {

// clip

        if (cube->Clip)
        {
            if (WorldFog) TransCubeVertsFogClip(&cube->Model);
            else TransCubeVertsClip(&cube->Model);
            if (cube->Clip & 2) DrawCubePolysNearClip(&cube->Model, cube->Lit);
            else DrawCubePolysClip(&cube->Model, cube->Lit);
        }

// don't clip

        else
        {
            if (WorldFog) TransCubeVertsFog(&cube->Model);
            else TransCubeVerts(&cube->Model);
            DrawCubePolys(&cube->Model, cube->Lit);
        }
    }

// add to poly count

    if (Version == VERSION_DEV)
    {
        WorldPolyCount += cube->Model.QuadNumTex * 2;
        WorldPolyCount += cube->Model.TriNumTex;
        WorldPolyCount += cube->Model.QuadNumRGB * 2;
        WorldPolyCount += cube->Model.TriNumRGB;
    }
}

///////////////////////
// draw a world cube //
///////////////////////

void DrawWorldCubeMirror(CUBE_HEADER *cube)
{
    if (cube->MeshFxFlag & MESHFX_USENEWVERTS)
    {
        TransCubeVertsMirrorNewVerts(&cube->Model);
        DrawCubePolysMirror(&cube->Model, cube->Lit);
    }
    else
    {
        TransCubeVertsMirror(&cube->Model);
        DrawCubePolysMirror(&cube->Model, cube->Lit);
    }

    if (Version == VERSION_DEV)
    {
        WorldPolyCount += cube->Model.MirrorQuadNumTex * 2;
        WorldPolyCount += cube->Model.MirrorTriNumTex;
        WorldPolyCount += cube->Model.MirrorQuadNumRGB * 2;
        WorldPolyCount += cube->Model.MirrorTriNumRGB;
    }
}

///////////////////////
// draw a world cube //
///////////////////////

void DrawWorldCubeWireframe(CUBE_HEADER *cube)
{
    long i, j, vcount, rgb1, rgb2;
    WORLD_POLY *mp;
    WORLD_MODEL *m = &cube->Model;
    WORLD_VERTEX **v;

// get rgb

    rgb1 = rgb2 = 0xffffffff;

// loop thru all polys in cube

    mp = m->PolyPtr;

    for (i = m->PolyNum ; i ; i--, mp++)
    {

// reject?

        REJECT_WORLD_POLY_3D();

// nope, draw each edge

        vcount = mp->Type & POLY_QUAD ? 4 : 3;

        v = &mp->v0;
        for (j = 0 ; j < vcount ; j++)
        {
            DrawLine((VEC*)&v[j]->x, (VEC*)&v[(j + 1) % vcount]->x, rgb1, rgb2);
        }
    }
}

///////////////////////
// trans world verts //
///////////////////////

void TransCubeVertsClip(WORLD_MODEL *m)
{
    short i;
    float z;
    float nearclip = RenderSettings.NearClip;
    WORLD_VERTEX *mv;

    mv = m->VertPtr;

    for (i = 0 ; i < m->VertNum ; i++, mv++)
    {
        z = mv->x * ViewMatrixScaled.m[RZ] + mv->y * ViewMatrixScaled.m[UZ] + mv->z * ViewMatrixScaled.m[LZ] + ViewTransScaled.v[Z];

        mv->sx = (mv->x * ViewMatrixScaled.m[RX] + mv->y * ViewMatrixScaled.m[UX] + mv->z * ViewMatrixScaled.m[LX] + ViewTransScaled.v[X]) / z + RenderSettings.GeomCentreX;
        mv->sy = (mv->x * ViewMatrixScaled.m[RY] + mv->y * ViewMatrixScaled.m[UY] + mv->z * ViewMatrixScaled.m[LY] + ViewTransScaled.v[Y]) / z + RenderSettings.GeomCentreY;

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
// trans world verts //
///////////////////////

void TransCubeVertsFogClip(WORLD_MODEL *m)
{
    short i;
    float z;
    float fog;
    float nearclip = RenderSettings.NearClip;
    WORLD_VERTEX *mv;

    mv = m->VertPtr;

    for (i = 0 ; i < m->VertNum ; i++, mv++)
    {
        z = mv->x * ViewMatrixScaled.m[RZ] + mv->y * ViewMatrixScaled.m[UZ] + mv->z * ViewMatrixScaled.m[LZ] + ViewTransScaled.v[Z];

        mv->sx = (mv->x * ViewMatrixScaled.m[RX] + mv->y * ViewMatrixScaled.m[UX] + mv->z * ViewMatrixScaled.m[LX] + ViewTransScaled.v[X]) / z + RenderSettings.GeomCentreX;
        mv->sy = (mv->x * ViewMatrixScaled.m[RY] + mv->y * ViewMatrixScaled.m[UY] + mv->z * ViewMatrixScaled.m[LY] + ViewTransScaled.v[Y]) / z + RenderSettings.GeomCentreY;

        mv->rhw = 1 / z;
        mv->sz = GET_ZBUFFER(z);

        fog = (RenderSettings.FarClip - z) * RenderSettings.FogMul;
        if (fog > 255) fog = 255;
        fog -= mv->VertFog;
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
// trans world verts //
///////////////////////

void TransCubeVerts(WORLD_MODEL *m)
{
    short i;
    float z;
    WORLD_VERTEX *mv;

    mv = m->VertPtr;

    for (i = 0 ; i < m->VertNum ; i++, mv++)
    {
        z = mv->x * ViewMatrixScaled.m[RZ] + mv->y * ViewMatrixScaled.m[UZ] + mv->z * ViewMatrixScaled.m[LZ] + ViewTransScaled.v[Z];
        mv->rhw = 1 / z;

        mv->sx = (mv->x * ViewMatrixScaled.m[RX] + mv->y * ViewMatrixScaled.m[UX] + mv->z * ViewMatrixScaled.m[LX] + ViewTransScaled.v[X]) / z + RenderSettings.GeomCentreX;
        mv->sy = (mv->x * ViewMatrixScaled.m[RY] + mv->y * ViewMatrixScaled.m[UY] + mv->z * ViewMatrixScaled.m[LY] + ViewTransScaled.v[Y]) / z + RenderSettings.GeomCentreY;

        mv->sz = GET_ZBUFFER(z);
    }
}

///////////////////////
// trans world verts //
///////////////////////

void TransCubeVertsFog(WORLD_MODEL *m)
{
    short i;
    float z;
    float fog;
    WORLD_VERTEX *mv;

    mv = m->VertPtr;

    for (i = 0 ; i < m->VertNum ; i++, mv++)
    {
        z = mv->x * ViewMatrixScaled.m[RZ] + mv->y * ViewMatrixScaled.m[UZ] + mv->z * ViewMatrixScaled.m[LZ] + ViewTransScaled.v[Z];
        mv->rhw = 1 / z;

        mv->sx = (mv->x * ViewMatrixScaled.m[RX] + mv->y * ViewMatrixScaled.m[UX] + mv->z * ViewMatrixScaled.m[LX] + ViewTransScaled.v[X]) / z + RenderSettings.GeomCentreX;
        mv->sy = (mv->x * ViewMatrixScaled.m[RY] + mv->y * ViewMatrixScaled.m[UY] + mv->z * ViewMatrixScaled.m[LY] + ViewTransScaled.v[Y]) / z + RenderSettings.GeomCentreY;

        mv->sz = GET_ZBUFFER(z);

        fog = (RenderSettings.FarClip - z) * RenderSettings.FogMul;
        if (fog > 255) fog = 255;
        fog -= mv->VertFog;
        if (fog < 0) fog = 0;
        mv->specular = FTOL3(fog) << 24;
    }
}

///////////////////////
// trans world verts //
///////////////////////

void TransCubeVertsClipNewVerts(WORLD_MODEL *m)
{
    short i;
    float z;
    float nearclip = RenderSettings.NearClip;
    WORLD_VERTEX *mv;

    mv = m->VertPtr;

    for (i = 0 ; i < m->VertNum ; i++, mv++)
    {
        z = mv->x2 * ViewMatrixScaled.m[RZ] + mv->y2 * ViewMatrixScaled.m[UZ] + mv->z2 * ViewMatrixScaled.m[LZ] + ViewTransScaled.v[Z];

        mv->sx = (mv->x2 * ViewMatrixScaled.m[RX] + mv->y2 * ViewMatrixScaled.m[UX] + mv->z2 * ViewMatrixScaled.m[LX] + ViewTransScaled.v[X]) / z + RenderSettings.GeomCentreX;
        mv->sy = (mv->x2 * ViewMatrixScaled.m[RY] + mv->y2 * ViewMatrixScaled.m[UY] + mv->z2 * ViewMatrixScaled.m[LY] + ViewTransScaled.v[Y]) / z + RenderSettings.GeomCentreY;

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
// trans world verts //
///////////////////////

void TransCubeVertsFogClipNewVerts(WORLD_MODEL *m)
{
    short i;
    float z;
    float fog;
    float nearclip = RenderSettings.NearClip;
    WORLD_VERTEX *mv;

    mv = m->VertPtr;

    for (i = 0 ; i < m->VertNum ; i++, mv++)
    {
        z = mv->x2 * ViewMatrixScaled.m[RZ] + mv->y2 * ViewMatrixScaled.m[UZ] + mv->z2 * ViewMatrixScaled.m[LZ] + ViewTransScaled.v[Z];

        mv->sx = (mv->x2 * ViewMatrixScaled.m[RX] + mv->y2 * ViewMatrixScaled.m[UX] + mv->z2 * ViewMatrixScaled.m[LX] + ViewTransScaled.v[X]) / z + RenderSettings.GeomCentreX;
        mv->sy = (mv->x2 * ViewMatrixScaled.m[RY] + mv->y2 * ViewMatrixScaled.m[UY] + mv->z2 * ViewMatrixScaled.m[LY] + ViewTransScaled.v[Y]) / z + RenderSettings.GeomCentreY;

        mv->rhw = 1 / z;
        mv->sz = GET_ZBUFFER(z);

        fog = (RenderSettings.FarClip - z) * RenderSettings.FogMul;
        if (fog > 255) fog = 255;
        fog -= mv->VertFog;
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
// trans world verts //
///////////////////////

void TransCubeVertsNewVerts(WORLD_MODEL *m)
{
    short i;
    float z;
    WORLD_VERTEX *mv;

    mv = m->VertPtr;

    for (i = 0 ; i < m->VertNum ; i++, mv++)
    {
        z = mv->x2 * ViewMatrixScaled.m[RZ] + mv->y2 * ViewMatrixScaled.m[UZ] + mv->z2 * ViewMatrixScaled.m[LZ] + ViewTransScaled.v[Z];
        mv->rhw = 1 / z;

        mv->sx = (mv->x2 * ViewMatrixScaled.m[RX] + mv->y2 * ViewMatrixScaled.m[UX] + mv->z2 * ViewMatrixScaled.m[LX] + ViewTransScaled.v[X]) / z + RenderSettings.GeomCentreX;
        mv->sy = (mv->x2 * ViewMatrixScaled.m[RY] + mv->y2 * ViewMatrixScaled.m[UY] + mv->z2 * ViewMatrixScaled.m[LY] + ViewTransScaled.v[Y]) / z + RenderSettings.GeomCentreY;

        mv->sz = GET_ZBUFFER(z);
    }
}

///////////////////////
// trans world verts //
///////////////////////

void TransCubeVertsFogNewVerts(WORLD_MODEL *m)
{
    short i;
    float z;
    float fog;
    WORLD_VERTEX *mv;

    mv = m->VertPtr;

    for (i = 0 ; i < m->VertNum ; i++, mv++)
    {
        z = mv->x2 * ViewMatrixScaled.m[RZ] + mv->y2 * ViewMatrixScaled.m[UZ] + mv->z2 * ViewMatrixScaled.m[LZ] + ViewTransScaled.v[Z];
        mv->rhw = 1 / z;

        mv->sx = (mv->x2 * ViewMatrixScaled.m[RX] + mv->y2 * ViewMatrixScaled.m[UX] + mv->z2 * ViewMatrixScaled.m[LX] + ViewTransScaled.v[X]) / z + RenderSettings.GeomCentreX;
        mv->sy = (mv->x2 * ViewMatrixScaled.m[RY] + mv->y2 * ViewMatrixScaled.m[UY] + mv->z2 * ViewMatrixScaled.m[LY] + ViewTransScaled.v[Y]) / z + RenderSettings.GeomCentreY;

        mv->sz = GET_ZBUFFER(z);

        fog = (RenderSettings.FarClip - z) * RenderSettings.FogMul;
        if (fog > 255) fog = 255;
        fog -= mv->VertFog;
        if (fog < 0) fog = 0;
        mv->specular = FTOL3(fog) << 24;
    }
}

///////////////////////
// trans world verts //
///////////////////////

void TransCubeVertsMirror(WORLD_MODEL *m)
{
    short i;
    float z;
    float fog;
    float nearclip = RenderSettings.NearClip;
    WORLD_MIRROR_VERTEX *mv;

    mv = m->MirrorVertPtr;

    for (i = 0 ; i < m->MirrorVertNum ; i++, mv++)
    {
        z = mv->x * ViewMatrixScaled.m[RZ] + mv->y * ViewMatrixScaled.m[UZ] + mv->z * ViewMatrixScaled.m[LZ] + ViewTransScaled.v[Z];

        mv->sx = (mv->x * ViewMatrixScaled.m[RX] + mv->y * ViewMatrixScaled.m[UX] + mv->z * ViewMatrixScaled.m[LX] + ViewTransScaled.v[X]) / z + RenderSettings.GeomCentreX;
        mv->sy = (mv->x * ViewMatrixScaled.m[RY] + mv->y * ViewMatrixScaled.m[UY] + mv->z * ViewMatrixScaled.m[LY] + ViewTransScaled.v[Y]) / z + RenderSettings.GeomCentreY;

        mv->rhw = 1 / z;
        mv->sz = GET_ZBUFFER(z);

        fog = (RenderSettings.FarClip - z) * RenderSettings.FogMul;
        if (fog > 255) fog = 255;
        fog -= mv->VertFog;
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
// trans world verts //
///////////////////////

void TransCubeVertsMirrorNewVerts(WORLD_MODEL *m)
{
    short i;
    float z;
    float fog;
    float nearclip = RenderSettings.NearClip;
    WORLD_MIRROR_VERTEX *mv;

    mv = m->MirrorVertPtr;

    for (i = 0 ; i < m->MirrorVertNum ; i++, mv++)
    {
        z = mv->x2 * ViewMatrixScaled.m[RZ] + mv->y2 * ViewMatrixScaled.m[UZ] + mv->z2 * ViewMatrixScaled.m[LZ] + ViewTransScaled.v[Z];

        mv->sx = (mv->x2 * ViewMatrixScaled.m[RX] + mv->y2 * ViewMatrixScaled.m[UX] + mv->z2 * ViewMatrixScaled.m[LX] + ViewTransScaled.v[X]) / z + RenderSettings.GeomCentreX;
        mv->sy = (mv->x2 * ViewMatrixScaled.m[RY] + mv->y2 * ViewMatrixScaled.m[UY] + mv->z2 * ViewMatrixScaled.m[LY] + ViewTransScaled.v[Y]) / z + RenderSettings.GeomCentreY;

        mv->rhw = 1 / z;
        mv->sz = GET_ZBUFFER(z);

        fog = (RenderSettings.FarClip - z) * RenderSettings.FogMul;
        if (fog > 255) fog = 255;
        fog -= mv->VertFog;
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

//////////////////////
// draw world polys //
//////////////////////

void DrawCubePolysNearClip(WORLD_MODEL *m, long lit)
{
    long i, clip;
    WORLD_POLY *mp;
    VERTEX_TEX1 *vert;
    BUCKET_TEX1 *bucket;
    VERTEX_TEX0 *vertrgb;
    BUCKET_TEX0 *bucketrgb;
    BUCKET_ENV *envbucket;
    short count;

// draw textured quads

    mp = m->PolyPtr;

    for (i = m->QuadNumTex ; i ; i--, mp++)
    {

// reject?

        REJECT_WORLD_POLY_3D();
        CLIP_QUAD_NEAR();

// get vert ptr

        if (mp->Type & POLY_SEMITRANS)
        {
            if (!SEMI_POLY_FREE()) continue;
            SEMI_POLY_SETUP(vert, WorldFog, 4, mp->Tpage, clip, (mp->Type & POLY_SEMITRANS_ONE) != 0);
        }
        else
        {
            if (clip) bucket = &WorldBucketHeadClip[mp->Tpage];
            else bucket = &WorldBucketHead[mp->Tpage];
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

        if (lit)
        {
            COPY_WORLD_QUAD_COLOR_LIT(vert);
        }
        else
        {
            COPY_WORLD_QUAD_COLOR(vert);
        }

        if (WorldFog)
            COPY_QUAD_SPECULAR(vert);

// env?

        if (mp->Type & WorldEnvMask)
        {

// get env vert ptr

            if (clip) envbucket = WorldBucketHeadEnvClip;
            else envbucket = WorldBucketHeadEnv;
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

    for (i = m->TriNumTex ; i ; i--, mp++)
    {

// reject?

        REJECT_WORLD_POLY_3D();
        CLIP_TRI_NEAR();

// get vert ptr

        if (mp->Type & POLY_SEMITRANS)
        {
            if (!SEMI_POLY_FREE()) continue;
            SEMI_POLY_SETUP(vert, WorldFog, 3, mp->Tpage, clip, (mp->Type & POLY_SEMITRANS_ONE) != 0);
        }
        else
        {
            if (clip) bucket = &WorldBucketHeadClip[mp->Tpage];
            else bucket = &WorldBucketHead[mp->Tpage];
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

        if (lit)
        {
            COPY_WORLD_TRI_COLOR_LIT(vert);
        }
        else
        {
            COPY_WORLD_TRI_COLOR(vert);
        }

        if (WorldFog)
            COPY_TRI_SPECULAR(vert);

// env?

        if (mp->Type & WorldEnvMask)
        {

// get env vert ptr

            if (clip) envbucket = WorldBucketHeadEnvClip;
            else envbucket = WorldBucketHeadEnv;
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

    for (i = m->QuadNumRGB ; i ; i--, mp++)
    {

// reject?

        REJECT_WORLD_POLY_3D();
        CLIP_QUAD_NEAR();

// get vert ptr

        if (mp->Type & POLY_SEMITRANS)
        {
            if (!SEMI_POLY_FREE()) continue;
            SEMI_POLY_SETUP_RGB(vertrgb, WorldFog, 4, clip, (mp->Type & POLY_SEMITRANS_ONE) != 0);
        }
        else
        {
            if (clip) bucketrgb = WorldBucketHeadClipRGB;
            else bucketrgb = WorldBucketHeadRGB;
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

        if (lit)
        {
            COPY_WORLD_QUAD_COLOR_LIT(vertrgb);
        }
        else
        {
            COPY_WORLD_QUAD_COLOR(vertrgb);
        }

        if (WorldFog)
            COPY_QUAD_SPECULAR(vertrgb);

// env?

        if (mp->Type & WorldEnvMask)
        {

// get env vert ptr

            if (clip) envbucket = WorldBucketHeadEnvClip;
            else envbucket = WorldBucketHeadEnv;
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

    for (i = m->TriNumRGB ; i ; i--, mp++)
    {

// reject?

        REJECT_WORLD_POLY_3D();
        CLIP_TRI_NEAR();

// get vert ptr

        if (mp->Type & POLY_SEMITRANS)
        {
            if (!SEMI_POLY_FREE()) continue;
            SEMI_POLY_SETUP_RGB(vertrgb, WorldFog, 3, clip, (mp->Type & POLY_SEMITRANS_ONE) != 0);
        }
        else
        {
            if (clip) bucketrgb = WorldBucketHeadClipRGB;
            else bucketrgb = WorldBucketHeadRGB;
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

        if (lit)
        {
            COPY_WORLD_TRI_COLOR_LIT(vertrgb);
        }
        else
        {
            COPY_WORLD_TRI_COLOR(vertrgb);
        }

        if (WorldFog)
            COPY_TRI_SPECULAR(vertrgb);

// env?

        if (mp->Type & WorldEnvMask)
        {

// get env vert ptr

            if (clip) envbucket = WorldBucketHeadEnvClip;
            else envbucket = WorldBucketHeadEnv;
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
// draw world polys //
//////////////////////

void DrawCubePolysClip(WORLD_MODEL *m, long lit)
{
    long i, clip;
    WORLD_POLY *mp;
    VERTEX_TEX1 *vert;
    BUCKET_TEX1 *bucket;
    VERTEX_TEX0 *vertrgb;
    BUCKET_TEX0 *bucketrgb;
    BUCKET_ENV *envbucket;
    short count;

// draw textured quads

    mp = m->PolyPtr;

    for (i = m->QuadNumTex ; i ; i--, mp++)
    {

// reject?

        REJECT_WORLD_POLY();
        CLIP_QUAD();

// get vert ptr

        if (mp->Type & POLY_SEMITRANS)
        {
            if (!SEMI_POLY_FREE()) continue;
            SEMI_POLY_SETUP(vert, WorldFog, 4, mp->Tpage, clip, (mp->Type & POLY_SEMITRANS_ONE) != 0);
        }
        else
        {
            if (clip) bucket = &WorldBucketHeadClip[mp->Tpage];
            else bucket = &WorldBucketHead[mp->Tpage];
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

        if (lit)
        {
            COPY_WORLD_QUAD_COLOR_LIT(vert);
        }
        else
        {
            COPY_WORLD_QUAD_COLOR(vert);
        }

        if (WorldFog)
            COPY_QUAD_SPECULAR(vert);

// env?

        if (mp->Type & WorldEnvMask)
        {

// get env vert ptr

            if (clip) envbucket = WorldBucketHeadEnvClip;
            else envbucket = WorldBucketHeadEnv;
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

    for (i = m->TriNumTex ; i ; i--, mp++)
    {

// reject?

        REJECT_WORLD_POLY();
        CLIP_TRI();

// get vert ptr

        if (mp->Type & POLY_SEMITRANS)
        {
            if (!SEMI_POLY_FREE()) continue;
            SEMI_POLY_SETUP(vert, WorldFog, 3, mp->Tpage, clip, (mp->Type & POLY_SEMITRANS_ONE) != 0);
        }
        else
        {
            if (clip) bucket = &WorldBucketHeadClip[mp->Tpage];
            else bucket = &WorldBucketHead[mp->Tpage];
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

        if (lit)
        {
            COPY_WORLD_TRI_COLOR_LIT(vert);
        }
        else
        {
            COPY_WORLD_TRI_COLOR(vert);
        }

        if (WorldFog)
            COPY_TRI_SPECULAR(vert);

// env?

        if (mp->Type & WorldEnvMask)
        {

// get env vert ptr

            if (clip) envbucket = WorldBucketHeadEnvClip;
            else envbucket = WorldBucketHeadEnv;
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

    for (i = m->QuadNumRGB ; i ; i--, mp++)
    {

// reject?

        REJECT_WORLD_POLY();
        CLIP_QUAD();

// get vert ptr

        if (mp->Type & POLY_SEMITRANS)
        {
            if (!SEMI_POLY_FREE()) continue;
            SEMI_POLY_SETUP_RGB(vertrgb, WorldFog, 4, clip, (mp->Type & POLY_SEMITRANS_ONE) != 0);
        }
        else
        {
            if (clip) bucketrgb = WorldBucketHeadClipRGB;
            else bucketrgb = WorldBucketHeadRGB;
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

        if (lit)
        {
            COPY_WORLD_QUAD_COLOR_LIT(vertrgb);
        }
        else
        {
            COPY_WORLD_QUAD_COLOR(vertrgb);
        }

        if (WorldFog)
            COPY_QUAD_SPECULAR(vertrgb);

// env?

        if (mp->Type & WorldEnvMask)
        {

// get env vert ptr

            if (clip) envbucket = WorldBucketHeadEnvClip;
            else envbucket = WorldBucketHeadEnv;
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

    for (i = m->TriNumRGB ; i ; i--, mp++)
    {

// reject?

        REJECT_WORLD_POLY();
        CLIP_TRI();

// get vert ptr

        if (mp->Type & POLY_SEMITRANS)
        {
            if (!SEMI_POLY_FREE()) continue;
            SEMI_POLY_SETUP_RGB(vertrgb, WorldFog, 3, clip, (mp->Type & POLY_SEMITRANS_ONE) != 0);
        }
        else
        {
            if (clip) bucketrgb = WorldBucketHeadClipRGB;
            else bucketrgb = WorldBucketHeadRGB;
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

        if (lit)
        {
            COPY_WORLD_TRI_COLOR_LIT(vertrgb);
        }
        else
        {
            COPY_WORLD_TRI_COLOR(vertrgb);
        }

        if (WorldFog)
            COPY_TRI_SPECULAR(vertrgb);

// env?

        if (mp->Type & WorldEnvMask)
        {

// get env vert ptr

            if (clip) envbucket = WorldBucketHeadEnvClip;
            else envbucket = WorldBucketHeadEnv;
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
// draw world polys //
//////////////////////

void DrawCubePolys(WORLD_MODEL *m, long lit)
{
    long i;
    WORLD_POLY *mp;
    VERTEX_TEX1 *vert;
    BUCKET_TEX1 *bucket;
    VERTEX_TEX0 *vertrgb;
    BUCKET_TEX0 *bucketrgb;
    BUCKET_ENV *envbucket = WorldBucketHeadEnv;
    short count;

// draw textured quads

    mp = m->PolyPtr;

    for (i = m->QuadNumTex ; i ; i--, mp++)
    {

// reject?

        REJECT_WORLD_POLY();

// get vert ptr

        if (mp->Type & POLY_SEMITRANS)
        {
            if (!SEMI_POLY_FREE()) continue;
            SEMI_POLY_SETUP(vert, WorldFog, 4, mp->Tpage, FALSE, (mp->Type & POLY_SEMITRANS_ONE) != 0);
        }
        else
        {
            bucket = &WorldBucketHead[mp->Tpage];
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

        if (lit)
        {
            COPY_WORLD_QUAD_COLOR_LIT(vert);
        }
        else
        {
            COPY_WORLD_QUAD_COLOR(vert);
        }

        if (WorldFog)
            COPY_QUAD_SPECULAR(vert);

// env?

        if (mp->Type & WorldEnvMask)
        {

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

    for (i = m->TriNumTex ; i ; i--, mp++)
    {

// reject?

        REJECT_WORLD_POLY();

// get vert ptr

        if (mp->Type & POLY_SEMITRANS)
        {
            if (!SEMI_POLY_FREE()) continue;
            SEMI_POLY_SETUP(vert, WorldFog, 3, mp->Tpage, FALSE, (mp->Type & POLY_SEMITRANS_ONE) != 0);
        }
        else
        {
            bucket = &WorldBucketHead[mp->Tpage];
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

        if (lit)
        {
            COPY_WORLD_TRI_COLOR_LIT(vert);
        }
        else
        {
            COPY_WORLD_TRI_COLOR(vert);
        }

        if (WorldFog)
            COPY_TRI_SPECULAR(vert);

// env?

        if (mp->Type & WorldEnvMask)
        {

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

    for (i = m->QuadNumRGB ; i ; i--, mp++)
    {

// reject?

        REJECT_WORLD_POLY();

// get vert ptr

        if (mp->Type & POLY_SEMITRANS)
        {
            if (!SEMI_POLY_FREE()) continue;
            SEMI_POLY_SETUP_RGB(vertrgb, WorldFog, 4, FALSE, (mp->Type & POLY_SEMITRANS_ONE) != 0);
        }
        else
        {
            bucketrgb = WorldBucketHeadRGB;
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

        if (lit)
        {
            COPY_WORLD_QUAD_COLOR_LIT(vertrgb);
        }
        else
        {
            COPY_WORLD_QUAD_COLOR(vertrgb);
        }

        if (WorldFog)
            COPY_QUAD_SPECULAR(vertrgb);

// env?

        if (mp->Type & WorldEnvMask)
        {

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

    for (i = m->TriNumRGB ; i ; i--, mp++)
    {

// reject?

        REJECT_WORLD_POLY();

// get vert ptr

        if (mp->Type & POLY_SEMITRANS)
        {
            if (!SEMI_POLY_FREE()) continue;
            SEMI_POLY_SETUP_RGB(vertrgb, WorldFog, 3, FALSE, (mp->Type & POLY_SEMITRANS_ONE) != 0);
        }
        else
        {
            bucketrgb = WorldBucketHeadRGB;
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

        if (lit)
        {
            COPY_WORLD_TRI_COLOR_LIT(vertrgb);
        }
        else
        {
            COPY_WORLD_TRI_COLOR(vertrgb);
        }

        if (WorldFog)
            COPY_TRI_SPECULAR(vertrgb);

// env?

        if (mp->Type & WorldEnvMask)
        {

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
// draw world polys //
//////////////////////

void DrawCubePolysMirror(WORLD_MODEL *m, long lit)
{
    short count;
    long i, clip;
    WORLD_MIRROR_POLY *mp;
    VERTEX_TEX1 *vert;
    BUCKET_TEX1 *bucket;
    VERTEX_TEX0 *vertrgb;
    BUCKET_TEX0 *bucketrgb;

// draw textured quads

    mp = m->MirrorPolyPtr;

    for (i = m->MirrorQuadNumTex ; i ; i--, mp++)
    {

// reject?

        REJECT_WORLD_POLY_3D();
        CLIP_QUAD();

// get vert ptr

        if (mp->Type & POLY_SEMITRANS)
        {
            if (!SEMI_POLY_FREE()) continue;
            SEMI_POLY_SETUP(vert, FALSE, 4, mp->Tpage, clip, (mp->Type & POLY_SEMITRANS_ONE) != 0);
        }
        else
        {
            if (clip) bucket = &BucketClipFog[mp->Tpage];
            else bucket = &BucketFog[mp->Tpage];
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

        if (lit)
        {
            COPY_WORLD_QUAD_COLOR_LIT_MIRROR(vert);
        }
        else
        {
            COPY_WORLD_QUAD_COLOR(vert);
        }

        COPY_QUAD_SPECULAR(vert);
    }

// draw textured tri's

    for (i = m->MirrorTriNumTex ; i ; i--, mp++)
    {

// reject?

        REJECT_WORLD_POLY_3D();
        CLIP_TRI();

// get vert ptr

        if (mp->Type & POLY_SEMITRANS)
        {
            if (!SEMI_POLY_FREE()) continue;
            SEMI_POLY_SETUP(vert, FALSE, 3, mp->Tpage, clip, (mp->Type & POLY_SEMITRANS_ONE) != 0);
        }
        else
        {
            if (clip) bucket = &BucketClipFog[mp->Tpage];
            else bucket = &BucketFog[mp->Tpage];
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

        if (lit)
        {
            COPY_WORLD_TRI_COLOR_LIT_MIRROR(vert);
        }
        else
        {
            COPY_WORLD_TRI_COLOR(vert);
        }

        COPY_TRI_SPECULAR(vert);
    }

// draw rgb quads

    for (i = m->MirrorQuadNumRGB ; i ; i--, mp++)
    {

// reject?

        REJECT_WORLD_POLY_3D();
        CLIP_QUAD();

// get vert ptr

        if (mp->Type & POLY_SEMITRANS)
        {
            if (!SEMI_POLY_FREE()) continue;
            SEMI_POLY_SETUP_RGB(vertrgb, FALSE, 4, clip, (mp->Type & POLY_SEMITRANS_ONE) != 0);
        }
        else
        {
            if (clip) bucketrgb = &BucketClipFogRGB;
            else bucketrgb = &BucketFogRGB;
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

        if (lit)
        {
            COPY_WORLD_QUAD_COLOR_LIT_MIRROR(vertrgb);
        }
        else
        {
            COPY_WORLD_QUAD_COLOR(vertrgb);
        }

        COPY_QUAD_SPECULAR(vertrgb);
    }

// draw rgb tri's

    for (i = m->MirrorTriNumRGB ; i ; i--, mp++)
    {

// reject?

        REJECT_WORLD_POLY_3D();
        CLIP_TRI();

// get vert ptr

        if (mp->Type & POLY_SEMITRANS)
        {
            if (!SEMI_POLY_FREE()) continue;
            SEMI_POLY_SETUP_RGB(vertrgb, FALSE, 3, clip, (mp->Type & POLY_SEMITRANS_ONE) != 0);
        }
        else
        {
            if (clip) bucketrgb = &BucketClipFogRGB;
            else bucketrgb = &BucketFogRGB;
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

        if (lit)
        {
            COPY_WORLD_TRI_COLOR_LIT_MIRROR(vertrgb);
        }
        else
        {
            COPY_WORLD_TRI_COLOR(vertrgb);
        }

        COPY_TRI_SPECULAR(vertrgb);
    }
}

////////////////////////////////
// process texture animations //
////////////////////////////////

void ProcessTextureAnimations(void)
{
    long i;

    for (i = 0 ; i < TexAnimNum ; i++)
    {
        TexAnim[i].FrameTime += TimeStep;
        if (TexAnim[i].FrameTime >= TexAnim[i].CurrentFrame->Time)
        {
            TexAnim[i].FrameTime -= TexAnim[i].CurrentFrame->Time;
            TexAnim[i].CurrentFrameNum = (TexAnim[i].CurrentFrameNum + 1) % TexAnim[i].FrameNum;
            TexAnim[i].CurrentFrame = &TexAnim[i].Frame[TexAnim[i].CurrentFrameNum];
        }
    }
}
