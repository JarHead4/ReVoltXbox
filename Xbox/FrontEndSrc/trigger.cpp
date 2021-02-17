//-----------------------------------------------------------------------------
// File: trigger.cpp
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"

#include "trigger.h"
#include "main.h"
#include "geom.h"
#include "car.h"
#include "ctrlread.h"
#include "object.h"
#include "control.h"
#include "player.h"
#include "ai.h"
#include "obj_init.h"

#include "gameloop.h"

#include "ai_car.h"
#include "camera.h"
#include "piano.h"
#include "panel.h"

// globals


void TriggerObjectThrower(PLAYER *player, long flag, long n, PLANE *planes);
void TriggerGapCamera(PLAYER *player, long flag, long n, PLANE *planes);
void TriggerRepositionCar(PLAYER *player, long flag, long n, PLANE *planes);

long TriggerNum;

TRIGGER *Triggers;

// trigger info - handler, local only

#ifndef _PSX
static TRIGGER_INFO TriggerInfo[] = {
    TriggerPiano, FALSE,
    TriggerSplit, TRUE,
    TriggerTrackDir, TRUE,
    TriggerCamera, FALSE,
    CAI_TriggerAiHome, FALSE,
    TriggerCameraShorten, FALSE,
    TriggerObjectThrower, FALSE,
#ifdef _PC
    TriggerGapCamera, FALSE,                    // GAP-style time slow-down and camera rotate
#else
    DummyTrigger, FALSE,
#endif
    TriggerRepositionCar, FALSE,
};
#endif


///////////////////
// load triggers //
///////////////////
#ifdef _PC
void LoadTriggers(char *file)
{
    long i;
    REAL time;
    FILE *fp;
    FILE_TRIGGER ftri;
    VEC vec;

// zero misc

    TriggerNum = 0;
    Triggers = NULL;

// open trigger file

    fp = fopen(file, "rb");
    if (!fp)
    {
        return;
    }

// read trigger num

    fread(&TriggerNum, sizeof(TriggerNum), 1, fp);
    if (!TriggerNum)
    {
        fclose(fp);
        return;
    }

// alloc ram for triggers

    Triggers = (TRIGGER*)malloc(sizeof(TRIGGER) * TriggerNum);
    if (!Triggers)
    {
        fclose(fp);
        DumpMessage(NULL, "Can't alloc memory for triggers!");
        g_bQuitGame = TRUE;
        return;
    }

// load and convert each trigger

    for (i = 0 ; i < TriggerNum ; i++)
    {

// read file zone

        fread(&ftri, sizeof(ftri), 1, fp);

// set misc

        Triggers[i].ID = ftri.ID;
        Triggers[i].Flag = ftri.Flag;
        Triggers[i].GlobalFirst = TRUE;
        Triggers[i].Function = TriggerInfo[ftri.ID].Func;
        Triggers[i].LocalPlayerOnly = TriggerInfo[ftri.ID].LocalPlayerOnly;

// set XYZ size

        Triggers[i].Size[0] = ftri.Size[0];
        Triggers[i].Size[1] = ftri.Size[1];
        Triggers[i].Size[2] = ftri.Size[2];

// build 3 planes

        Triggers[i].Plane[0].v[A] = ftri.Matrix.m[RX];
        Triggers[i].Plane[0].v[B] = ftri.Matrix.m[RY];
        Triggers[i].Plane[0].v[C] = ftri.Matrix.m[RZ];
        Triggers[i].Plane[0].v[D] = -DotProduct(&ftri.Matrix.mv[R], &ftri.Pos);

        Triggers[i].Plane[1].v[A] = ftri.Matrix.m[UX];
        Triggers[i].Plane[1].v[B] = ftri.Matrix.m[UY];
        Triggers[i].Plane[1].v[C] = ftri.Matrix.m[UZ];
        Triggers[i].Plane[1].v[D] = -DotProduct(&ftri.Matrix.mv[U], &ftri.Pos);

        Triggers[i].Plane[2].v[A] = ftri.Matrix.m[LX];
        Triggers[i].Plane[2].v[B] = ftri.Matrix.m[LY];
        Triggers[i].Plane[2].v[C] = ftri.Matrix.m[LZ];
        Triggers[i].Plane[2].v[D] = -DotProduct(&ftri.Matrix.mv[L], &ftri.Pos);

// set vec?

        vec.v[Y] = 0;

        if (Triggers[i].ID == TRIGGER_AIHOME)
        {
            if (Triggers[i].Flag < 8)
            {
                time = (float)Triggers[i].Flag / 8.0f;
                vec.v[X] = -Triggers[i].Size[X];
                vec.v[Z] = -Triggers[i].Size[Z] + Triggers[i].Size[Z] * time * 2;
            }
            else if (Triggers[i].Flag < 16)
            {
                time = (float)(Triggers[i].Flag - 8) / 8.0f;
                vec.v[X] = -Triggers[i].Size[X] + Triggers[i].Size[X] * time * 2;
                vec.v[Z] = Triggers[i].Size[Z];
            }
            else if (Triggers[i].Flag < 24)
            {
                time = (float)(Triggers[i].Flag - 16) / 8.0f;
                vec.v[X] = Triggers[i].Size[X];
                vec.v[Z] = Triggers[i].Size[Z] - Triggers[i].Size[Z] * time * 2;
            }
            else if (Triggers[i].Flag < 32)
            {
                time = (float)(Triggers[i].Flag - 24) / 8.0f;
                vec.v[X] = Triggers[i].Size[X] - Triggers[i].Size[X] * time * 2;
                vec.v[Z] = -Triggers[i].Size[Z];
            }

            RotTransVector(&ftri.Matrix, &ftri.Pos, &vec, &Triggers[i].Vector);
        }
    }

// close file

    fclose(fp);
}
#endif


///////////////////
// free triggers //
///////////////////

void FreeTriggers(void)
{
    free(Triggers);
}

////////////////////
// check triggers //
////////////////////


#ifndef _PSX

void CheckTriggers(void)
{
    long i, k, skip, flag;
    REAL dist;
    TRIGGER *trigger;
    CAR *car;
    VEC *pos;
    PLAYER *player;

// loop thru all triggers

    trigger = Triggers;
    for (i = 0 ; i < TriggerNum ; i++, trigger++) if (trigger->ID < TRIGGER_NUM)
    {

// loop thru players
        

        for (player = PLR_PlayerHead ; player != NULL ; player = player->next)
        {
            car = &player->car;

            if (trigger->LocalPlayerOnly && player->type != PLAYER_LOCAL)
                continue;

// inside trigger?

            pos = &car->Body->Centre.Pos;

            skip = FALSE;
            for (k = 0 ; k < 3 ; k++)
            {
                dist = PlaneDist(&trigger->Plane[k], pos);
                if (dist < -trigger->Size[k] || dist > trigger->Size[k])
                {
                    skip = TRUE;
                    break;
                }
            }

// yep

            if (!skip)
            {
                flag = 0;
                if (trigger->GlobalFirst) flag |= TRIGGER_GLOBAL_FIRST;
                if (trigger->FrameStamp != FrameCount) flag |= TRIGGER_FRAME_FIRST;

                if (trigger->Function) trigger->Function(player, flag, trigger->Flag, trigger->Plane);
            
                trigger->FrameStamp = FrameCount;
                trigger->GlobalFirst = FALSE;
            }
        }
    }
}


#else


void CheckTriggers(void)
{
    long i, k, skip, flag;
    REAL dist;
    TRIGGER *trigger;
    CAR *car;
    VEC *pos;
    PLAYER *player;


// loop thru all triggers


    trigger = Triggers;
    for (i = 0 ; i < TriggerNum ; i++, trigger++) if (trigger->ID < TRIGGER_NUM)
    {

// loop thru players


        for (player = PLR_PlayerHead ; player != NULL ; player = player->next)
        {
            car = &player->car;

            if (trigger->LocalPlayerOnly && player->type != PLAYER_LOCAL)
                continue;

// inside trigger?

            pos = &car->Body->Centre.Pos;

            skip = FALSE;
            for (k = 0 ; k < 3 ; k++)
            {
                dist = PlaneDist(&trigger->Plane[k], pos);
                if (dist < -trigger->Size[k] || dist > trigger->Size[k])
                {
                    skip = TRUE;
                    break;
                }
            }

// yep

            if (!skip)
            {
                flag = 0;
                if (trigger->GlobalFirst) flag |= TRIGGER_GLOBAL_FIRST;
                if (trigger->FrameStamp != TotalRaceTime) flag |= TRIGGER_FRAME_FIRST;

                if (trigger->Function) trigger->Function(player, flag, trigger->Flag, trigger->Plane);
            
                trigger->FrameStamp = TotalRaceTime;
                trigger->GlobalFirst = FALSE;
            }
        }
    }
}


#endif


/////////////////////////////////////////
// reset trigger flags of a given type //
/////////////////////////////////////////

#ifndef _PSX
void ResetTriggerFlags(long ID)
{
    long i;

    for (i = 0 ; i < TriggerNum ; i++)
    {
        if (Triggers[i].ID == ID)
        {
            Triggers[i].FrameStamp--;
            Triggers[i].GlobalFirst = TRUE;
        }
    }
}

#else
/*
void ResetTriggerFlags(long ID, long playernum)
{
    long i, j;

    j = playernum*TriggerNum;

    for (i = 0 ; i < TriggerNum ; i++)
    {
        if (Triggers[i+j].ID == ID)
        {
            Triggers[i+j].FrameStamp--;
            Triggers[i+j].GlobalFirst = TRUE;
        }
    }
}
*/
#endif

////////////////////////////////////


////////////////////////////////////////////////////////////////
//
// Trigger Object Thrower
//
////////////////////////////////////////////////////////////////

#ifndef _PSX

void TriggerObjectThrower(PLAYER *player, long flag, long n, PLANE *planes)
{
    OBJECT *obj, *newObj;
    OBJECT_THROWER_OBJ *objThrower;

    // Only happens once per level
    if (!(flag & TRIGGER_GLOBAL_FIRST)) return;

    // Find the object associated with the trigger
    for (obj = OBJ_ObjectHead; obj != NULL; obj = obj->next) {

        // Is this an object of the right type?
        if (obj->Type != OBJECT_TYPE_OBJECT_THROWER) continue;

        // Is this the right object?
        objThrower = (OBJECT_THROWER_OBJ*)obj->Data;
        if (objThrower->ID != n) continue;

        // Chuck the object
        if (objThrower->ObjectType < OBJECT_TYPE_MAX) {
            newObj = CreateObject(&obj->body.Centre.Pos, &obj->body.Centre.WMatrix, objThrower->ObjectType, NULL);
        }
        if (newObj != NULL && objThrower->Speed > 0) {
            VecEqScalarVec(&newObj->body.Centre.Vel, objThrower->Speed * 50, &obj->body.Centre.WMatrix.mv[L]);
        }

        return;
    }

}


#endif


////////////////////////////////////////////////////////////////
//
// TriggerGapCamera:
//
////////////////////////////////////////////////////////////////

void TriggerGapCamera(PLAYER *player, long flag, long n, PLANE *planes)
{
#ifdef _PC
    // Only for object looked at by main camera
    if (CAM_MainCamera == NULL) return;
    if (player->ownobj != CAM_MainCamera->Object) {
        return;
    }

    // Only happen every 30 seconds
    if (GLP_GapCameraTimer > TO_TIME(Real(-30))) return;

    // Set the camera sweep variables
    GLP_TriggerGapCamera = TRUE;
    CopyVec(PlaneNormal(&planes[0]), &GLP_GapCameraMat.mv[R]);
    CopyVec(PlaneNormal(&planes[1]), &GLP_GapCameraMat.mv[U]);
    CopyVec(PlaneNormal(&planes[2]), &GLP_GapCameraMat.mv[L]);
#endif
}

////////////////////////////
// trigger reposition car //
////////////////////////////

void TriggerRepositionCar(PLAYER *player, long flag, long n, PLANE *planes)
{

// start reposition

    player->controls.digital |= CTRL_REPOSITION;
#ifndef _PSX
    player->ownobj->OutOfBounds = TRUE;
#endif
}
