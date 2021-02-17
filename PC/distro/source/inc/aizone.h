
#ifndef AIZONE_H
#define AIZONE_H

#include "car.h"

// macros

typedef struct {
	long ID;
	VEC Pos;
	float Size[3];
	PLANE Plane[3];
	struct _AINODE *ZoneNodes;
} AIZONE;

typedef struct {
	long Count;
	AIZONE *Zones;
} AIZONE_HEADER;

// prototypes

#ifdef _PC
extern void LoadAiZones(char *file);
#endif
#ifdef _N64
extern void LoadAiZones();
#endif
extern void FreeAiZones(void);
extern char UpdateCarAiZone(struct PlayerStruct *Player);

// globals

extern long AiZoneNum, AiZoneNumID;
extern AIZONE *AiZones;
extern AIZONE_HEADER *AiZoneHeaders;

#endif
