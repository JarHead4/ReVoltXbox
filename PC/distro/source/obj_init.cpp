/*********************************************************************************************
 *
 * obj_init.cpp
 *
 * Re-Volt (Generic) Copyright (c) Probe Entertainment 1998
 * 
 * Contents:
 *			Initialisation (and destruction) functions for objects
 *			This is a companion file to ai_init.cpp that intialises the object's AI variables
 *
 *********************************************************************************************
 *
 * 01/07/98 Matt Taylor
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
#include "obj_init.h"
#include "player.h"
#include "ai.h"
#include "ai_init.h"
#include "editobj.h"
#include "drawobj.h"
#include "move.h"
#include "timing.h"
#include "visibox.h"
#include "spark.h"
#include "field.h"
#include "weapon.h"
#ifdef _PC
#include "input.h"
#endif
#ifdef _N64
#include "ffs_code.h"
#include "ffs_list.h"
#include "utils.h"
#endif

#ifdef _PC
// globals

static char *DragonMorphFrames[] = {
	"models\\dragon2.m",
	"models\\dragon3.m",
};

static VEC DragonFireOffset = {-494.2f, 208.0f, 96.0f};
static VEC DragonFireDir = {-0.45f, 2.0f, 6.0f};
#endif

STROBE_TABLE StrobeTable[] = {
	5, 10, 1024, {192, 192, 0}, {0, 0, 0},		// muse post
	5, 10, 1024, {0, 0, 128}, {0, 12, 0},		// muse wall
};

///////////////////////////
// object init functions //
///////////////////////////

static long InitBarrel(OBJECT *obj, long *flags);
static long InitFootball(OBJECT *obj, long *flags);
static long InitBeachball(OBJECT *obj, long *flags);
static long InitPlanet(OBJECT *obj, long *flags);
static long InitPlane(OBJECT *obj, long *flags);
static long InitCopter(OBJECT *obj, long *flags);
static long InitDragon(OBJECT *obj, long *flags);
static long InitWater(OBJECT *obj, long *flags);
static long InitTrolley(OBJECT *obj, long *flags);
static void FreeTrolley(OBJECT *obj);
static long InitBoat(OBJECT *obj, long *flags);
static long InitSpeedup(OBJECT *obj, long *flags);
static long InitRadar(OBJECT *obj, long *flags);
static long InitBalloon(OBJECT *obj, long *flags);
static long InitHorse(OBJECT *obj, long *flags);
static long InitTrain(OBJECT *obj, long *flags);
static long InitStrobe(OBJECT *obj, long *flags);
static long InitSparkGen(OBJECT *obj, long *flags);
static long InitSpaceman(OBJECT *obj, long *flags);
static long InitPickup(OBJECT *obj, long *flags);
static long InitDissolveModel(OBJECT *obj, long *flags);
static long InitFlap(OBJECT *obj, long *flags);
static long InitLaser(OBJECT *obj, long *flags);
static long InitSplash(OBJECT *obj, long *flags);


static OBJECT_INIT_DATA ObjInitData[] = {
	InitBarrel, sizeof(BARREL_OBJ),
	InitBeachball, 0,
	InitPlanet, 0,
	InitPlane, sizeof(PLANE_OBJ),
	InitCopter, sizeof(COPTER_OBJ),
#ifdef _PC
	InitDragon, sizeof(DRAGON_OBJ),
#else
	NULL, 0,
#endif
	InitWater, 0,
#ifdef _PC
	InitTrolley, sizeof(PLAYER),
	InitBoat, sizeof(BOAT_OBJ),
	InitSpeedup, sizeof(SPEEDUP_OBJ),
	InitRadar, sizeof(RADAR_OBJ),
	InitBalloon, sizeof(BALLOON_OBJ),
	InitHorse, sizeof(HORSE_OBJ),
	InitTrain, sizeof(TRAIN_OBJ),
	InitStrobe, sizeof(STROBE_OBJ),
	InitFootball, 0,
	InitSparkGen, sizeof(SPARK_GEN),
	InitSpaceman, sizeof(SPACEMAN_OBJ),

	InitShockwave, sizeof(SHOCKWAVE_OBJ),
	InitFirework, sizeof(FIREWORK_OBJ),
	InitPuttyBomb, 0,
	InitWaterBomb, sizeof(WATERBOMB_OBJ),
	InitElectroPulse, 0,
	InitOilSlick, sizeof(OILSLICK_OBJ),
	InitOilSlickDropper, sizeof(OILSLICK_DROPPER_OBJ),
	InitChromeBall, sizeof(CHROMEBALL_OBJ),
	InitClone, sizeof(CLONE_OBJ),
	InitTurbo2, sizeof(TURBO2_OBJ),
	InitElectroZapped, 0,
	InitSpring, sizeof(SPRING_OBJ),

	InitPickup, sizeof(PICKUP_OBJ),
	InitDissolveModel, 0,

	InitFlap, 0,
	InitLaser, sizeof(LASER_OBJ),
	InitSplash, sizeof(SPLASH_OBJ),
	InitBombGlow, 0,
#endif
};

/////////////////////////////////
// load and init level objects //
/////////////////////////////////

#ifdef _PC
void LoadObjects(char *file)
{
	long i;
	FILE *fp;
	FILE_OBJECT fileobj;
	MAT mat;

// quit if in object edit mode

	if (EditMode == EDIT_OBJECTS)
		return;

// open object file

	fp = fopen(file, "rb");
	if (!fp)
		return;

// loop thru all objects

	fread(&i, sizeof(i), 1, fp);

	for ( ; i ; i--)
	{

// read file obj

		fread(&fileobj, sizeof(fileobj), 1, fp);

// init object

		CopyVec(&fileobj.Up, &mat.mv[U]);
		CopyVec(&fileobj.Look, &mat.mv[L]);
		CrossProduct(&mat.mv[U], &mat.mv[L], &mat.mv[R]);

		CreateObject(&fileobj.Pos, &mat, fileobj.ID, fileobj.Flag);
	}

// close file

	fclose(fp);
}
#endif


//===========================================
// Load and init level objects - N64 version
//===========================================

#ifdef _N64
void LoadObjects()
{
	long i, j;
	FIL *fp;
	FILE_OBJECT fileobj;
	MAT mat;

// open object file
	printf("Loading level objects...\n");
	fp = FFS_Open(FFS_TYPE_TRACK | TRK_OBJECTS);
	if (!fp) 
	{
		printf("...could not open object file.\n");
		return;
	}

// loop thru all objects

	FFS_Read(&i, sizeof(i), fp);
	i = EndConvLong(i);

	for ( ; i ; i--)
	{

// read file obj
		FFS_Read(&fileobj, sizeof(fileobj), fp);
		fileobj.ID = EndConvLong(fileobj.ID);
		for (j = 0; j < FILE_OBJECT_FLAG_NUM; j++)
		{
			fileobj.Flag[j] = EndConvLong(fileobj.Flag[j]);
		}		
		fileobj.Pos.v[0] = EndConvReal(fileobj.Pos.v[0]);
		fileobj.Pos.v[1] = EndConvReal(fileobj.Pos.v[1]);
		fileobj.Pos.v[2] = EndConvReal(fileobj.Pos.v[2]);
		fileobj.Up.v[0] = EndConvReal(fileobj.Up.v[0]);
		fileobj.Up.v[1] = EndConvReal(fileobj.Up.v[1]);
		fileobj.Up.v[2] = EndConvReal(fileobj.Up.v[2]);
		fileobj.Look.v[0] = EndConvReal(fileobj.Look.v[0]);
		fileobj.Look.v[1] = EndConvReal(fileobj.Look.v[1]);
		fileobj.Look.v[2] = EndConvReal(fileobj.Look.v[2]);

// init object
		CopyVec(&fileobj.Up, &mat.mv[U]);
		CopyVec(&fileobj.Look, &mat.mv[L]);
		CrossProduct(&mat.mv[U], &mat.mv[L], &mat.mv[R]);

		CreateObject(&fileobj.Pos, &mat, fileobj.ID, fileobj.Flag);
	}

// close file
	FFS_Close(fp);
}
#endif


///////////////////
// create object //
///////////////////

OBJECT *CreateObject(VEC *pos, MAT *mat, long ID, long *flags)
{
	OBJECT *obj;

// legal object?

#ifdef _N64
	if (ID > OBJECT_TYPE_WATER) return NULL;
#else
	if (ID >= OBJECT_TYPE_MAX)
		return NULL;
#endif
// get object slot

	obj = OBJ_AllocObject();
	if (!obj) return NULL;

// set defaults

	obj->flag.Draw = TRUE;
	obj->flag.Move = TRUE;

	obj->renderflag.envmap = TRUE;
	obj->renderflag.envonly = FALSE;
	obj->renderflag.light = TRUE;
	obj->renderflag.litsimple = FALSE;
	obj->renderflag.reflect = TRUE;
	obj->renderflag.fog = TRUE;
	obj->renderflag.glare = FALSE;
	obj->renderflag.meshfx = TRUE;

	obj->objref = NULL;
	obj->priority = 0;
	obj->Type = ID;
	obj->EnvRGB = 0x808080;
	obj->DefaultModel = -1;
	obj->CollType = COLL_TYPE_NONE;
	obj->Light = NULL;
	obj->Field = NULL;
#ifdef _PC
	obj->Sfx3D = NULL;
#endif

	obj->aihandler = NULL;
	obj->collhandler = NULL;
	obj->movehandler = NULL;
	obj->renderhandler = (RENDER_HANDLER)RenderObject;
	obj->freehandler = NULL;

// Set up safe default values for the body

	InitBodyDefault(&obj->body);

// initialise position and orientation from mapped object

	SetBodyPos(&obj->body, pos, mat);

// alloc required data

	obj->Data = NULL;
	if (ObjInitData[ID].AllocSize)
	{
		obj->Data = malloc(ObjInitData[ID].AllocSize);
		if (!obj->Data)
		{
			OBJ_FreeObject(obj);
			return NULL;
		}
	}

// call setup function

	if (ObjInitData[ID].InitFunc)
	{
		if (!ObjInitData[ID].InitFunc(obj, flags))
		{
			OBJ_FreeObject(obj);
			return NULL;
		}
	}

// return created object

	return obj;
}


/////////////////
// init barrel //
/////////////////

static long InitBarrel(OBJECT *obj, long *flags)
{
	BARREL_OBJ *barrel = (BARREL_OBJ*)obj->Data;

// set render flags

	obj->renderflag.envmap = FALSE;
	obj->renderflag.reflect = FALSE;

// set spin speed

	barrel->SpinSpeed = (REAL)flags[0] / 32768.0f;

// set ai handler

	obj->aihandler = (AI_HANDLER)AI_BarrelHandler;

// set default model

	obj->DefaultModel = LoadOneLevelModel(LEVEL_MODEL_BARREL, TRUE, obj->renderflag, 0);

// return OK

	return TRUE;
}


#ifdef _PC
///////////////////
// init football //
///////////////////

static long InitFootball(OBJECT *obj, long *flags)
{

	// set default model
	obj->DefaultModel = LoadOneLevelModel(LEVEL_MODEL_FOOTBALL, TRUE, obj->renderflag, 0);

	// set collision handler and type
	obj->CollType = COLL_TYPE_BODY;
	obj->collhandler = (COLL_HANDLER)COL_BodyCollHandler;

	// set move handler
	obj->movehandler = (MOVE_HANDLER)MOV_MoveBody;

	// Physical properties
	obj->body.Centre.Mass = Real(0.2f);
	obj->body.Centre.InvMass = ONE / Real(0.2f);
	SetMat(&obj->body.BodyInertia, Real(100), ZERO, ZERO, ZERO, Real(100), ZERO, ZERO, ZERO, Real(100));
	SetMat(&obj->body.BodyInvInertia, ONE / Real(100), ZERO, ZERO, ZERO, ONE / Real(100), ZERO, ZERO, ZERO, ONE / Real(100));

	obj->body.Centre.Hardness = Real(0.6);
	obj->body.Centre.Resistance = Real(0.001);
	obj->body.DefaultAngRes = Real(0.005);
	obj->body.AngResistance = Real(0.005);
	obj->body.AngResMod = Real(1.0);
	obj->body.Centre.Grip = Real(0.005);
	obj->body.Centre.StaticFriction = Real(1.3);
	obj->body.Centre.KineticFriction = Real(0.8);

	// Collision skin
	SetBodySphere(&obj->body);
	obj->body.CollSkin.Sphere = (SPHERE *)malloc(sizeof(SPHERE));
	SetVecZero(&obj->body.CollSkin.Sphere[0].Pos);
	obj->body.CollSkin.Sphere[0].Radius = Real(30);
	obj->body.CollSkin.NSpheres = 1;
	CreateCopyCollSkin(&obj->body.CollSkin);
	MakeTightLocalBBox(&obj->body.CollSkin);
	BuildWorldSkin(&obj->body.CollSkin, &obj->body.Centre.Pos, &obj->body.Centre.WMatrix);

	return TRUE;
}
#endif

/////////////////////////////////////////////////////////////////////
// init beachball
/////////////////////////////////////////////////////////////////////

static long InitBeachball(OBJECT *obj, long *flags)
{

	// set env rgb
	obj->EnvRGB = 0x202000;

	// set default model
	obj->DefaultModel = LoadOneLevelModel(LEVEL_MODEL_BEACHBALL, TRUE, obj->renderflag, 0);

	// set collision handler and type
	obj->CollType = COLL_TYPE_BODY;
	obj->collhandler = (COLL_HANDLER)COL_BodyCollHandler;

	// set move handler
	obj->movehandler = (MOVE_HANDLER)MOV_MoveBody;

	// Physical properties
	obj->body.Centre.Mass = Real(0.1f);
	obj->body.Centre.InvMass = ONE / Real(0.1f);
	SetMat(&obj->body.BodyInertia, Real(100), ZERO, ZERO, ZERO, Real(100), ZERO, ZERO, ZERO, Real(100));
	SetMat(&obj->body.BodyInvInertia, ONE / Real(100), ZERO, ZERO, ZERO, ONE / Real(100), ZERO, ZERO, ZERO, ONE / Real(100));

	obj->body.Centre.Hardness = Real(0.6);
	obj->body.Centre.Resistance = Real(0.005);
	obj->body.DefaultAngRes = Real(0.005);
	obj->body.AngResistance = Real(0.005);
	obj->body.AngResMod = Real(1.0);
	obj->body.Centre.Grip = Real(0.005);
	obj->body.Centre.StaticFriction = Real(1.0);
	obj->body.Centre.KineticFriction = Real(0.5);
	obj->body.Centre.Boost = ZERO;

	// Collision skin
	SetBodySphere(&obj->body);
	obj->body.CollSkin.Sphere = (SPHERE *)malloc(sizeof(SPHERE));
	SetVecZero(&obj->body.CollSkin.Sphere[0].Pos);
	obj->body.CollSkin.Sphere[0].Radius = Real(100);
	obj->body.CollSkin.NSpheres = 1;
	CreateCopyCollSkin(&obj->body.CollSkin);
	MakeTightLocalBBox(&obj->body.CollSkin);
	BuildWorldSkin(&obj->body.CollSkin, &obj->body.Centre.Pos, &obj->body.Centre.WMatrix);

	return TRUE;
}


/////////////////
// init planet //
/////////////////

static long InitPlanet(OBJECT *obj, long *flags)
{
	long i;
	VEC vec;
	MAT mat;
	PLANET_OBJ *planet;
	SUN_OBJ *sun;
	BOUNDING_BOX box;

// alloc memory

	if (flags[0] != PLANET_SUN)
		obj->Data = malloc(sizeof(PLANET_OBJ));
	else
		obj->Data = malloc(sizeof(SUN_OBJ));

	if (!obj->Data)
		return FALSE;

	planet = (PLANET_OBJ*)obj->Data;
	sun = (SUN_OBJ*)obj->Data;

// set render flags

	obj->renderflag.envmap = FALSE;
	obj->renderflag.reflect = FALSE;

// set me / orbit planets

	planet->OwnPlanet = flags[0];
	planet->OrbitPlanet = flags[1];

// set orbit speed

	planet->OrbitSpeed = (REAL)abs(flags[2]) / 32768.0f;

// set spin speed

	planet->SpinSpeed = (REAL)flags[3] / 4096.0f;

// set ai handler

	obj->aihandler = (AI_HANDLER)AI_PlanetHandler;

// set default model?

	if (planet->OwnPlanet != PLANET_SUN)
	{
		obj->renderhandler = (RENDER_HANDLER)RenderPlanet;
		obj->DefaultModel = LoadOneLevelModel(LEVEL_MODEL_MERCURY + planet->OwnPlanet, TRUE, obj->renderflag, 0);
	}

// setup sun?

	else
	{
		obj->renderhandler = (RENDER_HANDLER)RenderSun;
		obj->Light = AllocLight();
		if (obj->Light)
		{
			obj->Light->x = obj->body.Centre.Pos.v[X];
			obj->Light->y = obj->body.Centre.Pos.v[Y];
			obj->Light->z = obj->body.Centre.Pos.v[Z];
			obj->Light->Reach = 12000;
			obj->Light->Flag = LIGHT_MOVING;
			obj->Light->Type= LIGHT_OMNINORMAL;
			obj->Light->r = 256;
			obj->Light->g = 256;
			obj->Light->b = 256;
		}

#ifdef _PC
		SunFacingPoly.Xsize = 1750;
		SunFacingPoly.Ysize = 1750;
		SunFacingPoly.U = 0.0f;
		SunFacingPoly.V = 0.0f;
		SunFacingPoly.Usize = 1.0f;
		SunFacingPoly.Vsize = 1.0f;
		SunFacingPoly.Tpage = TPAGE_MISC1;
		SunFacingPoly.RGB = 0xffffff;

		LoadTextureClever("levels\\muse2\\sun.bmp", TPAGE_MISC1, 256, 256, 0, FxTextureSet, FALSE);
#endif
		for (i = 0 ; i < SUN_OVERLAY_NUM ; i++)
		{
			sun->Overlay[i].Rot = frand(1.0f);
			sun->Overlay[i].RotVel = frand(0.001f) - 0.0005f;
		}

		for (i = 0 ; i < SUN_STAR_NUM ; i++)
		{
			SetVector(&vec, 0, 0, 6144);
			RotMatrixZYX(&mat, frand(0.5f) - 0.25f, frand(1.0f), 0);
			RotVector(&mat, &vec, &sun->Star[i].Pos);
			sun->Star[i].rgb = ((rand() & 127) + 128) | ((rand() & 127) + 128) << 8 | ((rand() & 127) + 128) << 16;
		}

		box.Xmin = obj->body.Centre.Pos.v[X] - 3072;
		box.Xmax = obj->body.Centre.Pos.v[X] + 3072;
		box.Ymin = obj->body.Centre.Pos.v[Y] - 3072;
		box.Ymax = obj->body.Centre.Pos.v[Y] + 3072;
		box.Zmin = obj->body.Centre.Pos.v[Z] - 3072;
		box.Zmax = obj->body.Centre.Pos.v[Z] + 3072;

		sun->VisiMask = SetObjectVisiMask(&box);
	}

// return OK

	return TRUE;
}


////////////////
// init plane //
////////////////

static long InitPlane(OBJECT *obj, long *flags)
{
	MAT mat, mat2;
	PLANE_OBJ *plane = (PLANE_OBJ*)obj->Data;

// set render flags

	obj->renderflag.reflect = FALSE;

// save pos

	CopyVec(&obj->body.Centre.Pos, &plane->GenPos);

// set speed

	plane->Speed = (REAL)flags[0] / 16384.0f;
	plane->Rot = 0;

// set radius

	SetVector(&plane->Offset, 0, 0, (REAL)flags[1]);

// set bank

	if (plane->Speed > 0)
		RotMatrixY(&mat2, 0.25f);
	else
		RotMatrixY(&mat2, 0.75f);

	RotMatrixZ(&mat, (REAL)flags[2] / 512.0f);
	MulMatrix(&mat2, &mat, &plane->BankMatrix);
	
// set ai handler

	obj->aihandler = (AI_HANDLER)AI_PlaneHandler;

// set render handler

	obj->renderhandler = (RENDER_HANDLER)RenderPlane;

// set default model

	obj->DefaultModel = LoadOneLevelModel(LEVEL_MODEL_PLANE, TRUE, obj->renderflag, 0);

// load propellor model

	plane->PropModel = LoadOneLevelModel(LEVEL_MODEL_PLANE_PROPELLOR, TRUE, obj->renderflag, 0);

// create 3D sfx
#ifdef _PC
	obj->Sfx3D = CreateSfx3D(SFX_TOY_PLANE, SFX_MAX_VOL, flags[0] * 100 + 11025, TRUE, &obj->body.Centre.Pos);
#endif
// return OK

	return TRUE;
}

/////////////////
// init copter //
/////////////////

static long InitCopter(OBJECT *obj, long *flags)
{
	BBOX	bBox = {-200, 200, -1000, 1000, -200, 200};
	VEC	size = {200, 2000, 200};

	COPTER_OBJ *copter = (COPTER_OBJ*)obj->Data;

// set render flags

	obj->renderflag.reflect = FALSE;

// set ai handler

	obj->aihandler = (AI_HANDLER)AI_CopterHandler;

// set render handler

	obj->renderhandler = (RENDER_HANDLER)RenderCopter;

// set default model

	obj->DefaultModel = LoadOneLevelModel(LEVEL_MODEL_COPTER, TRUE, obj->renderflag, 0);

// load extra models

	copter->BladeModel1 = LoadOneLevelModel(LEVEL_MODEL_COPTER_BLADE1, TRUE, obj->renderflag, 0);
	copter->BladeModel2 = LoadOneLevelModel(LEVEL_MODEL_COPTER_BLADE2, TRUE, obj->renderflag, 0);

// create 3D sfx
#ifdef _PC
	obj->Sfx3D = CreateSfx3D(SFX_TOY_COPTER, SFX_MAX_VOL, 22050, TRUE, &obj->body.Centre.Pos);
#endif

// set bounding box for flying around in

	copter->FlyBox.XMin = obj->body.Centre.Pos.v[X] - (float)flags[0] * 10;
	copter->FlyBox.XMax = obj->body.Centre.Pos.v[X] + (float)flags[0] * 10;
	copter->FlyBox.YMin = obj->body.Centre.Pos.v[Y] - (float)flags[1] * 10 - (float)flags[3] * 50;
	copter->FlyBox.YMax = obj->body.Centre.Pos.v[Y] + (float)flags[1] * 10 - (float)flags[3] * 50;
	copter->FlyBox.ZMin = obj->body.Centre.Pos.v[Z] - (float)flags[2] * 10;
	copter->FlyBox.ZMax = obj->body.Centre.Pos.v[Z] + (float)flags[2] * 10;

// set collision handler and type
	obj->CollType = COLL_TYPE_NONE;
	obj->collhandler = NULL;

// set motion properties of copter
	copter->State = COPTER_WAIT;
	copter->TurnTime = ZERO;
	copter->Acc = 100;
	copter->MaxVel = 300;

	obj->movehandler = NULL;

// Physical properties
	obj->body.Centre.Mass = ZERO;
	obj->body.Centre.InvMass = ZERO;
	SetMat(&obj->body.BodyInertia, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO);
	SetMat(&obj->body.BodyInvInertia, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO);

	obj->body.Centre.Hardness = Real(0.5);
	obj->body.Centre.Resistance = Real(0.0);
	obj->body.DefaultAngRes = Real(0.0);
	obj->body.AngResistance = Real(0.0);
	obj->body.AngResMod = Real(0.0);
	obj->body.Centre.Grip = Real(0.01);
	obj->body.Centre.StaticFriction = Real(1.0);
	obj->body.Centre.KineticFriction = Real(0.5);

// Store initial orientation
	MatToQuat(&obj->body.Centre.WMatrix, &copter->InitialQuat);
	CopyQuat(&copter->InitialQuat, &copter->CurrentUpQuat);

// Collision skin
	/*SetBodySphere(&obj->body);
	obj->body.CollSkin.AllowWorldColls = FALSE;
	SetBBox(&obj->body.CollSkin.BBox, Real(-150), Real(150), Real(-150), Real(150), Real(-150), Real(150));
	obj->body.CollSkin.Radius = Real(150);*/

// Force field
		obj->Field = AddLocalField(
		obj->ObjID,
		FIELD_PRIORITY_MAX, 
		&obj->body.Centre.Pos,
		&obj->body.Centre.WMatrix,
		&bBox,
		&size,
		&obj->body.Centre.WMatrix.mv[U],
		500,
		ZERO);

// return OK

	return TRUE;
}

#ifdef _PC
/////////////////
// init dragon //
/////////////////

static long InitDragon(OBJECT *obj, long *flags)
{
	long i;
	DRAGON_OBJ *dragon = (DRAGON_OBJ*)obj->Data;

// set render flags

	obj->renderflag.reflect = FALSE;

// set ai handler

	obj->aihandler = (AI_HANDLER)AI_DragonHandler;

// set render handler

	obj->renderhandler = (RENDER_HANDLER)RenderDragon;

// load body

	dragon->BodyModel = LoadOneLevelModel(LEVEL_MODEL_DRAGON1, TRUE, obj->renderflag, 0);

// load head models

	dragon->HeadModel = LoadOneLevelModel(LEVEL_MODEL_DRAGON2, TRUE, obj->renderflag, 0);
	if (dragon->HeadModel != -1)
	{
		SetModelFrames(&LevelModel[dragon->HeadModel].Model, DragonMorphFrames, 2);
	}

// set anim count

	dragon->Count = 0;

// init fire

	for (i = 0 ; i < DRAGON_FIRE_NUM ; i++)
	{
		dragon->Fire[i].Time = 0;
	}

// set fire gen point, normal

	RotTransVector(&obj->body.Centre.WMatrix, &obj->body.Centre.Pos, &DragonFireOffset, &dragon->FireGenPoint);
	RotVector(&obj->body.Centre.WMatrix, &DragonFireDir, &dragon->FireGenDir);

// setup fire facing poly

	DragonFireFacingPoly.U = 64.0f / 256.0f;
	DragonFireFacingPoly.V = 0.0f;
	DragonFireFacingPoly.Usize = 64.0f / 256.0f;
	DragonFireFacingPoly.Vsize = 64.0f / 256.0f;
	DragonFireFacingPoly.Tpage = TPAGE_FX1;

// set fire gen time stamp

	dragon->FireGenTime = CurrentTimer();

// return OK

	return TRUE;
}
#endif

////////////////
// init water //
////////////////

static long InitWater(OBJECT *obj, long *flags)
{
	long i;
	WATER_OBJ *water;
	MODEL *model;

// set render flags

	obj->renderflag.reflect = FALSE;
	obj->renderflag.envmap = FALSE;
#ifdef _N64
	obj->renderflag.envonly = TRUE;
	obj->renderflag.litsimple = FALSE;
	obj->renderhandler = (RENDER_HANDLER)RenderWater;
#endif

// set ai handler

	obj->aihandler = (AI_HANDLER)AI_WaterHandler;

// load default model

	obj->DefaultModel = LoadOneLevelModel(LEVEL_MODEL_WATER, TRUE, obj->renderflag, 0);
	if (obj->DefaultModel == -1) return FALSE;
	model = &LevelModel[obj->DefaultModel].Model;

// alloc ram

#ifdef _PC
	obj->Data = (WATER_OBJ*)malloc(sizeof(WATER_OBJ) + sizeof(WATER_VERTEX) * (model->VertNum - 1));
#endif
#ifdef _N64
	obj->Data = (WATER_OBJ*)malloc(sizeof(WATER_OBJ) + sizeof(WATER_VERTEX) * (model->hdr->vtxnum - 1));
#endif
	if (!obj->Data) return FALSE;

// load texture

#ifdef _PC
	LoadTextureClever("levels\\toylite\\water.bmp", TPAGE_MISC1, 256, 256, 0, FxTextureSet, TRUE);
#endif

// setup water

	water = (WATER_OBJ*)obj->Data;
	
	water->Scale = 5;

#ifdef _PC
	water->VertNum = model->VertNum;
#endif
#ifdef _N64
	water->VertNum = model->hdr->vtxnum;
#endif

	for (i = 0 ; i < water->VertNum ; i++)
	{
#ifdef _PC
		water->Vert[i].Height = model->VertPtr[i].y;
#endif
#ifdef _N64
		water->Vert[i].Height = model->hdr->evtxptr[i].v.ob[1];
#endif
		water->Vert[i].Time = 0;
		water->Vert[i].TotalTime = frand(2.0f) + 1.0f;
	}

// force model poly attribs
#ifdef _PC
	model->QuadNumTex = 0;
	model->TriNumTex = model->PolyNum;
	model->QuadNumRGB = 0;
	model->TriNumRGB = 0;

	for (i = 0 ; i < model->PolyNum ; i++)
	{
		model->PolyPtr[i].Type = POLY_SEMITRANS;
		model->PolyPtr[i].Tpage = TPAGE_MISC1;

		*(long*)&model->PolyRGB[i].rgb[0] = 0xc0808080;
		*(long*)&model->PolyRGB[i].rgb[1] = 0xc0808080;
		*(long*)&model->PolyRGB[i].rgb[2] = 0xc0808080;
	}
#endif
// return OK

	return TRUE;
}

#ifdef _PC
/////////////////////////////////////////////////////////////////////
//
// InitTrolley: shopping trolley
//
/////////////////////////////////////////////////////////////////////

static long InitTrolley(OBJECT *obj, long *flags)
{

	obj->player = (PLAYER *)obj->Data;

	
	obj->player->car.Body = &obj->body;
	obj->player->type = PLAYER_CPU;
	obj->player->ctrltype = CTRL_TYPE_NONE;
	obj->player->ownobj = obj;
	obj->player->Slot = 0;

	obj->player->conhandler = NULL;
	obj->CollType = COLL_TYPE_CAR;
	obj->movehandler = (MOVE_HANDLER)MOV_MoveCarNew;
	obj->collhandler = (COLL_HANDLER)COL_CarCollHandler;
	obj->aihandler = NULL;

	obj->flag.Draw = TRUE;
	obj->flag.Move = TRUE;
	obj->renderhandler = (RENDER_HANDLER)RenderTrolley;
	obj->freehandler  = (FREE_HANDLER)FreeTrolley;
	obj->Type = OBJECT_TYPE_TROLLEY;
	obj->Field = NULL;

	InitCar(&obj->player->car);
	SetupCar(obj->player, 28);
	SetCarPos(&obj->player->car, &obj->body.Centre.Pos, &obj->body.Centre.WMatrix);

	return TRUE;
}

static void FreeTrolley(OBJECT *obj)
{
	FreeCar(obj->player);
}

///////////////
// init boat //
///////////////

static long InitBoat(OBJECT *obj, long *flags)
{
	BOAT_OBJ *boat = (BOAT_OBJ*)obj->Data;

// set render flags

	obj->renderflag.reflect = FALSE;
	obj->EnvRGB = 0x404080;

// set ai handler

	obj->aihandler = (AI_HANDLER)AI_BoatHandler;

// load default model

	obj->DefaultModel = LoadOneLevelModel(LEVEL_MODEL_BOAT1 + flags[0], TRUE, obj->renderflag, 0);
	if (obj->DefaultModel == -1) return FALSE;

// setup boat

	boat->Height = obj->body.Centre.Pos.v[Y];

	CopyMat(&obj->body.Centre.WMatrix, &boat->Ori);

	boat->TimeX = boat->TimeHeight = boat->TimeZ = 0;

	boat->TotalTimeX = frand(2.0f) + 4.0f;
	boat->TotalTimeHeight = frand(1.0f) + 2.0f;
	boat->TotalTimeZ = frand(2.0f) + 4.0f;
	boat->SteamTime = ZERO;

// return OK

	return TRUE;
}

////////////////
// init radar //
////////////////

static long InitRadar(OBJECT *obj, long *flags)
{
	RADAR_OBJ *radar = (RADAR_OBJ*)obj->Data;

// set render flags

	obj->renderflag.reflect = FALSE;

// set ai handler

	obj->aihandler = (AI_HANDLER)AI_RadarHandler;

// load default model

	obj->DefaultModel = LoadOneLevelModel(LEVEL_MODEL_RADAR, TRUE, obj->renderflag, 0);
	if (obj->DefaultModel == -1) return FALSE;

// setup radar

	radar->Time = 0;

	radar->Light1 = AllocLight();
	if (radar->Light1)
	{
		radar->Light1->x = obj->body.Centre.Pos.v[X];
		radar->Light1->y = obj->body.Centre.Pos.v[Y];
		radar->Light1->z = obj->body.Centre.Pos.v[Z];
		radar->Light1->Reach = 2500;
		radar->Light1->Flag = LIGHT_FIXED | LIGHT_MOVING;
		radar->Light1->Type= LIGHT_SPOTNORMAL;
		radar->Light1->Cone = 60.0f;
		radar->Light1->r = 0;
		radar->Light1->g = 0;
		radar->Light1->b = 512;
	}

	radar->Light2 = AllocLight();
	if (radar->Light2)
	{
		radar->Light2->x = obj->body.Centre.Pos.v[X];
		radar->Light2->y = obj->body.Centre.Pos.v[Y];
		radar->Light2->z = obj->body.Centre.Pos.v[Z];
		radar->Light2->Reach = 2500;
		radar->Light2->Flag = LIGHT_FIXED | LIGHT_MOVING;
		radar->Light2->Type= LIGHT_SPOTNORMAL;
		radar->Light2->Cone = 60.0f;
		radar->Light2->r = 512;
		radar->Light2->g = 0;
		radar->Light2->b = 0;
	}

// return OK

	return TRUE;
}

/////////////////////////////////////////////////////////////////////
// InitSpeedup:
/////////////////////////////////////////////////////////////////////

static long InitSpeedup(OBJECT *object, long *flags)
{
	VEC		pos;

	MAT			*objMat = &object->body.Centre.WMatrix;
	SPEEDUP_OBJ *speedup = (SPEEDUP_OBJ *)object->Data;
	NEWCOLLPOLY *collPoly = &speedup->CollPoly;

	// Model and rendering...
	object->DefaultModel = LoadOneLevelModel(LEVEL_MODEL_SPEEDUP, TRUE, object->renderflag, TPAGE_FX1);
	if (object->DefaultModel == -1) return FALSE;

	// AI and handlers
	object->aihandler = (AI_HANDLER)AI_SpeedupAIHandler;
	object->renderhandler = (RENDER_HANDLER)RenderSpeedup;
	object->CollType = COLL_TYPE_NONE;
	//object->body.CollSkin.AllowWorldColls = FALSE;
	//object->body.CollSkin.AllowObjectColls = FALSE:

	// Other stuff
	speedup->Width = 5.0f * (REAL)flags[0];
	speedup->LoSpeed = MPH2OGU_SPEED * (REAL)flags[1];
	speedup->HiSpeed = MPH2OGU_SPEED * (REAL)flags[2];
	speedup->Speed = speedup->LoSpeed;
	speedup->ChangeTime = (REAL)flags[3];
	speedup->Time = ZERO;
	speedup->Height = 120.0f;
	SetBBox(&collPoly->BBox, LARGEDIST, -LARGEDIST, LARGEDIST, -LARGEDIST, LARGEDIST, -LARGEDIST);

	// positions of the posts
	VecPlusScalarVec(&object->body.Centre.Pos, -speedup->Width, &objMat->mv[R], &speedup->PostPos[0]);
	VecPlusScalarVec(&object->body.Centre.Pos, speedup->Width, &objMat->mv[R], &speedup->PostPos[1]);

	// Build the collision poly
	collPoly->Plane.v[A] = objMat->m[LX];
	collPoly->Plane.v[B] = objMat->m[LY];
	collPoly->Plane.v[C] = objMat->m[LZ];
	collPoly->Plane.v[D] = -VecDotVec(&objMat->mv[L], &object->body.Centre.Pos);

	VecPlusScalarVec(&object->body.Centre.Pos, -speedup->Width, &objMat->mv[R], &pos);
	collPoly->EdgePlane[0].v[A] = -objMat->m[RX];
	collPoly->EdgePlane[0].v[B] = -objMat->m[RY];
	collPoly->EdgePlane[0].v[C] = -objMat->m[RZ];
	collPoly->EdgePlane[0].v[D] = VecDotVec(&objMat->mv[R], &pos);
	AddPointToBBox(&collPoly->BBox, &pos);

	VecPlusScalarVec(&object->body.Centre.Pos, -speedup->Height, &objMat->mv[U], &pos);
	collPoly->EdgePlane[1].v[A] = -objMat->m[UX];
	collPoly->EdgePlane[1].v[B] = -objMat->m[UY];
	collPoly->EdgePlane[1].v[C] = -objMat->m[UZ];
	collPoly->EdgePlane[1].v[D] = VecDotVec(&objMat->mv[U], &pos);
	AddPointToBBox(&collPoly->BBox, &pos);

	VecPlusScalarVec(&object->body.Centre.Pos, speedup->Width, &objMat->mv[R], &pos);
	collPoly->EdgePlane[2].v[A] = objMat->m[RX];
	collPoly->EdgePlane[2].v[B] = objMat->m[RY];
	collPoly->EdgePlane[2].v[C] = objMat->m[RZ];
	collPoly->EdgePlane[2].v[D] = -VecDotVec(&objMat->mv[R], &pos);
	AddPointToBBox(&collPoly->BBox, &pos);

	CopyVec(&object->body.Centre.Pos, &pos);
	collPoly->EdgePlane[3].v[A] = objMat->m[UX];
	collPoly->EdgePlane[3].v[B] = objMat->m[UY];
	collPoly->EdgePlane[3].v[C] = objMat->m[UZ];
	collPoly->EdgePlane[3].v[D] = -VecDotVec(&objMat->mv[U], &pos);
	AddPointToBBox(&collPoly->BBox, &pos);

	collPoly->Type = POLY_QUAD;
	collPoly->Material = MATERIAL_DEFAULT;

	return TRUE;
}


//////////////////
// init balloon //
//////////////////

static long InitBalloon(OBJECT *obj, long *flags)
{
	BALLOON_OBJ *balloon = (BALLOON_OBJ*)obj->Data;

// set ai handler

	obj->aihandler = (AI_HANDLER)AI_BalloonHandler;

// load default model

	obj->DefaultModel = LoadOneLevelModel(LEVEL_MODEL_BALLOON, TRUE, obj->renderflag, 0);
	if (obj->DefaultModel == -1) return FALSE;

// init balloon

	balloon->Time = frand(1.0f);
	balloon->Height = obj->body.Centre.Pos.v[Y];

// return OK

	return TRUE;
}

////////////////
// init horse //
////////////////

static long InitHorse(OBJECT *obj, long *flags)
{
	HORSE_OBJ *horse = (HORSE_OBJ*)obj->Data;

// set render flags

	obj->renderflag.reflect = FALSE;

// set ai handler

	obj->aihandler = (AI_HANDLER)AI_HorseRipper;

// load default model

	obj->DefaultModel = LoadOneLevelModel(LEVEL_MODEL_HORSE, TRUE, obj->renderflag, 0);
	if (obj->DefaultModel == -1) return FALSE;

// init horse

	horse->Time = frand(1.0f);
	horse->Mat = obj->body.Centre.WMatrix;
	horse->CreakFlag = 0.5f;

// return OK

	return TRUE;
}

////////////////
// init train //
////////////////

static long InitTrain(OBJECT *obj, long *flags)
{
	TRAIN_OBJ *train = (TRAIN_OBJ*)obj->Data;

// set ai handler

	obj->aihandler = (AI_HANDLER)AI_TrainHandler;
	obj->movehandler = (MOVE_HANDLER)MOV_MoveTrain;

// set render handler

	obj->renderhandler = (RENDER_HANDLER)RenderTrain;

// load default model

	obj->DefaultModel = LoadOneLevelModel(LEVEL_MODEL_TRAIN, TRUE, obj->renderflag, 0);
	if (obj->DefaultModel == -1) return FALSE;

// load wheels

	train->BackWheel = LoadOneLevelModel(LEVEL_MODEL_TRAIN2, TRUE, obj->renderflag, 0);
	train->FrontWheel = LoadOneLevelModel(LEVEL_MODEL_TRAIN3, TRUE, obj->renderflag, 0);

// create sfx

	obj->Sfx3D = CreateSfx3D(SFX_TOY_TRAIN, SFX_MAX_VOL, 22050, TRUE, &obj->body.Centre.Pos);

// init train

	train->SteamTime = 0;
	train->WhistleFlag = TRUE;

// init physics stuff

	obj->body.Centre.Mass = ZERO;
	obj->body.Centre.InvMass = ZERO;
	SetMat(&obj->body.BodyInertia, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO);
	SetMat(&obj->body.BodyInvInertia, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO);

	SetVector(&obj->body.AngVel, 0, 0, 0);
	SetVector(&obj->body.Centre.Vel, 0, 0, -200.0f);

	obj->CollType = COLL_TYPE_BODY;
	obj->body.CollSkin.CollType = BODY_COLL_POLY;
	obj->body.CollSkin.AllowWorldColls = FALSE;
	obj->body.CollSkin.AllowObjColls = TRUE;

	obj->body.CollSkin.NConvex = LevelModel[obj->DefaultModel].CollSkin.NConvex;
	obj->body.CollSkin.NSpheres = LevelModel[obj->DefaultModel].CollSkin.NSpheres;
	obj->body.CollSkin.NCollPolys = LevelModel[obj->DefaultModel].CollSkin.NCollPolys;
	obj->body.CollSkin.Convex = LevelModel[obj->DefaultModel].CollSkin.Convex;
	obj->body.CollSkin.Sphere = LevelModel[obj->DefaultModel].CollSkin.Sphere;
	obj->body.CollSkin.CollPoly = LevelModel[obj->DefaultModel].CollSkin.CollPoly;
	CopyBBox(&LevelModel[obj->DefaultModel].CollSkin.TightBBox, &obj->body.CollSkin.TightBBox);

	SetBBox(&obj->body.CollSkin.TightBBox, -2000, 2000, -2000, 2000, -2000, 2000);

	CreateCopyCollSkin(&obj->body.CollSkin);
	BuildWorldSkin(&obj->body.CollSkin, &obj->body.Centre.Pos, &obj->body.Centre.WMatrix);
	TransCollPolys(obj->body.CollSkin.CollPoly, obj->body.CollSkin.NCollPolys, &obj->body.Centre.Pos);

// return OK

	return TRUE;
}

/////////////////
// init strobe //
/////////////////

static long InitStrobe(OBJECT *obj, long *flags)
{
	STROBE_OBJ *strobe = (STROBE_OBJ*)obj->Data;

// set render flags

	obj->renderflag.reflect = FALSE;
	obj->renderflag.envmap = FALSE;

// set ai handler

	obj->aihandler = (AI_HANDLER)AI_StrobeHandler;
	obj->renderhandler = (RENDER_HANDLER)RenderStrobe;

// load default model

	obj->DefaultModel = LoadOneLevelModel(LEVEL_MODEL_LIGHT1 + flags[0], TRUE, obj->renderflag, 0);
	if (obj->DefaultModel == -1) return FALSE;

// setup strobe

	strobe->StrobeNum = flags[1];
	strobe->StrobeCount = flags[2];

	strobe->FadeUp = StrobeTable[flags[0]].FadeUp;
	strobe->FadeDown = StrobeTable[flags[0]].FadeDown;

	strobe->Range = StrobeTable[flags[0]].Range;

	strobe->r = StrobeTable[flags[0]].rgb.r;
	strobe->g = StrobeTable[flags[0]].rgb.g;
	strobe->b = StrobeTable[flags[0]].rgb.b;

// setup light pos

	RotTransVector(&obj->body.Centre.WMatrix, &obj->body.Centre.Pos, &StrobeTable[flags[0]].GlowOffset, &strobe->LightPos);

// return OK

	return TRUE;
}


/////////////////////////////////////////////////////////////////////
//
// Init Spark Generator
//
/////////////////////////////////////////////////////////////////////

static long InitSparkGen(OBJECT *obj, long *flags)
{
	SPARK_GEN *sparkGen = (SPARK_GEN *)obj->Data;

	obj->aihandler = (AI_HANDLER)SparkGenHandler;

	sparkGen->Type = (enum SparkTypeEnum)flags[0];
	sparkGen->Parent = NULL;
	VecEqScalarVec(&sparkGen->SparkVel, (REAL)(flags[1] * 10), &obj->body.Centre.WMatrix.mv[L]);
	sparkGen->SparkVelVar = REAL(flags[2] * 10);
	sparkGen->MaxTime = Real(5) / Real(flags[3]);
	AddPointToBBox(&obj->body.CollSkin.BBox, &obj->body.Centre.Pos);
	sparkGen->VisiMask = SetObjectVisiMask((BOUNDING_BOX *)&obj->body.CollSkin.BBox);

	return TRUE;
}

/////////////////
// init strobe //
/////////////////

static long InitSpaceman(OBJECT *obj, long *flags)
{
	SPACEMAN_OBJ *spaceman = (SPACEMAN_OBJ*)obj->Data;

// set render flags

	obj->renderflag.reflect = FALSE;

// set ai handler

	obj->aihandler = (AI_HANDLER)AI_SpacemanHandler;

// load default model

	obj->DefaultModel = LoadOneLevelModel(LEVEL_MODEL_SPACEMAN, TRUE, obj->renderflag, 0);
	if (obj->DefaultModel == -1) return FALSE;

// return OK

	return TRUE;
}

///////////////////////////
// init pickup generator //
///////////////////////////

static long InitPickup(OBJECT *obj, long *flags)
{
	PICKUP_OBJ *pickup = (PICKUP_OBJ*)obj->Data;

// set render flags

	obj->renderflag.light = FALSE;
	obj->renderflag.glare = TRUE;

// set handlers

	obj->aihandler = (AI_HANDLER)AI_PickupHandler;
	obj->renderhandler = (RENDER_HANDLER)RenderPickup;

// set default model

	obj->DefaultModel = LoadOneLevelModel(LEVEL_MODEL_PICKUP, FALSE, obj->renderflag, 0);
	if (obj->DefaultModel == -1) return FALSE;

// setup pickup

	pickup->Mode = 0;
	pickup->Timer = 1;

	CopyVec(&obj->body.Centre.Pos, &pickup->Pos);

// setup collision info

	SetBodySphere(&obj->body);
	obj->body.CollSkin.Sphere = (SPHERE *)malloc(sizeof(SPHERE));
	SetVecZero(&obj->body.CollSkin.Sphere[0].Pos);
	obj->body.CollSkin.Sphere[0].Radius = LevelModel[obj->DefaultModel].Model.Radius;
	obj->body.CollSkin.NSpheres = 1;
	CreateCopyCollSkin(&obj->body.CollSkin);
	MakeTightLocalBBox(&obj->body.CollSkin);
	BuildWorldSkin(&obj->body.CollSkin, &obj->body.Centre.Pos, &obj->body.Centre.WMatrix);

	SetVector(&obj->body.AngVel, 0, 0, 0);
	SetVector(&obj->body.Centre.Vel, 0, 0, 0);

// return OK

	return TRUE;
}

/////////////////////////
// init dissolve model //
/////////////////////////

static long InitDissolveModel(OBJECT *obj, long *flags)
{
	long i, j, k, l, polynum, vertnum, mem, vcount, ltime, lrgb[4];
	MODEL *model = (MODEL*)flags[0];
	DISSOLVE_OBJ *dissolve;
	DISSOLVE_PARTICLE *particle;
	MODEL_POLY *mpsource, *mpdest;
	POLY_RGB *mrgbsource, *mrgbdest;
	MODEL_RGB crgb, rgb[4];
	MODEL_VERTEX **mvsource, *mvdest, cvert, vert[4];
	REAL *uvsource, time, cuv[2], uv[8];
	VEC centre;

// get mem needed

	polynum = (model->QuadNumTex + model->QuadNumRGB) * 4 + (model->TriNumTex + model->TriNumRGB) * 3;
	vertnum = (model->QuadNumTex + model->QuadNumRGB) * 16 + (model->TriNumTex + model->TriNumRGB) * 12;

	mem = sizeof(DISSOLVE_OBJ);
	mem += sizeof(MODEL_POLY) * polynum;
	mem += sizeof(POLY_RGB) * polynum;
	mem += sizeof(MODEL_VERTEX) * vertnum;

	mem += sizeof(DISSOLVE_PARTICLE) * polynum;

	obj->Data = malloc(mem);
	if (!obj->Data) return FALSE;
	dissolve = (DISSOLVE_OBJ*)obj->Data;

// setup copy model

	memcpy(&dissolve->Model, model, sizeof(MODEL));

	dissolve->Model.PolyNum = (short)polynum;
	dissolve->Model.VertNum = (short)vertnum;

	dissolve->Model.QuadNumTex = model->QuadNumTex * 4 + model->TriNumTex * 3;
	dissolve->Model.QuadNumRGB = model->QuadNumRGB * 4 + model->TriNumRGB * 3;
	dissolve->Model.TriNumTex = 0;
	dissolve->Model.TriNumRGB = 0;

	dissolve->Model.PolyPtr = (MODEL_POLY*)(dissolve + 1);
	dissolve->Model.PolyRGB = (POLY_RGB*)(dissolve->Model.PolyPtr + dissolve->Model.PolyNum);
	dissolve->Model.VertPtr = (MODEL_VERTEX*)(dissolve->Model.PolyRGB + dissolve->Model.PolyNum);

// create new polys + verts - quads then tri's

	mpdest = dissolve->Model.PolyPtr;
	mrgbdest = dissolve->Model.PolyRGB;
	mvdest = dissolve->Model.VertPtr;

	for (l = 0 ; l < 2 ; l++)
	{
		mpsource = model->PolyPtr;
		mrgbsource = model->PolyRGB;

		for (i = 0 ; i < model->PolyNum ; i++, mpsource++, mrgbsource++)
		{
			if ((l + (mpsource->Type & POLY_QUAD)) != 1) continue;

			vcount = 3 + (mpsource->Type & POLY_QUAD);
			mvsource = &mpsource->v0;
			uvsource = &mpsource->tu0;

// get new points on poly

			ZeroMemory(&cvert, sizeof(cvert));
			cuv[0] = cuv[1] = 0.0f;
			lrgb[0] = lrgb[1] = lrgb[2] = lrgb[3] = 0;

			for (j = 0 ; j < vcount ; j++)
			{
				k = (j + 1) % vcount;
				time = frand(0.5f) + 0.25f;
				FTOL(time * 256.0f, ltime);

// get edge point

				vert[j].x = mvsource[j]->x + (mvsource[k]->x - mvsource[j]->x) * time;
				vert[j].y = mvsource[j]->y + (mvsource[k]->y - mvsource[j]->y) * time;
				vert[j].z = mvsource[j]->z + (mvsource[k]->z - mvsource[j]->z) * time;

				vert[j].nx = mvsource[j]->nx + (mvsource[k]->nx - mvsource[j]->nx) * time;
				vert[j].ny = mvsource[j]->ny + (mvsource[k]->ny - mvsource[j]->ny) * time;
				vert[j].nz = mvsource[j]->nz + (mvsource[k]->nz - mvsource[j]->nz) * time;
				NormalizeVector((VEC*)&vert[j].nx);

				uv[j * 2] = uvsource[j * 2] + (uvsource[k * 2] - uvsource[j * 2]) * time;
				uv[j * 2 + 1] = uvsource[j * 2 + 1] + (uvsource[k * 2 + 1] - uvsource[j * 2] + 1) * time;

				rgb[j].r = mrgbsource->rgb[j].r + (unsigned char)(((mrgbsource->rgb[k].r - mrgbsource->rgb[j].r) * ltime) >> 8);
				rgb[j].g = mrgbsource->rgb[j].g + (unsigned char)(((mrgbsource->rgb[k].g - mrgbsource->rgb[j].g) * ltime) >> 8);
				rgb[j].b = mrgbsource->rgb[j].b + (unsigned char)(((mrgbsource->rgb[k].b - mrgbsource->rgb[j].b) * ltime) >> 8);
				rgb[j].a = mrgbsource->rgb[j].a + (unsigned char)(((mrgbsource->rgb[k].b - mrgbsource->rgb[j].b) * ltime) >> 8);

// add to centre point

				AddVector((VEC*)&cvert.x, (VEC*)&mvsource[j]->x, (VEC*)&cvert.x);
				AddVector((VEC*)&cvert.nx, (VEC*)&mvsource[j]->nx, (VEC*)&cvert.nx);

				cuv[0] += uvsource[j * 2];
				cuv[1] += uvsource[j * 2 + 1];

				lrgb[0] += mrgbsource->rgb[j].b;
				lrgb[1] += mrgbsource->rgb[j].g;
				lrgb[2] += mrgbsource->rgb[j].r;
				lrgb[3] += mrgbsource->rgb[j].a;
			}

// normalize centre point

			cvert.x /= (float)vcount;
			cvert.y /= (float)vcount;
			cvert.z /= (float)vcount;

			NormalizeVector((VEC*)&cvert.nx);

			cuv[0] /= (float)vcount;
			cuv[1] /= (float)vcount;

			crgb.b = (unsigned char)(lrgb[0] / vcount);
			crgb.g = (unsigned char)(lrgb[1] / vcount);
			crgb.r = (unsigned char)(lrgb[2] / vcount);
			crgb.a = (unsigned char)(lrgb[3] / vcount);

// build new polys from new points

			if (mpsource->Type & POLY_QUAD)
			{
// quad 1
				mpdest->Type = (mpsource->Type | POLY_DOUBLE | POLY_SEMITRANS) & ~POLY_SEMITRANS_ONE;
				mpdest->Tpage = mpsource->Tpage;

				mpdest->tu0 = mpsource->tu0;
				mpdest->tv0 = mpsource->tv0;
				mpdest->tu1 = uv[0];
				mpdest->tv1 = uv[1];
				mpdest->tu2 = cuv[0];
				mpdest->tv2 = cuv[1];
				mpdest->tu3 = uv[6];
				mpdest->tv3 = uv[7];

				mrgbdest->rgb[0] = mrgbsource->rgb[0];
				mrgbdest->rgb[1] = rgb[0];
				mrgbdest->rgb[2] = crgb;
				mrgbdest->rgb[3] = rgb[3];

				mpdest->v0 = mvdest;
				*mvdest++ = *mpsource->v0;
				mpdest->v1 = mvdest;
				*mvdest++ = vert[0];
				mpdest->v2 = mvdest;
				*mvdest++ = cvert;
				mpdest->v3 = mvdest;
				*mvdest++ = vert[3];

				mpdest++;
				mrgbdest++;
// quad 2
				mpdest->Type = (mpsource->Type | POLY_DOUBLE | POLY_SEMITRANS) & ~POLY_SEMITRANS_ONE;
				mpdest->Tpage = mpsource->Tpage;

				mpdest->tu0 = uv[0];
				mpdest->tv0 = uv[1];
				mpdest->tu1 = mpsource->tu1;
				mpdest->tv1 = mpsource->tv1;
				mpdest->tu2 = uv[2];
				mpdest->tv2 = uv[3];
				mpdest->tu3 = cuv[0];
				mpdest->tv3 = cuv[1];

				mrgbdest->rgb[0] = rgb[0];
				mrgbdest->rgb[1] = mrgbsource->rgb[1];
				mrgbdest->rgb[2] = rgb[1];
				mrgbdest->rgb[3] = crgb;

				mpdest->v0 = mvdest;
				*mvdest++ = vert[0];
				mpdest->v1 = mvdest;
				*mvdest++ = *mpsource->v1;
				mpdest->v2 = mvdest;
				*mvdest++ = vert[1];
				mpdest->v3 = mvdest;
				*mvdest++ = cvert;

				mpdest++;
				mrgbdest++;
// quad 3
				mpdest->Type = (mpsource->Type | POLY_DOUBLE | POLY_SEMITRANS) & ~POLY_SEMITRANS_ONE;
				mpdest->Tpage = mpsource->Tpage;

				mpdest->tu0 = cuv[0];
				mpdest->tv0 = cuv[1];
				mpdest->tu1 = uv[2];
				mpdest->tv1 = uv[3];
				mpdest->tu2 = mpsource->tu2;
				mpdest->tv2 = mpsource->tv2;
				mpdest->tu3 = uv[4];
				mpdest->tv3 = uv[5];

				mrgbdest->rgb[0] = crgb;
				mrgbdest->rgb[1] = rgb[1];
				mrgbdest->rgb[2] = mrgbsource->rgb[2];
				mrgbdest->rgb[3] = rgb[2];

				mpdest->v0 = mvdest;
				*mvdest++ = cvert;
				mpdest->v1 = mvdest;
				*mvdest++ = vert[1];
				mpdest->v2 = mvdest;
				*mvdest++ = *mpsource->v2;
				mpdest->v3 = mvdest;
				*mvdest++ = vert[2];

				mpdest++;
				mrgbdest++;
// quad 4
				mpdest->Type = (mpsource->Type | POLY_DOUBLE | POLY_SEMITRANS) & ~POLY_SEMITRANS_ONE;
				mpdest->Tpage = mpsource->Tpage;

				mpdest->tu0 = uv[6];
				mpdest->tv0 = uv[7];
				mpdest->tu1 = cuv[0];
				mpdest->tv1 = cuv[1];
				mpdest->tu2 = uv[4];
				mpdest->tv2 = uv[5];
				mpdest->tu3 = mpsource->tu3;
				mpdest->tv3 = mpsource->tv3;

				mrgbdest->rgb[0] = rgb[3];
				mrgbdest->rgb[1] = crgb;
				mrgbdest->rgb[2] = rgb[2];
				mrgbdest->rgb[3] = mrgbsource->rgb[3];

				mpdest->v0 = mvdest;
				*mvdest++ = vert[3];
				mpdest->v1 = mvdest;
				*mvdest++ = cvert;
				mpdest->v2 = mvdest;
				*mvdest++ = vert[2];
				mpdest->v3 = mvdest;
				*mvdest++ = *mpsource->v3;

				mpdest++;
				mrgbdest++;
			}
			else
			{
// tri 1
				mpdest->Type = (mpsource->Type | POLY_DOUBLE | POLY_SEMITRANS | POLY_QUAD) & ~POLY_SEMITRANS_ONE;
				mpdest->Tpage = mpsource->Tpage;

				mpdest->tu0 = mpsource->tu0;
				mpdest->tv0 = mpsource->tv0;
				mpdest->tu1 = uv[0];
				mpdest->tv1 = uv[1];
				mpdest->tu2 = cuv[0];
				mpdest->tv2 = cuv[1];
				mpdest->tu3 = uv[4];
				mpdest->tv3 = uv[5];

				mrgbdest->rgb[0] = mrgbsource->rgb[0];
				mrgbdest->rgb[1] = rgb[0];
				mrgbdest->rgb[2] = crgb;
				mrgbdest->rgb[3] = rgb[2];

				mpdest->v0 = mvdest;
				*mvdest++ = *mpsource->v0;
				mpdest->v1 = mvdest;
				*mvdest++ = vert[0];
				mpdest->v2 = mvdest;
				*mvdest++ = cvert;
				mpdest->v3 = mvdest;
				*mvdest++ = vert[2];

				mpdest++;
				mrgbdest++;
// tri 2
				mpdest->Type = (mpsource->Type | POLY_DOUBLE | POLY_SEMITRANS | POLY_QUAD) & ~POLY_SEMITRANS_ONE;
				mpdest->Tpage = mpsource->Tpage;

				mpdest->tu0 = uv[0];
				mpdest->tv0 = uv[1];
				mpdest->tu1 = mpsource->tu1;
				mpdest->tv1 = mpsource->tv1;
				mpdest->tu2 = uv[2];
				mpdest->tv2 = uv[3];
				mpdest->tu3 = cuv[0];
				mpdest->tv3 = cuv[1];

				mrgbdest->rgb[0] = rgb[0];
				mrgbdest->rgb[1] = mrgbsource->rgb[1];
				mrgbdest->rgb[2] = rgb[1];
				mrgbdest->rgb[3] = crgb;

				mpdest->v0 = mvdest;
				*mvdest++ = vert[0];
				mpdest->v1 = mvdest;
				*mvdest++ = *mpsource->v1;
				mpdest->v2 = mvdest;
				*mvdest++ = vert[1];
				mpdest->v3 = mvdest;
				*mvdest++ = cvert;

				mpdest++;
				mrgbdest++;
// tri 3
				mpdest->Type = (mpsource->Type | POLY_DOUBLE | POLY_SEMITRANS | POLY_QUAD) & ~POLY_SEMITRANS_ONE;
				mpdest->Tpage = mpsource->Tpage;

				mpdest->tu0 = cuv[0];
				mpdest->tv0 = cuv[1];
				mpdest->tu1 = uv[2];
				mpdest->tv1 = uv[3];
				mpdest->tu2 = mpsource->tu2;
				mpdest->tv2 = mpsource->tv2;
				mpdest->tu3 = uv[4];
				mpdest->tv3 = uv[5];

				mrgbdest->rgb[0] = crgb;
				mrgbdest->rgb[1] = rgb[1];
				mrgbdest->rgb[2] = mrgbsource->rgb[2];
				mrgbdest->rgb[3] = rgb[2];

				mpdest->v0 = mvdest;
				*mvdest++ = cvert;
				mpdest->v1 = mvdest;
				*mvdest++ = vert[1];
				mpdest->v2 = mvdest;
				*mvdest++ = *mpsource->v2;
				mpdest->v3 = mvdest;
				*mvdest++ = vert[2];

				mpdest++;
				mrgbdest++;
			}
		}
	}

// setup particles

	particle = (DISSOLVE_PARTICLE*)(dissolve->Model.VertPtr + vertnum);
	mpsource = dissolve->Model.PolyPtr;

	for (i = 0 ; i < polynum ; i++, particle++, mpsource++)
	{
		centre.v[X] = (mpsource->v0->x + mpsource->v1->x + mpsource->v2->x + mpsource->v3->x) / 4.0f;
		centre.v[Y] = (mpsource->v0->y + mpsource->v1->y + mpsource->v2->y + mpsource->v3->y) / 4.0f;
		centre.v[Z] = (mpsource->v0->z + mpsource->v1->z + mpsource->v2->z + mpsource->v3->z) / 4.0f;

		NormalizeVector(&centre);

		particle->Vel.v[X] = centre.v[X] * (frand(96.0f) + 96.0f);
		particle->Vel.v[Y] = centre.v[Y] * frand(96.0f) - 192.0f;
		particle->Vel.v[Z] = centre.v[Z] * (frand(96.0f) + 96.0f);

		particle->Rot.v[X] = frand(1.0f) - 0.5f;
		particle->Rot.v[Y] = frand(1.0f) - 0.5f;
		particle->Rot.v[Z] = frand(1.0f) - 0.5f;
	}

// setup dissolve

	dissolve->Age = 0.0f;

	CopyMatrix(&Identity, &obj->body.Centre.WMatrix);

// set render flag

	obj->renderflag.envmap = FALSE;
	obj->renderflag.light = FALSE;
	obj->renderflag.reflect = FALSE;

// set handlers

	obj->aihandler = (AI_HANDLER)AI_DissolveModelHandler;
	obj->renderhandler = (RENDER_HANDLER)RenderDissolveModel;

// return OK

	return TRUE;
}


/////////////////////////////////////////////////////////////////////
//
// InitFlap:
//
/////////////////////////////////////////////////////////////////////

long InitFlap(OBJECT *obj, long *flags)
{

	return TRUE;
}


/////////////////////////////////////////////////////////////////////
//
// InitLaser:
//
/////////////////////////////////////////////////////////////////////

long InitLaser(OBJECT *obj, long *flags)
{
	REAL dist;
	VEC dest;
	LASER_OBJ *laser = (LASER_OBJ *)obj->Data;

	obj->aihandler = (AI_HANDLER)AI_LaserHandler;
	obj->renderhandler = (RENDER_HANDLER)RenderLaser;

	// Find the default destination of the laser beam
	VecPlusScalarVec(&obj->body.Centre.Pos, 10000, &obj->body.Centre.WMatrix.mv[L], &dest);
	if (!LineOfSightDist(&obj->body.Centre.Pos, &dest, &dist, NULL)) {
		Box("Error", "Laser goes on for ever.........", MB_OK | MB_ICONWARNING);
	}
	VecPlusScalarVec(&obj->body.Centre.Pos, 10000 * dist, &obj->body.Centre.WMatrix.mv[L], &laser->Dest);
	VecMinusVec(&laser->Dest, &obj->body.Centre.Pos, &laser->Delta);
	SetBBox(&obj->body.CollSkin.BBox, LARGEDIST, -LARGEDIST, LARGEDIST, -LARGEDIST, LARGEDIST, -LARGEDIST);
	AddPointToBBox(&obj->body.CollSkin.BBox, &obj->body.Centre.Pos);
	AddPointToBBox(&obj->body.CollSkin.BBox, &laser->Dest);
	obj->body.CollSkin.Radius = obj->body.CollSkin.BBox.XMax - obj->body.CollSkin.BBox.XMin;
	obj->body.CollSkin.Radius = Max(obj->body.CollSkin.Radius, obj->body.CollSkin.BBox.YMax - obj->body.CollSkin.BBox.YMin);
	obj->body.CollSkin.Radius = Max(obj->body.CollSkin.Radius, obj->body.CollSkin.BBox.ZMax - obj->body.CollSkin.BBox.ZMin);

	laser->Width = (REAL)flags[0];
	laser->RandWidth = (REAL)flags[1];
	laser->Length = VecLen(&laser->Delta);
	laser->ObjectCollide = (bool)flags[2];
	laser->Phase = (long)frand(1000);
	laser->VisiMask = SetObjectVisiMask((BOUNDING_BOX *)&obj->body.CollSkin.BBox);

	return TRUE;
}

/////////////////////
// init splash obj //
/////////////////////

long InitSplash(OBJECT *obj, long *flags)
{
	SPLASH_OBJ *splash = (SPLASH_OBJ*)obj->Data;
	SPLASH_POLY *spoly;
	long i;
	REAL rad, size, mul;
	MAT mat, mat2;
	VEC v0, v1, v2, v3, vel;

// set ai handler

	obj->aihandler = (AI_HANDLER)AI_SplashHandler;
	obj->renderhandler = (RENDER_HANDLER)RenderSplash;

// setup splash

	splash->Count = SPLASH_POLY_NUM;

// setup each poly

	spoly = splash->Poly;

	for (i = 0 ; i < SPLASH_POLY_NUM ; i++, spoly++)
	{
		spoly->Frame = 0.0f;
		spoly->FrameAdd = (frand(0.5f) + 1.0f) * 16.0f;

		RotMatrixY(&mat, frand(1.0f));
		MulMatrix(&obj->body.Centre.WMatrix, &mat, &mat2);

		rad = frand(48.0f);
//		size = frand(32.0f) + 8.0f;
		size = frand(rad * 0.666f) + 8.0f;

		SetVector(&v0, -size, -size * 4.0f, rad * 1.8f);
		SetVector(&v1, size, -size * 4.0f, rad * 1.8f);
		SetVector(&v2, size, 0.0f, rad);
		SetVector(&v3, -size, 0.0f, rad);

		RotTransVector(&mat2, &obj->body.Centre.Pos, &v0, &spoly->Pos[0]);
		RotTransVector(&mat2, &obj->body.Centre.Pos, &v1, &spoly->Pos[1]);
		RotTransVector(&mat2, &obj->body.Centre.Pos, &v2, &spoly->Pos[2]);
		RotTransVector(&mat2, &obj->body.Centre.Pos, &v3, &spoly->Pos[3]);

		SubVector(&spoly->Pos[0], &spoly->Pos[3], &vel);
		mul = (frand(256.0f) + 128.0f) / Length(&vel);
		VecMulScalar(&vel, mul);

		SetVector(&spoly->Vel[0], vel.v[X] + frand(32.0f) - 16.0f, vel.v[Y] + frand(32.0f) - 16.0f, vel.v[Z] + frand(32.0f) - 16.0f);
		SetVector(&spoly->Vel[1], vel.v[X] + frand(32.0f) - 16.0f, vel.v[Y] + frand(32.0f) - 16.0f, vel.v[Z] + frand(32.0f) - 16.0f);
		SetVector(&spoly->Vel[2], vel.v[X] + frand(32.0f) - 16.0f, vel.v[Y] + frand(32.0f) - 16.0f, vel.v[Z] + frand(32.0f) - 16.0f);
		SetVector(&spoly->Vel[3], vel.v[X] + frand(32.0f) - 16.0f, vel.v[Y] + frand(32.0f) - 16.0f, vel.v[Z] + frand(32.0f) - 16.0f);
	}

// return OK

	return TRUE;
}

#endif


#ifdef _N64
//
// LoadOneLevelModel
//
// Loads the specified object from the level model list
//

long LoadOneLevelModel(long id, long UseMRGBper, struct renderflags renderflag, long tpage)
{
	long	ii;
	FIL		*fp;
	long	rgbper;
	long	Flag = 0;

// look for existing model
#if 0
	for (ii = 0; ii < MAX_LEVEL_MODELS ; ii++)
	{
		if (LevelModel[ii].ID == id)
		{
			LevelModel[ii].RefCount++;
			return ii;
		}
	}
#endif
// find new slot
	for (ii = 0; ii < MAX_LEVEL_MODELS; ii++)
	{
		if (LevelModel[ii].ID == -1)
		{
// load model
			if (UseMRGBper)
				rgbper = LevelInf[GameSettings.Level].ModelRGBper;
			else
				rgbper = 100;

			if (renderflag.envmap)  Flag |= MODEL_ENV;
			if (renderflag.envonly) Flag |= MODEL_ENVONLY;
			if (renderflag.light) Flag |= MODEL_LIT;

			MOD_LoadModel(LevelModelList[id].Model, LevelModelList[id].Tex, &LevelModel[ii].Model, 0x808080, rgbper, Flag);
	
// load coll skin
			if (LevelModelList[id].Coll)
			{
				if ((fp = FFS_Open(LevelModelList[id].Coll)) != NULL) 
				{
					if ((LevelModel[ii].CollSkin.Convex = LoadConvex(fp, &LevelModel[ii].CollSkin.NConvex, 0)) != NULL)
					{
						if ((LevelModel[ii].CollSkin.Sphere = LoadSpheres(fp, &LevelModel[ii].CollSkin.NSpheres)) != NULL) 
						{
							LevelModel[ii].CollSkin.CollType = BODY_COLL_CONVEX;
							MakeTightLocalBBox(&LevelModel[ii].CollSkin);
						}
					}
					FFS_Close(fp);
				}
			}

// set ID / ref count
			LevelModel[ii].ID = id;
			LevelModel[ii].RefCount = 1;
			return ii;
		}
	}

// slots full

	return -1;
}
#endif