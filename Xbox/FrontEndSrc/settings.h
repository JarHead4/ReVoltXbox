//-----------------------------------------------------------------------------
// File: settings.h
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef SETTINGS_H
#define SETTINGS_H

#include "main.h"
#include "LevelLoad.h"

// macros

//$REMOVED#define REGISTRY_SECURITY_CHECK 0
#define REGISTRY_ROOT HKEY_LOCAL_MACHINE

#define GET_REGISTRY_VALUE(_key, _reg, _buf, _size) \
{ \
    DWORD _s = (_size); \
    RegQueryValueEx((_key), (_reg), 0, NULL, (unsigned char*)(_buf), &_s); \
}

#define SET_REGISTRY_VALUE(_key, _reg, _type, _buf, _size) \
{ \
    RegSetValueEx((_key), (_reg), 0, (_type), (unsigned char*)(_buf), (_size)); \
}

typedef struct {
    DWORD EnvFlag, MirrorFlag, AutoBrake, ShadowFlag;
    DWORD LightFlag, InstanceFlag, SkidFlag, PickupFlag, CarType;
    DWORD NCars, NLaps, SfxVol, MusicVol;
    DWORD ScreenWidth, ScreenHeight, ScreenBpp;
//$REMOVED    DWORD DrawDevice;
    DWORD Brightness, Contrast, DrawDist, Texture24;
    DWORD Vsync, ShowFPS, SfxChannels, MusicOn;
    DWORD KeyLeft, KeyRight, KeyFwd, KeyBack;
    DWORD KeyFire, KeyReset, KeyReposition, KeyHonka, KeyHandbrake;
    DWORD KeySelWeapon, KeySelCamera, KeyFullBrake, KeyPause;
    long Joystick, SteeringDeadzone, SteeringRange;
    long ParticleLevel;
    long PlayMode;

    char PlayerName[MAX_PLAYER_NAME];
    char LevelDir[MAX_LEVEL_DIR_NAME];
    char HostComputer[MAX_HOST_COMPUTER];

    BOOL PositionSave; //$ADDITION(jedl) - dave mccoy wants to start where he left off for tuning the art
    BOOL bUseGPU; //$ADDITION(jedl) - allow toggling b/w new pipeline and old pipeline
} REGISTRY_SETTINGS;

// prototypes

extern void GetRegistrySettings(void);
extern void SetRegistrySettings(void);

// globals

extern REGISTRY_SETTINGS RegistrySettings;

#endif // SETTINGS_H

