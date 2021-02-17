//-----------------------------------------------------------------------------
// File: object.h
//
// Desc: Object processing code
//
// Re-Volt (Generic) Copyright (c) Probe Entertainment 1998
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef OBJECT_H
#define OBJECT_H

#ifdef _N64
#include "ISound.h"
#endif

#include "body.h"
#include "replay.h"
#include "sfx.h" //$ADDITION


// Forward decalartions 
struct PlayerStruct;
struct LightStruct;
#ifndef _PSX
struct SparkGenStruct;
#endif
struct ForceFieldStruct;
struct AnimationDataStruct;
struct CtrlStruct;
struct CollGridStruct;
//$ADDITION_BEGIN
class CSoundEffectInstance;
//$ADDITION_END

//
// Defines and macros
//

#if defined(_PC)
#define MAX_OBJECTS 128
#elif defined(_N64)
#define MAX_OBJECTS 80
#else
#define MAX_OBJECTS 24
#endif

//
// Typedefs and structures
//

#ifdef _PC

typedef  WORD  GLOBAL_ID;
const GLOBAL_ID INVALID_GLOBAL_ID = -1;

typedef struct {

    long    NewData;

    VEC     Pos;                // Probably should be passed as full vector
    VEC     Vel;                // Could be passed as three signed shorts
    VEC     AngVel;             // Also three signed shorts
    QUATERNION  Quat;           // Four signed chars should do it

    DWORD   Time;               // senders race time

} OBJECT_REMOTE_DATA;


#include <pshpack1.h> // one-byte alignment for structs sent over network frequently

typedef struct {

    VEC             Pos;                        // car pos
    short           VelX, VelY, VelZ;           // car vel
    short           AngVelX, AngVelY, AngVelZ;  // car ang vel
    char            QuatX, QuatY, QuatZ, QuatW; // car orientation

    DWORD           Time;                       // race time

    GLOBAL_ID       GlobalID;                   // global ID of object

} OBJECT_REMOTE_DATA_SEND;

#include <poppack.h>

#endif

// Handler types

typedef void (*CON_HANDLER)(struct CtrlStruct *Control, void *Object);      // Typedef for control handling functions
typedef void (*CTRL_HANDLER)(struct PlayerStruct *player);      // Typedef for hardware reading functions
typedef void (*MOVE_HANDLER)(void *Object);                     // Typedef for object movement functions
typedef void (*COLL_HANDLER)(void *Object);
typedef void (*AI_HANDLER)(void *Object);
#ifdef _PSX
typedef void (*RENDER_HANDLER)(void *Object, long *OT, MATRIX *Cam, VECTOR *CamPos);
#else
typedef void (*RENDER_HANDLER)(void *Object);
#endif
typedef void (*FREE_HANDLER)(void *Object);
typedef void (*REMOTE_HANDLER)(void *Object);                   // Typedef for remote data transmitting

// objflags - main object flag structure for primary functions

struct objflags
{
    unsigned long
    Draw      : 1,          // Object render off / on
    Move      : 1,          // Object movement off / on
    IsInGrid  : 1;          // Is object is in grid list
};


// renderflags - flags for controlling aspects of rendering

struct renderflags
{
    unsigned long
    envmap : 1,
    envgood : 1,
    envonly : 1,
    light : 1,
    litsimple : 1,
    reflect : 1,
    fog : 1,
    visible : 1,
    glare : 1,
    meshfx : 1,
    shadow : 1;
};

typedef struct AnimationStateStruct {
    unsigned long StartTime;                        // Start time of the animation (ms relative to TotalRacePhysicsTime)
    long CurrentPosKey;
    long CurrentQuatKey;
    long CurrentScaleKey;
} ANIMATION_STATE;

//
// Main OBJECT definition
//

typedef struct object_def
{
    NEWBODY             body;

    long                Type;                       // Object's type
    long                priority;                   // Object's priority
    long                ObjID;

    struct objflags     flag;
    struct renderflags  renderflag;

#ifndef _PSX
    REAL                EnvOffsetX;
    REAL                EnvOffsetY;
    REAL                EnvScale;
#endif

    struct PlayerStruct *player;
#ifndef _PSX
    struct object_def   *objref;                    // Used for objects that follow or flee other objects
    struct object_def   *creator;                   // Object's creator
#endif

    REAL                CamLength;                  // Scaling factor of camera pole

    REPLAY_STORE_INFO   ReplayStoreInfo;            // information on the last stored data for the object

#ifndef _PSX
    struct object_def   *carcoll[MAX_NUM_PLAYERS];  // Object pointer array for any car-to-car collisions
    struct object_def   *objcoll;                   // Object pointer for a non-car object collision
#endif

    long                EnvRGB;                     // envmap RGB
    long                DefaultModel;               // default model index
    long                CollType;                   // Type of collision object has (for detection)

    struct ForceFieldStruct     *Field;                     // Force field attached to object
    long                FieldPriority;

#ifdef _N64
    SPHERE              Sphere;                     // Collskin sphere, if needed
#endif
#ifndef _PSX
    struct LightStruct  *Light;                     // light source
    struct LightStruct  *Light2;                    // light source 2!
#endif
#ifndef _PSX
#ifndef _CARCONV
    struct SparkGenStruct       *SparkGen;          // Spark generator

#ifndef _PSX
 #ifdef OLD_AUDIO
    SAMPLE_3D           *Sfx3D;                     // 3D sfx ptr
 #else // !OLD_AUDIO
    CSoundEffectInstance* pSfxInstance;
 #endif // !OLD_AUDIO
#endif
#endif
#endif

#ifndef _PSX
    long                OutOfBounds;                // TRUE if object in out of bounds trigger
#endif

    void                *Data;                      // ptr to alloc'd memory

    MOVE_HANDLER        movehandler;                // Function that handles movement of object
    COLL_HANDLER        collhandler;                // Function to handle collision response for this object
    AI_HANDLER          aihandler;                  // Function to handle ai for this object
    RENDER_HANDLER      renderhandler;              // Function to handle rendering for this object
    FREE_HANDLER        freehandler;                // Function to deallocate any allocated ram
    REPLAY_HANDLER      replayhandler;              // replay handler - store or restore replay data

#ifdef _PC
    REMOTE_HANDLER      remotehandler;              // remote handler - transmit data to remote players
    OBJECT_REMOTE_DATA  RemoteData[3];              // Most recent remote data recieved from the network
    long                ServerControlled;           // Whether object is controlled by the server
    int                 OldDat, Dat, NewDat;
    int                 RemoteNStored;
    GLOBAL_ID           GlobalID;                   // multiplayer global ID
#endif

    // storage of default handlers so that they can be changed if they aren't moving
    MOVE_HANDLER        defaultmovehandler;
    COLL_HANDLER        defaultcollhandler;

#ifdef _PC
    struct AnimationDataStruct *AnimData;           // pointer to animation data
    ANIMATION_STATE     AnimState;                  // current animation state
#endif

    struct object_def   *prev;                      // Linked list pointers
    struct object_def   *next;

#ifndef _PSX
    struct CollGridStruct   *Grids[4];              // grids that the player must check for collision in (or NULL)
    long                    GridIndex;              // index of the grid that the player actually occupies
    struct object_def       *PrevGridObj;           // Previous object in this object's grid (grid[0])
    struct object_def       *NextGridObj;           // Next  "  "   "    "   "
#endif

} OBJECT;

///
// External global variables
//

extern OBJECT *OBJ_ObjectList;
extern OBJECT *OBJ_ObjectHead;
extern OBJECT *OBJ_ObjectTail;
extern long OBJ_NumObjects;
extern long ReplayID;
extern long MinReplayID;
#ifdef _PC
extern DWORD GlobalIDCounter; // for generating unique GlobalID values
#endif
extern BBOX OutOfBoundsBox;

//
// External function prototypes
//


extern long     OBJ_InitObjSys(void);
extern void     OBJ_KillObjSys(void);
extern OBJECT*  OBJ_AllocObject(void);
extern OBJECT*  OBJ_ReplaceObject(void);
extern long     OBJ_FreeObject(OBJECT *Obj);
extern void     OBJ_KillAllObjects();
extern OBJECT*  OBJ_GetObjectWithReplayID(unsigned long replayID);
extern OBJECT*  NextObjectOfType(OBJECT *objStart, long type);
extern void     MoveObjectToHead(OBJECT *obj);

#ifdef _PC
extern GLOBAL_ID GetGlobalID(DWORD playerID);
extern OBJECT* GetObjectFromGlobalID(GLOBAL_ID id);
extern void TransmitRemoteObjectData(void);
extern void InitRemoteObjectData(OBJECT *obj);
extern OBJECT_REMOTE_DATA *NextRemoteObjectData(OBJECT *obj);
extern void SendObjectData(OBJECT *obj);
extern int  ProcessObjectData(void);
#endif //_PC


#endif // OBJECT_H

