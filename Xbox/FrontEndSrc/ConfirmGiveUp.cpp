//-----------------------------------------------------------------------------
// File: ConfirmGiveUp.cpp
//
// Desc: ConfirmGiveUp.cpp
//
// Hist: 02.08.02 - ported/created
//
// Copyright (c)  Microsoft Corporation.  All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"
#include "menu2.h"
#include "MenuDraw.h"   // DrawSpruBox
#include "main.h"       // TimeStep

// re-volt specific
#include "menutext.h"   // re-volt strings

// temporary includes?
#include "titlescreen.h"

void CreateConfirmGiveupMenu(MENU_HEADER *menuHeader, MENU *menu);
void SetGiveup(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
void UnsetGiveup(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
void GiveupGoForward(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);

void DrawConfirmGiveup(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex);

bool gConfirmGiveup = FALSE;

MENU Menu_ConfirmGiveup = {
    TEXT_GIVEUP_CHAMP,
    MENU_IMAGE_NONE,                        // Menu title bar image enum
    TITLESCREEN_CAMPOS_INIT,                         // Camera pos index
    MENU_DEFAULT | MENU_CENTRE_X | MENU_CENTRE_Y,   // Menu type
    SPRU_COL_RANDOM,                        // Spru colour
    CreateConfirmGiveupMenu,                // Create menu function
    0,                                      // X coord
    0,                                      // Y Coord
    0,                                      // Current Item index
    NULL,                                   // Parent Menu (OverWritten in Create)
};

// Menu item
MENU_ITEM MenuItem_ConfirmGiveup = {
    TEXT_NONE,                              // Text label index

    MENU_TEXT_WIDTH * 22,                   // Space needed to draw item data
    NULL,                                   // Data

    DrawConfirmGiveup,                      // Draw Function

	NULL,								    // Input handler
	0,                                      // Flags

    NULL,                                   // Up Action
    NULL,                                   // Down Action
    SetGiveup,                              // Left Action
    UnsetGiveup,                            // Right Action
    MenuGoBack,                             // Back Action
    GiveupGoForward,                        // Forward Action
};

// Create
void CreateConfirmGiveupMenu(MENU_HEADER *menuHeader, MENU *menu)
{
    g_bMenuDrawMenuBox = FALSE;
    gConfirmGiveup = FALSE;
    AddMenuItem(menuHeader, menu, &MenuItem_ConfirmGiveup);
}

// utility
void SetGiveup(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    gConfirmGiveup = TRUE;
}

void UnsetGiveup(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    gConfirmGiveup = FALSE;
}

void GiveupGoForward(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    if (gConfirmGiveup) {
        Players[0].RaceFinishPos = CRAZY_FINISH_POS;
        ChampionshipEndMode = CHAMPIONSHIP_END_GAVEUP;
        MenuGoForward(menuHeader, menu, menuItem);
    } else {
        MenuGoBack(menuHeader, menu, menuItem);
    }
}

////////////////////////////////////////////////////////////////
//
// Draw Confirm Giveup 
//
////////////////////////////////////////////////////////////////

void DrawConfirmGiveup(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex)
{
    REAL xPos, yPos, xSize;

    sprintf(MenuBuffer, "%s %ld %s", TEXT_TABLE(TEXT_CONFIRM_GIVEUP_YOUHAVE), CupTable.TriesLeft, CupTable.TriesLeft == 1 ? TEXT_TABLE(TEXT_CONFIRM_GIVEUP_TRYLEFT) : TEXT_TABLE(TEXT_CONFIRM_GIVEUP_TRIESLEFT));

    xPos = menuHeader->XPos + menuHeader->ItemTextWidth + MENU_TEXT_GAP;
    yPos = menuHeader->YPos + (MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP) * itemIndex;
    xSize = strlen(MenuBuffer) * Real(MENU_TEXT_WIDTH);

    DrawSpruBox(
        gMenuWidthScale * (xPos - MENU_FRAME_WIDTH),
        gMenuHeightScale * (yPos - MENU_FRAME_HEIGHT),
        gMenuWidthScale * (xSize + 2*MENU_FRAME_WIDTH),
        gMenuHeightScale * (MENU_TEXT_HEIGHT*2 + 2*MENU_FRAME_HEIGHT + 2*MENU_TEXT_VSKIP),
        menuHeader->SpruColIndex, 0);

    DrawMenuText(xPos, yPos, MENU_TEXT_RGB_NORMAL, MenuBuffer);
    yPos += MENU_TEXT_HEIGHT + 2*MENU_TEXT_VSKIP;

    DrawMenuText(xPos, yPos, MENU_TEXT_RGB_HILITE, TEXT_TABLE(TEXT_ARE_YOU_SURE));
    xPos += MENU_TEXT_WIDTH * strlen(TEXT_TABLE(TEXT_ARE_YOU_SURE)) + 4*MENU_TEXT_HSKIP;

    if (gConfirmGiveup) {
        DrawMenuText(xPos, yPos, MENU_TEXT_RGB_CHOICE, TEXT_TABLE(TEXT_YES));
        xPos += MENU_TEXT_WIDTH * strlen(TEXT_TABLE(TEXT_YES)) + 2*MENU_TEXT_HSKIP;
        DrawMenuText(xPos, yPos, MENU_TEXT_RGB_NOTCHOICE, TEXT_TABLE(TEXT_NO));
    } else {
        DrawMenuText(xPos, yPos, MENU_TEXT_RGB_NOTCHOICE, TEXT_TABLE(TEXT_YES));
        xPos += MENU_TEXT_WIDTH * strlen(TEXT_TABLE(TEXT_YES)) + 2*MENU_TEXT_HSKIP;
        DrawMenuText(xPos, yPos, MENU_TEXT_RGB_CHOICE, TEXT_TABLE(TEXT_NO));
    }

}

