//-----------------------------------------------------------------------------
// File: ui_ContinueLobby.cpp
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

// re-volt specific
#include "ui_menutext.h"   // re-volt strings

// temporary includes?
#include "player.h"         // for ui_TitleScreen.h to work
#include "ui_TitleScreen.h"
#include "ui_TopLevelMenu.h"

void CreateContinueLobbyMenu(MENU_HEADER *menuHeader, MENU *menu);

BOOL QuitLobbyGame(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);

#define MENU_TOPLEVEL_XPOS              0
#define MENU_TOPLEVEL_YPOS              0




/////////////////////////////////////////////////////////////////////
//
// continue lobby menu
//
/////////////////////////////////////////////////////////////////////
extern MENU Menu_ContinueLobby = 
{
    TEXT_NONE,
    MENU_DEFAULT | MENU_CENTRE_X | MENU_CENTRE_Y,   // Menu type
    CreateContinueLobbyMenu,                // Create menu function
    NULL,                                   // Input handler function
    NULL,                                   // Menu draw function
    MENU_TOPLEVEL_XPOS,                     // X coord
    MENU_TOPLEVEL_YPOS,                     // Y Coord
};

// Top Level - continue with lobby game
MENU_ITEM MenuItem_ContinueLobby = 
{
    TEXT_LOBBYCONTINUE,                     // Text label index

    0,                                      // Space needed to draw item data
    NULL,                                   // Data
    NULL,                                   // Draw Function

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
        MenuItem_ContinueLobby.Data = &Menu_EnterName;

    menuHeader->AddMenuItem( &MenuItem_ContinueLobby);
    menuHeader->AddMenuItem( &MenuItem_QuitToGame);
#endif // ! XBOX_DISABLE_NETWORK
}

// quit lobby game

BOOL QuitLobbyGame(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
#ifndef XBOX_DISABLE_NETWORK
    LobbyFree();
    RELEASE(DP);
#endif // ! XBOX_DISABLE_NETWORK
    MenuGoForward(menuHeader, menu, menuItem);
	return TRUE;
}

