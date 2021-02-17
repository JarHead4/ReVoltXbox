//-----------------------------------------------------------------------------
// File: ui_WaitForLobby.cpp
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
//#include "ui_WaitConfirm.h"

void CreateWaitForLobbyMenu(MENU_HEADER *menuHeader, MENU *menu);

BOOL MenuGoForwardNumPlayers(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
BOOL MenuGoBackFromLobbyWait(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);

static void DrawLobbyWait(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex);

#define MENU_TOPLEVEL_XPOS              0
#define MENU_TOPLEVEL_YPOS              0

/////////////////////////////////////////////////////////////////////
//
// wait for lobby menu
//
/////////////////////////////////////////////////////////////////////
extern MENU Menu_WaitForLobby = 
{
    TEXT_NONE,
    MENU_DEFAULT | MENU_CENTRE_X | MENU_CENTRE_Y,   // Menu type
    CreateWaitForLobbyMenu,                 // Create menu function
    NULL,                                   // Input handler function
    NULL,                                   // Menu draw function
    MENU_TOPLEVEL_XPOS,                     // X coord
    MENU_TOPLEVEL_YPOS,                     // Y Coord
};

// wait for lobby settings
MENU_ITEM MenuItem_WaitForLobbyDummy = {
    TEXT_NONE,                              // Text label index

    0,                                      // Space needed to draw item data
    NULL,                                   // Data
    DrawLobbyWait,                          // Draw Function

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

    menuHeader->AddMenuItem( &MenuItem_WaitForLobbyDummy);

// create lobby object

    r = CoCreateInstance(CLSID_DirectPlayLobby, NULL, CLSCTX_INPROC_SERVER, IID_IDirectPlayLobby3A, (void**)&Lobby);
    if (r != DP_OK)
    {
        ErrorDX(r, "Can't create DirectPlay Lobby object");
        QuitGame();
        return;
    }

// set wait for connection

    r = Lobby->WaitForConnectionSettings(0);
    if (r != DP_OK)
    {
        ErrorDX(r, "Can't set lobby wait mode");
        QuitGame();
        return;
    }
#endif // ! XBOX_DISABLE_NETWORK
}

// go back from lobby wait
BOOL MenuGoBackFromLobbyWait(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
#ifndef XBOX_DISABLE_NETWORK
    RELEASE(Lobby);
#endif
//$REMOVED    KillNetwork();
    MenuGoBack(menuHeader, menu, menuItem);
    return TRUE;
}

/////////////////////////////////////////////////////////////////////
//
// Draw lobby wait
//
/////////////////////////////////////////////////////////////////////
void DrawLobbyWait( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem, int itemIndex)
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
        DrawMenuText((640 - 31 * MENU_TEXT_WIDTH) / 2, (480 - MENU_TEXT_HEIGHT) / 2, MENU_TEXT_RGB_NORMAL, "Waiting for Lobby Connection...", pMenuHeader->ItemDataWidth);
    }

// look for settings

    size = PACKET_BUFFER_SIZE; //$MODIFIED: was originally set to DP_BUFFER_MAX
    flags = DPLMSG_SYSTEM;

    r = Lobby->ReceiveLobbyMessage(0, 0, &flags, ReceiveHeader, &size);
    if (r == DP_OK)
    {
//      DPLSYS_NEWCONNECTIONSETTINGS;
    }
 #endif // ! XBOX_DISABLE_NETWORK
}

