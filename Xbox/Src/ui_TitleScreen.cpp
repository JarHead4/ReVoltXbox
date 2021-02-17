//-----------------------------------------------------------------------------
// File: ui_TitleScreen.cpp
//
// Desc: UI implementation
//
// Re-Volt (Generic) Copyright (c) Probe Entertainment 1998
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"
#include "draw.h"
#include "main.h"
#include "text.h"
#include "input.h"
#include "network.h"
#include "net_xonline.h"
#include "model.h"
#include "world.h"
#include "texture.h"
#include "geom.h"
#include "particle.h"
#include "drawobj.h"
#include "visibox.h"
#include "LevelLoad.h"
#include "ctrlread.h"
#include "object.h"
#include "player.h"
#include "settings.h"
#include "light.h"
#include "spark.h"
#include "timing.h"
#include "competition.h"
#include "InitPlay.h"
#include "obj_init.h"
#include "credits.h"
#include "move.h"
#include "gameloop.h"
#include "SoundEffectEngine.h"
#include "MusicManager.h"
#include "VoiceManager.h"
#include "Cheats.h"
#include "XBResource.h"
#include "XBOnline.h"
#include "content.h" // for REVOLT_SUBSCRIPTION_ID

#include "ui_Menu.h"
#include "ui_MenuText.h"
#include "ui_MenuDraw.h"
#include "ui_podium.h"
#include "ui_StateEngine.h"
#include "ui_SplashScreen.h"
#include "ui_TitleScreen.h"
#include "ui_TopLevelMenu.h"
#include "ui_RaceOverview.h"
#include "ui_SelectRaceMode.h"
#include "ui_SelectLanguage.h"
#include "ui_PlayLive.h"
#include "ui_Friends.h"
#include "ui_players.h"
#include "ui_LiveSignOn.h"
#include "FriendsManager.h"



CStateEngine* g_pActiveStateEngine = NULL;

void InitFrontendMenuCamera();
void InitWinLoseCamera();
void UpdateFrontendMenuCamera();
void InitTitleScreen();
void SetCupDifficulty();
void SetDefaultDifficulty();
void InitMenuMessage(FLOAT timeOut);
void SetMenuMessage(char *message);
void SetBonusMenuMessage();

#define FE_DISPLAYTIMER         FALSE
#define TEST_WINLOSE_SEQUENCE   FALSE // TRUE

#define TITLESCREEN_TIMEOUT TO_TIME(60.0f) // #seconds to wait for input in title screen before going to demo mode
#define DEMO_TIMEOUT        TO_TIME(60.0f) // max #seconds to stay in demo mode before returning to title screen

// clockwork names
char *ClockworkNames[] = 
{
    "Zed",
    "Def Stef",
    "Doc C",
    "Gaztastic",
    "CT Baynes",
    "Fodge",
    "Eoin",
    "Roland",
    "Matt DC",
    "Tri Si",
    "Goombah",
    "Nutnut",
    "Bliz",
    "Rippance",
    "Shehad",
    "Jimlad",
    "Kev",
    "Big Mike",
    "New York Jez",
    "Traf",
    "J",
    "Eggy",
    "Big Rich",
    "Super Turk",
    "Pab",
    "Twan'Mobile",
    "Berb",
    "Gregm",
    "Fungus",
    "Liddon",
};


/////////////////////////////////////////////////////////////////////////////////
// Variables
/////////////////////////////////////////////////////////////////////////////////
TITLESCREENVARS     gTitleScreenVars;
BOOL                g_bTitleScreenRunGame     = FALSE;
BOOL                g_bTitleScreenRunDemo     = FALSE;
BOOL                g_bTitleScreenFadeStarted = FALSE;
BOOL                g_bShowWinLoseSequence    = FALSE;


CTitleScreenCamera* g_pTitleScreenCamera;




long                InitialMenuMessage = MENU_MESSAGE_NONE;

FLOAT               g_fTitleScreenTimer;

extern void InitNameWheelStuff();
extern void InitCarBoxStuff();

long                gCurrentScreenTPage = TPAGE_FX1;
int                 gTrackScreenLevelNum = 0;

bool                gTitleScreenFirstTime = TRUE;



struct TITLESCREEN_VIEWPORT
{
    BOOL bActive;

    // Dimensions
    FLOAT X, Y, Width, Height;

    // Menu stuff
    MENU_HEADER* pMenuHeader;

    // Camera
    CTitleScreenCamera* pCamera;

    // Active state engine
    CStateEngine* pActiveStateEngine;
};

TITLESCREEN_VIEWPORT g_Viewport[4];
BOOL g_bSplitScreen = FALSE;


VOID InitTitleScreenViewports()
{
    static MENU_HEADER MenuHeader[4];

    for( int i=0; i<4; i++ )
    {
        g_Viewport[i].bActive = TRUE;
        g_Viewport[i].X       = ( i==0 || i==2 ) ? 48.0f : 320.0f;
        g_Viewport[i].Y       = ( i==0 || i==1 ) ? 36.0f : 240.0f;
        g_Viewport[i].Width   = 320.0f-48;
        g_Viewport[i].Height  = 240.0f-36;

        g_Viewport[i].pMenuHeader = &MenuHeader[i];
        g_Viewport[i].pMenuHeader->InitMenuHeader();

        g_Viewport[i].pCamera = new CTitleScreenCamera();
        g_Viewport[i].pCamera->SetMoveTime( TS_CAMERA_MOVE_TIME );

        g_Viewport[i].pActiveStateEngine = NULL;
    }
}





//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
void StartInitTitleScreenVars()
{
    //$BUG: shouldn't ever write to "D:\..." on Xbox!
    FILE* file = fopen( "d:\\dump.txt", "w" );
    if( NULL != file )
    {
        for( int i=0;i<TEXT_NTEXTS;i++ )
            fprintf( file, "%04d: %s\n", i, gTitleScreen_Text[i] );
        fclose( file );
    }



    //$REVISIT: do we need to set these vars here?  Seems like they get init'd
    // again in GetRegistrySettings, which happens later.  (Note that this
    // function is only called once, when the game boots up.)
    gTitleScreenVars.GameType        = GAMETYPE_TRIAL;
    gTitleScreenVars.iLevelNum       = 0;                   // Level #
    gTitleScreenVars.numberOfLaps    = DEFAULT_RACE_LAPS;   // Number of race laps
    gTitleScreenVars.numberOfCars    = DEFAULT_RACE_CARS;   // Number of cars
    gTitleScreenVars.numberOfPlayers = 1;                   // Number of players
    gTitleScreenVars.bUseXOnline     = false;               // Use XOnline or SysLink?

    gTitleScreenVars.numberOfPrivateSlots = 0;
    gTitleScreenVars.numberOfPublicSlots  = gTitleScreenVars.numberOfCars;

    // Init data for the name wheel
    for( int i=0; i<4; i++ )
    {
        gTitleScreenVars.PlayerData[i].nameSelectPos      = NAMESELECT_END;        // Name selection position
        gTitleScreenVars.PlayerData[i].nameEnterPos       = 0;                     // Name enter position
        gTitleScreenVars.PlayerData[i].nameWheelDestAngle = 0.0f;
        gTitleScreenVars.PlayerData[i].nameWheelAngle     = 0;
        for( int iChar = 0; iChar < MAX_PLAYER_NAME; iChar++ ) 
            gTitleScreenVars.PlayerData[i].nameEnter[iChar] = '\0';
        gTitleScreenVars.PlayerData[i].iCarNum = 0;
    }
    gTitleScreenVars.iCurrentPlayer = 0;
    gTitleScreenVars.pCurrentPlayer = &gTitleScreenVars.PlayerData[gTitleScreenVars.iCurrentPlayer];


    gTitleScreenVars.mirror  = FALSE;
    gTitleScreenVars.reverse = FALSE;
    gTitleScreenVars.pickUps = TRUE;

    gTitleScreenVars.reflections = TRUE;
    gTitleScreenVars.shinyness   = TRUE;
    gTitleScreenVars.shadows     = TRUE;
    gTitleScreenVars.skidmarks   = TRUE;
    gTitleScreenVars.lights      = TRUE;
    gTitleScreenVars.LocalGhost  = TRUE;

    gTitleScreenVars.RandomCars  = FALSE;
    gTitleScreenVars.RandomTrack = FALSE;

    gTitleScreenVars.sparkLevel  = 2;
    gTitleScreenVars.smoke       = TRUE;

    gTitleScreenVars.bAllowDemoToRun = FALSE;

    gTitleScreenVars.rearview = FALSE;

    gTitleScreenVars.playMode = MODE_ARCADE;

    gTitleScreenVars.connectionType = 0;
    gTitleScreenVars.antialias      = FALSE;
    gTitleScreenVars.textureFilter  = 1;
    gTitleScreenVars.mipLevel       = 2;

    gTitleScreenVars.Language = LANGUAGE_ENGLISH;
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
static BOOL g_bTitleScreenFirstTime = TRUE;
CXBPackedResource g_xprFrontEnd;
D3DTexture*     g_pLiveSignInTexture;
D3DTexture*     g_pLiveSignInPlayerTexture;
D3DTexture*     g_pLiveSignInListBoxTexture;
D3DTexture*     g_pHeaderLayer2Texture;
D3DTexture*     g_pHeaderLayer1Texture;
D3DTexture*     g_pFooterLayer2Texture;
D3DTexture*     g_pFooterLayer1Texture;
D3DTexture*     g_pBoxTexture;
D3DTexture*     g_pTitleBoxTexture;

CXBPackedResource   g_xprOnlineIcons;
D3DTexture* g_pCommunicatorThruTVTexture;
D3DTexture* g_pFriendOnlineTexture;
D3DTexture* g_pFriendReqReceivedTexture;
D3DTexture* g_pFriendReqSentTexture;
D3DTexture* g_pGameInviteReceivedTexture;
D3DTexture* g_pGameInviteSentTexture;
D3DTexture* g_pTalkEnabledTexture;
D3DTexture* g_pTalkMutedTexture;
D3DTexture* g_pTalking0Texture;
D3DTexture* g_pTalking1Texture;
D3DTexture* g_pTalking2Texture;
D3DTexture* g_pTalking3Texture;

void GoTitleScreen(void)
{
    // The first time around, load fonts and resources.
    if( g_bTitleScreenFirstTime )
    {
        // Load the fonts;
        if( FAILED( LoadFonts() ) )
            return;

        // Load textures used for the frontend
        if( FAILED( g_xprFrontEnd.Create( "d:\\gfx\\FrontEnd.xpr", 9 ) ) )
            return;
        g_pLiveSignInTexture        = g_xprFrontEnd.GetTexture(   0UL );
        g_pLiveSignInPlayerTexture  = g_xprFrontEnd.GetTexture(  20UL );
        g_pLiveSignInListBoxTexture = g_xprFrontEnd.GetTexture(  40UL );
        g_pHeaderLayer2Texture      = g_xprFrontEnd.GetTexture(  60UL );
        g_pHeaderLayer1Texture      = g_xprFrontEnd.GetTexture(  80UL );
        g_pFooterLayer2Texture      = g_xprFrontEnd.GetTexture( 100UL );
        g_pFooterLayer1Texture      = g_xprFrontEnd.GetTexture( 120UL );
        g_pBoxTexture               = g_xprFrontEnd.GetTexture( 140UL );
        g_pTitleBoxTexture          = g_xprFrontEnd.GetTexture( 160UL );

        // Load the online notification icons
        if( FAILED( g_xprOnlineIcons.Create( "d:\\gfx\\OnlineIcons.xpr", 12 ) ) )
            return;
        g_pCommunicatorThruTVTexture            = g_xprOnlineIcons.GetTexture(   0UL );
        g_pFriendOnlineTexture                  = g_xprOnlineIcons.GetTexture(  20UL );
        g_pFriendReqReceivedTexture             = g_xprOnlineIcons.GetTexture(  40UL );
        g_pFriendReqSentTexture                 = g_xprOnlineIcons.GetTexture(  60UL );
        g_pGameInviteReceivedTexture            = g_xprOnlineIcons.GetTexture(  80UL );
        g_pGameInviteSentTexture                = g_xprOnlineIcons.GetTexture( 100UL );
        g_pTalkEnabledTexture                   = g_xprOnlineIcons.GetTexture( 120UL );
        g_pTalkMutedTexture                     = g_xprOnlineIcons.GetTexture( 140UL );
        g_pTalking0Texture                      = g_xprOnlineIcons.GetTexture( 160UL );
        g_pTalking1Texture                      = g_xprOnlineIcons.GetTexture( 180UL );
        g_pTalking2Texture                      = g_xprOnlineIcons.GetTexture( 200UL );
        g_pTalking3Texture                      = g_xprOnlineIcons.GetTexture( 220UL );

        g_bTitleScreenFirstTime = FALSE;
    }

    AI_Testing = FALSE;

    // no sepia
    RenderSettings.Sepia = FALSE;

    // init textures
    PickTextureFormat();
    InitTextures();
    InitFadeShit();
    SetupDxState();

    // pick texture sets
    PickTextureSets(MAX_RACE_CARS, TPAGE_WORLD_NUM, TPAGE_SCALE_NUM, TPAGE_FIXED_NUM);

    // load misc textures
    // $TODO: put these in a bundle
    LoadMipTexture("D:\\gfx\\font.bmp", TPAGE_FONT, 256, 256, 0, 1, FALSE);
    LoadMipTexture("D:\\gfx\\loading.bmp", TPAGE_LOADING, 256, 256, 0, 1, FALSE);
    LoadMipTexture("D:\\gfx\\spru.bmp", TPAGE_SPRU, 256, 256, 0, 1, FALSE);

    // set geom vars
    RenderSettings.GeomPers = BaseGeomPers;
    SetViewport(0, 0, (float)ScreenXsize, (float)ScreenYsize, RenderSettings.GeomPers);

    // Setup default player name
    gTitleScreenVars.iLevelNum = GetLevelNum(RegistrySettings.LevelDir);
    if (gTitleScreenVars.iLevelNum < 0) 
        gTitleScreenVars.iLevelNum = 0;

    // Full on effects on frontend
    RenderSettings.Env = TRUE;
    RenderSettings.Instance = TRUE;
    RenderSettings.Mirror = TRUE;
    RenderSettings.Shadow = TRUE;
    RenderSettings.Skid = FALSE;
    RenderSettings.Light = TRUE;

    InitTitleScreen();

#ifndef XBOX_DISABLE_NETWORK
    // lobby override?
    if (Lobby)
    {
        GameSettings.GameType = GAMETYPE_NETWORK_RACE;

        if (LobbyConnection->dwFlags == DPLCONNECTION_CREATESESSION)
        {
            GameSettings.MultiType = MULTITYPE_SERVER;
            g_pMenuHeader->SetNextMenu( &Menu_MultiType);

            //$TODO - is this correct?
            Menu_MultiType.ParentMenu = &Menu_ContinueLobby;
        }
        else
        {
            GameSettings.MultiType = MULTITYPE_CLIENT;
            g_pMenuHeader->SetNextMenu( &Menu_EnterName);

            //$TODO - is this correct?
            Menu_EnterName.ParentMenu = &Menu_ContinueLobby;
        }
    }
#endif // ! XBOX_DISABLE_NETWORK

    // init sfx / music vol
#ifdef OLD_AUDIO
    UpdateMusicVol(gTitleScreenVars.musicVolume);
    UpdateSfxVol(gTitleScreenVars.sfxVolume);
#else
    g_MusicManager.SetVolume( gTitleScreenVars.musicVolume );
    // TODO (JHarding): Need control over global sound effects volume
#endif // OLD_AUDIO

    // Clipping distances big
    SetNearFar(NEAR_CLIP_DIST, 75000.0f);

    // Fade in...
    SetFadeEffect(FADE_UP);

    SET_EVENT(TitleScreen);
}




//-----------------------------------------------------------------------------
// Name: RenderTitleScreenScene()
// Desc: 
//-----------------------------------------------------------------------------
VOID RenderTitleScreenScene()
{
    // Clear the viewport
    InitRenderStates();
    ClearBuffers();

    // Render opaque polys
    ResetSemiList();
    DrawWorld();
    DrawInstances();

    // Render
    DrawObjects();
    DrawAllCars();
    Draw3dPolyList();

    // Flush poly buckets
    FlushPolyBuckets();
    FlushEnvBuckets();

    // Render semi polys
    DrawSemiList();
    DrawSparks();
    DrawTrails();
    DrawSkidMarks();
    DrawAllCarShadows();

    // $BEGIN_ADDITION jedl - add frame rate display
#ifdef SHIPPING
    // Code below is disabled in shipping versions.
#else // !SHIPPING
    if (RegistrySettings.bGraphicsDebug)
    {
        WCHAR buf[100];
        FrameRate_NextFrame();
        BeginTextState();
        if (RegistrySettings.bUseGPU)
            DumpText(100, 340, 8, 16, 0xffffff, L"bUseGPU");
        swprintf(buf, L"Frame time %.3f ms", FrameRate_GetMsecPerFrame());
        DumpText(100, 360, 8, 16, 0xffffff, buf);
        swprintf(buf, L"Frame rate %2.1f fps", FrameRate_GetFPS());
        DumpText(100, 380, 8, 16, 0xffffff, buf);
    }
#endif // !SHIPPING
    // $END_ADDITION
}
    
    




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
void TitleScreen()
{
    int CameraCount = 0;

    // Input
    ReadJoystick();
    CheckCheatKeys();

    // maintain sounds
#ifdef OLD_AUDIO
    MaintainAllSfx();
#else
    g_SoundEngine.UpdateAll();
#endif // OLD_AUDIO
    g_VoiceManager.ProcessVoice();
    g_FriendsManager.Process();

    // buffer flip / clear
    FlipBuffers();

    // reset 3d poly list
    Reset3dPolyList();
    InitPolyBuckets();

    // Time
    FrameCount++;
    UpdateTimeStep();
    if( g_fTitleScreenTimer < 0.0f) g_fTitleScreenTimer = 0.0f;
    g_fTitleScreenTimer += TimeStep;

    // reset mesh fx list
    ResetMeshFxList();

    // perform AI
    AI_ProcessAllAIs();

    // move objects
    DefaultPhysicsLoop();

    // Build car world matrices
    BuildAllCarWorldMatrices();

    // Process lights
    ProcessLights();

    // Process sparks
    ProcessSparks();

//$ADDITION
    // Pump any pending XOnline tasks
    OnlineTasks_Continue();
//$END_ADDITION

    // Are we split screen?
    extern MENU Menu_EnterName;
    extern MENU Menu_CarSelect;
    extern MENU Menu_CarSelected;
    g_bSplitScreen = FALSE;

/*
    if( g_pMenuHeader->m_pMenu == &Menu_CarSelect || g_pMenuHeader->m_pMenu == &Menu_CarSelected )
        g_bSplitScreen = TRUE;
*/

    if( g_bShowWinLoseSequence ) 
    {
        // Update camera + set camera view vars
        UpdateFrontendMenuCamera();

        // Process Win/Lose sequence
        ProcessWinLoseSequence();
    }
    else
    {
        if( FALSE == g_bSplitScreen )
        {
/*
            // Update camera + set camera view vars
            UpdateFrontendMenuCamera();
*/

            gTitleScreenVars.iCurrentPlayer = 0;
            gTitleScreenVars.pCurrentPlayer = &gTitleScreenVars.PlayerData[0];
            SetViewport( 0, 0, 640, 480, RenderSettings.GeomPers );

            // Render the background scene
            RenderTitleScreenScene();

/*
            // Process and draw menus
            if( g_pMenuHeader )
                g_pMenuHeader->HandleMenus();
*/
            // Pass control to the active state engine
            if( g_pActiveStateEngine )
                g_pActiveStateEngine->Process();

#ifdef DEBUGSTATEENGINES
            // STATE ENGINE DEBUGGING STUFF
            DrawNewSpruBoxWithTabs( 40, 436-130, 220, 130 );
            g_pFont->DrawText( 48, 444-130, MENU_TEXT_RGB_HILITE, L"StateEngine Debug Stack" );

            CStateEngine* pStateEngine = g_pActiveStateEngine;
            FLOAT sx = 48.0f;
            FLOAT sy = 464.0f-130;
            while( pStateEngine )
            {
                g_pFont->DrawText( sx, sy, MENU_TEXT_RGB_NORMAL, pStateEngine->DebugGetName() );
                pStateEngine = pStateEngine->GetParent();
                sy += 20;
            }
#endif
        }
        else
        {
            BaseGeomPers = 256;

            for( int i=0; i<4; i++ )
            {
BaseGeomPers = 256;
                // Mirror global variables
                g_pTitleScreenCamera = g_Viewport[i].pCamera;
//              g_pMenuHeader        = g_Viewport[i].pMenuHeader;
//              g_pActiveStateEngine = g_Viewport[i].pActiveStateEngine;

                // Update camera + set camera view vars
                UpdateFrontendMenuCamera();

                gTitleScreenVars.iCurrentPlayer = i;
                gTitleScreenVars.pCurrentPlayer = &gTitleScreenVars.PlayerData[gTitleScreenVars.iCurrentPlayer];
                SetViewport( g_Viewport[i].X, g_Viewport[i].Y, g_Viewport[i].Width, g_Viewport[i].Height, 
                             RenderSettings.GeomPers );

                // Render the background scene
                RenderTitleScreenScene();

SetViewport( 0, 0, 640, 480, RenderSettings.GeomPers );
BaseGeomPers = 512;
                // Process and draw menus
                if( g_pMenuHeader )
                    g_pMenuHeader->HandleMenus();

                // Pass control to the active state engine
                if( g_pActiveStateEngine )
                    g_pActiveStateEngine->Process();

                // Restore global variables;
//              g_Viewport[i].pMenuHeader        = g_pMenuHeader;
//              g_Viewport[i].pActiveStateEngine = g_pActiveStateEngine;
            }

            BaseGeomPers = 512;
        }

        // Time to run demo?
        if( gTitleScreenVars.bAllowDemoToRun  &&  g_fTitleScreenTimer > TITLESCREEN_TIMEOUT )
        {
            DemoTimeout = DEMO_TIMEOUT;
            g_bTitleScreenRunDemo = TRUE;
        }
    }

    if( !g_bShowWinLoseSequence ) 
    {
        // After a certain amount of dead time, run the demo
        if( gTitleScreenVars.bAllowDemoToRun  &&  g_fTitleScreenTimer > TITLESCREEN_TIMEOUT )
        {
            DemoTimeout = DEMO_TIMEOUT;
            g_bTitleScreenRunDemo = TRUE;
        }
    }

// Debug Stuff
#if FE_DISPLAYTIMER
    char string[256];
    sprintf(string, "%.3f", TimeStep);

    BeginTextState();
    DumpText(8,8, 12,16, 0xc0c0c0, string);
#endif

    // Handle incoming and outgoing messages
    GetRemoteMessages();
    // Check to make sure that:
    // 1) If we're hosting and in the waiting room, we should listen for
    // client connections.
    // 2) If we're a client, and have been notified to start the game,
    // we should start the freakin' game.
    // We're ok to do this because
    // 1) We won't be IsInWaitingRoom unless we're in a game session
    // that hasn't started yet.
    // 2) The game can't be started unless we're actually in a session.
    if( IsMultiPlayer() )
    {
        if( IsServer() && IsInWaitingRoom() )
            LookForClientConnections();

        if( bGameStarted )
        {
            //SetRaceData(menuHeader, menu, menuItem);
            g_bTitleScreenRunGame = TRUE;

            // Until we clean up state engines, there's some
            // stuff we have clean up right here:
            g_pMenuHeader->SetNextMenu( NULL );
            g_FriendsManager.StopUpdatingFriends( 0 );
        }
    }
    TransmitMessageQueue();

    // fade shit
    DrawFadeShit();

    if( IsLoggedIn(0) )
    {
        // BUGBUG: Should make this flash for a couple seconds, then disappear
        //$HACK: We're always telling the online APIs the user signed in on controller 0.
        if( XOnlineGetNotification( 0, XONLINE_NOTIFICATION_GAME_INVITE ) )
        {
            DrawScreenSpaceQuad( 500, 390, g_pGameInviteReceivedTexture );
        }
        if( XOnlineGetNotification( 0, XONLINE_NOTIFICATION_FRIEND_REQUEST ) )
        {
            DrawScreenSpaceQuad( 540, 390, g_pFriendReqReceivedTexture );
        }
    }

    // Run Game or Demo?
    if( g_bTitleScreenRunGame || g_bTitleScreenRunDemo )
    {
        if( FALSE == g_bTitleScreenFadeStarted ) 
        {
            SetFadeEffect(FADE_DOWN);
            g_bTitleScreenFadeStarted = TRUE;
            if (GameSettings.GameType != GAMETYPE_CHAMPIONSHIP) 
            {
                g_pTitleScreenCamera->SetNewPos( TITLESCREEN_CAMPOS_INTO_TRACKSCREEN );
                g_pTitleScreenCamera->SetMoveTime( 0.25f );
            }
#ifdef OLD_AUDIO
            SetRedbookFade(REDBOOK_FADE_DOWN);
#endif // OLD_AUDIO
        }
        if (GetFadeEffect() == FADE_DOWN_DONE) 
        {
            ReleaseTitleScreen();
            if (g_bTitleScreenRunDemo) 
            {
                SetDemoData();
            }
            else 
            {
                if( IsMultiPlayer() )  { ClearSessionList(); }  //$ADDITION(cprince): clear the session list (if any) when we actually start the race (and thus can't go back to view session list)
                                                                //$NOTE: if we don't want to clear this unless session list exists, could check for (SessionCount > 0), either here or inside ClearSessionList.
                if (GameSettings.GameType == GAMETYPE_CALCSTATS) 
                {
                    SetCalcStatsData();
                } 
                else 
                {
                    SetRaceData();
                }
            }
            SET_EVENT(SetupGame);
        }
    }
}




//-----------------------------------------------------------------------------
// Name: LoadFrontEndTextures()
// Desc: 
//-----------------------------------------------------------------------------
void LoadFrontEndTextures()
{
    // $TODO - jedl use resource loading here, instead
    // Revolt logo
    LoadMipTexture("D:\\levels\\frontend\\carbox1.bmp", TPAGE_FX1, 256, 256, 0, 1, FALSE);
    LoadMipTexture("D:\\levels\\frontend\\carbox2.bmp", TPAGE_FX2, 256, 256, 0, 1, FALSE);
    LoadMipTexture("D:\\levels\\frontend\\carbox3.bmp", TPAGE_FX3, 256, 256, 0, 1, FALSE);
}




//-----------------------------------------------------------------------------
// Name: FreeFrontEndTextures()
// Desc: 
//-----------------------------------------------------------------------------
void FreeFrontEndTextures()
{
}



//-----------------------------------------------------------------------------
// Name: CheckForAcceptedInvites
// Desc: Checks for accepted cross-title invites.  If an invite is found, 
//          logs on all available players and attempts to join the session.
//$TODO: If the invite is old (TBD: how old is "old"?), we should prompt to
//          make sure they still want to attempt to join.
//$TODO: If one of the users who accepted the invite is no longer available,
//          we should prompt to see if they can insert the appropriate MU.
//-----------------------------------------------------------------------------
HRESULT CheckForAcceptedInvites()
{
    // We have to call XOnlineStartup in order to call 
    // XOnlineFriendsGetAcceptedGameInvite
    XBOnline_Startup();

    HRESULT                     hr;
    XONLINE_ACCEPTED_GAMEINVITE gameInvite;
    hr = g_FriendsManager.GetAcceptedGameInvite( &gameInvite );
    if( S_OK == hr )
    {
        XONLINE_FRIEND* pFriend = &gameInvite.InvitingFriend;
        DumpMessageVar( "Friends", "User %I64x accepted invite from %s", gameInvite.xuidAcceptedFriend, gameInvite.InvitingFriend.szGamertag );
        assert( XOnlineTitleIdIsSameTitle( pFriend->dwTitleID ) );

        // Get the list of available XONLINE_USERs
        XONLINE_USER usersAvailable[ XONLINE_MAX_STORED_ONLINE_USERS ];
        DWORD         dwNumUsers;
        XOnlineGetUsers( usersAvailable, &dwNumUsers );

        // Set up a list of XONLINE_USERs to log on
        XONLINE_USER usersLogon[ XONLINE_MAX_LOGON_USERS ] = {0};

        // Find all the users who accepted the invite.  It's conceivable
        // that one or more may not be available.  Right now, we'll do
        // the best we can.
        DWORD dwNumLogonUsers = 0;
        for( DWORD i = 0; i < XONLINE_MAX_LOGON_USERS; i++ )
        {
            // Ignore zero xuids - that means no one was there
            if( IsZeroXUID( gameInvite.xuidLogonUsers[i] ) )
                continue;

            //$TODO: Handle guests (including possibility that the player they
            // are a guest of is no longer there).
            // Make sure they're still available
            for( DWORD j = 0; j < dwNumUsers; j++ )
            {
                if( XOnlineAreUsersIdentical( &gameInvite.xuidLogonUsers[i], &usersAvailable[j].xuid ) )
                {
                    dwNumLogonUsers++;
                    memcpy( &usersLogon[i], &usersAvailable[j], sizeof( XONLINE_USER ) );
                    break;
                }
            }

            // If the user wasn't available, dump a message.  Eventually,
            // we should be more user-friendly here, and prompt them to
            // either insert the MU or accept that the user won't be
            // available
            if( j == dwNumUsers )
            {
                DumpMessageVar( "Friends", "User %I64X is no longer available to accept the invite", gameInvite.xuidLogonUsers[i] );
            }
        }

        // If we didn't end up with ANY of the accepting users,
        // then there's no one to log on.
        //$TODO: Should probably prompt here.
        if( dwNumLogonUsers == 0 )
        {
            DumpMessageVar( "Friends", "None of the original users are left to accept invite - bailing" );
            hr = E_FAIL;
        }
        else
        {
            // Try to log on the users who were previosly logged on
            HRESULT hr = XOnlineLogon( usersLogon, 
                                       g_pXOnlineServices, 
                                       NUM_XONLINE_SERVICES, 
                                       NULL, 
                                       &g_hSignInTask );
            if( SUCCEEDED( hr ) )
            {
                HRESULT hrSignOn;

                // Synchronously pump the logon task until we get a result
                //$REVISIT: Should we create a state engine for this?
                do
                {
                    hrSignOn = XOnlineTaskContinue( g_hSignInTask );
                    if( hrSignOn == XONLINETASK_S_RUNNING )
                    {
                        // La la la
                    }
                    else if( FAILED(hrSignOn) )
                    {
                        // Dude, this sucks
                        DumpMessageVar( "Friends", "Couldn't auto-log-on.  What do we do?" );
                        //$TODO: should correctly handle system logon error codes
                    }
                    else
                    {
                        // Mirror per-player logon results to our local copy
                        XONLINE_USER* g_XOnlineUserList;
                        DWORD         g_dwNumUsers;

                        // Get the online user list, which is used by the player screens
                        XBOnline_GetUserList( &g_XOnlineUserList, &g_dwNumUsers );

                        memcpy( g_XOnlineUserList, XOnlineGetLogonUsers(), XONLINE_MAX_LOGON_USERS * sizeof(XONLINE_USER) );

                        // Check for user errors
                        for( DWORD i = 0; i < XONLINE_MAX_LOGON_USERS; i++ )
                        {
                            if( g_XOnlineUserList[i].xuid.qwUserID != 0 ) // if valid user
                            {
                                //$REVISIT: Not sure if this is right - even if it is,
                                // it's still butt-ugly
                                Players[i].XOnlineInfo.pXOnlineUser = &g_XOnlineUserList[i];
                                if( SUCCEEDED( Players[i].XOnlineInfo.pXOnlineUser->hr ) )
                                    Players[i].XOnlineInfo.bIsLoggedIn = TRUE;
                                else
                                    hrSignOn = E_FAIL; // notify code below that logon failed
                                    //$TODO: should correctly handle user auth errors
                            }
                        }
                        // Check for service errors
                        for( DWORD i = 0; i < NUM_XONLINE_SERVICES; ++i )
                        {
                            HRESULT hr = XOnlineGetServiceInfo( g_pXOnlineServices[i], NULL );

                            if( g_pXOnlineServices[i] == REVOLT_SUBSCRIPTION_ID )
                            {
                                // special-case check for Re-Volt subscription service
                                if( SUCCEEDED(hr) )
                                    g_bHasSubscription = TRUE;
                            }
                            else
                            {
                                // common-case check for "standard" services
                                if( FAILED(hr) )
                                {
                                    hrSignOn = E_FAIL; // notify code below that logon failed
                                    //$TODO: should correctly handle service auth errors
                                }
                            }
                        }   
                    }
                } while( hrSignOn == XONLINETASK_S_RUNNING );

                // If sign-on failed, then we'll bail out
                if( SUCCEEDED( hrSignOn ) )
                {
                    //
                    // Some final steps in logon process ($REVISIT: blindly copied from ui_LiveSignOn.cpp)
                    //
                    CheckSignInConditions();

                    if( FAILED( g_FriendsManager.Initialize() ) )
                        DumpMessage( "Warning", "Couldn't initialize friends manager." );

                    //$HACK: We're always telling the online APIs the user signed in on controller 0.
                    AddOnlinePresenceFlag( 0, XONLINE_FRIENDSTATE_FLAG_ONLINE );

                    // Now that we're signed in, check the voice bit in the player's
                    // XUID and see if we can enable voice
                    assert( Players[0].XOnlineInfo.pXOnlineUser != NULL );
                    if( XOnlineIsUserVoiceAllowed( Players[0].XOnlineInfo.pXOnlineUser->xuid.dwUserFlags ) )
                        g_VoiceManager.EnableCommunicator( g_dwSignedInController, TRUE );


                    //
                    // Look up our friend's game session
                    //
                    XONLINETASK_HANDLE hFindSessionTask;
                    XOnlineMatchSessionFindFromID( pFriend->sessionID,
                                                   NULL,
                                                   &hFindSessionTask );
                    OnlineTasks_Add( hFindSessionTask, TASK_MATCH_SEARCH );

                    // Attempt to join the session
                    gTitleScreenVars.bUseXOnline = true;  // waiting room UI looks at this var when determining how to draw screen
                    g_pMenuHeader->SetNextMenu( NULL );
                    g_bInvitedByFriend = TRUE;
                    g_JoinMatchStateEngine.MakeActive();

                    // Return without calling XBOnline_Cleanup, because
                    // we're trying to join the session
                    return S_OK;
                }
            }
        }
    }

    // If we got here, then we didn't succesfully accept an invite,
    // so we should clean up online/networking
    XBOnline_Cleanup();
    return hr;
}



//-----------------------------------------------------------------------------
// Name: 
// Desc: Generic initialisation of menu and cars etc for the frontend.
//-----------------------------------------------------------------------------
void InitTitleScreen()
{
#if TEST_WINLOSE_SEQUENCE
    CupTable.CupType = RACE_CLASS_BRONZE;
    CupTable.LocalPlayerPos = 1;
    CupTable.WinLoseCarType[0] = rand() % (CARID_AMW + 1);//CARID_RC;
    CupTable.WinLoseCarType[1] = rand() % (CARID_AMW + 1);//CARID_DUSTMITE;
    CupTable.WinLoseCarType[2] = rand() % (CARID_AMW + 1);//CARID_HARVESTER;
    CupTable.WinLoseCarType[3] = rand() % (CARID_AMW + 1);//CARID_DOCGRUDGE;
    g_bShowWinLoseSequence = TRUE;
    InitialMenuMessage = MENU_MESSAGE_NEWCARS;
    //CupTable.CupType = RACE_CLASS_BRONZE;
    //InitialMenuMessage = MENU_MESSAGE_REVERSE;
    gTitleScreenFirstTime = FALSE;
#endif

    // Clear Credit active
    InitCreditStateInactive();

    // Set level availability
    // $MD: removed
    //InitDefaultLevels();

    // Set car availability
    //SetAllCarSelect();

    gTitleScreenVars.numberOfPlayers = 1;
    //gTitleScreenVars.numberOfCars = DEFAULT_RACE_CARS;

    // Setup / reset menu options
    if( FALSE == g_bShowWinLoseSequence &&
        FALSE == g_bInvitedByFriend )
    {
        // Activate the Top Level menu
        g_SplashScreenStateEngine.MakeActive( NULL );
    }

    // Setup Level StartData
    ts_SetupCar();

    // Gamesettings and type
    GameSettings.GameType = GAMETYPE_FRONTEND;
    GameSettings.Level = LEVEL_FRONTEND;

    GameSettings.DrawFollowView = FALSE;
    GameSettings.DrawRearView = FALSE;

    GameSettings.PlayMode = MODE_ARCADE;
    GameSettings.Mirrored = FALSE;
    GameSettings.Reversed = FALSE;
    // $MD: the car stored in gTitleScreenVars.PlayerData[0].iCarNum may have been deleted
    //      if it is corrupt
    GameSettings.CarType = gTitleScreenVars.PlayerData[0].iCarNum = RegistrySettings.CarType;
    GameSettings.RandomCars = FALSE;
    GameSettings.RandomTrack = FALSE;

    assert( !bGameStarted );  // should always get cleared in DestroySession/LeaveSession

    EnableLoadThread(STAGE_ONE_LOAD_COUNT + StartData.PlayerNum);

    // Load level data
    SetupLevelAndPlayers();

    if (!g_bShowWinLoseSequence) 
    {
        // Load extra textures for Car boxes (wait a bit if doing the win/lose as it needs FX tpages)
        LoadFrontEndTextures();
    }

    DisableLoadThread();

    // NameStand
    InitNameWheelStuff();

    // Create Camera
    g_pTitleScreenCamera = new CTitleScreenCamera();
    g_pTitleScreenCamera->SetMoveTime( TS_CAMERA_MOVE_TIME );

    // For now, don't do this until after the main camera is created
    InitTitleScreenViewports();

    if (!g_bShowWinLoseSequence) 
    {
        // init Camera for menu system
        InitFrontendMenuCamera();
    } 
    else 
    {
        // init camera for win/lose sequence
        InitWinLoseCamera();

        // setup win/lose sequence variables
        InitPodiumVars();
    }

    // Calculate car-box destinations
    InitCarBoxStuff();

    // Initialise state variables
    g_fTitleScreenTimer       = -1.0f;
    g_bTitleScreenRunGame     = FALSE;
    g_bTitleScreenRunDemo     = FALSE;
    g_bTitleScreenFadeStarted = FALSE;

    // Make sure track select screen always loads
    gTrackScreenLevelNum = gTitleScreenVars.iLevelNum;

    // Check for, and handle, any accepted game invites, but just
    // the first time through
    static BOOL bFirstTime = TRUE;
    if( bFirstTime )
    {
        bFirstTime = FALSE;
        CheckForAcceptedInvites();
    }
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
void ReleaseTitleScreen()
{
    FreeFrontEndTextures();

    LEV_EndLevelStageTwo();
    LEV_EndLevelStageOne();
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
void InitFrontendMenuCamera()
{
// Old stuff
    g_pTitleScreenCamera->SetInitalPos( TITLESCREEN_CAMPOS_INIT );

    if( InitialMenuMessage == MENU_MESSAGE_NONE ) 
        g_pTitleScreenCamera->SetNewPos( TITLESCREEN_CAMPOS_START );
    else 
        g_pTitleScreenCamera->SetNewPos( TITLESCREEN_CAMPOS_INIT );

    g_pTitleScreenCamera->m_fTimer = -0.5f;

// New stuff
    for( int i=0; i<4; i++ )
    {
        g_Viewport[i].pCamera->SetInitalPos( TITLESCREEN_CAMPOS_INIT );

        if( InitialMenuMessage == MENU_MESSAGE_NONE ) 
            g_Viewport[i].pCamera->SetNewPos( TITLESCREEN_CAMPOS_START );
        else 
            g_Viewport[i].pCamera->SetNewPos( TITLESCREEN_CAMPOS_INIT );

        g_Viewport[i].pCamera->m_fTimer = -0.5f;
    }
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
void InitWinLoseCamera()
{
    g_pTitleScreenCamera->SetInitalPos( TITLESCREEN_CAMPOS_PODIUMSTART );
    g_pTitleScreenCamera->SetNewPos( TITLESCREEN_CAMPOS_PODIUM );
    g_pTitleScreenCamera->m_fTimer = -0.5f;
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
void UpdateFrontendMenuCamera()
{
/*
    // Set camera destination position pointer
    if( !g_bShowWinLoseSequence )
    {
        if( g_pMenuHeader->m_pMenu != NULL )
        {
            if( g_pMenuHeader->m_pMenu->CamPosIndex != TITLESCREEN_CAMPOS_DONT_CHANGE )
            {
                g_pTitleScreenCamera->SetNewPos( g_pMenuHeader->m_pMenu->CamPosIndex );
            }
        }
    }
*/
    
    // Update the camera
    g_pTitleScreenCamera->Update();
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
void ts_SetupCar()
{
    InitStartData();

    if (!g_bShowWinLoseSequence) 
    {
        ////////////////////////////////////////////////////////////////
        // Little dodge'm things
        AddPlayerToStartData(PLAYER_FRONTEND, 0, CARID_KEY1, FALSE, 0, CTRL_TYPE_TS_TRAINING, 0, "Dinky1");
        AddPlayerToStartData(PLAYER_FRONTEND, 1, CARID_KEY2, FALSE, 0, CTRL_TYPE_TS_TRAINING, 0, "Dinky2");
        AddPlayerToStartData(PLAYER_FRONTEND, 2, CARID_KEY3, FALSE, 0, CTRL_TYPE_TS_TRAINING, 0, "Dinky3");
    } 
    else 
    {
        // Win sequence cars

        // Top three cars
        for( int iCar = 0; iCar < 3; iCar++ ) 
        {
            AddPlayerToStartData(PLAYER_DISPLAY, iCar, CupTable.WinLoseCarType[iCar], FALSE, 0, CTRL_TYPE_NONE, 0, "");
        }
       
        if (CupTable.LocalPlayerPos > 3) 
        {
            // Player's car if not in top three
            AddPlayerToStartData(PLAYER_DISPLAY, 3, CupTable.WinLoseCarType[3], FALSE, 0, CTRL_TYPE_NONE, 0, "");
        }
    }
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
// Setup data for Calculating the Car stats
void SetCalcStatsData()
{
    LEVELINFO *levelInfo;

    // Set the level settings
    GameSettings.GameType = GAMETYPE_CALCSTATS;
    GameSettings.Level = LEVEL_NEIGHBOURHOOD1;

    GameSettings.DrawRearView = FALSE;
    GameSettings.DrawFollowView = FALSE;

    GameSettings.PlayMode = MODE_ARCADE;
    GameSettings.Mirrored = FALSE;
    GameSettings.Reversed = FALSE;
    
    GameSettings.NumberOfLaps = -1;
    GameSettings.AllowPickups = FALSE;
    
    levelInfo = GetLevelInfo(GameSettings.Level);

    // Set the render settings
    RenderSettings.Env = RegistrySettings.EnvFlag = gTitleScreenVars.shinyness;
    RenderSettings.Light = RegistrySettings.LightFlag = gTitleScreenVars.lights;
    RenderSettings.Instance = RegistrySettings.InstanceFlag = gTitleScreenVars.instances;
    RenderSettings.Mirror = RegistrySettings.MirrorFlag = gTitleScreenVars.reflections;
    RenderSettings.Shadow = RegistrySettings.ShadowFlag = gTitleScreenVars.shadows;
    RenderSettings.Skid = RegistrySettings.SkidFlag = gTitleScreenVars.skidmarks;

    gSparkDensity = gTitleScreenVars.sparkLevel * HALF;

    // Init starting data
    InitStartData();
    strncpy(StartData.LevelDir, levelInfo->szDir, MAX_PATH);

    AddPlayerToStartData(PLAYER_CALCSTATS, 0, CARID_RC, 0, 0, CTRL_TYPE_NONE, 0, "");
}   




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
void SetDemoData()
{
    int iPlayer;
    LEVELINFO *levelInfo;

    // Set the level settings
    GameSettings.GameType = GAMETYPE_DEMO;
    if (GoStraightToDemo) 
    {
        // In rolling demo mode, play any level that was shipped
        do {
            GameSettings.Level = rand() % LEVEL_NCUP_LEVELS;
        } while (!DoesLevelExist(GameSettings.Level));
    } 
    else if (g_CreditVars.State != CREDIT_STATE_INACTIVE) 
    {
        // Show West2 in Credits
        GameSettings.Level = LEVEL_GHOSTTOWN2;
#ifdef OLD_AUDIO
        UpdateSfxVol(0);
#else
        // TODO (JHarding): Need global sfx volume control
#endif // OLD_AUDIO
    } 
    else
    {
        // In normal demo mode, play only levels that are accessible
        do 
        {
            GameSettings.Level = rand() % LEVEL_NCUP_LEVELS;
        } 
        while (!IsLevelTypeAvailable(GameSettings.Level, FALSE, FALSE));
    }

    GameSettings.DrawRearView = FALSE;
    GameSettings.DrawFollowView = FALSE;
    GameSettings.PlayMode = MODE_ARCADE;
    GameSettings.Mirrored = FALSE;
    GameSettings.Reversed = FALSE;
    
    GameSettings.NumberOfLaps = -1;
    GameSettings.AllowPickups = TRUE;
    
    levelInfo = GetLevelInfo(GameSettings.Level);

    // Set the render settings
    RenderSettings.Env = RegistrySettings.EnvFlag = gTitleScreenVars.shinyness;
    RenderSettings.Light = RegistrySettings.LightFlag = gTitleScreenVars.lights;
    RenderSettings.Instance = RegistrySettings.InstanceFlag = gTitleScreenVars.instances;
    RenderSettings.Mirror = RegistrySettings.MirrorFlag = gTitleScreenVars.reflections;
    RenderSettings.Shadow = RegistrySettings.ShadowFlag = gTitleScreenVars.shadows;
    RenderSettings.Skid = RegistrySettings.SkidFlag = gTitleScreenVars.skidmarks;
    gSparkDensity = gTitleScreenVars.sparkLevel * HALF;

    // Init starting data
    InitStartData();
    strncpy(StartData.LevelDir, levelInfo->szDir, MAX_PATH);

    for (iPlayer = 0; iPlayer < DEFAULT_RACE_CARS; iPlayer++) 
    {
        AddPlayerToStartData(PLAYER_CPU, iPlayer, iPlayer, 0, 0, CTRL_TYPE_CPU_AI, 0, "");
    }

    RandomizeStartingGrid();
}   




//-----------------------------------------------------------------------------
// Name: 
// Desc: Set Race Data from Titlescreen settings
//-----------------------------------------------------------------------------
void SetRaceData()
{
    int iPlayer;
    long i, j, k, car, gridused[MAX_RACE_CARS];
    LEVELINFO *levelInfo = GetLevelInfo(gTitleScreenVars.iLevelNum);

    // Set the render settings
    RenderSettings.Env = RegistrySettings.EnvFlag = gTitleScreenVars.shinyness;
    RenderSettings.Light = RegistrySettings.LightFlag = gTitleScreenVars.lights;
    RenderSettings.Instance = RegistrySettings.InstanceFlag = gTitleScreenVars.instances;
    RenderSettings.Mirror = RegistrySettings.MirrorFlag = gTitleScreenVars.reflections;
    RenderSettings.Shadow = RegistrySettings.ShadowFlag = gTitleScreenVars.shadows;
    RenderSettings.Skid = RegistrySettings.SkidFlag = gTitleScreenVars.skidmarks;
    gSparkDensity = gTitleScreenVars.sparkLevel * HALF;

    // Settings for all race modes
    GameSettings.PlayMode = gTitleScreenVars.playMode;
    GameSettings.DrawRearView = gTitleScreenVars.rearview;
    GameSettings.DrawFollowView = FALSE;
    SetDefaultDifficulty();

    switch (GameSettings.GameType)
    {
        // trial
        case GAMETYPE_TRIAL:

            // Init starting data
            InitStartData();
            StartData.GameType = GameSettings.GameType;
            GameSettings.NumberOfLaps = 0;
            
            // Setup the player
            AddPlayerToStartData(PLAYER_LOCAL, MAX_RACE_CARS - 1, gTitleScreenVars.PlayerData[0].iCarNum, 0, 0, CTRL_TYPE_LOCAL, 0, gTitleScreenVars.PlayerData[0].nameEnter);
            
            // Set the level
            levelInfo = GetLevelInfo(gTitleScreenVars.iLevelNum);
            strncpy(StartData.LevelDir, levelInfo->szDir, MAX_PATH);

            // Set the level settings
            GameSettings.Level = gTitleScreenVars.iLevelNum;
            GameSettings.Mirrored = gTitleScreenVars.mirror;
            GameSettings.Reversed = gTitleScreenVars.reverse;
            StartData.Laps = GameSettings.NumberOfLaps = gTitleScreenVars.numberOfLaps;
            StartData.AllowPickups = GameSettings.AllowPickups = gTitleScreenVars.pickUps;
            GameSettings.LocalGhost = gTitleScreenVars.LocalGhost;
            break;

        // single
        case GAMETYPE_SINGLE:

            // Init starting data
            InitStartData();
            StartData.GameType = GameSettings.GameType;
            StartData.Laps = GameSettings.NumberOfLaps;

#if TRUE
            if (GameSettings.RandomCars)
            {
                car = PickRandomCar();
            }

            if (GameSettings.GameType == GAMETYPE_SINGLE) {
                for (iPlayer = 0; iPlayer < gTitleScreenVars.numberOfPlayers; iPlayer++)
                {
                    if (!GameSettings.RandomCars) car = gTitleScreenVars.PlayerData[iPlayer].iCarNum;
                    AddPlayerToStartData(PLAYER_LOCAL, iPlayer, car, 0, 0, CTRL_TYPE_LOCAL, 0, gTitleScreenVars.PlayerData[iPlayer].nameEnter);
                }
            } else {
//$MODIFIED
//                AddPlayerToStartData(PLAYER_LOCAL, iPlayer, CARID_KEY4, 0, 0, CTRL_TYPE_LOCAL, 0, gTitleScreenVars.nameEnter[iPlayer]);
                iPlayer = 0;  //$NOTE(cprince): to avoid "var used before init" error.
                              //$NOTE(cprince): but probably doesn't matter; looks like we'll only execute this switch case when GameType is GAMETYPE_SINGLE.
                AddPlayerToStartData(PLAYER_LOCAL, iPlayer, CARID_KEY4, 0, 0, CTRL_TYPE_LOCAL, 0, gTitleScreenVars.PlayerData[iPlayer].nameEnter);
//$END_MODIFICATIONS
            }

            {
                if (GameSettings.GameType == GAMETYPE_SINGLE) {
                    for (iPlayer = gTitleScreenVars.numberOfPlayers; iPlayer < gTitleScreenVars.numberOfCars; iPlayer++)
                    {
                        AddPlayerToStartData(PLAYER_CPU, iPlayer, car, 0, 0, CTRL_TYPE_CPU_AI, 0, CarInfo[car].Name);
                    }

                    if (!GameSettings.RandomCars)
                    {   
                        RandomizeSingleRaceCars();
                    }
                } else {
                    for (iPlayer = 1; iPlayer < 4; iPlayer++) {
                        AddPlayerToStartData(PLAYER_CPU, iPlayer, CARID_KEY4, 0, 0, CTRL_TYPE_CPU_AI, 0, CarInfo[CARID_KEY4].Name);
                    }
                }
            }

            RandomizeStartingGrid();
#else       
            // JCC - for profiling and optimisation
            for (iPlayer = 0; iPlayer < gTitleScreenVars.numberOfPlayers; iPlayer++) {
                AddPlayerToStartData(PLAYER_LOCAL, iPlayer, 3, 0, 0, CTRL_TYPE_LOCAL, 0, gTitleScreenVars.nameEnter[iPlayer]);
            }

            for (iPlayer = gTitleScreenVars.numberOfPlayers; iPlayer < gTitleScreenVars.numberOfCars; iPlayer++) {
                AddPlayerToStartData(PLAYER_CPU, iPlayer, 3, 0, 0, CTRL_TYPE_CPU_AI, 0, "");
            }
#endif


            // Set the level settings
            if (GameSettings.RandomTrack)
                GameSettings.Level = PickRandomTrack();
            else
                GameSettings.Level = gTitleScreenVars.iLevelNum;

            GameSettings.Mirrored = gTitleScreenVars.mirror;
            GameSettings.Reversed = gTitleScreenVars.reverse;
            StartData.Laps = GameSettings.NumberOfLaps = gTitleScreenVars.numberOfLaps;
            StartData.AllowPickups = GameSettings.AllowPickups = gTitleScreenVars.pickUps;

            // Set the level
            levelInfo = GetLevelInfo(GameSettings.Level);
            strncpy(StartData.LevelDir, levelInfo->szDir, MAX_PATH);
            break;

        // clockwork
        case GAMETYPE_CLOCKWORK:
            // Init starting data
            InitStartData();
            StartData.GameType = GameSettings.GameType;
            StartData.Laps = GameSettings.NumberOfLaps;

            AddPlayerToStartData(PLAYER_LOCAL, 0, CARID_KEY4, 0, 0, CTRL_TYPE_LOCAL, 0, gTitleScreenVars.PlayerData[0].nameEnter);

            for (iPlayer = 1 ; iPlayer < MAX_NUM_PLAYERS ; iPlayer++) {
                AddPlayerToStartData(PLAYER_CPU, iPlayer, CARID_KEY4, 0, 0, CTRL_TYPE_CPU_AI, 0, ClockworkNames[(iPlayer - 1) % 30]);
            }

            RandomizeStartingGrid();

            // Set the level settings
            if (GameSettings.RandomTrack)
                GameSettings.Level = PickRandomTrack();
            else
                GameSettings.Level = gTitleScreenVars.iLevelNum;

            GameSettings.Mirrored = gTitleScreenVars.mirror;
            GameSettings.Reversed = gTitleScreenVars.reverse;
            StartData.Laps = GameSettings.NumberOfLaps = gTitleScreenVars.numberOfLaps;
            GameSettings.AllowPickups = gTitleScreenVars.pickUps;

            // Set the level
            levelInfo = GetLevelInfo(GameSettings.Level);
            strncpy(StartData.LevelDir, levelInfo->szDir, MAX_PATH);
            break;

        // multiplayer
        case GAMETYPE_NETWORK_RACE:
        case GAMETYPE_NETWORK_BATTLETAG:
            // force arcade mode
            GameSettings.PlayMode = MODE_ARCADE;

            // show sync message
            LoadMipTexture("D:\\gfx\\font.bmp", TPAGE_FONT, 256, 256, 0, 1, FALSE);

            for (i = 0 ; i < 3 ; i++)
            {
                ClearBuffers();
                InitRenderStates();
                BeginTextState();
                SET_TPAGE(TPAGE_FONT);
                g_pFont->SetScaleFactors( 1.5f, 1.5f );
                g_pFont->DrawText( 320, 240, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_STARTINGGAME), XBFONT_CENTER_X|XBFONT_CENTER_Y );
                g_pFont->SetScaleFactors( 1.0f, 1.0f );
                FlipBuffers();
            }

            FreeOneTexture(TPAGE_FONT);

            // force instances on
            RenderSettings.Instance = TRUE;

            // server
            if( IsServer() )
            {

                // Set the level settings
                if (GameSettings.RandomTrack)
                    GameSettings.Level = PickRandomTrack();
                else
                    GameSettings.Level = gTitleScreenVars.iLevelNum;

                GameSettings.Mirrored = gTitleScreenVars.mirror;
                GameSettings.Reversed = gTitleScreenVars.reverse;
                GameSettings.NumberOfLaps = gTitleScreenVars.numberOfLaps;
                GameSettings.AllowPickups = gTitleScreenVars.pickUps;

                // setup start data
                InitStartData();

                StartData.Mirrored     = GameSettings.Mirrored;
                StartData.Reversed     = GameSettings.Reversed;
                StartData.GameType     = GameSettings.GameType;
                StartData.Laps         = GameSettings.NumberOfLaps;
                StartData.AllowPickups = GameSettings.AllowPickups;

                StartData.Seed = rand();

                // set level name
                levelInfo = GetLevelInfo(GameSettings.Level);
                strncpy(StartData.LevelDir, levelInfo->szDir, MAX_PATH);

                // set session to 'started'
                SetGameStarted();
//$REMOVED - not required
//                SetSessionDesc(gTitleScreenVars.nameEnter[0], levelInfo->Dir, TRUE, GameSettings.GameType, GameSettings.RandomCars, GameSettings.RandomTrack);
//$END_REMOVAL

                // setup players
//$REMOVED - server maintains player list and updates clients
//                ListPlayers(NULL);
//$END_REMOVAL

                if (GameSettings.RandomCars)
//$MODIFIED
//                    carname = CarInfo[PickRandomCar()].Name;
                    car = PickRandomCar();
//$END_MODIFICATIONS

                for (i = 0 ; i < PlayerCount ; i++)
                    gridused[i] = FALSE;

                for (i = 0 ; i < PlayerCount ; i++)
                {
                    // randomly distribute the N players among the first N "grid" slots.
                    // (First player gets random slot 0..N, 2nd player gets random slot 0..N-1 <skipping used slots>, etc)
                    k = (rand() % (PlayerCount - i)) + 1;  // generates rand number between (1) and (NumPlayersLeftToBeProcessed), inclusive.
                    for (j = 0 ; j < PlayerCount ; j++)    // selects the 'k'th untaken RaceStartIndex
                    {
                        if (!gridused[j])
                            k--;
                        if (!k)
                            break;
                    }

                    gridused[j] = TRUE;

                    if (!GameSettings.RandomCars)
                    {
//$MODIFIED - we're using car type instead of name
//                        carname = PlayerList[i].Data.CarName;
                        car = PlayerList[i].CarType;
//$END_MODIFICATIONS
                    }

//$REMOVED_DONTCARE                    StartData.PlayerData[StartData.PlayerNum].Cheating = PlayerList[i].Data.Cheating;
//$MODIFIED - we're using car type instead of name
//                    strncpy(StartData.PlayerData[StartData.PlayerNum].CarName, carname, CAR_NAMELEN);

                    //$NOTE: Removed this because we're took out the CarName field from StartData.PlayerData[]
                    //strncpy(StartData.PlayerData[StartData.PlayerNum].CarName, CarInfo[car].Name, CAR_NAMELEN);
//$END_MODIFICATIONS


                    if (PlayerList[i].PlayerID == LocalPlayerID) 
                    {
//$MODIFIED
//                        AddPlayerToStartData(PLAYER_LOCAL, j, GetCarTypeFromName(carname), PlayerList[i].Spectator, TotalRacePhysicsTime, CTRL_TYPE_LOCAL, PlayerList[i].PlayerID, PlayerList[i].Name);
                        AddPlayerToStartData(PLAYER_LOCAL, j, car, PlayerList[i].Spectator, TotalRacePhysicsTime, CTRL_TYPE_LOCAL, PlayerList[i].PlayerID, PlayerList[i].Name);
//$END_MODIFICATIONS
                        StartData.LocalPlayerNum = i;
                    } 
                    else 
                    {
//$MODIFIED
//                        AddPlayerToStartData(PLAYER_REMOTE, j, GetCarTypeFromName(carname), PlayerList[i].Spectator, TotalRacePhysicsTime, CTRL_TYPE_REMOTE, PlayerList[i].PlayerID, PlayerList[i].Name);
                        AddPlayerToStartData(PLAYER_REMOTE, j, car, PlayerList[i].Spectator, TotalRacePhysicsTime, CTRL_TYPE_REMOTE, PlayerList[i].PlayerID, PlayerList[i].Name);
//$END_MODIFICATIONS
                    }
                }

                // send game started
                SendGameStarted();

                // sync
                RemoteSyncHost();
            }

            // client
            else
            {
                // get start data
                StartData = MultiStartData;

                // set level settings
                GameSettings.NumberOfLaps = StartData.Laps;
                GameSettings.Mirrored = StartData.Mirrored;
                GameSettings.Reversed = StartData.Reversed;
                GameSettings.GameType = StartData.GameType;
                GameSettings.Level = GetLevelNum(StartData.LevelDir);
                GameSettings.AllowPickups = StartData.AllowPickups;

                // setup player info
                for (i = 0; i < StartData.PlayerNum; i++) 
                {
                    if (StartData.PlayerData[i].PlayerID == LocalPlayerID) 
                    {
                        StartData.PlayerData[i].PlayerType = PLAYER_LOCAL;
                        StartData.PlayerData[i].CtrlType = CTRL_TYPE_LOCAL;
                        StartData.LocalPlayerNum = i;
                    } 
                    else
                    {
                        StartData.PlayerData[i].PlayerType = PLAYER_REMOTE;
                        StartData.PlayerData[i].CtrlType = CTRL_TYPE_REMOTE;
                    }

//$MODIFIED
//                    StartData.PlayerData[i].CarType = GetCarTypeFromName(StartData.PlayerData[i].CarName);
                    StartData.PlayerData[i].CarType = StartData.PlayerData[i].CarType;  //$REVISIT: this is a no-op?! (left side == right side).  So maybe StartData.PlayerData[i].CarName isn't getting set??
//$END_MODIFICATIONS
                }

                // sync
                RemoteSyncClient();
            }
            break;

        case GAMETYPE_REPLAY:
            FILE *fp;

            fp = fopen("Replay.rpl", "rb");
            if (fp == NULL) 
            {
                SetMenuMessage(TEXT_TABLE(TEXT_COULD_NOT_OPEN_REPLAY_FILE));
            } 
            else 
            {
                if (LoadReplayData(fp)) 
                {
                    GameSettings.GameType = GAMETYPE_REPLAY;
                    g_bTitleScreenRunGame = TRUE;
                } 
                else 
                {
                    SetMenuMessage(TEXT_TABLE(TEXT_COULD_NOT_READ_REPLAY_FILE));
                }
                fclose(fp);
            }

            // Set the level settings
            GameSettings.GameType = GAMETYPE_REPLAY;//StartData.GameType;
            GameSettings.Level = GetLevelNum(StartData.LevelDir);
            GameSettings.Mirrored = StartData.Mirrored;
            GameSettings.Reversed = StartData.Reversed;
            GameSettings.NumberOfLaps = StartData.Laps;
            GameSettings.AllowPickups = StartData.AllowPickups;
            
            break;

        // champion chip! 
        case GAMETYPE_CHAMPIONSHIP:

            // Set difficulty
            SetCupDifficulty();

            // setup cup table
            InitCupTable();

            // init 1st level
            InitOneCupLevel();
            break;

        // practice 
        case GAMETYPE_PRACTICE:
            InitStartData();
            StartData.GameType = GameSettings.GameType;

            AddPlayerToStartData(PLAYER_LOCAL, 0, gTitleScreenVars.PlayerData[0].iCarNum, 0, 0, CTRL_TYPE_LOCAL, 0, gTitleScreenVars.PlayerData[0].nameEnter);

            // Set the level settings
            GameSettings.Level = gTitleScreenVars.iLevelNum;
            GameSettings.Mirrored = gTitleScreenVars.mirror;
            GameSettings.Reversed = gTitleScreenVars.reverse;
            StartData.Laps = GameSettings.NumberOfLaps = gTitleScreenVars.numberOfLaps;
            GameSettings.AllowPickups = FALSE;
            // Set the level
            levelInfo = GetLevelInfo(gTitleScreenVars.iLevelNum);
            strncpy(StartData.LevelDir, levelInfo->szDir, MAX_PATH);
            break;

        // training
        case GAMETYPE_TRAINING:
            InitStartData();
            StartData.GameType = GameSettings.GameType;

            AddPlayerToStartData(PLAYER_LOCAL, 0, gTitleScreenVars.PlayerData[0].iCarNum, 0, 0, CTRL_TYPE_LOCAL, 0, gTitleScreenVars.PlayerData[0].nameEnter);

            // Set the level settings
            GameSettings.Level = gTitleScreenVars.iLevelNum;
            GameSettings.Mirrored = FALSE;
            GameSettings.Reversed = FALSE;
            StartData.Laps = GameSettings.NumberOfLaps = gTitleScreenVars.numberOfLaps;
            GameSettings.AllowPickups = FALSE;
            GameSettings.PlayMode = MODE_ARCADE;

            // Set the level
            levelInfo = GetLevelInfo(gTitleScreenVars.iLevelNum);
            strncpy(StartData.LevelDir, levelInfo->szDir, MAX_PATH);
            break;

        // not working yet
        case GAMETYPE_NONE:
        case GAMETYPE_FRONTEND:
        case GAMETYPE_INTRO:
        default:
            return;
    }
}




//-----------------------------------------------------------------------------
// Name: ClockHandler()
// Desc: Best Times Clock
//-----------------------------------------------------------------------------
void ClockHandler(OBJECT *obj)
{
    OBJECT_CLOCK_OBJ *clock = (OBJECT_CLOCK_OBJ*)obj->Data;

    clock->LargeHandAngle += TimeStep;
    clock->SmallHandAngle += TimeStep / 60;
    clock->DiscAngle -= TimeStep / 6;

}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
void SetCupDifficulty()
{
    switch( gTitleScreenVars.CupType )
    {
        default:
        case RACE_CLASS_BRONZE:
            CAI_InitCatchUp(DIFFICULTY_EASY);
            break;
        case RACE_CLASS_SILVER:
        case RACE_CLASS_GOLD:
            CAI_InitCatchUp(DIFFICULTY_MEDIUM);
            break;
        case RACE_CLASS_SPECIAL:
            CAI_InitCatchUp(DIFFICULTY_HARD);
            break;
    }
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
void SetDefaultDifficulty()
{
    CAI_InitCatchUp(DIFFICULTY_VERY_EASY);
}




//-----------------------------------------------------------------------------
// Name: InitMenuMessage()
// Desc: Initialise the initial message to show at the start
//-----------------------------------------------------------------------------
int InitialMenuMessageCount = 0;
int InitialMenuMessageWidth = 0;

WCHAR  InitialMenuMessageString[256];
WCHAR* InitialMenuMessageLines[10];
FLOAT  InitialMenuMessageTimer = ZERO;
FLOAT  InitialMenuMessageMaxTime = ZERO;

void InitMenuMessage(FLOAT timeOut)
{
    int iChar, sWidth;
    WCHAR c;

    InitialMenuMessageTimer = ZERO;
    InitialMenuMessageMaxTime = timeOut;

    // Count the lines and get max width
    iChar = 0;
    sWidth = 0;
    InitialMenuMessageWidth = 0;
    InitialMenuMessageCount = 0;
    InitialMenuMessageLines[0] = &InitialMenuMessageString[0];

    while( (c = InitialMenuMessageString[iChar++]) != L'\0' ) 
    {
        sWidth++;

        // New line
        if( c == L'\n' ) 
        {
            // Count lines and store pointer to start of line
            InitialMenuMessageCount++;
            InitialMenuMessageString[iChar-1] = L'\0';
            InitialMenuMessageLines[InitialMenuMessageCount] = &InitialMenuMessageString[iChar];

            // Store max width
            if (sWidth > InitialMenuMessageWidth)
                InitialMenuMessageWidth = sWidth;

            sWidth = 0;
            continue;
        }
    }
}

void SetMenuMessage(WCHAR *message)
{
    wcsncpy(InitialMenuMessageString, message, 256);
}




//-----------------------------------------------------------------------------
// Name: SetBonusMenuMessage()
// Desc: Set startup message from championship etc
//-----------------------------------------------------------------------------
void SetBonusMenuMessage()
{
    // Build the message string
    switch( InitialMenuMessage )
    {
        case MENU_MESSAGE_NEWCARS:
            swprintf(InitialMenuMessageString, TEXT_TABLE(TEXT_BONUS_NEWCARS));
            break;

        case MENU_MESSAGE_REVERSE:
            swprintf(InitialMenuMessageString, TEXT_TABLE(TEXT_BONUS_REVERSE), TEXT_TABLE(TEXT_BRONZE_CUP + CupTable.CupType - 1));
            break;

        case MENU_MESSAGE_MIRROR:
            swprintf(InitialMenuMessageString, TEXT_TABLE(TEXT_BONUS_MIRROR), TEXT_TABLE(TEXT_BRONZE_CUP + CupTable.CupType - 1));
            break;

        case MENU_MESSAGE_REVMIR:
            swprintf(InitialMenuMessageString, TEXT_TABLE(TEXT_BONUS_REVMIR), TEXT_TABLE(TEXT_BRONZE_CUP + CupTable.CupType - 1));
            break;

        case MENU_MESSAGE_COCKWORK:
            swprintf(InitialMenuMessageString, TEXT_TABLE(TEXT_BONUS_COCKWORK));
            break;
    }
}




