//-----------------------------------------------------------------------------
// File: pickup.cpp
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"
#include "pickup.h"
#include "player.h"
#include "geom.h"
#include "InitPlay.h"
#include "main.h"
#include "timing.h"
#include "obj_init.h"
#include "ai.h"
#include "weapon.h"
#include "TitleScreen.h"
#include "settings.h"

#include "load.h"
#include "Light.h"
#include "Cheats.h"

#define MAX_PICKUP_PLAYERS      100

PICKUP PickupArray[MAX_PICKUPS];
long NPickups;
REAL GlobalPickupFlash;

static MAT PickupMatrix = {1, 0, 0, 0, 1, 0, 0, 0, 1};

static void PickupHandler(PICKUP *pickup);
static void CalcPickupMatrix();

extern long FramesEver;


// Pickup weighting tables
#define PICKUP_WEIGHT_TABLE_NUM     4
short PickupWeight[][PICKUP_NTYPES] =
{
    {  5, 18,  5, 10,  5, 18, 18,  4,  8,  9,  0},      // 1st
    { 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,  0},      // mid
    { 20,  3, 15,  2, 15,  2,  2, 21, 13,  2,  5},      // last
    { 10, 10,  5, 10, 10,  5, 15, 15, 10, 10,  0},      // CPU cars
};

short PickupWeightPos[MAX_PICKUP_PLAYERS+1][PICKUP_NTYPES];

/*
    PICKUP_TYPE_SHOCKWAVE = 0,
    PICKUP_TYPE_FIREWORK,
    PICKUP_TYPE_FIREWORKPACK,
    PICKUP_TYPE_PUTTYBOMB,
    PICKUP_TYPE_WATERBOMB,
    PICKUP_TYPE_ELECTROPULSE,
    PICKUP_TYPE_OILSLICK,
    PICKUP_TYPE_CHROMEBALL,
    PICKUP_TYPE_TURBO,
    PICKUP_TYPE_CLONE,
    PICKUP_TYPE_GLOBAL,
*/
#if 0
char PickupRanges[] =
{
    0,0,        // 0 players
    0,1,        // 1 players
    0,1,        // 2 players
    0,1,        // 3 players
    0,2,        // 4 players
    0,0,        // 5 players
    0,0,        // 6 players
    0,0,        // 7 players
    0,0,        // 8 players
    0,0,        // 9 players
    0,0,        // 10 players
    0,0,        // 11 players
};
#endif

////////////////////////////////////////////////////////////////
//
// Initialise the pickup array
//
////////////////////////////////////////////////////////////////

void InitPickupArray()
{
    int iPickup;
    PICKUP *pickup;

    NPickups = 0;

    for (iPickup = 0; iPickup < MAX_PICKUPS; iPickup++) {

        pickup = &PickupArray[iPickup];

        pickup->ID = iPickup;
#ifndef _PSX
        pickup->RenderFlags.envmap = TRUE;
        pickup->RenderFlags.envonly = FALSE;
        pickup->RenderFlags.litsimple = FALSE;
        pickup->RenderFlags.reflect = TRUE;
        pickup->RenderFlags.fog = TRUE;
        pickup->RenderFlags.meshfx = TRUE;
        pickup->RenderFlags.shadow = FALSE;

        pickup->RenderFlags.light = FALSE;
        pickup->RenderFlags.glare = TRUE;

        pickup->EnvOffsetX = 0.0f;
        pickup->EnvOffsetY = 0.0f;
        pickup->EnvScale = 1.0f;
#endif

        InitOnePickup(pickup);
    }
}


void InitOnePickup(PICKUP *pickup)
{
    pickup->Clone = FALSE;
    pickup->Mode = PICKUP_STATE_FREE;
    pickup->EnvRGB = 0xffff80;
    SetVecZero(&pickup->GenPos);
    SetVecZero(&pickup->Pos);
    pickup->Timer = ZERO;
    SetVecZero(&pickup->Vel);
    pickup->Light = NULL;


}


////////////////////////////////////////////////////////////////
// Init pickup weight tables
////////////////////////////////////////////////////////////////
void InitPickupWeightTables(void)
{
    int i,c;
    int total;
    int weight, cur;

    for (i = 0; i < PICKUP_WEIGHT_TABLE_NUM; i++)
    {
    // Sum total weights
        total = 0;
        for (c = 0; c < PICKUP_NTYPES; c++)
            total += PickupWeight[i][c];

    // Create weights
        cur = 0;
        for (c = 0; c < PICKUP_NTYPES; c++)
        {
            weight = (PickupWeight[i][c] << 15) / total;
            PickupWeight[i][c] = cur + weight;
            cur += weight;
        }
    }
}

////////////////////////////////////////////////////////////////
// Init pickup weight tables
////////////////////////////////////////////////////////////////
void InitPickupRaceWeightTables(void)
{
    int i,c;
    int weight[2];

// not if 1 player game

    if (StartData.PlayerNum <= 1)
    {
        return;
    }

// Create pickup weights based on position.  1st to MidTable
    for (i = 0; i < StartData.PlayerNum/2; i++)
    {
        weight[1] = (i * 32768) / (StartData.PlayerNum-1);
        weight[0] = 32768 - weight[1];
        for (c = 0; c < PICKUP_NTYPES; c++)
            PickupWeightPos[i][c] = ((PickupWeight[0][c] * weight[0]) + (PickupWeight[1][c] * weight[1])) >> 15;

        // Make sure values add up to 32768
        if (PickupWeightPos[i][PICKUP_NTYPES-1] != 32767)
        {
            if (PickupWeightPos[i][PICKUP_NTYPES-1]  == PickupWeightPos[i][PICKUP_NTYPES-2])
                PickupWeightPos[i][PICKUP_NTYPES-2] = 32767;
            PickupWeightPos[i][PICKUP_NTYPES-1] = 32767;
        }
    }

// Create pickup weights based on position.  MidTable to Last
    for (i = StartData.PlayerNum/2; i < StartData.PlayerNum; i++)
    {
        weight[1] = (i * 32768) / (StartData.PlayerNum-1);
        weight[0] = 32768 - weight[1];
        for (c = 0; c < PICKUP_NTYPES; c++)
            PickupWeightPos[i][c] = ((PickupWeight[1][c] * weight[0]) + (PickupWeight[2][c] * weight[1])) >> 15;

        // Make sure values add up to 32768
        if (PickupWeightPos[i][PICKUP_NTYPES-1] != 32767)
        {
            if (PickupWeightPos[i][PICKUP_NTYPES-1]  == PickupWeightPos[i][PICKUP_NTYPES-2])
                PickupWeightPos[i][PICKUP_NTYPES-2] = 32767;
            PickupWeightPos[i][PICKUP_NTYPES-1] = 32767;
        }
    }

// Create pickup weights for computer cars
    i = MAX_PICKUP_PLAYERS;
    for (c = 0; c < PICKUP_NTYPES; c++)
        PickupWeightPos[i][c] = PickupWeight[PICKUP_WEIGHT_TABLE_NUM-1][c];

    // Make sure values add up to 32768
    if (PickupWeightPos[i][PICKUP_NTYPES-1] != 32767)
    {
        if (PickupWeightPos[i][PICKUP_NTYPES-1]  == PickupWeightPos[i][PICKUP_NTYPES-2])
            PickupWeightPos[i][PICKUP_NTYPES-2] = 32767;
        PickupWeightPos[i][PICKUP_NTYPES-1] = 32767;
    }
}


////////////////////////////////////////////////////////////////
//
// AllocOnePickup:
//
////////////////////////////////////////////////////////////////

PICKUP *AllocOnePickup()
{
    int iPickup;

    // Make sure there are some left
    if (NPickups == MAX_PICKUPS) return NULL;

    // Find the first empty slot
    for (iPickup = 0; iPickup < MAX_PICKUPS; iPickup++) {
        if (PickupArray[iPickup].Mode == PICKUP_STATE_FREE) {
            NPickups++;
            return &PickupArray[iPickup];
        }
    }

    // No spare slots
    return NULL;
}


////////////////////////////////////////////////////////////////
//
// FreeOnePickup
//
////////////////////////////////////////////////////////////////

void FreeOnePickup(PICKUP *pickup)
{
    if (pickup->Mode == PICKUP_STATE_FREE) return;

    NPickups--;

#ifndef _PSX
    if (pickup->Light) {
        FreeLight(pickup->Light);
    }
#endif 

    InitOnePickup(pickup);

}


////////////////////////////////////////////////////////////////
//
// FreeAllPickups
//
////////////////////////////////////////////////////////////////

void FreeAllPickups()
{
    int iPickup;

    for (iPickup = 0; iPickup < MAX_PICKUPS; iPickup++) {
        FreeOnePickup(&PickupArray[iPickup]);
    }
    NPickups = 0;
}




////////////////////////////////////////////////////////////////
//
// InitPickups:  Turn on random selection of pickups
//
////////////////////////////////////////////////////////////////

void InitPickups(void) 
{
    int i, max;

    max = MAX_PICKUPS_ALLOWED * StartData.PlayerNum + INITIAL_PICKUPS;

    for (i = 0 ; i < max ; i++)
    {
        AllowOnePickup(i);
    }
}

////////////////////////////////////////////////////////////////
//
// Activate one more pickup
//
////////////////////////////////////////////////////////////////

void AllowOnePickup(long alive)
{
    long i, num, iPickup;
    PICKUP *pickup;


    // Don't do this in replay mode
#ifndef _PSX
    if (ReplayMode) return;
#endif

    // get nth 'held' pickup to allow
    if (alive == -1)
    {
        num = NPickups - (MAX_PICKUPS_ALLOWED * StartData.PlayerNum + INITIAL_PICKUPS) + 1;
        if (num < 1) num = 1;
    }
    else
    {
        num = NPickups - alive;
        if (num < 1)
            return;
    }

    i = rand() % num;

    // find it
    for (iPickup = 0; iPickup < MAX_PICKUPS; iPickup++)
    {
        pickup = &PickupArray[iPickup];
        if (pickup->Clone) continue;
        if (pickup->Mode != PICKUP_STATE_INACTIVE) continue;

        if (!i--)
        {
            // Set pickup to appear
            pickup->Mode = PICKUP_STATE_GENERATING;
            pickup->Timer = PICKUP_GEN_TIME;
            CopyVec(&pickup->GenPos, &pickup->Pos);

            // Store replay event
#ifndef _PSX
            if (RPL_RecordReplay) 
            {
                StoreGeneratePickup(pickup);
            }

#endif
            break;
        }
    }
}


////////////////////////////////////////////////////////////////
//
// UpdateAllPickups
//
////////////////////////////////////////////////////////////////

void UpdateAllPickups()
{
    int iPickup;

    if (GameSettings.AllowPickups) {
        CalcPickupMatrix();
    }

    for (iPickup = 0; iPickup < MAX_PICKUPS; iPickup++) {
        PickupHandler(&PickupArray[iPickup]);
    }

    if (GlobalPickupFlash)
    {
        GlobalPickupFlash -= TimeStep;
        if (GlobalPickupFlash < 0)
            GlobalPickupFlash = 0;
    }
}


////////////////////////////////////////////////////////////////
//
// Calculate the pickup's rotation matrix
//
////////////////////////////////////////////////////////////////

void CalcPickupMatrix()
{
#ifndef _PSX
    RotMatrixY(&PickupMatrix, TIME2MS(TimerCurrent) * 0.0005f);
#endif
}


////////////////////////////////////////////////////////////////
//
// pickup generator handler
//
////////////////////////////////////////////////////////////////

#ifndef _PSX
void PickupHandler(PICKUP *pickup)
{
    long col, flag[2];
    REAL mul;
    PLAYER *player;

// act on mode

    switch (pickup->Mode)
    {

// waiting to generate?

        case 0:

            pickup->Timer -= TimeStep;
            if (pickup->Timer <= 0)
            {
                pickup->Mode = 1;
                pickup->Timer = 0.0f;
                pickup->EnvRGB = 0xffff80;
#ifndef _PSX
  #ifdef OLD_AUDIO
                PlaySfx3D(SFX_PICKUP_REGEN, SFX_MAX_VOL, 22050, &pickup->Pos, 2);
  #else // !OLD_AUDIO
  #pragma message( "Please port this, Mr. Harding" )
  #endif // !OLD_AUDIO
#endif
            }

            return;
            break;

// alive

        case 1:

// inc age

            pickup->Timer += TimeStep;

// spin

            //RotMatrixY(&obj->body.Centre.WMatrix, TIME2MS(CurrentTimer()) / 2000.0f);
            CopyMat(&PickupMatrix, &pickup->WMatrix);
            if (pickup->Timer < 1.6f)
            {
                if (pickup->Timer < 0.5f)
                    mul = 0;
                else if (pickup->Timer < 1.0f)
                    mul = (pickup->Timer - 0.5f) * 2.0f + (float)sin((pickup->Timer - 0.5f) * RAD) * 0.6667f;
                else if (pickup->Timer < 1.35f)
                    mul = 1.0f - (float)sin((pickup->Timer - 1.0f) * 2.85714f * PI) * (1.0f / 6.0f);
                else
                    mul = 1.0f + (float)sin((pickup->Timer - 1.35f) * 4.0f * PI) * (1.0f / 12.0f);

                VecMulScalar(&pickup->WMatrix.mv[R], mul);
                VecMulScalar(&pickup->WMatrix.mv[U], mul);
                VecMulScalar(&pickup->WMatrix.mv[L], mul);
            }

// need a light source?

            if (!pickup->Light)
            {
                pickup->Light = AllocLight();
                if (pickup->Light)
                {
                    CopyVec(&pickup->Pos, (VEC*)&pickup->Light->x);
                    pickup->Light->Reach = 512;
                    pickup->Light->Flag = LIGHT_FIXED | LIGHT_MOVING;
                    pickup->Light->Type = LIGHT_OMNI;
                }
            }

// maintain light

            if (pickup->Light)
            {
                if (pickup->Timer < 0.75f)
                {
                    FTOL(pickup->Timer * 340.0f, col);
                }
                else
                {
                    col = 255;
                }

                pickup->Light->r = col / 2;
                pickup->Light->g = col * 3 / 8;
                pickup->Light->b = 0;
            }

// look for car collision

            for (player = PLR_PlayerHead ; player ; player = player->next)
            {
#if defined(_PC)
                if (((!player->PickupNum && !player->PickupCycleSpeed) || pickup->Clone) && !player->Spectator)
#elif defined(_N64)
                if ((!player->PickupNum && !player->PickupCycleSpeed) || pickup->Clone)
#endif
                {
                    if (BBTestXZY(&pickup->BBox, &player->car.BBox)) 
                    {
                        pickup->Mode = 2;
                        pickup->Timer = 0.0f;

                        SetVector(&pickup->Vel, player->ownobj->body.Centre.Vel.v[X] / 2.0f, -64.0f, player->ownobj->body.Centre.Vel.v[Z] / 2.0f);

                        if (!pickup->Clone)
                        {
#ifndef _PSX
  #ifdef OLD_AUDIO
                            PlaySfx3D(SFX_PICKUP, SFX_MAX_VOL, 22050, &pickup->Pos, 2);
  #else // !OLD_AUDIO
  #pragma message( "Please port this, Mr. Harding" )
  #endif // !OLD_AUDIO
#endif
                            if (player->type == PLAYER_REMOTE)
                            {
                                GivePickupToPlayer(player, PICKUP_TYPE_DUMMY);
                            }
                            else
                            {
                                int pos, r, c,lastweight;

                                // Choose the pickup that the player will get
                                if (player->type == PLAYER_LOCAL)
                                    pos = player->CarAI.racePosition;
                                else
                                    pos = MAX_PICKUP_PLAYERS;

                                lastweight = 0;
                                r = rand() & 32767;
                                for (c = 0; c < PICKUP_NTYPES-1; c++)
                                {
                                    if (lastweight != PickupWeightPos[pos][c])
                                    {
                                        if (r < PickupWeightPos[pos][c])
                                            break;
                                    }

                                    lastweight = PickupWeightPos[pos][c];
                                }

                                player->PickupCycleDest = (REAL)c;
                                player->PickupCycleSpeed = Real(PICKUP_CYCLE_TIME);
                            }
                        }
                        else
                        {
#ifndef _PSX
  #ifdef OLD_AUDIO
                            PlaySfx3D(SFX_PUTTYBOMB_BANG, SFX_MAX_VOL, 22050, &pickup->Pos, 2);
  #else // !OLD_AUDIO
  #pragma message( "Please port this, Mr. Harding" )
  #endif // !OLD_AUDIO
#endif
                            flag[0] = (long)player;
                            flag[1] = TRUE;
                            CreateObject(&player->car.Body->Centre.Pos, &player->car.Body->Centre.WMatrix, OBJECT_TYPE_PUTTYBOMB, flag);
                        }

                        break;
                    }
                }
            }

            return;
            break;

// dissappearing

        case 2:

// inc age

            pickup->Timer += TimeStep;

// spin

            RotMatrixY(&pickup->WMatrix, TIME2MS(CurrentTimer()) / 2000.0f);

            mul = pickup->Timer * 2.0f + 1.0f;

            pickup->WMatrix.m[XX] *= mul;
            pickup->WMatrix.m[YY] *= mul;
            pickup->WMatrix.m[ZZ] *= mul;

// set env rgb

            FTOL(-pickup->Timer * 255.0f + 255.0f, col);

            pickup->EnvRGB = col >> 1 | col << 8 | col << 16;

// maintain light

            if (pickup->Light)
            {
                pickup->Light->r = col / 2;
                pickup->Light->g = col * 3 / 8;
                pickup->Light->b = 0;
            }

// set pos

            VecPlusScalarVec(&pickup->GenPos, pickup->Timer, &pickup->Vel, &pickup->Pos);

// done?

            if (pickup->Timer > 1.0f)
            {
                if (pickup->Light)
                {
                    FreeLight(pickup->Light);
                    pickup->Light = NULL;
                }

                if (pickup->Clone)
                {
                    pickup->Mode = PICKUP_STATE_FREE;
                    return;
                }
                else
                {
#ifdef _PC
                    if (IsMultiPlayer())
                    {
                        pickup->Mode = PICKUP_STATE_GENERATING;
                        pickup->Timer = PICKUP_GEN_TIME;
                        CopyVec(&pickup->GenPos, &pickup->Pos);
                    }
                    else
#endif
                    {
                        pickup->Mode = PICKUP_STATE_INACTIVE;
                        AllowOnePickup(-1);
                    }
                }
            }

            return;
            break;

// holding - waiting for permission to generate

        case 3:
            return;
            break;

    }
}

#else // _PSX

void PickupHandler(PICKUP *pickup)
{
    long col, flag;
    REAL mul;
    PLAYER *player;
    OBJECT *bombobj;
    PUTTYBOMB_OBJ *bomb;

// act on mode

    switch (pickup->Mode)
    {

// waiting to generate?

        case 0:

            pickup->Timer -= TimeStep;
            if (pickup->Timer <= 0)
            {
                pickup->Mode = 1;
                pickup->Timer = 0;
                pickup->EnvRGB = 0xffff80;
            }

            break;

// alive

        case 1:

// inc age

            pickup->Timer += TimeStep;

// look for car collision

            for (player = PLR_PlayerHead ; player ; player = player->next)
            {
                if ((!player->PickupNum && !player->PickupCycleSpeed) || pickup->Clone)
                {
                    if (BBTestXZY(&pickup->BBox, &player->car.BBox)) 
                    {
                        pickup->Mode = 2;
                        pickup->Timer = 0;

                        SetVector(&pickup->Vel, DivScalar(player->ownobj->body.Centre.Vel.v[X], Real(2.0f) ), Real(-64.0f),
                            DivScalar( player->ownobj->body.Centre.Vel.v[Z], Real(2.0f) )  );

                        if (!pickup->Clone)
                        {
                            SFX_Play3D( SAMPLE_PICKUP_GET, 0x3fff, 2048, 0, &pickup->GenPos );
                            if( SplitScreenMode == 2 )
                            {
                                SFX_SecondPlayerFlag = TRUE;
                                SFX_Play3D( SAMPLE_PICKUP_GET, 0x3fff, 2048, 0, &pickup->GenPos );
                                SFX_SecondPlayerFlag = FALSE;
                            }
                            if (player->type == PLAYER_REMOTE)
                            {
                                GivePickupToPlayer(player, PICKUP_TYPE_DUMMY);
                            }
                            else
                            {
                                int pos, r, i, c;

                                // Choose the pickup that the player will get
                                pos = player->CarAI.racePosition;

                                r = FramesEver & 255;//32767;
                                for (c = 0; c < PICKUP_NTYPES-1; c++)
                                {
                                    if (r < PickupWeightPos[pos][c] )
                                        break;
                                }

                                player->PickupCycleDest = c << 16;
                                player->PickupCycleSpeed = PICKUP_CYCLE_TIME;//(FRAMESPERSECOND<<1) + (FramesEver%20);

                            }
                        


//                          else
//                          {
//                              player->PickupCycleType = rand()%PICKUP_NTYPES;
//                              player->PickupCycleSpeed = (FRAMESPERSECOND<<1) + (rand()%20);
//                          }



                        }
                        else
                        {
                            flag = (long)player;
                            bombobj = CreateObject(&player->car.Body->Centre.Pos, &player->car.Body->Centre.WMatrix, OBJECT_TYPE_PUTTYBOMB, &flag);
                            if (bombobj)
                            {
                                bomb = (PUTTYBOMB_OBJ*)bombobj->Data;
                                bomb->Timer = 0;
                                player->car.WillDetonate = TRUE;
                            }
                        }

                        break;
                    }
                }
            }

            break;

// dissappearing

        case 2:

// inc age

            pickup->Timer += TimeStep;
        
// set pos

        //  VecPlusScalarVec(&pickup->Pos, pickup->Timer, &pickup->Vel, &obj->body.Centre.Pos);

// done?

            if (pickup->Timer > TO_TIME(Real(1.0f)) )
            {
            
                if (pickup->Clone)
                {
                    pickup->Mode = PICKUP_STATE_FREE;
                    return;
                }
                else
                {
                    pickup->Mode = PICKUP_STATE_INACTIVE;
                    AllowOnePickup(-1);
                    
                }
            }

            break;

// holding - waiting for permission to generate

        case 3:
            break;

    }
}
#endif

////////////////////////////////////////////////////////////////
//
// GivePickupToPlayer:
//
////////////////////////////////////////////////////////////////

#ifndef _PSX
void GivePickupToPlayer(PLAYER *player, unsigned long type)
{

// init the pickup 

    player->PickupType = type;
    player->PickupTarget = NULL;
    player->PickupCycleSpeed = 0;

    switch (player->PickupType) {
    case PICKUP_TYPE_FIREWORKPACK:
        player->PickupNum = 3;
        break;
    case PICKUP_TYPE_WATERBOMB:
        player->PickupNum = 3;
        break;
    default:
        player->PickupNum = 1;
        break;
    }

// turn on target if necessary

    if (player->PickupType == PICKUP_TYPE_FIREWORK || player->PickupType == PICKUP_TYPE_FIREWORKPACK)
    {
        PlayerTargetOn(player);
#ifdef _PC
        if (IsMultiPlayer())
            SendTargetStatus(player->PlayerID, TRUE);
#endif
    } else {
        PlayerTargetOff(player);
    }

// Store replay event if necessary
    if (RPL_RecordReplay) {
        ReplayStoreGotPickup(player->ownobj);
    }

// If it is the putty bomb, activate it straight away
    if (player->PickupType == PICKUP_TYPE_PUTTYBOMB && !AllWeapons) {
        FirePlayerWeapon(player->ownobj);
    }

}

#else

void GivePickupToPlayer(struct PlayerStruct *player, unsigned long type)
{

    // Check that player hasn't already got the puttybomb...
    if( type == PICKUP_TYPE_PUTTYBOMB )
    {
        if( player->car.IsBomb )            // player is already a bomb
            type = PICKUP_TYPE_WATERBOMB;   // so give them waterbombs instead.
    }

// init the pickup
    
        player->PickupType = type;
        player->PickupTarget = NULL;
        player->PickupCycleSpeed = 0;

        switch (player->PickupType) {
        case PICKUP_TYPE_FIREWORKPACK:
                player->PickupNum = 3;
                break;
        case PICKUP_TYPE_WATERBOMB:
                player->PickupNum = 3;
                break;
        default:
                player->PickupNum = 1;
                break;
        }


    if (player->PickupType == PICKUP_TYPE_FIREWORK || player->PickupType == PICKUP_TYPE_FIREWORKPACK)
        PlayerTargetOn(player);
    else
        PlayerTargetOff(player);


    // Store replay event if necessary
    if (RPL_RecordReplay) {
        ReplayStoreGotPickup(player->ownobj);
    }

    // If it is the putty bomb, activate it straight away
    if (player->PickupType == PICKUP_TYPE_PUTTYBOMB ) 
        FirePlayerWeapon(player->ownobj);
    

}

#endif //_PSX

//***********************************************************************************************************


