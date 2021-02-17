//$REVISIT(cprince): This file should probably go away.  We won't be selecting
// a connection on Xbox.  But first, need to make sure no other code is
// using these functions (even though shouldn't be).

//-----------------------------------------------------------------------------
// File: ui_SelectConnection.cpp
//
// Desc: UI implementation
//
// Re-Volt (Generic) Copyright (c) Probe Entertainment 1998
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"
#include "ui_Menu.h"        // MENU_DEFAULT
#include "ui_MenuDraw.h"    // DrawSpruBox
#include "main.h"           // TimeStep
#include "text.h"           // BeginTextState
#include "timing.h"         // TIME2MS
#include "Settings.h"       // RegistrySettings
#include "Input.h"          // GetKeyPress

// re-volt specific
#include "ui_menutext.h"   // re-volt strings

// temporary includes?
#include "player.h"         // for ui_TitleScreen.h to work
#include "ui_TitleScreen.h"
#include "ui_StateEngine.h"
#include "ui_SelectConnection.h"
#include "ui_SelectRaceMode.h"
#include "ui_WaitForLobby.h"
#include "ui_EnterName.h"

static void CreateConnectionMenu( MENU_HEADER* pMenuHeader, MENU* pMenu );
static BOOL HandleConnectionMenu( MENU_HEADER* pMenuHeader, DWORD dwInput );
static void EnterHostName(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);

static BOOL SetGameHost(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
static BOOL SetGameClient(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
static BOOL SelectPrevConnectionType(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
static BOOL SelectNextConnectionType(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
static BOOL TogglePacketType(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
static BOOL ToggleProtocolType(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);

static void CreateMultiGameSelectMenu(MENU_HEADER *menuHeader, MENU *menu);
static BOOL SelectPrevMultiGame(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
static BOOL SelectNextMultiGame(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
static BOOL MenuGoBackKillPlay(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
static BOOL MenuJoinSession(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);

static void DrawConnectionType(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex);
static void DrawProtocolType(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex);
static void DrawHostComputer(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex);
static void DrawPacketType(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex);
static void DrawMultiGameList(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex);

#define MENU_CONNECTION_XPOS            100
#define MENU_CONNECTION_YPOS            150

#define MENU_MULTIGAMESELECT_XPOS       0
#define MENU_MULTIGAMESELECT_YPOS       0

//$REMOVEDextern char LocalIP[];
long SessionRefreshFlag;


////////////////////////////////////////////////////////////////
//
// Connection Type Menu
//
////////////////////////////////////////////////////////////////
extern MENU Menu_Connection = 
{
    TEXT_MULTIPLAYER,
    MENU_DEFAULT,                           // Menu type
    CreateConnectionMenu,                   // Create menu function
    HandleConnectionMenu,                   // Input handler function
    NULL,                                   // Menu draw function
    MENU_CONNECTION_XPOS,                   // X coord
    MENU_CONNECTION_YPOS,                   // Y Coord
};

// Data
SLIDER_DATA_LONG ConnectionSlider = 
{
    &gTitleScreenVars.connectionType,
    0, 0, 1,
    FALSE, TRUE,
};

long HostnameEntry;

// Connection - Start
MENU_ITEM MenuItem_StartMulti = 
{
    TEXT_STARTGAME,                      // Text label index
    0,                                      // Space needed to draw item data
    &Menu_MultiType,                        // Data
    NULL,                                   // Draw Function
};

// Connection - Join
MENU_ITEM MenuItem_JoinMulti = 
{
    TEXT_JOINGAME,                       // Text label index
    0,                                      // Space needed to draw item data
    &Menu_MultiGameSelect,                  // Data
    NULL,                                   // Draw Function
};

// Connection - Wait for lobby connection
MENU_ITEM MenuItem_WaitForLobby = 
{
    TEXT_MP_WAITFORLOBBY,                   // Text label index
    0,                                      // Space needed to draw item data
    &Menu_WaitForLobby,                     // Data
    NULL,                                   // Draw Function
};

// Connection - Connection Type
MENU_ITEM MenuItem_ConnectionType = 
{
    TEXT_CONNECTION,                        // Text label index
    MENU_DATA_WIDTH_TEXT,                   // Space needed to draw item data
    &ConnectionSlider,                      // Data
    DrawConnectionType,                     // Draw Function
};

// Connection - host computer
MENU_ITEM MenuItem_HostComputer = 
{
    TEXT_HOSTMACHINE,                      // Text label index
    MENU_DATA_WIDTH_TEXT + MENU_TEXT_WIDTH * 5, // Space needed to draw item data
    NULL,                                   // Data
    DrawHostComputer,                       // Draw Function
};

// Connection - packet type
MENU_ITEM MenuItem_PacketType = 
{
    TEXT_PACKET_TYPE,                       // Text label index
    MENU_DATA_WIDTH_TEXT,                   // Space needed to draw item data
    NULL,                                   // Data
    DrawPacketType,                         // Draw Function
};

// Connection - protocol type
MENU_ITEM MenuItem_ProtocolType = 
{
    TEXT_PROTOCOL_TYPE,                     // Text label index
    MENU_DATA_WIDTH_TEXT,                   // Space needed to draw item data
    NULL,                                   // Data
    DrawProtocolType,                       // Draw Function
};

// Create 
void CreateConnectionMenu(MENU_HEADER *menuHeader, MENU *menu)
{
    // init dplay, enum connection types
    assert( 0 && "Assumed ConnectionMenu wasn't used; InitNetwork/KillNetwork isn't handled correctly." );
    InitNetwork();

    // Init connection type slider range
//$MODIFIED
//    ConnectionSlider.Max = ConnectionCount - 1;
    ConnectionSlider.Max = 0;
//$END_MODIFICATIONS

    // init host name entry
    HostnameEntry = FALSE;

    // Add menu items
    menuHeader->AddMenuItem( &MenuItem_StartMulti );
    menuHeader->AddMenuItem( &MenuItem_JoinMulti );
//  menuHeader->AddMenuItem( &MenuItem_WaitForLobby );
    menuHeader->AddMenuItem( &MenuItem_ConnectionType );
    menuHeader->AddMenuItem( &MenuItem_HostComputer );
    menuHeader->AddMenuItem( &MenuItem_PacketType );
    menuHeader->AddMenuItem( &MenuItem_ProtocolType );
}

BOOL HandleConnectionMenu( MENU_HEADER* pMenuHeader, DWORD dwInput )
{
    switch( dwInput )
    {
        case MENU_INPUT_UP:
            return SelectPreviousMenuItem( pMenuHeader, pMenuHeader->m_pMenu, NULL );

        case MENU_INPUT_DOWN:
            return SelectNextMenuItem( pMenuHeader, pMenuHeader->m_pMenu, NULL );

        case MENU_INPUT_BACK:
#ifndef XBOX_DISABLE_NETWORK
            if (!Lobby)
#endif
            {
                assert( 0 && "Assumed ConnectionMenu wasn't used; InitNetwork/KillNetwork isn't handled correctly." );
                KillNetwork();
                GameSettings.MultiType = MULTITYPE_NONE;
            }
            g_SelectConnectionStateEngine.Return( STATEENGINE_TERMINATED );
            return TRUE;

        case MENU_INPUT_LEFT:
            switch( pMenuHeader->m_pCurrentItem->TextIndex )
            {
                case TEXT_CONNECTION:
                    return DecreaseSliderDataLong( pMenuHeader, pMenuHeader->m_pMenu, NULL );
                case TEXT_PACKET_TYPE:
                    return TogglePacketType( pMenuHeader, pMenuHeader->m_pMenu, NULL );
                case TEXT_PROTOCOL_TYPE:
                    return ToggleProtocolType( pMenuHeader, pMenuHeader->m_pMenu, NULL );
            }
            break;

        case MENU_INPUT_RIGHT:
            switch( pMenuHeader->m_pCurrentItem->TextIndex )
            {
                case TEXT_CONNECTION:
                    return IncreaseSliderDataLong( pMenuHeader, pMenuHeader->m_pMenu, NULL );
                case TEXT_PACKET_TYPE:
                    return TogglePacketType( pMenuHeader, pMenuHeader->m_pMenu, NULL );
                case TEXT_PROTOCOL_TYPE:
                    return ToggleProtocolType( pMenuHeader, pMenuHeader->m_pMenu, NULL );
            }
            break;

        case MENU_INPUT_SELECT:
            switch( pMenuHeader->m_pCurrentItem->TextIndex )
            {
                case TEXT_STARTGAME:
                    return SetGameHost( pMenuHeader, pMenuHeader->m_pMenu, NULL );

                case TEXT_JOINGAME:
                    return SetGameClient( pMenuHeader, pMenuHeader->m_pMenu, NULL );

                case TEXT_MP_WAITFORLOBBY:
                    pMenuHeader->SetNextMenu( &Menu_WaitForLobby );
                    return TRUE;

                case TEXT_HOSTMACHINE:
                    EnterHostName( pMenuHeader, pMenuHeader->m_pMenu, NULL );
                    return TRUE;
            }
            break;
    }

    return FALSE;
}

// Utility
BOOL MenuGoBackKillPlay(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
#ifndef XBOX_DISABLE_NETWORK
    if (!Lobby)
#endif
    {
        assert( 0 && "Assumed ConnectionMenu wasn't used; InitNetwork/KillNetwork isn't handled correctly." );
        KillNetwork();
        GameSettings.MultiType = MULTITYPE_NONE;
    }
    MenuGoBack(menuHeader, menu, menuItem);
    return TRUE;
}

void EnterHostName(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    HostnameEntry = !HostnameEntry;
}

BOOL SetGameHost(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    //$REVISIT: should these 2 lines go away eventually?
    /// (Should only have 1 copy of this name variable.  And it should have been filled with value from user's profile.)
    strncpy( gTitleScreenVars.PlayerData[0].nameEnter, RegistrySettings.PlayerName, MAX_PLAYER_NAME );
    gTitleScreenVars.PlayerData[0].nameEnter[MAX_PLAYER_NAME-1] = '\0';

//$REMOVED (tentative!!) - Xbox always connected
//    // init connection
//    if (!InitConnection((char)gTitleScreenVars.connectionType))
//    {
//        return;
//    }
//$END_REMOVAL

    // create session + create player
    LocalPlayerReady = FALSE;
    HostQuit = FALSE;

    assert( 0 && "Assumed this function wasn't used; DestroySession isn't handled correctly." );
    if( !CreateSession() )
    {
//$REMOVED
//        KillNetwork();
//        InitNetwork();
//$END_REMOVAL
        return FALSE;
    }

//$MOVED    CreateLocalServerPlayers(); //$ADDITION

//$REMOVED_DOESNOTHING    CreatePlayer(gTitleScreenVars.PlayerData[0].nameEnter, DP_SERVER_PLAYER);

#ifndef XBOX_DISABLE_NETWORK //$REVISIT: Probably can remove; I don't think we need to call this (level name, etc gets propagated to clients manually, not via DPlay)
    LEVELINFO *levinf = GetLevelInfo(gTitleScreenVars.iLevelNum);
    SetSessionDesc(gTitleScreenVars.nameEnter[0], levinf->Dir, FALSE, GAMETYPE_NETWORK_RACE, GameSettings.RandomCars, GameSettings.RandomTrack);
#endif // !XBOX_DISABLE_NETWORK

//$REMOVED
//    // get local IP
//    GetIPString(LocalIP);
//$END_REMOVAL

    // set host
    GameSettings.MultiType = MULTITYPE_SERVER;
    MenuGoForward(menuHeader, menu, menuItem);  
    return TRUE;
}

BOOL SetGameClient(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{

//$REMOVED (tentative!!) - Xbox always connected
//    // init connection
//    if (!InitConnection((char)gTitleScreenVars.connectionType))
//    {
//        return;
//    }
//$END_REMOVAL

//$REMOVED
//    // get local IP
//    GetIPString(LocalIP);
//$END_REMOVAL

    // set client
    GameSettings.MultiType = MULTITYPE_CLIENT;
    MenuGoForward(menuHeader, menu, menuItem);
    return TRUE;
}

BOOL TogglePacketType(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
#ifndef XBOX_DISABLE_NETWORK
    SessionFlag ^= DPSESSION_OPTIMIZELATENCY;
    return TRUE;
#endif
    return FALSE;
}

BOOL ToggleProtocolType(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
#ifndef XBOX_DISABLE_NETWORK
    SessionFlag ^= DPSESSION_DIRECTPLAYPROTOCOL;
    return TRUE;
#endif
    return FALSE;
}


////////////////////////////////////////////////////////////////
//
// Multi player Join Game Select
//
////////////////////////////////////////////////////////////////
extern MENU Menu_MultiGameSelect = 
{
    TEXT_MULTISELECT,
    MENU_DEFAULT,                           // Menu type
    CreateMultiGameSelectMenu,              // Create menu function
    NULL,                                   // Input handler function
    NULL,                                   // Menu draw function
    MENU_MULTIGAMESELECT_XPOS,              // X coord
    MENU_MULTIGAMESELECT_YPOS,              // Y Coord
};

// Multiplayer select game
MENU_ITEM MenuItem_MultiGameSelect = {
    TEXT_NONE,                              // Text label index

    640,                                    // Space needed to draw item data
    &Menu_EnterName,                        // Data (next menu in this case)
    DrawMultiGameList,                      // Update State Function (Refresh list and draw)

    SelectPrevMultiGame,                    // Up Action
    SelectNextMultiGame,                    // Down Action
    NULL,                                   // Left Action
    NULL,                                   // Right Action
    MenuGoBackKillPlay,                     // Back Action
    MenuJoinSession,                        // Forward Action
};

// Create
void CreateMultiGameSelectMenu(MENU_HEADER *menuHeader, MENU *menu)
{
    // prepare for session enum
    assert( GetSessionCount() == 0 ); //$NOTE: WAS ORIGINALLY "SessionCount = 0;" , calls to ClearSessionList() ensure SessionCount will be zero
    SessionPick = 0;
    SessionRefreshFlag = 3;

//$REMOVED - server maintains player list and updates clients
//    if ( GetSessionCount() )
//        ListPlayers(&SessionList[SessionPick].Guid); //$NOTE: 
//$END_REMOVAL

    // Add the dummy menu item necessary to check for keys and call the frame update function
    menuHeader->AddMenuItem( &MenuItem_MultiGameSelect );
    menuHeader->m_pMenu->dwFlags |= MENU_NOBOX;                                   // Don't draw a default box
}

// Utility

BOOL SelectPrevMultiGame(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    if (SessionPick)
    {
        SessionPick--;
        PlayerCount = 0;  //$REVISIT(cprince): do we need this line?  (We set PlayerCount elsewhere.)
        return TRUE;
    }

    return FALSE;
}

BOOL SelectNextMultiGame(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    if (SessionPick < GetSessionCount() - 1)
    {
        SessionPick++;
        PlayerCount = 0;  //$REVISIT(cprince): do we need this line?  (We set PlayerCount elsewhere.)
        return TRUE;
    }

    return FALSE;
}

// join session
BOOL MenuJoinSession(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    // any sessions?
    if ( GetSessionCount() )
        return FALSE;

    //$REVISIT - Is anything supposed to happen here?
    return FALSE;
}

////////////////////////////////////////////////////////////////
//
// DrawMultiGameList
//
////////////////////////////////////////////////////////////////
void DrawMultiGameList( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem, int itemIndex)
{
//$MODIFIED
//    short i, j, k;
    short i;
//$END_MODIFICATIONS
    FLOAT xPos, yPos, xSize, ySize;
    long rgb;
    WCHAR* status;

// request sessions / players?

//$HACK_MODIFIED: workaround for funky UI mapping that doesn't allow us to refresh manually
//    if (Keys[DIK_SPACE] && !LastKeys[DIK_SPACE] && !SessionRefreshFlag)
    if (!SessionRefreshFlag)
//$END_HACK
    {
#ifdef OLD_AUDIO
        PlaySfx(SFX_MENU_FORWARD, SFX_MAX_VOL, SFX_CENTRE_PAN, 44100, 0x7fffffff);
#else
        g_SoundEngine.Play2DSound( EFFECT_MenuNext, FALSE );
#endif // OLD_AUDIO
//$HACK_MODIFIED: workaround for funky UI mapping that doesn't allow us to refresh manually
//        SessionRefreshFlag = 3;
        SessionRefreshFlag = 5*60; // hack to reduce network flooding (and improve UI usability)
//$END_HACK
    }

    if (SessionRefreshFlag)
    {
        if (!--SessionRefreshFlag)
        {
            RequestSessionList(); //$MODIFIED: was RefreshSessions (equivalent)
        }
    }

// draw headings

    xSize = (58 * MENU_TEXT_WIDTH + 2 * MENU_TEXT_GAP);
    ySize = (26 * MENU_TEXT_HEIGHT + 2 * MENU_TEXT_GAP);

    xPos = 640 - xSize - 40;
    yPos = 120;

    xPos += pMenuHeader->m_XPos;
    yPos += pMenuHeader->m_YPos;

// draw spru box
    DrawSpruBox(
        gMenuWidthScale * (xPos - MENU_TEXT_GAP),
        gMenuHeightScale * (yPos - MENU_TEXT_GAP + 20),
        gMenuWidthScale * (xSize),
        gMenuHeightScale * (ySize - 30),
        SPRU_COL_RANDOM, 0);

    BeginTextState();

    DumpTextReal(xPos, yPos, MENU_TEXT_WIDTH, MENU_TEXT_HEIGHT, MENU_TEXT_RGB_HILITE, TEXT_TABLE(TEXT_HOST));
    DumpTextReal(xPos + MENU_TEXT_WIDTH * 18, yPos, MENU_TEXT_WIDTH, MENU_TEXT_HEIGHT, MENU_TEXT_RGB_HILITE, TEXT_TABLE(TEXT_TRACK));
    DumpTextReal(xPos + MENU_TEXT_WIDTH * 37, yPos, MENU_TEXT_WIDTH, MENU_TEXT_HEIGHT, MENU_TEXT_RGB_HILITE, TEXT_TABLE(TEXT_GAME));
    DumpTextReal(xPos + MENU_TEXT_WIDTH * 48, yPos, MENU_TEXT_WIDTH, MENU_TEXT_HEIGHT, MENU_TEXT_RGB_HILITE, TEXT_TABLE(TEXT_STATUS));

    if (SessionRefreshFlag)
    {
        DumpText((640 - (wcslen(gTitleScreen_Text[TEXT_SEARCHINGFORGAMES]) * 8)) / 2, 430, 8, 12, 0xffffffff, gTitleScreen_Text[TEXT_SEARCHINGFORGAMES]);
    }
//$REMOVED - not used
//    else if (SessionJoinFlag)
//    {
//        DumpText((640 - (wcslen(gTitleScreen_Text[TEXT_JOININGGAME]) * 8)) / 2, 430, 8, 12, 0xffffffff, gTitleScreen_Text[TEXT_GAME_JOIN]);
//    }
//$END_REMOVAL
    else
    {
        DumpTextReal(xPos - MENU_TEXT_GAP + (xSize - wcslen(gTitleScreen_Text[TEXT_PRESS_A_TOREFRESHGAMELIST]) * MENU_TEXT_WIDTH)/2 , 430 + pMenuHeader->m_YPos, MENU_TEXT_WIDTH, MENU_TEXT_HEIGHT, MENU_TEXT_RGB_NORMAL, gTitleScreen_Text[TEXT_PRESS_A_TOREFRESHGAMELIST]);
    }

    yPos += MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP;

    GetRemoteMessages();  //$ADDITION: we need to do this manually, since we're not using DPlay to enumerate sessions
                          //$REVISIT(cprince): should all the scattered calls to GetRemoteMessages (in UI code) be replaced with a single call inside TitleScreen()?  We might not want to call GetRemoteMessages until after certain things have been done (eg, JoinSession has been called), and/or might need a bConnected flag.

// draw sessions + players

    if ( GetSessionCount() )
    {
        for (i = 0 ; i < GetSessionCount() ; i++, yPos += MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP)
        {
            if (SessionPick == i) rgb = TIME2MS(TimerCurrent) & 128 ? 0x404040 : MENU_TEXT_RGB_NORMAL;
            else rgb = MENU_TEXT_RGB_NORMAL;

//$MODIFIED
//            char hostname[MAX_PLAYER_NAME + 1], levelname[MAX_LEVEL_NAME + 1], leveldir[MAX_LEVEL_DIR_NAME + 1];  //$REVISIT: do we really want/need "+1" for these?
//
//            j = 0;
//            while ((hostname[j] = SessionList[i].name[j]) != '\n' && SessionList[i].name[j]) j++;
//            hostname[j] = 0;
//            k = 0;
//            while ((leveldir[k] = SessionList[i].name[k + j + 1])) k++;
//
//            int levelNum = GetLevelNum(leveldir);
//            LEVELINFO *levelInfo = GetLevelInfo(levelNum);
//            if (levelInfo != NULL) {
//                strncpy(levelname, levelInfo->Name, MAX_LEVEL_NAME);
//            } else {
//                levelname[0] = '\0';
//            }
//
//            if (i == SessionPick) {
//                gTrackScreenLevelNum = levelNum;
//            }
//
//            DumpTextReal(xPos, yPos, MENU_TEXT_WIDTH, MENU_TEXT_HEIGHT, rgb, hostname);
//            DumpTextReal(xPos + (MENU_TEXT_WIDTH * (MAX_PLAYER_NAME + 2)), yPos, MENU_TEXT_WIDTH, MENU_TEXT_HEIGHT, rgb, SessionList[i].RandomTrack ? TEXT_TABLE(TEXT_RANDOM) : levelname);

            WCHAR strLevelName[MAX_LEVEL_NAME] = L"";
            LEVELINFO* levelInfo = GetLevelInfo(SessionList[i].iLevelNum);
            if( levelInfo != NULL ) 
                swprintf( strLevelName, L"%s", levelInfo->strName );

            if (i == SessionPick) 
                gTrackScreenLevelNum = SessionList[i].iLevelNum;

            DrawMenuText(xPos, yPos, rgb, SessionList[i].wstrHostNickname );
            DrawMenuText(xPos + (MENU_TEXT_WIDTH * (MAX_PLAYER_NAME + 2)), yPos, rgb, SessionList[i].RandomTrack ? TEXT_TABLE(TEXT_RANDOM) : strLevelName);
//$END_MODIFICATIONS
            DrawMenuText(xPos + (MENU_TEXT_WIDTH * (MAX_PLAYER_NAME + 21)), yPos, rgb, SessionList[i].GameType == GAMETYPE_NETWORK_RACE ? TEXT_TABLE(TEXT_RACE) : TEXT_TABLE(TEXT_BATTLETAG));


            if (SessionList[i].Version != (unsigned long)MultiplayerVersion)
                status = TEXT_TABLE(TEXT_WRONGVERSION);
            else if (SessionList[i].Started)
                status = TEXT_TABLE(TEXT_STARTED);
//$MODIFIED
//            else if ((DWORD)PlayerCount == SessionList[i].nCurrPlayers) status = TEXT_TABLE(TEXT_FULL);
            else if (SessionList[i].nCurrPlayersPublic == SessionList[i].nMaxPlayersPublic)  //$REVISIT: do we only want to check public slots (assuming most people can't use private slots and thus don't care)??
//$END_MODIFICATIONS
                status = TEXT_TABLE(TEXT_FULL);
            else
                status = TEXT_TABLE(TEXT_OPEN);

            DrawMenuText(xPos + (MENU_TEXT_WIDTH * (MAX_PLAYER_NAME + 32)), yPos, rgb, status);
        }

        if (PlayerCount)
        {
            yPos = 240 + pMenuHeader->m_YPos;

            DumpTextReal(xPos, yPos, MENU_TEXT_WIDTH, MENU_TEXT_HEIGHT, MENU_TEXT_RGB_HILITE, TEXT_TABLE(TEXT_PLAYER));
            DumpTextReal(xPos + MENU_TEXT_WIDTH * 18, yPos, MENU_TEXT_WIDTH, MENU_TEXT_HEIGHT, MENU_TEXT_RGB_HILITE, TEXT_TABLE(TEXT_CAR));
            DumpTextReal(xPos + MENU_TEXT_WIDTH * 36, yPos, MENU_TEXT_WIDTH, MENU_TEXT_HEIGHT, MENU_TEXT_RGB_HILITE, TEXT_TABLE(TEXT_STATUS));

            yPos += MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP;
            for (i = 0 ; i < PlayerCount ; i++, yPos += MENU_TEXT_HEIGHT)
            {
                swprintf( MenuBuffer, L"%S", PlayerList[i].Name);
                DrawMenuText(xPos, yPos, MENU_TEXT_RGB_NORMAL, MenuBuffer);
                if (SessionList[SessionPick].RandomCars) 
                {
                    DrawMenuText(xPos + (MENU_TEXT_WIDTH * (MAX_PLAYER_NAME + 2)), yPos, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_RANDOM));
                }
//$MODIFIED
//              else 
//              {
//                  DumpTextReal(xPos + (MENU_TEXT_WIDTH * (MAX_PLAYER_NAME + 2)), yPos, MENU_TEXT_WIDTH, MENU_TEXT_HEIGHT, MENU_TEXT_RGB_NORMAL, PlayerList[i].Data.CarName);
//              }
//              DumpTextReal(xPos + (MENU_TEXT_WIDTH * (MAX_PLAYER_NAME + 20)), yPos, MENU_TEXT_WIDTH, MENU_TEXT_HEIGHT, MENU_TEXT_RGB_NORMAL, PlayerList[i].Data.Ready ? TEXT_TABLE(TEXT_READY) : TEXT_TABLE(TEXT_NOTREADY));
                else 
                {
                    swprintf( MenuBuffer, L"%S", CarInfo[PlayerList[i].CarType].Name);
                    DrawMenuText(xPos + (MENU_TEXT_WIDTH * (MAX_PLAYER_NAME + 2)), yPos, MENU_TEXT_RGB_NORMAL, MenuBuffer );
                }
//$TODO: Display the actual player status (which still needs some under the hood work)
                DrawMenuText(xPos + (MENU_TEXT_WIDTH * (MAX_PLAYER_NAME + 20)), yPos, MENU_TEXT_RGB_NORMAL, L"Ready");
                //$BUG: should we get car name from CarType ??
//$END_MODIFICATIONS
            }
        }
    }

#if 0  //$ADDITION
    // are we trying to join?
    if (SessionJoinFlag)
    {

// yep, flag ready?

        if (--SessionJoinFlag)
        {
            return;
        }

// yep, refresh session list

        RequestSessionList();  //$MODIFIED: was RefreshSessions (equivalent)

// valid session?

        if (SessionPick > GetSessionCount() - 1)
            return;

//$HACK_REMOVAL: removed this so we can progress thru menus (but might actually want some of these checks later)
//// wrong checksum?
//
//        if (SessionList[SessionPick].Version != (unsigned long)MultiplayerVersion)
//            return;
//
//// session open?
//
//        if (SessionList[SessionPick].Started)
//            return;
//
//// max players?
//
//        if (SessionList[SessionPick].PlayerNum == (DWORD)PlayerCount)
//            return;
//$END_HACK

// yep, join session, create player

        //$REVISIT: should these 2 lines go away eventually?
        /// (Should only have 1 copy of this name variable.  And it should have been filled with value from user's profile.)
        strncpy( gTitleScreenVars.PlayerData[0].nameEnter, RegistrySettings.PlayerName, MAX_PLAYER_NAME );
        gTitleScreenVars.PlayerData[0].nameEnter[MAX_PLAYER_NAME-1] = '\0';

        LocalPlayerReady = FALSE;
        HostQuit = FALSE;

        if (!JoinSession((char)SessionPick))
        {
            DeleteSessionListEntry(SessionPick);
            SessionPick = 0;

            return;
        }

//$ADDITION - need to do some other stuff after joining session (eg, add players, etc)
        LocalPlayerID = INVALID_PLAYER_ID; // to know when server acknowledges our Join request
        RequestAddPlayers( dwLocalPlayerCount );  //$CMP_NOTE: should we do this elsewhere?
//$END_ADDITION

//$REMOVED_DOESNOTHING        CreatePlayer(gTitleScreenVars.PlayerData[0].nameEnter, DP_CLIENT_PLAYER);

// set random car / track flag

        GameSettings.RandomCars = SessionList[SessionPick].RandomCars;
        GameSettings.RandomTrack = SessionList[SessionPick].RandomTrack;

// next pMenu

        MenuGoForward(pMenuHeader, pMenu, pMenuItem);
    }
#endif //$ADDITION
}


////////////////////////////////////////////////////////////////
//
// DrawConnectionType
//
////////////////////////////////////////////////////////////////
void DrawConnectionType( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem, int itemIndex)
{
 #ifndef XBOX_DISABLE_NETWORK
    FLOAT xPos, yPos;

    xPos = pMenuHeader->m_XPos + pMenuHeader->m_ItemTextWidth + MENU_TEXT_GAP;
    yPos = pMenuHeader->m_YPos + (MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP) * itemIndex;

    DrawMenuText(xPos,yPos, MENU_TEXT_RGB_NORMAL, Connection[gTitleScreenVars.connectionType].Name, pMenuHeader->m_ItemDataWidth);

    if (Connection[gTitleScreenVars.connectionType].Guid.Data1 == 0x36e95ee0 && Connection[gTitleScreenVars.connectionType].Guid.Data2 == 0x8577 && Connection[gTitleScreenVars.connectionType].Guid.Data3 == 0x11cf && *(_int64*)Connection[gTitleScreenVars.connectionType].Guid.Data4 == 0x824e53c780000c96)
    {
        MenuItem_HostComputer.ActiveFlags = MENU_ITEM_ACTIVE | MENU_ITEM_SELECTABLE;
    }
    else
    {
        MenuItem_HostComputer.ActiveFlags = 0;
    }
 #endif // ! XBOX_DISABLE_NETWORK
}


/////////////////////////////////////////////////////////////////////
//
// Draw protocol type
//
/////////////////////////////////////////////////////////////////////
void DrawProtocolType( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem, int itemIndex)
{
 #ifndef XBOX_DISABLE_NETWORK
    FLOAT xPos, yPos;

    xPos = pMenuHeader->m_XPos + pMenuHeader->m_ItemTextWidth + MENU_TEXT_GAP;
    yPos = pMenuHeader->m_YPos + (MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP) * itemIndex;

    DrawMenuText(xPos,yPos, MENU_TEXT_RGB_NORMAL, SessionFlag & DPSESSION_DIRECTPLAYPROTOCOL ? TEXT_TABLE(TEXT_YES) : TEXT_TABLE(TEXT_NO), pMenuHeader->m_ItemDataWidth);
 #endif // ! XBOX_DISABLE_NETWORK
}

////////////////////////////////////////////////////////////////
//
// Draw Host Computer
//
////////////////////////////////////////////////////////////////
void DrawHostComputer( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem, int itemIndex)
{
    FLOAT xPos, yPos;
    short c;

    xPos = pMenuHeader->m_XPos + pMenuHeader->m_ItemTextWidth + MENU_TEXT_GAP;
    yPos = pMenuHeader->m_YPos + (MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP) * itemIndex;

    swprintf(MenuBuffer, L"%S", RegistrySettings.HostComputer);

    if (!HostnameEntry)
    {
        DrawMenuText(xPos, yPos, MENU_TEXT_RGB_NORMAL, MenuBuffer, pMenuHeader->m_ItemDataWidth);
    }
    else
    {
        DrawMenuText(xPos, yPos, 0xff0000, MenuBuffer, pMenuHeader->m_ItemDataWidth);
        if (!(TIME2MS(TimerCurrent) & 256)) DrawMenuText(xPos + wcslen(MenuBuffer) * MENU_TEXT_WIDTH, yPos, 0xff0000, L"_", pMenuHeader->m_ItemDataWidth);

        if (pMenu->CurrentItemIndex != itemIndex)
        {
            HostnameEntry = false;
            return;
        }

        if ((c = GetKeyPress()))
        {
            long len = strlen(RegistrySettings.HostComputer);

            if (c == 27 || c == 9 || c == 13) // escape / tab
            {
                return;
            }

            if (c == 8) // delete
            {
                if (len)
                {
                    RegistrySettings.HostComputer[len - 1] = 0;
                }
            }

            else if (len < MAX_HOST_COMPUTER - 1)
            {
                RegistrySettings.HostComputer[len] = (char)c;
                RegistrySettings.HostComputer[len + 1] = 0;
            }
        }
    }
}

/////////////////////////////////////////////////////////////////////
//
// Draw packet type
//
/////////////////////////////////////////////////////////////////////
void DrawPacketType( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem, int itemIndex)
{
 #ifndef XBOX_DISABLE_NETWORK
    FLOAT xPos, yPos;

    xPos = pMenuHeader->m_XPos + pMenuHeader->m_ItemTextWidth + MENU_TEXT_GAP;
    yPos = pMenuHeader->m_YPos + (MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP) * itemIndex;

    DrawMenuText(xPos,yPos, MENU_TEXT_RGB_NORMAL, SessionFlag & DPSESSION_OPTIMIZELATENCY ? "Latency" : "Bandwidth", pMenuHeader->m_ItemDataWidth);
 #endif // ! XBOX_DISABLE_NETWORK
}




CSelectConnectionStateEngine g_SelectConnectionStateEngine;

//-----------------------------------------------------------------------------
// Name: Process()
// Desc: Main control function for the state engine
//-----------------------------------------------------------------------------
HRESULT CSelectConnectionStateEngine::Process()
{
    enum
    {
        SELECTCONNECTION_STATE_BEGIN = STATEENGINE_STATE_BEGIN,
        SELECTCONNECTION_STATE_MAINLOOP,
    };

    switch( m_State )
    {
        case SELECTCONNECTION_STATE_BEGIN:
            g_pMenuHeader->SetNextMenu( &Menu_MultiGameSelect );
            g_pTitleScreenCamera->SetNewPos( TITLESCREEN_CAMPOS_MULTI );

            m_State = SELECTCONNECTION_STATE_MAINLOOP;
            break;

        // The user is viewing an initial message
        case SELECTCONNECTION_STATE_MAINLOOP:
            // Nothing to do, as control is in the menus
            break;
    }

    // Handle the menus
    g_pMenuHeader->HandleMenus();

    return S_FALSE;
}





