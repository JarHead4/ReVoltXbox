//-----------------------------------------------------------------------------
// File: EditObject.cpp
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"
#include "main.h"
#include "camera.h"
#include "input.h"
#include "LevelLoad.h"
#include "model.h"
#include "editobject.h"
#include "text.h"
#include "draw.h"
#include "ctrlread.h"
#include "object.h"
#include "obj_init.h"

// globals

EDIT_OBJECT *CurrentFileObject;
long ShowAiNodes = FALSE;

static long UsedFileObjectCount;
static EDIT_OBJECT *FileObjectList;
static EDIT_OBJECT *UsedFileObjectHead;
static EDIT_OBJECT *FreeFileObjectHead;

static long FileObjectModelNum;
static MODEL *FileObjectModels;
static char FileObjectAxis = 0, FileObjectAxisType = 0;
static long CurrentFileObjectFlag = -1;

// misc edit text

static char *FileObjectAxisNames[] = {
    "X Y",
    "X Z",
    "Z Y",
    "X",
    "Y",
    "Z",
};

static char *FileObjectAxisTypeNames[] = {
    "Camera",
    "World",
};

// stream edit models

static long StreamEditModels[] = {43, 54, 55, 56, 57, 58};
static long RippleEditModels[] = {43, 54, 55, 56, 57, 58};

//////////////////////////
// init file object mem //
//////////////////////////

long InitFileObjects(void)
{
    long i;

// alloc list memory

    FileObjectList = (EDIT_OBJECT*)malloc(sizeof(EDIT_OBJECT) * MAX_EDIT_OBJECTS);
    if (!FileObjectList)
    {
        DumpMessage("ERROR", "Can't alloc memory for file objects");
        QuitGame();
        return FALSE;
    }

// NULL used head

    UsedFileObjectHead = NULL;

// link free list

    FreeFileObjectHead = FileObjectList;

    for (i = 0 ; i < MAX_EDIT_OBJECTS ; i++)
    {
        FileObjectList[i].Prev = &FileObjectList[i - 1];
        FileObjectList[i].Next = &FileObjectList[i + 1];
    }

// NULL first->Prev & last->Next

    FileObjectList[0].Prev = NULL;
    FileObjectList[MAX_EDIT_OBJECTS - 1].Next = NULL;

// zero count

    UsedFileObjectCount = 0;

// return OK

    return TRUE;
}

/////////////////////////////
// free file object memory //
/////////////////////////////

void KillFileObjects(void)
{
    if (FileObjectList)
    {
        free(FileObjectList);
        FileObjectList = NULL;
    }
}

/////////////////////////
// alloc a file object //
/////////////////////////

EDIT_OBJECT *AllocFileObject(void)
{
    EDIT_OBJECT *obj;

// return NULL if none free

    if (!FreeFileObjectHead)
    {
        return NULL;
    }

// remove our obj from free list

    obj = FreeFileObjectHead;
    FreeFileObjectHead = obj->Next;

    if (FreeFileObjectHead)
    {
        FreeFileObjectHead->Prev = NULL;
    }
    
// add to used list

    obj->Prev = NULL;
    obj->Next = UsedFileObjectHead;
    UsedFileObjectHead = obj;

    if (obj->Next)
    {
        obj->Next->Prev = obj;
    }

// inc used count

    UsedFileObjectCount++;

// return our obj

    return obj;
}

////////////////////////
// free a file object //
////////////////////////

void FreeFileObject(EDIT_OBJECT *obj)
{

// return if NULL ptr

    if (!obj)
    {
        return;
    }

// remove our obj from used list

    if (obj->Prev)
    {
        obj->Prev->Next = obj->Next;
    }
    else
    {
        UsedFileObjectHead = obj->Next;
    }

    if (obj->Next)
    {
        obj->Next->Prev = obj->Prev;
    }

// add to free list

    obj->Prev = NULL;
    obj->Next = FreeFileObjectHead;
    FreeFileObjectHead = obj;

    if (obj->Next)
    {
        obj->Next->Prev = obj;
    }

// dec used count

    UsedFileObjectCount--;
}

//////////////////////////
// load in file objects //
//////////////////////////

void LoadFileObjects(char *file)
{
    long i, j;
    FILE *fp;
    EDIT_OBJECT *obj;
    FILE_OBJECT fileobj;

// open object file

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

// loop thru all objects

    fread(&i, sizeof(i), 1, fp);

    for ( ; i ; i--)
    {

// alloc object

        obj = AllocFileObject();
        if (!obj) break;

// setup from file

        fread(&fileobj, sizeof(fileobj), 1, fp);

        VecMulScalar(&fileobj.Pos, EditScale);
        VecPlusEqVec(&fileobj.Pos, &EditOffset);

// set ID

        obj->ID = fileobj.ID;

// set flags

        for (j = 0 ; j < FILE_OBJECT_FLAG_NUM ; j++)
            obj->Flag[j] = fileobj.Flag[j];

// set pos

        obj->Pos = fileobj.Pos;

// set matrix

        obj->Mat.mv[U] = fileobj.Up;
        obj->Mat.mv[L] = fileobj.Look;
        CrossProduct(&obj->Mat.mv[U], &obj->Mat.mv[L], &obj->Mat.mv[R]);
    }

// close file

    fclose(fp);
}

///////////////////////
// save file objects //
///////////////////////

void SaveFileObjects(char *file)
{
    long i;
    FILE *fp;
    FILE_OBJECT fileobj;
    EDIT_OBJECT *obj;
    char bak[256];

// backup old file

    memcpy(bak, file, strlen(file) - 3);
    sprintf(bak + strlen(file) - 3, "fo-");
    remove(bak);
    rename(file, bak);

// open object file

    fp = fopen(file, "wb");
    if (!fp) return;

// write num

    fwrite(&UsedFileObjectCount, sizeof(UsedFileObjectCount), 1, fp);

// write out each file object

    for (obj = UsedFileObjectHead ; obj ; obj = obj->Next)
    {

// set ID

        fileobj.ID = obj->ID;

// set flags

        for (i = 0 ; i < FILE_OBJECT_FLAG_NUM ; i++)
            fileobj.Flag[i] = obj->Flag[i];

// set pos

        fileobj.Pos = obj->Pos;

// set up / look vectors

        fileobj.Up = obj->Mat.mv[U];
        fileobj.Look = obj->Mat.mv[L];

// write it

        fwrite(&fileobj, sizeof(fileobj), 1, fp);
    }

// close file

    DumpMessage("Saved Object File:", file);
    fclose(fp);
}

///////////////////////
// edit file objects //
///////////////////////

void EditFileObjects(void)
{
    long i, j, flag;
    EDIT_OBJECT *obj, *nobj;
    VEC vec, vec2, r, u, l, r2, u2, l2;
    MAT mat, mat2;
    float xrad, yrad, z, sx, sy;

// toggle node show

    if (Keys[DIK_N] && !LastKeys[DIK_N])
        ShowAiNodes = !ShowAiNodes;

// quit if not in edit mode

    if (CAM_MainCamera->Type != CAM_EDIT)
    {
        CurrentFileObject = NULL;
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

// save file objects?

    if (Keys[DIK_LCONTROL] && Keys[DIK_F4] && !LastKeys[DIK_F4])
    {
        SaveFileObjects(GetLevelFilename("fob", FILENAME_MAKE_BODY));
    }

// get a current object?

    if (!CurrentFileObject && Keys[DIK_RETURN] && !LastKeys[DIK_RETURN])
    {
        nobj = NULL;
        z = RenderSettings.FarClip;

        for (obj = UsedFileObjectHead ; obj ; obj = obj->Next)
        {
            RotTransVector(&ViewMatrix, &ViewTrans, &obj->Pos, &vec);

            if (vec.v[Z] < RenderSettings.NearClip || vec.v[Z] >= RenderSettings.FarClip) continue;

            sx = vec.v[X] * RenderSettings.GeomPers / vec.v[Z] + REAL_SCREEN_XHALF;
            sy = vec.v[Y] * RenderSettings.GeomPers / vec.v[Z] + REAL_SCREEN_YHALF;

            xrad = (64 * RenderSettings.GeomPers) / vec.v[Z];
            yrad = (64 * RenderSettings.GeomPers) / vec.v[Z];

            if (MouseXpos > sx - xrad && MouseXpos < sx + xrad && MouseYpos > sy - yrad && MouseYpos < sy + yrad)
            {
                if (vec.v[Z] < z)
                {
                    nobj = obj;
                    z = vec.v[Z];
                }
            }
        }
        if (nobj)
        {
            CurrentFileObject = nobj;
            return;
        }
    }

// new file object?

    if (Keys[DIK_INSERT] && !LastKeys[DIK_INSERT])
    {
        if ((obj = AllocFileObject()))
        {
            vec.v[X] = 0;
            vec.v[Y] = 0;
            vec.v[Z] = 256;
            RotVector(&CAM_MainCamera->WMatrix, &vec, &vec2);
            obj->Pos.v[X] = CAM_MainCamera->WPos.v[X] + vec2.v[X];
            obj->Pos.v[Y] = CAM_MainCamera->WPos.v[Y] + vec2.v[Y];
            obj->Pos.v[Z] = CAM_MainCamera->WPos.v[Z] + vec2.v[Z];

            RotMatrixZYX(&obj->Mat, 0, 0, 0);

            obj->ID = 0;

            for (i = 0 ; i < FILE_OBJECT_FLAG_NUM ; i++)
                obj->Flag[i] = 0;

            CurrentFileObject = obj;
        }
    }

// quit now if no current file object

    if (!CurrentFileObject) return;

// exit current file object edit?

    if (Keys[DIK_RETURN] && !LastKeys[DIK_RETURN])
    {
        CurrentFileObject = NULL;
        return;
    }

// delete current file object?

    if (Keys[DIK_DELETE] && !LastKeys[DIK_DELETE])
    {
        FreeFileObject(CurrentFileObject);
        CurrentFileObject = NULL;
        return;
    }

// copy object?

    if (MouseLeft && !MouseLastLeft && Keys[DIK_LSHIFT])
    {
        if ((obj = AllocFileObject()))
        {
            memcpy(obj, CurrentFileObject, sizeof(EDIT_OBJECT) - 8);
            CurrentFileObject = obj;
            return;
        }
    }

// change axis?

    if (Keys[DIK_TAB] && !LastKeys[DIK_TAB])
    {
        if (Keys[DIK_LSHIFT]) FileObjectAxis--;
        else FileObjectAxis++;
        if (FileObjectAxis == -1) FileObjectAxis = 5;
        if (FileObjectAxis == 6) FileObjectAxis = 0;
    }

// change axis type?

    if (Keys[DIK_LALT] && !LastKeys[DIK_LALT])
        FileObjectAxisType ^= 1;

// change ID?

    if (CurrentFileObjectFlag == -1)
    {
        flag = CurrentFileObject->ID;
        if (Keys[DIK_LEFT] && !LastKeys[DIK_LEFT])
        {
            flag--;
            while (FileObjectInfo[flag].ModelID == -2) flag--;
            if (flag < 0)
            {
                while (FileObjectInfo[flag + 1].ModelID != -1) flag++;
                while (FileObjectInfo[flag].ModelID == -2) flag--;
            }
        }

        if (Keys[DIK_RIGHT] && !LastKeys[DIK_RIGHT])
        {
            flag++;
            while (FileObjectInfo[flag].ModelID == -2) flag++;
            if (FileObjectInfo[flag].ModelID == -1)
            {
                flag = 0;
                while (FileObjectInfo[flag].ModelID == -2) flag++;
            }
        }
        CurrentFileObject->ID = flag;
    }

// count valid flags + check flag constraints

    j = 0;
    for (i = 0 ; i < FILE_OBJECT_FLAG_NUM ; i++)
    {
        if (FileObjectInfo[CurrentFileObject->ID].FlagInfo[i].Name)
        {
            j++;

            if (CurrentFileObject->Flag[i] < FileObjectInfo[CurrentFileObject->ID].FlagInfo[i].Min)
                CurrentFileObject->Flag[i] = (char)FileObjectInfo[CurrentFileObject->ID].FlagInfo[i].Min;
            if (CurrentFileObject->Flag[i] > FileObjectInfo[CurrentFileObject->ID].FlagInfo[i].Max)
                CurrentFileObject->Flag[i] = (char)FileObjectInfo[CurrentFileObject->ID].FlagInfo[i].Max;
        }
    }

// up / down

    if (Keys[DIK_LSHIFT]) LastKeys[DIK_LEFT] = LastKeys[DIK_RIGHT] = 0;

    if (Keys[DIK_UP] && !LastKeys[DIK_UP])
        CurrentFileObjectFlag--;
    if (Keys[DIK_DOWN] && !LastKeys[DIK_DOWN])
        CurrentFileObjectFlag++;

    if (CurrentFileObjectFlag < -1) CurrentFileObjectFlag = j - 1;
    if (CurrentFileObjectFlag >= j) CurrentFileObjectFlag = -1;

// change flags?

    else
    {
        for (i = j = 0 ; i < CurrentFileObjectFlag ; i++)
        {
            if (FileObjectInfo[CurrentFileObject->ID].FlagInfo[i].Name) j++;
        }

        flag = CurrentFileObject->Flag[j];

        if (Keys[DIK_LEFT] && !LastKeys[DIK_LEFT]) flag--;
        if (Keys[DIK_RIGHT] && !LastKeys[DIK_RIGHT]) flag++;

        if (flag < FileObjectInfo[CurrentFileObject->ID].FlagInfo[j].Min)
            flag = FileObjectInfo[CurrentFileObject->ID].FlagInfo[j].Max;
        if (flag > FileObjectInfo[CurrentFileObject->ID].FlagInfo[j].Max)
            flag = FileObjectInfo[CurrentFileObject->ID].FlagInfo[j].Min;

        CurrentFileObject->Flag[j] = flag;
    }

// move object?

    if (MouseLeft)
    {
        RotTransVector(&ViewMatrix, &ViewTrans, &CurrentFileObject->Pos, &vec);

        switch (FileObjectAxis)
        {
            case FILE_OBJECT_AXIS_XY:
                vec.v[X] = MouseXrel * vec.v[Z] / RenderSettings.GeomPers + CameraEditXrel;
                vec.v[Y] = MouseYrel * vec.v[Z] / RenderSettings.GeomPers + CameraEditYrel;
                vec.v[Z] = CameraEditZrel;
                break;
            case FILE_OBJECT_AXIS_XZ:
                vec.v[X] = MouseXrel * vec.v[Z] / RenderSettings.GeomPers + CameraEditXrel;
                vec.v[Y] = CameraEditYrel;
                vec.v[Z] = -MouseYrel * vec.v[Z] / RenderSettings.GeomPers + CameraEditZrel;
                break;
            case FILE_OBJECT_AXIS_ZY:
                vec.v[X] = CameraEditXrel;
                vec.v[Y] = MouseYrel * vec.v[Z] / RenderSettings.GeomPers + CameraEditYrel;
                vec.v[Z] = MouseXrel * vec.v[Z] / RenderSettings.GeomPers + CameraEditZrel;
                break;
            case FILE_OBJECT_AXIS_X:
                vec.v[X] = MouseXrel * vec.v[Z] / RenderSettings.GeomPers + CameraEditXrel;
                vec.v[Y] = CameraEditYrel;
                vec.v[Z] = CameraEditZrel;
                break;
            case FILE_OBJECT_AXIS_Y:
                vec.v[X] = CameraEditXrel;
                vec.v[Y] = MouseYrel * vec.v[Z] / RenderSettings.GeomPers + CameraEditYrel;
                vec.v[Z] = CameraEditZrel;
                break;
            case FILE_OBJECT_AXIS_Z:
                vec.v[X] = CameraEditXrel;
                vec.v[Y] = CameraEditYrel;
                vec.v[Z] = -MouseYrel * vec.v[Z] / RenderSettings.GeomPers + CameraEditZrel;
                break;
        }

        if (FileObjectAxisType == 1) 
        {
            SetVector(&vec2, vec.v[X], vec.v[Y], vec.v[Z]);
        }
        else
        {
            RotVector(&CAM_MainCamera->WMatrix, &vec, &vec2);
        }

        CurrentFileObject->Pos.v[X] += vec2.v[X];
        CurrentFileObject->Pos.v[Y] += vec2.v[Y];
        CurrentFileObject->Pos.v[Z] += vec2.v[Z];
    }

// rotate object?

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

    if (Keys[DIK_NUMPAD0]) CopyMatrix(&IdentityMatrix, &CurrentFileObject->Mat);

    RotMatrixZYX(&mat, vec.v[X], vec.v[Y], vec.v[Z]);

    if (FileObjectAxisType)
    {
        MulMatrix(&mat, &CurrentFileObject->Mat, &mat2);
        CopyMatrix(&mat2, &CurrentFileObject->Mat);
        NormalizeMatrix(&CurrentFileObject->Mat);
    }
    else if (vec.v[X] || vec.v[Y] || vec.v[Z])
    {
        RotVector(&ViewMatrix, &CurrentFileObject->Mat.mv[X], &r);
        RotVector(&ViewMatrix, &CurrentFileObject->Mat.mv[Y], &u);
        RotVector(&ViewMatrix, &CurrentFileObject->Mat.mv[Z], &l);

        RotVector(&mat, &r, &r2);
        RotVector(&mat, &u, &u2);
        RotVector(&mat, &l, &l2);

        RotVector(&CAM_MainCamera->WMatrix, &r2, &CurrentFileObject->Mat.mv[X]);
        RotVector(&CAM_MainCamera->WMatrix, &u2, &CurrentFileObject->Mat.mv[Y]);
        RotVector(&CAM_MainCamera->WMatrix, &l2, &CurrentFileObject->Mat.mv[Z]);

        NormalizeMatrix(&CurrentFileObject->Mat);
    }
}

///////////////////////
// draw file objects //
///////////////////////

void DrawFileObjects(void)
{
    EDIT_OBJECT *obj;
    MODEL *model;

// loop thru file objects

    for (obj = UsedFileObjectHead ; obj ; obj = obj->Next)
    {

// draw model

        if (obj->ID == OBJECT_TYPE_PLANET && obj->Flag[0] != PLANET_SUN)
            model = &FileObjectModels[FileObjectInfo[obj->ID].ModelID + obj->Flag[0]];
        else if (obj->ID == OBJECT_TYPE_STROBE)
            model = &FileObjectModels[FileObjectInfo[obj->ID].ModelID + obj->Flag[0]];
        else if (obj->ID == OBJECT_TYPE_CUP)
            model = &FileObjectModels[FileObjectInfo[obj->ID].ModelID + obj->Flag[0]];
        else if (obj->ID == OBJECT_TYPE_STREAM)
            model = &FileObjectModels[StreamEditModels[obj->Flag[0]]];
        else if (obj->ID == OBJECT_TYPE_RIPPLE)
            model = &FileObjectModels[RippleEditModels[obj->Flag[0]]];
        else
            model = &FileObjectModels[FileObjectInfo[obj->ID].ModelID];

        DrawModel(model, &obj->Mat, &obj->Pos, MODEL_PLAIN);

        // Special for copter
        if (obj->ID == OBJECT_TYPE_COPTER) {
            DrawBoundingBox(
                obj->Pos.v[X] - ((float)obj->Flag[0] * 10), 
                obj->Pos.v[X] + ((float)obj->Flag[0] * 10), 
                obj->Pos.v[Y] - ((float)obj->Flag[1] * 10) - ((float)obj->Flag[3] * 50), 
                obj->Pos.v[Y] + ((float)obj->Flag[1] * 10) - ((float)obj->Flag[3] * 50),
                obj->Pos.v[Z] - ((float)obj->Flag[2] * 10), 
                obj->Pos.v[Z] + ((float)obj->Flag[2] * 10),
                0xff0000, 0x00ff00, 0x0000ff, 0x00ffff, 0xff00ff, 0xffff00);
        }

        // Special for water / fog box
        if (obj->ID == OBJECT_TYPE_WATERBOX || obj->ID == OBJECT_TYPE_FOGBOX) {
            DrawBoundingBox(
                obj->Pos.v[X] - ((float)obj->Flag[0]), 
                obj->Pos.v[X] + ((float)obj->Flag[0]), 
                obj->Pos.v[Y] - ((float)obj->Flag[1]), 
                obj->Pos.v[Y] + ((float)obj->Flag[1]),
                obj->Pos.v[Z] - ((float)obj->Flag[2]), 
                obj->Pos.v[Z] + ((float)obj->Flag[2]),
                0xff0000, 0x00ff00, 0x0000ff, 0x00ffff, 0xff00ff, 0xffff00);
        }

// draw axis?

        if (obj == CurrentFileObject)
        {
            if (FileObjectAxisType)
                DrawAxis(&IdentityMatrix, &obj->Pos);
            else
                DrawAxis(&CAM_MainCamera->WMatrix, &obj->Pos);
        }
        else
        {
            MAT mat;
            CopyMat(&IdentityMatrix, &mat);
            VecMulScalar(&mat.mv[R], 0.5f);
            VecMulScalar(&mat.mv[U], 0.5f);
            VecMulScalar(&mat.mv[L], 0.5f);
            DrawAxis(&IdentityMatrix, &obj->Pos);
        }
    }
}

///////////////////////////////////
// display info on a file object //
///////////////////////////////////

void DisplayFileObjectInfo(EDIT_OBJECT *obj)
{
    short y;
    long i;
    WCHAR buf[128];
    FILE_OBJECT_INFO *info = &FileObjectInfo[obj->ID];

// name

	swprintf( buf, L"%S", info->ObjName);
    DumpText(400, 0, 8, 16, 0xffff00, buf);

// flags

    y = 24;
    for (i = 0 ; i < FILE_OBJECT_FLAG_NUM ; i++)
    {

// used?

        if (!info->FlagInfo[i].Name)
            continue;

// yep, display flag name and setting

        if (!info->FlagInfo[i].Type)
            swprintf(buf, L"%S: %d", info->FlagInfo[i].Name, obj->Flag[i]);
        else
            swprintf(buf, L"%S: %S", info->FlagInfo[i].Name, info->FlagInfo[i].Type[obj->Flag[i]]);

        DumpText(400, y, 8, 16, 0x0000ff, buf);

// inc y pos

        y += 24;
    }

// axis

    swprintf(buf, L"Axis %S - %S", FileObjectAxisNames[FileObjectAxis], FileObjectAxisTypeNames[FileObjectAxisType]);
    DumpText(400, y, 8, 16, 0xff00ff, buf);

// selection

    DumpText(376, CurrentFileObjectFlag * 24 + 24, 8, 16, 0xff0000, L"->");
}

/////////////////////////////
// load edit object models //
/////////////////////////////

void LoadFileObjectModels(void)
{
    long i;

// count models

    FileObjectModelNum = 0;
    while (FileObjectModelList[FileObjectModelNum])
        FileObjectModelNum++;

// alloc ram for models

    FileObjectModels = (MODEL*)malloc(sizeof(MODEL) * FileObjectModelNum);
    
// load models

    for (i = 0 ; i < FileObjectModelNum ; i++)
    {
        LoadModel(FileObjectModelList[i], &FileObjectModels[i], 0, 1, LOADMODEL_OFFSET_TPAGE, 100);
    }
}

/////////////////////////////
// free edit object models //
/////////////////////////////

void FreeFileObjectModels(void)
{
    long i;

// free models

    for (i = 0 ; i < FileObjectModelNum ; i++)
    {
        FreeModel(&FileObjectModels[i], 1);
    }

// free ram

    free(FileObjectModels);
}

