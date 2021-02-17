//-----------------------------------------------------------------------------
// File: AIZone.h
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef AIZONE_H
#define AIZONE_H

#include "car.h"

// macros

typedef struct {
    long    ID, Index;
    VEC     Pos;
    REAL    Size[3];
    PLANE   Plane[3];
    VEC     boundsMin, boundsMax;
//  struct _AINODE_ZONE *ZoneNodes;
    int     Count;
    AINODE* FirstNode;
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
extern long FindAiZone( VEC *Pos );

bool AIZ_IsPointInZone(VEC *pPos, AIZONE *pZone);
bool AIZ_IsCarInZoneID(struct PlayerStruct *pPlayer, int ID);

// globals

extern long AiZoneNum, AiZoneNumID;
extern AIZONE *AiZones;
extern AIZONE_HEADER *AiZoneHeaders;

#endif // AIZONE_H

