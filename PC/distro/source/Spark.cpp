
#include "ReVolt.h"
#ifndef _PSX
#include "newcoll.h"
#include "Timing.h"
#include "geom.h"
#include "object.h"
#include "Field.h"
#endif
#include "Spark.h"
#ifdef _PC
#include "draw.h"
#endif
#include "Camera.h"
#ifndef _PSX
#include "Visibox.h"
#endif
#ifdef _N64
#include "gamegfx.h"
#endif

SPARK Sparks[MAX_SPARKS];
int NActiveSparks = 0;

TRAIL SparkTrail[MAX_TRAILS];
int NActiveTrails = 0;

void InitSparks();
SPARK *GetFreeSpark();
void FreeSpark(SPARK *spark);
void ProcessSparks();
bool CreateSpark(SPARK_TYPE type, VEC *pos, VEC *vel, REAL velVar, VISIMASK mask);
void DrawSparks();
void SparkCameraCollide(SPARK *spark, VEC *oldPos);
void SparkWorldCollide(SPARK *spark, VEC *oldPos);
void SparkObjectCollide(SPARK *spark);
void SparkConvexCollide(SPARK *spark, NEWBODY *body);
void SparkSphereCollide(SPARK *spark, NEWBODY *body);
REAL SparkProbability(REAL vel);

static SPARK_DATA SparkData[SPARK_NTYPES] = {
	{ // SPARK_SPARK
#ifdef _PC
		{1.0f, 1.0f,			// XSize, YSize
		240.0f / 256.0f, 0 / 256.0f,		// U, V
		16 / 256.0f, 16 / 256.0f,			// Usize, Vsize;
		TPAGE_FX1, 0,	// Tpage, pad;
		0xffffffff},		// RGB;
		SPARK_WORLD_COLLIDE | SPARK_FIELD_AFFECT | SPARK_CAMERA_COLLIDE | SPARK_SEMI | SPARK_OBJECT_COLLIDE,
#endif
#ifdef _N64
		8.0f, 8.0f,
		GFX_SPR_PARTS_2,
		GFX_S_PT2_YSPRK_U, GFX_S_PT2_YSPRK_V,
		16, 16,
		0xFFFF00FF,
		FME_FLARE,
		SPARK_CREATE_TRAIL | SPARK_FIELD_AFFECT | SPARK_SEMI,
#endif
		Real(0.1),		// Mass
		Real(0.02),		// Resistance
		Real(0.1),		// Friction
		Real(0.5),		// Restitution
		500,			// Lifetime (ms)
		50,				// Lifetime variation (ms)
		ZERO, ZERO,		// Spin rate and variation
		ZERO,			// Initial size variation
		ZERO, ZERO,		// Grow rate and variation
		TRAIL_SPARK,	// trail type
	},
	{ // SPARK_SPARK2
#ifdef _PC
		{1.0f, 1.0f,			// XSize, YSize
		240.0f / 256.0f, 0 / 256.0f,		// U, V
		16 / 256.0f, 16 / 256.0f,			// Usize, Vsize;
		TPAGE_FX1, 0,	// Tpage, pad;
		0xffffffff},		// RGB;
		SPARK_SEMI,
#endif
#ifdef _N64
		5.0f, 5.0f,
		GFX_SPR_PARTS_2,
		GFX_S_PT2_SPARK_U, GFX_S_PT2_SPARK_V,
		16, 16,
		0xFFFF00FF,
		SPARK_SEMI,
#endif
		Real(0.1),		// Mass
		Real(0.02),		// Resistance
		Real(0.1),		// Friction
		Real(0.5),		// Restitution
		300,			// Lifetime (ms)
		20,				// Lifetime variation (ms)
		ZERO, ZERO,		// Spin rate and variation
		ZERO,			// Initial size variation
		ZERO, ZERO,		// Grow rate and variation
		TRAIL_NONE,	// trail type
	},
	{ // SPARK_SNOW
#ifdef _PC
		{5.0f, 5.0f,			// XSize, YSize
		224.0f / 256.0f, 0.0f,			// U, V
		16.0f / 256.0f, 16.0f / 256.0f,			// Usize, Vsize;
		TPAGE_FX1, 0,	// Tpage, pad;
		0xffffff},		// RGB;
		SPARK_WORLD_COLLIDE | SPARK_OBJECT_COLLIDE | SPARK_FIELD_AFFECT | SPARK_CAMERA_COLLIDE | SPARK_SEMI,
#endif
#ifdef _N64
		5.0f, 5.0f,
		GFX_SPR_PARTS_2,
		GFX_S_PT2_SNOW_U, GFX_S_PT2_SNOW_V,
		16, 16,
		0xFFFFFFFF,
		FME_FLARE,
		SPARK_CREATE_TRAIL | SPARK_FIELD_AFFECT | SPARK_SEMI,
#endif
		Real(0.03),		// Mass
		Real(0.1),		// Resistance
		Real(0.6),		// Friction
		Real(0.2),		// Restitution
		10000,		// Lifetime (ms)
		5000,		// Lifetime variation (ms)
		ZERO, ZERO,		// Spin rate and variation
		ZERO,			// Initial size variation
		ZERO, ZERO,		// Grow rate and variation
		TRAIL_NONE,
	},
	{ // SPARK_POPCORN
#ifdef _PC
		{10.0f, 10.0f,			// XSize, YSize
		208.0f / 256.0f, 0.0f,			// U, V
		16.0f / 256.0f, 16.0f / 256.0f,			// Usize, Vsize;
		TPAGE_FX1, 0,	// Tpage, pad;
		0xFF33FF},		// RGB;
		SPARK_WORLD_COLLIDE | SPARK_OBJECT_COLLIDE | SPARK_CREATE_TRAIL | SPARK_FIELD_AFFECT | SPARK_CAMERA_COLLIDE | SPARK_SEMI,
#endif
#ifdef _N64
		10.0f, 10.0f,
		GFX_SPR_PARTS_1,
		GFX_S_PT1_POPC_U, GFX_S_PT1_POPC_V,
		16, 16,
		0x808080FF,
		SPARK_CREATE_TRAIL | SPARK_FIELD_AFFECT | SPARK_SEMI,
		0,
#endif
		Real(0.3),		// Mass
		Real(0.01),		// Resistance
		Real(0.7),		// Friction
		Real(0.8),		// Restitution
		10000,			// Lifetime (ms)
		0,				// Lifetime variation (ms)
		ZERO, ZERO,		// Spin rate and variation
		ZERO,			// Initial size variation
		ZERO, ZERO,		// Grow rate and variation
		TRAIL_NONE,
	},
	{ // SPARK_GRAVEL
#ifdef _PC
		{3.5f, 3.5f,			// XSize, YSize
		192.0f / 256.0f, 0 / 256.0f,		// U, V
		16 / 256.0f, 16 / 256.0f,			// Usize, Vsize;
		TPAGE_FX1, 0,	// Tpage, pad;
		0xff808080},		// RGB;
		SPARK_WORLD_COLLIDE | SPARK_OBJECT_COLLIDE | SPARK_CREATE_TRAIL | SPARK_FIELD_AFFECT | SPARK_CAMERA_COLLIDE,
#endif
#ifdef _N64
		2.0f, 2.0f,
		GFX_SPR_PARTS_1,
		GFX_S_PT1_GRAV_U, GFX_S_PT1_GRAV_V,
		16, 16,
		0x808080FF,
		0,
		SPARK_CREATE_TRAIL | SPARK_FIELD_AFFECT,
#endif
		Real(0.1),		// Mass
		Real(0.02),		// Resistance
		Real(0.7),		// Friction
		Real(0.5),		// Restitution
		500,			// Lifetime (ms)
		100,			// Lifetime variation (ms)
		ZERO, ZERO,		// Spin rate and variation
		ZERO,			// Initial size variation
		ZERO, ZERO,		// Grow rate and variation
		TRAIL_NONE,
	},
	{ // SPARK_SAND
#ifdef _PC
		{2.0f, 2.0f,			// XSize, YSize
		240.0f / 256.0f, 0 / 256.0f,		// U, V
		16 / 256.0f, 16 / 256.0f,			// Usize, Vsize;
		TPAGE_FX1, 0,	// Tpage, pad;
		0xffffffff},		// RGB;
		SPARK_WORLD_COLLIDE | SPARK_OBJECT_COLLIDE | SPARK_CREATE_TRAIL | SPARK_FIELD_AFFECT | SPARK_CAMERA_COLLIDE | SPARK_SEMI,
#endif
#ifdef _N64
		2.0f, 2.0f,
		GFX_SPR_PARTS_2,
		GFX_S_PT2_SPARK_U, GFX_S_PT2_SPARK_V,
		16, 16,
		0xFFFF00FF,
		0,
		SPARK_CREATE_TRAIL | SPARK_FIELD_AFFECT | SPARK_SEMI,
#endif
		Real(0.1),		// Mass
		Real(0.02),		// Resistance
		Real(0.7),		// Friction
		Real(0.5),		// Restitution
		500000,			// Lifetime (ms)
		100000,			// Lifetime variation (ms)
		ZERO, ZERO,		// Spin rate and variation
		ZERO,			// Initial size variation
		ZERO, ZERO,		// Grow rate and variation
		TRAIL_NONE,
	},
	{ // SPARK_MUD
#ifdef _PC
		{2.0f, 2.0f,			// XSize, YSize
		240.0f / 256.0f, 0 / 256.0f,		// U, V
		16 / 256.0f, 16 / 256.0f,			// Usize, Vsize;
		TPAGE_FX1, 0,	// Tpage, pad;
		0xffffffff},		// RGB;
		SPARK_WORLD_COLLIDE | SPARK_OBJECT_COLLIDE | SPARK_CREATE_TRAIL | SPARK_FIELD_AFFECT | SPARK_CAMERA_COLLIDE | SPARK_SEMI,
#endif
#ifdef _N64
		2.0f, 2.0f,
		GFX_SPR_PARTS_1,
		GFX_S_PT1_MUD_U, GFX_S_PT1_MUD_V,
		16, 16,
		0x808080FF,
		0,
		SPARK_CREATE_TRAIL | SPARK_FIELD_AFFECT | SPARK_SEMI,
#endif
		Real(0.1),		// Mass
		Real(0.02),		// Resistance
		Real(1.0),		// Friction
		Real(0.5),		// Restitution
		500,			// Lifetime (ms)
		100,			// Lifetime variation (ms)
		ZERO, ZERO,		// Spin rate and variation
		ZERO,			// Initial size variation
		ZERO, ZERO,		// Grow rate and variation
		TRAIL_NONE,
	},
	{ // SPARK_ELECTRIC
#ifdef _PC
		{0.8f, 0.8f,			// XSize, YSize
		224.0f / 256.0f, 16.0f / 256.0f,		// U, V
		16.0f / 256.0f, 16.0f / 256.0f,			// Usize, Vsize;
		TPAGE_FX1, 0,	// Tpage, pad;
		0xffc0c0ff},	// RGB;
		SPARK_OBJECT_COLLIDE | SPARK_SEMI | SPARK_FIELD_AFFECT,
#endif
#ifdef _N64
		1.0f, 1.0f,
		GFX_SPR_PARTS_2,
		GFX_S_PT2_EBALL_U, GFX_S_PT2_EBALL_V,
		16, 16,
		0x2020FFFF,
		FME_FLARE,
		SPARK_CREATE_TRAIL | SPARK_SEMI,
#endif
		Real(0.1),		// Mass
		Real(0.02),		// Resistance
		Real(0.1),		// Friction
		Real(0.5),		// Restitution
		150,			// Lifetime (ms)
		0,				// Lifetime variation (ms)
		ZERO, ZERO,		// Spin rate and variation
		ZERO,			// Initial size variation
		ZERO, ZERO,		// Grow rate and variation
		TRAIL_NONE,		// trail type
	},
	{ // SPARK_WATER
#ifdef _PC
		{10.0f, 10.0f,			// XSize, YSize
		192.0f / 256.0f, 16 / 256.0f,		// U, V
		16.0f / 256.0f, 16.0f / 256.0f,			// Usize, Vsize;
		TPAGE_FX1, 0,	// Tpage, pad;
		0xff4433dd},		// RGB;
		SPARK_WORLD_COLLIDE | SPARK_OBJECT_COLLIDE | SPARK_CREATE_TRAIL | SPARK_FIELD_AFFECT | SPARK_CAMERA_COLLIDE | SPARK_SEMI,
#endif
#ifdef _N64
		10.0f, 10.0f,
		GFX_SPR_PARTS_1,
		GFX_S_PT1_WATER_U, GFX_S_PT1_WATER_V,
		16, 16,
		0x808080FF,
		0,
		SPARK_CREATE_TRAIL | SPARK_FIELD_AFFECT | SPARK_SEMI,
#endif
		Real(0.03),		// Mass
		Real(0.001),	// Resistance
		Real(0.8),		// Friction
		Real(0.2),		// Restitution
		2000,			// Lifetime (ms)
		1000,			// Lifetime variation (ms)
		ZERO, ZERO,		// Spin rate and variation
		ZERO,			// Initial size variation
		ZERO, ZERO,		// Grow rate and variation
		TRAIL_NONE,
	},
	{ // SPARK_DIRT
#ifdef _PC
		{5.0f, 5.0f,			// XSize, YSize
		208.0f / 256.0f, 16.0f,			// U, V
		16.0f / 256.0f, 16.0f / 256.0f,			// Usize, Vsize;
		TPAGE_FX1, 0,	// Tpage, pad;
		0xffffffff},		// RGB;
		SPARK_WORLD_COLLIDE | SPARK_OBJECT_COLLIDE | SPARK_CREATE_TRAIL | SPARK_FIELD_AFFECT | SPARK_CAMERA_COLLIDE | SPARK_SEMI,
#endif
#ifdef _N64
		5.0f, 5.0f,
		GFX_SPR_PARTS_1,
		GFX_S_PT1_MUD_U, GFX_S_PT1_MUD_V,
		16, 16,
		0x808080FF,
		0,
		SPARK_CREATE_TRAIL | SPARK_FIELD_AFFECT | SPARK_SEMI,
#endif
		Real(0.03),		// Mass
		Real(0.001),	// Resistance
		Real(0.1),		// Friction
		Real(0.2),		// Restitution
		1000,			// Lifetime (ms)
		500,			// Lifetime variation (ms)
		ZERO, ZERO,		// Spin rate and variation
		ZERO,			// Initial size variation
		ZERO, ZERO,		// Grow rate and variation
		TRAIL_NONE,
	},
	{ // SPARK_SMOKE1
#ifdef _PC
		{6.0f, 6.0f,	// XSize, YSize
		0.0f, 0.0f,		// U, V
		64.0f / 256.0f, 64.0f / 256.0f,	// Usize, Vsize;
		TPAGE_FX1, 0,	// Tpage, pad;
		0x404040},	// RGB;
		SPARK_OBJECT_COLLIDE | SPARK_WORLD_COLLIDE | SPARK_SPINS | SPARK_GROWS | SPARK_SEMI,
#endif
#ifdef _N64
		24.0f, 24.0f,
		GFX_SPR_SMOKE,
		0, 0,
		GFX_SPR_SMOKE_W, GFX_SPR_SMOKE_H,
		0xA0A0A080,
		0,
		SPARK_SPINS | SPARK_GROWS | SPARK_SEMI,
#endif
		Real(0.03),		// Mass
		Real(0.002),		// Resistance
		Real(0.0),		// Friction
		Real(0.0),		// Restitution
		500,			// Lifetime (ms)
		100,			// Lifetime variation (ms)
		Real(0.0), Real(6.0),	// Spin rate and variation
		Real(2.0),		// Initial size variation
		Real(0.0), Real(36.0),		// Grow rate and variation
		TRAIL_NONE,
	},
	{ // SPARK_SMOKE2
#ifdef _PC
		{64.0f, 64.0f,	// XSize, YSize
		0.0f, 0.0f,		// U, V
		64.0f / 256.0f, 64.0f / 256.0f,			// Usize, Vsize;
		TPAGE_FX1, 0,	// Tpage, pad;
		0x606060},		// RGB;
		SPARK_OBJECT_COLLIDE | SPARK_WORLD_COLLIDE | SPARK_SPINS | SPARK_GROWS | SPARK_SEMI,
#endif
#ifdef _N64
		64.0f, 64.0f,
		GFX_SPR_SMOKE,
		0, 0,
		GFX_SPR_SMOKE_W, GFX_SPR_SMOKE_H,
		0xA0A0A0FF,
		0,
		SPARK_SPINS | SPARK_GROWS | SPARK_SEMI,
#endif
		Real(0.03),		// Mass
		Real(0.02),		// Resistance
		Real(0.001),	// Friction
		Real(0.0),		// Restitution
		2600,		// Lifetime (ms)
		0,				// Lifetime variation (ms)
		Real(0.0), Real(6.0),	// Spin rate and variation
		Real(0.0),		// Initial size variation
		Real(40.0), Real(20.0),		// Grow rate and variation
		TRAIL_NONE,
	},
	{ // SPARK_SMOKE3
#ifdef _PC
		{4.0f, 4.0f,	// XSize, YSize
		0.0f, 0.0f,		// U, V
		64.0f / 256.0f, 64.0f / 256.0f,	// Usize, Vsize;
		TPAGE_FX1, 0,	// Tpage, pad;
		0x404040},	// RGB;
		SPARK_OBJECT_COLLIDE | SPARK_SPINS | SPARK_GROWS | SPARK_SEMI,
#endif
#ifdef _N64
		4.0f, 4.0f,
		GFX_SPR_SMOKE,
		0, 0,
		GFX_SPR_SMOKE_W, GFX_SPR_SMOKE_H,
		0x404040FF,
		0,
		SPARK_SPINS | SPARK_GROWS | SPARK_SEMI,
#endif
		Real(0.03),		// Mass
		Real(0.0),		// Resistance
		Real(0.0),		// Friction
		Real(0.0),		// Restitution
		1500,			// Lifetime (ms)
		100,			// Lifetime variation (ms)
		Real(0.0), Real(6.0),	// Spin rate and variation
		Real(2.0),		// Initial size variation
		Real(0.0), Real(36.0),		// Grow rate and variation
		TRAIL_NONE,
	},
	{ // SPARK_BLUE
#ifdef _PC
		{4.0f, 4.0f,	// XSize, YSize
		224.0f / 256.0f, 16.0f / 256.0f,	// U, V
		16.0f / 256.0f, 16.0f / 256.0f,		// Usize, Vsize;
		TPAGE_FX1, 0,	// Tpage, pad;
		0x808080},		// RGB;
#endif
#ifdef _N64
		4.0f, 4.0f,
		GFX_SPR_PARTS_2,
		GFX_S_PT2_YSPRK_U, GFX_S_PT2_YSPRK_V,
		16, 16,
		0x0000ffFF,
		FME_FLARE,
#endif
		SPARK_SEMI,
		Real(0.0),		// Mass
		Real(0.0),		// Resistance
		Real(0.0),		// Friction
		Real(0.0),		// Restitution
		500,			// Lifetime (ms)
		0,				// Lifetime variation (ms)
		Real(0.0), Real(0.0),	// Spin rate and variation
		Real(0.0),				// Initial size variation
		Real(0.0), Real(0.0),	// Grow rate and variation
		TRAIL_NONE,
	},
	{ // SPARK_BIGBLUE
#ifdef _PC
		{4.0f, 4.0f,	// XSize, YSize
		224.0f / 256.0f, 16.0f / 256.0f,	// U, V
		16.0f / 256.0f, 16.0f / 256.0f,		// Usize, Vsize;
		TPAGE_FX1, 0,	// Tpage, pad;
		0x808080},		// RGB;
#endif
#ifdef _N64
		4.0f, 4.0f,
		GFX_SPR_FLARE2,
		0, 0, 
		GFX_SPR_FLARE2_W, GFX_SPR_FLARE2_H,
		0x0000FFFF,
		FME_FLARE,
#endif
		SPARK_FIELD_AFFECT | SPARK_SEMI,
		Real(0.1),		// Mass
		Real(0.02),		// Resistance
		Real(0.1),		// Friction
		Real(0.5),		// Restitution
		500,			// Lifetime (ms)
		500,			// Lifetime variation (ms)
		Real(0.0), Real(0.0),	// Spin rate and variation
		Real(8.0),				// Initial size variation
		Real(0.0), Real(0.0),	// Grow rate and variation
		TRAIL_NONE,
	},
	{ // SPARK_SMALLORANGE
#ifdef _PC
		{18.0f, 18.0f,						// XSize, YSize
		192.0f / 256.0f, 32 / 256.0f,		// U, V
		16.0f / 256.0f, 16.0f / 256.0f,		// Usize, Vsize;
		TPAGE_FX1, 0,						// Tpage, pad;
		0xffffffff},						// RGB;
#endif
#ifdef _N64
		10.0f, 10.0f,
		GFX_SPR_PARTS_2,
		GFX_S_PT2_YSPRK_U, GFX_S_PT2_YSPRK_V,
		16, 16,
		0xFFA000FF,
		FME_FLARE,
#endif
		SPARK_WORLD_COLLIDE | SPARK_FIELD_AFFECT | SPARK_SEMI,
		Real(0.03),		// Mass
		Real(0.016),	// Resistance
		Real(0.6),		// Friction
		Real(0.0),		// Restitution
		1200,			// Lifetime (ms)
		500,			// Lifetime variation (ms)
		ZERO, ZERO,		// Spin rate and variation
		Real(2),		// Initial size variation
		Real(-2), ZERO,		// Grow rate and variation
		TRAIL_FIREWORK,
	},
	{ // SPARK_SMALLRED
#ifdef _PC
		{20.0f, 20.0f,						// XSize, YSize
		240.0f / 256.0f, 32 / 256.0f,		// U, V
		16.0f / 256.0f, 16.0f / 256.0f,		// Usize, Vsize;
		TPAGE_FX1, 0,						// Tpage, pad;
		0xffffffff},						// RGB;
#endif
#ifdef _N64
		10.0f, 10.0f,
		GFX_SPR_PARTS_2,
		GFX_S_PT2_RSPRK_U, GFX_S_PT2_RSPRK_V,
		16, 16,
		0xFF0000FF,
		FME_FLARE,
#endif
		SPARK_WORLD_COLLIDE | SPARK_CREATE_TRAIL | SPARK_FIELD_AFFECT | SPARK_SEMI,
		Real(0.03),		// Mass
		Real(0.012),	// Resistance
		Real(0.6),		// Friction
		Real(0.0),		// Restitution
		1500,			// Lifetime (ms)
		500,			// Lifetime variation (ms)
		ZERO, ZERO,		// Spin rate and variation
		Real(5),		// Initial size variation
		Real(-5), ZERO,		// Grow rate and variation
		TRAIL_FIREWORK,
	},
	{ // SPARK_EXPLOSION1
#ifdef _PC
		{12.0f, 12.0f,						// XSize, YSize
		64.0f / 256.0f, 0 / 256.0f,		// U, V
		64.0f / 256.0f, 64.0f / 256.0f,		// Usize, Vsize;
		TPAGE_FX1, 0,						// Tpage, pad;
		0xffffffff},						// RGB;
#endif
#ifdef _N64
		10.0f, 10.0f,
		GFX_SPR_FIRE,
		0, 0,
		GFX_SPR_FIRE_W, GFX_SPR_FIRE_H,
		0x808080FF,
		FME_FLARE,
#endif
		SPARK_SEMI | SPARK_SPINS | SPARK_GROWS,
		Real(0.03),		// Mass
		Real(0.015),	// Resistance
		Real(0.6),		// Friction
		Real(0.0),		// Restitution
		500,			// Lifetime (ms)
		000,			// Lifetime variation (ms)
		ZERO, Real(6),		// Spin rate and variation
		Real(0),		// Initial size variation
		Real(128), Real(0),		// Grow rate and variation
		TRAIL_NONE,
	},
	{ // SPARK_EXPLOSION2
#ifdef _PC
		{8.0f, 8.0f,						// XSize, YSize
		64.0f / 256.0f, 0 / 256.0f,		// U, V
		64.0f / 256.0f, 64.0f / 256.0f,		// Usize, Vsize;
		TPAGE_FX1, 0,						// Tpage, pad;
		0xffffffff},						// RGB;
#endif
#ifdef _N64
		10.0f, 10.0f,
		GFX_SPR_FIRE,
		0, 0,
		GFX_SPR_FIRE_W, GFX_SPR_FIRE_H,
		0x808080FF,
		FME_FLARE,
#endif
		SPARK_SEMI | SPARK_SPINS | SPARK_GROWS,
		Real(0.03),		// Mass
		Real(0.015),	// Resistance
		Real(0.6),		// Friction
		Real(0.0),		// Restitution
		500,		// Lifetime (ms)
		300,			// Lifetime variation (ms)
		ZERO, Real(32),		// Spin rate and variation
		Real(0),		// Initial size variation
		Real(12), Real(5),		// Grow rate and variation
		TRAIL_NONE,
	},
	{ // SPARK_STAR
#ifdef _PC
		{8.0f, 8.0f,	// XSize, YSize
		241.0f / 256.0f, 32 / 256.0f,		// U, V
		15.0f / 256.0f, 16.0f / 256.0f,		// Usize, Vsize;
		TPAGE_FX1, 0,	// Tpage, pad;
		0xffffff},		// RGB;
#endif
#ifdef _N64
		4.0f, 4.0f,
		GFX_SPR_PARTS_2,
		GFX_S_PT2_YSPRK_U, GFX_S_PT2_YSPRK_V,
		16, 16,
		0x00ffffff,
		FME_FLARE | FME_NOZB,
#endif
		SPARK_SEMI | SPARK_GROWS,
		Real(0.0),		// Mass
		Real(0.0),		// Resistance
		Real(0.0),		// Friction
		Real(0.0),		// Restitution
		500,			// Lifetime (ms)
		0,				// Lifetime variation (ms)
		Real(0.0), Real(0.0),	// Spin rate and variation
		Real(0.0),				// Initial size variation
		Real(30.0), Real(0.0),	// Grow rate and variation
		TRAIL_NONE,
	},
};


/////////////////////////////////////////////////////////////////////
//
// Trail Data:
//
/////////////////////////////////////////////////////////////////////

TRAIL_DATA TrailData[TRAIL_NTYPES] = {
	{ // TRAIL_FIREWORK
		225.0f / 256.0f, 33.0f / 256.0f,	// U, V
		14.0f / 256.0f, 14.0f / 256.0f,		// Usize, Vsize
		225.0f / 256.0f, 33.0f / 256.0f,	// U, V
		0xff, 0xff, 0xff, 0xff,				// ARGB
		TRAIL_FADES | TRAIL_SHRINKS,
		Real(0.3),							// Lifetime
		Real(5),							// Width
		TRAIL_MAX_BITS,						// number of sections
	},
	{ // TRAIL_SPARK
		246.0f / 256.0f, 0.0f / 256.0f,	// U, V
		2.0f / 256.0f, 16.0f / 256.0f,		// Usize, Vsize
		246.0f / 256.0f, 0.0f / 256.0f,	// U, V
		0xff, 0xff, 0xff, 0xff,				// ARGB
		TRAIL_FADES | TRAIL_SHRINKS,
		Real(0.03),							// Lifetime
		Real(1),							// Width
		2,									// number of sections
	},
	{ // TRAIL_SMOKE
		224.0f / 256.0f, 48.0f / 256.0f,	// U, V
		16.0f / 256.0f, 16.0f / 256.0f,		// Usize, Vsize
		240.0f / 256.0f, 48.0f / 256.0f,	// U, V
		0xff, 0x88, 0x88, 0xaa,				// ARGB
		TRAIL_FADES | TRAIL_EXPANDS,
		Real(0.4),							// Lifetime
		Real(10),							// Width
		TRAIL_MAX_BITS,						// number of sections
	},
};


/////////////////////////////////////////////////////////////////////
//
// NextFreeTrail:
//
/////////////////////////////////////////////////////////////////////

TRAIL *GetFreeTrail(long trailType)
{
	int iTrail;

	Assert(trailType < TRAIL_NTYPES);

	for (iTrail = 0; iTrail < MAX_TRAILS; iTrail++) {
		if (SparkTrail[iTrail].Free) {
			NActiveTrails++;
			SparkTrail[iTrail].Free = FALSE;
			SparkTrail[iTrail].Data = &TrailData[trailType];
			SparkTrail[iTrail].NTrails = 0;
			SparkTrail[iTrail].MaxTrails = TrailData[trailType].MaxTrails;
			SparkTrail[iTrail].FirstTrail = 0;
			return &SparkTrail[iTrail];
		}
	}

	return NULL;
}

/////////////////////////////////////////////////////////////////////
//
// FreeTrail:
//
/////////////////////////////////////////////////////////////////////

void FreeTrail(TRAIL *trail)
{
	trail->Free = TRUE;
	NActiveTrails--;
}


/////////////////////////////////////////////////////////////////////
//
// UpdateTrail:
//
/////////////////////////////////////////////////////////////////////

void UpdateTrail(TRAIL *trail, VEC *newPos)
{
	if (trail->FirstTrail == trail->MaxTrails - 1) {
		trail->FirstTrail = 0;
	} else {
		trail->FirstTrail++;
	}

	if (trail->NTrails < trail->MaxTrails) {
		trail->NTrails++;
	}

	CopyVec(newPos, &trail->Pos[trail->FirstTrail]);
}

void ModifyFirstTrail(TRAIL *trail, VEC *newPos)
{
	CopyVec(newPos, &trail->Pos[trail->FirstTrail]);
}

/////////////////////////////////////////////////////////////////////
//
// InitSparks:
//
/////////////////////////////////////////////////////////////////////

void InitSparks()
{
	int iSpark, iTrail;

	for (iSpark = 0; iSpark < MAX_SPARKS; iSpark++) {
		Sparks[iSpark].Free = TRUE;
	}
	NActiveSparks = 0;

	for (iTrail = 0; iTrail < MAX_TRAILS; iTrail++) {
		SparkTrail[iTrail].Free = TRUE;
	}
	NActiveTrails = 0;
}

/////////////////////////////////////////////////////////////////////
//
// GetFreeSpark:
//
/////////////////////////////////////////////////////////////////////

SPARK *GetFreeSpark()
{
	int iSpark;

	// Find first free spark
	for (iSpark = 0; iSpark < MAX_SPARKS; iSpark++) {
		if (Sparks[iSpark].Free) {
			Sparks[iSpark].Free = FALSE;
			NActiveSparks++;
			return &Sparks[iSpark];
		}
	}

	// No free sparks
	return NULL;
}

/////////////////////////////////////////////////////////////////////
//
// FreeSpark:
//
/////////////////////////////////////////////////////////////////////

void FreeSpark(SPARK *spark)
{
	if (spark->Trail != NULL) {
		FreeTrail(spark->Trail);
	}

	spark->Free = TRUE;
	NActiveSparks--;
}

/////////////////////////////////////////////////////////////////////
//
// UpdateAllSparks:
//
/////////////////////////////////////////////////////////////////////

void ProcessSparks()
{

#ifndef _PSX	

	int iSpark;
	VEC imp, angImp, oldPos;
	FIELD_DATA fieldData;
	SPARK *spark;

	for (iSpark = 0; iSpark < MAX_SPARKS; iSpark++) {
		spark = &Sparks[iSpark];

		// See if this spark is active
		if (spark->Free) continue;

#ifndef _PSX
		// Check against visimask
		if (CamVisiMask & spark->VisiMask) continue;
#endif

		// See if it is time to kill the spark
		spark->Age += TimerDiff;
		if (spark->Age > MS2TIME(spark->Data->LifeTime)) {
			FreeSpark(spark);
			continue;
		}

		// Rotate the spark
		if (spark->Data->SparkType & SPARK_SPINS) {
			spark->Spin += spark->SpinRate * TimeStep;
		}

		// Expand the spark
		if (spark->Data->SparkType & SPARK_GROWS) {
			spark->Grow += spark->GrowRate * TimeStep;
//			if (spark->Grow < ZERO) spark->Grow = ZERO;
		}

		// Get the forces on the spark from the force fields
		if (spark->Data->SparkType & SPARK_FIELD_AFFECT) {
			fieldData.ObjectID = FIELD_PARENT_NONE;
			fieldData.Mass = spark->Data->Mass;
			fieldData.Pos = &spark->Pos;
			fieldData.Vel = &spark->Vel;
			fieldData.AngVel = &ZeroVector;
			fieldData.Quat = &IdentityQuat;
			fieldData.Mat = &Identity;

			AllFieldImpulses(&fieldData, &imp, &angImp);

			// Calculate new velocity
			VecPlusEqScalarVec(&spark->Vel, ONE / spark->Data->Mass, &imp);
		}

		// Add frictional drag
		VecMulScalar(&spark->Vel, ONE - (spark->Data->Resistance * FRICTION_TIME_SCALE * TimeStep));

		// Update position
		CopyVec(&spark->Pos, &oldPos);
		VecPlusEqScalarVec(&spark->Pos, TimeStep, &spark->Vel);

		// Deal with collisions
		if (spark->Data->SparkType & SPARK_OBJECT_COLLIDE) {
			SparkObjectCollide(spark);
		}
		if (spark->Data->SparkType & SPARK_WORLD_COLLIDE) {
			SparkWorldCollide(spark, &oldPos);
		}
#ifdef _PC
		if (spark->Data->SparkType & SPARK_CAMERA_COLLIDE) {
			SparkCameraCollide(spark, &oldPos);
		}
#endif

		// Update the trail
		if (spark->Trail != NULL) {
			spark->TrailTime += TimeStep;
			if (spark->TrailTime > spark->Trail->Data->LifeTime / spark->Trail->MaxTrails) {
				UpdateTrail(spark->Trail, &spark->Pos);
				spark->TrailTime = ZERO;
			} else {
				ModifyFirstTrail(spark->Trail, &spark->Pos);
			}

		}
	}



#endif

}


/////////////////////////////////////////////////////////////////////
//
// SparkCameraCollide:
//
/////////////////////////////////////////////////////////////////////
#ifdef _PC
void SparkCameraCollide(SPARK *spark, VEC *oldPos)
{
	REAL time, depth, velDotNorm;
	VEC dPos, wPos;
	BBOX bBox;
	NEWCOLLPOLY *collPoly;

	collPoly = &CAM_MainCamera->CollPoly;

	// Quick bounding-box test
	SetBBox(&bBox, 
		Min(spark->Pos.v[X], oldPos->v[X]),
		Max(spark->Pos.v[X], oldPos->v[X]),
		Min(spark->Pos.v[Y], oldPos->v[Y]),
		Max(spark->Pos.v[Y], oldPos->v[Y]),
		Min(spark->Pos.v[Z], oldPos->v[Z]),
		Max(spark->Pos.v[Z], oldPos->v[Z]));
	if(!BBTestXZY(&bBox, &collPoly->BBox)) return;

	// Check for point passing through collision polygon
	if (!LinePlaneIntersect(oldPos, &spark->Pos, &collPoly->Plane, &time, &depth)) {
		return;
	}

	// Calculate the intersection point
	VecMinusVec(&spark->Pos, oldPos, &dPos);
	VecPlusScalarVec(oldPos, time, &dPos, &wPos);

	// Make sure the spark is travelling towards the poly
	velDotNorm = VecDotVec(&spark->Vel, PlaneNormal(&collPoly->Plane));
	if (velDotNorm > ZERO) return;


	// Check intersection point is within the polygon boundary
	if (!PointInCollPolyBounds(&wPos, collPoly)) {
		return;
	}

	// Keep spark on inside of the poly
	VecPlusEqScalarVec(&spark->Pos, -depth + COLL_EPSILON, PlaneNormal(&collPoly->Plane));
	VecPlusEqScalarVec(&spark->Pos, TimeStep, &CAM_MainCamera->Vel);

	// Rebound
	VecPlusEqScalarVec(&spark->Vel, -velDotNorm, PlaneNormal(&collPoly->Plane));
	VecMulScalar(&spark->Vel, (ONE - spark->Data->Friction));
	VecPlusEqScalarVec(&spark->Vel, -(spark->Data->Restitution * velDotNorm), PlaneNormal(&collPoly->Plane));


}
#endif

/////////////////////////////////////////////////////////////////////
//
// SparkWorldCollide:
//
/////////////////////////////////////////////////////////////////////

void SparkWorldCollide(SPARK *spark, VEC *oldPos)
{
	int iPoly;
	REAL time, depth, velDotNorm;
	VEC dPos, wPos;
	BBOX bBox;
	COLLGRID *grid;
	NEWCOLLPOLY *collPoly;

	grid = PosToCollGrid(&spark->Pos);
	if (grid == NULL) return;

	for (iPoly = 0; iPoly < grid->NCollPolys; iPoly++)
	{
#ifndef _PSX
		collPoly = grid->CollPolyPtr[iPoly];
#else
		collPoly = &COL_WorldCollPoly[grid->CollPolyIndices[iPoly]];
#endif

		if (PolyCameraOnly(collPoly)) continue;

		// Quick bounding-box test
		SetBBox(&bBox, 
			Min(spark->Pos.v[X], oldPos->v[X]),
			Max(spark->Pos.v[X], oldPos->v[X]),
			Min(spark->Pos.v[Y], oldPos->v[Y]),
			Max(spark->Pos.v[Y], oldPos->v[Y]),
			Min(spark->Pos.v[Z], oldPos->v[Z]),
			Max(spark->Pos.v[Z], oldPos->v[Z]));
		if(!BBTestYXZ(&bBox, &collPoly->BBox)) continue;

		// Check for point passing through collision polygon
		if (!LinePlaneIntersect(oldPos, &spark->Pos, &collPoly->Plane, &time, &depth)) {
			continue;
		}

		// Calculate the intersection point
		VecMinusVec(&spark->Pos, oldPos, &dPos);
		VecPlusScalarVec(oldPos, time, &dPos, &wPos);

		// Make sure the spark is travelling towards the poly
		velDotNorm = VecDotVec(&spark->Vel, PlaneNormal(&collPoly->Plane));
		if (velDotNorm > ZERO) continue;


		// Check intersection point is within the polygon boundary
		if (!PointInCollPolyBounds(&wPos, collPoly)) {
			continue;
		}

		// Keep spark on inside of the poly
		VecPlusEqScalarVec(&spark->Pos, -depth, PlaneNormal(&collPoly->Plane));

		// Rebound
		VecPlusEqScalarVec(&spark->Vel, -velDotNorm, PlaneNormal(&collPoly->Plane));
		VecMulScalar(&spark->Vel, (ONE - spark->Data->Friction));
		VecPlusEqScalarVec(&spark->Vel, -(spark->Data->Restitution * velDotNorm), PlaneNormal(&collPoly->Plane));


	}
}



/////////////////////////////////////////////////////////////////////
//
// SparkObjectCollide: check for collisions between sparks and objects
//
/////////////////////////////////////////////////////////////////////

void SparkObjectCollide(SPARK *spark)
{
	OBJECT	*obj;

	for (obj = OBJ_ObjectHead; obj != NULL; obj = obj->next) {

		// See if this object allows collisions
		if (obj->CollType == COLL_TYPE_NONE) continue;
		if (obj->body.CollSkin.AllowObjColls == FALSE) continue;

		// Quick bounding box test
		if(!PointInBBox(&spark->Pos, &obj->body.CollSkin.BBox)) continue;

		if (IsBodyConvex(&obj->body)) {
			SparkConvexCollide(spark, &obj->body);
		} else if (IsBodySphere(&obj->body)) {
			SparkSphereCollide(spark, &obj->body);
		}
	}
}

void SparkConvexCollide(SPARK *spark, NEWBODY *body)
{
	int iSkin;
	REAL depth, velDotNorm;
	VEC dVel;
	PLANE plane;

	for (iSkin = 0; iSkin < body->CollSkin.NConvex; iSkin++) {
	
		if (!PointInConvex(&spark->Pos, &body->CollSkin.WorldConvex[iSkin], &plane, &depth)) continue;

		// move spark out of object and rebound
		VecPlusEqScalarVec(&spark->Pos, -depth + COLL_EPSILON, PlaneNormal(&plane));
		velDotNorm = VecDotVec(&spark->Vel, PlaneNormal(&plane));
		VecPlusEqScalarVec(&spark->Vel, -(ONE + spark->Data->Restitution) * velDotNorm, PlaneNormal(&plane));

		// Add simple friction
		VecMinusVec(&body->Centre.Vel, &spark->Vel, &dVel);
		VecPlusEqScalarVec(&spark->Vel, spark->Data->Friction, &dVel);
	}
}

void SparkSphereCollide(SPARK *spark, NEWBODY *body) 
{

}



/////////////////////////////////////////////////////////////////////
//
// CreateSpark: Create a spark travelling along the look direction
// of the passed matrix, with a random variation (0-1).
//
/////////////////////////////////////////////////////////////////////

bool CreateSpark(SPARK_TYPE type, VEC *pos, VEC *vel, REAL velVar, VISIMASK mask)
{

	
#ifndef _PSX	

	VEC dV;
	SPARK *spark;


	Assert(type < SPARK_NTYPES);

	// Get the next available spark
	spark = GetFreeSpark();

	// make sure there were some left
	if (spark == NULL) return FALSE;

	// Set the visimask
	spark->VisiMask = mask;

	// Choose the random vector to add to the velocity
	SetVec(&dV, velVar - frand(2 * velVar), velVar - frand(2 * velVar), velVar - frand(2 * velVar));

	// Generate the initial velocity
	VecPlusVec(vel, &dV, &spark->Vel)

	// Set the initial position
	CopyVec(pos, &spark->Pos);

	// Setup pointer to physical info for this spark type
	spark->Data = &SparkData[type];

	// Set the death time
	spark->Age = (long)frand(SparkData[type].LifeTimeVar);

	// Set initial rotation
	if (spark->Data->SparkType & SPARK_SPINS) {
		spark->Spin = frand(ONE);
		spark->SpinRate = spark->Data->SpinRateBase + (spark->Data->SpinRateVar * HALF) - frand(spark->Data->SpinRateVar);
	} else {
		spark->Spin = ZERO;
		spark->SpinRate = ZERO;
	}

	// Set initial growth amount
	spark->Grow = frand(spark->Data->SizeVar);
	if (spark->Data->SparkType & SPARK_GROWS) {
		spark->GrowRate = spark->Data->GrowRateBase + frand(spark->Data->GrowRateVar);
	} else {
		spark->GrowRate = ZERO;
	}

	// Set up the trail
	if (spark->Data->SparkType & SPARK_CREATE_TRAIL) {
		spark->Trail = GetFreeTrail(spark->Data->TrailType);
		if (spark->Trail != NULL) {
			CopyVec(&spark->Pos, &spark->Trail->Pos[0]);
			spark->TrailTime = ZERO;
		}
	} else {
		spark->Trail = NULL;
	}

	// Sucess
	return TRUE;

#endif

}


/////////////////////////////////////////////////////////////////////
//
// SparkProbability: return the probability of a spark being 
// generated for a sliding velocity as passed
//
/////////////////////////////////////////////////////////////////////

REAL SparkProbability(REAL vel)
{
	REAL prob;

	prob = (vel - MIN_SPARK_VEL) / (MAX_SPARK_VEL - MIN_SPARK_VEL);

	if (prob < ZERO) return ZERO;
	if (prob > ONE) return ONE;
	return prob;
}


/////////////////////////////////////////////////////////////////////
//
// RenderSpark: draw one spark
//
/////////////////////////////////////////////////////////////////////

#ifdef _PC
void DrawSparkTrail(TRAIL *trail)
{
	int iTrail, thisTrail, lastTrail;
	long a, r, g, b;
	REAL dx, dy, dt, dLen, width;
	DRAW_SEMI_POLY poly;
	VERTEX_TEX1 sPos, ePos;
	VERTEX_TEX1 *vert = poly.Verts;

	poly.Fog = FALSE;
	poly.VertNum = 4;
	poly.Tpage = TPAGE_FX1;
	poly.DrawFlag = D3DDP_DONOTUPDATEEXTENTS;
	poly.SemiType = TRUE;

	SET_TPAGE((short)poly.Tpage);
	vert[3].tu = trail->Data->U;
	vert[3].tv = trail->Data->V;

	vert[0].tu = trail->Data->U + trail->Data->Usize;
	vert[0].tv = trail->Data->V;

	vert[1].tu = trail->Data->U + trail->Data->Usize;
	vert[1].tv = trail->Data->V + trail->Data->Vsize;

	vert[2].tu = trail->Data->U;
	vert[2].tv = trail->Data->V + trail->Data->Vsize;
	

	FOG_OFF();
	ALPHA_SRC(D3DBLEND_ONE);
	ALPHA_DEST(D3DBLEND_ONE);

	thisTrail = trail->FirstTrail;
	lastTrail = (trail->FirstTrail - 1);
	if (lastTrail < 0) lastTrail = trail->MaxTrails - 1;

	// Calculate first section end coordinates
	RotTransPersVector(&ViewMatrixScaled, &ViewTransScaled, &trail->Pos[thisTrail], (REAL*)&ePos);
	RotTransPersVector(&ViewMatrixScaled, &ViewTransScaled, &trail->Pos[lastTrail], (REAL*)&sPos);
	dx = ePos.sx - sPos.sx;
	dy = ePos.sy - sPos.sy;
	dLen = (REAL)sqrt(dx * dx + dy * dy);
	dt = dx;
	dx = dy / dLen;
	dy = -dt / dLen;

	// Set up first end vertices
	width = trail->Data->Width;
	vert[2].sx = ePos.sx + dx * width * ePos.rhw * RenderSettings.GeomPers;
	vert[2].sy = ePos.sy + dy * width * ePos.rhw * RenderSettings.GeomPers;
	vert[2].sz = ePos.sz;
	vert[2].rhw = ePos.rhw;
	vert[3].sx = ePos.sx - dx * width * ePos.rhw * RenderSettings.GeomPers;
	vert[3].sy = ePos.sy - dy * width * ePos.rhw * RenderSettings.GeomPers;
	vert[3].sz = ePos.sz;
	vert[3].rhw = ePos.rhw;

	for (iTrail = 1; iTrail < trail->NTrails; iTrail++) {

		// Get coordinates of next section
		RotTransPersVector(&ViewMatrixScaled, &ViewTransScaled, &trail->Pos[lastTrail], (REAL*)&sPos);
		dx = ePos.sx - sPos.sx;
		dy = ePos.sy - sPos.sy;
		dLen = (REAL)sqrt(dx * dx + dy * dy);
		if (dLen < SMALL_REAL) {
			continue;
		}
		dt = dx;
		dx = dy / dLen;
		dy = -dt / dLen;

		// Set up next end vertices
		if (trail->Data->Type & TRAIL_SHRINKS) {
			width = trail->Data->Width - (trail->Data->Width * iTrail) / trail->MaxTrails;
		} else if (trail->Data->Type & TRAIL_EXPANDS) {
			width = trail->Data->Width + (3 * trail->Data->Width * iTrail) / (trail->MaxTrails * 2);
		} else {
			width = trail->Data->Width;
		}
		vert[0].sx = sPos.sx - dx * width * sPos.rhw * RenderSettings.GeomPers;
		vert[0].sy = sPos.sy - dy * width * sPos.rhw * RenderSettings.GeomPers;
		vert[0].sz = sPos.sz;
		vert[0].rhw = sPos.rhw;
		vert[1].sx = sPos.sx + dx * width * sPos.rhw * RenderSettings.GeomPers;
		vert[1].sy = sPos.sy + dy * width * sPos.rhw * RenderSettings.GeomPers;
		vert[1].sz = sPos.sz;
		vert[1].rhw = sPos.rhw;

		// Choose a colour
		if (trail->Data->Type & TRAIL_FADES) {
			a = trail->Data->A - iTrail * (trail->Data->A / trail->NTrails);
			r = trail->Data->R - iTrail * (trail->Data->R / trail->NTrails);
			g = trail->Data->G - iTrail * (trail->Data->G / trail->NTrails);
			b = trail->Data->B - iTrail * (trail->Data->B / trail->NTrails);
		} else {
			a = trail->Data->A;
			r = trail->Data->R;
			g = trail->Data->G;
			b = trail->Data->B;
		}
		vert[0].color = vert[1].color = r << 16 | g << 8 | b;
		if (trail->Data->Type & TRAIL_FADES) {
			a = trail->Data->A - (iTrail - 1) * (trail->Data->A / trail->NTrails);
			r = trail->Data->R - (iTrail - 1) * (trail->Data->R / trail->NTrails);
			g = trail->Data->G - (iTrail - 1) * (trail->Data->G / trail->NTrails);
			b = trail->Data->B - (iTrail - 1) * (trail->Data->B / trail->NTrails);
		} else {
			a = trail->Data->A;
			r = trail->Data->R;
			g = trail->Data->G;
			b = trail->Data->B;
		}
		vert[2].color = vert[3].color = r << 16 | g << 8 | b;

		if (iTrail == trail->NTrails - 1) {
			vert[3].tu = trail->Data->EndU;
			vert[3].tv = trail->Data->EndV;

			vert[0].tu = trail->Data->EndU + trail->Data->Usize;
			vert[0].tv = trail->Data->EndV;

			vert[1].tu = trail->Data->EndU + trail->Data->Usize;
			vert[1].tv = trail->Data->EndV + trail->Data->Vsize;

			vert[2].tu = trail->Data->EndU;
			vert[2].tv = trail->Data->EndV + trail->Data->Vsize;
		}

		// draw the poly
		D3Ddevice->DrawPrimitive(D3DPT_TRIANGLEFAN, FVF_TEX1, poly.Verts, poly.VertNum, poly.DrawFlag);

		// copy last to next
		ePos.sx = sPos.sx;
		ePos.sy = sPos.sy;
		ePos.sz = sPos.sz;
		ePos.rhw = sPos.rhw;

		vert[2].sx = vert[1].sx;
		vert[2].sy = vert[1].sy;
		vert[2].sz = vert[1].sz;
		vert[2].rhw = vert[1].rhw;
		vert[3].sx = vert[0].sx;
		vert[3].sy = vert[0].sy;
		vert[3].sz = vert[0].sz;
		vert[3].rhw = vert[0].rhw;

		// move on
		thisTrail = lastTrail;
		if (--lastTrail < 0) lastTrail = trail->MaxTrails - 1;

	}
}

void DrawSparks()
{
	int iSpark;
	long per, semi;
	MAT	mat;
	FACING_POLY poly, *sparkPoly;
	SPARK *spark;

	ALPHA_ON();
	ALPHA_SRC(D3DBLEND_ONE);
	ALPHA_DEST(D3DBLEND_ONE);

	ZWRITE_OFF();


	// loop thru all sparks
	for (iSpark = 0; iSpark < MAX_SPARKS; iSpark++) {
		if (!Sparks[iSpark].Free) {
			spark = &Sparks[iSpark];
			sparkPoly = &spark->Data->FacingPoly;

			// Check against visimask
			if (CamVisiMask & spark->VisiMask) continue;

			// Set up faceme info
			poly.Tpage = sparkPoly->Tpage;
			poly.Xsize = sparkPoly->Xsize;
			poly.Ysize = sparkPoly->Ysize;
			poly.U = sparkPoly->U;
			poly.V = sparkPoly->V;
			poly.Usize = sparkPoly->Usize;
			poly.Vsize = sparkPoly->Vsize;

			poly.RGB = sparkPoly->RGB;

			// Expand poly if spark
			poly.Xsize += spark->Grow;
			poly.Ysize += spark->Grow;
			if (poly.Xsize < 0.0f) {
				continue;
			}
			if (poly.Ysize < 0.0f) {
				continue;
			}

			// set the semi transparency flag
			if (spark->Data->SparkType & SPARK_SEMI) {
				semi = 0;
				per = (100 * (spark->Data->LifeTime - spark->Age)) / spark->Data->LifeTime;
				ModelChangeGouraud((MODEL_RGB*)&poly.RGB, per);
				ALPHA_ON();
				ZWRITE_OFF();
			} else {
				semi = -1;
				ALPHA_OFF();
				ZWRITE_ON();
			}

			// Rotate the spark if necessary and then draw
			if (spark->Data->SparkType & SPARK_SPINS) {
				RotationZ(&mat, spark->Spin);
				DrawFacingPolyRot(&Sparks[iSpark].Pos, &mat, &poly, -1, 0);
			} else {
				DrawFacingPoly(&Sparks[iSpark].Pos, &poly, -1, 0);
			}

		}
	}
}

/////////////////////////////////////////////////////////////////////
//
// DrawTrails:
//
/////////////////////////////////////////////////////////////////////

void DrawTrails()
{
	int iTrail;

	for (iTrail = 0; iTrail < MAX_TRAILS; iTrail++) {
		if (SparkTrail[iTrail].Free) continue;

		DrawSparkTrail(&SparkTrail[iTrail]);
	}
}

#endif

//--------------------------------------------------------------------------------------------------------------------------

#ifdef _N64

void DrawSparks()
{
	int iSpark;
	long alpha;

	// loop thru all sparks
	for (iSpark = 0; iSpark < MAX_SPARKS; iSpark++)
	{
		if (!Sparks[iSpark].Free)
		{
			alpha =  (Sparks[iSpark].Data->Colour & 0xFF) - ((0xff * Sparks[iSpark].Age) / (Sparks[iSpark].Data->LifeTime));
			if (alpha < 0) { alpha = 0; }
			FME_AddFaceMe(Sparks[iSpark].Data->GfxIdx, Sparks[iSpark].Data->u, Sparks[iSpark].Data->v, Sparks[iSpark].Data->w, Sparks[iSpark].Data->h,
						  &Sparks[iSpark].Pos, Sparks[iSpark].Data->XSize + Sparks[iSpark].Grow, Sparks[iSpark].Data->YSize + Sparks[iSpark].Grow, Sparks[iSpark].Spin, (Sparks[iSpark].Data->Colour & 0xFFFFFF00) | (alpha & 0xFF), Sparks[iSpark].Data->Flag);
		}
	}
}
#endif
