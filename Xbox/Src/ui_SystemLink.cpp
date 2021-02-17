//-----------------------------------------------------------------------------
// File: ui_SystemLink.cpp
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"
#include "ui_MenuText.h"
#include "ui_MenuDraw.h"
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
#include "text.h"
#include "ui_StateEngine.h"
#include "ui_SystemLink.h"
#include "ui_EnterName.h"
#include "ui_SelectRaceMode.h"
#include "ui_SelectCar.h"
#include "ui_SelectTrack.h"
#include "ui_WaitingRoom.h"


extern long SessionRefreshFlag;
extern long HostnameEntry;


#define MENU_SYSTEMLINK_XPOS            0
#define MENU_SYSTEMLINK_YPOS            0

static VOID CreateSystemLinkMenu( MENU_HEADER* pMenuHeader, MENU* pMenu );
static BOOL HandleSystemLinkMenu( MENU_HEADER* pMenuHeader, DWORD dwInput );
static VOID DrawSystemLinkMenu( MENU_HEADER* pMenuHeader, MENU* pMenu );


extern MENU Menu_SystemLink; 


//-----------------------------------------------------------------------------
// The JoinSystemLinkGame state engine
//-----------------------------------------------------------------------------
class CJoinSystemLinkGameStateEngine : public CUIStateEngine
{
protected:
    virtual VOID HandleEnterFromParent();
    virtual VOID HandleExitToParent();
    virtual VOID HandleEnterFromChild();
    virtual VOID HandleExitToChild();
public:
    virtual HRESULT Process();
    virtual WCHAR*  DebugGetName()   { return L"JoinSystemLinkGame"; }
};

extern CJoinSystemLinkGameStateEngine g_JoinSystemLinkGameStateEngine;




//-----------------------------------------------------------------------------
// The CreateSystemLinkGame state engine
//-----------------------------------------------------------------------------
class CCreateSystemLinkGameStateEngine : public CUIStateEngine
{
public:
    virtual HRESULT Process();
    virtual WCHAR*  DebugGetName()   { return L"CreateSystemLinkGame"; }
};

extern CCreateSystemLinkGameStateEngine g_CreateSystemLinkGameStateEngine;


//-----------------------------------------------------------------------------
// System Link Menu
//-----------------------------------------------------------------------------
MENU Menu_SystemLink = 
{
    TEXT_SYSTEMLINK,
    MENU_DEFAULT|MENU_NOBOX,                // Menu type
    CreateSystemLinkMenu,                   // Create menu function
    HandleSystemLinkMenu,                   // Input handler function
    DrawSystemLinkMenu,                     // Menu draw function
    MENU_SYSTEMLINK_XPOS,                   // X coord
    MENU_SYSTEMLINK_YPOS,                   // Y Coord
};




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
void CreateSystemLinkMenu( MENU_HEADER* pMenuHeader, MENU* pMenu )
{
    pMenu->fWidth  = 640.0f;
    pMenu->fHeight = 480.0f;
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
BOOL HandleSystemLinkMenu( MENU_HEADER* pMenuHeader, DWORD dwInput )
{
    // Allowed inputs are if different whether or not games are available to join
    if( GetSessionCount() == 0 )
    {
        switch( dwInput )
        {
            case MENU_INPUT_SELECT:
                // Call the Create SystemLink Game state engine
                g_SystemLinkStateEngine.Call( &g_CreateSystemLinkGameStateEngine );
                return TRUE;
        }
    }
    else // GetSessionCount() > 0
    {
        switch( dwInput )
        {
            case MENU_INPUT_SELECT:
                // Join a game
                if( SessionPick >= 0  &&  SessionPick < GetSessionCount() )
                {
//$REMOVED (tentative!!) - Xbox always connected
//                  // init connection
//                  if( !InitConnection((char)gTitleScreenVars.connectionType) )
//                  {
//                      // TODO: Back out somehow
//                  }
//$END_REMOVAL

//$REMOVED
//                  // get local IP
//                  GetIPString(LocalIP);
//$END_REMOVAL
                    // Call the Join SystemLink Game state engine
                    g_SystemLinkStateEngine.Call( &g_JoinSystemLinkGameStateEngine );
//$REMOVED - not used
//                  // Set session join flag
//                  if( 0 == SessionJoinFlag )
//                      SessionJoinFlag = 3;
//$END_REMOVAL

//$MOVED: this is now being done in _BEGIN state for JoinSystemLinkGameStateEngine
//                  // Set client
//                  GameSettings.MultiType = MULTITYPE_CLIENT;
//$END_MOVE
                    return TRUE;
                }
                else
                {
                    SessionPick = 0;

                    if( GetSessionCount() > 0 )
                    {
                        OutputDebugString( "Warning: SessionPick was invalid when player tried to select session.\n" );
                    }
                }
                break;

            case MENU_INPUT_Y:
//$MOVED: this is now being done in _BEGIN state for CreateSystemLinkGameStateEngine
//              // Set host
//              GameSettings.MultiType = MULTITYPE_SERVER;
//              LocalPlayerReady = FALSE;
//              HostQuit         = FALSE;
//
//              //$REVISIT: should these 2 lines go away eventually?
//              /// (Should only have 1 copy of this name variable.  And it should have been filled with value from user's profile.)
//              strncpy( gTitleScreenVars.PlayerData[0].nameEnter, RegistrySettings.PlayerName, MAX_PLAYER_NAME );
//              gTitleScreenVars.PlayerData[0].nameEnter[MAX_PLAYER_NAME-1] = '\0';
//$END_MOVE
                // Call the Create SystemLink Game state engine
                g_SystemLinkStateEngine.Call( &g_CreateSystemLinkGameStateEngine );
                return TRUE;

            case MENU_INPUT_UP:
                if( SessionPick )
                {
                    SessionPick--;
                    PlayerCount = 0;  //$REVISIT(cprince): do we need this line?  (We set PlayerCount elsewhere.)
                    return TRUE;
                }
                break;

            case MENU_INPUT_DOWN:
                if( SessionPick < GetSessionCount() - 1 )
                {
                    SessionPick++;
                    PlayerCount = 0;  //$REVISIT(cprince): do we need this line?  (We set PlayerCount elsewhere.)
                    return TRUE;
                }
                break;
        }
    }

    if( dwInput == MENU_INPUT_BACK )
    {
//$MOVED: this cleanup is now being done in same function where InitNetwork is called.
//#ifndef XBOX_DISABLE_NETWORK
//      if (!Lobby)
//#endif
//      {
//          KillNetwork();
////$MOVED: this is now being done in each state engine where gets set to value other than _NONE
////        GameSettings.MultiType = MULTITYPE_NONE;
////$END_MOVE
//      }
//$END_MOVE

//$MODIFIED
        ClearSessionList(); //$ADDITION(cprince): clear list when we back out of the session-list viewing screen
                            //$NOTE: this call would happen in the HandleExitToParent() function, if we were using a real state engine for this component of the UI.  (That's what we do in the PlayLive UI code.)
        g_SystemLinkStateEngine.Return( STATEENGINE_TERMINATED );

//      // don't need this; using HandleExitToParent() instead
//      //g_SystemLinkStateEngine.ProcessExitState();
//$END_MODIFICATIONS
        return TRUE;
    }

    return FALSE;
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
void DrawSystemLinkMenu( MENU_HEADER* pMenuHeader, MENU* pMenu )
{
//$MODIFIED
//    short i, j, k;
    short i;
//$END_MODIFICATIONS
    FLOAT xPos, yPos, xSize, ySize;
    long rgb;
    WCHAR* status;

    // request sessions / players?

//$HACK_MODIFIED: workaround for funky UI mapping that doesn't allow us to refresh manually
//    if (Keys[DIK_SPACE] && !LastKeys[DIK_SPACE] && !SessionRefreshFlag)
    if (!SessionRefreshFlag)
//$END_HACK
    {
//$MODIFIED
//        SessionRefreshFlag = 3;
        SessionRefreshFlag = 5*60; // hack to reduce network flooding (and improve UI usability)
//$END_MODIFICATIONS
    }

    if (SessionRefreshFlag)
    {
        if (!--SessionRefreshFlag)
        {
            RequestSessionList();  //$MODIFIED: was RefreshSessions (equivalent)
        }
    }

    // draw headings
    xSize = (640*0.85f);
    ySize = (480*0.85f) - 90;
    xPos  = (640*0.15f)/2;
    yPos  = (480*0.15f)/2 + 90;

    xPos += pMenuHeader->m_XPos;
    yPos += pMenuHeader->m_YPos;

    // draw spru box
    DrawNewSpruBoxWithTabs( (xPos) - 10,
                            (yPos) - 10,
                            (xSize) + 20,
                            (ySize) + 20 );

    xPos += MENU_BORDER_WIDTH;
    yPos += MENU_BORDER_HEIGHT;

    DrawMenuText( xPos +   0, yPos, MENU_TEXT_RGB_HILITE, TEXT_TABLE(TEXT_HOST));
    DrawMenuText( xPos + 144, yPos, MENU_TEXT_RGB_HILITE, TEXT_TABLE(TEXT_TRACK));
    DrawMenuText( xPos + 296, yPos, MENU_TEXT_RGB_HILITE, TEXT_TABLE(TEXT_GAME));
    DrawMenuText( xPos + 384, yPos, MENU_TEXT_RGB_HILITE, TEXT_TABLE(TEXT_STATUS));
    RenderTP = 0;

    BeginTextState();

    if( SessionRefreshFlag )
    {
        g_pFont->DrawText( pMenuHeader->m_XPos+320.0f, pMenuHeader->m_YPos+370.0f, MENU_TEXT_RGB_NORMAL, gTitleScreen_Text[TEXT_SEARCHINGFORGAMES], XBFONT_CENTER_X );
    }
//$REMOVED - not used
//    else if( SessionJoinFlag )
//    {
//        g_pFont->DrawText( pMenuHeader->m_XPos+320.0f, pMenuHeader->m_YPos+370.0f, MENU_TEXT_RGB_NORMAL, gTitleScreen_Text[TEXT_JOININGGAME], XBFONT_CENTER_X );
//    }
//$END_REMOVAL
    else
    {
        g_pFont->DrawText( pMenuHeader->m_XPos+320.0f, pMenuHeader->m_YPos+370.0f, MENU_TEXT_RGB_NORMAL, gTitleScreen_Text[TEXT_PRESS_A_TOREFRESHGAMELIST], XBFONT_CENTER_X );
    }

    // Draw the button help text
    if( GetSessionCount() == 0 )
    {
        g_pFont->DrawText( xPos, 415 + pMenuHeader->m_YPos, MENU_TEXT_RGB_NORMAL, L"\200 " );
        g_pFont->DrawText( MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_CREATEGAME) );
    }
    else
    {
        g_pFont->DrawText( xPos, 415 + pMenuHeader->m_YPos, MENU_TEXT_RGB_NORMAL, L"\200 " );
        g_pFont->DrawText( MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_JOINGAME) );

        g_pFont->DrawText( pMenuHeader->m_XPos+320, 415 + pMenuHeader->m_YPos, MENU_TEXT_RGB_NORMAL, L"\203 " );
        g_pFont->DrawText( MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_CREATEGAME) );
    }

    yPos += MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP;

    GetRemoteMessages();  //$ADDITION: we need to do this manually, since we're not using DPlay to enumerate sessions
                          //$REVISIT(cprince): should all the scattered calls to GetRemoteMessages (in UI code) be replaced with a single call inside TitleScreen()?  We might not want to call GetRemoteMessages until after certain things have been done (eg, JoinSession has been called), and/or might need a bConnected flag.

    // Draw sessions + players
    if ( GetSessionCount() )
    {
        for (i = 0 ; i < GetSessionCount() ; i++, yPos += MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP)
        {
            if( SessionPick == i ) 
            {
                DWORD dwAlpha = ((long)(sinf((FLOAT)TIME2MS(TimerCurrent)/200.0f) * 64.0f + 192.0f))<<24L;
                rgb = dwAlpha|MENU_COLOR_WHITE;
            }
            else 
                rgb = MENU_TEXT_RGB_NORMAL;

//$MODIFIED
//            char hostname[MAX_PLAYER_NAME + 1], levelname[MAX_LEVEL_NAME + 1], leveldir[MAX_LEVEL_DIR_NAME + 1];  //$REVISIT: do we really want/need "+1" for these?
//
//            j = 0;
//            while ((hostname[j] = SessionList[i].name[j]) != '\n' && SessionList[i].name[j]) j++;
//            hostname[j] = 0;
//            k = 0;
//            while ((leveldir[k] = SessionList[i].name[k + j + 1])) k++;
//
//            int levelNum = GetLevelNum(leveldir);
//            LEVELINFO *levelInfo = GetLevelInfo(levelNum);
//            if (levelInfo != NULL) {
//                strncpy(levelname, levelInfo->Name, MAX_LEVEL_NAME);
//            } else {
//                levelname[0] = '\0';
//            }
//
//            if (i == SessionPick) {
//                gTrackScreenLevelNum = levelNum;
//            }
//
//            DrawMenuText(xPos, yPos, rgb, hostname);
//            DrawMenuText(xPos + (MENU_TEXT_WIDTH * (MAX_PLAYER_NAME + 2)), yPos, rgb, SessionList[i].RandomTrack ? TEXT_TABLE(TEXT_RANDOM) : levelname);

            WCHAR strLevelName[MAX_LEVEL_NAME] = L"";
            LEVELINFO* levelInfo = GetLevelInfo(SessionList[i].iLevelNum);
            if( levelInfo != NULL )
                swprintf(strLevelName, L"%s", levelInfo->strName);

            if (i == SessionPick) 
                gTrackScreenLevelNum = SessionList[i].iLevelNum;

            DrawMenuText(xPos +   0, yPos, rgb, SessionList[i].wstrHostNickname, 136 );
            DrawMenuText(xPos + 144, yPos, rgb, SessionList[i].RandomTrack ? TEXT_TABLE(TEXT_RANDOM) : strLevelName);
//$END_MODIFICATIONS
            DrawMenuText(xPos + 296, yPos, rgb, SessionList[i].GameType == GAMETYPE_NETWORK_RACE ? TEXT_TABLE(TEXT_RACE) : TEXT_TABLE(TEXT_BATTLETAG));


            if (SessionList[i].Version != (unsigned long)MultiplayerVersion) 
                status = TEXT_TABLE(TEXT_WRONGVERSION);
            else if (SessionList[i].Started) 
                status = TEXT_TABLE(TEXT_STARTED);
//$MODIFIED
//            else if ((DWORD)PlayerCount == SessionList[i].nCurrPlayers) 
            else if (SessionList[i].nCurrPlayersPublic == SessionList[i].nMaxPlayersPublic)  //$REVISIT: do we only want to check public slots (assuming most people can't use private slots and thus don't care)??
//$END_MODIFICATIONS
                status = TEXT_TABLE(TEXT_FULL);
            else 
                status = TEXT_TABLE(TEXT_OPEN);

            DrawMenuText(xPos + 384, yPos, rgb, status);
        }

//$BUG: right now, this player list is broken, so don't bother trying to display it.
#if 0
        if( PlayerCount > 0 )
        {
            yPos = 240 + pMenuHeader->m_YPos;

            DrawMenuText( xPos +   0, yPos, MENU_TEXT_RGB_HILITE, TEXT_TABLE(TEXT_PLAYER) );
            DrawMenuText( xPos + 144, yPos, MENU_TEXT_RGB_HILITE, TEXT_TABLE(TEXT_CAR) );
            DrawMenuText( xPos + 288, yPos, MENU_TEXT_RGB_HILITE, TEXT_TABLE(TEXT_STATUS) );

            yPos += MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP;
            for (i = 0 ; i < PlayerCount ; i++, yPos += MENU_TEXT_HEIGHT)
            {
                swprintf( MenuBuffer, L"%S", PlayerList[i].Name);
                DrawMenuText(xPos, yPos, MENU_TEXT_RGB_NORMAL, MenuBuffer );
                if (SessionList[SessionPick].RandomCars)
                {
                    DrawMenuText(xPos + (MENU_TEXT_WIDTH * (MAX_PLAYER_NAME + 2)), yPos, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_RANDOM));
                }
                else 
                {
//$MODIFIED
//                  swprintf( MenuBuffer, L"%S", PlayerList[i].Data.CarName);
//                  DrawMenuText(xPos + 144, yPos, MENU_TEXT_RGB_NORMAL, MenuBuffer );
                    swprintf( MenuBuffer, L"%S", CarInfo[PlayerList[i].CarType].Name);
                    DrawMenuText(xPos + 144, yPos, MENU_TEXT_RGB_NORMAL, MenuBuffer );
                }
//$END_MODIFICATIONS

//$MODIFIED
//$TODO: Display the actual player status (which still needs some under the hood work)
//              DrawMenuText(xPos + 288, yPos, MENU_TEXT_RGB_NORMAL, PlayerList[i].Data.Ready ? TEXT_TABLE(TEXT_READY) : TEXT_TABLE(TEXT_NOTREADY));
                DrawMenuText(xPos + 288, yPos, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_READY) );
                //$BUG: do we want to implement 'Ready' state?
//$END_MODIFICATIONS
            }
        }
#endif // 0
    }

//$MODIFIED - we moved the state-engine call elsewhere, and removed the rest of this stuff
//    // Are we trying to join?
//    if( SessionJoinFlag )
//    {
//        // Yep, flag ready?
//        if( --SessionJoinFlag )
//            return;
//
//        // Yep, refresh session list
//        RequestSessionList();  //$MODIFIED: was RefreshSessions (equivalent)
//
//        // Valid session?
//        if( SessionPick > GetSessionCount() - 1 )
//            return;
//
////$HACK_REMOVAL: removed this so we can progress thru menus (but might actually want some of these checks later)
////// wrong checksum?
////
////        if (SessionList[SessionPick].Version != (unsigned long)MultiplayerVersion)
////            return;
////
////// session open?
////
////        if (SessionList[SessionPick].Started)
////            return;
////
////// max players?
////
////        if (SessionList[SessionPick].PlayerNum == (DWORD)PlayerCount)
////            return;
////$END_HACK
//
//        // Yep, it's safe to join the session
//
//        // Call the Join SystemLink Game state engine
//        g_SystemLinkStateEngine.Call( &g_JoinSystemLinkGameStateEngine );
//    }
//$END_MODIFICATIONS
}





CSystemLinkStateEngine g_SystemLinkStateEngine;


//-----------------------------------------------------------------------------
// Name: HandleEnter** / HandleExit**  functions
// Desc: Init/uninit work necessary when entering/exiting state engine.
//-----------------------------------------------------------------------------
VOID CSystemLinkStateEngine::HandleEnterFromParent()  { InitNetwork(); }
VOID CSystemLinkStateEngine::HandleEnterFromChild()   { InitNetwork(); }
VOID CSystemLinkStateEngine::HandleExitToParent()     { KillNetwork(); }
VOID CSystemLinkStateEngine::HandleExitToChild()      { KillNetwork(); }
//$REVISIT: handling the **Child() cases is kind of ugly, but it's necessary
/// because the UI code currently lets you proceed out the end of a state
/// engine without "unwinding the stack".  If the code returned out of all
/// state engines before exiting, we could eliminate the **Child() funcs.

//-----------------------------------------------------------------------------
// Name: Process()
// Desc: Main control function for the state engine
//-----------------------------------------------------------------------------
HRESULT CSystemLinkStateEngine::Process()
{
    enum
    {
        SYSTEMLINKGAME_STATE_BEGIN = STATEENGINE_STATE_BEGIN,
        SYSTEMLINKGAME_STATE_MAINLOOP,
        SYSTEMLINKGAME_STATE_JOINSESSION,
        SYSTEMLINKGAME_STATE_HOSTSESSION,
        SYSTEMLINKGAME_STATE_BACKOUT = STATEENGINE_STATE_EXIT,
    };

    switch( m_State )
    {
        case SYSTEMLINKGAME_STATE_BEGIN:
//$MOVED: now in HandleEnter**/HandleExit** states
//            // Init network stuff for enumerating sessions
//            InitNetwork();
//$END_MOVE

            // Initialise variables
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
            assert( GetSessionCount() == 0 ); // ClearSessionList(); //$NOTE: WAS ORIGINALLY "SessionCount = 0;"
            SessionPick        = 0;
            SessionRefreshFlag = 3;

//$REMOVED - server maintains player list and updates clients
//          if( GetSessionCount() != 0 )
//              ListPlayers(&SessionList[SessionPick].Guid); //$NOTE: 
//$END_REMOVAL

            // Set the System Link menu and camera position
            g_pMenuHeader->SetNextMenu( &Menu_SystemLink );
            g_pTitleScreenCamera->SetNewPos( TITLESCREEN_CAMPOS_MULTI );

            m_State = SYSTEMLINKGAME_STATE_MAINLOOP;
            break;

        case SYSTEMLINKGAME_STATE_BACKOUT:
//$MOVED: now in HandleEnter**/HandleExit** states
//            // Uninit network stuff
//            KillNetwork();
//$END_MOVE
    
            //$REVISIT: Should we uninit more/fewer vars here?  (Most probably not necessary.)
            GameSettings.GameType = GAMETYPE_NONE;
    
            Return( STATEENGINE_TERMINATED );
            break;

        case SYSTEMLINKGAME_STATE_MAINLOOP:
            // Nothing to do while control is in the menus
            break;

        case SYSTEMLINKGAME_STATE_JOINSESSION:
            // Nothing to do while control is in the menus
            break;

        case SYSTEMLINKGAME_STATE_HOSTSESSION:
            // Nothing to do while control is in the menus
            break;
    }

    // Handle the menus
    g_pMenuHeader->HandleMenus();

    return S_FALSE;
}




CCreateSystemLinkGameStateEngine g_CreateSystemLinkGameStateEngine;

//-----------------------------------------------------------------------------
// Name: Process()
// Desc: Main control function for the state engine
//-----------------------------------------------------------------------------
HRESULT CCreateSystemLinkGameStateEngine::Process()
{
    enum
    {
        CREATESYSTEMLINKGAME_STATE_BEGIN = STATEENGINE_STATE_BEGIN,
        CREATESYSTEMLINKGAME_STATE_SELECTRACEMODE,
        CREATESYSTEMLINKGAME_STATE_POSTSELECTRACEMODE,
        CREATESYSTEMLINKGAME_STATE_ENTERNAME,
        CREATESYSTEMLINKGAME_STATE_POSTENTERNAME,
        CREATESYSTEMLINKGAME_STATE_SELECTTRACK,
        CREATESYSTEMLINKGAME_STATE_POSTSELECTTRACK,
        CREATESYSTEMLINKGAME_STATE_SELECTCAR,
        CREATESYSTEMLINKGAME_STATE_POSTSELECTCAR,
        CREATESYSTEMLINKGAME_STATE_WAITINGROOM,
        CREATESYSTEMLINKGAME_STATE_POSTWAITINGROOM,
        CREATESYSTEMLINKGAME_STATE_BACKOUT = STATEENGINE_STATE_EXIT,
    };

    switch( m_State )
    {
        case CREATESYSTEMLINKGAME_STATE_BEGIN:
            // Set host
            GameSettings.MultiType = MULTITYPE_SERVER;
            LocalPlayerReady = FALSE;
            HostQuit         = FALSE;

            //$REVISIT: should these 2 lines go away eventually?
            /// (Should only have 1 copy of this name variable.  And it should have been filled with value from user's profile.)
            strncpy( gTitleScreenVars.PlayerData[0].nameEnter, RegistrySettings.PlayerName, MAX_PLAYER_NAME );
            gTitleScreenVars.PlayerData[0].nameEnter[MAX_PLAYER_NAME-1] = '\0';

            m_State = CREATESYSTEMLINKGAME_STATE_SELECTRACEMODE;
            break;

        case CREATESYSTEMLINKGAME_STATE_BACKOUT:
            // Un-set host
            GameSettings.MultiType = MULTITYPE_NONE;
    
            //$REVISIT: Should we uninit more/fewer vars here?  (Most probably not necessary.)

            Return( STATEENGINE_TERMINATED );
            break;
    
        case CREATESYSTEMLINKGAME_STATE_SELECTRACEMODE:
            // Call the SelectRaceMode state engine
            g_pActiveStateEngine->Call( &g_SelectRaceModeStateEngine );
            m_State = CREATESYSTEMLINKGAME_STATE_POSTSELECTRACEMODE;
            break;

        case CREATESYSTEMLINKGAME_STATE_POSTSELECTRACEMODE:
            // If user backed out, go back a few states
            if( g_SelectRaceModeStateEngine.GetStatus() == STATEENGINE_TERMINATED )
                return GotoState( CREATESYSTEMLINKGAME_STATE_BACKOUT );
            // Else, fall through to next state

        case CREATESYSTEMLINKGAME_STATE_SELECTTRACK:
            if( GameSettings.RandomTrack )
            {
                // Skip ahead past track selection
                m_State = CREATESYSTEMLINKGAME_STATE_ENTERNAME;
                break;
            }

            // Call the SelectTrack state engine
            g_pActiveStateEngine->Call( &g_SelectTrackStateEngine );
            m_State = CREATESYSTEMLINKGAME_STATE_POSTSELECTTRACK;
            break;

        case CREATESYSTEMLINKGAME_STATE_POSTSELECTTRACK:
            // If user backed out, go back a few states
            if( g_SelectTrackStateEngine.GetStatus() == STATEENGINE_TERMINATED )
                return GotoState( CREATESYSTEMLINKGAME_STATE_SELECTRACEMODE );
            // Else, fall through to next state

        case CREATESYSTEMLINKGAME_STATE_ENTERNAME:
            // Call the EnterName state engine
            g_pActiveStateEngine->Call( &g_EnterNameStateEngine );
            m_State = CREATESYSTEMLINKGAME_STATE_POSTENTERNAME;
            break;

        case CREATESYSTEMLINKGAME_STATE_POSTENTERNAME:
            // If user backed out, go back a few states
            if( g_EnterNameStateEngine.GetStatus() == STATEENGINE_TERMINATED )
            {
                if( GameSettings.RandomTrack )
                    return GotoState( CREATESYSTEMLINKGAME_STATE_SELECTRACEMODE );
                else
                    return GotoState( CREATESYSTEMLINKGAME_STATE_SELECTTRACK );
            }
            // Else, fall through to next state

        case CREATESYSTEMLINKGAME_STATE_SELECTCAR:
            if( GameSettings.RandomCars )
            {
                // Skip ahead past car selection
                m_State = CREATESYSTEMLINKGAME_STATE_WAITINGROOM;
                break;
            }

            // Call the SelectCar state engine
            g_pActiveStateEngine->Call( &g_SelectCarStateEngine );
            m_State = CREATESYSTEMLINKGAME_STATE_POSTSELECTCAR;
            break;

        case CREATESYSTEMLINKGAME_STATE_POSTSELECTCAR:
            // If user backed out, go back a few states
            if( g_SelectCarStateEngine.GetStatus() == STATEENGINE_TERMINATED )
                return GotoState( CREATESYSTEMLINKGAME_STATE_ENTERNAME );
            // Else, fall through to next state

        case CREATESYSTEMLINKGAME_STATE_WAITINGROOM:
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

            // Call the WaitingRoom state engine
            g_pActiveStateEngine->Call( &g_WaitingRoomStateEngine );
            m_State = CREATESYSTEMLINKGAME_STATE_POSTWAITINGROOM;
            break;

        case CREATESYSTEMLINKGAME_STATE_POSTWAITINGROOM:
            // If user backed out, go back a few states
            if( g_WaitingRoomStateEngine.GetStatus() == STATEENGINE_TERMINATED )
            {
                // Destroy the session
                DestroySession();

                if( GameSettings.RandomCars )
                    return GotoState( CREATESYSTEMLINKGAME_STATE_ENTERNAME );
                else
                    return GotoState( CREATESYSTEMLINKGAME_STATE_SELECTCAR );
            }

            // ??? 
            break;
    }

    // Handle the menus
    g_pMenuHeader->HandleMenus();

    return S_FALSE;
}




CJoinSystemLinkGameStateEngine g_JoinSystemLinkGameStateEngine;

//-----------------------------------------------------------------------------
// Name: HandleEnter** / HandleExit**  functions
// Desc: Init/uninit work necessary when entering/exiting state engine.
//-----------------------------------------------------------------------------
VOID CJoinSystemLinkGameStateEngine::HandleEnterFromParent()  { DisableSessionListUpdate = true; }
VOID CJoinSystemLinkGameStateEngine::HandleExitToParent()     { DisableSessionListUpdate = false; }
VOID CJoinSystemLinkGameStateEngine::HandleEnterFromChild()  { DisableSessionListUpdate = true; }
VOID CJoinSystemLinkGameStateEngine::HandleExitToChild()     { DisableSessionListUpdate = false; }
//$REVISIT: handling the **Child() cases is kind of ugly, but it's necessary
/// because the UI code currently lets you proceed out the end of a state
/// engine without "unwinding the stack".  If the code returned out of all
/// state engines before exiting, we could eliminate the **Child() funcs.

//-----------------------------------------------------------------------------
// Name: Process()
// Desc: Main control function for the state engine
//-----------------------------------------------------------------------------
HRESULT CJoinSystemLinkGameStateEngine::Process()
{
    enum
    {
        JOINSYSTEMLINKGAME_STATE_BEGIN = STATEENGINE_STATE_BEGIN,
        JOINSYSTEMLINKGAME_STATE_ENTERNAME,
        JOINSYSTEMLINKGAME_STATE_POSTENTERNAME,
        JOINSYSTEMLINKGAME_STATE_SELECTCAR,
        JOINSYSTEMLINKGAME_STATE_POSTSELECTCAR,
        JOINSYSTEMLINKGAME_STATE_WAITINGROOM,
        JOINSYSTEMLINKGAME_STATE_POSTWAITINGROOM,
        JOINSYSTEMLINKGAME_STATE_BACKOUT = STATEENGINE_STATE_EXIT,
    };

    switch( m_State )
    {
        case JOINSYSTEMLINKGAME_STATE_BEGIN:
            // Set client
            GameSettings.MultiType = MULTITYPE_CLIENT;
            LocalPlayerReady = FALSE;
            HostQuit         = FALSE;

            //$REVISIT: should these 2 lines go away eventually?
            /// (Should only have 1 copy of this name variable.  And it should have been filled with value from user's profile.)
            strncpy( gTitleScreenVars.PlayerData[0].nameEnter, RegistrySettings.PlayerName, MAX_PLAYER_NAME );
            gTitleScreenVars.PlayerData[0].nameEnter[MAX_PLAYER_NAME-1] = '\0';

            m_State = JOINSYSTEMLINKGAME_STATE_ENTERNAME;
            break;

        case JOINSYSTEMLINKGAME_STATE_BACKOUT:
            // Un-set client
            GameSettings.MultiType = MULTITYPE_NONE;
    
            Return( STATEENGINE_TERMINATED );
            break;
    
        case JOINSYSTEMLINKGAME_STATE_ENTERNAME:
            // Call the EnterName state engine
            g_pActiveStateEngine->Call( &g_EnterNameStateEngine );
            m_State = JOINSYSTEMLINKGAME_STATE_POSTENTERNAME;
            break;

        case JOINSYSTEMLINKGAME_STATE_POSTENTERNAME:
            // If user backed out, go back a few states
            if( g_EnterNameStateEngine.GetStatus() == STATEENGINE_TERMINATED )
                return GotoState( JOINSYSTEMLINKGAME_STATE_BACKOUT );

            if( GameSettings.RandomCars )
            {
                // Skip ahead past car selection
                return GotoState( JOINSYSTEMLINKGAME_STATE_WAITINGROOM );
            }

            // Fall through to the car selection state

        case JOINSYSTEMLINKGAME_STATE_SELECTCAR:
            // Call the SelectCar state engine
            g_pActiveStateEngine->Call( &g_SelectCarStateEngine );
            m_State = JOINSYSTEMLINKGAME_STATE_POSTSELECTCAR;
            break;

        case JOINSYSTEMLINKGAME_STATE_POSTSELECTCAR:
            // If user backed out, go back a few states
            if( g_SelectCarStateEngine.GetStatus() == STATEENGINE_TERMINATED )
                return GotoState( JOINSYSTEMLINKGAME_STATE_ENTERNAME );

            // Fall through to the next state

        case JOINSYSTEMLINKGAME_STATE_WAITINGROOM:
            // Join the session
            if( !JoinSession( (char)SessionPick) )
            {
                DeleteSessionListEntry(SessionPick);
                SessionPick = 0;
                
                // Note: error message gets displayed by JoinSession
                m_State = JOINSYSTEMLINKGAME_STATE_BACKOUT;
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
            g_pActiveStateEngine->Call( &g_WaitingRoomStateEngine );
            m_State = JOINSYSTEMLINKGAME_STATE_POSTWAITINGROOM;
            break;

        case JOINSYSTEMLINKGAME_STATE_POSTWAITINGROOM:
            // If user backed out, go back a few states
            if( g_WaitingRoomStateEngine.GetStatus() == STATEENGINE_TERMINATED )
            {
                // If connection to server was lost, then we already called LeaveSession().
                // And we displayed a status message in WaitingRoom UI, so just back out.
                if( ! IsInGameSession() )
                {
                    return GotoState( JOINSYSTEMLINKGAME_STATE_BACKOUT );
                }
                else
                {
                    // Leave the session
                    LeaveSession();

                    if( GameSettings.RandomCars )
                        return GotoState( JOINSYSTEMLINKGAME_STATE_ENTERNAME );
                    else
                        return GotoState( JOINSYSTEMLINKGAME_STATE_SELECTCAR );
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





