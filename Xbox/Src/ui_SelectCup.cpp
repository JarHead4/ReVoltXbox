//-----------------------------------------------------------------------------
// File: ui_SelectCup.cpp
//
// Desc: 
//
// Re-Volt (Generic) Copyright (c) Probe Entertainment 1998
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"
#include "main.h"
#include "Text.h"       // BeginTextState
#include "geom.h"
#include "particle.h"
#include "model.h"
#include "aerial.h"
#include "newcoll.h"
#include "body.h"
#include "car.h"
#include "ctrlread.h"
#include "object.h"
#include "light.h"
#include "obj_init.h"
#include "player.h"
#include "ai.h"
#include "ai_init.h"
#include "EditObject.h"
#include "drawobj.h"
#include "move.h"
#include "timing.h"
#include "visibox.h"
#include "spark.h"
#include "field.h"
#include "weapon.h"
#include "input.h"
#include "ui_menudraw.h"
#include "initplay.h"
#include "pickup.h"
#include "SoundEffectEngine.h"

#include "ui_Menu.h"
#include "ui_MenuText.h"
#include "ui_TitleScreen.h"
#include "ui_StateEngine.h"
#include "ui_RaceOverview.h"
#include "ui_SelectCup.h"


#define MENU_CUPSELECT_XPOS             100
#define MENU_CUPSELECT_YPOS             150

static void CreateCupSelectMenu(MENU_HEADER *menuHeader, MENU *menu);
static BOOL SelectNextCup(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
static BOOL SelectPreviousCup(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
static BOOL CupSelectBronzeCup(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
static BOOL CupSelectSilverCup(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
static BOOL CupSelectGoldCup(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
static BOOL CupSelectPlatinumCup(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);

static void DrawCupLocked(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex);


////////////////////////////////////////////////////////////////
//
// Cup Select Menu
//
////////////////////////////////////////////////////////////////
extern MENU Menu_CupSelect = 
{
    TEXT_CHOOSECUP,
    MENU_DEFAULT,                           // Menu type
    CreateCupSelectMenu,                    // Create menu function
    NULL,                                   // Input handler function
    NULL,                                   // Menu draw function
    MENU_CUPSELECT_XPOS,                    // X coord
    MENU_CUPSELECT_YPOS,                    // Y Coord
};

// Bronze Cup
MENU_ITEM MenuItem_BronzeCup = {
    TEXT_BRONZECUP,                         // Text label index

    0,                                      // Space needed to draw item data
    &Menu_Overview,                         // Data
    NULL,                                   // Draw Function

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

    SelectPreviousCup,                      // Up Action
    SelectNextCup,                          // Down Action
    SelectPreviousCup,                      // Left Action
    SelectNextCup,                          // Right Action
    MenuGoBack,                             // Back Action
    CupSelectPlatinumCup,                   // Forward Action
};

// Create
void CreateCupSelectMenu(MENU_HEADER *menuHeader, MENU *menu)
{
    // add menu items
    menuHeader->AddMenuItem( &MenuItem_BronzeCup);
    menuHeader->AddMenuItem( &MenuItem_SilverCup);
    menuHeader->AddMenuItem( &MenuItem_GoldCup);
    menuHeader->AddMenuItem( &MenuItem_PlatinumCup);

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


BOOL SelectPreviousCup(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    if( TRUE == SelectPreviousMenuItem( menuHeader, menu, menuItem ) )
    {
        // Put the camera on the right trophy
        g_pTitleScreenCamera->SetNewPos( (short)(TITLESCREEN_CAMPOS_TROPHY1 + menu->CurrentItemIndex) );
        return TRUE;
    }
    return FALSE;
}

BOOL SelectNextCup(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    if( TRUE == SelectNextMenuItem( menuHeader, menu, menuItem ) )
    {
        // Put the camera on the right trophy
        g_pTitleScreenCamera->SetNewPos( (short)(TITLESCREEN_CAMPOS_TROPHY1 + menu->CurrentItemIndex) );
        return TRUE;
    }
    return FALSE;
}


// Utility
BOOL CupSelectBronzeCup(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    gTitleScreenVars.CupType = (long)RACE_CLASS_BRONZE;
    MenuGoForward(menuHeader, menu, menuItem);  
    return TRUE;
}

BOOL CupSelectSilverCup(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    gTitleScreenVars.CupType = (long)RACE_CLASS_SILVER;
    MenuGoForward(menuHeader, menu, menuItem);  
    return TRUE;
}

BOOL CupSelectGoldCup(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    gTitleScreenVars.CupType = (long)RACE_CLASS_GOLD;
    MenuGoForward(menuHeader, menu, menuItem);  
    return TRUE;
}

BOOL CupSelectPlatinumCup(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    gTitleScreenVars.CupType = (long)RACE_CLASS_SPECIAL;
    MenuGoForward(menuHeader, menu, menuItem);  
    return TRUE;
}


/////////////////////////////////////////////////////////////////////
//
// Draw Track Name
//
/////////////////////////////////////////////////////////////////////
void DrawCupLocked( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem, int itemIndex)
{
    FLOAT xPos, yPos;

// only if I'm selected

    if (itemIndex != pMenu->CurrentItemIndex)
        return;

// flash locked message

    if (!(TIME2MS(TimerCurrent) & 128))
    {
        xPos = pMenuHeader->m_XPos;
        yPos = pMenuHeader->m_YPos;

        xPos = (640 - 6 * (FLOAT)(MENU_TEXT_WIDTH)) / 2;
        yPos = (480 - (FLOAT)(MENU_TEXT_HEIGHT)) / 2;
        DrawSpruBox(
            gMenuWidthScale * (xPos - MENU_TEXT_HSKIP), 
            gMenuHeightScale * (yPos - MENU_TEXT_VSKIP),
            gMenuWidthScale * (6 * MENU_TEXT_WIDTH + MENU_TEXT_HSKIP * 2),
            gMenuHeightScale * (MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP * 2),
            SPRU_COL_RANDOM, 0);

        BeginTextState();
        DrawMenuText(xPos, yPos, MENU_TEXT_RGB_HILITE, TEXT_TABLE(TEXT_TRACK_LOCKED) );
    }
}


