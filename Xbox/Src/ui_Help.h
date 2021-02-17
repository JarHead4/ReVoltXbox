//-----------------------------------------------------------------------------
// File: ui_Help.h
//
// Desc: Public UI Data Declarations
//
// Re-Volt (Generic) Copyright (c) Probe Entertainment 1998
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef UI_HELP_H
#define UI_HELP_H

#include "Content.h"

//-----------------------------------------------------------------------------
// content download states
//-----------------------------------------------------------------------------
enum
{
    HELP_STATE_BEGIN = STATEENGINE_STATE_BEGIN,
    HELP_STATE_LOOP

};

//-----------------------------------------------------------------------------
// Name: CContentDownloadEngine
// Desc: The content download engine
//-----------------------------------------------------------------------------
class CHelpControllerEngine : public CUIStateEngine
{
public:

    virtual WCHAR*  DebugGetName()   { return L"HelpController"; }
    virtual HRESULT Process();
};


//-----------------------------------------------------------------------------
// Name: g_HelpEngine
// Desc: global instance of help engine
//-----------------------------------------------------------------------------
extern CHelpControllerEngine g_HelpControllerEngine;



//-----------------------------------------------------------------------------
// Name: CContentDownloadEngine
// Desc: The content download engine
//-----------------------------------------------------------------------------
class CHelpWeaponsEngine : public CUIStateEngine
{
public:

    virtual WCHAR*  DebugGetName()   { return L"HelpWeapons"; }
    virtual HRESULT Process();
};


//-----------------------------------------------------------------------------
// Name: g_HelpEngine
// Desc: global instance of help engine
//-----------------------------------------------------------------------------
extern CHelpWeaponsEngine g_HelpWeaponsEngine;

#endif //  UI_HELP_H
