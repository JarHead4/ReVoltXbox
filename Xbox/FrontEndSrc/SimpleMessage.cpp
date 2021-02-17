//-----------------------------------------------------------------------------
// File: SimpleMessage.cpp
//
// Desc: SimpleMessage.cpp
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

// menus
#include "MainMenu.h"

// temporary includes?
#include "titlescreen.h"

void CreateInitialMessageMenu(MENU_HEADER *menuHeader, MENU *menu);
void CreateMessageMenu(MENU_HEADER *menuHeader, MENU *menu);

void DrawInitialMessage(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex);

// Menu
MENU Menu_Message = {
    TEXT_NONE,
    MENU_IMAGE_NONE,                        // Menu title bar image enum
    TITLESCREEN_CAMPOS_DONT_CHANGE,                  // Camera pos index
    MENU_DEFAULT | MENU_CENTRE_X | MENU_CENTRE_Y,   // Menu type
    SPRU_COL_KEEP,                          // Spru colour
    CreateMessageMenu,                      // Create menu function
    0,                                      // X coord
    0,                                      // Y Coord
    0,                                      // Current Item index
    NULL,                                   // Parent Menu (OverWritten in Create)
};

// Menu item
MENU_ITEM MenuItem_Message = {
    TEXT_NONE,                              // Text label index

    0,                                      // Space needed to draw item data
    NULL,                                   // Data

    DrawInitialMessage,                     // Draw Function

	NULL,								    // Input handler
	0,                                      // Flags

    NULL,                                   // Up Action
    NULL,                                   // Down Action
    NULL,                                   // Left Action
    NULL,                                   // Right Action
    MenuGoBack,                             // Back Action
    MenuGoBack,                             // Forward Action
};

// create
void CreateMessageMenu(MENU_HEADER *menuHeader, MENU *menu)
{
    AddMenuItem(menuHeader, menu, &MenuItem_Message);

    if (menu->pParentMenu != NULL) {
        menu->TextIndex = menu->pParentMenu->TextIndex;
        menu->ImageIndex = menu->pParentMenu->ImageIndex;
    } else {
        menu->TextIndex = TEXT_NONE;
        menu->ImageIndex = MENU_IMAGE_NONE;
    }

    menuHeader->ItemTextWidth = Real(MENU_TEXT_WIDTH) * InitialMenuMessageWidth;
    g_bMenuDrawMenuBox = FALSE;
}


////////////////////////////////////////////////////////////////
//
// Initial Message menu (for bonus messages)
//
////////////////////////////////////////////////////////////////

// Menu
MENU Menu_InitialMessage = {
    TEXT_NONE,
    MENU_IMAGE_NONE,                        // Menu title bar image enum
    TITLESCREEN_CAMPOS_INIT,                         // Camera pos index
    MENU_DEFAULT | MENU_CENTRE_X | MENU_CENTRE_Y,   // Menu type
    SPRU_COL_RANDOM,                        // Spru colour
    CreateInitialMessageMenu,               // Create menu function
    0,                                      // X coord
    0,                                      // Y Coord
    0,                                      // Current Item index
    NULL,                                   // Parent Menu (OverWritten in Create)
};

// Menu item
MENU_ITEM MenuItem_InitialMessage = {
    TEXT_NONE,                              // Text label index

    0,                                      // Space needed to draw item data
    &Menu_TopLevel,                         // Data

    DrawInitialMessage,                     // Draw Function

	NULL,								    // Input handler
	0,                                      // Flags

    NULL,                                   // Up Action
    NULL,                                   // Down Action
    NULL,                                   // Left Action
    NULL,                                   // Right Action
#ifndef _PSX
    MenuGoForward,                          // Back Action
    MenuGoForward,                          // Forward Action
#else
    NextMenuMessage,
    NextMenuMessage,
#endif
};

// create
void CreateInitialMessageMenu(MENU_HEADER *menuHeader, MENU *menu)
{
    AddMenuItem(menuHeader, menu, &MenuItem_InitialMessage);

    // Switch off "Go-Back" icon
    menuHeader->NavFlags &= ~MENU_FLAG_GOBACK;

    // Clear the message flag
#ifndef _PSX
    InitialMenuMessage = MENU_MESSAGE_NONE;
    menuHeader->ItemTextWidth = Real(MENU_TEXT_WIDTH) * InitialMenuMessageWidth;
#else
    if (InitialMenuMessage == MENU_MESSAGE_REVERSE) {
        InitialMenuMessage = MENU_MESSAGE_NEWCARS;
    } else {
        InitialMenuMessage = MENU_MESSAGE_PLEASE_SAVE;
    }
#endif

    g_bMenuDrawMenuBox = FALSE;
}

void NextMenuMessage(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    if (InitialMenuMessage == MENU_MESSAGE_NONE) {
        MenuGoForward(menuHeader, menu, menuItem);
    } else {
        SetBonusMenuMessage();
        InitMenuMessage(TO_TIME(Real(5)));
        if (InitialMenuMessage == MENU_MESSAGE_PLEASE_SAVE) {
            InitialMenuMessage = MENU_MESSAGE_NONE;
        } else {
            InitialMenuMessage = MENU_MESSAGE_PLEASE_SAVE;
        }
    }
}



////////////////////////////////////////////////////////////////
//
// Draw Initial menu message
//
////////////////////////////////////////////////////////////////


void DrawInitialMessage(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex)
{
    int iLine, sLen;
    REAL xPos, yPos, xSize, ySize;

    xPos = menuHeader->XPos;
    yPos = menuHeader->YPos;
    xSize = Real(MENU_TEXT_WIDTH) * InitialMenuMessageWidth;
    ySize = Real(MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP) * InitialMenuMessageCount;

    DrawSpruBox(
        gMenuWidthScale * (xPos - MENU_TEXT_GAP),
        gMenuHeightScale * (yPos - MENU_TEXT_GAP),
        gMenuWidthScale * (xSize + 2*MENU_TEXT_GAP),
        gMenuHeightScale * (ySize + 2*MENU_TEXT_GAP),
        menuHeader->SpruColIndex, 0);

    for (iLine = 0; iLine < InitialMenuMessageCount; iLine++) {
        sLen = strlen(InitialMenuMessageLines[iLine]);

        xPos = (menuHeader->XPos + (xSize - Real(MENU_TEXT_WIDTH) * sLen) / 2);

        DrawMenuText(xPos, yPos, MENU_TEXT_RGB_HILITE, InitialMenuMessageLines[iLine]);
        yPos += MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP;
    }

    // See if its time to move on
    InitialMenuMessageTimer += TimeStep;
    if (InitialMenuMessageTimer > InitialMenuMessageMaxTime) {
        InitialMenuMessageTimer = ZERO;
        MenuGoForward(menuHeader, menu, menuItem);
    }

}




