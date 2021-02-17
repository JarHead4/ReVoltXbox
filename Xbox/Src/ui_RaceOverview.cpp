//-----------------------------------------------------------------------------
// File: ui_RaceOverview.cpp
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
#include "LevelLoad.h"      // GAMETYPE_*
#include "Text.h"           // BeginTextState

// re-volt specific
#include "ui_menutext.h"   // re-volt strings

// temporary includes?
#include "player.h"         // for ui_TitleScreen.h to work
#include "ui_TitleScreen.h"
#include "ui_StateEngine.h"
#include "ui_RaceOverview.h"

static void CreateOverviewMenu( MENU_HEADER* pMenuHeader, MENU* pMenu );
static BOOL HandleOverviewMenu( MENU_HEADER* pMenuHeader, DWORD dwInput );
static void DrawOverview(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex);

#define MENU_OVERVIEW_XPOS              100
#define MENU_OVERVIEW_YPOS              150




////////////////////////////////////////////////////////////////
//
// Selection Overview
//
////////////////////////////////////////////////////////////////
extern MENU Menu_Overview = 
{
    TEXT_SUMMARY,
    MENU_DEFAULT,                           // Menu type
    CreateOverviewMenu,                     // Create menu function
    HandleOverviewMenu,                     // Input handler function
    NULL,                                   // Menu draw function
    MENU_OVERVIEW_XPOS,                     // X coord
    MENU_OVERVIEW_YPOS,                     // Y Coord
};

// Display Pre-Race Summary of selections
MENU_ITEM MenuItem_RaceOverview = 
{
    TEXT_NONE,                              // Text label index
    MENU_TEXT_WIDTH * 34,                   // Space needed to draw item data
    NULL,                                   // Data
    DrawOverview,                           // Draw Function
};


// Create
void CreateOverviewMenu(MENU_HEADER *menuHeader, MENU *menu)
{
    menuHeader->m_pMenu->dwFlags |= MENU_NOBOX;  
    
    menuHeader->AddMenuItem( &MenuItem_RaceOverview );
}

BOOL HandleOverviewMenu( MENU_HEADER* pMenuHeader, DWORD dwInput )
{
    switch( dwInput )
    {
        case MENU_INPUT_BACK:
            g_RaceOverviewStateEngine.Return( STATEENGINE_TERMINATED );
            return TRUE;

        case MENU_INPUT_SELECT:
            // Overview is just a GUI confirmation screen, 
            // the calling state engine will control what needs to happen to prepare and start the race.
            g_RaceOverviewStateEngine.Return( STATEENGINE_COMPLETED );
            return TRUE;
    }

    return FALSE;
}




////////////////////////////////////////////////////////////////
//
// Draw Overview
//
////////////////////////////////////////////////////////////////
extern long GameModeTextIndex[];

void DrawOverview( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem, int itemIndex)
{
    FLOAT xPos, xPos2, yPos, ySize, xSize;
    LEVELINFO *levelInfo;

    levelInfo = GetLevelInfo(gTitleScreenVars.iLevelNum);
    xPos = pMenuHeader->m_XPos;
    yPos = pMenuHeader->m_YPos;

    FLOAT fItemWidth = 0.0f;
    fItemWidth = max( fItemWidth, g_pFont->GetTextWidth( TEXT_TABLE(TEXT_LOADSCREEN_GAMEMODE) ) );
    fItemWidth = max( fItemWidth, g_pFont->GetTextWidth( TEXT_TABLE(TEXT_RACERS) ) );
    fItemWidth = max( fItemWidth, g_pFont->GetTextWidth( TEXT_TABLE(TEXT_LOADSCREEN_TRACK) ) );
    fItemWidth = max( fItemWidth, g_pFont->GetTextWidth( TEXT_TABLE(TEXT_LENGTH) ) );
    fItemWidth = max( fItemWidth, g_pFont->GetTextWidth( TEXT_TABLE(TEXT_NUMLAPS) ) );
    fItemWidth = max( fItemWidth, g_pFont->GetTextWidth( TEXT_TABLE(TEXT_CAR) ) );
//$REMOVED    fItemWidth = max( fItemWidth, g_pFont->GetTextWidth( TEXT_TABLE(TEXT_LOADSCREEN_WONRACE) ) );

    FLOAT fDataWidth = 0.0f;
    fDataWidth = max( fDataWidth, g_pFont->GetTextWidth( TEXT_TABLE(GameModeTextIndex[GameSettings.GameType]) ) );
    fDataWidth = max( fDataWidth, g_pFont->GetTextWidth( levelInfo->strName ) );
    swprintf( MenuBuffer, L"%S", CarInfo[gTitleScreenVars.PlayerData[0].iCarNum].Name );
    fDataWidth = max( fDataWidth, g_pFont->GetTextWidth( MenuBuffer ) );

    xPos2 = xPos + fItemWidth + 20;

    xSize = fItemWidth + fDataWidth + 20;
    ySize = MENU_TEXT_HEIGHT*7 + MENU_TEXT_VSKIP*6;

    DrawNewSpruBox( gMenuWidthScale  * (xPos - MENU_LEFT_PAD ),
                    gMenuHeightScale * (yPos - MENU_TOP_PAD ),
                    gMenuWidthScale  * (xSize + MENU_LEFT_PAD + MENU_RIGHT_PAD ),
                    gMenuHeightScale * (ySize + MENU_TOP_PAD + MENU_BOTTOM_PAD ) );

    BeginTextState();

    if (GameSettings.GameType == GAMETYPE_CHAMPIONSHIP)
    {
        swprintf(MenuBuffer, L"%s", TEXT_TABLE(TEXT_BRONZECUP + gTitleScreenVars.CupType - 1));
        //DrawMenuText(xPos, yPos, 1.0, MENU_TEXT_RGB_NORMAL,MENU_MAIN_ALPHA, MenuBuffer,0);
        DrawMenuText(xPos, yPos, MENU_TEXT_RGB_HILITE, MenuBuffer );
        yPos += MENU_TEXT_HEIGHT + 4*MENU_TEXT_VSKIP;

        // Finish 3rd or better 
        DrawMenuText(xPos, yPos, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_CHAMP_FINISH_RACE1));
        yPos += MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP;
        DrawMenuText(xPos, yPos, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_CHAMP_FINISH_RACE2));
        yPos += MENU_TEXT_HEIGHT + 4*MENU_TEXT_VSKIP;

        // Finish 1st to get next cup
        if (gTitleScreenVars.CupType != RACE_CLASS_SPECIAL) {
            DrawMenuText(xPos, yPos, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_CHAMP_FINISH_CHAMP1));
            yPos += MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP;
            DrawMenuText(xPos, yPos, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_CHAMP_FINISH_CHAMP2));
            yPos += MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP;
        } else {
            DrawMenuText(xPos, yPos, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_CHAMP_FINISH_CHAMP3));
            yPos+= MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP;
            DrawMenuText(xPos, yPos, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_CHAMP_FINISH_CHAMP4));
            yPos += MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP;
        }
        return;
    }

    // Game mode
    DrawMenuText(xPos, yPos, MENU_TEXT_RGB_HILITE, TEXT_TABLE(TEXT_LOADSCREEN_GAMEMODE));
    if (GameModeTextIndex[GameSettings.GameType] != TEXT_NONE)
    {
        DrawMenuText(xPos2, yPos, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(GameModeTextIndex[GameSettings.GameType]));
    }
    yPos += MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP;

    // Number of players / cpu cars
    if( (GameSettings.GameType == GAMETYPE_SINGLE) )
    {
        DrawMenuText(xPos, yPos, MENU_TEXT_RGB_HILITE, TEXT_TABLE(TEXT_RACERS));
        swprintf(MenuBuffer, L"%d (%d %s)", gTitleScreenVars.numberOfCars, 
                                            gTitleScreenVars.numberOfCars - gTitleScreenVars.numberOfPlayers, 
                                            TEXT_TABLE(TEXT_CPUOPPONENT));
        DrawMenuText(xPos2, yPos, MENU_TEXT_RGB_NORMAL, MenuBuffer);
        yPos += MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP;
    }

    // Track
    if (GameSettings.GameType != GAMETYPE_TRAINING)
    {
        DrawMenuText(xPos, yPos, MENU_TEXT_RGB_HILITE, TEXT_TABLE(TEXT_LOADSCREEN_TRACK));
        WCHAR strLevelName[80];
        if( gTitleScreenVars.RandomTrack )
            swprintf( strLevelName, L"%s", TEXT_TABLE(TEXT_RANDOM) );
        else
            swprintf( strLevelName, L"%s", levelInfo->strName );

        swprintf(MenuBuffer, L"%s %s%s", strLevelName,
                                         gTitleScreenVars.reverse ? TEXT_TABLE(TEXT_REVERSE_ABREV) : L"", 
                                         gTitleScreenVars.mirror ? TEXT_TABLE(TEXT_MIRROR_ABREV) : L"" );
        DrawMenuText(xPos2, yPos, MENU_TEXT_RGB_NORMAL, MenuBuffer);
        yPos += MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP;
    }

    // Track Length
    if (GameSettings.GameType != GAMETYPE_TRAINING && GameSettings.GameType != GAMETYPE_NETWORK_BATTLETAG)
    {
        swprintf(MenuBuffer, L"%s:", TEXT_TABLE(TEXT_LENGTH));
        DrawMenuText(xPos, yPos, MENU_TEXT_RGB_HILITE, MenuBuffer);
        swprintf(MenuBuffer, L"%ld%s", (long)levelInfo->Length, TEXT_TABLE(TEXT_METERS_ABREV));
        DrawMenuText(xPos2, yPos, MENU_TEXT_RGB_NORMAL, MenuBuffer);
        yPos += MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP;
    }

    // Number of Laps
    if (GameSettings.GameType == GAMETYPE_CHAMPIONSHIP || GameSettings.GameType == GAMETYPE_SINGLE || GameSettings.GameType == GAMETYPE_CLOCKWORK || GameSettings.GameType == GAMETYPE_NETWORK_RACE)
    {
        swprintf(MenuBuffer, L"%s:", TEXT_TABLE(TEXT_NUMLAPS));
        DrawMenuText(xPos, yPos, MENU_TEXT_RGB_HILITE, MenuBuffer);
        swprintf(MenuBuffer, L"%ld", gTitleScreenVars.numberOfLaps);
        DrawMenuText(xPos2, yPos, MENU_TEXT_RGB_NORMAL, MenuBuffer);
        yPos += MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP;
    }

    // Players Car
    if( (GameSettings.GameType != GAMETYPE_DEMO) && (GameSettings.GameType != GAMETYPE_CLOCKWORK) )
    {
        swprintf(MenuBuffer, L"%s:", TEXT_TABLE(TEXT_CAR));
        DrawMenuText(xPos, yPos, MENU_TEXT_RGB_HILITE, MenuBuffer);

        swprintf(MenuBuffer, L"%S", CarInfo[gTitleScreenVars.PlayerData[0].iCarNum].Name);
        DrawMenuText(xPos2, yPos, MENU_TEXT_RGB_NORMAL, MenuBuffer);
        yPos += MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP;
    }

//$REMOVED - since we don't unlock levels, it's misleading to players about the
//           relevance of having won a race
//    // Whether have completed
//    if (GameSettings.GameType == GAMETYPE_SINGLE && gTitleScreenVars.iLevelNum < LEVEL_NCUP_LEVELS)
//    {
//        DrawMenuText(xPos, yPos, MENU_TEXT_RGB_HILITE, TEXT_TABLE(TEXT_LOADSCREEN_WONRACE));
//        DrawMenuText(xPos2, yPos, MENU_TEXT_RGB_NORMAL, IsSecretWonSingleRace(gTitleScreenVars.iLevelNum) ? TEXT_TABLE(TEXT_YES) : TEXT_TABLE(TEXT_NO));
//        yPos += MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP;
//    }
//$END_REMOVAL

    if (GameSettings.GameType == GAMETYPE_PRACTICE)
    {
        DrawMenuText(xPos, yPos, MENU_TEXT_RGB_HILITE, TEXT_TABLE(TEXT_LOADSCREEN_FOUNDSTAR));
        DrawMenuText(xPos2, yPos, MENU_TEXT_RGB_NORMAL, IsSecretFoundPractiseStars(gTitleScreenVars.iLevelNum) ? TEXT_TABLE(TEXT_YES) : TEXT_TABLE(TEXT_NO));
        yPos += MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP;
    }

    if (GameSettings.GameType == GAMETYPE_TRAINING)
    {
        swprintf(MenuBuffer, L"%s:", TEXT_TABLE(TEXT_PROGRESS_STARS));
        DrawMenuText(xPos, yPos, MENU_TEXT_RGB_HILITE, MenuBuffer);
        swprintf(MenuBuffer, L"%ld %s %ld", StarList.NumFound, TEXT_TABLE(TEXT_OF), StarList.NumTotal);
        DrawMenuText(xPos2, yPos, MENU_TEXT_RGB_NORMAL, MenuBuffer);
        yPos += MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP;
    }
}




CRaceOverviewStateEngine g_RaceOverviewStateEngine;

//-----------------------------------------------------------------------------
// Name: Process()
// Desc: Main control function for the state engine
//-----------------------------------------------------------------------------
HRESULT CRaceOverviewStateEngine::Process()
{
    enum
    {
        OVERVIEW_STATE_BEGIN = STATEENGINE_STATE_BEGIN,
        OVERVIEW_STATE_MAINLOOP,
    };

    switch( m_State )
    {
        case OVERVIEW_STATE_BEGIN:
            g_pMenuHeader->SetNextMenu( &Menu_Overview );

            if( GameSettings.GameType != GAMETYPE_CHAMPIONSHIP ) 
                g_pTitleScreenCamera->SetNewPos( TITLESCREEN_CAMPOS_SUMMARY );
            else 
                g_pTitleScreenCamera->SetNewPos( TITLESCREEN_CAMPOS_TROPHYALL );
            
            m_State = OVERVIEW_STATE_MAINLOOP;
            break;

        case OVERVIEW_STATE_MAINLOOP:
            // Nothing to do, as control is in the menus
            break;
    }

    // Handle the menus
    g_pMenuHeader->HandleMenus();

    return S_FALSE;
}





