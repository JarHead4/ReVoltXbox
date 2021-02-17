
#ifndef WEAPON_H
#define WEAPON_H

#ifdef _PC
#include "draw.h"
#endif
#include "newcoll.h"
#include "particle.h"
#include "body.h"
#include "object.h"
#include "Spark.h"

// macros

#define TURBO_NTRAILS 4

#define SHOCKWAVE_VEL 6000.0f
#define SHOCKWAVE_MIN_VEL 3000.0f
#define SHOCKWAVE_MAX_AGE 2.0f
#define SHOCKWAVE_RAD 40.0f
#define SHOCKWAVE_PULL_MAX_MUL 0.1f
#define SHOCKWAVE_PULL_MIN_MUL 0.5f

#define OILSLICK_LIST_MAX 64

#define OILY_WHEEL_TIME		TO_TIME(Real(2.5))

#define OILSLICK_GRAV 2048.0f
#define OILSLICK_MIN_SIZE 32.0f
#define OILSLICK_MAX_SIZE 96.0f

#define DROPPER_GAP 64.0f

#define ELECTRO_RANGE 192.0f
#define ELECTRO_KILL_TIME 3.0f

#define CHROMEBALL_MIN_RAD 4.0f
#define CHROMEBALL_MAX_RAD 64.0f

#define WATERBOMB_RADIUS 30.0f
#define WATERBOMB_MAX_AGE 30.0f
#define WATERBOMB_BANG_VAR 2000.0f
#define WATERBOMB_BANG_MIN 500.0f

#define PUTTYBOMB_COUNTDOWN 10.0f
#define PUTTYBOMB_COUNTDOWN2 1.0f
#define PUTTYBOMB_NORETURN_TIME 1.0f
#define PUTTYBOMB_SMOKE_NUM 8
#define PUTTYBOMB_BANG_NUM 64
#define PUTTYBOMB_BANG_TIME 2.5f
#define PUTTYBOMB_ONE_BANG_TIME 2.0f
#define PUTTYBOMB_SPHERE_TIME 0.5f
#define PUTTYBOMB_BANG_STAGGER (PUTTYBOMB_BANG_TIME - PUTTYBOMB_ONE_BANG_TIME)
#define PUTTYBOMB_BANG_RADIUS 128.0f
#define PUTTYBOMB_SCORCH_RADIUS 256.0f
#define PUTTYBOMB_PUSH_RANGE 64.0f
#define PUTTYBOMB_BANG_IMPULSE_RANGE 128.0f

#define WEAPON_RANGE_MIN Real(100.0)
#define WEAPON_RANGE_MAX Real(3500.0)
#define WEAPON_RANGE_OFFSET Real(200.0)
#define WEAPON_DIR_OFFSET Real(0.25)

// structures

typedef struct {
	long Alive;
	REAL Age, Reach;
	VEC OldPos;
	BBOX Box;
} SHOCKWAVE_OBJ;

typedef struct {
	OBJECT *Target;
	bool Exploded;
	REAL Age;
	REAL SmokeTime;
	REAL SparkTime;
	TRAIL *Trail;
	REAL TrailTime;
} FIREWORK_OBJ;

typedef struct {
	VEC Pos, Vel;
	REAL Age, Size, Life;
} PUTTYBOMB_BANG;

typedef struct {
	REAL Timer, SphereRadius;
	REAL OrigAerialLen;
	VEC Pos;
	BBOX Box;
	PUTTYBOMB_BANG Bang[PUTTYBOMB_BANG_NUM];
	long SmokeVert[PUTTYBOMB_SMOKE_NUM];
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
	REAL Age, LifeTime, SparkTime, TrailTime;
	REAL Force;
	TRAIL *TurboTrail[TURBO_NTRAILS];
} TURBO_OBJ;

typedef struct {
	REAL Age, LifeTime;
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

// prototypes

void ResetOilSlickList(void);

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

extern void RenderShockwave(OBJECT *obj);
extern void ShockwaveWorldMeshFxChecker(void *data);
extern void ShockwaveModelMeshFxChecker(void *data);
extern void PuttyBombWorldMeshFxChecker(void *data);
extern void PuttyBombModelMeshFxChecker(void *data);
extern void RenderElectroPulse(OBJECT *obj);
extern void RenderOilSlick(OBJECT *obj);
extern void RenderChromeBall(OBJECT *obj);
extern void RenderElectroZapped(OBJECT *obj);
extern void RenderBombGlow(OBJECT *obj);
extern void RenderWaterBomb(OBJECT *obj);
extern void RenderPuttyBombBang(OBJECT *obj);
extern void PuttyBombMove(OBJECT *obj);

extern OBJECT *WeaponTarget(OBJECT *playerObj);

// globals

extern long 	     OilSlickCount;
extern OILSLICK_LIST OilSlickList[];


#ifdef PSX

void DrawOil(VEC *p0, VEC *p1, VEC *p2, VEC *p3, REAL tu, REAL tv, REAL twidth, REAL theight, long * OT,  MATRIX *Cam, VECTOR * CamPos );
void ProcessWeapons( CAR * car, long * OT, CAMERA * Cam );



#endif


#endif
