//-----------------------------------------------------------------------------
// File: ui_ProgressTable.h
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
#include "Panel.h"           // DrawPanelSprite

// re-volt specific
#include "ui_menutext.h"   // re-volt strings

// temporary includes?
#include "player.h"         // for ui_TitleScreen.h to work
#include "ui_TitleScreen.h"
#include "ui_StateEngine.h"
#include "ui_ProgressTable.h"


static VOID CreateProgressTableMenu( MENU_HEADER* pMenuHeader, MENU* pMenu );
static BOOL HandleProgressTableMenu( MENU_HEADER* pMenuHeader, DWORD dwInput );
static VOID DrawProgressTable(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex);

#define MENU_PROGRESSTABLE_XPOS         0
#define MENU_PROGRESSTABLE_YPOS         0




////////////////////////////////////////////////////////////////
//
// Progress Table
//
////////////////////////////////////////////////////////////////
extern MENU Menu_ProgressTable = 
{
    TEXT_PROGRESSTABLE,
    MENU_DEFAULT | MENU_CENTRE_X,           // Menu type
    CreateProgressTableMenu,                // Create menu function
    HandleProgressTableMenu,                // Menu input handler
    NULL,                                   // Menu draw function
    MENU_PROGRESSTABLE_XPOS,                // X coord
    MENU_PROGRESSTABLE_YPOS,                // Y Coord
};

// ProgressTable - dummy menu item
MENU_ITEM MenuItem_ProgressTableDummy = 
{
    TEXT_NONE,                              // Text label index
    640,                                    // Space needed to draw item data
    NULL,                                   // Data (Menu to set up game and then run it)
    DrawProgressTable,                      // Draw Function
};


// Create
void CreateProgressTableMenu( MENU_HEADER* pMenuHeader, MENU* pMenu )
{
    pMenuHeader->AddMenuItem( &MenuItem_ProgressTableDummy);
    pMenuHeader->m_pMenu->dwFlags |= MENU_NOBOX;
}


BOOL HandleProgressTableMenu( MENU_HEADER* pMenuHeader, DWORD dwInput )
{
    switch( dwInput )
    {
        case MENU_INPUT_BACK:
			g_ProgressTableStateEngine.Return( STATEENGINE_TERMINATED );
            return TRUE;
    }

    return FALSE;
}




/////////////////////////////////////////////////////////////////////
//
// Draw ProgressTable
//
/////////////////////////////////////////////////////////////////////
#define TICK_TU     (244.0f / 256.0f)
#define CROSS_TU    (231.0f / 256.0f)

void DrawProgressTable( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem, int itemIndex)
{
    long i, cup;
    FLOAT xpos, ypos, y;
    LEVELINFO *li;

// draw spru's

    xpos = pMenuHeader->m_XPos + 60;
    ypos = pMenuHeader->m_YPos + 112;

    DrawNewSpruBox( xpos * RenderSettings.GeomScaleX - 10,
                    ypos * RenderSettings.GeomScaleY - 10,
                    (FLOAT)(640 - 120) * RenderSettings.GeomScaleX + 20,
                    (FLOAT)(480 - 208) * RenderSettings.GeomScaleY + 20 );

    DrawNewSpruBox( xpos * RenderSettings.GeomScaleX - 10,
                    (ypos + 292) * RenderSettings.GeomScaleY - 10,
                    (FLOAT)(640 - 120) * RenderSettings.GeomScaleX + 20,
                    (FLOAT)(24) * RenderSettings.GeomScaleY + 20 );

	// pMenu headings
    BeginTextState();

    DumpTextReal(xpos + 4, ypos + 4, MENU_TEXT_WIDTH, MENU_TEXT_HEIGHT, MENU_COLOR_WHITE, TEXT_TABLE(TEXT_PROGRESS_TRACKS));

    DumpTextReal(xpos + 4 + MENU_TEXT_WIDTH * 22 - wcslen(TEXT_TABLE(TEXT_PROGRESS_WONRACE_TOP)) * 4, ypos + 4, MENU_TEXT_WIDTH, MENU_TEXT_HEIGHT, MENU_COLOR_WHITE, TEXT_TABLE(TEXT_PROGRESS_WONRACE_TOP));
    DumpTextReal(xpos + 4 + MENU_TEXT_WIDTH * 22 - wcslen(TEXT_TABLE(TEXT_PROGRESS_WONRACE_BOTTOM)) * 4, ypos + 4 + MENU_TEXT_HEIGHT, MENU_TEXT_WIDTH, MENU_TEXT_HEIGHT, MENU_COLOR_WHITE, TEXT_TABLE(TEXT_PROGRESS_WONRACE_BOTTOM));

    DumpTextReal(xpos + 4 + MENU_TEXT_WIDTH * 30 - wcslen(TEXT_TABLE(TEXT_PROGRESS_NORMAL)) * 4, ypos + 4, MENU_TEXT_WIDTH, MENU_TEXT_HEIGHT, MENU_COLOR_WHITE, TEXT_TABLE(TEXT_PROGRESS_NORMAL));
    DumpTextReal(xpos + 4 + MENU_TEXT_WIDTH * 40 - wcslen(TEXT_TABLE(TEXT_PROGRESS_REVERSE)) * 4, ypos + 4, MENU_TEXT_WIDTH, MENU_TEXT_HEIGHT, MENU_COLOR_WHITE, TEXT_TABLE(TEXT_PROGRESS_REVERSE));
    DumpTextReal(xpos + 4 + MENU_TEXT_WIDTH * 50 - wcslen(TEXT_TABLE(TEXT_PROGRESS_MIRROR)) * 4, ypos + 4, MENU_TEXT_WIDTH, MENU_TEXT_HEIGHT, MENU_COLOR_WHITE, TEXT_TABLE(TEXT_PROGRESS_MIRROR));

    DumpTextReal(xpos + 4 + MENU_TEXT_WIDTH * 60 - wcslen(TEXT_TABLE(TEXT_PROGRESS_STARS)) * 4, ypos + 4, MENU_TEXT_WIDTH, MENU_TEXT_HEIGHT, MENU_COLOR_WHITE, TEXT_TABLE(TEXT_PROGRESS_STARS));

	// loop thru each cup level
    cup = -1;
    y = ypos + 4 + MENU_TEXT_HEIGHT;
    for (i = 0 ; i < LEVEL_NCUP_LEVELS ; i++)
    {
        li = GetLevelInfo(i);

        // next cup?
        if (li->LevelClass != cup)
        {
            y += MENU_TEXT_HEIGHT;
            cup = li->LevelClass;
            DumpTextReal(xpos + 4, y, MENU_TEXT_WIDTH, MENU_TEXT_HEIGHT, MENU_COLOR_WHITE, TEXT_TABLE(TEXT_COMPETE_CUP_DEFAULT + cup));
            DrawPanelSprite(xpos + 4 + wcslen(TEXT_TABLE(TEXT_COMPETE_CUP_DEFAULT + cup)) * MENU_TEXT_WIDTH + 4, y, 12.0f, 12.0f, IsCupCompleted(cup) ? TICK_TU : CROSS_TU, 244.0f / 256.0f, 11.0f / 256.0f, 11.0f / 256.0f, 0xffffff);
            y += MENU_TEXT_HEIGHT;
        }

        // level name
        swprintf( MenuBuffer, L"%s", li->strName );
        DumpTextReal(xpos + 4 + MENU_TEXT_WIDTH, y, MENU_TEXT_WIDTH, MENU_TEXT_HEIGHT, MENU_COLOR_GRAY, MenuBuffer);

        // won race
        DrawPanelSprite(xpos + 172, y, 12.0f, 12.0f, IsSecretWonSingleRace(i) ? TICK_TU : CROSS_TU, 244.0f / 256.0f, 11.0f / 256.0f, 11.0f / 256.0f, 0xffffff);

        // challenge normal
        DrawPanelSprite(xpos + 240, y, 12.0f, 12.0f, IsSecretBeatTimeTrial(i) ? TICK_TU : CROSS_TU, 244.0f / 256.0f, 11.0f / 256.0f, 11.0f / 256.0f, 0xffffff);

        // challenge reversed
        DrawPanelSprite(xpos + 320, y, 12.0f, 12.0f, IsSecretBeatTimeTrialReverse(i) ? TICK_TU : CROSS_TU, 244.0f / 256.0f, 11.0f / 256.0f, 11.0f / 256.0f, 0xffffff);

        // challenge mirrored
        DrawPanelSprite(xpos + 400, y, 12.0f, 12.0f, IsSecretBeatTimeTrialMirror(i) ? TICK_TU : CROSS_TU, 244.0f / 256.0f, 11.0f / 256.0f, 11.0f / 256.0f, 0xffffff);

        // found practice stars
        DrawPanelSprite(xpos + 476, y, 12.0f, 12.0f, IsSecretFoundPractiseStars(i) ? TICK_TU : CROSS_TU, 244.0f / 256.0f, 11.0f / 256.0f, 11.0f / 256.0f, 0xffffff);

        // next line
        y += MENU_TEXT_HEIGHT;
    }

	// show stunt stars found
    swprintf(MenuBuffer, L"%s - %s ", TEXT_TABLE(TEXT_TRAINING), TEXT_TABLE(TEXT_LOADSCREEN_STARSCOLLECTED));
    DumpTextReal(xpos + 4, ypos + 292 + 6, MENU_TEXT_WIDTH, MENU_TEXT_HEIGHT, MENU_COLOR_WHITE, MenuBuffer);
    swprintf(MenuBuffer2, L"%ld %s %ld", StarList.NumFound, TEXT_TABLE(TEXT_OF), StarList.NumTotal);
    DumpTextReal(xpos + 4 + MENU_TEXT_WIDTH * wcslen(MenuBuffer), ypos + 292 + 6, MENU_TEXT_WIDTH, MENU_TEXT_HEIGHT, MENU_COLOR_GREEN, MenuBuffer2);
}





CProgressTableStateEngine g_ProgressTableStateEngine;

//-----------------------------------------------------------------------------
// Name: Process()
// Desc: Main control function for the state engine
//-----------------------------------------------------------------------------
HRESULT CProgressTableStateEngine::Process()
{
    enum
    {
        PROGRESSTABLE_STATE_BEGIN = STATEENGINE_STATE_BEGIN,
        PROGRESSTABLE_STATE_MAINLOOP,
    };

    switch( m_State )
    {
        case PROGRESSTABLE_STATE_BEGIN:
            g_pMenuHeader->SetNextMenu( &Menu_ProgressTable );
            g_pTitleScreenCamera->SetNewPos( TITLESCREEN_CAMPOS_TROPHYALL );

            m_State = PROGRESSTABLE_STATE_MAINLOOP;
            break;

        case PROGRESSTABLE_STATE_MAINLOOP:
			// Nothing to do, as control is in menu code
            break;
    }

    // Handle active menus
    g_pMenuHeader->HandleMenus();

    return S_FALSE;
}





