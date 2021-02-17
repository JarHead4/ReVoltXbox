//-----------------------------------------------------------------------------
// File: main.h
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef MAIN_H
#define MAIN_H

// defines

#define MULTIPLAYER_DEBUG 0
#define GHOST_TAKEOVER 0
#define CHECK_ZZZ 0
#define SCREEN_DEBUG 0
#define SCREEN_TIMES 0

#ifdef CHRIS_EXTRAS
#define SHOW_PHYSICS_INFO 1
#else
#define SHOW_PHYSICS_INFO 0
#endif


#define MSCOMPILER_FUDGE_OPTIMISATIONS 1

#define STAGE_ONE_LOAD_COUNT 19
#define STAGE_TWO_LOAD_COUNT 5

#define MINUTES(_t) (((_t) / 60000))
#define SECONDS(_t) (((_t) / 1000) % 60)
#define THOUSANDTHS(_t) ((_t) % 1000)

//$MODIFIED
//#define SET_EVENT(_f) \
//    (Event = (void(__cdecl*)(void))((char*)(_f) + (ChecksumCRC * RegistryZZZ - ChecksumCD) + (TracksCD - REDBOOK_TRACK_NUM)))
#define SET_EVENT(_f) \
    (Event = (void(*)(void))((char*)(_f)))
    //$NOTE: removed __cdecl to avoid __stdcall/__cdecl conflicts
//$END_MODIFICATIONS

// edit modes

enum {
    EDIT_NONE,
    EDIT_LIGHTS,
    EDIT_VISIBOXES,
    EDIT_OBJECTS,
    EDIT_INSTANCES,
    EDIT_AINODES,
    EDIT_ZONES,
    EDIT_TRIGGERS,
    EDIT_CAM,
    EDIT_FIELDS,
    EDIT_PORTALS,
    EDIT_POSNODE,

    EDIT_NUM
};

// Versions of the game

typedef enum VersionEnum {
    VERSION_DEV,                // Everything in it
    VERSION_RELEASE,            // Full Release

    VERSION_NTYPES
} VERSION;

// multiplayer versions

typedef enum {
    MULTIPLAYER_VERSION_DEV,
    MULTIPLAYER_VERSION_PCXL,
    MULTIPLAYER_VERSION_CGW,
    MULTIPLAYER_VERSION_PCGAMER,
    MULTIPLAYER_VERSION_TNT2,
    MULTIPLAYER_VERSION_RELEASE,
    MULTIPLAYER_VERSION_ACCLAIM,
    MULTIPLAYER_VERSION_GAMESPOT,
    MULTIPLAYER_VERSION_GAMECENTER,
    MULTIPLAYER_VERSION_IGN_PC,
} MULTIPLAYER_VERSION;

// typedefs and structures

enum LoadStageEnum {
    LOAD_STAGE_ZERO,
    LOAD_STAGE_ONE,
    LOAD_STAGE_TWO,

    LOAD_NSTAGES
};

typedef struct {
    long            GameType, MultiType, Level, LevelNum;
    long            NumberOfLaps, RandomCars, RandomTrack;
    unsigned long   Reversed, Mirrored;
    unsigned long   AutoBrake, CarType, Paws, AllowPickups;
    unsigned long   LoadStage, LoadWorld;
    unsigned long   LocalGhost;
    long            DrawRearView;
    long            DrawFollowView;
    long            PlayMode;
} GAME_SETTINGS;

typedef struct {
    REAL GeomPers;
    REAL GeomCentreX;
    REAL GeomCentreY;
    REAL GeomScaleX;
    REAL GeomScaleY;
    REAL MatScaleX;
    REAL MatScaleY;
    REAL NearClip;
    REAL FarClip;
    REAL ZedDrawDist;
    REAL ZedFarDivDist;
    REAL ZedFarMulNear;
    REAL ZedSub;
    REAL FogStart;
    REAL FogDist;
    REAL FogMul;
    REAL VertFogStart;
    REAL VertFogEnd;
    REAL VertFogDist;
    REAL VertFogMul;
    long Env;
    long Mirror;
    long Shadow;
    long Light;
    long Instance;
    long Skid;
    long AntiAlias;
    long Sepia;
} RENDER_SETTINGS;

// prototypes

//$REMOVEDextern LRESULT CALLBACK WindowFunc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
//$REMOVEDextern bool InitWin(HINSTANCE hThisInst, int nWinMode);
extern void DumpMessage(char *title, char *mess);  //$MODIFIED: renamed from Box(), removed 3rd param "long flag", and return type was "long"
//$REMOVED_UNREACHABLEextern long BoxNum(long num);
//$REMOVEDextern long BoxText(char *text);
//$REMOVED_UNREACHABLEextern void Vblank(char count);
extern void GameLoop(void);
extern void Go(void);
extern void GoFront(void);
extern void SetupGame(void);
extern void CheckCheatStrings(void);
extern void SetupLevelAndPlayers(void);
extern unsigned long GetFileChecksum(char *file, bool appendedChecksum, bool binaryMode);
extern unsigned long GetStreamChecksum(FILE *fp, bool appendedChecksum, bool binaryMode);
extern unsigned long GetMemChecksum(long *mem, long bytes);
extern void EnableLoadThread(long units);
extern void DisableLoadThread(void);
extern void IncLoadThreadUnitCount(void);
extern bool CheckStreamChecksum(FILE *fp, bool binaryMode);
extern unsigned long HexStringToInt(char *string);
extern void CreateSepiaRGB(void);
extern void KillSepiaRGB(void);

// globals

extern long UseLanguages;
extern char gGazzasAICar;
extern char gGazzasAICarDraw;
extern char gGazzasRouteChoice;
extern char gGazzasOvertake;
extern int giGazzaForceRoute;

enum TrackEditorSpawnState {
    TRACKEDIT_NOT_SPAWNED,
    TRACKEDIT_SPAWN,
    TRACKEDIT_SPAWNED,
    TRACKEDIT_SPAWNED_ALIVE,

    TRACKEDIT_NSTATES
};

#ifndef _PC
extern bool gTrackEditorSpawned;
#else
extern long gTrackEditorSpawnState;
#endif

extern bool ModifiedCarInfo;
extern long CarVisi;
extern char WheelRenderType;
extern char NoGamma;
//$REMOVEDextern char AppRestore;
extern char g_bQuitGame;  //$RENAMED: was g_bQuitGame
//$REMOVEDextern char FullScreen;
extern unsigned long FrameCount, FrameCountLast, FrameTime, FrameTimeLast, FrameRate;
extern long AlphaRef;
extern long NoUser;
extern long Skybox;
extern unsigned long ChecksumCRC, ChecksumCD;
extern long TracksCD;
extern long RegistryZZZ;
extern long NFO;
extern REAL TimeStep;
extern REAL RealTimeStep;
extern REAL EditScale;
extern VEC EditOffset;
//$REMOVEDextern HWND hwnd;
//extern HBITMAP TitleHbm;
extern void (*Event)(void);
extern GAME_SETTINGS    GameSettings;
extern RENDER_SETTINGS  RenderSettings;
  #ifndef XBOX_NOT_YET_IMPLEMENTED
extern int __argc;
extern char **__argv;
  #endif // ! XBOX_NOT_YET_IMPLEMENTED
extern bool ReplayMode;
extern unsigned short *SepiaRGB;

#if SHOW_PHYSICS_INFO
#include "draw.h"
extern long DEBUG_CollGrid;
extern int DEBUG_NCols;
extern int DEBUG_LastNCols;
extern int DEBUG_N2Cols;
extern VEC DEBUG_dR;
extern VEC DEBUG_Impulse;
extern VEC DEBUG_AngImpulse;
extern VEC DEBUG_SNorm[256];
extern FACING_POLY DEBUG_Faces[256];
#endif

extern REAL gDemoTimer;
extern REAL gDemoFlashTimer;
extern long gDemoShowMessage;
extern VERSION Version;
extern MULTIPLAYER_VERSION MultiplayerVersion;
extern char VersionString[];
extern char RegistryKey[];

                  
#endif // MAIN_H

