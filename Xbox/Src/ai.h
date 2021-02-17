//-----------------------------------------------------------------------------
// File: AI.h
//
// Desc: Utility AI functions, used by car and general AIs
//
// Re-Volt (Generic) Copyright (c) Probe Entertainment 1998
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef AI_H
#define AI_H

//#define GAZZA_TEACH_CAR_HANDLING          // To teach cars how to handle

struct object_def;
struct CarStruct;
struct PlayerStruct;

// fox stuff

#define FOX_NORETURN_TIME   TO_TIME(Real(2.0f))
#define FOX_RANGE 384

#ifdef _PC
#include <pshpack1.h> // one-byte alignment for structs sent over network frequently
typedef struct {
    unsigned long Player1ID;
    unsigned long Player2ID;
} FOX_TRANSFER_DATA;
#include <poppack.h>
#endif //_PC


////////////////////////////////////////////////////////////////
//
// Calculate frontend car stats stuff
//
////////////////////////////////////////////////////////////////

typedef enum CalcStatsStateEnum {
    CALCSTATS_STATE_WAIT,
    CALCSTATS_STATE_RACING,
    CALCSTATS_STATE_DONE,
} CALCSTATS_STATE;

typedef struct CalcStatsVars {
    CALCSTATS_STATE State;
    long            CarType;
    REAL            Timer;
    bool            GotAcc;
    bool            GotSpeed;
} CALCSTATS_VARS;

//
// External global variables
//

extern long AI_Testing;
extern struct object_def *FoxObj;
extern CALCSTATS_VARS CalcStatsVars;

//
// External function prototypes
//
extern void AI_ProcessAllAIs(void);
extern void AI_CarAiHandler(struct object_def *obj);
extern void AI_LocalAiHandler(struct object_def *obj);
extern void AI_RemoteAiHandler(struct object_def *obj);
extern void AI_GhostCarAiHandler(struct object_def *obj);
extern void AI_ReplayAiHandler(struct object_def *obj);
extern void AI_CalcStatsAiHandler(struct object_def *obj);

extern void AI_BarrelHandler(struct object_def *obj);
extern void AI_PlanetHandler(struct object_def *obj);
extern void AI_PlaneHandler(struct object_def *obj);
extern void AI_CopterHandler(struct object_def *obj);
extern void AI_DragonHandler(struct object_def *obj);
extern void AI_WaterHandler(struct object_def *obj);
extern void AI_BoatHandler(struct object_def *obj);
extern void AI_RadarHandler(struct object_def *obj);
extern void AI_BalloonHandler(struct object_def *obj);
extern void AI_HorseRipper(struct object_def *obj);
extern void NewCopterDest(struct object_def *obj);
extern void AI_TrainHandler(struct object_def *obj);
extern void AI_StrobeHandler(struct object_def *obj);
extern void SparkGenHandler(struct object_def *obj);
extern void AI_SpacemanHandler(struct object_def *obj);
extern void AI_DissolveModelHandler(struct object_def *obj);
extern void AI_LaserHandler(struct object_def *obj);
extern void AI_SplashHandler(struct object_def *obj);
extern void SpeedupImpulse(struct CarStruct *car);
extern void AI_SpeedupAIHandler(struct object_def *obj);
extern void PlayerTargetOn(struct PlayerStruct *player);
extern void PlayerTargetOff(struct PlayerStruct *player);
extern void AI_SprinklerHandler(struct object_def *obj);
extern void AI_SprinklerHoseHandler(struct object_def *obj);
extern void AI_StreamHandler(struct object_def *obj);
extern void AI_BangNoiseHandler(struct object_def *obj);
extern void AI_StarHandler(struct object_def *obj);
extern void AI_FoxHandler(struct object_def *obj);
extern void TransferFox(struct object_def *obj, struct PlayerStruct *player1, struct PlayerStruct *player2);
extern void AI_TumbleweedHandler(struct object_def *obj);
extern void RotorControlsSwitching(struct CarStruct *car);
extern void ElectroPulseTheWorld(long slot);
extern void AI_LanternHandler(struct object_def *obj);
extern void AI_3DSoundHandler(struct object_def *obj);
extern void AI_SliderHandler(struct object_def *obj);
extern void AI_RainHandler(struct object_def *obj);
extern void AI_LightningHandler(struct object_def *obj);
extern void AI_ShipLightHandler(struct object_def *obj);
extern void AI_WaterBoxHandler(struct object_def *obj);
extern void AI_RippleHandler(struct object_def *obj);
extern void AI_DolphinMoveHandler(struct object_def *obj);
extern void AI_FlagHandler(struct object_def *obj);
extern void GhostCarInitCloth(struct CarStruct* pCar);
extern void GhostCarProcessCloth(struct CarStruct* pCar, REAL speed);
extern void AI_FogBoxHandler(struct object_def *obj);

#endif // AI_H

