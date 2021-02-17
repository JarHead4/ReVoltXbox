//-----------------------------------------------------------------------------
// File: ui_SplashScreen.cpp
//
// Desc: UI implementation
//
// Re-Volt (Generic) Copyright (c) Probe Entertainment 1998
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"
#include "dx.h"
#include "timing.h"
#include "Text.h"
#include "XBOnline.h"
#include "ui_MenuText.h"
#include "ui_MenuDraw.h"
#include "ui_ShowMessage.h"
#include "ui_SplashScreen.h"
#include "ui_TopLevelMenu.h"
#include "ui_LiveSignOn.h"
#include "ui_PlayLive.h"


static VOID CreateSplashScreenMenu( MENU_HEADER* pMenuHeader, MENU* pMenu );
static BOOL HandleSplashScreenMenu( MENU_HEADER* pMenuHeader, DWORD dwInput );
static VOID DrawSplashScreenMenu( MENU_HEADER* pMenuHeader, MENU* pMenu );

static BOOL g_bUserHitStart = FALSE;


//-----------------------------------------------------------------------------
// SplashScreen menu
//-----------------------------------------------------------------------------
extern MENU Menu_SplashScreen = 
{
    TEXT_NONE,
    MENU_DEFAULT | MENU_NOBOX,              // Menu type
    CreateSplashScreenMenu,                 // Create menu function
    HandleSplashScreenMenu,                 // Input handler function
    DrawSplashScreenMenu,                   // Menu draw function
    0,                                      // X coord
    0,                                      // Y Coord
};




// Create Function
VOID CreateSplashScreenMenu( MENU_HEADER* pMenuHeader, MENU* pMenu )
{
    // Reset the time it takes to kick off the demo
    g_fTitleScreenTimer = 0.0f;
    gTitleScreenVars.bAllowDemoToRun = TRUE;

    // Show the Revolt logo
    g_bShowMenuLogo = TRUE;
}




// Menu input handler
BOOL HandleSplashScreenMenu( MENU_HEADER* pMenuHeader, DWORD dwInput )
{
    switch( dwInput )
    {
        case MENU_INPUT_SELECT:
            if( FALSE == g_bUserHitStart )
            {
                g_bUserHitStart = TRUE;
                return TRUE;
            }
    }

    return FALSE;
}




//-----------------------------------------------------------------------------
// Name: DrawSplashScreenMenu()
// Desc: 
//-----------------------------------------------------------------------------
VOID DrawSplashScreenMenu( MENU_HEADER* pMenuHeader, MENU* pMenu )
{
    DrawNewSpruBox( pMenuHeader->m_XPos+320-220.0f, pMenuHeader->m_YPos+400-5.0f, 440.0f, 37.0f );

    DWORD dwAlpha = ((long)(sinf((FLOAT)TIME2MS(TimerCurrent)/200.0f) * 64.0f + 192.0f))<<24L;
    
    BeginTextState();
    g_pFont->DrawText( pMenuHeader->m_XPos+320, pMenuHeader->m_YPos+400, dwAlpha|MENU_COLOR_WHITE,
                       TEXT_TABLE(TEXT_PRESSSTARTTOPLAY), XBFONT_CENTER_X );
}




//-----------------------------------------------------------------------------
// SplashScreen state engine
//-----------------------------------------------------------------------------
CSplashScreenStateEngine g_SplashScreenStateEngine;




//-----------------------------------------------------------------------------
// Name: Process()
// Desc: Main control function for the state engine
//-----------------------------------------------------------------------------
HRESULT CSplashScreenStateEngine::Process()
{
    enum
    {
        SPLASHSCREEN_STATE_BEGIN = STATEENGINE_STATE_BEGIN,
        SPLASHSCREEN_STATE_INITIALMESSAGE,
        SPLASHSCREEN_STATE_ENTER_MAINLOOP,
        SPLASHSCREEN_STATE_MAINLOOP,
        SPLASHSCREEN_STATE_SIGNIN,
        SPLASHSCREEN_STATE_GOTOTOPLEVELMENU,
    };

    switch( m_State )
    {
        // The SplashScreen was just activated
        case SPLASHSCREEN_STATE_BEGIN:
            // No pending menus are allowing before here
            g_pMenuHeader->ClearMenuHeader();

            // If there's a message waiting to appear, deal with that first
            if( InitialMenuMessage != MENU_MESSAGE_NONE ) 
            {
                // Show Bonus Message for a while
                SetBonusMenuMessage();
                InitMenuMessage( 5.0f );
                g_pMenuHeader->SetNextMenu( &Menu_InitialMessage );
                m_State = SPLASHSCREEN_STATE_INITIALMESSAGE;
            }
            else
            {
                // Display the top level menu
                g_pMenuHeader->SetNextMenu( &Menu_SplashScreen );
                g_pTitleScreenCamera->SetNewPos( TITLESCREEN_CAMPOS_INIT );

                m_State = SPLASHSCREEN_STATE_ENTER_MAINLOOP;
            }
            break;

        // The user is viewing an initial message
        case SPLASHSCREEN_STATE_INITIALMESSAGE:
            // Wait till the user is done with the initial message
            if( g_pMenuHeader->m_pMenu ) 
                break;

            // Then, go back to the first state
            m_State = SPLASHSCREEN_STATE_BEGIN;
            break;

        case SPLASHSCREEN_STATE_ENTER_MAINLOOP:
            // Reset the flag for whether the user has yet to hit the start button
            g_bUserHitStart = FALSE;

            m_State = SPLASHSCREEN_STATE_MAINLOOP;
            break;

        // The user is somewhere within the toplevel menu
        case SPLASHSCREEN_STATE_MAINLOOP:
            // Advance to the menu or to sign in, when the user hits start
            if( g_bUserHitStart )
            {
                // If we have a connection...
                if( 0 != XNetGetEthernetLinkStatus() )
                {
                    // And players are not signed in...
                    if( FALSE == g_LiveSignInStateEngine.PlayersSignedIn() )
                    {
                        // And we can init XOnline...
                        XONLINE_STARTUP_PARAMS xosp = { 0 };
                        HRESULT hr = XOnlineStartup( &xosp );

                        if( SUCCEEDED(hr) )
                        {
                            // And there's user accounts available...
                            XONLINE_USER* g_XOnlineUserList;
                            DWORD         g_dwNumUsers;
                            HRESULT hr = XBOnline_GetUserList( &g_XOnlineUserList, &g_dwNumUsers );

                            // Matching cleanup function for XOnlineStartup above.
                            XOnlineCleanup();

                            if( SUCCEEDED(hr) && g_dwNumUsers > 0 )
                            {
                                // Then see if players want to sign in
                                m_State = SPLASHSCREEN_STATE_SIGNIN;
                                break;
                            }
                        }
                    }
                }

                // If the above falls through, don't try to sign in players, but
                // just proceed to the top level menu
                m_State = SPLASHSCREEN_STATE_GOTOTOPLEVELMENU;
            }
            break;

        case SPLASHSCREEN_STATE_SIGNIN:
            g_bShowMenuLogo = FALSE;
            gTitleScreenVars.bAllowDemoToRun = FALSE;

            // When the next state engine returns, make sure we advanced to
            // the toplevelmenu
            m_State = SPLASHSCREEN_STATE_GOTOTOPLEVELMENU;

            // Call the Live Sign In state engine
            g_pActiveStateEngine->Call( &g_LiveSignInStateEngine );
            g_pActiveStateEngine->Process();
            return S_FALSE;

        case SPLASHSCREEN_STATE_GOTOTOPLEVELMENU:
            g_bShowMenuLogo = FALSE;
            gTitleScreenVars.bAllowDemoToRun = FALSE;

            // Set the next state to be the mainloop, in case we return
            // from the top level menu
            m_State = SPLASHSCREEN_STATE_ENTER_MAINLOOP;

            // Advance to the top level menu
            g_pActiveStateEngine->Call( &g_TopLevelMenuStateEngine );
            g_pActiveStateEngine->Process();
            return S_FALSE;
    }

    // Handle the menus
    g_pMenuHeader->HandleMenus();

    return S_FALSE;
}




