/*********************************************************************************************
 *
 * object.h
 *
 * Re-Volt (Generic) Copyright (c) Probe Entertainment 1998
 * 
 * Contents:
 * 			Object processing code
 *
 *********************************************************************************************
 *
 * 03/03/98 Matt Taylor
 *	File inception.
 *
 *********************************************************************************************/

#ifndef _OBJECT_H_
#define _OBJECT_H_

#include "grid.h"
#ifndef _PSX
#include "light.h"
#endif
#ifdef 	_PC
#include "sfx.h"
#endif
#include "control.h"
#ifdef _N64
#include "spark.h"
#endif

//
// Defines and macros
//

#ifndef _PSX
#define	MAX_OBJECTS	128
#else
#define MAX_OBJECTS 8
#endif

#define PLANET_SUN 11
#define DRAGON_FIRE_NUM 50
#define DISSOLVE_PARTICLE_SIZE 8.0f
#ifdef _PC
#define SUN_STAR_NUM 2048
#define SUN_OVERLAY_NUM 4
#endif
#ifdef _N64
#define SUN_STAR_NUM 512
#define SUN_OVERLAY_NUM 3
#endif
#ifdef _PSX
#define SUN_STAR_NUM 512
#define SUN_OVERLAY_NUM 3
#endif
#define PICKUP_GEN_TIME 10.0f

#define SPLASH_POLY_NUM 150

// object enum list

enum {
	OBJECT_TYPE_CAR = -1,

	OBJECT_TYPE_BARREL,
	OBJECT_TYPE_BEACHBALL,
	OBJECT_TYPE_PLANET,
	OBJECT_TYPE_PLANE,
	OBJECT_TYPE_COPTER,
	OBJECT_TYPE_DRAGON,
	OBJECT_TYPE_WATER,
	OBJECT_TYPE_TROLLEY,
	OBJECT_TYPE_BOAT,
	OBJECT_TYPE_SPEEDUP,
	OBJECT_TYPE_RADAR,
	OBJECT_TYPE_BALLOON,
	OBJECT_TYPE_HORSE,
	OBJECT_TYPE_TRAIN,
	OBJECT_TYPE_STROBE,
	OBJECT_TYPE_FOOTBALL,
	OBJECT_TYPE_SPARKGEN,
	OBJECT_TYPE_SPACEMAN,

	OBJECT_TYPE_SHOCKWAVE,
	OBJECT_TYPE_FIREWORK,
	OBJECT_TYPE_PUTTYBOMB,
	OBJECT_TYPE_WATERBOMB,
	OBJECT_TYPE_ELECTROPULSE,
	OBJECT_TYPE_OILSLICK,
	OBJECT_TYPE_OILSLICK_DROPPER,
	OBJECT_TYPE_CHROMEBALL,
	OBJECT_TYPE_CLONE,
	OBJECT_TYPE_TURBO,
	OBJECT_TYPE_ELECTROZAPPED,
	OBJECT_TYPE_SPRING,

	OBJECT_TYPE_PICKUP,
	OBJECT_TYPE_DISSOLVEMODEL,
	OBJECT_TYPE_FLAP,
	OBJECT_TYPE_LASER,
	OBJECT_TYPE_SPLASH,
	OBJECT_TYPE_BOMBGLOW,

	OBJECT_TYPE_MAX
};

enum {
	PICKUP_NONE = -1,
	PICKUP_SHOCKWAVE = 0,
	PICKUP_FIREWORK,
	PICKUP_FIREWORKPACK,
	PICKUP_PUTTYBOMB,
	PICKUP_WATERBOMB,
	PICKUP_ELECTROPULSE,
	PICKUP_OILSLICK,
	PICKUP_CHROMEBALL,
	PICKUP_TURBO,

	PICKUP_NUM
};

//
// Typedefs and structures
//

// Handler types

typedef	void (*CON_HANDLER)(CTRL *Control, void *Object);		// Typedef for control handling functions
typedef	void (*CTRL_HANDLER)(CTRL *Control);					// Typedef for hardware reading functions
typedef	void (*MOVE_HANDLER)(void *Object);						// Typedef for object movement functions
typedef void (*COLL_HANDLER)(void *Object);
typedef void (*AI_HANDLER)(void *Object);
typedef void (*RENDER_HANDLER)(void *Object);
typedef void (*FREE_HANDLER)(void *Object);

// objflags - main object flag structure for primary functions

struct objflags
{
	unsigned long
	Draw      : 1,			// Object render off / on
	Move      : 1,			// Object movement off / on
	IsInGrid  : 1;			// Is object is in grid list
};


// renderflags - flags for controlling aspects of rendering

struct renderflags
{
	unsigned long
	envmap : 1,
	envonly : 1,
	light : 1,
	litsimple : 1,
	reflect : 1,
	fog : 1,
	visible : 1,
	glare : 1,
	meshfx : 1;
};

//
// Main OBJECT definition
//
struct PlayerStruct;
struct ForceFieldStruct;
struct SparkGenStruct;


typedef struct object_def
{
	NEWBODY				body;

	long	 			Type;						// Object's type
	long	 			priority;					// Object's priority
	long				ObjID;

	struct objflags		flag;
	struct renderflags	renderflag;

	struct PlayerStruct	*player;
	struct object_def	*objref;					// Used for objects that follow or flee other objects
	struct object_def	*creator;					// Object's creator

	struct object_def	*carcoll[MAX_NUM_PLAYERS];	// Object pointer array for any car-to-car collisions
	struct object_def	*objcoll;					// Object pointer for a non-car object collision

	long				EnvRGB;						// envmap RGB
	long				DefaultModel;				// default model index
	long				CollType;					// Type of collision object has (for detection)

	struct ForceFieldStruct		*Field;						// Force field attached to object
	long				FieldPriority;

#ifndef _PSX
	LIGHT				*Light;						// light source
#endif
#ifndef _PSX
#ifndef _CARCONV
	struct SparkGenStruct		*SparkGen;			// Spark generator

#ifdef _PC
	SAMPLE_3D			*Sfx3D;						// 3D sfx ptr
#endif
#endif
#endif
	void				*Data;						// ptr to alloc'd memory

	MOVE_HANDLER		movehandler;				// Function that handles movement of object
	COLL_HANDLER		collhandler;				// Function to handle collision response for this object
	AI_HANDLER			aihandler;					// Function to handle ai for this object
	RENDER_HANDLER		renderhandler;				// Function to handle rendering for this object
	FREE_HANDLER		freehandler;				// Function to deallocate any allocated ram

	struct object_def	*prev;						// Linked list pointers
	struct object_def	*next;

	unsigned short		GridX[MAX_GRIDS];
	unsigned short		GridZ[MAX_GRIDS];

	struct object_def	*GridPrev[MAX_GRIDS];
	struct object_def	*GridNext[MAX_GRIDS];

} OBJECT;

////////////////////////////////////////////////////////////////////
// Object-pair collision info structure
/////////////////////////////////////////////////////////////////////
typedef struct PairCollInfoStruct {
	unsigned int Tested : 1;
	unsigned int Collided : 1;
} PAIRCOLLINFO;

#define ClearPairInfo(obj1, obj2) \
{ \
	OBJ_PairCollInfo[(obj1)->ObjID][(obj2)->ObjID].Tested = 0; \
	OBJ_PairCollInfo[(obj1)->ObjID][(obj2)->ObjID].Collided = 0; \
	OBJ_PairCollInfo[(obj2)->ObjID][(obj1)->ObjID].Tested = 0; \
	OBJ_PairCollInfo[(obj2)->ObjID][(obj1)->ObjID].Collided = 0; \
}

#define SetPairTested(obj1, obj2) \
{ \
	OBJ_PairCollInfo[(obj1)->ObjID][(obj2)->ObjID].Tested = 1; \
	OBJ_PairCollInfo[(obj2)->ObjID][(obj1)->ObjID].Tested = 1; \
}

#define SetPairCollided(obj1, obj2) \
{ \
	OBJ_PairCollInfo[(obj1)->ObjID][(obj2)->ObjID].Collided = 1; \
	OBJ_PairCollInfo[(obj2)->ObjID][(obj1)->ObjID].Collided = 1; \
}

#define IsPairCollided(obj1, obj2) (OBJ_PairCollInfo[(obj1)->ObjID][(obj2)->ObjID].Collided == 1)
#define IsPairTested(obj1, obj2) (OBJ_PairCollInfo[(obj1)->ObjID][(obj2)->ObjID].Tested == 1)


///
// External global variables
//

extern OBJECT *OBJ_ObjectList;
extern OBJECT *OBJ_ObjectHead;
extern OBJECT *OBJ_ObjectTail;
extern long	OBJ_NumObjects;
extern PAIRCOLLINFO	OBJ_PairCollInfo[MAX_OBJECTS][MAX_OBJECTS];


//
// External function prototypes
//

extern long		OBJ_InitObjSys(void);
extern void		OBJ_KillObjSys(void);
extern OBJECT  *OBJ_AllocObject(void);
extern OBJECT  *OBJ_ReplaceObject(void);
extern long		OBJ_FreeObject(OBJECT *Obj);

extern void ClearActivePairInfo();
extern void ClearAllPairInfo();
extern void ClearThisObjPairInfo(OBJECT *obj2);


#endif /* _OBJECT_H_ */
