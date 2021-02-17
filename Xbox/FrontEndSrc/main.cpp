//-----------------------------------------------------------------------------
// File: main.cpp
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include <process.h>

#include "revolt.h"
#include "main.h"
#include "dx.h"
#include "geom.h"
#include "model.h"
#include "texture.h"
#include "particle.h"
#include "aerial.h"
#include "network.h"
#include "NewColl.h"
#include "body.h"
#include "car.h"
#include "input.h"
#include "text.h"
#include "shadow.h"
#include "camera.h"
#include "light.h"
#include "world.h"
#include "draw.h"
#include "DrawObj.h"
#include "visibox.h"
#include "LevelLoad.h"
#include "readinit.h"
#include "gameloop.h"
#include "gaussian.h"
#include "timing.h"
#include "settings.h"
#include "ctrlread.h"
#include "object.h"
#include "control.h"
#include "player.h"
#include "ghost.h"
#include "replay.h"
#include "NewColl.h"
#include "TitleScreen.h"
#include "competition.h"
#include "InitPlay.h"
#include "Intro.h"
#include "Menu2.h"
#include "MenuDraw.h"
#include "obj_init.h"
#include "shareware.h"
#include "panel.h"
#include "pickup.h"
#include "credits.h"
//#include "gamegauge.h"
#include "menutext.h"
#include "MainMenu.h"
#include "field.h"
#include "mss.h"
#include "weapon.h"


// menus
#include "WaitingRoom.h"
#include "ui_TitleScreen.h"


#include "SoundEffectEngine.h"
#include "SoundEffects.h"
#include "MusicManager.h"
#include "soundbank.h"

#include "xbdm.h"  //$ADDITION(jedl) - for DmEnableGPUCounter

#include "XBInput.h"
#include "VoiceCommunicator.h"
#include "XBResource.h"
#include "VoiceManager.h"

// prototypes

void InitGameTrial(void);
void InitGamePractice(void);
void InitGameSingle(void);
void InitGameNetwork(void);
void InitGameReplay(void);
void InitGameFrontEnd(void);
void InitGameIntro(void);
void InitGameDemo(void);
void InitGameCalcStats(void);

void InitGameSettings(void);
//$REMOVED_UNREACHABLEstatic void RunBackgroundTasks(void);
//$REMOVED_DONTNEEDstatic void CheckNFO(void);
//$REMOVED_DONTNEEDstatic void CheckRegistryZZZ(void);
//$REMOVED_DONTNEEDstatic void CheckCD(void);

// DEBUGGING VARIABLE

#if SHOW_PHYSICS_INFO
long DEBUG_CollGrid = 0;
int DEBUG_NCols = 0;
int DEBUG_LastNCols = 0;
int DEBUG_N2Cols = 0;
VEC DEBUG_dR = {0, 0, 0};
VEC DEBUG_Impulse = {0, 0, 0};
VEC DEBUG_AngImpulse = {0, 0, 0};
VEC DEBUG_SNorm[256];
FACING_POLY DEBUG_Faces[256];
#endif

// version info

long UseLanguages = 0;
#define CHECK_IP 0
#define CHECK_CD 0  //$MODIFIED: was "1" originally
#define VERSION_TYPE_DEV  //$MODIFIED: was originally VERSION_TYPE_RELEASE

// version strings

#define VERSION_STRING_DEV          "dev"
#define VERSION_STRING_RELEASE      "v1.00atg"

// registry key

#define REGISTRY_KEY_DEV            "software\\Acclaim\\Re-Volt Dev\\1.0"
#define REGISTRY_KEY_RELEASE        "software\\Acclaim\\Re-Volt\\1.0"

// version shit

#if defined(VERSION_TYPE_DEV)

VERSION Version = VERSION_DEV;
char VersionString[] = VERSION_STRING_DEV;
char RegistryKey[] = REGISTRY_KEY_DEV;
MULTIPLAYER_VERSION MultiplayerVersion = MULTIPLAYER_VERSION_DEV;

#elif defined(VERSION_TYPE_RELEASE)

VERSION Version = VERSION_RELEASE;
char VersionString[] = VERSION_STRING_RELEASE;
char RegistryKey[] = REGISTRY_KEY_RELEASE;
MULTIPLAYER_VERSION MultiplayerVersion = MULTIPLAYER_VERSION_RELEASE;

#else

#error Invalid Version Type

#endif

// game mode text

long GameModeTextIndex[] = {
    TEXT_NONE,
    TEXT_TIMETRIAL,
    TEXT_SINGLERACE,
    TEXT_CLOCKWORKRACE2,
    TEXT_MULTIPLAYER,
    TEXT_REPLAY,
    TEXT_BATTLETAG,
    TEXT_CHAMPIONSHIP2,
    TEXT_PRACTICE,
    TEXT_TRAINING2,
    TEXT_NONE,
    TEXT_NONE,
    TEXT_DEMO,
    TEXT_NONE,
    TEXT_NONE,
    TEXT_NONE,
};

// globals

char gGazzasAICar = FALSE;
char gGazzasAICarDraw = FALSE;
char gGazzasRouteChoice = FALSE;
char gGazzasOvertake = FALSE;
int giGazzaForceRoute = LONG_MAX;

#ifndef _PC
bool gTrackEditorSpawned = FALSE;
#else
long gTrackEditorSpawnState = TRACKEDIT_NOT_SPAWNED;
#endif

bool ModifiedCarInfo = FALSE;
char WheelRenderType = 0;
char NoGamma = FALSE;
//$REMOVEDchar AppRestore = FALSE;
char g_bQuitGame = FALSE;
//$REMOVEDchar FullScreen = TRUE;
unsigned long FrameCount, FrameCountLast, FrameTime, FrameTimeLast, FrameRate;
long AlphaRef = 128;
long NoUser = FALSE;
long Skybox = FALSE;
unsigned long ChecksumCRC = 0xffffffff, ChecksumCD = 0x80808080;
long TracksCD = 0xffff;
long RegistryZZZ = FALSE;
long NFO = TRUE;
REAL TimeStep = ONE / 120.0f;
REAL RealTimeStep = ONE / 120.0f;
REAL gDemoTimer;
REAL gDemoFlashTimer;
long gDemoShowMessage;
REAL EditScale = 1.0f;
VEC EditOffset = {0.0f, 0.0f, 0.0f};
//$REMOVEDHWND hwnd = 0;
//HBITMAP TitleHbm;
GAME_SETTINGS GameSettings;
RENDER_SETTINGS RenderSettings;
bool ReplayMode = FALSE;
char LastFile[MAX_PATH];
unsigned short *SepiaRGB = NULL;

//$REMOVEDstatic WNDCLASS wcl;
static char WinName[] = "Revolt";
//$REMOVEDstatic char AppActive = TRUE;
static unsigned long StartTimeOffset = 0;
static HANDLE LoadThreadHandle = NULL;
static DWORD LoadThreadID = 0;
static volatile long LoadThreadPause, LoadThreadKill, LoadThreadUnits, LoadThreadUnitCount;
static long LoadThreadEnabled = FALSE;
static volatile float LoadThreadPer;

extern long WrongWayFlag;

// load thread shit

static unsigned long WINAPI LoadThread(void *param);
static long LoadingPic = 0;

static char *LoadBitmaps[2][4] = {
//$MODIFIED
//    {
//        "gfx\\loadfront1.bmp",
//        "gfx\\loadfront2.bmp",
//        "gfx\\loadfront3.bmp",
//        "gfx\\loadfront4.bmp",
//    },
//    {
//        "gfx\\loadlevel1.bmp",
//        "gfx\\loadlevel2.bmp",
//        "gfx\\loadlevel3.bmp",
//        "gfx\\loadlevel4.bmp",
//    }
    {
        "D:\\gfx\\loadfront1.bmp",
        "D:\\gfx\\loadfront2.bmp",
        "D:\\gfx\\loadfront3.bmp",
        "D:\\gfx\\loadfront4.bmp",
    },
    {
        "D:\\gfx\\loadlevel1.bmp",
        "D:\\gfx\\loadlevel2.bmp",
        "D:\\gfx\\loadlevel3.bmp",
        "D:\\gfx\\loadlevel4.bmp",
    }
//$END_MODIFICATIONS
};

// Car info file

char *CarInfoFile = "D:\\CarInfo.txt"; //$MODIFIED: added "D:\\" at start

// cd volume

static char CdVolume[] = "REVOLT";

// event ptr

void (*Event)(void);


//$MODIFIED
///////////////
//// WinMain //
///////////////
//int WINAPI WinMain(HINSTANCE hThisInst, HINSTANCE hPrevInst, LPSTR lpszArgs, int nWinMode)
void __cdecl main(void)
//$END_MODIFICATIONS
{
//$REMOVED    long i;
//$REMOVED    MSG msg;
//$REMOVED    HRESULT r;

#ifndef XBOX_NOT_YET_IMPLEMENTED
// parse command line args

    for (i = 1 ; i < __argc ; i++)
    {

// no sound

        if (!strcmp(__argv[i], "-nosound"))
        {
            SoundOff = TRUE;
            continue;
        }

// carinfo file

        if (!strcmp(__argv[i], "-carinfo"))
        {
            CarInfoFile = __argv[++i];
            continue;
        }

//$REMOVED
//// window
//
//        if (!strcmp(__argv[i], "-window"))
//        {
//            FullScreen = FALSE;
//            continue;
//        }
//
//// no gamma control
//
//        if (!strcmp(__argv[i], "-nogamma"))
//        {
//            NoGamma = TRUE;
//            continue;
//        }
//$END_REMOVAL

// no visicock per poly

        if (!strcmp(__argv[i], "-cubevisi"))
        {
            VisiPerPoly = FALSE;
            continue;
        }

// edit scale

        if (!strcmp(__argv[i], "-editscale"))
        {
            EditScale = (float)atof(__argv[++i]);
            continue;
        }

// edit offset

        if (!strcmp(__argv[i], "-editoffset"))
        {
            EditOffset.v[X] = (float)atof(__argv[++i]);
            EditOffset.v[Y] = (float)atof(__argv[++i]);
            EditOffset.v[Z] = (float)atof(__argv[++i]);
            continue;
        }

// alpha ref

        if (!strcmp(__argv[i], "-alpharef"))
        {
            AlphaRef = atol(__argv[++i]);
            continue;
        }

// no user tracks

        if (!strcmp(__argv[i], "-nouser"))
        {
            NoUser = TRUE;
            continue;
        }

// gazza's AI car

        if (!strcmp(__argv[i], "-gazzasaicar"))
        {
            gGazzasAICar = TRUE;
            gGazzasAICarDraw = TRUE;
            continue;
        }

// gazza's AI info draw

        if (!strcmp(__argv[i], "-gazzasaiinfo"))
        {
            gGazzasAICarDraw = TRUE;
            continue;
        }

// gazza's AI route choice

        if (!strcmp(__argv[i], "-gazzasroute"))
        {
            gGazzasRouteChoice = TRUE;
            continue;
        }

    }
#endif // ! XBOX_NOT_YET_IMPLEMENTED

	// init log file?
    wsprintf(DBG_TempPath, "D:\\revolt.log");
    DBG_LogFile = DBG_TempPath;
    InitLogFile();

//$ADDITION - initialize framerate tracker
    FrameRate_Init();
//$END_ADDITION

	// Initialize title screen options vars (must be before GetRegistrySettings)
    InitTitleScreenData();
    InitPickupWeightTables();

	// Initialise the credits
    InitCreditEntries();
    InitCreditStateInactive();

	// get registry settings
    InitGameSettings();
    GetRegistrySettings();

    ChecksumCD = ChecksumCRC = 0;
    TracksCD = REDBOOK_TRACK_NUM;
    NFO = FALSE;
    RegistryZZZ = TRUE;

	// find user-created levels
    FindUserLevels();

	// Create the replay buffer
    RPL_CreateReplayBuffer(REPLAY_DATA_SIZE);

	// read car info
    //if (!ReadAllCarInfo(CarInfoFile))
    if (!ReadAllCarInfoMultiple()) 
    {
        return;  //$MODIFIED: was "return FALSE"
    }

    SetAllCarCoMs();
    CalcCarStats();

#ifndef XBOX_DISABLE_NETWORK
// init lobby

    LobbyInit();  HEY -- BUFFERS GET ALLOCATED VIA HERE! (in LobbyConnect)
#endif // ! XBOX_DISABLE_NETWORK

    //$TODO(jedl) - get rid of the texture pages, since these will be handled
    // by faster resource loading that does the allocation.
	// create tpage mem
    if (!CreateTPages(TPAGE_NUM))
        return;  //$MODIFIED: was "return FALSE"

	// get timer freq
    __int64 time;
    QueryPerformanceFrequency((LARGE_INTEGER*)&time);
    TimerFreq = (unsigned long)(time >> TIMER_SHIFT);

	// set rand seed
    srand(CurrentTimer());

	// Initialize the Xbox controllers
    XBInput_InitControllers();

	// Initialize the Xbox voice communicators
    CVoiceCommunicator::InitCommunicators();

    if (!InitDX())
    {
        g_bQuitGame = TRUE;
    }

	//$ADDITION(jedl) - this should done just once at the beginning
    if (!InitD3D(640,480,32,XBOX_UNUSED_PARAM))
    {
        g_bQuitGame = TRUE;
    }
//$END_ADDITION

// init sound system
#ifdef OLD_AUDIO
    InitSound(RegistrySettings.SfxChannels);
#else
    g_SoundEngine.Initialize();
    g_MusicManager.Initialize();
    g_MusicManager.SetVolume( -600 );
    #pragma message( "jharding review this: they added a parameter to InitSound, if that matters" )
#endif    

	// init function keys
    InitFunctionKeys();
    AddAllFunctionKeys();

	// init Start data
    InitStartData();

	// create sepia table
    CreateSepiaRGB();

// set start event

    SET_EVENT(Go);

#pragma message( "JedL, you'll need to put back your code to save car posn every second." )
//$NOTE(cprince): Jed, this was because the framerate tracking code changed.  Maybe use TimerCurrent in the main loop below to re-implement it.

// main loop

    while (!g_bQuitGame)
    {
        Event();
    }

	// kill sepia table
    KillSepiaRGB();

	// release DX misc
    FreeTextures();
    KillInput();
    ReleaseD3D();
    ReleaseDX();

// kill direct play
//$TODO: fix this comment

    KillNetwork();  //$TODO(cprince): will migrate this to KillNetwork()

	// release sound system
#ifdef OLD_AUDIO
    ReleaseSound();
#else
    g_SoundEngine.Unload();  //$TODO(JHarding): What other shutdown?
#endif

	// destroy tpage mem
    DestroyTPages();

	// kill car info
    DestroyCarInfo();

	// Free replay Buffer
    RPL_DestroyReplayBuffer();

	// free levels
    FreeUserLevels();

	// save registry settings
    SetRegistrySettings();

#ifndef XBOX_DISABLE_NETWORK
// free lobby

    LobbyFree();
#endif // ! XBOX_DISABLE_NETWORK

#if USE_DEBUG_ROUTINES
	// See if we have forgotten to release anything
    CheckMemoryAllocation();
#endif
}

/////////////////////////
// display message box //
/////////////////////////
void DumpMessage(char *title, char *mess)
{
    #pragma message( "CPRINCE: do we need to implement this LoadThread** stuff?  Probably not..." )

    if(NULL != title) 
    {
        OutputDebugString( title );
        OutputDebugString( " -- " );
    }
    if( NULL != mess )
    {
        OutputDebugString( mess );
    }
    OutputDebugString( "\n" );
}




/////////////
// go game //
/////////////
void Go(void)
{

// load english language if game gauge, lobby launch, or not using languages

#ifndef XBOX_DISABLE_NETWORK
    if (Lobby || !UseLanguages)
#else
    if (!UseLanguages) //$CPRINCE MODIFIED VERSION
#endif
    {
        LANG_LoadStrings(ENGLISH);
    }

// set event

#ifndef XBOX_DISABLE_NETWORK
    if ((Version == VERSION_RELEASE) && !Lobby)
#else
    if ((Version == VERSION_RELEASE))  //$CPRINCE MODIFIED VERSION
#endif
        SET_EVENT(SetupSharewareIntro);
    else
        SET_EVENT(GoTitleScreen);  //$NOTE:  eg, VERSION_DEV
}




/////////////////////
// setup front end //
/////////////////////
void GoFront(void)
{
	// test linear problem solver??? (eh?)
#if DEBUG_SOLVER
    TestConjGrad();
#endif

    PickTextureFormat();  //$RENAMED: was GetTextureFormat()
    InitTextures();
    InitFadeShit();
    SetupDxState();

    // load misc textures
    LoadMipTexture("D:\\gfx\\font.bmp", TPAGE_FONT, 256, 256, 0, 1, FALSE);
    LoadMipTexture("D:\\gfx\\loading.bmp", TPAGE_LOADING, 256, 256, 0, 1, FALSE);
    LoadMipTexture("D:\\gfx\\spru.bmp", TPAGE_SPRU, 256, 256, 0, 1, FALSE);

	// set geom vars
    RenderSettings.GeomPers = BaseGeomPers;
    SetNearFar(NEAR_CLIP_DIST, 4096.0f);
    SetViewport(0, 0, (float)ScreenXsize, (float)ScreenYsize, RenderSettings.GeomPers);

	// setup main menu
    MenuCount = 0;
    SET_EVENT(MainMenu);
}




//////////////////
// setup a game //
//////////////////
void SetupGame(void)
{
//$REMOVED(jedl;see below)    unsigned long w, h;

// set sound channels

    SetSoundChannels(RegistrySettings.SfxChannels);

// set the level number

    GameSettings.Level = GetLevelNum(StartData.LevelDir);
    if (GameSettings.Level == -1) {
        char buf[256];
        wsprintf(buf, "Couldn't find level \"%s\"", StartData.LevelDir);
        DumpMessage(NULL,buf);
        g_bQuitGame = TRUE;
        return;
    }

// kill title bitmap + textures

    //FreeBitmap(TitleHbm);
    //FreeTextures();

//$REMOVED (tentative!!)
//// draw device changed?
//
//    if (RegistrySettings.DrawDevice != (DWORD)CurrentDrawDevice)
//    {
//        DD->FlipToGDISurface();
//        FreeTextures();
//        ReleaseD3D();
//        InitDX();
//    }
//$END_REMOVAL

// sepia?

    if (RenderSettings.Sepia = (CreditVars.State != CREDIT_STATE_INACTIVE))
        InitFilmLines();

//$REMOVED(jedl) - this should done just once at the beginning
//// init D3D
//
//    w = ScreenXsize;
//    h = ScreenYsize;
//
//    if (!InitD3D(DrawDevices[RegistrySettings.DrawDevice].DisplayMode[DisplayModeCount].Width, DrawDevices[RegistrySettings.DrawDevice].DisplayMode[DisplayModeCount].Height, DrawDevices[RegistrySettings.DrawDevice].DisplayMode[DisplayModeCount].Bpp, 0))
//    {
//        g_bQuitGame = TRUE;
//        return;
//    }
//$END_REMOVAL

// init textures

    PickTextureFormat();  //$RENAMED: was GetTextureFormat()
    InitTextures();
    InitFadeShit();
    SetupDxState();

// pick texture sets

    if (GameSettings.GameType == GAMETYPE_CLOCKWORK || (GameSettings.GameType == GAMETYPE_REPLAY && StartDataStorage.GameType == GAMETYPE_CLOCKWORK))
        PickTextureSets(1, TPAGE_WORLD_NUM, TPAGE_SCALE_NUM, TPAGE_FIXED_NUM);
    else if (GameSettings.GameType == GAMETYPE_TRIAL)
        PickTextureSets(2, TPAGE_WORLD_NUM, TPAGE_SCALE_NUM, TPAGE_FIXED_NUM);
    else
        PickTextureSets(StartData.PlayerNum, TPAGE_WORLD_NUM, TPAGE_SCALE_NUM, TPAGE_FIXED_NUM);

// load misc textures

//$MODIFIED
//    LoadMipTexture("gfx\\font.bmp", TPAGE_FONT, 256, 256, 0, 1, FALSE);
//    LoadMipTexture("gfx\\loading.bmp", TPAGE_LOADING, 256, 256, 0, 1, FALSE);
//    LoadMipTexture("gfx\\spru.bmp", TPAGE_SPRU, 256, 256, 0, 1, FALSE);
    LoadMipTexture("D:\\gfx\\font.bmp", TPAGE_FONT, 256, 256, 0, 1, FALSE);
    LoadMipTexture("D:\\gfx\\loading.bmp", TPAGE_LOADING, 256, 256, 0, 1, FALSE);
    LoadMipTexture("D:\\gfx\\spru.bmp", TPAGE_SPRU, 256, 256, 0, 1, FALSE);
//$END_MODIFICATIONS

//$REMOVED
//// move window if windowed and changed res
//
//    if (!FullScreen && (w != ScreenXsize || h != ScreenYsize))
//    {
//        long bx, by, cy;
//        bx = GetSystemMetrics(SM_CXSIZEFRAME);
//        by = GetSystemMetrics(SM_CYSIZEFRAME);
//        cy = GetSystemMetrics(SM_CYCAPTION);
//
//        MoveWindow(hwnd,
//            (GetSystemMetrics(SM_CXSCREEN) - ScreenXsize) / 2 - bx,
//            (GetSystemMetrics(SM_CYSCREEN) - ScreenYsize) / 2 - by - cy / 2,
//            ScreenXsize + bx + bx,
//            ScreenYsize + by + by + cy,
//            TRUE);
//    }
//$END_REMOVAL

    // set steering deadzone / range?
#ifdef _PC
  #ifndef XBOX_NOT_YET_IMPLEMENTED
    if (RegistrySettings.Joystick != -1)
    {
        if (KeyTable[KEY_LEFT].Type == KEY_TYPE_AXISNEG || KeyTable[KEY_LEFT].Type == KEY_TYPE_AXISPOS)
            SetAxisProperties(KeyTable[KEY_LEFT].Index, RegistrySettings.SteeringDeadzone * 100, RegistrySettings.SteeringRange * 100);

        if (KeyTable[KEY_RIGHT].Type == KEY_TYPE_AXISNEG || KeyTable[KEY_RIGHT].Type == KEY_TYPE_AXISPOS)
            SetAxisProperties(KeyTable[KEY_RIGHT].Index, RegistrySettings.SteeringDeadzone * 100, RegistrySettings.SteeringRange * 100);
    }
  #endif // !XBOX_NOT_YET_IMPLEMENTED
#endif

    // Load and setup Everything
    EnableLoadThread(STAGE_ONE_LOAD_COUNT + StartData.PlayerNum);
    SetupLevelAndPlayers();
    DisableLoadThread();

    // Misc stuff
    ClearMenuHeader(&gMenuHeader);

    // go game loop
    SET_EVENT(GLP_GameLoop);
}


void SetupLevelAndPlayers(void)
{

// Init AI time step

    CAI_InitTimeStep();

// Menuing scale factors dependent on screen mode

    gMenuWidthScale = ScreenXsize / 640.0f;
    gMenuHeightScale = ScreenYsize / 480.0f;

// disable skybox

    Skybox = FALSE;

// init level stage one

    if (GameSettings.LoadStage == LOAD_STAGE_ZERO) {
        LEV_InitLevelStageOne();
    }

// init level stage two

    if (GameSettings.LoadStage == LOAD_STAGE_TWO) {
        LEV_EndLevelStageTwo();
    }
    if (GameSettings.LoadStage == LOAD_STAGE_ONE) {
        LEV_InitLevelStageTwo();
    }

// create all players

    InitStartingPlayers();

// init sfx / music vol

    UpdateMusicVol(g_TitleScreenData.musicVolume);
    if (CreditVars.State == CREDIT_STATE_INACTIVE) {
        UpdateSfxVol(g_TitleScreenData.sfxVolume);
    } else {
        UpdateSfxVol(0);
    }

// play redbook?

    if (CurrentLevelInfo.RedbookStartTrack != -1 && CurrentLevelInfo.RedbookEndTrack != -1 && Version == VERSION_RELEASE)
    {
        SetRedbookFade(REDBOOK_FADE_UP);
        if ((GameSettings.GameType == GAMETYPE_FRONTEND) && g_bShowWinLoseSequence) {
            if (CupTable.LocalPlayerPos > 3) {
                PlayRedbookTrack(REDBOOK_TRACK_LOSECUP, REDBOOK_TRACK_LOSECUP, FALSE);
            } else {
                PlayRedbookTrack(REDBOOK_TRACK_WINCUP, REDBOOK_TRACK_WINCUP, FALSE);
            }
        } else {
            if (CreditVars.State == CREDIT_STATE_INACTIVE) {
            if (RegistrySettings.MusicOn || GameSettings.Level == LEVEL_FRONTEND)
                PlayRedbookTrackRandom(CurrentLevelInfo.RedbookStartTrack, CurrentLevelInfo.RedbookEndTrack, TRUE);
            } else {
                PlayRedbookTrack(REDBOOK_TRACK_CREDITS, REDBOOK_TRACK_CREDITS, FALSE);
            }
        } 
    }

// init game-type specific stuff

    switch (GameSettings.GameType)
    {
        case GAMETYPE_TRIAL:
            InitGameTrial();
            break;

        case GAMETYPE_PRACTICE:
            InitGamePractice();
            break;

        case GAMETYPE_TRAINING:
            InitGamePractice();
            break;

        case GAMETYPE_DEMO:
            InitGameDemo();
            break;

        case GAMETYPE_SINGLE:
        case GAMETYPE_CLOCKWORK:
        case GAMETYPE_CHAMPIONSHIP:
            InitGameSingle();
            break;

        case GAMETYPE_MULTI:
        case GAMETYPE_BATTLE:
            InitGameNetwork();
            break;

        case GAMETYPE_REPLAY:
            InitGameReplay();
            break;

        case GAMETYPE_FRONTEND:
            InitGameFrontEnd();
            break;

        case GAMETYPE_INTRO:
            InitGameIntro();
            break;

        case GAMETYPE_CALCSTATS:
            InitGameCalcStats();
            break;

        default:
            DumpMessage(NULL,"Impossible game mode!");
            g_bQuitGame = TRUE;
            return;
    }

// Setup rearview camera if required

    if (CAM_RearCamera)
    {
        SetCameraFollow(CAM_RearCamera, PLR_LocalPlayer->ownobj, CAM_FOLLOW_FRONT);
    }


// Set up a ghost if required

    if (GHO_GhostAllowed) {
        long playerCarType;

        GHO_GhostDataExists = LoadGhostData();
        if (GHO_GhostDataExists) {

            if (GHO_BestGhostInfo->CarType < NCarTypes) {
                playerCarType = GHO_BestGhostInfo->CarType;
            } else {
                playerCarType = 0;
            }
        } else {
            ClearBestGhostData();
        }

        // Create the ghost player and initialise
        InitBestGhostData();

        InitGhostData(PLR_LocalPlayer);
        InitGhostLight();

    
    } else {
        GHO_GhostDataExists = FALSE;
        GHO_GhostPlayer = NULL;
    }

// Misc other stuff

    GameSettings.Paws = FALSE;
    GameLoopQuit = GAMELOOP_QUIT_OFF;
    FoxObj = NULL;
    GLP_TriggerGapCamera = FALSE;
    GLP_GapCameraTimer = TO_TIME(Real(-100));
    ChallengeFlash = 0;
    PracticeStarFlash = 0;
    ChampionshipEndMode = CHAMPIONSHIP_END_WAITING_FOR_FINISH;
    GlobalPickupFlash = 0.0f;
    WrongWayFlag = FALSE;

// clear multi messages

    WaitingRoomMessageActive = FALSE;
    WaitingRoomMessageTimer = 0.0f;

    WaitingRoomMessagePos = 0;
    WaitingRoomCurrentMessage[0] = 0;

    for (long i = 0 ; i < WAITING_ROOM_MESSAGE_NUM ; i++)
    {
        WaitingRoomMessages[i][0] = 0;
    }

// clockwork jiggery pokery?

    if (GameSettings.GameType == GAMETYPE_CLOCKWORK || (GameSettings.GameType == GAMETYPE_REPLAY && StartDataStorage.GameType == GAMETYPE_CLOCKWORK))
    {
        RandomizeStartingGrid();
        LoadTextureClever(CarInfo[CARID_KEY4].TPageFile, TPAGE_CAR_START, 256, 256, 0, CarTextureSet, TRUE);

        for (PLAYER *player = PLR_PlayerHead ; player ; player = player->next)
        {
            long rgb = (rand() & 255) | (rand() & 255) << 8 | (rand() & 255) << 16;

            POLY_RGB *mrgb = player->car.Models->Model[CAR_MODEL_BODY][0].PolyRGB;
            for (long j = 0 ; j < player->car.Models->Model[CAR_MODEL_BODY][0].PolyNum ; j++, mrgb++)
            {
                *(long*)&mrgb->rgb[0] = rgb;
                *(long*)&mrgb->rgb[1] = rgb;
                *(long*)&mrgb->rgb[2] = rgb;
                *(long*)&mrgb->rgb[3] = rgb;
            }
        }
    }

// update time step

    UpdateTimeStep();

// set rand seed if multiplayer

    if (IsMultiPlayer())
    {
        srand(StartData.Seed);
    }

// init pickups

    if (GameSettings.AllowPickups) {
        InitPickups();
    }

// init stars

    InitStars();

// init pickup weight table

    InitPickupRaceWeightTables();


/////////////////// TEST
#if 0
    if (GameSettings.Level == LEVEL_GHOSTTOWN1)
    {
        VEC pos;
        MAT mat;
        long flags[4];
        OBJECT* pObj;
        //SetVec(&pos, 0,-100,0);
        CopyVec(&Players[0].car.Body->Centre.Pos, &pos);
        pos.v[1] -= 150;
        SetMatUnit(&mat);
        pObj = CreateObject(&pos, &mat, OBJECT_TYPE_FLAG, flags);
    }
#endif
/////////////////// TEST

// fade up

    SetFadeEffect(FADE_UP);

// Hopefully clear keyboard buffer

    ReadKeyboard();
}

/////////////////////////////
// init time trial players //
/////////////////////////////

void InitGameTrial(void)
{

    // Set camera to follow local player's car
    SetCameraFollow(CAM_MainCamera, PLR_LocalPlayer->ownobj, CAM_FOLLOW_BEHIND);

    // Run the game loop for a while to put the cars on the floor
    SetAllPlayerHandlersToDrop();

    GameSettings.AllowPickups = FALSE;
    AllPlayersReady = TRUE;
    GHO_GhostAllowed = TRUE;
    ReplayMode = FALSE;
    ReachedEndOfReplay = FALSE;
    RPL_RecordReplay = FALSE;
    //RPL_InitReplayBuffer();
    //InitCountDown();
    //InitCountDownNone();
    InitCountDownDelta(COUNTDOWN_START - 1);
    ResetAllPlayerHandlersToDefault();
}

///////////////////////////
// init practice players //
///////////////////////////

void InitGamePractice(void)
{
    // Set camera to follow local player's car
    SetCameraFollow(CAM_MainCamera, PLR_LocalPlayer->ownobj, CAM_FOLLOW_BEHIND);

    // Run the game loop for a while to put the cars on the floor
    SetAllPlayerHandlersToDrop();

    // Force Arcade mode in practise (for Stunt Arena so the loop and halp pipe work)
    GameSettings.PlayMode = PLAYMODE_ARCADE;

    GameSettings.AllowPickups = FALSE;
    AllPlayersReady = TRUE;
    GHO_GhostAllowed = FALSE;
    ReplayMode = FALSE;
    ReachedEndOfReplay = FALSE;
    RPL_RecordReplay = FALSE;
    //RPL_InitReplayBuffer();
    //InitCountDown();
    //InitCountDownNone();
    InitCountDownDelta(COUNTDOWN_START - 1);
    ResetAllPlayerHandlersToDefault();
}

/////////////////////////////
// init time trial players //
/////////////////////////////

void InitGameReplay(void)
{

    // Set camera to static by default
    if (GameSettings.Level < LEVEL_NCUP_LEVELS) {
        SetCameraRail(CAM_MainCamera, PLR_LocalPlayer->ownobj, CAM_RAIL_DYNAMIC_MONO);
    } else {
        SetCameraFollow(CAM_MainCamera, PLR_LocalPlayer->ownobj, CAM_FOLLOW_BEHIND);
    }

    // Run the game loop for a while to put the cars on the floor
    SetAllPlayerHandlersToDrop();

    GHO_GhostAllowed = FALSE;
    AllPlayersReady = TRUE;
    ReplayMode = TRUE;
    ReachedEndOfReplay = FALSE;
    GameSettings.NumberOfLaps = -1;

    RPL_InitReplay();
}

//////////////////////////////
// init single game players //
//////////////////////////////

void InitGameSingle(void)
{

    // Set camera to follow local player's car
    if (GameSettings.Level >= LEVEL_NCUP_LEVELS) {
        SetCameraFollow(CAM_MainCamera, PLR_LocalPlayer->ownobj, CAM_FOLLOW_BEHIND);
    } else {
        SetCameraSweep(CAM_MainCamera, PLR_LocalPlayer->ownobj, CAM_FOLLOW_BEHIND);
    }

    // Run the game loop for a while to put the cars on the floor
    SetAllPlayerHandlersToDrop();

    AllPlayersReady = TRUE;
    GHO_GhostAllowed = FALSE;
    ReachedEndOfReplay = FALSE;
    RPL_InitReplayBuffer();
    ReplayMode = FALSE;
    InitCountDown();
}

//////////////////////////////
// init demo
//////////////////////////////

void InitGameDemo(void)
{

    // Set camera to follow local player's car
    SetCameraRail(CAM_MainCamera, PLR_LocalPlayer->ownobj, CAM_RAIL_DYNAMIC_MONO);

    // Run the game loop for a while to put the cars on the floor
    SetAllPlayerHandlersToDrop();

    gDemoTimer = gDemoFlashTimer = ZERO;
    gDemoShowMessage = FALSE;

    AllPlayersReady = TRUE;
    GHO_GhostAllowed = FALSE;
    RPL_RecordReplay = FALSE;
    ReplayMode = FALSE;
    ReachedEndOfReplay = FALSE;
    InitCountDown();
}

////////////////////////////////////////////////////////////////
// Front end
////////////////////////////////////////////////////////////////

void InitGameFrontEnd(void)
{
    // Set camera to follow local player's car
    AllPlayersReady = TRUE;
    GHO_GhostAllowed = FALSE;
    ReplayMode = FALSE;
    ReachedEndOfReplay = FALSE;
    RPL_RecordReplay = FALSE;
    GameSettings.NumberOfLaps = -1;
    InitCountDownNone();
}

////////////////////////////////////////////////////////////////
// IntroSequence
////////////////////////////////////////////////////////////////

void InitGameIntro(void)
{
    // Set camera to follow local player's car
    AllPlayersReady = TRUE;
    GHO_GhostAllowed = FALSE;
    ReplayMode = FALSE;
    ReachedEndOfReplay = FALSE;
    RPL_RecordReplay = FALSE;
    GameSettings.NumberOfLaps = -1;
    InitCountDownNone();
}

//////////////////////////
// init network players //
//////////////////////////

void InitGameNetwork(void)
{
    // Set camera to follow local player's car
    SetCameraFollow(CAM_MainCamera, PLR_LocalPlayer->ownobj, CAM_FOLLOW_BEHIND);
    
    // Run the game loop for a while to put the cars on the floor
    SetAllPlayerHandlersToDrop();

    // set countdown timer + misc network flags
    InitCountDown();
    AllPlayersReady = FALSE;
    PLR_LocalPlayer->Ready = TRUE;

    SendGameLoaded();

    NextPacketTimer = 0.0f;
    NextSyncTimer = DP_SYNC_TIME;
    NextSyncMachine = 0;
    NextPositionTimer = DP_POSITION_TIME;
    MessageQueueSize = 0;
//$REMOVED
//    TotalDataSent = 0;
//$END_REMOVAL
    GlobalID = 0;
    RPL_InitReplayBuffer();
    ReplayMode = FALSE;
    ReachedEndOfReplay = FALSE;
    GHO_GhostAllowed = FALSE;
    AllPlayersFinished = FALSE;
}

/////////////////////////////
// init calc stats data    //
/////////////////////////////

void InitGameCalcStats(void)
{
    VEC pos;

    // Set camera to follow local player's car
    SetCameraFollow(CAM_MainCamera, PLR_LocalPlayer->ownobj, CAM_FOLLOW_BEHIND);

    // Run the game loop for a while to put the cars on the floor
    SetAllPlayerHandlersToDrop();

    // Kill the collision skin and gridding, then rebuild it with one poly covering the whole world
    DestroyCollGrids();
    DestroyCollPolys(COL_WorldCollPoly);
    COL_WorldCollPoly = NULL;
    COL_NWorldCollPolys = 0;
    DestroyCollPolys(COL_InstanceCollPoly);
    COL_InstanceCollPoly = NULL;
    COL_NInstanceCollPolys = 0;

    if ((COL_WorldCollPoly = CreateCollPolys(1)) != NULL) {
        COL_NWorldCollPolys = 1;

        SetBBox(&COL_WorldCollPoly[0].BBox, -LARGEDIST, LARGEDIST, -LARGEDIST, LARGEDIST, -LARGEDIST, LARGEDIST);
        
        SetPlane(&COL_WorldCollPoly[0].EdgePlane[0], -ONE, ZERO, ZERO, -LARGEDIST);
        SetPlane(&COL_WorldCollPoly[0].EdgePlane[1], ZERO, ZERO, ONE, -LARGEDIST);
        SetPlane(&COL_WorldCollPoly[0].EdgePlane[2], ONE, ZERO, ZERO, -LARGEDIST);
        SetPlane(&COL_WorldCollPoly[0].EdgePlane[3], ZERO, ZERO, -ONE, -LARGEDIST);
        
        SetPlane(&COL_WorldCollPoly[0].Plane, ZERO, -ONE, ZERO, ZERO);
        
        COL_WorldCollPoly[0].Material = MATERIAL_DEFAULT;
        
        COL_WorldCollPoly[0].Type = POLY_QUAD;

        LoadGridInfo(NULL);
    }

    // Place car
    SetVec(&pos, ZERO, -TO_LENGTH(Real(50)), ZERO);
    SetCarPos(&Players[0].car, &pos, &Identity);

    GameSettings.AllowPickups = FALSE;
    AllPlayersReady = TRUE;
    GHO_GhostAllowed = TRUE;
    ReplayMode = FALSE;
    ReachedEndOfReplay = FALSE;
    RPL_RecordReplay = FALSE;
    InitCountDownDelta(COUNTDOWN_START - 1);
    ResetAllPlayerHandlersToDefault();


}

/////////////////////////
// check cheat strings //
/////////////////////////

#define MAX_CHEAT_STRING_BUFFER 16
#define CHEAT_XOR (char)-1
#define CHEAT(c) (c ^ CHEAT_XOR)

static char CheatStringBuffer[MAX_CHEAT_STRING_BUFFER + 1];

static char CheatStrings[][MAX_CHEAT_STRING_BUFFER + 1] = {
    {CHEAT('b'), CHEAT('a'), CHEAT('r'), CHEAT('g'), CHEAT('a'), CHEAT('r'), CHEAT('a'), CHEAT('m'), CHEAT('a'), 0},
    {CHEAT('j'), CHEAT('o'), CHEAT('k'), CHEAT('e'), CHEAT('r'), 0},
    {CHEAT('g'), CHEAT('i'), CHEAT('m'), CHEAT('m'), CHEAT('e'), CHEAT('c'), CHEAT('r'), CHEAT('e'), CHEAT('d'), CHEAT('i'), CHEAT('t'), CHEAT('s'), 0},

    {0}
};

void CheckCheatStrings(void)
{
    long flag, i, len;
    unsigned char ch;

// update buffer

    ch = GetKeyPress();
    if (!ch) return;

    memcpy(CheatStringBuffer, CheatStringBuffer + 1, MAX_CHEAT_STRING_BUFFER - 1);
    CheatStringBuffer[MAX_CHEAT_STRING_BUFFER - 1] = ch ^ CHEAT_XOR;
    CheatStringBuffer[MAX_CHEAT_STRING_BUFFER] = 0;

// look for cheat string

    flag = 0;

    while (TRUE)
    {
        len = 0;
        while (CheatStrings[flag][len]) len++;

        if (!len)
            return;

        for (i = 0 ; i < len ; i++)
        {
            if (CheatStrings[flag][i] != CheatStringBuffer[MAX_CHEAT_STRING_BUFFER - len + i])
            {
                break;
            }
        }

        if (i == len)
            break;

        flag++;
    }

// act

    ZeroMemory(CheatStringBuffer, MAX_CHEAT_STRING_BUFFER);
#ifdef OLD_AUDIO
    PlaySfx(SFX_HONK, SFX_MAX_VOL, SFX_CENTRE_PAN, SFX_SAMPLE_RATE, 0);
#else
    g_SoundEngine.Play2DSound( EFFECT_HonkGood, FALSE );
#endif

    switch (flag)
    {
        case 0:
            FreeOneTexture(TPAGE_ENVSTILL);
            LoadMipTexture("D:\\gfx\\font.bmp", TPAGE_ENVSTILL, 128, 128, 0, 1, TRUE);  //$MODIFIED: added "D:\\"
            WheelRenderType = !WheelRenderType;
            break;

        case 1:
            if (IsMultiPlayer())
            {
                SendCarNewCarAll(GameSettings.CarType);
            }
            break;

        case 2:
            AllowCredits = !AllowCredits;
            break;
    }
}


/////////////////////////////////////////////////////////////////////
//
// InitGameSettings:
//
/////////////////////////////////////////////////////////////////////

void InitGameSettings(void)
{
    GameSettings.GameType = GAMETYPE_NONE;
    GameSettings.Level = -1;
    GameSettings.LevelNum = 0;
    GameSettings.NumberOfLaps = 0;
    GameSettings.Reversed = FALSE;
    GameSettings.Mirrored = FALSE;
    GameSettings.AutoBrake = FALSE;
    GameSettings.CarType = 0;
    GameSettings.Paws = FALSE;
    GameSettings.AllowPickups = FALSE;
    GameSettings.LoadStage = LOAD_STAGE_ZERO;
    GameSettings.LoadWorld = TRUE;
    GameSettings.LocalGhost = TRUE;
    GameSettings.DrawFollowView = FALSE;
    GameSettings.DrawRearView = FALSE;
    GameSettings.PlayMode = PLAYMODE_ARCADE;
}

//////////////////////////////////
// get CRC checksum from a file //
//////////////////////////////////

extern unsigned long GetFileChecksum(char *file, bool appendedChecksum, bool binaryMode)
{
    unsigned long crc;
    FILE *fp;
    char buf[MAX_PATH];

// open file

    fp = fopen(file, "rb");
    if (!fp)
    {
        wsprintf(buf, "Can't open %s for CRC checksum", file);
        DumpMessage(NULL,buf);
        return 0;
    }

// calculate checksum

    crc = GetStreamChecksum(fp, appendedChecksum, binaryMode);
    if (crc == 0) {
        wsprintf(buf, "Can't alloc memory for %s CRC checksum", file);
        DumpMessage(NULL,buf);
    }


    fclose(fp);
    return crc;
}

////////////////////////////////////////////////////////////////
//
// Calculate a stream's checksum
//
////////////////////////////////////////////////////////////////

unsigned long GetStreamChecksum(FILE *fp, bool appendedChecksum, bool binaryMode)
{
    long crc, filesize;
    long *data;
    fpos_t fPos;

// start from beginning of file (and remember position to reset at end)

    fgetpos(fp, &fPos);
    rewind(fp);

// read data

    fseek(fp, 0, SEEK_END);
    filesize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

// if the file has a checksum appended, don't include it in the calculation

    if (appendedChecksum) {
        if (binaryMode) {
            filesize -= sizeof(unsigned long);
        } else {
            filesize -= 8;
        }
    }

    data = (long*)malloc(filesize);
    if (!data)
    {
        return 0;
    }

    fread(data, filesize, 1, fp);

// calc crc

    crc = GetMemChecksum(data, filesize);

// free mem

    free(data);

    fsetpos(fp, &fPos);

// return checksum

    return crc;
}

////////////////////////////////////////////////////////////////
//
// Calculate a mem chunk checksum
//
////////////////////////////////////////////////////////////////

unsigned long GetMemChecksum(long *mem, long bytes)
{
    long crc, bits, i, flag;
    long *pos;

// calc crc

    bytes >>= 2;

    crc = 0xffffffff;
    pos = mem;
    for ( ; bytes ; bytes--, pos++)
    {
        bits = *pos;
        for (i = 32 ; i ; i--)
        {
            flag = crc & 0x80000000;
            crc  = (crc << 1) | (bits & 1);
            bits >>= 1;

            if (flag)
            {
                crc ^= 0x04c11db7;  // polynomial
            }
        }
    }

    crc ^= 0xffffffff;

// return crc

    return crc;
}

////////////////////////////////////////////////////////////////
//
// Compare the checksum of the file with the one appended to the
// file
//
////////////////////////////////////////////////////////////////

bool CheckStreamChecksum(FILE *fp, bool binaryMode)
{
    unsigned long crcRead, crcCalc;
    fpos_t fpos;

    // Store file position to reset it
    fgetpos(fp, &fpos);

    // Calculate crc checksum
    crcCalc = GetStreamChecksum(fp, TRUE, binaryMode);

    // Read the checksum form the end of the file
    if (binaryMode) {
        fseek(fp, -4, SEEK_END);
        fread(&crcRead, sizeof(unsigned long), 1, fp);
    } else {
        char crcString[16] = "0x";
        fseek(fp, -8, SEEK_END);
        fread(&crcString[2], 8, 1, fp);
        crcRead = HexStringToInt(crcString);
    }

    // reset file position
    fsetpos(fp, &fpos);

    // Compare the read and calculated checksums
    if (crcCalc == crcRead) {
        return TRUE;
    } else {
        return FALSE;
    }
}


////////////////////////////////////////////////////////////////
//
// HexStringToInt:
//
////////////////////////////////////////////////////////////////

unsigned long HexStringToInt(char *string)
{
    unsigned long   sLen = strlen(string);
    unsigned long   multiply, ret, num;
    long    iChar;
    char    c;

    ret = 0;
    multiply = 1;

    for (iChar = sLen - 1; iChar >= 0; iChar--) {
        c = toupper(string[iChar]);

        // ignore spaces
        if (c == ' ') continue;

        // if string is prepended with '0x'
        if ((c == 'x') || (c == 'X')) break;

        // get the value of the current char
        if (isdigit(c)) {
            num = c - '0';
        } else if (isalpha(c) && (c <= 'F')) {
            num = 10 + c - 'A';
        } else {
            return 0;
        }

        ret += num * multiply;
        multiply *= 16;

    }

    return ret;
}

////////////////////////
// enable load thread //
////////////////////////

void EnableLoadThread(long units)
{
    long i, j, beattime;
    float xstart, ystart, xsize, ysize;
    LEVELINFO *li = GetLevelInfo(GameSettings.Level);
    char buf[128];
    unsigned long time;
    short y;

// clear kill flag

    LoadThreadKill = FALSE;

// clear pause flag

    LoadThreadPause = 0;

// set unit stuff

    LoadThreadUnits = units;
    LoadThreadUnitCount = 0;
    LoadThreadPer = 0.0f;

// set enabled

    LoadThreadEnabled = TRUE;

// clear filename buffer

    LastFile[0] = 0;

// load bmp's

    for (i = 0 ; i < 4 ; i++)
    {
        LoadMipTexture(LoadBitmaps[LoadingPic][i], TPAGE_MISC1 + (char)i, 256, 256, 0, 1, FALSE);
    }

// set viewport

    SetViewport(0.0f, 0.0f, (float)ScreenXsize, (float)ScreenYsize, 640.0f);

// draw static shit on each frame buffer

    for (i = 0 ; i < 3 ; i++)
    {

// flip / clear buffers

        FlipBuffers();
//$MODIFIED
//        D3Dviewport->Clear2(1, &ViewportRect, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x000000, 1.0f, 0);
        D3Ddevice->Clear( 1, // num rectangles
                          &ViewportRect, // rectangle array
                          D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_ZBUFFER, // flags
                          0x000000,  // color value
                          1.0f, // z value
                          0 // stencil value
                        );
        //$CMP_NOTE: do we really need the rectangles here?  Or can we just clear the entire current viewport ???
//$END_MODIFICATIONS

// begin scene

        D3Ddevice->BeginScene();

// init render states

        InitRenderStates();
        ZBUFFER_OFF();

// draw pic

        for (j = 0 ; j < 4 ; j++)
        {
            DrawVertsTEX1[0].sx = DrawVertsTEX1[3].sx = (16.0f + 304.0f * (REAL)(j & 1)) * RenderSettings.GeomScaleX;
            DrawVertsTEX1[1].sx = DrawVertsTEX1[2].sx = (16.0f + 304.0f * (REAL)((j & 1) + 1)) * RenderSettings.GeomScaleX;

            DrawVertsTEX1[0].sy = DrawVertsTEX1[1].sy = (16.0f + 100.0f * (REAL)(j & 2)) * RenderSettings.GeomScaleY;
            DrawVertsTEX1[2].sy = DrawVertsTEX1[3].sy = (16.0f + 100.0f * (REAL)((j & 2) + 2)) * RenderSettings.GeomScaleY;

            DrawVertsTEX1[0].tu = DrawVertsTEX1[3].tu = 0.0f;
            DrawVertsTEX1[1].tu = DrawVertsTEX1[2].tu = 1.0f;

            DrawVertsTEX1[0].tv = DrawVertsTEX1[1].tv = 0.0f;
            DrawVertsTEX1[2].tv = DrawVertsTEX1[3].tv = 1.0f;

            DrawVertsTEX1[0].color = DrawVertsTEX1[1].color = DrawVertsTEX1[2].color = DrawVertsTEX1[3].color = 0xffffff;
            DrawVertsTEX1[0].rhw = DrawVertsTEX1[1].rhw = DrawVertsTEX1[2].rhw = DrawVertsTEX1[3].rhw = 1.0f;

            SET_TPAGE(TPAGE_MISC1 + (char)j);
            DRAW_PRIM(D3DPT_TRIANGLEFAN, FVF_TEX1, DrawVertsTEX1, 4, D3DDP_DONOTUPDATEEXTENTS);
        }

// draw revolt logo

        if (LoadingPic)
        {
            xstart = 128.0f * RenderSettings.GeomScaleX;
            ystart = 32.0f * RenderSettings.GeomScaleY;
            xsize = 384.0f * RenderSettings.GeomScaleX;
            ysize = 105.0f * RenderSettings.GeomScaleY;

            DrawVertsTEX1[0].sx = xstart;
            DrawVertsTEX1[0].sy = ystart;
            DrawVertsTEX1[0].color = 0xffffff;
            DrawVertsTEX1[0].rhw = 1.0f;
            DrawVertsTEX1[0].tu = 0.0f;
            DrawVertsTEX1[0].tv = 0.0f;

            DrawVertsTEX1[1].sx = xstart + xsize;
            DrawVertsTEX1[1].sy = ystart;
            DrawVertsTEX1[1].color = 0xffffff;
            DrawVertsTEX1[1].rhw = 1.0f;
            DrawVertsTEX1[1].tu = 1.0f;
            DrawVertsTEX1[1].tv = 0.0f;

            DrawVertsTEX1[2].sx = xstart + xsize;
            DrawVertsTEX1[2].sy = ystart + ysize;
            DrawVertsTEX1[2].color = 0xffffff;
            DrawVertsTEX1[2].rhw = 1.0f;
            DrawVertsTEX1[2].tu = 1.0f;
            DrawVertsTEX1[2].tv = 70.0f / 256.0f;

            DrawVertsTEX1[3].sx = xstart;
            DrawVertsTEX1[3].sy = ystart + ysize;
            DrawVertsTEX1[3].color = 0xffffff;
            DrawVertsTEX1[3].rhw = 1.0f;
            DrawVertsTEX1[3].tu = 0.0f;
            DrawVertsTEX1[3].tv = 70.0f / 256.0f;

            SET_RENDER_STATE(D3DRENDERSTATE_ALPHAREF, 240);
            SET_TPAGE(TPAGE_LOADING);
            DRAW_PRIM(D3DPT_TRIANGLEFAN, FVF_TEX1, DrawVertsTEX1, 4, D3DDP_DONOTUPDATEEXTENTS);
            SET_RENDER_STATE(D3DRENDERSTATE_ALPHAREF, AlphaRef);
        }

// draw pic spru

        DrawSpruBox(
            15.0f * RenderSettings.GeomScaleX,
            15.0f * RenderSettings.GeomScaleY,
            609.0f * RenderSettings.GeomScaleX,
            401.0f * RenderSettings.GeomScaleY,
            0, -1);

// begin text shit

        BeginTextState();

// show level info

        if (GameSettings.GameType == GAMETYPE_FRONTEND)
        {
            if (LoadingPic)
                DrawGameText(CENTRE_POS(TEXT_LOADINGFRONTEND), 224, 0xffffff, TEXT_TABLE(TEXT_LOADINGFRONTEND));
        }
        else if (CreditVars.State != CREDIT_STATE_INACTIVE)
        {
            DrawGameText(CENTRE_POS(TEXT_LOADINGCREDITS), 224, 0xffffff, TEXT_TABLE(TEXT_LOADINGCREDITS));
        }
        else
        {
            y = 160;

            DrawGameText(RIGHT_JUSTIFY_POS(TEXT_LOADSCREEN_GAMEMODE, 312), y, 0xffffff, TEXT_TABLE(TEXT_LOADSCREEN_GAMEMODE));
            if (GameSettings.GameType == GAMETYPE_CHAMPIONSHIP)
            {
                DrawGameText(328, y, 0x00ffff, TEXT_TABLE(TEXT_BRONZECUP + CupTable.CupType - 1));
                y += 32;

                DrawGameText(RIGHT_JUSTIFY_POS(TEXT_LOADSCREEN_STAGE, 312), y, 0xffffff, TEXT_TABLE(TEXT_LOADSCREEN_STAGE));
                sprintf(buf, "%ld / %ld", CupTable.RaceNum + 1, CupData[CupTable.CupType].NRaces);
                DrawGameText(328, y, 0x00ffff, buf);
            }
            else if (GameModeTextIndex[GameSettings.GameType] != TEXT_NONE)
            {
                DrawGameText(328, y, 0x00ffff, TEXT_TABLE(GameModeTextIndex[GameSettings.GameType]));
            }
            y += 32;

            if (GameSettings.GameType != GAMETYPE_TRAINING)
            {
                DrawGameText(RIGHT_JUSTIFY_POS(TEXT_LOADSCREEN_TRACK, 312), y, 0xffffff, TEXT_TABLE(TEXT_LOADSCREEN_TRACK));
                sprintf(buf, "%s %s%s", li->Name, GameSettings.Reversed ? TEXT_TABLE(TEXT_REVERSE_ABREV) : "", GameSettings.Mirrored ? TEXT_TABLE(TEXT_MIRROR_ABREV) : "");
                DrawGameText(328, y, 0x00ffff, buf);
                y += 32;
            }

            if (GameSettings.GameType != GAMETYPE_TRAINING && GameSettings.GameType != GAMETYPE_BATTLE)
            {
                DrawGameText(RIGHT_JUSTIFY_POS(TEXT_LENGTH, 304), y, 0xffffff, TEXT_TABLE(TEXT_LENGTH));
                DrawGameText(304, y, 0xffffff, ":");
                sprintf(buf, "%ldm", (long)li->Length);
                DrawGameText(328, y, 0x00ffff, buf);
                y += 32;
            }

            if (GameSettings.GameType == GAMETYPE_CHAMPIONSHIP || GameSettings.GameType == GAMETYPE_SINGLE || GameSettings.GameType == GAMETYPE_CLOCKWORK || GameSettings.GameType == GAMETYPE_MULTI)
            {
                DrawGameText(RIGHT_JUSTIFY_POS(TEXT_NUMLAPS, 304), y, 0xffffff, TEXT_TABLE(TEXT_NUMLAPS));
                DrawGameText(304, y, 0xffffff, ":");
                sprintf(buf, "%ld", GameSettings.NumberOfLaps);
                DrawGameText(328, y, 0x00ffff, buf);
                y += 32;
            }

            if (GameSettings.GameType != GAMETYPE_DEMO)
            {
                DrawGameText(RIGHT_JUSTIFY_POS(TEXT_CAR, 304), y, 0xffffff, TEXT_TABLE(TEXT_CAR));
                DrawGameText(304, y, 0xffffff, ":");
                DrawGameText(328, y, 0x00ffff, CarInfo[StartData.PlayerData[StartData.LocalPlayerNum].CarType].Name);
                y += 32;
            }

            if (GameSettings.GameType == GAMETYPE_SINGLE && GameSettings.Level < LEVEL_NCUP_LEVELS)
            {
                DrawGameText(RIGHT_JUSTIFY_POS(TEXT_LOADSCREEN_WONRACE, 312), y, 0xffffff, TEXT_TABLE(TEXT_LOADSCREEN_WONRACE));
                DrawGameText(328, y, 0x00ffff, IsSecretWonSingleRace(GameSettings.Level) ? TEXT_TABLE(TEXT_YES) : TEXT_TABLE(TEXT_NO));
                y += 32;
            }

            if (GameSettings.GameType == GAMETYPE_PRACTICE)
            {
                DrawGameText(RIGHT_JUSTIFY_POS(TEXT_LOADSCREEN_FOUNDSTAR, 312), y, 0xffffff, TEXT_TABLE(TEXT_LOADSCREEN_FOUNDSTAR));
                DrawGameText(328, y, 0x00ffff, IsSecretFoundPractiseStars(GameSettings.Level) ? TEXT_TABLE(TEXT_YES) : TEXT_TABLE(TEXT_NO));
                y += 32;
            }

            if (GameSettings.GameType == GAMETYPE_TRAINING)
            {
                DrawGameText(RIGHT_JUSTIFY_POS(TEXT_LOADSCREEN_STARSCOLLECTED, 312), y, 0xffffff, TEXT_TABLE(TEXT_LOADSCREEN_STARSCOLLECTED));
                sprintf(buf, "%ld %s %ld", StarList.NumFound, TEXT_TABLE(TEXT_OF), StarList.NumTotal);
                DrawGameText(328, y, 0x00ffff, buf);
                y += 32;
            }

            if (GameSettings.GameType == GAMETYPE_TRIAL && GameSettings.Level < LEVEL_NCUP_LEVELS)
            {
                if (!GameSettings.Mirrored && !GameSettings.Reversed)
                {
                    beattime = IsSecretBeatTimeTrial(GameSettings.Level);
                    time = li->ChallengeTimeNormal;
                }
                else if (!GameSettings.Mirrored && GameSettings.Reversed)
                {
                    beattime = IsSecretBeatTimeTrialReverse(GameSettings.Level);
                    time = li->ChallengeTimeReversed;
                }
                else if (GameSettings.Mirrored && !GameSettings.Reversed)
                {
                    beattime = IsSecretBeatTimeTrialMirror(GameSettings.Level);
                    time = li->ChallengeTimeNormal;
                }

                if (!beattime)
                    sprintf(buf, "%02d:%02d:%03d", MINUTES(time), SECONDS(time), THOUSANDTHS(time));
                else
                    sprintf(buf, "%s", TEXT_TABLE(TEXT_LOADSCREEN_BEATEN));

                DrawGameText(RIGHT_JUSTIFY_POS(TEXT_LOADSCREEN_CHALLENGETIME, 312), y, 0xffffff, TEXT_TABLE(TEXT_LOADSCREEN_CHALLENGETIME));
                DrawGameText(328, y, 0x00ffff, buf);
                y += 32;
            }
        }

// end scene

        D3Ddevice->EndScene();
    }

//$REVISIT: do we need to port this line?
//$REMOVED
//// wait for drawing finish
//
//    while(FrontBuffer->Flip(NULL, DDFLIP_NOVSYNC) == DDERR_WASSTILLDRAWING);
//$END_REMOVAL

// free bmp's

    for (i = 0 ; i < 4 ; i++)
    {
        FreeOneTexture(TPAGE_MISC1 + (char)i);
    }

// set loading pic

    LoadingPic = 1;

// create receive thread

#pragma message( "CPrince: re-evaluate Acclaim's use of threads." )
//$MODIFIED: larger stack size to fix the problem.
//    LoadThreadHandle = CreateThread(NULL, 0, LoadThread, NULL, 0, &LoadThreadID);
    LoadThreadHandle = CreateThread(NULL, 64*1024, LoadThread, NULL, 0, &LoadThreadID);
//$END_MODIFICATIONS
    if (!LoadThreadHandle)
    {
        ErrorDX(0, "Can't create load thread");
        return;
    }
}

/////////////////////////
// disable load thread //
/////////////////////////

void DisableLoadThread(void)
{

#pragma message( "CPrince: check safety of this thread hack" )
//$REMOVED
//// wait for dead
//
//    LoadThreadKill = TRUE;
//    while (LoadThreadKill);
//$END_REMOVAL

// close thread handle

    CloseHandle(LoadThreadHandle);

// set disabled flag

    LoadThreadEnabled = FALSE;
}

////////////////////////////////
// inc load thread unit count //
////////////////////////////////

void IncLoadThreadUnitCount(void)
{
    LoadThreadUnitCount++;
}

/////////////////
// load thread //
/////////////////

static unsigned long WINAPI LoadThread(void *param)
{
    LEVELINFO *li = GetLevelInfo(GameSettings.Level);
    D3DRECT rect;
    CRITICAL_SECTION cs;

// init critical section object

    InitializeCriticalSection(&cs);

// set priority

    HANDLE handle = GetCurrentThread();
    SetThreadPriority(handle, THREAD_PRIORITY_BELOW_NORMAL);

// calc loading bar clear rect

    rect.x1 = 0;
    rect.x2 = ScreenXsize;
    rect.y1 = (long)(432.0f * RenderSettings.GeomScaleY);
    rect.y2 = ScreenYsize;

#pragma message( "CPrince: investigate crash with render calls from multiple threads." )
//$REMOVED (tentative!! only render from main thread)
//// set viewport
//
//    SetViewport(0.0f, 0.0f, (float)ScreenXsize, (float)ScreenYsize, RenderSettings.GeomPers);
//$END_REMOVAL

// loop drawing loading bar

    while (!LoadThreadKill || LoadThreadPer < 100.0f)
    {

// pause?

        if (LoadThreadPause == 1)
        {
            LoadThreadPause = 2;
        }

        while (LoadThreadPause == 2);

// enter critical section

        EnterCriticalSection(&cs);

//$REMOVED2 (tentative!! only render from main thread)
//
//// flip / clear buffers
//
////$REMOVED        if (FullScreen)
////$REMOVED            FrontBuffer->Blt(NULL, BackBuffer, NULL, DDBLT_WAIT, NULL);
////$REMOVED        else
//            FlipBuffers();
//
////$MODIFIED
////        D3Dviewport->Clear2(1, &rect, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x000000, 1.0f, 0);
//        D3Ddevice->Clear( 1, // num rectangles
//                          &rect, // rectangle array
//                          D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_ZBUFFER, // flags
//                          0x000000,  // color value
//                          1.0f, // z value
//                          0 // stencil value
//                        );
//        //$CMP_NOTE: do we really need the rectangles here?  Or can we just clear the entire current viewport ???
////$END_MODIFICATIONS
//
//// begin scene
//
//        D3Ddevice->BeginScene();
//
//// draw bar + spru
//
//        UpdateTimeStep();
//
//        InitRenderStates();
//
//        destper = (float)(100 * LoadThreadUnitCount / LoadThreadUnits);
//        if (LoadThreadPer < destper)
//        {
//            LoadThreadPer += TimeStep * 100.0f;
//            if (LoadThreadPer > destper) LoadThreadPer = destper;
//        }
//
//        xstart = 16.0f * RenderSettings.GeomScaleX;
//        ystart = 432.0f * RenderSettings.GeomScaleY;
//        xsize = LoadThreadPer * 6.08f * RenderSettings.GeomScaleX;
//        ysize = 32.0f * RenderSettings.GeomScaleY;
//
//        col1 = 0x0000ff;
//        col2 = 0x0000ff;
//      col2 = (long)(LoadThreadPer * 2.55);
//      col2 = (255 - col) << 16 | col << 8;
//
//        DrawVertsTEX0[0].sx = xstart;
//        DrawVertsTEX0[0].sy = ystart;
//        DrawVertsTEX0[0].color = col1;
//        DrawVertsTEX0[0].rhw = 1.0f;
//
//        DrawVertsTEX0[1].sx = xstart + xsize;
//        DrawVertsTEX0[1].sy = ystart;
//        DrawVertsTEX0[1].color = col2;
//        DrawVertsTEX0[1].rhw = 1.0f;
//
//        DrawVertsTEX0[2].sx = xstart + xsize;
//        DrawVertsTEX0[2].sy = ystart + ysize;
//        DrawVertsTEX0[2].color = col2;
//        DrawVertsTEX0[2].rhw = 1.0f;
//
//        DrawVertsTEX0[3].sx = xstart;
//        DrawVertsTEX0[3].sy = ystart + ysize;
//        DrawVertsTEX0[3].color = col1;
//        DrawVertsTEX0[3].rhw = 1.0f;
//
//        SET_TPAGE(-1);
//        SET_RENDER_STATE(D3DRENDERSTATE_ALPHATESTENABLE, FALSE);
//        DRAW_PRIM(D3DPT_TRIANGLEFAN, FVF_TEX0, DrawVertsTEX0, 4, D3DDP_DONOTUPDATEEXTENTS);
//        SET_RENDER_STATE(D3DRENDERSTATE_ALPHATESTENABLE, TRUE);
//
//        DrawSpruBox(
//            15.0f * RenderSettings.GeomScaleX,
//            431.0f * RenderSettings.GeomScaleY,
//            609.0f * RenderSettings.GeomScaleX,
//            33.0f * RenderSettings.GeomScaleY,
//            0, 0);
//
//// begin text shit
//
//        BeginTextState();
//
//// draw filename
//
//        DrawGameText((640 - (strlen(LastFile) * 8)) / 2, 440, 0xffffff, LastFile);
//
//// end scene
//
//        D3Ddevice->EndScene();
//$END_REMOVAL2

// exit critical section

        LeaveCriticalSection(&cs);

// sleep for a while

        Sleep(10);
    }

// delete critical section

    DeleteCriticalSection(&cs);

// kill

    LoadThreadKill = FALSE;
    ExitThread(0);
    return 0;
}

///////////////////////////
// build sepia rgb table //
///////////////////////////

#define T0 0.0f
#define T1 0.37f
#define T2 1.0f

void CreateSepiaRGB(void)
{
    long i, lr, lg, lb;
    float r, g, b, rr, gg, bb;
    float bri;
    float aaa, bbb, ccc;

// alloc table

    SepiaRGB = (unsigned short*)malloc(32 * 32 * 32 * 2);
    if (!SepiaRGB)
    {
        return;
    }

// loop thru all rgb's

    i = 0;
    for (rr = 0 ; rr < 32 ; rr++) for (gg = 0 ; gg < 32 ; gg++) for (bb = 0 ; bb < 32 ; bb++)
    {
        bri = (rr * 0.299f + gg * 0.587f + bb * 0.114f) / 32.0f;

        aaa = ((bri - T1) * (bri - T2)) / ((T0 - T1) * (T0 - T2));
        bbb = ((bri - T0) * (bri - T2)) / ((T1 - T0) * (T1 - T2));
        ccc = ((bri - T0) * (bri - T1)) / ((T2 - T0) * (T2 - T1));

        r = aaa * 22.0f + bbb * 141.0f + ccc * 254.0f;
        g = aaa * 12.0f + bbb * 100.0f + ccc * 244.0f;
        b = aaa * 4.0f + bbb * 36.0f + ccc * 203.0f;

        lr = (long)r >> 3;
        lg = (long)g >> 3;
        lb = (long)b >> 3;

        SepiaRGB[i] = (unsigned short)(lb | lg << 5 | lr << 10);

        i++;
    }
}

//////////////////////////
// free sepia rgb table //
//////////////////////////

void KillSepiaRGB(void)
{
    free(SepiaRGB);
    SepiaRGB = NULL;
}

/* $REMOVED_UNREACHABLE
//////////////////////////////////////////////////
// run background tasks - game is not the focus //
//////////////////////////////////////////////////

static void RunBackgroundTasks(void)
{
    int status;
    PLAYER *player;

// waiting to spawn track editor

    if (gTrackEditorSpawnState == TRACKEDIT_SPAWN)
    {
        char langString[32];
        sprintf(langString, "-L%1d", LANG_GetLanguage());

        SetCurrentDirectory(".\\Editor");
        if (FullScreen) {
            status = _spawnl( _P_NOWAIT, "TrackEdit.exe", "TrackEdit.exe", langString, NULL);
        } else {
            status = _spawnl( _P_NOWAIT, "TrackEdit.exe", "TrackEdit.exe", "-window", langString, NULL);
        }
        SetCurrentDirectory("..");

        if (status != -1) {
            gTrackEditorSpawnState = TRACKEDIT_SPAWNED;
        }
    }

// waiting for track editor window?

    if ((gTrackEditorSpawnState == TRACKEDIT_SPAWNED) && FindWindow("Render Window", "Re-Volt Track Editor"))
    {
        gTrackEditorSpawnState = TRACKEDIT_SPAWNED_ALIVE;
    }

#ifndef XBOX_NOT_YET_IMPLEMENTED
// if trackeditor has been spawned and killed, reinstate revolt

    if ((gTrackEditorSpawnState == TRACKEDIT_SPAWNED_ALIVE) && !FindWindow("Render Window", "Re-Volt Track Editor")) 
    {
        ShowWindow(hwnd, SW_RESTORE);
    }
#endif // ! XBOX_NOT_YET_IMPLEMENTED

// update misc timers if not multiplayer

    if (!IsMultiPlayer())
    {

// get time diff

        UpdateTimeStep();

// countdown time

        if (CountdownTime)
        {
            CountdownEndTime += RealTimerDiff;
        }
        else
        {

// total race time

            TotalRaceStartTime += RealTimerDiff;

// player lap start times

            for (player = PLR_PlayerHead ; player ; player = player->next) if (player->type != PLAYER_NONE)
            {
                player->car.CurrentLapStartTime += RealTimerDiff;
            }
        }
    }
}
$END_REMOVAL */


/* $REMOVED_DONTNEED
/////////////////////
// .NFO file check //
/////////////////////

static void CheckNFO(void)
{
    HANDLE handle;
    WIN32_FIND_DATA data;

// search for .NFO files in current dir

    handle = FindFirstFile("*.nfo", &data);
    if (handle == INVALID_HANDLE_VALUE)
    {
        NFO = FALSE;
        FindClose(handle);
    }
}
$END_REMOVAL */

/* $REMOVED_DONTNEED
////////////////////////
// Registry ZZZ check //
////////////////////////

static void CheckRegistryZZZ(void)
{
    HKEY key;
    DWORD r;

// open registry key

    r = RegOpenKeyEx(REGISTRY_ROOT, "software\\zzzzzz", 0, KEY_ALL_ACCESS, &key);
    if (r == ERROR_SUCCESS)
    {
        RegistryZZZ = TRUE;
        RegCloseKey(key);
    }
}
$END_REMOVAL */

/* $REMOVED_DONTNEED
///////////////////////
// CD security check //
///////////////////////

static void CheckCD(void)
{
    HREDBOOK hr;
    DWORD i, r, drives, cdnum, namelen, flags;
    char cddrives[26][4];
    char virtualdrives[26];
    char virtualdriveflags[26];
    char buf[4];
    char file[128];
    char volume[128];
    FILE *fp;
    HKEY key;

// get list of 'Virtual CD' drives

    for (i = 0 ; i < 26 ; i++)
        virtualdriveflags[i] = 0;

    r = RegOpenKeyEx(REGISTRY_ROOT, "Software\\Logicraft\\Virtual CD-ROM", 0, KEY_ALL_ACCESS, &key);
    if (r == ERROR_SUCCESS)
    {
        virtualdrives[0] = 0;
        GET_REGISTRY_VALUE(key, "Default Mounts", virtualdrives, 26);
        RegCloseKey(key);

        for (i = 0 ; i < strlen(virtualdrives) ; i++)
        {
            virtualdriveflags[toupper(virtualdrives[i]) - 'A'] = TRUE;
        }
    }

// build list of CD drives

    drives = GetLogicalDrives();

    cdnum = 0;
    for (i = 0 ; i < 26 ; i++, drives >>= 1)
    {

// skip if not a logical drive

        if (!(drives & 1))
            continue;

// skip if a virtual CD drive

        if (virtualdriveflags[i])
            continue;

// skip if not CDROM

        sprintf(buf, "%c:\\", i + 'a');
        if (GetDriveType(buf) != DRIVE_CDROM)
            continue;

// skip if writable

        sprintf(file, "%s________", buf);
        fp = fopen(file, "wb");
        if (fp)
        {
            fclose(fp);
            remove(file);
            continue;
        }

// accept this drive

        strcpy(cddrives[cdnum++], buf);
    }

// search for Re-Volt CD

    SetErrorMode(SEM_FAILCRITICALERRORS);

    while (TRUE)
    {
        for (i = 0 ; i < cdnum ; i++)
        {

// check volume name

            if (!GetVolumeInformation(cddrives[i], volume, 128, NULL, &namelen, &flags, NULL, 0))
                continue;

            if (strcmp(volume, CdVolume))
                continue;

// found Re-Volt CD, save drive letter for redbook

            RedbookDeviceLetter = toupper(cddrives[i][0]);

// get redbook track count

//          hr = AIL_redbook_open_drive(RedbookDeviceLetter);
//          if (hr)
//          {
//              TracksCD = AIL_redbook_tracks(hr) - 1;
//              AIL_redbook_close(hr);
//          }

            TracksCD = REDBOOK_TRACK_NUM;

// get checksum from 'layout.bin'

//          sprintf(file, "%slayout.bin", cddrives[i]);
//          fp = fopen(file, "rb");
//          if (fp)
//          {
//              fseek(fp, -4, SEEK_END);
//              fread(&ChecksumCD, sizeof(long), 1, fp);
//              fclose(fp);
//          }

            ChecksumCD = ChecksumCRC;

// return OK

            return;
        }

// not found, report error + try again

        r = Box(NULL, "Please Insert the Re-Volt CD", MB_RETRYCANCEL);
        if (r == IDCANCEL)
        {
            exit(0);
        }
    }
}
$END_REMOVAL */
