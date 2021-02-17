//-----------------------------------------------------------------------------
// File: move.cpp
//
// Desc: Object processing code
//
// Re-Volt (Generic) Copyright (c) Probe Entertainment 1998
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
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
#include "LevelLoad.h"
#include "player.h"
#include "field.h"
#include "replay.h"
#include "ui_TitleScreen.h"
#ifdef _PC
#include "ghost.h"
#endif
#include "move.h"
#include "timing.h"
#ifdef _PC
#include "input.h"
#endif
#include "obj_init.h"

#ifdef _N64
#include "InitPlay.h"
#include "camera.h"
#include "gameloop.h"
#endif

//
// Global function prototypes
//

void MOV_MoveObjects(void);
void MOV_MoveBody(OBJECT *bodyObj);
void MOV_DropBody(OBJECT *obj);
void MOV_MoveCarNew(OBJECT *CarObj);
void MOV_DropCar(OBJECT *carObj);
void MOV_MoveGhost(OBJECT *CarObj);
void MOV_MoveTrain(OBJECT *obj);
void UpdateReplayPlayer(OBJECT *obj);
void UpdateRemotePlayer(OBJECT *obj);

//--------------------------------------------------------------------------------------------------------------------------

///////////////////
// move all objects
///////////////////

void MOV_MoveObjects(void)
{
    OBJECT  *obj, *next;


    obj = OBJ_ObjectHead;
    while (obj != NULL)
    {
        next = obj->next;

        if (obj->movehandler && obj->flag.Move)
        {
            // Move handler
            obj->movehandler(obj);
        }
        obj->renderflag.visible = FALSE;

        obj = next;
    }


}


////////////////////////////////////////////////////////////////
//
// Update the replay player's position if necessary
//
////////////////////////////////////////////////////////////////

void UpdateReplayPlayer(OBJECT *obj)
{
    CAR_REMOTE_DATA *rem2;
    CAR *car = &obj->player->car;

    rem2 = &car->RemoteData[car->NewDat];

    if (rem2->NewData) {
        CopyVec(&rem2->Pos, &obj->body.Centre.Pos);
        CopyVec(&rem2->Vel, &obj->body.Centre.Vel);
        CopyVec(&rem2->AngVel, &obj->body.AngVel);
        CopyQuat(&rem2->Quat, &obj->body.Centre.Quat);
        NormalizeQuat(&obj->body.Centre.Quat);
        QuatToMat(&obj->body.Centre.Quat, &obj->body.Centre.WMatrix);
        rem2->NewData = FALSE;
    }
    
}

/////////////////////////////////////////////////////////////////////
//
// UpdateRemotePlayer: if a new packet has been received from a 
// remote player, put the new values into its structure.
//
/////////////////////////////////////////////////////////////////////
#ifdef _PC

#define REMOTE_POS_INTERP_SCALE     Real(0.01f)
#define REMOTE_QUAT_INTERP_SCALE    Real(0.1f)
#define MOVE_INTERP_TIMEOUT         Real(0.5f)

REAL time0, time1, time2, timeNow;

void UpdateRemotePlayer(OBJECT *obj)
{
#if FALSE
    CAR_REMOTE_DATA *rem;
    CAR     *car;
    
    car = &obj->player->car;
    rem = &car->RemoteData[car->NewDat];

    // Make sure there is new data
    if (!rem->NewData) return;

    CopyVec(&rem->Pos, &car->Body->Centre.Pos);
    CopyVec(&rem->Vel, &car->Body->Centre.Vel);
    CopyVec(&rem->AngVel, &car->Body->AngVel);
    CopyQuat(&rem->Quat, &car->Body->Centre.Quat);

    // Flag data as used
    rem->NewData = FALSE;

#else
    //REAL time0, time1, time2, timeNow;
    CAR_REMOTE_DATA *rem0, *rem1, *rem2;
    CAR *car = &obj->player->car;

    rem0 = &car->RemoteData[car->OldDat];
    rem1 = &car->RemoteData[car->Dat];
    rem2 = &car->RemoteData[car->NewDat];

    if (car->RemoteNStored == 3) {
        // Calculate a destination from previously stored positions
        timeNow = (TotalRacePhysicsTime - rem0->Time) / Real(1000); //$NOTE(cprince): unsigned times causes HUGE result if (rem0->Time > TimeRacePhysicsTime)
        time0 = ZERO;
        time1 = (rem1->Time - rem0->Time) / Real(1000);
        time2 = (rem2->Time - rem0->Time) / Real(1000);

//$MODIFIED(cprince)
//        if ((time1 < MOVE_INTERP_TIMEOUT) && (abs(time2 - time1) < MOVE_INTERP_TIMEOUT) && (abs(timeNow - time2) < MOVE_INTERP_TIMEOUT)) {
        if(0) {
        //$BUG: Bad things were happening when we ran the code below.
        /// Specifically, time2==time1 caused NAN results in QuadInterpVec(),
        /// which got propagated through all computations here.  As a result, you'd
        /// get funky things like disappearing remote cars and a low framerate at
        /// startup, plus the cars would be sunken half way into the ground when they
        /// finally reappeared!
        ///
        /// Eventually we'll probably want to do some interpolation/extrapolation
        /// based on the time value, but bypassing this code fixed the problem
        /// for now (by falling back to direct data copy, without the extrapolation).
//$END_MODIFICATIONS
            VEC dPos, newPos;
            COLLINFO_WHEEL *wheelColl;
            COLLINFO_BODY *bodyColl;

            // Extrapolate position
            QuadInterpVec(&rem0->Pos, time0, &rem1->Pos, time1, &rem2->Pos, time2, timeNow, &obj->ReplayStoreInfo.InterpPos);
            ScalarVecPlusScalarVec(ONE - REMOTE_POS_INTERP_SCALE, &obj->body.Centre.Pos, REMOTE_POS_INTERP_SCALE, &obj->ReplayStoreInfo.InterpPos, &newPos);;
            VecMinusVec(&newPos, &obj->body.Centre.Pos, &dPos);

            // Remove component of change in position that goes into collision
            for (bodyColl = obj->body.BodyCollHead; bodyColl != NULL; bodyColl = bodyColl->Next) {
                if (bodyColl->Body2 == &BDY_MassiveBody) {
                    RemoveComponent(&dPos, PlaneNormal(&bodyColl->Plane));
                }
            }
            for (wheelColl = obj->player->car.WheelCollHead; wheelColl != NULL; wheelColl = wheelColl->Next) {
                if (wheelColl->Body2 == &BDY_MassiveBody) {
                    RemoveComponent(&dPos, PlaneNormal(&wheelColl->Plane));
                }
            }
            VecPlusEqVec(&obj->body.Centre.Pos, &dPos);

            // Extrapolate quaternion
            if (abs(time2 - time1) > Real(0.001f)) {
                SLerpQuat(&rem1->Quat, &rem2->Quat, (timeNow - time1) / (time2 - time1), &obj->ReplayStoreInfo.InterpQuat);
                ScalarQuatPlusScalarQuat(ONE - REMOTE_QUAT_INTERP_SCALE, &obj->body.Centre.Quat, REMOTE_QUAT_INTERP_SCALE, &obj->ReplayStoreInfo.InterpQuat, &obj->body.Centre.Quat);

                NormalizeQuat(&obj->body.Centre.Quat);
                QuatToMat(&obj->body.Centre.Quat, &obj->body.Centre.WMatrix);
            }

            car->RemoteTimeout = FALSE;
        } else {
            car->RemoteTimeout = TRUE;
            car->RemoteNStored = 1;
        }
    }

    // Update the velocity, ang velocity, and quaternion if there is new data
    if (rem2->NewData && car->RemoteNStored > 0) {
        if (car->RemoteTimeout) {
            CopyVec(&rem2->Pos, &obj->body.Centre.Pos);
            CopyQuat(&rem2->Quat, &obj->body.Centre.Quat);
            NormalizeQuat(&obj->body.Centre.Quat);
            QuatToMat(&obj->body.Centre.Quat, &obj->body.Centre.WMatrix);
        }
        CopyVec(&rem2->Vel, &obj->body.Centre.Vel);
        CopyVec(&rem2->AngVel, &obj->body.AngVel);
        rem2->NewData = FALSE;
    }

#endif

}
#endif


////////////////////////////////////////////////////////////////
//
// Update RemoteObject
//
////////////////////////////////////////////////////////////////
#ifdef _PC

void UpdateRemoteObject(OBJECT *obj)
{
#if FALSE
    OBJECT_REMOTE_DATA *rem;
    
    // Is there new data?
    rem = &obj->RemoteData[obj->NewDat];
    if (!rem->NewData) return;

    // Copy it to the object
    CopyVec(&rem->Pos, &obj->body.Centre.Pos);
    CopyVec(&rem->Vel, &obj->body.Centre.Vel);
    CopyQuat(&rem->Quat, &obj->body.Centre.Quat);
    CopyVec(&rem->AngVel, &obj->body.AngVel);
    QuatToMat(&obj->body.Centre.Quat, &obj->body.Centre.WMatrix);
    rem->NewData = FALSE;
#else
    bool remoteTimeout ;
    OBJECT_REMOTE_DATA *rem0, *rem1, *rem2;

    rem0 = &obj->RemoteData[obj->OldDat];
    rem1 = &obj->RemoteData[obj->Dat];
    rem2 = &obj->RemoteData[obj->NewDat];

    if (obj->RemoteNStored == 3) {
        // Calculate a destination from previously stored positions
        timeNow = (TotalRacePhysicsTime - rem0->Time) / Real(1000);
        time0 = ZERO;
        time1 = (rem1->Time - rem0->Time) / Real(1000);
        time2 = (rem2->Time - rem0->Time) / Real(1000);

        if ((time1 < MOVE_INTERP_TIMEOUT) && (abs(time2 - time1) < MOVE_INTERP_TIMEOUT) && (abs(timeNow - time2) < MOVE_INTERP_TIMEOUT)) {
            VEC dPos, newPos;
            COLLINFO_BODY *bodyColl;

            // Extrapolate position
            QuadInterpVec(&rem0->Pos, time0, &rem1->Pos, time1, &rem2->Pos, time2, timeNow, &obj->ReplayStoreInfo.InterpPos);
            ScalarVecPlusScalarVec(ONE - REMOTE_POS_INTERP_SCALE, &obj->body.Centre.Pos, REMOTE_POS_INTERP_SCALE, &obj->ReplayStoreInfo.InterpPos, &newPos);;
            VecMinusVec(&newPos, &obj->body.Centre.Pos, &dPos);

            // Remove component of change in position that goes into collision
            for (bodyColl = obj->body.BodyCollHead; bodyColl != NULL; bodyColl = bodyColl->Next) {
                if (bodyColl->Body2 == &BDY_MassiveBody) {
                    RemoveComponent(&dPos, PlaneNormal(&bodyColl->Plane));
                }
            }
            VecPlusEqVec(&obj->body.Centre.Pos, &dPos);

            // Extrapolate quaternion
            if (abs(time2 - time1) > Real(0.001f)) {
                SLerpQuat(&rem1->Quat, &rem2->Quat, (timeNow - time1) / (time2 - time1), &obj->ReplayStoreInfo.InterpQuat);
                ScalarQuatPlusScalarQuat(ONE - REMOTE_QUAT_INTERP_SCALE, &obj->body.Centre.Quat, REMOTE_QUAT_INTERP_SCALE, &obj->ReplayStoreInfo.InterpQuat, &obj->body.Centre.Quat);

                NormalizeQuat(&obj->body.Centre.Quat);
                QuatToMat(&obj->body.Centre.Quat, &obj->body.Centre.WMatrix);
            }

            remoteTimeout = FALSE;
        } else {
            remoteTimeout = TRUE;
            obj->RemoteNStored = 1;
        }
    } else {
        remoteTimeout = TRUE;
    }

    // Update the velocity, ang velocity, and quaternion if there is new data
    if (rem2->NewData && obj->RemoteNStored > 0) {
        if (remoteTimeout) {
            CopyVec(&rem2->Pos, &obj->body.Centre.Pos);
            CopyQuat(&rem2->Quat, &obj->body.Centre.Quat);
            NormalizeQuat(&obj->body.Centre.Quat);
            QuatToMat(&obj->body.Centre.Quat, &obj->body.Centre.WMatrix);
        }
        CopyVec(&rem2->Vel, &obj->body.Centre.Vel);
        CopyVec(&rem2->AngVel, &obj->body.AngVel);
        rem2->NewData = FALSE;
    }

#endif
}

#endif // _PC



/////////////////////////////////////////////////////////////////////
//
// MOV_MoveBodyClever: move an object. If the object has not moved for
// a while, stop moving it until a new collision with a moving body
// occurs.
//
/////////////////////////////////////////////////////////////////////

void MOV_MoveBodyClever(OBJECT *obj)
{

    // See if the body is moving
    //if ((abs(obj->body.Centre.Vel.v[X]) < MOVE_MIN_VEL) && (abs(obj->body.Centre.Vel.v[Y]) < MOVE_MIN_VEL) && (abs(obj->body.Centre.Vel.v[Z]) < MOVE_MIN_VEL)) {
    if (
        (abs(obj->body.Centre.Vel.v[X]) < TO_VEL(Real(30))) && 
        (abs(obj->body.Centre.Vel.v[Y]) < TO_VEL(Real(30))) && 
        (abs(obj->body.Centre.Vel.v[Z]) < TO_VEL(Real(30))) &&
        (abs(obj->body.AngVel.v[X]) < TO_ANGVEL(Real(1.0))) &&
        (abs(obj->body.AngVel.v[Y]) < TO_ANGVEL(Real(1.0))) &&
        (abs(obj->body.AngVel.v[Z]) < TO_ANGVEL(Real(1.0)))
        ) 
    {
        obj->body.NoMoveTime += TimeStep;
    } else {
        obj->body.NoMoveTime = ZERO;
    }

    

    // if body has not moved for a while, stop moving it altogether
    if ((obj->body.NoMoveTime > MOVE_MAX_NOMOVETIME)) {
        obj->defaultcollhandler = obj->collhandler;
        obj->defaultmovehandler = obj->movehandler;
        obj->collhandler = (COLL_HANDLER)COL_WaitForCollision;
        obj->movehandler = NULL;
        obj->body.NoMoveTime = ZERO;
        obj->body.Stacked = TRUE;
        SetVecZero(&obj->body.Centre.Vel);
        SetVecZero(&obj->body.AngVel);
        UpdateObjectGrid(obj);
        obj->body.CollSkin.AllowWorldColls = FALSE;
        return;
    }

    // Move the object as normal
    MOV_MoveBody(obj);
}


/////////////////////////////////////////////////////////////////////
//
// MOV_MoveBody: basic body moving
//
/////////////////////////////////////////////////////////////////////

void MOV_MoveBody(OBJECT *bodyObj)
{
#ifdef _PC
    if (IsMultiPlayer() && bodyObj->ServerControlled && !CountdownTime) {
        UpdateRemoteObject(bodyObj);
    }
#endif

    // Move the body
    UpdateBody(&bodyObj->body, TimeStep);

    // Put it in the grid system
    UpdateObjectGrid(bodyObj);
}


////////////////////////////////////////////////////////////////
//
// Mova a car
//
////////////////////////////////////////////////////////////////

void MOV_MoveCarNew(OBJECT *carObj)
{
    int ii, iWheel, nPowered;
    CAR *car = &carObj->player->car;
    VEC bodyShift;

    Assert(carObj->Type == OBJECT_TYPE_CAR || carObj->Type == OBJECT_TYPE_TROLLEY);

    switch (carObj->player->type) {
#ifdef _PC
    case PLAYER_REMOTE:
        UpdateRemotePlayer(carObj);
        break;
#endif // _PC
    case PLAYER_REPLAY:
        UpdateReplayPlayer(carObj);
    default: 
        break;
    }

    // Don't move cars if they aren't moving!
    if (
#ifdef _PSX
        abs(car->Body->Centre.Vel.v[X]) < TO_VEL(Real(30)) &&
        abs(car->Body->Centre.Vel.v[Z]) < TO_VEL(Real(30)) &&
        abs(car->Body->Centre.Vel.v[Y]) < TO_VEL(Real(30))
#else
        abs(car->Body->Centre.Vel.v[X]) < TO_VEL(Real(20)) &&
        abs(car->Body->Centre.Vel.v[Z]) < TO_VEL(Real(20)) &&
        abs(car->Body->Centre.Vel.v[Y]) < TO_VEL(Real(20)) &&
        abs(car->Body->AngVel.v[X]) < TO_ANGVEL(Real(0.5)) &&
        abs(car->Body->AngVel.v[Y]) < TO_ANGVEL(Real(0.5)) &&
        abs(car->Body->AngVel.v[Z]) < TO_ANGVEL(Real(0.5))
#endif
        )
    {
        car->Body->NoMoveTime += TimeStep;
    }
    else
    {
        car->Body->NoMoveTime = ZERO;
    }

    if (
        car->EngineVolt == 0 &&
        car->SteerAngle == 0 &&
        (carObj->player->controls.lastdigital & ~CTRL_FULLBRAKE) == 0 &&
        carObj->player->controls.lastdy == 0 &&
        car->Body->NoMoveTime > TO_TIME(Real(0.1)) &&
        !car->Righting && 
#ifdef _PSX
        (car->NWheelColls > 0 || car->Body->NBodyColls > 0)
#else
        car->NWheelFloorContacts == 0 &&
        car->Body->NOtherContacts == 0 &&
        (car->Body->NBodyColls > 1 || car->Body->Stacked)//stoppedMove)
#endif
        )
    {
        car->Body->Stacked = TRUE;
    } else {
        car->Body->Stacked = FALSE;
    }

    // Stop body moving if its has stopped moving!
    if (car->Body->Stacked) {
        SetVecZero(&car->Body->Centre.Impulse);
        SetVecZero(&car->Body->AngImpulse);
        SetVecZero(&car->Body->Centre.Vel);
        SetVecZero(&car->Body->AngVel);
        SetVecZero(&car->Body->Centre.Shift);
        car->Wheel[FL].Vel = ZERO;
        car->Wheel[FR].Vel = ZERO;
        car->Wheel[BL].Vel = ZERO;
        car->Wheel[BR].Vel = ZERO;
    }

    // Move the car according to the impulses applied to it
    CopyVec(&car->Body->Centre.Shift, &bodyShift);
    UpdateBody(car->Body, TimeStep);
    CopyBBox(&car->Body->CollSkin.BBox, &car->BBox);

    if (GameSettings.PlayMode != MODE_SIMULATION) {
        AddPosRadToBBox(&car->BBox, &car->Body->Centre.Pos, car->CollRadius);

        for (ii = 0; ii < car->NBodySpheres; ii++) {
            VecMulMat(&car->BodySphere[ii].Pos, &car->Body->Centre.WMatrix, &car->CollSphere[ii].Pos);
            VecPlusEqVec(&car->CollSphere[ii].Pos, &car->Body->Centre.Pos);
#if CALC_GOOD_CAR_BBOXES
            AddPosRadToBBox(&car->BBox, &car->CollSphere[ii].Pos, car->CollSphere[ii].Radius);
#else
            AddPosRadToBBox(&car->BBox, &car->CollSphere[ii].Pos, car->CollSphere[ii].Radius + TO_LENGTH(Real(15)));
#endif
        }
    }

    // Move the wheels relative to the car 
    car->Revs = ZERO;
    nPowered = 0;

    for (iWheel = 0; iWheel < CAR_NWHEELS; iWheel++) {
        if (IsWheelPresent(&car->Wheel[iWheel])) {
            VecPlusEqVec(&car->Wheel[iWheel].CentrePos, &bodyShift);
            UpdateCarWheel(car, iWheel, TimeStep);
#if !CALC_GOOD_CAR_BBOXES
            if (GameSettings.PlayMode == MODE_SIMULATION) 
#endif
            {
                AddPosRadToBBox(&car->BBox, &car->Wheel[iWheel].CentrePos, car->Wheel[iWheel].Radius);
            }
        }
        // Calculate car revs
        if (IsWheelPowered(&car->Wheel[iWheel])) {
            car->Revs += 8 * MulScalar(car->Wheel[iWheel].AngVel, car->Wheel[iWheel].Radius);
            nPowered++;
        }
    }
    if (nPowered > 0) {
        car->Revs /= nPowered;
    }

    // Move the Aerial
    if (CarHasAerial(car)) 
    {
        UpdateCarAerial2(car, TimeStep);
    }

    // Put it in the grid system
#ifndef _PSX
    UpdateObjectGrid(carObj);
#endif
}


/////////////////////////////////////////////////////////////////////
//
// Drop a car until its settled on the floor
//
/////////////////////////////////////////////////////////////////////

void MOV_DropCar(OBJECT *carObj)
{
    REAL ang;

    carObj->body.Centre.Impulse.v[X] = carObj->body.Centre.Impulse.v[Z] = ZERO;
    ang = VecDotVec(&carObj->body.AngImpulse, &carObj->body.Centre.WMatrix.mv[U]);
    VecPlusEqScalarVec(&carObj->body.AngImpulse, -ang, &carObj->body.Centre.WMatrix.mv[U]);

    MOV_MoveCarNew(carObj);
}


////////////////////////////////////////////////////////////////
//
// Drop a body ignoring and impulses in horizontal plane
//
////////////////////////////////////////////////////////////////

void MOV_DropBody(OBJECT *obj)
{
    REAL ang;

    obj->body.Centre.Impulse.v[X] = obj->body.Centre.Impulse.v[Z] = ZERO;
    ang = VecDotVec(&obj->body.AngImpulse, &obj->body.Centre.WMatrix.mv[U]);
    VecPlusEqScalarVec(&obj->body.AngImpulse, -ang, &obj->body.Centre.WMatrix.mv[U]);
    //obj->body.AngImpulse.v[Y] = ZERO;

    MOV_MoveBodyClever(obj);
}

////////////////////////////////////////////////////////////////
//
// put a car back on ots wheels
//
////////////////////////////////////////////////////////////////

void MOV_RightCar(OBJECT *obj)
{
    int iWheel;
    REAL lookLen;
    VEC dR;
    MAT mat;
    CAR *car = &obj->player->car;

    Assert((obj->Type == OBJECT_TYPE_CAR) || (obj->Type == OBJECT_TYPE_TROLLEY));

    // Choose an orientation and destination position
    if (!car->Righting) {

        // New position
        //VecPlusScalarVec(&car->Body->Centre.Pos, TO_LENGTH(Real(50)), &UpVec, &car->DestPos);
        SetVec(&car->DestPos, car->Body->Centre.Pos.v[X], car->Body->Centre.Pos.v[Y] - TO_LENGTH(Real(50)), car->Body->Centre.Pos.v[Z]);

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

        // Convert to quaternion (and make sure both are in same hemisphere)
        MatToQuat(&mat, &car->DestQuat);
        ConstrainQuat2(&car->DestQuat, &car->Body->Centre.Quat);

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
    car->Body->Stacked = FALSE;

    // Right the car
    VecMinusVec(&car->DestPos, &car->Body->Centre.Pos, &dR);
#ifndef _PSX
    VecPlusEqScalarVec(&car->Body->Centre.Pos, TimeStep * 10, &dR);
    SLerpQuat(&car->Body->Centre.Quat, &car->DestQuat, TimeStep * 8, &car->Body->Centre.Quat);
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
    if (CarHasAerial(car)) 
    {
        UpdateCarAerial2(car, TimeStep);
    }

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
        obj->movehandler = obj->defaultmovehandler;
    }
}

////////////////////////////////////////////////////////////////
//
// reposition car
//
////////////////////////////////////////////////////////////////

void MOV_RepositionCar(OBJECT *obj)
{
    CAR *car = &obj->player->car;
    REAL ts;

// car?

    Assert(obj->Type == OBJECT_TYPE_CAR);


// dec repos timer

    car->RepositionTimer -= TimeStep;
    if (car->RepositionTimer <= 0 )
    {
        car->RepositionTimer = 0;
        car->RenderFlag = CAR_RENDER_NORMAL;
        obj->movehandler = obj->defaultmovehandler;
        return;
    }

    
// start fade down?

#ifdef _PC
    if (car->RepositionTimer < TO_TIME(Real(1.5)) && car->RepositionTimer > TO_TIME(Real(1.0)) && GetFadeEffect() == FADE_UP_DONE)
    {
        if (CAM_MainCamera && 
            (GameSettings.GameType != GAMETYPE_DEMO) &&
            (CAM_MainCamera->Type == CAM_FOLLOW || CAM_MainCamera->Type == CAM_ATTACHED) && 
            (CAM_MainCamera->Object == obj))
        {
            SetFadeEffect(FADE_DOWN_STAY);
        }
    }
#endif

// reset car pos

    if (car->RepositionTimer < TO_TIME(Real(1.0)) && !car->RepositionHalf)
    {
#ifdef _N64
        int ii;
#endif

        car->RepositionHalf = TRUE;
        CAI_ResetCar(obj->player);
        obj->player->car.Body->CollSkin.AllowObjColls = TRUE;
#ifndef _PSX
        obj->OutOfBounds = FALSE;
#endif

        #if defined(_N64)
        //---------------
        for (ii = 0; ii < StartData.LocalNum; ii++)
            {
            if (CAM_PlayerCameras[ii] && (CAM_PlayerCameras[ii]->Type == CAM_FOLLOW || CAM_PlayerCameras[ii]->Type == CAM_ATTACHED) && CAM_PlayerCameras[ii]->Object == obj)
                InitCamPos(CAM_PlayerCameras[ii]);
            }

        #elif defined(_PC)  // PC

        //---------------

        if (CAM_MainCamera && (GameSettings.GameType != GAMETYPE_DEMO) && (CAM_MainCamera->Type == CAM_FOLLOW || CAM_MainCamera->Type == CAM_ATTACHED) && CAM_MainCamera->Object == obj)
        {
            InitCamPos(CAM_MainCamera);

            #ifndef _N64    // PC only
            //---------------
            SetFadeEffect(FADE_UP);
            #endif
        }

        #else

        // PSX ????????

        #endif // N64, PC, PSX
        //---------------

    }

// move car

    ts = TimeStep;

    TimeStep = MulScalar(abs(FROM_TIME(car->RepositionTimer) - Real(1.0)), TimeStep);
    MOV_MoveCarNew(obj);

    TimeStep = ts;
}

/////////////////////////////////////////////////////////////////
//
// move ghost car
//
/////////////////////////////////////////////////////////////////

#ifndef _PSX
void MOV_MoveGhost(OBJECT *CarObj)
{
    // Interpolate the ghost car data
    InterpGhostData(&CarObj->player->car);

    // Move the aerial
    if (CarHasAerial(&CarObj->player->car)) 
    {
        UpdateCarAerial2(&CarObj->player->car, TimeStep);
    }
}
#endif

/////////////////////////////////////////////////////////////////
//
// move toy 2 train
//
/////////////////////////////////////////////////////////////////

void MOV_MoveTrain(OBJECT *obj)
{
    VEC vec;

    // move train
    CopyVec(&obj->body.Centre.Pos, &obj->body.Centre.OldPos);
    obj->body.Centre.Pos.v[Z] -= TimeStep * Real(200.0f);
    if (obj->body.Centre.Pos.v[Z] < -11500)
        obj->body.Centre.Pos.v[Z] = -400;
    VecMinusVec(&obj->body.Centre.Pos, &obj->body.Centre.OldPos, &vec);
    TransCollPolys(obj->body.CollSkin.WorldCollPoly, obj->body.CollSkin.NCollPolys, &vec);

    // update collision skin
    BuildWorldSkin(&obj->body.CollSkin, &obj->body.Centre.Pos, &obj->body.Centre.WMatrix);

}


////////////////////////////////////////////////////////////////
//
// Move UFO (almost flies...)
//
////////////////////////////////////////////////////////////////

void MOV_MoveUFO(OBJECT *obj) 
{
    REAL impDotLook, impDotRight, impDotUp, angImp;
    VEC impulse, rotAxis;
    CAR *car = &obj->player->car;

    // If the UFO is on the ground, treat as normal
    if (obj->player->car.NWheelFloorContacts == 0) {

        // Drive UFO Forards, and keep it in the air longer
        impDotLook = 2000 * car->EngineVolt * TimeStep * abs(obj->body.Centre.WMatrix.m[UY]);
        impDotRight = -5 * VecDotVec(&obj->body.Centre.WMatrix.mv[R], &obj->body.Centre.Vel) * TimeStep;
        if ((obj->body.Centre.Vel.v[Y] >= ZERO) && (obj->body.Centre.WMatrix.m[UY] > ZERO)){
            impDotUp = Real(0.8) * FLD_Gravity * TimeStep * car->Body->Centre.Mass;
        } else {
            impDotUp = ZERO;
        }
        ScalarVecPlusScalarVec(impDotLook, &obj->body.Centre.WMatrix.mv[L], -impDotUp, &obj->body.Centre.WMatrix.mv[U], &impulse);
        VecPlusEqScalarVec(&impulse, impDotRight, &obj->body.Centre.WMatrix.mv[R]);
        ApplyParticleImpulse(&obj->body.Centre, &impulse);

        // Turn UFO
        angImp = Real(15000) * car->SteerAngle * TimeStep;
        VecPlusEqScalarVec(&obj->body.AngImpulse, angImp, &obj->body.Centre.WMatrix.mv[U]);

        // Keep UFO upright
        VecCrossVec(&obj->body.Centre.WMatrix.mv[U], &DownVec, &rotAxis);
        VecEqScalarVec(&impulse, 20000 * TimeStep, &rotAxis);
        VecPlusEqVec(&obj->body.AngImpulse, &impulse);  
    }

    // Make UFO float above the ground
//  VecPlusScalarVec(&obj->body.Centre.Pos, TO_LENGTH(Real(25)), &obj->body.Centre.WMatrix.mv[U], &pos);
//  if (!LineOfSight(&obj->body.Centre.Pos, &pos)) {
//      VecPlusEqScalarVec(&obj->body.Centre.Impulse, -10000 * TimeStep, &obj->body.Centre.WMatrix.mv[U]);
//  }


    // Move as normal
    MOV_MoveCarNew(obj);
}
