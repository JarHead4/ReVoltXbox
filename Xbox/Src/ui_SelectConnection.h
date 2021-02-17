//-----------------------------------------------------------------------------
// File: ui_SelectConnection.h
//
// Desc: UI implementation
//
// Re-Volt (Generic) Copyright (c) Probe Entertainment 1998
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef UI_SELECTCONNECTION_H
#define UI_SELECTCONNECTION_H


extern MENU Menu_Connection;
extern MENU Menu_MultiGameSelect;


//-----------------------------------------------------------------------------
// The SelectConnection state engine
//-----------------------------------------------------------------------------
class CSelectConnectionStateEngine : public CUIStateEngine
{
public:
    virtual HRESULT Process();
    virtual WCHAR*  DebugGetName()   { return L"SelectConnection"; }
};

extern CSelectConnectionStateEngine g_SelectConnectionStateEngine;


#endif //UI_SELECTCONNECTION_H
