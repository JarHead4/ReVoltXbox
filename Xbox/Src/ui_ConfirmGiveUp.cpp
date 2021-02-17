//-----------------------------------------------------------------------------
// File: ui_ConfirmGiveUp.cpp
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
#include "Text.h"           // BeginTextState

// re-volt specific
#include "ui_menutext.h"   // re-volt strings

// temporary includes?
#include "player.h"         // for ui_TitleScreen.h to work
#include "ui_TitleScreen.h"

bool gConfirmGiveup = FALSE;

void CreateConfirmGiveupMenu(MENU_HEADER *menuHeader, MENU *menu);
BOOL SetGiveup(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
BOOL UnsetGiveup(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
BOOL GiveupGoForward(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);

static void DrawConfirmGiveup(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex);




////////////////////////////////////////////////////////////////
//
// Confirm give up try
//
////////////////////////////////////////////////////////////////
extern MENU Menu_ConfirmGiveup = 
{
    TEXT_GIVEUP_CHAMP,
    MENU_DEFAULT | MENU_CENTRE_X | MENU_CENTRE_Y,   // Menu type
    CreateConfirmGiveupMenu,                // Create menu function
    NULL,                                   // Input handler function
    NULL,                                   // Menu draw function
    0,                                      // X coord
    0,                                      // Y Coord
};

// Menu item
MENU_ITEM MenuItem_ConfirmGiveup = {
    TEXT_NONE,                              // Text label index

    MENU_TEXT_WIDTH * 22,                   // Space needed to draw item data
    NULL,                                   // Data

    DrawConfirmGiveup,                      // Draw Function

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
    menuHeader->m_pMenu->dwFlags |= MENU_NOBOX;
    gConfirmGiveup = FALSE;
    menuHeader->AddMenuItem( &MenuItem_ConfirmGiveup);
}

// utility
BOOL SetGiveup(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    gConfirmGiveup = TRUE;
    return TRUE;
}

BOOL UnsetGiveup(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    gConfirmGiveup = FALSE;
    return TRUE;
}

BOOL GiveupGoForward(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    if (gConfirmGiveup) 
    {
        Players[0].RaceFinishPos = CRAZY_FINISH_POS;
        ChampionshipEndMode = CHAMPIONSHIP_END_GAVEUP;
        MenuGoForward(menuHeader, menu, menuItem);
    } 
    else 
    {
        MenuGoBack(menuHeader, menu, menuItem);
    }
    return TRUE;
}


////////////////////////////////////////////////////////////////
//
// Draw Confirm Giveup 
//
////////////////////////////////////////////////////////////////
void DrawConfirmGiveup( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem, int itemIndex)
{
    FLOAT xPos, yPos, xSize;

    if( CupTable.TriesLeft == 1 )
        swprintf( MenuBuffer, TEXT_TABLE(TEXT_CONFIRM_GIVEUP_YOUHAVE_1_TRYLEFT) );
    else
        swprintf( MenuBuffer, TEXT_TABLE(TEXT_CONFIRM_GIVEUP_YOUHAVE_N_TRIESLEFT), CupTable.TriesLeft );

    xPos = pMenuHeader->m_XPos + pMenuHeader->m_ItemTextWidth + MENU_TEXT_GAP;
    yPos = pMenuHeader->m_YPos + (MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP) * itemIndex;
    xSize = wcslen(MenuBuffer) * (FLOAT)(MENU_TEXT_WIDTH);

    DrawSpruBox(
        gMenuWidthScale * (xPos - MENU_FRAME_WIDTH),
        gMenuHeightScale * (yPos - MENU_FRAME_HEIGHT),
        gMenuWidthScale * (xSize + 2*MENU_FRAME_WIDTH),
        gMenuHeightScale * (MENU_TEXT_HEIGHT*2 + 2*MENU_FRAME_HEIGHT + 2*MENU_TEXT_VSKIP),
        SPRU_COL_RANDOM, 0);

    BeginTextState();

    DrawMenuText(xPos, yPos, MENU_TEXT_RGB_NORMAL, MenuBuffer);
    yPos += MENU_TEXT_HEIGHT + 2*MENU_TEXT_VSKIP;

    DrawMenuText(xPos, yPos, MENU_TEXT_RGB_HILITE, TEXT_TABLE(TEXT_ARE_YOU_SURE));
    xPos += MENU_TEXT_WIDTH * wcslen(TEXT_TABLE(TEXT_ARE_YOU_SURE)) + 4*MENU_TEXT_HSKIP;

    if( gConfirmGiveup ) 
    {
        DrawMenuText(xPos, yPos, MENU_TEXT_RGB_CHOICE, TEXT_TABLE(TEXT_YES));
        xPos += MENU_TEXT_WIDTH * wcslen(TEXT_TABLE(TEXT_YES)) + 2*MENU_TEXT_HSKIP;
        DrawMenuText(xPos, yPos, MENU_TEXT_RGB_NOTCHOICE, TEXT_TABLE(TEXT_NO));
    } 
    else 
    {
        DrawMenuText(xPos, yPos, MENU_TEXT_RGB_NOTCHOICE, TEXT_TABLE(TEXT_YES));
        xPos += MENU_TEXT_WIDTH * wcslen(TEXT_TABLE(TEXT_YES)) + 2*MENU_TEXT_HSKIP;
        DrawMenuText(xPos, yPos, MENU_TEXT_RGB_CHOICE, TEXT_TABLE(TEXT_NO));
    }
}

