//-----------------------------------------------------------------------------
// File: XBOnline.h
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef XBONLINE_H
#define XBONLINE_H




//-----------------------------------------------------------------------------
// Function prototypes
//-----------------------------------------------------------------------------

extern HRESULT XBOnline_Init( DWORD dwFlags );
extern HRESULT XBOnline_GetUserList( XONLINE_USER** ppUserList, DWORD* pdwNumUsers );
extern BOOL    XBOnline_IsActive();
extern HRESULT XBOnline_BeginSignOn( XONLINE_USER* pUserList[4] );
extern HRESULT XBOnline_PumpSignOnTask();
extern VOID    XBOnline_SignOff();




#endif // XBONLINE_H
