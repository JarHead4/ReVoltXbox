//-----------------------------------------------------------------------------
// File: EditZone.cpp
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"
#include "camera.h"
#include "main.h"
#include "geom.h"
#include "input.h"
#include "LevelLoad.h"
#include "dx.h"
#include "texture.h"
#include "text.h"
#include "draw.h"
#include "editzone.h"
#include "editai.h"
#include "ainode.h"
#include "ctrlread.h"
#include "player.h"

// globals

FILE_ZONE *CurrentFileZone;

static long FileZoneNum, ShowNodes, BigBrother;
static FILE_ZONE *FileZone;
static long FileZoneAxis, FileZoneAxisType;
static long FileZoneSide, LastFileZoneID;

// misc edit text

static char *FileZoneAxisNames[] = {
    "X Y",
    "X Z",
    "Z Y",
    "X",
    "Y",
    "Z",
};

static char *FileZoneAxisTypeNames[] = {
    "Camera",
    "World",
};

// misc draw info

static unsigned short DrawZoneIndex[] = {
    1, 3, 7, 5,
    2, 0, 4, 6,
    4, 5, 7, 6,
    0, 2, 3, 1,
    3, 2, 6, 7,
    0, 1, 5, 4,
};

static long DrawZoneCol[] = {
    0x80ff0000,
    0x8000ff00,
    0x800000ff,
    0x80ffff00,
    0x80ff00ff,
    0x8000ffff,
};

/////////////////////
// init file zones //
/////////////////////

void InitFileZones(void)
{
    FileZone = (FILE_ZONE*)malloc(sizeof(FILE_ZONE) * MAX_FILE_ZONES);
    if (!FileZone)
    {
        DumpMessage(NULL, "Can't alloc memory for file track zones!");
        QuitGame();
    }
}

////////////////////////
// kill edit ai nodes //
////////////////////////

void KillFileZones(void)
{
    free(FileZone);
}

/////////////////////
// load file zones //
/////////////////////

void LoadFileZones(char *file)
{
    long i;
    FILE *fp;
    FILE_ZONE taz;

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

// loop thru all zones

    fread(&FileZoneNum, sizeof(FileZoneNum), 1, fp);

    for (i = 0 ; i < FileZoneNum ; i++)
    {

// load one file zone

        fread(&taz, sizeof(taz), 1, fp);

        VecMulScalar(&taz.Pos, EditScale);
        VecPlusEqVec(&taz.Pos, &EditOffset);
        VecMulScalar((VEC*)taz.Size, EditScale);

// setup edit file zone

        FileZone[i] = taz;
    }

// close file

    fclose(fp);
}

/////////////////////
// save# file zones //
/////////////////////

void SaveFileZones(char *file)
{
    long i;
    FILE *fp;
    FILE_ZONE taz;
    char bak[256];

// backup old file

    memcpy(bak, file, strlen(file) - 3);
    sprintf(bak + strlen(file) - 3, "ta-");
    remove(bak);
    rename(file, bak);

// open zone file

    fp = fopen(file, "wb");
    if (!fp) return;

// write num

    fwrite(&FileZoneNum, sizeof(FileZoneNum), 1, fp);

// write out each zone

    for (i = 0 ; i < FileZoneNum ; i++)
    {

// set file zone

        taz = FileZone[i];

// write it

        fwrite(&taz, sizeof(taz), 1, fp);
    }

// close file

    DumpMessage("Saved Track Zone File:", file);
    fclose(fp);
}

///////////////////////
// alloc a file zone //
///////////////////////

FILE_ZONE *AllocFileZone(void)
{

// full?

    if (FileZoneNum >= MAX_FILE_ZONES)
        return NULL;

// inc counter, return slot

    return &FileZone[FileZoneNum++];
}

//////////////////////
// free a file zone //
//////////////////////

void FreeFileZone(FILE_ZONE *zone)
{
    long idx, i;

// find index into list

    idx = (long)(zone - FileZone);

// copy all higher zones down one

    for (i = idx ; i < FileZoneNum - 1; i++)
    {
        FileZone[i] = FileZone[i + 1];
    }

// dec num

    FileZoneNum--;
}

///////////////////////
// edit current zone //
///////////////////////

void EditFileZones(void)
{
    long i, j;
    VEC vec, vec2, r, u, l, r2, u2, l2;
    float z, sx, sy, rad, add;
    MAT mat, mat2;
    FILE_ZONE *nzone, *zone, tempzone;

// toggle show nodes

    if (Keys[DIK_1] && !LastKeys[DIK_1])
        ShowNodes = !ShowNodes;

// toggle big brother

    if (Keys[DIK_2] && !LastKeys[DIK_2])
        BigBrother = !BigBrother;

// quit if not in edit mode

    if (CAM_MainCamera->Type != CAM_EDIT)
    {
        CurrentFileZone = NULL;
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

// save zones?

    if (Keys[DIK_LCONTROL] && Keys[DIK_F4] && !LastKeys[DIK_F4])
    {
        if (FileZoneNum) for (i = FileZoneNum - 1 ; i ; i--) for (j = 0 ; j < i ; j++) if (FileZone[j].ID > FileZone[j + 1].ID)
        {
            tempzone = FileZone[j];
            FileZone[j] = FileZone[j + 1];
            FileZone[j + 1] = tempzone;
        }

        SaveFileZones(GetLevelFilename("taz", FILENAME_MAKE_BODY | FILENAME_GAME_SETTINGS));
    }

// get a current zone?

    if (!CurrentFileZone && Keys[DIK_RETURN] && !LastKeys[DIK_RETURN])
    {
        nzone = NULL;
        z = RenderSettings.FarClip;

        zone = FileZone;
        for (i = 0 ; i < FileZoneNum ; i++, zone++)
        {
            for (j = 0 ; j < 2 ; j++)
            {
                RotTransVector(&ViewMatrix, &ViewTrans, &zone->Pos, &vec);

                if (vec.v[Z] < RenderSettings.NearClip || vec.v[Z] >= RenderSettings.FarClip) continue;

                sx = vec.v[X] * RenderSettings.GeomPers / vec.v[Z] + REAL_SCREEN_XHALF;
                sy = vec.v[Y] * RenderSettings.GeomPers / vec.v[Z] + REAL_SCREEN_YHALF;

                rad = 128 * RenderSettings.GeomPers / vec.v[Z];

                if (MouseXpos > sx - rad && MouseXpos < sx + rad && MouseYpos > sy - rad && MouseYpos < sy + rad)
                {
                    if (vec.v[Z] < z)
                    {
                        nzone = zone;
                        z = vec.v[Z];
                    }
                }
            }
        }
        if (nzone)
        {
            CurrentFileZone = nzone;
            return;
        }
    }

// new zone?

    if (Keys[DIK_INSERT] && !LastKeys[DIK_INSERT])
    {
        if ((zone = AllocFileZone()))
        {
            vec.v[X] = 0;
            vec.v[Y] = 0;
            vec.v[Z] = 512;
            RotVector(&CAM_MainCamera->WMatrix, &vec, &vec2);
            AddVector(&CAM_MainCamera->WPos, &vec2, &zone->Pos);

            CopyMatrix(&IdentityMatrix, &zone->Matrix);

            zone->Size[X] = zone->Size[Y] = zone->Size[Z] = 128;
            zone->ID = LastFileZoneID;

            CurrentFileZone = zone;
        }
    }

// quit now if no current file zone

    if (!CurrentFileZone) return;

// save ID to last

    LastFileZoneID = CurrentFileZone->ID;

// exit current edit?

    if (Keys[DIK_RETURN] && !LastKeys[DIK_RETURN])
    {
        CurrentFileZone = NULL;
        return;
    }

// delete current file zone?

    if (Keys[DIK_DELETE] && !LastKeys[DIK_DELETE])
    {
        FreeFileZone(CurrentFileZone);
        CurrentFileZone = NULL;
        return;
    }

// change axis?

    if (Keys[DIK_TAB] && !LastKeys[DIK_TAB])
    {
        if (Keys[DIK_LSHIFT]) FileZoneAxis--;
        else FileZoneAxis++;
        if (FileZoneAxis == -1) FileZoneAxis = 5;
        if (FileZoneAxis == 6) FileZoneAxis = 0;
    }

// change axis type?

    if (Keys[DIK_LALT] && !LastKeys[DIK_LALT])
        FileZoneAxisType ^= 1;

// change ID?

    if (Keys[DIK_NUMPADMINUS] && !LastKeys[DIK_NUMPADMINUS])
        CurrentFileZone->ID--;
    if (Keys[DIK_NUMPADPLUS] && !LastKeys[DIK_NUMPADPLUS])
        CurrentFileZone->ID++;

    CurrentFileZone->ID &= 255;

// change side?

    if (Keys[DIK_SPACE] && !LastKeys[DIK_SPACE])
        FileZoneSide = (FileZoneSide + 1) % 6;

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
            CurrentFileZone->Size[X] += add;
            if (CurrentFileZone->Size[X] < 16) CurrentFileZone->Size[X] = 16;
            CurrentFileZone->Size[Y] += add;
            if (CurrentFileZone->Size[Y] < 16) CurrentFileZone->Size[Y] = 16;
            CurrentFileZone->Size[Z] += add;
            if (CurrentFileZone->Size[Z] < 16) CurrentFileZone->Size[Z] = 16;
        }
        else
        {
            switch (FileZoneSide)
            {
                case 0:
                    CurrentFileZone->Size[X] += add;
                    if (CurrentFileZone->Size[X] < 16) CurrentFileZone->Size[X] = 16;
                    CurrentFileZone->Pos.v[X] += CurrentFileZone->Matrix.m[RX] * add;
                    CurrentFileZone->Pos.v[Y] += CurrentFileZone->Matrix.m[RY] * add;
                    CurrentFileZone->Pos.v[Z] += CurrentFileZone->Matrix.m[RZ] * add;
                    break;
                case 1:
                    CurrentFileZone->Size[X] += add;
                    if (CurrentFileZone->Size[X] < 16) CurrentFileZone->Size[X] = 16;
                    CurrentFileZone->Pos.v[X] -= CurrentFileZone->Matrix.m[RX] * add;
                    CurrentFileZone->Pos.v[Y] -= CurrentFileZone->Matrix.m[RY] * add;
                    CurrentFileZone->Pos.v[Z] -= CurrentFileZone->Matrix.m[RZ] * add;
                    break;
                case 2:
                    CurrentFileZone->Size[Y] += add;
                    if (CurrentFileZone->Size[Y] < 16) CurrentFileZone->Size[Y] = 16;
                    CurrentFileZone->Pos.v[X] += CurrentFileZone->Matrix.m[UX] * add;
                    CurrentFileZone->Pos.v[Y] += CurrentFileZone->Matrix.m[UY] * add;
                    CurrentFileZone->Pos.v[Z] += CurrentFileZone->Matrix.m[UZ] * add;
                    break;
                case 3:
                    CurrentFileZone->Size[Y] += add;
                    if (CurrentFileZone->Size[Y] < 16) CurrentFileZone->Size[Y] = 16;
                    CurrentFileZone->Pos.v[X] -= CurrentFileZone->Matrix.m[UX] * add;
                    CurrentFileZone->Pos.v[Y] -= CurrentFileZone->Matrix.m[UY] * add;
                    CurrentFileZone->Pos.v[Z] -= CurrentFileZone->Matrix.m[UZ] * add;
                    break;
                case 4:
                    CurrentFileZone->Size[Z] += add;
                    if (CurrentFileZone->Size[Z] < 16) CurrentFileZone->Size[Z] = 16;
                    CurrentFileZone->Pos.v[X] += CurrentFileZone->Matrix.m[LX] * add;
                    CurrentFileZone->Pos.v[Y] += CurrentFileZone->Matrix.m[LY] * add;
                    CurrentFileZone->Pos.v[Z] += CurrentFileZone->Matrix.m[LZ] * add;
                    break;
                case 5:
                    CurrentFileZone->Size[Z] += add;
                    if (CurrentFileZone->Size[Z] < 16) CurrentFileZone->Size[Z] = 16;
                    CurrentFileZone->Pos.v[X] -= CurrentFileZone->Matrix.m[LX] * add;
                    CurrentFileZone->Pos.v[Y] -= CurrentFileZone->Matrix.m[LY] * add;
                    CurrentFileZone->Pos.v[Z] -= CurrentFileZone->Matrix.m[LZ] * add;
                    break;
            }
        }
    }

// move?

    if (MouseLeft)
    {
        RotTransVector(&ViewMatrix, &ViewTrans, &CurrentFileZone->Pos, &vec);

        switch (FileZoneAxis)
        {
            case FILE_ZONE_AXIS_XY:
                vec.v[X] = MouseXrel * vec.v[Z] / RenderSettings.GeomPers + CameraEditXrel;
                vec.v[Y] = MouseYrel * vec.v[Z] / RenderSettings.GeomPers + CameraEditYrel;
                vec.v[Z] = CameraEditZrel;
                break;
            case FILE_ZONE_AXIS_XZ:
                vec.v[X] = MouseXrel * vec.v[Z] / RenderSettings.GeomPers + CameraEditXrel;
                vec.v[Y] = CameraEditYrel;
                vec.v[Z] = -MouseYrel * vec.v[Z] / RenderSettings.GeomPers + CameraEditZrel;
                break;
            case FILE_ZONE_AXIS_ZY:
                vec.v[X] = CameraEditXrel;
                vec.v[Y] = MouseYrel * vec.v[Z] / RenderSettings.GeomPers + CameraEditYrel;
                vec.v[Z] = MouseXrel * vec.v[Z] / RenderSettings.GeomPers + CameraEditZrel;
                break;
            case FILE_ZONE_AXIS_X:
                vec.v[X] = MouseXrel * vec.v[Z] / RenderSettings.GeomPers + CameraEditXrel;
                vec.v[Y] = CameraEditYrel;
                vec.v[Z] = CameraEditZrel;
                break;
            case FILE_ZONE_AXIS_Y:
                vec.v[X] = CameraEditXrel;
                vec.v[Y] = MouseYrel * vec.v[Z] / RenderSettings.GeomPers + CameraEditYrel;
                vec.v[Z] = CameraEditZrel;
                break;
            case FILE_ZONE_AXIS_Z:
                vec.v[X] = CameraEditXrel;
                vec.v[Y] = CameraEditYrel;
                vec.v[Z] = -MouseYrel * vec.v[Z] / RenderSettings.GeomPers + CameraEditZrel;
                break;
        }

        if (FileZoneAxisType == 1) 
        {
            SetVector(&vec2, vec.v[X], vec.v[Y], vec.v[Z]);
        }
        else
        {
            RotVector(&CAM_MainCamera->WMatrix, &vec, &vec2);
        }

        CurrentFileZone->Pos.v[X] += vec2.v[X];
        CurrentFileZone->Pos.v[Y] += vec2.v[Y];
        CurrentFileZone->Pos.v[Z] += vec2.v[Z];
    }

// rotate?

    vec.v[X] = vec.v[Y] = vec.v[Z] = 0;

    if (Keys[DIK_NUMPAD7]) vec.v[X] -= 0.005f;
    if (Keys[DIK_NUMPAD4]) vec.v[X] += 0.005f;
    if (Keys[DIK_NUMPAD8]) vec.v[Y] -= 0.005f;
    if (Keys[DIK_NUMPAD5]) vec.v[Y] += 0.005f;
    if (Keys[DIK_NUMPAD9]) vec.v[Z] -= 0.005f;
    if (Keys[DIK_NUMPAD6]) vec.v[Z] += 0.005f;

    if (Keys[DIK_NUMPAD1] && !LastKeys[DIK_NUMPAD1]) vec.v[X] += 0.25f;
    if (Keys[DIK_NUMPAD2] && !LastKeys[DIK_NUMPAD2]) vec.v[Y] += 0.25f;
    if (Keys[DIK_NUMPAD3] && !LastKeys[DIK_NUMPAD3]) vec.v[Z] += 0.25f;

    if (Keys[DIK_NUMPAD0]) CopyMatrix(&IdentityMatrix, &CurrentFileZone->Matrix);

    RotMatrixZYX(&mat, vec.v[X], vec.v[Y], vec.v[Z]);

    if (FileZoneAxisType)
    {
        MulMatrix(&mat, &CurrentFileZone->Matrix, &mat2);
        CopyMatrix(&mat2, &CurrentFileZone->Matrix);
        NormalizeMatrix(&CurrentFileZone->Matrix);
    }
    else if (vec.v[X] || vec.v[Y] || vec.v[Z])
    {
        RotVector(&ViewMatrix, &CurrentFileZone->Matrix.mv[X], &r);
        RotVector(&ViewMatrix, &CurrentFileZone->Matrix.mv[Y], &u);
        RotVector(&ViewMatrix, &CurrentFileZone->Matrix.mv[Z], &l);

        RotVector(&mat, &r, &r2);
        RotVector(&mat, &u, &u2);
        RotVector(&mat, &l, &l2);

        RotVector(&CAM_MainCamera->WMatrix, &r2, &CurrentFileZone->Matrix.mv[X]);
        RotVector(&CAM_MainCamera->WMatrix, &u2, &CurrentFileZone->Matrix.mv[Y]);
        RotVector(&CAM_MainCamera->WMatrix, &l2, &CurrentFileZone->Matrix.mv[Z]);

        NormalizeMatrix(&CurrentFileZone->Matrix);
    }
}

/////////////////////
// draw file zones //
/////////////////////

void DrawFileZones(void)
{
    long i, j, k, col[4];
    FILE_ZONE *zone;
    VEC v[8], vpos[8], pos[4];
    MAT mat;
    VEC vec;
    WCHAR buf[128];
    ONE_AINODE *node;
    VEC v1, v2;

// draw zones?

    if (ShowNodes)
    {
        BLEND_OFF();
        ZBUFFER_ON();

// loop thru all nodes

        for (i = 0 ; i < AiNodeNum ; i++)
        {
            for (j = 0 ; j < 2 ; j++)
            {
                node = &AiNode[i].Node[j];

// draw it

                DrawModel(&EditAiNodeModel[j], &IdentityMatrix, &node->Pos, MODEL_PLAIN);

// draw link?

                for (k = 0 ; k < MAX_AINODE_LINKS ; k++) if (AiNode[i].Next[k])
                {
                    v1.v[X] = AiNode[i].Node[j].Pos.v[X];
                    v1.v[Y] = AiNode[i].Node[j].Pos.v[Y];
                    v1.v[Z] = AiNode[i].Node[j].Pos.v[Z];

                    v2.v[X] = AiNode[i].Next[k]->Node[j].Pos.v[X];
                    v2.v[Y] = AiNode[i].Next[k]->Node[j].Pos.v[Y];
                    v2.v[Z] = AiNode[i].Next[k]->Node[j].Pos.v[Z];

                    if (!j)
                    {
                        DrawLine(&v1, &v2, 0x000000, 0xff0000);
                    }
                    else
                    {
                        DrawLine(&v1, &v2, 0x000000, 0x00ff00);
                    }
                }
            }

// draw 'brother' link

        if (AiNode[i].ZoneID == LONG_MAX)
            DrawLine(&AiNode[i].Node[0].Pos, &AiNode[i].Node[1].Pos, 0xffffff, 0xffffff);
        else
            DrawLine(&AiNode[i].Node[0].Pos, &AiNode[i].Node[1].Pos, 0xffff00, 0xffff00);
        }
    }

// set render states

    BLEND_SRC(D3DBLEND_SRCALPHA);
    BLEND_DEST(D3DBLEND_INVSRCALPHA);

// loop thru all zones

    zone = FileZone;
    for (i = 0 ; i < FileZoneNum ; i++, zone++)
    {

// big brother skip?

        if (!BigBrother && CurrentFileZone && CurrentFileZone != zone)
            continue;

// get 8 corners

        SetVector(&v[0], -zone->Size[X], -zone->Size[Y], -zone->Size[Z]);
        SetVector(&v[1], zone->Size[X], -zone->Size[Y], -zone->Size[Z]);
        SetVector(&v[2], -zone->Size[X], -zone->Size[Y], zone->Size[Z]);
        SetVector(&v[3], zone->Size[X], -zone->Size[Y], zone->Size[Z]);

        SetVector(&v[4], -zone->Size[X], zone->Size[Y], -zone->Size[Z]);
        SetVector(&v[5], zone->Size[X], zone->Size[Y], -zone->Size[Z]);
        SetVector(&v[6], -zone->Size[X], zone->Size[Y], zone->Size[Z]);
        SetVector(&v[7], zone->Size[X], zone->Size[Y], zone->Size[Z]);

        for (j = 0 ; j < 8 ; j++)
        {
            RotTransVector(&zone->Matrix, &zone->Pos, &v[j], &vpos[j]);
        }

// draw

        SET_TPAGE(-1);
        BLEND_ALPHA();
        ZBUFFER_ON();

        for (j = 0 ; j < 6 ; j++)
        {
            for (k = 0 ; k < 4 ; k++)
            {
                pos[k] = vpos[DrawZoneIndex[j * 4 + k]];

                if (CurrentFileZone == zone && j == FileZoneSide)
                    col[k] = 0x80ffffff * (FrameCount & 1);
                else
                    col[k] = DrawZoneCol[zone->ID % 6];
            }

            DrawNearClipPolyTEX0(pos, col, 4);
        }

// display ID

        swprintf(buf, L"%d", zone->ID);
        RotTransVector(&ViewMatrix, &ViewTrans, &zone->Pos, &vec);
        vec.v[X] -= 64 * ((zone->ID >= 10) + 1);
        vec.v[Y] -= 128;
        if (vec.v[Z] > RenderSettings.NearClip)
        {
            BLEND_OFF();
            ZBUFFER_OFF();
            DumpText3D(&vec, 128, 256, 0x00ffff, buf);
        }

// draw axis?

        if (CurrentFileZone == zone)
        {
            BLEND_OFF();
            ZBUFFER_OFF();

            if (FileZoneAxisType)
                CopyMatrix(&IdentityMatrix, &mat);
            else
                CopyMatrix(&CAM_MainCamera->WMatrix, &mat);

            MatMulScalar(&mat, 2.0f);
            DrawAxis(&mat, &zone->Pos);
        }
    }

// reset render states

    BLEND_OFF();
    ZBUFFER_ON();
}

///////////////////////
// display zone info //
///////////////////////

void DisplayZoneInfo(FILE_ZONE *zone)
{
    WCHAR buf[128];

// ID

    swprintf(buf, L"ID %d", zone->ID);
    DumpText(450, 0, 8, 16, 0xffff00, buf);

// axis

    swprintf(buf, L"Axis %S - %S", FileZoneAxisNames[FileZoneAxis], FileZoneAxisTypeNames[FileZoneAxisType]);
    DumpText(450, 24, 8, 16, 0xff00ff, buf);

// big bastard

    swprintf(buf, L"Big Brother: %s", BigBrother ? L"Yep" : L"Nope");
    DumpText(450, 48, 8, 16, 0x00ffff, buf);
}

//////////////////////////
// display current zone //
//////////////////////////

void DisplayCurrentTrackZone(void)
{
    WCHAR buf[128];

    swprintf(buf, L"Zone %d", PLR_LocalPlayer->CarAI.ZoneID);
    DumpText(0, 128, 8, 16, 0xffffff, buf);
}
