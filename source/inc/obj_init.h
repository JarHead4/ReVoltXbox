/*********************************************************************************************
 *
 * obj_init.h
 *
 * Re-Volt (Generic) Copyright (c) Probe Entertainment 1998
 * 
 * Contents:
 *			Initialisation (and destruction) functions for objects
 *			This is a companion file to ai_init.cpp that intialises the object's AI variables
 *
 *********************************************************************************************
 *
 * 01/07/98 Matt Taylor
 *	File inception.
 *
 *********************************************************************************************/

#ifndef _OBJ_INIT_H_
#define _OBJ_INIT_H_

#include "object.h"
#include "editobj.h"

// defines and macros

#define SPEEDUP_GEN_HEIGHT 22.0f
#define SPEEDUP_GEN_WIDTH 15.0f


// structures

typedef struct {
	long (*InitFunc)(OBJECT *obj, long *flags);
	long AllocSize;
} OBJECT_INIT_DATA;

typedef struct {
	long FadeUp, FadeDown;
	REAL Range;
	MODEL_RGB rgb;
	VEC GlowOffset;
} STROBE_TABLE;

//
// object sub-structures
//

#ifndef _PSX
typedef struct {
	REAL SpinSpeed;
} BARREL_OBJ;

typedef struct {
	long OwnPlanet, OrbitPlanet;
	REAL OrbitSpeed, SpinSpeed;
	VEC OrbitOffset;
	MAT OrbitMatrix;
	VISIMASK VisiMask;
} PLANET_OBJ;

typedef struct {
	long r, g, b, rgb;
	REAL Rot, RotVel;
} SUN_OVERLAY;

typedef struct {
	VEC Pos;
	long rgb;
} SUN_STAR;

typedef struct {
	PLANET_OBJ Planet;
	SUN_OVERLAY Overlay[SUN_OVERLAY_NUM];
	SUN_STAR Star[SUN_STAR_NUM];
#ifdef _PC
	VERTEX_TEX0 Verts[SUN_STAR_NUM];
#endif
#ifdef _N64
	Vtx Verts[SUN_STAR_NUM];
#endif
	VISIMASK VisiMask;
} SUN_OBJ;

typedef struct {
	long PropModel;
	REAL Rot, Speed;
	VEC GenPos, Offset, PropPos;
	MAT BankMatrix, PropMatrix;
} PLANE_OBJ;


#define COPTER_FLYING	(0)
#define COPTER_TURNING	(1)
#define COPTER_WAIT		(2)

typedef struct {
	long BladeModel1, BladeModel2;
	VEC BladePos1, BladePos2;
	MAT BladeMatrix1, BladeMatrix2;

	long State;
	QUATERNION OldInitialQuat;
	QUATERNION InitialQuat;
	QUATERNION CurrentUpQuat;
	REAL TurnTime;
	BBOX FlyBox;
	VEC Destination;
	VEC Direction;
	REAL MaxVel;
	REAL Acc;

} COPTER_OBJ;

typedef struct {
	long rgb;
	REAL Time, MinSize, Size, Spin, SpinSpeed;
	VEC Pos;
	MAT Matrix;
} DRAGON_FIRE;

typedef struct {
	long BodyModel, HeadModel, FireGenTime;
	REAL Count;
	VEC FireGenPoint, FireGenDir;
	DRAGON_FIRE Fire[DRAGON_FIRE_NUM];
} DRAGON_OBJ;

typedef struct {
	REAL Height, Time, TotalTime;
} WATER_VERTEX;

typedef struct {
	long VertNum;
	REAL Scale;
	WATER_VERTEX Vert[1];
} WATER_OBJ;

typedef struct {
	REAL Height;
	REAL TimeX, TotalTimeX;
	REAL TimeHeight, TotalTimeHeight;
	REAL TimeZ, TotalTimeZ;
	REAL SteamTime;
	MAT Ori;
} BOAT_OBJ;

typedef struct {
	REAL Time;
	LIGHT *Light1, *Light2;
} RADAR_OBJ;

typedef struct {
	REAL Time, Height;
} BALLOON_OBJ;

typedef struct {
	REAL CreakFlag, Time;
	MAT Mat;
} HORSE_OBJ;

typedef struct {
	VEC WheelPos[4];
	long FrontWheel, BackWheel, WhistleFlag;
	REAL TimeFront, TimeBack, SteamTime;
} TRAIN_OBJ;

typedef struct {
	long StrobeCount, StrobeNum;
	long FadeUp, FadeDown;
	long r, g, b;
	REAL Range, Glow;
	VEC LightPos;
} STROBE_OBJ;

typedef struct {
} SPACEMAN_OBJ;

typedef struct {
	long Mode, Clone;
	REAL Timer;
	VEC Pos, Vel;
} PICKUP_OBJ;

typedef struct{
	VEC Vel, Rot;
} DISSOLVE_PARTICLE;

typedef struct{
	REAL Age;
	MODEL Model;
	long EnvRGB;
} DISSOLVE_OBJ;

typedef struct {
	VEC Dest;			// Lasers destination point (first world collision poly)
	VEC Delta;			// Vector from laser pos to destination
	REAL Dist;			// Fraction of Delta to first object collision
	REAL Width;			// Laser beam width
	REAL RandWidth;		// Width modifier maximum
	REAL Length;		// Full length of laser
	long Phase;			// Phase difference
	bool ObjectCollide;	// Whether to check against objects
	VISIMASK VisiMask;	// Visibox mask
} LASER_OBJ;

typedef struct {
	VEC Pos[4];
	VEC Vel[4];
	REAL Frame, FrameAdd;
} SPLASH_POLY;

typedef struct {
	long Count;
	SPLASH_POLY Poly[SPLASH_POLY_NUM];
} SPLASH_OBJ;

struct CollPolyStruct;
typedef struct {
	REAL Width, Height;
	REAL LoSpeed, HiSpeed, Speed;
	REAL ChangeTime, Time;
	struct CollPolyStruct CollPoly;
	VEC PostPos[2];
	REAL HeightMod[2];
} SPEEDUP_OBJ;


#endif // _PSX

// external function prototypes

#ifdef _PC
extern void LoadObjects(char *file);
#endif
#ifdef _N64
extern void LoadObjects();
extern long LoadOneLevelModel(long id, long flag, struct renderflags renderflag, long tpage);
#endif
extern OBJECT *CreateObject(VEC *pos, MAT *mat, long ID, long *flags);

// external global variables

#endif