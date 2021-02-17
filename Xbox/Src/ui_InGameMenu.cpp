//-----------------------------------------------------------------------------
// File: ui_InGameMenu.cpp
//
// Desc: 
//
// Re-Volt (Generic) Copyright (c) Probe Entertainment 1998
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"
#include "main.h"
#include "geom.h"
#include "particle.h"
#include "model.h"
#include "aerial.h"
#include "newcoll.h"
#include "body.h"
#include "car.h"
#include "ctrlread.h"
#include "object.h"
#include "light.h"
#include "obj_init.h"
#include "player.h"
#include "ai.h"
#include "ai_init.h"
#include "EditObject.h"
#include "drawobj.h"
#include "move.h"
#include "timing.h"
#include "visibox.h"
#include "spark.h"
#include "field.h"
#include "weapon.h"
#include "input.h"
#include "initplay.h"
#include "pickup.h"
#include "panel.h"
#include "SoundEffectEngine.h"
#include "MusicManager.h"
#include "GameLoop.h"
#include "ReadInit.h"
#include "Settings.h"
#include "Text.h"
#include "net_xonline.h"

#include "ui_Menu.h"
#include "ui_TitleScreen.h"
#include "ui_StateEngine.h"
#include "ui_StateEngine.h"
#include "ui_menudraw.h"
#include "ui_MenuText.h"
#include "ui_Confirm.h"     // gConfirmMenuReturnVal
#include "ui_ConfirmGiveUp.h"
#include "ui_InGameMenu.h"
#include "ui_Options.h"
#include "ui_ShowMessage.h"
#include "ui_Players.h"
#include "ui_Friends.h"

#ifdef ENABLE_STATISTICS
//$REVISIT: Statistics disabled for July Consumer Beta
#include "net_Statistics.h"
#endif // ENABLE_STATISTICS

#define MENU_INGAME_XPOS                100
#define MENU_INGAME_YPOS                150

static void CreateInGameMenu(MENU_HEADER *menuHeader, MENU *menu);
static BOOL HandleInGameMenu( MENU_HEADER* pMenuHeader, DWORD dwInput );

static void CreateInGameOptionsMenu(MENU_HEADER *menuHeader, MENU *menu);

static BOOL SelectPreviousTrack(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
static BOOL SelectNextTrack(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
static void MenuSaveReplay(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
static void MenuSaveLocalPlayerCarInfo(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);

static void DrawNextTrack(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex);


extern MENU Menu_InGameOptions;

CInGameMenuStateEngine g_InGameMenuStateEngine;
void DrawTextWithReceivedInviteIcon( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem, int itemIndex );
extern MENU_ITEM MenuItem_FriendsWithInviteIcon;



////////////////////////////////////////////////////////////////
//
// In Game Menu
//
////////////////////////////////////////////////////////////////
extern MENU Menu_InGame = 
{
    TEXT_NONE,
    MENU_DEFAULT | MENU_CENTRE_X | MENU_CENTRE_Y,   // Menu type
    CreateInGameMenu,                       // Create menu function
    HandleInGameMenu,                       // Input handler function
    NULL,                                   // Menu draw function
    MENU_INGAME_XPOS,                       // X coord
    MENU_INGAME_YPOS,                       // Y Coord
};

static MENU_ITEM MenuItem_StartNewRace = 
{
    TEXT_NEWRACE,                    // Text label index
    MENU_TEXT_WIDTH * 18,                   // Space needed to draw item data
    NULL,                                   // Data
    DrawNextTrack,                          // Draw Function
};

static MENU_ITEM MenuItem_StartNextRace = 
{
    TEXT_STARTNEXTRACE,                        // Text label index
    MENU_TEXT_WIDTH * 18,                   // Space needed to draw item data
    NULL,                                   // Data
    DrawNextTrack,                          // Draw Function
};


// Create
void CreateInGameMenu(MENU_HEADER *menuHeader, MENU *menu)
{
    // Calc the menu item data width needed for displaying track names
    FLOAT fMaxLevelNamePixelWidth = 0;
    for( long level=0; level < GameSettings.LevelNum; level++ )
    {
        if( IsLevelSelectable(level) && IsLevelAvailable(level) && DoesLevelExist(level) )
        {
            FLOAT fLevelNamePixelWidth = g_pFont->GetTextWidth( GetLevelInfo(level)->strName );
            if( fLevelNamePixelWidth > fMaxLevelNamePixelWidth )
                fMaxLevelNamePixelWidth = fLevelNamePixelWidth;
        }
    }
    static WCHAR strRightArrow[3] = { 0x0020, 0x2192, 0x0000 };
    fMaxLevelNamePixelWidth += g_pFont->GetTextWidth( strRightArrow );
    MenuItem_StartNewRace.DataWidth  = fMaxLevelNamePixelWidth;
    MenuItem_StartNextRace.DataWidth = fMaxLevelNamePixelWidth;


    menu->CurrentItemIndex = 0;

    BOOL bLocalPlayerHasFinishedRace = (PLR_LocalPlayer->RaceFinishTime && (ChampionshipEndMode == CHAMPIONSHIP_END_FINISHED || ChampionshipEndMode == CHAMPIONSHIP_END_WAITING_FOR_FINISH || ChampionshipEndMode == CHAMPIONSHIP_END_QUALIFIED || ChampionshipEndMode == CHAMPIONSHIP_END_FAILED));


//$MODIFIED - mwetzel - taking out pointless "resume" option
//  // Resume?
//  if ((ChampionshipEndMode != CHAMPIONSHIP_END_MENU) &&
//      (!ReachedEndOfReplay)) 
//  {
//          menuHeader->AddMenuItem( TEXT_RESUME_RACE );
//  }
//$END_MODIFICATIONS

    // continue / end championship
    if (ChampionshipEndMode == CHAMPIONSHIP_END_MENU || (GameSettings.GameType == GAMETYPE_REPLAY && StartDataStorage.GameType == GAMETYPE_CHAMPIONSHIP))
    {
        if (CupTable.QualifyFlag == CUP_QUALIFY_YES || CupTable.QualifyFlag == CUP_QUALIFY_TRIESLEFT)
        {
            // PT: Set the label to continue or end champion ship, depending if we are finished
            menuHeader->AddMenuItem( TEXT_CONTINUE_CHAMP );
        }
        else
            menuHeader->AddMenuItem( TEXT_END_CHAMP );
    }

    // give up championship try?
    if (GameSettings.GameType == GAMETYPE_CHAMPIONSHIP && ChampionshipEndMode == CHAMPIONSHIP_END_WAITING_FOR_FINISH && !Players[0].RaceFinishTime)
        menuHeader->AddMenuItem( TEXT_GIVEUP_CHAMP );

//$MODIFIED - mwetzel - taking out "restart race" option
//  // Restart?
//  //---------------------------
//  if ((GameSettings.GameType != GAMETYPE_CHAMPIONSHIP) &&         // Not in championship
//      (GameSettings.GameType != GAMETYPE_REPLAY || StartDataStorage.GameType != GAMETYPE_CHAMPIONSHIP) && // Not in championship replay
//      (GameSettings.GameType != GAMETYPE_PRACTICE) &&             // not in practice
//      (GameSettings.GameType != GAMETYPE_TRAINING) &&             // not in training
//      (!IsMultiPlayer() || (IsServer() && !ReplayMode)) &&        // Not in multiplayer unless server
//      !(ReplayMode && IsMultiPlayer())  // Not if replaying and was a multiplayer game
//      )
//  {
//      menuHeader->AddMenuItem( TEXT_RESTART_RACE );
//  }
//$END_MODIFICATIONS

    // next track
    if( IsMultiPlayer() && IsServer() && !GameSettings.RandomTrack && !ReplayMode )
    {
        // TODO: Handle case for "if( GameSettings.RandomTrack == TRUE )"

        if( bLocalPlayerHasFinishedRace )
            menuHeader->AddMenuItem( &MenuItem_StartNextRace );
        else
            menuHeader->AddMenuItem( &MenuItem_StartNewRace );
    }

    if( !IsMultiPlayer() && !ReplayMode )
    {
        if( bLocalPlayerHasFinishedRace )
            menuHeader->AddMenuItem( &MenuItem_StartNextRace );
        else
            menuHeader->AddMenuItem( &MenuItem_StartNewRace );
    }

    // Players menu only available in multiplayer games
    if( IsMultiPlayer() )
    {
        menuHeader->AddMenuItem( TEXT_PLAYERS );
    }

    // Friends only available if the player is logged in
    if( IsLoggedIn(0) )
    {
        menuHeader->AddMenuItem( &MenuItem_FriendsWithInviteIcon );
    }

    // Replay
    if (RPL_RecordReplay && !ReplayMode && !IsMultiPlayer()) 
    {
        if (PLR_LocalPlayer->RaceFinishTime != 0) 
        {
            menuHeader->AddMenuItem( TEXT_VIEW_REPLAY );
        }
    } 
    else if (ReplayMode) 
    {
        menuHeader->AddMenuItem( TEXT_RESTART_REPLAY );
    }

    // In-game best times
    if (GameSettings.GameType == GAMETYPE_TRIAL)
    {
        menuHeader->AddMenuItem( TEXT_BESTTRIALTIMES );
    }
    // In-game options
    menuHeader->AddMenuItem( TEXT_OPTIONS );

#ifdef SHIPPING
    // Don't allow save options for shipping versions.
#else
    // Save Replay for Game Gauge (in Dev mode)
    if (Version == VERSION_DEV) 
    {
        menuHeader->AddMenuItem( TEXT_SAVE_REPLAY );
//$REMOVED        menuHeader->AddMenuItem( TEXT_SAVECURRENTCARINFO );
    }
#endif // SHIPPING

    // Quit
    menuHeader->AddMenuItem( TEXT_QUIT_RACE );

    menuHeader->AddMenuItem( TEXT_TABLE(TEXT_BUTTON_A_SELECT_B_BACK), MENU_ITEM_ACTIVE );
}




HRESULT CInGameMenuStateEngine::Process()
{
    switch( m_State )
    {
        case INGAMEMENU_STATE_BEGIN:
            GameSettings.Paws = TRUE;

#ifdef OLD_AUDIO
            PauseAllSfx();
#else
            g_SoundEngine.PauseAll();
            g_MusicManager.Pause();
#endif // OLD_AUDIO
//$REVISIT - might actually want this, depending on how we implement vibration
//$REMOVED
//                        SetSafeAllJoyForces();
//$END_REMOVAL

            if (GameSettings.GameType == GAMETYPE_CHAMPIONSHIP && PLR_LocalPlayer->RaceFinishTime && !AllPlayersFinished)
            {
                ForceAllCarFinish();
            }
            else
            {
                g_pMenuHeader->ClearMenuHeader();
                g_pMenuHeader->SetNextMenu( &Menu_InGame);
            }

            m_State = INGAMEMENU_STATE_MAINLOOP;
            return S_OK;

        case INGAMEMENU_STATE_MAINLOOP:
            // Nothing to do, as control is in the menus
            break;
        
        case INGAMEMENU_STATE_POSTCONFIRMRESTART:
            // If user choose not to quite, return to mainloop
            if( g_ShowSimpleMessage.GetStatus() == STATEENGINE_TERMINATED )
                return GotoState( INGAMEMENU_STATE_MAINLOOP );
            // Else, fall through to next state

        case INGAMEMENU_STATE_RESTARTRACE:
            GameLoopQuit = GAMELOOP_QUIT_RESTART;
            SetFadeEffect(FADE_DOWN);
            Return();
            break;

        case INGAMEMENU_STATE_POSTCONFIRMQUIT:
            // If user choose not to quit, return to mainloop
            if( g_ShowSimpleMessage.GetStatus() == STATEENGINE_TERMINATED )
                return GotoState( INGAMEMENU_STATE_MAINLOOP );
            // Else, fall through to next state

        case INGAMEMENU_STATE_QUITRACE:
            //g_pMenuHeader->ClearMenuHeader();
            g_pMenuHeader->SetNextMenu(NULL);

//$ADDITION(cprince)
#ifdef ENABLE_STATISTICS
//$REVISIT: Statistics disabled for July Consumer Beta
            // Do work specific to XOnline networking
            if( gTitleScreenVars.bUseXOnline )
            {
                // Updated Number of Started Races/Battles Statistics
                StatsLocalPlayersExitingMatch();
            }
#endif // ENABLE_STATISTICS

            if( IsServer() ) { DestroySession(); }
            if( IsClient() )
            {
                //$REVISIT(cprince): Seems like this check shouldn't happen here.
                // I added it to avoid a net-init ref-count bug.  But
                // as soon as we detect server has dropped connection, then we
                // should call LeaveSession(), display a message to the user,
                // and go back to the FrontEnd UI, never even allowing the user
                // to access this in-game menu.  When that code has been
                // implemented, IsInGameSession() should always return true
                // if we reach this point.
                if( IsInGameSession() )
                {
                    LeaveSession();
                }
            }
            //$TODO: should verify (for client AND server) that this is the only way to manually end session.
            /// If there are other ways to end session, need to insert similar code at those locations.
//$END_ADDITION

            GameLoopQuit = GAMELOOP_QUIT_FRONTEND;
            g_bShowWinLoseSequence = FALSE;
            ChampionshipEndMode = CHAMPIONSHIP_END_FINISHED;

            SetFadeEffect(FADE_DOWN);
            Return();
            break;

        case INGAMEMENU_STATE_EXIT:
            g_pMenuHeader->SetNextMenu(NULL);
            BOOL bLocalPlayerHasFinishedRace = (PLR_LocalPlayer->RaceFinishTime && (ChampionshipEndMode == CHAMPIONSHIP_END_FINISHED || ChampionshipEndMode == CHAMPIONSHIP_END_WAITING_FOR_FINISH || ChampionshipEndMode == CHAMPIONSHIP_END_QUALIFIED || ChampionshipEndMode == CHAMPIONSHIP_END_FAILED));
            if( bLocalPlayerHasFinishedRace )
                SetConsoleMessage( TEXT_TABLE(TEXT_PRESS_START_TO_CONTINUE), NULL, MENU_COLOR_ORANGE, MENU_COLOR_WHITE, 10, CONSOLE_MESSAGE_FOREVER);
            GameSettings.Paws = FALSE;
#ifdef OLD_AUDIO
            ResumeAllSfx();
#else
            g_SoundEngine.ResumeAll();
            if( RegistrySettings.MusicOn )
                g_MusicManager.Play();
#endif // OLD_AUDIO

            Return();
            break;
    }

    // Handle the menus
    g_pMenuHeader->HandleMenus();

    return S_FALSE;
}


BOOL HandleInGameMenu( MENU_HEADER* pMenuHeader, DWORD dwInput )
{
    switch( pMenuHeader->m_pCurrentItem->TextIndex )
    {
//$MODIFIED - mwetzel - taking out pointless "resume" option
//      case TEXT_RESUME_RACE:
//          if( dwInput == MENU_INPUT_SELECT )  
//          { 
//              //MenuGoForward( pMenuHeader, pMenuHeader->m_pMenu, pMenuHeader->m_pCurrentItem ); 
//              g_InGameMenuStateEngine.NextState( CInGameMenuStateEngine::INGAMEMENU_STATE_EXIT);
//              return TRUE; 
//          }
//          break;
//$END_MODIFICATIONS

        case TEXT_CONTINUE_CHAMP:
            if( dwInput == MENU_INPUT_SELECT )  
            { 
                //SetContinueChamp( pMenuHeader, pMenuHeader->m_pMenu, pMenuHeader->m_pCurrentItem ); 
                GameLoopQuit = GAMELOOP_QUIT_CHAMP_CONTINUE;
                SetFadeEffect(FADE_DOWN);

                ReplayMode = FALSE;
                RPL_RecordReplay = TRUE;

                RPL_InitReplayBuffer();

                //MenuGoForward(menuHeader, menu, menuItem);                
                pMenuHeader->SetNextMenu( NULL);
                return TRUE; 
            }
            break;

        case TEXT_END_CHAMP:
            if( dwInput == MENU_INPUT_SELECT )  
            { 
                //SetEndChamp( pMenuHeader, pMenuHeader->m_pMenu, pMenuHeader->m_pCurrentItem ); 
                GameLoopQuit = GAMELOOP_QUIT_FRONTEND;
                SetFadeEffect(FADE_DOWN);

                ReplayMode = FALSE;
                RPL_RecordReplay = TRUE;
                RPL_InitReplayBuffer();
    
                //MenuGoForward(menuHeader, menu, menuItem);                
                pMenuHeader->SetNextMenu( NULL);
                return TRUE; 
            }
            break;

        case TEXT_GIVEUP_CHAMP:
            if( dwInput == MENU_INPUT_SELECT )  
            { 
                //SetGiveUpChampTry( pMenuHeader, pMenuHeader->m_pMenu, pMenuHeader->m_pCurrentItem ); 
                pMenuHeader->SetNextMenu( &Menu_ConfirmGiveup);
                return TRUE; 
            }
            break;

//$MODIFIED - mwetzel - taking out "restart race" option
//      case TEXT_RESTART_RACE:
//          if( dwInput == MENU_INPUT_SELECT )  
//          { 
//              //AskRestartRace( pMenuHeader, pMenuHeader->m_pMenu, pMenuHeader->m_pCurrentItem ); 
//              g_ShowSimpleMessage.Begin( -1, TEXT_MENU_CONFIRM_RESTART, TEXT_YES, TEXT_BUTTON_B_NO);
//              g_InGameMenuStateEngine.NextState( CInGameMenuStateEngine::INGAMEMENU_STATE_POSTCONFIRMRESTART);
//              return TRUE; 
//          }
//          break;
//$END_MODIFICATIONS

        case TEXT_RESTART_REPLAY:
            if( dwInput == MENU_INPUT_SELECT )  
            { 
                //SetRestartReplay( pMenuHeader, pMenuHeader->m_pMenu, pMenuHeader->m_pCurrentItem ); 
                Assert(ReplayMode);
                GameLoopQuit = GAMELOOP_QUIT_RESTART_REPLAY;

                //MenuGoForward(menuHeader, menu, menuItem);                
                pMenuHeader->SetNextMenu( NULL);

                SetFadeEffect(FADE_DOWN);
                return TRUE; 
            }
            break;

        case TEXT_VIEW_REPLAY:
            if( dwInput == MENU_INPUT_SELECT )  
            { 
                //SetViewReplay( pMenuHeader, pMenuHeader->m_pMenu, pMenuHeader->m_pCurrentItem ); 
                SetFadeEffect(FADE_DOWN);

                GameLoopQuit = GAMELOOP_QUIT_REPLAY;
                //MenuGoForward(menuHeader, menu, menuItem);                
                pMenuHeader->SetNextMenu( NULL);
                return TRUE; 
            }
            break;

        case TEXT_OPTIONS:
            if( dwInput == MENU_INPUT_SELECT )  
            { 
                //MenuGoForward( pMenuHeader, pMenuHeader->m_pMenu, pMenuHeader->m_pCurrentItem ); 
                pMenuHeader->SetNextMenu( &Menu_InGameOptions );
                return TRUE; 
            }
            break;

        case TEXT_QUIT_RACE:
            if( dwInput == MENU_INPUT_SELECT )  
            { 
                //AskQuitRace( pMenuHeader, pMenuHeader->m_pMenu, pMenuHeader->m_pCurrentItem ); 
                g_ShowSimpleMessage.Begin( NULL, TEXT_TABLE(TEXT_MENU_CONFIRM_QUIT), 
                                           TEXT_TABLE(TEXT_BUTTON_A_YES), TEXT_TABLE(TEXT_BUTTON_B_NO) );
                g_InGameMenuStateEngine.NextState( CInGameMenuStateEngine::INGAMEMENU_STATE_POSTCONFIRMQUIT);
                return TRUE; 
            }
            break;

        case TEXT_SAVE_REPLAY:
            if( dwInput == MENU_INPUT_SELECT )  
            { 
                MenuSaveReplay( pMenuHeader, pMenuHeader->m_pMenu, pMenuHeader->m_pCurrentItem ); 
                return TRUE; 
            }
            break;

        case TEXT_NEWRACE:
            switch( dwInput )
            {
                case MENU_INPUT_LEFT:  
                    return SelectPreviousTrack( pMenuHeader, pMenuHeader->m_pMenu, pMenuHeader->m_pCurrentItem ); 
                case MENU_INPUT_RIGHT:
                    return SelectNextTrack( pMenuHeader, pMenuHeader->m_pMenu, pMenuHeader->m_pCurrentItem );
                case MENU_INPUT_SELECT: 
                    //AskRestartRace( pMenuHeader, pMenuHeader->m_pMenu, pMenuHeader->m_pCurrentItem ); 
                    g_ShowSimpleMessage.Begin( NULL, TEXT_TABLE(TEXT_MENU_CONFIRM_RESTART), 
                                               TEXT_TABLE(TEXT_BUTTON_A_YES), TEXT_TABLE(TEXT_BUTTON_B_NO) );
                    g_InGameMenuStateEngine.NextState( CInGameMenuStateEngine::INGAMEMENU_STATE_POSTCONFIRMRESTART);
                    return TRUE; 
            }
            break;


        case TEXT_STARTNEXTRACE:
            switch( dwInput )
            {
                case MENU_INPUT_LEFT:  
                    return SelectPreviousTrack( pMenuHeader, pMenuHeader->m_pMenu, pMenuHeader->m_pCurrentItem ); 
                case MENU_INPUT_RIGHT:
                    return SelectNextTrack( pMenuHeader, pMenuHeader->m_pMenu, pMenuHeader->m_pCurrentItem );
                case MENU_INPUT_SELECT: 
                    //AskRestartRace( pMenuHeader, pMenuHeader->m_pMenu, pMenuHeader->m_pCurrentItem ); 
                    g_ShowSimpleMessage.Begin( NULL, TEXT_TABLE(TEXT_MENU_CONFIRM_RESTART), 
                                               TEXT_TABLE(TEXT_BUTTON_A_YES), TEXT_TABLE(TEXT_BUTTON_B_NO) );
                    g_InGameMenuStateEngine.NextState( CInGameMenuStateEngine::INGAMEMENU_STATE_POSTCONFIRMRESTART);
                    return TRUE; 
            }
            break;

//$REMOVED
//      case TEXT_SAVECURRENTCARINFO:
//            if( dwInput == MENU_INPUT_SELECT )  
//            { 
//                MenuSaveLocalPlayerCarInfo( pMenuHeader, pMenuHeader->m_pMenu, pMenuHeader->m_pCurrentItem ); 
//                return TRUE; 
//            }
//            break;

        case TEXT_PLAYERS:
            if( dwInput == MENU_INPUT_SELECT ) 
            {
                g_pActiveStateEngine->Call( &g_PlayersStateEngine );
                return TRUE;
            }
            break;

        case TEXT_FRIENDS:
            if( dwInput == MENU_INPUT_SELECT )
            {
                g_pActiveStateEngine->Call( &g_FriendsStateEngine );
                return TRUE;
            }
            break;
    }

    switch( dwInput )
    {
        case MENU_INPUT_UP:
            return SelectPreviousMenuItem( pMenuHeader, pMenuHeader->m_pMenu, pMenuHeader->m_pCurrentItem );

        case MENU_INPUT_DOWN:
            return SelectNextMenuItem( pMenuHeader, pMenuHeader->m_pMenu, pMenuHeader->m_pCurrentItem );

        case MENU_INPUT_BACK:
            //g_InGameMenuStateEngine.Return( STATEENGINE_TERMINATED );
            g_InGameMenuStateEngine.NextState( CInGameMenuStateEngine::INGAMEMENU_STATE_EXIT);
            return TRUE;

        case MENU_INPUT_SELECT:
            break;
    }

    return FALSE;
}



// Utility functions
//$NOTE(cprince): it seems this function is only reachable in VERSION_DEV.
void MenuSaveLocalPlayerCarInfo(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    int iChar, sLen;
    FILE *fp;
    VEC dR;
    char filename[MAX_CAR_FILENAME];
    char dirname[MAX_CAR_FILENAME];
    int iCar = PLR_LocalPlayer->car.CarType;

    // Undo CoM changes
    CopyVec(&CarInfo[iCar].CoMOffset, &dR);
    NegateVec(&dR);
    MoveCarCoM(&CarInfo[iCar], &dR);

    // get the filename for this car's carinfo (assumes car has a MODEL 0)
//$MODIFIED: changed "5" to "8" (to account for "cars\..." vs "D:\cars\...")
//    sLen = strlen(CarInfo[iCar].ModelFile[0]);
//    if (sLen <= 5) {
//        DumpMessage("Warning", "Could not get car directory");
//        return;                 // doesn't even have "cars\" at the start
//    }
//
//    iChar = 5;
//    while ((iChar < sLen) && (iChar < MAX_CAR_FILENAME - 1) && (CarInfo[iCar].ModelFile[0][iChar] != '\\')) {
//        dirname[iChar - 5] = CarInfo[iCar].ModelFile[0][iChar];
//        iChar++;
//    }
//    dirname[iChar - 5] = '\0';
    sLen = strlen(CarInfo[iCar].ModelFile[0]);
    if (sLen <= 8) {
        DumpMessage("Warning", "Could not get car directory");
        return;                 // doesn't even have "D:\cars\" at the start
    }

    iChar = 8;
    while ((iChar < sLen) && (iChar < MAX_CAR_FILENAME - 1) && (CarInfo[iCar].ModelFile[0][iChar] != '\\')) {
        dirname[iChar - 8] = CarInfo[iCar].ModelFile[0][iChar];
        iChar++;
    }
    dirname[iChar - 8] = '\0';
//$END_MODIFICATIONS

//$MODIFIED
//    sprintf(filename, "Cars\\%s\\Parameters.txt", dirname);
    sprintf(filename, "D:\\Cars\\%s\\Parameters.txt", dirname);
//$END_MODIFICATIONS

    // open the file
    fp = fopen(filename, "w");
    if (fp == NULL) {
        DumpMessage("Could not open file", filename);
        MoveCarCoM(&CarInfo[iCar], &CarInfo[iCar].CoMOffset);
        return;
    }

    // Write the info file
    if (!WriteOneCarInfo(&CarInfo[iCar], fp)) {
        DumpMessage("Error saving", filename);
        MoveCarCoM(&CarInfo[iCar], &CarInfo[iCar].CoMOffset);
        fclose(fp);
        return;
    }
    
    // Redo CoM changes
    MoveCarCoM(&CarInfo[iCar], &CarInfo[iCar].CoMOffset);

    fclose(fp);
}



void MenuSaveReplay(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    FILE *fp;

    fp = fopen("Replay.rpl", "wb");
    if (fp == NULL) {

        SetMenuMessage(TEXT_TABLE(TEXT_COULD_NOT_OPEN_FILE));

    } else {

        if (SaveReplayData(fp)) {
            SetMenuMessage(TEXT_TABLE(TEXT_SAVED_IN_REPLAY_FILE));
        } else {
            SetMenuMessage(TEXT_TABLE(TEXT_COULD_NOT_SAVE_REPLAY));
        }
        fclose(fp);
    }

    MenuGoForward(menuHeader, menu, menuItem);
}


BOOL SelectPreviousTrack(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
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
    strncpy(StartData.LevelDir, li->szDir, MAX_PATH);
    return TRUE;
}


BOOL SelectNextTrack(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
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
    strncpy(StartData.LevelDir, li->szDir, MAX_PATH);
    return TRUE;
}




////////////////////////////////////////////////////////////////
//
// In Game Options Menu
//
////////////////////////////////////////////////////////////////
static VOID CreateInGameSettingsMenu( MENU_HEADER* pMenuHeader, MENU* pMenu );
static BOOL HandleInGameSettingsMenu( MENU_HEADER* pMenuHeader, DWORD dwInput );

extern MENU Menu_InGameOptions = 
{
    TEXT_OPTIONS,
    MENU_DEFAULT | MENU_CENTRE_X | MENU_CENTRE_Y,   // Menu type
    CreateInGameOptionsMenu,                // Create menu function
    NULL,                                   // Input handler function
    NULL,                                   // Menu draw function
    MENU_INGAME_XPOS,                       // X coord
    MENU_INGAME_YPOS,                       // Y Coord
};

extern MENU Menu_InGameSettings = 
{
    TEXT_GAMESETTINGS,
    MENU_DEFAULT | MENU_CENTRE_X,           // Menu type
    CreateInGameSettingsMenu,               // Create menu function
    HandleInGameSettingsMenu,               // Input handler function
    NULL,                                   // Menu draw function
    100,                                    // X coord
    150,                                    // Y Coord
};

MENU_ITEM MenuItem_InGameGameSettings = 
{
    TEXT_GAMESETTINGS,                      // Text label index

    0,                                      // Space needed to draw item data
    &Menu_InGameSettings,                     // Data
    NULL,                                   // Draw Function

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    NULL,                                   // Left Action
    NULL,                                   // Right Action
    MenuGoBack,                             // Back Action
    MenuGoForward,                          // Forward Action
};

MENU_ITEM MenuItem_InGameAudioSettings = 
{
    TEXT_AUDIOSETTINGS,                     // Text label index

    0,                                      // Space needed to draw item data
    &Menu_AudioSettings,                    // Data
    NULL,                                   // Draw Function

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    NULL,                                   // Left Action
    NULL,                                   // Right Action
    MenuGoBack,                             // Back Action
    MenuGoForward,                          // Forward Action
};

MENU_ITEM MenuItem_InGameControllerSettings = 
{
    TEXT_CONTROLLERSETTINGS,                // Text label index

    0,                                      // Space needed to draw item data
    &Menu_ControllerSettings,               // Data
    NULL,                                   // Draw Function
    
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
    menuHeader->AddMenuItem( &MenuItem_InGameGameSettings );
    menuHeader->AddMenuItem( &MenuItem_InGameAudioSettings );
    menuHeader->AddMenuItem( &MenuItem_InGameControllerSettings, MENU_ITEM_INACTIVE );
}

extern MENU_ITEM MenuItem_RandomCars;
extern MENU_ITEM MenuItem_RandomTrack;
extern MENU_ITEM MenuItem_Pickups;
extern MENU_ITEM MenuItem_SpeedUnits;
extern MENU_ITEM MenuItem_NumberOfCars;
extern MENU_ITEM MenuItem_NumberOfLaps;

// Create Game settings menu
void CreateInGameSettingsMenu(MENU_HEADER *pMenuHeader, MENU *pMenu)
{
    // Calc max data width for the menuitems
    MenuItem_RandomCars.DataWidth   = CalcMaxStringWidth( 2, &TEXT_TABLE(TEXT_ON) ) + g_pFont->GetTextWidth( L" >" );;
    MenuItem_RandomTrack.DataWidth  = CalcMaxStringWidth( 2, &TEXT_TABLE(TEXT_ON) ) + g_pFont->GetTextWidth( L" >" );;
    MenuItem_Pickups.DataWidth      = CalcMaxStringWidth( 2, &TEXT_TABLE(TEXT_ON) ) + g_pFont->GetTextWidth( L" >" );;
    MenuItem_SpeedUnits.DataWidth   = CalcMaxStringWidth( SPEED_NTYPES, &TEXT_TABLE(TEXT_MPH) ) + g_pFont->GetTextWidth( L" >" );;

    if( PLR_LocalPlayer->RaceFinishTime )
    {
        // Race is finished, so allow options for the next race
        if( !IsMultiPlayer() )
        {
            // Single player, between-game options
            pMenuHeader->AddMenuItem( &MenuItem_NumberOfCars, MENU_ITEM_INACTIVE );
            pMenuHeader->AddMenuItem( &MenuItem_NumberOfLaps );
            pMenuHeader->AddMenuItem( &MenuItem_Pickups );
            pMenuHeader->AddMenuItem( &MenuItem_SpeedUnits );
        }
        else if( IsServer() )
        {
            // Multiplayer host, between-game options
            pMenuHeader->AddMenuItem( &MenuItem_NumberOfCars, MENU_ITEM_INACTIVE );
            pMenuHeader->AddMenuItem( &MenuItem_NumberOfLaps );
            pMenuHeader->AddMenuItem( &MenuItem_Pickups );
            pMenuHeader->AddMenuItem( &MenuItem_SpeedUnits );
        }
        else
        {
            // Multiplayer client, between-game options
            pMenuHeader->AddMenuItem( &MenuItem_NumberOfCars, MENU_ITEM_INACTIVE );
            pMenuHeader->AddMenuItem( &MenuItem_NumberOfLaps, MENU_ITEM_INACTIVE );
            pMenuHeader->AddMenuItem( &MenuItem_Pickups, MENU_ITEM_INACTIVE );
            pMenuHeader->AddMenuItem( &MenuItem_SpeedUnits );
        }
    }
    else
    {
        // Ingame options
        pMenuHeader->AddMenuItem( &MenuItem_NumberOfCars, MENU_ITEM_INACTIVE );
        pMenuHeader->AddMenuItem( &MenuItem_NumberOfLaps, MENU_ITEM_INACTIVE );
        pMenuHeader->AddMenuItem( &MenuItem_Pickups, MENU_ITEM_INACTIVE );
        pMenuHeader->AddMenuItem( &MenuItem_SpeedUnits );
    }
}

BOOL HandleInGameSettingsMenu( MENU_HEADER* pMenuHeader, DWORD dwInput )
{
    switch( pMenuHeader->m_pCurrentItem->TextIndex )
    {
        case TEXT_NUMBEROFCARS:
        case TEXT_NUMBEROFLAPS:
        case TEXT_UNITS:
            if( dwInput == MENU_INPUT_LEFT )  return DecreaseSliderDataLong( pMenuHeader, pMenuHeader->m_pMenu, pMenuHeader->m_pCurrentItem );
            if( dwInput == MENU_INPUT_RIGHT ) return IncreaseSliderDataLong( pMenuHeader, pMenuHeader->m_pMenu, pMenuHeader->m_pCurrentItem );
            break;

        case TEXT_RANDOM_CARS:
        case TEXT_RANDOM_TRACK:
        case TEXT_PICKUPS:
            if( dwInput == MENU_INPUT_LEFT )  return ToggleMenuDataOff( pMenuHeader, pMenuHeader->m_pMenu, pMenuHeader->m_pCurrentItem );
            if( dwInput == MENU_INPUT_RIGHT ) return ToggleMenuDataOn( pMenuHeader, pMenuHeader->m_pMenu, pMenuHeader->m_pCurrentItem );
            break;
    }
            
    switch( dwInput )
    {
        case MENU_INPUT_UP:
            return SelectPreviousMenuItem( pMenuHeader, pMenuHeader->m_pMenu, pMenuHeader->m_pCurrentItem );
        
        case MENU_INPUT_DOWN:
            return SelectNextMenuItem( pMenuHeader, pMenuHeader->m_pMenu, pMenuHeader->m_pCurrentItem );

        case MENU_INPUT_BACK:
            return MenuGoBack( pMenuHeader, pMenuHeader->m_pMenu, pMenuHeader->m_pCurrentItem );
    }

    return FALSE;
}




////////////////////////////////////////////////////////////////
//
// Draw Next Track
//
////////////////////////////////////////////////////////////////
void DrawNextTrack( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem, int itemIndex )
{
    LEVELINFO* pLevelInfo = GetLevelInfo( GetLevelNum( StartData.LevelDir ) );
    if( NULL == pLevelInfo )
        return;

    FLOAT xPos = pMenuHeader->m_XPos + pMenuHeader->m_ItemTextWidth + MENU_TEXT_GAP;
    FLOAT yPos = pMenuHeader->m_YPos + (MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP) * itemIndex;

    DrawMenuTextWithArrows( (pMenuItem == pMenuHeader->m_pCurrentItem) ? TRUE : FALSE,
                            xPos, yPos, MENU_TEXT_RGB_NORMAL, pLevelInfo->strName, 
                            pMenuHeader->m_ItemDataWidth );
}



