
#include "revolt.h"
#include "registry.h"

// globals

REGISTRY_SETTINGS RegistrySettings;

///////////////////////////
// get registry settings //
///////////////////////////

void GetRegistrySettings(void)
{
	long r;
	HKEY key;
	DWORD size, flag;

// set defaults

	RegistrySettings.EnvFlag = TRUE;
	RegistrySettings.MirrorFlag = TRUE;
	RegistrySettings.AutoBrake = FALSE;
	RegistrySettings.ShadowFlag = TRUE;

	RegistrySettings.LightFlag = TRUE;
	RegistrySettings.InstanceFlag = TRUE;
	RegistrySettings.SkidFlag = TRUE;
	RegistrySettings.CarID = 0;

	RegistrySettings.ScreenWidth = 640;
	RegistrySettings.ScreenHeight = 480;
	RegistrySettings.ScreenBpp = 16;
	RegistrySettings.DrawDevice = 0;

	RegistrySettings.Brightness = 256;
	RegistrySettings.Contrast = 256;
	RegistrySettings.TextureBpp = 16;

	size = MAX_PLAYER_NAME;
	GetUserName(RegistrySettings.PlayerName, &size);

	wsprintf(RegistrySettings.LevelDir, "MUSE1");

// create or open key

	r = RegCreateKeyEx(REGISTRY_ROOT, REGISTRY_KEY, 0, "Revolt", REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &key, &flag);
	if (r != ERROR_SUCCESS)	return;

// attempt to overwrite defaults

	GET_REGISTRY_VALUE(key, "EnvFlag", &RegistrySettings.EnvFlag, 4);
	GET_REGISTRY_VALUE(key, "MirrorFlag", &RegistrySettings.MirrorFlag, 4);
	GET_REGISTRY_VALUE(key, "AutoBrake", &RegistrySettings.AutoBrake, 4);
	GET_REGISTRY_VALUE(key, "ShadowFlag", &RegistrySettings.ShadowFlag, 4);

	GET_REGISTRY_VALUE(key, "LightFlag", &RegistrySettings.LightFlag, 4);
	GET_REGISTRY_VALUE(key, "InstanceFlag", &RegistrySettings.InstanceFlag, 4);
	GET_REGISTRY_VALUE(key, "SkidFlag", &RegistrySettings.SkidFlag, 4);
	GET_REGISTRY_VALUE(key, "CarID", &RegistrySettings.CarID, 4);

	GET_REGISTRY_VALUE(key, "ScreenWidth", &RegistrySettings.ScreenWidth, 4);
	GET_REGISTRY_VALUE(key, "ScreenHeight", &RegistrySettings.ScreenHeight, 4);
	GET_REGISTRY_VALUE(key, "ScreenBpp", &RegistrySettings.ScreenBpp, 4);
	GET_REGISTRY_VALUE(key, "DrawDevice", &RegistrySettings.DrawDevice, 4);

	GET_REGISTRY_VALUE(key, "Brightness", &RegistrySettings.Brightness, 4);
	GET_REGISTRY_VALUE(key, "Contrast", &RegistrySettings.Contrast, 4);
	GET_REGISTRY_VALUE(key, "TextureBpp", &RegistrySettings.TextureBpp, 4);

	GET_REGISTRY_VALUE(key, "PlayerName", RegistrySettings.PlayerName, MAX_PLAYER_NAME);
	GET_REGISTRY_VALUE(key, "LevelDir", RegistrySettings.LevelDir, MAX_LEVEL_DIR_NAME);

	RenderSettings.Env = RegistrySettings.EnvFlag;
	RenderSettings.Mirror = RegistrySettings.MirrorFlag;
	GameSettings.AutoBrake = RegistrySettings.AutoBrake;
	RenderSettings.Shadow = RegistrySettings.ShadowFlag;

	RenderSettings.Light = RegistrySettings.LightFlag;
	RenderSettings.Instance = RegistrySettings.InstanceFlag;
	RenderSettings.Skid = RegistrySettings.SkidFlag;
	GameSettings.CarID = RegistrySettings.CarID;

// security check?

#if REGISTRY_SECURITY_CHECK
	flag = 0;
	GET_REGISTRY_VALUE(key, "WindowType", &flag, 4);
	if (!flag)
	{
		Box(NULL, "Illegal copy of Revolt!", MB_OK);
		QuitGame = TRUE;
		return;
	}
#endif

// close key

	RegCloseKey(key);
}

////////////////////////////
// save registry settings //
////////////////////////////

void SetRegistrySettings(void)
{
	long r;
	HKEY key;

// open key

	r = RegOpenKeyEx(REGISTRY_ROOT, REGISTRY_KEY, 0, KEY_ALL_ACCESS, &key);
	if (r != ERROR_SUCCESS) return;

// write registry settings

	RegistrySettings.EnvFlag = RenderSettings.Env;
	RegistrySettings.MirrorFlag = RenderSettings.Mirror;
	RegistrySettings.AutoBrake = GameSettings.AutoBrake;
	RegistrySettings.ShadowFlag = RenderSettings.Shadow;

	RegistrySettings.LightFlag = RenderSettings.Light;
	RegistrySettings.InstanceFlag = RenderSettings.Instance;
	RegistrySettings.SkidFlag = RenderSettings.Skid;
	RegistrySettings.CarID = GameSettings.CarID;

	SET_REGISTRY_VALUE(key, "EnvFlag", REG_DWORD, &RegistrySettings.EnvFlag, 4);
	SET_REGISTRY_VALUE(key, "MirrorFlag", REG_DWORD, &RegistrySettings.MirrorFlag, 4);
	SET_REGISTRY_VALUE(key, "AutoBrake", REG_DWORD, &RegistrySettings.AutoBrake, 4);
	SET_REGISTRY_VALUE(key, "ShadowFlag", REG_DWORD, &RegistrySettings.ShadowFlag, 4);

	SET_REGISTRY_VALUE(key, "LightFlag", REG_DWORD, &RegistrySettings.LightFlag, 4);
	SET_REGISTRY_VALUE(key, "InstanceFlag", REG_DWORD, &RegistrySettings.InstanceFlag, 4);
	SET_REGISTRY_VALUE(key, "SkidFlag", REG_DWORD, &RegistrySettings.SkidFlag, 4);
	SET_REGISTRY_VALUE(key, "CarID", REG_DWORD, &RegistrySettings.CarID, 4);

	SET_REGISTRY_VALUE(key, "ScreenWidth", REG_DWORD, &RegistrySettings.ScreenWidth, 4);
	SET_REGISTRY_VALUE(key, "ScreenHeight", REG_DWORD, &RegistrySettings.ScreenHeight, 4);
	SET_REGISTRY_VALUE(key, "ScreenBpp", REG_DWORD, &RegistrySettings.ScreenBpp, 4);
	SET_REGISTRY_VALUE(key, "DrawDevice", REG_DWORD, &RegistrySettings.DrawDevice, 4);

	SET_REGISTRY_VALUE(key, "Brightness", REG_DWORD, &RegistrySettings.Brightness, 4);
	SET_REGISTRY_VALUE(key, "Contrast", REG_DWORD, &RegistrySettings.Contrast, 4);
	SET_REGISTRY_VALUE(key, "TextureBpp", REG_DWORD, &RegistrySettings.TextureBpp, 4);

	SET_REGISTRY_VALUE(key, "PlayerName", REG_SZ, RegistrySettings.PlayerName, MAX_PLAYER_NAME);
	SET_REGISTRY_VALUE(key, "LevelDir", REG_SZ, RegistrySettings.LevelDir, MAX_LEVEL_DIR_NAME);

// close key

	RegCloseKey(key);
}
