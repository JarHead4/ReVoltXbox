//-----------------------------------------------------------------------------
// File: TitleScreen.h
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef TITLESCREEN_H
#define TITLESCREEN_H

#include "Player.h"
#include "FrontEndCamera.h"


//-----------------------------------------------------------------------------
// Defines
//-----------------------------------------------------------------------------
#define TS_CAMERA_MOVE_TIME   0.85f




//-----------------------------------------------------------------------------
// Structures
//-----------------------------------------------------------------------------

// Starting messages
enum MENU_MESSAGE
{
    MENU_MESSAGE_NONE    = -1,
    MENU_MESSAGE_NEWCARS = 0,
    MENU_MESSAGE_REVERSE,
    MENU_MESSAGE_MIRROR,
    MENU_MESSAGE_REVMIR,
    MENU_MESSAGE_COCKWORK,
    MENU_MESSAGE_PLEASE_SAVE,

    NUM_MENU_MESSAGES
};


// Name selection
#define MAX_INPUT_NAME      MAX_PLAYER_NAME



enum PLAYMODE 
{
    PLAYMODE_SIMULATION = 0,
    PLAYMODE_ARCADE,
    PLAYMODE_CONSOLE,
    PLAYMODE_KIDS,

    NUM_PLAYMODES
};




enum FRONTENDSTATE
{
	FRONTENDSTATE_TOPLEVEL_MENU  = 0,
	FRONTENDSTATE_PLAYLIVE,
};




// Check that a variable _var lies within range [_minVal, _maxVal)
#define CheckRange(_var, _minVal, _maxVal) \
    (((_var) >= (_minVal)) && ((_var) < (_maxVal)))

// TitleScreen vars
struct TITLESCREEN_DATA
{
	BOOL    bGameTypeSelected;
	BOOL    bTrackSelected;
	BOOL    bCarsSelected;

    long    gameType;
    long    CupType;
    long    iLevelNum;                  // Level #
    long    iCurrentPlayer;             // Current player number entering data
    long    numberOfLaps;               // Number of race laps
    long    numberOfCars;               // Number of cars in the race (including players)
    long    numberOfPlayers;            // Number of players

    long    sfxVolume;                  // SFX Volume
    long    musicVolume;                // Music Volume
    long    sfxTest;                    // SFX Test
    long    musicTest;                  // Music Test

    bool    reverse;                    // Level is reversed?
    bool    mirror;                     // Level is mirrored?
    bool    pickUps;                    // Pickups on/off?
    
    long    sparkLevel;                 // Number of particle effects
    bool    smoke;                      // smoke effects on/off
    
    bool    bAllowDemoToRun;            // Is demo mode allowed to run automatically?

    bool    rearview;                   // Whether to draw rear view mirror
    long    playMode;                   // Whether to use proper collision (SIM, ARCADE, KIDS)

    int     nameSelectPos;              // Name selection position
    int     nameEnterPos;               // Name entered position

    char    nameEnter[MAX_LOCAL_PLAYERS][MAX_INPUT_NAME+1]; // Name entered
    int     iCarNum[MAX_LOCAL_PLAYERS]; // Car #

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




//-----------------------------------------------------------------------------
// Externs
//-----------------------------------------------------------------------------
extern CAMERA_POS           g_CameraPositions[];
extern CFrontEndCamera*     g_pTitleScreenCamera;


extern TITLESCREEN_DATA     g_TitleScreenData;
extern bool                 g_bTitleScreenFadeStarted;
extern bool                 g_bTitleScreenRunGame;
extern bool                 g_bTitleScreenRunDemo;
extern bool                 g_bShowWinLoseSequence;

extern FLOAT                g_fTitleScreenTimer;

extern int                  g_iTrackScreenLevelNum;


extern DWORD                g_FrontEndState;


//-----------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------
void GoTitleScreen();
void ReleaseTitleScreen();
void TitleScreen();
void InitTitleScreenData();

void SetupTitleScreenCar();

void SetRaceData();
void SetDemoData();
void SetCalcStatsData();

void InitFrontendMenuCamera();


#endif // TITLESCREEN_H

