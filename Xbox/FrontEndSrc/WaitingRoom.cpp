//-----------------------------------------------------------------------------
// File: WaitingRoom.cpp
//
// Desc: WaitingRoom.cpp
//
// Hist: 02.08.02 - ported/created
//
// Copyright (c)  Microsoft Corporation.  All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"
#include "menu2.h"
#include "MenuDraw.h"   // DrawSpruBox
#include "settings.h"   // RegistrySettings
#include "WaitingRoom.h"    // StartData

// re-volt specific
#include "menutext.h"   // re-volt strings

// temporary includes?
#include "titlescreen.h"  // TITLESCREEN_CAMPOS_ & g_bTitleScreenRunDemo

#define MENU_WAITINGROOM_XPOS           Real(0)
#define MENU_WAITINGROOM_YPOS           Real(0)

static void CreateWaitingRoomMenu(MENU_HEADER *menuHeader, MENU *menu);
static void MenuGoBackFromWaitingRoom(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
static void DrawJoinedPlayerList(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex);

#ifdef _PC
char WaitingRoomMessages[WAITING_ROOM_MESSAGE_NUM][WAITING_ROOM_MESSAGE_LEN + MAX_PLAYER_NAME + 2];
char WaitingRoomCurrentMessage[WAITING_ROOM_MESSAGE_LEN];
long WaitingRoomMessagePos;
long WaitingRoomMessageRGB[WAITING_ROOM_MESSAGE_NUM];
REAL WaitingRoomMessageTimer;
long WaitingRoomMessageActive;
#endif


// Menu
MENU Menu_WaitingRoom = {
    TEXT_WAITINGROOM,
    MENU_IMAGE_MULTI,                       // Menu title bar image enum
    TITLESCREEN_CAMPOS_MULTI,                        // Camera pos index
    MENU_DEFAULT,                           // Menu type
    SPRU_COL_RANDOM,                        // Spru colour
    CreateWaitingRoomMenu,                  // Create menu function
    MENU_WAITINGROOM_XPOS,                  // X coord
    MENU_WAITINGROOM_YPOS,                  // Y Coord
    0,                                      // Current Item index
    NULL,                                   // Parent Menu (OverWritten in Create)
};

// Waiting Room - Start Race
MENU_ITEM MenuItem_WaitingRoom = {
    TEXT_NONE,                              // Text label index

    640,                                    // Space needed to draw item data
    NULL,                                   // Data (Menu to set up game and then run it)
    DrawJoinedPlayerList,                   // Draw Function

	NULL,								    // Input handler
	0,                                      // Flags

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    NULL,                                   // Left Action
    NULL,                                   // Right Action
    MenuGoBackFromWaitingRoom,              // Back Action
    NULL,                                   // Forward Action
};

// Create
void CreateWaitingRoomMenu(MENU_HEADER *menuHeader, MENU *menu)
{
    // set misc flags
    LocalPlayerReady = TRUE;
    SetPlayerData();

    if (IsServer())
    {
#ifndef XBOX_DISABLE_NETWORK //$REVISIT: Probably can remove; I don't think we need to call this (level name, etc gets propagated to clients manually, not via DPlay)
        LEVELINFO *levinf = GetLevelInfo(gTitleScreenVars.iLevelNum);
        SetSessionDesc(gTitleScreenVars.nameEnter[0], levinf->Dir, FALSE, GameSettings.GameType, GameSettings.RandomCars, GameSettings.RandomTrack);
#endif // !XBOX_DISABLE_NETWORK
    }

//$REMOVED_NOTUSED    PlayersRequestTime = Real(1.0f);

    WaitingRoomMessagePos = 0;
    WaitingRoomCurrentMessage[0] = 0;

    for (long i = 0 ; i < WAITING_ROOM_MESSAGE_NUM ; i++)
    {
        WaitingRoomMessages[i][0] = 0;
    }

    // Add menu items
    AddMenuItem(menuHeader, menu, &MenuItem_WaitingRoom);
    g_bMenuDrawMenuBox = FALSE;                                   // Don't draw a default box
}

// back from waiting room
void MenuGoBackFromWaitingRoom(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    LocalPlayerReady = FALSE;
    SetPlayerData();

    MenuGoBack(menuHeader, menu, menuItem);
}

//#endif

////////////////////////////////////////////////////////////////
//
// DrawJoinedPlayerList:
//
////////////////////////////////////////////////////////////////
#ifdef _PC

void DrawJoinedPlayerList(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex)
{
 #ifndef XBOX_DISABLE_NETWORK
    REAL xPos, yPos, xSize, ySize;
    short i, j, k;
    unsigned char c;

// update players?

    PlayersRequestTime += TimeStep;
    if (PlayersRequestTime > 1.0f)
    {
        ListPlayers(NULL);
        PlayersRequestTime = 0.0f;
    }

    xPos = (640 - 42 * MENU_TEXT_WIDTH) / 2;
    yPos = 120;

    xPos += menuHeader->XPos;
    yPos += menuHeader->YPos;

    xSize = (42 * MENU_TEXT_WIDTH + 2 * MENU_TEXT_GAP);
    ySize = (26 * MENU_TEXT_HEIGHT + 2 * MENU_TEXT_GAP);

// draw spru box
    DrawSpruBox(
        gMenuWidthScale * (xPos - MENU_TEXT_GAP),
        gMenuHeightScale * (yPos - MENU_TEXT_GAP),
        gMenuWidthScale * (xSize),
        gMenuHeightScale * (ySize),
        menuHeader->SpruColIndex, 0);

	// dump track name
    char hostname[MAX_PLAYER_NAME + 1], levelname[MAX_LEVEL_NAME + 1], leveldir[MAX_LEVEL_DIR_NAME + 1];
    DPSESSIONDESC2 desc[2];
    DWORD size = sizeof(DPSESSIONDESC2) * 2;

    HRESULT r = DP->GetSessionDesc(desc, &size);
    if (r != DP_OK) {
        ErrorDX(r, "Could not get sessions description");
    }

    j = 0;
    while ((hostname[j] = desc->lpszSessionNameA[j]) != '\n' && desc->lpszSessionNameA[j]) j++;
    hostname[j] = 0;
    k = 0;
    while ((leveldir[k] = desc->lpszSessionNameA[k + j + 1])) k++;

    g_iTrackScreenLevelNum = GetLevelNum(leveldir);
    LEVELINFO *levelInfo = GetLevelInfo(g_iTrackScreenLevelNum);
    if (levelInfo != NULL) {
        strncpy(levelname, levelInfo->Name, MAX_LEVEL_NAME);
    } else {
        levelname[0] = '\0';
    }

    sprintf(MenuBuffer, "%s:", TEXT_TABLE(TEXT_MULTITYPE));
    DrawMenuText(xPos, yPos, MENU_TEXT_RGB_HILITE, MenuBuffer);
    DrawMenuText(xPos + MENU_TEXT_WIDTH * 18, yPos, MENU_TEXT_RGB_NORMAL, desc->dwUser2 == GAMETYPE_MULTI ? TEXT_TABLE(TEXT_SINGLERACE) : TEXT_TABLE(TEXT_BATTLE));

    yPos += MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP;
    DrawMenuText(xPos, yPos, MENU_TEXT_RGB_HILITE, TEXT_TABLE(TEXT_LOADSCREEN_TRACK));
    DrawMenuText(xPos + MENU_TEXT_WIDTH * 18, yPos, MENU_TEXT_RGB_NORMAL, GameSettings.RandomTrack ? TEXT_TABLE(TEXT_RANDOM) : levelname);

// show players

    yPos += MENU_TEXT_HEIGHT + MENU_TEXT_GAP;

    DrawMenuText(xPos, yPos, MENU_TEXT_RGB_HILITE, TEXT_TABLE(TEXT_PLAYER_SINGULAR));
    DrawMenuText(xPos + MENU_TEXT_WIDTH * 18, yPos, MENU_TEXT_RGB_HILITE, TEXT_TABLE(TEXT_CAR));
    DrawMenuText(xPos + MENU_TEXT_WIDTH * 36, yPos, MENU_TEXT_RGB_HILITE, TEXT_TABLE(TEXT_STATUS));

    yPos += MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP;

    for (i = 0 ; i < PlayerCount ; i++, yPos += MENU_TEXT_HEIGHT)
    {
        DrawMenuText(xPos, yPos, MENU_TEXT_RGB_NORMAL, PlayerList[i].Name);
        if (GameSettings.RandomCars) DrawMenuText(xPos + (MENU_TEXT_WIDTH * (MAX_PLAYER_NAME + 2)), yPos, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_RANDOM));
        else DrawMenuText(xPos + (MENU_TEXT_WIDTH * (MAX_PLAYER_NAME + 2)), yPos, MENU_TEXT_RGB_NORMAL, PlayerList[i].Data.CarName);
        DrawMenuText(xPos + (MENU_TEXT_WIDTH * (MAX_PLAYER_NAME + 20)), yPos, MENU_TEXT_RGB_NORMAL, PlayerList[i].Data.Ready ? TEXT_TABLE(TEXT_READY) : TEXT_TABLE(TEXT_NOTREADY));
    }

    if (GameSettings.MultiType == MULTITYPE_SERVER)	// host
    {
        if (!(TIME2MS(TimerCurrent) & 128))
            DrawMenuText((640 - (strlen(TEXT_TABLE(TEXT_HITTABTOSTART)) * MENU_TEXT_WIDTH)) / 2 + (short)menuHeader->XPos, 430 + (short)menuHeader->YPos, MENU_TEXT_RGB_NOTCHOICE, TEXT_TABLE(TEXT_HITTABTOSTART));
        else
            DrawMenuText((640 - (strlen(TEXT_TABLE(TEXT_HITTABTOSTART)) * MENU_TEXT_WIDTH)) / 2 + (short)menuHeader->XPos, 430 + (short)menuHeader->YPos, MENU_TEXT_RGB_CHOICE, TEXT_TABLE(TEXT_HITTABTOSTART));

        if (Keys[DIK_TAB])
        {
            //SetRaceData(menuHeader, menu, menuItem);
            g_bTitleScreenRunGame = TRUE;

        }
    }
    else // client
    {
        if (HostQuit)
        {
            if (!(TIME2MS(TimerCurrent) & 256))
            {
                DrawMenuText(CENTRE_POS(TEXT_MULTIGAMETERMINATED) + (short)menuHeader->XPos, 430 + (short)menuHeader->YPos, MENU_TEXT_RGB_CHOICE, TEXT_TABLE(TEXT_MULTIGAMETERMINATED));
            }
        }
        else
        {
            DrawMenuText(CENTRE_POS(TEXT_WAITINGFORHOST) + (short)menuHeader->XPos, 430 + (short)menuHeader->YPos, MENU_TEXT_RGB_CHOICE, TEXT_TABLE(TEXT_WAITINGFORHOST));
        }

        if (bGameStarted)
        {
            //SetRaceData(menuHeader, menu, menuItem);
            g_bTitleScreenRunGame = TRUE;
        }
    }

// display messages

    yPos = 340 + menuHeader->YPos;
    for (i = 0 ; i < WAITING_ROOM_MESSAGE_NUM ; i++)
    {
        DrawMenuText(xPos, yPos + i * MENU_TEXT_HEIGHT, WaitingRoomMessageRGB[i], WaitingRoomMessages[i]);
    }

// display current message

    DrawMenuText(xPos, yPos + (MENU_TEXT_HEIGHT * 6), 0xffffff, WaitingRoomCurrentMessage);
    if (!(TIME2MS(TimerCurrent) & 256)) DrawMenuText(xPos + strlen(WaitingRoomCurrentMessage) * MENU_TEXT_WIDTH, yPos + (MENU_TEXT_HEIGHT * 6), 0xffffff, "_");

// input?

    if ((c = GetKeyPress()))
    {
        if (c == 27 || c == 9) // escape / tab
        {
            return;
        }

        if (c == 8) // delete
        {
            if (WaitingRoomMessagePos)
                {WaitingRoomMessagePos--;
                WaitingRoomCurrentMessage[WaitingRoomMessagePos] = 0;
            }
        }

        else if (c == 13) // enter
        {
            for (i = 0 ; i < WAITING_ROOM_MESSAGE_NUM - 1 ; i++)
            {
                strcpy(WaitingRoomMessages[i], WaitingRoomMessages[i + 1]);
                WaitingRoomMessageRGB[i] = WaitingRoomMessageRGB[i + 1];
            }

            wsprintf(WaitingRoomMessages[i], "%s: %s", g_TitleScreenData.nameEnter, WaitingRoomCurrentMessage);
            SendChatMessage(WaitingRoomMessages[i], GroupID);

            WaitingRoomMessageRGB[i] = 0xffffff;
            for (j = 0 ; j < PlayerCount ; j++)
            {
                if (PlayerList[j].PlayerID == LocalPlayerID)
                {
                    WaitingRoomMessageRGB[i] = MultiPlayerColours[j];
                    break;
                }
            }

            WaitingRoomMessagePos = 0;
            WaitingRoomCurrentMessage[0] = 0;
        }

        else if (WaitingRoomMessagePos < WAITING_ROOM_MESSAGE_LEN - 1) // normal char
        {
            WaitingRoomCurrentMessage[WaitingRoomMessagePos++] = c;
            WaitingRoomCurrentMessage[WaitingRoomMessagePos] = 0;
        }
    }

// get remote messages

    GetRemoteMessages();
 #endif // ! XBOX_DISABLE_NETWORK
}
#endif

