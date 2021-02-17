//-----------------------------------------------------------------------------
// File: SelectCup.cpp
//
// Desc: SelectCup.cpp
//
// Hist: 02.08.02 - ported/created
//
// Copyright (c)  Microsoft Corporation.  All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"
#include "menu2.h"
#include "MenuDraw.h"   // DrawSpruBox
#include "timing.h"     // TIME2MS
#include "LevelLoad.h"  // GAMETYPE
#include "cheats.h"       // AllCars

// re-volt specific
#include "menutext.h"   // re-volt strings

// menus
#include "RaceSummary.h"

// temporary includes?
#include "titlescreen.h"  // TITLESCREEN_CAMPOS_ & g_bTitleScreenRunDemo

#define MENU_CUPSELECT_XPOS             Real(100)
#define MENU_CUPSELECT_YPOS             Real(150)

static void CreateCupSelectMenu(MENU_HEADER *menuHeader, MENU *menu);

static void CupSelectBronzeCup(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
static void CupSelectSilverCup(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
static void CupSelectGoldCup(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
static void CupSelectPlatinumCup(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);

static void SelectNextCup( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem );
static void SelectPreviousCup( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem );

static void DrawCupLocked(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex);

//Menu
MENU Menu_CupSelect = {
    TEXT_CHOOSECUP,
    MENU_IMAGE_CUPSELECT,                   // Menu title bar image enum
    TITLESCREEN_CAMPOS_ALL_TROPHIES,                 // Camera pos index
    MENU_DEFAULT,                           // Menu type
    SPRU_COL_RANDOM,                        // Spru colour
    CreateCupSelectMenu,                    // Create menu function
    MENU_CUPSELECT_XPOS,                    // X coord
    MENU_CUPSELECT_YPOS,                    // Y Coord
    0,                                      // Current Item index
    NULL,                                   // Parent Menu (OverWritten in Create)
};

// Bronze Cup
MENU_ITEM MenuItem_BronzeCup = {
    TEXT_BRONZECUP,                         // Text label index

    0,                                      // Space needed to draw item data
    &Menu_Overview,                         // Data
    NULL,                                   // Draw Function

	NULL,								    // Input handler
	0,                                      // Flags

    SelectPreviousCup,                      // Up Action
    SelectNextCup,                          // Down Action
    SelectPreviousCup,                      // Left Action
    SelectNextCup,                          // Right Action
    MenuGoBack,                             // Back Action
    CupSelectBronzeCup,                     // Forward Action
};

// Silver Cup
MENU_ITEM MenuItem_SilverCup = {
    TEXT_SILVERCUP,                         // Text label index

    0,                                      // Space needed to draw item data
    &Menu_Overview,                         // Data
    NULL,                                   // Draw Function

 	NULL,								    // Input handler
	0,                                      // Flags

    SelectPreviousCup,                      // Up Action
    SelectNextCup,                          // Down Action
    SelectPreviousCup,                      // Left Action
    SelectNextCup,                          // Right Action
    MenuGoBack,                             // Back Action
    CupSelectSilverCup,                     // Forward Action
};

// Bronze Cup
MENU_ITEM MenuItem_GoldCup = {
    TEXT_GOLDCUP,                           // Text label index

    0,                                      // Space needed to draw item data
    &Menu_Overview,                         // Data
    NULL,                                   // Draw Function

	NULL,								    // Input handler
	0,                                      // Flags

    SelectPreviousCup,                      // Up Action
    SelectNextCup,                          // Down Action
    SelectPreviousCup,                      // Left Action
    SelectNextCup,                          // Right Action
    MenuGoBack,                             // Back Action
    CupSelectGoldCup,                       // Forward Action
};

// Bronze Cup
MENU_ITEM MenuItem_PlatinumCup = {
    TEXT_PLATINUMCUP,                       // Text label index

    0,                                      // Space needed to draw item data
    &Menu_Overview,                         // Data
    NULL,                                   // Draw Function

	NULL,								    // Input handler
	0,                                      // Flags

    SelectPreviousCup,                      // Up Action
    SelectNextCup,                          // Down Action
    SelectPreviousCup,                      // Left Action
    SelectNextCup,                          // Right Action
    MenuGoBack,                             // Back Action
    CupSelectPlatinumCup,                   // Forward Action
};

void PutCameraOnCurrentTrophy( DWORD index )
{
	if( index == 1 ) Menu_CupSelect.CamPosIndex = TITLESCREEN_CAMPOS_BRONZE_TROPHY;
	if( index == 2 ) Menu_CupSelect.CamPosIndex = TITLESCREEN_CAMPOS_SILVER_TROPHY;
	if( index == 3 ) Menu_CupSelect.CamPosIndex = TITLESCREEN_CAMPOS_GOLD_TROPHY;
	if( index == 4 ) Menu_CupSelect.CamPosIndex = TITLESCREEN_CAMPOS_PLATINUM_TROPHY;
}

// Create
void CreateCupSelectMenu(MENU_HEADER *menuHeader, MENU *menu)
{
    // add menu items
    AddMenuItem(menuHeader, menu, &MenuItem_BronzeCup);
    AddMenuItem(menuHeader, menu, &MenuItem_SilverCup);
    AddMenuItem(menuHeader, menu, &MenuItem_GoldCup);
    AddMenuItem(menuHeader, menu, &MenuItem_PlatinumCup);

	Menu_CupSelect.CamPosIndex = TITLESCREEN_CAMPOS_BRONZE_TROPHY;
    
	// disable 'ungot' cups
    if (Version == VERSION_RELEASE)
    {
        if (IsCupCompleted(RACE_CLASS_BRONZE))
        {
            MenuItem_SilverCup.DrawFunc = NULL;
        }
        else
        {
            MenuItem_SilverCup.ActiveFlags = MENU_ITEM_SELECTABLE;
            MenuItem_SilverCup.DrawFunc = DrawCupLocked;
        }
        if (IsCupCompleted(RACE_CLASS_SILVER))
        {
            MenuItem_GoldCup.DrawFunc = NULL;
        }
        else
        {
            MenuItem_GoldCup.ActiveFlags = MENU_ITEM_SELECTABLE;
            MenuItem_GoldCup.DrawFunc = DrawCupLocked;
        }
        if (IsCupCompleted(RACE_CLASS_GOLD))
        {
            MenuItem_PlatinumCup.DrawFunc = NULL;
        }
        else
        {
            MenuItem_PlatinumCup.ActiveFlags = MENU_ITEM_SELECTABLE;
            MenuItem_PlatinumCup.DrawFunc = DrawCupLocked;
        }
    }
}

void SelectNextCup( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem )
{
    SelectNextMenuItem( pMenuHeader, pMenu, pMenuItem );

	PutCameraOnCurrentTrophy( pMenu->CurrentItemIndex );
}

void SelectPreviousCup( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem )
{
    SelectPreviousMenuItem( pMenuHeader, pMenu, pMenuItem );

	PutCameraOnCurrentTrophy( pMenu->CurrentItemIndex );
}

// Utility
void CupSelectBronzeCup(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    g_TitleScreenData.CupType = (long)RACE_CLASS_BRONZE;
    MenuGoForward(menuHeader, menu, menuItem);  
}

void CupSelectSilverCup(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    g_TitleScreenData.CupType = (long)RACE_CLASS_SILVER;
    MenuGoForward(menuHeader, menu, menuItem);  
}

void CupSelectGoldCup(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    g_TitleScreenData.CupType = (long)RACE_CLASS_GOLD;
    MenuGoForward(menuHeader, menu, menuItem);  
}

void CupSelectPlatinumCup(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    g_TitleScreenData.CupType = (long)RACE_CLASS_SPECIAL;
    MenuGoForward(menuHeader, menu, menuItem);  
}


/////////////////////////////////////////////////////////////////////
//
// Draw Track Name
//
/////////////////////////////////////////////////////////////////////


void DrawCupLocked(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex)
{
#ifdef _PC

    REAL xPos, yPos;

// only if I'm selected

    if (itemIndex != menu->CurrentItemIndex)
        return;

// flash locked message

    if (!(TIME2MS(TimerCurrent) & 128))
    {
        xPos = menuHeader->XPos;
        yPos = menuHeader->YPos;

        xPos = (640 - 6 * Real(MENU_TEXT_WIDTH)) / 2;
        yPos = (480 - Real(MENU_TEXT_HEIGHT)) / 2;
        DrawSpruBox(
            gMenuWidthScale * (xPos - MENU_TEXT_HSKIP), 
            gMenuHeightScale * (yPos - MENU_TEXT_VSKIP),
            gMenuWidthScale * (6 * MENU_TEXT_WIDTH + MENU_TEXT_HSKIP * 2),
            gMenuHeightScale * (MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP * 2),
            menuHeader->SpruColIndex, 0);

        DrawMenuText(xPos, yPos, MENU_TEXT_RGB_HILITE, "Locked");
    }
#endif
}

