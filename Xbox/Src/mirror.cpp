//-----------------------------------------------------------------------------
// File: mirror.cpp
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"
#include "mirror.h"
#include "model.h"
#include "settings.h"
#include "LevelLoad.h"

// globals

long MirrorPlaneNum;
MIRROR_PLANE *MirrorPlanes;
long MirrorType, MirrorAlpha;
float MirrorHeight, MirrorMul, MirrorAdd, MirrorDist;

////////////////////////
// load mirror planes //
////////////////////////

bool LoadMirrorPlanes(char *file)
{
    long i;
    MIRROR_PLANE_HEADER mph;
    MIRROR_PLANE_LOAD mpl;
    FILE *fp;

// open file for reading

    fp = fopen(file, "rb");
    if (!fp)
    {
        MirrorPlaneNum = 0;
        MirrorPlanes = NULL;
        return FALSE;
     }

// get header info, alloc coll memory

    fread(&mph, sizeof(mph), 1, fp);
    MirrorPlaneNum = mph.MirrorPlaneNum;
    MirrorPlanes = (MIRROR_PLANE*)malloc(MirrorPlaneNum * sizeof(MIRROR_PLANE));
    if (!MirrorPlanes)
    {
        MirrorPlaneNum = 0;
        return FALSE;
    }

// read in polys

    for (i = 0 ; i < MirrorPlaneNum ; i++)
    {
        fread(&mpl, sizeof(mpl), 1, fp);

        MirrorPlanes[i].Xmin = mpl.MinX - MIRROR_PLANE_OVERLAP;
        MirrorPlanes[i].Xmax = mpl.MaxX + MIRROR_PLANE_OVERLAP;

        MirrorPlanes[i].Zmin = mpl.MinZ - MIRROR_PLANE_OVERLAP;
        MirrorPlanes[i].Zmax = mpl.MaxZ + MIRROR_PLANE_OVERLAP;

        MirrorPlanes[i].Height = mpl.v0.v[Y];
    }

// close file

    fclose(fp);

// return OK

    return TRUE;
}

////////////////////////
// free mirror planes //
////////////////////////

void FreeMirrorPlanes(void)
{
    free(MirrorPlanes);
}

///////////////////////
// set mirror params //
///////////////////////

void SetMirrorParams()
{
    MirrorType = CurrentLevelInfo.MirrorType;
    MirrorAlpha = (long)(CurrentLevelInfo.MirrorMix * 255) << 24;

    MirrorAdd = CurrentLevelInfo.MirrorIntensity * 256;
    MirrorDist = CurrentLevelInfo.MirrorDist;
    MirrorMul = (256 - MirrorAdd) / MirrorDist;
}

////////////////////////
// get a mirror plane //
////////////////////////

long GetMirrorPlane(VEC *pos)
{
    long i;

// loop thru all planes

    for (i = 0 ; i < MirrorPlaneNum ; i++)
    {
        if (pos->v[X] < MirrorPlanes[i].Xmin ||
            pos->v[X] > MirrorPlanes[i].Xmax ||
            pos->v[Z] < MirrorPlanes[i].Zmin ||
            pos->v[Z] > MirrorPlanes[i].Zmax)
                continue;

// found a good one

        MirrorHeight = MirrorPlanes[i].Height;
        return TRUE;
    }
    
// return none

    return FALSE;
}
