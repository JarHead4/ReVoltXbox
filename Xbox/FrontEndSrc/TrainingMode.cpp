//-----------------------------------------------------------------------------
//
// File: TrainingMode.cpp
//
// Desc: TrainingMode.cpp
//
// Hist: 02.08.02 - ported/created
//
// Copyright (c)  Microsoft Corporation.  All rights reserved.
//-----------------------------------------------------------------------------

#include "revolt.h"
#include "menu2.h"
#include "MenuDraw.h"   // DrawSpruBox
#include "text.h"       // BeginTextState

// re-volt specific
#include "menutext.h"   // re-volt strings

// temporary includes?
#include "titlescreen.h"  // TITLESCREEN_CAMPOS_ & g_bTitleScreenRunDemo

#define MENU_TRAINING_XPOS              Real(100)
#define MENU_TRAINING_YPOS              Real(150)

void CreateTrainingMenu(MENU_HEADER *menuHeader, MENU *menu);

// Menu
MENU Menu_Training = {
    TEXT_TRAINING2,
    MENU_IMAGE_TRAINING,                    // Menu title bar image enum
    TITLESCREEN_CAMPOS_DINKYDERBY,                   // Camera pos index
    MENU_DEFAULT,                           // Menu type
    SPRU_COL_RANDOM,                        // Spru colour
    CreateTrainingMenu,                     // Create menu function
    MENU_TRAINING_XPOS,                     // X coord
    MENU_TRAINING_YPOS,                     // Y Coord
    0,                                      // Current Item index
    NULL,                                   // Parent Menu (OverWritten in Create)
};

// Training - mode
MENU_ITEM MenuItem_TrainingMode = {
    TEXT_TRAININGMODE,                      // Text label index

    MENU_DATA_WIDTH_TEXT,                   // Space needed to draw item data
    NULL,                                   // Data (Menu to set up game and then run it)
    NULL,                                   // Draw Function

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
void CreateTrainingMenu(MENU_HEADER *menuHeader, MENU *menu)
{
    AddMenuItem(menuHeader, menu, &MenuItem_TrainingMode);
}

