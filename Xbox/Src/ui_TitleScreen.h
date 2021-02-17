//-----------------------------------------------------------------------------
// File: ui_TitleScreen.h
//
// Desc: UI implementation
//
// Re-Volt (Generic) Copyright (c) Probe Entertainment 1998
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef TITLESCREEN_H
#define TITLESCREEN_H

#include "ui_TitleScreenCamera.h"
#include "player.h"     // for MAX_LOCAL_PLAYERS

/////////////////////////////////////////////////////////////////////////////////
// Defines
/////////////////////////////////////////////////////////////////////////////////

#define TS_COORD_SCALE      (100.0f)
#define TS_CAMERA_MOVE_TIME TO_TIME(0.85f)


/////////////////////////////////////////////////////////////////////////////////
// Structures
/////////////////////////////////////////////////////////////////////////////////

// Starting messages
enum 
{
    MENU_MESSAGE_NONE = -1,
    MENU_MESSAGE_NEWCARS = 0,
    MENU_MESSAGE_REVERSE,
    MENU_MESSAGE_MIRROR,
    MENU_MESSAGE_REVMIR,
    MENU_MESSAGE_COCKWORK,
    MENU_MESSAGE_PLEASE_SAVE,

    MENU_NMESSAGES
};


// Name selection
#define MAX_NAMESELECTCHARS (26+4)
#define NAMESELECT_END      (MAX_NAMESELECTCHARS-1)
#define NAMESELECT_DELETE   (MAX_NAMESELECTCHARS-2)
#define NAME_WHEEL_DEGREES  (1024.0f)


enum PlayModeEnum {
    MODE_SIMULATION = 0,
    MODE_ARCADE,
    MODE_CONSOLE,
    MODE_KIDS,

    MODE_NMODES
};

// Check that a variable _var lies within range [_minVal, _maxVal)
#define CheckRange(_var, _minVal, _maxVal) \
    (((_var) >= (_minVal)) && ((_var) < (_maxVal)))


struct TITLESCREENPLAYERDATA
{
    // Name wheel data
    int     nameSelectPos;     // Name selection position
    int     nameEnterPos;      // Name entered position
    FLOAT   nameWheelAngle;
    FLOAT   nameWheelDestAngle;
    CHAR    nameEnter[MAX_PLAYER_NAME]; // Name entered
    int     iCarNum;           // Car #  //$TODO(cprince): change this to 'long CarType' to be consistent with the rest of the code.  //$REVISIT: there's overlap between this and the StartData structure (undesirable).
};


// TitleScreen vars
struct TITLESCREENVARS
{
    //$REVISIT: seems to be a lot of overlap with the GAME_SETTINGS and START_DATA structs here.
    /// Not sure whether that's desired.  (Maybe want it so that user's default
    /// choices in UI are remembered, independent of game actually played.  For
    /// example, in network case where external host picks game.)  But when game
    /// starts, values should get copied from gTitleScreenVars to GameSettings,
    /// and all non-UI code should use the values in GameSettings.
    long    GameType;  //$BUG: in most places, UI code seems to be setting GameSettings.GameType instead of gTitleScreenVars.GameType.
    long    CupType;
    long    iLevelNum;                  // Level #
    long    numberOfLaps;               // Number of race laps
    long    numberOfCars;               // Number of cars in the race (including players) //$NOTE(cprince): and including AI cars
    long    numberOfPlayers;            // Number of players //$NOTE(cprince): number of *local* players (verified!), though multiplayer code hardly uses it currently... (maybe uses PlayerCount instead, which includes all cars in race?)
    bool    bUseXOnline;                //($ADDITION) use XOnline or SysLink for matchmaking, etc?

    long    numberOfPrivateSlots;       // # of private (friends-only) player slots
    long    numberOfPublicSlots;        // # of public player slots ($REVIEW: does this exclude local players?)

    long    sfxVolume;                  // SFX Volume
    long    musicVolume;                // Music Volume

    bool    reverse;                    // Level is reversed?
    bool    mirror;                     // Level is mirrored?
    bool    pickUps;                    // Pickups on/off?
    
    long    sparkLevel;                 // Number of particle effects
    bool    smoke;                      // smoke effects on/off
    
    bool    bAllowDemoToRun;            // Is demo mode allowed to run automatically?

    bool    rearview;                   // Whether to draw rear view mirror
    long    playMode;                   // Whether to use proper collision (SIM, ARCADE, KIDS)

//  int     nameSelectPos;              // Name selection position
//  int     nameEnterPos;               // Name entered position
//  char    nameEnter[MAX_LOCAL_PLAYERS][MAX_PLAYER_NAME]; // Name entered
//  int     iCarNum[MAX_LOCAL_PLAYERS]; // Car #
    long    iCurrentPlayer;             // Current player number entering data
    TITLESCREENPLAYERDATA  PlayerData[4];
    TITLESCREENPLAYERDATA* pCurrentPlayer;


    unsigned Language;

    bool    RandomCars;                 // Random cars for multiplayer
    bool    RandomTrack;                // Random tracks

    long    textureFilter;              // Texture filter
    long    mipLevel;                   // Mipmap level

    bool    LocalGhost;                 // Use local or downloaded ghost?
    bool    reflections;                // Allow reflections?
    bool    shinyness;                  // Shinyness?
    bool    shadows;                    // Shadows?
    bool    skidmarks;                  // SkidMarks?
    bool    lights;                     // Lights?
    bool    instances;                  // Instances?
    bool    antialias;                  // Antialiasing?

    long    connectionType;             // Multiplayer connection type
};




/////////////////////////////////////////////////////////////////////////////////
// Externs
/////////////////////////////////////////////////////////////////////////////////
extern CAMERA_POS           g_CameraPositions[];
extern CTitleScreenCamera*  g_pTitleScreenCamera;

extern TITLESCREENVARS      gTitleScreenVars;
extern BOOL                 g_bTitleScreenFadeStarted;
extern BOOL                 g_bTitleScreenRunGame;
extern BOOL                 g_bTitleScreenRunDemo;
extern BOOL                 g_bShowWinLoseSequence;

extern FLOAT                g_fTitleScreenTimer;

extern long                 gCurrentScreenTPage;
extern int                  gTrackScreenLevelNum;


extern long   InitialMenuMessage;
extern int    InitialMenuMessageCount;
extern int    InitialMenuMessageWidth;
extern WCHAR  InitialMenuMessageString[256];
extern WCHAR* InitialMenuMessageLines[10];
extern FLOAT  InitialMenuMessageTimer;
extern FLOAT  InitialMenuMessageMaxTime;

extern bool gfGhostLoadDone;


extern D3DTexture* g_pLiveSignInTexture;
extern D3DTexture* g_pLiveSignInPlayerTexture;
extern D3DTexture* g_pLiveSignInListBoxTexture;
extern D3DTexture* g_pHeaderLayer1Texture;
extern D3DTexture* g_pHeaderLayer2Texture;
extern D3DTexture* g_pFooterLayer1Texture;
extern D3DTexture* g_pFooterLayer2Texture;

extern D3DTexture* g_pCommunicatorThruTVTexture;
extern D3DTexture* g_pFriendOnlineTexture;
extern D3DTexture* g_pFriendReqReceivedTexture;
extern D3DTexture* g_pFriendReqSentTexture;
extern D3DTexture* g_pGameInviteReceivedTexture;
extern D3DTexture* g_pGameInviteSentTexture;
extern D3DTexture* g_pTalkEnabledTexture;
extern D3DTexture* g_pTalkMutedTexture;
extern D3DTexture* g_pTalking0Texture;
extern D3DTexture* g_pTalking1Texture;
extern D3DTexture* g_pTalking2Texture;
extern D3DTexture* g_pTalking3Texture;

/////////////////////////////////////////////////////////////////////////////////
// Prototypes
/////////////////////////////////////////////////////////////////////////////////
void GoTitleScreen();
void ReleaseTitleScreen();
void TitleScreen();
void StartInitTitleScreenVars();

void ts_SetupCar();

void ClockHandler(OBJECT *obj);

extern void SetRaceData();
extern void SetDemoData();
extern void SetCalcStatsData();

extern void InitFrontendMenuCamera();
extern void InitWinLoseCamera();

extern void InitMenuMessage(FLOAT timeOut);
extern void SetMenuMessage(WCHAR *message);
extern void SetBonusMenuMessage();

extern void LoadFrontEndTextures();
extern void FreeFrontEndTextures();

#endif // TITLESCREEN_H

