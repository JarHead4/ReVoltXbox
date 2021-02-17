//-----------------------------------------------------------------------------
// File: TitleScreen.cpp
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "Revolt.h"
#include "main.h"
#include "network.h"
#include "drawobj.h"
#include "LevelLoad.h"
#include "object.h"
#include "player.h"
#include "gameloop.h"
#include "settings.h"
#include "spark.h"
#include "Light.h"
#include "timing.h"
#include "competition.h"
#include "settings.h"

#include "TitleScreen.h"
#include "Menu2.h"
#include "MenuText.h"
#include "MenuDraw.h"
#include "InitPlay.h"
#include "obj_init.h"
#include "podium.h"
#include "credits.h"
#include "move.h"
#include "gameloop.h"
#include "SoundEffectEngine.h"
#include "SoundEffects.h"
#include "Cheats.h"
#include "ConnectionType.h"
#include "Text.h"
#include "XBFont.h"
#include "XBInput.h"

// menus
#include "MainMenu.h"
#include "Language.h"
#include "RaceSummary.h"
#include "SimpleMessage.h"
#include "StateEngine.h"

CStateEngine* g_pActiveStateEngine = NULL;


extern MENU Menu_SelectRace;

extern void SetCarBoxDestination();


static VOID InitWinLoseCamera();
static VOID UpdateFrontendMenuCamera();
static VOID InitTitleScreen();
static VOID RenderTitleScreenScene();


#define DEMO_TIMEOUT            60.0f
#define TEST_WINLOSE_SEQUENCE   FALSE
#define TITLESCREEN_TIMEOUT     60.0f

extern MENU Menu_TopLevel;

//-----------------------------------------------------------------------------
// Variables
//-----------------------------------------------------------------------------
TITLESCREEN_DATA    g_TitleScreenData;

DWORD               g_FrontEndState = 0;

bool                g_bTitleScreenRunGame = FALSE;
bool                g_bTitleScreenRunDemo = FALSE;
bool                g_bTitleScreenFadeStarted = FALSE;
bool                g_bShowWinLoseSequence = FALSE;

FLOAT               g_fTitleScreenTimer;

int                 g_iTrackScreenLevelNum = 0;

CFrontEndCamera*    g_pTitleScreenCamera;

CXBPackedResource   g_xprFrontEnd;
D3DTexture*         g_pLiveSignOnTexture        = NULL;
D3DTexture*         g_pLiveSignOnPlayerTexture  = NULL;
D3DTexture*         g_pLiveSignOnListBoxTexture = NULL;
D3DTexture* 		g_pHeaderLayer1Texture      = NULL;
D3DTexture* 		g_pHeaderLayer2Texture      = NULL;
D3DTexture* 		g_pFooterLayer1Texture      = NULL;
D3DTexture* 		g_pFooterLayer2Texture      = NULL;





//-----------------------------------------------------------------------------
// Local variables
//-----------------------------------------------------------------------------
static BOOL         g_bTitleScreenFirstTime = TRUE;
static OBJECT*      g_pNameWheel = NULL;




//-----------------------------------------------------------------------------
// Name: InitTitleScreenData()
// Desc: 
//-----------------------------------------------------------------------------
VOID InitTitleScreenData()
{
    g_TitleScreenData.gameType = GAMETYPE_TRIAL;
    g_TitleScreenData.iLevelNum = 0;                         // Level #
    g_TitleScreenData.numberOfLaps = DEFAULT_RACE_LAPS;      // Number of race laps
    g_TitleScreenData.numberOfCars = DEFAULT_RACE_CARS;      // Number of cars
    g_TitleScreenData.numberOfPlayers = 1;                   // Number of players

    g_TitleScreenData.sfxTest = 0;                           // SFX Test
    g_TitleScreenData.musicTest = 0;                         // Music Test

    g_TitleScreenData.nameEnterPos = 0;                      // Name enter position

    g_TitleScreenData.mirror = FALSE;
    g_TitleScreenData.reverse = FALSE;

    g_TitleScreenData.pickUps = TRUE;

    g_TitleScreenData.reflections = TRUE;
    g_TitleScreenData.shinyness = TRUE;
    g_TitleScreenData.shadows = TRUE;
    g_TitleScreenData.skidmarks = TRUE;
    g_TitleScreenData.lights = TRUE;
    g_TitleScreenData.LocalGhost = TRUE;

    g_TitleScreenData.RandomCars = FALSE;
    g_TitleScreenData.RandomTrack = FALSE;

    g_TitleScreenData.sparkLevel = 2;
    g_TitleScreenData.smoke = TRUE;

    g_TitleScreenData.bAllowDemoToRun = TRUE;

    g_TitleScreenData.rearview = FALSE;

    g_TitleScreenData.playMode = PLAYMODE_ARCADE;

    g_TitleScreenData.connectionType = 0;
    g_TitleScreenData.antialias = FALSE;
    g_TitleScreenData.textureFilter = 1;
    g_TitleScreenData.mipLevel = 2;

    // Init starting name and car choices
    for( int iPlayer = 0; iPlayer < MAX_LOCAL_PLAYERS; iPlayer++ ) 
	{
        for( int iChar = 0; iChar < MAX_INPUT_NAME+1; iChar++ ) 
            g_TitleScreenData.nameEnter[iPlayer][iChar] = '\0';
        g_TitleScreenData.iCarNum[iPlayer] = 0;
    }

    g_TitleScreenData.Language = ENGLISH;
}




//-----------------------------------------------------------------------------
// Name: GoTitleScreen()
// Desc: 
//-----------------------------------------------------------------------------
VOID GoTitleScreen()
{
	// The first time around, load fonts and resources.
	if( g_bTitleScreenFirstTime )
	{
		// Load the fonts;
		if( FAILED( LoadFonts() ) )
			return;

		// Load textures used for the frontend
		if( FAILED( g_xprFrontEnd.Create( "d:\\gfx\\FrontEnd.xpr", 7 ) ) )
			return;
		g_pLiveSignOnTexture        = g_xprFrontEnd.GetTexture(   0UL );
		g_pLiveSignOnPlayerTexture  = g_xprFrontEnd.GetTexture(  20UL );
		g_pLiveSignOnListBoxTexture = g_xprFrontEnd.GetTexture(  40UL );
		g_pHeaderLayer2Texture      = g_xprFrontEnd.GetTexture(  60UL );
		g_pHeaderLayer1Texture      = g_xprFrontEnd.GetTexture(  80UL );
		g_pFooterLayer2Texture      = g_xprFrontEnd.GetTexture( 100UL );
		g_pFooterLayer1Texture      = g_xprFrontEnd.GetTexture( 120UL );
	}

    // No sepia
    RenderSettings.Sepia = FALSE;

    // Onit textures
    PickTextureFormat();
    InitTextures();
    InitFadeShit();
    SetupDxState();

    // Pick texture sets
    PickTextureSets(MAX_RACE_CARS, TPAGE_WORLD_NUM, TPAGE_SCALE_NUM, TPAGE_FIXED_NUM);

    // Load misc textures
    LoadMipTexture("D:\\gfx\\font.bmp", TPAGE_FONT, 256, 256, 0, 1, FALSE);
    LoadMipTexture("D:\\gfx\\loading.bmp", TPAGE_LOADING, 256, 256, 0, 1, FALSE);
    LoadMipTexture("D:\\gfx\\spru.bmp", TPAGE_SPRU, 256, 256, 0, 1, FALSE);

    // Set geom vars
    RenderSettings.GeomPers = BaseGeomPers;
    SetViewport( 0, 0, (float)ScreenXsize, (float)ScreenYsize, RenderSettings.GeomPers );

    // Setup default player name
    g_TitleScreenData.iLevelNum = GetLevelNum( RegistrySettings.LevelDir );
    if( g_TitleScreenData.iLevelNum < 0 )
        g_TitleScreenData.iLevelNum = 0;

    // Full on effects on frontend
    RenderSettings.Env = TRUE;
    RenderSettings.Instance = TRUE;
    RenderSettings.Mirror = TRUE;
    RenderSettings.Shadow = TRUE;
    RenderSettings.Skid = FALSE;
    RenderSettings.Light = TRUE;

    InitTitleScreen();

#ifndef XBOX_DISABLE_NETWORK
    // Lobby override?
    if( Lobby )
    {
        GameSettings.GameType = GAMETYPE_MULTI;

        if( LobbyConnection->dwFlags == DPLCONNECTION_CREATESESSION )
        {
            GameSettings.MultiType = MULTITYPE_SERVER;
            SetNextMenu( &gMenuHeader, &Menu_MultiType );
            Menu_MultiType.ParentMenu = &Menu_ContinueLobby;
        }
        else
        {
            GameSettings.MultiType = MULTITYPE_CLIENT;
            SetNextMenu( &gMenuHeader, &Menu_PlayerName );
            Menu_PlayerName.ParentMenu = &Menu_ContinueLobby;
        }
    }
#endif // ! XBOX_DISABLE_NETWORK

    // Init sfx / music vol
    UpdateMusicVol( g_TitleScreenData.musicVolume );
    UpdateSfxVol( g_TitleScreenData.sfxVolume );

    // Clipping distances big
    SetNearFar( NEAR_CLIP_DIST, 75000.0f );

    // Fade in...
    SetFadeEffect( FADE_UP );

    SET_EVENT( TitleScreen );
}




//-----------------------------------------------------------------------------
// Name: TitleScreen()
// Desc: 
//-----------------------------------------------------------------------------
VOID TitleScreen()
{
	// Input
    XBInput_GetInput();

    // Check cheat keys for top two menus
    if( ( gMenuHeader.pMenu == &Menu_TopLevel ) || ( gMenuHeader.pMenu == &Menu_SelectRace ) )
	{
		LONG lCheatNum = CheckCheatKeys();

		if( lCheatNum == CHEAT_TABLE_VERSION_DEV )
		{
			static VERSION LastVersion = VERSION_DEV;

            VERSION temp = LastVersion;
            LastVersion  = Version;
            Version      = temp;

            InitDefaultLevels();
            SetAllCarSelect();
		}
    }

	// maintain sounds
    MaintainAllSfx();
	#pragma message( "jharding needs to look into porting this" )

	// buffer flip / clear
    FlipBuffers();

	// reset 3d poly list
    Reset3dPolyList();
    InitPolyBuckets();

	// Time
    FrameCount++;
    UpdateTimeStep();
    if (g_fTitleScreenTimer < ZERO) g_fTitleScreenTimer = ZERO;
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

	// update camera + set camera view vars
    UpdateFrontendMenuCamera();


	// Render the scene
	RenderTitleScreenScene();

	// Do Menus if not in Win/Lose sequence mode
    if( !g_bShowWinLoseSequence ) 
    {
		// Process and draw menu
		HandleMenus();

		// Pass control to the active state engine
		if( g_pActiveStateEngine )
			g_pActiveStateEngine->Process();

        // After a certain amount of time with no user input, run the demo
        if( g_TitleScreenData.bAllowDemoToRun && g_fTitleScreenTimer > TITLESCREEN_TIMEOUT ) 
		{
            DemoTimeout = DEMO_TIMEOUT;
            g_bTitleScreenRunDemo = TRUE;
        }
    }
    else 
    {
        // Process Win/Lose sequence
        ProcessWinLoseSequence();
    }

    // Update the fade effect
    DrawFadeShit();

    // Run Game or Demo?
    if( g_bTitleScreenRunGame || g_bTitleScreenRunDemo ) 
	{
        if( !g_bTitleScreenFadeStarted ) 
		{
            SetFadeEffect( FADE_DOWN );
            g_bTitleScreenFadeStarted = TRUE;
            if( GameSettings.GameType != GAMETYPE_CHAMPIONSHIP ) 
			{
                Menu_Overview.CamPosIndex = TITLESCREEN_CAMPOS_INTO_TRACKSCREEN;
                g_pTitleScreenCamera->SetMoveTime( 0.25f );
            }
            SetRedbookFade( REDBOOK_FADE_DOWN );
        }
        if( GetFadeEffect() == FADE_DOWN_DONE ) 
		{
            ReleaseTitleScreen();
            if( g_bTitleScreenRunDemo ) 
			{
                SetDemoData();
            } 
			else 
			{
                if( GameSettings.GameType == GAMETYPE_CALCSTATS ) 
				{
                    SetCalcStatsData();
                } 
				else 
				{
                    SetRaceData();
                }
            }
            SET_EVENT( SetupGame );
        }
    }
}




//-----------------------------------------------------------------------------
// Name: InitTitleScreen()
// Desc: Generic initialisation of menu and cars etc. for the frontend.
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
    /*CupTable.CupType = RACE_CLASS_BRONZE;
    InitialMenuMessage = MENU_MESSAGE_REVERSE;*/
    g_bTitleScreenFirstTime = FALSE;
#endif

    // Clear Credit active
    InitCreditStateInactive();

    // Set level availability
    InitDefaultLevels();

    // Set car availability
    SetAllCarSelect();

    g_TitleScreenData.numberOfPlayers = 1;
    //g_TitleScreenData.numberOfCars = DEFAULT_RACE_CARS;

    // Setup / reset menu options
    if( !g_bShowWinLoseSequence )
        SetTitleScreenInitialMenu();

    // Setup Level StartData
    SetupTitleScreenCar();

    // Gamesettings and type
    GameSettings.GameType = GAMETYPE_FRONTEND;
    GameSettings.Level = LEVEL_FRONTEND;

    GameSettings.DrawFollowView = FALSE;
    GameSettings.DrawRearView = FALSE;

    GameSettings.PlayMode = PLAYMODE_ARCADE;
    GameSettings.Mirrored = FALSE;
    GameSettings.Reversed = FALSE;
    GameSettings.CarType = g_TitleScreenData.iCarNum[0];
    GameSettings.RandomCars = FALSE;
    GameSettings.RandomTrack = FALSE;

    EnableLoadThread(STAGE_ONE_LOAD_COUNT + StartData.PlayerNum);

    // Load level data
    SetupLevelAndPlayers();

    if (!g_bShowWinLoseSequence) {
        // Load extra textures for Car boxes (wait a bit if doing the win/lose as it needs FX tpages)
        LoadFrontEndTextures();
    }

    DisableLoadThread();

    // NameStand
    g_pNameWheel = NextObjectOfType( OBJ_ObjectHead, OBJECT_TYPE_NAMEWHEEL );

    // Create Camera
    g_pTitleScreenCamera = new CFrontEndCamera();
    g_pTitleScreenCamera->SetMoveTime( TS_CAMERA_MOVE_TIME );

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
    SetCarBoxDestination();

    // Initialise state variables
    g_fTitleScreenTimer = -ONE;
    g_bTitleScreenRunGame = FALSE;
    g_bTitleScreenRunDemo = FALSE;
    g_bTitleScreenFadeStarted = FALSE;

    // Make sure track select screen always loads
    g_iTrackScreenLevelNum = g_TitleScreenData.iLevelNum;
}




//-----------------------------------------------------------------------------
// Name: SetTitleScreenInitialMenu()
// Desc: Setup the initial menu for the title screen
//-----------------------------------------------------------------------------
VOID SetTitleScreenInitialMenu()
{
    BOOL bSelectLanguage = UseLanguages;

    ClearMenuHeader( &gMenuHeader );

    if( InitialMenuMessage == MENU_MESSAGE_NONE )
	{
        if( g_bTitleScreenFirstTime && bSelectLanguage )
		{
            // First time entering titlescreen, do a load
            SetNextMenu( &gMenuHeader, &Menu_Language );
            g_bTitleScreenFirstTime = FALSE;
        }
        else
        {
            SetNextMenu( &gMenuHeader, &Menu_TopLevel );
        }

    } 
	else 
	{
        // Show Bonus Message for a while
        SetBonusMenuMessage();
        InitMenuMessage( 5.0f );
        SetNextMenu( &gMenuHeader, &Menu_InitialMessage );
    }
}




//-----------------------------------------------------------------------------
// Name: ReleaseTitleScreen()
// Desc: 
//-----------------------------------------------------------------------------
void ReleaseTitleScreen(void)
{
    FreeFrontEndTextures();
    if (Version != VERSION_DEV)

    LEV_EndLevelStageTwo();

    LEV_EndLevelStageOne();
}




//-----------------------------------------------------------------------------
// Name: InitFrontendMenuCamera()
// Desc: 
//-----------------------------------------------------------------------------
void InitFrontendMenuCamera()
{
    g_pTitleScreenCamera->SetInitalCameraPos( TITLESCREEN_CAMPOS_INIT );

    if( InitialMenuMessage == MENU_MESSAGE_NONE ) 
	    g_pTitleScreenCamera->SetNewCameraPos( TITLESCREEN_CAMPOS_START );
	else 
	    g_pTitleScreenCamera->SetNewCameraPos( TITLESCREEN_CAMPOS_INIT );

    g_pTitleScreenCamera->m_fTimer = -0.5f;
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
void InitWinLoseCamera()
{
    g_pTitleScreenCamera->SetInitalCameraPos( TITLESCREEN_CAMPOS_PODIUMSTART );
	g_pTitleScreenCamera->SetNewCameraPos( TITLESCREEN_CAMPOS_PODIUM );

    g_pTitleScreenCamera->m_fTimer = -0.5f;
}




//-----------------------------------------------------------------------------
// Name: UpdateFrontendMenuCamera()
// Desc: 
//-----------------------------------------------------------------------------
void UpdateFrontendMenuCamera()
{
    // Set camera destination position pointer
    if( !g_bShowWinLoseSequence ) 
	{
        if( gMenuHeader.pMenu != NULL ) 
		{
            if( gMenuHeader.pMenu->CamPosIndex != TITLESCREEN_CAMPOS_DONT_CHANGE )
			{
                g_pTitleScreenCamera->SetNewCameraPos( gMenuHeader.pMenu->CamPosIndex );
			}
        }
    }
    
    // Update the camera
    g_pTitleScreenCamera->UpdateCamera();
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
}
	
	


//-----------------------------------------------------------------------------
// Name: SetupTitleScreenCar()
// Desc: 
//-----------------------------------------------------------------------------
VOID SetupTitleScreenCar()
{
    InitStartData();

    if( !g_bShowWinLoseSequence )
	{
        // Little dodge'm things
        AddPlayerToStartData( PLAYER_FRONTEND, 0, CARID_KEY1, FALSE, 0, CTRL_TYPE_TS_TRAINING, 0, "Dinky1" );
        AddPlayerToStartData( PLAYER_FRONTEND, 1, CARID_KEY2, FALSE, 0, CTRL_TYPE_TS_TRAINING, 0, "Dinky2" );
        AddPlayerToStartData( PLAYER_FRONTEND, 2, CARID_KEY3, FALSE, 0, CTRL_TYPE_TS_TRAINING, 0, "Dinky3" );
    } 
	else 
	{
        // Win sequence cars

        // Top three cars
        for( int iCar = 0; iCar < 3; iCar++ ) 
            AddPlayerToStartData( PLAYER_DISPLAY, iCar, CupTable.WinLoseCarType[iCar], FALSE, 0, CTRL_TYPE_NONE, 0, "" );
       
        if( CupTable.LocalPlayerPos > 3 ) 
            // Player's car if not in top three
            AddPlayerToStartData( PLAYER_DISPLAY, 3, CupTable.WinLoseCarType[3], FALSE, 0, CTRL_TYPE_NONE, 0, "" );
    }
}




//-----------------------------------------------------------------------------
// Name: SetCalcStatsData()
// Desc: Setup data for Calculating the Car stats
//-----------------------------------------------------------------------------
void SetCalcStatsData()
{
    // Set the level settings
    GameSettings.GameType = GAMETYPE_CALCSTATS;
    GameSettings.Level    = LEVEL_NEIGHBOURHOOD1;
    GameSettings.DrawRearView   = FALSE;
    GameSettings.DrawFollowView = FALSE;
    GameSettings.PlayMode = PLAYMODE_ARCADE;
    GameSettings.Mirrored = FALSE;
    GameSettings.Reversed = FALSE;
    GameSettings.NumberOfLaps = -1;
    GameSettings.AllowPickups = FALSE;
    
    LEVELINFO* levelInfo = GetLevelInfo(GameSettings.Level);

    // Set the render settings
    RenderSettings.Env      = RegistrySettings.EnvFlag      = g_TitleScreenData.shinyness;
    RenderSettings.Light    = RegistrySettings.LightFlag    = g_TitleScreenData.lights;
    RenderSettings.Instance = RegistrySettings.InstanceFlag = g_TitleScreenData.instances;
    RenderSettings.Mirror   = RegistrySettings.MirrorFlag   = g_TitleScreenData.reflections;
    RenderSettings.Shadow   = RegistrySettings.ShadowFlag   = g_TitleScreenData.shadows;
    RenderSettings.Skid     = RegistrySettings.SkidFlag     = g_TitleScreenData.skidmarks;

    gSparkDensity = g_TitleScreenData.sparkLevel * HALF;

    // Init starting data
    InitStartData();
    strncpy(StartData.LevelDir, levelInfo->Dir, MAX_LEVEL_DIR_NAME);

    AddPlayerToStartData(PLAYER_CALCSTATS, 0, CARID_RC, 0, 0, CTRL_TYPE_NONE, 0, "");
}   




//-----------------------------------------------------------------------------
// Name: SetDemoData()
// Desc: Go Game Demo
//-----------------------------------------------------------------------------
void SetDemoData()
{
    int iPlayer;
    LEVELINFO *levelInfo;

    // Set the level settings
    GameSettings.GameType = GAMETYPE_DEMO;
    if (CreditVars.State != CREDIT_STATE_INACTIVE) 
    {
        // Show West2 in Credits
        GameSettings.Level = LEVEL_GHOSTTOWN2;
        UpdateSfxVol(0);
    } 
    else
    {
        // In normal demo mode, play only levels that are accessible
        do {
            GameSettings.Level = rand() % LEVEL_NCUP_LEVELS;
        } while (!IsLevelTypeAvailable(GameSettings.Level, FALSE, FALSE));
    }

    GameSettings.DrawRearView = FALSE;
    GameSettings.DrawFollowView = FALSE;

    GameSettings.PlayMode = PLAYMODE_ARCADE;
    GameSettings.Mirrored = FALSE;
    GameSettings.Reversed = FALSE;
    
    GameSettings.NumberOfLaps = -1;
    GameSettings.AllowPickups = TRUE;
    
    levelInfo = GetLevelInfo(GameSettings.Level);

    // Set the render settings
    RenderSettings.Env = RegistrySettings.EnvFlag = g_TitleScreenData.shinyness;
    RenderSettings.Light = RegistrySettings.LightFlag = g_TitleScreenData.lights;
    RenderSettings.Instance = RegistrySettings.InstanceFlag = g_TitleScreenData.instances;
    RenderSettings.Mirror = RegistrySettings.MirrorFlag = g_TitleScreenData.reflections;
    RenderSettings.Shadow = RegistrySettings.ShadowFlag = g_TitleScreenData.shadows;
    RenderSettings.Skid = RegistrySettings.SkidFlag = g_TitleScreenData.skidmarks;
    gSparkDensity = g_TitleScreenData.sparkLevel * HALF;

    // Init starting data
    InitStartData();
    strncpy(StartData.LevelDir, levelInfo->Dir, MAX_LEVEL_DIR_NAME);

    for (iPlayer = 0; iPlayer < DEFAULT_RACE_CARS; iPlayer++) {
        AddPlayerToStartData(PLAYER_CPU, iPlayer, iPlayer, 0, 0, CTRL_TYPE_CPU_AI, 0, "");
    }

    RandomizeStartingGrid();
}   




