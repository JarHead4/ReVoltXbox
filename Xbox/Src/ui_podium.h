//-----------------------------------------------------------------------------
// File: ui_podium.h
//
// Desc: UI implementation
//
// Re-Volt (Generic) Copyright (c) Probe Entertainment 1998
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef PODIUM_H
#define PODIUM_H


// Podium position types
enum PODIUM_POS
{
    PODIUM_POS_FIRST,
    PODIUM_POS_SECOND,
    PODIUM_POS_THIRD,
    PODIUM_POS_LOSER,

    PODIUM_NPOS
};

// Podium State
enum PODIUM_STATE
{
    PODIUM_STATE_INIT,
    PODIUM_STATE_DROP,
    PODIUM_STATE_DROPWAIT,
    PODIUM_STATE_EXIT,

    PODIUM_NTATES
};


// Variables for keeping track of the win/lose sequence
struct PODIUM_VARS
{
    PODIUM_STATE State;
    FLOAT        StateTimer;

    int          NCarsToDrop;
    int          CarDropCount;
    FLOAT        CarDropTimer;

    FLOAT        RacePosScale;
    bool         DisplayWinPos;

    bool         DropSparkles;

    FLOAT        FireworkTimer;
    FLOAT        FireworkMaxTime;

    bool         FadeStarted;
};


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

