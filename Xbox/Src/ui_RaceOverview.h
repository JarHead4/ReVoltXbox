//-----------------------------------------------------------------------------
// File: ui_RaceOverview.h
//
// Desc: Public UI Data Declarations
//
// Re-Volt (Generic) Copyright (c) Probe Entertainment 1998
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef UI_RACEOVERVIEW_H
#define UI_RACEOVERVIEW_H


extern MENU Menu_Overview;


//-----------------------------------------------------------------------------
// The RaceOverview state engine
//-----------------------------------------------------------------------------
class CRaceOverviewStateEngine : public CUIStateEngine
{
public:
    virtual HRESULT Process();
    virtual WCHAR*  DebugGetName()   { return L"Overview"; }
};

extern CRaceOverviewStateEngine g_RaceOverviewStateEngine;


#endif //  UI_RACEOVERVIEW_H
