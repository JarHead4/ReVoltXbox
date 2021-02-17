//-----------------------------------------------------------------------------
// File: path.cpp
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"
#include "path.h"
#include "geom.h"
#include "object.h"
#include "player.h"
#include "timing.h"
#include "obj_init.h"
#include "camera.h"
#include "drawobj.h"
#include "text.h"

#define PRE_TRANSFORM_PATH TRUE

void SetObjectAnimation(OBJECT *obj, ANIMATION_DATA *anim);
ANIMATION_DATA *CreateAnimation(long nFrames, long nPosKeys, long nQuatKeys, long nScaleKeys);
void DestroyAnimationData(ANIMATION_DATA *data);
ANIMATION_DATA *LoadAnimationData(FILE *fp);
void MoveAndScaleAnimationPath(ANIMATION_DATA *animData, VEC *shift, REAL scale);
void DrawAnimationPath(ANIMATION_DATA *animData);
void TransformAnimation(ANIMATION_DATA *animData);

void FollowPath(OBJECT *obj);



long currentPosKey = 0;
long currentQuatKey = 0;
long currentScaleKey = 0;

////////////////////////////////////////////////////////////////
//
// CreatePathData:
//
////////////////////////////////////////////////////////////////

ANIMATION_DATA *CreateAnimation(long nFrames, long nPosKeys, long nQuatKeys, long nScaleKeys)
{
    ANIMATION_DATA *data;

    // Allocate data store
    data = (ANIMATION_DATA*)malloc(sizeof(ANIMATION_DATA));
    if (data == NULL) return NULL;

    // Set key fame numbers
    data->cFrames = nFrames;
    data->cPosKeys = nPosKeys;
    data->cQuatKeys = nQuatKeys;
    data->cScaleKeys = nScaleKeys;

    data->pPosKeyHead = NULL;
    data->pQuatKeyHead = NULL;
    data->pScaleKeyHead = NULL;

    // Alocate pos key
    if (data->cPosKeys > 0) {
        data->pPosKeyHead = (KEY_POS*)malloc(sizeof(KEY_POS) * data->cPosKeys);
        if (data->pPosKeyHead == NULL) {                // Failed
            free(data);
            return NULL;
        }
    }

    // Alocate Quat key
    if (data->cQuatKeys > 0) {
        data->pQuatKeyHead = (KEY_QUAT*)malloc(sizeof(KEY_QUAT) * data->cQuatKeys);
        if (data->pQuatKeyHead == NULL) {               // Failed
            free(data->pPosKeyHead);
            free(data);
            return NULL;
        }
    }

    // Alocate Scale key
    if (data->cScaleKeys > 0) {
        data->pScaleKeyHead = (KEY_SCALE*)malloc(sizeof(KEY_SCALE) * data->cScaleKeys);
        if (data->pScaleKeyHead == NULL) {              // Failed?
            free(data->pQuatKeyHead);
            free(data->pPosKeyHead);
            free(data);
            return NULL;
        }
    }

    return data;

}

////////////////////////////////////////////////////////////////
//
// DestroyPathData:
//
////////////////////////////////////////////////////////////////

void DestroyAnimationData(ANIMATION_DATA *data) 
{
    free(data->pScaleKeyHead);
    free(data->pQuatKeyHead);
    free(data->pPosKeyHead);
    free(data);
}


////////////////////////////////////////////////////////////////
//
// LoadAnimationData
//
////////////////////////////////////////////////////////////////

ANIMATION_DATA *LoadAnimationData(FILE *fp)
{
    int nRead;
    long size, count;
    char string[5];
    ANIMATION_DATA *animData = NULL;

    // Make sure there is a header and read it
    nRead = fread(string, sizeof(char), 4, fp);
    if ((nRead != 4) || strncmp(string, "ANIM", 4)) {
        return NULL;
    }
    nRead = fread(&size, sizeof(long), 1, fp);
    if (nRead != 1) {
        return NULL;
    }
    nRead = fread(&count, sizeof(long), 1, fp);
    if (nRead != 1) {
        return NULL;
    }

    animData = (ANIMATION_DATA*)malloc(sizeof(ANIMATION_DATA));
    if (animData == NULL) {
        return NULL;
    }
    nRead = fread(animData, sizeof(ANIMATION_DATA), 1, fp);
    if (nRead != 1) {
        free(animData);
        return NULL;
    }

    // Load position keys
    nRead = fread(string, sizeof(char), 4, fp);
    if ((nRead != 4) || strncmp(string, "KPOS", 4)) {
        free(animData);
        return NULL;
    }
    nRead = fread(&size, sizeof(long), 1, fp);
    if (nRead != 1) {
        free(animData);
        return NULL;
    }
    nRead = fread(&count, sizeof(long), 1, fp);
    if (nRead != 1) {
        free(animData);
        return NULL;
    }
    animData->cPosKeys = count;
    animData->pPosKeyHead = (KEY_POS*)malloc(sizeof(KEY_POS) * animData->cPosKeys);
    if (animData->pPosKeyHead == NULL) {
        free(animData);
        return NULL;
    }
    nRead = fread(animData->pPosKeyHead, sizeof(KEY_POS), animData->cPosKeys, fp);
    if (nRead != animData->cPosKeys) {
        free(animData->pPosKeyHead);
        free(animData);
        return NULL;
    }

    // Load quaternion keys
    nRead = fread(string, sizeof(char), 4, fp);
    if ((nRead != 4) || strncmp(string, "KROT", 4)) {
        free(animData->pPosKeyHead);
        free(animData);
        return NULL;
    }
    nRead = fread(&size, sizeof(long), 1, fp);
    if (nRead != 1) {
        free(animData->pPosKeyHead);
        free(animData);
        return NULL;
    }
    nRead = fread(&count, sizeof(long), 1, fp);
    if (nRead != 1) {
        free(animData->pPosKeyHead);
        free(animData);
        return NULL;
    }
    animData->cQuatKeys = count;
    animData->pQuatKeyHead = (KEY_QUAT*)malloc(sizeof(KEY_QUAT) * animData->cQuatKeys);
    if (animData->pQuatKeyHead == NULL) {
        free(animData->pPosKeyHead);
        free(animData);
        return NULL;
    }
    nRead = fread(animData->pQuatKeyHead, sizeof(KEY_QUAT), animData->cQuatKeys, fp);
    if (nRead != animData->cQuatKeys) {
        free(animData->pQuatKeyHead);
        free(animData->pPosKeyHead);
        free(animData);
        return NULL;
    }

    // Load Scale keys
    nRead = fread(string, sizeof(char), 4, fp);
    if ((nRead != 4) || strncmp(string, "KSCL", 4)) {
        free(animData->pQuatKeyHead);
        free(animData->pPosKeyHead);
        free(animData);
        return NULL;
    }
    nRead = fread(&size, sizeof(long), 1, fp);
    if (nRead != 1) {
        free(animData->pQuatKeyHead);
        free(animData->pPosKeyHead);
        free(animData);
        return NULL;
    }
    nRead = fread(&count, sizeof(long), 1, fp);
    if (nRead != 1) {
        free(animData->pQuatKeyHead);
        free(animData->pPosKeyHead);
        free(animData);
        return NULL;
    }
    animData->cScaleKeys = count;
    animData->pScaleKeyHead = (KEY_SCALE*)malloc(sizeof(KEY_SCALE) * animData->cScaleKeys);
    if (animData->pScaleKeyHead == NULL) {
        free(animData->pQuatKeyHead);
        free(animData->pPosKeyHead);
        free(animData);
        return NULL;
    }
    nRead = fread(animData->pScaleKeyHead, sizeof(KEY_SCALE), animData->cScaleKeys, fp);
    if (nRead != animData->cScaleKeys) {
        free(animData->pScaleKeyHead);
        free(animData->pQuatKeyHead);
        free(animData->pPosKeyHead);
        free(animData);
        return NULL;
    }

    // Success!
    return animData;
}


////////////////////////////////////////////////////////////////
//
// Tell object to follow the given path
//
////////////////////////////////////////////////////////////////

void SetObjectAnimation(OBJECT *obj, ANIMATION_DATA *anim)
{
    obj->AnimData = anim;

    obj->AnimState.StartTime = TotalRacePhysicsTime;
    obj->AnimState.CurrentPosKey = 0;
    obj->AnimState.CurrentQuatKey = 0;
    obj->AnimState.CurrentScaleKey = 0;

    if (anim != NULL) {
        obj->movehandler = obj->defaultmovehandler = (MOVE_HANDLER)FollowPath;
    }
}


////////////////////////////////////////////////////////////////
//
// TransformAnimationPath: convert from Max to ReVolt format
//
////////////////////////////////////////////////////////////////

void TransformAnimation(ANIMATION_DATA *animData)
{
#if PRE_TRANSFORM_PATH
    int iQuat;
    MAT rotX, rotY, mat, interpMat;

    RotationX(&rotX, PI/2);
    RotationY(&rotY, -PI/2);

    for (iQuat = 0; iQuat < animData->cQuatKeys; iQuat++) {

        QuatToMat(&animData->pQuatKeyHead[iQuat].qQuat, &interpMat);
        
        TransMatMulMat(&rotX, &interpMat, &mat);
        MatMulMat(&mat, &rotX, &interpMat);

        MatMulMat(&rotY, &interpMat, &mat);

        MatToQuat(&mat, &animData->pQuatKeyHead[iQuat].qQuat);

        // Make sure quat is in positive VY hemisphere
        ConstrainQuat1(&animData->pQuatKeyHead[iQuat].qQuat);

        // Rotate -90 degrees about x axis
        //QuatMulQuat(&quatX, &animData->pQuatKeyHead[iQuat].qQuat, &qT1);
        //QuatMulInvQuat(&qT1, &quatX, &qT2);

        //ConjQuat(&qT2, &animData->pQuatKeyHead[iQuat].qQuat);
        //CopyQuat(&qT2, &animData->pQuatKeyHead[iQuat].qQuat);
    }
#endif

}

////////////////////////////////////////////////////////////////
//
// FollowPath:
//
////////////////////////////////////////////////////////////////

void FollowPath(OBJECT *obj)
{
    long frame;
    long posFrame = 0;
    long quatFrame = 0;
    REAL lastTime, curTime, nextTime, interpTime, scale;
    VEC interpPos;
    VEC *lastPos, *curPos, *nextPos;
    QUATERNION interpQuat;
    QUATERNION *lastQuat, *curQuat;
    MAT mat;
    ANIMATION_DATA *animData = obj->AnimData;
    ANIMATION_STATE *animState = &obj->AnimState;

    Assert(animData->cPosKeys > 2);
    Assert(animData->cQuatKeys > 2);
    Assert(obj->Type == OBJECT_TYPE_CAR);

    // Calculate the current frame number
    frame = ((TotalRacePhysicsTime - animState->StartTime) * ANIM_FPS) / 1000;

    //Position interpolation
    // Find the key frame for position
    while ((animState->CurrentPosKey < animData->cPosKeys) && (frame > animData->pPosKeyHead[animState->CurrentPosKey].iFrame)) 
    {
        animState->CurrentPosKey++;
    }

    // Get the positions to interpolate between
    if (animState->CurrentPosKey == 0) {

        lastPos = &animData->pPosKeyHead[0].vPos;
        lastTime = TO_TIME(Real(0.03333)) * animData->pPosKeyHead[0].iFrame;
        
        curPos = &animData->pPosKeyHead[1].vPos;
        curTime = TO_TIME(Real(0.03333)) * animData->pPosKeyHead[1].iFrame;
        
        nextPos = &animData->pPosKeyHead[2].vPos;
        nextTime = TO_TIME(Real(0.03333)) * animData->pPosKeyHead[2].iFrame;
        
        interpTime = TO_TIME(Real(0.03333)) * frame;

    } else if (animState->CurrentPosKey == animData->cPosKeys) {

        lastPos = curPos = nextPos = &animData->pPosKeyHead[animData->cPosKeys - 1].vPos;
        lastTime = ZERO;
        curTime = HALF;
        nextTime = ONE;
        interpTime = ONE;

    } else {
        
        lastPos = &animData->pPosKeyHead[animState->CurrentPosKey - 1].vPos;
        lastTime = TO_TIME(Real(0.03333)) * animData->pPosKeyHead[animState->CurrentPosKey - 1].iFrame;
        
        curPos = &animData->pPosKeyHead[animState->CurrentPosKey].vPos;
        curTime = TO_TIME(Real(0.03333)) * animData->pPosKeyHead[animState->CurrentPosKey].iFrame ;
        
        nextPos = &animData->pPosKeyHead[animState->CurrentPosKey + 1].vPos;
        nextTime = TO_TIME(Real(0.03333)) * animData->pPosKeyHead[animState->CurrentPosKey + 1].iFrame;
        
        interpTime = TO_TIME(Real(0.03333)) * frame;
    }

    // Calculate the new position
    QuadInterpVec(lastPos, lastTime, curPos, curTime, nextPos, nextTime, interpTime, &interpPos);
    ScalarVecPlusScalarVec(0.2f, &interpPos, 0.8f, &obj->body.Centre.Pos, &interpPos);

    //Quaternion interpolation
    // Find the key frame for quaternion
    while ((animState->CurrentQuatKey < animData->cQuatKeys) && (frame > animData->pQuatKeyHead[animState->CurrentQuatKey].iFrame)) 
    {
        animState->CurrentQuatKey++;
    }

    // Get the quaternions to (linearly) interpolate between
    if (animState->CurrentQuatKey == 0) {
        
        lastQuat = &animData->pQuatKeyHead[0].qQuat;
        lastTime = TO_TIME(Real(0.03333)) * animData->pQuatKeyHead[0].iFrame;

        curQuat = &animData->pQuatKeyHead[1].qQuat;
        curTime = TO_TIME(Real(0.03333)) * animData->pQuatKeyHead[1].iFrame;

        interpTime = TO_TIME(Real(0.03333)) * frame;

    } else if (animState->CurrentQuatKey == animData->cQuatKeys) {

        lastQuat = curQuat = &animData->pQuatKeyHead[animData->cQuatKeys - 1].qQuat;
        lastTime = ZERO;
        curTime = ONE;
        interpTime = ONE;

    } else {

        lastQuat = &animData->pQuatKeyHead[animState->CurrentQuatKey - 1].qQuat;
        lastTime = TO_TIME(Real(0.03333)) * animData->pQuatKeyHead[animState->CurrentQuatKey - 1].iFrame;

        curQuat = &animData->pQuatKeyHead[animState->CurrentQuatKey].qQuat;
        curTime = TO_TIME(Real(0.03333)) * animData->pQuatKeyHead[animState->CurrentQuatKey].iFrame;

        interpTime = TO_TIME(Real(0.03333)) * frame;
    }

    // Calculate new quaternion and matrix
    scale = (interpTime - lastTime) / (curTime - lastTime);
    SLerpQuat(lastQuat, curQuat, scale, &interpQuat);
    //LerpQuat(lastQuat, curQuat, scale, &interpQuat);
    NormalizeQuat(&interpQuat);

#if PRE_TRANSFORM_PATH

    QuatToMat(&interpQuat, &mat);

#else

    QuatToMat(&interpQuat, &interpMat);

    RotationX(&rotX, PI/2);
    TransMatMulMat(&rotX, &interpMat, &mat);
    MatMulMat(&mat, &rotX, &interpMat);

    RotationY(&rotX, -PI/2);
    MatMulMat(&rotX, &interpMat, &mat);

#endif

    // Set the new car position
    UpdateCarPos(&obj->player->car, &interpPos, &mat);



}


////////////////////////////////////////////////////////////////
//
// Scale Animation Path
//
////////////////////////////////////////////////////////////////

void MoveAndScaleAnimationPath(ANIMATION_DATA *animData, VEC *shift, REAL scale)
{
    long iPos;

    for (iPos = 0; iPos < animData->cPosKeys; iPos++) {
        VecMulScalar(&animData->pPosKeyHead[iPos].vPos, scale);
        VecPlusEqVec(&animData->pPosKeyHead[iPos].vPos, shift);
    }
}

////////////////////////////////////////////////////////////////
//
// DrawAnimationPath:
//
////////////////////////////////////////////////////////////////
MODEL AnimPathModel;

void DrawAnimationPath(ANIMATION_DATA *animData)
{
    long iPos;
    VEC pos;
    WCHAR buf[16];

    for (iPos = 0; iPos < animData->cPosKeys; iPos++) {
        DrawModel(&AnimPathModel, &IdentityMatrix, &animData->pPosKeyHead[iPos].vPos, MODEL_PLAIN);

        swprintf(buf, L"%d", iPos);
        RotTransVector(&ViewMatrix, &ViewTrans, &animData->pPosKeyHead[iPos].vPos, &pos);
        pos.v[X] -= wcslen(buf) * 4.0f;
        pos.v[Y] -= 48.0f;

        if (pos.v[Z] > RenderSettings.NearClip)
            DumpText3D(&pos, 8, 16, 0xffff00, buf);
    }



}



