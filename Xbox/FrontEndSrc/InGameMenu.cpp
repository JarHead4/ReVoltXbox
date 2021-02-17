//-----------------------------------------------------------------------------
// File: InGameMenu.cpp
//
// Desc: InGameMenu.cpp
//
// Hist: 02.08.02 - ported/created
//
// Copyright (c)  Microsoft Corporation.  All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"
#include "menu2.h"
#include "MenuDraw.h"   // DrawSpruBox
#include "settings.h"   // RegistrySettings
#include "LevelLoad.h"  // GAMETYPE
#include "InitPlay.h"   // StartDataStorage
#include "gameloop.h"   // GAME_LOOP_QUIT_
#include "draw.h"       // FADE_DOWN

#include "SimpleMessage.h"
#include "InGameMenu.h"
#include "Confirm.h"
#include "ConfirmGiveUp.h"

// re-volt specific
#include "menutext.h"   // re-volt strings

// menus
#include "Options.h"
#include "GameSettings.h"

// temporary includes?
#include "titlescreen.h"

#define MENU_INGAME_XPOS                Real(100)
#define MENU_INGAME_YPOS                Real(150)

static void CreateInGameMenu(MENU_HEADER *menuHeader, MENU *menu);

static void SetContinueChamp(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
static void SetEndChamp(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
static void SetGiveUpChampTry(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
static void SetRestartRace(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
static void SetViewReplay(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
static void SetRestartReplay(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
static void SetQuitRace(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
static void CreateInGameOptionsMenu(MENU_HEADER *menuHeader, MENU *menu);
static void SelectPreviousTrack(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
static void SelectNextTrack(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
static void AskRestartRace(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
static void AskQuitRace(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);

static void MenuSaveReplay(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
static void SetLoadReplay(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);

static void SetRestartRace(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
static void DrawNextTrack(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex);

long gConfirmType = -1;         // JCC: Fudged-in value set to say which menu item is waiting for confirm
                                        // 0 == Quit game; 1 == Restart game
                                        // Apologies to anyone who discovers this total fudge!

// Menu
MENU Menu_InGame = {
    TEXT_REVOLT,
    MENU_IMAGE_NONE,                        // Menu title bar image enum
    TITLESCREEN_CAMPOS_START,                        // Camera pos index
    MENU_DEFAULT | MENU_CENTRE_X | MENU_CENTRE_Y,   // Menu type
    SPRU_COL_RANDOM,                        // Spru colour
    CreateInGameMenu,                       // Create menu function
    MENU_INGAME_XPOS,                       // X coord
    MENU_INGAME_YPOS,                       // Y Coord
    0,                                      // Current Item index
    NULL,                                   // Parent Menu (OverWritten in Create)
};

// In Game - Resume
MENU_ITEM MenuItem_ResumeRace = {
    TEXT_RESUME_RACE,                       // Text label index

    0,                                      // Space needed to draw item data
    NULL,                                   // Data
    NULL,                                   // Draw Function

	NULL,								    // Input handler
	0,                                      // Flags

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    NULL,                                   // Left Action
    NULL,                                   // Right Action
    NULL,                                   // Back Action
    MenuGoForward,                          // Forward Action
};

// In Game - continue champ

MENU_ITEM MenuItem_ContinueChamp = {
    TEXT_CONTINUE_CHAMP,                    // Text label index

    0,                                      // Space needed to draw item data
    NULL,                                   // Data
    NULL,                                   // Draw Function

	NULL,								    // Input handler
	0,                                      // Flags

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    NULL,                                   // Left Action
    NULL,                                   // Right Action
    NULL,                                   // Back Action
    SetContinueChamp,                       // Forward Action
};

// In Game - end champ

MENU_ITEM MenuItem_EndChamp = {
    TEXT_END_CHAMP,                         // Text label index

    0,                                      // Space needed to draw item data
    NULL,                                   // Data
    NULL,                                   // Draw Function

	NULL,								    // Input handler
	0,                                      // Flags

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    NULL,                                   // Left Action
    NULL,                                   // Right Action
    NULL,                                   // Back Action
    SetEndChamp,                            // Forward Action
};

// In Game - give up champ try

MENU_ITEM MenuItem_GiveUpChampTry = {
    TEXT_GIVEUP_CHAMP,                      // Text label index

    0,                                      // Space needed to draw item data
    &Menu_ConfirmGiveup,                    // Data
    NULL,                                   // Draw Function

	NULL,								    // Input handler
	0,                                      // Flags

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    NULL,                                   // Left Action
    NULL,                                   // Right Action
    NULL,                                   // Back Action
    SetGiveUpChampTry,                      // Forward Action
};

// In Game - Restart
MENU_ITEM MenuItem_RestartRace = {
    TEXT_RESTART_RACE,                      // Text label index

    0,                                      // Space needed to draw item data
    &Menu_ConfirmYesNo,                     // Data
    NULL,                                   // Draw Function

 	NULL,								    // Input handler
	0,                                      // Flags

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    NULL,                                   // Left Action
    NULL,                                   // Right Action
    NULL,                                   // Back Action
    //SetRestartRace,                           // Forward Action
    AskRestartRace,                         // Forward Action
};

// In Game - Restart replay
MENU_ITEM MenuItem_RestartReplay = {
    TEXT_RESTART_REPLAY,                    // Text label index

    0,                                      // Space needed to draw item data
    NULL,                                   // Data
    NULL,                                   // Draw Function

	NULL,								    // Input handler
	0,                                      // Flags

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    NULL,                                   // Left Action
    NULL,                                   // Right Action
    NULL,                                   // Back Action
    SetRestartReplay,                       // Forward Action
};

// In Game - View Replay
MENU_ITEM MenuItem_ViewReplay = {
    TEXT_VIEW_REPLAY,                       // Text label index

    0,                                      // Space needed to draw item data
    NULL,                                   // Data
    NULL,                                   // Draw Function

 	NULL,								    // Input handler
	0,                                      // Flags

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    NULL,                                   // Left Action
    NULL,                                   // Right Action
    NULL,                                   // Back Action
    SetViewReplay,                          // Forward Action
};

// In Game - Options
MENU_ITEM MenuItem_InGameOptions = {
    TEXT_OPTIONS,                           // Text label index

    0,                                      // Space needed to draw item data
    &Menu_InGameOptions,                    // Data
    NULL,                                   // Draw Function

	NULL,								    // Input handler
	0,                                      // Flags

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    NULL,                                   // Left Action
    NULL,                                   // Right Action
    NULL,                                   // Back Action
    MenuGoForward,                          // Forward Action
};

// In Game - Quit
MENU_ITEM MenuItem_QuitRace = {
    TEXT_QUIT_RACE,                         // Text label index

    0,                                      // Space needed to draw item data
    &Menu_ConfirmYesNo,                     // Data
    NULL,                                   // Draw Function

 	NULL,								    // Input handler
	0,                                      // Flags

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    NULL,                                   // Left Action
    NULL,                                   // Right Action
    NULL,                                   // Back Action
    //SetQuitRace,                          // Forward Action
    //MenuGoForward,                            // Forward Action
    AskQuitRace,
};

#ifdef _PC
// In Game - Save Replay
MENU_ITEM MenuItem_SaveReplay = {
    TEXT_SAVE_REPLAY,                       // Text label index

    0,                                      // Space needed to draw item data
    &Menu_Message,                          // Data
    NULL,                                   // Draw Function

	NULL,								    // Input handler
	0,                                      // Flags

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    NULL,                                   // Left Action
    NULL,                                   // Right Action
    NULL,                                   // Back Action
    //SetQuitRace,                          // Forward Action
    MenuSaveReplay,                         // Forward Action
};
#endif

#ifdef _PC
// In Game - Next Track
MENU_ITEM MenuItem_NextTrack = {
    TEXT_NEXT_TRACK,                        // Text label index

    MENU_TEXT_WIDTH * 18,                   // Space needed to draw item data
    NULL,                                   // Data
    DrawNextTrack,                          // Draw Function

 	NULL,								    // Input handler
	0,                                      // Flags

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    SelectPreviousTrack,                    // Left Action
    SelectNextTrack,                        // Right Action
    NULL,                                   // Back Action
    NULL,                                   // Forward Action
};
#endif

// Create
void CreateInGameMenu(MENU_HEADER *menuHeader, MENU *menu)
{
    menu->CurrentItemIndex = 0;

    // Switch off "Go-Back" icon
    menuHeader->NavFlags &= ~MENU_FLAG_GOBACK;


    // Resume?
    if ((ChampionshipEndMode != CHAMPIONSHIP_END_MENU) &&
        (!ReachedEndOfReplay)) 
    {
            AddMenuItem(menuHeader, menu, &MenuItem_ResumeRace);
    }

    // continue / end championship
    if (ChampionshipEndMode == CHAMPIONSHIP_END_MENU || (GameSettings.GameType == GAMETYPE_REPLAY && StartDataStorage.GameType == GAMETYPE_CHAMPIONSHIP))
    {
        if (CupTable.QualifyFlag == CUP_QUALIFY_YES || CupTable.QualifyFlag == CUP_QUALIFY_TRIESLEFT)
            {
            // PT: Set the label to continue or end champion ship, depending if we are finished
            #ifdef _N64
            MenuItem_ContinueChamp.TextIndex = ((CupTable.QualifyFlag == CUP_QUALIFY_YES) && (CupTable.RaceNum+1) == CupData[CupTable.CupType].NRaces) ? TEXT_END_CHAMP : TEXT_CONTINUE_CHAMP;
            #endif
            AddMenuItem(menuHeader, menu, &MenuItem_ContinueChamp);
            }
        else
            AddMenuItem(menuHeader, menu, &MenuItem_EndChamp);
    }

    // give up championship try?
    if (GameSettings.GameType == GAMETYPE_CHAMPIONSHIP && ChampionshipEndMode == CHAMPIONSHIP_END_WAITING_FOR_FINISH && !Players[0].RaceFinishTime)
        AddMenuItem(menuHeader, menu, &MenuItem_GiveUpChampTry);

#if defined(_PC)
    // PC Restart?
    //---------------------------
    if ((GameSettings.GameType != GAMETYPE_CHAMPIONSHIP) &&         // Not in championship
        (GameSettings.GameType != GAMETYPE_REPLAY || StartDataStorage.GameType != GAMETYPE_CHAMPIONSHIP) && // Not in championship replay
        (GameSettings.GameType != GAMETYPE_PRACTICE) &&             // not in practice
        (GameSettings.GameType != GAMETYPE_TRAINING) &&             // not in training
        (!IsMultiPlayer() || (IsServer() && !ReplayMode)) &&        // Not in multiplayer unless server
        !(ReplayMode && (StartDataStorage.GameType == GAMETYPE_MULTI || StartDataStorage.GameType == GAMETYPE_BATTLE))  // Not if replaying and was a multiplayer game
        )
#endif
    {
        AddMenuItem(menuHeader, menu, &MenuItem_RestartRace);
    }

    // next track
#ifdef _PC
    if (IsMultiPlayer() && IsServer() && !GameSettings.RandomTrack)
    {
        AddMenuItem(menuHeader, menu, &MenuItem_NextTrack);
    }
#endif

    // Replay
#if defined (_PC)
    if (RPL_RecordReplay && !ReplayMode && !IsMultiPlayer()) {
        if (PLR_LocalPlayer->RaceFinishTime != 0) 
#endif
        {
            AddMenuItem(menuHeader, menu, &MenuItem_ViewReplay);
        }
    } else if (ReplayMode) {
        AddMenuItem(menuHeader, menu, &MenuItem_RestartReplay);
    }

    // In-game best times
    if (GameSettings.GameType == GAMETYPE_TRIAL){
        AddMenuItem(menuHeader, menu, &MenuItem_BestTimes);
    }
    // In-game options
    AddMenuItem(menuHeader, menu, &MenuItem_InGameOptions);

    // Save Replay for Game Gauge (in Dev mode)
    if (Version == VERSION_DEV) {
        AddMenuItem(menuHeader, menu, &MenuItem_SaveReplay);
        AddMenuItem(menuHeader, menu, &MenuItem_SaveLocalPlayerCarInfo);
    }

    // Quit
    AddMenuItem(menuHeader, menu, &MenuItem_QuitRace);

    // Have we quit 
    if (gConfirmMenuReturnVal == CONFIRM_YES) {
        if (gConfirmType == 0) {
            SetQuitRace(menuHeader, menu, &MenuItem_QuitRace);
            ClearMenuHeader(&gMenuHeader);
        } else if (gConfirmType == 1) {
            SetRestartRace(menuHeader, menu, &MenuItem_RestartRace);
            ClearMenuHeader(&gMenuHeader);
        }
    }

}

// Utility functions

#ifdef _PC
void MenuSaveReplay(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    FILE *fp;

    fp = fopen("Replay.rpl", "wb");
    if (fp == NULL) {

        SetMenuMessage("Could not open file");

    } else {

        if (SaveReplayData(fp)) {
            SetMenuMessage("Saved in \"Replay.rpl\"");
        } else {
            SetMenuMessage("Could not save replay data");
        }
        fclose(fp);
    }

    MenuGoForward(menuHeader, menu, menuItem);
}
#endif


void SetContinueChamp(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    GameLoopQuit = GAMELOOP_QUIT_CHAMP_CONTINUE;
    SetFadeEffect(FADE_DOWN);

    ReplayMode = FALSE;
    RPL_RecordReplay = TRUE;

    RPL_InitReplayBuffer();

    MenuGoForward(menuHeader, menu, menuItem);
}

void SetEndChamp(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    GameLoopQuit = GAMELOOP_QUIT_FRONTEND;
    SetFadeEffect(FADE_DOWN);

    ReplayMode = FALSE;
    RPL_RecordReplay = TRUE;
    RPL_InitReplayBuffer();
    
    MenuGoForward(menuHeader, menu, menuItem);
}

void SetGiveUpChampTry(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
//  Players[0].RaceFinishPos = 0xffffffff;
//  ChampionshipEndMode = CHAMPIONSHIP_END_GAVEUP;
    MenuGoForward(menuHeader, menu, menuItem);
}

void AskRestartRace(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    SetConfirmMenuStrings(TEXT_TABLE(TEXT_MENU_CONFIRM_RESTART), TEXT_TABLE(TEXT_YES), TEXT_TABLE(TEXT_NO), CONFIRM_NO);
    gConfirmType = 1;
    MenuGoForward(menuHeader, menu, menuItem);
}
    
void SetRestartRace(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    GameLoopQuit = GAMELOOP_QUIT_RESTART;
    SetFadeEffect(FADE_DOWN);
    MenuGoForward(menuHeader, menu, menuItem);
}

void SetRestartReplay(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    Assert(ReplayMode);
    GameLoopQuit = GAMELOOP_QUIT_RESTART_REPLAY;
    MenuGoForward(menuHeader, menu, menuItem);

    SetFadeEffect(FADE_DOWN);
}

void SetViewReplay(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{

    SetFadeEffect(FADE_DOWN);

    GameLoopQuit = GAMELOOP_QUIT_REPLAY;
    MenuGoForward(menuHeader, menu, menuItem);
}

void AskQuitRace(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    SetConfirmMenuStrings(TEXT_TABLE(TEXT_MENU_CONFIRM_QUIT), TEXT_TABLE(TEXT_YES), TEXT_TABLE(TEXT_NO), CONFIRM_NO);
    gConfirmType = 0;
    MenuGoForward(menuHeader, menu, menuItem);
}
    
void SetQuitRace(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    //ClearMenuHeader(menuHeader);
    SetNextMenu(menuHeader, NULL);

    GameLoopQuit = GAMELOOP_QUIT_FRONTEND;
    g_bShowWinLoseSequence = FALSE;
    ChampionshipEndMode = CHAMPIONSHIP_END_FINISHED;

    SetFadeEffect(FADE_DOWN);
}

#ifdef _PC
void SelectPreviousTrack(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    long level;
    LEVELINFO *li;

// get current track

    level = GetLevelNum(StartData.LevelDir);
    if (level == -1) level = LEVEL_NEIGHBOURHOOD1;

// get prev track

    do {
        level--;
        if (level < 0) level =  LEVEL_NSHIPPED_LEVELS - 1;
        li = GetLevelInfo(level);
    } while (!IsLevelSelectable(level) || !IsLevelAvailable(level) || !DoesLevelExist(level));

// save name in start data

    strncpy(StartData.LevelDir, li->Dir, MAX_LEVEL_DIR_NAME);
}

void SelectNextTrack(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    long level;
    LEVELINFO *li;

// get current track

    level = GetLevelNum(StartData.LevelDir);
    if (level == -1) level = LEVEL_NEIGHBOURHOOD1;

// get next track

    do {
        level++;
        if (level > LEVEL_NSHIPPED_LEVELS - 1) level = 0;
        li = GetLevelInfo(level);
    } while (!IsLevelSelectable(level) || !IsLevelAvailable(level) || !DoesLevelExist(level));

// save name in start data

    strncpy(StartData.LevelDir, li->Dir, MAX_LEVEL_DIR_NAME);
}
#endif



////////////////////////////////////////////////////////////////
//
// In Game Options Menu
//
////////////////////////////////////////////////////////////////

// Menu
MENU Menu_InGameOptions = {
    TEXT_OPTIONS,
    MENU_IMAGE_NONE,                        // Menu title bar image enum
    TITLESCREEN_CAMPOS_START,                        // Camera pos index
    MENU_DEFAULT | MENU_CENTRE_X | MENU_CENTRE_Y,   // Menu type
    SPRU_COL_RANDOM,                        // Spru colour
    CreateInGameOptionsMenu,                // Create menu function
    MENU_INGAME_XPOS,                       // X coord
    MENU_INGAME_YPOS,                       // Y Coord
    0,                                      // Current Item index
    NULL,                                   // Parent Menu (OverWritten in Create)
};

// In Game Options - Video settings
MENU_ITEM MenuItem_InGameGraphics = {
    TEXT_RENDERSETTINGS,                        // Text label index

    0,                                      // Space needed to draw item data
    &Menu_InGameGraphicsOptions,            // Data
    NULL,                                   // Draw Function

	NULL,								    // Input handler
	0,                                      // Flags

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    NULL,                                   // Left Action
    NULL,                                   // Right Action
    MenuGoBack,                             // Back Action
    MenuGoForward,                          // Forward Action
};



// Create
void CreateInGameOptionsMenu(MENU_HEADER *menuHeader, MENU *menu)
{
    AddMenuItem(menuHeader, menu, &MenuItem_InGameGraphics);
    AddMenuItem(menuHeader, menu, &MenuItem_AudioSettings);
    AddMenuItem(menuHeader, menu, &MenuItem_ControllerSettings);

    // Switch off "Go-Forward" icon
    menuHeader->NavFlags &= ~MENU_FLAG_ADVANCE;

}




////////////////////////////////////////////////////////////////
//
// Draw Next Track
//
////////////////////////////////////////////////////////////////
#ifdef _PC
void DrawNextTrack(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex)
{
    REAL xPos, yPos;
    LEVELINFO *li;

    li = GetLevelInfo(GetLevelNum(StartData.LevelDir));
    if (!li)
    {
        return;
    }

    xPos = menuHeader->XPos + menuHeader->ItemTextWidth + MENU_TEXT_GAP;
    yPos = menuHeader->YPos + (MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP) * itemIndex;

    DrawMenuText(xPos,yPos, MENU_TEXT_RGB_NORMAL, li->Name);
}
#endif

