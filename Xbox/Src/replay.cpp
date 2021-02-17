//-----------------------------------------------------------------------------
// File: replay.cpp
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"
#include "geom.h"
#include "replay.h"
#include "player.h"
#include "timing.h"
#include "object.h"
#include "obj_init.h"
#include "camera.h"
#include "weapon.h"
#include "InitPlay.h"
#include "move.h"
#include "pickup.h"
#include "ui_menu.h"
#include "ui_TitleScreen.h"
#include "ui_StateEngine.h"
#include "ui_InGameMenu.h"

#ifndef _PSX
#include "gameloop.h"
#endif

#ifdef _PC
#include "ghost.h"
//$MODIFIED
//#include "sfx.h"
#include "SoundEffectEngine.h"
//$END_MODIFICATIONS
#include "gamegauge.h"
#endif


unsigned long ReplayBufferBytesStored = 0;
bool sReplayTerminated = FALSE;


////////////////////////////////////////////////////////////////
//
// Prototypes
//
void RPL_CreateReplayBuffer(unsigned long size);
void RPL_DestroyReplayBuffer();

void RPL_StoreAllObjectReplayData();
void RPL_RestoreAllObjectReplayData();

void SetNextReplayEvent(unsigned long event);

void RPL_InitReplay();
void RPL_TerminateReplay();

void ReplayEventNone(REPLAY_DATA *data, unsigned long time);
void ReplayEventStartup(REPLAY_DATA *data, unsigned long time);
void ReplayEventTerminate(REPLAY_DATA *data, unsigned long time);
void ReplayEventUpdateObjectPos(REPLAY_DATA *data, unsigned long time);
void ReplayEventUpdatePlayerCtrl(REPLAY_DATA *data, unsigned long time);
void ReplayEventCreateObject(REPLAY_DATA *data, unsigned long time);
void ReplayEventDestroyObject(REPLAY_DATA *data, unsigned long time);
void ReplayEventGotPickup(REPLAY_DATA *data, unsigned long time);
void ReplayEventResetCar(REPLAY_DATA *data, unsigned long time);

void PlayerReplayStoreHandler(OBJECT *obj);
void DefaultObjectReplayStoreHandler(OBJECT *obj);
void ReplayStorePlayerCtrl(OBJECT *obj);
void ReplayStorePlayerPos(OBJECT *obj);
void ReplayStoreObjectPos(OBJECT *obj);
void ReplayStoreCreateObject(OBJECT *obj);
void ReplayStoreDestroyObject(OBJECT *obj);
void ReplayStoreGotPickup(OBJECT *obj);
void ReplayStoreResetCar(OBJECT *obj);
void StorePositionData(OBJECT *obj);

#ifndef _N64
bool SaveReplayData(FILE *fp);
bool LoadReplayData(FILE *fp);
#endif


////////////////////////////////////////////////////////////////
//
// Static variables
//

REPLAY_DATA *ReplayDataBuffer = NULL;
REPLAY_DATA *ReplayDataStart = NULL;
REPLAY_DATA *ReplayDataEnd = NULL;
REPLAY_DATA *ReplayDataPtr = NULL;

unsigned long ReplayDataBufSize = 0;
unsigned long ReplayStartTime, ReplayEndTime;

long ReachedEndOfReplay = FALSE;

////////////////////////////////////////////////////////////////
//
// Global variables
//

REPLAY_EVENT_DATA ReplayEventData[REPLAY_EVENT_NTYPES] = {
    ReplayEventNone,                "REPLAY_EVENT_NONE",                0,
    ReplayEventStartup,             "REPLAY_EVENT_STARTUP",             0,
    ReplayEventTerminate,           "REPLAY_EVENT_TERMINATE",           0,
    ReplayEventUpdatePlayerCtrl,    "REPLAY_EVENT_UPDATE_PLAYER_CTRL",  sizeof(REPLAY_CTRL_DATA),
    ReplayEventUpdatePlayerPos,     "REPLAY_EVENT_UPDATE_PLAYER_POS",   sizeof(REPLAY_OBJPOS_DATA),
    ReplayEventUpdateObjectPos,     "REPLAY_EVENT_UPDATE_OBJECT_POS",   sizeof(REPLAY_OBJPOS_DATA),
    ReplayEventCreateObject,        "REPLAY_EVENT_CREATE_OBJECT",       sizeof(REPLAY_CREATE_DATA),
    ReplayEventDestroyObject,       "REPLAY_EVENT_DESTROY_OBJECT",      sizeof(REPLAY_DESTROY_DATA),
    ReplayEventGotPickup,           "REPLAY_EVENT_GOT_PICKUP",          sizeof(REPLAY_PICKUP_DATA),
    ReplayEventResetCar,            "REPLAY_EVENT_RESET_CAR",           sizeof(REPLAY_RESET_DATA),
    ReplayEventGeneratePickup,      "REPLAY_EVENT_GENERATE_PICKUP",     sizeof(REPLAY_GENPICKIP_DATA),
};

bool RPL_RecordReplay = FALSE;


////////////////////////////////////////////////////////////////
//
// RPL_StoreAllObjectReplayData:
//
////////////////////////////////////////////////////////////////

void RPL_StoreAllObjectReplayData()
{
    OBJECT *obj, *next;

    if ((!RPL_RecordReplay) || (ReplayDataBuffer == NULL)) return;

    obj = OBJ_ObjectHead;
    while (obj != NULL) {
        next = obj->next;
        if (obj->replayhandler != NULL) {
            obj->replayhandler(obj);
        }
        obj = next;
    }
}


////////////////////////////////////////////////////////////////
//
// RPL_RestoreAllObjectReplayData:
//
////////////////////////////////////////////////////////////////

void RPL_RestoreAllObjectReplayData()
{
    unsigned long rTime, event;

    if (ReplayDataBuffer == NULL) return;
    
    rTime = ReadReplayData(unsigned long, ReplayDataPtr);

    // Read next time in from replay buffer
    while ((!ReachedEndOfReplay) && (rTime <= TotalRacePhysicsTime)) {

        // get the event type from the buffer
        GetReplayData(unsigned long, ReplayDataPtr, rTime);
        GetReplayData(unsigned long, ReplayDataPtr, event);

        Assert(event < REPLAY_EVENT_NTYPES);

        // Call the event handler
        ReplayEventData[event].Handler(ReplayDataPtr, rTime);

        // get the time of the next event
        rTime = ReadReplayData(unsigned long, ReplayDataPtr);
    }

}



////////////////////////////////////////////////////////////////
//
// Create the replay buffer
//
////////////////////////////////////////////////////////////////

void RPL_CreateReplayBuffer(unsigned long size)
{
    ReplayDataBufSize = size - 4;
    ReplayDataBuffer = (REPLAY_DATA *)malloc(size);
}


////////////////////////////////////////////////////////////////
//
// DestroyReplayBuffer:
//
////////////////////////////////////////////////////////////////

void RPL_DestroyReplayBuffer()
{
    free(ReplayDataBuffer);
    ReplayDataBufSize = 0;
    ReplayDataPtr = ReplayDataStart = ReplayDataEnd = ReplayDataBuffer = NULL;
    RPL_RecordReplay = FALSE;
}


////////////////////////////////////////////////////////////////
//
// InitReplayBuffer:
//
////////////////////////////////////////////////////////////////

void RPL_InitReplayBuffer()
{
    Assert(ReplayDataBuffer != NULL);
    Assert(ReplayDataBufSize > 0);

    RPL_RecordReplay = TRUE;
    ReachedEndOfReplay = FALSE;

    // Set up buffer pointers
    ReplayDataStart = ReplayDataEnd = ReplayDataPtr = ReplayDataBuffer;
    ReplayBufferBytesStored = 0;

    // Store the startup replay event
    StoreReplayData(unsigned long, ReplayDataPtr, 0);
    StoreReplayData(unsigned long, ReplayDataPtr, REPLAY_EVENT_STARTUP);

    sReplayTerminated = FALSE;
}


/////////////////////////////////////////////////////////////////////
//
// MoveReplayPointer:
//
/////////////////////////////////////////////////////////////////////

REPLAY_DATA *MoveReplayPointer(REPLAY_DATA *pData, long nBytes)
{
    pData += nBytes;
    if (pData >= ReplayDataBuffer + ReplayDataBufSize) {
        pData -= ReplayDataBufSize;
    } else if (pData < ReplayDataBuffer) {
        pData += ReplayDataBufSize;
    }
    return pData;
}

////////////////////////////////////////////////////////////////
//
// ReplayEventNone:
//
////////////////////////////////////////////////////////////////

void ReplayEventNone(REPLAY_DATA *data, unsigned long time)
{

}

////////////////////////////////////////////////////////////////
//
// ReplayEventStartup:
//
////////////////////////////////////////////////////////////////

void ReplayEventStartup(REPLAY_DATA *data, unsigned long time)
{
    ReachedEndOfReplay = FALSE;
}

////////////////////////////////////////////////////////////////
//
// ReplayEventTerminate:
//
////////////////////////////////////////////////////////////////

void ReplayEventTerminate(REPLAY_DATA *data, unsigned long time)
{
    ReachedEndOfReplay = TRUE;

    // Initiate in game menu, ensure no parent menu present
//$MODIFIED
//	g_pMenuHeader->SetNextMenu( &Menu_InGame);
    g_pMenuHeader->ClearMenuHeader();
    g_InGameMenuStateEngine.MakeActive( NULL);
//$END_MODIFICATIONS

#ifndef _PSX
#ifdef OLD_AUDIO
    StopAllSfx();
#else
    g_SoundEngine.StopAll();
#endif // OLD_AUDIO
#endif
}

////////////////////////////////////////////////////////////////
//
// ReplayEventUpdatePlayerCtrl:
//
////////////////////////////////////////////////////////////////

void ReplayEventUpdatePlayerCtrl(REPLAY_DATA *data, unsigned long time)
{
    unsigned long objID;
    signed char dx, dy;
    unsigned short dig;
    OBJECT *obj;
    CAR_REMOTE_DATA *rem;

    // get the object
    GetReplayData(unsigned long, ReplayDataPtr, objID);
    obj = OBJ_GetObjectWithReplayID(objID);
    if (obj == NULL) {
        ReplayDataPtr = MoveReplayPointer(ReplayDataPtr, sizeof(REPLAY_CTRL_DATA) - sizeof(unsigned long));
        return;
    }

    if (obj->Type != OBJECT_TYPE_CAR) return;

    GetReplayData(signed char, ReplayDataPtr, dx);
    GetReplayData(signed char, ReplayDataPtr, dy);
    GetReplayData(unsigned short, ReplayDataPtr, dig);

    // Store the controls in the remote car info buffer
    rem = &obj->player->car.RemoteData[obj->player->car.NewDat];
    rem->dx = dx;
    rem->dy = dy;
    rem->digital = dig;
    
#if defined(_PC) && defined(USE_DEBUG_ROUTINES)
//  char buf[256];
//  sprintf(buf, "\tObjID: %8d\ndx:  %8d\ndy:  %8d\ndig: %x\n", objID, dx, dy, dig);
//  WriteLogEntry(buf);
#endif
}


////////////////////////////////////////////////////////////////
//
// ReplayEventUpdatePlayerPos:
//
////////////////////////////////////////////////////////////////

void ReplayEventUpdatePlayerPos(REPLAY_DATA *data, unsigned long time)
{
    unsigned long objID;
    OBJECT *obj;
    CAR_REMOTE_DATA *rem, *remOld;

    // get the object
    GetReplayData(unsigned long, ReplayDataPtr, objID);
    obj = OBJ_GetObjectWithReplayID(objID);

    // Make sure object exists and is a car
    if ((obj == NULL) || (obj->Type != OBJECT_TYPE_CAR)) {
        ReplayDataPtr = MoveReplayPointer(ReplayDataPtr, sizeof(REPLAY_OBJPOS_DATA) - sizeof(unsigned long));
        return;
    }

    rem = NextRemoteData(&obj->player->car);
    remOld = &obj->player->car.RemoteData[obj->player->car.Dat];

    // position
    GetReplayData(REAL, ReplayDataPtr, rem->Pos.v[X]);
    GetReplayData(REAL, ReplayDataPtr, rem->Pos.v[Y]);
    GetReplayData(REAL, ReplayDataPtr, rem->Pos.v[Z]);

    // Velocity
    GetReplayData(REAL, ReplayDataPtr, rem->Vel.v[X]);
    GetReplayData(REAL, ReplayDataPtr, rem->Vel.v[Y]);
    GetReplayData(REAL, ReplayDataPtr, rem->Vel.v[Z]);

    // Angular Velocity
    GetReplayData(REAL, ReplayDataPtr, rem->AngVel.v[X]);
    GetReplayData(REAL, ReplayDataPtr, rem->AngVel.v[Y]);
    GetReplayData(REAL, ReplayDataPtr, rem->AngVel.v[Z]);

    // Quaternion
    GetReplayData(REAL, ReplayDataPtr, rem->Quat.v[VX]);
    GetReplayData(REAL, ReplayDataPtr, rem->Quat.v[VY]);
    GetReplayData(REAL, ReplayDataPtr, rem->Quat.v[VZ]);
    GetReplayData(REAL, ReplayDataPtr, rem->Quat.v[S]);

    // Copy Controls from last data
    rem->Time = time;
    rem->dx = remOld->dx;
    rem->dy = remOld->dy;
    rem->digital = remOld->digital;

    rem->NewData = TRUE;

#if defined(_PC) && defined(USE_DEBUG_ROUTINES)
//  char buf[256];
//  sprintf(buf, "\tObjID: %8d\n", objID);
//  WriteLogEntry(buf);
#endif

}

////////////////////////////////////////////////////////////////
//
// ReplayEventUpdateObjectPos:
//
////////////////////////////////////////////////////////////////

void ReplayEventUpdateObjectPos(REPLAY_DATA *data, unsigned long time)
{
    unsigned long objID;
    OBJECT *obj;

    // get the object
    GetReplayData(unsigned long, ReplayDataPtr, objID);
    obj = OBJ_GetObjectWithReplayID(objID);
    if (obj == NULL) {
        ReplayDataPtr = MoveReplayPointer(ReplayDataPtr, sizeof(REPLAY_OBJPOS_DATA) - sizeof(unsigned long));
        return;
    }

    // position
    GetReplayData(REAL, ReplayDataPtr, obj->body.Centre.Pos.v[X]);
    GetReplayData(REAL, ReplayDataPtr, obj->body.Centre.Pos.v[Y]);
    GetReplayData(REAL, ReplayDataPtr, obj->body.Centre.Pos.v[Z]);

    // Velocity
    GetReplayData(REAL, ReplayDataPtr, obj->body.Centre.Vel.v[X]);
    GetReplayData(REAL, ReplayDataPtr, obj->body.Centre.Vel.v[Y]);
    GetReplayData(REAL, ReplayDataPtr, obj->body.Centre.Vel.v[Z]);

    // Angular Velocity
    GetReplayData(REAL, ReplayDataPtr, obj->body.AngVel.v[X]);
    GetReplayData(REAL, ReplayDataPtr, obj->body.AngVel.v[Y]);
    GetReplayData(REAL, ReplayDataPtr, obj->body.AngVel.v[Z]);

    // Quaternion
    GetReplayData(REAL, ReplayDataPtr, obj->body.Centre.Quat.v[VX]);
    GetReplayData(REAL, ReplayDataPtr, obj->body.Centre.Quat.v[VY]);
    GetReplayData(REAL, ReplayDataPtr, obj->body.Centre.Quat.v[VZ]);
    GetReplayData(REAL, ReplayDataPtr, obj->body.Centre.Quat.v[S]);

    QuatToMat(&obj->body.Centre.Quat, &obj->body.Centre.WMatrix);

}


////////////////////////////////////////////////////////////////
//
// ReplayEventCreateObject:
//
// NOTE: dooesn't store flags yet...
//
////////////////////////////////////////////////////////////////

void ReplayEventCreateObject(REPLAY_DATA *data, unsigned long time)
{
    unsigned long objType, newID, flags[4];
    VEC pos;
    MAT mat;
    QUATERNION quat;
    OBJECT *obj;

    // Get object type
    GetReplayData(unsigned long, ReplayDataPtr, objType);

    // Get the replay ID to give the object
    GetReplayData(unsigned long, ReplayDataPtr, newID);
    ReplayID = newID;                                   // this should work because cannot have more objects created in replay then in original game

    // Get the flags
    GetReplayData(unsigned long, ReplayDataPtr, flags[0]);
    GetReplayData(unsigned long, ReplayDataPtr, flags[1]);
    GetReplayData(unsigned long, ReplayDataPtr, flags[2]);
    GetReplayData(unsigned long, ReplayDataPtr, flags[3]);

    // Get the position
    GetReplayData(REAL, ReplayDataPtr, pos.v[X]);
    GetReplayData(REAL, ReplayDataPtr, pos.v[Y]);
    GetReplayData(REAL, ReplayDataPtr, pos.v[Z]);

    // Get the matrix
    GetReplayData(REAL, ReplayDataPtr, quat.v[VX]);
    GetReplayData(REAL, ReplayDataPtr, quat.v[VY]);
    GetReplayData(REAL, ReplayDataPtr, quat.v[VZ]);
    GetReplayData(REAL, ReplayDataPtr, quat.v[S]);

    // Get the quaternion
    QuatToMat(&quat, &mat);

    // Create the object
    obj = CreateObject(&pos, &mat, objType, 0);
    //obj->ReplayStoreInfo.ID = newID;
}

////////////////////////////////////////////////////////////////
//
// ReplayEventDestroyObject:
//
////////////////////////////////////////////////////////////////

void ReplayEventDestroyObject(REPLAY_DATA *data, unsigned long time)
{
    unsigned long objID;
    OBJECT *obj;

    // get the object
    GetReplayData(unsigned long, ReplayDataPtr, objID);
    obj = OBJ_GetObjectWithReplayID(objID);
    if (obj == NULL) {
        ReplayDataPtr = MoveReplayPointer(ReplayDataPtr, sizeof(REPLAY_DESTROY_DATA) - sizeof(unsigned long));
        return;
    }

    OBJ_FreeObject(obj);
}


////////////////////////////////////////////////////////////////
//
// ReplayEventGotPickup:
//
////////////////////////////////////////////////////////////////

void ReplayEventGotPickup(REPLAY_DATA *data, unsigned long time)
{
    unsigned long objID, pickupType;
    OBJECT *obj;

    // get the object
    GetReplayData(unsigned long, ReplayDataPtr, objID);
    obj = OBJ_GetObjectWithReplayID(objID);
    if (obj == NULL) {
        ReplayDataPtr = MoveReplayPointer(ReplayDataPtr, sizeof(REPLAY_PICKUP_DATA) - sizeof(unsigned long));
        return;
    }
    Assert(obj->Type == OBJECT_TYPE_CAR);

    if (obj->Type != OBJECT_TYPE_CAR) return;

    // get the pickup type
    GetReplayData(unsigned long, ReplayDataPtr, pickupType);

    // Set the players pickup
    GivePickupToPlayer(obj->player, pickupType); 

#ifdef _PC
//  char buf[256];
//  sprintf(buf, "%s\nType: %8d\n", ReplayEventData[REPLAY_EVENT_GOTPICKUP].EventName, pickupType);
//  WriteLogEntry(buf);
//  WriteLogEntry("\n");
#endif
}


////////////////////////////////////////////////////////////////
//
// ReplayeEventResetCar: set car to right itself
//
////////////////////////////////////////////////////////////////

void ReplayEventResetCar(REPLAY_DATA *data, unsigned long time)
{
    unsigned long objID;
    OBJECT *obj;

    // get the object
    GetReplayData(unsigned long, ReplayDataPtr, objID);
    obj = OBJ_GetObjectWithReplayID(objID);
    if (obj == NULL) {
        ReplayDataPtr = MoveReplayPointer(ReplayDataPtr, sizeof(REPLAY_RESET_DATA) - sizeof(unsigned long));
        return;
    }

    if (obj->Type != OBJECT_TYPE_CAR) return;

    obj->player->controls.digital |= CTRL_RESET;

}


////////////////////////////////////////////////////////////////
//
// ReplayEventGeneratePickup
//
////////////////////////////////////////////////////////////////

void ReplayEventGeneratePickup(REPLAY_DATA *data, unsigned long time)
{
    unsigned long pickupID;
    PICKUP *pickup;

    // get the pickup
    GetReplayData(unsigned long, ReplayDataPtr, pickupID);
    
    pickup = &PickupArray[pickupID];

    pickup->Mode = PICKUP_STATE_GENERATING;
    pickup->Timer = PICKUP_GEN_TIME;
    CopyVec(&pickup->GenPos, &pickup->Pos);

}


////////////////////////////////////////////////////////////////
//
// SetNextReplayEvent:
//
////////////////////////////////////////////////////////////////

void SetNextReplayEvent(unsigned long event)
{
    unsigned long nextEvent, dataSize, headerSize;
    unsigned long nextEventDataSize, nextEventTime;
    REPLAY_DATA *nextEventPtr;

    // make sure we aren't going to overwrite the start event
    dataSize = ReplayEventData[event].DataSize + sizeof(unsigned long) + sizeof(unsigned long);
    headerSize = sizeof(unsigned long) + sizeof(unsigned long);
    while (ReplayBufferBytesStored + dataSize  > ReplayDataBufSize ) {

        // calculate the number of bytes to move the start event along by
        nextEventPtr = ReplayDataStart + headerSize;
        GetReplayData(unsigned long, nextEventPtr, nextEventTime);
        GetReplayData(unsigned long, nextEventPtr, nextEvent);
        nextEventDataSize = ReplayEventData[nextEvent].DataSize;

        // move the start event
        ReplayBufferBytesStored -= (nextEventDataSize + headerSize + headerSize);
        ReplayDataStart = MoveReplayPointer(ReplayDataStart, nextEventDataSize + headerSize);

        nextEventPtr = ReplayDataStart;
        StoreReplayData(unsigned long, nextEventPtr, nextEventTime);
        StoreReplayData(unsigned long, nextEventPtr, REPLAY_EVENT_STARTUP);
    }

    StoreReplayData(unsigned long, ReplayDataPtr, TotalRacePhysicsTime);
    StoreReplayData(unsigned long, ReplayDataPtr, event);

#if USE_DEBUG_ROUTINES
//  char buf[256];
//  sprintf(buf, "Stored \"%s\" at %dms\n", ReplayEventData[event].EventName, TotalRacePhysicsTime);
//  WriteLogEntry(buf);
#endif // USE_DEBUG_ROUTINES
}


////////////////////////////////////////////////////////////////
//
// PlayerReplayStoreHandler:
//
////////////////////////////////////////////////////////////////
#if TRUE
void PlayerReplayStoreHandler(OBJECT *obj)
{
    VEC dR;

    // Save the controls if they have changed
    if (obj->player->controls.changes != CTRL_CHANGE_NONE) {
        ReplayStorePlayerCtrl(obj);
    }

    // Save the position if necessary
    obj->ReplayStoreInfo.Timer += TimeStep;
    if (obj->ReplayStoreInfo.Timer < TO_TIME(Real(0.5f))) {//REPLAY_PLAYER_POS_STORE_TIME) {
        return;
    }
    obj->ReplayStoreInfo.Timer = ZERO;

    // Make sure the object has actually moved
    VecMinusVec(&obj->body.Centre.Pos, &obj->ReplayStoreInfo.Pos, &dR);
    if ((abs(dR.v[X]) < REPLAY_MIN_MOVEMENT) && (abs(dR.v[Y]) < REPLAY_MIN_MOVEMENT) && (abs(dR.v[Z]) < REPLAY_MIN_MOVEMENT)) {
        return;
    }
    CopyVec(&obj->body.Centre.Pos, &obj->ReplayStoreInfo.Pos);

    // Store the position
    ReplayStorePlayerPos(obj);
}
#else
void PlayerReplayStoreHandler(OBJECT *obj)
{
    VEC dR;

    // Save the controls if they have changed
    if (obj->player->controls.changes != CTRL_CHANGE_NONE) {
        ReplayStorePlayerCtrl(obj);
        ReplayStorePlayerPos(obj);
        obj->ReplayStoreInfo.Timer = ZERO;
        return;
    }

    // Save the position if necessary
    obj->ReplayStoreInfo.Timer += TimeStep;
    if (obj->ReplayStoreInfo.Timer < TO_TIME(Real(1.0f))) {//REPLAY_PLAYER_POS_STORE_TIME) {
        return;
    }
    obj->ReplayStoreInfo.Timer = ZERO;

    // Make sure the object has actually moved
    VecMinusVec(&obj->body.Centre.Pos, &obj->ReplayStoreInfo.Pos, &dR);
    if ((abs(dR.v[X]) < REPLAY_MIN_MOVEMENT) && (abs(dR.v[Y]) < REPLAY_MIN_MOVEMENT) && (abs(dR.v[Z]) < REPLAY_MIN_MOVEMENT)) {
        return;
    }
    CopyVec(&obj->body.Centre.Pos, &obj->ReplayStoreInfo.Pos);

    // Store the position
    ReplayStorePlayerPos(obj);
}
#endif

////////////////////////////////////////////////////////////////
//
// DefaultObjectReplayStoreHandler:
//
////////////////////////////////////////////////////////////////

void DefaultObjectReplayStoreHandler(OBJECT *obj)
{
    VEC dR;

    // Make sure enough time has passed
    obj->ReplayStoreInfo.Timer += TimeStep;
    if (obj->ReplayStoreInfo.Timer < REPLAY_MAX_TIME) {
        if (obj->ReplayStoreInfo.Timer < REPLAY_MIN_TIME) {
            return;
        }

        // Make sure the object has actually moved
        VecMinusVec(&obj->body.Centre.Pos, &obj->ReplayStoreInfo.Pos, &dR);
        if ((abs(dR.v[X]) < REPLAY_MIN_MOVEMENT) && (abs(dR.v[Y]) < REPLAY_MIN_MOVEMENT) && (abs(dR.v[Z]) < REPLAY_MIN_MOVEMENT)) {
            return;
        }
    }
    obj->ReplayStoreInfo.Timer = ZERO;
    CopyVec(&obj->body.Centre.Pos, &obj->ReplayStoreInfo.Pos);

    // Store the position
    ReplayStoreObjectPos(obj);
}

////////////////////////////////////////////////////////////////
//
// ReplayStorePlayerCtrl:
//
////////////////////////////////////////////////////////////////

void ReplayStorePlayerCtrl(OBJECT *obj)
{
    // Store generic replay event info
    SetNextReplayEvent(REPLAY_EVENT_UPDATE_PLAYER_CTRL);

    // Store event-specific info
    StoreReplayData(unsigned long, ReplayDataPtr, obj->ReplayStoreInfo.ID);
    StoreReplayData(signed char, ReplayDataPtr, obj->player->controls.lastdx);
    StoreReplayData(signed char, ReplayDataPtr, obj->player->controls.lastdy);
    StoreReplayData(unsigned short, ReplayDataPtr, obj->player->controls.lastdigital);

}


////////////////////////////////////////////////////////////////
//
// ReplayStorePlayerPos:
//
////////////////////////////////////////////////////////////////

void ReplayStorePlayerPos(OBJECT *obj)
{
    // Store generic replay event info
    SetNextReplayEvent(REPLAY_EVENT_UPDATE_PLAYER_POS);

    // Store event-specific info
    StoreReplayData(unsigned long, ReplayDataPtr, obj->ReplayStoreInfo.ID);
    StorePositionData(obj);
}


/////////////////////////////////////////////////////////////////////
//
// ReplayStoreResetCar:
//
/////////////////////////////////////////////////////////////////////

void ReplayStoreResetCar(OBJECT *obj)
{
    return;
    // Set replay event
//  SetNextReplayEvent(REPLAY_EVENT_RESET_CAR);

    // Store event-specific info
//  StoreReplayData(unsigned long, ReplayDataPtr, obj->ReplayStoreInfo.ID);
}


////////////////////////////////////////////////////////////////
//
// ReplayStoreObjectPos:
//
////////////////////////////////////////////////////////////////

void ReplayStoreObjectPos(OBJECT *obj)
{
    // Store generic replay event info
    SetNextReplayEvent(REPLAY_EVENT_UPDATE_OBJECT_POS);

    // Store event-specific info
    StoreReplayData(unsigned long, ReplayDataPtr, obj->ReplayStoreInfo.ID);
    StorePositionData(obj);
}


////////////////////////////////////////////////////////////////
//
// ReplayStoreCreateObject:
//
////////////////////////////////////////////////////////////////

void ReplayStoreCreateObject(OBJECT *obj)
{
    // Store generic replay event info
    SetNextReplayEvent(REPLAY_EVENT_CREATE_OBJECT);

    // Store event-specific info
    StoreReplayData(unsigned long, ReplayDataPtr, obj->Type);
    StoreReplayData(unsigned long, ReplayDataPtr, obj->ReplayStoreInfo.ID);

    // Flags (not done yet)
    StoreReplayData(unsigned long, ReplayDataPtr, 0);
    StoreReplayData(unsigned long, ReplayDataPtr, 0);
    StoreReplayData(unsigned long, ReplayDataPtr, 0);
    StoreReplayData(unsigned long, ReplayDataPtr, 0);

    // Position
    StoreReplayData(REAL, ReplayDataPtr, obj->body.Centre.Pos.v[X]);
    StoreReplayData(REAL, ReplayDataPtr, obj->body.Centre.Pos.v[Y]);
    StoreReplayData(REAL, ReplayDataPtr, obj->body.Centre.Pos.v[Z]);

    // Quaternion
    StoreReplayData(REAL, ReplayDataPtr, obj->body.Centre.Quat.v[VX]);
    StoreReplayData(REAL, ReplayDataPtr, obj->body.Centre.Quat.v[VY]);
    StoreReplayData(REAL, ReplayDataPtr, obj->body.Centre.Quat.v[VZ]);
    StoreReplayData(REAL, ReplayDataPtr, obj->body.Centre.Quat.v[S]);

}


////////////////////////////////////////////////////////////////
//
// ReplayStoreDestroyObject:
//
////////////////////////////////////////////////////////////////

void ReplayStoreDestroyObject(OBJECT *obj)
{
    // Store generic replay event info
    SetNextReplayEvent(REPLAY_EVENT_CREATE_OBJECT);

    // Store event-specific info
    StoreReplayData(unsigned long, ReplayDataPtr, obj->ReplayStoreInfo.ID);
}


////////////////////////////////////////////////////////////////
//
// ReplayStoreGotPickup:
//
////////////////////////////////////////////////////////////////

void ReplayStoreGotPickup(OBJECT *obj)
{
    // Store generic event info
    SetNextReplayEvent(REPLAY_EVENT_GOTPICKUP);

    // Store event specific data
    StoreReplayData(unsigned long, ReplayDataPtr, obj->ReplayStoreInfo.ID);
    StoreReplayData(unsigned long, ReplayDataPtr, obj->player->PickupType);
}


////////////////////////////////////////////////////////////////
//
// StorePositionData:
//
////////////////////////////////////////////////////////////////

void StorePositionData(OBJECT *obj)
{
    // Position
    StoreReplayData(REAL, ReplayDataPtr, obj->body.Centre.Pos.v[X]);
    StoreReplayData(REAL, ReplayDataPtr, obj->body.Centre.Pos.v[Y]);
    StoreReplayData(REAL, ReplayDataPtr, obj->body.Centre.Pos.v[Z]);

    // Velocity
    StoreReplayData(REAL, ReplayDataPtr, obj->body.Centre.Vel.v[X]);
    StoreReplayData(REAL, ReplayDataPtr, obj->body.Centre.Vel.v[Y]);
    StoreReplayData(REAL, ReplayDataPtr, obj->body.Centre.Vel.v[Z]);

    // Angular Velocity
    StoreReplayData(REAL, ReplayDataPtr, obj->body.AngVel.v[X]);
    StoreReplayData(REAL, ReplayDataPtr, obj->body.AngVel.v[Y]);
    StoreReplayData(REAL, ReplayDataPtr, obj->body.AngVel.v[Z]);

    // Quaternion
    StoreReplayData(REAL, ReplayDataPtr, obj->body.Centre.Quat.v[VX]);
    StoreReplayData(REAL, ReplayDataPtr, obj->body.Centre.Quat.v[VY]);
    StoreReplayData(REAL, ReplayDataPtr, obj->body.Centre.Quat.v[VZ]);
    StoreReplayData(REAL, ReplayDataPtr, obj->body.Centre.Quat.v[S]);

}


////////////////////////////////////////////////////////////////
//
// StoreGeneratePickup
//
////////////////////////////////////////////////////////////////

void StoreGeneratePickup(PICKUP *pickup)
{
    // Store generic replay event info
    SetNextReplayEvent(REPLAY_EVENT_GENERATE_PICKUP);

    // Store event-specific info
    StoreReplayData(unsigned long, ReplayDataPtr, pickup->ID);
}

////////////////////////////////////////////////////////////////
//
// InitReplay:
//
////////////////////////////////////////////////////////////////

void RPL_InitReplay()
{
    unsigned long startTime;
    PLAYER *player;

    // Find the start of the replay buffer
    ReplayDataPtr = ReplayDataStart;

    // Make players into replay players
    for (player = PLR_PlayerHead; player != NULL; player = player->next) {
        if (player->type != PLAYER_GHOST) {
            PLR_SetPlayerType(player, PLAYER_REPLAY);
        }
    }

    // misc
    ReachedEndOfReplay = FALSE;

    // setup the game timers
    startTime = ReadReplayData(unsigned long, ReplayDataStart);

    if (startTime == 0) {
        // If replay start at beginning of race, then drop the cars
        SetAllPlayerHandlersToDrop();
        InitCountDown();
    } else {
        // otherwise run the game for a second to get all objects in roughly the right place
#ifndef _PSX
        startTime += 1000;
#else
        if (startTime < 500) {      // Make sure PSX doesn't start half way through countdown
            startTime = 500;
        } else {
            startTime += 50;
        }
#endif
        InitCountDownDelta(startTime);
    }


}


////////////////////////////////////////////////////////////////
//
// TerminateReplay:
//
////////////////////////////////////////////////////////////////

void RPL_TerminateReplay()
{
    if (sReplayTerminated) return;
    sReplayTerminated = TRUE;

    // Store the location of the terminate event
    ReplayDataEnd = ReplayDataPtr;


    // put a termination event at the current position
    SetNextReplayEvent(REPLAY_EVENT_TERMINATE);
}



////////////////////////////////////////////////////////////////
//
// SaveReplay:
//
////////////////////////////////////////////////////////////////

#ifdef _PC
bool SaveReplayData(FILE *fp)
{
    size_t nWritten;
    unsigned long offset, size;

    RPL_TerminateReplay();

    // Save out the starting data
    if (!ReplayMode) {
        nWritten = fwrite(&StartData, sizeof(StartData), 1, fp);
    } else {
        nWritten = fwrite(&StartDataStorage, sizeof(StartDataStorage), 1, fp);
    }
    if (nWritten != 1) return FALSE; 

    // write out whether pickups were enabled on this track
    //nWritten = fwrite(&StartData.AllowPickups, sizeof(unsigned long), 1, fp);
    //if (nWritten != 1) return FALSE;

    // Size of the replay buffer
    size = ReplayDataBufSize + 4;
    nWritten = fwrite(&size, sizeof(unsigned long), 1, fp);
    if (nWritten != 1) return FALSE;

    // Start of replay data relative to start of buffer
    offset = ReplayDataStart - ReplayDataBuffer;
    nWritten = fwrite(&offset, sizeof(unsigned long), 1, fp);
    if (nWritten != 1) return FALSE;

    // End of replay data relative to start of buffer
    offset = ReplayDataEnd - ReplayDataBuffer;
    nWritten = fwrite(&offset, sizeof(unsigned long), 1, fp);
    if (nWritten != 1) return FALSE;

    // Save out the replay buffer
    nWritten = fwrite(ReplayDataBuffer, size, 1, fp);
    if (nWritten != 1) return FALSE;

    return TRUE;
}
#endif


////////////////////////////////////////////////////////////////
//
// LoadReplay:
//
////////////////////////////////////////////////////////////////

#ifdef _PC
bool LoadReplayData(FILE *fp)
{
    size_t nRead;
    unsigned long bufSize, offset;
    //char buf[256];

    // Load the starting data
    nRead = fread(&StartData, sizeof(StartData), 1, fp);
    if (nRead != 1) return FALSE;

    // Store it to allow restarting of level
    memcpy(&StartDataStorage, &StartData, sizeof(START_DATA));


    // read in whether pickups were enabled on this track
    //nRead = fread(&StartData.AllowPickups, sizeof(unsigned long), 1, fp);
    //if (nRead != 1) return FALSE;

    // Load the size of the buffer
    nRead = fread(&bufSize, sizeof(unsigned long), 1, fp);
    if (nRead != 1) return FALSE;

    // Allocate buffer if necessary
    if (ReplayDataBufSize != bufSize - 4) {
        if (ReplayDataBufSize > 0) RPL_DestroyReplayBuffer();
        RPL_CreateReplayBuffer(bufSize);
        if (ReplayDataBuffer == NULL) return FALSE;
    }

    // Load the start offset
    nRead = fread(&offset, sizeof(unsigned long), 1, fp);
    if (nRead != 1) return FALSE;
    if (offset > bufSize) return FALSE;
    ReplayDataPtr = ReplayDataStart = ReplayDataBuffer + offset;

    // Load the end offset
    nRead = fread(&offset, sizeof(unsigned long), 1, fp);
    if (nRead != 1) return FALSE;
    if (offset > bufSize) return FALSE;
    ReplayDataEnd = ReplayDataBuffer + offset;

    // Load the replay data buffer
    nRead = fread(ReplayDataBuffer, bufSize, 1, fp);
    if (nRead != 1) return FALSE;

    return TRUE;
}
#endif



