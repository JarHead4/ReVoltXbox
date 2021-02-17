//-----------------------------------------------------------------------------
// File: XBOnline.cpp
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include <xtl.h>
#include <xonline.h>
#include "XBOnline.h"

#include "network.h"     // for XnAddrLocal and InitNetwork()/KillNetwork()
#include "net_xonline.h" // for OnlineTasks_**() funcs

#include "ui_MenuText.h" // For access to localized text strings
#include "ui_ShowMessage.h" // for displaying XOnline errors to player
#include "content.h"



//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------

// The task handle for XOnline API
XONLINETASK_HANDLE    g_hSignInTask = NULL;


// Available XOnline services
DWORD g_pXOnlineServices[] = 
{
    XONLINE_MATCHMAKING_SERVICE,
    XONLINE_BILLING_OFFERING_SERVICE,
    XONLINE_STATISTICS_SERVICE,
    XONLINE_FEEDBACK_SERVICE,
    REVOLT_SUBSCRIPTION_ID,
};

const DWORD NUM_XONLINE_SERVICES = (sizeof(g_pXOnlineServices)/sizeof(DWORD));

// The list of XOnline users
static XONLINE_USER          g_XOnlineUserList[XONLINE_MAX_STORED_ONLINE_USERS];

#ifdef _DEBUG
    //$ADDED - mwetzel - Added a flag for working on UI when no valid user
    // accounts exist
    BOOL g_bFakeOnlineStuff = FALSE;
#endif




//-----------------------------------------------------------------------------
// Name: XBOnline_Startup
// Desc: Initializes the online libraries
//-----------------------------------------------------------------------------
HRESULT XBOnline_Startup()
{
    HRESULT hr;

    // Network init
    InitNetwork();

    XONLINE_STARTUP_PARAMS xosp = { 0 };
    hr = XOnlineStartup( &xosp );
    if( FAILED(hr) )
    {
        swprintf( g_SimpleMessageBuffer,
                  L"XOnlineStartup\n"
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

        KillNetwork();
        return E_FAIL;
    }

    // Init online task handling
    OnlineTasks_Startup();

    return S_OK;
}



//-----------------------------------------------------------------------------
// Name: XBOnline_Cleanup
// Desc: Cleans up after the online libraries
//-----------------------------------------------------------------------------
HRESULT XBOnline_Cleanup()
{
    // Clean up online
    XOnlineCleanup();

    // Clean up net stack
    KillNetwork();

    return S_OK;
}



//-----------------------------------------------------------------------------
// Name: XBOnline_GetUserList()
// Desc: Gets the list on available online users
//-----------------------------------------------------------------------------
HRESULT XBOnline_GetUserList( XONLINE_USER** ppUserList, DWORD* pdwNumUsers )
{
    // Static list of users
    DWORD dwNumUsers = 0;

    // Get accounts stored on the hard disk
    HRESULT hr = XOnlineGetUsers( g_XOnlineUserList, &dwNumUsers );  //$UGLY: 'g_XOnlineUserList' gets passed as argument 'ppUserList', but then we use it directly here?
    if( FAILED(hr) )
    {
        (*pdwNumUsers) = 0;
        return hr;
    }

    (*ppUserList)  = g_XOnlineUserList;
    (*pdwNumUsers) = dwNumUsers;

#ifdef _DEBUG
    // If no users exist, you can create some fake ones here for debugging
    if( g_bFakeOnlineStuff && dwNumUsers == 0)
    {
        (*pdwNumUsers) = 2;
        (*ppUserList)  = g_XOnlineUserList;

        // g_XOnlineUserList[0].xuid = ?;

        strcpy( g_XOnlineUserList[0].szGamertag, "PinXXXX" );
        g_XOnlineUserList[0].dwUserOptions = XONLINE_USER_OPTION_REQUIRE_PASSCODE;
        g_XOnlineUserList[0].passcode[0] = XONLINE_PASSCODE_GAMEPAD_X;
        g_XOnlineUserList[0].passcode[1] = XONLINE_PASSCODE_GAMEPAD_X;
        g_XOnlineUserList[0].passcode[2] = XONLINE_PASSCODE_GAMEPAD_X;
        g_XOnlineUserList[0].passcode[3] = XONLINE_PASSCODE_GAMEPAD_X;
        ZeroMemory( g_XOnlineUserList[0].reserved, XONLINE_USER_RESERVED_SIZE );
        g_XOnlineUserList[0].hr = S_OK;

        // g_XOnlineUserList[0].xuid = ?;
        strcpy( g_XOnlineUserList[1].szGamertag, "PinlessJoe" );
        g_XOnlineUserList[1].dwUserOptions = 0;
        ZeroMemory( g_XOnlineUserList[1].passcode, XONLINE_PASSCODE_LENGTH );
        ZeroMemory( g_XOnlineUserList[1].reserved, XONLINE_USER_RESERVED_SIZE );
        g_XOnlineUserList[1].hr = S_OK;
    }
#endif

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: XBOnline_IsActive()
// Desc: Returns true is the ethernet link is active
//-----------------------------------------------------------------------------
BOOL XBOnline_IsActive()
{
    // Poll status
    DWORD dwStatus = XNetGetEthernetLinkStatus();
    return ( dwStatus & XNET_ETHERNET_LINK_ACTIVE ) ? TRUE : FALSE;
}




//-----------------------------------------------------------------------------
// Name: XBOnline_BeginSignIn()
// Desc: Initiate the authentication process
//-----------------------------------------------------------------------------
HRESULT XBOnline_BeginSignIn( XONLINE_USER* pUserList[4] )
{
    HRESULT hr;

    //$REVISIT: Should the XBOnline_Startup remain implicit, or should we
    // make it be called explicitly?
    hr = XBOnline_Startup();
    if( FAILED( hr ) )
    {
        return hr;
    }

    // XOnlineLogon() allows a list of up to 4 players (1 per controller)
    // to login in a single call. This sample shows how to authenticate
    // a single user. The list must be a one-to-one match of controller 
    // to player in order for the online system to recognize which player
    // is using which controller.
    XONLINE_USER UserList[4];
    ZeroMemory( UserList, sizeof(UserList) );

    for( DWORD i=0; i<4; i++ )
    {
        if( pUserList[i] )
            CopyMemory( &UserList[i], pUserList[i], sizeof(XONLINE_USER) );
    }
    
    // Initiate the login process. XOnlineTaskContinue() is used to poll
    // the status of the login.
    hr = XOnlineLogon( UserList, g_pXOnlineServices, NUM_XONLINE_SERVICES, 
                        NULL, &g_hSignInTask );
    if( FAILED(hr) )
    {
        swprintf( g_SimpleMessageBuffer,
                  L"XOnlineLogon\n"
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

        XBOnline_SignOut();
        return hr;
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: XBOnline_PumpSignInTask()
// Desc: 
//-----------------------------------------------------------------------------
HRESULT XBOnline_PumpSignInTask()
{
    // Make sure we have a valid task handle
    if( NULL == g_hSignInTask )
        return E_INVALIDARG;

#ifdef _DEBUG
    // Fake the sign in succeeding
    if( g_bFakeOnlineStuff )
        return XONLINETASK_S_SUCCESS;
#endif

    // Continue with the sign in task
    HRESULT hrPumpTaskResult = XOnlineTaskContinue( g_hSignInTask );
    if( hrPumpTaskResult == XONLINETASK_S_RUNNING )
        return hrPumpTaskResult;

    if( FAILED(hrPumpTaskResult) )
    {
//$TODO: If this fails, we should postpone displaying a message here,
// but go ahead and sign out and return the error to the calling function.
// That other function should display the error (using g_ShowSimpleMessage
// in fine) and then return players to the main menu.
// So far, the only two valid errors are CONNECTION_LOST and
// KICKED_BY_DUPLICATE_LOGON

//$REMOVED - mwetzel - errors are displayed elsewhere
//        swprintf( g_SimpleMessageBuffer,
//                  L"OnlineLogon TaskContinue\n"
//                  L"returned HR=%08x\n"
//                  L"\n"
//                  L"Please write this down and report it.\n"
//                  L"Also, mention whether you saw any problems\n"
//                  L"soon after this message appeared.\n"
//                  , hrPumpTaskResult
//                );
//        g_ShowSimpleMessage.Begin( L"XON Warning!",
//                                   g_SimpleMessageBuffer,
//                                   NULL, TEXT_TABLE(TEXT_BUTTON_A_CONTINUE) );
//$END_REMOVAL

        // "Don't continue" flags indicate that the authentication failed,
        // perhaps because the connection was lost
        XBOnline_SignOut();
        return hrPumpTaskResult;
    }

    // clear subscription
    g_bHasSubscription = FALSE;

    // Check for service errors
    for( DWORD i = 0; i < NUM_XONLINE_SERVICES ; i++ )
    {
        HRESULT hrGetServicesResult = XOnlineGetServiceInfo( g_pXOnlineServices[i], 
                                                             NULL );

        // $MD: added.  Check to see if we are subscribed.
        if( g_pXOnlineServices[i] == REVOLT_SUBSCRIPTION_ID )
        {
            // if the subcription service request succeded, we are subscribed
            if( SUCCEEDED(hrGetServicesResult) )
                g_bHasSubscription = TRUE;
            continue;
            
        }
        

            
            //$REVISIT: In the case where an account requires management, they're
            // going to be denied access to some services.  However, the basic
            // logon was successful.  If we return a service error here, we're 
            // going to mask the fact that the account requires management as
            // a failed logon.
#if 0
        if( FAILED(hrGetServicesResult) )
        {
            swprintf( g_SimpleMessageBuffer,
                      L"GetServicesInfo (index %d)\n"
                      L"returned HR=%08x\n"
                      L"\n"
                      L"Please write this down and report it.\n"
                      L"Also, mention whether you saw any problems\n"
                      L"soon after this message appeared.\n"
                      , i, hrGetServicesResult
                    );
            g_ShowSimpleMessage.Begin( L"XON Warning!",
                                       g_SimpleMessageBuffer,
                                       NULL, TEXT_TABLE(TEXT_BUTTON_A_CONTINUE) );
            
            XBOnline_SignOut();
            return hrGetServicesResult;
        }
#endif
    }

    // If we get here, sign in succeeded.

    // After XOnlineLogon succeeds, game needs to refresh its XNADDR
    while( XNET_GET_XNADDR_PENDING == XNetGetTitleXnAddr(&XnAddrLocal) ) { /*spin*/ }
    //$REVISIT: do we need to do this only when state changes from none<->some
    /// users logged on, or every time number of logged-in users changes?
    /// (Maybe no effect on perf and thus doesn't hurt to do on every change.)
    
    // Return the success code.
    return hrPumpTaskResult;
}




//-----------------------------------------------------------------------------
// Name: XBOnline_SignOut()
// Desc: 
//-----------------------------------------------------------------------------
VOID XBOnline_SignOut()
{
    // Cleanup the sign in task, if necessary
    if( g_hSignInTask ) 
    {
        XOnlineTaskClose( g_hSignInTask );
        g_hSignInTask = NULL;
    }

    // Cleanup online task handling
    OnlineTasks_Cleanup();

    // Network uninit
    XBOnline_Cleanup();

    // When sign out, game needs to refresh its XNADDR
    while( XNET_GET_XNADDR_PENDING == XNetGetTitleXnAddr(&XnAddrLocal) ) { /*spin*/ }
    //$REVISIT: do we need to do this only when state changes from none<->some
    /// users logged on, or every time number of logged-in users changes?
    /// (Maybe no effect on perf and thus doesn't hurt to do on every change.)

    // //$MD: not subscribed when not online
    g_bHasSubscription = FALSE;
}

