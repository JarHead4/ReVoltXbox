//-----------------------------------------------------------------------------
// File: replay.h
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef REPLAY_H
#define REPLAY_H

/////////////////////////////////////////////////////////////////////
//
// Replay:
//
// Replays are stored as a list of events with times and data
// associated with them. The events are stored in a wrapping buffer
// keeping track of the earliest valid event.
//
// Events are stored as:
//
//      unsigned long       Event Time (milliseconds)
//      unsigned long       Event Type (enum)
//      -------------       Data
//      ..  ..  ..  .       Data
//      -------------       Data
//
// with the amount of data stored being dependent on the type of
// event, but always a multiple of four bytes.
//
// When an event is added, but the buffer is full, the earlier 
// events are removed (but adding a new starting event) until there 
// is enough space for the recent event.
//
/////////////////////////////////////////////////////////////////////

struct object_def;
struct PickupStruct;

#if defined(_PC)
#define REPLAY_DATA_SIZE (1024 * 1024)
#elif defined(_N64)
#define REPLAY_DATA_SIZE (50 * 1024) 
#elif defined(_PSX)
#define REPLAY_DATA_SIZE (32 * 1024)
#endif

#define REPLAY_MIN_MOVEMENT             TO_LENGTH(Real(10))         // min movement between position stores
#define REPLAY_PLAYER_POS_STORE_TIME    TO_TIME(Real(3.0))          // time between storing players positions

#define REPLAY_MIN_TIME                 TO_TIME(Real(0.2))          // min time between position stores (for objects)
#define REPLAY_MAX_TIME                 TO_TIME(Real(3.0))          // max time between position stores

/////////////////////////////////////////////////////////////////////
//
// Different types of replay events
//
/////////////////////////////////////////////////////////////////////
enum ReplayEventID {
    REPLAY_EVENT_NONE,
    REPLAY_EVENT_STARTUP,
    REPLAY_EVENT_TERMINATE,
    REPLAY_EVENT_UPDATE_PLAYER_CTRL,
    REPLAY_EVENT_UPDATE_PLAYER_POS,
    REPLAY_EVENT_UPDATE_OBJECT_POS,
    REPLAY_EVENT_CREATE_OBJECT,
    REPLAY_EVENT_DESTROY_OBJECT,
    REPLAY_EVENT_GOTPICKUP,
    REPLAY_EVENT_RESET_CAR,
    REPLAY_EVENT_GENERATE_PICKUP,

    REPLAY_EVENT_NTYPES
};


typedef unsigned char REPLAY_DATA;
typedef void (*REPLAY_EVENT_HANDLER)(REPLAY_DATA *data, unsigned long time);
typedef void (*REPLAY_HANDLER)(struct object_def *obj);

/////////////////////////////////////////////////////////////////////
//
// Info stored for each individual replay event type
//
/////////////////////////////////////////////////////////////////////
typedef struct ReplayUpdatePlayerCtrlDataStruct {
    long ReplayID;
    signed char Dx, Dy;
    unsigned short Digital;
} REPLAY_CTRL_DATA;

typedef struct ReplayUpdateObjectPosDataStruct {
    long ReplayID;
    VEC Pos;
    VEC Vel;
    VEC AngVel;
    QUATERNION Quat;
} REPLAY_OBJPOS_DATA;

typedef struct ReplayCreateObjectDataStruct {
    unsigned long ObjectType;
    long ReplayID;
    unsigned long Flags[4];
    VEC Pos;
    QUATERNION Quat;
} REPLAY_CREATE_DATA;

typedef struct ReplayDestroyObjectDataStruct {
    long ReplayID;
} REPLAY_DESTROY_DATA;

typedef struct ReplayGotPickupDataStruct {
    long ReplayID;
    unsigned long PickupType;
} REPLAY_PICKUP_DATA;

typedef struct ReplayEventResetCar {
    long ReplayID;
} REPLAY_RESET_DATA;

typedef struct ReplayEventGeneratePickupStruct {
    long ReplayID;
} REPLAY_GENPICKIP_DATA;

typedef struct ReplayEventDataStruct {
    REPLAY_EVENT_HANDLER Handler;
    char            *EventName;
    unsigned long   DataSize;
} REPLAY_EVENT_DATA;

/////////////////////////////////////////////////////////////////////
//
// Info stored for each object with replay ability
//
/////////////////////////////////////////////////////////////////////

#define REPLAY_STATUS_INITIALISED   (0x1)

typedef struct ReplayStoreInfoStruct {
    unsigned long ID;
    unsigned long Status;
    REAL Timer;
    VEC Pos;
    VEC InterpPos;
    VEC InterpVel;
    VEC InterpAngVel;
    QUATERNION InterpQuat;
} REPLAY_STORE_INFO;

/////////////////////////////////////////////////////////////////////
//
// Replay event handler declarations
//
/////////////////////////////////////////////////////////////////////
extern void ReplayEventUpdateObjectPos(REPLAY_DATA *data, unsigned long time);
extern void ReplayEventUpdatePlayerCtrl(REPLAY_DATA *data, unsigned long time);
extern void ReplayEventUpdatePlayerPos(REPLAY_DATA *data, unsigned long time);
extern void ReplayEventGeneratePickup(REPLAY_DATA *data, unsigned long time);

extern void PlayerReplayStoreHandler(struct object_def *obj);
extern void DefaultObjectReplayStoreHandler(struct object_def *obj);
extern void ReplayStorePlayerCtrl(struct object_def *obj);
extern void ReplayStoreObjectPos(struct object_def *obj);
extern void ReplayStoreAllObjectPos(struct object_def *obj);
extern void ReplayStoreGotPickup(struct object_def *obj);
extern void ReplayStoreResetCar(struct object_def *obj);
extern void StoreGeneratePickup(struct PickupStruct *pickup);

extern void UpdateBufferStart();

extern void CreateTimerDiffBuffer();
extern void DestroyTimerDiffBuffer();
extern void InitTimerDiffBuffer();
extern void UpdateTimerDiffBuffer();

extern void RPL_CreateReplayBuffer(unsigned long size);
extern void RPL_DestroyReplayBuffer();
extern void RPL_StoreAllObjectReplayData();
extern void RPL_RestoreAllObjectReplayData();
extern void RPL_InitReplayBuffer();
extern void RPL_InitReplay();
extern void RPL_TerminateReplay();

#ifdef _PC
extern bool SaveReplayData(FILE *fp);
extern bool LoadReplayData(FILE *fp);
#endif

extern REPLAY_EVENT_DATA ReplayEventData[REPLAY_EVENT_NTYPES];
extern unsigned char *ReplayDataBuffer;


extern unsigned long ReplayBufferBytesStored;
extern bool RPL_RecordReplay;
extern long ReachedEndOfReplay;

/////////////////////////////////////////////////////////////////////
// use this to move a pointer about in the buffer - keeps checks on
// bounds and wraps.
extern REPLAY_DATA *MoveReplayPointer(REPLAY_DATA *pData, long nBytes);

////////////////////////////////////////////////////////////////
// Store variable of type _dataType in buffer and update pointer
#define StoreReplayData(_dataType, _dataPtr, _data) \
{ \
    Assert(ReplayDataBuffer != NULL); \
    Assert(ReplayDataBufSize != 0); \
    (*((_dataType *)(_dataPtr))) = (_dataType)(_data); \
    (_dataPtr) = MoveReplayPointer((_dataPtr), sizeof(_dataType)); \
    ReplayBufferBytesStored += sizeof(_dataType); \
}

////////////////////////////////////////////////////////////////
// read replay data into _data and shift pointer
#define GetReplayData(_dataType, _dataPtr, _data) \
{ \
    Assert(ReplayDataBuffer != NULL); \
    Assert(ReplayDataBufSize != 0); \
    (_data) = *(_dataType *)(_dataPtr); \
    (_dataPtr) = MoveReplayPointer((_dataPtr), sizeof(_dataType)); \
}
    
////////////////////////////////////////////////////////////////
// return value of type _dataType without updating pointer
#define ReadReplayData(_dataType, _dataPtr) (*(_dataType *)(_dataPtr))



#endif // REPLAY_H

