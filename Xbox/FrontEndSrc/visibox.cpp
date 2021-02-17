//-----------------------------------------------------------------------------
// File: visibox.cpp
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"
#include "camera.h"
#include "input.h"
#include "main.h"
#include "geom.h"
#include "text.h"
#include "dx.h"
#include "draw.h"
#include "instance.h"
#include "LevelLoad.h"
#include "visibox.h"

// globals

#ifdef _PC
PERM_VISIBOX CubeVisiBox[VISIBOX_MAX];
#endif

VISIMASK    CamVisiMask;
long        CubeVisiBoxCount;
VISIBOX *   CurrentVisiBox;
long        VisiPerPoly = TRUE;

////////////////////////////////////////////////////////////////
//
// PC - Vbles
//
////////////////////////////////////////////////////////////////
#ifdef _PC
static VISIBOX       VisiBox[VISIBOX_MAX];
static PERM_VISIBOX  CamVisiBox[VISIBOX_MAX];
static PERM_VISIBOX *TestVisiBox[VISIBOX_MAX];
#endif

////////////////////////////////////////////////////////////////
//
// N64 - Vbles
//
////////////////////////////////////////////////////////////////
#ifdef _N64
static unsigned       s_uNVisiBoxes = 0;
static VISIBOX *      VisiBox       = NULL;
static PERM_VISIBOX * CamVisiBox    = NULL;
static PERM_VISIBOX * CubeVisiBox   = NULL;
static PERM_VISIBOX **TestVisiBox   = NULL;
#endif

////////////////////////////////////////////////////////////////
//
// PC & N64 - Vbles
//
////////////////////////////////////////////////////////////////

static PERM_VISIBOX_HEADER CubeVisiBoxHeader[VISIBOX_MAX_ID];
static char BigID = TRUE;
static char LastVisiID = 0;
static char CurrentVisiSide = 0;
static char ForceID = 0;
static char VisiAxis = 0;
static char VisiAxisType = 0;
static long VisiboxSemi = 0x40000000;
#ifdef _N64
long CamVisiBoxCount, TestVisiBoxCount; // Global to check out_of_world condition for respawn
#else
static long CamVisiBoxCount, TestVisiBoxCount;
#endif

////////////////////////////////////////////////////////////////
//
// PC Only
//
////////////////////////////////////////////////////////////////
#ifdef _PC


// edit text

static char *VisiBoxNames[] = {
    "Camera",
    "Cubes",
};

static char *VisiAxisNames[] = {
    "X Y",
    "X Z",
    "Z Y",
    "X",
    "Y",
    "Z",
};

static char *VisiAxisTypeNames[] = {
    "Camera",
    "World",
};


////////////////////
// init visiboxes //
////////////////////

void InitVisiBoxes(void)
{
    short i;

    for (i = 0 ; i < VISIBOX_MAX ; i++) VisiBox[i].Flag = 0;
}

///////////////////
// alloc visibox //
///////////////////

VISIBOX *AllocVisiBox(void)
{
    short i;

// find free slot

    for (i = 0 ; i < VISIBOX_MAX ; i++) if (!VisiBox[i].Flag)
    {
        return &VisiBox[i];
    }

// no slots

    return NULL;
}

//////////////////
// free visibox //
//////////////////

void FreeVisiBox(VISIBOX *visibox)
{
    visibox->Flag = 0;
}

////////////////////
// load visiboxes //
////////////////////

void LoadVisiBoxes(char *file)
{
    long i;
    FILE *fp;
    VISIBOX *vb, fvb;

// open visibox file

    fp = fopen(file, "rb");

// if not there create empty one

    if (!fp)
    {
        fp = fopen(file, "wb");
        if (!fp) return;
        i = 0;
        fwrite(&i, sizeof(i), 1, fp);
        fclose(fp);
        fp = fopen(file, "rb");
        if (!fp) return;
    }

// loop thru all visiboxes

    fread(&i, sizeof(i), 1, fp);

    for ( ; i ; i--)
    {

// alloc visibox

        vb = AllocVisiBox();
        if (!vb) break;

// setup from file

        fread(&fvb, sizeof(fvb), 1, fp);

        vb->Flag = fvb.Flag;
        vb->ID = fvb.ID;

        vb->xmin = fvb.xmin;
        vb->xmax = fvb.xmax;
        vb->ymin = fvb.ymin;
        vb->ymax = fvb.ymax;
        vb->zmin = fvb.zmin;
        vb->zmax = fvb.zmax;
    }

// close file

    fclose(fp);
}


////////////////////
// save visiboxes //
////////////////////

void SaveVisiBoxes(char *file)
{
    long num, i;
    FILE *fp;
    VISIBOX *vb, fvb;
    char bak[256];

// backup old file

    memcpy(bak, file, strlen(file) - 3);
    wsprintf(bak + strlen(file) - 3, "vi-");
    remove(bak);
    rename(file, bak);

// open visibox file

    fp = fopen(file, "wb");
    if (!fp) return;

// count num

    for (i = num = 0 ; i < VISIBOX_MAX ; i++) if (VisiBox[i].Flag) num++;

// write num

    fwrite(&num, sizeof(num), 1, fp);

// write out each file light

    vb = VisiBox;
    for (i = 0 ; i < VISIBOX_MAX ; i++, vb++) if (vb->Flag)
    {
        fvb.Flag = vb->Flag;
        fvb.ID = vb->ID;

        fvb.xmin = vb->xmin;
        fvb.xmax = vb->xmax;
        fvb.ymin = vb->ymin;
        fvb.ymax = vb->ymax;
        fvb.zmin = vb->zmin;
        fvb.zmax = vb->zmax;

        fwrite(&fvb, sizeof(fvb), 1, fp);
    }

// close file

    DumpMessage("Saved Visibox File:", file);
    fclose(fp);
}

/////////////////////////////
// set permanent visiboxes //
/////////////////////////////

void SetPermVisiBoxes(void)
{
    long i, j, k, l;
    float xmin, xmax, ymin, ymax, zmin, zmax;
    VISIBOX *vb;
    PERM_VISIBOX *camvb, *cubevb, swap;
    CUBE_HEADER *cube;
    WORLD_POLY *poly;
    WORLD_VERTEX **vert;

// split into camera and cube lists

    vb = VisiBox;
    CamVisiBoxCount = 0;
    CubeVisiBoxCount = 0;
    camvb = CamVisiBox;
    cubevb = CubeVisiBox;

    for (i = 0 ; i < VISIBOX_MAX ; i++, vb++)
    {
        if (vb->Flag & VISIBOX_CAMERA)
        {
            camvb->ID = vb->ID;
            camvb->Mask = (VISIMASK)1 << camvb->ID;
            camvb->xmin = vb->xmin;
            camvb->xmax = vb->xmax;
            camvb->ymin = vb->ymin;
            camvb->ymax = vb->ymax;
            camvb->zmin = vb->zmin;
            camvb->zmax = vb->zmax;

            CamVisiBoxCount++;
            camvb++;
        }
        if (vb->Flag & VISIBOX_CUBE)
        {
            cubevb->ID = vb->ID;
            cubevb->Mask = (VISIMASK)1 << cubevb->ID;
            cubevb->xmin = vb->xmin;
            cubevb->xmax = vb->xmax;
            cubevb->ymin = vb->ymin;
            cubevb->ymax = vb->ymax;
            cubevb->zmin = vb->zmin;
            cubevb->zmax = vb->zmax;

            CubeVisiBoxCount++;
            cubevb++;
        }
    }

// sort cube visibox list into mask order

    if (CubeVisiBoxCount > 1)
    {
        for (i = CubeVisiBoxCount - 1 ; i ; i--) for (j = 0 ; j < i ; j++)
        {
            if (CubeVisiBox[j].Mask > CubeVisiBox[j + 1].Mask)
            {
                swap = CubeVisiBox[j];
                CubeVisiBox[j] = CubeVisiBox[j + 1];
                CubeVisiBox[j + 1] = swap;
            }
        }
    }

// setup cube visibox headers

    for (i = 0 ; i < VISIBOX_MAX_ID ; i++)
    {
        CubeVisiBoxHeader[i].Count = 0;
        for (j = 0 ; j < CubeVisiBoxCount ; j++)
        {
            if (CubeVisiBox[j].Mask == ((VISIMASK)1 << i))
            {
                CubeVisiBoxHeader[i].VisiBoxes = &CubeVisiBox[j];
                while (j < CubeVisiBoxCount && CubeVisiBox[j].Mask == ((VISIMASK)1 << i))
                {
                    j++;
                    CubeVisiBoxHeader[i].Count++;
                }
                break;
            }
        }
    }

// loop thru all world cubes setting bit masks

    cube = World.Cube;
    for (i = 0 ; i < World.CubeNum ; i++, cube++)
    {

// clear cube and poly bit masks

        cube->VisiMask = 0;

        poly = cube->Model.PolyPtr;
        for (j = 0 ; j < cube->Model.PolyNum ; j++, poly++) poly->VisiMask = 0;

// loop thru all cube visiboxes

        cubevb = CubeVisiBox;
        for (j = 0 ; j < CubeVisiBoxCount ; j++, cubevb++)
        {

// set cube mask if cube fully inside visibox

            if (cube->Xmin >= cubevb->xmin && cube->Xmax <= cubevb->xmax &&
                cube->Ymin >= cubevb->ymin && cube->Ymax <= cubevb->ymax &&
                cube->Zmin >= cubevb->zmin && cube->Zmax <= cubevb->zmax)
            {
                cube->VisiMask |= cubevb->Mask;
            }

// else set poly masks if cube partially inside visibox

            else if (VisiPerPoly && cube->Xmax >= cubevb->xmin && cube->Xmin <= cubevb->xmax &&
                cube->Ymax >= cubevb->ymin && cube->Ymin <= cubevb->ymax &&
                cube->Zmax >= cubevb->zmin && cube->Zmin <= cubevb->zmax)
            {
                poly = cube->Model.PolyPtr;
                for (k = 0 ; k < cube->Model.PolyNum ; k++, poly++)
                {
                    xmin = ymin = zmin = 999999;
                    xmax = ymax = zmax = -999999;
                    vert = &poly->v0;

                    for (l = 0 ; l < 3 + (poly->Type & 1) ; l++, vert++)
                    {
                        if ((*vert)->x < xmin) xmin = (*vert)->x;
                        if ((*vert)->x > xmax) xmax = (*vert)->x;
                        if ((*vert)->y < ymin) ymin = (*vert)->y;
                        if ((*vert)->y > ymax) ymax = (*vert)->y;
                        if ((*vert)->z < zmin) zmin = (*vert)->z;
                        if ((*vert)->z > zmax) zmax = (*vert)->z;
                    }

                    if (xmin >= cubevb->xmin && xmax <= cubevb->xmax &&
                        ymin >= cubevb->ymin && ymax <= cubevb->ymax &&
                        zmin >= cubevb->zmin && zmax <= cubevb->zmax)
                    {
                        poly->VisiMask |= cubevb->Mask;
                    }
                }
            }
        }
    }
}

#endif

////////////////////////////////////////////////////////////////
//
// PC & N64 
//
////////////////////////////////////////////////////////////////


//////////////////////////////////////////////
// calculate a visimask from a bounding box //
//////////////////////////////////////////////

VISIMASK SetObjectVisiMask(BOUNDING_BOX *box)
{
    long i;
    VISIMASK mask = 0;

    for (i = 0 ; i < CubeVisiBoxCount ; i++)
    {
        if (box->Xmin < CubeVisiBox[i].xmin || box->Xmax > CubeVisiBox[i].xmax ||
            box->Ymin < CubeVisiBox[i].ymin || box->Ymax > CubeVisiBox[i].ymax ||
            box->Zmin < CubeVisiBox[i].zmin || box->Zmax > CubeVisiBox[i].zmax)
                continue;

        mask |= CubeVisiBox[i].Mask;
    }
    return mask;
}

////////////////////////////////////////////////////////
// test an object bounding box against cube visiboxes //
////////////////////////////////////////////////////////

char TestObjectVisiboxes(BOUNDING_BOX *box)
{
    long i;

// loop thru all test visiboxes

    for (i = 0 ; i < TestVisiBoxCount ; i++)
    {
        if (box->Xmin >= TestVisiBox[i]->xmin && box->Xmax <= TestVisiBox[i]->xmax &&
            box->Ymin >= TestVisiBox[i]->ymin && box->Ymax <= TestVisiBox[i]->ymax &&
            box->Zmin >= TestVisiBox[i]->zmin && box->Zmax <= TestVisiBox[i]->zmax)
                return TRUE;
    }

// return OK

    return FALSE;
}

/////////////////////////
// set camera visimask //
/////////////////////////

void SetCameraVisiMask(VEC *pos)
{
    long i, j;
    PERM_VISIBOX *cvb;

// quit if ForceID set

    if (ForceID) return;

// loop thru camera visiboxes

    TestVisiBoxCount = 0;
    CamVisiMask = 0;
    cvb = CamVisiBox;

    for (i = 0 ; i < CamVisiBoxCount ; i++, cvb++)
    {

// camera inside this box?

        if (pos->v[X] < cvb->xmin || pos->v[X] > cvb->xmax ||
            pos->v[Y] < cvb->ymin || pos->v[Y] > cvb->ymax ||
            pos->v[Z] < cvb->zmin || pos->v[Z] > cvb->zmax)
                continue;

// yep, add mask

        CamVisiMask |= cvb->Mask;

// add cube visiboxes to test list

        for (j = 0 ; j < CubeVisiBoxHeader[cvb->ID].Count ; j++)
        {
            TestVisiBox[TestVisiBoxCount] = &CubeVisiBoxHeader[cvb->ID].VisiBoxes[j];
            TestVisiBoxCount++;
        }
    }
}

////////////////////////////////////////////////////////////////
//
// N64 Only
//
////////////////////////////////////////////////////////////////

#ifdef _N64

//-------------------------------------------------------------------
// void DestroyVisiBoxes()
//-------------------------------------------------------------------
void DestroyVisiBoxes()
    {    
    if (VisiBox) 
        { 
        free(VisiBox); 
        VisiBox=NULL; 
        };

    if (CamVisiBox)
        {
        free(CamVisiBox);
        CamVisiBox = NULL;
        }

    if (CubeVisiBox)
        {
        free(CubeVisiBox);
        CubeVisiBox = NULL;
        }

    if (TestVisiBox)
        {
        free(TestVisiBox);
        TestVisiBox = NULL;
        }

    s_uNVisiBoxes    =
    CamVisiBoxCount  =
    CubeVisiBoxCount = 
    TestVisiBoxCount = 0;
    }


//-------------------------------------------------------------------
// void InitVisiBoxes()
//-------------------------------------------------------------------
void InitVisiBoxes()
{
    DestroyVisiBoxes();
}


//-------------------------------------------------------------------
// void LoadVisiBoxes(FIL_ID file)
//-------------------------------------------------------------------
void LoadVisiBoxes(FIL_ID file)
{
    long    ii;
    FIL     *fp;
    VISIBOX *vb, fvb;

    // ... just in case
    //---------------------
    DestroyVisiBoxes();

    // try to open 
    //---------------------
    if (!(fp = FFS_Open(file)) )
        {
        printf ("WARNING: VisiBox file (*.v64) not found.\n");
        return;
        }

    // alloc VB
    //------------------
    FFS_Read(&ii, sizeof(ii), fp);
    if (!(s_uNVisiBoxes = ii) ) 
        {
        printf ("WARNING: VisiBox file contains 0 visiboxes.\n");
        FFS_Close(fp);    
        return;
        }

    if (s_uNVisiBoxes > VISIBOX_MAX)
        {
        printf("WARNING: Truncating nvisiboxes from %d to %d",s_uNVisiBoxes,VISIBOX_MAX);
        s_uNVisiBoxes = VISIBOX_MAX;
        }

    vb = VisiBox = malloc(s_uNVisiBoxes * sizeof(VISIBOX));

    // Read VB
    //------------------
    ii = s_uNVisiBoxes;
    while(ii--)
        {
        // setup from file
        //------------------
        FFS_Read(&fvb, sizeof(fvb), fp);
        if (fvb.Flag)
            {
            vb->Flag = fvb.Flag;
            vb->ID   = fvb.ID;
            vb->xmin = fvb.xmin;
            vb->xmax = fvb.xmax;
            vb->ymin = fvb.ymin;
            vb->ymax = fvb.ymax;
            vb->zmin = fvb.zmin;
            vb->zmax = fvb.zmax;
            ++vb;
            }
        }

    // close file
    //------------------
    FFS_Close(fp);    
}


//-------------------------------------------------------------------
// void SetPermVisiBoxes()
//-------------------------------------------------------------------
void SetPermVisiBoxes()
{
    // Locals
    //---------------------
    unsigned i;
    long j, k, l;
    float xmin, xmax, ymin, ymax, zmin, zmax;
    PERM_VISIBOX *camvb, *cubevb, swap;
    CUBE_HEADER *cube;
    VISIBOX *vb;

    // Validation
    //---------------------
    if (!VisiBox)
        return;

    // Counting
    //---------------------
    vb               = VisiBox;
    i                = s_uNVisiBoxes;
    CamVisiBoxCount  = 0;
    CubeVisiBoxCount = 0;

    while(i--)
        {
             if (vb->Flag & VISIBOX_CAMERA) ++CamVisiBoxCount;
        else if (vb->Flag & VISIBOX_CUBE  ) ++CubeVisiBoxCount;
        ++vb;
        }

    // Alloc'ing
    //---------------------
    if (CamVisiBox )  free(CamVisiBox ); 
    if (CubeVisiBox)  free(CubeVisiBox); 
    camvb  = CamVisiBox  = CamVisiBoxCount  ? (PERM_VISIBOX*)malloc(CamVisiBoxCount  * sizeof(PERM_VISIBOX)) : NULL;
    cubevb = CubeVisiBox = CubeVisiBoxCount ? (PERM_VISIBOX*)malloc(CubeVisiBoxCount * sizeof(PERM_VISIBOX)) : NULL;
    TestVisiBox = CubeVisiBoxCount ? (PERM_VISIBOX**)malloc(CubeVisiBoxCount * sizeof(PERM_VISIBOX*)) : NULL;
    
    // Copying
    //---------------------
    vb = VisiBox;
    i = s_uNVisiBoxes;
    while (i--)
        {
        if (vb->Flag & VISIBOX_CAMERA) 
            {
            camvb->ID = vb->ID;
            camvb->Mask = (VISIMASK)1 << camvb->ID;
            camvb->xmin = vb->xmin;
            camvb->xmax = vb->xmax;
            camvb->ymin = vb->ymin;
            camvb->ymax = vb->ymax;
            camvb->zmin = vb->zmin;
            camvb->zmax = vb->zmax;
            ++camvb;
            }
        else if (vb->Flag & VISIBOX_CUBE  ) 
            {
            cubevb->ID = vb->ID;
            cubevb->Mask = (VISIMASK)1 << cubevb->ID;
            cubevb->xmin = vb->xmin;
            cubevb->xmax = vb->xmax;
            cubevb->ymin = vb->ymin;
            cubevb->ymax = vb->ymax;
            cubevb->zmin = vb->zmin;
            cubevb->zmax = vb->zmax;
            ++cubevb;
            }
        ++vb;
        }

    // Freeing VisiBox
    //---------------------
    free(VisiBox);
    VisiBox = NULL;
    s_uNVisiBoxes = 0;



// sort cube visibox list into mask order

    if (CubeVisiBoxCount > 1)
    {
        for (i = CubeVisiBoxCount - 1 ; i ; i--) for (j = 0 ; j < i ; j++)
        {
            if (CubeVisiBox[j].Mask > CubeVisiBox[j + 1].Mask)
            {
                swap = CubeVisiBox[j];
                CubeVisiBox[j] = CubeVisiBox[j + 1];
                CubeVisiBox[j + 1] = swap;
            }
        }
    }

// setup cube visibox headers

    for (i = 0 ; i < VISIBOX_MAX_ID ; i++)
    {
        CubeVisiBoxHeader[i].Count = 0;
        for (j = 0 ; j < CubeVisiBoxCount ; j++)
        {
            if (CubeVisiBox[j].Mask == ((VISIMASK)1 << i))
            {
                CubeVisiBoxHeader[i].VisiBoxes = &CubeVisiBox[j];
                while (j < CubeVisiBoxCount && CubeVisiBox[j].Mask == ((VISIMASK)1 << i))
                {
                    j++;
                    CubeVisiBoxHeader[i].Count++;
                }
                break;
            }
        }
    }

// loop thru all world cubes setting bit masks

    cube = World->Cube;
    for (i = 0 ; i < World->CubeNum ; i++, cube++)
    {

// clear cube and poly bit masks

        cube->VisiMask = 0;

// loop thru all cube visiboxes

        cubevb = CubeVisiBox;
        for (j = 0 ; j < CubeVisiBoxCount ; j++, cubevb++)
        {

// set cube mask if cube fully inside visibox

            if (cube->Xmin >= cubevb->xmin && cube->Xmax <= cubevb->xmax &&
                cube->Ymin >= cubevb->ymin && cube->Ymax <= cubevb->ymax &&
                cube->Zmin >= cubevb->zmin && cube->Zmax <= cubevb->zmax)
            {
                cube->VisiMask |= cubevb->Mask;
            }

        }
    }
}

#endif


