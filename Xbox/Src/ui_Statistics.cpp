//-----------------------------------------------------------------------------
// File: ui_Statistics.cpp
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"
#include <string.h>
#include "dx.h"
#include "input.h"
#include "text.h"
#include "ui_Menu.h"
#include "ui_MenuText.h"
#include "ui_MenuDraw.h"
#include "ui_TitleScreen.h"
#include "ui_StateEngine.h"

#include "net_xonline.h"
#include "net_Statistics.h"
#include "ui_Animation.h"
#include "ui_Statistics.h"
#include "ui_ShowMessage.h"


static CUIStatsRacerTrackRankList g_UITrackRanks;

static VOID CreateStatisticsMenu( MENU_HEADER* pMenuHeader, MENU* pMenu );
static BOOL HandleStatisticsMenu( MENU_HEADER* pMenuHeader, DWORD dwInput );

#define MENU_XPOS_STATISTICS    100
#define MENU_YPOS_STATISTICS    150


//-----------------------------------------------------------------------------
// StatisticsMenu menu
//-----------------------------------------------------------------------------
extern MENU Menu_StatisticsMenu = 
{
    TEXT_RACERSTATISTICS,
    MENU_DEFAULT | MENU_CENTRE_X,           // Menu type
    CreateStatisticsMenu,                   // Create menu function
    HandleStatisticsMenu,                   // Input handler function
    NULL,                                   // Menu draw function
    MENU_XPOS_STATISTICS,                   // X coord
    MENU_YPOS_STATISTICS,
};


VOID CreateStatisticsMenu(MENU_HEADER *pMenuHeader, MENU *pMenu)
{
    // Add pMenu items
    pMenuHeader->AddMenuItem( TEXT_MYSTATISTICS );
    pMenuHeader->AddMenuItem( TEXT_FRIENDSSTATISTICS );
    pMenuHeader->AddMenuItem( TEXT_WORLDSBEST );
}


BOOL HandleStatisticsMenu( MENU_HEADER* pMenuHeader, DWORD dwInput )
{
    switch( dwInput )
    {
        case MENU_INPUT_BACK:
//$MODIFIED
            g_UIStatisticsMenu.Return( STATEENGINE_TERMINATED );
//            g_PlayLiveStateEngine.ProcessExitState();
//$END_MODIFICATIONS
            return TRUE;

        case MENU_INPUT_UP:
            return SelectPreviousMenuItem( pMenuHeader, pMenuHeader->m_pMenu, pMenuHeader->m_pCurrentItem );
    
        case MENU_INPUT_DOWN:
            return SelectNextMenuItem( pMenuHeader, pMenuHeader->m_pMenu, pMenuHeader->m_pCurrentItem );
    
        case MENU_INPUT_SELECT:
            switch( pMenuHeader->m_pCurrentItem->TextIndex )
            {
                case TEXT_MYSTATISTICS:
                {
#ifdef XONLINE_OFFLINE
                    XONLINE_USER dummyUser;
                    dummyUser.xuid = { pMenuHeader->m_nLastControllerInput, 0 };
                    int x = pMenuHeader->m_nLastControllerInput;
                    sprintf( dummyUser.szGamertag, "User%d%d%d%d%d%d", x,x,x,x,x,x);
                    XONLINE_USER *pUser = &dummyUser;
#else
                    Assert( pMenuHeader->m_nLastControllerInput != -1 );
                    // $SINGLEPLAYER hack: need to know which controller each player is mapped to
                    // int nPlayerIndex = pMenuHeader->m_nLastControllerInput;
                    int nPlayerIndex = 0;
                    Assert( Players[ nPlayerIndex ].XOnlineInfo.pXOnlineUser );
                    XONLINE_USER *pUser = Players[ nPlayerIndex ].XOnlineInfo.pXOnlineUser;
#endif

                    g_UIStatisticsMenu.m_UIUserSummary.Init( pUser, STAT_LB_RACING_OVERALL );
                    g_UIStatisticsMenu.Call( &g_UIStatisticsMenu.m_UIUserSummary );
                    return TRUE;
                }
                case TEXT_FRIENDSSTATISTICS:
                {
                    g_UIStatisticsMenu.m_UIRacerRankList.Init( STAT_LB_RACING_OVERALL );
                    g_UIStatisticsMenu.Call( &g_UIStatisticsMenu.m_UIRacerRankList );
                    return TRUE;
                }
                case TEXT_WORLDSBEST:
                {
                    g_UIStatisticsMenu.m_UIWorldLeaderRankList.Init( STAT_LB_RACING_OVERALL );
                    g_UIStatisticsMenu.Call( &g_UIStatisticsMenu.m_UIWorldLeaderRankList );
                    return TRUE;
                }
            }
            break;

#ifdef _DEBUG
            case MENU_INPUT_DEBUG_WHITE:
            {
            // Clear out this users stats
                XONLINETASK_HANDLE hTask = NULL;
                // $SINGLEPLAYER gotta get this single player stuff worked out...
                HRESULT hr = XOnlineStatReset( Players[0].XOnlineInfo.pXOnlineUser->xuid, 0, NULL, &hTask);

                if ( SUCCEEDED( hr) )
                {
                    OnlineTasks_Add( hTask, TASK_STAT_RESET_USER );
                }
                else
                {
                    swprintf( g_SimpleMessageBuffer,
                              L"XOnlineStatReset for user\n"
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

                g_RacerStats.PurgeStats();
                g_BattlerStats.PurgeStats();
                g_UserTrackRanks.PurgeStats();
            }
#endif
    }

    return FALSE;
}




CUIStatisticsMenu g_UIStatisticsMenu;

//-----------------------------------------------------------------------------
// Name: Process()
// Desc: Main control function for the state engine
//-----------------------------------------------------------------------------
HRESULT CUIStatisticsMenu::Process()
{
    enum
    {
        STATISTICSMENU_STATE_BEGIN = STATEENGINE_STATE_BEGIN,
        STATISTICSMENU_STATE_MAINLOOP,
    };

    switch( m_State )
    {
        case STATISTICSMENU_STATE_BEGIN:
            g_pMenuHeader->SetNextMenu( &Menu_StatisticsMenu );
            g_pTitleScreenCamera->SetNewPos( TITLESCREEN_CAMPOS_TROPHYALL );
            m_State = STATISTICSMENU_STATE_MAINLOOP;
            break;

        case STATISTICSMENU_STATE_MAINLOOP:
            // Nothing to do, as control is in the menus
            break;
    }

    // Handle the menus
    g_pMenuHeader->HandleMenus();

    return S_FALSE;
}

VOID CUIStatisticsMenu::HandleExitToParent()
{ 
    g_RacerStats.PurgeStats();
    g_BattlerStats.PurgeStats();
    g_UserTrackRanks.PurgeStats();
}



////////////////////////////////////////////////////////////////
//
// Racer Rank List
//
////////////////////////////////////////////////////////////////
CUIStatsRacerRankList::CUIStatsRacerRankList()
{
    m_pDisplayedRanks = NULL;
    m_nLocalPlayers = 0;
}

HRESULT CUIStatsRacerRankList::Init( DWORD dwLeaderBoardID )
{
    Assert( ( STAT_LB_RACING_OVERALL == dwLeaderBoardID ) || ( STAT_LB_RACING_OVERALL == dwLeaderBoardID ) );

    m_List.Clear( 10 );

    m_RacingRanks.Reset( STAT_LB_RACING_OVERALL );
    m_BattleRanks.Reset( STAT_LB_BATTLE_OVERALL );

    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: Process()
// Desc: Main control function for the state engine
//-----------------------------------------------------------------------------
HRESULT CUIStatsRacerRankList::Process()
{
    HRESULT hr = S_OK;

    switch( m_State )
    {
        case STATE_BEGIN:
            m_pDisplayedRanks = &m_RacingRanks;

            g_pMenuHeader->SetNextMenu( NULL );
            //g_pTitleScreenCamera->SetNewPos( TITLESCREEN_CAMPOS_TRACK_SELECT );

            m_State = STATE_REQUEST_FRIENDS_LIST;
            // fall through

        case STATE_REQUEST_FRIENDS_LIST:
            m_StatFriends.StartRequest();
            m_State = STATE_WAIT_FOR_FRIENDS_LIST;
            break;
            
        case STATE_WAIT_FOR_FRIENDS_LIST:
            if ( m_StatFriends.m_hTask )
            {
                hr = m_StatFriends.TaskContinue();

                // Stat friends closes task once list is retrieved.
                if ( NULL == m_StatFriends.m_hTask )
                {
                    m_State = STATE_REQUEST_STATS;
                }
            }
            break;

        case STATE_REQUEST_STATS:
            // hrStatus inited to S_FALSE, stats not yet retrieved
            if ( S_FALSE == m_pDisplayedRanks->m_hrStatus )
            {
#ifdef XONLINE_OFFLINE
                int nNumTestUsers = 6;
                for( int i = 0 ; i < nNumTestUsers ; i++ )
                {
                    XUID xuidTestDummy = { i, 0 };
                    m_pDisplayedRanks->AddUser( xuidTestDummy );
                }
                m_List.m_nNumberInList = nNumTestUsers;
#else
                m_List.m_nNumberInList = 0;
                m_nLocalPlayers = 0;
                for( int i = 0 ; i < MAX_LOCAL_PLAYERS ; i++ )
                {
                    if ( Players[i].XOnlineInfo.pXOnlineUser )
                    {
                        m_pDisplayedRanks->AddUser( Players[i].XOnlineInfo.pXOnlineUser );
                        m_List.m_nNumberInList++;
                        m_nLocalPlayers++;
                    }
                }

                for( DWORD dwFriend = 0; dwFriend < m_StatFriends.m_dwNumFriends; dwFriend++ )
                {

                    for (int nLocalPlayer = 0; nLocalPlayer < m_nLocalPlayers; nLocalPlayer++)
                    {
                        if ( m_pDisplayedRanks->m_Specs[nLocalPlayer].xuidUser.qwUserID 
                                == m_StatFriends.m_Friends[ dwFriend ].xuid.qwUserID ) 
                        {
                            // found a friend is also local player
                            break;
                        }
                    }

                    if (nLocalPlayer == m_nLocalPlayers)
                    {
                        // this friend is not local, so not yet in list
                        XONLINE_USER User;
                        strcpy( User.szGamertag, m_StatFriends.m_Friends[dwFriend].szGamertag);
                        User.xuid = m_StatFriends.m_Friends[dwFriend].xuid;

                        m_pDisplayedRanks->AddUser( &User );
                        m_List.m_nNumberInList++;
                    }

                }
#endif

                hr = m_pDisplayedRanks->RequestStats();
                m_State = STATE_WAIT_FOR_STATS;
            }
            else
            {
                // stats already cached; go to got stats
                m_State = STATE_GOT_STATS;
            } 
            break;

        case STATE_WAIT_FOR_STATS:
            if ( m_pDisplayedRanks->m_hTask )
            {
                hr = m_pDisplayedRanks->TaskContinue();

                if (XONLINETASK_S_RUNNING != hr)
                {
                    if ( SUCCEEDED( hr ) )
                    {
                        m_State = STATE_GOT_STATS;
                    }
                    else
                    {
                        Assert( SUCCEEDED( hr ) );
                        m_State = STATE_STATS_ERROR;
                    }
                }
            }
            break;

        case STATE_GOT_STATS:
            m_List.Clear();
            m_List.m_nNumberInList = m_pDisplayedRanks->m_nUsers;
            m_State = STATE_MAINLOOP;
            break;

        case STATE_STATS_ERROR:
            // Nothing to do, as control is in the menus
            break;

        case STATE_MAINLOOP:
            // Nothing to do, as control is in the menus
            break;
    }

    // Handle the menus
    //g_pMenuHeader->HandleMenus();
    //HandleInput();
    //Draw();
    hr = CUIScreen::Process();

    return S_FALSE;
}

bool CUIStatsRacerRankList::HandleInput( MenuInputTypeEnum input)
{
    // Read Input
    //MenuInputTypeEnum input = GetFullScreenMenuInput();
    BOOL bMenuInputProcessed = FALSE;


    // Process Input
    switch( input )
    {
        case MENU_INPUT_UP:
            bMenuInputProcessed = m_List.ScrollUp();
            break;

        case MENU_INPUT_DOWN:
            bMenuInputProcessed = m_List.ScrollDown();
            break;

        case MENU_INPUT_BACK:
            Return( STATEENGINE_TERMINATED );
            bMenuInputProcessed = TRUE;
            break;

        case MENU_INPUT_SELECT:
        {
            // Racer is selected show a summary
            m_UIUserSummary.Init(  &m_pDisplayedRanks->m_Users[ m_List.m_nCurrentSelection], 
                                   m_pDisplayedRanks->m_dwLeaderBoardID );
            Call( &m_UIUserSummary );

            // call player summary stat
            bMenuInputProcessed = TRUE;
            break;
        }

        case MENU_INPUT_X:
            // toggle beteen battle and race
            if ( m_pDisplayedRanks == &m_RacingRanks )
                m_pDisplayedRanks = &m_BattleRanks;
            else
                m_pDisplayedRanks = &m_RacingRanks;

            m_State = STATE_REQUEST_STATS;
            bMenuInputProcessed = TRUE;
            break;

    }

    if (bMenuInputProcessed)
    {
        g_SoundEngine.Play2DSound( gMenuInputSFXIndex[input], FALSE );
    }

    if( input != MENU_INPUT_NONE )
    {
        g_fTitleScreenTimer = 0.0f;
    }


    return bMenuInputProcessed;
}


void CUIStatsRacerRankList::Draw()
{
//    LEVELINFO* pLevelInfo = GetLevelInfo(gTitleScreenVars.iLevelNum);
    BeginTextState();

    m_RC.m_Title.DrawNewTitleBox( 0,  23, 640, 48, 0xffffffff, 0 );

    m_RC.m_Panel.DrawNewSpruBox( 40 * RenderSettings.GeomScaleX,
                    70 * RenderSettings.GeomScaleY,
                    (FLOAT)(640 - 80) * RenderSettings.GeomScaleX,
                    (FLOAT)(480 - 140) * RenderSettings.GeomScaleY );

    BeginTextState();

    if ( STAT_LB_RACING_OVERALL == m_pDisplayedRanks->m_dwLeaderBoardID )
        m_RC.m_Title.DrawMenuTitleText( 320, 30, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_STATS_LOCALRACERRANKS), XBFONT_CENTER_X );
    else
        m_RC.m_Title.DrawMenuTitleText( 320, 30, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_STATS_LOCALBATTLERRANKS), XBFONT_CENTER_X );


    float xPosNameLeft = 60;
    float xPosRankRight = 640 - 90;
    float ySize = MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP;
    float yPos = 76;

    // column headers
    if ( STAT_LB_RACING_OVERALL == m_pDisplayedRanks->m_dwLeaderBoardID )
        m_RC.m_Panel.DrawMenuTitleText( xPosNameLeft,  yPos, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_STATS_RACERNAME), XBFONT_LEFT );
    else
        m_RC.m_Panel.DrawMenuTitleText( xPosNameLeft,  yPos, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_STATS_BATTLERNAME), XBFONT_LEFT );

    m_RC.m_Panel.DrawMenuTitleText( xPosRankRight, yPos, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_STATS_RANK), XBFONT_RIGHT );
    yPos += 30;

    int nItemIndex;
    WCHAR strRank[20];
    DWORD dwItemColor = 0;
    for ( int i = 0; i < m_List.m_nNumberOfVisibleItems; i++ )
    {
        nItemIndex = m_List.m_nFirstVisible + i;
        
        if ( nItemIndex >= m_List.m_nNumberInList )
            break;

        // skip if stats not yet retrieved
        if ( m_pDisplayedRanks->m_nUsers )
        {
            WCHAR strUserID[ XONLINE_GAMERTAG_SIZE ];
            swprintf(strRank, L"%d",        m_pDisplayedRanks->GetUserRank(nItemIndex));
#ifdef XONLINE_OFFLINE
            swprintf(strUserID, L"OFFLINE_User%d", nItemIndex);
#else
            // convert to WCHAR
            swprintf(strUserID, L"%S", m_pDisplayedRanks->m_Users[i].szGamertag);
#endif


            if ( i == m_List.m_nCurrentSelection ) 
            {
                dwItemColor = MENU_TEXT_RGB_HILITE;
            }
            else
            {
                dwItemColor = MENU_TEXT_RGB_NORMAL;
            }

            m_RC.m_Panel.DrawMenuText( xPosNameLeft,  yPos, dwItemColor, strUserID, XBFONT_LEFT );
            m_RC.m_Panel.DrawMenuText( xPosRankRight, yPos, dwItemColor, strRank, XBFONT_RIGHT );
            yPos += ySize;

        }

    }

    // action buttons
    yPos += ySize;
    yPos += ySize;
    if ( STAT_LB_RACING_OVERALL == m_pDisplayedRanks->m_dwLeaderBoardID )
    {
        // "A Button" action
        m_RC.m_Panel.DrawMenuText( xPosRankRight, yPos, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_STATS_A_RACERDETAILS), XBFONT_RIGHT );
        yPos += ySize;

        // "X Button" action
        m_RC.m_Panel.DrawMenuText( xPosRankRight, yPos, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_STATS_X_BATTLEMODESTATS), XBFONT_RIGHT );
        yPos += ySize;
    }
    else
    {
        // "A Button" action
        m_RC.m_Panel.DrawMenuText( xPosRankRight, yPos, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_STATS_A_BATTLERDETAILS), XBFONT_RIGHT );
        yPos += ySize;

        // "X Button" action
        m_RC.m_Panel.DrawMenuText( xPosRankRight, yPos, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_STATS_X_RACEMODESTATS), XBFONT_RIGHT );
        yPos += ySize;
    }

}

////////////////////////////////////////////////////////////////
//
// World Leader Racer Rank List
//
////////////////////////////////////////////////////////////////
CUIStatsWorldLeaderRankList::CUIStatsWorldLeaderRankList()
{
    m_pDisplayedRanks = NULL;
}

HRESULT CUIStatsWorldLeaderRankList::Init( DWORD dwLeaderBoardID )
{
    Assert( ( STAT_LB_RACING_OVERALL == dwLeaderBoardID ) || ( STAT_LB_RACING_OVERALL == dwLeaderBoardID ) );

    m_List.Clear( 10 );

    m_RacingRanks.Reset( STAT_LB_RACING_OVERALL, m_List.m_nNumberOfVisibleItems );
    m_BattleRanks.Reset( STAT_LB_BATTLE_OVERALL, m_List.m_nNumberOfVisibleItems );

    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: Process()
// Desc: Main control function for the state engine
//-----------------------------------------------------------------------------
HRESULT CUIStatsWorldLeaderRankList::Process()
{
    HRESULT hr = S_OK;

    switch( m_State )
    {
        case STATE_BEGIN:
            m_pDisplayedRanks = &m_RacingRanks;

            g_pMenuHeader->SetNextMenu( NULL );
            //g_pTitleScreenCamera->SetNewPos( TITLESCREEN_CAMPOS_TRACK_SELECT );

            m_State = STATE_REQUEST_STATS;
            // fall through

        case STATE_REQUEST_STATS:
            if ( 0 == m_pDisplayedRanks->m_dwReturnedResults )
            {
                hr = m_pDisplayedRanks->RequestStats();
                m_State = STATE_WAIT_FOR_RESULTS;
            }
            else
            {
                // stats already cached; go to got stats
                m_State = STATE_GOT_STATS;
            } 
            break;

        case STATE_WAIT_FOR_RESULTS:
            if ( m_pDisplayedRanks->m_hTask )
            {
                hr = m_pDisplayedRanks->TaskContinue();

                if (XONLINETASK_S_RUNNING != hr)
                {
                    if ( SUCCEEDED( hr ) )
                    {
                        m_State = STATE_GOT_STATS;
                    }
                    else
                    {
                        Assert( SUCCEEDED( hr ) );
                        m_State = STATE_STATS_ERROR;
                    }
                }
            }
            break;

        case STATE_GOT_STATS:
            m_List.Clear();
            m_List.m_nNumberInList = m_pDisplayedRanks->m_dwReturnedResults;
            m_State = STATE_MAINLOOP;
            break;


        case STATE_STATS_ERROR:
            // Nothing to do, as control is in the menus
            break;

        case STATE_MAINLOOP:
            // Nothing to do, as control is in the menus
            break;
    }

    // Handle the menus
    //g_pMenuHeader->HandleMenus();
    //HandleInput();
    //Draw();
    hr = CUIScreen::Process();

    return S_FALSE;
}

bool CUIStatsWorldLeaderRankList::HandleInput( MenuInputTypeEnum input)
{
    // Read Input
    //MenuInputTypeEnum input = GetFullScreenMenuInput();
    BOOL bMenuInputProcessed = FALSE;


    // Process Input
    switch( input )
    {
        case MENU_INPUT_UP:
            bMenuInputProcessed = m_List.ScrollUp();
            break;

        case MENU_INPUT_DOWN:
            bMenuInputProcessed = m_List.ScrollDown();
            break;

        case MENU_INPUT_BACK:
            Return( STATEENGINE_TERMINATED );
            bMenuInputProcessed = TRUE;
            break;

        case MENU_INPUT_SELECT:
            if ( m_List.m_nNumberInList > 0 )
            {
                // Racer is selected show a summary

                // we want to reuse the stat summary screen, but it needs xuid and szGamertag
                // pack these three into struct for call into child UI
                // REVIEW: hopefully the XONLINE_STAT_USER and XONLINE_USER will become more alike. either way $revisit this code!!!
                XONLINE_STAT_USER *pUser = &m_pDisplayedRanks->m_Users[ m_List.m_nCurrentSelection ];
                static XONLINE_USER tmpUser;  //$REVISIT: static var here? Ugly.
                memset( &tmpUser, 0, sizeof(tmpUser) );
                tmpUser.xuid = pUser->xuidUser;
                strcpy( tmpUser.szGamertag, pUser->szGamertag );
                m_UIUserSummary.Init(  &tmpUser, m_pDisplayedRanks->m_dwLeaderBoardID );

                Call( &m_UIUserSummary );

                // call player summary stat
                bMenuInputProcessed = TRUE;
            }
            break;

        case MENU_INPUT_X:
            // toggle beteen battle and race
            if ( m_pDisplayedRanks == &m_RacingRanks )
                m_pDisplayedRanks = &m_BattleRanks;
            else
                m_pDisplayedRanks = &m_RacingRanks;

            m_State = STATE_REQUEST_STATS;
            bMenuInputProcessed = TRUE;
            break;
    }

    if (bMenuInputProcessed)
    {
        g_SoundEngine.Play2DSound( gMenuInputSFXIndex[input], FALSE );
    }

    if( input != MENU_INPUT_NONE )
    {
        g_fTitleScreenTimer = 0.0f;
    }


    return bMenuInputProcessed;
}


void CUIStatsWorldLeaderRankList::Draw()
{
//    LEVELINFO* pLevelInfo = GetLevelInfo(gTitleScreenVars.iLevelNum);
    BeginTextState();

    m_RC.m_Title.DrawNewTitleBox(0,  23, 640, 48, 0xffffffff, 0 );

    m_RC.m_Panel.DrawNewSpruBox( 40 * RenderSettings.GeomScaleX,
                    70 * RenderSettings.GeomScaleY,
                    (FLOAT)(640 - 80) * RenderSettings.GeomScaleX,
                    (FLOAT)(480 - 140) * RenderSettings.GeomScaleY );

    BeginTextState();

    if ( STAT_LB_RACING_OVERALL == m_pDisplayedRanks->m_dwLeaderBoardID )
        m_RC.m_Title.DrawMenuTitleText( 320, 30, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_STATS_CHAMPRACERSTATS), XBFONT_CENTER_X );
    else
        m_RC.m_Title.DrawMenuTitleText( 320, 30, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_STATS_CHAMPBATTLERSTATS), XBFONT_CENTER_X );


    float xPosNameLeft = 60;
    float xPosRankRight = 640 - 90;
    float ySize = MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP;
    float yPos = 76;

    // column headers
    if ( STAT_LB_RACING_OVERALL == m_pDisplayedRanks->m_dwLeaderBoardID )
        m_RC.m_Panel.DrawMenuTitleText( xPosNameLeft,  yPos, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_STATS_RACERNAME), XBFONT_LEFT );
    else
        m_RC.m_Panel.DrawMenuTitleText( xPosNameLeft,  yPos, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_STATS_BATTLERNAME), XBFONT_LEFT );

    m_RC.m_Panel.DrawMenuTitleText( xPosRankRight, yPos, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_STATS_RANK), XBFONT_RIGHT );
    yPos += 30;

    int   nItemIndex;
    DWORD dwItemColor = 0;
    WCHAR strRank[20];
    for ( int i = 0; i < m_List.m_nNumberOfVisibleItems; i++ )
    {
        nItemIndex = m_List.m_nFirstVisible + i;
        
        if ( nItemIndex >= m_List.m_nNumberInList )
            break;

        swprintf(strRank, L"%d",    m_pDisplayedRanks->GetUserRank(nItemIndex));
        WCHAR strUserID[ XONLINE_GAMERTAG_SIZE ];
        swprintf(strUserID, L"%S",  m_pDisplayedRanks->m_Users[ nItemIndex ].szGamertag);

        if ( i == m_List.m_nCurrentSelection ) 
        {
            dwItemColor = MENU_TEXT_RGB_HILITE;
        }
        else
        {
            dwItemColor = MENU_TEXT_RGB_NORMAL;
        }

        m_RC.m_Panel.DrawMenuText( xPosNameLeft,  yPos, dwItemColor, strUserID, XBFONT_LEFT );
        m_RC.m_Panel.DrawMenuText( xPosRankRight, yPos, dwItemColor, strRank, XBFONT_RIGHT );
        yPos += ySize;

    }

    // action buttons
    yPos += ySize;
    yPos += ySize;
    if ( STAT_LB_RACING_OVERALL == m_pDisplayedRanks->m_dwLeaderBoardID )
    {
        // "A Button" action
        m_RC.m_Panel.DrawMenuText( xPosRankRight, yPos, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_STATS_A_RACERDETAILS), XBFONT_RIGHT );
        yPos += ySize;

        // "X Button" action
        m_RC.m_Panel.DrawMenuText( xPosRankRight, yPos, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_STATS_X_BATTLEMODESTATS), XBFONT_RIGHT );
        yPos += ySize;
    }
    else
    {
        // "A Button" action
        Assert( STAT_LB_BATTLE_OVERALL == m_pDisplayedRanks->m_dwLeaderBoardID );
        m_RC.m_Panel.DrawMenuText( xPosRankRight, yPos, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_STATS_A_BATTLERDETAILS), XBFONT_RIGHT );
        yPos += ySize;

        // "X Button" action
        m_RC.m_Panel.DrawMenuText( xPosRankRight, yPos, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_STATS_X_RACEMODESTATS), XBFONT_RIGHT );
        yPos += ySize;
    }

}



////////////////////////////////////////////////////////////////
//
// Racer Track Rank List
//
////////////////////////////////////////////////////////////////
CUIStatsRacerTrackRankList::CUIStatsRacerTrackRankList()
{
    m_pStatRanks = NULL;
    m_State = STATE_BEGIN;

}

HRESULT CUIStatsRacerTrackRankList::Init( XONLINE_USER *pUser, bool bBattleMode )
{
    HRESULT hr = S_OK;

    Assert( pUser );
    m_User = *pUser;
    m_bBattleMode = bBattleMode;

    m_List.Clear( 10 );
    m_pStatRanks = NULL;
    m_State = STATE_BEGIN;

    PlayerModeKey key;
    key.xuid = m_User.xuid;
    key.bBattleMode = bBattleMode;


    if ( g_UserTrackRanks.find( key ) != g_UserTrackRanks.end() )
    {
        m_pStatRanks = g_UserTrackRanks[ key ];
    }
    else
    {

        m_pStatRanks = new CStatUserTrackRanks();
        if ( m_pStatRanks )
        {
            m_pStatRanks->Reset( &m_User, bBattleMode );
            hr = m_pStatRanks->RequestStats();
            g_UserTrackRanks.insert( UserTrackRanks::value_type( key, m_pStatRanks ) );
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

    return hr;
}

//-----------------------------------------------------------------------------
// Name: Process()
// Desc: Main control function for the state engine
//-----------------------------------------------------------------------------
HRESULT CUIStatsRacerTrackRankList::Process()
{

    HRESULT hr;

    switch( m_State )
    {
        case STATE_BEGIN:
        {
            g_pMenuHeader->SetNextMenu( NULL );
            //g_pTitleScreenCamera->SetNewPos( TITLESCREEN_CAMPOS_TRACK_SELECT );

            m_State = STATE_WAIT_FOR_RESULTS;
            break;
        }

        case STATE_WAIT_FOR_RESULTS:
            if ( m_pStatRanks->m_hTask )
            {
                hr = m_pStatRanks->TaskContinue();

                if (XONLINETASK_S_RUNNING != hr)
                {
                    if ( SUCCEEDED( hr ) )
                    {
                        m_State = STATE_GOT_STATS;
                    }
                    else
                    {
                        Assert( SUCCEEDED( hr ) );
                        m_State = STATE_STATS_ERROR;
                    }
                }
            }
            else
            {
                m_State = STATE_GOT_STATS;
            }
            break;

        case STATE_GOT_STATS:
            m_List.m_nNumberInList = m_pStatRanks->m_nRaceLevels;
            m_State = STATE_MAINLOOP;
            break;

        case STATE_STATS_ERROR:
            // Nothing to do, as control is in the menus
            break;

        case STATE_MAINLOOP:
            // Nothing to do, as control is in the menus
            break;
    }

    // Handle the menus
    //g_pMenuHeader->HandleMenus();
    //HandleInput();
    //Draw();
    hr = CUIScreen::Process();

    return S_FALSE;
}

bool CUIStatsRacerTrackRankList::HandleInput( MenuInputTypeEnum input )
{
    // Read Input
    //MenuInputTypeEnum input = GetFullScreenMenuInput();
    BOOL bMenuInputProcessed = FALSE;


    // Process Input
    switch( input )
    {
        case MENU_INPUT_UP:
            bMenuInputProcessed = m_List.ScrollUp();
            break;

        case MENU_INPUT_DOWN:
            bMenuInputProcessed = m_List.ScrollDown();
            break;

        case MENU_INPUT_BACK:
            m_pStatRanks = NULL;
            m_State = STATE_BEGIN;
            Return( STATEENGINE_TERMINATED );
            bMenuInputProcessed = TRUE;
            break;

        case MENU_INPUT_SELECT:
        {
            m_UIDetail.Init(  &m_User, m_pStatRanks->m_Specs[ m_List.m_nCurrentSelection].dwLeaderBoardID );
            Call( &m_UIDetail );
            bMenuInputProcessed = TRUE;
            break;
        }

        case MENU_INPUT_X:
            if ( NULL == m_pStatRanks->m_hTask )
            {
                // toggle beteen battle and race
                m_bBattleMode = !m_bBattleMode;
                Init( &m_User, m_bBattleMode );
                m_State = STATE_WAIT_FOR_RESULTS;
                bMenuInputProcessed = TRUE;
            }
            break;
    }

    if (bMenuInputProcessed)
    {
        g_SoundEngine.Play2DSound( gMenuInputSFXIndex[input], FALSE );
    }

    if( input != MENU_INPUT_NONE )
    {
        g_fTitleScreenTimer = 0.0f;
    }


    return bMenuInputProcessed;
}

const float c_yposAButton = 360.0f;
const float c_yposXButton = c_yposAButton + 20.0f;

void CUIStatsRacerTrackRankList::Draw()
{
//    LEVELINFO* pLevelInfo = GetLevelInfo(gTitleScreenVars.iLevelNum);
    BeginTextState();

    m_RC.m_Title.DrawNewTitleBox(0,  23, 640, 48, 0xffffffff, 0 );

    m_RC.m_Panel.DrawNewSpruBox( 40 * RenderSettings.GeomScaleX,
                    70 * RenderSettings.GeomScaleY,
                    (FLOAT)(640 - 80) * RenderSettings.GeomScaleX,
                    (FLOAT)(480 - 140) * RenderSettings.GeomScaleY );

    BeginTextState();

    if ( STATE_MAINLOOP == m_State )
    {
        WCHAR strUserID[ XONLINE_GAMERTAG_SIZE ];
#ifdef XONLINE_OFFLINE
        swprintf(strUserID, L"OFFLINE_User");
#else
        swprintf(strUserID, L"%S", m_User.szGamertag);
#endif

        if ( m_bBattleMode )
        {
            m_RC.m_Title.DrawMenuTitleText( 320, 30, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_STATS_BATTLETRACKRANKS), XBFONT_CENTER_X );
        }
        else
        {
            m_RC.m_Title.DrawMenuTitleText( 320, 30, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_STATS_RACINGTRACKRANKS), XBFONT_CENTER_X );
        }

        m_RC.m_Panel.DrawMenuTitleText( 320, 80, MENU_TEXT_RGB_NORMAL, strUserID, XBFONT_CENTER_X );


        float xPosNameLeft = 120;
        float xPosRankRight = 640 - 150;
        float ySize = MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP;
        float yPos = 110;

        m_RC.m_Panel.DrawMenuTitleText( xPosNameLeft,  yPos, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_STATS_TRACK), XBFONT_LEFT );
        m_RC.m_Panel.DrawMenuTitleText( xPosRankRight, yPos, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_STATS_RANK), XBFONT_RIGHT );
        yPos += 30;

        int nItemIndex;
        WCHAR strRank[20];
        DWORD dwItemColor = 0;
        WCHAR strTrack[ MAX_LEVEL_NAME];
        for ( int i = 0; i < m_List.m_nNumberOfVisibleItems; i++ )
        {
            nItemIndex = m_List.m_nFirstVisible + i;
        
            if ( nItemIndex >= m_List.m_nNumberInList )
                break;

            swprintf(strRank, L"%d", m_pStatRanks->m_Stats[ nItemIndex ].lValue );
            swprintf(strTrack, L"%s", DefaultLevelInfo[ TrackLevel( m_pStatRanks->m_Specs[ nItemIndex ].dwLeaderBoardID ) ].strName );

            if ( i == (m_List.m_nCurrentSelection - m_List.m_nFirstVisible ) ) 
            {
                dwItemColor = MENU_TEXT_RGB_HILITE;
            }
            else
            {
                dwItemColor = MENU_TEXT_RGB_NORMAL;
            }

            m_RC.m_Panel.DrawMenuText( xPosNameLeft,  yPos, dwItemColor, strTrack, XBFONT_LEFT );
            m_RC.m_Panel.DrawMenuText( xPosRankRight, yPos, dwItemColor, strRank, XBFONT_RIGHT );
            yPos += ySize;

        }

        // "A Button" action
        yPos += ySize;
        yPos += ySize;
        if ( m_bBattleMode )
        {
            // "A Button" action
            m_RC.m_Panel.DrawMenuText( xPosRankRight, c_yposAButton, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_STATS_A_TRACKDETAILS), XBFONT_RIGHT );
            yPos += ySize;

            // "X Button" action
            m_RC.m_Panel.DrawMenuText( xPosRankRight, c_yposXButton, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_STATS_X_RACETRACKRANKS), XBFONT_RIGHT );
            yPos += ySize;
        }
        else
        {
            // "A Button" action
            m_RC.m_Panel.DrawMenuText( xPosRankRight, c_yposAButton, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_STATS_A_TRACKDETAILS), XBFONT_RIGHT );
            yPos += ySize;

            // "X Button" action
            m_RC.m_Panel.DrawMenuText( xPosRankRight, c_yposXButton, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_STATS_X_BATTLETRACKRANKS), XBFONT_RIGHT );
            yPos += ySize;
        }
    }
    else
    {
        m_RC.m_Panel.DrawMenuTitleText( 320, 80, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_STATS_RETRIEVINGDATA), XBFONT_CENTER_X );
    }

}



////////////////////////////////////////////////////////////////
//
// Racer Summary
//
////////////////////////////////////////////////////////////////
CUIStatsUserSummary::CUIStatsUserSummary( )
{
    m_bBattleMode = FALSE;
    m_pRaceStats = NULL;
    m_pBattleStats = NULL;

}

HRESULT CUIStatsUserSummary::Init( XONLINE_USER *pUser, DWORD dwLeaderBoardID  )
{
    Assert( pUser );

    HRESULT hr = S_OK;

    m_User = *pUser;
    m_dwLeaderBoardID = dwLeaderBoardID;

    if ( IsBattleStat( m_dwLeaderBoardID ) )
    {
        m_bBattleMode = TRUE;

        if ( g_BattlerStats.find( m_User.xuid ) != g_BattlerStats.end() )
        {
            m_pBattleStats = g_BattlerStats[ m_User.xuid ];
        }
        else
        {

            m_pBattleStats = new CStatBattler();
            if ( m_pBattleStats )
            {
                m_pBattleStats->Reset( m_User.xuid );
                hr = m_pBattleStats->RequestStats();
                g_BattlerStats.insert( BattlerStats::value_type( m_User.xuid, m_pBattleStats ) );

            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
    }
    else
    {
        m_bBattleMode = FALSE;

        if ( g_RacerStats.find( m_User.xuid ) != g_RacerStats.end() )
        {
            m_pRaceStats = g_RacerStats[ m_User.xuid ];
        }
        else
        {

            m_pRaceStats = new CStatRacer();
            if ( m_pRaceStats )
            {
                m_pRaceStats->Reset( m_User.xuid );
                hr = m_pRaceStats->RequestStats();
                g_RacerStats.insert( RacerStats::value_type( m_User.xuid, m_pRaceStats ) );

            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
    }

    return hr;
}

//-----------------------------------------------------------------------------
// Name: Process()
// Desc: Main control function for the state engine
//-----------------------------------------------------------------------------
HRESULT CUIStatsUserSummary::Process()
{
    HRESULT hr;

    switch( m_State )
    {
        case STATE_BEGIN:
        {
            //m_RC.BeginEnter();

            // Set the track select menu and camera position
            g_pMenuHeader->SetNextMenu( NULL );
            //g_pTitleScreenCamera->SetNewPos( TITLESCREEN_CAMPOS_TRACK_SELECT );

            m_State = STATE_WAIT_FOR_RESULTS;
            break;
        }

        case STATE_WAIT_FOR_RESULTS:
            if ( m_bBattleMode )
            {
                if ( m_pBattleStats->m_hTask )
                {
                    hr = m_pBattleStats->TaskContinue();

                    if (XONLINETASK_S_RUNNING != hr)
                    {
                        if ( SUCCEEDED( hr ) )
                        {
                            m_State = STATE_MAINLOOP;
                        }
                        else if (FAILED( hr ) )
                        {
                            Assert( SUCCEEDED( hr ) );
                            m_State = STATE_STATS_ERROR;
                        }
                    }
                }
            }
            else
            {
                if ( m_pRaceStats->m_hTask )
                {
                    hr = m_pRaceStats->TaskContinue();

                    if (XONLINETASK_S_RUNNING != hr)
                    {
                        if ( SUCCEEDED( hr ) )
                        {
                            m_State = STATE_MAINLOOP;
                        }
                        else if (FAILED( hr ) )
                        {
                            Assert( SUCCEEDED( hr ) );
                            m_State = STATE_STATS_ERROR;
                        }
                    }
                }
            }
            break;

        case STATE_STATS_ERROR:
            // Nothing to do, as control is in the menus
            break;

        case STATE_MAINLOOP:
            // Nothing to do, as control is in the menus
            break;
    }

    // Handle the menus
    //g_pMenuHeader->HandleMenus();
    //HandleInput();
    //Draw();
    hr = CUIScreen::Process();

    return S_FALSE;
}

bool CUIStatsUserSummary::HandleInput( MenuInputTypeEnum input)
{
    bool bMenuInputProcessed = FALSE;

    // Process Input
    switch( input )
    {
        case MENU_INPUT_UP:
            break;

        case MENU_INPUT_DOWN:
            break;

        case MENU_INPUT_BACK:
            if ( m_pBattleStats )
            {
                m_pBattleStats->CancelPendingRequest(  );
                m_pBattleStats = NULL;
            }

            if ( m_pRaceStats ) 
            {
                m_pRaceStats->CancelPendingRequest(  );
                m_pRaceStats = NULL;
            }

            Return( STATEENGINE_TERMINATED );
            bMenuInputProcessed = TRUE;
            break;

        case MENU_INPUT_SELECT:
        {
            if (! IsTrack( m_dwLeaderBoardID ) )
            {
                g_UITrackRanks.Init(  &m_User, m_bBattleMode );
                Call( &g_UITrackRanks );
                bMenuInputProcessed = TRUE;
            }

            break;
        }

        case MENU_INPUT_X:
            // toggle between race and battle summaries
            if ( STAT_LB_RACING_OVERALL == m_dwLeaderBoardID )
            {
                Init( &m_User, STAT_LB_BATTLE_OVERALL );
                m_State = STATE_WAIT_FOR_RESULTS;
                bMenuInputProcessed = TRUE;
            }
            else if ( STAT_LB_BATTLE_OVERALL == m_dwLeaderBoardID )
            {
                Init( &m_User, STAT_LB_RACING_OVERALL );
                m_State = STATE_WAIT_FOR_RESULTS;
                bMenuInputProcessed = TRUE;
            }
            break;
    }

    if (bMenuInputProcessed)
    {
        g_SoundEngine.Play2DSound( gMenuInputSFXIndex[input], FALSE );
    }

    if( input != MENU_INPUT_NONE )
    {
        g_fTitleScreenTimer = 0.0f;
    }


    return bMenuInputProcessed;
}



/////////////////////////////////////////////////////////////////////
//
// Draw Racer Summary
//
/////////////////////////////////////////////////////////////////////


void CUIStatsUserSummary::Draw()
{
//    LEVELINFO* pLevelInfo = GetLevelInfo(gTitleScreenVars.iLevelNum);
    BeginTextState();

    m_RC.m_Title.DrawNewTitleBox( 0,  23, 640, 48, 0xffffffff, 0 );

    m_RC.m_Panel.DrawNewSpruBox( 40 * RenderSettings.GeomScaleX,
                    70 * RenderSettings.GeomScaleY,
                    (FLOAT)(640 - 80) * RenderSettings.GeomScaleX,
                    (FLOAT)(480 - 140) * RenderSettings.GeomScaleY );

    BeginTextState();

    WCHAR strUserID[ XONLINE_GAMERTAG_SIZE ];
#ifdef XONLINE_OFFLINE
    swprintf(strUserID, L"OFFLINE_%d", m_xuidUser.qwUserID+1);
#else
    swprintf(strUserID, L"%S", m_User.szGamertag);
#endif

    if ( STAT_LB_RACING_OVERALL == m_dwLeaderBoardID )
    {
        m_RC.m_Title.DrawMenuTitleText( 320, 30, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_STATS_RACINGSUMMARY), XBFONT_CENTER_X );
    }
    else if ( STAT_LB_BATTLE_OVERALL == m_dwLeaderBoardID )
    {
        m_RC.m_Title.DrawMenuTitleText( 320, 30, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_STATS_BATTLESUMMARY), XBFONT_CENTER_X );
    }
    else
    {
        WCHAR strTrack[ MAX_LEVEL_NAME];
        swprintf(strTrack, L"%s", DefaultLevelInfo[ TrackLevel( m_dwLeaderBoardID ) ].strName );
        m_RC.m_Title.DrawMenuTitleText( 320, 30, MENU_TEXT_RGB_NORMAL, strTrack, XBFONT_CENTER_X );
    }

    m_RC.m_Panel.DrawMenuTitleText( 320, 80, MENU_TEXT_RGB_NORMAL, strUserID, XBFONT_CENTER_X );


    float xPosNameLeft = 120;
    float xPosRankRight = 640 - 150;
    float ySize = MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP;
    float yPos = 110;

    WCHAR strData[64];

    if ( m_bBattleMode )
    {
        if (m_pBattleStats)
        {
            BattlerStatsRecord *pStat = m_pBattleStats->GetLeaderBoardStats( m_dwLeaderBoardID );

            if ( 0 == pStat->BattlesStarted.lValue )
            {
                m_RC.m_Panel.DrawMenuTitleText( xPosNameLeft, yPos, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_STATS_NOBATTLESFOUGHT), XBFONT_LEFT );
            }
            else
            {
                // Rank
                if ( pStat->LeaderBoardSize.lValue && ( pStat->Rank.type != XONLINE_STAT_NONE ) )
                {
                    //swprintf( strData, TEXT_TABLE(TEXT_STATS_PERCENT_N_OF_N),  pStat->ComputeRankPercentage(),
                    swprintf( strData, L"%.2f%% (%d of %d)",  pStat->ComputeRankPercentile(),
                                                            pStat->Rank.lValue,
                                                            pStat->LeaderBoardSize.lValue );
                }
                else
                {
                    swprintf( strData, L"---");
                }
                m_RC.m_Panel.DrawMenuText( xPosNameLeft,  yPos, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_STATS_RANK), XBFONT_LEFT );
                m_RC.m_Panel.DrawMenuText( xPosRankRight, yPos, MENU_TEXT_RGB_HILITE, strData, XBFONT_RIGHT );
                yPos += ySize;


                // Battles Won
                swprintf(strData, L"%d", pStat->BattlesWon.lValue);
                m_RC.m_Panel.DrawMenuText( xPosNameLeft,  yPos, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_STATS_BATTLESWON), XBFONT_LEFT );
                m_RC.m_Panel.DrawMenuText( xPosRankRight, yPos, MENU_TEXT_RGB_HILITE, strData, XBFONT_RIGHT );
                yPos += ySize;

                // Battles Completed
                swprintf(strData, L"%d", pStat->BattlesCompleted.lValue);
                m_RC.m_Panel.DrawMenuText( xPosNameLeft,  yPos, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_STATS_BATTLESCOMPLETED), XBFONT_LEFT );
                m_RC.m_Panel.DrawMenuText( xPosRankRight, yPos, MENU_TEXT_RGB_HILITE, strData, XBFONT_RIGHT );
                yPos += ySize;

                // Battles Abandoned
                swprintf(strData, L"%d", pStat->ComputeBattlesAbandoned() );
                m_RC.m_Panel.DrawMenuText( xPosNameLeft,  yPos, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_STATS_BATTLESABANDONED), XBFONT_LEFT );
                m_RC.m_Panel.DrawMenuText( xPosRankRight, yPos, MENU_TEXT_RGB_HILITE, strData, XBFONT_RIGHT );
                yPos += ySize;

                // Win/Loss Ratio
                long lWinLoss = pStat->ComputeWinLossPercentage();
                if (-1 == lWinLoss )
                {
                    swprintf(strData, TEXT_TABLE(TEXT_STATS_NOBATTLESFOUGHT), pStat->ComputeWinLossPercentage() );
                }
                else
                {
                    swprintf(strData, L"%d%%", lWinLoss );
                }
                m_RC.m_Panel.DrawMenuText( xPosNameLeft,  yPos, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_STATS_WINLOSSRATIO), XBFONT_LEFT );
                m_RC.m_Panel.DrawMenuText( xPosRankRight, yPos, MENU_TEXT_RGB_HILITE, strData, XBFONT_RIGHT );
                yPos += ySize;

                // Total It Time
                DWORD dwHours, dwMinutes, dwSeconds;
                pStat->ComputeTotalItTimeHMS( dwHours, dwMinutes, dwSeconds);
                swprintf(strData, L"%d:%#.2d:%#.2d", dwHours, dwMinutes, dwSeconds );
                m_RC.m_Panel.DrawMenuText( xPosNameLeft,  yPos, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_STATS_TOTALITTIME), XBFONT_LEFT );
                m_RC.m_Panel.DrawMenuText( xPosRankRight, yPos, MENU_TEXT_RGB_HILITE, strData, XBFONT_RIGHT );
                yPos += ySize;

                // Total Battle Time
                pStat->ComputeTotalBattleTimeHMS( dwHours, dwMinutes, dwSeconds);
                swprintf(strData, L"%d:%#.2d:%#.2d", dwHours, dwMinutes, dwSeconds );
                m_RC.m_Panel.DrawMenuText( xPosNameLeft,  yPos, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_STATS_TOTALBATTLETIME), XBFONT_LEFT );
                m_RC.m_Panel.DrawMenuText( xPosRankRight, yPos, MENU_TEXT_RGB_HILITE, strData, XBFONT_RIGHT );
                yPos += ySize;
            }
        }
    }
    else
    {
        if (m_pRaceStats)
        {
            RacerStatsRecord *pStat = m_pRaceStats->GetLeaderBoardStats( m_dwLeaderBoardID );

            if ( 0 == pStat->RacesStarted.lValue )
            {
                m_RC.m_Panel.DrawMenuTitleText( xPosNameLeft, yPos, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_STATS_NOTYETRACED), XBFONT_LEFT );
            }
            else
            {
                // Rank
                if ( pStat->LeaderBoardSize.lValue && ( pStat->Rank.type != XONLINE_STAT_NONE ) )
                {
                    //swprintf( strData, TEXT_TABLE(TEXT_STATS_PERCENT_N_OF_N),  pStat->ComputeRankPercentage(),
                    swprintf( strData, L"%.2f%% (%d of %d)",  pStat->ComputeRankPercentile(),
                                                            pStat->Rank.lValue,
                                                            pStat->LeaderBoardSize.lValue );
                }
                else
                {
                    swprintf( strData, L"---" );
                }
                m_RC.m_Panel.DrawMenuText( xPosNameLeft,  yPos, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_STATS_RANK), XBFONT_LEFT );
                m_RC.m_Panel.DrawMenuText( xPosRankRight, yPos, MENU_TEXT_RGB_HILITE, strData, XBFONT_RIGHT );
                yPos += ySize;


                // Races Won
                swprintf(strData, L"%d", pStat->RacesWon.lValue);
                m_RC.m_Panel.DrawMenuText( xPosNameLeft,  yPos, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_STATS_RACESWON), XBFONT_LEFT );
                m_RC.m_Panel.DrawMenuText( xPosRankRight, yPos, MENU_TEXT_RGB_HILITE, strData, XBFONT_RIGHT );
                yPos += ySize;

                // Races Completed
                swprintf(strData, L"%d", pStat->RacesCompleted.lValue);
                m_RC.m_Panel.DrawMenuText( xPosNameLeft,  yPos, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_STATS_RACESCOMPLETED), XBFONT_LEFT );
                m_RC.m_Panel.DrawMenuText( xPosRankRight, yPos, MENU_TEXT_RGB_HILITE, strData, XBFONT_RIGHT );
                yPos += ySize;

                // Races Abandoned
                swprintf(strData, L"%d", pStat->ComputeRacesAbandoned() );
                m_RC.m_Panel.DrawMenuText( xPosNameLeft,  yPos, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_STATS_RACESABANDONED), XBFONT_LEFT );
                m_RC.m_Panel.DrawMenuText( xPosRankRight, yPos, MENU_TEXT_RGB_HILITE, strData, XBFONT_RIGHT );
                yPos += ySize;

                // Win/Loss Ratio
                long lWinLoss = pStat->ComputeWinLossPercentage();
                if (-1 == lWinLoss )
                {
                    swprintf(strData, TEXT_TABLE(TEXT_STATS_NOTYETRACED), pStat->ComputeWinLossPercentage() );
                }
                else
                {
                    swprintf(strData, L"%d%%", lWinLoss );
                }
                m_RC.m_Panel.DrawMenuText( xPosNameLeft,  yPos, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_STATS_WINLOSSRATIO), XBFONT_LEFT );
                m_RC.m_Panel.DrawMenuText( xPosRankRight, yPos, MENU_TEXT_RGB_HILITE, strData, XBFONT_RIGHT );
                yPos += ySize;

                // Total Racing Time
                DWORD dwHours, dwMinutes, dwSeconds;
                pStat->ComputeTotalRaceTimeHMS( dwHours, dwMinutes, dwSeconds);
                swprintf(strData, L"%d:%#.2d:%#.2d", dwHours, dwMinutes, dwSeconds );
                m_RC.m_Panel.DrawMenuText( xPosNameLeft,  yPos, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_STATS_TOTALRACINGTIME), XBFONT_LEFT );
                m_RC.m_Panel.DrawMenuText( xPosRankRight, yPos, MENU_TEXT_RGB_HILITE, strData, XBFONT_RIGHT );
                yPos += ySize;

                // Average Speed
			    swprintf(strData, TEXT_TABLE(TEXT_STATS_AVERAGE_KPH), pStat->ComputeAverageKPH() );
                m_RC.m_Panel.DrawMenuText( xPosNameLeft,  yPos, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_STATS_AVERAGESPEED), XBFONT_LEFT );
                m_RC.m_Panel.DrawMenuText( xPosRankRight, yPos, MENU_TEXT_RGB_HILITE, strData, XBFONT_RIGHT );
                yPos += ySize;

                if ( STAT_LB_RACING_OVERALL != m_dwLeaderBoardID )
                {
    #ifdef USE_SPLIT_TIMES
                    // Best Split Time
                    pStat->ComputeBestSplitTimeHMS( dwHours, dwMinutes, dwSeconds);
                    swprintf(strData, L"%d:%#.2d:%#.2d", dwHours, dwMinutes, dwSeconds );
                    m_RC.m_Panel.DrawMenuText( xPosNameLeft,  yPos, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_STATS_BESTSPLITTIME), XBFONT_LEFT );
                    m_RC.m_Panel.DrawMenuText( xPosRankRight, yPos, MENU_TEXT_RGB_HILITE, strData, XBFONT_RIGHT );
                    yPos += ySize;

                    // Best Split Car
                    swprintf(strData, L"%S", pStat->LookupBestSplitCar() );
                    m_RC.m_Panel.DrawMenuText( xPosNameLeft,  yPos, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_STATS_BESTSPLITCAR), XBFONT_LEFT );
                    m_RC.m_Panel.DrawMenuText( xPosRankRight, yPos, MENU_TEXT_RGB_HILITE, strData, XBFONT_RIGHT );
                    yPos += ySize;
    #else
                    // $HACK - best split is now best lap, cleanup code later.
                    // Best Lap Time
                    pStat->ComputeBestSplitTimeHMS( dwHours, dwMinutes, dwSeconds);
                    swprintf(strData, L"%d:%#.2d:%#.2d", dwHours, dwMinutes, dwSeconds );
                    m_RC.m_Panel.DrawMenuText( xPosNameLeft,  yPos, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_STATS_BESTLAPTIME), XBFONT_LEFT );
                    m_RC.m_Panel.DrawMenuText( xPosRankRight, yPos, MENU_TEXT_RGB_HILITE, strData, XBFONT_RIGHT );
                    yPos += ySize;

                    // Best Lap Car
                    swprintf(strData, L"%S", pStat->LookupBestSplitCar() );
                    m_RC.m_Panel.DrawMenuText( xPosNameLeft,  yPos, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_STATS_BESTLAPCAR), XBFONT_LEFT );
                    m_RC.m_Panel.DrawMenuText( xPosRankRight, yPos, MENU_TEXT_RGB_HILITE, strData, XBFONT_RIGHT );
                    yPos += ySize;
    #endif
                }
            }
        }
        
    }

    if ( !IsTrack( m_dwLeaderBoardID ) )
    {
        yPos += ySize;
        yPos += ySize;
        if ( STAT_LB_RACING_OVERALL == m_dwLeaderBoardID )
        {
            // "A Button" action
            m_RC.m_Panel.DrawMenuText( xPosRankRight, yPos, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_STATS_A_TRACKSTATS), XBFONT_RIGHT );
            yPos += ySize;

            // "X Button" action
            m_RC.m_Panel.DrawMenuText( xPosRankRight, yPos, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_STATS_X_BATTLEMODESTATS), XBFONT_RIGHT );
            yPos += ySize;
        }
        else
        {
            // "A Button" action
            m_RC.m_Panel.DrawMenuText( xPosRankRight, yPos, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_STATS_A_TRACKSTATS), XBFONT_RIGHT );
            yPos += ySize;

            // "X Button" action
            m_RC.m_Panel.DrawMenuText( xPosRankRight, yPos, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_STATS_X_RACEMODESTATS), XBFONT_RIGHT );
            yPos += ySize;
        }
    }
}

