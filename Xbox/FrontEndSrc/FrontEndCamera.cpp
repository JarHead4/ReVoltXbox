//-----------------------------------------------------------------------------
// File: FrontEndCamera.cpp
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "Revolt.h"
#include "FrontEndCamera.h"
#include "Timing.h"
#include "Visibox.h"



//-----------------------------------------------------------------------------
// Camera Info
//-----------------------------------------------------------------------------
CAMERA_POS g_CameraPositions[NUM_TITLESCREEN_CAMERA_POSITIONS] =
{
    {{   76.5f,  -79.3f,-4462.6f},{   109.5f, -155.3f,  -3466.1f }}, // Init
    {{   93.5f, -102.9f,-1926.7f},{   110.9f, -296.562f, -945.7f }}, // Start
    {{  863.5f,-1092.5f,  512.5f},{  1491.2f, -561.9f,   1082.2f }}, // Car select
    {{ 1355.5f, -166.2f,  864.4f},{  2204.6f,  360.0f,    911.5f }}, // Car select 2
    {{ 1589.6f,  -81.8f,-1743.8f},{  2445.7f, -365.5f,  -1311.8f }}, // track select
    {{ -212.5f, -664.6f, -706.8f},{  -212.5f, -901.3f,    134.4f }}, // multi player
    {{-1773.9f, -503.9f, -477.2f},{ -2657.6f,  -41.1f,   -546.7f }}, // trophy 1
    {{-1637.2f, -764.6f,  206.4f},{ -2420.9f, -224.8f,   -101.0f }}, // trophy 2
    {{-1855.2f, -733.4f,  669.9f},{ -2503.0f,  -83.4f,    271.3f }}, // trophy 3
    {{-2007.7f, -713.5f, 1026.9f},{ -2575.5f,  -29.3f,    569.3f }}, // trophy 4

    {{-1171.6f, -137.8f,  834.4f},{ -2080.1f, -287.1f,  444.1f }},   // trophy all
    {{ -816.2f, -534.5f, -500.0f},{ -1591.0f, -392.4f,-1115.9f }},   // Dinky Derby
    {{  527.1f, -930.5f, 1322.3f},{  1105.5f, -375.5f, 1922.1f }},   // Name select
    {{ -521.2f,-2870.0f, -325.4f},{  -306.2f,-1894.6f, -276.3f }},   // Overview
    {{  146.4f, -919.2f, -206.2f},{    35.6f,   33.0f,  -78.3f }},   // Race Type Select
    {{ -696.8f, -493.2f,-1728.4f},{ -1611.2f, -510.2f,-2133.0f }},   // best times
    {{ 2233.9f, -507.6f,-1393.0f},{  3125.8f, -664.1f, -968.6f }},   // into trackscreen
    {{ 2026.2f, -407.2f,-1501.5f},{  2914.3f, -579.5f,-1075.5f }},   // Summary
    {{-1047.1f, -746.2f, 1165.8f},{ -1692.1f, -377.2f, 1835.1f }},   // Podium Start
    {{-1047.1f, -746.2f, 1165.8f},{ -1692.1f, -377.2f, 1835.1f }},   // Podium View
    {{-1436.8f, -612.4f, 1353.8f},{ -1906.3f, -354.4f, 2198.2f }},   // Podium View 1
    {{-1676.0f, -534.9f, 1268.1f},{ -1950.7f,  -21.6f, 2081.1f }},   // Podium View 2
    {{-1052.3f, -437.1f, 1770.1f},{ -1926.2f,  -44.5f, 2056.7f }},   // Podium View 3
    {{-1047.1f, -746.2f, 1165.8f},{ -1692.1f,   -0.0f, 1835.1f }},   // Podium Lose View
};




//-----------------------------------------------------------------------------
// Name: InitFrontEndCameraPositions()
// Desc: 
//-----------------------------------------------------------------------------
VOID InitFrontEndCameraPositions()
{
	for( int i = 0; i < NUM_TITLESCREEN_CAMERA_POSITIONS; i++ )
    {
	    MAT matrix;
        BuildLookMatrixForward( &g_CameraPositions[i].vEye, &g_CameraPositions[i].vFocusPt, &matrix );
        MatToQuat( &matrix, &g_CameraPositions[i].Quat );
    }
}




//-----------------------------------------------------------------------------
// Name: CFrontEndCamera()
// Desc: 
//-----------------------------------------------------------------------------
CFrontEndCamera::CFrontEndCamera()
{
	// Create an actual camera
	m_pCamera    = AddCamera( 0, 0, 0, 0, 0 );
	SetCameraFreedom( m_pCamera, NULL, 0 );

	// Initialize members
	m_pCamPos    = NULL;
	m_pOldCamPos = NULL;

	// Initialize the frontend camera positions
	InitFrontEndCameraPositions();
}




//-----------------------------------------------------------------------------
// Name: UpdateCamera()
// Desc: 
//-----------------------------------------------------------------------------
VOID CFrontEndCamera::UpdateCamera()
{
    // Update timer
    m_fTimer += TimeStep;
    if( m_fTimer > m_fMoveTime )
		m_fTimer = m_fMoveTime;

	// Determine position and quaternion
    FLOAT t = sinf( (RAD / 4.0f) * m_fTimer / m_fMoveTime );
    if( t < 0.0f ) t = 0.0f;
    if( t > 1.0f ) t = 1.0f;

    InterpVec( &m_vStartPos, &m_pCamera->DestOffset, t, &m_pCamera->WPos );
    SLerpQuat( &m_qStartQuat, &m_pCamera->DestQuat, t, &m_pCamera->Quat );

    ConstrainQuat2( &m_pCamPos->Quat, &m_pCamera->Quat );
    QuatToMat( &m_pCamera->Quat, &m_pCamera->WMatrix);

	// Set viewport stuff (does this really need to happen?)
	SetViewport( m_pCamera->X, m_pCamera->Y, m_pCamera->Xsize, m_pCamera->Ysize, BaseGeomPers + m_pCamera->Lens);
	SetCameraView( &m_pCamera->WMatrix, &m_pCamera->WPos, m_pCamera->Shake );
	SetCameraVisiMask( &m_pCamera->WPos );
}




//-----------------------------------------------------------------------------
// Name: SetInitalCameraPos()
// Desc: 
//-----------------------------------------------------------------------------
VOID CFrontEndCamera::SetInitalCameraPos( LONG CamPosIndex )
{
    m_pCamera->WPos = g_CameraPositions[CamPosIndex].vEye;
    m_pCamera->Quat = g_CameraPositions[CamPosIndex].Quat;
}




//-----------------------------------------------------------------------------
// Name: SetNewCameraPos()
// Desc: 
//-----------------------------------------------------------------------------
VOID CFrontEndCamera::SetNewCameraPos( LONG CamPosIndex )
{
	if( CamPosIndex < 0 )
		return;

	m_pCamPos = &g_CameraPositions[CamPosIndex];

    // If camera destination has changed, set destination state
    if( m_pCamPos != m_pOldCamPos ) 
	{
		m_pCamera->DestOffset = m_pCamPos->vEye;
		m_pCamera->DestQuat   = m_pCamPos->Quat;

		m_vStartPos  = m_pCamera->WPos;
		m_qStartQuat = m_pCamera->Quat;
	
		ConstrainQuat2( &m_pCamera->DestQuat, &m_qStartQuat );

		m_pOldCamPos = m_pCamPos;
		
		m_fTimer = 0.0f;
    }
};
