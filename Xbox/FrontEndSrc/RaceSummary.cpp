//-----------------------------------------------------------------------------
// File: RaceSummary.cpp
//
// Desc: RaceSummary.cpp
//
// Hist: 02.08.02 - ported/created
//
// Copyright (c)  Microsoft Corporation.  All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"
#include "menu2.h"
#include "MenuDraw.h"   // DrawSpruBox
#include "settings.h"   // RegistrySettings

// re-volt specific
#include "menutext.h"   // re-volt strings

// temporary includes?
#include "titlescreen.h"  // TITLESCREEN_CAMPOS_ & g_bTitleScreenRunDemo

#define MENU_OVERVIEW_XPOS              Real(100)
#define MENU_OVERVIEW_YPOS              Real(150)

static void CreateOverviewMenu(MENU_HEADER *menuHeader, MENU *menu);

static void DrawOverview(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex);

// Menu
MENU Menu_Overview = 
{
    TEXT_SUMMARY,
    MENU_IMAGE_NONE,                        // Menu title bar image enum
    TITLESCREEN_CAMPOS_SUMMARY,                      // Camera pos index
    MENU_DEFAULT | MENU_CENTRE_X,           // Menu type
    SPRU_COL_RANDOM,                        // Spru colour
    CreateOverviewMenu,                     // Create menu function
    MENU_OVERVIEW_XPOS,                     // X coord
    MENU_OVERVIEW_YPOS,                     // Y Coord
    0,                                      // Current Item index
    NULL,                                   // Parent Menu (OverWritten in Create)
};

// Car Select Menu Item
MENU_ITEM MenuItem_Overview = 
{
    TEXT_NONE,                              // Text label index

    MENU_TEXT_WIDTH * 34,               // Space needed to draw item data

    &g_bTitleScreenRunGame,                  // Data
    DrawOverview,                           // Draw Function

	NULL,								    // Input handler
	0,                                      // Flags

    NULL,                                   // Up Action
    NULL,                                   // Down Action
    NULL,                                   // Left Action
    NULL,                                   // Right Action
    MenuGoBack,                             // Back Action
    ToggleMenuDataOn,                       // Forward Action
};

// Create
void CreateOverviewMenu(MENU_HEADER *menuHeader, MENU *menu)
{

    g_bMenuDrawMenuBox = FALSE;   
    
    if (GameSettings.GameType != GAMETYPE_CHAMPIONSHIP) 
	{
        Menu_Overview.CamPosIndex = TITLESCREEN_CAMPOS_SUMMARY;
    } 
	else 
	{
        Menu_Overview.CamPosIndex = TITLESCREEN_CAMPOS_ALL_TROPHIES;
    }

    AddMenuItem(menuHeader, menu, &MenuItem_Overview);

}

////////////////////////////////////////////////////////////////
//
// Draw Overview
//
////////////////////////////////////////////////////////////////
extern long GameModeTextIndex[];

void DrawOverview(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex)
{
    REAL xPos, xPos2, yPos, ySize, xSize;
    LEVELINFO *levelInfo;

    levelInfo = GetLevelInfo(g_TitleScreenData.iLevelNum);
    xPos = menuHeader->XPos;
    yPos = menuHeader->YPos;

    xSize = menuHeader->ItemDataWidth;
    ySize = (MENU_TEXT_HEIGHT * 5 + MENU_TEXT_VSKIP * 10 + MENU_TEXT_GAP * 2);

    xPos2 = xPos + MENU_TEXT_WIDTH * 12;//xSize/2;

    DrawSpruBox(
        gMenuWidthScale * (xPos - MENU_TEXT_GAP), 
        gMenuHeightScale * (yPos - MENU_TEXT_GAP),
        gMenuWidthScale * (xSize + MENU_TEXT_GAP * 2),
        gMenuHeightScale * ySize, 
        menuHeader->SpruColIndex, 0);

    if (GameSettings.GameType == GAMETYPE_CHAMPIONSHIP)
    {
        sprintf(MenuBuffer, "%s", TEXT_TABLE(TEXT_BRONZECUP + g_TitleScreenData.CupType - 1));
        //DrawMenuText(xPos, yPos, MENU_TEXT_RGB_NORMAL,MENU_MAIN_ALPHA, MenuBuffer);
        DrawMenuText(xPos, yPos, MENU_TEXT_RGB_HILITE, MenuBuffer );
        yPos += MENU_TEXT_HEIGHT + 4*MENU_TEXT_VSKIP;

        // Finish 3rd or better 
        DrawMenuText(xPos, yPos, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_CHAMP_FINISH_RACE1));
        yPos += MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP;
        DrawMenuText(xPos, yPos, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_CHAMP_FINISH_RACE2));
        yPos += MENU_TEXT_HEIGHT + 4*MENU_TEXT_VSKIP;

        // Finish 1st to get next cup
        if (g_TitleScreenData.CupType != RACE_CLASS_SPECIAL) {
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
    if ((GameSettings.GameType == GAMETYPE_SINGLE)) {
        DrawMenuText(xPos, yPos, MENU_TEXT_RGB_HILITE, TEXT_TABLE(TEXT_RACERS));
        sprintf(MenuBuffer, "%d (%d %s)", 
            g_TitleScreenData.numberOfCars, 
            g_TitleScreenData.numberOfCars - g_TitleScreenData.numberOfPlayers, 
            TEXT_TABLE(TEXT_CPUOPPONENT)
            );
        DrawMenuText(xPos2, yPos, MENU_TEXT_RGB_NORMAL, MenuBuffer);
        yPos += MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP;
    }

    // Track
    if (GameSettings.GameType != GAMETYPE_TRAINING)
    {
        DrawMenuText(xPos, yPos, MENU_TEXT_RGB_HILITE, TEXT_TABLE(TEXT_LOADSCREEN_TRACK));
        sprintf(MenuBuffer, "%s %s%s", 
            g_TitleScreenData.RandomTrack ? TEXT_TABLE(TEXT_RANDOM) : levelInfo->Name, 
            g_TitleScreenData.reverse ? TEXT_TABLE(TEXT_REVERSE_ABREV) : "", g_TitleScreenData.mirror ? TEXT_TABLE(TEXT_MIRROR_ABREV) : "");
        DrawMenuText(xPos2, yPos, MENU_TEXT_RGB_NORMAL, MenuBuffer);
        yPos += MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP;
    }

    // Track Length
    if (GameSettings.GameType != GAMETYPE_TRAINING && GameSettings.GameType != GAMETYPE_BATTLE)
    {
        sprintf(MenuBuffer, "%s:", TEXT_TABLE(TEXT_LENGTH));
        DrawMenuText(xPos, yPos, MENU_TEXT_RGB_HILITE, MenuBuffer);
        sprintf(MenuBuffer, "%ldm", (long)levelInfo->Length);
        DrawMenuText(xPos2, yPos, MENU_TEXT_RGB_NORMAL, MenuBuffer);
        yPos += MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP;
    }

    // Number of Laps
    if (GameSettings.GameType == GAMETYPE_CHAMPIONSHIP || GameSettings.GameType == GAMETYPE_SINGLE || GameSettings.GameType == GAMETYPE_CLOCKWORK || GameSettings.GameType == GAMETYPE_MULTI)
    {
        sprintf(MenuBuffer, "%s:", TEXT_TABLE(TEXT_NUMLAPS));
        DrawMenuText(xPos, yPos, MENU_TEXT_RGB_HILITE, MenuBuffer);
        sprintf(MenuBuffer, "%ld", g_TitleScreenData.numberOfLaps);
        DrawMenuText(xPos2, yPos, MENU_TEXT_RGB_NORMAL, MenuBuffer);
        yPos += MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP;
    }

    // Players Car
    if ((GameSettings.GameType != GAMETYPE_DEMO) && (GameSettings.GameType != GAMETYPE_CLOCKWORK))
    {
        sprintf(MenuBuffer, "%s:", TEXT_TABLE(TEXT_CAR));
        DrawMenuText(xPos, yPos, MENU_TEXT_RGB_HILITE, MenuBuffer);
        DrawMenuText(xPos2, yPos, MENU_TEXT_RGB_NORMAL, CarInfo[g_TitleScreenData.iCarNum[0]].Name);
        yPos += MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP;
    }

    // Whether have completed
    if (GameSettings.GameType == GAMETYPE_SINGLE && g_TitleScreenData.iLevelNum < LEVEL_NCUP_LEVELS)
    {
        DrawMenuText(xPos, yPos, MENU_TEXT_RGB_HILITE, TEXT_TABLE(TEXT_LOADSCREEN_WONRACE));
        DrawMenuText(xPos2, yPos, MENU_TEXT_RGB_NORMAL, IsSecretWonSingleRace(g_TitleScreenData.iLevelNum) ? TEXT_TABLE(TEXT_YES) : TEXT_TABLE(TEXT_NO));
        yPos += MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP;
    }

    if (GameSettings.GameType == GAMETYPE_PRACTICE)
    {
        DrawMenuText(xPos, yPos, MENU_TEXT_RGB_HILITE, TEXT_TABLE(TEXT_LOADSCREEN_FOUNDSTAR));
        DrawMenuText(xPos2, yPos, MENU_TEXT_RGB_NORMAL, IsSecretFoundPractiseStars(g_TitleScreenData.iLevelNum) ? TEXT_TABLE(TEXT_YES) : TEXT_TABLE(TEXT_NO));
        yPos += MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP;
    }

    if (GameSettings.GameType == GAMETYPE_TRAINING)
    {
        sprintf(MenuBuffer, "%s:", TEXT_TABLE(TEXT_SECRETS_STARS));
        DrawMenuText(xPos, yPos, MENU_TEXT_RGB_HILITE, MenuBuffer);
        sprintf(MenuBuffer, "%ld %s %ld", StarList.NumFound, TEXT_TABLE(TEXT_OF), StarList.NumTotal);
        DrawMenuText(xPos2, yPos, MENU_TEXT_RGB_NORMAL, MenuBuffer);
        yPos += MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP;
    }
}

