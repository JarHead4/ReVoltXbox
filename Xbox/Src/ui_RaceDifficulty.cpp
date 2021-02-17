//-----------------------------------------------------------------------------
// File: ui_RaceDifficulty.cpp
//
// Desc: UI implementation
//
// Re-Volt (Generic) Copyright (c) Probe Entertainment 1998
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"
#include "ui_MenuDraw.h"   // DrawSpruBox
#include "main.h"       // TimeStep
#include "Text.h"       // BeginTextState
#include "panel.h"      // SpeedUnits
#include "readinit.h"   // WriteAllCarInfo
#include "LevelLoad.h"  // GAMETYPE

// re-volt specific
#include "ui_menutext.h"   // re-volt strings


// temporary includes?
#include "player.h"         // for ui_TitleScreen.h to work
#include "ui_TitleScreen.h"
#include "ui_StateEngine.h"
#include "ui_RaceDifficulty.h"
#include "ui_Confirm.h"
#include "ui_EnterName.h"

#define MENU_GAMEDIFFICULTY_XPOS        100
#define MENU_GAMEDIFFICULTY_YPOS        150

static void CreateGameDifficultyMenu( MENU_HEADER* pMenuHeader, MENU* menu );
static BOOL HandleGameDifficultyMenu( MENU_HEADER* pMenuHeader, DWORD dwInput );
static void DrawPlayDifficulty(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex);




////////////////////////////////////////////////////////////////
//
// Game Difficulty
//
////////////////////////////////////////////////////////////////
extern MENU Menu_GameDifficulty = 
{
    TEXT_MODE,
    MENU_DEFAULT | MENU_CENTRE_X,           // Menu type
    CreateGameDifficultyMenu,               // Create menu function
    HandleGameDifficultyMenu,               // Input handler function
    NULL,                                   // Menu draw function
    MENU_GAMEDIFFICULTY_XPOS,               // X coord
    MENU_GAMEDIFFICULTY_YPOS,               // Y Coord
};

MENU_ITEM MenuItem_Sim = 
{
    TEXT_MODE_SIMULATION,                   // Text label index
    0,                                      // Space needed to draw item data
    NULL,                                   // Data
    DrawPlayDifficulty,                     // Draw Function
};
MENU_ITEM MenuItem_Arc = 
{
    TEXT_MODE_ARCADE,                       // Text label index
    0,                                      // Space needed to draw item data
    NULL,                                   // Data
    DrawPlayDifficulty,                     // Draw Function
};
MENU_ITEM MenuItem_Con = 
{
    TEXT_MODE_CONSOLE,                      // Text label index
    0,                                      // Space needed to draw item data
    NULL,                                   // Data
    DrawPlayDifficulty,                     // Draw Function
};
MENU_ITEM MenuItem_Kids = 
{
    TEXT_MODE_KIDS,                         // Text label index
    0,                                      // Space needed to draw item data
    NULL,                                   // Data
    DrawPlayDifficulty,                     // Draw Function
};


void CreateGameDifficultyMenu(MENU_HEADER *menuHeader, MENU *menu)
{
    Menu_GameDifficulty.CurrentItemIndex = gTitleScreenVars.playMode;

    menuHeader->AddMenuItem( &MenuItem_Sim );
    menuHeader->AddMenuItem( &MenuItem_Arc );
    menuHeader->AddMenuItem( &MenuItem_Con );

    if( GameSettings.GameType != GAMETYPE_NETWORK_BATTLETAG ) 
    {
        menuHeader->AddMenuItem( &MenuItem_Kids);
    } 
    else 
    {
        if( gTitleScreenVars.playMode == MODE_KIDS ) 
        {
            Menu_GameDifficulty.CurrentItemIndex = MODE_CONSOLE;
        }
    }
}

BOOL HandleGameDifficultyMenu( MENU_HEADER* pMenuHeader, DWORD dwInput )
{
    switch( dwInput )
    {
        case MENU_INPUT_UP:
            return SelectPreviousMenuItem( pMenuHeader, pMenuHeader->m_pMenu, NULL );

        case MENU_INPUT_DOWN:
            return SelectNextMenuItem( pMenuHeader, pMenuHeader->m_pMenu, NULL );

        case MENU_INPUT_BACK:
            g_SelectDifficultyStateEngine.Return( STATEENGINE_TERMINATED );
            return TRUE;

        case MENU_INPUT_SELECT:
            gTitleScreenVars.playMode = Menu_GameDifficulty.CurrentItemIndex;
            g_SelectDifficultyStateEngine.Return( STATEENGINE_COMPLETED );
            return TRUE;
    }

    return FALSE;
}




////////////////////////////////////////////////////////////////
//
// Draw Play Difficulty
//
////////////////////////////////////////////////////////////////
void DrawPlayDifficulty( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem, int itemIndex)
{
    gTitleScreenVars.playMode = pMenu->CurrentItemIndex;
    swprintf(MenuBuffer, TEXT_TABLE(TEXT_SIMULATION_HELPSTRING + gTitleScreenVars.playMode));

    FLOAT fTextWidth = g_pFont->GetTextWidth( MenuBuffer );

    FLOAT xPos = (640.0f - fTextWidth) / 2;
    FLOAT yPos = pMenuHeader->m_YPos + 150;

    DrawNewSpruBox( gMenuWidthScale  * (xPos - MENU_LEFT_PAD ),
                    gMenuHeightScale * (yPos - MENU_TOP_PAD ),
                    gMenuWidthScale  * (fTextWidth + MENU_LEFT_PAD + MENU_RIGHT_PAD ),
                    gMenuHeightScale * (MENU_TEXT_HEIGHT + MENU_TOP_PAD + MENU_BOTTOM_PAD ) );

    BeginTextState();
    DrawMenuText(xPos, yPos, MENU_TEXT_RGB_CHOICE, MenuBuffer);
}





CSelectDifficultyStateEngine g_SelectDifficultyStateEngine;

//-----------------------------------------------------------------------------
// Name: Process()
// Desc: Main control function for the state engine
//-----------------------------------------------------------------------------
HRESULT CSelectDifficultyStateEngine::Process()
{
    enum
    {
        RACEDIFFICULTY_STATE_BEGIN = STATEENGINE_STATE_BEGIN,
        RACEDIFFICULTY_STATE_MAINLOOP,
    };

    switch( m_State )
    {
        case RACEDIFFICULTY_STATE_BEGIN:
            g_pMenuHeader->SetNextMenu( &Menu_GameDifficulty );

            m_State = RACEDIFFICULTY_STATE_MAINLOOP;
            break;

        case RACEDIFFICULTY_STATE_MAINLOOP:
            // Nothing to do, as control is in the menus
            break;
    }

    // Handle the menus
    g_pMenuHeader->HandleMenus();

    return S_FALSE;
}





