/*********************************************************************************************
 *
 * ai_init.cpp
 *
 * Re-Volt (Generic) Copyright (c) Probe Entertainment 1998
 * 
 * Contents:
 *			Initialisation (and destruction) functions for object AIs
 *			This is a companion file to obj_init.cpp that intialises the object structure
 *			and calls the appropriate AI init function
 *
 *********************************************************************************************
 *
 * 01/07/98 Matt Taylor
 *  File inception.
 *
 *********************************************************************************************/

#include "revolt.h"
#ifndef _PSX
#include "main.h"
#endif
#include "geom.h"
#include "particle.h"
#include "model.h"
#include "aerial.h"
#include "newcoll.h"
#include "body.h"
#include "car.h"
#include "ctrlread.h"
#include "object.h"
#include "player.h"
#include "ai.h"
#include "ai_init.h"
#ifdef _PC
#include "sfx.h"
#endif

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

#ifdef _PC

// create engine sfx

	player->car.SfxEngine = CreateSfx3D(SFX_ENGINE, 0, 0, TRUE, &player->car.Body->Centre.Pos);

// null scrape sfx

	player->car.SfxScrape = NULL;

// null screech sfx

	player->car.SfxScreech = NULL;

#endif

// Returns true on success

	return(TRUE);
}

//--------------------------------------------------------------------------------------------------------------------------
