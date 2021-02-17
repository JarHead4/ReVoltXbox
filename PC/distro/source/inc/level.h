/*********************************************************************************************
 *
 * level.h
 *
 * Re-Volt (Generic) Copyright (c) Probe Entertainment 1998
 * 
 * Contents:
 * 			Level specific code and data
 *
 *********************************************************************************************
 *
 * 28/02/98 Matt Taylor
 *	File inception.
 *
 *********************************************************************************************/

#ifndef _LEVEL_H_
#define _LEVEL_H_

#include "car.h"

//
// Defines and macros
//

#define MAX_LEVELS 256
#define MAX_LEVEL_DIR_NAME 16
#define MAX_LEVEL_NAME 64
#define MAX_ENV_NAME 32

#define FILENAME_MAKE_BODY 1
#define FILENAME_GAME_SETTINGS 2

enum {
	GAMETYPE_TRIAL,
	GAMETYPE_SINGLE,
	GAMETYPE_SERVER,
	GAMETYPE_CLIENT,
	GAMETYPE_SESSIONLOST,
};

typedef struct {
	long Time;
	char Player[MAX_PLAYER_NAME];
	char Car[CAR_NAMELEN];
} ONE_RECORD_ENTRY;

typedef struct {
	long SplitTime[MAX_SPLIT_TIMES];
	ONE_RECORD_ENTRY RecordLap[MAX_RECORD_TIMES];
	ONE_RECORD_ENTRY RecordRace[MAX_RECORD_TIMES];
} RECORD_ENTRY;

// Level information

typedef struct {
	char Dir[MAX_LEVEL_DIR_NAME];
	char Name[MAX_LEVEL_NAME];
	char EnvStill[MAX_ENV_NAME];
	char EnvRoll[MAX_ENV_NAME];
	VEC NormalStartPos, ReverseStartPos;
	float NormalStartRot, ReverseStartRot;
	long  NormalStartGrid, ReverseStartGrid;
	float FarClip;
	float FogStart;
	long FogColor;
	float VertFogStart;
	float VertFogEnd;
	long WorldRGBper;
	long ModelRGBper;
	long InstanceRGBper;
	long MirrorType;
	float MirrorMix;
	float MirrorIntensity;
	float MirrorDist;
} LEVELINFO;

// Global variables

extern LEVELINFO	*LevelInf;
extern GAME_SETTINGS GameSettings;
extern VEC			LEV_StartPos;
extern REAL			LEV_StartRot; 
extern long			LEV_StartGrid;


// Global function externs

extern void LEV_InitLevel(void);
extern void LEV_EndLevel(void);
extern void FindLevels(void);
extern void FreeLevels(void);
extern long GetLevelNum(char *dir);
extern char *GetLevelFilename(char *filename, long flag);
extern bool LoadLevelFields(LEVELINFO *LevelInfo);
extern VEC *LEV_LevelFieldPos;
extern MAT *LEV_LevelFieldMat;

#endif /* _LEVEL_H_ */

