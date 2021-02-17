//-----------------------------------------------------------------------------
// File: control.cpp
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"
#ifdef _PC
#include "input.h"
#endif
#include "model.h"
#ifdef _PC
#include "network.h"
#endif
#include "particle.h"
#include "aerial.h"
#include "NewColl.h"
#include "body.h"
#include "car.h"
#ifdef _PC
#include "input.h"
#endif
#include "main.h"
#include "LevelLoad.h"
#include "object.h"
#include "player.h"
#include "geom.h"
#include "util.h"
#include "timing.h"
#include "camera.h"
#include "move.h"
#include "obj_init.h"
#include "aizone.h"
#include "weapon.h"
#include "ai.h"
#include "control.h"
#include "posnode.h"
#include "pickup.h"
#include "cheats.h"
#include "ui_TitleScreen.h"

#define BETTER_CATHCUP

//
// Global function prototypes
//

//void CON_DoPlayerControl(void);
void CON_LocalCarControl(CTRL *Control, OBJECT *CarObj);
void CON_RationaliseControl(unsigned short *digital);

// pickup to weapon table

static long Pickup2WeaponTable[PICKUP_NTYPES] = {
    OBJECT_TYPE_SHOCKWAVE,
    OBJECT_TYPE_FIREWORK,
    OBJECT_TYPE_FIREWORK,
    OBJECT_TYPE_PUTTYBOMB,
    OBJECT_TYPE_WATERBOMB,
    OBJECT_TYPE_ELECTROPULSE,
    OBJECT_TYPE_OILSLICK_DROPPER,
    OBJECT_TYPE_CHROMEBALL,
    OBJECT_TYPE_TURBO,
    OBJECT_TYPE_PICKUP,
    OBJECT_TYPE_GLOBAL,
};

bool NonLinearSteering = TRUE; 

//--------------------------------------------------------------------------------------------------------------------------
void CON_DoPlayerControl(void)
{
    PLAYER  *player;

// loop thru players

    for (player = PLR_PlayerHead; player; player = player->next)
    {
        // Car AI
        if (!ReplayMode)
        {
            // CPU Car AI
            if (player->type == PLAYER_CPU)
            {
                CAI_Process(player);
            }
        }

        // Special for Rotor...
        if (player->car.CarType == CARID_ROTOR) {
            RotorControlsSwitching(&player->car);
        }


// get inputs
    #if 1
        if ((player->type == PLAYER_LOCAL) && (Version == VERSION_DEV) && (gGazzasAICar) && (!Keys[DIK_RCONTROL]))
        {
            if (!ReplayMode)
                CAI_Process(player);
        }
        else if (player->ctrlhandler)
        {
            player->ctrlhandler(player);
        }
    #else
        if (player->ctrlhandler)
        {
            player->ctrlhandler(player);
        }
    #endif

// no power?

        if (player->car.PowerTimer > ZERO) 
        {
            player->controls.dy = 0;
            player->controls.digital &= (CTRL_FIRE | CTRL_PAUSE | CTRL_CHANGECAMERA);
        }

#ifndef _PSX
        if (GameSettings.Paws)
        {
            player->controls.dx = 0;
            player->controls.dy = 0;
            player->controls.digital = 0;
            player->controls.idigital = 0;
            player->controls.adigital = 0;
        }
#endif

        if (player->RaceFinishTime)
        {
            //player->controls.dx = 0;
            player->controls.dy = 0;
            player->controls.digital &= (CTRL_PAUSE | CTRL_HONKA);
            player->controls.digital |= CTRL_FULLBRAKE;
        }

        if (CountdownTime > 0) 
        {
            player->controls.digital &= (CTRL_PAUSE | CTRL_HONKA);
        }

// immediate and accumulated digital

        player->controls.idigital = (player->controls.digital ^ player->controls.lastdigital) & player->controls.digital;
        player->controls.adigital |= player->controls.digital;

// act on inputs

        if (player->conhandler)
        {
            player->conhandler(&player->controls, player->ownobj);
        }

// Set the changes flags (mainly for replay stuff)

        player->controls.changes = CTRL_CHANGE_NONE;
        if (player->controls.dx != player->controls.lastdx) player->controls.changes |= CTRL_CHANGE_DX;
        if (player->controls.dy != player->controls.lastdy) player->controls.changes |= CTRL_CHANGE_DY;
        if (player->controls.digital != player->controls.lastdigital) player->controls.changes |= CTRL_CHANGE_DIGITAL;

// save last controls

        player->controls.lastdx = player->controls.dx;
        player->controls.lastdy = player->controls.dy;
        player->controls.lastdigital = player->controls.digital;

// Reset controls for players

        player->controls.digital = 0;
        if ((player->type == PLAYER_LOCAL) || (player->type == PLAYER_CPU))
        {
            player->controls.dx = player->controls.dy = 0;
        }
    }
}

//--------------------------------------------------------------------------------------------------------------------------

// CON_RationaliseControl
//
// Removes control conflicts (eg. left + right) and converts key presses into postion deltas
// where required.

void CON_RationaliseControl(unsigned short *digital)
{

// Remove control clashes

    if ((*digital & CTRL_LR) == CTRL_LR)
    {
        *digital &= ~CTRL_LR;
    }

    if ((*digital & CTRL_FB) == CTRL_FB)
    {
        *digital &= ~CTRL_FB;
    }
}

//--------------------------------------------------------------------------------------------------------------------------

void CON_LocalCarControl(CTRL *Control, OBJECT *CarObj)
{
    CAR     *car;
    REAL    dest, step;
    int     i;

    Assert(CarObj->player != NULL);

    car = &CarObj->player->car;

    // slow player if 'it' in battle tag
    if (FoxObj && FoxObj->player)
    {
        FoxObj->player->controls.digital |= (3 << CTRL_CATCHUP_SHIFT);
    }

    // Set the angle of the wheels and engine voltage from
    // the position of the controls 

    // Set the angle of the steering wheel

    car->LastSteerAngle = car->SteerAngle;

#if defined(_PC)
    if (NonLinearSteering || (CarObj->player->type == PLAYER_CPU)) {
		// $MODIFIED jedl - cube the control rather than squaring it.
        // dest = (ONE / (CTRL_RANGE_MAX * CTRL_RANGE_MAX)) * Real(Control->dx) * abs(Real(Control->dx));
		dest = (ONE / (CTRL_RANGE_MAX * CTRL_RANGE_MAX * CTRL_RANGE_MAX)) * Real(Control->dx) * Real(Control->dx) * Real(Control->dx);
    } else {
        dest = (ONE / CTRL_RANGE_MAX) * Real(Control->dx);
    }
    step = MulScalar(car->SteerRate, TimeStep);
    if ((dest == ZERO) || (Sign(dest) != Sign(car->SteerAngle))) {
        step *=2;
    } else {
        step *= HALF;
    }
#elif defined (_PSX)
    //dest = (ONE  * Control->dx * abs(Control->dx)) / (CTRL_RANGE_MAX * CTRL_RANGE_MAX);
    dest = (Control->dx * Control->dx * Control->dx) / 32;
    step = MulScalar(car->SteerRate, TimeStep);
    if ((abs(dest) < 20) || (Sign(dest) != Sign(car->SteerAngle))) {
        step <<= 2;
    //} else if (abs(Control->dx) == CTRL_RANGE_MAX) {
    //  step >>= 1;
    } else {
        step >>= 2;
    }
#elif defined (_N64)
    dest = (ONE  * Control->dx * Control->dx * Control->dx) / (CTRL_RANGE_MAX * CTRL_RANGE_MAX * CTRL_RANGE_MAX);
    step = MulScalar(car->SteerRate, TimeStep);
    if ((dest == ZERO) || (Sign(dest) != Sign(car->SteerAngle))) {
        step *=2;
    } else {
        step *= HALF;
    }
#endif
    
    if (CarObj->player->car.PowerTimer > ZERO) {
        step /= 3;
    }

    if ((dest == ZERO) || (Sign(dest) != Sign(car->SteerAngle))) {
        step *= 2;
    }

    if (dest > car->SteerAngle) {
        if (dest - car->SteerAngle < step) {
            car->SteerAngle = dest;
        } else {
            car->SteerAngle += step;
        }
    }
    if (dest < car->SteerAngle) {
        if (car->SteerAngle - dest < step) {
            car->SteerAngle = dest;
        } else {
            car->SteerAngle -= step;
        }
    }

    // Set voltage across the motor
    dest = -((ONE / CTRL_RANGE_MAX) * (REAL)(Control->dy));
#ifndef _PSX
    step = MulScalar(car->EngineRate, TimeStep);
#else
    step = MulScalar(car->EngineRate, TimeStep);
    //step = MulScalar(car->EngineRate, TimeFactor) / 60;
#endif
    if ((dest < ZERO && car->EngineVolt > ZERO) || (dest > ZERO && car->EngineVolt < ZERO)) {
        car->EngineVolt = ZERO;
    }
    if (dest > car->EngineVolt) {
        if (dest - car->EngineVolt < step) {
            car->EngineVolt = dest;
        } else {
            car->EngineVolt += step;
        }
    }
    if (dest < car->EngineVolt) {
        if (car->EngineVolt - dest < step) {
            car->EngineVolt = dest;
        } else {
            car->EngineVolt -= step;
        }
    }


    // Speed adjustment ?
    if (Control->digital & CTRL_CATCHUP_MASK)
    {
        int speed;
        REAL speedMod;

        speed = ((Control->digital & CTRL_CATCHUP_MASK) >> CTRL_CATCHUP_SHIFT);
        if (Control->digital & CTRL_CATCHUP_DIR)    // Speed up ?
        {
            speed = speed & 3;
            speedMod = gpCatchUpVars->pSpeedUpTable[speed];

#ifdef BETTER_CATHCUP
//          if (car->DefaultTopSpeed > Players[0].car.DefaultTopSpeed)
                car->TopSpeed = car->DefaultTopSpeed + speedMod;
//          else
//              car->TopSpeed = Players[0].car.DefaultTopSpeed + speedMod;
#else
            if (car->DefaultTopSpeed > Players[0].car.DefaultTopSpeed)
                car->TopSpeed = car->DefaultTopSpeed + speedMod;
            else
                car->TopSpeed = Players[0].car.DefaultTopSpeed + speedMod;
#endif
            speed++;

            // Adjust wheel parameters
            for (i = 0; i < CAR_NWHEELS; i++)
            {
                car->Wheel[i].StaticFriction  = car->Wheel[i].defaultStaticFriction +
                                                ((speed * car->Wheel[i].defaultStaticFriction) / 24);
                car->Wheel[i].KineticFriction = car->Wheel[i].defaultKineticFriction +
                                                ((speed * car->Wheel[i].defaultKineticFriction) / 24);
            }
        }
        else                                        // Slow down
        {
            speedMod = gpCatchUpVars->pSlowDownTable[speed-1];

#ifdef BETTER_CATHCUP
            if (car->DefaultTopSpeed < Players[0].car.DefaultTopSpeed)
                car->TopSpeed = car->DefaultTopSpeed;
            else
                car->TopSpeed = car->DefaultTopSpeed + speedMod;
#else
            if (car->DefaultTopSpeed < Players[0].car.DefaultTopSpeed)
                car->TopSpeed = car->DefaultTopSpeed + speedMod;
            else
                car->TopSpeed = Players[0].car.DefaultTopSpeed + speedMod;
#endif
        }
    }
    else
    {
        // Restore top speed
        car->TopSpeed = car->DefaultTopSpeed;

        // Restore wheel parameters
        for (i = 0; i < CAR_NWHEELS; i++)
        {
            car->Wheel[i].StaticFriction  = car->Wheel[i].defaultStaticFriction;
            car->Wheel[i].KineticFriction = car->Wheel[i].defaultKineticFriction;
        }
    }


    // Reset car?
    if ((Control->idigital & CTRL_RESET) &&                                         // Act on reposition, but...
        (CarObj->player->car.RepositionTimer == ZERO) &&                            // Not when repositioning...
        (CarObj->body.Centre.WMatrix.mv[Y].v[Y] <= Real(0.3)) &&                    // Not unless nearly unpside-down...
        ((CarObj->player->car.NWheelColls > 0) || (CarObj->body.NBodyColls > 0)     // Not if totally in the air...
         || (CarObj->player->car.Body->NoContactTime < TO_TIME(Real(0.1)))          // Car must be on floor most of the time...
         || (CarObj->body.Stacked)                                                  // Or it must be not moving...
         )
        )
    {
        CarObj->player->car.RightingCollide = TRUE;
        CarObj->player->car.RightingReachDest = FALSE;
        CarObj->movehandler = (MOVE_HANDLER)MOV_RightCar;
    }



    #ifdef _PSX     

        car->RPCounter -= TimeFactor;

        if( car->RPCounter < 0 || car->RPCounter > FRAMESPERSECOND )
            car->RPCounter  = 0;

    #endif

    // Reposition car
    if (Control->idigital & CTRL_REPOSITION) 
    {
        if (GameSettings.GameType == GAMETYPE_NETWORK_BATTLETAG)
        {

//          if ((CarObj->player->car.Body->NBodyColls > 0) ||
            if ((car->Body->NoContactTime < TO_TIME(Real(0.1))) ||  (car->NWheelColls > 0))
            {

                #ifdef _PSX     
                if( !car->RPCounter  )
                {
                    car->RPCounter = FRAMESPERSECOND;

                #endif



                    car->Body->Centre.Vel.v[X] += (car->Body->Centre.WMatrix.m[LX] * TO_VEL(-200));
                    car->Body->Centre.Vel.v[Y] += TO_VEL(-500);
                    car->Body->Centre.Vel.v[Z] += (car->Body->Centre.WMatrix.m[LZ] * TO_VEL(-200));
                    car->Body->AngVel.v[X] += car->Body->Centre.WMatrix.mv[R].v[X] * TO_VEL(50);
                    car->Body->AngVel.v[Y] += car->Body->Centre.WMatrix.mv[R].v[Y] * TO_VEL(50);
                    car->Body->AngVel.v[Z] += car->Body->Centre.WMatrix.mv[R].v[Z] * TO_VEL(50);

                
                #ifdef _PSX     
                }
                #endif

            }
        }
        else
        {
            StartCarReposition(CarObj->player);
        }
    }

    // honka
#ifdef _PC
    if (Control->idigital & CTRL_HONKA && CarObj->player->type != PLAYER_REMOTE) 
    {
#ifdef OLD_AUDIO
        PlaySfx3D(SFX_HONKA, SFX_MAX_VOL, SFX_SAMPLE_RATE, &CarObj->body.Centre.Pos, 2);
#else
        g_SoundEngine.Play2DSound( EFFECT_Honka, FALSE );
#endif // OLD_AUDIO

        if (IsMultiPlayer())
        {
            SendHonka();
        }
    }
#endif

    // Restart car?
#ifdef _N64
/*  if (CarObj->player->controls.idigital & CTRL_RESTART && !CountdownTime)
    {
        VEC     vec;
        MAT     mat;

        GetCarStartGrid(0, &vec, &mat);
        SetCarPos(car, &vec, &mat);

        CarObj->player->CarAI.ZoneID = 0;

        CarObj->player->CarAI.BackTracking = TRUE;
        CarObj->player->CarAI.FinishDistNode = PosStartNode;
        CarObj->player->CarAI.FinishDist = 0.0f;
        CarObj->player->CarAI.FinishDistPanel = 0.0f;

        CarObj->movehandler = CarObj->defaultmovehandler;
    }*/
#endif

#ifdef _PSX
/*  if (CarObj->player->controls.idigital & CTRL_RESTART)
    {
        VEC vec;
        MAT mat;

        GetCarStartGrid(0, &vec, &mat);
        SetCarPos(car, &vec, &mat);

        CarObj->movehandler = CarObj->defaultmovehandler;
    }*/
#endif

    // release weapon?
    if (Control->idigital & CTRL_FIRE && CarObj->player->PickupNum)
    {
        FirePlayerWeapon(CarObj);
    }


#if 0
    // handbrake?
    if (Control->digital & CTRL_HANDBRAKE) {
        SetWheelLocked(&car->Wheel[2]);
        SetWheelLocked(&car->Wheel[3]);
    } else  if (Control->digital & CTRL_FULLBRAKE) {
        SetWheelLocked(&car->Wheel[0]);
        SetWheelLocked(&car->Wheel[1]);
        SetWheelLocked(&car->Wheel[2]);
        SetWheelLocked(&car->Wheel[3]);
    } else {
        SetWheelNotLocked(&car->Wheel[0]);
        SetWheelNotLocked(&car->Wheel[1]);
        SetWheelNotLocked(&car->Wheel[2]);
        SetWheelNotLocked(&car->Wheel[3]);
    }
#endif

    // weapon select?
#ifdef _PC
    if ((CarObj->player->controls.idigital & CTRL_SELWEAPON) && AllWeapons && !IsMultiPlayer() && GameSettings.GameType != GAMETYPE_TRIAL)
#else
    if ((CarObj->player->controls.idigital & CTRL_SELWEAPON) && AllWeapons)
#endif
    {
        static long pickup = PICKUP_TYPE_NONE;

        pickup = (pickup + 1) % PICKUP_NTYPES;
        GivePickupToPlayer(CarObj->player, pickup);
        /*CarObj->player->PickupType = pickup;
        CarObj->player->PickupTarget = NULL;
        switch (CarObj->player->PickupType)
        {
            case PICKUP_FIREWORKPACK:
            case PICKUP_WATERBOMB:
                CarObj->player->PickupNum = 3;
                break;
            default:
                CarObj->player->PickupNum = 1;
                break;
        }*/
    }
}

//--------------------------------------------------------------------------------------------------------------------------

void CON_CopyControls(CTRL *src, CTRL *dest)
{
    dest->dx = src->dx;
    dest->dy = src->dy;
    dest->digital = src->digital;
    dest->idigital = src->idigital;
    dest->adigital = src->adigital;
}

//--------------------------------------------------------------------------------------------------------------------------

void CON_InitControls(CTRL *ctrl)
{
    ctrl->dx = ctrl->dy = 0;
    ctrl->lastdx = ctrl->lastdy = 0;
    ctrl->digital = 0;
    ctrl->lastdigital = 0;
    ctrl->idigital = 0;
    ctrl->adigital = 0;
    ctrl->changes = 0;
}


////////////////////////////////////////////////////////////////
//
// Fire a player's weapon
//
////////////////////////////////////////////////////////////////

void FirePlayerWeapon(OBJECT *CarObj)
{
    long    flag[2], weapon;
    OBJECT  *obj;

    // good obj?
    Assert(CarObj->Type == OBJECT_TYPE_CAR);

    // dummy weapon?
    if (CarObj->player->PickupType == PICKUP_TYPE_DUMMY)
    {
        CarObj->player->PickupType = PICKUP_TYPE_NONE;
        CarObj->player->PickupNum = 0;
        return;
    }

    // get weapon / obj
    flag[0] = (long)CarObj->player;
    flag[1] = FALSE;
    weapon = Pickup2WeaponTable[CarObj->player->PickupType];
    obj = CreateObject(&CarObj->player->car.Body->Centre.Pos, &CarObj->player->car.Body->Centre.WMatrix, weapon, flag);

#ifdef _PC
    // multiplayer broadcast?
    if (IsMultiPlayer())
    {
        if (obj)
        {
            obj->GlobalID = GetGlobalID(CarObj->player->PlayerID);
            SendWeaponNew(&CarObj->player->car.Body->Centre.Pos, &CarObj->player->car.Body->Centre.WMatrix, CarObj->player->PlayerID, weapon, obj->GlobalID);
        }
        else if (weapon == OBJECT_TYPE_PICKUP)
        {
            GLOBAL_ID gid = INVALID_GLOBAL_ID;
            SendWeaponNew(&CarObj->player->car.Body->Centre.Pos, &CarObj->player->car.Body->Centre.WMatrix, CarObj->player->PlayerID, weapon, gid);
        }
    }
#endif

    // dec pickup num
    if (--CarObj->player->PickupNum == 0)
    {
#ifndef DEBUG
        CarObj->player->PickupType = PICKUP_TYPE_NONE;
#endif
        PlayerTargetOff(CarObj->player);

#ifdef _PC
        if (IsMultiPlayer())
            SendTargetStatus(CarObj->player->PlayerID, FALSE);
#endif
    }
}
