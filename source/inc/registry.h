
#ifndef REGISTRY_H
#define REGISTRY_H

#include "main.h"
#include "level.h"

// macros

#define REGISTRY_SECURITY_CHECK 0
#define REGISTRY_ROOT HKEY_LOCAL_MACHINE
#define REGISTRY_KEY "software\\Acclaim\\Revolt"

#define	GET_REGISTRY_VALUE(_key, _reg, _buf, _size) \
{ \
	DWORD _s = (_size); \
	RegQueryValueEx((_key), (_reg), 0, NULL, (unsigned char*)(_buf), &_s); \
}

#define	SET_REGISTRY_VALUE(_key, _reg, _type, _buf, _size) \
{ \
	RegSetValueEx((_key), (_reg), 0, (_type), (unsigned char*)(_buf), (_size)); \
}

typedef struct {
	DWORD EnvFlag, MirrorFlag, AutoBrake, ShadowFlag;
	DWORD LightFlag, InstanceFlag, SkidFlag, CarID;
	DWORD ScreenWidth, ScreenHeight, ScreenBpp, DrawDevice;
	DWORD Brightness, Contrast, TextureBpp;
	char PlayerName[MAX_PLAYER_NAME];
	char LevelDir[MAX_LEVEL_DIR_NAME];
} REGISTRY_SETTINGS;

// prototypes

extern void GetRegistrySettings(void);
extern void SetRegistrySettings(void);

// globals

extern REGISTRY_SETTINGS RegistrySettings;

#endif
