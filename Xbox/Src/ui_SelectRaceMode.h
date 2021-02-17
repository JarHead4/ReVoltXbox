//-----------------------------------------------------------------------------
// File: ui_SelectRaceMode.h
//
// Desc: UI implementation
//
// Re-Volt (Generic) Copyright (c) Probe Entertainment 1998
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef UI_SELECTRACEMODE_H
#define UI_SELECTRACEMODE_H


extern MENU Menu_StartRace;
extern MENU Menu_MultiType;




//-----------------------------------------------------------------------------
// The SelectRaceMode state engine
//-----------------------------------------------------------------------------
class CSelectRaceModeStateEngine : public CUIStateEngine
{
public:
    virtual HRESULT Process();
    virtual WCHAR*  DebugGetName()   { return L"SelectRaceMode"; }
};

extern CSelectRaceModeStateEngine g_SelectRaceModeStateEngine;




#endif //UI_SELECTRACEMODE_H
