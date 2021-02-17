//-----------------------------------------------------------------------------
// File: ui_PlayLive.cpp
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"
#include "dx.h"
#include "input.h"
#include "geom.h"
#include "camera.h"
#include "LevelLoad.h"
#include "player.h"
#include "settings.h"
#include "spark.h"
#include "timing.h"
#include "ui_TitleScreen.h"
#include "InitPlay.h"
#include "competition.h"
#include "main.h"
#include "player.h"
#include "credits.h"
#include "SoundEffectEngine.h"
#include "MusicManager.h"
#include "gameloop.h"
#include "readinit.h"
#include "panel.h"
#include "cheats.h"
#include "text.h"
#include "net_xonline.h"
#include "ui_MenuText.h"
#include "ui_MenuDraw.h"
#include "ui_StateEngine.h"
#include "ui_PlayLive.h"
#include "ui_LiveSignOn.h"
#include "ui_EnterName.h"
#include "ui_RaceOverview.h"
#include "ui_SelectRaceMode.h"
#include "ui_SelectTrack.h"
#include "ui_SelectCar.h"
#include "ui_RaceOverview.h"
#include "ui_ShowMessage.h"
#include "ui_Friends.h"
#include "ui_WaitingRoom.h"
#include "ui_Animation.h"
#ifdef ENABLE_STATISTICS
//$REVISIT: Statistics disabled for July Consumer Beta
#include "net_Statistics.h"
#include "ui_Statistics.h"
#endif // ENABLE_STATISTICS
#include "ui_contentdownload.h"

//$DEBUG: use this #define to skip XOnline apis when testnet is down
//#define XONLINE_OFFLINE


extern long SessionRefreshFlag;
extern long HostnameEntry;



CPlayLiveStateEngine g_PlayLiveStateEngine;



class CCreateMatchStateEngine : public CUIStateEngine
{
public:
    virtual HRESULT Process();
    virtual WCHAR*  DebugGetName()   { return L"CreateMatch"; }
};

CCreateMatchStateEngine g_CreateMatchStateEngine;



class COptiMatchStateEngine : public CUIStateEngine
{
public:
    virtual HRESULT Process();
    virtual WCHAR*  DebugGetName()   { return L"OptiMatch"; }
};

COptiMatchStateEngine g_OptiMatchStateEngine;


// extern'd from header file
CQuickMatchStateEngine g_QuickMatchStateEngine;

// extern'd from header file
CJoinMatchStateEngine g_JoinMatchStateEngine;


class CSelectMatchStateEngine : public CUIStateEngine
{
public:
    virtual HRESULT Process();
    virtual WCHAR*  DebugGetName()   { return L"SelectMatch"; }
};

static CSelectMatchStateEngine g_SelectMatchStateEngine;





#define MENU_PLAYLIVE_XPOS              100
#define MENU_PLAYLIVE_YPOS              150

#define MENU_SELECTRACEMODE_XPOS        100
#define MENU_SELECTRACEMODE_YPOS        150


static VOID CreatePlayLiveMenu( MENU_HEADER* pMenuHeader, MENU* pMenu );
static BOOL HandlePlayLiveMenu( MENU_HEADER* pMenuHeader, DWORD dwInput );

static VOID CreateSelectRaceModeMenu( MENU_HEADER *pMenuHeader, MENU *pMenu );
static BOOL HandleSelectRaceModeMenu( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem );




//-----------------------------------------------------------------------------
// PlayLive menu
//-----------------------------------------------------------------------------
extern MENU Menu_PlayLive = 
{
    TEXT_PLAYLIVE,
    MENU_DEFAULT | MENU_CENTRE_X,           // Menu type
    CreatePlayLiveMenu,                     // Create menu function
    HandlePlayLiveMenu,                     // Input handler function
    NULL,                                   // Menu draw function
    MENU_PLAYLIVE_XPOS,                     // X coord
    MENU_PLAYLIVE_YPOS,
};

void DrawTextWithReceivedInviteIcon( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem, int itemIndex );
MENU_ITEM MenuItem_FriendsWithInviteIcon =
{
    TEXT_FRIENDS,
    40.0f,
    NULL,
    DrawTextWithReceivedInviteIcon,
};


VOID CreatePlayLiveMenu(MENU_HEADER *pMenuHeader, MENU *pMenu)
{
    // Add pMenu items
    pMenuHeader->AddMenuItem( TEXT_QUICKMATCH );
    pMenuHeader->AddMenuItem( TEXT_OPTIMATCH );
    pMenuHeader->AddMenuItem( TEXT_CREATEMATCH );
    //pMenuHeader->AddMenuItem( TEXT_FRIENDS );
    pMenuHeader->AddMenuItem( &MenuItem_FriendsWithInviteIcon );
#ifdef ENABLE_STATISTICS
    pMenuHeader->AddMenuItem( TEXT_STATISTICS );
#else
    pMenuHeader->AddMenuItem( TEXT_STATISTICS, MENU_ITEM_INACTIVE );
#endif

    // $MD: Added the "online store" menu itme
    // $LOCALIZE
    pMenuHeader->AddMenuItem( TEXT_ONLINESTORE );
}


BOOL HandlePlayLiveMenu( MENU_HEADER* pMenuHeader, DWORD dwInput )
{
    switch( dwInput )
    {
        case MENU_INPUT_BACK:
//$MODIFIED
            g_PlayLiveStateEngine.Return( STATEENGINE_TERMINATED );
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
                case TEXT_QUICKMATCH:
                    g_PlayLiveStateEngine.Call( &g_QuickMatchStateEngine );
                    return TRUE;

                case TEXT_OPTIMATCH:
                    g_PlayLiveStateEngine.Call( &g_OptiMatchStateEngine );
                    return TRUE;

                case TEXT_CREATEMATCH:
                    g_PlayLiveStateEngine.Call( &g_CreateMatchStateEngine );
                    return TRUE;
    
                case TEXT_FRIENDS:
                    g_PlayLiveStateEngine.Call( &g_FriendsStateEngine );
                    return TRUE;

#ifdef ENABLE_STATISTICS
//$REVISIT: Statistics disabled for July Consumer Beta
                case TEXT_STATISTICS:
                    g_PlayLiveStateEngine.Call( &g_UIStatisticsMenu );
                    return TRUE;
                // $MD: $BUGBUG: The online store menu itme doesn't have a TextIndex yet.
#endif // ENABLE_STATISTICS
                case TEXT_ONLINESTORE:
                    g_PlayLiveStateEngine.Call( &g_OptionalDownloadEngine );
                    return TRUE;
            }
            break;
    }

    return FALSE;
}




//-----------------------------------------------------------------------------
// SelectRaceMode menu
//-----------------------------------------------------------------------------
extern MENU Menu_SelectRaceMode = 
{
    TEXT_SELECTRACE,
    MENU_DEFAULT | MENU_CENTRE_X,           // Menu type
    CreateSelectRaceModeMenu,               // Create menu function
    NULL,                                   // Input handler function
    NULL,                                   // Menu draw function
    MENU_SELECTRACEMODE_XPOS,               // X coord
    MENU_SELECTRACEMODE_YPOS,               // Y coord
};

// Select Race Mode - Race
MENU_ITEM MenuItem_Race = 
{
    TEXT_RACE,                              // Text label index

    0,                                      // Space needed to draw item data
    NULL,                                   // Data
    NULL,                                   // Draw Function

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    NULL,                                   // Left Action
    NULL,                                   // Right Action
    MenuGoBack,                             // Back Action
    HandleSelectRaceModeMenu,               // Forward Action
};

// Select Race Mode - BattleTag
MENU_ITEM MenuItem_BattleTag = 
{
    TEXT_BATTLETAG,                         // Text label index

    0,                                      // Space needed to draw item data
    NULL,                                   // Data
    NULL,                                   // Draw Function

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    NULL,                                   // Left Action
    NULL,                                   // Right Action
    MenuGoBack,                             // Back Action
    HandleSelectRaceModeMenu,               // Forward Action
};


void CreateSelectRaceModeMenu(MENU_HEADER *pMenuHeader, MENU *pMenu)
{

    // Add pMenu items
    pMenuHeader->AddMenuItem( &MenuItem_Race );
    pMenuHeader->AddMenuItem( &MenuItem_BattleTag );
}


//VOID SelectRaceModeGoBack( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem )
//{
//  g_pMenuHeader->SetNextMenu( NULL );
//}


BOOL HandleSelectRaceModeMenu( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem )
{
    switch( pMenuItem->TextIndex )
    {
        case TEXT_RACE:
            GameSettings.GameType = GAMETYPE_NETWORK_RACE;
            break;

        case TEXT_BATTLETAG:
            GameSettings.GameType = GAMETYPE_NETWORK_BATTLETAG;
            break;
    }

    MenuGoForward( pMenuHeader, pMenu, pMenuItem );  
    return TRUE;
}




//-----------------------------------------------------------------------------
// The Play Live state engine
//-----------------------------------------------------------------------------
HRESULT CPlayLiveStateEngine::Process()
{
    enum
    {
        PLAYLIVE_STATE_BEGIN = STATEENGINE_STATE_BEGIN,
        PLAYLIVE_STATE_LIVESIGNIN,
        PLAYLIVE_STATE_PLAYERSSIGNEDIN,
        PLAYLIVE_STATE_MAINLOOP,
        PLAYLIVE_STATE_BACKOUT = STATEENGINE_STATE_EXIT,
    };

    switch( m_State )
    {
        case PLAYLIVE_STATE_BEGIN:
            GameSettings.GameType    = GAMETYPE_NETWORK_RACE;
//$MOVED: this is now being done in each state engine where gets set to value other than _NONE
//            GameSettings.MultiType   = MULTITYPE_NONE;
//$END_MOVE
            GameSettings.RandomCars  = gTitleScreenVars.RandomCars;
            GameSettings.RandomTrack = gTitleScreenVars.RandomTrack;
            gTitleScreenVars.iCurrentPlayer  = 0;
            gTitleScreenVars.numberOfPlayers = 1;

            // Init host name entry
            HostnameEntry = FALSE;

            // Prepare for session enum
            assert( GetSessionCount() == 0 ); //$NOTE: WAS ORIGINALLY "SessionCount = 0;" , calls to ClearSessionList() ensure SessionCount will be zero
            SessionPick        = 0;
            SessionRefreshFlag = 3;

            g_pTitleScreenCamera->SetNewPos( TITLESCREEN_CAMPOS_RACE );
            g_pMenuHeader->SetNextMenu( &Menu_PlayLive );

            // Check if players need to be signed on
            if( FALSE == g_LiveSignInStateEngine.PlayersSignedIn() )
            {
                // Call the Live Sign In state engine
                Call( &g_LiveSignInStateEngine );
                m_State = PLAYLIVE_STATE_LIVESIGNIN;
            }
            else
            {
                // Skip ahead
                m_State = PLAYLIVE_STATE_PLAYERSSIGNEDIN;
            }
            break;

        case PLAYLIVE_STATE_BACKOUT:
            //$REVISIT: Should we uninit more/fewer vars here?  (Most probably not necessary.)
            GameSettings.GameType = GAMETYPE_NONE;

            Return( STATEENGINE_TERMINATED );
            break;

        case PLAYLIVE_STATE_LIVESIGNIN:
            // If players aren't successfully signed in, user must have backed
            // out, so cleanup and return to parent menu
            if( FALSE == g_LiveSignInStateEngine.PlayersSignedIn() )
                return GotoState( PLAYLIVE_STATE_BACKOUT );
            // Else, fall through to next state
        
        case PLAYLIVE_STATE_PLAYERSSIGNEDIN:
            // Players were signed on, so it's safe to proceed to the Play Live menu
            m_State = PLAYLIVE_STATE_MAINLOOP;
            break;

        case PLAYLIVE_STATE_MAINLOOP:
            // Nothing to do, as control is in the menus
            break;
    }

    // Handle the menus
    g_pMenuHeader->HandleMenus();

    return S_FALSE;
}




//-----------------------------------------------------------------------------
// The SelectCreateMatchParams state engine
//-----------------------------------------------------------------------------
class CSelectCreateMatchParamsStateEngine : public CUIStateEngine
{
public:
    virtual HRESULT Process();
    virtual WCHAR*  DebugGetName()   { return L"SelectCreateMatchParams"; }
};

CSelectCreateMatchParamsStateEngine g_SelectCreateMatchParamsStateEngine;


void CreateCreateMatchParamsMenu( MENU_HEADER* pMenuHeader, MENU* pMenu );
BOOL HandleCreateMatchParamsMenu( MENU_HEADER* pMenuHeader, DWORD dwInput );
void DrawGameMode(MENU_HEADER *pMenuHeader, MENU *pMenu, MENU_ITEM *menuItem, int itemIndex);
void DrawInviteMode(MENU_HEADER *pMenuHeader, MENU *pMenu, MENU_ITEM *menuItem, int itemIndex);

static MENU Menu_CreateMatchParams = 
{
    TEXT_CREATEMATCH,
    MENU_DEFAULT|MENU_CENTRE_X|MENU_CENTRE_Y|MENU_PAD_FOR_ARROWS,                           // Menu type
    CreateCreateMatchParamsMenu,                    // Create menu function
    HandleCreateMatchParamsMenu,                    // Input handler function
    NULL,                                   // Menu draw function
    0,                    // X coord
    0,                    // Y Coord
};


//$REVISIT: should consider removing these vars; they seem to overlap with gTitleScreenVars.numberOfCars and similar vars, which is ugly...
static LONG g_lGameMode   = 0;
static LONG g_lNumLaps    = DEFAULT_RACE_LAPS;
static LONG g_lMaxPlayers = DEFAULT_RACE_CARS; //$REVISIT: need to check that value of DEFAULT_RACE_CARS != MAX_RACE_CARS is safe!
static LONG g_lInviteMode = 0;


static SLIDER_DATA_LONG GameModeSlider = 
{
    &g_lGameMode,
    0, 1, 1,
    FALSE, FALSE,
};

static SLIDER_DATA_LONG NumLapsSlider = 
{
    &g_lNumLaps,
    MIN_RACE_LAPS, MAX_RACE_LAPS, 1,
    FALSE, TRUE,
};

static SLIDER_DATA_LONG MaxPlayersSlider = 
{
    &g_lMaxPlayers,
    2, MAX_NUM_PLAYERS, 1,
    FALSE, TRUE,
};

static SLIDER_DATA_LONG InviteModeSlider = 
{
    &g_lInviteMode,
    0, 1, 1,
    FALSE, FALSE,
};


static MENU_ITEM MenuItem_GameMode = 
{
    TEXT_CREATEMATCH_GAMEMODE,              // Text label index
    12.0f * MENU_TEXT_WIDTH,                // Space needed to draw item data
    &GameModeSlider,                        // Data
    DrawGameMode,                           // Draw Function
};

static MENU_ITEM MenuItem_NumLaps = 
{
    TEXT_CREATEMATCH_NUMLAPS,               // Text label index
    MENU_DATA_WIDTH_INT,                    // Space needed to draw item data
    &NumLapsSlider,                         // Data
    DrawSliderDataLong,                     // Draw Function
};

static MENU_ITEM MenuItem_MaxPlayers = 
{
    TEXT_CREATEMATCH_TOTALPLAYERS,          // Text label index
    MENU_DATA_WIDTH_INT,                    // Space needed to draw item data
    &MaxPlayersSlider,                      // Data
    DrawSliderDataLong,                     // Draw Function
};

static MENU_ITEM MenuItem_InviteMode = 
{
    TEXT_CREATEMATCH_INVITEMODE,            // Text label index
    12.0f * MENU_TEXT_WIDTH,                // Space needed to draw item data
    &InviteModeSlider,                      // Data
    DrawInviteMode,                         // Draw Function
};


// Create
void CreateCreateMatchParamsMenu( MENU_HEADER* pMenuHeader, MENU* pMenu )
{
    // Compute max widths for menuitems
    MenuItem_GameMode.DataWidth   = max( g_pFont->GetTextWidth( TEXT_TABLE(TEXT_RACE) ),
                                         g_pFont->GetTextWidth( TEXT_TABLE(TEXT_BATTLETAG) ) );
    MenuItem_InviteMode.DataWidth = max( g_pFont->GetTextWidth( TEXT_TABLE(TEXT_MATCHTYPE_PUBLIC) ),
                                         g_pFont->GetTextWidth( TEXT_TABLE(TEXT_MATCHTYPE_PRIVATE) ) );

    // Add menu items
    pMenuHeader->AddMenuItem( &MenuItem_GameMode );
    pMenuHeader->AddMenuItem( &MenuItem_NumLaps );
    pMenuHeader->AddMenuItem( TEXT_NONE, MENU_ITEM_INACTIVE );
    pMenuHeader->AddMenuItem( &MenuItem_MaxPlayers );
    pMenuHeader->AddMenuItem( &MenuItem_InviteMode );
}

BOOL HandleCreateMatchParamsMenu( MENU_HEADER* pMenuHeader, DWORD dwInput )
{
    switch( dwInput )
    {
        case MENU_INPUT_LEFT:
            switch( pMenuHeader->m_pCurrentItem->TextIndex )
            {
                case TEXT_CREATEMATCH_GAMEMODE:
//$BUGBUG: This #ifdef is ONLY for May02_TechBeta!!
/// For the final release, we want to allow users to select different game modes.
/// So remove this #ifdef when all game modes are supported.
#ifdef SHIPPING
                break;
#endif
                case TEXT_CREATEMATCH_NUMLAPS:
                case TEXT_CREATEMATCH_TOTALPLAYERS:
                case TEXT_CREATEMATCH_INVITEMODE:
                    return DecreaseSliderDataLong( pMenuHeader, pMenuHeader->m_pMenu, pMenuHeader->m_pCurrentItem ); 
            }
            break;

        case MENU_INPUT_RIGHT:
            switch( pMenuHeader->m_pCurrentItem->TextIndex )
            {
                case TEXT_CREATEMATCH_GAMEMODE:
//$BUGBUG: This #ifdef is ONLY for May02_TechBeta!!
/// For the final release, we want to allow users to select different game modes.
/// So remove this #ifdef when all game modes are supported.
#ifdef SHIPPING
                break;
#endif
                case TEXT_CREATEMATCH_NUMLAPS:
                case TEXT_CREATEMATCH_TOTALPLAYERS:
                case TEXT_CREATEMATCH_INVITEMODE:
                    return IncreaseSliderDataLong( pMenuHeader, pMenuHeader->m_pMenu, pMenuHeader->m_pCurrentItem );
            }
            break;

        case MENU_INPUT_UP:
            return SelectPreviousMenuItem( pMenuHeader, pMenuHeader->m_pMenu, NULL );
        
        case MENU_INPUT_DOWN:
            return SelectNextMenuItem( pMenuHeader, pMenuHeader->m_pMenu, NULL );
        
        case MENU_INPUT_BACK:
            GameSettings.GameType = GAMETYPE_NONE;
            g_SelectCreateMatchParamsStateEngine.Return( STATEENGINE_TERMINATED );
            return TRUE;

        case MENU_INPUT_SELECT:
            if( TEXT_RACE + g_lGameMode == TEXT_RACE )
                GameSettings.GameType = GAMETYPE_NETWORK_RACE;

            if( TEXT_RACE + g_lGameMode == TEXT_BATTLETAG )
                GameSettings.GameType = GAMETYPE_NETWORK_BATTLETAG;

            gTitleScreenVars.numberOfLaps         = g_lNumLaps;
            GameSettings.NumberOfLaps = gTitleScreenVars.numberOfLaps;  //$REVISIT: do we need this line?
            gTitleScreenVars.numberOfCars         = g_lMaxPlayers;
            
            if( g_lInviteMode == 0 )
            {
                gTitleScreenVars.numberOfPrivateSlots = 0;
                gTitleScreenVars.numberOfPublicSlots  = g_lMaxPlayers;
            }
            else
            {
                gTitleScreenVars.numberOfPrivateSlots = g_lMaxPlayers;
                gTitleScreenVars.numberOfPublicSlots  = 0;
            }

#ifndef XBOX_DISABLE_NETWORK //$REVISIT: Probably can remove; I don't think we need to call this (level name, etc gets propagated to clients manually, not via DPlay)
            LEVELINFO *levinf = GetLevelInfo(gTitleScreenVars.iLevelNum);
            SetSessionDesc(gTitleScreenVars.nameEnter[0], levinf->Dir, FALSE, GameSettings.GameType, GameSettings.RandomCars, GameSettings.RandomTrack);
#endif // !XBOX_DISABLE_NETWORK
            g_SelectCreateMatchParamsStateEngine.Return( STATEENGINE_COMPLETED );
            return TRUE;
    }

    return FALSE;
}


void DrawGameMode( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem, int itemIndex )
{
    FLOAT xPos = pMenuHeader->m_XPos + pMenuHeader->m_ItemTextWidth + MENU_TEXT_GAP;
    FLOAT yPos = pMenuHeader->m_YPos + (MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP) * itemIndex;

    DrawMenuTextWithArrows( (pMenuItem == pMenuHeader->m_pCurrentItem) ? TRUE : FALSE, 
                            xPos, yPos, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_RACE + g_lGameMode), 
                            pMenuHeader->m_ItemDataWidth );
}


void DrawInviteMode( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem, int itemIndex )
{
    FLOAT xPos = pMenuHeader->m_XPos + pMenuHeader->m_ItemTextWidth + MENU_TEXT_GAP;
    FLOAT yPos = pMenuHeader->m_YPos + (MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP) * itemIndex;

    DrawMenuTextWithArrows( (pMenuItem == pMenuHeader->m_pCurrentItem) ? TRUE : FALSE, 
                            xPos, yPos, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_MATCHTYPE_PUBLIC + g_lInviteMode), 
                            pMenuHeader->m_ItemDataWidth );
}




//-----------------------------------------------------------------------------
// Name: Process()
// Desc: Main control function for the state engine
//-----------------------------------------------------------------------------
HRESULT CSelectCreateMatchParamsStateEngine::Process()
{
    enum
    {
        SELECTCREATEMATCHPARAMS_STATE_BEGIN = STATEENGINE_STATE_BEGIN,
        SELECTCREATEMATCHPARAMS_STATE_MAINLOOP,
    };

    switch( m_State )
    {
        case SELECTCREATEMATCHPARAMS_STATE_BEGIN:
            // Initialize state
            GameSettings.GameType = GAMETYPE_NONE;

            // Set the menu and camera settings
            g_pMenuHeader->SetNextMenu( &Menu_CreateMatchParams );
            g_pTitleScreenCamera->SetNewPos( TITLESCREEN_CAMPOS_MULTI );

            m_State = SELECTCREATEMATCHPARAMS_STATE_MAINLOOP;
            break;

        case SELECTCREATEMATCHPARAMS_STATE_MAINLOOP:
            // Nothing to do, as control is in the menus
            break;
    }

    // Handle the menus
    g_pMenuHeader->HandleMenus();

    return S_FALSE;
}




//-----------------------------------------------------------------------------
// Name: CCreateMatchStateEngine::Process()
// Desc: Sequence the user interface through steps necessary for creating a
//       match:  Select Race, track, car, show summary, create the match on the
//       server, and start the race.
//-----------------------------------------------------------------------------
HRESULT CCreateMatchStateEngine::Process()
{
    enum
    {
        CREATEMATCH_STATE_BEGIN = STATEENGINE_STATE_BEGIN,
        CREATEMATCH_STATE_SELECTMATCHPARAMS,
        CREATEMATCH_STATE_POSTSELECTMATCHPARAMS,
        CREATEMATCH_STATE_SELECTTRACK,
        CREATEMATCH_STATE_POSTSELECTTRACK,
        CREATEMATCH_STATE_ENTERNAME,
        CREATEMATCH_STATE_POSTENTERNAME,
        CREATEMATCH_STATE_SELECTCAR,
        CREATEMATCH_STATE_POSTSELECTCAR,
        CREATEMATCH_STATE_WAITINGROOM,
        CREATEMATCH_STATE_POSTWAITINGROOM,
        CREATEMATCH_STATE_STARTGAME,
        CREATEMATCH_STATE_BACKOUT = STATEENGINE_STATE_EXIT,
    };

    switch( m_State )
    {
        case CREATEMATCH_STATE_BEGIN:
        case CREATEMATCH_STATE_SELECTMATCHPARAMS:
            // Set host
            GameSettings.MultiType = MULTITYPE_SERVER;
            LocalPlayerReady = FALSE;
            HostQuit         = FALSE;

            //$REVISIT: should these 2 lines go away eventually?
            /// (Should only have 1 copy of this name variable.  And it should have been filled with value from user's profile.)
            strncpy( gTitleScreenVars.PlayerData[0].nameEnter, RegistrySettings.PlayerName, MAX_PLAYER_NAME );
            gTitleScreenVars.PlayerData[0].nameEnter[MAX_PLAYER_NAME-1] = '\0';

            // Call the SelectRaceMode state engine
            Call( &g_SelectCreateMatchParamsStateEngine );
            m_State = CREATEMATCH_STATE_POSTSELECTMATCHPARAMS;
            break;

        case CREATEMATCH_STATE_BACKOUT:
            // Un-set host
            GameSettings.MultiType = MULTITYPE_NONE;

            //$REVISIT: Should we uninit more/fewer vars here?  (Most probably not necessary.)

            Return( STATEENGINE_TERMINATED );
            break;

        case CREATEMATCH_STATE_POSTSELECTMATCHPARAMS:
            if( g_SelectCreateMatchParamsStateEngine.GetStatus() == STATEENGINE_TERMINATED )
                return GotoState( CREATEMATCH_STATE_BACKOUT );
            // Else, fall through to next state

        case CREATEMATCH_STATE_SELECTTRACK:
            if( GameSettings.RandomTrack )
            {
                // Skip ahead past track selection
                m_State = CREATEMATCH_STATE_ENTERNAME;
                break;
            }

            // Call the SelectTrack state engine
            Call( &g_SelectTrackStateEngine );
            m_State = CREATEMATCH_STATE_POSTSELECTTRACK;
            break;

        case CREATEMATCH_STATE_POSTSELECTTRACK:
            // If user backed out, go back a few states
            if( g_SelectTrackStateEngine.GetStatus() == STATEENGINE_TERMINATED )
                return GotoState( CREATEMATCH_STATE_SELECTMATCHPARAMS );
            // Else, fall through to next state

        case CREATEMATCH_STATE_ENTERNAME:
            // Call the EnterName state engine
            Call( &g_EnterNameStateEngine );
            m_State = CREATEMATCH_STATE_POSTENTERNAME;
            break;

        case CREATEMATCH_STATE_POSTENTERNAME:
            // If user backed out, go back a few states
            if( g_EnterNameStateEngine.GetStatus() == STATEENGINE_TERMINATED )
            {
                if( GameSettings.RandomTrack )
                    return GotoState( CREATEMATCH_STATE_SELECTMATCHPARAMS );
                else
                    return GotoState( CREATEMATCH_STATE_SELECTTRACK );
            }
            // Else, fall through to next state

        case CREATEMATCH_STATE_SELECTCAR:
            if( GameSettings.RandomCars )
            {
                // Skip ahead past car selection
                m_State = CREATEMATCH_STATE_WAITINGROOM;
                break;
            }

            // Call the SelectCar state engine
            Call( &g_SelectCarStateEngine );
            m_State = CREATEMATCH_STATE_POSTSELECTCAR;
            break;

        case CREATEMATCH_STATE_POSTSELECTCAR:
            // If user backed out, go back a few states
            if( g_SelectCarStateEngine.GetStatus() == STATEENGINE_TERMINATED )
                return GotoState( CREATEMATCH_STATE_ENTERNAME );
            // Else, fall through to next state

        case CREATEMATCH_STATE_WAITINGROOM:
            // Init the network stuff

//$REMOVED (tentative!!) - Xbox always connected
//          // init connection
//          if (!InitConnection((char)gTitleScreenVars.connectionType))
//          {
//             // Back out somehow, display an error message, etc.
//          }
//$END_REMOVAL

            // Create session + create player
            if( CreateSession() )
            {
//$REMOVED_DOESNOTHING                CreatePlayer(gTitleScreenVars.PlayerData[0].nameEnter, DP_SERVER_PLAYER);

#ifndef XBOX_DISABLE_NETWORK //$REVISIT: Probably can remove; I don't think we need to call this (level name, etc gets propagated to clients manually, not via DPlay)
                LEVELINFO *levinf = GetLevelInfo(gTitleScreenVars.iLevelNum);
                SetSessionDesc(gTitleScreenVars.nameEnter[0], levinf->Dir, FALSE, GAMETYPE_NETWORK_RACE, GameSettings.RandomCars, GameSettings.RandomTrack);
#endif // !XBOX_DISABLE_NETWORK

//$REMOVED
//              // Get local IP
//              GetIPString( LocalIP );
//$END_REMOVAL
            }
            else
            {
//$REMOVED
//                KillNetwork();
//                InitNetwork();
//$END_REMOVAL
                //$TODO: Need to back out somehow, display an error message, etc.
            }
            //$BUG: we're assuming that XOnline match creation will never fail.
            // But should we actually do check here to make sure that's true (eg, in
            // case of server failure, etc)?  We could either spin waiting for
            // success (say "Creating match..."), or could show waiting room
            // immediately and back out on failure.

            // Call the WaitingRoom state engine
            g_pActiveStateEngine->Call( &g_WaitingRoomStateEngine );
            m_State = CREATEMATCH_STATE_POSTWAITINGROOM;
            break;

        case CREATEMATCH_STATE_POSTWAITINGROOM:
            // If user backed out, go back a few states
            if( g_WaitingRoomStateEngine.GetStatus() == STATEENGINE_TERMINATED )
            {
                // Destroy the session
                DestroySession();

                if( GameSettings.RandomCars )
                    return GotoState( CREATEMATCH_STATE_ENTERNAME );
                else
                    return GotoState( CREATEMATCH_STATE_SELECTCAR );
            }

            // If we get here, a game should be starting 
            break;
    }

    // Handle the menus
    g_pMenuHeader->HandleMenus();

    return S_FALSE;
}




//-----------------------------------------------------------------------------
// The SelectOptiMatchParams state engine
//-----------------------------------------------------------------------------
class CSelectOptiMatchParamsStateEngine : public CUIStateEngine
{
public:
    virtual HRESULT Process();
    virtual WCHAR*  DebugGetName()   { return L"SelectOptiMatchParams"; }
};

CSelectOptiMatchParamsStateEngine g_SelectOptiMatchParamsStateEngine;


void CreateOptiMatchParamsMenu( MENU_HEADER* pMenuHeader, MENU* pMenu );
BOOL HandleOptiMatchParamsMenu( MENU_HEADER* pMenuHeader, DWORD dwInput );
BOOL OptiMatch_SelectPreviousTrack();
BOOL OptiMatch_SelectNextTrack();
void OptiMatch_DrawGameMode( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem, int itemIndex );
void OptiMatch_DrawTrack( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem, int itemIndex );

static MENU Menu_OptiMatchParams = 
{
    TEXT_OPTIMATCH,
    MENU_DEFAULT|MENU_CENTRE_X|MENU_CENTRE_Y|MENU_PAD_FOR_ARROWS, // Menu type
    CreateOptiMatchParamsMenu,                    // Create menu function
    HandleOptiMatchParamsMenu,                    // Input handler function
    NULL,                                   // Menu draw function
    0,                    // X coord
    0,                    // Y Coord
};


static LONG g_lOptiMatchGameMode       = 0;
static LONG g_bOptiMatchUseAnyGameMode = TRUE;
static LONG g_lOptiMatchTrackNum       = 0;
static LONG g_bOptiMatchUseAnyTrack    = TRUE;

static MENU_ITEM MenuItem_OptiMatchGameMode = 
{
    TEXT_OPTIMATCH_GAMEMODE,                // Text label index
    12.0f * MENU_TEXT_WIDTH,                // Space needed to draw item data
    NULL,                                   // Data
    OptiMatch_DrawGameMode,                 // Draw Function
};

static MENU_ITEM MenuItem_OptiMatchTrack = 
{
    TEXT_OPTIMATCH_TRACK,                   // Text label index
    18.0f * MENU_TEXT_WIDTH,                // Space needed to draw item data
    NULL,                                   // Data
    OptiMatch_DrawTrack,                    // Draw Function
};


// Create
void CreateOptiMatchParamsMenu( MENU_HEADER* pMenuHeader, MENU* pMenu )
{
    // Add menu items
    pMenuHeader->AddMenuItem( &MenuItem_OptiMatchGameMode );
    pMenuHeader->AddMenuItem( &MenuItem_OptiMatchTrack );

    //$TODO - fix this hack
    // Valid levels depend on game type, but yet game type is allowed to
    // be "any" for optimatches. Crapola, this is fatally flawed. So
    // here, just hack in a game type for now
    GameSettings.GameType = GAMETYPE_SINGLE;
}

BOOL HandleOptiMatchParamsMenu( MENU_HEADER* pMenuHeader, DWORD dwInput )
{
    switch( dwInput )
    {
        case MENU_INPUT_LEFT:
            switch( pMenuHeader->m_pCurrentItem->TextIndex )
            {
                case TEXT_OPTIMATCH_GAMEMODE:
//$BUGBUG: This #ifdef is ONLY for May02_TechBeta!!
/// For the final release, we want to allow users to select different game modes.
/// So remove this #ifdef when all game modes are supported.
#ifdef SHIPPING
                // Don't allow user to select game mode.
#else
                    if( g_lOptiMatchGameMode == 0 && FALSE == g_bOptiMatchUseAnyGameMode )
                    {
                        g_bOptiMatchUseAnyGameMode = TRUE;
                        return TRUE;
                    }
                    if( g_lOptiMatchGameMode > 0 )
                    {
                        g_lOptiMatchGameMode--;
                        return TRUE;
                    }
#endif //SHIPPING
                    break;

                case TEXT_OPTIMATCH_TRACK:
                    return OptiMatch_SelectPreviousTrack(); 
            }
            break;

        case MENU_INPUT_RIGHT:
            switch( pMenuHeader->m_pCurrentItem->TextIndex )
            {
                case TEXT_OPTIMATCH_GAMEMODE:
//$BUGBUG: This #ifdef is ONLY for May02_TechBeta!!
/// For the final release, we want to allow users to select different game modes.
/// So remove this #ifdef when all game modes are supported.
#ifdef SHIPPING
                // Don't allow user to select game mode.
#else
                    if( g_bOptiMatchUseAnyGameMode )
                    {
                        g_bOptiMatchUseAnyGameMode = FALSE;
                        g_lOptiMatchGameMode = 0;
                        return TRUE;
                    }

                    if( g_lOptiMatchGameMode < 1 )
                    {
                        g_lOptiMatchGameMode++;
                        return TRUE;
                    }
#endif //SHIPPING
                    break;

                case TEXT_OPTIMATCH_TRACK:
                    return OptiMatch_SelectNextTrack(); 
            }
            break;

        case MENU_INPUT_UP:
            return SelectPreviousMenuItem( pMenuHeader, pMenuHeader->m_pMenu, NULL );
        
        case MENU_INPUT_DOWN:
            return SelectNextMenuItem( pMenuHeader, pMenuHeader->m_pMenu, NULL );
        
        case MENU_INPUT_BACK:
            g_SelectOptiMatchParamsStateEngine.Return( STATEENGINE_TERMINATED );
            return TRUE;

        case MENU_INPUT_SELECT:
            if( g_bOptiMatchUseAnyGameMode )
            {
                GameSettings.GameType = GAMETYPE_NONE;
            }
            else
            {
                if( g_lOptiMatchGameMode == 0 )
                    GameSettings.GameType = GAMETYPE_NETWORK_RACE;
                else
                    GameSettings.GameType = GAMETYPE_NETWORK_BATTLETAG;
            }
            
            if( g_bOptiMatchUseAnyTrack )
            {
                // gTitleScreenVars.iLevelNum = MATCH_TRACKID_ANY;
                g_lMatchmakingLevelNum = MATCH_TRACKID_ANY;
            }
            else
            {
                // gTitleScreenVars.iLevelNum = g_lOptiMatchTrackNum;
                g_lMatchmakingLevelNum = g_lOptiMatchTrackNum;
            }
            
            g_SelectOptiMatchParamsStateEngine.Return( STATEENGINE_COMPLETED );
            return TRUE;
    }

    return FALSE;
}

BOOL OptiMatch_SelectPreviousTrack()
{
//  if( g_bOptiMatchUseAnyTrack )
//  {
//      g_bOptiMatchUseAnyTrack = FALSE;
//      g_lOptiMatchTrackNum    = LEVEL_NSHIPPED_LEVELS;
//  }

    long level = g_lOptiMatchTrackNum;

    // get next track
    while( --level >= 0 )
    {
        if( IsLevelSelectable(level) && IsLevelAvailable(level)  && DoesLevelExist(level) )
        {
            g_lOptiMatchTrackNum = level;
            return TRUE;
        }
    }

    // Traversed past the edge of the list, so use any track
    g_bOptiMatchUseAnyTrack = TRUE;
    return TRUE;
}

BOOL OptiMatch_SelectNextTrack()
{
    if( g_bOptiMatchUseAnyTrack )
    {
        g_bOptiMatchUseAnyTrack = FALSE;
        g_lOptiMatchTrackNum    = -1;
    }

    long level = g_lOptiMatchTrackNum;

    // get next track
    while( ++level < LEVEL_NSHIPPED_LEVELS )
    {
        if( IsLevelSelectable(level) && IsLevelAvailable(level)  && DoesLevelExist(level) )
        {
            g_lOptiMatchTrackNum = level;
            return TRUE;
        }
    }

    // Traversed past the edge of the list, so use any track
//  g_bOptiMatchUseAnyTrack = TRUE;
    return TRUE;
}

void OptiMatch_DrawGameMode( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem, int itemIndex )
{
    WCHAR* strMenuItemText;
    FLOAT  xPos = pMenuHeader->m_XPos + pMenuHeader->m_ItemTextWidth + MENU_TEXT_GAP;
    FLOAT  yPos = pMenuHeader->m_YPos + (MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP) * itemIndex;

    if( g_bOptiMatchUseAnyGameMode )
        strMenuItemText = TEXT_TABLE(TEXT_OPTIMATCH_ANY);
    else
        strMenuItemText = TEXT_TABLE(TEXT_RACE + g_lOptiMatchGameMode);

    DrawMenuTextWithArrows( (pMenuItem == pMenuHeader->m_pCurrentItem) ? TRUE : FALSE, 
                        xPos, yPos, MENU_TEXT_RGB_NORMAL, strMenuItemText, 
                        pMenuHeader->m_ItemDataWidth );
}

void OptiMatch_DrawTrack( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem, int itemIndex )
{
    FLOAT xPos = pMenuHeader->m_XPos + pMenuHeader->m_ItemTextWidth + MENU_TEXT_GAP;
    FLOAT yPos = pMenuHeader->m_YPos + (MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP) * itemIndex;

    if( g_bOptiMatchUseAnyTrack )
    {
        swprintf( MenuBuffer, L"%s", TEXT_TABLE(TEXT_OPTIMATCH_ANY) );
    }
    else
    {
        LEVELINFO* pLevelInfo = GetLevelInfo( g_lOptiMatchTrackNum );
        if( NULL == pLevelInfo )
            return;

        swprintf( MenuBuffer, L"%s", pLevelInfo->strName );
    }

    DrawMenuTextWithArrows( (pMenuItem == pMenuHeader->m_pCurrentItem) ? TRUE : FALSE, 
                        xPos, yPos, MENU_TEXT_RGB_NORMAL, MenuBuffer, 
                        pMenuHeader->m_ItemDataWidth );
}




//-----------------------------------------------------------------------------
// Name: Process()
// Desc: Main control function for the state engine
//-----------------------------------------------------------------------------
HRESULT CSelectOptiMatchParamsStateEngine::Process()
{
    enum
    {
        SELECTOPTIMATCHPARAMS_STATE_BEGIN = STATEENGINE_STATE_BEGIN,
        SELECTOPTIMATCHPARAMS_STATE_MAINLOOP,
    };

    switch( m_State )
    {
        case SELECTOPTIMATCHPARAMS_STATE_BEGIN:
            // Initialize state
            GameSettings.GameType = GAMETYPE_NONE;

            // Set the menu and camera settings
            g_pMenuHeader->SetNextMenu( &Menu_OptiMatchParams );
            g_pTitleScreenCamera->SetNewPos( TITLESCREEN_CAMPOS_MULTI );

            m_State = SELECTOPTIMATCHPARAMS_STATE_MAINLOOP;
            break;

        case SELECTOPTIMATCHPARAMS_STATE_MAINLOOP:
            // Nothing to do, as control is in the menus
            break;
    }

    // Handle the menus
    g_pMenuHeader->HandleMenus();

    return S_FALSE;
}




//-----------------------------------------------------------------------------
// Name: COptiMatchStateEngine::Process()
// Desc: Process all step necessary to collect match parameters, perform 
//       XOnline query, present found matches, select car, and start game
//-----------------------------------------------------------------------------
HRESULT COptiMatchStateEngine::Process()
{
    enum
    {
        OPTIMATCH_STATE_BACKOUT = STATEENGINE_STATE_EXIT,
        OPTIMATCH_STATE_BEGIN = STATEENGINE_STATE_BEGIN,
        OPTIMATCH_STATE_SELECTMATCHPARAMS,
        OPTIMATCH_STATE_POSTSELECTMATCHPARAMS,
        OPTIMATCH_STATE_SELECTMATCH,
        OPTIMATCH_STATE_POSTSELECTMATCH,
        OPTIMATCH_STATE_ENTERNAME,
        OPTIMATCH_STATE_POSTENTERNAME,
        OPTIMATCH_STATE_SELECTCAR,
        OPTIMATCH_STATE_POSTSELECTCAR,
        OPTIMATCH_STATE_WAITINGROOM,
        OPTIMATCH_STATE_POSTWAITINGROOM,
        OPTIMATCH_STATE_STARTGAME,
    };

    switch( m_State )
    {
        //$CLEANUP: OptiMatch and CreateMatch handle restoration of GameSettings.GameType and gTitleScreenVars.iLevelNum differently.
        /// They really should work the same way (since OptiMatch and CreateMatch are so similar).
        case OPTIMATCH_STATE_BACKOUT:
            // No cleanup required here.
            Return( STATEENGINE_TERMINATED );
            break;

        case OPTIMATCH_STATE_BEGIN:
        case OPTIMATCH_STATE_SELECTMATCHPARAMS:
            // Switch control to the Select Race state engine
            Call( &g_SelectOptiMatchParamsStateEngine );
            m_State = OPTIMATCH_STATE_POSTSELECTMATCHPARAMS;
            break;

        case OPTIMATCH_STATE_POSTSELECTMATCHPARAMS:
            if( g_SelectOptiMatchParamsStateEngine.GetStatus() == STATEENGINE_TERMINATED )
                return GotoState( OPTIMATCH_STATE_BACKOUT );

            // Else, fall through to next state

        case OPTIMATCH_STATE_SELECTMATCH:
            Call( &g_SelectMatchStateEngine );
            m_State = OPTIMATCH_STATE_POSTSELECTMATCH;
            break;

        case OPTIMATCH_STATE_POSTSELECTMATCH:
            // If user backed out, go back a few states
            if( g_SelectMatchStateEngine.GetStatus() == STATEENGINE_TERMINATED )
            {
                ClearSessionList();  // clear list whenever user backs out from viewing session list
                return GotoState( OPTIMATCH_STATE_SELECTMATCHPARAMS );
            }

            // Else, fall through to next state

        case OPTIMATCH_STATE_ENTERNAME:
            // Call the EnterName state engine
            Call( &g_EnterNameStateEngine );
            m_State = OPTIMATCH_STATE_POSTENTERNAME;
            break;

        case OPTIMATCH_STATE_POSTENTERNAME:
            // If user backed out, go back a few states
            if( g_EnterNameStateEngine.GetStatus() == STATEENGINE_TERMINATED )
                return GotoState( OPTIMATCH_STATE_SELECTMATCH );
            // Else, fall through to next state

        case OPTIMATCH_STATE_SELECTCAR:
            if( GameSettings.RandomCars )
            {
                // Skip ahead past car selection
                m_State = OPTIMATCH_STATE_WAITINGROOM;
                break;
            }

            // Call the SelectCar state engine
            Call( &g_SelectCarStateEngine );
            m_State = OPTIMATCH_STATE_POSTSELECTCAR;
            break;

        case OPTIMATCH_STATE_POSTSELECTCAR:
            // If user backed out, go back a few states
            if( g_SelectCarStateEngine.GetStatus() == STATEENGINE_TERMINATED )
                return GotoState( OPTIMATCH_STATE_ENTERNAME );
            // Else, fall through to next state

        case OPTIMATCH_STATE_WAITINGROOM:
// $(UITODO) render some progress while waiting for XOnline response
// $(UITODO) render processing and handle "BACK" as cancel

            // Join the session
            if( !JoinSession( (char)SessionPick) )
            {
                DeleteSessionListEntry( SessionPick );
                SessionPick = 0;

                // Note: error message gets displayed by JoinSession
                m_State = OPTIMATCH_STATE_SELECTMATCH;
                break;
            }
    
//$ADDITION - need to do some other stuff after joining session (eg, add players, etc)
            LocalPlayerID = INVALID_PLAYER_ID; // to know when server acknowledges our Join request
            RequestAddPlayers( dwLocalPlayerCount );  //$CMP_NOTE: should we do this elsewhere?
//$END_ADDITION
    
//$REMOVED_DOESNOTHING            CreatePlayer( gTitleScreenVars.PlayerData[0].nameEnter, DP_CLIENT_PLAYER );
    
            // Set random car / track flag
            GameSettings.RandomCars  = SessionList[SessionPick].RandomCars;
            GameSettings.RandomTrack = SessionList[SessionPick].RandomTrack;
    
            // Call the WaitingRoom state engine
            Call( &g_WaitingRoomStateEngine );
            m_State = OPTIMATCH_STATE_POSTWAITINGROOM;
            break;
    
        case OPTIMATCH_STATE_POSTWAITINGROOM:
            // If user backed out, go back a few states
            if( g_WaitingRoomStateEngine.GetStatus() == STATEENGINE_TERMINATED )
            {
                // If connection to server was lost, then we already called LeaveSession().
                // And we displayed a status message in WaitingRoom UI, so just back out.
                if( !IsInGameSession() )
                {
                    return GotoState( OPTIMATCH_STATE_BACKOUT );
                }
                else
                {
                    // Leave the session
                    LeaveSession();

                    if( GameSettings.RandomCars )
                        return GotoState( OPTIMATCH_STATE_ENTERNAME );
                    else
                        return GotoState( OPTIMATCH_STATE_SELECTCAR );
                }
            }
    
            // (waiting for the game to start)
            break;
    }

    // Handle the menus
    g_pMenuHeader->HandleMenus();

    return S_FALSE;
}




//-----------------------------------------------------------------------------
// Name: CQuickMatchStateEngine::Process()
// Desc: Start simple quick XOnline query, pump task, when results recieved
//       display first, or show error, user may retry back or accept, 
//       join match pump join task, display error or start game.
//-----------------------------------------------------------------------------
HRESULT CQuickMatchStateEngine::Process()
{
    enum
    {
        QUICKMATCH_STATE_BACKOUT = STATEENGINE_STATE_EXIT,
        QUICKMATCH_STATE_BEGIN = STATEENGINE_STATE_BEGIN,
        QUICKMATCH_STATE_SELECTMATCH,
        QUICKMATCH_STATE_POSTSELECTMATCH,
        QUICKMATCH_STATE_ENTERNAME,
        QUICKMATCH_STATE_POSTENTERNAME,
        QUICKMATCH_STATE_SELECTCAR,
        QUICKMATCH_STATE_POSTSELECTCAR,
        QUICKMATCH_STATE_WAITINGROOM,
        QUICKMATCH_STATE_POSTWAITINGROOM,
        QUICKMATCH_STATE_STARTGAME,
    
    };

    static DWORD m_dwPreviousGameType;
    static int   m_iPreviousLevelNum;

    switch( m_State )
    {
        case QUICKMATCH_STATE_BACKOUT:
            // Restore states
            GameSettings.GameType = m_dwPreviousGameType;
            gTitleScreenVars.iLevelNum = m_iPreviousLevelNum;
            
            Return( STATEENGINE_TERMINATED );
            break;

        case QUICKMATCH_STATE_BEGIN:
            // Save old states
            m_dwPreviousGameType = GameSettings.GameType;
            m_iPreviousLevelNum  = gTitleScreenVars.iLevelNum;

            // Look for any track and any game type
            GameSettings.GameType = GAMETYPE_NONE;
            // gTitleScreenVars.iLevelNum = MATCH_TRACKID_ANY;
            g_lMatchmakingLevelNum = MATCH_TRACKID_ANY;

            // Fall through to next state

        case QUICKMATCH_STATE_SELECTMATCH:
            Call( &g_SelectMatchStateEngine );
            m_State = QUICKMATCH_STATE_POSTSELECTMATCH;
            break;

        case QUICKMATCH_STATE_POSTSELECTMATCH:
            // If user backed out, go back a few states
            if( g_SelectMatchStateEngine.GetStatus() == STATEENGINE_TERMINATED )
            {
                ClearSessionList();  // clear list whenever user backs out from viewing session list
                return GotoState( QUICKMATCH_STATE_BACKOUT );
            }

            // Else, fall through to next state

        case QUICKMATCH_STATE_ENTERNAME:
            // Call the EnterName state engine
            Call( &g_EnterNameStateEngine );
            m_State = QUICKMATCH_STATE_POSTENTERNAME;
            break;

        case QUICKMATCH_STATE_POSTENTERNAME:
            // If user backed out, go back a few states
            if( g_EnterNameStateEngine.GetStatus() == STATEENGINE_TERMINATED )
                return GotoState( QUICKMATCH_STATE_SELECTMATCH );
            // Else, fall through to next state

        case QUICKMATCH_STATE_SELECTCAR:
            if( GameSettings.RandomCars )
            {
                // Skip ahead past car selection
                m_State = QUICKMATCH_STATE_WAITINGROOM;
                break;
            }

            // Call the SelectCar state engine
            Call( &g_SelectCarStateEngine );
            m_State = QUICKMATCH_STATE_POSTSELECTCAR;
            break;

        case QUICKMATCH_STATE_POSTSELECTCAR:
            // If user backed out, go back a few states
            if( g_SelectCarStateEngine.GetStatus() == STATEENGINE_TERMINATED )
                return GotoState( QUICKMATCH_STATE_ENTERNAME );
            // Else, fall through to next state

        case QUICKMATCH_STATE_WAITINGROOM:
// $(UITODO) render some progress while waiting for XOnline response
// $(UITODO) render processing and handle "BACK" as cancel

            // Join the session
            if( !JoinSession( (char)SessionPick) )
            {
                DeleteSessionListEntry(SessionPick);
                SessionPick = 0;
    
                // Note: error message gets displayed by JoinSession
                m_State = QUICKMATCH_STATE_SELECTMATCH;
                break;
            }
    
//$ADDITION - need to do some other stuff after joining session (eg, add players, etc)
            LocalPlayerID = INVALID_PLAYER_ID; // to know when server acknowledges our Join request
            RequestAddPlayers( dwLocalPlayerCount );  //$CMP_NOTE: should we do this elsewhere?
//$END_ADDITION
    
//$REMOVED_DOESNOTHING            CreatePlayer( gTitleScreenVars.PlayerData[0].nameEnter, DP_CLIENT_PLAYER );
    
            // Set random car / track flag
            GameSettings.RandomCars  = SessionList[SessionPick].RandomCars;
            GameSettings.RandomTrack = SessionList[SessionPick].RandomTrack;
    
            // Call the WaitingRoom state engine
            Call( &g_WaitingRoomStateEngine );
            m_State = QUICKMATCH_STATE_POSTWAITINGROOM;
            break;
    
        case QUICKMATCH_STATE_POSTWAITINGROOM:
            // If user backed out, go back a few states
            if( g_WaitingRoomStateEngine.GetStatus() == STATEENGINE_TERMINATED )
            {
                // If connection to server was lost, then we already called LeaveSession().
                // And we displayed a status message in WaitingRoom UI, so just back out.
                if( !IsInGameSession() )
                {
                    return GotoState( QUICKMATCH_STATE_BACKOUT );
                }
                else
                {
                    // Leave the session
                    LeaveSession();

                    if( GameSettings.RandomCars )
                        return GotoState( QUICKMATCH_STATE_ENTERNAME );
                    else
                        return GotoState( QUICKMATCH_STATE_SELECTCAR );
                }
            }
    
            // (waiting for the game to start)
            break;
    }

    // Handle the menus
    g_pMenuHeader->HandleMenus();

    return S_FALSE;
}





//-----------------------------------------------------------------------------
// Name: HandleEnter** / HandleExit**  functions
// Desc: Init/uninit work necessary when entering/exiting state engine.
//-----------------------------------------------------------------------------
VOID CJoinMatchStateEngine::HandleEnterFromParent()  { DisableSessionListUpdate = false; }
VOID CJoinMatchStateEngine::HandleExitToParent()     { DisableSessionListUpdate = false; }
VOID CJoinMatchStateEngine::HandleEnterFromChild()  { DisableSessionListUpdate = false; }
VOID CJoinMatchStateEngine::HandleExitToChild()     { DisableSessionListUpdate = false; }
//$REVISIT: handling the **Child() cases is kind of ugly, but it's necessary
/// because the UI code currently lets you proceed out the end of a state
/// engine without "unwinding the stack".  If the code returned out of all
/// state engines before exiting, we could eliminate the **Child() funcs.
//$REVISIT: actually, we might not need to set this for XOnline games b/c we
/// don't auto-refresh the session list.  Though it doesn't hurt to do this,
/// and it makes us consistent with the SysLink code...

//-----------------------------------------------------------------------------
// Name: CJoinMatchStateEngine::Process()
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CJoinMatchStateEngine::Process()
{
    enum
    {
        JOINMATCH_STATE_BEGIN = STATEENGINE_STATE_BEGIN,
        JOINMATCH_STATE_ENTERNAME,
        JOINMATCH_STATE_POSTENTERNAME,
        JOINMATCH_STATE_SELECTCAR,
        JOINMATCH_STATE_POSTSELECTCAR,
        JOINMATCH_STATE_WAITINGROOM,
        JOINMATCH_STATE_POSTWAITINGROOM,
        JOINMATCH_STATE_BACKOUT = STATEENGINE_STATE_EXIT,
    };

    switch( m_State )
    {
        case JOINMATCH_STATE_BEGIN:
            // Set client
            GameSettings.MultiType = MULTITYPE_CLIENT;
            LocalPlayerReady = FALSE;
            HostQuit         = FALSE;
    
            //$REVISIT: should these 2 lines go away eventually?
            /// (Should only have 1 copy of this name variable.  And it should have been filled with value from user's profile.)
            strncpy( gTitleScreenVars.PlayerData[0].nameEnter, RegistrySettings.PlayerName, MAX_PLAYER_NAME );
            gTitleScreenVars.PlayerData[0].nameEnter[MAX_PLAYER_NAME-1] = '\0';

            // Wait until the search completes
            if( g_bXOnlineSessionSearchComplete )
            {
                if( GetSessionCount() > 0 )
                {
                    // Found the session - try to connect
                    m_State = JOINMATCH_STATE_ENTERNAME;
                }
                else
                {
                    // Session is gone - host must have killed the session
                    NextState( JOINMATCH_STATE_BACKOUT );
                    g_ShowSimpleMessage.Begin( TEXT_TABLE(TEXT_SESSION_GONE),
                                               TEXT_TABLE(TEXT_COULD_NOT_JOIN),
                                               NULL,
                                               TEXT_TABLE(TEXT_BUTTON_B_BACK) );
                }
            }
            break;
    
        case JOINMATCH_STATE_BACKOUT:
            // Un-set client
            GameSettings.MultiType = MULTITYPE_NONE;
            g_bInvitedByFriend = FALSE;
            ClearSessionList();
    
            Return( STATEENGINE_TERMINATED );
            break;
    
        case JOINMATCH_STATE_ENTERNAME:
            // Call the EnterName state engine
            g_pActiveStateEngine->Call( &g_EnterNameStateEngine );
            m_State = JOINMATCH_STATE_POSTENTERNAME;
            break;
    
        case JOINMATCH_STATE_POSTENTERNAME:
            // If user backed out, go back a few states
            if( g_EnterNameStateEngine.GetStatus() == STATEENGINE_TERMINATED )
                return GotoState( JOINMATCH_STATE_BACKOUT );
    
            if( GameSettings.RandomCars )
            {
                // Skip ahead past car selection
                return GotoState( JOINMATCH_STATE_WAITINGROOM );
            }
    
            // Else, fall through to next state
    
        case JOINMATCH_STATE_SELECTCAR:
            // Call the SelectCar state engine
            g_pActiveStateEngine->Call( &g_SelectCarStateEngine );
            m_State = JOINMATCH_STATE_POSTSELECTCAR;
            break;
    
        case JOINMATCH_STATE_POSTSELECTCAR:
            // If user backed out, go back a few states
            if( g_SelectCarStateEngine.GetStatus() == STATEENGINE_TERMINATED )
                return GotoState( JOINMATCH_STATE_ENTERNAME );

            // Else, fall through to next state
    
        case JOINMATCH_STATE_WAITINGROOM:
// $(UITODO) render some progress while waiting for XOnline response
// $(UITODO) render processing and handle "BACK" as cancel

            // Join the session
            if( !JoinSession( (char)SessionPick) )
            {
                DeleteSessionListEntry(SessionPick);
                SessionPick = 0;
    
                // Note: error message gets displayed by JoinSession
                m_State = JOINMATCH_STATE_BACKOUT;
                break;
            }
    
//$ADDITION - need to do some other stuff after joining session (eg, add players, etc)
            LocalPlayerID = INVALID_PLAYER_ID; // to know when server acknowledges our Join request
            RequestAddPlayers( dwLocalPlayerCount );  //$CMP_NOTE: should we do this elsewhere?
            g_bInvitedByFriend = FALSE;
//$END_ADDITION
    
//$REMOVED_DOESNOTHING            CreatePlayer( gTitleScreenVars.PlayerData[0].nameEnter, DP_CLIENT_PLAYER );
    
            // Set random car / track flag
            GameSettings.RandomCars  = SessionList[SessionPick].RandomCars;
            GameSettings.RandomTrack = SessionList[SessionPick].RandomTrack;
    
            // Call the WaitingRoom state engine
            g_pActiveStateEngine->Call( &g_WaitingRoomStateEngine );
            m_State = JOINMATCH_STATE_POSTWAITINGROOM;
            break;
    
        case JOINMATCH_STATE_POSTWAITINGROOM:
            // If user backed out, go back a few states
            if( g_WaitingRoomStateEngine.GetStatus() == STATEENGINE_TERMINATED )
            {
                // If connection to server was lost, then we already called LeaveSession().
                // And we displayed a status message in WaitingRoom UI, so just back out.
                if( ! IsInGameSession() )
                {
                    return GotoState( JOINMATCH_STATE_BACKOUT );
                }
                else
                {
                    // Leave the session
                    LeaveSession();

                    if( GameSettings.RandomCars )
                        return GotoState( JOINMATCH_STATE_ENTERNAME );
                    else
                        return GotoState( JOINMATCH_STATE_SELECTCAR );
                }
            }
    
            // ???
            // (waiting for the game to start)
            break;
    }

    // Handle the menus
    g_pMenuHeader->HandleMenus();

    return S_FALSE;
}







static BOOL g_bNeedRequestSessionList = TRUE;

static VOID CreatePlayLiveSelectMatchMenu( MENU_HEADER *pMenuHeader, MENU *pMenu );
static BOOL HandlePlayLiveSelectMatchMenu( MENU_HEADER* pMenuHeader, DWORD dwInput );
static VOID DrawPlayLiveSelectMatchMenu( MENU_HEADER* pMenuHeader, MENU* pMenu );

static MENU Menu_PlayLiveSelectMatch = 
{
    TEXT_SELECTMATCH,
    MENU_DEFAULT|MENU_NOBOX,  // Menu type
    CreatePlayLiveSelectMatchMenu,          // Create menu function
    HandlePlayLiveSelectMatchMenu,          // Input handler function
    DrawPlayLiveSelectMatchMenu,            // Menu draw function
    0,                                      // X coord
    0,                                      // Y coord 
};


VOID CreatePlayLiveSelectMatchMenu( MENU_HEADER *pMenuHeader, MENU *pMenu )
{
    // Add the dummy menu item necessary to check for keys and call the frame update function
    pMenuHeader->AddMenuItem( TEXT_NONE );

    pMenu->fWidth  = 640.0f;
    pMenu->fHeight = 480.0f;
}


BOOL HandlePlayLiveSelectMatchMenu( MENU_HEADER* pMenuHeader, DWORD dwInput )
{
    switch( dwInput )
    {
        case MENU_INPUT_UP:
            if( g_bXOnlineSessionSearchComplete &&
               (GetSessionCount() > 0) && (SessionPick > 0) )
            {
                SessionPick--;
                return TRUE;
            }
            break;

        case MENU_INPUT_DOWN:
            if( g_bXOnlineSessionSearchComplete &&
               (GetSessionCount() > 0) && (SessionPick < GetSessionCount()-1) )
            {
                SessionPick++;
                return TRUE;
            }
            break;

        case MENU_INPUT_X:
            if( g_bXOnlineSessionSearchComplete )
            {
                // Request updated session list
                g_bNeedRequestSessionList = TRUE;
                return TRUE;
            }
            break;

        case MENU_INPUT_Y:
            if( g_bXOnlineSessionSearchComplete )
            {
                // Request updated session list
                g_bNeedRequestSessionList = TRUE;
    
                // Transfer control to the Create Match state engine
                g_SelectMatchStateEngine.Call( &g_CreateMatchStateEngine );
                return TRUE;
            }
            break;

        case MENU_INPUT_BACK:
            g_SelectMatchStateEngine.Return( STATEENGINE_TERMINATED );
            return TRUE;

        case MENU_INPUT_SELECT:
            if( g_bXOnlineSessionSearchComplete  &&  GetSessionCount() > 0 )
            {
                g_SelectMatchStateEngine.Return( STATEENGINE_COMPLETED );
                return TRUE;
            }
            break;
    }
    return FALSE;
}

void DrawPlayLiveSelectMatchMenu( MENU_HEADER* pMenuHeader, MENU* pMenu )
{
    short i;
    FLOAT xPos, yPos, xSize, ySize;
    long rgb;
    WCHAR* status;

    // draw headings
    xSize = (640*0.85f);
    ySize = (480*0.85f) - 60;
    xPos  = (640*0.15f)/2;
    yPos  = (480*0.15f)/2 + 60;

    xPos += pMenuHeader->m_XPos;
    yPos += pMenuHeader->m_YPos;

    // draw spru box
    DrawNewSpruBoxWithTabs( (xPos) - 10,
                            (yPos) - 10,
                            (xSize) + 20,
                            (ySize) + 20 );

    BeginTextState();

    xPos += MENU_BORDER_WIDTH;
//    yPos += MENU_BORDER_HEIGHT;

    DrawMenuText( xPos +   0, yPos, MENU_TEXT_RGB_HILITE, TEXT_TABLE(TEXT_HOST) );
    DrawMenuText( xPos + 144, yPos, MENU_TEXT_RGB_HILITE, TEXT_TABLE(TEXT_TRACK) );
    DrawMenuText( xPos + 296, yPos, MENU_TEXT_RGB_HILITE, TEXT_TABLE(TEXT_GAME) );
    DrawMenuText( xPos + 384, yPos, MENU_TEXT_RGB_HILITE, TEXT_TABLE(TEXT_STATUS) );
    DrawMenuText( xPos + 470, yPos, MENU_TEXT_RGB_HILITE, TEXT_TABLE(TEXT_PING) );
    yPos += MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP + MENU_TEXT_VSKIP;

    RenderTP = 0;

    GetRemoteMessages();  //$ADDITION: we need to do this manually, since we're not using DPlay to enumerate sessions
                          //$REVISIT(cprince): should all the scattered calls to GetRemoteMessages (in UI code) be replaced with a single call inside TitleScreen()?  We might not want to call GetRemoteMessages until after certain things have been done (eg, JoinSession has been called), and/or might need a bConnected flag.

    // Draw sessions + players
    if( GetSessionCount() > 0 )
    {
        for( i = 0 ; i < GetSessionCount() ; i++, yPos += MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP )
        {
            if( SessionPick == i ) 
            {
                DWORD dwAlpha = ((long)(sinf((FLOAT)TIME2MS(TimerCurrent)/200.0f) * 64.0f + 192.0f))<<24L;
                rgb = dwAlpha|MENU_COLOR_WHITE;
            }
            else 
                rgb = MENU_TEXT_RGB_NORMAL;

            WCHAR strLevelName[MAX_LEVEL_NAME] = L"";
            LEVELINFO* pLevelInfo = GetLevelInfo(SessionList[i].iLevelNum);
            if( pLevelInfo ) 
                swprintf( strLevelName, L"%.*s", MAX_LEVEL_NAME-1, pLevelInfo->strName );

            if( i == SessionPick ) 
                gTrackScreenLevelNum = SessionList[i].iLevelNum;

            DrawMenuText(xPos +   0, yPos, rgb, SessionList[i].wstrHostNickname, 136 );
            DrawMenuText(xPos + 144, yPos, rgb, SessionList[i].RandomTrack ? TEXT_TABLE(TEXT_RANDOM) : strLevelName, 144 );
            DrawMenuText(xPos + 296, yPos, rgb, SessionList[i].GameType == GAMETYPE_NETWORK_RACE ? TEXT_TABLE(TEXT_RACE) : TEXT_TABLE(TEXT_BATTLETAG) );

            if( SessionList[i].Version != (unsigned long)MultiplayerVersion ) 
                status = TEXT_TABLE(TEXT_WRONGVERSION);
            else if( SessionList[i].Started ) 
                status = TEXT_TABLE(TEXT_STARTED);
            else if (SessionList[i].nCurrPlayersPublic == SessionList[i].nMaxPlayersPublic)  //$REVISIT: do we only want to check public slots (assuming most people can't use private slots and thus don't care)??
                status = TEXT_TABLE(TEXT_FULL);
            else 
                status = TEXT_TABLE(TEXT_OPEN);

            DrawMenuText(xPos + 384, yPos, rgb, status);

            if( SessionList[i].pQosResults != NULL
                && SessionList[i].pQosResults->cxnqosPending == 0 )
            {
                WCHAR strPing[16];
                swprintf( strPing, L"%dms", SessionList[i].pQosResults->axnqosinfo[0].wRttMedInMsecs );
                DrawMenuText( xPos + 470, yPos, rgb, strPing );
            }
        }

//$BUG: right now, this player list is broken, so don't bother trying to display it.
#if 0
        if( PlayerCount )
        {
            yPos = 240 + pMenuHeader->m_YPos;

            DrawMenuText(xPos, yPos, MENU_TEXT_RGB_HILITE, TEXT_TABLE(TEXT_PLAYER));
            DrawMenuText(xPos + MENU_TEXT_WIDTH * 18, yPos, MENU_TEXT_RGB_HILITE, TEXT_TABLE(TEXT_CAR));
            DrawMenuText(xPos + MENU_TEXT_WIDTH * 36, yPos, MENU_TEXT_RGB_HILITE, TEXT_TABLE(TEXT_STATUS));
            yPos += MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP;

            for( i = 0; i < PlayerCount; i++, yPos += MENU_TEXT_HEIGHT )
            {
                swprintf( MenuBuffer, L"%S", PlayerList[i].Name);
                DrawMenuText(xPos, yPos, MENU_TEXT_RGB_NORMAL, MenuBuffer );
                if (SessionList[SessionPick].RandomCars)
                    swprintf( MenuBuffer, L"%s", TEXT_TABLE(TEXT_RANDOM));
                else 
                    swprintf( MenuBuffer, L"%S", CarInfo[PlayerList[i].CarType].Name);
                DrawMenuText(xPos + 144, yPos, MENU_TEXT_RGB_NORMAL, MenuBuffer );

                //$BUG: do we want to implement 'Ready' state?
//$TODO: Display the actual player status (which still needs some under the hood work)
//              DrawMenuText(xPos + 288, yPos, MENU_TEXT_RGB_NORMAL, PlayerList[i].Data.Ready ? TEXT_TABLE(TEXT_READY) : TEXT_TABLE(TEXT_NOTREADY));
                DrawMenuText(xPos + 288, yPos, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_READY) );
            }
        }
#endif // 0
    }
    else
    {
        // If there's no sessions, inform the player that either we are still
        // searching, or that no matches were found.
        if( g_bXOnlineSessionSearchComplete )
        {
            g_pFont->DrawText( pMenuHeader->m_XPos + 320.0f, pMenuHeader->m_YPos + 240.0f, 
                                   MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_PLAYLIVE_NOMATCHESFOUND), XBFONT_CENTER_X );
        }
        else
        {
            g_pFont->DrawText( pMenuHeader->m_XPos + 320.0f, pMenuHeader->m_YPos + 240.0f, 
                                   MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_SEARCHINGFORGAMES), XBFONT_CENTER_X );
        }

    }

    if( g_bXOnlineSessionSearchComplete )
    {
        // Display the option to create a new game
        g_pFont->DrawText( xPos, 395 + pMenuHeader->m_YPos, MENU_TEXT_RGB_NORMAL, L"\202 " );
        g_pFont->DrawText( MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_REFRESHGAMELIST) );
        g_pFont->DrawText( xPos, 415 + pMenuHeader->m_YPos, MENU_TEXT_RGB_NORMAL, L"\203 " );
        g_pFont->DrawText( MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_CREATEGAME) );
    }
}




//-----------------------------------------------------------------------------
// Name: CSelectMatchStateEngine::Process()
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CSelectMatchStateEngine::Process()
{
    enum
    {
        SELECTMATCH_STATE_BACKOUT = STATEENGINE_STATE_EXIT,
        SELECTMATCH_STATE_BEGIN = STATEENGINE_STATE_BEGIN,
        SELECTMATCH_STATE_GETSESSIONLIST,
        SELECTMATCH_STATE_MAINLOOP,
    };

    switch( m_State )
    {
        case SELECTMATCH_STATE_BEGIN:
            // Set the menu and camera position
            g_pMenuHeader->SetNextMenu( &Menu_PlayLiveSelectMatch );
            g_pTitleScreenCamera->SetNewPos( TITLESCREEN_CAMPOS_MULTI );

            m_State = SELECTMATCH_STATE_GETSESSIONLIST;
            // Fall through

        case SELECTMATCH_STATE_GETSESSIONLIST:
            // Set client
            GameSettings.MultiType = MULTITYPE_CLIENT;
            LocalPlayerReady = FALSE;
            HostQuit         = FALSE;

            // Look for sessions
            RequestSessionList();
            g_bNeedRequestSessionList = FALSE;

            m_State = SELECTMATCH_STATE_MAINLOOP;
            break;

        case SELECTMATCH_STATE_MAINLOOP:
            if( TRUE == g_bNeedRequestSessionList )
                return GotoState( SELECTMATCH_STATE_GETSESSIONLIST );

            // Nothing to do, as control is in the menus
            break;
    }

    // Handle the menus
    g_pMenuHeader->HandleMenus();

    return S_FALSE;
}



