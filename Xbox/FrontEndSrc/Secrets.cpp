//-----------------------------------------------------------------------------
// File: Secrets.cpp
//
// Desc: Secrets.cpp
//
// Hist: 02.08.02 - ported/created
//
// Copyright (c)  Microsoft Corporation.  All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"
#include "menu2.h"
#include "MenuDraw.h"   // DrawSpruBox
#include "panel.h"      // DrawPanelSprite
#include "main.h"       // RenderSettings

// re-volt specific
#include "menutext.h"   // re-volt strings

// temporary includes?
#include "titlescreen.h"  // TITLESCREEN_CAMPOS_ & g_bTitleScreenRunDemo

#define MENU_SECRETS_XPOS               Real(0)
#define MENU_SECRETS_YPOS               Real(0)

static void CreateSecretsMenu(MENU_HEADER *menuHeader, MENU *menu);

static void DrawSecrets(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex);

// Menu
MENU Menu_Secrets = {
    TEXT_SECRETS,
    MENU_IMAGE_TRAINING,                    // Menu title bar image enum
    TITLESCREEN_CAMPOS_ALL_TROPHIES,                 // Camera pos index
    MENU_DEFAULT | MENU_CENTRE_X,           // Menu type
    SPRU_COL_RANDOM,                        // Spru colour
    CreateSecretsMenu,                      // Create menu function
    MENU_SECRETS_XPOS,                      // X coord
    MENU_SECRETS_YPOS,                      // Y Coord
    0,                                      // Current Item index
    NULL,                                   // Parent Menu (OverWritten in Create)
};

// Secrets - dummy menu item
MENU_ITEM MenuItem_SecretsDummy = {
    TEXT_NONE,                              // Text label index

    640,                                    // Space needed to draw item data
    NULL,                                   // Data (Menu to set up game and then run it)
    DrawSecrets,                            // Draw Function

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
void CreateSecretsMenu(MENU_HEADER *menuHeader, MENU *menu)
{
    AddMenuItem(menuHeader, menu, &MenuItem_SecretsDummy);
    g_bMenuDrawMenuBox = FALSE;

    // Switch off "Go-Forward" icon
    menuHeader->NavFlags &= ~MENU_FLAG_ADVANCE;
}

/////////////////////////////////////////////////////////////////////
//
// Draw Secrets
//
/////////////////////////////////////////////////////////////////////

#define TICK_TU     (244.0f / 256.0f)
#define CROSS_TU    (231.0f / 256.0f)

void DrawSecrets(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex)
{
#ifdef _PC
    long i, cup;
    REAL xpos, ypos, y;
    LEVELINFO *li;
    char buf[128];

// draw spru's

    xpos = menuHeader->XPos + 60;
    ypos = menuHeader->YPos + 112;

    DrawSpruBox(
        xpos * RenderSettings.GeomScaleX,
        ypos * RenderSettings.GeomScaleY,
        Real(640 - 120) * RenderSettings.GeomScaleX,
        Real(480 - 208) * RenderSettings.GeomScaleY,
        menuHeader->SpruColIndex, 0);   

    DrawSpruBox(
        xpos * RenderSettings.GeomScaleX,
        (ypos + 292) * RenderSettings.GeomScaleY,
        Real(640 - 120) * RenderSettings.GeomScaleX,
        Real(24) * RenderSettings.GeomScaleY,
        menuHeader->SpruColIndex, 0);

// menu headings

    DrawMenuText(xpos + 4, ypos + 4, 0xffffff, TEXT_TABLE(TEXT_SECRETS_TRACKS));

    DrawMenuText(xpos + 4 + MENU_TEXT_WIDTH * 22 - strlen(TEXT_TABLE(TEXT_SECRETS_WONRACE_TOP)) * 4, ypos + 4, 0xffffff, TEXT_TABLE(TEXT_SECRETS_WONRACE_TOP));
    DrawMenuText(xpos + 4 + MENU_TEXT_WIDTH * 22 - strlen(TEXT_TABLE(TEXT_SECRETS_WONRACE_BOTTOM)) * 4, ypos + 4 + MENU_TEXT_HEIGHT, 0xffffff, TEXT_TABLE(TEXT_SECRETS_WONRACE_BOTTOM));

    DrawMenuText(xpos + 4 + MENU_TEXT_WIDTH * 30 - strlen(TEXT_TABLE(TEXT_SECRETS_NORM)) * 4, ypos + 4, 0xffffff, TEXT_TABLE(TEXT_SECRETS_NORM));
    DrawMenuText(xpos + 4 + MENU_TEXT_WIDTH * 40 - strlen(TEXT_TABLE(TEXT_SECRETS_REV)) * 4, ypos + 4, 0xffffff, TEXT_TABLE(TEXT_SECRETS_REV));
    DrawMenuText(xpos + 4 + MENU_TEXT_WIDTH * 50 - strlen(TEXT_TABLE(TEXT_SECRETS_MIRR)) * 4, ypos + 4, 0xffffff, TEXT_TABLE(TEXT_SECRETS_MIRR));

    DrawMenuText(xpos + 4 + MENU_TEXT_WIDTH * 60 - strlen(TEXT_TABLE(TEXT_SECRETS_STARS)) * 4, ypos + 4, 0xffffff, TEXT_TABLE(TEXT_SECRETS_STARS));

// loop thru each cup level

    cup = -1;
    y = ypos + 4 + MENU_TEXT_HEIGHT;
    for (i = 0 ; i < LEVEL_NCUP_LEVELS ; i++)
    {
        li = GetLevelInfo(i);

        // next cup?
        if (li->LevelClass != cup)
        {
            y += MENU_TEXT_HEIGHT;
            cup = li->LevelClass;
            DrawMenuText(xpos + 4, y, 0xffffff, TEXT_TABLE(TEXT_COMPETE_CUP_DEFAULT + cup));
            DrawPanelSprite(xpos + 4 + strlen(TEXT_TABLE(TEXT_COMPETE_CUP_DEFAULT + cup)) * MENU_TEXT_WIDTH + 4, y, 12.0f, 12.0f, IsCupCompleted(cup) ? TICK_TU : CROSS_TU, 244.0f / 256.0f, 11.0f / 256.0f, 11.0f / 256.0f, 0xffffff);
            y += MENU_TEXT_HEIGHT;
        }

        // level name
        DrawMenuText(xpos + 4 + MENU_TEXT_WIDTH, y, 0x808080, li->Name);

        // won race
        DrawPanelSprite(xpos + 172, y, 12.0f, 12.0f, IsSecretWonSingleRace(i) ? TICK_TU : CROSS_TU, 244.0f / 256.0f, 11.0f / 256.0f, 11.0f / 256.0f, 0xffffff);

        // challenge normal
        DrawPanelSprite(xpos + 240, y, 12.0f, 12.0f, IsSecretBeatTimeTrial(i) ? TICK_TU : CROSS_TU, 244.0f / 256.0f, 11.0f / 256.0f, 11.0f / 256.0f, 0xffffff);

        // challenge reversed
        DrawPanelSprite(xpos + 320, y, 12.0f, 12.0f, IsSecretBeatTimeTrialReverse(i) ? TICK_TU : CROSS_TU, 244.0f / 256.0f, 11.0f / 256.0f, 11.0f / 256.0f, 0xffffff);

        // challenge mirrored
        DrawPanelSprite(xpos + 400, y, 12.0f, 12.0f, IsSecretBeatTimeTrialMirror(i) ? TICK_TU : CROSS_TU, 244.0f / 256.0f, 11.0f / 256.0f, 11.0f / 256.0f, 0xffffff);

        // found practice stars
        DrawPanelSprite(xpos + 476, y, 12.0f, 12.0f, IsSecretFoundPractiseStars(i) ? TICK_TU : CROSS_TU, 244.0f / 256.0f, 11.0f / 256.0f, 11.0f / 256.0f, 0xffffff);

        // next line
        y += MENU_TEXT_HEIGHT;
    }

	// show stunt stars found
    sprintf(MenuBuffer, "%s - %s ", TEXT_TABLE(TEXT_TRAINING2), TEXT_TABLE(TEXT_LOADSCREEN_STARSCOLLECTED));
    DrawMenuText(xpos + 4, ypos + 292 + 6, 0xffffff, MenuBuffer);
    sprintf(buf, "%ld %s %ld", StarList.NumFound, TEXT_TABLE(TEXT_OF), StarList.NumTotal);
    DrawMenuText(xpos + 4 + MENU_TEXT_WIDTH * strlen(MenuBuffer), ypos + 292 + 6, 0x00ff00, buf);
#endif
}

