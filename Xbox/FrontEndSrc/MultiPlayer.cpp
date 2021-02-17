//-----------------------------------------------------------------------------
//
// File: MultiPlayer.cpp
//
// Desc: MultiPlayer.cpp
//
// Hist: 02.08.02 - ported/created
//
// Copyright (c)  Microsoft Corporation.  All rights reserved.
//-----------------------------------------------------------------------------

#include "revolt.h"
#include "menu2.h"
#include "MenuDraw.h"   // DrawSpruBox
#include "text.h"       // BeginTextState
#include "settings.h"   // RegistrySettings
#include "GameSettings.h" // GameDifficulty

// re-volt specific
#include "menutext.h"   // re-volt strings

// temporary includes?
#include "titlescreen.h"

#define MENU_MULTITYPE_XPOS             Real(100)
#define MENU_MULTITYPE_YPOS             Real(150)

void CreateMultiTypeMenu(MENU_HEADER *menuHeader, MENU * menu);

void SetRaceMultiPlayerTypeRace(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
void SetRaceMultiPlayerTypeBattle(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);


// Menu
MENU Menu_MultiType = 
{
    TEXT_MULTITYPE,
    MENU_IMAGE_MULTI,                       // Menu title bar image enum
    TITLESCREEN_CAMPOS_MULTI,                        // Camera pos index
    MENU_DEFAULT | MENU_CENTRE_X | MENU_CENTRE_Y,   // Menu type
    SPRU_COL_RANDOM,                        // Spru colour
    CreateMultiTypeMenu,                    // Create menu function
    MENU_MULTITYPE_XPOS,                    // X coord
    MENU_MULTITYPE_YPOS,                    // Y Coord
    0,                                      // Current Item index
    NULL,                                   // Parent Menu (OverWritten in Create)
};

// Multi Type - Single race
MENU_ITEM MenuItem_MultiSingle = 
{
    TEXT_SINGLERACE,                        // Text label index

    0,                                      // Space needed to draw item data
    &Menu_GameDifficulty,                   // Data
    NULL,                                   // Draw Function

	NULL,								    // Input handler
	0,                                      // Flags

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    NULL,                                   // Left Action
    NULL,                                   // Right Action
    MenuGoBack,                             // Back Action
    SetRaceMultiPlayerTypeRace,             // Forward Action
};

// Multi Type - Battle Mode
MENU_ITEM MenuItem_MultiBattle = {
    TEXT_BATTLETAG,                         // Text label index

    0,                                      // Space needed to draw item data
    &Menu_GameDifficulty,                   // Data
    NULL,                                   // Draw Function

	NULL,								    // Input handler
	0,                                      // Flags

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    NULL,                                   // Left Action
    NULL,                                   // Right Action
    MenuGoBack,                             // Back Action
    SetRaceMultiPlayerTypeBattle,           // Forward Action
};

// Create
void CreateMultiTypeMenu(MENU_HEADER *menuHeader, MENU * menu)
{
#ifndef _PC
    if(g_TitleScreenData.numberOfPlayers<2) g_TitleScreenData.numberOfPlayers = 2;
#endif
    // add menu items
    AddMenuItem(menuHeader, menu, &MenuItem_MultiSingle);
    AddMenuItem(menuHeader, menu, &MenuItem_MultiBattle);
}

// set multiplayer game type
void SetRaceMultiPlayerTypeRace(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{

#ifdef _N64
    int i;
    if (!g_Cheats){
    for (i = 0 ; i < LEVEL_NSHIPPED_LEVELS ; i++)
    {
        LEVELINFO *li = GetLevelInfo(i);
        if(li->TrackType == TRACK_TYPE_BATTLE)
            li->ObtainFlags = 0;
    
    }
    }
#endif
    GameSettings.GameType = GAMETYPE_SINGLE;
    MenuGoForward(menuHeader, menu, menuItem);
}

void SetRaceMultiPlayerTypeBattle(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{

    GameSettings.GameType = GAMETYPE_BATTLE;
#ifdef _N64
    LEVELINFO *li;
    int i;
    if (!g_Cheats){
    for (i = 0 ; i < LEVEL_NSHIPPED_LEVELS ; i++)
    {
#ifdef _NO_BATTLE_
        if(i==LEVEL_MUSEUM_BATTLE)  continue;
        if(i==LEVEL_GARDEN_BATTLE)  continue;
#endif
        li = GetLevelInfo(i);
        if(li->TrackType == TRACK_TYPE_BATTLE)
            li->ObtainFlags = LEVEL_AVAILABLE|LEVEL_SELECTABLE;
    
    }
    }
#endif

    MenuGoForward(menuHeader, menu, menuItem);
}

