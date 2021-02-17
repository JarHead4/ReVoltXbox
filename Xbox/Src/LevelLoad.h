//-----------------------------------------------------------------------------
// File: LevelLoad.h
//
// Desc: Code and data used when loading a level.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef LEVEL_H
#define LEVEL_H

#include "car.h"
#include "main.h"
#include "competition.h"
#include "LevelInfo.h"

//
// Defines and macros
//

//$NOTE: moved MULTITYPE_** enum from here to "network.h"

enum {
    GAMETYPE_NONE,
    GAMETYPE_TRIAL,
    GAMETYPE_SINGLE,
    GAMETYPE_CLOCKWORK,
    GAMETYPE_NETWORK_RACE, //$NOTE: was originally named GAMETYPE_MULTI
    GAMETYPE_REPLAY,
    GAMETYPE_NETWORK_BATTLETAG, //$NOTE: was originally named GAMETYPE_BATTLE
    GAMETYPE_CHAMPIONSHIP,
    GAMETYPE_PRACTICE,
    GAMETYPE_TRAINING,
    GAMETYPE_FRONTEND,
    GAMETYPE_INTRO,
    GAMETYPE_DEMO,
    GAMETYPE_TWOPLAYER,
    GAMETYPE_GREGMODE,
    GAMETYPE_CALCSTATS,

    GAMETYPE_NTYPES
};

/*typedef struct {
    long Time;
    char Player[MAX_PLAYER_NAME];
    char Car[CAR_NAMELEN];
} ONE_RECORD_ENTRY;

typedef struct {
    long SplitTime[MAX_SPLIT_TIMES];
    ONE_RECORD_ENTRY RecordLap[MAX_RECORD_TIMES];
    ONE_RECORD_ENTRY RecordRace[MAX_RECORD_TIMES];
} RECORD_ENTRY;*/

typedef struct {
    long Time;
    char Player[MAX_PLAYER_NAME];

#ifdef _PC
    char Car[CAR_NAMELEN];
#else
    short CarType;
#endif

} ONE_RECORD_ENTRY;

typedef struct {
    long SplitTime[MAX_SPLIT_TIMES];
    ONE_RECORD_ENTRY RecordLap[MAX_RECORD_TIMES];
} RECORD_ENTRY;




typedef struct
{
    short   X;
    short   Y; 
    short   Z;

} TE_LINK;


typedef struct
{
    short   CentreX;
    short   CentreY;
    short   CentreZ;
    short   XSize;
    short   YSize;
    short   ZSize;
    TE_LINK Links[2];

} TRACKZONE;


typedef struct
{
    int     ZoneID;
    int     Forwards;

} ZONESEQENTRY;


typedef struct
{
#ifdef _PSX
    VECTOR          Position;
#endif
    int             Distance;
    int             Prev[4];
    int             Next[4];

} TE_POSNODE;

typedef struct
{
    int     Offset;
    int     Size;
    int     Data1;
    int     Data2;

} MOD_ENTRY;


typedef struct
{
    int         Count;
    MOD_ENTRY   Entry[ 1 ];

} MOD_HEADER;


typedef struct
{
    short       ZoneCount;
    TRACKZONE   Zones[1];

} TE_ZONE_HEADER;


typedef struct
{
    short           GreenX;
    short           GreenY;
    short           GreenZ;
    short           RedX;
    short           RedY;
    short           RedZ;
    unsigned short  RacingLine;

} TE_NODE_INFO;


typedef struct
{
    short           NodeCount;
    TE_NODE_INFO    Nodes[1];

} TE_NODE_HEADER;


typedef struct
{
#ifdef _PSX
    VECTOR      Ends[2];
#endif
    int         RacingLine; //value in the range 0.0 (green) thru 1.0 (red) - used to express distance between Ends that the racing line is at
    int         Type;

} TE_AINODEINFO;

enum {AI_GREEN_NODE, AI_RED_NODE};  //enumerated values for the indices into AI_NODE::Ends[]

#define min(a,b)            (((a) < (b)) ? (a) : (b))

extern  int             TE_AINodeCount;
extern  int             TE_AINodesPlaced;
extern  TE_AINODEINFO*  TE_AINodes;
extern  TE_POSNODE      *PosNodeBuffer;
extern  int             LapDistance;
extern  int             PosnNodeCount;


// Global variables

extern GAME_SETTINGS GameSettings;
extern VEC          LEV_StartPos;
extern REAL         LEV_StartRot; 
extern long         LEV_StartGrid;
extern VEC *LEV_LevelFieldPos;
extern MAT *LEV_LevelFieldMat;

// Global function externs

extern bool LEV_InitLevelStageOne();
extern bool LEV_InitLevelStageTwo();
extern void LEV_EndLevelStageOne();
extern void LEV_EndLevelStageTwo();

extern void InitLoadingDisplay(void);
extern void LoadingDisplay(WCHAR *text);

extern void LoadCurrentLevelInfo(long Level);

#endif // LEVEL_H

