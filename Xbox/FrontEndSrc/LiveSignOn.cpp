//-----------------------------------------------------------------------------
// File: LiveSignOn.cpp
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include <xtl.h>
#include <xonline.h>
#include "revolt.h"
#include "XBOnline.h"

#include "Text.h"
#include "XBFont.h"
#include "XBResource.h"
#include "XBInput.h"
#include "main.h"
#include "StateEngine.h"
#include "LiveSignOn.h"



CLiveSignOnStateEngine g_LiveSignOnStateEngine;







extern VOID    DrawScreenSpaceQuad( FLOAT sx, FLOAT sy, D3DTexture* pTexture );

extern D3DTexture* g_pLiveSignOnTexture;
extern D3DTexture* g_pLiveSignOnPlayerTexture;
extern D3DTexture* g_pLiveSignOnListBoxTexture;
extern D3DTexture* g_pHeaderLayer1Texture;
extern D3DTexture* g_pHeaderLayer2Texture;
extern D3DTexture* g_pFooterLayer1Texture;
extern D3DTexture* g_pFooterLayer2Texture;





//-----------------------------------------------------------------------------
// Name: struct PLAYER
// Desc: Data for a player
//-----------------------------------------------------------------------------
struct PLAYER
{
    enum 
    {
        SIGNONERROR_DUPLICATE_USER = 1,
        SIGNONERROR_GUEST_HAS_NO_VALID_USER,
    };

    WCHAR         m_strName[16+XONLINE_NAME_SIZE]; // Displayable user name
    
    XONLINE_USER* m_pXOnlineUser;                  // Ptr to online user info
    BOOL          m_bIsGuestOfUser;                // Actual user, or a guest?
    BYTE          m_PassCode[4];                   // Pass code for logging on
    DWORD         m_dwSignOnError;
};




//-----------------------------------------------------------------------------
// Name: class CPlayerScreen
// Desc: Class to handle and render a screen for a player that is signing on.
//-----------------------------------------------------------------------------
class CPlayerScreen
{
public:
    FLOAT   m_x;
    FLOAT   m_y;
    
    DWORD   m_dwCurrentUser;                     // User ID in global list
    PLAYER* m_pPlayer;                           // Pointer to the player

    enum 
    {
        STATE_WAITING_FOR_PLAYER,
        STATE_CREATING_ACCOUNT,
        STATE_SELECTING_ACCOUNT,
        STATE_ENTERING_PASS_CODE,
        STATE_WRONG_PASSCODE,
        STATE_WAITING_TO_SIGNON,
        STATE_SIGNING_ON,
        STATE_SIGNON_SUCCEEDED,
        STATE_SIGNON_FAILED,

        STATE_EXIT_LIVESIGNON_BACK,
        STATE_EXIT_LIVESIGNON_FORWARD,
    };

    DWORD  m_State;        // State of the player screen
    DWORD  m_PrevState;    // Previous state to return to

    FLOAT  m_fSignOnTime;  // Time it's taking to sign on
    
    // Initialization functions
    VOID Initialize( PLAYER* pPlayer );
    VOID SetViewportOffset( DWORD quadrant, FLOAT x, FLOAT y );

    // Input handling functions
    VOID Update( DWORD InputEvent );
    VOID UpdateWaitingForPlayerScreen( DWORD InputEvent );
    VOID UpdateCreateAccountScreen( DWORD InputEvent );
    VOID UpdateSelectAccountScreen( DWORD InputEvent );
    VOID UpdateEnterPassCodeScreen( DWORD InputEvent );
    VOID UpdateWrongPassCodeScreen( DWORD InputEvent );
    VOID UpdateSigningOnScreen( DWORD InputEvent );
    VOID UpdateSignOnFinished( DWORD InputEvent );
    VOID UpdateWaitingForOtherUsers( DWORD InputEvent );

    // Render functions
    VOID Render();
    VOID RenderWaitingForPlayerScreen();
    VOID RenderCreateAccountScreen();
    VOID RenderSelectAccountScreen();
    VOID RenderEnterPassCodeScreen();
    VOID RenderWrongPassCodeScreen();
    VOID RenderWaitingForOtherUsers();
    VOID RenderSigningOnScreen();
    VOID RenderSignOnFinished();
};




PLAYER        g_Players[4];
CPlayerScreen g_PlayerScreens[4];

XONLINE_USER* g_XOnlineUserList;
DWORD         g_dwNumUsers;

DWORD         g_dwNumActivePlayers = 0L;

BOOL          g_bReadyToSignOn      = FALSE;
BOOL          g_bAllPlayersSignedOn = FALSE;
BOOL          g_bPlayersCanJoin     = TRUE;




//-----------------------------------------------------------------------------
// Name: CheckSignOnConditions()
// Desc: 
//-----------------------------------------------------------------------------
VOID CheckSignOnConditions()
{
    // Check states of all players
    g_dwNumActivePlayers = 0L;
    DWORD dwNumPlayersNotActive     = 0;
    DWORD dwNumPlayersReadyToSignOn = 0;
    DWORD dwNumPlayersSigningOn     = 0;
    DWORD dwNumPlayersSignedOn      = 0;

    for( DWORD i=0; i<4; i++ )
    {
        // Count active players (if the player screen is not at the
        // starting screen, it means a player is active)
        if( g_PlayerScreens[i].m_State != CPlayerScreen::STATE_WAITING_FOR_PLAYER )
            g_dwNumActivePlayers++;

        // Count players who have already successfully signed on
        if( g_PlayerScreens[i].m_State == CPlayerScreen::STATE_SIGNING_ON )
            dwNumPlayersSigningOn++;

        // Count players who have already successfully signed on
        if( g_PlayerScreens[i].m_State == CPlayerScreen::STATE_SIGNON_SUCCEEDED )
            dwNumPlayersSignedOn++;

        // Count players (and determine errors) on players waiting to sign on
        if( g_PlayerScreens[i].m_State == CPlayerScreen::STATE_WAITING_TO_SIGNON )
        {
            BOOL bGuestHasValidUser = FALSE;

            // Assume no error to start
            g_Players[i].m_dwSignOnError = 0;

            // Check current players against all other players (for duplicate
            // users and make sure guests have valid users)
            for( DWORD j=0; j<4; j++ )
            {
                if( i == j || g_PlayerScreens[j].m_State != CPlayerScreen::STATE_WAITING_TO_SIGNON )
                    continue;

                // Check for two players referencing the same user
                if( g_Players[i].m_pXOnlineUser == g_Players[j].m_pXOnlineUser )
                {
                    // If neither are players are guests, then they are erroneous
                    // duplicates of the same user
                    if( !g_Players[i].m_bIsGuestOfUser && !g_Players[j].m_bIsGuestOfUser )
                    {
                        g_Players[i].m_dwSignOnError = PLAYER::SIGNONERROR_DUPLICATE_USER;
                    }
                    else
                    {
                        // Else, test if one player is a guest of the other.
                        if( g_Players[i].m_bIsGuestOfUser != g_Players[j].m_bIsGuestOfUser )
                            bGuestHasValidUser = TRUE;
                    }
                }
            }

            // For guests only, make sure a matching user was found
            if( g_Players[i].m_bIsGuestOfUser )
            {
                if( FALSE == bGuestHasValidUser )
                    g_Players[i].m_dwSignOnError = PLAYER::SIGNONERROR_GUEST_HAS_NO_VALID_USER;
            }

            // If the above code detected no errors, the player is ready to sign on
            if( 0 == g_Players[i].m_dwSignOnError )
                dwNumPlayersReadyToSignOn++;
        }
    }

    // Mark convenient global flags based on the above tests
    g_bReadyToSignOn      = FALSE;
    g_bAllPlayersSignedOn = FALSE;
    g_bPlayersCanJoin     = TRUE;

    if( g_dwNumActivePlayers > 0 )
    {
        // Check if all active players are ready to sign on
        if( dwNumPlayersReadyToSignOn == g_dwNumActivePlayers )
            g_bReadyToSignOn = TRUE;

        // Check if all active players are currently signed on
        if( dwNumPlayersSignedOn == g_dwNumActivePlayers )
            g_bAllPlayersSignedOn = TRUE;

        // Check condition for whether we still let other players join
        if( dwNumPlayersSigningOn+dwNumPlayersSignedOn == g_dwNumActivePlayers )
            g_bPlayersCanJoin = FALSE;
    }
}




//-----------------------------------------------------------------------------
// Name: Render_LiveSignOnScreen()
// Desc: 
//-----------------------------------------------------------------------------
VOID Render_LiveSignOnScreen( FLOAT ViewportStartX, FLOAT ViewportStartY )
{
    DrawScreenSpaceQuad( ViewportStartX+48, ViewportStartY+26,  g_pHeaderLayer1Texture );
    DrawScreenSpaceQuad( ViewportStartX+48, ViewportStartY+411, g_pFooterLayer1Texture );

    // Draw the sign on screen frame
//    DrawScreenSpaceQuad( ViewportStartX, ViewportStartY, g_pLiveSignOnTexture );


    // Draw the player screens
    for( DWORD i=0; i<4; i++ )
    {
        // Set the viewport offset for the player screens
        g_PlayerScreens[i].SetViewportOffset( i, ViewportStartX, ViewportStartY );

        // Render the screen
        g_PlayerScreens[i].Render();
    }

    DrawScreenSpaceQuad( ViewportStartX+48, ViewportStartY+26,  g_pHeaderLayer2Texture );
    DrawScreenSpaceQuad( ViewportStartX+48, ViewportStartY+411, g_pFooterLayer2Texture );
    g_pMenuTitleFont->DrawText( 140+ViewportStartX, 38+ViewportStartY, 
                                0xffffffff, L"Live! Sign On" );
}




//-----------------------------------------------------------------------------
// Name: LiveSignOn()
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CLiveSignOnStateEngine::Process()
{
    enum 
    {
        LIVESIGNON_STATE_BEGIN = STATEENGINE_STATE_BEGIN,
        LIVESIGNON_STATE_MAINLOOP,
        LIVESIGNON_STATE_EXITFORWARD,
        LIVESIGNON_STATE_EXITBACK,
    };

    static FLOAT ViewportStartX;           // Animate-able positiion
    static FLOAT ViewportStartY;

    switch( m_State )
    {
        case LIVESIGNON_STATE_BEGIN:
        {
            // Initialize the XOnline stuff
            if( FAILED( XBOnline_Init(0) ) )
            {
                Return();
                return E_FAIL;
            }

            // Get the online user list, which is used by the player screens
            XBOnline_GetUserList( &g_XOnlineUserList, &g_dwNumUsers );

            // Initialize the player screens used to handle input for
            // users trying to sign on
            for( DWORD i=0; i<4; i++ )
                g_PlayerScreens[i].Initialize( &g_Players[i] );

            // Start the viewport off the left edge of the screen, so we
            // can animate it in
            ViewportStartX = -640.0f;
            ViewportStartY =    0.0f;

            // The menu is gone, so advance to the next state
            m_State = LIVESIGNON_STATE_MAINLOOP;

            break;
        }

        case LIVESIGNON_STATE_MAINLOOP:
        {
            // Check the connection
            if( XBOnline_IsActive() == FALSE )
            {
                // We lost our connection
            }

            // Animate the Viewport starting point
            if( ViewportStartX<0 ) 
                ViewportStartX += 4*640.0f*TimeStep;
            ViewportStartX = min( 0.0f, ViewportStartX );

            // Render the screen
            Render_LiveSignOnScreen( ViewportStartX, ViewportStartY );

            // Check player input
            for( DWORD i=0; i<4; i++ )
            {
                g_PlayerScreens[i].Update( g_Controllers[i].dwMenuInput );

                if( g_PlayerScreens[i].m_State == CPlayerScreen::STATE_EXIT_LIVESIGNON_BACK  )
                {
                    // Select the state to animate the menu away
                    m_State = LIVESIGNON_STATE_EXITBACK;
                    return S_FALSE;
                }

                if( g_PlayerScreens[i].m_State == CPlayerScreen::STATE_EXIT_LIVESIGNON_FORWARD  )
                {
                    m_State = LIVESIGNON_STATE_EXITFORWARD;
                    return S_FALSE;
                }
            }

            // Check conditions for before, during, or after sign on
            CheckSignOnConditions();

            break;
        }

        case LIVESIGNON_STATE_EXITFORWARD:
        {
            if( ViewportStartY < 480 )
            {
                // Animate the Viewport starting point
                ViewportStartY += 4*480.0f*TimeStep;

                Render_LiveSignOnScreen( ViewportStartX, ViewportStartY );
            }
            else
            {
                // We're finished. Reset the state and exit
                Return();
                return S_OK;
            }
            break;
        }

        case LIVESIGNON_STATE_EXITBACK:
        {
            if( ViewportStartY < 480 )
            {
                // Animate the Viewport starting point
                ViewportStartY += 4*480.0f*TimeStep;

                Render_LiveSignOnScreen( ViewportStartX, ViewportStartY );
            }
            else
            {
                // We're finished. Reset the state and exit
                Return();
                return E_FAIL;
            }
            break;
        }
    }

    return S_FALSE;
}




//-----------------------------------------------------------------------------
// Name: LiveSignOff()
// Desc: 
//-----------------------------------------------------------------------------
HRESULT LiveSignOff()
{
    // TODO: Should we be displaying any confirmation UI here?

    XBOnline_SignOff();

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: ArePlayersSignedOn()
// Desc: 
//-----------------------------------------------------------------------------
BOOL ArePlayersSignedOn()
{
    return g_bAllPlayersSignedOn;
}




//-----------------------------------------------------------------------------
// Name: Initialize()
// Desc: Initializes a player screen
//-----------------------------------------------------------------------------
VOID CPlayerScreen::Initialize( PLAYER* pPlayer )
{
    m_x = 0;
    m_y = 0;
    m_State = STATE_WAITING_FOR_PLAYER;

    m_pPlayer = pPlayer;
    m_pPlayer->m_PassCode[0] = 0;
    m_pPlayer->m_PassCode[1] = 0;
    m_pPlayer->m_PassCode[2] = 0;
    m_pPlayer->m_PassCode[3] = 0;
}




//-----------------------------------------------------------------------------
// Name: SetViewportOffset()
// Desc: A player screen lives in one of four on-screen quadrants, that can be
//       set here, along with an optional offset.
//-----------------------------------------------------------------------------
VOID CPlayerScreen::SetViewportOffset( DWORD quadrant, FLOAT x, FLOAT y )
{
    switch( quadrant )
    {
        case 0: m_x = x +  48;  m_y = y +  36 + 20;  break;
        case 1: m_x = x + 320;  m_y = y +  36 + 20;  break;
        case 2: m_x = x +  48;  m_y = y + 240;       break;
        case 3: m_x = x + 320;  m_y = y + 240;       break;
    }
}




//-----------------------------------------------------------------------------
// Name: Update()
// Desc: Process input for the player screen
//-----------------------------------------------------------------------------
VOID CPlayerScreen::Update( DWORD InputEvent )
{
    switch( m_State )
    {
        case STATE_WAITING_FOR_PLAYER:
            UpdateWaitingForPlayerScreen( InputEvent );
            break;
        
        case STATE_CREATING_ACCOUNT:
            UpdateCreateAccountScreen( InputEvent );
            break;

        case STATE_SELECTING_ACCOUNT:
            UpdateSelectAccountScreen( InputEvent );
            break;

        case STATE_ENTERING_PASS_CODE:
            UpdateEnterPassCodeScreen( InputEvent );
            break;

        case STATE_WRONG_PASSCODE:
            UpdateWrongPassCodeScreen( InputEvent );
            break;

        case STATE_WAITING_TO_SIGNON:
            UpdateWaitingForOtherUsers( InputEvent );
            break;

        case STATE_SIGNING_ON:
            UpdateSigningOnScreen( InputEvent );
            break;

        case STATE_SIGNON_SUCCEEDED:
        case STATE_SIGNON_FAILED:
            UpdateSignOnFinished( InputEvent );
            break;
    }
}




//-----------------------------------------------------------------------------
// Name: UpdateWaitingForPlayerScreen()
// Desc: Waiting for a player to come along
//-----------------------------------------------------------------------------
VOID CPlayerScreen::UpdateWaitingForPlayerScreen( DWORD InputEvent )
{
    m_pPlayer->m_pXOnlineUser = NULL;

    switch( InputEvent )
    {
        // Handle case when user presses the BACK or B button
        case XBINPUT_BACK_BUTTON:
        case XBINPUT_B_BUTTON:
        {
            // Select the state to animate the menu away
            m_State = STATE_EXIT_LIVESIGNON_BACK;
            break;
        }

        // Handle case when user presses the START or A button
        case XBINPUT_START_BUTTON:
        case XBINPUT_A_BUTTON:
        {
            if( g_bPlayersCanJoin )
            {
                if( g_dwNumUsers == 0 )
                {
                    m_State     = STATE_CREATING_ACCOUNT;
                    m_PrevState = STATE_WAITING_FOR_PLAYER;
                }
                else
                {
                    m_State     = STATE_SELECTING_ACCOUNT;
                    m_PrevState = STATE_WAITING_FOR_PLAYER;
                }
                m_dwCurrentUser = 0;
            }
            break;
        }
    }
}




//-----------------------------------------------------------------------------
// Name: UpdateCreateAccountScreen()
// Desc: Allow player to launch account creation tool
//-----------------------------------------------------------------------------
VOID CPlayerScreen::UpdateCreateAccountScreen( DWORD InputEvent )
{
    m_pPlayer->m_pXOnlineUser = NULL;

    switch( InputEvent )
    {
        // Handle case when user presses the BACK or B button
        case XBINPUT_BACK_BUTTON:
        case XBINPUT_B_BUTTON:
        {
            m_State = m_PrevState;
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
            ld.dwReason = XLD_LAUNCH_DASHBOARD_MAIN_MENU;
            XLaunchNewImage( NULL, PLAUNCH_DATA( &ld ) );
            break;
        }

        default:
        {
/*
            // If any MUs are inserted, update the user list
            // and go to account selection if there are any accounts
            DWORD dwInsertions;
            DWORD dwRemovals;
            if( CXBMemUnit::GetMemUnitChanges( dwInsertions, dwRemovals ) )
            {
                m_UserList.clear();
                XBOnline_GetUserList( m_UserList );
                if( !m_UserList.empty() )
                {
                    m_State     = STATE_SELECTING_ACCOUNT;
                }
            }
*/
            break;
        }
    }
}




//-----------------------------------------------------------------------------
// Name: UpdateSelectAccountScreen()
// Desc: Allow player to choose account
//-----------------------------------------------------------------------------
VOID CPlayerScreen::UpdateSelectAccountScreen( DWORD InputEvent )
{
    m_pPlayer->m_pXOnlineUser = NULL;

    switch( InputEvent )
    {
        // Handle case when user presses the BACK or B button
        case XBINPUT_BACK_BUTTON:
        case XBINPUT_B_BUTTON:
        {
            m_State = STATE_WAITING_FOR_PLAYER;
            break;
        }

        // Handle case when user presses the START or A button
        case XBINPUT_START_BUTTON:
        case XBINPUT_A_BUTTON:
        {
            // Sign on as the selected user
            m_pPlayer->m_pXOnlineUser = &g_XOnlineUserList[m_dwCurrentUser];
            m_pPlayer->m_bIsGuestOfUser = FALSE;

            // Copy the user name (converting to a wide string )
            swprintf( m_pPlayer->m_strName, L"%S", m_pPlayer->m_pXOnlineUser->name );

            // Clear the passcode
            m_pPlayer->m_PassCode[0] = 0;
            m_pPlayer->m_PassCode[1] = 0;
            m_pPlayer->m_PassCode[2] = 0;
            m_pPlayer->m_PassCode[3] = 0;

            // Advance state, dpeending on whether the user needs to enter his pass code
            if( m_pPlayer->m_pXOnlineUser->dwUserOptions & XONLINE_USER_OPTION_REQUIRE_PIN )
            {
                // Switch state to enter the pass code
                m_State = STATE_ENTERING_PASS_CODE;
            }
            else
            {
                // Switch state to wait for other players
                m_State = STATE_WAITING_TO_SIGNON;
            }

            break;
        }
        
        case XBINPUT_X_BUTTON:
        {
            // Sign on as a guest of the selected user
            m_pPlayer->m_pXOnlineUser = &g_XOnlineUserList[m_dwCurrentUser];
            m_pPlayer->m_bIsGuestOfUser = TRUE;

            // Copy the user name (converting to a wide string )
            swprintf( m_pPlayer->m_strName, L"Guest of %S", m_pPlayer->m_pXOnlineUser->name );

            // Switch state to wait for other players
            m_State = STATE_WAITING_TO_SIGNON;

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
            if( InputEvent == XBINPUT_Y_BUTTON )
            {
                m_State     = STATE_CREATING_ACCOUNT;
                m_PrevState = STATE_SELECTING_ACCOUNT;
            }
/*
            // If any MUs are inserted/removed, need to update the
            // user account list
            DWORD dwInsertions;
            DWORD dwRemovals;
            if( CXBMemUnit::GetMemUnitChanges( dwInsertions, dwRemovals ) )
            {
                m_UserList.clear();
                XBOnline_GetUserList( m_UserList );
                if( m_UserList.empty() )
                {
                    m_State     = STATE_CREATING_ACCOUNT;
                }   
                else
                    m_dwUser = 0;
            }
*/
            break;
        }
    }
}




//-----------------------------------------------------------------------------
// Name: UpdateEnterPassCodeScreen()
// Desc: Allow player to enter PIN number
//-----------------------------------------------------------------------------
VOID CPlayerScreen::UpdateEnterPassCodeScreen( DWORD InputEvent )
{
    switch( InputEvent )
    {
        case XBINPUT_BACK_BUTTON:
        case XBINPUT_B_BUTTON:
        {
            if( m_pPlayer->m_PassCode[0] )
            {
                for( INT i=3; i>=0; --i )
                {
                    if( m_pPlayer->m_PassCode[i] )
                    {
                        m_pPlayer->m_PassCode[i] = 0;
                        break;
                    }
                }
            }
            else
            {
                // Unselect the user for this player
                m_pPlayer->m_pXOnlineUser = NULL;

                m_State = STATE_SELECTING_ACCOUNT;
            }
            break;
        }

        case XBINPUT_START_BUTTON:
        case XBINPUT_A_BUTTON:
        {
            // If the pass code is entered, proceed to the next state
            if( m_pPlayer->m_PassCode[3] )
            {
                // Check the passcode
                if( 0 == memcmp( m_pPlayer->m_PassCode, m_pPlayer->m_pXOnlineUser->pin, 4 ) )
                {
                    m_State = STATE_WAITING_TO_SIGNON;
                }
                else
                {
                    m_State = STATE_WRONG_PASSCODE;
                }
            }
            break;
        }

        default:
        {
            // Record the next pass code character
            for( DWORD i=0; i<4; i++ )
            {
                if( m_pPlayer->m_PassCode[i] == 0 )
                {
                    switch( InputEvent )
                    {
                        case XBINPUT_UP:            m_pPlayer->m_PassCode[i] =  1; break;
                        case XBINPUT_DOWN:          m_pPlayer->m_PassCode[i] =  2; break;
                        case XBINPUT_LEFT:          m_pPlayer->m_PassCode[i] =  3; break;
                        case XBINPUT_RIGHT:         m_pPlayer->m_PassCode[i] =  4; break;
                        case XBINPUT_X_BUTTON:      m_pPlayer->m_PassCode[i] =  5; break;
                        case XBINPUT_Y_BUTTON:      m_pPlayer->m_PassCode[i] =  6; break;
                        case XBINPUT_BLACK_BUTTON:  m_pPlayer->m_PassCode[i] =  7; break;
                        case XBINPUT_WHITE_BUTTON:  m_pPlayer->m_PassCode[i] =  8; break;
                        case XBINPUT_LEFT_TRIGGER:  m_pPlayer->m_PassCode[i] =  9; break;
                        case XBINPUT_RIGHT_TRIGGER: m_pPlayer->m_PassCode[i] = 10; break;
                    }
                    break;
                }
            }
            break;
        }
    }
}




//-----------------------------------------------------------------------------
// Name: UpdateWrongPassCodeScreen()
// Desc: User entered wrong pass code, so let him go back and try again
//-----------------------------------------------------------------------------
VOID CPlayerScreen::UpdateWrongPassCodeScreen( DWORD InputEvent )
{
    switch( InputEvent )
    {
        case XBINPUT_BACK_BUTTON:
        case XBINPUT_B_BUTTON:
        {
            m_pPlayer->m_PassCode[0] = 0;
            m_pPlayer->m_PassCode[1] = 0;
            m_pPlayer->m_PassCode[2] = 0;
            m_pPlayer->m_PassCode[3] = 0;
            m_State = STATE_ENTERING_PASS_CODE;
            break;
        }
    }
}




//-----------------------------------------------------------------------------
// Name: UpdateWaitingForOtherUsers()
// Desc: 
//-----------------------------------------------------------------------------
VOID CPlayerScreen::UpdateWaitingForOtherUsers( DWORD InputEvent )
{
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
            if( TRUE == g_bReadyToSignOn )
            {
                XONLINE_USER* pUserList[4];
                pUserList[0] = g_Players[0].m_pXOnlineUser;
                pUserList[1] = g_Players[1].m_pXOnlineUser;
                pUserList[2] = g_Players[2].m_pXOnlineUser;
                pUserList[3] = g_Players[3].m_pXOnlineUser;

                BOOL bBeginSignOnSucceeded = SUCCEEDED( XBOnline_BeginSignOn( pUserList ) );

                // Advance state to sign on any waiting users
                for( DWORD i=0; i<4; i++ )
                {
                    if( g_PlayerScreens[i].m_State == STATE_WAITING_TO_SIGNON )
                    {
                        if( bBeginSignOnSucceeded )
                        {
                            g_PlayerScreens[i].m_State = STATE_SIGNING_ON;
                            g_PlayerScreens[i].m_fSignOnTime = 0.0f;
                            g_Players[i].m_pXOnlineUser->hr = S_OK;
                        }
                        else
                        {
                            g_PlayerScreens[i].m_State = STATE_SIGNON_FAILED;
                            g_PlayerScreens[i].m_fSignOnTime = 0.0f;
                            g_Players[i].m_pXOnlineUser->hr = E_FAIL;
                        }
                    }
                }
            }
            break;
        }
    }
}




//-----------------------------------------------------------------------------
// Name: UpdateSigningOnScreen()
// Desc: 
//-----------------------------------------------------------------------------
VOID CPlayerScreen::UpdateSigningOnScreen( DWORD InputEvent )
{
    switch( InputEvent )
    {
        case XBINPUT_BACK_BUTTON:
        case XBINPUT_B_BUTTON:
        {
            // Close the task (this cancels the logon process)
            XBOnline_SignOff();

            for( DWORD i=0; i<4; i++ )
            {
                if( g_PlayerScreens[i].m_State == STATE_SIGNING_ON )
                    g_PlayerScreens[i].m_State = STATE_WAITING_TO_SIGNON;
            }
            break;
        }

        // Debugging addition, to fake a sign on failure
        case XBINPUT_X_BUTTON:
        case XBINPUT_Y_BUTTON:
        {
            XBOnline_SignOff();

            for( DWORD i=0; i<4; i++ )
                if( g_PlayerScreens[i].m_State == STATE_SIGNING_ON )
                    g_PlayerScreens[i].m_State = STATE_SIGNON_FAILED;

            break;
        }

        default:
        {
            // Pump with the sign on task
            HRESULT hr = XBOnline_PumpSignOnTask();
            if( FAILED(hr) )
            {
                for( DWORD i=0; i<4; i++ )
                    if( g_PlayerScreens[i].m_State == STATE_SIGNING_ON )
                        g_PlayerScreens[i].m_State = STATE_SIGNON_FAILED;
            }
            else
            {
                if( hr != XONLINETASK_S_RUNNING )
                {
                    for( DWORD i=0; i<4; i++ )
                        if( g_PlayerScreens[i].m_State == STATE_SIGNING_ON )
                            g_PlayerScreens[i].m_State = STATE_SIGNON_SUCCEEDED;
                }
            }
            break;
        }
    }
}




//-----------------------------------------------------------------------------
// Name: UpdateSignOnFinished()
// Desc: 
//-----------------------------------------------------------------------------
VOID CPlayerScreen::UpdateSignOnFinished( DWORD InputEvent )
{
    switch( InputEvent )
    {
        case XBINPUT_BACK_BUTTON:
        case XBINPUT_B_BUTTON:
        {
            // Sign off
            XBOnline_SignOff();

            // Move other players back to the "Waiting..." screen
            for( DWORD i=0; i<4; i++ )
            {
                if( g_PlayerScreens[i].m_State == STATE_SIGNON_SUCCEEDED ) 
                    g_PlayerScreens[i].m_State = STATE_WAITING_TO_SIGNON;

                if( ( g_Players[i].m_pXOnlineUser == m_pPlayer->m_pXOnlineUser ) &&
                    ( g_Players[i].m_bIsGuestOfUser && !m_pPlayer->m_bIsGuestOfUser ) )
                    g_PlayerScreens[i].m_State = STATE_WAITING_TO_SIGNON;
            }

            // For this player, go back to select a new account
            m_State = STATE_SELECTING_ACCOUNT;

            break;
        }

        case XBINPUT_START_BUTTON:
        case XBINPUT_A_BUTTON:
        {
            if( g_bAllPlayersSignedOn )
            {
                for( DWORD i=0; i<4; i++ )
                    g_PlayerScreens[i].m_State = STATE_EXIT_LIVESIGNON_FORWARD;
            }
            break;
        }

        case XBINPUT_X_BUTTON:
        {
            // For debugging, if sign off fails, the user can hit the X button
            // to continue as if sign on actually succeeded
            g_bAllPlayersSignedOn = TRUE;

            for( DWORD i=0; i<4; i++ )
                g_PlayerScreens[i].m_State = STATE_EXIT_LIVESIGNON_FORWARD;
            break;
        }
    }
}




//-----------------------------------------------------------------------------
// Name: Render()
// Desc: 
//-----------------------------------------------------------------------------
VOID CPlayerScreen::Render()
{
    DrawScreenSpaceQuad( m_x, m_y+1, g_pLiveSignOnPlayerTexture );

    switch( m_State )
    {
        case STATE_WAITING_FOR_PLAYER:
            RenderWaitingForPlayerScreen();
            break;

        case STATE_CREATING_ACCOUNT:
            RenderCreateAccountScreen();
            break;

        case STATE_SELECTING_ACCOUNT:
            RenderSelectAccountScreen();
            break;

        case STATE_ENTERING_PASS_CODE:
            RenderEnterPassCodeScreen();
            break;

        case STATE_WRONG_PASSCODE:
            RenderWrongPassCodeScreen();
            break;

        case STATE_WAITING_TO_SIGNON:
            RenderWaitingForOtherUsers();
            break;

        case STATE_SIGNING_ON:
            RenderSigningOnScreen();
            break;

        case STATE_SIGNON_SUCCEEDED:
        case STATE_SIGNON_FAILED:
            RenderSignOnFinished();
            break;
    }
}
        

        
        
//-----------------------------------------------------------------------------
// Name: RenderWaitingForPlayerScreen()
// Desc: 
//-----------------------------------------------------------------------------
VOID CPlayerScreen::RenderWaitingForPlayerScreen()
{
    if( g_bPlayersCanJoin )
    {
        g_pMenuFont->DrawText( m_x + 134, m_y + 80, 0xffffffff, 
                               L"Press \200 to Sign On", XBFONT_CENTER_X );
    }
}




//-----------------------------------------------------------------------------
// Name: RenderCreateAccountScreen()
// Desc: 
//-----------------------------------------------------------------------------
VOID CPlayerScreen::RenderCreateAccountScreen()
{
    g_pMenuFont->DrawText( m_x + 134, m_y + 10, 0xffffffff, 
                           L"Do you want to reboot to\n"
                           L"the Dashboard and create a\n"
                           L"new account?", 
                           XBFONT_CENTER_X );
    
    g_pMenuFont->DrawText( m_x + 34, m_y + 140, 0xffffffff, 
                                  L"\200 Accept" );
    g_pMenuFont->DrawText( m_x + 34, m_y + 160, 0xffffffff, 
                                  L"\201 Back" );
}




//-----------------------------------------------------------------------------
// Name: RenderSelectAccountScreen()
// Desc: 
//-----------------------------------------------------------------------------
VOID CPlayerScreen::RenderSelectAccountScreen()
{
    DrawScreenSpaceQuad( m_x+32, m_y+31, g_pLiveSignOnListBoxTexture );

    g_pMenuFont->DrawText( m_x + 135, m_y + 10, 0xffffffff, 
                           L"Select Account", XBFONT_CENTER_X );

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
        if( row < 0 || row >= 5 )
            continue;

        // Convert name to a wide string for displaying
        WCHAR strUser[80];
        swprintf( strUser, L"%S", g_XOnlineUserList[user].name );

        // Draw the text
        DWORD color = ( user == m_dwCurrentUser ) ? 0xffff0000 : 0xffffffff;
        g_pMenuFont->DrawText( m_x + 50, m_y + 36 + row*20, color, 
                                        strUser );
    }

    // Draw button options at bottom of screen
    g_pMenuFont->DrawText( m_x + 34, m_y + 130, 0xffffffff, 
                                    L"\202 SignOn as a guest" );
    g_pMenuFont->DrawText( m_x + 34, m_y + 150, 0xffffffff, 
                                    L"\203 Create New Account" );

/*
    static FLOAT ax = 32;
    static FLOAT ay = 35;
    static FLOAT bx = 32;
    static FLOAT by = 35;

    if( g_dwNumUsers > 5 && m_dwCurrentUser > 2 )
    {
        if( g_dwNumUsers - m_dwCurrentUser <= 2 )
            DrawScreenSpaceQuad( m_x+ax, m_y+ay, g_pLiveSignOnUpArrowTexture );
        else
            DrawScreenSpaceQuad( m_x+bx, m_y+by, g_pLiveSignOnDownArrowTexture );
    }
*/
}




//-----------------------------------------------------------------------------
// Name: RenderEnterPassCodeScreen()
// Desc: 
//-----------------------------------------------------------------------------
VOID CPlayerScreen::RenderEnterPassCodeScreen()
{
    g_pMenuFont->DrawText( m_x + 134, m_y + 10, 0xffffffff, 
                                    L"Enter Pass Code", XBFONT_CENTER_X );
    g_pMenuFont->DrawText( m_x + 134, m_y + 35, 0xffffffff, 
                                    m_pPlayer->m_strName, XBFONT_CENTER_X );

    for( DWORD j=0; j<4; j++ )
    {
        if( m_pPlayer->m_PassCode[j] )
        {
            g_pMenuTitleFont->DrawText( m_x + 134 - 30 + j*20, m_y + 80, 
                                        0xffffffff, L"*", XBFONT_CENTER_X );
        }
        else
        {
            g_pMenuTitleFont->DrawText( m_x + 134 - 30 + j*20, m_y + 80, 
                                        0xffffffff, L"-", XBFONT_CENTER_X );
        }
    }
    
    if( m_pPlayer->m_PassCode[3] )
    {
        g_pMenuFont->DrawText( m_x + 34, m_y + 140, 0xffffffff, 
                                        L"\200 Accept" );
    }
    g_pMenuFont->DrawText( m_x + 34, m_y + 160, 0xffffffff, 
                                    L"\201 Back" );
}




//-----------------------------------------------------------------------------
// Name: RenderWrongPassCodeScreen()
// Desc: 
//-----------------------------------------------------------------------------
VOID CPlayerScreen::RenderWrongPassCodeScreen()
{
    g_pMenuFont->DrawText( m_x + 134, m_y + 10, 0xffffffff, 
                                    m_pPlayer->m_strName, XBFONT_CENTER_X );

    g_pMenuFont->DrawText( m_x + 134, m_y + 50, 0xffffffff, 
                                    L"Error: You entered the\n"
                                    L"wrong passcode.",
                                    XBFONT_CENTER_X );

    g_pMenuFont->DrawText( m_x + 34, m_y + 160, 0xffffffff, 
                                    L"\201 Back" );
}




//-----------------------------------------------------------------------------
// Name: RenderWaitingForOtherUsers()
// Desc: 
//-----------------------------------------------------------------------------
VOID CPlayerScreen::RenderWaitingForOtherUsers()
{
    g_pMenuFont->DrawText( m_x + 134, m_y + 10, 0xffffffff, 
                                    m_pPlayer->m_strName, XBFONT_CENTER_X );

    switch( m_pPlayer->m_dwSignOnError )
    {
        case PLAYER::SIGNONERROR_DUPLICATE_USER:
            g_pMenuFont->DrawText( m_x + 134, m_y + 50, 0xffffffff, 
                                            L"Warning: You have selected\n"
                                            L"the same user account as\n"
                                            L"another player.\n",
                                            XBFONT_CENTER_X );
            break;

        case PLAYER::SIGNONERROR_GUEST_HAS_NO_VALID_USER:
            g_pMenuFont->DrawText( m_x + 134, m_y + 50, 0xffffffff, 
                                            L"Warning: You have selected\n"
                                            L"a guest account for a user\n"
                                            L"that has not yet signed on.\n",
                                            XBFONT_CENTER_X );
            break;
        
        default:
            g_pMenuFont->DrawText( m_x + 134, m_y + 50, 0xffffffff, 
                                            L"Waiting for other players...\n",
                                            XBFONT_CENTER_X );
    }
    
    if( TRUE == g_bReadyToSignOn )
    {
        g_pMenuFont->DrawText( m_x + 34, m_y + 140, 0xffffffff, 
                                        L"\200 No more users" );
    }

    g_pMenuFont->DrawText( m_x + 34, m_y + 160, 0xffffffff,
                                    L"\201 Back" );
}




//-----------------------------------------------------------------------------
// Name: RenderSigningOnScreen()
// Desc: 
//-----------------------------------------------------------------------------
VOID CPlayerScreen::RenderSigningOnScreen()
{
    g_pMenuFont->DrawText( m_x + 134, m_y + 10, 0xffffffff, 
                                    m_pPlayer->m_strName, XBFONT_CENTER_X );
    g_pMenuFont->DrawText( m_x + 134, m_y + 30, 0xffffffff, 
                                    L"Players are being signed on.\n\n"
                                    L"Please wait...",
                                    XBFONT_CENTER_X );
    
    g_pMenuFont->DrawText( m_x + 34, m_y + 160, 0xffffffff,
                                    L"\201 Cancel" );
}




//-----------------------------------------------------------------------------
// Name: RenderSignOnFinished()
// Desc: 
//-----------------------------------------------------------------------------
VOID CPlayerScreen::RenderSignOnFinished()
{
    g_pMenuFont->DrawText( m_x + 134, m_y + 10, 0xffffffff, 
                                    m_pPlayer->m_strName, XBFONT_CENTER_X );
 
    if( m_State != STATE_SIGNON_FAILED && 0 == m_pPlayer->m_dwSignOnError )
    {
        if( g_bAllPlayersSignedOn )
        {
            g_pMenuFont->DrawText( m_x + 134, m_y + 30, 0xffffffff, 
                                            L"You are Signed On!",
                                            XBFONT_CENTER_X );

            g_pMenuFont->DrawText( m_x + 34, m_y + 140, 0xffffffff,
                                            L"\200 Let's play!" );
        }
        else
        {
            g_pMenuFont->DrawText( m_x + 134, m_y + 50, 0xffffffff, 
                                            L"Other players are\n"
                                            L"experiencing problems\n"
                                            L"Signing on...",
                                            XBFONT_CENTER_X );
        }
    }
    else
    {
        g_pMenuFont->DrawText( m_x + 134, m_y + 50, 0xffffffff, 
                                        L"Sign on failed",
                                        XBFONT_CENTER_X );
        
        if( m_pPlayer->m_bIsGuestOfUser )
        {
            g_pMenuFont->DrawText( m_x + 134, m_y + 70, 0xffffffff, 
                                            L"The hosting user account\n"
                                            L"could not sign on.",
                                            XBFONT_CENTER_X );
        }
        else
        {
            g_pMenuFont->DrawText( m_x + 134, m_y + 70, 0xffffffff, 
                                            L"Reason <unknown>.",
                                            XBFONT_CENTER_X );
        }
    }

    g_pMenuFont->DrawText( m_x + 34, m_y + 160, 0xffffffff,
                                    L"\201 Change Account" );
}




