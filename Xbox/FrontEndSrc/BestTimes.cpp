//-----------------------------------------------------------------------------
// File: BestTimes.cpp
//
// Desc: BestTimes.cpp
//
// Hist: 02.08.02 - ported/created
//
// Copyright (c)  Microsoft Corporation.  All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"
#include "menu2.h"
#include "MenuDraw.h"   // DrawSpruBox
#include "main.h"       // TimeStep
#include "settings.h"   // RegistrySettings
#include "timing.h"     // LoadTrackTimes
#include "SelectTrack.h"

#include "GetPlayerName.h"

// re-volt specific
#include "menutext.h"   // re-volt strings

// temporary includes?
#include "titlescreen.h"

#define MENU_BESTTIMES_XPOS             Real(100)
#define MENU_BESTTIMES_YPOS             Real(150)

static void CreateBestTimesMenu(MENU_HEADER *menuHeader, MENU *menu);

static void DrawBestTimes(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex);

// Menu
MENU Menu_BestTimes = {
    TEXT_BESTTIMES,
    MENU_IMAGE_BESTTIMES,                   // Menu title bar image enum
    TITLESCREEN_CAMPOS_BESTTIMES,                    // Camera pos index
    MENU_DEFAULT | MENU_CENTRE_X,       // Menu type
    SPRU_COL_RANDOM,                        // Spru colour
    CreateBestTimesMenu,                    // Create menu function
    MENU_BESTTIMES_XPOS,                    // X coord
    MENU_BESTTIMES_YPOS,                    // Y Coord
    0,                                      // Current Item index
    NULL,                                   // Parent Menu (OverWritten in Create)
};

// Best Times - select level
MENU_ITEM MenuItem_BestTimesLevel = {
    TEXT_SELECTTRACK,                       // Text label index
    MENU_DATA_WIDTH_TEXT,                   // Space needed to draw item data
    &Menu_PlayerName,                       // Data (Menu to set up game and then run it)
    DrawBestTimes,                          // Draw Function

	NULL,								    // Input handler
	0,                                      // Flags

    SelectTrackMirrorTrack,                 // Up Action
    SelectTrackReverseTrack,                // Down Action
    SelectTrackPrevTrack,                   // Left Action
    SelectTrackNextTrack,                   // Right Action
    MenuGoBack,                             // Back Action
    NULL,                                   // Forward Action (depends on whether server or client)
};

// Best Times - select level In-Game
MENU_ITEM MenuItem_BestTimesLevelInGame = {
    TEXT_NONE,                              // Text label index

    340,                                    // Space needed to draw item data
    NULL,                                   // Data (Menu to set up game and then run it)
    DrawBestTimes,                          // Draw Function

	NULL,								    // Input handler
	0,                                      // Flags

    NULL,                                   // Up Action
    NULL,                                   // Down Action
    NULL,                                   // Left Action
    NULL,                                   // Right Action
    MenuGoBack,                             // Back Action
    NULL,                                   // Forward Action (depends on whether server or client)
};

// Create
void CreateBestTimesMenu(MENU_HEADER *menuHeader, MENU *menu)
{
    // Initialise
    g_TitleScreenData.iCurrentPlayer = 0;
    g_TitleScreenData.numberOfPlayers = 1;
    menuHeader->NavFlags &= ~MENU_FLAG_ADVANCE;

    if (GameSettings.Level == LEVEL_FRONTEND)
    {
        AddMenuItem(menuHeader, menu, &MenuItem_BestTimesLevel);
        GameSettings.GameType = GAMETYPE_TRIAL;
    }
    else
    {
        AddMenuItem(menuHeader, menu, &MenuItem_BestTimesLevelInGame);
        g_bMenuDrawMenuBox = FALSE;
        SaveTrackTimes();
    }

    // Make sure level is valid
    SelectTrackPrevTrack(menuHeader, menu, &MenuItem_BestTimesLevel);
    SelectTrackNextTrack(menuHeader, menu, &MenuItem_BestTimesLevel);

    LoadTrackTimes(g_TitleScreenData.iLevelNum, g_TitleScreenData.mirror, g_TitleScreenData.reverse);
}

/////////////////////////////////////////////////////////////////////
//
// Draw Best Times
//
/////////////////////////////////////////////////////////////////////

void DrawBestTimes(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex)
{
#ifdef _PC
    int j;
    REAL xPos, yPos;
    LEVELINFO *levelInfo;
    long col;
    bool avail;

    // Draw the current level for which the times are shown
    levelInfo = GetLevelInfo(g_TitleScreenData.iLevelNum);
    if (levelInfo == NULL) return;
    avail = IsLevelTypeAvailable(g_TitleScreenData.iLevelNum, g_TitleScreenData.mirror, g_TitleScreenData.reverse);

    // Choose text colour
    if (avail) {
        col = MENU_TEXT_RGB_CHOICE;
    } else {
        col = MENU_TEXT_RGB_LOLITE;
    }

    xPos = menuHeader->XPos + menuHeader->ItemTextWidth + MENU_TEXT_GAP;
    yPos = menuHeader->YPos + (MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP) * itemIndex;

    if (GameSettings.Level == LEVEL_FRONTEND)
    {
        sprintf(MenuBuffer, "%s%s%s", 
            levelInfo->Name, 
            (g_TitleScreenData.mirror)? " (M)": "",
            (g_TitleScreenData.reverse)? " (R)": "");

        DrawMenuText( xPos, yPos, col, MenuBuffer );
    }

    // Draw the table of best times
    xPos = menuHeader->XPos + 6;
    //yPos -= (MAX_RECORD_TIMES * (MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP) + 2 * MENU_TEXT_GAP);
    yPos += MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP + 2 * MENU_TEXT_GAP;

    DrawSpruBox(
        gMenuWidthScale * (xPos- MENU_TEXT_GAP), 
        gMenuHeightScale * (yPos - MENU_TEXT_GAP), 
        gMenuWidthScale * (42 * MENU_TEXT_WIDTH + 2 * MENU_TEXT_GAP), 
        gMenuHeightScale * (MAX_RECORD_TIMES * (MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP) + 2 * MENU_TEXT_GAP), 
        menuHeader->SpruColIndex, 0);

    if (avail) {
        col = MENU_TEXT_RGB_NORMAL;
    } else {
        col = MENU_TEXT_RGB_LOLITE;
    }

    for (j = 0 ; j < MAX_RECORD_TIMES ; j++)
    {
        if (TrackRecords.RecordLap[j].Time == MAX_LAP_TIME)
            sprintf(MenuBuffer, "--------- %16.16s %16.16s", TrackRecords.RecordLap[j].Player, TrackRecords.RecordLap[j].Car);
        else
            sprintf(MenuBuffer, "%02d:%02d:%03d %16.16s %16.16s", MINUTES(TrackRecords.RecordLap[j].Time), SECONDS(TrackRecords.RecordLap[j].Time), THOUSANDTHS(TrackRecords.RecordLap[j].Time), TrackRecords.RecordLap[j].Player, TrackRecords.RecordLap[j].Car);

        DrawMenuText(xPos, yPos, col, MenuBuffer);
        yPos += MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP;
    }
#endif
}

