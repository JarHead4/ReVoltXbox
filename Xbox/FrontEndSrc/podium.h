//-----------------------------------------------------------------------------
// File: podium.h
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef PODIUM_H
#define PODIUM_H


// Podium position types
typedef enum PodiumPosEnum {
    PODIUM_POS_FIRST,
    PODIUM_POS_SECOND,
    PODIUM_POS_THIRD,
    PODIUM_POS_LOSER,

    PODIUM_NPOS
} PODIUM_POS;

// Podium State
typedef enum PodiumStateEnum {
    PODIUM_STATE_INIT,
    PODIUM_STATE_DROP,
    PODIUM_STATE_DROPWAIT,
    PODIUM_STATE_EXIT,

    PODIUM_NTATES
} PODIUM_STATE;


// Variables for keeping track of the win/lose sequence
typedef struct PodiumVarsStruct {
    
    PODIUM_STATE State;
    REAL        StateTimer;

    int         NCarsToDrop;
    int         CarDropCount;
    REAL        CarDropTimer;

    REAL        RacePosScale;
    bool        DisplayWinPos;

    bool        DropSparkles;

    REAL        FireworkTimer;
    REAL        FireworkMaxTime;

    bool        FadeStarted;

} PODIUM_VARS;


////////////////////////////////////////////////////////////////
//
// External prototypes
//
////////////////////////////////////////////////////////////////

extern void InitPodiumVars();
extern void ProcessWinLoseSequence();


////////////////////////////////////////////////////////////
//
// Externs
//
////////////////////////////////////////////////////////////

extern VEC PodiumCarDropPos[PODIUM_NPOS];
extern PODIUM_VARS PodiumVars;


#endif // PODIUM_H

