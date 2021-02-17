//-----------------------------------------------------------------------------
// File: ui_players.h
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef UI_PLAYERS_H
#define UI_PLAYERS_H


#include "ui_StateEngine.h"

// We want to rotate through several "has voice" icons to indicate speaking
extern const DWORD NUM_TALKICONCYCLE;
extern D3DTexture**    g_TalkIcons[];

//-----------------------------------------------------------------------------
// The Play Live state engine
//-----------------------------------------------------------------------------
class CPlayersStateEngine : public CUIStateEngine
{
public:
    enum
    {
        PLAYERS_STATE_BEGIN = STATEENGINE_STATE_BEGIN,
        PLAYERS_STATE_PLAYERACTION,
        PLAYERS_STATE_MAINLOOP,
    };

    MENU_ITEM m_amiPlayers[MAX_NUM_PLAYERS + MAX_DEPARTED_PLAYERS];
    virtual HRESULT Process();
    virtual WCHAR*  DebugGetName()   { return L"Players"; }
    DWORD   m_dwSelectedPlayer;

    HRESULT AddFriend();    // Adds the currently selected player as a friend

    LONG    m_lFirstPlayerOnScreen;
    BOOL    m_bPlayersListChanged;
    REAL    m_TalkIconTimer;
    DWORD   m_dwTalkIconIndex;

private:
    HRESULT GeneratePlayersMenu();
};

extern CPlayersStateEngine g_PlayersStateEngine;





#endif // UI_PLAYERS_H