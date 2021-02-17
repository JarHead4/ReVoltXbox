//-----------------------------------------------------------------------------
// File: DrawObj.cpp
//
// Desc: Header file for drawing objects like the car, wheels, and aerial.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"
#include "model.h"
#include "network.h"
#include "particle.h"
#include "aerial.h"
#include "NewColl.h"
#include "body.h"
#include "main.h"
#include "wheel.h"
#include "car.h"
#include "DrawObj.h"
#include "shadow.h"
#include "light.h"
#include "draw.h"
#include "geom.h"
#include "camera.h"
#include "dx.h"
#include "input.h"
#include "ctrlread.h"
#include "object.h"
#include "control.h"
#include "player.h"
#include "visibox.h"
#include "timing.h"
#include "settings.h"
#include "mirror.h"
#include "ai.h"
#include "ghost.h"
#include "text.h"
#include "spark.h"
#include "obj_init.h"
#include "weapon.h"
#include "TitleScreen.h"
#include "Menu2.h"
#include "pickup.h"
#include "field.h"
#include "panel.h"
#include "gameloop.h"
#include "MenuDraw.h"

// prototypes

void DrawTarget(PLAYER *player);
void PulseSpeedupModelGourad(OBJECT *obj);
void RenderPickup(PICKUP *pickup);


// globals
extern bool GHO_ShowGhost;

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
    -52, 52, 121, -138, -8,
    128, 0, 32, 64,
// col
    -40, 40, 85, -90, -8,
    160, 0, 32, 64,
// harvester
    -44, 44, 94, -83, -8,
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
    -64, 64, 132, -119, -8,
    96, 64, 32, 64,
// evil weasil
    -40, 40, 82, -73, -8,
    64, 0, 32, 64,
// panga
    -38, 38, 94, -67, -8,
    32, 128, 32, 64,
// r5
    -34, 34, 56, -74, -8,
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
// special 1 - rotor
    -70, 70, 72, -66, -8,
    32, 192, 32, 64,
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
// panga 2
    -74, 74, 85, -86, -8,
    32, 192, 32, 64,
// trolley
    -82, 82, 198, -177, -8,
    64, 128, 32, 64,
// key 1
    -28, 28, 47, -42, -8,
    32, 192, 32, 64,
// key 2
    -28, 28, 41, -44, -8,
    32, 192, 32, 64,
// key 3
    -28, 28, 44, -40, -8,
    32, 192, 32, 64,
// key 4
    -28, 28, 44, -40, -8,
    32, 192, 32, 64,
// ufo
    -52, 52, 52, -52, -8,
    66, 194, 61, 61,
// mystery
    -41, 41, 84, -81, -8,
    160, 128, 32, 64,
// extra
    -0, 0, 0, -0, -8,
    225, 129, 30, 62,
};

// skybox verts

#define SKYBOX_SIZE 1000.0f

VEC SkyboxVerts[] = {
    -SKYBOX_SIZE, -SKYBOX_SIZE, SKYBOX_SIZE,
    SKYBOX_SIZE, -SKYBOX_SIZE, SKYBOX_SIZE,
    -SKYBOX_SIZE, SKYBOX_SIZE, SKYBOX_SIZE,
    SKYBOX_SIZE, SKYBOX_SIZE, SKYBOX_SIZE,
    -SKYBOX_SIZE, -SKYBOX_SIZE, -SKYBOX_SIZE,
    SKYBOX_SIZE, -SKYBOX_SIZE, -SKYBOX_SIZE,
    -SKYBOX_SIZE, SKYBOX_SIZE, -SKYBOX_SIZE,
    SKYBOX_SIZE, SKYBOX_SIZE, -SKYBOX_SIZE,
};

long SkyboxIndex[6][4] = {
//  4, 5, 7, 6,     // far
//  5, 1, 3, 7,     // right
//  1, 0, 2, 3,     // near
//  0, 4, 6, 2,     // left
//  0, 1, 5, 4,     // top
//  6, 7, 3, 2,     // bottom

    1, 0, 2, 3,     // near
    0, 4, 6, 2,     // left
    4, 5, 7, 6,     // far
    5, 1, 3, 7,     // right
    5, 4, 0, 1,     // top
    3, 2, 6, 7,     // bottom
};

// Motion blur plygons
#define MOTION_BLUR_MAX     (AERIAL_NTOTSECTIONS * MAX_NUM_PLAYERS)
int gcMotionBlur;
VEC gMotionBlurCoords[MOTION_BLUR_MAX][4];
long gMotionBlurColors[MOTION_BLUR_MAX][2];

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

    GhostSineCount += TimeStep * 3.6f;
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

        //VecPlusScalarVec(&car->WheelOffset[iWheel], wheel->Pos, &DownVec, &fixedPos);
        SetVec(&fixedPos, car->WheelOffset[iWheel].v[X], car->WheelOffset[iWheel].v[Y] + wheel->Pos, car->WheelOffset[iWheel].v[Z]);

// set spring world matrix + pos

        if (CarHasSpring(car, iWheel))
        {
            BuildLookMatrixDown(&car->SuspOffset[iWheel], &fixedPos, &tmpMat);
            SubVector(&fixedPos, &car->SuspOffset[iWheel], &tmpVec);
            scale = Length(&tmpVec) / susp->SpringLen;
            VecMulScalar(&tmpMat.mv[U], scale / car->DrawScale);

            MulMatrix(&car->Body->Centre.WMatrix, &tmpMat, &susp->SpringCarMatrix);
            RotTransVector(&car->Body->Centre.WMatrix, &car->Body->Centre.Pos, &car->SuspOffset[iWheel], &susp->SpringWorldPos);
        }

// set axle world matrix + pos

        if (CarHasAxle(car, iWheel))
        {
            BuildLookMatrixForward(&car->AxleOffset[iWheel], &fixedPos, &tmpMat);
            SubVector(&fixedPos, &car->AxleOffset[iWheel], &tmpVec);
            scale = Length(&tmpVec) / susp->AxleLen;
            VecMulScalar(&tmpMat.mv[L], scale / car->DrawScale);

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
            if (car->CarType != CARID_PANGA) {
                BuildRotation3D(car->Spinner.Axis.v[X], car->Spinner.Axis.v[Y], car->Spinner.Axis.v[Z], TimeStep * car->Spinner.AngVel, &tmpMat);
                MatMulTransMat(&tmpMat, &car->Spinner.Matrix, &tmpMat2);
                TransMat(&tmpMat2, &car->Spinner.Matrix);
                MulMatrix(&car->Body->Centre.WMatrix, &car->Spinner.Matrix, &car->Spinner.CarMatrix);
                RotTransVector(&car->Body->Centre.WMatrix, &car->Body->Centre.Pos, &car->Models->OffSpinner, &car->Spinner.WorldPos);
            } else {
                REAL scale;
                VEC headPos;
                BuildRotation3D(car->Spinner.Axis.v[X], car->Spinner.Axis.v[Y], car->Spinner.Axis.v[Z], 1.5f * car->Wheel[FL].TurnAngle, &tmpMat);
                CopyMat(&tmpMat, &car->Spinner.Matrix);
                MulMatrix(&car->Body->Centre.WMatrix, &car->Spinner.Matrix, &car->Spinner.CarMatrix);

                scale = VecDotVec(&car->Body->Centre.Vel, &car->Body->Centre.WMatrix.mv[L]) * 0.001f;
                if (abs(scale) > ONE) scale /= abs(scale);
                headPos.v[X] = car->Models->OffSpinner.v[X];
                headPos.v[Y] = car->Models->OffSpinner.v[Y] + scale * 3;
                headPos.v[Z] = car->Models->OffSpinner.v[Z] + scale * 6;
                RotTransVector(&car->Body->Centre.WMatrix, &car->Body->Centre.Pos, &headPos, &car->Spinner.WorldPos);
            }
        }

// if car is glow class, create some smoke

/*      if (car->Class == CAR_CLASS_GLOW) 
        {
            VEC wPos, pos = {0, 0, -50};
            VecMulMat(&pos, &car->Body->Centre.WMatrix, &wPos);
            VecPlusEqVec(&wPos, &car->Body->Centre.Pos);
            CreateSpark(SPARK_SMOKE1, &wPos, &ZeroVector, 100, 0);
            CreateSpark(SPARK_SMOKE1, &wPos, &ZeroVector, 100, 0);
            CreateSpark(SPARK_SMOKE1, &wPos, &ZeroVector, 100, 0);
        }
*/

    }
}

///////////////////
// draw all cars //
///////////////////

void DrawAllCars(void)
{
    PLAYER *player;
    VEC vec;

// loop thru cars

    for (player = PLR_PlayerHead ; player ; player = player->next)
    {

// skip?

        if (player->type == PLAYER_NONE || player->type == PLAYER_GHOST)
            continue;

// draw target

        if (player->PickupTarget != NULL) 
        {
            DrawTarget(player);
        }

// skip draw car if incar camera

        if (Camera[CameraCount].Object == player->ownobj && Camera[CameraCount].Type == CAM_ATTACHED && Camera[CameraCount].SubType == CAM_ATTACHED_INCAR)
        {
            SubVector(&Camera[CameraCount].DestOffset, &Camera[CameraCount].PosOffset, &vec);
            if (vec.v[X] < 4.0f && vec.v[Y] < 4.0f && vec.v[Z] < 4.0f)
                continue;
        }

// draw car

        DrawCar(player);
    }
}

/////////////////////////
// draw all ghost cars //
/////////////////////////

void DrawAllGhostCars(void)
{
    PLAYER *player;
    VEC vec;

// set render states

    ZWRITE_ON();
    BLEND_ALPHA();
    BLEND_SRC(D3DBLEND_SRCALPHA);
    BLEND_DEST(D3DBLEND_INVSRCALPHA);

// draw cars

    for (player = PLR_PlayerHead ; player ; player = player->next)
    {

// skip?

        if (player->type == PLAYER_NONE || player->car.RenderFlag != CAR_RENDER_GHOST)
            continue;


// skip if we're not allowed to draw ghosts

        if ((player->type == PLAYER_GHOST) && !GHO_ShowGhost) continue;


// draw target

        if (player->PickupTarget != NULL) 
        {
            DrawTarget(player);
        }

// skip draw car if incar camera

        if (Camera[CameraCount].Object == player->ownobj && Camera[CameraCount].Type == CAM_ATTACHED && Camera[CameraCount].SubType == CAM_ATTACHED_INCAR)
        {
            SubVector(&Camera[CameraCount].DestOffset, &Camera[CameraCount].PosOffset, &vec);
            if (vec.v[X] < 4.0f && vec.v[Y] < 4.0f && vec.v[Z] < 4.0f)
                continue;
        }

// draw car

        DrawCarGhost(player);
    }

    FlushPolyBuckets();
}


//$ADDITION(jedl) - short circuit all the local lighting and transform and fog tests

// $TODO change rendering order and add more passes for reflections

// $TODO For efficiency, compute all the matrices, then do one big
//  draw call. Currently, the DrawModelGPU call is simply concatenating
//  the matrix and then calling the Effect.

//////////////
// draw car using the GPU
//////////////

void DrawCarGPU(CAR *car)
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
//$MODIFIED:
//    FTOL(flod, lod);
    long lTemp;
    FTOL(flod, lTemp);
    lod = (char)lTemp;
    //$CMP_NOTE: this ugliness is just to avoid a compile warning (implicit long->char conversion; possible loss of data)

    lod2 = lod;
    while (!car->Models->Body[lod2].PolyNum) lod2--;
    // if (flag & MODEL_LIT) AddModelLight(&car->Models->Body[lod2], &car->BodyWorldPos, &car->Body->Centre.WMatrix);
    DrawModelGPU(&car->Models->Body[lod2], &car->Body->Centre.WMatrix, &car->BodyWorldPos, flag | MODEL_ENV);

    for (ii = 0; ii < CAR_NWHEELS; ii++)
    {
        // draw wheel
        if (IsWheelPresent(&car->Wheel[ii]))
        {
            lod2 = lod;
            while (!car->Models->Wheel[ii][lod2].PolyNum) lod2--;
            // if (flag & MODEL_LIT) AddModelLight(&car->Models->Wheel[ii][lod2], &car->Wheel[ii].WPos, &car->Wheel[ii].WMatrix);
            DrawModelGPU(&car->Models->Wheel[ii][lod2], &car->Wheel[ii].WMatrix, &car->Wheel[ii].WPos, flag);
        }

        // draw spring
        if (CarHasSpring(car, ii))
        {
            lod2 = lod;
            while (!car->Models->Spring[ii][lod2].PolyNum) lod2--;
            // if (flag & MODEL_LIT) AddModelLightSimple(&car->Models->Spring[ii][lod2], &car->Sus[ii].SpringWorldPos);
            DrawModelGPU(&car->Models->Spring[ii][lod2], &car->Sus[ii].SpringCarMatrix, &car->Sus[ii].SpringWorldPos, flag);
        }

        // draw axle
        if (CarHasAxle(car, ii))
        {
            lod2 = lod;
            while (!car->Models->Axle[ii][lod2].PolyNum) lod2--;
            // if (flag & MODEL_LIT) AddModelLightSimple(&car->Models->Axle[ii][lod2], &car->Sus[ii].AxleWorldPos);
            DrawModelGPU(&car->Models->Axle[ii][lod2], &car->Sus[ii].AxleCarMatrix, &car->Sus[ii].AxleWorldPos, flag);
        }

        // draw pin
        if (CarHasPin(car, ii))
        {
            lod2 = lod;
            while (!car->Models->Pin[ii][lod2].PolyNum) lod2--;
            // if (flag & MODEL_LIT) AddModelLightSimple(&car->Models->Pin[ii][lod2], &car->Sus[ii].PinWorldPos);
            DrawModelGPU(&car->Models->Pin[ii][lod2], &car->Sus[ii].PinCarMatrix, &car->Sus[ii].PinWorldPos, flag);
        }
    }

    // draw spinner
    if (CarHasSpinner(car))
    {
        lod2 = lod;
        while (!car->Models->Spinner[lod2].PolyNum) lod2--;
        // if (flag & MODEL_LIT) AddModelLightSimple(&car->Models->Spinner[lod2], &car->Spinner.WorldPos);
        DrawModelGPU(&car->Models->Spinner[lod2], &car->Spinner.CarMatrix, &car->Spinner.WorldPos, flag);
    }

    // draw aerial
    if (CarHasAerial(car)) 
    {
        void DrawCarAerial2GPU(AERIAL *aerial, MODEL *secModel, MODEL *topModel, short flag);
        DrawCarAerial2GPU(&car->Aerial, car->Models->Aerial[0], car->Models->Aerial[1], flag);
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
}
//$END_ADDITION


//////////////
// draw car //
//////////////

void DrawCar(PLAYER *player)
{
    CAR *car = &player->car;

//$ADDITION(jedl) - short circuit all the local lighting and transform and fog tests
    if( RegistrySettings.bUseGPU
        && car->Models->Body[0].m_pEffect != NULL )
    {
        // TODO: lights, glare, fx, mirror
        DrawCarGPU(car);
        return;
    }
//$END_ADDITION

    long ii, visflag, envrgb = car->Models->EnvRGB;
    REAL z, flod;
    BOUNDING_BOX box;
    short flag = MODEL_PLAIN;
    char lod, lod2;
    REAL rad;

// zero car rendered flag

    car->Rendered = FALSE;

// set rad

    rad = (player->ownobj->Type == OBJECT_TYPE_TROLLEY ? CAR_RADIUS * 3.0f : CAR_RADIUS);

// set whole car bounding box

    box.Xmin = car->Body->Centre.Pos.v[X] - rad;
    box.Xmax = car->Body->Centre.Pos.v[X] + rad;
    box.Ymin = car->Body->Centre.Pos.v[Y] - rad;
    box.Ymax = car->Body->Centre.Pos.v[Y] + rad;
    box.Zmin = car->Body->Centre.Pos.v[Z] - rad;
    box.Zmax = car->Body->Centre.Pos.v[Z] + rad;

// test against visicubes

    if (TestObjectVisiboxes(&box))
        return;

// skip if offscreen

    visflag = TestSphereToFrustum(&car->Body->Centre.Pos, rad, &z);
    if (visflag == SPHERE_OUT) return;
    if (visflag == SPHERE_IN) flag |= MODEL_DONOTCLIP;
    if (z - rad < RenderSettings.NearClip) flag |= MODEL_NEARCLIP;

// set car rendered flag

    car->Rendered = TRUE;
    car->RenderedAll = TRUE;

// calc lod

    flod = z / CAR_LOD_BIAS - 2;
    if (flod < 0) flod = 0;
    if (flod > MAX_CAR_LOD - 1) flod = MAX_CAR_LOD - 1;
//$MODIFIED
//    FTOL(flod, lod);
    long lTemp;
    FTOL(flod, lTemp);
    lod = (char)lTemp;
    //$CMP_NOTE: this ugliness is just to avoid a compile warning (implicit long->char conversion; possible loss of data)
    /// Eventually, we might want to use "#pragma warning" instead, or just get rid of FTOL macro.
//$END_MODIFICATIONS

// in fog?

    if (z + rad > RenderSettings.FogStart && DxState.Fog)
    {
        ModelVertFog = (car->Body->Centre.Pos.v[1] - RenderSettings.VertFogStart) * RenderSettings.VertFogMul;
        if (ModelVertFog < 0) ModelVertFog = 0;
        if (ModelVertFog > 255) ModelVertFog = 255;

        flag |= MODEL_FOG;
        FOG_ON();
    }

// in light?

    if (CheckObjectLight(&car->Body->Centre.Pos, &box, rad))
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

// reflection only if not ghost, but renders as one

    if (player->type != PLAYER_GHOST && player->car.RenderFlag == CAR_RENDER_GHOST)
    {
        if (!(flag & MODEL_MIRROR))
            return;

        flag |= MODEL_NODRAW;
    }

// draw models

    if (CAM_MainCamera->Type == CAM_FOLLOW && CAM_MainCamera->SubType == CAM_FOLLOW_ROTATE)
    {
        SetEnvStatic(&car->Body->Centre.Pos, &car->Body->Centre.WMatrix, envrgb, 0.0f, 0.0f, 1.0f);
        EnvTpage = TPAGE_ENVROLL;
    }
    else
    {
        SetEnvActive(&car->Body->Centre.Pos, &car->Body->Centre.WMatrix, &car->EnvMatrix, envrgb, 0.0f, 0.0f, 1.0f);
    }

    lod2 = lod;
    while (!car->Models->Body[lod2].PolyNum) lod2--;
    if (flag & MODEL_LIT) AddModelLight(&car->Models->Body[lod2], &car->BodyWorldPos, &car->Body->Centre.WMatrix);
    DrawModel(&car->Models->Body[lod2], &car->Body->Centre.WMatrix, &car->BodyWorldPos, flag | MODEL_ENV);

// loop thru possible wheels

    for (ii = 0; ii < CAR_NWHEELS; ii++)
    {

// draw wheel

        if (CarHasWheel(car, ii))
        {
            lod2 = lod;
            while (!car->Models->Wheel[ii][lod2].PolyNum) lod2--;
            if (flag & MODEL_LIT) AddModelLight(&car->Models->Wheel[ii][lod2], &car->Wheel[ii].WPos, &car->Wheel[ii].WMatrix);
            if (!WheelRenderType)
            {
                DrawModel(&car->Models->Wheel[ii][lod2], &car->Wheel[ii].WMatrix, &car->Wheel[ii].WPos, flag);
            }
            else
            {
                FACING_POLY fp;
                fp.RGB = 0xffffffff;
                fp.Tpage = TPAGE_ENVSTILL;
                fp.U = fp.V = 0.0f;
                fp.Usize = fp.Vsize = 1.0f;
                fp.Xsize = fp.Ysize = car->Wheel[ii].Radius;

                MAT mat, mat2, mat3;
                BuildLookMatrixForward(&car->Wheel[ii].WPos, &ViewCameraPos, &mat);
                TransposeMatrix(&mat, &mat2);
                RotMatrixY(&mat, 0.25f);
                MulMatrix(&car->Wheel[ii].WMatrix, &mat, &mat3);
                MulMatrix(&mat2, &mat3, &mat);
                
                DrawFacingPolyRotMirror(&car->Wheel[ii].WPos, &mat, &fp, -1, 0.0f);
            }
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
        if (flag & MODEL_LIT) AddModelLight(&car->Models->Spinner[lod2], &car->Spinner.WorldPos, &car->Spinner.CarMatrix);
        DrawModel(&car->Models->Spinner[lod2], &car->Spinner.CarMatrix, &car->Spinner.WorldPos, flag);
    }

// show physics info?

#if CHRIS_EXTRAS
    if (CAR_DrawCarAxes) {
        DrawAxis(&car->Body->Centre.WMatrix, &car->Body->Centre.Pos);
    }
    if (CAR_DrawCarBBoxes) {
        BLEND_ON();
        BLEND_SRC(D3DBLEND_ONE);
        BLEND_DEST(D3DBLEND_ONE);

        DrawCarBoundingBoxes(car);

        BLEND_OFF();
    }
#endif

// draw aerial

    if (CarHasAerial(car)) 
    {
        DrawCarAerial2(&car->Aerial, car->Models->Aerial[0], car->Models->Aerial[1], flag & ~MODEL_SCALE);
    }

// reset render states?

    if (flag & MODEL_FOG)
        FOG_OFF();
}

////////////////////
// draw ghost car //
////////////////////

void DrawCarGhost(PLAYER *player)
{
    CAR *car = &player->car;

//$ADDITION(jedl) - short circuit all the local lighting and transform and fog tests
    if( RegistrySettings.bUseGPU
        && car->Models->Body[0].m_pEffect != NULL )
    {
        // $TODO jedl - emulate ghost vertex tweaking with new vertex shader.
        // For now, use the same rendering pathway as the regular car.
        // $TODO: lights, glare, fx, mirror
        DrawCarGPU(car);
        return;
    }
//$END_ADDITION

    long ii, visflag;
    REAL z, flod;
    BOUNDING_BOX box;
    short flag = MODEL_GHOST;
    char lod, lod2;
    MODEL_RGB envrgb;

// set ghost shit

    if (!player->car.RepositionTimer)
    {
        ModelGhostSinePos = GhostSinePos;
        ModelGhostFadeMul = 2.0f;
    }
    else
    {
        ModelGhostSinePos = -100.0f;
        if (!player->car.RepositionHalf)
            ModelGhostFadeMul = (CAR_REPOS_TIMER - player->car.RepositionTimer) * 6.0f;
        else
            ModelGhostFadeMul = player->car.RepositionTimer * 6.0f;
    }

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
    if (z - CAR_RADIUS < RenderSettings.NearClip) flag |= MODEL_NEARCLIP;

// set car rendered flag

    car->Rendered = TRUE;
    car->RenderedAll = TRUE;

// calc lod

    flod = z / CAR_LOD_BIAS - 2;
    if (flod < 0) flod = 0;
    if (flod > MAX_CAR_LOD - 1) flod = MAX_CAR_LOD - 1;
//$MODIFIED
//    FTOL(flod, lod);
    long lTemp;
    FTOL(flod, lTemp);
    lod = (char)lTemp;
    //$CMP_NOTE: this ugliness is just to avoid a compile warning (implicit long->char conversion; possible loss of data)
    /// Eventually, we might want to use "#pragma warning" instead, or just get rid of FTOL macro.
//$END_MODIFICATIONS

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

    if (car->RepositionTimer) if (CheckObjectLight(&car->Body->Centre.Pos, &box, CAR_RADIUS))
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

/*  if (RenderSettings.Mirror)
    {
        if (GetMirrorPlane(&car->Body->Centre.Pos))
        {
            if (ViewCameraPos.v[Y] < MirrorHeight)
                flag |= MODEL_MIRROR;
        }
    }*/

// draw models

    ModelGhostSineOffset = car->BodyOffset.v[Z];
    lod2 = lod;
    while (!car->Models->Body[lod2].PolyNum) lod2--;

// normal ghost

    if (!car->RepositionTimer)
    {
        DrawModel(&car->Models->Body[lod2], &car->Body->Centre.WMatrix, &car->BodyWorldPos, flag);
    }

// repos car

    else
    {
        FTOL(abs(car->RepositionTimer - 1.0f) * 512.0f, ii);
        ii -= 256;
        if (ii < 0) ii = 0;

        envrgb.r = ((MODEL_RGB*)&car->Models->EnvRGB)->r * ii / 256;
        envrgb.g = ((MODEL_RGB*)&car->Models->EnvRGB)->g * ii / 256;
        envrgb.b = ((MODEL_RGB*)&car->Models->EnvRGB)->b * ii / 256;

        SetEnvActive(&car->Body->Centre.Pos, &car->Body->Centre.WMatrix, &car->EnvMatrix, *(long*)&envrgb, 0.0f, 0.0f, 1.0f);
        if (flag & MODEL_LIT) AddModelLight(&car->Models->Body[lod2], &car->BodyWorldPos, &car->Body->Centre.WMatrix);
        DrawModel(&car->Models->Body[lod2], &car->Body->Centre.WMatrix, &car->BodyWorldPos, flag | MODEL_ENV);
    }

// loop thru possible wheels

    for (ii = 0; ii < CAR_NWHEELS; ii++)
    {

// draw wheel

        if (CarHasWheel(car, ii))
        {
            ModelGhostSineOffset = car->WheelOffset[ii].v[Z];
            lod2 = lod;
            while (!car->Models->Wheel[ii][lod2].PolyNum) lod2--;
            if (flag & MODEL_LIT) AddModelLight(&car->Models->Wheel[ii][lod2], &car->Wheel[ii].WPos, &car->Wheel[ii].WMatrix);
            DrawModel(&car->Models->Wheel[ii][lod2], &car->Wheel[ii].WMatrix, &car->Wheel[ii].WPos, flag);
        }

// draw spring

        if (CarHasSpring(car, ii))
        {
            ModelGhostSineOffset = car->SuspOffset[ii].v[Z];
            lod2 = lod;
            while (!car->Models->Spring[ii][lod2].PolyNum) lod2--;
            if (flag & MODEL_LIT) AddModelLightSimple(&car->Models->Spring[ii][lod2], &car->Sus[ii].SpringWorldPos);
            DrawModel(&car->Models->Spring[ii][lod2], &car->Sus[ii].SpringCarMatrix, &car->Sus[ii].SpringWorldPos, flag);
        }

// draw axle

        if (CarHasAxle(car, ii))
        {
            ModelGhostSineOffset = car->AxleOffset[ii].v[Z];
            lod2 = lod;
            while (!car->Models->Axle[ii][lod2].PolyNum) lod2--;
            if (flag & MODEL_LIT) AddModelLightSimple(&car->Models->Axle[ii][lod2], &car->Sus[ii].AxleWorldPos);
            DrawModel(&car->Models->Axle[ii][lod2], &car->Sus[ii].AxleCarMatrix, &car->Sus[ii].AxleWorldPos, flag);
        }

// draw pin

        if (CarHasPin(car, ii))
        {
            ModelGhostSineOffset = car->SuspOffset[ii].v[Z];
            lod2 = lod;
            while (!car->Models->Pin[ii][lod2].PolyNum) lod2--;
            if (flag & MODEL_LIT) AddModelLightSimple(&car->Models->Pin[ii][lod2], &car->Sus[ii].PinWorldPos);
            DrawModel(&car->Models->Pin[ii][lod2], &car->Sus[ii].PinCarMatrix, &car->Sus[ii].PinWorldPos, flag);
        }
    }

// draw spinner

    if (CarHasSpinner(car))
    {
        ModelGhostSineOffset = car->SpinnerOffset.v[Z];
        lod2 = lod;
        while (!car->Models->Spinner[lod2].PolyNum) lod2--;
        if (flag & MODEL_LIT) AddModelLight(&car->Models->Spinner[lod2], &car->Spinner.WorldPos, &car->Spinner.CarMatrix);
        DrawModel(&car->Models->Spinner[lod2], &car->Spinner.CarMatrix, &car->Spinner.WorldPos, flag);
    }

// show physics info?

#if CHRIS_EXTRAS
    if (CAR_DrawCarAxes) {
        DrawAxis(&car->Body->Centre.WMatrix, &car->Body->Centre.Pos);
    }
    if (CAR_DrawCarBBoxes) {
        BLEND_ON();
        BLEND_SRC(D3DBLEND_ONE);
        BLEND_DEST(D3DBLEND_ONE);
        ZWRITE_OFF();

        DrawCarBoundingBoxes(car);

        ZWRITE_ON();
        BLEND_OFF();
    }
#endif

// draw aerial

    ModelGhostSineOffset = car->AerialOffset.v[Z];
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
    VEC vec, s0, s1, s2, s3;
    VEC p0, p1, p2, p3;
    CAR *car;
    PLAYER *player;
    CAR_SHADOW_TABLE *sh;
    long rgb, a;

// not if shadows off

    if (!RenderSettings.Shadow)
        return;

// set render states

    if (BlendSubtract)
    {
        BLEND_ON();
        BLEND_SRC(D3DBLEND_ZERO);
        BLEND_DEST(D3DBLEND_INVSRCCOLOR);
    }
    else
    {
        BLEND_ALPHA();
        BLEND_SRC(D3DBLEND_SRCALPHA);
        BLEND_DEST(D3DBLEND_INVSRCALPHA);
    }

    ZWRITE_OFF();
    SET_TPAGE(TPAGE_SHADOW);

// draw all visible car shadows

    for (player = PLR_PlayerHead ; player ; player = player->next) if (player->type != PLAYER_NONE)
    {
        if (Camera[CameraCount].Object == player->ownobj && Camera[CameraCount].Type == CAM_ATTACHED && Camera[CameraCount].SubType == CAM_ATTACHED_INCAR)
        {
            SubVector(&Camera[CameraCount].DestOffset, &Camera[CameraCount].PosOffset, &vec);
            if (vec.v[X] < 4.0f && vec.v[Y] < 4.0f && vec.v[Z] < 4.0f)
                continue;
        }

        if (GameSettings.Level == LEVEL_FRONTEND && player->car.CarType >= CARID_KEY1 && player->car.CarType <= CARID_KEY3)
            continue;

        car = &player->car;

        if (car->CarType < CARID_NTYPES)
        {
            sh = &CarShadowTable[car->CarType];
        }
        else
        {
            sh = &CarShadowTable[CARID_NTYPES];
            sh->Back = car->Models->Body[0].Zmin;
            sh->Front = car->Models->Body[0].Zmax;
            sh->Left = car->Models->Body[0].Xmin;
            sh->Right = car->Models->Body[0].Xmax;
        }

        if (car->Rendered)
        {
            SetVector(&s0, sh->Left, sh->Height, sh->Front);
            SetVector(&s1, sh->Right, sh->Height, sh->Front);
            SetVector(&s2, sh->Right, sh->Height, sh->Back);
            SetVector(&s3, sh->Left, sh->Height, sh->Back);

            VecMulScalar(&s0, car->DrawScale);
            VecMulScalar(&s1, car->DrawScale);
            VecMulScalar(&s2, car->DrawScale);
            VecMulScalar(&s3, car->DrawScale);

            RotTransVector(&car->Body->Centre.WMatrix, &car->Body->Centre.Pos, &s0, &p0);
            RotTransVector(&car->Body->Centre.WMatrix, &car->Body->Centre.Pos, &s1, &p1);
            RotTransVector(&car->Body->Centre.WMatrix, &car->Body->Centre.Pos, &s2, &p2);
            RotTransVector(&car->Body->Centre.WMatrix, &car->Body->Centre.Pos, &s3, &p3);

            if (car->RepositionTimer)
            {
                if (!car->RepositionHalf)
                {
                    FTOL((car->RepositionTimer - 1.0f) * 128.0f, a);
                }
                else
                {
                    FTOL((1.0f - car->RepositionTimer) * 128.0f, a);
                }
            }
            else
            {
                a = 128;
            }

            if (BlendSubtract)
                rgb = a | a << 8 | a << 16;
            else
                rgb = a << 24;;

            DrawShadow(&p0, &p1, &p2, &p3, sh->tu / 256.0f, sh->tv / 256.0f, sh->twidth / 256.0f, sh->theight / 256.0f, rgb, -2.0f, 0.0f, -1, TPAGE_SHADOW, NULL);

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
            DrawWideGameText(128, 128, 0xff00ff, buf);
#endif
        }
    }
}


//$ADDITION(jedl)
//-----------------------------------------------------------------------------
// Name: DrawCarAerial2GPU
// Desc: Draws the car's aerial using the GPU.
//-----------------------------------------------------------------------------
void DrawCarAerial2GPU(AERIAL *aerial, MODEL *secModel, MODEL *topModel, short flag)
{
    int iSec;
    VEC thisPos;
    VEC lastPos;
    MAT wMatrix;

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
            //if (flag & MODEL_LIT) {
            //    AddModelLightSimple(secModel, &thisPos);
            //}
            DrawModelGPU(secModel, &wMatrix, &thisPos, flag);
        } else {
            //if (flag & MODEL_LIT) {
            //    AddModelLightSimple(topModel, &thisPos);
            //}
            DrawModelGPU(topModel, &wMatrix, &thisPos, flag);
        }

        CopyVec(&thisPos, &lastPos);
    }
}
//$END_ADDITION


/////////////////////////////////////////////////////////////////////
//
// DrawCarAerial: draw the car's aerial!
//
/////////////////////////////////////////////////////////////////////

void DrawCarAerial2(AERIAL *aerial, MODEL *secModel, MODEL *topModel, short flag)
{
    int iSec;
    VEC lastPos;
    MAT wMatrix;
    REAL    t, dT;
    VEC     delta[2];

#if 1

// Copy thisPos to lastPos
    if (!GameSettings.Paws)
    {
        for (iSec = 0; iSec < AERIAL_NTOTSECTIONS; iSec++)
            CopyVec(&aerial->thisPos[iSec], &aerial->lastPos[iSec]);
    }

    // Calculate the positions of the non-control sections by interpolating from the control sections
    CopyVec(&aerial->Section[0].Pos, &lastPos);
    CopyVec(&aerial->Section[0].Pos, &aerial->thisPos[0]);

    for (iSec = AERIAL_START; iSec < AERIAL_NTOTSECTIONS; iSec++) {
        
        // calculate the position of the interpolated node
        Interpolate3D(
            &aerial->Section[0].Pos,
            &aerial->Section[AERIAL_SKIP].Pos,
            &aerial->Section[AERIAL_LASTSECTION].Pos,
            iSec * AERIAL_UNITLEN,
            &aerial->thisPos[iSec]);

        // Set the up vector of the node (already scaled to give correct length)
        VecMinusVec(&aerial->thisPos[iSec], &lastPos, &wMatrix.mv[U]);

        if ((abs(wMatrix.mv[U].v[X]) < 0.1f) && (abs(wMatrix.mv[U].v[Y]) < 0.1f) && (abs(wMatrix.mv[U].v[Z]) < 0.1f)) {
            continue;
        }

        // Build the world Matrix for the section
        BuildMatrixFromUp(&wMatrix);
        // Must normalise look vector when passed up vector was not normalised
        NormalizeVec(&wMatrix.mv[L]);

        // Draw the actual model
        if (iSec != AERIAL_NTOTSECTIONS - 1) {
            if (flag & MODEL_LIT) {
                AddModelLightSimple(secModel, &aerial->thisPos[iSec]);
            }
            DrawModel(secModel, &wMatrix, &aerial->thisPos[iSec], flag);
        } else {
            if (flag & MODEL_LIT) {
                AddModelLightSimple(topModel, &aerial->thisPos[iSec]);
            }
            DrawModel(topModel, &wMatrix, &aerial->thisPos[iSec], flag);
        }

        CopyVec(&aerial->thisPos[iSec], &lastPos);
    }

// Render motion blur
    if (aerial->visibleLast == FALSE)
    {
        aerial->visibleLast = TRUE;
        return;
    }

    if (NPhysicsLoops != 0)
        dT = (Real(2)) / Real(NPhysicsLoops);
    else
        dT = ZERO;

    dT /= AERIAL_NTOTSECTIONS;
    t = dT * 2;

    for (iSec = 0; iSec < AERIAL_NTOTSECTIONS-2; iSec++)
    {
        // stick
        VecMinusVec(&aerial->lastPos[iSec], &aerial->thisPos[iSec], &delta[0]);
        VecEqScalarVec(&delta[0], t, &delta[0]);
        VecPlusEqVec(&delta[0], &aerial->thisPos[iSec])
        VecMinusVec(&aerial->lastPos[iSec+1], &aerial->thisPos[iSec+1], &delta[1]);
        VecEqScalarVec(&delta[1], t+dT, &delta[1]);
        VecPlusEqVec(&delta[1], &aerial->thisPos[iSec+1])

        MotionBlurAdd(&aerial->thisPos[iSec], &aerial->thisPos[iSec+1], &delta[1], &delta[0], 0x000000, 0x000000);

        t += dT;
    }

    // Red nobble
    VecMinusVec(&aerial->lastPos[iSec], &aerial->thisPos[iSec], &delta[0]);
    VecEqScalarVec(&delta[0], t, &delta[0]);
    VecPlusEqVec(&delta[0], &aerial->thisPos[iSec])
    VecMinusVec(&aerial->lastPos[iSec+1], &aerial->thisPos[iSec+1], &delta[1]);
    VecEqScalarVec(&delta[1], t+dT, &delta[1]);
    VecPlusEqVec(&delta[1], &aerial->thisPos[iSec+1])

    MotionBlurAdd(&aerial->thisPos[iSec], &aerial->thisPos[iSec+1], &delta[1], &delta[0], 0x000000, 0x800000);

#else
    REAL t;

    CopyVec(&aerial->Section[0].Pos, &lastPos);

    for (iSec = 1; iSec < AERIAL_NTOTSECTIONS; iSec++) {
        
        // Build all the intermediate positions
        t = iSec * AERIAL_UNITLEN;
        InterpolatePosDir(
            &aerial->Section[0].Pos, 
            &aerial->Section[AERIAL_LASTSECTION].Pos, 
            &aerial->WorldDirection, 
            t, 
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
#endif
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
    BLEND_ON();
    BLEND_SRC(D3DBLEND_ZERO);
    BLEND_DEST(D3DBLEND_INVSRCCOLOR);

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
    for (iSkid = 0; iSkid < WHL_NSkids; iSkid++)
    {
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
            DrawVertsTEX1[j].color = 0x11000000 | skid->RGB[j];
        }
        DRAW_PRIM(D3DPT_TRIANGLEFAN, FVF_TEX1, DrawVertsTEX1, 4, D3DDP_DONOTUPDATEEXTENTS);
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
        obj->renderflag.visible |= RenderObjectModel(&obj->body.Centre.WMatrix, &obj->body.Centre.Pos, &LevelModel[obj->DefaultModel].Model, obj->EnvRGB, obj->EnvOffsetX, obj->EnvOffsetY, obj->EnvScale, obj->renderflag, FALSE);
}

////////////////////////////
// render an object model //
////////////////////////////

bool RenderObjectModel(MAT *mat, VEC *pos, MODEL *model, long envrgb, REAL envoffx, REAL envoffy, REAL envscale, struct renderflags renderflag, long scaled)
{
    REAL z;
    BOUNDING_BOX box;
    long visflag;
    short flag = MODEL_PLAIN;
    VEC v0, v1, v2, v3;
    float rad, len;

// get radius

    rad = model->Radius;
    if (scaled)
    {
        len = Length(&mat->mv[R]) * model->Radius;
        if (len > rad) rad = len;

        len = Length(&mat->mv[U]) * model->Radius;
        if (len > rad) rad = len;

        len = Length(&mat->mv[L]) * model->Radius;
        if (len > rad) rad = len;
    }

// get bounding box

    box.Xmin = pos->v[X] - rad;
    box.Xmax = pos->v[X] + rad;
    box.Ymin = pos->v[Y] - rad;
    box.Ymax = pos->v[Y] + rad;
    box.Zmin = pos->v[Z] - rad;
    box.Zmax = pos->v[Z] + rad;

// test against visicubes

    if (TestObjectVisiboxes(&box))
        return FALSE;

// skip if offscreen

    visflag = TestSphereToFrustum(pos, rad, &z);
    if (visflag == SPHERE_OUT) return FALSE;
    if (visflag == SPHERE_IN) flag |= MODEL_DONOTCLIP;
    if (z - rad < RenderSettings.NearClip) flag |= MODEL_NEARCLIP;

//$ADDITION(jedl) - short circuit all the local lighting and transform and fog tests
    if( RegistrySettings.bUseGPU
        && model->m_pEffect != NULL )
    {
        //$TODO: lights, glare, fx, mirror
        DrawModelGPU(model, mat, pos, flag);
        return TRUE;
    }
//$END_ADDITION

// env?

    if (renderflag.envmap)
    {
        flag |= MODEL_ENV;
        SetEnvStatic(pos, mat, envrgb, envoffx, envoffy, envscale);
    }

// good env

    if (renderflag.envgood)
    {
        flag |= MODEL_ENVGOOD;
        SetEnvGood(envrgb, envoffx, envoffy, envscale);
    }

// in light?

    if (renderflag.light)
    {
        if (CheckObjectLight(pos, &box, rad))
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

    if (renderflag.fog && z + rad > RenderSettings.FogStart && DxState.Fog)
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

// draw shadow

    if (renderflag.shadow)
    {
        SetVector(&v0, pos->v[X] - rad, pos->v[Y], pos->v[Z] - rad);
        SetVector(&v1, pos->v[X] + rad, pos->v[Y], pos->v[Z] - rad);
        SetVector(&v2, pos->v[X] + rad, pos->v[Y], pos->v[Z] + rad);
        SetVector(&v3, pos->v[X] - rad, pos->v[Y], pos->v[Z] + rad);

        DrawShadow(&v0, &v1, &v2, &v3, 193.0f / 256.0f, 129.0f / 256.0f, 62.0f / 256.0f, 62.0f / 256.0f, BlendSubtract ? 0x808080 : 0x80000000, -2.0f, 0.0f, BlendSubtract ? 2 : 0, TPAGE_FX1, NULL);
    }

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
    if (z - model->Radius < RenderSettings.NearClip) flag |= MODEL_NEARCLIP;

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
    REAL z, fog, zbuf;
//$REMOVED    REAL zres;
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

//$MODIFIED
//    zres = (float)(1 << ZedBufferFormat.dwZBufferBitDepth);
//    zbuf = (zres - 1.0f) / zres;
    zbuf = 0.99999f;  //$REVISIT:  This is ugly. Maybe we should use 1.0, and ensure ZCMP is LESSEQUAL
//$END_MODIFICATIONS

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

    Assert(starnum > 0);  //$ADDITION [If we hit this assert, need to put back JedL's code to only do this stuff if (starnum > 0) ]
    FOG_ON();
    SET_TPAGE(-1);

    SET_RENDER_STATE(D3DRENDERSTATE_ALPHATESTENABLE, FALSE);
    DRAW_PRIM(D3DPT_POINTLIST, FVF_TEX0, sun->Verts, starnum, D3DDP_DONOTUPDATEEXTENTS);
    SET_RENDER_STATE(D3DRENDERSTATE_ALPHATESTENABLE, TRUE);

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
        vis1 = RenderObjectModel(&obj->body.Centre.WMatrix, &obj->body.Centre.Pos, &LevelModel[obj->DefaultModel].Model, obj->EnvRGB, obj->EnvOffsetX, obj->EnvOffsetY, obj->EnvScale, obj->renderflag, FALSE);
    else
        vis1 = FALSE;

// render propellor

    if (plane->PropModel != -1)
        vis2 = RenderObjectModel(&plane->PropMatrix, &plane->PropPos, &LevelModel[plane->PropModel].Model, obj->EnvRGB, obj->EnvOffsetX, obj->EnvOffsetY, obj->EnvScale, obj->renderflag, FALSE);
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
        vis1 = RenderObjectModel(&obj->body.Centre.WMatrix, &obj->body.Centre.Pos, &LevelModel[obj->DefaultModel].Model, obj->EnvRGB, obj->EnvOffsetX, obj->EnvOffsetY, obj->EnvScale, obj->renderflag, FALSE);
    else
        vis1 = FALSE;

// render blade1

    if (copter->BladeModel1 != -1)
        vis2 = RenderObjectModel(&copter->BladeMatrix1, &copter->BladePos1, &LevelModel[copter->BladeModel1].Model, obj->EnvRGB, obj->EnvOffsetX, obj->EnvOffsetY, obj->EnvScale, obj->renderflag, FALSE);
    else
        vis2 = FALSE;

// render blade2

    if (copter->BladeModel2 != -1)
        vis3 = RenderObjectModel(&copter->BladeMatrix2, &copter->BladePos2, &LevelModel[copter->BladeModel2].Model, obj->EnvRGB, obj->EnvOffsetX, obj->EnvOffsetY, obj->EnvScale, obj->renderflag, FALSE);
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
        vis1 = RenderObjectModel(&obj->body.Centre.WMatrix, &obj->body.Centre.Pos, &LevelModel[dragon->BodyModel].Model, obj->EnvRGB, obj->EnvOffsetX, obj->EnvOffsetY, obj->EnvScale, obj->renderflag, FALSE);
    else
        vis1 = FALSE;

// render head

    if (dragon->HeadModel != -1)
        vis2 = RenderObjectModel(&obj->body.Centre.WMatrix, &obj->body.Centre.Pos, &LevelModel[dragon->HeadModel].Model, obj->EnvRGB, obj->EnvOffsetX, obj->EnvOffsetY, obj->EnvScale, obj->renderflag, FALSE);
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

    if (grid == NULL) return;

    SET_TPAGE(-1);

    for (iPoly = 0; iPoly < grid->NCollPolys; iPoly++)
    {
        NEWCOLLPOLY *collPoly = GetCollPoly(grid->CollPolyIndices[iPoly]);
        DrawCollPoly(collPoly);
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
    DrawCar(obj->player);
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
        obj->renderflag.visible |= RenderObjectModel(&obj->body.Centre.WMatrix, &obj->body.Centre.Pos, &LevelModel[obj->DefaultModel].Model, obj->EnvRGB, obj->EnvOffsetX, obj->EnvOffsetY, obj->EnvScale, obj->renderflag, FALSE);

// render wheels

    obj->renderflag.envmap = FALSE;

    if (train->FrontWheel)
    {
        RotMatrixX(&mat1, train->TimeFront);
        MulMatrix(&obj->body.Centre.WMatrix, &mat1, &mat2);

        RenderObjectModel(&mat2, &train->WheelPos[2], &LevelModel[train->FrontWheel].Model, obj->EnvRGB, obj->EnvOffsetX, obj->EnvOffsetY, obj->EnvScale, obj->renderflag, FALSE);
        RenderObjectModel(&mat2, &train->WheelPos[3], &LevelModel[train->FrontWheel].Model, obj->EnvRGB, obj->EnvOffsetX, obj->EnvOffsetY, obj->EnvScale, obj->renderflag, FALSE);
    }

    if (train->BackWheel)
    {
        RotMatrixX(&mat1, train->TimeBack);
        MulMatrix(&obj->body.Centre.WMatrix, &mat1, &mat2);

        RenderObjectModel(&mat2, &train->WheelPos[0], &LevelModel[train->BackWheel].Model, obj->EnvRGB, obj->EnvOffsetX, obj->EnvOffsetY, obj->EnvScale, obj->renderflag, FALSE);
        RenderObjectModel(&mat2, &train->WheelPos[1], &LevelModel[train->BackWheel].Model, obj->EnvRGB, obj->EnvOffsetX, obj->EnvOffsetY, obj->EnvScale, obj->renderflag, FALSE);
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

//      ang = -(float)atan(ViewMatrix.m[LZ] / ViewMatrix.m[LX]) / PI;
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

void DrawAllPickups()
{
    int iPickup;

    for (iPickup = 0; iPickup < MAX_PICKUPS; iPickup++) {
        if (PickupArray[iPickup].Mode != PICKUP_STATE_FREE) {
            RenderPickup(&PickupArray[iPickup]);
        }
    }
}

void RenderPickup(PICKUP *pickup)
{
    long col, i, alpha;
    MODEL *model = &LevelModel[pickup->DefaultModel].Model;
    FACING_POLY poly;
    MAT mat;
    REAL size;

    // Overall BBox
    if (CAR_DrawCarBBoxes) {
        DrawBoundingBox(
            pickup->BBox.XMin, 
            pickup->BBox.XMax, 
            pickup->BBox.YMin, 
            pickup->BBox.YMax, 
            pickup->BBox.ZMin, 
            pickup->BBox.ZMax, 
            0x33ff0000, 0x3300ff00, 0x330000ff, 
            0x33ff0000, 0x3300ff00, 0x330000ff);
    }
            
// act on mode

    switch (pickup->Mode)
    {

// waiting to gen

        case PICKUP_STATE_GENERATING:
            return;

// waiting to be picked up

        case PICKUP_STATE_ACTIVE:

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

                if (pickup->DefaultModel != -1)
                    pickup->RenderFlags.visible |= RenderObjectModel(&pickup->WMatrix, &pickup->Pos, &LevelModel[pickup->DefaultModel].Model, pickup->EnvRGB, pickup->EnvOffsetX, pickup->EnvOffsetY, pickup->EnvScale, pickup->RenderFlags, TRUE);
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
                DrawFacingPolyRotMirror(&pickup->Pos, &mat, &poly, 1, -256);

                RotMatrixZ(&mat, pickup->Timer / 2.0f + 0.5f);
                DrawFacingPolyRotMirror(&pickup->Pos, &mat, &poly, 1, -256);

                poly.U = 128.0f / 256.0f;
                poly.V = 0.0f / 256.0f;
                poly.RGB = (col) | (col << 8) | (col << 16);

                RotMatrixZ(&mat, 0.0f);
                DrawFacingPolyRotMirror(&pickup->Pos, &mat, &poly, 1, -256);
            }

            break;

// disappearing

        case PICKUP_STATE_DYING:

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

            if (pickup->DefaultModel != -1)
                pickup->RenderFlags.visible |= RenderObjectModel(&pickup->WMatrix, &pickup->Pos, &LevelModel[pickup->DefaultModel].Model, pickup->EnvRGB, pickup->EnvOffsetX, pickup->EnvOffsetY, pickup->EnvScale, pickup->RenderFlags, TRUE);

            break;

        default:
            break;
    }
}

////////////////////////
// render star pickup //
////////////////////////

void RenderStar(OBJECT *obj)
{
    long i, alpha;
    MODEL *model = &LevelModel[obj->DefaultModel].Model;
    STAR_OBJ *star = (STAR_OBJ*)obj->Data;

// act on mode

    switch (star->Mode)
    {

// waiting to be picked up

        case 0:

// draw model

            if (model->PolyPtr->Type & POLY_SEMITRANS)
            {
                for (i = 0 ; i < model->PolyNum ; i++)
                {
                    model->PolyPtr[i].Type &= ~POLY_SEMITRANS;
                }
            }

            if (obj->DefaultModel != -1)
                obj->renderflag.visible |= RenderObjectModel(&obj->body.Centre.WMatrix, &obj->body.Centre.Pos, &LevelModel[obj->DefaultModel].Model, obj->EnvRGB, obj->EnvOffsetX, obj->EnvOffsetY, obj->EnvScale, obj->renderflag, TRUE);

            break;

// disappearing

        case 1:

// set alpha

            FTOL(-star->Timer * 255.0f + 255.0f, alpha);

            for (i = 0 ; i < model->PolyNum ; i++)
            {
                model->PolyPtr[i].Type |= POLY_SEMITRANS;
                model->PolyRGB[i].rgb[0].a = (unsigned char)alpha;
                model->PolyRGB[i].rgb[1].a = (unsigned char)alpha;
                model->PolyRGB[i].rgb[2].a = (unsigned char)alpha;
                model->PolyRGB[i].rgb[3].a = (unsigned char)alpha;
            }

// draw model

            if (obj->DefaultModel != -1)
                obj->renderflag.visible |= RenderObjectModel(&obj->body.Centre.WMatrix, &obj->body.Centre.Pos, &LevelModel[obj->DefaultModel].Model, obj->EnvRGB, obj->EnvOffsetX, obj->EnvOffsetY, obj->EnvScale, obj->renderflag, TRUE);

            break;
    }
}

////////////////
// render fox //
////////////////

void RenderFox(OBJECT *obj)
{
    FOX_OBJ *fox = (FOX_OBJ*)obj->Data;
    CAR *car = &obj->player->car;
    VEC dir1, dir2;

// draw model

    obj->renderflag.visible |= RenderObjectModel(&car->Body->Centre.WMatrix, &car->BodyWorldPos, &fox->Model, obj->EnvRGB, obj->EnvOffsetX, obj->EnvOffsetY, obj->EnvScale, obj->renderflag, FALSE);

// draw jump spark?

    if (fox->JumpFlag)
    {
        dir2.v[0] = Real(0);
        dir2.v[1] = Real(-50);
        dir2.v[2] = Real(0);

        dir1.v[0] = Real(0);
        dir1.v[1] = Real(-50);
        dir1.v[2] = Real(0);

        DrawJumpSpark(&fox->JumpVec1, &fox->JumpVec2, &dir1, &dir2, 1);
    }
}

///////////////////////////
// render dissolve model //
///////////////////////////

void RenderDissolveModel(OBJECT *obj)
{
    DISSOLVE_OBJ *dissolve = (DISSOLVE_OBJ*)obj->Data;

// render model

    obj->renderflag.visible |= RenderObjectModel(&obj->body.Centre.WMatrix, &obj->body.Centre.Pos, &dissolve->Model, dissolve->EnvRGB, obj->EnvOffsetX, obj->EnvOffsetY, obj->EnvScale, obj->renderflag, FALSE);
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
        vert[1].sx = vert[0].sx + dx * (laser->Width + widMod) * delta * RenderSettings.MatScaleX;
        vert[1].sy = vert[0].sy + dy * (laser->Width + widMod) * delta * RenderSettings.MatScaleY;
        vert[1].sz = vert[0].sz;
        vert[1].rhw = vert[0].rhw;
        vert[0].sx -= dx * (laser->Width + widMod) * delta * RenderSettings.MatScaleX;
        vert[0].sy -= dy * (laser->Width + widMod) * delta * RenderSettings.MatScaleY;

        delta = vert[2].rhw;
        vert[3].sx = vert[2].sx - dx * (laser->Width + widMod) * delta * RenderSettings.MatScaleX;
        vert[3].sy = vert[2].sy - dy * (laser->Width + widMod) * delta * RenderSettings.MatScaleY;
        vert[3].sz = vert[2].sz;
        vert[3].rhw = vert[2].rhw;
        vert[2].sx += dx * (laser->Width + widMod) * delta * RenderSettings.MatScaleX;
        vert[2].sy += dy * (laser->Width + widMod) * delta * RenderSettings.MatScaleY;

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
    long col;
    REAL scale;
    MAT mat;
    FACING_POLY poly;

    FOG_ON();

    scale = player->PickupTargetTime / TARGET_ANIM_TIME;
    if (scale < ONE) {
        col = 0xff & ((long)(255.0f * scale));
    } else {
        col = 0xff;
    }

    if (player == PLR_LocalPlayer) {
        poly.RGB = col << 8;
    } else {
        poly.RGB = col << 16;
    }
    /*if (player->PickupTarget == PLR_LocalPlayer->ownobj) {
        poly.RGB = col << 16;//0xff0000;
    } else {
        poly.RGB = col << 8;//col << 16 | col << 8 | col;//0xaaffaa;
    }*/
    poly.Usize = poly.Vsize = 32.0f / 256.0f;
    poly.Tpage = TPAGE_FX1;
    poly.U = 32.0f / 256.0f;
    poly.V = 224.0f / 256.0f;

    if (player->PickupTargetTime < TARGET_ANIM_TIME) {
        if (fmod(player->PickupTargetTime * TARGET_BLINKMUL, Real(2)) < ONE) {
            poly.Xsize = poly.Ysize = TARGET_SIZE * (Real(3) - 2 * scale);
            RotMatrixZ(&mat, scale);
            DrawFacingPolyRot(&player->PickupTarget->player->car.Body->Centre.Pos, &mat, &poly, 1, -256);
        }
    } else {
        poly.Xsize = poly.Ysize = TARGET_SIZE;
        RotMatrixZ(&mat, 0.25f * scale);
        DrawFacingPolyRot(&player->PickupTarget->player->car.Body->Centre.Pos, &mat, &poly, 1, -256);
    }

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

    PulseSpeedupModelGourad(obj);

    RenderObjectModel(&obj->body.Centre.WMatrix, &speedup->PostPos[0], &LevelModel[obj->DefaultModel].Model, obj->EnvRGB, obj->EnvOffsetX, obj->EnvOffsetY, obj->EnvScale, obj->renderflag, FALSE);
    RenderObjectModel(&obj->body.Centre.WMatrix, &speedup->PostPos[1], &LevelModel[obj->DefaultModel].Model, obj->EnvRGB, obj->EnvOffsetX, obj->EnvOffsetY, obj->EnvScale, obj->renderflag, FALSE);

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

void PulseSpeedupModelGourad(OBJECT *obj)
{
    long iPoly;
    unsigned long baseRGB, col;
    SPEEDUP_OBJ *speedup = (SPEEDUP_OBJ *)obj->Data;
    MODEL *model;
    long nPolys;

    if (obj->DefaultModel == -1) return;
    model = &LevelModel[obj->DefaultModel].Model;
    nPolys = model->PolyNum;

    /*if (speedup->Speed == speedup->HiSpeed) {
        baseRGB = 0x003300;
    } else {
        baseRGB = 0x330000;
    }*/
    col = (long)(125.0f * (speedup->Speed - speedup->LoSpeed) / (speedup->HiSpeed - speedup->LoSpeed));
    col &= 0xff;
    baseRGB = (196 - col) << 16 | (64 + col) << 8;

    // Modify model gourad according to y height and timer
    for (iPoly = 0; iPoly < nPolys; iPoly++) {
        *(long *)&model->PolyRGB[iPoly].rgb[0] = baseRGB;
        *(long *)&model->PolyRGB[iPoly].rgb[1] = baseRGB;
        *(long *)&model->PolyRGB[iPoly].rgb[2] = baseRGB;
        *(long *)&model->PolyRGB[iPoly].rgb[3] = baseRGB;
    }

}

///////////////////
// render clouds //
///////////////////

void RenderClouds(OBJECT *obj)
{
    CLOUDS_OBJ *clouds = (CLOUDS_OBJ*)obj->Data;
    CLOUD *cloud;
    long i, j;
    REAL z, zbuf, u, v;

// not if camera inside fog box

    if (Camera[CameraCount].SpecialFog)
        return;

// set render states

    SET_RENDER_STATE(D3DRENDERSTATE_ALPHAREF, 248);
    SET_TPAGE(TPAGE_MISC1);

    cloud = clouds->Cloud;
//$MODIFIED
//    zbuf = (float)(1 << ZedBufferFormat.dwZBufferBitDepth);
//    zbuf = (zbuf - 1.0f) / zbuf;
    zbuf = 0.99999f;  //$REVISIT:  This is ugly. Maybe we should use 1.0, and ensure ZCMP is LESSEQUAL
//$END_MODIFICATIONS

    for (i = 0 ; i < CLOUD_NUM ; i++, cloud++)
    {

// viewcone test

        z = cloud->Centre.v[X] * ViewMatrixScaled.m[RZ] + cloud->Centre.v[Y] * ViewMatrixScaled.m[UZ] + cloud->Centre.v[Z] * ViewMatrixScaled.m[LZ];
        if (z < 0) continue;

        if (DotProduct(&CameraPlaneLeft, &cloud->Centre) >= cloud->Radius) continue;
        if (DotProduct(&CameraPlaneRight, &cloud->Centre) >= cloud->Radius) continue;
        if (DotProduct(&CameraPlaneBottom, &cloud->Centre) >= cloud->Radius) continue;
        if (DotProduct(&CameraPlaneTop, &cloud->Centre) >= cloud->Radius) continue;

// render

        for (j = 0 ; j < 4 ; j++)
        {
            z = cloud->Pos[j].v[X] * ViewMatrixScaled.m[RZ] + cloud->Pos[j].v[Y] * ViewMatrixScaled.m[UZ] + cloud->Pos[j].v[Z] * ViewMatrixScaled.m[LZ];
            DrawVertsTEX1[j].sx = (cloud->Pos[j].v[X] * ViewMatrixScaled.m[RX] + cloud->Pos[j].v[Y] * ViewMatrixScaled.m[UX] + cloud->Pos[j].v[Z] * ViewMatrixScaled.m[LX]) / z + RenderSettings.GeomCentreX;
            DrawVertsTEX1[j].sy = (cloud->Pos[j].v[X] * ViewMatrixScaled.m[RY] + cloud->Pos[j].v[Y] * ViewMatrixScaled.m[UY] + cloud->Pos[j].v[Z] * ViewMatrixScaled.m[LY]) / z + RenderSettings.GeomCentreY;
            DrawVertsTEX1[j].sz = zbuf;
            DrawVertsTEX1[j].rhw = 1.0f / z;
            DrawVertsTEX1[j].color = 0x40ffffff;
        }

        u = (float)(cloud->Type & 1) / 2.0f + (1.0f / 256.0f);
        v = (float)(cloud->Type & 2) / 4.0f + (1.0f / 256.0f);

        DrawVertsTEX1[0].tu = u;
        DrawVertsTEX1[0].tv = v;

        DrawVertsTEX1[1].tu = u + 126.0f / 256.0f;
        DrawVertsTEX1[1].tv = v;

        DrawVertsTEX1[2].tu = u + 126.0f / 256.0f;
        DrawVertsTEX1[2].tv = v + 126.0f / 256.0f;

        DrawVertsTEX1[3].tu = u;
        DrawVertsTEX1[3].tv = v + 126.0f / 256.0f;

        DRAW_PRIM(D3DPT_TRIANGLEFAN, FVF_TEX1, DrawVertsTEX1, 4, D3DDP_DONOTUPDATEEXTENTS);
    }

    SET_RENDER_STATE(D3DRENDERSTATE_ALPHAREF, AlphaRef);
}

//////////////////////
// render sprinkler //
//////////////////////

void RenderSprinkler(OBJECT *obj)
{
    bool vis1, vis2;
    SPRINKLER_OBJ *sprinkler = (SPRINKLER_OBJ*)obj->Data;

// render base

    if (sprinkler->BaseModel != -1)
        vis1 = RenderObjectModel(&obj->body.Centre.WMatrix, &obj->body.Centre.Pos, &LevelModel[sprinkler->BaseModel].Model, obj->EnvRGB, obj->EnvOffsetX, obj->EnvOffsetY, obj->EnvScale, obj->renderflag, FALSE);
    else
        vis1 = FALSE;

// render head

    if (sprinkler->HeadModel != -1)
        vis2 = RenderObjectModel(&sprinkler->HeadMat, &sprinkler->HeadPos, &LevelModel[sprinkler->HeadModel].Model, obj->EnvRGB, obj->EnvOffsetX, obj->EnvOffsetY, obj->EnvScale, obj->renderflag, FALSE);
    else
        vis2 = FALSE;

// set visible flag

    obj->renderflag.visible |= (vis1 || vis2);
}


////////////////////////////////////////////////////////////////
//
// Draw Cup
//
////////////////////////////////////////////////////////////////

void DrawCup(OBJECT *obj)
{
    SetEnvGood(0xFFFFFF, 0, 0, 1);
    DrawModel(&LevelModel[obj->DefaultModel].Model, &obj->body.Centre.WMatrix, &obj->body.Centre.Pos, MODEL_ENVGOOD);
}


////////////////////////////////////////////////////////////////
//
// Draw Demo message
//
////////////////////////////////////////////////////////////////

void DrawDemoMessage()
{
    REAL xPos, yPos, xSize, ySize;
    VERTEX_TEX1 verts[4];
    VERTEX_TEX1 *vert;
    long col;
    static REAL timer = ZERO;

    vert = verts;

    xSize = gMenuWidthScale * 200;
    ySize = gMenuHeightScale * 60;

    xPos = gMenuWidthScale * (640.0f - xSize) * HALF;
    yPos = gMenuHeightScale * 20;

    // Calculate coordinates of box corners
    vert[0].sx = xPos;
    vert[0].sy = yPos;
    vert[1].sx = xPos + xSize;
    vert[1].sy = yPos;
    vert[2].sx = xPos + xSize;
    vert[2].sy = yPos + ySize;
    vert[3].sx = xPos;
    vert[3].sy = yPos + ySize;

    vert[0].sz = vert[1].sz = vert[2].sz = vert[3].sz = 300;
    vert[0].rhw = vert[1].rhw = vert[2].rhw = vert[3].rhw = 1;

    // Set colours
    timer += TimeStep;
    /*if (timer < 1.5f) {
        col = (long)(255.0f * timer / 1.5f);
        col = col << 24 | 0xffffff;
    } else {
        col = (long)(255.0f * (3.0f - timer) / 1.5f);
        col = col << 24 | 0xffffff;
    }
    if (timer > 3.0f) timer -= 3.0f;*/
    col = (long)(255.0 * (1.0 + sin(timer * 2)) / 2);
    col = col << 24 | 0xffffff;

    vert[0].color = vert[1].color = vert[2].color = vert[3].color = col;

    // set uvs
    vert[0].tu = 0.0f / 256.0f;
    vert[0].tv = 230.0f / 256.0f;
    vert[1].tu = 182.0f / 256.0f;
    vert[1].tv = 230.0f / 256.0f;
    vert[2].tu = 182.0f / 256.0f;
    vert[2].tv = 256.0f / 256.0f;
    vert[3].tu = 0.0f / 256.0f;
    vert[3].tv = 256.0f / 256.0f;

    // Draw background poly
    SET_TPAGE(TPAGE_LOADING);
    FOG_OFF();
    BLEND_SRC(D3DBLEND_SRCALPHA);
    BLEND_DEST(D3DBLEND_INVSRCALPHA);
    BLEND_ALPHA();

    DRAW_PRIM(D3DPT_TRIANGLEFAN, FVF_TEX1, vert, 4, D3DDP_DONOTUPDATEEXTENTS);
}

void DrawDemoLogo(REAL alphaMod, int posIndex)
{
    REAL xPos, yPos, xSize, ySize;
    VERTEX_TEX1 verts[4];
    VERTEX_TEX1 *vert;
    long col;
    static REAL timer = ZERO;

    vert = verts;

#if 0
    timer += TimeStep;
    if (timer < 1.5f) {
        xSize = gMenuWidthScale * (200 + timer * 20);
        ySize = gMenuHeightScale * (60 + timer * 6);
    } else {
        xSize = gMenuWidthScale * (200 + (3.0f - timer) * 20);
        ySize = gMenuHeightScale * (60 + (3.0f - timer) * 6);
    }
    if (timer > 3.0f) timer -= 3.0f;
#else
    xSize = gMenuWidthScale * 300;
    ySize = gMenuHeightScale * 90;
#endif  

    if (posIndex == 0) {
        xPos = (gMenuWidthScale * 640.0f - xSize) * HALF;
        yPos = gMenuHeightScale * 350;
    } else {
        xPos = (gMenuWidthScale * 640.0f - xSize) * HALF;
        yPos = gMenuHeightScale * 60;
    }

    // Calculate coordinates of box corners
    vert[0].sx = xPos;
    vert[0].sy = yPos;
    vert[1].sx = xPos + xSize;
    vert[1].sy = yPos;
    vert[2].sx = xPos + xSize;
    vert[2].sy = yPos + ySize;
    vert[3].sx = xPos;
    vert[3].sy = yPos + ySize;

    vert[0].sz = vert[1].sz = vert[2].sz = vert[3].sz = 300;
    vert[0].rhw = vert[1].rhw = vert[2].rhw = vert[3].rhw = 1;


    // Set colours
    col = (long)(255.0f * alphaMod);
    if (col > 255) col = 255;
    col = col << 24 | 0xffffff;
    vert[0].color = vert[1].color = vert[2].color = vert[3].color = col;

    // set uvs
    vert[0].tu = 0.0f / 256.0f;
    vert[0].tv = 0.0f / 256.0f;
    vert[1].tu = 256.0f / 256.0f;
    vert[1].tv = 0.0f / 256.0f;
    vert[2].tu = 256.0f / 256.0f;
    vert[2].tv = 75.0f / 256.0f;
    vert[3].tu = 0.0f / 256.0f;
    vert[3].tv = 75.0f / 256.0f;

    // Draw background poly
    SET_TPAGE(TPAGE_LOADING);
    FOG_OFF();
    BLEND_SRC(D3DBLEND_SRCALPHA);
    BLEND_DEST(D3DBLEND_INVSRCALPHA);
    BLEND_ALPHA();

    DRAW_PRIM(D3DPT_TRIANGLEFAN, FVF_TEX1, vert, 4, D3DDP_DONOTUPDATEEXTENTS);


}


////////////////////////////////////////////////////////////////
//
// Draw E3 message
//
////////////////////////////////////////////////////////////////

void DrawE3Message(REAL alphaMod)
{
    REAL xPos, yPos, xSize, ySize;
    VERTEX_TEX1 verts[4];
    VERTEX_TEX1 *vert;
    long col;
    static REAL timer = ZERO;

    vert = verts;

    xSize = gMenuWidthScale * 195;
    ySize = gMenuHeightScale * 60;

#if 1
        xPos = (gMenuWidthScale * 640.0f - xSize) * HALF;
        yPos = gMenuHeightScale * 400;
#else
        xPos = (gMenuWidthScale * 640.0f - xSize) * HALF;
        yPos = gMenuHeightScale * 60;
#endif

    // Calculate coordinates of box corners
    vert[0].sx = xPos;
    vert[0].sy = yPos;
    vert[1].sx = xPos + xSize;
    vert[1].sy = yPos;
    vert[2].sx = xPos + xSize;
    vert[2].sy = yPos + ySize;
    vert[3].sx = xPos;
    vert[3].sy = yPos + ySize;

    vert[0].sz = vert[1].sz = vert[2].sz = vert[3].sz = 300;
    vert[0].rhw = vert[1].rhw = vert[2].rhw = vert[3].rhw = 1;


    // Set colours
    col = (long)(255.0f * alphaMod);
    if (col > 255) col = 255;
    col = col << 24 | 0xffffff;
    vert[0].color = vert[1].color = vert[2].color = vert[3].color = col;

    // set uvs
    vert[0].tu = 0.0f / 256.0f;
    vert[0].tv = 194.0f / 256.0f;
    vert[1].tu = 167.0f / 256.0f;
    vert[1].tv = 194.0f / 256.0f;
    vert[2].tu = 167.0f / 256.0f;
    vert[2].tv = 228.0f / 256.0f;
    vert[3].tu = 0.0f / 256.0f;
    vert[3].tv = 228.0f / 256.0f;

    // Draw background poly
    SET_TPAGE(TPAGE_LOADING);
    FOG_OFF();
    BLEND_SRC(D3DBLEND_SRCALPHA);
    BLEND_DEST(D3DBLEND_INVSRCALPHA);
    BLEND_ALPHA();

    DRAW_PRIM(D3DPT_TRIANGLEFAN, FVF_TEX1, vert, 4, D3DDP_DONOTUPDATEEXTENTS);


}

////////////////////////////////////////////////////////////////
//
// Draw Replay message
//
////////////////////////////////////////////////////////////////

void DrawReplayMessage()
{
    REAL xPos, yPos, xSize, ySize;
    VERTEX_TEX1 verts[4];
    VERTEX_TEX1 *vert;
    long col;
    static REAL timer = ZERO;

    vert = verts;

    xSize = gMenuWidthScale * 60;
    ySize = gMenuHeightScale * 20;

    xPos = gMenuWidthScale * (640.0f - xSize - 20);
    yPos = gMenuHeightScale * 10;

    // Calculate coordinates of box corners
    vert[0].sx = xPos;
    vert[0].sy = yPos;
    vert[1].sx = xPos + xSize;
    vert[1].sy = yPos;
    vert[2].sx = xPos + xSize;
    vert[2].sy = yPos + ySize;
    vert[3].sx = xPos;
    vert[3].sy = yPos + ySize;

    vert[0].sz = vert[1].sz = vert[2].sz = vert[3].sz = 300;
    vert[0].rhw = vert[1].rhw = vert[2].rhw = vert[3].rhw = 1;

    // Set colours
    timer += TimeStep;
    /*if (timer < 1.5f) {
        col = (long)(255.0f * timer / 1.5f);
        col = col << 24 | 0xffffff;
    } else {
        col = (long)(255.0f * (3.0f - timer) / 1.5f);
        col = col << 24 | 0xffffff;
    }
    if (timer > 3.0f) timer -= 3.0f;*/
    col = (long)(255.0 * (1.0 + sin(timer * 2)) / 2);
    col = col << 24 | 0xffffff;

    vert[0].color = vert[1].color = vert[2].color = vert[3].color = col;

    // set uvs
    vert[0].tu = 186.0f / 256.0f;
    vert[0].tv = 236.0f / 256.0f;
    vert[1].tu = 246.0f / 256.0f;
    vert[1].tv = 235.0f / 256.0f;
    vert[2].tu = 246.0f / 256.0f;
    vert[2].tv = 256.0f / 256.0f;
    vert[3].tu = 186.0f / 256.0f;
    vert[3].tv = 256.0f / 256.0f;

    // Draw background poly
    SET_TPAGE(TPAGE_LOADING);
    FOG_OFF();
    BLEND_SRC(D3DBLEND_SRCALPHA);
    BLEND_DEST(D3DBLEND_INVSRCALPHA);
    BLEND_ALPHA();

    DRAW_PRIM(D3DPT_TRIANGLEFAN, FVF_TEX1, vert, 4, D3DDP_DONOTUPDATEEXTENTS);
}

////////////////////
// render lantern //
////////////////////

void RenderLantern(OBJECT *obj)
{
    long b;
    FACING_POLY poly;
    VEC pos;
    MODEL_RGB *mrgb = (MODEL_RGB*)&poly.RGB;
    LANTERN_OBJ *lantern = (LANTERN_OBJ*)obj->Data;

// render lantern

    RenderObject(obj);

// render glow

    poly.Tpage = TPAGE_FX1;
    poly.U = 129.0f / 256.0f;
    poly.V = 1.0f / 256.0f;
    poly.Usize = 62.0f / 256.0f;
    poly.Vsize = 62.0f / 256.0f;
    poly.Xsize = poly.Ysize = 64.0f;

    FTOL(lantern->Brightness * 255.0f, b);
    mrgb->r = (unsigned char)b;
    mrgb->g = (unsigned char)(b * 82 / 100);
    mrgb->b = (unsigned char)(b * 48 / 100);

    pos = obj->body.Centre.Pos;
    pos.v[Y] -= 40.0f;

    DrawFacingPoly(&pos, &poly, 1, -64.0f);
    DrawFacingPoly(&pos, &poly, 1, -64.0f);
}

///////////////////
// render skybox //
///////////////////

void RenderSkybox(void)
{
    long i, j, clip[8];
    VERTEX_TEX1 vert[8];
//$REMOVED    float zres;
    MAT mat, viewmat;

// setup draw verts

    DrawVertsTEX1[0].tu = DrawVertsTEX1[3].tu = 0.0f;
    DrawVertsTEX1[1].tu = DrawVertsTEX1[2].tu = 1.0f;
    DrawVertsTEX1[0].tv = DrawVertsTEX1[1].tv = 0.0f;
    DrawVertsTEX1[2].tv = DrawVertsTEX1[3].tv = 1.0f;

//$MODIFIED
//    zres = (float)(1 << ZedBufferFormat.dwZBufferBitDepth);
//    DrawVertsTEX1[0].sz = DrawVertsTEX1[1].sz = DrawVertsTEX1[2].sz = DrawVertsTEX1[3].sz = (zres - 1.0f) / zres;
    DrawVertsTEX1[0].sz
     = DrawVertsTEX1[1].sz
      = DrawVertsTEX1[2].sz
        = DrawVertsTEX1[3].sz = 0.99999f;  //$REVISIT:  This is ugly. Maybe we should use 1.0, and ensure ZCMP is LESSEQUAL
//$END_MODIFICATIONS
    DrawVertsTEX1[0].color = DrawVertsTEX1[1].color = DrawVertsTEX1[2].color = DrawVertsTEX1[3].color = 0xffffffff;

// transform each vert

    mat.mv[U] = FLD_GravityVector;
    SetVector(&mat.mv[R], mat.m[UY], -mat.m[UX], 0);
    NormalizeVector(&mat.mv[R]);
    CrossProduct(&mat.mv[R], &mat.mv[U], &mat.mv[L]);
    CrossProduct(&mat.mv[U], &mat.mv[L], &mat.mv[R]);
    MulMatrix(&ViewMatrixScaled, &mat, &viewmat);

    for (i = 0 ; i < 8 ; i++)
    {
        RotTransPersVectorZleave(&viewmat, &ZeroVector, &SkyboxVerts[i], &vert[i].sx);

        clip[i] = 0;
        if (vert[i].sx < ScreenLeftClipGuard) clip[i] |= CLIP_LEFT;
        else if (vert[i].sx > ScreenRightClipGuard) clip[i] |= CLIP_RIGHT;
        if (vert[i].sy < ScreenTopClipGuard) clip[i] |= CLIP_TOP;
        else if (vert[i].sy > ScreenBottomClipGuard) clip[i] |= CLIP_BOTTOM;
        if ((1.0f / vert[i].rhw) < RenderSettings.NearClip) clip[i] |= CLIP_NEAR;
        else if (vert[i].sz >= 1) clip[i] |= CLIP_FAR;
    }

// render each side

    ZCMP(D3DCMP_ALWAYS);

    for (i = 0 ; i < 6 ; i++)
    {
        if (clip[SkyboxIndex[i][0]] & clip[SkyboxIndex[i][1]] & clip[SkyboxIndex[i][2]] & clip[SkyboxIndex[i][3]])
            continue;

        for (j = 0 ; j < 4 ; j++)
        {
            DrawVertsTEX1[j].sx = vert[SkyboxIndex[i][j]].sx;
            DrawVertsTEX1[j].sy = vert[SkyboxIndex[i][j]].sy;
            DrawVertsTEX1[j].rhw = vert[SkyboxIndex[i][j]].rhw;
        }

        SET_TPAGE(TPAGE_MISC3 + (short)i);
        DRAW_PRIM(D3DPT_TRIANGLEFAN, FVF_TEX1, DrawVertsTEX1, 4, D3DDP_DONOTUPDATEEXTENTS);
    }

    ZCMP(D3DCMP_LESSEQUAL);
}

///////////////////
// render slider //
///////////////////

void RenderSlider(OBJECT *obj)
{
    VEC vec;
    REAL dist;

// save pos

    CopyVec(&obj->body.Centre.Pos, &vec);

// offset

    dist = Camera[CameraCount].WPos.v[X] - obj->body.Centre.Pos.v[X];
    obj->body.Centre.Pos.v[X] -= dist * 0.004f;

// render

    RenderObject(obj);

// restore pos

    CopyVec(&vec, &obj->body.Centre.Pos);
}

/////////////////
// render rain //
/////////////////

void RenderRain(OBJECT *obj)
{
    long i, j;
    RAIN_OBJ *rain = (RAIN_OBJ*)obj->Data;
    RAINDROP *raindrop = rain->Drop;
    VERTEX_TEX0 *vert;
    VEC pos;
    REAL scrmin, scrmax, z;
    FACING_POLY fp;

// only on main cam

    if (&Camera[CameraCount] != CAM_MainCamera)
        return;

// get lens + scr max

    scrmin = -RAIN_YTOL;
    scrmax = (REAL)ScreenYsize + RAIN_YTOL;

// setup facing poly for splash

    fp.Tpage = TPAGE_FX1;
    fp.Xsize = 8.0f;
    fp.Ysize = 8.0f;
    fp.Usize = 16.0f / 256.0f;
    fp.Vsize = 16.0f / 256.0f;
    fp.RGB = 0x808080;

// render each drop

    raindrop = rain->Drop;
    vert = rain->Vert;

    for (i = 0 ; i < RAINDROP_NUM ; i++, raindrop++)
    {
        if (raindrop->Mode == RAINDROP_FALL)
        {
            VecPlusScalarVec(&raindrop->Pos, -0.05f, &raindrop->Velocity, &pos);

            RotTransPersVectorZleave(&ViewMatrixScaled, &ViewTransScaled, &pos, &vert[0].sx);
            RotTransPersVectorZleave(&ViewMatrixScaled, &ViewTransScaled, &raindrop->Pos, &vert[1].sx);

            z = 1.0f / vert[1].rhw;
            if ((z < RAIN_ZMIN || z > RAIN_ZMAX) || (z > 0.0f && (vert[1].sy < scrmin || vert[1].sy > scrmax)))
            {
                raindrop->Mode = RAINDROP_SLEEP;
                raindrop->Timer = 0.0f;
            }

            vert += 2;
        }

        else if (raindrop->Mode == RAINDROP_SPLASH)
        {
            j = (long)((0.25f - raindrop->Timer) * 16.0f);
            fp.U = (144.0f + (float)(j * 16)) / 256.0f;
            fp.V = 240.0f / 256.0f;
            DrawFacingPoly(&raindrop->Pos, &fp, 1, -16.0f);
        }
    }

// draw

    FlushPolyBuckets();

    SET_TPAGE(-1);

    BLEND_ALPHA();
    BLEND_SRC(D3DBLEND_SRCALPHA);
    BLEND_DEST(D3DBLEND_INVSRCALPHA);

    SET_RENDER_STATE(D3DRENDERSTATE_ALPHATESTENABLE, FALSE);

    DRAW_PRIM(D3DPT_LINELIST, FVF_TEX0, rain->Vert, (long)(vert - rain->Vert), D3DDP_DONOTUPDATEEXTENTS);

    SET_RENDER_STATE(D3DRENDERSTATE_ALPHATESTENABLE, TRUE);

    BLEND_OFF();
}

//////////////////////
// render lightning //
//////////////////////

void RenderLightning(OBJECT *obj)
{
}

///////////////////////
// render ship light //
///////////////////////

void RenderShipLight(OBJECT *obj)
{
    VEC pos;
    FACING_POLY poly;

// render light

    RenderObject(obj);

// render glow

    poly.Tpage = TPAGE_FX1;
    poly.U = 129.0f / 256.0f;
    poly.V = 1.0f / 256.0f;
    poly.Usize = 62.0f / 256.0f;
    poly.Vsize = 62.0f / 256.0f;
    poly.Xsize = poly.Ysize = 64.0f;
    poly.RGB = 0xffd17a;

    VecPlusScalarVec(&obj->body.Centre.Pos, 80.0f, &FLD_GravityVector, &pos);

    DrawFacingPoly(&pos, &poly, 1, -64.0f);
}

/////////////////////
// render waterbox //
/////////////////////

void RenderWaterBox(OBJECT *obj)
{
    WATERBOX_OBJ *wb = (WATERBOX_OBJ*)obj->Data;
    CAMERA *cam = &Camera[CameraCount];

// is current camera inside box?

    if (cam->WPos.v[X] < wb->Box.Xmin || cam->WPos.v[X] > wb->Box.Xmax || 
        cam->WPos.v[Y] < wb->Box.Ymin || cam->WPos.v[Y] > wb->Box.Ymax || 
        cam->WPos.v[Z] < wb->Box.Zmin || cam->WPos.v[Z] > wb->Box.Zmax)
            return;

// yep, set underwater flag

    cam->UnderWater = TRUE;
}

////////////////////////////////////
// render under water camera poly //
////////////////////////////////////

void RenderUnderWaterPoly(void)
{

// create water poly

    DrawVertsTEX0[0].color = DrawVertsTEX0[1].color = DrawVertsTEX0[2].color = DrawVertsTEX0[3].color = 0x003040;
    DrawVertsTEX0[0].rhw = DrawVertsTEX0[1].rhw = DrawVertsTEX0[2].rhw = DrawVertsTEX0[3].rhw = 1.0f;

    DrawVertsTEX0[0].sx = DrawVertsTEX0[3].sx = ScreenLeftClip;
    DrawVertsTEX0[1].sx = DrawVertsTEX0[2].sx = ScreenRightClip;
    DrawVertsTEX0[0].sy = DrawVertsTEX0[1].sy = ScreenTopClip;
    DrawVertsTEX0[2].sy = DrawVertsTEX0[3].sy = ScreenBottomClip;

// render

    FOG_OFF();
    ZBUFFER_OFF();
    SET_TPAGE(-1);
    BLEND_ALPHA();
    BLEND_SRC(D3DBLEND_ONE);
    BLEND_DEST(D3DBLEND_ONE);

    SET_RENDER_STATE(D3DRENDERSTATE_ALPHATESTENABLE, FALSE);
    DRAW_PRIM(D3DPT_TRIANGLEFAN, FVF_TEX0, DrawVertsTEX0, 4, D3DDP_DONOTUPDATEEXTENTS);
    SET_RENDER_STATE(D3DRENDERSTATE_ALPHATESTENABLE, TRUE);

    BLEND_OFF();
    ZBUFFER_ON();
}

/////////////////////////
// render ripple model //
/////////////////////////

void RenderRipple(OBJECT *obj)
{
    RIPPLE_OBJ *ripple = (RIPPLE_OBJ*)obj->Data;

// visicok test

    if (ripple->VisiMask & CamVisiMask)
        return;

// render ripple model

    RenderObject(obj);
}

/////////////////////////
// render stream model //
/////////////////////////

void RenderStream(OBJECT *obj)
{
    STREAM_OBJ *stream = (STREAM_OBJ*)obj->Data;

// visicok test

    if (stream->VisiMask & CamVisiMask)
        return;

// render ripple model

    RenderObject(obj);
}



/////////////////////////////////////////////////////////////////////
// AerialMotionBlur()
//
// pP0      Most opaque point #1
// pP1      Most opaque point #2
// pP2      Least opaque point #1
// pP3      Least opaque point #2
/////////////////////////////////////////////////////////////////////
void MotionBlurInit(void)
{
    gcMotionBlur = 0;
}

// Add blur segment
void MotionBlurAdd(VEC* pP0, VEC* pP1, VEC* pP2, VEC* pP3, long rgb1, long rgb2)
{
    if (gcMotionBlur >= MOTION_BLUR_MAX)
        return;

    CopyVec(pP0, &gMotionBlurCoords[gcMotionBlur][0]);
    CopyVec(pP1, &gMotionBlurCoords[gcMotionBlur][1]);
    CopyVec(pP2, &gMotionBlurCoords[gcMotionBlur][2]);
    CopyVec(pP3, &gMotionBlurCoords[gcMotionBlur][3]);

    gMotionBlurColors[gcMotionBlur][0] = rgb1;
    gMotionBlurColors[gcMotionBlur][1] = rgb2;

    gcMotionBlur++;
}

// Render blur segments
void MotionBlurRender(void)
{
    long    rgb[4];
    int     i;

    FOG_OFF();
    ZWRITE_OFF();
    SET_TPAGE(-1);
    BLEND_ALPHA();
    BLEND_SRC(D3DBLEND_SRCALPHA);
    BLEND_DEST(D3DBLEND_INVSRCALPHA);

    for (i = 0; i < gcMotionBlur; i++)
    {
        rgb[0] = gMotionBlurColors[i][0] | (32 << 24);
        rgb[1] = gMotionBlurColors[i][1] | (32 << 24);
        rgb[2] = gMotionBlurColors[i][1] | (0 << 24);
        rgb[3] = gMotionBlurColors[i][0] | (0 << 24);

        DrawNearClipPolyTEX0(&gMotionBlurCoords[i][0], rgb, 4);
    }

    BLEND_OFF();
    ZWRITE_ON();
}

////////////////////////////
// dolphin render handler //
////////////////////////////

void RenderDolphin(OBJECT *obj)
{
    DOLPHIN_OBJ *dolphin = (DOLPHIN_OBJ*)obj->Data;
    REAL z;

// set visibility

    if (dolphin->VisiMask & CamVisiMask)
        return;

    if (TestSphereToFrustum(&obj->body.Centre.Pos, 768.0f, &z) == SPHERE_OUT)
        return;

    obj->renderflag.visible = TRUE;
}


/////////////////////////////////////////////////
// render stream model
/////////////////////////////////////////////////
void RenderFlag(OBJECT *obj)
{
    FLAG_DATA_OBJ*  pFlag = (FLAG_DATA_OBJ*)obj->Data;
    FLAG_PARTICLE*  pP;
    VERTEX_TEX1*    pVert;
    int             i;
    long            visflag;
    REAL            z;

// Make sure flag is visible
    if (pFlag->VisiMask & CamVisiMask)
        return;

    visflag = TestSphereToFrustum(&obj->body.Centre.Pos, TO_LENGTH(500), &z);
    if (visflag == SPHERE_OUT) return;

    obj->renderflag.visible = TRUE;

// Transform
    pP = pFlag->pParticle;
    pVert = pFlag->pVert;
    for (i = 0; i < pFlag->cVert; i++)
    {
        RotTransPersVectorZleave(&ViewMatrixScaled, &ViewTransScaled, &pP->pos, &pVert->sx);
        pP++;
        pVert++;
    }

// Draw
    FlushPolyBuckets();

    SET_TPAGE(pFlag->tpage);
    DRAW_PRIM_INDEX(D3DPT_TRIANGLELIST, FVF_TEX1, pFlag->pVert, pFlag->cVert, pFlag->pIndex, pFlag->cTris, D3DDP_DONOTUPDATEEXTENTS);
}

