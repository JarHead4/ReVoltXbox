//-----------------------------------------------------------------------------
// File: ui_SplashScreen.h
//
// Desc: UI implementation
//
// Re-Volt (Generic) Copyright (c) Probe Entertainment 1998
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef SPLASHSCREEN_H
#define SPLASHSCREEN_H

#include "ui_StateEngine.h"


extern MENU Menu_SplashScreen;


//-----------------------------------------------------------------------------
// SplashScreen Menu state engine
//-----------------------------------------------------------------------------
class CSplashScreenStateEngine : public CStateEngine
{
public:
    virtual HRESULT Process();
    virtual WCHAR*  DebugGetName()   { return L"SplashScreen"; }
};

extern CSplashScreenStateEngine g_SplashScreenStateEngine;




#endif // SPLASHSCREEN_H
