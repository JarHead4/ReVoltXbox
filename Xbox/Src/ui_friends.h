//-----------------------------------------------------------------------------
// File: ui_friends.h
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef UI_FRIENDS_H
#define UI_FRIENDS_H


#include "ui_StateEngine.h"
#include "XBResource.h"
#include <xonline.h>

//-----------------------------------------------------------------------------
// The Play Live state engine
//-----------------------------------------------------------------------------
class CFriendsStateEngine : public CUIStateEngine
{
public:
    enum
    {
        FRIENDS_STATE_BEGIN = STATEENGINE_STATE_BEGIN,
        FRIENDS_STATE_FRIENDACTION,
        FRIENDS_STATE_ENUMERATING,
        FRIENDS_STATE_MAINLOOP,
        FRIENDS_STATE_HANDLEINVITE,
        FRIENDS_STATE_HANDLEREQUEST,
        FRIENDS_STATE_CONFIRM_REMOVE,
        FRIENDS_STATE_CONFIRM_ACCEPT,
        FRIENDS_STATE_CONFIRM_JOIN,
    };

    HRESULT CheckForFriendsUpdate();
    VOID    SyncCurrentFriend();
    HRESULT JoinFriend( XONLINE_FRIEND* pFriend, BOOL bInvited );

    MENU_ITEM m_amiFriends[MAX_FRIENDS];

    virtual HRESULT Process();
    virtual WCHAR*  DebugGetName()   { return L"Friends"; }

    XONLINE_FRIEND* m_pFriend;
    XONLINE_FRIEND  m_CurrentFriend;
    
    DWORD   m_dwCurrentPage;
    DWORD   m_dwUserIndex;

    WCHAR   m_strMessage[ XONLINE_MAX_GAMERTAG_LENGTH + 200 ];

};

extern CFriendsStateEngine g_FriendsStateEngine;

VOID RevokeAllInvites();



#endif // UI_FRIENDS_H