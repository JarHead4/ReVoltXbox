/*********************************************************************************************
 *
 * ai.cpp
 *
 * Re-Volt (Generic) Copyright (c) Probe Entertainment 1998
 * 
 * Contents:
 *			Utility AI functions, used by car and general AIs
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
#include "ainode.h"
#include "move.h"
#include "player.h"
#ifndef _PSX
#include "timing.h"
#endif
#include "ai.h"
#include "ai_car.h"
#include "spark.h"
#include "newcoll.h"
#ifndef _PSX
#include "obj_init.h"
#endif
#ifdef _PC
#include "registry.h"
#include "ghost.h"
#include "input.h"
#endif
#include "weapon.h"
#ifndef _PSX
#include "visibox.h"
#endif


//
// Global variables
//

long	AI_Testing = FALSE;

//
// Static variables
//

static VEC PlanePropOff = {0, -120, 110};
static VEC CopterBlade1Off = {0, -340, 66};
static VEC CopterBlade2Off = {28, -252, -200};
static VEC TrainSteamOffset = {0, -550, -196};
static VEC TrainSteamDir = {0, -350, 0};
static VEC BoatSteamOffset = {0, -350, 0};
static VEC BoatSteamVel = {0, -500, -100};
static VEC TrainWheelOffsets[] = {{98, -136, 330}, {-98, -136, 330}, {98, -92, -204}, {-98, -92, -204}};

//
// Global function prototypes
//

void AI_ProcessAllAIs(void);
void AI_CarAiHandler(OBJECT *obj);

static void UpdateCarFinishDist(PLAYER *player);
static void UpdatePlayerPickup(PLAYER *player);
static void UpdateCarSfx(CAR *car);
static void UpdateCarMisc(CAR *car);
static void TurnCopter(OBJECT *obj);
static void FlyCopter(OBJECT *obj);
static void CopterWait(OBJECT *obj);
void NewCopterDest(OBJECT *obj);
void AI_LaserHandler(OBJECT *obj);

void SparkGenHandler(OBJECT *obj);

// scrape sfx list

#ifdef _PC
long SfxScrapeList[] = {
	SFX_SCRAPE1,		// default
	SFX_SCRAPE1,		// marble
	SFX_SCRAPE1,		// stone
	SFX_SCRAPE1,		// wood
	SFX_SCRAPE1,		// sand
	SFX_SCRAPE1,		// plastic
	SFX_SCRAPE1,		// carpet tile
	SFX_SCRAPE1,		// carpet shag
	SFX_SCRAPE1,		// boundary
	SFX_SCRAPE1,		// glass
	SFX_SCRAPE1,		// ice 1
	SFX_SCRAPE1,		// metal
	SFX_SCRAPE1,		// grass
	SFX_SCRAPE1,		// bumpy metal
	SFX_SCRAPE1,		// pebbles
	SFX_SCRAPE1,		// gravel
	SFX_SCRAPE1,		// conveyor 1
	SFX_SCRAPE1,		// conyeyor 2
	SFX_SCRAPE1,		// dirt 1
	SFX_SCRAPE1,		// dirt 2
	SFX_SCRAPE1,		// dirt 3
	SFX_SCRAPE1,		// ice 2
	SFX_SCRAPE1,		// ice 3
};
#endif

//--------------------------------------------------------------------------------------------------------------------------

//
// AI_ProcessAllAIs
//
// Processes all the AI functions for active objects
//

void AI_ProcessAllAIs(void)
{
	OBJECT	*obj, *next;

	for (obj = OBJ_ObjectHead ; obj ; )
	{
		next = obj->next;	// get next now in case object frees itself!
		if (obj->aihandler)
		{
			obj->aihandler(obj);
		}
		obj = next;
	}	
}


//--------------------------------------------------------------------------------------------------------------------------

///////////////////////////
// perform misc car jobs //
///////////////////////////

void AI_CarAiHandler(OBJECT *obj)
{
	obj->player->LastValidRailCamNode = obj->player->ValidRailCamNode;
	obj->player->ValidRailCamNode = -1;
#ifndef _PSX
	CAI_CarHelper(obj->player);
	UpdateCarFinishDist(obj->player);
#endif
#ifdef _PC
	CarAccTimings(&obj->player->car);
	UpdateCarSfx(&obj->player->car);
	UpdatePlayerPickup(obj->player);
#endif
	UpdateCarMisc(&obj->player->car);

#ifndef _PSX
	if (AI_Testing)
	{
		obj->player->CarAI.CurNode = AIN_GetForwardNode(obj->player, 200, (REAL *)&obj->player->CarAI.NodeDist);
	}
#endif
}

void AI_RemoteAiHandler(OBJECT *obj)
{
	obj->player->LastValidRailCamNode = obj->player->ValidRailCamNode;
	obj->player->ValidRailCamNode = -1;
#ifdef _PC
//	UpdateRemotePlayer(obj->player);
	UpdateCarFinishDist(obj->player);
	UpdateCarSfx(&obj->player->car);
	UpdateCarMisc(&obj->player->car);
	UpdatePlayerPickup(obj->player);
#endif
}
	
void AI_GhostCarAiHandler(OBJECT *obj)
{
	obj->player->LastValidRailCamNode = obj->player->ValidRailCamNode;
	obj->player->ValidRailCamNode = -1;
#ifdef _PC
	MOV_MoveGhost(obj);
	UpdateCarFinishDist(obj->player);
	UpdateCarSfx(&obj->player->car);
	UpdateCarMisc(&obj->player->car);
	UpdatePlayerPickup(obj->player);
#endif
}

////////////////////////////////
// update car finish distance //
////////////////////////////////

static void UpdateCarFinishDist(PLAYER *player)
{
#ifndef _PSX

	AINODE *cnode, *nnode;
	REAL cdist, ndist, dist, add, max;
	VEC vec, norm;
	long i;

// not if no nodes

	if (!AiNodeNum)
		return;

// get dist to current node

	cnode = &AiNode[player->CarAI.FinishDistNode];
	SubVector(&cnode->Centre, &player->car.Body->Centre.Pos, &vec);
	cdist = vec.v[X] * vec.v[X] + vec.v[Y] * vec.v[Y] + vec.v[Z] * vec.v[Z];

	ndist = FLT_MAX;

// look for nearest forward or backward node

	for (i = 0 ; i < MAX_AINODE_LINKS ; i++)
	{
		if (cnode->Prev[i])
		{
			SubVector(&cnode->Prev[i]->Centre, &player->car.Body->Centre.Pos, &vec);
			dist = vec.v[X] * vec.v[X] + vec.v[Y] * vec.v[Y] + vec.v[Z] * vec.v[Z];

			if (dist < ndist)
			{
				ndist = dist;
				nnode = cnode->Prev[i];
			}
		}

		if (cnode->Next[i])
		{
			SubVector(&cnode->Next[i]->Centre, &player->car.Body->Centre.Pos, &vec);
			dist = vec.v[X] * vec.v[X] + vec.v[Y] * vec.v[Y] + vec.v[Z] * vec.v[Z];

			if (dist < ndist)
			{
				ndist = dist;
				nnode = cnode->Next[i];
			}
		}
	}

// update current node?

	if (ndist < cdist)
	{
		player->CarAI.FinishDistNode = (long)(nnode - AiNode);
	}

// update finish dist

	SetVector(&norm, -cnode->RVec.v[Z], cnode->RVec.v[Y], cnode->RVec.v[X]);
	SubVector(&player->car.Body->Centre.Pos, &cnode->Centre, &vec);
	player->CarAI.FinishDist = cnode->FinishDist + DotProduct(&norm, &vec);

// update 'wrong way' flag

	player->CarAI.WrongWay = (DotProduct(&norm, &player->car.Body->Centre.WMatrix.mv[L]) > 0.7f);

// update panel finish dist

	dist = player->CarAI.FinishDist / AiNodeTotalDist;
	add = dist - player->CarAI.FinishDistPanel;

	if (add > 0.5f) add -= 1.0f;
	else if (add < -0.5f) add += 1.0f;

	max = 0.1f * TimeStep;
	if (add > max) add = max;
	else if (add < -max) add = -max;

	player->CarAI.FinishDistPanel += add;
	if (player->CarAI.FinishDistPanel >= 1.0f) player->CarAI.FinishDistPanel -= 1.0f;
	else if (player->CarAI.FinishDistPanel < 0.0f) player->CarAI.FinishDistPanel += 1.0f;

#endif
}

///////////////////////////////////
// update players cycling pickup //
///////////////////////////////////
#ifndef _PSX
static void UpdatePlayerPickup(PLAYER *player)
{
	REAL f;

// tempy pickup select

	static long pickup = PICKUP_NONE;
#ifdef _PC
	if (Keys[DIK_RSHIFT] && !LastKeys[DIK_RSHIFT] && player == PLR_LocalPlayer)
#endif
#ifdef _N64
	if ((player == PLR_LocalPlayer) && (player->controls.digital & CTRL_SELWEAPON))
#endif
	{
		pickup = (pickup + 1) % PICKUP_NUM;
		player->PickupType = pickup;
		player->PickupNum = 1000000;
	}

// only if no cycling pickup

	if (player->PickupCycleSpeed) {

// dec cycle speed

		player->PickupCycleSpeed -= TimeStep;

// inc cycle type

		player->PickupCycleType += player->PickupCycleSpeed * TimeStep * 2.0f;
		if (player->PickupCycleType > PICKUP_NUM)
			player->PickupCycleType -= PICKUP_NUM;

// give to player?

		f = 1.0f - (player->PickupCycleType - (float)(long)player->PickupCycleType);
		if (player->PickupCycleSpeed < f)
		{

// yep

			player->PickupCycleSpeed = 0;
			player->PickupType = (long)(player->PickupCycleType + 0.5f) % PICKUP_NUM;
			player->PickupTarget = NULL;

			if (player->PickupType == PICKUP_FIREWORKPACK)
				player->PickupNum = 3;
			else
				player->PickupNum = 1;
		}

	}

// Select weapon target if the weapon type requires it
	if ((player->PickupType == PICKUP_FIREWORK) || (player->PickupType == PICKUP_FIREWORKPACK)) {
		player->PickupTarget = WeaponTarget(player->ownobj);
	}
}
#endif

/////////////////////////////////////////////////////////////////////
// UpdateCarSfx:
/////////////////////////////////////////////////////////////////////
#ifdef _PC
static void UpdateCarSfx(CAR *car)
{
	long i, revs, vel, maxvol, time, sfx, screech;

// get car revs, velocity

	FTOL(abs(car->Revs), revs);
	FTOL(Length(&car->Body->Centre.Vel), vel);

// update engine

	if (car->SfxEngine)
	{
		car->SfxEngine->Freq = 10000 + revs * 80;

		car->SfxEngine->Vol = revs / 2;
		if (car->SfxEngine->Vol > SFX_MAX_VOL) car->SfxEngine->Vol = SFX_MAX_VOL;

		CopyVec(&car->Body->Centre.Pos, &car->SfxEngine->Pos);
	}

// update scrape

	if (car->Body->ScrapeMaterial == MATERIAL_NONE)
	{
		if (car->SfxScrape)
		{
			FreeSfx3D(car->SfxScrape);
			car->SfxScrape = NULL;
		}
	}
	else
	{
		sfx = SfxScrapeList[car->Body->ScrapeMaterial];

		if (!car->SfxScrape)
		{
			car->SfxScrape = CreateSfx3D(sfx, 0, 0, TRUE, &car->Body->Centre.Pos);
			car->ScrapeMaterial = car->Body->ScrapeMaterial;
		}

		if (car->SfxScrape)
		{
			car->SfxScrape->Freq = 20000 + vel * 5;

			FTOL(TimeStep * 600.0f, time);
			if (!time) time = 1;
			maxvol = car->SfxScrape->Vol + time;
			if (maxvol > SFX_MAX_VOL) maxvol = SFX_MAX_VOL;

			car->SfxScrape->Vol = vel / 10;
			if (car->SfxScrape->Vol > maxvol) car->SfxScrape->Vol = maxvol;

			CopyVec(&car->Body->Centre.Pos, &car->SfxScrape->Pos);

			if (car->ScrapeMaterial != car->Body->ScrapeMaterial)
			{
				ChangeSfxSample3D(car->SfxScrape, sfx);
				car->ScrapeMaterial = car->Body->ScrapeMaterial;
			}
		}
	}

// update screech

	screech = FALSE;
	for (i = 0 ; i < CAR_NWHEELS ; i++)
	{
		if (IsWheelPresent(&car->Wheel[i]) && IsWheelSkidding(&car->Wheel[i]) && IsWheelInContact(&car->Wheel[i]))
		{
			screech = TRUE;
			break;
		}
	}

	if (!screech)
	{
		if (car->SfxScreech)
		{
			FreeSfx3D(car->SfxScreech);
			car->SfxScreech = NULL;
		}
	}
	else
	{
		if (!car->SfxScreech)
		{
			car->SfxScreech = CreateSfx3D(SFX_SCREECH, 0, 0, TRUE, &car->Body->Centre.Pos);
		}

		if (car->SfxScreech)
		{
			car->SfxScreech->Freq = 15000 + vel * 2;

			FTOL(TimeStep * 600.0f, time);
			if (!time) time = 1;
			maxvol = car->SfxScreech->Vol + time;
			if (maxvol > SFX_MAX_VOL) maxvol = SFX_MAX_VOL;

			car->SfxScreech->Vol = vel / 10;
			if (car->SfxScreech->Vol > maxvol) car->SfxScreech->Vol = maxvol;

			CopyVec(&car->Body->Centre.Pos, &car->SfxScreech->Pos);
		}
	}
}
#endif

/////////////////////////////////////////////////////////////////////
// UpdateCarMisc:
/////////////////////////////////////////////////////////////////////
static void UpdateCarMisc(CAR *car)
{
	VEC vec;
	MAT mat1, mat2;

// env matrix

#ifdef _PC
	if (RenderSettings.Env)
	{
		SubVector(&car->Body->Centre.Pos, &car->Body->Centre.OldPos, &vec);
		RotMatrixZYX(&mat1, vec.v[X] / 6144, 0, vec.v[Z] / 6144);
		MulMatrix(&car->EnvMatrix, &mat1, &mat2);
		CopyMatrix(&mat2, &car->EnvMatrix);
	}
#endif

// electropulsed power timer?

	if (car->PowerTimer)
	{	
		car->PowerTimer -= TimeStep;
		if (car->PowerTimer < 0.0f)
			car->PowerTimer = 0.0f;
	}

// zero AddLit

#ifdef _PC
	if (car->AddLit > 0)
	{
		car->AddLit -= (long)(TimeStep * 1000);
		if (car->AddLit < 0) car->AddLit = 0;
	}
	else if (car->AddLit < 0)
	{
		car->AddLit += (long)(TimeStep * 1000);
		if (car->AddLit > 0) car->AddLit = 0;
	}
#endif

// bomb return timer
	if (car->NoReturnTimer > ZERO) {
		car->NoReturnTimer -= TimeStep;
	}

// Bang storing
#ifdef _PC
	if (car->Body->BangMag > TO_VEL(Real(300))) {
		long vol, freq;

		//vol = SFX_MIN_VOL + (SFX_MAX_VOL - SFX_MIN_VOL) * obj->body.BangMag / 1000;
		//if (vol > SFX_MAX_VOL) vol = SFX_MAX_VOL;
		vol = SFX_MAX_VOL;

		freq = 22050;

		PlaySfx3D(SFX_CHROMEBALL_HIT, vol, freq, &car->Body->Centre.Pos);
	}
#endif
	car->Body->Banged = FALSE;
	car->Body->BangMag = ZERO;
}

////////////////////
// barrel handler //
////////////////////
#ifndef _PSX

void AI_BarrelHandler(OBJECT *obj)
{
	MAT mat, mat2;
	BARREL_OBJ *barrel = (BARREL_OBJ*)obj->Data;

// quit if not visible

	if (!obj->renderflag.visible)
		return;

// spin

	RotMatrixX(&mat, barrel->SpinSpeed * TimeFactor);
	MulMatrix(&obj->body.Centre.WMatrix, &mat, &mat2);
	NormalizeMatrix(&mat2);
	CopyMatrix(&mat2, &obj->body.Centre.WMatrix);
}
#endif

////////////////////
// planet handler //
////////////////////
#ifndef _PSX
void AI_PlanetHandler(OBJECT *obj)
{
	long i;
	MAT mat, mat2;
	VEC vec;
	OBJECT *findobj, *findsun;
	REAL len;
	PLANET_OBJ *planet = (PLANET_OBJ*)obj->Data, *findplanet;
	SUN_OBJ *sun = (SUN_OBJ*)obj->Data;

// get orbit object?

	if (!obj->objref)
	{
		for (findobj = OBJ_ObjectHead ; findobj ; findobj = findobj->next)
		{
			findplanet = (PLANET_OBJ*)findobj->Data;
			if (findobj->Type == OBJECT_TYPE_PLANET && findplanet->OwnPlanet == planet->OrbitPlanet)
			{
				obj->objref = findobj;

				for (findsun = OBJ_ObjectHead ; findsun ; findsun = findsun->next)
					if (findsun->Type == OBJECT_TYPE_PLANET && ((PLANET_OBJ*)findsun->Data)->OwnPlanet == PLANET_SUN)
						planet->VisiMask = ((SUN_OBJ*)(findsun->Data))->VisiMask;

				break;
			}
		}

		if (!obj->objref)
			return;

		if (planet->OwnPlanet != PLANET_SUN)
		{
// setup orbit info
			SubVector(&obj->body.Centre.Pos, &obj->objref->body.Centre.Pos, &vec);
			len = Length(&vec);
			SetVector(&planet->OrbitOffset, 0, 0, len);

			BuildLookMatrixForward(&obj->objref->body.Centre.Pos, &obj->body.Centre.Pos, &mat2);
//			RotMatrixZ(&mat, ((float)rand() / RAND_MAX - 0.5f) / 3.0f);
			RotMatrixZ(&mat, 0.1f);
			MulMatrix(&mat2, &mat, &planet->OrbitMatrix);
		}
	}

// rotate on orbit
	if (planet->OwnPlanet != planet->OrbitPlanet)
	{
		RotMatrixX(&mat, planet->OrbitSpeed * TimeFactor);
		MulMatrix(&planet->OrbitMatrix, &mat, &mat2);
		NormalizeMatrix(&mat2);
		CopyMatrix(&mat2, &planet->OrbitMatrix);
		RotTransVector(&planet->OrbitMatrix, &obj->objref->body.Centre.Pos, &planet->OrbitOffset, &obj->body.Centre.Pos);
	}

// quit if not visible

//	if (planet->OwnPlanet != PLANET_SUN && !obj->renderflag.visible)
	if (!obj->renderflag.visible)
		return;

// spin on local Y axis

	RotMatrixY(&mat, planet->SpinSpeed * TimeFactor);
	MulMatrix(&obj->body.Centre.WMatrix, &mat, &mat2);
	NormalizeMatrix(&mat2);
	CopyMatrix(&mat2, &obj->body.Centre.WMatrix);

// sun?

	if (planet->OwnPlanet != PLANET_SUN)
		return;

// maintain overlays

	for (i = 0 ; i < SUN_OVERLAY_NUM ; i++)
	{
//		sun->Overlay[i].RotVel += frand(0.0001f) - 0.00005f;
//		if (sun->Overlay[i].RotVel < -0.001f) sun->Overlay[i].RotVel = -0.001f;
//		else if (sun->Overlay[i].RotVel > 0.001f) sun->Overlay[i].RotVel = 0.001f;

		sun->Overlay[i].Rot += sun->Overlay[i].RotVel * TimeFactor;

		sun->Overlay[i].r += (rand() % 5) - 2;
		if (sun->Overlay[i].r > 128) sun->Overlay[i].r = 128;
		else if (sun->Overlay[i].r < 96) sun->Overlay[i].r = 96;

		sun->Overlay[i].g += (rand() % 5) - 2;
		if (sun->Overlay[i].g > 128) sun->Overlay[i].g = 128;
		else if (sun->Overlay[i].g < 96) sun->Overlay[i].g = 96;

		sun->Overlay[i].b += (rand() % 5) - 2;
		if (sun->Overlay[i].b > 96) sun->Overlay[i].b = 96;
		else if (sun->Overlay[i].b < 64) sun->Overlay[i].b = 64;

		sun->Overlay[i].rgb = (sun->Overlay[i].r << 16) | (sun->Overlay[i].g << 8) | sun->Overlay[i].b;
	}
}
#endif

///////////////////
// plane handler //
///////////////////
#ifndef _PSX
void AI_PlaneHandler(OBJECT *obj)
{
	MAT mat;
	PLANE_OBJ *plane = (PLANE_OBJ*)obj->Data;

// get world mat / pos

	plane->Rot += plane->Speed * TimeFactor;
	RotMatrixY(&mat, plane->Rot);
	MulMatrix(&mat, &plane->BankMatrix, &obj->body.Centre.WMatrix);
	RotTransVector(&mat, &plane->GenPos, &plane->Offset, &obj->body.Centre.Pos);

// set propellor world pos / mat

	RotMatrixZ(&mat, (float)TIME2MS(CurrentTimer()) / 500.0f);
	MulMatrix(&obj->body.Centre.WMatrix, &mat, &plane->PropMatrix);
	RotTransVector(&obj->body.Centre.WMatrix, &obj->body.Centre.Pos, &PlanePropOff, &plane->PropPos);

// update sfx pos

#ifdef _PC
	if (obj->Sfx3D)
	{
		CopyVec(&obj->body.Centre.Pos, &obj->Sfx3D->Pos);
	}
#endif
}

////////////////////
// copter handler //
////////////////////
void AI_CopterHandler(OBJECT *obj)
{
	MAT mat;
	COPTER_OBJ *copter = (COPTER_OBJ*)obj->Data;

// move the copter
	switch (copter->State) {
	case COPTER_WAIT:
		CopterWait(obj);
		break;
	case COPTER_TURNING:
		TurnCopter(obj);
		break;
	case COPTER_FLYING:
		FlyCopter(obj);
		break;
	default:
		break;
	}


// set blade world pos / mat

	RotMatrixY(&mat, (float)TIME2MS(CurrentTimer()) / 500.0f);
	MulMatrix(&obj->body.Centre.WMatrix, &mat, &copter->BladeMatrix1);
	RotTransVector(&obj->body.Centre.WMatrix, &obj->body.Centre.Pos, &CopterBlade1Off, &copter->BladePos1);

	RotMatrixX(&mat, (float)TIME2MS(CurrentTimer()) / 400.0f);
	MulMatrix(&obj->body.Centre.WMatrix, &mat, &copter->BladeMatrix2);
	RotTransVector(&obj->body.Centre.WMatrix, &obj->body.Centre.Pos, &CopterBlade2Off, &copter->BladePos2);

// update sfx pos
#ifdef _PC
	if (obj->Sfx3D)
	{
		CopyVec(&obj->body.Centre.Pos, &obj->Sfx3D->Pos);
	}
#endif
}


void TurnCopter(OBJECT *obj)
{
	COPTER_OBJ *copter = (COPTER_OBJ*)obj->Data;

	//if (copter->TurnTime >= 5) {
	//	NewCopterDest(obj);
		copter->State = COPTER_FLYING;
		return;
	//}

	// Interpolate to new destination quaternion
	SLerpQuat(&copter->OldInitialQuat, &copter->InitialQuat, copter->TurnTime / 5, &copter->CurrentUpQuat);
	NormalizeQuat(&copter->CurrentUpQuat);
	CopyQuat(&copter->CurrentUpQuat, &obj->body.Centre.Quat);
	copter->TurnTime += TimeStep;

	// Set the copters matrix
	QuatToMat(&obj->body.Centre.Quat, &obj->body.Centre.WMatrix);

}


void FlyCopter(OBJECT *obj)
{
	bool reachedDest;
	REAL dRLen, velDest, vel, t;
	VEC dR, axis;
	QUATERNION dQuat;
	COPTER_OBJ *copter = (COPTER_OBJ*)obj->Data;

	reachedDest = FALSE;

	// Get distance from destination
	VecMinusVec(&obj->body.Centre.Pos, &copter->Destination, &dR);
	dRLen = VecLen(&dR);

	// If have reached destination, choose a new one
	if (dRLen < 10) {
		reachedDest = TRUE;
	}

	// accelerate/ decelerate towards destination
	velDest = (dRLen);
	if (velDest > copter->MaxVel) velDest = copter->MaxVel;

	vel = VecLen(&obj->body.Centre.Vel);
	if (vel < velDest) {
		vel += copter->Acc * TimeStep;
	} else if (vel > velDest) {
		vel = velDest;
	}
	if (vel < ZERO) {
		vel = ZERO;
		reachedDest = TRUE;
	}

	// Move the copter
	VecEqScalarVec(&obj->body.Centre.Vel, vel, &copter->Direction);
	VecPlusEqScalarVec(&obj->body.Centre.Pos, TimeStep, &obj->body.Centre.Vel);


	// Interpolate to new destination quaternion
	t = HALF * ((float)sin(((copter->TurnTime - 2.5f) / 5) * PI) + ONE);
	SLerpQuat(&copter->OldInitialQuat, &copter->InitialQuat, t, &copter->CurrentUpQuat);
	NormalizeQuat(&copter->CurrentUpQuat);
	copter->TurnTime += TimeStep;
	if (copter->TurnTime >= 5) copter->TurnTime = 5;


	VecCrossVec(&copter->Direction, &UpVec, &axis);
	VecMulScalar(&axis, -0.15f * vel / copter->MaxVel);
	//VecMulQuat(&axis, &copter->InitialQuat, &dQuat);
	//QuatPlusQuat(&copter->InitialQuat, &dQuat, &obj->body.Centre.Quat);
	VecMulQuat(&axis, &copter->CurrentUpQuat, &dQuat);
	QuatPlusQuat(&copter->CurrentUpQuat, &dQuat, &obj->body.Centre.Quat);
	NormalizeQuat(&obj->body.Centre.Quat);
	QuatToMat(&obj->body.Centre.Quat, &obj->body.Centre.WMatrix);

	if (reachedDest) {	
		NewCopterDest(obj);
		copter->State = COPTER_TURNING;
	}

}

void CopterWait(OBJECT *obj)
{
	REAL dPosLen;
	COPTER_OBJ *copter = (COPTER_OBJ*)obj->Data;

	// wait for 2 seconds
	copter->TurnTime += TimeStep;
	if (copter->TurnTime > 2) {

		// Choose new destination
		SetVec(&copter->Destination,
			obj->body.Centre.Pos.v[X] - 600 * obj->body.Centre.WMatrix.mv[L].v[X],
			copter->FlyBox.YMin,
			obj->body.Centre.Pos.v[Z] - 600 * obj->body.Centre.WMatrix.mv[L].v[Z]);
		VecMinusVec(&copter->Destination, &obj->body.Centre.Pos, &copter->Direction);
		dPosLen = VecLen(&copter->Direction);
		VecDivScalar(&copter->Direction, dPosLen);
		CopyQuat(&copter->CurrentUpQuat, &copter->OldInitialQuat);
		copter->State = COPTER_FLYING;
	}
}


void NewCopterDest(OBJECT *obj)
{
	int its;
	MAT newMat;
	REAL dPosLen, lookLen;
	VEC look;
	COPTER_OBJ *copter = (COPTER_OBJ*)obj->Data;

	SetVecZero(&obj->body.Centre.Vel);
	CopyQuat(&copter->CurrentUpQuat, &copter->OldInitialQuat);
	copter->TurnTime = ZERO;

	// Choose a new destination
	its = 0;
	do {
		SetVec(&copter->Destination, 
			copter->FlyBox.XMin + frand(ONE) * (copter->FlyBox.XMax - copter->FlyBox.XMin),
			copter->FlyBox.YMin + frand(ONE) * (copter->FlyBox.YMax - copter->FlyBox.YMin),
			copter->FlyBox.ZMin + frand(ONE) * (copter->FlyBox.ZMax - copter->FlyBox.ZMin));
		VecMinusVec(&copter->Destination, &obj->body.Centre.Pos, &copter->Direction);
		dPosLen = VecLen(&copter->Direction);
	} while (dPosLen < Real(1000) && ++its < 10);
	VecDivScalar(&copter->Direction, dPosLen);

	// Choose a new default orientation
	if (frand(ONE) > Real(0.2)) {
		SetVec(&look, copter->Direction.v[X], ZERO, copter->Direction.v[Z]);
		lookLen = VecLen(&look);
		if (lookLen > SMALL_REAL) {
			VecDivScalar(&look, lookLen);
			CopyVec(&look, &newMat.mv[L]);
			CopyVec(&DownVec, &newMat.mv[U]);
			VecCrossVec(&DownVec, &look, &newMat.mv[R]);
			MatToQuat(&newMat, &copter->InitialQuat);
		}
	}

	copter->State = COPTER_FLYING;
}
#endif //ifndef _PSX

////////////////////
// dragon handler //
////////////////////
#ifdef _PC
void AI_DragonHandler(OBJECT *obj)
{
	long i, j, col;
	MODEL *model;
	VEC vec;
	MAT mat;
	DRAGON_OBJ *dragon = (DRAGON_OBJ*)obj->Data;

// quit if not visible and waiting

	if (!obj->renderflag.visible && dragon->Count > 6.0f)
		return;

// get morph model

	if (dragon->HeadModel)
		model = &LevelModel[dragon->HeadModel].Model;
	else
		model = NULL;

// inc anim count

	dragon->Count += TimeStep;
	if (dragon->Count > 8.0f) dragon->Count -= 8.0f;

// handle morphs

	if (model)
	{
		if (dragon->Count <= 2.0f)
		{
			SetModelMorph(model, 0, 1, dragon->Count / 2.0f);
		}

		else if (dragon->Count <= 4.0f)
		{
			if (dragon->Count < 2.2f)
				SetModelMorph(model, 0, 1, (float)sin((dragon->Count - 2.0f) * 2.5f * RAD) * 0.03f + 1.0f);
			else
				SetModelMorph(model, 1, 0, 0);
		}

		else if (dragon->Count <= 6.0f)
		{
			SetModelMorph(model, 0, 1, (6.0f - dragon->Count) / 2.0f);
		}

		else
		{
			if (dragon->Count < 6.2f)
				SetModelMorph(model, 1, 0, (float)sin((dragon->Count - 6.0f) * 2.5f * RAD) * 0.03f + 1.0f);
			else
				SetModelMorph(model, 0, 0, 0);
		}
	}

// firestarter?

	if (dragon->Count > 2.0f && dragon->Count < 4.0f)
	{

// yep, gen light?

		if (!obj->Light)
		{
			obj->Light = AllocLight();
			if (obj->Light)
			{
				CopyVec(&dragon->FireGenPoint, (VEC*)&obj->Light->x);
				obj->Light->Reach = 1024;
				obj->Light->Flag = LIGHT_FIXED | LIGHT_MOVING;
				obj->Light->Type= LIGHT_OMNINORMAL;
				obj->Light->r = 0;
				obj->Light->g = 0;
				obj->Light->b = 0;

				obj->Sfx3D = CreateSfx3D(SFX_TOY_DRAGON, SFX_MAX_VOL, 22050, FALSE, &dragon->FireGenPoint);
			}
		}		

// flicker light?

		if (obj->Light)
		{
			if (dragon->Count < 2.5f)
			{
				FTOL((dragon->Count - 2.0f) * 448, obj->Light->r);
				obj->Light->r += rand() & 31;
			}
			else
			{
				obj->Light->r = (rand() & 31) + 224;
			}

			obj->Light->g = obj->Light->r >> 1;
		}

// new fire?

		if ((long)(CurrentTimer() - dragon->FireGenTime) >= 0)
		{
			
			for (j = 0 ; j < 2 ; j++) for (i = 0 ; i < DRAGON_FIRE_NUM ; i++) if (!dragon->Fire[i].Time)
			{
				SetVector(&vec, 0, 0, frand(32));
				RotMatrixZYX(&mat, 0, frand(1.0f), frand(0.5f) - 0.25f);
				RotTransVector(&mat, &dragon->FireGenPoint, &vec, &dragon->Fire[i].Pos);

				dragon->Fire[i].Time = 0.5f;
				dragon->Fire[i].MinSize = frand(8) + 8;
				dragon->Fire[i].Spin = frand(1);
				dragon->Fire[i].SpinSpeed = frand(0.02f) - 0.01f;

				dragon->FireGenTime = CurrentTimer() + MS2TIME(20);
				break;
			}
		}
	}

// kill light + sfx?

	else
	{
		if (obj->Light)
		{
			if (dragon->Count < 4.5f)
			{
				FTOL((4.5f - dragon->Count) * 448, obj->Light->r);
				obj->Light->r += rand() & 31;
				obj->Light->g = obj->Light->r >> 1;
			}
			else
			{
				FreeLight(obj->Light);
				obj->Light = NULL;

				if (obj->Sfx3D) if (!obj->Sfx3D->Sample)
				{
					FreeSfx3D(obj->Sfx3D);
					obj->Sfx3D = NULL;
				}
			}
		}
	}

// maintain existing fire

	for (i = 0 ; i < DRAGON_FIRE_NUM ; i++) if (dragon->Fire[i].Time)
	{
		dragon->Fire[i].Pos.v[X] += dragon->FireGenDir.v[X] * TimeFactor;
		dragon->Fire[i].Pos.v[Y] += dragon->FireGenDir.v[Y] * TimeFactor;
		dragon->Fire[i].Pos.v[Z] += dragon->FireGenDir.v[Z] * TimeFactor;

		dragon->Fire[i].Size = dragon->Fire[i].Time * 32 + dragon->Fire[i].MinSize;

		dragon->Fire[i].Spin += dragon->Fire[i].SpinSpeed * TimeFactor;
		RotMatrixZ(&dragon->Fire[i].Matrix, dragon->Fire[i].Spin);

		FTOL(dragon->Fire[i].Time * 511, col);
		dragon->Fire[i].rgb = col | (col << 8) | (col << 16);

		dragon->Fire[i].Time -= TimeStep;
		if (dragon->Fire[i].Time < 0) dragon->Fire[i].Time = 0;
	}
}
#endif

///////////////////
// water handler //
///////////////////
#ifdef _PC
void AI_WaterHandler(OBJECT *obj)
{
	long i;
	WATER_OBJ *water = (WATER_OBJ*)obj->Data;
	MODEL *model = &LevelModel[obj->DefaultModel].Model;
	WATER_VERTEX *wv;
	MODEL_VERTEX *mv;
	MODEL_POLY *mp;
	POLY_RGB *mrgb;
	VEC vec1, vec2, norm;

// quit if not visible

	if (!obj->renderflag.visible)
		return;

// move verts

	wv = water->Vert;
	mv = model->VertPtr;

	for (i = 0 ; i < water->VertNum ; i++, wv++, mv++)
	{
		wv->Time += TimeStep;
		while (wv->Time >= wv->TotalTime) wv->Time -= wv->TotalTime;
		mv->y = wv->Height + (float)sin(wv->Time / wv->TotalTime * RAD) * water->Scale;

		mv->nx = mv->ny = mv->nz = 0;
		mv->a = 0;
	}

// calc vert normals + uv's

	mp = model->PolyPtr;

	for (i = model->PolyNum ; i ; i--, mp++)
	{
		SubVector((VEC*)&mp->v1->x, (VEC*)&mp->v0->x, &vec1);
		SubVector((VEC*)&mp->v2->x, (VEC*)&mp->v0->x, &vec2);
		CrossProduct(&vec2, &vec1, &norm);
		NormalizeVector(&norm);

		AddVector((VEC*)&mp->v0->nx, &norm, (VEC*)&mp->v0->nx);
		mp->v0->a++;

		AddVector((VEC*)&mp->v1->nx, &norm, (VEC*)&mp->v1->nx);
		mp->v1->a++;

		AddVector((VEC*)&mp->v2->nx, &norm, (VEC*)&mp->v2->nx);
		mp->v2->a++;
	}

	mv = model->VertPtr;

	for (i = 0 ; i < water->VertNum ; i++, mv++)
	{
		mv->nx /= mv->a;
		mv->ny /= mv->a;
		mv->nz /= mv->a;

		mv->tu = mv->nx * 6.0f + 0.5f;
		mv->tv = mv->nz * 6.0f + 0.5f;

		FTOL((mv->ny + 1.0f) * 11000.0f + 192.0f, mv->a);
	}

// give vert uv's to poly uv's

	mp = model->PolyPtr;
	mrgb = model->PolyRGB;

	for (i = model->PolyNum ; i ; i--, mp++, mrgb++)
	{
		mp->tu0 = mp->v0->tu;
		mp->tv0 = mp->v0->tv;

		mp->tu1 = mp->v1->tu;
		mp->tv1 = mp->v1->tv;

		mp->tu2 = mp->v2->tu;
		mp->tv2 = mp->v2->tv;

		mrgb->rgb[0].a = (unsigned char)mp->v0->a;
		mrgb->rgb[1].a = (unsigned char)mp->v1->a;
		mrgb->rgb[2].a = (unsigned char)mp->v2->a;
	}	
}
#endif

#ifdef _N64
void AI_WaterHandler(OBJECT *obj)
{
	long 			ii;
	WATER_OBJ		*water = (WATER_OBJ*)obj->Data;
	MODEL 			*model = &LevelModel[obj->DefaultModel].Model;
	WATER_VERTEX	*wv;
	Vtx				*mv;

// quit if not visible
	if (!obj->renderflag.visible)
		return;

// move verts
	wv = water->Vert;
	mv = model->hdr->evtxptr;

	for (ii = 0; ii < water->VertNum; ii++, wv++, mv++)
	{
		wv->Time += TimeStep;
		while (wv->Time >= wv->TotalTime) wv->Time -= wv->TotalTime;
		mv->n.ob[1] = wv->Height + (float)sin(wv->Time / wv->TotalTime * RAD) * water->Scale;
		mv->n.n[0] = (char)((float)(sin(wv->Time / wv->TotalTime * RAD) * 127));
		mv->n.n[2] = (char)((float)(cosf(wv->Time / wv->TotalTime * RAD) * 127));
	}
}
#endif

//////////////////
// boat handler //
//////////////////
#ifdef _PC

void AI_BoatHandler(OBJECT *obj)
{
	BOAT_OBJ *boat = (BOAT_OBJ*)obj->Data;
	//VEC vec, vec2;
	MAT mat;

// quit if not visible

	if (!obj->renderflag.visible)
		return;

// update times

	boat->TimeX += TimeStep;
	while (boat->TimeX >= boat->TotalTimeX) boat->TimeX -= boat->TotalTimeX;

	boat->TimeHeight += TimeStep;
	while (boat->TimeHeight >= boat->TotalTimeHeight) boat->TimeHeight -= boat->TotalTimeHeight;

	boat->TimeZ += TimeStep;
	while (boat->TimeZ >= boat->TotalTimeZ) boat->TimeZ -= boat->TotalTimeZ;

// set height

	obj->body.Centre.Pos.v[Y] = boat->Height + (float)sin(boat->TimeHeight / boat->TotalTimeHeight * RAD) * 15.0f;

// set ori

	RotMatrixZYX(&mat, (float)sin(boat->TimeZ / boat->TotalTimeZ * RAD) / 90.0f, 0, (float)sin(boat->TimeX / boat->TotalTimeX * RAD) / 90.0f);
	MulMatrix(&boat->Ori, &mat, &obj->body.Centre.WMatrix);

// steam?

/*	boat->SteamTime += TimeStep;
	if (boat->SteamTime > Real(0.1f))
	{
		boat->SteamTime -= Real(0.1f);
		VecMulMat(&BoatSteamOffset, &obj->body.Centre.WMatrix, &vec);
		VecPlusEqVec(&vec, &obj->body.Centre.Pos);
		VecMulMat(&BoatSteamVel, &obj->body.Centre.WMatrix, &vec2);
		CreateSpark(SPARK_SMOKE2, &vec, &vec2, ZERO);
	}
*/


}
#endif

///////////////////
// radar handler //
///////////////////
#ifdef _PC

void AI_RadarHandler(OBJECT *obj)
{
	RADAR_OBJ *radar = (RADAR_OBJ*)obj->Data;

// set matrix

	radar->Time += TimeStep;
	RotMatrixY(&obj->body.Centre.WMatrix, radar->Time * 0.75f);

// set light dirs

	if (radar->Light2)
		CopyVec(&obj->body.Centre.WMatrix.mv[R], &radar->Light2->DirMatrix.mv[L]);
	
	if (radar->Light1)
		SetVector(&radar->Light1->DirMatrix.mv[L], -obj->body.Centre.WMatrix.m[RX], -obj->body.Centre.WMatrix.m[RY], -obj->body.Centre.WMatrix.m[RZ]);
}
#endif

/////////////////////
// balloon handler //
/////////////////////
#ifdef _PC

void AI_BalloonHandler(OBJECT *obj)
{
	BALLOON_OBJ *balloon = (BALLOON_OBJ*)obj->Data;

// quit if not visible

	if (!obj->renderflag.visible)
		return;

// bob

	balloon->Time += TimeStep;
	obj->body.Centre.Pos.v[Y] = balloon->Height + (float)sin(balloon->Time) * 16;
}
#endif

///////////////////
// horse handler //
///////////////////
#ifdef _PC

void AI_HorseRipper(OBJECT *obj)
{
	HORSE_OBJ *horse = (HORSE_OBJ*)obj->Data;
	MAT mat;
	REAL rock;

// get rock num

	horse->Time += TimeStep * 4.0f;
	rock = (float)sin(horse->Time);

// creak?

	if (horse->CreakFlag > 0)
	{
		if (rock > horse->CreakFlag)
		{
			horse->CreakFlag = -horse->CreakFlag;
			PlaySfx3D(SFX_TOY_CREAK, SFX_MAX_VOL, 20000, &obj->body.Centre.Pos);
		}
	}
	else
	{
		if (rock < horse->CreakFlag)
		{
			horse->CreakFlag = -horse->CreakFlag;
			PlaySfx3D(SFX_TOY_CREAK, SFX_MAX_VOL, 17000, &obj->body.Centre.Pos);
		}
	}

// quit if not visible

	if (!obj->renderflag.visible)
		return;

// rock

	RotMatrixX(&mat, rock / 50.0f);
	MulMatrix(&horse->Mat, &mat, &obj->body.Centre.WMatrix);
}
#endif

///////////////////
// train handler //
///////////////////
#ifdef _PC

void AI_TrainHandler(OBJECT *obj)
{
	long i, flag;
	TRAIN_OBJ *train = (TRAIN_OBJ*)obj->Data;
	VEC vec, vec2;
	PLAYER *player;

// rot wheels

	train->TimeFront -= TimeStep * 0.5f;
	train->TimeBack -= TimeStep * 0.3f;

// set wheel positions

	for (i = 0 ; i < 4 ; i++)
	{
		AddVector(&obj->body.Centre.Pos, &TrainWheelOffsets[i], &train->WheelPos[i]);
	}

// update sfx pos

	if (obj->Sfx3D)
	{
		CopyVec(&obj->body.Centre.Pos, &obj->Sfx3D->Pos);
	}

// steam?

	AddVector(&obj->body.Centre.Pos, &TrainSteamOffset, &vec);

	train->SteamTime += TimeStep;
	if (train->SteamTime > 0.1f)
	{
		train->SteamTime -= 0.1f;
		CreateSpark(SPARK_SMOKE2, &vec, &TrainSteamDir, ZERO, 0);
	}

// whistle?

	flag = FALSE;

	for (player = PLR_PlayerHead ; player ; player = player->next)
	{
		SubVector(&obj->body.Centre.Pos, &player->car.Body->Centre.Pos, &vec2);
		if (abs(vec2.v[X]) < 400 && vec2.v[Z] > -400 && vec2.v[Z] < 800)
		{
			flag = TRUE;
			break;
		}
	}

	if (flag)
	{
		if (train->WhistleFlag)
		{
			train->WhistleFlag = FALSE;
			PlaySfx3D(SFX_TOY_WHISTLE, SFX_MAX_VOL, 22050, &vec);
		}
	}
	else
	{
	 	train->WhistleFlag = TRUE;
	}
}
#endif

////////////////////
// strobe handler //
////////////////////
#ifdef _PC

void AI_StrobeHandler(OBJECT *obj)
{
	long num, diff, per, col;
	STROBE_OBJ *strobe = (STROBE_OBJ*)obj->Data;

// get brightness

	num = (TIME2MS(CurrentTimer()) / 20) % strobe->StrobeCount;
	diff = num - strobe->StrobeNum;
	if (diff < -strobe->StrobeCount / 2) diff += strobe->StrobeCount;
	if (diff > strobe->StrobeCount / 2) diff -= strobe->StrobeCount;

// off

	if (diff < -strobe->FadeUp || diff > strobe->FadeDown)
	{
		if (obj->Light)
		{
			FreeLight(obj->Light);
			obj->Light = NULL;
		}
		strobe->Glow = 0;
	}

// on

	else
	{
		if (!obj->Light)
		{
			obj->Light = AllocLight();
			if (obj->Light)
			{
				obj->Light->x = strobe->LightPos.v[X];
				obj->Light->y = strobe->LightPos.v[Y];
				obj->Light->z = strobe->LightPos.v[Z];
				obj->Light->Reach = strobe->Range;
				obj->Light->Flag = LIGHT_FIXED | LIGHT_MOVING;
				obj->Light->Type= LIGHT_OMNI;
			}
		}

		if (obj->Light)
		{
			if (diff < 0) per = (diff + strobe->FadeUp) * (100 / strobe->FadeUp);
			else per = (strobe->FadeDown - diff) * (100 / strobe->FadeDown);

			obj->Light->r = strobe->r * per / 100;
			obj->Light->g = strobe->g * per / 100;
			obj->Light->b = strobe->b * per / 100;

			col = (obj->Light->r << 16) | (obj->Light->g << 8) | obj->Light->b;
		}
		strobe->Glow = (float)per / 100.0f;
	}
}
#endif

/////////////////////////////////////////////////////////////////////
//
// Spark Generator Handler
//
/////////////////////////////////////////////////////////////////////

#ifdef _PC
void SparkGenHandler(OBJECT *obj)
{
	int ii, nTries;
	VEC pos, vel;
	SPARK_GEN *sparkGen = (SPARK_GEN *)obj->Data;

	// update time for this generator
	sparkGen->Time += TimeStep;

	// make sure it is visible
	if (CamVisiMask & sparkGen->VisiMask) return;

	nTries = 1 + (int)(TimeStep / sparkGen->MaxTime);
	if (nTries > 5) nTries = 5;
	for (ii = 0; ii < nTries; ii++) {
		
		// See if a new spark should be generated
		if (frand(ONE) > sparkGen->Time / sparkGen->MaxTime) continue;
		sparkGen->Time = ZERO;

		if (sparkGen->Parent != NULL) {
			// Calculate average spark velocity and start position
			VecMulMat(&obj->body.Centre.Pos, &sparkGen->Parent->body.Centre.WMatrix, &pos);
			VecPlusEqVec(&pos, &sparkGen->Parent->body.Centre.Pos);
			VecMulMat(&sparkGen->SparkVel, &sparkGen->Parent->body.Centre.WMatrix, &vel);
	
			// Generate the object-relative spark
			CreateSpark(sparkGen->Type, &pos, &vel, sparkGen->SparkVelVar, sparkGen->VisiMask);
		} else {
			// Generate the spark
			CreateSpark(sparkGen->Type, &obj->body.Centre.Pos, &sparkGen->SparkVel, sparkGen->SparkVelVar, sparkGen->VisiMask);
		}
	}
}
#endif

//////////////////////
// spaceman handler //
//////////////////////

#ifdef _PC
void AI_SpacemanHandler(OBJECT *obj)
{
	MAT mat1, mat2;
	SPACEMAN_OBJ *spaceman = (SPACEMAN_OBJ*)obj->Data;

	RotMatrixZYX(&mat1, 0.001f, 0.001f, 0);
	MulMatrix(&obj->body.Centre.WMatrix, &mat1, &mat2);
	CopyMatrix(&mat2, &obj->body.Centre.WMatrix);
}
#endif

//////////////////////////////
// pickup generator handler //
//////////////////////////////

#ifdef _PC
void AI_PickupHandler(OBJECT *obj)
{
	long col, flag;
	REAL mul;
	PICKUP_OBJ *pickup = (PICKUP_OBJ*)obj->Data;
	PLAYER *player;
	OBJECT *bombobj;
	PUTTYBOMB_OBJ *bomb;

// act on mode

	switch (pickup->Mode)
	{

// waiting to generate?

		case 0:

			pickup->Timer -= TimeStep;
			if (pickup->Timer <= 0)
			{
				pickup->Mode = 1;
				pickup->Timer = 0.0f;
				pickup->Clone = (!(rand() & 7));

				obj->EnvRGB = 0xffff80;
			}

		break;

// waiting to be picked up

		case 1:

// inc age

			pickup->Timer += TimeStep;

// spin

			RotMatrixY(&obj->body.Centre.WMatrix, TIME2MS(CurrentTimer()) / 2000.0f);
			if (pickup->Timer < 1.6f)
			{
				if (pickup->Timer < 0.5f)
					mul = 0;
				else if (pickup->Timer < 1.0f)
					mul = (pickup->Timer - 0.5f) * 2.0f + (float)sin((pickup->Timer - 0.5f) * RAD) / 1.5f;
				else if (pickup->Timer < 1.35f)
					mul = 1.0f - (float)sin((pickup->Timer - 1.0f) * 2.85714f * PI) / 6.0f;
				else
					mul = 1.0f + (float)sin((pickup->Timer - 1.35f) * 4.0f * PI) / 12.0f;

				obj->body.Centre.WMatrix.m[XX] *= mul;
				obj->body.Centre.WMatrix.m[YY] *= mul;
				obj->body.Centre.WMatrix.m[ZZ] *= mul;
			}

// need a light source?

			if (!obj->Light)
			{
				obj->Light = AllocLight();
				if (obj->Light)
				{
					CopyVec(&obj->body.Centre.Pos, (VEC*)&obj->Light->x);
					obj->Light->Reach = 512;
					obj->Light->Flag = LIGHT_FIXED | LIGHT_MOVING;
					obj->Light->Type = LIGHT_OMNI;
				}
			}

// maintain light

			if (obj->Light)
			{
				if (pickup->Timer < 0.75f)
				{
					FTOL(pickup->Timer * 340.0f, col);
				}
				else
				{
					col = 255;
				}

				if (!pickup->Clone)
				{
					obj->Light->r = col;
					obj->Light->g = col * 3 / 4;
					obj->Light->b = 0;
				}
				else
				{
					obj->Light->r = 0;
					obj->Light->g = -col;
					obj->Light->b = -col;
				}
			}

// look for car collision

			obj->CollType = COLL_TYPE_BODY;

			for (player = PLR_PlayerHead ; player ; player = player->next)
			{
				if (player != GHO_GhostPlayer && !player->PickupNum && !player->PickupCycleSpeed)
				{
					COL_NBodyColls = 0;
					DetectCarBodyColls(&player->car, &obj->body);
					if (COL_NBodyColls)
					{
						pickup->Mode = 2;
						pickup->Timer = 0.0f;

						SetVector(&pickup->Vel, player->ownobj->body.Centre.Vel.v[X] / 2.0f, -64.0f, player->ownobj->body.Centre.Vel.v[Z] / 2.0f);

						if (!pickup->Clone)
						{
							PlaySfx3D(SFX_PICKUP, SFX_MAX_VOL, 22050, &obj->body.Centre.Pos);

							player->PickupCycleType = frand(PICKUP_NUM);
							player->PickupCycleSpeed = frand(1.0f) + 4.0f;
						}
						else
						{
							PlaySfx3D(SFX_PICKUP_CLONE, SFX_MAX_VOL, 22050, &obj->body.Centre.Pos);

							flag = (long)player;
					 		bombobj = CreateObject(&player->car.Body->Centre.Pos, &player->car.Body->Centre.WMatrix, OBJECT_TYPE_PUTTYBOMB, &flag);
							if (bombobj)
							{
								bomb = (PUTTYBOMB_OBJ*)bombobj->Data;
								bomb->Timer = 0.0f;
								player->car.WillDetonate = TRUE;
							}
						}

						break;
					}
				}
			}

			obj->CollType = COLL_TYPE_NONE;

		break;

// dissappearing

		case 2:

// inc age

			pickup->Timer += TimeStep;

// spin

			RotMatrixY(&obj->body.Centre.WMatrix, TIME2MS(CurrentTimer()) / 2000.0f);

			mul = pickup->Timer * 2.0f + 1.0f;

			obj->body.Centre.WMatrix.m[XX] *= mul;
			obj->body.Centre.WMatrix.m[YY] *= mul;
			obj->body.Centre.WMatrix.m[ZZ] *= mul;

// set env rgb

			FTOL(-pickup->Timer * 255.0f + 255.0f, col);

			obj->EnvRGB = col >> 1 | col << 8 | col << 16;

// maintain light

			if (obj->Light)
			{
				if (!pickup->Clone)
				{
					obj->Light->r = col;
					obj->Light->g = col * 3 / 4;
					obj->Light->b = 0;
				}
				else
				{
					obj->Light->r = 0;
					obj->Light->g = -col;
					obj->Light->b = -col;
				}
			}

// set pos

			VecPlusScalarVec(&pickup->Pos, pickup->Timer, &pickup->Vel, &obj->body.Centre.Pos);

// done?

			if (pickup->Timer > 1.0f)
			{
				pickup->Mode = 0;
				pickup->Timer = PICKUP_GEN_TIME;

				CopyVec(&pickup->Pos, &obj->body.Centre.Pos);

				if (obj->Light)
				{
					FreeLight(obj->Light);
					obj->Light = NULL;
				}
			}

		break;
	}
}
#endif

////////////////////////////
// dissolve model handler //
////////////////////////////

#ifdef _PC
void AI_DissolveModelHandler(OBJECT *obj)
{
	long i, alpha, col;
	MODEL_POLY *mp;
	POLY_RGB *mrgb;
	DISSOLVE_OBJ *dissolve = (DISSOLVE_OBJ*)obj->Data;
	DISSOLVE_PARTICLE *particle = (DISSOLVE_PARTICLE*)(dissolve->Model.VertPtr + dissolve->Model.VertNum);
	VEC centre, delta[4];
	MAT mat;

// loop thru polys

	FTOL(255.0f - dissolve->Age * 127, alpha);
	mp = dissolve->Model.PolyPtr;
	mrgb = dissolve->Model.PolyRGB;

	for (i = 0 ; i < dissolve->Model.PolyNum ; i++, mp++, mrgb++, particle++)
	{

// set alpha

		mrgb->rgb[0].a = (unsigned char)alpha;
		mrgb->rgb[1].a = (unsigned char)alpha;
		mrgb->rgb[2].a = (unsigned char)alpha;
		mrgb->rgb[3].a = (unsigned char)alpha;

// get centre + vector offsets

		centre.v[X] = (mp->v0->x + mp->v1->x + mp->v2->x + mp->v3->x) / 4.0f;
		centre.v[Y] = (mp->v0->y + mp->v1->y + mp->v2->y + mp->v3->y) / 4.0f;
		centre.v[Z] = (mp->v0->z + mp->v1->z + mp->v2->z + mp->v3->z) / 4.0f;

		SubVector((VEC*)&mp->v0->x, &centre, &delta[0]);
		SubVector((VEC*)&mp->v1->x, &centre, &delta[1]);
		SubVector((VEC*)&mp->v2->x, &centre, &delta[2]);
		SubVector((VEC*)&mp->v3->x, &centre, &delta[3]);

// spin points + add back centre

		RotMatrixZYX(&mat, particle->Rot.v[X] * TimeStep, particle->Rot.v[Y] * TimeStep, particle->Rot.v[Z] * TimeStep);

		RotTransVector(&mat, &centre, &delta[0], (VEC*)&mp->v0->x);
		RotTransVector(&mat, &centre, &delta[1], (VEC*)&mp->v1->x);
		RotTransVector(&mat, &centre, &delta[2], (VEC*)&mp->v2->x);
		RotTransVector(&mat, &centre, &delta[3], (VEC*)&mp->v3->x);

// add velocity

		particle->Vel.v[Y] += 192.0f * TimeStep;

		VecPlusEqScalarVec((VEC*)&mp->v0->x, TimeStep, &particle->Vel);
		VecPlusEqScalarVec((VEC*)&mp->v1->x, TimeStep, &particle->Vel);
		VecPlusEqScalarVec((VEC*)&mp->v2->x, TimeStep, &particle->Vel);
		VecPlusEqScalarVec((VEC*)&mp->v3->x, TimeStep, &particle->Vel);
	}

// set env rgb

	FTOL(255.0f - dissolve->Age * 127, col);
	((MODEL_RGB*)&dissolve->EnvRGB)->r = ((MODEL_RGB*)&obj->EnvRGB)->r * col / 256;
	((MODEL_RGB*)&dissolve->EnvRGB)->g = ((MODEL_RGB*)&obj->EnvRGB)->g * col / 256;
	((MODEL_RGB*)&dissolve->EnvRGB)->b = ((MODEL_RGB*)&obj->EnvRGB)->b * col / 256;

// inc age

	dissolve->Age += TimeStep;
	if (dissolve->Age > 2.0f)
		OBJ_FreeObject(obj);
}
#endif

/////////////////////////////////////////////////////////////////////
// LaserHandler:
/////////////////////////////////////////////////////////////////////

#ifdef _PC
void AI_LaserHandler(OBJECT *obj)
{
	VEC vel, pos;
	LASER_OBJ *laser = (LASER_OBJ *)obj->Data;

	// make sure it is visible
	if (!obj->renderflag.visible) return;

	// Find the fractional distance from the laser source to intersection point with objects
	if (laser->ObjectCollide) {
		LineOfSightObj(&obj->body.Centre.Pos, &laser->Dest, &laser->Dist);
	} else {
		laser->Dist = ONE;
	}

	// Create sparks at the contact point
	if (laser->Dist < ONE) {
		VecEqScalarVec(&vel, -100, &obj->body.Centre.WMatrix.mv[L]);
		VecPlusScalarVec(&obj->body.Centre.Pos, laser->Dist, &laser->Delta, &pos)
		CreateSpark(SPARK_SPARK, &pos, &vel, 200, laser->VisiMask);
	}
}
#endif

////////////////////
// splash handler //
////////////////////

#ifdef _PC
void AI_SplashHandler(OBJECT *obj)
{
	long i;
	SPLASH_OBJ *splash = (SPLASH_OBJ *)obj->Data;
	SPLASH_POLY *spoly;
	REAL grav;

// process each poly

	spoly = splash->Poly;
	for (i = 0 ; i < SPLASH_POLY_NUM ; i++, spoly++) if (spoly->Frame < 16.0f)
	{
		spoly->Frame += spoly->FrameAdd * TimeStep;
		if (spoly->Frame >= 16.0f)
		{
			splash->Count--;
			continue;
		}

		grav = 384.0f * TimeStep;
		spoly->Vel[0].v[Y] += grav;
		spoly->Vel[1].v[Y] += grav;
		spoly->Vel[2].v[Y] += grav;
		spoly->Vel[3].v[Y] += grav;

		VecPlusEqScalarVec(&spoly->Pos[0], TimeStep, &spoly->Vel[0]);
		VecPlusEqScalarVec(&spoly->Pos[1], TimeStep, &spoly->Vel[1]);
		VecPlusEqScalarVec(&spoly->Pos[2], TimeStep, &spoly->Vel[2]);
		VecPlusEqScalarVec(&spoly->Pos[3], TimeStep, &spoly->Vel[3]);
	}

// kill?

	if (!splash->Count)
	{
		OBJ_FreeObject(obj);
	}
}
#endif


/////////////////////////////////////////////////////////////////////
//
// Speedup handlers
//
/////////////////////////////////////////////////////////////////////

#ifdef _PC

void AI_SpeedupAIHandler(OBJECT *obj)
{
	SPEEDUP_OBJ *speedup = (SPEEDUP_OBJ *)obj->Data;

	// does this speedup change state?
	if (speedup->ChangeTime == ZERO) return;

	// Is it time to change?
	speedup->Time += TimeStep;
	if (speedup->Time > speedup->ChangeTime) {
		if (speedup->Speed == speedup->LoSpeed) {
			speedup->Speed = speedup->HiSpeed;
		} else {
			speedup->Speed = speedup->LoSpeed;
		}
		speedup->Time = ZERO;
	}
}


void SpeedupImpulse(CAR *car)
{
	REAL time, depth, velDotNorm, vel, impMag;
	VEC dPos, wPos;
	BBOX bBox;
	OBJECT *obj;
	SPEEDUP_OBJ *speedup;

	// loop over objects checking for speedups
	for (obj = OBJ_ObjectHead; obj != NULL; obj = obj->next) {

		if (obj->Type != OBJECT_TYPE_SPEEDUP) continue;
		speedup = (SPEEDUP_OBJ *)obj->Data;

		// Check for collision between speedup and car
		if (!BBTestXZY(&speedup->CollPoly.BBox, &car->BBox)) continue;

		// Quick bounding-box test
		SetBBox(&bBox, 
			Min(car->Body->Centre.Pos.v[X], car->Body->Centre.OldPos.v[X]),
			Max(car->Body->Centre.Pos.v[X], car->Body->Centre.OldPos.v[X]),
			Min(car->Body->Centre.Pos.v[Y], car->Body->Centre.OldPos.v[Y]),
			Max(car->Body->Centre.Pos.v[Y], car->Body->Centre.OldPos.v[Y]),
			Min(car->Body->Centre.Pos.v[Z], car->Body->Centre.OldPos.v[Z]),
			Max(car->Body->Centre.Pos.v[Z], car->Body->Centre.OldPos.v[Z]));
		if(!BBTestYXZ(&bBox, &speedup->CollPoly.BBox)) continue;

		// Check for point passing through collision polygon
		if (!LinePlaneIntersect(&car->Body->Centre.OldPos, &car->Body->Centre.Pos, &speedup->CollPoly.Plane, &time, &depth)) {
			continue;
		}

		// Calculate the intersection point
		VecMinusVec(&car->Body->Centre.Pos, &car->Body->Centre.OldPos, &dPos);
		VecPlusScalarVec(&car->Body->Centre.OldPos, time, &dPos, &wPos);

		// Check intersection point is within the polygon boundary
		if (!PointInCollPolyBounds(&wPos, &speedup->CollPoly)) {
			continue;
		}

		// Make sure the particle is travelling towards the poly
		velDotNorm = VecDotVec(&car->Body->Centre.Vel, PlaneNormal(&speedup->CollPoly.Plane));
		if (velDotNorm > ZERO) {
			vel = VecDotVec(&car->Body->Centre.Vel, &car->Body->Centre.WMatrix.mv[L]);
			if (vel > ZERO) {
				impMag = car->Body->Centre.Mass * (speedup->Speed - vel);
			} else {
				impMag = car->Body->Centre.Mass * (-speedup->Speed - vel);
			}
		} else {
			vel = VecDotVec(&car->Body->Centre.Vel, &car->Body->Centre.WMatrix.mv[L]);
			if (vel > ZERO) {
				impMag = car->Body->Centre.Mass * (-speedup->LoSpeed - vel);
			} else {
				impMag = car->Body->Centre.Mass * (speedup->LoSpeed - vel);
			}
		}

		if (depth < ZERO) {
			VecPlusEqScalarVec(&car->Body->Centre.Shift, -2 * (depth - COLL_EPSILON), &speedup->CollPoly.Plane);
		}


		// Apply impulse to the car
		VecPlusEqScalarVec(&car->Body->Centre.Impulse, impMag, &car->Body->Centre.WMatrix.mv[L]);

	}


}
#endif

