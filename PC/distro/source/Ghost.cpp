
#include "ReVolt.h"
#include "NewColl.h"
#include "Particle.h"
#include "Body.h"
#include "Wheel.h"
#include "Car.h"
#include "Geom.h"
#include "Main.h"
#include "Ctrlread.h"
#include "Object.h"
#include "Control.h"
#include "Player.h"
#include "level.h"
#include "Ghost.h"
#include "main.h"
#include "light.h"
#include "timing.h"

static GHOST_INFO GhostInfoStore1;
static GHOST_DATA GhostDataStore1[GHOST_DATA_MAX];
static GHOST_INFO GhostInfoStore2;
static GHOST_DATA GhostDataStore2[GHOST_DATA_MAX];

GHOST_INFO *GHO_BestGhostInfo = &GhostInfoStore1;
GHOST_DATA *BestGhostData = GhostDataStore1;
long GHO_BestFrame = 0;
long GhostSolid = FALSE;

GHOST_INFO *GhostInfo = &GhostInfoStore2;
GHOST_DATA *GhostData = GhostDataStore2;

bool	GHO_GhostExists = FALSE;
PLAYER	*GHO_GhostPlayer = NULL;

LIGHT *GhostLight;

REAL DBG_dt = ZERO;

/////////////////////////////////////////////////////////////////////
//
// InitWhiteweeData:
//
/////////////////////////////////////////////////////////////////////

void InitGhostData(PLAYER *player)
{

	GhostInfo->CarID = player->car.CarID;
	strncpy(GhostInfo->PlayerName, player->PlayerName, MAX_PLAYER_NAME); 
	GhostInfo->NFrames = 0;

}

void EndGhostData(PLAYER *player)
{
	GhostInfo->Time[GHOST_LAP_TIME] = player->car.LastLapTime;
}

void InitBestGhostData()
{
	if (!GHO_GhostPlayer)
		return;

	GHO_BestFrame = 0;
	GHO_GhostPlayer->car.CurrentLapTime = 0;
	GHO_GhostPlayer->car.CurrentLapStartTime = CurrentTimer();
	strncpy(GHO_BestGhostInfo->PlayerName, GHO_GhostPlayer->PlayerName, MAX_PLAYER_NAME);

	if (GHO_GhostPlayer->car.CarID != GHO_BestGhostInfo->CarID)
		SetupCar(GHO_GhostPlayer, GHO_BestGhostInfo->CarID);
}


void ClearBestGhostData()
{
	int iWheel;

	GHO_BestFrame = 0;
	GHO_BestGhostInfo->Time[GHOST_LAP_TIME] = MAKE_TIME(10, 0, 0);
	GHO_BestGhostInfo->CarID = 0;
	GHO_BestGhostInfo->PlayerName[0] = '\0';
	GHO_BestGhostInfo->NFrames = 1;

	BestGhostData[0].Time = MAKE_TIME(0, 0, 0);
	BestGhostData[1].Time = MAKE_TIME(10, 0, 0);
	CopyVec(&GHO_GhostPlayer->car.Body->Centre.Pos, &BestGhostData[0].Pos); 
#pragma warning(disable : 4244)
	BestGhostData[0].Quat.v[VX] = (char)(GHOST_VECTOR_SCALE * GHO_GhostPlayer->car.Body->Centre.Quat.v[VX]);
	BestGhostData[0].Quat.v[VY] = (char)(GHOST_VECTOR_SCALE * GHO_GhostPlayer->car.Body->Centre.Quat.v[VY]);
	BestGhostData[0].Quat.v[VZ] = (char)(GHOST_VECTOR_SCALE * GHO_GhostPlayer->car.Body->Centre.Quat.v[VZ]);
	BestGhostData[0].Quat.v[S] = (char)(GHOST_VECTOR_SCALE * GHO_GhostPlayer->car.Body->Centre.Quat.v[S]);
#pragma warning(default : 4244)

	for (iWheel = 0; iWheel < CAR_NWHEELS; iWheel++) {
		BestGhostData[0].WheelAngle[iWheel] = 0;
		BestGhostData[0].WheelPos[iWheel] = 0;
	}

	GHO_GhostExists = FALSE;
}

/////////////////////////////////////////////////////////////////////
//
// SwitchGhostDataStores: use to swap ghost data array pointers
// when a new best time is recorded
//
/////////////////////////////////////////////////////////////////////

void SwitchGhostDataStores()
{
	GHOST_DATA *tmpData;
	GHOST_INFO *tmpInfo;

	if (GhostInfo->NFrames == 0) return;

	tmpData = GhostData;
	tmpInfo = GhostInfo;
	GhostData = BestGhostData;
	GhostInfo = GHO_BestGhostInfo;
	BestGhostData = tmpData;
	GHO_BestGhostInfo = tmpInfo;
	if (GHO_BestGhostInfo->NFrames < GHOST_DATA_MAX) {
		BestGhostData[GHO_BestGhostInfo->NFrames].Time = GHO_BestGhostInfo->Time[GHOST_LAP_TIME];
	}


	GHO_GhostExists = TRUE;

}

/////////////////////////////////////////////////////////////////////
//
// StoreGhostData:
//
/////////////////////////////////////////////////////////////////////

#define GHOST_POS_DEVIATION		500.0f
#define GHOST_QUAT_DEVIATION	0.85f

bool StoreGhostData(CAR *car)
{
	int iWheel;
	GHOST_DATA *data;
	

	// Make sure there is enough space in the data array
	if (GhostInfo->NFrames >= GHOST_DATA_MAX) {
		return FALSE;
	}

	// Make sure enough time has passed
	if (car->CurrentLapTime - GhostData[GhostInfo->NFrames - 1].Time < GHOST_MIN_TIMESTEP) {
		return FALSE;
	}

	// Store current car data for later replay
	data = &GhostData[GhostInfo->NFrames];

	data->Time = car->CurrentLapTime;
	CopyVec(&car->Body->Centre.Pos, &data->Pos);

	data->Quat.v[VX] = (char)(GHOST_VECTOR_SCALE * car->Body->Centre.Quat.v[VX]);
	data->Quat.v[VY] = (char)(GHOST_VECTOR_SCALE * car->Body->Centre.Quat.v[VY]);
	data->Quat.v[VZ] = (char)(GHOST_VECTOR_SCALE * car->Body->Centre.Quat.v[VZ]);
	data->Quat.v[S] = (char)(GHOST_VECTOR_SCALE * car->Body->Centre.Quat.v[S]);

	for (iWheel = 0; iWheel < CAR_NWHEELS; iWheel++) {
		data->WheelPos[iWheel] = (char)(GHOST_WHEEL_SCALE * car->Wheel[iWheel].Pos);
		data->WheelAngle[iWheel] = (char)(GHOST_ANGLE_SCALE * car->Wheel[iWheel].TurnAngle);
	}

	// See if there was sufficient deviation from the interpolated values obtained from previous stored data
	if (GhostInfo->NFrames > 2) {
		REAL dt;
		VEC intPos, dR;
		QUATERNION intQuat, lastQuat, nextQuat;
		GHOST_DATA *lastData = &GhostData[GhostInfo->NFrames - 2];
		GHOST_DATA *nextData = &GhostData[GhostInfo->NFrames - 1];
		bool posOkay = FALSE;
		bool quatOkay = FALSE;

		// position
		VecMinusVec(&nextData->Pos, &lastData->Pos, &dR);
		dt = Real(nextData->Time - lastData->Time);
		if (dt > SMALL_REAL) {
			dt = Real(data->Time - lastData->Time) / dt;
		} else {
			dt = ZERO;
		}
		Assert(dt >= ZERO);
		VecPlusScalarVec(&lastData->Pos, dt, &dR, &intPos);

		VecMinusVec(&intPos, &car->Body->Centre.Pos, &dR);
		if (VecDotVec(&dR, &dR) < GHOST_POS_DEVIATION) {
			posOkay = TRUE;
		}

		// quaternion
		lastQuat.v[VX] = GHOST_VECTOR_INVSCALE * (REAL)(lastData->Quat.v[VX]);
		lastQuat.v[VY] = GHOST_VECTOR_INVSCALE * (REAL)(lastData->Quat.v[VY]);
		lastQuat.v[VZ] = GHOST_VECTOR_INVSCALE * (REAL)(lastData->Quat.v[VZ]);
		lastQuat.v[S] = GHOST_VECTOR_INVSCALE * (REAL)(lastData->Quat.v[S]);
		nextQuat.v[VX] = GHOST_VECTOR_INVSCALE * (REAL)(nextData->Quat.v[VX]);
		nextQuat.v[VY] = GHOST_VECTOR_INVSCALE * (REAL)(nextData->Quat.v[VY]);
		nextQuat.v[VZ] = GHOST_VECTOR_INVSCALE * (REAL)(nextData->Quat.v[VZ]);
		nextQuat.v[S] = GHOST_VECTOR_INVSCALE * (REAL)(nextData->Quat.v[S]);
		LerpQuat(&lastQuat, &nextQuat, dt, &intQuat);
		NormalizeQuat(&intQuat);
		if (QuatDotQuat(&intQuat, &car->Body->Centre.Quat) > GHOST_QUAT_DEVIATION) {
			quatOkay = TRUE;
		}

	
		if (posOkay && quatOkay) return FALSE;

	}

	GhostInfo->NFrames++;

	return TRUE;

}


/////////////////////////////////////////////////////////////////////
//
// InterpGhostData:
//
/////////////////////////////////////////////////////////////////////

void InterpGhostData(CAR *car)
{
	REAL dt;
	VEC dR;
	MAT tmpMat;
	QUATERNION lastQuat, nextQuat;
	int iWheel;

	GHOST_DATA *lastData, *nextData;

	// Figure out which element of the data is just behind current lap time
	while ((GHO_BestFrame > 0) && (car->CurrentLapTime < BestGhostData[GHO_BestFrame - 1].Time)) {
		GHO_BestFrame--;
	}
	while ((GHO_BestFrame < GHO_BestGhostInfo->NFrames - 1) && (car->CurrentLapTime > BestGhostData[GHO_BestFrame + 1].Time)) {
		GHO_BestFrame++;
	}
	lastData = &BestGhostData[GHO_BestFrame];
	nextData = &BestGhostData[GHO_BestFrame + 1];

	// Keep frame number within bounds of data (last item is used as a buffer)
	if (GHO_BestFrame > GHO_BestGhostInfo->NFrames - 3) {
		lastData = &BestGhostData[GHO_BestGhostInfo->NFrames - 2];
		nextData = &BestGhostData[GHO_BestGhostInfo->NFrames - 2];
		GHO_BestFrame = GHO_BestGhostInfo->NFrames - 2;
	}

	// Calculate new position
	CopyVec(&car->Body->Centre.Pos, &car->Body->Centre.OldPos);
	VecMinusVec(&nextData->Pos, &lastData->Pos, &dR);
	dt = Real(nextData->Time - lastData->Time);
	if (dt > SMALL_REAL) {
		dt = Real(car->CurrentLapTime - lastData->Time) / dt;
	} else {
		dt = ZERO;
	}
	if (dt < ZERO) dt = ZERO;
	if (dt > ONE) dt = ONE;

	DBG_dt = dt;

	VecPlusScalarVec(&lastData->Pos, dt, &dR, &car->Body->Centre.Pos);

	// Calculate ghost velocity
	if (TimeStep > SMALL_REAL) {
		VecMinusVec(&car->Body->Centre.Pos, &car->Body->Centre.OldPos, &car->Body->Centre.Vel);
		VecDivScalar(&car->Body->Centre.Vel, TimeStep);
	} else {
		SetVecZero(&car->Body->Centre.Vel);
	}

	lastQuat.v[VX] = GHOST_VECTOR_INVSCALE * (REAL)(lastData->Quat.v[VX]);
	lastQuat.v[VY] = GHOST_VECTOR_INVSCALE * (REAL)(lastData->Quat.v[VY]);
	lastQuat.v[VZ] = GHOST_VECTOR_INVSCALE * (REAL)(lastData->Quat.v[VZ]);
	lastQuat.v[S] = GHOST_VECTOR_INVSCALE * (REAL)(lastData->Quat.v[S]);
	nextQuat.v[VX] = GHOST_VECTOR_INVSCALE * (REAL)(nextData->Quat.v[VX]);
	nextQuat.v[VY] = GHOST_VECTOR_INVSCALE * (REAL)(nextData->Quat.v[VY]);
	nextQuat.v[VZ] = GHOST_VECTOR_INVSCALE * (REAL)(nextData->Quat.v[VZ]);
	nextQuat.v[S] = GHOST_VECTOR_INVSCALE * (REAL)(nextData->Quat.v[S]);
//	SLerpQuat(&lastQuat, &nextQuat, dt, &car->Body->Centre.Quat);
//	CopyQuat(&lastQuat, &car->Body->Centre.Quat);
	LerpQuat(&lastQuat, &nextQuat, dt, &car->Body->Centre.Quat);
	NormalizeQuat(&car->Body->Centre.Quat);
	QuatToMat(&car->Body->Centre.Quat, &car->Body->Centre.WMatrix);

	// Set the wheel positions
	car->Revs = ZERO;
	for (iWheel = 0; iWheel < CAR_NWHEELS; iWheel++) {
		car->Wheel[iWheel].Pos = GHOST_WHEEL_INVSCALE * (REAL)lastData->WheelPos[iWheel];
		car->Wheel[iWheel].TurnAngle = GHOST_ANGLE_INVSCALE * (REAL)lastData->WheelAngle[iWheel];
		
		// Wheel position
		VecPlusScalarVec(&car->WheelOffset[iWheel], car->Wheel[iWheel].Pos, &DownVec, &dR);
		VecMulMat(&dR, &car->Body->Centre.WMatrix, &car->Wheel[iWheel].WPos);
		VecPlusEqVec(&car->Wheel[iWheel].WPos, &car->Body->Centre.Pos);

		// Calculate wheel angular velocity
		car->Wheel[iWheel].AngVel = VecDotVec(&car->Body->Centre.Vel, &car->Body->Centre.WMatrix.mv[L]) / car->Wheel[iWheel].Radius;
		car->Wheel[iWheel].AngPos += TimeStep * car->Wheel[iWheel].AngVel;
		GoodWrap(&car->Wheel[iWheel].AngPos, ZERO, FULL_CIRCLE);
		car->Revs += car->Wheel[iWheel].AngVel;
		
		// Wheel matrix
		RotationY(&tmpMat, car->Wheel[iWheel].TurnAngle);
		MatMulMat(&tmpMat, &car->Body->Centre.WMatrix, &car->Wheel[iWheel].Axes);
		RotationX(&tmpMat, car->Wheel[iWheel].AngPos);
		MatMulMat(&tmpMat, &car->Wheel[iWheel].Axes, &car->Wheel[iWheel].WMatrix);
	}
	car->Revs *= Real(0.25);

	// set light params
	CopyVec(&car->Body->Centre.Pos, (VEC*)&GhostLight->x);
	GhostLight->r = 0;
	GhostLight->g = 32;
	GhostLight->b = 64;
}


/////////////////////////////////////////////////////////////////////
//
// LoadGhostData:
//
/////////////////////////////////////////////////////////////////////

bool LoadGhostData(LEVELINFO *levelInfo)
{
	size_t	nRead;
	FILE	*fp;

	// open the ghost data filename
	if (GameSettings.Mirrored)
		fp = fopen(GetLevelFilename(GHOST_FILENAME_MIRRORED, FILENAME_GAME_SETTINGS), "rb");
	else
		fp = fopen(GetLevelFilename(GHOST_FILENAME, FILENAME_GAME_SETTINGS), "rb");

	if (fp == NULL) {
		return FALSE;
	}

	// Read the ghost info header
	nRead = fread(GHO_BestGhostInfo, sizeof(GHOST_INFO), 1, fp);
	if (nRead < 1) {
		fclose(fp);
		return FALSE;
	}

	// Read the ghost data
	nRead = fread(BestGhostData, sizeof(GHOST_DATA), GHOST_DATA_MAX, fp);
	if (nRead < GHOST_DATA_MAX) {
		fclose(fp);
		return FALSE;
	}

	// Success!
	fclose(fp);
	return TRUE;
}


/////////////////////////////////////////////////////////////////////
//
// SaveGhostData
//
/////////////////////////////////////////////////////////////////////

bool SaveGhostData(LEVELINFO *levelInfo)
{
	int		nWritten;
	FILE	*fp;

	// Make sure there is some ghost data
	if (!GHO_GhostExists) {
		return FALSE;
	}

	// open the ghost data filename
	if (GameSettings.Mirrored) {
		fp = fopen(GetLevelFilename(GHOST_FILENAME_MIRRORED, FILENAME_GAME_SETTINGS), "wb");
	} else {
		fp = fopen(GetLevelFilename(GHOST_FILENAME, FILENAME_GAME_SETTINGS), "wb");
	}

	if (fp == NULL) {
		return FALSE;
	}

	// write the info 
	nWritten = fwrite(GHO_BestGhostInfo, sizeof(GHOST_INFO), 1, fp);
	if (nWritten < 1) {
		fclose(fp);
		return FALSE;
	}

	// write the data
	nWritten = fwrite(BestGhostData, sizeof(GHOST_DATA), GHOST_DATA_MAX, fp);
	if (nWritten < GHOST_DATA_MAX) {
		fclose(fp);
		return FALSE;
	}

	// Success!
	fclose(fp);
	return TRUE;
}

//////////////////////
//					//
// init ghost light //
//					//
//////////////////////

void InitGhostLight(void)
{
	if ((GhostLight = AllocLight()))
	{
		GhostLight->Reach = 768;
		GhostLight->Flag = LIGHT_FIXED | LIGHT_MOVING;
		GhostLight->Type = LIGHT_OMNI;
	}
}
