//-----------------------------------------------------------------------------
// File: panel.h
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef PANEL_H
#define PANEL_H

#include "car.h"

// macros

#define SPLIT_COUNT 2.0f
#define TRACK_DIR_COUNT 640
#define TRACK_DIR_FADE_COUNT 128
#define WRONG_WAY_TOLERANCE 1.0f
#define REV_LIT_NUM 7
#define REV_LIT_MAX 1024
#define MAX_MAP_DIR_NAME 16
#define CONSOLE_MESSAGE_MAX 32

#define CONSOLE_MESSAGE_FOREVER         Real(3600.0)
#define CONSOLE_MESSAGE_DEFAULT_TIME    Real(2.0)

#define CENTRE_POS(_s) \
    (FLOAT)((640 - strlen(TEXT_TABLE(_s)) * 8) / 2)

#define RIGHT_JUSTIFY_POS(_s, _p) \
    (FLOAT)(_p - strlen(TEXT_TABLE(_s)) * 8)

// prototypes

extern void TriggerTrackDir(struct PlayerStruct *player, long flag, long n, PLANE *planes);
extern void TriggerSplit(struct PlayerStruct *player, long flag, long n, PLANE *planes);
extern void DrawControlPanel(void);
extern void DrawPanelSprite(float x, float y, float width, float height, float tu, float tv, float twidth, float theight, long rgba);
extern void LoadDrum(void);
extern void FreeDrum(void);
extern void UpdateDrum(void);
extern void DrawDrum(void);
extern void LoadCountdownModels(void);
extern void FreeCountdownModels(void);
extern void DrawCountdown(void);
extern void DrawPracticeStars(void);
extern void DisplayPlayers(void);
extern void InitConsole(void);
extern void SetConsoleMessage(char *message1, char *message2, long rgb1, long rgb2, long pri, REAL time);

// globals

enum {
    SPEED_MPH,  // Miles Per Hour
    SPEED_SCALEDMPH,    // Scaled speeds 1/10
    SPEED_FPM,  // Feet Per Minute
    SPEED_KPH,  // Kilometers Per Hour
    SPEED_SCALEDKPH,    // Kilometers Per Hour

    SPEED_NTYPES
};

typedef struct {
    char Dir[MAX_MAP_DIR_NAME];
    REAL x, y, xscale, yscale;
    REAL tu, tv, tw, th;
} MAP_INFO;

extern char *PosText[];
extern long SpeedUnits;
extern long AllowCredits;
extern long ChallengeFlash;
extern long PracticeStarFlash;
extern char *SpeedUnitText[];
extern REAL SpeedUnitScale[];
extern long MultiPlayerColours[];

#endif // PANEL_H

