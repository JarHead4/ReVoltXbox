//-----------------------------------------------------------------------------
// File: ui_SinglePlayerGame.cpp
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"
#include "settings.h"
#include "ui_Menu.h"
#include "ui_TitleScreen.h"
#include "ui_StateEngine.h"
#include "ui_SinglePlayerGame.h"
#include "ui_RaceDifficulty.h"
#include "ui_EnterName.h"
#include "ui_SelectCar.h"
#include "ui_SelectTrack.h"
#include "ui_RaceOverview.h"





CSinglePlayerGameStateEngine g_SinglePlayerGameStateEngine;


//-----------------------------------------------------------------------------
// Name: Process()
// Desc: Main control function for the state engine
//-----------------------------------------------------------------------------
HRESULT CSinglePlayerGameStateEngine::Process()
{
    enum
    {
        SINGLEPLAYERGAME_STATE_BEGIN = STATEENGINE_STATE_BEGIN,
        SINGLEPLAYERGAME_STATE_BACKOUT,
        SINGLEPLAYERGAME_STATE_SELECTDIFFICULTY,
        SINGLEPLAYERGAME_STATE_POSTSELECTDIFFICULTY,
        SINGLEPLAYERGAME_STATE_ENTERNAME,
        SINGLEPLAYERGAME_STATE_POSTENTERNAME,
        SINGLEPLAYERGAME_STATE_SELECTCAR,
        SINGLEPLAYERGAME_STATE_POSTSELECTCAR,
        SINGLEPLAYERGAME_STATE_SELECTTRACK,
        SINGLEPLAYERGAME_STATE_POSTSELECTTRACK,
        SINGLEPLAYERGAME_STATE_RACEOVERVIEW,
        SINGLEPLAYERGAME_STATE_POSTRACEOVERVIEW,
        SINGLEPLAYERGAME_STATE_STARTGAME,
    };

    switch( m_State )
    {
        case SINGLEPLAYERGAME_STATE_BACKOUT:
            Return( STATEENGINE_TERMINATED );
            break;

        case SINGLEPLAYERGAME_STATE_BEGIN:
            // Initialise variables
            GameSettings.GameType    = GAMETYPE_SINGLE;
            GameSettings.RandomCars  = gTitleScreenVars.RandomCars;
            GameSettings.RandomTrack = gTitleScreenVars.RandomTrack;
            gTitleScreenVars.iCurrentPlayer  = 0;
            gTitleScreenVars.numberOfPlayers = 1;

            g_pMenuHeader->SetNextMenu( NULL );
            g_pTitleScreenCamera->SetNewPos( TITLESCREEN_CAMPOS_RACE );
            // Fall through to next state

        case SINGLEPLAYERGAME_STATE_SELECTDIFFICULTY:
            // Call the SelectDifficulty state engine
            g_pActiveStateEngine->Call( &g_SelectDifficultyStateEngine );
            m_State = SINGLEPLAYERGAME_STATE_POSTSELECTDIFFICULTY;
            break;

        case SINGLEPLAYERGAME_STATE_POSTSELECTDIFFICULTY:
            // If user backed out, go back a few states
            if( g_SelectDifficultyStateEngine.GetStatus() == STATEENGINE_TERMINATED )
                return GotoState( SINGLEPLAYERGAME_STATE_BACKOUT );
            // Else, fall through to next state

        case SINGLEPLAYERGAME_STATE_ENTERNAME:
            // Call the EnterName state engine
            g_pActiveStateEngine->Call( &g_EnterNameStateEngine );
            m_State = SINGLEPLAYERGAME_STATE_POSTENTERNAME;
            break;

        case SINGLEPLAYERGAME_STATE_POSTENTERNAME:
            // If user backed out, go back a few states
            if( g_EnterNameStateEngine.GetStatus() == STATEENGINE_TERMINATED )
                return GotoState( SINGLEPLAYERGAME_STATE_SELECTDIFFICULTY );
            // Else, fall through to next state

        case SINGLEPLAYERGAME_STATE_SELECTCAR:
            // Call the SelectCar state engine
            g_pActiveStateEngine->Call( &g_SelectCarStateEngine );
            m_State = SINGLEPLAYERGAME_STATE_POSTSELECTCAR;
            break;

        case SINGLEPLAYERGAME_STATE_POSTSELECTCAR:
            // If user backed out, go back a few states
            if( g_SelectCarStateEngine.GetStatus() == STATEENGINE_TERMINATED )
                return GotoState( SINGLEPLAYERGAME_STATE_ENTERNAME );
            // Else, fall through to next state

        case SINGLEPLAYERGAME_STATE_SELECTTRACK:
            // Call the SelectTrack state engine
            g_pActiveStateEngine->Call( &g_SelectTrackStateEngine );
            m_State = SINGLEPLAYERGAME_STATE_POSTSELECTTRACK;
            break;

        case SINGLEPLAYERGAME_STATE_POSTSELECTTRACK:
            // If user backed out, go back a few states
            if( g_SelectTrackStateEngine.GetStatus() == STATEENGINE_TERMINATED )
            {
                if( GameSettings.RandomCars )
                    return GotoState( SINGLEPLAYERGAME_STATE_ENTERNAME );
                else
                    return GotoState( SINGLEPLAYERGAME_STATE_SELECTCAR );
            }
            // Else, fall through to next state

        case SINGLEPLAYERGAME_STATE_RACEOVERVIEW:
            // Call the SelectTrack state engine
            g_pActiveStateEngine->Call( &g_RaceOverviewStateEngine );
            m_State = SINGLEPLAYERGAME_STATE_POSTRACEOVERVIEW;
            break;

        case SINGLEPLAYERGAME_STATE_POSTRACEOVERVIEW:
            // If user backed out, go back a few states
            if( g_RaceOverviewStateEngine.GetStatus() == STATEENGINE_TERMINATED )
            {
                if( GameSettings.RandomTrack )
                {
                    if( GameSettings.RandomCars )
                        return GotoState( SINGLEPLAYERGAME_STATE_ENTERNAME );
                    else
                        return GotoState( SINGLEPLAYERGAME_STATE_SELECTCAR );
                }
                else
                    return GotoState( SINGLEPLAYERGAME_STATE_SELECTTRACK );
            }
            // Else, fall through to next state

        case SINGLEPLAYERGAME_STATE_STARTGAME:
            // Start the game!
            g_bTitleScreenRunGame = TRUE;
            //Return( STATEENGINE_COMPLETED );
            g_pMenuHeader->SetNextMenu( NULL );

            // no state exists during race.
            //g_pActiveStateEngine->HandleExitToChild();
            g_pActiveStateEngine = NULL;
            //$REVISIT: not sure we want to be touching g_pActiveStateEngine directly (isn't done anywhere else).
            //$REVISIT: we're bypassing the HandleEnter**/HandleExit** functions here, which seems bad.
            break;
    }

    // Handle the menus
    g_pMenuHeader->HandleMenus();

    return S_FALSE;
}



