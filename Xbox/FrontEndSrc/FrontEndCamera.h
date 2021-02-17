//-----------------------------------------------------------------------------
// File: FrontEndCamera.h
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef FRONTEND_CAMERA_H
#define FRONTEND_CAMERA_H

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
    TITLESCREEN_CAMPOS_BRONZE_TROPHY,
    TITLESCREEN_CAMPOS_SILVER_TROPHY,
    TITLESCREEN_CAMPOS_GOLD_TROPHY,
    TITLESCREEN_CAMPOS_PLATINUM_TROPHY,
    TITLESCREEN_CAMPOS_ALL_TROPHIES,
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
struct CFrontEndCamera 
{
	// Data
	CAMERA*     m_pCamera;
	
	FLOAT       m_fTimer;
	FLOAT       m_fMoveTime;

	VEC         m_vStartPos;
	QUATERNION  m_qStartQuat;

	CAMERA_POS* m_pCamPos;
	CAMERA_POS* m_pOldCamPos;

	// Member functions
	CFrontEndCamera();

    VOID SetMoveTime( FLOAT fTime ) { m_fMoveTime = fTime; }

	VOID SetInitalCameraPos( LONG CamPosIndex );
	VOID SetNewCameraPos( LONG CamPosIndex );
	VOID UpdateCamera();
};





#endif // FRONTEND_CAMERA_H

