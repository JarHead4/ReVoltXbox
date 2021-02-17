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
#include "EditObject.h"
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
#include "competition.h"
#include "InitPlay.h"
#include "Intro.h"
#include "obj_init.h"
#include "shareware.h"
#include "panel.h"
#include "pickup.h"
#include "credits.h"
#include "gamegauge.h"
#include "ui_menu.h"
#include "ui_MenuDraw.h"
#include "ui_MenuText.h"
#include "ui_TitleScreen.h"
#include "ui_StateEngine.h"
#include "ui_WaitingRoom.h"
#include "net_xonline.h"
#include "content.h"
#include "soundbank.h"
#include "SoundEffectEngine.h"
#include "MusicManager.h"
#include "dspimage.h"
#include "VoiceManager.h"

#ifdef ENABLE_STATISTICS
//$REVISIT: Statistics disabled for July Consumer Beta
#include "net_Statistics.h"
#endif // ENABLE_STATISTICS


#include "xbdm.h"  //$ADDITION(jedl) - for DmEnableGPUCounter

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

// Callback function signature for communicator-related events
VOID OnCommunicatorEvent( DWORD dwPort, VOICE_COMMUNICATOR_EVENT event, VOID* pContext );

// Callback function signature for voice data 
VOID OnVoicePacket( DWORD dwPort, DWORD dwSize, VOID* pvData, VOID* pContext );

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
#define VERSION_STRING_PCXL         "v0.01pcxl"
#define VERSION_STRING_CGW          "v0.01cgw"
#define VERSION_STRING_PCGAMER      "v0.01pcgamer"
#define VERSION_STRING_TNT2         "v0.01nvidia"
#define VERSION_STRING_ROLLINGDEMO  "v0.01"
#define VERSION_STRING_E3           "v0.01e3"
//$CPRINCE_MODIFIED
//#define VERSION_STRING_RELEASE        "v1.00"
#define VERSION_STRING_RELEASE      "v1.00atg"
//$END_MODIFICATIONS
#define VERSION_STRING_GAMESPOT     "v0.01gamespot"
#define VERSION_STRING_GAMECENTER   "v0.01gamecenter"
#define VERSION_STRING_IGN_PC       "v0.01ign.pc"
#define VERSION_STRING_ACCLAIM      "v0.01acclaim"
#define VERSION_STRING_MACDONALDS   "v0.01ziffdavies"
#define VERSION_STRING_CREATIVE     "v0.01creativelabs"

// registry key

#define REGISTRY_KEY_DEV            "software\\Acclaim\\Re-Volt Dev\\1.0"
#define REGISTRY_KEY_SHAREWARE      "software\\Acclaim\\Re-Volt Demo\\1.0"
#define REGISTRY_KEY_E3             "software\\Acclaim\\Re-Volt E3\\1.0"
#define REGISTRY_KEY_RELEASE        "software\\Acclaim\\Re-Volt\\1.0"
#define REGISTRY_KEY_CREATIVE       "software\\Acclaim\\Re-Volt Creative\\1.0"

// version shit

#if defined(VERSION_TYPE_DEV)

VERSION Version = VERSION_DEV;
char VersionString[] = VERSION_STRING_DEV;
char RegistryKey[] = REGISTRY_KEY_DEV;
MULTIPLAYER_VERSION MultiplayerVersion = MULTIPLAYER_VERSION_DEV;

#elif defined(VERSION_TYPE_SHAREWARE)

VERSION Version = VERSION_SHAREWARE;
char VersionString[] = VERSION_STRING_MACDONALDS;
char RegistryKey[] = REGISTRY_KEY_SHAREWARE;
MULTIPLAYER_VERSION MultiplayerVersion = MULTIPLAYER_VERSION_MACDONALDS;

#elif defined(VERSION_TYPE_E3)

VERSION Version = VERSION_E3;
char VersionString[] = VERSION_STRING_E3;
char RegistryKey[] = REGISTRY_KEY_E3;
MULTIPLAYER_VERSION MultiplayerVersion = MULTIPLAYER_VERSION_E3;

#elif defined(VERSION_TYPE_RELEASE)

VERSION Version = VERSION_RELEASE;
char VersionString[] = VERSION_STRING_RELEASE;
char RegistryKey[] = REGISTRY_KEY_RELEASE;
MULTIPLAYER_VERSION MultiplayerVersion = MULTIPLAYER_VERSION_RELEASE;

#elif defined(VERSION_TYPE_CREATIVE)

VERSION Version = VERSION_CREATIVE;
char VersionString[] = VERSION_STRING_CREATIVE;
char RegistryKey[] = REGISTRY_KEY_CREATIVE;
MULTIPLAYER_VERSION MultiplayerVersion = MULTIPLAYER_VERSION_CREATIVE;

#else

#error Invalid Version Type

#endif

// Set this to TRUE for rolling demo...
bool GoStraightToDemo = FALSE;

// game mode text

long GameModeTextIndex[] = {
    TEXT_NONE,
    TEXT_TIMETRIAL,
    TEXT_SINGLERACE,
    TEXT_CLOCKWORKRACE,
    TEXT_MULTIPLAYER,
    TEXT_REPLAY,
    TEXT_BATTLETAG,
    TEXT_CHAMPIONSHIP,
    TEXT_PRACTICE,
    TEXT_TRAINING,
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
char Software = FALSE;
char WheelRenderType = 0;
char NoGamma = FALSE;
//$REMOVEDchar AppRestore = FALSE;
char g_bQuitGame = FALSE;
//$REMOVEDchar FullScreen = TRUE;
unsigned long FrameCount, FrameCountLast, FrameTime, FrameTimeLast, FrameRate;
long EditMode = 0;
long AlphaRef = 128;
long NoUser = FALSE;
long Skybox = FALSE;
long ForceDrawDevice = -1;
long ForceScreenWidth = 0;
long ForceScreenHeight = 0;
long ForceScreenBpp = 0;
long ForceDoubleBuffer = FALSE;
long ForceTextureBpp = -1;
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

void QuitGame()
{
    Assert(!"Not valid to exit Xbox main.  Debug code that called QuitGame.");
    g_bQuitGame = TRUE;
}

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

// rgb

        if (!strcmp(__argv[i], "-rgb"))
        {
            Software = 1;
            continue;
        }

// mmx

        if (!strcmp(__argv[i], "-mmx"))
        {
            Software = 2;
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

// go straight into DEMO mode

        if (!strcmp(__argv[i], "-gogodemo"))
        {
            GoStraightToDemo = TRUE;
            continue;
        }

// set current dir to exe dir for gay installshield

        if (!strcmp(__argv[i], "-setdir"))
        {
            char dir[MAX_PATH];

            strncpy(dir, __argv[0], MAX_PATH);
            for (i = strlen(dir) ; i ; i--)
            {
                if (dir[i] == '\\')
                {
                    dir[i] = 0;
                    break;
                }
            }

            SetCurrentDirectory(dir);

            continue;
        }

// force draw device

        if (!strcmp(__argv[i], "-device"))
        {
            ForceDrawDevice = atol(__argv[++i]);
            continue;
        }

// force resolution

        if (!strcmp(__argv[i], "-res"))
        {
            ForceScreenWidth = atol(__argv[++i]);
            ForceScreenHeight = atol(__argv[++i]);
            ForceScreenBpp = atol(__argv[++i]);
            continue;
        }

// force double buffering

        if (!strcmp(__argv[i], "-doublebuffer"))
        {
            ForceDoubleBuffer = TRUE;
            continue;
        }

// force texture bpp

        if (!strcmp(__argv[i], "-tex"))
        {
            ForceTextureBpp = (atol(__argv[++i]) == 24);
            continue;
        }

// game gauge

        if (!strcmp(__argv[i], "-gamegauge"))
        {
            GAME_GAUGE = TRUE;
            continue;
        }
    }
#endif // ! XBOX_NOT_YET_IMPLEMENTED

//$REMOVED
//// revolt already running?
//
//    if (FindWindow(WinName, WinName) && FullScreen)
//    {
//        DumpMessage(NULL, "Revolt is already running!");
//        return FALSE;
//    }
//$END_REMOVAL

// init log file?
//$TODO - Error logging stuff should be turned off for release builds

//$MODIFIED
//    char path[MAX_PATH - 16];
//
//    if (!GetTempPath(MAX_PATH - 16, path))
//        sprintf(path, "c:\\");
//
//    sprintf(DBG_TempPath, "%srevolt.log", path);
    sprintf(DBG_TempPath, "D:\\revolt.log");
//$END_MODIFICATIONS
    DBG_LogFile = DBG_TempPath;
    InitLogFile();

//$MODIFIED - Direct3D initialization is done before everything else, so we can
//            quickly display a splash screen
//    if (!InitDX())
//    {
//        QuitGame();
//    }

    if( !InitD3D( 640, 480, 32, XBOX_UNUSED_PARAM ) )
    {
        QuitGame();
    }
//$END_MODIFICATION

//$ADDITION - initialize framerate tracker
    FrameRate_Init();
//$END_ADDITION

//$REMOVED
//// Initialize COM library
//
//    r = CoInitialize(NULL);
//    if (r != S_OK)
//    {
//        DumpMessage(NULL, "Can't initialize COM library!");
//        return FALSE;
//    }
//$END_REMOVAL

// Initialize title screen options vars (must be before GetRegistrySettings)

    StartInitTitleScreenVars();
    InitPickupWeightTables();

// Initialise the credits
//$MODIFIED - moved InitCreditEntries() call to inside the credit handling code
//  InitCreditEntries();
    InitCreditStateInactive();

// get registry settings

    InitGameSettings();
    GetRegistrySettings();

//$MODIFIED - This was moved from InitD3D(), so that we can use the values
//            from the registry
    SetGamma( RegistrySettings.Brightness, RegistrySettings.Contrast );

//$TODO(cprince): see whether it goes deeper than just these calls; maybe some checks elsewhere...
//$REMOVED
//
//// get CRC checksum
//
//    ChecksumCRC = GetFileChecksum(__argv[0], FALSE, TRUE);
//
//// CD security?
//
//    #if CHECK_CD
//
//    CheckCD();
//    CheckNFO();
//    CheckRegistryZZZ();
//
//    #else
//$END_REMOVAL

//$MODIFIED
//    ChecksumCD = ChecksumCRC;
    ChecksumCD = ChecksumCRC = 0;
//$END_MODIFICATIONS
    TracksCD = REDBOOK_TRACK_NUM;
    NFO = FALSE;
    RegistryZZZ = TRUE;

//$REMOVED
//    #endif
//$END_REMOVAL    

//$REMOVED
//// init window
//
//    if (!InitWin(hThisInst, nWinMode))
//        return FALSE;
//$END_REMOVAL

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

    // $MD: added reading of car packages
    if (!ReadAllCarPackagesMultiple())
    {
        return;
    }
    
    CalcCarStats();

    // $MD: added reading of car keys
    if (!ReadAllCarKeysMultiple())
    {
        return;
    }

    if(g_ContentManager.OwnCorruptContent())
    {
        g_bHadCorruptContent = TRUE;
        g_ContentManager.DeleteCorruptContent();
    }

//$REMOVED
//// init lobby
//
//    LobbyInit();  HEY -- BUFFERS GET ALLOCATED VIA HERE! (in LobbyConnect)
//$END_REMOVAL

//$REMOVED
//// check exe
//
//    if (!Lobby)
//    {
//        char badname[] = "re-volt\\pc\\game\\revolt.exe";
//
//        if (!stricmp(badname, __argv[0] + strlen(__argv[0]) - strlen(badname)))
//        {
//            DumpMessage("Nope!", "Run Revolt with the batch file!!!");
//            return;  //$MODIFIED: was "return FALSE"
//        }
//    }
//$END_REMOVAL

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

//$REMOVED
//// get all available draw devices
//
//    GetDrawDevices();
//$END_REMOVAL

// init DX misc

    XBInput_InitControllers();  //ADDITION(mwetzel) - Initialize input 

// init sound system

#ifdef OLD_AUDIO
    InitSound(RegistrySettings.SfxChannels);
#else
    g_SoundEngine.Initialize();
    g_MusicManager.Initialize();
    g_MusicManager.SetVolume( -600 );
#endif    

    // Need a pointer to DSound - the sound bank
    // creates one and holds on to it, but we 
    // can get one this way, too.  Ditto for the DSP
    // image desc
    LPDIRECTSOUND8 pDSound;
    DirectSoundCreate( NULL, &pDSound, NULL );
    extern LPDSEFFECTIMAGEDESC g_pDSPImageDesc;

    VOICE_MANAGER_CONFIG VoiceCfg;
    VoiceCfg.dwFirstSRCEffectIndex      = Graph1_SRCforheadphone1;
    VoiceCfg.dwMaxRemotePlayers         = MAX_NUM_PLAYERS;
    VoiceCfg.dwNumBuffers               = 4;
    VoiceCfg.dwQueueResetThreshold      = 20;
    VoiceCfg.dwVoicePacketTime          = 40;
    VoiceCfg.dwVoiceSamplingRate        = 8000;
    VoiceCfg.pCallbackContext           = NULL;
    VoiceCfg.pDSound                    = pDSound;
    VoiceCfg.pEffectImageDesc           = g_pDSPImageDesc;
    VoiceCfg.pfnCommunicatorCallback    = OnCommunicatorEvent;
    VoiceCfg.pfnVoiceDataCallback       = OnVoicePacket;
    g_VoiceManager.Initialize( &VoiceCfg );

    // Disable all voice communicators until a player signs in
    //$REVISIT: We'll need to make sure this interacts well with
    // system link play and the voice mask options menu
    for( DWORD i = 0; i < XGetPortCount(); i++ )
    {
        g_VoiceManager.EnableCommunicator( i, FALSE );
    }

    pDSound->Release();

//$REMOVED
//// check for legal IP
//
//    #if CHECK_IP
//    if (!CheckLegalIP())
//    {
//        DumpMessage(NULL, "Illegal copy of Revolt!");
//        QuitGame();
//    }
//    #endif
//$END_REMOVAL

// init function keys

    InitFunctionKeys();
    AddAllFunctionKeys();

// init Start data

    InitStartData();

// create sepia table

    CreateSepiaRGB();

// set start event

    SET_EVENT(Go);

// main loop

    while (!g_bQuitGame)
    {
//$REMOVED
//// message?
//
//        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
//        {
//            if (msg.message == WM_QUIT)
//            {
//                QuitGame();
//            }
//            else
//            {
//                TranslateMessage(&msg);
//                DispatchMessage(&msg);
//            }
//        }
//
//
//        if (AppActive) 
//        {
//$END_REMOVAL

// handle current event

            if (Event != NULL) 
            {
                Event();
            }
//$REMOVED
//        } 
//        else 
//        {
//
//// background tasks
//
//            RunBackgroundTasks();
//        }
//$END_REMOVAL
    }


    // on xbox we should not quit.
    Assert(!"What 'Event' set g_bQuitGame?");

// kill sepia table

    KillSepiaRGB();

// release DX misc

//$REMOVED    DD->RestoreDisplayMode();
    FreeTextures();
//$REMOVED(mwetzel)    KillInput();
    ReleaseD3D();
//$REMOVED    ReleaseDX();

// kill direct play
//$TODO: fix this comment

//$REMOVED    KillNetwork();

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

//$REMOVED
//// free lobby
//
//    LobbyFree();
//$END_REMOVAL

//$REMOVED
//// free COM library
//
//    CoUninitialize();
//$END_REMOVAL

// See if we have forgotten to release anything

#if USE_DEBUG_ROUTINES
    CheckMemoryAllocation();
#endif

//$REMOVED
//// return
//
//    return msg.wParam;
//$END_REMOVAL
}

/* $REMOVED
//////////////////////////////////////
// define win class, create window  //
// return TRUE if sucessful         //
//////////////////////////////////////

bool InitWin(HINSTANCE hThisInst, int nWinMode)
{
    long bx, by, cy;

// define / register windows class

    wcl.style = CS_HREDRAW | CS_VREDRAW;
    wcl.lpfnWndProc = WindowFunc;
    wcl.hInstance = hThisInst;
    wcl.lpszClassName = WinName;

    wcl.hIcon = LoadIcon(hThisInst, MAKEINTRESOURCE(IDI_ICON1));
    wcl.hCursor = LoadCursor(NULL, IDC_CROSS);
    wcl.lpszMenuName = NULL;

    wcl.cbClsExtra = 0;
    wcl.cbWndExtra = 0;
    wcl.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);

    if (!RegisterClass(&wcl)) return FALSE;

// create / show a window

    if (FullScreen)
    {
        hwnd = CreateWindow(WinName, WinName, WS_POPUP,
            0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN),
            NULL, NULL, hThisInst, NULL);
    }
    else
    {
        bx = GetSystemMetrics(SM_CXSIZEFRAME);
        by = GetSystemMetrics(SM_CYSIZEFRAME);
        cy = GetSystemMetrics(SM_CYCAPTION);

        hwnd = CreateWindow(WinName, WinName, WS_OVERLAPPED | WS_CAPTION | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX,
            (GetSystemMetrics(SM_CXSCREEN) - RegistrySettings.ScreenWidth) / 2 - bx, (GetSystemMetrics(SM_CYSCREEN) - RegistrySettings.ScreenHeight) / 2 - by - cy / 2,
            RegistrySettings.ScreenWidth + bx + bx, RegistrySettings.ScreenHeight + by + by + cy,
            NULL, NULL, hThisInst, NULL);
    }

    if (!hwnd) return FALSE;

    ShowWindow(hwnd, nWinMode);
    UpdateWindow(hwnd);
    SetCursor(NULL);

// return ok

    return TRUE;
}
$END_REMOVAL */

/////////////////////////
// display message box //
/////////////////////////

void DumpMessage(const char *title, const char *mess)  //$MODIFIED: renamed from Box(), removed 3rd param "long flag", and return type was "long"
{
//$MODIFIED
//    long ret;
//
//// disable load thread?
//
//    if (LoadThreadEnabled)
//    {
//        LoadThreadPause = 1;
//        while (LoadThreadPause == 1);
//    }
//
//// flip to GDI surface
//
//    if (DD)
//    {
//        DD->FlipToGDISurface();
//    }
//
//// box
//
//    ret = MessageBox(hwnd, mess, title, flag);
//
//// enable load thread
//
//    if (LoadThreadEnabled)
//    {
//        LoadThreadPause = 0;
//    }
//
//// return result
//
//    return ret;

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
//$END_MODIFICATIONS
}

void DumpMessageVar( CHAR* title, CHAR* format, ... )
{
    va_list arglist;
    CHAR    string[MAX_PATH];

    va_start( arglist, format );
    _vsnprintf( string, sizeof(string), format, arglist );
    va_end( arglist );

    DumpMessage( title, string );
}

/* $REMOVED_UNREACHABLE
////////////////////////////////
// display message box number //
////////////////////////////////

long BoxNum(long num)
{
    char buf[32];

    sprintf(buf, "%ld", num);
    return Box(NULL, buf, MB_OK);
}
*/

//////////////////////////////
// display message box text //
//////////////////////////////

//$REMOVED - changed BoxText(xxx) calls to DumpMessage(NULL,xxx)
//long BoxText(char *text)
//{
//    return DumpMessage(NULL, text);
//}
//$END_REMOVAL

/* $REMOVED_UNREACHABLE
////////////////////////////
// wait for 'count' vbl's //
////////////////////////////

void Vblank(char count)
{
    for ( ; count ; count--)
    {
        while (DD->WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN, NULL) != DD_OK);
    }
}
$END_REMOVAL */

/* $REMOVED
/////////////////////
// Win95 callbacks //
/////////////////////

LRESULT CALLBACK WindowFunc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    HANDLE handle;

// handle message

    switch (message)
    {

// set AppActive

        case WM_ACTIVATEAPP:
            AppActive = wParam;
            AppRestore = wParam;

            handle = GetCurrentProcess();

            if (AppActive)
            {
                SetPriorityClass(handle, NORMAL_PRIORITY_CLASS);

                // Unset trackedit state flags if the user killed it (the bastard)
                if ((gTrackEditorSpawnState == TRACKEDIT_SPAWNED_ALIVE) && !FindWindow("Render Window", "Re-Volt Track Editor")) 
                {
                    FindUserLevels();
                    gTrackEditorSpawnState = TRACKEDIT_NOT_SPAWNED;
                }
            }
            else
            {
                SetPriorityClass(handle, IDLE_PRIORITY_CLASS);

                PauseAllSfx();
                SetSafeAllJoyForces();

                if (!GameSettings.Paws && GameSettings.Level != LEVEL_FRONTEND)
                {
                    GameSettings.Paws = TRUE;
                    g_pMenuHeader->ClearMenuHeader();
                    g_pMenuHeader->SetNextMenu( &Menu_InGame);
                }
            }

            return TRUE;

// no cursor

        case WM_SETCURSOR:
            if (FullScreen)
            {
                SetCursor(NULL);
                return TRUE;
            }
            else
            {
                SetCursor(wcl.hCursor);
                break;
            }

// terminating

        case WM_DESTROY:
            PostQuitMessage(0);

            return TRUE;
    }

// default windows processing

    return DefWindowProc(hwnd, message, wParam, lParam);
}
$END_REMOVAL */

/////////////
// go game //
/////////////

void Go(void)
{

// load english language if game gauge, lobby launch, or not using languages

#ifndef XBOX_DISABLE_NETWORK
    if (GAME_GAUGE || Lobby || !UseLanguages)
#else
    if (GAME_GAUGE || !UseLanguages) //$CPRINCE MODIFIED VERSION
#endif
    {
        //$MODIFIED: Modified to load a language based on the dash settings
        gTitleScreenVars.Language = LANGUAGE_ENGLISH;

        switch( XGetLanguage() )
        {
            case XC_LANGUAGE_JAPANESE:
                gTitleScreenVars.Language = LANGUAGE_JAPANESE;
                g_bUseHardcodedStrings = FALSE;
                break;
            case XC_LANGUAGE_GERMAN:
                gTitleScreenVars.Language = LANGUAGE_GERMAN;
                g_bUseHardcodedStrings = FALSE;
                break;
            case XC_LANGUAGE_FRENCH:
                gTitleScreenVars.Language = LANGUAGE_FRENCH;
                g_bUseHardcodedStrings = FALSE;
                break;
            case XC_LANGUAGE_SPANISH:
                gTitleScreenVars.Language = LANGUAGE_SPANISH;
                g_bUseHardcodedStrings = FALSE;
                break;
            case XC_LANGUAGE_ITALIAN:
                gTitleScreenVars.Language = LANGUAGE_ITALIAN;
                g_bUseHardcodedStrings = FALSE;
                break;
            case XC_LANGUAGE_PORTUGUESE:
                gTitleScreenVars.Language = LANGUAGE_PORTUGUESE;
                g_bUseHardcodedStrings = FALSE;
                break;
            case XC_LANGUAGE_KOREAN:
                gTitleScreenVars.Language = LANGUAGE_KOREAN;
                g_bUseHardcodedStrings = FALSE;
                break;
            case XC_LANGUAGE_TCHINESE:
                gTitleScreenVars.Language = LANGUAGE_CHINESE;
                g_bUseHardcodedStrings = FALSE;
                break;
        }
        LANG_LoadStrings( (LANGUAGE)gTitleScreenVars.Language );
    }

// set event

    if (GAME_GAUGE)
        SET_EVENT(SetupGameGaugeDemo);
#ifndef XBOX_DISABLE_NETWORK
    else if ((Version == VERSION_SHAREWARE || Version == VERSION_E3 || Version == VERSION_RELEASE || Version == VERSION_CREATIVE) && !Lobby)
#else
    else if ((Version == VERSION_SHAREWARE || Version == VERSION_E3 || Version == VERSION_RELEASE || Version == VERSION_CREATIVE))  //$CPRINCE MODIFIED VERSION
#endif
        SET_EVENT(SetupSharewareIntro);
    // $BEGIN_TEMPORARY(jedl) - jump to saved position and skip front-end UI
    else if (RegistrySettings.bGraphicsDebug
             && RegistrySettings.PositionSave)
    {
        // $TODO: Get rid of dependence of in-game menus from front-end menus.
        // We currently have to load up the whole frontend. Yuck.
        GoTitleScreen();  // initialize title screen junk. 
        ReleaseTitleScreen();   // clean up title screen junk
        g_pActiveStateEngine = NULL;
        
        // $TODO: get rid of all the shadowed junk between RenderSettings, GameSettings, and gTitleScreenVars
        RenderSettings.Env      = gTitleScreenVars.shinyness   = RegistrySettings.EnvFlag     ;
        RenderSettings.Light    = gTitleScreenVars.lights      = RegistrySettings.LightFlag   ;
        RenderSettings.Instance = gTitleScreenVars.instances   = RegistrySettings.InstanceFlag;
        RenderSettings.Mirror   = gTitleScreenVars.reflections = RegistrySettings.MirrorFlag  ;
        RenderSettings.Shadow   = gTitleScreenVars.shadows     = RegistrySettings.ShadowFlag  ;
        RenderSettings.Skid     = gTitleScreenVars.skidmarks   = RegistrySettings.SkidFlag    ;

        GameSettings.AllowPickups   = RegistrySettings.PickupFlag;
        GameSettings.AutoBrake      = RegistrySettings.AutoBrake;
        GameSettings.CarType        = RegistrySettings.CarType;
        GameSettings.DrawFollowView = FALSE;
        GameSettings.DrawRearView = FALSE;
        GameSettings.GameType = GAMETYPE_SINGLE;
        GameSettings.Mirrored       = gTitleScreenVars.mirror;
        GameSettings.NumberOfLaps   = RegistrySettings.NLaps;
        GameSettings.PlayMode       = gTitleScreenVars.playMode;
        GameSettings.RandomCars     = gTitleScreenVars.RandomCars;
        GameSettings.RandomTrack    = gTitleScreenVars.RandomTrack;
        GameSettings.Reversed       = gTitleScreenVars.reverse;
        
        // Init starting data
        InitStartData();
        StartData.GameType = GameSettings.GameType;
        StartData.Laps = GameSettings.NumberOfLaps;
        StartData.AllowPickups = GameSettings.AllowPickups;
        strncpy(StartData.LevelDir, RegistrySettings.LevelDir, MAX_PATH); // $TODO: why do we need StartData.LevelDir and RegistrySettings.LevelDir?
        StartData.LevelDir[MAX_PATH - 1] = 0; // make string is null terminated

        // Init cars
        DWORD iPlayer = 0;
        LONG car = RegistrySettings.CarType;
        AddPlayerToStartData(PLAYER_LOCAL, iPlayer, car, 0, 0, CTRL_TYPE_LOCAL, 0, RegistrySettings.PlayerName);
        for (iPlayer = 1; iPlayer < RegistrySettings.NCars; iPlayer++)
        {
            static CHAR *s_rFakePlayerNames[MAX_NUM_PLAYERS] = {
#ifdef SHIPPING  // different MAX_NUM_PLAYERS in shipping version
                "Me", "Angel", "Biff", "Cooper", "D-Day", "Eloise"
#else
                "Me", "Angel", "Biff", "Cooper", "D-Day", "Eloise", "Fickle", "Gunko", "Hoober", "Icicle", "Juke",
                "Killer", "Lucy", "Mop", "Nuke", "Oh Oh", "Paco", "Querp", "Rumble", "Stew", "Teaser", "Uncle Bob",
                "Violet", "Wailer" /* "X-boy", "Ynot", "Zeke" */
#endif
            };
            AddPlayerToStartData(PLAYER_CPU, iPlayer, car, 0, 0, CTRL_TYPE_CPU_AI, 0, s_rFakePlayerNames[iPlayer]);
        }
        RandomizeCPUCarType();

        // Setup game (which sets the event proc to the game loop)
        SetupGame();

        CountdownEndTime = TimerCurrent;    // get rid of 3..2..1..Go
        SetCameraAttached(CAM_MainCamera, PLR_LocalPlayer->ownobj, CAM_ATTACHED_INCAR); // jump camera to camera
        SetCameraFollow(CAM_MainCamera, PLR_LocalPlayer->ownobj, CAM_FOLLOW_BEHIND);    // move camera behind car
    }
    // $END_TEMPORARY(jedl)
    else
        SET_EVENT(GoTitleScreen);  //$NOTE:  eg, VERSION_DEV
}

/* $REMOVED_UNREACHABLE
/////////////////////
// setup front end //
/////////////////////

void GoFront(void)
{

// test linear problem solver??? (eh?)

#if DEBUG_SOLVER
    TestConjGrad();
#endif

//$REMOVED(jedl) - this should done just once at the beginning
//// init D3D
//
//    if (!InitD3D(DrawDevices[RegistrySettings.DrawDevice].DisplayMode[DisplayModeCount].Width, DrawDevices[RegistrySettings.DrawDevice].DisplayMode[DisplayModeCount].Height, DrawDevices[RegistrySettings.DrawDevice].DisplayMode[DisplayModeCount].Bpp, 0))
//    {
//        QuitGame();
//        return;
//    }
//$END_MODIFICATIONS

    PickTextureFormat();  //$RENAMED: was GetTextureFormat()
    InitTextures();
    InitFadeShit();
    SetupDxState();

    // load misc textures
//$MODIFIED
//    LoadMipTexture("gfx\\font.bmp", TPAGE_FONT, 256, 256, 0, 1, FALSE);
//    LoadMipTexture("gfx\\loading.bmp", TPAGE_LOADING, 256, 256, 0, 1, FALSE);
//    LoadMipTexture("gfx\\spru.bmp", TPAGE_SPRU, 256, 256, 0, 1, FALSE);
    LoadMipTexture("D:\\gfx\\font.bmp", TPAGE_FONT, 256, 256, 0, 1, FALSE);
    LoadMipTexture("D:\\gfx\\loading.bmp", TPAGE_LOADING, 256, 256, 0, 1, FALSE);
    LoadMipTexture("D:\\gfx\\spru.bmp", TPAGE_SPRU, 256, 256, 0, 1, FALSE);
//$END_MODIFICATIONS

// set geom vars

    RenderSettings.GeomPers = BaseGeomPers;
    SetNearFar(NEAR_CLIP_DIST, 4096.0f);
    SetViewport(0, 0, (float)ScreenXsize, (float)ScreenYsize, RenderSettings.GeomPers);

// setup main menu

//  LoadBitmap("gfx\\title.bmp", &TitleHbm);

    MenuCount = 0;
    SET_EVENT(MainMenu);
}
*/

//////////////////
// setup a game //
//////////////////

void SetupGame(void)
{
//$REMOVED(jedl;see below)    unsigned long w, h;

    // $ADDED jedl - save current registry settings to the fake registry.
    // Since we never exit the main loop, this would otherwise never be called.
    SetRegistrySettings();

#ifdef OLD_AUDIO
// set sound channels

    SetSoundChannels(RegistrySettings.SfxChannels);
#endif // OLD_AUDIO

// set the level number

    GameSettings.Level = GetLevelNum(StartData.LevelDir);
    if (GameSettings.Level == -1) {
        char buf[256];
        sprintf(buf, "Couldn't find level \"%s\"", StartData.LevelDir);
        DumpMessage(NULL,buf);
        QuitGame();
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

    if (RenderSettings.Sepia = (g_CreditVars.State != CREDIT_STATE_INACTIVE))
        InitFilmLines();

//$REMOVED(jedl) - this should done just once at the beginning
//// init D3D
//
//    w = ScreenXsize;
//    h = ScreenYsize;
//
//    if (!InitD3D(DrawDevices[RegistrySettings.DrawDevice].DisplayMode[DisplayModeCount].Width, DrawDevices[RegistrySettings.DrawDevice].DisplayMode[DisplayModeCount].Height, DrawDevices[RegistrySettings.DrawDevice].DisplayMode[DisplayModeCount].Bpp, 0))
//    {
//        QuitGame();
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
    g_pMenuHeader->ClearMenuHeader();

    // game game vars
    if (GAME_GAUGE)
    {
        SetupGameGaugeVars();
    }

    //$HACK: We're always telling the online APIs the user signed in on controller 0.
    AddOnlinePresenceFlag( 0, XONLINE_FRIENDSTATE_FLAG_PLAYING );
    RemoveOnlinePresenceFlag( 0, XONLINE_FRIENDSTATE_FLAG_JOINABLE );

#ifdef ENABLE_STATISTICS
//$REVISIT: Statistics disabled for July Consumer Beta
    // Do work specific to XOnline networking
    if( gTitleScreenVars.bUseXOnline )
    {
        // Updated Number of Started Races/Battles Statistics
        StatsLocalPlayersStartingMatch();
    }
#endif // ENABLE_STATISTICS

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

#ifdef OLD_AUDIO
    UpdateMusicVol(gTitleScreenVars.musicVolume);
    if (g_CreditVars.State == CREDIT_STATE_INACTIVE) {
        UpdateSfxVol(gTitleScreenVars.sfxVolume);
    } else {
        UpdateSfxVol(0);
    }
#else
    g_MusicManager.SetVolume( gTitleScreenVars.musicVolume );
    // TODO (JHarding): Need to implement global volume control
    // over sound effects 
#endif // OLD_AUDIO

#ifdef OLD_AUDIO
    // NOTE (JHarding): why didn't Acclaim start music in LoadTrackDataStageTwo,
    // for symmetry with UnloadTrackDataStageTwo?
// play MP3?

    if (strlen(CurrentLevelInfo.Mp3) && Version == VERSION_SHAREWARE)
    {
        PlayMP3(CurrentLevelInfo.Mp3);
    }

// play redbook?

//$REMOVED - play music in all versions
//    if (CurrentLevelInfo.RedbookStartTrack != -1 && CurrentLevelInfo.RedbookEndTrack != -1 && Version == VERSION_RELEASE)
//$END_REMOVAL
    {
        SetRedbookFade(REDBOOK_FADE_UP);
        if ((GameSettings.GameType == GAMETYPE_FRONTEND) && g_bShowWinLoseSequence) {
            if (CupTable.LocalPlayerPos > 3) {
                PlayRedbookTrack(REDBOOK_TRACK_LOSECUP, REDBOOK_TRACK_LOSECUP, FALSE);
            } else {
                PlayRedbookTrack(REDBOOK_TRACK_WINCUP, REDBOOK_TRACK_WINCUP, FALSE);
            }
        } else {
            if (g_CreditVars.State == CREDIT_STATE_INACTIVE) {
            if (RegistrySettings.MusicOn || GameSettings.Level == LEVEL_FRONTEND)
                PlayRedbookTrackRandom(CurrentLevelInfo.RedbookStartTrack, CurrentLevelInfo.RedbookEndTrack, TRUE);
            } else {
                PlayRedbookTrack(REDBOOK_TRACK_CREDITS, REDBOOK_TRACK_CREDITS, FALSE);
            }
        } 
    }
#endif // OLD_AUDIO

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

        case GAMETYPE_NETWORK_RACE:
        case GAMETYPE_NETWORK_BATTLETAG:
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
            QuitGame();
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

//$REMOVED(mwetzel)    ReadKeyboard();
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
    GameSettings.PlayMode = MODE_ARCADE;

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
    NextSyncMachine = 1;  //$MODIFIED(cprince): was 0 originally, but that represents server player, and don't want server to send sync to self
                          //$REVISIT(cprince): only used by server, so can/should probably put inside SetGameStarted (and remove from network.h)
    NextPositionTimer = DP_POSITION_TIME;
    MessageQueueSize = 0;
//$REMOVED
//    TotalDataSent = 0;
//$END_REMOVAL
    GlobalIDCounter = 0;
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
    GameSettings.PlayMode = MODE_ARCADE;
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
        sprintf(buf, "Can't open %s for CRC checksum", file);
        DumpMessage(NULL,buf);
        return 0;
    }

// calculate checksum

    crc = GetStreamChecksum(fp, appendedChecksum, binaryMode);
    if (crc == 0) {
        sprintf(buf, "Can't alloc memory for %s CRC checksum", file);
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
        c = (char)toupper(string[iChar]);

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
    WCHAR buf[128];
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

    if (GAME_GAUGE)
        LoadingPic = 1;

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
                          D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, // flags
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
//$MODIFIED (mwetzel) - To keep in Xbox safe area
            DrawVertsTEX1[0].sx = DrawVertsTEX1[3].sx = (48.0f + 272.0f * (REAL)(j & 1)) * RenderSettings.GeomScaleX;
            DrawVertsTEX1[1].sx = DrawVertsTEX1[2].sx = (48.0f + 272.0f * (REAL)((j & 1) + 1)) * RenderSettings.GeomScaleX;

            DrawVertsTEX1[0].sy = DrawVertsTEX1[1].sy = (36.0f + 102.0f * (REAL)(j & 2)) * RenderSettings.GeomScaleY;
            DrawVertsTEX1[2].sy = DrawVertsTEX1[3].sy = (36.0f + 102.0f * (REAL)((j & 2) + 2)) * RenderSettings.GeomScaleY;
//$END_MODIFICATIONS
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
//$MODIFIED (mwetzel) - To keep in Xbox safe area
        DrawSpruBox( 48.0f * RenderSettings.GeomScaleX,
                     36.0f * RenderSettings.GeomScaleY,
                    544.0f * RenderSettings.GeomScaleX,
                    408.0f * RenderSettings.GeomScaleY,
                        0, -1);
//$END_MODIFICATIONS

// begin text shit

        BeginTextState();

// show level info

        if (GameSettings.GameType == GAMETYPE_FRONTEND)
        {
            if (LoadingPic)
            {
                g_pFont->SetScaleFactors( 2.0f, 2.0f );
                g_pFont->DrawText( 320, 240, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_LOADINGFRONTEND), XBFONT_CENTER_X|XBFONT_CENTER_Y );
                g_pFont->SetScaleFactors( 1.0f, 1.0f );
            }
        }
        else if (g_CreditVars.State != CREDIT_STATE_INACTIVE)
        {
            g_pFont->SetScaleFactors( 2.0f, 2.0f );
            g_pFont->DrawText( 320, 224, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_LOADINGCREDITS), XBFONT_CENTER_X|XBFONT_CENTER_Y );
            g_pFont->SetScaleFactors( 1.0f, 1.0f );
        }
        else if (GAME_GAUGE)
        {
            g_pFont->SetScaleFactors( 2.0f, 2.0f );
            g_pFont->DrawText( 320, 224, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_LOADINGGAMEGAUGE), XBFONT_CENTER_X|XBFONT_CENTER_Y );
            g_pFont->SetScaleFactors( 1.0f, 1.0f );
        }
        else if (GameSettings.GameType == GAMETYPE_DEMO)
        {
            g_pFont->SetScaleFactors( 2.0f, 2.0f );
            g_pFont->DrawText( 320, 224, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_LOADINGDEMO), XBFONT_CENTER_X|XBFONT_CENTER_Y );
            g_pFont->SetScaleFactors( 1.0f, 1.0f );
        }
        else
        {
            y = 160;

            g_pFont->DrawText( 312, y, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_LOADSCREEN_GAMEMODE), XBFONT_RIGHT );
            if (GameSettings.GameType == GAMETYPE_CHAMPIONSHIP)
            {
                g_pFont->DrawText(328, y, MENU_TEXT_RGB_CHOICE, TEXT_TABLE(TEXT_BRONZECUP + CupTable.CupType - 1));
                y += 32;

                g_pFont->DrawText( 312, y, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_LOADSCREEN_STAGE), XBFONT_RIGHT );
                swprintf(buf, L"%ld / %ld", CupTable.RaceNum + 1, CupData[CupTable.CupType].NRaces);
                g_pFont->DrawText(328, y, MENU_TEXT_RGB_CHOICE, buf);
            }
            else if (GameModeTextIndex[GameSettings.GameType] != TEXT_NONE)
            {
                g_pFont->DrawText(328, y, MENU_TEXT_RGB_CHOICE, TEXT_TABLE(GameModeTextIndex[GameSettings.GameType]));
            }
            y += 32;

            if (GameSettings.GameType != GAMETYPE_TRAINING)
            {
                g_pFont->DrawText( 312, y, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_LOADSCREEN_TRACK), XBFONT_RIGHT );
                swprintf(buf, L"%s %s%s", li->strName, GameSettings.Reversed ? TEXT_TABLE(TEXT_REVERSE_ABREV) : L"", GameSettings.Mirrored ? TEXT_TABLE(TEXT_MIRROR_ABREV) : L"");
                g_pFont->DrawText(328, y, MENU_TEXT_RGB_CHOICE, buf);
                y += 32;
            }

            if (GameSettings.GameType != GAMETYPE_TRAINING && GameSettings.GameType != GAMETYPE_NETWORK_BATTLETAG)
            {
                g_pFont->DrawText(304, y, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_LENGTH), XBFONT_RIGHT );
                g_pFont->DrawText(304, y, MENU_TEXT_RGB_NORMAL, L":");
                swprintf(buf, L"%ld%s", (long)li->Length, TEXT_TABLE(TEXT_METERS_ABREV));
                g_pFont->DrawText(328, y, MENU_TEXT_RGB_CHOICE, buf);
                y += 32;
            }

            if (GameSettings.GameType == GAMETYPE_CHAMPIONSHIP || GameSettings.GameType == GAMETYPE_SINGLE || GameSettings.GameType == GAMETYPE_CLOCKWORK || GameSettings.GameType == GAMETYPE_NETWORK_RACE)
            {
                g_pFont->DrawText(304, y, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_NUMLAPS), XBFONT_RIGHT );
                g_pFont->DrawText(304, y, MENU_TEXT_RGB_NORMAL, L":");
                swprintf(buf, L"%ld", GameSettings.NumberOfLaps);
                g_pFont->DrawText(328, y, MENU_TEXT_RGB_CHOICE, buf);
                y += 32;
            }

/*
            if (GameSettings.GameType != GAMETYPE_DEMO)
            {
                g_pFont->DrawText(304, y, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_CAR), XBFONT_RIGHT );
                g_pFont->DrawText(304, y, MENU_TEXT_RGB_NORMAL, L":");
                swprintf( buf, L"%S", CarInfo[StartData.PlayerData[StartData.LocalPlayerNum].CarType].Name);
                g_pFont->DrawText(328, y, MENU_TEXT_RGB_CHOICE, buf);
                y += 32;
            }
*/

//$REMOVED - since we don't unlock levels, it's misleading to players about the
//            if (GameSettings.GameType == GAMETYPE_SINGLE && GameSettings.Level < LEVEL_NCUP_LEVELS)
//            {
//                g_pFont->DrawText(312, y, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_LOADSCREEN_WONRACE), XBFONT_RIGHT );
//                g_pFont->DrawText(328, y, MENU_TEXT_RGB_CHOICE, IsSecretWonSingleRace(GameSettings.Level) ? TEXT_TABLE(TEXT_YES) : TEXT_TABLE(TEXT_NO));
//                y += 32;
//            }
//$END_REMOVAL

            if (GameSettings.GameType == GAMETYPE_PRACTICE)
            {
                g_pFont->DrawText(312, y, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_LOADSCREEN_FOUNDSTAR), XBFONT_RIGHT );
                g_pFont->DrawText(328, y, MENU_TEXT_RGB_CHOICE, IsSecretFoundPractiseStars(GameSettings.Level) ? TEXT_TABLE(TEXT_YES) : TEXT_TABLE(TEXT_NO));
                y += 32;
            }

            if (GameSettings.GameType == GAMETYPE_TRAINING)
            {
                g_pFont->DrawText(312, y, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_LOADSCREEN_STARSCOLLECTED), XBFONT_RIGHT );
                swprintf(buf, L"%ld %s %ld", StarList.NumFound, TEXT_TABLE(TEXT_OF), StarList.NumTotal);
                g_pFont->DrawText(328, y, MENU_TEXT_RGB_CHOICE, buf);
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
                    swprintf(buf, L"%02d:%02d:%03d", MINUTES(time), SECONDS(time), THOUSANDTHS(time));
                else
                    swprintf(buf, L"%s", TEXT_TABLE(TEXT_LOADSCREEN_BEATEN));

                g_pFont->DrawText(312, y, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_LOADSCREEN_CHALLENGETIME), XBFONT_RIGHT );
                g_pFont->DrawText(328, y, MENU_TEXT_RGB_CHOICE, buf);
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

// wait for dead

    LoadThreadKill = TRUE;    //$NOTE: tells the load thread to terminate
//$MODIFIED
//    while (LoadThreadKill); //$NOTE: waits for the load thread to terminate

    if( LoadThreadHandle )
    {
        if( WaitForSingleObject( LoadThreadHandle, 10000 ) == WAIT_TIMEOUT )
        {
            DumpMessageVar( "Shit!", "Waiting for load thread, but I think it's toast." );
            WaitForSingleObject( LoadThreadHandle, INFINITE );  //$NOTE: we use this instead of the while() loop, b/c load thread was set to lower priority and never gets executed if we spin here
        }
    }
//$END_MODIFICATIONS

// close thread handle

    CloseHandle(LoadThreadHandle);
    //$MODIFIED: We've had sporadic hangs in WaitForSingleObject above.  When I checked
    // in the debugger, the thread was no longer in existence, so I imagine 
    // LoadThreadHandle was stale
    LoadThreadHandle = 0;

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
//    long col1, col2;
//    float destper, xstart, ystart, xsize, ysize;
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

//$REVISIT: we had to remove the render code here (b/c only render from main thread), but see if we want to bother with putting it back.
//$REMOVED (tentative!! only render from main thread)
//// set viewport
//
//    SetViewport(0.0f, 0.0f, (float)ScreenXsize, (float)ScreenYsize, RenderSettings.GeomPers);
//$END_REMOVAL

// loop drawing loading bar

//$MODIFIED
//    while (!LoadThreadKill || LoadThreadPer < 100.0f)
    while (!LoadThreadKill /*|| LoadThreadPer < 100.0f*/)  //$TODO: should re-enable this check if/when we actually update 'LoadThreadPer'
//$END_MODIFICATIONS
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
//        g_pFont->DrawText( 320, 440, MENU_TEXT_RGB_NORMAL, LastFile, XBFONT_CENTER_X);
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

            RedbookDeviceLetter = (char)toupper(cddrives[i][0]);

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
