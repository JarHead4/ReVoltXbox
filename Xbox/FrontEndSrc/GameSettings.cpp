//-----------------------------------------------------------------------------
// File: GameSettings.cpp
//
// Desc: GameSettings.cpp
//
// Hist: 02.08.02 - ported/created
//
// Copyright (c)  Microsoft Corporation.  All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"
#include "menu2.h"
#include "MenuDraw.h"   // DrawSpruBox
#include "main.h"       // TimeStep
#include "panel.h"      // SpeedUnits
#include "readinit.h"   // WriteAllCarInfo
#include "LevelLoad.h"  // GAMETYPE

// re-volt specific
#include "menutext.h"   // re-volt strings

// menus
#include "Confirm.h"
#include "GetPlayerName.h"

// temporary includes?
#include "titlescreen.h"


#define MENU_SETTINGS_XPOS              Real(100)
#define MENU_SETTINGS_YPOS              Real(150)

static void CreateGameSettingsMenu(MENU_HEADER *menuHeader, MENU *menu);
static void GameSettingsGoBack(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);

static void MenuSaveCarInfoSingle(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
static void MenuSaveCarInfoMultiple(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
static void MenuSaveLocalPlayerCarInfo(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);

static void CreateGameDifficultyMenu(MENU_HEADER *menuHeader, MENU *menu);
static BOOL GameDifficultyHandler( DWORD input, MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem );

static void DrawGhostType(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex);
static void DrawPlayDifficulty(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex);
static void DrawSpeedUnits(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex);


// Menu
MENU Menu_GameSettings = {
    TEXT_GAMESETTINGS,
    MENU_IMAGE_OPTIONS,                     // Menu title bar image enum
    TITLESCREEN_CAMPOS_OVERVIEW,                     // Camera pos index
    MENU_DEFAULT | MENU_CENTRE_X,   // Menu type
    SPRU_COL_RANDOM,                        // Spru colour
    CreateGameSettingsMenu,                 // Create menu function
    MENU_SETTINGS_XPOS,                     // X coord
    MENU_SETTINGS_YPOS,                     // Y Coord
    0,                                      // Current Item index
    NULL,                                   // Parent Menu (OverWritten in Create)
};

// Data
SLIDER_DATA_LONG NumberOfCarsSlider = {
    &g_TitleScreenData.numberOfCars,
    //MIN_RACE_CARS, 
    2, MAX_RACE_CARS, 1,
    FALSE, TRUE,
};
SLIDER_DATA_LONG NumberOfLapsSlider = {
    &g_TitleScreenData.numberOfLaps,
    MIN_RACE_LAPS, MAX_RACE_LAPS, 1,
    FALSE, TRUE,
};
SLIDER_DATA_LONG PlayModeSlider = {
    &g_TitleScreenData.playMode,
    0, PLAYMODE_KIDS, 1,
    FALSE, TRUE,
};
SLIDER_DATA_LONG SpeedUnitsSlider = {
    &SpeedUnits,
    0, SPEED_NTYPES - 1, 1,
    FALSE, FALSE,
};

// Game Settings - # cars
MENU_ITEM MenuItem_NumberOfCars = {
    TEXT_NUMBEROFCARS,                      // Text label index

    MENU_DATA_WIDTH_INT,                    // Space needed to draw item data
    &NumberOfCarsSlider,                    // Data
    DrawSliderDataLong,                     // Draw Function

	NULL,								    // Input handler
	0,                                      // Flags

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    DecreaseSliderDataLong,                 // Left Action
    IncreaseSliderDataLong,                 // Right Action
    MenuGoBack,                             // Back Action
    NULL,                                   // Forward Action
};

// Game Settings - # laps
MENU_ITEM MenuItem_NumberOfLaps = {
    TEXT_NUMBEROFLAPS,                      // Text label index

    MENU_DATA_WIDTH_INT,                    // Space needed to draw item data
    &NumberOfLapsSlider,                    // Data
    DrawSliderDataLong,                     // Draw Function

	NULL,								    // Input handler
	0,                                      // Flags

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    DecreaseSliderDataLong,                 // Left Action
    IncreaseSliderDataLong,                 // Right Action
    GameSettingsGoBack,                     // Back Action
    NULL,                                   // Forward Action
};

// Game Settings - Random cars
MENU_ITEM MenuItem_RandomCars = {
    TEXT_RANDOM_CARS,                       // Text label index

    0,                                      // Space needed to draw item data
    &g_TitleScreenData.RandomCars,           // Data
    DrawMenuDataOnOff,                      // Draw Function

	NULL,								    // Input handler
	0,                                      // Flags

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    ToggleMenuDataOff,                      // Left Action
    ToggleMenuDataOn,                       // Right Action
    GameSettingsGoBack,                     // Back Action
    NULL,                                   // Forward Action
};

// Game Settings - Random track
MENU_ITEM MenuItem_RandomTrack = {
    TEXT_RANDOM_TRACK,                      // Text label index

    0,                                      // Space needed to draw item data
    &g_TitleScreenData.RandomTrack,          // Data
    DrawMenuDataOnOff,                      // Draw Function

	NULL,								    // Input handler
	0,                                      // Flags

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    ToggleMenuDataOff,                      // Left Action
    ToggleMenuDataOn,                       // Right Action
    GameSettingsGoBack,                     // Back Action
    NULL,                                   // Forward Action
};

// Game Settings - pickups
MENU_ITEM MenuItem_Pickups = {
    TEXT_PICKUPS,                           // Text label index

    MENU_DATA_WIDTH_BOOL,                   // Space needed to draw item data
    &g_TitleScreenData.pickUps,              // Data
    DrawMenuDataOnOff,                      // Draw Function

	NULL,								    // Input handler
	0,                                      // Flags

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    ToggleMenuDataOff,                      // Left Action
    ToggleMenuDataOn,                       // Right Action
    GameSettingsGoBack,                     // Back Action
    NULL,                                   // Forward Action
};

// Game Settings - Speed units
MENU_ITEM MenuItem_SpeedUnits = {
    TEXT_UNITS,                             // Text label index

    Real(12) * MENU_TEXT_WIDTH,                 // Space needed to draw item data
    &SpeedUnitsSlider,                      // Data
    DrawSpeedUnits,                         // Draw Function

	NULL,								    // Input handler
	0,                                      // Flags

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    DecreaseSliderDataLong,                 // Left Action
    IncreaseSliderDataLong,                 // Right Action
    GameSettingsGoBack,                     // Back Action
    NULL,                                   // Forward Action
};

// Game Settings - Local Ghost?
MENU_ITEM MenuItem_GhostType = {
    TEXT_GHOSTTYPE,                         // Text label index

    MENU_TEXT_WIDTH * 12,                   // Space needed to draw item data
    &g_TitleScreenData.LocalGhost,           // Data
    DrawGhostType,                          // Draw Function

	NULL,								    // Input handler
	0,                                      // Flags

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    ToggleMenuDataOff,                      // Left Action
    ToggleMenuDataOn,                       // Right Action
    GameSettingsGoBack,                     // Back Action
    NULL,                                   // Forward Action
};

// Game Settings - Clear Registry Secrets

MENU_ITEM MenuItem_ClearSecrets = {
    TEXT_CLEARSECRETS,                      // Text label index

    0,                                      // Space needed to draw item data
    &Menu_ConfirmYesNo,                     // Data
    NULL,                                   // Draw Function

	NULL,								    // Input handler
	0,                                      // Flags

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    NULL,                                   // Left Action
    NULL,                                   // Right Action
    GameSettingsGoBack,                     // Back Action
    MenuGoForward,                          // Forward Action
};

// Game Settings - Save CarInfo (Dev only)
MENU_ITEM MenuItem_SaveCarInfoSingle = {
    TEXT_SAVECARINFOSINGLE,                 // Text label index

    0,                                      // Space needed to draw item data
    NULL,                                   // Data
    NULL,                                   // Draw Function

	NULL,								    // Input handler
	0,                                      // Flags

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    NULL,                                   // Left Action
    NULL,                                   // Right Action
    GameSettingsGoBack,                     // Back Action
    //MenuSaveCarInfoMultiple,              // Forward Action
    MenuSaveCarInfoSingle,              // Forward Action
};

MENU_ITEM MenuItem_SaveCarInfoMultiple = {
    TEXT_SAVECARINFOMULTIPLE,               // Text label index

    0,                                      // Space needed to draw item data
    NULL,                                   // Data
    NULL,                                   // Draw Function

	NULL,								    // Input handler
	0,                                      // Flags

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    NULL,                                   // Left Action
    NULL,                                   // Right Action
    GameSettingsGoBack,                     // Back Action
    MenuSaveCarInfoMultiple,                // Forward Action
};

MENU_ITEM MenuItem_SaveLocalPlayerCarInfo = {
    TEXT_SAVECURRENTCARINFO,                // Text label index

    0,                                      // Space needed to draw item data
    NULL,                                   // Data
    NULL,                                   // Draw Function

	NULL,								    // Input handler
	0,                                      // Flags

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    NULL,                                   // Left Action
    NULL,                                   // Right Action
    GameSettingsGoBack,                     // Back Action
    MenuSaveLocalPlayerCarInfo,             // Forward Action
};

// Create Game settings menu
void CreateGameSettingsMenu(MENU_HEADER *menuHeader, MENU *menu)
{

    // add menu items
    AddMenuItem(menuHeader, menu, &MenuItem_NumberOfCars);
    AddMenuItem(menuHeader, menu, &MenuItem_NumberOfLaps);
    AddMenuItem(menuHeader, menu, &MenuItem_RandomCars);
    AddMenuItem(menuHeader, menu, &MenuItem_RandomTrack);
    AddMenuItem(menuHeader, menu, &MenuItem_Pickups);
    AddMenuItem(menuHeader, menu, &MenuItem_SpeedUnits);
    AddMenuItem(menuHeader, menu, &MenuItem_GhostType);

    // Switch off "Go-Forward" icon
    menuHeader->NavFlags &= ~MENU_FLAG_ADVANCE;

    //clear secrets?
    if (gConfirmMenuReturnVal == CONFIRM_YES)
    {
        StarList.NumFound = 0;
        ZeroMemory(&LevelSecrets, sizeof(LevelSecrets));
        InitDefaultLevels();
        SetAllCarSelect();
    }

    SetConfirmMenuStrings(TEXT_TABLE(TEXT_MENU_CONFIRM_GENERIC), TEXT_TABLE(TEXT_MENU_CONFIRM_REALLY) , TEXT_TABLE(TEXT_MENU_CONFIRM_PRETEND), CONFIRM_NO);
    AddMenuItem(menuHeader, menu, &MenuItem_ClearSecrets);

    // save carinfo
    if (Version == VERSION_DEV) {
        AddMenuItem(menuHeader, menu, &MenuItem_SaveCarInfoSingle);
//      AddMenuItem(menuHeader, menu, &MenuItem_SaveCarInfoMultiple);   // GAZZA
    }
}

// Utility
void GameSettingsGoBack(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    MenuGoBack(menuHeader, menu, menuItem);
}

void MenuSaveCarInfoSingle(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    WriteAllCarInfoSingle(CarInfo, NCarTypes, "AllCarInfo.txt");
}

void MenuSaveCarInfoMultiple(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    WriteAllCarInfoMultiple(CarInfo, NCarTypes);
}

void MenuSaveLocalPlayerCarInfo(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    int iChar, sLen;
    FILE *fp;
    VEC dR;
    char filename[MAX_CAR_FILENAME];
    char dirname[MAX_CAR_FILENAME];
    int iCar = PLR_LocalPlayer->car.CarType;

    // Undo CoM changes
    CopyVec(&CarInfo[iCar].CoMOffset, &dR);
    NegateVec(&dR);
    MoveCarCoM(&CarInfo[iCar], &dR);

    // get the filename for this car's carinfo (assumes car has a MODEL 0)
    sLen = strlen(CarInfo[iCar].ModelFile[0]);
    if (sLen <= 5) {
        DumpMessage("Warning", "Could not get car directory");
        return;                 // doesn't even have "cars\" at the start
    }

    iChar = 5;
    while ((iChar < sLen) && (iChar < MAX_CAR_FILENAME - 1) && (CarInfo[iCar].ModelFile[0][iChar] != '\\')) {
        dirname[iChar - 5] = CarInfo[iCar].ModelFile[0][iChar];
        iChar++;
    }
    dirname[iChar - 5] = '\0';
    #pragma message( "CPrince: will this break if we're storing D:\\ at start of these filenames?" )

//$MODIFIED
//    sprintf(filename, "Cars\\%s\\Parameters.txt", dirname);
    sprintf(filename, "D:\\Cars\\%s\\Parameters.txt", dirname);
//$END_MODIFICATIONS

    // open the file
    fp = fopen(filename, "w");
    if (fp == NULL) {
        DumpMessage("Could not open file", filename);
        MoveCarCoM(&CarInfo[iCar], &CarInfo[iCar].CoMOffset);
        return;
    }

    // Write the info file
    if (!WriteOneCarInfo(&CarInfo[iCar], fp)) {
        DumpMessage("Error saving", filename);
        MoveCarCoM(&CarInfo[iCar], &CarInfo[iCar].CoMOffset);
        fclose(fp);
        return;
    }
    
    // Redo CoM changes
    MoveCarCoM(&CarInfo[iCar], &CarInfo[iCar].CoMOffset);

    fclose(fp);
}



//-----------------------------------------------------------------------------
// Game Difficulty
//-----------------------------------------------------------------------------

// Game Difficulty Menu
MENU Menu_GameDifficulty = 
{
    TEXT_MODE,
    MENU_IMAGE_NONE,                        // Menu title bar image enum
    TITLESCREEN_CAMPOS_DONT_CHANGE,                  // Camera pos index
    MENU_DEFAULT | MENU_CENTRE_X,           // Menu type
    SPRU_COL_RANDOM,                        // Spru colour
    CreateGameDifficultyMenu,               // Create menu function
    MENU_SETTINGS_XPOS,                     // X coord
    MENU_SETTINGS_YPOS,                     // Y Coord
    0,                                      // Current Item index
    NULL,                                   // Parent Menu (OverWritten in Create)
};

MENU_ITEM MenuItem_Simulation = 
{
    TEXT_MODE_SIMULATION,                   // Text label index
    0,                                      // Space needed to draw item data
    &Menu_PlayerName,                       // Data
    DrawPlayDifficulty,                     // Draw Function
	GameDifficultyHandler,				    // Input handler
	0,                                      // Flags
};

MENU_ITEM MenuItem_Arcade = 
{
    TEXT_MODE_ARCADE,                       // Text label index
    0,                                      // Space needed to draw item data
    &Menu_PlayerName,                       // Data
    DrawPlayDifficulty,                     // Draw Function
	GameDifficultyHandler,				    // Input handler
	0,                                      // Flags
};

MENU_ITEM MenuItem_Console = 
{
    TEXT_MODE_CONSOLE,                      // Text label index
    0,                                      // Space needed to draw item data
    &Menu_PlayerName,                       // Data
    DrawPlayDifficulty,                     // Draw Function
	GameDifficultyHandler,				    // Input handler
	0,                                      // Flags
};

MENU_ITEM MenuItem_Kids = 
{
    TEXT_PLAYMODE_KIDS,                         // Text label index
    0,                                      // Space needed to draw item data
    &Menu_PlayerName,                       // Data
    DrawPlayDifficulty,                     // Draw Function
	GameDifficultyHandler,				    // Input handler
	0,                                      // Flags
};




VOID CreateGameDifficultyMenu( MENU_HEADER* pMenuHeader, MENU* pMenu )
{
    Menu_GameDifficulty.CurrentItemIndex = g_TitleScreenData.playMode;

    AddMenuItem( pMenuHeader, pMenu, &MenuItem_Simulation );
    AddMenuItem( pMenuHeader, pMenu, &MenuItem_Arcade );
    AddMenuItem( pMenuHeader, pMenu, &MenuItem_Console );
    
	if( GameSettings.GameType != GAMETYPE_BATTLE ) 
	{
        AddMenuItem( pMenuHeader, pMenu, &MenuItem_Kids );
    } 
	else 
	{
        if( g_TitleScreenData.playMode == PLAYMODE_KIDS ) 
		{
            Menu_GameDifficulty.CurrentItemIndex = PLAYMODE_CONSOLE;
        }
    }
}




BOOL GameDifficultyHandler( DWORD input, MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem )
{
	if( input == MENU_INPUT_SELECT )
	{
	    MenuGoForward( pMenuHeader, pMenu, pMenuItem );
		g_TitleScreenData.playMode = Menu_GameDifficulty.CurrentItemIndex;
		return TRUE;
	}

	return MenuDefHandler( input, pMenuHeader, pMenu, pMenuItem );
}

////////////////////////////////////////////////////////////////
//
// Draw Ghost Type
//
////////////////////////////////////////////////////////////////

void DrawGhostType(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex)
{
    REAL xPos, yPos;

#ifdef _PC
    xPos = menuHeader->XPos + menuHeader->ItemTextWidth + MENU_TEXT_GAP;
    yPos = menuHeader->YPos + (MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP) * itemIndex;

    sprintf(MenuBuffer, "%s", (g_TitleScreenData.LocalGhost)? TEXT_TABLE(TEXT_LOCAL) : TEXT_TABLE(TEXT_DOWNLOAD));

    DrawMenuText(xPos, yPos, MENU_TEXT_RGB_NORMAL, MenuBuffer );
#endif
}

////////////////////////////////////////////////////////////////
//
// Draw Difficulty
//
////////////////////////////////////////////////////////////////


/*void DrawPlayDifficulty(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex)
{
    REAL xPos, yPos, xSize;
#ifdef YOU_WANNIT

    g_TitleScreenData.playMode = menu->CurrentItemIndex;
    switch (g_TitleScreenData.playMode) {

    case PLAYMODE_SIMULATION:
        sprintf(MenuBuffer, "Full Speed; Realistic collision");
        break;

    case PLAYMODE_ARCADE:
        sprintf(MenuBuffer, "Full Speed; Simple car collision");
        break;

    case PLAYMODE_CONSOLE:
        sprintf(MenuBuffer, "Full Speed; Simple collision");
        break;

    case PLAYMODE_KIDS:
        sprintf(MenuBuffer, "Reduced Speed; Simple collision");
        break;
    }

    xSize = GetTextLen(MenuBuffer);
    xPos = (640.0f - xSize) / 2;
    yPos = menuHeader->YPos + 150;

    DrawSpruBox(
        gMenuWidthScale * (xPos - MENU_FRAME_WIDTH),
        gMenuHeightScale * (yPos - MENU_FRAME_HEIGHT),
        gMenuWidthScale * (xSize + 2 * MENU_FRAME_WIDTH),
        gMenuHeightScale * (MENU_TEXT_HEIGHT + 2 * MENU_FRAME_HEIGHT),
        menuHeader->SpruColIndex, 0);

    DrawMenuText(xPos, yPos, MENU_TEXT_RGB_CHOICE, MenuBuffer );

#endif

}*/

////////////////////////////////////////////////////////////////
//
// Draw Play Difficulty
//
////////////////////////////////////////////////////////////////

void DrawPlayDifficulty(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex)
{
    REAL xPos, yPos, xSize;

    g_TitleScreenData.playMode = menu->CurrentItemIndex;
    sprintf(MenuBuffer, TEXT_TABLE(TEXT_SIMULATION_HELPSTRING + g_TitleScreenData.playMode));

    xSize = strlen(MenuBuffer) * Real(MENU_TEXT_WIDTH);
    xPos = (640.0f - xSize) / 2;
    yPos = menuHeader->YPos + 150;

    DrawSpruBox(
        gMenuWidthScale * (xPos - MENU_FRAME_WIDTH),
        gMenuHeightScale * (yPos - MENU_FRAME_HEIGHT),
        gMenuWidthScale * (xSize + 2 * MENU_FRAME_WIDTH),
        gMenuHeightScale * (MENU_TEXT_HEIGHT + 2 * MENU_FRAME_HEIGHT),
        menuHeader->SpruColIndex, 0);

    DrawMenuText(xPos, yPos, MENU_TEXT_RGB_CHOICE, MenuBuffer );
}
////////////////////////////////////////////////////////////////
//
// Draw Speed Units
//
////////////////////////////////////////////////////////////////

void DrawSpeedUnits(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex)
{
    REAL xPos, yPos;

    xPos = menuHeader->XPos + menuHeader->ItemTextWidth + MENU_TEXT_GAP;
    yPos = menuHeader->YPos + (MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP) * itemIndex;

    //DrawMenuText(xPos,yPos, MENU_TEXT_RGB_NORMAL, SpeedUnitText[SpeedUnits]);
    DrawMenuText(xPos,yPos, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_MPH + SpeedUnits));
}

////////////////////////////////////////////////////////////////
//
// Draw Arcade mode
//
////////////////////////////////////////////////////////////////

void DrawPlayMode(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex)
{
    REAL xPos, yPos, xSize;

#ifdef _PC
    xPos = menuHeader->XPos + menuHeader->ItemTextWidth + MENU_TEXT_GAP;
    yPos = menuHeader->YPos + (MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP) * itemIndex;

    sprintf(MenuBuffer, "%s", TEXT_TABLE(TEXT_MODE_SIMULATION + g_TitleScreenData.playMode));

    DrawMenuText(xPos, yPos, MENU_TEXT_RGB_NORMAL, MenuBuffer );


    // Draw a description of the mode
    if (itemIndex == menu->CurrentItemIndex) {
        
        switch (g_TitleScreenData.playMode) {

        case PLAYMODE_SIMULATION:
            sprintf(MenuBuffer, "Full Speed; Realistic collision");
            break;

        case PLAYMODE_ARCADE:
            sprintf(MenuBuffer, "Full Speed; Simplified car-car collision");
            break;

        case PLAYMODE_CONSOLE:
            sprintf(MenuBuffer, "Full Speed; Simple collision");
            break;

        case PLAYMODE_KIDS:
            sprintf(MenuBuffer, "Reduced Speed; Simple collision");
            break;
        }

        xSize = (REAL)strlen(MenuBuffer) * MENU_TEXT_WIDTH;
        xPos = (640.0f - xSize) / 2;
        yPos = menuHeader->YPos + 150;

        DrawSpruBox(
            gMenuWidthScale * (xPos - MENU_FRAME_WIDTH),
            gMenuHeightScale * (yPos - MENU_FRAME_HEIGHT),
            gMenuWidthScale * (xSize + 2 * MENU_FRAME_WIDTH),
            gMenuHeightScale * (MENU_TEXT_HEIGHT + 2 * MENU_FRAME_HEIGHT),
            menuHeader->SpruColIndex, 0);

        DrawMenuText(xPos, yPos, MENU_TEXT_RGB_CHOICE, MenuBuffer );
    }
#endif
}


