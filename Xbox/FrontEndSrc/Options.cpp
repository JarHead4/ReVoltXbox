//-----------------------------------------------------------------------------
//
// File: Options.cpp
//
// Desc: Options.cpp
//
// Hist: 02.08.02 - ported/created
//
// Copyright (c)  Microsoft Corporation.  All rights reserved.
//-----------------------------------------------------------------------------

#include "revolt.h"
#include "menu2.h"
#include "MenuDraw.h"   // DrawSpruBox
#include "text.h"       // BeginTextState
#include "LevelLoad.h"  // GAMETYPE
#include "credits.h"    // InitCreditStateActive
#include "Gallery.h"    // InitCreditStateActive

// re-volt specific
#include "menutext.h"   // re-volt strings

// menus
#include "MainMenu.h"
#include "BestTimes.h"
#include "VideoSettings.h"
#include "AudioSettings.h"
#include "ControllerOptions.h"
#include "GameSettings.h"
#include "Options.h"

// temporary includes?
#include "titlescreen.h"  // TITLESCREEN_CAMPOS_ & g_bTitleScreenRunDemo
#include "gameloop.h"       // DemoTimeout

#define MENU_OPTIONS_XPOS               Real(100)
#define MENU_OPTIONS_YPOS               Real(150)

void CreateOptionsMenu(MENU_HEADER *menuHeader, MENU *menu);


extern bool gCarChosen;
extern bool gCarTaken;


// Menu
MENU Menu_Options = {
    TEXT_OPTIONS,
    MENU_IMAGE_OPTIONS,                     // Menu title bar image enum
    TITLESCREEN_CAMPOS_OVERVIEW,                     // Camera pos index
    MENU_DEFAULT | MENU_CENTRE_X,           // Menu type
    SPRU_COL_RANDOM,                        // Spru colour
    CreateOptionsMenu,                      // Create menu function
    MENU_OPTIONS_XPOS,                      // X coord
    MENU_OPTIONS_YPOS,                      // Y Coord
    0,                                      // Current Item index
    NULL,                                   // Parent Menu (OverWritten in Create)
};

// Options Menu - Best Times
MENU_ITEM MenuItem_BestTimes = 
{
    TEXT_BESTTIMES,                         // Text label index
    0,                                      // Space needed to draw item data
    &Menu_BestTimes,                        // Data
    NULL,                                   // Draw Function
	MenuDefHandler,                         // Input handler
	0,                                      // Flags
};

// Options Menu - Game Setting
MENU_ITEM MenuItem_GameSettings = {
    TEXT_GAMESETTINGS,                      // Text label index

    0,                                      // Space needed to draw item data
    &Menu_GameSettings,                     // Data
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

// Options Menu - Video Setting
MENU_ITEM MenuItem_VideoSettings = {
    TEXT_VIDEOSETTINGS,                     // Text label index

    0,                                      // Space needed to draw item data
    &Menu_VideoSettings,                    // Data
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

// Options Menu - Audio Setting
MENU_ITEM MenuItem_AudioSettings = {
    TEXT_AUDIOSETTINGS,                     // Text label index

    0,                                      // Space needed to draw item data
    &Menu_AudioSettings,                    // Data
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

// Options Menu - Controller Setting
MENU_ITEM MenuItem_ControllerSettings = {
    TEXT_CONTROLLER,                        // Text label index

    0,                                      // Space needed to draw item data
    &Menu_ControllerSettings,               // Data
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

// Options Menu - Show Credits
MENU_ITEM MenuItem_ShowCredits = {
    TEXT_CREDITS,                           // Text label index

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
    SetRaceCredits,                         // Forward Action
};

// Options Menu - Show Gallery
MENU_ITEM MenuItem_ShowGallery = {
    TEXT_GALLERY,                           // Text label index

    0,                                      // Space needed to draw item data
    &Menu_Gallery,                          // Data
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

// Create
void CreateOptionsMenu( MENU_HEADER* pMenuHeader, MENU* pMenu )
{
    // add menu items
    AddMenuItem( pMenuHeader, pMenu, &MenuItem_BestTimes );
    AddMenuItem( pMenuHeader, pMenu, &MenuItem_GameSettings );
    AddMenuItem( pMenuHeader, pMenu, &MenuItem_VideoSettings );
    AddMenuItem( pMenuHeader, pMenu, &MenuItem_AudioSettings );
    AddMenuItem( pMenuHeader, pMenu, &MenuItem_ControllerSettings );
    AddMenuItem( pMenuHeader, pMenu, &MenuItem_ShowCredits );
    AddMenuItem( pMenuHeader, pMenu, &MenuItem_ShowGallery );
}

// Utility
void SetRaceCredits(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    GameSettings.GameType = GAMETYPE_DEMO;
    g_bTitleScreenRunDemo = TRUE;
    InitCreditStateActive();
    DemoTimeout = ZERO;
    MenuGoForward(menuHeader, menu, menuItem);  
}

