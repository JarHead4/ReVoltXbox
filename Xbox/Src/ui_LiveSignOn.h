//-----------------------------------------------------------------------------
// File: ui_LiveSignOn.h
//
// Desc: UI implementation
//
// Re-Volt (Generic) Copyright (c) Probe Entertainment 1998
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef LIVESIGNON_H
#define LIVESIGNON_H


VOID CheckSignInConditions();


//-----------------------------------------------------------------------------
// Live Sign In state engine
//-----------------------------------------------------------------------------
class CLiveSignInStateEngine : public CUIStateEngine
{
protected:
    virtual VOID HandleEnterFromParent();
    virtual VOID HandleExitToParent();
public:
    virtual HRESULT Process();
    virtual WCHAR*  DebugGetName()   { return L"LiveSignIn"; }

    BOOL    PlayersSignedIn();
	HRESULT SignOut();
};


extern CLiveSignInStateEngine g_LiveSignInStateEngine;




//-----------------------------------------------------------------------------
// Live Sign Out state engine
//-----------------------------------------------------------------------------
class CLiveSignOutStateEngine : public CUIStateEngine
{
protected:
    virtual VOID HandleEnterFromParent();
    virtual VOID HandleExitToParent();
public:
    virtual HRESULT Process();
    virtual WCHAR*  DebugGetName()   { return L"LiveSignOut"; }
};


extern CLiveSignOutStateEngine g_LiveSignOutStateEngine;




#endif //  LIVESIGNON_H
