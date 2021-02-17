//-----------------------------------------------------------------------------
// File: net_Statistics.h
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------

#include "revolt.h"
#include "LevelInfo.h"
#include "car.h"
#include "player.h"
#include "ui_MenuText.h" // For access to localized text strings
#include "ui_TitleScreen.h"

#include "net_xonline.h"
#include "net_Statistics.h"

extern RatingConstants g_RatingConstants = 
{
    2.718f,  // Base;               // Base for Elo equaltion, e.g. e or 10
    1.0f,    // ScaleFactor;        // Sigmoid curve temperature value
    10000.0f,  // MaxDisplayedRating; // Max rating in Re-Volt UI (min is 0.0)
    4.0f,      // MinBonusLaps;       // Min race laps for MajorityLeadBonus to apply
    0.0f,      // LeadBonus;          // Bonus awarded for ever leading the race
    0.0f,      // MajorityLeadBonus;  // Bonus awarded for leading most of the race
    4.0f,    // MaxWeight;          // Weight is max amount rating can change
    1.0f,    // MinWeight;          // Max and min specify range of weights
    25.0f,     // RaceThreshold;      // Races completed; triggers weight change
    0.5f     // RatingThreshold;    // Player rating; triggers weight change

};

CStatRacerCache             g_RacerStats;
CStatBattlerCache           g_BattlerStats;
CStatUserTrackRanksCache    g_UserTrackRanks;
CXOnlineTasks               g_XOnlineTasks;

CStatAdjustMatchesStarted  g_StatAdjustMatchesStarted;
CStatUpdateEndgamePlayerStats   g_StatUpdateEndgamePlayerStats;
// $TODO: RBAILEYleaving code cleanup and reorg still pending

static bool g_bStatsRaceInProgress = false;

void ShowXStatsError( int nLineNumber, HRESULT hr )
{
    swprintf( g_SimpleMessageBuffer,
              L"Stats (Line %d)\n"
              L"returned HR=%08x\n"
              L"\n"
              L"Please write this down and report it.\n"
              L"Also, mention whether you saw any problems\n"
              L"soon after this message appeared.\n"
              , nLineNumber, hr
            );
    g_ShowSimpleMessage.Begin( L"XON Warning!",
                               g_SimpleMessageBuffer,
                               NULL, TEXT_TABLE(TEXT_BUTTON_A_CONTINUE) );
}


void StatsLocalPlayersAdjustMatchesStarted( int nOffset )
{

    g_StatAdjustMatchesStarted.Reset( LeaderBoardID( gTitleScreenVars.iLevelNum ), nOffset );

    // $SINGLEPLAYER gotta get this single player stuff worked out...
    //PLAYER *pPlayer;   
    //for ( pPlayer = PLR_PlayerHead ; pPlayer ; pPlayer = pPlayer->next)
    //{
    //    if ( PLAYER_LOCAL == pPlayer->type  && !pPlayer->XOnlineInfo.bIsGuestOfUser )
    //    {
    //         g_StatIncrementMatchesStarted.AddLocalPlayer( pPlayer->XOnlineInfo.pXOnlineUser->xuid );
    //
    //    }
    //}

    if( IsLoggedIn( 0 ) )
    {
        PLAYER* pPlayer = &Players[0]; //GetPlayerFromPlayerID( LocalPlayerID );
        g_StatAdjustMatchesStarted.AddLocalPlayer( pPlayer->XOnlineInfo.pXOnlineUser->xuid );
    }

    g_StatAdjustMatchesStarted.BeginStatUpdate();
    g_XOnlineTasks.push_back( &g_StatAdjustMatchesStarted );


}

// called when starting new race
// also called when restarting race
// Restart can occur:
//      1) from in game menu on host
//      2) from net signal from host
// current states when restart can happen
//      1) New race from main.cpp
//      2) New race from in-game menu, after race is completed.
//      3) Re-start race before race is completed.
void StatsLocalPlayersStartingMatch( )
{
    // Assert( !g_bStatsRaceInProgress);  restart may occur before race is finished

    // restart may occur before race is finished
    // if we are already are in race, no need to increment counter...
    if ( !g_bStatsRaceInProgress )
    {
        StatsLocalPlayersAdjustMatchesStarted( +1 );
        g_bStatsRaceInProgress = true;
    }
}

void StatsLocalPlayersExitingMatch( )
{
    if ( g_bStatsRaceInProgress && !IsInGameSession() && IsClient() )
    {
        // HOST disconnected, local player are NOT abandoning match
        StatsLocalPlayersAdjustMatchesStarted( -1 );
    }
    g_bStatsRaceInProgress = false;
}

void StatUpdateEndgamePlayerStats()
{
    Assert( g_bStatsRaceInProgress );
    g_bStatsRaceInProgress = false;

    g_StatUpdateEndgamePlayerStats.BeginStatUpdate();
    g_XOnlineTasks.push_back( &g_StatUpdateEndgamePlayerStats );
}

