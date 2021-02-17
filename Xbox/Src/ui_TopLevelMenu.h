//-----------------------------------------------------------------------------
// File: ui_TopLevelMenu.h
//
// Desc: UI implementation
//
// Re-Volt (Generic) Copyright (c) Probe Entertainment 1998
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef TOPLEVELMENU_H
#define TOPLEVELMENU_H

#include "ui_StateEngine.h"


extern MENU Menu_TopLevel;


//-----------------------------------------------------------------------------
// TopLevel Menu state engine
//-----------------------------------------------------------------------------
class CTopLevelMenuStateEngine : public CUIStateEngine
{
public:
    virtual HRESULT Process();
    virtual WCHAR*  DebugGetName()   { return L"TopLevelMenu"; }
};

extern CTopLevelMenuStateEngine g_TopLevelMenuStateEngine;




#endif // TOPLEVELMENU_H
