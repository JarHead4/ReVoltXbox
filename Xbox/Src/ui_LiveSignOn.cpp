//-----------------------------------------------------------------------------
// File: ui_LiveSignOn.cpp
//
// Desc: UI implementation
//
// Re-Volt (Generic) Copyright (c) Probe Entertainment 1998
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"
#include "XBOnline.h"
#include "XBFont.h"
#include "XBResource.h"
#include "XBInput.h"

#include "main.h"
#include "Text.h"
#include "ui_Menu.h"
#include "ui_MenuText.h"
#include "ui_TitleScreen.h"
#include "ui_StateEngine.h"
#include "ui_LiveSignOn.h"
#include "ui_MenuDraw.h"
#include "net_xonline.h"
#include "VoiceManager.h"
#include "ui_contentdownload.h"
#include "FriendsManager.h"


//$HACK: right now, parts of the game engine assume 1 player per box, so
// use this define to slightly alter the UI look and flow as necessary to
// match.
#define SINGLE_PLAYER_UI


CLiveSignInStateEngine g_LiveSignInStateEngine;


extern void DrawNewTitleBox( FLOAT xPos, FLOAT yPos, FLOAT xSize, FLOAT ySize, int col, int trans );
extern void DrawNewSpruBox( FLOAT, FLOAT, FLOAT, FLOAT );
extern void DrawNewSpruBoxWithTabs( FLOAT, FLOAT, FLOAT, FLOAT );


//-----------------------------------------------------------------------------
// Name: class CPlayerScreenStateEngine
// Desc: Class to handle and render a screen for a player that is signing in.
//-----------------------------------------------------------------------------
class CPlayerScreenStateEngine
{
protected:
    FLOAT     m_x;
    FLOAT     m_y;
    
    DWORD     m_dwCurrentUser;                     // User ID in global list
    PLAYER_XONLINE_INFO* m_pPlayer;                // Pointer to the player
    XBINPUT_CONTROLLER*  m_pController;
    BOOL      m_bEnteredWrongPassCode;

    enum 
    {
        STATE_WAITING_FOR_PLAYER,
        STATE_SELECTING_ACCOUNT,
        STATE_REBOOT_TO_ONLINE_DASH,
        STATE_ENTERING_PASS_CODE,
        STATE_WAITING_TO_SIGNIN,
        STATE_SIGNING_IN,
        STATE_SIGNIN_SUCCEEDED,
        STATE_REQUIRED_CONTENT_ENUM,
        STATE_UPDATE_REQUIRED_CONTENT,
        STATE_REQUIRED_CONTENT_UPDATED,
        STATE_UPDATE_SUCCEEDED,
        STATE_SIGNIN_FAILED,
    };
    DWORD  m_State;        // State of the player screen
    FLOAT  m_fSignInTime;

protected:
    // Input handling functions
    HRESULT UpdateWaitingForPlayerScreen( DWORD InputEvent );
    HRESULT UpdateSelectAccountScreen( DWORD InputEvent );
    HRESULT UpdateRebootToOnlineDashScreen( DWORD InputEvent );
    HRESULT UpdateEnterPassCodeScreen( DWORD InputEvent );
    HRESULT UpdateSigningInScreen( DWORD InputEvent );
    HRESULT UpdateSignInSucceeded( DWORD InputEvent );
    HRESULT UpdateSignInFailed( DWORD InputEvent );
    HRESULT UpdateWaitingForOtherUsers( DWORD InputEvent );

    // Render functions
    VOID RenderWaitingForPlayerScreen( FLOAT sx, FLOAT sy );
    VOID RenderSelectAccountScreen( FLOAT sx, FLOAT sy );
    VOID RenderRebootToOnlineDashScreen( FLOAT sx, FLOAT sy );
    VOID RenderEnterPassCodeScreen( FLOAT sx, FLOAT sy );
    VOID RenderWaitingForOtherUsers( FLOAT sx, FLOAT sy );
    VOID RenderSigningInScreen( FLOAT sx, FLOAT sy );
    VOID RenderSignInSucceeded( FLOAT sx, FLOAT sy );
    VOID RenderSignInFailed( FLOAT sx, FLOAT sy );

public:
    // Initialization functions
    VOID    Initialize( FLOAT x, FLOAT y, PLAYER_XONLINE_INFO* pPlayer, XBINPUT_CONTROLLER* pController );
    VOID    SignOut()  { m_State = STATE_WAITING_TO_SIGNIN; m_pPlayer->bIsLoggedIn = FALSE; }

    HRESULT Process();
    VOID    Render( FLOAT ViewportStartX, FLOAT ViewportStartY );

    // Status checking functions
    BOOL    IsActive()           { return m_State != STATE_WAITING_FOR_PLAYER; }
    BOOL    IsPlayerSigningIn()  { return m_State == STATE_SIGNING_IN; }
    //$MD: changed from sign in succeeded to update succeded
    BOOL    IsPlayerSignedIn()   { return m_State == STATE_UPDATE_SUCCEEDED; }
    BOOL    IsWaitingToSignIn()  { return m_State == STATE_WAITING_TO_SIGNIN; }
};




CPlayerScreenStateEngine g_PlayerScreens[MAX_LOCAL_PLAYERS];

XONLINE_USER* g_XOnlineUserList;
DWORD         g_dwNumUsers;

DWORD         g_dwNumActivePlayers = 0L;

BOOL          g_bReadyToSignIn      = FALSE;
BOOL          g_bAllPlayersSignedIn = FALSE;
BOOL          g_bPlayersCanJoin     = TRUE;

BOOL          g_bSignInTerminated   = FALSE;
BOOL          g_bSignInCompleted    = FALSE;

HRESULT       g_hrSignInResult;



//-----------------------------------------------------------------------------
// Name: CheckSignInConditions()
// Desc: 
//-----------------------------------------------------------------------------
VOID CheckSignInConditions()
{
    // Check states of all players
    g_dwNumActivePlayers = 0;
    DWORD dwNumPlayersNotActive     = 0;
    DWORD dwNumPlayersReadyToSignIn = 0;
    DWORD dwNumPlayersSigningIn     = 0;
    DWORD dwNumPlayersSignedIn      = 0;

    for( DWORD i=0; i<MAX_LOCAL_PLAYERS; i++ )
    {
        // Count active players (if the player screen is not at the
        // starting screen, it means a player is active)
        if( g_PlayerScreens[i].IsActive() )
            g_dwNumActivePlayers++;

        // Count players who have already successfully signed in
        if( g_PlayerScreens[i].IsPlayerSigningIn() )
            dwNumPlayersSigningIn++;

        // Count players who have already successfully signed in
        if( g_PlayerScreens[i].IsPlayerSignedIn() )
            dwNumPlayersSignedIn++;

        // Count players (and determine errors) on players waiting to sign in
        if( g_PlayerScreens[i].IsWaitingToSignIn() )
        {
            BOOL bGuestHasValidUser = FALSE;

            // Assume no error to start
            Players[i].XOnlineInfo.dwSignInError = 0;

            // Check current players against all other players (for duplicate
            // users and make sure guests have valid users)
            for( DWORD j=0; j<MAX_LOCAL_PLAYERS; j++ )
            {
                if( i == j || FALSE == g_PlayerScreens[j].IsWaitingToSignIn() )
                    continue;

                // Check for two players referencing the same user
                if( Players[i].XOnlineInfo.pXOnlineUser == Players[j].XOnlineInfo.pXOnlineUser )
                {
                    // If neither are players are guests, then they are erroneous
                    // duplicates of the same user
                    if( !Players[i].XOnlineInfo.bIsGuestOfUser && !Players[j].XOnlineInfo.bIsGuestOfUser )
                    {
                        Players[i].XOnlineInfo.dwSignInError = PLAYER_XONLINE_INFO::SIGNINERROR_DUPLICATE_USER;
                    }
                    else
                    {
                        // Else, test if one player is a guest of the other.
                        if( Players[i].XOnlineInfo.bIsGuestOfUser != Players[j].XOnlineInfo.bIsGuestOfUser )
                            bGuestHasValidUser = TRUE;
                    }
                }
            }

            // For guests only, make sure a matching user was found
            if( Players[i].XOnlineInfo.bIsGuestOfUser )
            {
                if( FALSE == bGuestHasValidUser )
                    Players[i].XOnlineInfo.dwSignInError = PLAYER_XONLINE_INFO::SIGNINERROR_GUEST_HAS_NO_VALID_USER;
            }

            // If the above code detected no errors, the player is ready to sign in
            if( 0 == Players[i].XOnlineInfo.dwSignInError )
                dwNumPlayersReadyToSignIn++;
        }
    }

    // Mark convenient global flags based on the above tests
    g_bReadyToSignIn      = FALSE;
    g_bAllPlayersSignedIn = FALSE;
    g_bPlayersCanJoin     = TRUE;

    if( g_dwNumActivePlayers > 0 )
    {
        // Check if all active players are ready to sign in
        if( dwNumPlayersReadyToSignIn == g_dwNumActivePlayers )
            g_bReadyToSignIn = TRUE;

        // Check if all active players are currently signed in
        if( dwNumPlayersSignedIn == g_dwNumActivePlayers )
            g_bAllPlayersSignedIn = TRUE;

        // Check condition for whether we still let other players join
        if( dwNumPlayersSigningIn + dwNumPlayersSignedIn == g_dwNumActivePlayers )
            g_bPlayersCanJoin = FALSE;
    }
}




//-----------------------------------------------------------------------------
// Name: Render_LiveSignInScreen()
// Desc: 
//-----------------------------------------------------------------------------
VOID Render_LiveSignInScreen( FLOAT ViewportStartX, FLOAT ViewportStartY )
{
    // Draw the player screens
    for( DWORD i=0; i<MAX_LOCAL_PLAYERS; i++ )
    {
        // Render the screen
        g_PlayerScreens[i].Render( ViewportStartX, ViewportStartY );
    }
}




static VOID CreateSignInMenu( MENU_HEADER* pMenuHeader, MENU* pMenu );
static BOOL HandleSignInMenu( MENU_HEADER* pMenuHeader, DWORD dwInput );
static VOID DrawSignInMenu( MENU_HEADER* pMenuHeader, MENU* pMenu );

//-----------------------------------------------------------------------------
// SignIn menu
//-----------------------------------------------------------------------------
extern MENU Menu_SignIn = 
{
    TEXT_LIVESIGNIN,
    MENU_DEFAULT | MENU_NOBOX,              // Menu type
    CreateSignInMenu,                       // Create menu function
    HandleSignInMenu,                       // Input handler function
    DrawSignInMenu,                         // Menu draw function
    0,                                      // X coord
    0,                                      // Y Coord
};

// Create Function
VOID CreateSignInMenu( MENU_HEADER* pMenuHeader, MENU* pMenu )
{
    pMenu->fWidth  = 300.0f;
    pMenu->fHeight = 195.0f;
}

// Menu input handler
BOOL HandleSignInMenu( MENU_HEADER* pMenuHeader, DWORD dwInput )
{
    return FALSE;
}

VOID DrawSignInMenu( MENU_HEADER* pMenuHeader, MENU* pMenu )
{
    // Render the screen
    Render_LiveSignInScreen( pMenuHeader->m_XPos, pMenuHeader->m_YPos );
}




//-----------------------------------------------------------------------------
// Name: HandleEnter** / HandleExit**  functions
// Desc: Init/uninit work necessary when entering/exiting state engine.
//-----------------------------------------------------------------------------
VOID CLiveSignInStateEngine::HandleEnterFromParent()  { InitNetwork();  XOnlineStartup(NULL); }
VOID CLiveSignInStateEngine::HandleExitToParent()     { XOnlineCleanup();  KillNetwork(); }




//-----------------------------------------------------------------------------
// Name: LiveSignIn()
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CLiveSignInStateEngine::Process()
{
    enum 
    {
        LIVESIGNIN_STATE_BEGIN = STATEENGINE_STATE_BEGIN,
        LIVESIGNIN_STATE_MAINLOOP,
        LIVESIGNIN_STATE_EXITFORWARD,
        LIVESIGNIN_STATE_EXITBACK,
    };

    switch( m_State )
    {
        case LIVESIGNIN_STATE_BEGIN:
        {
            // Kill the current menu
            g_pMenuHeader->SetNextMenu( &Menu_SignIn );
        
            // Get the online user list, which is used by the player screens
            HRESULT hr = XBOnline_GetUserList( &g_XOnlineUserList, &g_dwNumUsers );

            // Initialize the player screens used to handle input for
            // users trying to sign in
            for( DWORD i=0; i<MAX_LOCAL_PLAYERS; i++ )
            {
                // $MD: consumber beta hack: Menus were too small in german
                FLOAT x = (640-400)/2;
                FLOAT y = (480-200)/2;
                

                if( MAX_LOCAL_PLAYERS > 1 )
                {
                    switch(i)
                    {
                        case 0: x =  48; y =  56; break;
                        case 1: x = 320; y =  56; break;
                        case 2: x =  48; y = 240; break;
                        case 3: x = 320; y = 240; break;
                    }
                }
                g_PlayerScreens[i].Initialize( x, y, &Players[i].XOnlineInfo, &g_Controllers[i] );
            }

            // SignIn is neither terminated (user backed out) nor completed (all
            // players signed in)
            g_bSignInTerminated = FALSE;
            g_bSignInCompleted  = FALSE;

            // Advance to the next state
            m_State = LIVESIGNIN_STATE_MAINLOOP;

            break;
        }

        case LIVESIGNIN_STATE_MAINLOOP:
        {
            // Check the connection
            if( XBOnline_IsActive() == FALSE )
            {
                // We lost our connection
            }

            // Check player input
            for( DWORD i=0; i<MAX_LOCAL_PLAYERS; i++ )
            {
                g_PlayerScreens[i].Process();

                if( g_bSignInTerminated )
                {
                    // Select the state to animate the menu away
                    return GotoState( LIVESIGNIN_STATE_EXITBACK );
                }

                if( g_bSignInCompleted )
                {
                    // Remember what controller we signed in on
                    //$REVISIT: This was wrong. We now set 
                    // g_dwSignedInController to be the last controller
                    // to hit A or START up until signing in (see
                    // input.cpp)
                    // g_dwSignedInController = i;
                    return GotoState( LIVESIGNIN_STATE_EXITFORWARD );
                }
            }

            // Check conditions for before, during, or after sign in
            CheckSignInConditions();

            break;
        }

        case LIVESIGNIN_STATE_EXITFORWARD:
        {
            if( FAILED( g_FriendsManager.Initialize() ) )
                DumpMessage( "Warning", "Couldn't initialize friends manager" );

            //$HACK: We're always telling the online APIs the user signed in on controller 0.
            AddOnlinePresenceFlag( 0, XONLINE_FRIENDSTATE_FLAG_ONLINE );

            // Now that we're signed in, check the voice bit in the player's
            // XUID and see if we can enable voice
            assert( Players[0].XOnlineInfo.pXOnlineUser != NULL );
            if( XOnlineIsUserVoiceAllowed( Players[0].XOnlineInfo.pXOnlineUser->xuid.dwUserFlags ) )
                g_VoiceManager.EnableCommunicator( g_dwSignedInController, TRUE );

            Return( STATEENGINE_COMPLETED );
            break;
        }

        case LIVESIGNIN_STATE_EXITBACK:
        {
            // We're finished. Reset the state and exit
            Return( STATEENGINE_TERMINATED );
            break;
        }
    }

    // Handle the menus
    g_pMenuHeader->HandleMenus();

    return S_FALSE;
}




//-----------------------------------------------------------------------------
// Name: SignOut()
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CLiveSignInStateEngine::SignOut()
{
    //$TODO: Should we be displaying any confirmation UI here?

    //$HACK: We're always telling the online APIs the user signed in on controller 0.
    RemoveOnlinePresenceFlag( 0, XONLINE_FRIENDSTATE_FLAG_ONLINE );

    XBOnline_SignOut();

    for( DWORD i=0; i<MAX_LOCAL_PLAYERS; i++ )
        g_PlayerScreens[i].SignOut();

    // Disable voice for the player who signed out
    g_VoiceManager.EnableCommunicator( g_dwSignedInController, FALSE );

    g_FriendsManager.Shutdown();

    // Update global status flags
    CheckSignInConditions();

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: PlayersSignedIn()
// Desc: 
//-----------------------------------------------------------------------------
BOOL CLiveSignInStateEngine::PlayersSignedIn()
{
    return g_bAllPlayersSignedIn;
}




//-----------------------------------------------------------------------------
// Name: Initialize()
// Desc: Initializes a player screen
//-----------------------------------------------------------------------------
VOID CPlayerScreenStateEngine::Initialize( FLOAT x, FLOAT y, PLAYER_XONLINE_INFO* pPlayer, 
                                           XBINPUT_CONTROLLER* pController )
{
    // Initialize member variables
    m_x             = x;
    m_y             = y;
    m_pController   = pController;
    m_pPlayer       = pPlayer;
    m_dwCurrentUser = 0;

    // Init the pass code
    ZeroMemory( &m_pPlayer->PassCode, sizeof(m_pPlayer->PassCode) );

    // Start with the "Press A to Sign In" screen.
    m_State = STATE_WAITING_FOR_PLAYER;
}




//-----------------------------------------------------------------------------
// Name: Process()
// Desc: Process input for the player screen
//-----------------------------------------------------------------------------
HRESULT CPlayerScreenStateEngine::Process()
{
#ifdef SINGLE_PLAYER_UI
    // Do a lame hack for single player UI's to combine all inputs together
    DWORD InputEvent = g_Controllers[0].dwMenuInput!=XBINPUT_NONE ? g_Controllers[0].dwMenuInput :
                       g_Controllers[1].dwMenuInput!=XBINPUT_NONE ? g_Controllers[1].dwMenuInput :
                       g_Controllers[2].dwMenuInput!=XBINPUT_NONE ? g_Controllers[2].dwMenuInput :
                       g_Controllers[3].dwMenuInput!=XBINPUT_NONE ? g_Controllers[3].dwMenuInput : 
                       XBINPUT_NONE;
#else
    DWORD InputEvent = m_pController->dwMenuInput;
#endif

    switch( m_State )
    {
        case STATE_WAITING_FOR_PLAYER:
            return UpdateWaitingForPlayerScreen( InputEvent );
        
        case STATE_SELECTING_ACCOUNT:
            return UpdateSelectAccountScreen( InputEvent );

        case STATE_REBOOT_TO_ONLINE_DASH:
            return UpdateRebootToOnlineDashScreen( InputEvent );

        case STATE_ENTERING_PASS_CODE:
            return UpdateEnterPassCodeScreen( InputEvent );

        case STATE_WAITING_TO_SIGNIN:
            return UpdateWaitingForOtherUsers( InputEvent );

        case STATE_SIGNING_IN:
            return UpdateSigningInScreen( InputEvent );

        case STATE_SIGNIN_FAILED:
            return UpdateSignInFailed( InputEvent );

        case STATE_SIGNIN_SUCCEEDED:
            m_State = STATE_UPDATE_REQUIRED_CONTENT;
            //m_State = STATE_UPDATE_SUCCEEDED; break;// uncomment to skip updates
        
        //$MD
        case STATE_UPDATE_REQUIRED_CONTENT:
            // BUGBUG: This isn't going to work when we move to split screen logon.
            // At that point we can silently start the update process and move
            // to a update progress screen after everyone is logged on or figure
            // out some other UI
#ifndef SINGLE_PLAYER_UI
            assert( FALSE );
#endif

            // download required content
            g_ContentManager.Init();
            g_ContentManager.BeginEnum( CONTENT_MANDATORY_FLAG, XONLINE_OFFERING_CONTENT, FALSE );
            m_State = STATE_REQUIRED_CONTENT_ENUM;
            // fall throught

        case STATE_REQUIRED_CONTENT_ENUM:
            switch( InputEvent )
            {
                case XBINPUT_BACK_BUTTON:
                case XBINPUT_B_BUTTON:
                {
                    // sign out
                    XBOnline_SignOut();
                    g_bSignInTerminated = TRUE;
                    m_pPlayer->bIsLoggedIn = FALSE;

                    // cleanup download
                    g_ContentManager.CleanUp();
                    return S_OK;
                }
            }

            // udate enum progress
            g_ContentManager.UpdateEnum();

            // an update is required, move to content download
            // engine
            // NOTE: UpdateRequired is true after the first peice
            //       of new content is detected.  Any more content
            //       enumeration is done inside of the contentdownload
            //       engine.  This was done in order to keep the sign
            //       on process from showing the download menu even
            //       if no new content is available
            if(g_ContentManager.GetNumInfos() != 0 || 
               g_ContentManager.Error())
            {
                m_State = STATE_REQUIRED_CONTENT_UPDATED;
                g_pActiveStateEngine->Call(&g_RequiredDownloadEngine);
                return S_OK;
            }
            
            // requred, signon is complete
            if(!g_ContentManager.Working() )
            {
                m_State = STATE_UPDATE_SUCCEEDED;
                g_ContentManager.CleanUp();
            }
            return S_OK;

        case STATE_REQUIRED_CONTENT_UPDATED:
            // if the required content was not updated, back out
            if( STATEENGINE_TERMINATED == g_RequiredDownloadEngine.GetStatus() )
            {
                // sign out
                XBOnline_SignOut();
                g_bSignInTerminated = TRUE;
                m_pPlayer->bIsLoggedIn = FALSE;
                return S_OK;
            }

            // update succeded
            m_State = STATE_UPDATE_SUCCEEDED;
            // fallthrough
                
        case STATE_UPDATE_SUCCEEDED:
            //$MD: change this to UpdateContentSucceeded?
            return UpdateSignInSucceeded( InputEvent );
    }

    // We should not get here
    return E_FAIL;
}




//-----------------------------------------------------------------------------
// Name: UpdateWaitingForPlayerScreen()
// Desc: Waiting for a player to come along
//-----------------------------------------------------------------------------
HRESULT CPlayerScreenStateEngine::UpdateWaitingForPlayerScreen( DWORD InputEvent )
{
    m_pPlayer->pXOnlineUser = NULL;
    m_pPlayer->bIsLoggedIn = FALSE;

#ifdef SINGLE_PLAYER_UI
    // With only one player, we don't need to wait for players to join, so just
    // advance immediately to the next appropriate screen
    if( g_dwNumUsers > 0 )
        m_State = STATE_SELECTING_ACCOUNT;
    else
        m_State = STATE_REBOOT_TO_ONLINE_DASH;
    return Process();
#endif


    switch( InputEvent )
    {
        // Handle case when user presses the BACK or B button
        case XBINPUT_BACK_BUTTON:
        case XBINPUT_B_BUTTON:
        {
            // Select the state to animate the menu away
            g_bSignInTerminated = TRUE;
            break;
        }

        // Handle case when user presses the START or A button
        case XBINPUT_START_BUTTON:
        case XBINPUT_A_BUTTON:
        {
            if( g_bPlayersCanJoin )
            {
                // If players are allowed to join, take the player to the
                // account selection screen
                if( g_dwNumUsers > 0 )
                    m_State = STATE_SELECTING_ACCOUNT;
                else
                    m_State = STATE_REBOOT_TO_ONLINE_DASH;
            }
            break;
        }
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: UpdateSelectAccountScreen()
// Desc: Allow player to choose account
//-----------------------------------------------------------------------------
HRESULT CPlayerScreenStateEngine::UpdateSelectAccountScreen( DWORD InputEvent )
{
    m_pPlayer->pXOnlineUser = NULL;
    m_pPlayer->bIsLoggedIn = FALSE;

    switch( InputEvent )
    {
        // Handle case when user presses the BACK or B button
        case XBINPUT_BACK_BUTTON:
        case XBINPUT_B_BUTTON:
        {
#ifdef SINGLE_PLAYER_UI
            // With only one player, we skipped the initial screen, so
            // pressing B here terminates the sign in process.
            g_bSignInTerminated = TRUE;
#else
            // Go back to the "Press A to Sign In" screen.
            m_State = STATE_WAITING_FOR_PLAYER;
#endif
            break;
        }

        // Handle case when user presses the START or A button
        case XBINPUT_START_BUTTON:
        case XBINPUT_A_BUTTON:
        {
            // Sign in as the selected user
            m_pPlayer->pXOnlineUser = &g_XOnlineUserList[m_dwCurrentUser];
            m_pPlayer->bIsGuestOfUser = FALSE;

            // Copy the user name (converting to a wide string )
            swprintf( m_pPlayer->wstrName, L"%S", m_pPlayer->pXOnlineUser->szGamertag );

            // Clear the passcode
            ZeroMemory( &m_pPlayer->PassCode, sizeof(m_pPlayer->PassCode) );

            // Advance state, depending on whether the user needs to enter his pass code
            if( m_pPlayer->pXOnlineUser->dwUserOptions & XONLINE_USER_OPTION_REQUIRE_PASSCODE )
            {
                // Switch state to enter the pass code
                m_State = STATE_ENTERING_PASS_CODE;
                m_bEnteredWrongPassCode = FALSE;

                // Temporarily disallow thumbstick movement to control pass code entry
                g_bMapThumbstickToDpadControls = FALSE;
            }
            else
            {
                // Switch state to wait for other players
                m_State = STATE_WAITING_TO_SIGNIN;
            }
            break;
        }
        
        // Handle case when user presses the X button
        case XBINPUT_X_BUTTON:
        {
            // Sign in as a guest
#ifdef SINGLE_PLAYER_UI
            // With one player, it's not possible to sign in as a guest, so do not
            // handle the X button
            break;
#endif

            // Sign in as a guest of the selected user
            m_pPlayer->pXOnlineUser = &g_XOnlineUserList[m_dwCurrentUser];
            m_pPlayer->bIsGuestOfUser = TRUE;

            // Copy the user name (converting to a wide string )
            swprintf( m_pPlayer->wstrName, TEXT_TABLE(TEXT_LIVESIGNIN_GUESTOFPLAYER), m_pPlayer->pXOnlineUser->szGamertag );

            // Switch state to wait for other players
            m_State = STATE_WAITING_TO_SIGNIN;
            break;
        }

        // Handle case when user presses the X button
        case XBINPUT_Y_BUTTON:
        {
            // Create a new account
            m_State = STATE_REBOOT_TO_ONLINE_DASH;
            break;
        }
        
        // Handle case when user presses up on the gamepad
        case XBINPUT_UP:
        {
            // Move to previous user account
            if( m_dwCurrentUser > 0 )
                m_dwCurrentUser--;
            break;
        }
        
        // Handle case when user presses down on the gamepad
        case XBINPUT_DOWN:
        {
            // Move to next user account
            if( m_dwCurrentUser+1 < g_dwNumUsers )
                m_dwCurrentUser++;
            break;
        }
        
        default:
        {
            // If any MUs are inserted, update the user list and go to account
            // selection if there are any new online accounts
            DWORD dwInsertions;
            DWORD dwRemovals;
            if( TRUE == XGetDeviceChanges( XDEVICE_TYPE_MEMORY_UNIT, &dwInsertions, &dwRemovals ) )
            {
                ULONGLONG qwOldUserID = g_XOnlineUserList[m_dwCurrentUser].xuid.qwUserID;

                // Get the latest online user list
                XBOnline_GetUserList( &g_XOnlineUserList, &g_dwNumUsers );

                // Try to keep the selection on the same user
                m_dwCurrentUser = 0;
                for( DWORD i=0; i< g_dwNumUsers; i++ )
                {
                    if( qwOldUserID == g_XOnlineUserList[i].xuid.qwUserID )
                        m_dwCurrentUser = i;
                }

/*
                CHAR strOldGamerTag[XONLINE_GAMERTAG_SIZE];
                strcpy( strOldGamerTag, g_XOnlineUserList[m_dwCurrentUser].szGamertag );
                
                // Get the latest online user list
                XBOnline_GetUserList( &g_XOnlineUserList, &g_dwNumUsers );

                // Try to keep the selection on the same user
                m_dwCurrentUser = 0;
                for( DWORD i=0; i< g_dwNumUsers; i++ )
                {
                    if( 0 == strcmp( strOldGamerTag, g_XOnlineUserList[i].szGamertag ) )
                    {
                        m_dwCurrentUser = i;
                    }
                }
*/
            }
            break;
        }
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: UpdateRebootToOnlineDashScreen()
// Desc: Allow player to launch account management tool
//-----------------------------------------------------------------------------
HRESULT CPlayerScreenStateEngine::UpdateRebootToOnlineDashScreen( DWORD InputEvent )
{
    m_pPlayer->pXOnlineUser = NULL;

    switch( InputEvent )
    {
        // Handle case when user presses the BACK or B button
        case XBINPUT_BACK_BUTTON:
        case XBINPUT_B_BUTTON:
        {
            // If there are user accounts to select from, go back to the state
            // for selecting accounts. Otherwise, go back to the initial state.
            if( g_dwNumUsers )
            {
                // Go back to the state for selecting accounts
                m_State = STATE_SELECTING_ACCOUNT;
            }
            else
            {
#ifdef SINGLE_PLAYER_UI
                // With only one player, we skipped the "Press A to Sign In"
                // screen, so pressing B here terminates the sign in process.
                g_bSignInTerminated = TRUE;
#else
                // Go back to the state for waiting for a player to sign in
                m_State = STATE_WAITING_TO_SIGNIN;
#endif
            }
            break;
        }

        // Handle case when user presses the START or A button
        case XBINPUT_START_BUTTON:
        case XBINPUT_A_BUTTON:
        {
            // Return to Dashboard. Retail Dashboard will include
            // online account creation. Development XDK Launcher
            // includes the Xbox Online Setup Tool for creating accounts.
            LD_LAUNCH_DASHBOARD ld;
            ZeroMemory( &ld, sizeof(ld) );
            ld.dwReason = XLD_LAUNCH_DASHBOARD_NEW_ACCOUNT_SIGNUP;
            XLaunchNewImage( NULL, PLAUNCH_DATA( &ld ) );
            break;
        }

        default:
        {
            // If any MUs are inserted, update the user list and go to account
            // selection if there are any new online accounts
            DWORD dwInsertions;
            DWORD dwRemovals;
            if( TRUE == XGetDeviceChanges( XDEVICE_TYPE_MEMORY_UNIT, &dwInsertions, &dwRemovals ) )
            {
                if( dwInsertions > 0 )
                {
                    DWORD dwOldNumUsers = g_dwNumUsers;

                    // Get the latest online user list
                    XBOnline_GetUserList( &g_XOnlineUserList, &g_dwNumUsers );
                    
                    if( g_dwNumUsers > dwOldNumUsers )
                        m_State = STATE_SELECTING_ACCOUNT;
                }
            }
            break;
        }
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: UpdateEnterPassCodeScreen()
// Desc: Allow player to enter Passcode number
//-----------------------------------------------------------------------------
HRESULT CPlayerScreenStateEngine::UpdateEnterPassCodeScreen( DWORD InputEvent )
{
    switch( InputEvent )
    {
        case XBINPUT_BACK_BUTTON:
        case XBINPUT_B_BUTTON:
        {
            // Clear last entry in Passcode
            for( int i = (XONLINE_PASSCODE_LENGTH-1) ; i >= 0 ; i-- )
            {
                if( m_pPlayer->PassCode[i] )
                {
                    m_pPlayer->PassCode[i] = 0;
                    break;
                }
            }

            // If Passcode was already empty...
            if( i < 0 )
            {
                // Unselect the user for this player
                m_pPlayer->pXOnlineUser = NULL;

                m_State = STATE_SELECTING_ACCOUNT;

                // Re-allow thumbstick movement to control menu system
                g_bMapThumbstickToDpadControls = TRUE;
            }
            break;
        }

        case XBINPUT_START_BUTTON:
        case XBINPUT_A_BUTTON:
        {
            // If the pass code is entered, proceed to the next state
            if( m_pPlayer->PassCode[XONLINE_PASSCODE_LENGTH-1] )
            {
                // Check the passcode
                if( 0 == memcmp( m_pPlayer->PassCode, m_pPlayer->pXOnlineUser->passcode, XONLINE_PASSCODE_LENGTH ) )
                {
                    m_State = STATE_WAITING_TO_SIGNIN;

                    // Re-allow thumbstick movement to control menu system
                    g_bMapThumbstickToDpadControls = TRUE;
                }
                else
                {
                    // A correct passcode was not entered. Clear the passcode
                    // set a flag to and inform the user to try again
                    m_bEnteredWrongPassCode = TRUE;

                    // Clear the passcode
                    ZeroMemory( &m_pPlayer->PassCode, sizeof(m_pPlayer->PassCode) );
                }
            }
            break;
        }

        default:
        {
            // Record the next pass code character
            for( DWORD i=0; i<XONLINE_PASSCODE_LENGTH; i++ )
            {
                if( m_pPlayer->PassCode[i] == 0 )
                {
                    switch( InputEvent )
                    {
                        case XBINPUT_UP:            m_pPlayer->PassCode[i] = XONLINE_PASSCODE_DPAD_UP; break;
                        case XBINPUT_DOWN:          m_pPlayer->PassCode[i] = XONLINE_PASSCODE_DPAD_DOWN; break;
                        case XBINPUT_LEFT:          m_pPlayer->PassCode[i] = XONLINE_PASSCODE_DPAD_LEFT; break;
                        case XBINPUT_RIGHT:         m_pPlayer->PassCode[i] = XONLINE_PASSCODE_DPAD_RIGHT; break;
                        case XBINPUT_X_BUTTON:      m_pPlayer->PassCode[i] = XONLINE_PASSCODE_GAMEPAD_X; break;
                        case XBINPUT_Y_BUTTON:      m_pPlayer->PassCode[i] = XONLINE_PASSCODE_GAMEPAD_Y; break;
#if 0
                        // Removed for bug 21783
                        case XBINPUT_BLACK_BUTTON:  m_pPlayer->PassCode[i] = XONLINE_PASSCODE_GAMEPAD_BLACK; break;
                        case XBINPUT_WHITE_BUTTON:  m_pPlayer->PassCode[i] = XONLINE_PASSCODE_GAMEPAD_WHITE; break;
#endif
                        case XBINPUT_LEFT_TRIGGER:  m_pPlayer->PassCode[i] = XONLINE_PASSCODE_GAMEPAD_LEFT_TRIGGER; break;
                        case XBINPUT_RIGHT_TRIGGER: m_pPlayer->PassCode[i] = XONLINE_PASSCODE_GAMEPAD_RIGHT_TRIGGER; break;
                    }
                    break;
                }
            }
            break;
        }
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: UpdateWaitingForOtherUsers()
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CPlayerScreenStateEngine::UpdateWaitingForOtherUsers( DWORD InputEvent )
{
#ifdef SINGLE_PLAYER_UI
    // With only one user, we don't need to wait for other users. Simulate
    // pressing the A button to skip past this step
    InputEvent = XBINPUT_A_BUTTON;
#endif

    switch( InputEvent )
    {
        case XBINPUT_BACK_BUTTON:
        case XBINPUT_B_BUTTON:
        {
            m_State = STATE_SELECTING_ACCOUNT;
            break;
        }

        case XBINPUT_START_BUTTON:
        case XBINPUT_A_BUTTON:
        {
            if( TRUE == g_bReadyToSignIn )
            {
                XONLINE_USER* pUserList[4] = { NULL, NULL, NULL, NULL };
                for( DWORD i=0 ; i < MAX_LOCAL_PLAYERS ; i++ )
                {
                    pUserList[i] = Players[i].XOnlineInfo.pXOnlineUser;
                }

#ifdef XONLINE_OFFLINE
                g_hrSignInResult = S_OK;
#else
                g_hrSignInResult = XBOnline_BeginSignIn( pUserList );
#endif

                // Advance state to sign in any waiting users
                for( DWORD i=0; i<MAX_LOCAL_PLAYERS; i++ )
                {
                    if( g_PlayerScreens[i].m_State == STATE_WAITING_TO_SIGNIN )
                    {
//                      Players[i].XOnlineInfo.pXOnlineUser->hr = g_hrSignInResult;

                        if( SUCCEEDED( g_hrSignInResult ) )
                        {
                            g_PlayerScreens[i].m_State = STATE_SIGNING_IN;
                            g_PlayerScreens[i].m_fSignInTime = 0.0f;
                        }
                        else
                            g_PlayerScreens[i].m_State = STATE_SIGNIN_FAILED;
                    }
                }
            }
            break;
        }
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: UpdateSigningInScreen()
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CPlayerScreenStateEngine::UpdateSigningInScreen( DWORD InputEvent )
{
#ifdef XONLINE_OFFLINE
    for( DWORD i=0; i<MAX_LOCAL_PLAYERS; i++ )
        if( g_PlayerScreens[i].m_State == STATE_SIGNING_IN )
            g_PlayerScreens[i].m_State = STATE_SIGNIN_SUCCEEDED;

    return S_OK;
#endif

    // Keep track of the time it's taking to sign in. We can use this info
    // to play an animation, keep stats, etc.
    m_fSignInTime += TimeStep;

    switch( InputEvent )
    {
        case XBINPUT_BACK_BUTTON:
        case XBINPUT_B_BUTTON:
        {
            // Close the task (this cancels the sign in process)
            XBOnline_SignOut();

            for( DWORD i=0; i<MAX_LOCAL_PLAYERS; i++ )
            {
                if( g_PlayerScreens[i].m_State == STATE_SIGNING_IN )
                {
#ifdef SINGLE_PLAYER_UI
                    // Send player back to the acount selection state
                    g_PlayerScreens[i].m_State = STATE_SELECTING_ACCOUNT;
#else
                    // Send all players back to the waiting to sign in state
                    g_PlayerScreens[i].m_State = STATE_WAITING_TO_SIGNIN;
#endif
                }
            }
        }
        break;

#ifdef SHIPPING
        // Don't enable this debug addition for shipping versions.
#else
        //$DEBUG ADDITION: to fake a sign in failure
        case XBINPUT_X_BUTTON:
        case XBINPUT_Y_BUTTON:
        {
            //XBOnline_SignOut();

            for( DWORD i=0; i<MAX_LOCAL_PLAYERS; i++ )
            {
                if( g_PlayerScreens[i].m_State == STATE_SIGNING_IN )
                {
                    g_PlayerScreens[i].m_State = STATE_SIGNIN_FAILED;
                    Players[i].XOnlineInfo.pXOnlineUser->hr = E_FAIL;
                }
            }
        }
        break;
#endif // SHIPPING

        default:
        {
            // Pump with the sign in task
            g_hrSignInResult = XBOnline_PumpSignInTask();
            if( FAILED(g_hrSignInResult) )
            {
                CHAR strTemp[1024];
                sprintf( strTemp, "FAILED: XBOnline_PumpSignInTask returned 0x%08X\n", g_hrSignInResult );
                OutputDebugString( strTemp );
                //$TODO: make the above debug output a no-op in shipping version!

                for( DWORD i=0; i<MAX_LOCAL_PLAYERS; i++ )
                {
                    if( g_PlayerScreens[i].m_State == STATE_SIGNING_IN )
                    {
                        g_PlayerScreens[i].m_State = STATE_SIGNIN_FAILED;
//                      Players[i].XOnlineInfo.pXOnlineUser->hr = g_hrSignInResult;
                    }
                }
            }
            else
            {
                if( g_hrSignInResult != XONLINETASK_S_RUNNING )
                {
                    // Sign in succeeded, so advance to the next state.
                    // Note: This may seem weird, but let's enforce a minimum
                    // sign in time of 1 second here, just so the UI doesn't
                    // flash through the sign in screen to0 fast.
                    if( m_fSignInTime > 1.0f )
                    {
                        // Mirror per-player sign in results to our local copy
                        memcpy( g_XOnlineUserList, XOnlineGetLogonUsers(), 4 * sizeof(XONLINE_USER) );

                        //$TODO: Revisit this for the 4-player case

                        for( DWORD i=0; i<MAX_LOCAL_PLAYERS; i++ )
                        {
                           Players[i].XOnlineInfo.pXOnlineUser = &g_XOnlineUserList[i];

                            if( g_PlayerScreens[i].m_State == STATE_SIGNING_IN )
                            {
                                if( SUCCEEDED( Players[i].XOnlineInfo.pXOnlineUser->hr ) )
                                {
                                    Players[i].XOnlineInfo.bIsLoggedIn = TRUE;
                                    g_PlayerScreens[i].m_State = STATE_SIGNIN_SUCCEEDED;
                                }
                                else
                                {
                                    Players[i].XOnlineInfo.bIsLoggedIn = FALSE;
                                    g_PlayerScreens[i].m_State = STATE_SIGNIN_FAILED;
                                }
                            }   
                        }
                    }
                }
            }
        }
        break;
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: UpdateSignInFailed()
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CPlayerScreenStateEngine::UpdateSignInFailed( DWORD InputEvent )
{
    // Logic flow for this function:
    //  if( m_pPlayer->bIsGuestOfUser )
    //      B goes back to select account
    //
    //  else if( g_hrSignInResult == XONLINE_E_LOGON_NO_NETWORK_CONNECTION )
    //      A reboots to network troublshooter
    //      B goes back to select account
    //
    //  else if( g_hrSignInResult == XONLINE_E_LOGON_UPDATE_REQUIRED )
    //      A begins update installation
    //      B goes back to main menu
    //
    //  else if( g_hrSignInResult == XONLINE_E_LOGON_CANNOT_ACCESS_SERVICE )
    //      A reboots to network troublshooter
    //      B goes back to select account
    //  
    //  else if( g_hrSignInResult == XONLINE_E_LOGON_SERVERS_TOO_BUSY )
    //      B goes back to select account
    //  
    //  else if( m_pPlayer->pXOnlineUser->hr == XONLINE_E_LOGON_USER_ACCOUNT_REQUIRES_MANAGEMENT )
    //      A reboots to online dash, acct management
    //      B goes back to select account - actually, no it doesn't anymore
    //
    //  else
    //      A reboots to network troublshooter
    //      B goes back to select account

    switch( InputEvent )
    {
        case XBINPUT_START_BUTTON:
        case XBINPUT_A_BUTTON:
        {
            if( m_pPlayer->bIsGuestOfUser )
            {
                // Do nothing
            }
            else if( g_hrSignInResult == XONLINE_E_LOGON_SERVERS_TOO_BUSY )
            {
                // Do nothing
            }
            else if( g_hrSignInResult == XONLINE_E_LOGON_UPDATE_REQUIRED )
            {
                //$TODO: Review that 0x00000000 is a valid context
                // Begin update installation
                XOnlineTitleUpdate( 0x00000000 );
            }
            else if( m_pPlayer->pXOnlineUser->hr == XONLINE_E_LOGON_USER_ACCOUNT_REQUIRES_MANAGEMENT )
            {
                // Go to acct management
                LD_LAUNCH_DASHBOARD ld;
                ZeroMemory( &ld, sizeof(ld) );
                ld.dwReason = XLD_LAUNCH_DASHBOARD_ACCOUNT_MANAGEMENT;
                OutputDebugString( "Launching the online dash...\n" );
                XLaunchNewImage( NULL, PLAUNCH_DATA( &ld ) );
            }
            else
            {
                // Go to network troubleshooter
                LD_LAUNCH_DASHBOARD ld;
                ZeroMemory( &ld, sizeof(ld) );
                ld.dwReason = XLD_LAUNCH_DASHBOARD_NETWORK_CONFIGURATION;
                OutputDebugString( "Launching the online dash...\n" );
                XLaunchNewImage( NULL, PLAUNCH_DATA( &ld ) ); 
            }

            break;
        }

        case XBINPUT_BACK_BUTTON:
        case XBINPUT_B_BUTTON:
        {
            if( g_hrSignInResult == XONLINE_E_LOGON_UPDATE_REQUIRED )
            {
                // Exit signin state engine and go back to the main menu
                g_bSignInTerminated = TRUE;
            }
            else if( m_pPlayer->pXOnlineUser->hr == XONLINE_E_LOGON_USER_ACCOUNT_REQUIRES_MANAGEMENT )
            {
                //$REVISIT: do nothing - they MUST go to the dash
            }
            else
            {
                // Go back to the select account screen
                m_State = STATE_SELECTING_ACCOUNT;
            }

            break;
        }

#ifndef SHIPPING
        case XBINPUT_X_BUTTON:
        {
            // For debugging, if sign in fails, the user can hit the X button
            // to continue as if sign in actually succeeded
            g_bAllPlayersSignedIn = TRUE;
            g_bSignInCompleted    = TRUE;
            break;
        }
#endif // SHIPPING
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: UpdateSignInSucceeded()
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CPlayerScreenStateEngine::UpdateSignInSucceeded( DWORD InputEvent )
{
    switch( InputEvent )
    {
        case XBINPUT_START_BUTTON:
        case XBINPUT_A_BUTTON:
            if( g_bAllPlayersSignedIn )
            {
                g_bSignInCompleted = TRUE;
            }
            break;

        case XBINPUT_Y_BUTTON:
            if( m_pPlayer->pXOnlineUser->hr == XONLINE_S_LOGON_USER_HAS_MESSAGE )
            {
                // Go to the dash to view messages
                LD_LAUNCH_DASHBOARD ld;
                ZeroMemory( &ld, sizeof(ld) );
                ld.dwReason = XLD_LAUNCH_DASHBOARD_ACCOUNT_MANAGEMENT;
                OutputDebugString( "Launching the online dash...\n" );
                XLaunchNewImage( NULL, PLAUNCH_DATA( &ld ) ); 
            }
            break;
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: Render()
// Desc: 
//-----------------------------------------------------------------------------
VOID CPlayerScreenStateEngine::Render( FLOAT ViewportStartX, FLOAT ViewportStartY )
{
    FLOAT sx = floorf( m_x + ViewportStartX );
    FLOAT sy = floorf( m_y + ViewportStartY );
    FLOAT w = 400.0f; FLOAT h  = 200.0f;
    
    DrawNewSpruBox( sx, sy, w, h );

    // $MD: consumer beta hack.  the menus were too small
    if(m_State == STATE_SIGNIN_FAILED )
        sx += 0;
    else
        sx += 55;
    
    switch( m_State )
    {
        case STATE_WAITING_FOR_PLAYER:
            RenderWaitingForPlayerScreen( sx, sy );
            break;

        case STATE_REBOOT_TO_ONLINE_DASH:
            RenderRebootToOnlineDashScreen( sx, sy );
            break;

        case STATE_SELECTING_ACCOUNT:
            RenderSelectAccountScreen( sx, sy );
            break;

        case STATE_ENTERING_PASS_CODE:
            RenderEnterPassCodeScreen( sx, sy );
            break;

        case STATE_WAITING_TO_SIGNIN:
            RenderWaitingForOtherUsers( sx, sy );
            break;

        case STATE_SIGNING_IN:
        case STATE_SIGNIN_SUCCEEDED:
        case STATE_REQUIRED_CONTENT_ENUM:
            RenderSigningInScreen( sx, sy );
            break;

        case STATE_SIGNIN_FAILED:
            RenderSignInFailed( sx, sy );
            break;

        case STATE_UPDATE_SUCCEEDED:
            //$MD should this be RenderUpdateSucceded?
            RenderSignInSucceeded( sx, sy );
            break;
    }
}
        

        
        
//-----------------------------------------------------------------------------
// Name: RenderWaitingForPlayerScreen()
// Desc: 
//-----------------------------------------------------------------------------
VOID CPlayerScreenStateEngine::RenderWaitingForPlayerScreen( FLOAT sx, FLOAT sy )
{
    if( g_bPlayersCanJoin )
    {
        g_pFont->DrawText( sx + 150, sy + 80, MENU_TEXT_RGB_NORMAL, 
                           TEXT_TABLE(TEXT_LIVESIGNIN_PRESS_A_TOSIGNIN), XBFONT_CENTER_X );
    }
}




//-----------------------------------------------------------------------------
// Name: RenderSelectAccountScreen()
// Desc: 
//-----------------------------------------------------------------------------
VOID CPlayerScreenStateEngine::RenderSelectAccountScreen( FLOAT sx, FLOAT sy )
{
    DrawNewSpruBox( sx+38, sy+40, 220, 122 );
    g_pFont->DrawText( sx + 150, sy + 10, MENU_TEXT_RGB_NORMAL, 
                       TEXT_TABLE(TEXT_LIVESIGNIN_SELECTACCOUNT), XBFONT_CENTER_X );

    // Draw the list of available users
    for( DWORD user = 0; user < g_dwNumUsers; user++ )
    {
        long row = user;

        if( g_dwNumUsers > 5 && m_dwCurrentUser > 2 )
        {
            if( g_dwNumUsers - m_dwCurrentUser <= 2 )
                row = user + 5 - g_dwNumUsers;
            else
                row = 2 + user - m_dwCurrentUser;
        }
        if( row < 0 ||  row >= 5 )
            continue;

        // Convert name to a wide string for displaying
        WCHAR strUser[XONLINE_GAMERTAG_SIZE];
        swprintf( strUser, L"%S", g_XOnlineUserList[user].szGamertag );

        // Draw the text
        DWORD color = ( user == m_dwCurrentUser ) ? MENU_TEXT_RGB_HILITE : MENU_TEXT_RGB_NORMAL;
        g_pFont->DrawText( sx + 54, sy + 48 + row*20, color, strUser, XBFONT_TRUNCATED, 200 );
    }

    // Draw arrows to indicate more users are available by scrolling
    if( g_dwNumUsers > 5 )
    {
        static WCHAR strUpArrowString[2]   = { 0x25b2, 0x0000 };
        static WCHAR strDownArrowString[2] = { 0x25bc, 0x0000 };

        if( m_dwCurrentUser > 2 )
            g_pFont->DrawText( sx + 231, sy + 38, 0xffebebeb, strUpArrowString );

        if( m_dwCurrentUser + 2 < g_dwNumUsers - 1 )
            g_pFont->DrawText( sx + 231, sy + 136, 0xffebebeb, strDownArrowString );
    }

    // Draw button options at bottom of screen
#ifdef SINGLE_PLAYER_UI
    // Note: with one player, it's not possible to sign in as a guest
    g_pFont->DrawText( sx + 16, sy + 166, MENU_TEXT_RGB_NORMAL, L"\203 " );
    g_pFont->DrawText( MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_LIVESIGNIN_CREATEACCOUNT) );
#else
    g_pFont->DrawText( sx + 16, sy + 146, MENU_TEXT_RGB_NORMAL, L"\202 " );
    g_pFont->DrawText( MENU_TEXT_RGB_NORMAL, 
                       TEXT_TABLE(TEXT_LIVESIGNIN_SIGNINASGUEST) );
    g_pFont->DrawText( sx + 16, sy + 166, MENU_TEXT_RGB_NORMAL, L"\203 " );
    g_pFont->DrawText( MENU_TEXT_RGB_NORMAL, 
                       TEXT_TABLE(TEXT_LIVESIGNIN_CREATEACCOUNT) );
#endif
}




//-----------------------------------------------------------------------------
// Name: RenderRebootToOnlineDashScreen()
// Desc: 
//-----------------------------------------------------------------------------
VOID CPlayerScreenStateEngine::RenderRebootToOnlineDashScreen( FLOAT sx, FLOAT sy )
{
    if( g_dwNumUsers == 0 )
    {
        g_pFont->DrawText( sx + 18, sy + 8, MENU_TEXT_RGB_NORMAL, 
                           TEXT_TABLE(TEXT_LIVESIGNIN_ERROR_NOACCOUNTS) );
        g_pFont->DrawText( sx + 18, sy + 63, MENU_TEXT_RGB_NORMAL, 
                           TEXT_TABLE(TEXT_LIVESIGNIN_DOYOUWANTTOSTARTTHEDASH) );
    }
    else
    {
        g_pFont->DrawText( sx + 18, sy + 28, MENU_TEXT_RGB_NORMAL, 
                           TEXT_TABLE(TEXT_LIVESIGNIN_DOYOUWANTTOSTARTTHEDASH) );
    }

    
    g_pFont->DrawText( sx + 16, sy + 146, MENU_TEXT_RGB_NORMAL, L"\200 " );
    g_pFont->DrawText( MENU_TEXT_RGB_NORMAL, 
                       TEXT_TABLE(TEXT_LIVESIGNIN_GOTOONLINEDASHBOARD) );
    g_pFont->DrawText( sx + 16, sy + 166, MENU_TEXT_RGB_NORMAL, L"\201 " );
    g_pFont->DrawText( MENU_TEXT_RGB_NORMAL, 
                       TEXT_TABLE(TEXT_BACK) );
}




//-----------------------------------------------------------------------------
// Name: RenderEnterPassCodeScreen()
// Desc: 
//-----------------------------------------------------------------------------
VOID CPlayerScreenStateEngine::RenderEnterPassCodeScreen( FLOAT sx, FLOAT sy )
{
    g_pFont->DrawText( sx + 150, sy + 20, MENU_TEXT_RGB_NORMAL, 
                       m_pPlayer->wstrName, XBFONT_CENTER_X );

    if( FALSE == m_bEnteredWrongPassCode )
    {
        g_pFont->DrawText( sx + 150, sy + 60, MENU_TEXT_RGB_NORMAL, 
                           TEXT_TABLE(TEXT_LIVESIGNIN_ENTERPASSCODE), XBFONT_CENTER_X );
    }
    else
    {
        g_pFont->DrawText( sx + 150, sy + 50, MENU_TEXT_RGB_NORMAL, 
                           TEXT_TABLE(TEXT_LIVESIGNIN_WARNING_REENTERPASSCODE), 
                           XBFONT_CENTER_X );
    }

    for( DWORD j=0; j<XONLINE_PASSCODE_LENGTH; j++ )
    {
        if( m_pPlayer->PassCode[j] )
        {
            g_pFont->DrawText( sx + 150 - 30 + j*20, sy + 100, 
                               MENU_TEXT_RGB_NORMAL, L"\401", XBFONT_CENTER_X );
        }
        else
        {
            g_pFont->DrawText( sx + 150 - 30 + j*20, sy + 100, 
                               MENU_TEXT_RGB_NORMAL, L"\400", XBFONT_CENTER_X );
        }
    }
    
    if( m_pPlayer->PassCode[XONLINE_PASSCODE_LENGTH-1] )
    {
        g_pFont->DrawText( sx + 16, sy + 146, MENU_TEXT_RGB_NORMAL, L"\200 " );
        g_pFont->DrawText( MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_ACCEPT) );
    }
    g_pFont->DrawText( sx + 16, sy + 166, MENU_TEXT_RGB_NORMAL, L"\201 " );
    g_pFont->DrawText( MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_BACK) );
}




//-----------------------------------------------------------------------------
// Name: RenderWaitingForOtherUsers()
// Desc: 
//-----------------------------------------------------------------------------
VOID CPlayerScreenStateEngine::RenderWaitingForOtherUsers( FLOAT sx, FLOAT sy )
{
#ifdef SINGLE_PLAYER_UI
    // With only one player, this screen will be short-circuited
    return;
#endif

    g_pFont->DrawText( sx + 150, sy + 10, MENU_TEXT_RGB_NORMAL, 
                       m_pPlayer->wstrName, XBFONT_CENTER_X );

    switch( m_pPlayer->dwSignInError )
    {
        case PLAYER_XONLINE_INFO::SIGNINERROR_DUPLICATE_USER:
            g_pFont->DrawText( sx + 150, sy + 50, MENU_TEXT_RGB_NORMAL, 
                               TEXT_TABLE(TEXT_LIVESIGNIN_WARNING_DUPLICATEPLAYER),
                               XBFONT_CENTER_X );
            break;

        case PLAYER_XONLINE_INFO::SIGNINERROR_GUEST_HAS_NO_VALID_USER:
            g_pFont->DrawText( sx + 150, sy + 50, MENU_TEXT_RGB_NORMAL, 
                               TEXT_TABLE(TEXT_LIVESIGNIN_WARNING_NOHOSTFORGUEST),
                               XBFONT_CENTER_X );
            break;
        
        default:
            g_pFont->DrawText( sx + 150, sy + 50, MENU_TEXT_RGB_NORMAL, 
                               TEXT_TABLE(TEXT_LIVESIGNIN_WAITINGFOROTHERPLAYERS),
                               XBFONT_CENTER_X );
    }
    
    if( TRUE == g_bReadyToSignIn )
    {
        g_pFont->DrawText( sx + 16, sy + 146, MENU_TEXT_RGB_NORMAL, L"\200 " );
        g_pFont->DrawText( MENU_TEXT_RGB_NORMAL, 
                           TEXT_TABLE(TEXT_LIVESIGNIN_NOMOREUSERS) );
    }

    g_pFont->DrawText( sx + 16, sy + 166, MENU_TEXT_RGB_NORMAL, L"\201 " );
    g_pFont->DrawText( MENU_TEXT_RGB_NORMAL,
                       TEXT_TABLE(TEXT_BACK) );
}




//-----------------------------------------------------------------------------
// Name: RenderSigningInScreen()
// Desc: 
//-----------------------------------------------------------------------------
VOID CPlayerScreenStateEngine::RenderSigningInScreen( FLOAT sx, FLOAT sy )
{
    g_pFont->DrawText( sx + 150, sy + 20, MENU_TEXT_RGB_NORMAL, 
                       m_pPlayer->wstrName, XBFONT_CENTER_X );
    
#ifdef SINGLE_PLAYER_UI 
    g_pFont->DrawText( sx + 150, sy + 50, MENU_TEXT_RGB_NORMAL, 
                       TEXT_TABLE(TEXT_LIVESIGNIN_SINGLEPLAYER_SIGNIN),
                       XBFONT_CENTER_X );
#else
    g_pFont->DrawText( sx + 150, sy + 50, MENU_TEXT_RGB_NORMAL, 
                       TEXT_TABLE(TEXT_LIVESIGNIN_MULTIPLAYER_SIGNIN),
                       XBFONT_CENTER_X );
#endif
    
    g_pFont->DrawText( sx + 16, sy + 166, MENU_TEXT_RGB_NORMAL, L"\201 " );
    g_pFont->DrawText( MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_CANCEL) );
}




//-----------------------------------------------------------------------------
// Name: RenderSignInFailed()
// Desc: 
//-----------------------------------------------------------------------------
VOID CPlayerScreenStateEngine::RenderSignInFailed( FLOAT sx, FLOAT sy )
{
    g_pFont->DrawText( sx + 150, sy + 20, MENU_TEXT_RGB_NORMAL, 
                       TEXT_TABLE(TEXT_LIVESIGNIN_SIGNINFAILED),
                       XBFONT_CENTER_X );
    
    if( m_pPlayer->bIsGuestOfUser )
    {
        g_pFont->DrawText( sx + 150, sy + 50, MENU_TEXT_RGB_NORMAL, 
                           TEXT_TABLE(TEXT_LIVESIGNIN_ERROR_HOSTHASERRORS),
                           XBFONT_CENTER_X );

        g_pFont->DrawText( sx + 16, sy + 166, MENU_TEXT_RGB_NORMAL, L"\201 " );
        g_pFont->DrawText( MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_LIVESIGNIN_CHANGEACCOUNT) );
    }
    else
    {
        if( g_hrSignInResult == XONLINE_E_LOGON_NO_NETWORK_CONNECTION )
        {
            g_pFont->DrawText( sx + 150, sy + 50, MENU_TEXT_RGB_NORMAL,
                               TEXT_TABLE(TEXT_LIVESIGNIN_ERROR_NONETWORKCONNECTION),
                               XBFONT_CENTER_X );

            g_pFont->DrawText( sx + 16, sy + 146, MENU_TEXT_RGB_NORMAL, L"\200 " );
            g_pFont->DrawText( MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_LIVESIGNIN_NETWORKTROUBLESHOOTER)  );
            g_pFont->DrawText( sx + 16, sy + 166, MENU_TEXT_RGB_NORMAL, L"\201 " );
            g_pFont->DrawText( MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_BACK) );
        }
        else if( g_hrSignInResult == XONLINE_E_LOGON_UPDATE_REQUIRED )
        {
            g_pFont->DrawText( sx + 150, sy + 50, MENU_TEXT_RGB_NORMAL,
                               TEXT_TABLE(TEXT_LIVESIGNIN_ERROR_UPDATEREQUIRED),
                               XBFONT_CENTER_X );

            g_pFont->DrawText( sx + 16, sy + 146, MENU_TEXT_RGB_NORMAL, L"\200 " );
            g_pFont->DrawText( MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_LIVESIGNIN_BEGININSTALLATION) );
            g_pFont->DrawText( sx + 16, sy + 166, MENU_TEXT_RGB_NORMAL, L"\201 " );
            g_pFont->DrawText( MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_LIVESIGNIN_BACKTOMAINMENU) );
        }
        else if( g_hrSignInResult == XONLINE_E_LOGON_CANNOT_ACCESS_SERVICE )
        {
            g_pFont->DrawText( sx + 150, sy + 50, MENU_TEXT_RGB_NORMAL,
                               TEXT_TABLE(TEXT_LIVESIGNIN_ERROR_CANNOTACCESSSERVICE),
                               XBFONT_CENTER_X );

            g_pFont->DrawText( sx + 16, sy + 146, MENU_TEXT_RGB_NORMAL, L"\200 " );
            g_pFont->DrawText( MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_LIVESIGNIN_NETWORKTROUBLESHOOTER)  );
            g_pFont->DrawText( sx + 16, sy + 166, MENU_TEXT_RGB_NORMAL, L"\201 " );
            g_pFont->DrawText( MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_BACK) );
        }
        else if( g_hrSignInResult == XONLINE_E_LOGON_SERVERS_TOO_BUSY )
        {
            g_pFont->DrawText( sx + 150, sy + 50, MENU_TEXT_RGB_NORMAL,
                               TEXT_TABLE(TEXT_LIVESIGNIN_ERROR_SERVERSTOOBUSY),
                               XBFONT_CENTER_X );

            g_pFont->DrawText( sx + 16, sy + 166, MENU_TEXT_RGB_NORMAL, L"\201 " );
            g_pFont->DrawText( MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_BACK) );
        }
        else if( m_pPlayer->pXOnlineUser->hr == XONLINE_E_LOGON_USER_ACCOUNT_REQUIRES_MANAGEMENT )
        {
//$TODO: Could assert that the global sign in succeeded
            g_pFont->DrawText( sx + 150, sy + 50, MENU_TEXT_RGB_NORMAL,
                               TEXT_TABLE(TEXT_LIVESIGNIN_ERROR_ACCTREQUIRESMGMT),
                               XBFONT_CENTER_X );

            g_pFont->DrawText( sx + 16, sy + 146, MENU_TEXT_RGB_NORMAL, L"\200 " );
            g_pFont->DrawText( MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_LIVESIGNIN_REBOOTTODASH) );
//$REVISIT: For consumer beta, disabling Back - user MUST go to dashboard.            
//            g_pFont->DrawText( sx + 16, sy + 166, MENU_TEXT_RGB_NORMAL, L"\201 " );
//            g_pFont->DrawText( MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_BACK) );
        }
        else
        {
//$TODO: Probably should never get here. Could put in an assert.
            WCHAR strBuffer[256];
            swprintf( strBuffer, TEXT_TABLE(TEXT_LIVESIGNIN_ERROR_UNKNOWNPROBLEM) );
            g_pFont->DrawText( sx + 150, sy + 50, MENU_TEXT_RGB_NORMAL,
                               strBuffer, XBFONT_CENTER_X );

            g_pFont->DrawText( sx + 16, sy + 146, MENU_TEXT_RGB_NORMAL, L"\200 " );
            g_pFont->DrawText( MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_LIVESIGNIN_NETWORKTROUBLESHOOTER) );
            g_pFont->DrawText( sx + 16, sy + 166, MENU_TEXT_RGB_NORMAL, L"\201 " );
            g_pFont->DrawText( MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_BACK) );
        }

/*
//$TODO: Revisit this for the 4-player case. With 4-players, there are
//       additional errors that the app must catch.

        WCHAR strReason[80] = L"Reason unknown.";
        
//$REVISIT: Do we really want to handle these return values here?
//$TODO: Make this error handling consistent with handling in net_xonline.cpp
        if( m_pPlayer->dwSignInError )
        {
            switch( m_pPlayer->dwSignInError )
            {
                case PLAYER_XONLINE_INFO::SIGNINERROR_DUPLICATE_USER:
                    swprintf( strReason, L"Reason: Duplicate User." );
                    break;
                case PLAYER_XONLINE_INFO::SIGNINERROR_GUEST_HAS_NO_VALID_USER:
                    swprintf( strReason, L"Reason: Guest has no valid user." );
                    break;
            }
        }
        g_pFont->DrawText( sx + 150, sy + 50, MENU_TEXT_RGB_NORMAL, strReason,
                           XBFONT_CENTER_X );

        g_pFont->DrawText( sx + 16, sy + 160, MENU_TEXT_RGB_NORMAL,
                           L"\201 Change Account" );
*/
    }
}




//-----------------------------------------------------------------------------
// Name: RenderSignInSucceeded()
// Desc: 
//-----------------------------------------------------------------------------
VOID CPlayerScreenStateEngine::RenderSignInSucceeded( FLOAT sx, FLOAT sy )
{
    g_pFont->DrawText( sx + 150, sy + 20, MENU_TEXT_RGB_NORMAL, 
                       m_pPlayer->wstrName, XBFONT_CENTER_X );

    if( m_pPlayer->pXOnlineUser->hr == XONLINE_S_LOGON_USER_HAS_MESSAGE )
    {
        g_pFont->DrawText( sx + 150, sy + 50, MENU_TEXT_RGB_NORMAL, 
                           TEXT_TABLE(TEXT_LIVESIGNIN_YOUARESIGNEDIN),
                           XBFONT_CENTER_X );
        g_pFont->DrawText( sx + 150, sy + 80, MENU_TEXT_RGB_NORMAL, 
                           TEXT_TABLE(TEXT_LIVESIGNIN_YOUHAVEMESSAGES),
                           XBFONT_CENTER_X );

        g_pFont->DrawText( sx + 16, sy + 146, MENU_TEXT_RGB_NORMAL, L"\200 " );
        g_pFont->DrawText( MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_LIVESIGNIN_LETSPLAY) );
        g_pFont->DrawText( sx + 16, sy + 166, MENU_TEXT_RGB_NORMAL, L"\203 " );
        g_pFont->DrawText( MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_LIVESIGNIN_GETMESSAGES) );
    }
    else
    {
        g_pFont->DrawText( sx + 150, sy + 70, MENU_TEXT_RGB_NORMAL, 
                           TEXT_TABLE(TEXT_LIVESIGNIN_YOUARESIGNEDIN),
                           XBFONT_CENTER_X );

        g_pFont->DrawText( sx + 16, sy + 166, MENU_TEXT_RGB_NORMAL, L"\200 " );
        g_pFont->DrawText( MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_LIVESIGNIN_LETSPLAY) );
    }
}





static void CreateSignOutMenu( MENU_HEADER* pMenuHeader, MENU* pMenu );
static BOOL HandleSignOutMenu( MENU_HEADER* pMenuHeader, DWORD input );
static VOID DrawSignOutMenu( MENU_HEADER* pMenuHeader, MENU* pMenu );


extern MENU Menu_SignOut = 
{
    TEXT_SIGNOUT,
    MENU_DEFAULT | MENU_CENTRE_X | MENU_CENTRE_Y,   // Menu type
    CreateSignOutMenu,                      // Create menu function
    HandleSignOutMenu,                      // Input handler function
    NULL, // DrawSignOutMenu,                        // Menu draw function
    0,                                      // X coord
    0,                                      // Y Coord
};


// Create
void CreateSignOutMenu( MENU_HEADER* pMenuHeader, MENU* pMenu )
{
    // add menu items
    pMenuHeader->AddMenuItem( TEXT_ARE_YOU_SURE );
    pMenuHeader->AddMenuItem( TEXT_NONE, MENU_ITEM_INACTIVE );
    pMenuHeader->AddMenuItem( TEXT_NO );
    pMenuHeader->AddMenuItem( TEXT_YES );
    
    // Put the selection on the "No" menuitem
    pMenu->CurrentItemIndex = 2;
}


BOOL HandleSignOutMenu( MENU_HEADER* pMenuHeader, DWORD dwInput )
{
    switch( dwInput )
    {
        case MENU_INPUT_UP:
            if( pMenuHeader->m_pCurrentItem->TextIndex == TEXT_YES )
            {
                return SelectPreviousMenuItem( pMenuHeader, pMenuHeader->m_pMenu, NULL );
            }
            break;
        
        case MENU_INPUT_DOWN:
            if( pMenuHeader->m_pCurrentItem->TextIndex == TEXT_NO )
            {
                return SelectNextMenuItem( pMenuHeader, pMenuHeader->m_pMenu, NULL );
            }
            break;

        case MENU_INPUT_BACK:
            g_LiveSignOutStateEngine.Return( STATEENGINE_TERMINATED );
            return TRUE;

        case MENU_INPUT_SELECT:
            if( pMenuHeader->m_pCurrentItem->TextIndex == TEXT_YES )
                g_LiveSignInStateEngine.SignOut();
            g_LiveSignOutStateEngine.Return( STATEENGINE_COMPLETED );
            return TRUE;
    }

    return FALSE;
}




CLiveSignOutStateEngine g_LiveSignOutStateEngine;

//-----------------------------------------------------------------------------
// Name: HandleEnter** / HandleExit**  functions
// Desc: Init/uninit work necessary when entering/exiting state engine.
//-----------------------------------------------------------------------------
VOID CLiveSignOutStateEngine::HandleEnterFromParent()  { }
VOID CLiveSignOutStateEngine::HandleExitToParent()     { }



//-----------------------------------------------------------------------------
// Name: Process()
// Desc: Main control function for the state engine
//-----------------------------------------------------------------------------
HRESULT CLiveSignOutStateEngine::Process()
{
    enum
    {
        LIVESIGNOUT_STATE_BEGIN = STATEENGINE_STATE_BEGIN,
        LIVESIGNOUT_STATE_MAINLOOP,
    };

    switch( m_State )
    {
        case LIVESIGNOUT_STATE_BEGIN:
            g_pMenuHeader->SetNextMenu( &Menu_SignOut );

            m_State = LIVESIGNOUT_STATE_MAINLOOP;
            break;

        case LIVESIGNOUT_STATE_MAINLOOP:
            // Nothing to do, as control is in the menus
            break;
    }

    // Handle the menus
    g_pMenuHeader->HandleMenus();

    return S_FALSE;
}



