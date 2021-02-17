//-----------------------------------------------------------------------------
// File: ui_friends.cpp
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"
#include "dx.h"
#include "input.h"
#include "geom.h"
#include "camera.h"
#include "LevelLoad.h"
#include "player.h"
#include "settings.h"
#include "spark.h"
#include "timing.h"
#include "ui_TitleScreen.h"
#include "InitPlay.h"
#include "competition.h"
#include "main.h"
#include "player.h"
#include "credits.h"
#include "SoundEffectEngine.h"
#include "MusicManager.h"
#include "gameloop.h"
#include "readinit.h"
#include "panel.h"
#include "Cheats.h"
#include "ui_MenuText.h"
#include "ui_MenuDraw.h"
#include "ui_friends.h"
#include "ui_showmessage.h"
#include "text.h"
#include "net_xonline.h"
#include "ui_RaceOverview.h"
#include "ui_PlayLive.h"
#include "ui_LiveSignOn.h"
#include "ui_TopLevelMenu.h"
#include "VoiceManager.h"
#include "ui_menutext.h"
#include "FriendsManager.h"

// use this define to skip xonline apis when testnet is dead
//#define XONLINE_OFFLINE


CFriendsStateEngine g_FriendsStateEngine;

#define MENU_FRIENDS_XPOS              100
#define MENU_FRIENDS_YPOS              120
const DWORD FRIENDS_PER_PAGE = 5;

//-----------------------------------------------------------------------------
// Friends pMenu
//-----------------------------------------------------------------------------

// Friends helper functions
void CreateFriendsMenu( MENU_HEADER* pMenuHeader, MENU* pMenu );
BOOL HandleFriendsMenu( MENU_HEADER* pMenuHeader, DWORD dwInput );
void DrawFriendsMenu( MENU_HEADER* pMenuHeader, MENU* pMenu );
void DrawFriendsMenuItem( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem, int itemIndex );
void DrawFriendDetails( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem, int itemIndex );

// Friends menu struct
extern MENU Menu_Friends = 
{
    TEXT_FRIENDS,
    MENU_DEFAULT | MENU_CENTRE_X | MENU_NOBOX,  // Menu type
    CreateFriendsMenu,                      // Create menu function
    HandleFriendsMenu,                      // Input handler function
    DrawFriendsMenu,                        // Menu draw function
    MENU_FRIENDS_XPOS,                      // X coord
    MENU_FRIENDS_YPOS,
};

static MENU_ITEM MenuItem_FriendDetails =
{
    TEXT_NONE,
    100.0f,
    NULL,
    DrawFriendDetails,
};




//-----------------------------------------------------------------------------
// Enumerating friends pMenu
//-----------------------------------------------------------------------------

// Enumerating Friends helper functions
void CreateEnumeratingFriendsMenu( MENU_HEADER* pMenuHeader, MENU* pMenu );

// Enumerating Friends menu struct
extern MENU Menu_EnumeratingFriends = 
{
    TEXT_FRIENDS,
    MENU_DEFAULT | MENU_CENTRE_X,           // Menu type
    CreateEnumeratingFriendsMenu,           // Create menu function
    HandleFriendsMenu,                      // Input handler function
    NULL,                                   // Menu draw function
    MENU_FRIENDS_XPOS,                      // X coord
    MENU_FRIENDS_YPOS,
};




//-----------------------------------------------------------------------------
// Friends Handle Invite pMenu
//-----------------------------------------------------------------------------

// Friends Handle Invite helper functions
void CreateFriendHandleInviteMenu( MENU_HEADER* pMenuHeader, MENU* pMenu );
BOOL HandleFriendHandleInviteMenu( MENU_HEADER* pMenuHeader, DWORD dwInput );

// Friends Handle Invite menu struct
extern MENU Menu_FriendHandleInvite =
{
    TEXT_FRIENDS,
    MENU_DEFAULT | MENU_CENTRE_X,           // Menu type
    CreateFriendHandleInviteMenu,         // Create menu function
    HandleFriendHandleInviteMenu,         // Input handler function
    NULL,                                   // Menu draw function
    MENU_FRIENDS_XPOS,                      // X coord
    MENU_FRIENDS_YPOS,
};




//-----------------------------------------------------------------------------
// Friends Handle Request pMenu
//-----------------------------------------------------------------------------

// Friends Handle Request helper functions
void CreateFriendHandleRequestMenu( MENU_HEADER *pMenuHeader, MENU *pMenu );
BOOL HandleFriendHandleRequestMenu( MENU_HEADER* pMenuHeader, DWORD dwInput );

// Friends Handle Request menu struct
extern MENU Menu_FriendHandleRequest =
{
    TEXT_FRIENDS,
    MENU_DEFAULT | MENU_CENTRE_X,           // Menu type
    CreateFriendHandleRequestMenu,          // Create menu function
    HandleFriendHandleRequestMenu,          // Input handler function
    NULL,                                   // Menu draw function
    MENU_FRIENDS_XPOS,                      // X coord
    MENU_FRIENDS_YPOS,
};




//-----------------------------------------------------------------------------
// Friends Action pMenu
//-----------------------------------------------------------------------------

// Friends Action helper functions
void CreateFriendActionMenu(MENU_HEADER *pMenuHeader, MENU *pMenu);
BOOL HandleFriendActionMenu( MENU_HEADER* pMenuHeader, DWORD dwInput );

// Friends Action menu struct
extern MENU Menu_FriendAction = 
{
    TEXT_FRIENDS,
    MENU_DEFAULT | MENU_CENTRE_X,           // Menu type
    CreateFriendActionMenu,                 // Create menu function
    HandleFriendActionMenu,                 // Input handler function
    NULL,                                   // Menu draw function
    MENU_FRIENDS_XPOS,                      // X coord
    MENU_FRIENDS_YPOS,
};


BOOL IsFriendJoinable( XONLINE_FRIEND* pFriend )
{
    // A friend is joinable if they have the flag set, and we're not
    // currently in a game with them.  Unfortunately, PlayerIndexFromXUID
    // can operate off of stale data if we're not in a game
    return( ( pFriend->dwFriendState & XONLINE_FRIENDSTATE_FLAG_ONLINE ) &&
            ( pFriend->dwFriendState & XONLINE_FRIENDSTATE_FLAG_JOINABLE ) &&
            ( ( PlayerIndexFromXUID( pFriend->xuid ) == -1 ) ||
              ( !IsInGameSession() ) ) );
}




//-----------------------------------------------------------------------------
// Name: CreateFriendsMenu()
// Desc: Creation function to populate the friends menu with either a 
//          placeholder saying "no friends" or the list of friends
//-----------------------------------------------------------------------------
void CreateFriendsMenu(MENU_HEADER *pMenuHeader, MENU *pMenu)
{
    pMenu->CurrentItemIndex = 0;

    // Don't have anything to add for a while
    if( g_FriendsManager.GetNumFriends( g_FriendsStateEngine.m_dwUserIndex ) == 0 )
    {
        pMenuHeader->AddMenuItem( TEXT_FRIENDS_LISTISEMPTY, 0 );
    }

    //$HACK(Apr02_GameBash) - create a menu with only 1 page of FRIENDS
    DWORD i = 0;
    while( i < FRIENDS_PER_PAGE &&
           g_FriendsStateEngine.m_dwCurrentPage * FRIENDS_PER_PAGE + i < g_FriendsManager.GetNumFriends( g_FriendsStateEngine.m_dwUserIndex ) )
    {
        pMenuHeader->AddMenuItem( &g_FriendsStateEngine.m_amiFriends[ g_FriendsStateEngine.m_dwCurrentPage * FRIENDS_PER_PAGE + i ] );
        i++;
    }

    if( g_FriendsManager.GetNumFriends( g_FriendsStateEngine.m_dwUserIndex ) > 0 )
    {
        g_FriendsStateEngine.m_pFriend = g_FriendsManager.GetFriend( g_FriendsStateEngine.m_dwUserIndex, g_FriendsStateEngine.m_dwCurrentPage * FRIENDS_PER_PAGE + pMenuHeader->m_pMenu->CurrentItemIndex );

        // MenuItem_FriendDetails uses the Data field as a Y offset
        MenuItem_FriendDetails.Data = (VOID*)(DWORD)( ( MENU_TEXT_HEIGHT * 2.0f + MENU_TEXT_VSKIP ) * i );
        pMenuHeader->AddMenuItem( &MenuItem_FriendDetails, MENU_ITEM_INACTIVE );
    }
}




//-----------------------------------------------------------------------------
// Name: HandleFriendsMenu()
// Desc: Handles inputs for the Friends Menu
//-----------------------------------------------------------------------------
BOOL HandleFriendsMenu( MENU_HEADER* pMenuHeader, DWORD dwInput )
{
    switch( dwInput )
    {
        case MENU_INPUT_UP:
            if( TRUE == SelectPreviousMenuItem( pMenuHeader, pMenuHeader->m_pMenu, NULL ) )
            {
                g_FriendsStateEngine.m_pFriend = g_FriendsManager.GetFriend( g_FriendsStateEngine.m_dwUserIndex, g_FriendsStateEngine.m_dwCurrentPage * FRIENDS_PER_PAGE + pMenuHeader->m_pMenu->CurrentItemIndex );
                return TRUE;
            }
            return FALSE;

        case MENU_INPUT_DOWN:
            if( TRUE == SelectNextMenuItem( pMenuHeader, pMenuHeader->m_pMenu, NULL ) )
            {
                g_FriendsStateEngine.m_pFriend = g_FriendsManager.GetFriend( g_FriendsStateEngine.m_dwUserIndex, g_FriendsStateEngine.m_dwCurrentPage * FRIENDS_PER_PAGE + pMenuHeader->m_pMenu->CurrentItemIndex );
                return TRUE;
            }
            return FALSE;

        //$HACK(Apr02_GameBash) - Right and left mean forward/back 1 page
        case MENU_INPUT_RIGHT:
            if( g_FriendsManager.GetNumFriends( g_FriendsStateEngine.m_dwUserIndex ) > FRIENDS_PER_PAGE * ( g_FriendsStateEngine.m_dwCurrentPage + 1 ) )
            {
                g_FriendsStateEngine.m_dwCurrentPage += 1;
                pMenuHeader->SetNextMenu( &Menu_Friends );
                return TRUE;
            }
            break;

        case MENU_INPUT_LEFT:
            if( g_FriendsStateEngine.m_dwCurrentPage > 0 )
            {
                g_FriendsStateEngine.m_dwCurrentPage -= 1;
                pMenuHeader->SetNextMenu( &Menu_Friends );
                return TRUE;
            }
            break;

        case MENU_INPUT_SELECT:
            if( g_FriendsManager.GetNumFriends( g_FriendsStateEngine.m_dwUserIndex ) )
            {
                g_FriendsStateEngine.m_pFriend = g_FriendsManager.GetFriend( g_FriendsStateEngine.m_dwUserIndex, g_FriendsStateEngine.m_dwCurrentPage * FRIENDS_PER_PAGE + pMenuHeader->m_pMenu->CurrentItemIndex );
                memcpy( &g_FriendsStateEngine.m_CurrentFriend, 
                        g_FriendsStateEngine.m_pFriend,
                        sizeof( XONLINE_FRIEND ) );

                g_FriendsStateEngine.NextState( CFriendsStateEngine::FRIENDS_STATE_FRIENDACTION );
                pMenuHeader->SetNextMenu( &Menu_FriendAction );
                return TRUE;
            }
            break;

        case MENU_INPUT_BACK:
            g_FriendsManager.StopUpdatingFriends( g_FriendsStateEngine.m_dwUserIndex );
            g_FriendsStateEngine.Return( STATEENGINE_TERMINATED );
            return TRUE;
    }

    return FALSE;
}




//-----------------------------------------------------------------------------
// Name: DrawFriendsMenu()
// Desc: 
//-----------------------------------------------------------------------------
VOID DrawFriendsMenu( MENU_HEADER* pMenuHeader, MENU* pMenu )
{
    FLOAT x = pMenuHeader->m_XPos - (MENU_TEXT_WIDTH*2);
    FLOAT y = pMenuHeader->m_YPos - (MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP);

    //$HACK(Apr02_GameBash) - only draw FRIENDS_PER_PAGE number of entries
    DWORD dwNumFriendsLeftInList = g_FriendsManager.GetNumFriends( g_FriendsStateEngine.m_dwUserIndex ) - g_FriendsStateEngine.m_dwCurrentPage * FRIENDS_PER_PAGE;
    DWORD dwNumFriendsOnPage = min( dwNumFriendsLeftInList, FRIENDS_PER_PAGE );
    DWORD dwNumEntries = max( 3, dwNumFriendsOnPage + 2 );

    FLOAT fHeight = ( ( MENU_TEXT_HEIGHT * 2.0f + MENU_TEXT_VSKIP ) * ( dwNumEntries + 1 ) );

    DrawNewSpruBox( x, y, 350.0f, fHeight );

    BeginTextState();

    DrawMenuText( pMenuHeader->m_XPos, 
                  pMenuHeader->m_YPos + ( ( MENU_TEXT_HEIGHT * 2.0f + MENU_TEXT_VSKIP ) * ( dwNumEntries - 1 ) ), 
                  MENU_TEXT_RGB_NORMAL, 
                  g_FriendsManager.GetNumFriends( g_FriendsStateEngine.m_dwUserIndex ) > 0 ? TEXT_TABLE(TEXT_BUTTON_A_SELECT_B_BACK) : TEXT_TABLE(TEXT_BUTTON_B_BACK) );

    if( dwNumFriendsLeftInList > FRIENDS_PER_PAGE )
    {
        static WCHAR strRightArrowString[3] = { 0x0020, 0x2192, 0x0000 };

        g_pFont->DrawText( x + 310, 
                           pMenuHeader->m_YPos - MENU_TEXT_HEIGHT / 2, 
                           MENU_TEXT_RGB_NORMAL, 
                           strRightArrowString );
    }
    if( g_FriendsStateEngine.m_dwCurrentPage > 0 )
    {
        static WCHAR strLeftArrowString[3]  = { 0x2190, 0x0020, 0x0000 };

        g_pFont->DrawText( pMenuHeader->m_XPos - MENU_TEXT_WIDTH, 
                           pMenuHeader->m_YPos - MENU_TEXT_HEIGHT / 2, 
                           MENU_TEXT_RGB_NORMAL, 
                           strLeftArrowString );
    }
}




//-----------------------------------------------------------------------------
// Name: DrawFriendsMenuItem()
// Desc: Draws a menu item for the Friends menu.  This includes status icons
//          and the friend's name
//-----------------------------------------------------------------------------
void DrawFriendsMenuItem( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem, int itemIndex )
{
    // Assert that the friend we're about to draw is actually a valid entry
    // in the friends list
    assert( pMenuItem->Data <= g_FriendsManager.GetFriend( g_FriendsStateEngine.m_dwUserIndex, g_FriendsManager.GetNumFriends( g_FriendsStateEngine.m_dwUserIndex ) - 1 ) );

    FLOAT fX = pMenuHeader->m_XPos;
    FLOAT fY = pMenuHeader->m_YPos + ( MENU_TEXT_HEIGHT * 2 + MENU_TEXT_VSKIP ) * itemIndex;
    XONLINE_FRIEND* pFriend = (XONLINE_FRIEND *)pMenuItem->Data;
    WCHAR wstrFriend[ XONLINE_GAMERTAG_SIZE ];  // convert to WCHAR for display
    swprintf( wstrFriend, L"%S", pFriend->szGamertag );
    DrawMenuText( fX + 80.0f, 
                  fY,
                  pMenu->CurrentItemIndex == itemIndex ? MENU_TEXT_RGB_CHOICE : MENU_TEXT_RGB_NORMAL,
                  wstrFriend );

    // Determine which of the friend status textures to display
    D3DTexture* pFriendStatusTexture = NULL;
    if( pFriend->dwFriendState & XONLINE_FRIENDSTATE_FLAG_RECEIVEDINVITE )
        pFriendStatusTexture = g_pGameInviteReceivedTexture;
    else if( pFriend->dwFriendState & XONLINE_FRIENDSTATE_FLAG_SENTINVITE &&
             !( ( pFriend->dwFriendState & XONLINE_FRIENDSTATE_FLAG_INVITEACCEPTED ||
                  pFriend->dwFriendState & XONLINE_FRIENDSTATE_FLAG_INVITEREJECTED ) ) )
        pFriendStatusTexture = g_pGameInviteSentTexture;
    else if( pFriend->dwFriendState & XONLINE_FRIENDSTATE_FLAG_RECEIVEDREQUEST )
        pFriendStatusTexture = g_pFriendReqReceivedTexture;
    else if( pFriend->dwFriendState & XONLINE_FRIENDSTATE_FLAG_SENTREQUEST )
        pFriendStatusTexture = g_pFriendReqSentTexture;
    else if( pFriend->dwFriendState & XONLINE_FRIENDSTATE_FLAG_ONLINE )
        pFriendStatusTexture = g_pFriendOnlineTexture;

    if( pFriendStatusTexture )
    {
        DrawScreenSpaceQuad( fX + 40.0f,
                             fY - 6.0f,
                             pFriendStatusTexture );
    }

    // Determine what voice image to draw.  If they're muted, then we should
    // display the muted icon.  Otherwise, if the voice manager or their
    // friends state says they have voice, draw the regular enabled icon.
    //$CONSIDER: If they're in the session, and we don't think they have 
    // voice, but their friends state says they do, we should probably
    // respect that.  In addition, we could display the talking animation
    // here if they're in-session
    D3DTexture* pVoiceTexture = NULL;
    if( g_FriendsManager.IsPlayerInMuteList( 0, pFriend->xuid ) )
    {
        pVoiceTexture = g_pTalkMutedTexture;
    }
    else if( pFriend->dwFriendState & XONLINE_FRIENDSTATE_FLAG_VOICE ||
             g_VoiceManager.DoesPlayerHaveVoice( pFriend->xuid ) )
    {
        pVoiceTexture = g_pTalkEnabledTexture;
    }
    
    if( pVoiceTexture )
    {
        DrawScreenSpaceQuad( fX,
                             fY - 6.0f,
                             g_pTalkEnabledTexture );
    }
}




//-----------------------------------------------------------------------------
// Name: DrawFriendDetails()
// Desc: Renders the detailed status message about the current friend.
// Note: Expects a y offset in pMenuItem->Data
//-----------------------------------------------------------------------------
void DrawFriendDetails( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem, int itemIndex )
{
    WCHAR           str[256];
    WCHAR           strFriendTitle[ MAX_TITLENAME_LEN ];
    XONLINE_FRIEND* pFriend = g_FriendsStateEngine.m_pFriend;
    BOOL            bIsInRevolt;

    // If g_FriendsStateEngine.m_pFriend is NULL, that means we detected
    // that the friend disappeared from the list.  We properly detect
    // this when it happens and switch menus, but the old menu may still
    // draw as it's being swapped out.
    if( !pFriend )
        return;

    // Assert that the friend we're about to draw is actually a valid entry
    // in the friends list
    assert( g_FriendsManager.GetNumFriends( 0 ) > 0 &&
            g_FriendsStateEngine.m_pFriend <= g_FriendsManager.GetFriend( g_FriendsStateEngine.m_dwUserIndex, g_FriendsManager.GetNumFriends( g_FriendsStateEngine.m_dwUserIndex ) - 1 ) );

    // Determine if the current friend is playing Re-Volt
    if( XOnlineTitleIdIsSameTitle( pFriend->dwTitleID ) )
    {
        bIsInRevolt = TRUE;
        swprintf( strFriendTitle, TEXT_TABLE(TEXT_REVOLT) );
    }
    else
    {
        bIsInRevolt = FALSE;
        //$TODO: Need to use correct language here
        g_FriendsManager.GetFriendTitleName( pFriend, 
                                             XC_LANGUAGE_ENGLISH, 
                                             MAX_TITLENAME_LEN, 
                                             strFriendTitle );
    }

    // This follows the same priority order as the icons
    if( pFriend->dwFriendState & XONLINE_FRIENDSTATE_FLAG_RECEIVEDINVITE )
    {
        swprintf( str, TEXT_TABLE(TEXT_FRIENDS_RECEIVEDINVITATION), strFriendTitle );
    }
    else if( pFriend->dwFriendState & XONLINE_FRIENDSTATE_FLAG_SENTINVITE )
    {
        if( pFriend->dwFriendState & XONLINE_FRIENDSTATE_FLAG_INVITEACCEPTED )
            swprintf( str, TEXT_TABLE(TEXT_FRIENDS_INVITATIONACCEPTED) );
        else if( pFriend->dwFriendState & XONLINE_FRIENDSTATE_FLAG_INVITEREJECTED )
            swprintf( str, TEXT_TABLE(TEXT_FRIENDS_INVITATIONDECLINED) );
        else
            swprintf( str, TEXT_TABLE(TEXT_FRIENDS_SENTINVITATION) );
    }
    else if( pFriend->dwFriendState & XONLINE_FRIENDSTATE_FLAG_RECEIVEDREQUEST )
    {
        swprintf( str, TEXT_TABLE(TEXT_FRIENDS_RECEIVEDFRIENDREQUEST) );
    }
    else if( pFriend->dwFriendState & XONLINE_FRIENDSTATE_FLAG_SENTREQUEST )
    {
        swprintf( str, TEXT_TABLE(TEXT_FRIENDS_SENTFRIENDREQUEST) );
    }
    else if( pFriend->dwFriendState & XONLINE_FRIENDSTATE_FLAG_ONLINE )
    {
        if( IsFriendJoinable( pFriend ) )
            swprintf( str, TEXT_TABLE(TEXT_FRIENDS_JOINABLE), strFriendTitle );
        else if( pFriend->dwFriendState & XONLINE_FRIENDSTATE_FLAG_PLAYING )
            swprintf( str, TEXT_TABLE(TEXT_FRIENDS_PLAYING), strFriendTitle );
        else
            swprintf( str, TEXT_TABLE(TEXT_FRIENDS_ONLINE), strFriendTitle );
    }
    else
    {
        swprintf( str, TEXT_TABLE(TEXT_FRIENDS_OFFLINE), pFriend->szGamertag );
    }

    DrawMenuText( pMenuHeader->m_XPos,
                  pMenuHeader->m_YPos + (DWORD)pMenuItem->Data,
                  MENU_TEXT_RGB_NORMAL,
                  str );

}




//-----------------------------------------------------------------------------
// Name: CreateEnumeratingFriendsMenu()
// Desc: Creation function to create a placeholder menu while we're
//          enumerating friends
//-----------------------------------------------------------------------------
void CreateEnumeratingFriendsMenu( MENU_HEADER *pMenuHeader, MENU* pMenu )
{
    pMenuHeader->AddMenuItem( TEXT_FRIENDS_ENUMERATING, 0 );
}




//-----------------------------------------------------------------------------
// Name: CreateFriendHandleInviteMenu()
// Desc: Creation function for the "handle invitation" menu
//-----------------------------------------------------------------------------
void CreateFriendHandleInviteMenu( MENU_HEADER *pMenuHeader, MENU* pMenu )
{
    // MenuItem_FriendDetails uses the Data field as a Y offset
    MenuItem_FriendDetails.Data = 0;
    pMenuHeader->AddMenuItem( &MenuItem_FriendDetails, MENU_ITEM_INACTIVE );
    pMenuHeader->AddMenuItem( TEXT_NONE, MENU_ITEM_INACTIVE );

    pMenuHeader->AddMenuItem( TEXT_FRIENDS_ACCEPTINVITATION );
    pMenuHeader->AddMenuItem( TEXT_FRIENDS_DECLINEINVITATION );

    pMenuHeader->AddMenuItem( TEXT_TABLE(TEXT_BUTTON_A_SELECT_B_BACK), MENU_ITEM_ACTIVE );

    pMenu->CurrentItemIndex = 2;
}




//-----------------------------------------------------------------------------
// Name: HandleFriendHandleInviteMenu()
// Desc: Handles inputs for the friend action menu
//-----------------------------------------------------------------------------
BOOL HandleFriendHandleInviteMenu( MENU_HEADER* pMenuHeader, DWORD dwInput )
{
    XONLINE_FRIEND* pFriend = g_FriendsStateEngine.m_pFriend;
    assert( pFriend->dwFriendState & XONLINE_FRIENDSTATE_FLAG_RECEIVEDINVITE );

    switch( dwInput )
    {
        case MENU_INPUT_UP:
            return SelectPreviousMenuItem( pMenuHeader, pMenuHeader->m_pMenu, NULL );

        case MENU_INPUT_DOWN:
            return SelectNextMenuItem( pMenuHeader, pMenuHeader->m_pMenu, NULL );

        case MENU_INPUT_BACK:
            g_FriendsStateEngine.NextState( CFriendsStateEngine::FRIENDS_STATE_MAINLOOP );
            pMenuHeader->SetNextMenu( &Menu_Friends );
            return TRUE;

        case MENU_INPUT_SELECT:
            // Handle the selected action
            if( pMenuHeader->m_pCurrentItem->TextIndex == TEXT_FRIENDS_ACCEPTINVITATION )
            {
                if( IsServer() && IsInWaitingRoom() )
                {
                    g_FriendsStateEngine.NextState( CFriendsStateEngine::FRIENDS_STATE_CONFIRM_ACCEPT );
                    g_ShowSimpleMessage.Begin( TEXT_TABLE(TEXT_FRIENDS_ACCEPTINVITATION ), 
                                               TEXT_TABLE(TEXT_FRIENDS_END_SESSION),
                                               TEXT_TABLE(TEXT_BUTTON_A_YES),
                                               TEXT_TABLE(TEXT_BUTTON_B_NO) );
                }
                else
                {
                    // Accept the game invitation
                    g_FriendsStateEngine.JoinFriend( pFriend, TRUE );
                }
            }
            else if( pMenuHeader->m_pCurrentItem->TextIndex == TEXT_FRIENDS_DECLINEINVITATION )
            {
                // Decline the game invitation
                g_FriendsManager.AnswerGameInvite( g_FriendsStateEngine.m_dwUserIndex,
                                                   pFriend,
                                                   XONLINE_GAMEINVITE_NO );
                g_FriendsStateEngine.NextState( CFriendsStateEngine::FRIENDS_STATE_MAINLOOP );
                pMenuHeader->SetNextMenu( &Menu_FriendAction );
            }

            return TRUE;
    }

    return FALSE;
}




//-----------------------------------------------------------------------------
// Name: CreateFriendHandleRequestMenu()
// Desc: Creation function for the "handle invitation" menu
//-----------------------------------------------------------------------------
void CreateFriendHandleRequestMenu( MENU_HEADER *pMenuHeader, MENU* pMenu )
{
    // MenuItem_FriendDetails uses the Data field as a Y offset
    MenuItem_FriendDetails.Data = 0;
    pMenuHeader->AddMenuItem( &MenuItem_FriendDetails, MENU_ITEM_INACTIVE );
    pMenuHeader->AddMenuItem( TEXT_NONE, MENU_ITEM_INACTIVE );

    pMenuHeader->AddMenuItem( TEXT_FRIENDS_ACCEPTREQUEST );
    pMenuHeader->AddMenuItem( TEXT_FRIENDS_DECLINEREQUEST );
    pMenuHeader->AddMenuItem( TEXT_FRIENDS_BLOCKREQUESTSFROMPLAYER );

    pMenuHeader->AddMenuItem( TEXT_TABLE(TEXT_BUTTON_A_SELECT_B_BACK), MENU_ITEM_ACTIVE );
    pMenu->CurrentItemIndex = 2;
}



//-----------------------------------------------------------------------------
// Name: HandleFriendHandleRequestMenu()
// Desc: Handles inputs for the friend action menu
//-----------------------------------------------------------------------------
BOOL HandleFriendHandleRequestMenu( MENU_HEADER* pMenuHeader, DWORD dwInput )
{
    XONLINE_FRIEND* pFriend = g_FriendsStateEngine.m_pFriend;
    assert( pFriend->dwFriendState & XONLINE_FRIENDSTATE_FLAG_RECEIVEDREQUEST );

    switch( dwInput )
    {
        case MENU_INPUT_UP:
            return SelectPreviousMenuItem( pMenuHeader, pMenuHeader->m_pMenu, NULL );

        case MENU_INPUT_DOWN:
            return SelectNextMenuItem( pMenuHeader, pMenuHeader->m_pMenu, NULL );

        case MENU_INPUT_BACK:
            g_FriendsStateEngine.NextState( CFriendsStateEngine::FRIENDS_STATE_MAINLOOP );
            pMenuHeader->SetNextMenu( &Menu_Friends );
            return TRUE;

        case MENU_INPUT_SELECT:
            // Handle the selected action
            if( pMenuHeader->m_pCurrentItem->TextIndex == TEXT_FRIENDS_ACCEPTREQUEST )
            {
                // Accept the friends request
                g_FriendsManager.AnswerFriendRequest( g_FriendsStateEngine.m_dwUserIndex, 
                                                      pFriend,
                                                      XONLINE_REQUEST_YES );
            }
            else if( pMenuHeader->m_pCurrentItem->TextIndex == TEXT_FRIENDS_DECLINEREQUEST )
            {
                // Decline the friends request
                g_FriendsManager.AnswerFriendRequest( g_FriendsStateEngine.m_dwUserIndex, 
                                                      pFriend,
                                                      XONLINE_REQUEST_NO );
            }
            else if( pMenuHeader->m_pCurrentItem->TextIndex == TEXT_FRIENDS_BLOCKREQUESTSFROMPLAYER )
            {
                // Block all friends request from this guy
                g_FriendsManager.AnswerFriendRequest( g_FriendsStateEngine.m_dwUserIndex, 
                                                      pFriend,
                                                      XONLINE_REQUEST_BLOCK );

                swprintf( g_FriendsStateEngine.m_strMessage, TEXT_TABLE(TEXT_FRIENDS_REQUEST_FROM_BLOCKED), pFriend->szGamertag );
                g_ShowSimpleMessage.Begin( TEXT_TABLE(TEXT_FRIENDS_BLOCKREQUESTSFROMPLAYER),
                                     g_FriendsStateEngine.m_strMessage,
                                     NULL,
                                     TEXT_TABLE(TEXT_BUTTON_B_BACK) );
            }

            g_FriendsStateEngine.NextState( CFriendsStateEngine::FRIENDS_STATE_MAINLOOP );
            pMenuHeader->SetNextMenu( &Menu_FriendAction );
            return TRUE;
    }

    return FALSE;
}




//-----------------------------------------------------------------------------
// Name: CreateFriendActionMenu()
// Desc: Creation function for the Friend Action menu
//-----------------------------------------------------------------------------
void CreateFriendActionMenu(MENU_HEADER *pMenuHeader, MENU *pMenu)
{
    XONLINE_FRIEND* pFriend = g_FriendsStateEngine.m_pFriend;

    // MenuItem_FriendDetails uses the Data field as a Y offset
    MenuItem_FriendDetails.Data = 0;
    pMenuHeader->AddMenuItem( &MenuItem_FriendDetails, MENU_ITEM_INACTIVE );
    pMenuHeader->AddMenuItem( TEXT_NONE, MENU_ITEM_INACTIVE );

    pMenu->CurrentItemIndex = 2;

    // Only allow invitations to be sent when in a play live waiting room
    // Also don't allow invitations to players who are currently in the game
    // Also don't allow invitations to friends who have not accepted our
    // request yet
    if( IsInWaitingRoom() &&
        gTitleScreenVars.bUseXOnline &&
        PlayerIndexFromXUID( g_FriendsStateEngine.m_pFriend->xuid ) == -1 &&
        !( pFriend->dwFriendState & XONLINE_FRIENDSTATE_FLAG_SENTREQUEST ) )
    {
        pMenuHeader->AddMenuItem( TEXT_FRIENDS_SENDINVITATION  );
    }

    if( ( pFriend->dwFriendState & XONLINE_FRIENDSTATE_FLAG_SENTINVITE ) &&
        !( pFriend->dwFriendState & XONLINE_FRIENDSTATE_FLAG_INVITEACCEPTED ) &&
        !( pFriend->dwFriendState & XONLINE_FRIENDSTATE_FLAG_INVITEREJECTED ) )
    {
        pMenuHeader->AddMenuItem( TEXT_FRIENDS_REVOKEINVITATION );
    }

    // We can join a friend if we're not already in a session with them, and
    // if they say they're joinable
    if( IsFriendJoinable( pFriend ) )
    {
        pMenuHeader->AddMenuItem( TEXT_FRIENDS_JOINFRIEND );
    }

    pMenuHeader->AddMenuItem( TEXT_FRIENDS_REMOVEFRIEND );

    pMenuHeader->AddMenuItem( TEXT_TABLE(TEXT_BUTTON_A_SELECT_B_BACK), MENU_ITEM_ACTIVE );
}




//-----------------------------------------------------------------------------
// Name: HandleFriendActionMenu()
// Desc: Handles inputs for the friend action menu
//-----------------------------------------------------------------------------
BOOL HandleFriendActionMenu( MENU_HEADER* pMenuHeader, DWORD dwInput )
{
    XONLINE_FRIEND* pFriend = g_FriendsStateEngine.m_pFriend;

    switch( dwInput )
    {
        case MENU_INPUT_UP:
            return SelectPreviousMenuItem( pMenuHeader, pMenuHeader->m_pMenu, NULL );

        case MENU_INPUT_DOWN:
            return SelectNextMenuItem( pMenuHeader, pMenuHeader->m_pMenu, NULL );

        case MENU_INPUT_BACK:
            g_FriendsStateEngine.NextState( CFriendsStateEngine::FRIENDS_STATE_MAINLOOP );
            pMenuHeader->SetNextMenu( &Menu_Friends );
            return TRUE;

        case MENU_INPUT_SELECT:
            // Handle the selected action

            if( pMenuHeader->m_pCurrentItem->TextIndex == TEXT_FRIENDS_REMOVEFRIEND )
            {
                g_FriendsStateEngine.NextState( CFriendsStateEngine::FRIENDS_STATE_CONFIRM_REMOVE );
                g_ShowSimpleMessage.Begin( TEXT_TABLE(TEXT_FRIENDS_REMOVEFRIEND), 
                                           TEXT_TABLE(TEXT_FRIENDS_REMOVE_CONFIRM),
                                           TEXT_TABLE(TEXT_BUTTON_A_YES),
                                           TEXT_TABLE(TEXT_BUTTON_B_NO) );
                return TRUE;
            }
            else if( pMenuHeader->m_pCurrentItem->TextIndex == TEXT_FRIENDS_SENDINVITATION )
            {
                // Send a game invitiation
                g_FriendsManager.SendGameInvite( g_FriendsStateEngine.m_dwUserIndex,
                                                 SessionCurr.keyID, 
                                                 pFriend );
            }
            else if( pMenuHeader->m_pCurrentItem->TextIndex == TEXT_FRIENDS_REVOKEINVITATION )
            {
                // Revoke game invitiation
                g_FriendsManager.RevokeGameInvite( g_FriendsStateEngine.m_dwUserIndex,
                                                   SessionCurr.keyID, 
                                                   pFriend );
            }
            else if( pMenuHeader->m_pCurrentItem->TextIndex == TEXT_FRIENDS_JOINFRIEND )
            {
                if( IsServer() && IsInWaitingRoom() )
                {
                    g_FriendsStateEngine.NextState( CFriendsStateEngine::FRIENDS_STATE_CONFIRM_JOIN );
                    g_ShowSimpleMessage.Begin( TEXT_TABLE(TEXT_FRIENDS_JOINFRIEND ), 
                                               TEXT_TABLE(TEXT_FRIENDS_END_SESSION),
                                               TEXT_TABLE(TEXT_BUTTON_A_YES),
                                               TEXT_TABLE(TEXT_BUTTON_B_NO) );
                    return TRUE;
                }
                else
                {
                    // Accept the game invitation
                    g_FriendsStateEngine.JoinFriend( pFriend, FALSE );
                }
            }

            g_FriendsStateEngine.NextState( CFriendsStateEngine::FRIENDS_STATE_MAINLOOP );
            pMenuHeader->SetNextMenu( &Menu_Friends );
            return TRUE;
    }
    return FALSE;
}



//-----------------------------------------------------------------------------
// The Friends state engine
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Name: CheckForFriendsUpdate()
// Desc: Pumps the enumeration task and handles any updates
//-----------------------------------------------------------------------------
HRESULT CFriendsStateEngine::CheckForFriendsUpdate()
{
    HRESULT hr = S_OK;

    if( g_FriendsManager.HasFriendsListChanged( m_dwUserIndex ) )
    {
        for( DWORD i = 0; i < g_FriendsManager.GetNumFriends( m_dwUserIndex ); i++ )
        {
            m_amiFriends[i].TextIndex = TEXT_NONE;
            m_amiFriends[i].Data = g_FriendsManager.GetFriend( m_dwUserIndex, i );
            m_amiFriends[i].DataWidth = 250.0f;
            m_amiFriends[i].DrawFunc = DrawFriendsMenuItem;
            m_amiFriends[i].ActiveFlags = MENU_ITEM_ACTIVE | MENU_ITEM_SELECTABLE;
        }

        //$HACK(Apr02_GameBash) - default back to first page
        m_dwCurrentPage = 0;

        hr = S_FALSE;
    }

    return hr;
}



//-----------------------------------------------------------------------------
// Nane: JoinFriend
// Desc: Handles joining the specified friend's game session
//-----------------------------------------------------------------------------
HRESULT CFriendsStateEngine::JoinFriend( XONLINE_FRIEND* pFriend, BOOL bInvited )
{
    if( bInvited )
    {
        // First, answer the invitation
        g_FriendsManager.AnswerGameInvite( g_FriendsStateEngine.m_dwUserIndex,
                                           pFriend,
                                           XONLINE_GAMEINVITE_YES );
    }

    if( !IsZeroXNKID( pFriend->sessionID ) )
    {
        // If the invitation is coming from someone in Re-Volt, go ahead and
        // accept the invite
        if( XOnlineTitleIdIsSameTitle( pFriend->dwTitleID ) )
        {
            // Look up our friend's game session
            g_bXOnlineSessionSearchComplete = FALSE;
            XONLINETASK_HANDLE hFindSessionTask;
            XOnlineMatchSessionFindFromID( pFriend->sessionID,
                                           NULL,
                                           &hFindSessionTask );
            OnlineTasks_Add( hFindSessionTask, TASK_MATCH_SEARCH );

            // Quit our current game, if any
            if( IsInGameSession() )
            {
                // Paraphrased from ui_InGameMenu.cpp
                if( IsServer() ) { DestroySession(); }
                if( IsClient() )
                {
                    LeaveSession();
                }
            }

            g_bInvitedByFriend = TRUE;
            g_FriendsManager.StopUpdatingFriends( m_dwUserIndex );

            //$TODO: should verify (for client AND server) that this is the only way to manually end session.
            /// If there are other ways to end session, need to insert similar code at those locations.
            SET_EVENT(GoTitleScreen);
            LEV_EndLevelStageTwo();
            LEV_EndLevelStageOne();

            // Join the match now
            // g_FriendsStateEngine.NextState( CFriendsStateEngine::FRIENDS_STATE_EXITING );
            g_FriendsStateEngine.Return( STATEENGINE_TERMINATED );
            g_pMenuHeader->SetNextMenu(NULL);
            //$HACK(Apr02_GameBash) - if we back out of join match, we need to end up at top level
            //$TODO - mwetzel - this seems both incorrect and not safe
            g_TopLevelMenuStateEngine.MakeActive( NULL );
            g_pActiveStateEngine->Call( &g_JoinMatchStateEngine );
        }
        else
        {
            // Otherwise, tell the user to insert that disc
            WCHAR strFriendTitle[ MAX_TITLENAME_LEN ];

            g_FriendsManager.GetFriendTitleName( pFriend,
                                                 XC_LANGUAGE_ENGLISH,
                                                 MAX_TITLENAME_LEN,
                                                 strFriendTitle );

            swprintf( g_SimpleMessageBuffer, TEXT_TABLE(TEXT_FRIENDS_INSERT_GAMEDISK), strFriendTitle );
            g_ShowSimpleMessage.Begin( TEXT_TABLE(TEXT_FRIENDS_INVITE_ACCEPTED),
                                       g_SimpleMessageBuffer,
                                       NULL,
                                       TEXT_TABLE(TEXT_BUTTON_B_BACK) );
        }
    }
    else
    {
        g_ShowSimpleMessage.Begin( TEXT_TABLE(TEXT_FRIENDS),
                                   TEXT_TABLE(TEXT_FRIENDS_BADSESSIONID),
                                   NULL,
                                   TEXT_TABLE(TEXT_BUTTON_B_BACK) );
        g_FriendsStateEngine.NextState( CFriendsStateEngine::FRIENDS_STATE_MAINLOOP );
        g_pMenuHeader->SetNextMenu( &Menu_FriendAction );
    }

    return S_OK;
}



//-----------------------------------------------------------------------------
// Name: SyncCurrentFriend
// Desc: Synchronizes our m_pFriend pointer after a friends list update.
//-----------------------------------------------------------------------------
VOID CFriendsStateEngine::SyncCurrentFriend()
{
    // If m_pFriend was already NULL, we don't have anything to do.
    if( !m_pFriend )
    {
        assert( m_CurrentFriend.xuid.qwUserID == 0 );
        return;
    }

    if( !XOnlineAreUsersIdentical( &m_pFriend->xuid, &m_CurrentFriend.xuid ) )
    {
        // m_pFriend isn't pointing to the right guy anymore.  See if he's
        // still in our friends list
        m_pFriend = g_FriendsManager.FindPlayerInFriendsList( m_dwUserIndex, m_CurrentFriend.xuid );
        if( !m_pFriend )
        {
            //$REVISIT: This probably shouldn't be a warning
            DumpMessage( "Warning", "Currently selected friend went away." );
            ZeroMemory( &m_CurrentFriend, sizeof( XONLINE_FRIEND ) );
        }
    }
}



//-----------------------------------------------------------------------------
// Name: Process()
// Desc: Main function for the Friends State Engine.
//       * BEGIN: Initialize online icon textures the first time through
//                Kick off the enumeration task, and move to appropriate state
//       * ENUMERATING: Wait until we have results available, then populate
//                          the menu items with our friends
//       * FRIENDACTION: Nothing special
//       * MAINLOOP: Check for updates of the friends list, and repopulate
//       * ENUM_FINSH: Finish the enumeration task, then go to EXITING
//       * EXITING: Wait for enumeration task to complete, then close and
//                      return
//-----------------------------------------------------------------------------
HRESULT CFriendsStateEngine::Process()
{
    BOOL    bFriendsChanged = FALSE;

    switch( m_State )
    {
        case FRIENDS_STATE_BEGIN:

            // BUGBUG (JHarding): This needs to be the correct player index
            // when we have multiple people on one box
            //$HACK: We're always telling the online APIs the user signed in on controller 0.
            // m_dwUserIndex = g_dwSignedInController;
            m_dwUserIndex = 0;

            // Set up the array of friends
            ZeroMemory( m_amiFriends, sizeof( m_amiFriends ) );
            m_pFriend = NULL;
            ZeroMemory( &m_CurrentFriend, sizeof( XONLINE_FRIEND ) );
            if( SUCCEEDED( g_FriendsManager.StartUpdatingFriends( m_dwUserIndex ) ) )
            {
                g_pMenuHeader->SetNextMenu( &Menu_EnumeratingFriends );
                m_State = FRIENDS_STATE_ENUMERATING;
            }
            else
            {
                g_pMenuHeader->SetNextMenu( &Menu_Friends );
                m_State = FRIENDS_STATE_MAINLOOP;
            }
            break;

        case FRIENDS_STATE_ENUMERATING:
            // Wait until we get results
            if( S_FALSE == CheckForFriendsUpdate() )
            {
                g_pMenuHeader->SetNextMenu( &Menu_Friends );
                m_State = FRIENDS_STATE_MAINLOOP;
                // No need to call SyncCurrentFriend, because we don't have one
                assert( m_pFriend == NULL );
            }

            break;

        case FRIENDS_STATE_FRIENDACTION:
            if( S_FALSE == CheckForFriendsUpdate() )
            {
                // Friend list may have changed - re-synchronize
                bFriendsChanged = TRUE;
                assert( m_pFriend != NULL );
                SyncCurrentFriend();
            }

            // If friend is still valid, see if we need to handle a pending
            // request or invitation.  If not, go to the action menu
            if( m_pFriend )
            {
                if( m_pFriend->dwFriendState & XONLINE_FRIENDSTATE_FLAG_RECEIVEDINVITE )
                {
                    g_FriendsStateEngine.NextState( CFriendsStateEngine::FRIENDS_STATE_HANDLEINVITE );
                    g_pMenuHeader->SetNextMenu( &Menu_FriendHandleInvite );
                }
                else if( m_pFriend->dwFriendState & XONLINE_FRIENDSTATE_FLAG_RECEIVEDREQUEST )
                {
                    g_FriendsStateEngine.NextState( CFriendsStateEngine::FRIENDS_STATE_HANDLEREQUEST );
                    g_pMenuHeader->SetNextMenu( &Menu_FriendHandleRequest );
                }
                else if( g_pMenuHeader->m_pMenu != &Menu_FriendAction ||
                         bFriendsChanged )
                {
                    g_pMenuHeader->SetNextMenu( &Menu_FriendAction );
                }
            }
            else
            {
                // Either we removed our friend or they removed us.  Go back
                // to friends menu
                m_State = FRIENDS_STATE_MAINLOOP;
                g_pMenuHeader->SetNextMenu( &Menu_Friends );
            }

            break;

        case FRIENDS_STATE_HANDLEINVITE:
            break;

        case FRIENDS_STATE_HANDLEREQUEST:
            break;

        case FRIENDS_STATE_CONFIRM_REMOVE:
            // We should only end up here after the pop-up confirm dialog 
            // is complete
            if( g_ShowSimpleMessage.GetStatus() == STATEENGINE_TERMINATED )
            {
                // They backed out - do nothing
                NextState( FRIENDS_STATE_FRIENDACTION );
            }
            else
            {
                // Remove friend from friends list
                g_FriendsManager.RemoveFriendFromFriendsList( m_dwUserIndex, m_pFriend );
                NextState( CFriendsStateEngine::FRIENDS_STATE_MAINLOOP );
                g_pMenuHeader->SetNextMenu( &Menu_Friends );
            }
            break;

        case FRIENDS_STATE_CONFIRM_ACCEPT:
            // We should only end up here after the pop-up confirm dialog 
            // is complete
            if( g_ShowSimpleMessage.GetStatus() == STATEENGINE_TERMINATED )
            {
                // They backed out - do nothing
                NextState( FRIENDS_STATE_FRIENDACTION );
            }
            else
            {
                // Accept the game invitation
                g_FriendsStateEngine.JoinFriend( m_pFriend, TRUE );
            }
            break;

        case FRIENDS_STATE_CONFIRM_JOIN:
            // We should only end up here after the pop-up confirm dialog 
            // is complete
            if( g_ShowSimpleMessage.GetStatus() == STATEENGINE_TERMINATED )
            {
                // They backed out - do nothing
                NextState( FRIENDS_STATE_FRIENDACTION );
            }
            else
            {
                // Accept the game invitation
                g_FriendsStateEngine.JoinFriend( m_pFriend, FALSE );
            }
            break;

        case FRIENDS_STATE_MAINLOOP:
            if( S_FALSE == CheckForFriendsUpdate() )
            {
                SyncCurrentFriend();

                // Force re-creation of the friends menu
                g_pMenuHeader->SetNextMenu( NULL );
                g_pMenuHeader->SetNextMenu( &Menu_Friends );
            }
            break;
    }

    // Handle the menus
    g_pMenuHeader->HandleMenus();

    return S_FALSE;
}



VOID RevokeAllInvites()
{
    assert( IsLoggedIn(0) ); //$SINGLEPLAYER - Assuming single local player

    //$HACK: We're always telling the online APIs the user signed in on controller 0.
    g_FriendsManager.RevokeGameInvite( 0,
                                       SessionCurr.keyID,
                                       NULL );
}
