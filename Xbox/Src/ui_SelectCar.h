//-----------------------------------------------------------------------------
// File: ui_SelectCar.h
//
// Desc: Public UI Data Declarations
//
// Re-Volt (Generic) Copyright (c) Probe Entertainment 1998
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef UI_SELECTCAR_H
#define UI_SELECTCAR_H


extern MENU Menu_CarSelect;


//-----------------------------------------------------------------------------
// The SelectCar state engine
//-----------------------------------------------------------------------------
class CSelectCarStateEngine : public CUIStateEngine
{
public:
    virtual HRESULT Process();
    virtual WCHAR*  DebugGetName()   { return L"SelectCar"; }
};

extern CSelectCarStateEngine g_SelectCarStateEngine;




#endif //  UI_SELECTCAR_H
