//-----------------------------------------------------------------------------
// File: XBOnline.cpp
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include <xtl.h>
#include <xonline.h>
#include "XBOnline.h"




//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------

// The task handle for XOnline API
static XONLINETASK_HANDLE	 g_hSignOnTask = NULL;

// Available XOnline services
static DWORD g_pXOnlineServices[] = 
{
	XONLINE_MATCHMAKING_SERVICE,
	XONLINE_BILLING_OFFERING_SERVICE,
};

#define NUM_XONLINE_SERVICES (sizeof(g_pXOnlineServices)/sizeof(DWORD))

static XONLINE_SERVICE_INFO  g_XOnlineServiceInfo[NUM_XONLINE_SERVICES];

// The list of XOnline users
static XONLINE_USER 		 g_XOnlineUserList[XONLINE_MAX_STORED_ONLINE_USERS];

// Flag for debug fallback when XOnline APIs fail
static BOOL g_bUsingXOnlineDebugFlag = FALSE;




//-----------------------------------------------------------------------------
// Name: XBOnline_Init()
// Desc: Performs all required initialization for XOnline and network APIs
//-----------------------------------------------------------------------------
HRESULT XBOnline_Init( DWORD dwFlags )
{
	// Mark a flag so we can fake online stuff in case the real stuff fails
	g_bUsingXOnlineDebugFlag = FALSE;

	// Initialize the network stack. For default initialization, call
	// XNetStartup( NULL );
	XNetStartupParams xnsp;
	ZeroMemory( &xnsp, sizeof(xnsp) );
	xnsp.cfgSizeOfStruct = sizeof(xnsp);
	xnsp.cfgFlags		 = (BYTE)dwFlags;
	
	INT iResult = XNetStartup( &xnsp );
	if( iResult != NO_ERROR )
		return E_FAIL;

	// Standard WinSock startup. The Xbox allows all versions of Winsock
	// up through 2.2 (i.e. 1.0, 1.1, 2.0, 2.1, and 2.2), although it 
	// technically supports only and exactly what is specified in the 
	// Xbox network documentation, not necessarily the full Winsock 
	// functional specification.
	WSADATA wsaData;
	iResult = WSAStartup( MAKEWORD(2,2), &wsaData );
	if( iResult != NO_ERROR )
		return E_FAIL;

	// Do startup for Xbox online functions
	XONLINE_STARTUP_PARAMS xosp = { 0 };
	
	HRESULT hr = XOnlineStartup( &xosp );
	if( FAILED(hr) )
		return hr;

	// Success
	g_bUsingXOnlineDebugFlag = TRUE;

	return S_OK;
}




//-----------------------------------------------------------------------------
// Name: XBOnline_GetUserList()
// Desc: Gets the list on available online users
//-----------------------------------------------------------------------------
HRESULT XBOnline_GetUserList( XONLINE_USER** ppUserList, DWORD* pdwNumUsers )
{
	// Static list of users
	DWORD dwNumUsers = 0;

	// Test flag to see if we should fake it
	if( g_bUsingXOnlineDebugFlag == FALSE )
	{
		strcpy( g_XOnlineUserList[0].name,	 "Mikey" );
		strcpy( g_XOnlineUserList[0].kingdom, "DagYo!" );
		g_XOnlineUserList[0].index = 0xffffffff;
		g_XOnlineUserList[0].dwUserOptions = XONLINE_USER_OPTION_REQUIRE_PIN;

		strcpy( g_XOnlineUserList[1].name,	 "Wendy" );
		strcpy( g_XOnlineUserList[1].kingdom, "Spr" );
		g_XOnlineUserList[1].index = 0xffffffff;
		g_XOnlineUserList[0].dwUserOptions = 0;
		
		strcpy( g_XOnlineUserList[2].name,	 "Tamie" );
		strcpy( g_XOnlineUserList[2].kingdom, "Ker" );
		g_XOnlineUserList[2].index = 0xffffffff;
		g_XOnlineUserList[0].dwUserOptions = 0;
		
		strcpy( g_XOnlineUserList[3].name,	 "Lori" );
		strcpy( g_XOnlineUserList[3].kingdom, "Dar" );
		g_XOnlineUserList[3].index = 0xffffffff;
		g_XOnlineUserList[0].dwUserOptions = 0;
		
		strcpy( g_XOnlineUserList[4].name,	 "Rook" );
		strcpy( g_XOnlineUserList[4].kingdom, "DagYo!" );
		g_XOnlineUserList[4].index = 0xffffffff;
		g_XOnlineUserList[0].dwUserOptions = XONLINE_USER_OPTION_REQUIRE_PIN;
		
		strcpy( g_XOnlineUserList[5].name,	 "Lily" );
		strcpy( g_XOnlineUserList[5].kingdom, "DagYo!" );
		g_XOnlineUserList[5].index = 0xffffffff;
		g_XOnlineUserList[0].dwUserOptions = 0;

		(*ppUserList)  = g_XOnlineUserList;
		(*pdwNumUsers) = 6;
		return S_OK;
	}
			
	// Get accounts stored on the hard disk
	HRESULT hr = XOnlineGetUsers( g_XOnlineUserList, &dwNumUsers );
	if( FAILED(hr) )
	{
		(*pdwNumUsers) = 0;
		return hr;
	}

	(*ppUserList)  = g_XOnlineUserList;
	(*pdwNumUsers) = dwNumUsers;

	return S_OK;
}




//-----------------------------------------------------------------------------
// Name: XBOnline_IsActive()
// Desc: Returns true is the ethernet link is active
//-----------------------------------------------------------------------------
BOOL XBOnline_IsActive()
{
	// Test flag to see if we should fake it
	if( g_bUsingXOnlineDebugFlag == FALSE )
		return TRUE;
	
	// Poll status
	DWORD dwStatus = XNetGetEthernetLinkStatus();
	return ( dwStatus & XNET_ETHERNET_LINK_ACTIVE ) ? TRUE : FALSE;
}




//-----------------------------------------------------------------------------
// Name: XBOnline_BeginSignOn()
// Desc: Initiate the authentication process
//-----------------------------------------------------------------------------
HRESULT XBOnline_BeginSignOn(	XONLINE_USER* pUserList[4] )
{
	// XOnlineLogon() allows a list of up to 4 players (1 per controller)
	// to login in a single call. This sample shows how to authenticate
	// a single user. The list must be a one-to-one match of controller 
	// to player in order for the online system to recognize which player
	// is using which controller.
	XONLINE_USER UserList[4];
	ZeroMemory( UserList, sizeof(UserList) );

	for( DWORD i=0; i<4; i++ )
	{
		if( pUserList[i] )
			CopyMemory( &UserList[i], pUserList[i], sizeof(XONLINE_USER) );
	}
	
	// Initiate the login process. XOnlineTaskContinue() is used to poll
	// the status of the login.
	HRESULT hr = XOnlineLogon( UserList, g_pXOnlineServices, NUM_XONLINE_SERVICES, 
							   NULL, &g_hSignOnTask );
	
	if( FAILED(hr) )
	{
		XBOnline_SignOff();
		return hr;
	}

	return S_OK;
}




//-----------------------------------------------------------------------------
// Name: XBOnline_PumpSignOnTask()
// Desc: 
//-----------------------------------------------------------------------------
HRESULT XBOnline_PumpSignOnTask()
{
	// Make sure we have a valid task handle
	if( NULL == g_hSignOnTask )
		return E_INVALIDARG;

	// Continue with the sign on task
	HRESULT hrPumpTaskResult = XOnlineTaskContinue( g_hSignOnTask );
	if( hrPumpTaskResult == XONLINETASK_S_RUNNING )
		return hrPumpTaskResult;

	if( FAILED(hrPumpTaskResult) )
	{
		// "Don't continue" flags indicate that the authentication failed,
		// perhaps because the connection was lost
		XBOnline_SignOff();
		return hrPumpTaskResult;
	}

	// Check for service errors
	for( DWORD i = 0; i < NUM_XONLINE_SERVICES; ++i )
	{
		HRESULT hrGetServicesResult = XOnlineGetServiceInfo( g_pXOnlineServices[i], 
															 &g_XOnlineServiceInfo[i] );
		if( FAILED(hrGetServicesResult) )
		{
			XBOnline_SignOff();
			return hrGetServicesResult;
		}
	}

	// If we get here, sign on succeeded. Return the success code.
	return hrPumpTaskResult;
}




//-----------------------------------------------------------------------------
// Name: XBOnline_SignOff()
// Desc: 
//-----------------------------------------------------------------------------
VOID XBOnline_SignOff()
{
	if( g_hSignOnTask ) 
		XOnlineTaskClose( g_hSignOnTask );
	g_hSignOnTask = NULL;
}




