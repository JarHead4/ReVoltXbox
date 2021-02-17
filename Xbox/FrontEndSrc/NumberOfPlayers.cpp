//-----------------------------------------------------------------------------
//
// File: NumberOfPlayers.cpp
//
// Desc: NumberOfPlayers.cpp
//
// Hist: 02.08.02 - ported/created
//
// Copyright (c)  Microsoft Corporation.  All rights reserved.
//-----------------------------------------------------------------------------

#include "revolt.h"
#include "menu2.h"
#include "MenuDraw.h"   // DrawSpruBox
#include "text.h"       // BeginTextState

// re-volt specific
#include "menutext.h"   // re-volt strings

// menus
#include "MultiPlayer.h"


// temporary includes?
#include "titlescreen.h"

void CreateNumberOfPlayersMenu(MENU_HEADER *menuHeader, MENU *menu);

void MenuGoForwardNumPlayers(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);

#define MENU_NUMPLAYERS_XPOS            Real(150)
#define MENU_NUMPLAYERS_YPOS            Real(100)


//#ifndef _PC

// Menu
MENU Menu_NumberOfPlayers = {
    TEXT_NONE,
    MENU_IMAGE_MULTI,                       // Menu title bar image enum
    TITLESCREEN_CAMPOS_MULTI,                        // Camera pos index
    MENU_DEFAULT,                           // Menu type
    SPRU_COL_RANDOM,                        // Spru colour
    CreateNumberOfPlayersMenu,              // Create menu function
    MENU_NUMPLAYERS_XPOS,                   // X coord
    MENU_NUMPLAYERS_YPOS,                   // Y Coord
    0,                                      // Current Item index
    NULL,                                   // Parent Menu (OverWritten in Create)
};

SLIDER_DATA_LONG NumPlayersSlider = {
#ifdef _N64
    &g_nPreferedPlayers,
#else
    &g_TitleScreenData.numberOfPlayers,
#endif
    2, MAX_LOCAL_PLAYERS,
    1,
    FALSE, TRUE,
};

// Multiplayer - select number of players
MENU_ITEM MenuItem_NumberOfPlayers = 
{
    TEXT_NUMPLAYERS,                        // Text label index
    MENU_DATA_WIDTH_INT,                    // Space needed to draw item data
    &NumPlayersSlider,                      // Data
    DrawSliderDataLong,                     // Draw Function

	NULL,								    // Input handler
	0,                                      // Flags

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    DecreaseSliderDataLong,                 // Left Action
    IncreaseSliderDataLong,                 // Right Action
    MenuGoBack,                             // Back Action
    MenuGoForwardNumPlayers,                // Forward Action
};




// Create Function
void CreateNumberOfPlayersMenu(MENU_HEADER *menuHeader, MENU *menu)
{

#ifdef _N64
    g_uCurrentNumControl = CRD_NumControllers();
    NumPlayersSlider.Max = g_uCurrentNumControl;
    if(g_nPreferedPlayers > g_uCurrentNumControl) g_nPreferedPlayers = g_uCurrentNumControl;
#endif

    AddMenuItem(menuHeader, menu, &MenuItem_NumberOfPlayers);
}

// Utility
void MenuGoForwardNumPlayers(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    //g_TitleScreenData.numberOfCars = g_TitleScreenData.numberOfPlayers;

#ifdef _N64
    g_TitleScreenData.numberOfPlayers =  g_nPreferedPlayers;
#endif
    
    g_TitleScreenData.iCurrentPlayer = 0;

    SetNextMenu(menuHeader, &Menu_MultiType);
}

//#endif

