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

// macros

#define TRIGGER_GLOBAL_FIRST 1
#define TRIGGER_FRAME_FIRST 2


typedef struct {
    long ID, Flag;
    VEC Pos;
    MAT Matrix;
    float Size[3];
} FILE_TRIGGER;


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

extern void ResetTriggerFlags(long ID);

#ifdef _PC
extern void LoadTriggers(char *file);
#endif


// globals

extern long TriggerNum;
extern TRIGGER *Triggers;

#endif // TRIGGER_H

