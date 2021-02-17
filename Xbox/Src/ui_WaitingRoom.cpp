//-----------------------------------------------------------------------------
// File: ui_WaitingRoom.cpp
//
// Desc: UI implementation
//
// Re-Volt (Generic) Copyright (c) Probe Entertainment 1998
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"
#include "main.h"
#include "geom.h"
#include "particle.h"
#include "model.h"
#include "aerial.h"
#include "newcoll.h"
#include "body.h"
#include "car.h"
#include "ctrlread.h"
#include "object.h"
#include "light.h"
#include "obj_init.h"
#include "player.h"
#include "ai.h"
#include "ai_init.h"
#include "EditObject.h"
#include "drawobj.h"
#include "move.h"
#include "timing.h"
#include "visibox.h"
#include "spark.h"
#include "field.h"
#include "weapon.h"
#include "input.h"
#include "initplay.h"
#include "pickup.h"
#include "SoundEffectEngine.h"
#include "VoiceManager.h"
#include "Text.h"
#include "Panel.h"          // CENTRE_POS
#include "net_xonline.h"

#include "ui_Menu.h"
#include "ui_MenuText.h"
#include "ui_menudraw.h"
#include "ui_TitleScreen.h"
#include "ui_StateEngine.h"
#include "ui_WaitingRoom.h"
#include "ui_Players.h"
#include "ui_Friends.h"

#define MENU_WAITINGROOM_XPOS           0
#define MENU_WAITINGROOM_YPOS           0

static void CreateWaitingRoomMenu( MENU_HEADER* pMenuHeader, MENU* pMenu );
static BOOL HandleWaitingRoomMenu( MENU_HEADER* pMenuHeader, DWORD dwInput );
static void DrawWaitingRoom(MENU_HEADER *menuHeader, MENU *menu );//, MENU_ITEM *menuItem, int itemIndex);

////////////////////////////////////////////////////////////////
//
// Waiting Room
//
////////////////////////////////////////////////////////////////

// Menu
extern MENU Menu_WaitingRoom = 
{
    TEXT_WAITINGROOM,
    MENU_DEFAULT | MENU_NOBOX,              // Menu type
    CreateWaitingRoomMenu,                  // Create menu function
    HandleWaitingRoomMenu,                  // Menu input function
    DrawWaitingRoom,                        // Menu draw function
    MENU_WAITINGROOM_XPOS,                  // X coord
    MENU_WAITINGROOM_YPOS,                  // Y Coord
};

/*
// Waiting Room - Start Race
MENU_ITEM MenuItem_WaitingRoom = 
{
    TEXT_NONE,                              // Text label index
    640,                                    // Space needed to draw item data
    NULL,                                   // Data (Menu to set up game and then run it)
    DrawWaitingRoom,                        // Draw Function
};
*/

// Create
void CreateWaitingRoomMenu( MENU_HEADER* pMenuHeader, MENU* pMenu )
{
    pMenu->fWidth  = 640.0f;
    pMenu->fHeight = 480.0f;

    // set misc flags
    LocalPlayerReady = TRUE;
    SetPlayerData();

    if (IsServer())
    {
#ifndef XBOX_DISABLE_NETWORK //$REVISIT: Probably can remove; I don't think we need to call this (level name, etc gets propagated to clients manually, not via DPlay)
        LEVELINFO *levinf = GetLevelInfo(gTitleScreenVars.iLevelNum);
        SetSessionDesc(gTitleScreenVars.nameEnter[0], levinf->Dir, FALSE, GameSettings.GameType, GameSettings.RandomCars, GameSettings.RandomTrack);
#endif // !XBOX_DISABLE_NETWORK
    }

//$REMOVED_NOTUSED    PlayersRequestTime = 1.0f;

    // Add menu items
//    pMenuHeader->AddMenuItem( &MenuItem_WaitingRoom );
//    pMenuHeader->m_pMenu->dwFlags |= MENU_NOBOX;                                   // Don't draw a default box
}

BOOL HandleWaitingRoomMenu( MENU_HEADER* pMenuHeader, DWORD dwInput )
{
    switch( dwInput )
    {
        case MENU_INPUT_UP:
            return SelectPreviousMenuItem( pMenuHeader, pMenuHeader->m_pMenu, NULL );
        
        case MENU_INPUT_DOWN:
            return SelectNextMenuItem( pMenuHeader, pMenuHeader->m_pMenu, NULL );

        case MENU_INPUT_BACK:
            LocalPlayerReady = FALSE;
            SetPlayerData();

            g_WaitingRoomStateEngine.Return( STATEENGINE_TERMINATED );

            return TRUE;

        case MENU_INPUT_Y:
            // Players should be available even if not online.
            g_pActiveStateEngine->Call( &g_PlayersStateEngine );
            return TRUE;
            
        case MENU_INPUT_X:
            //$SINGLEPLAYER - Assuming single local player
            // Friends should be available if the player is logged in
            if( IsLoggedIn(0) )
            {
                g_pActiveStateEngine->Call( &g_FriendsStateEngine );
                return TRUE;
            }
            break;

        case MENU_INPUT_LEFT:
            if( IsServer() && gTitleScreenVars.bUseXOnline && !bGameStarted )
            {
                DWORD dwPublicFilled  = SessionCurr.nCurrPlayersPublic;
                DWORD dwPublicOpen    = SessionCurr.nMaxPlayersPublic - SessionCurr.nCurrPlayersPublic;
                DWORD dwPrivateFilled = SessionCurr.nCurrPlayersPrivate;
                DWORD dwPrivateOpen   = SessionCurr.nMaxPlayersPrivate - SessionCurr.nCurrPlayersPrivate;

                if( dwPublicOpen > 0 )
                {
                    dwPublicOpen--;
                    dwPrivateOpen++;

                    SessionCurr.nMaxPlayersPublic--;
                    SessionCurr.nMaxPlayersPrivate++;

                    XONLINETASK_HANDLE hTask;
                    XOnlineMatchSessionUpdate( SessionCurr.keyID,
                                               dwPublicFilled, dwPublicOpen,
                                               dwPrivateFilled, dwPrivateOpen,
                                               NUM_XATTRIB_MATCHCREATE, g_rgAttribs_MatchCreate,
                                               NULL, &hTask );
            
                    OnlineTasks_Add( hTask, TASK_MATCH_UPDATE );
                    return TRUE;
                }
            }
            break;

        case MENU_INPUT_RIGHT:
            if( IsServer() && gTitleScreenVars.bUseXOnline && !bGameStarted )
            {
                DWORD dwPublicFilled  = SessionCurr.nCurrPlayersPublic;
                DWORD dwPublicOpen    = SessionCurr.nMaxPlayersPublic - SessionCurr.nCurrPlayersPublic;
                DWORD dwPrivateFilled = SessionCurr.nCurrPlayersPrivate;
                DWORD dwPrivateOpen   = SessionCurr.nMaxPlayersPrivate - SessionCurr.nCurrPlayersPrivate;

                if( dwPrivateOpen > 0 )
                {
                    dwPublicOpen++;
                    dwPrivateOpen--;

                    SessionCurr.nMaxPlayersPublic++;
                    SessionCurr.nMaxPlayersPrivate--;

                    XONLINETASK_HANDLE hTask;
                    XOnlineMatchSessionUpdate( SessionCurr.keyID,
                                               dwPublicFilled, dwPublicOpen,
                                               dwPrivateFilled, dwPrivateOpen,
                                               NUM_XATTRIB_MATCHCREATE, g_rgAttribs_MatchCreate,
                                               NULL, &hTask );
            
                    OnlineTasks_Add( hTask, TASK_MATCH_UPDATE );
                    return TRUE;
                }
            }
            break;

        case MENU_INPUT_SELECT:
            if( IsServer() )
            {
                g_bTitleScreenRunGame = TRUE;
                return TRUE;
            }
            break;
    }
    return FALSE;
}




////////////////////////////////////////////////////////////////
//
// DrawWaitingRoom:
//
////////////////////////////////////////////////////////////////
void DrawWaitingRoom( MENU_HEADER* pMenuHeader, MENU* pMenu )//, MENU_ITEM* pMenuItem, int itemIndex)
{
    FLOAT xPos,  yPos, xSize, ySize;
    FLOAT xPos2;
#ifndef XBOX_DISABLE_NETWORK
    short i, j, k;
    unsigned char c;
#else
    short i;
#endif

// update players?

//$REMOVED_NOTUSED - ListPlayers/GetPlayerList WAS REMOVED BY US; NOT NEEDED (b/c server maintains player list and updates clients)
//    PlayersRequestTime += TimeStep;
//    if (PlayersRequestTime > 1.0f)
//    {
//        ListPlayers(NULL);
//        PlayersRequestTime = 0.0f;
//    }
//$END_REMOVAL

    xSize = (640*0.85f);
    ySize = (480*0.85f) - 60;
    xPos  = (640*0.15f)/2;
    yPos  = (480*0.15f)/2 + 60;

    xPos += pMenuHeader->m_XPos;
    yPos += pMenuHeader->m_YPos;

    xPos2 = 320 + pMenuHeader->m_XPos;

    // draw spru box
    DrawNewSpruBox( gMenuWidthScale  * (xPos) - 10,
                    gMenuHeightScale * (yPos) - 10,
                    gMenuWidthScale  * (xSize) + 20,
                    gMenuHeightScale * (ySize) + 20 );

    xPos += MENU_BORDER_WIDTH;
    yPos += MENU_BORDER_HEIGHT/2;

    BeginTextState();

// dump track name

//$MODIFIED
//    char hostname[MAX_PLAYER_NAME + 1], levelname[MAX_LEVEL_NAME + 1], leveldir[MAX_LEVEL_DIR_NAME + 1];
//    DPSESSIONDESC2 desc[2];
//    DWORD size = sizeof(DPSESSIONDESC2) * 2;
//
////$NOTE: not using DP::SetSessionDesc, so DP::GetSessionDesc won't work.  If really want to print these stats, need to get them from elsewhere.
//    HRESULT r = DP->GetSessionDesc(desc, &size);
//    if (r != DP_OK) {
//        ErrorDX(r, "Could not get sessions description");
//    }
//
//    j = 0;
//    while ((hostname[j] = desc->lpszSessionNameA[j]) != '\n' && desc->lpszSessionNameA[j]) j++;
//    hostname[j] = 0;
//    k = 0;
//    while ((leveldir[k] = desc->lpszSessionNameA[k + j + 1])) k++;
//
//    gTrackScreenLevelNum = GetLevelNum(leveldir);
//    LEVELINFO *levelInfo = GetLevelInfo(gTrackScreenLevelNum);
//    if (levelInfo != NULL) {
//        strncpy(levelname, levelInfo->Name, MAX_LEVEL_NAME);
//    } else {
//        levelname[0] = '\0';
//    }
//
//    sprintf(MenuBuffer, "%s:", TEXT_TABLE(TEXT_MULTITYPE));
//    DrawMenuText(xPos, yPos, MENU_TEXT_RGB_HILITE, MenuBuffer);
//    DrawMenuText(xPos + MENU_TEXT_WIDTH * 18, yPos, MENU_TEXT_RGB_NORMAL, desc->dwUser2 == GAMETYPE_NETWORK_RACE ? TEXT_TABLE(TEXT_SINGLERACE) : TEXT_TABLE(TEXT_BATTLETAG));
//
//    yPos += MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP;
//    DrawMenuText(xPos, yPos, MENU_TEXT_RGB_HILITE, TEXT_TABLE(TEXT_LOADSCREEN_TRACK));
//    DrawMenuText(xPos + MENU_TEXT_WIDTH * 18, yPos, MENU_TEXT_RGB_NORMAL, GameSettings.RandomTrack ? TEXT_TABLE(TEXT_RANDOM) : levelname);

    WCHAR strLevelName[MAX_LEVEL_NAME];
    //$HACK(Apr02_GameBash) - invitations don't get session attributes
    if( g_bInvitedByFriend )
    {
        SessionCurr.iLevelNum = 1;
    }
    LEVELINFO* levelInfo = GetLevelInfo(SessionCurr.iLevelNum);
    if( levelInfo != NULL ) 
    {
        swprintf(strLevelName, L"%s ", levelInfo->strName);
        if( gTitleScreenVars.mirror ) //$BUG: this value should come from SessionCurr, but we don't have a way of getting it yet.
            wcscat( strLevelName, TEXT_TABLE(TEXT_MIRROR_ABREV_PARENTHESIS));
        if( gTitleScreenVars.reverse ) //$BUG: this value should come from SessionCurr, but we don't have a way of getting it yet.
            wcscat( strLevelName, TEXT_TABLE(TEXT_REVERSE_ABREV_PARENTHESIS));
    } 
    else 
    {
        strLevelName[0] = L'\0';
    }

    gTrackScreenLevelNum = SessionCurr.iLevelNum;
    long nMaxPlayers = SessionCurr.nMaxPlayersPublic + SessionCurr.nMaxPlayersPrivate;

    DrawMenuText(xPos, yPos, MENU_TEXT_RGB_HILITE, TEXT_TABLE(TEXT_WAITINGROOM_GAMETYPE));
    DrawMenuText(xPos + 160, yPos, MENU_TEXT_RGB_NORMAL, SessionCurr.GameType == GAMETYPE_NETWORK_RACE ? TEXT_TABLE(TEXT_SINGLERACE) : TEXT_TABLE(TEXT_BATTLETAG));
    yPos += MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP;

    DrawMenuText(xPos, yPos, MENU_TEXT_RGB_HILITE, TEXT_TABLE(TEXT_WAITINGROOM_TRACK));
    DrawMenuText(xPos + 160, yPos, MENU_TEXT_RGB_NORMAL, SessionCurr.RandomTrack ? TEXT_TABLE(TEXT_RANDOM) : strLevelName);
    yPos += MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP;

    DrawMenuText(xPos, yPos, MENU_TEXT_RGB_HILITE, TEXT_TABLE(TEXT_WAITINGROOM_LAPS));
    swprintf(MenuBuffer, L"%d", 0); //$BUG: this value should come from SessionCurr, but we don't have a way of getting it yet.
    DrawMenuText(xPos + 160, yPos, MENU_TEXT_RGB_NORMAL, MenuBuffer );
    yPos += MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP;

    DrawMenuText(xPos, yPos, MENU_TEXT_RGB_HILITE, TEXT_TABLE(TEXT_WAITINGROOM_MAXPLAYERS));
    swprintf(MenuBuffer, L"%d", nMaxPlayers);
    DrawMenuText(xPos + 160, yPos, MENU_TEXT_RGB_NORMAL, MenuBuffer );
    yPos += MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP;

//$END_MODIFICATIONS

    if( IsServer() && gTitleScreenVars.bUseXOnline && !bGameStarted )
    {
        swprintf( MenuBuffer, L"%ld", SessionCurr.nCurrPlayersPrivate );
        g_pFont->DrawText(  64 + pMenuHeader->m_XPos, yPos, MENU_TEXT_RGB_HILITE, TEXT_TABLE(TEXT_LOBBY_PRIVATESLOTSFILLED) );
        g_pFont->DrawText( 270 + pMenuHeader->m_XPos, yPos, MENU_TEXT_RGB_NORMAL, MenuBuffer );
        swprintf( MenuBuffer, L"%ld", SessionCurr.nCurrPlayersPublic );
        g_pFont->DrawText( 320 + pMenuHeader->m_XPos, yPos, MENU_TEXT_RGB_HILITE, TEXT_TABLE(TEXT_LOBBY_PUBLICSLOTSFILLED) );
        g_pFont->DrawText( 520 + pMenuHeader->m_XPos, yPos, MENU_TEXT_RGB_NORMAL, MenuBuffer );
        yPos += MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP;
        
        swprintf( MenuBuffer, L"%c %ld %c", 0x2190, SessionCurr.nMaxPlayersPrivate - SessionCurr.nCurrPlayersPrivate, 0x2192 );
        g_pFont->DrawText(  64 + pMenuHeader->m_XPos, yPos, MENU_TEXT_RGB_HILITE, TEXT_TABLE(TEXT_LOBBY_PRIVATESLOTSOPEN) );
        g_pFont->DrawText( 246 + pMenuHeader->m_XPos, yPos, MENU_TEXT_RGB_NORMAL, MenuBuffer );
        swprintf( MenuBuffer, L"%c %ld %c", 0x2190, SessionCurr.nMaxPlayersPublic - SessionCurr.nCurrPlayersPublic, 0x2192 );
        g_pFont->DrawText( 320 + pMenuHeader->m_XPos, yPos, MENU_TEXT_RGB_HILITE, TEXT_TABLE(TEXT_LOBBY_PUBLICSLOTSOPEN) );
        g_pFont->DrawText( 496 + pMenuHeader->m_XPos, yPos, MENU_TEXT_RGB_NORMAL, MenuBuffer );
        yPos += MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP;
    }
        
    // show players
    yPos += MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP;

    if( nMaxPlayers <= 8 )
    {
        // Draw player list in single-column mode
        DrawMenuText(xPos, yPos, MENU_TEXT_RGB_HILITE, TEXT_TABLE(TEXT_PLAYER));
        DrawMenuText(xPos + 214, yPos, MENU_TEXT_RGB_HILITE, TEXT_TABLE(TEXT_CAR));
        DrawMenuText(xPos + 448, yPos, MENU_TEXT_RGB_HILITE, TEXT_TABLE(TEXT_STATUS));
        yPos += MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP;

        for (i = 0 ; i < PlayerCount ; i++, yPos += MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP)
        {
            swprintf( MenuBuffer, L"%S", PlayerList[i].Name );
            DrawMenuText(xPos, yPos, MENU_TEXT_RGB_NORMAL, MenuBuffer, 204 );
            if( GameSettings.RandomCars ) 
                swprintf( MenuBuffer, L"%s", TEXT_TABLE(TEXT_RANDOM));
            else 
                swprintf( MenuBuffer, L"%S", CarInfo[PlayerList[i].CarType].Name);
            DrawMenuText(xPos + 214, yPos, MENU_TEXT_RGB_NORMAL, MenuBuffer);
//$TODO: Display the actual player status (which still needs some under the hood work)
            DrawMenuText(xPos + 448, yPos, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_READY) );
            //$BUG: should we get car name from CarType ??
            //$HEY: do we want per-machine Ready status?  (or none at all?)
    //$END_MODIFICATIONS
        }

        // Show empty players
        for( ; i < nMaxPlayers ; i++, yPos += MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP )
        {
            swprintf( MenuBuffer, L"----------", PlayerList[i].Name);
            DrawMenuText(xPos, yPos, MENU_TEXT_RGB_NORMAL, MenuBuffer );
            DrawMenuText(xPos + 214, yPos, MENU_TEXT_RGB_NORMAL, MenuBuffer);
            DrawMenuText(xPos + 448, yPos, MENU_TEXT_RGB_NORMAL, MenuBuffer);
        }
    }
    else
    {
        // Draw player list in two-column mode
        DrawMenuText(xPos, yPos, MENU_TEXT_RGB_HILITE, TEXT_TABLE(TEXT_PLAYER));
        DrawMenuText(xPos + 144, yPos, MENU_TEXT_RGB_HILITE, TEXT_TABLE(TEXT_CAR));
        DrawMenuText(xPos2, yPos, MENU_TEXT_RGB_HILITE, TEXT_TABLE(TEXT_PLAYER));
        DrawMenuText(xPos2 + 144, yPos, MENU_TEXT_RGB_HILITE, TEXT_TABLE(TEXT_CAR));
        yPos += MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP;

        for (i = 0 ; i < PlayerCount ; i++)
        {
            FLOAT sx = ( i < 8 ) ? xPos : xPos2;
            FLOAT sy = yPos + (MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP) * (i%8);

            swprintf( MenuBuffer, L"%S", PlayerList[i].Name);
            DrawMenuText(sx, sy, MENU_TEXT_RGB_NORMAL, MenuBuffer, 134 );
            if( GameSettings.RandomCars ) 
                swprintf( MenuBuffer, L"%s", TEXT_TABLE(TEXT_RANDOM));
            else 
                swprintf( MenuBuffer, L"%S", CarInfo[PlayerList[i].CarType].Name);
            DrawMenuText(sx + 144, sy, MENU_TEXT_RGB_NORMAL, MenuBuffer, 102 );
        }

        // Show empty players
        for( ; i < nMaxPlayers ; i++)
        {
            FLOAT sx = ( i < 8 ) ? xPos : xPos2;
            FLOAT sy = yPos + (MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP) * (i%8);

            swprintf( MenuBuffer, L"----------", PlayerList[i].Name);
            DrawMenuText(sx, sy, MENU_TEXT_RGB_NORMAL, MenuBuffer );
            DrawMenuText(sx + 144, sy, MENU_TEXT_RGB_NORMAL, MenuBuffer);
        }
    }

    // host
//$MODIFIED
//    if (GameSettings.MultiType == MULTITYPE_SERVER)
    if( IsServer() )
//$END_MODIFICATIONS
    {
//$BEGIN ADDITION
        LookForClientConnections();
//$END ADDITION

        if( !(TIME2MS(TimerCurrent) & 128) )
            g_pFont->DrawText( pMenuHeader->m_XPos + 320, 400 + pMenuHeader->m_YPos, MENU_TEXT_RGB_NOTCHOICE, TEXT_TABLE(TEXT_HITTABTOSTART), XBFONT_CENTER_X );
        else
            g_pFont->DrawText( pMenuHeader->m_XPos + 320, 400 + pMenuHeader->m_YPos, MENU_TEXT_RGB_CHOICE, TEXT_TABLE(TEXT_HITTABTOSTART), XBFONT_CENTER_X );
    }
    else // client
    {
        if( HostQuit )
        {
            if( !(TIME2MS(TimerCurrent) & 256) )
            {
                g_pFont->DrawText( pMenuHeader->m_XPos + 320, 400 + pMenuHeader->m_YPos, MENU_TEXT_RGB_CHOICE, TEXT_TABLE(TEXT_MULTIGAMETERMINATED), XBFONT_CENTER_X );
            }
        }
        else
        {
            g_pFont->DrawText( pMenuHeader->m_XPos + 320, 400 + pMenuHeader->m_YPos, MENU_TEXT_RGB_CHOICE, TEXT_TABLE(TEXT_WAITINGFORHOST), XBFONT_CENTER_X );
        }

        if( bGameStarted )
        {
            //SetRaceData(pMenuHeader, pMenu, pMenuItem);
            g_bTitleScreenRunGame = TRUE;
        }
    }

    if( gTitleScreenVars.bUseXOnline )
    {
        g_pFont->DrawText( xPos,  pMenuHeader->m_YPos+420, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_LOBBY_X_DISPLAYFRIENDS) );
        g_pFont->DrawText( xPos2, pMenuHeader->m_YPos+420, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_LOBBY_Y_DISPLAYPLAYERS) );
    }

    // get remote messages
    GetRemoteMessages();
}




CWaitingRoomStateEngine g_WaitingRoomStateEngine;

//-----------------------------------------------------------------------------
// Name: Process()
// Desc: Main control function for the state engine
//-----------------------------------------------------------------------------
HRESULT CWaitingRoomStateEngine::Process()
{
    enum
    {
        WAITINGROOM_STATE_BEGIN = STATEENGINE_STATE_BEGIN,
        WAITINGROOM_STATE_MAINLOOP,
    };

    switch( m_State )
    {
        case WAITINGROOM_STATE_BEGIN:
            g_pMenuHeader->SetNextMenu( &Menu_WaitingRoom );
            g_pTitleScreenCamera->SetNewPos( TITLESCREEN_CAMPOS_MULTI );

            m_State = WAITINGROOM_STATE_MAINLOOP;
            break;

        case WAITINGROOM_STATE_MAINLOOP:
            //$BUGBUG: This is running the WHOLE time the game is going?!
            /// If so, then not only is it wasteful, but the check below will
            /// get triggered if the server drops in the middle of the game
            /// (but we don't want to trigger it; we want something different
            /// to happen).

            // If connection to server was lost, then we already called LeaveSession().
            // So display a status message here and then back out.
            if( IsClient() && !IsInGameSession() )
            {
                //$TODO: display a message here saying that server machine was disconnected or turned off

                // $HACK(Apr02_GameBash) - If we did this when we were in the game, we'd get
                // screwed, because you could hit "B" all the way back to top level menu.
                // Ideally, if you're in the watiting room when the server dies, we should
                // back out, but it's not terrible if we don't
                // g_WaitingRoomStateEngine.Return( STATEENGINE_TERMINATED );
            }

            break;
    }

    // Handle the menus
    g_pMenuHeader->HandleMenus();

    return S_FALSE;
}



