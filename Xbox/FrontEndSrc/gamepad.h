//-----------------------------------------------------------------------------
// File: Gamepad.h
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef GAMEPAD_H
#define GAMEPAD_H


#include <xtl.h>


// Struct to maintain gamepad state
struct GamepadState
{
    HANDLE          hDevice;  // handle to device
    XINPUT_GAMEPAD  status;   // device status
};

// Global var containing state for all local gamepads
extern GamepadState g_Gamepads[4];

// Constants for thumbsticks
#define THUMB_MAX       (32767.5f)
#define THUMB_DEADZONE  (0.24f * THUMB_MAX)

// Function prototypes
void InitGamepads();
void UnInitGamepads();
void ReadGamepads();



#endif // GAMEPAD_H

