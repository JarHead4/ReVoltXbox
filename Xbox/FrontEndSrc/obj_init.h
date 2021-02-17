//-----------------------------------------------------------------------------
// File: obj_init.h
//
// Desc: Initialization (and destruction) functions for objects.
//       This is a companion file to ai_init.cpp that intializes the object's
//       AI variables.
//
// Re-Volt (Generic) Copyright (c) Probe Entertainment 1998
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef OBJ_INIT_H
#define OBJ_INIT_H

#include "object.h"
#include "model.h"

#include "Draw.h"


// Forward declarations
struct LightStruct;
struct PlayerStruct;

// defines and macros

#define SPEEDUP_GEN_HEIGHT Real(22.0f)
#define SPEEDUP_GEN_WIDTH Real(15.0f)
#define CLOUD_NUM 64
#define CLOUD_DIST Real(1024.0f)

#define PLANET_SUN 11
#define DRAGON_FIRE_NUM 50
#define DISSOLVE_PARTICLE_SIZE Real(8.0f)
#define SUN_STAR_NUM 2048
#define SUN_OVERLAY_NUM 4

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
    OBJECT_TYPE_WEEBEL,
    OBJECT_TYPE_PROBELOGO,
    OBJECT_TYPE_CLOUDS,
    OBJECT_TYPE_NAMEWHEEL,
    OBJECT_TYPE_SPRINKLER,
    OBJECT_TYPE_SPRINKLER_HOSE,
    OBJECT_TYPE_OBJECT_THROWER,
    OBJECT_TYPE_BASKETBALL,
    OBJECT_TYPE_TRACKSCREEN,
    OBJECT_TYPE_CLOCK,
    OBJECT_TYPE_CARBOX,
    OBJECT_TYPE_STREAM,
    OBJECT_TYPE_CUP,
    OBJECT_TYPE_3DSOUND,
    OBJECT_TYPE_STAR,
    OBJECT_TYPE_FOX,
    OBJECT_TYPE_TUMBLEWEED,
    OBJECT_TYPE_SMALLSCREEN,
    OBJECT_TYPE_LANTERN,
    OBJECT_TYPE_SKYBOX,
    OBJECT_TYPE_SLIDER,
    OBJECT_TYPE_BOTTLE,
    OBJECT_TYPE_BUCKET,
    OBJECT_TYPE_CONE,
    OBJECT_TYPE_CAN,
    OBJECT_TYPE_LILO,
    OBJECT_TYPE_GLOBAL,
    OBJECT_TYPE_RAIN,
    OBJECT_TYPE_LIGHTNING,
    OBJECT_TYPE_SHIPLIGHT,
    OBJECT_TYPE_PACKET,
    OBJECT_TYPE_ABC,
    OBJECT_TYPE_WATERBOX,
    OBJECT_TYPE_RIPPLE,
    OBJECT_TYPE_FLAG,
    OBJECT_TYPE_DOLPHIN,
    OBJECT_TYPE_GARDEN_FOG,
    OBJECT_TYPE_FOGBOX,

    OBJECT_TYPE_MAX
};

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


#define COPTER_FLYING   (0)
#define COPTER_TURNING  (1)
#define COPTER_WAIT     (2)

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

#ifndef _PSX
typedef struct {
    REAL Time;
} RADAR_OBJ;

#endif

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

typedef struct {
    long Mode, ID;
    REAL Timer;
    VEC Pos,Vel;
} STAR_OBJ;

typedef struct{
    VEC Vel, Rot;
} DISSOLVE_PARTICLE;

typedef struct{
    REAL Age;
    MODEL Model;
    long EnvRGB;
} DISSOLVE_OBJ;

typedef struct {
    VEC Dest;           // Lasers destination point (first world collision poly)
    VEC Delta;          // Vector from laser pos to destination
    REAL Dist;          // Fraction of Delta to first object collision
    REAL Width;         // Laser beam width
    REAL RandWidth;     // Width modifier maximum
    REAL Length;        // Full length of laser
    REAL AlarmTimer;    // Alarm timeout
    long Phase;         // Phase difference
    bool ObjectCollide; // Whether to check against objects
    VISIMASK VisiMask;  // Visibox mask
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

typedef struct {
    long Type;
    REAL Radius;
    VEC Centre;
    VEC Pos[4];
} CLOUD;

typedef struct {
    CLOUD Cloud[CLOUD_NUM];
} CLOUDS_OBJ;

typedef struct {
    REAL    Angle, DestAngle;
    VEC     WheelPos;
    long    StandModel;
    long    WheelModel;
    long    WheelModel2;
#ifdef _N64
    long    WheelModelMid;
#endif
} NAMEWHEEL_OBJ;

typedef struct {
    long ID, NextSfx;//, OnHose;
    long BaseModel, HeadModel;
    VEC HeadPos;
    MAT HeadMat;
    REAL HeadRot, LastRot, Sine, Reach;
    REAL OnHoseTimer;
} SPRINKLER_OBJ;

typedef struct {
    long ID;
    SPRINKLER_OBJ *Sprinkler;
} SPRINKLER_HOSE_OBJ;

typedef struct {
    long ID;
    long ObjectType;
    REAL Speed;
    long ReUse;
} OBJECT_THROWER_OBJ;

enum {
    TRACKSCREEN_STEADY,
    TRACKSCREEN_WOBBLY,

    TRACKSCREEN_NSTATES
};


typedef struct {
    VEC Corner[4];
    unsigned long State;
    long CurrentLevel;
    REAL Timer;
    long TPage;
    #ifndef _PSX
    DRAW_3D_POLY Poly;
    #endif

} OBJECT_TRACKSCREEN_OBJ;

typedef struct {
    REAL SmallHandAngle;
    REAL LargeHandAngle;
    REAL DiscAngle;
    long BodyModel;
    long SmallHandModel;
    long LargeHandModel;
    long DiscModel;
} OBJECT_CLOCK_OBJ;


////////////////////////////////////////////////////////////////
// Car Box
#ifndef _PSX
enum {
    CARBOX_STACKED = 0,
    CARBOX_SELECTED,
    CARBOX_CHOOSABLE,
    CARBOX_OPENING,
    CARBOX_OPEN,

    CARBOX_NSTATES
};

typedef struct {
    long CarType;
    long BoxModel;
    long PlainBoxModel;
    REAL Timer;
    unsigned long State;
    long AtHome;
    VEC HomePos;
    QUATERNION HomeQuat;
#ifdef _PC
    MODEL_POLY *FacePoly;
    //VERTEX_TEX1 FaceVerts[4];
#endif
#ifdef _N64
//  long    BoxFaceModel;
    MODEL   BoxFaceModel;
#endif
} OBJECT_CARBOX_OBJ;

#else // !_PSX

typedef struct {
    long    StandModel;
    long    DisplayModel;
    REAL    Timer;
    REAL    Angle, DestAngle;
    int     LastFace;
    int     CurrentFace;
    int     CurrentItem;
    int     Dir;
    int     LastCarType;
} CARDISP_OBJ;

#endif // !_PSX
////////////////////////////////////////////////////////////////

typedef struct {
    REAL Height, Time, TotalTime;
    REAL Uoff, Voff;
} STREAM_VERTEX;

typedef struct {
    long VertNum;
    REAL Scale;
    VISIMASK VisiMask;
    STREAM_VERTEX Vert[1];
} STREAM_OBJ;

#define RIPPLE_TABLE_DIM 128

typedef struct {
    long Tpage, Width, Height, Dolphin, DolphinCount;
    long OffsetX, OffsetY, Master;
    REAL Timer, Scale;
    PLANE PlaneX, PlaneZ;
    BBOX Box;
    REAL WaterTable1[RIPPLE_TABLE_DIM * RIPPLE_TABLE_DIM];
    REAL WaterTable2[RIPPLE_TABLE_DIM * RIPPLE_TABLE_DIM];
    REAL *WaterTableCurrent, *WaterTableLast;
    VISIMASK VisiMask;
} RIPPLE_OBJ;

typedef struct {
    REAL Timer;
    long JumpFlag;
    VEC JumpVec1, JumpVec2;
    struct PlayerStruct *FromPlayer;
    MODEL Model;
} FOX_OBJ;

typedef struct {
    REAL Time, TimeAdd;
} FOX_VERT;

typedef struct {
    long Type;
} OBJECT_CUP_OBJ;


typedef struct {
    long CurrentLevel;
    REAL Timer;
    long TPage;
#ifndef _PSX
    DRAW_3D_POLY Poly;
#endif
} OBJECT_SMALLSCREEN_OBJ;

typedef struct {
    REAL Brightness;
} LANTERN_OBJ;

typedef struct _SLIDER_OBJ {
    long ID;
    REAL LastTime, Time;
    VEC Origin;
} SLIDER_OBJ;

#define SOUND_3D_MAX_WAIT Real(20)

typedef struct {
    long Sfx;
    long Mode;
    REAL Range;
    REAL Timer;
} SOUND3D_OBJ;

#ifndef _PSX

enum {
    RAINDROP_SLEEP,
    RAINDROP_FALL,
    RAINDROP_SPLASH,
};

#define RAINDROP_NUM 500
#define RAIN_YTOL 50.0f
#define RAIN_XTOL 320.0f
#define RAIN_ZMIN -1000.0f
#define RAIN_ZMAX 3000.0f

typedef struct {
    long Mode;
    REAL Timer, HitHeight;
    VEC Pos;
    VEC Velocity;
    PLANE *Plane;
} RAINDROP;

typedef struct {
#ifdef _PC
    RAINDROP Drop[RAINDROP_NUM];
    VERTEX_TEX0 Vert[RAINDROP_NUM * 2];
#endif
} RAIN_OBJ;

#endif

typedef struct {
    long Mode;
    REAL Timer;
} LIGHTNING_OBJ;

typedef struct {
    REAL Time;
    BOUNDING_BOX Box;
} WATERBOX_OBJ;

typedef struct {
    REAL Time;
    BOUNDING_BOX Box;
    long RealFogRGB;
} FOGBOX_OBJ;

#ifdef _PC

typedef struct {
    VEC     pos, vel, imp;
    REAL    length;
    REAL    invMass;
} FLAG_PARTICLE;

typedef struct {

    VISIMASK        VisiMask;

    int             tpage;
    int             w,h;
    int             cVert;
    int             cTris;
    REAL            length, lengthD;
    FLAG_PARTICLE*  pParticle;

    REAL            cxRipple;
    REAL            cyRipple;
    REAL            czRipple;
    REAL            amplitude;

#ifndef _N64
    VERTEX_TEX1*    pVert;
#else
    void*           pVert;
#endif
    unsigned short* pIndex;

} FLAG_DATA_OBJ;

#endif

typedef struct {
    VISIMASK VisiMask;
    REAL Time;
} DOLPHIN_OBJ;

// external function prototypes

extern long TotalStarNum, StarModelNum;


extern void LoadObjects(char *file);
extern long CountFileStars(long levnum);
extern LARGE_INTEGER GetGlobalID(DWORD id);
extern OBJECT *CreateObject(VEC *pos, MAT *mat, long ID, long *flags);
extern void InitStars(void);



// Macro used to register object init data.
// This macro creates a static constructor that gets executed when the app
// starts up. This guarantees the object init data is registered before any
// objects are actually created.
#define REGISTER_OBJECT(Type,InitFn,Size)         \
	struct REGISTER_##Type                        \
	{	                                          \
		REGISTER_##Type()                         \
		{                                         \
			ObjInitData[Type].InitFunc  = InitFn; \
			ObjInitData[Type].AllocSize = Size;   \
		}                                         \
	};                                            \
	static REGISTER_##Type _##Type;               \





// external global variables
extern OBJECT_INIT_DATA ObjInitData[];




#endif // OBJ_INIT_H

