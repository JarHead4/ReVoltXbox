
#include "revolt.h"
#include "main.h"
#include "weapon.h"
#include "car.h"
#include "ctrlread.h"
#include "player.h"
#include "geom.h"
#include "move.h"
#include "field.h"
#include "timing.h"
#include "spark.h"
#include "drawobj.h"
#ifdef _PC
#include "shadow.h"
#endif
#include "obj_init.h"
#include "visibox.h"
#ifdef _PC
#include "mirror.h"
#include "input.h"
#endif
#include "camera.h"

#define MAX_FIREWORK_AGE Real(1.2)

// prototypes

void FireWorkMove(OBJECT *obj);
void FireworkExplode(OBJECT *obj);
void TurboAIHandler(OBJECT *obj);
void TurboMoveHandler(OBJECT *obj);
void PuttyBombMove(OBJECT *obj);

// globals

long OilSlickCount;
OILSLICK_LIST OilSlickList[OILSLICK_LIST_MAX];

static VEC WaterBombVel = {0.0f, -1500.0f, 2000.0f};
static VEC WaterBombOff = {0.0f, -32.0f, 0.0f};
static VEC BombSmokeVel = {0.0f, -80.0f, 0.0f};


//////////////////////////
// reset oil slick list //
//////////////////////////

void ResetOilSlickList(void)
{
	OilSlickCount = 0;
}

///////////////////////
// weapon setup test //
///////////////////////

static void WeaponSetupTest(OBJECT *obj)
{

	// light
	obj->Light = AllocLight();
	if (obj->Light)
	{
		obj->Light->Reach = 1024;
		obj->Light->Flag = LIGHT_FIXED | LIGHT_MOVING;
		obj->Light->Type= LIGHT_OMNI;
		switch (rand() & 3)
		{
			case 0:
				obj->Light->r = 128;
				obj->Light->g = 0;
				obj->Light->b = 0;
			break;

			case 1:
				obj->Light->r = 0;
				obj->Light->g = 128;
				obj->Light->b = 0;
			break;

			case 2:
				obj->Light->r = 0;
				obj->Light->g = 0;
				obj->Light->b = 128;
			break;

			case 3:
				obj->Light->r = 128;
				obj->Light->g = 0;
				obj->Light->b = 128;
			break;
		}
	}

	// offset pos
	RotTransVector(&obj->body.Centre.WMatrix, &obj->body.Centre.Pos, &obj->player->car.WeaponOffset, &obj->body.Centre.Pos);

	// set collision handler and type
	obj->CollType = COLL_TYPE_BODY;
	obj->collhandler = (COLL_HANDLER)COL_BodyCollHandler;

	// Physical properties
	obj->body.Centre.Mass = Real(0.1f);
	obj->body.Centre.InvMass = ONE / Real(0.1f);
	SetMat(&obj->body.BodyInertia, Real(100), ZERO, ZERO, ZERO, Real(100), ZERO, ZERO, ZERO, Real(100));
	SetMat(&obj->body.BodyInvInertia, ONE / Real(100), ZERO, ZERO, ZERO, ONE / Real(100), ZERO, ZERO, ZERO, ONE / Real(100));

	obj->body.Centre.Hardness = Real(0.7);
	obj->body.Centre.Resistance = Real(0.0001);
	obj->body.DefaultAngRes = Real(0.005);
	obj->body.AngResistance = Real(0.005);
	obj->body.AngResMod = Real(1.0);
	obj->body.Centre.Grip = Real(0.015);
	obj->body.Centre.StaticFriction = Real(1.5);
	obj->body.Centre.KineticFriction = Real(1.1);

	// Collision skin
	SetBodySphere(&obj->body);
	obj->body.CollSkin.Sphere = (SPHERE *)malloc(sizeof(SPHERE));
	SetVecZero(&obj->body.CollSkin.Sphere[0].Pos);
	obj->body.CollSkin.Sphere[0].Radius = Real(30);
	obj->body.CollSkin.NSpheres = 1;
	CreateCopyCollSkin(&obj->body.CollSkin);
	MakeTightLocalBBox(&obj->body.CollSkin);
	BuildWorldSkin(&obj->body.CollSkin, &obj->body.Centre.Pos, &obj->body.Centre.WMatrix);

	// vel
	SetVector(&obj->body.Centre.Vel, obj->body.Centre.WMatrix.m[LX] * 2560, obj->body.Centre.WMatrix.m[LY] * 2560, obj->body.Centre.WMatrix.m[LZ] * 2560);
}

//////////////////////
// weapon move test //
//////////////////////

static void WeaponMoveTest(OBJECT *obj)
{

// light

	if (obj->Light)
	{
		obj->Light->x = obj->body.Centre.Pos.v[X];
		obj->Light->y = obj->body.Centre.Pos.v[Y];
		obj->Light->z = obj->body.Centre.Pos.v[Z];
	}

// move body

	MOV_MoveBody(obj);
}

#ifdef _PC
/////////////////
// init weapon //
/////////////////

long InitShockwave(OBJECT *obj, long *flags)
{
	VEC bVec, dir, axis;
	BBOX bBox;
	SHOCKWAVE_OBJ *shockwave = (SHOCKWAVE_OBJ*)obj->Data;

// remember owner player

	obj->player = (PLAYER*)flags[0];

// setup 

	shockwave->Alive = TRUE;
	shockwave->Age = 0.0f;
	shockwave->Reach = 1024.0f;

	RotTransVector(&obj->body.Centre.WMatrix, &obj->body.Centre.Pos, &obj->player->car.WeaponOffset, &obj->body.Centre.Pos);
	VecEqScalarVec(&obj->body.Centre.Vel, SHOCKWAVE_VEL, &obj->body.Centre.WMatrix.mv[L]);

	VecPlusScalarVec(&obj->body.Centre.Pos, -SHOCKWAVE_VEL, &obj->body.Centre.Vel, &shockwave->OldPos);

// setup handlers

	obj->aihandler = (AI_HANDLER)ShockwaveHandler;
	obj->renderhandler = (RENDER_HANDLER)RenderShockwave;
	obj->collhandler = (COLL_HANDLER)COL_BodyCollHandler;
	obj->movehandler = (MOVE_HANDLER)MOV_MoveBody;

// Physical properties

	obj->CollType = COLL_TYPE_BODY;

	obj->body.Centre.Mass = Real(1.0);
	obj->body.Centre.InvMass = ONE / Real(1.0);
	SetMat(&obj->body.BodyInertia, Real(100), ZERO, ZERO, ZERO, Real(100), ZERO, ZERO, ZERO, Real(100));
	SetMat(&obj->body.BodyInvInertia, ONE / Real(100), ZERO, ZERO, ZERO, ONE / Real(100), ZERO, ZERO, ZERO, ONE / Real(100));

	obj->body.Centre.Hardness = Real(0.1);
	obj->body.Centre.Resistance = Real(0.0);
	obj->body.DefaultAngRes = Real(0.0);
	obj->body.AngResistance = Real(0.0);
	obj->body.AngResMod = Real(1.0);
	obj->body.Centre.Grip = Real(0.0);
	obj->body.Centre.StaticFriction = Real(0.0);
	obj->body.Centre.KineticFriction = Real(0.0);

// Collision skin

	SetBodySphere(&obj->body);
	obj->body.CollSkin.Sphere = (SPHERE *)malloc(sizeof(SPHERE));
	SetVecZero(&obj->body.CollSkin.Sphere[0].Pos);
	obj->body.CollSkin.Sphere[0].Radius = SHOCKWAVE_RAD;
	obj->body.CollSkin.NSpheres = 1;
	CreateCopyCollSkin(&obj->body.CollSkin);
	MakeTightLocalBBox(&obj->body.CollSkin);
	BuildWorldSkin(&obj->body.CollSkin, &obj->body.Centre.Pos, &obj->body.Centre.WMatrix);

	obj->body.CollSkin.AllowObjColls = FALSE;

// setup light

	obj->Light = AllocLight();
	if (obj->Light)
	{
		obj->Light->Reach = 1024;
		obj->Light->Flag = LIGHT_FIXED | LIGHT_MOVING;
		obj->Light->Type = LIGHT_OMNI;
		obj->Light->r = 0;
		obj->Light->g = 0;
		obj->Light->b = 128;
	}

// setup sfx

	obj->Sfx3D = CreateSfx3D(SFX_SHOCKWAVE, SFX_MAX_VOL, 22050, TRUE, &obj->body.Centre.Pos);

// Add a force field

	SetBBox(&bBox, -shockwave->Reach, shockwave->Reach, -shockwave->Reach, shockwave->Reach, -shockwave->Reach, shockwave->Reach);
	SetVec(&bVec, shockwave->Reach / 2, shockwave->Reach / 2, shockwave->Reach / 2);
	VecPlusScalarVec(&obj->body.Centre.Vel, -Real(12000), &UpVec, &dir);
	NormalizeVec(&dir);
	VecEqScalarVec(&axis, Real(50000), &obj->body.Centre.WMatrix.mv[R]);
	obj->Field = AddLinearTwistField(
		obj->player->ownobj->ObjID, 
		FIELD_PRIORITY_MIN,
		&obj->body.Centre.Pos,
		&obj->body.Centre.WMatrix,
		&bBox,
		&bVec,
		&dir,
		-Real(6000),
		&axis,
		ZERO);

	obj->FieldPriority = FIELD_PRIORITY_MAX;

// return OK

	return TRUE;
}

/////////////////
// init weapon //
/////////////////

long InitFirework(OBJECT *obj, long *flags)
{
	REAL dRLen, speedMod;
	VEC dR, dir;
	MAT *playerMat;
	FIREWORK_OBJ *firework = (FIREWORK_OBJ*)obj->Data;

// set render flags

	obj->renderflag.envmap = FALSE;
	obj->renderflag.light = FALSE;

// setup handlers

	obj->movehandler = (MOVE_HANDLER)FireworkHandler;
	obj->CollType = COLL_TYPE_BODY;
	obj->collhandler = (COLL_HANDLER)COL_BodyCollHandler;

// load default model

	obj->DefaultModel = LoadOneLevelModel(LEVEL_MODEL_FIREWORK, FALSE, obj->renderflag, TPAGE_FX1);
	if (obj->DefaultModel == -1) return FALSE;

// remember owner player

	obj->player = (PLAYER*)flags[0];
	playerMat = &obj->player->car.Body->Centre.WMatrix;

	// misc
	firework->Exploded = FALSE;
	firework->Age = ZERO;
	firework->Target = obj->player->PickupTarget;

	// offset pos
	RotTransVector(&obj->body.Centre.WMatrix, &obj->body.Centre.Pos, &obj->player->car.WeaponOffset, &obj->body.Centre.Pos);

	// Physical properties
	obj->body.Centre.Mass = Real(0.1f);
	obj->body.Centre.InvMass = ONE / Real(0.1f);
	SetMat(&obj->body.BodyInertia, Real(100), ZERO, ZERO, ZERO, Real(100), ZERO, ZERO, ZERO, Real(100));
	SetMat(&obj->body.BodyInvInertia, ONE / Real(100), ZERO, ZERO, ZERO, ONE / Real(100), ZERO, ZERO, ZERO, ONE / Real(100));

	obj->body.Centre.Hardness = Real(0.7);
	obj->body.Centre.Resistance = Real(0.003);
	obj->body.DefaultAngRes = Real(0.01);
	obj->body.AngResistance = Real(0.01);
	obj->body.AngResMod = Real(1.0);
	obj->body.Centre.Grip = Real(0.015);
	obj->body.Centre.StaticFriction = Real(1.5);
	obj->body.Centre.KineticFriction = Real(1.1);

	// Collision skin
	SetBodyConvex(&obj->body);
	obj->body.CollSkin.AllowObjColls = FALSE;
	obj->body.CollSkin.NConvex = LevelModel[obj->DefaultModel].CollSkin.NConvex;
	obj->body.CollSkin.NSpheres = LevelModel[obj->DefaultModel].CollSkin.NSpheres;
	obj->body.CollSkin.Convex = LevelModel[obj->DefaultModel].CollSkin.Convex;
	obj->body.CollSkin.Sphere = LevelModel[obj->DefaultModel].CollSkin.Sphere;
	CopyBBox(&LevelModel[obj->DefaultModel].CollSkin.TightBBox, &obj->body.CollSkin.TightBBox);
	CreateCopyCollSkin(&obj->body.CollSkin);
	BuildWorldSkin(&obj->body.CollSkin, &obj->body.Centre.Pos, &obj->body.Centre.WMatrix);

	// vel and matrix
	if (firework->Target != NULL) {
		VecMinusVec(&firework->Target->body.Centre.Pos, &obj->body.Centre.Pos, &dR);
		dRLen = VecLen(&dR) / 2;
		speedMod = (dRLen - WEAPON_RANGE_MIN) / (WEAPON_RANGE_MAX - WEAPON_RANGE_MIN);
		speedMod = (speedMod * speedMod * speedMod * speedMod) / 8;
	} else {
		dRLen = WEAPON_RANGE_MAX / 3;
		speedMod = ONE / 4;
	}
	VecEqScalarVec(&dir, dRLen / WEAPON_RANGE_MAX, &playerMat->mv[U]);
	VecPlusEqScalarVec(&dir, dRLen / WEAPON_RANGE_MAX - ONE, &playerMat->mv[L]);
	NormalizeVec(&dir);
	CopyVec(&dir, &obj->body.Centre.WMatrix.mv[U]);
	BuildMatrixFromUp(&obj->body.Centre.WMatrix);
	SetBodyPos(&obj->body, &obj->body.Centre.Pos, &obj->body.Centre.WMatrix);
	VecPlusScalarVec(&obj->player->car.Body->Centre.Vel, 1500 * speedMod, &dir, &obj->body.Centre.Vel);

	// setup sfx
	obj->Sfx3D = CreateSfx3D(SFX_FIREWORK, SFX_MAX_VOL, 22050, TRUE, &obj->body.Centre.Pos);

	// Set up the trail
	firework->Trail = GetFreeTrail(TRAIL_SMOKE);
	if (firework->Trail != NULL) {
		CopyVec(&obj->body.Centre.Pos, &firework->Trail->Pos[0]);
		firework->TrailTime = ZERO;
	}

	// return OK
	return TRUE;
}

/////////////////
// init weapon //
/////////////////

long InitPuttyBomb(OBJECT *obj, long *flags)
{
	PUTTYBOMB_OBJ *bomb;
	PUTTYBOMB_VERT *vert;
	MODEL *model;
	long i;

// set render flags

	obj->renderflag.envmap = FALSE;
	obj->renderflag.light = FALSE;
	obj->renderflag.reflect = FALSE;
	obj->renderflag.meshfx = FALSE;

// setup handlers

	obj->aihandler = (AI_HANDLER)PuttyBombHandler;
	obj->movehandler = (MOVE_HANDLER)PuttyBombMove;
	obj->renderhandler = NULL;

// load bang model

	obj->DefaultModel = LoadOneLevelModel(LEVEL_MODEL_BOMBBALL, FALSE, obj->renderflag, 0);
	if (obj->DefaultModel == -1)
		return FALSE;

// remember owner player

	obj->player = (PLAYER*)flags[0];

// alloc + setup bomb

	obj->Data = malloc(sizeof(PUTTYBOMB_OBJ) + sizeof (PUTTYBOMB_VERT) * LevelModel[obj->DefaultModel].Model.VertNum);
	if (!obj->Data) return FALSE;
	bomb = (PUTTYBOMB_OBJ*)obj->Data;

	bomb->Timer = PUTTYBOMB_COUNTDOWN;
	obj->player->car.IsBomb = TRUE;
	obj->player->car.WillDetonate = FALSE;
	obj->player->car.NoReturnTimer = PUTTYBOMB_NORETURN_TIME;
	bomb->OrigAerialLen = obj->player->car.Aerial.Length;

// init bang model + verts

	model = &LevelModel[obj->DefaultModel].Model;

	if (model->QuadNumRGB)
	{
		model->QuadNumTex = model->QuadNumRGB;
		model->TriNumTex = model->TriNumRGB;
		model->QuadNumRGB = 0;
		model->TriNumRGB = 0;

		for (i = 0 ; i < model->PolyNum ; i++)
		{
			model->PolyPtr[i].Tpage = TPAGE_FX3;
			model->PolyPtr[i].Type |= POLY_DOUBLE | POLY_SEMITRANS | POLY_SEMITRANS_ONE;

			*(long*)&model->PolyRGB[i].rgb[0] = 0xffffff;
			*(long*)&model->PolyRGB[i].rgb[1] = 0xffffff;
			*(long*)&model->PolyRGB[i].rgb[2] = 0xffffff;
			*(long*)&model->PolyRGB[i].rgb[3] = 0xffffff;
		}
	}

	vert = (PUTTYBOMB_VERT*)(bomb + 1);
	for (i = 0 ; i < LevelModel[obj->DefaultModel].Model.VertNum ; i++)
	{
		vert[i].Time = frand(RAD);
		vert[i].TimeAdd = frand(5.0f) + 5.0f;
		if (rand() & 1) vert[i].TimeAdd = -vert[i].TimeAdd;
	}

// init sfx

	obj->Sfx3D = CreateSfx3D(SFX_FUSE, SFX_MAX_VOL, 22050, TRUE, &obj->player->car.Body->Centre.Pos);

// return OK

	return TRUE;
}

/////////////////
// init weapon //
/////////////////

long InitWaterBomb(OBJECT *obj, long *flags)
{
	WATERBOMB_OBJ *bomb = (WATERBOMB_OBJ*)obj->Data;
	VEC vec;

// render flags

	obj->renderflag.light = FALSE;

// setup handlers

	obj->aihandler = (AI_HANDLER)WaterBombHandler;
	obj->collhandler = (COLL_HANDLER)COL_BodyCollHandler;
	obj->movehandler = (MOVE_HANDLER)MOV_MoveBody;
	obj->renderhandler = (RENDER_HANDLER)RenderWaterBomb;

// load default model

	obj->DefaultModel = LoadOneLevelModel(LEVEL_MODEL_WATERBOMB, FALSE, obj->renderflag, 0);
	if (obj->DefaultModel == -1) return FALSE;

// remember owner player

	obj->player = (PLAYER*)flags[0];

// setup waterbomb

	bomb->Age = 0.0f;
	bomb->BangTol = frand(WATERBOMB_BANG_VAR) + WATERBOMB_BANG_MIN;

// pos / vel

	RotTransVector(&obj->body.Centre.WMatrix, &obj->body.Centre.Pos, &obj->player->car.WeaponOffset, &obj->body.Centre.Pos);
	RotVector(&obj->body.Centre.WMatrix, &WaterBombVel, &vec);
//	AddVector(&obj->player->car.Body->Centre.Vel, &vec, &obj->body.Centre.Vel);
	SetVector(&obj->body.Centre.Vel, vec.v[X] + obj->player->car.Body->Centre.Vel.v[X], vec.v[Y], vec.v[Z] + obj->player->car.Body->Centre.Vel.v[Z]);

	VecEqScalarVec(&obj->body.AngVel, -15.0f, &obj->body.Centre.WMatrix.mv[R]);

// setup sfx

	obj->Sfx3D = CreateSfx3D(SFX_WATERBOMB, SFX_MAX_VOL, 22050, TRUE, &obj->body.Centre.Pos);

// Physical properties

	obj->body.Centre.Mass = Real(0.6f);
	obj->body.Centre.InvMass = ONE / Real(0.6f);
	SetMat(&obj->body.BodyInertia, Real(100), ZERO, ZERO, ZERO, Real(100), ZERO, ZERO, ZERO, Real(100));
	SetMat(&obj->body.BodyInvInertia, ONE / Real(100), ZERO, ZERO, ZERO, ONE / Real(100), ZERO, ZERO, ZERO, ONE / Real(100));

	obj->body.Centre.Hardness = Real(0.2);
	obj->body.Centre.Resistance = Real(0.001);
	obj->body.DefaultAngRes = Real(0.005);
	obj->body.AngResistance = Real(0.005);
	obj->body.AngResMod = Real(1.0);
	obj->body.Centre.Grip = Real(0.02);
	obj->body.Centre.StaticFriction = Real(2.0);
	obj->body.Centre.KineticFriction = Real(1.5);

// Collision skin

	obj->CollType = COLL_TYPE_BODY;
	SetBodySphere(&obj->body);
	obj->body.CollSkin.Sphere = (SPHERE *)malloc(sizeof(SPHERE));
	CopyVec(&WaterBombOff, &obj->body.CollSkin.Sphere[0].Pos);
	obj->body.CollSkin.Sphere[0].Radius = WATERBOMB_RADIUS;
	obj->body.CollSkin.NSpheres = 1;
	CreateCopyCollSkin(&obj->body.CollSkin);
	MakeTightLocalBBox(&obj->body.CollSkin);
	BuildWorldSkin(&obj->body.CollSkin, &obj->body.Centre.Pos, &obj->body.Centre.WMatrix);

// return OK

	return TRUE;
}

/////////////////
// init weapon //
/////////////////

long InitElectroPulse(OBJECT *obj, long *flags)
{
	long i, ram, off;
	REAL mul;
	ELECTROPULSE_OBJ *electro;
	MODEL *smodel, *dmodel;
	ELECTROPULSE_VERT *evert;

// set render flags

	obj->renderflag.envmap = FALSE;
	obj->renderflag.light = FALSE;
	obj->renderflag.reflect = FALSE;
	obj->renderflag.meshfx = FALSE;

// setup handlers

	obj->aihandler = (AI_HANDLER)ElectroPulseHandler;
	obj->renderhandler = (RENDER_HANDLER)RenderElectroPulse;

// remember owner player

	obj->player = (PLAYER*)flags[0];

// calc + alloc ram

	smodel = &obj->player->car.Models->Body[0];

	ram = sizeof(ELECTROPULSE_OBJ);
	ram += sizeof(MODEL_POLY) * smodel->PolyNum;
	ram += sizeof(POLY_RGB) * smodel->PolyNum;
	ram += sizeof(MODEL_VERTEX) * smodel->VertNum;
	ram += sizeof(ELECTROPULSE_VERT) * smodel->VertNum;

	obj->Data = malloc(ram);
	if (!obj->Data)
		return FALSE;

// setup electro pulse

	electro = (ELECTROPULSE_OBJ*)obj->Data;
	electro->Age = 0.0f;
	electro->JumpFlag = 0;

// setup model

	dmodel = &electro->Model;

	memcpy(dmodel, smodel, sizeof(MODEL));
	dmodel->PolyPtr = (MODEL_POLY*)(electro + 1);
	dmodel->PolyRGB = (POLY_RGB*)(dmodel->PolyPtr + dmodel->PolyNum);
	dmodel->VertPtr = (MODEL_VERTEX*)(dmodel->PolyRGB + dmodel->PolyNum);

	off = (long)dmodel->VertPtr - (long)smodel->VertPtr;

	for (i = 0 ; i < dmodel->PolyNum ; i++)
	{
		dmodel->PolyPtr[i] = smodel->PolyPtr[i];

		dmodel->PolyPtr[i].Type |= POLY_SEMITRANS | POLY_SEMITRANS_ONE;
		dmodel->PolyPtr[i].Tpage = TPAGE_FX1;

		dmodel->PolyPtr[i].v0 = (MODEL_VERTEX*)((long)dmodel->PolyPtr[i].v0 + off);
		dmodel->PolyPtr[i].v1 = (MODEL_VERTEX*)((long)dmodel->PolyPtr[i].v1 + off);
		dmodel->PolyPtr[i].v2 = (MODEL_VERTEX*)((long)dmodel->PolyPtr[i].v2 + off);
		dmodel->PolyPtr[i].v3 = (MODEL_VERTEX*)((long)dmodel->PolyPtr[i].v3 + off);

		*(long*)&dmodel->PolyRGB[i].rgb[0] = 0;
		*(long*)&dmodel->PolyRGB[i].rgb[1] = 0;
		*(long*)&dmodel->PolyRGB[i].rgb[2] = 0;
		*(long*)&dmodel->PolyRGB[i].rgb[3] = 0;
	}

	for (i = 0 ; i < dmodel->VertNum ; i++)
	{
		dmodel->VertPtr[i] = smodel->VertPtr[i];

		mul = 2.0f / Length((VEC*)&dmodel->VertPtr[i].x) + 1.0f;
		dmodel->VertPtr[i].x *= mul;
		dmodel->VertPtr[i].y *= mul;
		dmodel->VertPtr[i].z *= mul;
	}

// setup electro verts

	evert = (ELECTROPULSE_VERT*)(dmodel->VertPtr + dmodel->VertNum);
	for (i = 0 ; i < dmodel->VertNum ; i++)
	{
		evert[i].Time = frand(RAD);
		evert[i].TimeAdd = frand(5.0f) + 1.0f;
		if (rand() & 1) evert[i].TimeAdd = -evert[i].TimeAdd;
	}

// setup light

	obj->Light = AllocLight();
	if (obj->Light)
	{
		obj->Light->Reach = 768;
		obj->Light->Flag = LIGHT_FIXED | LIGHT_MOVING;
		obj->Light->Type = LIGHT_OMNI;
		obj->Light->r = 0;
		obj->Light->g = 0;
		obj->Light->b = 0;
	}

// setup sfx

	obj->Sfx3D = CreateSfx3D(SFX_ELECTROPULSE, SFX_MAX_VOL, 22050, TRUE, &obj->body.Centre.Pos);

// return OK

	return TRUE;
}
#endif

/////////////////
// init weapon //
/////////////////

long InitOilSlick(OBJECT *obj, long *flags)
{
	OILSLICK_OBJ *oil = (OILSLICK_OBJ*)obj->Data;
	CAR *car;
	REAL time;
	VEC vec;

// setup handlers

	obj->aihandler = (AI_HANDLER)OilSlickHandler;
#ifdef _PC
	obj->renderhandler = (RENDER_HANDLER)RenderOilSlick;
#endif
#ifdef _N64
	obj->renderhandler = NULL;
#endif

// remember owner player

	obj->player = (PLAYER*)flags[0];

// setup

	car = &obj->player->car;

	oil->Mode = 0;
	oil->Age = 0.0f;

	CopyVec(&car->Body->Centre.Pos, &obj->body.Centre.Pos);

	SetVector(&obj->body.Centre.Vel, 0, 0, 0);

	SetVector(&vec, obj->body.Centre.Pos.v[X], obj->body.Centre.Pos.v[Y] + 1000.0f, obj->body.Centre.Pos.v[Z]);
	LineOfSightDist(&obj->body.Centre.Pos, &vec, &time, NULL);
	if (time > 0.0f && time < 1.0f)
		oil->LandHeight = obj->body.Centre.Pos.v[Y] + time * 1000.0f;
	else
		oil->LandHeight = vec.v[Y];

	oil->MaxSize = (REAL)flags[1];

// return OK

	return TRUE;
}

/////////////////
// init weapon //
/////////////////

long InitOilSlickDropper(OBJECT *obj, long *flags)
{
	OILSLICK_DROPPER_OBJ *dropper = (OILSLICK_DROPPER_OBJ*)obj->Data;

// setup handlers

	obj->aihandler = (AI_HANDLER)OilSlickDropperHandler;

// remember owner player

	obj->player = (PLAYER*)flags[0];

// setup

	dropper->Count = 0;
	dropper->Age = 0;

	CopyVec(&obj->body.Centre.Pos, &dropper->LastPos);

// return OK

	return TRUE;
}

#ifdef _PC
/////////////////
// init weapon //
/////////////////

long InitChromeBall(OBJECT *obj, long *flags)
{
	CHROMEBALL_OBJ *ball = (CHROMEBALL_OBJ*)obj->Data;
	MODEL *model;
	long i, col;

// setup ai / move handlers

	obj->aihandler = (AI_HANDLER)ChromeBallHandler;
	obj->movehandler = (MOVE_HANDLER)MOV_MoveBody;
	obj->collhandler = (COLL_HANDLER)COL_BodyCollHandler;
	obj->renderhandler = (RENDER_HANDLER)RenderChromeBall;

// load default model

	obj->DefaultModel = LoadOneLevelModel(LEVEL_MODEL_CHROMEBALL, FALSE, obj->renderflag, 0);
	if (obj->DefaultModel == -1) return FALSE;

// remember owner player

	obj->player = (PLAYER*)flags[0];

// setup ball

	ball->Age = 0.0f;
	ball->Radius = CHROMEBALL_MIN_RAD;

// pos / vel

	VecPlusScalarVec(&obj->player->car.BodyWorldPos, -70.0f, &obj->player->car.Body->Centre.WMatrix.mv[L], &obj->body.Centre.Pos);
	VecPlusScalarVec(&obj->player->car.Body->Centre.Vel, -512.0f, &obj->player->car.Body->Centre.WMatrix.mv[L], &obj->body.Centre.Vel);

// setup sfx

	obj->Sfx3D = CreateSfx3D(SFX_CHROMEBALL, SFX_MIN_VOL, 22050, TRUE, &obj->body.Centre.Pos);

// Physical properties

	obj->body.Centre.Mass = Real(3.0f);
	obj->body.Centre.InvMass = ONE / Real(3.0f);
	SetMat(&obj->body.BodyInertia, Real(100), ZERO, ZERO, ZERO, Real(100), ZERO, ZERO, ZERO, Real(100));
	SetMat(&obj->body.BodyInvInertia, ONE / Real(100), ZERO, ZERO, ZERO, ONE / Real(100), ZERO, ZERO, ZERO, ONE / Real(100));

	obj->body.Centre.Hardness = Real(0.2);
	obj->body.Centre.Resistance = Real(0.001);
	obj->body.DefaultAngRes = Real(0.005);
	obj->body.AngResistance = Real(0.005);
	obj->body.AngResMod = Real(1.0);
	obj->body.Centre.Grip = Real(0.1);
	obj->body.Centre.StaticFriction = Real(2.0);
	obj->body.Centre.KineticFriction = Real(1.0);

// Collision skin

	obj->CollType = COLL_TYPE_BODY;
	SetBodySphere(&obj->body);
	obj->body.CollSkin.Sphere = (SPHERE *)malloc(sizeof(SPHERE));
	SetVecZero(&obj->body.CollSkin.Sphere[0].Pos);
	obj->body.CollSkin.Sphere[0].Radius = CHROMEBALL_MAX_RAD;
	obj->body.CollSkin.NSpheres = 1;
	CreateCopyCollSkin(&obj->body.CollSkin);
	MakeTightLocalBBox(&obj->body.CollSkin);
	BuildWorldSkin(&obj->body.CollSkin, &obj->body.Centre.Pos, &obj->body.Centre.WMatrix);
	obj->body.CollSkin.WorldSphere[0].Radius = CHROMEBALL_MIN_RAD;

// force model textures

	model = &LevelModel[obj->DefaultModel].Model;
	if (model->QuadNumRGB)
	{
		model->QuadNumTex = model->QuadNumRGB;
		model->TriNumTex = model->TriNumRGB;
		model->QuadNumRGB = 0;
		model->TriNumRGB = 0;

		for (i = 0 ; i < model->VertNum ; i++)
		{
			col = (rand() & 63) + 32;
			model->VertPtr[i].color = col | col << 8 | col << 16;
		}

		for (i = 0 ; i < model->PolyNum ; i++)
		{
			model->PolyPtr[i].Tpage = TPAGE_FX1;
			model->PolyPtr[i].tu0 = model->PolyPtr[i].tu3 = 242.0f / 256.0f;
			model->PolyPtr[i].tu1 = model->PolyPtr[i].tu2 = 254.0f / 256.0f;
			model->PolyPtr[i].tv0 = model->PolyPtr[i].tv1 = 242.0f / 256.0f;
			model->PolyPtr[i].tv2 = model->PolyPtr[i].tv3 = 254.0f / 256.0f;

			*(long*)&model->PolyRGB[i].rgb[0] = model->PolyPtr[i].v0->color;
			*(long*)&model->PolyRGB[i].rgb[1] = model->PolyPtr[i].v1->color;
			*(long*)&model->PolyRGB[i].rgb[2] = model->PolyPtr[i].v2->color;
	
			if (model->PolyPtr[i].Type & POLY_QUAD)
				*(long*)&model->PolyRGB[i].rgb[3] = model->PolyPtr[i].v3->color;
		}
	}

// return OK

	return TRUE;
}

/////////////////
// init weapon //
/////////////////

long InitClone(OBJECT *obj, long *flags)
{
	CLONE_OBJ *clone = (CLONE_OBJ*)obj->Data;

// return OK

	return TRUE;
}

/////////////////
// init weapon //
/////////////////

long InitTurbo(OBJECT *obj, long *flags)
{
	//int iTrail;
	TURBO_OBJ *turbo = (TURBO_OBJ*)obj->Data;

// setup handlers

	obj->aihandler = (AI_HANDLER)Turbo2Handler;
	obj->movehandler = (MOVE_HANDLER)TurboMoveHandler;

// remember owner player

	obj->player = (PLAYER*)flags[0];

// setup

	turbo->Age = turbo->SparkTime = ZERO;
	turbo->LifeTime = TO_TIME(Real(3));
	turbo->Force = TO_FORCE(Real(3500));
	/*for (iTrail = 0; iTrail < TURBO_NTRAILS; iTrail++) {
		turbo->TurboTrail[iTrail] = GetFreeTrail(TRAIL_SMOKE);
		if (turbo->TurboTrail[iTrail] != NULL) {
			CopyVec(&obj->player->car.Wheel[iTrail].WPos, &turbo->TurboTrail[iTrail]->Pos[0]);
			turbo->TrailTime = ZERO;
		}
	}*/


// return OK

	return TRUE;
}

/////////////////
// init weapon //
/////////////////

long InitTurbo2(OBJECT *obj, long *flags)
{
	TURBO2_OBJ *turbo = (TURBO2_OBJ*)obj->Data;

// setup handlers

	obj->aihandler = (AI_HANDLER)Turbo2Handler;

// remember owner player

	obj->player = (PLAYER*)flags[0];

// setup

	turbo->Age = 0.0f;
	turbo->LifeTime = 3.0f;

// return OK

	return TRUE;
}

/////////////////
// init weapon //
/////////////////

long InitSpring(OBJECT *obj, long *flags)
{
	SPRING_OBJ *spring = (SPRING_OBJ*)obj->Data;

// remember owner player

	obj->player = (PLAYER*)flags[0];

// set up handlers

	obj->aihandler = (AI_HANDLER)SpringHandler;

// return OK

	return TRUE;
}

/////////////////
// init weapon //
/////////////////

long InitElectroZapped(OBJECT *obj, long *flags)
{
	long i, ram, off;
	REAL mul;
	ELECTROZAPPED_OBJ *electro;
	MODEL *smodel, *dmodel;
	ELECTROZAPPED_VERT *evert;

// set render flags

	obj->renderflag.envmap = FALSE;
	obj->renderflag.light = FALSE;
	obj->renderflag.reflect = FALSE;
	obj->renderflag.meshfx = FALSE;

// setup handlers

	obj->aihandler = (AI_HANDLER)ElectroZappedHandler;
	obj->renderhandler = (RENDER_HANDLER)RenderElectroZapped;

// remember owner player

	obj->player = (PLAYER*)flags[0];

// calc + alloc ram

	smodel = &obj->player->car.Models->Body[0];

	ram = sizeof(ELECTROZAPPED_OBJ);
	ram += sizeof(MODEL_POLY) * smodel->PolyNum;
	ram += sizeof(POLY_RGB) * smodel->PolyNum;
	ram += sizeof(MODEL_VERTEX) * smodel->VertNum;
	ram += sizeof(ELECTROZAPPED_VERT) * smodel->VertNum;

	obj->Data = malloc(ram);
	if (!obj->Data)
		return FALSE;

// setup electro zapped

	electro = (ELECTROZAPPED_OBJ*)obj->Data;

// setup model

	dmodel = &electro->Model;

	memcpy(dmodel, smodel, sizeof(MODEL));
	dmodel->PolyPtr = (MODEL_POLY*)(electro + 1);
	dmodel->PolyRGB = (POLY_RGB*)(dmodel->PolyPtr + dmodel->PolyNum);
	dmodel->VertPtr = (MODEL_VERTEX*)(dmodel->PolyRGB + dmodel->PolyNum);

	off = (long)dmodel->VertPtr - (long)smodel->VertPtr;

	for (i = 0 ; i < dmodel->PolyNum ; i++)
	{
		dmodel->PolyPtr[i] = smodel->PolyPtr[i];

		dmodel->PolyPtr[i].Type |= POLY_SEMITRANS | POLY_SEMITRANS_ONE;
		dmodel->PolyPtr[i].Tpage = TPAGE_FX1;

		dmodel->PolyPtr[i].v0 = (MODEL_VERTEX*)((long)dmodel->PolyPtr[i].v0 + off);
		dmodel->PolyPtr[i].v1 = (MODEL_VERTEX*)((long)dmodel->PolyPtr[i].v1 + off);
		dmodel->PolyPtr[i].v2 = (MODEL_VERTEX*)((long)dmodel->PolyPtr[i].v2 + off);
		dmodel->PolyPtr[i].v3 = (MODEL_VERTEX*)((long)dmodel->PolyPtr[i].v3 + off);

		*(long*)&dmodel->PolyRGB[i].rgb[0] = 0;
		*(long*)&dmodel->PolyRGB[i].rgb[1] = 0;
		*(long*)&dmodel->PolyRGB[i].rgb[2] = 0;
		*(long*)&dmodel->PolyRGB[i].rgb[3] = 0;
	}

	for (i = 0 ; i < dmodel->VertNum ; i++)
	{
		dmodel->VertPtr[i] = smodel->VertPtr[i];

		mul = 2.0f / Length((VEC*)&dmodel->VertPtr[i].x) + 1.0f;
		dmodel->VertPtr[i].x *= mul;
		dmodel->VertPtr[i].y *= mul;
		dmodel->VertPtr[i].z *= mul;
	}

// setup electro verts

	evert = (ELECTROZAPPED_VERT*)(dmodel->VertPtr + dmodel->VertNum);
	for (i = 0 ; i < dmodel->VertNum ; i++)
	{
		evert[i].Time = frand(RAD);
		evert[i].TimeAdd = frand(5.0f) + 1.0f;
		if (rand() & 1) evert[i].TimeAdd = -evert[i].TimeAdd;
	}

// return OK

	return TRUE;
}

/////////////////
// init weapon //
/////////////////

long InitBombGlow(OBJECT *obj, long *flags)
{
	long i, ram, off;
	REAL mul;
	BOMBGLOW_OBJ *glow;
	MODEL *smodel, *dmodel;
	BOMBGLOW_VERT *gvert;

// set render flags

	obj->renderflag.envmap = FALSE;
	obj->renderflag.light = FALSE;
	obj->renderflag.reflect = FALSE;
	obj->renderflag.meshfx = FALSE;

// setup handlers

	obj->aihandler = (AI_HANDLER)BombGlowHandler;
	obj->renderhandler = (RENDER_HANDLER)RenderBombGlow;

// remember owner player

	obj->player = (PLAYER*)flags[0];

// calc + alloc ram

	smodel = &obj->player->car.Models->Body[0];

	ram = sizeof(BOMBGLOW_OBJ);
	ram += sizeof(MODEL_POLY) * smodel->PolyNum;
	ram += sizeof(POLY_RGB) * smodel->PolyNum;
	ram += sizeof(MODEL_VERTEX) * smodel->VertNum;
	ram += sizeof(BOMBGLOW_VERT) * smodel->VertNum;

	obj->Data = malloc(ram);
	if (!obj->Data)
		return FALSE;

// setup bomb glow

	glow = (BOMBGLOW_OBJ*)obj->Data;
	glow->Timer = 0.0f;

// setup model

	dmodel = &glow->Model;

	memcpy(dmodel, smodel, sizeof(MODEL));
	dmodel->PolyPtr = (MODEL_POLY*)(glow + 1);
	dmodel->PolyRGB = (POLY_RGB*)(dmodel->PolyPtr + dmodel->PolyNum);
	dmodel->VertPtr = (MODEL_VERTEX*)(dmodel->PolyRGB + dmodel->PolyNum);

	off = (long)dmodel->VertPtr - (long)smodel->VertPtr;

	for (i = 0 ; i < dmodel->PolyNum ; i++)
	{
		dmodel->PolyPtr[i] = smodel->PolyPtr[i];

		dmodel->PolyPtr[i].Type |= POLY_SEMITRANS | POLY_SEMITRANS_ONE;
		dmodel->PolyPtr[i].Tpage = TPAGE_FX3;

		dmodel->PolyPtr[i].v0 = (MODEL_VERTEX*)((long)dmodel->PolyPtr[i].v0 + off);
		dmodel->PolyPtr[i].v1 = (MODEL_VERTEX*)((long)dmodel->PolyPtr[i].v1 + off);
		dmodel->PolyPtr[i].v2 = (MODEL_VERTEX*)((long)dmodel->PolyPtr[i].v2 + off);
		dmodel->PolyPtr[i].v3 = (MODEL_VERTEX*)((long)dmodel->PolyPtr[i].v3 + off);

		*(long*)&dmodel->PolyRGB[i].rgb[0] = 0;
		*(long*)&dmodel->PolyRGB[i].rgb[1] = 0;
		*(long*)&dmodel->PolyRGB[i].rgb[2] = 0;
		*(long*)&dmodel->PolyRGB[i].rgb[3] = 0;
	}

	for (i = 0 ; i < dmodel->VertNum ; i++)
	{
		dmodel->VertPtr[i] = smodel->VertPtr[i];

		mul = 2.0f / Length((VEC*)&dmodel->VertPtr[i].x) + 1.0f;
		dmodel->VertPtr[i].x *= mul;
		dmodel->VertPtr[i].y *= mul;
		dmodel->VertPtr[i].z *= mul;
	}

// setup glow verts

	gvert = (BOMBGLOW_VERT*)(dmodel->VertPtr + dmodel->VertNum);
	for (i = 0 ; i < dmodel->VertNum ; i++)
	{
		gvert[i].Time = frand(RAD);
		gvert[i].TimeAdd = frand(5.0f) + 1.0f;
		if (rand() & 1) gvert[i].TimeAdd = -gvert[i].TimeAdd;
	}

// return OK

	return TRUE;
}

////////////////////
// weapon handler //
////////////////////

void ShockwaveHandler(OBJECT *obj)
{
	SHOCKWAVE_OBJ *shockwave = (SHOCKWAVE_OBJ*)obj->Data;
	VEC vec, vec2, vec3, vec4;
	REAL mul;
	long i, count;

// alive?

	if (shockwave->Alive)
	{

// inc age

		shockwave->Age += TimeStep;

// create particles

		FTOL(TimeStep * 200.0f, count);
		CopyVec(&obj->body.Centre.Pos, &vec3);
		VecEqScalarVec(&vec4, 1.0f / 200.0f, &obj->body.Centre.Vel);

		for (i = 0 ; i < count ; i++)
		{
			SetVector(&vec, frand(1.0f) - 0.5f, frand(1.0f) - 0.5f, 0.0f);
			mul = 64.0f / Length(&vec);
			VecMulScalar(&vec, mul);
			RotVector(&obj->body.Centre.WMatrix, &vec, &vec2);
			VecPlusScalarVec(&vec3, 0.1f, &vec2, &vec);
			CreateSpark(SPARK_BLUE, &vec, &vec2, 0.0f, 0);

			AddVector(&vec3, &vec4, &vec3);
		}

// maintain light

		if (obj->Light)
		{
			obj->Light->x = obj->body.Centre.Pos.v[X];
			obj->Light->y = obj->body.Centre.Pos.v[Y];
			obj->Light->z = obj->body.Centre.Pos.v[Z];
		}

// maintain sfx

		if (obj->Sfx3D)
		{
			CopyVec(&obj->body.Centre.Pos, &obj->Sfx3D->Pos);
		}

// stuck?

		if (shockwave->Age > 0.25f)
		{
			SubVector(&obj->body.Centre.Pos, &shockwave->OldPos, &vec);
			if (Length(&vec) / TimeStep < SHOCKWAVE_MIN_VEL)
				shockwave->Age = SHOCKWAVE_MAX_AGE;
		}

		CopyVec(&obj->body.Centre.Pos, &shockwave->OldPos);

// kill?

		if (shockwave->Age >= SHOCKWAVE_MAX_AGE)
		{
			shockwave->Alive = FALSE;

			if (obj->Sfx3D)
				FreeSfx3D(obj->Sfx3D);

			if (obj->Field)
			{
				RemoveField(obj->Field);
				obj->Field = NULL;
			}

			for (i = 0 ; i < 128 ; i++)
			{
				SetVector(&vec, frand(2.0f) - 1.0f, -frand(3.0f), frand(2.0f) - 1.0f);
				mul = (frand(512.0f) + 512.0f) / Length(&vec);
				VecMulScalar(&vec, mul);
				CreateSpark(SPARK_BIGBLUE, &obj->body.Centre.Pos, &vec, 0.0f, 0);
			}
		}
	}

// dying

	else
	{
		shockwave->Reach -= TimeStep * 4096;
		if (shockwave->Reach < 0)
		{
			OBJ_FreeObject(obj);
			return;
		}

		if (obj->Light)
		{
			FTOL(shockwave->Reach / 8.0f, obj->Light->b);
		}
	}	

// set bounding box + add to mesh fx lists

	SetBBox(&shockwave->Box,
		obj->body.Centre.Pos.v[X] - shockwave->Reach,
		obj->body.Centre.Pos.v[X] + shockwave->Reach,
		obj->body.Centre.Pos.v[Y] - shockwave->Reach,
		obj->body.Centre.Pos.v[Y] + shockwave->Reach,
		obj->body.Centre.Pos.v[Z] - shockwave->Reach,
		obj->body.Centre.Pos.v[Z] + shockwave->Reach);

	AddWorldMeshFx(ShockwaveWorldMeshFxChecker, obj);
	AddModelMeshFx(ShockwaveModelMeshFxChecker, obj);
}

////////////////////
// weapon handler //
////////////////////

void FireworkHandler(OBJECT *obj)
{
	FIREWORK_OBJ *firework = (FIREWORK_OBJ *)obj->Data;

	// maintain sfx
	if (obj->Sfx3D)
	{
		CopyVec(&obj->body.Centre.Pos, &obj->Sfx3D->Pos);
	}

	// check age of firework
	firework->Age += TimeStep;

	// Move or set up explosion
	if (firework->Age < MAX_FIREWORK_AGE) {
		// Move firework
		FireWorkMove(obj);
	} else {
		if (!firework->Exploded) {
			// Set up explosions
			FireworkExplode(obj);
		} else {
			// update the lightsource
			if ((obj->Light != NULL) && (firework->Age < MAX_FIREWORK_AGE + 0.8)) {
				obj->Light->r = 128 + (long)(frand(32)) - (long)((128 * (firework->Age - MAX_FIREWORK_AGE)) / 0.8);
				obj->Light->g = 32 - (long)((32 * (firework->Age - MAX_FIREWORK_AGE)) / 0.8);
				obj->Light->b = 64 - (long)((64 * (firework->Age - MAX_FIREWORK_AGE)) / 0.8);
			} else {
				// Kill the firework
				OBJ_FreeObject(obj);
			}
		}
	}

}

void FireWorkMove(OBJECT *obj)
{
	REAL dRLen, impMod;
	VEC dR, imp, offset;
	// VEC angImp;
	FIREWORK_OBJ *firework = (FIREWORK_OBJ *)obj->Data;

	// Get the target relative position
	if (firework->Target != NULL) {
		VecMinusVec(&firework->Target->body.Centre.Pos, &obj->body.Centre.Pos, &dR);
		dRLen = VecLen(&dR);
		if (dRLen > SMALL_REAL) {
			VecDivScalar(&dR, dRLen / 2.5f);
		} else {
			firework->Age = MAX_FIREWORK_AGE;
			return;
		}
	} else {
		CopyVec(&DownVec, &dR);
		dRLen = WEAPON_RANGE_MAX;
	}

	// Accelerate firework towards target, or forwards if no target
	impMod = ONE;//0.2f + dRLen / WEAPON_RANGE_MAX;
	VecEqScalarVec(&imp, -0.5f * impMod * FLD_Gravity * TimeStep, &obj->body.Centre.WMatrix.mv[U]);
	ApplyParticleImpulse(&obj->body.Centre, &imp);

	VecEqScalarVec(&imp, -0.005f * FLD_Gravity * TimeStep, &dR);
	VecEqScalarVec(&offset, 50, &obj->body.Centre.WMatrix.mv[U]);
	ApplyBodyImpulse(&obj->body, &imp, &offset);

	//VecPlusEqScalarVec(&offset, 500, &obj->body.Centre.WMatrix.mv[R]);
	//VecEqScalarVec(&imp, -30 * TimeStep, &obj->body.Centre.WMatrix.mv[L]);
	//ApplyBodyImpulse(&obj->body, &imp, &offset);



	// Move the particle
	UpdateBody(&obj->body, TimeStep);

	/*firework->SmokeTime += TimeStep;
	if (firework->SmokeTime > 0.03) {
		CreateSpark(SPARK_SMOKE1, &obj->body.Centre.Pos, &ZeroVector, ZERO, 0);
		firework->SmokeTime = ZERO;
	}*/
	firework->SparkTime += TimeStep;
	if (firework->SparkTime > 0.005 && frand(ONE) < HALF) {
		CreateSpark(SPARK_SPARK, &obj->body.Centre.Pos, &ZeroVector, 80, 0);
		firework->SparkTime = ZERO;
	}

	// Update the trail
	if (firework->Trail != NULL) {
		firework->TrailTime += TimeStep;
		if (firework->TrailTime > firework->Trail->Data->LifeTime / firework->Trail->MaxTrails) {
			UpdateTrail(firework->Trail, &obj->body.Centre.Pos);
			firework->TrailTime = ZERO;
		} else {
			ModifyFirstTrail(firework->Trail, &obj->body.Centre.Pos);
		}

	}


}

void FireworkExplode(OBJECT *obj)
{
	int iFlash;
	VEC vel;
	FIREWORK_OBJ *firework = (FIREWORK_OBJ *)obj->Data;

	// Free up the models and sound effects
#ifdef _PC
	if (obj->Sfx3D)
	{
		FreeSfx3D(obj->Sfx3D);
	}
#endif
	obj->DefaultModel = -1;
	obj->renderhandler = NULL;

	// Get rid of the trail
	if (firework->Trail != NULL) {
		FreeTrail(firework->Trail);
	}

	// Oooaaahhhhh
	PlaySfx3D(SFX_FIREWORK_BANG, SFX_MAX_VOL, 22050, &obj->body.Centre.Pos);

	// Create Explosion
	CreateSpark(SPARK_EXPLOSION1, &obj->body.Centre.Pos, &ZeroVector, 0, 0);

	// Create smoke
	VecEqScalarVec(&vel, 60, &UpVec);
	for (iFlash = 0; iFlash < 3; iFlash++) {
		CreateSpark(SPARK_SMOKE2, &obj->body.Centre.Pos, &vel, 60, 0);
	}

	//Create flashy bits
	//VecPlusScalarVec(&obj->body.Centre.Vel, 300, &UpVec, &vel);
	VecEqScalarVec(&vel, 300, &UpVec);
	for (iFlash = 0; iFlash < 30; iFlash++) {
		CreateSpark(SPARK_SMALLORANGE, &obj->body.Centre.Pos, &vel, 600, 0);
		CreateSpark(SPARK_SMALLRED, &obj->body.Centre.Pos, &vel, 800, 0);
	}

	// setup light
	obj->Light = AllocLight();
	if (obj->Light)
	{
		obj->Light->Reach = 1024;
		obj->Light->Flag = LIGHT_FIXED;
		obj->Light->Type = LIGHT_OMNI;
		obj->Light->r = 128;
		obj->Light->g = 32;
		obj->Light->b = 64;
		obj->Light->x = obj->body.Centre.Pos.v[X];
		obj->Light->y = obj->body.Centre.Pos.v[Y];
		obj->Light->z = obj->body.Centre.Pos.v[Z];
	}

	firework->Exploded = TRUE;
}

////////////////////
// weapon handler //
////////////////////

void PuttyBombHandler(OBJECT *obj)
{
	PUTTYBOMB_OBJ *bomb = (PUTTYBOMB_OBJ*)obj->Data;
	long i, j, k, vcount, per, sides, flag;
	VEC vec, off, imp, *pos;
	MAT mat;
	REAL dx, dy, dz, dist, len, rot, mul, mass;
	BBOX box;
	CUBE_HEADER *cube;
	WORLD_POLY *wp;
	WORLD_VERTEX **wv;
	long *wrgb;
	MODEL_RGB *mrgb;
	NEWCOLLPOLY *p;
	COLLGRID *header;

// darken car

	obj->player->car.AddLit -= (long)(TimeStep * 2000);
	if (obj->player->car.AddLit < -1000) obj->player->car.AddLit = -1000;

// maintain sfx

	if (obj->Sfx3D)
	{
		CopyVec(&obj->player->car.Body->Centre.Pos, &obj->Sfx3D->Pos);
	}

// dec countdown, bang?

	bomb->Timer -= TimeStep;
	if (obj->player->car.WillDetonate == FALSE){

		if (bomb->Timer < ZERO) 
		{
			// Set bomb to explode
			obj->player->car.WillDetonate = TRUE;
			bomb->Timer = PUTTYBOMB_COUNTDOWN2;
			if (obj->Sfx3D)
			{
				FreeSfx3D(obj->Sfx3D);
				obj->Sfx3D = NULL;
			}
			flag = (long)obj->player;
	 		CreateObject(&obj->player->car.Body->Centre.Pos, &obj->player->car.Body->Centre.WMatrix, OBJECT_TYPE_BOMBGLOW, &flag);
		} 
		else 
		{
			// shrink the fuse
			obj->player->car.Aerial.Length = bomb->OrigAerialLen * bomb->Timer / PUTTYBOMB_COUNTDOWN;

			// create sparks at the end of fuse
			CreateSpark(SPARK_SPARK2, &obj->player->car.Aerial.Section[AERIAL_LASTSECTION].Pos, &obj->player->car.Body->Centre.Vel, 50, 0);
			CreateSpark(SPARK_SPARK2, &obj->player->car.Aerial.Section[AERIAL_LASTSECTION].Pos, &obj->player->car.Body->Centre.Vel, 50, 0);
		}
	} 
	else if (bomb->Timer <= 0.0f)
	{

// yep!

		CopyVec(&obj->player->car.Body->Centre.Pos, &bomb->Pos);

		bomb->Timer = 0.0f;
		bomb->SphereRadius = 80.0f;

		SubVector(&bomb->Pos, &CAM_MainCamera->WPos, &vec);
		CAM_MainCamera->Shake = 1.0f - (Length(&vec) / 2048.0f);
		if (CAM_MainCamera->Shake < 0.0f) CAM_MainCamera->Shake = 0.0f;

		obj->aihandler = (AI_HANDLER)PuttyBombBang;
		obj->renderhandler = (RENDER_HANDLER)RenderPuttyBombBang;

// play bang sfx

		PlaySfx3D(SFX_PUTTYBOMB_BANG, SFX_MAX_VOL, 22050, &bomb->Pos);

// light

		obj->Light = AllocLight();
		if (obj->Light)
		{
			CopyVec(&bomb->Pos, (VEC*)&obj->Light->x);
			obj->Light->Reach = 1024;
			obj->Light->Flag = LIGHT_FIXED | LIGHT_MOVING;
			obj->Light->Type = LIGHT_OMNI;
			obj->Light->r = 0;
			obj->Light->g = 0;
			obj->Light->b = 0;
		}

// setup bang pieces

		for (i = 0 ; i < PUTTYBOMB_BANG_NUM ; i++)
		{
			bomb->Bang[i].Age = -frand(PUTTYBOMB_BANG_STAGGER);
			bomb->Bang[i].Size = 64.0f - bomb->Bang[i].Age * 64.0f;
			bomb->Bang[i].Life = PUTTYBOMB_ONE_BANG_TIME - PUTTYBOMB_BANG_STAGGER - bomb->Bang[i].Age;
			SetVector(&bomb->Bang[i].Vel, 0.0f, -frand(64.0f) - 64.0f, 0.0f);

			SetVector(&vec, 0, 0, PUTTYBOMB_BANG_RADIUS * (-bomb->Bang[i].Age / PUTTYBOMB_BANG_STAGGER));
			RotMatrixZYX(&mat, frand(0.25f) - 0.25f, frand(1.0f), 0.0f);
			RotTransVector(&mat, &bomb->Pos, &vec, &bomb->Bang[i].Pos);
		}

// setup smoke verts

		bomb->SmokeTime = 0.0f;

		for (i = 0 ; i < PUTTYBOMB_SMOKE_NUM ; i++)
		{
			bomb->SmokeVert[i] = rand() % obj->player->car.Models->Body->VertNum;
		}

// scorch

		SetBBox(&box,
			bomb->Pos.v[X] - PUTTYBOMB_SCORCH_RADIUS,
			bomb->Pos.v[X] + PUTTYBOMB_SCORCH_RADIUS,
			bomb->Pos.v[Y] - PUTTYBOMB_SCORCH_RADIUS,
			bomb->Pos.v[Y] + PUTTYBOMB_SCORCH_RADIUS,
			bomb->Pos.v[Z] - PUTTYBOMB_SCORCH_RADIUS,
			bomb->Pos.v[Z] + PUTTYBOMB_SCORCH_RADIUS);

		cube = World.Cube;
		for (i = 0 ; i < World.CubeNum ; i++, cube++)
		{
			if (cube->Xmin > box.XMax || cube->Xmax < box.XMin || cube->Ymin > box.YMax || cube->Ymax < box.YMin || cube->Zmin > box.ZMax || cube->Zmax < box.ZMin) continue;

			dx = cube->CentreX - bomb->Pos.v[X];
			dy = cube->CentreY - bomb->Pos.v[Y];
			dz = cube->CentreZ - bomb->Pos.v[Z];
			if ((float)sqrt(dx * dx + dy * dy + dz * dz) > PUTTYBOMB_SCORCH_RADIUS + cube->Radius) continue;

			wp = cube->Model.PolyPtr;
			for (j = cube->Model.PolyNum ; j ; j--, wp++)
			{
				wv = &wp->v0;
				wrgb = &wp->rgb0;
				vcount = 3 + (wp->Type & 1);
				for (k = 0 ; k < vcount ; k++)
				{
					SubVector(&bomb->Pos, (VEC*)&wv[k]->x, &vec);
					len = Length(&vec);
					if (len < PUTTYBOMB_SCORCH_RADIUS)
					{
						FTOL((1.0f - len / PUTTYBOMB_SCORCH_RADIUS) * 512.0f, per);
						if (per > 256) per = 256;

						mrgb = (MODEL_RGB*)&wrgb[k];
						mrgb->r += (unsigned char)(((48 - mrgb->r) * per) >> 8);
						mrgb->g += (unsigned char)(((24 - mrgb->g) * per) >> 8);
						mrgb->b += (unsigned char)(((0 - mrgb->b) * per) >> 8);
						mrgb->a += (unsigned char)(((255 - mrgb->a) * per) >> 8);
					}
				}
			}
		}

// calc bang impulse

		SetVector(&imp, 0.0f, 0.0f, 0.0f);
		pos = &obj->player->car.Body->Centre.Pos;
		mass = obj->player->car.Body->Centre.Mass * 0.2f + 0.4f;

		header = PosToCollGrid(pos);
		if (header)
		{
			for (i = 0 ; i < header->NCollPolys ; i++)
			{
				p = header->CollPolyPtr[i];
				sides = IsPolyQuad(p) ? 4 : 3;

				for (j = 0 ; j < sides ; j++)
				{
					if (PlaneDist(&p->EdgePlane[j], pos) > 0.0f)
						break;
				}

				if (j == sides)
				{
					dist = PlaneDist(&p->Plane, pos);
					if (dist > 0.0f && dist < PUTTYBOMB_BANG_IMPULSE_RANGE)
					{
						mul = (PUTTYBOMB_BANG_IMPULSE_RANGE - dist) * 20.0f * mass;
						VecPlusEqScalarVec(&imp, mul, (VEC*)&p->Plane);
					}
				}
			}
		}
	
		rot = frand(RAD);
		mul = frand(20.0f);
		SetVector(&off, (float)sin(rot) * mul, 0.0f, (float)cos(rot) * mul);
		ApplyBodyImpulse(obj->player->car.Body, &imp, &off);

// give its aerial back

		obj->player->car.Aerial.Length = bomb->OrigAerialLen;
	}
}

////////////////////
// weapon handler //
////////////////////

void PuttyBombBang(OBJECT *obj)
{
	PUTTYBOMB_OBJ *bomb = (PUTTYBOMB_OBJ*)obj->Data;
	PUTTYBOMB_VERT *vert;
	MODEL *model = &LevelModel[obj->DefaultModel].Model;
	MODEL_VERTEX *mv;
	VEC vec;
	long i;

// darken car

	if (bomb->Timer < PUTTYBOMB_SPHERE_TIME)
	{
		obj->player->car.AddLit -= (long)(TimeStep * 2000);
		if (obj->player->car.AddLit < -1000) obj->player->car.AddLit = -1000;
	}

// maintain light?

	if (obj->Light)
	{
		FTOL((float)sin(bomb->Timer / PUTTYBOMB_BANG_TIME * (RAD / 2.0f)) * 240.0f + frand(15.0f), obj->Light->r);
		obj->Light->g = obj->Light->r >> 1;
	}

// maintain pieces

	for (i = 0 ; i < PUTTYBOMB_BANG_NUM ; i++)
	{
		if (bomb->Bang[i].Age < 0.0f)
		{
			VecPlusEqScalarVec(&bomb->Bang[i].Pos, TimeStep, &obj->player->car.Body->Centre.Vel);

			bomb->Bang[i].Vel.v[X] = obj->player->car.Body->Centre.Vel.v[X];
			bomb->Bang[i].Vel.v[Z] = obj->player->car.Body->Centre.Vel.v[Z];
		}
		else
		{
			VecPlusEqScalarVec(&bomb->Bang[i].Pos, TimeStep, &bomb->Bang[i].Vel);

			bomb->Bang[i].Vel.v[X] *= (1.0f - 1.5f * TimeStep);
			bomb->Bang[i].Vel.v[Z] *= (1.0f - 1.5f * TimeStep);
		}

		bomb->Bang[i].Age += TimeStep;
	}

// release smoke

	if (bomb->Timer > PUTTYBOMB_SPHERE_TIME)
	{
		bomb->SmokeTime += TimeStep;
		if (bomb->SmokeTime >= 0.1f)
		{
			for (i = 0 ; i < PUTTYBOMB_SMOKE_NUM ; i++)
			{
				RotTransVector(&obj->player->car.Body->Centre.WMatrix, &obj->player->car.BodyWorldPos, (VEC*)&obj->player->car.Models->Body->VertPtr[bomb->SmokeVert[i]].x, &vec);
				CreateSpark(SPARK_SMOKE3, &vec, &BombSmokeVel, 16.0f, 0);
			}
			bomb->SmokeTime -= 0.1f;
		}
	}

// set vert UV's

	vert = (PUTTYBOMB_VERT*)(bomb + 1);
	mv = model->VertPtr;

	for (i = model->VertNum ; i ; i--, mv++, vert++)
	{
		vert->Time += vert->TimeAdd * TimeStep;

		mv->tu = (float)sin(vert->Time) * (12.0f / 256.0f) + (36.0f / 256.0f);
		mv->tv = (float)cos(vert->Time) * (12.0f / 256.0f) + (68.0f / 256.0f);
	}

// set sphere size

	if (bomb->SphereRadius < 512.0f)
	bomb->SphereRadius += (512.0f - bomb->SphereRadius) * 0.07f * TimeFactor;

// inc timer, kill?

	bomb->Timer += TimeStep;
	if (bomb->Timer >= PUTTYBOMB_BANG_TIME)
	{
		obj->player->car.IsBomb = FALSE;
		OBJ_FreeObject(obj);
		return;
	}

// set bounding box + add to mesh fx lists

	if (bomb->Timer < PUTTYBOMB_SPHERE_TIME)
	{
		SetBBox(&bomb->Box,
			bomb->Pos.v[X] - bomb->SphereRadius - PUTTYBOMB_PUSH_RANGE,
			bomb->Pos.v[X] + bomb->SphereRadius + PUTTYBOMB_PUSH_RANGE,
			bomb->Pos.v[Y] - bomb->SphereRadius - PUTTYBOMB_PUSH_RANGE,
			bomb->Pos.v[Y] + bomb->SphereRadius + PUTTYBOMB_PUSH_RANGE,
			bomb->Pos.v[Z] - bomb->SphereRadius - PUTTYBOMB_PUSH_RANGE,
			bomb->Pos.v[Z] + bomb->SphereRadius + PUTTYBOMB_PUSH_RANGE);

		AddWorldMeshFx(PuttyBombWorldMeshFxChecker, obj);
		AddModelMeshFx(PuttyBombModelMeshFxChecker, obj);
	}
}

void PuttyBombMove(OBJECT *obj) 
{
	PLAYER *player;
	PUTTYBOMB_OBJ *bomb = (PUTTYBOMB_OBJ*)obj->Data;

	// Make sure it isn't too late
	if (obj->player->car.WillDetonate) return;

	// Loop over all other players
	for (player = PLR_PlayerHead; player != NULL; player = player ->next) {
		if (player == obj->player) continue;

		// Have the players collided?
		if (!IsPairCollided(obj->player->ownobj, player->ownobj)) continue;

		// Can the bomb be tranferred?
		if (player->car.NoReturnTimer > ZERO) continue;

		// Transfer the bomb
		obj->player->car.NoReturnTimer = PUTTYBOMB_NORETURN_TIME;
		obj->player->car.Aerial.Length = bomb->OrigAerialLen;
		obj->player->car.IsBomb = FALSE;
		obj->player->car.WillDetonate = FALSE;

		player->car.IsBomb = TRUE;
		player->car.WillDetonate = FALSE;
		bomb->OrigAerialLen = player->car.Aerial.Length;
		obj->player->car.NoReturnTimer = PUTTYBOMB_NORETURN_TIME;
		obj->player = player;

	}
}


////////////////////
// weapon handler //
////////////////////

void WaterBombHandler(OBJECT *obj)
{
	long flag;
	WATERBOMB_OBJ *bomb = (WATERBOMB_OBJ*)obj->Data;
	MAT mat;
	VEC pos;
	REAL dist;

// inc age

	bomb->Age += TimeStep;

// maintain sfx

	if (obj->Sfx3D)
	{
		CopyVec(&obj->body.Centre.Pos, &obj->Sfx3D->Pos);
	}

// set wobble scalars

	bomb->ScalarHoriz = (float)sin((float)TIME2MS(TimerCurrent) / 100.0f) / 10.0f + 1.0f;
	bomb->ScalarVert = 2.0f - bomb->ScalarHoriz;

// sub hit mag

	if (obj->body.BangMag)
	{
		bomb->BangTol -= obj->body.BangMag;
		if (obj->body.BangPlane.v[Y] > -0.7f && bomb->BangTol < 0.0f)
			bomb->BangTol = 0.0f;

		obj->body.BangMag = 0.0f;
	}

// bang?

	if (bomb->BangTol < 0.0f || bomb->Age > WATERBOMB_MAX_AGE)
	{
		PlaySfx3D(SFX_WATERBOMB_HIT, SFX_MAX_VOL, 22050, &obj->body.Centre.Pos);

//		flag = (long)&LevelModel[obj->DefaultModel].Model;
//		CreateObject(&obj->body.Centre.Pos, &obj->body.Centre.WMatrix, OBJECT_TYPE_DISSOLVEMODEL, &flag);

		dist = -PlaneDist(&obj->body.BangPlane, &obj->body.Centre.Pos);
		VecPlusScalarVec(&obj->body.Centre.Pos, dist, (VEC*)&obj->body.BangPlane, &pos);

		SetVec(&mat.mv[U], -obj->body.BangPlane.v[A], -obj->body.BangPlane.v[B], -obj->body.BangPlane.v[C]);
		SetVec(&mat.mv[L], mat.m[UY], mat.m[UZ], mat.m[UX]);
		CrossProduct(&mat.mv[U], &mat.mv[L], &mat.mv[R]);
		CrossProduct(&mat.mv[R], &mat.mv[U], &mat.mv[L]);

		CreateObject(&pos, &mat, OBJECT_TYPE_SPLASH, &flag);

		OBJ_FreeObject(obj);
	}
}

////////////////////
// weapon handler //
////////////////////

void ElectroPulseHandler(OBJECT *obj)
{
	long i, rgb, lmul, flag;
	ELECTROPULSE_OBJ *electro = (ELECTROPULSE_OBJ*)obj->Data;
	MODEL *dmodel, *model = (MODEL*)&electro->Model;
	MODEL_POLY *mp;
	POLY_RGB *mrgb;
	MODEL_VERTEX *mv;
	ELECTROPULSE_VERT *evert;
	PLAYER *player;
	VEC delta, vec, *nvec1, *nvec2, *pos, rotpos, v1, v2;
	REAL dist, ndist;

// set vert UV's

	mv = model->VertPtr;
	evert = (ELECTROPULSE_VERT*)(model->VertPtr + model->VertNum);

	for (i = model->VertNum ; i ; i--, mv++, evert++)
	{
		evert->Time += evert->TimeAdd * TimeStep;

		mv->tu = (float)sin(evert->Time) * (24.0f / 256.0f) + (96.0f / 256.0f);
		mv->tv = (float)cos(evert->Time) * (24.0f / 256.0f) + (96.0f / 256.0f);
	}

// copy vert UV' to poly UV's + set rgb's

	if (electro->Age < 0.5f)
	{
		FTOL(electro->Age * 511.0f, lmul);
	}
	else if (electro->Age > 9.5f)
	{
		FTOL((10.0f - electro->Age) * 511.0f, lmul);
	}
	else
	{
		lmul = 255;
	}

	rgb = lmul | lmul << 8 | lmul << 16;

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

		if (mp->Type & POLY_QUAD)
		{
			mp->tu3 = mp->v3->tu;
			mp->tv3 = mp->v3->tv;
		}	

		*(long*)&mrgb->rgb[0] = rgb;
		*(long*)&mrgb->rgb[1] = rgb;
		*(long*)&mrgb->rgb[2] = rgb;
		*(long*)&mrgb->rgb[3] = rgb;
	}

// maintain light

	if (obj->Light)
	{
		CopyVec(&obj->player->car.Body->Centre.Pos, (VEC*)&obj->Light->x);

		obj->Light->b = lmul / 4 + (rand() & 31);
	}

// maintain sfx

	if (obj->Sfx3D)
	{
		CopyVec(&obj->player->car.Body->Centre.Pos, &obj->Sfx3D->Pos);

		obj->Sfx3D->Vol = SFX_MAX_VOL * lmul / 255;
	}

// kill?

	electro->Age += TimeStep;
	if (electro->Age > 10.0f)
	{
		OBJ_FreeObject(obj);
		return;
	}

// check against other cars

	electro->JumpFlag = 0;

	pos = &obj->player->car.BodyWorldPos;

	for (player = PLR_PlayerHead ; player ; player = player->next)
	{
		 if (player == obj->player)
			continue;

		SubVector(pos, &player->car.BodyWorldPos, &delta);
		dist = delta.v[X] * delta.v[X] + delta.v[Y] * delta.v[Y] + delta.v[Z] * delta.v[Z];
		if (dist > (ELECTRO_RANGE * ELECTRO_RANGE))
			continue;

// car in range

		if (!player->car.PowerTimer)
		{
			flag = (long)player;
	 		CreateObject(&player->car.Body->Centre.Pos, &player->car.Body->Centre.WMatrix, OBJECT_TYPE_ELECTROZAPPED, &flag);
		}

		player->car.PowerTimer = ELECTRO_KILL_TIME;

// find his nearest vert to me

		SubVector(pos, &player->car.BodyWorldPos, &delta);
		TransposeRotVector(&player->car.Body->Centre.WMatrix, &delta, &rotpos);

		dmodel = &player->car.Models->Body[0];

		ndist = 1000000;
		mv = dmodel->VertPtr;

		for (i = dmodel->VertNum ; i ; i--, mv++)
		{
			SubVector((VEC*)&mv->x, &rotpos, &vec);
			dist = vec.v[X] * vec.v[X] + vec.v[Y] * vec.v[Y] + vec.v[Z] * vec.v[Z];
			if (dist < ndist)
			{
				ndist = dist;
				nvec1 = (VEC*)&mv->x;
			}
		}

// find my nearest vert to him

		SubVector(&player->car.BodyWorldPos, pos, &delta);
		TransposeRotVector(&obj->player->car.Body->Centre.WMatrix, &delta, &rotpos);

		ndist = 1000000;
		mv = model->VertPtr;

		for (i = model->VertNum ; i ; i--, mv++)
		{
			SubVector((VEC*)&mv->x, &rotpos, &vec);
			dist = vec.v[X] * vec.v[X] + vec.v[Y] * vec.v[Y] + vec.v[Z] * vec.v[Z];
			if (dist < ndist)
			{
				ndist = dist;
				nvec2 = (VEC*)&mv->x;
			}
		}

// save player + verts

		electro->Player[electro->JumpFlag] = player;

		CopyVec(nvec1, &electro->JumpPos1[electro->JumpFlag]);
		CopyVec(nvec2, &electro->JumpPos2[electro->JumpFlag]);

// create sparks

		RotTransVector(&player->car.Body->Centre.WMatrix, &player->car.BodyWorldPos, &electro->JumpPos1[electro->JumpFlag], &v1);
		RotTransVector(&obj->player->car.Body->Centre.WMatrix, &obj->player->car.BodyWorldPos, &electro->JumpPos2[electro->JumpFlag], &v2);

		FTOL(TimeStep * 50.0f + 1.0f, i);
		for ( ; i ; i--)
		{
			CreateSpark(SPARK_ELECTRIC, &v1, &ZeroVector, 200, 0);
			CreateSpark(SPARK_ELECTRIC, &v2, &ZeroVector, 200, 0);
		}

// inc jump flag
	
		electro->JumpFlag++;
	}
}
#endif

////////////////////
// weapon handler //
////////////////////

void OilSlickHandler(OBJECT *obj)
{
	long i;
	OILSLICK_OBJ *oil = (OILSLICK_OBJ*)obj->Data;
	VEC vec, pos, vel, v0, v1, v2, v3;
	MAT mat;
	REAL mul, time;
	PLANE *plane;

// falling

	if (!oil->Mode)
	{

// update time

		oil->Age += TimeStep;

// add vel

		obj->body.Centre.Vel.v[Y] += OILSLICK_GRAV * TimeStep;
		obj->body.Centre.Pos.v[Y] += obj->body.Centre.Vel.v[Y] * TimeStep;
		if (obj->body.Centre.Pos.v[Y] >= oil->LandHeight)
		{
			oil->Mode = 1;
			oil->Age = 0.0f;
			oil->Size = OILSLICK_MIN_SIZE;

			RotMatrixY(&mat, frand(1.0f));

			SetVector(&v0, -1.0f, 0, 1.0f);
			SetVector(&v1, 1.0f, 0, 1.0f);
			SetVector(&v2, 1.0f, 0, -1.0f);
			SetVector(&v3, -1.0f, 0, -1.0f);

			RotVector(&mat, &v0, &oil->Vel[0]);
			RotVector(&mat, &v1, &oil->Vel[1]);
			RotVector(&mat, &v2, &oil->Vel[2]);
			RotVector(&mat, &v3, &oil->Vel[3]);

			SetVector(&pos, obj->body.Centre.Pos.v[X], oil->LandHeight - 32.0f, obj->body.Centre.Pos.v[Z]);

			SetVector(&v0, -OILSLICK_MIN_SIZE, 0, OILSLICK_MIN_SIZE);
			SetVector(&v1, OILSLICK_MIN_SIZE, 0, OILSLICK_MIN_SIZE);
			SetVector(&v2, OILSLICK_MIN_SIZE, 0, -OILSLICK_MIN_SIZE);
			SetVector(&v3, -OILSLICK_MIN_SIZE, 0, -OILSLICK_MIN_SIZE);

			RotTransVector(&mat, &pos, &v0, &oil->Pos[0]);
			RotTransVector(&mat, &pos, &v1, &oil->Pos[1]);
			RotTransVector(&mat, &pos, &v2, &oil->Pos[2]);
			RotTransVector(&mat, &pos, &v3, &oil->Pos[3]);
		}
	}

// on floor

	if (oil->Mode)
	{

// expand

		mul = (oil->MaxSize - oil->Size) * TimeStep;
		oil->Size += mul;

		for (i = 0 ; i < 4 ; i++)
		{
			CopyVec(&oil->Vel[i], &vel);

			SetVector(&vec, oil->Pos[i].v[X], oil->Pos[i].v[Y] + 1024, oil->Pos[i].v[Z]);
			LineOfSightDist(&oil->Pos[i], &vec, &time, &plane);
			if (time > 0.0f && time < 1.0f)
			{
				vel.v[X] += plane->v[X] * 2.0f;
				vel.v[Z] += plane->v[Z] * 2.0f;
			}

			VecPlusEqScalarVec(&oil->Pos[i], mul, &vel);
		}

// add to oilslick list

		if (OilSlickCount < OILSLICK_LIST_MAX)
		{
			OilSlickList[OilSlickCount].X = obj->body.Centre.Pos.v[X];
			OilSlickList[OilSlickCount].Z = obj->body.Centre.Pos.v[Z];
			OilSlickList[OilSlickCount].Radius = oil->Size;
			OilSlickList[OilSlickCount].SquaredRadius = oil->Size * oil->Size;

			OilSlickList[OilSlickCount].Ymin = oil->Ymin;
			OilSlickList[OilSlickCount].Ymax = oil->Ymax;

			OilSlickCount++;
		}

// kill?

		oil->Age += TimeStep;
		if (oil->Age > 30.0f)
		{
			OBJ_FreeObject(obj);
		}
	}
}

////////////////////
// weapon handler //
////////////////////

void OilSlickDropperHandler(OBJECT *obj)
{
	OILSLICK_DROPPER_OBJ *dropper = (OILSLICK_DROPPER_OBJ*)obj->Data;
	long flags[2];
	REAL len, mul;
	VEC delta;
	CAR *car;

// create new oil slick?

	car = &obj->player->car;

	if (!dropper->Count)
	{
		len = DROPPER_GAP;
	}
	else
	{
		SubVector(&car->Body->Centre.Pos, &dropper->LastPos, &delta);
		len = Length(&delta);
	}

	if (len >= DROPPER_GAP)
	{

// yep

		mul = DROPPER_GAP / len;
		VecPlusScalarVec(&dropper->LastPos, mul, &delta, &dropper->LastPos);

		flags[0] = (long)obj->player;
		FTOL(OILSLICK_MAX_SIZE - (float)dropper->Count * 16.0f, flags[1]);

		CreateObject(&dropper->LastPos, &car->Body->Centre.WMatrix, OBJECT_TYPE_OILSLICK, flags);

		dropper->Count++;
	}


// kill dropper?

	dropper->Age += TimeStep;
	if (dropper->Age > 1.0f || dropper->Count >= 3)
	{
		OBJ_FreeObject(obj);
	}
}

#ifdef _PC
////////////////////
// weapon handler //
////////////////////

void ChromeBallHandler(OBJECT *obj)
{
	long vel;
	CHROMEBALL_OBJ *ball = (CHROMEBALL_OBJ*)obj->Data;

// inc age

	ball->Age += TimeStep;

// set radius

	if (ball->Age < 0.5f)
		ball->Radius = (CHROMEBALL_MAX_RAD - CHROMEBALL_MIN_RAD) * ball->Age * 2.0f + CHROMEBALL_MIN_RAD;
	else if (ball->Age > 29.0f)
		ball->Radius = (CHROMEBALL_MAX_RAD - CHROMEBALL_MIN_RAD) * (30.0f - ball->Age) + CHROMEBALL_MIN_RAD;
	else
		ball->Radius = CHROMEBALL_MAX_RAD;

	obj->body.CollSkin.Sphere[0].Radius = obj->body.CollSkin.WorldSphere[0].Radius = obj->body.CollSkin.OldWorldSphere[0].Radius = ball->Radius;

// maintain sfx

	if (obj->Sfx3D)
	{
		FTOL(Length(&obj->body.Centre.Vel), vel);

		obj->Sfx3D->Freq = 5000 + vel * 4;

		obj->Sfx3D->Vol = vel / 2;
		if (obj->Sfx3D->Vol > SFX_MAX_VOL) obj->Sfx3D->Vol = SFX_MAX_VOL;

		CopyVec(&obj->body.Centre.Pos, &obj->Sfx3D->Pos);
	}

	// Reset knocks
	if (obj->body.BangMag > TO_VEL(Real(300))) {
		long vol, freq;

		//vol = SFX_MIN_VOL + (SFX_MAX_VOL - SFX_MIN_VOL) * obj->body.BangMag / 1000;
		//if (vol > SFX_MAX_VOL) vol = SFX_MAX_VOL;
		vol = SFX_MAX_VOL;

		freq = 22050;

		PlaySfx3D(SFX_CHROMEBALL_HIT, vol, freq, &obj->body.Centre.Pos);
		obj->body.Banged = FALSE;
		obj->body.BangMag = ZERO;
	}

// kill?

	if (ball->Age >= 30.0f)
	{
		OBJ_FreeObject(obj);
	}
}

////////////////////
// weapon handler //
////////////////////

void CloneHandler(OBJECT *obj)
{
}

////////////////////
// weapon handler //
////////////////////

void TurboAIHandler(OBJECT *obj)
{
	//int iTrail;
	VEC pos, vel;
	TURBO_OBJ *turbo = (TURBO_OBJ *)obj->Data;

	turbo->Age += TimeStep;
	turbo->SparkTime += TimeStep;

	if (turbo->SparkTime > 0.01) {
		VecEqScalarVec(&vel, HALF, &obj->player->car.Body->Centre.Vel);
		pos.v[X] = obj->player->car.Body->Centre.Pos.v[X] + frand(30) - 15;
		pos.v[Y] = obj->player->car.Body->Centre.Pos.v[Y] + frand(30) - 15;
		pos.v[Z] = obj->player->car.Body->Centre.Pos.v[Z] + frand(30) - 15;
		
		CreateSpark(SPARK_SPARK, &pos, &vel, TO_VEL(Real(100)), 0);
	}

	if (turbo->Age < turbo->LifeTime) {
		//obj->player->car.Body->Centre.Boost = turbo->Force * (ONE - turbo->Age / turbo->LifeTime);
		obj->player->car.TopSpeed = TO_VEL(MPH2OGU_SPEED * 75) * (ONE + HALF * (ONE - turbo->Age / turbo->LifeTime));
	} else {
		obj->player->car.TopSpeed = obj->player->car.DefaultTopSpeed;
		//obj->player->car.Body->Centre.Boost = ZERO;
		/*for (iTrail = 0; iTrail < TURBO_NTRAILS; iTrail++) {
			if (turbo->TurboTrail[iTrail] != NULL) {
				FreeTrail(turbo->TurboTrail[iTrail]);
			}
		}*/
		OBJ_FreeObject(obj);
	}

}

void TurboMoveHandler(OBJECT *obj)
{
	//int iTrail;
	TURBO_OBJ *turbo = (TURBO_OBJ *)obj->Data;

	// Update the trail
	/*for (iTrail = 0; iTrail < TURBO_NTRAILS; iTrail++) {
		if (turbo->TurboTrail[iTrail] != NULL) {
			turbo->TrailTime += TimeStep;
			if (turbo->TrailTime > turbo->TurboTrail[iTrail]->Data->LifeTime / turbo->TurboTrail[iTrail]->MaxTrails) {
				UpdateTrail(turbo->TurboTrail[iTrail], &obj->player->car.Wheel[iTrail].WPos);
				turbo->TrailTime = ZERO;
			} else {
				ModifyFirstTrail(turbo->TurboTrail[iTrail], &obj->player->car.Wheel[iTrail].WPos);
			}

		}
	}*/
}

////////////////////
// weapon handler //
////////////////////

void Turbo2Handler(OBJECT *obj)
{
	TURBO2_OBJ *turbo = (TURBO2_OBJ*)obj->Data;
/*	long i, count;
	REAL mul;
	VEC vec, vec2, vec3, vec4;

// create particles

	FTOL(TimeStep * 100.0f, count);
	CopyVec(&obj->player->car.Body->Centre.Pos, &vec3);
	VecEqScalarVec(&vec4, 1.0f / 100.0f, &obj->player->car.Body->Centre.Vel);

	for (i = 0 ; i < count ; i++)
	{
		SetVector(&vec, frand(1.0f) - 0.5f, frand(1.0f) - 0.5f, 0.0f);
		mul = 64.0f / Length(&vec);
		VecMulScalar(&vec, mul);
		RotVector(&obj->player->car.Body->Centre.WMatrix, &vec, &vec2);
		VecPlusScalarVec(&vec3, 0.1f, &vec2, &vec);
		CreateSpark(SPARK_STAR, &vec, &vec2, 0.0f, 0);

		AddVector(&vec3, &vec4, &vec3);
	}*/

// age

	turbo->Age += TimeStep;
	if (turbo->Age < turbo->LifeTime)
	{
		obj->player->car.TopSpeed = TO_VEL(MPH2OGU_SPEED * 75) * (ONE + HALF * (ONE - turbo->Age / turbo->LifeTime));
	}
	else
	{
		OBJ_FreeObject(obj);
	}
}

////////////////////
// weapon handler //
////////////////////

void SpringHandler(OBJECT *obj)
{
	VEC imp;

	ScalarVecPlusScalarVec(-obj->player->car.Body->Centre.Mass * 1000, &obj->player->car.Body->Centre.WMatrix.mv[U], obj->player->car.Body->Centre.Mass * 1000, &obj->player->car.Body->Centre.WMatrix.mv[L], &imp)
	VecPlusEqVec(&obj->player->car.Body->Centre.Impulse, &imp);

	OBJ_FreeObject(obj);
}

////////////////////
// weapon handler //
////////////////////

void ElectroZappedHandler(OBJECT *obj)
{
	long i, rgb, lmul;
	ELECTROZAPPED_OBJ *electro = (ELECTROZAPPED_OBJ*)obj->Data;
	MODEL *model = (MODEL*)&electro->Model;
	MODEL_POLY *mp;
	POLY_RGB *mrgb;
	MODEL_VERTEX *mv;
	ELECTROZAPPED_VERT *evert;

// set vert UV's

	mv = model->VertPtr;
	evert = (ELECTROZAPPED_VERT*)(model->VertPtr + model->VertNum);

	for (i = model->VertNum ; i ; i--, mv++, evert++)
	{
		evert->Time += evert->TimeAdd * TimeStep;

		mv->tu = (float)sin(evert->Time) * (24.0f / 256.0f) + (96.0f / 256.0f);
		mv->tv = (float)cos(evert->Time) * (24.0f / 256.0f) + (96.0f / 256.0f);
	}

// copy vert UV' to poly UV's + set rgb's

	FTOL(obj->player->car.PowerTimer * 80.0f, lmul);

	rgb = lmul | lmul << 8 | lmul << 16;

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

		if (mp->Type & POLY_QUAD)
		{
			mp->tu3 = mp->v3->tu;
			mp->tv3 = mp->v3->tv;
		}	

		*(long*)&mrgb->rgb[0] = rgb;
		*(long*)&mrgb->rgb[1] = rgb;
		*(long*)&mrgb->rgb[2] = rgb;
		*(long*)&mrgb->rgb[3] = rgb;
	}

// kill?

	if (!obj->player->car.PowerTimer)
	{
		OBJ_FreeObject(obj);
		return;
	}
}

////////////////////
// weapon handler //
////////////////////

void BombGlowHandler(OBJECT *obj)
{
	long i, rgb, lmul;
	BOMBGLOW_OBJ *glow = (BOMBGLOW_OBJ*)obj->Data;
	MODEL *model = (MODEL*)&glow->Model;
	MODEL_POLY *mp;
	POLY_RGB *mrgb;
	MODEL_VERTEX *mv;
	BOMBGLOW_VERT *gvert;

// set vert UV's

	mv = model->VertPtr;
	gvert = (BOMBGLOW_VERT*)(model->VertPtr + model->VertNum);

	for (i = model->VertNum ; i ; i--, mv++, gvert++)
	{
		gvert->Time += gvert->TimeAdd * TimeStep;

		mv->tu = (float)sin(gvert->Time) * (12.0f / 256.0f) + (36.0f / 256.0f);
		mv->tv = (float)cos(gvert->Time) * (12.0f / 256.0f) + (44.0f / 256.0f);
	}

// copy vert UV' to poly UV's + set rgb's

	FTOL(glow->Timer / PUTTYBOMB_COUNTDOWN2 * 128.0f, lmul);
	rgb = lmul | lmul << 8 | lmul << 16;

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

		if (mp->Type & POLY_QUAD)
		{
			mp->tu3 = mp->v3->tu;
			mp->tv3 = mp->v3->tv;
		}	

		*(long*)&mrgb->rgb[0] = rgb;
		*(long*)&mrgb->rgb[1] = rgb;
		*(long*)&mrgb->rgb[2] = rgb;
		*(long*)&mrgb->rgb[3] = rgb;
	}

// kill?

	glow->Timer += TimeStep;
	if (glow->Timer >= PUTTYBOMB_COUNTDOWN2)
	{
		OBJ_FreeObject(obj);
		return;
	}
}

//////////////////////
// render shockwave //
//////////////////////

void RenderShockwave(OBJECT *obj)
{
	SHOCKWAVE_OBJ *shockwave = (SHOCKWAVE_OBJ*)obj->Data;
	FACING_POLY poly;
	MAT mat;
	REAL ang;

// render head

	if (shockwave->Alive)
	{
		poly.Xsize = poly.Ysize = 40.0f;
		poly.Tpage = TPAGE_FX1;
		poly.U = 0.0f / 256.0f;
		poly.V = 64.0f / 256.0f;
		poly.Usize = poly.Vsize = 64.0f / 256.0f;
		poly.RGB = 0xffffff;

		ang = (float)TIME2MS(CurrentTimer()) / 2000.0f;

		RotMatrixZ(&mat, ang);
		DrawFacingPolyRotMirror(&obj->body.Centre.Pos, &mat, &poly, 1, -128);

		RotMatrixZ(&mat, -ang);
		DrawFacingPolyRotMirror(&obj->body.Centre.Pos, &mat, &poly, 1, -128);
	}
}

///////////////////////////////
// shockwave mesh fx checker //
///////////////////////////////

void ShockwaveWorldMeshFxChecker(void *data)
{
	long i, j;
	CUBE_HEADER **cubelist;
	OBJECT *obj = (OBJECT*)data;
	SHOCKWAVE_OBJ *shockwave = (SHOCKWAVE_OBJ*)obj->Data;
	VEC vec, *pos = &obj->body.Centre.Pos;
	WORLD_VERTEX *wv;
	WORLD_MIRROR_VERTEX *wmv;
	REAL dist, mul, pull;

// loop thru world cubes

	cubelist = World.CubeList;

	for (i = 0 ; i < WorldCubeCount ; i++)
	{

// check bounding box

		if (cubelist[i]->Xmin > shockwave->Box.XMax || cubelist[i]->Xmax < shockwave->Box.XMin ||
			cubelist[i]->Ymin > shockwave->Box.YMax || cubelist[i]->Ymax < shockwave->Box.YMin ||
			cubelist[i]->Zmin > shockwave->Box.ZMax || cubelist[i]->Zmax < shockwave->Box.ZMin)
				continue;

// check spheres

		SubVector((VEC*)&cubelist[i]->CentreX, pos, &vec);
		if (Length(&vec) > cubelist[i]->Radius + shockwave->Reach)
			continue;

// ok, set verts

		if (cubelist[i]->MeshFxFlag & MESHFX_USENEWVERTS)
		{
			wv = cubelist[i]->Model.VertPtr;
			for (j = cubelist[i]->Model.VertNum ; j ; j--, wv++)
			{
				SubVector((VEC*)&wv->x2, pos, &vec);
				dist = Length(&vec);
				if (dist < shockwave->Reach)
				{
					pull = (shockwave->Reach - dist) * SHOCKWAVE_PULL_MAX_MUL;
					if (pull > dist * SHOCKWAVE_PULL_MIN_MUL) pull = dist * SHOCKWAVE_PULL_MIN_MUL;
					mul = pull / dist;
					VecMinusScalarVec((VEC*)&wv->x2, mul, &vec, (VEC*)&wv->x2);
				}
			}

			wmv = cubelist[i]->Model.MirrorVertPtr;
			for (j = cubelist[i]->Model.MirrorVertNum ; j ; j--, wmv++)
			{
				SubVector((VEC*)&wmv->RealVertex->x2, pos, &vec);
				dist = Length(&vec);
				if (dist < shockwave->Reach)
				{
					pull = (shockwave->Reach - dist) * SHOCKWAVE_PULL_MAX_MUL;
					if (pull > dist * SHOCKWAVE_PULL_MIN_MUL) pull = dist * SHOCKWAVE_PULL_MIN_MUL;
					mul = pull / dist;
					VecMinusScalarVec((VEC*)&wmv->x2, mul, &vec, (VEC*)&wmv->x2);
				}
			}
		}
		else
		{
			wv = cubelist[i]->Model.VertPtr;
			for (j = cubelist[i]->Model.VertNum ; j ; j--, wv++)
			{
				SubVector((VEC*)&wv->x, pos, &vec);
				dist = Length(&vec);
				if (dist < shockwave->Reach)
				{
					pull = (shockwave->Reach - dist) * SHOCKWAVE_PULL_MAX_MUL;
					if (pull > dist * SHOCKWAVE_PULL_MIN_MUL) pull = dist * SHOCKWAVE_PULL_MIN_MUL;
					mul = pull / dist;
					VecMinusScalarVec((VEC*)&wv->x, mul, &vec, (VEC*)&wv->x2);
				}
				else
				{
					CopyVec((VEC*)&wv->x, (VEC*)&wv->x2);
				}
			}

			wmv = cubelist[i]->Model.MirrorVertPtr;
			for (j = cubelist[i]->Model.MirrorVertNum ; j ; j--, wmv++)
			{
				SubVector((VEC*)&wmv->RealVertex->x, pos, &vec);
				dist = Length(&vec);
				if (dist < shockwave->Reach)
				{
					pull = (shockwave->Reach - dist) * SHOCKWAVE_PULL_MAX_MUL;
					if (pull > dist * SHOCKWAVE_PULL_MIN_MUL) pull = dist * SHOCKWAVE_PULL_MIN_MUL;
					mul = pull / dist;
					VecMinusScalarVec((VEC*)&wmv->x, mul, &vec, (VEC*)&wmv->x2);
				}
				else
				{
					CopyVec((VEC*)&wmv->x, (VEC*)&wmv->x2);
				}
			}
		}

// set mesh flag

		cubelist[i]->MeshFxFlag |= MESHFX_USENEWVERTS;
	}
}

///////////////////////////////
// shockwave mesh fx checker //
///////////////////////////////

void ShockwaveModelMeshFxChecker(void *data)
{
	long j;
	REAL pull, dist, mul, rad = ModelMeshModel->Radius;
	OBJECT *obj = (OBJECT*)data;
	SHOCKWAVE_OBJ *shockwave = (SHOCKWAVE_OBJ*)obj->Data;
	VEC delta, newdelta, vec;
	MODEL_VERTEX *mv;

// quick radius bounding box test

	if (ModelMeshPos->v[X] + rad < shockwave->Box.XMin ||
		ModelMeshPos->v[X] - rad > shockwave->Box.XMax ||
		ModelMeshPos->v[Y] + rad < shockwave->Box.YMin ||
		ModelMeshPos->v[Y] - rad > shockwave->Box.YMax ||
		ModelMeshPos->v[Z] + rad < shockwave->Box.ZMin ||
		ModelMeshPos->v[Z] - rad > shockwave->Box.ZMax)
			return;

// get delta vector

	SubVector(&obj->body.Centre.Pos, ModelMeshPos, &delta);

// sphere test

	dist = Length(&delta);
	if (dist > rad + shockwave->Reach)
		return;

// put delta vector into model space

	TransposeRotVector(ModelMeshMat, &delta, &newdelta);

// proper bounding box test

	if (ModelMeshModel->Xmax < newdelta.v[X] - shockwave->Reach ||
		ModelMeshModel->Xmin > newdelta.v[X] + shockwave->Reach ||
		ModelMeshModel->Ymax < newdelta.v[Y] - shockwave->Reach ||
		ModelMeshModel->Ymin > newdelta.v[Y] + shockwave->Reach ||
		ModelMeshModel->Zmax < newdelta.v[Z] - shockwave->Reach ||
		ModelMeshModel->Zmin > newdelta.v[Z] + shockwave->Reach)
			return;

// ok, set verts

		mv = ModelMeshModel->VertPtr;

		if (*ModelMeshFlag & MODEL_USENEWVERTS)
		{
			for (j = ModelMeshModel->VertNum ; j ; j--, mv++)
			{
				SubVector((VEC*)&mv->x2, &newdelta, &vec);
				dist = Length(&vec);
				if (dist < shockwave->Reach)
				{
					pull = (shockwave->Reach - dist) * SHOCKWAVE_PULL_MAX_MUL;
					if (pull > dist * SHOCKWAVE_PULL_MIN_MUL) pull = dist * SHOCKWAVE_PULL_MIN_MUL;
					mul = pull / dist;
					VecMinusScalarVec((VEC*)&mv->x2, mul, &vec, (VEC*)&mv->x2);
				}
			}
		}
		else
		{
			for (j = ModelMeshModel->VertNum ; j ; j--, mv++)
			{
				SubVector((VEC*)&mv->x, &newdelta, &vec);
				dist = Length(&vec);
				if (dist < shockwave->Reach)
				{
					pull = (shockwave->Reach - dist) * SHOCKWAVE_PULL_MAX_MUL;
					if (pull > dist * SHOCKWAVE_PULL_MIN_MUL) pull = dist * SHOCKWAVE_PULL_MIN_MUL;
					mul = pull / dist;
					VecMinusScalarVec((VEC*)&mv->x, mul, &vec, (VEC*)&mv->x2);
				}
				else
				{
					CopyVec((VEC*)&mv->x, (VEC*)&mv->x2);
				}
			}
		}

// set flag

	*ModelMeshFlag |= MODEL_USENEWVERTS;
}

////////////////////////////////
// putty bomb mesh fx checker //
////////////////////////////////

void PuttyBombWorldMeshFxChecker(void *data)
{
	long i, j;
	CUBE_HEADER **cubelist;
	OBJECT *obj = (OBJECT*)data;
	PUTTYBOMB_OBJ *bomb = (PUTTYBOMB_OBJ*)obj->Data;
	VEC vec, *pos = &bomb->Pos;
	WORLD_VERTEX *wv;
	WORLD_MIRROR_VERTEX *wmv;
	REAL dist, mul, push, scalar;

// loop thru world cubes

	cubelist = World.CubeList;

	for (i = 0 ; i < WorldCubeCount ; i++)
	{

// check bounding box

		if (cubelist[i]->Xmin > bomb->Box.XMax || cubelist[i]->Xmax < bomb->Box.XMin ||
			cubelist[i]->Ymin > bomb->Box.YMax || cubelist[i]->Ymax < bomb->Box.YMin ||
			cubelist[i]->Zmin > bomb->Box.ZMax || cubelist[i]->Zmax < bomb->Box.ZMin)
				continue;

// check spheres

		SubVector((VEC*)&cubelist[i]->CentreX, pos, &vec);
		if (Length(&vec) > cubelist[i]->Radius + bomb->SphereRadius + PUTTYBOMB_PUSH_RANGE)
			continue;

// ok, set verts

		scalar = (PUTTYBOMB_SPHERE_TIME - bomb->Timer) / PUTTYBOMB_SPHERE_TIME;

		if (cubelist[i]->MeshFxFlag & MESHFX_USENEWVERTS)
		{
			wv = cubelist[i]->Model.VertPtr;
			for (j = cubelist[i]->Model.VertNum ; j ; j--, wv++)
			{
				SubVector((VEC*)&wv->x2, pos, &vec);
				dist = Length(&vec);

				push = (PUTTYBOMB_PUSH_RANGE - abs(bomb->SphereRadius - dist)) * scalar;
				if (push > 0.0f)
				{
					mul = push / dist;
					VecPlusScalarVec((VEC*)&wv->x2, mul, &vec, (VEC*)&wv->x2);
				}
			}

			wmv = cubelist[i]->Model.MirrorVertPtr;
			for (j = cubelist[i]->Model.MirrorVertNum ; j ; j--, wmv++)
			{
				SubVector((VEC*)&wmv->RealVertex->x2, pos, &vec);
				dist = Length(&vec);

				push = (PUTTYBOMB_PUSH_RANGE - abs(bomb->SphereRadius - dist)) * scalar;
				if (push > 0.0f)
				{
					mul = push / dist;
					VecPlusScalarVec((VEC*)&wmv->x2, mul, &vec, (VEC*)&wmv->x2);
				}
			}
		}
		else
		{
			wv = cubelist[i]->Model.VertPtr;
			for (j = cubelist[i]->Model.VertNum ; j ; j--, wv++)
			{
				SubVector((VEC*)&wv->x, pos, &vec);
				dist = Length(&vec);

				push = (PUTTYBOMB_PUSH_RANGE - abs(bomb->SphereRadius - dist)) * scalar;
				if (push > 0.0f)
				{
					mul = push / dist;
					VecPlusScalarVec((VEC*)&wv->x, mul, &vec, (VEC*)&wv->x2);
				}
				else
				{
					CopyVec((VEC*)&wv->x, (VEC*)&wv->x2);
				}
			}

			wmv = cubelist[i]->Model.MirrorVertPtr;
			for (j = cubelist[i]->Model.MirrorVertNum ; j ; j--, wmv++)
			{
				SubVector((VEC*)&wmv->RealVertex->x, pos, &vec);
				dist = Length(&vec);

				push = (PUTTYBOMB_PUSH_RANGE - abs(bomb->SphereRadius - dist)) * scalar;
				if (push > 0.0f)
				{
					mul = push / dist;
					VecPlusScalarVec((VEC*)&wmv->x, mul, &vec, (VEC*)&wmv->x2);
				}
				else
				{
					CopyVec((VEC*)&wmv->x, (VEC*)&wmv->x2);
				}
			}
		}

// set mesh flag

		cubelist[i]->MeshFxFlag |= MESHFX_USENEWVERTS;
	}
}

////////////////////////////////
// putty bomb mesh fx checker //
////////////////////////////////

void PuttyBombModelMeshFxChecker(void *data)
{
	long j;
	REAL push, dist, mul, scalar, rad = ModelMeshModel->Radius;
	OBJECT *obj = (OBJECT*)data;
	PUTTYBOMB_OBJ *bomb = (PUTTYBOMB_OBJ*)obj->Data;
	VEC delta, newdelta, vec;
	MODEL_VERTEX *mv;

// quick radius bounding box test

	if (ModelMeshPos->v[X] + rad < bomb->Box.XMin ||
		ModelMeshPos->v[X] - rad > bomb->Box.XMax ||
		ModelMeshPos->v[Y] + rad < bomb->Box.YMin ||
		ModelMeshPos->v[Y] - rad > bomb->Box.YMax ||
		ModelMeshPos->v[Z] + rad < bomb->Box.ZMin ||
		ModelMeshPos->v[Z] - rad > bomb->Box.ZMax)
			return;

// get delta vector

	SubVector(&bomb->Pos, ModelMeshPos, &delta);

// sphere test

	dist = Length(&delta);
	if (dist > rad + bomb->SphereRadius + PUTTYBOMB_PUSH_RANGE)
		return;

// put delta vector into model space

	TransposeRotVector(ModelMeshMat, &delta, &newdelta);

// proper bounding box test

	if (ModelMeshModel->Xmax < newdelta.v[X] - bomb->SphereRadius ||
		ModelMeshModel->Xmin > newdelta.v[X] + bomb->SphereRadius ||
		ModelMeshModel->Ymax < newdelta.v[Y] - bomb->SphereRadius ||
		ModelMeshModel->Ymin > newdelta.v[Y] + bomb->SphereRadius ||
		ModelMeshModel->Zmax < newdelta.v[Z] - bomb->SphereRadius ||
		ModelMeshModel->Zmin > newdelta.v[Z] + bomb->SphereRadius)
			return;

// ok, set verts

		scalar = (PUTTYBOMB_SPHERE_TIME - bomb->Timer) / PUTTYBOMB_SPHERE_TIME;

		mv = ModelMeshModel->VertPtr;

		if (*ModelMeshFlag & MODEL_USENEWVERTS)
		{
			for (j = ModelMeshModel->VertNum ; j ; j--, mv++)
			{
				SubVector((VEC*)&mv->x2, &newdelta, &vec);
				dist = Length(&vec);

				push = (PUTTYBOMB_PUSH_RANGE - abs(bomb->SphereRadius - dist)) * scalar;
				if (push > 0.0f)
				{
					mul = push / dist;
					VecPlusScalarVec((VEC*)&mv->x2, mul, &vec, (VEC*)&mv->x2);
				}
			}
		}
		else
		{
			for (j = ModelMeshModel->VertNum ; j ; j--, mv++)
			{
				SubVector((VEC*)&mv->x, &newdelta, &vec);
				dist = Length(&vec);

				push = (PUTTYBOMB_PUSH_RANGE - abs(bomb->SphereRadius - dist)) * scalar;
				if (push > 0.0f)
				{
					mul = push / dist;
					VecPlusScalarVec((VEC*)&mv->x, mul, &vec, (VEC*)&mv->x2);
				}
				else
				{
					CopyVec((VEC*)&mv->x, (VEC*)&mv->x2);
				}
			}
		}

// set flag

	*ModelMeshFlag |= MODEL_USENEWVERTS;
}

//////////////////////////
// render electro pulse //
//////////////////////////

void RenderElectroPulse(OBJECT *obj)
{
	long i;
	ELECTROPULSE_OBJ *electro = (ELECTROPULSE_OBJ*)obj->Data;
	CAR *car = &obj->player->car;
	VEC v1, v2;
	PLAYER *nplayer;

// draw model

	obj->renderflag.visible |= RenderObjectModel(&car->Body->Centre.WMatrix, &car->BodyWorldPos, &electro->Model, obj->EnvRGB, obj->renderflag);

// draw jump spark?

	for (i = 0 ; i < electro->JumpFlag ; i++)
	{
		nplayer = (PLAYER*)electro->Player[i];

		RotTransVector(&nplayer->car.Body->Centre.WMatrix, &nplayer->car.BodyWorldPos, &electro->JumpPos1[i], &v1);
		RotTransVector(&obj->player->car.Body->Centre.WMatrix, &obj->player->car.BodyWorldPos, &electro->JumpPos2[i], &v2);
		DrawJumpSpark(&v1, &v2);
	}
}

//////////////////////
// render oil slick //
/////////////////////

void RenderOilSlick(OBJECT *obj)
{
	OILSLICK_OBJ *oil = (OILSLICK_OBJ*)obj->Data;
	FACING_POLY poly;
	long rgb;
	BOUNDING_BOX box;

// falling?

	if (!oil->Mode)
	{
		if (oil->Age < 0.5f)
			poly.Xsize = poly.Ysize = oil->Age * 48.0f + 8.0f;
		else
			poly.Xsize = poly.Ysize = 32.0f;

		poly.U = 130.0f / 256.0f;
		poly.V = 66.0f / 256.0f;
		poly.Usize = poly.Vsize = 60.0f / 256.0f;
		poly.Tpage = TPAGE_FX1;
		poly.RGB = 0xffffff;

		DrawFacingPoly(&obj->body.Centre.Pos, &poly, 2, 0);
	}

// on floor

	else
	{
		SET_TPAGE(TPAGE_FX1);

		if (oil->Age < 28.0f)
		{
			rgb = 0xffffff;
		}
		else
		{
			FTOL((30.0f - oil->Age) * 127, rgb);
			rgb |= (rgb << 8) | (rgb << 16);
		}

		DrawShadow(&oil->Pos[0], &oil->Pos[1], &oil->Pos[2], &oil->Pos[3], 130.0f / 256.0f, 66.0f / 256.0f, 60.0f / 256.0f, 60.0f / 256.0f, rgb, -2.0f, 256.0f, 2, TPAGE_FX1, &box);

		oil->Ymin = box.Ymin - 1.0f;
		oil->Ymax = box.Ymax + 1.0f;
	}
}

////////////////////////
// render chrome ball //
////////////////////////

void RenderChromeBall(OBJECT *obj)
{
	CHROMEBALL_OBJ *ball = (CHROMEBALL_OBJ*)obj->Data;
	CAR *car = &obj->player->car;
	MAT mat;
	REAL mul;
	REAL z;
	BOUNDING_BOX box;
	MODEL *model = &LevelModel[obj->DefaultModel].Model;
	VEC v0, v1, v2, v3, *pos = &obj->body.Centre.Pos;
	long visflag;
	short flag = MODEL_ENV;

// set rad matrix

	CopyMat(&obj->body.Centre.WMatrix, &mat);
	mul = ball->Radius / LevelModel[obj->DefaultModel].Model.Radius;
	mat.m[XX] *= mul;
	mat.m[XY] *= mul;
	mat.m[XZ] *= mul;
	mat.m[YX] *= mul;
	mat.m[YY] *= mul;
	mat.m[YZ] *= mul;
	mat.m[ZX] *= mul;
	mat.m[ZY] *= mul;
	mat.m[ZZ] *= mul;

// get bounding box

	box.Xmin = pos->v[X] - model->Radius;
	box.Xmax = pos->v[X] + model->Radius;
	box.Ymin = pos->v[Y] - model->Radius;
	box.Ymax = pos->v[Y] + model->Radius;
	box.Zmin = pos->v[Z] - model->Radius;
	box.Zmax = pos->v[Z] + model->Radius;

// test against visicubes

	if (TestObjectVisiboxes(&box))
		return;

// skip if offscreen

	visflag = TestSphereToFrustum(pos, ball->Radius, &z);
	if (visflag == SPHERE_OUT) return;
	if (visflag == SPHERE_IN) flag |= MODEL_DONOTCLIP;

// set visible flag

	obj->renderflag.visible = TRUE;

// set env

	SetEnvStatic(pos, &obj->body.Centre.WMatrix, 0x808080, 0.0f, 0.1f, 0.5f);

// in light?

	if (CheckObjectLight(pos, &box, model->Radius))
	{
		flag |= MODEL_LIT;
		AddModelLight(model, pos, &obj->body.Centre.WMatrix);
	}

// reflect?

	if (RenderSettings.Mirror)
	{
		if (GetMirrorPlane(pos))
		{
			if (ViewCameraPos.v[Y] < MirrorHeight)
				flag |= MODEL_MIRROR;
		}
	}

// in fog?

	if (z + model->Radius > RenderSettings.FogStart && DxState.Fog)
	{
		ModelVertFog = (pos->v[Y] - RenderSettings.VertFogStart) * RenderSettings.VertFogMul;
		if (ModelVertFog < 0) ModelVertFog = 0;
		if (ModelVertFog > 255) ModelVertFog = 255;

		flag |= MODEL_FOG;
		FOG_ON();
	}

// mesh fx?

	CheckModelMeshFx(model, &obj->body.Centre.WMatrix, pos, &flag);

// draw model

	DrawModel(model, &mat, pos, flag);

// fog off?

	if (flag & MODEL_FOG)
		FOG_OFF();

// draw shadow

	SetVector(&v0, pos->v[X] - ball->Radius, pos->v[Y], pos->v[Z] - ball->Radius);
	SetVector(&v1, pos->v[X] + ball->Radius, pos->v[Y], pos->v[Z] - ball->Radius);
	SetVector(&v2, pos->v[X] + ball->Radius, pos->v[Y], pos->v[Z] + ball->Radius);
	SetVector(&v3, pos->v[X] - ball->Radius, pos->v[Y], pos->v[Z] + ball->Radius);

	DrawShadow(&v0, &v1, &v2, &v3, 193.0f / 256.0f, 129.0f / 256.0f, 62.0f / 256.0f, 62.0f / 256.0f, 0x808080, -2.0f, 0.0f, 2, TPAGE_FX1, NULL);
}

///////////////////////////
// render electro zapped //
///////////////////////////

void RenderElectroZapped(OBJECT *obj)
{
	ELECTROZAPPED_OBJ *electro = (ELECTROZAPPED_OBJ*)obj->Data;
	CAR *car = &obj->player->car;

// draw model

	obj->renderflag.visible |= RenderObjectModel(&car->Body->Centre.WMatrix, &car->BodyWorldPos, &electro->Model, obj->EnvRGB, obj->renderflag);
}

//////////////////////
// render bomb glow //
//////////////////////

void RenderBombGlow(OBJECT *obj)
{
	BOMBGLOW_OBJ *glow = (BOMBGLOW_OBJ*)obj->Data;
	CAR *car = &obj->player->car;

// draw model

	obj->renderflag.visible |= RenderObjectModel(&car->Body->Centre.WMatrix, &car->BodyWorldPos, &glow->Model, obj->EnvRGB, obj->renderflag);
}

//////////////////////
// render waterbomb //
//////////////////////

void RenderWaterBomb(OBJECT *obj)
{
	WATERBOMB_OBJ *bomb = (WATERBOMB_OBJ*)obj->Data;
	VEC vec, vec2;
	MAT mat;

// get world pos / mat

	SetVector(&vec2, WaterBombOff.v[X], WaterBombOff.v[Y] + (WATERBOMB_RADIUS * bomb->ScalarHoriz) - WATERBOMB_RADIUS, WaterBombOff.v[Z]);
	RotTransVector(&obj->body.Centre.WMatrix, &obj->body.Centre.Pos, &vec2, &vec);
	CopyMatrix(&obj->body.Centre.WMatrix, &mat);

	mat.m[RX] *= bomb->ScalarHoriz;
	mat.m[RY] *= bomb->ScalarHoriz;
	mat.m[RZ] *= bomb->ScalarHoriz;
	mat.m[LX] *= bomb->ScalarHoriz;
	mat.m[LY] *= bomb->ScalarHoriz;
	mat.m[LZ] *= bomb->ScalarHoriz;

	mat.m[YX] *= bomb->ScalarVert;
	mat.m[YY] *= bomb->ScalarVert;
	mat.m[YZ] *= bomb->ScalarVert;

// draw

	obj->renderflag.visible |= RenderObjectModel(&mat, &vec, &LevelModel[obj->DefaultModel].Model, obj->EnvRGB, obj->renderflag);
}

///////////////////////////
// render puttybomb bang //
///////////////////////////

void RenderPuttyBombBang(OBJECT *obj)
{
	PUTTYBOMB_OBJ *bomb = (PUTTYBOMB_OBJ*)obj->Data;
	long i, frame, rgb;
	REAL mul, x, z, tu, tv, size;
	FACING_POLY poly;
	MAT mat;
	MODEL *model = &LevelModel[obj->DefaultModel].Model;
	MODEL_POLY *mp;
	POLY_RGB *mrgb;
	VEC v0, v1, v2, v3;

// draw bang pieces

	poly.Usize = poly.Vsize = 30.0f / 256.0f;
	poly.Tpage = TPAGE_FX3;
	poly.RGB = 0xffffff;

	for (i = 0 ; i < PUTTYBOMB_BANG_NUM ; i++) if (bomb->Bang[i].Age >= 0.0f && bomb->Bang[i].Age < bomb->Bang[i].Life)
	{
		FTOL(bomb->Bang[i].Age / bomb->Bang[i].Life * 16.0f, frame);
		poly.U = ((float)(frame & 7) * 32.0f + 1.0f) / 256.0f;
		poly.V = ((float)(frame / 8) * 32.0f + 33.0f) / 256.0f;
	
		poly.Xsize = poly.Ysize = bomb->Bang[i].Size;

		DrawFacingPoly(&bomb->Bang[i].Pos, &poly, 1, 0.0f);
	}

// draw sphere

	if (bomb->Timer < PUTTYBOMB_SPHERE_TIME)
	{

// copy vert info to poly's

		FTOL((PUTTYBOMB_SPHERE_TIME - bomb->Timer) / PUTTYBOMB_SPHERE_TIME * 255.0f, rgb);
		rgb |= rgb << 8 | rgb << 16;

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

			if (mp->Type & POLY_QUAD)
			{
				mp->tu3 = mp->v3->tu;
				mp->tv3 = mp->v3->tv;
			}

			*(long*)&mrgb->rgb[0] = rgb;
			*(long*)&mrgb->rgb[1] = rgb;
			*(long*)&mrgb->rgb[2] = rgb;
			*(long*)&mrgb->rgb[3] = rgb;
		}

// set mat, draw

		CopyMatrix(&Identity, &mat);

		mul = bomb->SphereRadius / model->Radius;

		VecMulScalar(&mat.mv[R], mul);
		VecMulScalar(&mat.mv[U], mul);
		VecMulScalar(&mat.mv[L], mul);

		if (obj->DefaultModel != -1)
			obj->renderflag.visible |= RenderObjectModel(&mat, &bomb->Pos, &LevelModel[obj->DefaultModel].Model, 0, obj->renderflag);

// draw shockwave

		if (bomb->Timer < PUTTYBOMB_SPHERE_TIME)
		{
			size = bomb->SphereRadius * 0.5f;

			for (x = -2.0f ; x < 2.0f ; x++) for (z = -2.0f ; z < 2.0f ; z++)
			{
				SetVector(&v0, x * size + bomb->Pos.v[X], bomb->Pos.v[Y] - 256.0f, z * size + bomb->Pos.v[Z]);
				SetVector(&v1, v0.v[X] + size, bomb->Pos.v[Y] - 256.0f, v0.v[Z]);
				SetVector(&v2, v0.v[X] + size, bomb->Pos.v[Y] - 256.0f, v0.v[Z] + size);
				SetVector(&v3, v0.v[X], bomb->Pos.v[Y] - 256.0f, v0.v[Z] + size);

				tu = (64.0f + x * 31.0f) / 256.0f;
				tv = (192.0f + z * 31.0f) / 256.0f;

				DrawShadow(&v0, &v1, &v2, &v3, tu, tv, 31.0f / 256.0f, 31.0f / 256.0f, rgb, -2.0f, 512.0f, 1, TPAGE_FX3, NULL);
			}
		}
	}
}

#endif

/////////////////////////////////////////////////////////////////////
//
// WeaponTarget: choose a target object for the given players weapon
//
/////////////////////////////////////////////////////////////////////

OBJECT *WeaponTarget(OBJECT *playerObj)
{
	REAL score, best;
	REAL dRLen, lookdR;
	VEC dR;
	PLAYER *target, *bestTarget, *player;


	best = -LARGEDIST;
	bestTarget = NULL;
	player = playerObj->player;

	Assert(player != NULL);

	// Loop over other players
	for (target = PLR_PlayerHead; target != NULL; target = target->next) {

		// Only target other players and CPU cars
		if ((target->type != PLAYER_CPU) && (player->type != PLAYER_REMOTE) && (player->type != PLAYER_LOCAL)) continue;

		// do not target self
		if (target == player) continue;

		// Separation dependence
		VecMinusVec(&target->car.Body->Centre.Pos, &player->car.Body->Centre.Pos, &dR);
		dRLen = VecLen(&dR);
		if (dRLen > SMALL_REAL) {
			VecDivScalar(&dR, dRLen);
		} else {
			SetVecZero(&dR);
		}

		// Quick range check
		if ((dRLen > WEAPON_RANGE_MAX) || (dRLen < WEAPON_RANGE_MIN)) continue;

		// directional dependence
		lookdR = VecDotVec(&dR, &player->car.Body->Centre.WMatrix.mv[L]) - WEAPON_DIR_OFFSET;

		// Quit if target out of range
		if ((dRLen > (WEAPON_RANGE_MAX * lookdR)) || (lookdR < ZERO)) {
			continue;
		}

		// Generate target score for this player and see if it is the best
		score = (WEAPON_RANGE_MAX / (dRLen + WEAPON_RANGE_OFFSET)) * (lookdR * lookdR * lookdR * lookdR);

		if (score > best) {
			best = score;
			bestTarget = target;
		}

	}

	if (bestTarget == NULL) {
		return NULL;
	}

	return bestTarget->ownobj;
}
