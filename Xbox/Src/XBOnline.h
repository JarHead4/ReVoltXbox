//-----------------------------------------------------------------------------
// File: XBOnline.h
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef XBONLINE_H
#define XBONLINE_H

#include <xonline.h>


// The task handle for XOnline API
extern XONLINETASK_HANDLE g_hSignInTask;

// Available XOnline services
extern DWORD g_pXOnlineServices[];
extern const DWORD NUM_XONLINE_SERVICES;

//-----------------------------------------------------------------------------
// Function prototypes
//-----------------------------------------------------------------------------

extern HRESULT XBOnline_Startup();  // Wrapper for XOnlineStartup
extern HRESULT XBOnline_Cleanup();  // Wrapper for XOnlineCleanup
extern HRESULT XBOnline_GetUserList( XONLINE_USER** ppUserList, DWORD* pdwNumUsers );
extern BOOL    XBOnline_IsActive();
extern HRESULT XBOnline_BeginSignIn( XONLINE_USER* pUserList[4] );
extern HRESULT XBOnline_PumpSignInTask();
extern VOID    XBOnline_SignOut();


#endif // XBONLINE_H
