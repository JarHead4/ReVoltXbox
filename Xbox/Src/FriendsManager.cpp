//-----------------------------------------------------------------------------
// File: FriendsManager.cpp
//
// Desc: Implementation for Friends Manager and related objects
//
// Hist: 7.25.02 - Initial creation
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "FriendsManager.h"
#include <assert.h>


CFriendsManager g_FriendsManager;




//-----------------------------------------------------------------------------
// Name: CFriendsManager (ctor)
// Desc: Initializes member variables
//-----------------------------------------------------------------------------
CFriendsManager::CFriendsManager()
{
    // Initialize all friends tasks to NULL
    m_hFriendsStartup  = NULL;
    ZeroMemory( m_aFriendsLists, sizeof( m_aFriendsLists ) );

    // Initialize all mute list tasks to NULL
    m_hMuteListStartup = NULL;
    ZeroMemory( m_aMuteLists, sizeof( m_aMuteLists ) );

    // Feedback task vector is already empty
}




//-----------------------------------------------------------------------------
// Name: ~CFriendsManager (dtor)
// Desc: Performs any final cleanup necessary
//-----------------------------------------------------------------------------
CFriendsManager::~CFriendsManager()
{
    // Make sure everything is shut down
    Shutdown();
}




//-----------------------------------------------------------------------------
// Name: Initialize
// Desc: Initializes the Friends Manager object
// Returns: S_OK if everything was succesfully started
//          E_OUTOFMEMORY if there wasn't enough memory (for us or XOnline API)
//-----------------------------------------------------------------------------
HRESULT CFriendsManager::Initialize()
{
    HRESULT hr = S_OK;

    // Create the friends startup task
    hr = XOnlineFriendsStartup( NULL, &m_hFriendsStartup );
    if( FAILED( hr ) )
    {
        // The only error we expect from XOnlineFriendsStartup is E_OUTOFMEMORY
        assert( hr == E_OUTOFMEMORY );
        FM_WARN(( "Couldn't create friends startup task.  HR = %lx\n", hr ));
        goto Cleanup;
    }

    // Create the mutelist startup task
    hr = XOnlineMutelistStartup( NULL, &m_hMuteListStartup );
    if( FAILED( hr ) )
    {
        // The only error we expect from XOnlineMutelistStartp is E_OUTOFMEMORY
        assert( hr == E_OUTOFMEMORY );
        FM_WARN(( "Couldn't create mutelist startup task.  HR = %lx\n", hr ));
        goto Cleanup;
    }

    // Determine which users are currently logged on.  Note that you MUST 
    // have logged at least one user on before calling Initialize
    XONLINE_USER* pLoggedOnUsers = XOnlineGetLogonUsers();
    assert( pLoggedOnUsers );

    for( DWORD i = 0; i < XONLINE_MAX_LOGON_USERS; i++ )
    {
        // See if we actually have a non-guest user logged on in this slot
        if( pLoggedOnUsers[ i ].xuid.qwUserID != 0 &&
            !XOnlineIsUserGuest( pLoggedOnUsers[ i ].xuid.dwUserFlags ) )
        {
            // Allocate space for their friends list
            m_aFriendsLists[ i ].pFriends = new XONLINE_FRIEND[ MAX_FRIENDS ];
            if( NULL == m_aFriendsLists[ i ].pFriends )
            {
                FM_WARN(( "Couldn't allocate memory for friends list\n" ));
                hr = E_OUTOFMEMORY;
                goto Cleanup;
            }

            // Grab the initial friends list
            if( SUCCEEDED( XOnlineFriendsEnumerate( i, NULL, &m_aFriendsLists[ i ].hEnumerate ) ) )
            {
                m_aFriendsLists[ i ].dwState = STATE_ENUMERATING;

                // Mark the list so that we know to automatically stop 
                // enumeration as soon as we get results.  If the title
                // calls StartUpdatingFriends before we get results,
                // we'll clear the flag
                m_aFriendsLists[ i ].bAutoStop = TRUE;
            }
    
            // Allocate space for their mute list
            m_aMuteLists[ i ].pUsers = new XONLINE_MUTELISTUSER[ MAX_MUTELISTUSERS ];
            if( NULL == m_aMuteLists[ i ].pUsers )
            {
                FM_WARN(( "Couldn't allocate memory for mute list\n" ));
                hr = E_OUTOFMEMORY;
                goto Cleanup;

            }

            // Start the initial population of their mute list
            hr = UpdateMuteList( i );
            if( FAILED( hr ) )
            {
                // Only error we expect from UpdateMuteList is E_OUTOFMEMORY
                assert( hr == E_OUTOFMEMORY );
                goto Cleanup;
            }
        }
    }

Cleanup:
    if( FAILED( hr ) )
    {
        Shutdown();
    }

#if _DEBUG
    DbgValidateState();
#endif // _DEBUG

    assert( hr == S_OK ||
            hr == E_OUTOFMEMORY );

    return hr;
}




//-----------------------------------------------------------------------------
// Name: Shutdown
// Desc: Shuts down the Friends Manager object.  Note that since this must
//          synchronously pump tasks to completion, it could take a little 
//          while (10s of milliseconds).  In addition, since it frees all
//          memory, any cached friend pointers will be invalid.
// Returns: S_OK always
//-----------------------------------------------------------------------------
HRESULT CFriendsManager::Shutdown()
{
    //-------------------------------------------------------------------------
    // Friends
    //-------------------------------------------------------------------------

    // Pump friends tasks until all enumeration has completed
    for( DWORD i = 0; i < XONLINE_MAX_LOGON_USERS; i++ )
    {
        if( m_aFriendsLists[ i ].dwState != STATE_IDLE )
        {
            StopUpdatingFriends( i );

            // Pump the task until it's done - PumpFriends will automatically
            // close the enumeration handle once it's done, or if an error
            // occurs.
            while( m_aFriendsLists[ i ].dwState != STATE_IDLE )
                PumpFriends();
        }
        
        assert( m_aFriendsLists[ i ].dwState == STATE_IDLE &&
                m_aFriendsLists[ i ].hEnumerate == NULL );

        // Now that enumeration is done, it's safe to delete
        // the friends list
        if( m_aFriendsLists[ i ].pFriends  )
        {
            delete[] m_aFriendsLists[ i ].pFriends;
            m_aFriendsLists[ i ].pFriends = NULL;
        }
        m_aFriendsLists[ i ].dwNumFriends = 0;
        m_aFriendsLists[ i ].bHasChanged = FALSE;
    }

    // Now that friends enumeration is done, it's safe to shut down the friends
    // startup task
    if( m_hFriendsStartup )
    {
        // We have to pump the startup task until it returns 
        // XONLINETASK_S_RUNNING_IDLE to ensure it completed all its work
        for( ; ; )
        {
            HRESULT hr = XOnlineTaskContinue( m_hFriendsStartup );
            if( FAILED( hr ) || hr == XONLINETASK_S_RUNNING_IDLE )
                break;
        }

        // Now we can close the task 
        XOnlineTaskClose( m_hFriendsStartup );
        m_hFriendsStartup = NULL;
    }

    //-------------------------------------------------------------------------
    // Mute List
    //-------------------------------------------------------------------------

    // Close any open mute list enumeration tasks
    for( DWORD i = 0; i < XONLINE_MAX_LOGON_USERS; i++ )
    {
        if( m_aMuteLists[ i ].hEnumerate != NULL )
        {
            XOnlineTaskClose( m_aMuteLists[ i ].hEnumerate );
            m_aMuteLists[ i ].hEnumerate = NULL;
        }
        m_aMuteLists[ i ].bIsStale = FALSE;

        // Now that the must list enumeration task is done, it's safe
        // to delete the mute list memory
        if( m_aMuteLists[ i ].pUsers )
        {
            delete[] m_aMuteLists[ i ].pUsers;
            m_aMuteLists[ i ].pUsers = NULL;
        }
    }

    // Now that mutelist enumeration is done, it's safe to shut down the 
    // mutelist enumeration startup task
    if( m_hMuteListStartup )
    {
        // We have to pump the startup task until it returns 
        // XONLINETASK_S_RUNNING_IDLE to ensure it completed all its work
        for( ; ; )
        {
            HRESULT hr = XOnlineTaskContinue( m_hMuteListStartup );
            if( FAILED( hr ) || hr == XONLINETASK_S_RUNNING_IDLE )
                break;
        }

        // Now we can close the task 
        XOnlineTaskClose( m_hMuteListStartup );
        m_hMuteListStartup = NULL;
    }

    //-------------------------------------------------------------------------
    // Feedback
    //-------------------------------------------------------------------------

    // Pump all the feedback tasks to completion (or error)
    while( m_ahFeedback.size() != 0 )
    {
        PumpFeedback();
    }

#if _DEBUG
    DbgValidateState();
#endif // _DEBUG

    return S_OK;
}



//-----------------------------------------------------------------------------
// Name: Process
// Desc: Performs any work needed
// Returns: S_OK if everything was succesful
//          hr from pumping tasks otherwise (friends, mutelist, feedback)
//-----------------------------------------------------------------------------
HRESULT CFriendsManager::Process()
{
    HRESULT hrTotal = S_OK;

#if _DEBUG
    DbgValidateState();
#endif // _DEBUG

    // If we haven't been initialized yet, there's nothing to do!
    if( m_hFriendsStartup )
    {
        // If pumping a set of tasks fails, we'll continue to pump the
        // other sets, so that a failure in friends won't starve out feedback
        HRESULT hrFriends   = PumpFriends();
        HRESULT hrMuteList  = PumpMuteList();
        HRESULT hrFeedback  = PumpFeedback();

        if( FAILED( hrFriends ) )
            hrTotal = hrFriends;
        else if( FAILED( hrMuteList ) )
            hrTotal = hrMuteList;
        else if( FAILED( hrFeedback ) )
            hrTotal = hrFeedback;
    }

    return hrTotal;
}



//-----------------------------------------------------------------------------
// Name: StartUpdatingFriends( DWORD dwUser )
// Desc: Begins enumerating the friends list for the specified user
// Returns: S_OK if enumeration was successfully started (or if we were
//              already enumerating for that user)
//          Failure hr from XOnlineFriendsEnumerate otherwise
//-----------------------------------------------------------------------------
HRESULT CFriendsManager::StartUpdatingFriends( DWORD dwUser )
{
    // If initialization failed, pretend everything's OK
    if( !m_hFriendsStartup )
        return S_OK;

    m_aFriendsLists[ dwUser ].bAutoStop = FALSE;

    // Check our current state before doing anything:
    switch( m_aFriendsLists[ dwUser ].dwState )
    {
    case STATE_ENUMERATING:
        // If we were already enumerating, we've got nothing left to do
        FM_WARN(( "Already enumerating friends for user %d", dwUser ));
        return S_OK;

    case STATE_CLOSING:
        // If we're waiting for a previous enumeration to finish,
        // then we have to spin until it's done
        FM_WARN(( "Previous enumeration active for user %d - blocking until it's done", dwUser ));

        while( m_aFriendsLists[ dwUser ].dwState == STATE_CLOSING )
        {
            // We don't need to look for errors here - if enumeration fails
            // for any reason, the task will be closed.
            PumpFriends();
        }
        break;
    }
    
    // We must be idle before beginning a new enumeration
    assert( m_aFriendsLists[ dwUser ].dwState == STATE_IDLE );

    // Now kick off the enumeration task
    HRESULT hr = XOnlineFriendsEnumerate( dwUser, NULL, &m_aFriendsLists[ dwUser ].hEnumerate );
    if( SUCCEEDED( hr ) )
    {
        m_aFriendsLists[ dwUser ].dwState = STATE_ENUMERATING;
    }
    else
    {
        // The only specific failure to look for here is E_OUTOFMEMORY.  Any
        // other error implies an unrecoverable internal error.
        FM_WARN(( "XOnlineFriendsEnumerate for user %d failed.  HR = %lx\n", dwUser, hr ));
    }

    return hr;
}



//-----------------------------------------------------------------------------
// Name: StopUpdatingFriends( DWORD dwUser )
// Desc: Stops enumerating the friends list for the specified user.  Note that
//          this doesn't necessarily happen immediately - it may take a few
//          pumps before the enumeration is completely done
// Returns: S_OK always
//-----------------------------------------------------------------------------
HRESULT CFriendsManager::StopUpdatingFriends( DWORD dwUser )
{
    // If initialization failed, pretend everything's OK
    if( !m_hFriendsStartup )
        return S_OK;

    m_aFriendsLists[ dwUser ].bAutoStop = FALSE;

    // Check our current state before doing anything
    switch( m_aFriendsLists[ dwUser ].dwState )
    {
    case STATE_IDLE:
    case STATE_CLOSING:
        // If we're already idle or closing, that means they already called
        // StopUpdatingFriends and there's nothing for us to do.
        FM_WARN(( "Friends enumeration for user %d already stopped\n", dwUser ));
        goto Cleanup;
    }

    // We must be actually enumerating if we're going to tell friends
    // enumeration to shut down
    assert( m_aFriendsLists[ dwUser ].dwState == STATE_ENUMERATING );

    // Tell the online service to stop enumerating friends - this call should
    // never fail
    HRESULT hr = XOnlineFriendsEnumerateFinish( m_aFriendsLists[ dwUser ].hEnumerate );
    assert( SUCCEEDED( hr ) );

    m_aFriendsLists[ dwUser ].dwState = STATE_CLOSING;

Cleanup:
    return S_OK;
}



//-----------------------------------------------------------------------------
// Name: AddPlayerToFriendsList
// Desc: Adds the specified player to the user's friends list.
// Returns: S_OK if operation was succesful
//          XONLINE_E_NOTIFICATION_SELF if xuidPlayer is the same as the user
//          XONLINE_E_NOTIFICATION_LIST_FULL if the user's friend list is full
//          XONLINE_E_NOTIFICATION_USER_ALREADY_EXISTS if already in list
//          XONLINE_E_NOTIFICATION_TOO_MANY_REQUESTS if tasks aren't being pumped
//          E_OUTOFMEMORY if out of memory
//          Other errors imply network or service failure
//-----------------------------------------------------------------------------
HRESULT CFriendsManager::AddPlayerToFriendsList( DWORD dwUser, XUID xuidPlayer )
{
    // If initialization failed, pretend everything's OK
    if( !m_hFriendsStartup )
        return S_OK;

    assert( !FindPlayerInFriendsList( dwUser, xuidPlayer ) );
    assert( !XOnlineIsUserGuest( xuidPlayer.dwUserFlags ) );

    HRESULT hr = XOnlineFriendsRequest( dwUser, xuidPlayer );
    if( FAILED( hr ) )
    {
        FM_WARN(( "XOnlineFriendsRequest by user %d of player %I64x failed.  HR = %lx\n", dwUser, xuidPlayer.qwUserID, hr ));
    }
    
    return hr;
}



//-----------------------------------------------------------------------------
// Name: RemovePlayerFromFriendsList
// Desc: Removes the specified player from the user's friends list.  Must have
//          succesfully initialized, otherwise there's no friend to remove.
// Returns: S_OK if operation was successful
//          S_FALSE if player was not in friends list
//          E_OUTOFMEMORY if out of memory
//          Other errors imply network or service failure
//-----------------------------------------------------------------------------
HRESULT CFriendsManager::RemoveFriendFromFriendsList( DWORD dwUser, XONLINE_FRIEND* pFriend )
{
    assert( m_hFriendsStartup );
    assert( FindPlayerInFriendsList( dwUser, pFriend->xuid ) );
    assert( !XOnlineIsUserGuest( pFriend->xuid.dwUserFlags ) );

    HRESULT hr = XOnlineFriendsRemove( dwUser, pFriend );
    if( FAILED( hr ) )
    {
        FM_WARN(( "XOnlineFriendsRemove by user %d of player %I64x failed.  HR = %lx\n", dwUser, pFriend->xuid.qwUserID, hr ));
    }

    return hr;
}



//-----------------------------------------------------------------------------
// Name: AnswerFriendRequest
// Desc: Answers a friend request.  Must have successfully initialized, 
//          otherwise there's no friend request to answer.
// Returns: S_OK if operation was successful
//          XONLINE_E_NOTIFICATION_USER_NOT_FOUND if friend is not in list
//          XONLINE_E_NOTIFICATION_FRIEND_PENDING if we sent them request
//          S_FALSE if they're already a friend
//          E_OUTOFMEMORY if out of memory
//          Other errors imply network or service failure
//-----------------------------------------------------------------------------
HRESULT CFriendsManager::AnswerFriendRequest( DWORD dwUser, XONLINE_FRIEND* pFriend, XONLINE_REQUEST_ANSWER_TYPE answer )
{
    assert( m_hFriendsStartup );
    assert( pFriend->dwFriendState & XONLINE_FRIENDSTATE_FLAG_RECEIVEDREQUEST );

    HRESULT hr = XOnlineFriendsAnswerRequest( dwUser, pFriend, answer );
    if( FAILED( hr ) )
    {
        FM_WARN(( "XOnlineFriendsAnswerRequest by user %d of player %I64x failed.  HR = %lx\n", dwUser, pFriend->xuid.qwUserID, hr ));
    }

    return hr;
}



//-----------------------------------------------------------------------------
// Name: SendGameInvite
// Desc: Sends a game invitation to the friend.  If pFriend is not NULL, we
//          must have sucessfully initialized, otherwise they have no friends
//          to invite.  If pFriend is NULL, invites ALL Friends.  Must be 
//          updating friends list to call.
// Returns: S_OK if operation was succesful
//          E_OUTOFMEMORY if out of memory
//          Other errors imply network or service failure
//-----------------------------------------------------------------------------
HRESULT CFriendsManager::SendGameInvite( DWORD dwUser, XNKID sessionID, XONLINE_FRIEND* pFriend )
{
    assert( m_hFriendsStartup || !pFriend );

    // If initialization failed, pretend everything's OK
    if( !m_hFriendsStartup )
        return S_OK;

    // Must be enumerating friends in order to send invites
    assert( m_aFriendsLists[ dwUser ].dwState == STATE_ENUMERATING );

    HRESULT hr = XOnlineFriendsGameInvite( dwUser, sessionID, pFriend ? 1 : 0, pFriend );
    if( FAILED( hr ) )
    {
        FM_WARN(( "XOnlineFriendsGameInvite by user %d failed.  HR = %lx\n", dwUser, hr ));
    }

    return hr;
}



//-----------------------------------------------------------------------------
// Name: RevokeGameInvite
// Desc: Revokes a game invitation.  If pFriend is not NULL, we must have
//          succesfully initialized and be updating the friends list, otherwise 
//          they have no friends to have invited.  If pFriend is NULL, revokes 
//          all game invitations
// Returns: S_OK if operation was succesful
//          E_OUTOFMEMORY if out of memory
//          Other errors imply network connectivity 
//-----------------------------------------------------------------------------
HRESULT CFriendsManager::RevokeGameInvite( DWORD dwUser, XNKID sessionID, XONLINE_FRIEND* pFriend )
{
    assert( m_hFriendsStartup || !pFriend );

    // If initialization failed, pretend everything's OK
    if( !m_hFriendsStartup )
        return S_OK;

    // Must be enumerating friends to revoke a specific invite.  Revoking
    // ALL invites is OK.
    assert( m_aFriendsLists[ dwUser ].dwState == STATE_ENUMERATING || !pFriend );

    HRESULT hr = XOnlineFriendsRevokeGameInvite( dwUser, sessionID, pFriend ? 1 : 0, pFriend );
    if( FAILED( hr ) )
    {
        FM_WARN(( "XOnlineFriendsRevokeGameInvite by user %d failed.  HR = %lx\n", dwUser, hr ));
    }

    return hr;
}



//-----------------------------------------------------------------------------
// Name: AnswerGameInvite
// Desc: Answers a game invitiation.  Must have succesfully initalized, 
//          otherwise, there's no game invite to answer.  Must be updating
//          friends list to call.
// Returns: S_OK if operation was succesful
//          XONLINE_E_NOTIFICATION_USER_NOT FOUND if player isn't a friend
//          E_OUTOFMEMORY if out of memory
//          Other errors imply network or service failure, or unrecoverable
//              internal error
//-----------------------------------------------------------------------------
HRESULT CFriendsManager::AnswerGameInvite( DWORD dwUser, XONLINE_FRIEND* pFriend, XONLINE_GAMEINVITE_ANSWER_TYPE answer )
{
    assert( m_hFriendsStartup );
    assert( pFriend->dwFriendState & XONLINE_FRIENDSTATE_FLAG_RECEIVEDINVITE );

    HRESULT hr = XOnlineFriendsAnswerGameInvite( dwUser, pFriend, answer );
    if( FAILED( hr ) )
    {
        FM_WARN(( "XOnlineFriendsAnswerGameInvite by user %d to player %I64x failed.  HR = %lx\n", dwUser, pFriend->xuid.qwUserID, hr ));
    }

    return hr;
}



//-----------------------------------------------------------------------------
// Name: GetAcceptedGameInvite
// Desc: Checks for accepted game invitations on the hard drive (that came
//          from when the user was playing another title).  Title should
//          automatically log users on and join the session in the invite
// Returns: S_OK if operation was succesful and there was an accepted 
//              invitation
//          S_FALSE if operation was succesful and there was NOT an accepted
//              invitation
//          Other errors imply unrecoverable internal error
//-----------------------------------------------------------------------------
HRESULT CFriendsManager::GetAcceptedGameInvite( XONLINE_ACCEPTED_GAMEINVITE* pAcceptedGameInvite )
{
    HRESULT hr = XOnlineFriendsGetAcceptedGameInvite( pAcceptedGameInvite );
    if( FAILED( hr ) )
    {
        FM_WARN(( "XOnlineFriendsGetAcceptedGameInvite failed.  HR = %lx\n", hr ));
    }

    return hr;
}



//-----------------------------------------------------------------------------
// Name: JoinCrossTitleGame
// Desc: Called when the user wants to join a friend's session in another
//          title.  Writes out an accepted invitation to the hard drive - if
//          this succeeds, the title should prompt the user to put in the 
//          appropriate disc.  Must have initialized properly, otherwise
//          there's no friend to try to join.
// Returns: S_OK if the operation was succesful
//          XONLINE_E_NOTIFICATION_SAME_TITLE if the friend is playing the
//              same title
//          Other errors imply unrecoverable internal error
//-----------------------------------------------------------------------------
HRESULT CFriendsManager::JoinCrossTitleGame( DWORD dwUser, XONLINE_FRIEND* pFriend )
{
    assert( m_hFriendsStartup );

    HRESULT hr = XOnlineFriendsJoinGame( dwUser, pFriend );
    if( FAILED( hr ) )
    {
        FM_WARN(( "XOnlineFriendsJoinGame by user %d to player %I64x failed.  HR = %lx\n", dwUser, pFriend->xuid.qwUserID, hr ));
    }

    return hr;
}



//-----------------------------------------------------------------------------
// Name: FindPlayerInFriendsList
// Desc: Tries to find the given player in the user's friends list.
// Returns: Pointer to XONLINE_FRIEND struct if the player is in the list
//          NULL if the player is not in the list
//-----------------------------------------------------------------------------
XONLINE_FRIEND* CFriendsManager::FindPlayerInFriendsList( DWORD dwUser, XUID xuidPlayer )
{
    for( DWORD i = 0; i < m_aFriendsLists[ dwUser ].dwNumFriends; i++ )
    {
        XONLINE_FRIEND* pFriend = GetFriend( dwUser, i );
        if( XOnlineAreUsersIdentical( &pFriend->xuid, &xuidPlayer ) )
            return pFriend;
    }

    return NULL;
}



//-----------------------------------------------------------------------------
// Name: HasFriendsListChanged
// Desc: Determines whether or not the friends list has changed
// Returns: TRUE if friends list has changed since the last call
//          FALSE if it has not changed
//-----------------------------------------------------------------------------
BOOL CFriendsManager::HasFriendsListChanged( DWORD dwUser )
{
    BOOL bReturn = m_aFriendsLists[ dwUser ].bHasChanged;
    m_aFriendsLists[ dwUser ].bHasChanged = FALSE;
    return bReturn;
}



//-----------------------------------------------------------------------------
// Name: GetNumFriends
// Desc: Determines the number of friends contained in the user's friends list
// Returns: Number of friends in user's friends list
//-----------------------------------------------------------------------------
DWORD CFriendsManager::GetNumFriends( DWORD dwUser )
{
    return m_aFriendsLists[ dwUser ].dwNumFriends;
}



//-----------------------------------------------------------------------------
// Name: GetFriend
// Desc: Gets a pointer to an entry in the user's friends list
// Returns: Pointer to XONLINE_FRIEND struct at specified index
//-----------------------------------------------------------------------------
XONLINE_FRIEND* CFriendsManager::GetFriend( DWORD dwUser, DWORD dwIndex )
{
    assert( dwIndex < m_aFriendsLists[ dwUser ].dwNumFriends );

    return &m_aFriendsLists[ dwUser ].pFriends[ dwIndex ];
}



//-----------------------------------------------------------------------------
// Name: GetFriendTitleName
// Desc: Retrieves the name of the friend's current title
// Returns: S_OK if the title name was successfully retrieved
//          Failure hr from XOnlineFriendsGetTitleName otherwise
//-----------------------------------------------------------------------------
HRESULT CFriendsManager::GetFriendTitleName( XONLINE_FRIEND* pFriend, WORD wLanguage, DWORD cTitleName, WCHAR* strTitleName )
{
    assert( cTitleName > 0 && strTitleName != NULL );

    // If we're not currently enumerating any friends lists, we can't
    // call XOnlineFriendsGetTitleName.  In that case, we'll just
    // return an empty string.
    BOOL bIsEnumerating = FALSE;
    for( DWORD i = 0; i < XONLINE_MAX_LOGON_USERS; i++ )
    {
        if( m_aFriendsLists[ i ].dwState == STATE_ENUMERATING )
            bIsEnumerating = TRUE;
    }

    HRESULT hr = S_OK;
    if( bIsEnumerating )
    {
        hr = XOnlineFriendsGetTitleName( pFriend->dwTitleID, wLanguage, cTitleName, strTitleName );
        if( FAILED( hr ) )
        {
            FM_WARN(( "XOnlineFriendsGetTitleName failed.  HR = %lx\n", hr ));
            strTitleName[0] = L'\0';
        }
    }
    else
    {
        strTitleName[0] = L'\0';
    }

    return hr;
}



//-----------------------------------------------------------------------------
// Name: AddPlayerToMuteList
// Desc: Adds a player to the user's mute list
// Returns: S_OK if operation was successful
//          XONLINE_E_NOTIFICATION_SELF if xuidPlayer is the same as user
//          E_OUTOFMEMORY if out of memory
//          Other errors imply network or service failure
//-----------------------------------------------------------------------------
HRESULT CFriendsManager::AddPlayerToMuteList( DWORD dwUser, XUID xuidPlayer )
{
    // If initialization failed, pretend everything's OK
    if( !m_hFriendsStartup )
        return S_OK;

    assert( m_hMuteListStartup );
    assert( m_aMuteLists[ dwUser ].pUsers != NULL );
    assert( !IsPlayerInMuteList( dwUser, xuidPlayer ) );

    // Add the player to the mute list.
    HRESULT hr = XOnlineMutelistAdd( dwUser, xuidPlayer );
    if( SUCCEEDED( hr ) )
    {
        // If the add succeeded, we need to grab an updated mute list.  We
        // can't just call XOnlineMutelistGet right here, because we might
        // already be in the middle of a get.  Just set a flag so we know
        // to update
        m_aMuteLists[ dwUser ].bIsStale = TRUE;
    }
    else
    {
        FM_WARN(( "XOnlineMuteListAdd for user %d of player %I64x failed.  HR = %lx\n", dwUser, xuidPlayer.qwUserID ));
    }

    return hr;
}



//-----------------------------------------------------------------------------
// Name: RemovePlayerFromMuteList
// Desc: Removes a player from the user's mute list.  Must have intialized
//          properly, otherwise there's no in the mute list to remove.
// Returns: S_OK if operation was successful
//          XONLINE_E_NOTIFICATION_SELF if xuidPlayer is the same as user
//          E_OUTOFMEMORY if out of memory
//          Other errors imply network or service failure
//-----------------------------------------------------------------------------
HRESULT CFriendsManager::RemovePlayerFromMuteList( DWORD dwUser, XUID xuidPlayer )
{
    assert( m_hMuteListStartup );
    assert( m_aMuteLists[ dwUser ].pUsers != NULL );
    assert( IsPlayerInMuteList( dwUser, xuidPlayer ) );

    // Remove the player from the mute list
    HRESULT hr = XOnlineMutelistRemove( dwUser, xuidPlayer );
    if( SUCCEEDED( hr ) )
    {
        // If the remove succeeded, we need to grab an updated mute list.  We
        // can't just call XOnlineMutelistGet right here, because we might
        // already be in the middle of a get.  Just set a flag so we know
        // to update
        m_aMuteLists[ dwUser ].bIsStale = TRUE;
    }
    else
    {
        FM_WARN(( "XOnlineMuteListRemove for user %d of player %I64X failed.  HR = %lx\n", dwUser, xuidPlayer.qwUserID ));
    }

    return hr;
}



//-----------------------------------------------------------------------------
// Name: IsPlayerInMuteList
// Desc: Determines whether a player is in the user's mute list or not
// Returns: TRUE if the player is in the user's mute list
//          FALSE if the player is not in the user's mute list
//-----------------------------------------------------------------------------
BOOL CFriendsManager::IsPlayerInMuteList( DWORD dwUser, XUID xuidPlayer )
{
    // Walk through the user's mute list, looking for the specified XUID
    for( DWORD i = 0; i < m_aMuteLists[ dwUser ].dwNumUsers; i++ )
    {
        if( XOnlineAreUsersIdentical( &m_aMuteLists[ dwUser ].pUsers[ i ].xuid, &xuidPlayer ) )
        {
            return TRUE;
        }
    }
    
    return FALSE;
}



//-----------------------------------------------------------------------------
// Name: SendFeedback
// Desc: Sends feedback on the specified player
// Returns: hr from XOnlineFeedbackSend
//-----------------------------------------------------------------------------
HRESULT CFriendsManager::SendFeedback( DWORD dwUser, 
                                       XUID xuidPlayer, 
                                       XONLINE_FEEDBACK_TYPE feedback,
                                       WCHAR* strNickname )
{
    // If initialization failed, pretend everything's OK
    if( !m_hFriendsStartup )
        return S_OK;

    XONLINETASK_HANDLE hFeedback = NULL;

    HRESULT hr = XOnlineFeedbackSend( dwUser,
                                      xuidPlayer,
                                      feedback,
                                      strNickname,
                                      NULL,
                                      &hFeedback );

    // If we succesfully started the feedback task, add it to the list
    // of tasks to pump
    if( SUCCEEDED( hr ) )
    {
        m_ahFeedback.push_back( hFeedback );
    }

    return hr;
}



///////////////////////////////////////////////////////////////////////////////
// Protected Functions
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
// Name: UpdateMuteList
// Desc: Begins enumerating the mute list for the specified player.  Note that
//          this just starts the update process
// Returns: S_OK if enumeration was successfully started
//          E_OUTOFMEMORY if out of memory
//-----------------------------------------------------------------------------
HRESULT CFriendsManager::UpdateMuteList( DWORD dwUser )
{
    assert( m_hMuteListStartup );
    assert( m_aMuteLists[ dwUser ].hEnumerate == NULL );

    // Start getting an updated list
    HRESULT hr = XOnlineMutelistGet( dwUser,
                                     MAX_MUTELISTUSERS,
                                     NULL,
                                     &m_aMuteLists[ dwUser ].hEnumerate,
                                     m_aMuteLists[ dwUser ].pUsers,
                                     &m_aMuteLists[ dwUser ].dwNumUsers );
    if( FAILED( hr ) )
    {
        // The only error we expect from XOnlineMutelistGet is E_OUTOFMEMORY
        assert( hr == E_OUTOFMEMORY );
        FM_WARN(( "XOnlineMuteListGet for user %d failed.  HR = %lx\n", hr ));
    }

    return hr;
}



//-----------------------------------------------------------------------------
// Name: PumpFriends
// Desc: Handles pumping of all friends-related work
// Returns: S_OK if all tasks were processed succesfully.
//          hr from first failure otherwise
//-----------------------------------------------------------------------------
HRESULT CFriendsManager::PumpFriends()
{
    HRESULT hrReturn = S_OK;

    // Pump the friends startup task
    HRESULT hrStartup;
    hrStartup = XOnlineTaskContinue( m_hFriendsStartup );

    // If the friends startup task failed, then one of the friends operations
    // (add friend, remove friend, etc.) failed.  Since there's not much we
    // can do about it, we'll just ignore it and return the HRESULT.  We could
    // continue processing other tasks, but this gives the title a chance to
    // check for a connectivity problem via the logon task before we spew a 
    // bunch of redundant errors.
    if( FAILED( hrStartup ) )
    {
        FM_WARN(( "XOnlineTaskContinue of Friends Startup task failed - ignoring.  HR = %lx\n", hrStartup ));
        hrReturn = hrStartup;
        goto Cleanup;
    }

    // Pump each individual user's friends enumeration task
    HRESULT hrEnumerate = XONLINETASK_S_SUCCESS;
    for( DWORD i = 0; i < XONLINE_MAX_LOGON_USERS; i++ )
    {
        switch( m_aFriendsLists[ i ].dwState )
        {
        case STATE_IDLE:
            // Idle -  nothing to do
            break;

        case STATE_ENUMERATING:
            // Enumerating - keep pumping the enmueration task,
            // looking for failure or results available
            hrEnumerate = XOnlineTaskContinue( m_aFriendsLists[ i ].hEnumerate );

            // If the enumeration failed, we should restart the  enumeration.
            // If it failed because of a connectivity problem, that will be 
            // reflected by the logon task, and the friends manager should be
            // shut down.
            if( FAILED( hrEnumerate ) )
            {
                FM_WARN(( "XOnlineTaskContinue of Friends Enumeration task for user %d failed.  HR = %lx\n", i, hrEnumerate ));
                hrReturn = hrEnumerate;
                XOnlineTaskClose( m_aFriendsLists[ i ].hEnumerate );

                // Attempt to restart enumeration
                hrEnumerate = XOnlineFriendsEnumerate( i, NULL, &m_aFriendsLists[ i ].hEnumerate );
                if( SUCCEEDED( hrEnumerate ) )
                {
                    FM_WARN(( "Successfully restarted friends enumeration for user %d.\n", i ));
                }
                else
                {
                    FM_WARN(( "Restart of Friends Enumeration task for user %d failed.  HR = %lx\n", i, hrEnumerate ));
                    m_aFriendsLists[ i ].hEnumerate = NULL;
                    m_aFriendsLists[ i ].dwState = STATE_IDLE;
                }

                goto Cleanup;
            }
            else if( hrEnumerate == XONLINETASK_S_RESULTS_AVAIL )
            {
                // Get the updated friends list
                m_aFriendsLists[ i ].dwNumFriends = XOnlineFriendsGetLatest( i, MAX_FRIENDS, m_aFriendsLists[ i ].pFriends );
                m_aFriendsLists[ i ].bHasChanged  = TRUE;

                // This flag should be set only for the enumeration we kicked
                // off in Initialize().  If the title started or stopped 
                // enumeration explicitly, the flag should have been cleared.
                if( m_aFriendsLists[ i ].bAutoStop )
                {
                    m_aFriendsLists[ i ].bAutoStop = FALSE;
                    StopUpdatingFriends( i );
                }
            }
            else
            {
                // We're not supposed to get any other success code
                assert( hrEnumerate == XONLINETASK_S_RUNNING );
            }
            break;

        case STATE_CLOSING:
            // Closing - keep pumping the enumeration task until
            // it returns XONLINETASK_S_SUCCESS or an error
            hrEnumerate = XOnlineTaskContinue( m_aFriendsLists[ i ].hEnumerate );

            if( FAILED( hrEnumerate ) || hrEnumerate == XONLINETASK_S_SUCCESS )
            {
                XOnlineTaskClose( m_aFriendsLists[ i ].hEnumerate );
                m_aFriendsLists[ i ].hEnumerate = NULL;
                m_aFriendsLists[ i ].dwState = STATE_IDLE;

                if( FAILED( hrEnumerate ) )
                {
                    FM_WARN(( "XOnlineTaskContinue of Friends Enumeration task for user %d failed while closing.  HR = %lx\n", i, hrEnumerate ));
                    hrReturn = hrEnumerate;
                    goto Cleanup;
                }
            }
            else
            {
                // We're not supposed to get any other success code
                assert( hrEnumerate == XONLINETASK_S_RUNNING );
            }
            break;
        }
    }

Cleanup:
    return hrReturn;
}



//-----------------------------------------------------------------------------
// Name: PumpMuteList
// Desc: Handles pumping of all mutelist-related work
// Returns: S_OK if all tasks were processed successfully
//          hr from first failure otherwise
//-----------------------------------------------------------------------------
HRESULT CFriendsManager::PumpMuteList()
{
    HRESULT hrReturn = S_OK;

    // Pump the mutelist startup task
    HRESULT hrStartup = XOnlineTaskContinue( m_hMuteListStartup );

    // If the mutelist startup task failed, then one of the mutelist operations
    // failed.  Since there's not much we can do about it, we'll just ignore
    // it and return the HRESULT, so that the title has a chance to check for
    // connectivity problems via the logon task.  
    if( FAILED( hrStartup ) )
    {
        FM_WARN(( "XOnlineTaskContinue of Mutelist starup task failed - ignoring.  HR = %lx\n", hrStartup ));
        hrReturn = hrStartup;
        goto Cleanup;
    }

    // Pump each individual user's mutelist enumeration task
    HRESULT hrEnumerate = XONLINETASK_S_SUCCESS;
    for( DWORD i = 0; i < XONLINE_MAX_LOGON_USERS; i++ )
    {
        if( m_aMuteLists[ i ].hEnumerate ) 
        {
            hrEnumerate = XOnlineTaskContinue( m_aMuteLists[ i ].hEnumerate );

            // If the enumeration failed, we should restart the enumeration.
            // If it failed because of a connectivity problem, that will be
            // reflected by the logon task, and the friends manager should be
            // shut down.
            if( FAILED( hrEnumerate ) ) 
            {
                FM_WARN(( "XOnlineTaskContinue of Mutelist Enumeration task for user %d failed.  HR = %lx\n", i, hrEnumerate ));
                hrReturn = hrEnumerate;
                XOnlineTaskClose( m_aMuteLists[ i ].hEnumerate );

                // Set flag noting that we need to get an updated mute list
                m_aMuteLists[ i ].bIsStale = TRUE;
                goto Cleanup;
            }
            else if( hrEnumerate == XONLINETASK_S_SUCCESS )
            {
                XOnlineTaskClose( m_aMuteLists[ i ].hEnumerate );
                m_aMuteLists[ i ].hEnumerate = NULL;
            }
            else
            {
                // We're not supposed to get any other success code
                assert( hrEnumerate == XONLINETASK_S_RUNNING );
            }
        }
        else if( m_aMuteLists[ i ].bIsStale )
        {
            // If we need to get an updated mute list, kick that off now
            if( SUCCEEDED( UpdateMuteList( i ) ) )
            {
                m_aMuteLists[ i ].bIsStale = FALSE;
            }
        }
    }

Cleanup:
    return hrReturn;
}




//-----------------------------------------------------------------------------
// Name: PumpFeedback
// Desc: Handles pumping of all feedback-related work.
// Returns: XONLINETASK_S_RUNNING if work is still being done (and no errors)
//          XONLINETASK_S_SUCCESS if all work is complete (and no errors)
//          Errors imply network or service failure
//-----------------------------------------------------------------------------
HRESULT CFriendsManager::PumpFeedback()
{
    HRESULT hrTotal = XONLINETASK_S_SUCCESS;

    // Loop over each feedback task that we have to pump
    for( TASKLIST::iterator i = m_ahFeedback.begin(); i < m_ahFeedback.end(); ++i )
    {
        HRESULT hrTask = XOnlineTaskContinue( *i );

        if( hrTask != XONLINETASK_S_RUNNING )
        {
            // If the task is no longer running, we can close it and remove it 
            // from the list.  Note that we will not pass out an error - if an 
            // attempt to send feedback failed, then no one will ever know
            XOnlineTaskClose( *i );
            m_ahFeedback.erase( i );

            if( FAILED( hrTask ) )
            {
                // If a feedback task fails, there's pretty much nothing you
                // can do about it.  It's not worth trying to re-start.  We'll
                // pass the error out in case the caller is interested.
                FM_WARN(( "XOnlineTaskContinue of feedback task failed.  HR = %lx\n", hrTask ));

                hrTotal = hrTask;
                goto Cleanup;
            }
        }
        else
        {
            // If _any_ task is still running, then we should return 
            // XONLINETASK_S_RUNNING to our caller
            hrTotal = XONLINETASK_S_RUNNING;
        }
    }

Cleanup:
    return hrTotal;
}



#if _DEBUG
//-----------------------------------------------------------------------------
// Name: DbgValidateState
// Desc: Validates all internal state
//-----------------------------------------------------------------------------
void CFriendsManager::DbgValidateState()
{
    // See if we're active or not
    if( !m_hFriendsStartup )
    {
        // If we don't have a friends startup task, we better not have a 
        // mutelist startup task, either
        assert( !m_hMuteListStartup );

        // Make sure each friends lists is zero-ed out
        for( DWORD i = 0; i < XONLINE_MAX_LOGON_USERS; i++ )
        {
            assert( m_aFriendsLists[ i ].dwState == STATE_IDLE );
            assert( m_aFriendsLists[ i ].hEnumerate == NULL );
            assert( m_aFriendsLists[ i ].bHasChanged == FALSE );
            assert( m_aFriendsLists[ i ].dwNumFriends == 0 );
            assert( m_aFriendsLists[ i ].pFriends == NULL );
        }

        // Make sure each mute list is zero-ed out
        for( DWORD i = 0; i < XONLINE_MAX_LOGON_USERS; i++ )
        {
            assert( m_aMuteLists[ i ].bIsStale == FALSE );
            assert( m_aMuteLists[ i ].dwNumUsers == 0 );
            assert( m_aMuteLists[ i ].hEnumerate == NULL );
            assert( m_aMuteLists[ i ].pUsers == NULL );
        }

        // Shouldn't have any feedback tasks
        assert( m_ahFeedback.size() == 0 );
    }
    else
    {
        // If we do have a friends startup task, we must have a mutelist
        // startup task
        assert( m_hMuteListStartup );

        // We're active.  Verify that we have logged-on users
        XONLINE_USER* pUsers = XOnlineGetLogonUsers();
        assert( pUsers != NULL );

        for( DWORD i = 0; i < XONLINE_MAX_LOGON_USERS; i++ )
        {
            // Is there someone logged on in this slot, and not a guest?
            if( pUsers[ i ].xuid.qwUserID != 0 &&
                !XOnlineIsUserGuest( pUsers[ i ].xuid.dwUserFlags ) )
            {
                // Check friends list
                assert( m_aFriendsLists[ i ].pFriends != NULL );

                // Verify that all friends up to dwNumFriends are valid
                for( DWORD j = 0; j < m_aFriendsLists[ i ].dwNumFriends; j++ )
                {
                    assert( m_aFriendsLists[ i ].pFriends[ j ].xuid.qwUserID != 0 );
                }

                // Make sure their friends enumeration state matches
                // to their friends enumeration task handle
                switch( m_aFriendsLists[ i ].dwState )
                {
                case STATE_IDLE:
                    assert( m_aFriendsLists[ i ].hEnumerate == NULL );
                    assert( !m_aFriendsLists[ i ].bAutoStop );
                    break;
                case STATE_ENUMERATING:
                case STATE_CLOSING:
                    assert( m_aFriendsLists[ i ].hEnumerate != NULL );
                    break;
                }

                // Check mute list
                assert( m_aMuteLists[ i ].pUsers != NULL );

                // Verify that all users up to dwNumUsers are valid
                for( DWORD j = 0; j < m_aMuteLists[ i ].dwNumUsers; j++ )
                {
                    assert( m_aMuteLists[ i ].pUsers[ j ].xuid.qwUserID != 0 );
                }
            }
            else
            {
                // Not logged on, or not guest - no friends list or mute list
                assert( m_aFriendsLists[ i ].dwState == STATE_IDLE );
                assert( m_aFriendsLists[ i ].hEnumerate == NULL );
                assert( m_aFriendsLists[ i ].bHasChanged == FALSE );
                assert( m_aFriendsLists[ i ].dwNumFriends == 0 );
                assert( m_aFriendsLists[ i ].pFriends == NULL );

                assert( m_aMuteLists[ i ].bIsStale == FALSE );
                assert( m_aMuteLists[ i ].dwNumUsers == 0 );
                assert( m_aMuteLists[ i ].hEnumerate == NULL );
                assert( m_aMuteLists[ i ].pUsers == NULL );
            }
        }

        // For feedback, the only thing we can do is make sure the handles
        // are valid (ie, not NULL)
        for( TASKLIST::iterator i = m_ahFeedback.begin(); i < m_ahFeedback.end(); ++i )
        {
            assert( *i != NULL );
        }
    }
}


//-----------------------------------------------------------------------------
// Name: DbgWarn
// Desc: Prints out a debug warning message
//-----------------------------------------------------------------------------
void CFriendsManager::DbgWarn( CHAR* format, ... )
{
    va_list arglist;
    CHAR strTemp[ DBGWARN_SIZE ];

    va_start( arglist, format );
    _vsnprintf( strTemp, DBGWARN_SIZE, format, arglist );
    va_end( arglist );

    OutputDebugString( strTemp );
}
#endif // _DEBUG