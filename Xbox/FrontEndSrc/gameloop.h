//-----------------------------------------------------------------------------
// File: gameloop.h
//
// Desc: Main game loop code.
//
// Re-Volt (Generic) Copyright (c) Probe Entertainment 1998
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef GAMELOOP_H
#define GAMELOOP_H

#define MAX_TIMESTEP    (150)           // 1 / max time step allowed for physics

#define GHOST_TAKEOVER_TIME 10000000

typedef enum {
    GAMELOOP_QUIT_OFF,
    GAMELOOP_QUIT_FRONTEND,
    GAMELOOP_QUIT_CHAMP_CONTINUE,
    GAMELOOP_QUIT_REPLAY,
    GAMELOOP_QUIT_RESTART_REPLAY,
    GAMELOOP_QUIT_RESTART,
    GAMELOOP_QUIT_GAMEGAUGE,
    GAMELOOP_QUIT_DEMO,
} GAMELOOP_QUIT;

extern GAMELOOP_QUIT GameLoopQuit;
extern REAL DemoTimeout;
extern unsigned long NPhysicsLoops;
extern bool DrawGridCollSkin;

// Externs for the Gap-style slo-mo camera
extern bool GLP_TriggerGapCamera;
extern REAL GLP_GapCameraTimer;
extern VEC GLP_GapCameraPos[3];
extern MAT GLP_GapCameraMat;

// defines + macros

enum {
    PAWS_MENU_NOTHING = -1,
    PAWS_MENU_RESUME = 0,
    PAWS_MENU_REPLAY,
    PAWS_MENU_SAVEREPLAY,
    PAWS_MENU_QUIT,

    PAWS_MENU_NUM
};

// External function prototpyes

extern void GLP_GameLoop(void);
extern long PawsMenu(void);
extern void DefaultPhysicsLoop();
extern void ReplayPhysicsLoop();


#endif // GAMELOOP_H

