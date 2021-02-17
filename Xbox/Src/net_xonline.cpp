//-----------------------------------------------------------------------------
// File: net_xonline.cpp
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "net_xonline.h"
#include "network.h"
#include "LevelLoad.h"
#include "player.h"
#include "VoiceManager.h"
#include "ui_MenuText.h" // For access to localized text strings
#include "ui_ShowMessage.h" // for displaying XOnline errors to player
#include "ui_TitleScreen.h"
#ifdef ENABLE_STATISTICS
//$REVISIT: Statistics disabled for July Consumer Beta
#include "net_Statistics.h"
#endif // ENABLE_STATISTICS

void OnSessionEntered();  //$UGLY: reaching into network.cpp ...


//$REVISIT: make sure we're handling all XOnline return values correctly
// (ie, displaying user messages for potential return values and handling
// them correctly, asserting on unexpected return values, etc.)
// Not just in this file, but in other files as well (eg, XBOnline.cpp) !!


//
// Type definitions
//
struct OnlineTask
{
    XONLINETASK_HANDLE handle;
    OnlineTaskType     type;
};


//
// Global variables
//
static OnlineTask OnlineTaskList[MAX_PENDING_XONLINE_TASKS];
static int        OnlineTaskCount = 0;

//$TODO: revisit this. We just need a way to distinguish between
// "No matches found" and "Still searching"
BOOL g_bXOnlineSessionSearchComplete = FALSE;

//$REVISIT: Once we support multiple people, this shouldn't be needed
// This will be set to the controller that was signed in
DWORD g_dwSignedInController;


//
// Matchmaking attribute data
//

// Attributes used during session-create
XONLINE_ATTRIBUTE g_rgAttribs_MatchCreate[NUM_XATTRIB_MATCHCREATE] =
{
    { XATTRIB_GAME_MODE,     FALSE, 0 },
    { XATTRIB_TRACK_ID,      FALSE, 0 },
    { XATTRIB_HOST_USERNAME, FALSE, 0 },
    { XATTRIB_HOST_NICKNAME, FALSE, 0 },
};

// Attributes used during session-search
XONLINE_ATTRIBUTE g_rgAttribs_MatchSearch[NUM_XATTRIB_MATCHSEARCH] =
{
    { XATTRIB_GAME_MODE, FALSE, 0 },
    { XATTRIB_TRACK_ID,  FALSE, 0 },
};

// Attribute struct and SPECS for session search results
#include <PshPack1.h>  // struct receiving parsed search results must be 1-byte aligned
struct MatchSearchResultParsed
{
    ULONGLONG  qwGameMode;
    ULONGLONG  qwTrackID;
    WCHAR      wstrHostUserName[XONLINE_GAMERTAG_SIZE];
    WCHAR      wstrHostNickName[MAX_NICKNAME];
};
#include <PopPack.h>

XONLINE_ATTRIBUTE_SPEC g_rgAttribs_MatchResult[NUM_XATTRIB_MATCHRESULT] =
  //$NOTE: this array should be const, but XOnline argument lists don't seem to use 'const' yet
{
    { X_ATTRIBUTE_DATATYPE_INTEGER, 0 },                     // INTEGER: game mode (0 or 1)
    { X_ATTRIBUTE_DATATYPE_INTEGER, 0 },                     // INTEGER: track ID
    { X_ATTRIBUTE_DATATYPE_STRING,  sizeof(WCHAR)*XONLINE_GAMERTAG_SIZE }, // STRING: host's XOnline username
    { X_ATTRIBUTE_DATATYPE_STRING,  sizeof(WCHAR)*MAX_NICKNAME },          // STRING: host's nickname in game
};



//
// Functions
//

//-----------------------------------------------------------------------------
// Name: IsLoggedIn
// Desc: Returns TRUE if the player is logged in
//-----------------------------------------------------------------------------
BOOL IsLoggedIn( DWORD dwUserIndex )
{
    //$SINGLEPLAYER: Assuming single player at Players[0]
    assert( Players[0].XOnlineInfo.pXOnlineUser != NULL ||
            !Players[0].XOnlineInfo.bIsLoggedIn );
    return Players[0].XOnlineInfo.bIsLoggedIn;
}

//-----------------------------------------------------------------------------
// Name: AddOnlinePresenceFlag
// Desc: Adds in an online presence flag (ie, "has voice", etc.)
//-----------------------------------------------------------------------------
void AddOnlinePresenceFlag( DWORD dwController, DWORD dwFlag )
{
    //$SINGLEPLAYER: Assuming single player at Players[0]
    Players[0].dwOnlineState |= dwFlag;
    if( IsLoggedIn( 0 ) )
    {
        XOnlineNotificationSetState( dwController, 
                                     Players[0].dwOnlineState, 
                                     SessionCurr.keyID,
                                     0,
                                     NULL );
    }
}

//-----------------------------------------------------------------------------
// Name: RemoveOnlinePresenceFlag
// Desc: Removes an online presence flag (ie, "has voice", etc.)
//-----------------------------------------------------------------------------
void RemoveOnlinePresenceFlag( DWORD dwController, DWORD dwFlag )
{
    //$SINGLEPLAYER: Assuming single player at Players[0]
    Players[0].dwOnlineState &= ~dwFlag;
    if( IsLoggedIn( 0 ) )
    {
        XOnlineNotificationSetState( dwController, 
                                     Players[0].dwOnlineState, 
                                     SessionCurr.keyID,
                                     0,
                                     NULL );
    }
}

//-----------------------------------------------------------------------------
// Name: OnlineTasks_Startup
// Desc: 
//-----------------------------------------------------------------------------
  //$REVISIT(cprince): do we really need this function?
void OnlineTasks_Startup( void )
{
    assert( 0 == OnlineTaskCount );

    // Nothing to do here right now
}


//-----------------------------------------------------------------------------
// Name: OnlineTasks_Cleanup
// Desc: 
//-----------------------------------------------------------------------------
  //$REVISIT(cprince): do we really need this function?
void OnlineTasks_Cleanup( void )
{
    //$REVISIT(cprince): is it possible to cause problems by closing
    /// handle of certain in-progress tasks?

    // Close all pending tasks and remove all from the list
    for( int i=0 ; i < OnlineTaskCount ; i++ )
    {
        XOnlineTaskClose( OnlineTaskList[i].handle );
    }
    OnlineTaskCount = 0;
}


//-----------------------------------------------------------------------------
// Name: OnlineTasks_Add
// Desc: 
//-----------------------------------------------------------------------------
void OnlineTasks_Add( XONLINETASK_HANDLE handle, OnlineTaskType type )
{
    assert( (OnlineTaskCount+1) < MAX_PENDING_XONLINE_TASKS );

    OnlineTaskList[OnlineTaskCount].handle = handle;
    OnlineTaskList[OnlineTaskCount].type   = type;
    OnlineTaskCount++;
}


//-----------------------------------------------------------------------------
// Name: OnlineTasks_Remove
// Desc: 
//-----------------------------------------------------------------------------
void OnlineTasks_Remove( int index )
{
    assert( index < OnlineTaskCount  &&  index >= 0 );
    
    // swap removed entry with last one in list
    OnlineTaskCount--;
    OnlineTaskList[index] = OnlineTaskList[OnlineTaskCount];
}


//-----------------------------------------------------------------------------
// Name: OnlineTasks_RemoveByValue
// Desc: 
//-----------------------------------------------------------------------------
void OnlineTasks_RemoveByValue( XONLINETASK_HANDLE handle )
{
    for( int i = 0; i < OnlineTaskCount; i++ )
    {
        if( OnlineTaskList[i].handle == handle )
        {
            OnlineTasks_Remove( i );
            return;
        }
    }

    assert( 0 && "Couldn't find handle" );
}



//-----------------------------------------------------------------------------
// Name: OnlineTasks_Continue
// Desc: 
//-----------------------------------------------------------------------------
void OnlineTasks_Continue( void )
{
    int i;
    HRESULT hr;

#ifdef ENABLE_STATISTICS
//$REVISIT: Statistics disabled for July Consumer Beta
    // $HACK: merge CXOnlineTask class with this code and features.
    g_XOnlineTasks.TaskContinueAll();
#endif // ENABLE_STATISTICS

    // Pump each pending task in the list
    for( i=0 ; i < OnlineTaskCount ; i++ )
    {
        hr = XOnlineTaskContinue( OnlineTaskList[i].handle );

        // If the task has not completed...
        if( hr != XONLINETASK_S_SUCCESS )
        { 
            // Print message for unexpected return values
            if( hr != XONLINETASK_S_RUNNING
                && hr != XONLINETASK_S_RESULTS_AVAIL  //$REVISIT: should we remove this line?  Seems safer not to remove it.
                && hr != XONLINETASK_S_RUNNING_IDLE 
              )
            {
                swprintf( g_SimpleMessageBuffer,
                          L"TaskContinue (type=%d)\n"
                          L"returned HR=%08x\n"
                          L"\n"
                          L"Please write this down and report it.\n"
                          L"Also, mention whether you saw any problems\n"
                          L"soon after this message appeared.\n"
                          , OnlineTaskList[i].type, hr
                        );
                g_ShowSimpleMessage.Begin( L"XON Warning!",
                                           g_SimpleMessageBuffer,
                                           NULL, TEXT_TABLE(TEXT_BUTTON_A_CONTINUE) );
            }

            continue; 
        }
        //$REVISIT: partial results ONLY for enumeration APIs ?
        //$REVISIT: no task-specific return codes for matchmaking tasks (but then what tasks do have special return values?)


        // Task has completed, so take appropriate action
        switch( OnlineTaskList[i].type )
        {
            case TASK_MATCH_CREATE:
            {
                // Get info about newly created session
                hr = XOnlineMatchSessionGetInfo(
                         OnlineTaskList[i].handle, // task handle for session-create
                         &(SessionCurr.keyID),     // receives session XNKID
                         &(SessionCurr.key)        // receives session XNKEY
                     );
                if( hr != S_OK )
                {
                    swprintf( g_SimpleMessageBuffer,
                              L"MatchSessionGetInfo\n"
                              L"returned HR=%08x\n"
                              L"\n"
                              L"Please write this down and report it.\n"
                              L"Also, mention whether you saw any problems\n"
                              L"soon after this message appeared.\n"
                              , hr
                            );
                    g_ShowSimpleMessage.Begin( L"XON Warning!",
                                               g_SimpleMessageBuffer,
                                               NULL, TEXT_TABLE(TEXT_BUTTON_A_CONTINUE) );
                }

                // Register session keys
                XNetRegisterKey( &(SessionCurr.keyID), &(SessionCurr.key) );

                // When we know our session keyID, start listening for QoS probes from clients
                XNetQosListen( &(SessionCurr.keyID), // (ptr to) session XNKID
                               NULL, // (used with _SET_DATA) ptr to game data block
                               0,    // (used with _SET_DATA) num bytes in game data block
                               0,    // (used with _SET_BITSPERSEC) bits/sec value
                               XNET_QOS_LISTEN_ENABLE // subset of the XNET_QOS_LISTEN_ flags
                             );

                // Add ourselves to the global MachineList
                assert( 0 == MachineCount );
                MachineList[0].Socket = INVALID_SOCKET;
                MachineList[0].XnAddr = XnAddrLocal;
                MachineList[0].SoAddr.sin_family = AF_INET;
                MachineList[0].SoAddr.sin_port   = htons(GAME_PORT);
                XNetXnAddrToInAddr( &(XnAddrLocal),
                                    &(SessionCurr.keyID),
                                    &(MachineList[0].SoAddr.sin_addr) );
                MachineCount = 1;
                OnSessionEntered();
            }
            break;

            case TASK_MATCH_SEARCH:
            {
                // Get session search results
                PXONLINE_MATCH_SEARCHRESULT * rgResultPtrs = NULL;
                DWORD dwNumResults;
                hr = XOnlineMatchSearchGetResults(
                         OnlineTaskList[i].handle, // task handle for session-search
                         &rgResultPtrs, // receives addr of array of pointers to search results
                         &dwNumResults // receives num search results
                     );

                if( hr != S_OK )
                {
                    swprintf( g_SimpleMessageBuffer,
                              L"MatchSearchGetResults\n"
                              L"returned HR=%08x\n"
                              L"\n"
                              L"Please write this down and report it.\n"
                              L"Also, mention whether you saw any problems\n"
                              L"soon after this message appeared.\n"
                              , hr
                            );
                    g_ShowSimpleMessage.Begin( L"XON Warning!",
                                               g_SimpleMessageBuffer,
                                               NULL, TEXT_TABLE(TEXT_BUTTON_A_CONTINUE) );
                }

                //$TODO: revisit this. We just need a way to distinguish between
                // "No matches found" and "Still searching"
                g_bXOnlineSessionSearchComplete = TRUE;

                // We now have the full set of search results for our query,
                // so clear the old session list
                ClearSessionList();

                // Get session attributes from each search result
                assert( dwNumResults <= MAX_SESSION_SEARCH_RESULTS );
                unsigned int iResult;
                for( iResult = 0 ; iResult < dwNumResults ; iResult++ )
                {
                    MatchSearchResultParsed resultParsed = {0};
                    hr = XOnlineMatchSearchParse(
                             rgResultPtrs[iResult],   // ptr to a search result
                             rgResultPtrs[iResult]->dwNumAttributes, // num session attribute specs (for parsing search result)
                             g_rgAttribs_MatchResult, // array of session attribute specs (for parsing search result)
                             &resultParsed            // ptr to buffer receiving session attributes from search result
                         );
                    if( hr != S_OK )
                    {
                        swprintf( g_SimpleMessageBuffer,
                                  L"MatchSearchParse\n"
                                  L"returned HR=%08x\n"
                                  L"\n"
                                  L"Please write this down and report it.\n"
                                  L"Also, mention whether you saw any problems\n"
                                  L"soon after this message appeared.\n"
                                  , hr
                                );
                        g_ShowSimpleMessage.Begin( L"XON Warning!",
                                                   g_SimpleMessageBuffer,
                                                   NULL, TEXT_TABLE(TEXT_BUTTON_A_CONTINUE) );
                    }

                    // use results to fill NET_SESSION struct, and insert into SessionList
                    NET_SESSION session;
                    memcpy( &session.keyID, &rgResultPtrs[iResult]->SessionID, sizeof(XNKID) );
                    memcpy( &session.key, &rgResultPtrs[iResult]->KeyExchangeKey, sizeof(XNKEY) );
                    memcpy( &session.XnAddrServer, &rgResultPtrs[iResult]->HostAddress, sizeof(XNADDR) );
                    session.pQosResults = NULL;
                    swprintf( session.wstrHostNickname, L"%s", resultParsed.wstrHostUserName );  //$HACK: B/C NOT YET IMPLEMENTED (need to include *NICKNAME* in attribs for MatchCreate/MatchSearch/MatchResult)
                    swprintf( session.wstrHostUsername, L"%s", resultParsed.wstrHostUserName );
                    session.Started = 0;    //$TODO: NOT YET IMPLEMENTED
                    if( resultParsed.qwGameMode == MATCH_GAMEMODE_RACE ) {
                        session.GameType = GAMETYPE_NETWORK_RACE;
                    } else {
                        assert( resultParsed.qwGameMode == MATCH_GAMEMODE_BATTLETAG
                                || g_bInvitedByFriend //$BUG: Apr02_GameBash hack to avoid assert
                              );
                        session.GameType = GAMETYPE_NETWORK_BATTLETAG;
                    }
                    session.Version = MultiplayerVersion;
                    session.RandomCars = 0; //$TODO: NOT YET IMPLEMENTED (b/c not included in attribs for MatchCreate/MatchSearch/MatchResults)
                    session.RandomTrack = 0; //$TODO: NOT YET IMPLEMENTED (b/c not included in attribs for MatchCreate/MatchSearch/MatchResults)

                    session.nCurrPlayersPublic = rgResultPtrs[iResult]->dwPublicFilled;
                    session.nMaxPlayersPublic  = rgResultPtrs[iResult]->dwPublicFilled
                                                 + rgResultPtrs[iResult]->dwPublicOpen;
                    session.nCurrPlayersPrivate = rgResultPtrs[iResult]->dwPrivateFilled;
                    session.nMaxPlayersPrivate  = rgResultPtrs[iResult]->dwPrivateFilled
                                                  + rgResultPtrs[iResult]->dwPrivateOpen;
                    session.iLevelNum = (long)resultParsed.qwTrackID;

                    InsertSessionListEntry( &session );
                }


                // Send QoS probes to each host in the list
                for( int s = 0 ; s < GetSessionCount() ; s++ )
                {
                    const int MAX_QOS_BITS_PER_SEC = (24 * 1024); // for QoS probing; doesn't include QoS replies
                    assert( NULL == SessionList[s].pQosResults );
                    const XNADDR* apXnAddr[] = { &(SessionList[s].XnAddrServer) };
                    const XNKID*  apKeyID[]  = { &(SessionList[s].keyID) };
                    const XNKEY*  apKey[]    = { &(SessionList[s].key) };
                    XNetQosLookup( 1,        // num Xbox to probe
                                   apXnAddr, // array of ptrs to Xbox XNADDRs
                                   apKeyID,  // array of ptrs to Xbox XNKIDs
                                   apKey,    // array of ptrs to Xbox XNKEYs
                                   0,        // num Security Gateway (SG) to probe
                                   NULL,     // array of SG IN_ADDRs
                                   NULL,     // array of SG service IDs
                                   8,        // num probes to each destination (0 to only check connectivity)
                                   MAX_QOS_BITS_PER_SEC/GetSessionCount(), // max outbound bits/sec to use for sending QoS probes (0 to use default)
                                   0,        // flags
                                   NULL,     // [optional] event to signal when each probe completes
                                   &(SessionList[s].pQosResults) // ptr that will be set to point at results struct
                                 );
                    //$REVISIT: is it okay that we call XNetQosLookup for each session (instead of one big array)?
                    /// Did it this way b/c otherwise, hard to keep pQosResults associated with session info, because SessionList can get re-arranged.
                }

            }
            break;
            
            case TASK_MUTELIST_ENUMERATE:
                if( g_VoiceManager.IsInChatSession() )
                {
                    //$REVISIT: If the enumeration doesn't complete until after
                    // we've already gotten into a game, we may need to walk
                    // the mute list and mute anyone in the game
                }
                break;

            case TASK_STAT_RESET_USER:
            case TASK_STAT_RESET_ALL_USERS:
            case TASK_MATCH_UPDATE:
            case TASK_MATCH_DELETE:
            default:
            ; // No new work to be done; just close the task handle
        }
        
        // Cleanup the task
        XOnlineTaskClose( OnlineTaskList[i].handle );
        OnlineTasks_Remove( i );
        i--;  //$HACK: OnlineTasks_Remove will swap removed entry with the last one in list.  So if we don't do this, we'll skip over an online task (until next time function is called).
              /// Another option would be to have a 'bUsed' flag and set that to FALSE for removal.  Then we wouldn't need this hack, but we'd have to iterate through the entire task list every time (even empty tasks), which means minor perf issues.  (Plus it's less elegant.)
    }

}

