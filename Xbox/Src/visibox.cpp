//$REVISIT(cprince): should this file be renamed EditVisibox.cpp?  (Same for visibox.h vs EditVisibox.h)
//-----------------------------------------------------------------------------
// File: visibox.cpp
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"
#include "camera.h"
#ifdef _PC
#include "input.h"
#endif
#include "main.h"
#include "geom.h"
#ifdef _PC
#include "text.h"
#include "dx.h"
#include "draw.h"
#include "instance.h"
#endif
#include "LevelLoad.h"
#include "visibox.h"
#include "editai.h"

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
/*static*/ PERM_VISIBOX  CamVisiBox[VISIBOX_MAX];   // $MODIFIED jedl - make export of camera visiboxes easier
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
/*static*/ long CamVisiBoxCount, TestVisiBoxCount;  // $MODIFIED jedl - make export of camera visiboxes easier
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

        if (EditMode == EDIT_VISIBOXES)
        {
            fvb.xmin *= EditScale;
            fvb.xmax *= EditScale;
            fvb.ymin *= EditScale;
            fvb.ymax *= EditScale;
            fvb.zmin *= EditScale;
            fvb.zmax *= EditScale;

            fvb.xmin += EditOffset.v[X];
            fvb.xmax += EditOffset.v[X];
            fvb.ymin += EditOffset.v[Y];
            fvb.ymax += EditOffset.v[Y];
            fvb.zmin += EditOffset.v[Z];
            fvb.zmax += EditOffset.v[Z];
        }

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
    sprintf(bak + strlen(file) - 3, "vi-");
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

////////////////////
// edit visiboxes //
////////////////////

void EditVisiBoxes(void)
{
    short i, n;
    VEC vec, vec2;
    float z, sx, sy, xrad, yrad, add;
    VISIBOX *vb;
//$REMOVED (tentative!!)    MAT mat, mat2;
    FILE *fp;

// toggle show big ID's

    if (Keys[DIK_1] && !LastKeys[DIK_1]) BigID = !BigID;
    
// toggle allow force ID

    if (Keys[DIK_2] && !LastKeys[DIK_2]) ForceID = !ForceID;

// quit if not in edit mode

    if (CAM_MainCamera->Type != CAM_EDIT)
    {
        CurrentVisiBox = NULL;
        return;
    }

// rotate camera?

    if (MouseRight)
    {
//$REMOVED (tentative!!)
//        RotMatrixZYX(&mat, (float)-Mouse.lY / 3072, -(float)Mouse.lX / 3072, 0);
//        MulMatrix(&CAM_MainCamera->WMatrix, &mat, &mat2);
//        CopyMatrix(&mat2, &CAM_MainCamera->WMatrix);
//
//        CAM_MainCamera->WMatrix.m[RY] = 0;
//        NormalizeVector(&CAM_MainCamera->WMatrix.mv[X]);
//        CrossProduct(&CAM_MainCamera->WMatrix.mv[Z], &CAM_MainCamera->WMatrix.mv[X], &CAM_MainCamera->WMatrix.mv[Y]);
//        NormalizeVector(&CAM_MainCamera->WMatrix.mv[Y]);
//        CrossProduct(&CAM_MainCamera->WMatrix.mv[X], &CAM_MainCamera->WMatrix.mv[Y], &CAM_MainCamera->WMatrix.mv[Z]);
//$END_REMOVAL
    }

// save visiboxes?

    if (Keys[DIK_LCONTROL] && Keys[DIK_F4] && !LastKeys[DIK_F4])
    {
        SaveVisiBoxes(GetLevelFilename("vis", FILENAME_MAKE_BODY));

        if ((fp = fopen(GetLevelFilename("cam", FILENAME_MAKE_BODY | FILENAME_GAME_SETTINGS), "rb")) != NULL)
        {
            CAM_NCameraNodes = LoadCameraNodes(fp);
            fclose(fp);
        }
    }

// get a current visibox?

    if (!CurrentVisiBox && Keys[DIK_RETURN] && !LastKeys[DIK_RETURN])
    {
        n = -1;
        z = RenderSettings.FarClip;

        for (i = 0 ; i < VISIBOX_MAX ; i++) if (VisiBox[i].Flag)
        {
            vec2.v[X] = (VisiBox[i].xmin + VisiBox[i].xmax) / 2;
            vec2.v[Y] = (VisiBox[i].ymin + VisiBox[i].ymax) / 2;
            vec2.v[Z] = (VisiBox[i].zmin + VisiBox[i].zmax) / 2;

            RotTransVector(&ViewMatrix, &ViewTrans, &vec2, &vec);

            if (vec.v[Z] < RenderSettings.NearClip || vec.v[Z] >= RenderSettings.FarClip) continue;

            sx = vec.v[X] * RenderSettings.GeomPers / vec.v[Z] + REAL_SCREEN_XHALF;
            sy = vec.v[Y] * RenderSettings.GeomPers / vec.v[Z] + REAL_SCREEN_YHALF;

            xrad = (32 * RenderSettings.GeomPers) / vec.v[Z];
            yrad = (32 * RenderSettings.GeomPers) / vec.v[Z];

            if (MouseXpos > sx - xrad && MouseXpos < sx + xrad && MouseYpos > sy - yrad && MouseYpos < sy + yrad)
            {
                if (vec.v[Z] < z)
                {
                    n = i;
                    z = vec.v[Z];
                }
            }
        }
        if (n != -1)
        {
            CurrentVisiBox = &VisiBox[n];
            return;
        }
    }

// new visibox?

    if (Keys[DIK_INSERT] && !LastKeys[DIK_INSERT])
    {
        if ((vb = AllocVisiBox()))
        {
            vec.v[X] = 0;
            vec.v[Y] = 0;
            vec.v[Z] = 256;
            RotVector(&CAM_MainCamera->WMatrix, &vec, &vec2);
            AddVector(&vec2, &CAM_MainCamera->WPos, &vec);

            vb->xmin = vec.v[X] - 64;
            vb->xmax = vec.v[X] + 64;
            vb->ymin = vec.v[Y] - 64;
            vb->ymax = vec.v[Y] + 64;
            vb->zmin = vec.v[Z] - 64;
            vb->zmax = vec.v[Z] + 64;

            vb->Flag = VISIBOX_CAMERA;
            vb->ID = LastVisiID;

            CurrentVisiBox = vb;
        }
    }

// quit now if no current visibox

    if (!CurrentVisiBox) return;

// update perm visiboxes

    if (!(FrameCount & 15))
    {
        SetPermVisiBoxes();

        for (i = 0 ; i < InstanceNum ; i++)
            Instances[i].VisiMask = SetObjectVisiMask(&Instances[i].Box);
    }

// update LastVisiID

    LastVisiID = CurrentVisiBox->ID;

// exit current visibox edit?

    if (Keys[DIK_RETURN] && !LastKeys[DIK_RETURN])
    {
        CurrentVisiBox = NULL;
        return;
    }

// delete visibox?

    if (Keys[DIK_DELETE] && !LastKeys[DIK_DELETE])
    {
        FreeVisiBox(CurrentVisiBox);
        CurrentVisiBox = NULL;
        return;
    }

// toggle flag?

    if (Keys[DIK_NUMPADENTER] && !LastKeys[DIK_NUMPADENTER])
    {
        CurrentVisiBox->Flag ^= 3;
    }

// change ID?

    if (Keys[DIK_NUMPADMINUS] && !LastKeys[DIK_NUMPADMINUS])
        CurrentVisiBox->ID--;
    if (Keys[DIK_NUMPADPLUS] && !LastKeys[DIK_NUMPADPLUS])
        CurrentVisiBox->ID++;

    CurrentVisiBox->ID &= 63;

// change side?

    if (Keys[DIK_SPACE] && !LastKeys[DIK_SPACE])
        CurrentVisiSide = (CurrentVisiSide + 1) % 6;

// change axis?

    if (Keys[DIK_TAB] && !LastKeys[DIK_TAB])
        {
            if (Keys[DIK_LSHIFT]) VisiAxis--;
            else VisiAxis++;
            if (VisiAxis == -1) VisiAxis = 5;
            if (VisiAxis == 6) VisiAxis = 0;
        }

// change axis type?

    if (Keys[DIK_LALT] && !LastKeys[DIK_LALT])
        VisiAxisType ^= 1;

// change ST rate?

    if (Keys[DIK_EQUALS] && VisiboxSemi < 0xff000000) VisiboxSemi += 0x01000000;
    if (Keys[DIK_MINUS] && VisiboxSemi) VisiboxSemi -= 0x01000000;

// move visibox?

    if (MouseLeft)
    {
        vec2.v[X] = (CurrentVisiBox->xmin + CurrentVisiBox->xmax) / 2;
        vec2.v[Y] = (CurrentVisiBox->ymin + CurrentVisiBox->ymax) / 2;
        vec2.v[Z] = (CurrentVisiBox->zmin + CurrentVisiBox->zmax) / 2;
        RotTransVector(&ViewMatrix, &ViewTrans, &vec2, &vec);

        switch (VisiAxis)
        {
            case VISI_AXIS_XY:
                vec.v[X] = MouseXrel * vec.v[Z] / RenderSettings.GeomPers + CameraEditXrel;
                vec.v[Y] = MouseYrel * vec.v[Z] / RenderSettings.GeomPers + CameraEditYrel;
                vec.v[Z] = CameraEditZrel;
                break;
            case VISI_AXIS_XZ:
                vec.v[X] = MouseXrel * vec.v[Z] / RenderSettings.GeomPers + CameraEditXrel;
                vec.v[Y] = CameraEditYrel;
                vec.v[Z] = -MouseYrel * vec.v[Z] / RenderSettings.GeomPers + CameraEditZrel;
                break;
            case VISI_AXIS_ZY:
                vec.v[X] = CameraEditXrel;
                vec.v[Y] = MouseYrel * vec.v[Z] / RenderSettings.GeomPers + CameraEditYrel;
                vec.v[Z] = MouseXrel * vec.v[Z] / RenderSettings.GeomPers + CameraEditZrel;
                break;
            case VISI_AXIS_X:
                vec.v[X] = MouseXrel * vec.v[Z] / RenderSettings.GeomPers + CameraEditXrel;
                vec.v[Y] = CameraEditYrel;
                vec.v[Z] = CameraEditZrel;
                break;
            case VISI_AXIS_Y:
                vec.v[X] = CameraEditXrel;
                vec.v[Y] = MouseYrel * vec.v[Z] / RenderSettings.GeomPers + CameraEditYrel;
                vec.v[Z] = CameraEditZrel;
                break;
            case VISI_AXIS_Z:
                vec.v[X] = CameraEditXrel;
                vec.v[Y] = CameraEditYrel;
                vec.v[Z] = -MouseYrel * vec.v[Z] / RenderSettings.GeomPers + CameraEditZrel;
                break;
        }

        if (VisiAxisType == 1)
        {
            SetVector(&vec2, vec.v[X], vec.v[Y], vec.v[Z]);
        }
        else
        {
            RotVector(&CAM_MainCamera->WMatrix, &vec, &vec2);
        }

        CurrentVisiBox->xmin += vec2.v[X];
        CurrentVisiBox->xmax += vec2.v[X];
        CurrentVisiBox->ymin += vec2.v[Y];
        CurrentVisiBox->ymax += vec2.v[Y];
        CurrentVisiBox->zmin += vec2.v[Z];
        CurrentVisiBox->zmax += vec2.v[Z];
    }

// move side?

    add = 0;
    if (Keys[DIK_NUMPADSTAR])
        add = 576 * TimeStep;
    if (Keys[DIK_NUMPADSLASH])
        add = -576 * TimeStep;

    if (Keys[DIK_LCONTROL]) add *= 4;
    
    if (add)
    {
        if (Keys[DIK_LSHIFT])
        {
            if (CurrentVisiBox->xmin - add < CurrentVisiBox->xmax)
            {
                CurrentVisiBox->xmin -= add;
                CurrentVisiBox->xmax += add;
            }

            if (CurrentVisiBox->ymin - add < CurrentVisiBox->ymax)
            {
                CurrentVisiBox->ymin -= add;
                CurrentVisiBox->ymax += add;
            }

            if (CurrentVisiBox->zmin - add < CurrentVisiBox->zmax)
            {
                CurrentVisiBox->zmin -= add;
                CurrentVisiBox->zmax += add;
            }
        }
        else
        {
            switch (CurrentVisiSide)
            {
                case 0:
                    CurrentVisiBox->xmin += add;
                    if (CurrentVisiBox->xmin >= CurrentVisiBox->xmax)
                        CurrentVisiBox->xmin = CurrentVisiBox->xmax - 8;
                    break;
                case 1:
                    CurrentVisiBox->xmax += add;
                    if (CurrentVisiBox->xmax <= CurrentVisiBox->xmin)
                        CurrentVisiBox->xmax = CurrentVisiBox->xmin + 8;
                    break;
                case 2:
                    CurrentVisiBox->ymin += add;
                    if (CurrentVisiBox->ymin >= CurrentVisiBox->ymax)
                        CurrentVisiBox->ymin = CurrentVisiBox->ymax - 8;
                    break;
                case 3:
                    CurrentVisiBox->ymax += add;
                    if (CurrentVisiBox->ymax <= CurrentVisiBox->ymin)
                        CurrentVisiBox->ymax = CurrentVisiBox->ymin + 8;
                    break;
                case 4:
                    CurrentVisiBox->zmin += add;
                    if (CurrentVisiBox->zmin >= CurrentVisiBox->zmax)
                        CurrentVisiBox->zmin = CurrentVisiBox->zmax - 8;
                    break;
                case 5:
                    CurrentVisiBox->zmax += add;
                    if (CurrentVisiBox->zmax <= CurrentVisiBox->zmin)
                        CurrentVisiBox->zmax = CurrentVisiBox->zmin + 8;
                    break;
            }
        }
    }
}

//////////////////////////
// display visibox info //
//////////////////////////

void DisplayVisiBoxInfo(VISIBOX *visibox)
{
    WCHAR buf[128];

// flag
    swprintf( buf, L"%S", VisiBoxNames[visibox->Flag - 1]);
    DumpText(450, 0, 8, 16, 0xffff00, buf);

// ID

    swprintf(buf, L"ID %d", (short)visibox->ID);
    DumpText(450, 24, 8, 16, 0x00ff00, buf);

// axis

    swprintf(buf, L"Axis %S - %S", VisiAxisNames[VisiAxis], VisiAxisTypeNames[VisiAxisType]);
    DumpText(450, 48, 8, 16, 0xff00ff, buf);
}

///////////////////////////////////
// display 'camera in' visiboxes //
///////////////////////////////////

void DisplayCamVisiMask(void)
{
    unsigned long i;
    VISIMASK mask;
    short y;
    WCHAR buf[128];

// BigID

    if (BigID)
        DumpText(0, 32, 8, 16, 0xff00ff, L"Big Brothers");

// ForceID

    if (ForceID)
        DumpText(0, 48, 8, 16, 0xff00ff, L"Locked Camera");

// 'in' ID's

    mask = CamVisiMask;
    y = 64;

    for (i = 0 ; i < VISIBOX_MAX_ID ; i++, mask >>= 1) if (mask & 1)
    {
        swprintf(buf, L"%d", i);
        DumpText(0, y, 8, 16, 0xffffff, buf);
        y += 16;
    }
}

////////////////////
// draw visiboxes //
////////////////////

void DrawVisiBoxes(void)
{
    long i, j;
    VISIBOX *vb;
    float xmin, xmax, ymin, ymax, zmin, zmax, size;
    VEC vec, vec2;
    long col[6];
    WCHAR buf[128];

// draw camera nodes

    for (i = 0 ; i < CAM_NCameraNodes ; i++)
    {
        DrawModel(&EditAiNodeModel[0], &IdentityMatrix, &CAM_CameraNode[i].Pos, MODEL_PLAIN);
    }

// set box render states

    BLEND_SRC(D3DBLEND_SRCALPHA);
    BLEND_DEST(D3DBLEND_INVSRCALPHA);
    ZWRITE_ON();

// loop thru all visiboxes

    vb = VisiBox;
    for (i = 0 ; i < VISIBOX_MAX ; i++, vb++) if (vb->Flag)
    {

// real size?

        if (vb == CurrentVisiBox || (CurrentVisiBox && vb->ID == CurrentVisiBox->ID && BigID))
        {
            xmin = vb->xmin;
            xmax = vb->xmax;
            ymin = vb->ymin;
            ymax = vb->ymax;
            zmin = vb->zmin;
            zmax = vb->zmax;

            col[2] = 0xff0000 | VisiboxSemi;
            col[1] = 0x00ff00 | VisiboxSemi;
            col[0] = 0x0000ff | VisiboxSemi;
            col[4] = 0xffff00 | VisiboxSemi;
            col[3] = 0x00ffff | VisiboxSemi;
            col[5] = 0xff00ff | VisiboxSemi;

            if (vb == CurrentVisiBox)
                col[CurrentVisiSide] = rand() | VisiboxSemi;

            ZBUFFER_ON();
            BLEND_ALPHA();
            DrawBoundingBox(xmin, xmax, ymin, ymax, zmin, zmax, col[0], col[1], col[2], col[3], col[4], col[5]);
        }

// no, small!

        else
        {
            vec.v[X] = (vb->xmin + vb->xmax) / 2;
            vec.v[Y] = (vb->ymin + vb->ymax) / 2;
            vec.v[Z] = (vb->zmin + vb->zmax) / 2;

            xmin = vec.v[X] - 32;
            xmax = vec.v[X] + 32;
            ymin = vec.v[Y] - 32;
            ymax = vec.v[Y] + 32;
            zmin = vec.v[Z] - 32;
            zmax = vec.v[Z] + 32;

            for (j = 0 ; j < 6 ; j++)
                col[j] = 0xff0000 | VisiboxSemi;

            ZBUFFER_OFF();
            BLEND_ALPHA();
            DrawBoundingBox(xmin, xmax, ymin, ymax, zmin, zmax, col[0], col[1], col[2], col[3], col[4], col[5]);
        }

// text info

        if (vb != CurrentVisiBox)
        {
            ZBUFFER_OFF();
            BLEND_OFF();

            if (CurrentVisiBox && vb->ID == CurrentVisiBox->ID)
                size = 128;
            else
                size = 32;

            vec.v[X] = (vb->xmin + vb->xmax) / 2;
            vec.v[Y] = (vb->ymin + vb->ymax) / 2;
            vec.v[Z] = (vb->zmin + vb->zmax) / 2;

            swprintf(buf, L"%S %d", VisiBoxNames[vb->Flag - 1], vb->ID);
            RotTransVector(&ViewMatrix, &ViewTrans, &vec, &vec2);
            vec2.v[X] -= size * 4;
            vec2.v[Y] -= size;

            if (vec2.v[Z] > RenderSettings.NearClip)
                DumpText3D(&vec2, size, size * 2, 0xffffff, buf);
        }
    }

// draw axis

    if (CurrentVisiBox)
    {
        ZBUFFER_OFF();
        BLEND_OFF();

        vec.v[X] = (CurrentVisiBox->xmin + CurrentVisiBox->xmax) / 2;
        vec.v[Y] = (CurrentVisiBox->ymin + CurrentVisiBox->ymax) / 2;
        vec.v[Z] = (CurrentVisiBox->zmin + CurrentVisiBox->zmax) / 2;

        if (VisiAxisType)
            DrawAxis(&IdentityMatrix, &vec);
        else
            DrawAxis(&CAM_MainCamera->WMatrix, &vec);
    }

// reset render states

    ZBUFFER_ON();
    BLEND_OFF();
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

    CamVisiBoxCount = 0;
    CubeVisiBoxCount = 0;
    camvb = CamVisiBox;
    cubevb = CubeVisiBox;
//$MODIFIED
//    vb = VisiBox;
//
//    for (i = 0 ; i < VISIBOX_MAX ; i++, vb++)
//    {
    for (i = 0 ; i < VISIBOX_MAX ; i++)
    {
        vb = &VisiBox[i];
        // Changed this due to a compiler optimizer bug.  (When loops get
        // unrolled, some values incremented in the loop can get incremented
        // too much.  See Xbox bug 11155 for more details.)
        //
        // Until we start using RTM version of VC7 (which fixes this bug), be
        // aware of this issue, especially when loops have a fixed number of
        // iterations and a pointer is being incremented.
        //
        // In this particular case, the bug caused us to skip a bunch of
        // visiboxes!  Thus, whole swatches of terrain would be randomly
        // invisible on each level, for added fun!
//$END_MODIFICATIONS
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
        // $REVISIT jedl - This passes on intersection. Should it be max < min || min > max instead?
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
        // $REVISIT jedl - This is testing that the bounding box is completely inside. Intersection would be max >= min && min <= max.
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


