/*********************************************************************************************
 *
 * player.h
 *
 * Re-Volt (Generic) Copyright (c) Probe Entertainment 1998
 * 
 * Contents:
 * 			Player handling code
 *
 *********************************************************************************************
 *
 * 03/03/98 Matt Taylor
 *	File inception.
 *
 *********************************************************************************************/

#ifndef _PLAYER_H_
#define _PLAYER_H_

#include "ai_car.h"

//
// Defines and macros
//

typedef enum
{
	PLAYER_NONE = 0,
	PLAYER_LOCAL,
	PLAYER_REMOTE,
	PLAYER_CPU,
	PLAYER_GHOST
} PLAYER_TYPE;



//
// Typedefs and structures
//

// Player structure

typedef struct PlayerStruct
{
	long					Slot;					// slot number
	PLAYER_TYPE				type;
	struct PlayerStruct	   *prev;
	struct PlayerStruct	   *next;
	CTRL_TYPE				ctrltype;			// If LOCAL player, selected control method
	CTRL_HANDLER			ctrlhandler;		// Function that handles hardware inputs (keyboard, joystick, etc).
	CON_HANDLER     		conhandler;			// Function that handles controller inputs
	CTRL					controls;

	long					cartype;			// Car type player has selected and is using
	OBJECT					*ownobj;			// Object that player controls (may not be a car!)

	CAR						car;				// car structure
	CAR_MODEL				carmodels;			// car models
	struct _CAR_AI			CarAI;				// Car AI data

	long					score;				// Bomb tag score or race position
	long					lastscore;			// Last frame race position
	long					raceswon;			// Races/games won

	REAL					PickupCycleType;	// current pickup cycling type
	REAL					PickupCycleSpeed;	// current pickup cycling speed
	long					PickupType;			// current pickup type;
	long					PickupNum;			// current pickup num;
	OBJECT					*PickupTarget;		// target object for missiles etc

#ifdef _PC
	long			Ready;						// network game 'ready' flag
	char			PlayerName[MAX_PLAYER_NAME];
	DPID			PlayerID;
#endif

	long			ValidRailCamNode;			// Set to node number of valid rail camera or -1
	long			LastValidRailCamNode;

} PLAYER;

//
// External global variables
//

extern PLAYER	Players[MAX_NUM_PLAYERS];
extern PLAYER	*PLR_PlayerHead, *PLR_PlayerTail;
//extern long		MyPlayerNum;
extern long		NumPlayers;
extern PLAYER *PLR_LocalPlayer;
extern CTRL_TYPE PLR_LocalCtrlType;

//
// External function prototypes
//

extern void PLR_InitPlayers(void);
extern PLAYER *PLR_CreatePlayer(PLAYER_TYPE Type, CTRL_TYPE CtrlType, CAR_TYPE CarType, VEC *Pos, MAT *Mat);
extern void PLR_KillPlayer(PLAYER *Player);
extern void PLR_KillAllPlayers(void);
extern void PLR_SetPlayerType(PLAYER *player, PLAYER_TYPE type);


#endif /* _PLAYER_H_ */
