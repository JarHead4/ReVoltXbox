#pragma message( "JHarding probably needs to add pServoSound (CAR struct in car.h)" )
//-----------------------------------------------------------------------------
// File: car.cpp
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"

#include "Main.h"
#ifdef _PC
#include "input.h"
#endif
#include "Geom.h"
#include "Util.h"
#include "NewColl.h"
#include "Model.h"
#include "Particle.h"
#include "Body.h"
#include "Aerial.h"
#include "Wheel.h"
#include "Car.h"
#include "ctrlread.h"
#include "object.h"
#include "Control.h"
#include "move.h"
#ifndef _PSX
#include "Spark.h"
#include "DrawObj.h"
#endif
#include "player.h"
#include "timing.h"
#ifdef _PC
#include "settings.h"
#include "LevelInfo.h"
#endif
#include "weapon.h"
#include "competition.h"
#include "initplay.h"
#include "Field.h"
#include "TitleScreen.h"
#include "Cheats.h"

#if MSCOMPILER_FUDGE_OPTIMISATIONS
#pragma optimize("", off)
#endif


//-----------------------------------------------------------------------------
// globals
//-----------------------------------------------------------------------------

extern bool GHO_ShowGhost;

#if USE_DEBUG_ROUTINES
extern long DEBUG_CollGrid;
#endif

CAR_INFO    *CarInfo = NULL;
REAL        MaxCannotMoveTime = 3;


long    NCarTypes = 0;

bool    CAR_WheelsHaveSuspension = TRUE;
bool    CAR_DrawCarBBoxes = FALSE;
bool    CAR_DrawCarAxes = FALSE;

VEC     SmokeVel = {ZERO, Real(-72.0f), ZERO};
VEC     TurboVel = {ZERO, Real(-64.0f), ZERO};

//
// Global function prototypes
//

void SetAllCarSelect();
void InitAllCars(void);
void SetupCar(struct PlayerStruct *player, int carType);
void SetCarPos(CAR *car, VEC *pos, MAT *mat);
void BuildTurnMatrices(CAR *car);
void Car2Car(CAR *MyCar);
void CarWorldColls(CAR *car);
void CarCarColls(CAR *car1, CAR *car2);
int DetectWheelWheelColls(CAR *car, CAR *car2);
int DetectWheelBodyColls(CAR *car, NEWBODY *body);
int DetectWheelSphereColls(CAR *car, int iWheel, NEWBODY *body);
int DetectWheelConvexColls(CAR *car, int iWheel, NEWBODY *body);
int DetectWheelPolyColls(CAR *car, int iWheel, NEWBODY *body);
void InitRemoteCarData(CAR *car);
int NextValidCarType(int iCurrType);
int PrevValidCarType(int iCurrType);
void RemoveWheelColl(CAR *car, COLLINFO_WHEEL *collInfo);
COLLINFO_WHEEL *AddWheelColl(CAR *car, COLLINFO_WHEEL *newHead);
int DetectCarBodyCarBodyColls(CAR *car1, CAR *car2);
int DetectHullHullCollsArcade(NEWBODY *body1, NEWBODY *body2);
void SetPlayerToSnail(CAR *car);


//--------------------------------------------------------------------------------------------------------------------------

// Car grid start positions

GRID_POS CarGridStarts[][MAX_RACE_CARS] = {

// type 0 - 2 wide

    {
        {TO_LENGTH(Real(0)),    TO_LENGTH(Real(0)),     TO_LENGTH(Real(0)),     TO_LENGTH(Real(0))},
        {TO_LENGTH(Real(256)),  TO_LENGTH(Real(0)),     TO_LENGTH(Real(-40)),   TO_LENGTH(Real(0))},
        {TO_LENGTH(Real(0)),    TO_LENGTH(Real(0)),     TO_LENGTH(Real(-300)),  TO_LENGTH(Real(0))},
        {TO_LENGTH(Real(256)),  TO_LENGTH(Real(0)),     TO_LENGTH(Real(-340)),  TO_LENGTH(Real(0))},
#ifdef _PC
        {TO_LENGTH(Real(0)),    TO_LENGTH(Real(0)),     TO_LENGTH(Real(-600)),  TO_LENGTH(Real(0))},
        {TO_LENGTH(Real(256)),  TO_LENGTH(Real(0)),     TO_LENGTH(Real(-640)),  TO_LENGTH(Real(0))},
        {TO_LENGTH(Real(0)),    TO_LENGTH(Real(0)),     TO_LENGTH(Real(-900)),  TO_LENGTH(Real(0))},
        {TO_LENGTH(Real(256)),  TO_LENGTH(Real(0)),     TO_LENGTH(Real(-940)),  TO_LENGTH(Real(0))},
        {TO_LENGTH(Real(0)),    TO_LENGTH(Real(0)),     TO_LENGTH(Real(-1200)), TO_LENGTH(Real(0))},
        {TO_LENGTH(Real(256)),  TO_LENGTH(Real(0)),     TO_LENGTH(Real(-1240)), TO_LENGTH(Real(0))},
        {TO_LENGTH(Real(0)),    TO_LENGTH(Real(0)),     TO_LENGTH(Real(-1500)), TO_LENGTH(Real(0))},
        {TO_LENGTH(Real(256)),  TO_LENGTH(Real(0)),     TO_LENGTH(Real(-1540)), TO_LENGTH(Real(0))},
#endif
    },

// type 1 - 2 wide - mirrored

    {
        {TO_LENGTH(Real(0)),    TO_LENGTH(Real(0)),     TO_LENGTH(Real(0)),     TO_LENGTH(Real(0))},
        {TO_LENGTH(Real(-256)), TO_LENGTH(Real(0)),     TO_LENGTH(Real(-40)),   TO_LENGTH(Real(0))},
        {TO_LENGTH(Real(0)),    TO_LENGTH(Real(0)),     TO_LENGTH(Real(-300)),  TO_LENGTH(Real(0))},
        {TO_LENGTH(Real(-256)), TO_LENGTH(Real(0)),     TO_LENGTH(Real(-340)),  TO_LENGTH(Real(0))},
#ifdef _PC
        {TO_LENGTH(Real(0)),    TO_LENGTH(Real(0)),     TO_LENGTH(Real(-600)),  TO_LENGTH(Real(0))},
        {TO_LENGTH(Real(-256)), TO_LENGTH(Real(0)),     TO_LENGTH(Real(-640)),  TO_LENGTH(Real(0))},
        {TO_LENGTH(Real(0)),    TO_LENGTH(Real(0)),     TO_LENGTH(Real(-900)),  TO_LENGTH(Real(0))},
        {TO_LENGTH(Real(-256)), TO_LENGTH(Real(0)),     TO_LENGTH(Real(-950)),  TO_LENGTH(Real(0))},
        {TO_LENGTH(Real(0)),    TO_LENGTH(Real(0)),     TO_LENGTH(Real(-1200)), TO_LENGTH(Real(0))},
        {TO_LENGTH(Real(-256)), TO_LENGTH(Real(0)),     TO_LENGTH(Real(-1240)), TO_LENGTH(Real(0))},
        {TO_LENGTH(Real(0)),    TO_LENGTH(Real(0)),     TO_LENGTH(Real(-1500)), TO_LENGTH(Real(0))},
        {TO_LENGTH(Real(-256)), TO_LENGTH(Real(0)),     TO_LENGTH(Real(-1540)), TO_LENGTH(Real(0))},
#endif
    },

// type 2 - used for FRONT END only

    {
        {TO_LENGTH(Real(-1600)), TO_LENGTH(Real(-250)), TO_LENGTH(Real(-1100)), TO_LENGTH(Real(0))},
        {TO_LENGTH(Real(-1700)), TO_LENGTH(Real(-250)), TO_LENGTH(Real(-1200)), TO_LENGTH(Real(0))},
        {TO_LENGTH(Real(-1500)), TO_LENGTH(Real(-250)), TO_LENGTH(Real(-1000)), TO_LENGTH(Real(0))},
        {TO_LENGTH(Real(1600)), TO_LENGTH(Real(-200)), TO_LENGTH(Real(1200)), TO_LENGTH(Real(-0.25))},
#ifdef _PC
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
#endif
    },

// type 3 - 3 wide (for 12 car races)

    {
        {TO_LENGTH(Real(0)),    TO_LENGTH(Real(0)),     TO_LENGTH(Real(0+300)),     TO_LENGTH(Real(0))},
        {TO_LENGTH(Real(256)),  TO_LENGTH(Real(0)),     TO_LENGTH(Real(-30+300)),   TO_LENGTH(Real(0))},
        {TO_LENGTH(Real(-256)), TO_LENGTH(Real(0)),     TO_LENGTH(Real(-60+300)),   TO_LENGTH(Real(0))},
        {TO_LENGTH(Real(0-44)), TO_LENGTH(Real(0)),     TO_LENGTH(Real(-200+300)),  TO_LENGTH(Real(0))},
#ifdef _PC
        {TO_LENGTH(Real(256-44)),   TO_LENGTH(Real(0)),     TO_LENGTH(Real(-230+300)),  TO_LENGTH(Real(0))},
        {TO_LENGTH(Real(-256-44)),  TO_LENGTH(Real(0)),     TO_LENGTH(Real(-260+300)),  TO_LENGTH(Real(0))},
        {TO_LENGTH(Real(0+44)), TO_LENGTH(Real(0)),     TO_LENGTH(Real(-400+300)),  TO_LENGTH(Real(0))},
        {TO_LENGTH(Real(256+44)),   TO_LENGTH(Real(0)),     TO_LENGTH(Real(-430+300)),  TO_LENGTH(Real(0))},
        {TO_LENGTH(Real(-256+44)),  TO_LENGTH(Real(0)),     TO_LENGTH(Real(-460+300)),  TO_LENGTH(Real(0))},
        {TO_LENGTH(Real(0)),    TO_LENGTH(Real(0)),     TO_LENGTH(Real(-600+300)),  TO_LENGTH(Real(0))},
        {TO_LENGTH(Real(256)),  TO_LENGTH(Real(0)),     TO_LENGTH(Real(-630+300)),  TO_LENGTH(Real(0))},
        {TO_LENGTH(Real(-256)), TO_LENGTH(Real(0)),     TO_LENGTH(Real(-660+300)),  TO_LENGTH(Real(0))},
#endif
    },

};

// Directory list for use when CarInfo is split into one file per car
#ifdef _PC
char *CarDirs[CARID_NTYPES] = {
//$MODIFIED:
//    "cars\\rc",         //CARID_RC,
//    "cars\\mite",       //CARID_DUSTMITE,
//    "cars\\phat",       //CARID_PHATSLUG,
//    "cars\\moss",       //CARID_COLMOSS,
//    "cars\\mud",        //CARID_HARVESTER,
//    "cars\\beatall",    //CARID_DOCGRUDGE,
//    "cars\\volken",     //CARID_VOLKEN,
//    "cars\\tc6",        //CARID_SPRINTER,
//    "cars\\dino",       //CARID_DYNAMO,
//    "cars\\candy",      //CARID_CANDY,
//    "cars\\gencar",     //CARID_GENGHIS,
//    "cars\\tc4",        //CARID_FISH,
//    "cars\\mouse",      //CARID_MOUSE,
//    "cars\\flag",       //CARID_FLAG,
//    "cars\\tc2",        //CARID_PANGATC,
//    "cars\\r5",         //CARID_R5,
//    "cars\\tc5",        //CARID_LOADED,
//    "cars\\sgt",        //CARID_BERTHA,
//    "cars\\tc3",        //CARID_INSECTO,
//    "cars\\adeon",      //CARID_ADEON,
//    "cars\\fone",       //CARID_FONE,
//    "cars\\tc1",        //CARID_ZIPPER,
//    "cars\\rotor",      //CARID_ROTOR,
//    "cars\\cougar",     //CARID_COUGAR,
//    "cars\\sugo",       //CARID_SUGO,
//    "cars\\toyeca",     //CARID_TOYECA,
//    "cars\\amw",        //CARID_AMW,
//    "cars\\panga",      //CARID_PANGA,
//    "cars\\trolley",    //CARID_TROLLEY,
//    "cars\\wincar",     //CARID_KEY1,
//    "cars\\wincar2",    //CARID_KEY2,
//    "cars\\wincar3",    //CARID_KEY3,
//    "cars\\wincar4",    //CARID_KEY4,
//    "cars\\ufo",        //CARID_UFO,
//    "cars\\q",          //CARID_MYSTERY,
    "D:\\cars\\rc",         //CARID_RC,
    "D:\\cars\\mite",       //CARID_DUSTMITE,
    "D:\\cars\\phat",       //CARID_PHATSLUG,
    "D:\\cars\\moss",       //CARID_COLMOSS,
    "D:\\cars\\mud",        //CARID_HARVESTER,
    "D:\\cars\\beatall",    //CARID_DOCGRUDGE,
    "D:\\cars\\volken",     //CARID_VOLKEN,
    "D:\\cars\\tc6",        //CARID_SPRINTER,
    "D:\\cars\\dino",       //CARID_DYNAMO,
    "D:\\cars\\candy",      //CARID_CANDY,
    "D:\\cars\\gencar",     //CARID_GENGHIS,
    "D:\\cars\\tc4",        //CARID_FISH,
    "D:\\cars\\mouse",      //CARID_MOUSE,
    "D:\\cars\\flag",       //CARID_FLAG,
    "D:\\cars\\tc2",        //CARID_PANGATC,
    "D:\\cars\\r5",         //CARID_R5,
    "D:\\cars\\tc5",        //CARID_LOADED,
    "D:\\cars\\sgt",        //CARID_BERTHA,
    "D:\\cars\\tc3",        //CARID_INSECTO,
    "D:\\cars\\adeon",      //CARID_ADEON,
    "D:\\cars\\fone",       //CARID_FONE,
    "D:\\cars\\tc1",        //CARID_ZIPPER,
    "D:\\cars\\rotor",      //CARID_ROTOR,
    "D:\\cars\\cougar",     //CARID_COUGAR,
    "D:\\cars\\sugo",       //CARID_SUGO,
    "D:\\cars\\toyeca",     //CARID_TOYECA,
    "D:\\cars\\amw",        //CARID_AMW,
    "D:\\cars\\panga",      //CARID_PANGA,
    "D:\\cars\\trolley",    //CARID_TROLLEY,
    "D:\\cars\\wincar",     //CARID_KEY1,
    "D:\\cars\\wincar2",    //CARID_KEY2,
    "D:\\cars\\wincar3",    //CARID_KEY3,
    "D:\\cars\\wincar4",    //CARID_KEY4,
    "D:\\cars\\ufo",        //CARID_UFO,
    "D:\\cars\\q",          //CARID_MYSTERY,
//$END_MODIFICATIONS
};
#endif

//--------------------------------------------------------------------------------------------------------------------------


////////////////////////////////////////////////////////////////
// SetAllCarSelect:
////////////////////////////////////////////////////////////////
void SetAllCarSelect()
{
    long i;
    CAR_INFO *ci;
    bool obtain;

// quit if no carinfo

    if ((CarInfo == NULL) || (NCarTypes < 1))
    {
        return;
    }

    // allow all if 'AllCars' true
    if (AllCars)
    {
        for (i = 0 ; i < NCarTypes ; i++)
        {
            ci = &CarInfo[i];
#ifdef _PC
            ci->Selectable = TRUE;
#else
            if (ci->ObtainMethod != CAR_OBTAIN_NEVER) {
                ci->Selectable = TRUE;
            }
#endif
        }
        return;
    }

    // allow all if dev on PC
#ifdef _PC
    if (Version == VERSION_DEV)
    {
        for (i = 0 ; i < NCarTypes ; i++)
        {
            ci = &CarInfo[i];
            ci->Selectable = TRUE;
        }
        return;
    }
#endif

    // allow with debug on on _PSX
#if FALSE //defined(_PSX) && defined(DEBUG)
    for (i = 0 ; i < NCarTypes ; i++)
    {
        ci = &CarInfo[i];
        ci->Selectable = TRUE;
    }
    return;
#endif


    // allow 2 cars in Console preview versions
#if !defined(_PC) && defined(DEMO_RELEASE)
    for (i = 0 ; i < NCarTypes ; i++)
    {
        ci = &CarInfo[i];
        ci->Selectable = FALSE;
        if (!ci->Rating && (i == CARID_RC || i == CARID_SPRINTER))
        {
            if (ci->ObtainMethod != CAR_OBTAIN_NEVER)
                ci->Selectable = TRUE;
        }
    }
    return;
#endif


    // allow cars based on secrets completed
    for (i = 0 ; i < NCarTypes ; i++)
    {
        ci = &CarInfo[i];

        obtain = FALSE;

        switch (ci->ObtainMethod)
        {
            case CAR_OBTAIN_DEFAULT:
                obtain = TRUE;
                break;

            case CAR_OBTAIN_CHAMPIONSHIP:
                obtain = IsCupCompleted(ci->Rating);
                break;

            case CAR_OBTAIN_TIMETRIAL:
                obtain = IsCupBeatTimeTrials(ci->Rating);
                break;

            case CAR_OBTAIN_PRACTICE:
                obtain = IsCupFoundPractiseStars(ci->Rating);
                break;

            case CAR_OBTAIN_SINGLE:
                obtain = IsCupWonSingleRaces(ci->Rating);
                break;

            case CAR_OBTAIN_NEVER:
#ifdef _PC
                if ((i == CARID_UFO) && AllowUFO) obtain = TRUE;
#endif
                break;
        }

        ci->Selectable = obtain;
    }
}

/////////////////////////////////////////////////////////////////////
// InitCar: set all pointers to NULL and zero any counts
/////////////////////////////////////////////////////////////////////

void InitCar(CAR *car)
{

// init model / coll misc

    InitBodyDefault(car->Body);
    car->Models = NULL;

// init timers stuff

    car->NextSplit = 0;
    car->NextTrackDir = 0;
    car->Laps = 0;
    car->CurrentLapTime = 0;
    car->LastLapTime = 0;
    car->LastRaceTime = 0;
#ifndef _PSX
    car->BestLapTime = MAKE_TIME(5, 0, 0);
    car->BestRaceTime = MAKE_TIME(60, 0, 0);
#endif
    car->Best0to15 = 100;
    car->Best0to25 = 100;
    car->Current0to15 = -ONE;
    car->Current0to25 = -ONE;
    car->Timing0to15 = FALSE;
    car->Timing0to25 = FALSE;

// init misc

    car->Righting = FALSE;
    car->AddLit = 0;
    car->DrawScale = ONE;
    car->PowerTimer = -ELECTROPULSE_NO_RETURN_TIME;
    car->IsBomb = FALSE;
    car->WillDetonate = FALSE;
    car->NoReturnTimer = ZERO;
    car->IsFox = FALSE;
    car->FoxReturnTimer = ZERO;
    car->RepositionTimer = ZERO;
    car->LastHitTimer = ZERO;
    car->Rendered = FALSE;
    car->RenderedAll = FALSE;
#ifndef _PSX
  #pragma message( "JHarding needs to port the code below, I think." )
  #ifndef XBOX_NOT_YET_IMPLEMENTED
    car->SfxEngine = NULL;
    car->SfxScrape = NULL;
    car->SfxScreech = NULL;
    car->SfxServo = NULL;
  #endif // ! XBOX_NOT_YET_IMPLEMENTED
#endif
#ifdef _PC
    for (long i = 0 ; i < 3 ; i++)
    {
        car->RemoteData[i].Time = 0;
    }
#endif
}


/////////////////////////////////////////////////////////////////////
//
// NextValidCarType:
// PrevValidCarType:
//
/////////////////////////////////////////////////////////////////////

//$CMP_NOTE: in this function, renamed "ID" to "Type" in lots of places
int NextValidCarType(int iCurrType)  //$CMP_NOTE: this was originally named NextValidCarID()
{
    int carType;

    // Check all higher car types
    for (carType = iCurrType + 1; carType < NCarTypes; carType++) {
#ifndef _PC
        if (CarInfo[carType].Selectable) {
#else
        if (CarInfo[carType].Selectable && !CarInfo[carType].Moved) {
#endif
            {
                return carType;
            }
        }
    }
    // Check lower car types
    for (carType = 0; carType < iCurrType; carType++) {
#ifndef _PC
        if (CarInfo[carType].Selectable) {
#else
        if (CarInfo[carType].Selectable && !CarInfo[carType].Moved) {
#endif
            {
                return carType;
            }
        }
    }
    // No other selectables - return input value
    return iCurrType;
}

int PrevValidCarType(int iCurrType)
{
    int carType;

    // Check all lower car types
    for (carType = iCurrType - 1; carType >= 0; carType--) {
#ifndef _PC
        if (CarInfo[carType].Selectable) {
#else
        if (CarInfo[carType].Selectable && !CarInfo[carType].Moved) {
#endif
            {
                return carType;
            }
        }
    }
    // Check higher car types
    for (carType = NCarTypes - 1; carType > iCurrType; carType--) {
#ifndef _PC
        if (CarInfo[carType].Selectable) {
#else
        if (CarInfo[carType].Selectable && !CarInfo[carType].Moved) {
#endif
            {
                return carType;
            }
        }
    }
    // No other selectables - return input value
    return iCurrType;
}

/////////////////////////////////////////////////////////////////////
// SetupCar: copy information for the car of type "carType" from the
// CarInfo storage space into the specified CAR structure
/////////////////////////////////////////////////////////////////////

void SetupCar(struct PlayerStruct *player, int carType)
{
    long    ii;
    REAL    y;
    CAR     *car = &player->car;

    // load models + tpage
    if ((player->type != PLAYER_GHOST) || GHO_ShowGhost) {
#ifndef _PSX
        if (car->Models) {
            FreeOneCarModelSet(player);
        }
        car->Models = &player->carmodels;
        LoadOneCarModelSet(player, carType);

#else
        //LoadCar( player - Players, carType );
        car->Models = &Car_Model[player - Players];
#endif

        // Create / re-create the world-frame collision skins
        if (car->Body->CollSkin.WorldConvex != NULL) {
            DestroyConvex(car->Body->CollSkin.WorldConvex, car->Body->CollSkin.NConvex);
            car->Body->CollSkin.WorldConvex = NULL;
        }
        if (car->Body->CollSkin.OldWorldConvex != NULL) {
            DestroyConvex(car->Body->CollSkin.OldWorldConvex, car->Body->CollSkin.NConvex);
            car->Body->CollSkin.OldWorldConvex = NULL;
        }
        if (car->Body->CollSkin.WorldSphere != NULL) {
            DestroySpheres(car->Body->CollSkin.WorldSphere);
            car->Body->CollSkin.WorldSphere = NULL;
        }
        if (car->Body->CollSkin.OldWorldSphere != NULL) {
            DestroySpheres(car->Body->CollSkin.OldWorldSphere);
            car->Body->CollSkin.OldWorldSphere = NULL;
        }
        car->Body->CollSkin.NConvex = car->Models->CollSkin.NConvex;
        car->Body->CollSkin.NSpheres = car->Models->CollSkin.NSpheres;
        car->Body->CollSkin.Convex = car->Models->CollSkin.Convex;
        car->Body->CollSkin.Sphere = car->Models->CollSkin.Sphere;
        CopyBBox(&car->Models->CollSkin.TightBBox, &car->Body->CollSkin.TightBBox);
#ifndef _N64
        CreateCopyCollSkin(&car->Body->CollSkin);
#else
        if (!CreateCopyCollSkin(&car->Body->CollSkin))
        {
            ERROR("CAR", "SetupCar", "CreateCopyCollSkin failed", 1);
        }
    
#endif

    } else {
        // Do not have any models loaded for ghost cars if we can't have them
#ifndef _PSX
        car->Models = &player->carmodels;
        car->Models->BodyPartsFlag = 0;
        car->Models->WheelPartsFlag[0] = 0;
        car->Models->WheelPartsFlag[1] = 0;
        car->Models->WheelPartsFlag[2] = 0;
        car->Models->WheelPartsFlag[3] = 0;
#else
        car->Models = &Car_Model[player - Players];
#endif
    }


#ifndef _PSX

    // fix env matrix
    CopyMat(&IdentityMatrix, &car->EnvMatrix);

#endif


    // Set car type
    car->CarType = carType;

    // Set car misc stuff
    car->SteerRate = CarInfo[carType].SteerRate;
    car->SteerModifier = CarInfo[carType].SteerModifier;
    car->EngineRate = CarInfo[carType].EngineRate;
    car->TopSpeed = car->DefaultTopSpeed = CarInfo[carType].TopSpeed;

    //car->MaxRevs = CarInfo[carType].MaxRevs;
    car->MaxRevs = CarInfo[carType].TopEnd;


    car->AllowedBestTime = CarInfo[carType].AllowedBestTime;
    car->DownForceMod = CarInfo[carType].DownForceMod;
    car->Selectable = CarInfo[carType].Selectable;
    car->ObtainMethod = CarInfo[carType].ObtainMethod;
    car->Class = CarInfo[carType].Class;
    car->Rating = CarInfo[carType].Rating;
    CopyVec(&CarInfo[carType].WeaponOffset, &car->WeaponOffset);
    car->Best0to15 = -ONE;
    car->Best0to25 = -ONE;
    car->Current0to15 = ZERO;
    car->Current0to25 = ZERO;
    car->AerialTimer = TO_TIME(Real(0.2));

    car->NWheelColls = 0;
    car->NWheelFloorContacts = 0;
    car->NWheelsInContact = 0;
    car->NoReturnTimer = ZERO;

    // Car Body Stuff
    SetBodyConvex(car->Body);
    CopyVec(&CarInfo[carType].Body.Offset, &car->BodyOffset);
    SetParticleMass(&car->Body->Centre, CarInfo[carType].Body.Mass);
    SetBodyInertia(car->Body, &CarInfo[carType].Body.Inertia);
    car->Body->Centre.Gravity = CarInfo[carType].Body.Gravity;
    car->Body->Centre.Hardness = CarInfo[carType].Body.Hardness;
    car->Body->Centre.Resistance = CarInfo[carType].Body.Resistance;
    car->Body->Centre.StaticFriction = CarInfo[carType].Body.StaticFriction;
    car->Body->Centre.KineticFriction = CarInfo[carType].Body.KineticFriction;
    car->Body->DefaultAngRes = CarInfo[carType].Body.AngResistance;
    car->Body->AngResistance = CarInfo[carType].Body.AngResistance;
    car->Body->AngResMod = CarInfo[carType].Body.ResModifier;
    car->Body->Centre.Grip = CarInfo[carType].Body.Grip;
    car->Body->AllowSparks = TRUE;

    // Jitter stuff
#if REMOVE_JITTER
    car->Body->JitterCountMax = 2;
    car->Body->JitterFramesMax = 10;
#endif

    // Car Wheel Stuff
    for (ii = 0; ii < CAR_NWHEELS; ii++) {
        // Set the wheel offset and adjust for spring compression
        CopyVec(&CarInfo[carType].Wheel[ii].Offset1, &car->WheelOffset[ii]);
        //CopyVec(&CarInfo[carType].Wheel[ii].Offset2, &car->WheelCentre[ii]);
        VecPlusVec(&CarInfo[carType].Wheel[ii].Offset1,&CarInfo[carType].Wheel[ii].Offset2, &car->WheelCentre[ii]);
        // Set up the rest of the wheel stuff
        SetupWheel(&car->Wheel[ii], &CarInfo[carType].Wheel[ii]);

    }

    // Setup steer ratio vars
#ifndef _PSX
    car->fullLockAngle = abs(MulScalar(car->Wheel[FL].SteerRatio, RAD2DEG));
#else
    car->fullLockAngle = abs(car->Wheel[FL].SteerRatio * 360);
#endif
    car->AISteerConvert = DivScalar(Real(90), car->fullLockAngle);


    // Suspension
    for (ii = 0; ii < CAR_NWHEELS; ii++) {
#ifndef _PSX
        car->Sus[ii].SpringLen = car->Models->SpringLen[ii];
        car->Sus[ii].AxleLen = car->Models->AxleLen[ii];
        car->Sus[ii].PinLen = car->Models->PinLen[ii];
#endif
        CopyVec(&CarInfo[carType].Spring[ii].Offset, &car->SuspOffset[ii]);
        CopyVec(&CarInfo[carType].Axle[ii].Offset, &car->AxleOffset[ii]);
        SetupSuspension(&car->Spring[ii], &CarInfo[carType].Spring[ii]);
    }

    // Set up the car's spinny thing
#ifndef _PSX
    CopyMat(&Identity, &car->Spinner.Matrix);
    CopyVec(&CarInfo[carType].Spinner.Offset, &car->SpinnerOffset);
    CopyVec(&CarInfo[carType].Spinner.Axis, &car->Spinner.Axis);
    car->Spinner.AngVel = CarInfo[carType].Spinner.AngVel;
#endif

    // Set up the car's aerial
    if (CarHasAerial(car)) {
        CopyVec(&CarInfo[carType].Aerial.Offset, &car->AerialOffset);
#ifndef _PSX
        InitAerial(&car->Aerial, 
            &car->Models->DirAerial, 
            car->Models->AerialLen / 5, 
            1.0f, 1.0f, 0.00f, 100.0f);

            SetAerialSprings(&car->Aerial, 
            CarInfo[carType].Aerial.Stiffness, 
            CarInfo[carType].Aerial.Damping,
            6000.0f);

#else 
        InitAerial(&car->Aerial, 
            &UpVec, 
            TO_LENGTH(20<<16), 
            Real(1.0f), Real(1.0f), Real(0.00f), Real(100.0f));

            SetAerialSprings(&car->Aerial, 
            CarInfo[carType].Aerial.Stiffness, 
            CarInfo[carType].Aerial.Damping>>6,
            Real(6000.0f));

#endif
    }

    // Set position to get the bounding box from which to calculate the Arcade collision ball dimensions
    SetCarPos(car, &ZeroVector, &Identity);

    // Car radius for Arcade-mode collision
    car->CollRadius = (
        (car->BBox.XMax - car->Body->Centre.Pos.v[X]) - 
        (car->BBox.XMin - car->Body->Centre.Pos.v[X]) + 
        (car->BBox.ZMax - car->Body->Centre.Pos.v[Z]) -
        (car->BBox.ZMin - car->Body->Centre.Pos.v[Z])) / 4;

    // Build collision spheres for arcade mode
    y = (car->BBox.YMax + car->BBox.YMin) / 2;
    if (DivScalar(car->BBox.ZMax - car->BBox.ZMin, car->BBox.XMax - car->BBox.XMin) > Real(1.2)) {
        // Two spheres
        car->NBodySpheres = 2;
        car->CollSphere[0].Radius = car->CollSphere[1].Radius = 
        car->BodySphere[0].Radius = car->BodySphere[1].Radius = MulScalar(Real(1.2), (car->BBox.XMax - car->BBox.XMin))/2;
        SetVec(&car->BodySphere[0].Pos, ZERO, y, car->BBox.ZMax - car->BodySphere[0].Radius);
        SetVec(&car->BodySphere[1].Pos, ZERO, y, car->BBox.ZMin + car->BodySphere[0].Radius);
    } else {
        // One sphere
        car->NBodySpheres = 1;
        car->CollSphere[0].Radius = car->BodySphere[0].Radius = MulScalar(Real(1.4), (car->BBox.ZMax - car->BBox.ZMin))/2;
        car->CollSphere[1].Radius = car->BodySphere[1].Radius = ZERO;
        SetVec(&car->BodySphere[0].Pos, ZERO, y, (car->BBox.ZMax + car->BBox.ZMin)/2);
        SetVecZero(&car->BodySphere[1].Pos);
    }


    // Do "Kids Mode" modifications
    if (GameSettings.PlayMode == PLAYMODE_KIDS) {
        car->SteerRate = car->SteerRate / 2;
        car->TopSpeed = car->DefaultTopSpeed = (car->TopSpeed * 6) / 10;
        for(ii = 0; ii < CAR_NWHEELS; ii++) {
            car->Wheel[ii].StaticFriction = (car->Wheel[ii].StaticFriction * 5) / 4;
            car->Wheel[ii].KineticFriction = car->Wheel[ii].StaticFriction;
        }
    }

// Slow player down in snail mode
#ifndef _PSX
    if (SnailMode) // && (Type != PLAYER_LOCAL)) 
    {
        SetPlayerToSnail(car);
    }
#endif

// Setup ghost car cloth
#ifdef _PC
    GhostCarInitCloth(car);
#endif
}

/////////////////////////////////////////////////////////////////////
//
// FreeCar: Deallocate any allocated ram set up in SetupCar
//
/////////////////////////////////////////////////////////////////////

void FreeCar(struct PlayerStruct *player)
{
    CAR *car = &player->car;

#ifndef _PSX
    FreeOneCarModelSet(player);
#endif
    DestroyConvex(car->Body->CollSkin.OldWorldConvex, car->Body->CollSkin.NConvex);
    DestroyConvex(car->Body->CollSkin.WorldConvex, car->Body->CollSkin.NConvex);
    car->Body->CollSkin.OldWorldConvex = NULL;
    car->Body->CollSkin.WorldConvex = NULL;

    DestroySpheres(car->Body->CollSkin.OldWorldSphere);
    DestroySpheres(car->Body->CollSkin.WorldSphere);
    car->Body->CollSkin.OldWorldSphere = NULL;
    car->Body->CollSkin.WorldSphere = NULL;

#ifdef OLD_AUDIO
    //$NOTE: nothing equivalent was here before
#else // !OLD_AUDIO
    //$ADDITION
    if( car->pSourceMix )
    {
        g_SoundEngine.ReturnInstance( car->pSourceMix );
        car->pSourceMix = NULL;
    }
    if( car->pEngineSound )
    {
        g_SoundEngine.ReturnInstance( car->pEngineSound );
        car->pEngineSound = NULL;
    }
    if( car->pScrapeSound )
    {
        g_SoundEngine.ReturnInstance( car->pScrapeSound );
        car->pScrapeSound = NULL;
    }
    if( car->pScreechSound )
    {
        g_SoundEngine.ReturnInstance( car->pScreechSound );
        car->pScreechSound = NULL;
    }
    //$END_ADDITION
    #pragma message( "JHarding needs to add pServoSound, I think." )
#endif // !OLD_AUDIO
}

///////////////////////////////////
// get a grid position for a car //
///////////////////////////////////

void GetCarStartGrid(long position, VEC *pos, MAT *mat)  //$CMP_NOTE: this was originally named GetCarGrid()
{
    GRID_POS *grid;
    VEC off;
    MAT mat2;

// frig if clockwork game
#ifdef _PC
    if (GameSettings.GameType == GAMETYPE_CLOCKWORK || (GameSettings.GameType == GAMETYPE_REPLAY && StartDataStorage.GameType == GAMETYPE_CLOCKWORK))
    {
        RotMatrixY(mat, -LEV_StartRot);

        SetVector(&off, (float)(position % 5) * 80.0f - 40.0f, 0.0f, (float)(position / 5) * -100.0f);
        VecMulMat(&off, mat, pos);
        VecPlusEqVec(pos, &LEV_StartPos);
        return;
    }
#endif
// get grid pos

    grid = &CarGridStarts[LEV_StartGrid][position];

// get start matrix

#ifndef _PSX
    RotMatrixY(mat, -LEV_StartRot - grid->rotoff);
    RotMatrixY(&mat2, -LEV_StartRot);
#else
    
    
    RotMatrixY(mat, -LEV_StartRot - grid->rotoff);
    RotMatrixY(&mat2, -LEV_StartRot);

//  SetMatUnit(mat);
//  SetMatUnit(&mat2);

#endif

// transform start offset into start pos

    SetVector(&off, grid->xoff, grid->yoff, grid->zoff);
    VecMulMat(&off, &mat2, pos);
    VecPlusEqVec(pos, &LEV_StartPos);

}

///////////////////////////////////////
// set a cars position / orientation //
///////////////////////////////////////

void SetCarPos(CAR *car, VEC *pos, MAT *mat)
{
    int iWheel;

    // Set the body position and orientation
    SetBodyPos(car->Body, pos, mat);

    // Other stuff
    car->SteerAngle = ZERO;
    car->LastSteerAngle = ZERO;
    car->EngineVolt = ZERO;
    car->LastEngineVolt = ZERO;
#ifdef _PC
    car->ScrapeMaterial = MATERIAL_NONE;
    car->SkidMaterial = MATERIAL_NONE;
    car->ServoFlag = 0.0f;
    car->InWater = FALSE;
#endif

    car->NWheelColls = 0;
    car->NWheelFloorContacts = 0;
    car->NWheelsInContact = 0;

    
    // Set Wheel orientation and positions
    for (iWheel = 0; iWheel < CAR_NWHEELS; iWheel++) {
        ResetCarWheelPos(car, iWheel);
    }

    // Set car's bounding box
    InitWorldSkin(&car->Body->CollSkin, &car->Body->Centre.Pos, &car->Body->Centre.WMatrix);
    CopyBBox(&car->Body->CollSkin.BBox, &car->BBox);
    for (iWheel = 0; iWheel < CAR_NWHEELS; iWheel++) {
        AddPosRadToBBox(&car->BBox, &car->Wheel[iWheel].CentrePos, car->Wheel[iWheel].Radius);
    }

    // Set the starting aerial position
#ifndef _PSX
    if (CarHasAerial(car)) 
#endif
    {
        SetCarAerialPos(car);
    }

#ifdef _PC
    SetVecZero(&car->FieldVec);

// Setup the remote car data stores

    InitRemoteCarData(car);
#endif
}

////////////////////////////////////////////////////////////////
//
// UpdateCarPos:
//
////////////////////////////////////////////////////////////////

void UpdateCarPos(CAR *car, VEC *pos, MAT *mat)
{
    int iWheel;

    // Set the body position and orientation
    SetBodyPos(car->Body, pos, mat);

    // Other stuff
    car->SteerAngle = ZERO;
    car->LastSteerAngle = ZERO;
    car->EngineVolt = ZERO;
    car->LastEngineVolt = ZERO;
    
    // Set Wheel orientation and positions
    for (iWheel = 0; iWheel < CAR_NWHEELS; iWheel++) {
        ResetCarWheelPos(car, iWheel);
    }

    // Set car's bounding box
    CopyBBox(&car->Body->CollSkin.BBox, &car->BBox);
    for (iWheel = 0; iWheel < CAR_NWHEELS; iWheel++) {
        AddPosRadToBBox(&car->BBox, &car->Wheel[iWheel].CentrePos, car->Wheel[iWheel].Radius);
    }

    // Set the starting aerial position
    UpdateCarAerial2(car, TimeStep);

}
/////////////////////////////////////////////////////////////////////
//
// InitRemoteCarData: initialise the remote car data to the current
// car's setup (which will be set up at the starting grid.
//
/////////////////////////////////////////////////////////////////////
#ifdef _PC
void InitRemoteCarData(CAR *car)
{
    long i;

    car->OldDat = 0;
    car->Dat = 1;
    car->NewDat = 2;
    car->RemoteNStored = 0;
    car->RemoteTimeout = FALSE;

    for (i = 0 ; i < 3 ; i++)
    {
        car->RemoteData[i].NewData = FALSE;
        CopyVec(&car->Body->Centre.Pos, &car->RemoteData[i].Pos);
        CopyVec(&car->Body->Centre.Vel, &car->RemoteData[i].Vel);
        CopyVec(&car->Body->AngVel, &car->RemoteData[i].AngVel);
        CopyQuat(&car->Body->Centre.Quat, &car->RemoteData[i].Quat);
    }
}

#endif

/////////////////////////////////////////////////////////////////////
//
// NextRemoteData: return a pointer to the oldest remote data
// structure for filling with data from the broadcast packet.
// Also reflects the change in the data-order indices
//
/////////////////////////////////////////////////////////////////////

CAR_REMOTE_DATA *NextRemoteData(CAR *car)
{
    int tmp;

    // Shift data stores and return pointer to oldest for overwriting
    tmp = car->OldDat;
    car->OldDat = car->Dat;
    car->Dat = car->NewDat;
    car->NewDat = tmp;
    car->RemoteNStored++;

    if (car->RemoteNStored > 3) car->RemoteNStored = 3;

    return &car->RemoteData[tmp];

}

#ifdef _PC

/////////////////////////
// send local car data //
/////////////////////////

void SendCarData(OBJECT *obj)
{
    CAR_REMOTE_DATA_SEND *data = (CAR_REMOTE_DATA_SEND*)(SendHeader + 1);
    CAR *car = &obj->player->car;

// setup header

    SendHeader->Type = MESSAGE_CAR_DATA;

#ifndef XBOX_DISABLE_NETWORK
//$ADDITION_BEGIN
    // set player ID
    *((DWORD*)ptr) = LocalPlayerID;
    ptr += sizeof(DWORD);
//$ADDITION_END
#endif // ! XBOX_DISABLE_NETWORK

// set pos

    CopyVec(&car->Body->Centre.Pos, &data->Pos);

// set vel

    data->VelX = (short)(car->Body->Centre.Vel.v[X] * REMOTE_VEL_SCALE);
    data->VelY = (short)(car->Body->Centre.Vel.v[Y] * REMOTE_VEL_SCALE);
    data->VelZ = (short)(car->Body->Centre.Vel.v[Z] * REMOTE_VEL_SCALE);

// set ang vel

    data->AngVelX = (short)(car->Body->AngVel.v[X] * REMOTE_ANGVEL_SCALE);
    data->AngVelY = (short)(car->Body->AngVel.v[Y] * REMOTE_ANGVEL_SCALE);
    data->AngVelZ = (short)(car->Body->AngVel.v[Z] * REMOTE_ANGVEL_SCALE);

// set quat

    data->QuatX = (char)(car->Body->Centre.Quat.v[VX] * REMOTE_QUAT_SCALE);
    data->QuatY = (char)(car->Body->Centre.Quat.v[VY] * REMOTE_QUAT_SCALE);
    data->QuatZ = (char)(car->Body->Centre.Quat.v[VZ] * REMOTE_QUAT_SCALE);
    data->QuatW = (char)(car->Body->Centre.Quat.v[S] * REMOTE_QUAT_SCALE);

// set control input

    data->dx = PLR_LocalPlayer->controls.lastdx;
    data->dy = PLR_LocalPlayer->controls.lastdy;
    data->digital = PLR_LocalPlayer->controls.adigital;
    CON_RationaliseControl(&data->digital);
    PLR_LocalPlayer->controls.adigital = 0;

// set time

    data->Time = TotalRacePhysicsTime;

// send

    QueueMessage(SendHeader, sizeof(MESSAGE_HEADER) + sizeof(CAR_REMOTE_DATA_SEND));
#ifndef XBOX_DISABLE_NETWORK
  #error BUGBUG CPRINCE: make sure that CAR_REMOTE_DATA_SEND includes size of my added LocalPlayerID field (if still needed)!
  #pragma message( "CPrince needs to make sure this does the right thing.  (Differs from old network code.)" )
#endif // ! XBOX_DISABLE_NETWORK

// send bomb tag clock?

    if (GameSettings.GameType == GAMETYPE_BATTLE)
    {
        SendBombTagClock();
    }
}

//////////////////
// get car data //
//////////////////

void ProcessCarData()
{
    CAR *car;
    PLAYER *player;
    CAR_REMOTE_DATA_SEND *data = (CAR_REMOTE_DATA_SEND*)(ReceiveHeader + 1);
    CAR_REMOTE_DATA *rem;

// get relevant player / car

    player = GetPlayerFromPlayerID(FromID);
    if (!player)
        return;

    car = &player->car;

// inc packet count

    player->CarPacketCount++;

// get remote data struct to fill

    rem = NextRemoteData(car);

// get pos

    CopyVec(&data->Pos, &rem->Pos);

// get vel

    rem->Vel.v[X] = data->VelX / REMOTE_VEL_SCALE;
    rem->Vel.v[Y] = data->VelY / REMOTE_VEL_SCALE;
    rem->Vel.v[Z] = data->VelZ / REMOTE_VEL_SCALE;

// get ang vel

    rem->AngVel.v[X] = data->AngVelX / REMOTE_ANGVEL_SCALE;
    rem->AngVel.v[Y] = data->AngVelY / REMOTE_ANGVEL_SCALE;
    rem->AngVel.v[Z] = data->AngVelZ / REMOTE_ANGVEL_SCALE;

// get quat

    rem->Quat.v[VX] = (REAL)data->QuatX / REMOTE_QUAT_SCALE;
    rem->Quat.v[VY] = (REAL)data->QuatY / REMOTE_QUAT_SCALE;
    rem->Quat.v[VZ] = (REAL)data->QuatZ / REMOTE_QUAT_SCALE;
    rem->Quat.v[S] = (REAL)data->QuatW / REMOTE_QUAT_SCALE;
    NormalizeQuat(&rem->Quat);

// get control input

    rem->dx = data->dx;
    rem->dy = data->dy;
    rem->digital = data->digital;

// get time

    rem->Time = data->Time;

// set new data

    rem->NewData = TRUE;
}

/////////////////////////
// send new car packet //
/////////////////////////

void SendCarNewCar(long car)
{
    CAR_REMOTE_NEWCAR *data = (CAR_REMOTE_NEWCAR*)(SendHeader + 1);

// setup header

    SendHeader->Type = MESSAGE_CAR_NEWCAR;

// set car

    data->Car = car;

// send

    QueueMessage(SendHeader, sizeof(MESSAGE_HEADER) + sizeof(CAR_REMOTE_NEWCAR));
}

//////////////////////
// get new car data //
//////////////////////

void ProcessCarNewCar(void)
{
    CAR_REMOTE_NEWCAR *data = (CAR_REMOTE_NEWCAR*)(ReceiveHeader + 1);
    PLAYER *player;

// setup new car

    player = GetPlayerFromPlayerID(FromID);
    if (!player)
        return;

    VEC pos;
    MAT mat;

    CopyVec(&player->car.Body->Centre.Pos, &pos);
    CopyMat(&player->car.Body->Centre.WMatrix, &mat);

    SetupCar(player, data->Car);

    SetCarPos(&player->car, &pos, &mat);
    SetCarAerialPos(&player->car);
}

/////////////////////////////
// send new car all packet //
/////////////////////////////

void SendCarNewCarAll(long car)
{
    CAR_REMOTE_NEWCAR *data = (CAR_REMOTE_NEWCAR*)(SendHeader + 1);
    PLAYER *player;

// setup header

    SendHeader->Type = MESSAGE_CAR_NEWCAR_ALL;

// set car

    data->Car = car;

// send

    QueueMessage(SendHeader, sizeof(MESSAGE_HEADER) + sizeof(CAR_REMOTE_NEWCAR));

// change all cars

    for (player = PLR_PlayerHead ; player ; player = player->next)
    {
        if (player->type == PLAYER_LOCAL || player->type == PLAYER_REMOTE)
        {
            VEC pos;
            MAT mat;

            CopyVec(&player->car.Body->Centre.Pos, &pos);
            CopyMat(&player->car.Body->Centre.WMatrix, &mat);

            SetupCar(player, data->Car);

            SetCarPos(&player->car, &pos, &mat);
            SetCarAerialPos(&player->car);
        }
    }
}

//////////////////////////
// get new car all data //
//////////////////////////

void ProcessCarNewCarAll(void)
{
    CAR_REMOTE_NEWCAR *data = (CAR_REMOTE_NEWCAR*)(ReceiveHeader + 1);
    PLAYER *player;

// play sfx

    PlaySfx(SFX_HONK, SFX_MAX_VOL, SFX_CENTRE_PAN, SFX_SAMPLE_RATE, 0);

// change all cars

    for (player = PLR_PlayerHead ; player ; player = player->next)
    {
        if (player->type == PLAYER_LOCAL || player->type == PLAYER_REMOTE)
        {
            VEC pos;
            MAT mat;

            CopyVec(&player->car.Body->Centre.Pos, &pos);
            CopyMat(&player->car.Body->Centre.WMatrix, &mat);

            SetupCar(player, data->Car);

            SetCarPos(&player->car, &pos, &mat);
            SetCarAerialPos(&player->car);
        }
    }
}

#endif

/////////////////////////////////////////////////////////////////////
//
// SetCarAerialPos: set the aerial's starting coords from the car's
//
/////////////////////////////////////////////////////////////////////


void SetCarAerialPos(CAR *car)
{
    VEC vecTemp;
    int iSec, iCount;
    PARTICLE *pSection = &car->Aerial.Section[0];
    iCount = 0;

    car->Aerial.visibleLast = FALSE;

    // Set position of the base section and the aerial direction
    VecMulMat(&car->Aerial.Direction, &car->Body->Centre.WMatrix, &car->Aerial.WorldDirection);
    VecMulMat(&car->AerialOffset, &car->Body->Centre.WMatrix, &vecTemp);
    VecPlusVec(&vecTemp, &car->Body->Centre.Pos, &pSection->Pos);

    SetVecZero(&pSection->Vel);
    SetVecZero(&pSection->Acc);
    SetVecZero(&pSection->Impulse);

    // Place all control sections in the correct place and set up their matrices
    for (iSec = AERIAL_START; iSec < AERIAL_NSECTIONS; iSec += AERIAL_SKIP) {
        iCount++;
        // Store a pointer to the current section (avoid excessive dereferencing)
        pSection = &car->Aerial.Section[iSec];

        // Set the section positions
        VecEqScalarVec(&pSection->Pos, iCount * car->Aerial.Length, &car->Aerial.WorldDirection);
        VecPlusEqVec(&pSection->Pos, &car->Aerial.Section[0].Pos);
        CopyVec(&pSection->Pos, &pSection->OldPos);

        // Zero the velocities
        SetVecZero(&pSection->Vel);
        SetVecZero(&pSection->Acc);
        SetVecZero(&pSection->Impulse);
        
    }
}

/////////////////////////////////////////////////////////////////////
//
// UpdateCarAerial: update the aerial sections 
//
/////////////////////////////////////////////////////////////////////


#if defined(_PC) // || defined(_N64)
void UpdateCarAerial2(CAR *car, REAL dt)
{
    int iSec;
    VEC vecTemp;
    VEC dRPrev;
    REAL prevLen;
    REAL scale;
    VEC dRThis;
    REAL thisLen;
    VEC thisCrossPrev;
    REAL crossLen;
    REAL velDotThis;
    REAL velDotDir;
    VEC velPar;
    VEC impulse = {ZERO, ZERO, ZERO};
    VEC dRTot = {ZERO, ZERO, ZERO};


    // Don't bother if car not rendered last frame
    if (!car->RenderedAll) {
        SetCarAerialPos(car);
        return;
    }

    // Calculate the world position of the aerial base from the car's
    // world position and world matrix
    // Calculate the aerial's look direction and set the world matrix
    VecMulMat(&car->AerialOffset, &car->Body->Centre.WMatrix, &vecTemp);
    VecPlusVec(&vecTemp, &car->Body->Centre.Pos, &car->Aerial.Section[0].Pos);
    VecMulMat(&car->Aerial.Direction, &car->Body->Centre.WMatrix, &dRPrev);

    VecEqScalarVec(&car->Aerial.WorldDirection, -35, &dRPrev);

    prevLen = ONE;

    // calculate the position of the controlled nodes (others are interpolated)
    for (iSec = AERIAL_START; iSec < AERIAL_NSECTIONS; iSec += AERIAL_SKIP) {

        // Update the position of the node
        UpdateParticle(&car->Aerial.Section[iSec], dt);

        // Calculate the length of this section of aerial and
        // the unit vector along this aerial section
        VecMinusVec(&car->Aerial.Section[iSec].Pos, &car->Aerial.Section[iSec - AERIAL_SKIP].Pos, &dRThis);
        thisLen = Length(&dRThis);
        if (thisLen > SMALL_REAL) {
            VecDivScalar(&dRThis, thisLen);
        } else {
            thisLen = ONE;
            CopyVec(&car->Aerial.Direction, &dRThis);
        }
        
        // Move aerial section to keep aerial length constant
        VecPlusEqScalarVec(&car->Aerial.Section[iSec].Pos, car->Aerial.Length - thisLen, &dRThis);

        // Calculate force on the end of the section due to the bend of the aerial
        VecCrossVec(&dRThis, &dRPrev, &thisCrossPrev);
        crossLen = VecLen(&thisCrossPrev);
        VecCrossVec(&dRThis, &thisCrossPrev, &vecTemp);
        scale = MulScalar3(car->Aerial.Length, car->Aerial.Stiffness, crossLen);
        //scale = MulScalar(scale, crossLen);
        VecEqScalarVec(&impulse, -scale, &vecTemp);

        // add the spring damping
        VecMinusVec(&car->Aerial.Section[iSec].Vel, &car->Aerial.Section[0].Vel, &velPar);
        velDotDir = VecDotVec(&velPar, &dRPrev);
        VecPlusEqScalarVec(&velPar, -velDotDir, &dRPrev);
        VecPlusEqScalarVec(&impulse, -car->Aerial.Damping, &velPar);

        // Turn the force into an impulse
        VecMulScalar(&impulse, dt);

        // Apply the impulse to the node from the force
        ApplyParticleImpulse(&car->Aerial.Section[iSec], &impulse);

        // This node will now become previous node
        CopyVec(&dRThis, &dRPrev);
        prevLen = thisLen;

        // Fudge the position so that the section lengths are restored
        VecMinusVec(&car->Aerial.Section[iSec].Pos, &car->Aerial.Section[iSec - AERIAL_SKIP].Pos, &dRThis);
        thisLen = VecLen(&dRThis);  
        if (thisLen > SMALL_REAL) {
            VecDivScalar(&dRThis, thisLen);
        } else {
            thisLen = ONE;
            CopyVec(&car->Aerial.Direction, &dRThis);
        }
        VecPlusEqScalarVec(&car->Aerial.Section[iSec].Pos, car->Aerial.Length - thisLen, &dRThis);

        // Check for collisions
#ifdef _PC
        ParticleWorldColls(&car->Aerial.Section[iSec]);
        VecMinusVec(&car->Aerial.Section[iSec].Pos, &car->Aerial.Section[iSec - AERIAL_SKIP].Pos, &dRThis);
        thisLen = VecLen(&dRThis);  
        if (thisLen > SMALL_REAL) {
            VecDivScalar(&dRThis, thisLen);
        } else {
            thisLen = ONE;
            CopyVec(&car->Aerial.Direction, &dRThis);
        }
        VecPlusEqScalarVec(&car->Aerial.Section[iSec].Pos, car->Aerial.Length - thisLen, &dRThis);
#endif

        // remove component of velocity along direction of the aerial
        VecMinusVec(&car->Aerial.Section[iSec].Vel, &car->Aerial.Section[iSec - AERIAL_SKIP].Vel, &vecTemp);
        velDotThis = VecDotVec(&vecTemp, &dRThis);
        VecPlusEqScalarVec(&car->Aerial.Section[iSec].Vel, -velDotThis, &dRThis);

    }

    // Add a random impulse to the end of the aerial
    if (frand(1) < TimeStep * 80) {
        scale = VecDotVec(&car->Body->Centre.Vel, &car->Body->Centre.Vel);
        if (scale > 1000000) scale = 1000000;
        scale *= TimeStep * 10;
        if (scale > 100) {
            VecEqScalarVec(&impulse, (0.001f - frand(0.002f)) * scale, &car->Body->Centre.WMatrix.mv[R]);
            //SetVec(&impulse, (0.001f - frand(0.002f)) * scale, (0.001f - frand(0.002f)) * scale, (0.001f - frand(0.002f)) * scale);
            ApplyParticleImpulse(&car->Aerial.Section[AERIAL_LASTSECTION], &impulse);
        }
    }
}

/*#elif defined(_PSX) // _PSX

void UpdateCarAerial2(CAR *car, REAL dt)
{
    VEC vecTemp;

    VecMulMat(&car->AerialOffset, &car->Body->Centre.WMatrix, &vecTemp);
    VecPlusVec(&vecTemp, &car->Body->Centre.Pos, &car->Aerial.Section[0].Pos);

    VecPlusScalarVec(&car->Aerial.Section[0].Pos, -car->Aerial.Length, &car->Body->Centre.WMatrix.mv[U], &car->Aerial.Section[1].Pos);
    VecPlusScalarVec(&car->Aerial.Section[1].Pos, -car->Aerial.Length, &car->Body->Centre.WMatrix.mv[U], &car->Aerial.Section[2].Pos);

    car->AerialPos[0] = car->Aerial.Section[0].Pos;
    car->AerialPos[2] = car->Aerial.Section[1].Pos;
    car->AerialPos[4] = car->Aerial.Section[2].Pos;

    Interpolate3D( &car->Aerial.Section[0].Pos, &car->Aerial.Section[1].Pos, &car->Aerial.Section[2].Pos, 16384, &car->AerialPos[1] );
    Interpolate3D( &car->Aerial.Section[0].Pos, &car->Aerial.Section[1].Pos, &car->Aerial.Section[2].Pos, 49152, &car->AerialPos[3] );
}
*/
#else 

void UpdateCarAerial2(CAR *car, REAL dt)
{
    VEC vecTemp;
    VEC dRPrev;
    REAL scale;
    VEC dRThis;
    REAL thisLen;
    VEC thisCrossPrev;
    REAL crossLen;
    VEC impulse = {ZERO, ZERO, ZERO};

    
    // Don't bother if car not rendered last frame
    if (!car->RenderedAll) {
        SetCarAerialPos(car);
        return;
    }

    // Calculate the world position of the aerial base from the car's
    // world position and world matrix
    // Calculate the aerial's look direction and set the world matrix
    VecMulMat(&car->AerialOffset, &car->Body->Centre.WMatrix, &vecTemp);
    VecPlusVec(&vecTemp, &car->Body->Centre.Pos, &car->Aerial.Section[0].Pos);
    VecMulMat(&car->Aerial.Direction, &car->Body->Centre.WMatrix, &dRPrev);

#ifndef _PSX
    VecEqScalarVec(&car->Aerial.WorldDirection, -car->Aerial.Length, &dRPrev);
#else
    VecEqScalarVec(&car->Aerial.WorldDirection, - 2 * car->Aerial.Length, &dRPrev);
#endif

    // Update the position of the node
    UpdateParticle(&car->Aerial.Section[2], dt);

    // Calculate the length of this section of aerial and
    // the unit vector along this aerial section
    VecMinusVec(&car->Aerial.Section[2].Pos, &car->Aerial.Section[0].Pos, &dRThis);
    thisLen = Length(&dRThis);
    if (thisLen > SMALL_REAL) {
        VecDivScalar(&dRThis, thisLen);
    } else {
        thisLen = ONE;
        CopyVec(&car->Aerial.Direction, &dRThis);
    }
    
    // Move aerial section to keep aerial length constant
    VecPlusEqScalarVec(&car->Aerial.Section[2].Pos, 2 * car->Aerial.Length - thisLen, &dRThis);

    // Calculate force on the end of the section due to the bend of the aerial
    VecCrossVec(&dRThis, &dRPrev, &thisCrossPrev);
    crossLen = VecLen(&thisCrossPrev);
    VecCrossVec(&dRThis, &thisCrossPrev, &vecTemp);
    scale = MulScalar3(2 * car->Aerial.Length, car->Aerial.Stiffness, crossLen);
    VecEqScalarVec(&impulse, -scale, &vecTemp);

    // add the spring damping
    RemoveComponent(&car->Aerial.Section[2].Vel, &dRPrev);
    VecPlusEqScalarVec(&impulse, -car->Aerial.Damping, &car->Aerial.Section[2].Vel);

    // Turn the force into an impulse
    VecMulScalar(&impulse, dt);

    // Apply the impulse to the node from the force
    ApplyParticleImpulse(&car->Aerial.Section[2], &impulse);
}

#endif // _PSX

/////////////////////////////////////////////////////////////////////
//
// CreateCarInfo: allocate space for the required number of
// different car types
//
/////////////////////////////////////////////////////////////////////

CAR_INFO *CreateCarInfo(long nInfo)
{
//$MODIFIED(jedl) - zero init
//    return (CAR_INFO *)malloc(sizeof(CAR_INFO) * nInfo );
    int cb = sizeof(CAR_INFO) * nInfo;
    CAR_INFO* pCarInfo = (CAR_INFO*) malloc(cb);
    ZeroMemory( pCarInfo, cb );
    return pCarInfo;
//$END_MODIFICATIONS
}

void DestroyCarInfo() 
{
    if ((CarInfo == NULL) || (NCarTypes == 0)) return;
    free(CarInfo);
    CarInfo = NULL;
    NCarTypes = 0;
}

/////////////////////////////////////////////////////////////////////
//
// CreateCarModel: allocate space for the required number of
// different car types
//
/////////////////////////////////////////////////////////////////////

CAR_MODEL *CreateCarModels(long nModels)
{
    return (CAR_MODEL *)malloc(sizeof(CAR_MODEL) * nModels);
}

void DestroyCarModels(CAR_MODEL *carModels) 
{
    free(carModels);
}


/////////////////////////////////////////////////////////////////////
//
// ResetCarWheelPos: set initial wheel position, velocity etc;
//
/////////////////////////////////////////////////////////////////////

void ResetCarWheelPos(CAR *car, int iWheel)
{
    WHEEL   *wheel = &car->Wheel[iWheel];

    wheel->Pos = ZERO;
    wheel->Vel = ZERO;
    wheel->Acc = ZERO;
    wheel->Impulse = ZERO;
    wheel->AngPos = ZERO;
    wheel->AngVel = ZERO;
    wheel->AngAcc = ZERO;
    wheel->AngImpulse = ZERO;

    wheel->TurnAngle = ZERO;

#ifndef _PSX
    VecMulMat(&car->WheelOffset[iWheel], &car->Body->Centre.WMatrix, &wheel->WPos);
    
    VecPlusEqVec(&wheel->WPos, &car->Body->Centre.Pos);
    CopyVec(&wheel->WPos, &wheel->OldWPos);
#endif

    VecMulMat(&car->WheelCentre[iWheel], &car->Body->Centre.WMatrix, &wheel->CentrePos);
    //VecPlusEqVec(&wheel->CentrePos, &wheel->WPos);
    
    CopyVec(&wheel->CentrePos, &wheel->OldCentrePos);
    wheel->Skid.Started = FALSE;

    // Build bounding box
#ifndef _PSX
    SetBBox(&wheel->BBox,
        Min(wheel->OldCentrePos.v[X], wheel->CentrePos.v[X]) - wheel->Radius,
        Max(wheel->OldCentrePos.v[X], wheel->CentrePos.v[X]) + wheel->Radius,
        Min(wheel->OldCentrePos.v[Y], wheel->CentrePos.v[Y]) - wheel->Radius,
        Max(wheel->OldCentrePos.v[Y], wheel->CentrePos.v[Y]) + wheel->Radius,
        Min(wheel->OldCentrePos.v[Z], wheel->CentrePos.v[Z]) - wheel->Radius,
        Max(wheel->OldCentrePos.v[Z], wheel->CentrePos.v[Z]) + wheel->Radius);
#endif

    CopyMat(&car->Body->Centre.WMatrix, &wheel->Axes);
#ifndef _PSX
    CopyMat(&car->Body->Centre.WMatrix, &wheel->WMatrix);
#endif

}

/////////////////////////////////////////////////////////////////////
//
// UpdateWheel: update the position of the wheel according to its
// impulses
//
/////////////////////////////////////////////////////////////////////

#ifndef _PSX

void UpdateCarWheel(CAR *car, int iWheel, REAL dt)
{
    REAL scale;
    VEC tmpVec;
    VEC dR, centAcc;
    MAT tmpMat;
    WHEEL  *wheel = &car->Wheel[iWheel];
    SPRING *spring = &car->Spring[iWheel];


    // Apply torque from engine
    if ((IsWheelPowered(wheel) && !IsWheelInContact(wheel)) || (IsWheelSpinning(wheel) && IsWheelInContact(wheel))) {
        //wheel->AngImpulse += MulScalar(dt, MulScalar(car->EngineVolt, wheel->EngineRatio));
        scale = MulScalar3(car->EngineVolt, wheel->SpinAngImp, dt);
        if (wheel->EngineRatio < ZERO) {
            scale = -scale;
        }
        wheel->AngImpulse += scale;
    }

    // Get accelerations
    wheel->Acc = MulScalar(wheel->InvMass, wheel->Impulse);
    wheel->AngAcc = MulScalar(wheel->InvInertia, wheel->AngImpulse);

#ifdef _PC
    if (CAR_WheelsHaveSuspension) {

        // Remove centripetal acceleration from the spin of the car
        // and up component of linear acceleration
        VecMinusVec(&wheel->WPos, &car->Body->Centre.Pos, &dR);
        VecCrossVec(&dR, &car->Body->AngVel, &tmpVec);
        VecCrossVec(&car->Body->AngVel, &tmpVec, &centAcc);
        wheel->Acc += MulScalar(dt, VecDotVec(&centAcc, &car->Body->Centre.WMatrix.mv[U]));
        wheel->Acc -= VecDotVec(&car->Body->Centre.Acc, &car->Body->Centre.WMatrix.mv[U]);
    }
#endif

    // Get new velocities
    wheel->Vel += wheel->Acc;
    wheel->AngVel += wheel->AngAcc;

    // Add damping
    scale = ONE - MulScalar3(FRICTION_TIME_SCALE, dt, wheel->AxleFriction);
    wheel->AngVel = MulScalar(wheel->AngVel, scale);

    // Get new positions
    wheel->Pos += MulScalar(wheel->Vel, dt);
    Limit(wheel->Pos, -wheel->MaxPos, wheel->MaxPos);
    wheel->AngPos += MulScalar(wheel->AngVel, dt);
    GoodWrap(&wheel->AngPos, ZERO, FULL_CIRCLE);

    // Add spring forces
    if (!IsWheelInContact(wheel)) {
        scale = MulScalar(dt, SpringDampedForce(spring, wheel->Pos, wheel->Vel));
        wheel->Vel += MulScalar(wheel->InvMass, scale);
    } else {
        scale = MulScalar(dt, SpringDampedForce(spring, wheel->Pos, wheel->Vel));
        wheel->Vel += MulScalar(wheel->InvMass, scale) / 2;
    }

    // Calculate the wheel's world matrix and position
/*  CopyVec(&wheel->WPos, &wheel->OldWPos);

    SetVec(&tmpVec, car->WheelOffset[iWheel].v[X], car->WheelOffset[iWheel].v[Y] + wheel->Pos, car->WheelOffset[iWheel].v[Z]);

    VecMulMat(&tmpVec, &car->Body->Centre.WMatrix, &wheel->WPos);
    VecPlusEqVec(&wheel->WPos, &car->Body->Centre.Pos);

    CopyVec(&wheel->CentrePos, &wheel->OldCentrePos);
    VecMulMat(&car->WheelCentre[iWheel], &car->Body->Centre.WMatrix, &tmpVec);

    VecPlusVec(&wheel->WPos, &tmpVec, &wheel->CentrePos);
*/

    CopyVec(&wheel->WPos, &wheel->OldWPos);
    CopyVec(&wheel->CentrePos, &wheel->OldCentrePos);

    SetVec(&tmpVec, car->WheelCentre[iWheel].v[X], car->WheelCentre[iWheel].v[Y] + wheel->Pos, car->WheelCentre[iWheel].v[Z]);
    VecMulMat(&tmpVec, &car->Body->Centre.WMatrix, &wheel->CentrePos);
    VecPlusEqVec(&wheel->CentrePos, &car->Body->Centre.Pos);

    SetVec(&tmpVec, car->WheelOffset[iWheel].v[X], car->WheelOffset[iWheel].v[Y] + wheel->Pos, car->WheelOffset[iWheel].v[Z]);
    VecMulMat(&tmpVec, &car->Body->Centre.WMatrix, &wheel->WPos);
    VecPlusEqVec(&wheel->WPos, &car->Body->Centre.Pos);

    if (iWheel & 1) {
        CopyMat(&car->Wheel[iWheel - 1].Axes, &wheel->Axes);
        CopyMat(&car->Wheel[iWheel - 1].WMatrix, &wheel->WMatrix);
    } else {
        if (IsWheelTurnable(wheel)) {
            wheel->TurnAngle = MulScalar(car->SteerAngle, wheel->SteerRatio);
            RotationY(&tmpMat, wheel->TurnAngle);
            MatMulMat(&tmpMat, &car->Body->Centre.WMatrix, &wheel->Axes);
        } else {
            CopyMat(&car->Body->Centre.WMatrix, &wheel->Axes);
        }
#ifdef _PC
        RotationX(&tmpMat, wheel->AngPos);
        MatMulMat(&tmpMat, &wheel->Axes, &wheel->WMatrix);
#endif
    }

    // Build bounding box
    /*SetBBox(&wheel->BBox,
        Min(wheel->OldCentrePos.v[X], wheel->CentrePos.v[X]) - wheel->Radius,
        Max(wheel->OldCentrePos.v[X], wheel->CentrePos.v[X]) + wheel->Radius,
        Min(wheel->OldCentrePos.v[Y], wheel->CentrePos.v[Y]) - wheel->Radius,
        Max(wheel->OldCentrePos.v[Y], wheel->CentrePos.v[Y]) + wheel->Radius,
        Min(wheel->OldCentrePos.v[Z], wheel->CentrePos.v[Z]) - wheel->Radius,
        Max(wheel->OldCentrePos.v[Z], wheel->CentrePos.v[Z]) + wheel->Radius);*/
    if (wheel->OldCentrePos.v[X] < wheel->CentrePos.v[X]) {
        wheel->BBox.XMin = wheel->OldCentrePos.v[X] - wheel->Radius;
        wheel->BBox.XMax = wheel->CentrePos.v[X] + wheel->Radius;
    } else {
        wheel->BBox.XMin = wheel->CentrePos.v[X] - wheel->Radius;
        wheel->BBox.XMax = wheel->OldCentrePos.v[X] + wheel->Radius;
    }
    if (wheel->OldCentrePos.v[Y] < wheel->CentrePos.v[Y]) {
        wheel->BBox.YMin = wheel->OldCentrePos.v[Y] - wheel->Radius;
        wheel->BBox.YMax = wheel->CentrePos.v[Y] + wheel->Radius;
    } else {
        wheel->BBox.YMin = wheel->CentrePos.v[Y] - wheel->Radius;
        wheel->BBox.YMax = wheel->OldCentrePos.v[Y] + wheel->Radius;
    }
    if (wheel->OldCentrePos.v[Z] < wheel->CentrePos.v[Z]) {
        wheel->BBox.ZMin = wheel->OldCentrePos.v[Z] - wheel->Radius;
        wheel->BBox.ZMax = wheel->CentrePos.v[Z] + wheel->Radius;
    } else {
        wheel->BBox.ZMin = wheel->CentrePos.v[Z] - wheel->Radius;
        wheel->BBox.ZMax = wheel->OldCentrePos.v[Z] + wheel->Radius;
    }

    // Zero impulses
    wheel->Impulse = ZERO;
    wheel->AngImpulse = ZERO;

}



#else  //.............  PSX Version ................


void UpdateCarWheel(CAR *car, int iWheel, REAL dt)
{
    VEC tmpVec;
    VEC dR, centAcc;
    MAT tmpMat;
    WHEEL  *wheel = &car->Wheel[iWheel];
    SPRING *spring = &car->Spring[iWheel];


    // Apply torque from engine
    if ((IsWheelPowered(wheel) && !IsWheelInContact(wheel)) || (IsWheelSpinning(wheel) && IsWheelInContact(wheel))) 
        wheel->AngImpulse += MulScalar3(car->EngineVolt, wheel->SpinAngImp, dt);
    

    // Get accelerations
    wheel->Acc = MulScalar(wheel->InvMass, wheel->Impulse);
    wheel->AngAcc = MulScalar(wheel->InvInertia, wheel->AngImpulse);

    // Get new velocities
    wheel->Vel += wheel->Acc;
    wheel->AngVel += wheel->AngAcc;

    // Add damping

    wheel->AngVel = MulScalar(wheel->AngVel, ONE - MulScalar3(FRICTION_TIME_SCALE, dt, wheel->AxleFriction) );

    // Get new positions
    wheel->Pos += MulScalar(wheel->Vel, dt);
    Limit(wheel->Pos, -wheel->MaxPos, wheel->MaxPos);
    wheel->AngPos += MulScalar(wheel->AngVel, dt);
    GoodWrap(&wheel->AngPos, ZERO, FULL_CIRCLE);

    // Add spring forces
    if (!IsWheelInContact(wheel)) 
        wheel->Vel += MulScalar(wheel->InvMass, MulScalar(dt, SpringDampedForce(spring, wheel->Pos, wheel->Vel)));
    else 
        wheel->Vel += MulScalar(wheel->InvMass, MulScalar(dt, SpringDampedForce(spring, wheel->Pos, wheel->Vel)) ) / 2;
    

    // Calculate the wheel's world matrix and position
#ifndef _PSX
    CopyVec(&wheel->WPos, &wheel->OldWPos);
#endif
    CopyVec(&wheel->CentrePos, &wheel->OldCentrePos);

    SetVec(&tmpVec, car->WheelCentre[iWheel].v[X], car->WheelCentre[iWheel].v[Y] + wheel->Pos, car->WheelCentre[iWheel].v[Z]);
    VecMulMat(&tmpVec, &car->Body->Centre.WMatrix, &wheel->CentrePos);
    VecPlusEqVec(&wheel->CentrePos, &car->Body->Centre.Pos);

    if (iWheel & 1)
    {
        CopyMat(&car->Wheel[iWheel - 1].Axes, &wheel->Axes);
#ifndef _PSX
        CopyMat(&car->Wheel[iWheel - 1].WMatrix, &wheel->WMatrix);
#endif
    }
    else
    {
        if (IsWheelTurnable(wheel)) 
        {
            wheel->TurnAngle = MulScalar(car->SteerAngle, wheel->SteerRatio);
            if (wheel->TurnAngle != 0) 
            {
                RotationY(&tmpMat, wheel->TurnAngle);
                MatMulMat(&tmpMat, &car->Body->Centre.WMatrix, &wheel->Axes);
            } 
            else 
            {
                CopyMat(&car->Body->Centre.WMatrix, &wheel->Axes);
            }
        }
        else
        {
            CopyMat(&car->Body->Centre.WMatrix, &wheel->Axes);
        }
        
    }

    // Build bounding box
#if 0
    if (wheel->OldCentrePos.v[X] < wheel->CentrePos.v[X]) {
        wheel->BBox.XMin = wheel->OldCentrePos.v[X] - wheel->Radius;
        wheel->BBox.XMax = wheel->CentrePos.v[X] + wheel->Radius;
    } else {
        wheel->BBox.XMin = wheel->CentrePos.v[X] - wheel->Radius;
        wheel->BBox.XMax = wheel->OldCentrePos.v[X] + wheel->Radius;
    }
    if (wheel->OldCentrePos.v[Y] < wheel->CentrePos.v[Y]) {
        wheel->BBox.YMin = wheel->OldCentrePos.v[Y] - wheel->Radius;
        wheel->BBox.YMax = wheel->CentrePos.v[Y] + wheel->Radius;
    } else {
        wheel->BBox.YMin = wheel->CentrePos.v[Y] - wheel->Radius;
        wheel->BBox.YMax = wheel->OldCentrePos.v[Y] + wheel->Radius;
    }
    if (wheel->OldCentrePos.v[Z] < wheel->CentrePos.v[Z]) {
        wheel->BBox.ZMin = wheel->OldCentrePos.v[Z] - wheel->Radius;
        wheel->BBox.ZMax = wheel->CentrePos.v[Z] + wheel->Radius;
    } else {
        wheel->BBox.ZMin = wheel->CentrePos.v[Z] - wheel->Radius;
        wheel->BBox.ZMax = wheel->OldCentrePos.v[Z] + wheel->Radius;
    }
#endif

    // Zero impulses
    wheel->Impulse = ZERO;
    wheel->AngImpulse = ZERO;

}


#endif



/////////////////////////////////////////////////////////////////////
//
// PreProcessCarWheelColls: remove duplicate collisions
//
/////////////////////////////////////////////////////////////////////

void PreProcessCarWheelColls(CAR *car)
{
    bool    keepGoing, removeColl;
    int     iOil;
    REAL    dx, dz;
    register WHEEL  *wheel;
    register COLLINFO_WHEEL *wheelColl1, *wheelColl2;
    VEC     *collpos;

    int     iWheel;
    REAL    shiftDotUp;
    VEC     worldShift[4] = {0,};
    VEC     bodyShift[4] = {0,};

    for (wheelColl1 = car->WheelCollHead; wheelColl1 != NULL; wheelColl1 = wheelColl1->Next) {

        wheel = &car->Wheel[wheelColl1->IWheel];


#ifndef _PSX
        // check for oil slick contact
        collpos = &wheelColl1->WorldPos;

        for (iOil = 0 ; iOil < OilSlickCount ; iOil++)
        {
            if (collpos->v[Y] < OilSlickList[iOil].Ymin)
                continue;

            if (collpos->v[Y] > OilSlickList[iOil].Ymax)
                continue;

            dx = OilSlickList[iOil].X - collpos->v[X];
            if (abs(dx) > OilSlickList[iOil].Radius)
                continue;

            dz = OilSlickList[iOil].Z - collpos->v[Z];
            if (abs(dz) > OilSlickList[iOil].Radius)
                continue;

            if ((dx * dx + dz * dz) > OilSlickList[iOil].SquaredRadius)
                continue;

            SetWheelInOil(&wheelColl1->Car->Wheel[wheelColl1->IWheel]);
            break;
        }

        // Reduce friction if wheel is oiled up
        if (wheel->OilTime < OILY_WHEEL_TIME) {
            wheelColl1->StaticFriction *= HALF * (wheel->OilTime + TO_TIME(Real(0.2))) / (OILY_WHEEL_TIME + TO_TIME(Real(0.2)));
            wheelColl1->KineticFriction *= HALF * (wheel->OilTime + TO_TIME(Real(0.2))) / (OILY_WHEEL_TIME + TO_TIME(Real(0.2)));
        }

#else   // If it's _PSX then do this....

        // check for oil slick contact
        collpos = &wheelColl1->WorldPos;

        for (iOil = 0 ; iOil < OilSlickCount ; iOil++)
        {
            if (collpos->v[Y] < OilSlickList[iOil].Ymin)
            {
                continue;
            }

            if (collpos->v[Y] > OilSlickList[iOil].Ymax)
            {
                continue;
            }


            dx = OilSlickList[iOil].X - collpos->v[X];
            if (abs(dx) > OilSlickList[iOil].Radius)
                continue;

            dz = OilSlickList[iOil].Z - collpos->v[Z];
            if (abs(dz) > OilSlickList[iOil].Radius)
                continue;

            dx = PSX_LENGTH( dx );
            dz = PSX_LENGTH( dz );
            if ((dx * dx + dz * dz) > OilSlickList[iOil].SquaredRadius)
                continue;

            SetWheelInOil(&wheelColl1->Car->Wheel[wheelColl1->IWheel]);
            break;
        }

        // Reduce friction if wheel is oiled up
        if( wheel->OilTime < OILY_WHEEL_TIME )
        {
            wheelColl1->StaticFriction = MulScalar( wheelColl1->StaticFriction, DivScalar( MulScalar( HALF, (wheel->OilTime + TO_TIME(Real(0.2))) ), (OILY_WHEEL_TIME + TO_TIME(Real(0.2))) ) );
            wheelColl1->KineticFriction = MulScalar( wheelColl1->KineticFriction, DivScalar( MulScalar( HALF, (wheel->OilTime + TO_TIME(Real(0.2))) ), (OILY_WHEEL_TIME + TO_TIME(Real(0.2))) ) );
        }
#endif

        // Merge close collisions with similar planes
        keepGoing = TRUE;
        for (wheelColl2 = wheelColl1->Next; (wheelColl2 != NULL) && keepGoing; wheelColl2 = wheelColl2->Next) {

            if (wheelColl1->IWheel != wheelColl2->IWheel) continue;

            removeColl = FALSE;
            if ((wheelColl1->CollPoly != NULL) && (wheelColl2->CollPoly != NULL))
            {
                if (abs(wheelColl1->Pos.v[Y] - wheelColl2->Pos.v[Y]) < TO_LENGTH(Real(3)))
                {
                    removeColl = TRUE;
                }
                else if (abs(wheelColl1->WorldPos.v[Y] - wheelColl2->WorldPos.v[Y]) < TO_LENGTH(Real(3)))
                {
                    removeColl = TRUE;
                }
            }

            if (removeColl) {
                if (wheelColl1->Depth > wheelColl2->Depth) {
                    RemoveWheelColl(car, wheelColl1);
                    if (wheelColl2->CollPoly != NULL) {
                        CopyPlane(&wheelColl2->CollPoly->Plane, &wheelColl2->Plane);
                    }
                    keepGoing = FALSE;
                } else {
                    RemoveWheelColl(car, wheelColl2);
                    if (wheelColl1->CollPoly != NULL) {
                        CopyPlane(&wheelColl1->CollPoly->Plane, &wheelColl1->Plane);
                    }
                }
            }
        }

        // Keep track of the shift to get the car out of the world and other bodies
        if (keepGoing) {
            // Only if this collision was not removed
            if (wheelColl1->Depth < ZERO) {
                if (wheelColl1->CollPoly != NULL) {
                    // Collision is with the world
                    //ModifyShift(&worldShift[wheelColl1->IWheel], -wheelColl1->Depth, PlaneNormal(&wheelColl1->Plane));
                    VecPlusEqScalarVec(&worldShift[wheelColl1->IWheel], -wheelColl1->Depth, PlaneNormal(&wheelColl1->Plane));
                } else {
                    // Collision is with another object
                    ModifyShift(&bodyShift[wheelColl1->IWheel], -wheelColl1->Depth / 2, PlaneNormal(&wheelColl1->Plane));
                    SetWheelInOtherContact(wheel);
                }
            }
        }
    }

    // Shift the car out contact
    if (!car->Body->Stacked) {
        for (iWheel = 0; iWheel < CAR_NWHEELS; iWheel++) {
            wheel = &car->Wheel[iWheel];

            // Iron out inconsistencies in the shifts
            
            if (worldShift[iWheel].v[X] != ZERO) {
                if (Sign(worldShift[iWheel].v[X]) == Sign(bodyShift[iWheel].v[X])) {
                    if (Sign(worldShift[iWheel].v[X]) > ZERO) {
                        worldShift[iWheel].v[X] = Max(worldShift[iWheel].v[X], bodyShift[iWheel].v[X]);
                    } else {
                        worldShift[iWheel].v[X] = Min(worldShift[iWheel].v[X], bodyShift[iWheel].v[X]);
                    }
                }
            } else {
                worldShift[iWheel].v[X] = bodyShift[iWheel].v[X];
            }

            if (worldShift[iWheel].v[Y] != ZERO) {
                if (Sign(worldShift[iWheel].v[Y]) == Sign(bodyShift[iWheel].v[Y])) {
                    if (Sign(worldShift[iWheel].v[Y]) > ZERO) {
                        worldShift[iWheel].v[Y] = Max(worldShift[iWheel].v[Y], bodyShift[iWheel].v[Y]);
                    } else {
                        worldShift[iWheel].v[Y] = Min(worldShift[iWheel].v[Y], bodyShift[iWheel].v[Y]);
                    }
                }
            } else {
                worldShift[iWheel].v[Y] = bodyShift[iWheel].v[Y];
            }

            if (worldShift[iWheel].v[Z] != ZERO) {
                if (Sign(worldShift[iWheel].v[Z]) == Sign(bodyShift[iWheel].v[Z])) {
                    if (Sign(worldShift[iWheel].v[Z]) > ZERO) {
                        worldShift[iWheel].v[Z] = Max(worldShift[iWheel].v[Z], bodyShift[iWheel].v[Z]);
                    } else {
                        worldShift[iWheel].v[Z] = Min(worldShift[iWheel].v[Z], bodyShift[iWheel].v[Z]);
                    }
                }
            } else {
                worldShift[iWheel].v[Z] = bodyShift[iWheel].v[Z];
            }

            // Move the wheel
            shiftDotUp = VecDotVec(&worldShift[iWheel], &car->Body->Centre.WMatrix.mv[U]);
            wheel->Pos += shiftDotUp;
            if (wheel->Pos > wheel->MaxPos) {
                shiftDotUp -= wheel->Pos - wheel->MaxPos;
                wheel->Pos = wheel->MaxPos;
            } else if (wheel->Pos < -wheel->MaxPos) {
                shiftDotUp -= wheel->Pos + wheel->MaxPos;
                wheel->Pos = -wheel->MaxPos;
            }

            // Move the car body
            VecMinusEqScalarVec(&worldShift[iWheel], shiftDotUp, &car->Body->Centre.WMatrix.mv[U]);
            ModifyShift(&car->Body->Centre.Shift, ONE, &worldShift[iWheel]);

        }
    }
}


/////////////////////////////////////////////////////////////////////
//
// ProcessCarWheelColls: calculate net impulse from each collision
// on the passed wheel on the car and add it to the car
//
// Calculate collision impulse as if it were a perfect-friction
// collision, then adjust the components along the look, right and
// up directions (which should be in the wheel's "TurnMatrix").
//
/////////////////////////////////////////////////////////////////////

void ProcessCarWheelColls(CAR *car)
{
    COLLINFO_WHEEL  *wheelColl;
    WHEEL   *wheel;
    REAL    scale;
    VEC collImpulse, tmpVec;
    VEC totImpulse = {0, 0, 0};
    VEC totAngImpulse = {0, 0, 0};

    for (wheelColl = car->WheelCollHead; wheelColl != NULL; wheelColl = wheelColl->Next) {

        wheel = &car->Wheel[wheelColl->IWheel];
        
        // Velocity and UP vector along collision normal (used in CarWheelImpulse2 and PostProcessCarWheelColls)
        wheelColl->VelDotNorm = VecDotVec(&wheelColl->Vel, PlaneNormal(&wheelColl->Plane));
        wheelColl->UpDotNorm = VecDotVec(PlaneNormal(&wheelColl->Plane), &car->Body->Centre.WMatrix.mv[U]);

        // Calculate the shift required to extract the wheel from the road
        if (wheelColl->Depth < ZERO) {

            // Stop wheel relative to road
            wheel->Vel = -MulScalar(wheelColl->UpDotNorm, wheelColl->VelDotNorm);
            scale = MulScalar(wheel->Gravity, TimeStep);
            wheel->Vel += MulScalar(scale, car->Body->Centre.WMatrix.m[UY]);

        }

        // Calculate the impulse to apply to the car from the wheel
        CarWheelImpulse2(car, wheelColl, &collImpulse);

        // Calculate and store the linear and angular components of the impulse
        VecPlusEqVec(&totImpulse, &collImpulse);

        if ((GameSettings.PlayMode < PLAYMODE_CONSOLE) || (abs(wheelColl->Plane.v[B]) > Real(0.3))) {
            CalcAngImpulse(&collImpulse, &wheelColl->Pos, &tmpVec);
            VecPlusEqVec(&totAngImpulse, &tmpVec);
        }
    }
    
    // Apply impulse from wheels to car body
    ApplyBodyAngImpulse(car->Body, &totAngImpulse);
    ApplyParticleImpulse(&car->Body->Centre, &totImpulse);

}


////////////////////////////////////////////////////////////////
//
// CarWheelImpulse:
//
////////////////////////////////////////////////////////////////

#if defined(_PC) || defined(_N64)

#define SKID_MIN_VEL 100.0f

void CarWheelImpulse2(CAR *car, COLLINFO_WHEEL *collInfo, VEC *impulse)
{
    REAL    hardness;
    REAL    dVelNorm, impUp, scale, impDotNorm, knock;
    REAL    impTanLen, angVel, springImp;
    REAL    lookLen, impDotLook, torque;
    REAL    fricMod, maxTorque, tReal, timeScale;
    VEC     tmpVec;
    VEC     velTan, lookVec;
    VEC     sparkVel;

    bool doSpark = FALSE;
    VEC  impNorm = {ZERO, ZERO, ZERO};
    VEC impTan = {ZERO, ZERO, ZERO};
    WHEEL   *wheel = &car->Wheel[collInfo->IWheel];
    NEWBODY *body = car->Body;

    timeScale = MulScalar(FRICTION_TIME_SCALE, TimeStep);

    // Calculate forward vector
    VecCrossVec(PlaneNormal(&collInfo->Plane), &wheel->Axes.mv[R], &lookVec);
    lookLen = VecLen(&lookVec);

    hardness = collInfo->Restitution;

    // Scale the friction if the sides of the wheels are the contact point
    if (lookLen > Real(0.7)) {
        // Wheel in driving contact
        VecDivScalar(&lookVec, lookLen);
        fricMod = ONE;
        if (abs(collInfo->Plane.v[B]) > HALF) {
            SetWheelInFloorContact(wheel);
        } else {
            SetWheelInWallContact(wheel);
        }
    } else {
        // Wheel in Side contact
        doSpark = TRUE;
        if (abs(collInfo->Plane.v[B]) > HALF) {
            fricMod = ONE;
            SetWheelInFloorContact(wheel);
            SetWheelInSideContact(wheel);
        } else {
            fricMod = Real(0.1) + lookLen / 4;
            SetWheelInWallContact(wheel);
            SetWheelInSideContact(wheel);
        }
    }

    // Calculate the change in normal velocity required for the collision
    dVelNorm = MulScalar(collInfo->VelDotNorm, -(ONE + hardness));

    if (collInfo->UpDotNorm < Real(0.9)) {

        // Calculate normal (zero-friction) impulse required to get this change in velocity
        impDotNorm = OneBodyZeroFrictionImpulse(body, &collInfo->Pos, PlaneNormal(&collInfo->Plane), dVelNorm);
    
        // Dampers
        impUp = MulScalar(impDotNorm, collInfo->UpDotNorm);
        impUp = MulScalar(impUp, collInfo->UpDotNorm);
        scale = MulScalar((car->Spring[collInfo->IWheel].Restitution), impUp);
        impDotNorm += scale;

    } else {

        impDotNorm = ZERO;
    }

    // Springs
    VecMinusVec(&wheel->CentrePos, &car->Body->Centre.Pos, &tmpVec);
    VecMinusEqVec(&tmpVec, &collInfo->Pos);
    if (Sign(wheel->Pos) == Sign(VecDotVec(&tmpVec, &body->Centre.WMatrix.mv[U]))) {
        springImp = MulScalar(TimeStep, SpringDampedForce(&car->Spring[collInfo->IWheel], wheel->Pos, wheel->Vel));
        springImp = MulScalar(springImp, collInfo->UpDotNorm);
        impDotNorm -= springImp;
    }
    else
    {
        springImp = 0;
    }

    if (impDotNorm < 0)
    {
        SetVecZero(impulse);
        return;
    }

    VecEqScalarVec(&impNorm, impDotNorm, PlaneNormal(&collInfo->Plane));

    // Flag a hard knock if necessary
    if (collInfo->Material != &COL_MaterialInfo[MATERIAL_BOUNDARY]) {
        knock = MulScalar(abs(impDotNorm), car->Body->Centre.InvMass);
        if (knock > car->Body->BangMag) {
            car->Body->BangMag = knock;
            CopyPlane(&collInfo->Plane, &car->Body->BangPlane);
        }
    } else {
            car->Body->BangMag = ZERO;
    }

    // Calculate sliding velocity
    VecPlusScalarVec(&collInfo->Vel, -collInfo->VelDotNorm, PlaneNormal(&collInfo->Plane), &velTan);

    // Remove component along roll direction of wheel
    impDotLook = VecDotVec(&lookVec, &velTan);
    VecPlusEqScalarVec(&velTan, -impDotLook, &lookVec);
    collInfo->SlideVel = Length(&velTan);

    // Turn sliding velocities into fricitonal impulse
    impTanLen = MulScalar3(collInfo->Grip, abs(springImp), timeScale);
    impTanLen = MulScalar(impTanLen, Real(-0.35f));
    VecEqScalarVec(&impTan, impTanLen, &velTan);
    impTanLen = MulScalar(impTanLen, collInfo->SlideVel);

    // Add wheel torque
    if (IsWheelPowered(wheel)) {
        torque = MulScalar(TimeStep, MulScalar(car->EngineVolt, wheel->EngineRatio));
        //tReal = Sign(torque) * MulScalar(torque, DivScalar(MulScalar(wheel->AngVel, wheel->Radius), car->TopSpeed));
        tReal = MulScalar(abs(torque), DivScalar(MulScalar(wheel->AngVel, wheel->Radius), car->TopSpeed));
        /*if (car->Body->Centre.WMatrix.m[UY] > ZERO) {
            tReal = MulScalar(abs(torque), DivScalar(MulScalar(abs(wheel->AngVel), wheel->Radius), car->TopSpeed));
        } else {
            tReal = MulScalar(abs(torque), DivScalar(MulScalar(abs(wheel->AngVel), wheel->Radius), car->TopSpeed));
        }
        if (torque > ZERO) {
            tReal = torque - tReal;
        } else {
            tReal = -torque + tReal;
        }*/
        tReal = torque - tReal;
        if (Sign(torque) != Sign(tReal)) {
            torque = ZERO;
        } else {
            torque = tReal;
        }
    } else {
        torque = ZERO;
    }

    // Apply axle friction
    if (IsWheelLocked(wheel) || (abs(car->EngineVolt) < Real(0.01)) || (Sign(car->EngineVolt) == -Sign(wheel->AngVel))) {
        tReal = MulScalar(wheel->AxleFriction, MulScalar(wheel->AngVel, wheel->Radius));
        tReal = MulScalar(tReal, timeScale);
        if (IsWheelLocked(wheel)) tReal *= 3;
        torque -= tReal;
    }

    // Check Engine Torque for wheelspin
    maxTorque = MulScalar(MulScalar(wheel->Radius, fricMod), MulScalar(collInfo->StaticFriction, impDotNorm));
    if (abs(torque) > abs(maxTorque)) {
        SetWheelSpinning(wheel);
    }

    VecPlusEqScalarVec(&impTan, DivScalar(torque, wheel->Radius), &lookVec);

    // Scale sliding to friction cone
    impTanLen = VecLen(&impTan);

    //$BUGBUG(JHarding, 1/24/02): This divide-by-zero is really bad and will hose
    // the physics model.
    //$REVISIT: does this problem still exist in the new physics code from Acclaim?
    if( impTanLen == 0.0f )
    {
        static bool bWarned = FALSE;
        if( !bWarned )
        {
            bWarned = TRUE;
            OutputDebugString( "Protecting against divide-by-zero\n" );
        }

        impTanLen = 1.0f;
    }

#ifdef _PC
    //maxTorque = 2 * MulScalar(MulScalar(TimeStep, car->Body->Centre.Mass), car->Body->Centre.Gravity);
    maxTorque = 4 * MulScalar(MulScalar(TimeStep, car->Body->Centre.Mass), car->Body->Centre.Gravity);
#else
    maxTorque = MulScalar(MulScalar(TimeStep, car->Body->Centre.Mass), car->Body->Centre.Gravity);
#endif
    if (impTanLen > maxTorque) {
        VecMulScalar(&impTan, DivScalar(maxTorque, impTanLen));
        impTanLen = maxTorque;
        SetWheelSliding(wheel);
    }
    maxTorque = MulScalar(MulScalar(collInfo->StaticFriction, fricMod), impDotNorm);
    if (impTanLen > maxTorque) {
        maxTorque = MulScalar(MulScalar(collInfo->KineticFriction, fricMod), impDotNorm);
        VecMulScalar(&impTan, DivScalar(maxTorque, impTanLen));
        impTanLen = maxTorque;
        SetWheelSliding(wheel);
    }

    // Reduce friction if floor contact and only one wheel on floor
//  if ((car->NWheelColls < 3) && (lookLen > Real(0.7))) {
//      VecMulScalar(&impTan, Real(0.0));
//  }

    // Set wheel's spin
    if (!IsWheelSpinning(wheel)) {
        angVel = DivScalar(VecDotVec(&collInfo->Vel, &lookVec), wheel->Radius);
        wheel->AngVel += MulScalar((angVel - wheel->AngVel),  DivScalar(MulScalar(FRICTION_TIME_SCALE, TimeStep), 4));
    }

    // Generate spark if side of wheel scraping
#ifndef _PSX
    if (car->RenderedAll && doSpark &&  (collInfo->Material != NULL) && (collInfo->SlideVel > MIN_SPARK_VEL))
    {
        body->ScrapeMaterial = collInfo->Material - COL_MaterialInfo;
        body->LastScrapeTime = ZERO;
        if ((frand(2.0f) < SparkProbability(collInfo->SlideVel)) && MaterialAllowsSparks(collInfo->Material))
        {
            VecEqScalarVec(&sparkVel, HALF, &velTan);
            CreateSpark(SPARK_SPARK, &collInfo->WorldPos, &sparkVel, collInfo->SlideVel / 3, 0);
        }
    }
#endif

    // Spray dust if the material is dusty
#ifdef _PC
    if (car->RenderedAll && collInfo->Material != NULL && MaterialDusty(collInfo->Material))
    {
        enum SparkTypeEnum sparkType;

        // Create dust spark (stones, grass etc...)
        if ((impTanLen > (5 * body->Centre.InvMass * MIN_DUST_IMPULSE)) &&
            (COL_DustInfo[collInfo->Material->DustType].SparkType != DUST_NONE) &&
            (frand(1.0f) < (gSparkDensity * COL_DustInfo[collInfo->Material->DustType].SparkProbability)))
        {
            sparkType = (enum SparkTypeEnum)COL_DustInfo[collInfo->Material->DustType].SparkType;
            scale = body->Centre.InvMass * DUST_SCALE;
            VecEqScalarVec(&sparkVel, -scale, &impTan);
            sparkVel.v[Y] -= HALF * scale * impTanLen;
            VecPlusEqScalarVec(&sparkVel, HALF * body->Centre.InvMass * torque, &lookVec);
            CreateSpark(sparkType, &collInfo->WorldPos, &sparkVel, COL_DustInfo[collInfo->Material->DustType].SparkVar * scale * impTanLen, 0);
        }
        // Create smoke spark
        if ((impTanLen > (2 * body->Centre.InvMass * MIN_DUST_IMPULSE)) &&
            (COL_DustInfo[collInfo->Material->DustType].SmokeType != DUST_NONE) &&
            (frand(1.0f) < (gSparkDensity * COL_DustInfo[collInfo->Material->DustType].SmokeProbability)))
        {
            SPARK* pSpark;
            sparkType = (enum SparkTypeEnum)COL_DustInfo[collInfo->Material->DustType].SmokeType;
            scale = body->Centre.InvMass * Real(10);
            VecEqScalarVec(&sparkVel, -scale, &impTan);
            sparkVel.v[Y] -= HALF * scale * impTanLen;
            VecPlusEqScalarVec(&sparkVel, HALF * body->Centre.InvMass * torque, &lookVec);
            if (pSpark = CreateSpark(sparkType, &collInfo->WorldPos, &sparkVel, COL_DustInfo[collInfo->Material->DustType].SmokeVar * scale * impTanLen, 0))
            {
                pSpark->Grow += wheel->SkidWidth * (ONE/2);
            }
        }
    }
#endif

    // Sum the impulses
    VecPlusVec(&impNorm, &impTan, impulse);

    // Zero the small components
    if (abs(impulse->v[X]) < SMALL_IMPULSE_COMPONENT) impulse->v[X] = ZERO;
    if (abs(impulse->v[Y]) < SMALL_IMPULSE_COMPONENT) impulse->v[Y] = ZERO;
    if (abs(impulse->v[Z]) < SMALL_IMPULSE_COMPONENT) impulse->v[Z] = ZERO;


}

#else //////////////_PSX/////////////////

#define VecCrossVecUnit VecCrossVec
#define VecLenUnit VecLen

void CarWheelImpulse2(CAR *car, COLLINFO_WHEEL *collInfo, VEC *impulse)
{
    REAL    lookLen, velDotLook, netFriction, maxSq, scale;
    REAL    impDotNorm, impDotNormTrue, impDotLook, impSlide;
    REAL    impTanLen, max, velSlideLen;
    VEC     lookVec, velSideways;

    VEC     impTan;// = {0, 0, 0};
    register WHEEL  *wheel = &car->Wheel[collInfo->IWheel];
    NEWBODY *body = car->Body;

    // Calculate forward vector
    VecCrossVecUnit(PlaneNormal(&collInfo->Plane), &wheel->Axes.mv[R], &lookVec);
    lookLen = VecLenUnit(&lookVec);
    
    // Check for side of wheel contact
    if (lookLen < Real(0.7))
    {
        SetWheelInSideContact(wheel);
        // this next bit added by Greg...
        if( (collInfo->Material != NULL) && (collInfo->SlideVel > TO_VEL(Real(100)) ) )
        {
            body->ScrapeMaterial = collInfo->Material - COL_MaterialInfo;
            body->LastScrapeTime = ZERO;
        }
    }

    if (abs(collInfo->Plane.v[B]) < Real(0.5)) {
        // scale friction if vertical(ish) wall
        lookLen >>= 3;
        SetWheelInWallContact(wheel);
    } else {
        // Normal floor contact
        SetWheelInFloorContact(wheel);
    }

    // Useful components of vectors  (The next few lines could be combined in asm to get rid of about 12 reads...)
    velDotLook = VecDotVec(&collInfo->Vel, &lookVec);

#if 1
    // Sliding velocity
    VecPlusScalarVec(&collInfo->Vel, -collInfo->VelDotNorm, PlaneNormal(&collInfo->Plane), &velSideways);
    if (!IsWheelLocked(wheel)) {
        VecPlusEqScalarVec(&velSideways, -velDotLook, &lookVec);
        RemoveComponent(&velSideways, &lookVec);        // repeat to remove initial fixed-point-piece-of-shit error
    }
    velSlideLen = VecLen(&velSideways);
    collInfo->SlideVel = velSlideLen;
#else
    collInfo->SlideVel = ZERO;
#endif

    // Rigid-body single collision impulse
    if (abs(collInfo->UpDotNorm) < Real(0.9)) 
    {
        impDotNormTrue = OneBodyZeroFrictionImpulse(body, &collInfo->Pos, PlaneNormal(&collInfo->Plane), -collInfo->VelDotNorm);
        impDotNormTrue -= MulScalar(abs(collInfo->UpDotNorm), impDotNormTrue);

        if (impDotNormTrue < ZERO) {
            SetVecZero(impulse);
            return;
        }
    } 
    else
    {
        impDotNormTrue = ZERO;
    }
    

    
    // Add a spring impulse
    impDotNormTrue -= MulScalar3(  SpringDampedForce(&car->Spring[collInfo->IWheel], wheel->Pos, wheel->Vel), TimeStep, collInfo->UpDotNorm);
    

    // Flag a hard knock if necessary
    if (impDotNormTrue > car->Body->BangMag) {
        car->Body->BangMag = impDotNormTrue;
        CopyPlane(&collInfo->Plane, &car->Body->BangPlane);
    }
#if 1
    // Make sure normal impulse not too large for friction calculations
    if (impDotNormTrue > 1000) 
        impDotNorm = 1000; 
    else 
        impDotNorm = impDotNormTrue;
    
    // Max impulse for downforce
    max = MulScalar(collInfo->StaticFriction, impDotNorm);

    // Driving impulse
    impDotLook = ONE - DivScalar(abs(velDotLook), car->TopSpeed);
    impDotLook = MulScalar3(impDotLook, MulScalar3(car->EngineVolt, wheel->EngineRatio, lookLen), TimeStep);

    // Add the wheel axle friction
    if ((car->EngineVolt == ZERO) || ((Sign(car->EngineVolt) != Sign(velDotLook)) && (collInfo->UpDotNorm < ZERO))) {
        impDotLook -= MulScalar(MulScalar(wheel->AxleFriction, velDotLook), TimeStep) >> 4;
    }

    // Add the sliding friction impulse
    impSlide = MulScalar3(collInfo->Grip, impDotNorm, lookLen);

    //impSlide = MulScalar(impSlide, Real(0.4));
    if (abs(car->EngineVolt) > Real(0.2)) {
        //impSlide >>= 2;
        impSlide = MulScalar(impSlide, Real(1.2) - abs(car->EngineVolt));
    }
    
    // Limit friction and torque
    scale = MulScalar(impSlide, velSlideLen);
    netFriction = MulScalar(scale, scale) + MulScalar(impDotLook, impDotLook);
    maxSq = MulScalar(max, max);
    if ((netFriction > maxSq)) {
        scale = DivScalar(maxSq, netFriction);
        scale = SquareRoot1616(scale);
        impSlide = MulScalar(scale, impSlide);
        impDotLook = MulScalar(scale, impDotLook);
        SetWheelSliding(wheel);
    }

    // Generate tangential impulse
    ScalarVecPlusScalarVec(impDotLook, &lookVec, -impSlide, &velSideways, &impTan);
    // Sum the impulses
    VecPlusScalarVec(&impTan, impDotNormTrue, PlaneNormal(&collInfo->Plane), impulse);
#else
    VecEqScalarVec(impulse, impDotNormTrue, PlaneNormal(&collInfo->Plane));
#endif

    // Set the wheel angular velocity
    if (abs(velDotLook) > 100)
        wheel->AngVel = DivScalar(velDotLook, wheel->Radius);
    else 
        wheel->AngVel = ZERO;

}


#endif // PSX





/////////////////////////////////////////////////////////////////////
//
// PostProcessCarWheelColls: if the wheels are spinning and in contact
// with something, do the skidmarks
//
/////////////////////////////////////////////////////////////////////
#ifndef _PSX
#define SKID_MAX_VELPAR         TO_VEL(Real(1000.0f))
#define SKID_MIN_NOSKID_TIME    TO_TIME(Real(0.1f))

void PostProcessCarWheelColls(CAR *car)
{
    long    skidColour;
    REAL    velParLen, dRLen;
    REAL    minSkidVel;
    REAL    normDotNorm, dirDotDir;
    VEC     velPar, dR, lookVec, vel;
    register WHEEL          *wheel;
    SKIDMARK_START  skidEnd;
    REAL    carDotUp;
    register COLLINFO_WHEEL *wheelColl;

    carDotUp = car->Body->Centre.WMatrix.m[UY];

    for (wheelColl = car->WheelCollHead; wheelColl != NULL; wheelColl = wheelColl->Next) {

        // Count Wheel collisions with floor polys
        if (carDotUp > ZERO) {
            if (MulScalar(carDotUp, wheelColl->Plane.v[Y]) < -Real(0.15)) {
                car->NWheelFloorContacts++;
            }
        }
        wheel = &car->Wheel[wheelColl->IWheel];


#ifndef _N64
        // Initialise
        if (wheelColl->Material == NULL) {
            wheel->SkidMaterial = MATERIAL_NONE;
        } else {
            wheel->SkidMaterial = wheelColl->Material - COL_MaterialInfo;
        }
#endif

        // Make sure the collision is with the world
        if ((wheelColl->Material == NULL) || 
            !(wheelColl->Material->Type & MATERIAL_SKID) || 
            ((wheelColl->CollPoly != NULL) && (wheelColl->CollPoly->Type & NO_SKID))
            ) {
            // Reset the skid started flag
            wheel->Skid.Started = FALSE;
        }

        // Don't do skidmarks below threshold sliding velocity
        if (wheel->OilTime < OILY_WHEEL_TIME) {
            minSkidVel = TO_VEL(Real(0));
        } else {
            minSkidVel = TO_VEL(Real(450));
        }
        if (wheelColl->SlideVel < minSkidVel) {
            wheel->Skid.Started = FALSE;
        }

        // Calculate velocity parallel to plane
        //velDotNorm = VecDotVec(&wheelColl->Vel, PlaneNormal(&wheelColl->Plane));
        VecPlusScalarVec(&wheelColl->Vel, -wheelColl->VelDotNorm, PlaneNormal(&wheelColl->Plane), &velPar);
        velParLen = VecDotVec(&velPar, &velPar);//Length(&velPar);
        if (velParLen > SMALL_REAL) {
            velParLen = (REAL)sqrt(velParLen);
            VecDivScalar(&velPar, velParLen);
        } else {
            //SetVecZero(&velPar);
            wheel->Skid.Started = FALSE;
            continue;
        }


        // If necessary, add a skidmark to the skidmark list
        if ((IsWheelSkidding(wheel) || (wheel->Skid.NoSkidTime < SKID_MIN_NOSKID_TIME)) && wheel->Skid.Started) {
            // Calculate length of skidmark so far
            VecMinusVec(&wheelColl->WorldPos, &wheel->Skid.Pos, &dR);
            dRLen = Length(&dR);

            // Parameters for this end of skid
            VecCrossVec(PlaneNormal(&wheelColl->Plane), &wheel->Axes.mv[R], &lookVec);
            CopyVec(&wheelColl->WorldPos, &skidEnd.Pos);
            CopyVec(&velPar, &skidEnd.Dir);
            CopyVec(PlaneNormal(&wheelColl->Plane), &skidEnd.Normal);
            skidEnd.Width = MulScalar( wheel->SkidWidth / 2, abs(VecDotVec(&lookVec, &skidEnd.Dir)) );
            if (skidEnd.Width < MulScalar( Real(0.2), wheel->SkidWidth) ) skidEnd.Width = MulScalar( Real(0.2), wheel->SkidWidth );
            skidEnd.Material = wheelColl->Material;

            dirDotDir = VecDotVec(&skidEnd.Dir, &wheel->Skid.Dir);
            normDotNorm = VecDotVec(&skidEnd.Normal, &wheel->Skid.Normal);

            // Get skid colour
            if (wheel->OilTime < OILY_WHEEL_TIME) {
                skidColour = 0xffffff;
            } else {
                skidColour = wheelColl->Material->SkidColour;
#ifdef _PC
                // scale skid colour according to sliding speed
                long alpha = (long)(wheelColl->SlideVel * Real(500.0) / Real(1000.0));
                if (alpha < 255) {
                    long r, g, b;
                    r = (skidColour && RGB_RED_MASK) >> 16;
                    g = (skidColour && RGB_GREEN_MASK) >> 8;
                    b = (skidColour && RGB_BLUE_MASK);
                    r = (r * alpha) >> 8;
                    g = (g * alpha) >> 8;
                    b = (b * alpha) >> 8;
                    skidColour = r << 26 | g << 8 | b;
                }
#endif
            }

            // Add a skidmark to the list or update the current one
            if (wheel->Skid.CurrentSkid == NULL) {
                // Make sure we are on the same surface
                //if ((normDotNorm > SKID_MAX_DOT)) {
                if ((normDotNorm > SKID_MAX_DOT)  && (skidEnd.Material == wheel->Skid.Material)) {
                    wheel->Skid.CurrentSkid = AddSkid(&wheel->Skid, &skidEnd, skidColour);
                } else {
                    wheel->Skid.Started = FALSE;
                    continue;
                }
            } else {
                if (dirDotDir < ZERO) {
                    NegateVec(&skidEnd.Dir);
                }
                // Make sure we are on the same surface
                if ((normDotNorm > SKID_MAX_DOT)) {
                    MoveSkidEnd(wheel->Skid.CurrentSkid, &skidEnd, skidColour);
                }
            }

#ifndef _PSX
            // Smokin'
            wheel->Skid.LastSmokeTime += TimeStep;
            if (car->RenderedAll && (wheel->Skid.LastSmokeTime > SKID_SMOKE_TIME) 
                #ifdef _PC
                && RenderSettings.Skid
                #endif
                ) {
                VecPlusVec(&SmokeVel, &velPar, &vel)
                CreateSpark(SPARK_SMOKE1, &wheelColl->WorldPos, &vel, ZERO, 0);
                wheel->Skid.LastSmokeTime = ZERO;
            }
#endif // _NPSX

            // Check to see if it is time to start a new skid
            if ((dRLen > SKID_MAX_LEN) ||
                ((dRLen > SKID_MIN_LEN) && (dirDotDir < SKID_MAX_DOT)) ||
                (normDotNorm < SKID_MAX_DOT)) 
            {
                // Reset the skid started flag
                wheel->Skid.Started = FALSE;
            }
        }

        if (!IsWheelSkidding(wheel)) {
            wheel->Skid.NoSkidTime += TimeStep;
        } else {
            wheel->Skid.NoSkidTime = ZERO;
        }

        // Start a new skid if necessary
        if (!wheel->Skid.Started || !IsWheelSkidding(wheel)) {
            // Store skid parameters
            VecCrossVec(PlaneNormal(&wheelColl->Plane), &wheel->Axes.mv[R], &lookVec);
            CopyVec(&wheelColl->WorldPos, &wheel->Skid.Pos);
            CopyVec(&velPar, &wheel->Skid.Dir);
            CopyVec(PlaneNormal(&wheelColl->Plane), &wheel->Skid.Normal);
            wheel->Skid.Width = MulScalar( wheel->SkidWidth / 2, abs(VecDotVec(&lookVec, &wheel->Skid.Dir)) );
            if (wheel->Skid.Width < MulScalar( Real(0.2), wheel->SkidWidth) ) wheel->Skid.Width = MulScalar( Real(0.2), wheel->SkidWidth );
            wheel->Skid.Material = wheelColl->Material;
            wheel->Skid.LastSmokeTime = ZERO;

            // Set the skid started flag
            wheel->Skid.Started = TRUE;
            wheel->Skid.CurrentSkid = NULL;

        }
    }
}

#else //NPSX

#define SKID_MAX_VELPAR         TO_VEL(Real(1000.0f))
#define SKID_MIN_NOSKID_TIME    TO_TIME(Real(0.1f))

void PostProcessCarWheelColls(CAR *car)
{
    long            skidColour;
    REAL            velParLen, dRLen;
    REAL            minSkidVel;
    REAL            normDotNorm, dirDotDir;
    VEC             velPar, dR, lookVec, vel;
    WHEEL           *wheel;
    SKIDMARK_START  skidEnd;
    COLLINFO_WHEEL  *wheelColl;
    REAL            carDotUp = car->Body->Centre.WMatrix.m[UY];

    for (wheelColl = car->WheelCollHead; wheelColl != NULL; wheelColl = wheelColl->Next) {
        wheel = &car->Wheel[wheelColl->IWheel];

        // Count Wheel collisions with floor polys
        if (carDotUp > ZERO) {
            if (MulScalar(carDotUp, wheelColl->Plane.v[Y]) < -Real(0.15)) {
                car->NWheelFloorContacts++;
            }
        }

        // Don't do skidmarks for wheels that don't want them
        if (wheel->SkidWidth < ZERO) continue;
        //continue;

        // Make sure the collision is with the world
        if ((wheelColl->Material == NULL) || 
            !(wheelColl->Material->Type & MATERIAL_SKID) || 
            ((wheelColl->CollPoly != NULL) && (wheelColl->CollPoly->Type & NO_SKID))
            ) {
            // Reset the skid started flag
            wheel->Skid.Started = FALSE;
        }



        // Don't do skidmarks below threshold sliding velocity
        if (wheel->OilTime < OILY_WHEEL_TIME) {
            minSkidVel = TO_VEL(Real(0));
        } else {
            minSkidVel = TO_VEL(Real(450));
        }
        if (wheelColl->SlideVel < minSkidVel) {
            wheel->Skid.Started = FALSE;
        }



        // Calculate velocity parallel to plane
        VecPlusScalarVec(&wheelColl->Vel, -wheelColl->VelDotNorm, PlaneNormal(&wheelColl->Plane), &velPar);
        velParLen = VecDotVec(&velPar, &velPar);
        if (velParLen > SMALL_REAL) {
            velParLen = (REAL)sqrt(velParLen);
            VecDivScalar(&velPar, velParLen);
        } else {
            //SetVecZero(&velPar);
            wheel->Skid.Started = FALSE;
            continue;
        }


        // If necessary, add a skidmark to the skidmark list
        if ((IsWheelSkidding(wheel) || (wheel->Skid.NoSkidTime < SKID_MIN_NOSKID_TIME)) && wheel->Skid.Started) {
            // Calculate length of skidmark so far
            VecMinusVec(&wheelColl->WorldPos, &wheel->Skid.Pos, &dR);
            //dRLen = Length(&dR);
            dRLen = Max(Max(dR.v[X], dR.v[Y]), dR.v[Z]);

            // Parameters for this end of skid
            //VecCrossVec(PlaneNormal(&wheelColl->Plane), &wheel->Axes.mv[R], &lookVec);
            CopyVec(&wheel->Axes.mv[L], &lookVec);
            CopyVec(&wheelColl->WorldPos, &skidEnd.Pos);
            CopyVec(&velPar, &skidEnd.Dir);
            CopyVec(PlaneNormal(&wheelColl->Plane), &skidEnd.Normal);

            skidEnd.Width = wheel->SkidWidth>>1;//MulScalar( wheel->SkidWidth / 2, abs(VecDotVec(&lookVec, &skidEnd.Dir)) );
            //if (skidEnd.Width < MulScalar( Real(0.2), wheel->SkidWidth) ) skidEnd.Width = MulScalar( Real(0.2), wheel->SkidWidth );
            skidEnd.Material = wheelColl->Material;

            dirDotDir = VecDotVec(&skidEnd.Dir, &wheel->Skid.Dir);
            normDotNorm = VecDotVec(&skidEnd.Normal, &wheel->Skid.Normal);
            //normDotNorm = skidEnd.Normal.v[Y] * wheel->Skid.Normal.v[Y];

            // Get skid colour
            if (wheel->OilTime < OILY_WHEEL_TIME) {
                skidColour = 0xffffff;
            } else {
                skidColour = wheelColl->Material->SkidColour;
            }

            // Add a skidmark to the list or update the current one
            if (wheel->Skid.CurrentSkid == NULL) {
                // Make sure we are on the same surface
                if ((normDotNorm > SKID_MAX_DOT)) {//  && (skidEnd.Material == wheel->Skid.Material)) {
                    wheel->Skid.CurrentSkid = AddSkid(&wheel->Skid, &skidEnd, skidColour);
                } else {
                    wheel->Skid.Started = FALSE;
                    continue;
                }
            } else {
                if (dirDotDir < ZERO) {
                    NegateVec(&skidEnd.Dir);
                }
                // Make sure we are on the same surface
                if ((normDotNorm > SKID_MAX_DOT)) {
                    MoveSkidEnd(wheel->Skid.CurrentSkid, &skidEnd, skidColour);
                }
            }


            // Check to see if it is time to start a new skid
            if ((dRLen > SKID_MAX_LEN) ||
                ((dRLen > SKID_MIN_LEN) && (dirDotDir < SKID_MAX_DOT)) ||
                (normDotNorm < SKID_MAX_DOT)) 
            {
                // Reset the skid started flag
                wheel->Skid.Started = FALSE;
            }
        }

        if (!IsWheelSkidding(wheel)) {
            wheel->Skid.NoSkidTime += TimeStep;
        } else {
            wheel->Skid.NoSkidTime = ZERO;
        }

        // Start a new skid if necessary
        if (!wheel->Skid.Started || !IsWheelSkidding(wheel)) {
            // Store skid parameters
            //VecCrossVec(PlaneNormal(&wheelColl->Plane), &wheel->Axes.mv[R], &lookVec);
            CopyVec(&wheel->Axes.mv[L], &lookVec);
            CopyVec(&wheelColl->WorldPos, &wheel->Skid.Pos);
            CopyVec(&velPar, &wheel->Skid.Dir);
            CopyVec(PlaneNormal(&wheelColl->Plane), &wheel->Skid.Normal);

            wheel->Skid.Width = wheel->SkidWidth>>1;//MulScalar( wheel->SkidWidth / 2, abs(VecDotVec(&lookVec, &wheel->Skid.Dir)) );
            //if (wheel->Skid.Width < MulScalar( Real(0.2), wheel->SkidWidth) ) wheel->Skid.Width = MulScalar( Real(0.2), wheel->SkidWidth );
            wheel->Skid.Material = wheelColl->Material;
            wheel->Skid.LastSmokeTime = ZERO;

            // Set the skid started flag
            wheel->Skid.Started = TRUE;
            wheel->Skid.CurrentSkid = NULL;

        }
    }

}
#endif

/////////////////////////////////////////////////////////////////////
//
// SetAllCarCoMs: move all car centre of masses according to the
// value in the CarInfo structure.
//
/////////////////////////////////////////////////////////////////////

void SetAllCarCoMs()
{
    int iCar;

    for (iCar = 0; iCar < NCarTypes; iCar++) {

        MoveCarCoM(&CarInfo[iCar], &CarInfo[iCar].CoMOffset);

    }
}

////////////////////////////////////////////////////////////
//
// UnsetAllCarCoMs: put all offsets back to their original
// values (assuming SetAllCarCoMs has already been called.
// This is used to save out the CarInfo files.
//
////////////////////////////////////////////////////////////

void UnsetAllCarCoMs()
{
    int iCar;
    VEC dR;

    for (iCar = 0; iCar < NCarTypes; iCar++) {

        CopyVec(&CarInfo[iCar].CoMOffset, &dR);
        NegateVec(&dR);
        MoveCarCoM(&CarInfo[iCar], &dR);

    }
}


/////////////////////////////////////////////////////////////////////
//
// MoveCarCoM: move the centre of mass of the specified car by the
// specified vector. Does not alter the inertia matrix of the car.
//
/////////////////////////////////////////////////////////////////////

void MoveCarCoM(CAR_INFO *carInfo, VEC *dR)
{
    int iWheel;

    // Move the body model
    VecPlusEqVec(&carInfo->Body.Offset, dR);

    // Move the Wheels, axles and suspension fixing points
    for (iWheel = 0; iWheel < CAR_NWHEELS; iWheel++) {
        VecPlusEqVec(&carInfo->Wheel[iWheel].Offset1, dR);
        VecPlusEqVec(&carInfo->Spring[iWheel].Offset, dR);
        VecPlusEqVec(&carInfo->Pin[iWheel].Offset, dR);
        VecPlusEqVec(&carInfo->Axle[iWheel].Offset, dR);
    }

    // Move the Aerial
    VecPlusEqVec(&carInfo->Aerial.Offset, dR);

    // Move car spinner
    VecPlusEqVec(&carInfo->Spinner.Offset, dR);
}

/////////////////////////////////////////////////////////////////////
//
// SetCarAngResistance:
//
/////////////////////////////////////////////////////////////////////

void SetCarAngResistance(CAR *car)
{
    int iWhl;

    int nCont = 0;

    // Count wheels on the floor
    for (iWhl = 0; iWhl < CAR_NWHEELS; iWhl++) {
        if (IsWheelInContact(&car->Wheel[iWhl])) {
            nCont++;
        }
    }

    // slow car spinning if no wheels on the floor
    if (nCont == 0) {
        car->Body->AngResistance = MulScalar(car->Body->AngResMod, car->Body->DefaultAngRes);
    } else {
        car->Body->AngResistance = car->Body->DefaultAngRes;
    }

}

/////////////////////////////////////////////////////////////////////
//
// CarWorldColls: detect all collisions between the car and the world
// mesh
//
/////////////////////////////////////////////////////////////////////


void DetectCarWorldColls(CAR *car)
{
    long    iPoly, iWheel;
    long    start, end;
    COLLGRID *collGrid;
    BBOX carBBox, polyBBox;
    WHEEL *wheel;
    short *pIndex;
    NEWCOLLPOLY *collPoly;

    // Calculate the grid position and which polys to check against
    collGrid = PosToCollGrid(&car->Body->Centre.Pos);
    if (collGrid == NULL) return;

    // Reset all collision related flags for the car
    wheel = &car->Wheel[0];
    for (iWheel = 0; iWheel < CAR_NWHEELS; iWheel++, wheel++) {
        // Make sure skidmarks are stopped if the wheel is in the air
        if (!IsWheelInContact(wheel)) {
            wheel->Skid.Started = FALSE;
        }

        // Clear most of the wheel flags...
        wheel->Status &= (WHEEL_PRESENT | WHEEL_POWERED | WHEEL_STEERED | WHEEL_OIL | WHEEL_LOCKED);

        if (IsWheelinOil(wheel)) {
            SetWheelNotInOil(wheel);
            wheel->OilTime = ZERO;
        } else {
            wheel->OilTime += TimeStep;
            if( wheel->OilTime > OILY_WHEEL_TIME )  // Added by Greg
                wheel->OilTime = OILY_WHEEL_TIME;
        }
    }

    CopyBBox(&car->BBox, &carBBox);

    // Detect collisions
#ifndef _PSX
    GetGridIndexRange(collGrid, carBBox.YMin, carBBox.YMax, &start, &end);
#else
    start = 0;
    end = collGrid->NCollPolys;
#endif

    pIndex = &collGrid->CollPolyIndices[start];
    for (iPoly = start; iPoly < end; iPoly++, pIndex++) {
        collPoly = GetCollPoly(*pIndex);

        CopyBBox(&collPoly->BBox, &polyBBox);

#if DEBUG
        COL_NCollsTested++;
#endif

        // Full car-poly bounding box test
        if (!BBTestXZY(&polyBBox, &carBBox)) continue;

#if DEBUG
        COL_NCollsPassed++;
#endif

        if (PolyCameraOnly(collPoly)) continue;

        // WHEEL - WORLD
        wheel = &car->Wheel[0];
        for (iWheel = 0; iWheel < CAR_NWHEELS; iWheel++, wheel++) {

#ifndef _PSX
            if (BBTestXZY(&wheel->BBox, &polyBBox)) 
#else
            if (SphereBBTest(&wheel->CentrePos, wheel->Radius, &polyBBox))
#endif
            {
                DetectCarWheelColls2(car, iWheel, collPoly);
            }
            
        }

        // BODY - WORLD
        if (BBTestYXZ(&polyBBox, &car->Body->CollSkin.BBox)) {
            DetectConvexHullPolyColls(car->Body, collPoly);
        }

    }

#if USE_DEBUG_ROUTINES
    DEBUG_CollGrid = collGrid - COL_CollGrid;
#endif
}


////////////////////////////////////////////////////////////////
//
// Detect wheel-world collisions
//
////////////////////////////////////////////////////////////////
void DetectCarWheelColls2(CAR *car, int iWheel, NEWCOLLPOLY *worldPoly)
{
    VEC tmpVec;//, tmpVec2;
    REAL    time;
    WHEEL   *wheel = &car->Wheel[iWheel];
    COLLINFO_WHEEL  *wheelColl;

    // Make sure we don't overstep the collision array
    if ((wheelColl = NextWheelCollInfo()) == NULL) return;

    // Quick swepth-volume axis-aligned bounding-box test
    //if (!BBTestYXZ(&wheel->BBox, &worldPoly->BBox)) return;

    // Do a sphere-to-poly collision test
    if (SphereCollPoly(&wheel->OldCentrePos, &wheel->CentrePos,
        wheel->Radius, 
        worldPoly, 
        &wheelColl->Plane,
        &wheelColl->Pos,
        &wheelColl->WorldPos,
        &wheelColl->Depth,
        &time))
    {

        //CopyVec(&worldPoly->Plane, &wheelColl->Plane);

        // Calculate the collision point on the plane (for skidmarks and smoke generator)
        VecPlusEqScalarVec(&wheelColl->WorldPos, SKID_RAISE, PlaneNormal(&wheelColl->Plane));

        // Calculate the car-relative collision point for response
        VecPlusEqVec(&wheelColl->Pos, &wheel->CentrePos);
        VecMinusEqVec(&wheelColl->Pos, &car->Body->Centre.Pos);

        // Calculate world velocity of the wheel collision point (not including wheel spin)
        VecCrossVec(&car->Body->AngVel, &wheelColl->Pos, &wheelColl->Vel);
        VecPlusEqVec(&wheelColl->Vel, &car->Body->Centre.Vel);

        // Make sure that the wheel is not already travelling away from the surface
        VecPlusScalarVec(&wheelColl->Vel, wheel->Vel, &car->Body->Centre.WMatrix.mv[U], &tmpVec);
        //if (VecDotVec(&tmpVec, PlaneNormal(&wheelColl->Plane)) > ZERO) return;
        if (VecDotVec(&tmpVec, PlaneNormal(&worldPoly->Plane)) > ZERO) return;

        // Add bumps from surface corrugation
        wheelColl->Material = &COL_MaterialInfo[worldPoly->Material];
        AdjustWheelColl(wheelColl, wheelColl->Material);

        // Set other necessary stuff
        wheelColl->Car = car;
        wheelColl->IWheel = iWheel;
        wheelColl->Grip = MulScalar(wheel->Grip, wheelColl->Material->Gripiness);
        wheelColl->StaticFriction = MulScalar(wheel->StaticFriction, wheelColl->Material->Roughness);
        wheelColl->KineticFriction = MulScalar(wheel->KineticFriction, wheelColl->Material->Roughness);
        wheelColl->Restitution = ZERO;
        wheelColl->Body2 = &BDY_MassiveBody;
        wheelColl->CollPoly = worldPoly;
        SetVecZero(&wheelColl->Pos2);


        // Set the wheel-in-contact flag
//      SetWheelInContact(wheel);
//      SetWheelNotSpinning(wheel);

        // add collision to car's list
        AddWheelColl(car, wheelColl);

    }
}

#if defined(_PSX)

/*#undef VecDotPlane
#define VecDotPlane(vec, plane)                                                     \
({                                                                                  \
    register REAL m1 = (vec)->v[X];                                                 \
    register REAL m2 = (vec)->v[Y];                                                 \
    register REAL m3 = (plane)->v[A];                                               \
    register REAL m4 = (plane)->v[B];                                               \
    register REAL r;                                                                \
    r = MulScalar(m1, m3) + MulScalar(m2, m4);                                      \
    m1 = (vec)->v[Z]; m2 = (plane)->v[C]; m3 = (plane)->v[D];                       \
    r += MulScalar(m1, m2) + m3;                                                    \
})
*/
bool SphereCollPoly(VEC *oldPos, VEC *newPos, REAL radius, NEWCOLLPOLY *collPoly, PLANE *plane, VEC *relPos, VEC *worldPos, REAL *depth, REAL *time)
{
    REAL oldDist, newDist;

    // Make sure sphere is within "radius" of the plane of the polygon or on the inside
    newDist = VecDotPlane(newPos, &collPoly->Plane);
    if (newDist - radius > COLL_EPSILON) return FALSE;

    // If we were inside the plane last time, probably no collision
    oldDist = VecDotPlane(oldPos, &collPoly->Plane);
    if (oldDist < -(radius + COLL_EPSILON)) return FALSE;

    // Calculate the coordinates of the collision point
    //VecPlusScalarVec(newPos, -newDist, PlaneNormal(&collPoly->Plane), worldPos);

    oldDist -= radius;
    newDist -= radius;

    if (oldDist - newDist > TO_LENGTH(Real(2))) {
        *time = DivScalar(oldDist, (oldDist - newDist));
        //if (*time > ONE) return FALSE;
        //InterpVec(oldPos, newPos, *time, worldPos);
        ScalarVecPlusScalarVec(ONE - *time, oldPos, *time, newPos, worldPos);
        VecPlusEqScalarVec(worldPos, -radius, PlaneNormal(&collPoly->Plane));
    } else {
        *time = ZERO;
        VecPlusScalarVec(newPos, -(newDist + radius), PlaneNormal(&collPoly->Plane), worldPos);
    }


    // Find axis with a normal component greater than 0.5 (must be at least one component > 1/sqrt(3))
    if (abs(collPoly->Plane.v[A]) > HALF) {
        if (!LineCollPolyYZ(worldPos, collPoly)) return FALSE;
    } else if(abs(collPoly->Plane.v[B]) > HALF) {
        if (!LineCollPolyXZ(worldPos, collPoly)) return FALSE;
    } else {
        if (!LineCollPolyXY(worldPos, collPoly)) return FALSE;
    }

    // Calculate the time at which the sphere was radius away from the poly
    /*if (oldDist - newDist > SMALL_REAL) {
        //*time = DivScalar((oldDist - radius), (oldDist - newDist));
        *time = DivScalar(oldDist, (oldDist - newDist));
    } else {
        *time = ZERO;
    }
    if (*time < ZERO) *time = ZERO;*/

    // Set the return parameters
    //*depth = newDist - radius;
    *depth = newDist;
    CopyPlane(&collPoly->Plane, plane);
    VecEqScalarVec(relPos, -radius, PlaneNormal(plane));

    
    return TRUE;
}

bool LineCollPolyXY(VEC *pos, NEWCOLLPOLY *collPoly) 
{
    REAL x0, y0, x1, y1, xp, yp, t;
    int leftCount, iPt1, iPt2, nEdges;

    // Initialise
    leftCount = 0;
    nEdges = (IsPolyQuad(collPoly))? 4: 3;

    x1 = collPoly->Vertex[0].v[X];
    y1 = collPoly->Vertex[0].v[Y];

    xp = pos->v[X];
    yp = pos->v[Y];

    // Loop over edges
    for (iPt1 = 1; iPt1 <= nEdges; iPt1++) {

        // Get next 2D edge coords
        x0 = x1;
        y0 = y1;
        if (iPt1 == nEdges) {
            x1 = collPoly->Vertex[0].v[X];
            y1 = collPoly->Vertex[0].v[Y];
        } else {
            x1 = collPoly->Vertex[iPt1].v[X];
            y1 = collPoly->Vertex[iPt1].v[Y];
        }

        // Is point in y bounds of edge?
        if ((yp > y0 + (COLL_EPSILON*0)) && (yp > y1 + (COLL_EPSILON*0))) continue;
        if ((yp < y0 - (COLL_EPSILON*0)) && (yp < y1 - (COLL_EPSILON*0))) continue;

        // Is point beyond far left of edge?
        if ((xp < x0 - (COLL_EPSILON*0)) && (xp < x1 - (COLL_EPSILON*0))) {
            leftCount++;
            continue;
        }

        // Is Point beyond far right of edge?
        if ((xp > x0 + (COLL_EPSILON*0)) && (xp > x1 + (COLL_EPSILON*0))) continue;

        // Is point to left?
        t = DivScalar(yp - y0, y1 - y0);
        t = MulScalar(t, x1 - x0);
        t += x0 - xp;
        if (t > ZERO + (COLL_EPSILON*0)) {
            leftCount++;
            continue;
        }

    }

    // If to left of odd number of edges, collision occurred
    if (leftCount & 1) return TRUE;

    // No collision
    return FALSE;
}

bool LineCollPolyXZ(VEC *pos, NEWCOLLPOLY *collPoly) 
{
    REAL x0, y0, x1, y1, xp, yp, t;
    int leftCount, iPt1, iPt2, nEdges;

    // Initialise
    leftCount = 0;
    nEdges = (IsPolyQuad(collPoly))? 4: 3;

    x1 = collPoly->Vertex[0].v[X];
    y1 = collPoly->Vertex[0].v[Z];

    xp = pos->v[X];
    yp = pos->v[Z];

    // Loop over edges
    for (iPt1 = 1; iPt1 <= nEdges; iPt1++) {

        // Get next 2D edge coords
        x0 = x1;
        y0 = y1;
        if (iPt1 == nEdges) {
            x1 = collPoly->Vertex[0].v[X];
            y1 = collPoly->Vertex[0].v[Z];
        } else {
            x1 = collPoly->Vertex[iPt1].v[X];
            y1 = collPoly->Vertex[iPt1].v[Z];
        }

        // Is point in y bounds of edge?
        if ((yp >= y0 + (COLL_EPSILON*0)) && (yp >= y1 + (COLL_EPSILON*0))) continue;
        if ((yp <= y0 - (COLL_EPSILON*0)) && (yp <= y1 - (COLL_EPSILON*0))) continue;

        // Is point beyond far left of edge?
        if ((xp <= x0 - (COLL_EPSILON*0)) && (xp <= x1 - (COLL_EPSILON*0))) {
            leftCount++;
            continue;
        }

        // Is Point beyond far right of edge?
        if ((xp >= x0 + (COLL_EPSILON*0)) && (xp >= x1 + (COLL_EPSILON*0))) continue;

        // Is point to left?
        t = DivScalar(yp - y0, y1 - y0);
        t = MulScalar(t, x1 - x0);
        t += x0 - xp;
        if (t > ZERO + (COLL_EPSILON*0)) {
            leftCount++;
            continue;
        }

    }

    // If to left of odd number of edges, collision occurred
    if (leftCount & 1) return TRUE;

    // No collision
    return FALSE;
}

bool LineCollPolyYZ(VEC *pos, NEWCOLLPOLY *collPoly) 
{
    REAL x0, y0, x1, y1, xp, yp, t;
    int leftCount, iPt1, iPt2, nEdges;

    // Initialise
    leftCount = 0;
    nEdges = (IsPolyQuad(collPoly))? 4: 3;

    x1 = collPoly->Vertex[0].v[Y];
    y1 = collPoly->Vertex[0].v[Z];

    xp = pos->v[Y];
    yp = pos->v[Z];

    // Loop over edges
    for (iPt1 = 1; iPt1 <= nEdges; iPt1++) {

        // Get next 2D edge coords
        x0 = x1;
        y0 = y1;
        if (iPt1 == nEdges) {
            x1 = collPoly->Vertex[0].v[Y];
            y1 = collPoly->Vertex[0].v[Z];
        } else {
            x1 = collPoly->Vertex[iPt1].v[Y];
            y1 = collPoly->Vertex[iPt1].v[Z];
        }

        // Is point in y bounds of edge?
        if ((yp > y0 + (COLL_EPSILON*0)) && (yp > y1 + (COLL_EPSILON*0))) continue;
        if ((yp < y0 - (COLL_EPSILON*0)) && (yp < y1 - (COLL_EPSILON*0))) continue;

        // Is point beyond far left of edge?
        if ((xp < x0 - (COLL_EPSILON*0)) && (xp < x1 - (COLL_EPSILON*0))) {
            leftCount++;
            continue;
        }

        // Is Point beyond far right of edge?
        if ((xp > x0 + (COLL_EPSILON*0)) && (xp > x1 + (COLL_EPSILON*0))) continue;

        // Is point to left?
        t = DivScalar(yp - y0, y1 - y0);
        t = MulScalar(t, x1 - x0);
        t += x0 - xp;
        if (t > ZERO + (COLL_EPSILON*0)) {
            leftCount++;
            continue;
        }

    }

    // If to left of odd number of edges, collision occurred
    if (leftCount & 1) return TRUE;

    // No collision
    return FALSE;
}

#endif  //defined(_PC, _N64, PSX)

////////////////////////////////////////////////////////////////
//
// Detect collisions between wheels and polygonal bodies
//
////////////////////////////////////////////////////////////////

int DetectWheelPolyColls(CAR *car, int iWheel, NEWBODY *body)
{
    int iPoly;
    VEC tmpVec;
    REAL    time;
    NEWCOLLPOLY *collPoly;
    int nColls = 0;
    WHEEL   *wheel = &car->Wheel[iWheel];
    COLLINFO_WHEEL  *wheelColl;

    Assert(IsBodyPoly(body));

    // Bounding box test
#ifndef _PSX
    if (!BBTestYXZ(&wheel->BBox, &body->CollSkin.BBox)) return 0;
#else
    if (!SphereBBTest(&wheel->CentrePos, wheel->Radius, &body->CollSkin.BBox)) return 0;
#endif

    for (iPoly = 0; iPoly < body->CollSkin.NCollPolys; iPoly++) {
        collPoly = &body->CollSkin.WorldCollPoly[iPoly];

        if ((wheelColl = NextWheelCollInfo()) == NULL) return nColls;
     
        // Quick swepth-volume axis-aligned bounding-box test
#ifndef _PSX
        if (!BBTestYXZ(&wheel->BBox, &collPoly->BBox)) continue;
#else
        if (!SphereBBTest(&wheel->CentrePos, wheel->Radius, &collPoly->BBox)) continue;
#endif

        // Do a sphere-to-poly collision test
        if (SphereCollPoly(&wheel->OldCentrePos, &wheel->CentrePos,
            wheel->Radius, 
            collPoly, 
            &wheelColl->Plane,
            &wheelColl->Pos,
            &wheelColl->WorldPos,
            &wheelColl->Depth,
            &time))
        {

            // Calculate the collision point on the plane (for skidmarks and smoke generator)
            VecPlusEqScalarVec(&wheelColl->WorldPos, SKID_RAISE, PlaneNormal(&wheelColl->Plane));

            // Calculate the car-relative collision point for response
            VecPlusEqVec(&wheelColl->Pos, &wheel->CentrePos);
            VecMinusEqVec(&wheelColl->Pos, &car->Body->Centre.Pos);

            // Calculate world velocity of the wheel collision point (not including wheel spin)
            VecCrossVec(&car->Body->AngVel, &wheelColl->Pos, &wheelColl->Vel);
            VecPlusEqVec(&wheelColl->Vel, &car->Body->Centre.Vel);
            VecMinusEqVec(&wheelColl->Vel, &body->Centre.Vel);

            // Make sure that the wheel is not already travelling away from the surface
            VecPlusScalarVec(&wheelColl->Vel, wheel->Vel, &car->Body->Centre.WMatrix.mv[U], &tmpVec);
            if (VecDotVec(&tmpVec, PlaneNormal(&wheelColl->Plane)) > ZERO) continue;

            // Add bumps from surface corrugation
            wheelColl->Material = &COL_MaterialInfo[collPoly->Material];
            //AdjustWheelColl(wheelColl, wheelColl->Material);

            // Set other necessary stuff
            wheelColl->Car = car;
            wheelColl->IWheel = iWheel;
#ifndef _PSX
            wheelColl->Grip = MulScalar(wheel->Grip, wheelColl->Material->Gripiness);
            wheelColl->StaticFriction = MulScalar(wheel->StaticFriction, wheelColl->Material->Roughness);
            wheelColl->KineticFriction = MulScalar(wheel->KineticFriction, wheelColl->Material->Roughness);
#else
            wheelColl->Grip = wheel->Grip;
            wheelColl->StaticFriction = wheel->StaticFriction;
            wheelColl->KineticFriction = wheel->KineticFriction;
#endif
            wheelColl->Restitution = ZERO;
            wheelColl->Body2 = &BDY_MassiveBody;
            SetVecZero(&wheelColl->Pos2);
            wheelColl->CollPoly = NULL;


            // Set the wheel-in-contact flag
//          SetWheelInContact(wheel);
//          SetWheelNotSpinning(wheel);

            AddWheelColl(car, wheelColl);
            nColls++;

        }
    }
    return nColls;
}


int DetectWheelSphereColls(CAR *car, int iWheel, NEWBODY *body)
{
    VEC dR;
    REAL    dRLen;
    COLLINFO_WHEEL  *wheelColl;
    COLLINFO_BODY   *bodyColl;
    WHEEL   *wheel = &car->Wheel[iWheel];
    SPHERE  *sphere = &body->CollSkin.WorldSphere[0];

    // Bounding box test
#ifndef _PSX
    if (!BBTestYXZ(&wheel->BBox, &body->CollSkin.BBox)) return 0;
#else
    if (!SphereBBTest(&wheel->CentrePos, wheel->Radius, &body->CollSkin.BBox)) return 0;
#endif

    // Set up the collision info
    if ((wheelColl = NextWheelCollInfo()) == NULL) return 0;
    if ((bodyColl = NextBodyCollInfo(body)) == NULL) return 0;

    //Get relative position
    VecMinusVec(&wheel->CentrePos, &sphere->Pos, &dR);
    dRLen = VecLen(&dR);

    // Check for collision
    if (dRLen > wheel->Radius + sphere->Radius) return 0;

    // Collision Occurred

    bodyColl->Body1 = body;
    bodyColl->Body2 = car->Body;
    wheelColl->Car = car;
    wheelColl->IWheel = iWheel;
    wheelColl->Body2 = body;
    SetVecZero(&wheelColl->WorldPos);   // DODGY....
    SetVecZero(&bodyColl->WorldPos);    // DODGY....

    bodyColl->Depth = MulScalar(HALF, (dRLen - wheel->Radius - sphere->Radius));
    wheelColl->Depth = bodyColl->Depth;

    // Collision plane
    if (dRLen > SMALL_REAL) {
        CopyVec(&dR, PlaneNormal(&wheelColl->Plane));
        VecDivScalar(PlaneNormal(&wheelColl->Plane), dRLen);
        wheelColl->Time = ONE - DivScalar(wheelColl->Depth, dRLen);
    } else {
        SetVec(PlaneNormal(&wheelColl->Plane), ONE, ZERO, ZERO);
        wheelColl->Time = ONE;
    }
    FlipPlane(&wheelColl->Plane, &bodyColl->Plane);
    bodyColl->Time = wheelColl->Time;

    // Calculate the collision points for response
    VecEqScalarVec(&wheelColl->Pos, wheel->Radius, PlaneNormal(&wheelColl->Plane));
    VecPlusEqVec(&wheelColl->Pos, &wheel->CentrePos);
    VecMinusEqVec(&wheelColl->Pos, &car->Body->Centre.Pos);
    //VecEqScalarVec(&bodyColl->Pos1, sphere->Radius, PlaneNormal(&bodyColl->Plane));
    VecPlusScalarVec(&wheel->CentrePos, HALF, &dR, &wheelColl->WorldPos);
    CopyVec(&wheelColl->Pos, &bodyColl->Pos2);
    CopyVec(&wheelColl->WorldPos, &bodyColl->WorldPos);
    VecPlusScalarVec(&sphere->Pos, -sphere->Radius, PlaneNormal(&wheelColl->Plane), &bodyColl->Pos1);
    VecMinusEqVec(&bodyColl->Pos1, &body->Centre.Pos);
    VecMinusVec(&bodyColl->WorldPos, &body->Centre.Pos, &bodyColl->Pos1);
    CopyVec(&bodyColl->Pos1, &wheelColl->Pos2);

    // Calculate velocity
    VecCrossVec(&car->Body->AngVel, &wheelColl->Pos, &wheelColl->Vel);
    VecPlusEqVec(&wheelColl->Vel, &car->Body->Centre.Vel);
    VecCrossVec(&body->AngVel, &bodyColl->Pos1, &bodyColl->Vel);
    VecPlusEqVec(&bodyColl->Vel, &body->Centre.Vel);
    VecMinusEqVec(&bodyColl->Vel, &wheelColl->Vel);
    CopyVec(&bodyColl->Vel, &wheelColl->Vel);
    NegateVec(&wheelColl->Vel);

#ifndef _PSX
    wheelColl->Grip = wheel->Grip * body->Centre.Grip;
    wheelColl->StaticFriction = wheel->StaticFriction * body->Centre.StaticFriction;
    wheelColl->KineticFriction = wheel->KineticFriction * body->Centre.KineticFriction;
    wheelColl->Restitution = car->Spring[iWheel].Restitution;
#else
    wheelColl->Grip = wheel->Grip;
    wheelColl->StaticFriction = wheel->StaticFriction;
    wheelColl->KineticFriction = wheel->KineticFriction;
    wheelColl->Restitution = ZERO;
#endif
    wheelColl->Material = NULL;
    wheelColl->CollPoly = NULL;
    bodyColl->Grip = wheelColl->Grip;
    bodyColl->StaticFriction = wheelColl->StaticFriction;
    bodyColl->KineticFriction = wheelColl->KineticFriction;
    bodyColl->Restitution = wheelColl->Restitution;
    bodyColl->Material = NULL;
    bodyColl->CollPoly = NULL;

    AddWheelColl(car, wheelColl);
    AddBodyColl(body, bodyColl);

    return 1;
}

int DetectWheelConvexColls(CAR *car, int iWheel, NEWBODY *body)
{
    int iSkin;
    COLLINFO_WHEEL  *wheelColl;
    COLLINFO_BODY   *bodyColl;
    WHEEL *wheel = &car->Wheel[iWheel];
    int nColls = 0;

    Assert(body != NULL);
    Assert(car != NULL);
    Assert((iWheel >= 0) && (iWheel < CAR_NWHEELS));

    // Bounding box test
#ifndef _PSX
    if (!BBTestYXZ(&wheel->BBox, &body->CollSkin.BBox)) return 0;
#else
    if (!SphereBBTest(&wheel->CentrePos, wheel->Radius, &body->CollSkin.BBox)) return 0;
#endif

    // Check against each convex hull
    for (iSkin = 0; iSkin < body->CollSkin.NConvex; iSkin++) {

        // Set up the collision info
        if ((wheelColl = NextWheelCollInfo()) == NULL) return nColls;
        if ((bodyColl = NextBodyCollInfo(body)) == NULL) return nColls;

        // Check for collision
        if (!SphereConvex(&wheel->CentrePos, wheel->Radius, &body->CollSkin.WorldConvex[iSkin], &wheelColl->Pos, &wheelColl->Plane, &wheelColl->Depth)) {
            continue;
        }

        // Collision Occurred
        
        bodyColl->Body1 = body;
        bodyColl->Body2 = car->Body;
        wheelColl->Car = car;
        wheelColl->IWheel = iWheel;
        wheelColl->Body2 = body;
        SetVecZero(&wheelColl->WorldPos);   // DODGY....
        SetVecZero(&bodyColl->WorldPos);    // DODGY....

        // World position of the collision
        //VecPlusScalarVec(&wheel->CentrePos, -(wheel->Radius + wheelColl->Depth), PlaneNormal(&wheelColl->Plane), &wheelColl->WorldPos);
        //SetVecZero(&wheelColl->WorldPos);
        //CopyVec(&wheelColl->WorldPos, &bodyColl->WorldPos);

        // Calculate the relative collision points for response
        //VecMinusVec(&bodyColl->WorldPos, &body->Centre.Pos, &bodyColl->Pos1);
        VecMinusVec(&wheelColl->Pos, &body->Centre.Pos, &bodyColl->Pos1);
        VecMinusEqVec(&wheelColl->Pos, &car->Body->Centre.Pos);
        CopyVec(&wheelColl->Pos, &bodyColl->Pos2);
        CopyVec(&bodyColl->Pos1, &wheelColl->Pos2);

        // Calculate velocity
        VecCrossVec(&car->Body->AngVel, &wheelColl->Pos, &wheelColl->Vel);
        VecPlusEqVec(&wheelColl->Vel, &car->Body->Centre.Vel);
        VecCrossVec(&body->AngVel, &bodyColl->Pos1, &bodyColl->Vel);
        VecPlusEqVec(&bodyColl->Vel, &body->Centre.Vel);
        VecMinusEqVec(&bodyColl->Vel, &wheelColl->Vel);
        CopyVec(&bodyColl->Vel, &wheelColl->Vel);
        NegateVec(&wheelColl->Vel);

        FlipPlane(&wheelColl->Plane, &bodyColl->Plane);
        bodyColl->Depth = wheelColl->Depth;
        bodyColl->Time = ZERO;

        // Make sure that the wheel is not already travelling away from the surface
        //VecPlusScalarVec(&wheelColl->Vel, wheel->Vel, &car->Body->Centre.WMatrix.mv[U], &tmpVec);
        //if (VecDotVec(&tmpVec, PlaneNormal(&wheelColl->Plane)) > ZERO) return;
#ifndef _PSX
        wheelColl->Grip = MulScalar(wheel->Grip, body->Centre.Grip);
        wheelColl->StaticFriction = MulScalar(wheel->StaticFriction, body->Centre.StaticFriction);
        wheelColl->KineticFriction = MulScalar(wheel->KineticFriction, body->Centre.KineticFriction);
        wheelColl->Restitution = car->Spring[iWheel].Restitution;
#else
        wheelColl->Grip = wheel->Grip;
        wheelColl->StaticFriction = wheel->StaticFriction;
        wheelColl->KineticFriction = wheel->KineticFriction;
        wheelColl->Restitution = ZERO;
#endif
        wheelColl->Material = NULL;
        wheelColl->CollPoly = NULL;
        bodyColl->Grip = wheelColl->Grip;
        bodyColl->StaticFriction = wheelColl->StaticFriction;
        bodyColl->KineticFriction = wheelColl->KineticFriction;
        bodyColl->Restitution = wheelColl->Restitution;
        bodyColl->Material = NULL;
        bodyColl->CollPoly = NULL;

        AddBodyColl(body, bodyColl);
        AddWheelColl(car, wheelColl);
        nColls++;

    }

    return nColls;
}


/////////////////////////////////////////////////////////////////////
//
// CarCarColls:
//
/////////////////////////////////////////////////////////////////////

int DetectCarCarColls(CAR *car1, CAR *car2)
{
    int nColls = 0;

    // Check all car parts against other car's parts
    if (GameSettings.PlayMode != PLAYMODE_SIMULATION) 
    {
        nColls += DetectCarBodyCarBodyColls(car1, car2);
    } 
    else 
    {
        nColls += DetectBodyBodyColls(car1->Body, car2->Body);
        nColls += DetectWheelBodyColls(car1, car2->Body);
        nColls += DetectWheelBodyColls(car2, car1->Body);
        nColls += DetectWheelWheelColls(car1, car2);
    }

    return nColls;
}

////////////////////////////////////////////////////////////////
//
// Detect Car body to Car body collisions in arcade mode
//
////////////////////////////////////////////////////////////////
int DetectCarBodyCarBodyColls(CAR *car1, CAR *car2)
{
    REAL    dRLen, d, min;
    VEC dR;
    COLLINFO_BODY   *bodyColl1, *bodyColl2;
    SPHERE *sphere1, *sphere2;
    NEWBODY *body1, *body2;


    Assert(car1 != NULL);
    Assert(car2 != NULL);

    body1 = car1->Body;
    body2 = car2->Body;


    // Work out which two spheres are closest together
    sphere1 = &car1->CollSphere[0];
    sphere2 = &car2->CollSphere[0];
    VecMinusVec(&car1->CollSphere[0].Pos, &car2->CollSphere[0].Pos, &dR);
    min = Max(Max(abs(dR.v[X]), abs(dR.v[Y])), abs(dR.v[Z]));

    if (car1->NBodySpheres > 1) {
        VecMinusVec(&car1->CollSphere[1].Pos, &car2->CollSphere[0].Pos, &dR);
        d = Max(Max(abs(dR.v[X]), abs(dR.v[Y])), abs(dR.v[Z]));
        if (d < min) {
            min = d;
            sphere1 = &car1->CollSphere[1];
        }

        if (car2->NBodySpheres > 1) {
            VecMinusVec(&car1->CollSphere[1].Pos, &car2->CollSphere[1].Pos, &dR);
            d = Max(Max(abs(dR.v[X]), abs(dR.v[Y])), abs(dR.v[Z]));
            if (d < min) {
                min = d;
                sphere1 = &car1->CollSphere[1];
                sphere2 = &car2->CollSphere[1];
            }
        }
    }

    if (car2->NBodySpheres > 1) {
        VecMinusVec(&car1->CollSphere[0].Pos, &car2->CollSphere[1].Pos, &dR);
        d = Max(Max(abs(dR.v[X]), abs(dR.v[Y])), abs(dR.v[Z]));
        if (d < min) {
            min = d;
            sphere1 = &car1->CollSphere[0];
            sphere2 = &car2->CollSphere[1];
        }
    }

    // Quick test
    if (min > sphere1->Radius + sphere2->Radius) return 0;

    // Relative position and separation
    VecMinusVec(&sphere1->Pos, &sphere2->Pos, &dR);
    dRLen = VecDotVec(&dR, &dR);

    // Check for collision
    if (dRLen > MulScalar((sphere1->Radius + sphere2->Radius),(sphere1->Radius + sphere2->Radius))) return 0;
    dRLen = (REAL)sqrt(dRLen);

    if ((bodyColl1 = NextBodyCollInfo(body1)) == NULL) return 0;
    AddBodyColl(body1, bodyColl1);
    if ((bodyColl2 = NextBodyCollInfo(body2)) == NULL) {
        RemoveBodyColl(body1, bodyColl1);
        return 0;
    }
    AddBodyColl(body2, bodyColl2);

    bodyColl1->Body1 = body1;
    bodyColl1->Body2 = body2;
    bodyColl2->Body1 = body2;
    bodyColl2->Body2 = body1;

    bodyColl1->Depth = (dRLen - sphere1->Radius - sphere2->Radius) / 2;
    bodyColl2->Depth = bodyColl1->Depth;

    // Collision plane
    if (dRLen > SMALL_REAL) {
        CopyVec(&dR, PlaneNormal(&bodyColl1->Plane));
        VecDivScalar(PlaneNormal(&bodyColl1->Plane), dRLen);
        bodyColl1->Time = ONE - DivScalar(bodyColl1->Depth, dRLen);
    } else {
        SetVec(PlaneNormal(&bodyColl1->Plane), ONE, ZERO, ZERO);
        bodyColl1->Time = ONE;
    }
    FlipPlane(&bodyColl1->Plane, &bodyColl2->Plane);
    bodyColl2->Time = bodyColl1->Time;

    // Always act through centre of mass
    SetVecZero(&bodyColl1->Pos1);
    SetVecZero(&bodyColl1->Pos2);
    SetVecZero(&bodyColl2->Pos1);
    SetVecZero(&bodyColl2->Pos2);
    VecPlusScalarVec(&sphere1->Pos, HALF, &dR, &bodyColl1->WorldPos);
    CopyVec(&bodyColl1->WorldPos, &bodyColl2->WorldPos);
    
    // collsion velocity
    VecCrossVec(&body1->AngVel, &bodyColl1->Pos1, &bodyColl1->Vel);
    VecPlusEqVec(&bodyColl1->Vel, &body1->Centre.Vel);
    VecCrossVec(&body2->AngVel, &bodyColl2->Pos1, &bodyColl2->Vel);
    VecPlusEqVec(&bodyColl2->Vel, &body2->Centre.Vel);
    VecMinusEqVec(&bodyColl1->Vel, &bodyColl2->Vel);
    CopyVec(&bodyColl1->Vel, &bodyColl2->Vel);
    NegateVec(&bodyColl2->Vel);


    // Other stuff
    bodyColl1->Grip = ZERO;//MulScalar(body1->Centre.Grip, body2->Centre.Grip);
    bodyColl1->StaticFriction = ZERO;//MulScalar(body1->Centre.StaticFriction, body2->Centre.StaticFriction);
    bodyColl1->KineticFriction = ZERO;//MulScalar(body1->Centre.KineticFriction, body2->Centre.KineticFriction);
    bodyColl1->Restitution = ZERO;//MulScalar(body1->Centre.Hardness, body2->Centre.Hardness);
    bodyColl1->Material = NULL;
    bodyColl1->CollPoly = NULL;
    
    bodyColl2->Grip = ZERO;//bodyColl1->Grip;
    bodyColl2->StaticFriction = ZERO;//bodyColl1->StaticFriction;
    bodyColl2->KineticFriction = ZERO;//bodyColl1->KineticFriction;
    bodyColl2->Restitution = ZERO;//bodyColl1->Restitution;
    bodyColl2->Material = NULL;
    bodyColl2->CollPoly = NULL;

    return 1;

}

/////////////////////////////////////////////////////////////////////
//
// DetectWheelBodyColls:
//
/////////////////////////////////////////////////////////////////////

int DetectWheelBodyColls(CAR *car, NEWBODY *body) 
{
    int     iWheel;
    WHEEL   *wheel;
    int nColls = 0;

    // Check for each wheel
    for (iWheel = 0; iWheel < CAR_NWHEELS; iWheel++) {
        wheel = &car->Wheel[iWheel];
        if (!IsWheelPresent(wheel)) continue;

        if (IsBodyConvex(body)) {
            nColls += DetectWheelConvexColls(car, iWheel, body);
        } 
        else if (IsBodySphere(body)) {
            nColls += DetectWheelSphereColls(car, iWheel, body);
        }
        else if (IsBodyPoly(body)) {
            nColls += DetectWheelPolyColls(car, iWheel, body);
        }
    }

    return nColls;
}


////////////////////////////////////////////////////////////////
//
// DetectWheelWheelColls:
//
////////////////////////////////////////////////////////////////

int DetectWheelWheelColls(CAR *car1, CAR *car2)
{
    register int iWheel1, iWheel2;
    VEC dR;
    REAL    dRLen;
    register COLLINFO_WHEEL *wheelColl;
    register COLLINFO_WHEEL *wheelColl2;
    WHEEL   *wheel1;
    WHEEL   *wheel2;
    int nColls = 0;


    for (iWheel1 = 0; iWheel1 < CAR_NWHEELS; iWheel1++) {
        wheel1 = &car1->Wheel[iWheel1];
        //if (!IsWheelPresent(wheel1)) continue;

        for (iWheel2 = 0; iWheel2 < CAR_NWHEELS; iWheel2++) {
            wheel2 = &car2->Wheel[iWheel2];
            //if (!IsWheelPresent(wheel2)) continue;

            // Bounding box test
#ifndef _PSX
            if (!BBTestYXZ(&wheel1->BBox, &wheel2->BBox)) continue;
#endif

            //Get relative position
            VecMinusVec(&wheel1->CentrePos, &wheel2->CentrePos, &dR);

            dRLen = VecDotVec(&dR, &dR);

            // Check for collision
            if (dRLen > MulScalar(wheel1->Radius + wheel2->Radius, wheel1->Radius + wheel2->Radius)) continue;

            // Set up the collision info
            if ((wheelColl = NextWheelCollInfo()) == NULL) return nColls;
            AddWheelColl(car1, wheelColl);
            if ((wheelColl2 = NextWheelCollInfo()) == NULL) {
                RemoveWheelColl(car1, wheelColl);
                return nColls;
            }
            AddWheelColl(car2, wheelColl2);

            dRLen = (REAL)sqrt(dRLen);

            // Collision Occurred
            wheelColl2->Car = car2;
            wheelColl2->IWheel = iWheel2;
            wheelColl2->Body2 = car2->Body;
            wheelColl->Car = car1;
            wheelColl->IWheel = iWheel1;
            wheelColl->Body2 = car1->Body;
            SetVecZero(&wheelColl->WorldPos);   // DODGY....
            SetVecZero(&wheelColl2->WorldPos);  // DODGY....

            wheelColl2->Depth = dRLen - wheel1->Radius - wheel2->Radius;
            wheelColl->Depth = wheelColl2->Depth;

            // Collision plane
            if (dRLen > SMALL_REAL) {
                CopyVec(&dR, PlaneNormal(&wheelColl->Plane));
                VecDivScalar(PlaneNormal(&wheelColl->Plane), dRLen);
                wheelColl->Time = ONE - DivScalar(wheelColl->Depth, dRLen);
            } else {
                SetVec(PlaneNormal(&wheelColl->Plane), ONE, ZERO, ZERO);
                wheelColl->Time = ONE;
            }
            FlipPlane(&wheelColl->Plane, &wheelColl2->Plane);
            wheelColl2->Time = wheelColl->Time;

            // Calculate the collision points for response
            VecEqScalarVec(&wheelColl->Pos, wheel1->Radius, PlaneNormal(&wheelColl->Plane));
            VecPlusEqVec(&wheelColl->Pos, &wheel1->CentrePos);
            VecMinusEqVec(&wheelColl->Pos, &car1->Body->Centre.Pos);

            VecEqScalarVec(&wheelColl2->Pos, wheel2->Radius, PlaneNormal(&wheelColl2->Plane));
            VecPlusEqVec(&wheelColl2->Pos, &wheel2->CentrePos);
            VecMinusEqVec(&wheelColl2->Pos, &car2->Body->Centre.Pos);

            VecPlusScalarVec(&wheel1->CentrePos, HALF, &dR, &wheelColl->WorldPos);
            CopyVec(&wheelColl->WorldPos, &wheelColl2->WorldPos);

            // Calculate velocity
            VecCrossVec(&car1->Body->AngVel, &wheelColl->Pos, &wheelColl->Vel);
            VecPlusEqVec(&wheelColl->Vel, &car1->Body->Centre.Vel);

            VecCrossVec(&car2->Body->AngVel, &wheelColl2->Pos, &wheelColl2->Vel);
            VecPlusEqVec(&wheelColl2->Vel, &car2->Body->Centre.Vel);

            VecMinusEqVec(&wheelColl2->Vel, &wheelColl->Vel);
            CopyVec(&wheelColl2->Vel, &wheelColl->Vel);
            NegateVec(&wheelColl->Vel);

#ifndef _PSX
            wheelColl->Grip = MulScalar(wheel1->Grip, wheel2->Grip);
            wheelColl->StaticFriction = MulScalar(wheel1->StaticFriction, wheel2->StaticFriction);
            wheelColl->KineticFriction = MulScalar(wheel1->KineticFriction, wheel2->KineticFriction);
#else
            wheelColl->Grip = wheel1->Grip;
            wheelColl->StaticFriction = wheel1->StaticFriction;
            wheelColl->KineticFriction = wheel1->KineticFriction;
#endif
            wheelColl->Restitution = car1->Spring[iWheel1].Restitution;
            wheelColl->Material = NULL;
            wheelColl->CollPoly = NULL;
            wheelColl2->Grip = wheelColl->Grip;
            wheelColl2->StaticFriction = wheelColl->StaticFriction;
            wheelColl2->KineticFriction = wheelColl->KineticFriction;
            wheelColl2->Restitution = car2->Spring[iWheel2].Restitution;
            wheelColl2->Material = NULL;
            wheelColl2->CollPoly = NULL;

            nColls++;
        }
    }

    return nColls;
}




/////////////////////////////////////////////////////////////////////
//
// CarBodyColls:
//
/////////////////////////////////////////////////////////////////////

int DetectCarBodyColls(CAR *car, NEWBODY *body)
{
    int nColls = 0;

    // Check car parts against body
    nColls += DetectBodyBodyColls(car->Body, body);
    nColls += DetectWheelBodyColls(car, body);

    return nColls;

}


//$ADDITION_BEGIN(jedl)
// Use base model directory as name of compiled xbr file. 
// For example, D:/cars/rc/body.m becomes D:/cars/rc/rc.xbr
CHAR* GetCarResourcePath( CHAR path_xbr[], CAR_INFO *pCarInfo )
{
    CHAR drive[_MAX_DRIVE];
    CHAR dir[_MAX_DIR];
    CHAR name[_MAX_PATH];
    CHAR ext[_MAX_EXT];
    _splitpath( pCarInfo->ModelFile[0], drive, dir, name, ext );
    dir[strlen(dir) - 1] = 0;   // get rid of ending slash
    CHAR *strDir = strrchr(dir, '\\') + 1;  // get directory name
    _makepath( path_xbr, drive, dir, strDir, ".xbr");
    return path_xbr;
}
//$ADDITION_END


////////////////////////////////
// load one car model + tpage //
////////////////////////////////
#ifdef _PC

void LoadOneCarModelSet(struct PlayerStruct *player, long car)
{
    CAR_INFO *ci;
    CAR_MODEL *cm = &player->carmodels;
    COLLSKIN_INFO *collinfo = &cm->CollSkin;
    CONVEX *pSkin;
    FILE *fp;
    long i, iSkin, iFace;
    char tPage;

// get car info

    car %= NCarTypes;
    ci = &CarInfo[car];

//$ADDITION_BEGIN(jedl) - start loading car resources.
    // $TODO Convert all the car info to a binary file to be loaded quickly.
    // Check to see if there's a compiled version of the car.
    if (ci->m_pXBR != NULL)
    {
        // Keep track of the number of cars using this resource bundle
        // $TODO Add refcount to XBResource class?
        Assert(ci->m_RefCountXBR > 0);
        ci->m_RefCountXBR++;
    }
    else
    {
        CHAR path_xbr[_MAX_PATH];
        GetCarResourcePath( path_xbr, ci );
        DWORD FileAttrib = GetFileAttributes( path_xbr );
        if (FileAttrib != 0xffffffff)
        {
            // Start loading resources from file
            ci->m_pXBR = new XBResource;
            Assert(ci->m_pXBR != NULL);
            Assert(ci->m_RefCountXBR == 0);
            ci->m_RefCountXBR = 1;
            ci->m_pXBR->StartLoading( path_xbr );

            // Busy-wait until resources are loaded.  This should instead
            // be overlapped with the car selection or screens displaying
            // network play state, etc.
            LOADINGSTATE LoadingState = ci->m_pXBR->CurrentLoadingState();
            while ( LoadingState != LOADING_DONE )
            {
                Assert(LoadingState != LOADING_FAILED );
                ci->m_pXBR->PollLoadingState();
                LoadingState = ci->m_pXBR->CurrentLoadingState();
            }
        }
    }
    // Zero out effect pointers
    for (i = 0 ; i < MAX_CAR_MODEL_TYPES; i++)
        for (int j = 0; j < MAX_CAR_LOD; j++)
            cm->Model[i][j].m_pEffect = NULL;
//$ADDITION_END

// set parts flags

    cm->BodyPartsFlag = 0;
    cm->WheelPartsFlag[FL] = cm->WheelPartsFlag[FR] = cm->WheelPartsFlag[BL] = cm->WheelPartsFlag[BR] = 0;

// Load TPage

    if (car == CARID_TROLLEY)
    {
        LoadTextureClever("D:\\levels\\market1\\trolley.bmp", TPAGE_MISC2, 256, 256, 0, FxTextureSet, TRUE); //$MODIFIED: added "D:\\" at start
#pragma message( "JedL needs to review this -- do we call LoadTextureGPU here?" )
    }
    else if (GameSettings.GameType != GAMETYPE_CLOCKWORK && !(GameSettings.GameType == GAMETYPE_REPLAY && StartDataStorage.GameType == GAMETYPE_CLOCKWORK) && strlen(ci->TPageFile))
    {
//$ADDITION_BEGIN(jedl) - load packed texture resource
        if (ci->m_pXBR != NULL)
            LoadTextureGPU(NULL, ci->TPageFile, TPAGE_CAR_START + (int)player->Slot, ci->m_pXBR);
        else
//$ADDITION_END
            LoadTextureClever(ci->TPageFile, TPAGE_CAR_START + (char)player->Slot, 256, 256, 0, CarTextureSet, TRUE);
    }

// Load models

    for (i = 0 ; i < MAX_CAR_MODEL_TYPES; i++)
    {
        if (strlen(ci->ModelFile[i]))
        {
            if (car == CARID_TROLLEY)
                tPage = TPAGE_MISC2;

            else if (i == CAR_MODEL_AERIAL_SEC1 || i == CAR_MODEL_AERIAL_TOP1)
                tPage = TPAGE_FX1;

            else if (GameSettings.GameType == GAMETYPE_CLOCKWORK || (GameSettings.GameType == GAMETYPE_REPLAY && StartDataStorage.GameType == GAMETYPE_CLOCKWORK))
                tPage = TPAGE_CAR_START;

            else
                tPage = TPAGE_CAR_START + (char)player->Slot;

            LoadModel(ci->ModelFile[i], cm->Model[i], tPage, MAX_CAR_LOD, LOADMODEL_FORCE_TPAGE, CurrentLevelInfo.ModelRGBper);
        }
        else
        {
            cm->Model[i]->AllocPtr = NULL;
        }
    }

// Set Env map RGB

    cm->EnvRGB = ci->EnvRGB;

// Load Collision Skin

    if (ci->CollFile && ci->CollFile[0])
    {
        if ((fp = fopen(ci->CollFile, "rb")) != NULL) 
        {

            // Load the convex hulls
            if ((collinfo->Convex = LoadConvex(fp, &collinfo->NConvex)) != NULL)
            {
                collinfo->CollType = BODY_COLL_CONVEX;

                // Move the collision skins to centre on CoM
                for (iSkin = 0; iSkin < collinfo->NConvex; iSkin++) {
                    pSkin = &collinfo->Convex[iSkin];

                    // collision planes
                    for (iFace = 0; iFace < pSkin->NFaces; iFace++) {
                        MovePlane(&pSkin->Faces[iFace], &ci->CoMOffset);
                    }
                }

            }

            // Load the spheres
            if ((collinfo->Sphere = LoadSpheres(fp, &collinfo->NSpheres)) != NULL)
            {
                // Move the spheres to centre on CoM
                for (iSkin = 0; iSkin < collinfo->NSpheres; iSkin++) {
                    // Position
                    VecPlusEqVec(&collinfo->Sphere[iSkin].Pos, &ci->CoMOffset);
                }
            }

            MakeTightLocalBBox(collinfo);

            fclose(fp);
        }
    }
    else
    {
        collinfo->NConvex = 0;
        collinfo->Convex = NULL;
        collinfo->NSpheres = 0;
        collinfo->Sphere = NULL;
        SetBBox(&collinfo->BBox, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO);
    }

//$ADDITION_BEGIN(jedl) - get effect references
    if( ci->m_pXBR != NULL )
    {
        for( i = 0 ; i < MAX_CAR_MODEL_TYPES ; i++ )
        {
            if( strlen(ci->ModelFile[i]) )
                LoadModelGPU(ci->ModelFile[i], cm->Model[i], MAX_CAR_LOD, ci->m_pXBR);
            else
                cm->Model[i]->m_pEffect = NULL;
        }
    }
    else
    {
        // Export models in XDX resource format
        extern HRESULT ExportCar(CAR_INFO *pCarInfo, CAR_MODEL *pCarModel, INT Tpage);
        ExportCar(ci, cm, TPAGE_CAR_START + (char)player->Slot);
    }
//$ADDITION_END

// Setup body

    cm->Body = cm->Model[ci->Body.ModelNum];
    CopyVec(&ci->Body.Offset, &cm->OffBody);

// Setup wheels

    for (i = 0; i < 4; i++)
    {
        if (ci->Wheel[i].ModelNum != CAR_MODEL_NONE) {
            cm->Wheel[i] = cm->Model[ci->Wheel[i].ModelNum];
            CopyVec(&ci->Wheel[i].Offset1, &cm->OffWheel[i]);
            CopyVec(&ci->Wheel[i].Offset2, &cm->OffWheelColl[i])
            cm->WheelRad[i] = ci->Wheel[i].Radius;
            cm->WheelPartsFlag[i] |= CAR_MODEL_WHEEL;
        }
    }

// Set up spinner

    if (ci->Spinner.ModelNum != CAR_MODEL_NONE)
    {
        cm->BodyPartsFlag |= CAR_MODEL_SPINNER;
        cm->Spinner = cm->Model[ci->Spinner.ModelNum];
        CopyVec(&ci->Spinner.Offset, &cm->OffSpinner);
    }

// Setup aerial models

    if (ci->Aerial.SecModelNum != CAR_MODEL_NONE && ci->Aerial.TopModelNum != CAR_MODEL_NONE)
    {
        cm->BodyPartsFlag |= CAR_MODEL_AERIAL;
        cm->Aerial[0] = cm->Model[ci->Aerial.SecModelNum];
        cm->Aerial[1] = cm->Model[ci->Aerial.TopModelNum];
        CopyVec(&ci->Aerial.Offset, &cm->OffAerial); 
        CopyVec(&ci->Aerial.Direction, &cm->DirAerial); 
        cm->AerialLen = ci->Aerial.SecLen;
    }

// setup springs / axles / pins

    for (i = 0; i < 4; i++)
    {

// Setup springs

        if (ci->Spring[i].ModelNum != CAR_MODEL_NONE)
        {
            cm->Spring[i] = cm->Model[ci->Spring[i].ModelNum];
            CopyVec(&ci->Spring[i].Offset, &cm->OffSpring[i]);
            cm->WheelPartsFlag[i] |= CAR_MODEL_SPRING;
            cm->SpringLen[i] = ci->Spring[i].Length;
        }
    
// Setup axles

        if (ci->Axle[i].ModelNum != CAR_MODEL_NONE)
        {
            cm->Axle[i] = cm->Model[ci->Axle[i].ModelNum];
            CopyVec(&ci->Axle[i].Offset, &cm->OffAxle[i]);
            cm->AxleLen[i] = ci->Axle[i].Length;
            cm->WheelPartsFlag[i] |= CAR_MODEL_AXLE;
        }

// Setup pins

        if (ci->Pin[i].ModelNum != CAR_MODEL_NONE)
        {
            cm->Pin[i] = cm->Model[ci->Pin[i].ModelNum];
            CopyVec(&ci->Pin[i].Offset, &cm->OffPin[i]);
            cm->PinLen[i] = ci->Pin[i].Length;
            cm->WheelPartsFlag[i] |= CAR_MODEL_PIN;
        }
    }
}
#endif

//--------------------------------------------------------------------------------------------------------------------------

////////////////////////////////
// free one car model + tpage //
////////////////////////////////
#ifdef _PC

void FreeOneCarModelSet(struct PlayerStruct *player)
{
    long i;

//$ADDITION_BEGIN(jedl) - decrement resource reference count
    CAR_INFO *pCarInfo = &CarInfo[player->car.CarType % NCarTypes];
    if( pCarInfo->m_pXBR != NULL )
    {
        Assert(pCarInfo->m_RefCountXBR > 0);
        pCarInfo->m_RefCountXBR--;
        if( pCarInfo->m_RefCountXBR == 0 )
        {
            delete pCarInfo->m_pXBR;
            pCarInfo->m_pXBR = NULL;
        }
    }
//$ADDITION_END

// free models

    for (i = 0 ; i < MAX_CAR_MODEL_TYPES ; i++)
    {
        if (player->carmodels.Model[i]->AllocPtr)
        {
            FreeModel(player->carmodels.Model[i], MAX_CAR_LOD);
        }
    }

// free texture

    if (player->car.CarType == CARID_TROLLEY)
        FreeOneTexture(TPAGE_MISC2);

    else if (GameSettings.GameType != GAMETYPE_CLOCKWORK && !(GameSettings.GameType == GAMETYPE_REPLAY && StartDataStorage.GameType == GAMETYPE_CLOCKWORK))
        FreeOneTexture(TPAGE_CAR_START + (char)player->Slot);

// free coll skin

    DestroyConvex(player->car.Models->CollSkin.Convex, player->car.Models->CollSkin.NConvex);
    player->car.Models->CollSkin.Convex = NULL;
    player->car.Body->CollSkin.Convex = NULL;
    player->car.Models->CollSkin.NConvex = 0;

    DestroySpheres(player->car.Models->CollSkin.Sphere);
    player->car.Models->CollSkin.Sphere = NULL;
    player->car.Body->CollSkin.Sphere = NULL;
    player->car.Models->CollSkin.NSpheres = 0;
}
#endif


//--------------------------------------------------------------------------------------------------------------------------

/////////////////////////////////////////////////////////////////////
//
// CarAccTimings: count the best acceleration times of the cars
// from 0-20 and 0-30mph
//
/////////////////////////////////////////////////////////////////////

#ifndef _PSX

void CarAccTimings(CAR *car)
{
    REAL carVel;

    carVel = OGU2MPH_SPEED * VecLen(&car->Body->Centre.Vel);

    // See if timing should start
    if (carVel < Real(0.01)) {
        car->Timing0to15 = TRUE;
        car->Timing0to25 = TRUE;
        car->Current0to15 = ZERO;
        car->Current0to25 = ZERO;
        return;
    }

    // Update timers
    if (car->Timing0to15) {
        car->Current0to15 += TimeStep;
    }
    if (car->Timing0to25) {
        car->Current0to25 += TimeStep;
    }

    // See if the speeds have been reached
    if (car->Timing0to15 && carVel > 15) {
        if ((car->Current0to15 < car->Best0to15) || (car->Best0to15 < ZERO)) {
            car->Best0to15 = car->Current0to15;
        }
        car->Timing0to15 = FALSE;
    }
    if (car->Timing0to25 && carVel > 25) {
        if ((car->Current0to25 < car->Best0to25) || (car->Best0to25 < ZERO)) {
            car->Best0to25 = car->Current0to25;
        }
        car->Timing0to25 = FALSE;
    }

}


#endif

/////////////////////////////////////////////////////////////////////
//
// CarDownForce:
//
/////////////////////////////////////////////////////////////////////

#define WFL_CONTACT 1
#define WFR_CONTACT 2
#define WBL_CONTACT 4
#define WBR_CONTACT 8
#if USE_DEBUG_ROUTINES
VEC DEBUG_DownForce;
#endif

void CarDownForce(CAR *car)
{
#ifndef _PSX
    int ii;
    REAL vel, mod;
    VEC downForce = {ZERO, ZERO, ZERO};
    long contact = 0;

    // Set up the wheel contact flags
    for (ii = 0; ii < CAR_NWHEELS; ii++) {
        if (IsWheelPresent(&car->Wheel[ii]) && IsWheelInContact(&car->Wheel[ii])) {
            contact |= 1 << ii;
        }
    }

    // if all wheel in contact, no need to continue
    if (contact == (WFL_CONTACT | WFR_CONTACT | WBL_CONTACT | WBR_CONTACT)) return;

    //mod = ONE + HALF - abs(VecDotVec(&car->Body->Centre.WMatrix.mv[U], &UpVec));
    mod = ONE + HALF - abs(car->Body->Centre.WMatrix.m[UY]);
    if (mod > ONE) mod = ONE;

    // See if car on two wheel on left side
    if (contact == (WFL_CONTACT | WBL_CONTACT)) {
        vel = MulScalar3(TimeStep, mod, VecDotVec(&car->Body->Centre.Vel, &car->Body->Centre.WMatrix.mv[L]));
        //vel = MulScalar(vel, mod);
        VecPlusEqScalarVec(&downForce, MulScalar(car->DownForceMod, vel), &car->Body->Centre.WMatrix.mv[U]);
    }

    // See if car on two wheel on right side
    if (contact == (WFR_CONTACT | WBR_CONTACT)) {
        vel = MulScalar3(TimeStep, mod, VecDotVec(&car->Body->Centre.Vel, &car->Body->Centre.WMatrix.mv[L]));
        //vel = MulScalar(vel, mod);
        VecPlusEqScalarVec(&downForce, MulScalar(car->DownForceMod, vel), &car->Body->Centre.WMatrix.mv[U]);
    }

    VecPlusEqVec(&car->Body->Centre.Impulse, &downForce);

#if USE_DEBUG_ROUTINES
    CopyVec(&downForce, &DEBUG_DownForce);
#endif

#endif
}


/////////////////////////////////////////////////////////////////////
//
// AddWheelColl:
//
/////////////////////////////////////////////////////////////////////

COLLINFO_WHEEL *AddWheelColl(CAR *car, COLLINFO_WHEEL *newHead)
{
    COLLINFO_WHEEL *oldHead = car->WheelCollHead;

    car->WheelCollHead = newHead;
    newHead->Next = oldHead;
    newHead->Prev = NULL;

    if (oldHead != NULL) {
        oldHead->Prev = newHead;
    }

    car->NWheelColls++;
    COL_NWheelColls++;

    return newHead;
}

/////////////////////////////////////////////////////////////////////
//
// RemoveWheelColl:
//
/////////////////////////////////////////////////////////////////////

void RemoveWheelColl(CAR *car, COLLINFO_WHEEL *collInfo)
{
    Assert(collInfo != NULL);

    if (collInfo->Next != NULL) {
        (collInfo->Next)->Prev = collInfo->Prev;
    }

    if (collInfo->Prev != NULL) {
        (collInfo->Prev)->Next = collInfo->Next;
    } else {
        car->WheelCollHead = collInfo->Next;
    }

    car->NWheelColls--;
    COL_NWheelDone++;

    Assert((car->NWheelColls == 0)? (car->WheelCollHead == NULL): (car->WheelCollHead != NULL));
    Assert(COL_NWheelColls - COL_NWheelDone >= 0);
}


////////////////////////////////////////////////////////////////
//
// SetCarStats: calculate car handling statistics for display
// on front end
//
////////////////////////////////////////////////////////////////

void CalcCarStats()
{
    int iCar;
    
    for (iCar = 0; iCar < NCarTypes; iCar++) {

        // Weight
        CarInfo[iCar].Weight = CarInfo[iCar].Body.Mass;

        // Transmission
        if (CarInfo[iCar].Wheel[0].IsPowered && CarInfo[iCar].Wheel[2].IsPowered) {
            CarInfo[iCar].Trans = CAR_TRANS_4WD;
        } else if (CarInfo[iCar].Wheel[0].IsPowered) {
            CarInfo[iCar].Trans = CAR_TRANS_FWD;
        } else {
            CarInfo[iCar].Trans = CAR_TRANS_RWD;
        }

    }

}

//////////////////////////
// start car reposition //
//////////////////////////

void StartCarReposition(PLAYER *player)
{

// not if already in the throws

    if (GameSettings.GameType != GAMETYPE_TRIAL && player->car.RepositionTimer)
    {
        return;
    }

// set repos timer

    player->car.RepositionTimer = CAR_REPOS_TIMER;

// set repos flag

    player->car.RepositionHalf = FALSE;

// set car render flag

    player->car.RenderFlag = CAR_RENDER_GHOST;

// set repos handler

    player->ownobj->movehandler = (MOVE_HANDLER)MOV_RepositionCar;

// turn off object colls

    player->car.Body->CollSkin.AllowObjColls = FALSE;

// Set AI state

    player->CarAI.AIState = CAI_S_RACE;
}

#if MSCOMPILER_FUDGE_OPTIMISATIONS
#pragma optimize("", on)
#endif


//$REVISIT - might actually want some of this code, depending on how we implement vibration
/* $REMOVED (tentative!!)
#ifdef _PC
///////////////////////////////////////////////
// UpdatePlayerForceFeedback(PLAYER* pPlayer)
//
///////////////////////////////////////////////
#define FORCEFEEDBACK_FRICTION_MIN      Real(0.05)

void UpdatePlayerForceFeedback(PLAYER* pPlayer)
{
    int     i;
    int     material[4];
    REAL    freqScale;
    REAL    friction, frictionW[4];
    REAL    bumpyness, bumpynessW[4];
    REAL    frequency, frequencyW[4];
    REAL    pan, dur;
    REAL    steerFriction;
    int     period, mag;

    if (RegistrySettings.Joystick != -1)
    {
        LONG            rglDirection[2] = {0,0};

    // Set joystick pointer
        JOYSTICK* pJoy = &Joystick[RegistrySettings.Joystick];
        if (pJoy->forceFeedback.flags == 0)
            return;

    // Is the car in the air ?
        if (pPlayer->car.NWheelFloorContacts == 0)
        {
            // Stop vibration & friction
            SetJoyVibration(pJoy, 0,0,0,1);
            SetJoyConstant(pJoy, 0,0);
            SetJoyFriction(pJoy, FORCEFEEDBACK_FRICTION_MIN);
            pJoy->forceFeedback.durVibration = 0;
            return;
        }

    // Setup car wheel attributes
        freqScale = pPlayer->CarAI.speedCur / (MPH2OGU_SPEED*30);
        friction = 0;
        bumpyness = 0;
        frequency = 0;
        pan = 0;

        for (i= 0; i < 4; i++)
        {
            material[i] = pPlayer->car.Wheel[i].SkidMaterial;

            if (IsWheelInFloorContact(&pPlayer->car.Wheel[i]) && (material[i] >= 0))
            {
                frictionW[i] = COL_MaterialInfo[material[i]].Roughness;
                bumpynessW[i] = COL_CorrugationInfo[COL_MaterialInfo[material[i]].Corrugation].Amp;
                frequencyW[i] = (COL_CorrugationInfo[COL_MaterialInfo[material[i]].Corrugation].Lx +
                                 COL_CorrugationInfo[COL_MaterialInfo[material[i]].Corrugation].Ly) * 0.5f;

                if (IsWheelinOil(&pPlayer->car.Wheel[i]))
                    frictionW[i] *= 0.1f;
            }
            else
            {
                frictionW[i] = 0;
                bumpynessW[i] = 0;
                frequencyW[i] = 0;
            }

            friction += frictionW[i];
            bumpyness += bumpynessW[i];
            frequency += frequencyW[i];
        }

        friction *= Real(0.25) * freqScale;
        friction -= Real(0.2);
        if (friction < 0)   friction = 0;

        bumpyness *= Real(0.25 * 2.0) * freqScale;
        frequency *= Real(0.25) * freqScale;

        pan += bumpyness * 0.05f;
        if (rand() & 1)
            pan = -pan;


    // Set steering wheel friction
        steerFriction = abs((pPlayer->CarAI.speedCur - (pPlayer->CarAI.speedMax * 0.25f)) / (MPH2OGU_SPEED * 20));
        steerFriction = FORCEFEEDBACK_FRICTION_MIN + (steerFriction * ((frictionW[0] + frictionW[1]) * 0.5f));
        SetJoyFriction(pJoy, steerFriction);

    // Setup bang
        if (pPlayer->car.Body->BangMag > (MPH2OGU_SPEED * 0.1f))
        {
            REAL bangMag, bangPan, bangPan1, bangPan2, bangDir;
            
            bangMag = pPlayer->car.Body->BangMag / (MPH2OGU_SPEED * 3);
            bangPan1 = -VecDotVec(PlaneNormal(&pPlayer->car.Body->BangPlane), &pPlayer->car.Body->Centre.WMatrix.mv[R]);
            bangPan2 = -VecDotVec(PlaneNormal(&pPlayer->car.Body->BangPlane), &pPlayer->car.Body->Centre.WMatrix.mv[L]);

            if (bangPan1 < 0)   bangDir = -1;
            else                bangDir = 1;

            if (bangPan1 > bangPan2)    bangPan = bangPan1;
            else                        bangPan = bangPan2;
            dur = abs(bangPan) * 0.5f;

            SetJoyConstant(pJoy, bangDir*bangMag, dur);
        }

        mag     = Int(bumpyness * 3000);
        period  = Int(frequency * 2000);
        dur     = (float)period / 100000;

    // Has the last vibration elapsed ?
        if (pJoy->forceFeedback.durVibration > 0)
        {
            pJoy->forceFeedback.durVibration -= TimeStep;
            return;
        }
        else
        {
            SetJoyVibration(pJoy, mag, pan, period, Int(dur*1000));
            pJoy->forceFeedback.durVibration = dur;
        }
    }
}

///////////////////////////////////////////////
// UpdateReplayForceFeedback(PLAYER* pPlayer)
//
///////////////////////////////////////////////
void UpdateReplayForceFeedback(PLAYER* pPlayer)
{
    if (Joystick[0].forceFeedback.flags)
    {
        SetJoyVibration(&Joystick[0], 0, (float)pPlayer->controls.lastdx / CTRL_RANGE_MAX, 0, 1);
        SetJoyConstant(&Joystick[0], 0,0);
        SetJoyFriction(&Joystick[0], 0);
    }
}

#endif // _PC
$END_REMOVAL */

//////////////////////////////////
// get a car ID from a car name //
//////////////////////////////////

#ifdef _PC

long GetCarTypeFromName(char *name)
{
    long i;

// search for name

    for (i = 0 ; i < NCarTypes ; i++)
    {
        if (!_stricmp(CarInfo[i].Name, name)) //$MODIFIED: changed stricmp to _stricmp
        {
            return i;
        }
    }

// not found, return default car

    return CARID_MYSTERY;
}
#endif


////////////////////////////////////////////////////////////////
//
// SetPlayerToSnail
//
////////////////////////////////////////////////////////////////

#define CAR_SCALE   Real(0.5f)
#ifndef _PSX
void SetPlayerToSnail(CAR *car)
{
    int ii, jj;

    // Shrink car by a scale factor 
    car->DrawScale = CAR_SCALE;

    for (ii = 0; ii < CAR_NWHEELS; ii++) {

        // Wheel part offsets
        VecMulScalar(&car->WheelCentre[ii], CAR_SCALE);
        VecMulScalar(&car->WheelOffset[ii], CAR_SCALE);
        if (CarHasAxle(car, ii))
            VecMulScalar(&car->AxleOffset[ii], CAR_SCALE);
        if (CarHasSpring(car, ii))
            VecMulScalar(&car->SuspOffset[ii], CAR_SCALE);

        // Wheel size
        car->Wheel[ii].Radius *= CAR_SCALE;

        car->Wheel[ii].EngineRatio *= CAR_SCALE;
        car->Wheel[ii].SkidWidth *= CAR_SCALE;

    }

//  car->Body->Centre.Mass *= CAR_SCALE;
//  MatMulScalar(&car->Body->BodyInertia, CAR_SCALE);
//  MatMulScalar(&car->Body->BodyInvInertia, ONE/CAR_SCALE);

    // Body part offsets
    VecMulScalar(&car->AerialOffset, CAR_SCALE);
    VecMulScalar(&car->BodyOffset, CAR_SCALE);
    if (CarHasSpinner(car))
        VecMulScalar(&car->SpinnerOffset, CAR_SCALE);

    // Collision spheres
    for (ii = 0; ii < car->NBodySpheres; ii++) {
        VecMulScalar(&car->BodySphere[ii].Pos, CAR_SCALE);
        car->BodySphere[ii].Radius *= CAR_SCALE;

        VecMulScalar(&car->CollSphere[ii].Pos, CAR_SCALE);
        car->CollSphere[ii].Radius *= CAR_SCALE;
    }

    // Convex hulls
    for (ii = 0; ii < car->Body->CollSkin.NConvex; ii++) {
        for (jj = 0; jj < car->Body->CollSkin.Convex[ii].NFaces; jj++) {
            car->Body->CollSkin.Convex[ii].Faces[jj].v[D] *= CAR_SCALE;
            car->Body->CollSkin.WorldConvex[ii].Faces[jj].v[D] *= CAR_SCALE;
            car->Body->CollSkin.OldWorldConvex[ii].Faces[jj].v[D] *= CAR_SCALE;
        }
    }

    // Balls
    for (ii = 0; ii < car->Body->CollSkin.NSpheres; ii++) {
        VecMulScalar(&car->Body->CollSkin.Sphere[ii].Pos, CAR_SCALE);
        car->Body->CollSkin.Sphere[ii].Radius *= CAR_SCALE;
        VecMulScalar(&car->Body->CollSkin.WorldSphere[ii].Pos, CAR_SCALE);
        car->Body->CollSkin.WorldSphere[ii].Radius *= CAR_SCALE;
        VecMulScalar(&car->Body->CollSkin.OldWorldSphere[ii].Pos, CAR_SCALE);
        car->Body->CollSkin.OldWorldSphere[ii].Radius *= CAR_SCALE;
    }

}
#endif

