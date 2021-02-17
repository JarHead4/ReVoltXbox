//-----------------------------------------------------------------------------
// File: ui_SelectRaceMode.cpp
//
// Desc: UI implementation
//
// Re-Volt (Generic) Copyright (c) Probe Entertainment 1998
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"
#include "main.h"           // TimeStep
#include "cheats.h"         // AllowEditors
#include "LevelLoad.h"      // GAMETYPE_*
#include "player.h"         // for ui_TitleScreen.h to work
#include "Text.h"           // DrawMenuText
#include "ui_Menu.h"        // MENU_DEFAULT
#include "ui_MenuDraw.h"    // DrawSpruBox
#include "ui_menutext.h"   // re-volt strings
#include "ui_TitleScreen.h"
#include "ui_StateEngine.h"
#include "ui_RaceDifficulty.h" 
#include "ui_EnterName.h" 
#include "ui_SelectConnection.h"
#include "ui_SelectRaceMode.h"

static void CreateStartRaceMenu(MENU_HEADER *menuHeader, MENU *menu);

static BOOL SetRaceChampionship(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
static BOOL SetRaceSingle(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
static BOOL SetRaceClockwork(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
static BOOL SetRaceConsoleMulti(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
static BOOL SetRaceTrial(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
static BOOL SetRaceBattle(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
static BOOL SetRacePractice(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
static BOOL SetRaceTraining(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
static BOOL SetRaceMultiPlayer(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
static BOOL SelectPrevEditMode(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
static BOOL SelectNextEditMode(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
static BOOL SetRaceCalcStats(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);

static BOOL SetLoadReplay(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);

static void CreateMultiTypeMenu( MENU_HEADER* pMenuHeader, MENU* pMenu );
static BOOL HandleMultiTypeMenu( MENU_HEADER* pMenuHeader, DWORD dwInput );

static void DrawEditMode(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex);

#define MENU_STARTRACE_XPOS             100
#define MENU_STARTRACE_YPOS             150

#define MENU_TOPLEVEL_XPOS              0
#define MENU_TOPLEVEL_YPOS              0

#define MENU_MULTITYPE_XPOS             100
#define MENU_MULTITYPE_YPOS             150

WCHAR *EditMenuText[] = 
{
    L"None",
    L"Lights",
    L"Vizibox",
    L"Objects",
    L"Instances",
    L"AI Nodes",
    L"Track Zones",
    L"Triggers",
    L"Camera Nodes",
    L"Farce Fields",
    L"Erm, nothing to see here",
    L"Pos Nodes",
};




/////////////////////////////////////////////////////////////////////
//
//  Start Race Menu
//
/////////////////////////////////////////////////////////////////////
extern MENU Menu_StartRace = 
{
    TEXT_SELECTRACE,
    MENU_DEFAULT | MENU_CENTRE_X,           // Menu type
    CreateStartRaceMenu,                    // Create menu function
    NULL,                                   // Input handler function
    NULL,                                   // Menu draw function
    MENU_STARTRACE_XPOS,                    // X coord
    MENU_STARTRACE_YPOS,
};

// Start Race - ChampionShip
MENU_ITEM MenuItem_SelectChampionship = 
{
    TEXT_CHAMPIONSHIP,                      // Text label index

    0,                                      // Space needed to draw item data
    &Menu_GameDifficulty,                       // Data
    NULL,                                   // Draw Function

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    NULL,                                   // Left Action
    NULL,                                   // Right Action
    MenuGoBack,                             // Back Action
    SetRaceChampionship,                    // Forward Action
};

// Start Race - Single Race
MENU_ITEM MenuItem_SelectSingleRace = 
{
    TEXT_SINGLERACE,                        // Text label index

    0,                                      // Space needed to draw item data
    &Menu_GameDifficulty,                       // Data
    NULL,                                   // Draw Function

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    NULL,                                   // Left Action
    NULL,                                   // Right Action
    MenuGoBack,                             // Back Action
    SetRaceSingle,                          // Forward Action
};

// Start Race - Clockwork Race
MENU_ITEM MenuItem_SelectClockworkRace = 
{
    TEXT_CLOCKWORKRACE,                     // Text label index

    0,                                      // Space needed to draw item data
    &Menu_GameDifficulty,                       // Data
    NULL,                                   // Draw Function

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    NULL,                                   // Left Action
    NULL,                                   // Right Action
    MenuGoBack,                             // Back Action
    SetRaceClockwork,                       // Forward Action
};

// Start Race - Time Trial
MENU_ITEM MenuItem_SelectTrial = 
{
    TEXT_TIMETRIAL,                         // Text label index

    0,                                      // Space needed to draw item data
    &Menu_GameDifficulty,                       // Data
    NULL,                                   // Draw Function

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    NULL,                                   // Left Action
    NULL,                                   // Right Action
    MenuGoBack,                             // Back Action
    SetRaceTrial,                           // Forward Action
};

// Start Race - Bomb Tag
#if 0
MENU_ITEM MenuItem_SelectBattle = 
{
    TEXT_BATTLETAG,                         // Text label index

    0,                                      // Space needed to draw item data
    &Menu_EnterName,                        // Data
    NULL,                                   // Draw Function

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    NULL,                                   // Left Action
    NULL,                                   // Right Action
    MenuGoBack,                             // Back Action
    SetRaceBattle,                          // Forward Action
};
#endif

// Start Race - Practice
MENU_ITEM MenuItem_SelectPractice = 
{
    TEXT_PRACTICE,                          // Text label index

    0,                                      // Space needed to draw item data
    &Menu_GameDifficulty,                       // Data
    NULL,                                   // Draw Function

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    NULL,                                   // Left Action
    NULL,                                   // Right Action
    MenuGoBack,                             // Back Action
    SetRacePractice,                        // Forward Action
};

// Start Race - Training
MENU_ITEM MenuItem_SelectTraining = 
{
    TEXT_TRAINING,                          // Text label index

    0,                                      // Space needed to draw item data
    //&Menu_GameDifficulty,                     // Data
    &Menu_EnterName,                       // No difficulty choice on stunt track// Data
    NULL,                                   // Draw Function

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    NULL,                                   // Left Action
    NULL,                                   // Right Action
    MenuGoBack,                             // Back Action
    SetRaceTraining,                        // Forward Action
};

// Start Race - Calculate car stats
MENU_ITEM MenuItem_SelectCalcStats = 
{
    TEXT_CALCSTATS,                         // Text label index

    0,                                      // Space needed to draw item data
    NULL,                                   // Data
    NULL,                                   // Draw Function

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    NULL,                                   // Left Action
    NULL,                                   // Right Action
    MenuGoBack,                             // Back Action
    SetRaceCalcStats,                       // Forward Action
};

// Start Race - Multi player PC ONLY
MENU_ITEM MenuItem_SelectMultiPlayer = 
{
    TEXT_MULTIPLAYER,                       // Text label index

    0,                                      // Space needed to draw item data
    &Menu_Connection,                       // Data
    NULL,                                   // Draw Function

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    NULL,                                   // Left Action
    NULL,                                   // Right Action
    MenuGoBack,                             // Back Action
    SetRaceMultiPlayer,                     // Forward Action
};

// Edit Mode
MENU_ITEM MenuItem_EditMode = 
{
    TEXT_EDITMODE,                          // Text label index

    MENU_DATA_WIDTH_TEXT,                   // Space needed to draw item data
    &Menu_EnterName,                       // Data
    DrawEditMode,                           // Draw Function

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    SelectPrevEditMode,                     // Left Action
    SelectNextEditMode,                     // Right Action
    MenuGoBack,                             // Back Action
    SetRaceTrial,                           // Forward Action
};

// Load a Replay
MENU_ITEM MenuItem_LoadReplay = 
{
    TEXT_LOAD_REPLAY,                       // Text label index

    0,                                      // Space needed to draw item data
    NULL,                                   // Data
    NULL,                                   // Draw Function

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    SelectPrevEditMode,                     // Left Action
    SelectNextEditMode,                     // Right Action
    MenuGoBack,                             // Back Action
    SetLoadReplay,                          // Forward Action
};


// Create Function
void CreateStartRaceMenu(MENU_HEADER *menuHeader, MENU *menu)
{
    // Initialise variables
    gTitleScreenVars.iCurrentPlayer = 0;
    gTitleScreenVars.numberOfPlayers = 1;
    GameSettings.RandomCars = FALSE;
    GameSettings.RandomTrack = FALSE;

    // Add menu items
    menuHeader->AddMenuItem( &MenuItem_SelectSingleRace);
    menuHeader->AddMenuItem( &MenuItem_SelectChampionship);
    menuHeader->AddMenuItem( &MenuItem_SelectMultiPlayer);
    menuHeader->AddMenuItem( &MenuItem_SelectTrial);
    menuHeader->AddMenuItem( &MenuItem_SelectPractice);
    menuHeader->AddMenuItem( &MenuItem_SelectTraining);

    if (Version == VERSION_DEV) 
    {
        menuHeader->AddMenuItem( &MenuItem_SelectCalcStats);
        menuHeader->AddMenuItem( &MenuItem_LoadReplay);
    }

    if ((StarList.NumFound == StarList.NumTotal && Version == VERSION_RELEASE) || Version == VERSION_DEV) 
    {
        menuHeader->AddMenuItem( &MenuItem_SelectClockworkRace);
    }

    if (AllowEditors || Version == VERSION_DEV) 
    {
        menuHeader->AddMenuItem( &MenuItem_EditMode);
    }
}

// Utility

BOOL SetLoadReplay(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    GameSettings.GameType = GAMETYPE_REPLAY;
    g_bTitleScreenRunGame = TRUE;
    return TRUE;
}

BOOL SetRaceChampionship(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{

    GameSettings.GameType = GAMETYPE_CHAMPIONSHIP;
    MenuGoForward(menuHeader, menu, menuItem);  
    return TRUE;
}

BOOL SetRaceSingle(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{

    GameSettings.GameType = GAMETYPE_SINGLE;
    GameSettings.RandomCars = gTitleScreenVars.RandomCars;
    GameSettings.RandomTrack = gTitleScreenVars.RandomTrack;
    MenuGoForward(menuHeader, menu, menuItem);  
    return TRUE;
}

BOOL SetRaceClockwork(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    GameSettings.GameType = GAMETYPE_CLOCKWORK;
    GameSettings.RandomTrack = gTitleScreenVars.RandomTrack;
    MenuGoForward(menuHeader, menu, menuItem);  
    return TRUE;
}

BOOL SetRaceTrial(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{

    GameSettings.GameType = GAMETYPE_TRIAL;
    MenuGoForward(menuHeader, menu, menuItem);  
    return TRUE;
}

BOOL SetRaceBattle(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    GameSettings.GameType = GAMETYPE_NETWORK_BATTLETAG;
    MenuGoForward(menuHeader, menu, menuItem);  
    return TRUE;
}

BOOL SetRacePractice(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{

    GameSettings.GameType = GAMETYPE_PRACTICE;
    MenuGoForward(menuHeader, menu, menuItem);  
    return TRUE;
}

BOOL SetRaceTraining(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{

    gTrackScreenLevelNum = gTitleScreenVars.iLevelNum = LEVEL_STUNT_ARENA;

    GameSettings.GameType = GAMETYPE_TRAINING;
    MenuGoForward(menuHeader, menu, menuItem);  
    return TRUE;
}

BOOL SetRaceCalcStats(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    GameSettings.GameType = GAMETYPE_CALCSTATS;
    g_bTitleScreenRunGame = TRUE;
    MenuGoForward(menuHeader, menu, menuItem);  
    return TRUE;
}


BOOL SetRaceMultiPlayer(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    GameSettings.GameType = GAMETYPE_NETWORK_RACE;
    GameSettings.RandomCars = gTitleScreenVars.RandomCars;
    GameSettings.RandomTrack = gTitleScreenVars.RandomTrack;
    MenuGoForward(menuHeader, menu, menuItem);
    return TRUE;
}

BOOL SelectPrevEditMode(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    EditMode--;
    if (EditMode < 0) EditMode = EDIT_NUM - 1;
    if (EditMode == EDIT_NUM) EditMode = 0;
    return TRUE;
}

BOOL SelectNextEditMode(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    EditMode++;
    if (EditMode < 0) EditMode = EDIT_NUM - 1;
    if (EditMode == EDIT_NUM) EditMode = 0;
    return TRUE;
}

////////////////////////////////////////////////////////////////
//
// Multi player game type menu
//
////////////////////////////////////////////////////////////////
extern MENU Menu_MultiType = 
{
    TEXT_GAMETYPE,
    MENU_DEFAULT,                           // Menu type
    CreateMultiTypeMenu,                    // Create menu function
    HandleMultiTypeMenu,                    // Input handler function
    NULL,                                   // Menu draw function
    MENU_MULTITYPE_XPOS,                    // X coord
    MENU_MULTITYPE_YPOS,                    // Y Coord
};

// Create
void CreateMultiTypeMenu( MENU_HEADER* pMenuHeader, MENU* pMenu )
{
    // Add menu items
    pMenuHeader->AddMenuItem( TEXT_SINGLERACE );
    pMenuHeader->AddMenuItem( TEXT_BATTLETAG, MENU_ITEM_INACTIVE );
}

BOOL HandleMultiTypeMenu( MENU_HEADER* pMenuHeader, DWORD dwInput )
{
    switch( dwInput )
    {
        case MENU_INPUT_UP:
            return SelectPreviousMenuItem( pMenuHeader, pMenuHeader->m_pMenu, NULL );
        
        case MENU_INPUT_DOWN:
            return SelectNextMenuItem( pMenuHeader, pMenuHeader->m_pMenu, NULL );
        
        case MENU_INPUT_BACK:
            GameSettings.GameType = GAMETYPE_NONE;
            g_SelectRaceModeStateEngine.Return( STATEENGINE_TERMINATED );
            return TRUE;

        case MENU_INPUT_SELECT:
            if( pMenuHeader->m_pCurrentItem->TextIndex == TEXT_SINGLERACE )
                GameSettings.GameType = GAMETYPE_NETWORK_RACE;

            if( pMenuHeader->m_pCurrentItem->TextIndex == TEXT_BATTLETAG )
                GameSettings.GameType = GAMETYPE_NETWORK_BATTLETAG;

#ifndef XBOX_DISABLE_NETWORK //$REVISIT: Probably can remove; I don't think we need to call this (level name, etc gets propagated to clients manually, not via DPlay)
            LEVELINFO *levinf = GetLevelInfo(gTitleScreenVars.iLevelNum);
            SetSessionDesc(gTitleScreenVars.nameEnter[0], levinf->Dir, FALSE, GameSettings.GameType, GameSettings.RandomCars, GameSettings.RandomTrack);
#endif // !XBOX_DISABLE_NETWORK
            g_SelectRaceModeStateEngine.Return( STATEENGINE_COMPLETED );
            return TRUE;
    }

    return FALSE;
}





void SetRaceMultiPlayerTypeBattle(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
}

// back to connection menu
void MenuGoBackCloseSession(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
#ifndef XBOX_DISABLE_NETWORK
    // kill play
    DP->Close();
#endif

    // go back
    MenuGoBack(menuHeader, menu, menuItem);
}

////////////////////////////////////////////////////////////////
//
// DrawEditMode
//
////////////////////////////////////////////////////////////////
void DrawEditMode( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem, int itemIndex)
{
    FLOAT xPos, yPos;

    xPos = pMenuHeader->m_XPos + pMenuHeader->m_ItemTextWidth + MENU_TEXT_GAP;
    yPos = pMenuHeader->m_YPos + (MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP) * itemIndex;

    DrawMenuText(xPos,yPos, MENU_TEXT_RGB_NORMAL, EditMenuText[EditMode], pMenuHeader->m_ItemDataWidth);
}







CSelectRaceModeStateEngine g_SelectRaceModeStateEngine;

//-----------------------------------------------------------------------------
// Name: Process()
// Desc: Main control function for the state engine
//-----------------------------------------------------------------------------
HRESULT CSelectRaceModeStateEngine::Process()
{
    enum
    {
        SELECTRACEMODE_STATE_BEGIN = STATEENGINE_STATE_BEGIN,
        SELECTRACEMODE_STATE_MAINLOOP,
    };

    switch( m_State )
    {
        case SELECTRACEMODE_STATE_BEGIN:
            // Initialize state
            GameSettings.GameType = GAMETYPE_NONE;

            // Set the menu and camera settings
            g_pMenuHeader->SetNextMenu( &Menu_MultiType );
            g_pTitleScreenCamera->SetNewPos( TITLESCREEN_CAMPOS_MULTI );

            m_State = SELECTRACEMODE_STATE_MAINLOOP;
            break;

        case SELECTRACEMODE_STATE_MAINLOOP:
            // Nothing to do, as control is in the menus
            break;
    }

    // Handle the menus
    g_pMenuHeader->HandleMenus();

    return S_FALSE;
}



