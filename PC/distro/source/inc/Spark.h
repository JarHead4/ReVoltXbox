#ifndef __SPARK_H__
#define __SPARK_H__

#include "Typedefs.h"
#ifdef _PC
#include "Draw.h"
#endif
#ifdef _N64
#include "faceme.h"
#endif

struct object_def;

// The different kinds of sparks
typedef enum SparkTypeEnum {
	SPARK_NONE = -1,
	SPARK_SPARK = 0,
	SPARK_SPARK2,
	SPARK_SNOW,
	SPARK_POPCORN,
	SPARK_GRAVEL,
	SPARK_SAND,
	SPARK_MUD,
	SPARK_ELECTRIC,
	SPARK_WATER,
	SPARK_DIRT,
	SPARK_SMOKE1,
	SPARK_SMOKE2,
	SPARK_SMOKE3,
	SPARK_BLUE,
	SPARK_BIGBLUE,
	SPARK_SMALLORANGE,
	SPARK_SMALLRED,
	SPARK_EXPLOSION1,
	SPARK_EXPLOSION2,
	SPARK_STAR,

	SPARK_NTYPES
} SPARK_TYPE;

/////////////////////////////////////////////////////////////////////
//
// Spark trail info
//
/////////////////////////////////////////////////////////////////////

enum {
	TRAIL_NONE = -1,
	TRAIL_FIREWORK = 0,
	TRAIL_SPARK,
	TRAIL_SMOKE,

	TRAIL_NTYPES
};

#define MAX_TRAILS			(128)
#define TRAIL_MAX_BITS		(12)

#define TRAIL_FADES			(0x1)
#define TRAIL_SHRINKS		(0x2)
#define TRAIL_EXPANDS		(0x4)

typedef struct TrailDataStruct {
	REAL U, V, Usize, Vsize;
	REAL EndU, EndV;
	long A, R, G, B;
	long Type;
	REAL LifeTime;
	REAL Width;
	long MaxTrails;
} TRAIL_DATA;


typedef struct SparkTrailStruct {
	bool Free;
	VEC Pos[TRAIL_MAX_BITS];
	TRAIL_DATA *Data;
	long FirstTrail;
	long MaxTrails;
	long NTrails;
} TRAIL;


/////////////////////////////////////////////////////////////////////
//
// The static info for each different spark type
//
/////////////////////////////////////////////////////////////////////

typedef struct SparkDataStruct {
#ifdef _PC
	FACING_POLY FacingPoly;
#endif
#ifdef _N64
	REAL		XSize, YSize;
	GAME_GFX	GfxIdx;
	long		u, v;
	long		w, h;
	long		Colour;
	FACEME_FLAG	Flag;
#endif
	long SparkType;				// Bit flags for spark subtypes
	REAL Mass;					// Mass
	REAL Resistance;			// Air resistance
	REAL Friction;				// Sliding friction
	REAL Restitution;			// Bounciness
	unsigned long LifeTime;		// Max lifetime
	unsigned long LifeTimeVar;	// Randomly subratcted from lifetime
	REAL SpinRateBase;			// Average spin rate (radians per second)
	REAL SpinRateVar;			// Variation about spin rate
	REAL SizeVar;				// Variation about size
	REAL GrowRateBase;				// Average grow rate
	REAL GrowRateVar;			// Variation about grow rate
	long TrailType;				// The trail number
} SPARK_DATA;

#define SPARK_WORLD_COLLIDE		(0x1)
#define SPARK_OBJECT_COLLIDE	(0x2)
#define SPARK_CREATE_TRAIL		(0x4)
#define SPARK_FIELD_AFFECT		(0x8)
#define SPARK_SPINS				(0x10)
#define SPARK_GROWS				(0x20)
#define SPARK_CAMERA_COLLIDE	(0x40)
#define SPARK_SEMI				(0x80)


/////////////////////////////////////////////////////////////////////
//
// The spark variables
//
/////////////////////////////////////////////////////////////////////

typedef struct SparkStruct {
	// Status
	bool Free;				// Whether spark is available
	VISIMASK VisiMask;

	VEC	Pos;				// Current position
	VEC	Vel;				// Current velocity

	REAL Spin, SpinRate;	// Current angle and rate of spin
	REAL Grow, GrowRate;	// Rate of expansion/ contraction

	TRAIL *Trail;			// the spark trail number if it exists
	REAL TrailTime;
	
	SPARK_DATA *Data;		// Information on type of spark
	
	unsigned long Age;		// Current age
} SPARK;

// Spark generator
typedef struct SparkGenStruct {
	SPARK_TYPE	Type;		// Type of spark
	VISIMASK	VisiMask;	// Visibox mask

	struct object_def	*Parent;		// Parent object

	VEC		SparkVel;			// Average (parent-relative) velocity
	REAL	SparkVelVar;		// Variation about velocity

	REAL	Time, MaxTime;		// Time and max time between sparks
} SPARK_GEN;


#ifdef _PC
#define MAX_SPARKS	1024					// max number of sparks in game
#endif
#ifdef _N64
#define MAX_SPARKS	512						// max number of sparks in game
#endif
#ifdef _PSX
#define MAX_SPARKS	0
#endif

#define MIN_SPARK_VEL		Real( 100.0f )		// No sparks below this speed
#define MAX_SPARK_VEL		Real( 1600.0f )	// Always sparks above this speed

#define MIN_DUST_IMPULSE	Real( 3.0f )
#define DUST_PROBABILITY	Real( 0.2f )
#define DUST_SCALE			Real( 50.0f )
#define DUST_RND_SCALE		Real( 30.0f )



/////////////////////////////////////////////////////////////////////
//
// External function prototypes
//
extern void InitSparks();
extern void ProcessSparks();
extern bool CreateSpark(SPARK_TYPE type, VEC *pos, VEC *vel, REAL velVar, VISIMASK mask);
extern void DrawSparks();
extern REAL SparkProbability(REAL vel);

extern TRAIL *GetFreeTrail(long trailType);
extern void FreeTrail(TRAIL *trail);
extern void UpdateTrail(TRAIL *trail, VEC *newPos);
extern void ModifyFirstTrail(TRAIL *trail, VEC *newPos);
extern void DrawTrails();


/////////////////////////////////////////////////////////////////////
//
// Externed Gloabl variables
//

extern SPARK Sparks[MAX_SPARKS];
extern int NActiveSparks;

extern TRAIL SparkTrail[MAX_TRAILS];
extern int NActiveTrails;

#endif
