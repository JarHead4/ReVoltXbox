//-----------------------------------------------------------------------------
// File: ui_Players.cpp
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
#include "ui_players.h"
#include "ui_showmessage.h"
#include "ui_friends.h"
#include "net_xonline.h"
#include "text.h"
#include "VoiceManager.h"
#include "FriendsManager.h"

// use this define to skip xonline apis when testnet is dead
//#define XONLINE_OFFLINE


CPlayersStateEngine g_PlayersStateEngine;
const DWORD NUM_TALKICONCYCLE = 4;
D3DTexture**    g_TalkIcons[ NUM_TALKICONCYCLE ] =
{
    &g_pTalking0Texture,
    &g_pTalking1Texture,
    &g_pTalking2Texture,
    &g_pTalking3Texture,
};



//-----------------------------------------------------------------------------
// Menu functions
//-----------------------------------------------------------------------------
#define MENU_PLAYERS_XPOS                0
#define MENU_PLAYERS_YPOS              120

void CreatePlayersMenu(MENU_HEADER *pMenuHeader, MENU *pMenu);
BOOL HandlePlayersMenu( MENU_HEADER* pMenuHeader, DWORD dwInput );
void DrawPlayersMenu( MENU_HEADER* pMenuHeader, MENU* pMenu );
void DrawPlayersMenuItem( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem, int itemIndex );

void CreatePlayerActionMenu(MENU_HEADER *pMenuHeader, MENU *pMenu);
BOOL HandlePlayerActionMenu( MENU_HEADER* pMenuHeader, DWORD dwInput );

void CreateSendFeedbackMenu(MENU_HEADER *pMenuHeader, MENU *pMenu);
BOOL HandleSendFeedbackMenu( MENU_HEADER* pMenuHeader, DWORD dwInput );
void DrawNicknameAndGamertag( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem, int itemIndex );



//-----------------------------------------------------------------------------
// Menu structures
//-----------------------------------------------------------------------------

// Players menu
extern MENU Menu_Players = 
{
    TEXT_PLAYERS,
    MENU_DEFAULT | MENU_CENTRE_X | MENU_NOBOX,           // Menu type
    CreatePlayersMenu,                      // Create menu function
    HandlePlayersMenu,                      // Input handler function
    DrawPlayersMenu,                        // Menu draw function
    MENU_PLAYERS_XPOS,                      // X coord
    MENU_PLAYERS_YPOS,
};


// PlayerAction menu
extern MENU Menu_PlayerAction = 
{
    TEXT_NONE,
    MENU_DEFAULT | MENU_CENTRE_X,           // Menu type
    CreatePlayerActionMenu,                 // Create menu function
    HandlePlayerActionMenu,                 // Input handler function
    NULL,                                   // Menu draw function
    MENU_PLAYERS_XPOS,                      // X coord
    MENU_PLAYERS_YPOS,
};


// Send Feedback menu
extern MENU Menu_SendFeedback = 
{
    TEXT_PLAYERS_SENDFEEDBACK,
    MENU_DEFAULT | MENU_CENTRE_X,           // Menu type
    CreateSendFeedbackMenu,                 // Create menu function
    HandleSendFeedbackMenu,                 // Input handler function
    NULL,                                   // Menu draw function
    MENU_PLAYERS_XPOS,                      // X coord
    MENU_PLAYERS_YPOS,
};

static MENU_ITEM MenuItem_NicknameAndGamertag =
{
    TEXT_NONE,
    100.0f,
    NULL,
    DrawNicknameAndGamertag,
};



// Create Function
static const LONG PLAYERS_ON_SCREEN = 5;
//-----------------------------------------------------------------------------
// Name: CreatePlayersMenu()
// Desc: 
//-----------------------------------------------------------------------------
void CreatePlayersMenu(MENU_HEADER *pMenuHeader, MENU *pMenu)
{
    // pMenuHeader->m_ItemTextWidth = g_pFont->GetTextWidth( TEXT_TABLE(TEXT_BUTTON_A_SELECT_B_BACK) );

    if( PlayerCount <= 1 && DepartedPlayerList.size() == 0 )
    {
        pMenuHeader->AddMenuItem( TEXT_TABLE(TEXT_PLAYERS_NOOTHERPLAYERS), MENU_ITEM_ACTIVE );
    }
    else
    {
        for( DWORD i = 0; i < (DWORD)PlayerCount + DepartedPlayerList.size() - 1; i++ )
        {
            pMenuHeader->AddMenuItem( &g_PlayersStateEngine.m_amiPlayers[i] );
        }
    }

    // Make sure we default to the first menu item - if the last player's
    // menu had more entries than this one, CurrentItemIndex could be off
    // the end of the list.  Freakin' menu code...
    pMenu->CurrentItemIndex = 0;
}




//-----------------------------------------------------------------------------
// Name: HandlePlayersMenu()
// Desc: 
//-----------------------------------------------------------------------------
BOOL HandlePlayersMenu( MENU_HEADER* pMenuHeader, DWORD dwInput )
{
    
    switch( dwInput )
    {
        case MENU_INPUT_UP:
            if( pMenuHeader->m_pMenu->CurrentItemIndex > 0 )
                return SelectPreviousMenuItem( pMenuHeader, pMenuHeader->m_pMenu, NULL );
            break;

        case MENU_INPUT_DOWN:
            if( pMenuHeader->m_pMenu->CurrentItemIndex < pMenuHeader->m_dwNumMenuItems - 1 )
                return SelectNextMenuItem( pMenuHeader, pMenuHeader->m_pMenu, NULL );
            break;

        case MENU_INPUT_SELECT:
            if( PlayerCount + DepartedPlayerList.size() > 1 )
            {
                g_PlayersStateEngine.m_dwSelectedPlayer = pMenuHeader->m_pMenu->CurrentItemIndex;
                g_PlayersStateEngine.NextState( CPlayersStateEngine::PLAYERS_STATE_PLAYERACTION );
                pMenuHeader->SetNextMenu( &Menu_PlayerAction );
                return TRUE;
            }
            break;

        case MENU_INPUT_BACK:
            //$HACK: Spoofing controller 0 for consumer beta
            g_FriendsManager.StopUpdatingFriends( 0 );
            g_PlayersStateEngine.Return( STATEENGINE_TERMINATED );
            return TRUE;
    }

    return FALSE;
}


static const FLOAT STATUS_WIDTH     = 80.0f;
static const FLOAT NICKNAME_WIDTH   = 200.0f;
static const FLOAT GAMERTAG_WIDTH   = 200.0f;
static const FLOAT PLAYER_HEIGHT    = MENU_TEXT_HEIGHT * 2.0f + MENU_TEXT_VSKIP;
//-----------------------------------------------------------------------------
// Name: DrawPlayersMenu()
// Desc: Handles drawing the players menu.  There will be up to 
//      PLAYERS_ON_SCREEN players on the screen at once, and we'll scroll
//      between them as necessary.  In addition, there are two more lines
//      in the menu - one for selected player status, one for the "A select"
//      message
//-----------------------------------------------------------------------------
VOID DrawPlayersMenu( MENU_HEADER* pMenuHeader, MENU* pMenu )
{
    FLOAT x = pMenuHeader->m_XPos - (MENU_TEXT_WIDTH*2);
    FLOAT y = pMenuHeader->m_YPos - (MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP);
    DWORD dwEntryCount = max( 2, PlayerCount - 1 + DepartedPlayerList.size() );
    DWORD dwOnScreen = min( PLAYERS_ON_SCREEN, dwEntryCount );

    DrawNewSpruBox( x, y, 
                    pMenuHeader->m_XSize+(MENU_TEXT_WIDTH*4), 
                    ( PLAYER_HEIGHT * ( dwOnScreen + 4 ) ) );

    BeginTextState();

    if( PlayerCount + DepartedPlayerList.size() > 1 )
    {
        // Only draw column headers if there's a player there
        DrawMenuText( pMenuHeader->m_XPos,
                      pMenuHeader->m_YPos,
                      MENU_TEXT_RGB_NORMAL,
                      TEXT_TABLE(TEXT_STATUS) );
        DrawMenuText( pMenuHeader->m_XPos + STATUS_WIDTH,
                      pMenuHeader->m_YPos,
                      MENU_TEXT_RGB_NORMAL,
                      TEXT_TABLE(TEXT_ENTERNAME_NICKNAME) );
        DrawMenuText( pMenuHeader->m_XPos + STATUS_WIDTH + NICKNAME_WIDTH,
                      pMenuHeader->m_YPos,
                      MENU_TEXT_RGB_NORMAL,
                      TEXT_TABLE(TEXT_GAMERTAG) );

        DrawMenuText( pMenuHeader->m_XPos, 
                      pMenuHeader->m_YPos + ( PLAYER_HEIGHT * ( dwOnScreen + 2 ) ), 
                      MENU_TEXT_RGB_NORMAL, 
                      TEXT_TABLE(TEXT_BUTTON_A_SELECT_B_BACK) );
    }
    else
    {
        DrawMenuText( pMenuHeader->m_XPos, 
                      pMenuHeader->m_YPos + ( PLAYER_HEIGHT * ( dwOnScreen + 2 ) ), 
                      MENU_TEXT_RGB_NORMAL, 
                      TEXT_TABLE(TEXT_BUTTON_B_BACK) );
    }


    if( g_PlayersStateEngine.m_lFirstPlayerOnScreen > 0 )
    {
        DrawMenuText( pMenuHeader->m_XPos - MENU_TEXT_WIDTH,
                      pMenuHeader->m_YPos + PLAYER_HEIGHT,
                      MENU_TEXT_RGB_NORMAL,
                      L"^" );
    }
    if( g_PlayersStateEngine.m_lFirstPlayerOnScreen + (LONG)PLAYERS_ON_SCREEN < PlayerCount - 1 + (LONG)DepartedPlayerList.size() )
    {
        DrawMenuText( pMenuHeader->m_XPos - MENU_TEXT_WIDTH,
                      pMenuHeader->m_YPos + ( PLAYER_HEIGHT * ( dwOnScreen ) ),
                      MENU_TEXT_RGB_NORMAL,
                      L"v" );
    }

    // Scroll if necessary
    if( pMenu->CurrentItemIndex < g_PlayersStateEngine.m_lFirstPlayerOnScreen )
        g_PlayersStateEngine.m_lFirstPlayerOnScreen = pMenu->CurrentItemIndex;
    else if( pMenu->CurrentItemIndex >= g_PlayersStateEngine.m_lFirstPlayerOnScreen + PLAYERS_ON_SCREEN )
        g_PlayersStateEngine.m_lFirstPlayerOnScreen = pMenu->CurrentItemIndex - PLAYERS_ON_SCREEN + 1;

    if( PlayerCount + DepartedPlayerList.size() > 1 )
    {
        // Get the NET_PLAYER ptr from the menu item, because
        // m_dwSelectedPlayer can't be used as an index into PlayerList.
        NET_PLAYER* pPlayer = (NET_PLAYER *)g_PlayersStateEngine.m_amiPlayers[ pMenu->CurrentItemIndex ].Data;
        WCHAR strPlayerStatus[256];
        if( pMenu->CurrentItemIndex >= PlayerCount - 1 )
        {
            swprintf( strPlayerStatus, TEXT_TABLE(TEXT_PLAYERS_LEFT_GAME), pPlayer->Name );
        }
        else
        {
            swprintf( strPlayerStatus, TEXT_TABLE(TEXT_PLAYERS_IN_GAME), pPlayer->Name );
        }
        DrawMenuText( pMenuHeader->m_XPos + MENU_TEXT_WIDTH,
                      pMenuHeader->m_YPos + ( PLAYER_HEIGHT * ( dwOnScreen + 1 ) ),
                      MENU_TEXT_RGB_NORMAL,
                      strPlayerStatus );
    }
       
    // We should always have the current selection on screen
    assert( pMenu->CurrentItemIndex >= g_PlayersStateEngine.m_lFirstPlayerOnScreen &&
            pMenu->CurrentItemIndex < g_PlayersStateEngine.m_lFirstPlayerOnScreen + PLAYERS_ON_SCREEN );
}



//-----------------------------------------------------------------------------
// Name: DrawPlayersMenuItem()
// Desc: Handles drawing the menu items on the players menu, including
//          voice icons, talking notification, etc.
//-----------------------------------------------------------------------------
void DrawPlayersMenuItem( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem, int itemIndex )
{
    // Cheap scrolling
    if( itemIndex < g_PlayersStateEngine.m_lFirstPlayerOnScreen ||
        itemIndex >= g_PlayersStateEngine.m_lFirstPlayerOnScreen + PLAYERS_ON_SCREEN )
        return;

    NET_PLAYER* pPlayer = (NET_PLAYER *)pMenuItem->Data;
    DWORD       dwScreenIndex = itemIndex - g_PlayersStateEngine.m_lFirstPlayerOnScreen + 1;
    FLOAT fX = pMenuHeader->m_XPos;
    FLOAT fY = pMenuHeader->m_YPos + PLAYER_HEIGHT * dwScreenIndex;

    // Determine what voice icon to show
    BOOL bIsPlayerTalking = FALSE;
    D3DTexture* pTexture  = NULL;

    //$HACK: Spoofing controller 0
    if( g_FriendsManager.IsPlayerInMuteList( 0, pPlayer->xuid ) )
    {
        pTexture = g_pTalkMutedTexture;
    }
    else if( g_VoiceManager.DoesPlayerHaveVoice( pPlayer->xuid ) )
    {
        if( g_VoiceManager.IsPlayerMuted( pPlayer->xuid, g_dwSignedInController ) )
            pTexture = g_pTalkMutedTexture;
        else
            pTexture = g_pTalkEnabledTexture;

        if( g_VoiceManager.IsPlayerTalking( pPlayer->xuid ) &&
            !g_VoiceManager.IsPlayerMuted( pPlayer->xuid, g_dwSignedInController ) &&
            !g_VoiceManager.IsPlayerRemoteMuted( pPlayer->xuid, g_dwSignedInController ) &&
            g_VoiceManager.IsCommunicatorInserted( g_dwSignedInController ) )
        {
            bIsPlayerTalking = TRUE;
            pTexture = *g_TalkIcons[ g_PlayersStateEngine.m_dwTalkIconIndex ];
        }
    }
    if( pTexture )
    {
        DrawScreenSpaceQuad( fX - MENU_TEXT_WIDTH, fY - 6.0f, pTexture );
    }

    // See if they're in our friend list
    //$HACK: Spoofing controller 0 for consumer beta
    XONLINE_FRIEND* pFriend = g_FriendsManager.FindPlayerInFriendsList( 0, pPlayer->xuid );

    pTexture = NULL;
    if( pFriend != NULL )
    {
        if( pFriend->dwFriendState & XONLINE_FRIENDSTATE_FLAG_RECEIVEDREQUEST )
            pTexture = g_pFriendReqReceivedTexture;
        else if( pFriend->dwFriendState & XONLINE_FRIENDSTATE_FLAG_SENTREQUEST )
            pTexture = g_pFriendReqSentTexture;
        else if( pFriend->dwFriendState & XONLINE_FRIENDSTATE_FLAG_ONLINE )
            pTexture = g_pFriendOnlineTexture;
    }
    if( pTexture )
    {
        DrawScreenSpaceQuad( fX - MENU_TEXT_WIDTH + 40.0f, fY - 6.0f, pTexture );
    }

    WCHAR wstrPlayer[ MAX_PLAYER_NAME + 12 ];
    swprintf( wstrPlayer, L"%S", pPlayer->Name );
    DrawMenuText( fX + STATUS_WIDTH, fY,
                  pMenu->CurrentItemIndex == itemIndex ? MENU_TEXT_RGB_CHOICE : MENU_TEXT_RGB_NORMAL,
                  wstrPlayer );

    WCHAR wstrGamertag[ XONLINE_GAMERTAG_SIZE ];
    swprintf( wstrGamertag, L"%S", pPlayer->GamerTag );
    DrawMenuText( fX + STATUS_WIDTH + NICKNAME_WIDTH, fY,
                  pMenu->CurrentItemIndex == itemIndex ? MENU_TEXT_RGB_CHOICE : MENU_TEXT_RGB_NORMAL,
                  wstrGamertag );

}




//-----------------------------------------------------------------------------
// Name: CreatePlayerActionMenu()
// Desc: Creates and populates the "player action" menu
//-----------------------------------------------------------------------------
void CreatePlayerActionMenu( MENU_HEADER* pMenuHeader, MENU* pMenu )
{
    NET_PLAYER* pPlayer = (NET_PLAYER *)g_PlayersStateEngine.m_amiPlayers[ g_PlayersStateEngine.m_dwSelectedPlayer ].Data;
    XONLINE_FRIEND* pFriend = g_FriendsManager.FindPlayerInFriendsList( 0, pPlayer->xuid );
    DWORD dwOnlineFlags = ( IsLoggedIn(0) && pPlayer->xuid.qwUserID != 0 ) ? MENU_ITEM_ACTIVE | MENU_ITEM_SELECTABLE : MENU_ITEM_INACTIVE;

    if( pFriend )
    {
        // Allow handling of friend requests
        if( pFriend->dwFriendState & XONLINE_FRIENDSTATE_FLAG_RECEIVEDREQUEST )
        {
            pMenuHeader->AddMenuItem( TEXT_FRIENDS_ACCEPTREQUEST );
            pMenuHeader->AddMenuItem( TEXT_FRIENDS_DECLINEREQUEST );
            pMenuHeader->AddMenuItem( TEXT_FRIENDS_BLOCKREQUESTSFROMPLAYER );
        }
        else
        {
            pMenuHeader->AddMenuItem( TEXT_FRIENDS_REMOVEFRIEND );
        }
    }
    else
    {
        pMenuHeader->AddMenuItem( TEXT_PLAYERS_ADDFRIEND, dwOnlineFlags );
    }

    //$REVISIT: Disabling mute control for system link
    if( gTitleScreenVars.bUseXOnline &&
        g_VoiceManager.DoesPlayerHaveVoice( PlayerList[ PlayerIndexFromPlayerID( LocalPlayerID ) ].xuid ) &&
        g_VoiceManager.DoesPlayerHaveVoice( pPlayer->xuid ) )
    {
        if( g_VoiceManager.IsPlayerMuted( pPlayer->xuid, g_dwSignedInController ) )
        {
            pMenuHeader->AddMenuItem( TEXT_PLAYERS_UNMUTEPLAYER );
        }
        else
        {
            pMenuHeader->AddMenuItem( TEXT_PLAYERS_MUTEPLAYER );
        }
    }

    // Only allow feedback if online match
    if( gTitleScreenVars.bUseXOnline )
        pMenuHeader->AddMenuItem( TEXT_PLAYERS_SENDFEEDBACK, dwOnlineFlags );

    pMenuHeader->AddMenuItem( TEXT_TABLE(TEXT_BUTTON_A_SELECT_B_BACK), MENU_ITEM_ACTIVE );
}




//-----------------------------------------------------------------------------
// Name: HandlePlayerActionMenu()
// Desc: Handles input for the "player action" menu.
//-----------------------------------------------------------------------------
BOOL HandlePlayerActionMenu( MENU_HEADER* pMenuHeader, DWORD dwInput )
{
    HRESULT hr;
    NET_PLAYER* pPlayer = (NET_PLAYER *)g_PlayersStateEngine.m_amiPlayers[ g_PlayersStateEngine.m_dwSelectedPlayer ].Data;

    switch( dwInput )
    {
        case MENU_INPUT_UP:
            return SelectPreviousMenuItem( pMenuHeader, pMenuHeader->m_pMenu, NULL );

        case MENU_INPUT_DOWN:
            return SelectNextMenuItem( pMenuHeader, pMenuHeader->m_pMenu, NULL );

        case MENU_INPUT_BACK:
            g_PlayersStateEngine.NextState( CPlayersStateEngine::PLAYERS_STATE_MAINLOOP );
            pMenuHeader->SetNextMenu( &Menu_Players );
            return TRUE;

        case MENU_INPUT_SELECT:
            if( pMenuHeader->m_pCurrentItem->TextIndex == TEXT_PLAYERS_ADDFRIEND )
            {
                hr = g_PlayersStateEngine.AddFriend();
                if( FAILED( hr ) )
                {
                    g_ShowSimpleMessage.Begin( TEXT_TABLE(TEXT_PLAYERS),
                                               TEXT_TABLE(TEXT_PLAYERS_PLAYERNOTVALID), 
                                               NULL,
                                               TEXT_TABLE(TEXT_BUTTON_B_BACK) );
                }
            }
            else if( pMenuHeader->m_pCurrentItem->TextIndex == TEXT_FRIENDS_ACCEPTREQUEST )
            {
                // If the player is no longer a friend (ie, they removed us from their
                // friends list), then this will just silently fail.
                //$HACK: Spoofing controller 0
                g_FriendsManager.AnswerFriendRequest( 0,
                                                      g_FriendsManager.FindPlayerInFriendsList( 0, pPlayer->xuid ),
                                                      XONLINE_REQUEST_YES );
            }
            else if( pMenuHeader->m_pCurrentItem->TextIndex == TEXT_FRIENDS_DECLINEREQUEST )
            {
                // If the player is no longer a friend (ie, they removed us from their
                // friends list), then this will just silently fail.
                //$HACK: Spoofing controller 0
                g_FriendsManager.AnswerFriendRequest( 0,
                                                      g_FriendsManager.FindPlayerInFriendsList( 0, pPlayer->xuid ),
                                                      XONLINE_REQUEST_NO );
            }
            else if( pMenuHeader->m_pCurrentItem->TextIndex == TEXT_FRIENDS_BLOCKREQUESTSFROMPLAYER )
            {
                // If the player is no longer a friend (ie, they removed us from their
                // friends list), then this will just silently fail.
                //$HACK: Spoofing controller 0
                XONLINE_FRIEND* pFriend = g_FriendsManager.FindPlayerInFriendsList( 0, pPlayer->xuid );
                g_FriendsManager.AnswerFriendRequest( 0,
                                                      pFriend,
                                                      XONLINE_REQUEST_BLOCK );

                // Need to do this before activating the simple message engine..
                pMenuHeader->SetNextMenu( &Menu_Players );

                swprintf( g_SimpleMessageBuffer, TEXT_TABLE(TEXT_FRIENDS_REQUEST_FROM_BLOCKED), pFriend->szGamertag );
                g_ShowSimpleMessage.Begin( TEXT_TABLE(TEXT_FRIENDS_BLOCKREQUESTSFROMPLAYER),
                                           g_SimpleMessageBuffer,
                                           NULL,
                                           TEXT_TABLE(TEXT_BUTTON_B_BACK) );
            }
            else if( pMenuHeader->m_pCurrentItem->TextIndex == TEXT_FRIENDS_REMOVEFRIEND )
            {
                //$HACK: Spoofing controller 0
                //$TODO: Need confirmation screen here
                g_FriendsManager.RemoveFriendFromFriendsList( 0, g_FriendsManager.FindPlayerInFriendsList( 0, pPlayer->xuid ) );
            }
            else if( pMenuHeader->m_pCurrentItem->TextIndex == TEXT_PLAYERS_MUTEPLAYER )
            {
                DWORD dwPlayerIndex = PlayerIndexFromXUID(pPlayer->xuid );

                g_VoiceManager.MutePlayer( pPlayer->xuid, g_dwSignedInController );
                SendVoiceInfoMessage( VOICEINFO_ADDREMOTEMUTE, PlayerList[ dwPlayerIndex ].PlayerID );

                if( IsLoggedIn( 0 ) )
                {
                    //$HACK: Spoofing controller 0
                    g_FriendsManager.AddPlayerToMuteList( 0, PlayerList[ dwPlayerIndex ].xuid );
                }
            }
            else if( pMenuHeader->m_pCurrentItem->TextIndex == TEXT_PLAYERS_UNMUTEPLAYER )
            {
                DWORD dwPlayerIndex = PlayerIndexFromXUID(pPlayer->xuid );

                g_VoiceManager.UnMutePlayer( pPlayer->xuid, g_dwSignedInController );
                SendVoiceInfoMessage( VOICEINFO_REMOVEREMOTEMUTE, PlayerList[ PlayerIndexFromXUID(pPlayer->xuid ) ].PlayerID );

                //$HACK: Spoofing controller 0
                if( IsLoggedIn( 0 ) && g_FriendsManager.IsPlayerInMuteList( 0, PlayerList[ dwPlayerIndex ].xuid ) )
                {
                    g_FriendsManager.RemovePlayerFromMuteList( 0, PlayerList[ dwPlayerIndex ].xuid );
                }
            }
            else if( pMenuHeader->m_pCurrentItem->TextIndex == TEXT_PLAYERS_SENDFEEDBACK )
            {
                g_PlayersStateEngine.NextState( CPlayersStateEngine::PLAYERS_STATE_PLAYERACTION );
                pMenuHeader->SetNextMenu( &Menu_SendFeedback );
                return TRUE;
            }

            g_PlayersStateEngine.NextState( CPlayersStateEngine::PLAYERS_STATE_MAINLOOP );
            pMenuHeader->SetNextMenu( &Menu_Players );
            return TRUE;
    }
    return FALSE;
}




//-----------------------------------------------------------------------------
// Name: CreateSendFeedbackMenu()
// Desc: Creates the "feedback" menu.  Currently just has the list of 
//          feedback options and needs some polish
//-----------------------------------------------------------------------------
void CreateSendFeedbackMenu(MENU_HEADER *pMenuHeader, MENU *pMenu)
{
    NET_PLAYER* pPlayer = (NET_PLAYER *)g_PlayersStateEngine.m_amiPlayers[ g_PlayersStateEngine.m_dwSelectedPlayer ].Data;

    assert( gTitleScreenVars.bUseXOnline );

    MenuItem_NicknameAndGamertag.Data = pPlayer;
    pMenuHeader->AddMenuItem( &MenuItem_NicknameAndGamertag, MENU_ITEM_INACTIVE );
    pMenuHeader->AddMenuItem( L"", MENU_ITEM_INACTIVE );
    pMenuHeader->AddMenuItem( L"", MENU_ITEM_INACTIVE );

    pMenuHeader->AddMenuItem( TEXT_PLAYERS_GOODATTITUDE );
    pMenuHeader->AddMenuItem( TEXT_PLAYERS_GOODSESSION );
    pMenuHeader->AddMenuItem( TEXT_PLAYERS_OFFENSIVENICKNAME );
    pMenuHeader->AddMenuItem( TEXT_PLAYERS_INAPPROPRIATEGAMEPLAY );
    pMenuHeader->AddMenuItem( TEXT_PLAYERS_VERBALLYABUSIVE );
    pMenuHeader->AddMenuItem( TEXT_PLAYERS_HARASSMENT );
    pMenuHeader->AddMenuItem( TEXT_PLAYERS_LEWDNESS );

    pMenuHeader->AddMenuItem( TEXT_TABLE(TEXT_BUTTON_A_SELECT_B_BACK), MENU_ITEM_ACTIVE );

    pMenu->CurrentItemIndex = 3;
}




//-----------------------------------------------------------------------------
// Name: HandleSendFeedbackMenu()
// Desc: Handles input for the send feedback menu
//-----------------------------------------------------------------------------
BOOL HandleSendFeedbackMenu( MENU_HEADER* pMenuHeader, DWORD dwInput )
{
    NET_PLAYER* pPlayer = (NET_PLAYER *)g_PlayersStateEngine.m_amiPlayers[ g_PlayersStateEngine.m_dwSelectedPlayer ].Data;

    switch( dwInput )
    {
        case MENU_INPUT_UP:
            return SelectPreviousMenuItem( pMenuHeader, pMenuHeader->m_pMenu, NULL );

        case MENU_INPUT_DOWN:
            return SelectNextMenuItem( pMenuHeader, pMenuHeader->m_pMenu, NULL );

        case MENU_INPUT_BACK:
            g_PlayersStateEngine.NextState( CPlayersStateEngine::PLAYERS_STATE_PLAYERACTION );
            pMenuHeader->SetNextMenu( &Menu_PlayerAction );
            return TRUE;

        case MENU_INPUT_SELECT:
            // Determine the feedback type from the menu item.  Once we have
            // entries in the string table for these guys, we can use the
            // data field to store the feedback constant, so we won't need
            // this series of checks
            XONLINE_FEEDBACK_TYPE feedback;
            if( pMenuHeader->m_pCurrentItem->TextIndex == TEXT_PLAYERS_OFFENSIVENICKNAME )
            {
                feedback = XONLINE_FEEDBACK_NEG_NICKNAME;
            }
            else if( pMenuHeader->m_pCurrentItem->TextIndex == TEXT_PLAYERS_INAPPROPRIATEGAMEPLAY )
            {
                feedback = XONLINE_FEEDBACK_NEG_GAMEPLAY;
            }
            else if( pMenuHeader->m_pCurrentItem->TextIndex == TEXT_PLAYERS_VERBALLYABUSIVE )
            {
                feedback = XONLINE_FEEDBACK_NEG_SCREAMING;
            }
            else if( pMenuHeader->m_pCurrentItem->TextIndex == TEXT_PLAYERS_HARASSMENT )
            {
                feedback = XONLINE_FEEDBACK_NEG_HARASSMENT;
            }
            else if( pMenuHeader->m_pCurrentItem->TextIndex == TEXT_PLAYERS_LEWDNESS )
            {
                feedback = XONLINE_FEEDBACK_NEG_LEWDNESS;
            }
            else if( pMenuHeader->m_pCurrentItem->TextIndex == TEXT_PLAYERS_GOODATTITUDE )
            {
                feedback = XONLINE_FEEDBACK_POS_ATTITUDE;
            }
            else if( pMenuHeader->m_pCurrentItem->TextIndex == TEXT_PLAYERS_GOODSESSION )
            {
                feedback = XONLINE_FEEDBACK_POS_SESSION;
            }
            else
            {
                assert( "Menu items out of sync with code" && FALSE );
            }
            
            WCHAR strTemp[ MAX_PLAYER_NAME ];
            swprintf( strTemp, L"%S", pPlayer->Name );

            //$HACK: We're always telling the online APIs the user signed in on controller 0.
            g_FriendsManager.SendFeedback( 0,
                                           pPlayer->xuid,
                                           feedback,
                                           strTemp );
                                           
            g_PlayersStateEngine.NextState( CPlayersStateEngine::PLAYERS_STATE_MAINLOOP );
            pMenuHeader->SetNextMenu( &Menu_Players );

            g_ShowSimpleMessage.Begin( TEXT_TABLE(TEXT_PLAYERS_SENDFEEDBACK),
                                 TEXT_TABLE(TEXT_FEEDBACK_SENT),
                                 NULL,
                                 TEXT_TABLE(TEXT_BUTTON_A_CONTINUE) );

            return TRUE;
    }
    
    return FALSE;
}



//-----------------------------------------------------------------------------
// Name: DrawNicknameAndGamertag()
// Desc: Handles drawing the nickname and gamertag of the NET_PLAYER pointed
//          to by the data field of the menu item
//-----------------------------------------------------------------------------
void DrawNicknameAndGamertag( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem, int itemIndex )
{
    NET_PLAYER* pPlayer = (NET_PLAYER *)pMenuItem->Data;
    FLOAT fX = pMenuHeader->m_XPos;
    FLOAT fY = pMenuHeader->m_YPos + MENU_TEXT_HEIGHT * itemIndex;

    WCHAR wstrPlayer[ XONLINE_GAMERTAG_SIZE + MAX_PLAYER_NAME + 20 ];
    swprintf( wstrPlayer, TEXT_TABLE(TEXT_FEEDBACK_FOR), pPlayer->Name );
    DrawMenuText( fX, fY, MENU_TEXT_RGB_NORMAL, wstrPlayer );
    swprintf( wstrPlayer, L"(%s %S)", TEXT_TABLE(TEXT_GAMERTAG), pPlayer->GamerTag );
    DrawMenuText( fX, fY + MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP, MENU_TEXT_RGB_NORMAL, wstrPlayer );
}




//-----------------------------------------------------------------------------
// Name: GeneratePlayersMenu()
// Desc: Generates an array of MenuItem structs to be used for the players
//          menu
//-----------------------------------------------------------------------------
HRESULT CPlayersStateEngine::GeneratePlayersMenu()
{
    assert( 0 <= PlayerCount && PlayerCount <= MAX_RACE_CARS );
    assert( 0 <= DepartedPlayerList.size() && DepartedPlayerList.size() <= MAX_DEPARTED_PLAYERS );

    m_bPlayersListChanged = FALSE;
    m_lFirstPlayerOnScreen = 0;

    // Set up the array of players
    ZeroMemory( m_amiPlayers, sizeof( m_amiPlayers ) );
    DWORD dwIndex = 0;
    for( DWORD i = 0; i < (DWORD)PlayerCount; i++ )
    {
        // Don't add ourself to the menu
        if( PlayerList[i].PlayerID != LocalPlayerID )
        {
            m_amiPlayers[dwIndex].TextIndex = TEXT_NONE;
            m_amiPlayers[dwIndex].Data = &PlayerList[i];
            m_amiPlayers[dwIndex].DataWidth = STATUS_WIDTH + NICKNAME_WIDTH + GAMERTAG_WIDTH;
            m_amiPlayers[dwIndex].DrawFunc = DrawPlayersMenuItem;
            m_amiPlayers[dwIndex].ActiveFlags = MENU_ITEM_ACTIVE | MENU_ITEM_SELECTABLE;
            dwIndex++;
        }
    }

    // Now add the dearly departed
    for( PLAYER_LIST::iterator i = DepartedPlayerList.begin(); i < DepartedPlayerList.end(); ++i )
    {
        m_amiPlayers[ dwIndex ].TextIndex   = TEXT_NONE;
        m_amiPlayers[ dwIndex ].Data        = &*i;
        m_amiPlayers[ dwIndex ].DataWidth   = STATUS_WIDTH + NICKNAME_WIDTH + GAMERTAG_WIDTH;
        m_amiPlayers[ dwIndex ].DrawFunc    = DrawPlayersMenuItem;
        m_amiPlayers[ dwIndex ].ActiveFlags = MENU_ITEM_ACTIVE | MENU_ITEM_SELECTABLE;
        dwIndex++;
    }

    assert( ( dwIndex == 0 && PlayerCount == 0 ) || 
            ( dwIndex == PlayerCount + DepartedPlayerList.size() - 1 ) );
    g_pMenuHeader->SetNextMenu( &Menu_Players );

    if( m_dwSelectedPlayer >= DWORD( PlayerCount + DepartedPlayerList.size() - 1 ) )
        m_dwSelectedPlayer = 0;

    return S_OK;
}




//-----------------------------------------------------------------------------
// The Players state engine
//-----------------------------------------------------------------------------
HRESULT CPlayersStateEngine::Process()
{
    switch( m_State )
    {
        case PLAYERS_STATE_BEGIN:
        {
            m_dwSelectedPlayer  = 0;
            m_dwTalkIconIndex   = 0;
            m_TalkIconTimer     = 0.0f;
            GeneratePlayersMenu();
            m_State = PLAYERS_STATE_MAINLOOP;

            //$HACK: Spoofing controller 0 for consumer beta
            g_FriendsManager.StartUpdatingFriends( 0 );
            break;
        }

        case PLAYERS_STATE_PLAYERACTION:
            break;

        case PLAYERS_STATE_MAINLOOP:
            m_TalkIconTimer += TimeStep;
            if( m_TalkIconTimer > 0.3f )
            {
                m_dwTalkIconIndex = ( m_dwTalkIconIndex + 1 ) % NUM_TALKICONCYCLE;
                m_TalkIconTimer = 0.0f;
            }
            break;
    }

    // Make sure we check for changes to the player list before
    // drawing menus or handling input
    if( m_bPlayersListChanged )
        GeneratePlayersMenu();

    // Handle the menus
    g_pMenuHeader->HandleMenus();

    return S_FALSE;
}




//-----------------------------------------------------------------------------
// Name: AddFriend()
// Desc: Handles adding a player to the friends list
//-----------------------------------------------------------------------------
HRESULT CPlayersStateEngine::AddFriend()
{
    assert( m_dwSelectedPlayer >= 0 &&
            m_dwSelectedPlayer < (DWORD)PlayerCount + DepartedPlayerList.size() - 1 );

    // Get the NET_PLAYER ptr from the menu item, because
    // m_dwSelectedPlayer can't be used as an index into PlayerList.
    NET_PLAYER* pPlayer = (NET_PLAYER *)m_amiPlayers[ m_dwSelectedPlayer ].Data;
    if( pPlayer->xuid.qwUserID != 0 )
    {
        //$HACK: We're always telling the online APIs the user signed in on controller 0.
        return g_FriendsManager.AddPlayerToFriendsList( 0, pPlayer->xuid );
    }

    return E_FAIL;
}



