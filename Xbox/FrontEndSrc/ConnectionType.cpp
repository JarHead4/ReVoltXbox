//-----------------------------------------------------------------------------
// File: ConnectionType.cpp
//
// Desc: ConnectionType.cpp
//
// Hist: 02.08.02 - ported/created
//
// Copyright (c)  Microsoft Corporation.  All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"
#include "menu2.h"
#include "MenuDraw.h"   // DrawSpruBox
#include "settings.h"   // RegistrySettings
#include "timing.h"     // TIME2MS
#include "input.h"      // GetKeyPress
#include "LevelLoad.h"  // GAMETYPE
#include "GameSettings.h" // GameDifficulty
#include "GetPlayerName.h"
#include "ConnectionType.h"

// re-volt specific
#include "menutext.h"   // re-volt strings

// menus
#include "SyslinkGameSelect.h"
#include "MultiPlayer.h"
#include "Lobby.h"

// temporary includes?
#include "titlescreen.h"  // TITLESCREEN_CAMPOS_ & g_bTitleScreenRunDemo

#define MENU_CONNECTION_XPOS            Real(100)
#define MENU_CONNECTION_YPOS            Real(150)

static void CreateConnectionMenu(MENU_HEADER *menuHeader, MENU *menu);

static void EnterHostName(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
static void SetGameHost(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
static void SetGameClient(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
static void SelectPrevConnectionType(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
static void SelectNextConnectionType(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
static void TogglePacketType(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
static void ToggleProtocolType(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);

static void DrawConnectionType(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex);

static void DrawPacketType(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex);
static void DrawProtocolType(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex);
static void DrawHostComputer(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex);

// local IP
char LocalIP[64];
long HostnameEntry;



// Menu
MENU Menu_Connection = {
    TEXT_MULTIPLAYER,
    MENU_IMAGE_MULTI,                       // Menu title bar image enum
    TITLESCREEN_CAMPOS_MULTI,                        // Camera pos index
    MENU_DEFAULT,                           // Menu type
    SPRU_COL_RANDOM,                        // Spru colour
    CreateConnectionMenu,                   // Create menu function
    MENU_CONNECTION_XPOS,                   // X coord
    MENU_CONNECTION_YPOS,                   // Y Coord
    0,                                      // Current Item index
    NULL,                                   // Parent Menu (OverWritten in Create)
};

// Data
SLIDER_DATA_LONG ConnectionSlider = {
    &g_TitleScreenData.connectionType,
    0, 0, 1,
    FALSE, TRUE,
};


// Connection - Start
MENU_ITEM MenuItem_StartMulti = {
    TEXT_MP_STARTGAME,                      // Text label index

    0,                                      // Space needed to draw item data
    &Menu_MultiType,                        // Data
    NULL,                                   // Draw Function

	NULL,								    // Input handler
	0,                                      // Flags

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    NULL,                                   // Left Action
    NULL,                                   // Right Action
    MenuGoBackKillPlay,                     // Back Action
    SetGameHost,                            // Forward Action
};

// Connection - Join
MENU_ITEM MenuItem_JoinMulti = {
    TEXT_MP_JOINGAME,                       // Text label index

    0,                                      // Space needed to draw item data
    &Menu_MultiGameSelect,                  // Data
    NULL,                                   // Draw Function

	NULL,								    // Input handler
	0,                                      // Flags

    SelectPreviousMenuItem,                 // Up THE SKIRT Action
    SelectNextMenuItem,                     // Down THE BLOUSE Action
    NULL,                                   // Left Action
    NULL,                                   // Right Action
    MenuGoBackKillPlay,                     // Back PASSAGE Action
    SetGameClient,                          // Forward Action
};

// Connection - Wait for lobby connection
MENU_ITEM MenuItem_WaitForLobby = {
    TEXT_MP_WAITFORLOBBY,                   // Text label index

    0,                                      // Space needed to draw item data
    &Menu_WaitForLobby,                     // Data
    NULL,                                   // Draw Function

	NULL,								    // Input handler
	0,                                      // Flags

    SelectPreviousMenuItem,                 // Up THE SKIRT Action
    SelectNextMenuItem,                     // Down THE BLOUSE Action
    NULL,                                   // Left Action
    NULL,                                   // Right Action
    MenuGoBackKillPlay,                     // Back PASSAGE Action
    MenuGoForward,                          // Forward Action
};

// Connection - Connection Type
MENU_ITEM MenuItem_ConnectionType = {
    TEXT_CONNECTION,                        // Text label index

    MENU_DATA_WIDTH_TEXT,                   // Space needed to draw item data
    &ConnectionSlider,                      // Data
    DrawConnectionType,                     // Draw Function

	NULL,								    // Input handler
	0,                                      // Flags

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    DecreaseSliderDataLong,                 // Left Action
    IncreaseSliderDataLong,                 // Right Action
    MenuGoBackKillPlay,                     // Back Action
    NULL,                                   // Forward Action
};

// Connection - host computer
MENU_ITEM MenuItem_HostComputer = {
    TEXT_HOSTCOMPUTER,                      // Text label index

    MENU_DATA_WIDTH_TEXT + MENU_TEXT_WIDTH * 5, // Space needed to draw item data
    NULL,                                   // Data
    DrawHostComputer,                       // Draw Function

 	NULL,								    // Input handler
	0,                                      // Flags

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    NULL,                                   // Left Action
    NULL,                                   // Right Action
    MenuGoBackKillPlay,                     // Back Action
    EnterHostName,                          // Forward Action
};

// Connection - packet type
MENU_ITEM MenuItem_PacketType = {
    TEXT_PACKET_TYPE,                       // Text label index

    MENU_DATA_WIDTH_TEXT,                   // Space needed to draw item data
    NULL,                                   // Data
    DrawPacketType,                         // Draw Function

	NULL,								    // Input handler
	0,                                      // Flags

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    TogglePacketType,                       // Left Action
    TogglePacketType,                       // Right Action
    MenuGoBackKillPlay,                     // Back Action
    NULL,                                   // Forward Action
};

// Connection - protocol type
MENU_ITEM MenuItem_ProtocolType = {
    TEXT_PROTOCOL_TYPE,                     // Text label index

    MENU_DATA_WIDTH_TEXT,                   // Space needed to draw item data
    NULL,                                   // Data
    DrawProtocolType,                       // Draw Function

	NULL,								    // Input handler
	0,                                      // Flags

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    ToggleProtocolType,                     // Left Action
    ToggleProtocolType,                     // Right Action
    MenuGoBackKillPlay,                     // Back Action
    NULL,                                   // Forward Action
};

// Create 
void CreateConnectionMenu(MENU_HEADER *menuHeader, MENU *menu)
{
    // init dplay, enum connection types
    InitNetwork();

    // Init connection type slider range
//$MODIFIED
//    ConnectionSlider.Max = ConnectionCount - 1;
    ConnectionSlider.Max = 0;
//$END_MODIFICATIONS

    // init host name entry
    HostnameEntry = FALSE;

    // Add menu items
    AddMenuItem(menuHeader, menu, &MenuItem_StartMulti);
    AddMenuItem(menuHeader, menu, &MenuItem_JoinMulti);
//  AddMenuItem(menuHeader, menu, &MenuItem_WaitForLobby);
    AddMenuItem(menuHeader, menu, &MenuItem_ConnectionType);
    AddMenuItem(menuHeader, menu, &MenuItem_HostComputer);
    AddMenuItem(menuHeader, menu, &MenuItem_PacketType);
    AddMenuItem(menuHeader, menu, &MenuItem_ProtocolType);
}

// Utility
void MenuGoBackKillPlay(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
#ifndef XBOX_DISABLE_NETWORK
    if (!Lobby)
#endif
    {
        KillNetwork();
        GameSettings.MultiType = MULTITYPE_NONE;
    }
    MenuGoBack(menuHeader, menu, menuItem);
}

void EnterHostName(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    HostnameEntry = !HostnameEntry;
}

void SetGameHost(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{

//$REMOVED (tentative!!) - Xbox always connected
//    // init connection
//    if (!InitConnection((char)g_TitleScreenData.connectionType))
//    {
//        return;
//    }
//$END_REMOVAL

    // create session + create player
    LocalPlayerReady = FALSE;
    bGameStarted = FALSE;
    HostQuit = FALSE;

    if (!CreateSession("Revolt"))
    {
        KillNetwork();
        InitNetwork();
        return;
    }

//$ADDITION
    CreateLocalServerPlayers();
//$END_ADDITION

    CreatePlayer(g_TitleScreenData.nameEnter[0], DP_SERVER_PLAYER);

#ifndef XBOX_DISABLE_NETWORK //$REVISIT: Probably can remove; I don't think we need to call this (level name, etc gets propagated to clients manually, not via DPlay)
    LEVELINFO *levinf = GetLevelInfo(g_TitleScreenData.iLevelNum);
    SetSessionDesc(g_TitleScreenData.nameEnter[0], levinf->Dir, FALSE, GAMETYPE_MULTI, GameSettings.RandomCars, GameSettings.RandomTrack);
#endif // !XBOX_DISABLE_NETWORK

//$REMOVED
//    // get local IP
//    GetIPString(LocalIP);
//$END_REMOVAL

    // set host
    GameSettings.MultiType = MULTITYPE_SERVER;
    MenuGoForward(menuHeader, menu, menuItem);  
}

void SetGameClient(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{

//$REMOVED (tentative!!) - Xbox always connected
//    // init connection
//    if (!InitConnection((char)g_TitleScreenData.connectionType))
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
}

void TogglePacketType(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
#ifndef XBOX_DISABLE_NETWORK
    SessionFlag ^= DPSESSION_OPTIMIZELATENCY;
#endif
}

void ToggleProtocolType(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
#ifndef XBOX_DISABLE_NETWORK
    SessionFlag ^= DPSESSION_DIRECTPLAYPROTOCOL;
#endif
}


////////////////////////////////////////////////////////////////
//
// DrawConnectionType
//
////////////////////////////////////////////////////////////////
#ifdef _PC
void DrawConnectionType(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex)
{
 #ifndef XBOX_DISABLE_NETWORK
    REAL xPos, yPos;

    xPos = menuHeader->XPos + menuHeader->ItemTextWidth + MENU_TEXT_GAP;
    yPos = menuHeader->YPos + (MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP) * itemIndex;

    DrawMenuText(xPos,yPos, MENU_TEXT_RGB_NORMAL, Connection[g_TitleScreenData.connectionType].Name );

    if (Connection[g_TitleScreenData.connectionType].Guid.Data1 == 0x36e95ee0 && Connection[g_TitleScreenData.connectionType].Guid.Data2 == 0x8577 && Connection[g_TitleScreenData.connectionType].Guid.Data3 == 0x11cf && *(_int64*)Connection[g_TitleScreenData.connectionType].Guid.Data4 == 0x824e53c780000c96)
    {
        MenuItem_HostComputer.ActiveFlags = MENU_ITEM_ACTIVE | MENU_ITEM_SELECTABLE;
    }
    else
    {
        MenuItem_HostComputer.ActiveFlags = 0;
    }
 #endif // ! XBOX_DISABLE_NETWORK
}
#endif


/////////////////////////////////////////////////////////////////////
//
// Draw packet type
//
/////////////////////////////////////////////////////////////////////
#ifdef _PC

void DrawPacketType(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex)
{
 #ifndef XBOX_DISABLE_NETWORK
    REAL xPos, yPos;

    xPos = menuHeader->XPos + menuHeader->ItemTextWidth + MENU_TEXT_GAP;
    yPos = menuHeader->YPos + (MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP) * itemIndex;

    DrawMenuText(xPos,yPos, MENU_TEXT_RGB_NORMAL, SessionFlag & DPSESSION_OPTIMIZELATENCY ? "Latency" : "Bandwidth" );
 #endif // ! XBOX_DISABLE_NETWORK
}

#endif

/////////////////////////////////////////////////////////////////////
//
// Draw protocol type
//
/////////////////////////////////////////////////////////////////////
#ifdef _PC

void DrawProtocolType(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex)
{
 #ifndef XBOX_DISABLE_NETWORK
    REAL xPos, yPos;

    xPos = menuHeader->XPos + menuHeader->ItemTextWidth + MENU_TEXT_GAP;
    yPos = menuHeader->YPos + (MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP) * itemIndex;

    DrawMenuText(xPos,yPos, MENU_TEXT_RGB_NORMAL, SessionFlag & DPSESSION_DIRECTPLAYPROTOCOL ? TEXT_TABLE(TEXT_YES) : TEXT_TABLE(TEXT_NO) );
 #endif // ! XBOX_DISABLE_NETWORK
}

#endif

////////////////////////////////////////////////////////////////
//
// Draw Host Computer
//
////////////////////////////////////////////////////////////////

#ifdef _PC
void DrawHostComputer(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex)
{
    REAL xPos, yPos;
    short c;

    xPos = menuHeader->XPos + menuHeader->ItemTextWidth + MENU_TEXT_GAP;
    yPos = menuHeader->YPos + (MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP) * itemIndex;

    sprintf(MenuBuffer, "%s", RegistrySettings.HostComputer);

    if (!HostnameEntry)
    {
        DrawMenuText(xPos, yPos, MENU_TEXT_RGB_NORMAL, MenuBuffer );
    }
    else
    {
        DrawMenuText(xPos, yPos, 0xff0000, MenuBuffer );
        if (!(TIME2MS(TimerCurrent) & 256)) 
			DrawMenuText(xPos + strlen(MenuBuffer) * MENU_TEXT_WIDTH, yPos, 0xff0000, "_" );

        if (menu->CurrentItemIndex != itemIndex)
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
#endif

