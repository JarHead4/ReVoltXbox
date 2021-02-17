//$REVISIT: should this file be named "InitGame" instead?
//-----------------------------------------------------------------------------
// File: InitPlay.h
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef INITPLAY_H
#define INITPLAY_H

#include "revolt.h"
#include "ctrlread.h"
#include "player.h"

////////////////////////////////////////////////////////////////
//
// Player initialisation structure
//
typedef struct {
    PLAYER_TYPE     PlayerType;                 // Type of player (PLAYER_LOCAL etc...)
    long            GridNum;                    // Grid location  //$TODO(cprince): should we rename this to RaceStartIndex again?
    long            CarType;                    // Car choice
    long            Spectator;                  // Is spectator?
    unsigned long   StartTime;                  // Time introduced to game
    CTRL_TYPE       CtrlType;                   // Control type
#ifdef _PC
    DPID            PlayerID;                   // Direct play ID
    long            Cheating;                   // cheating in multiplayer
//$REMOVED - just use CarType
//    char            CarName[CAR_NAMELEN];       // car name for multiplayer
    //$BUG: we need to change all the code to use CarType instead of CarName eventually...
//$END_REMOVAL
#endif
    char            Name[MAX_PLAYER_NAME];      // Players name
} PLAYER_START_DATA;

typedef struct {
    long            LocalPlayerNum;                         // The number of the local player in the start data
    long            PlayerNum;                              // Number of players in game
      //$TODO: should rename this to NumPlayers
    long            LocalNum;                               // !MT! Number of human players in game
    long            GameType;                               // Game type (for PC multiplayer)
    long            Laps;                                   // Number of laps (for PC multiplayer)
    unsigned long   Mirrored;                               // Mirrored track (for PC multiplayer)
    unsigned long   Reversed;                               // Reversed track (for PC multiplayer)
    unsigned long   Seed;                                   // rand seed (for PC multiplayer)
    unsigned long   AllowPickups;                           // pickups allowed (for PC multiplayer)
#ifdef _PC
    char            LevelDir[MAX_PATH];                     // Level directory name
#endif
    PLAYER_START_DATA PlayerData[MAX_NUM_PLAYERS];          // Starting players info
} START_DATA;




////////////////////////////////////////////////////////////////
//
// Externed function declarations
//
////////////////////////////////////////////////////////////////
#ifdef _PC
extern bool AddPlayerToStartData(PLAYER_TYPE playerType, long grid, long carType, long spectator, unsigned long time, CTRL_TYPE ctrlType, DPID id, char *name);
extern void SetStartingPlayerData(long playerNum, PLAYER_TYPE playerType, long grid, long carType, long spectator, unsigned long time, CTRL_TYPE ctrlType, DPID id, char *name);
#else
extern bool AddPlayerToStartData(PLAYER_TYPE playerType, long grid, long carType, long spectator, unsigned long time, CTRL_TYPE ctrlType, long unUsed, char *name);
extern void SetStartingPlayerData(long playerNum, PLAYER_TYPE playerType, long grid, long carType, long spectator, unsigned long time, CTRL_TYPE ctrlType, long unUsed, char *name);
#endif

extern void InitStartData();
extern void InitStartingPlayers();
extern PLAYER *InitOneStartingPlayer(PLAYER_START_DATA *startData);

extern void RandomizeStartingGrid();
extern void RandomizeCPUCarType();
extern void RandomizeSingleRaceCars(void);
extern long PickRandomCar(void);
extern long PickRandomTrack(void);

////////////////////////////////////////////////////////////////
//
// Global Variables
//
////////////////////////////////////////////////////////////////

extern START_DATA StartData, StartDataStorage, MultiStartData;

#endif // INITPLAY_H

