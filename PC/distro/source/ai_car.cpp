/*********************************************************************************************
 *
 * cai.cpp
 *
 * Re-Volt (Generic) Copyright (c) Probe Entertainment 1998
 * 
 * Contents:
 *			Car AI
 *
 *********************************************************************************************
 *
 * 02/07/98 Matt Taylor
 *  File inception.
 *
 *********************************************************************************************/

#include "revolt.h"
#include "main.h"
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
#include "aizone.h"
#include "ai.h"
#include "ai_init.h"
#include "ai_car.h"

//
// Static variables
//

CAI_SKILLS	s_Skills[MAX_SKILL_TYPES] = {
											{
												15,
												15,
												15,
												15,
											},
										};


//
// Global variables
//


//
// Global function prototypes
//

void CAI_CarHelper(PLAYER *Player);
void CAI_ResetCar(PLAYER *Player);
bool CAI_IsCarStuck(PLAYER *Player);
bool CAI_IsCarInZone(PLAYER *Player);
bool CAI_IsCarOnTrack(PLAYER *Player);

//--------------------------------------------------------------------------------------------------------------------------

//
// CAI_InitCarAI
//
// Initialises the AI variables for the specified player
//

void CAI_InitCarAI(PLAYER *Player, long Skill)
{
	Player->CarAI.ZoneID = 0;
	Player->CarAI.LastValidZone = 0;
	CAI_IsCarInZone(Player);

	Player->CarAI.PriChoice = 1;
	Player->CarAI.RouteChoice = 0;
	Player->CarAI.CurNode = AIN_GetForwardNode(Player, 0, (REAL *)&Player->CarAI.NodeDist);
	Player->CarAI.LastNode = Player->CarAI.CurNode;

	Player->CarAI.AIState = CAI_S_RACE;
	Player->CarAI.TrackMode = CAI_T_FORWARD;
	Player->CarAI.Skills = s_Skills[Skill];

	Player->CarAI.ResetCnt = 0;
	Player->CarAI.StuckCnt = 0;

	Player->CarAI.FinishDistNode = AiStartNode;
	Player->CarAI.FinishDist = 0.0f;
	Player->CarAI.FinishDistPanel = 0.0f;
}

//--------------------------------------------------------------------------------------------------------------------------

//
// CAI_CarHelper
//
// Checks the cars "condition" to see if it needs correcting. This includes, checking if the car is on valid track, in a valid
// zone and facing the right way, etc
//

void CAI_CarHelper(PLAYER *Player)
{
	Player->CarAI.IsInZone = CAI_IsCarInZone(Player);
	Player->CarAI.IsOnTrack = CAI_IsCarOnTrack(Player);

	if ((!Player->CarAI.IsInZone) || (!Player->CarAI.IsOnTrack))
	{
		Player->CarAI.ResetCnt++;
		if (Player->CarAI.ResetCnt == MAX_RESET_CNT)
		{
			CAI_ResetCar(Player);
		}
	}
	else
	{
		Player->CarAI.ResetCnt = 0;
	}

	if (CAI_IsCarStuck(Player))
	{
		Player->CarAI.StuckCnt += TimeStep;
		if (Player->CarAI.StuckCnt > MAX_STUCK_CNT)
		{
			CAI_ResetCar(Player);
		}
	}
	else
	{
		Player->CarAI.StuckCnt = ZERO;
	}
}

//--------------------------------------------------------------------------------------------------------------------------

//
// CAI_ResetCar
//
// Places car back on track at last valid node and zone, facing in direction of tavel
//

void CAI_ResetCar(PLAYER *Player)
{
	CAR	*car;

	car = &Player->car;
}

//--------------------------------------------------------------------------------------------------------------------------


//
// CAI_IsCarStuck
//
// Checks the cars orientation, velocity and state to determine if the car is "stuck", eg. crashed
//

bool CAI_IsCarStuck(PLAYER *Player)
{
	CAR	*car;

	car = &Player->car;

	if (car->NWheelFloorContacts == 0) {
		if ((car->Body->NWorldContacts != 0) || (car->Body->NoContactTime < 0.05) || (car->NWheelColls > 0)) {
			return TRUE;
		}
	}

	return(FALSE);
}

//--------------------------------------------------------------------------------------------------------------------------


//
// CAI_IsCarInZone
//
// Checks if the specified car is in a zone or not, returning TRUE if in a zone
//

bool CAI_IsCarInZone(PLAYER *Player)
{
	long	ii, jj, kk;
	long	flag;
	float	dist;
	AIZONE	*zone;
	CAR	*car;

	car = &Player->car;

	if (!AiZones) { return(FALSE); }								// If no AI zones load, return FALSE

	for (ii = 0; ii < AiZoneNumID; ii++)							// Check all car centre against all AI zones
	{
		for (jj = 0; jj < AiZoneHeaders[ii].Count; jj++)
		{
			zone = AiZoneHeaders[ii].Zones + jj;
			flag = 0;
			for (kk = 0; kk < 3; kk++)
			{
				dist = PlaneDist(&zone->Plane[kk], &car->Body->Centre.Pos);
				if (dist < -zone->Size[kk] || dist > zone->Size[kk])
				{
					flag = 1;
					break;
				}
			}
			if (!flag)
			{
				Player->CarAI.LastValidZone = Player->CarAI.CurZone;
				Player->CarAI.CurZone = ii;
				Player->CarAI.CurZoneBBox = jj;
				return(TRUE);
			}
		}
	}
	return(FALSE);
}

//--------------------------------------------------------------------------------------------------------------------------

//
// CAI_IsCarOnTrack
//
// Checks if the specified car is in contact with an OOB collison poly. Returns FALSE if yes.
//

bool CAI_IsCarOnTrack(PLAYER *Player)
{
	CAR	*car;

	car = &Player->car;

	return(TRUE);
}

//--------------------------------------------------------------------------------------------------------------------------

// AI Home trigger func

void CAI_TriggerAiHome(PLAYER *Player, long flag, long n, VEC *vec)
{
	CAR	*car;

	car = &Player->car;
}

//--------------------------------------------------------------------------------------------------------------------------
