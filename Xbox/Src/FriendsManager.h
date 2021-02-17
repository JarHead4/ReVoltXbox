//-----------------------------------------------------------------------------
// File: FriendsManager.h
//
// Desc: Class and structure definitions for Friends Manager and related
//          objects
//
// Hist: 7.25.02 - Initial creation
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef _FRIENDSMANAGER_H_
#define _FRIENDSMANAGER_H_

#include <xtl.h>
#include <xonline.h>
#include <vector>

#if _DEBUG
static const DWORD DBGWARN_SIZE = 200;
#define FM_WARN(x) DbgWarn##x
#else
#define FM_WARN(x)
#endif // _DEBUG

//-----------------------------------------------------------------------------
// Name: class CFriendsManager
// Desc: Provides a layer of abstraction for managing friends, presence, etc.
//-----------------------------------------------------------------------------
class CFriendsManager
{
public:
    CFriendsManager();
    ~CFriendsManager();

    HRESULT Initialize();       // Initialize the Friends Manager object
    HRESULT Shutdown();         // Shut down the Friends Manager object
    HRESULT Process();          // Do any pending work - call every frame

    //-------------------------------------------------------------------------
    // Friends
    //-------------------------------------------------------------------------

    // Call StartUpdatingFriends anytime you want to have up-to-date
    // friends information for the user.  Call StopUpdatingFriends when
    // done.  You'll still have friends information even when not enumerating,
    // it just won't be up-to-date
    HRESULT         StartUpdatingFriends( DWORD dwUser );
    HRESULT         StopUpdatingFriends( DWORD dwUser );

    // Add/Remove can be called to add/remove a friend from the user's
    // friends list.  Answer is used to answer a friend request
    HRESULT         AddPlayerToFriendsList( DWORD dwUser, XUID xuidPlayer );
    HRESULT         RemoveFriendFromFriendsList( DWORD dwUser, XONLINE_FRIEND* pFriend );
    HRESULT         AnswerFriendRequest( DWORD dwUser, XONLINE_FRIEND* pFriend, XONLINE_REQUEST_ANSWER_TYPE answer );

    // Send/Revoke are used to send and revoke game invites to a friend.  NULL
    // can be used to send/revoke to ALL friends.  Answer is used to answer a
    // game invite
    HRESULT         SendGameInvite( DWORD dwUser, XNKID sessionID, XONLINE_FRIEND* pFriend );
    HRESULT         RevokeGameInvite( DWORD dwUser, XNKID sessionID, XONLINE_FRIEND* pFriend );
    HRESULT         AnswerGameInvite( DWORD dwUser, XONLINE_FRIEND* pFriend, XONLINE_GAMEINVITE_ANSWER_TYPE answer );

    // GetAcceptedGameInvite looks for an invitation that was accepted while
    // the player was in another title.  You should automatically log them on
    // and join the session
    HRESULT         GetAcceptedGameInvite( XONLINE_ACCEPTED_GAMEINVITE* pAcceptedGameInvite );

    // JoinCrossTitleGame can be used to initiate a join to a different title.
    // It will write out an accepted invite to the hard drive, and the title
    // should prompt the user to insert the proper disc
    HRESULT         JoinCrossTitleGame( DWORD dwUser, XONLINE_FRIEND* pFriend );

    // Attempt to find the player in the user's friends list
    XONLINE_FRIEND* FindPlayerInFriendsList( DWORD dwUser, XUID xuidPlayer );

    // Returns TRUE if the user's friends list has changed since the last time
    // it was called
    BOOL            HasFriendsListChanged( DWORD dwUser );

    // Used for walking the list of friends (for display, etc.)
    DWORD           GetNumFriends( DWORD dwUser );
    XONLINE_FRIEND* GetFriend( DWORD dwUser, DWORD dwIndex );

    // Gets the name of the title the friend is playing.  If not enumerating
    // friends, strTitlename will be set to an empty string.
    HRESULT         GetFriendTitleName( XONLINE_FRIEND* pFriend, WORD wLanguage, DWORD cTitleName, WCHAR* strTitlename );

    //-------------------------------------------------------------------------
    // Mute List
    //-------------------------------------------------------------------------

    // Add/Remove can be called to add/remove a player from the user's mute
    // list
    HRESULT AddPlayerToMuteList( DWORD dwUser, XUID xuidPlayer );
    HRESULT RemovePlayerFromMuteList( DWORD dwUser, XUID xuidPlayer );

    // Determines whether or not the player is in the user's mute list
    BOOL    IsPlayerInMuteList( DWORD dwUser, XUID xuidPlayer );

    //-------------------------------------------------------------------------
    // Feedback
    //-------------------------------------------------------------------------

    // Sends feedback on the player
    HRESULT SendFeedback( DWORD dwUser, 
                          XUID xuidPlayer, 
                          XONLINE_FEEDBACK_TYPE feedback,
                          WCHAR* strNickname );


protected:
    HRESULT PumpFeedback();
    HRESULT PumpFriends();
    HRESULT PumpMuteList();
    HRESULT UpdateMuteList( DWORD dwUser );

    // Friends enumeration state
    enum FRIEND_ENUMERATE_STATE
    {
        STATE_IDLE,
        STATE_ENUMERATING,
        STATE_CLOSING,
    };

    // Friend list struct
    struct FRIENDLIST
    {
        DWORD                   dwState;        // State of this friends list
        XONLINETASK_HANDLE      hEnumerate;     // Enumeration task
        BOOL                    bAutoStop;      // Automatic stop when done?
        BOOL                    bHasChanged;    // Changed since last check?
        DWORD                   dwNumFriends;   // Number of friends in list
        XONLINE_FRIEND*         pFriends;       // Array of XONLINE_FRIENDs
    };

    // Friends data:
    XONLINETASK_HANDLE          m_hFriendsStartup;
    FRIENDLIST                  m_aFriendsLists[ XONLINE_MAX_LOGON_USERS ];


    // Mute list struct
    struct MUTELIST
    {
        XONLINETASK_HANDLE      hEnumerate;     // Enumeration task
        DWORD                   dwNumUsers;     // Number of users in list
        XONLINE_MUTELISTUSER*   pUsers;         // Array of XONLINE_MUTELISTUSERs
        BOOL                    bIsStale;       // List is stale - need to call XOnlineMutelistGet
    };

    // MuteList data:
    XONLINETASK_HANDLE          m_hMuteListStartup;
    MUTELIST                    m_aMuteLists[ XONLINE_MAX_LOGON_USERS ];

    // Vector of tasks for processing feedback
    typedef std::vector<XONLINETASK_HANDLE> TASKLIST;

    // Feedback data:
    // Every time feedback is sent on a particular user, a new task is 
    // created which must be pumped to completion.
    TASKLIST                    m_ahFeedback;

#if _DEBUG
    // Debug-only function for validating internal state
    void DbgValidateState();

    void DbgWarn( CHAR* format, ... );
#endif // _DEBUG
};

extern CFriendsManager g_FriendsManager;

#endif // _FRIENDSMANAGER_H_