//-----------------------------------------------------------------------------
// File: net_xonline.h
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef NET_XONLINE_H
#define NET_XONLINE_H

#include <xtl.h>
#include <xonline.h>


//
// Constants
//

#define MAX_PENDING_XONLINE_TASKS  16


//
// Type definitions
//

enum OnlineTaskType
{
    // matchmaking tasks
    TASK_MATCH_CREATE,
    TASK_MATCH_UPDATE,
    TASK_MATCH_DELETE,
    TASK_MATCH_SEARCH,          // Search or FindFromID
    TASK_FRIENDS_CORE,          // Core Friends task
    TASK_FRIENDS_ENUMERATE,     // Friends list enumeration task
    TASK_MUTELIST_CORE,         // Core mute list task
    TASK_MUTELIST_ENUMERATE,    // Mute list enumeration task
    TASK_STAT_RESET_USER,       
    TASK_STAT_RESET_ALL_USERS,
    TASK_FEEDBACK,              // Send Feedback task
};


//
// Functions
//

void OnlineTasks_Startup( void );
void OnlineTasks_Cleanup( void );
void OnlineTasks_Add( XONLINETASK_HANDLE handle, OnlineTaskType type );
void OnlineTasks_Remove( int index );
void OnlineTasks_RemoveByValue( XONLINETASK_HANDLE handle );
void OnlineTasks_Continue( void );

BOOL IsLoggedIn( DWORD dwUserIndex );
void AddOnlinePresenceFlag( DWORD dwController, DWORD dwFlag );
void RemoveOnlinePresenceFlag( DWORD dwController, DWORD dwFlag );

//
// Matchmaking attribute data
//

const DWORD MATCH_SEARCH_PROC_INDEX = 0x0000FFFF;  //$NOTE: temporary procedure index

// Attribute IDs:                   ID num   type                           scope
//                                  ------   ----------------------------   --------------------------------
const DWORD XATTRIB_GAME_MODE     = 0x0001 | X_ATTRIBUTE_DATATYPE_INTEGER | X_ATTRIBUTE_SCOPE_TITLE_SPECIFIC;
const DWORD XATTRIB_TRACK_ID      = 0x0002 | X_ATTRIBUTE_DATATYPE_INTEGER | X_ATTRIBUTE_SCOPE_TITLE_SPECIFIC;
const DWORD XATTRIB_HOST_USERNAME = 0x0003 | X_ATTRIBUTE_DATATYPE_STRING  | X_ATTRIBUTE_SCOPE_TITLE_SPECIFIC;
const DWORD XATTRIB_HOST_NICKNAME = 0x0004 | X_ATTRIBUTE_DATATYPE_STRING  | X_ATTRIBUTE_SCOPE_TITLE_SPECIFIC;

// Attributes used during session-create
#define NUM_XATTRIB_MATCHCREATE  4
extern XONLINE_ATTRIBUTE g_rgAttribs_MatchCreate[NUM_XATTRIB_MATCHCREATE];

// Attributes used during session-search
#define NUM_XATTRIB_MATCHSEARCH  2
extern XONLINE_ATTRIBUTE g_rgAttribs_MatchSearch[NUM_XATTRIB_MATCHSEARCH];

// SPECS for attributes contained in session search results
#define NUM_XATTRIB_MATCHRESULT  4
extern XONLINE_ATTRIBUTE_SPEC g_rgAttribs_MatchResult[NUM_XATTRIB_MATCHRESULT];
  //$NOTE: this array should be const, but XOnline argument lists don't seem to use 'const' yet

// Game modes
#define MATCH_GAMEMODE_RACE       0
#define MATCH_GAMEMODE_BATTLETAG  1
#define MATCH_GAMEMODE_ANY        -1  //$REVIST: no longer used?  (We use GAMEMODE_NONE instead; kind of weird, and inconsistent with track-search.)

// Track IDs
#define MATCH_TRACKID_ANY         -1




//$TODO: revisit this. We just need a way to distinguish between
// "No matches found" and "Still searching"
extern BOOL g_bXOnlineSessionSearchComplete;

//$REVISIT: Once we support multiple people, this shouldn't be needed
// This will be set to the controller that was signed in
extern DWORD g_dwSignedInController;




#endif // NET_XONLINE_H
