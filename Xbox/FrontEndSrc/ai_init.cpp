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

extern bool GHO_ShowGhost;

#include "SoundEffectEngine.h"
#include "SoundEffects.h"

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
    #pragma message( "JHarding needs to finish porting the section above.  Code below is what we had before Acclaim added new stuff..." )
    // Create the submix for car-based sounds (engine, scrape, etc). and start up the
    // engine sound
  #pragma message( "JHarding please investigate this.  Was causing crash on level load." )
  //$NOTE(cprince): disabled this, to avoid crash on level load (message saying can't StopEx a MIXIN/FXIN)
  /**
    g_SoundEngine.CreateSoundSource( player->ownobj, &(player->car.pSourceMix) );
    g_SoundEngine.PlaySubmixedSound( EFFECT_Motor, TRUE, player->car.pSourceMix, &(player->car.pEngineSound) );
  **/
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
