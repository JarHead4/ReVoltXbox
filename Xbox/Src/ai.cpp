//-----------------------------------------------------------------------------
// File: ai.cpp
//
// Desc: Utility AI functions, used by car and general AIs
//
// Re-Volt (Generic) Copyright (c) Probe Entertainment 1998
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------

#include "revolt.h"
#include "ai.h"
#include "weapon.h"
#include "player.h"
#include "timing.h"
#include "obj_init.h"
#ifdef _N64
#include "light.h"
#endif
#ifdef _PC
#include "settings.h"
#include "ghost.h"
#include "input.h"
#include "panel.h"
#endif
#ifndef _PSX
#include "spark.h"
#include "visibox.h"
#endif
#ifdef _N64
#include "gamegfx.h"
#include "gfx.h"
#include "ISound.h"
#include "panel.h"
#include "gameloop.h"
#endif
#include "aizone.h"
#include "posnode.h"
#include "initplay.h"
#include "main.h"
#include "util.h"
#include "move.h"
#include "pickup.h"
#include "text.h"
#include "gameloop.h"
#include "ui_TitleScreen.h"
#include "field.h"

#ifdef _PSX
#include "overlay.h"
#include "sound.h"
#endif

#ifndef _PC
#include "MenuDraw.h"
#endif

#include "SoundEffectEngine.h"


//
// Global variables
//

long    AI_Testing = FALSE;
OBJECT *FoxObj;
CALCSTATS_VARS CalcStatsVars = {
    CALCSTATS_STATE_WAIT,
    CARID_RC,
    ZERO,
};


//
// Car Sfx's priorities
//
#ifdef _N64
#define SFX_PRIORITY_SCRAPE     -1
#define SFX_PRIORITY_SCREECH    -1
#define SFX_PRIORITY_SERVO      -1
#else
#define SFX_PRIORITY_SCRAPE      0
#define SFX_PRIORITY_SCREECH     0
#define SFX_PRIORITY_SERVO       0
#endif
//
// Static variables
//

static VEC PlanePropOff = {0, -120, 110};
static VEC CopterBlade1Off = {0, -340, 66};
static VEC CopterBlade2Off = {28, -252, -200};
static VEC TrainSteamOffset = {0, -550, -196};
static VEC TrainSteamDir = {0, -350, 0};
static VEC BoatSteamOffset = {0, -350, 0};
static VEC BoatSteamVel = {0, -500, -100};
static VEC TrainWheelOffsets[] = {{98, -136, 330}, {-98, -136, 330}, {98, -92, -204}, {-98, -92, -204}};
static VEC SprinklerJetOffset = {0, 8, -24};
static VEC SprinklerJetVel = {0, -1300, -2000};
static VEC DolphinVel = {0, -500, -300};

//
// Global function prototypes
//

void AI_ProcessAllAIs(void);
void AI_CarAiHandler(OBJECT *obj);

static void UpdatePlayerPickup(PLAYER *player);
static void UpdateCarSfx(PLAYER *player);
static void UpdateCarMisc(PLAYER *player);
static void TurnCopter(OBJECT *obj);
static void FlyCopter(OBJECT *obj);
static void CopterWait(OBJECT *obj);
void NewCopterDest(OBJECT *obj);
void AI_LaserHandler(OBJECT *obj);
void SparkGenHandler(OBJECT *obj);
static void WaterBoxWorldMeshFxChecker(void *data);
static void WaterBoxModelMeshFxChecker(void *data);

#ifndef _PSX

#ifdef OLD_AUDIO
// scrape sfx list
static long SfxScrapeList[MATERIAL_NTYPES] = {
    SFX_SCRAPE,             // default        
    SFX_SCRAPE,             // marble     
    SFX_SCRAPE,             // stone      
    SFX_SCRAPE,             // wood       
    SFX_SCRAPE,             // sand       
    SFX_SCRAPE,             // plastic    
    SFX_SCRAPE,             // carpet tile
    SFX_SCRAPE,             // carpet shag
    SFX_SCRAPE,             // boundary   
    SFX_SCRAPE,             // glass      
    SFX_SCRAPE,             // ice 1      
    SFX_SCRAPE,             // metal      
    SFX_SCRAPE,             // grass      
    SFX_SCRAPE,             // bumpy metal
    SFX_SCRAPE,             // pebbles    
    SFX_SCRAPE,             // gravel     
    SFX_SCRAPE,             // conveyor 1 
    SFX_SCRAPE,             // conyeyor 2 
    SFX_SCRAPE,             // dirt 1     
    SFX_SCRAPE,             // dirt 2     
    SFX_SCRAPE,             // dirt 3     
    SFX_SCRAPE,             // ice 2      
    SFX_SCRAPE,             // ice 3      
    SFX_SCRAPE,             // wood 2     
    SFX_SCRAPE,             // market1 conveyor
    SFX_SCRAPE,             // market2 conveyor
};

// skid sfx list
static long SfxSkidList[MATERIAL_NTYPES] = {
    SFX_SKID_NORMAL,            // default    
    SFX_SKID_NORMAL,            // marble     
    SFX_SKID_NORMAL,            // stone      
    SFX_SKID_NORMAL,            // wood       
    SFX_SKID_ROUGH,             // sand       
    SFX_SKID_NORMAL,            // plastic    
    SFX_SKID_ROUGH,             // carpet tile
    SFX_SKID_ROUGH,             // carpet shag
    SFX_SKID_NORMAL,            // boundary   
    SFX_SKID_NORMAL,            // glass      
    SFX_SKID_ROUGH,             // ice 1      
    SFX_SKID_NORMAL,            // metal      
    SFX_SKID_ROUGH,             // grass      
    SFX_SKID_NORMAL,            // bumpy metal
    SFX_SKID_ROUGH,             // pebbles    
    SFX_SKID_ROUGH,             // gravel     
    SFX_SKID_NORMAL,            // conveyor 1 
    SFX_SKID_NORMAL,            // conyeyor 2 
    SFX_SKID_ROUGH,             // dirt 1     
    SFX_SKID_ROUGH,             // dirt 2     
    SFX_SKID_ROUGH,             // dirt 3     
    SFX_SKID_ROUGH,             // ice 2      
    SFX_SKID_ROUGH,             // ice 3      
    SFX_SKID_NORMAL,            // wood 2     
    SFX_SKID_NORMAL,            // market1 conveyor
    SFX_SKID_NORMAL,            // market2 conveyor
};
#else
// scrape sfx list
static long SfxScrapeList[MATERIAL_NTYPES] = {
    EFFECT_Scrape,             // default        
    EFFECT_Scrape,             // marble     
    EFFECT_Scrape,             // stone      
    EFFECT_Scrape,             // wood       
    EFFECT_Scrape,             // sand       
    EFFECT_Scrape,             // plastic    
    EFFECT_Scrape,             // carpet tile
    EFFECT_Scrape,             // carpet shag
    EFFECT_Scrape,             // boundary   
    EFFECT_Scrape,             // glass      
    EFFECT_Scrape,             // ice 1      
    EFFECT_Scrape,             // metal      
    EFFECT_Scrape,             // grass      
    EFFECT_Scrape,             // bumpy metal
    EFFECT_Scrape,             // pebbles    
    EFFECT_Scrape,             // gravel     
    EFFECT_Scrape,             // conveyor 1 
    EFFECT_Scrape,             // conyeyor 2 
    EFFECT_Scrape,             // dirt 1     
    EFFECT_Scrape,             // dirt 2     
    EFFECT_Scrape,             // dirt 3     
    EFFECT_Scrape,             // ice 2      
    EFFECT_Scrape,             // ice 3      
    EFFECT_Scrape,             // wood 2     
    EFFECT_Scrape,             // market1 conveyor
    EFFECT_Scrape,             // market2 conveyor
};

// skid sfx list
static long SfxSkidList[MATERIAL_NTYPES] = {
    EFFECT_SkidNormal,            // default    
    EFFECT_SkidNormal,            // marble     
    EFFECT_SkidNormal,            // stone      
    EFFECT_SkidNormal,            // wood       
    EFFECT_SkidRough,             // sand       
    EFFECT_SkidNormal,            // plastic    
    EFFECT_SkidRough,             // carpet tile
    EFFECT_SkidRough,             // carpet shag
    EFFECT_SkidNormal,            // boundary   
    EFFECT_SkidNormal,            // glass      
    EFFECT_SkidRough,             // ice 1      
    EFFECT_SkidNormal,            // metal      
    EFFECT_SkidRough,             // grass      
    EFFECT_SkidNormal,            // bumpy metal
    EFFECT_SkidRough,             // pebbles    
    EFFECT_SkidRough,             // gravel     
    EFFECT_SkidNormal,            // conveyor 1 
    EFFECT_SkidNormal,            // conyeyor 2 
    EFFECT_SkidRough,             // dirt 1     
    EFFECT_SkidRough,             // dirt 2     
    EFFECT_SkidRough,             // dirt 3     
    EFFECT_SkidRough,             // ice 2      
    EFFECT_SkidRough,             // ice 3      
    EFFECT_SkidNormal,            // wood 2     
    EFFECT_SkidNormal,            // market1 conveyor
    EFFECT_SkidNormal,            // market2 conveyor
};
#endif //OLD_AUDIO

#endif

//--------------------------------------------------------------------------------------------------------------------------

//
// AI_ProcessAllAIs
//
// Processes all the AI functions for active objects
//

void AI_ProcessAllAIs(void)
{
    OBJECT  *obj, *next;

// Update car race positions

    CAI_CalcCarRacePositions();

// call all AI handlers

    obj = OBJ_ObjectHead;
    while (obj != NULL)
    {

// get next now in case object frees itself!

        next = obj->next; 

// call AI handler

        if (obj->aihandler)
        {
            obj->aihandler(obj);
        }

// check for out-of-bounds

        if (!PointInBBox(&obj->body.Centre.Pos, &OutOfBoundsBox))
        {
            if (obj->Type != OBJECT_TYPE_CAR)   
            {
#ifdef _PC

                if (Version == VERSION_DEV)
                {
                    char buf[256];
                    sprintf(buf, "Object type %d out of bounds at %ld, %ld, %ld", obj->Type, (long)obj->body.Centre.Pos.v[X], (long)obj->body.Centre.Pos.v[Y], (long)obj->body.Centre.Pos.v[Z]);
                    DumpMessage("Warning!", buf);
                }
#else
    #if DEBUG
                printf( "Object killed due to falling out of World Bounding Box\n" );
    #endif
#endif


                OBJ_FreeObject(obj);
            } 
            else 
            {
                obj->player->controls.digital |= CTRL_REPOSITION;
            }
        }

// next

        obj = next;
    }   
}


//--------------------------------------------------------------------------------------------------------------------------

///////////////////////////
// perform misc car jobs //
///////////////////////////

// Rotor Specific stuff

void RotorControlsSwitching(CAR *car)
{
    int iWheel;
    bool switchControls = FALSE;

    if ((car->Body->Centre.WMatrix.m[UY] > Real(0.85f)) && (car->Wheel[0].SteerRatio > ZERO)) {
        switchControls = TRUE;
    } else if ((car->Body->Centre.WMatrix.m[UY] < Real(-0.85f)) && (car->Wheel[0].SteerRatio < ZERO)) {
        switchControls = TRUE;
    }
    if (switchControls) {
        for (iWheel = 0; iWheel < CAR_NWHEELS; iWheel++) {
            car->Wheel[iWheel].SteerRatio = -car->Wheel[iWheel].SteerRatio;
            car->Wheel[iWheel].EngineRatio = -car->Wheel[iWheel].EngineRatio;
        }
    }
}

// CPU AI handler

void AI_CarAiHandler(OBJECT *obj)
{
    obj->player->LastValidRailCamNode = obj->player->ValidRailCamNode;
    obj->player->ValidRailCamNode = -1;
    obj->CamLength = ONE;

#ifndef _PSX
    UpdateCarSfx(obj->player);
#endif
    CAI_CarHelper(obj->player);

    UpdateCarMisc(obj->player);
//#ifndef _PSX
    UpdatePlayerPickup(obj->player);
//#endif


    // Special for Rotor...
//  if (obj->player->car.CarType == 22) {
//      RotorControlsSwitching(&obj->player->car);
//  }

#ifdef _PC
    GhostCarProcessCloth(&obj->player->car, obj->player->CarAI.speedCur);
#endif
}

// Local AI handler

void AI_LocalAiHandler(OBJECT *obj)
{
    obj->player->LastValidRailCamNode = obj->player->ValidRailCamNode;
    obj->player->ValidRailCamNode = -1;

#ifndef _PSX
    obj->CamLength = ONE;
    CarAccTimings(&obj->player->car);
    UpdateCarSfx(obj->player);
#endif
    CAI_CarHelper(obj->player);
    UpdateCarMisc(obj->player);
    UpdatePlayerPickup(obj->player);
    CAI_UpdateNodeData(obj->player);

#ifdef _PC
    GhostCarProcessCloth(&obj->player->car, obj->player->CarAI.speedCur);
#endif
}

// Remote AI handler

void AI_RemoteAiHandler(OBJECT *obj)
{
    obj->player->LastValidRailCamNode = obj->player->ValidRailCamNode;
    obj->player->ValidRailCamNode = -1;
    obj->CamLength = ONE;

#ifndef _PSX
    UpdateCarSfx(obj->player);
#endif
    UpdateCarMisc(obj->player);
#ifndef _PSX
    UpdatePlayerPickup(obj->player);
#endif
    CAI_UpdateNodeData(obj->player);    // needed for repos

#ifdef _PC
    GhostCarProcessCloth(&obj->player->car, obj->player->CarAI.speedCur);
#endif
}
    
// Ghost AI handler

void AI_GhostCarAiHandler(OBJECT *obj)
{
    obj->player->LastValidRailCamNode = obj->player->ValidRailCamNode;
    obj->player->ValidRailCamNode = -1;
    obj->CamLength = ONE;

    //MOV_MoveGhost(obj);

#ifndef _PSX
    UpdateCarSfx(obj->player);
#endif
    UpdateCarMisc(obj->player);
#ifndef _PSX
    UpdatePlayerPickup(obj->player);
#endif

#ifdef _PC
    GhostCarProcessCloth(&obj->player->car, obj->player->CarAI.speedCur);
#endif
}

// Replay AI handler

void AI_ReplayAiHandler(OBJECT *obj)
{
    obj->player->LastValidRailCamNode = obj->player->ValidRailCamNode;
    obj->player->ValidRailCamNode = -1;
    obj->CamLength = ONE;

#ifndef _PSX
    UpdateCarSfx(obj->player);
#endif
    UpdateCarMisc(obj->player);
#ifndef _PSX
    UpdatePlayerPickup(obj->player);
#endif

    // Special for Rotor...
    if (obj->player->car.CarType == 22) {
        RotorControlsSwitching(&obj->player->car);
    }

#ifdef _PC
    GhostCarProcessCloth(&obj->player->car, obj->player->CarAI.speedCur);
#endif
}
    
///////////////////////////////////
// update players cycling pickup //
///////////////////////////////////
//#ifdef _PSX
#if 0

void UpdatePlayerPickup(PLAYER *player)
{
    REAL f;

// only if no cycling pickup
    if (player->PickupCycleSpeed) {

    // dec cycle speed

        player->PickupCycleSpeed -= TimeFactor;

// inc cycle type

        player->PickupCycleType ++;
        if (player->PickupCycleType > PICKUP_NTYPES)
            player->PickupCycleType -= PICKUP_NTYPES;

// give to player?
        if (player->PickupCycleSpeed < 10 )
        {
// yep
            GivePickupToPlayer(player, NearestInt(player->PickupCycleDest) );//FramesEver % PICKUP_NTYPES);
            player->PickupCycleSpeed = 0;
            player->CarAI.pickupDuration = 0;           // Reset pickup duration
        }
    }

// maintain pickup target

    
    if( player->PickupTargetOn )
    {
        OBJECT *oldTarget;
        oldTarget = player->PickupTarget;
        player->PickupTarget = WeaponTarget(player->ownobj);
        if( oldTarget != player->PickupTarget )
            player->PickupTargetTime = ZERO;
        else
        {
            player->PickupTargetTime += TimeStep;
            if( player->PickupTargetTime > (TO_TIME(Real(16))) )    // PSX version needs clamping.
                player->PickupTargetTime = TO_TIME( ONE );
        }
    }
    
}

#else

void UpdatePlayerPickup(PLAYER *player)
{

// only if no cycling pickup
    if ((player->PickupCycleSpeed > ZERO) && !ReplayMode) {

        // dec cycle speed
        player->PickupCycleSpeed -= TimeStep;
        if (player->PickupCycleSpeed < ZERO) player->PickupCycleSpeed = ZERO;

        // calculate current pickup
#ifndef _PSX
        player->PickupCycleType = 
            player->PickupCycleDest + 
            (2*PICKUP_NTYPES) - 
            ((2*PICKUP_NTYPES) / (PICKUP_CYCLE_TIME * PICKUP_CYCLE_TIME * PICKUP_CYCLE_TIME)) * player->PickupCycleSpeed * player->PickupCycleSpeed * player->PickupCycleSpeed;
#else
        player->PickupCycleType = 
            player->PickupCycleDest + 
            (2*PICKUP_NTYPES) - 
//          MulScalar(MulScalar(MulScalar(DivScalar((2*PICKUP_NTYPES), PICKUP_CYCLE_TIME_CUBED), player->PickupCycleSpeed), player->PickupCycleSpeed), player->PickupCycleSpeed);
            ((2*PICKUP_NTYPES) * MulScalar3(DivScalar(player->PickupCycleSpeed, PICKUP_CYCLE_TIME),
                                            DivScalar(player->PickupCycleSpeed, PICKUP_CYCLE_TIME),
                                            DivScalar(player->PickupCycleSpeed, PICKUP_CYCLE_TIME)));
#endif
        GoodWrap(&player->PickupCycleType, 0, Real(PICKUP_NTYPES));
        //if (player->PickupCycleType >= Real(PICKUP_NTYPES - 1)) player->PickupCycleType -= Real(PICKUP_NTYPES - 1);


        if ((player->PickupCycleSpeed == ZERO) && (GameSettings.GameType != GAMETYPE_REPLAY)) {
            GivePickupToPlayer(player, NearestInt(player->PickupCycleDest));
            player->CarAI.pickupDuration = 0;           // Reset pickup duration
        }
    }

// maintain pickup target

    if (player->PickupTargetOn)
    {
        OBJECT *oldTarget;
        oldTarget = player->PickupTarget;
        player->PickupTarget = WeaponTarget(player->ownobj);
        if (oldTarget != player->PickupTarget) {
            player->PickupTargetTime = ZERO;
        } else {
            player->PickupTargetTime += TimeStep;
        }
    }
}
#endif

////////////////////////////////////////////////////////////
//
// Car Stats calculating AI handler
//
////////////////////////////////////////////////////////////

void AI_CalcStatsAiHandler(OBJECT *obj)
{
    REAL vel, acc;
    VEC accXZ;
    VEC pos = {ZERO, TO_LENGTH(Real(-50)), ZERO};

    obj->player->LastValidRailCamNode = obj->player->ValidRailCamNode;
    obj->player->ValidRailCamNode = -1;
    obj->CamLength = ONE;

#ifndef _PSX
    CarAccTimings(&obj->player->car);
    UpdateCarSfx(obj->player);
#endif

    UpdateCarMisc(obj->player);
    UpdatePlayerPickup(obj->player);

    // Calculate stats handler
    CalcStatsVars.Timer += TimeStep;
    
    switch (CalcStatsVars.State) {

    case CALCSTATS_STATE_WAIT:

        // Stop
        Players[0].controls.dx = 0;
        Players[0].controls.dy = 0;

        if ((CalcStatsVars.Timer > TO_TIME(Real(2))) && (abs(Players[0].car.Body->Centre.Vel.v[Y]) < TO_VEL(Real(1)))) {
            CalcStatsVars.Timer = ZERO;
            CalcStatsVars.State = CALCSTATS_STATE_RACING;
            CalcStatsVars.GotAcc = FALSE;
            CalcStatsVars.GotSpeed = FALSE;
        }
        break;

    case CALCSTATS_STATE_RACING:

        // Accelerate
        Players[0].controls.dy = -CTRL_RANGE_MAX;

        vel = VecLen(&Players[0].car.Body->Centre.Vel);
        if (!CalcStatsVars.GotAcc) {
            if (vel > MPH2OGU_SPEED * 25) {
                CarInfo[CalcStatsVars.CarType].Acc = CalcStatsVars.Timer;
                CalcStatsVars.GotAcc = TRUE;
            } else {
#ifdef _PC
                DumpText(100, 50, 8, 16, 0xff0000, L"Calculating Acc");
#else
                DumpMenuText(50, 50, MENU_TEXT_WIDTH, MENU_TEXT_HEIGHT, 0xffff0000, L"Calc Acc", -1);
#endif
            }
        }

        SetVec(&accXZ, Players[0].car.Body->Centre.Acc.v[X], ZERO, Players[0].car.Body->Centre.Acc.v[Z]); 
        acc = VecLen(&accXZ);
        if (!CalcStatsVars.GotSpeed) {
#ifndef _PSX
            if (CalcStatsVars.GotAcc && (acc < TO_ACC(Real(0.1)))) {
#else
            if (CalcStatsVars.GotAcc && (acc <= TO_ACC(Real(0.1))) && (Players[0].car.Body->Centre.Pos.v[Z] > TO_LENGTH(Real(5000)))) {
#endif
                CarInfo[CalcStatsVars.CarType].TopEnd = vel;
                CalcStatsVars.GotSpeed = TRUE;
            } else {
#ifdef _PC
                DumpText(100, 70, 8, 16, 0xff0000, L"Calculating Speed");
#else
                DumpMenuText(50, 70, MENU_TEXT_WIDTH, MENU_TEXT_HEIGHT, 0xffff0000, L"Calc Spd", -1);
#endif
            }
        }

        if (CalcStatsVars.GotAcc && CalcStatsVars.GotSpeed) {
            Players[0].controls.dx = CTRL_RANGE_MAX;
            Players[0].controls.dy = 0;
#ifdef _PC
            if (CalcStatsVars.CarType++ < CARID_AMW) {

                // Next Car
                SetupCar(&Players[0], CalcStatsVars.CarType);
                SetCarAerialPos(&Players[0].car);

                // Reposition
                SetCarPos(&Players[0].car, &pos, &Identity);

                // Reset state
                CalcStatsVars.Timer = ZERO;
                CalcStatsVars.State = CALCSTATS_STATE_WAIT;
            } 
            else 
            {
                // Quit
                GameLoopQuit = GAMELOOP_QUIT_FRONTEND;
                SetFadeEffect(FADE_DOWN);

                CalcStatsVars.Timer = ZERO;
                CalcStatsVars.State = CALCSTATS_STATE_DONE;
            }
#else
            // Do nothing until I've written it down....
            sprintf(MenuBuffer, "Spd: %d", Int(FROM_VEL(CarInfo[CalcStatsVars.CarType].TopEnd)));
            DumpMenuText(50, 120, MENU_TEXT_WIDTH, MENU_TEXT_HEIGHT, 0xffffffff, MenuBuffer, -1);
            sprintf(MenuBuffer, "Acc: %d", Int(FROM_TIME(CarInfo[CalcStatsVars.CarType].Acc)*100));
            DumpMenuText(50, 140, MENU_TEXT_WIDTH, MENU_TEXT_HEIGHT, 0xffffffff, MenuBuffer, -1);
#endif
        }
        break;

    case CALCSTATS_STATE_DONE:
        break;
    }
}

///////////////////////////
// turn on player target //
///////////////////////////

void PlayerTargetOn(PLAYER *player)
{
    player->PickupTargetOn = TRUE;
    player->PickupTarget = NULL;
    player->PickupTargetTime = ZERO;
}

////////////////////////////
// turn off player target //
////////////////////////////

void PlayerTargetOff(PLAYER *player)
{
    player->PickupTargetOn = FALSE;
    player->PickupTarget = NULL;
    player->PickupTargetTime = ZERO;
}

/////////////////////////////////////////////////////////////////////
// UpdateCarSfx:
/////////////////////////////////////////////////////////////////////
#ifndef _PSX
static void UpdateCarSfx(PLAYER *player)
{
    long i, revs, vel;
#ifdef OLD_AUDIO
    long maxvol, time, sfx;
#endif // OLD_AUDIO
    long matcount[MATERIAL_NTYPES];
    long skidmat, skidmax;
    CAR *car = &player->car;

// get car revs, velocity

    FTOL(abs(car->Revs / 9.0f), revs);
    FTOL(Length(&car->Body->Centre.Vel), vel);

// update engine

#ifdef OLD_AUDIO
    if (car->SfxEngine)
    {
        if (car->SfxEngine->Num == SFX_ENGINE)
        {
            car->SfxEngine->Vol = revs / 20;
            if (car->SfxEngine->Vol > SFX_MAX_VOL) car->SfxEngine->Vol = SFX_MAX_VOL;
            car->SfxEngine->Freq = 10000 + (long)(revs * 8);
        }
        else if (car->SfxEngine->Num == SFX_ENGINE_PETROL)
        {
            car->SfxEngine->Vol = SFX_MAX_VOL;
            car->SfxEngine->Freq = 7000 + (long)(revs * 15);
            if (car->SfxEngine->Freq > 70000) car->SfxEngine->Freq = 70000;
        }
  #ifdef _PC
        else if (car->SfxEngine->Num == SFX_ENGINE_CLOCKWORK)
        {
            car->SfxEngine->Vol = SFX_MAX_VOL;
            car->SfxEngine->Freq = 20000 + (long)(revs * 5);
        }
        else if (car->SfxEngine->Num == SFX_ENGINE_UFO)
        {
            car->SfxEngine->Vol = SFX_MAX_VOL;
            car->SfxEngine->Freq = 20000 + (long)(revs * 5);
        }
  #endif // _PC

        CopyVec(&car->Body->Centre.Pos, &car->SfxEngine->Pos);
    }
#else // !OLD_AUDIO
    // TODO (JHarding): Figure out appropriate revs->freq/vol conversion
    // Sometimes the revs seem to go WAY high
    if( revs > 1500 )
        revs = 1500;

    if( GameSettings.Level != LEVEL_FRONTEND )
    {
        assert( car->pEngineSound );

        car->pEngineSound->SetPitch( revs * 3 );
        // Set engine volume based off revs
        car->pEngineSound->SetVolume( -1000 + revs / 2 );
    }
#endif // !OLD_AUDIO

// update scrape

    if (car->Body->ScrapeMaterial == MATERIAL_NONE || GameSettings.Level == LEVEL_FRONTEND)
    {
#ifdef OLD_AUDIO
        if (car->SfxScrape)
        {
            FreeSfx3D(car->SfxScrape);
            car->SfxScrape = NULL;
        }
#else // !OLD_AUDIO
        if( car->pScrapeSound != NULL )
        {
            g_SoundEngine.ReturnInstance( car->pScrapeSound );
            car->pScrapeSound = NULL;
        }
        car->ScrapeMaterial = MATERIAL_NONE;
#endif // !OLD_AUDIO
    }
    else
    {
#ifdef OLD_AUDIO
        sfx = SfxScrapeList[car->Body->ScrapeMaterial];

        if (!car->SfxScrape)
        {
            car->SfxScrape = CreateSfx3D(sfx, 0, 0, TRUE, &car->Body->Centre.Pos, SFX_PRIORITY_SCRAPE); 
            car->ScrapeMaterial = car->Body->ScrapeMaterial;
        }

        if (car->SfxScrape)
        {
            car->SfxScrape->Freq = 20000 + vel * 5;

            FTOL(TimeStep * 600.0f, time);
            if (!time) time = 1;
            maxvol = car->SfxScrape->Vol + time;
            if (maxvol > SFX_MAX_VOL) maxvol = SFX_MAX_VOL;

            car->SfxScrape->Vol = vel / 10;
            if (car->SfxScrape->Vol > maxvol) car->SfxScrape->Vol = maxvol;

            CopyVec(&car->Body->Centre.Pos, &car->SfxScrape->Pos);

            if (car->ScrapeMaterial != car->Body->ScrapeMaterial)
            {
                ChangeSfxSample3D(car->SfxScrape, sfx);
                car->ScrapeMaterial = car->Body->ScrapeMaterial;
            }
        }
#else // !OLD_AUDIO
        // Changing material...
        if( car->ScrapeMaterial != car->Body->ScrapeMaterial )
        {
            // Shut off the old sound, if there was one
            if( car->pScrapeSound != NULL )
            {
                g_SoundEngine.ReturnInstance( car->pScrapeSound );
                car->pScrapeSound = NULL;
            }

            car->ScrapeMaterial = car->Body->ScrapeMaterial;

            // Start the new sound
            g_SoundEngine.PlaySubmixedSound( SfxScrapeList[car->ScrapeMaterial], 
                                             TRUE, 
                                             car->pSourceMix,
                                             &car->pScrapeSound );
        }
#endif // !OLD_AUDIO
    }

// suss skid material

    for (i = 0 ; i < MATERIAL_NTYPES ; i++)
        matcount[i] = 0;

    for (i = 0 ; i < CAR_NWHEELS ; i++)
    {
        if (IsWheelPresent(&car->Wheel[i]) && IsWheelSkidding(&car->Wheel[i]) && IsWheelInContact(&car->Wheel[i]))
        {
            if( car->Wheel[i].SkidMaterial != MATERIAL_NONE ) //$ADDITION(cprince) - nasty bug w/o this
            {
                matcount[car->Wheel[i].SkidMaterial]++;
            }
        }
    }

    skidmax = 0;
    skidmat = MATERIAL_NONE;

    for (i = 0 ; i < MATERIAL_NTYPES ; i++)
    {
        if (matcount[i] > skidmax)
        {
            skidmat = i;
            skidmax = matcount[i];
        }
    }

// update skid

#ifdef OLD_AUDIO
    if (skidmat == MATERIAL_NONE || GameSettings.Level == LEVEL_FRONTEND)
    {
        if (car->SfxScreech)
        {
            FreeSfx3D(car->SfxScreech);
            car->SfxScreech = NULL;
        }
    }
    else
    {
        sfx = SfxSkidList[skidmat];

        if (!car->SfxScreech)
        {
            car->SfxScreech = CreateSfx3D(sfx, 0, 0, TRUE, &car->Body->Centre.Pos, SFX_PRIORITY_SCREECH);
            car->SkidMaterial = skidmat;
        }

        if (car->SfxScreech)
        {
            car->SfxScreech->Freq = 15000 + vel * 2;

            FTOL(TimeStep * 600.0f, time);
            if (!time) time = 1;
            maxvol = car->SfxScreech->Vol + time;
            if (maxvol > SFX_MAX_VOL) maxvol = SFX_MAX_VOL;

            car->SfxScreech->Vol = vel / 10;
            if (car->SfxScreech->Vol > maxvol) car->SfxScreech->Vol = maxvol;

            CopyVec(&car->Body->Centre.Pos, &car->SfxScreech->Pos);

            if (car->SkidMaterial != skidmat)
            {
                ChangeSfxSample3D(car->SfxScreech, sfx);
                car->SkidMaterial = skidmat;
            }
        }
    }
#else // !OLD_AUDIO
    if( skidmat == MATERIAL_NONE || GameSettings.Level == LEVEL_FRONTEND )
    {
        if( car->pScreechSound )
        {
            g_SoundEngine.ReturnInstance( car->pScreechSound );
            car->pScreechSound = NULL;
        }
    }
    else if( !car->pScreechSound )
    {
        g_SoundEngine.PlaySubmixedSound( SfxSkidList[skidmat], TRUE, car->pSourceMix, &car->pScreechSound );
    }

#endif // !OLD_AUDIO

// servo sfx?

    if ( 
#ifdef _PC
        ( player == PLR_LocalPlayer ) && 
#endif
        abs(car->SteerAngle - car->LastSteerAngle) > 0.01f
        )
    {
        car->ServoFlag += TimeStep * 8.0f;
        if (car->ServoFlag > 1.0f) car->ServoFlag = 1.0f;
    }
    else
    {
        car->ServoFlag -= TimeStep * 8.0f;
        if (car->ServoFlag < 0.0f) car->ServoFlag = 0.0f;
    }

#ifdef OLD_AUDIO
    if (car->ServoFlag && GameSettings.Level != LEVEL_FRONTEND)
    {
        if (!car->SfxServo)
        {
            car->SfxServo = CreateSfx3D(SFX_SERVO, 0, 0, TRUE, &car->Body->Centre.Pos, SFX_PRIORITY_SERVO);
        }

        if (car->SfxServo)
        {
            CopyVec(&car->Body->Centre.Pos, &car->SfxServo->Pos);
            car->SfxServo->Vol = (long)(car->ServoFlag * SFX_MAX_VOL);
            car->SfxServo->Freq = (long)(car->ServoFlag * 10000.0f) + 22050;
            if (car->SteerAngle < car->LastSteerAngle)
                car->SfxServo->Freq -= 5000;
        }
    }
    else
    {
        if (car->SfxServo)
        {
            FreeSfx3D(car->SfxServo);
            car->SfxServo = NULL;
        }
    }
#else // !OLD_AUDIO
    if (car->ServoFlag && GameSettings.Level != LEVEL_FRONTEND)
    {
        if( !car->pServoSound )
        {
            g_SoundEngine.PlaySubmixedSound( EFFECT_Servo, TRUE, car->pSourceMix, &car->pServoSound );
        }
    }
    else
    {
        if( car->pServoSound )
        {
            g_SoundEngine.ReturnInstance( car->pServoSound );
            car->pServoSound = NULL;
        }
    }
#endif // !OLD_AUDIO

// dir change?

//  if ((car->EngineVolt > 0.0f && car->LastEngineVolt <= 0.0f) || (car->EngineVolt < 0.0f && car->LastEngineVolt >= 0.0f))
//  {
//      PlaySfx3D(SFX_CHANGEDIR, SFX_MAX_VOL, SFX_SAMPLE_RATE, &car->Body->Centre.Pos, 0);
//  }

    car->LastEngineVolt = car->EngineVolt;
}
#endif

/////////////////////////////////////////////////////////////////////
// UpdateCarMisc:
/////////////////////////////////////////////////////////////////////
static void UpdateCarMisc(PLAYER *player)
{
    VEC vec;
    MAT mat1, mat2;
    CAR *car = &player->car;
#ifdef _PSX
    int     vol;
    int     sfx;
    PLAYER  *other_player;
#endif

// env matrix

#ifdef _PC
    if (RenderSettings.Env)
    {
        SubVector(&car->Body->Centre.Pos, &car->Body->Centre.OldPos, &vec);
        RotMatrixZYX(&mat1, vec.v[X] / 16384, 0, vec.v[Z] / 16384);
        MulMatrix(&car->EnvMatrix, &mat1, &mat2);
        CopyMat(&mat2, &car->EnvMatrix);
    }
#endif

// electropulsed power timer?

    // The PSX *DOES* use this code now.....

    if (car->PowerTimer > -ELECTROPULSE_NO_RETURN_TIME)
    {   
        car->PowerTimer -= TimeStep;
        if (car->PowerTimer < -ELECTROPULSE_NO_RETURN_TIME)
            car->PowerTimer = -ELECTROPULSE_NO_RETURN_TIME;
    }

// zero AddLit

#ifndef _PSX
    if (car->AddLit > 0)
    {
        car->AddLit -= (long)(TimeStep * 2000.0f);
        if (car->AddLit < 0) car->AddLit = 0;
    }
    else if (car->AddLit < 0)
    {
        car->AddLit += (long)(TimeStep * 2000.0f);
        if (car->AddLit > 0) car->AddLit = 0;
    }
#endif

// bomb return timer

    if (car->NoReturnTimer > ZERO) {
        car->NoReturnTimer -= TimeStep;
    }

// fox return timer

    if (car->FoxReturnTimer > ZERO) {
        car->FoxReturnTimer -= TimeStep;
    }

// weapon hit timer
    car->LastHitTimer += TimeStep;

// aerial grow timer
    car->AerialTimer += TimeStep;
    if (car->AerialTimer > TO_TIME(Real(1.0))) {
        car->AerialTimer = TO_TIME(Real(1.0));
    }
    if (!car->IsBomb) {
        car->Aerial.Length = MulScalar(FROM_TIME(car->AerialTimer), CarInfo[car->CarType].Aerial.SecLen);
    }

// Bang storing
#ifndef _PSX
    if (car->Body->BangMag > TO_VEL(Real(500)))
    {
#ifdef OLD_AUDIO
        long vol, freq;

        //vol = SFX_MAX_VOL;
        vol = (long)car->Body->BangMag / 10;
        if (vol > SFX_MAX_VOL) vol = SFX_MAX_VOL;

        freq = 22050;

        PlaySfx3D(SFX_HIT2, vol, freq, &car->Body->Centre.Pos, 1);
#else // !OLD_AUDIO
        g_SoundEngine.PlaySubmixedSound( EFFECT_Hit2, 
                                         FALSE, 
                                         car->pSourceMix );
        
#endif // !OLD_AUDIO
    }
#endif

#ifdef _PSX
    if( car->Body->BangMag > TO_VEL(Real(500)) )
    {
        vol = FROM_VEL( car->Body->BangMag ) >> 12;
        if( vol > 0x7fff )
            vol = 0x7fff;
#if DEBUG
//      printf( "vol = %d\n", vol );
#endif

        // See if two cars have collided
        sfx = SAMPLE_BUMP_CAR_TO_ENV;
        for( other_player = PLR_PlayerHead; other_player != NULL; other_player = other_player ->next)
        {
            if( player == other_player )
                continue;

            // Have the players collided?
            if( !HaveCarsCollided( other_player->ownobj, player->ownobj) )
                continue;

            sfx = SAMPLE_BUMP_CAR_TO_CAR;
        }

        SFX_Play3D( sfx, vol, 1024, 0, &car->Body->Centre.Pos );
        if( SplitScreenMode == 2 )
        {
            SFX_SecondPlayerFlag = TRUE;
            SFX_Play3D( sfx, vol, 1024, 0, &car->Body->Centre.Pos );
            SFX_SecondPlayerFlag = FALSE;
        }
    }
#endif

    car->Body->Banged = FALSE;
    car->Body->BangMag = ZERO;

// countdown turbo boost?

#ifndef _PSX
    if (player->type == PLAYER_LOCAL)
    {
        if (CountdownTime && !player->AccelerateTimeStamp && player->controls.lastdy < 0)
        {
            player->AccelerateTimeStamp = CountdownTime;
        }

        if (!CountdownTime && player->AccelerateTimeStamp > 0)
        {
            if (player->AccelerateTimeStamp > 800 && player->AccelerateTimeStamp < 900)
            {
                VecEqScalarVec(&vec, car->Body->Centre.Mass * MPH2OGU_SPEED * 5, &car->Body->Centre.WMatrix.mv[L]);
                ApplyBodyImpulse(car->Body, &vec, &ZeroVector);
            }
            player->AccelerateTimeStamp = 0;
        }
    }
#endif
}

////////////////////
// barrel handler //
////////////////////
#ifndef _PSX

void AI_BarrelHandler(OBJECT *obj)
{
    MAT mat, mat2;
    BARREL_OBJ *barrel = (BARREL_OBJ*)obj->Data;

// quit if not visible

    if (!obj->renderflag.visible)
        return;

// spin

    RotMatrixX(&mat, barrel->SpinSpeed * TimeStep * 72);
    MulMatrix(&obj->body.Centre.WMatrix, &mat, &mat2);
    NormalizeMatrix(&mat2);
    CopyMat(&mat2, &obj->body.Centre.WMatrix);
}
#endif

////////////////////
// planet handler //
////////////////////
#ifndef _PSX
void AI_PlanetHandler(OBJECT *obj)
{
    long i;
    MAT mat, mat2;
    VEC vec;
    OBJECT *findobj, *findsun;
    REAL len;
    PLANET_OBJ *planet = (PLANET_OBJ*)obj->Data, *findplanet;
    SUN_OBJ *sun = (SUN_OBJ*)obj->Data;

// get orbit object?

    if (!obj->objref)
    {
        for (findobj = OBJ_ObjectHead ; findobj ; findobj = findobj->next)
        {
            findplanet = (PLANET_OBJ*)findobj->Data;
            if (findobj->Type == OBJECT_TYPE_PLANET && findplanet->OwnPlanet == planet->OrbitPlanet)
            {
                obj->objref = findobj;

                for (findsun = OBJ_ObjectHead ; findsun ; findsun = findsun->next)
                    if (findsun->Type == OBJECT_TYPE_PLANET && ((PLANET_OBJ*)findsun->Data)->OwnPlanet == PLANET_SUN)
                        planet->VisiMask = ((SUN_OBJ*)(findsun->Data))->VisiMask;

                break;
            }
        }

        if (!obj->objref)
            return;

        if (planet->OwnPlanet != PLANET_SUN)
        {
// setup orbit info
            SubVector(&obj->body.Centre.Pos, &obj->objref->body.Centre.Pos, &vec);
            len = Length(&vec);
            SetVector(&planet->OrbitOffset, 0, 0, len);

            BuildLookMatrixForward(&obj->objref->body.Centre.Pos, &obj->body.Centre.Pos, &mat2);
//          RotMatrixZ(&mat, ((float)rand() / RAND_MAX - 0.5f) / 3.0f);
            RotMatrixZ(&mat, Real(0.1f));
            MulMatrix(&mat2, &mat, &planet->OrbitMatrix);
        }
    }

// rotate on orbit
    if (planet->OwnPlanet != planet->OrbitPlanet)
    {
        RotMatrixX(&mat, planet->OrbitSpeed * TimeStep * 72);
        MulMatrix(&planet->OrbitMatrix, &mat, &mat2);
        NormalizeMatrix(&mat2);
        CopyMat(&mat2, &planet->OrbitMatrix);
        RotTransVector(&planet->OrbitMatrix, &obj->objref->body.Centre.Pos, &planet->OrbitOffset, &obj->body.Centre.Pos);
    }

// quit if not visible

//  if (planet->OwnPlanet != PLANET_SUN && !obj->renderflag.visible)
    if (!obj->renderflag.visible)
        return;

// spin on local Y axis

    RotMatrixY(&mat, planet->SpinSpeed * TimeStep * 72);
    MulMatrix(&obj->body.Centre.WMatrix, &mat, &mat2);
    NormalizeMatrix(&mat2);
    CopyMat(&mat2, &obj->body.Centre.WMatrix);

// sun?

    if (planet->OwnPlanet != PLANET_SUN)
        return;

// maintain overlays

    for (i = 0 ; i < SUN_OVERLAY_NUM ; i++)
    {
//      sun->Overlay[i].RotVel += frand(0.0001f) - 0.00005f;
//      if (sun->Overlay[i].RotVel < -0.001f) sun->Overlay[i].RotVel = -0.001f;
//      else if (sun->Overlay[i].RotVel > 0.001f) sun->Overlay[i].RotVel = 0.001f;

        sun->Overlay[i].Rot += sun->Overlay[i].RotVel * TimeStep * 72;

        sun->Overlay[i].r += (rand() % 5) - 2;
        if (sun->Overlay[i].r > 128) sun->Overlay[i].r = 128;
        else if (sun->Overlay[i].r < 96) sun->Overlay[i].r = 96;

        sun->Overlay[i].g += (rand() % 5) - 2;
        if (sun->Overlay[i].g > 128) sun->Overlay[i].g = 128;
        else if (sun->Overlay[i].g < 96) sun->Overlay[i].g = 96;

        sun->Overlay[i].b += (rand() % 5) - 2;
        if (sun->Overlay[i].b > 96) sun->Overlay[i].b = 96;
        else if (sun->Overlay[i].b < 64) sun->Overlay[i].b = 64;

        sun->Overlay[i].rgb = (sun->Overlay[i].r << 16) | (sun->Overlay[i].g << 8) | sun->Overlay[i].b;
    }
}
#endif

///////////////////
// plane handler //
///////////////////
#ifndef _PSX
void AI_PlaneHandler(OBJECT *obj)
{
    MAT mat;
    PLANE_OBJ *plane = (PLANE_OBJ*)obj->Data;

// get world mat / pos

    plane->Rot += plane->Speed * TimeStep * 72;
    RotMatrixY(&mat, plane->Rot);
    MulMatrix(&mat, &plane->BankMatrix, &obj->body.Centre.WMatrix);
    RotTransVector(&mat, &plane->GenPos, &plane->Offset, &obj->body.Centre.Pos);

// set propellor world pos / mat

    RotMatrixZ(&mat, (float)TIME2MS(CurrentTimer()) / 500.0f);
    MulMatrix(&obj->body.Centre.WMatrix, &mat, &plane->PropMatrix);
    RotTransVector(&obj->body.Centre.WMatrix, &obj->body.Centre.Pos, &PlanePropOff, &plane->PropPos);

// update sfx pos

#ifndef _PSX
  #ifdef OLD_AUDIO
    if (obj->Sfx3D)
    {
        CopyVec(&obj->body.Centre.Pos, &obj->Sfx3D->Pos);
    }
  #endif // OLD_AUDIO
#endif
}

////////////////////
// copter handler //
////////////////////
void AI_CopterHandler(OBJECT *obj)
{
    MAT mat;
    COPTER_OBJ *copter = (COPTER_OBJ*)obj->Data;

// move the copter
    switch (copter->State) {
    case COPTER_WAIT:
        CopterWait(obj);
        break;
    case COPTER_TURNING:
        TurnCopter(obj);
        break;
    case COPTER_FLYING:
        FlyCopter(obj);
        break;
    default:
        break;
    }


// set blade world pos / mat

    RotMatrixY(&mat, (float)TIME2MS(CurrentTimer()) / 500.0f);
    MulMatrix(&obj->body.Centre.WMatrix, &mat, &copter->BladeMatrix1);
    RotTransVector(&obj->body.Centre.WMatrix, &obj->body.Centre.Pos, &CopterBlade1Off, &copter->BladePos1);

    RotMatrixX(&mat, (float)TIME2MS(CurrentTimer()) / 400.0f);
    MulMatrix(&obj->body.Centre.WMatrix, &mat, &copter->BladeMatrix2);
    RotTransVector(&obj->body.Centre.WMatrix, &obj->body.Centre.Pos, &CopterBlade2Off, &copter->BladePos2);

// update sfx pos
#ifndef _PSX
  #ifdef OLD_AUDIO
    if (obj->Sfx3D)
    {
        CopyVec(&obj->body.Centre.Pos, &obj->Sfx3D->Pos);
    }
  #endif OLD_AUDIO
#endif
}


void TurnCopter(OBJECT *obj)
{
    COPTER_OBJ *copter = (COPTER_OBJ*)obj->Data;

    //if (copter->TurnTime >= 5) {
    //  NewCopterDest(obj);
        copter->State = COPTER_FLYING;
        return;
    //}

    // Interpolate to new destination quaternion
    SLerpQuat(&copter->OldInitialQuat, &copter->InitialQuat, copter->TurnTime / 5, &copter->CurrentUpQuat);
    NormalizeQuat(&copter->CurrentUpQuat);
    CopyQuat(&copter->CurrentUpQuat, &obj->body.Centre.Quat);
    copter->TurnTime += TimeStep;

    // Set the copters matrix
    QuatToMat(&obj->body.Centre.Quat, &obj->body.Centre.WMatrix);

}


void FlyCopter(OBJECT *obj)
{
    bool reachedDest;
    REAL dRLen, velDest, vel, t;
    VEC dR, axis;
    QUATERNION dQuat;
    COPTER_OBJ *copter = (COPTER_OBJ*)obj->Data;

    reachedDest = FALSE;

    // Get distance from destination
    VecMinusVec(&obj->body.Centre.Pos, &copter->Destination, &dR);
    dRLen = VecLen(&dR);

    // If have reached destination, choose a new one
    if (dRLen < 10) {
        reachedDest = TRUE;
    }

    // accelerate/ decelerate towards destination
    velDest = (dRLen);
    if (velDest > copter->MaxVel) velDest = copter->MaxVel;

    vel = VecLen(&obj->body.Centre.Vel);
    if (vel < velDest) {
        vel += copter->Acc * TimeStep;
    } else if (vel > velDest) {
        vel = velDest;
    }
    if (vel < ZERO) {
        vel = ZERO;
        reachedDest = TRUE;
    }

    // Move the copter
    VecEqScalarVec(&obj->body.Centre.Vel, vel, &copter->Direction);
    VecPlusEqScalarVec(&obj->body.Centre.Pos, TimeStep, &obj->body.Centre.Vel);


    // Interpolate to new destination quaternion
    t = HALF * ((float)sin(((copter->TurnTime - 2.5f) / 5) * PI) + ONE);
    SLerpQuat(&copter->OldInitialQuat, &copter->InitialQuat, t, &copter->CurrentUpQuat);
    NormalizeQuat(&copter->CurrentUpQuat);
    copter->TurnTime += TimeStep;
    if (copter->TurnTime >= 5) copter->TurnTime = 5;


    VecCrossVec(&copter->Direction, &UpVec, &axis);
    VecMulScalar(&axis, -0.15f * vel / copter->MaxVel);
    //VecMulQuat(&axis, &copter->InitialQuat, &dQuat);
    //QuatPlusQuat(&copter->InitialQuat, &dQuat, &obj->body.Centre.Quat);
    VecMulQuat(&axis, &copter->CurrentUpQuat, &dQuat);
    QuatPlusQuat(&copter->CurrentUpQuat, &dQuat, &obj->body.Centre.Quat);
    NormalizeQuat(&obj->body.Centre.Quat);
    QuatToMat(&obj->body.Centre.Quat, &obj->body.Centre.WMatrix);

    if (reachedDest) {  
        NewCopterDest(obj);
        copter->State = COPTER_TURNING;
    }

}

void CopterWait(OBJECT *obj)
{
    REAL dPosLen;
    COPTER_OBJ *copter = (COPTER_OBJ*)obj->Data;

    // wait for 2 seconds
    copter->TurnTime += TimeStep;
    if (copter->TurnTime > 2) {

        // Choose new destination
        SetVec(&copter->Destination,
            obj->body.Centre.Pos.v[X] - 600 * obj->body.Centre.WMatrix.mv[L].v[X],
            copter->FlyBox.YMin,
            obj->body.Centre.Pos.v[Z] - 600 * obj->body.Centre.WMatrix.mv[L].v[Z]);
        VecMinusVec(&copter->Destination, &obj->body.Centre.Pos, &copter->Direction);
        dPosLen = VecLen(&copter->Direction);
        VecDivScalar(&copter->Direction, dPosLen);
        CopyQuat(&copter->CurrentUpQuat, &copter->OldInitialQuat);
        copter->State = COPTER_FLYING;
    }
}


void NewCopterDest(OBJECT *obj)
{
    int its;
    MAT newMat;
    REAL dPosLen, lookLen;
    VEC look;
    COPTER_OBJ *copter = (COPTER_OBJ*)obj->Data;

    SetVecZero(&obj->body.Centre.Vel);
    CopyQuat(&copter->CurrentUpQuat, &copter->OldInitialQuat);
    copter->TurnTime = ZERO;

    // Choose a new destination
    its = 0;
    do {
        SetVec(&copter->Destination, 
            copter->FlyBox.XMin + frand(ONE) * (copter->FlyBox.XMax - copter->FlyBox.XMin),
            copter->FlyBox.YMin + frand(ONE) * (copter->FlyBox.YMax - copter->FlyBox.YMin),
            copter->FlyBox.ZMin + frand(ONE) * (copter->FlyBox.ZMax - copter->FlyBox.ZMin));
        VecMinusVec(&copter->Destination, &obj->body.Centre.Pos, &copter->Direction);
        dPosLen = VecLen(&copter->Direction);
    } while (dPosLen < Real(1000) && ++its < 10);
    VecDivScalar(&copter->Direction, dPosLen);

    // Choose a new default orientation
    if (frand(ONE) > Real(0.2)) {
        SetVec(&look, copter->Direction.v[X], ZERO, copter->Direction.v[Z]);
        lookLen = VecLen(&look);
        if (lookLen > SMALL_REAL) {
            VecDivScalar(&look, lookLen);
            CopyVec(&look, &newMat.mv[L]);
            CopyVec(&DownVec, &newMat.mv[U]);
            VecCrossVec(&DownVec, &look, &newMat.mv[R]);
            MatToQuat(&newMat, &copter->InitialQuat);
        }
    }

    copter->State = COPTER_FLYING;
}
#endif //ifndef _PSX

////////////////////
// dragon handler //
////////////////////
#ifdef _PC
void AI_DragonHandler(OBJECT *obj)
{
    long i, j, col;
    MODEL *model;
    VEC vec;
    MAT mat;
    DRAGON_OBJ *dragon = (DRAGON_OBJ*)obj->Data;

// quit if not visible and waiting

    if (!obj->renderflag.visible && dragon->Count > 6.0f)
        return;

// get morph model

    if (dragon->HeadModel)
        model = &LevelModel[dragon->HeadModel].Model;
    else
        model = NULL;

// inc anim count

    dragon->Count += TimeStep;
    if (dragon->Count > 8.0f) dragon->Count -= 8.0f;

// handle morphs

    if (model)
    {
        if (dragon->Count <= 2.0f)
        {
            SetModelMorph(model, 0, 1, dragon->Count / 2.0f);
        }

        else if (dragon->Count <= 4.0f)
        {
            if (dragon->Count < 2.2f)
                SetModelMorph(model, 0, 1, (float)sin((dragon->Count - 2.0f) * 2.5f * RAD) * 0.03f + 1.0f);
            else
                SetModelMorph(model, 1, 0, 0);
        }

        else if (dragon->Count <= 6.0f)
        {
            SetModelMorph(model, 0, 1, (6.0f - dragon->Count) / 2.0f);
        }

        else
        {
            if (dragon->Count < 6.2f)
                SetModelMorph(model, 1, 0, (float)sin((dragon->Count - 6.0f) * 2.5f * RAD) * 0.03f + 1.0f);
            else
                SetModelMorph(model, 0, 0, 0);
        }
    }

// firestarter?

    if (dragon->Count > 2.0f && dragon->Count < 4.0f)
    {

// yep, gen light?

        if (!obj->Light)
        {
            obj->Light = AllocLight();
            if (obj->Light)
            {
                CopyVec(&dragon->FireGenPoint, (VEC*)&obj->Light->x);
                obj->Light->Reach = 1024;
                obj->Light->Flag = LIGHT_FIXED | LIGHT_MOVING;
                obj->Light->Type= LIGHT_OMNINORMAL;
                obj->Light->r = 0;
                obj->Light->g = 0;
                obj->Light->b = 0;

#ifdef OLD_AUDIO
                obj->Sfx3D = CreateSfx3D(SFX_TOY_DRAGON, SFX_MAX_VOL, 22050, FALSE, &dragon->FireGenPoint, 0);
#else
                g_SoundEngine.Play3DSound( g_dwLevelSoundsOffset + EFFECT_toy_dragon, 
                                           FALSE, 
                                           dragon->FireGenPoint.v[0],
                                           dragon->FireGenPoint.v[1],
                                           dragon->FireGenPoint.v[2],
                                           &obj->pSfxInstance );
#endif // OLD_AUDIO
            }
        }       

// flicker light?

        if (obj->Light)
        {
            if (dragon->Count < 2.5f)
            {
                FTOL((dragon->Count - 2.0f) * 448, obj->Light->r);
                obj->Light->r += rand() & 31;
            }
            else
            {
                obj->Light->r = (rand() & 31) + 224;
            }

            obj->Light->g = obj->Light->r >> 1;
        }

// new fire?

        if ((long)(CurrentTimer() - dragon->FireGenTime) >= 0)
        {
            
            for (j = 0 ; j < 2 ; j++) for (i = 0 ; i < DRAGON_FIRE_NUM ; i++) if (!dragon->Fire[i].Time)
            {
                SetVector(&vec, 0, 0, frand(32));
                RotMatrixZYX(&mat, 0, frand(1.0f), frand(0.5f) - 0.25f);
                RotTransVector(&mat, &dragon->FireGenPoint, &vec, &dragon->Fire[i].Pos);

                dragon->Fire[i].Time = 0.5f;
                dragon->Fire[i].MinSize = frand(8) + 16;
                dragon->Fire[i].Spin = frand(1);
                dragon->Fire[i].SpinSpeed = frand(0.02f) - 0.01f;

                dragon->FireGenTime = CurrentTimer() + MS2TIME(20);
                break;
            }
        }
    }

// kill light + sfx?

    else
    {
        if (obj->Light)
        {
            if (dragon->Count < 4.5f)
            {
                FTOL((4.5f - dragon->Count) * 448, obj->Light->r);
                obj->Light->r += rand() & 31;
                obj->Light->g = obj->Light->r >> 1;
            }
            else
            {
                FreeLight(obj->Light);
                obj->Light = NULL;

#ifdef OLD_AUDIO
                if (obj->Sfx3D) if (!obj->Sfx3D->Sample)
                {
                    FreeSfx3D(obj->Sfx3D);
                    obj->Sfx3D = NULL;
                }
#else
                if( obj->pSfxInstance )
                {
                    g_SoundEngine.ReturnInstance( obj->pSfxInstance );
                    obj->pSfxInstance = NULL;
                }
#endif // OLD_AUDIO
            }
        }
    }

// maintain existing fire

    for (i = 0 ; i < DRAGON_FIRE_NUM ; i++) if (dragon->Fire[i].Time)
    {
        dragon->Fire[i].Pos.v[X] += dragon->FireGenDir.v[X] * TimeStep * 72;
        dragon->Fire[i].Pos.v[Y] += dragon->FireGenDir.v[Y] * TimeStep * 72;
        dragon->Fire[i].Pos.v[Z] += dragon->FireGenDir.v[Z] * TimeStep * 72;

//      dragon->Fire[i].Size = dragon->Fire[i].Time * 32 + dragon->Fire[i].MinSize;
        dragon->Fire[i].Size = (0.5f - dragon->Fire[i].Time) * 32.0f + dragon->Fire[i].MinSize;

        dragon->Fire[i].Spin += dragon->Fire[i].SpinSpeed * TimeStep * 72;
        RotMatrixZ(&dragon->Fire[i].Matrix, dragon->Fire[i].Spin);

        FTOL(dragon->Fire[i].Time * 511, col);
        dragon->Fire[i].rgb = col | (col << 8) | (col << 16);

        dragon->Fire[i].Time -= TimeStep;
        if (dragon->Fire[i].Time < 0) dragon->Fire[i].Time = 0;
    }
}
#endif

///////////////////
// water handler //
///////////////////
#ifdef _PC
void AI_WaterHandler(OBJECT *obj)
{
    long i;
    WATER_OBJ *water = (WATER_OBJ*)obj->Data;
    MODEL *model = &LevelModel[obj->DefaultModel].Model;
    WATER_VERTEX *wv;
    MODEL_VERTEX *mv;
    MODEL_POLY *mp;
    POLY_RGB *mrgb;
    VEC vec1, vec2, norm;

// quit if not visible

    if (!obj->renderflag.visible)
        return;

// move verts

    wv = water->Vert;
    mv = model->VertPtr;

    for (i = 0 ; i < water->VertNum ; i++, wv++, mv++)
    {
        wv->Time += TimeStep;
        while (wv->Time >= wv->TotalTime) wv->Time -= wv->TotalTime;
        mv->y = wv->Height + (float)sin(wv->Time / wv->TotalTime * RAD) * water->Scale;

        mv->nx = mv->ny = mv->nz = 0;
        mv->a = 0;
    }

// calc vert normals + uv's

    mp = model->PolyPtr;

    for (i = model->PolyNum ; i ; i--, mp++)
    {
        SubVector((VEC*)&mp->v1->x, (VEC*)&mp->v0->x, &vec1);
        SubVector((VEC*)&mp->v2->x, (VEC*)&mp->v0->x, &vec2);
        CrossProduct(&vec2, &vec1, &norm);
        NormalizeVector(&norm);

        AddVector((VEC*)&mp->v0->nx, &norm, (VEC*)&mp->v0->nx);
        mp->v0->a++;

        AddVector((VEC*)&mp->v1->nx, &norm, (VEC*)&mp->v1->nx);
        mp->v1->a++;

        AddVector((VEC*)&mp->v2->nx, &norm, (VEC*)&mp->v2->nx);
        mp->v2->a++;
    }

    mv = model->VertPtr;

    for (i = 0 ; i < water->VertNum ; i++, mv++)
    {
        mv->nx /= mv->a;
        mv->ny /= mv->a;
        mv->nz /= mv->a;

        mv->tu = mv->nx * 6.0f + 0.5f;
        mv->tv = mv->nz * 6.0f + 0.5f;

        FTOL((mv->ny + 1.0f) * 11000.0f + 192.0f, mv->a);
    }

// give vert uv's to poly uv's

    mp = model->PolyPtr;
    mrgb = model->PolyRGB;

    for (i = model->PolyNum ; i ; i--, mp++, mrgb++)
    {
        mp->tu0 = mp->v0->tu;
        mp->tv0 = mp->v0->tv;

        mp->tu1 = mp->v1->tu;
        mp->tv1 = mp->v1->tv;

        mp->tu2 = mp->v2->tu;
        mp->tv2 = mp->v2->tv;

        mrgb->rgb[0].a = (unsigned char)mp->v0->a;
        mrgb->rgb[1].a = (unsigned char)mp->v1->a;
        mrgb->rgb[2].a = (unsigned char)mp->v2->a;
    }   
}
#endif

#ifdef _N64
void AI_WaterHandler(OBJECT *obj)
{
    long            ii;
    WATER_OBJ       *water = (WATER_OBJ*)obj->Data;
    MODEL           *model = &LevelModel[obj->DefaultModel].Model;
    WATER_VERTEX    *wv;
    Vtx             *mv;

// quit if not visible
    if (!obj->renderflag.visible)
        return;

// quit if already performed on global model once
    if (GG_WaterAnim) return;

// move verts
    wv = water->Vert;
    mv = model->hdr->evtxptr;

    for (ii = 0; ii < water->VertNum; ii++, wv++, mv++)
    {
        wv->Time += TimeStep;
        while (wv->Time >= wv->TotalTime) wv->Time -= wv->TotalTime;
        mv->n.ob[1] = wv->Height + (float)sin(wv->Time / wv->TotalTime * RAD) * water->Scale;
        mv->n.n[0] = (char)((float)(sin(wv->Time / wv->TotalTime * RAD) * 32));
        mv->n.n[2] = (char)((float)(cosf(wv->Time / wv->TotalTime * RAD) * 32));
    }

    GG_WaterAnim = 1;
}
#endif

//////////////////
// boat handler //
//////////////////
#ifndef _PSX
void AI_BoatHandler(OBJECT *obj)
{
    BOAT_OBJ *boat = (BOAT_OBJ*)obj->Data;
    //VEC vec, vec2;
    MAT mat;

// quit if not visible

    if (!obj->renderflag.visible)
        return;

// update times

    boat->TimeX += TimeStep;
    while (boat->TimeX >= boat->TotalTimeX) boat->TimeX -= boat->TotalTimeX;

    boat->TimeHeight += TimeStep;
    while (boat->TimeHeight >= boat->TotalTimeHeight) boat->TimeHeight -= boat->TotalTimeHeight;

    boat->TimeZ += TimeStep;
    while (boat->TimeZ >= boat->TotalTimeZ) boat->TimeZ -= boat->TotalTimeZ;

// set height

    obj->body.Centre.Pos.v[Y] = boat->Height + (float)sin(boat->TimeHeight / boat->TotalTimeHeight * RAD) * 15.0f;

// set ori

    RotMatrixZYX(&mat, (float)sin(boat->TimeZ / boat->TotalTimeZ * RAD) / 90.0f, 0, (float)sin(boat->TimeX / boat->TotalTimeX * RAD) / 90.0f);
    MulMatrix(&boat->Ori, &mat, &obj->body.Centre.WMatrix);

// steam?

/*  boat->SteamTime += TimeStep;
    if (boat->SteamTime > Real(0.1f))
    {
        boat->SteamTime -= Real(0.1f);
        VecMulMat(&BoatSteamOffset, &obj->body.Centre.WMatrix, &vec);
        VecPlusEqVec(&vec, &obj->body.Centre.Pos);
        VecMulMat(&BoatSteamVel, &obj->body.Centre.WMatrix, &vec2);
        CreateSpark(SPARK_SMOKE2, &vec, &vec2, ZERO);
    }
*/


}

///////////////////
// radar handler //
///////////////////
void AI_RadarHandler(OBJECT *obj)
{
    RADAR_OBJ *radar = (RADAR_OBJ*)obj->Data;

// set matrix

    radar->Time += TimeStep;
    RotMatrixY(&obj->body.Centre.WMatrix, radar->Time * 0.75f);

// set light dirs

    if (obj->Light2)
        CopyVec(&obj->body.Centre.WMatrix.mv[R], &obj->Light2->DirMatrix.mv[L]);
    
    if (obj->Light)
        SetVector(&obj->Light->DirMatrix.mv[L], -obj->body.Centre.WMatrix.m[RX], -obj->body.Centre.WMatrix.m[RY], -obj->body.Centre.WMatrix.m[RZ]);
}

/////////////////////
// balloon handler //
/////////////////////
void AI_BalloonHandler(OBJECT *obj)
{
    BALLOON_OBJ *balloon = (BALLOON_OBJ*)obj->Data;

// quit if not visible

    if (!obj->renderflag.visible)
        return;

// bob

    balloon->Time += TimeStep;
    obj->body.Centre.Pos.v[Y] = balloon->Height + (float)sin(balloon->Time) * 16;
}

///////////////////
// horse handler //
///////////////////
void AI_HorseRipper(OBJECT *obj)
{
    HORSE_OBJ *horse = (HORSE_OBJ*)obj->Data;
    MAT mat;
    REAL rock;

// get rock num

    horse->Time += TimeStep * 4.0f;
    rock = (float)sin(horse->Time);

// creak?

    if (horse->CreakFlag > 0)
    {
        if (rock > horse->CreakFlag)
        {
            horse->CreakFlag = -horse->CreakFlag;
            #ifndef _PSX
#ifdef OLD_AUDIO
            PlaySfx3D(SFX_TOY_CREAK, SFX_MAX_VOL, 22050, &obj->body.Centre.Pos, 0);
#else // !OLD_AUDIO
            g_SoundEngine.Play3DSound( g_dwLevelSoundsOffset + EFFECT_toy_creak, FALSE, obj );
#endif // !OLD_AUDIO
            #endif
        }
    }
    else
    {
        if (rock < horse->CreakFlag)
        {
            horse->CreakFlag = -horse->CreakFlag;
            #ifndef _PSX
#ifdef OLD_AUDIO
            PlaySfx3D(SFX_TOY_CREAK, SFX_MAX_VOL, 20000, &obj->body.Centre.Pos, 0);
#else // !OLD_AUDIO
            g_SoundEngine.Play3DSound( g_dwLevelSoundsOffset + EFFECT_toy_creak, FALSE, obj );
#endif // !OLD_AUDIO
            #endif
        }
    }

// quit if not visible

    if (!obj->renderflag.visible)
        return;

// rock

    RotMatrixX(&mat, rock / 50.0f);
    MulMatrix(&horse->Mat, &mat, &obj->body.Centre.WMatrix);
}
#endif


///////////////////
// train handler //
///////////////////

void AI_TrainHandler(OBJECT *obj)
{
    long i, flag;
    TRAIN_OBJ *train = (TRAIN_OBJ*)obj->Data;
    VEC vec, vec2;
    PLAYER *player;

// rot wheels

    train->TimeFront -= TimeStep * Real(0.5f);
    train->TimeBack -= TimeStep * Real(0.3f);

    GoodWrap(&train->TimeBack, ZERO, ONE);
    GoodWrap(&train->TimeFront, ZERO, ONE);

// set wheel positions

    for (i = 0 ; i < 4 ; i++)
    {
        AddVector(&obj->body.Centre.Pos, &TrainWheelOffsets[i], &train->WheelPos[i]);
    }

// update sfx pos

#ifndef _PSX
  #ifdef OLD_AUDIO
    if (obj->Sfx3D)
    {
        CopyVec(&obj->body.Centre.Pos, &obj->Sfx3D->Pos);
    }
  #endif // OLD_AUDIO
#endif

// steam?

    AddVector(&obj->body.Centre.Pos, &TrainSteamOffset, &vec);

    train->SteamTime += TimeStep;
    if (train->SteamTime > Real(0.1f))
    {
        train->SteamTime -= Real(0.1f);
        #ifndef _PSX
        CreateSpark(SPARK_SMOKE2, &vec, &TrainSteamDir, ZERO, 0);
        #endif
    }

// whistle?

    flag = FALSE;

    for (player = PLR_PlayerHead ; player ; player = player->next)
    {
        SubVector(&obj->body.Centre.Pos, &player->car.Body->Centre.Pos, &vec2);
        if (abs(vec2.v[X]) < 400 && vec2.v[Z] > -400 && vec2.v[Z] < 800)
        {
            flag = TRUE;
            break;
        }
    }

    if (flag)
    {
        if (train->WhistleFlag)
        {
            train->WhistleFlag = FALSE;
#ifndef _PSX
  #ifdef OLD_AUDIO
            PlaySfx3D(SFX_TOY_WHISTLE, SFX_MAX_VOL, 22050, &vec, 0);
  #else // !OLD_AUDIO
            g_SoundEngine.Play3DSound( g_dwLevelSoundsOffset + EFFECT_toy_whistle, FALSE, vec.v[0], vec.v[1], vec.v[2] );
  #endif // !OLD_AUDIO
#endif
        }
    }
    else
    {
        train->WhistleFlag = TRUE;
    }
}

////////////////////
// strobe handler //
////////////////////
#ifndef _PSX

void AI_StrobeHandler(OBJECT *obj)
{
    long num, diff, per, col;
    STROBE_OBJ *strobe = (STROBE_OBJ*)obj->Data;

// get brightness

    num = (TIME2MS(CurrentTimer()) / 20) % strobe->StrobeCount;
    diff = num - strobe->StrobeNum;
    if (diff < -strobe->StrobeCount / 2) diff += strobe->StrobeCount;
    if (diff > strobe->StrobeCount / 2) diff -= strobe->StrobeCount;

// off

    if (diff < -strobe->FadeUp || diff > strobe->FadeDown)
    {
        if (obj->Light)
        {
            FreeLight(obj->Light);
            obj->Light = NULL;
        }
        strobe->Glow = 0;
    }

// on

    else
    {
        if (!obj->Light)
        {
            obj->Light = AllocLight();
            if (obj->Light)
            {
                obj->Light->x = strobe->LightPos.v[X];
                obj->Light->y = strobe->LightPos.v[Y];
                obj->Light->z = strobe->LightPos.v[Z];
                obj->Light->Reach = strobe->Range;
                obj->Light->Flag = LIGHT_FIXED | LIGHT_MOVING;
                obj->Light->Type= LIGHT_OMNI;
            }
        }

        if (obj->Light)
        {
            if (diff < 0) per = (diff + strobe->FadeUp) * (100 / strobe->FadeUp);
            else per = (strobe->FadeDown - diff) * (100 / strobe->FadeDown);

            obj->Light->r = strobe->r * per / 100;
            obj->Light->g = strobe->g * per / 100;
            obj->Light->b = strobe->b * per / 100;

            col = (obj->Light->r << 16) | (obj->Light->g << 8) | obj->Light->b;
        }
        strobe->Glow = (float)per / 100.0f;
    }
}
#endif

/////////////////////////////////////////////////////////////////////
//
// Spark Generator Handler
//
/////////////////////////////////////////////////////////////////////
#ifndef _PSX
void SparkGenHandler(OBJECT *obj)
{
    int ii, nTries;
    VEC pos, vel;
    SPARK_GEN *sparkGen = (SPARK_GEN *)obj->Data;

    // update time for this generator
    sparkGen->Time += TimeStep;

    // make sure it is visible
    if (CamVisiMask & sparkGen->VisiMask) return;

    nTries = 1 + (int)(TimeStep / sparkGen->MaxTime);
    if (nTries > 5) nTries = 5;
    for (ii = 0; ii < nTries; ii++) {
        
        // See if a new spark should be generated
        if (frand(ONE) > sparkGen->Time / sparkGen->MaxTime) continue;
        sparkGen->Time = ZERO;

        if (sparkGen->Parent != NULL) {
            // Calculate average spark velocity and start position
            VecMulMat(&obj->body.Centre.Pos, &sparkGen->Parent->body.Centre.WMatrix, &pos);
            VecPlusEqVec(&pos, &sparkGen->Parent->body.Centre.Pos);
            VecMulMat(&sparkGen->SparkVel, &sparkGen->Parent->body.Centre.WMatrix, &vel);
    
            // Generate the object-relative spark
            CreateSpark(sparkGen->Type, &pos, &vel, sparkGen->SparkVelVar, sparkGen->VisiMask);
        } else {
            // Generate the spark
            CreateSpark(sparkGen->Type, &obj->body.Centre.Pos, &sparkGen->SparkVel, sparkGen->SparkVelVar, sparkGen->VisiMask);
        }
    }
}
#endif

//////////////////////
// spaceman handler //
//////////////////////

#ifdef _PC
void AI_SpacemanHandler(OBJECT *obj)
{
    MAT mat1, mat2;
    SPACEMAN_OBJ *spaceman = (SPACEMAN_OBJ*)obj->Data;

    RotMatrixZYX(&mat1, 0.001f, 0.001f, 0);
    MulMatrix(&obj->body.Centre.WMatrix, &mat1, &mat2);
    CopyMat(&mat2, &obj->body.Centre.WMatrix);
}
#endif

////////////////////////////
// dissolve model handler //
////////////////////////////

#ifdef _PC
void AI_DissolveModelHandler(OBJECT *obj)
{
    long i, alpha, col;
    MODEL_POLY *mp;
    POLY_RGB *mrgb;
    DISSOLVE_OBJ *dissolve = (DISSOLVE_OBJ*)obj->Data;
    DISSOLVE_PARTICLE *particle = (DISSOLVE_PARTICLE*)(dissolve->Model.VertPtr + dissolve->Model.VertNum);
    VEC centre, delta[4];
    MAT mat;

// loop thru polys

    FTOL(255.0f - dissolve->Age * 127, alpha);
    mp = dissolve->Model.PolyPtr;
    mrgb = dissolve->Model.PolyRGB;

    for (i = 0 ; i < dissolve->Model.PolyNum ; i++, mp++, mrgb++, particle++)
    {

// set alpha

        mrgb->rgb[0].a = (unsigned char)alpha;
        mrgb->rgb[1].a = (unsigned char)alpha;
        mrgb->rgb[2].a = (unsigned char)alpha;
        mrgb->rgb[3].a = (unsigned char)alpha;

// get centre + vector offsets

        centre.v[X] = (mp->v0->x + mp->v1->x + mp->v2->x + mp->v3->x) / 4.0f;
        centre.v[Y] = (mp->v0->y + mp->v1->y + mp->v2->y + mp->v3->y) / 4.0f;
        centre.v[Z] = (mp->v0->z + mp->v1->z + mp->v2->z + mp->v3->z) / 4.0f;

        SubVector((VEC*)&mp->v0->x, &centre, &delta[0]);
        SubVector((VEC*)&mp->v1->x, &centre, &delta[1]);
        SubVector((VEC*)&mp->v2->x, &centre, &delta[2]);
        SubVector((VEC*)&mp->v3->x, &centre, &delta[3]);

// spin points + add back centre

        RotMatrixZYX(&mat, particle->Rot.v[X] * TimeStep, particle->Rot.v[Y] * TimeStep, particle->Rot.v[Z] * TimeStep);

        RotTransVector(&mat, &centre, &delta[0], (VEC*)&mp->v0->x);
        RotTransVector(&mat, &centre, &delta[1], (VEC*)&mp->v1->x);
        RotTransVector(&mat, &centre, &delta[2], (VEC*)&mp->v2->x);
        RotTransVector(&mat, &centre, &delta[3], (VEC*)&mp->v3->x);

// add velocity

        particle->Vel.v[Y] += 192.0f * TimeStep;

        VecPlusEqScalarVec((VEC*)&mp->v0->x, TimeStep, &particle->Vel);
        VecPlusEqScalarVec((VEC*)&mp->v1->x, TimeStep, &particle->Vel);
        VecPlusEqScalarVec((VEC*)&mp->v2->x, TimeStep, &particle->Vel);
        VecPlusEqScalarVec((VEC*)&mp->v3->x, TimeStep, &particle->Vel);
    }

// set env rgb

    FTOL(255.0f - dissolve->Age * 127, col);
    ((MODEL_RGB*)&dissolve->EnvRGB)->r = (unsigned char)( ((MODEL_RGB*)&obj->EnvRGB)->r * col / 256 );
    ((MODEL_RGB*)&dissolve->EnvRGB)->g = (unsigned char)( ((MODEL_RGB*)&obj->EnvRGB)->g * col / 256 );
    ((MODEL_RGB*)&dissolve->EnvRGB)->b = (unsigned char)( ((MODEL_RGB*)&obj->EnvRGB)->b * col / 256 );
//$ADDITION - typecast to (unsigned char) above, to avoid compiler warning

// inc age

    dissolve->Age += TimeStep;
    if (dissolve->Age > 2.0f)
        OBJ_FreeObject(obj);
}
#endif

/////////////////////////////////////////////////////////////////////
// LaserHandler:
/////////////////////////////////////////////////////////////////////

void AI_LaserHandler(OBJECT *obj)
{
#ifndef _PSX
    VEC vel, pos;
    LASER_OBJ *laser = (LASER_OBJ *)obj->Data;

#ifdef OLD_AUDIO
    // change sfx?
    if (obj->Sfx3D)
    {
        if (laser->Dist < ONE)
        {
            laser->AlarmTimer = Real(5);
            if (obj->Sfx3D->Num == SFX_MUSE_LASER)
            {
                ChangeSfxSample3D(obj->Sfx3D, SFX_MUSE_ALARM);
            }   
        }
        else if (laser->Dist >= ONE)
        {
            laser->AlarmTimer -= TimeStep;
            if (laser->AlarmTimer < Real(0) && obj->Sfx3D->Num == SFX_MUSE_ALARM)
            {
                ChangeSfxSample3D(obj->Sfx3D, SFX_MUSE_LASER);
            }
        }
    }
#else
    if( obj->pSfxInstance )
    {
        if( laser->Dist < ONE )
        {
            laser->AlarmTimer = Real(5);
            if( obj->pSfxInstance->m_dwEffect == g_dwLevelSoundsOffset + EFFECT_museum_laserhum )
            {
                g_SoundEngine.ReturnInstance( obj->pSfxInstance );
                obj->pSfxInstance = NULL;
                g_SoundEngine.Play3DSound( g_dwLevelSoundsOffset + EFFECT_museum_alarm2,
                                           TRUE,
                                           obj,
                                           &obj->pSfxInstance );
            }
        }
        else if ( laser->Dist >= ONE )
        {
            laser->AlarmTimer -= TimeStep;
            if( laser->AlarmTimer < Real(0) && 
                obj->pSfxInstance->m_dwEffect == g_dwLevelSoundsOffset + EFFECT_museum_alarm2 )
            {
                g_SoundEngine.ReturnInstance( obj->pSfxInstance );
                obj->pSfxInstance = NULL;
                g_SoundEngine.Play3DSound( g_dwLevelSoundsOffset + EFFECT_museum_laserhum,
                                           TRUE,
                                           obj,
                                           &obj->pSfxInstance );
            }
        }
    }
#endif // OLD_AUDIO

    // make sure it is visible
    if (!obj->renderflag.visible) return;

    // Find the fractional distance from the laser source to intersection point with objects
    if (laser->ObjectCollide) {
        LineOfSightObj(&obj->body.Centre.Pos, &laser->Dest, &laser->Dist, NULL);
    } else {
        laser->Dist = ONE;
    }


    // Create sparks at the contact point
    if (laser->Dist < ONE) {
        VecEqScalarVec(&vel, -100, &obj->body.Centre.WMatrix.mv[L]);
        VecPlusScalarVec(&obj->body.Centre.Pos, laser->Dist, &laser->Delta, &pos)
        CreateSpark(SPARK_SPARK, &pos, &vel, 200, laser->VisiMask);
    }
#endif
}

////////////////////
// splash handler //
////////////////////

#ifndef _PSX
void AI_SplashHandler(OBJECT *obj)
{
    long i;
    SPLASH_OBJ *splash = (SPLASH_OBJ *)obj->Data;
    SPLASH_POLY *spoly;
    REAL grav;

// process each poly

//$MODIFIED
//    spoly = splash->Poly;
//    for (i = 0 ; i < SPLASH_POLY_NUM ; i++, spoly++)
//    {
    for (i = 0 ; i < SPLASH_POLY_NUM ; i++)
    {
        spoly = &(splash->Poly[i]);
        // Changed this due to a compiler optimizer bug.  (When loops get
        // unrolled, some values incremented in the loop can get incremented
        // too much.  See Xbox bug 11155 for more details.)
        //
        // Until we start using RTM version of VC7 (which fixes this bug), be
        // aware of this issue, especially when loops have a fixed number of
        // iterations and a pointer is being incremented.
//$END_MODIFICATIONS
        if (spoly->Frame < 16.0f)
        {
            spoly->Frame += spoly->FrameAdd * TimeStep;
            if (spoly->Frame >= 16.0f)
            {
                splash->Count--;
                continue;
            }

            grav = 384.0f * TimeStep;
            spoly->Vel[0].v[Y] += grav;
            spoly->Vel[1].v[Y] += grav;
            spoly->Vel[2].v[Y] += grav;
            spoly->Vel[3].v[Y] += grav;

            VecPlusEqScalarVec(&spoly->Pos[0], TimeStep, &spoly->Vel[0]);
            VecPlusEqScalarVec(&spoly->Pos[1], TimeStep, &spoly->Vel[1]);
            VecPlusEqScalarVec(&spoly->Pos[2], TimeStep, &spoly->Vel[2]);
            VecPlusEqScalarVec(&spoly->Pos[3], TimeStep, &spoly->Vel[3]);
        }
    }

// kill?

    if (!splash->Count)
    {
        OBJ_FreeObject(obj);
    }
}

#endif

/////////////////////////////////////////////////////////////////////
//
// Speedup handlers
//
/////////////////////////////////////////////////////////////////////
#ifndef _PSX
void AI_SpeedupAIHandler(OBJECT *obj)
{
    SPEEDUP_OBJ *speedup = (SPEEDUP_OBJ *)obj->Data;

    // does this speedup change state?
    if (speedup->ChangeTime == ZERO) return;

    // Is it time to change?
    speedup->Time += TimeStep;
    if (speedup->Time > speedup->ChangeTime) {
        if (speedup->Speed == speedup->LoSpeed) {
            speedup->Speed = speedup->HiSpeed;
        } else {
            speedup->Speed = speedup->LoSpeed;
        }
        speedup->Time = ZERO;
    }
}


void SpeedupImpulse(CAR *car)
{
    REAL time, depth, velDotNorm, vel, impMag;
    VEC dPos, wPos;
    BBOX bBox;
    OBJECT *obj;
    SPEEDUP_OBJ *speedup;

    // loop over objects checking for speedups
    for (obj = OBJ_ObjectHead; obj != NULL; obj = obj->next) {

        if (obj->Type != OBJECT_TYPE_SPEEDUP) continue;
        speedup = (SPEEDUP_OBJ *)obj->Data;

        // Check for collision between speedup and car
        if (!BBTestXZY(&speedup->CollPoly.BBox, &car->BBox)) continue;

        // Quick bounding-box test
        SetBBox(&bBox, 
            Min(car->Body->Centre.Pos.v[X], car->Body->Centre.OldPos.v[X]),
            Max(car->Body->Centre.Pos.v[X], car->Body->Centre.OldPos.v[X]),
            Min(car->Body->Centre.Pos.v[Y], car->Body->Centre.OldPos.v[Y]),
            Max(car->Body->Centre.Pos.v[Y], car->Body->Centre.OldPos.v[Y]),
            Min(car->Body->Centre.Pos.v[Z], car->Body->Centre.OldPos.v[Z]),
            Max(car->Body->Centre.Pos.v[Z], car->Body->Centre.OldPos.v[Z]));
        if(!BBTestYXZ(&bBox, &speedup->CollPoly.BBox)) continue;

        // Check for point passing through collision polygon
        if (!LinePlaneIntersect(&car->Body->Centre.OldPos, &car->Body->Centre.Pos, &speedup->CollPoly.Plane, &time, &depth)) {
            continue;
        }

        // Calculate the intersection point
        VecMinusVec(&car->Body->Centre.Pos, &car->Body->Centre.OldPos, &dPos);
        VecPlusScalarVec(&car->Body->Centre.OldPos, time, &dPos, &wPos);

        // Check intersection point is within the polygon boundary
        if (!PointInCollPolyBounds(&wPos, &speedup->CollPoly)) {
            continue;
        }

        // Make sure the particle is travelling towards the poly
        velDotNorm = VecDotVec(&car->Body->Centre.Vel, PlaneNormal(&speedup->CollPoly.Plane));
        if (velDotNorm > ZERO) {
            vel = VecDotVec(&car->Body->Centre.Vel, &car->Body->Centre.WMatrix.mv[L]);
            if (vel > ZERO) {
                impMag = car->Body->Centre.Mass * (speedup->Speed - vel);
            } else {
                impMag = car->Body->Centre.Mass * (-speedup->Speed - vel);
            }
        } else {
            vel = VecDotVec(&car->Body->Centre.Vel, &car->Body->Centre.WMatrix.mv[L]);
            if (vel > ZERO) {
                impMag = car->Body->Centre.Mass * (-speedup->LoSpeed - vel);
            } else {
                impMag = car->Body->Centre.Mass * (speedup->LoSpeed - vel);
            }
        }

        if (depth < ZERO) {
            VecPlusEqScalarVec(&car->Body->Centre.Shift, -2 * (depth - COLL_EPSILON), &speedup->CollPoly.Plane);
        }


        // Apply impulse to the car
        VecPlusEqScalarVec(&car->Body->Centre.Impulse, impMag, &car->Body->Centre.WMatrix.mv[L]);

    }
}

#endif


///////////////////////
// sprinkler handler //
///////////////////////
#ifdef _PC
void AI_SprinklerHandler(OBJECT *obj)
{
    SPRINKLER_OBJ *sprinkler = (SPRINKLER_OBJ*)obj->Data;
    long i;
    MAT mat;
    VEC vec, vel;
    REAL rot, mul;

// inc / dec reach?

    sprinkler->OnHoseTimer += TimeStep;
    if (sprinkler->OnHoseTimer > TO_TIME(Real(1))) sprinkler->OnHoseTimer = TO_TIME(Real(1));

    if (sprinkler->OnHoseTimer < TO_TIME(Real(0.2)))
    {
        if (sprinkler->Reach > 0.2f)
            sprinkler->Reach -= TimeStep * 0.5f;
    }
    else
    {
        if (sprinkler->Reach < 1.0f)
            sprinkler->Reach += TimeStep * 0.5f;
        else
            sprinkler->Reach = 1.0f;
    }

// rotate head

    sprinkler->HeadRot -= (TimeStep * 0.2f) * sprinkler->Reach;

// inc sine

    sprinkler->Sine += (TimeStep * 14.285f) * ((1.5f - sprinkler->Reach) * 2.0f);
    rot = (float)sin(sprinkler->Sine) * (0.04f * sprinkler->Reach);

// set head mat

    RotMatrixY(&mat, sprinkler->HeadRot + rot);
    MulMatrix(&obj->body.Centre.WMatrix, &mat, &sprinkler->HeadMat);

// spraying?

    if (rot < sprinkler->LastRot)
    {
        if (sprinkler->NextSfx)
        {
            sprinkler->NextSfx = FALSE;
#ifdef OLD_AUDIO
            PlaySfx3D(SFX_HOOD_SPRINKLER, (long)(SFX_MAX_VOL * sprinkler->Reach), 22050, &sprinkler->HeadPos, 0);
#else // !OLD_AUDIO
            // TODO (JHarding): This is not a hood-specific sound, so it should move to common
            if( obj->pSfxInstance )
            {
                g_SoundEngine.ReturnInstance( obj->pSfxInstance );
                obj->pSfxInstance = NULL;
            }
            g_SoundEngine.Play3DSound( EFFECT_Splash, 
                                       TRUE,
                                       sprinkler->HeadPos.v[0], 
                                       sprinkler->HeadPos.v[1], 
                                       sprinkler->HeadPos.v[2],
                                       &obj->pSfxInstance );
#endif // !OLD_AUDIO
        }

        if (obj->renderflag.visible) for (i = 0 ; i < 6 ; i++)
        {
            mul = (frand(0.8f) + 0.2f) * sprinkler->Reach;
            RotTransVector(&sprinkler->HeadMat, &sprinkler->HeadPos, &SprinklerJetOffset, &vec);
            RotVector(&sprinkler->HeadMat, &SprinklerJetVel, &vel);
            VecMulScalar(&vel, mul);
            CreateSpark(i & 1 ? SPARK_SPRINKLER : SPARK_SPRINKLER_BIG, &vec, &vel, 0.0f * mul, 0);
        }
    }

    if (rot >= sprinkler->LastRot)
    {
        sprinkler->NextSfx = TRUE;
    }

// save rot

    sprinkler->LastRot = rot;
}
#endif

////////////////////////////
// sprinkler hose handler //
////////////////////////////
#ifdef _PC
void AI_SprinklerHoseHandler(OBJECT *obj)
{
    SPRINKLER_HOSE_OBJ *hose = (SPRINKLER_HOSE_OBJ*)obj->Data;
    SPRINKLER_OBJ *sprinkler;
    OBJECT *objref;

// find brother sprinkler?

    if (!hose->Sprinkler) for (objref = OBJ_ObjectHead ; objref ; objref = objref->next) if (objref->Type == OBJECT_TYPE_SPRINKLER)
    {
        sprinkler = (SPRINKLER_OBJ*)objref->Data;
        if (sprinkler->ID == hose->ID)
        {
            hose->Sprinkler = sprinkler;
            break;
        }
    }

// reset collision detected flag
    //hose->Sprinkler->OnHose = FALSE;
}
#endif


///////////////////
// stream handler //
///////////////////
#ifdef _PC

void AI_StreamHandler(OBJECT *obj)
{
    long i;
    STREAM_OBJ *stream = (STREAM_OBJ*)obj->Data;
    MODEL *model = &LevelModel[obj->DefaultModel].Model;
    STREAM_VERTEX *wv;
    MODEL_VERTEX *mv;
    MODEL_POLY *mp;
    POLY_RGB *mrgb;
    VEC vec1, vec2, norm;

// quit if not visible

    if (!obj->renderflag.visible)
        return;

// move verts

    wv = stream->Vert;
    mv = model->VertPtr;

    for (i = 0 ; i < stream->VertNum ; i++, wv++, mv++)
    {
        wv->Time += TimeStep;
        while (wv->Time >= wv->TotalTime) wv->Time -= wv->TotalTime;
        mv->y = wv->Height + (float)sin(wv->Time / wv->TotalTime * RAD) * stream->Scale;

        mv->nx = mv->ny = mv->nz = 0;
        mv->a = 0;
    }

// calc vert normals + uv's

    mp = model->PolyPtr;

    for (i = model->PolyNum ; i ; i--, mp++)
    {
        SubVector((VEC*)&mp->v1->x, (VEC*)&mp->v0->x, &vec1);
        SubVector((VEC*)&mp->v2->x, (VEC*)&mp->v0->x, &vec2);
        CrossProduct(&vec2, &vec1, &norm);
        NormalizeVector(&norm);

        AddVector((VEC*)&mp->v0->nx, &norm, (VEC*)&mp->v0->nx);
        mp->v0->a++;

        AddVector((VEC*)&mp->v1->nx, &norm, (VEC*)&mp->v1->nx);
        mp->v1->a++;

        AddVector((VEC*)&mp->v2->nx, &norm, (VEC*)&mp->v2->nx);
        mp->v2->a++;
    }

    mv = model->VertPtr;

    for (i = 0 ; i < stream->VertNum ; i++, mv++)
    {
        mv->nx /= mv->a;
        mv->ny /= mv->a;
        mv->nz /= mv->a;

        mv->tu = mv->nz * 5.0f + stream->Vert[i].Uoff;
        mv->tv = mv->nx * 5.0f + stream->Vert[i].Voff;
    }

// give vert uv's to poly uv's

    mp = model->PolyPtr;
    mrgb = model->PolyRGB;

    for (i = model->PolyNum ; i ; i--, mp++, mrgb++)
    {
        mp->tu0 = mp->v0->tu;
        mp->tv0 = mp->v0->tv;

        mp->tu1 = mp->v1->tu;
        mp->tv1 = mp->v1->tv;

        mp->tu2 = mp->v2->tu;
        mp->tv2 = mp->v2->tv;
    }   
}
#endif

////////////////////////
// bang noise handler //
////////////////////////
#ifndef _PSX

#ifdef OLD_AUDIO
typedef struct {
    long ID, Sfx, VolOffset;
    REAL MinMag;
} BANG_NOISE_TABLE;

static BANG_NOISE_TABLE BangNoiseTable[] = {
    OBJECT_TYPE_BEACHBALL, SFX_BEACHBALL, 0, 300.0f,
    OBJECT_TYPE_BASKETBALL, SFX_HOOD_BASKETBALL, 0, 300.0f,
    OBJECT_TYPE_BOTTLE, SFX_BOTTLE, 200, 100.0f,
    OBJECT_TYPE_CONE, SFX_HOOD_CONE, -150, 300.0f,
    OBJECT_TYPE_ABC, SFX_TOY_BRICK, 200, 100.0f,
    OBJECT_TYPE_PACKET, SFX_MARKET_CARTON, 200, 100.0f,
#ifdef _PC
    OBJECT_TYPE_TUMBLEWEED, SFX_GHOST_TUMBLEWEED, 200, 100.0f,
#endif

    -1, -1,
};
#else // !OLD_AUDIO
typedef struct {
    long ID, Sfx;
    BOOL bLevel;
    REAL MinMag;
} BANG_NOISE_TABLE;

static BANG_NOISE_TABLE BangNoiseTable[] = {
    OBJECT_TYPE_BEACHBALL, EFFECT_BeachBall, FALSE, 300.0f, 
    OBJECT_TYPE_BASKETBALL, EFFECT_hood_basketball, TRUE, 300.0f, 
    OBJECT_TYPE_BOTTLE, EFFECT_Bottle, FALSE, 100.0f, 
    OBJECT_TYPE_CONE, EFFECT_hood_roadcone, TRUE, 300.0f, 
    OBJECT_TYPE_ABC, EFFECT_toy_toybrick, TRUE, 100.0f, 
    OBJECT_TYPE_PACKET, EFFECT_market_carton, TRUE, 100.0f, 
    OBJECT_TYPE_TUMBLEWEED, EFFECT_ghost_tumbweed, TRUE, 100.0f, 
    -1, -1, FALSE,
};
#endif // OLD_AUDIO

void AI_BangNoiseHandler(OBJECT *obj)
{
    long vol, i;

// find obj

    i = 0;
    while (BangNoiseTable[i].ID != obj->Type && BangNoiseTable[i].ID != -1) i++;
    if (BangNoiseTable[i].ID == -1)
    {
        return;
    }

// bounce sfx?

    if (obj->body.BangMag > TO_VEL(BangNoiseTable[i].MinMag))
    {
        if (BangNoiseTable[i].ID != -1)
        {
#ifdef OLD_AUDIO
            vol = (long)(obj->body.BangMag + BangNoiseTable[i].VolOffset) / 10;
            if (vol < SFX_MIN_VOL) vol = SFX_MIN_VOL;
            if (vol > SFX_MAX_VOL) vol = SFX_MAX_VOL;
#else
            // TODO (JHarding): Revisit levels here
            vol = -1000 + (long)(obj->body.BangMag) / 10;
#endif // OLD_AUDIO

#ifdef OLD_AUDIO
            PlaySfx3D(BangNoiseTable[i].Sfx, vol, 22050, &obj->body.Centre.Pos, 0);
#else // !OLD_AUDIO
            g_SoundEngine.Play3DSound( BangNoiseTable[i].Sfx + ( BangNoiseTable[i].bLevel ? g_dwLevelSoundsOffset : 0 ), 
                                       FALSE,
                                       obj->body.Centre.Pos.v[0], 
                                       obj->body.Centre.Pos.v[1], 
                                       obj->body.Centre.Pos.v[2] );
#endif // !OLD_AUDIO
        }

        obj->body.Banged = FALSE;
        obj->body.BangMag = ZERO;
    }
}
#endif

/////////////////////////
// star pickup handler //
/////////////////////////

#ifndef _PSX
void AI_StarHandler(OBJECT *obj)
{
    long col;
    REAL mul;
    STAR_OBJ *star = (STAR_OBJ*)obj->Data;
    PLAYER *player;
    LEVELINFO *levinfo;
    bool unlock;

// act on mode

    switch (star->Mode)
    {

// alive

        case 0:

// spin

            RotMatrixY(&obj->body.Centre.WMatrix, TIME2MS(CurrentTimer()) / -3000.0f);

// need a light source?

            if (!obj->Light)
            {
                obj->Light = AllocLight();
                if (obj->Light)
                {
                    CopyVec(&obj->body.Centre.Pos, (VEC*)&obj->Light->x);
                    obj->Light->Reach = 512;
                    obj->Light->Flag = LIGHT_FIXED | LIGHT_MOVING;
                    obj->Light->Type = LIGHT_OMNI;
                }
            }

// maintain light

            if (obj->Light)
            {
                col = 255;

                obj->Light->r = col / 2;
                obj->Light->g = col * 3 / 8;
                obj->Light->b = 0;
            }

// look for car collision

            for (player = PLR_PlayerHead ; player ; player = player->next)
            {

#ifdef _N64                 
                if (player->RaceFinishTime)
                    continue;
#endif

#ifdef _PC
                if (GameSettings.GameType == GAMETYPE_NETWORK_RACE && player->type == PLAYER_REMOTE)
                    continue;
#endif

// skip if race and player already has a pickup

                if ((player->PickupNum || player->PickupCycleSpeed) && (GameSettings.GameType == GAMETYPE_SINGLE || GameSettings.GameType == GAMETYPE_CLOCKWORK || GameSettings.GameType == GAMETYPE_NETWORK_RACE || GameSettings.GameType == GAMETYPE_CHAMPIONSHIP))
                    continue;

// got one

                if (BBTestXZY(&obj->body.CollSkin.BBox, &player->car.BBox)) 
                {
                    star->Mode = 1;
                    star->Timer = 0.0f;

                    SetVector(&star->Vel, player->ownobj->body.Centre.Vel.v[X] / 2.0f, -64.0f, player->ownobj->body.Centre.Vel.v[Z] / 2.0f);

#ifdef OLD_AUDIO
                    PlaySfx3D(SFX_PICKUP, SFX_MAX_VOL, 22050, &obj->body.Centre.Pos, 2);
#else // !OLD_AUDIO
                    g_SoundEngine.Play3DSound( EFFECT_Pickup, 
                                               FALSE,
                                               obj->body.Centre.Pos.v[0], 
                                               obj->body.Centre.Pos.v[1], 
                                               obj->body.Centre.Pos.v[2] );
#endif // !OLD_AUDIO

                    // become fox in battle mode
                    
#ifndef _N64                    
                    if (GameSettings.GameType == GAMETYPE_NETWORK_BATTLETAG && player == PLR_LocalPlayer)
#else
                    if (GameSettings.GameType == GAMETYPE_NETWORK_BATTLETAG)
#endif
                    {
#ifdef _PC
                        SendTransferFox(-1, player->PlayerID);
#endif
                        TransferFox(NULL, NULL, player);
                    }
                    // set star found if practice
                    else if (GameSettings.GameType == GAMETYPE_PRACTICE)
                    {
#ifndef _PSX
                        PracticeStarFlash = 2000;
#endif
                        if (GameSettings.Level < LEVEL_NCUP_LEVELS)
                        {
                            levinfo = GetLevelInfo(GameSettings.Level);
                            unlock = IsCupFoundPractiseStars(levinfo->LevelClass);
                            SetSecretFoundPractiseStars(GameSettings.Level);
                            if (!unlock && IsCupFoundPractiseStars(levinfo->LevelClass))
                                InitialMenuMessage = MENU_MESSAGE_NEWCARS;
                        }
                    }
                    // store star in 'got' list if training
                    else if (GameSettings.GameType == GAMETYPE_TRAINING)
                    {
                        StarList.ID[StarList.NumFound++] = (char)star->ID;
                        if (StarList.NumFound == StarList.NumTotal)
                        {
                            InitialMenuMessage = MENU_MESSAGE_COCKWORK;
                        }
                    }
                    // give global weapon if race
                    else if (GameSettings.GameType == GAMETYPE_SINGLE || GameSettings.GameType == GAMETYPE_CLOCKWORK || GameSettings.GameType == GAMETYPE_NETWORK_RACE || GameSettings.GameType == GAMETYPE_CHAMPIONSHIP)
                    {
                        GivePickupToPlayer(player, PICKUP_TYPE_GLOBAL);
#ifdef _PC
                        if (IsMultiPlayer())
                        {
                            SendGotGlobal();
                        }
#endif
                    }

                    break;
                }
            }

            break;

// dissappearing

        case 1:

// inc age

            star->Timer += TimeStep;

// spin

            RotMatrixY(&obj->body.Centre.WMatrix, TIME2MS(CurrentTimer()) / -3000.0f);

            mul = star->Timer * 2.0f + 1.0f;

            VecMulScalar(&obj->body.Centre.WMatrix.mv[R], mul);
            VecMulScalar(&obj->body.Centre.WMatrix.mv[U], mul);
            VecMulScalar(&obj->body.Centre.WMatrix.mv[L], mul);

// set env rgb

            FTOL(-star->Timer * 255.0f + 255.0f, col);

            obj->EnvRGB = col >> 1 | col << 8 | col << 16;

// maintain light

            if (obj->Light)
            {
                obj->Light->r = col / 2;
                obj->Light->g = col * 3 / 8;
                obj->Light->b = 0;
            }

// set pos

            VecPlusEqScalarVec(&obj->body.Centre.Pos, TimeStep, &star->Vel);

// done?

            if (star->Timer > 1.0f)
            {
                star->Mode = 2;
                if (obj->Light)
                {
                    FreeLight(obj->Light);
                    obj->Light = NULL;
                }
#ifdef _N64
                if (GameSettings.GameType != GAMETYPE_NETWORK_BATTLETAG)
#endif
                OBJ_FreeObject(obj);

                return;
            }

            break;

// waiting...

        case 2:
            break;
    }
}
#endif

/////////////////
// fox handler //
/////////////////
#ifndef _PSX
void AI_FoxHandler(OBJECT *obj)
{

    long i, rgb;
    FOX_OBJ *fox = (FOX_OBJ*)obj->Data;


#ifdef _PC//A>S>
    MODEL *dmodel, *model = (MODEL*)&fox->Model;
    MODEL_POLY *mp;
    POLY_RGB *mrgb;
    MODEL_VERTEX *mv;
    FOX_VERT *fvert;
#elif defined(_N64)
    MODEL *dmodel , *model = obj->player->car.Models->Body;
    VEC vec1,vec2;
    Vtx *mv;
#endif

    VEC *pos, delta, rotpos, vec, *nvec1, *nvec2;
    PLAYER *player, *nplayer;
    REAL ndist, dist;

    // inc timer

    fox->Timer += TimeStep;

#ifdef _PC//A>S>


// set vert UV's

    mv = model->VertPtr;
    fvert = (FOX_VERT*)(model->VertPtr + model->VertNum);

    for (i = model->VertNum ; i ; i--, mv++, fvert++)
    {
        fvert->Time += fvert->TimeAdd * TimeStep;

        mv->tu = (float)sin(fvert->Time) * (24.0f / 256.0f) + (160.0f / 256.0f);
        mv->tv = (float)cos(fvert->Time) * (24.0f / 256.0f) + (160.0f / 256.0f);
    }

// copy vert UV' to poly UV's + set rgb's

    rgb = 0xffffff;

    mp = model->PolyPtr;
    mrgb = model->PolyRGB;

    for (i = model->PolyNum ; i ; i--, mp++, mrgb++)
    {
        mp->tu0 = mp->v0->tu;
        mp->tv0 = mp->v0->tv;

        mp->tu1 = mp->v1->tu;
        mp->tv1 = mp->v1->tv;

        mp->tu2 = mp->v2->tu;
        mp->tv2 = mp->v2->tv;

        if (mp->Type & POLY_QUAD)
        {
            mp->tu3 = mp->v3->tu;
            mp->tv3 = mp->v3->tv;
        }   

        *(long*)&mrgb->rgb[0] = rgb;
        *(long*)&mrgb->rgb[1] = rgb;
        *(long*)&mrgb->rgb[2] = rgb;
        *(long*)&mrgb->rgb[3] = rgb;
    }
#endif//A>S>

// maintain light

    if (obj->Light)
    {
        CopyVec(&obj->player->car.Body->Centre.Pos, (VEC*)&obj->Light->x);
    }
// dec player timer
    if(!GameSettings.Paws)
        obj->player->BombTagTimer -= (long)(TimeStep * 1000.0f);
    
    if (obj->player->BombTagTimer & 0x80000000)
    {
        obj->player->BombTagTimer = 0;
#ifdef _N64
        if (!obj->player->RaceFinishTime)
#else
        if (obj->player == PLR_LocalPlayer && !PLR_LocalPlayer->RaceFinishTime)
#endif
        {
            SetPlayerFinished(obj->player, TotalRaceTime);

#ifdef _N64 
        {
        PLAYER *_player;
        int playersRemaining = 0;
            for (_player = PLR_PlayerHead ; _player ; _player = _player->next){
                
                if(!_player->RaceFinishTime)
                    playersRemaining++;
            }
            if(playersRemaining < 2){
                for (_player = PLR_PlayerHead ; _player ; _player = _player->next){
                    if(!_player->RaceFinishTime)
                        SetPlayerFinished(_player,MAX_LAP_TIME);
                }
            }else{
                #ifdef _N64
                // Clear glow effect here
                obj->player->car.Models->Body->Flag &= ~MODEL_ENV_FOX;
                #endif

                obj->player->car.IsFox = FALSE;
                OBJ_FreeObject(FoxObj);
                FoxObj = NULL;
                InitStars();
            }
        }
#endif

#ifdef _PC
            SendRaceFinishTime();
#endif
        }
    }

// zapping?

    fox->JumpFlag = FALSE;

    if (fox->Timer < 1.0f && fox->FromPlayer)
    {
        SubVector(&obj->player->car.BodyWorldPos, &fox->FromPlayer->car.BodyWorldPos, &delta);
        if (Length(&delta) < 512)
        {

            TransposeRotVector(&fox->FromPlayer->car.Body->Centre.WMatrix, &delta, &rotpos);

            dmodel = &fox->FromPlayer->car.Models->Body[0];

            ndist = 1000000;
#ifdef _PC
            mv = dmodel->VertPtr;
            for (i = dmodel->VertNum ; i ; i--, mv++)
            {
                SubVector((VEC*)&mv->x, &rotpos, &vec);
                dist = vec.v[X] * vec.v[X] + vec.v[Y] * vec.v[Y] + vec.v[Z] * vec.v[Z];
                if (dist < ndist)
                {
                    ndist = dist;
                    nvec1 = (VEC*)&mv->x;
                }
            }
#elif defined _N64
            mv = dmodel->hdr->vtxptr;
            for (i = dmodel->hdr->vtxnum; i ; i--, mv++)
            {
                vec.v[X] = mv->v.ob[X] - rotpos.v[X];
                vec.v[Y] = mv->v.ob[Y] - rotpos.v[Y];
                vec.v[Z] = mv->v.ob[Z] - rotpos.v[Z];
                dist = vec.v[X] * vec.v[X] + vec.v[Y] * vec.v[Y] + vec.v[Z] * vec.v[Z];
                if (dist < ndist)
                {
                    ndist = dist;
                    nvec1 = (VEC*)&mv->v.ob[0];
                }
            }
            vec1.v[X] = ((Vtx*)nvec1)->v.ob[0];
            vec1.v[Y] = ((Vtx*)nvec1)->v.ob[1];
            vec1.v[Z] = ((Vtx*)nvec1)->v.ob[2];
            nvec1 = &vec1;
#endif

// find my nearest vert to him

            SubVector(&fox->FromPlayer->car.BodyWorldPos, &obj->player->car.BodyWorldPos, &delta);
            TransposeRotVector(&obj->player->car.Body->Centre.WMatrix, &delta, &rotpos);

            ndist = 1000000;

#ifdef _PC
            mv = model->VertPtr;
            for (i = model->VertNum ; i ; i--, mv++)
            {
                SubVector((VEC*)&mv->x, &rotpos, &vec);
                dist = vec.v[X] * vec.v[X] + vec.v[Y] * vec.v[Y] + vec.v[Z] * vec.v[Z];
                if (dist < ndist)
                {
                    ndist = dist;
                    nvec2 = (VEC*)&mv->x;
                }
            }
#elif defined _N64
            mv = model->hdr->vtxptr;
            for (i = model->hdr->vtxnum; i ; i--, mv++)
            {
                vec.v[X] = mv->v.ob[X] - rotpos.v[X];
                vec.v[Y] = mv->v.ob[Y] - rotpos.v[Y];
                vec.v[Z] = mv->v.ob[Z] - rotpos.v[Z];
                dist = vec.v[X] * vec.v[X] + vec.v[Y] * vec.v[Y] + vec.v[Z] * vec.v[Z];
                if (dist < ndist)
                {
                    ndist = dist;
                    nvec2 = (VEC*)&mv->v.ob[0];
                }
            }
            vec2.v[X] = ((Vtx*)nvec2)->v.ob[0];
            vec2.v[Y] = ((Vtx*)nvec2)->v.ob[1];
            vec2.v[Z] = ((Vtx*)nvec2)->v.ob[2];
            nvec2 = &vec2;
#endif

// save jump verts
            RotTransVector(&fox->FromPlayer->car.Body->Centre.WMatrix, &fox->FromPlayer->car.BodyWorldPos, nvec1, &fox->JumpVec1);
            RotTransVector(&obj->player->car.Body->Centre.WMatrix, &obj->player->car.BodyWorldPos, nvec2, &fox->JumpVec2);

            fox->JumpFlag = TRUE;

// create sparks
#ifndef _N64
            FTOL(TimeStep * 50.0f + 1.0f, i);
            for ( ; i ; i--)
            {
                CreateSpark(SPARK_ELECTRIC, &fox->JumpVec1, &ZeroVector, 200, 0);
                CreateSpark(SPARK_ELECTRIC, &fox->JumpVec2, &ZeroVector, 200, 0);
            }
#endif

        }
    }

// don't transfer if local player giving fox

#ifdef _PC
    if (obj->player->type == PLAYER_LOCAL)
        return;
#endif

// check for transfer

    nplayer = NULL;
    ndist = FLT_MAX;

    pos = &obj->player->car.BodyWorldPos;
    for (player = PLR_PlayerHead ; player ; player = player->next)
    {

// Don't affect self
        
        if (player == obj->player) continue;
#ifdef _N64
        if(player->RaceFinishTime) continue;
#endif

// skip if not local player receiving fox

#ifdef _PC
        if (player->type != PLAYER_LOCAL) continue;
#endif

// skip if this player already finished

        if (player->RaceFinishTime) continue;

// check no return timer

        if (player->car.FoxReturnTimer > ZERO) continue;

// Check separation

        SubVector(pos, &player->car.BodyWorldPos, &delta);
        dist = delta.v[X] * delta.v[X] + delta.v[Y] * delta.v[Y] + delta.v[Z] * delta.v[Z];
        if (dist > (FOX_RANGE * FOX_RANGE))
            continue;

// line of sight

        if (!LineOfSight(&obj->player->car.Body->Centre.Pos, &player->car.Body->Centre.Pos))
            continue;

// car in range, remember if nearest

        if (dist < ndist)
        {
            ndist = dist;
            nplayer = player;
        }
    }

// transfer?

    if (nplayer)
    {

// send fox transfer packet to everyone

#ifdef _PC
        SendTransferFox(obj->player->PlayerID, nplayer->PlayerID);
#endif

// transfer fox

        TransferFox(obj, obj->player, nplayer);
    }

}
#endif

//////////////////
// transfer fox //
//////////////////
void TransferFox(OBJECT *obj, PLAYER *player1, PLAYER *player2)
{
#ifndef _PSX
    long objflag[2];

//char buf[128];
//sprintf(buf, "object type %d, slot %d - player 1 slot %d - player 2 slot %d", obj ? obj->Type : -1, obj ? (long)(obj - OBJ_ObjectHead) : -1, player1 ? player1->Slot : -1, player2 ? player2->Slot : -1);
//DumpMessage(NULL,buf);


// if claiming to be first, check for existing fox and give to lowest player slot!

#ifndef _PSX
    if (!player1)
    {
        OBJECT *foxobj;
        for (foxobj = OBJ_ObjectHead ; foxobj ; foxobj = foxobj->next) if (foxobj->Type == OBJECT_TYPE_FOX)
        {
            if (foxobj->player->Slot < player2->Slot)
            {
                return;
            }
            else
            {
                #ifdef _N64
                // Clear glow effect here
                foxobj->player->car.Models->Body->Flag &= ~MODEL_ENV_FOX;
                #endif

                foxobj->player->car.IsFox = FALSE;
                foxobj->player->car.FoxReturnTimer = 0;
                OBJ_FreeObject(foxobj);
                break;
            }
        }
    }
#endif

// kill old fox

    if (obj)
    {
        OBJ_FreeObject(obj);
    }

// player 1 loses it

    if (player1)
    {
        #ifdef _N64
        // Clear glow effect here
        player1->car.Models->Body->Flag &= ~MODEL_ENV_FOX;
        #endif
        player1->car.IsFox = FALSE;
        player1->car.FoxReturnTimer = FOX_NORETURN_TIME;
    }

// player 2 gets it

    #ifdef _N64
    // Clear glow effect here
    player2->car.Models->Body->Flag |= MODEL_ENV_FOX;
    #endif
    player2->car.IsFox = TRUE;
    objflag[0] = (long)player2;
    objflag[1] = (long)player1;
    FoxObj = CreateObject(&player2->car.Body->Centre.Pos, &player2->car.Body->Centre.WMatrix, OBJECT_TYPE_FOX, objflag);
#endif
}

////////////////////////
// tumbleweed handler //
////////////////////////
#ifndef _PSX
void AI_TumbleweedHandler(OBJECT *obj)
{
    AI_BangNoiseHandler(obj);
}
#endif

#ifdef _PSX

/////////////////////////
// star pickup handler //
/////////////////////////

void AI_StarHandler(OBJECT *obj)
{
    long col;
    REAL mul;
    STAR_OBJ *star = (STAR_OBJ*)obj->Data;
    PLAYER *player;
    LEVELINFO *levinfo;
    bool unlock;



// act on mode



    switch (star->Mode)
    {

// alive

        case 0:

// look for car collision
            for (player = PLR_PlayerHead ; player ; player = player->next)
            {

                
                if ((player->PickupNum || player->PickupCycleSpeed) && (GameSettings.GameType == GAMETYPE_SINGLE || GameSettings.GameType == GAMETYPE_CLOCKWORK || GameSettings.GameType == GAMETYPE_NETWORK_RACE || GameSettings.GameType == GAMETYPE_CHAMPIONSHIP))
                    continue;


                if (BBTestXZY(&obj->body.CollSkin.BBox, &player->car.BBox)) 
                {

            
                    star->Mode = 1;
                    star->Timer = 0;

                    SFX_Play3D( SAMPLE_PICKUP_GET, 0x3fff, 2048, 0, &obj->body.Centre.Pos );
                    if( SplitScreenMode == 2 )
                    {
                        SFX_SecondPlayerFlag = TRUE;
                        SFX_Play3D( SAMPLE_PICKUP_GET, 0x3fff, 2048, 0, &obj->body.Centre.Pos );
                        SFX_SecondPlayerFlag = FALSE;
                    }

                    if (GameSettings.GameType == GAMETYPE_NETWORK_BATTLETAG )
                    {

                        if( player == &Players[0] )
                            BattleFlag = 0;
                        else
                            BattleFlag = 1;
                    
                    }
                    else if (GameSettings.GameType == GAMETYPE_PRACTICE)
                    {
                        if (GameSettings.Level < LEVEL_NCUP_LEVELS)
                        {
                            levinfo = GetLevelInfo(GameSettings.Level);
                            unlock = IsCupFoundPractiseStars(levinfo->LevelClass);
                            SetSecretFoundPractiseStars(GameSettings.Level);
                            if (!unlock && IsCupFoundPractiseStars(levinfo->LevelClass))
                                InitialMenuMessage = MENU_MESSAGE_NEWCARS;
                    


                        }
                    }
                        
                    // store star in 'got' list if training
                    else if (GameSettings.GameType == GAMETYPE_TRAINING)
                    {
                        StarList.ID[StarList.NumFound++] = (char)star->ID;
                        
                        StuntStar[0][star->ID].r0 = 128;
                        StuntStar[0][star->ID].g0 = 128;
                        StuntStar[0][star->ID].b0 = 128;
                        StuntStar[1][star->ID].r0 = 128;
                        StuntStar[1][star->ID].g0 = 128;
                        StuntStar[1][star->ID].b0 = 128;


                        if (StarList.NumFound == StarList.NumTotal)
                        {
                            InitialMenuMessage = MENU_MESSAGE_COCKWORK;
                        }
                    }

                    else if (GameSettings.GameType == GAMETYPE_SINGLE || GameSettings.GameType == GAMETYPE_NETWORK_RACE || GameSettings.GameType == GAMETYPE_CHAMPIONSHIP)
                    {
                        GivePickupToPlayer(player, PICKUP_TYPE_GLOBAL);
                    }


                    break;
                }
            }

            break;

// dissappearing

        case 1:

// inc age

            star->Timer += TimeStep;

// done?

            if (star->Timer > TO_TIME(Real(1.0f)) )
            {
            
                star->Mode = 2;
                return;
            }

            break;

// waiting...

        case 2:
            break;
    }
}
#endif

///////////////////////////
// electropulse everyone //
///////////////////////////

void ElectroPulseTheWorld(long slot)
{
    long flag;
    PLAYER *player;

// zap all players except 'slot'

    for (player = PLR_PlayerHead ; player ; player = player->next) if (player->Slot != slot)
    {
        if (player->type == PLAYER_LOCAL || player->type == PLAYER_REMOTE || player->type == PLAYER_CPU)
        {
            flag = (long)player;
            CreateObject(&player->car.Body->Centre.Pos, &player->car.Body->Centre.WMatrix, OBJECT_TYPE_ELECTROZAPPED, &flag);
            player->car.PowerTimer = ELECTRO_KILL_TIME;
        }
    }

    GlobalPickupFlash = TO_TIME(HALF);

// shake + sfx

#ifdef _PC
    CAM_MainCamera->Shake = 0.5f;
#endif
#ifdef _N64
    CAM_PlayerCameras[0]->Shake = 0.5f;
    g_Fade      = 0x63;
    g_FadeColor = 0xff7f7fff;
    g_FadeSub   = 0x04020204;
#endif
#ifndef _PSX
#ifdef OLD_AUDIO
    PlaySfx(SFX_GLOBAL, SFX_MAX_VOL, SFX_CENTRE_PAN, SFX_SAMPLE_RATE, 0x7fffffff);
#else
    g_SoundEngine.Play2DSound( EFFECT_ElectroZap, FALSE );
#endif  // OLD_AUDIO
#endif
}

/////////////////////
// lantern handler //
/////////////////////
#ifndef _PSX

void AI_LanternHandler(OBJECT *obj)
{
    LANTERN_OBJ *lantern = (LANTERN_OBJ*)obj->Data;

// quit if no light

    if (!obj->Light)
        return;

// bias to centre brightness

    lantern->Brightness += (0.5f - lantern->Brightness) * 0.2f * TimeStep;

// flicker

    lantern->Brightness += (frand(2.0f) - 1.0f) * TimeStep;
    if (lantern->Brightness < 0.0f) lantern->Brightness = 0.0f;
    if (lantern->Brightness > 1.0f) lantern->Brightness = 1.0f;

// set reach + rgb from brightness

    obj->Light->Reach = lantern->Brightness * 800.0f + 600.0f;

    obj->Light->r = (long)(lantern->Brightness * 300.0f + 150.0f);
    obj->Light->g = (long)(lantern->Brightness * 246.0f + 123.0f);
    obj->Light->b = (long)(lantern->Brightness * 144.0f + 72.0f);
}
#endif

//////////////////////
// 3d sound handler //
//////////////////////
#ifndef _PSX

void AI_3DSoundHandler(OBJECT *obj)
{
    SOUND3D_OBJ *sound = (SOUND3D_OBJ*)obj->Data;

// waiting to play

    if (!sound->Mode)
    {
        sound->Timer -= TimeStep;
        if (sound->Timer < 0.0f)
        {
            sound->Mode = 1;
#ifdef OLD_AUDIO
            obj->Sfx3D = CreateSfx3D(sound->Sfx, SFX_MAX_VOL, SFX_SAMPLE_RATE, FALSE, &obj->body.Centre.Pos, 0);
            if (obj->Sfx3D)
            {
                obj->Sfx3D->RangeMul = sound->Range;
            }
#else
            g_SoundEngine.Play3DSound( sound->Sfx + g_dwLevelSoundsOffset, FALSE, obj, &obj->pSfxInstance );
#endif // OLD_AUDIO
        }
    }

// playing

    else
    {
#ifdef OLD_AUDIO
        if (!obj->Sfx3D || !obj->Sfx3D->Sample)
        {
            if (obj->Sfx3D)
            {
                FreeSfx3D(obj->Sfx3D);
                obj->Sfx3D = NULL;
            }
            sound->Mode = 0;
            sound->Timer = frand(SOUND_3D_MAX_WAIT) + Real(10);
        }
#else
        if( obj->pSfxInstance )
        {
            g_SoundEngine.ReturnInstance( obj->pSfxInstance );
            obj->pSfxInstance = NULL;
            sound->Mode = 0;
            sound->Timer = frand(SOUND_3D_MAX_WAIT) + Real(10);
        }
#endif // OLD_AUDIO
    }
}
#endif

////////////////////
// slider handler //
////////////////////
#ifdef _PC

void AI_SliderHandler(OBJECT *obj)
{
    long sfx = 0;
    SLIDER_OBJ *slider = (SLIDER_OBJ*)obj->Data;
    VEC vec;
    REAL time;

// get pos

    slider->LastTime = slider->Time;

    time = slider->Time = (float)(TIME2MS(TimerCurrent) % 3000) / 1500.0f;

    if (time >= 1.0f) time = 2.0f - time;
    if (!slider->ID) time = -time;
    time *= 400.0f;

    CopyVec(&obj->body.Centre.Pos, &vec);
    VecPlusScalarVec(&slider->Origin, time, &obj->body.Centre.WMatrix.mv[R], &obj->body.Centre.Pos);

// play sfx?

#ifdef OLD_AUDIO
    if (slider->Time >= 1.0f && slider->LastTime < 1.0f)
        sfx = SFX_MARKET_DOOR_CLOSE;
    else if (slider->Time < 1.0f && slider->LastTime >= 1.0f)
        sfx = SFX_MARKET_DOOR_OPEN;

    if (sfx)
        PlaySfx3D(sfx, SFX_MAX_VOL, SFX_SAMPLE_RATE, &slider->Origin, 0);
#else // !OLD_AUDIO
    // TODO (JHarding): Revisit with level-specific sounds
    if( slider->Time > 1.0f && slider->LastTime < 1.0f ||
        slider->Time < 1.0f && slider->LastTime > 1.0f )
    {
        g_SoundEngine.Play3DSound( EFFECT_Scrape, 
                                   FALSE,
                                   slider->Origin.v[0], 
                                   slider->Origin.v[1], 
                                   slider->Origin.v[2] );
    }
#endif // !OLD_AUDIO

// update collision skin

    SubVector(&obj->body.Centre.Pos, &vec, &vec);
    TransCollPolys(obj->body.CollSkin.WorldCollPoly, obj->body.CollSkin.NCollPolys, &vec);
    VecEqScalarVec(&obj->body.Centre.Vel, 1.0f / TimeStep, &vec);
}
#endif

//////////////////
// rain handler //
//////////////////
#ifdef _PC

void AI_RainHandler(OBJECT *obj)
{
    long i;
    RAIN_OBJ *rain = (RAIN_OBJ*)obj->Data;
    RAINDROP *raindrop;
    VEC pos, pos2;
    REAL lens, time, dist1, dist2;

// sort each drop

    raindrop = rain->Drop;
    for (i = 0 ; i < RAINDROP_NUM ; i++, raindrop++)
    {

// act on mode

        switch (raindrop->Mode)
        {

// asleep

            case RAINDROP_SLEEP:

                raindrop->Timer -= TimeStep;
                if (raindrop->Timer < 0.0f)
                {

// set to fall

                    raindrop->Mode = RAINDROP_FALL;

// get re-gen pos / vel

                    lens = BaseGeomPers + CAM_MainCamera->Lens;
                    pos.v[Z] = frand(RAIN_ZMAX - RAIN_ZMIN) + RAIN_ZMIN;
                    pos.v[X] = (frand(640.0f + RAIN_XTOL * 2) - 320.0f - RAIN_XTOL) * pos.v[Z] / lens;
                    pos.v[Y] = -240.0f * pos.v[Z] / lens;

                    RotTransVector(&CAM_MainCamera->WMatrix, &CAM_MainCamera->WPos, &pos, &raindrop->Pos);

                    VecEqScalarVec(&raindrop->Velocity, 1500.0f, &FLD_GravityVector);

// find intersect pos

                    VecPlusScalarVec(&raindrop->Pos, -5.0f, &raindrop->Velocity, &pos);
                    VecPlusScalarVec(&raindrop->Pos, 5.0f, &raindrop->Velocity, &pos2);

                    LineOfSightDist(&pos, &pos2, &time, &raindrop->Plane);
                    if (time == ONE)
                    {
                        raindrop->HitHeight = CAM_MainCamera->WPos.v[Y] + 1000.0f;
                        raindrop->Plane = NULL;
                    }
                    else
                    {
                        raindrop->HitHeight = pos.v[Y] + (pos2.v[Y] - pos.v[Y]) * time;
                        if (raindrop->HitHeight < CAM_MainCamera->WPos.v[Y])
                        {
                            raindrop->Mode = RAINDROP_SLEEP;
                            raindrop->Timer = 1.0f;
                        }
                    }
                }

                break;

// falling

            case RAINDROP_FALL:

                VecPlusEqScalarVec(&raindrop->Pos, TimeStep, &raindrop->Velocity);
                if (raindrop->Pos.v[Y] > raindrop->HitHeight)
                {
                    if (!raindrop->Plane)
                    {
                        raindrop->Mode = RAINDROP_SLEEP;
                        raindrop->Timer = 0.0f;
                    }
                    else
                    {
                        raindrop->Mode = RAINDROP_SPLASH;
                        raindrop->Timer = 0.25f;

                        VecMinusScalarVec(&raindrop->Pos, TimeStep, &raindrop->Velocity, &pos);
                        dist1 = -PlaneDist(raindrop->Plane, &pos);
                        dist2 = -PlaneDist(raindrop->Plane, &raindrop->Pos);
                        FindIntersection(&pos, dist1, &raindrop->Pos, dist2, &raindrop->Pos);
                    }
                }
                break;

// splashing

            case RAINDROP_SPLASH:

                raindrop->Timer -= TimeStep;
                if (raindrop->Timer <= 0.0f)
                {
                    raindrop->Mode = RAINDROP_SLEEP;
                    raindrop->Timer = 0.0f;
                }

                break;
        }
    }
}

#endif

///////////////////////
// lightning handler //
///////////////////////
#ifndef _PSX

void AI_LightningHandler(OBJECT *obj)
{
    LIGHTNING_OBJ *lightning = (LIGHTNING_OBJ*)obj->Data;
    REAL time;
    long col;

// waiting

    if (!lightning->Mode)
    {
        lightning->Timer -= TimeStep;
        if (lightning->Timer < 0.0f)
        {
            lightning->Mode = 1;
            lightning->Timer = frand(0.3f) + 0.2f;

            obj->Light = AllocLight();
            if (obj->Light)
            {
                SetVector((VEC*)&obj->Light->x, 0.0f, -10000.0f, 0.0f);
                obj->Light->Reach = 50000.0f;
                obj->Light->Flag = LIGHT_FIXED | LIGHT_MOVING;
                obj->Light->Type = LIGHT_OMNINORMAL;
                obj->Light->r = 0;
                obj->Light->g = 0;
                obj->Light->b = 0;
            }
        }
    }

// flash!

    else
    {
        if (obj->Light)
        {
            time = lightning->Timer;
            while (time > 0.15f) time -= 0.15f;
            FTOL((float)sin(time * RAD * (0.5f / 0.15f)) * 1000.0f, col);
            obj->Light->r = obj->Light->g = obj->Light->b = col;

        }

        lightning->Timer -= TimeStep;
        if (lightning->Timer < 0.0f)
        {
            lightning->Mode = 0;
            lightning->Timer = frand(30.0f) + 10.0f;

            if (obj->Light)
            {
                FreeLight(obj->Light);
                obj->Light = NULL;
            }
        }
    }
}
#endif

////////////////////////
// ship light handler //
////////////////////////
#ifdef _PC

void AI_ShipLightHandler(OBJECT *obj)
{
    MAT mat;

// swing

    mat.mv[U] = FLD_GravityVector;
    SetVector(&mat.mv[R], mat.m[UY], -mat.m[UX], 0);
    NormalizeVector(&mat.mv[R]);
    CrossProduct(&mat.mv[R], &mat.mv[U], &mat.mv[L]);
    CrossProduct(&mat.mv[U], &mat.mv[L], &mat.mv[R]);

    CopyMat(&mat, &obj->body.Centre.WMatrix);
}
#endif

//////////////////////
// waterbox handler //
//////////////////////
#ifdef _PC

void AI_WaterBoxHandler(OBJECT *obj)
{
    WATERBOX_OBJ *wb = (WATERBOX_OBJ*)obj->Data;
    PLAYER *player;
    long flag;

// add mesh fx if main camera outside waterbox

    if (CAM_MainCamera->WPos.v[X] < wb->Box.Xmin || CAM_MainCamera->WPos.v[X] > wb->Box.Xmax || 
        CAM_MainCamera->WPos.v[Y] < wb->Box.Ymin || CAM_MainCamera->WPos.v[Y] > wb->Box.Ymax || 
        CAM_MainCamera->WPos.v[Z] < wb->Box.Zmin || CAM_MainCamera->WPos.v[Z] > wb->Box.Zmax)
    {
        AddWorldMeshFx(WaterBoxWorldMeshFxChecker, obj);
        AddModelMeshFx(WaterBoxModelMeshFxChecker, obj);
    }

// check cars for contact with water box

    for (player = PLR_PlayerHead ; player ; player = player->next) if (player->type != PLAYER_NONE)
    {
        if (PointInBBox(&player->car.Body->Centre.Pos, (BBOX*)&wb->Box))
        {
            if (!player->car.InWater)
            {
                CreateObject(&player->car.Body->Centre.Pos, &IdentityMatrix, OBJECT_TYPE_SPLASH, &flag);
#ifdef OLD_AUDIO
                PlaySfx3D(SFX_SPLASH, SFX_MAX_VOL, SFX_SAMPLE_RATE, &player->car.Body->Centre.Pos, 0x7fffffff);
#else // !OLD_AUDIO
                g_SoundEngine.Play3DSound( EFFECT_Splash, 
                                           FALSE,
                                           player->car.Body->Centre.Pos.v[0], 
                                           player->car.Body->Centre.Pos.v[1], 
                                           player->car.Body->Centre.Pos.v[2] );
#endif // !OLD_AUDIO
                player->car.InWater = TRUE;
            }
        }
        else
        {
            player->car.InWater = FALSE;
        }
    }
}
#endif

//////////////////////////////
// waterbox mesh fx checker //
//////////////////////////////
#ifdef _PC

#define WATERBOX_SINMUL 6.0f

static float WaterBoxSinTable[9];

static void WaterBoxWorldMeshFxChecker(void *data)
{
    long i, j, hash;
    CUBE_HEADER **cubelist;
    OBJECT *obj = (OBJECT*)data;
    WATERBOX_OBJ *wb = (WATERBOX_OBJ*)obj->Data;
    WORLD_VERTEX *wv;
    WORLD_POLY *wp;

// create sin offsets

    for (i = 0 ; i < 9 ; i++)
    {
        WaterBoxSinTable[i] = (float)sin(TIME2MS(TimerCurrent) / ((float)i * 10.0f + 200.0f)) * WATERBOX_SINMUL;
    }

// loop thru world cubes

    cubelist = World.CubeList;

    for (i = 0 ; i < WorldCubeCount ; i++)
    {

// check bounding box

        if (cubelist[i]->Xmin > wb->Box.Xmax || cubelist[i]->Xmax < wb->Box.Xmin ||
            cubelist[i]->Ymin > wb->Box.Ymax || cubelist[i]->Ymax < wb->Box.Ymin ||
            cubelist[i]->Zmin > wb->Box.Zmax || cubelist[i]->Zmax < wb->Box.Zmin)
                continue;

// ok, check verts

        if (cubelist[i]->MeshFxFlag & MESHFX_USENEWVERTS)
        {
            wv = cubelist[i]->Model.VertPtr;
            for (j = cubelist[i]->Model.VertNum ; j ; j--, wv++)
            {
                if (wv->x < wb->Box.Xmin || wv->x > wb->Box.Xmax || wv->y < wb->Box.Ymin || wv->y > wb->Box.Ymax || wv->z < wb->Box.Zmin || wv->z > wb->Box.Zmax)
                {
                    continue;
                }
                else
                {
                    hash = abs((long)(wv->x * wv->z));
                    wv->x2 += WaterBoxSinTable[(hash + 0) % 9];
                    wv->y2 += WaterBoxSinTable[(hash + 3) % 9] + WATERBOX_SINMUL;
                    wv->z2 += WaterBoxSinTable[(hash + 6) % 9];
                }
            }

            wp = cubelist[i]->Model.PolyPtr;
            for (j = cubelist[i]->Model.PolyNum ; j ; j--, wp++)
            {
            }
        }
        else
        {
            wv = cubelist[i]->Model.VertPtr;
            for (j = cubelist[i]->Model.VertNum ; j ; j--, wv++)
            {
                if (wv->x < wb->Box.Xmin || wv->x > wb->Box.Xmax || wv->y < wb->Box.Ymin || wv->y > wb->Box.Ymax || wv->z < wb->Box.Zmin || wv->z > wb->Box.Zmax)
                {
                    CopyVec((VEC*)&wv->x, (VEC*)&wv->x2);
                }
                else
                {
                    hash = abs((long)(wv->x * wv->z));
                    wv->x2 = wv->x + WaterBoxSinTable[(hash + 0) % 9];
                    wv->y2 = wv->y + WaterBoxSinTable[(hash + 3) % 9] + WATERBOX_SINMUL;
                    wv->z2 = wv->z + WaterBoxSinTable[(hash + 6) % 9];
                }
            }

            wp = cubelist[i]->Model.PolyPtr;
            for (j = cubelist[i]->Model.PolyNum ; j ; j--, wp++)
            {
            }
        }

// set mesh flag

        cubelist[i]->MeshFxFlag |= MESHFX_USENEWVERTS;
    }
}

//////////////////////////////
// waterbox mesh fx checker //
//////////////////////////////

static void WaterBoxModelMeshFxChecker(void *data)
{
    long j, hash;
    REAL rad = ModelMeshModel->Radius;
    OBJECT *obj = (OBJECT*)data;
    WATERBOX_OBJ *wb = (WATERBOX_OBJ*)obj->Data;
    MODEL_VERTEX *mv;
    BOUNDING_BOX bb;

// quick radius bounding box test

    if (ModelMeshPos->v[X] + rad < wb->Box.Xmin ||
        ModelMeshPos->v[X] - rad > wb->Box.Xmax ||
        ModelMeshPos->v[Y] + rad < wb->Box.Ymin ||
        ModelMeshPos->v[Y] - rad > wb->Box.Ymax ||
        ModelMeshPos->v[Z] + rad < wb->Box.Zmin ||
        ModelMeshPos->v[Z] - rad > wb->Box.Zmax)
            return;

// build model space bounding box

    bb.Xmin = wb->Box.Xmin - ModelMeshPos->v[X];
    bb.Xmax = wb->Box.Xmax - ModelMeshPos->v[X];
    bb.Ymin = wb->Box.Ymin - ModelMeshPos->v[Y];
    bb.Ymax = wb->Box.Ymax - ModelMeshPos->v[Y];
    bb.Zmin = wb->Box.Zmin - ModelMeshPos->v[Z];
    bb.Zmax = wb->Box.Zmax - ModelMeshPos->v[Z];

// ok, check verts

        mv = ModelMeshModel->VertPtr;

        if (*ModelMeshFlag & MODEL_USENEWVERTS)
        {
            for (j = ModelMeshModel->VertNum ; j ; j--, mv++)
            {
                if (mv->x < bb.Xmin || mv->x > bb.Xmax ||   mv->y < bb.Ymin || mv->y > bb.Ymax || mv->z < bb.Zmin || mv->z > bb.Zmax)
                {
                    continue;
                }
                else
                {
                    hash = abs((long)(mv->x * mv->z));
                    mv->x2 += WaterBoxSinTable[(hash + 0) % 9] * 0.2f;
                    mv->y2 += WaterBoxSinTable[(hash + 3) % 9] * 0.2f;
                    mv->z2 += WaterBoxSinTable[(hash + 6) % 9] * 0.2f;
                }
            }
        }
        else
        {
            for (j = ModelMeshModel->VertNum ; j ; j--, mv++)
            {
                if (mv->x < bb.Xmin || mv->x > bb.Xmax ||   mv->y < bb.Ymin || mv->y > bb.Ymax || mv->z < bb.Zmin || mv->z > bb.Zmax)
                {
                    CopyVec((VEC*)&mv->x, (VEC*)&mv->x2);
                }
                else
                {
                    hash = abs((long)(mv->x * mv->z));
                    mv->x2 = mv->x + WaterBoxSinTable[(hash + 0) % 9] * 0.2f;
                    mv->y2 = mv->y + WaterBoxSinTable[(hash + 3) % 9] * 0.2f;
                    mv->z2 = mv->z + WaterBoxSinTable[(hash + 6) % 9] * 0.2f;
                }
            }
        }

// set flag

    *ModelMeshFlag |= MODEL_USENEWVERTS;
}
#endif

////////////////////
// ripple handler //
////////////////////
#ifdef _PC

#define RIPPLE_TABLE_DAMPING 0.95f

void AI_RippleHandler(OBJECT *obj)
{
    RIPPLE_OBJ *ripple = (RIPPLE_OBJ*)obj->Data;
	REAL *wc;
//    REAL *wc, *wlt, *wlb, *wll, *wlr;
//    DDSURFACEDESC2 ddsd2;
//    HRESULT r;
    long x, y;
//    long x, y, bri;
    REAL mag;

//$REMOVED
//// quit if no texture
//
//    if (!TexFormatProcedural.dwFlags)
//        return;
//$END_REMOVAL

// quit if not visible

    if (!obj->renderflag.visible)
        return;

// quit if not time to update

    ripple->Timer -= TimeStep;
    if (ripple->Timer >= 0.0f)
        return;

    ripple->Timer += (1.0f / 30.0f);

// create random ripple

    x = (rand() % (ripple->Width - 2)) + 1;
    y = (rand() % (ripple->Height - 2)) + 1;
    ripple->WaterTableCurrent[y * RIPPLE_TABLE_DIM + x] = -0.2f;

// dolphin ripple?

    if (ripple->Dolphin)
    {
        ripple->DolphinCount++;
        ripple->DolphinCount %= 8;
        if (ripple->DolphinCount < 4)
        {
            ripple->WaterTableCurrent[17 * RIPPLE_TABLE_DIM + 63] = -1.0f;
        }
    }

// toggle current & last table

    wc = ripple->WaterTableCurrent;
    ripple->WaterTableCurrent = ripple->WaterTableLast;
    ripple->WaterTableLast = wc;

#ifndef XBOX_NOT_YET_IMPLEMENTED
// lock source texture

    ddsd2.dwSize = sizeof(ddsd2);

    r = TexInfo[ripple->Tpage].SourceSurface->Lock(NULL, &ddsd2, DDLOCK_WAIT, NULL);
    if (r != DD_OK)
    {
        ErrorDX(r, "Can't lock procedural texture source surface");
    }

//$REMOVED_PALETTIZED
//// update ripple table + source texture - palettized texture
//
//    if (TexFormatProcedural.dwRGBBitCount == 8)
//    {
//        unsigned char *ptr = (unsigned char*)ddsd2.lpSurface + ripple->OffsetY * ddsd2.lPitch + ripple->OffsetX;
//
//        wc = ripple->WaterTableCurrent + RIPPLE_TABLE_DIM;
//        wlt = ripple->WaterTableLast;
//        wlb = wlt + RIPPLE_TABLE_DIM * 2;
//        wll = wlt + RIPPLE_TABLE_DIM - 1;
//        wlr = wll + 2;
//
//        for (y = 1 ; y < ripple->Height - 1 ; y++)
//        {
//            for (x = 1 ; x < ripple->Width - 1 ; x++)
//            {
//                wc[x] = ((wll[x] + wlr[x] + wlt[x] + wlb[x]) / 2.0f - wc[x]) * RIPPLE_TABLE_DAMPING;
//
//                bri = (long)(wc[x] * 128.0f + 128.0f);
//                if (bri < 0) bri = 0;
//                else if (bri > 255) bri = 255;
//
//                ptr[x] = (unsigned char)bri;
//            }
//
//            wc += RIPPLE_TABLE_DIM;
//            wlt += RIPPLE_TABLE_DIM;
//            wlb += RIPPLE_TABLE_DIM;
//            wll += RIPPLE_TABLE_DIM;
//            wlr += RIPPLE_TABLE_DIM;
//
//            ptr += ddsd2.lPitch;
//        }
//    }
//    else
//$END_REMOVAL

// update ripple table + source texture - 24 bit texture

    {
        unsigned long *ptr = (unsigned long*)ddsd2.lpSurface + ripple->OffsetY * (ddsd2.lPitch / sizeof(long)) + ripple->OffsetX;

        wc = ripple->WaterTableCurrent + RIPPLE_TABLE_DIM;
        wlt = ripple->WaterTableLast;
        wlb = wlt + RIPPLE_TABLE_DIM * 2;
        wll = wlt + RIPPLE_TABLE_DIM - 1;
        wlr = wll + 2;

        for (y = 1 ; y < ripple->Height - 1 ; y++)
        {
            for (x = 1 ; x < ripple->Width - 1 ; x++)
            {
                wc[x] = ((wll[x] + wlr[x] + wlt[x] + wlb[x]) / 2.0f - wc[x]) * RIPPLE_TABLE_DAMPING;

                bri = (long)(wc[x] * 128.0f + 128.0f);
                if (bri < 0) bri = 0;
                else if (bri > 255) bri = 255;

                ptr[x] = bri | bri << 8 | bri << 16 | 0xff000000;
            }

            wc += RIPPLE_TABLE_DIM;
            wlt += RIPPLE_TABLE_DIM;
            wlb += RIPPLE_TABLE_DIM;
            wll += RIPPLE_TABLE_DIM;
            wlr += RIPPLE_TABLE_DIM;

            ptr += ddsd2.lPitch / sizeof(long);
        }
    }

// unlock source texture

    TexInfo[ripple->Tpage].SourceSurface->Unlock(NULL);

// blit source tex to dest if master

//  RECT rect;
//  rect.left = 0;
//  rect.right = ripple->Width;
//  rect.top = 0;
//  rect.bottom = ripple->Height;

    if (ripple->Master)
    {
        r = TexInfo[ripple->Tpage].Surface->BltFast(0, 0, TexInfo[ripple->Tpage].SourceSurface, NULL, DDBLTFAST_WAIT | DDBLTFAST_NOCOLORKEY);
//      if (r != DD_OK)
//      {
//          ErrorDX(r, "Can't blit procedural source to dest!");
//      }
    }
#endif // ! XBOX_NOT_YET_IMPLEMENTED

// look for collisions

    PLAYER *player;
    float xdist, zdist;

    for (player = PLR_PlayerHead ; player ; player = player->next) if (player->type != PLAYER_NONE)
    {
        if (!(player->car.NWheelFloorContacts | player->car.Body->NWorldContacts))
            continue;

        if (!PointInBBox(&player->car.Body->Centre.Pos, &ripple->Box))
            continue;

        xdist = PlaneDist(&ripple->PlaneX, &player->car.Body->Centre.Pos) * ripple->Scale + 0.5f;
        if (xdist < 1.0f || xdist > ripple->Width - 2.0f)
            continue;

        zdist = PlaneDist(&ripple->PlaneZ, &player->car.Body->Centre.Pos) * ripple->Scale + 0.5f;
        if (zdist < 1.0f || zdist > ripple->Height - 2.0f)
            continue;

        mag = player->CarAI.speedCur * -0.0008f;
        if (mag < -0.5f) mag = -0.5f;
        ripple->WaterTableCurrent[(long)zdist * RIPPLE_TABLE_DIM + (long)xdist] = mag;
    }
}
#endif

/////////////////////
// dolphin handler //
/////////////////////
#ifdef _PC

void AI_DolphinMoveHandler(OBJECT *obj)
{
    DOLPHIN_OBJ *dolphin = (DOLPHIN_OBJ*)obj->Data;
    REAL mul;
    MAT mat;
    VEC vec;

// only if visible

    if (!obj->renderflag.visible)
        return;

// dec time

    dolphin->Time -= TimeStep;
    if (dolphin->Time <= 0.0f)
    {

// create new particle

        dolphin->Time += 0.01f;

        RotMatrixY(&mat, frand(0.06f) - 0.035f);
        RotVector(&mat, &DolphinVel, &vec);
        mul = (frand(0.2f) + 1.2f);
        VecMulScalar(&vec, mul);
        CreateSpark(rand() & 1 ? SPARK_DOLPHIN : SPARK_DOLPHIN_BIG, &obj->body.Centre.Pos, &vec, 0.0f, 0);
    }
}
#endif

/////////////////////////////////////////////////
// AI_FlagHandler
/////////////////////////////////////////////////

#ifdef _PC

void AI_FlagAddSpringForce(FLAG_PARTICLE* pO, FLAG_PARTICLE* pN, REAL l)
{
#if 0
    VEC     delta;
    REAL    d, s, damping;
//  VEC     dV;
//  REAL    k, v, x;

    VecMinusVec(&pN->pos, &pO->pos, &delta);
    d = VecLen(&delta);
#if 1
    damping = Real(0.99);

//  s = l * (Real(0.5) / d);
//  delta.v[X] = damping * (delta.v[X] - (s * delta.v[X]));
//  delta.v[Y] = damping * (delta.v[Y] - (s * delta.v[Y]));
//  delta.v[Z] = damping * (delta.v[Z] - (s * delta.v[Z]));

//  s = (d - l) / (l * Real(0.05));
    s = (d - l) / (l * Real(0.05));
    delta.v[X] *= s;
    delta.v[Y] *= s;
    delta.v[Z] *= s;

    VecPlusEqVec(&pO->imp, &delta);
    VecMinusEqVec(&pN->imp, &delta);
#else
    VecMinusVec(&pN->vel, &pO->vel, &dV);
    VecDivScalar(&delta, d);
    v = VecDotVec(&dV, &delta);
    x = l - d;
    k = Real(200);
    damping = sqrt(1/k);// * 2;

    delta.v[X] = (-k * delta.v[X] * x) + ((damping * v) * delta.v[X]);
    delta.v[Y] = (-k * delta.v[Y] * x) + ((damping * v) * delta.v[Y]);
    delta.v[Z] = (-k * delta.v[Z] * x) + ((damping * v) * delta.v[Z]);

//  NegateVec(&delta);

    VecPlusEqVec(&pO->imp, &delta);
    VecMinusEqVec(&pN->imp, &delta);
#endif
#endif
}

void AI_FlagHandler(OBJECT *obj)
{
    FLAG_DATA_OBJ*  pFlag = (FLAG_DATA_OBJ*)obj->Data;
    FLAG_PARTICLE*  pP;
    int     cW,cH;

    VERTEX_TEX1* pV;
    REAL    xRipple, dxRipple, sxRipple;
    REAL    yRipple, dyRipple;
    REAL    zRipple, dzRipple;
    REAL    amp, dxAmp;
    REAL    d;
    REAL    distX, distY, dlast;
    int     rgb;

    VEC     posY, posXY, posDX, posDY;

    REAL    oldTimeStep = TimeStep;


    TimeStep = NPhysicsLoops * PHYSICSTIMESTEP/1000.0f;

// Make sure flag is visible
    if (!obj->renderflag.visible)
        return;

// Setup default positions
    pP = pFlag->pParticle;
    CopyVec(&obj->body.Centre.Pos, &posY);
    VecEqScalarVec(&posDX, pFlag->length, &obj->body.Centre.WMatrix.mv[R]);
    VecEqScalarVec(&posDY, pFlag->length, &obj->body.Centre.WMatrix.mv[U]);
    for (cH = 0; cH < pFlag->h; cH++)
    {
        CopyVec(&posY, &posXY);

        for (cW = 0; cW < pFlag->w; cW++)
        {
            CopyVec(&posXY, &pP->pos);
            SetVec(&pP->vel, 0,0,0);
            SetVec(&pP->imp, 0,0,0);
            pP++;
            VecPlusEqVec(&posXY, &posDX);
        }

        VecPlusEqVec(&posY, &posDY);
    }

// Ripple flag
    pP = pFlag->pParticle;

    dxRipple = (DEG2RAD * 900) / pFlag->w;
    sxRipple = pFlag->cxRipple;
    dyRipple = (DEG2RAD * 900) / pFlag->h;
    yRipple = pFlag->cyRipple;
    dzRipple = (DEG2RAD * 300 * 1) / (pFlag->w + 1);
    zRipple = pFlag->czRipple;
    amp = pFlag->amplitude;
    dxAmp = (DEG2RAD * 25);

    for (cH = 0; cH < pFlag->h; cH++)
    {
        distY = Real(sin(yRipple));

        pP++;
        dlast = 0;
        xRipple = sxRipple;
        for (cW = 1; cW < pFlag->w; cW++, pP++)
        {
            distX = Real(sin(xRipple)) + distY;

            d = Real((abs(sin(amp)) * 10)) + 5;
            d *= distX;
            pP->vel.v[X] = d;

            VecPlusEqScalarVec(&pP->pos, d, &obj->body.Centre.WMatrix.mv[L]);
            VecPlusEqScalarVec(&pP->pos, Real(sin(xRipple) * 5), &obj->body.Centre.WMatrix.mv[U]);
            VecPlusEqScalarVec(&pP->pos, Real(sin(xRipple) * 5), &obj->body.Centre.WMatrix.mv[R]);

            xRipple += dxRipple;
            zRipple += dzRipple;

            amp += dxAmp;
            dlast = distX;
        }

        sxRipple += (DEG2RAD * 15);
        yRipple += dyRipple;
    }

    pFlag->cxRipple += TimeStep * (DEG2RAD * 250);
    pFlag->cyRipple += TimeStep * (DEG2RAD * 100);
    pFlag->czRipple += TimeStep * (DEG2RAD * 75);
    pFlag->amplitude += TimeStep * (DEG2RAD * 195);

// Light flag vertices
    pP = pFlag->pParticle;
    pV = pFlag->pVert;
    pP += pFlag->w;
    pV += pFlag->w;
    for (cH = 1; cH < pFlag->h-1; cH++)
    {
        pP++;
        pV++;
        for (cW = 1; cW < pFlag->w-1; cW++)
        {
            d = pP[-1].vel.v[X] - pP[1].vel.v[X];
            d += pP[-pFlag->w].vel.v[X] - pP[pFlag->w].vel.v[X];

            rgb = (int)(d * (128/40));
            rgb += 128 + 32;
            if (rgb < 0)
                rgb = 0;
            else if (rgb > 255)
                rgb = 255;
            pV->color = (rgb << 16) | (rgb << 8) | rgb;

            pP++;
            pV++;
        }

        pP++;
        pV++;
    }

    TimeStep = oldTimeStep;
}

#endif

#ifdef _PC

#define GHOST_CLOTH_DAMPING_UPPER   Real(-120)
#define GHOST_CLOTH_DAMPING_LOWER   Real(-80)

/////////////////////////////////////////////////
// GhostCarInitCloth()
/////////////////////////////////////////////////
void GhostCarInitCloth(CAR* pCar)
{
    MODEL_POLY*     pPoly;
    MODEL_VERTEX*   pVert;
    int             cP;
    int             vertCnt;
    CAR_CLOTH_POINT*    pCP;

    if (pCar->CarType != CARID_MYSTERY)
        return;

// Clear vertex flags
    pVert = pCar->Models->Body->VertPtr;
    for (cP = 0; cP < pCar->Models->Body->VertNum; cP++)
    {
        pVert->Clip = 0;
        pVert++;
    }

// Select fx vertices
    pCar->Models->cloth.cVerts = 0;

    pPoly = pCar->Models->Body->PolyPtr;
    pCP = pCar->Models->cloth.pVerts;
    for (cP = 0; cP < pCar->Models->Body->PolyNum; cP++)
    {
        if (pPoly->Type & POLY_FX1)
        {
            vertCnt = 0;

            if (pPoly->v0->Clip == 0)
            {
                pPoly->v0->Clip = 1;
                pCP->pV = pPoly->v0;
                if (pPoly->v0->y > Real(-10))
                    pCP->damping = GHOST_CLOTH_DAMPING_LOWER;
                else
                    pCP->damping = GHOST_CLOTH_DAMPING_UPPER;
                pCP++;
                pCar->Models->cloth.cVerts++;
            }
            if (pPoly->v1->Clip == 0)
            {
                pPoly->v1->Clip = 1;
                pCP->pV = pPoly->v1;
                if (pPoly->v1->y > Real(-10))
                    pCP->damping = GHOST_CLOTH_DAMPING_LOWER;
                else
                    pCP->damping = GHOST_CLOTH_DAMPING_UPPER;
                pCP++;
                pCar->Models->cloth.cVerts++;
            }
            if (pPoly->v2->Clip == 0)
            {
                pPoly->v2->Clip = 1;
                pCP->pV = pPoly->v2;
                if (pPoly->v2->y > Real(-10))
                    pCP->damping = GHOST_CLOTH_DAMPING_LOWER;
                else
                    pCP->damping = GHOST_CLOTH_DAMPING_UPPER;
                pCP++;
                pCar->Models->cloth.cVerts++;
            }
            if ((pPoly->Type & POLY_QUAD) && (pPoly->v3->Clip == 0))
            {
                pPoly->v3->Clip = 1;
                pCP->pV = pPoly->v3;
                if (pPoly->v3->y > Real(-10))
                    pCP->damping = GHOST_CLOTH_DAMPING_LOWER;
                else
                    pCP->damping = GHOST_CLOTH_DAMPING_UPPER;
                pCP++;
                pCar->Models->cloth.cVerts++;
            }
        }

        pPoly++;
    }

    Assert(pCar->Models->cloth.cVerts <= CAR_CLOTH_VERTS_MAX);


// Create springs
    pCP = pCar->Models->cloth.pVerts;
    for (cP = 0; cP < pCar->Models->cloth.cVerts; cP++)
    {
        SetVec(&pCP->vel, 0,0,0);
        SetVec(&pCP->imp, 0,0,0);
        pCP->pos.v[X] = pCP->pV->x;
        pCP->pos.v[Y] = pCP->pV->y;
        pCP->pos.v[Z] = pCP->pV->z;

        pCP++;
    }
}

/////////////////////////////////////////////////
// GhostCarProcessCloth()
/////////////////////////////////////////////////
void GhostCarProcessCloth(CAR* pCar, REAL speed)
{
    CAR_CLOTH_POINT*    pCP;
    VEC     delta;
    int     cP;
    REAL    oldTimeStep = TimeStep;
    VEC     carImp;
    static  REAL k = Real(200);
    static  REAL damping = (REAL)sqrt(1/Real(200));// * 2;
//  static  REAL damping = (REAL)sqrt(1/k);// * 2;
//  static  REAL impDamp = -80;
    static  REAL velDamp = Real(1);

    if (pCar->CarType != CARID_MYSTERY)
        return;

    TimeStep = NPhysicsLoops * PHYSICSTIMESTEP/1000.0f;

// Calc
    MatMulVec(&pCar->Body->Centre.WMatrix, &pCar->Body->Centre.Vel, &carImp);
    carImp.v[X] = -carImp.v[X];
    carImp.v[Y] = -carImp.v[Y];
    carImp.v[Z] = -carImp.v[Z];
    carImp.v[Y] += (200*1) * TimeStep;          // Gravity
    speed = speed * Real(0.1);

    pCP = pCar->Models->cloth.pVerts;
    for (cP = 0; cP < pCar->Models->cloth.cVerts; cP++)
    {
        static REAL dampUpper = Real(-160);
        static REAL dampLower = Real(-120);

        VecPlusEqVec(&pCP->imp, &carImp);
        pCP->imp.v[X] += frand(speed+speed) - speed;
        pCP->imp.v[Y] += frand(speed+speed) - speed;
        pCP->imp.v[Z] += frand(speed+speed) - speed;

        delta.v[X] = pCP->pV->x - pCP->pos.v[X];
        delta.v[Y] = pCP->pV->y - pCP->pos.v[Y]; //$ADDITION(cprince): avoid "var used before init" error
                                                 //$REVISIT(cprince): was that the correct value to init var with?  (Maybe they never used this func?  Or value not critical??)
        delta.v[Z] = pCP->pV->z - pCP->pos.v[Z];

            if (pCP->pV->y > Real(-10)) pCP->damping = dampLower;
            else                        pCP->damping = dampUpper;

        delta.v[X] *= pCP->damping;
        delta.v[Y] *= pCP->damping; // = 0;
        delta.v[Z] *= pCP->damping;

        VecPlusEqVec(&pCP->imp, &delta);
        pCP->imp.v[Y] = 0;

        pCP++;
    }


// Update positions
    pCP = pCar->Models->cloth.pVerts;
    for (cP = 0; cP < pCar->Models->cloth.cVerts; cP++)
    {
        VecMulScalar(&pCP->imp, TimeStep);
        VecPlusEqScalarVec(&pCP->vel, ONE/Real(1), &pCP->imp);  // Apply impulse
//      VecMulScalar(&pCP->vel, ONE - (Real(0.01) * TimeStep)); // Damping (air resitance etc..)
        VecMulScalar(&pCP->vel, ONE - (velDamp * TimeStep));    // Damping (air resitance etc..)

        pCP->pV->x += TimeStep * pCP->vel.v[X];
        pCP->pV->y += TimeStep * pCP->vel.v[Y];
        pCP->pV->z += TimeStep * pCP->vel.v[Z];

        SetVecZero(&pCP->imp);

        pCP++;
    }

    TimeStep = oldTimeStep;
}

#endif

/////////////////////
// fog box handler //
/////////////////////
#ifndef _PSX

void AI_FogBoxHandler(OBJECT *obj)
{
    long i;
    FOGBOX_OBJ *fb = (FOGBOX_OBJ*)obj->Data;

// check this box against each active camera

    for (i = 0 ; i < MAX_CAMERAS ; i++) if (Camera[i].Flag != CAMERA_FLAG_FREE)
    {
        if (PointInBBox(&Camera[i].WPos, (BBOX*)&fb->Box))
        {

// camera inside this box, set special fog

            Camera[i].SpecialFog = TRUE;
            Camera[i].SpecialFogColor = 0x000000;   // always black for now
        }
        else
        {

// camera outside this box, clear special fog

            Camera[i].SpecialFog = FALSE;
        }
    }
}
#endif