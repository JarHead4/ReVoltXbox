//-----------------------------------------------------------------------------
// File: weapon.h
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef WEAPON_H
#define WEAPON_H

#ifdef _PC
//$MODIFIED
//#include "sfx.h"
#include "SoundEffectEngine.h"
#include "SoundEffects.h"
//$END_MODIFICATIONS
#include "draw.h"
#endif
#include "model.h"
#include "newcoll.h"
#include "particle.h"
#include "body.h"
#include "object.h"
//#include "spark.h"

#define PSX_COMPILE_WEAPONS

#define PSX_COMPILE_SPARKS
//#define PSX_SPARKS_FOR_ALL_WEAPONS



struct SparkTrailStruct;


enum    {
    HALOTYPE_CIRCLE,
    HALOTYPE_ELLIPSE,
    HALOTYPE_4POINTSTAR,
    HALOTYPE_RING,
    HALOTYPE_CIRCLEHORIZ,
    HALOTYPE_SPINNER,
    HALOTYPE_SMOKE,
    HALOTYPE_MALTESECROSS,
    HALOTYPE_CIRCLE_STEF_SUN,
    HALOTYPE_PICKUP_INNER,
    HALOTYPE_PICKUP_OUTER,
    HALOTYPE_WINDMILL,
    HALOTYPE_REPOS_CIRCLE,
    HALOTYPE_REPOS_RING1,
    HALOTYPE_REPOS_RING2,
    HALOTYPE_REPOS_REDRING,

    HALOTYPE_HALO_COUNT,
};


// macros
enum    {           // Define types of environment mapping
    ENVTYPE_NONE,
    ENVTYPE_STANDARD,
    ENVTYPE_ELECTROPULSE,
    ENVTYPE_TURBO,
};


#define TARGET_BLINKMUL Real(10)
#define TARGET_ANIM_TIME TO_TIME(Real(0.5))
#define TARGET_SIZE      Real(30)
#define TARGET_RANGE_MIN TO_LENGTH(Real(100.0))
#define TARGET_RANGE_MAX TO_LENGTH(Real(6000.0))

#define TURBO_NTRAILS 4

#define SHOCKWAVE_VEL           TO_VEL(Real(6000.0f))
#define SHOCKWAVE_MIN_VEL       TO_VEL(Real(1500.0f))
#define SHOCKWAVE_MAX_AGE       TO_TIME(Real(2.0f))
#define SHOCKWAVE_RAD           TO_LENGTH(Real(40.0f))
#define SHOCKWAVE_PULL_MAX_MUL  (Real(0.1f))
#define SHOCKWAVE_PULL_MIN_MUL  (Real(0.5f))

#define OILSLICK_LIST_MAX 64

#define OILY_WHEEL_TIME     TO_TIME(Real(2.5))

#define OILSLICK_GRAV Real(2048.0f)
#define OILSLICK_MIN_SIZE Real(32.0f)
#define OILSLICK_MAX_SIZE Real(128.0f)

#define FIREWORK_MAX_AGE                TO_TIME(Real(1.2))
#define FIREWORK_RADIUS                 TO_LENGTH(Real(100.0f))
#define FIREWORK_RADIUS_SQ              (FIREWORK_RADIUS * FIREWORK_RADIUS)
#define FIREWORK_EXPLODE_RADIUS         TO_LENGTH(Real(500.0f))
#define FIREWORK_EXPLODE_RADIUS_SQ      (FIREWORK_EXPLODE_RADIUS * FIREWORK_EXPLODE_RADIUS)
#define FIREWORK_EXPLODE_MIN_RADIUS     TO_LENGTH(Real(75.0f))
#define FIREWORK_EXPLODE_MIN_RADIUS_SQ  (FIREWORK_EXPLODE_MIN_RADIUS * FIREWORK_EXPLODE_MIN_RADIUS)
#define FIREWORK_EXPLODE_OFFSET         TO_LENGTH(Real(50.0f))
#define FIREWORK_EXPLODE_IMPULSE        Real(100000.0f)
#define FIREWORK_EXPLODE_IMPULSE2       Real(100000.0f)
#define FIREWORK_RANGE_OFFSET           Real(200.0)
#define FIREWORK_DIR_OFFSET             Real(0.25)

#define DROPPER_GAP         Real(64.0f)

#define ELECTRO_RANGE       384
#define ELECTRO_KILL_TIME   TO_TIME(Real(3.0f))
#define ELECTROPULSE_NO_RETURN_TIME TO_TIME(Real(3.0))

#define CHROMEBALL_MIN_RAD  TO_LENGTH(Real(4.0f))
#define CHROMEBALL_MAX_RAD  TO_LENGTH(Real(64.0f))

#define WATERBOMB_RADIUS    TO_LENGTH( Real(45.0f))
#define WATERBOMB_MAX_AGE   TO_TIME( Real(30.0f))
#define WATERBOMB_BANG_VAR  Real(2000.0f)
#define WATERBOMB_BANG_MIN  Real(500.0f)

#define WATERBOMB_EXPLODE_RADIUS Real(700.0f)
#define WATERBOMB_EXPLODE_RADIUS_SQ (WATERBOMB_EXPLODE_RADIUS * WATERBOMB_EXPLODE_RADIUS)
#define WATERBOMB_EXPLODE_MIN_RADIUS Real(50.0f)
#define WATERBOMB_EXPLODE_MIN_RADIUS_SQ (WATERBOMB_EXPLODE_MIN_RADIUS * WATERBOMB_EXPLODE_MIN_RADIUS)
#define WATERBOMB_EXPLODE_OFFSET Real(50.0f)
#define WATERBOMB_EXPLODE_IMPULSE Real(100000.0f)
#define WATERBOMB_EXPLODE_IMPULSE2 Real(800000.0f)


#define PUTTYBOMB_COUNTDOWN     TO_TIME(Real(10.0f))
#define PUTTYBOMB_COUNTDOWN2    TO_TIME(Real(1.0f))
#define PUTTYBOMB_NORETURN_TIME TO_TIME(Real(3.0f))
#define PUTTYBOMB_BANG_NUM 64
#define PUTTYBOMB_BANG_TIME     TO_TIME(Real(2.5f))
#define PUTTYBOMB_ONE_BANG_TIME TO_TIME(Real(2.0f))
#define PUTTYBOMB_SPHERE_TIME   TO_TIME(Real(0.5f))
#define PUTTYBOMB_BANG_STAGGER (PUTTYBOMB_BANG_TIME - PUTTYBOMB_ONE_BANG_TIME)
#define PUTTYBOMB_BANG_RADIUS   TO_LENGTH(Real(128.0f))
#define PUTTYBOMB_SCORCH_RADIUS TO_LENGTH(Real(256.0f))
#define PUTTYBOMB_PUSH_RANGE    TO_LENGTH(Real(64.0f))
#define PUTTYBOMB_BANG_IMPULSE_RANGE TO_LENGTH(Real(128.0f))
#define PUTTYBOMB_SMOKE_NUM 8


// structures

#ifdef _PC
typedef struct {
    VEC Pos;
    char QuatX, QuatY, QuatZ, QuatW;
    unsigned long ID;
    long Weapon;
    LARGE_INTEGER GlobalID;
    unsigned long Time;
} WEAPON_REMOTE_DATA;
#endif

typedef struct {
    unsigned long ID;
    long Status;
} TARGET_STATUS_DATA;


typedef struct
{
    VEC     Pos;
    VEC     Vel;
    short   Angle;
    short   Spin;

} SHOCK_SPINNER;


typedef struct
{
    long    Alive; 
    REAL    Age, Reach;
    VEC     OldPos;
    BBOX    Box;
} SHOCKWAVE_OBJ;


typedef struct {
    OBJECT *Target;
    bool Exploded;
    REAL Age;
    REAL SmokeTime;
    REAL SparkTime;
#ifndef _PSX
    struct SparkTrailStruct *Trail;
    REAL TrailTime;
#endif
} FIREWORK_OBJ;

typedef struct {
    VEC     Pos, Vel;
    REAL    Age, Size, Life;
} PUTTYBOMB_BANG;


typedef struct {
    REAL Timer, SphereRadius;
    REAL OrigAerialLen;
    VEC Pos;
    BBOX Box;
    PUTTYBOMB_BANG Bang[PUTTYBOMB_BANG_NUM];
#ifndef _PSX
    long SmokeVert[PUTTYBOMB_SMOKE_NUM];
#endif
    REAL SmokeTime;
} PUTTYBOMB_OBJ;


typedef struct {
    REAL Time, TimeAdd;
} PUTTYBOMB_VERT;


typedef struct {
    REAL Age, BangTol, ScalarHoriz, ScalarVert;
} WATERBOMB_OBJ;

typedef struct {
    REAL Age;
    MODEL Model;
    long JumpFlag;
    VEC JumpPos1[MAX_NUM_PLAYERS], JumpPos2[MAX_NUM_PLAYERS];
    void *Player[MAX_NUM_PLAYERS];
  #ifdef OLD_AUDIO
    SAMPLE_3D *ZapSfx;
  #else
    //$TODO: Hey JHarding, do we need a replacement for this?
  #endif
} ELECTROPULSE_OBJ;


typedef struct {
    long Mode;
    REAL Age, LandHeight, Size, MaxSize, Ymin, Ymax;
    VEC Pos[4], Vel[4];
} OILSLICK_OBJ;

typedef struct {
    long Count;
    REAL Age;
    VEC LastPos;
} OILSLICK_DROPPER_OBJ;


typedef struct {
    REAL Age, Radius;
} CHROMEBALL_OBJ;

typedef struct {
} CLONE_OBJ;

typedef struct {
    REAL Time, TimeAdd;
    REAL Size, Offset;
} TURBO2_VERT;

typedef struct {
    REAL Age, LifeTime, SparkTime, TrailTime;
    REAL Force;
} TURBO_OBJ;

typedef struct {
    REAL Age, LifeTime;
    MODEL Model;
} TURBO2_OBJ;

typedef struct {
} SPRING_OBJ;

typedef struct {
    REAL Time, TimeAdd;
} ELECTROPULSE_VERT;

typedef struct {
    REAL X, Z, Radius, SquaredRadius;
    REAL Ymin, Ymax;
} OILSLICK_LIST;

typedef struct {
    MODEL Model;
} ELECTROZAPPED_OBJ;

typedef struct {
    REAL Time, TimeAdd;
} ELECTROZAPPED_VERT;

typedef struct {
    REAL Timer;
    MODEL Model;
} BOMBGLOW_OBJ;

typedef struct {
    REAL Time, TimeAdd;
} BOMBGLOW_VERT;

typedef struct {
    int     Age;
} DUMMYPICKUP_OBJ;

#ifdef _PC
typedef struct {
    LARGE_INTEGER ObjID;
    unsigned long Player1ID;
    unsigned long Player2ID;
} BOMB_TRANSFER_DATA;
#endif

// prototypes
struct PlayerStruct;

extern void ResetOilSlickList(void);
#ifdef _PC
extern void SendWeaponNew(VEC *pos, MAT *mat, unsigned long id, long weapon, LARGE_INTEGER GlobalID);
#endif
extern void ProcessWeaponNew(void);
extern void SendTargetStatus(unsigned long id, long status);
extern void ProcessTargetStatus(void);

extern long InitShockwave(OBJECT *obj, long *flags);
extern long InitFirework(OBJECT *obj, long *flags);
extern long InitPuttyBomb(OBJECT *obj, long *flags);
extern long InitWaterBomb(OBJECT *obj, long *flags);
extern long InitElectroPulse(OBJECT *obj, long *flags);
extern long InitOilSlick(OBJECT *obj, long *flags);
extern long InitOilSlickDropper(OBJECT *obj, long *flags);
extern long InitChromeBall(OBJECT *obj, long *flags);
extern long InitClone(OBJECT *obj, long *flags);
extern long InitTurbo(OBJECT *obj, long *flags);
extern long InitTurbo2(OBJECT *obj, long *flags);
extern long InitSpring(OBJECT *obj, long *flags);
extern long InitElectroZapped(OBJECT *obj, long *flags);
extern long InitBombGlow(OBJECT *obj, long *flags);


extern void ShockwaveHandler(OBJECT *obj);
extern void FireworkHandler(OBJECT *obj);
extern void PuttyBombHandler(OBJECT *obj);
extern void PuttyBombBang(OBJECT *obj);
extern void WaterBombHandler(OBJECT *obj);
extern void ElectroPulseHandler(OBJECT *obj);
extern void OilSlickHandler(OBJECT *obj);
extern void OilSlickDropperHandler(OBJECT *obj);
extern void ChromeBallHandler(OBJECT *obj);
extern void CloneHandler(OBJECT *obj);
extern void Turbo2Handler(OBJECT *obj);
extern void SpringHandler(OBJECT *obj);
extern void ElectroZappedHandler(OBJECT *obj);
extern void BombGlowHandler(OBJECT *obj);
extern void TransferBomb(OBJECT *obj, struct PlayerStruct *player1, struct PlayerStruct *player2);

#ifndef _PSX

extern void RenderShockwave(OBJECT *obj);
extern void RenderElectroPulse(OBJECT *obj);
extern void RenderChromeBall(OBJECT *obj);
extern void RenderPuttyBombBang(OBJECT *obj);
extern void RenderElectroZapped(OBJECT *obj);
extern void RenderWaterBomb(OBJECT *obj);
extern void RenderOilSlick(OBJECT *obj);

#else

extern long InitDummyPickup( OBJECT *obj, long *flags );
extern int  InitFireworkExplosion( OBJECT *obj, long *flags );
extern long InitSplash(OBJECT *obj, long *flags);
extern void RenderShockwave( OBJECT * obj, long * OT, MATRIX * Cam, VECTOR * CamPos );
extern void RenderElectroPulse( OBJECT * obj, long * OT, MATRIX * Cam, VECTOR * CamPos );
extern void RenderChromeBall( OBJECT * obj, long * OT, MATRIX * Cam, VECTOR * CamPos );
extern void RenderPuttyBombBang( OBJECT * obj, long * OT, MATRIX * Cam, VECTOR * CamPos );
extern void RenderElectroZapped( OBJECT *obj, long * OT, MATRIX * Cam, VECTOR * CamPos );
extern void RenderWaterBomb( OBJECT *obj, long * OT, MATRIX * Cam, VECTOR * CamPos );
extern void RenderOilSlick( OBJECT *obj, long * OT, MATRIX * Cam, VECTOR * CamPos );


#endif

extern void ShockwaveWorldMeshFxChecker(void *data);
extern void ShockwaveModelMeshFxChecker(void *data);
extern void PuttyBombWorldMeshFxChecker(void *data);
extern void PuttyBombModelMeshFxChecker(void *data);

extern void RenderTurbo2(OBJECT *obj);
extern void RenderBombGlow(OBJECT *obj);
extern void PuttyBombMove(OBJECT *obj);

extern OBJECT *WeaponTarget(OBJECT *playerObj);


extern long InitPickup(OBJECT *obj, long *flags);

// globals

extern long          OilSlickCount;
extern OILSLICK_LIST OilSlickList[];

#ifdef N64
extern VEC WaterBombVel 
extern VEC WaterBombOff 
extern VEC BombSmokeVel 
#endif


#ifdef PSX

//void DrawOil(VEC *p0, VEC *p1, VEC *p2, VEC *p3, REAL tu, REAL tv, REAL twidth, REAL theight, long * OT,  MATRIX *Cam, VECTOR * CamPos );
void    RenderWeapons( long * OT, CAMERA * Cam );
void    OilSlick( CAR *car, long * OT, MATRIX * Cam, VECTOR * CamPos );
void    RenderRepositionStuff( unsigned long *ot, CAR *car, MATRIX *cam_mat, VECTOR *cam_pos );
void    RenderHalo_2D( long *ot, short x, short y, int in_col, int out_col, short halo_type, short ot_offset, short scale );
void    InitHaloPolys( void );

#endif


#endif
