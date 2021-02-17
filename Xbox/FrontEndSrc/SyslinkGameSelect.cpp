//-----------------------------------------------------------------------------
// File: SyslinkGameSelect.cpp
//
// Desc: SyslinkGameSelect.cpp
//
// Hist: 02.08.02 - ported/created
//
// Copyright (c)  Microsoft Corporation.  All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"
#include "menu2.h"
#include "MenuDraw.h"   // DrawSpruBox
#include "LevelLoad.h"  // GAMETYPE
#include "GetPlayerName.h"
#include "ConnectionType.h"

// re-volt specific
#include "menutext.h"   // re-volt strings

// temporary includes?
#include "titlescreen.h"  // TITLESCREEN_CAMPOS_ & g_bTitleScreenRunDemo

#define MENU_MULTIGAMESELECT_XPOS       Real(0)
#define MENU_MULTIGAMESELECT_YPOS       Real(0)

static void CreateMultiGameSelectMenu(MENU_HEADER *menuHeader, MENU *menu);

static void SelectPrevMultiGame(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
static void SelectNextMultiGame(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
static void MenuJoinSession(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);

static void DrawMultiGameList(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex);

long SessionRefreshFlag;
long SessionJoinFlag;


// Menu
MENU Menu_MultiGameSelect = {
    TEXT_MULTISELECT,
    MENU_IMAGE_MULTI,                       // Menu title bar image enum
    TITLESCREEN_CAMPOS_MULTI,                        // Camera pos index
    MENU_DEFAULT,                           // Menu type
    SPRU_COL_RANDOM,                        // Spru colour
    CreateMultiGameSelectMenu,              // Create menu function
    MENU_MULTIGAMESELECT_XPOS,              // X coord
    MENU_MULTIGAMESELECT_YPOS,              // Y Coord
    0,                                      // Current Item index
    NULL,                                   // Parent Menu (OverWritten in Create)
};

// Multiplayer select game
MENU_ITEM MenuItem_MultiGameSelect = {
    TEXT_NONE,                              // Text label index

    640,                                    // Space needed to draw item data
    &Menu_PlayerName,                       // Data (next menu in this case)
    DrawMultiGameList,                      // Update State Function (Refresh list and draw)

 	NULL,								    // Input handler
	0,                                      // Flags

    SelectPrevMultiGame,                    // Up Action
    SelectNextMultiGame,                    // Down Action
    NULL,                                   // Left Action
    NULL,                                   // Right Action
    MenuGoBackKillPlay,                     // Back Action
    MenuJoinSession,                        // Forward Action
};

// Create
void CreateMultiGameSelectMenu(MENU_HEADER *menuHeader, MENU *menu)
{
#ifndef XBOX_DISABLE_NETWORK

    // prepare for session enum
    SessionCount = 0;
    SessionPick = 0;
    SessionRefreshFlag = 3;
    SessionJoinFlag = 0;

    if (SessionCount)
        ListPlayers(&SessionList[SessionPick].Guid);
#endif // ! XBOX_DISABLE_NETWORK

    // Add the dummy menu item necessary to check for keys and call the frame update function
    AddMenuItem(menuHeader, menu, &MenuItem_MultiGameSelect);
    g_bMenuDrawMenuBox = FALSE;                                   // Don't draw a default box
}

// Utility

void SelectPrevMultiGame(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
#ifndef XBOX_DISABLE_NETWORK
    if (SessionPick)
    {
        SessionPick--;
        PlayerCount = 0;
    }
#endif // ! XBOX_DISABLE_NETWORK
}

void SelectNextMultiGame(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
#ifndef XBOX_DISABLE_NETWORK
    if (SessionPick < SessionCount - 1)
    {
        SessionPick++;
        PlayerCount = 0;
    }
#endif // ! XBOX_DISABLE_NETWORK
}

// join session
void MenuJoinSession(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
#ifndef XBOX_DISABLE_NETWORK

    // any sessions?
    if (!SessionCount)
        return;

    // set session join flag
    if (!SessionJoinFlag)
    {
        SessionJoinFlag = 3;
    }
#endif // ! XBOX_DISABLE_NETWORK
}

////////////////////////////////////////////////////////////////
//
// DrawMultiGameList
//
////////////////////////////////////////////////////////////////
#ifdef _PC
void DrawMultiGameList(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex)
{
 #ifndef XBOX_DISABLE_NETWORK
    short i, j, k;
    REAL xPos, yPos, xSize, ySize;
    long rgb;
    char *status;

// request sessions / players?

    if (Keys[DIK_SPACE] && !LastKeys[DIK_SPACE] && !SessionRefreshFlag)
    {
        PlaySfx(SFX_MENU_FORWARD, SFX_MAX_VOL, SFX_CENTRE_PAN, 44100, 0x7fffffff);
        SessionRefreshFlag = 3;
    }

    if (SessionRefreshFlag)
    {
        if (!--SessionRefreshFlag)
        {
            RefreshSessions();
        }
    }

// draw headings

    xSize = (58 * MENU_TEXT_WIDTH + 2 * MENU_TEXT_GAP);
    ySize = (26 * MENU_TEXT_HEIGHT + 2 * MENU_TEXT_GAP);

    xPos = 640 - xSize - 40;
    yPos = 120;

    xPos += menuHeader->XPos;
    yPos += menuHeader->YPos;

// draw spru box
    DrawSpruBox(
        gMenuWidthScale * (xPos - MENU_TEXT_GAP),
        gMenuHeightScale * (yPos - MENU_TEXT_GAP + 20),
        gMenuWidthScale * (xSize),
        gMenuHeightScale * (ySize - 30),
        menuHeader->SpruColIndex, 0);

    DrawMenuText(xPos, yPos, MENU_TEXT_RGB_HILITE, TEXT_TABLE(TEXT_HOST));
    DrawMenuText(xPos + MENU_TEXT_WIDTH * 18, yPos, MENU_TEXT_RGB_HILITE, TEXT_TABLE(TEXT_TRACK));
    DrawMenuText(xPos + MENU_TEXT_WIDTH * 37, yPos, MENU_TEXT_RGB_HILITE, TEXT_TABLE(TEXT_GAME));
    DrawMenuText(xPos + MENU_TEXT_WIDTH * 48, yPos, MENU_TEXT_RGB_HILITE, TEXT_TABLE(TEXT_STATUS));

    if (SessionRefreshFlag)
    {
        DrawSmallText((640 - (strlen(gTitleScreen_Text[TEXT_GAME_SEARCH]) * 8)) / 2, 430, 0xffffffff, gTitleScreen_Text[TEXT_GAME_SEARCH]);
    }
    else if (SessionJoinFlag)
    {
        DrawSmallText((640 - (strlen(gTitleScreen_Text[TEXT_GAME_JOIN]) * 8)) / 2, 430, 0xffffffff, gTitleScreen_Text[TEXT_GAME_JOIN]);
    }
    else
    {
        DrawMenuText(xPos - MENU_TEXT_GAP + (xSize - strlen(gTitleScreen_Text[TEXT_GAME_LIST]) * MENU_TEXT_WIDTH)/2 , 430 + menuHeader->YPos, MENU_TEXT_RGB_NORMAL, gTitleScreen_Text[TEXT_GAME_LIST]);
    }

    yPos += MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP;

// draw sessions + players

    if (SessionCount)
    {
        for (i = 0 ; i < SessionCount ; i++, yPos += MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP)
        {
            if (SessionPick == i) rgb = TIME2MS(TimerCurrent) & 128 ? 0x404040 : MENU_TEXT_RGB_NORMAL;
            else rgb = MENU_TEXT_RGB_NORMAL;

            char hostname[MAX_PLAYER_NAME + 1], levelname[MAX_LEVEL_NAME + 1], leveldir[MAX_LEVEL_DIR_NAME + 1];

            j = 0;
            while ((hostname[j] = SessionList[i].Name[j]) != '\n' && SessionList[i].Name[j]) j++;
            hostname[j] = 0;
            k = 0;
            while ((leveldir[k] = SessionList[i].Name[k + j + 1])) k++;

            int levelNum = GetLevelNum(leveldir);
            LEVELINFO *levelInfo = GetLevelInfo(levelNum);
            if (levelInfo != NULL) {
                strncpy(levelname, levelInfo->Name, MAX_LEVEL_NAME);
            } else {
                levelname[0] = '\0';
            }

            if (i == SessionPick) {
                g_iTrackScreenLevelNum = levelNum;
            }

            DrawMenuText(xPos, yPos, rgb, hostname);
            DrawMenuText(xPos + (MENU_TEXT_WIDTH * (MAX_PLAYER_NAME + 2)), yPos, rgb, SessionList[i].RandomTrack ? TEXT_TABLE(TEXT_RANDOM) : levelname);
            DrawMenuText(xPos + (MENU_TEXT_WIDTH * (MAX_PLAYER_NAME + 21)), yPos, rgb, SessionList[i].GameType == GAMETYPE_MULTI ? TEXT_TABLE(TEXT_RACE) : TEXT_TABLE(TEXT_BATTLE));


            if (SessionList[i].Version != (unsigned long)MultiplayerVersion) status = TEXT_TABLE(TEXT_WRONGVERSION);
            else if (SessionList[i].Started) status = TEXT_TABLE(TEXT_STARTED);
            else if ((DWORD)PlayerCount == SessionList[i].PlayerNum) status = TEXT_TABLE(TEXT_FULL);
            else status = TEXT_TABLE(TEXT_OPEN);

            DrawMenuText(xPos + (MENU_TEXT_WIDTH * (MAX_PLAYER_NAME + 32)), yPos, rgb, status);
        }

        if (PlayerCount)
        {
            yPos = 240 + menuHeader->YPos;

            DrawMenuText(xPos, yPos, MENU_TEXT_RGB_HILITE, TEXT_TABLE(TEXT_PLAYER_SINGULAR));
            DrawMenuText(xPos + MENU_TEXT_WIDTH * 18, yPos, MENU_TEXT_RGB_HILITE, TEXT_TABLE(TEXT_CAR));
            DrawMenuText(xPos + MENU_TEXT_WIDTH * 36, yPos, MENU_TEXT_RGB_HILITE, TEXT_TABLE(TEXT_STATUS));

            yPos += MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP;
            for (i = 0 ; i < PlayerCount ; i++, yPos += MENU_TEXT_HEIGHT)
            {
                DrawMenuText(xPos, yPos, MENU_TEXT_RGB_NORMAL, PlayerList[i].Name);
                if (SessionList[SessionPick].RandomCars) DrawMenuText(xPos + (MENU_TEXT_WIDTH * (MAX_PLAYER_NAME + 2)), yPos, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_RANDOM));
                else DrawMenuText(xPos + (MENU_TEXT_WIDTH * (MAX_PLAYER_NAME + 2)), yPos, MENU_TEXT_RGB_NORMAL, PlayerList[i].Data.CarName);
                DrawMenuText(xPos + (MENU_TEXT_WIDTH * (MAX_PLAYER_NAME + 20)), yPos, MENU_TEXT_RGB_NORMAL, PlayerList[i].Data.Ready ? TEXT_TABLE(TEXT_READY) : TEXT_TABLE(TEXT_NOTREADY));
            }
        }
    }

// are we trying to join?

    if (SessionJoinFlag)
    {

// yep, flag ready?

        if (--SessionJoinFlag)
        {
            return;
        }

// yep, refresh session list

        RefreshSessions();

// valid session?

        if (SessionPick > SessionCount - 1)
            return;

// wrong checksum?

        if (SessionList[SessionPick].Version != (unsigned long)MultiplayerVersion)
            return;

// session open?

        if (SessionList[SessionPick].Started)
            return;

// max players?

        if (SessionList[SessionPick].PlayerNum == (DWORD)PlayerCount)
            return;

// yep, join session, create player

        LocalPlayerReady = FALSE;
        bGameStarted = FALSE;
        HostQuit = FALSE;
        SessionRequestTime = 0;

        if (!JoinSession((char)SessionPick))
        {
            for (i = SessionPick ; i < SessionCount - 1; i++)
            {
                SessionList[i] = SessionList[i + 1];
            }

            SessionPick = 0;
            SessionCount--;
            return;
        }

        CreatePlayer(g_TitleScreenData.nameEnter[0], DP_CLIENT_PLAYER);

// set random car / track flag

        GameSettings.RandomCars = SessionList[SessionPick].RandomCars;
        GameSettings.RandomTrack = SessionList[SessionPick].RandomTrack;

// next menu

        MenuGoForward(menuHeader, menu, menuItem);
    }
 #endif // ! XBOX_DISABLE_NETWORK
}
#endif

