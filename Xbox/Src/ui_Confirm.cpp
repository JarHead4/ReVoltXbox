//-----------------------------------------------------------------------------
// File: ui_Confirm.cpp
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
#include "Text.h"           // DrawMenuText

#include "ui_Confirm.h"

// re-volt specific
#include "ui_menutext.h"   // re-volt strings

// temporary includes?
#include "player.h"         // for ui_TitleScreen.h to work
#include "ui_TitleScreen.h"

#define MENU_NUMPLAYERS_XPOS            150
#define MENU_NUMPLAYERS_YPOS            100


void CreateConfirmMenu(MENU_HEADER *menuHeader, MENU *menu);
BOOL DecreaseConfirmValue(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
BOOL IncreaseConfirmValue(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
BOOL MenuConfirmYesNoGoBack(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);

static void DrawConfirmYesNo(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex);

long gConfirmType = -1;         // JCC: Fudged-in value set to say which menu item is waiting for confirm
                                        // 0 == Quit game; 1 == Restart game
                                        // Apologies to anyone who discovers this total fudge!

////////////////////////////////////////////////////////////////
//
// Confirm Yes/No menu
//
////////////////////////////////////////////////////////////////

WCHAR gConfirmMenuText[64];
WCHAR gConfirmMenuTextYes[32];
WCHAR gConfirmMenuTextNo[32];
long  gConfirmMenuType = CONFIRM_TYPE_YESNO;
long  gConfirmMenuReturnVal = 0;
static long gConfirmMenuReturnValDefault = CONFIRM_NO;

// Menu
MENU Menu_ConfirmYesNo = 
{
    TEXT_NONE,
    MENU_DEFAULT | MENU_CENTRE_X | MENU_CENTRE_Y,   // Menu type
    CreateConfirmMenu,                      // Create menu function
    NULL,                                   // Input handler function
    NULL,                                   // Menu draw function
    MENU_NUMPLAYERS_XPOS,                   // X coord
    MENU_NUMPLAYERS_YPOS,                   // Y Coord
};

// Confirm
MENU_ITEM MenuItem_ConfirmYesNo = {
    TEXT_NONE,                              // Text label index

    0,                                      // Space needed to draw item data
    NULL,                                   // Data

    DrawConfirmYesNo,                       // Draw Function

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    DecreaseConfirmValue,                   // Left Action
    IncreaseConfirmValue,                   // Right Action
    MenuConfirmYesNoGoBack,                 // Back Action
    MenuGoBack,                             // Forward Action
};

// Create Function
void CreateConfirmMenu(MENU_HEADER *menuHeader, MENU *menu)
{
    int extra = (wcslen(gConfirmMenuTextYes) || wcslen(gConfirmMenuTextNo)) ? 10 : 0;
    MenuItem_ConfirmYesNo.DataWidth = (wcslen(gConfirmMenuText) + wcslen(gConfirmMenuTextYes) + wcslen(gConfirmMenuTextNo) + extra) * (FLOAT)(MENU_TEXT_WIDTH);    
    gConfirmMenuReturnVal = gConfirmMenuReturnValDefault;
    menuHeader->AddMenuItem( &MenuItem_ConfirmYesNo);

    if( menu->ParentMenu ) 
        menu->TextIndex = menu->ParentMenu->TextIndex;
    else 
        menu->TextIndex = TEXT_NONE;
}

// Utility
BOOL DecreaseConfirmValue(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    if( gConfirmMenuReturnVal == CONFIRM_NO ) 
    {
        gConfirmMenuReturnVal = CONFIRM_YES;
        return TRUE;
    }

    return FALSE;
}

BOOL IncreaseConfirmValue(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    if( gConfirmMenuReturnVal == CONFIRM_YES ) 
    {
        gConfirmMenuReturnVal = CONFIRM_NO;
        return TRUE;
    }

    return FALSE;
}

BOOL MenuConfirmYesNoGoBack(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    gConfirmMenuReturnVal = CONFIRM_GOBACK;
    MenuGoBack(menuHeader, menu, menuItem);
    return TRUE;
}

void SetConfirmMenuStrings(WCHAR *title, WCHAR *yes, WCHAR *no, long def)
{
    wcsncpy( gConfirmMenuText,    title, 64 );
    wcsncpy( gConfirmMenuTextYes, yes,   32 );
    wcsncpy( gConfirmMenuTextNo,  no,    32 );
    gConfirmMenuReturnValDefault = def;
}

////////////////////////////////////////////////////////////////
//
// DrawConfirmYesNo
//
////////////////////////////////////////////////////////////////
void DrawConfirmYesNo( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem, int itemIndex)
{
    FLOAT xPos, yPos, xSize, ySize;

    xPos = pMenuHeader->m_XPos;
    yPos = pMenuHeader->m_YPos;

    xSize = (FLOAT)(wcslen(gConfirmMenuText)) * MENU_TEXT_WIDTH;
    ySize = (MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP) * 3;
    if ((gConfirmMenuTextYes[0] != '\0') || (gConfirmMenuTextNo[0] != '\0')) {
        //tmp = ((FLOAT)(wcslen(gConfirmMenuTextYes) + wcslen(gConfirmMenuTextNo) + 5)) * MENU_TEXT_WIDTH;
        //if (tmp > xSize) xSize = tmp;
        xSize += (wcslen(gConfirmMenuTextYes) + wcslen(gConfirmMenuTextNo) + 5) * (FLOAT)(MENU_TEXT_WIDTH);
    }

    // Draw the spru
    /*DrawSpruBox(
        gMenuWidthScale * (xPos - MENU_TEXT_GAP), 
        gMenuHeightScale * (yPos - MENU_TEXT_GAP),
        gMenuWidthScale * (xSize + MENU_TEXT_GAP * 2),
        gMenuHeightScale * (ySize + MENU_TEXT_GAP * 2), 
        rand() % SPRU_COL_RANDOM);

    BeginTextState();*/

    // Confirm message
    DrawMenuText(xPos, yPos, MENU_TEXT_RGB_HILITE, gConfirmMenuText, xSize);

    // Choices
    if (gConfirmMenuReturnVal == CONFIRM_YES) {
        xPos += (FLOAT)(wcslen(gConfirmMenuText) + 5) * MENU_TEXT_WIDTH;
        DrawMenuText(xPos, yPos, MENU_TEXT_RGB_CHOICE, gConfirmMenuTextYes, xSize);
        xPos += (FLOAT)(wcslen(gConfirmMenuTextYes) + 5) * MENU_TEXT_WIDTH;
        DrawMenuText(xPos, yPos, MENU_TEXT_RGB_NOTCHOICE, gConfirmMenuTextNo, xSize);
    } else {
        xPos += (FLOAT)(wcslen(gConfirmMenuText) + 5) * MENU_TEXT_WIDTH;
        DrawMenuText(xPos, yPos, MENU_TEXT_RGB_NOTCHOICE, gConfirmMenuTextYes, xSize);
        xPos += (FLOAT)(wcslen(gConfirmMenuTextYes) + 5) * MENU_TEXT_WIDTH;
        DrawMenuText(xPos, yPos, MENU_TEXT_RGB_CHOICE, gConfirmMenuTextNo, xSize);
    }
}


