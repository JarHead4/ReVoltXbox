//-----------------------------------------------------------------------------
// File: Lobby.cpp
//
// Desc: Lobby.cpp
//
// Hist: 02.08.02 - ported/created
//
// Copyright (c)  Microsoft Corporation.  All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"
#include "menu2.h"
#include "MenuDraw.h"   // DrawSpruBox

// re-volt specific
#include "menutext.h"   // re-volt strings

// menus
#include "MainMenu.h"

// temporary includes?
#include "titlescreen.h"

#define MENU_TOPLEVEL_XPOS              Real(0)
#define MENU_TOPLEVEL_YPOS              Real(0)

static void CreateContinueLobbyMenu(MENU_HEADER *menuHeader, MENU *menu);
static void CreateWaitForLobbyMenu(MENU_HEADER *menuHeader, MENU *menu);
static void QuitLobbyGame(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
static void MenuGoBackFromLobbyWait(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);

static void DrawLobbyWait(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex);

#ifdef _PC

// Menu
MENU Menu_ContinueLobby = {
    TEXT_NONE,
    MENU_IMAGE_MULTI,                       // Menu title bar image enum
    TITLESCREEN_CAMPOS_MULTI,                        // Camera pos index
    MENU_DEFAULT | MENU_CENTRE_X | MENU_CENTRE_Y,   // Menu type
    SPRU_COL_RANDOM,                        // Spru colour
    CreateContinueLobbyMenu,                // Create menu function
    MENU_TOPLEVEL_XPOS,                     // X coord
    MENU_TOPLEVEL_YPOS,                     // Y Coord
    0,                                      // Current Item index
    &Menu_ContinueLobby,                    // Parent Menu (OverWritten in Create)
};

// Top Level - continue with lobby game
MENU_ITEM MenuItem_ContinueLobby = {
    TEXT_LOBBYCONTINUE,                     // Text label index

    0,                                      // Space needed to draw item data
    NULL,                                   // Data
    NULL,                                   // Draw Function

	NULL,								    // Input handler
	0,                                      // Flags

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    NULL,                                   // Left Action
    NULL,                                   // Right Action
    MenuGoBack,                             // Back Action
    MenuGoForward,                          // Forward Action
};

// Top Level - quit to normal game
MENU_ITEM MenuItem_QuitToGame = {
    TEXT_LOBBYQUIT,                         // Text label index

    0,                                      // Space needed to draw item data
    &Menu_TopLevel,                         // Data
    NULL,                                   // Draw Function

	NULL,								    // Input handler
	0,                                      // Flags

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    NULL,                                   // Left Action
    NULL,                                   // Right Action
    MenuGoBack,                             // Back Action
    QuitLobbyGame,                          // Forward Action
};

// Create Function
void CreateContinueLobbyMenu(MENU_HEADER *menuHeader, MENU *menu)
{
#ifndef XBOX_DISABLE_NETWORK
    if (LobbyConnection->dwFlags == DPLCONNECTION_CREATESESSION)
        MenuItem_ContinueLobby.Data = &Menu_MultiType;
    else
        MenuItem_ContinueLobby.Data = &Menu_PlayerName;

    AddMenuItem(menuHeader, menu, &MenuItem_ContinueLobby);
    AddMenuItem(menuHeader, menu, &MenuItem_QuitToGame);
#endif // ! XBOX_DISABLE_NETWORK
}

// quit lobby game

void QuitLobbyGame(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
#ifndef XBOX_DISABLE_NETWORK
    LobbyFree();
    RELEASE(DP);
#endif // ! XBOX_DISABLE_NETWORK
    MenuGoForward(menuHeader, menu, menuItem);
}
#endif

/////////////////////////////////////////////////////////////////////
//
// wait for lobby menu
//
/////////////////////////////////////////////////////////////////////
#ifdef _PC

// Menu
MENU Menu_WaitForLobby = {
    TEXT_NONE,
    MENU_IMAGE_MULTI,                       // Menu title bar image enum
    TITLESCREEN_CAMPOS_MULTI,                        // Camera pos index
    MENU_DEFAULT | MENU_CENTRE_X | MENU_CENTRE_Y,   // Menu type
    SPRU_COL_RANDOM,                        // Spru colour
    CreateWaitForLobbyMenu,             // Create menu function
    MENU_TOPLEVEL_XPOS,             // X coord
    MENU_TOPLEVEL_YPOS,                 // Y Coord
    0,                                  // Current Item index
    NULL,                               // Parent Menu (OverWritten in Create)
};

// wait for lobby settings
MENU_ITEM MenuItem_WaitForLobbyDummy = {
    TEXT_NONE,                              // Text label index

    0,                                      // Space needed to draw item data
    NULL,                                   // Data
    DrawLobbyWait,                          // Draw Function

	NULL,								    // Input handler
	0,                                      // Flags

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    NULL,                                   // Left Action
    NULL,                                   // Right Action
    MenuGoBackFromLobbyWait,                // Back Action
    NULL,                                   // Forward Action
};

// Create Function
void CreateWaitForLobbyMenu(MENU_HEADER *menuHeader, MENU *menu)
{
#ifndef XBOX_DISABLE_NETWORK
    HRESULT r;

// add menu item

    AddMenuItem(menuHeader, menu, &MenuItem_WaitForLobbyDummy);

// create lobby object

    r = CoCreateInstance(CLSID_DirectPlayLobby, NULL, CLSCTX_INPROC_SERVER, IID_IDirectPlayLobby3A, (void**)&Lobby);
    if (r != DP_OK)
    {
        ErrorDX(r, "Can't create DirectPlay Lobby object");
        g_bQuitGame = TRUE;
        return;
    }

// set wait for connection

    r = Lobby->WaitForConnectionSettings(0);
    if (r != DP_OK)
    {
        ErrorDX(r, "Can't set lobby wait mode");
        g_bQuitGame = TRUE;
        return;
    }
#endif // ! XBOX_DISABLE_NETWORK
}

// go back from lobby wait
void MenuGoBackFromLobbyWait(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
#ifndef XBOX_DISABLE_NETWORK
    RELEASE(Lobby);
#endif
    KillNetwork();
    MenuGoBack(menuHeader, menu, menuItem);
}

#endif

/////////////////////////////////////////////////////////////////////
//
// Draw lobby wait
//
/////////////////////////////////////////////////////////////////////
#ifdef _PC

void DrawLobbyWait(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex)
{
 #ifndef XBOX_DISABLE_NETWORK
    HRESULT r;
    DWORD size, flags;

// quit if no lobby

    if (!Lobby)
    {
        return;
    }

// draw 'waiting'

    if (TIME2MS(TimerCurrent) & 256)
    {
        DrawMenuText((640 - 31 * MENU_TEXT_WIDTH) / 2, (480 - MENU_TEXT_HEIGHT) / 2, MENU_TEXT_RGB_NORMAL, "Waiting for Lobby Connection..." );
    }

// look for settings

    size = DP_BUFFER_MAX;
    flags = DPLMSG_SYSTEM;

    r = Lobby->ReceiveLobbyMessage(0, 0, &flags, ReceiveHeader, &size);
    if (r == DP_OK)
    {
//      DPLSYS_NEWCONNECTIONSETTINGS;
    }
 #endif // ! XBOX_DISABLE_NETWORK
}

#endif

