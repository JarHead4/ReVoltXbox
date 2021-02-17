//-----------------------------------------------------------------------------
// File: EditPortal.cpp
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"
#include "editportal.h"
#include "main.h"
#include "camera.h"
#include "geom.h"
#include "LevelLoad.h"
#include "input.h"
#include "text.h"
#include "draw.h"

// globals

EDIT_PORTAL *CurrentPortal;

static long EditPortalNum, BigBrother = TRUE;
static EDIT_PORTAL *EditPortal;
static long EditPortalAxis, EditPortalAxisType;
static long EditPortalSide, LastEditPortalID;

// misc edit text

static char *EditPortalAxisNames[] = {
    "X Y",
    "X Z",
    "Z Y",
    "X",
    "Y",
    "Z",
};

static char *EditPortalAxisTypeNames[] = {
    "Camera",
    "World",
};

// misc draw info

static unsigned short DrawPortalIndex[] = {
    1, 3, 7, 5,
    2, 0, 4, 6,
    4, 5, 7, 6,
    0, 2, 3, 1,
    3, 2, 6, 7,
    0, 1, 5, 4,
};

static long DrawPortalCol[] = {
    0x80ff0000,
    0x8000ff00,
    0x800000ff,
    0x80ffff00,
    0x80ff00ff,
    0x8000ffff,
};

///////////////////////
// kill edit portals //
///////////////////////

void KillEditPortals(void)
{
    free(EditPortal);
}

///////////////////////
// load edit portals //
///////////////////////

void LoadEditPortals(char *file)
{
    long i;
    FILE *fp;
    EDIT_PORTAL por;

// alloc mem

    EditPortal = (EDIT_PORTAL*)malloc(sizeof(EDIT_PORTAL) * MAX_EDIT_PORTALS);
    if (!EditPortal)
    {
        DumpMessage(NULL, "Can't alloc memory for editing portals!");
        QuitGame();
        return;
    }

// open file

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

// loop thru all portals

    fread(&EditPortalNum, sizeof(EditPortalNum), 1, fp);

    for (i = 0 ; i < EditPortalNum ; i++)
    {

// load one portal

        fread(&por, sizeof(por), 1, fp);

        VecMulScalar(&por.Pos, EditScale);
        VecPlusEqVec(&por.Pos, &EditOffset);
        VecMulScalar((VEC*)&por.Size, EditScale);

// setup edit portal

        EditPortal[i] = por;
    }

// close file

    fclose(fp);
}

///////////////////////
// save edit portals //
///////////////////////

void SaveEditPortals(char *file)
{
    long i;
    FILE *fp;
    EDIT_PORTAL por;
    char bak[256];

// backup old file

    memcpy(bak, file, strlen(file) - 3);
    sprintf(bak + strlen(file) - 3, "po-");
    remove(bak);
    rename(file, bak);

// open portal file

    fp = fopen(file, "wb");
    if (!fp) return;

// write num

    fwrite(&EditPortalNum, sizeof(EditPortalNum), 1, fp);

// write out each portal

    for (i = 0 ; i < EditPortalNum ; i++)
    {

// set file portal

        por = EditPortal[i];

// write it

        fwrite(&por, sizeof(por), 1, fp);
    }

// close file

    DumpMessage("Saved Portal File:", file);
    fclose(fp);
}

//////////////////////////
// alloc an edit portal //
//////////////////////////

EDIT_PORTAL *AllocEditPortal(void)
{

// full?

    if (EditPortalNum >= MAX_EDIT_PORTALS)
        return NULL;

// inc counter, return slot

    return &EditPortal[EditPortalNum++];
}

/////////////////////////
// free an edit portal //
/////////////////////////

void FreeEditPortal(EDIT_PORTAL *portal)
{
    long idx, i;

// find index into list

    idx = (long)(portal - EditPortal);

// copy all higher portals down one

    for (i = idx ; i < EditPortalNum - 1; i++)
    {
        EditPortal[i] = EditPortal[i + 1];
    }

// dec num

    EditPortalNum--;
}

//////////////////
// edit portals //
//////////////////

void EditPortals(void)
{
    long i, j;
    VEC vec, vec2, r, u, l, r2, u2, l2;
    float z, sx, sy, rad, add;
    MAT mat, mat2;
    EDIT_PORTAL *nportal, *portal;

// toggle big brother

    if (Keys[DIK_NUMPADENTER] && !LastKeys[DIK_NUMPADENTER])
        BigBrother = !BigBrother;

// quit if not in edit mode

    if (CAM_MainCamera->Type != CAM_EDIT)
    {
        CurrentPortal = NULL;
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

// save portals?

    if (Keys[DIK_LCONTROL] && Keys[DIK_F4] && !LastKeys[DIK_F4])
    {
        SaveEditPortals(GetLevelFilename("por", FILENAME_MAKE_BODY));
    }

// get a current portal?

    if (!CurrentPortal && Keys[DIK_RETURN] && !LastKeys[DIK_RETURN])
    {
        nportal = NULL;
        z = RenderSettings.FarClip;

        portal = EditPortal;
        for (i = 0 ; i < EditPortalNum ; i++, portal++)
        {
            for (j = 0 ; j < 2 ; j++)
            {
                RotTransVector(&ViewMatrix, &ViewTrans, &portal->Pos, &vec);

                if (vec.v[Z] < RenderSettings.NearClip || vec.v[Z] >= RenderSettings.FarClip) continue;

                sx = vec.v[X] * RenderSettings.GeomPers / vec.v[Z] + REAL_SCREEN_XHALF;
                sy = vec.v[Y] * RenderSettings.GeomPers / vec.v[Z] + REAL_SCREEN_YHALF;

                rad = 128 * RenderSettings.GeomPers / vec.v[Z];

                if (MouseXpos > sx - rad && MouseXpos < sx + rad && MouseYpos > sy - rad && MouseYpos < sy + rad)
                {
                    if (vec.v[Z] < z)
                    {
                        nportal = portal;
                        z = vec.v[Z];
                    }
                }
            }
        }
        if (nportal)
        {
            CurrentPortal = nportal;
            return;
        }
    }

// new portal?

    if (Keys[DIK_INSERT] && !LastKeys[DIK_INSERT])
    {
        if ((portal = AllocEditPortal()))
        {
            vec.v[X] = 0;
            vec.v[Y] = 0;
            vec.v[Z] = 512;
            RotVector(&CAM_MainCamera->WMatrix, &vec, &vec2);
            AddVector(&CAM_MainCamera->WPos, &vec2, &portal->Pos);

            CopyMatrix(&IdentityMatrix, &portal->Matrix);

            portal->Size[X] = portal->Size[Y] = portal->Size[Z] = 128;

            portal->Region = TRUE;
            portal->ID1 = LastEditPortalID;
            portal->ID2 = LastEditPortalID;

            CurrentPortal = portal;
        }
    }

// quit now if no current portal

    if (!CurrentPortal) return;

// save ID to last

    LastEditPortalID = CurrentPortal->ID1;

// exit current edit?

    if (Keys[DIK_RETURN] && !LastKeys[DIK_RETURN])
    {
        CurrentPortal = NULL;
        return;
    }

// delete current portal?

    if (Keys[DIK_DELETE] && !LastKeys[DIK_DELETE])
    {
        FreeEditPortal(CurrentPortal);
        CurrentPortal = NULL;
        return;
    }

// change axis?

    if (Keys[DIK_TAB] && !LastKeys[DIK_TAB])
    {
        if (Keys[DIK_LSHIFT]) EditPortalAxis--;
        else EditPortalAxis++;
        if (EditPortalAxis == -1) EditPortalAxis = 5;
        if (EditPortalAxis == 6) EditPortalAxis = 0;
    }

// change axis type?

    if (Keys[DIK_LALT] && !LastKeys[DIK_LALT])
        EditPortalAxisType ^= 1;

// change to / from region?

    if (Keys[DIK_NUMPADPERIOD] && !LastKeys[DIK_NUMPADPERIOD])
        CurrentPortal->Region = !CurrentPortal->Region;

// change ID?

    if (Keys[DIK_LSHIFT])
    {
        if (Keys[DIK_NUMPADMINUS] && !LastKeys[DIK_NUMPADMINUS])
            CurrentPortal->ID2--;
        if (Keys[DIK_NUMPADPLUS] && !LastKeys[DIK_NUMPADPLUS])
            CurrentPortal->ID2++;

        if (CurrentPortal->ID2 < 0)
            CurrentPortal->ID2 = MAX_EDIT_PORTALS - 1;
        else if (CurrentPortal->ID2 >= MAX_EDIT_PORTALS)
            CurrentPortal->ID2 = 0;
    }
    else
    {
        if (Keys[DIK_NUMPADMINUS] && !LastKeys[DIK_NUMPADMINUS])
            CurrentPortal->ID1--;
        if (Keys[DIK_NUMPADPLUS] && !LastKeys[DIK_NUMPADPLUS])
            CurrentPortal->ID1++;

        if (CurrentPortal->ID1 < 0)
            CurrentPortal->ID1 = MAX_EDIT_PORTALS - 1;
        else if (CurrentPortal->ID1 >= MAX_EDIT_PORTALS)
            CurrentPortal->ID1 = 0;
    }

// change side?

    if (Keys[DIK_SPACE] && !LastKeys[DIK_SPACE])
        EditPortalSide = (EditPortalSide + 1) % 6;

    if (!CurrentPortal->Region)
        EditPortalSide %= 4;

// resize?

    add = 0;
    if (Keys[DIK_NUMPADSTAR])
        add = 288 * TimeStep;
    if (Keys[DIK_NUMPADSLASH])
        add = -288 * TimeStep;

    if (Keys[DIK_LCONTROL]) add *= 4;

    if (add)
    {
        if (Keys[DIK_LSHIFT])
        {
            CurrentPortal->Size[X] += add;
            if (CurrentPortal->Size[X] < 16) CurrentPortal->Size[X] = 16;
            CurrentPortal->Size[Y] += add;
            if (CurrentPortal->Size[Y] < 16) CurrentPortal->Size[Y] = 16;
            CurrentPortal->Size[Z] += add;
            if (CurrentPortal->Size[Z] < 16) CurrentPortal->Size[Z] = 16;
        }
        else
        {
            switch (EditPortalSide)
            {
                case 0:
                    CurrentPortal->Size[X] += add;
                    if (CurrentPortal->Size[X] < 16) CurrentPortal->Size[X] = 16;
                    CurrentPortal->Pos.v[X] += CurrentPortal->Matrix.m[RX] * add;
                    CurrentPortal->Pos.v[Y] += CurrentPortal->Matrix.m[RY] * add;
                    CurrentPortal->Pos.v[Z] += CurrentPortal->Matrix.m[RZ] * add;
                    break;
                case 1:
                    CurrentPortal->Size[X] += add;
                    if (CurrentPortal->Size[X] < 16) CurrentPortal->Size[X] = 16;
                    CurrentPortal->Pos.v[X] -= CurrentPortal->Matrix.m[RX] * add;
                    CurrentPortal->Pos.v[Y] -= CurrentPortal->Matrix.m[RY] * add;
                    CurrentPortal->Pos.v[Z] -= CurrentPortal->Matrix.m[RZ] * add;
                    break;
                case 2:
                    CurrentPortal->Size[Y] += add;
                    if (CurrentPortal->Size[Y] < 16) CurrentPortal->Size[Y] = 16;
                    CurrentPortal->Pos.v[X] += CurrentPortal->Matrix.m[UX] * add;
                    CurrentPortal->Pos.v[Y] += CurrentPortal->Matrix.m[UY] * add;
                    CurrentPortal->Pos.v[Z] += CurrentPortal->Matrix.m[UZ] * add;
                    break;
                case 3:
                    CurrentPortal->Size[Y] += add;
                    if (CurrentPortal->Size[Y] < 16) CurrentPortal->Size[Y] = 16;
                    CurrentPortal->Pos.v[X] -= CurrentPortal->Matrix.m[UX] * add;
                    CurrentPortal->Pos.v[Y] -= CurrentPortal->Matrix.m[UY] * add;
                    CurrentPortal->Pos.v[Z] -= CurrentPortal->Matrix.m[UZ] * add;
                    break;
                case 4:
                    CurrentPortal->Size[Z] += add;
                    if (CurrentPortal->Size[Z] < 16) CurrentPortal->Size[Z] = 16;
                    CurrentPortal->Pos.v[X] += CurrentPortal->Matrix.m[LX] * add;
                    CurrentPortal->Pos.v[Y] += CurrentPortal->Matrix.m[LY] * add;
                    CurrentPortal->Pos.v[Z] += CurrentPortal->Matrix.m[LZ] * add;
                    break;
                case 5:
                    CurrentPortal->Size[Z] += add;
                    if (CurrentPortal->Size[Z] < 16) CurrentPortal->Size[Z] = 16;
                    CurrentPortal->Pos.v[X] -= CurrentPortal->Matrix.m[LX] * add;
                    CurrentPortal->Pos.v[Y] -= CurrentPortal->Matrix.m[LY] * add;
                    CurrentPortal->Pos.v[Z] -= CurrentPortal->Matrix.m[LZ] * add;
                    break;
            }
        }
    }

// move?

    if (MouseLeft)
    {
        RotTransVector(&ViewMatrix, &ViewTrans, &CurrentPortal->Pos, &vec);

        switch (EditPortalAxis)
        {
            case PORTAL_AXIS_XY:
                vec.v[X] = MouseXrel * vec.v[Z] / RenderSettings.GeomPers + CameraEditXrel;
                vec.v[Y] = MouseYrel * vec.v[Z] / RenderSettings.GeomPers + CameraEditYrel;
                vec.v[Z] = CameraEditZrel;
                break;
            case PORTAL_AXIS_XZ:
                vec.v[X] = MouseXrel * vec.v[Z] / RenderSettings.GeomPers + CameraEditXrel;
                vec.v[Y] = CameraEditYrel;
                vec.v[Z] = -MouseYrel * vec.v[Z] / RenderSettings.GeomPers + CameraEditZrel;
                break;
            case PORTAL_AXIS_ZY:
                vec.v[X] = CameraEditXrel;
                vec.v[Y] = MouseYrel * vec.v[Z] / RenderSettings.GeomPers + CameraEditYrel;
                vec.v[Z] = MouseXrel * vec.v[Z] / RenderSettings.GeomPers + CameraEditZrel;
                break;
            case PORTAL_AXIS_X:
                vec.v[X] = MouseXrel * vec.v[Z] / RenderSettings.GeomPers + CameraEditXrel;
                vec.v[Y] = CameraEditYrel;
                vec.v[Z] = CameraEditZrel;
                break;
            case PORTAL_AXIS_Y:
                vec.v[X] = CameraEditXrel;
                vec.v[Y] = MouseYrel * vec.v[Z] / RenderSettings.GeomPers + CameraEditYrel;
                vec.v[Z] = CameraEditZrel;
                break;
            case PORTAL_AXIS_Z:
                vec.v[X] = CameraEditXrel;
                vec.v[Y] = CameraEditYrel;
                vec.v[Z] = -MouseYrel * vec.v[Z] / RenderSettings.GeomPers + CameraEditZrel;
                break;
        }

        if (EditPortalAxisType == 1) 
        {
            SetVector(&vec2, vec.v[X], vec.v[Y], vec.v[Z]);
        }
        else
        {
            RotVector(&CAM_MainCamera->WMatrix, &vec, &vec2);
        }

        CurrentPortal->Pos.v[X] += vec2.v[X];
        CurrentPortal->Pos.v[Y] += vec2.v[Y];
        CurrentPortal->Pos.v[Z] += vec2.v[Z];
    }

// rotate?

    vec.v[X] = vec.v[Y] = vec.v[Z] = 0;

    if (Keys[DIK_NUMPAD7]) vec.v[X] -= 0.001f;
    if (Keys[DIK_NUMPAD4]) vec.v[X] += 0.001f;
    if (Keys[DIK_NUMPAD8]) vec.v[Y] -= 0.001f;
    if (Keys[DIK_NUMPAD5]) vec.v[Y] += 0.001f;
    if (Keys[DIK_NUMPAD9]) vec.v[Z] -= 0.001f;
    if (Keys[DIK_NUMPAD6]) vec.v[Z] += 0.001f;

    if (Keys[DIK_NUMPAD1] && !LastKeys[DIK_NUMPAD1]) vec.v[X] += 0.25f;
    if (Keys[DIK_NUMPAD2] && !LastKeys[DIK_NUMPAD2]) vec.v[Y] += 0.25f;
    if (Keys[DIK_NUMPAD3] && !LastKeys[DIK_NUMPAD3]) vec.v[Z] += 0.25f;

    if (Keys[DIK_NUMPAD0]) CopyMatrix(&IdentityMatrix, &CurrentPortal->Matrix);

    RotMatrixZYX(&mat, vec.v[X], vec.v[Y], vec.v[Z]);

    if (EditPortalAxisType)
    {
        MulMatrix(&mat, &CurrentPortal->Matrix, &mat2);
        CopyMatrix(&mat2, &CurrentPortal->Matrix);
        NormalizeMatrix(&CurrentPortal->Matrix);
    }
    else if (vec.v[X] || vec.v[Y] || vec.v[Z])
    {
        RotVector(&ViewMatrix, &CurrentPortal->Matrix.mv[X], &r);
        RotVector(&ViewMatrix, &CurrentPortal->Matrix.mv[Y], &u);
        RotVector(&ViewMatrix, &CurrentPortal->Matrix.mv[Z], &l);

        RotVector(&mat, &r, &r2);
        RotVector(&mat, &u, &u2);
        RotVector(&mat, &l, &l2);

        RotVector(&CAM_MainCamera->WMatrix, &r2, &CurrentPortal->Matrix.mv[X]);
        RotVector(&CAM_MainCamera->WMatrix, &u2, &CurrentPortal->Matrix.mv[Y]);
        RotVector(&CAM_MainCamera->WMatrix, &l2, &CurrentPortal->Matrix.mv[Z]);

        NormalizeMatrix(&CurrentPortal->Matrix);
    }
}

//////////////////
// draw portals //
//////////////////

void DrawPortals(void)
{
    long i, j, k, col[4], rgb;
    EDIT_PORTAL *portal;
    VEC v[8], vpos[8], pos[4];
    MAT mat;
    VEC vec;
    WCHAR buf[128];

// set render states

    BLEND_SRC(D3DBLEND_SRCALPHA);
    BLEND_DEST(D3DBLEND_INVSRCALPHA);

// loop thru all portals

    portal = EditPortal;
    for (i = 0 ; i < EditPortalNum ; i++, portal++)
    {

// region?

        if (portal->Region)
        {
            if (CurrentPortal && CurrentPortal->Region && ((CurrentPortal->ID1 == portal->ID1 && BigBrother) || CurrentPortal == portal))
            {

// get 8 corners

                SetVector(&v[0], -portal->Size[X], -portal->Size[Y], -portal->Size[Z]);
                SetVector(&v[1], portal->Size[X], -portal->Size[Y], -portal->Size[Z]);
                SetVector(&v[2], -portal->Size[X], -portal->Size[Y], portal->Size[Z]);
                SetVector(&v[3], portal->Size[X], -portal->Size[Y], portal->Size[Z]);

                SetVector(&v[4], -portal->Size[X], portal->Size[Y], -portal->Size[Z]);
                SetVector(&v[5], portal->Size[X], portal->Size[Y], -portal->Size[Z]);
                SetVector(&v[6], -portal->Size[X], portal->Size[Y], portal->Size[Z]);
                SetVector(&v[7], portal->Size[X], portal->Size[Y], portal->Size[Z]);

                for (j = 0 ; j < 8 ; j++)
                {
                    RotTransVector(&portal->Matrix, &portal->Pos, &v[j], &vpos[j]);
                }

// draw

                SET_TPAGE(-1);
                BLEND_ALPHA();
                ZBUFFER_ON();

                for (j = 0 ; j < 6 ; j++)
                {
                    for (k = 0 ; k < 4 ; k++)
                    {
                        pos[k] = vpos[DrawPortalIndex[j * 4 + k]];
    
                        if (CurrentPortal == portal && j == EditPortalSide)
                            col[k] = 0x80ffffff * (FrameCount & 1);
                        else
                            col[k] = DrawPortalCol[portal->ID1 % 6];
                    }

                    DrawNearClipPolyTEX0(pos, col, 4);
                }
            }

// display region ID

            swprintf(buf, L"%d", portal->ID1);
            RotTransVector(&ViewMatrix, &ViewTrans, &portal->Pos, &vec);
            vec.v[X] -= 64 * ((portal->ID1 >= 10) + 1);
            vec.v[Y] -= 128;
            if (vec.v[Z] > RenderSettings.NearClip)
            {
                BLEND_OFF();
                ZBUFFER_OFF();
                DumpText3D(&vec, 128, 256, 0x00ffff, buf);
            }
        }

// portal

        else
        {

// get rgb

            SubVector(&portal->Pos, &ViewCameraPos, &vec);
            if (DotProduct(&vec, &portal->Matrix.mv[L]) < 0.0f)
                rgb = 0x80ff0000;
            else
                rgb = 0x8000ff00;
        
// get 4 corners

            SetVector(&v[0], -portal->Size[X], -portal->Size[Y], 0);
            SetVector(&v[1], portal->Size[X], -portal->Size[Y], 0);
            SetVector(&v[2], portal->Size[X], portal->Size[Y], 0);
            SetVector(&v[3], -portal->Size[X], portal->Size[Y], 0);

            for (j = 0 ; j < 4 ; j++)
            {
                RotTransVector(&portal->Matrix, &portal->Pos, &v[j], &pos[j]);
                col[j] = rgb;
            }

// draw

            SET_TPAGE(-1);
            BLEND_ALPHA();
            ZBUFFER_ON();

            DrawNearClipPolyTEX0(pos, col, 4);
        }

// draw axis?

        if (CurrentPortal == portal)
        {
            BLEND_OFF();
            ZBUFFER_OFF();

            if (EditPortalAxisType)
                CopyMatrix(&IdentityMatrix, &mat);
            else
                CopyMatrix(&CAM_MainCamera->WMatrix, &mat);

            MatMulScalar(&mat, 2.0f);
            DrawAxis(&mat, &portal->Pos);
        }
    }

// reset render states

    BLEND_OFF();
    ZBUFFER_ON();
}

/////////////////////////
// display portal info //
/////////////////////////

void DisplayPortalInfo(EDIT_PORTAL *portal)
{
    WCHAR buf[128];

// type

    DumpText(450, 0, 8, 16, 0x00ffff, portal->Region ? L"Region" : L"Portal");

// ID's

    if (portal->Region)
    {
        swprintf(buf, L"ID %d", portal->ID1);
        DumpText(450, 24, 8, 16, 0xffff00, buf);
    }
    else
    {
        swprintf(buf, L"ID1 %d", portal->ID1);
        DumpText(450, 24, 8, 16, 0xff0000, buf);
        swprintf(buf, L"ID2 %d", portal->ID2);
        DumpText(530, 24, 8, 16, 0x00ff00, buf);
    }

// axis

    swprintf(buf, L"Axis %S - %S", EditPortalAxisNames[EditPortalAxis], EditPortalAxisTypeNames[EditPortalAxisType]);
    DumpText(450, 48, 8, 16, 0xff00ff, buf);

// big bastards

    swprintf(buf, L"Big Brother:  %s", BigBrother ? L"Yep" : L"Nope");
    DumpText(450, 72, 8, 16, 0x0000ff, buf);
}
