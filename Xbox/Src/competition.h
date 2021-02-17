//-----------------------------------------------------------------------------
// File: competition.h
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef COMPETITION_H
#define COMPETITION_H

// Race laps

// Number of CARS
#ifdef _PC
#define MIN_RACE_CARS           4
  //$REVISIT:  This #ifdef is a temp hack for May02_TechBeta (and possibly future beta releases).
  /// Eventually, all builds should have same value for MAX/DEFAULT_RACE_CARS, which will make this #ifdef unnecessary.
  #ifdef SHIPPING
#define MAX_RACE_CARS           6
#define DEFAULT_RACE_CARS       6
  #else
#define MAX_RACE_CARS           12
#define DEFAULT_RACE_CARS       8
  #endif
#define MIN_RACE_LAPS           1
#define MAX_RACE_LAPS           20
#define DEFAULT_RACE_LAPS       3
#else
#define MIN_RACE_CARS           2
#define MAX_RACE_CARS           4
#define DEFAULT_RACE_CARS       4
#define MIN_RACE_LAPS           1
#define MAX_RACE_LAPS           8
#define DEFAULT_RACE_LAPS       3
#endif

// made up finish pos for give up try

#define CRAZY_FINISH_POS 0xffffffff

// championship end modes

typedef enum {
    CHAMPIONSHIP_END_WAITING_FOR_FINISH,
    CHAMPIONSHIP_END_GAVEUP,
    CHAMPIONSHIP_END_FINISHED,
    CHAMPIONSHIP_END_FAILED,
    CHAMPIONSHIP_END_QUALIFIED,
    CHAMPIONSHIP_END_MENU,

} CHAMPIONSHIP_END_MODE;

typedef enum {
    CUP_QUALIFY_GAMEOVER,
    CUP_QUALIFY_TRIESLEFT,
    CUP_QUALIFY_YES,
} CUP_QUALIFY_FLAG;

////////////////////////////////////////////////////////////////
// Used to store which competitions have been completed
////////////////////////////////////////////////////////////////

enum RaceStyleEnum {
    RACE_STYLE_DEFAULT,
    RACE_STYLE_CHAMPIONSHIP,
    RACE_STYLE_TIMETRIAL,
    RACE_STYLE_PRACTICE,
    RACE_STYLE_SINGLE,
    RACE_STYLE_TRAINING,

    RACE_STYLE_NTYPES
};
typedef long RACE_STYLE;


////////////////////////////////////////////////////////////////
// The different classes 
////////////////////////////////////////////////////////////////

enum RaceClassEnum {
    RACE_CLASS_NONE = -1,
    RACE_CLASS_DEFAULT = 0,
    RACE_CLASS_BRONZE,
    RACE_CLASS_SILVER,
    RACE_CLASS_GOLD,
    RACE_CLASS_SPECIAL,

    RACE_CLASS_NTYPES
};
typedef long RACE_CLASS;


/////////////////////////////////////////////////////////////////////
// Cup Data - races in each cup
/////////////////////////////////////////////////////////////////////

#define MAX_CUP_RACES   5
#define CUP_NUM_TRIES   3

#define CUP_QUALIFY_POS 3

typedef struct {
    int Num, Laps, Mir, Rev;
} CUP_INFO_LEVEL;

typedef struct CupInfoStruct {
    int NRaces;                 // Number of races in the cup
    CUP_INFO_LEVEL Level[MAX_CUP_RACES];
} CUP_INFO;

typedef struct {
    long            PlayerSlot;
    long            Points;
    long            NewPoints;
    long            CarType;
    long            FinishPos[MAX_CUP_RACES];
    long            FinishTime[MAX_CUP_RACES];
} CUP_TABLE_PLAYER;

typedef struct CupTableStruct {
    long CupType, RaceNum, TriesLeft;
    CUP_QUALIFY_FLAG QualifyFlag;
    CUP_TABLE_PLAYER Player[DEFAULT_RACE_CARS];
    CUP_TABLE_PLAYER PlayerOrder[DEFAULT_RACE_CARS];

    bool    CupCompleted;           // Whether all races in cup were finished (i.e. not if player quit half way through)
    long    LocalPlayerPos;         // Local players final race position
    int     WinLoseCarType[4];        // Car IDs of top three cars (indices 1-3), plus player's car if not in top three (index 0)
} CUP_TABLE;


////////////////////////////////////////////////////////////////
// External function prototypes
////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

extern void InitCupTable(void);
extern void InitOneCupLevel(void);

extern bool IsCupBeatTimeTrials(int cup);
extern bool IsCupBeatTimeTrialsReversed(int cup);
extern bool IsCupBeatTimeTrialsMirrored(int cup);
extern bool IsCupFoundPractiseStars(int cup);
extern bool IsCupWonSingleRaces(int cup);
extern bool IsCupCompleted(int cup);

extern void SetCupCompleted(int cup);

extern void ForceAllCarFinish(void);
extern void MaintainChampionshipEndScreens(void);
extern void UpdateChampionshipData(void);
extern void GoToNextChampionshipRace(void);

#ifdef _PSX
extern void DrawChampionshipTable(void);
extern long ChampTableOff;
extern long TableTableOff;

#endif

// Externed variables
////////////////////////////////////////////////////////////////
extern CUP_TABLE CupTable;
extern char *CupNameText[];
extern CUP_INFO CupData[RACE_CLASS_NTYPES];
extern CHAMPIONSHIP_END_MODE ChampionshipEndMode;

#endif  // COMPETITION_H