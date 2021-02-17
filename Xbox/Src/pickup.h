//-----------------------------------------------------------------------------
// File: pickup.h
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef PICKUP_H
#define PICKUP_H

#include "NewColl.h"
#include "Object.h"

struct LightStruct;
struct PlayerStruct;

#define MAX_PICKUPS     40                  // Max number of pickup generating positions
#define MAX_PICKUPS_ALLOWED 2               // Max number of pickups per player
#define INITIAL_PICKUPS 5                       // initial number of pickups
#define PICKUP_GEN_TIME Real(5.0f)          // Time for pickup to generate
#define PICKUP_CYCLE_TIME TO_TIME(Real(4.0f))
#define PICKUP_CYCLE_TIME_CUBED TO_TIME(Real(4.0f * 4.0f * 4.0f))



typedef enum PickupStateEnum {
    PICKUP_STATE_GENERATING,
    PICKUP_STATE_ACTIVE,
    PICKUP_STATE_DYING,
    PICKUP_STATE_INACTIVE,
    PICKUP_STATE_FREE,

    PICKUP_NSTATES
} PICKUP_STATE;

enum {
    PICKUP_TYPE_NONE = -1,
    PICKUP_TYPE_SHOCKWAVE = 0,
    PICKUP_TYPE_FIREWORK,
    PICKUP_TYPE_FIREWORKPACK,
    PICKUP_TYPE_PUTTYBOMB,
    PICKUP_TYPE_WATERBOMB,
    PICKUP_TYPE_ELECTROPULSE,
    PICKUP_TYPE_OILSLICK,
    PICKUP_TYPE_CHROMEBALL,
    PICKUP_TYPE_TURBO,
    PICKUP_TYPE_CLONE,
    PICKUP_TYPE_GLOBAL,

    PICKUP_NTYPES,
    PICKUP_TYPE_DUMMY,
};


////////////////////////////////////////////////////////////////
//
// Pickip data structure
//
////////////////////////////////////////////////////////////////

typedef struct PickupStruct {

    unsigned long ID;
    long Mode, Clone;
    long EnvRGB;
    long DefaultModel;
    REAL Timer;
    VEC Pos;
#ifndef _PSX
    MAT WMatrix;
#endif
    VEC GenPos, Vel;
    BBOX BBox;
    struct LightStruct *Light;
#ifndef _PSX
    struct renderflags RenderFlags;
    REAL EnvOffsetX;
    REAL EnvOffsetY;
    REAL EnvScale;
#endif

} PICKUP;



extern PICKUP *AllocOnePickup();
extern void FreeOnePickup(PICKUP *pickup);
extern void FreeAllPickups();

extern void InitPickups(void);
extern void InitOnePickup(PICKUP *pickup);
extern void InitPickupArray();
extern void UpdateAllPickups();
extern void AllowOnePickup(long alive);
extern void GivePickupToPlayer(struct PlayerStruct *player, unsigned long type);

extern void InitPickupWeightTables(void);
extern void InitPickupRaceWeightTables(void);

#ifdef _PSX
extern void DrawAllPickups(long * OT, MATRIX * Cam, VECTOR * CamPos );
extern void RenderPickup( PICKUP *pickup, long * OT, MATRIX * Cam, VECTOR * CamPos );
#endif


extern PICKUP PickupArray[MAX_PICKUPS];
extern long NPickups;
extern REAL GlobalPickupFlash;

extern short PickupWeight[][PICKUP_NTYPES];



#endif // PICKUP_H

