//-----------------------------------------------------------------------------
// File: ui_EnterName.h
//
// Desc: UI implementation
//
// Re-Volt (Generic) Copyright (c) Probe Entertainment 1998
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef UI_ENTERNAME_H
#define UI_ENTERNAME_H




extern MENU Menu_EnterName;




//-----------------------------------------------------------------------------
// The EnterName state engine
//-----------------------------------------------------------------------------
class CEnterNameStateEngine : public CUIStateEngine
{
public:
    virtual HRESULT Process();
    virtual WCHAR*  DebugGetName()   { return L"EnterName"; }
};

extern CEnterNameStateEngine g_EnterNameStateEngine;


#endif //UI_ENTERNAME_H
