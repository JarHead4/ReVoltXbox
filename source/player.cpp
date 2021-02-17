/*********************************************************************************************
 *
 * player.cpp
 *
 * Re-Volt (Generic) Copyright (c) Probe Entertainment 1998
 * 
 * Contents:
 *			Player handling code
 *
 *********************************************************************************************
 *
 * 03/03/98 Matt Taylor
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
#include "Control.h"
#include "player.h"
#include "move.h"
#include "move.h"
#include "ai.h"
#include "ai_car.h"
#include "ai_init.h"
#ifdef _PC
#include "sfx.h"
#endif
#include "field.h"

//
// Static variables
// 

PLAYER *s_NextFreePlayer;

//
// Global variables
//

PLAYER	Players[MAX_NUM_PLAYERS];
PLAYER *PLR_PlayerHead = NULL;
PLAYER *PLR_PlayerTail = NULL;

//long	MyPlayerNum;						// Index into Players array - "who you are" :)
PLAYER *PLR_LocalPlayer = NULL;
CTRL_TYPE PLR_LocalCtrlType;

long	NumPlayers;

//
// Global function prototypes
//

void PLR_InitPlayers(void);
PLAYER *PLR_CreatePlayer(PLAYER_TYPE Type, CTRL_TYPE CtrlType, CAR_TYPE CarType, VEC *Pos, MAT *Mat);
void PLR_KillPlayer(PLAYER *Player);
void CreatePlayerForceField(PLAYER *player);

//--------------------------------------------------------------------------------------------------------------------------

void PLR_InitPlayers(void)
{
	long	ii;

	for (ii = 0; ii < MAX_NUM_PLAYERS; ii++)
	{
		Players[ii].type = PLAYER_NONE;
		Players[ii].score = 0;
		Players[ii].lastscore = 0;
		Players[ii].raceswon = 0;
		Players[ii].ctrlhandler = NULL;
		Players[ii].conhandler = NULL;
		Players[ii].Slot = ii;
	}

	NumPlayers = 0;
	s_NextFreePlayer = Players;

	Players[0].prev = NULL;
	Players[0].next = &(Players[1]);

	for (ii = 1; ii < (MAX_NUM_PLAYERS - 1); ii++)
	{
		Players[ii].prev = &(Players[ii - 1]);
		Players[ii].next = &(Players[ii + 1]);
	}

	Players[MAX_NUM_PLAYERS - 1].prev = &(Players[MAX_NUM_PLAYERS - 2]);
	Players[MAX_NUM_PLAYERS - 1].next = NULL;

	PLR_PlayerHead = NULL;
	PLR_PlayerTail = NULL;
}

//--------------------------------------------------------------------------------------------------------------------------

PLAYER *PLR_CreatePlayer(PLAYER_TYPE Type, CTRL_TYPE CtrlType, CAR_TYPE CarType, VEC *Pos, MAT *Mat)
{
	PLAYER  *newplayer;
	OBJECT	*newobj;

	// Make sure there are some spare player slots (huh huh....I think I'm gay...!)
	if (s_NextFreePlayer == NULL) {
		return NULL;
	}
#ifdef _PC
	Assert(NumPlayers <= MAX_NUM_PLAYERS);
#endif

	newobj = OBJ_AllocObject();
	if (newobj == NULL)
	{
		return(NULL);									// Could not allocate object for player
	}

	newplayer = s_NextFreePlayer;						// Get next empty player

	s_NextFreePlayer = s_NextFreePlayer->next;
	if (s_NextFreePlayer != NULL)
	{
		s_NextFreePlayer->prev = NULL;
	}

	newplayer->prev = PLR_PlayerTail;

	if (PLR_PlayerHead == NULL)
	{
		PLR_PlayerHead = newplayer;
	}
	else
	{
        PLR_PlayerTail->next = newplayer;
	}
	PLR_PlayerTail = newplayer;
	
	newplayer->next = NULL;

	newplayer->ownobj = newobj;
	newplayer->ownobj->player = newplayer;
	newplayer->car.Body = &newplayer->ownobj->body;
	newplayer->type = Type;

	newplayer->ctrltype = CtrlType;							// Set up controller used by player
	newplayer->conhandler = NULL;
#ifndef _PSX
	CRD_InitPlayerControl(newplayer, CtrlType);
#endif

	newplayer->ownobj->flag.Draw = FALSE;
	newplayer->ownobj->flag.Move = TRUE;
	newplayer->ownobj->renderhandler = NULL;
	newplayer->ownobj->freehandler = NULL;
	newplayer->ownobj->Type = OBJECT_TYPE_CAR;
	newplayer->ownobj->Field = NULL;

	if (Type == PLAYER_LOCAL)
	{
#ifndef _PSX
		newplayer->conhandler = (CON_HANDLER)CON_LocalCarControl;
#endif
		newplayer->ownobj->aihandler = (AI_HANDLER)AI_CarAiHandler;
		newplayer->ownobj->CollType = COLL_TYPE_CAR;
		newplayer->ownobj->movehandler = (MOVE_HANDLER)MOV_MoveCarNew;
		newplayer->ownobj->collhandler = (COLL_HANDLER)COL_CarCollHandler;
	}
	else if (Type == PLAYER_CPU)
	{
		newplayer->ownobj->aihandler = (AI_HANDLER)AI_CarAiHandler;
		newplayer->ownobj->CollType = COLL_TYPE_CAR;
		newplayer->ownobj->movehandler = (MOVE_HANDLER)MOV_MoveCarNew;
		newplayer->ownobj->collhandler = (COLL_HANDLER)COL_CarCollHandler;
	}
#ifdef _PC
	else if (Type == PLAYER_GHOST)
	{
		newplayer->ownobj->CollType = COLL_TYPE_NONE;
		newplayer->ownobj->movehandler = NULL;
		newplayer->ownobj->collhandler = NULL;
		newplayer->ownobj->aihandler = (AI_HANDLER)AI_GhostCarAiHandler;
	}
	else if (Type == PLAYER_REMOTE)
	{
		newplayer->conhandler = (CON_HANDLER)CON_LocalCarControl;
		newplayer->ownobj->CollType = COLL_TYPE_CAR;
		newplayer->ownobj->movehandler = (MOVE_HANDLER)MOV_MoveCarNew;
		newplayer->ownobj->collhandler = (COLL_HANDLER)COL_CarCollHandler;
		newplayer->ownobj->aihandler = (AI_HANDLER)AI_RemoteAiHandler;
	}

#endif
	else 
	{
		newplayer->ownobj->CollType = COLL_TYPE_NONE;
		newplayer->ownobj->movehandler = NULL;
		newplayer->ownobj->collhandler = NULL;
		newplayer->ownobj->aihandler = NULL;
	}

	AI_InitPlayerAI(newplayer);

	InitCar(&newplayer->car);
	SetupCar(newplayer, CarType);
	SetCarPos(&newplayer->car, Pos, Mat);

#ifndef _PSX
	CAI_InitCarAI(newplayer, 0);
#endif

	NumPlayers++;

	newplayer->PickupCycleSpeed = 0;
	newplayer->PickupNum = 0;
	newplayer->ValidRailCamNode = -1;
	newplayer->LastValidRailCamNode = -1;

	return(newplayer);				// Success
}

//--------------------------------------------------------------------------------------------------------------------------

void PLR_SetPlayerType(PLAYER *player, PLAYER_TYPE type)
{
	player->type = type;

	if (type == PLAYER_LOCAL)
	{
#if defined(_N64)
		player->ctrltype = CTRL_TYPE_STD;
		CRD_InitPlayerControl(player, CTRL_TYPE_STD);
		player->conhandler = (CON_HANDLER)CON_LocalCarControl;
#elif defined(_PC)
		player->ctrltype = PLR_LocalCtrlType;
		CRD_InitPlayerControl(player, player->ctrltype);
		player->conhandler = (CON_HANDLER)CON_LocalCarControl;
#elif defined(_PSX)
		player->ctrltype = PLR_LocalCtrlType;
#endif
	}
	else if (type == PLAYER_CPU)
	{
		player->ctrltype = CTRL_TYPE_NONE;
#ifndef _PSX
		CRD_InitPlayerControl(player, player->ctrltype);
#endif
		player->conhandler = NULL;
	}
}

//--------------------------------------------------------------------------------------------------------------------------

void PLR_KillPlayer(PLAYER *Player)
{
	FreeCar(Player);
	OBJ_FreeObject(Player->ownobj);

	if (Player->prev != NULL)
	{						 
		(Player->prev)->next = Player->next;
	}
	else
    {
        PLR_PlayerHead = Player->next;
	}

	if (Player->next != NULL)
	{
		(Player->next)->prev = Player->prev;
	}
	else 
	{
		PLR_PlayerTail = Player->prev;
	}


	if (s_NextFreePlayer != NULL)
	{
		s_NextFreePlayer->prev = Player;
	}

	Player->next = s_NextFreePlayer;
	Player->prev = NULL;
	s_NextFreePlayer = Player;
	

	// Reinitialise
	Player->type = PLAYER_NONE;
	Player->ownobj = NULL;

	// Keep count
	NumPlayers--;
}

//--------------------------------------------------------------------------------------------------------------------------

void PLR_KillAllPlayers(void)
{
	int iP;

	for (iP = 0; iP < MAX_NUM_PLAYERS; iP++)
	{
		if (Players[iP].type != PLAYER_NONE)
		{
			PLR_KillPlayer(&Players[iP]);
		}
	}
}


/////////////////////////////////////////////////////////////////////
//
// CreatePlayerForceField:
//
/////////////////////////////////////////////////////////////////////

void CreatePlayerForceField(PLAYER *player) 
{
	BBOX bBox;
	VEC size;

	SetBBox(&bBox, -1.73f * CAR_RADIUS, 1.73f * CAR_RADIUS, -1.73f * CAR_RADIUS, 1.73f * CAR_RADIUS, -1.73f * CAR_RADIUS, 1.73f * CAR_RADIUS);
	SetVec(&size, CAR_RADIUS, CAR_RADIUS, CAR_RADIUS);

	player->ownobj->Field = AddLocalField(
		player->ownobj->ObjID,
		FIELD_PRIORITY_MAX, 
		&player->car.Body->Centre.Pos,
		&player->car.Body->Centre.WMatrix,
		&bBox, 
		&size,
		&player->car.FieldVec,
		ONE,
		ZERO);
}

