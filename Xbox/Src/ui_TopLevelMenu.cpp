//-----------------------------------------------------------------------------
// File: ui_TopLevelMenu.cpp
//
// Desc: UI implementation
//
// Re-Volt (Generic) Copyright (c) Probe Entertainment 1998
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"
#include "dx.h"
#include "LevelLoad.h"
#include "Text.h"
#include "ui_MenuText.h"
#include "ui_MenuDraw.h"
#include "ui_TopLevelMenu.h"
#include "ui_SinglePlayerGame.h"
#include "ui_LiveSignOn.h"
#include "ui_RaceDifficulty.h"
#include "ui_SelectConnection.h"
#include "ui_Confirm.h"
#include "ui_PlayLive.h"
#include "ui_SystemLink.h"
#include "ui_Options.h"
#include "ui_ShowMessage.h"
#include "ui_SelectRaceMode.h"
#include "ui_SelectLanguage.h"
#include "ui_Help.h"
#include "content.h"
#include "verify.h"
#include "net_xonline.h"

#define MENU_TOPLEVEL_XPOS              0
#define MENU_TOPLEVEL_YPOS              0


void CreateTopLevelMenu( MENU_HEADER* pMenuHeader, MENU* pMenu );
BOOL HandleTopLevelMenu( MENU_HEADER* pMenuHeader, DWORD dwInput );



//-----------------------------------------------------------------------------
// Top-level front end menu
//-----------------------------------------------------------------------------
extern MENU Menu_TopLevel = 
{
    TEXT_REVOLT,
    MENU_DEFAULT | MENU_CENTRE_X | MENU_CENTRE_Y,   // Menu type
    CreateTopLevelMenu,                     // Create menu function
    HandleTopLevelMenu,                     // Menu input handler
    NULL,                                   // Menu draw function
    MENU_TOPLEVEL_XPOS,                     // X coord
    MENU_TOPLEVEL_YPOS,                     // Y Coord
};

void DrawTextWithReceivedInviteIcon( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem, int itemIndex )
{
    NET_PLAYER* pPlayer = (NET_PLAYER *)pMenuItem->Data;
    FLOAT fX = pMenuHeader->m_XPos;
    FLOAT fY = pMenuHeader->m_YPos + MENU_TEXT_HEIGHT * itemIndex;

    long col = MENU_TEXT_RGB_NORMAL;
    if( pMenuHeader->m_pCurrentItem == pMenuItem )
    {
        col = MENU_TEXT_RGB_HILITE;
    }
    if( !(pMenuItem->ActiveFlags & MENU_ITEM_ACTIVE) )
    {
        if( pMenuHeader->m_pCurrentItem == pMenuItem )
            col = MENU_TEXT_RGB_MIDLITE;
        else 
            col = MENU_TEXT_RGB_LOLITE;
    }

    // DrawMenuText( fX, fY, col, TEXT_TABLE( pMenuItem->TextIndex ), pMenuHeader->m_ItemTextWidth );
    FLOAT fTextWidth = g_pFont->GetTextWidth( TEXT_TABLE( pMenuItem->TextIndex ) );
    //$HACK: We're always telling the online APIs the user signed in on controller 0.
    if( IsLoggedIn( 0 ) )
    {
        if( XOnlineGetNotification( 0, XONLINE_NOTIFICATION_GAME_INVITE ) )
        {
            DrawScreenSpaceQuad( fX + fTextWidth + 5.0f, fY - 2.0f, g_pGameInviteReceivedTexture );
        }
        else if( XOnlineGetNotification( 0, XONLINE_NOTIFICATION_FRIEND_REQUEST ) )
        {
            DrawScreenSpaceQuad( fX + fTextWidth + 5.0f, fY - 2.0f, g_pFriendReqReceivedTexture );
        }
    }
}

static MENU_ITEM MenuItem_PlayLiveWithInviteIcon =
{
    TEXT_PLAYLIVE,
    40.0f,
    NULL,
    DrawTextWithReceivedInviteIcon,
};

// Create Function
void CreateTopLevelMenu( MENU_HEADER* pMenuHeader, MENU* pMenu )
{
    // Re-enable auto demo running
    gTitleScreenVars.bAllowDemoToRun = TRUE;

    // disable online actions when returning from playlive menu
    gTitleScreenVars.bUseXOnline = false;

    // Show the Revolt logo
//  g_bShowMenuLogo = TRUE;

    // add menu items
    pMenuHeader->AddMenuItem( TEXT_SINGLEPLAYER );
    pMenuHeader->AddMenuItem( TEXT_MULTIPLAYER, MENU_ITEM_INACTIVE );
    pMenuHeader->AddMenuItem( TEXT_SYSTEMLINK );

    // pMenuHeader->AddMenuItem( TEXT_PLAYLIVE ); //$HEY: Don't move this to the top of the list!  Games aren't going to do that, as it isn't very logical.  The UI specs should be updated.
    pMenuHeader->AddMenuItem( &MenuItem_PlayLiveWithInviteIcon );
    pMenuHeader->AddMenuItem( TEXT_OPTIONS );
    pMenuHeader->AddMenuItem( TEXT_REVOLT_HELP);  
    pMenuHeader->AddMenuItem( TEXT_QUIT );

//    // Add in temporary items to help localize languages and also to create
//    // new user accounts
//#if defined( USE_HARDCODED_TEXT )
//    pMenuHeader->AddMenuItem( TEXT_LANGUAGE, MENU_ITEM_INACTIVE );
//#else
//    pMenuHeader->AddMenuItem( TEXT_LANGUAGE );
//#endif
    
    // Add in temporary item to sign in/out
    pMenuHeader->AddMenuItem( TEXT_NONE, MENU_ITEM_INACTIVE );

    if( FALSE == g_LiveSignInStateEngine.PlayersSignedIn() )
        pMenuHeader->AddMenuItem( TEXT_SIGNIN );
    else
        pMenuHeader->AddMenuItem( TEXT_SIGNOUT );
    
////$BUGBUG: This #ifdef is a temp addition for May02_TechBeta (and possibly future beta releases).
//// Eventually, we shouldn't need to allow access to the OnlineDash from in-game.
//#ifdef SHIPPING
//    // Add a top-level menu item to get to the OnlineDash
//    pMenuHeader->AddMenuItem( TEXT_ONLINEDASHBOARD );
//#endif

    GameSettings.MultiType = MULTITYPE_NONE;

    // Fudge to reset the loading message because we don't want it at the start

    // Reset the time it takes to kick off the demo
    g_fTitleScreenTimer = 0.0f;
}


BOOL g_bQuit = FALSE; // $MD: yet another menu / state machine communication global

BOOL HandleTopLevelMenu( MENU_HEADER* pMenuHeader, DWORD dwInput )
{
    switch( dwInput )
    {
        case MENU_INPUT_UP:
            return SelectPreviousMenuItem( pMenuHeader, pMenuHeader->m_pMenu, NULL );
        
        case MENU_INPUT_DOWN:
            return SelectNextMenuItem( pMenuHeader, pMenuHeader->m_pMenu, NULL );

        case MENU_INPUT_BACK:
            if( NULL != g_pActiveStateEngine->GetParent() )
            {
                g_TopLevelMenuStateEngine.Return( STATEENGINE_TERMINATED );
                return TRUE;
            }
            break;

        case MENU_INPUT_SELECT:
            switch( pMenuHeader->m_pMenuItem[pMenuHeader->m_pMenu->CurrentItemIndex]->TextIndex )
            {
                case TEXT_PLAYLIVE:
                    // Init variables
//                  g_bShowMenuLogo = FALSE;
                    gTitleScreenVars.bAllowDemoToRun = FALSE;
                    gTitleScreenVars.bUseXOnline = true;
                    g_pActiveStateEngine->Call( &g_PlayLiveStateEngine );
                    return TRUE;

                case TEXT_SINGLEPLAYER:
                    // Initialise variables
//                  g_bShowMenuLogo = FALSE;
                    gTitleScreenVars.bAllowDemoToRun = FALSE;
                    GameSettings.GameType = GAMETYPE_SINGLE;
                    GameSettings.RandomCars = gTitleScreenVars.RandomCars;
                    GameSettings.RandomTrack = gTitleScreenVars.RandomTrack;
                    gTitleScreenVars.iCurrentPlayer = 0;
                    gTitleScreenVars.numberOfPlayers = 1;

                    g_pActiveStateEngine->Call( &g_SinglePlayerGameStateEngine );
                    return TRUE;

                case TEXT_MULTIPLAYER:
                    // Initialise variables
//                  g_bShowMenuLogo = FALSE;
                    gTitleScreenVars.bAllowDemoToRun = FALSE;
                    GameSettings.GameType = GAMETYPE_NETWORK_RACE;  //$BUG: Is this right?  Recall that "TEXT_MULTIPLAYER" indicates split-screen mode.
                    GameSettings.RandomCars = gTitleScreenVars.RandomCars;
                    GameSettings.RandomTrack = gTitleScreenVars.RandomTrack;
                    gTitleScreenVars.iCurrentPlayer = 0;
                    gTitleScreenVars.numberOfPlayers = 1;
                    gTitleScreenVars.bUseXOnline = false;

                    pMenuHeader->SetNextMenu( &Menu_Connection );
                    return TRUE;

                case TEXT_SYSTEMLINK:
                    // Init variables
//                  g_bShowMenuLogo = FALSE;
                    gTitleScreenVars.bAllowDemoToRun = FALSE;
                    gTitleScreenVars.bUseXOnline = false;

                    g_pActiveStateEngine->Call( &g_SystemLinkStateEngine );
                    return TRUE;

                case TEXT_OPTIONS:
//                  g_bShowMenuLogo = FALSE;
                    g_pActiveStateEngine->Call( &g_OptionsStateEngine );
                    return TRUE;

                // Temporary language option for localizing strings
                case TEXT_LANGUAGE:
//                  g_bShowMenuLogo = FALSE;
                    g_pActiveStateEngine->Call( &g_SelectLanguageStateEngine );
                    return TRUE;

                // Temporary language option for signing on/off
                case TEXT_SIGNIN:
//                  g_bShowMenuLogo = FALSE;
                    // Call the Live Sign In state engine
                    gTitleScreenVars.bAllowDemoToRun = FALSE;
                    g_pActiveStateEngine->Call( &g_LiveSignInStateEngine );
                    return TRUE;

                case TEXT_SIGNOUT:
//                  g_bShowMenuLogo = FALSE;
                    gTitleScreenVars.bAllowDemoToRun = FALSE;
                    g_pActiveStateEngine->Call( &g_LiveSignOutStateEngine );
                    return TRUE;

                case TEXT_REVOLT_HELP:
                    g_pActiveStateEngine->Call( &g_HelpControllerEngine );
                    return TRUE;

                case TEXT_QUIT:
                    g_ShowSimpleMessage.Begin(TEXT_TABLE(TEXT_QUIT), TEXT_TABLE(TEXT_QUIT_GAME),
                                              TEXT_TABLE(TEXT_BUTTON_A_YES), TEXT_TABLE(TEXT_BUTTON_B_NO));
                    g_bQuit = TRUE;
                    return TRUE;

////$BUGBUG: This #ifdef is a temp addition for May02_TechBeta (and possibly future beta releases).
//// Eventually, we shouldn't need to allow access to the OnlineDash from in-game.
//#ifdef SHIPPING
//                case TEXT_ONLINEDASHBOARD:
//                    // Launch the account creation app on the Re-Volt disc
//                    LD_LAUNCH_DASHBOARD ld;
//                    ZeroMemory( &ld, sizeof(ld) );
//                    ld.dwReason = XLD_LAUNCH_DASHBOARD_NEW_ACCOUNT_SIGNUP;
//                    OutputDebugString( "Launching the online dash...\n" );
//                    XLaunchNewImage( NULL, PLAUNCH_DATA( &ld ) ); 
//                    return TRUE;
//#endif
            }
    }

    return FALSE;
}




//-----------------------------------------------------------------------------
// TopLevel Menu state engine
//-----------------------------------------------------------------------------
CTopLevelMenuStateEngine g_TopLevelMenuStateEngine;




//-----------------------------------------------------------------------------
// Name: Process()
// Desc: Main control function for the state engine
//-----------------------------------------------------------------------------
HRESULT CTopLevelMenuStateEngine::Process()
{
    enum
    {
        TOPLEVELMENU_STATE_BEGIN = STATEENGINE_STATE_BEGIN,
        TOPLEVELMENU_STATE_INITIALMESSAGE,
        TOPLEVELMENU_STATE_SELECTLANGUAGE,
        TOPLEVELMENU_STATE_QUIT,
        TOPLEVELMENU_STATE_MAINLOOP,
    };

    switch( m_State )
    {
        // The Top Level menu was just activated
        case TOPLEVELMENU_STATE_BEGIN:
            // Display the top level menu
            g_pMenuHeader->SetNextMenu( &Menu_TopLevel );
            g_pTitleScreenCamera->SetNewPos( TITLESCREEN_CAMPOS_START );
/*
            // On the first time here, have the user select a language
            static BOOL bFirstTime = TRUE;
            if( bFirstTime )
            {
                bFirstTime = FALSE;
                // Select a language
                g_pActiveStateEngine->Call( &g_SelectLanguageStateEngine );
            }
*/
            g_bQuit = FALSE;
            
            // $MD: added message for corrupted content
            //$REVISIT: We had to remove these messages from
            // consumer beta because of the wording
            if(g_bHadCorruptContent)
            {
                // $LOCALIZE
                g_ShowSimpleMessage.Begin(TEXT_TABLE(TEXT_CORRUPT_CONTENT),TEXT_TABLE(TEXT_CORRUPT_CONTENT_REDOWNLOAD),
                                          TEXT_TABLE(TEXT_BUTTON_A_CONTINUE), NULL);
                g_bHadCorruptContent = FALSE;
            }
            else if(g_bSigFailure)
            {
                // $LOCALIZE
                g_ShowSimpleMessage.Begin(TEXT_TABLE(TEXT_CORRUPT_FILES),
                                          TEXT_TABLE(TEXT_CORRUPT_FILES_MAY_HAVE_LOST),
                                          TEXT_TABLE(TEXT_BUTTON_A_CONTINUE), NULL);
                g_bSigFailure = FALSE;
            }
            
            else
                m_State = TOPLEVELMENU_STATE_MAINLOOP;
            break;

        // The user is viewing an initial message
        case TOPLEVELMENU_STATE_INITIALMESSAGE:
            // Wait till the user is done with the initial message
            if( g_pMenuHeader->m_pMenu ) 
                break;

            // Then, go back to the first state
            m_State = TOPLEVELMENU_STATE_BEGIN;
            break;

        // The user is somewhere within the toplevel menu
        case TOPLEVELMENU_STATE_MAINLOOP:
            // This should never happen, but if the top level menu goes away, 
            // bring it back.
            if( g_pMenuHeader->m_pNextMenu == NULL ) 
            {
                g_pMenuHeader->SetNextMenu( &Menu_TopLevel );
                g_pTitleScreenCamera->SetNewPos( TITLESCREEN_CAMPOS_START );
            }

            if(g_bQuit)
            {
                m_State = TOPLEVELMENU_STATE_QUIT;
                g_bQuit = FALSE;
            }

            break;

        case TOPLEVELMENU_STATE_QUIT:
            
            if(g_ShowSimpleMessage.GetStatus() == STATEENGINE_COMPLETED)
            {
                //launch default.xbe on DVD
                XLaunchNewImage("D:\\default.xbe", NULL);
                
                // just in case, go back to main loop
                assert( FALSE );
                m_State = TOPLEVELMENU_STATE_MAINLOOP;
            }
            else
                m_State = TOPLEVELMENU_STATE_MAINLOOP;
            break;
    }

    // Handle the menus
    g_pMenuHeader->HandleMenus();

    return S_FALSE;
}




