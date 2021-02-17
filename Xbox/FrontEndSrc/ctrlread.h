//-----------------------------------------------------------------------------
// File: ctrlread.h
//
// Desc: Controller reading code (for mouse, keyboard, etc)
//
//       Moved over code from control.cpp. control.cpp now used for processing
//       controller inputs into game controls. Controller inputs are generated
//       in this file. Local key handling is done here without being passed to
//       the game code itself (ie. for car model changes, etc).
//
// Re-Volt (PC) Copyright (c) Probe Entertainment 1998
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef CTRLREAD_H
#define CTRLREAD_H


#include "Control.h"

//
// Defines and macros
//

#define WANKY_KEYS FALSE
#define FUNCTION_KEY_MAX 50

// key type enum

enum KEY_TYPE
{
    KEY_TYPE_KEYBOARD,
    KEY_TYPE_BUTTON,
    KEY_TYPE_AXISNEG,
    KEY_TYPE_AXISPOS,
};

//
// Typedefs and structures
//

typedef struct {
    long SysKey, NotSysKey, Key, NewOnly;
    void (*Func)(void);
} FUNCTION_KEY;

typedef struct {
    short Index;
    char pad, Type;
} KEY;

// CTRL_TYPE - platform specific - each platform should have it's own ctrlread functions

typedef enum
{
    CTRL_TYPE_NONE = 0,
    CTRL_TYPE_LOCAL,
    CTRL_TYPE_CPU_AI,
    CTRL_TYPE_MOUSE,
    CTRL_TYPE_REMOTE,
    CTRL_TYPE_REPLAY,
    CTRL_TYPE_TS_TRAINING,
} CTRL_TYPE;

//
// External function prototypes
//

struct PlayerStruct;

extern bool GetDigitalState(KEY *key);
extern bool GetDigitalStateJoy(KEY *key);
extern char GetAnaloguePair(KEY *key1, KEY *key2);
extern void CRD_CheckLocalKeys(void);
extern void CRD_InitPlayerControl(struct PlayerStruct *player, CTRL_TYPE CtrlType);
extern void CRD_LocalInput(CTRL *Control);
//extern void CRD_TitleScreenTraining(PLAYER *player);
extern void InitFunctionKeys(void);
extern long AddFunctionKey(long syskey, long notsyskey, long key, void (*func)(void), long newonly);
extern void AddAllFunctionKeys(void);
extern void CRD_CarAiInput(CTRL *Control);
extern long SearchForKeyPress(KEY *key);
extern bool AnyKeyOrButton(void);
extern char *GetStringFromDik(long index);
extern void CRD_SetDefaultControls();
extern void CRD_SetControlsFromRegistry();

// globals

extern KEY KeyTable[];


#endif // CTRLREAD_H

