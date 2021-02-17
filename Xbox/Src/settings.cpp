//-----------------------------------------------------------------------------
// File: settings.cpp
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"
#include "settings.h"
#include "ctrlread.h"
#include "competition.h"
#include "geom.h"
#include "camera.h"
#include "player.h"
#include "LevelInfo.h"
#include "obj_init.h"
#include "ui_TitleScreen.h"
#include "verify.h"


//$TODO(cprince): should remove all references to 'Registry' eventually.
/// Maybe change to 'Startup' or 'App'.  'Startup' seems better, but some things already use 'App' and need check how/when values are referended during game.

// globals

REGISTRY_SETTINGS RegistrySettings;
static char Filename[MAX_PATH];

///////////////////////////
// get registry settings //
///////////////////////////
void GetRegistrySettings(void)
{
//$REMOVED    DWORD size;
#ifndef XBOX_NOT_YET_IMPLEMENTED
    long r;
    HKEY key;
    DWORD flag;
    LEVEL_SECRETS ls;
    STAR_LIST sl;
#endif // ! XBOX_NOT_YET_IMPLEMENTED

// set defaults

    RegistrySettings.EnvFlag = TRUE;
    RegistrySettings.MirrorFlag = TRUE;
    RegistrySettings.AutoBrake = FALSE;
    RegistrySettings.ShadowFlag = TRUE;
    RegistrySettings.LightFlag = TRUE;
    RegistrySettings.InstanceFlag = TRUE;
    RegistrySettings.SkidFlag = TRUE;
    RegistrySettings.CarType = 0;
    RegistrySettings.ScreenWidth = 640;
    RegistrySettings.ScreenHeight = 480;
    RegistrySettings.ScreenBpp = 32;  //$MODIFIED: originally set to 16
//$REMOVED    RegistrySettings.DrawDevice = 1;    // attempt to default to voodoo!
    RegistrySettings.Brightness = 256;
    RegistrySettings.Contrast = 256;
    RegistrySettings.DrawDist = 4;
    RegistrySettings.Texture24 = TRUE;  //$MODIFIED: originally was FALSE
    RegistrySettings.Vsync = FALSE; //$MODIFIED: originally was TRUE (and should go back to that for shipping game!)
#ifdef SHIPPING
    RegistrySettings.ShowFPS = FALSE;
#else
    RegistrySettings.ShowFPS = TRUE; //$MODIFIED: originally was FALSE (and should go back to that for shipping game!)
#endif
    RegistrySettings.SfxChannels = 16;
    RegistrySettings.MusicOn = TRUE;
    RegistrySettings.Joystick = 0; //$MODIFIED: originally got set to -1
    RegistrySettings.SteeringDeadzone = 10;
    RegistrySettings.SteeringRange = 90;
    RegistrySettings.ParticleLevel = 2;
    RegistrySettings.PlayMode = MODE_ARCADE;

    RegistrySettings.PickupFlag = TRUE;
    RegistrySettings.NLaps = DEFAULT_RACE_LAPS;
    RegistrySettings.NCars = DEFAULT_RACE_CARS;
    RegistrySettings.SfxVol = SFX_DEFAULT_VOL;
    RegistrySettings.MusicVol = MUSIC_DEFAULT_VOL;

    RegistrySettings.PositionSave = FALSE; //$ADDITION(jedl) - dave mccoy wants to start where he left off for tuning the art
    RegistrySettings.bUseGPU = FALSE; //$ADDITION(jedl) - allow toggling between new pipeline and old pipeline
//$NOTE(cprince): setting 'bUseGPU' to false by default.  Seeing glitches after merging Acclaim drop.  (It's screwing up some texture stage states?)
    RegistrySettings.bGraphicsDebug = FALSE;//$ADDITION(jedl) - make PositionSave and bUseGPU harder to toggle by default.
    // $NOTE(jedl): Right now, the only way to set bGraphicsDebug is to manually edit the T:\AppSettings.txt file (because code path that toggles the var is unreachable).
    RegistrySettings.VoiceMaskPreset = 0; //$ADDITION(jharding) - Default voice mask to Dark Master

    CRD_SetDefaultControls();

    //GetUserName(RegistrySettings.PlayerName, &size);
    strncpy(RegistrySettings.PlayerName, "Player", MAX_PLAYER_NAME);
    //RegistrySettings.PlayerName[MAX_PLAYER_NAME-1] = '\0';
    sprintf(RegistrySettings.LevelDir, "D:\\levels\\NHOOD1");
    RegistrySettings.HostComputer[0] = 0;

    ZeroMemory(&LevelSecrets, sizeof(LevelSecrets));
    LevelSecrets.Checksum = GetMemChecksum(LevelSecrets.LevelFlag, sizeof(LevelSecrets.LevelFlag));

    ZeroMemory(&StarList, sizeof(StarList));
    StarList.NumTotal = CountFileStars(LEVEL_STUNT_ARENA);
    StarList.Checksum = GetMemChecksum(&StarList.NumFound, sizeof(StarList) - sizeof(long));

#ifndef XBOX_NOT_YET_IMPLEMENTED
// create or open key

    r = RegCreateKeyEx(REGISTRY_ROOT, RegistryKey, 0, "Revolt", REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &key, &flag);
    if (r != ERROR_SUCCESS) return;

// attempt to overwrite defaults

    GET_REGISTRY_VALUE(key, "EnvFlag", &RegistrySettings.EnvFlag, 4);
    GET_REGISTRY_VALUE(key, "MirrorFlag", &RegistrySettings.MirrorFlag, 4);
    GET_REGISTRY_VALUE(key, "AutoBrake", &RegistrySettings.AutoBrake, 4);
    GET_REGISTRY_VALUE(key, "ShadowFlag", &RegistrySettings.ShadowFlag, 4);
    GET_REGISTRY_VALUE(key, "LightFlag", &RegistrySettings.LightFlag, 4);
    GET_REGISTRY_VALUE(key, "InstanceFlag", &RegistrySettings.InstanceFlag, 4);
    GET_REGISTRY_VALUE(key, "SkidFlag", &RegistrySettings.SkidFlag, 4);
    GET_REGISTRY_VALUE(key, "CarType", &RegistrySettings.CarType, 4);
    GET_REGISTRY_VALUE(key, "ScreenWidth", &RegistrySettings.ScreenWidth, 4);
    GET_REGISTRY_VALUE(key, "ScreenHeight", &RegistrySettings.ScreenHeight, 4);
    GET_REGISTRY_VALUE(key, "ScreenBpp", &RegistrySettings.ScreenBpp, 4);
//$REMOVED    GET_REGISTRY_VALUE(key, "DrawDevice", &RegistrySettings.DrawDevice, 4);
    GET_REGISTRY_VALUE(key, "Brightness", &RegistrySettings.Brightness, 4);
    GET_REGISTRY_VALUE(key, "Contrast", &RegistrySettings.Contrast, 4);
    GET_REGISTRY_VALUE(key, "DrawDist", &RegistrySettings.DrawDist, 4);
    GET_REGISTRY_VALUE(key, "Texture24", &RegistrySettings.Texture24, 4);
    GET_REGISTRY_VALUE(key, "Vsync", &RegistrySettings.Vsync, 4);
    GET_REGISTRY_VALUE(key, "ShowFPS", &RegistrySettings.ShowFPS, 4);
    GET_REGISTRY_VALUE(key, "SfxChannels", &RegistrySettings.SfxChannels, 4);
    GET_REGISTRY_VALUE(key, "MusicOn", &RegistrySettings.MusicOn, 4);
    GET_REGISTRY_VALUE(key, "Joystick", &RegistrySettings.Joystick, 4);
    GET_REGISTRY_VALUE(key, "SteeringDeadzone", &RegistrySettings.SteeringDeadzone, 4);
    GET_REGISTRY_VALUE(key, "SteeringRange", &RegistrySettings.SteeringRange, 4);
    GET_REGISTRY_VALUE(key, "ParticleLevel", &RegistrySettings.ParticleLevel, 4);
    GET_REGISTRY_VALUE(key, "PlayMode", &RegistrySettings.PlayMode, 4);

    GET_REGISTRY_VALUE(key, "PickupFlag", &RegistrySettings.PickupFlag, 4); //$RENAMED: changed registry key from "Pickups" to "PickupFlag" (to match var name)
    GET_REGISTRY_VALUE(key, "NLaps", &RegistrySettings.NLaps, 4);
    GET_REGISTRY_VALUE(key, "SfxVol", &RegistrySettings.SfxVol, 4); //$RENAMED: changed registry key from "SFXVol" to "SfxVol" (to match var name)
    GET_REGISTRY_VALUE(key, "MusicVol", &RegistrySettings.MusicVol, 4); 

    GET_REGISTRY_VALUE(key, "KeyLeft", &RegistrySettings.KeyLeft, 4);
    GET_REGISTRY_VALUE(key, "KeyRight", &RegistrySettings.KeyRight, 4);
    GET_REGISTRY_VALUE(key, "KeyFwd", &RegistrySettings.KeyFwd, 4);
    GET_REGISTRY_VALUE(key, "KeyBack", &RegistrySettings.KeyBack, 4);
    GET_REGISTRY_VALUE(key, "KeyFire", &RegistrySettings.KeyFire, 4);
    GET_REGISTRY_VALUE(key, "KeyReset", &RegistrySettings.KeyReset, 4);
    GET_REGISTRY_VALUE(key, "KeyReposition", &RegistrySettings.KeyReposition, 4);
    GET_REGISTRY_VALUE(key, "KeyHonka", &RegistrySettings.KeyHonka, 4);
    GET_REGISTRY_VALUE(key, "KeyHandbrake", &RegistrySettings.KeyHandbrake, 4);
    GET_REGISTRY_VALUE(key, "KeySelWeapon", &RegistrySettings.KeySelWeapon, 4);
    GET_REGISTRY_VALUE(key, "KeySelCamera", &RegistrySettings.KeySelCamera, 4);
    GET_REGISTRY_VALUE(key, "KeyFullBrake", &RegistrySettings.KeyFullBrake, 4);
    GET_REGISTRY_VALUE(key, "KeyPause", &RegistrySettings.KeyPause, 4);

    GET_REGISTRY_VALUE(key, "PlayerName", RegistrySettings.PlayerName, MAX_PLAYER_NAME);
    GET_REGISTRY_VALUE(key, "LevelName", RegistrySettings.LevelName, MAX_LEVEL_DIR_NAME);
    GET_REGISTRY_VALUE(key, "HostComputer", RegistrySettings.HostComputer, MAX_HOST_COMPUTER);
    GET_REGISTRY_VALUE(key, "VoiceMaskPreset", RegistrySettings.VoiceMaskPreset, 4);

    //$NOTE: not yet handling JedL vars here (eg, PositionSave and bUseGPU)

    ZeroMemory(&ls, sizeof(ls));
    GET_REGISTRY_VALUE(key, "LevelSecrets", &ls, sizeof(ls));
    if (GetMemChecksum(ls.LevelFlag, sizeof(ls.LevelFlag)) == ls.Checksum)
    {
        LevelSecrets = ls;
    }

    ZeroMemory(&sl, sizeof(sl));
    GET_REGISTRY_VALUE(key, "StarList", &sl, sizeof(sl));
    if (GetMemChecksum(&sl.NumFound, sizeof(sl) - sizeof(long)) == sl.Checksum)
    {
        StarList = sl;
    }
#endif // ! XBOX_NOT_YET_IMPLEMENTED

//$ADDITION(jedl) - fake registry
    //$PERF(cprince) - This is pretty inefficient.  Consider replacing it.
    FILE *fp = fopen("T:\\AppSettings.txt", "rb");
    if (fp != NULL)
    {
        // verify signature
        if(!VerifySignature("T:\\AppSettings.txt"))
        {
            g_bSigFailure = true;
            fclose(fp);
            DeleteFile("T:\\AppSettings.txt");
           
        }
        else
        {
            while (!feof(fp))
            {
                const int buflen = 100;
                CHAR buf[buflen];
		        if (fgets(buf, buflen, fp) == NULL)	// get next line, including newline
			        break;							// stop on end of file
                if( strlen(buf) > 0 )
                    buf[strlen(buf)-1] = 0;		    // strip off newline

        #define MATCH(token)                          (strncmp(buf, #token, strlen(#token)) == 0)
        #define EXTRACT_REGISTRY_STRING(token,string) (strcpy(string,&buf[strlen(#token)+1]))
        #define EXTRACT_REGISTRY_INT(token,intvar)    (intvar = atoi(&buf[strlen(#token)+1]))

                if     ( MATCH(EnvFlag) )        EXTRACT_REGISTRY_INT( EnvFlag, RegistrySettings.EnvFlag );
                else if( MATCH(MirrorFlag) )     EXTRACT_REGISTRY_INT( MirrorFlag, RegistrySettings.MirrorFlag );
                else if( MATCH(AutoBrake) )      EXTRACT_REGISTRY_INT( AutoBrake, RegistrySettings.AutoBrake );
                else if( MATCH(ShadowFlag) )     EXTRACT_REGISTRY_INT( ShadowFlag, RegistrySettings.ShadowFlag );
                else if( MATCH(LightFlag) )      EXTRACT_REGISTRY_INT( LightFlag, RegistrySettings.LightFlag );
                else if( MATCH(InstanceFlag) )   EXTRACT_REGISTRY_INT( InstanceFlag, RegistrySettings.InstanceFlag );
                else if( MATCH(SkidFlag) )       EXTRACT_REGISTRY_INT( SkidFlag, RegistrySettings.SkidFlag );
                else if( MATCH(CarType) )        EXTRACT_REGISTRY_INT( CarType, RegistrySettings.CarType );
                else if( MATCH(ScreenWidth) )    EXTRACT_REGISTRY_INT( ScreenWidth, RegistrySettings.ScreenWidth );
                else if( MATCH(ScreenHeight) )   EXTRACT_REGISTRY_INT( ScreenHeight, RegistrySettings.ScreenHeight );
                else if( MATCH(ScreenBpp) )      EXTRACT_REGISTRY_INT( ScreenBpp, RegistrySettings.ScreenBpp );
        //$REMOVED  else if( MATCH(DrawDevice) )     EXTRACT_REGISTRY_INT( DrawDevice, RegistrySettings.DrawDevice );
                else if( MATCH(Brightness) )     EXTRACT_REGISTRY_INT( Brightness, RegistrySettings.Brightness );
                else if( MATCH(Contrast) )       EXTRACT_REGISTRY_INT( Contrast, RegistrySettings.Contrast );
                else if( MATCH(DrawDist) )       EXTRACT_REGISTRY_INT( DrawDist, RegistrySettings.DrawDist );
                else if( MATCH(Texture24) )      EXTRACT_REGISTRY_INT( Texture24, RegistrySettings.Texture24 );
                else if( MATCH(Vsync) )          EXTRACT_REGISTRY_INT( Vsync, RegistrySettings.Vsync );
                else if( MATCH(ShowFPS) )        EXTRACT_REGISTRY_INT( ShowFPS, RegistrySettings.ShowFPS );
                else if( MATCH(SfxChannels) )    EXTRACT_REGISTRY_INT( SfxChannels, RegistrySettings.SfxChannels );
                else if( MATCH(MusicOn) )        EXTRACT_REGISTRY_INT( MusicOn, RegistrySettings.MusicOn );
                else if( MATCH(Joystick) )       EXTRACT_REGISTRY_INT( Joystick, RegistrySettings.Joystick );
                else if( MATCH(SteeringDeadzone) ) EXTRACT_REGISTRY_INT( SteeringDeadzone, RegistrySettings.SteeringDeadzone );
                else if( MATCH(SteeringRange) )  EXTRACT_REGISTRY_INT( SteeringRange, RegistrySettings.SteeringRange );
                else if( MATCH(ParticleLevel) )  EXTRACT_REGISTRY_INT( ParticleLevel, RegistrySettings.ParticleLevel );
                else if( MATCH(PlayMode) )       EXTRACT_REGISTRY_INT( PlayMode, RegistrySettings.PlayMode );

                else if( MATCH(PickupFlag) )     EXTRACT_REGISTRY_INT( PickupFlag, RegistrySettings.PickupFlag );
                else if( MATCH(NLaps) )          EXTRACT_REGISTRY_INT( NLaps, RegistrySettings.NLaps );
                else if( MATCH(SfxVol) )         EXTRACT_REGISTRY_INT( SfxVol, RegistrySettings.SfxVol );
                else if( MATCH(MusicVol) )       EXTRACT_REGISTRY_INT( MusicVol, RegistrySettings.MusicVol ); 

                else if( MATCH(KeyLeft) )        EXTRACT_REGISTRY_INT( KeyLeft, RegistrySettings.KeyLeft );
                else if( MATCH(KeyRight) )       EXTRACT_REGISTRY_INT( KeyRight, RegistrySettings.KeyRight );
                else if( MATCH(KeyFwd) )         EXTRACT_REGISTRY_INT( KeyFwd, RegistrySettings.KeyFwd );
                else if( MATCH(KeyBack) )        EXTRACT_REGISTRY_INT( KeyBack, RegistrySettings.KeyBack );
                else if( MATCH(KeyFire) )        EXTRACT_REGISTRY_INT( KeyFire, RegistrySettings.KeyFire );
                else if( MATCH(KeyReset) )       EXTRACT_REGISTRY_INT( KeyReset, RegistrySettings.KeyReset );
                else if( MATCH(KeyReposition) )  EXTRACT_REGISTRY_INT( KeyReposition, RegistrySettings.KeyReposition );
                else if( MATCH(KeyHonka) )       EXTRACT_REGISTRY_INT( KeyHonka, RegistrySettings.KeyHonka );
                else if( MATCH(KeyHandbrake) )   EXTRACT_REGISTRY_INT( KeyHandbrake, RegistrySettings.KeyHandbrake );
                else if( MATCH(KeySelWeapon) )   EXTRACT_REGISTRY_INT( KeySelWeapon, RegistrySettings.KeySelWeapon );
                else if( MATCH(KeySelCamera) )   EXTRACT_REGISTRY_INT( KeySelCamera, RegistrySettings.KeySelCamera );
                else if( MATCH(KeyFullBrake) )   EXTRACT_REGISTRY_INT( KeyFullBrake, RegistrySettings.KeyFullBrake );
                else if( MATCH(KeyPause) )       EXTRACT_REGISTRY_INT( KeyPause, RegistrySettings.KeyPause );

                // string vars
                else if( MATCH(PlayerName) )     EXTRACT_REGISTRY_STRING( PlayerName, RegistrySettings.PlayerName );
                else if( MATCH(LevelDir ) )      EXTRACT_REGISTRY_STRING( LevelDir, RegistrySettings.LevelDir );
                else if( MATCH(HostComputer) )   EXTRACT_REGISTRY_STRING( HostComputer, RegistrySettings.HostComputer );

                // new vars
                else if( MATCH(PositionSave) )   EXTRACT_REGISTRY_INT( PositionSave, RegistrySettings.PositionSave ); //$ADDITION(jedl) - dave mccoy wants to start where he left off for tuning the art
                else if( MATCH(bUseGPU) )        EXTRACT_REGISTRY_INT( bUseGPU, RegistrySettings.bUseGPU ); //$ADDITION(jedl) - allow toggling b/w new pipeline and old pipeline
                else if( MATCH(bGraphicsDebug) ) EXTRACT_REGISTRY_INT( bGraphicsDebug, RegistrySettings.bGraphicsDebug ); //$ADDITION(jedl) - suppress PositionSave and bUseGPU
                else if( MATCH(VoiceMaskPreset) ) EXTRACT_REGISTRY_INT( VoiceMaskPreset, RegistrySettings.VoiceMaskPreset ); //$ADDITION(jharding)

                //$NOTE: not yet handling "LevelSecrets" and "StarList" settings here.
        #undef MATCH
        #undef EXTRACT_REGISTRY_STRING
        #undef EXTRACT_REGISTRY_INT
                else
                {
                    OutputDebugString("Unknown AppSetting string: ");
                    OutputDebugString(buf);
                }
            }
        }
        if(fp != NULL)
            fclose(fp);
    }
    if (!RegistrySettings.bGraphicsDebug)
    {
        // $NOTE(jedL) - if bGraphicsDebug is false, then make sure PositionSave and bUseGPU are turned off
        RegistrySettings.PositionSave = FALSE;
        RegistrySettings.bUseGPU = FALSE;
    }
//$END_ADDITION


#pragma warning(disable:4800) //  disable long-to-bool conversion moaning
    gTitleScreenVars.shinyness = RegistrySettings.EnvFlag;
    gTitleScreenVars.reflections = RegistrySettings.MirrorFlag;
    GameSettings.AutoBrake = RegistrySettings.AutoBrake;
    gTitleScreenVars.shadows = RegistrySettings.ShadowFlag;

    gTitleScreenVars.lights = RegistrySettings.LightFlag;
    gTitleScreenVars.instances = RegistrySettings.InstanceFlag;
    gTitleScreenVars.skidmarks = RegistrySettings.SkidFlag;
    gTitleScreenVars.PlayerData[0].iCarNum = RegistrySettings.CarType;

    gTitleScreenVars.pickUps = RegistrySettings.PickupFlag;
#pragma warning(default:4800)
    gTitleScreenVars.numberOfLaps = RegistrySettings.NLaps;
    gTitleScreenVars.numberOfCars = RegistrySettings.NCars;
    gTitleScreenVars.sfxVolume = RegistrySettings.SfxVol;
    gTitleScreenVars.musicVolume = RegistrySettings.MusicVol;
    gTitleScreenVars.sparkLevel = RegistrySettings.ParticleLevel;
    gTitleScreenVars.playMode = RegistrySettings.PlayMode;
    strncpy(gTitleScreenVars.PlayerData[0].nameEnter, RegistrySettings.PlayerName, MAX_PLAYER_NAME);
    gTitleScreenVars.PlayerData[0].nameEnter[MAX_PLAYER_NAME-1] = '\0';  //$HEY: is this necessary?  (Not reqd if source player name is guaranteed to be ok.)

    CRD_SetControlsFromRegistry();

// command line overrides

//$REMOVED    if (ForceDrawDevice != -1)
//$REMOVED    {
//$REMOVED        RegistrySettings.DrawDevice = ForceDrawDevice;
//$REMOVED    }

    if (ForceScreenWidth && ForceScreenHeight && ForceScreenBpp)
    {
        RegistrySettings.ScreenWidth = ForceScreenWidth;
        RegistrySettings.ScreenHeight = ForceScreenHeight;
        RegistrySettings.ScreenBpp = ForceScreenBpp;
    }

    if (ForceTextureBpp != -1)
    {
        RegistrySettings.Texture24 = ForceTextureBpp;
    }

//$REMOVED
//// security check?
//
//#if REGISTRY_SECURITY_CHECK
//    flag = 0;
//    GET_REGISTRY_VALUE(key, "WindowType", &flag, 4);
//    if (!flag)
//    {
//        DumpMessage(NULL, "Illegal copy of Revolt!");
//        QuitGame();
//        return;
//    }
//#endif
//$END_REMOVAL

#ifndef XBOX_NOT_YET_IMPLEMENTED
// write game version for 'zone'

    SET_REGISTRY_VALUE(key, "Version", REG_SZ, VersionString, strlen(VersionString));

// close key

    RegCloseKey(key);
#endif // ! XBOX_NOT_YET_IMPLEMENTED
}

////////////////////////////
// save registry settings //
////////////////////////////

void SetRegistrySettings(void)
{
#ifndef XBOX_NOT_YET_IMPLEMENTED
    long r;
    HKEY key;

// open key

    r = RegOpenKeyEx(REGISTRY_ROOT, RegistryKey, 0, KEY_ALL_ACCESS, &key);
    if (r != ERROR_SUCCESS) return;
#endif // ! XBOX_NOT_YET_IMPLEMENTED

// write registry settings

    RegistrySettings.KeyLeft = *(DWORD*)&KeyTable[KEY_LEFT];
    RegistrySettings.KeyRight = *(DWORD*)&KeyTable[KEY_RIGHT];
    RegistrySettings.KeyFwd = *(DWORD*)&KeyTable[KEY_FWD];
    RegistrySettings.KeyBack = *(DWORD*)&KeyTable[KEY_BACK];
    RegistrySettings.KeyFire = *(DWORD*)&KeyTable[KEY_FIRE];
    RegistrySettings.KeyReset = *(DWORD*)&KeyTable[KEY_RESET];
    RegistrySettings.KeyReposition = *(DWORD*)&KeyTable[KEY_REPOSITION];
    RegistrySettings.KeyHonka = *(DWORD*)&KeyTable[KEY_HONKA];
//  RegistrySettings.KeyHandbrake = *(DWORD*)&KeyTable[KEY_HANDBRAKE];
    RegistrySettings.KeyHandbrake = *(DWORD*)&KeyTable[KEY_REARVIEW];
    RegistrySettings.KeySelWeapon = *(DWORD*)&KeyTable[KEY_SELWEAPON];
    RegistrySettings.KeySelCamera = *(DWORD*)&KeyTable[KEY_SELCAMERAMODE];
    RegistrySettings.KeyFullBrake = *(DWORD*)&KeyTable[KEY_FULLBRAKE];
    RegistrySettings.KeyPause = *(DWORD*)&KeyTable[KEY_PAUSE];

    RegistrySettings.EnvFlag = gTitleScreenVars.shinyness;
    RegistrySettings.MirrorFlag = gTitleScreenVars.reflections;
    RegistrySettings.AutoBrake = GameSettings.AutoBrake;
    RegistrySettings.ShadowFlag = gTitleScreenVars.shadows;
    RegistrySettings.LightFlag = gTitleScreenVars.lights;
    RegistrySettings.InstanceFlag = gTitleScreenVars.instances;
    RegistrySettings.SkidFlag = gTitleScreenVars.skidmarks;
    RegistrySettings.PickupFlag = gTitleScreenVars.pickUps;
    RegistrySettings.CarType = gTitleScreenVars.PlayerData[0].iCarNum;
    RegistrySettings.NLaps = gTitleScreenVars.numberOfLaps;
    RegistrySettings.NCars = gTitleScreenVars.numberOfCars;
    RegistrySettings.SfxVol = gTitleScreenVars.sfxVolume;
    RegistrySettings.MusicVol = gTitleScreenVars.musicVolume;
    RegistrySettings.ParticleLevel = gTitleScreenVars.sparkLevel;
    RegistrySettings.PlayMode = gTitleScreenVars.playMode;

    strncpy(RegistrySettings.PlayerName, gTitleScreenVars.PlayerData[0].nameEnter, MAX_PLAYER_NAME);
    RegistrySettings.PlayerName[MAX_PLAYER_NAME-1] = '\0';  //$HEY: is this necessary?  (Not reqd if source player name is guaranteed to be ok.)

#ifndef XBOX_NOT_YET_IMPLEMENTED
    SET_REGISTRY_VALUE(key, "EnvFlag", REG_DWORD, &RegistrySettings.EnvFlag, 4);
    SET_REGISTRY_VALUE(key, "MirrorFlag", REG_DWORD, &RegistrySettings.MirrorFlag, 4);
    SET_REGISTRY_VALUE(key, "AutoBrake", REG_DWORD, &RegistrySettings.AutoBrake, 4);
    SET_REGISTRY_VALUE(key, "ShadowFlag", REG_DWORD, &RegistrySettings.ShadowFlag, 4);
    SET_REGISTRY_VALUE(key, "LightFlag", REG_DWORD, &RegistrySettings.LightFlag, 4);
    SET_REGISTRY_VALUE(key, "InstanceFlag", REG_DWORD, &RegistrySettings.InstanceFlag, 4);
    SET_REGISTRY_VALUE(key, "SkidFlag", REG_DWORD, &RegistrySettings.SkidFlag, 4);
    SET_REGISTRY_VALUE(key, "CarType", REG_DWORD, &RegistrySettings.CarType, 4);
    SET_REGISTRY_VALUE(key, "ScreenWidth", REG_DWORD, &RegistrySettings.ScreenWidth, 4);
    SET_REGISTRY_VALUE(key, "ScreenHeight", REG_DWORD, &RegistrySettings.ScreenHeight, 4);
    SET_REGISTRY_VALUE(key, "ScreenBpp", REG_DWORD, &RegistrySettings.ScreenBpp, 4);
//$REMOVED    SET_REGISTRY_VALUE(key, "DrawDevice", REG_DWORD, &RegistrySettings.DrawDevice, 4);
    SET_REGISTRY_VALUE(key, "Brightness", REG_DWORD, &RegistrySettings.Brightness, 4);
    SET_REGISTRY_VALUE(key, "Contrast", REG_DWORD, &RegistrySettings.Contrast, 4);
    SET_REGISTRY_VALUE(key, "DrawDist", REG_DWORD, &RegistrySettings.DrawDist, 4);
    SET_REGISTRY_VALUE(key, "Texture24", REG_DWORD, &RegistrySettings.Texture24, 4);
    SET_REGISTRY_VALUE(key, "Vsync", REG_DWORD, &RegistrySettings.Vsync, 4);
    SET_REGISTRY_VALUE(key, "ShowFPS", REG_DWORD, &RegistrySettings.ShowFPS, 4);
    SET_REGISTRY_VALUE(key, "SfxChannels", REG_DWORD, &RegistrySettings.SfxChannels, 4);
    SET_REGISTRY_VALUE(key, "MusicOn", REG_DWORD, &RegistrySettings.MusicOn, 4);
    SET_REGISTRY_VALUE(key, "Joystick", REG_DWORD, &RegistrySettings.Joystick, 4);
    SET_REGISTRY_VALUE(key, "SteeringDeadzone", REG_DWORD, &RegistrySettings.SteeringDeadzone, 4);
    SET_REGISTRY_VALUE(key, "SteeringRange", REG_DWORD, &RegistrySettings.SteeringRange, 4);
    SET_REGISTRY_VALUE(key, "ParticleLevel", REG_DWORD, &RegistrySettings.ParticleLevel, 4);
    SET_REGISTRY_VALUE(key, "PlayMode", REG_DWORD, &RegistrySettings.PlayMode, 4);

    SET_REGISTRY_VALUE(key, "NLaps", REG_DWORD, &RegistrySettings.NLaps, 4);
    SET_REGISTRY_VALUE(key, "NCars", REG_DWORD, &RegistrySettings.NCars, 4);
    SET_REGISTRY_VALUE(key, "SfxVol", REG_DWORD, &RegistrySettings.SfxVol, 4); //$RENAMED: changed registry key from "SFXVol" to "SfxVol" (to match var name)
    SET_REGISTRY_VALUE(key, "MusicVol", REG_DWORD, &RegistrySettings.MusicVol, 4);
    SET_REGISTRY_VALUE(key, "PickupFlag", REG_DWORD, &RegistrySettings.PickupFlag, 4); //$RENAMED: changed registry key from "Pickups" to "PickupFlag" (to match var name)

    SET_REGISTRY_VALUE(key, "KeyLeft", REG_DWORD, &RegistrySettings.KeyLeft, 4);
    SET_REGISTRY_VALUE(key, "KeyRight", REG_DWORD, &RegistrySettings.KeyRight, 4);
    SET_REGISTRY_VALUE(key, "KeyFwd", REG_DWORD, &RegistrySettings.KeyFwd, 4);
    SET_REGISTRY_VALUE(key, "KeyBack", REG_DWORD, &RegistrySettings.KeyBack, 4);
    SET_REGISTRY_VALUE(key, "KeyFire", REG_DWORD, &RegistrySettings.KeyFire, 4);
    SET_REGISTRY_VALUE(key, "KeyReset", REG_DWORD, &RegistrySettings.KeyReset, 4);
    SET_REGISTRY_VALUE(key, "KeyReposition", REG_DWORD, &RegistrySettings.KeyReposition, 4);
    SET_REGISTRY_VALUE(key, "KeyHonka", REG_DWORD, &RegistrySettings.KeyHonka, 4);
    SET_REGISTRY_VALUE(key, "KeyHandbrake", REG_DWORD, &RegistrySettings.KeyHandbrake, 4);
    SET_REGISTRY_VALUE(key, "KeySelWeapon", REG_DWORD, &RegistrySettings.KeySelWeapon, 4);
    SET_REGISTRY_VALUE(key, "KeySelCamera", REG_DWORD, &RegistrySettings.KeySelCamera, 4);
    SET_REGISTRY_VALUE(key, "KeyFullBrake", REG_DWORD, &RegistrySettings.KeyFullBrake, 4);
    SET_REGISTRY_VALUE(key, "KeyPause", REG_DWORD, &RegistrySettings.KeyPause, 4);

    SET_REGISTRY_VALUE(key, "PlayerName", REG_SZ, RegistrySettings.PlayerName, MAX_PLAYER_NAME);
    SET_REGISTRY_VALUE(key, "LevelDir", REG_SZ, RegistrySettings.LevelDir, MAX_LEVEL_DIR_NAME);
    SET_REGISTRY_VALUE(key, "HostComputer", REG_SZ, RegistrySettings.HostComputer, MAX_HOST_COMPUTER);
    SET_REGISTRY_VALUE(key, "VoiceMaskPreset", REG_SZ, RegistrySettings.VoiceMaskPreset, 4);

    //$NOTE: not yet handling JedL vars here (eg, PositionSave and bUseGPU)

    LevelSecrets.Checksum = GetMemChecksum(LevelSecrets.LevelFlag, sizeof(LevelSecrets.LevelFlag));
    SET_REGISTRY_VALUE(key, "LevelSecrets", REG_BINARY, &LevelSecrets, sizeof(LevelSecrets));

    StarList.Checksum = GetMemChecksum(&StarList.NumFound, sizeof(StarList) - sizeof(long));
    SET_REGISTRY_VALUE(key, "StarList", REG_BINARY, &StarList, sizeof(StarList));

// close key

    RegCloseKey(key);
#endif // ! XBOX_NOT_YET_IMPLEMENTED

//$ADDITION(jedl) - fake registry
    FILE *fp = fopen("T:\\AppSettings.txt", "wb");
    if (fp != NULL)
    {
        fprintf(fp, "EnvFlag %d\n",  RegistrySettings.EnvFlag);
        fprintf(fp, "MirrorFlag %d\n",  RegistrySettings.MirrorFlag);
        fprintf(fp, "AutoBrake %d\n",  RegistrySettings.AutoBrake);
        fprintf(fp, "ShadowFlag %d\n",  RegistrySettings.ShadowFlag);
        fprintf(fp, "LightFlag %d\n",  RegistrySettings.LightFlag);
        fprintf(fp, "InstanceFlag %d\n",  RegistrySettings.InstanceFlag);
        fprintf(fp, "SkidFlag %d\n",  RegistrySettings.SkidFlag);
        fprintf(fp, "CarType %d\n",  RegistrySettings.CarType);
        fprintf(fp, "ScreenWidth %d\n",  RegistrySettings.ScreenWidth);
        fprintf(fp, "ScreenHeight %d\n",  RegistrySettings.ScreenHeight);
        fprintf(fp, "ScreenBpp %d\n",  RegistrySettings.ScreenBpp);
//$REMOVED        fprintf(fp, "DrawDevice %d\n",  RegistrySettings.DrawDevice);
        fprintf(fp, "Brightness %d\n",  RegistrySettings.Brightness);
        fprintf(fp, "Contrast %d\n",  RegistrySettings.Contrast);
        fprintf(fp, "DrawDist %d\n",  RegistrySettings.DrawDist);
        fprintf(fp, "Texture24 %d\n",  RegistrySettings.Texture24);
        fprintf(fp, "Vsync %d\n",  RegistrySettings.Vsync);
        fprintf(fp, "ShowFPS %d\n",  RegistrySettings.ShowFPS);
        fprintf(fp, "SfxChannels %d\n",  RegistrySettings.SfxChannels);
        fprintf(fp, "MusicOn %d\n",  RegistrySettings.MusicOn);
        fprintf(fp, "Joystick %d\n",  RegistrySettings.Joystick);
        fprintf(fp, "SteeringDeadzone %d\n",  RegistrySettings.SteeringDeadzone);
        fprintf(fp, "SteeringRange %d\n",  RegistrySettings.SteeringRange);
        fprintf(fp, "ParticleLevel %d\n",  RegistrySettings.ParticleLevel);
        fprintf(fp, "PlayMode %d\n",  RegistrySettings.PlayMode);

        fprintf(fp, "PickupFlag %d\n",  RegistrySettings.PickupFlag);
        fprintf(fp, "NLaps %d\n",  RegistrySettings.NLaps);
        fprintf(fp, "SfxVol %d\n",  RegistrySettings.SfxVol);
        fprintf(fp, "MusicVol %d\n",  RegistrySettings.MusicVol);

        fprintf(fp, "KeyLeft %d\n",  RegistrySettings.KeyLeft);
        fprintf(fp, "KeyRight %d\n",  RegistrySettings.KeyRight);
        fprintf(fp, "KeyFwd %d\n",  RegistrySettings.KeyFwd);
        fprintf(fp, "KeyBack %d\n",  RegistrySettings.KeyBack);
        fprintf(fp, "KeyFire %d\n",  RegistrySettings.KeyFire);
        fprintf(fp, "KeyReset %d\n",  RegistrySettings.KeyReset);
        fprintf(fp, "KeyReposition %d\n",  RegistrySettings.KeyReposition);
        fprintf(fp, "KeyHonka %d\n",  RegistrySettings.KeyHonka);
        fprintf(fp, "KeyHandbrake %d\n",  RegistrySettings.KeyHandbrake);
        fprintf(fp, "KeySelWeapon %d\n",  RegistrySettings.KeySelWeapon);
        fprintf(fp, "KeySelCamera %d\n",  RegistrySettings.KeySelCamera);
        fprintf(fp, "KeyFullBrake %d\n",  RegistrySettings.KeyFullBrake);
        fprintf(fp, "KeyPause %d\n",  RegistrySettings.KeyPause);

        // string vars
        fprintf(fp, "PlayerName %s\n",  RegistrySettings.PlayerName);
        fprintf(fp, "LevelDir %s\n",  RegistrySettings.LevelDir);
        fprintf(fp, "HostComputer %s\n",  RegistrySettings.HostComputer);

        // new vars
        fprintf(fp, "PositionSave %d\n",  RegistrySettings.PositionSave);
        fprintf(fp, "bUseGPU %d\n",  RegistrySettings.bUseGPU);
        fprintf(fp, "bGraphicsDebug %d\n",  RegistrySettings.bGraphicsDebug);
        fprintf(fp, "VoiceMaskPreset %d\n", RegistrySettings.VoiceMaskPreset);

        //$NOTE: not yet handling "LevelSecrets" and "StarList" settings here.

        fclose(fp);
    }

#ifdef _DEBUG
    bool bSigSaved = SaveSignature("T:\\Appsettings.txt");
    assert(bSigSaved);
#else
    SaveSignature("T:\\Appsettings.txt");
#endif

//$END_ADDITION
}

