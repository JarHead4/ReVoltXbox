/*********************************************************************************************
 *
 * ai_car.h
 *
 * Re-Volt (Generic) Copyright (c) Probe Entertainment 1998
 * 
 * Contents:
 *			Car AI
 *
 *********************************************************************************************
 *
 * 02/07/98 Matt Taylor
 *	File inception.
 *
 *********************************************************************************************/

#ifndef _AI_CAR_H_
#define _AI_CAR_H_

#include "ainode.h"

//
// Defines and macros
//

#define MAX_RESET_CNT	100
#define MAX_STUCK_CNT	3			// Max time allowed to be stuck (in seconds)

typedef enum
{
	CAI_S_RACE = 0,
	CAI_S_CORRECT,
	CAI_S_BLOCK,
	CAI_S_PICKUP,
	CAI_S_OVERTAKE,
	CAI_S_REVESC,
	CAI_S_CARAVOID,
	CAI_S_OBSTAVOID,
	CAI_S_FLEE,
	CAI_S_BACKTRACK,
} CAI_STATE;


typedef enum
{
	CAI_T_FORWARD = 0,
	CAI_T_BACKWARD,
	CAI_T_REVERSE_F,
	CAI_T_REVERSE_B
} CAI_TMODE;


typedef enum
{
	CAI_SK_DEFAULT,
	MAX_SKILL_TYPES
} CAI_SK_TYPES;



//
// Typedefs and structures
//

typedef struct _CAI_SKILLS
{
	long	RaceBias;							// Bias for lerp between racing line and track centre
	long	PriBias;							// Bias for random priority choices (0-15)
	long	PickupBias;							// Base desire for pick-ups
	long	BlockBias;							// Base desire to block
	long	OvertakeBias;						// Base desire to overtake
} CAI_SKILLS;

// Car Ai structure

typedef struct _CAR_AI
{
	long	 		IsInZone;					// true if car is within a zone
	long	 		IsOnTrack;					// true if car is on valid track

	long	 		ZoneID; 					// current race zone ID
	long	 		CurZone;					// current zone occupied by the car
	long	 		CurZoneBBox;				// current "sub-zone" occupied by the car
	long	 		LastValidZone;				// last valid zone ID
	struct _AINODE	*CurNode;					// current AI forward node being tracked
	struct _AINODE	*LastNode;					// last valid node
	REAL	 		NodeDist;					// distance to nearest forward node

	long	 		PriChoice;					// current choice (0 or 1) of priority for route splits
	long	 		RouteChoice;				// current choice (0 to MAX_AINODE_LINKS-1) for route splits

						  						// "Intelligence"
	CAI_STATE		AIState;	   				// Current AI state
	CAI_TMODE		TrackMode;					// Current node tracking mode
	CAI_SKILLS		Skills;						// Skill bias structure

	REAL	 		ResetCnt;					// counter for car "resetting" (off track, out of zone)
	REAL	 		StuckCnt;					// counter for car "resetting" (stuck)

	long			WrongWay;					// true if car facing wrong way
	long			FinishDistNode;				// current 'finish dist' node
	REAL	 		FinishDist;					// distance to finish line
	REAL	 		FinishDistPanel;			// distance to finish line for control panel
} CAR_AI;

//
// External global variables
//


//
// External function prototypes
//

extern void CAI_InitCarAI(struct PlayerStruct *Player, long Skill);
extern void CAI_CarHelper(struct PlayerStruct *Player);
extern void CAI_ResetCar(struct PlayerStruct *Player);
extern bool CAI_IsCarStuck(struct PlayerStruct *Player);
extern bool CAI_IsCarInZone(struct PlayerStruct *Player);
extern bool CAI_IsCarOnTrack(struct PlayerStruct *Player);
extern void CAI_TriggerAiHome(struct PlayerStruct *Player, long flag, long n, VEC *vec);

#endif