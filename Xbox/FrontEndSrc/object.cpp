//-----------------------------------------------------------------------------
// File: object.cpp
//
// Desc: Object processing code
//
// Re-Volt (Generic) Copyright (c) Probe Entertainment 1998
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"
#include "object.h"
#include "main.h"
#include "model.h"
#include "aerial.h"
#include "timing.h"
#include "field.h"
#ifdef _N64
#include "spark.h"
#endif
#ifdef _PC
#include "Path.h"
#include "Light.h"
#endif
#ifndef _PSX
#include "ctrlread.h"
#endif
#include "obj_init.h"

#include "SoundEffectEngine.h"

//
// Static variables
//

static OBJECT *s_NextFreeObj;

//
// Global variables
//

OBJECT *OBJ_ObjectList;
OBJECT *OBJ_ObjectHead = NULL;
OBJECT *OBJ_ObjectTail = NULL;

long    OBJ_NumObjects;
long ReplayID = 0;
long MinReplayID = 0;
#ifdef _PC
DWORD   GlobalID;
#endif

BBOX OutOfBoundsBox;

// Array giving contact info about object pairs
//PAIRCOLLINFO  OBJ_PairCollInfo[MAX_OBJECTS][MAX_OBJECTS];

//
// Global function prototypes
//

long    OBJ_InitObjSys(void);
void    OBJ_KillObjSys(void);
OBJECT *OBJ_AllocObject(void);
OBJECT *OBJ_ReplaceObject(void);
void    OBJ_KillAllObjects();
long    OBJ_FreeObject(OBJECT *Obj);
OBJECT  *OBJ_GetObjectWithReplayID(unsigned long replayID);

//--------------------------------------------------------------------------------------------------------------------------

//
// OBJ_InitObjsys
//
// Initialises the object processing system
//

long OBJ_InitObjSys(void)
{
    long    ii;

    OBJ_ObjectList = (OBJECT *)malloc(sizeof(OBJECT) * MAX_OBJECTS);
    s_NextFreeObj = OBJ_ObjectList;

    OBJ_ObjectList[0].prev = NULL;                          // Setup first object in linked list
    OBJ_ObjectList[0].next = &(OBJ_ObjectList[1]);

    for (ii = 1; ii < (MAX_OBJECTS - 1); ii++)              // Initialise bulk of object list links
    {
        OBJ_ObjectList[ii].prev = &(OBJ_ObjectList[ii - 1]);
        OBJ_ObjectList[ii].next = &(OBJ_ObjectList[ii + 1]);
    }
                                                            // Initialise last object
    OBJ_ObjectList[MAX_OBJECTS - 1].prev = &(OBJ_ObjectList[MAX_OBJECTS - 2]);
    OBJ_ObjectList[MAX_OBJECTS - 1].next = NULL;

    // initialise object IDs
    for (ii = 0; ii < MAX_OBJECTS; ii++) {
        OBJ_ObjectList[ii].ObjID = ii;
    }

    OBJ_NumObjects = 0;
    OBJ_ObjectHead = NULL;
    OBJ_ObjectTail = NULL;

    ReplayID = 0;
    MinReplayID = 0;

    return(1);          // Success
}

//--------------------------------------------------------------------------------------------------------------------------

//
// OBJ_KillObjSys(void)
//
// Removes resources used by the object processing system
//

void OBJ_KillObjSys(void)
{
    if (OBJ_ObjectList != NULL)
    {
        while (OBJ_ObjectHead)  // free all alive objects
        {
            OBJ_FreeObject(OBJ_ObjectHead);
        }

        free(OBJ_ObjectList);
    }

    OBJ_ObjectList = NULL;
    OBJ_ObjectHead = NULL;
    OBJ_ObjectTail = NULL;
    s_NextFreeObj = NULL;

    OBJ_NumObjects = 0;
}

//--------------------------------------------------------------------------------------------------------------------------

//
// OBJ_AllocObject
//
// Alocates an object from the object buffer
//

OBJECT *OBJ_AllocObject(void)
{
    OBJECT *newobj;
    
    newobj = s_NextFreeObj;
    if (newobj == NULL)
    {
        return(NULL);                                       // Could not allocate object (buffer full)
    }

    s_NextFreeObj = s_NextFreeObj->next;                    // Update free object list
    if (s_NextFreeObj != NULL)
    {
        s_NextFreeObj->prev = NULL;
    }

    newobj->prev = OBJ_ObjectTail;

    if (OBJ_ObjectHead == NULL)
    {
        OBJ_ObjectHead = newobj;                            // newobj is the first to be allocated
    }
    else
    {
        OBJ_ObjectTail->next = newobj;
    }
    OBJ_ObjectTail = newobj;
    
    newobj->next = NULL;

    // Set some defaults
    newobj->player = NULL;
    newobj->flag.IsInGrid = 0;                              // Mark object as not in grid yet
    newobj->Data = NULL;
    newobj->Field = NULL;
    newobj->FieldPriority = FIELD_PRIORITY_MIN;             // Default - affected by all fields
#ifndef _PSX
    newobj->creator = NULL;
    newobj->objref = NULL;
    newobj->OutOfBounds = NULL;
    newobj->SparkGen = NULL;
    newobj->Light = NULL;
    newobj->Light2 = NULL;
  #ifdef OLD_AUDIO
    newobj->Sfx3D = NULL;
  #else // !OLD_AUDIO
    newobj->pSfxInstance = NULL;
  #endif // !OLD_AUDIO
#endif
    newobj->movehandler = NULL;
    newobj->collhandler = NULL;
    newobj->aihandler = NULL;
    newobj->renderhandler = NULL;
    newobj->freehandler = NULL;
    newobj->replayhandler = NULL;
#ifdef _PC
    newobj->remotehandler = NULL;
    newobj->GlobalID.QuadPart = -1;
    newobj->ServerControlled = FALSE;
#endif

    newobj->CamLength = ONE;

    newobj->ReplayStoreInfo.Timer = ZERO;
    newobj->ReplayStoreInfo.ID = ReplayID++;
    newobj->ReplayStoreInfo.Status = 0;
    SetVec(&newobj->ReplayStoreInfo.Pos, LARGEDIST, LARGEDIST, LARGEDIST);

#ifdef _PC
    SetObjectAnimation(newobj, NULL);
#endif

#ifndef _PSX
    newobj->Grids[0] = NULL;
    newobj->Grids[1] = NULL;
    newobj->Grids[2] = NULL;
    newobj->Grids[3] = NULL;
    newobj->PrevGridObj = NULL;
    newobj->NextGridObj = NULL;
    newobj->GridIndex = -1;
#endif

    OBJ_NumObjects++;


#if DEBUG

    if( OBJ_NumObjects > 32 )
    {
        printf( "Too many objects!!!\n" );
        exit(0);
    }   
#endif


    return(newobj);
}

//--------------------------------------------------------------------------------------------------------------------------

//
// OBJ_ReplaceObject(void)
//
// Searches the list of allocated objects for low priority objects
// and returns a pointer to the oldest one it can find.
//
// !MT! Not used - yet. Only required if number of objects required exceeds
//      availble free

OBJECT *OBJ_ReplaceObject(void)
{
    OBJECT *newobj = NULL;
    long    found = 0;

    newobj = OBJ_ObjectTail;                                // Start at the end of the list and work towards "newer" objects

#if 0   /* Examples: */
    while ((!found) && (newobj != NULL))
    {
        switch(newobj->type)
        {
            case TYPE_EYECANDY:
            case TYPE_PICKUP:
            found = 1;
            break;

            default:
            newobj = newobj->prev;
            break
        }
    }

    #ifndef _PSX
    GRD_RemoveObject(newobj);
    #endif

    return(newobj);
#else
    return(NULL);
#endif
}


//--------------------------------------------------------------------------------------------------------------------------

void OBJ_KillAllObjects()
{
    OBJECT *obj, *next;
    
    obj = OBJ_ObjectHead;
    while (obj != NULL) {
        next = obj->next;
        OBJ_FreeObject(obj);
        obj = next;
    }
}


//--------------------------------------------------------------------------------------------------------------------------

//
// OBJ_FreeObject
//
// Frees an allocated object from the object buffer
//

long OBJ_FreeObject(OBJECT *Obj)
{

    // Free ram allocated in object
    if (Obj->freehandler)
    {
        Obj->freehandler(Obj);
    }

#ifndef _PSX

    if (Obj->SparkGen) 
    {
        //FreeSparkGen(Obj->SparkGen);
        Obj->SparkGen = NULL;
    }

 #ifdef OLD_AUDIO
    if (Obj->Sfx3D)
    {
        FreeSfx3D(Obj->Sfx3D);
    }
 #else // !OLD_AUDIO
    if( Obj->pSfxInstance )
    {
        g_SoundEngine.ReturnInstance( Obj->pSfxInstance );
        Obj->pSfxInstance = NULL;
    }
 #endif // !OLD_AUDIO

    if (Obj->Light)
    {
        FreeLight(Obj->Light);
    }

    if (Obj->Light2)
    {
        FreeLight(Obj->Light2);
    }

#endif


    
    if (Obj->Data)
    {
        free(Obj->Data);
        Obj->Data = NULL;
    }

    
    if (Obj->Field)
    {
        RemoveField(Obj->Field);
        Obj->Field = NULL;
    }



    // Free the collision skin if necessary
    FreeCollSkin(&Obj->body.CollSkin);
    if (IsBodySphere(&Obj->body)) {
#ifdef _N64
        if (Obj->body.CollSkin.Sphere != &Obj->Sphere)
#endif      
            {
            free(Obj->body.CollSkin.Sphere);
            }
        Obj->body.CollSkin.Sphere = NULL;
        Obj->body.CollSkin.NSpheres = 0;
    }

#ifndef _PSX
    if (Obj->GridIndex != -1) {
        RemoveObjectFromGrid(Obj, &COL_CollGrid[Obj->GridIndex]);
    }
#endif

    // Nullify all the handlers
    Obj->renderhandler = NULL;
    Obj->movehandler = NULL;
    Obj->collhandler = NULL;
    Obj->defaultmovehandler = NULL;
    Obj->defaultcollhandler = NULL;
    Obj->aihandler = NULL;
    Obj->freehandler = NULL;



    if (Obj->prev != NULL)                                  // Update next and prev pointers of adjacent objects
    {                                                       // to close up list
        (Obj->prev)->next = Obj->next;
    }
    else
    {
        OBJ_ObjectHead = Obj->next;
    }

    if (Obj->next != NULL)
    {
        (Obj->next)->prev = Obj->prev;
    }
    else 
    {
        OBJ_ObjectTail = Obj->prev;
    }


    if (s_NextFreeObj != NULL)                              // Add object to free list
    {
        s_NextFreeObj->prev = Obj;
    }

    Obj->next = s_NextFreeObj;
    Obj->prev = NULL;
    s_NextFreeObj = Obj;
    
    OBJ_NumObjects--;

    return(1);
}

/*
/////////////////////////////////////////////////////////////////////
//
// ClearPairCollInfo:
//
/////////////////////////////////////////////////////////////////////
void ClearThisObjPairInfo(OBJECT *obj2)
{
    OBJECT *obj1;

    for (obj1 = OBJ_ObjectHead; obj1 != NULL; obj1 = obj1->next) {

            ClearPairInfo(obj1, obj2);
    
    }
}

void ClearActivePairInfo()
{
    memset(OBJ_PairCollInfo, 0, sizeof(OBJ_PairCollInfo));
}

void ClearAllPairInfo()
{
    memset(OBJ_PairCollInfo, 0, sizeof(OBJ_PairCollInfo));
}
*/

////////////////////////////////////////////////////////////////
//
// OBJ_GetObjectWithGlobalID(long globalID)
//
////////////////////////////////////////////////////////////////

OBJECT  *OBJ_GetObjectWithReplayID(unsigned long replayID)
{
    OBJECT *obj;

    for (obj = OBJ_ObjectHead; obj != NULL; obj = obj->next) {
        if (obj->ReplayStoreInfo.ID == replayID) return obj;
    }
    return NULL;
}

#ifdef _PC

///////////////////////////////
// get object from global ID //
///////////////////////////////

OBJECT *GetObjectFromGlobalID(LARGE_INTEGER id)
{
    OBJECT *obj;

// find object

    for (obj = OBJ_ObjectHead ; obj ; obj = obj->next)
    {
        if (obj->GlobalID.QuadPart == id.QuadPart)
            return obj;
    }

// return none

    return NULL;
}

////////////////////////////////////////////////
// transmit all object data to remote players //
////////////////////////////////////////////////

void TransmitRemoteObjectData(void)
{
    OBJECT *obj;

// send position?

    if (NextPositionReady)
    {
        SendPosition();
    }

// time?

    if (!NextPacketReady)
        return;

#ifndef XBOX_DISABLE_NETWORK //$NOTE: not supporting dynamic packets/sec rate right now...
// adjust pps?

    long queue = GetSendQueueLength();  //$MODIFIED: was originally GetSendQueue(0)

    if (queue > 6 && PacketsPerSecond > 1)
        PacketsPerSecond--;

    if (!queue && PacketsPerSecond < 6)
        PacketsPerSecond++;

// cancel all?

    if (queue > 16)
    {
        DP->CancelMessage(0, 0);
    }
#endif // !XBOX_DISABLE_NETWORK

// process all

    for (obj = OBJ_ObjectHead ; obj ; obj = obj->next)
    {
        if (obj->remotehandler)
        {
            obj->remotehandler(obj);
        }
    }

// send message queue

//$MODIFIED
//    SendMessage((MESSAGE_HEADER*)MessageQueue, (short)MessageQueueSize, GroupID);
//    MessageQueueSize = 0;
    TransmitMessageQueue();
//$END_MODIFICATIONS
}

/////////////////////////////
// init object remote data //
/////////////////////////////

void InitRemoteObjectData(OBJECT *obj)
{
    long i;

    obj->OldDat = 0;
    obj->Dat = 1;
    obj->NewDat = 2;
    obj->RemoteNStored = 0;

    for (i = 0 ; i < 3 ; i++)
    {
        obj->RemoteData[i].NewData = FALSE;
        CopyVec(&obj->body.Centre.Pos, &obj->RemoteData[i].Pos);
        CopyVec(&obj->body.Centre.Vel, &obj->RemoteData[i].Vel);
        CopyVec(&obj->body.AngVel, &obj->RemoteData[i].AngVel);
        CopyQuat(&obj->body.Centre.Quat, &obj->RemoteData[i].Quat);
        obj->RemoteData[i].Time = 0;
    }
}

/////////////////////////////////////////
// get new remote data slot for object //
/////////////////////////////////////////

OBJECT_REMOTE_DATA *NextRemoteObjectData(OBJECT *obj)
{
    int tmp;

    // If the newest data has not been used, overwrite it
    if (obj->RemoteData[obj->NewDat].NewData) {
        return &obj->RemoteData[obj->NewDat];
    }

    // Shift data stores and return pointer to oldest for overwriting
    tmp = obj->OldDat;
    obj->OldDat = obj->Dat;
    obj->Dat = obj->NewDat;
    obj->NewDat = tmp;
    obj->RemoteNStored++;

    if (obj->RemoteNStored > 3) obj->RemoteNStored = 3;

    return &obj->RemoteData[tmp];
}

/////////////////////////////
// send object remote data //
/////////////////////////////

void SendObjectData(OBJECT *obj)
{
    OBJECT_REMOTE_DATA_SEND *data = (OBJECT_REMOTE_DATA_SEND*)(SendHeader + 1);

// setup header

    SendHeader->Type = MESSAGE_OBJECT_DATA;

// set pos

    CopyVec(&obj->body.Centre.Pos, &data->Pos);

// set vel

    data->VelX = (short)(obj->body.Centre.Vel.v[X] * REMOTE_VEL_SCALE);
    data->VelY = (short)(obj->body.Centre.Vel.v[Y] * REMOTE_VEL_SCALE);
    data->VelZ = (short)(obj->body.Centre.Vel.v[Z] * REMOTE_VEL_SCALE);

// set ang vel

    data->AngVelX = (short)(obj->body.AngVel.v[X] * REMOTE_ANGVEL_SCALE);
    data->AngVelY = (short)(obj->body.AngVel.v[Y] * REMOTE_ANGVEL_SCALE);
    data->AngVelZ = (short)(obj->body.AngVel.v[Z] * REMOTE_ANGVEL_SCALE);

// set quat

    data->QuatX = (char)(obj->body.Centre.Quat.v[VX] * REMOTE_QUAT_SCALE);
    data->QuatY = (char)(obj->body.Centre.Quat.v[VY] * REMOTE_QUAT_SCALE);
    data->QuatZ = (char)(obj->body.Centre.Quat.v[VZ] * REMOTE_QUAT_SCALE);
    data->QuatW = (char)(obj->body.Centre.Quat.v[S] * REMOTE_QUAT_SCALE);

// set time

    data->Time = TotalRaceTime;

// set global ID

    data->GlobalID = obj->GlobalID;

// send

    QueueMessage(SendHeader, sizeof(MESSAGE_HEADER) + sizeof(OBJECT_REMOTE_DATA_SEND));
}

/////////////////////
// get object data //
/////////////////////

void ProcessObjectData(void)
{
    OBJECT *obj;
    OBJECT_REMOTE_DATA_SEND *data = (OBJECT_REMOTE_DATA_SEND*)(ReceiveHeader + 1);
    OBJECT_REMOTE_DATA *rem;

// get relevant object

    obj = GetObjectFromGlobalID(data->GlobalID);
    if (!obj)
        return;

// get remote data struct to fill

    rem = NextRemoteObjectData(obj);

// get pos

    CopyVec(&data->Pos, &rem->Pos);

// get vel

    rem->Vel.v[X] = data->VelX / REMOTE_VEL_SCALE;
    rem->Vel.v[Y] = data->VelY / REMOTE_VEL_SCALE;
    rem->Vel.v[Z] = data->VelZ / REMOTE_VEL_SCALE;

// get ang vel

    rem->AngVel.v[X] = data->AngVelX / REMOTE_ANGVEL_SCALE;
    rem->AngVel.v[Y] = data->AngVelY / REMOTE_ANGVEL_SCALE;
    rem->AngVel.v[Z] = data->AngVelZ / REMOTE_ANGVEL_SCALE;

// get quat

    rem->Quat.v[VX] = (REAL)data->QuatX / REMOTE_QUAT_SCALE;
    rem->Quat.v[VY] = (REAL)data->QuatY / REMOTE_QUAT_SCALE;
    rem->Quat.v[VZ] = (REAL)data->QuatZ / REMOTE_QUAT_SCALE;
    rem->Quat.v[S] = (REAL)data->QuatW / REMOTE_QUAT_SCALE;
    NormalizeQuat(&rem->Quat);

// get time

    rem->Time = data->Time;

// set new data

    rem->NewData = TRUE;
}

#endif


////////////////////////////////////////////////////////////////
//
// Get the next object of the specified type. Start looking
// from the passed object.
//
////////////////////////////////////////////////////////////////

OBJECT *NextObjectOfType(OBJECT *objStart, long type)
{
    OBJECT *obj;

    for (obj = objStart; obj != NULL; obj = obj->next) {

        if (obj->Type == type) {
            return obj;
        }
    }

    return NULL;
}

////////////////////////////////////////////////////////////////
//
// Move Object to top of list:
// use this to keep the objects which allow object collisions
// at the head of the object list
//
////////////////////////////////////////////////////////////////

void MoveObjectToHead(OBJECT *obj)
{
    Assert (obj != NULL);

    // Remove object from the list
    if (obj->prev != NULL) {
        (obj->prev)->next = obj->next;
    } else {
        // Already head of list
        return;
    }
    if (obj->next != NULL) {
        (obj->next)->prev = obj->prev;
    } else {
        OBJ_ObjectTail = obj->prev;
    }

    // Insert at head of list
    if (OBJ_ObjectHead != NULL) {
        OBJ_ObjectHead->prev = obj;
    }
    obj->next = OBJ_ObjectHead;
    obj->prev = NULL;
    OBJ_ObjectHead = obj;

}


