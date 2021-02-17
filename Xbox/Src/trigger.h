//-----------------------------------------------------------------------------
// File: trigger.h
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef TRIGGER_H
#define TRIGGER_H

#include "car.h"

#ifdef _N64
#include "player.h"
#endif

// macros

#define TRIGGER_GLOBAL_FIRST 1
#define TRIGGER_FRAME_FIRST 2



#ifdef _PSX

typedef struct {
    long ID, Flag;
    VEC Pos;
    MAT Matrix;
    REAL Size[3];
} FILE_TRIGGER;

#endif


typedef struct {
    unsigned long GlobalFirst, FrameStamp;
    long ID, Flag, LocalPlayerOnly;
    REAL Size[3];
    PLANE Plane[3];
    VEC Vector;
    void (*Function)(struct PlayerStruct *player, long flag, long n, PLANE *planes);
} TRIGGER;

typedef struct {
    void (*Func)(struct PlayerStruct *player, long flag, long n, PLANE *planes);
    long LocalPlayerOnly;
} TRIGGER_INFO;

enum {
    TRIGGER_PIANO,
    TRIGGER_SPLIT,
    TRIGGER_TRACK_DIR,
    TRIGGER_CAMCHANGE,
    TRIGGER_AIHOME,
    TRIGGER_CAMSHORTEN,
    TRIGGER_OBJECTTHROWER,
    TRIGGER_GAPCAM,
    TRIGGER_REPOSITION_CAR,

    TRIGGER_NUM
};

// prototypes

extern void FreeTriggers(void);
extern void CheckTriggers(void);

#ifdef _PSX
extern void ResetTriggerFlags(long ID, long playernum );
#else
extern void ResetTriggerFlags(long ID);
#endif

#ifdef _PC
extern void LoadTriggers(char *file);
#endif

#ifdef _PSX
extern void LoadTriggers(char *Filename);
void DummyTrigger( void );
#endif

#ifdef _N64
extern void LoadTriggers(void);
void DummyTrigger(struct PlayerStruct *player, long flag, long n, VEC *vec);
#endif




// globals

extern long TriggerNum;
extern TRIGGER *Triggers;

#endif // TRIGGER_H

