//-----------------------------------------------------------------------------
// File: ui_ProgressTable.h
//
// Desc: Public UI Data Declarations
//
// Re-Volt (Generic) Copyright (c) Probe Entertainment 1998
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef UI_PROGRESSTABLE_H
#define UI_PROGRESSTABLE_H


extern MENU Menu_ProgressTable;


//-----------------------------------------------------------------------------
// The ProgressTable state engine
//-----------------------------------------------------------------------------
class CProgressTableStateEngine : public CUIStateEngine
{
public:
    virtual HRESULT Process();
    virtual WCHAR*  DebugGetName()   { return L"ProgressTable"; }
};

extern CProgressTableStateEngine g_ProgressTableStateEngine;



#endif //  UI_PROGRESSTABLE_H
