/*********************************************************************************************
 *
 * ctrlread.cpp
 *
 * Re-Volt (PC) Copyright (c) Probe Entertainment 1998
 * 
 * Contents:
 *			Controller reading code (for mouse, keyboard, etc)
 *
 *********************************************************************************************
 *
 * 03/03/98 Matt Taylor
 *  File inception.
 *	Moved over code from control.cpp. control.cpp now used for processing controller inputs
 *	into game controls. Controller inputs are generated in this file. Local key handling is
 *	done here without being passed to the game code itself (ie. for car model changes, etc).
 *
 *********************************************************************************************/

#include "revolt.h"
#include "main.h"
#include "dx.h"
#include "geom.h"
#include "particle.h"
#include "texture.h"
#include "model.h"
#include "aerial.h"
#include "play.h"
#include "NewColl.h"
#include "Body.h"
#include "car.h"
#include "input.h"
#include "camera.h"
#include "light.h"
#include "visibox.h"
#include "level.h"
#include "ctrlread.h"
#include "object.h"
#include "obj_init.h"
#include "control.h"
#include "player.h"
#include "editobj.h"
#include "aizone.h"
#include "timing.h"
#include "ghost.h"
#include "registry.h"
#include "panel.h"
#include "move.h"
#include "Readinit.h"


extern char *CarInfoFile;
//
// Static variable declarations
//


//
// Static function prototypes
//

static void s_RationaliseControl(CTRL *Control);

//
// Global function prototypes
//

void CRD_CheckLocalKeys(void);
void CRD_InitPlayerControl(PLAYER *player, CTRL_TYPE CtrlType);
void CRD_KeyboardInput(CTRL *Control);
void CRD_JoystickInput(CTRL *Control);

//--------------------------------------------------------------------------------------------------------------------------

//
// CRD_CheckLocalKeys
//
// Checks local keypresses for test suspension on the specified car, change model, and reset car position
//

void CRD_CheckLocalKeys(void)
{
	CAR		*car;
#if CHRIS_EXTRAS
	REAL	impPos, impMag;
	VEC		vPos, vImp;
#endif

	car = &PLR_LocalPlayer->car;

	if (car == NULL)
	{
		return;			// Local player is not controlling a car!
	}

	// fuck up cheating rectums
	if (Keys[DIK_X] || Keys[DIK_Y] || Keys[DIK_Z] || Keys[DIK_LCONTROL])
		car->CurrentLapStartTime -= MS2TIME(1000);

	// toggle solid ghost?
	if (Keys[DIK_8] && !LastKeys[DIK_8])
		GhostSolid = !GhostSolid;

	// change car
	if (Keys[DIK_PGUP] && !LastKeys[DIK_PGUP] && Everything)
	{
		GameSettings.CarID = PrevValidCarID(GameSettings.CarID);
		SetupCar(PLR_LocalPlayer, GameSettings.CarID);
		SetCarAerialPos(car);
		car->CurrentLapStartTime -= 10000;
	}

	if (Keys[DIK_PGDN] && !LastKeys[DIK_PGDN] && Everything)
	{
		GameSettings.CarID = NextValidCarID(GameSettings.CarID);
		SetupCar(PLR_LocalPlayer, GameSettings.CarID);
		SetCarAerialPos(car);
		car->CurrentLapStartTime -= 10000;
	}

	// ReRead the carinfo file
	if (Keys[DIK_LSHIFT] && Keys[DIK_F1] && !LastKeys[DIK_F1] && Everything) {
		if (!ReadAllCarInfo(CarInfoFile)){
			QuitGame = TRUE;
			return;
		}
		SetAllCarCoMs();
	}

	// change camera?
	if (Keys[DIK_F1] && !LastKeys[DIK_F1]) {
		long subType;
		if (CAM_MainCamera->Type != CAM_FOLLOW) {
			subType = 0;
		} else {
			subType = ++(CAM_MainCamera->SubType);
			if (subType >= CAM_FOLLOW_NTYPES) subType = 0;
		}
		SetCameraFollow(CAM_MainCamera, PLR_LocalPlayer->ownobj, subType);
	}

	if (Keys[DIK_F2] && !LastKeys[DIK_F2]) {
		long subType;
		if (CAM_MainCamera->Type != CAM_ATTACHED) {
			subType = 0;
		} else {
			subType = CAM_MainCamera->SubType + 1;
			if (subType >= CAM_ATTACHED_NTYPES) subType = 0;
		}
		SetCameraAttached(CAM_MainCamera, PLR_LocalPlayer->ownobj, subType);
	}

	if (Keys[DIK_F3] && !LastKeys[DIK_F3]) {
		if (CAM_MainCamera->Type != CAM_FREEDOM) {
			SetCameraFreedom(CAM_MainCamera, NULL, 0);
		} else {
			if (CAM_MainCamera->Object == NULL) {
				SetCameraFreedom(CAM_MainCamera, PLR_LocalPlayer->ownobj, 0);
			} else {
				SetCameraFreedom(CAM_MainCamera, NULL, 0);
			}
		}
	}

	if (Keys[DIK_F4] && EditMode) SetCameraEdit(CAM_MainCamera, NULL, 0);

	if (Keys[DIK_F5] && !LastKeys[DIK_F5]) {
		long subType;
		if (CAM_MainCamera->Type != CAM_RAIL) {
			subType = 0;
		} else {
			subType = CAM_MainCamera->SubType + 1;
			if (subType == CAM_RAIL_NTYPES) subType = 0;
		}
		SetCameraRail(CAM_MainCamera, PLR_LocalPlayer->ownobj, subType);
	}


	// Lock back wheels
	if (Keys[DIK_SPACE]) {
		SetWheelLocked(&car->Wheel[2]);
		SetWheelLocked(&car->Wheel[3]);
	} else {
		SetWheelNotLocked(&car->Wheel[2]);
		SetWheelNotLocked(&car->Wheel[3]);
	}

	// Turbo boost
	/*if (Keys[DIK_C] && Everything) {
		PLR_LocalPlayer->car.Body->Centre.Boost = Real(10000);
	} else {
		PLR_LocalPlayer->car.Body->Centre.Boost = ZERO;
	}*/

	// AutoBraking?
	if (Keys[DIK_4] && !LastKeys[DIK_4] && Everything) {
		GameSettings.AutoBrake = !GameSettings.AutoBrake;
	}

	// ReStart ghost car
	if (Keys[DIK_5] && Everything) {
		if (Keys[DIK_LSHIFT]) {
			ClearBestGhostData();
		}
		InitBestGhostData();
	}

	// Add another CPU car
	if (Keys[DIK_9] && !LastKeys[DIK_9] && Everything) {
		PLR_CreatePlayer(PLAYER_CPU, CTRL_TYPE_NONE, 0, &CAM_MainCamera->WPos, &IdentityMatrix);
	}

	// Take control of next computer car
	if (Keys[DIK_0] && !LastKeys[DIK_0] && Everything) {
		PLAYER *newPlayer;

		for (newPlayer = PLR_LocalPlayer->next; newPlayer != NULL; newPlayer = newPlayer->next) {
			if (newPlayer->type == PLAYER_CPU || newPlayer->type == PLAYER_LOCAL) {
				break;
			}
		}
		if (newPlayer == NULL) {
			for (newPlayer = PLR_PlayerHead; newPlayer != NULL; newPlayer = newPlayer->next) {
				if (newPlayer->type == PLAYER_CPU || newPlayer->type == PLAYER_LOCAL) {
					break;
				}
			}
		}
		if (newPlayer != NULL) {
			if (Keys[DIK_LSHIFT]) {
				PLR_KillPlayer(PLR_LocalPlayer);
			} else {
				PLR_SetPlayerType(PLR_LocalPlayer, newPlayer->type);
			}
			PLR_SetPlayerType(newPlayer, PLAYER_LOCAL);
			PLR_LocalPlayer = newPlayer;
			CAM_MainCamera->Object = PLR_LocalPlayer->ownobj;
		}
	}


#if PHYSICS_DEBUG_KEYS

	if (!Everything)
		return;

	// Change speed units 
	if (Keys[DIK_1] && !LastKeys[DIK_1]) {
		//CAR_WheelsHaveSuspension = !CAR_WheelsHaveSuspension;
		SpeedUnits++;
		if (SpeedUnits == SPEED_NTYPES) SpeedUnits = 0;
	}

	// Draw Car axes?
	if (Keys[DIK_2] && !LastKeys[DIK_2]) CAR_DrawCarAxes = !CAR_DrawCarAxes;

	// adjust the friction coefficient
	if (Keys[DIK_P] && !LastKeys[DIK_P]) {
		int ii;
		for (ii = 0; ii < CAR_NWHEELS; ii++) {
			car->Wheel[ii].StaticFriction += Real(0.1);
			Limit(car->Wheel[ii].StaticFriction, ZERO, Real(5.0));
		}
	}
	if (Keys[DIK_O] && !LastKeys[DIK_O]) {
		int ii;
		for (ii = 0; ii < CAR_NWHEELS; ii++) {
			car->Wheel[ii].StaticFriction -= Real(0.1);
			Limit(car->Wheel[ii].StaticFriction, ZERO, Real(5.0));
		}
	}
	if (Keys[DIK_L] && !LastKeys[DIK_L]) {
		int ii;
		for (ii = 0; ii < CAR_NWHEELS; ii++) {
			car->Wheel[ii].KineticFriction += Real(0.1);
			Limit(car->Wheel[ii].KineticFriction, ZERO, Real(5.0));
		}
	}
	if (Keys[DIK_K] && !LastKeys[DIK_K]) {
		int ii;
		for (ii = 0; ii < CAR_NWHEELS; ii++) {
			car->Wheel[ii].KineticFriction -= Real(0.1);
			Limit(car->Wheel[ii].KineticFriction, ZERO, Real(5.0));
		}
	}

	// Apply impulses to car from keypresses
#ifdef CHRIS_EXTRAS
	impPos = 10.0f;
	impMag = 100.0f;
	SetVec(&vPos, 0.0f, 0.0f, 0.0f);

	if (Keys[DIK_X]) {
		if (Keys[DIK_LSHIFT]) {
			SetVec(&vImp, -impMag, 0.0f, 0.0f);
			//VecMulMat(&vTmp, &car->Body.Centre.WMatrix, &vImp);
			ApplyBodyImpulse(car->Body, &vImp, &vPos);
		} else {
			SetVec(&vImp, impMag, 0.0f, 0.0f);
			//VecMulMat(&vTmp, &car->Body.Centre.WMatrix, &vImp);
			ApplyBodyImpulse(car->Body, &vImp, &vPos);
		}
	}
	if (Keys[DIK_Y]) {
		if (Keys[DIK_LSHIFT]) {
			SetVec(&vImp, 0.0f, -impMag, 0.0f);
			//VecMulMat(&vTmp, &car->Body.Centre.WMatrix, &vImp);
			ApplyBodyImpulse(car->Body, &vImp, &vPos);
		} else {
			SetVec(&vImp, 0.0f, impMag, 0.0f);
			//VecMulMat(&vTmp, &car->Body.Centre.WMatrix, &vImp);
			ApplyBodyImpulse(car->Body, &vImp, &vPos);
		}
	}
	if (Keys[DIK_Z]) {
		if (Keys[DIK_LSHIFT]) {
			SetVec(&vImp, 0.0f, 0.0f, -impMag);
			//VecMulMat(&vTmp, &car->Body.Centre.WMatrix, &vImp);
			ApplyBodyImpulse(car->Body, &vImp, &vPos);
		} else {
			SetVec(&vImp, 0.0f, 0.0f, impMag);
			//VecMulMat(&vTmp, &car->Body.Centre.WMatrix, &vImp);
			ApplyBodyImpulse(car->Body, &vImp, &vPos);
		}
	}


	if (Keys[DIK_B]) {
		if (Keys[DIK_LSHIFT]) {
			SetVec(&vPos, 0.0f, 0.0f, impPos);
			SetVec(&vImp, 0.0f, -impMag, 0.0f);
			ApplyBodyImpulse(car->Body, &vImp, &vPos);
			SetVec(&vPos, 0.0f, 0.0f, -impPos);
			SetVec(&vImp, 0.0f, impMag, 0.0f);
			ApplyBodyImpulse(car->Body, &vImp, &vPos);
		} else {
			SetVec(&vPos, 0.0f, 0.0f, impPos);
			SetVec(&vImp, 0.0f, impMag, 0.0f);
			ApplyBodyImpulse(car->Body, &vImp, &vPos);
			SetVec(&vPos, 0.0f, 0.0f, -impPos);
			SetVec(&vImp, 0.0f, -impMag, 0.0f);
			ApplyBodyImpulse(car->Body, &vImp, &vPos);
		}
	}
	if (Keys[DIK_N]) {
		if (Keys[DIK_LSHIFT]) {
			SetVec(&vPos, 0.0f, 0.0f, impPos);
			SetVec(&vImp, impMag, 0.0f, 0.0f);
			ApplyBodyImpulse(car->Body, &vImp, &vPos);
			SetVec(&vPos, 0.0f, 0.0f, -impPos);
			SetVec(&vImp, -impMag, 0.0f, 0.0f);
			ApplyBodyImpulse(car->Body, &vImp, &vPos);
		} else {
			SetVec(&vPos, 0.0f, 0.0f, -impPos);
			SetVec(&vImp, impMag, 0.0f, 0.0f);
			ApplyBodyImpulse(car->Body, &vImp, &vPos);
			SetVec(&vPos, 0.0f, 0.0f, impPos);
			SetVec(&vImp, -impMag, 0.0f, 0.0f);
			ApplyBodyImpulse(car->Body, &vImp, &vPos);
		}
	}
	if (Keys[DIK_M]) {
		if (Keys[DIK_LSHIFT]) {
			SetVec(&vPos, 0.0f, impPos, 0.0f);
			SetVec(&vImp, impMag, 0.0f, 0.0f);
			ApplyBodyImpulse(car->Body, &vImp, &vPos);
			SetVec(&vPos, 0.0f, -impPos, 0.0f);
			SetVec(&vImp, -impMag, 0.0f, 0.0f);
			ApplyBodyImpulse(car->Body, &vImp, &vPos);
		} else {
			SetVec(&vPos, 0.0f, -impPos, 0.0f);
			SetVec(&vImp, impMag, 0.0f, 0.0f);
			ApplyBodyImpulse(car->Body, &vImp, &vPos);
			SetVec(&vPos, 0.0f, impPos, 0.0f);
			SetVec(&vImp, -impMag, 0.0f, 0.0f);
			ApplyBodyImpulse(car->Body, &vImp, &vPos);
		}
	}
#endif

	// Stop motion
	if (Keys[DIK_T]) {
		SetVecZero(&car->Body->AngVel);
		SetVecZero(&car->Body->Centre.Vel);
	}

#endif

}


//--------------------------------------------------------------------------------------------------------------------------

void CRD_InitPlayerControl(PLAYER *player, CTRL_TYPE CtrlType)
{
	switch(CtrlType)
	{
		case CTRL_TYPE_KBD:
		player->ctrlhandler = (CTRL_HANDLER)CRD_KeyboardInput;
		break;

		case CTRL_TYPE_JOY:
		player->ctrlhandler = (CTRL_HANDLER)CRD_JoystickInput;
		break;

		case CTRL_TYPE_NONE:
		break;

		//CTRL_TYPE_KBDJOY:
		//CTRL_TYPE_MOUSE:
		//return(0);										// Not supported yet
		//break;

		default:
		break;
	};
}

//--------------------------------------------------------------------------------------------------------------------------


void CRD_KeyboardInput(CTRL *Control)
{
	if (!DetailMenuTogg)
	{
		if (GameSettings.Mirrored)
		{
			if (Keys[DIK_RIGHT])  { Control->digital += CTRL_LEFT;  }
			if (Keys[DIK_LEFT]) { Control->digital += CTRL_RIGHT; } 
		}
		else
		{
			if (Keys[DIK_LEFT])  { Control->digital += CTRL_LEFT;  }
			if (Keys[DIK_RIGHT]) { Control->digital += CTRL_RIGHT; } 
		}
		if (Keys[DIK_UP] || Keys[DIK_F])    { Control->digital += CTRL_FWD;   }
		if (Keys[DIK_DOWN] || Keys[DIK_V])  { Control->digital += CTRL_BACK;  }

		if (Keys[DIK_HOME]) { Control->digital += CTRL_RESTART; }
		if (Keys[DIK_END]) { Control->digital += CTRL_RESET; }
		if (Keys[DIK_LCONTROL]) { Control->digital += CTRL_FIRE; }

	}
	s_RationaliseControl(Control);
}

//--------------------------------------------------------------------------------------------------------------------------

//
// s_RationaliseControl
//
// Removes control conflicts (eg. left + right) and converts key presses into postion deltas
// where required.
//

static void s_RationaliseControl(CTRL *Control)
{
	// Remove control clashes
	if ((Control->digital & CTRL_LR) == CTRL_LR)
	{
		Control->digital &= ~CTRL_LR;
	}

	if ((Control->digital & CTRL_FB) == CTRL_FB)
	{
		Control->digital &= ~CTRL_FB;
	}
	
	// Provide dx and dy values from the LEFT,RIGHT,FWD,BACK keys
	if (Control->digital & CTRL_LEFT)
	{
	    Control->dx = -CTRL_RANGE_MAX;
	}
	else
	if (Control->digital & CTRL_RIGHT)
	{
		Control->dx = CTRL_RANGE_MAX;
	}

	if (Control->digital & CTRL_FWD)
	{
		Control->dy = -CTRL_RANGE_MAX;
	}
	else
	if (Control->digital & CTRL_BACK)
	{			 
  		Control->dy = CTRL_RANGE_MAX;
	}
}

//--------------------------------------------------------------------------------------------------------------------------


void CRD_JoystickInput(CTRL *Control)
{
	long delta;

// gayness

	static long x= 0, y = 1;
	if (Keys[DIK_1] && !LastKeys[DIK_1] && Everything) x = (x + 1) % 6;
	if (Keys[DIK_2] && !LastKeys[DIK_2] && Everything) y = (y + 1) % 6;

// read keyboard

	CRD_KeyboardInput(Control);

// dx

	if (GameSettings.Mirrored) delta = Control->dx - ((long*)&JoystickState)[x];
	else delta = Control->dx + ((long*)&JoystickState)[x];

	if (delta < -CTRL_RANGE_MAX) delta = -CTRL_RANGE_MAX;
	else if (delta > CTRL_RANGE_MAX) delta = CTRL_RANGE_MAX;
	Control->dx = (char)delta;

// dy

//	delta = Control->dy + ((long*)&JoystickState)[y];

//	if (delta < -CTRL_RANGE_MAX) delta = -CTRL_RANGE_MAX;
//	else if (delta > CTRL_RANGE_MAX) delta = CTRL_RANGE_MAX;
//	Control->dy = (char)delta;

// digital

	if (JoystickState.rgbButtons[3])
		Control->dy = CTRL_RANGE_MAX;

	if (JoystickState.rgbButtons[1])
		Control->dy = -CTRL_RANGE_MAX;

	if (JoystickState.rgbButtons[3])
		Control->digital += CTRL_FIRE;

	if (JoystickState.rgbButtons[4])
		Control->digital += CTRL_RESET;
}
