//-----------------------------------------------------------------------------
// File: Language.cpp
//
// Desc: Language.cpp
//
// Hist: 02.08.02 - ported/created
//
// Copyright (c)  Microsoft Corporation.  All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"
#include "menu2.h"
#include "MenuDraw.h"   // DrawSpruBox
#include "LevelLoad.h"  // GAMETYPE, GameSettings
#include "cheats.h"       // CheckNameCheats

// re-volt specific
#include "menutext.h"   // re-volt strings

// menus
#include "MainMenu.h"

// temporary includes?
#include "titlescreen.h"

static void CreateLanguageMenu(MENU_HEADER *menuHeader, MENU *menu);
static void SelectPrevLang(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
static void SelectNextLang(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
static void SelectLang(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);

static void DrawLanguage(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex);

MENU Menu_Language = {
    TEXT_LANGUAGE,
    MENU_IMAGE_SELECTRACE,                   // Menu title bar image enum
    TITLESCREEN_CAMPOS_INIT,                         // Camera pos index
    MENU_DEFAULT | MENU_CENTRE_X | MENU_CENTRE_Y,   // Menu type
    SPRU_COL_RANDOM,                        // Spru colour
    CreateLanguageMenu,                     // Create menu function
    0,                                      // X coord
    0,                                      // Y Coord
    0,                                      // Current Item index
    NULL,                                   // Parent Menu (OverWritten in Create)
};


// Menu item
MENU_ITEM MenuItem_Language = {
    TEXT_NONE,                              // Text label index

    MENU_TEXT_WIDTH * 8,                    // Space needed to draw item data
    &Menu_TopLevel,                         // Data

    DrawLanguage,                           // Draw Function

	NULL,								    // Input handler
	0,                                      // Flags

    SelectPrevLang,                         // Up Action
    SelectNextLang,                         // Down Action
    SelectPrevLang,                         // Left Action
    SelectNextLang,                         // Right Action
    NULL,                                   // Back Action
    SelectLang,                             // Forward Action
};

// Create
void CreateLanguageMenu(MENU_HEADER *menuHeader, MENU *menu)
{
    g_bMenuDrawMenuBox = FALSE;
    AddMenuItem(menuHeader, menu, &MenuItem_Language);
}

// utility
void SelectPrevLang(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    if (g_TitleScreenData.Language > 0) g_TitleScreenData.Language--;
}

void SelectNextLang(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    if (g_TitleScreenData.Language < NLANGS - 1) g_TitleScreenData.Language++;
}

extern bool         g_bGameMantainMemPak;
void SelectLang(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    LANG_LoadStrings(g_TitleScreenData.Language);

    MenuGoForward(menuHeader, menu, menuItem);  
}

////////////////////////////////////////////////////////////////
//
// Draw Language
//
////////////////////////////////////////////////////////////////

void DrawLanguage(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex)
{
    long i;
    REAL x, y;

    x = menuHeader->XPos;
    y = menuHeader->YPos - (MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP) * 2;

    DrawSpruBox((x - Real(16)) * RenderSettings.GeomScaleX, (y - Real(16)) * RenderSettings.GeomScaleY, (MENU_TEXT_WIDTH * 8 + Real(32)) * RenderSettings.GeomScaleX, ((MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP) * NLANGS + Real(28)) * RenderSettings.GeomScaleY, menuHeader->SpruColIndex, 0);

    for (i = 0 ; i < NLANGS ; i++)
    {
        DrawMenuText(x, y + i * (MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP), i == (signed)g_TitleScreenData.Language ? 0xff00ffff : 0xffffffff, LangNames[i]);
    }
}

