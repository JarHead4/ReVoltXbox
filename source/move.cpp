/*********************************************************************************************
 *
 * move.cpp
 *
 * Re-Volt (Generic) Copyright (c) Probe Entertainment 1998
 * 
 * Contents:
 *			Object processing code
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
#include "control.h"
#include "level.h"
#include "player.h"
#include "Field.h"
#ifdef _PC
#include "ghost.h"
#endif

//
// Global function prototypes
//

void MOV_MoveObjects(void);
void MOV_MoveBody(OBJECT *bodyObj);
void MOV_MoveCarNew(OBJECT *CarObj);
void MOV_MoveGhost(OBJECT *CarObj);
void MOV_MoveTrain(OBJECT *obj);

//--------------------------------------------------------------------------------------------------------------------------

///////////////////
// move all objects
///////////////////

void MOV_MoveObjects(void)
{
	OBJECT	*obj;

	for (obj = OBJ_ObjectHead; obj; obj = obj->next)
	{
		if (obj->movehandler && obj->flag.Move)
		{
			obj->movehandler(obj);
		}
		obj->renderflag.visible = FALSE;
	}	
}

//--------------------------------------------------------------------------------------------------------------------------

/*void MOV_MoveCar(OBJECT *CarObj)
{
	VEC v;
	CAR	*car;

	if (CarObj->flag.Move)
	{
		car = CarObj->car;		

		CarBuildCollision(car);

// Get move vector

		SetVector(&v, 0, 0, car->Throttle * TimeFactor);
		RotVector(&car->TurnCarMatrix, &v, &car->Velocity);
		memcpy(&v, &car->Velocity, sizeof(VEC));

// Move wheels

		car->Wheel[FL].Gravity += 0.4f * TimeFactor;
		CarWheelMove(&car->Wheel[FL]);
		AddVector(&car->Wheel[FL].WorldPos, &v, &car->Wheel[FL].WorldPos);
		AddVector(&car->Wheel[FL].CollPoint, &v, &car->Wheel[FL].CollPoint);
		if (CarWheelCollision(&(car->Wheel[FL])))
		{
			car->Wheel[FL].Gravity = 0;
		}
		FixWheel(&car->Wheel[FL], &car->Wheel[BR], car->WheelDiag);

		car->Wheel[FR].Gravity += 0.4f * TimeFactor;
		CarWheelMove(&car->Wheel[FR]);
		AddVector(&car->Wheel[FR].WorldPos, &v, &car->Wheel[FR].WorldPos);
		AddVector(&car->Wheel[FR].CollPoint, &v, &car->Wheel[FR].CollPoint);
		if (CarWheelCollision(&(car->Wheel[FR])))
		{
			car->Wheel[FR].Gravity = 0;
		}
		FixWheel(&car->Wheel[FR], &car->Wheel[BL], car->WheelDiag);

		CarWheelMove(&car->Wheel[BL]);
		if (CarWheelCollision(&(car->Wheel[BL])))
		{
			car->Wheel[BL].Gravity = 0;
		}
		FixWheel(&car->Wheel[BL], &car->Wheel[FR], car->WheelDiag);

		CarWheelMove(&car->Wheel[BR]);
		if (CarWheelCollision(&(car->Wheel[BR])))
		{
			car->Wheel[BR].Gravity = 0;
		}
		FixWheel(&car->Wheel[BR], &car->Wheel[FL], car->WheelDiag);

		BuildCarFromWheels(car);

// car 2 car cobblers

		Car2Car(car);

// update aerial

		UpdateCarAerial2(car, TimeFactor / 72.0f);
	}
}*/

void MOV_MoveBody(OBJECT *bodyObj)
{
	UpdateBody(&bodyObj->body, TimeStep);
}

void MOV_MoveCarNew(OBJECT *carObj)
{
	int iWheel, nPowered;
	CAR *car = &carObj->player->car;

	Assert(carObj->Type == OBJECT_TYPE_CAR || carObj->Type == OBJECT_TYPE_TROLLEY);

	// Move the car according to the impulses applied to it
	UpdateBody(car->Body, TimeStep);
	CopyBBox(&car->Body->CollSkin.BBox, &car->BBox);

	// Move the wheels relative to the car 
	car->Revs = ZERO;
	nPowered = 0;
	for (iWheel = 0; iWheel < CAR_NWHEELS; iWheel++) {
		if (IsWheelPresent(&car->Wheel[iWheel])) {
			UpdateCarWheel(car, iWheel, TimeStep);
			AddPosRadToBBox(&car->BBox, &car->Wheel[iWheel].CentrePos, car->Wheel[iWheel].Radius);
		}
		// Calculate car revs
		if (IsWheelPowered(&car->Wheel[iWheel])) {
			car->Revs += car->Wheel[iWheel].AngVel;
			nPowered++;
		}
	}
	if (nPowered > 0) {
		car->Revs /= nPowered;
	}
#ifdef _PC
	if (carObj->player->type == PLAYER_REMOTE) {
		UpdateRemotePlayer(carObj->player);
	}
#endif

	// Move the Aerial
	UpdateCarAerial2(car, TimeStep);
}

void MOV_RightCar(OBJECT *obj)
{
	int iWheel;
	REAL lookLen;
	VEC dR;
	MAT	mat;
	CAR *car = &obj->player->car;

	Assert(obj->Type == OBJECT_TYPE_CAR);

	// Choose an orientation and destination position
	if (!car->Righting) {

		// New position
		VecPlusScalarVec(&car->Body->Centre.Pos, TO_LENGTH(Real(50)), &UpVec, &car->DestPos);

		// New look direction
		SetVec(&mat.mv[L], car->Body->Centre.WMatrix.m[LX], ZERO, car->Body->Centre.WMatrix.m[LZ]);
		lookLen = VecLen(&mat.mv[L]);
		if (lookLen > SMALL_REAL) {
			VecDivScalar(&mat.mv[L], lookLen);
		} else {
			SetVec(&mat.mv[L], ONE, ZERO, ZERO);
		}

		// Complete the matrix
		CopyVec(&DownVec, &mat.mv[U]);
		VecCrossVec(&mat.mv[U], &mat.mv[L], &mat.mv[R]);

		// Convert to quaternion
		MatToQuat(&mat, &car->DestQuat);

		// set the self-righting flag
		car->Righting = TRUE;
	}

	// Reset some car status variables
	SetVecZero(&car->Body->Centre.Vel);
	SetVecZero(&car->Body->Centre.Impulse);
	SetVecZero(&car->Body->AngVel);
	SetVecZero(&car->Body->AngImpulse);
	if (!car->RightingCollide) {
		SetVecZero(&car->Body->Centre.Shift);
	}

	// Right the car
	VecMinusVec(&car->DestPos, &car->Body->Centre.Pos, &dR);
#ifndef _PSX
	VecPlusEqScalarVec(&car->Body->Centre.Pos, TimeStep * 10, &dR);
	SLerpQuat(&car->Body->Centre.Quat, &car->DestQuat, TimeStep * 3, &car->Body->Centre.Quat);
#else
	VecPlusEqScalarVec(&car->Body->Centre.Pos, Real(0.12), &dR);
	LerpQuat(&car->Body->Centre.Quat, &car->DestQuat, Real(0.12), &car->Body->Centre.Quat);
#endif
	UpdateBody(car->Body, TimeStep);

	for (iWheel = 0; iWheel < CAR_NWHEELS; iWheel++) {
		if (IsWheelPresent(&car->Wheel[iWheel])) {
			UpdateCarWheel(car, iWheel, TimeStep);
		}
	}
	car->Revs = ZERO;

	// Move the Aerial
	UpdateCarAerial2(car, TimeStep);

	// Check to see if the destination has been reached
	if (car->RightingReachDest) {
		VecMinusVec(&car->Body->Centre.OldPos, &car->Body->Centre.Pos, &dR);
	} else {
		SetVecZero(&dR);
	}
	if ((VecDotVec(&dR, &dR) < SMALL_REAL) &&
		(QuatDotQuat(&car->Body->Centre.Quat, &car->DestQuat) > Real(0.9999)))
	{
		car->Righting = FALSE;
		obj->movehandler = (MOVE_HANDLER)MOV_MoveCarNew;
	}
}

#ifdef _PC
void MOV_MoveGhost(OBJECT *CarObj)
{
	// Interpolate the ghost car data
	InterpGhostData(&CarObj->player->car);

	// Move the aerial
	UpdateCarAerial2(&CarObj->player->car, TimeStep);
}
#endif

void MOV_MoveTrain(OBJECT *obj)
{
	VEC vec;

	// move train
	CopyVec(&obj->body.Centre.Pos, &obj->body.Centre.OldPos);
	obj->body.Centre.Pos.v[Z] -= TimeStep * 200.0f;
	if (obj->body.Centre.Pos.v[Z] < -11500)
		obj->body.Centre.Pos.v[Z] = -400;
	VecMinusVec(&obj->body.Centre.Pos, &obj->body.Centre.OldPos, &vec);
	TransCollPolys(obj->body.CollSkin.CollPoly, obj->body.CollSkin.NCollPolys, &vec);

	// update collision skin
	BuildWorldSkin(&obj->body.CollSkin, &obj->body.Centre.Pos, &obj->body.Centre.WMatrix);

}


