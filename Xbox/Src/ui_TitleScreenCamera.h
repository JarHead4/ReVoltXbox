//-----------------------------------------------------------------------------
// File: ui_TitleScreenCamera.h
//
// Desc: UI implementation
//
// Re-Volt (Generic) Copyright (c) Probe Entertainment 1998
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef TITLESCREENCAMERA_H
#define TITLESCREENCAMERA_H

#include "Camera.h"


// Camera positions
enum TITLESCREEN_CAMERA_POSITION
{
    TITLESCREEN_CAMPOS_DONT_CHANGE = -1, 
    TITLESCREEN_CAMPOS_INIT        = 0,
    TITLESCREEN_CAMPOS_START,
    TITLESCREEN_CAMPOS_CAR_SELECT,
    TITLESCREEN_CAMPOS_CAR_SELECTED,
    TITLESCREEN_CAMPOS_TRACK_SELECT,
    TITLESCREEN_CAMPOS_MULTI,
    TITLESCREEN_CAMPOS_TROPHY1,
    TITLESCREEN_CAMPOS_TROPHY2,
    TITLESCREEN_CAMPOS_TROPHY3,
    TITLESCREEN_CAMPOS_TROPHY4,
    TITLESCREEN_CAMPOS_TROPHYALL,
    TITLESCREEN_CAMPOS_OVERVIEW,
    TITLESCREEN_CAMPOS_NAME_SELECT,
    TITLESCREEN_CAMPOS_DINKYDERBY,
    TITLESCREEN_CAMPOS_RACE,
    TITLESCREEN_CAMPOS_BESTTIMES,
    TITLESCREEN_CAMPOS_INTO_TRACKSCREEN,
    TITLESCREEN_CAMPOS_SUMMARY,
    TITLESCREEN_CAMPOS_PODIUMSTART,
    TITLESCREEN_CAMPOS_PODIUM,
    TITLESCREEN_CAMPOS_PODIUMVIEW1,
    TITLESCREEN_CAMPOS_PODIUMVIEW2,
    TITLESCREEN_CAMPOS_PODIUMVIEW3,
    TITLESCREEN_CAMPOS_PODIUMLOSE,

    NUM_TITLESCREEN_CAMERA_POSITIONS
};




// Camera position
struct CAMERA_POS
{
    VEC            vEye;
    VEC            vFocusPt;
    QUATERNION     Quat;
};




// Camera for the frontend
struct CTitleScreenCamera 
{
    // Data
    CAMERA*     m_pCamera;
    
    FLOAT       m_fTimer;
    FLOAT       m_fMoveTime;

    VEC         m_vStartPos;
    QUATERNION  m_qStartQuat;

    LONG        m_CamPosIndex;
    CAMERA_POS* m_pCamPos;
    CAMERA_POS* m_pOldCamPos;

    // Member functions
    CTitleScreenCamera();

    VOID SetMoveTime( FLOAT fTime ) { m_fMoveTime = fTime; }

    VOID SetInitalPos( LONG CamPosIndex );
    LONG SetNewPos( LONG CamPosIndex );
    LONG GetPosIndex() { return m_CamPosIndex; }
    VOID Update();
};





#endif // TITLESCREENCAMERA_H

