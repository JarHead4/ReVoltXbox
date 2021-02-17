
#include "Revolt.h"
#include "ctrlread.h"
#ifdef _PC
#include "input.h"
#endif
#include "Model.h"
#ifdef _PC
#include "Play.h"
#endif
#include "Particle.h"
#include "Aerial.h"
#include "NewColl.h"
#include "Body.h"
#include "Car.h"
#ifdef _PC
#include "input.h"
#endif
#ifndef _PSX
#include "Main.h"
#include "level.h"
#endif
#include "object.h"
#include "player.h"
#include "geom.h"
#ifndef _PSX
#include "timing.h"
#endif
#include "camera.h"
#include "move.h"
#ifndef _PSX
#include "obj_init.h"
#include "AIZone.h"
#endif

//
// Global function prototypes
//

void CON_DoPlayerControl(void);
void CON_LocalCarControl(CTRL *Control, OBJECT *CarObj);

// pickup to weapon table

static long Pickup2WeaponTable[PICKUP_NUM] = {
	OBJECT_TYPE_SHOCKWAVE,
	OBJECT_TYPE_FIREWORK,
	OBJECT_TYPE_FIREWORK,
	OBJECT_TYPE_PUTTYBOMB,
	OBJECT_TYPE_WATERBOMB,
	OBJECT_TYPE_ELECTROPULSE,
	OBJECT_TYPE_OILSLICK_DROPPER,
	OBJECT_TYPE_CHROMEBALL,
	OBJECT_TYPE_TURBO,
};

//--------------------------------------------------------------------------------------------------------------------------

void CON_DoPlayerControl(void)
{
	PLAYER	*player;
	unsigned short lastdigital;

// loop thru players

	for (player = PLR_PlayerHead; player; player = player->next)
	{

// save last digital

		lastdigital = player->controls.digital;

// no control?

#ifdef _PC
		if (CountdownTime || GameSettings.Paws)
		{
			player->controls.dx = 0;
			player->controls.dy = 0;
			player->controls.digital = 0;
			player->controls.idigital = 0;
			continue;
		}
#endif

// zero inputs if local player

		if (player->type == PLAYER_LOCAL)
		{
			player->controls.dx = 0;
			player->controls.dy = 0;
			player->controls.digital = 0;
		}

// get inputs

		if (player->ctrlhandler)
		{
			player->ctrlhandler(&player->controls);
			player->controls.idigital = (player->controls.digital ^ lastdigital) & player->controls.digital;
		}

// lost power?

		if (player->car.PowerTimer)
		{
			player->controls.dy = 0;
		}

// act on inputs

		if (player->conhandler)
		{
			player->conhandler(&player->controls, player->ownobj);
		}
	}
}

//--------------------------------------------------------------------------------------------------------------------------

void CON_LocalCarControl(CTRL *Control, OBJECT *CarObj)
{
	CAR		*car;
	REAL	dest, step;
	VEC		vec, vec2;
	MAT		mat;
	long	flag;

	car = &CarObj->player->car;
	if (car == NULL)
	{
		return;					// Specified object is not a car
	}	

	// Set the angle of the wheels and engine voltage from
	// the position of the controls 

	// Set the angle of the steering wheel
	dest = ((ONE / CTRL_RANGE_MAX) * (REAL)(Control->dx));
#ifndef _PSX
	step = MulScalar(car->SteerRate, TimeStep);
#else
	step = MulScalar(car->SteerRate, TimeStep) << 1;
#endif
	if ((dest == ZERO) || (Sign(dest) != Sign(car->SteerAngle))) {
		step *= 2;
	}
#ifndef _PSX
	else {
		step *= ONE - (car->SteerModifier * VecDotVec(&car->Body->Centre.Vel, &car->Body->Centre.WMatrix.mv[L]) / car->TopSpeed);
	}
#endif
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
	step = MulScalar(car->EngineRate, TimeStep);
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

	// Reset car?
	if (Control->idigital & CTRL_RESET)
	{
#ifdef _PC
		if ((Keys[DIK_LSHIFT] || Keys[DIK_RSHIFT]) && Everything) {
			SetVector(&vec2, 0, 0, 256);
			RotTransVector(&CAM_MainCamera->WMatrix, &CAM_MainCamera->WPos, &vec2, &vec);

			CopyVec(&CAM_MainCamera->WMatrix.mv[R], &mat.mv[R]);
			SetVector(&mat.mv[U], 0, 1, 0);
			CrossProduct(&mat.mv[R], &mat.mv[U], &mat.mv[L]);
			SetCarPos(car, &vec, &mat);
		}
#endif		
		CarObj->player->car.RightingCollide = TRUE;
		CarObj->player->car.RightingReachDest = FALSE;
		CarObj->movehandler = (MOVE_HANDLER)MOV_RightCar;	
	}

	// Restart car?
#ifndef _PSX
	if (CarObj->player->controls.idigital & CTRL_RESTART && !CountdownTime)
	{
		GetCarGrid(0, &vec, &mat);
		SetCarPos(car, &vec, &mat);

		car->Laps = -1;
		CarObj->player->CarAI.ZoneID = AiZoneNumID - 1;

		CarObj->player->CarAI.FinishDistNode = AiStartNode;
		CarObj->player->CarAI.FinishDist = 0.0f;
		CarObj->player->CarAI.FinishDistPanel = 0.0f;

		CarObj->movehandler = (MOVE_HANDLER)MOV_MoveCarNew;

#ifdef _PC																	// !MT! TEMP OUT of N64 version
		CarObj->player->car.CurrentLapStartTime = CurrentTimer() - MS2TIME(MAKE_TIME(10, 0, 0));

		SaveTrackTimes(&LevelInf[GameSettings.Level]);
		LoadTrackTimes(&LevelInf[GameSettings.Level]);
#endif
	}
#else 
	if (CarObj->player->controls.idigital & CTRL_RESTART)
	{
		GetCarGrid(0, &vec, &mat);
		SetCarPos(car, &vec, &Identity);

		CarObj->movehandler = (MOVE_HANDLER)MOV_MoveCarNew;
	}
#endif

#ifdef _PC
	// release weapon?
	if (Control->idigital & CTRL_FIRE && CarObj->player->PickupNum)
	{
		flag = (long)CarObj->player;
 		CreateObject(&CarObj->player->car.Body->Centre.Pos, &CarObj->player->car.Body->Centre.WMatrix, Pickup2WeaponTable[CarObj->player->PickupType], &flag);

		if (--CarObj->player->PickupNum == 0) {
			CarObj->player->PickupType = PICKUP_NONE;
		}

	}
#endif
}

//--------------------------------------------------------------------------------------------------------------------------
