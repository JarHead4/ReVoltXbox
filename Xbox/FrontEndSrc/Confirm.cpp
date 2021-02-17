//-----------------------------------------------------------------------------
// File: Confirm.cpp
//
// Desc: Confirm.cpp
//
// Hist: 02.08.02 - ported/created
//
// Copyright (c)  Microsoft Corporation.  All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"
#include "menu2.h"
#include "MenuDraw.h"   // DrawSpruBox
#include "main.h"       // TimeStep

#include "Confirm.h"

// re-volt specific
#include "menutext.h"   // re-volt strings

// temporary includes?
#include "titlescreen.h"

#define MENU_NUMPLAYERS_XPOS            Real(150)
#define MENU_NUMPLAYERS_YPOS            Real(100)

void CreateConfirmMenu(MENU_HEADER *menuHeader, MENU *menu);
void DecreaseConfirmValue(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
void IncreaseConfirmValue(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
void MenuConfirmYesNoGoBack(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);

void DrawConfirmYesNo(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex);

enum 
{
    CONFIRM_TYPE_YESNO,

		CONFIRM_NTYPES
};


char gConfirmMenuText[64];
char gConfirmMenuTextYes[32];
char gConfirmMenuTextNo[32];
long gConfirmMenuType = CONFIRM_TYPE_YESNO;
long gConfirmMenuReturnVal = 0;
static long gConfirmMenuReturnValDefault = CONFIRM_NO;

// Menu
MENU Menu_ConfirmYesNo = {
    TEXT_NONE,
    MENU_IMAGE_NONE,                        // Menu title bar image enum
    TITLESCREEN_CAMPOS_DONT_CHANGE,                      // Camera pos index
    MENU_DEFAULT | MENU_CENTRE_X | MENU_CENTRE_Y,   // Menu type
    SPRU_COL_KEEP,                          // Spru colour
    CreateConfirmMenu,                      // Create menu function
    MENU_NUMPLAYERS_XPOS,                   // X coord
    MENU_NUMPLAYERS_YPOS,                   // Y Coord
    0,                                      // Current Item index
    NULL,                                   // Parent Menu (OverWritten in Create)
};

// Confirm
MENU_ITEM MenuItem_ConfirmYesNo = {
    TEXT_NONE,                              // Text label index

    0,                                      // Space needed to draw item data
    NULL,                                   // Data

    DrawConfirmYesNo,                       // Draw Function

	NULL,								    // Input handler
	0,                                      // Flags

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    DecreaseConfirmValue,                   // Left Action
    IncreaseConfirmValue,                   // Right Action
#ifndef _PSX
    MenuConfirmYesNoGoBack,                 // Back Action
#else
    NULL,
#endif
    MenuGoBack,                             // Forward Action
};

// Create Function
void CreateConfirmMenu(MENU_HEADER *menuHeader, MENU *menu)
{
    int extra = (strlen(gConfirmMenuTextYes) || strlen(gConfirmMenuTextNo)) ? 10 : 0;
    MenuItem_ConfirmYesNo.DataWidth = (strlen(gConfirmMenuText) + strlen(gConfirmMenuTextYes) + strlen(gConfirmMenuTextNo) + extra) * Real(MENU_TEXT_WIDTH);    
    gConfirmMenuReturnVal = gConfirmMenuReturnValDefault;
    AddMenuItem(menuHeader, menu, &MenuItem_ConfirmYesNo);

    if (menu->pParentMenu != NULL) {
        menu->TextIndex = menu->pParentMenu->TextIndex;
        menu->ImageIndex = menu->pParentMenu->ImageIndex;
    } else {
        menu->TextIndex = TEXT_NONE;
        menu->ImageIndex = MENU_IMAGE_NONE;
    }

}

// Utility
void DecreaseConfirmValue(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    if (gConfirmMenuReturnVal == CONFIRM_NO) {
        gConfirmMenuReturnVal = CONFIRM_YES;
    }
}

void IncreaseConfirmValue(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    if (gConfirmMenuReturnVal == CONFIRM_YES) {
        gConfirmMenuReturnVal = CONFIRM_NO;
    }
}

void MenuConfirmYesNoGoBack(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    gConfirmMenuReturnVal = CONFIRM_GOBACK;
    MenuGoBack(menuHeader, menu, menuItem);
}

void SetConfirmMenuStrings(char *title, char *yes, char *no, long def)
{
    strncpy(gConfirmMenuText, title, 64);
    strncpy(gConfirmMenuTextYes, yes, 32);
    strncpy(gConfirmMenuTextNo, no, 32);
    gConfirmMenuReturnValDefault = def;
}

////////////////////////////////////////////////////////////////
//
// DrawConfirmYesNo
//
////////////////////////////////////////////////////////////////

void DrawConfirmYesNo(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex)
{
    REAL xPos, yPos, xSize, ySize;

    xPos = menuHeader->XPos;
    yPos = menuHeader->YPos;

    xSize = Real(strlen(gConfirmMenuText)) * MENU_TEXT_WIDTH;
    ySize = (MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP) * 3;
    if ((gConfirmMenuTextYes[0] != '\0') || (gConfirmMenuTextNo[0] != '\0')) {
        //tmp = (Real(strlen(gConfirmMenuTextYes) + strlen(gConfirmMenuTextNo) + 5)) * MENU_TEXT_WIDTH;
        //if (tmp > xSize) xSize = tmp;
        xSize += (strlen(gConfirmMenuTextYes) + strlen(gConfirmMenuTextNo) + 5) * Real(MENU_TEXT_WIDTH);
    }

    // Draw the spru
    /*DrawSpruBox(
        gMenuWidthScale * (xPos - MENU_TEXT_GAP), 
        gMenuHeightScale * (yPos - MENU_TEXT_GAP),
        gMenuWidthScale * (xSize + MENU_TEXT_GAP * 2),
        gMenuHeightScale * (ySize + MENU_TEXT_GAP * 2), 
        rand() % SPRU_COL_RANDOM);
	*/

    // Confirm message
    DrawMenuText(xPos, yPos, MENU_TEXT_RGB_HILITE, gConfirmMenuText);

    // Choices
    if (gConfirmMenuReturnVal == CONFIRM_YES) {
        xPos += Real(strlen(gConfirmMenuText) + 5) * MENU_TEXT_WIDTH;
        DrawMenuText(xPos, yPos, MENU_TEXT_RGB_CHOICE, gConfirmMenuTextYes);
        xPos += Real(strlen(gConfirmMenuTextYes) + 5) * MENU_TEXT_WIDTH;
        DrawMenuText(xPos, yPos, MENU_TEXT_RGB_NOTCHOICE, gConfirmMenuTextNo);
    } else {
        xPos += Real(strlen(gConfirmMenuText) + 5) * MENU_TEXT_WIDTH;
        DrawMenuText(xPos, yPos, MENU_TEXT_RGB_NOTCHOICE, gConfirmMenuTextYes);
        xPos += Real(strlen(gConfirmMenuTextYes) + 5) * MENU_TEXT_WIDTH;
        DrawMenuText(xPos, yPos, MENU_TEXT_RGB_CHOICE, gConfirmMenuTextNo);
    }
}

