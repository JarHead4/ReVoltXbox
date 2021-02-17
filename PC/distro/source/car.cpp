#include "Revolt.h"

#ifndef _PSX
#include "Main.h"
#endif
#ifdef _PC
#include "input.h"
#endif
#include "Geom.h"
#include "NewColl.h"
#ifndef _PSX
#include "Model.h"
#endif
#include "Particle.h"
#include "Body.h"
#include "Aerial.h"
#include "Wheel.h"
#include "Car.h"
#ifndef _PSX
#include "ctrlread.h"
#endif
#include "object.h"
#include "Control.h"
#include "move.h"
#ifndef _PSX
#include "DrawObj.h"
#endif
#include "player.h"
#ifndef _PSX
#include "timing.h"
#endif
#ifdef _PC
#include "registry.h"
#include "sfx.h"
#endif
#ifndef _PSX
#include "Spark.h"
#endif
#include "weapon.h"


#if MSCOMPILER_FUDGE_OPTIMISATIONS
#pragma optimize("", off)
#endif

// globals


#ifdef _PSX
VEC		LEV_StartPos;
long		LEV_StartGrid;
extern CAR_MODEL Car_Model[];
#endif

#if USE_DEBUG_ROUTINES
extern long DEBUG_CollGrid;
#endif

CAR_INFO	*CarInfo = NULL;
REAL		MaxCannotMoveTime = 3;


long	NCarTypes = 0;

bool	CAR_WheelsHaveSuspension = TRUE;
bool	CAR_DrawCarBBoxes = FALSE;
bool	CAR_DrawCarAxes = FALSE;

VEC		SmokeVel = {ZERO, -72.0f, ZERO};
VEC		TurboVel = {ZERO, -64.0f, ZERO};

//
// Global function prototypes
//

void InitAllCars(void);
void SetupCar(struct PlayerStruct *player, int carType);
void SetCarPos(CAR *car, VEC *pos, MAT *mat);
void BuildTurnMatrices(CAR *car);
void Car2Car(CAR *MyCar);
void CarWorldColls(CAR *car);
void CarCarColls(CAR *car1, CAR *car2);
int DetectWheelBodyColls(CAR *car, NEWBODY *body);
int DetectWheelSphereColls(CAR *car, int iWheel, NEWBODY *body);
int DetectWheelConvexColls(CAR *car, int iWheel, NEWBODY *body);
int DetectWheelPolyColls(CAR *car, int iWheel, NEWBODY *body);
void InitRemoteCarData(CAR *car);
int NextValidCarID(int currentID);
int PrevValidCarID(int currentID);
void RemoveWheelColl(CAR *car, COLLINFO_WHEEL *collInfo);
COLLINFO_WHEEL *AddWheelColl(CAR *car, COLLINFO_WHEEL *newHead);


//--------------------------------------------------------------------------------------------------------------------------

// Car grid start positions

GRID_POS CarGridStarts[][MAX_NUM_PLAYERS] = {

// type 0

	{
		{TO_LENGTH(Real(-128)),	TO_LENGTH(Real(0)),		TO_LENGTH(Real(0.0f))},
		{TO_LENGTH(Real(128)),	TO_LENGTH(Real(-64)),	TO_LENGTH(Real(0.0f))},
		{TO_LENGTH(Real(-128)),	TO_LENGTH(Real(-128)),	TO_LENGTH(Real(0.0f))},
		{TO_LENGTH(Real(128)),	TO_LENGTH(Real(-192)),	TO_LENGTH(Real(0.0f))},
#ifdef _PC
		{-128,	-256,	0.0f},
		{128,	-320,	0.0f},
		{-128,	-384,	0.0f},
		{128,	-448,	0.0f},
		{-128,	-512,	0.0f},
		{128,	-576,	0.0f},
		{-128,	-640,	0.0f},
		{128,	-704,	0.0f},
#endif
	},
// type 1: 3 cars wide
	{
		{0,		0,		0},
		{-250,	0,		0},
		{250,	0,		0},
		{0,		-320,	0},
#ifdef _PC
		{-250,	-320,	0},
		{250,	-320,	0},
		{0,		-640,	0},
		{-250,	-640,	0},
		{250,	-640,	0},
#endif
	},
// type 2: 4 cars wide, straight
	{
		{-150,	0,		0},
		{-50,	0,		0},
		{50,	0,		0},
		{150,	0,		0},
#ifdef _PC
		{-150,	-256,	0},
		{-50,	-256,	0},
		{50,	-256,	0},
		{150,	-256,	0},
#endif
	},
// type 3: 2 cars wide, straight
	{
		{-200,	0,		0},
		{200,	0,		0},
		{-200,	-320,	0},
		{200,	-320,	0},
#ifdef _PC
		{-200,	-640,	0},
		{200,	-640,	0},
		{-200,	-960,	0},
		{200,	-960,	0},
		{-200,	-1080,	0},
		{200,	-1080,	0},
#endif
	}
};

//--------------------------------------------------------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////
// InitCar: set all pointers to NULL and zero any counts
/////////////////////////////////////////////////////////////////////

void InitCar(CAR *car)
{

// init model / coll misc

	InitBodyDefault(car->Body);
	car->Models = NULL;

// init lap stuff

	car->NextSplit = 0;
	car->Laps = 0;
	car->CurrentLapTime = 0;
	car->LastLapTime = 0;
	car->LastRaceTime = 0;
#ifdef _PC
	car->BestLapTime = MAKE_TIME(5, 0, 0);
	car->BestRaceTime = MAKE_TIME(60, 0, 0);
#endif
	car->Best0to15 = 100;
	car->Best0to25 = 100;
	car->Current0to15 = -ONE;
	car->Current0to25 = -ONE;
	car->Timing0to15 = FALSE;
	car->Timing0to25 = FALSE;
	car->Righting = FALSE;
	car->AddLit = 0;
	car->DrawScale = ONE;
	car->PowerTimer = ZERO;
	car->IsBomb = FALSE;
	car->WillDetonate = FALSE;
	car->NoReturnTimer = ZERO;
}


/////////////////////////////////////////////////////////////////////
//
// NextValidCarID:
// PrevValidCarID:
//
/////////////////////////////////////////////////////////////////////

int NextValidCarID(int currentID)
{
	int carID;

	// Check all higher carIDs
	for (carID = currentID + 1; carID < NCarTypes; carID++) {
		if (CarInfo[carID].Selectable) {
			return carID;
		}
	}
	// Check lower car IDs
	for (carID = 0; carID < currentID; carID++) {
		if (CarInfo[carID].Selectable) {
			return carID;
		}
	}
	// No other selectables - return passed ID
	return currentID;
}

int PrevValidCarID(int currentID)
{
	int carID;

	// Check all lower carIDs
	for (carID = currentID - 1; carID >= 0; carID--) {
		if (CarInfo[carID].Selectable) {
			return carID;
		}
	}
	// Check higher car IDs
	for (carID = NCarTypes - 1; carID > currentID; carID--) {
		if (CarInfo[carID].Selectable) {
			return carID;
		}
	}
	// No other selectables - return passed ID
	return currentID;
}

/////////////////////////////////////////////////////////////////////
// SetupCar: copy information for the car of type "carType" from the
// CarInfo storage space into the specified CAR structure
/////////////////////////////////////////////////////////////////////

void SetupCar(struct PlayerStruct *player, int carType)
{
	long	ii;
	CAR		*car = &player->car;

	// load models + tpage
#ifndef _PSX
	if (car->Models) {
		FreeOneCarModelSet(player);
	}
	car->Models = &player->carmodels;
	LoadOneCarModelSet(player, carType);
#else
	car->Models = &Car_Model[0];
#endif

	// Create / re-create the world-frame collision skins
	if (car->Body->CollSkin.WorldConvex != NULL) {
		DestroyConvex(car->Body->CollSkin.WorldConvex, car->Body->CollSkin.NConvex);
		car->Body->CollSkin.WorldConvex = NULL;
	}
	if (car->Body->CollSkin.OldWorldConvex != NULL) {
		DestroyConvex(car->Body->CollSkin.OldWorldConvex, car->Body->CollSkin.NConvex);
		car->Body->CollSkin.OldWorldConvex = NULL;
	}
	if (car->Body->CollSkin.WorldSphere != NULL) {
		DestroySpheres(car->Body->CollSkin.WorldSphere);
		car->Body->CollSkin.WorldSphere = NULL;
	}
	if (car->Body->CollSkin.OldWorldSphere != NULL) {
		DestroySpheres(car->Body->CollSkin.OldWorldSphere);
		car->Body->CollSkin.OldWorldSphere = NULL;
	}
	car->Body->CollSkin.NConvex = car->Models->CollSkin.NConvex;
	car->Body->CollSkin.NSpheres = car->Models->CollSkin.NSpheres;
	car->Body->CollSkin.Convex = car->Models->CollSkin.Convex;
	car->Body->CollSkin.Sphere = car->Models->CollSkin.Sphere;
	CopyBBox(&car->Models->CollSkin.TightBBox, &car->Body->CollSkin.TightBBox);
	CreateCopyCollSkin(&car->Body->CollSkin);


#ifndef _PSX

	// fix env matrix
	CopyMatrix(&IdentityMatrix, &car->EnvMatrix);

#endif


	// Set car ID
	car->CarID = carType;

	// Set car misc stuff
	car->SteerRate = CarInfo[carType].SteerRate;
	car->SteerModifier = CarInfo[carType].SteerModifier;
	car->EngineRate = CarInfo[carType].EngineRate;
	car->TopSpeed = car->DefaultTopSpeed = CarInfo[carType].TopSpeed;
	car->MaxRevs = CarInfo[carType].MaxRevs;
	car->AllowedBestTime = CarInfo[carType].AllowedBestTime;
	car->DownForceMod = CarInfo[carType].DownForceMod;
	car->Selectable = CarInfo[carType].Selectable;
	CopyVec(&CarInfo[carType].WeaponOffset, &car->WeaponOffset);
	car->Best0to15 = -ONE;
	car->Best0to25 = -ONE;
	car->Current0to15 = ZERO;
	car->Current0to25 = ZERO;

	// Car Body Stuff
	SetBodyConvex(car->Body);
	CopyVec(&CarInfo[carType].Body.Offset, &car->BodyOffset);
	SetParticleMass(&car->Body->Centre, CarInfo[carType].Body.Mass);
	SetBodyInertia(car->Body, &CarInfo[carType].Body.Inertia);
	car->Body->Centre.Gravity = CarInfo[carType].Body.Gravity;
	car->Body->Centre.Hardness = CarInfo[carType].Body.Hardness;
	car->Body->Centre.Resistance = CarInfo[carType].Body.Resistance;
	car->Body->Centre.StaticFriction = CarInfo[carType].Body.StaticFriction;
	car->Body->Centre.KineticFriction = CarInfo[carType].Body.KineticFriction;
	car->Body->DefaultAngRes = CarInfo[carType].Body.AngResistance;
	car->Body->AngResistance = CarInfo[carType].Body.AngResistance;
	car->Body->AngResMod = CarInfo[carType].Body.ResModifier;
	car->Body->Centre.Grip = CarInfo[carType].Body.Grip;
	car->Body->AllowSparks = TRUE;

	// Jitter stuff
	car->Body->JitterCountMax = 2;
	car->Body->JitterFramesMax = 10;

	// Car Wheel Stuff
	for (ii = 0; ii < CAR_NWHEELS; ii++) {
		// Set the wheel offset and adjust for spring compression
		CopyVec(&CarInfo[carType].Wheel[ii].Offset1, &car->WheelOffset[ii]);
		CopyVec(&CarInfo[carType].Wheel[ii].Offset2, &car->WheelCentre[ii]);
		// Set up the rest of the wheel stuff
		SetupWheel(&car->Wheel[ii], &CarInfo[carType].Wheel[ii]);
	}


	// Suspension
	for (ii = 0; ii < CAR_NWHEELS; ii++) {
#ifndef _PSX
		car->Sus[ii].SpringLen = car->Models->SpringLen[ii];
		car->Sus[ii].AxleLen = car->Models->AxleLen[ii];
		car->Sus[ii].PinLen = car->Models->PinLen[ii];
#endif
		CopyVec(&CarInfo[carType].Spring[ii].Offset, &car->SuspOffset[ii]);
		CopyVec(&CarInfo[carType].Axle[ii].Offset, &car->AxleOffset[ii]);
		SetupSuspension(&car->Spring[ii], &CarInfo[carType].Spring[ii]);
	}

	// Set up the car's spinny thing
	CopyMat(&Identity, &car->Spinner.Matrix);
	CopyVec(&CarInfo[carType].Spinner.Offset, &car->SpinnerOffset);
	CopyVec(&CarInfo[carType].Spinner.Axis, &car->Spinner.Axis);
	car->Spinner.AngVel = CarInfo[carType].Spinner.AngVel;


	// Set up the car's aerial
	CopyVec(&CarInfo[carType].Aerial.Offset, &car->AerialOffset);
#ifndef _PSX
	InitAerial(&car->Aerial, 
		&car->Models->DirAerial, 
		car->Models->AerialLen, 
		1.0f, 1.0f, 0.00f, 100.0f);
#else 
	InitAerial(&car->Aerial, 
		&UpVec, 
		TO_LENGTH(20<<16), 
		1.0f, 1.0f, 0.00f, 100.0f);
#endif
	SetAerialSprings(&car->Aerial, 
		CarInfo[carType].Aerial.Stiffness, 
		CarInfo[carType].Aerial.Damping,
		6000.0f);



}

/////////////////////////////////////////////////////////////////////
//
// FreeCar: Deallocate any allocated ram set up in SetupCar
//
/////////////////////////////////////////////////////////////////////

void FreeCar(struct PlayerStruct *player)
{
	CAR *car = &player->car;

#ifndef _PSX
	FreeOneCarModelSet(player);
#endif
	DestroyConvex(car->Body->CollSkin.OldWorldConvex, car->Body->CollSkin.NConvex);
	DestroyConvex(car->Body->CollSkin.WorldConvex, car->Body->CollSkin.NConvex);
	car->Body->CollSkin.OldWorldConvex = NULL;
	car->Body->CollSkin.WorldConvex = NULL;

	DestroySpheres(car->Body->CollSkin.OldWorldSphere);
	DestroySpheres(car->Body->CollSkin.WorldSphere);
	car->Body->CollSkin.OldWorldSphere = NULL;
	car->Body->CollSkin.WorldSphere = NULL;
}

///////////////////////////////////
// get a grid position for a car //
///////////////////////////////////

void GetCarGrid(long position, VEC *pos, MAT *mat)
{
	GRID_POS *grid;
	VEC off;

// get grid pos

	grid = &CarGridStarts[LEV_StartGrid][position];

// get start matrix

#ifndef _PSX
	RotMatrixY(mat, -LEV_StartRot - grid->rotoff);
#else
	SetMatUnit(mat);
#endif

// transform start offset into start pos

	SetVector(&off, grid->xoff, 0, grid->zoff);
	VecMulMat(&off, mat, pos);
	VecPlusEqVec(pos, &LEV_StartPos);
	
}

///////////////////////////////////////
// set a cars position / orientation //
///////////////////////////////////////

void SetCarPos(CAR *car, VEC *pos, MAT *mat)
{

	int iWheel;

	// Set the body position and orientation
	SetBodyPos(car->Body, pos, mat);

	// Other stuff
	car->SteerAngle = ZERO;
	car->EngineVolt = ZERO;
	
	// Set Wheel orientation and positions
	for (iWheel = 0; iWheel < CAR_NWHEELS; iWheel++) {
		ResetCarWheelPos(car, iWheel);
	}

	// Set car's bounding box
	CopyBBox(&car->Body->CollSkin.BBox, &car->BBox);
	for (iWheel = 0; iWheel < CAR_NWHEELS; iWheel++) {
		AddPosRadToBBox(&car->BBox, &car->Wheel[iWheel].WPos, car->Wheel[iWheel].Radius);
	}

	// Set the starting aerial position
	SetCarAerialPos(car);


#ifdef _PC
	SetVecZero(&car->FieldVec);
	// Setup the remote car data stores
	InitRemoteCarData(car);
#endif
}

#ifdef _PC
/////////////////////////////////////////////////////////////////////
//
// InitRemoteCarData: initialise the remote car data to the current
// car's setup (which will be set up at the starting grid.
//
/////////////////////////////////////////////////////////////////////

void InitRemoteCarData(CAR *car)
{
	long i;

	car->OldDat = 0;
	car->Dat = 1;
	car->NewDat = 2;

	for (i = 0 ; i < 3 ; i++)
	{
		car->RemoteData[i].PacketInfo = 0xffffffff;
		car->RemoteData[i].NewData = FALSE;
		CopyVec(&car->Body->Centre.Pos, &car->RemoteData[i].Pos);
		CopyVec(&car->Body->Centre.Vel, &car->RemoteData[i].Vel);
		CopyVec(&car->Body->AngVel, &car->RemoteData[i].AngVel);
		CopyQuat(&car->Body->Centre.Quat, &car->RemoteData[i].Quat);
	}
}

/////////////////////////////////////////////////////////////////////
//
// NestRemoteData: return a pointer to the oldest remote data
// structure for filling with data from the broadcast packet.
// Also reflects the change in the data-order indices
//
// NOTE: if newset data has not been used, it is overwritten.
//
/////////////////////////////////////////////////////////////////////

REMOTE_DATA *NextRemoteData(CAR *car)
{
	int	tmp;

	// If the newest data has not been used, overwrite it
	if (car->RemoteData[car->NewDat].NewData) {
		return &car->RemoteData[car->NewDat];
	}

	// Shift data stores and return pointer to oldest for overwriting
	tmp = car->OldDat;
	car->OldDat = car->Dat;
	car->Dat = car->NewDat;
	car->NewDat = tmp;

	return &car->RemoteData[tmp];

}

/////////////////////////
// send local car data //
/////////////////////////

void SendLocalCarData(void)
{
	MESSAGE_HEADER *header = (MESSAGE_HEADER*)TransmitBuff;
	CAR *car = &PLR_LocalPlayer->car;
	short *sh;
	char *ptr = (CHAR*)(header + 1);

// setup header

	header->Type = MESSAGE_CAR_DATA;
	header->Contents = 0;

// set pos

	CopyVec(&car->Body->Centre.Pos, (VEC*)ptr);
	ptr += sizeof(VEC);
	header->Contents |= MESSAGE_CONTENTS_CAR_POS;

// set quat

	*ptr++ = (char)(car->Body->Centre.Quat.v[VX] * CAR_REMOTE_QUAT_SCALE);
	*ptr++ = (char)(car->Body->Centre.Quat.v[VY] * CAR_REMOTE_QUAT_SCALE);
	*ptr++ = (char)(car->Body->Centre.Quat.v[VZ] * CAR_REMOTE_QUAT_SCALE);
	*ptr++ = (char)(car->Body->Centre.Quat.v[S] * CAR_REMOTE_QUAT_SCALE);
	header->Contents |= MESSAGE_CONTENTS_CAR_QUAT;
	//CopyQuat(&car->Body->Centre.Quat, (QUATERNION*)ptr);
	//ptr += sizeof(QUATERNION);

// set vel

	sh = (short*)ptr;
	*sh++ = (short)(car->Body->Centre.Vel.v[X] * CAR_REMOTE_VEL_SCALE);
	*sh++ = (short)(car->Body->Centre.Vel.v[Y] * CAR_REMOTE_VEL_SCALE);
	*sh++ = (short)(car->Body->Centre.Vel.v[Z] * CAR_REMOTE_VEL_SCALE);
	header->Contents |= MESSAGE_CONTENTS_CAR_VEL;
	ptr = (char*)sh;
	//CopyVec(&car->Body->Centre.Vel, (VEC*)ptr);
	//ptr += sizeof(VEC);

// set ang vel

	sh = (short*)ptr;
	*sh++ = (short)(car->Body->AngVel.v[X] * CAR_REMOTE_ANGVEL_SCALE);
	*sh++ = (short)(car->Body->AngVel.v[Y] * CAR_REMOTE_ANGVEL_SCALE);
	*sh++ = (short)(car->Body->AngVel.v[Z] * CAR_REMOTE_ANGVEL_SCALE);
	header->Contents |= MESSAGE_CONTENTS_CAR_ANGVEL;
	ptr = (char*)sh;
	//CopyVec(&car->Body->AngVel, (VEC*)ptr);
	//ptr += sizeof(VEC);

// set control input

	*ptr++ = PLR_LocalPlayer->controls.dx;
	*ptr++ = PLR_LocalPlayer->controls.dy;
	header->Contents |= MESSAGE_CONTENTS_CAR_CONTROL;


// send

//if (Keys[DIK_SPACE])
//{
//	DP->
//}

//	CancelPriority(MESSAGE_PRIORITY_CAR);
	TransmitMessage(TransmitBuff, (short)(ptr - TransmitBuff), DPID_ALLPLAYERS, MESSAGE_PRIORITY_CAR);
}

#endif

/////////////////////////////////////////////////////////////////////
//
// SetCarAerialPos: set the aerial's starting coords from the car's
//
/////////////////////////////////////////////////////////////////////

#ifndef _PSX

void SetCarAerialPos(CAR *car)
{
	VEC vecTemp;
	VEC wDirection;
	int iSec, iCount;
	PARTICLE *pSection = &car->Aerial.Section[0];
	iCount = 0;

	// Set position of the base section and the aerial direction
	VecMulMat(&car->Aerial.Direction, &car->Body->Centre.WMatrix, &wDirection);
	VecMulMat(&car->AerialOffset, &car->Body->Centre.WMatrix, &vecTemp);
	VecPlusVec(&vecTemp, &car->Body->Centre.Pos, &pSection->Pos);

	SetVecZero(&pSection->Vel);
	SetVecZero(&pSection->Acc);
	SetVecZero(&pSection->Impulse);

	// Place all control sections in the correct place and set up their matrices
	for (iSec = AERIAL_START; iSec < AERIAL_NSECTIONS; iSec += AERIAL_SKIP) {
		iCount++;
		// Store a pointer to the current section (avoid excessive dereferencing)
		pSection = &car->Aerial.Section[iSec];

		// Set the section positions
		VecEqScalarVec(&pSection->Pos, MulScalar(iCount, car->Aerial.Length), &wDirection);
		VecPlusEqVec(&pSection->Pos, &car->Aerial.Section[0].Pos);

		// Zero the velocities
		SetVecZero(&pSection->Vel);
		SetVecZero(&pSection->Acc);
		SetVecZero(&pSection->Impulse);
		
	}
}

#else

void SetCarAerialPos(CAR *car)
{
	VEC vecTemp;
	VEC wDirection;
	int iSec, iCount;
	PARTICLE *pSection = &car->Aerial.Section[0];
	VEC Temp;

	MAT Matrix;


	Matrix.m[0] = car->Matrix.m[0][0] << 4;
	Matrix.m[1] = car->Matrix.m[1][0] << 4;
	Matrix.m[2] = car->Matrix.m[2][0] << 4;
	Matrix.m[3] = car->Matrix.m[0][1] << 4;
	Matrix.m[4] = car->Matrix.m[1][1] << 4;
	Matrix.m[5] = car->Matrix.m[2][1] << 4;
	Matrix.m[6] = car->Matrix.m[0][2] << 4;
	Matrix.m[7] = car->Matrix.m[1][2] << 4;
	Matrix.m[8] = car->Matrix.m[2][2] << 4;

	iCount = 0;
	
	// Set position of the base section and the aerial direction

	VecMulMat(&car->Aerial.Direction, &Matrix, &wDirection);
	
	VecMulMat(&car->AerialOffset, &Matrix, &vecTemp);


	Temp.v[X] = car->Pos.vx << 16;
	Temp.v[Y] = car->Pos.vy << 16;
	Temp.v[Z] = car->Pos.vz << 16;

	VecPlusVec(&vecTemp, &Temp, &pSection->Pos);

	SetVecZero(&pSection->Vel);
	SetVecZero(&pSection->Acc);
	SetVecZero(&pSection->Impulse);

	// Place all control sections in the correct place and set up their matrices

	for (iSec = AERIAL_START; iSec < AERIAL_NSECTIONS; iSec += AERIAL_SKIP)
	{
		iCount++;
		// Store a pointer to the current section (avoid excessive dereferencing)
		pSection = &car->Aerial.Section[iSec];

		// Set the section positions
		VecEqScalarVec(&pSection->Pos, MulScalar(iCount<<16, car->Aerial.Length), &wDirection);
		VecPlusEqVec(&pSection->Pos, &car->Aerial.Section[0].Pos);

		// Zero the velocities
		SetVecZero(&pSection->Vel);
		SetVecZero(&pSection->Acc);
		SetVecZero(&pSection->Impulse);
	}
}

#endif



/////////////////////////////////////////////////////////////////////
//
// UpdateCarAerial: update the aerial sections 
//
/////////////////////////////////////////////////////////////////////

#ifndef _PSX

void UpdateCarAerial2(CAR *car, REAL dt)
{
	int iSec;
	VEC vecTemp;
	VEC dRPrev;
	REAL prevLen;
	REAL scale;
	VEC dRThis;
	REAL thisLen;
	VEC thisCrossPrev;
	REAL crossLen;
	REAL velDotThis;
	REAL velDotDir;
	VEC velPar;
	VEC impulse = {ZERO, ZERO, ZERO};
	VEC dRTot = {ZERO, ZERO, ZERO};

	// Calculate the world position of the aerial base from the car's
	// world position and world matrix
	// Calculate the aerial's look direction and set the world matrix
	VecMulMat(&car->AerialOffset, &car->Body->Centre.WMatrix, &vecTemp);
	VecPlusVec(&vecTemp, &car->Body->Centre.Pos, &car->Aerial.Section[0].Pos);
	VecMulMat(&car->Aerial.Direction, &car->Body->Centre.WMatrix, &dRPrev);

	prevLen = ONE;

	// calculate the position of the controlled nodes (others are interpolated)
	for (iSec = AERIAL_START; iSec < AERIAL_NSECTIONS; iSec += AERIAL_SKIP) {

		// Update the position of the node
		UpdateParticle(&car->Aerial.Section[iSec], dt);

		// Calculate the length of this section of aerial and
		// the unit vector along this aerial section
		VecMinusVec(&car->Aerial.Section[iSec].Pos, &car->Aerial.Section[iSec - AERIAL_SKIP].Pos, &dRThis);
		thisLen = Length(&dRThis);

		VecDivScalar(&dRThis, thisLen);
		
		// Move aerial section to keep aerial length constant
		VecPlusEqScalarVec(&car->Aerial.Section[iSec].Pos, car->Aerial.Length - thisLen, &dRThis);

		// Calculate force on the end of the section due to the bend of the aerial
		VecCrossVec(&dRThis, &dRPrev, &thisCrossPrev);
		crossLen = VecLen(&thisCrossPrev);
		VecCrossVec(&dRThis, &thisCrossPrev, &vecTemp);
		scale = MulScalar(car->Aerial.Length, car->Aerial.Stiffness);
		scale = MulScalar(scale, crossLen);
		VecEqScalarVec(&impulse, -scale, &vecTemp);

		// add the spring damping
		VecMinusVec(&car->Aerial.Section[iSec].Vel, &car->Aerial.Section[0].Vel, &velPar);
		velDotDir = VecDotVec(&velPar, &dRPrev);
		VecPlusEqScalarVec(&velPar, -velDotDir, &dRPrev);
		VecPlusEqScalarVec(&impulse, -car->Aerial.Damping, &velPar);

		// Turn the force into an impulse
		VecMulScalar(&impulse, dt);

		// Apply the impulse to the node from the force
		ApplyParticleImpulse(&car->Aerial.Section[iSec], &impulse);

		// This node will now become previous node
		CopyVec(&dRThis, &dRPrev);
		prevLen = thisLen;

		// Fudge the position so that the section lengths are restored
		VecMinusVec(&car->Aerial.Section[iSec].Pos, &car->Aerial.Section[iSec - AERIAL_SKIP].Pos, &dRThis);
		thisLen = VecLen(&dRThis);
		
		VecDivScalar(&dRThis, thisLen);

		VecPlusEqScalarVec(&car->Aerial.Section[iSec].Pos, car->Aerial.Length - thisLen, &dRThis);

		// Check for collisions
#ifdef _PC
		ParticleWorldColls(&car->Aerial.Section[iSec]);
#endif

		// remove component of velocity along direction of the aerial
		VecMinusVec(&car->Aerial.Section[iSec].Vel, &car->Aerial.Section[iSec - AERIAL_SKIP].Vel, &vecTemp);
		velDotThis = VecDotVec(&vecTemp, &dRThis);
		VecPlusEqScalarVec(&car->Aerial.Section[iSec].Vel, -velDotThis, &dRThis);


	}

}

#else

void UpdateCarAerial2(CAR *car, REAL dt)
{
	int iSec;
	VEC vecTemp;
	VEC dRPrev;
	REAL prevLen;
	REAL scale;
	VEC dRThis;
	REAL thisLen;
	VEC thisCrossPrev;
	REAL crossLen;
	REAL velDotThis;
	REAL velDotDir;
	VEC velPar;
	VEC impulse = {ZERO, ZERO, ZERO};
	VEC dRTot = {ZERO, ZERO, ZERO};
	VEC Temp;

	MAT Matrix;

	Matrix.m[0] = car->Matrix.m[0][0]<<4;
	Matrix.m[1] = car->Matrix.m[1][0]<<4;
	Matrix.m[2] = car->Matrix.m[2][0]<<4;
	Matrix.m[3] = car->Matrix.m[0][1]<<4;
	Matrix.m[4] = car->Matrix.m[1][1]<<4;
	Matrix.m[5] = car->Matrix.m[2][1]<<4;
	Matrix.m[6] = car->Matrix.m[0][2]<<4;
	Matrix.m[7] = car->Matrix.m[1][2]<<4;
	Matrix.m[8] = car->Matrix.m[2][2]<<4;

	
	// Calculate the world position of the aerial base from the car's
	// world position and world matrix
	// Calculate the aerial's look direction and set the world matrix


	VecMulMat(&car->AerialOffset, &Matrix, &vecTemp);

	Temp.v[0] = car->Pos.vx << 16;
	Temp.v[1] = car->Pos.vy << 16;
	Temp.v[2] = car->Pos.vz << 16;
	VecPlusVec(&vecTemp, &Temp, &car->Aerial.Section[0].Pos);


	VecMulMat(&car->Aerial.Direction, &Matrix, &dRPrev);

	prevLen = ONE;

	// calculate the position of the controlled nodes (others are interpolated)
	for (iSec = AERIAL_START; iSec < AERIAL_NSECTIONS; iSec += AERIAL_SKIP) {

		// Update the position of the node
		UpdateParticle(&car->Aerial.Section[iSec], dt);

		// Calculate the length of this section of arial and
		// the unit vector along this aerial section
		VecMinusVec(&car->Aerial.Section[iSec - AERIAL_SKIP].Pos, &car->Aerial.Section[iSec].Pos, &dRThis);


		thisLen = Length(&dRThis);

		VecDivScalar(&dRThis, thisLen);

		// Move arial section to keep aerial length constant
		VecPlusEqScalarVec(&car->Aerial.Section[iSec].Pos, (car->Aerial.Length - thisLen), &dRThis);

		// Calculate force on the end of the section due to the bend of the aerial
		VecCrossVec(&dRThis, &dRPrev, &thisCrossPrev);
		crossLen = VecLen(&thisCrossPrev);
		VecCrossVec(&dRThis, &thisCrossPrev, &vecTemp);
		scale = MulScalar(car->Aerial.Length, car->Aerial.Stiffness);
		scale = MulScalar(scale, crossLen);
		VecEqScalarVec(&impulse, -scale, &vecTemp);

		// add the spring damping
		VecMinusVec(&car->Aerial.Section[iSec].Vel, &car->Aerial.Section[0].Vel, &velPar);
		velDotDir = VecDotVec(&velPar, &dRPrev);
		VecPlusEqScalarVec(&velPar, -velDotDir, &dRPrev);
		VecPlusEqScalarVec(&impulse, -car->Aerial.Damping, &velPar);

		// Turn the force into an impulse
		VecMulScalar(&impulse, dt);

		// Apply the impulse to the node from the force
		ApplyParticleImpulse(&car->Aerial.Section[iSec], &impulse);

		// This node will now become previous node
		CopyVec(&dRThis, &dRPrev);
		prevLen = thisLen;
	
		// Fudge the position so that the section lengths are restored
		VecMinusVec(&car->Aerial.Section[iSec].Pos, &car->Aerial.Section[iSec - AERIAL_SKIP].Pos, &dRThis);
		thisLen = VecLen(&dRThis);
		
		VecDivScalar(&dRThis, thisLen);

		VecPlusEqScalarVec(&car->Aerial.Section[iSec].Pos, car->Aerial.Length - thisLen, &dRThis);

		// remove component of velocity along direction of the aerial
		VecMinusVec(&car->Aerial.Section[iSec].Vel, &car->Aerial.Section[iSec - AERIAL_SKIP].Vel, &vecTemp);
		velDotThis = VecDotVec(&vecTemp, &dRThis);
		VecPlusEqScalarVec(&car->Aerial.Section[iSec].Vel, -velDotThis, &dRThis);

	}


	car->AerialPos[0] = car->Aerial.Section[0].Pos;
	car->AerialPos[2] = car->Aerial.Section[1].Pos;
	car->AerialPos[4] = car->Aerial.Section[2].Pos;

	Interpolate3D( &car->Aerial.Section[0].Pos, &car->Aerial.Section[1].Pos, &car->Aerial.Section[2].Pos, 16384, &car->AerialPos[1] );
	Interpolate3D( &car->Aerial.Section[0].Pos, &car->Aerial.Section[1].Pos, &car->Aerial.Section[2].Pos, 49152, &car->AerialPos[3] );

}


#endif



/////////////////////////////////////////////////////////////////////
//
// CreateCarInfo: allocate space for the required number of
// different car types
//
/////////////////////////////////////////////////////////////////////

CAR_INFO *CreateCarInfo(long nInfo)
{
	return (CAR_INFO *)malloc(sizeof(CAR_INFO) * nInfo);
}

void DestroyCarInfo() 
{
	if ((CarInfo == NULL) || (NCarTypes == 0)) return;
	free(CarInfo);
	CarInfo = NULL;
	NCarTypes = 0;
}

/////////////////////////////////////////////////////////////////////
//
// CreateCarModel: allocate space for the required number of
// different car types
//
/////////////////////////////////////////////////////////////////////

CAR_MODEL *CreateCarModels(long nModels)
{
	return (CAR_MODEL *)malloc(sizeof(CAR_MODEL) * nModels);
}

void DestroyCarModels(CAR_MODEL *carModels) 
{
	free(carModels);
}


/////////////////////////////////////////////////////////////////////
//
// ResetCarWheelPos: set initial wheel position, velocity etc;
//
/////////////////////////////////////////////////////////////////////

void ResetCarWheelPos(CAR *car, int iWheel)
{
	WHEEL	*wheel = &car->Wheel[iWheel];

	wheel->Pos = ZERO;
	wheel->Vel = ZERO;
	wheel->Acc = ZERO;
	wheel->Impulse = ZERO;
	wheel->AngPos = ZERO;
	wheel->AngVel = ZERO;
	wheel->AngAcc = ZERO;
	wheel->AngImpulse = ZERO;

	wheel->TurnAngle = ZERO;

	VecMulMat(&car->WheelOffset[iWheel], &car->Body->Centre.WMatrix, &wheel->WPos);
	VecPlusEqVec(&wheel->WPos, &car->Body->Centre.Pos);
	CopyVec(&wheel->WPos, &wheel->OldWPos);
	VecMulMat(&car->WheelCentre[iWheel], &car->Body->Centre.WMatrix, &wheel->CentrePos);
	VecPlusEqVec(&wheel->CentrePos, &wheel->WPos);
	CopyVec(&wheel->CentrePos, &wheel->OldCentrePos);

	// Build bounding box
	SetBBox(&wheel->BBox,
		Min(wheel->OldCentrePos.v[X], wheel->CentrePos.v[X]) - wheel->Radius,
		Max(wheel->OldCentrePos.v[X], wheel->CentrePos.v[X]) + wheel->Radius,
		Min(wheel->OldCentrePos.v[Y], wheel->CentrePos.v[Y]) - wheel->Radius,
		Max(wheel->OldCentrePos.v[Y], wheel->CentrePos.v[Y]) + wheel->Radius,
		Min(wheel->OldCentrePos.v[Z], wheel->CentrePos.v[Z]) - wheel->Radius,
		Max(wheel->OldCentrePos.v[Z], wheel->CentrePos.v[Z]) + wheel->Radius);

	CopyMat(&car->Body->Centre.WMatrix, &wheel->Axes);

}

/////////////////////////////////////////////////////////////////////
//
// UpdateWheel: update the position of the wheel according to its
// impulses
//
/////////////////////////////////////////////////////////////////////

void UpdateCarWheel(CAR *car, int iWheel, REAL dt)
{
	REAL scale;
	VEC tmpVec;
	VEC dR, centAcc;
	MAT tmpMat;
	WHEEL  *wheel = &car->Wheel[iWheel];
	SPRING *spring = &car->Spring[iWheel];

	// Apply torque from engine
	if ((IsWheelPowered(wheel) && !IsWheelInContact(wheel)) || (IsWheelSpinning(wheel) && IsWheelInContact(wheel))) {
		//wheel->AngImpulse += MulScalar(dt, MulScalar(car->EngineVolt, wheel->EngineRatio));
		wheel->AngImpulse += MulScalar(dt, MulScalar(car->EngineVolt, wheel->SpinAngImp));
	}

	// Get accelerations
	wheel->Acc = MulScalar(wheel->InvMass, wheel->Impulse);
	wheel->AngAcc = MulScalar(wheel->InvInertia, wheel->AngImpulse);

	if (CAR_WheelsHaveSuspension) {

		// Remove centripetal acceleration from the spin of the car
		// and up component of linear acceleration
#ifndef _PSX
		VecMinusVec(&wheel->WPos, &car->Body->Centre.Pos, &dR);
		VecCrossVec(&dR, &car->Body->AngVel, &tmpVec);
		VecCrossVec(&car->Body->AngVel, &tmpVec, &centAcc);
		wheel->Acc += MulScalar(dt, VecDotVec(&centAcc, &car->Body->Centre.WMatrix.mv[U]));
		wheel->Acc -= VecDotVec(&car->Body->Centre.Acc, &car->Body->Centre.WMatrix.mv[U]);
#endif

	}

	// Get new velocities
	wheel->Vel += wheel->Acc;
	wheel->AngVel += wheel->AngAcc;

	// Add damping
	scale = MulScalar(MulScalar(FRICTION_TIME_SCALE, dt), wheel->AxleFriction);
	wheel->AngVel = MulScalar(wheel->AngVel, (ONE - scale));

	// Get new positions
	wheel->Pos += MulScalar(wheel->Vel, dt);
	Limit(wheel->Pos, -wheel->MaxPos, wheel->MaxPos);
	wheel->AngPos += MulScalar(wheel->AngVel, dt);
	GoodWrap(&wheel->AngPos, ZERO, FULL_CIRCLE);

	if (CAR_WheelsHaveSuspension) {

		// Add spring forces
		if (!IsWheelInContact(wheel)) {
			wheel->Vel += MulScalar(wheel->InvMass, MulScalar(SpringDampedForce(spring, wheel->Pos, wheel->Vel), dt));
		} else {
			wheel->Vel += MulScalar(wheel->InvMass, MulScalar(SpringDampedForce(spring, wheel->Pos, wheel->Vel), dt)) / 2;
		}

	}

	// Set the wheel angles
	if (IsWheelTurnable(wheel)) {
		wheel->TurnAngle = MulScalar(car->SteerAngle, wheel->SteerRatio);
	}

	// Calculate the wheel's world matrix and position
	CopyVec(&wheel->WPos, &wheel->OldWPos);
	VecPlusScalarVec(&car->WheelOffset[iWheel], wheel->Pos, &DownVec, &tmpVec);
	VecMulMat(&tmpVec, &car->Body->Centre.WMatrix, &wheel->WPos);
	VecPlusEqVec(&wheel->WPos, &car->Body->Centre.Pos);

	CopyVec(&wheel->CentrePos, &wheel->OldCentrePos);
	VecMulMat(&car->WheelCentre[iWheel], &car->Body->Centre.WMatrix, &tmpVec);
	VecPlusVec(&wheel->WPos, &tmpVec, &wheel->CentrePos);

	// Spin the wheel according to its angular position
	RotationY(&tmpMat, wheel->TurnAngle);
	MatMulMat(&tmpMat, &car->Body->Centre.WMatrix, &wheel->Axes);
	RotationX(&tmpMat, wheel->AngPos);
	MatMulMat(&tmpMat, &wheel->Axes, &wheel->WMatrix);



	// Build bounding box
	SetBBox(&wheel->BBox,
		Min(wheel->OldCentrePos.v[X], wheel->CentrePos.v[X]) - wheel->Radius,
		Max(wheel->OldCentrePos.v[X], wheel->CentrePos.v[X]) + wheel->Radius,
		Min(wheel->OldCentrePos.v[Y], wheel->CentrePos.v[Y]) - wheel->Radius,
		Max(wheel->OldCentrePos.v[Y], wheel->CentrePos.v[Y]) + wheel->Radius,
		Min(wheel->OldCentrePos.v[Z], wheel->CentrePos.v[Z]) - wheel->Radius,
		Max(wheel->OldCentrePos.v[Z], wheel->CentrePos.v[Z]) + wheel->Radius);

	// Zero impulses
	wheel->Impulse = ZERO;
	wheel->AngImpulse = ZERO;

}



void DetectCarWheelColls2(CAR *car, int iWheel, NEWCOLLPOLY *worldPoly)
{
	VEC	tmpVec;//, tmpVec2;
	REAL	time;
	WHEEL	*wheel = &car->Wheel[iWheel];
	COLLINFO_WHEEL	*wheelColl;

	// Make sure we don't overstep the collision array
	if ((wheelColl = NextWheelCollInfo()) == NULL) return;

	// Quick swepth-volume axis-aligned bounding-box test
	if (!BBTestYXZ(&wheel->BBox, &worldPoly->BBox)) return;

	// Do a sphere-to-poly collision test
	if (SphereCollPoly(&wheel->OldCentrePos, &wheel->CentrePos,
		wheel->Radius, 
		worldPoly, 
		&wheelColl->Plane,
		&wheelColl->Pos,
		&wheelColl->WorldPos,
		&wheelColl->Depth,
		&time))
	{

		// Calculate the collision point on the plane (for skidmarks and smoke generator)
		VecPlusEqScalarVec(&wheelColl->WorldPos, SKID_RAISE, PlaneNormal(&wheelColl->Plane));

		// Calculate the car-relative collision point for response
		VecPlusEqVec(&wheelColl->Pos, &wheel->CentrePos);
		VecMinusEqVec(&wheelColl->Pos, &car->Body->Centre.Pos);

		// Calculate world velocity of the wheel collision point (not including wheel spin)
		VecCrossVec(&car->Body->AngVel, &wheelColl->Pos, &wheelColl->Vel);
		VecPlusEqVec(&wheelColl->Vel, &car->Body->Centre.Vel);

		// Make sure that the wheel is not already travelling away from the surface
		VecPlusScalarVec(&wheelColl->Vel, wheel->Vel, &car->Body->Centre.WMatrix.mv[U], &tmpVec);
		if (VecDotVec(&tmpVec, PlaneNormal(&wheelColl->Plane)) > ZERO) return;

		// Add bumps from surface corrugation
		wheelColl->Material = &COL_MaterialInfo[worldPoly->Material];
		AdjustWheelColl(wheelColl, wheelColl->Material);

		// Set other necessary stuff
		wheelColl->Car = car;
		wheelColl->IWheel = iWheel;
		wheelColl->Grip = MulScalar(wheel->Grip, wheelColl->Material->Gripiness);
		wheelColl->StaticFriction = MulScalar(wheel->StaticFriction, wheelColl->Material->Roughness);
		wheelColl->KineticFriction = MulScalar(wheel->KineticFriction, wheelColl->Material->Roughness);
		wheelColl->Restitution = ZERO;
		wheelColl->Body2 = &BDY_MassiveBody;
		SetVecZero(&wheelColl->Pos2);


		// Set the wheel-in-contact flag
		SetWheelInContact(wheel);
		SetWheelNotSpinning(wheel);

		// add collision to car's list
		AddWheelColl(car, wheelColl);

	}
}


/////////////////////////////////////////////////////////////////////
//
// PreProcessCarWheelColls: remove duplicate collisions
//
/////////////////////////////////////////////////////////////////////

void PreProcessCarWheelColls(CAR *car)
{
	bool	keepGoing;
	int		iOil;
	WHEEL	*wheel;
	COLLINFO_WHEEL	*wheelColl1, *wheelColl2;
	VEC		*collpos;
	REAL	dx, dz;

	for (wheelColl1 = car->WheelCollHead; wheelColl1 != NULL; wheelColl1 = wheelColl1->Next) {

		wheel = &car->Wheel[wheelColl1->IWheel];

#ifndef _PSX
		// check for oil slick contact
		collpos = &wheelColl1->WorldPos;

		for (iOil = 0 ; iOil < OilSlickCount ; iOil++)
		{
			if (collpos->v[Y] < OilSlickList[iOil].Ymin)
				continue;

			if (collpos->v[Y] > OilSlickList[iOil].Ymax)
				continue;

			dx = OilSlickList[iOil].X - collpos->v[X];
			if (abs(dx) > OilSlickList[iOil].Radius)
				continue;

			dz = OilSlickList[iOil].Z - collpos->v[Z];
			if (abs(dz) > OilSlickList[iOil].Radius)
				continue;

			if ((dx * dx + dz * dz) > OilSlickList[iOil].SquaredRadius)
				continue;

			SetWheelInOil(&wheelColl1->Car->Wheel[wheelColl1->IWheel]);
			break;
		}

		// Reduce friction if wheel is oiled up
		if (wheel->OilTime < OILY_WHEEL_TIME) {
			wheelColl1->StaticFriction *= HALF * (wheel->OilTime + TO_TIME(Real(0.2))) / (OILY_WHEEL_TIME + TO_TIME(Real(0.2)));
			wheelColl1->KineticFriction *= HALF * (wheel->OilTime + TO_TIME(Real(0.2))) / (OILY_WHEEL_TIME + TO_TIME(Real(0.2)));
		}
#endif


		// Merge close collisions with similar planes
		keepGoing = TRUE;
		for (wheelColl2 = wheelColl1->Next; (wheelColl2 != NULL) && keepGoing; wheelColl2 = wheelColl2->Next) {

			if (wheelColl1->IWheel != wheelColl2->IWheel) continue;

			//VecMinusVec(&wheelColl1->Pos, &wheelColl2->Pos, &tmpVec);
			//if (VecDotVec(&tmpVec, &tmpVec) < 25.0f) {
				if (VecDotVec(PlaneNormal(&wheelColl1->Plane), PlaneNormal(&wheelColl2->Plane)) > CLOSE_WHEEL_COLL_ANGLE) {

					if (wheelColl1->Depth > wheelColl2->Depth) {
						RemoveWheelColl(car, wheelColl1);
						keepGoing = FALSE;
					} else {
						RemoveWheelColl(car, wheelColl2);
					}

					//COL_NWheelDone++;
				}
			//}
		}
	}
}


/////////////////////////////////////////////////////////////////////
//
// ProcessCarWheelColls: calculate net impulse from each collision
// on the passed wheel on the car and add it to the car
//
// Calculate collision impulse as if it were a perfect-friction
// collision, then adjust the components along the look, right and
// up directions (which should be in the wheel's "TurnMatrix").
//
/////////////////////////////////////////////////////////////////////

void ProcessCarWheelColls(CAR *car)
{
	COLLINFO_WHEEL	*wheelColl;
	WHEEL	*wheel;
	REAL	shiftLen, shiftDotUp;
	VEC	collImpulse, tmpVec, shift;
	VEC	totImpulse = {0, 0, 0};
	VEC	totAngImpulse = {0, 0, 0};

	for (wheelColl = car->WheelCollHead; wheelColl != NULL; wheelColl = wheelColl->Next) {

		wheel = &car->Wheel[wheelColl->IWheel];
	
		// Calculate the shift required to extract the wheel from the road
		if (CAR_WheelsHaveSuspension) {
			if (wheelColl->Depth < ZERO) {
				// Calculate shift to extract wheel from road
				CopyVec(PlaneNormal(&wheelColl->Plane), &shift);
				VecMulScalar(&shift, -wheelColl->Depth);
				shiftDotUp = VecDotVec(&shift, &car->Body->Centre.WMatrix.mv[U]);
				wheel->Pos += shiftDotUp;
				if (wheel->Pos > wheel->MaxPos) {
					shiftDotUp -= wheel->Pos - wheel->MaxPos;
					wheel->Pos = wheel->MaxPos;
				} else if (wheel->Pos < -wheel->MaxPos) {
					shiftDotUp -= wheel->Pos + wheel->MaxPos;
					wheel->Pos = -wheel->MaxPos;
				}

				// Get shift needed to shift body out of road
				shiftDotUp = MulScalar(shiftDotUp, VecDotVec(PlaneNormal(&wheelColl->Plane), &car->Body->Centre.WMatrix.mv[U]));
				shiftLen = -wheelColl->Depth - shiftDotUp;
				if (shiftLen > ZERO) {
					ModifyShift(&car->Body->Centre.Shift, shiftLen, PlaneNormal(&wheelColl->Plane));
				}

				// Stop wheel relative to road
				VecCrossVec(&car->Body->AngVel, &wheelColl->Pos, &tmpVec);
				VecPlusEqVec(&tmpVec, &car->Body->Centre.Vel);
				wheel->Vel = -MulScalar(VecDotVec(PlaneNormal(&wheelColl->Plane), &car->Body->Centre.WMatrix.mv[U]), VecDotVec(&tmpVec, PlaneNormal(&wheelColl->Plane)));
				wheel->Vel += MulScalar(MulScalar(wheel->Gravity, TimeStep), VecDotVec(&DownVec, &car->Body->Centre.WMatrix.mv[U]));
			}
		} else {
			if (wheelColl->Depth < ZERO) {
				ModifyShift(&car->Body->Centre.Shift, -wheelColl->Depth, PlaneNormal(&wheelColl->Plane));
			}
		}

		// Calculate the impulse to apply to the car from the wheel
		CarWheelImpulse2(car, wheelColl, &collImpulse);

		// Calculate and store the linear and angular components of the impulse
		VecPlusEqVec(&totImpulse, &collImpulse);
		CalcAngImpulse(&collImpulse, &wheelColl->Pos, &tmpVec);
		VecPlusEqVec(&totAngImpulse, &tmpVec);

	}
	
	// Apply impulse from wheels to car body
	ApplyBodyAngImpulse(car->Body, &totAngImpulse);
	ApplyParticleImpulse(&car->Body->Centre, &totImpulse);

}


/////////////////////////////////////////////////////////////////////
//
// PostProcessCarWheelColls: if the wheels are spinning and in contact
// with something, do the skidmarks
//
/////////////////////////////////////////////////////////////////////
#define SKID_MAX_VELPAR		1000.0f


void PostProcessCarWheelColls(CAR *car)
{
	long	skidColour;
	REAL	velParLen, dRLen;
	REAL	velDotNorm;
	REAL	normDotNorm, dirDotDir, carDotUp;
	VEC	velPar, dR, lookVec, vel;
	COLLINFO_WHEEL	*wheelColl;
	WHEEL			*wheel;
	SKIDMARK_START	skidEnd;

	carDotUp = VecDotVec(&car->Body->Centre.WMatrix.mv[U], &DownVec);

	for (wheelColl = car->WheelCollHead; wheelColl != NULL; wheelColl = wheelColl->Next) {

		wheel = &car->Wheel[wheelColl->IWheel];

		// Count Wheel collisions with floor polys
		if (carDotUp > ZERO) {
			if ( MulScalar(carDotUp, VecDotVec(PlaneNormal(&wheelColl->Plane), &UpVec) ) > HALF) {
				car->NWheelFloorContacts++;
			}
		}

		// Make sure the collision is with the world
		if ((wheelColl->Material == NULL) || !(wheelColl->Material->Type & MATERIAL_SKID)) {
			// Reset the skid started flag
			wheel->Skid.Started = FALSE;
		}

		// Calculate velocity parallel to plane
		velDotNorm = VecDotVec(&wheelColl->Vel, PlaneNormal(&wheelColl->Plane));
		VecPlusScalarVec(&wheelColl->Vel, -velDotNorm, PlaneNormal(&wheelColl->Plane), &velPar);
		velParLen = Length(&velPar);
		if (velParLen > SMALL_REAL) {
			VecDivScalar(&velPar, velParLen);
		} else {
			SetVecZero(&velPar);
		}
		//if (velParLen > SKID_MAX_VELPAR) velParLen = SKID_MAX_VELPAR;

		// If necessary, add a skidmark to the skidmark list
		if (IsWheelSkidding(wheel) && wheel->Skid.Started) {
			// Calculate length of skidmark so far
			VecMinusVec(&wheelColl->WorldPos, &wheel->Skid.Pos, &dR);
			dRLen = Length(&dR);

			// Parameters for this end of skid
			VecCrossVec(PlaneNormal(&wheelColl->Plane), &wheel->Axes.mv[R], &lookVec);
			CopyVec(&wheelColl->WorldPos, &skidEnd.Pos);
			CopyVec(&velPar, &skidEnd.Dir);
			CopyVec(PlaneNormal(&wheelColl->Plane), &skidEnd.Normal);
			skidEnd.Width = MulScalar( wheel->SkidWidth / 2, abs(VecDotVec(&lookVec, &skidEnd.Dir)) );
			if (skidEnd.Width < MulScalar( Real(0.2), wheel->SkidWidth) ) skidEnd.Width = MulScalar( Real(0.2), wheel->SkidWidth );
			skidEnd.Material = wheelColl->Material;

			dirDotDir = VecDotVec(&skidEnd.Dir, &wheel->Skid.Dir);
			normDotNorm = VecDotVec(&skidEnd.Normal, &wheel->Skid.Normal);


			// Add a skidmark to the list or update the current one
			if (wheel->Skid.CurrentSkid == NULL) {
				// Make sure we are on the same surface
				if ((normDotNorm > SKID_MAX_DOT) && (skidEnd.Material == wheel->Skid.Material)) {
					// Get skid colour
					if (wheel->OilTime < OILY_WHEEL_TIME) {
						skidColour = 0xffffff;
					} else {
						//skidColour = (long)(256.0f * velParLen / SKID_MAX_VELPAR);
						//skidColour = skidColour << 26 | skidColour << 8 | skidColour;
						skidColour = wheelColl->Material->SkidColour;
					}
					wheel->Skid.CurrentSkid = AddSkid(&wheel->Skid, &skidEnd, skidColour);
				} else {
					wheel->Skid.Started = FALSE;
					continue;
				}
			} else {
				if (dirDotDir < ZERO) {
					NegateVec(&skidEnd.Dir);
				}
				// Make sure we are on the same surface
				if ((normDotNorm > SKID_MAX_DOT) && (skidEnd.Material == wheel->Skid.Material)) {
					MoveSkidEnd(wheel->Skid.CurrentSkid, &skidEnd);
				}
			}

#ifndef _PSX
			// Smokin'
			wheel->Skid.LastSmokeTime += TimeStep;
			if (car->Rendered && (wheel->Skid.LastSmokeTime > SKID_SMOKE_TIME)) {
				VecPlusVec(&SmokeVel, &velPar, &vel)
				CreateSpark(SPARK_SMOKE1, &wheelColl->WorldPos, &vel, ZERO, 0);
				wheel->Skid.LastSmokeTime = ZERO;
			}
#endif

			// Check to see if it is time to start a new skid
			if ((dRLen > SKID_MAX_LEN) ||
				((dRLen > SKID_MIN_LEN) && (dirDotDir < SKID_MAX_DOT)) ||
				(normDotNorm < SKID_MAX_DOT)) 
			{
				// Reset the skid started flag
				wheel->Skid.Started = FALSE;
			}
		}

		// Start a new skid if necessary
		if (!IsWheelSkidding(wheel) || !wheel->Skid.Started) {
			// Store skid parameters
			VecCrossVec(PlaneNormal(&wheelColl->Plane), &wheel->Axes.mv[R], &lookVec);
			CopyVec(&wheelColl->WorldPos, &wheel->Skid.Pos);
			CopyVec(&velPar, &wheel->Skid.Dir);
			CopyVec(PlaneNormal(&wheelColl->Plane), &wheel->Skid.Normal);
			wheel->Skid.Width = MulScalar( wheel->SkidWidth / 2, abs(VecDotVec(&lookVec, &wheel->Skid.Dir)) );
			if (wheel->Skid.Width < MulScalar( Real(0.2), wheel->SkidWidth) ) wheel->Skid.Width = MulScalar( Real(0.2), wheel->SkidWidth );
			wheel->Skid.Material = wheelColl->Material;
			wheel->Skid.LastSmokeTime = ZERO;

			// Set the skid started flag
			wheel->Skid.Started = TRUE;
			wheel->Skid.CurrentSkid = NULL;
		}
	}
}


/////////////////////////////////////////////////////////////////////
//
// SetAllCarCoMs: move all car centre of masses according to the
// value in the CarInfo structure.
//
/////////////////////////////////////////////////////////////////////

void SetAllCarCoMs()
{
	int iCar;

	for (iCar = 0; iCar < NCarTypes; iCar++) {

		MoveCarCoM(&CarInfo[iCar], &CarInfo[iCar].CoMOffset);

	}
}


/////////////////////////////////////////////////////////////////////
//
// MoveCarCoM: move the centre of mass of the specified car by the
// specified vector. Does not affect the inertia matrix of the car.
//
/////////////////////////////////////////////////////////////////////

void MoveCarCoM(CAR_INFO *carInfo, VEC *dR)
{
	int iWheel;

	// Move the body model
	VecPlusEqVec(&carInfo->Body.Offset, dR);

	// Move the Wheels, axles and suspension fixing points
	for (iWheel = 0; iWheel < CAR_NWHEELS; iWheel++) {
		VecPlusEqVec(&carInfo->Wheel[iWheel].Offset1, dR);
		VecPlusEqVec(&carInfo->Spring[iWheel].Offset, dR);
		VecPlusEqVec(&carInfo->Pin[iWheel].Offset, dR);
		VecPlusEqVec(&carInfo->Axle[iWheel].Offset, dR);
	}

	// Move the Aerial
	VecPlusEqVec(&carInfo->Aerial.Offset, dR);

	// Move car spinner
	VecPlusEqVec(&carInfo->Spinner.Offset, dR);
}

#ifndef _PSX
void CarWheelImpulse2(CAR *car, COLLINFO_WHEEL *collInfo, VEC *impulse)
{
	REAL	hardness;
	REAL	dVelNorm, impUp, scale, impDotNorm, knock;
	REAL	velTanLen, impTanLen, angVel;
	REAL	lookLen, velDotNorm, impDotLook, torque;
	REAL	fricMod, maxTorque, tReal;
	VEC	dVel, tmpVec;
	VEC	velTan, lookVec;
	VEC sparkVel;
//	COLLINFO_BODY *bodyColl;

	bool doSpark = FALSE;
	VEC  impNorm = {ZERO, ZERO, ZERO};
	VEC	impTan = {ZERO, ZERO, ZERO};
	WHEEL	*wheel = &car->Wheel[collInfo->IWheel];
	NEWBODY *body = car->Body;

	// Calculate forward vector
	VecCrossVec(PlaneNormal(&collInfo->Plane), &wheel->Axes.mv[R], &lookVec);
	lookLen = VecLen(&lookVec);

	// Scale the friction if the sides of the wheels are the contact point
	if (lookLen > Real(0.7)) {
		hardness = collInfo->Restitution;
		VecDivScalar(&lookVec, lookLen);
		fricMod = ONE;
	} else {
		hardness = Real(0.1);
		doSpark = TRUE;
		if (abs(collInfo->Plane.v[B]) > HALF) {
			fricMod = ONE;
		} else {
			fricMod = Real(0.1) + lookLen / 4;
		}
	}

	// Calculate the change in normal velocity required for the collision
	VecEqScalarVec(&dVel, -(ONE + hardness), &collInfo->Vel);
	dVelNorm = VecDotVec(&dVel, PlaneNormal(&collInfo->Plane));

	// Calculate normal (zero-friction) impulse required to get this change in velocity
	//OneBodyZeroFrictionImpulse(body, &collInfo->Pos, PlaneNormal(&collInfo->Plane), dVelNorm, &impNorm);
	TwoBodyZeroFrictionImpulse(body, collInfo->Body2, &collInfo->Pos, &collInfo->Pos2, PlaneNormal(&collInfo->Plane), dVelNorm, &impNorm);
	
	// Rescale due to spring-damper system
	if (CAR_WheelsHaveSuspension) {

		//if (abs(wheel->Pos) < wheel->MaxPos) {
			// Normal suspension forces
			// Dampers
			impUp = VecDotVec(&impNorm, &body->Centre.WMatrix.mv[U]);
			impUp = MulScalar(impUp, VecDotVec(&body->Centre.WMatrix.mv[U], PlaneNormal(&collInfo->Plane)));
			scale = MulScalar((car->Spring[collInfo->IWheel].Restitution), impUp);
			VecPlusEqScalarVec(&impNorm, scale, PlaneNormal(&collInfo->Plane));

			// Springs
			VecMinusVec(&wheel->CentrePos, &car->Body->Centre.Pos, &tmpVec);
			VecMinusEqVec(&tmpVec, &collInfo->Pos);
			if (Sign(wheel->Pos) == Sign(VecDotVec(&tmpVec, &body->Centre.WMatrix.mv[U]))) {
				scale = MulScalar(TimeStep, SpringDampedForce(&car->Spring[collInfo->IWheel], wheel->Pos, wheel->Vel));
				scale = MulScalar(scale, VecDotVec(&body->Centre.WMatrix.mv[U], PlaneNormal(&collInfo->Plane)));
				VecPlusEqScalarVec(&impNorm, -scale, PlaneNormal(&collInfo->Plane));
			}

			impDotNorm = VecDotVec(&impNorm, PlaneNormal(&collInfo->Plane));

		/*} else {
			// Body collision if suspension grounded out
			// Add collision to car body
			if ((bodyColl = NextBodyCollInfo()) != NULL) {
				impDotNorm = VecDotVec(&impNorm, PlaneNormal(&collInfo->Plane));
				VecPlusEqScalarVec(&impNorm, -impDotNorm, PlaneNormal(&collInfo->Plane));

				bodyColl->Body1 = body;
				bodyColl->Body2 = collInfo->Body2;
				CopyVec(&collInfo->Pos, &bodyColl->Pos1);
				//VecMinusVec(&wheel->CentrePos, &car->Body->Centre.Pos, &bodyColl->Pos1);
				VecCrossVec(&body->AngVel, &bodyColl->Pos1, &bodyColl->Vel);
				VecPlusEqVec(&bodyColl->Vel, &body->Centre.Vel);
				//CopyVec(&collInfo->Vel, &bodyColl->Vel);
				CopyPlane(&collInfo->Plane, &bodyColl->Plane);
				CopyVec(&collInfo->WorldPos, &bodyColl->WorldPos);
				//CopyVec(&wheel->WPos, &bodyColl->WorldPos);
				bodyColl->Depth = -COLL_EPSILON;//collInfo->Depth;
				bodyColl->Time = collInfo->Time;
				bodyColl->StaticFriction = ZERO;//MulScalar(fricMod, collInfo->StaticFriction);
				bodyColl->KineticFriction = ZERO;//MulScalar(fricMod, collInfo->KineticFriction);
				bodyColl->Restitution = ZERO;
				AddBodyColl(body, bodyColl);
			}
		}*/

	}

	// Flag a hard knock if necessary
	knock = MulScalar(abs(impDotNorm), car->Body->Centre.InvMass);
	if (knock > car->Body->BangMag) {
		car->Body->BangMag = knock;
		CopyPlane(&collInfo->Plane, &car->Body->BangPlane);
	}

	// Calculate sliding velocity
	velDotNorm = VecDotVec(&collInfo->Vel, PlaneNormal(&collInfo->Plane));
	VecPlusScalarVec(&collInfo->Vel, -velDotNorm, PlaneNormal(&collInfo->Plane), &velTan);
	velTanLen = Length(&velTan);

	// Turn sliding velocities into fricitonal impulse
	impTanLen = -MulScalar(collInfo->Grip, impDotNorm);
	VecEqScalarVec(&impTan, impTanLen, &velTan);
	impTanLen = MulScalar(impTanLen, velTanLen);

	// Remove friction along roll-direction of tyre if wheel not locked
	if (!IsWheelLocked(wheel)) {
		impDotLook = VecDotVec(&lookVec, &impTan);
		VecPlusEqScalarVec(&impTan, -impDotLook, &lookVec);
	}

	if (!IsWheelLocked(wheel)) {
		// Add wheel torque
		if (IsWheelPowered(wheel)) {
			torque = MulScalar(TimeStep, MulScalar(car->EngineVolt, wheel->EngineRatio));
			tReal = Sign(torque) * MulScalar(torque, DivScalar(MulScalar(wheel->AngVel, wheel->Radius), car->TopSpeed));
			if (abs(tReal) > abs(torque)) {
				torque = MulScalar(0.01f, torque);
			} else {
				torque -= tReal;
			}
		} else {
			torque = ZERO;
		}

		// Apply axle friction
		if ((GameSettings.AutoBrake && abs(car->EngineVolt) < Real(0.01)) || (Sign(car->EngineVolt) == -Sign(wheel->AngVel))) {
			tReal = MulScalar(wheel->AxleFriction, MulScalar(wheel->AngVel, wheel->Radius));
			tReal = MulScalar(tReal, MulScalar(FRICTION_TIME_SCALE, TimeStep));
			torque -= tReal;
		}

		// Check Engine Torque for wheelspin
		maxTorque = MulScalar(MulScalar(wheel->Radius, fricMod), MulScalar(collInfo->StaticFriction, impDotNorm));
		if (abs(torque) > abs(maxTorque)) {
			SetWheelSpinning(wheel);
		}

		VecPlusEqScalarVec(&impTan, DivScalar(torque, wheel->Radius), &lookVec);
	}

	// Scale sliding to friction cone
	impTanLen = VecLen(&impTan);
	maxTorque = 2 * MulScalar(MulScalar(TimeStep, car->Body->Centre.Mass), car->Body->Centre.Gravity);
	if (impTanLen > maxTorque) {
		VecMulScalar(&impTan, DivScalar(maxTorque, impTanLen));
		impTanLen = maxTorque;
		SetWheelSliding(wheel);
	}
	maxTorque = MulScalar(MulScalar(collInfo->StaticFriction, fricMod), impDotNorm);
	if (impTanLen > maxTorque) {
		maxTorque = MulScalar(MulScalar(collInfo->KineticFriction, fricMod), impDotNorm);
		VecMulScalar(&impTan, DivScalar(maxTorque, impTanLen));
		impTanLen = maxTorque;
		SetWheelSliding(wheel);
	}

	// Calculate wheel angular velocity
	if (IsWheelLocked(wheel)) {
		wheel->AngVel = ZERO;
	} else {
#ifndef _PSX
		if (IsWheelSpinning(wheel)) {
			//wheel->AngImpulse += MulScalar(dt, MulScalar(car->EngineVolt, wheel->EngineRatio));
			//wheel->AngImpulse += torque;
		} else {
			angVel = DivScalar(VecDotVec(&collInfo->Vel, &lookVec), wheel->Radius);
			wheel->AngVel += MulScalar((angVel - wheel->AngVel),  DivScalar(MulScalar(FRICTION_TIME_SCALE, TimeStep), 4));
		}
#else
		angVel = DivScalar(VecDotVec(&collInfo->Vel, &lookVec), wheel->Radius);
		wheel->AngVel = angVel;
#endif
	}

#ifndef _PSX
	// Generate spark if side of wheel scraping
	if (car->Rendered && doSpark &&  (collInfo->Material != NULL) && (velTanLen > MIN_SPARK_VEL))
	{
		body->ScrapeMaterial = collInfo->Material - COL_MaterialInfo;
		body->LastScrapeTime = ZERO;
		if ((frand(2.0f) < SparkProbability(velTanLen)) && MaterialAllowsSparks(collInfo->Material))
		{
			VecEqScalarVec(&sparkVel, HALF, &velTan);
			CreateSpark(SPARK_SPARK, &collInfo->WorldPos, &sparkVel, velTanLen / 3, 0);
		}
	}

	// Spray dust if the material is dusty
	if (car->Rendered && collInfo->Material != NULL && MaterialDusty(collInfo->Material) && impTanLen > 5 * body->Centre.InvMass * MIN_DUST_IMPULSE &&(frand(1.0f) < COL_DustInfo[collInfo->Material->DustType].SparkProbability, 0)) 
	{
		enum SparkTypeEnum sparkType;
		sparkType = (enum SparkTypeEnum)COL_DustInfo[collInfo->Material->DustType].SparkType;
		scale = body->Centre.InvMass * DUST_SCALE;
		VecEqScalarVec(&sparkVel, -scale, &impTan);
		VecPlusEqScalarVec(&sparkVel, HALF * scale * impTanLen, &UpVec);
		VecPlusEqScalarVec(&sparkVel, HALF * body->Centre.InvMass * torque, &lookVec);
		CreateSpark(sparkType, &collInfo->WorldPos, &sparkVel, COL_DustInfo[collInfo->Material->DustType].SparkVar * scale * impTanLen, 0);
	}
#endif

	// Sum the impulses
	VecPlusVec(&impNorm, &impTan, impulse);

	// Zero the small components
	if (abs(impulse->v[X]) < SMALL_IMPULSE_COMPONENT) impulse->v[X] = ZERO;
	if (abs(impulse->v[Y]) < SMALL_IMPULSE_COMPONENT) impulse->v[Y] = ZERO;
	if (abs(impulse->v[Z]) < SMALL_IMPULSE_COMPONENT) impulse->v[Z] = ZERO;


}

#else // PSX
#if FALSE
void CarWheelImpulse2(CAR *car, COLLINFO_WHEEL *collInfo, VEC *impulse)
{

	REAL	dVelNorm, impUp, scale, impDotNorm;
	REAL	velTanLen, impTanLen, angVel;
	REAL	lookLen, velDotNorm, velDotLook, impDotLook, torque;
	REAL	fricMod, maxTorque, tReal;
	VEC	dVel, tmpVec;
	VEC	velTan, lookVec;
	VEC sparkVel;
	COLLINFO_BODY *bodyColl;

	bool doSpark = FALSE;
	VEC  impNorm = {ZERO, ZERO, ZERO};
	VEC	impTan = {ZERO, ZERO, ZERO};
	WHEEL	*wheel = &car->Wheel[collInfo->IWheel];
	NEWBODY *body = car->Body;

	// Calculate forward vector
	VecCrossVecUnit(PlaneNormal(&collInfo->Plane), &wheel->Axes.mv[R], &lookVec);
	lookLen = VecLenUnit(&lookVec);

	// Scale the friction if the sides of the wheels are the contact point
	if (lookLen > Real(0.7)) {
		VecDivScalar(&lookVec, lookLen);
		fricMod = ONE;
	} else {
		doSpark = TRUE;
		if (abs(collInfo->Plane.v[B]) > HALF) {
			fricMod = ONE;
		} else {
			fricMod = Real(0.1) + lookLen / 4;
		}
	}

	// Calculate the change in normal velocity required for the collision
	VecEqScalarVec(&dVel, -(ONE + collInfo->Restitution), &collInfo->Vel);
	dVelNorm = VecDotVec(&dVel, PlaneNormal(&collInfo->Plane));

	// Calculate normal (zero-friction) impulse required to get this change in velocity
	TwoBodyZeroFrictionImpulse(body, collInfo->Body2, &collInfo->Pos, &collInfo->Pos2, PlaneNormal(&collInfo->Plane), dVelNorm, &impNorm);

	// Rescale due to spring-damper system
	if (CAR_WheelsHaveSuspension) {
		// Dampers
		impUp = VecDotVec(&impNorm, &body->Centre.WMatrix.mv[U]);
		impUp = MulScalar(impUp, VecDotVec(&body->Centre.WMatrix.mv[U], PlaneNormal(&collInfo->Plane)));
		scale = MulScalar((car->Spring[collInfo->IWheel].Restitution), impUp);
		VecPlusEqScalarVec(&impNorm, scale, PlaneNormal(&collInfo->Plane));

		// Springs
		VecMinusVec(&wheel->CentrePos, &car->Body->Centre.Pos, &tmpVec);
		VecMinusEqVec(&tmpVec, &collInfo->Pos);
		if (Sign(wheel->Pos) == Sign(VecDotVec(&tmpVec, &body->Centre.WMatrix.mv[U]))) {
			scale = MulScalar(TimeStep, SpringDampedForce(&car->Spring[collInfo->IWheel], wheel->Pos, wheel->Vel));
			scale = MulScalar(scale, VecDotVec(&body->Centre.WMatrix.mv[U], PlaneNormal(&collInfo->Plane)));
			VecPlusEqScalarVec(&impNorm, -scale, PlaneNormal(&collInfo->Plane));
		}

		// Body collision if suspension grounded out
/*		if (abs(wheel->Pos) >= wheel->MaxPos && COL_NBodyColls >= MAX_COLLS_BODY) {
			// Add collision to car body
			bodyColl = &COL_BodyColl[COL_NBodyColls++];
			bodyColl->Ignore = FALSE;
			bodyColl->Body1 = body;
			bodyColl->Body2 = NULL;
			CopyVec(&collInfo->Pos, &bodyColl->Pos1);
			CopyVec(&collInfo->Vel, &bodyColl->Vel);
			CopyPlane(&collInfo->Plane, &bodyColl->Plane);
			CopyVec(&collInfo->WorldPos, &bodyColl->WorldPos);
			bodyColl->Depth = collInfo->Depth;
			bodyColl->Time = collInfo->Time;
			bodyColl->StaticFriction = MulScalar(fricMod, collInfo->StaticFriction);
			bodyColl->KineticFriction = MulScalar(fricMod, collInfo->KineticFriction);
			bodyColl->Restitution = collInfo->Restitution;
		}
*/	}

	// Calculate sliding velocity
	impDotNorm = VecDotVec(&impNorm, PlaneNormal(&collInfo->Plane));
	velDotNorm = VecDotVec(&collInfo->Vel, PlaneNormal(&collInfo->Plane));
	VecPlusScalarVec(&collInfo->Vel, -velDotNorm, PlaneNormal(&collInfo->Plane), &velTan);
	velTanLen = VecLen(&velTan);
	velDotLook = VecDotVec(&collInfo->Vel, &lookVec);

	// Turn sliding velocities into fricitonal impulse
	impTanLen = -MulScalar(collInfo->Grip, impDotNorm);
	tReal = MulScalar(impTanLen, velTanLen);
	if (abs(tReal) < 15000) {
		VecEqScalarVec(&impTan, impTanLen, &velTan);
		impTanLen = tReal;
	} else {
		VecEqScalarVec(&impTan, -DivScalar(15000, velTanLen), &velTan);
		impTanLen = 15000;
	}
	printf("before: %8d (%8d) ", VecDotVec(&impTan, &lookVec), VecLen(&impTan));

	// Remove friction along roll-direction of tyre if wheel not locked
	if (!IsWheelLocked(wheel)) {
		RemoveComponent(&impTan, &lookVec); 
	}
	printf("after: %8d (%8d)\n", VecDotVec(&impTan, &lookVec), VecLen(&impTan));

	if (!IsWheelLocked(wheel)) {
		if (IsWheelPowered(wheel)) {
			// Add engine torque
			torque = MulScalar(TimeStep, MulScalar(car->EngineVolt, wheel->EngineRatio));
			scale = MulScalar(wheel->AngVel, wheel->Radius);
			torque -= MulScalar(torque, DivScalar(scale, car->TopSpeed));
		} else {
			torque = ZERO;
		}

		// Apply axle friction
		if ((GameSettings.AutoBrake && abs(car->EngineVolt) < Real(0.01)) || (Sign(car->EngineVolt) != Sign(wheel->AngVel))) {
			tReal = MulScalar(wheel->AxleFriction, wheel->AngVel);
			tReal = MulScalar(tReal, MulScalar(FRICTION_TIME_SCALE, TimeStep));
			torque -= tReal;
		}

		// Check Engine Torque for wheelspin
		maxTorque = MulScalar(fricMod, MulScalar(collInfo->StaticFriction, impDotNorm));
		if (abs(torque) > abs(maxTorque)) {
			SetWheelSpinning(wheel);
		}

		VecPlusEqScalarVec(&impTan, torque, &lookVec);
	}

	// Scale sliding to friction cone
	impTanLen = VecLen(&impTan);
	maxTorque = MulScalar(MulScalar(TimeStep, car->Body->Centre.Mass), car->Body->Centre.Gravity) << 1;
	if (impTanLen > maxTorque) {
		VecMulScalar(&impTan, DivScalar(maxTorque, impTanLen));
		impTanLen = maxTorque;
		SetWheelSliding(wheel);
	}
	maxTorque = MulScalar(MulScalar(collInfo->StaticFriction, fricMod), impDotNorm);
	if (impTanLen > maxTorque) {
		maxTorque = MulScalar(MulScalar(collInfo->KineticFriction, fricMod), impDotNorm);
		VecMulScalar(&impTan, DivScalar(maxTorque, impTanLen));
		impTanLen = maxTorque;
		SetWheelSliding(wheel);
	}

	// Calculate wheel angular velocity
	if (IsWheelLocked(wheel)) {
		wheel->AngVel = ZERO;
	} else {
#ifndef _PSX
		if (IsWheelSpinning(wheel)) {
			wheel->AngImpulse += torque;
		} else {
			angVel = DivScalar(VecDotVec(&collInfo->Vel, &lookVec), wheel->Radius);
			wheel->AngVel = angVel;
		}
#else
		angVel = DivScalar(velDotLook, wheel->Radius);
		wheel->AngVel = angVel;
#endif
	}

	//printf("ImpTan: (%d) %d  AngVel: %d\n", collInfo->IWheel, VecLen(&impTan), wheel->AngVel);
	//printf("ImpTan: (%d) %d %d %d\n", collInfo->IWheel, impTan.v[X], impTan.v[Y], impTan.v[Z]);

	//printf("forward: %8d\n", VecDotVec(&impTan, &lookVec));

	impTan.v[X] >> SMALL_SHIFT;
	impTan.v[Y] >> SMALL_SHIFT;
	impTan.v[Z] >> SMALL_SHIFT;

	// Sum the impulses
	VecPlusVec(&impNorm, &impTan, impulse);

	// Zero the small components
	if (abs(impulse->v[X]) < SMALL_IMPULSE_COMPONENT) impulse->v[X] = ZERO;
	if (abs(impulse->v[Y]) < SMALL_IMPULSE_COMPONENT) impulse->v[Y] = ZERO;
	if (abs(impulse->v[Z]) < SMALL_IMPULSE_COMPONENT) impulse->v[Z] = ZERO;

}
#endif
#endif

#if TRUE && defined(_PSX)
void CarWheelImpulse2(CAR *car, COLLINFO_WHEEL *collInfo, VEC *impulse)
{
	REAL	lookLen, velDotLook, upDotNorm, velDotNorm, knock;
	REAL	impDotNorm, impDotNormTrue, impDotLook, impSlide, scale, dVelNorm;
	REAL	impTanLen, max, velSlideLen;
	VEC		lookVec, relPos, velSideways;

	VEC		impTan = {0, 0, 0};
	WHEEL	*wheel = &car->Wheel[collInfo->IWheel];
	NEWBODY *body = car->Body;

	// Calculate forward vector
	VecCrossVecUnit(PlaneNormal(&collInfo->Plane), &wheel->Axes.mv[R], &lookVec);
	lookLen = VecLenUnit(&lookVec);
	if (lookLen > Real(0.7)) {
		VecDivScalar(&lookVec, lookLen);
		lookLen = ONE;
	}

	// scale friction if vertical(ish) wall
	if (abs(collInfo->Plane.v[B]) < Real(0.5)) {
		lookLen /= 5;
	}

	// Useful components of vectors
	velDotLook = VecDotVec(&collInfo->Vel, &lookVec);
	velDotNorm = VecDotVec(&collInfo->Vel, PlaneNormal(&collInfo->Plane));
	upDotNorm = VecDotVec(&body->Centre.WMatrix.mv[U], PlaneNormal(&collInfo->Plane));

	// Sliding velocity
	VecPlusScalarVec(&collInfo->Vel, -velDotNorm, PlaneNormal(&collInfo->Plane), &velSideways);
	VecPlusEqScalarVec(&velSideways, -velDotLook, &lookVec);
	RemoveComponent(&velSideways, &lookVec);		// repeat to remove initial error
	velSlideLen = VecLen(&velSideways);

	// Rigid-body single collision impulse
	//dVelNorm = -MulScalar(ONE + collInfo->Restitution, velDotNorm);
	dVelNorm = -velDotNorm;
	impDotNormTrue = OneBodyZeroFrictionImpulse(body, &collInfo->Pos, PlaneNormal(&collInfo->Plane), dVelNorm);
	impDotNormTrue = MulScalar(impDotNormTrue, ONE + collInfo->Restitution);
	impDotNormTrue -= MulScalar(impDotNormTrue, abs(upDotNorm));

	// Wheel-relative collision position
	VecMinusVec(&wheel->CentrePos, &car->Body->Centre.Pos, &relPos);
	VecMinusEqVec(&relPos, &collInfo->Pos);

	// Add a spring impulse
	if (Sign(wheel->Pos) == Sign(VecDotVec(&relPos, &body->Centre.WMatrix.mv[U]))) {
		scale = MulScalar(TimeStep, SpringDampedForce(&car->Spring[collInfo->IWheel], wheel->Pos, wheel->Vel));
		scale = MulScalar(scale, upDotNorm);
		impDotNormTrue -= scale;
	}

	// Flag a hard knock if necessary
	knock = MulScalar(impDotNormTrue, car->Body->Centre.InvMass);
	if (knock > car->Body->BangMag) {
		car->Body->BangMag = knock;
		CopyPlane(&collInfo->Plane, &car->Body->BangPlane);
	}

	// Make sure normal impulse not too large for friction calculations
	if (impDotNormTrue > 1000) {
		impDotNorm = 1000;
	} else {
		impDotNorm = impDotNormTrue;
	}

//	if (!IsWheelLocked )
	// Add the driving impulse
	if (IsWheelPowered(wheel)) {
		impDotLook = ONE - DivScalar(velDotLook, car->TopSpeed);
		impDotLook = MulScalar(impDotLook, MulScalar(car->EngineVolt, wheel->EngineRatio));
		impDotLook = MulScalar(impDotLook, TimeStep);
	} else {
		impDotLook = ZERO;
	}

	// Add the wheel axle friction
	if ((GameSettings.AutoBrake && abs(car->EngineVolt) < Real(0.01)) || (Sign(car->EngineVolt) != Sign(velDotLook))) {
		scale = MulScalar(wheel->AxleFriction, velDotLook);
		scale = MulScalar(scale, MulScalar(FRICTION_TIME_SCALE, TimeStep));
		impDotLook -= scale;
	}


	// Scale driving impulse to friction square
	max = MulScalar(collInfo->StaticFriction, impDotNorm) << SMALL_SHIFT;
	if (abs(impDotLook) > max) {
		if (impDotLook > 0) {
			impDotLook = MulScalar(collInfo->KineticFriction, impDotNorm) << SMALL_SHIFT;
		} else {
			impDotLook = -MulScalar(collInfo->KineticFriction, impDotNorm) << SMALL_SHIFT;
		}
		SetWheelSpinning(wheel);
	}

	// Add the sliding friction impulse
	impSlide = MulScalar(collInfo->Grip, impDotNorm);
	scale = MulScalar(impSlide, velSlideLen);
	if (scale > max) {
		impSlide = MulScalar(collInfo->KineticFriction, impDotNorm) << SMALL_SHIFT;
		impSlide = MulScalar(lookLen, DivScalar(impSlide, velSlideLen));
		SetWheelSliding(wheel);
	}


	// Generate tangential impulse
	ScalarVecPlusScalarVec(impDotLook, &lookVec, -impSlide, &velSideways, &impTan);

	impTan.v[X] >>= SMALL_SHIFT;
	impTan.v[Y] >>= SMALL_SHIFT;
	impTan.v[Z] >>= SMALL_SHIFT;

	// Sum the impulses
	VecPlusScalarVec(&impTan, impDotNormTrue, PlaneNormal(&collInfo->Plane), impulse);

	// Set the wheel angular velocity
	wheel->AngVel = DivScalar(&velDotLook, wheel->Radius);

	// Zero the small components
	if (abs(impulse->v[X]) < SMALL_IMPULSE_COMPONENT) impulse->v[X] = ZERO;
	if (abs(impulse->v[Y]) < SMALL_IMPULSE_COMPONENT) impulse->v[Y] = ZERO;
	if (abs(impulse->v[Z]) < SMALL_IMPULSE_COMPONENT) impulse->v[Z] = ZERO;

}

#endif // PSX



/////////////////////////////////////////////////////////////////////
//
// SetCarAngResistance:
//
/////////////////////////////////////////////////////////////////////

void SetCarAngResistance(CAR *car)
{
	int iWhl;

	int nCont = 0;

	// Count wheels on the floor
	for (iWhl = 0; iWhl < CAR_NWHEELS; iWhl++) {
		if (IsWheelInContact(&car->Wheel[iWhl])) {
			nCont++;
		}
	}

	// slow car spinning if no wheels on the floor
	if (nCont == 0) {
		car->Body->AngResistance = MulScalar(car->Body->AngResMod, car->Body->DefaultAngRes);
	} else {
		car->Body->AngResistance = car->Body->DefaultAngRes;
	}

}


/////////////////////////////////////////////////////////////////////
//
// CAR_AllCarColls: find all collisions between all cars and all 
// other objects
//
/////////////////////////////////////////////////////////////////////

/*void CAR_AllCarColls()
{
	PLAYER *player1, *player2;

	COL_NBodyColls = 0;
	COL_NBodyDone = 0;
	COL_NWheelColls = 0;
	COL_NWheelDone = 0;
	COL_NCollsTested = 0;

#if SHOW_PHYSICS_INFO
	DEBUG_NCols = 0;
	DEBUG_N2Cols = 0;
#endif

	for (player1 = PLR_PlayerHead; player1 != NULL; player1 = player1->next) {

		// Make sure player is collidable
		if ((player1->type == PLAYER_NONE) || (player1->type == PLAYER_GHOST)) continue;

		// Find all car-world collisions
		CarWorldColls(&player1->car);

		// Find all car-car collsions
		for (player2 = player1->next; player2 != NULL; player2 = player2->next) {

			// Make sure player is collidable
			if ((player2->type == PLAYER_NONE) || (player2->type == PLAYER_GHOST)) continue;

			CarCarColls(&player1->car, &player2->car);

		}

	}

}*/


/////////////////////////////////////////////////////////////////////
//
// CarWorldColls: detect all collisions between the car and the world
// mesh
//
/////////////////////////////////////////////////////////////////////

void DetectCarWorldColls(CAR *car)
{
	long	iPoly, iWheel;
	long	nCollPolys;
	COLLGRID *collGrid;

#ifndef _PSX
	NEWCOLLPOLY **collPolyPtr;
#endif
	NEWCOLLPOLY *collPoly;

	// Calculate the grid position and which polys to check against
	collGrid = PosToCollGrid(&car->Body->Centre.Pos);
	if (collGrid == NULL) return;

#ifndef _PSX
	collPolyPtr = collGrid->CollPolyPtr;
#endif
	nCollPolys = collGrid->NCollPolys;

	// Reset all collision related flags for the car
	for (iWheel = 0; iWheel < CAR_NWHEELS; iWheel++) {
		// Make sure skidmarks are stopped if the wheel is in the air
		if (!IsWheelInContact(&car->Wheel[iWheel])) {
			car->Wheel[iWheel].Skid.Started = FALSE;
		}

		// Clear the contact / sliding / oil flags
		SetWheelNotInContact(&car->Wheel[iWheel]);
		SetWheelNotSliding(&car->Wheel[iWheel]);
		if (IsWheelinOil(&car->Wheel[iWheel])) {
			SetWheelNotInOil(&car->Wheel[iWheel]);
			car->Wheel[iWheel].OilTime = ZERO;
		} else {
			car->Wheel[iWheel].OilTime += TimeStep;
		}
	}

	// Detect collisions
	for (iPoly = 0; iPoly < nCollPolys; iPoly++) {

#ifndef _PSX
		collPoly = collPolyPtr[iPoly];
#else 
		collPoly = &COL_WorldCollPoly[collGrid->CollPolyIndices[iPoly]];
#endif

		if (PolyCameraOnly(collPoly)) continue;

		// Full car-poly bounding box test
		if (!BBTestYXZ(&collPoly->BBox, &car->BBox)) continue;

		COL_NCollsTested++;

		// WHEEL - WORLD
		for (iWheel = 0; iWheel < CAR_NWHEELS; iWheel++) {
			if (IsWheelPresent(&car->Wheel[iWheel])) {
				// Detect collision and add them to the list
				DetectCarWheelColls2(car, iWheel, collPoly);
			}
		}

		// BODY - WORLD
		DetectBodyPolyColls(car->Body, collPoly);

	}

#if USE_DEBUG_ROUTINES
	DEBUG_CollGrid = collGrid - COL_CollGrid;
#endif
}


/////////////////////////////////////////////////////////////////////
//
// DetectWheelBodyColls:
//
/////////////////////////////////////////////////////////////////////

int DetectWheelBodyColls(CAR *car, NEWBODY *body) 
{
	int		iWheel;
	WHEEL	*wheel;
	int nColls = 0;

	// Check for each wheel
	for (iWheel = 0; iWheel < CAR_NWHEELS; iWheel++) {
		wheel = &car->Wheel[iWheel];
		if (!IsWheelPresent(wheel)) continue;

		if (IsBodyConvex(body)) {
			nColls += DetectWheelConvexColls(car, iWheel, body);
		} 
		else if (IsBodySphere(body)) {
			nColls += DetectWheelSphereColls(car, iWheel, body);
		}
		else if (IsBodyPoly(body)) {
			nColls += DetectWheelPolyColls(car, iWheel, body);
		}
	}

	return nColls;
}

int DetectWheelPolyColls(CAR *car, int iWheel, NEWBODY *body)
{
	int iPoly;
	VEC	tmpVec;
	REAL	time;
	NEWCOLLPOLY *collPoly;
	int nColls = 0;
	WHEEL	*wheel = &car->Wheel[iWheel];
	COLLINFO_WHEEL	*wheelColl;

	Assert(IsBodyPoly(body));

	// Bounding box test
	if (!BBTestYXZ(&wheel->BBox, &body->CollSkin.BBox)) return 0;

	for (iPoly = 0; iPoly < body->CollSkin.NCollPolys; iPoly++) {
		collPoly = &body->CollSkin.CollPoly[iPoly];

		if ((wheelColl = NextWheelCollInfo()) == NULL) return nColls;
	 
		// Quick swepth-volume axis-aligned bounding-box test
		if (!BBTestYXZ(&wheel->BBox, &collPoly->BBox)) continue;

		// Do a sphere-to-poly collision test
		if (SphereCollPoly(&wheel->OldCentrePos, &wheel->CentrePos,
			wheel->Radius, 
			collPoly, 
			&wheelColl->Plane,
			&wheelColl->Pos,
			&wheelColl->WorldPos,
			&wheelColl->Depth,
			&time))
		{

			// Calculate the collision point on the plane (for skidmarks and smoke generator)
			VecPlusEqScalarVec(&wheelColl->WorldPos, SKID_RAISE, PlaneNormal(&wheelColl->Plane));

			// Calculate the car-relative collision point for response
			VecPlusEqVec(&wheelColl->Pos, &wheel->CentrePos);
			VecMinusEqVec(&wheelColl->Pos, &car->Body->Centre.Pos);

			// Calculate world velocity of the wheel collision point (not including wheel spin)
			VecCrossVec(&car->Body->AngVel, &wheelColl->Pos, &wheelColl->Vel);
			VecPlusEqVec(&wheelColl->Vel, &car->Body->Centre.Vel);
			VecMinusEqVec(&wheelColl->Vel, &body->Centre.Vel);

			// Make sure that the wheel is not already travelling away from the surface
			VecPlusScalarVec(&wheelColl->Vel, wheel->Vel, &car->Body->Centre.WMatrix.mv[U], &tmpVec);
			if (VecDotVec(&tmpVec, PlaneNormal(&wheelColl->Plane)) > ZERO) continue;

			// Add bumps from surface corrugation
			wheelColl->Material = &COL_MaterialInfo[collPoly->Material];
			//AdjustWheelColl(wheelColl, wheelColl->Material);

			// Set other necessary stuff
			wheelColl->Car = car;
			wheelColl->IWheel = iWheel;
			wheelColl->Grip = MulScalar(wheel->Grip, wheelColl->Material->Gripiness);
			wheelColl->StaticFriction = MulScalar(wheel->StaticFriction, wheelColl->Material->Roughness);
			wheelColl->KineticFriction = MulScalar(wheel->KineticFriction, wheelColl->Material->Roughness);
			wheelColl->Restitution = ZERO;
			wheelColl->Body2 = &BDY_MassiveBody;
			SetVecZero(&wheelColl->Pos2);


			// Set the wheel-in-contact flag
			SetWheelInContact(wheel);
			SetWheelNotSpinning(wheel);

			AddWheelColl(car, wheelColl);
			nColls++;

		}
	}
	return nColls;
}


int DetectWheelSphereColls(CAR *car, int iWheel, NEWBODY *body)
{
	VEC	dR;
	REAL	dRLen;
	COLLINFO_WHEEL	*wheelColl;
	COLLINFO_BODY	*bodyColl;
	WHEEL	*wheel = &car->Wheel[iWheel];
	SPHERE  *sphere = &body->CollSkin.WorldSphere[0];

	// Bounding box test
	if (!BBTestYXZ(&wheel->BBox, &body->CollSkin.BBox)) return 0;

	// Set up the collision info
	if ((wheelColl = NextWheelCollInfo()) == NULL) return 0;
	if ((bodyColl = NextBodyCollInfo()) == NULL) return 0;

	//Get relative position
	VecMinusVec(&wheel->CentrePos, &sphere->Pos, &dR);
	dRLen = VecLen(&dR);

	// Check for collision
	if (dRLen > wheel->Radius + sphere->Radius) return 0;

	// Collision Occurred

	bodyColl->Body1 = body;
	bodyColl->Body2 = car->Body;
	wheelColl->Car = car;
	wheelColl->IWheel = iWheel;
	wheelColl->Body2 = body;
	SetVecZero(&wheelColl->WorldPos);	// DODGY....
	SetVecZero(&bodyColl->WorldPos);	// DODGY....

	bodyColl->Depth = HALF * (dRLen - wheel->Radius - sphere->Radius);
	wheelColl->Depth = bodyColl->Depth;

	// Collision plane
	if (dRLen > SMALL_REAL) {
		CopyVec(&dR, PlaneNormal(&wheelColl->Plane));
		VecDivScalar(PlaneNormal(&wheelColl->Plane), dRLen);
		wheelColl->Time = ONE - (wheelColl->Depth / dRLen);
	} else {
		SetVec(PlaneNormal(&wheelColl->Plane), ONE, ZERO, ZERO);
		wheelColl->Time = ONE;
	}
	FlipPlane(&wheelColl->Plane, &bodyColl->Plane);
	bodyColl->Time = wheelColl->Time;

	// Calculate the collision points for response
	VecEqScalarVec(&wheelColl->Pos, wheel->Radius, PlaneNormal(&wheelColl->Plane));
	VecPlusEqVec(&wheelColl->Pos, &wheel->CentrePos);
	VecMinusEqVec(&wheelColl->Pos, &car->Body->Centre.Pos);
	//VecEqScalarVec(&bodyColl->Pos1, sphere->Radius, PlaneNormal(&bodyColl->Plane));
	VecPlusScalarVec(&wheel->CentrePos, HALF, &dR, &wheelColl->WorldPos);
	CopyVec(&wheelColl->Pos, &bodyColl->Pos2);
	CopyVec(&wheelColl->WorldPos, &bodyColl->WorldPos);
	VecPlusScalarVec(&sphere->Pos, -sphere->Radius, PlaneNormal(&wheelColl->Plane), &bodyColl->Pos1);
	VecMinusEqVec(&bodyColl->Pos1, &body->Centre.Pos);
	VecMinusVec(&bodyColl->WorldPos, &body->Centre.Pos, &bodyColl->Pos1);
	CopyVec(&bodyColl->Pos1, &wheelColl->Pos2);

	// Calculate velocity
	VecCrossVec(&car->Body->AngVel, &wheelColl->Pos, &wheelColl->Vel);
	VecPlusEqVec(&wheelColl->Vel, &car->Body->Centre.Vel);
	VecCrossVec(&body->AngVel, &bodyColl->Pos1, &bodyColl->Vel);
	VecPlusEqVec(&bodyColl->Vel, &body->Centre.Vel);
	VecMinusEqVec(&bodyColl->Vel, &wheelColl->Vel);
	CopyVec(&bodyColl->Vel, &wheelColl->Vel);
	NegateVec(&wheelColl->Vel);

	wheelColl->Grip = wheel->Grip * body->Centre.Grip;
	wheelColl->StaticFriction = wheel->StaticFriction * body->Centre.StaticFriction;
	wheelColl->KineticFriction = wheel->KineticFriction * body->Centre.KineticFriction;
	wheelColl->Restitution = car->Spring[iWheel].Restitution;
	wheelColl->Material = NULL;
	bodyColl->Grip = wheelColl->Grip;
	bodyColl->StaticFriction = wheelColl->StaticFriction;
	bodyColl->KineticFriction = wheelColl->KineticFriction;
	bodyColl->Restitution = wheelColl->Restitution;
	bodyColl->Material = NULL;

	AddWheelColl(car, wheelColl);
	AddBodyColl(body, bodyColl);

	return 1;
}

int DetectWheelConvexColls(CAR *car, int iWheel, NEWBODY *body)
{
	int iSkin;
	COLLINFO_WHEEL	*wheelColl;
	COLLINFO_BODY	*bodyColl;
	WHEEL *wheel = &car->Wheel[iWheel];
	int nColls = 0;

	Assert(body != NULL);
	Assert(car != NULL);
	Assert((iWheel >= 0) && (iWheel < CAR_NWHEELS));

	// Bounding box test
	if (!BBTestYXZ(&wheel->BBox, &body->CollSkin.BBox)) return 0;

	// Check against each convex hull
	for (iSkin = 0; iSkin < body->CollSkin.NConvex; iSkin++) {

		// Bounding box test
		if (!BBTestYXZ(&wheel->BBox, &body->CollSkin.WorldConvex[iSkin].BBox)) continue;

		// Set up the collision info
		if ((wheelColl = NextWheelCollInfo()) == NULL) return nColls;
		if ((bodyColl = NextBodyCollInfo()) == NULL) return nColls;

		// Check for collision
		if (!SphereConvex(&wheel->CentrePos, wheel->Radius, &body->CollSkin.WorldConvex[iSkin], &wheelColl->Pos, &wheelColl->Plane, &wheelColl->Depth)) {
			continue;
		}

		// Collision Occurred
		
		bodyColl->Body1 = body;
		bodyColl->Body2 = car->Body;
		wheelColl->Car = car;
		wheelColl->IWheel = iWheel;
		wheelColl->Body2 = body;
		SetVecZero(&wheelColl->WorldPos);	// DODGY....
		SetVecZero(&bodyColl->WorldPos);	// DODGY....

		// Calculate the relative collision points for response
		//VecPlusScalarVec(&wheel->CentrePos, -(wheel->radius + wheelColl->Depth), PlaneNormal(&wheelColl->Plane), &wheelColl->Pos);
		VecMinusVec(&wheelColl->Pos, &body->Centre.Pos, &bodyColl->Pos1);
		VecMinusEqVec(&wheelColl->Pos, &car->Body->Centre.Pos);
		CopyVec(&wheelColl->Pos, &bodyColl->Pos2);
		CopyVec(&bodyColl->Pos1, &wheelColl->Pos2);

		// Calculate velocity
		VecCrossVec(&car->Body->AngVel, &wheelColl->Pos, &wheelColl->Vel);
		VecPlusEqVec(&wheelColl->Vel, &car->Body->Centre.Vel);
		VecCrossVec(&body->AngVel, &bodyColl->Pos1, &bodyColl->Vel);
		VecPlusEqVec(&bodyColl->Vel, &body->Centre.Vel);
		VecMinusEqVec(&bodyColl->Vel, &wheelColl->Vel);
		CopyVec(&bodyColl->Vel, &wheelColl->Vel);
		NegateVec(&wheelColl->Vel);

		FlipPlane(&wheelColl->Plane, &bodyColl->Plane);
		bodyColl->Depth = wheelColl->Depth;
		bodyColl->Time = ZERO;

		// Make sure that the wheel is not already travelling away from the surface
		//VecPlusScalarVec(&wheelColl->Vel, wheel->Vel, &car->Body->Centre.WMatrix.mv[U], &tmpVec);
		//if (VecDotVec(&tmpVec, PlaneNormal(&wheelColl->Plane)) > ZERO) return;

		wheelColl->Grip = wheel->Grip * body->Centre.Grip;
		wheelColl->StaticFriction = wheel->StaticFriction * body->Centre.StaticFriction;
		wheelColl->KineticFriction = wheel->KineticFriction * body->Centre.KineticFriction;
		wheelColl->Restitution = car->Spring[iWheel].Restitution;
		wheelColl->Material = NULL;
		bodyColl->Grip = wheelColl->Grip;
		bodyColl->StaticFriction = wheelColl->StaticFriction;
		bodyColl->KineticFriction = wheelColl->KineticFriction;
		bodyColl->Restitution = wheelColl->Restitution;
		bodyColl->Material = NULL;

		AddBodyColl(body, bodyColl);
		AddWheelColl(car, wheelColl);
		nColls++;

	}

	return nColls;
}


int DetectWheelWheelColls(CAR *car1, CAR *car2)
{
	int iWheel1, iWheel2;
	VEC	dR;
	REAL	dRLen;
	COLLINFO_WHEEL	*wheelColl;
	COLLINFO_WHEEL	*wheelColl2;
	WHEEL	*wheel1;
	WHEEL	*wheel2;
	int nColls = 0;


	for (iWheel1 = 0; iWheel1 < CAR_NWHEELS; iWheel1++) {
		wheel1 = &car1->Wheel[iWheel1];
		if (!IsWheelPresent(wheel1)) continue;

		for (iWheel2 = 0; iWheel2 < CAR_NWHEELS; iWheel2++) {
			wheel2 = &car2->Wheel[iWheel2];
			if (!IsWheelPresent(wheel2)) continue;

			// Bounding box test
			if (!BBTestYXZ(&wheel1->BBox, &wheel2->BBox)) continue;

			//Get relative position
			VecMinusVec(&wheel1->CentrePos, &wheel2->CentrePos, &dR);
			dRLen = VecLen(&dR);

			// Check for collision
			if (dRLen > wheel1->Radius + wheel2->Radius) continue;

			// Set up the collision info
			if ((wheelColl = NextWheelCollInfo()) == NULL) return nColls;
			AddWheelColl(car1, wheelColl);
			if ((wheelColl2 = NextWheelCollInfo()) == NULL) {
				RemoveWheelColl(car1, wheelColl);
				return nColls;
			}
			AddWheelColl(car2, wheelColl2);


			// Collision Occurred
			wheelColl2->Car = car2;
			wheelColl2->IWheel = iWheel2;
			wheelColl2->Body2 = car2->Body;
			wheelColl->Car = car1;
			wheelColl->IWheel = iWheel1;
			wheelColl->Body2 = car1->Body;
			SetVecZero(&wheelColl->WorldPos);	// DODGY....
			SetVecZero(&wheelColl2->WorldPos);	// DODGY....

			wheelColl2->Depth = dRLen - wheel1->Radius - wheel2->Radius;
			wheelColl->Depth = wheelColl2->Depth;

			// Collision plane
			if (dRLen > SMALL_REAL) {
				CopyVec(&dR, PlaneNormal(&wheelColl->Plane));
				VecDivScalar(PlaneNormal(&wheelColl->Plane), dRLen);
				wheelColl->Time = ONE - (wheelColl->Depth / dRLen);
			} else {
				SetVec(PlaneNormal(&wheelColl->Plane), ONE, ZERO, ZERO);
				wheelColl->Time = ONE;
			}
			FlipPlane(&wheelColl->Plane, &wheelColl2->Plane);
			wheelColl2->Time = wheelColl->Time;

			// Calculate the collision points for response
			VecEqScalarVec(&wheelColl->Pos, wheel1->Radius, PlaneNormal(&wheelColl->Plane));
			VecPlusEqVec(&wheelColl->Pos, &wheel1->CentrePos);
			VecMinusEqVec(&wheelColl->Pos, &car1->Body->Centre.Pos);

			VecEqScalarVec(&wheelColl2->Pos, wheel2->Radius, PlaneNormal(&wheelColl2->Plane));
			VecPlusEqVec(&wheelColl2->Pos, &wheel2->CentrePos);
			VecMinusEqVec(&wheelColl2->Pos, &car2->Body->Centre.Pos);

			VecPlusScalarVec(&wheel1->CentrePos, HALF, &dR, &wheelColl->WorldPos);
			CopyVec(&wheelColl->WorldPos, &wheelColl2->WorldPos);

			// Calculate velocity
			VecCrossVec(&car1->Body->AngVel, &wheelColl->Pos, &wheelColl->Vel);
			VecPlusEqVec(&wheelColl->Vel, &car1->Body->Centre.Vel);

			VecCrossVec(&car2->Body->AngVel, &wheelColl2->Pos, &wheelColl2->Vel);
			VecPlusEqVec(&wheelColl2->Vel, &car2->Body->Centre.Vel);

			VecMinusEqVec(&wheelColl2->Vel, &wheelColl->Vel);
			CopyVec(&wheelColl2->Vel, &wheelColl->Vel);
			NegateVec(&wheelColl->Vel);

			wheelColl->Grip = wheel1->Grip * wheel2->Grip;
			wheelColl->StaticFriction = wheel1->StaticFriction * wheel2->StaticFriction;
			wheelColl->KineticFriction = wheel1->KineticFriction * wheel2->KineticFriction;
			wheelColl->Restitution = car1->Spring[iWheel1].Restitution;
			wheelColl->Material = NULL;
			wheelColl2->Grip = wheelColl->Grip;
			wheelColl2->StaticFriction = wheelColl->StaticFriction;
			wheelColl2->KineticFriction = wheelColl->KineticFriction;
			wheelColl2->Restitution = car2->Spring[iWheel2].Restitution;
			wheelColl2->Material = NULL;

			nColls++;
		}
	}

	return nColls;
}

/////////////////////////////////////////////////////////////////////
//
// CarCarColls:
//
/////////////////////////////////////////////////////////////////////

int DetectCarCarColls(CAR *car1, CAR *car2)
{
	int nColls = 0;

	// Quick bounding-box test
	if (!BBTestXZY(&car1->BBox, &car2->BBox)) return nColls;

	// Check all car parts against other car's parts
	nColls += DetectBodyBodyColls(car1->Body, car2->Body);
	nColls += DetectWheelBodyColls(car1, car2->Body);
	nColls += DetectWheelBodyColls(car2, car1->Body);
	nColls += DetectWheelWheelColls(car1, car2);

	return nColls;
}


/////////////////////////////////////////////////////////////////////
//
// CarBodyColls:
//
/////////////////////////////////////////////////////////////////////

int DetectCarBodyColls(CAR *car, NEWBODY *body)
{
	int nColls = 0;

	// Bounding box test
	if (!BBTestXZY(&car->BBox, &body->CollSkin.BBox)) return nColls;

	// Check car parts against body
	nColls += DetectBodyBodyColls(car->Body, body);
	nColls += DetectWheelBodyColls(car, body);

	return nColls;

}


/////////////////////////////////////////////////////////////////////
//
// UpdateRemotePlayer: if a new packet has been received from a 
// remote player, put the new values into its structure.
//
/////////////////////////////////////////////////////////////////////
#ifdef _PC
VEC DBG_PosDiff;
VEC DBG_VelDiff;

void UpdateRemotePlayer(PLAYER *player)
{
	REMOTE_DATA *rem;
	CAR		*car;
	
	car = &player->car;
	rem = &car->RemoteData[car->NewDat];

	// Make sure there is new data
	if (!rem->NewData) return;

	// Update the players structure
	player->controls.dx = rem->dx;
	player->controls.dy = rem->dy;

	// DEBUGGING
	VecMinusVec(&rem->Pos, &car->Body->Centre.Pos, &DBG_PosDiff);
	VecMinusVec(&rem->Vel, &car->Body->Centre.Vel, &DBG_VelDiff);


	CopyVec(&rem->Pos, &car->Body->Centre.Pos);
	CopyVec(&rem->Vel, &car->Body->Centre.Vel);
	CopyVec(&rem->AngVel, &car->Body->AngVel);
	CopyQuat(&rem->Quat, &car->Body->Centre.Quat);

	// Flag data as used
	rem->NewData = FALSE;

}
#endif


////////////////////////////////
// load one car model + tpage //
////////////////////////////////
#ifdef _PC

void LoadOneCarModelSet(struct PlayerStruct *player, long car)
{
	CAR_INFO *ci;
	CAR_MODEL *cm = &player->carmodels;
	COLLSKIN_INFO *collinfo = &cm->CollSkin;
	CONVEX *pSkin;
	FILE *fp;
	long i, iSkin, iFace, iPt;
	char tPage;

// get car info

	car %= NCarTypes;
	ci = &CarInfo[car];

// set parts flags

	cm->BodyPartsFlag = 0;
	cm->WheelPartsFlag[FL] = cm->WheelPartsFlag[FR] = cm->WheelPartsFlag[BL] = cm->WheelPartsFlag[BR] = 0;

// Load models

	for (i = 0 ; i < MAX_CAR_MODEL_TYPES; i++)
	{
		if (strlen(ci->ModelFile[i]))
		{
			if (i == 17 || i == 18) {
				tPage = TPAGE_FX1;
			} else {
				tPage = TPAGE_CAR_START + (char)player->Slot;
			}
			LoadModel(ci->ModelFile[i], cm->Model[i], tPage, MAX_CAR_LOD, LOADMODEL_FORCE_TPAGE, LevelInf[GameSettings.Level].ModelRGBper);
		}
		else
		{
			cm->Model[i]->AllocPtr = NULL;
		}
	}

// Load TPage

	if (strlen(ci->TPageFile))
	{
		LoadTextureClever(ci->TPageFile, TPAGE_CAR_START + (char)player->Slot, 256, 256, 0, CarTextureSet, TRUE);
	}


// Set Env map RGB

	cm->EnvRGB = ci->EnvRGB;

// Load Collision Skin

	if (ci->CollFile && ci->CollFile[0])
	{
		if ((fp = fopen(ci->CollFile, "rb")) != NULL) 
		{

			// Load the convex hulls
			if ((collinfo->Convex = LoadConvex(fp, &collinfo->NConvex, 0)) != NULL)
			{
				collinfo->CollType = BODY_COLL_CONVEX;

				// Move the collision skins to centre on CoM
				for (iSkin = 0; iSkin < collinfo->NConvex; iSkin++) {
					pSkin = &collinfo->Convex[iSkin];

					// offset
					//VecPlusEqVec(&pSkin->Offset, &ci->CoMOffset);

					// collision planes
					for (iFace = 0; iFace < pSkin->NFaces; iFace++) {
						MovePlane(&pSkin->Faces[iFace], &ci->CoMOffset);
					}

					// vertices
					for (iPt = 0; iPt < pSkin->NPts; iPt++) {
						VecPlusEqVec(&pSkin->Pts[iPt], &ci->CoMOffset);
					}
				}

			}

			// Load the spheres
			if ((collinfo->Sphere = LoadSpheres(fp, &collinfo->NSpheres)) != NULL)
			{
				// Move the spheres to centre on CoM
				for (iSkin = 0; iSkin < collinfo->NSpheres; iSkin++) {
					// Position
					VecPlusEqVec(&collinfo->Sphere[iSkin].Pos, &ci->CoMOffset);
				}
			}

			MakeTightLocalBBox(collinfo);

			fclose(fp);
		}
	}
	else
	{
		collinfo->NConvex = 0;
		collinfo->Convex = NULL;
		collinfo->NSpheres = 0;
		collinfo->Sphere = NULL;
		SetBBox(&collinfo->BBox, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO);
	}

// Setup body

	cm->Body = cm->Model[ci->Body.ModelNum];
	CopyVec(&ci->Body.Offset, &cm->OffBody);

// Setup wheels

	for (i = 0; i < 4; i++)
	{
		cm->Wheel[i] = cm->Model[ci->Wheel[i].ModelNum];
		CopyVec(&ci->Wheel[i].Offset1, &cm->OffWheel[i]);
		CopyVec(&ci->Wheel[i].Offset2, &cm->OffWheelColl[i])
		cm->WheelRad[i] = ci->Wheel[i].Radius;
	}

// Set up spinner

	if (ci->Spinner.ModelNum != CAR_MODEL_NONE)
	{
		cm->BodyPartsFlag |= CAR_MODEL_SPINNER;
		cm->Spinner = cm->Model[ci->Spinner.ModelNum];
		CopyVec(&ci->Spinner.Offset, &cm->OffSpinner);
	}

// Setup aerial models

	if (ci->Aerial.SecModelNum != CAR_MODEL_NONE && ci->Aerial.TopModelNum != CAR_MODEL_NONE)
	{
		cm->BodyPartsFlag |= CAR_MODEL_AERIAL;
		cm->Aerial[0] = cm->Model[ci->Aerial.SecModelNum];
		cm->Aerial[1] = cm->Model[ci->Aerial.TopModelNum];
		CopyVec(&ci->Aerial.Offset, &cm->OffAerial); 
		CopyVec(&ci->Aerial.Direction, &cm->DirAerial); 
		cm->AerialLen = ci->Aerial.SecLen;
	}

// setup springs / axles / pins

	for (i = 0; i < 4; i++)
	{

// Setup springs

		if (ci->Spring[i].ModelNum != CAR_MODEL_NONE)
		{
			cm->Spring[i] = cm->Model[ci->Spring[i].ModelNum];
			CopyVec(&ci->Spring[i].Offset, &cm->OffSpring[i]);
			cm->WheelPartsFlag[i] |= CAR_MODEL_SPRING;
			cm->SpringLen[i] = ci->Spring[i].Length;
		}
	
// Setup axles

		if (ci->Axle[i].ModelNum != CAR_MODEL_NONE)
		{
			cm->Axle[i] = cm->Model[ci->Axle[i].ModelNum];
			CopyVec(&ci->Axle[i].Offset, &cm->OffAxle[i]);
			cm->AxleLen[i] = ci->Axle[i].Length;
			cm->WheelPartsFlag[i] |= CAR_MODEL_AXLE;
		}

// Setup pins

		if (ci->Pin[i].ModelNum != CAR_MODEL_NONE)
		{
			cm->Pin[i] = cm->Model[ci->Pin[i].ModelNum];
			CopyVec(&ci->Pin[i].Offset, &cm->OffPin[i]);
			cm->PinLen[i] = ci->Pin[i].Length;
			cm->WheelPartsFlag[i] |= CAR_MODEL_PIN;
		}
	}
}
#endif

//--------------------------------------------------------------------------------------------------------------------------
// N64 Version
#ifdef _N64

void LoadOneCarModelSet(struct PlayerStruct *player, long car)
{
	CAR_INFO		*ci;
	CAR_MODEL		*cm = &player->carmodels;
	COLLSKIN_INFO 	*collinfo = &cm->CollSkin;
	CONVEX 			*pSkin;
	long 			ii, jj, iSkin, iFace, iPt;
	FIL				*fp;
	long	Flag = 0;

// get car info

	car %= NCarTypes;
	ci = &CarInfo[car];

// set parts flags

	cm->BodyPartsFlag = 0;
	cm->WheelPartsFlag[FL] = cm->WheelPartsFlag[FR] = cm->WheelPartsFlag[BL] = cm->WheelPartsFlag[BR] = 0;

// Load models

	for (ii = 0 ; ii < MAX_CAR_MODEL_TYPES; ii++)
	{
		if (ci->EnvRGB) { Flag = MODEL_ENV; }
		if (ci->ModelFile[ii])
		{
			MOD_LoadModel(ci->ModelFile[ii], ci->TextureFile[ii], &cm->Model[ii][0], ci->EnvRGB, LevelInf[GameSettings.Level].ModelRGBper, Flag);
		}
		else
		{
			cm->Model[ii][0].hdr = NULL;
		}
	}

// Set Env map RGB
	cm->EnvRGB = ci->EnvRGB;

// Load Collision Skin

	if (ci->CollFile)
	{
		if ((fp = FFS_Open(ci->CollFile)) != NULL) 
		{

			// Load the convex hulls
			if ((collinfo->Convex = LoadConvex(fp, &collinfo->NConvex, 0)) != NULL)
			{
				collinfo->CollType = BODY_COLL_CONVEX;

				// Move the collision skins to centre on CoM
				for (iSkin = 0; iSkin < collinfo->NConvex; iSkin++)
				{
					pSkin = &collinfo->Convex[iSkin];

					// offset
					//VecPlusEqVec(&pSkin->Offset, &ci->CoMOffset);

					// collision planes
					for (iFace = 0; iFace < pSkin->NFaces; iFace++)
					{
						MovePlane(&pSkin->Faces[iFace], &ci->CoMOffset);
					}

					// vertices
					for (iPt = 0; iPt < pSkin->NPts; iPt++)
					{
						VecPlusEqVec(&pSkin->Pts[iPt], &ci->CoMOffset);
					}
				}
			}

			// Load the spheres
			if ((collinfo->Sphere = LoadSpheres(fp, &collinfo->NSpheres)) != NULL)
			{
				// Move the spheres to centre on CoM
				for (iSkin = 0; iSkin < collinfo->NSpheres; iSkin++)
				{
					// Position
					VecPlusEqVec(&collinfo->Sphere[iSkin].Pos, &ci->CoMOffset);
				}
			}

			MakeTightLocalBBox(collinfo);

			FFS_Close(fp);
		}
	}
	else
	{
		collinfo->NConvex = 0;
		collinfo->Convex = NULL;
		collinfo->NSpheres = 0;
		collinfo->Sphere = NULL;
		SetBBox(&collinfo->BBox, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO);
	}

// Setup body

	cm->Body = cm->Model[ci->Body.ModelNum];
	CopyVec(&ci->Body.Offset, &cm->OffBody);

// Setup wheels

	for (ii = 0; ii < 4; ii++)
	{
		cm->Wheel[ii] = cm->Model[ci->Wheel[ii].ModelNum];
		CopyVec(&ci->Wheel[ii].Offset1, &cm->OffWheel[ii]);
		CopyVec(&ci->Wheel[ii].Offset2, &cm->OffWheelColl[ii])
		cm->WheelRad[ii] = ci->Wheel[ii].Radius;
	}

// Set up spinner

	if (ci->Spinner.ModelNum != CAR_MODEL_NONE)
	{
		cm->BodyPartsFlag |= CAR_MODEL_SPINNER;
		cm->Spinner = cm->Model[ci->Spinner.ModelNum];
		CopyVec(&ci->Spinner.Offset, &cm->OffSpinner);
	}

// Setup aerial models

	if (ci->Aerial.SecModelNum != CAR_MODEL_NONE && ci->Aerial.TopModelNum != CAR_MODEL_NONE)
	{
		cm->BodyPartsFlag |= CAR_MODEL_AERIAL;
		cm->Aerial[0] = cm->Model[ci->Aerial.SecModelNum];
		cm->Aerial[1] = cm->Model[ci->Aerial.TopModelNum];
		CopyVec(&ci->Aerial.Offset, &cm->OffAerial); 
		CopyVec(&ci->Aerial.Direction, &cm->DirAerial); 
		cm->AerialLen = ci->Aerial.SecLen;
	}

// setup springs / axles / pins

	for (ii = 0; ii < 4; ii++)
	{

// Setup springs

		if (ci->Spring[ii].ModelNum != CAR_MODEL_NONE)
		{
			cm->Spring[ii] = cm->Model[ci->Spring[ii].ModelNum];
			CopyVec(&ci->Spring[ii].Offset, &cm->OffSpring[ii]);
			cm->WheelPartsFlag[ii] |= CAR_MODEL_SPRING;
			cm->SpringLen[ii] = ci->Spring[ii].Length;
		}
	
// Setup axles

		if (ci->Axle[ii].ModelNum != CAR_MODEL_NONE)
		{
			cm->Axle[ii] = cm->Model[ci->Axle[ii].ModelNum];
			CopyVec(&ci->Axle[ii].Offset, &cm->OffAxle[ii]);
			cm->AxleLen[ii] = ci->Axle[ii].Length;
			cm->WheelPartsFlag[ii] |= CAR_MODEL_AXLE;
		}

// Setup pins

		if (ci->Pin[ii].ModelNum != CAR_MODEL_NONE)
		{
			cm->Pin[ii] = cm->Model[ci->Pin[ii].ModelNum];
			CopyVec(&ci->Pin[ii].Offset, &cm->OffPin[ii]);
			cm->PinLen[ii] = ci->Pin[ii].Length;
			cm->WheelPartsFlag[ii] |= CAR_MODEL_PIN;
		}
	}
}

#endif

//--------------------------------------------------------------------------------------------------------------------------

////////////////////////////////
// free one car model + tpage //
////////////////////////////////
#ifdef _PC

void FreeOneCarModelSet(struct PlayerStruct *player)
{
	long i;

// free models

	for (i = 0 ; i < MAX_CAR_MODEL_TYPES ; i++)
	{
		if (player->carmodels.Model[i]->AllocPtr)
		{
			FreeModel(player->carmodels.Model[i], MAX_CAR_LOD);
		}
	}

// free texture

	FreeOneTexture(TPAGE_CAR_START + (char)player->Slot);

// free coll skin

	DestroyConvex(player->car.Models->CollSkin.Convex, player->car.Models->CollSkin.NConvex);
	player->car.Models->CollSkin.Convex = NULL;
	player->car.Body->CollSkin.Convex = NULL;
	player->car.Models->CollSkin.NConvex = 0;

	DestroySpheres(player->car.Models->CollSkin.Sphere);
	player->car.Models->CollSkin.Sphere = NULL;
	player->car.Body->CollSkin.Sphere = NULL;
	player->car.Models->CollSkin.NSpheres = 0;
}
#endif

//--------------------------------------------------------------------------------------------------------------------------

//N64 version
#ifdef _N64

void FreeOneCarModelSet(struct PlayerStruct *player)
{
	long i;

// free models

	for (i = 0 ; i < MAX_CAR_MODEL_TYPES ; i++)
	{
		if (player->carmodels.Model[i])
		{
			MOD_FreeModel(player->carmodels.Model[i]);
		}
	}

// free coll skin

	DestroyConvex(player->car.Models->CollSkin.Convex, player->car.Models->CollSkin.NConvex);
	player->car.Models->CollSkin.Convex = NULL;
	player->car.Body->CollSkin.Convex = NULL;
	player->car.Models->CollSkin.NConvex = 0;

	DestroySpheres(player->car.Models->CollSkin.Sphere);
	player->car.Models->CollSkin.Sphere = NULL;
	player->car.Body->CollSkin.Sphere = NULL;
	player->car.Models->CollSkin.NSpheres = 0;

}

#endif

//--------------------------------------------------------------------------------------------------------------------------

/////////////////////////////////////////////////////////////////////
//
// CarAccTimings: count the best acceleration times of the cars
// from 0-20 and 0-30mph
//
/////////////////////////////////////////////////////////////////////

#ifndef _PSX

void CarAccTimings(CAR *car)
{
	REAL carVel;

	carVel = OGU2MPH_SPEED * VecLen(&car->Body->Centre.Vel);

	// See if timing should start
	if (carVel < Real(0.01)) {
		car->Timing0to15 = TRUE;
		car->Timing0to25 = TRUE;
		car->Current0to15 = ZERO;
		car->Current0to25 = ZERO;
		return;
	}

	// Update timers
	if (car->Timing0to15) {
		car->Current0to15 += TimeStep;
	}
	if (car->Timing0to25) {
		car->Current0to25 += TimeStep;
	}

	// See if the speeds have been reached
	if (car->Timing0to15 && carVel > 15) {
		if ((car->Current0to15 < car->Best0to15) || (car->Best0to15 < ZERO)) {
			car->Best0to15 = car->Current0to15;
		}
		car->Timing0to15 = FALSE;
	}
	if (car->Timing0to25 && carVel > 25) {
		if ((car->Current0to25 < car->Best0to25) || (car->Best0to25 < ZERO)) {
			car->Best0to25 = car->Current0to25;
		}
		car->Timing0to25 = FALSE;
	}

}


#endif

/////////////////////////////////////////////////////////////////////
//
// CarDownForce:
//
/////////////////////////////////////////////////////////////////////

#define WFL_CONTACT 1
#define WFR_CONTACT 2
#define WBL_CONTACT 4
#define WBR_CONTACT 8
#if USE_DEBUG_ROUTINES
VEC DEBUG_DownForce;
#endif

void CarDownForce(CAR *car)
{
#ifndef _PSX
	int ii;
	REAL vel, mod;
	VEC downForce = {ZERO, ZERO, ZERO};
	long contact = 0;

	// Set up the wheel contact flags
	for (ii = 0; ii < CAR_NWHEELS; ii++) {
		if (IsWheelPresent(&car->Wheel[ii]) && IsWheelInContact(&car->Wheel[ii])) {
			contact |= 1 << ii;
		}
	}

	// if all wheel in contact, no need to continue
	if (contact == (WFL_CONTACT | WFR_CONTACT | WBL_CONTACT | WBR_CONTACT)) return;

	mod = ONE + HALF - abs(VecDotVec(&car->Body->Centre.WMatrix.mv[U], &UpVec));
	if (mod > ONE) mod = ONE;

	// See if car on two wheel on left side
	if (contact == (WFL_CONTACT | WBL_CONTACT)) {
		vel = MulScalar(TimeStep, VecDotVec(&car->Body->Centre.Vel, &car->Body->Centre.WMatrix.mv[L]));
		vel = MulScalar(vel, mod);
		VecPlusEqScalarVec(&downForce, MulScalar(car->DownForceMod, vel), &car->Body->Centre.WMatrix.mv[U]);
	}

	// See if car on two wheel on right side
	if (contact == (WFR_CONTACT | WBR_CONTACT)) {
		vel = MulScalar(TimeStep, VecDotVec(&car->Body->Centre.Vel, &car->Body->Centre.WMatrix.mv[L]));
		vel = MulScalar(vel, mod);
		VecPlusEqScalarVec(&downForce, MulScalar(car->DownForceMod, vel), &car->Body->Centre.WMatrix.mv[U]);
	}

	VecPlusEqVec(&car->Body->Centre.Impulse, &downForce);

#if USE_DEBUG_ROUTINES
	CopyVec(&downForce, &DEBUG_DownForce);
#endif

#endif
}


/////////////////////////////////////////////////////////////////////
//
// AddWheelColl:
//
/////////////////////////////////////////////////////////////////////

COLLINFO_WHEEL *AddWheelColl(CAR *car, COLLINFO_WHEEL *newHead)
{
	COLLINFO_WHEEL *oldHead = car->WheelCollHead;

	car->WheelCollHead = newHead;
	newHead->Next = oldHead;
	newHead->Prev = NULL;

	if (oldHead != NULL) {
		oldHead->Prev = newHead;
	}

	car->NWheelColls++;
	COL_NWheelColls++;

	return newHead;
}

/////////////////////////////////////////////////////////////////////
//
// RemoveWheelColl:
//
/////////////////////////////////////////////////////////////////////

void RemoveWheelColl(CAR *car, COLLINFO_WHEEL *collInfo)
{
	Assert(collInfo != NULL);

	if (collInfo->Next != NULL) {
		(collInfo->Next)->Prev = collInfo->Prev;
	}

	if (collInfo->Prev != NULL) {
		(collInfo->Prev)->Next = collInfo->Next;
	} else {
		car->WheelCollHead = collInfo->Next;
	}

	car->NWheelColls--;

	Assert((car->NWheelColls == 0)? (car->WheelCollHead == NULL): (car->WheelCollHead != NULL));
}




#if MSCOMPILER_FUDGE_OPTIMISATIONS
#pragma optimize("", on)
#endif
