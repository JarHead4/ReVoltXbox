//-----------------------------------------------------------------------------
// File: player.h
//
// Desc: Player handling code.
//
// Re-Volt (Generic) Copyright (c) Probe Entertainment 1998
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef PLAYER_H
#define PLAYER_H

#include "ai_car.h"
#include "object.h"
#include "car.h"
#include "replay.h"
#include "ctrlread.h"

//
// Defines and macros
//

typedef enum
{
    PLAYER_NONE = 0,
    PLAYER_LOCAL,
    PLAYER_REMOTE,
    PLAYER_CPU,
    PLAYER_GHOST,
    PLAYER_REPLAY,
    PLAYER_FRONTEND,
    PLAYER_DISPLAY,
    PLAYER_CALCSTATS,

    PLAYER_NTYPES
} PLAYER_TYPE;

#if defined(_PC)
#define MAX_LOCAL_PLAYERS   1
#elif defined(_N64)
#define MAX_LOCAL_PLAYERS   4
#elif defined(_PSX)
#define MAX_LOCAL_PLAYERS   2
#endif

//
// Typedefs and structures
//

// Player structure


typedef struct PlayerStruct
{
    long                    PickupType;         // current pickup type;

    
    long                    Slot;                   // slot number
    PLAYER_TYPE             type;
    struct PlayerStruct    *prev;
    struct PlayerStruct    *next;
    CTRL_TYPE               ctrltype;           // If LOCAL player, selected control method
    CTRL_HANDLER            ctrlhandler;        // Function that handles hardware inputs (keyboard, joystick, etc).
    CON_HANDLER             conhandler;         // Function that handles controller inputs
    CTRL                    controls;
    //CTRL                  oldcontrols;

    //long                  cartype;            // Car type player has selected and is using
    OBJECT                  *ownobj;            // Object that player controls (may not be a car!)

    CAR                     car;                // car structure
    CAR_MODEL               carmodels;          // car models
    CAR_AI                  CarAI;              // Car AI data

    long                    score;              // Bomb tag score or race position
    long                    lastscore;          // Last frame race position
    long                    raceswon;           // Races/games won

    REAL                    PickupCycleDest;    // destination pickup type for the cycling
    REAL                    PickupCycleType;    // current pickup cycling type
    REAL                    PickupCycleSpeed;   // current pickup cycling speed
    long                    PickupNum;          // current pickup num;
    OBJECT                  *PickupTarget;      // target object for missiles etc
    REAL                    PickupTargetTime;   // time current target is selected
    long                    PickupTargetOn;     // player is targetting
    unsigned long           RacePosition;       // Current race position
    unsigned long           RaceFinishTime;     // race finish time or 0
    unsigned long           RaceFinishPos;      // race finish position
    unsigned long           BombTagTimer;       // bomb tag timer

    long                    TrackDirType;       // last track dir icon
    long                    TrackDirCount;      // track dir icon counter

    long                    DisplaySplitTime;   // last split time
    REAL                    DisplaySplitCount;  // split time counter

    REAL                    DinkyTimer;         // 

    unsigned long           AccelerateTimeStamp;// CountdownTimer stamp of first acc press for countdown boost

    char            PlayerName[MAX_PLAYER_NAME];

#ifdef _PC
    long            Spectator;          // player is a spectator
    long            Ready;                      // network game 'ready' flag
//$MODIFIED
//    DPID            PlayerID;
    DWORD           PlayerID;
//$END_MODIFICATIONS
    DWORD           LastPing;
    DWORD           CarPacketCount;
#endif

    long            ValidRailCamNode;           // Set to node number of valid rail camera or -1
    long            LastValidRailCamNode;

} PLAYER;



typedef struct {
    unsigned long Time;
    PLAYER *Player;
} FINISH_ENTRY;

//
// External global variables
//

extern PLAYER   Players[MAX_NUM_PLAYERS];
extern PLAYER   *PLR_PlayerHead, *PLR_PlayerTail;
//extern long       MyPlayerNum;
extern long     NumPlayers, AllPlayersFinished;
extern FINISH_ENTRY FinishTable[];
#ifdef _PC
extern PLAYER *PLR_LocalPlayer;
#else
extern long AllHumansFinished;
#endif
//
// External function prototypes
//

extern void PLR_InitPlayers(void);
extern PLAYER *PLR_CreatePlayer(PLAYER_TYPE Type, CTRL_TYPE CtrlType, CAR_TYPE CarType, VEC *Pos, MAT *Mat);
extern void PLR_KillPlayer(PLAYER *Player);
extern void PLR_KillAllPlayers(void);
extern void PLR_SetPlayerType(PLAYER *player, PLAYER_TYPE type);
extern PLAYER *GetPlayerFromPlayerID(unsigned long id);
extern long PLR_CreateStartupPlayers();
extern void SetPlayerFinished(PLAYER *player, unsigned long time);
extern void ResetAllPlayerHandlersToDefault();
extern void SetAllPlayerHandlersToDrop();



#endif // PLAYER_H

