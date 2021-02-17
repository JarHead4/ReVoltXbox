//-----------------------------------------------------------------------------
// File: weapon.cpp
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
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
#include "drawobj.h"
#include "spark.h"
#ifdef _PC
#include "shadow.h"
#endif
#include "obj_init.h"
#include "visibox.h"
#ifdef _PC
#include "mirror.h"
#include "input.h"
#include "text.h"
#endif
#include "camera.h"
#include "ai.h"
#include "replay.h"
#include "light.h"
#include "pickup.h"
#include "ui_TitleScreen.h"

#ifdef _N64
#include "ISound.h"
#endif

#include "SoundEffectEngine.h"


// prototypes

void FireWorkMove(OBJECT *obj);
void FireworkExplode(OBJECT *obj);
void TurboAIHandler(OBJECT *obj);
void TurboMoveHandler(OBJECT *obj);
void PuttyBombMove(OBJECT *obj);
void AttachWeaponCamera(OBJECT *obj);
void DetachWeaponCamera(OBJECT *obj);
void FireWorkMoveAndHome(OBJECT *obj);
void MoveShockwave(OBJECT *obj);

// globals

long OilSlickCount;
OILSLICK_LIST OilSlickList[OILSLICK_LIST_MAX];

static VEC WaterBombVel = {0.0f, -500.0f, 4000.0f};
static VEC WaterBombOff = {0.0f, -32.0f, 0.0f};
static VEC WaterBombFireOff = {0.0f, -16.0f, 80.0f};
static VEC BombSmokeVel = {0.0f, -80.0f, 0.0f};


//////////////////////////
// reset oil slick list //
//////////////////////////

void ResetOilSlickList(void)
{
    OilSlickCount = 0;
}

///////////////////////////////
// create multiplayer weapon //
///////////////////////////////
#ifdef _PC

void SendWeaponNew(VEC *pos, MAT *mat, unsigned long id, long weapon, GLOBAL_ID GlobalID)
{
    const int cbHeader = sizeof(MSG_HEADER);
    WEAPON_REMOTE_DATA* pData = (WEAPON_REMOTE_DATA*)(SendMsgBuffer + cbHeader);
    QUATERNION quat;

// setup header

    SetSendMsgHeader( MSG_WEAPON_DATA );

// set pos

    CopyVec(pos, &pData->Pos);

// set quat

    MatToQuat(mat, &quat);
    pData->QuatX = (char)(quat.v[VX] * REMOTE_QUAT_SCALE);
    pData->QuatY = (char)(quat.v[VY] * REMOTE_QUAT_SCALE);
    pData->QuatZ = (char)(quat.v[VZ] * REMOTE_QUAT_SCALE);
    pData->QuatW = (char)(quat.v[S]  * REMOTE_QUAT_SCALE);

// set id

    pData->ID = id;

// set weapon

    pData->Weapon = weapon;

// set global ID

    pData->GlobalID = GlobalID;

// set time

    pData->Time = TotalRaceTime;

// send

    QueueMessage( SendMsgBuffer, cbHeader + sizeof(WEAPON_REMOTE_DATA) );
}

/////////////////////////
// process weapon data //
/////////////////////////

int ProcessWeaponNew()
{
    const int cbHeader = sizeof(MSG_HEADER);
    const int cbMessageRecv = cbHeader + sizeof(WEAPON_REMOTE_DATA);
    WEAPON_REMOTE_DATA* pData = (WEAPON_REMOTE_DATA*)(RecvMsgBuffer + cbHeader);
    PLAYER* pPlayer;
    OBJECT* pObj;
    long flag[2];
    float lag;
    void *w;
    VEC pos;
    MAT mat;
    QUATERNION quat;

// get player

    pPlayer = GetPlayerFromPlayerID(pData->ID);
    if (!pPlayer)
        goto Return;

    flag[0] = (long)pPlayer;
    flag[1] = FALSE;

// reset player pickup

    pPlayer->PickupType = PICKUP_TYPE_NONE;
    pPlayer->PickupNum = 0;

// get pos

    CopyVec(&pData->Pos, &pos);

// get mat

    quat.v[VX] = (REAL)pData->QuatX / REMOTE_QUAT_SCALE;
    quat.v[VY] = (REAL)pData->QuatY / REMOTE_QUAT_SCALE;
    quat.v[VZ] = (REAL)pData->QuatZ / REMOTE_QUAT_SCALE;
    quat.v[S]  = (REAL)pData->QuatW / REMOTE_QUAT_SCALE;
    NormalizeQuat(&quat);
    QuatToMat(&quat, &mat);

// create weapon

    if (pData->Weapon == OBJECT_TYPE_OILSLICK_DROPPER || pData->Weapon == OBJECT_TYPE_PICKUP)
    {
        pObj = CreateObject(&pos, &mat, pData->Weapon, flag);
    }
    else
    {
        pObj = CreateObject(&pPlayer->car.Body->Centre.Pos, &pPlayer->car.Body->Centre.WMatrix, pData->Weapon, flag);
    }

    if (!pObj)
        goto Return;

    pObj->GlobalID = pData->GlobalID;
    pObj->remotehandler = NULL;
    pObj->ServerControlled = TRUE;

// adjust for lag

    lag = (float)(TotalRaceTime - pData->Time) / 1000.0f;
    w = pObj->Data;

    switch (pData->Weapon)
    {
        case OBJECT_TYPE_SHOCKWAVE:
            ((SHOCKWAVE_OBJ*)w)->Age += lag;

            break;

        case OBJECT_TYPE_FIREWORK:
            ((FIREWORK_OBJ*)w)->Age += lag;
            break;

        case OBJECT_TYPE_PUTTYBOMB:
            ((PUTTYBOMB_OBJ*)w)->Timer -= lag;
            break;

        case OBJECT_TYPE_WATERBOMB:
            ((WATERBOMB_OBJ*)w)->Age += lag;
            break;

        case OBJECT_TYPE_ELECTROPULSE:
            ((ELECTROPULSE_OBJ*)w)->Age += lag;
            break;

        case OBJECT_TYPE_OILSLICK_DROPPER:
            ((OILSLICK_DROPPER_OBJ*)w)->Age += lag;
            break;

        case OBJECT_TYPE_CHROMEBALL:
            ((CHROMEBALL_OBJ*)w)->Age += lag;
            break;

        case OBJECT_TYPE_TURBO:
            ((TURBO_OBJ*)w)->Age += lag;
            break;

        case OBJECT_TYPE_PICKUP:
            ((PICKUP_OBJ*)w)->Timer += lag;
            break;
    }

Return:
// return num bytes in message received
    return cbMessageRecv;
}

////////////////////////////////
// send target status message //
////////////////////////////////

void SendTargetStatus(unsigned long id, long status)
{
    const int cbHeader = sizeof(MSG_HEADER);
    TARGET_STATUS_DATA* pData = (TARGET_STATUS_DATA*)(SendMsgBuffer + cbHeader);

    SetSendMsgHeader( MSG_TARGET_STATUS_DATA );

    pData->ID = id;
    pData->Status = status;

    QueueMessage( SendMsgBuffer, cbHeader + sizeof(TARGET_STATUS_DATA) );
}

///////////////////////////////////
// process target status message //
///////////////////////////////////

int ProcessTargetStatus(void)
{
    const int cbHeader = sizeof(MSG_HEADER);
    const int cbMessageRecv = cbHeader + sizeof(TARGET_STATUS_DATA);
    TARGET_STATUS_DATA* pData = (TARGET_STATUS_DATA*)(RecvMsgBuffer + cbHeader);
    PLAYER* pPlayer;

    pPlayer = GetPlayerFromPlayerID(pData->ID);
    if (!pPlayer)
        goto Return;

    if (pData->Status)
        PlayerTargetOn(pPlayer);
    else
        PlayerTargetOff(pPlayer);

Return:
    // return num bytes in message received
    return cbMessageRecv;
}
#endif

/////////////////
// init weapon //
/////////////////

long InitShockwave(OBJECT *obj, long *flags)
{
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
    obj->movehandler = (MOVE_HANDLER)MoveShockwave;

// setup remote handler

#ifdef _PC
    obj->remotehandler = (REMOTE_HANDLER)SendObjectData;
    InitRemoteObjectData(obj);
#endif

// sfx

#ifndef _PSX
 #ifdef OLD_AUDIO
    PlaySfx3D(SFX_SHOCKWAVE_FIRE, SFX_MAX_VOL, 22050, &obj->body.Centre.Pos, 2);
 #else // !OLD_AUDIO
    g_SoundEngine.Play3DSound( EFFECT_ShockFire, FALSE, obj );
 #endif // !OLD_AUDIO
#endif

// Physical properties

    obj->CollType = COLL_TYPE_BODY;

    obj->body.Centre.Mass = Real(1.0);
    obj->body.Centre.InvMass = ONE / Real(1.0);
    SetMat(&obj->body.BodyInertia, Real(100), ZERO, ZERO, ZERO, Real(100), ZERO, ZERO, ZERO, Real(100));
    SetMat(&obj->body.BodyInvInertia, ONE / Real(100), ZERO, ZERO, ZERO, ONE / Real(100), ZERO, ZERO, ZERO, ONE / Real(100));
    GetFrameInertia(&obj->body.BodyInvInertia, &obj->body.Centre.WMatrix, &obj->body.WorldInvInertia);

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
#ifdef _N64
    obj->body.CollSkin.Sphere = &obj->Sphere;
#else
    obj->body.CollSkin.Sphere = (SPHERE *)malloc(sizeof(SPHERE));
#endif
    SetVecZero(&obj->body.CollSkin.Sphere[0].Pos);
    obj->body.CollSkin.Sphere[0].Radius = SHOCKWAVE_RAD;
    obj->body.CollSkin.NSpheres = 1;
    CreateCopyCollSkin(&obj->body.CollSkin);
    MakeTightLocalBBox(&obj->body.CollSkin);
    InitWorldSkin(&obj->body.CollSkin, &obj->body.Centre.Pos, &obj->body.Centre.WMatrix);
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
#ifndef _PSX
 #ifdef OLD_AUDIO
    obj->Sfx3D = CreateSfx3D(SFX_SHOCKWAVE, SFX_MAX_VOL, 22050, TRUE, &obj->body.Centre.Pos, 2);
 #else // !OLD_AUDIO
    g_SoundEngine.Play3DSound( EFFECT_Shock, FALSE, obj );
 #endif // !OLD_AUDIO
#endif
// Add a force field

    /*SetBBox(&bBox, -shockwave->Reach, shockwave->Reach, -shockwave->Reach, shockwave->Reach, -shockwave->Reach, shockwave->Reach);
    SetVec(&bVec, shockwave->Reach / 2, shockwave->Reach / 2, shockwave->Reach / 2);
    //VecPlusScalarVec(&obj->body.Centre.Vel, -Real(12000), &UpVec, &dir);
    SetVec(&dir, obj->body.Centre.Vel.v[X], obj->body.Centre.Vel.v[Y] + Real(12000), obj->body.Centre.Vel.v[Z]);
    NormalizeVec(&dir);
    VecEqScalarVec(&axis, Real(50000), &obj->body.Centre.WMatrix.mv[R]);
    obj->Field = AddLinearTwistField(
        obj->player->ownobj->ObjID, 
        FIELD_PRIORITY_MID,
        &obj->body.Centre.Pos,
        &obj->body.Centre.WMatrix,
        &bBox,
        &bVec,
        &dir,
        -Real(6000),
        &axis,
        ZERO);

    obj->FieldPriority = FIELD_PRIORITY_MAX;*/

// return OK

    return TRUE;
}


////////////////////////////////////////////////////////////////
//
// Move Shockwave and blow away nearby cars
//
////////////////////////////////////////////////////////////////

#define SHOCKWAVE_REACH_SQ Real(1000000)

void MoveShockwave(OBJECT *obj)
{
    PLAYER *player;
    REAL dRLenSq, impUp, torque;
    VEC dR;

    // Move the shockwave
    MOV_MoveBody(obj);

    // Check for nearby players
    for (player = PLR_PlayerHead; player != NULL; player = player->next) {

        if (player == obj->player) continue;

        // Is the car near enough?
        VecMinusVec(&player->car.Body->Centre.Pos, &obj->body.Centre.Pos, &dR);
        dR.v[Y] *= 10;
        dRLenSq = VecDotVec(&dR, &dR);
        if (dRLenSq > SHOCKWAVE_REACH_SQ) {
            continue;
        }

        // Lift Car off floor
        impUp = player->car.Body->Centre.Mass * (ONE - dRLenSq / SHOCKWAVE_REACH_SQ) * Real(4500) * TimeStep;
        player->car.Body->Centre.Impulse.v[Y] -= impUp;

        // Spin car a bit
        torque = player->car.Body->BodyInertia.m[XX] * (ONE - dRLenSq / SHOCKWAVE_REACH_SQ) * Real(100) * TimeStep;
        VecPlusEqScalarVec(&player->car.Body->AngImpulse, torque, &player->car.Body->Centre.WMatrix.mv[L]);

        // Make sure player reacts to it
        player->car.Body->Stacked = FALSE;
        player->car.Body->NoContactTime = ZERO;
        player->car.Body->NoMoveTime = ZERO;

    }


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

// setup remote handler

#ifdef _PC
    obj->remotehandler = (REMOTE_HANDLER)SendObjectData;
    InitRemoteObjectData(obj);
#endif

// remember owner player

    obj->player = (PLAYER*)flags[0];

    // if no player owner, just want an explosion
    if (obj->player == NULL) {
        firework->Exploded = FALSE;
        firework->Age = FIREWORK_MAX_AGE;
        firework->SmokeTime = ZERO;
        firework->SparkTime = ZERO;
        firework->Target = NULL;
        firework->Trail = NULL;
        firework->TrailTime = ZERO;
        obj->DefaultModel = -1;
        return TRUE;
    }

// load default model

    obj->DefaultModel = LoadOneLevelModel(LEVEL_MODEL_FIREWORK, FALSE, obj->renderflag, TPAGE_FX1);
    if (obj->DefaultModel == -1) return FALSE;

#ifdef _N64
    obj->renderhandler = (COLL_HANDLER)RenderFirework;
#endif



    playerMat = &obj->player->car.Body->Centre.WMatrix;

    // misc
    firework->Exploded = FALSE;
    firework->Age = ZERO;
    firework->SmokeTime = ZERO;
    firework->SparkTime = ZERO;
    firework->Target = obj->player->PickupTarget;

    // offset pos
    RotTransVector(&obj->body.Centre.WMatrix, &obj->body.Centre.Pos, &obj->player->car.WeaponOffset, &obj->body.Centre.Pos);
    obj->body.Centre.Pos.v[Y] += obj->player->car.Body->CollSkin.TightBBox.YMin;

    // Stop firework appearing halfway through walls
    VecPlusScalarVec(&obj->body.Centre.Pos, TO_LENGTH(Real(35)), &obj->player->car.Body->Centre.WMatrix.mv[L], &dR);
    if (!LineOfSight(&obj->player->car.Body->Centre.Pos, &dR)){
        // No line of sight - make firework explode
        firework->Exploded = FALSE;
        firework->Age = FIREWORK_MAX_AGE;
        firework->SmokeTime = ZERO;
        firework->SparkTime = ZERO;
        firework->Target = NULL;
        firework->Trail = NULL;
        firework->TrailTime = ZERO;
        obj->DefaultModel = -1;
        return TRUE;
    }


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
    obj->body.CollSkin.AllowObjColls = TRUE;
    obj->body.CollSkin.NConvex = LevelModel[obj->DefaultModel].CollSkin.NConvex;
    obj->body.CollSkin.NSpheres = LevelModel[obj->DefaultModel].CollSkin.NSpheres;
    obj->body.CollSkin.Convex = LevelModel[obj->DefaultModel].CollSkin.Convex;
    obj->body.CollSkin.Sphere = LevelModel[obj->DefaultModel].CollSkin.Sphere;
    CopyBBox(&LevelModel[obj->DefaultModel].CollSkin.TightBBox, &obj->body.CollSkin.TightBBox);
    CreateCopyCollSkin(&obj->body.CollSkin);
    InitWorldSkin(&obj->body.CollSkin, &obj->body.Centre.Pos, &obj->body.Centre.WMatrix);
    BuildWorldSkin(&obj->body.CollSkin, &obj->body.Centre.Pos, &obj->body.Centre.WMatrix);

    // vel and matrix
    if (firework->Target != NULL) {
        VecMinusVec(&firework->Target->body.Centre.Pos, &obj->body.Centre.Pos, &dR);
        VecMinusVec(&obj->body.Centre.Pos, &firework->Target->body.Centre.Pos, &dir);
        dRLen = VecLen(&dR) / 2;
        speedMod = (dRLen - TARGET_RANGE_MIN) / (TARGET_RANGE_MAX - TARGET_RANGE_MIN);
        speedMod = (speedMod * speedMod * speedMod * speedMod) / 8;
    } else {
        dRLen = TARGET_RANGE_MAX / 3;
        VecEqScalarVec(&dir, dRLen / TARGET_RANGE_MAX - ONE, &playerMat->mv[L]);
        speedMod = ONE / 4;
    }
    VecPlusEqScalarVec(&dir, dRLen / TARGET_RANGE_MAX, &playerMat->mv[U]);
    NormalizeVec(&dir);
    CopyVec(&dir, &obj->body.Centre.WMatrix.mv[U]);
    BuildMatrixFromUp(&obj->body.Centre.WMatrix);
    SetBodyPos(&obj->body, &obj->body.Centre.Pos, &obj->body.Centre.WMatrix);
    VecPlusScalarVec(&obj->player->car.Body->Centre.Vel, 1500 * speedMod, &dir, &obj->body.Centre.Vel);

    // setup sfx
#ifndef _PSX
 #ifdef OLD_AUDIO
    obj->Sfx3D = CreateSfx3D(SFX_FIREWORK, SFX_MAX_VOL, 22050, FALSE, &obj->body.Centre.Pos, 2);
 #else // !OLD_AUDIO
    g_SoundEngine.Play3DSound( EFFECT_FireworkFire, FALSE, obj );
 #endif // !OLD_AUDIO
#endif
    // Set up the trail
    firework->Trail = GetFreeTrail(TRAIL_SMOKE);
    if (firework->Trail != NULL) {
        CopyVec(&obj->body.Centre.Pos, &firework->Trail->Pos[0]);
        firework->TrailTime = ZERO;
    }

    // Add the weapon camera if it is available and wanted
#if FALSE//ndef _N64
    if (firework->Target != NULL && (obj->player == PLR_LocalPlayer || firework->Target == PLR_LocalPlayer->ownobj)) {
        AttachWeaponCamera(obj);
    }
#endif

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

// remember owner player

    obj->player = (PLAYER*)flags[0];

// Can't have two bombs on one player

    if (obj->player->car.IsBomb && !flags[1]) return FALSE;

// set render flags

    obj->renderflag.envmap = FALSE;
    obj->renderflag.light = FALSE;
    obj->renderflag.reflect = FALSE;
    obj->renderflag.meshfx = FALSE;

// setup handlers

    obj->aihandler = (AI_HANDLER)PuttyBombHandler;
    obj->movehandler = (MOVE_HANDLER)PuttyBombMove;
#ifdef _N64
    obj->renderhandler = (RENDER_HANDLER)RenderPuttyBomb;
#else
    obj->renderhandler = NULL;
#endif


// load bang model
#ifdef _PC
    obj->DefaultModel = LoadOneLevelModel(LEVEL_MODEL_BOMBBALL, FALSE, obj->renderflag, 0);
    if (obj->DefaultModel == -1)
        return FALSE;
#else
    obj->DefaultModel = -1;
#endif

// No collision

    obj->CollType = COLL_TYPE_NONE;
    obj->body.CollSkin.AllowObjColls = FALSE;
    obj->body.CollSkin.AllowWorldColls = FALSE;
    
// alloc + setup bomb
#ifdef _PC
    obj->Data = malloc(sizeof(PUTTYBOMB_OBJ) + sizeof (PUTTYBOMB_VERT) * LevelModel[obj->DefaultModel].Model.VertNum);
#endif
#ifdef _N64
    obj->Data = malloc(sizeof(PUTTYBOMB_OBJ));
#endif
    if (!obj->Data) return FALSE;
    bomb = (PUTTYBOMB_OBJ*)obj->Data;

    obj->player->car.IsBomb = TRUE;
    obj->player->car.NoReturnTimer = PUTTYBOMB_NORETURN_TIME;
    bomb->OrigAerialLen = obj->player->car.Aerial.Length;

    if (!flags[1])  // puttybomb
    {
        bomb->Timer = PUTTYBOMB_COUNTDOWN;
        obj->player->car.WillDetonate = FALSE;
    }
    else    // clone bang
    {
        bomb->Timer = 0.0f;
        obj->player->car.WillDetonate = TRUE;
    }

// init bang model + verts
#ifdef _PC
    model = &LevelModel[obj->DefaultModel].Model;

    for (i = 0 ; i < model->PolyNum ; i++)
    {
        model->PolyPtr[i].Tpage = TPAGE_FX1;
        model->PolyPtr[i].Type |= POLY_DOUBLE | POLY_SEMITRANS | POLY_SEMITRANS_ONE;

        *(long*)&model->PolyRGB[i].rgb[0] = 0xffffff;
        *(long*)&model->PolyRGB[i].rgb[1] = 0xffffff;
        *(long*)&model->PolyRGB[i].rgb[2] = 0xffffff;
        *(long*)&model->PolyRGB[i].rgb[3] = 0xffffff;
    }

    vert = (PUTTYBOMB_VERT*)(bomb + 1);
    for (i = 0 ; i < LevelModel[obj->DefaultModel].Model.VertNum ; i++)
    {
        vert[i].Time = frand(RAD);
        vert[i].TimeAdd = frand(5.0f) + 5.0f;
        if (rand() & 1) vert[i].TimeAdd = -vert[i].TimeAdd;
    }

// init sfx

#endif

#ifndef _PSX
    if (!flags[1])
    {
 #ifdef OLD_AUDIO
        obj->Sfx3D = CreateSfx3D(SFX_FUSE, SFX_MAX_VOL, 22050, TRUE, &obj->player->car.Body->Centre.Pos, 2);
 #else // !OLD_AUDIO
        g_SoundEngine.PlaySubmixedSound( EFFECT_Fuse, TRUE, obj->player->car.pSourceMix, &obj->pSfxInstance );
 #endif // !OLD_AUDIO
    }
#endif
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

// setup remote handler

#ifdef _PC
    obj->remotehandler = (REMOTE_HANDLER)SendObjectData;
    InitRemoteObjectData(obj);
#endif

// load default model

    obj->DefaultModel = LoadOneLevelModel(LEVEL_MODEL_WATERBOMB, FALSE, obj->renderflag, 0);
    if (obj->DefaultModel == -1) return FALSE;

// remember owner player

    obj->player = (PLAYER*)flags[0];

// setup waterbomb

    bomb->Age = 0.0f;
    bomb->BangTol = frand(WATERBOMB_BANG_VAR) + WATERBOMB_BANG_MIN;
    bomb->ScalarHoriz = Real(1);
    bomb->ScalarVert = Real(1);

// pos / vel

//  RotTransVector(&obj->body.Centre.WMatrix, &obj->body.Centre.Pos, &obj->player->car.WeaponOffset, &obj->body.Centre.Pos);
    RotTransVector(&obj->body.Centre.WMatrix, &obj->body.Centre.Pos, &WaterBombFireOff, &obj->body.Centre.Pos);
    RotVector(&obj->body.Centre.WMatrix, &WaterBombVel, &vec);
//  AddVector(&obj->player->car.Body->Centre.Vel, &vec, &obj->body.Centre.Vel);
    SetVector(&obj->body.Centre.Vel, vec.v[X] + obj->player->car.Body->Centre.Vel.v[X], vec.v[Y], vec.v[Z] + obj->player->car.Body->Centre.Vel.v[Z]);

    VecEqScalarVec(&obj->body.AngVel, -15.0f, &obj->body.Centre.WMatrix.mv[R]);

// Check for line of sight to start position
    if (!LineOfSight(&obj->player->car.Body->Centre.Pos, &obj->body.Centre.Pos)) {
        // explode waterbomb straght away
        bomb->BangTol = ZERO;
        obj->body.BangMag = ONE;
        CopyVec(&obj->player->car.Body->Centre.Pos, &obj->body.Centre.Pos);
        SetPlane(&obj->body.BangPlane, 
            -obj->player->car.Body->Centre.WMatrix.m[LX],
            -obj->player->car.Body->Centre.WMatrix.m[LY],
            -obj->player->car.Body->Centre.WMatrix.m[LZ],
            ZERO);
    }


// setup sfx
#ifndef _PSX
 #ifdef OLD_AUDIO
    obj->Sfx3D = CreateSfx3D(SFX_WATERBOMB, SFX_MAX_VOL, 22050, TRUE, &obj->body.Centre.Pos, 2);
    PlaySfx3D(SFX_WATERBOMB_FIRE, SFX_MAX_VOL, 22050, &obj->body.Centre.Pos, 2);
 #else // !OLD_AUDIO
    g_SoundEngine.Play3DSound( EFFECT_WaterBombFire, FALSE, obj );
 #endif // !OLD_AUDIO
#endif
// Physical properties

    obj->body.Centre.Mass = Real(0.6f);
    obj->body.Centre.InvMass = ONE / Real(0.6f);
    SetMat(&obj->body.BodyInertia, Real(100), ZERO, ZERO, ZERO, Real(100), ZERO, ZERO, ZERO, Real(100));
    SetMat(&obj->body.BodyInvInertia, ONE / Real(100), ZERO, ZERO, ZERO, ONE / Real(100), ZERO, ZERO, ZERO, ONE / Real(100));
    GetFrameInertia(&obj->body.BodyInvInertia, &obj->body.Centre.WMatrix, &obj->body.WorldInvInertia);

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
#ifdef _N64
    obj->body.CollSkin.Sphere = &obj->Sphere;
#else
    obj->body.CollSkin.Sphere = (SPHERE *)malloc(sizeof(SPHERE));
#endif
    CopyVec(&WaterBombOff, &obj->body.CollSkin.Sphere[0].Pos);
    obj->body.CollSkin.Sphere[0].Radius = WATERBOMB_RADIUS;
    obj->body.CollSkin.NSpheres = 1;
    CreateCopyCollSkin(&obj->body.CollSkin);
    MakeTightLocalBBox(&obj->body.CollSkin);
    InitWorldSkin(&obj->body.CollSkin, &obj->body.Centre.Pos, &obj->body.Centre.WMatrix);
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

#ifdef _PC
    ram += sizeof(MODEL_POLY) * smodel->PolyNum;
    ram += sizeof(POLY_RGB) * smodel->PolyNum;
    ram += sizeof(MODEL_VERTEX) * smodel->VertNum;
    ram += sizeof(ELECTROPULSE_VERT) * smodel->VertNum;
#endif

    obj->Data = malloc(ram);
    if (!obj->Data)
        return FALSE;

// setup electro pulse

    electro = (ELECTROPULSE_OBJ*)obj->Data;
    electro->Age = 0.0f;
    electro->JumpFlag = 0;
#ifndef _PSX
 #ifdef OLD_AUDIO
    electro->ZapSfx = NULL;
 #else
    electro->pZapSound = NULL;
 #endif
#endif

// setup model

#ifdef _PC
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
#endif


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

// No collision

    obj->CollType = COLL_TYPE_NONE;
    obj->body.CollSkin.AllowObjColls = FALSE;
    obj->body.CollSkin.AllowWorldColls = FALSE;
    
// setup sfx

#ifndef _PSX
 #ifdef OLD_AUDIO
    obj->Sfx3D = CreateSfx3D(SFX_ELECTROPULSE, SFX_MAX_VOL, 22050, TRUE, &obj->body.Centre.Pos, 2);
 #else // !OLD_AUDIO
    g_SoundEngine.PlaySubmixedSound( EFFECT_Electro, TRUE, obj->player->car.pSourceMix, &obj->pSfxInstance );
 #endif // !OLD_AUDIO
#endif

// return OK

    return TRUE;
}

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
    obj->renderhandler = (RENDER_HANDLER)RenderOilSlick;

// remember owner player

    obj->player = (PLAYER*)flags[0];

// setup

    car = &obj->player->car;

    oil->Mode = 0;
    oil->Age = 0.0f;

//  CopyVec(&car->Body->Centre.Pos, &obj->body.Centre.Pos);
    VecPlusScalarVec(&obj->player->car.Body->Centre.Pos, obj->player->car.Body->CollSkin.TightBBox.ZMin, &obj->player->car.Body->Centre.WMatrix.mv[L], &obj->body.Centre.Pos);

    SetVector(&obj->body.Centre.Vel, 0, 128.0f, 0);

    SetVector(&vec, obj->body.Centre.Pos.v[X], obj->body.Centre.Pos.v[Y] + 1000.0f, obj->body.Centre.Pos.v[Z]);
    LineOfSightDist(&obj->body.Centre.Pos, &vec, &time, NULL);
    if (time > 0.0f && time < 1.0f)
        oil->LandHeight = obj->body.Centre.Pos.v[Y] + time * 1000.0f;
    else
        oil->LandHeight = vec.v[Y];

    oil->MaxSize = (REAL)flags[1];

// No collision

    obj->CollType = COLL_TYPE_NONE;
    obj->body.CollSkin.AllowObjColls = FALSE;
    obj->body.CollSkin.AllowWorldColls = FALSE;
    
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

// sfx

#ifndef _PSX
 #ifdef OLD_AUDIO
    PlaySfx3D(SFX_OILDROP, SFX_MAX_VOL, 22050, &obj->body.Centre.Pos, 2);
 #else // !OLD_AUDIO
    g_SoundEngine.Play3DSound( EFFECT_OilDrop, FALSE, obj );
 #endif // !OLD_AUDIO
#endif

// No collision

    obj->CollType = COLL_TYPE_NONE;
    obj->body.CollSkin.AllowObjColls = FALSE;
    obj->body.CollSkin.AllowWorldColls = FALSE;
    
// return OK

    return TRUE;
}

/////////////////
// init weapon //
/////////////////

long InitChromeBall(OBJECT *obj, long *flags)
{
    REAL time;
    PLANE *planePtr;
    CHROMEBALL_OBJ *ball = (CHROMEBALL_OBJ*)obj->Data;

// render flags

    obj->renderflag.envmap = FALSE;

// setup handlers

    obj->aihandler = (AI_HANDLER)ChromeBallHandler;
    obj->movehandler = (MOVE_HANDLER)MOV_MoveBody;
    obj->collhandler = (COLL_HANDLER)COL_BodyCollHandler;
    obj->renderhandler = (RENDER_HANDLER)RenderChromeBall;


    // setup remote (PC) and replay handler (PC + N64) 
    //-------------------------------------------------
#ifndef _PSX
    obj->replayhandler = (REPLAY_HANDLER)DefaultObjectReplayStoreHandler;
#endif

#ifdef _PC
    obj->remotehandler = (REMOTE_HANDLER)SendObjectData;
    InitRemoteObjectData(obj);

// load default model

    obj->DefaultModel = LoadOneLevelModel(LEVEL_MODEL_CHROMEBALL, FALSE, obj->renderflag, 0);
    if (obj->DefaultModel == -1) return FALSE;
#endif

// remember owner player

    obj->player = (PLAYER*)flags[0];

// setup ball

    ball->Age = 0.0f;
    ball->Radius = CHROMEBALL_MIN_RAD;

// pos / vel

    //VecPlusScalarVec(&obj->player->car.Body->Centre.Pos, obj->player->car.Body->CollSkin.TightBBox.ZMin - CHROMEBALL_MAX_RAD, &obj->player->car.Body->Centre.WMatrix.mv[L], &obj->body.Centre.Pos);
    VecPlusScalarVec(&obj->player->car.Body->Centre.Pos, Real(-1.5) * obj->player->car.CollRadius - CHROMEBALL_MAX_RAD, &obj->player->car.Body->Centre.WMatrix.mv[L], &obj->body.Centre.Pos);
    VecPlusScalarVec(&obj->player->car.Body->Centre.Vel, Real(-512), &obj->player->car.Body->Centre.WMatrix.mv[L], &obj->body.Centre.Vel);

// Don't let ball drop through surfaces
    LineOfSightDist(&obj->player->car.Body->Centre.Pos, &obj->body.Centre.Pos, &time, &planePtr);
    if (time == ONE) {
        LineOfSightObj(&obj->player->car.Body->Centre.Pos, &obj->body.Centre.Pos, &time, obj->player->ownobj);
    }
    if (time != ONE) {
        CopyVec(&obj->player->car.Body->Centre.Pos, &obj->body.Centre.Pos);
        obj->body.Centre.Pos.v[Y] -= TO_LENGTH(Real(75));
    }


// setup sfx
#ifndef _PSX
 #ifdef OLD_AUDIO
    obj->Sfx3D = CreateSfx3D(SFX_CHROMEBALL, SFX_MIN_VOL, 22050, TRUE, &obj->body.Centre.Pos, 2);
    PlaySfx3D(SFX_CHROMEBALL_DROP, SFX_MAX_VOL, 22050, &obj->body.Centre.Pos, 2);
 #else // !OLD_AUDIO
//    g_SoundEngine.Play3DSound( EFFECT_Ball, TRUE, obj, &obj->pSfxInstance );
 #endif // !OLD_AUDIO
#endif

// Physical properties

    obj->body.Centre.Mass = Real(3.0f);
    obj->body.Centre.InvMass = ONE / Real(3.0f);
    SetMat(&obj->body.BodyInertia, Real(100), ZERO, ZERO, ZERO, Real(100), ZERO, ZERO, ZERO, Real(100));
    SetMat(&obj->body.BodyInvInertia, ONE / Real(100), ZERO, ZERO, ZERO, ONE / Real(100), ZERO, ZERO, ZERO, ONE / Real(100));
    GetFrameInertia(&obj->body.BodyInvInertia, &obj->body.Centre.WMatrix, &obj->body.WorldInvInertia);

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
#ifdef _N64
    obj->body.CollSkin.Sphere = &obj->Sphere;
#else
    obj->body.CollSkin.Sphere = (SPHERE *)malloc(sizeof(SPHERE));
#endif
    SetVecZero(&obj->body.CollSkin.Sphere[0].Pos);
    obj->body.CollSkin.Sphere[0].Radius = CHROMEBALL_MAX_RAD;
    obj->body.CollSkin.NSpheres = 1;
    CreateCopyCollSkin(&obj->body.CollSkin);
    MakeTightLocalBBox(&obj->body.CollSkin);
    InitWorldSkin(&obj->body.CollSkin, &obj->body.Centre.Pos, &obj->body.Centre.WMatrix);
    BuildWorldSkin(&obj->body.CollSkin, &obj->body.Centre.Pos, &obj->body.Centre.WMatrix);
    obj->body.CollSkin.Sphere[0].Radius = CHROMEBALL_MIN_RAD;
    obj->body.CollSkin.WorldSphere[0].Radius = CHROMEBALL_MIN_RAD;

// return OK
    return TRUE;
}


#ifdef _PC
/////////////////
// init weapon //
/////////////////

long InitClone(OBJECT *obj, long *flags)
{
    CLONE_OBJ *clone = (CLONE_OBJ*)obj->Data;

// No collision

    obj->CollType = COLL_TYPE_NONE;
    obj->body.CollSkin.AllowObjColls = FALSE;
    obj->body.CollSkin.AllowWorldColls = FALSE;
    
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

// No collision

    obj->CollType = COLL_TYPE_NONE;
    obj->body.CollSkin.AllowObjColls = FALSE;
    obj->body.CollSkin.AllowWorldColls = FALSE;
    
// return OK

    return TRUE;
}
#endif

/////////////////
// init weapon //
/////////////////
#ifndef _PSX
long InitTurbo2(OBJECT *obj, long *flags)
{
    TURBO2_OBJ *turbo;
    long i, k, ram, off;
    REAL mul, minz, maxz;
    MODEL *smodel, *dmodel;
    TURBO2_VERT *tvert;

// setup handlers

    obj->aihandler = (AI_HANDLER)Turbo2Handler;

    // Move handler does turbo's AI so that it happens before the car coll handlers - JCC
    obj->movehandler = (COLL_HANDLER)TurboAIHandler;


    obj->renderhandler = (RENDER_HANDLER)RenderTurbo2;  

// remember owner player

    obj->player = (PLAYER*)flags[0];

// if being electropulsed, cancel it and the turbo

    if (obj->player->car.PowerTimer > ZERO)
    {
#ifndef _PSX
 #ifdef OLD_AUDIO
        PlaySfx3D(SFX_SHOCKWAVE, SFX_MAX_VOL, SFX_SAMPLE_RATE, &obj->body.Centre.Pos, 2);
 #else // !OLD_AUDIO
        g_SoundEngine.Play3DSound( EFFECT_ShockFire, FALSE, obj );
 #endif // !OLD_AUDIO
#endif
        obj->player->car.PowerTimer = ZERO;
        return FALSE;
    }

// set render flags

    obj->renderflag.envmap = FALSE;
    obj->renderflag.light = FALSE;
    obj->renderflag.reflect = FALSE;
    obj->renderflag.meshfx = FALSE;

// calc + alloc ram

    smodel = &obj->player->car.Models->Body[0];

    ram = sizeof(TURBO2_OBJ);

#ifdef _PC
    ram += sizeof(MODEL_POLY) * smodel->PolyNum;
    ram += sizeof(POLY_RGB) * smodel->PolyNum;
    ram += sizeof(MODEL_VERTEX) * smodel->VertNum;
    ram += sizeof(TURBO2_VERT) * smodel->VertNum;
#endif

    obj->Data = malloc(ram);
    if (!obj->Data)
        return FALSE;

// setup turbo2


    turbo = (TURBO2_OBJ*)obj->Data;
    turbo->Age = 0.0f;
    turbo->LifeTime = 10.0f;

#ifdef _PC
// setup model

    dmodel = &turbo->Model;
    memcpy(dmodel, smodel, sizeof(MODEL));
    dmodel->PolyPtr = (MODEL_POLY*)(turbo + 1);
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

    minz = 99999.0f;
    maxz = -99999.0f;

    for (i = 0 ; i < dmodel->VertNum ; i++)
    {
        dmodel->VertPtr[i] = smodel->VertPtr[i];
        if (dmodel->VertPtr[i].z < minz) minz = dmodel->VertPtr[i].z;
        if (dmodel->VertPtr[i].z > maxz) maxz = dmodel->VertPtr[i].z;

        mul = 2.0f / Length((VEC*)&dmodel->VertPtr[i].x) + 1.0f;
        dmodel->VertPtr[i].x *= mul;
        dmodel->VertPtr[i].y *= mul;
        dmodel->VertPtr[i].z *= mul;
    }

// setup verts

    tvert = (TURBO2_VERT*)(dmodel->VertPtr + dmodel->VertNum);
    for (i = 0 ; i < dmodel->VertNum ; i++)
    {
        tvert[i].Time = frand(RAD);
        tvert[i].TimeAdd = frand(2.0f) + 1.0f;
        if (rand() & 1) tvert[i].TimeAdd = -tvert[i].TimeAdd;

//      mul = (dmodel->VertPtr[i].z - minz) / (maxz - minz);
//      tvert[i].Size = (mul * -16.0f + 24.0f) / 256.0f;
//      tvert[i].Offset = (mul * 56.0f + 104.0f) / 256.0f;
        tvert[i].Size = 10.0f / 256.0f;
        tvert[i].Offset = 96.0f / 256.0f;
    }

// lower down facing polys to prevent zbuffer artifacts

    for (i = 0 ; i < dmodel->PolyNum ; i++)
    {
        k = (dmodel->PolyPtr[i].Plane.v[Y] < -0.95f);
            
        dmodel->PolyPtr[i].v0->Clip = k;
        dmodel->PolyPtr[i].v1->Clip = k;
        dmodel->PolyPtr[i].v2->Clip = k;
        if (dmodel->PolyPtr[i].Type & POLY_QUAD)
            dmodel->PolyPtr[i].v3->Clip = k;
    }

    for (i = 0 ; i < dmodel->VertNum ; i++)
    {
        if (dmodel->VertPtr[i].Clip)
        {
            dmodel->VertPtr[i].y += 6.0f;
        }
    }

// setup light
#endif

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

// No collision

    obj->CollType = COLL_TYPE_NONE;
    obj->body.CollSkin.AllowObjColls = FALSE;
    obj->body.CollSkin.AllowWorldColls = FALSE;
    
    // setup sfx
#ifndef _PSX
 #ifdef OLD_AUDIO
    obj->Sfx3D = CreateSfx3D(SFX_TURBO, SFX_MAX_VOL, 22050, TRUE, &obj->body.Centre.Pos, 2);
 #else // !OLD_AUDIO
    g_SoundEngine.PlaySubmixedSound( EFFECT_Turbo, TRUE, obj->player->car.pSourceMix, &obj->pSfxInstance );
 #endif // !OLD_AUDIO
#endif

// return OK

    return TRUE;
}
#endif

/////////////////
// init weapon //
/////////////////
#ifdef _PC
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

// No collision

    obj->CollType = COLL_TYPE_NONE;
    obj->body.CollSkin.AllowObjColls = FALSE;
    obj->body.CollSkin.AllowWorldColls = FALSE;
    
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

// No collision

    obj->CollType = COLL_TYPE_NONE;
    obj->body.CollSkin.AllowObjColls = FALSE;
    obj->body.CollSkin.AllowWorldColls = FALSE;
    
// return OK

    return TRUE;
}
#endif

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
#ifndef _PSX
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
#endif
// maintain light

        if (obj->Light)
        {
            obj->Light->x = obj->body.Centre.Pos.v[X];
            obj->Light->y = obj->body.Centre.Pos.v[Y];
            obj->Light->z = obj->body.Centre.Pos.v[Z];
        }

// maintain sfx

#ifndef _PSX
 #ifdef OLD_AUDIO
        if (obj->Sfx3D)
        {
            CopyVec(&obj->body.Centre.Pos, &obj->Sfx3D->Pos);
        }
 #endif // !OLD_AUDIO
#endif

// stuck?

        if (shockwave->Age > 0.25f)
        {
            SubVector(&obj->body.Centre.Pos, &shockwave->OldPos, &vec);
            if ((Length(&vec) / TimeStep) < SHOCKWAVE_MIN_VEL)
                shockwave->Age = SHOCKWAVE_MAX_AGE;
        }

        CopyVec(&obj->body.Centre.Pos, &shockwave->OldPos);

// kill?

        if (shockwave->Age >= SHOCKWAVE_MAX_AGE)
        {
            shockwave->Alive = FALSE;
#ifndef _PSX
 #ifdef OLD_AUDIO
            if (obj->Sfx3D)
                FreeSfx3D(obj->Sfx3D);
 #else // !OLD_AUDIO
            if( obj->pSfxInstance )
            {
                g_SoundEngine.ReturnInstance( obj->pSfxInstance );
                obj->pSfxInstance = NULL;
            }
 #endif // !OLD_AUDIO
#endif
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

#ifdef _PC
    AddWorldMeshFx(ShockwaveWorldMeshFxChecker, obj);
    AddModelMeshFx(ShockwaveModelMeshFxChecker, obj);
#endif
}

////////////////////
// weapon handler //
////////////////////

void FireworkHandler(OBJECT *obj)
{
    REAL dRLenSq;
    VEC dR;
    BBOX bBox;
    PLAYER *player;
    FIREWORK_OBJ *firework = (FIREWORK_OBJ *)obj->Data;

    // maintain sfx
#ifndef _PSX
 #ifdef OLD_AUDIO
    if (obj->Sfx3D)
    {
        CopyVec(&obj->body.Centre.Pos, &obj->Sfx3D->Pos);
    }
 #endif // !OLD_AUDIO
#endif
    // check age of firework
    firework->Age += TimeStep;

    // Move or set up explosion
    if (firework->Age < FIREWORK_MAX_AGE) {
        // Move firework
        FireWorkMove(obj);

        // See if we're near any players
        SetBBox(&bBox, 
            obj->body.Centre.Pos.v[X] - FIREWORK_RADIUS, 
            obj->body.Centre.Pos.v[X] + FIREWORK_RADIUS, 
            obj->body.Centre.Pos.v[Y] - FIREWORK_RADIUS, 
            obj->body.Centre.Pos.v[Y] + FIREWORK_RADIUS, 
            obj->body.Centre.Pos.v[Z] - FIREWORK_RADIUS, 
            obj->body.Centre.Pos.v[Z] + FIREWORK_RADIUS);
        for (player = PLR_PlayerHead; player != NULL; player = player->next) {
            if (player->type == PLAYER_GHOST) continue;
            if (player->type == PLAYER_NONE) continue;
            if (player == obj->player) continue;

            // Quick bounding box test
            if (!BBTestXZY(&bBox, &player->car.Body->CollSkin.BBox)) continue;

            // Sphere test
            VecMinusVec(&player->car.Body->Centre.Pos, &obj->body.Centre.Pos, &dR);
            if ((dRLenSq = VecDotVec(&dR, &dR)) < FIREWORK_RADIUS_SQ) {
                firework->Age = FIREWORK_MAX_AGE;
                break;
            }
        }

    } else {
        if (!firework->Exploded) {
            // Set up explosions
            FireworkExplode(obj);
            obj->CollType = COLL_TYPE_NONE;
            obj->body.CollSkin.AllowObjColls = FALSE;
            obj->body.CollSkin.AllowWorldColls = FALSE;
            obj->collhandler = NULL;
        } else {
            // update the lightsource
            if ((obj->Light != NULL) && (firework->Age < FIREWORK_MAX_AGE + 0.8)) {
                obj->Light->r = 128 + (long)(frand(32)) - (long)((128 * (firework->Age - FIREWORK_MAX_AGE)) / 0.8);
                obj->Light->g = 32 - (long)((32 * (firework->Age - FIREWORK_MAX_AGE)) / 0.8);
                obj->Light->b = 64 - (long)((64 * (firework->Age - FIREWORK_MAX_AGE)) / 0.8);
            } else {
                // Kill the firework
#if FALSE
                DetachWeaponCamera(obj);
#endif
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
        FireWorkMoveAndHome(obj);
        return;
        //VecMinusVec(&firework->Target->body.Centre.Pos, &obj->body.Centre.Pos, &dR);
        //dRLen = VecLen(&dR);
        //if (dRLen > SMALL_REAL) {
        //  VecDivScalar(&dR, dRLen / 2.5f);
        //} else {
        //  firework->Age = FIREWORK_MAX_AGE;
        //  return;
        //}
    } else {
        CopyVec(&DownVec, &dR);
        dRLen = TARGET_RANGE_MAX;
    }

    // Accelerate firework towards target, or forwards if no target
    impMod = ONE;//0.2f + dRLen / WEAPON_RANGE_MAX;
    VecEqScalarVec(&imp, -0.5f * impMod * FLD_Gravity * TimeStep, &obj->body.Centre.WMatrix.mv[U]);
    ApplyParticleImpulse(&obj->body.Centre, &imp);

    // Make firework dip downwards as it travels
    if (firework->Target == NULL) {
        VecEqScalarVec(&imp, -0.005f * FLD_Gravity * TimeStep, &dR);
    } else {
        VecEqScalarVec(&imp, -35 * TimeStep, &dR);
    }
    VecEqScalarVec(&offset, 50, &obj->body.Centre.WMatrix.mv[U]);
    ApplyBodyImpulse(&obj->body, &imp, &offset);

    // Move the particle
    UpdateBody(&obj->body, TimeStep);

    // Put it in the grid system
    UpdateObjectGrid(obj);

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

void FireWorkMoveAndHome(OBJECT *obj)
{
    REAL dRLen, scale;
    VEC dR, axis, imp;
    FIREWORK_OBJ *firework = (FIREWORK_OBJ *)obj->Data;

    // Get the target relative position
    VecMinusVec(&firework->Target->body.Centre.Pos, &obj->body.Centre.Pos, &dR);
    dRLen = VecLen(&dR);
    if (dRLen > SMALL_REAL) {
        VecDivScalar(&dR, dRLen);
    } else {
        firework->Age = FIREWORK_MAX_AGE;
        return;
    }

    // Accelerate firework forwards
    scale = dRLen / TARGET_RANGE_MAX;
    VecEqScalarVec(&imp, -HALF * scale * FLD_Gravity * TimeStep, &obj->body.Centre.WMatrix.mv[U]);
    ApplyParticleImpulse(&obj->body.Centre, &imp);

    // Accelerate firework toward target
    VecEqScalarVec(&imp, HALF * FLD_Gravity * TimeStep, &dR);
    ApplyParticleImpulse(&obj->body.Centre, &imp);

    // Rotate firework towards target
    VecCrossVec(&dR, &obj->body.Centre.WMatrix.mv[U], &axis);
    VecMulScalar(&axis, 50);
    //axisLen = VecLen(&axis);
    ApplyBodyAngImpulse(&obj->body, &axis);

    // Move the body
    UpdateBody(&obj->body, TimeStep);

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
    REAL dRLenSq, dRLen;
    VEC vel, dR, pos;
    BBOX bBox;
    PLAYER *player;
    FIREWORK_OBJ *firework = (FIREWORK_OBJ *)obj->Data;

    // Free up the models and sound effects
#ifndef _PSX
 #ifdef OLD_AUDIO
    if (obj->Sfx3D)
    {
        FreeSfx3D(obj->Sfx3D);
    }
 #else // !OLD_AUDIO
    if( obj->pSfxInstance )
    {
        g_SoundEngine.ReturnInstance( obj->pSfxInstance );
        obj->pSfxInstance = NULL;
    }
 #endif // !OLD_AUDIO
#endif
    obj->DefaultModel = -1;
    obj->renderhandler = NULL;

    // Get rid of the trail
    if (firework->Trail != NULL) {
        FreeTrail(firework->Trail);
    }

#ifndef _PSX
 #ifdef OLD_AUDIO
    // Oooaaahhhhh
    PlaySfx3D(SFX_FIREWORK_BANG, SFX_MAX_VOL, 22050, &obj->body.Centre.Pos, 2);
 #else // !OLD_AUDIO
    g_SoundEngine.Play3DSound( EFFECT_FireworkBang, FALSE, obj );
 #endif // !OLD_AUDIO
#endif
    // Create Explosion
    CreateSpark(SPARK_EXPLOSION1, &obj->body.Centre.Pos, &ZeroVector, 0, 0);

    // Create smoke
    //VecEqScalarVec(&vel, 60, &UpVec);
    SetVec(&vel, 0, -60, 0);
    for (iFlash = 0; iFlash < 3; iFlash++) {
        CreateSpark(SPARK_SMOKE2, &obj->body.Centre.Pos, &vel, 60, 0);
    }

    //Create flashy bits
    //VecPlusScalarVec(&obj->body.Centre.Vel, 300, &UpVec, &vel);
    //VecEqScalarVec(&vel, 1200, &UpVec);
    SetVec(&vel, 0, -1200, 0);
    for (iFlash = 0; iFlash < 30; iFlash++) {
        CreateSpark(SPARK_SMALLORANGE, &obj->body.Centre.Pos, &vel, 1200, 0);
        CreateSpark(SPARK_SMALLRED, &obj->body.Centre.Pos, &vel, 1400, 0);
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

    // blow nearby cars away
    if (GameSettings.GameType != GAMETYPE_FRONTEND) {
        SetBBox(&bBox, 
            obj->body.Centre.Pos.v[X] - FIREWORK_EXPLODE_RADIUS, 
            obj->body.Centre.Pos.v[X] + FIREWORK_EXPLODE_RADIUS, 
            obj->body.Centre.Pos.v[Y] - FIREWORK_EXPLODE_RADIUS, 
            obj->body.Centre.Pos.v[Y] + FIREWORK_EXPLODE_RADIUS, 
            obj->body.Centre.Pos.v[Z] - FIREWORK_EXPLODE_RADIUS, 
            obj->body.Centre.Pos.v[Z] + FIREWORK_EXPLODE_RADIUS);
        for (player = PLR_PlayerHead; player != NULL; player = player->next) 
        {
            if (player->type == PLAYER_GHOST) continue;

            // Quick bounding box test
            if (!BBTestXZY(&bBox, &player->car.Body->CollSkin.BBox)) continue;

            // Sphere test
            VecMinusVec(&player->car.Body->Centre.Pos, &obj->body.Centre.Pos, &dR);
            if ((dRLenSq = VecDotVec(&dR, &dR)) > FIREWORK_EXPLODE_RADIUS_SQ) continue;
            if (dRLenSq < FIREWORK_EXPLODE_MIN_RADIUS_SQ) dRLenSq = FIREWORK_EXPLODE_MIN_RADIUS_SQ;
            dRLen = (REAL)sqrt(dRLenSq);

            // Calculate the explosion impulse
            VecEqScalarVec(&vel, FIREWORK_EXPLODE_IMPULSE / dRLenSq, &dR);
            vel.v[Y] = -FIREWORK_EXPLODE_IMPULSE2 / (4 * dRLen);

            SetVec(&pos, 
                frand(2 * FIREWORK_EXPLODE_OFFSET) - FIREWORK_EXPLODE_OFFSET, 
                frand(2 * FIREWORK_EXPLODE_OFFSET) - FIREWORK_EXPLODE_OFFSET, 
                frand(2 * FIREWORK_EXPLODE_OFFSET) - FIREWORK_EXPLODE_OFFSET);

            ApplyBodyImpulse(player->car.Body, &vel, &pos);

            // Reset last hit timer
            player->car.LastHitTimer = ZERO;

            player->car.Body->Stacked = FALSE;
            player->car.Body->NoContactTime = ZERO;
            player->car.Body->NoMoveTime = ZERO;
        }
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
#ifdef _PC
    CUBE_HEADER *cube;
    WORLD_POLY *wp;
    WORLD_VERTEX **wv;
    long *wrgb;
    MODEL_RGB *mrgb;
#endif
    NEWCOLLPOLY *p;
    COLLGRID *header;


// maintain sfx

#ifndef _PSX
 #ifdef OLD_AUDIO
    if (obj->Sfx3D)
    {
        CopyVec(&obj->player->car.Body->Centre.Pos, &obj->Sfx3D->Pos);
    }
 #endif // !OLD_AUDIO
#endif

// darken car

    obj->player->car.AddLit -= (long)(TimeStep * 4000);
    if (obj->player->car.AddLit < -1000) obj->player->car.AddLit = -1000;

// dec countdown, bang?

    if (!obj->player->car.RepositionTimer)
    {
        bomb->Timer -= TimeStep;
    }

    if (obj->player->car.WillDetonate == FALSE){

        
        if (bomb->Timer < ZERO) 
        {

            // Set bomb to explode
            obj->player->car.WillDetonate = TRUE;
            bomb->Timer = PUTTYBOMB_COUNTDOWN2;
#ifndef _PSX
 #ifdef OLD_AUDIO
            if (obj->Sfx3D)
            {
                FreeSfx3D(obj->Sfx3D);
                obj->Sfx3D = NULL;
            }
 #else // !OLD_AUDIO
            if( obj->pSfxInstance )
            {
                g_SoundEngine.ReturnInstance( obj->pSfxInstance );
                obj->pSfxInstance = NULL;
            }
 #endif // !OLD_AUDIO
            flag = (long)obj->player;
            CreateObject(&obj->player->car.Body->Centre.Pos, &obj->player->car.Body->Centre.WMatrix, OBJECT_TYPE_BOMBGLOW, &flag);
#endif
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

#ifdef _PC
        SubVector(&bomb->Pos, &CAM_MainCamera->WPos, &vec);
        CAM_MainCamera->Shake = 1.0f - (Length(&vec) / 2048.0f);
        if (CAM_MainCamera->Shake < 0.0f) CAM_MainCamera->Shake = 0.0f;
#else
        if (CAM_PlayerCameras[obj->player->Slot])                       // Only shake human player camera
        {
            SubVector(&bomb->Pos, &CAM_PlayerCameras[obj->player->Slot]->WPos, &vec);
            CAM_PlayerCameras[obj->player->Slot]->Shake = 1.0f - (Length(&vec) / 2048.0f);
            if (CAM_PlayerCameras[obj->player->Slot]->Shake < 0.0f) CAM_PlayerCameras[obj->player->Slot]->Shake = 0.0f;
        }
#endif

        obj->aihandler = (AI_HANDLER)PuttyBombBang;
        obj->renderhandler = (RENDER_HANDLER)RenderPuttyBombBang;

// play bang sfx
#ifndef _PSX
 #ifdef OLD_AUDIO
        PlaySfx3D(SFX_PUTTYBOMB_BANG, SFX_MAX_VOL, 22050, &bomb->Pos, 2);
 #else // !OLD_AUDIO
        g_SoundEngine.Play3DSound( EFFECT_PuttyBombBang, 
                                   FALSE, 
                                   bomb->Pos.v[0],
                                   bomb->Pos.v[1],
                                   bomb->Pos.v[2] );
 #endif // !OLD_AUDIO
#endif
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
#ifdef _PC
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
#endif

// calc bang impulse

        SetVector(&imp, 0.0f, 0.0f, 0.0f);
        pos = &obj->player->car.Body->Centre.Pos;
        mass = obj->player->car.Body->Centre.Mass * 0.2f + 0.4f;
        flag = FALSE;

        header = PosToCollGrid(pos);
        if (header)
        {
            for (i = 0 ; i < header->NCollPolys ; i++)
            {
                p = GetCollPoly(header->CollPolyIndices[i]);

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
                        mul = (PUTTYBOMB_BANG_IMPULSE_RANGE - dist) * 10.0f * mass;
                        VecPlusEqScalarVec(&imp, mul, (VEC*)&p->Plane);
                        flag = TRUE;
                    }
                }
            }
        }

        rot = frand(RAD);
        mul = frand(90.0f);
        SetVector(&off, (float)sin(rot) * mul, 0.0f, (float)cos(rot) * mul);

        if (flag)
        {
            ApplyBodyImpulse(obj->player->car.Body, &imp, &off);
        }
        else
        {
            SetVector(&imp, frand(100000.0f) - 50000.0f, frand(100000.0f) - 50000.0f, frand(100000.0f) - 50000.0f);
            ApplyBodyAngImpulse(obj->player->car.Body, &imp);
        }

// set player moving just in case

        obj->player->car.Body->Stacked = FALSE;
        obj->player->car.Body->NoContactTime = ZERO;
        obj->player->car.Body->NoMoveTime = ZERO;

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
    VEC vec;
    long i;
#ifdef _PC
    MODEL_VERTEX *mv;
#endif

// darken car

    if (bomb->Timer < PUTTYBOMB_SPHERE_TIME)
    {
        obj->player->car.AddLit -= (long)(TimeStep * 4000);
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
#ifdef _PC
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
#endif

#ifdef _PC
// set vert UV's
    vert = (PUTTYBOMB_VERT*)(bomb + 1);
    mv = model->VertPtr;

    for (i = model->VertNum ; i ; i--, mv++, vert++)
    {
        vert->Time += vert->TimeAdd * TimeStep;

        mv->tu = (float)sin(vert->Time) * (12.0f / 256.0f) + (36.0f / 256.0f);
        mv->tv = (float)cos(vert->Time) * (12.0f / 256.0f) + (68.0f / 256.0f);
    }
#endif

// set sphere size

    if (bomb->SphereRadius < 512.0f)
    bomb->SphereRadius += (512.0f - bomb->SphereRadius) * Real(5.04f) * TimeStep;

// inc timer, kill?

    bomb->Timer += TimeStep;
    if (bomb->Timer >= PUTTYBOMB_BANG_TIME)
    {
        obj->player->car.IsBomb = FALSE;
        OBJ_FreeObject(obj);
        return;
    }

// set bounding box + add to mesh fx lists
#ifdef _PC
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
#endif
}



void PuttyBombMove(OBJECT *obj) 
{
    PLAYER *player;
    PUTTYBOMB_OBJ *bomb = (PUTTYBOMB_OBJ*)obj->Data;

    // don't do if multiplayer and local player giving bomb
    if (IsMultiPlayer() && obj->player->type == PLAYER_LOCAL) return;

    // Make sure it isn't too late
    if (obj->player->car.WillDetonate) return;

//$ADDITION
    //$TODO(JHarding): Confirm this is OK to do.  
    // Keep the our "object" in the same position as the car
    // for sound effect purposes.
    CopyVec( &obj->player->car.Body->Centre.Pos,
             &obj->body.Centre.Pos );
//$END_ADDITION

    // Loop over all other players
    for (player = PLR_PlayerHead; player != NULL; player = player->next) {
        if (player == obj->player) continue;

        // skip if multiplayer and not local player receiving bomb
        if (IsMultiPlayer() && player->type != PLAYER_LOCAL) continue;

        // Have the players collided?
        if (!HaveCarsCollided(obj->player->ownobj, player->ownobj)) continue;

        // If other player is already the bomb, no transferring
        if (player->car.IsBomb) continue;

        // Can the bomb be tranferred?
        if (player->car.NoReturnTimer > ZERO) continue;

#ifdef _PC
        // send bomb transfer packet to everyone if multiplayer
        if (IsMultiPlayer())
        {
            SendTransferBomb(obj->GlobalID, obj->player->PlayerID, player->PlayerID);
        }
#endif

        // transfer bomb
        TransferBomb(obj, obj->player, player);
    }
}

////////////////////////
// transfer bomb from //
////////////////////////

void TransferBomb(OBJECT *obj, PLAYER *player1, PLAYER *player2)
{
    PUTTYBOMB_OBJ *bomb = (PUTTYBOMB_OBJ*)obj->Data;

    player1->car.IsBomb = FALSE;
    player1->car.WillDetonate = FALSE;
    player1->car.Aerial.Length = bomb->OrigAerialLen;
    player1->car.NoReturnTimer = PUTTYBOMB_NORETURN_TIME;

    player2->car.IsBomb = TRUE;
    player2->car.WillDetonate = FALSE;
    bomb->OrigAerialLen = player2->car.Aerial.Length;

    obj->player = player2;
}

////////////////////
// weapon handler //
////////////////////

void WaterBombHandler(OBJECT *obj)
{
    long flag;
    WATERBOMB_OBJ *bomb = (WATERBOMB_OBJ*)obj->Data;
    MAT mat;
    VEC pos, dR, imp;
    REAL dist, dRLenSq, dRLen;
    BBOX bBox;
    PLAYER *player;

// inc age

    bomb->Age += TimeStep;

// maintain sfx
#ifndef _PSX
 #ifdef OLD_AUDIO
    if (obj->Sfx3D)
    {
        CopyVec(&obj->body.Centre.Pos, &obj->Sfx3D->Pos);
    }
 #endif // !OLD_AUDIO
#endif
// set wobble scalars

    bomb->ScalarHoriz = (float)sin((float)TIME2MS(TimerCurrent) / 100.0f) / 10.0f + 1.0f;
    bomb->ScalarVert = 2.0f - bomb->ScalarHoriz;

// sub hit mag

    if (obj->body.BangMag)
    {
        if (obj->body.NOtherContacts > ZERO)
        {
            bomb->BangTol = -ONE;
        }
        else
        {
            bomb->BangTol -= obj->body.BangMag;
            if (obj->body.BangPlane.v[Y] > -0.7f && bomb->BangTol < 0.0f)
                bomb->BangTol = 0.0f;

#ifndef _PSX
            if (obj->body.BangMag > 100.0f)
            {
 #ifdef OLD_AUDIO
                long vol = (long)obj->body.BangMag / 10;
                if (vol > SFX_MAX_VOL) vol = SFX_MAX_VOL;
                PlaySfx3D(SFX_WATERBOMB_BOUNCE, vol, 22050, &obj->body.Centre.Pos, 2);
 #else // !OLD_AUDIO
                g_SoundEngine.Play3DSound( EFFECT_WaterBombBounce, 
                                           FALSE, 
                                           obj->body.Centre.Pos.v[0],
                                           obj->body.Centre.Pos.v[1],
                                           obj->body.Centre.Pos.v[2] );
 #endif // !OLD_AUDIO
            }
#endif
        }

        obj->body.BangMag = 0.0f;
    }

// bang?
    if (bomb->Age > WATERBOMB_MAX_AGE) {
        SetPlane(&obj->body.BangPlane, ZERO, -ONE, ZERO, obj->body.Centre.Pos.v[Y]);
        bomb->BangTol = -ONE;

    }

    if (bomb->BangTol < 0.0f)// || bomb->Age > WATERBOMB_MAX_AGE)
    {
#ifndef _PSX
 #ifdef OLD_AUDIO
        PlaySfx3D(SFX_WATERBOMB_HIT, SFX_MAX_VOL, 22050, &obj->body.Centre.Pos, 2);
 #else // !OLD_AUDIO
        g_SoundEngine.Play3DSound( EFFECT_WaterBombHit, 
                                   FALSE, 
                                   obj->body.Centre.Pos.v[0],
                                   obj->body.Centre.Pos.v[1],
                                   obj->body.Centre.Pos.v[2] );
 #endif // !OLD_AUDIO
#endif
//      flag = (long)&LevelModel[obj->DefaultModel].Model;
//      CreateObject(&obj->body.Centre.Pos, &obj->body.Centre.WMatrix, OBJECT_TYPE_DISSOLVEMODEL, &flag);

        dist = -PlaneDist(&obj->body.BangPlane, &obj->body.Centre.Pos);
        VecPlusScalarVec(&obj->body.Centre.Pos, dist, (VEC*)&obj->body.BangPlane, &pos);

        SetVec(&mat.mv[U], -obj->body.BangPlane.v[A], -obj->body.BangPlane.v[B], -obj->body.BangPlane.v[C]);
        SetVec(&mat.mv[L], mat.m[UY], mat.m[UZ], mat.m[UX]);
        CrossProduct(&mat.mv[U], &mat.mv[L], &mat.mv[R]);
        CrossProduct(&mat.mv[R], &mat.mv[U], &mat.mv[L]);

        CreateObject(&pos, &mat, OBJECT_TYPE_SPLASH, &flag);

// blow away nearby cars
        SetBBox(&bBox, 
            obj->body.Centre.Pos.v[X] - WATERBOMB_EXPLODE_RADIUS, 
            obj->body.Centre.Pos.v[X] + WATERBOMB_EXPLODE_RADIUS, 
            obj->body.Centre.Pos.v[Y] - WATERBOMB_EXPLODE_RADIUS, 
            obj->body.Centre.Pos.v[Y] + WATERBOMB_EXPLODE_RADIUS, 
            obj->body.Centre.Pos.v[Z] - WATERBOMB_EXPLODE_RADIUS, 
            obj->body.Centre.Pos.v[Z] + WATERBOMB_EXPLODE_RADIUS);
        for (player = PLR_PlayerHead; player != NULL; player = player->next) 
        {
            if (player->type == PLAYER_GHOST || player->type == PLAYER_NONE) continue;

            // Quick bounding box test
            if (!BBTestXZY(&bBox, &player->car.Body->CollSkin.BBox)) continue;

            // Sphere test
            VecMinusVec(&player->car.Body->Centre.Pos, &obj->body.Centre.Pos, &dR);
            if ((dRLenSq = VecDotVec(&dR, &dR)) > WATERBOMB_EXPLODE_RADIUS_SQ) continue;
            if (dRLenSq < WATERBOMB_EXPLODE_MIN_RADIUS_SQ) dRLenSq = WATERBOMB_EXPLODE_MIN_RADIUS_SQ;
            dRLen = (REAL)sqrt(dRLenSq);

            // Calculate the explosion impulse
            VecEqScalarVec(&imp, WATERBOMB_EXPLODE_IMPULSE / dRLenSq, &dR);
            imp.v[Y] = -WATERBOMB_EXPLODE_IMPULSE2 / dRLenSq;

            SetVec(&pos, 
                frand(WATERBOMB_EXPLODE_OFFSET) - WATERBOMB_EXPLODE_OFFSET / 2, 
                frand(WATERBOMB_EXPLODE_OFFSET) - WATERBOMB_EXPLODE_OFFSET / 2, 
                frand(WATERBOMB_EXPLODE_OFFSET) - WATERBOMB_EXPLODE_OFFSET / 2);

            ApplyBodyImpulse(player->car.Body, &imp, &pos);

            // Reset last hit timer
            player->car.LastHitTimer = ZERO;

            player->car.Body->Stacked = FALSE;
            player->car.Body->NoContactTime = ZERO;
            player->car.Body->NoMoveTime = ZERO;
        }

// Kill the water bomb
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
    PLAYER *player;
    VEC delta, vec, *nvec1, *nvec2, *pos, rotpos, v1, v2;
    REAL dist, ndist;

#ifdef _PC
    MODEL *dmodel, *model = (MODEL*)&electro->Model;
    MODEL_POLY *mp;
    POLY_RGB *mrgb;
    MODEL_VERTEX *mv;
    ELECTROPULSE_VERT *evert;

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
#elif defined(_N64)
    MODEL *dmodel , *model = obj->player->car.Models->Body;
    VEC vec1,vec2;
    Vtx *mv;
#endif

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

#ifdef _PC
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
#endif

// maintain light

    if (obj->Light)
    {
        CopyVec(&obj->player->car.Body->Centre.Pos, (VEC*)&obj->Light->x);

        obj->Light->b = lmul / 4 + (rand() & 31);
    }

// maintain sfx

#ifndef _PSX
 #ifdef OLD_AUDIO
    if (obj->Sfx3D)
    {
        CopyVec(&obj->player->car.Body->Centre.Pos, &obj->Sfx3D->Pos);

        obj->Sfx3D->Vol = SFX_MAX_VOL * lmul / 255;
    }
 #else // !OLD_AUDIO
    if( obj->pSfxInstance )
    {
        obj->pSfxInstance->SetVolume( DSBVOLUME_MIN + lmul * ( DSBVOLUME_MAX - DSBVOLUME_MIN ) / 255 );
    }
 #endif // !OLD_AUDIO
#endif

// kill?

    electro->Age += TimeStep;
    if (electro->Age > 10.0f || obj->player->type == PLAYER_NONE)
    {
#ifndef _PSX
        #ifdef _N64
        // Clear glow effect here
        obj->player->car.Models->Body->Flag &= ~MODEL_ENV_ELECTRO;
        #endif

 #ifdef OLD_AUDIO
        if (electro->ZapSfx)
            FreeSfx3D(electro->ZapSfx);
 #else // !OLD_AUDIO
        if( electro->pZapSound )
        {
            g_SoundEngine.ReturnInstance( electro->pZapSound );
            electro->pZapSound = NULL;
        }
 #endif // !OLD_AUDIO
#endif

        OBJ_FreeObject(obj);
        return;
    }


// N64 Env Map

#ifdef _N64
    obj->player->car.Models->Body->Flag |= MODEL_ENV_ELECTRO;
#endif

// check against other cars

    electro->JumpFlag = 0;

    pos = &obj->player->car.BodyWorldPos;

    for (player = PLR_PlayerHead ; player ; player = player->next)
    {

// Don't affect self
        
        if (player == obj->player)
            continue;

// Check separation

        SubVector(pos, &player->car.BodyWorldPos, &delta);
        dist = delta.v[X] * delta.v[X] + delta.v[Y] * delta.v[Y] + delta.v[Z] * delta.v[Z];
        if (dist > (ELECTRO_RANGE * ELECTRO_RANGE))
            continue;

// line of sight

        if (!LineOfSight(&obj->player->car.Body->Centre.Pos, &player->car.Body->Centre.Pos))
            continue;

// car in range

        if (player->car.PowerTimer == -ELECTROPULSE_NO_RETURN_TIME)
        {
            flag = (long)player;
            CreateObject(&player->car.Body->Centre.Pos, &player->car.Body->Centre.WMatrix, OBJECT_TYPE_ELECTROZAPPED, &flag);

            // Reset last hit timer
            player->car.LastHitTimer = ZERO;

        }
    
// Don't increase electropulse timer for cars already affected - horsepiss - do - cockpiss - don't; in fact give a bit of time afterwards in which you can't be affected!
        
        if (player->car.PowerTimer == -ELECTROPULSE_NO_RETURN_TIME) 
        {
            player->car.PowerTimer = ELECTRO_KILL_TIME;
        }

// find his nearest vert to me

        SubVector(pos, &player->car.BodyWorldPos, &delta);
        TransposeRotVector(&player->car.Body->Centre.WMatrix, &delta, &rotpos);

        dmodel = &player->car.Models->Body[0];

        ndist = 1000000;
#ifdef _PC
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
#elif defined _N64
        //nvec1 = &player->car.BodyWorldPos;
        mv = dmodel->hdr->vtxptr;
        for (i = dmodel->hdr->vtxnum; i ; i--, mv++)
        {
            vec.v[X] = mv->v.ob[X] - rotpos.v[X];
            vec.v[Y] = mv->v.ob[Y] - rotpos.v[Y];
            vec.v[Z] = mv->v.ob[Z] - rotpos.v[Z];
            dist = vec.v[X] * vec.v[X] + vec.v[Y] * vec.v[Y] + vec.v[Z] * vec.v[Z];
            if (dist < ndist)
            {
                ndist = dist;
                nvec1 = (VEC*)&mv->v.ob[0];
            }
        }
        vec1.v[X] = ((Vtx*)nvec1)->v.ob[0];
        vec1.v[Y] = ((Vtx*)nvec1)->v.ob[1];
        vec1.v[Z] = ((Vtx*)nvec1)->v.ob[2];
        nvec1 = &vec1;
#endif

// find my nearest vert to him

        SubVector(&player->car.BodyWorldPos, pos, &delta);
        TransposeRotVector(&obj->player->car.Body->Centre.WMatrix, &delta, &rotpos);

        ndist = 1000000;

#ifdef _PC
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
#elif defined _N64
        //nvec2 = &obj->player->car.BodyWorldPos;
        mv = model->hdr->vtxptr;
        for (i = model->hdr->vtxnum; i ; i--, mv++)
        {
            vec.v[X] = mv->v.ob[X] - rotpos.v[X];
            vec.v[Y] = mv->v.ob[Y] - rotpos.v[Y];
            vec.v[Z] = mv->v.ob[Z] - rotpos.v[Z];
            dist = vec.v[X] * vec.v[X] + vec.v[Y] * vec.v[Y] + vec.v[Z] * vec.v[Z];
            if (dist < ndist)
            {
                ndist = dist;
                nvec2 = (VEC*)&mv->v.ob[0];
            }
        }
        vec2.v[X] = ((Vtx*)nvec2)->v.ob[0];
        vec2.v[Y] = ((Vtx*)nvec2)->v.ob[1];
        vec2.v[Z] = ((Vtx*)nvec2)->v.ob[2];
        nvec2 = &vec2;
#endif

// save player + verts

        electro->Player[electro->JumpFlag] = player;

        {
        VEC vEmpty;
        vEmpty.v[0] = vEmpty.v[1] = vEmpty.v[2] = 0.f;
        CopyVec(&vEmpty, &electro->JumpPos1[electro->JumpFlag]);
        CopyVec(&vEmpty, &electro->JumpPos2[electro->JumpFlag]);
        }


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

// update zap sfx
#ifndef _PSX
    if (electro->JumpFlag)
    {
 #ifdef OLD_AUDIO
        if (!electro->ZapSfx)
            electro->ZapSfx = CreateSfx3D(SFX_ELECTROZAP, SFX_MAX_VOL, 22050, TRUE, &obj->player->car.Body->Centre.Pos, 2);
        else
            CopyVec(&obj->player->car.Body->Centre.Pos, &electro->ZapSfx->Pos);
 #else // !OLD_AUDIO
        if( !electro->pZapSound )
            g_SoundEngine.Play3DSound( EFFECT_ElectroZap, TRUE, obj, &electro->pZapSound );
 #endif // !OLD_AUDIO
    }
    else
    {
 #ifdef OLD_AUDIO
        if (electro->ZapSfx)
        {
            FreeSfx3D(electro->ZapSfx);
            electro->ZapSfx = NULL;
        }
 #else // !OLD_AUDIO
        if( electro->pZapSound )
        {
            g_SoundEngine.ReturnInstance( electro->pZapSound );
            electro->pZapSound = NULL;
        }
 #endif // !OLD_AUDIO
    }
#endif
}

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
            oil->Ymin = oil->LandHeight;
            oil->Ymax = oil->LandHeight;

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

        if ((OilSlickCount < OILSLICK_LIST_MAX) && (oil->Age < 28.75f))
        {
            OilSlickList[OilSlickCount].X = obj->body.Centre.Pos.v[X];
            OilSlickList[OilSlickCount].Z = obj->body.Centre.Pos.v[Z];
            OilSlickList[OilSlickCount].Radius = oil->Size;
            OilSlickList[OilSlickCount].SquaredRadius = oil->Size * oil->Size;

            OilSlickList[OilSlickCount].Ymin = oil->Ymin - COLL_EPSILON;
            OilSlickList[OilSlickCount].Ymax = oil->Ymax + COLL_EPSILON;

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
        SetVector(&delta, 0.0f, 0.0f, 0.0f);
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

#ifndef _PSX
 #ifdef OLD_AUDIO
    if (obj->Sfx3D)
    {
        FTOL(Length(&obj->body.Centre.Vel), vel);

        obj->Sfx3D->Freq = 5000 + vel * 4;

        obj->Sfx3D->Vol = vel / 2;
        if (obj->Sfx3D->Vol > SFX_MAX_VOL) obj->Sfx3D->Vol = SFX_MAX_VOL;

        CopyVec(&obj->body.Centre.Pos, &obj->Sfx3D->Pos);
    }
 #else // !OLD_AUDIO
    if( obj->pSfxInstance )
    {
        FTOL(Length(&obj->body.Centre.Vel), vel);
        LONG lPitch = vel * 2;
        if( lPitch > DSBPITCH_MAX )
            lPitch = DSBPITCH_MAX;
        else if( lPitch < DSBPITCH_MIN )
            lPitch = DSBPITCH_MIN;

        obj->pSfxInstance->SetPitch( vel * 2 );
        obj->pSfxInstance->SetVolume( -vel );
    }
 #endif // !OLD_AUDIO
#endif //!MT!

    // Reset knocks
    if (obj->body.BangMag > TO_VEL(Real(500))) {
        long vol, freq;

#ifndef _PSX 
//      vol = SFX_MAX_VOL;
        vol = (long)obj->body.BangMag / 10;
        if (vol > SFX_MAX_VOL) vol = SFX_MAX_VOL;

        freq = 22050;

 #ifdef OLD_AUDIO
        PlaySfx3D(SFX_CHROMEBALL_DROP, vol, freq, &obj->body.Centre.Pos, 2);
 #else // !OLD_AUDIO
        g_SoundEngine.Play3DSound( EFFECT_BallDrop, FALSE, obj );
 #endif // !OLD_AUDIO
#endif
        obj->body.Banged = FALSE;
        obj->body.BangMag = ZERO;
    }

// kill?

    if (ball->Age >= 30.0f)
    {
        OBJ_FreeObject(obj);
    }
}

#ifdef _PC
////////////////////
// weapon handler //
////////////////////
void CloneHandler(OBJECT *obj)
{
}
#endif

////////////////////
// weapon handler //
////////////////////
#ifndef _PSX
void TurboAIHandler(OBJECT *obj)
{
    TURBO2_OBJ *turbo = (TURBO2_OBJ*)obj->Data;
    int iWhl;

    turbo->Age += TimeStep;
    if (turbo->Age < turbo->LifeTime && obj->player->type != PLAYER_NONE)
    {
        obj->player->car.TopSpeed = obj->player->car.DefaultTopSpeed * (ONE + (ONE - turbo->Age / turbo->LifeTime));
        //obj->player->car.TopSpeed = TO_VEL(MPH2OGU_SPEED * 75) * (ONE + HALF * (ONE - turbo->Age / turbo->LifeTime));

        for (iWhl = 0; iWhl < CAR_NWHEELS; iWhl++) {
            obj->player->car.Wheel[iWhl].StaticFriction = Real(2.0f);
            obj->player->car.Wheel[iWhl].KineticFriction = Real(2.0f);
            obj->player->car.DownForceMod = Real(4.0);
            //obj->player->car.Wheel[iWhl].Grip = TO_GRIP(Real(0.025));
        }

    }
    else
    {
        obj->player->car.TopSpeed = obj->player->car.DefaultTopSpeed;

        for(iWhl = 0; iWhl < CAR_NWHEELS; iWhl++) {
            obj->player->car.Wheel[iWhl].StaticFriction = CarInfo[obj->player->car.CarType].Wheel[iWhl].StaticFriction;
            obj->player->car.Wheel[iWhl].KineticFriction = CarInfo[obj->player->car.CarType].Wheel[iWhl].KineticFriction;
            //obj->player->car.Wheel[iWhl].Grip = CarInfo[obj->player->car.CarType].Wheel[iWhl].Grip;
            obj->player->car.DownForceMod = CarInfo[obj->player->car.CarType].DownForceMod;

            if (GameSettings.PlayMode == MODE_KIDS) {
                obj->player->car.Wheel[iWhl].StaticFriction = (obj->player->car.Wheel[iWhl].StaticFriction * 5) / 4;
                obj->player->car.Wheel[iWhl].KineticFriction = obj->player->car.Wheel[iWhl].StaticFriction;
            }
        }

        #ifdef _N64
        obj->player->car.Models->Body->Flag &= ~MODEL_ENV_TURBO;
        #endif
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
#endif

////////////////////
// weapon handler //
////////////////////
#ifndef _PSX
void Turbo2Handler(OBJECT *obj)
{
    TURBO2_OBJ *turbo = (TURBO2_OBJ*)obj->Data;

#ifndef _PSX
 #ifdef OLD_AUDIO
    // maintain sfx
    if (obj->Sfx3D)
    {
        CopyVec(&obj->player->car.BodyWorldPos, &obj->Sfx3D->Pos);
    }
 #endif // !OLD_AUDIO
#endif

#ifdef _PC

    long i, rgb, lmul;
    MODEL *model = (MODEL*)&turbo->Model;

    POLY_RGB *mrgb;
    MODEL_VERTEX *mv;
    TURBO2_VERT *tvert;
    MODEL_POLY *mp;

// set vert UV's

    mv = model->VertPtr;
    tvert = (TURBO2_VERT*)(model->VertPtr + model->VertNum);

    for (i = model->VertNum ; i ; i--, mv++, tvert++)
    {
        tvert->Time += tvert->TimeAdd * TimeStep;

        mv->tu = (float)sin(tvert->Time) * tvert->Size + tvert->Offset;
        mv->tv = (float)cos(tvert->Time) * tvert->Size + (160.0f / 256.0f);
    }

// copy vert UV's to poly UV's + set rgb's

    if (turbo->Age < 0.5f)
    {
        FTOL(turbo->Age * 384.0f, lmul);
    }
    else if (turbo->Age > turbo->LifeTime - 0.5f)
    {
        FTOL((turbo->LifeTime - turbo->Age) * 384.0f, lmul);
    }
    else
    {
        lmul = 192;
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

        obj->Light->r = lmul / 4 + (rand() & 7);
        obj->Light->g = obj->Light->r / 2;
    }

// age / kill?
#endif

// N64 version of the effect... 
// simply swap the usual gamecar env map
//----------------------------------------
#ifdef _N64
    obj->player->car.Models->Body->Flag |= MODEL_ENV_TURBO;
#endif

    /*turbo->Age += TimeStep;
    if (turbo->Age < turbo->LifeTime && obj->player->type != PLAYER_NONE)
    {
        obj->player->car.TopSpeed = TO_VEL(MPH2OGU_SPEED * 75) * (ONE + HALF * (ONE - turbo->Age / turbo->LifeTime));
    }
    else
    {
        obj->player->car.TopSpeed = obj->player->car.DefaultTopSpeed;
        OBJ_FreeObject(obj);
        #ifdef _N64
        obj->player->car.Models->Body->Flag &= ~MODEL_ENV_TURBO;
        #endif

    }*/

}
#endif

////////////////////
// weapon handler //
////////////////////
#ifdef _PC
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

    if (obj->player->car.PowerTimer > ZERO) {
        FTOL(obj->player->car.PowerTimer * 80.0f, lmul);
    } else {
        lmul = 0;
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

// kill?

    if ((obj->player->car.PowerTimer <= ZERO) || (obj->player->type == PLAYER_NONE))
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
#endif

//////////////////////////
// render electro pulse //
//////////////////////////
#ifndef _PSX
void RenderElectroPulse(OBJECT *obj)
{
    long i;
    ELECTROPULSE_OBJ *electro = (ELECTROPULSE_OBJ*)obj->Data;
    CAR *car = &obj->player->car;
    VEC v1, v2, dir1, dir2;
    PLAYER *nplayer;

// draw model

    #ifdef _PC
    obj->renderflag.visible |= RenderObjectModel(&car->Body->Centre.WMatrix, &car->BodyWorldPos, &electro->Model, obj->EnvRGB, obj->EnvOffsetX, obj->EnvOffsetY, obj->EnvScale, obj->renderflag, FALSE);
    #endif

// draw jump spark?

    for (i = 0 ; i < electro->JumpFlag ; i++)
    {
        nplayer = (PLAYER*)electro->Player[i];

        RotTransVector(&nplayer->car.Body->Centre.WMatrix, &nplayer->car.BodyWorldPos, &electro->JumpPos1[i], &v1);
        RotTransVector(&obj->player->car.Body->Centre.WMatrix, &obj->player->car.BodyWorldPos, &electro->JumpPos2[i], &v2);

        VecEqScalarVec(&dir2, Real(300), &obj->player->car.Body->Centre.WMatrix.mv[L]);
        dir2.v[1] = Real(0);

        dir1.v[0] = Real(0);
        dir1.v[1] = Real(-100);
        dir1.v[2] = Real(0);

        DrawJumpSpark(&v1, &v2, &dir1, &dir2, 0);
    }
#endif

}

//////////////////////
// render oil slick //
/////////////////////
#ifdef _PC
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
        if (BlendSubtract)
        {
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
        }
        else
        {
            if (oil->Age < 28.0f)
            {
                rgb = 0xf0000000;
            }
            else
            {
                FTOL((30.0f - oil->Age) * 120, rgb);
                rgb <<= 24;
            }

            DrawShadow(&oil->Pos[0], &oil->Pos[1], &oil->Pos[2], &oil->Pos[3], 130.0f / 256.0f, 66.0f / 256.0f, 60.0f / 256.0f, 60.0f / 256.0f, rgb, -2.0f, 256.0f, 0, TPAGE_FX1, &box);
        }

#ifdef _N64
        oil->Ymin = box.Ymin - 4.0f;
        oil->Ymax = box.Ymax + 4.0f;
#else
        oil->Ymin = box.Ymin - 1.0f;
        oil->Ymax = box.Ymax + 1.0f;
#endif
    }
}

/////////////////////////
// render turbo2 model //
/////////////////////////

void RenderTurbo2(OBJECT *obj)
{
    TURBO2_OBJ *turbo = (TURBO2_OBJ*)obj->Data;
    CAR *car = &obj->player->car;

// draw model

    obj->renderflag.visible |= RenderObjectModel(&car->Body->Centre.WMatrix, &car->BodyWorldPos, &turbo->Model, obj->EnvRGB, obj->EnvOffsetX, obj->EnvOffsetY, obj->EnvScale, obj->renderflag, FALSE);
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
    MODEL_VERTEX *mv;
    MODEL_POLY *mp;
    long visflag;
    short flag = 0;
    long i;

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
    if (z - ball->Radius < RenderSettings.NearClip) flag |= MODEL_NEARCLIP;

// set visible flag

    obj->renderflag.visible = TRUE;

// force env textures

    SetEnvStatic(pos, &obj->body.Centre.WMatrix, 0x808080, 0.0f, 0.1f, 0.5f);

    mv = model->VertPtr;

    for (i = 0 ; i < model->VertNum ; i++, mv++)
    {
        mv->tu = (mv->nx * EnvMatrix.m[RX] + mv->ny * EnvMatrix.m[UX] + mv->nz * EnvMatrix.m[LX]) + 0.5f;
        mv->tv = (mv->nx * EnvMatrix.m[RY] + mv->ny * EnvMatrix.m[UY] + mv->nz * EnvMatrix.m[LY]) + 0.6f;
        mv->color = *(long*)&EnvRgb;
    } 

    mp = model->PolyPtr;

    for (i = 0 ; i < model->PolyNum ; i++, mp++)
    {
        mp->Tpage = TPAGE_ENVSTILL;
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

        *(long*)&model->PolyRGB[i].rgb[0] = 0xc0c0c0;
        *(long*)&model->PolyRGB[i].rgb[1] = 0xc0c0c0;
        *(long*)&model->PolyRGB[i].rgb[2] = 0xc0c0c0;
        *(long*)&model->PolyRGB[i].rgb[3] = 0xc0c0c0;
    }

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

    DrawShadow(&v0, &v1, &v2, &v3, 193.0f / 256.0f, 129.0f / 256.0f, 62.0f / 256.0f, 62.0f / 256.0f, BlendSubtract ? 0x808080 : 0x80000000, -2.0f, 0.0f, BlendSubtract ? 2 : 0, TPAGE_FX1, NULL);
}

///////////////////////////
// render electro zapped //
///////////////////////////

void RenderElectroZapped(OBJECT *obj)
{
    ELECTROZAPPED_OBJ *electro = (ELECTROZAPPED_OBJ*)obj->Data;
    CAR *car = &obj->player->car;

// draw model

    obj->renderflag.visible |= RenderObjectModel(&car->Body->Centre.WMatrix, &car->BodyWorldPos, &electro->Model, obj->EnvRGB, obj->EnvOffsetX, obj->EnvOffsetY, obj->EnvScale, obj->renderflag, FALSE);
}

//////////////////////
// render bomb glow //
//////////////////////

void RenderBombGlow(OBJECT *obj)
{
    BOMBGLOW_OBJ *glow = (BOMBGLOW_OBJ*)obj->Data;
    CAR *car = &obj->player->car;

// draw model

    obj->renderflag.visible |= RenderObjectModel(&car->Body->Centre.WMatrix, &car->BodyWorldPos, &glow->Model, obj->EnvRGB, obj->EnvOffsetX, obj->EnvOffsetY, obj->EnvScale, obj->renderflag, FALSE);
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

    obj->renderflag.visible |= RenderObjectModel(&mat, &vec, &LevelModel[obj->DefaultModel].Model, obj->EnvRGB, obj->EnvOffsetX, obj->EnvOffsetY, obj->EnvScale, obj->renderflag, TRUE);
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

// draw sphere / shockwave

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
            obj->renderflag.visible |= RenderObjectModel(&mat, &bomb->Pos, &LevelModel[obj->DefaultModel].Model, 0, obj->EnvOffsetX, obj->EnvOffsetY, obj->EnvScale, obj->renderflag, TRUE);

// draw shockwave

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
        if ((target == player) || (target->type == PLAYER_GHOST) || (target->type == PLAYER_NONE)) continue;

        // Separation dependence
        VecMinusVec(&target->car.Body->Centre.Pos, &player->car.Body->Centre.Pos, &dR);
        dR.v[Y] *= 5;
        dRLen = VecLen(&dR);
        if (dRLen > SMALL_REAL) {
            VecDivScalar(&dR, dRLen);
        } else {
            SetVecZero(&dR);
        }

        // Quick range check
        if ((dRLen > TARGET_RANGE_MAX) || (dRLen < TARGET_RANGE_MIN)) continue;

        // directional dependence
        lookdR = VecDotVec(&dR, &player->car.Body->Centre.WMatrix.mv[L]) - FIREWORK_DIR_OFFSET;

        // Quit if target out of range
        if ((dRLen > (TARGET_RANGE_MAX * lookdR)) || (lookdR < ZERO)) {
            continue;
        }

        // Generate target score for this player and see if it is the best
        score = (TARGET_RANGE_MAX / (dRLen + FIREWORK_RANGE_OFFSET)) * (lookdR * lookdR * lookdR * lookdR);

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


////////////////////////////////////////////////////////////////
//
// AttachWeaponCamera:
//
////////////////////////////////////////////////////////////////

void AttachWeaponCamera(OBJECT *obj)
{
#ifdef _PC
    // Make sure camera is free
    if (CAM_WeaponCamera != NULL) return;

    // Create the camera
    CAM_WeaponCamera = AddCamera(504, 192, 120, 90, CAMERA_FLAG_SECONDARY);
    if (CAM_WeaponCamera == NULL) return;

    // Set the camera type
    SetCameraFollow(CAM_WeaponCamera, obj, CAM_FOLLOW_WATCH);
#endif
}


////////////////////////////////////////////////////////////////
//
// DetachWeaponCamera:
//
////////////////////////////////////////////////////////////////

void DetachWeaponCamera(OBJECT *obj)
{
#ifdef _PC
    // Make sure camera exists
    if (CAM_WeaponCamera == NULL) return;

    // Make sure camera is attached to this object
    if (CAM_WeaponCamera->Object != obj) return;

    // Free camera
    RemoveCamera(CAM_WeaponCamera);
    CAM_WeaponCamera = NULL;
#endif
}



