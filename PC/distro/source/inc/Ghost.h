
#ifndef __GHOST_H__
#define __GHOST_H__

#include "light.h"
#include "level.h"

#define GHOST_FILENAME			"Ghost.dat"
#define GHOST_FILENAME_MIRRORED	"Ghost.tad"

#define GHOST_DATA_MAX			(10000)
#define GHOST_MAX_SPLIT_TIMES	(MAX_SPLIT_TIMES)
#define GHOST_LAP_TIME			(GHOST_MAX_SPLIT_TIMES)

#define GHOST_MIN_TIMESTEP		MAKE_TIME(0, 0, 75)

#define GHOST_VECTOR_SCALE		(100.0f)
#define GHOST_VECTOR_INVSCALE	(1.0f / GHOST_VECTOR_SCALE)

#define GHOST_WHEEL_SCALE		(5.0f)
#define GHOST_WHEEL_INVSCALE	(1.0f / GHOST_WHEEL_SCALE)

#define GHOST_ANGLE_SCALE		(5.0f)
#define GHOST_ANGLE_INVSCALE	(1.0f / GHOST_WHEEL_SCALE)


/////////////////////////////////////////////////////////////////////
//
// Ghost car header info
//

typedef struct GhostInfoStruct {

	long	CarID;
	char	PlayerName[MAX_PLAYER_NAME];
	long	Time[GHOST_MAX_SPLIT_TIMES + 1];
	long	NFrames;

} GHOST_INFO;


/////////////////////////////////////////////////////////////////////
//
// Ghost car data per frame
//

typedef struct GhostDataStruct {

	long		Time;

	VEC		Pos;
	CHARQUAT	Quat;
	char		WheelAngle[CAR_NWHEELS];
	char		WheelPos[CAR_NWHEELS];

} GHOST_DATA;


extern void InitGhostData(PLAYER *player);
extern void EndGhostData(PLAYER *player);
extern void InitBestGhostData();
extern void ClearBestGhostData();
extern void SwitchGhostDataStores();
extern bool StoreGhostData(CAR *car);
extern bool LoadGhostData(LEVELINFO *levelInfo);
extern bool SaveGhostData(LEVELINFO *levelInfo);

extern void InterpGhostData(CAR *ghostCar);
extern void InitGhostLight(void);


extern GHOST_INFO *GHO_BestGhostInfo;
extern long GHO_BestFrame;
extern long GhostSolid;
extern PLAYER *GHO_GhostPlayer;
extern bool GHO_GhostExists;
extern LIGHT *GhostLight;

#endif

