//-----------------------------------------------------------------------------
// File: ui_SinglePlayerGame.h
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef SINGLEPLAYERGAME_H
#define SINGLEPLAYERGAME_H




//-----------------------------------------------------------------------------
// The SinglePlayerGame state engine
//-----------------------------------------------------------------------------
class CSinglePlayerGameStateEngine : public CUIStateEngine
{
public:
    virtual HRESULT Process();
    virtual WCHAR*  DebugGetName()   { return L"SinglePlayerGame"; }
};

extern CSinglePlayerGameStateEngine g_SinglePlayerGameStateEngine;




#endif // SINGLEPLAYERGAME_H