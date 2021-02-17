/////////////////////////////////////////////////////////////////////
//
// Drawing objects like the car, wheels, and aerial
//
/////////////////////////////////////////////////////////////////////

#include "Revolt.h"
#include "Model.h"
#include "Play.h"
#include "Particle.h"
#include "Aerial.h"
#include "NewColl.h"
#include "Body.h"
#include "main.h"
#include "Wheel.h"
#include "Car.h"
#include "DrawObj.h"
#include "Shadow.h"
#include "Light.h"
#include "Draw.h"
#include "Geom.h"
#include "camera.h"
#include "dx.h"
#include "input.h"
#include "ctrlread.h"
#include "object.h"
#include "control.h"
#include "player.h"
#include "visibox.h"
#include "timing.h"
#include "registry.h"
#include "mirror.h"
#include "ai.h"
#include "ghost.h"
#include "text.h"
#include "spark.h"
#include "obj_init.h"

// prototypes

void DrawTarget(PLAYER *player);


// globals

FACING_POLY SunFacingPoly, DragonFireFacingPoly;

// car shadow tables

CAR_SHADOW_TABLE CarShadowTable[] = {
// rc
	-37, 37, 77, -70, -8,
	32, 0, 32, 64,
// mite
	-37, 37, 81, -79, -8,
	224, 0, 32, 64,
// phat
	-52, 52, 115, -124, -8,
	128, 0, 32, 64,
// col
	-40, 40, 85, -90, -8,
	160, 0, 32, 64,
// harvester
	-44, 44, 95, -93, -8,
	128, 64, 32, 64,
// doc
	-39, 39, 89, -86, -8,
	192, 0, 32, 64,
// volken
	-44, 44, 84, -97, -8,
	192, 64, 32, 64,
// sprinter
	-35, 35, 76, -69, -8,
	32, 192, 32, 64,
// dino
	-45, 45, 76, -92, -8,
	96, 0, 32, 64,
// candy
	-49, 49, 118, -97, -8,
	160, 64, 32, 64,
// genghis
	-51, 51, 107, -100, -8,
	0, 64, 32, 64,
// aquasonic
	-41, 41, 84, -81, -8,
	160, 128, 32, 64,
// mouse
	-64, 64, 132, -127, -8,
	96, 64, 32, 64,
// evil weasil
	-40, 40, 82, -82, -8,
	64, 0, 32, 64,
// panga
	-38, 38, 94, -67, -8,
	32, 128, 32, 64,
// r5
	-34, 34, 63, -69, -8,
	0, 0, 32, 64,
// loaded chique
	-38, 38, 74, -72, -8,
	192, 128, 32, 64,
// sgt bertha
	-56, 56, 132, -108, -8,
	64, 64, 32, 64,
// pest control
	-41, 41, 75, -85, -8,
	96, 128, 32, 64,
// adeon
	-34, 34, 75, -59, -8,
	224, 128, 32, 64,
// polepot
	-55, 55, 115, -110, -8,
	32, 64, 32, 64,
// zipper
	-38, 38, 86, -77, -8,
	0, 128, 32, 64,
// trolley
	-60, 60, 60, -60, -8,
	0, 0, 1, 1,
// cougar
	-43, 43, 92, -86, -8,
	128, 128, 32, 64,
// humma
	-37, 37, 82, -68, -8,
	0, 192, 32, 64,
// toyeca
	-38, 38, 70, -89, -8,
	224, 64, 32, 64,
// amw
	-42, 42, 96, -81, -8,
	64, 128, 32, 64,
};

////////////////////////////////////////
// test a sphere against view frustum //
////////////////////////////////////////

long TestSphereToFrustum(VEC *pos, float rad, float *z)
{
	float l, r, t, b;

// check if outside

	*z = pos->v[X] * ViewMatrix.m[RZ] + pos->v[Y] * ViewMatrix.m[UZ] + pos->v[Z] * ViewMatrix.m[LZ] + ViewTrans.v[Z];
	if (*z + rad < RenderSettings.NearClip || *z - rad >= RenderSettings.FarClip) return SPHERE_OUT;

	if ((l = PlaneDist(&CameraPlaneLeft, pos)) >= rad) return SPHERE_OUT;
	if ((r = PlaneDist(&CameraPlaneRight, pos)) >= rad) return SPHERE_OUT;
	if ((b = PlaneDist(&CameraPlaneBottom, pos)) >= rad) return SPHERE_OUT;
	if ((t = PlaneDist(&CameraPlaneTop, pos)) >= rad) return SPHERE_OUT;

// inside, check if needs to be clipped

	if (l > -rad || r > -rad || b > -rad || t > -rad || *z - rad < RenderSettings.NearClip || *z + rad >= RenderSettings.FarClip) return SPHERE_CLIP;
	return SPHERE_IN;
}

/*long TestBBoxToFrustum(BBOX *bBox, float *z)
{
	VEC pos;
	float l, r, t, b, rad;

	pos.v[X] = HALF * (bBox->XMin + bBox->XMax);
	pos.v[Y] = HALF * (bBox->YMin + bBox->YMax);
	pos.v[Z] = HALF * (bBox->ZMin + bBox->ZMax);
	r = HALF * (bBox->XMax - bBox->XMin);
	t = HALF * (bBox->YMax - bBox->YMin);
	b = HALF * (bBox->ZMax - bBox->ZMin);
	rad = sqrt(r * r + t * t + b * b);

// check if outside

	*z = pos.v[X] * ViewMatrix.m[RZ] + pos.v[Y] * ViewMatrix.m[UZ] + pos.v[Z] * ViewMatrix.m[LZ] + ViewTrans.v[Z];
	if (*z + rad < RenderSettings.NearClip || *z - rad >= RenderSettings.FarClip) return SPHERE_OUT;

	if ((l = PlaneDist(&CameraPlaneLeft, pos)) >= rad) return SPHERE_OUT;
	if ((r = PlaneDist(&CameraPlaneRight, pos)) >= rad) return SPHERE_OUT;
	if ((b = PlaneDist(&CameraPlaneBottom, pos)) >= rad) return SPHERE_OUT;
	if ((t = PlaneDist(&CameraPlaneTop, pos)) >= rad) return SPHERE_OUT;

// inside, check if needs to be clipped

	if (l > -rad || r > -rad || b > -rad || t > -rad || *z - rad < RenderSettings.NearClip || *z + rad >= RenderSettings.FarClip) return SPHERE_CLIP;
	return SPHERE_IN;
}*/

////////////////////////////
// build all car matrices //
////////////////////////////

void BuildAllCarWorldMatrices(void)
{
	PLAYER *player;

// set GhostSines;

	GhostSineCount += TimeFactor / 20;
	GhostSinePos = (float)sin(GhostSineCount) * 128;

// update world matrices

	for (player = PLR_PlayerHead ; player ; player = player->next)
	{
		BuildCarMatricesNew(&player->car);
	}
}

/////////////////////////////////////////////////////////////////////
//
// BuildCarMatricesNew: Build world matrices + positions for all
// car models
//
/////////////////////////////////////////////////////////////////////

void BuildCarMatricesNew(CAR *car) 
{
	WHEEL *wheel;
	SUSPENSION *susp;
	VEC tmpVec;
	VEC fixedPos;
	MAT tmpMat, tmpMat2;
	REAL scale;
	int iWheel;

// build body world pos

	RotTransVector(&car->Body->Centre.WMatrix, &car->Body->Centre.Pos, &car->BodyOffset, &car->BodyWorldPos);

// loop thru possible wheels

	for (iWheel = 0; iWheel < CAR_NWHEELS; iWheel++)
	{
		wheel = &car->Wheel[iWheel];
		susp = &car->Sus[iWheel];

// get axle / spring fix position for this wheel

		VecPlusScalarVec(&car->WheelOffset[iWheel], wheel->Pos, &DownVec, &fixedPos);

// set spring world matrix + pos

		if (CarHasSpring(car, iWheel))
		{
			BuildLookMatrixDown(&car->SuspOffset[iWheel], &fixedPos, &tmpMat);
			SubVector(&fixedPos, &car->SuspOffset[iWheel], &tmpVec);
			scale = Length(&tmpVec) / susp->SpringLen;
			VecMulScalar(&tmpMat.mv[U], scale);

			MulMatrix(&car->Body->Centre.WMatrix, &tmpMat, &susp->SpringCarMatrix);
			RotTransVector(&car->Body->Centre.WMatrix, &car->Body->Centre.Pos, &car->SuspOffset[iWheel], &susp->SpringWorldPos);
		}

// set axle world matrix + pos

		if (CarHasAxle(car, iWheel))
		{
			BuildLookMatrixForward(&car->AxleOffset[iWheel], &fixedPos, &tmpMat);
			SubVector(&fixedPos, &car->AxleOffset[iWheel], &tmpVec);
			scale = Length(&tmpVec) / susp->AxleLen;
			VecMulScalar(&tmpMat.mv[L], scale);

			MulMatrix(&car->Body->Centre.WMatrix, &tmpMat, &susp->AxleCarMatrix);
			RotTransVector(&car->Body->Centre.WMatrix, &car->Body->Centre.Pos, &car->AxleOffset[iWheel], &susp->AxleWorldPos);
		}

// set pin world matrix + pos

		if (CarHasPin(car, iWheel))
		{
			BuildLookMatrixDown(&car->SuspOffset[iWheel], &fixedPos, &tmpMat);
			VecMulScalar(&tmpMat.mv[U], -susp->PinLen);

			MulMatrix(&car->Body->Centre.WMatrix, &tmpMat, &susp->PinCarMatrix);
			RotTransVector(&car->Body->Centre.WMatrix, &car->Body->Centre.Pos, &fixedPos, &susp->PinWorldPos);
		}

		if (CarHasSpinner(car)) {
			BuildRotation3D(car->Spinner.Axis.v[X], car->Spinner.Axis.v[Y], car->Spinner.Axis.v[Z], TimeStep * car->Spinner.AngVel, &tmpMat);
			MatMulTransMat(&tmpMat, &car->Spinner.Matrix, &tmpMat2);
			TransMat(&tmpMat2, &car->Spinner.Matrix);
			MulMatrix(&car->Body->Centre.WMatrix, &car->Spinner.Matrix, &car->Spinner.CarMatrix);

			RotTransVector(&car->Body->Centre.WMatrix, &car->Body->Centre.Pos, &car->Models->OffSpinner, &car->Spinner.WorldPos);
		}
	}
}

///////////////////
// draw all cars //
///////////////////

void DrawAllCars(void)
{
	PLAYER *player;

// draw cars

	if (GhostSolid)
		GHO_GhostPlayer->type = PLAYER_LOCAL;

	for (player = PLR_PlayerHead ; player ; player = player->next) if (player->type != PLAYER_GHOST)
	{
		DrawCar(&player->car);			// draw the car models
		if (player->PickupTarget != NULL) 
		{
			DrawTarget(player);				// draw the weapon target
		}
	}

	if (GhostSolid)
		GHO_GhostPlayer->type = PLAYER_GHOST;
}

/////////////////////////
// draw all ghost cars //
/////////////////////////

void DrawAllGhostCars(void)
{
	PLAYER *player;

// set render states

	if (GhostSolid)
		return;

	ZWRITE_ON();
	ALPHA_ON();
	ALPHA_SRC(D3DBLEND_SRCALPHA);
	ALPHA_DEST(D3DBLEND_INVSRCALPHA);

// draw cars

	for (player = PLR_PlayerHead ; player ; player = player->next) if (player->type == PLAYER_GHOST)
	{
		DrawCarGhost(&player->car);
	}

	FlushPolyBuckets();
}

//////////////
// draw car //
//////////////

void DrawCar(CAR *car)
{
	long ii, visflag, envrgb = car->Models->EnvRGB;
	REAL z, flod;
	BOUNDING_BOX box;
	short flag = MODEL_PLAIN;
	char lod, lod2;

// zero car rendered flag

	car->Rendered = FALSE;

// set whole car bounding box

	box.Xmin = car->Body->Centre.Pos.v[X] - CAR_RADIUS;
	box.Xmax = car->Body->Centre.Pos.v[X] + CAR_RADIUS;
	box.Ymin = car->Body->Centre.Pos.v[Y] - CAR_RADIUS;
	box.Ymax = car->Body->Centre.Pos.v[Y] + CAR_RADIUS;
	box.Zmin = car->Body->Centre.Pos.v[Z] - CAR_RADIUS;
	box.Zmax = car->Body->Centre.Pos.v[Z] + CAR_RADIUS;

// test against visicubes

	if (TestObjectVisiboxes(&box))
		return;

// skip if offscreen

	visflag = TestSphereToFrustum(&car->Body->Centre.Pos, CAR_RADIUS, &z);
	if (visflag == SPHERE_OUT) return;
	if (visflag == SPHERE_IN) flag |= MODEL_DONOTCLIP;

// set car rendered flag

	car->Rendered = TRUE;

// calc lod

	flod = z / CAR_LOD_BIAS - 2;
	if (flod < 0) flod = 0;
	if (flod > MAX_CAR_LOD - 1) flod = MAX_CAR_LOD - 1;
	FTOL(flod, lod);

// in fog?

	if (z + CAR_RADIUS > RenderSettings.FogStart && DxState.Fog)
	{
		ModelVertFog = (car->Body->Centre.Pos.v[1] - RenderSettings.VertFogStart) * RenderSettings.VertFogMul;
		if (ModelVertFog < 0) ModelVertFog = 0;
		if (ModelVertFog > 255) ModelVertFog = 255;

		flag |= MODEL_FOG;
		FOG_ON();
	}

// in light?

	if (CheckObjectLight(&car->Body->Centre.Pos, &box, CAR_RADIUS))
	{
		flag |= MODEL_LIT;
	}

// is bomb?

	if (car->AddLit)
	{

		ModelAddLit = car->AddLit;
		flag |= MODEL_ADDLIT;
	}

// scale?

	if (car->DrawScale != 1.0f)
	{
		ModelScale = car->DrawScale;
		flag |= MODEL_SCALE;
	}

// reflect?

	if (RenderSettings.Mirror)
	{
		if (GetMirrorPlane(&car->Body->Centre.Pos))
		{
			if (ViewCameraPos.v[Y] < MirrorHeight)
				flag |= MODEL_MIRROR;
		}
	}

// draw models

	SetEnvActive(&car->Body->Centre.Pos, &car->Body->Centre.WMatrix, &car->EnvMatrix, envrgb, 0.0f, 0.0f, 1.0f);
//	SetEnvStatic(&car->Body->Centre.Pos, &car->Body->Centre.WMatrix, envrgb, 0.0f, 0.0f, 1.0f);

	lod2 = lod;
	while (!car->Models->Body[lod2].PolyNum) lod2--;
	if (flag & MODEL_LIT) AddModelLight(&car->Models->Body[lod2], &car->BodyWorldPos, &car->Body->Centre.WMatrix);
	DrawModel(&car->Models->Body[lod2], &car->Body->Centre.WMatrix, &car->BodyWorldPos, flag | MODEL_ENV);

// loop thru possible wheels

	for (ii = 0; ii < CAR_NWHEELS; ii++)
	{

// draw wheel

		if (IsWheelPresent(&car->Wheel[ii]))
		{
			lod2 = lod;
			while (!car->Models->Wheel[ii][lod2].PolyNum) lod2--;
			if (flag & MODEL_LIT) AddModelLight(&car->Models->Wheel[ii][lod2], &car->Wheel[ii].WPos, &car->Wheel[ii].WMatrix);
			DrawModel(&car->Models->Wheel[ii][lod2], &car->Wheel[ii].WMatrix, &car->Wheel[ii].WPos, flag);
		}

// draw spring

		if (CarHasSpring(car, ii))
		{
			lod2 = lod;
			while (!car->Models->Spring[ii][lod2].PolyNum) lod2--;
			if (flag & MODEL_LIT) AddModelLightSimple(&car->Models->Spring[ii][lod2], &car->Sus[ii].SpringWorldPos);
			DrawModel(&car->Models->Spring[ii][lod2], &car->Sus[ii].SpringCarMatrix, &car->Sus[ii].SpringWorldPos, flag);
		}

// draw axle

		if (CarHasAxle(car, ii))
		{
			lod2 = lod;
			while (!car->Models->Axle[ii][lod2].PolyNum) lod2--;
			if (flag & MODEL_LIT) AddModelLightSimple(&car->Models->Axle[ii][lod2], &car->Sus[ii].AxleWorldPos);
			DrawModel(&car->Models->Axle[ii][lod2], &car->Sus[ii].AxleCarMatrix, &car->Sus[ii].AxleWorldPos, flag);
		}

// draw pin

		if (CarHasPin(car, ii))
		{
			lod2 = lod;
			while (!car->Models->Pin[ii][lod2].PolyNum) lod2--;
			if (flag & MODEL_LIT) AddModelLightSimple(&car->Models->Pin[ii][lod2], &car->Sus[ii].PinWorldPos);
			DrawModel(&car->Models->Pin[ii][lod2], &car->Sus[ii].PinCarMatrix, &car->Sus[ii].PinWorldPos, flag);
		}
	}

// draw spinner

	if (CarHasSpinner(car))
	{
		lod2 = lod;
		while (!car->Models->Spinner[lod2].PolyNum) lod2--;
		if (flag & MODEL_LIT) AddModelLightSimple(&car->Models->Spinner[lod2], &car->Spinner.WorldPos);
		DrawModel(&car->Models->Spinner[lod2], &car->Spinner.CarMatrix, &car->Spinner.WorldPos, flag);
	}

// show physics info?

#if SHOW_PHYSICS_INFO
	if (CAR_DrawCarAxes) {
		DrawAxis(&car->Body->Centre.WMatrix, &car->Body->Centre.Pos);
	}
	if (CAR_DrawCarBBoxes) {
		ALPHA_ON();
		ALPHA_SRC(D3DBLEND_ONE);
		ALPHA_DEST(D3DBLEND_ONE);

		DrawCarBoundingBoxes(car);

		ALPHA_OFF();
	}
#endif

// draw aerial

	if (CarHasAerial(car)) 
	{
		DrawCarAerial2(&car->Aerial, car->Models->Aerial[0], car->Models->Aerial[1], flag);
	}

// reset render states?

	if (flag & MODEL_FOG)
		FOG_OFF();
}

////////////////////
// draw ghost car //
////////////////////

void DrawCarGhost(CAR *car)
{
	long ii, visflag;
	REAL z, flod;
	BOUNDING_BOX box;
	short flag = MODEL_GHOST;
	char lod, lod2;

// zero car rendered flag

	car->Rendered = FALSE;

// set whole car bounding box

	box.Xmin = car->Body->Centre.Pos.v[X] - CAR_RADIUS;
	box.Xmax = car->Body->Centre.Pos.v[X] + CAR_RADIUS;
	box.Ymin = car->Body->Centre.Pos.v[Y] - CAR_RADIUS;
	box.Ymax = car->Body->Centre.Pos.v[Y] + CAR_RADIUS;
	box.Zmin = car->Body->Centre.Pos.v[Z] - CAR_RADIUS;
	box.Zmax = car->Body->Centre.Pos.v[Z] + CAR_RADIUS;

// test against visicubes

	if (TestObjectVisiboxes(&box))
		return;

// skip if offscreen

	visflag = TestSphereToFrustum(&car->Body->Centre.Pos, CAR_RADIUS, &z);
	if (visflag == SPHERE_OUT) return;
	if (visflag == SPHERE_IN) flag |= MODEL_DONOTCLIP;

// set car rendered flag

	car->Rendered = TRUE;

// calc lod

	flod = z / CAR_LOD_BIAS - 2;
	if (flod < 0) flod = 0;
	if (flod > MAX_CAR_LOD - 1) flod = MAX_CAR_LOD - 1;
	FTOL(flod, lod);

// in fog?

	if (z + CAR_RADIUS > RenderSettings.FogStart && DxState.Fog)
	{
		ModelVertFog = (car->Body->Centre.Pos.v[1] - RenderSettings.VertFogStart) * RenderSettings.VertFogMul;
		if (ModelVertFog < 0) ModelVertFog = 0;
		if (ModelVertFog > 255) ModelVertFog = 255;

		flag |= MODEL_FOG;
		FOG_ON();
	}

// draw models

	GhostSineOffset = car->BodyOffset.v[Z];
	lod2 = lod;
	while (!car->Models->Body[lod2].PolyNum) lod2--;
	DrawModel(&car->Models->Body[lod2], &car->Body->Centre.WMatrix, &car->BodyWorldPos, flag);

// loop thru possible wheels

	for (ii = 0; ii < CAR_NWHEELS; ii++)
	{

// draw wheel

		if (IsWheelPresent(&car->Wheel[ii]))
		{
			GhostSineOffset = car->WheelOffset[ii].v[Z];
			lod2 = lod;
			while (!car->Models->Wheel[ii][lod2].PolyNum) lod2--;
			DrawModel(&car->Models->Wheel[ii][lod2], &car->Wheel[ii].WMatrix, &car->Wheel[ii].WPos, flag);
		}

// draw spring

		if (CarHasSpring(car, ii))
		{
			GhostSineOffset = car->SuspOffset[ii].v[Z];
			lod2 = lod;
			while (!car->Models->Spring[ii][lod2].PolyNum) lod2--;
			DrawModel(&car->Models->Spring[ii][lod2], &car->Sus[ii].SpringCarMatrix, &car->Sus[ii].SpringWorldPos, flag);
		}

// draw axle

		if (CarHasAxle(car, ii))
		{
			GhostSineOffset = car->AxleOffset[ii].v[Z];
			lod2 = lod;
			while (!car->Models->Axle[ii][lod2].PolyNum) lod2--;
			DrawModel(&car->Models->Axle[ii][lod2], &car->Sus[ii].AxleCarMatrix, &car->Sus[ii].AxleWorldPos, flag);
		}

// draw pin

		if (CarHasPin(car, ii))
		{
			GhostSineOffset = car->SuspOffset[ii].v[Z];
			lod2 = lod;
			while (!car->Models->Pin[ii][lod2].PolyNum) lod2--;
			DrawModel(&car->Models->Pin[ii][lod2], &car->Sus[ii].PinCarMatrix, &car->Sus[ii].PinWorldPos, flag);
		}
	}

// draw spinner

	if (CarHasSpinner(car))
	{
		GhostSineOffset = car->SpinnerOffset.v[Z];
		lod2 = lod;
		while (!car->Models->Spinner[lod2].PolyNum) lod2--;
		DrawModel(&car->Models->Spinner[lod2], &car->Spinner.CarMatrix, &car->Spinner.WorldPos, flag);
	}

// show physics info?

#if SHOW_PHYSICS_INFO
	if (CAR_DrawCarAxes) {
		DrawAxis(&car->Body->Centre.WMatrix, &car->Body->Centre.Pos);
	}
	if (CAR_DrawCarBBoxes) {
		ALPHA_ON();
		ALPHA_SRC(D3DBLEND_ONE);
		ALPHA_DEST(D3DBLEND_ONE);
		ZWRITE_OFF();

		DrawCarBoundingBoxes(car);

		ZWRITE_ON();
		ALPHA_OFF();
	}
#endif

// draw aerial

	GhostSineOffset = car->AerialOffset.v[Z];
	if (CarHasAerial(car)) 
	{
		DrawCarAerial2(&car->Aerial, car->Models->Aerial[0], car->Models->Aerial[1], flag);
	}

// reset render states?

	if (flag & MODEL_FOG)
		FOG_OFF();
}

/////////////////////
// draw car shadow //
/////////////////////

void DrawAllCarShadows(void)
{
	VEC s0, s1, s2, s3;
	VEC p0, p1, p2, p3;
	CAR *car;
	PLAYER *player;
	CAR_SHADOW_TABLE *sh;

// not if shadows off

	if (!RenderSettings.Shadow)
		return;

// set render states

	ALPHA_ON();
	ALPHA_SRC(D3DBLEND_ZERO);
	ALPHA_DEST(D3DBLEND_INVSRCCOLOR);

	ZWRITE_OFF();
	SET_TPAGE(TPAGE_SHADOW);

// draw all visible car shadows

	for (player = PLR_PlayerHead ; player ; player = player->next)
	{
		car = &player->car;
		sh = &CarShadowTable[car->CarID];

		if (car->Rendered)
		{
			SetVector(&s0, sh->Left, sh->Height, sh->Front);
			SetVector(&s1, sh->Right, sh->Height, sh->Front);
			SetVector(&s2, sh->Right, sh->Height, sh->Back);
			SetVector(&s3, sh->Left, sh->Height, sh->Back);

			RotTransVector(&car->Body->Centre.WMatrix, &car->Body->Centre.Pos, &s0, &p0);
			RotTransVector(&car->Body->Centre.WMatrix, &car->Body->Centre.Pos, &s1, &p1);
			RotTransVector(&car->Body->Centre.WMatrix, &car->Body->Centre.Pos, &s2, &p2);
			RotTransVector(&car->Body->Centre.WMatrix, &car->Body->Centre.Pos, &s3, &p3);

			DrawShadow(&p0, &p1, &p2, &p3, sh->tu / 256.0f, sh->tv / 256.0f, sh->twidth / 256.0f, sh->theight / 256.0f, 0x808080, -2.0f, 0.0f, -1, TPAGE_SHADOW, NULL);


#if FALSE
			if (player == GHO_GhostPlayer) continue;

			if (Keys[DIK_1]) sh->Left--;
			if (Keys[DIK_2]) sh->Left++;
			if (Keys[DIK_3]) sh->Right--;
			if (Keys[DIK_4]) sh->Right++;
			if (Keys[DIK_5]) sh->Front--;
			if (Keys[DIK_6]) sh->Front++;
			if (Keys[DIK_7]) sh->Back--;
			if (Keys[DIK_8]) sh->Back++;
			if (Keys[DIK_MINUS]) sh->Height--;
			if (Keys[DIK_EQUALS]) sh->Height++;

			BeginTextState();
			char buf[128];
			wsprintf(buf, "%d %d %d %d %d", (long)sh->Left, (long)sh->Right, (long)sh->Front, (long)sh->Back, (long)sh->Height);
			DumpText(128, 128, 12, 16, 0xff00ff, buf);
#endif
		}
	}
}

/////////////////////////////////////////////////////////////////////
//
// DrawCarAerial: draw the car's aerial!
//
/////////////////////////////////////////////////////////////////////

void DrawCarAerial2(AERIAL *aerial, MODEL *secModel, MODEL *topModel, short flag)
{
	int iSec;
	VEC	thisPos;
	VEC	lastPos;
	MAT	wMatrix;

	// Calculate the positions of the non-control sections by interpolating from the control sections
	CopyVec(&aerial->Section[0].Pos, &lastPos);

	for (iSec = AERIAL_START; iSec < AERIAL_NTOTSECTIONS; iSec++) {
		
		// calculate the position of the interpolated node
		Interpolate3D(
			&aerial->Section[0].Pos,
			&aerial->Section[AERIAL_SKIP].Pos,
			&aerial->Section[AERIAL_LASTSECTION].Pos,
			iSec * AERIAL_UNITLEN,
			&thisPos);

		// Set the up vector of the node (already scaled to give correct length)
		VecMinusVec(&thisPos, &lastPos, &wMatrix.mv[U]);

		// Build the world Matrix for the section
		BuildMatrixFromUp(&wMatrix);
		// Must normalise look vector when passed up vector was not normalised
		NormalizeVec(&wMatrix.mv[L]);

		// Draw the actual model
		if (iSec != AERIAL_NTOTSECTIONS - 1) {
			if (flag & MODEL_LIT) {
				AddModelLightSimple(secModel, &thisPos);
			}
			DrawModel(secModel, &wMatrix, &thisPos, flag);
		} else {
			if (flag & MODEL_LIT) {
				AddModelLightSimple(topModel, &thisPos);
			}
			DrawModel(topModel, &wMatrix, &thisPos, flag);
		}

		CopyVec(&thisPos, &lastPos);
	}
}

/////////////////////////////////////////////////////////////////////
//
// DrawCarBoundingBoxes:
//
/////////////////////////////////////////////////////////////////////

void DrawCarBoundingBoxes(CAR *car)
{
	int iSkin;
	CONVEX *pSkin;
	int iWheel;
	WHEEL *pWheel;
	
	
	int iCol = 0;
	int nCols = 6;
	long cols[][3] = { 
		{0x000022, 0x000022, 0x000022},
		{0x002200, 0x002200, 0x002200},
		{0x220000, 0x220000, 0x220000},
		{0x002222, 0x002222, 0x002222},
		{0x222200, 0x222200, 0x222200},
		{0x220022, 0x220022, 0x220022},
	};

	// Overall car BBox
	DrawBoundingBox(
		car->BBox.XMin, 
		car->BBox.XMax, 
		car->BBox.YMin, 
		car->BBox.YMax, 
		car->BBox.ZMin, 
		car->BBox.ZMax, 
		cols[iCol][0], cols[iCol][1], cols[iCol][2], 
		cols[iCol][0], cols[iCol][1], cols[iCol][2]);
	iCol++;
	if (iCol == nCols) iCol = 0;

	// Main body BBox
	DrawBoundingBox(
		car->Body->CollSkin.BBox.XMin, 
		car->Body->CollSkin.BBox.XMax, 
		car->Body->CollSkin.BBox.YMin, 
		car->Body->CollSkin.BBox.YMax, 
		car->Body->CollSkin.BBox.ZMin, 
		car->Body->CollSkin.BBox.ZMax, 
		cols[iCol][0], cols[iCol][1], cols[iCol][2], 
		cols[iCol][0], cols[iCol][1], cols[iCol][2]);
	iCol++;
	if (iCol == nCols) iCol = 0;


	// Collision Skin boxes
	for (iSkin = 0; iSkin < car->Body->CollSkin.NConvex; iSkin++) {
		pSkin = &car->Body->CollSkin.WorldConvex[iSkin];

		DrawBoundingBox(
			pSkin->BBox.XMin, 
			pSkin->BBox.XMax,
			pSkin->BBox.YMin, 
			pSkin->BBox.YMax,
			pSkin->BBox.ZMin, 
			pSkin->BBox.ZMax,
			cols[iCol][0], cols[iCol][1], cols[iCol][2], 
			cols[iCol][0], cols[iCol][1], cols[iCol][2]);
		iCol++;
		if (iCol == nCols) iCol = 0;
	}

	// Wheel BBoxes
	for (iWheel = 0; iWheel < CAR_NWHEELS; iWheel++) {
		pWheel = &car->Wheel[iWheel];
		if (!IsWheelPresent(pWheel)) continue;

		DrawBoundingBox(
			pWheel->BBox.XMin + pWheel->CentrePos.v[X], 
			pWheel->BBox.XMax + pWheel->CentrePos.v[X],
			pWheel->BBox.YMin + pWheel->CentrePos.v[Y], 
			pWheel->BBox.YMax + pWheel->CentrePos.v[Y],
			pWheel->BBox.ZMin + pWheel->CentrePos.v[Z], 
			pWheel->BBox.ZMax + pWheel->CentrePos.v[Z],
			cols[iCol][0], cols[iCol][1], cols[iCol][2], 
			cols[iCol][0], cols[iCol][1], cols[iCol][2]);
		iCol++;
		if (iCol == nCols) iCol = 0;
	}

}



/////////////////////////////////////////////////////////////////////
//
// DrawSkidMarks:
//
/////////////////////////////////////////////////////////////////////

void DrawSkidMarks()
{
	int iSkid, j;
	int currentSkid;
	SKIDMARK *skid;
	REAL z;

	// quit?
	if (!WHL_NSkids || !RenderSettings.Skid) return;

	// render states
	ALPHA_ON();
	ALPHA_SRC(D3DBLEND_ZERO);
	ALPHA_DEST(D3DBLEND_INVSRCCOLOR);

	ZWRITE_OFF();
	SET_TPAGE(TPAGE_SHADOW);

	DrawVertsTEX1[0].tu = 224.0f / 256.0f;
	DrawVertsTEX1[0].tv = 194.0f / 256.0f;

	DrawVertsTEX1[1].tu = 256.0f / 256.0f;
	DrawVertsTEX1[1].tv = 194.0f / 256.0f;

	DrawVertsTEX1[2].tu = 256.0f / 256.0f;
	DrawVertsTEX1[2].tv = 254.0f / 256.0f;

	DrawVertsTEX1[3].tu = 224.0f / 256.0f;
	DrawVertsTEX1[3].tv = 254.0f / 256.0f;


	// draw skidmarks
	currentSkid = WHL_SkidHead;
	for (iSkid = 0; iSkid < WHL_NSkids; iSkid++) {

		// address of next skidmark to draw
		currentSkid--;
		Wrap(currentSkid, 0, SKID_MAX_SKIDS);
		skid = &WHL_SkidMark[currentSkid];

		// basic visibility test
		if (skid->VisiMask & CamVisiMask) continue;
		z = skid->Centre.v[X] * ViewMatrix.m[RZ] + skid->Centre.v[Y] * ViewMatrix.m[UZ] + skid->Centre.v[Z] * ViewMatrix.m[LZ] + ViewTrans.v[Z];
		if (z + SKID_HALF_LEN < RenderSettings.NearClip || z - SKID_HALF_LEN >= RenderSettings.FarClip) continue;

		// draw skidmark
		for (j = 0 ; j < 4 ; j++)
		{
			RotTransPersVector(&ViewMatrixScaled, &ViewTransScaled, &skid->Corner[j], (REAL*)&DrawVertsTEX1[j]);
			DrawVertsTEX1[j].color = skid->RGB;
		}
		D3Ddevice->DrawPrimitive(D3DPT_TRIANGLEFAN, FVF_TEX1, DrawVertsTEX1, 4, D3DDP_DONOTUPDATEEXTENTS);
	}
}

//////////////////////
// draw all objects //
//////////////////////

void DrawObjects(void)
{
	OBJECT *obj;

	for (obj = OBJ_ObjectHead; obj; obj = obj->next)
	{
		if (obj->renderhandler && obj->flag.Draw)

		{
			obj->renderhandler(obj);
		}
	}	
}

/////////////////////////////
// default object renderer //
/////////////////////////////

void RenderObject(OBJECT *obj)
{
	if (obj->DefaultModel != -1)
		obj->renderflag.visible |= RenderObjectModel(&obj->body.Centre.WMatrix, &obj->body.Centre.Pos, &LevelModel[obj->DefaultModel].Model, obj->EnvRGB, obj->renderflag);
}

////////////////////////////
// render an object model //
////////////////////////////

bool RenderObjectModel(MAT *mat, VEC *pos, MODEL *model, long envrgb, struct renderflags renderflag)
{
	REAL z;
	BOUNDING_BOX box;
	long visflag;
	short flag = MODEL_PLAIN;

// get bounding box

	box.Xmin = pos->v[X] - model->Radius;
	box.Xmax = pos->v[X] + model->Radius;
	box.Ymin = pos->v[Y] - model->Radius;
	box.Ymax = pos->v[Y] + model->Radius;
	box.Zmin = pos->v[Z] - model->Radius;
	box.Zmax = pos->v[Z] + model->Radius;

// test against visicubes

	if (TestObjectVisiboxes(&box))
		return FALSE;

// skip if offscreen

	visflag = TestSphereToFrustum(pos, model->Radius, &z);
	if (visflag == SPHERE_OUT) return FALSE;
	if (visflag == SPHERE_IN) flag |= MODEL_DONOTCLIP;

// env?

	if (renderflag.envmap)
	{
		flag |= MODEL_ENV;
		SetEnvStatic(pos, mat, envrgb, 0.0f, 0.0f, 1.0f);
	}

// in light?

	if (renderflag.light)
	{
		if (CheckObjectLight(pos, &box, model->Radius))
		{
			flag |= MODEL_LIT;
			AddModelLight(model, pos, mat);
		}
	}

// reflect?

	if (renderflag.reflect && RenderSettings.Mirror)
	{
		if (GetMirrorPlane(pos))
		{
			if (ViewCameraPos.v[Y] < MirrorHeight)
				flag |= MODEL_MIRROR;
		}
	}

// in fog?

	if (renderflag.fog && z + model->Radius > RenderSettings.FogStart && DxState.Fog)
	{
		ModelVertFog = (pos->v[Y] - RenderSettings.VertFogStart) * RenderSettings.VertFogMul;
		if (ModelVertFog < 0) ModelVertFog = 0;
		if (ModelVertFog > 255) ModelVertFog = 255;

		flag |= MODEL_FOG;
		FOG_ON();
	}

// glare?

	if (renderflag.glare)
		flag |= MODEL_GLARE;

// mesh fx?

	if (renderflag.meshfx)
	{
		CheckModelMeshFx(model, mat, pos, &flag);
	}

// draw model

	DrawModel(model, mat, pos, flag);

// fog off?

	if (flag & MODEL_FOG)
		FOG_OFF();

// return rendered

	return TRUE;
}

///////////////////
// render planet //
///////////////////

void RenderPlanet(OBJECT *obj)
{
	REAL z;
	BOUNDING_BOX box;
	long visflag;
	VEC *pos = &obj->body.Centre.Pos;
	MAT *mat = &obj->body.Centre.WMatrix;
	MODEL *model = &LevelModel[obj->DefaultModel].Model;
	PLANET_OBJ *planet = (PLANET_OBJ*)obj->Data;
	short flag = MODEL_PLAIN;

// get bounding box

	box.Xmin = pos->v[X] - model->Radius;
	box.Xmax = pos->v[X] + model->Radius;
	box.Ymin = pos->v[Y] - model->Radius;
	box.Ymax = pos->v[Y] + model->Radius;
	box.Zmin = pos->v[Z] - model->Radius;
	box.Zmax = pos->v[Z] + model->Radius;

// test against visicubes

	if (planet->VisiMask & CamVisiMask)
		return;

// skip if offscreen

	visflag = TestSphereToFrustum(pos, model->Radius, &z);
	if (visflag == SPHERE_OUT) return;
	if (visflag == SPHERE_IN) flag |= MODEL_DONOTCLIP;

// set visible flag

	obj->renderflag.visible = TRUE;

// in light?

	if (CheckObjectLight(pos, &box, model->Radius))
	{
		flag |= MODEL_LIT;
		AddModelLight(model, pos, mat);
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

	CheckModelMeshFx(model, mat, pos, &flag);

// draw model

	DrawModel(model, mat, pos, flag);

// fog off?

	if (flag & MODEL_FOG)
		FOG_OFF();
}

///////////////////////
// render museum sun //
///////////////////////

void RenderSun(OBJECT *obj)
{
	long i, starnum;
	REAL z, fog, zres, zbuf;
	MAT mat, mat2;
	VEC vec;
	SUN_OBJ *sun = (SUN_OBJ*)obj->Data;

// test against visicubes

	if (sun->VisiMask & CamVisiMask)
		return;

// skip if offscreen

	if (TestSphereToFrustum(&obj->body.Centre.Pos, 6144, &z) == SPHERE_OUT)
		return;

// yep, draw

	obj->renderflag.visible = TRUE;

// draw sun

	for (i = 0 ; i < SUN_OVERLAY_NUM ; i++)
	{
		RotMatrixZ(&mat, sun->Overlay[i].Rot);
		SunFacingPoly.RGB = sun->Overlay[i].rgb;
		DrawFacingPolyRot(&obj->body.Centre.Pos, &mat, &SunFacingPoly, 1, 0);
	}

// draw stars

	RotMatrixY(&mat2, (float)TIME2MS(CurrentTimer()) / 100000.0f);
	MulMatrix(&ViewMatrixScaled, &mat2, &mat);
	RotTransVector(&ViewMatrixScaled, &ViewTransScaled, &obj->body.Centre.Pos, &vec);

	zres = (float)(1 << ZedBufferFormat.dwZBufferBitDepth);
	zbuf = (zres - 1.0f) / zres;

	starnum = 0;

	for (i = 0 ; i < SUN_STAR_NUM ; i++)
	{
		z = sun->Star[i].Pos.v[X] * mat.m[RZ] + sun->Star[i].Pos.v[Y] * mat.m[UZ] + sun->Star[i].Pos.v[Z] * mat.m[LZ] + vec.v[Z];
		if (z < RenderSettings.NearClip || z >= RenderSettings.FarClip) continue;

		sun->Verts[starnum].sx = (sun->Star[i].Pos.v[X] * mat.m[RX] + sun->Star[i].Pos.v[Y] * mat.m[UX] + sun->Star[i].Pos.v[Z] * mat.m[LX] + vec.v[X]) / z + RenderSettings.GeomCentreX;
		sun->Verts[starnum].sy = (sun->Star[i].Pos.v[X] * mat.m[RY] + sun->Star[i].Pos.v[Y] * mat.m[UY] + sun->Star[i].Pos.v[Z] * mat.m[LY] + vec.v[Y]) / z + RenderSettings.GeomCentreY;

		sun->Verts[starnum].rhw = 1.0f;
		sun->Verts[starnum].sz = zbuf;

		sun->Verts[starnum].color = sun->Star[i].rgb;

		fog = (RenderSettings.FarClip - z) * RenderSettings.FogMul;
		if (fog > 255) fog = 255;
		else if (fog < 0) fog = 0;
		sun->Verts[starnum].specular = FTOL3(fog) << 24;

		starnum++;
	}

	FOG_ON();
	SET_TPAGE(-1);

	D3Ddevice->DrawPrimitive(D3DPT_POINTLIST, FVF_TEX0, sun->Verts, starnum, D3DDP_DONOTUPDATEEXTENTS);

	FOG_OFF();
}

//////////////////
// render plane //
//////////////////

void RenderPlane(OBJECT *obj)
{
	bool vis1, vis2;
	PLANE_OBJ *plane = (PLANE_OBJ*)obj->Data;

// render plane

	if (obj->DefaultModel != -1)
		vis1 = RenderObjectModel(&obj->body.Centre.WMatrix, &obj->body.Centre.Pos, &LevelModel[obj->DefaultModel].Model, obj->EnvRGB, obj->renderflag);
	else
		vis1 = FALSE;

// render propellor

	if (plane->PropModel != -1)
		vis2 = RenderObjectModel(&plane->PropMatrix, &plane->PropPos, &LevelModel[plane->PropModel].Model, obj->EnvRGB, obj->renderflag);
	else
		vis2 = FALSE;

// set visible flag

	obj->renderflag.visible |= (vis1 || vis2);
}

///////////////////
// render copter //
///////////////////

void RenderCopter(OBJECT *obj)
{
	bool vis1, vis2, vis3;
	COPTER_OBJ *copter = (COPTER_OBJ*)obj->Data;

// render copter

	if (obj->DefaultModel != -1)
		vis1 = RenderObjectModel(&obj->body.Centre.WMatrix, &obj->body.Centre.Pos, &LevelModel[obj->DefaultModel].Model, obj->EnvRGB, obj->renderflag);
	else
		vis1 = FALSE;

// render blade1

	if (copter->BladeModel1 != -1)
		vis2 = RenderObjectModel(&copter->BladeMatrix1, &copter->BladePos1, &LevelModel[copter->BladeModel1].Model, obj->EnvRGB, obj->renderflag);
	else
		vis2 = FALSE;

// render blade2

	if (copter->BladeModel2 != -1)
		vis3 = RenderObjectModel(&copter->BladeMatrix2, &copter->BladePos2, &LevelModel[copter->BladeModel2].Model, obj->EnvRGB, obj->renderflag);
	else
		vis3 = FALSE;

// set visible flag

	obj->renderflag.visible |= (vis1 || vis2 || vis3);

// draw bounding box

	//DrawBoundingBox(copter->FlyBox.XMin, copter->FlyBox.XMax, copter->FlyBox.YMin, copter->FlyBox.YMax, copter->FlyBox.ZMin, copter->FlyBox.ZMax, 0xff0000, 0x00ff00, 0x0000ff, 0x00ffff, 0xff00ff, 0xffff00);
}

///////////////////
// render dragon //
///////////////////

void RenderDragon(OBJECT *obj)
{
	bool vis1, vis2;
	long i;
	DRAGON_OBJ *dragon = (DRAGON_OBJ*)obj->Data;

// render body

	if (dragon->BodyModel != -1)
		vis1 = RenderObjectModel(&obj->body.Centre.WMatrix, &obj->body.Centre.Pos, &LevelModel[dragon->BodyModel].Model, obj->EnvRGB, obj->renderflag);
	else
		vis1 = FALSE;

// render head

	if (dragon->HeadModel != -1)
		vis2 = RenderObjectModel(&obj->body.Centre.WMatrix, &obj->body.Centre.Pos, &LevelModel[dragon->HeadModel].Model, obj->EnvRGB, obj->renderflag);
	else
		vis2 = FALSE;

// set visible flag

	obj->renderflag.visible |= (vis1 || vis2);

// draw fire

	for (i = 0 ; i < DRAGON_FIRE_NUM ; i++) if (dragon->Fire[i].Time)
	{
		DragonFireFacingPoly.Xsize = dragon->Fire[i].Size;
		DragonFireFacingPoly.Ysize = dragon->Fire[i].Size;
		DragonFireFacingPoly.RGB = dragon->Fire[i].rgb;
		DrawFacingPolyRot(&dragon->Fire[i].Pos, &dragon->Fire[i].Matrix, &DragonFireFacingPoly, 1, 0);
	}
}


/////////////////////////////////////////////////////////////////////
//
// DrawGridCollPolys:
//
/////////////////////////////////////////////////////////////////////

void DrawGridCollPolys(COLLGRID *grid)
{
	int iPoly;

	SET_TPAGE(-1);

	for (iPoly = 0; iPoly < grid->NCollPolys; iPoly++)
	{
		DrawCollPoly(grid->CollPolyPtr[iPoly]);
	}
}


/////////////////////////////////////////////////////////////////////
//
// RenderTrolley
//
/////////////////////////////////////////////////////////////////////

void RenderTrolley(OBJECT *obj)
{
	BuildCarMatricesNew(&obj->player->car);
	DrawCar(&obj->player->car);
}

//////////////////
// render train //
//////////////////

void RenderTrain(OBJECT *obj)
{
	TRAIN_OBJ *train = (TRAIN_OBJ*)obj->Data;
	MAT mat1, mat2;
	
// render train

	if (obj->DefaultModel != -1)
		obj->renderflag.visible |= RenderObjectModel(&obj->body.Centre.WMatrix, &obj->body.Centre.Pos, &LevelModel[obj->DefaultModel].Model, obj->EnvRGB, obj->renderflag);

// render wheels

	obj->renderflag.envmap = FALSE;

	if (train->FrontWheel)
	{
		RotMatrixX(&mat1, train->TimeFront);
		MulMatrix(&obj->body.Centre.WMatrix, &mat1, &mat2);

		RenderObjectModel(&mat2, &train->WheelPos[2], &LevelModel[train->FrontWheel].Model, obj->EnvRGB, obj->renderflag);
		RenderObjectModel(&mat2, &train->WheelPos[3], &LevelModel[train->FrontWheel].Model, obj->EnvRGB, obj->renderflag);
	}

	if (train->BackWheel)
	{
		RotMatrixX(&mat1, train->TimeBack);
		MulMatrix(&obj->body.Centre.WMatrix, &mat1, &mat2);

		RenderObjectModel(&mat2, &train->WheelPos[0], &LevelModel[train->BackWheel].Model, obj->EnvRGB, obj->renderflag);
		RenderObjectModel(&mat2, &train->WheelPos[1], &LevelModel[train->BackWheel].Model, obj->EnvRGB, obj->renderflag);
	}

	obj->renderflag.envmap = TRUE;
}

/////////////////////////
// render strobe light //
/////////////////////////

void RenderStrobe(OBJECT *obj)
{
	FACING_POLY poly;
	STROBE_OBJ *strobe = (STROBE_OBJ*)obj->Data;
	MAT mat;
	REAL ang;

// render model

	RenderObject(obj);

// draw glow?

	if (strobe->Glow && obj->Light && obj->renderflag.visible)
	{
		FOG_ON();

		poly.Xsize = poly.Ysize = strobe->Glow * 64.0f + 16.0f;
		poly.U = 128.0f / 256.0f;
		poly.V = 0.0f / 256.0f;
		poly.Usize = poly.Vsize = 64.0f / 256.0f;
		poly.Tpage = TPAGE_FX1;
		poly.RGB = obj->Light->r << 16 | obj->Light->g << 8 | obj->Light->b;

		DrawFacingPoly(&strobe->LightPos, &poly, 1, -256);

		poly.U = 192.0f / 256.0f;
		poly.V = 64.0f / 256.0f;

//		ang = -(float)atan(ViewMatrix.m[LZ] / ViewMatrix.m[LX]) / PI;
		ang = TIME2MS(CurrentTimer()) / 5000.0f;

		RotMatrixZ(&mat, ang);
		DrawFacingPolyRot(&strobe->LightPos, &mat, &poly, 1, -256);

		RotMatrixZ(&mat, ang * 2.0f);
		DrawFacingPolyRot(&strobe->LightPos, &mat, &poly, 1, -256);

		FOG_OFF();
	}
}

///////////////////
// render pickup //
///////////////////

void RenderPickup(OBJECT *obj)
{
	long col, i, alpha;
	MODEL *model = &LevelModel[obj->DefaultModel].Model;
	PICKUP_OBJ *pickup = (PICKUP_OBJ*)obj->Data;
	FACING_POLY poly;
	MAT mat;
	REAL size;

// act on mode

	switch (pickup->Mode)
	{

// waiting to gen

		case 0:
			return;

// waiting to be picked up

		case 1:

// draw model?

			if (pickup->Timer > 0.5f)
			{
				if (model->PolyPtr->Type & POLY_SEMITRANS)
				{
					for (i = 0 ; i < model->PolyNum ; i++)
					{
						model->PolyPtr[i].Type &= ~POLY_SEMITRANS;
					}
				}

				RenderObject(obj);
			}

// draw 'generation'

			if (pickup->Timer < 0.75f)
			{
				FTOL((float)sin(pickup->Timer * 4.0f / 3.0f * PI) * 255.0f, col);

				size = (float)sin(pickup->Timer * 4.0f / 3.0f * PI) * 64.0f + 32;

				poly.Xsize = poly.Ysize = size;

				poly.Usize = poly.Vsize = 64.0f / 256.0f;
				poly.Tpage = TPAGE_FX1;

				poly.U = 193.0f / 256.0f;
				poly.V = 64.0f / 256.0f;
				poly.RGB = (col >> 1) | (col << 8) | (col << 16);

				RotMatrixZ(&mat, pickup->Timer / 4.0f);
				DrawFacingPolyRotMirror(&obj->body.Centre.Pos, &mat, &poly, 1, -256);

				RotMatrixZ(&mat, pickup->Timer / 2.0f + 0.5f);
				DrawFacingPolyRotMirror(&obj->body.Centre.Pos, &mat, &poly, 1, -256);

				poly.U = 128.0f / 256.0f;
				poly.V = 0.0f / 256.0f;
				poly.RGB = (col) | (col << 8) | (col << 16);

				RotMatrixZ(&mat, 0.0f);
				DrawFacingPolyRotMirror(&obj->body.Centre.Pos, &mat, &poly, 1, -256);
			}

		break;

// disappearing

		case 2:

// set alpha

			FTOL(-pickup->Timer * 255.0f + 255.0f, alpha);

			for (i = 0 ; i < model->PolyNum ; i++)
			{
				model->PolyPtr[i].Type |= POLY_SEMITRANS;
				model->PolyRGB[i].rgb[0].a = (unsigned char)alpha;
				model->PolyRGB[i].rgb[1].a = (unsigned char)alpha;
				model->PolyRGB[i].rgb[2].a = (unsigned char)alpha;
				model->PolyRGB[i].rgb[3].a = (unsigned char)alpha;
			}

// draw model

			RenderObject(obj);

		break;
	}
}

///////////////////////////
// render dissolve model //
///////////////////////////

void RenderDissolveModel(OBJECT *obj)
{
	DISSOLVE_OBJ *dissolve = (DISSOLVE_OBJ*)obj->Data;

// render model

	obj->renderflag.visible |= RenderObjectModel(&obj->body.Centre.WMatrix, &obj->body.Centre.Pos, &dissolve->Model, dissolve->EnvRGB, obj->renderflag);
}

/////////////////////////////////////////////////////////////////////
// RenderLaser:
/////////////////////////////////////////////////////////////////////

void RenderLaser(OBJECT *obj)
{
	int ii, nBits;
	long col, size, visflag;
	REAL delta, bitLen, dRLen, ang, dx, dy, dt, dLen, widMod, z;
	VEC sPos, ePos, dR;
	MAT mat;
	FACING_POLY poly;
	VERTEX_TEX1 *vert;
	LASER_OBJ *laser = (LASER_OBJ *)obj->Data;

	// Check against visi-boxes
	if (CamVisiMask & laser->VisiMask) {
		return;
	}

	// Check against view frustum
	visflag = TestSphereToFrustum(&obj->body.Centre.Pos, obj->body.CollSkin.Radius, &z);
	if (visflag == SPHERE_OUT) {
		return;
	}

	// Laser is visible
	obj->renderflag.visible = TRUE;

	// Calculate the number of sections to split the laser into
	VecEqScalarVec(&dR, laser->Dist, &laser->Delta);
	dRLen = VecLen(&dR);
	if (dRLen > SMALL_REAL) {
		VecDivScalar(&dR, dRLen);
		nBits = 1 + (int)(dRLen / 100.0f);
		bitLen = dRLen / (REAL)nBits;
	} else {
		return;
	}
	
	// Calculate the end shifts
	RotTransPersVector(&ViewMatrixScaled, &ViewTransScaled, &obj->body.Centre.Pos, (REAL*)&DrawVertsTEX1[0]);
	RotTransPersVector(&ViewMatrixScaled, &ViewTransScaled, &laser->Dest, (REAL*)&DrawVertsTEX1[1]);
	dx = DrawVertsTEX1[1].sx - DrawVertsTEX1[0].sx;
	dy = DrawVertsTEX1[1].sy - DrawVertsTEX1[0].sy;
	dLen = (REAL)sqrt(dx * dx + dy * dy);
	dt = dx;
	dx = dy / dLen;
	dy = -dt / dLen;


	// Draw each bit
	CopyVec(&obj->body.Centre.Pos, &sPos);
	size = ((TIME2MS(TimerCurrent) + laser->Phase) % 100l);
	widMod = (REAL)size * laser->RandWidth / 100.0f;
	for (ii = 0; ii < nBits; ii++) {

		VecPlusScalarVec(&sPos, bitLen, &dR, &ePos);

		// Generate the polys
		if (!SEMI_POLY_FREE()) return;
		SEMI_POLY_SETUP(vert, FALSE, 4, TPAGE_FX1, TRUE, TRUE);

		vert[3].tu = 225.0f / 256.0f;
		vert[3].tv = 33.0f / 256.0f;

		vert[0].tu = 239.0f / 256.0f;
		vert[0].tv = 33.0f / 256.0f;

		vert[1].tu = 239.0f / 256.0f;
		vert[1].tv = 47.0f / 256.0f;

		vert[2].tu = 225.0f / 256.0f;
		vert[2].tv = 47.0f / 256.0f;

		// Transform src and dest into view coords
		RotTransPersVector(&ViewMatrixScaled, &ViewTransScaled, &sPos, (REAL*)&vert[0]);
		RotTransPersVector(&ViewMatrixScaled, &ViewTransScaled, &ePos, (REAL*)&vert[2]);

		delta = vert[0].rhw;
		vert[1].sx = vert[0].sx + dx * (laser->Width + widMod) * delta * RenderSettings.GeomPers;
		vert[1].sy = vert[0].sy + dy * (laser->Width + widMod) * delta * RenderSettings.GeomPers;
		vert[1].sz = vert[0].sz;
		vert[1].rhw = vert[0].rhw;
		vert[0].sx -= dx * (laser->Width + widMod) * delta * RenderSettings.GeomPers;
		vert[0].sy -= dy * (laser->Width + widMod) * delta * RenderSettings.GeomPers;

		delta = vert[2].rhw;
		vert[3].sx = vert[2].sx - dx * (laser->Width + widMod) * delta * RenderSettings.GeomPers;
		vert[3].sy = vert[2].sy - dy * (laser->Width + widMod) * delta * RenderSettings.GeomPers;
		vert[3].sz = vert[2].sz;
		vert[3].rhw = vert[2].rhw;
		vert[2].sx += dx * (laser->Width + widMod) * delta * RenderSettings.GeomPers;
		vert[2].sy += dy * (laser->Width + widMod) * delta * RenderSettings.GeomPers;

		vert[0].color = 0x888888;
		vert[1].color = 0x888888;
		vert[2].color = 0x888888;
		vert[3].color = 0x888888;

		VecPlusEqScalarVec(&sPos, bitLen, &dR);
	}

	FOG_ON();
	col = 0xff3333;
	poly.Usize = poly.Vsize = 64.0f / 256.0f;
	poly.Tpage = TPAGE_FX1;
	poly.RGB = (col >> 1) | (col << 8) | (col << 16);
	poly.U = 192.0f / 256.0f;
	poly.V = 64.0f / 256.0f;


	ang = TIME2MS(TimerCurrent) / 10000.0f;
	size = (long)((TIME2MS(TimerCurrent) % 100l) - 50l);
	poly.Xsize = poly.Ysize = (2 * laser->Width) + (size / 5);
	RotMatrixZ(&mat, ang);
	DrawFacingPolyRotMirror(&sPos, &mat, &poly, 1, -16);

	ang = TIME2MS(TimerCurrent) / 5000.0f;
	size = (long)((TIME2MS(TimerCurrent) % 200l) - 100l);
	poly.Xsize = poly.Ysize = (2 * laser->Width) + (size / 10);
	RotMatrixZ(&mat, ang);
	DrawFacingPolyRotMirror(&sPos, &mat, &poly, 1, -16);

	FOG_OFF();

	// draw the polys
	//DrawModel(&PLR_LocalPlayer->car.Models->Wheel[0][0], &Identity, &sPos, MODEL_PLAIN);

}

/////////////////////////////////////////////////////////////////////
//
// DrawTarget:
//
/////////////////////////////////////////////////////////////////////

void DrawTarget(PLAYER *player)
{
	long mod, col;
	REAL ang;
	MAT mat;
	FACING_POLY poly;

	FOG_ON();

	col = 0xff;
	poly.Usize = poly.Vsize = 64.0f / 256.0f;
	poly.Tpage = TPAGE_FX1;
	poly.RGB = (col >> 1) | (col << 8) | (col << 16);
	poly.U = 192.0f / 256.0f;
	poly.V = 64.0f / 256.0f;

	mod = TIME2MS(TimerCurrent) % 10000;
	ang = mod / Real(10000);
	poly.Xsize = poly.Ysize = Real(30);

	RotMatrixZ(&mat, ang);
	DrawFacingPolyRotMirror(&player->PickupTarget->player->car.Body->Centre.Pos, &mat, &poly, 1, -256);

	FOG_OFF();
}

///////////////////
// render splash //
///////////////////

void RenderSplash(OBJECT *obj)
{
	long i, frame, rgb;
	REAL tu, tv;
	SPLASH_OBJ *splash = (SPLASH_OBJ *)obj->Data;
	SPLASH_POLY *spoly;
	VERTEX_TEX1 *vert;

// loop thru all polys

	spoly = splash->Poly;
	for (i = 0 ; i < SPLASH_POLY_NUM ; i++, spoly++) if (spoly->Frame < 16.0f)
	{

// get semi poly

		if (!SEMI_POLY_FREE()) return;
		SEMI_POLY_SETUP(vert, FALSE, 4, TPAGE_FX3, TRUE, 1);

// transform poly

		RotTransPersVector(&ViewMatrixScaled, &ViewTransScaled, &spoly->Pos[0], &vert[0].sx);
		RotTransPersVector(&ViewMatrixScaled, &ViewTransScaled, &spoly->Pos[1], &vert[1].sx);
		RotTransPersVector(&ViewMatrixScaled, &ViewTransScaled, &spoly->Pos[2], &vert[2].sx);
		RotTransPersVector(&ViewMatrixScaled, &ViewTransScaled, &spoly->Pos[3], &vert[3].sx);

// setup tex + rgb

		FTOL(spoly->Frame, frame);

		tu = frame * 16.0f + 1.0f;
		tv = 1.0f;

		vert[0].tu = vert[3].tu = tu / 256.0f;
		vert[1].tu = vert[2].tu = (tu + 14.0f) / 256.0f;
		vert[0].tv = vert[1].tv = tv / 256.0f;
		vert[2].tv = vert[3].tv = (tv + 30.0f) / 256.0f;

		FTOL((16.0f - spoly->Frame) * 8.0f, rgb);
		rgb |= rgb << 8 | rgb << 16;
		vert[0].color = vert[1].color = vert[2].color = vert[3].color = rgb;
	}
}


/////////////////////////////////////////////////////////////////////
//
// RenderSpeedup
//
/////////////////////////////////////////////////////////////////////
void RenderSpeedup(OBJECT *obj)
{
	VEC sPos, ePos, pPos, vel, dR;
	PLAYER *player;
	bool playerNear = FALSE;

	SPEEDUP_OBJ *speedup = (SPEEDUP_OBJ *)obj->Data;

	RenderObjectModel(&obj->body.Centre.WMatrix, &speedup->PostPos[0], &LevelModel[obj->DefaultModel].Model, obj->EnvRGB, obj->renderflag);
	RenderObjectModel(&obj->body.Centre.WMatrix, &speedup->PostPos[1], &LevelModel[obj->DefaultModel].Model, obj->EnvRGB, obj->renderflag);

	if ((TimerCurrent % 1000ul) < 500) {
		speedup->HeightMod[0] += TimeStep * (frand(70) + 70.0f);
		speedup->HeightMod[1] += TimeStep * (frand(70) + 70.0f);
	} else {
		speedup->HeightMod[0] -= TimeStep * (frand(70) + 70.0f);
		speedup->HeightMod[1] -= TimeStep * (frand(70) + 70.0f);
	}
	if (speedup->HeightMod[0] > SPEEDUP_GEN_HEIGHT) speedup->HeightMod[0] = frand(2 * SPEEDUP_GEN_HEIGHT) - SPEEDUP_GEN_HEIGHT;
	if (speedup->HeightMod[0] < -SPEEDUP_GEN_HEIGHT) speedup->HeightMod[0] = frand(2 * SPEEDUP_GEN_HEIGHT) - SPEEDUP_GEN_HEIGHT;
	if (speedup->HeightMod[1] > SPEEDUP_GEN_HEIGHT) speedup->HeightMod[1] = frand(2 * SPEEDUP_GEN_HEIGHT) - SPEEDUP_GEN_HEIGHT;
	if (speedup->HeightMod[1] < -SPEEDUP_GEN_HEIGHT) speedup->HeightMod[1] = frand(2 * SPEEDUP_GEN_HEIGHT) - SPEEDUP_GEN_HEIGHT;

	ScalarVecPlusScalarVec(SPEEDUP_GEN_WIDTH, &obj->body.Centre.WMatrix.mv[R], -speedup->Height + speedup->HeightMod[0], &obj->body.Centre.WMatrix.mv[U], &sPos);
	VecPlusEqVec(&sPos, &speedup->PostPos[0]);
	ScalarVecPlusScalarVec(-SPEEDUP_GEN_WIDTH, &obj->body.Centre.WMatrix.mv[R], -speedup->Height + speedup->HeightMod[1], &obj->body.Centre.WMatrix.mv[U], &ePos);
	VecPlusEqVec(&ePos, &speedup->PostPos[1]);

	// Find the players near to the speedup and electrocute them
	for (player = PLR_PlayerHead; player != NULL; player = player->next) {

		CopyVec(&player->car.Aerial.Section[AERIAL_LASTSECTION].Pos, &pPos);
		VecMinusVec(&sPos, &pPos, &dR);
		if (VecDotVec(&dR, &dR) > 4 * speedup->Width * speedup->Width) continue;
		VecMinusVec(&ePos, &pPos, &dR);
		if (VecDotVec(&dR, &dR) > 4 * speedup->Width * speedup->Width) continue;
		
		playerNear = TRUE;
		DrawJumpSpark2(&sPos, &pPos);
		DrawJumpSpark2(&ePos, &pPos);
		DrawJumpSpark2(&player->car.Aerial.Section[2].Pos, &player->car.Aerial.Section[1].Pos);
		DrawJumpSpark2(&player->car.Aerial.Section[1].Pos, &player->car.Aerial.Section[0].Pos);

	}

	if (!playerNear) {
		DrawJumpSpark2(&sPos, &ePos);
	}


	VecEqScalarVec(&vel, 300, &obj->body.Centre.WMatrix.mv[R]);
	CreateSpark(SPARK_ELECTRIC, &sPos, &vel, 200, 0);
	NegateVec(&vel);
	CreateSpark(SPARK_ELECTRIC, &ePos, &vel, 200, 0);


}