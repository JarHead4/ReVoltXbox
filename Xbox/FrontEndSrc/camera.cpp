//-----------------------------------------------------------------------------
// File: Camera.cpp
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"
#include "geom.h"
#include "model.h"
#include "network.h"
#include "particle.h"
#include "aerial.h"
#include "NewColl.h"
#include "Body.h"
#include "input.h"
#include "main.h"
#include "camera.h"
#include "ctrlread.h"
#include "object.h"
#include "player.h"
#include "LevelLoad.h"
#include "ghost.h"
#include "visibox.h"
#include "weapon.h"
#include "obj_init.h"
#include "InitPlay.h"


#define CAM_MOVE_SPEED Real(2.0f)
#define MIN_CAM_SWITCH_TIME TO_TIME(Real(2.0f))

void AttachObjectToCamera(OBJECT *obj, CAMERA * cam);
void DetachObjectFromCamera(CAMERA *cam);
void DetachCamerasFromObject(OBJECT *obj);


// Camera Type setting function prototypes
void SetCameraFollow(CAMERA *camera, OBJECT *object, long type);
void SetCameraAttached(CAMERA *camera, OBJECT *object, long type);
void SetCameraFreedom(CAMERA *camera, OBJECT *object, long unUsed);
void SetCameraRail(CAMERA *camera, OBJECT *object, long unUsed);
void SetCameraNewFollow(CAMERA *camera, OBJECT *object, long type);
#ifdef _PC
void SetCameraEdit(CAMERA *camera, OBJECT *object, long unUsed);
#endif

// Camera matrix calculating functions
void CameraAwayLook(CAMERA *camera);
void CameraMouseLook(CAMERA *camera);
void CameraNullLook(CAMERA *camera);
void CameraForwardLook(CAMERA *camera);
void CameraRearLook(CAMERA *camera);
void CameraWatchLook(CAMERA *camera);

// Camera position calculating functions
void InitCamPos(CAMERA *camera);
void CameraSweepPos(CAMERA *camera);
void CameraFollowPos(CAMERA *camera);
void CameraRotatePos(CAMERA *camera);
//void CameraRailPos(CAMERA *camera);
void CameraFreedomPos(CAMERA *camera);
void CameraRelativePos(CAMERA *camera);
void CameraNewFollowPos(CAMERA *camera);
void CameraNearestNodePos(CAMERA *camera);
void CameraDynamicNodePos(CAMERA *camera);
#ifdef _PC
void CameraEditPos(CAMERA *camera);
#endif

// Misc Camera functions
void CameraWorldColls(CAMERA *camera);
void CalcCamZoom(CAMERA *camera);
void CalcCameraCollPoly(CAMERA *camera);
CAMNODE *NearestNode(long type, VEC *pos);
bool CameraStartEndNodes(CAMERA *camera);


// globals

char CameraCount;
REAL CameraHomeHeight;
MAT ViewMatrixScaled, ViewMatrix, ViewCameraMatrix, ViewMatrixScaledMirrorY;
VEC ViewTransScaled, ViewTrans, ViewCameraPos;
float MouseXpos = REAL_SCREEN_XHALF, MouseYpos = REAL_SCREEN_YHALF, MouseXrel, MouseYrel;
float CameraEditXrel, CameraEditYrel, CameraEditZrel;
char MouseLeft, MouseRight, MouseLastLeft, MouseLastRight;
PLANE CameraPlaneLeft, CameraPlaneRight, CameraPlaneTop, CameraPlaneBottom;
CAMERA Camera[MAX_CAMERAS];
#ifdef _PC
D3DRECT ViewportRect;
#endif

bool gAllowCameraChange = TRUE;

static VEC LastPoleVector, LastPoleVectorGhost;


CAMNODE CAM_CameraNode[CAMERA_MAX_NODES];
long    CAM_NCameraNodes = 0;

CAMNODE *CAM_StartNode = NULL;
CAMNODE *CAM_EndNode = NULL;

VEC CAM_NodeCamPos = {ZERO, ZERO, ZERO};
VEC CAM_NodeCamLookPos = {ZERO, ONE, ZERO};
VEC CAM_NodeCamOldPos = {ZERO, ZERO, ZERO};
VEC CAM_NodeCamDir = {ONE, ZERO, ZERO};
REAL    CAM_NodeCamPoleLen = ZERO;
bool    CAM_NodeCamDoColls = TRUE;
bool    CAM_LineOfSight = TRUE;

// Special Cameras
CAMERA *CAM_WeaponCamera = NULL;
CAMERA *CAM_RearCamera = NULL;
CAMERA *CAM_OtherCamera = NULL;
#ifndef _N64
CAMERA *CAM_MainCamera = NULL;
#else
CAMERA *CAM_PlayerCameras[MAX_LOCAL_PLAYERS];
#endif


static REAL OuterRadius = 120.0f;
static REAL InnerRadius = 65.0f;

static CAMFOLLOWDATA CamFollowData[CAM_FOLLOW_NTYPES] = {
    { // Behind Camera
        TRUE,
        {0, -150, -460},
        {0, -60, 0}
    },
    { // Close Behind Camera
        TRUE,
//      {0, -70, -210},
//      {0, -40, 0}
#ifdef _PC
        {0, -120, -210},
        {0, -70, 0}
#else
        {0, -100, -250},
        {0, -50, 0}
#endif
    },
    { // Side Left Camera
        TRUE,
        {-310, -95, 0},
        {0, -20, 15}
    },
    { // Side Right Camera
        TRUE,
        {310, -95, 0},
        {0, -20, 15}
    },
    { // Front Camera
        TRUE,
        {0, -90, 210},
        {0, -30, 0}
    },
    { // Rotate Camera
        TRUE,
        {0, -120, -280},
        {0, -60, 0}
    },
    { // Watch Camera
        TRUE,
//      {0, -40, -100},
        {0, 0, -500},
        {0, 0, 0},
    },

};

static CAMATTACHEDDATA CamAttachedData[CAM_ATTACHED_NTYPES] = {
    { // In Car Camera
        {0, -44, -20},
        TRUE
    },
    { // Wheel Right Camera
        {60, -40, -190},
        TRUE
    },
    { // Wheel Left Camera
        {-60, -40, -190},
        TRUE
    },
    { // Rear View Camera
        {0, -44, 0},
        FALSE
    },
};


/////////////////////////////////////////////////////////////////////
//
// Camera attach and detach functions
//
/////////////////////////////////////////////////////////////////////

void AttachObjectToCamera(OBJECT *obj, CAMERA * cam)
{
    cam->Object = obj;
}

void DetachCamerasFromObject(OBJECT *obj)
{
    long iCam;

    for (iCam = 0; iCam < MAX_CAMERAS; iCam++) {
        if (Camera[iCam].Flag == CAMERA_FLAG_FREE) continue;

        if (Camera[iCam].Object == obj) {
            RemoveCamera(&Camera[iCam]);
        }
    }
}

void DetachObjectFromCamera(CAMERA *cam)
{
    cam->Object = NULL;
}


/////////////////////////////////////////////////////////////////////
//
// UpdateCamera: update the position and matrix of the desired
// camera
//
/////////////////////////////////////////////////////////////////////

void UpdateCamera(CAMERA *camera)
{
    REAL dist;
    VEC out;
    MAT mat;
#ifdef _PC

    // Move the camera offset if required
    if (camera->Type == CAM_FOLLOW)
    {
        if (!GameSettings.Paws)
        {
            dist = Length(&camera->DestOffset);
            SetVector(&out, 0, 0, 0);
            BuildLookMatrixForward(&out, &camera->DestOffset, &mat);
        }
    }

#endif

    camera->Timer += TimeStep;
    camera->HasCollided = camera->HasCollidedWithWall = FALSE;

    // Calculate camera's world position
    if (camera->CalcCamPos != NULL) {
        camera->CalcCamPos(camera);
    }
    // Update the Lens modifier if necessary
    if (camera->Zoom && (camera->Object != NULL)) {
        CalcCamZoom(camera);
    }
    // Calculate camera matrix
    if (camera->CalcCamLook != NULL) {
        camera->CalcCamLook(camera);
    }

    // Create the collision polygon for the view glass
#ifdef _PC
    CalcCameraCollPoly(camera);
#endif

    // Decrease shake?
    if (camera->Shake)
    {
        camera->Shake -= TimeStep;
        if (camera->Shake < 0.0f)
            camera->Shake = 0.0f;
    }

    // clear underwater flag
    camera->UnderWater = FALSE;
}


/////////////////////////////////////////////////////////////////////
//
// InitCamPos: set the starting position of the passed camera
//
/////////////////////////////////////////////////////////////////////

void InitCamPos(CAMERA *camera) 
{
    MAT mat;

    // Set default stuf, same for all camera types
    SetVecZero(&camera->Vel);

    switch (camera->Type) {
        // Follow Camera
    case CAM_FOLLOW:
        // get desired camera world matrix
        mat.m[RX] = camera->Object->body.Centre.WMatrix.m[LZ];
        mat.m[RY] = ZERO;
        mat.m[RZ] = -camera->Object->body.Centre.WMatrix.m[LX];

        if (!mat.m[RX] && !mat.m[RZ])
            mat.m[RX] = ONE;

        NormalizeVector(&mat.mv[R]);

        mat.m[UX] = ZERO;
        mat.m[UY] = ONE;
        mat.m[UZ] = ZERO;

        CrossProduct(&mat.mv[R], &mat.mv[U], &mat.mv[L]);

        // get desired offset vector
        RotVector(&mat, &camera->PosOffset, &camera->WorldPosOffset);
        VecPlusVec(&camera->WorldPosOffset, &camera->Object->body.Centre.Pos, &camera->WPos);
        CopyVec(&camera->WPos, &camera->OldWPos);


        CameraAwayLook(camera);
        break;

    case CAM_ATTACHED:
        VecMulMat(&camera->PosOffset, &camera->Object->body.Centre.WMatrix, &camera->WPos);
        VecPlusEqVec(&camera->WPos, &camera->Object->body.Centre.Pos);
        MatToQuat(&camera->Object->body.Centre.WMatrix, &camera->Quat);
        CopyMat(&camera->Object->body.Centre.WMatrix, &camera->WMatrix);
        break;

    case CAM_RAIL:
        switch (camera->SubType) {
        case CAM_RAIL_DYNAMIC_MONO:
            break;
        case CAM_RAIL_STATIC_NEAREST:
            break;
        default:
            break;
        }

    default:
        break;

    }
}


/////////////////////////////////////////////////////////////////////
//
// SetCamera#####: Set the camera to the specified type and subtype
//
/////////////////////////////////////////////////////////////////////
#ifdef _PC
void SetCameraNewFollow(CAMERA *camera, OBJECT *object, long followType)
{
    Assert(followType < CAM_FOLLOW_NTYPES);

    camera->Type = CAM_NEWFOLLOW;
    camera->SubType = followType;
    camera->CalcCamPos = CameraNewFollowPos;
    camera->CalcCamLook = CameraAwayLook;
    AttachObjectToCamera(object, camera);
    camera->Collide = CamFollowData[followType].Collide;
    camera->Zoom = FALSE;
    camera->ZoomMod = LENS_DIST_MOD;
    camera->Lens = ZERO;
    camera->Timer = ZERO;
    camera->LoS = TRUE;
    camera->LosScale = ONE;

    CopyVec(&CamFollowData[followType].PosOffset, &camera->DestOffset);
    CopyVec(&CamFollowData[followType].LookOffset, &camera->LookOffset);
    CopyVec(&CamFollowData[followType].LookOffset, &camera->OldLookOffset);
    InitCamPos(camera);
}
#endif

void SetCameraFollow(CAMERA *camera, OBJECT *object, long followType)
{
    Assert(followType < CAM_FOLLOW_NTYPES);

    camera->Type = CAM_FOLLOW;
    camera->SubType = followType;
    switch (followType) {

    case CAM_FOLLOW_ROTATE:
        camera->CalcCamPos = CameraRotatePos;
        camera->CalcCamLook = CameraAwayLook;
        camera->Lens = ZERO;
        break;

    case CAM_FOLLOW_WATCH:
        camera->CalcCamPos = CameraFollowPos;
        camera->CalcCamLook = CameraWatchLook;
        camera->Lens = 0.5f * BaseGeomPers;
        break;

    default:
        camera->CalcCamPos = CameraFollowPos;
        camera->CalcCamLook = CameraAwayLook;
        camera->Lens = ZERO;
        break;
    }

    AttachObjectToCamera(object, camera);
    camera->Collide = CamFollowData[followType].Collide;
    camera->Timer = ZERO;
    camera->Zoom = FALSE;
    camera->ZoomMod = LENS_DIST_MOD;
    camera->LoS = TRUE;
    camera->LosScale = ONE;

    CopyVec(&CamFollowData[followType].PosOffset, &camera->DestOffset);
    CopyVec(&CamFollowData[followType].LookOffset, &camera->LookOffset);
    CopyVec(&CamFollowData[followType].LookOffset, &camera->OldLookOffset);
    InitCamPos(camera);
}

void SetCameraSweep(CAMERA *camera, OBJECT *object, long gridIndex)
{
    int grid0, grid1;
    VEC pos0, pos1;
    MAT mat;

    camera->Type = CAM_SWEEP;

    AttachObjectToCamera(object, camera);

    camera->CalcCamPos = CameraSweepPos;
    camera->CalcCamLook = CameraAwayLook;
    camera->Lens = ZERO;

    camera->Collide = FALSE;
    camera->Timer = ZERO;
    camera->Zoom = FALSE;
    camera->ZoomMod = LENS_DIST_MOD;
    camera->LoS = TRUE;
    camera->LosScale = ONE;

    // Calculate the start and end points of the sweep
    grid0 = gridIndex;
    if (grid0 < DEFAULT_RACE_CARS - 1) {
        grid1 = grid0 + 1;
    } else {
        grid1 = grid0 - 1;
    }

    GetCarStartGrid(grid0, &pos0, &mat);
    GetCarStartGrid(grid1, &pos1, &mat);

    VecPlusEqVec(&pos0, &pos1);
    VecMulScalar(&pos0, HALF);
    pos0.v[Y] -= TO_LENGTH(Real(150));

    VecPlusScalarVec(&pos0, TO_LENGTH(Real(3000)), &mat.mv[L], &camera->PosOffset);
    VecPlusScalarVec(&pos0, TO_LENGTH(Real(-300)), &mat.mv[L], &camera->DestOffset);
    //VecPlusVec(&camera->Object->body.Centre.Pos, &CamFollowData[CAM_FOLLOW_BEHIND].PosOffset, &camera->DestOffset);

    // Lookoffset
    CopyVec(&CamFollowData[CAM_FOLLOW_BEHIND].LookOffset, &camera->LookOffset);
    CopyVec(&CamFollowData[CAM_FOLLOW_BEHIND].LookOffset, &camera->OldLookOffset);
    //VecMinusVec(&camera->DestOffset, &object->body.Centre.Pos, &camera->LookOffset);
    CopyVec(&camera->LookOffset, &camera->OldLookOffset);

    SetVecZero(&camera->Vel);

    gAllowCameraChange = FALSE;
}

void SetCameraAttached(CAMERA *camera, OBJECT *object, long attachedType)
{
    VEC offset;
    Assert(attachedType < CAM_ATTACHED_NTYPES);

    camera->Type = CAM_ATTACHED;
    camera->SubType = attachedType;
    camera->CalcCamPos = CameraRelativePos;
    if (CamAttachedData[attachedType].Forward) {
        camera->CalcCamLook = CameraForwardLook;
    } else {
        camera->CalcCamLook = CameraRearLook;
    }
    AttachObjectToCamera(object, camera);
    camera->Collide = FALSE;
    camera->Zoom = FALSE;
    camera->ZoomMod = LENS_DIST_MOD;
    camera->Lens = ZERO;
    camera->Timer = ZERO;
    camera->LoS = TRUE;
    camera->LosScale = ONE;

#if 0
    SetVec(&offset, 
        (object->body.CollSkin.TightBBox.XMax + object->body.CollSkin.TightBBox.XMin) * HALF,
        (object->body.CollSkin.TightBBox.YMax + object->body.CollSkin.TightBBox.YMin) * HALF,
        (object->body.CollSkin.TightBBox.ZMax + object->body.CollSkin.TightBBox.ZMin) * HALF);
#else 
    CopyVec(&CamAttachedData[attachedType].PosOffset, &offset);
#endif

    CopyVec(&offset, &camera->DestOffset);
    CopyVec(&offset, &camera->WorldPosOffset);
    InitCamPos(camera);
}

void SetCameraRail(CAMERA *camera, OBJECT *object, long type)
{
    CAMNODE *node;

    Assert(type < CAM_RAIL_NTYPES);

    camera->Type = CAM_RAIL;
    camera->SubType = type;
    camera->CalcCamLook = CameraAwayLook;
    AttachObjectToCamera(object, camera);
    camera->Collide = TRUE;
    camera->Timer = MIN_CAM_SWITCH_TIME;
    camera->LoS = TRUE;
    camera->LosScale = ONE;

    // Find the nearest camera node and set sub-type according to the node type
    node = NearestNode(-1, &object->body.Centre.Pos);
    if (node == NULL)
    {
        SetCameraFollow(camera, object, CAM_FOLLOW_BEHIND);
        return;
    }

    // Set sub-type specific stuff
    switch (type) {
    case CAM_RAIL_DYNAMIC_MONO:
        camera->Zoom = TRUE;
        camera->ZoomMod = LENS_DIST_MOD;
        camera->CalcCamPos = CameraDynamicNodePos;
        break;
    case CAM_RAIL_STATIC_NEAREST:
        camera->CalcCamPos = CameraNearestNodePos;
        camera->Zoom = TRUE;
        camera->ZoomMod = LENS_DIST_MOD;
        break;
    default:
        break;
    }

    //InitCamPos(camera);
}

void SetCameraFreedom(CAMERA *camera, OBJECT *object, long unUsed)
{
    camera->Type = CAM_FREEDOM;
    camera->SubType = 0;
    camera->CalcCamPos = CameraFreedomPos;
    if (object == NULL) {
        camera->CalcCamLook = CameraMouseLook;
        DetachObjectFromCamera(camera);
    } else {
        camera->CalcCamLook = CameraAwayLook;
        AttachObjectToCamera(object, camera);
        SetVecZero(&camera->LookOffset);
        SetVecZero(&camera->OldLookOffset);
    }
    camera->Collide = FALSE;
    camera->Zoom = FALSE;
    camera->Timer = ZERO;
    camera->LoS = TRUE;
    camera->LosScale = ONE;

}

#ifdef _PC
void SetCameraEdit(CAMERA *camera, OBJECT *object, long unUsed)
{
    camera->Type = CAM_EDIT;
    camera->SubType = 0;
    camera->CalcCamPos = CameraEditPos;
    camera->CalcCamLook = CameraNullLook;
    DetachObjectFromCamera(camera);
    camera->Timer = ZERO;
    camera->LoS = TRUE;
    camera->LosScale = ONE;
}
#endif


////////////////////////////////////////////////////////////////
//
// Camera Sweep Pos
//
////////////////////////////////////////////////////////////////
#define CAMERA_SWEEP_TIME TO_TIME(Real(3))

void CameraSweepPos(CAMERA *camera)
{
    long followType = CAM_FOLLOW_BEHIND;
    REAL t = DivScalar(camera->Timer, CAMERA_SWEEP_TIME);

    if (t > ONE) t = ONE;

    CopyVec(&camera->WPos, &camera->OldWPos);
    InterpVec(&camera->PosOffset, &camera->DestOffset, t, &camera->WPos);

    if (TimeStep > SMALL_REAL) {
        VecMinusVec(&camera->WPos, &camera->OldWPos, &camera->Vel);
        VecDivScalar(&camera->Vel, TimeStep);
    } else {
        SetVecZero(&camera->Vel);
    }

    InterpVec(&camera->OldLookOffset, &CamFollowData[followType].LookOffset, t, &camera->LookOffset);


    if (t == ONE) {
        VecMinusVec(&camera->WPos, &camera->Object->body.Centre.Pos, &camera->WorldPosOffset);

        MatMulVec(&camera->Object->body.Centre.WMatrix, &camera->WorldPosOffset, &camera->PosOffset);

        SetCameraFollow(camera, camera->Object, followType);
        camera->CalcCamPos = CameraFollowPos;
        camera->Collide = CamFollowData[followType].Collide;
        camera->Timer = ZERO;
        camera->Zoom = FALSE;
        camera->ZoomMod = LENS_DIST_MOD;
        camera->LoS = TRUE;
        camera->LosScale = ONE;

        CopyVec(&CamFollowData[followType].PosOffset, &camera->DestOffset);
        CopyVec(&CamFollowData[followType].LookOffset, &camera->LookOffset);
        CopyVec(&CamFollowData[followType].LookOffset, &camera->OldLookOffset);

        gAllowCameraChange = TRUE;
    }
}


/////////////////////////////////////////////////////////////////////
//
// CameraForwardLook: use the object's matrix for the camera
//
/////////////////////////////////////////////////////////////////////

void CameraForwardLook(CAMERA *camera)
{
    QUATERNION quat;

    ConstrainQuat2(&camera->Object->body.Centre.Quat, &camera->Quat);
    SLerpQuat(&camera->Quat, &camera->Object->body.Centre.Quat, TimeStep * 10, &quat);
    NormalizeQuat(&quat);

    CopyQuat(&quat, &camera->Quat);
    QuatToMat(&quat, &camera->WMatrix);
}

void CameraRearLook(CAMERA *camera)
{
    QUATERNION quat;

    ConstrainQuat2(&camera->Object->body.Centre.Quat, &camera->Quat);
    SLerpQuat(&camera->Quat, &camera->Object->body.Centre.Quat, TimeStep * 10, &quat);
    NormalizeQuat(&quat);

    CopyQuat(&quat, &camera->Quat);
    QuatToMat(&quat, &camera->WMatrix);
    NegateVec(&camera->WMatrix.mv[L]);
    NegateVec(&camera->WMatrix.mv[R]);

}


/////////////////////////////////////////////////////////////////////
//
// CameraRelativePos: Fix camera at an object-reltive position (in
// the object's frame)
//
/////////////////////////////////////////////////////////////////////

void CameraRelativePos(CAMERA *camera)
{
    VEC dR;
    VEC xyOff;
    VEC xyWorld;

    CopyVec(&camera->WPos, &camera->OldWPos);

    VecMinusVec(&camera->DestOffset, &camera->PosOffset, &dR);
    VecPlusEqScalarVec(&camera->PosOffset, ONE / 8, &dR);

    SetVec(&xyOff, camera->PosOffset.v[X], ZERO, camera->PosOffset.v[Z]);
    VecMulMat(&xyOff, &camera->Object->body.Centre.WMatrix, &xyWorld);
    xyWorld.v[Y] = camera->PosOffset.v[Y];
    VecPlusVec(&xyWorld, &camera->Object->body.Centre.Pos, &camera->WPos);

    if (TimeStep > SMALL_REAL) {
        VecMinusVec(&camera->WPos, &camera->OldWPos, &camera->Vel);
        VecDivScalar(&camera->Vel, TimeStep);
    } else {
        SetVecZero(&camera->Vel);
    }
}

/////////////////////////////////////////////////////////////////////
//
// CameraAwayLook: calculate camera matrix to look from the camera's
// position to the objects centre plus a vector offset
//
/////////////////////////////////////////////////////////////////////

void CameraAwayLook(CAMERA *camera)
{
    REAL dRLen, yComp, scale;
    VEC lookPos, lookOff, dR;

    VecMinusVec(&camera->Object->body.Centre.Pos, &camera->WPos, &dR);
    dRLen = VecLen(&dR);

    // Check line of sight
#ifndef _PSX
    if ((camera->Type == CAM_FOLLOW) && ((camera->SubType == CAM_FOLLOW_BEHIND) || (camera->SubType == CAM_FOLLOW_ROTATE))) {
        if ((dRLen > 100) && (!(camera->LoS = LineOfSight(&camera->CollPos, &camera->Object->body.Centre.Pos)))) {
            camera->LosScale -= CAM_MOVE_SPEED * TimeStep;
            if (camera->LosScale < Real(0.3)) camera->LosScale = Real(0.3);
        } else {
            if (!camera->HasCollidedWithWall) {
                camera->LosScale += CAM_MOVE_SPEED * TimeStep;
                if (camera->LosScale > ONE) camera->LosScale = ONE;
            }
        }
    } else {
        camera->LosScale = ONE;
    }
#else
    camera->LosScale = ONE;
#endif

    // Make sure matrix doesn't bugger up when cam to close to object
    if (dRLen < 50) {
        //CopyMat(&camera->Object->body.Centre.WMatrix, &camera->WMatrix);
        return;
    }

    // scale look offset when close to car
    if (camera->Type == CAM_RAIL) {
        scale = dRLen / (dRLen + 700.0f);
        VecEqScalarVec(&lookOff, scale, &camera->LookOffset);
    } else {
        CopyVec(&camera->LookOffset, &lookOff);
    }

    // Make camera look at car
    yComp = lookOff.v[Y];
    lookOff.v[Y] = ZERO;
    VecMulMat(&lookOff, &camera->Object->body.Centre.WMatrix, &lookPos);
    lookPos.v[Y] = yComp;
    VecPlusEqVec(&lookPos, &camera->Object->body.Centre.Pos);
    BuildLookMatrixForward(&camera->WPos, &lookPos, &camera->WMatrix);


}


////////////////////////////////////////////////////////////////
//
// CameraWatchLook: camera looks at the player pointed to
// by the camera's object...
//
////////////////////////////////////////////////////////////////

void CameraWatchLook(CAMERA *camera)
{
    OBJECT *obj, *target;

    // Find the target to look at
    obj = camera->Object;
    switch(obj->Type) {

    case OBJECT_TYPE_FIREWORK:
        target = ((FIREWORK_OBJ*)obj->Data)->Target;
        break;

    default:
        target = NULL;
        break;
    }

    if (target == NULL) {
        CameraAwayLook(camera);
        return;
    }

    // Look at the target
    BuildLookMatrixForward(&camera->WPos, &target->body.Centre.Pos, &camera->WMatrix);
}


/////////////////////////////////////////////////////////////////////
//
// CameraFollowPos: calculate camera's world position following an
// object
//
/////////////////////////////////////////////////////////////////////
#ifdef _PC
void CameraNewFollowPos(CAMERA *camera)
{
    QUATERNION q;
    VEC     *objPos;
    MAT     *objMat;
    QUATERNION *objQuat;

    objQuat = &camera->Object->body.Centre.Quat;
    objMat = &camera->Object->body.Centre.WMatrix;
    objPos = &camera->Object->body.Centre.Pos;

    CopyVec(&camera->WPos, &camera->OldWPos);

    // Calculate position of camera mount point
    SLerpQuat(&camera->Quat, objQuat, TimeStep * 3, &q);
    CopyQuat(&q, &camera->Quat);
    QuatRotVec(&camera->Quat, &camera->PosOffset, &camera->WorldPosOffset);

    VecPlusVec(&camera->WorldPosOffset, objPos, &camera->WPos);

    CameraWorldColls(camera);

    VecMinusVec(&camera->WPos, objPos, &camera->WorldPosOffset);
    CopyVec(&camera->CollPos, &camera->OldCollPos);

    if (TimeStep > SMALL_REAL) {
        VecMinusVec(&camera->WPos, &camera->OldWPos, &camera->Vel);
        VecDivScalar(&camera->Vel, TimeStep);
    } else {
        SetVecZero(&camera->Vel);
    }
}
#endif

void CameraFollowPos(CAMERA *camera)
{
    VEC     newPole, delta, *objPos, offset;
    MAT     mat, *objMat;
    REAL    poleLen, homeLen, scale;

    objMat = &camera->Object->body.Centre.WMatrix;
    objPos = &camera->Object->body.Centre.Pos;

    CopyVec(&camera->WPos, &camera->OldWPos);

    // get desired camera world matrix
    mat.m[RX] = objMat->m[LZ];
    mat.m[RY] = ZERO;
    mat.m[RZ] = -objMat->m[LX];

    if (!mat.m[RX] && !mat.m[RZ])
        mat.m[RX] = ONE;

    NormalizeVector(&mat.mv[R]);

    mat.m[UX] = ZERO;
    mat.m[UY] = ONE;
    mat.m[UZ] = ZERO;

    CrossProduct(&mat.mv[R], &mat.mv[U], &mat.mv[L]);

    // get desired offset vector
    if (TRUE) {//camera->Flag == CAMERA_FLAG_PRIMARY) {
        VecEqScalarVec(&offset, camera->LosScale, &camera->DestOffset);
        offset.v[Y] *= camera->Object->CamLength;
        scale = (ONE + camera->Object->CamLength) * HALF;
        offset.v[X] *= scale;
        offset.v[Z] *= scale;
    } else {
        CopyVec(&camera->DestOffset, &offset);
    }
    VecMinusVec(&offset, &camera->PosOffset, &newPole);
    /*poleLen = VecLen(&newPole);
    if (poleLen > SMALL_REAL) {
        //VecDivScalar(&newPole, poleLen);
        VecPlusEqScalarVec(&camera->PosOffset, 2 * TimeStep, &newPole);
    } else {
        CopyVec(&camera->DestOffset, &camera->PosOffset);
    }*/
    VecPlusEqScalarVec(&camera->PosOffset, ONE / 4, &newPole);

#if FALSE
    MAT matY;
    MAT mat2;
    REAL angvel = camera->Object->body.AngVel.v[Y];
    static REAL angvelscale = 0.07f;
    angvel *= angvelscale;
    if (angvel > 0.25f) angvel = 0.25f;
    if (angvel < -0.25f) angvel = -0.25f;
    RotationY(&matY, -RAD * angvel);
    MatMulMat(&matY, &mat, &mat2);
#endif

    RotVector(&mat, &camera->PosOffset, &newPole);
    homeLen = Length(&camera->PosOffset);
    if (homeLen > SMALL_REAL) {
        VecDivScalar(&newPole, homeLen);
    }

    // Get length and direction of last offset
    poleLen = VecLen(&camera->WorldPosOffset);
    if (poleLen > SMALL_REAL) {
        VecDivScalar(&camera->WorldPosOffset, poleLen);
    }
 
    // rotate offset towards desired offset
    SubVector(&newPole, &camera->WorldPosOffset, &delta);
    VecPlusScalarVec(&camera->WorldPosOffset, TimeStep / 0.25f, &delta, &newPole);
    if ((abs(newPole.v[X]) > SMALL_REAL) || (abs(newPole.v[Y]) > SMALL_REAL) || (abs(newPole.v[Z]) > SMALL_REAL)) {
        NormalizeVec(&newPole);
    }

    // Calculate new offset length
    poleLen = poleLen + (homeLen - poleLen) * TimeStep / 0.30f;

    // Set the new offset
    VecMulScalar(&newPole, poleLen);

    // Modify desired postion according to camera collision
    VecPlusVec(objPos, &newPole, &camera->WPos);
    CameraWorldColls(camera);


    // copy new to last
    VecMinusVec(&camera->WPos, objPos, &camera->WorldPosOffset);
    CopyVec(&camera->CollPos, &camera->OldCollPos);

    if (TimeStep > SMALL_REAL) {
        VecMinusVec(&camera->WPos, &camera->OldWPos, &camera->Vel);
        VecDivScalar(&camera->Vel, TimeStep);
    } else {
        SetVecZero(&camera->Vel);
    }
}


void CameraRotatePos(CAMERA *camera)
{
    VEC     newPole, delta, *objPos;
    MAT     mat, *objMat;
    REAL    poleLen, homeLen, theta, sinTheta, cosTheta;

    objMat = &camera->Object->body.Centre.WMatrix;
    objPos = &camera->Object->body.Centre.Pos;

    CopyVec(&camera->WPos, &camera->OldWPos);

    // get rotation angle of camera for current frame
    theta = camera->Timer * 2 * PI / 10.0f;
    sinTheta = (REAL)sin(theta);
    cosTheta = (REAL)cos(theta);

    // get desired camera world matrix
    mat.m[RX] = objMat->m[LZ] * cosTheta + objMat->m[LX] * sinTheta;
    mat.m[RY] = ZERO;
    mat.m[RZ] = -objMat->m[LX] * cosTheta + objMat->m[LZ] * sinTheta;

    if (!mat.m[RX] && !mat.m[RZ])
        mat.m[RX] = ONE;

    NormalizeVector(&mat.mv[R]);

    mat.m[UX] = ZERO;
    mat.m[UY] = ONE;
    mat.m[UZ] = ZERO;

    CrossProduct(&mat.mv[R], &mat.mv[U], &mat.mv[L]);

    // get desired offset vector
    VecMinusVec(&camera->DestOffset, &camera->PosOffset, &newPole);
    VecPlusEqScalarVec(&camera->PosOffset, ONE / 8, &newPole);

    RotVector(&mat, &camera->PosOffset, &newPole);
    homeLen = Length(&camera->PosOffset);
    if (homeLen > SMALL_REAL) {
        VecDivScalar(&newPole, homeLen);
    }

    // Get length and direction of last offset
    poleLen = VecLen(&camera->WorldPosOffset);
    if (poleLen > SMALL_REAL) {
        VecDivScalar(&camera->WorldPosOffset, poleLen);
    }
 
    // rotate offset towards desired offset
    SubVector(&newPole, &camera->WorldPosOffset, &delta);
    VecPlusScalarVec(&camera->WorldPosOffset, TimeStep / 0.25f, &delta, &newPole);
    if ((abs(newPole.v[X]) > SMALL_REAL) || (abs(newPole.v[Y]) > SMALL_REAL) || (abs(newPole.v[Z]) > SMALL_REAL)) {
        NormalizeVec(&newPole);
    }

    // Calculate new offset length
    poleLen = poleLen + (homeLen - poleLen) * TimeStep / 0.30f;

    // Set the new offset
    VecMulScalar(&newPole, poleLen);

    // Modify desired postion according to camera collision
    VecPlusVec(objPos, &newPole, &camera->WPos);
    CameraWorldColls(camera);


    // copy new to last
    VecMinusVec(&camera->WPos, objPos, &camera->WorldPosOffset);
    CopyVec(&camera->CollPos, &camera->OldCollPos);

    if (TimeStep > SMALL_REAL) {
        VecMinusVec(&camera->WPos, &camera->OldWPos, &camera->Vel);
        VecDivScalar(&camera->Vel, TimeStep);
    } else {
        SetVecZero(&camera->Vel);
    }
}

/////////////////////////////////////////////////////////////////////
//
// CameraWorldColls:
//
/////////////////////////////////////////////////////////////////////

void CameraWorldColls(CAMERA *camera) 
{
    //bool      los;
    //REAL      shiftDotFace;
    int         iPoly;
    REAL        depth, time, objCamDist, newObjCamDist, radius;
    VEC         shift, relPos, worldPos, objCamVec;
    PLANE       collPlane;
    COLLGRID    *collGrid;
    NEWCOLLPOLY *poly;

    // Make sure this camera can collide
    if (!camera->Collide) return;

    // Initialisation
    SetVecZero(&shift);

    // Calculate object-camera relative unit vector and their separation
    VecMinusVec(&camera->Object->body.Centre.Pos, &camera->WPos, &objCamVec);
    objCamDist = VecLen(&objCamVec);
    if (objCamDist > SMALL_REAL) {
        VecDivScalar(&objCamVec, objCamDist);
    } else {
        SetVecZero(&objCamVec);
    }

    // Calculate the point to check for collisions
    VecPlusScalarVec(&camera->WPos, RenderSettings.NearClip, &objCamVec, &camera->CollPos);

    // Calculate the grid location of the camera
    collGrid = PosToCollGrid(&camera->CollPos);
    if (collGrid == NULL) return;


    if (camera->Object != NULL) {
        radius = InnerRadius * camera->Object->CamLength;
        if (radius < InnerRadius * 0.75f) radius = InnerRadius * 0.75f;
    } else {
        radius = InnerRadius;
    }

    // Check that the camera hasn't passed through any polys
    for (iPoly = 0; iPoly < collGrid->NCollPolys; iPoly++) {
        poly = GetCollPoly(collGrid->CollPolyIndices[iPoly]);

        if (PolyObjectOnly(poly)) continue;
        if (SphereCollPoly(&camera->OldCollPos, &camera->CollPos, radius, poly, &collPlane,  &relPos, &worldPos, &depth, &time)) {

            // move camera out of poly
            ModifyShift(&shift, -depth, PlaneNormal(&collPlane));

            // Set collision flags
            camera->HasCollided = TRUE;
            if ((collPlane.v[Y] > Real(-0.7)) || (worldPos.v[Y] < camera->Object->body.Centre.Pos.v[Y])) {
                camera->HasCollidedWithWall = TRUE;
            }

        }
    }

    // Shift camera out of collision
    VecPlusEqVec(&camera->CollPos, &shift);

    // Calculate new pole length
    VecMinusVec(&camera->Object->body.Centre.Pos, &camera->CollPos, &objCamVec);
    newObjCamDist = VecLen(&objCamVec);
    if (newObjCamDist > SMALL_REAL) {
        VecDivScalar(&objCamVec, newObjCamDist);
        if (newObjCamDist > objCamDist - RenderSettings.NearClip) {
            VecPlusScalarVec(&camera->Object->body.Centre.Pos, -objCamDist + RenderSettings.NearClip, &objCamVec, &camera->CollPos);
        }
    }

    // Get new camera position
    VecPlusScalarVec(&camera->CollPos, - RenderSettings.NearClip, &objCamVec, &camera->WPos);

}


/*void CameraRailPos(CAMERA *camera)
{
    REAL    dRLen, force, velRod, poleLen;
    long    nearNode, link;
    VEC     dR, dVel, nodeCamVel;
    SPRING  camSpring = {60.0f, 14.0f, 0.0f};
    REAL    camMass = 1.0f;

    VEC     *objPos = &camera->Object->body.Centre.Pos;
    MAT     *objMat = &camera->Object->body.Centre.WMatrix;

    CopyVec(&camera->WPos, &camera->OldWPos);

    // Get distance from current camera pos to rail runner
    FindNearestCameraPath(&camera->Object->body.Centre.Pos, &nearNode, &link);
    VecMinusVec(&camera->WPos, &CAM_NodeCamPos, &dR);
    dRLen = VecLen(&dR);

    // Calculate normalised direction vector
    if (dRLen > SMALL_REAL) {
        VecDivScalar(&dR, dRLen);
    } else {
        SetVecZero(&dR);
    }

    // Calculate length of the camera pole and spring stiffness
    poleLen = CAM_NodeCamPoleLen;
    camSpring.Stiffness *= (VecLen(&PLR_LocalPlayer->car.Body->Centre.Vel) / 1000.0f);

    // Calculate velocity of the runner and relative velocity to camera
    VecMinusVec(&CAM_NodeCamPos, &CAM_NodeCamOldPos, &nodeCamVel);
    if (TimeStep > SMALL_REAL) {
        VecMulScalar(&nodeCamVel, TimeStep);
    } else {
        SetVecZero(&nodeCamVel);
    }
    VecMinusVec(&camera->Vel, &nodeCamVel, &dVel);

    // Force due to extension of rod
    velRod = VecDotVec(&dVel, &dR);
    force = SpringDampedForce(&camSpring, dRLen - poleLen, velRod);
    VecPlusEqScalarVec(&camera->Vel, TimeStep * force / camMass, &dR);

    // Force due to vertical deviation
    force = SpringDampedForce(&camSpring, dR.v[Y] * dRLen, camera->Vel.v[Y]);
    VecPlusEqScalarVec(&camera->Vel, TimeStep * force / camMass, &DownVec);

    // Friciton
    VecMulScalar(&camera->Vel, 0.95f);

    // Update position
    VecPlusEqScalarVec(&camera->WPos, TimeStep, &camera->Vel);

    // Do collisions
    if (CAM_NodeCamDoColls) {
        CameraWorldColls(camera);
    }

    // Store old stuff 
    CopyVec(&camera->WPos, &camera->OldCollPos);
    VecMinusVec(&camera->WPos, objPos, &camera->PosOffset);
    VecMinusVec(&camera->WPos, objPos, &camera->OldPosOffset);

    if (TimeStep > SMALL_REAL) {
        VecMinusVec(&camera->WPos, &camera->OldWPos, &camera->Vel);
        VecDivScalar(&camera->Vel, TimeStep);
    } else {
        SetVecZero(&camera->Vel);
    }
}*/

/////////////////////////////////////////////////////////////////////
//
// CameraDynamicNodePos:
//
/////////////////////////////////////////////////////////////////////

void CameraDynamicNodePos(CAMERA *camera)
{
    REAL t;
    CAMNODE *node1, *node2;
    static long invertDirection = FALSE;

    if ((camera->Object->player->ValidRailCamNode == -1))
        //|| ((camera->Object->player->LastValidRailCamNode == -1) && (camera->Timer < MIN_CAM_SWITCH_TIME)))   // and that enough time has passed
    {
        CameraNearestNodePos(camera);
        return;
    }

    node1 = &CAM_CameraNode[camera->Object->player->ValidRailCamNode];
    node2 = &CAM_CameraNode[node1->Link];

    // See if we have just switched to dynamic camera mode
    if (camera->Object->player->ValidRailCamNode != camera->Object->player->LastValidRailCamNode) {

        // Yep, decide whether to switch follow direction
        if (node1->RailType == CAM_RAILTYPE_RANDOM) {
            if (rand() % 2) invertDirection = !invertDirection;
        } else if (node1->RailType == CAM_RAILTYPE_TRACK) {
            invertDirection = FALSE;
        } else {
            invertDirection = TRUE;
        }

        // Make cam look at player
        camera->CalcCamLook = CameraAwayLook;

        // Reset timer
        camera->Timer = ZERO;
    }

    // Housekeeping
    CopyVec(&camera->WPos, &camera->OldWPos);

    t = NearPointOnLine(&node1->Pos, &node2->Pos, &camera->Object->body.Centre.Pos, &camera->WPos);
    if (invertDirection) {
        t = ONE - t;
    } else {
        t += 0.03f;
    }

    if (t < ZERO) {
        t = ZERO;
    } else if (t > ONE) {
        t = ONE;
    }

    ScalarVecPlusScalarVec(t, &node2->Pos, ONE - t, &node1->Pos, &camera->WPos);
    camera->ZoomMod = t * node1->ZoomMod + (ONE - t) * node2->ZoomMod;

    if (TimeStep > SMALL_REAL) {
        VecMinusVec(&camera->WPos, &camera->OldWPos, &camera->Vel);
        VecDivScalar(&camera->Vel, TimeStep);
    } else {
        SetVecZero(&camera->Vel);
    }
}

/////////////////////////////////////////////////////////////////////
//
// CameraStartEndNodes:
//
/////////////////////////////////////////////////////////////////////

bool CameraStartEndNodes(CAMERA *camera)
{

    // Find the nearest node of type MONORAIL
    CAM_EndNode = NearestNode(CAMNODE_MONORAIL, &camera->Object->body.Centre.Pos);
    if (CAM_EndNode == NULL) {
        return FALSE;
    }

    // Node should always be attached to another
    if (CAM_EndNode->Link == -1) {
        return FALSE;
    }

    // Set the start node
    CAM_StartNode = &CAM_CameraNode[CAM_EndNode->Link];

    return TRUE;
}

////////////////////
// freedom camera //
////////////////////

void CameraFreedomPos(CAMERA *camera)
{
    VEC vec, vec2;
    REAL add;
    CopyVec(&camera->WPos, &camera->OldWPos);

    // get pos
    add = 1152 * TimeStep;
//$REMOVED
//#ifdef _PC
//    if (GetKeyState(VK_SCROLL) & 1) add *= 4;
//#endif
    if (CAMERA_RIGHT) vec.v[X] = add;
    else if (CAMERA_LEFT) vec.v[X] = -add;
    else vec.v[X] = 0;

    if (CAMERA_DOWN) vec.v[Y] = add;
    else if (CAMERA_UP) vec.v[Y] = -add;
    else vec.v[Y] = 0;

#ifdef _PC
//$MODIFIED:
//    if (CAMERA_FORWARDS || Mouse.rgbButtons[0]) vec.v[Z] = add;
//    else if (CAMERA_BACKWARDS || Mouse.rgbButtons[1]) vec.v[Z] = -add;
    if(false) {/*nothing*/}  //$CMP_NOTE: should we modify this to do something on Xbox !?!
#endif
    else vec.v[Z] = 0;

    RotVector(&camera->WMatrix, &vec, &vec2);
    AddVector(&camera->WPos, &vec2, &camera->WPos);

    if (TimeStep > SMALL_REAL) {
        VecMinusVec(&camera->WPos, &camera->OldWPos, &camera->Vel);
        VecDivScalar(&camera->Vel, TimeStep);
    } else {
        SetVecZero(&camera->Vel);
    }
}

void CameraMouseLook(CAMERA *camera)
{
    MAT mat, mat2;

//$REMOVED
//#ifdef _PC
//    RotMatrixZYX(&mat, (REAL)-Mouse.lY / 3072, -(REAL)Mouse.lX / 3072, 0);
//#endif
    MulMatrix(&camera->WMatrix, &mat, &mat2);
    CopyMatrix(&mat2, &camera->WMatrix);

    camera->WMatrix.m[RY] = 0;
    NormalizeVector(&camera->WMatrix.mv[X]);
    CrossProduct(&camera->WMatrix.mv[Z], &camera->WMatrix.mv[X], &camera->WMatrix.mv[Y]);
    NormalizeVector(&camera->WMatrix.mv[Y]);
    CrossProduct(&camera->WMatrix.mv[X], &camera->WMatrix.mv[Y], &camera->WMatrix.mv[Z]);
}

//////////////////////
// camera edit mode //
//////////////////////
#ifdef _PC
void CameraEditPos(CAMERA *camera)
{
    REAL add;
    VEC vec, vec2;

// update mouse ptr

//$MODIFIED
//    MouseXrel = (REAL)Mouse.lX / 3;
//    MouseYrel = (REAL)Mouse.lY / 3;
    MouseXrel = 0;
    MouseYrel = 0;
//$END_MODIFICATIONS
    MouseXpos += MouseXrel;
    MouseYpos += MouseYrel;
    if (MouseXpos < 0) MouseXpos = 0, MouseXrel = 0;
    if (MouseXpos > 639) MouseXpos = 639, MouseXrel = 0;
    if (MouseYpos < 0) MouseYpos = 0, MouseYrel = 0;
    if (MouseYpos > 479) MouseYpos = 479, MouseYrel = 0;

// update mouse buttons

    MouseLastLeft = MouseLeft;
    MouseLastRight = MouseRight;

//$MODIFIED
//    MouseLeft = Mouse.rgbButtons[0];
//    MouseRight = Mouse.rgbButtons[1];
    MouseLeft  = 0;
    MouseRight = 0;
//$END_MODIFICATIONS

// slide camera?

    add = 1152 * TimeStep;
//$REMOVED    if (GetKeyState(VK_SCROLL) & 1) add *= 4;

    if (CAMERA_RIGHT) vec.v[X] = add;
    else if (CAMERA_LEFT) vec.v[X] = -add;
    else vec.v[X] = 0;

    if (CAMERA_DOWN) vec.v[Y] = add;
    else if (CAMERA_UP) vec.v[Y] = -add;
    else vec.v[Y] = 0;

    if (CAMERA_FORWARDS) vec.v[Z] = add;
    else if (CAMERA_BACKWARDS) vec.v[Z] = -add;
    else vec.v[Z] = 0;

    CameraEditXrel = vec.v[X];
    CameraEditYrel = vec.v[Y];
    CameraEditZrel = vec.v[Z];

    RotVector(&camera->WMatrix, &vec, &vec2);
    AddVector(&camera->WPos, &vec2, &camera->WPos);
}
#endif

void CameraNullLook(CAMERA *camera)
{
}

////////////////////////////////////
// set camera view matrix / trans //
////////////////////////////////////

void SetCameraView(MAT *cammat, VEC *campos, REAL shake)
{
    VEC vec, tl, tr, bl, br;
    MAT mat, mat2;

// save camera pos + matrix

    CopyVec(campos, &ViewCameraPos);
    CopyMatrix(cammat, &ViewCameraMatrix);

// build scaled eye matrix / trans

    if (shake)
    {
        RotMatrixZYX(&mat2, (frand(0.01f) - 0.005f) * shake, (frand(0.01f) - 0.005f) * shake, (frand(0.01f) - 0.005f) * shake);
        MulMatrix(cammat, &mat2, &mat);
    }
    else
    {
        CopyMatrix(cammat, &mat);
    }

    mat.m[RX] *= RenderSettings.MatScaleX;
    mat.m[RY] *= RenderSettings.MatScaleX;
    mat.m[RZ] *= RenderSettings.MatScaleX;
    mat.m[UX] *= RenderSettings.MatScaleY;
    mat.m[UY] *= RenderSettings.MatScaleY;
    mat.m[UZ] *= RenderSettings.MatScaleY;

    if (GameSettings.Mirrored)
    {
        mat.m[RX] = -mat.m[RX];
        mat.m[RY] = -mat.m[RY];
        mat.m[RZ] = -mat.m[RZ];
    }

    TransposeMatrix(&mat, &ViewMatrixScaled);

    SetVector(&vec, -campos->v[X], -campos->v[Y], -campos->v[Z]);
    RotVector(&ViewMatrixScaled, &vec, &ViewTransScaled);

// build unscaled eye matrix / trans

    TransposeMatrix(cammat, &ViewMatrix);
    RotVector(&ViewMatrix, &vec, &ViewTrans);

// build camera view frustum planes

#ifdef _PC
    SetVector(&vec, -REAL_SCREEN_XHALF, -REAL_SCREEN_YHALF, RenderSettings.GeomPers);
    RotVector(cammat, &vec, &tl);
    AddVector(&tl, campos, &tl);
    SetVector(&vec, REAL_SCREEN_XHALF, -REAL_SCREEN_YHALF, RenderSettings.GeomPers);
    RotVector(cammat, &vec, &tr);
    AddVector(&tr, campos, &tr);
    SetVector(&vec, -REAL_SCREEN_XHALF, REAL_SCREEN_YHALF, RenderSettings.GeomPers);
    RotVector(cammat, &vec, &bl);
    AddVector(&bl, campos, &bl);
    SetVector(&vec, REAL_SCREEN_XHALF, REAL_SCREEN_YHALF, RenderSettings.GeomPers);
    RotVector(cammat, &vec, &br);
    AddVector(&br, campos, &br);
#endif

    BuildPlane(campos, &tl, &bl, &CameraPlaneLeft);
    BuildPlane(campos, &tr, &tl, &CameraPlaneTop);
    BuildPlane(campos, &br, &tr, &CameraPlaneRight);
    BuildPlane(campos, &bl, &br, &CameraPlaneBottom);

// build mirrored scaled eye matrix

    CopyMatrix(&ViewMatrixScaled, &ViewMatrixScaledMirrorY);
    ViewMatrixScaledMirrorY.m[UX] = -ViewMatrixScaledMirrorY.m[UX];
    ViewMatrixScaledMirrorY.m[UY] = -ViewMatrixScaledMirrorY.m[UY];
    ViewMatrixScaledMirrorY.m[UZ] = -ViewMatrixScaledMirrorY.m[UZ];


//$BEGIN_ADDITION(jedl)
    // Set projection matrix for GPU pipeline

    // Here's what we're emulating with matrices
    /*
    float x, y, z;      // source
    float sx, sy, sz, rhw;  // destination
    float sw;   // temp
    sw = x * ViewMatrixScaled.m[RZ] + y * ViewMatrixScaled.m[UZ] + z * ViewMatrixScaled.m[LZ] + ViewTransScaled.v[Z];
    if (sw < 1) sw = 1; // What's this?  Clip to near plane?
    sx = (x * ViewMatrixScaled.m[RX] + y * ViewMatrixScaled.m[UX] + z * ViewMatrixScaled.m[LX] + ViewTransScaled.v[X]) / sw + RenderSettings.GeomCentreX;
    sy = (x * ViewMatrixScaled.m[RY] + y * ViewMatrixScaled.m[UY] + z * ViewMatrixScaled.m[LY] + ViewTransScaled.v[Y]) / sw + RenderSettings.GeomCentreY;
    sz = (RenderSettings.ZedFarDivDist - (RenderSettings.ZedFarMulNear / (RenderSettings.ZedDrawDist * (sw))))
    rhw = 1 / sw;
    */
    
    // $TODO: Get rid of this, or move to the right place.  The communication
    //        between the app and the effects is not yet worked out.
    extern XGMATRIX g_Proj;
    extern XGVECTOR4 g_vScale;
    extern XGVECTOR4 g_vOffset;

    // $TODO There's still mismatched scaling on the z-values between the GPU and the original Revolt CPU
    // pipelines. The scaling of the resulting Z values.
    
    // Rewrite sz = (RenderSettings.ZedFarDivDist * sw - RenderSettings.ZedFarMulNear / RenderSettings.ZedDrawDist ) / sw
    g_vScale = XGVECTOR4(1.f, 1.f, 1.f, 1.f);
    g_vOffset = XGVECTOR4(RenderSettings.GeomCentreX, RenderSettings.GeomCentreY, 0.f, 0.f);
        
    g_Proj._11 = ViewMatrixScaled.m[RX];
    g_Proj._12 = ViewMatrixScaled.m[UX];
    g_Proj._13 = ViewMatrixScaled.m[LX];
    g_Proj._14 = ViewTransScaled.v[X];
    
    g_Proj._21 = ViewMatrixScaled.m[RY];
    g_Proj._22 = ViewMatrixScaled.m[UY];
    g_Proj._23 = ViewMatrixScaled.m[LY];
    g_Proj._24 = ViewTransScaled.v[Y];
    
    FLOAT fZScale = (FLOAT)D3DZ_MAX_D24S8 * RenderSettings.ZedFarDivDist; 
    g_Proj._31 = fZScale * ViewMatrixScaled.m[RZ];
    g_Proj._32 = fZScale * ViewMatrixScaled.m[UZ];
    g_Proj._33 = fZScale * ViewMatrixScaled.m[LZ];
    g_Proj._34 = fZScale * (ViewTransScaled.v[Z] - RenderSettings.ZedFarMulNear / RenderSettings.ZedDrawDist);

    g_Proj._41 = ViewMatrixScaled.m[RZ];
    g_Proj._42 = ViewMatrixScaled.m[UZ];
    g_Proj._43 = ViewMatrixScaled.m[LZ];
    g_Proj._44 = ViewTransScaled.v[Z];
//$END_ADDITION
}

#ifdef _PC
///////////////////////
// set viewport vars //
///////////////////////

void SetViewport(REAL x, REAL y, REAL xsize, REAL ysize, REAL pers)
{
//$REMOVED    HRESULT r;
    D3DVIEWPORT2 vd;

// set geom vars

    RenderSettings.GeomPers = pers;
    RenderSettings.GeomCentreX = x + xsize / 2;
    RenderSettings.GeomCentreY = y + ysize / 2;
    RenderSettings.GeomScaleX = xsize / REAL_SCREEN_XSIZE;
    RenderSettings.GeomScaleY = ysize / REAL_SCREEN_YSIZE;
    RenderSettings.MatScaleX = RenderSettings.GeomScaleX * RenderSettings.GeomPers;
    RenderSettings.MatScaleY = RenderSettings.GeomScaleY * RenderSettings.GeomPers;

// set clip vars

    ScreenLeftClip = ScreenLeftClipGuard = x;
    ScreenRightClip = ScreenRightClipGuard = x + xsize - 1;
    ScreenTopClip = ScreenTopClipGuard = y;
    ScreenBottomClip = ScreenBottomClipGuard = y + ysize - 1;

//  if (ScreenLeftClipGuard == 0 && D3Dcaps.dvGuardBandLeft != 0) ScreenLeftClipGuard = D3Dcaps.dvGuardBandLeft;
//  if (ScreenRightClipGuard == (REAL)(ScreenXsize - 1) && D3Dcaps.dvGuardBandRight != 0) ScreenRightClipGuard = D3Dcaps.dvGuardBandRight;
//  if (ScreenTopClipGuard == 0 && D3Dcaps.dvGuardBandTop != 0) ScreenTopClipGuard = D3Dcaps.dvGuardBandTop;
//  if (ScreenBottomClipGuard == (REAL)(ScreenYsize - 1) && D3Dcaps.dvGuardBandBottom != 0) ScreenBottomClipGuard = D3Dcaps.dvGuardBandBottom;

// set viewport-clear rect

    FTOL(x, ViewportRect.x1);
    FTOL(y, ViewportRect.y1);
    FTOL(x + xsize, ViewportRect.x2);
    FTOL(y + ysize, ViewportRect.y2);

// set dx viewport

//$MODIFIED
//    ZeroMemory(&vd, sizeof(vd));
//    vd.dwSize = sizeof(vd);
//
//    FTOL(x, vd.dwX);
//    FTOL(y, vd.dwY);
//    FTOL(xsize, vd.dwWidth);
//    FTOL(ysize, vd.dwHeight);
//
//    vd.dvClipX = x;
//    vd.dvClipY = y;
//    vd.dvClipWidth = xsize;
//    vd.dvClipHeight = ysize;
//    vd.dvMinZ = 0;
//    vd.dvMaxZ = 1;
    FTOL(x, vd.X);
    FTOL(y, vd.Y);
    FTOL(xsize, vd.Width);
    FTOL(ysize, vd.Height);
    vd.MinZ = 0.0f;
    vd.MaxZ = 1.0f;
//$END_MODIFICATIONS

//$MODIFIED
//    r = D3Dviewport->SetViewport2(&vd);
//    if (r != DD_OK)
//    {
//        ErrorDX(r, "Can't set viewport");
//        g_bQuitGame = TRUE;
//    }
    D3DDevice_SetViewport( &vd );
//$END_MODIFICATIONS
}
#endif

///////////////////////////////////
// add a camera to 'active' list //
///////////////////////////////////

CAMERA *AddCamera(REAL x, REAL y, REAL xsize, REAL ysize, long flag)
{
    long i;
    REAL scalex, scaley;

// find a free camera

    for (i = 0 ; i < MAX_CAMERAS ; i++) if (Camera[i].Flag == CAMERA_FLAG_FREE)
    {

// set info

#ifdef _PC
        scalex = (REAL)ScreenXsize / REAL_SCREEN_XSIZE;
        scaley = (REAL)ScreenYsize / REAL_SCREEN_YSIZE;
        Camera[i].X = x * scalex;
        Camera[i].Y = y * scaley;
        if (!xsize) Camera[i].Xsize = (REAL)ScreenXsize;
        else Camera[i].Xsize = xsize * scalex;
        if (!ysize) Camera[i].Ysize = (REAL)ScreenYsize;
        else Camera[i].Ysize = ysize * scaley;
#endif
        Camera[i].Flag = flag;

        Camera[i].Shake = 0.0f;
        Camera[i].LoS = TRUE;
        Camera[i].LosScale = ONE;
        Camera[i].SpecialFog = FALSE;

        SetVec(&Camera[i].PosOffset, ZERO, Real(-44), ZERO);
        SetVecZero(&Camera[i].WorldPosOffset);
        SetVecZero(&Camera[i].Vel);
        
// return this camera

        return &Camera[i];
    }

// return null

    return NULL;
}

////////////////////////////////////////
// remove a camera from 'active' list //
////////////////////////////////////////

void RemoveCamera(CAMERA *camera)
{
    camera->Flag = CAMERA_FLAG_FREE;
}

//////////////////
// init cameras //
//////////////////

void InitCameras(void)
{
    long i;

    for (i = 0 ; i < MAX_CAMERAS ; i++)
    {
        Camera[i].Type = -1;
        Camera[i].SubType = -1;
        Camera[i].Flag = CAMERA_FLAG_FREE;
        SetVecZero(&Camera[i].WPos);
        SetMatUnit(&Camera[i].WMatrix);
        Camera[i].Lens = ZERO;
        Camera[i].Zoom = FALSE;
        Camera[i].ZoomMod = ZERO;
        Camera[i].LoS = TRUE;
        Camera[i].LosScale = ONE;

        SetVecZero(&Camera[i].PosOffset);
        SetVecZero(&Camera[i].WorldPosOffset);
        SetVecZero(&Camera[i].Vel);

        Camera[i].CalcCamPos = NULL;
        Camera[i].CalcCamLook = NULL;
    }

    // Reset special camera pointers
    CAM_WeaponCamera = NULL;
    CAM_RearCamera = NULL;
    CAM_OtherCamera = NULL;
#ifndef _N64
    CAM_MainCamera = NULL;
#else
    for (i = 0; i < MAX_LOCAL_PLAYERS; i++)
    {
        CAM_PlayerCameras[0] = NULL;
    }
#endif
}

/////////////////////
// set proj matrix //
/////////////////////
#ifdef _PC
void SetProjMatrix(REAL n, REAL f, REAL fov)
{
    REAL c, s, q;
    D3DMATRIX mat;

    fov = fov * PI / 180;

    c = (REAL)cos(fov * 0.5f);
    s = (REAL)sin(fov * 0.5f);
    q = s / (1.0f - n / f);

    mat._11 = c;
    mat._12 = 0;
    mat._13 = 0;
    mat._14 = 0;

    mat._21 = 0;
    mat._22 = -c;
    mat._23 = 0;
    mat._24 = 0;

    mat._31 = 0;
    mat._32 = 0;
    mat._33 = q;
    mat._34 = s;

    mat._41 = 0;
    mat._42 = 0;
    mat._43 = -q * n;
    mat._44 = 0;

//$MODIFIED
//    D3Ddevice->SetTransform(D3DTRANSFORMSTATE_PROJECTION, &mat);
    D3Ddevice->SetTransform(D3DTS_PROJECTION, &mat);
//$END_MODIFICATIONS
} 
#endif

/////////////////////////////////////////////////////////////////////
//
// LoadCameraNodes: 
//
/////////////////////////////////////////////////////////////////////

#ifdef _PC
long LoadCameraNodes(FILE *fp)
{
    long            iNode, nNodes;
    size_t          nRead;
    CAMNODE         *cameraNode;
    FILE_CAM_NODE   fnode;
    BOUNDING_BOX    bbox;
    Assert(fp != NULL);

    // Read in the number of nodes
    nRead = fread(&nNodes, sizeof(long), 1, fp);
    if (nRead < 1) {
        return -1;
    }

    Assert((nNodes >= 0) && (nNodes < CAMERA_MAX_NODES));

    // Load in the nodes
    for (iNode = 0; iNode < nNodes; iNode++) {
        cameraNode = &CAM_CameraNode[iNode];

        nRead = fread(&fnode, sizeof(fnode), 1, fp);
        if (nRead < 1) {
            return -1;
        }

        cameraNode->Type = fnode.Type;
        cameraNode->Pos.v[X] = fnode.x / 65536.0f;
        cameraNode->Pos.v[Y] = fnode.y / 65536.0f;
        cameraNode->Pos.v[Z] = fnode.z / 65536.0f;
        cameraNode->ZoomMod = MulScalar(Real(0.001), fnode.ZoomFactor);
        cameraNode->Link = fnode.Link;
        cameraNode->ID = fnode.ID;
        cameraNode->RailType = fnode.RailType;

        // visimask
        bbox.Xmin = bbox.Xmax = cameraNode->Pos.v[X];
        bbox.Ymin = bbox.Ymax = cameraNode->Pos.v[Y];
        bbox.Zmin = bbox.Zmax = cameraNode->Pos.v[Z];
        cameraNode->VisiMask = SetObjectVisiMask(&bbox);
    }

    return nNodes;
}
#endif


/////////////////////////////////////////////////////////////////////
//
// FindNearestCameraPath:
//
/////////////////////////////////////////////////////////////////////

/*CAMNODE *FindNearestCameraPath(VEC *pos, long *nodeNum, long *linkNum) 
{
    long    iNode, iLink, nearNode, nearLink;
    REAL    dist, minNodeDist, minLineDist, t, nearTime;
    VEC nearPos, linePos, dRLine, dRNode, lookAtPos;
    CAMNODE *camNode1, *camNode2;

    minNodeDist = 1.0e10;
    nearNode = 0;
    minLineDist = 1.0e10;
    nearLink = 0;
    nearTime = ZERO;

    CopyVec(&PLR_LocalPlayer->car.Body->Centre.Pos, &lookAtPos);
    lookAtPos.v[Y] += CameraHomeHeight;

    // Find the camera path line (node and link) which is nearest the car
    for (iNode = 0; iNode < CAM_NCameraNodes; iNode++) {
        camNode1 = &CAM_CameraNode[iNode];

        // Make sure this is a monorail node
        if (camNode1->Type != CAMNODE_MONORAIL) continue;

        // See if this node is nearest
        VecMinusVec(&camNode1->Pos, &lookAtPos, &dRNode);
        dist = VecDotVec(&dRNode, &dRNode);
        if (dRNode.v[Y] > ZERO) dist *= Real(2.0);      // Adjustment to favour nodes above
        if (dist < minNodeDist && dist < minLineDist) {
            minNodeDist = dist;
            nearNode = iNode;
            nearLink = -1;
            nearTime = ZERO;
            CopyVec(&camNode1->Pos, &nearPos);
        }

        // Find out if there is a point on any of the node paths nearer to the camera
        for (iLink = 0; iLink < 4; iLink++) {
            if (camNode1->Link[iLink] == -1) break;
            camNode2 = &CAM_CameraNode[camNode1->Link[iLink]];

            // Make sure this is a monorail node
            if (camNode2->Type != CAMNODE_MONORAIL) continue;

            t = NearPointOnLine(&camNode1->Pos, &camNode2->Pos, &lookAtPos, &linePos);
            if (t < ZERO || t > ONE) continue;
            VecMinusVec(&linePos, &lookAtPos, &dRLine);

            dist = VecDotVec(&dRLine, &dRLine);
            if (dRLine.v[Y] > ZERO) dist *= Real(2.0);      // Adjustment to favour nodes above
            if (dist < minLineDist && dist < minNodeDist) {
                minLineDist = dist;
                nearNode = iNode;
                nearLink = iLink;
                nearTime = t;
                CopyVec(&linePos, &nearPos);
            }

        }
    }

    CopyVec(&CAM_NodeCamPos, &CAM_NodeCamOldPos);
    CopyVec(&nearPos, &CAM_NodeCamPos);
    if (nearLink != -1) {
        VecMinusVec(&CAM_CameraNode[CAM_CameraNode[nearNode].Link[nearLink]].Pos, &CAM_CameraNode[nearNode].Pos, &CAM_NodeCamDir);
        NormalizeVec(&CAM_NodeCamDir);
        CAM_NodeCamPoleLen = (ONE - nearTime) * CAM_CameraNode[nearNode].PoleLen + nearTime * CAM_CameraNode[CAM_CameraNode[nearNode].Link[nearLink]].PoleLen;
    } else {
        CAM_NodeCamPoleLen = CAM_CameraNode[nearNode].PoleLen;
    }


    *nodeNum = nearNode;
    *linkNum = nearLink;
    return &CAM_CameraNode[nearNode];

}*/

/////////////////////////////////////////////////////////////////////
//
// CameraTrigger: trigger function to change camera mode when in 
// trigger box
//
/////////////////////////////////////////////////////////////////////

void TriggerCamera(PLAYER *player, long flag, long n, PLANE *planes)
{
    long iNode;
    CAR *car;

    car = &player->car;

    if ((player->LastValidRailCamNode != -1) && 
        ((CAM_CameraNode[player->LastValidRailCamNode].ID == n) || 
         (CAM_CameraNode[CAM_CameraNode[player->LastValidRailCamNode].Link].ID == n))) 
    {
        // Still in same box
        player->ValidRailCamNode = player->LastValidRailCamNode;
        return;
    }
    
    // New camera node required
    for (iNode = 0; iNode < CAM_NCameraNodes; iNode++) {
        if (CAM_CameraNode[iNode].ID == n) {
            player->ValidRailCamNode = iNode;
            return;
        }
    }

}

void TriggerCameraShorten(PLAYER *player, long flag, long n, PLANE *planes)
{
    player->ownobj->CamLength = (n + 1) / Real(8);
}

/////////////////////////////////////////////////////////////////////
//
// CalcCamZoom:
//
/////////////////////////////////////////////////////////////////////
float MAX_LENS = 62.f;
float SCALE_LENS = 1.f;

void CalcCamZoom(CAMERA *camera)
{
    VEC dR;
    REAL dRLen;

    VecMinusVec(&camera->WPos, &camera->Object->body.Centre.Pos, &dR);
    dRLen = VecLen(&dR);

    camera->Lens = camera->ZoomMod * (dRLen - BaseGeomPers);
    if (camera->Lens < MIN_LENS) camera->Lens = MIN_LENS;
}


/////////////////////////////////////////////////////////////////////
//
// NearestNode: find the nearest node of the specified type
//
/////////////////////////////////////////////////////////////////////

CAMNODE *NearestNode(long type, VEC *pos)
{
    int iNode;
    REAL dist, nearDist;
    VEC dR;
    CAMNODE *node, *nearNode;
    VISIMASK tempmask;

    nearNode = NULL;
    nearDist = LARGEDIST;

    tempmask = CamVisiMask;
    SetCameraVisiMask(pos);

    for (iNode = 0; iNode < CAM_NCameraNodes; iNode++) {
        node = &CAM_CameraNode[iNode];

        // good visimask?
        if (CamVisiMask & node->VisiMask) continue;

        // is this node of the desired type?
        if ((type != -1) && (node->Type != type)) continue;

        // Get distance to node from pos
        VecMinusVec(pos, &node->Pos, &dR);
        dist = VecLen(&dR);

        // see if it is the closest so far
        if (dist < nearDist) {
            nearDist = dist;
            nearNode = node;
        }
    }

    CamVisiMask = tempmask;

    return nearNode;
}


/////////////////////////////////////////////////////////////////////
//
// CameraNearestNodePos:
//
/////////////////////////////////////////////////////////////////////

void CameraNearestNodePos(CAMERA *camera)
{
    static CAMNODE *node = NULL;
    CAMNODE *lastNode;


    // Make sure the camera is attached to an object
    if (camera->Object == NULL) {
        return;
    }

    // Don't switch too often
    if (camera->Timer < MIN_CAM_SWITCH_TIME) return;

    // find the nearest node
    lastNode = node;
    node = NearestNode(CAMNODE_STATIC, &camera->Object->body.Centre.Pos);
    
    // Reset timer when camera changes
    if (lastNode != node) {
        camera->Timer = ZERO;
    }
    
    if (node == NULL) {
        return;
    }

    // Put camera at node position
    CopyVec(&node->Pos, &camera->WPos);
    CopyVec(&node->Pos, &camera->OldWPos);
    SetVecZero(&camera->Vel);

    // Set Camera's look
    if (node->Link < 0) {
        camera->CalcCamLook = CameraAwayLook;
    } else {
        camera->CalcCamLook = NULL;
        BuildLookMatrixForward(&camera->WPos, &CAM_CameraNode[node->Link].Pos, &camera->WMatrix);
    }


    // Set zoom factor
    camera->ZoomMod = node->ZoomMod;

}


/////////////////////////////////////////////////////////////////////
//
// CalcCameraCollPoly:
//
/////////////////////////////////////////////////////////////////////
#ifdef _PC
void CalcCameraCollPoly(CAMERA *camera)
{
    VEC screenPos, cPos;

    // calculate the mid-point of the camera plane
    VecPlusScalarVec(&camera->WPos, RenderSettings.NearClip + 20, &camera->WMatrix.mv[L], &screenPos);

    // Calculate the face plane
    SetPlane(&camera->CollPoly.Plane, 
        camera->WMatrix.m[LX],
        camera->WMatrix.m[LY],
        camera->WMatrix.m[LZ],
        -VecDotVec(&screenPos, &camera->WMatrix.mv[L]));

    // Calculate the edge plane
    VecPlusScalarVec(&screenPos, -REAL_SCREEN_XHALF, &camera->WMatrix.mv[R], &cPos);
    SetPlane(&camera->CollPoly.EdgePlane[0], 
        -camera->WMatrix.m[RX],
        -camera->WMatrix.m[RY],
        -camera->WMatrix.m[RZ],
        VecDotVec(&cPos, &camera->WMatrix.mv[R]));

    VecPlusScalarVec(&screenPos, -REAL_SCREEN_YHALF, &camera->WMatrix.mv[U], &cPos);
    SetPlane(&camera->CollPoly.EdgePlane[1], 
        -camera->WMatrix.m[UX],
        -camera->WMatrix.m[UY],
        -camera->WMatrix.m[UZ],
        VecDotVec(&cPos, &camera->WMatrix.mv[U]));

    VecPlusScalarVec(&screenPos, REAL_SCREEN_XHALF, &camera->WMatrix.mv[R], &cPos);
    SetPlane(&camera->CollPoly.EdgePlane[2], 
        camera->WMatrix.m[RX],
        camera->WMatrix.m[RY],
        camera->WMatrix.m[RZ],
        -VecDotVec(&cPos, &camera->WMatrix.mv[R]));

    VecPlusScalarVec(&screenPos, REAL_SCREEN_YHALF, &camera->WMatrix.mv[U], &cPos);
    SetPlane(&camera->CollPoly.EdgePlane[3], 
        camera->WMatrix.m[UX],
        camera->WMatrix.m[UY],
        camera->WMatrix.m[UZ],
        -VecDotVec(&cPos, &camera->WMatrix.mv[U]));

    // Set other stuff
    camera->CollPoly.Type = QUAD;
    camera->CollPoly.Material = MATERIAL_GLASS;

    SetBBox(&camera->CollPoly.BBox, 
        camera->WPos.v[X] - REAL_SCREEN_XHALF,
        camera->WPos.v[X] + REAL_SCREEN_XHALF,
        camera->WPos.v[Y] - REAL_SCREEN_XHALF,
        camera->WPos.v[Y] + REAL_SCREEN_XHALF,
        camera->WPos.v[Z] - REAL_SCREEN_XHALF,
        camera->WPos.v[Z] + REAL_SCREEN_XHALF);

}
#endif


////////////////////////////////////////////////////////////////
//
// Set Random Camera Type: for replays
//
////////////////////////////////////////////////////////////////

int CameraProbabilities[] = {10,20, 30,40, 50, 90, 100};



void SetRandomCameraType(CAMERA *camera, OBJECT *object)
{
    int val = rand() % 100;

    if (val < CameraProbabilities[0]) 
    {
        SetCameraFollow(camera, object, CAM_FOLLOW_BEHIND);
    }
    else if (val < CameraProbabilities[1]) 
    {
        SetCameraFollow(camera, object, CAM_FOLLOW_CLOSE);
    } 
    else if (val < CameraProbabilities[2]) 
    {
        SetCameraFollow(camera, object, CAM_FOLLOW_LEFT);
    } 
    else if (val < CameraProbabilities[3]) 
    {
        SetCameraFollow(camera, object, CAM_FOLLOW_RIGHT);
    }
    else if (val < CameraProbabilities[4]) 
    {
        SetCameraFollow(camera, object, CAM_FOLLOW_FRONT);
    }
    else if (val < CameraProbabilities[5]) 
    {
        SetCameraRail(camera, object, CAM_RAIL_DYNAMIC_MONO);
    }
    else if (val < CameraProbabilities[6]) 
    {
        SetCameraAttached(camera, object, CAM_ATTACHED_INCAR);
    }
}


