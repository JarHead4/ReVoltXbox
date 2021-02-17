//-----------------------------------------------------------------------------
// File: ui_RaceDifficulty.h
//
// Desc: UI implementation
//
// Re-Volt (Generic) Copyright (c) Probe Entertainment 1998
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef UI_RACEDIFFICULTY_H
#define UI_RACEDIFFICULTY_H




extern MENU Menu_GameDifficulty;




//-----------------------------------------------------------------------------
// The SelectDifficulty state engine
//-----------------------------------------------------------------------------
class CSelectDifficultyStateEngine : public CUIStateEngine
{
public:
    virtual HRESULT Process();
    virtual WCHAR*  DebugGetName()   { return L"SelectDifficulty"; }
};

extern CSelectDifficultyStateEngine g_SelectDifficultyStateEngine;




#endif //UI_RACEDIFFICULTY_H
