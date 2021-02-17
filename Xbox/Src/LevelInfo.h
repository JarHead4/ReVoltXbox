//-----------------------------------------------------------------------------
// File: LevelInfo.h
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef LEVELINFO_H
#define LEVELINFO_H

#include "revolt.h"
#include "competition.h"

#define MAX_LEVELS 256
//#define MAX_LEVEL_DIR_NAME 16
#define MAX_LEVEL_NAME 64
#define MAX_ENV_NAME 64
#define MAX_MP3_NAME 64

#define FILENAME_MAKE_BODY 1
#define FILENAME_GAME_SETTINGS 2


////////////////////////////////////////////////////////////////
//
// Enumeration for default levels that come with the game
//
////////////////////////////////////////////////////////////////
#ifdef _N64 

//#define _NO_BATTLE_

typedef enum DefaultLevelEnum {
    LEVEL_TOYWORLD1,
    LEVEL_TOYWORLD2,
    LEVEL_FRONTEND,
    LEVEL_MUSEUM1,
    LEVEL_MUSEUM2,
    LEVEL_GHOSTTOWN1,
    LEVEL_SUPERMARKET1,
    LEVEL_NEIGHBOURHOOD1,
    LEVEL_NEIGHBOURHOOD2,
    LEVEL_CRUISE1,
    LEVEL_NEIGHBOURHOOD_BATTLE,
    LEVEL_GARDEN1,
    LEVEL_SUPERMARKET2,
    LEVEL_GHOSTTOWN2,
    LEVEL_CRUISE2,

    LEVEL_NCUP_LEVELS,

    LEVEL_STUNT_ARENA = LEVEL_NCUP_LEVELS,

    
    LEVEL_MUSEUM_BATTLE,
    LEVEL_GARDEN_BATTLE,

    LEVEL_USER00,

    LEVEL_NSHIPPED_LEVELS,

} DEFAULT_LEVEL;


#else
typedef enum DefaultLevelEnum {
    LEVEL_NEIGHBOURHOOD1,
    LEVEL_SUPERMARKET2,
    LEVEL_MUSEUM2,
    LEVEL_GARDEN1,
    LEVEL_TOYWORLD1,
    LEVEL_GHOSTTOWN1,
    LEVEL_TOYWORLD2,
    LEVEL_NEIGHBOURHOOD2,
    LEVEL_CRUISE1,
    LEVEL_MUSEUM1,
    LEVEL_SUPERMARKET1,
    LEVEL_GHOSTTOWN2,
    LEVEL_CRUISE2,

    LEVEL_NCUP_LEVELS,

    LEVEL_FRONTEND = LEVEL_NCUP_LEVELS,
    LEVEL_INTRO,

    LEVEL_STUNT_ARENA,

    LEVEL_NEIGHBOURHOOD_BATTLE,
#ifdef _PC
    LEVEL_GARDEN_BATTLE,
    LEVEL_MARKET_BATTLE,
    LEVEL_MUSEUM_BATTLE,
#endif
    
#ifdef _PSX
//  LEVEL_TRACKEDITOR,
#endif

    LEVEL_NSHIPPED_LEVELS
} DEFAULT_LEVEL;
#endif
////////////////////////////////////////////////////////////////
//
// Track type - battle or race
//
////////////////////////////////////////////////////////////////

typedef enum TrackTypeEnum {
    TRACK_TYPE_NONE = -1,
    TRACK_TYPE_RACE = 0,
    TRACK_TYPE_BATTLE,
    TRACK_TYPE_TRAINING,
    TRACK_TYPE_USER,

    TRACK_NTYPES
} TRACK_TYPE;

////////////////////////////////////////////////////////////////
//
// Level information (Accessibility etc) for shipped levels
// (user levels are always available)
//
////////////////////////////////////////////////////////////////

#define LEVEL_SELECTABLE                1
#define LEVEL_AVAILABLE                 2
#define LEVEL_MIRROR_AVAILABLE          4
#define LEVEL_REVERSE_AVAILABLE         8
#define LEVEL_MIRROR_REVERSE_AVAILABLE  16

//$MD: added
#define LEVEL_ONLINEONLY                32

//----------------------------------
//#ifdef _N64 // N64 Macros
//----------------------------------

//#define IsLevelSelectable(_iLevel)                (GetLevelInfo((_iLevel))->ObtainFlags & LEVEL_SELECTABLE)
//#define IsLevelAvailable(_iLevel)             (GetLevelInfo((_iLevel))->ObtainFlags & LEVEL_AVAILABLE)
//#define IsLevelReverseAvailable(_iLevel)      (GetLevelInfo((_iLevel))->ObtainFlags & LEVEL_REVERSE_AVAILABLE)
//#define IsLevelMirrorAvailable(_iLevel)           (GetLevelInfo((_iLevel))->ObtainFlags & LEVEL_MIRROR_AVAILABLE)
//#define IsLevelMirrorReverseAvailable(_iLevel)    (GetLevelInfo((_iLevel))->ObtainFlags & LEVEL_MIRROR_REVERSE_AVAILABLE)

//extern bool IsLevelTypeAvailable(int iLevel, bool mirror, bool reverse);

//#define SetLevelAvailable(_iLevel)                (GetLevelInfo((_iLevel))->ObtainFlags |= LEVEL_AVAILABLE)
//#define SetLevelReverseAvailable(_iLevel)     (GetLevelInfo((_iLevel))->ObtainFlags |= LEVEL_REVERSE_AVAILABLE)
//#define SetLevelMirrorAvailable(_iLevel)      (GetLevelInfo((_iLevel))->ObtainFlags |= LEVEL_MIRROR_AVAILABLE)
//#define SetLevelMirrorReverseAvailable(_iLevel) (GetLevelInfo((_iLevel))->ObtainFlags |= LEVEL_MIRROR_REVERSE_AVAILABLE)
//#define SetLevelBoltObtained(_iLevel)         (GetLevelInfo((_iLevel))->ObtainFlags |= LEVEL_BOLT_OBTAINED)

//#define SetLevelUnavailable(_iLevel)          (GetLevelInfo((_iLevel))->ObtainFlags &= ~(LEVEL_AVAILABLE | LEVEL_REVERSE_AVAILABLE | LEVEL_MIRROR_AVAILABLE | LEVEL_MIRROR_REVERSE_AVAILABLE))

//----------------------------------
//#else  // PC
//----------------------------------

extern bool IsLevelSelectable(int iLevel);
extern bool IsLevelAvailable(int iLevel);
extern bool IsLevelReverseAvailable(int iLevel);
extern bool IsLevelMirrorAvailable(int iLevel);
extern bool IsLevelMirrorReverseAvailable(int iLevel);

extern bool IsLevelTypeAvailable(int iLevel, bool mirror, bool reverse);

extern void SetLevelSelectable(int iLevel);
extern void SetLevelUnselectable(int iLevel);
extern void SetLevelAvailable(int iLevel);
extern void SetLevelReverseAvailable(int iLevel);
extern void SetLevelMirrorAvailable(int iLevel);
extern void SetLevelMirrorReverseAvailable(int iLevel);

extern void SetLevelUnavailable(int iLevel);

//#endif
//----------------------------------

////////////////////////
// Level secret info //
////////////////////////

typedef struct {
    unsigned long Checksum;
    long LevelFlag[LEVEL_NCUP_LEVELS];
} LEVEL_SECRETS;

#define LEVEL_SECRET_BEAT_TIMETRIAL         1
#define LEVEL_SECRET_BEAT_TIMETRIAL_REVERSE 2
#define LEVEL_SECRET_BEAT_TIMETRIAL_MIRROR  4
#define LEVEL_SECRET_FOUND_PRACTISE_STARS   8
#define LEVEL_SECRET_WON_SINGLE_RACE        16
#define LEVEL_SECRET_CUP_COMPLETED          32

#define IsSecretBeatTimeTrial(lev)          (LevelSecrets.LevelFlag[lev] & LEVEL_SECRET_BEAT_TIMETRIAL)
#define IsSecretBeatTimeTrialReverse(lev)   (LevelSecrets.LevelFlag[lev] & LEVEL_SECRET_BEAT_TIMETRIAL_REVERSE)
#define IsSecretBeatTimeTrialMirror(lev)    (LevelSecrets.LevelFlag[lev] & LEVEL_SECRET_BEAT_TIMETRIAL_MIRROR)
#define IsSecretFoundPractiseStars(lev)     (LevelSecrets.LevelFlag[lev] & LEVEL_SECRET_FOUND_PRACTISE_STARS)
#define IsSecretWonSingleRace(lev)          (LevelSecrets.LevelFlag[lev] & LEVEL_SECRET_WON_SINGLE_RACE)
#define IsSecretCupCompleted(lev)           (LevelSecrets.LevelFlag[lev] & LEVEL_SECRET_CUP_COMPLETED)

#define SetSecretBeatTimeTrial(lev)         (LevelSecrets.LevelFlag[lev] |= LEVEL_SECRET_BEAT_TIMETRIAL)
#define SetSecretBeatTimeTrialReverse(lev)  (LevelSecrets.LevelFlag[lev] |= LEVEL_SECRET_BEAT_TIMETRIAL_REVERSE)
#define SetSecretBeatTimeTrialMirror(lev)   (LevelSecrets.LevelFlag[lev] |= LEVEL_SECRET_BEAT_TIMETRIAL_MIRROR)
#define SetSecretFoundPractiseStars(lev)    (LevelSecrets.LevelFlag[lev] |= LEVEL_SECRET_FOUND_PRACTISE_STARS)
#define SetSecretWonSingleRace(lev)         (LevelSecrets.LevelFlag[lev] |= LEVEL_SECRET_WON_SINGLE_RACE)
#define SetSecretCupCompleted(lev)          (LevelSecrets.LevelFlag[lev] |= LEVEL_SECRET_CUP_COMPLETED)

////////////////////////////
// star found list struct //
////////////////////////////

#define MAX_STAR_LIST 64

typedef struct {
    unsigned long Checksum;
    long NumFound, NumTotal;
    char ID[MAX_STAR_LIST];
} STAR_LIST;

////////////////////////////////////////////////////////////////
//
// Level info data structures
//
////////////////////////////////////////////////////////////////
#ifdef _PC
typedef struct LevelInfoStruct {
    
    // $MD: Directory and name info added
    char szName[MAX_LEVEL_INF_NAME]; 
    char szDir[MAX_PATH];

    WCHAR strName[MAX_LEVEL_NAME];

    // Track length in metres
    REAL Length;

    // Generic level information
    long ObtainFlags;
    RACE_CLASS LevelClass;
    TRACK_TYPE TrackType;

    // challenge times
    long ChallengeTimeNormal;
    long ChallengeTimeReversed;

} LEVELINFO;

typedef struct CurrentLevelInfoStruct {

    // Names
    char szName[MAX_LEVEL_INF_NAME];
    char szDir[MAX_PATH];

    WCHAR strName[MAX_LEVEL_NAME];

    // Level starting positions and start grid type
    VEC NormalStartPos, ReverseStartPos;
    float NormalStartRot, ReverseStartRot;
    long  NormalStartGrid, ReverseStartGrid;


    // Render settings
    char EnvStill[MAX_ENV_NAME];
    char EnvRoll[MAX_ENV_NAME];
    char Mp3[MAX_MP3_NAME];
    float FarClip;
    float FogStart;
    long FogColor;
    float VertFogStart;
    float VertFogEnd;
    long WorldRGBper;
    long ModelRGBper;
    long InstanceRGBper;
    long MirrorType;
    REAL MirrorMix;
    REAL MirrorIntensity;
    REAL MirrorDist;
    long RedbookStartTrack;
    long RedbookEndTrack;
    REAL RockX;
    REAL RockZ;
    REAL RockTimeX;
    REAL RockTimeZ;

} CURRENT_LEVELINFO;

#endif // _PC

#ifdef _PSX
typedef struct LevelInfoStruct {
    char *Name;
    SVECTOR StartPos;
    SVECTOR StartPosRev;
    short StartRot;
    short StartRotRev;
    long Fog;

    // Generic level information
    long ObtainFlags;                   // Accessibility settings
    RACE_CLASS LevelClass;              // Cup type
    TRACK_TYPE TrackType;               // Track type

    // challenge times
    long ChallengeTimeNormal;
    long ChallengeTimeReversed;

} LEVELINFO;

typedef struct CurrentLevelInfoStruct {

    // Names
    //char Dir[MAX_LEVEL_DIR_NAME];
    char Name[MAX_LEVEL_NAME];

    // Level starting positions and start grid type
    VEC NormalStartPos, ReverseStartPos;
    REAL NormalStartRot, ReverseStartRot;
    long  NormalStartGrid, ReverseStartGrid;


    // Render settings
    //char EnvStill[MAX_ENV_NAME];
    //char EnvRoll[MAX_ENV_NAME];
    //float FarClip;
    //float FogStart;
    long FogColor;
    //float VertFogStart;
    //float VertFogEnd;
    //long WorldRGBper;
    //long ModelRGBper;
    //long InstanceRGBper;
    //long MirrorType;
    //REAL MirrorMix;
    //REAL MirrorIntensity;
    //REAL MirrorDist;

} CURRENT_LEVELINFO;

#endif // _PSX

#ifdef _N64
typedef struct LevelInfoStruct{
    char    Name[20];
    char    ShortName[16];

    // N64 specific level information (derived from PC file xxxx.INF)
    VEC NormalStartPos, ReverseStartPos;
    float NormalStartRot, ReverseStartRot;
    long NormalStartGrid, ReverseStartGrid;
    float FarClip;
    float FogPercent;
    long FogColor;
    long WorldRGBper;
    long ModelRGBper;
    long InstanceRGBper;

    // Track length in metres
    REAL Length;

    // Generic level information
    long ObtainFlags;                   // Accessibility settings
    RACE_CLASS LevelClass;              // Cup type
    TRACK_TYPE TrackType;               // Track type

    // challenge times
    long ChallengeTimeNormal;
    long ChallengeTimeReversed;

    long FrontEndTmapID;
    unsigned Mp3;
    long userTrackNum;

} LEVELINFO;



typedef struct CurrentLevelInfoStruct
{
    // Names
    char Name[MAX_LEVEL_NAME];

    // Level starting positions and start grid type
    VEC NormalStartPos, ReverseStartPos;
    float NormalStartRot, ReverseStartRot;
    long  NormalStartGrid, ReverseStartGrid;

    // Render settings
    float    FarClip;
    float    FogPercent;
    long     FogColor;
    long     WorldRGBper;
    long     ModelRGBper;
    long     InstanceRGBper;
    unsigned Mp3;
} CURRENT_LEVELINFO;

#endif // _N64


////////////////////////////////////////////////////////////////
//
// Global function prototypes
//

extern LEVELINFO *GetLevelInfo(int levelNum);
extern long GetLevelNum(char *dir);
// $MD: removed extern void InitDefaultLevels();


#ifdef _PC
extern void FindUserLevels(void);
extern void FreeUserLevels(void);
extern char *GetLevelFilename(char *filename, long flag);
extern char *GetAnyLevelFilename(long iLevel, long reversed, char *filename, long flag);
extern char *GetGhostFilename(long level, long reversed, long mirrored, char *ext);
extern void LoadCurrentLevelInfo(char *szName, char *szDir);
extern bool DoesLevelExist(long levelNum);
#endif

#ifdef _N64
void LoadCurrentLevelInfo(long Level);
void LoadUserLevelInfo(void);

#endif

#ifdef _PSX
extern void BuildUserLevelInfo();
#endif



////////////////////////////////////////////////////////////////
//
// Global variables
//

extern LEVEL_SECRETS        LevelSecrets;
extern STAR_LIST            StarList;
extern CURRENT_LEVELINFO    CurrentLevelInfo;
extern LEVELINFO            DefaultLevelInfo[LEVEL_NSHIPPED_LEVELS];

#ifdef _PSX
extern LEVELINFO            UserLevelInfo[10];
#else
extern LEVELINFO            *UserLevelInfo;
#endif


#endif // LEVELINFO_H

