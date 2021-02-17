//-----------------------------------------------------------------------------
// File: AI_Init.cpp
//
// Desc: Initialisation (and destruction) functions for object AIs.
//       This is a companion file to obj_init.cpp that intialises the object
//       structure and calls the appropriate AI init function.
//
// Re-Volt (Generic) Copyright (c) Probe Entertainment 1998
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"
#include "main.h"
#include "geom.h"
#include "player.h"
#include "aizone.h"
#include "posnode.h"

#ifdef _N64
#include "LevelInfo.h"
#endif

#ifndef _PSX
extern bool GHO_ShowGhost;
#endif

#include "SoundEffectEngine.h"

//
// Static variables
//


//
// Global variables
//


//
// Global function prototypes
//

bool AI_InitPlayerAI(PLAYER *player);


//--------------------------------------------------------------------------------------------------------------------------

//
// AI_InitPlayerAI
//
// Initialises the player's "AI" (sound handling, etc). Requires a player structure be passed to the function, rather than 
// an object structure. This should be called from PLR_CreatePlayer.
//

bool AI_InitPlayerAI(PLAYER *player)
{
#ifndef _PSX
 #ifdef OLD_AUDIO
    // TODO (JHarding): Need to support different engine sounds.  Keep this code around for a while
    // until that's done.
    long sfx;

// create engine sfx

    if (GameSettings.Level != LEVEL_FRONTEND && (player->type != PLAYER_GHOST || GHO_ShowGhost))
    {
        if (CarInfo[player->car.CarType].Class == CAR_CLASS_ELEC) sfx = SFX_ENGINE;
        else sfx = SFX_ENGINE_PETROL;

#ifdef _PC
        if (player->car.CarType == CARID_KEY4) sfx = SFX_ENGINE_CLOCKWORK;
        else if (player->car.CarType == CARID_UFO) sfx = SFX_ENGINE_UFO;
#endif

        player->car.SfxEngine = CreateSfx3D(sfx, 0, 0, TRUE, &player->car.Body->Centre.Pos, player->type == PLAYER_LOCAL ? 4 : 1);
    }

// null scrape sfx

    player->car.SfxScrape = NULL;

// null screech sfx

    player->car.SfxScreech = NULL;
 #else // !OLD_AUDIO
    // Create the submix for car-based sounds (engine, scrape, etc). and start up the
    // engine sound
    g_SoundEngine.CreateSoundSource( player->ownobj, &(player->car.pSourceMix) );

    if( GameSettings.Level != LEVEL_FRONTEND && (player->type != PLAYER_GHOST || GHO_ShowGhost) )
    {
        DWORD dwEffect;

        if( CarInfo[ player->car.CarType ].Class == CAR_CLASS_ELEC )
            dwEffect = EFFECT_Motor;
        else if( player->car.CarType == CARID_KEY4 )
            dwEffect = EFFECT_Clockwork;
        else if( player->car.CarType == CARID_UFO )
            dwEffect = EFFECT_UFO;
        else
            dwEffect = EFFECT_Petrol;

        g_SoundEngine.PlaySubmixedSound( dwEffect, TRUE, player->car.pSourceMix, &(player->car.pEngineSound) );
        assert( player->car.pEngineSound );
        // Mute the engine so we don't hear it until we're ready.
        player->car.pEngineSound->SetVolume( DSBVOLUME_MIN );
    }

 #endif // !OLD_AUDIO

#endif

//  player->CarAI.ResetCnt = 0;
    player->CarAI.StuckCnt = 0;
    
    player->CarAI.ZoneID = 0;
    player->CarAI.LastValidZone = 0;
    CAI_IsCarInZone(player);

    player->CarAI.FinishDistNode = PosStartNode;
    player->CarAI.FinishDist = Real(0.0f);
    player->CarAI.FinishDistPanel = Real(0.0f);
    player->CarAI.BackTracking = TRUE;
    player->CarAI.PreLap = TRUE;
    UpdateCarAiZone(player);
    UpdateCarFinishDist(player, NULL);

    player->AccelerateTimeStamp = 0;

// Returns true on success

    return(TRUE);
}

//--------------------------------------------------------------------------------------------------------------------------
