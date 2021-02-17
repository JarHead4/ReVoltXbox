//-----------------------------------------------------------------------------
// File: ghost.h
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef GHOST_H
#define GHOST_H

#include "light.h"
#include "LevelLoad.h"

#define GHOSTDOWNLOAD_EXT   "LapDownLoad"
#define GHOSTLOCAL_EXT      "LapLocal"

#ifdef _PC
#define GHOST_DATA_MAX          (1000)
#else
#define GHOST_DATA_MAX          (600)
#endif

#define GHOST_MAX_SPLIT_TIMES   (MAX_SPLIT_TIMES)
#define GHOST_LAP_TIME          (GHOST_MAX_SPLIT_TIMES)

#ifdef _PC
#define GHOST_MIN_TIMESTEP      MAKE_TIME(0, 0, 75)
#else
#define GHOST_MIN_TIMESTEP      MAKE_TIME(0, 0, 200)
#endif

#define GHOST_VECTOR_SCALE      (100.0f)
#define GHOST_VECTOR_INVSCALE   (1.0f / GHOST_VECTOR_SCALE)

#define GHOST_WHEEL_SCALE       (5.0f)
#define GHOST_WHEEL_INVSCALE    (1.0f / GHOST_WHEEL_SCALE)

#define GHOST_ANGLE_SCALE       (5.0f)
#define GHOST_ANGLE_INVSCALE    (1.0f / GHOST_WHEEL_SCALE)

#define GHOST_TIME_SCALE        (0.5f)
#define GHOST_TIME_INVSCALE     (2.0f)


#ifdef _PC
////////////////////////////////////////////////////////////////
//
// Ghost File header: mainly for internet ladder
//
#define GHOST_HEADER_STRING_SIZE 32

typedef struct GhostHeaderStruct {
    char Header[GHOST_HEADER_STRING_SIZE];
    char LevelName[GHOST_HEADER_STRING_SIZE];
    char LevelFlags[GHOST_HEADER_STRING_SIZE];
    char CarName[GHOST_HEADER_STRING_SIZE];
    char PlayerName[GHOST_HEADER_STRING_SIZE];
    char Time[GHOST_HEADER_STRING_SIZE];
    char Date[GHOST_HEADER_STRING_SIZE];
    char Unused[GHOST_HEADER_STRING_SIZE];
} GHOST_HEADER;
#endif




/////////////////////////////////////////////////////////////////////
//
// Ghost car header info
//
typedef struct GhostInfoStruct {

    long            CarType;
    char            PlayerName[MAX_PLAYER_NAME];
    unsigned long   Time[GHOST_MAX_SPLIT_TIMES + 1];
    long            NFrames;

} GHOST_INFO;

/////////////////////////////////////////////////////////////////////
//
// Ghost car data per frame
//
typedef struct GhostDataStruct {

    unsigned long       Time;
    signed char         WheelAngle;
    unsigned char       WheelPos;
    short               PosX, PosY, PosZ;
    CHARQUAT            Quat;

} GHOST_DATA;

#ifdef _N64
//////////////////////////////////////////////////////////////////
// Struct saved onto memcard
//

typedef struct GameGhostLevelDataStruct {

    u32         ID;
    u16         TrackID;
    u8          M,R;

    GHOST_INFO  ghostInfo;
    GHOST_DATA  ghostData[GHOST_DATA_MAX];  

} GAME_LEVEL_GHOST_SAVEDATA;

#endif


extern void InitGhostData(PLAYER *player);
extern void EndGhostData(PLAYER *player);
extern void InitBestGhostData();
extern void ClearBestGhostData();
extern void SwitchGhostDataStores();
extern bool StoreGhostData(CAR *car);
extern bool LoadGhostData();
extern bool SaveGhostData();

extern void InterpGhostData(CAR *car);
extern void InitGhostLight(void);
extern void ReleaseGhostLight(void);


extern GHOST_INFO *GHO_BestGhostInfo;
extern long GHO_BestFrame;
extern PLAYER *GHO_GhostPlayer;
extern bool GHO_GhostDataExists;
extern bool GHO_GhostAllowed;
extern LIGHT *GhostLight;

extern bool gGotNewGhost;

#endif // GHOST_H

