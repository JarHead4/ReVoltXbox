//-----------------------------------------------------------------------------
// File: spark.cpp
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"
#ifdef _N64
#include "gfx.h"
#include "gamegfx.h"
#endif
#include "newcoll.h"
#include "timing.h"
#include "geom.h"
#include "util.h"
#include "object.h"
#include "field.h"
#include "spark.h"
#ifdef _PC
#include "draw.h"
#endif
#include "camera.h"
#ifndef _PSX
#include "visibox.h"
#endif

//
// Global Function prototypes
//

void InitSparks();
SPARK *GetFreeSpark();
void FreeSpark(SPARK *spark);
void ProcessSparks();
void DrawSparks(void);
void DrawSparkTrail(TRAIL *trail);
void SparkCameraCollide(SPARK *spark, VEC *oldPos);
bool SparkWorldCollide(SPARK *spark, VEC *oldPos);
bool SparkObjectCollide(SPARK *spark);
bool SparkConvexCollide(SPARK *spark, NEWBODY *body);
bool SparkSphereCollide(SPARK *spark, NEWBODY *body);
REAL SparkProbability(REAL vel);


//
// Global variables
//

SPARK Sparks[MAX_SPARKS];
int NActiveSparks = 0;

TRAIL SparkTrail[MAX_TRAILS];
int NActiveTrails = 0;

REAL gSparkDensity = ONE;
REAL gSmokeDensity = ONE;

//
// Static variables
//

//static SPARK_DATA SparkData[SPARK_NTYPES] = {
SPARK_DATA SparkData[SPARK_NTYPES] = {
    { // SPARK_SPARK
#ifdef _PC
        {1.0f, 1.0f,            // XSize, YSize
        240.0f / 256.0f, 0 / 256.0f,        // U, V
        16 / 256.0f, 16 / 256.0f,           // Usize, Vsize;
        TPAGE_FX1, 0,   // Tpage, pad;
        0xffffffff},        // RGB;
        SPARK_WORLD_COLLIDE | SPARK_FIELD_AFFECT | SPARK_CAMERA_COLLIDE | SPARK_SEMI | SPARK_OBJECT_COLLIDE,
#endif
#ifdef _N64
        8.0f, 8.0f,
        GFX_SPR_PARTS_2,
        GFX_S_PT2_YSPRK_U, GFX_S_PT2_YSPRK_V,
        16, 16,
        0xFFFF00FF,
        FME_FLARE,
        SPARK_FIELD_AFFECT | SPARK_SEMI,
#endif
        Real(0.1),      // Mass
        Real(0.02),     // Resistance
        Real(0.1),      // Friction
        Real(0.5),      // Restitution
        TO_TIME(Real(0.5)),         // Lifetime (ms)
        TO_TIME(Real(0.05)),                // Lifetime variation (ms)
        ZERO, ZERO,     // Spin rate and variation
        ZERO,           // Initial size variation
        ZERO, ZERO,     // Grow rate and variation
        TRAIL_SPARK,    // trail type
    },
    { // SPARK_SPARK2
#ifdef _PC
        {1.0f, 1.0f,            // XSize, YSize
        240.0f / 256.0f, 0 / 256.0f,        // U, V
        16 / 256.0f, 16 / 256.0f,           // Usize, Vsize;
        TPAGE_FX1, 0,   // Tpage, pad;
        0xffffffff},        // RGB;
        SPARK_SEMI,
#endif
#ifdef _N64
        5.0f, 5.0f,
        GFX_SPR_PARTS_2,
        GFX_S_PT2_YSPRK_U, GFX_S_PT2_YSPRK_V,
        16, 16,
        0xFFFF00FF,
        FME_FLARE,
        SPARK_SEMI,
#endif
        Real(0.1),      // Mass
        Real(0.02),     // Resistance
        Real(0.1),      // Friction
        Real(0.5),      // Restitution
        TO_TIME(Real(0.3)),         // Lifetime (ms)
        TO_TIME(Real(0.02)),                // Lifetime variation (ms)
        ZERO, ZERO,     // Spin rate and variation
        ZERO,           // Initial size variation
        ZERO, ZERO,     // Grow rate and variation
        TRAIL_NONE, // trail type
    },
    { // SPARK_SNOW
#ifdef _PC
        {5.0f, 5.0f,            // XSize, YSize
        224.0f / 256.0f, 0.0f,          // U, V
        16.0f / 256.0f, 16.0f / 256.0f,         // Usize, Vsize;
        TPAGE_FX1, 0,   // Tpage, pad;
        0xffffff},      // RGB;
        SPARK_WORLD_COLLIDE | SPARK_OBJECT_COLLIDE | SPARK_FIELD_AFFECT | SPARK_CAMERA_COLLIDE | SPARK_SEMI,
#endif
#ifdef _N64
        5.0f, 5.0f,
        GFX_SPR_PARTS_2,
        GFX_S_PT2_SNOW_U, GFX_S_PT2_SNOW_V,
        16, 16,
        0xFFFFFFFF,
        FME_FLARE,
        SPARK_FIELD_AFFECT | SPARK_SEMI,
#endif
        Real(0.03),     // Mass
        Real(0.1),      // Resistance
        Real(0.6),      // Friction
        Real(0.2),      // Restitution
        TO_TIME(Real(10.0)),        // Lifetime (ms)
        TO_TIME(Real(5.0)),     // Lifetime variation (ms)
        ZERO, ZERO,     // Spin rate and variation
        ZERO,           // Initial size variation
        ZERO, ZERO,     // Grow rate and variation
        TRAIL_NONE,
    },
    { // SPARK_POPCORN
#ifdef _PC
        {10.0f, 10.0f,          // XSize, YSize
        208.0f / 256.0f, 0.0f,          // U, V
        16.0f / 256.0f, 16.0f / 256.0f,         // Usize, Vsize;
        TPAGE_FX1, 0,   // Tpage, pad;
        0xFF33FF},      // RGB;
        SPARK_WORLD_COLLIDE | SPARK_OBJECT_COLLIDE | SPARK_FIELD_AFFECT | SPARK_CAMERA_COLLIDE | SPARK_SEMI,
#endif
#ifdef _N64
        10.0f, 10.0f,
        GFX_SPR_PARTS_1,
        GFX_S_PT1_POPC_U, GFX_S_PT1_POPC_V,
        16, 16,
        0x808080FF,
        SPARK_FIELD_AFFECT | SPARK_SEMI,
        0,
#endif
        Real(0.3),      // Mass
        Real(0.01),     // Resistance
        Real(0.7),      // Friction
        Real(0.8),      // Restitution
        TO_TIME(Real(10.0)),            // Lifetime (ms)
        TO_TIME(Real(0)),               // Lifetime variation (ms)
        ZERO, ZERO,     // Spin rate and variation
        ZERO,           // Initial size variation
        ZERO, ZERO,     // Grow rate and variation
        TRAIL_NONE,
    },
    { // SPARK_GRAVEL
#ifdef _PC
        {3.5f, 3.5f,            // XSize, YSize
        192.0f / 256.0f, 0 / 256.0f,        // U, V
        16 / 256.0f, 16 / 256.0f,           // Usize, Vsize;
        TPAGE_FX1, 0,   // Tpage, pad;
        0xff808080},        // RGB;
        SPARK_WORLD_COLLIDE | SPARK_OBJECT_COLLIDE | SPARK_FIELD_AFFECT | SPARK_CAMERA_COLLIDE,
#endif
#ifdef _N64
        2.0f, 2.0f,
        GFX_SPR_PARTS_1,
        GFX_S_PT1_GRAV_U, GFX_S_PT1_GRAV_V,
        16, 16,
        0x808080FF,
        0,
        SPARK_FIELD_AFFECT,
#endif
        Real(0.1),      // Mass
        Real(0.02),     // Resistance
        Real(0.7),      // Friction
        Real(0.5),      // Restitution
        TO_TIME(Real(1.5)),         // Lifetime (ms)
        TO_TIME(Real(0.5)),         // Lifetime variation (ms)
        ZERO, Real(25),             // Spin rate and variation
        ZERO,           // Initial size variation
        ZERO, ZERO,     // Grow rate and variation
        TRAIL_NONE,
    },
    { // SPARK_SAND
#ifdef _PC
        {2.0f, 2.0f,            // XSize, YSize
        240.0f / 256.0f, 0 / 256.0f,        // U, V
        16 / 256.0f, 16 / 256.0f,           // Usize, Vsize;
        TPAGE_FX1, 0,   // Tpage, pad;
        0xffffffff},        // RGB;
        SPARK_WORLD_COLLIDE | SPARK_OBJECT_COLLIDE | SPARK_FIELD_AFFECT | SPARK_CAMERA_COLLIDE | SPARK_SEMI,
#endif
#ifdef _N64
        2.0f, 2.0f,
        GFX_SPR_PARTS_2,
        GFX_S_PT2_SPARK_U, GFX_S_PT2_SPARK_V,
        16, 16,
        0xFFFF00FF,
        0,
        SPARK_FIELD_AFFECT | SPARK_SEMI,
#endif
        Real(0.1),      // Mass
        Real(0.02),     // Resistance
        Real(0.7),      // Friction
        Real(0.5),      // Restitution
        TO_TIME(Real(0.5)),         // Lifetime (ms)
        TO_TIME(Real(0.1)),         // Lifetime variation (ms)
        ZERO, ZERO,     // Spin rate and variation
        ZERO,           // Initial size variation
        ZERO, ZERO,     // Grow rate and variation
        TRAIL_NONE,
    },
    { // SPARK_GRASS
#ifdef _PC
        {4.0f, 4.0f,            // XSize, YSize
        208.0f / 256.0f, 0.0f / 256.0f,     // U, V
        16 / 256.0f, 16 / 256.0f,           // Usize, Vsize;
        TPAGE_FX1, 0,   // Tpage, pad;
        0xffffffff},        // RGB;
        SPARK_WORLD_COLLIDE | SPARK_OBJECT_COLLIDE | SPARK_FIELD_AFFECT | SPARK_SPINS,// | SPARK_SEMI,
#endif
#ifdef _N64
        3.0f, 3.0f,
        GFX_SPR_PARTS_1,
        GFX_S_PT1_MUD_U, GFX_S_PT1_MUD_V,
        16, 16,
        0x808080FF,
        0,
        SPARK_FIELD_AFFECT | SPARK_SEMI,
#endif
        Real(0.1),      // Mass
        Real(0.02),     // Resistance
        Real(1.0),      // Friction
        Real(0.5),      // Restitution
        TO_TIME(Real(1.0)),     // Lifetime (ms)
        TO_TIME(Real(0.3)),     // Lifetime variation (ms)
        Real(0.0), Real(15.0),  // Spin rate and variation
        ZERO,                   // Initial size variation
        ZERO, ZERO,             // Grow rate and variation
        TRAIL_NONE,
    },
    { // SPARK_ELECTRIC
#ifdef _PC
        {0.8f, 0.8f,            // XSize, YSize
        224.0f / 256.0f, 16.0f / 256.0f,        // U, V
        16.0f / 256.0f, 16.0f / 256.0f,         // Usize, Vsize;
        TPAGE_FX1, 0,   // Tpage, pad;
        0xffc0c0ff},    // RGB;
        SPARK_OBJECT_COLLIDE | SPARK_SEMI | SPARK_FIELD_AFFECT,
#endif
#ifdef _N64
        1.0f, 1.0f,
        GFX_SPR_PARTS_2,
        GFX_S_PT2_EBALL_U, GFX_S_PT2_EBALL_V,
        16, 16,
        0x2020FFFF,
        FME_FLARE,
        SPARK_SEMI,
#endif
        Real(0.1),      // Mass
        Real(0.02),     // Resistance
        Real(0.1),      // Friction
        Real(0.5),      // Restitution
        TO_TIME(Real(0.15)),            // Lifetime (ms)
        TO_TIME(Real(0)),               // Lifetime variation (ms)
        ZERO, ZERO,     // Spin rate and variation
        ZERO,           // Initial size variation
        ZERO, ZERO,     // Grow rate and variation
        TRAIL_NONE,     // trail type
    },
    { // SPARK_WATER
#ifdef _PC
        {10.0f, 10.0f,          // XSize, YSize
        192.0f / 256.0f, 16 / 256.0f,       // U, V
        16.0f / 256.0f, 16.0f / 256.0f,         // Usize, Vsize;
        TPAGE_FX1, 0,   // Tpage, pad;
        0xff4433dd},        // RGB;
        SPARK_WORLD_COLLIDE | SPARK_OBJECT_COLLIDE | SPARK_FIELD_AFFECT | SPARK_CAMERA_COLLIDE | SPARK_SEMI,
#endif
#ifdef _N64
        10.0f, 10.0f,
        GFX_SPR_PARTS_1,
        GFX_S_PT1_WATER_U, GFX_S_PT1_WATER_V,
        16, 16,
        0x808080FF,
        0,
        SPARK_FIELD_AFFECT | SPARK_SEMI,
#endif
        Real(0.03),     // Mass
        Real(0.001),    // Resistance
        Real(0.8),      // Friction
        Real(0.2),      // Restitution
        TO_TIME(Real(2.0)),         // Lifetime (ms)
        TO_TIME(Real(1.0)),         // Lifetime variation (ms)
        ZERO, ZERO,     // Spin rate and variation
        ZERO,           // Initial size variation
        ZERO, ZERO,     // Grow rate and variation
        TRAIL_NONE,
    },
    { // SPARK_DIRT
#ifdef _PC
        {3.5f, 3.5f,            // XSize, YSize
        208.0f / 256.0f, 16.0f / 256.0f,            // U, V
        16.0f / 256.0f, 16.0f / 256.0f,         // Usize, Vsize;
        TPAGE_FX1, 0,   // Tpage, pad;
        0xFFFFFFFF},        // RGB;
        SPARK_WORLD_COLLIDE | SPARK_OBJECT_COLLIDE | SPARK_FIELD_AFFECT,
#endif
#ifdef _N64
        5.0f, 5.0f,
        GFX_SPR_PARTS_1,
        GFX_S_PT1_MUD_U, GFX_S_PT1_MUD_V,
        16, 16,
        0x808080FF,
        0,
        SPARK_FIELD_AFFECT,
#endif
        Real(0.03),     // Mass
        Real(0.001),    // Resistance
        Real(0.4),      // Friction
        Real(0.2),      // Restitution
        TO_TIME(Real(1.5)),         // Lifetime (ms)
        TO_TIME(Real(0.5)),         // Lifetime variation (ms)
        ZERO, Real(25),             // Spin rate and variation
        ZERO,           // Initial size variation
        ZERO, ZERO,     // Grow rate and variation
        TRAIL_NONE,
    },
    { // SPARK_SMOKE1
#ifdef _PC
        {6.0f, 6.0f,    // XSize, YSize
        0.0f, 0.0f,     // U, V
        64.0f / 256.0f, 64.0f / 256.0f, // Usize, Vsize;
        TPAGE_FX1, 0,   // Tpage, pad;
        0x404040},  // RGB;
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
        Real(0.03),     // Mass
        Real(0.002),        // Resistance
        Real(0.0),      // Friction
        Real(0.0),      // Restitution
        TO_TIME(Real(0.5)),         // Lifetime (ms)
        TO_TIME(Real(0.1)),         // Lifetime variation (ms)
        Real(0.0), Real(6.0),   // Spin rate and variation
        Real(2.0),      // Initial size variation
        Real(0.0), Real(36.0),      // Grow rate and variation
        TRAIL_NONE,
    },
    { // SPARK_SMOKE2
#ifdef _PC
        {64.0f, 64.0f,  // XSize, YSize
        0.0f, 0.0f,     // U, V
        64.0f / 256.0f, 64.0f / 256.0f,         // Usize, Vsize;
        TPAGE_FX1, 0,   // Tpage, pad;
        0x606060},      // RGB;
        SPARK_OBJECT_COLLIDE | SPARK_WORLD_COLLIDE | SPARK_SPINS | SPARK_GROWS | SPARK_SEMI,
#endif
#ifdef _N64
        64.0f, 64.0f,
        GFX_SPR_SMOKE,
        0, 0,
        GFX_SPR_SMOKE_W, GFX_SPR_SMOKE_H,
        0xA0A0A080,
        0,
        SPARK_SPINS | SPARK_GROWS | SPARK_SEMI,
#endif
        Real(0.03),     // Mass
        Real(0.02),     // Resistance
        Real(0.001),    // Friction
        Real(0.0),      // Restitution
        TO_TIME(Real(2.6)),         // Lifetime (ms)
        TO_TIME(Real(0)),               // Lifetime variation (ms)
        Real(0.0), Real(6.0),   // Spin rate and variation
        Real(0.0),      // Initial size variation
        Real(40.0), Real(20.0),     // Grow rate and variation
        TRAIL_NONE,
    },
    { // SPARK_SMOKE3
#ifdef _PC
        {4.0f, 4.0f,    // XSize, YSize
        0.0f, 0.0f,     // U, V
        64.0f / 256.0f, 64.0f / 256.0f, // Usize, Vsize;
        TPAGE_FX1, 0,   // Tpage, pad;
        0x404040},  // RGB;
        SPARK_OBJECT_COLLIDE | SPARK_SPINS | SPARK_GROWS | SPARK_SEMI,
#endif
#ifdef _N64
        4.0f, 4.0f,
        GFX_SPR_SMOKE,
        0, 0,
        GFX_SPR_SMOKE_W, GFX_SPR_SMOKE_H,
        0xA0A0A080,
        0,
        SPARK_SPINS | SPARK_GROWS | SPARK_SEMI,
#endif
        Real(0.03),     // Mass
        Real(0.0),      // Resistance
        Real(0.0),      // Friction
        Real(0.0),      // Restitution
        TO_TIME(Real(1.5)),         // Lifetime (ms)
        TO_TIME(Real(0.1)),         // Lifetime variation (ms)
        Real(0.0), Real(6.0),   // Spin rate and variation
        Real(2.0),      // Initial size variation
        Real(0.0), Real(36.0),      // Grow rate and variation
        TRAIL_NONE,
    },
    { // SPARK_BLUE
#ifdef _PC
        {4.0f, 4.0f,    // XSize, YSize
        224.0f / 256.0f, 16.0f / 256.0f,    // U, V
        16.0f / 256.0f, 16.0f / 256.0f,     // Usize, Vsize;
        TPAGE_FX1, 0,   // Tpage, pad;
        0x808080},      // RGB;
#endif
#ifdef _N64
        16.0f, 16.0f,
        GFX_SPR_PARTS_2,
        GFX_S_PT2_YSPRK_U, GFX_S_PT2_YSPRK_V,
        16, 16,
        0x0000ffFF,
        FME_FLARE,
#endif
        SPARK_SEMI,
        Real(0.0),      // Mass
        Real(0.0),      // Resistance
        Real(0.0),      // Friction
        Real(0.0),      // Restitution
        TO_TIME(Real(0.5)),         // Lifetime (ms)
        TO_TIME(Real(0)),               // Lifetime variation (ms)
        Real(0.0), Real(0.0),   // Spin rate and variation
        Real(0.0),              // Initial size variation
        Real(0.0), Real(0.0),   // Grow rate and variation
        TRAIL_NONE,
    },
    { // SPARK_BIGBLUE
#ifdef _PC
        {4.0f, 4.0f,    // XSize, YSize
        224.0f / 256.0f, 16.0f / 256.0f,    // U, V
        16.0f / 256.0f, 16.0f / 256.0f,     // Usize, Vsize;
        TPAGE_FX1, 0,   // Tpage, pad;
        0x808080},      // RGB;
#endif
#ifdef _N64
        16.0f, 16.0f,
        GFX_SPR_FLARE2,
        0, 0, 
        GFX_SPR_FLARE2_W, GFX_SPR_FLARE2_H,
        0x0000FFFF,
        FME_FLARE,
#endif
        SPARK_FIELD_AFFECT | SPARK_SEMI,
        Real(0.1),      // Mass
        Real(0.02),     // Resistance
        Real(0.1),      // Friction
        Real(0.5),      // Restitution
        TO_TIME(Real(0.5)),         // Lifetime (ms)
        TO_TIME(Real(0.5)),         // Lifetime variation (ms)
        Real(0.0), Real(0.0),   // Spin rate and variation
        Real(8.0),              // Initial size variation
        Real(0.0), Real(0.0),   // Grow rate and variation
        TRAIL_NONE,
    },
    { // SPARK_SMALLORANGE
#ifdef _PC
        {18.0f, 18.0f,                      // XSize, YSize
        192.0f / 256.0f, 32 / 256.0f,       // U, V
        16.0f / 256.0f, 16.0f / 256.0f,     // Usize, Vsize;
        TPAGE_FX1, 0,                       // Tpage, pad;
        0xffffffff},                        // RGB;
#endif
#ifdef _N64
        48.0f, 48.0f,
        GFX_SPR_PARTS_2,
        GFX_S_PT2_YSPRK_U, GFX_S_PT2_YSPRK_V,
        16, 16,
        0xFFA000FF,
        FME_FLARE,
#endif
        SPARK_WORLD_COLLIDE | SPARK_FIELD_AFFECT | SPARK_SEMI,
        Real(0.03),     // Mass
        Real(0.016),    // Resistance
        Real(0.0),      // Friction
        Real(0.4),      // Restitution
        TO_TIME(Real(1.2)),         // Lifetime (ms)
        TO_TIME(Real(0.5)),         // Lifetime variation (ms)
        ZERO, ZERO,     // Spin rate and variation
        Real(2),        // Initial size variation
        Real(-2), ZERO,     // Grow rate and variation
        TRAIL_FIREWORK,
    },
    { // SPARK_SMALLRED
#ifdef _PC
        {20.0f, 20.0f,                      // XSize, YSize
        240.0f / 256.0f, 32 / 256.0f,       // U, V
        16.0f / 256.0f, 16.0f / 256.0f,     // Usize, Vsize;
        TPAGE_FX1, 0,                       // Tpage, pad;
        0xffffffff},                        // RGB;
#endif
#ifdef _N64
        48.0f, 48.0f,
        GFX_SPR_PARTS_2,
        GFX_S_PT2_RSPRK_U, GFX_S_PT2_RSPRK_V,
        16, 16,
        0xFF0000FF,
        FME_FLARE,
#endif
        SPARK_WORLD_COLLIDE | SPARK_CREATE_TRAIL | SPARK_FIELD_AFFECT | SPARK_SEMI,
        Real(0.03),     // Mass
        Real(0.012),    // Resistance
        Real(0.0),      // Friction
        Real(0.4),      // Restitution
        TO_TIME(Real(1.5)),         // Lifetime (ms)
        TO_TIME(Real(0.5)),         // Lifetime variation (ms)
        ZERO, ZERO,     // Spin rate and variation
        Real(5),        // Initial size variation
        Real(-5), ZERO,     // Grow rate and variation
        TRAIL_FIREWORK,
    },
    { // SPARK_EXPLOSION1

#ifdef _PC
        {12.0f, 12.0f,                      // XSize, YSize
        64.0f / 256.0f, 0 / 256.0f,     // U, V
        64.0f / 256.0f, 64.0f / 256.0f,     // Usize, Vsize;
        TPAGE_FX1, 0,                       // Tpage, pad;
        0xffffffff},                        // RGB;
        SPARK_SEMI | SPARK_SPINS | SPARK_GROWS,
        Real(0.03),     // Mass
        Real(0.015),    // Resistance
        Real(0.6),      // Friction
        Real(0.0),      // Restitution
        TO_TIME(Real(0.5)),         // Lifetime (ms)
        TO_TIME(Real(0)),           // Lifetime variation (ms)
        ZERO, Real(6),      // Spin rate and variation
        Real(0),        // Initial size variation
        Real(128), Real(0),     // Grow rate and variation
        TRAIL_NONE,
#endif

#ifdef _N64
        128.0f, 128.f,
        GFX_SPR_FIRE,
        0, 0,
        GFX_SPR_FIRE_W, GFX_SPR_FIRE_H,
        0xff4000FF,
        FME_FLARE,
        SPARK_SEMI | SPARK_GROWS,
        Real(0.03),     // Mass
        Real(0.015),    // Resistance
        Real(0.6),      // Friction
        Real(0.0),      // Restitution
        TO_TIME(Real(1.0)),         // Lifetime (ms)
        TO_TIME(Real(0)),           // Lifetime variation (ms)
        ZERO,ZERO,      // Spin rate and variation
        Real(0),        // Initial size variation
        Real(1024), Real(0),        // Grow rate and variation
        TRAIL_NONE,
#endif
    },
    { // SPARK_EXPLOSION2
#ifdef _PC
        {8.0f, 8.0f,                        // XSize, YSize
        64.0f / 256.0f, 0 / 256.0f,     // U, V
        64.0f / 256.0f, 64.0f / 256.0f,     // Usize, Vsize;
        TPAGE_FX1, 0,                       // Tpage, pad;
        0xffffffff},                        // RGB;
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
        Real(0.03),     // Mass
        Real(0.015),    // Resistance
        Real(0.6),      // Friction
        Real(0.0),      // Restitution
        TO_TIME(Real(0.5)),     // Lifetime (ms)
        TO_TIME(Real(0.3)),         // Lifetime variation (ms)
        ZERO, Real(32),     // Spin rate and variation
        Real(0),        // Initial size variation
        Real(12), Real(5),      // Grow rate and variation
        TRAIL_NONE,
    },
    { // SPARK_STAR
#ifdef _PC
        {8.0f, 8.0f,    // XSize, YSize
        241.0f / 256.0f, 32 / 256.0f,       // U, V
        15.0f / 256.0f, 16.0f / 256.0f,     // Usize, Vsize;
        TPAGE_FX1, 0,   // Tpage, pad;
        0xffffff},      // RGB;
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
        Real(0.0),      // Mass
        Real(0.0),      // Resistance
        Real(0.0),      // Friction
        Real(0.0),      // Restitution
        TO_TIME(Real(0.5)),         // Lifetime (ms)
        TO_TIME(Real(0)),               // Lifetime variation (ms)
        Real(0.0), Real(0.0),   // Spin rate and variation
        Real(0.0),              // Initial size variation
        Real(30.0), Real(0.0),  // Grow rate and variation
        TRAIL_NONE,
    },
    { // SPARK_PROBE_SMOKE
#ifdef _PC
        {700.0f, 150.0f,    // XSize, YSize
        0.0f / 256.0f, 186.0f / 256.0f,     // U, V
        256.0f / 256.0f, 64.0f / 256.0f,        // Usize, Vsize;
        TPAGE_WORLD_START + 1, 0,   // Tpage, pad;
        0xffffffff},        // RGB;
#endif
#ifdef _N64
        4.0f, 4.0f,
        GFX_SPR_PARTS_2,
        GFX_S_PT2_YSPRK_U, GFX_S_PT2_YSPRK_V,
        16, 16,
        0x00ffffff,
        FME_FLARE | FME_NOZB,
#endif
        SPARK_SEMI | SPARK_GROWS | SPARK_FLAT,
        Real(0.0),      // Mass
        Real(0.0),      // Resistance
        Real(0.0),      // Friction
        Real(0.0),      // Restitution
        TO_TIME(Real(0.5)),         // Lifetime (ms)
        TO_TIME(Real(0)),               // Lifetime variation (ms)
        Real(0.0), Real(0.0),   // Spin rate and variation
        Real(0.0),              // Initial size variation
        Real(100.0), Real(0.0), // Grow rate and variation
        TRAIL_NONE,
    },
    { // SPARK_SPRINKLER
#ifdef _PC
        {3.0f, 3.0f,            // XSize, YSize
        192.0f / 256.0f, 16 / 256.0f,       // U, V
        16.0f / 256.0f, 16.0f / 256.0f,         // Usize, Vsize;
//      208.0f / 256.0f, 49.0f / 256.0f,        // U, V
//      14.0f / 256.0f, 14.0f / 256.0f,         // Usize, Vsize;
        TPAGE_FX1, 0,   // Tpage, pad;
        0xff202028},        // RGB;
        SPARK_FIELD_AFFECT | SPARK_SEMI | SPARK_GROWS,
#endif
#ifdef _N64
        10.0f, 10.0f,
        GFX_SPR_PARTS_1,
        GFX_S_PT1_WATER_U, GFX_S_PT1_WATER_V,
        16, 16,
        0x808080FF,
        0,
        SPARK_FIELD_AFFECT | SPARK_SEMI | SPARK_GROWS,
#endif
        Real(0.03),     // Mass
        Real(0.01), // Resistance
        Real(0.2),      // Friction
        Real(0.2),      // Restitution
        TO_TIME(Real(0.9)),         // Lifetime (ms)
        TO_TIME(Real(0.0)),         // Lifetime variation (ms)
        ZERO, ZERO,     // Spin rate and variation
        ZERO,           // Initial size variation
        Real(8), Real(8),       // Grow rate and variation
        TRAIL_NONE,
    },
    { // SPARK_SPRINKLER_BIG
#ifdef _PC
        {3.0f, 3.0f,            // XSize, YSize
        192.0f / 256.0f, 16 / 256.0f,       // U, V
        16.0f / 256.0f, 16.0f / 256.0f,         // Usize, Vsize;
//      208.0f / 256.0f, 49.0f / 256.0f,        // U, V
//      14.0f / 256.0f, 14.0f / 256.0f,         // Usize, Vsize;
        TPAGE_FX1, 0,   // Tpage, pad;
        0xff08080a},        // RGB;
        SPARK_FIELD_AFFECT | SPARK_SEMI | SPARK_GROWS | SPARK_SPINS,
#endif
#ifdef _N64
        10.0f, 10.0f,
        GFX_SPR_PARTS_1,
        GFX_S_PT1_WATER_U, GFX_S_PT1_WATER_V,
        16, 16,
        0x808080FF,
        0,
        SPARK_FIELD_AFFECT | SPARK_SEMI | SPARK_GROWS | SPARK_SPINS,
#endif
        Real(0.03),     // Mass
        Real(0.01), // Resistance
        Real(0.2),      // Friction
        Real(0.2),      // Restitution
        TO_TIME(Real(1.0)),         // Lifetime (ms)
        TO_TIME(Real(0.0)),         // Lifetime variation (ms)
        Real(0), Real(6),       // Spin rate and variation
        ZERO,           // Initial size variation
        Real(128), Real(64),        // Grow rate and variation
        TRAIL_NONE,
    },
    { // SPARK_DOLPHIN
#ifdef _PC
        {3.0f, 3.0f,            // XSize, YSize
        192.0f / 256.0f, 16 / 256.0f,       // U, V
        16.0f / 256.0f, 16.0f / 256.0f,         // Usize, Vsize;
//      208.0f / 256.0f, 49.0f / 256.0f,        // U, V
//      14.0f / 256.0f, 14.0f / 256.0f,         // Usize, Vsize;
        TPAGE_FX1, 0,   // Tpage, pad;
        0xff404050},        // RGB;
        SPARK_FIELD_AFFECT | SPARK_SEMI | SPARK_GROWS,
#endif
#ifdef _N64
        10.0f, 10.0f,
        GFX_SPR_PARTS_1,
        GFX_S_PT1_WATER_U, GFX_S_PT1_WATER_V,
        16, 16,
        0x808080FF,
        0,
        SPARK_FIELD_AFFECT | SPARK_SEMI | SPARK_GROWS,
#endif
        Real(0.03),     // Mass
        Real(0.005),    // Resistance
        Real(0.2),      // Friction
        Real(0.2),      // Restitution
        TO_TIME(Real(1.3)),         // Lifetime (ms)
        TO_TIME(Real(0.0)),         // Lifetime variation (ms)
        ZERO, ZERO,     // Spin rate and variation
        ZERO,           // Initial size variation
        Real(8), Real(8),       // Grow rate and variation
        TRAIL_NONE,
    },
    { // SPARK_DOLPHIN_BIG
#ifdef _PC
        {3.0f, 3.0f,            // XSize, YSize
        192.0f / 256.0f, 16 / 256.0f,       // U, V
        16.0f / 256.0f, 16.0f / 256.0f,         // Usize, Vsize;
//      208.0f / 256.0f, 49.0f / 256.0f,        // U, V
//      14.0f / 256.0f, 14.0f / 256.0f,         // Usize, Vsize;
        TPAGE_FX1, 0,   // Tpage, pad;
        0xff101018},        // RGB;
        SPARK_FIELD_AFFECT | SPARK_SEMI | SPARK_GROWS | SPARK_SPINS,
#endif
#ifdef _N64
        10.0f, 10.0f,
        GFX_SPR_PARTS_1,
        GFX_S_PT1_WATER_U, GFX_S_PT1_WATER_V,
        16, 16,
        0x808080FF,
        0,
        SPARK_FIELD_AFFECT | SPARK_SEMI | SPARK_GROWS | SPARK_SPINS,
#endif
        Real(0.03),     // Mass
        Real(0.005),    // Resistance
        Real(0.2),      // Friction
        Real(0.2),      // Restitution
        TO_TIME(Real(1.3)),         // Lifetime (ms)
        TO_TIME(Real(0.0)),         // Lifetime variation (ms)
        Real(0), Real(6),       // Spin rate and variation
        ZERO,           // Initial size variation
        Real(96), Real(48),     // Grow rate and variation
        TRAIL_NONE,
    },
    { // SPARK_SPARK3
#ifdef _PC
        {3.0f, 3.0f,            // XSize, YSize
        240.0f / 256.0f, 0 / 256.0f,        // U, V
        16 / 256.0f, 16 / 256.0f,           // Usize, Vsize;
        TPAGE_FX1, 0,   // Tpage, pad;
        0xffffffff},        // RGB;
        SPARK_WORLD_COLLIDE | SPARK_FIELD_AFFECT | SPARK_CAMERA_COLLIDE | SPARK_SEMI | SPARK_OBJECT_COLLIDE,
#endif
#ifdef _N64
        // Not sorted yet...JCC
        8.0f, 8.0f,
        GFX_SPR_PARTS_2,
        GFX_S_PT2_YSPRK_U, GFX_S_PT2_YSPRK_V,
        16, 16,
        0xFFFF00FF,
        FME_FLARE,
        SPARK_FIELD_AFFECT | SPARK_SEMI | SPARK_WORLD_COLLIDE,
#endif
        Real(0.1),      // Mass
        Real(0.002),    // Resistance
        Real(0.0),      // Friction
        Real(0.8),      // Restitution
        TO_TIME(Real(4.0)),         // Lifetime (ms)
        TO_TIME(Real(0.05)),                // Lifetime variation (ms)
        ZERO, ZERO,     // Spin rate and variation
        ZERO,           // Initial size variation
        ZERO, ZERO,     // Grow rate and variation
        TRAIL_SPARK,    // trail type
    },

    { // SPARK_ROADDUST
#ifdef _PC
        {6.0f, 6.0f,    // XSize, YSize
        0.0f, 0.0f,     // U, V
        64.0f / 256.0f, 64.0f / 256.0f, // Usize, Vsize;
        TPAGE_FX1, 0,   // Tpage, pad;
        0x404040},  // RGB;
        SPARK_OBJECT_COLLIDE | SPARK_WORLD_COLLIDE | SPARK_SPINS | SPARK_GROWS | SPARK_SEMI,
#endif
#ifdef _N64
        24.0f, 24.0f,
        GFX_SPR_SMOKE,
        0, 0,
        GFX_SPR_SMOKE_W, GFX_SPR_SMOKE_H,
        0xa0a0a080,
        0,
        SPARK_SPINS | SPARK_GROWS | SPARK_SEMI,
#endif
        Real(0.03),     // Mass
        Real(0.002),        // Resistance
        Real(0.0),      // Friction
        Real(0.0),      // Restitution
        TO_TIME(Real(0.5)),         // Lifetime (ms)
        TO_TIME(Real(0.1)),         // Lifetime variation (ms)
        Real(0.0), Real(6.0),   // Spin rate and variation
        Real(2.0),      // Initial size variation
        Real(0.0), Real(36.0),      // Grow rate and variation
        TRAIL_NONE,
    },
    { // SPARK_GRASSDUST
#ifdef _PC
        {6.0f, 6.0f,    // XSize, YSize
        0.0f, 0.0f,     // U, V
        64.0f / 256.0f, 64.0f / 256.0f, // Usize, Vsize;
        TPAGE_FX1, 0,   // Tpage, pad;
        0x141b10},  // RGB;
        SPARK_OBJECT_COLLIDE | SPARK_WORLD_COLLIDE | SPARK_SPINS | SPARK_GROWS | SPARK_SEMI,
#endif
#ifdef _N64
        24.0f, 24.0f,
        GFX_SPR_SMOKE,
        0, 0,
        GFX_SPR_SMOKE_W, GFX_SPR_SMOKE_H,
        0x2c200080,
        0,
        SPARK_SPINS | SPARK_GROWS | SPARK_SEMI,
#endif
        Real(0.03),     // Mass
        Real(0.002),        // Resistance
        Real(0.0),      // Friction
        Real(0.0),      // Restitution
        TO_TIME(Real(0.5)),         // Lifetime (ms)
        TO_TIME(Real(0.1)),         // Lifetime variation (ms)
        Real(0.0), Real(6.0),   // Spin rate and variation
        Real(2.0),      // Initial size variation
        Real(0.0), Real(36.0),      // Grow rate and variation
        TRAIL_NONE,
    },
    { // SPARK_SOILDUST
#ifdef _PC
        {6.0f, 6.0f,    // XSize, YSize
        0.0f, 0.0f,     // U, V
        64.0f / 256.0f, 64.0f / 256.0f, // Usize, Vsize;
        TPAGE_FX1, 0,   // Tpage, pad;
//      0x080400},  // RGB;
        0x1c1910},  // RGB;
        SPARK_OBJECT_COLLIDE | SPARK_WORLD_COLLIDE | SPARK_SPINS | SPARK_GROWS | SPARK_SEMI,
#endif
#ifdef _N64
        24.0f, 24.0f,
        GFX_SPR_SMOKE,
        0, 0,
        GFX_SPR_SMOKE_W, GFX_SPR_SMOKE_H,
        0x20100080,
        0,
        SPARK_SPINS | SPARK_GROWS | SPARK_SEMI,
#endif
        Real(0.03),             // Mass
        Real(0.002),            // Resistance
        Real(0.0),              // Friction
        Real(0.0),              // Restitution
        TO_TIME(Real(0.5)),     // Lifetime (ms)
        TO_TIME(Real(0.1)),     // Lifetime variation (ms)
        Real(0.0), Real(6.0),   // Spin rate and variation
        Real(2.0),              // Initial size variation
        Real(120.0), Real(20.0),    // Grow rate and variation
        TRAIL_NONE,
    },
    { // SPARK_GRAVELDUST
#ifdef _PC
        {6.0f, 6.0f,    // XSize, YSize
        0.0f, 0.0f,     // U, V
        64.0f / 256.0f, 64.0f / 256.0f, // Usize, Vsize;
        TPAGE_FX1, 0,   // Tpage, pad;
        0x404040},  // RGB;
        SPARK_OBJECT_COLLIDE | SPARK_WORLD_COLLIDE | SPARK_SPINS | SPARK_GROWS | SPARK_SEMI,
#endif
#ifdef _N64
        24.0f, 24.0f,
        GFX_SPR_SMOKE,
        0, 0,
        GFX_SPR_SMOKE_W, GFX_SPR_SMOKE_H,
        0xa0a0a080,
        0,
        SPARK_SPINS | SPARK_GROWS | SPARK_SEMI,
#endif
        Real(0.03),     // Mass
        Real(0.002),        // Resistance
        Real(0.0),      // Friction
        Real(0.0),      // Restitution
        TO_TIME(Real(0.5)),         // Lifetime (ms)
        TO_TIME(Real(0.1)),         // Lifetime variation (ms)
        Real(0.0), Real(6.0),   // Spin rate and variation
        Real(2.0),      // Initial size variation
        Real(0.0), Real(36.0),      // Grow rate and variation
        TRAIL_NONE,
    },
    { // SPARK_SANDDUST
#ifdef _PC
        {6.0f, 6.0f,    // XSize, YSize
        0.0f, 0.0f,     // U, V
        64.0f / 256.0f, 64.0f / 256.0f, // Usize, Vsize;
        TPAGE_FX1, 0,   // Tpage, pad;
        0x100800},  // RGB;
        SPARK_OBJECT_COLLIDE | SPARK_WORLD_COLLIDE | SPARK_SPINS | SPARK_GROWS | SPARK_SEMI,
#endif
#ifdef _N64
        24.0f, 24.0f,
        GFX_SPR_SMOKE,
        0, 0,
        GFX_SPR_SMOKE_W, GFX_SPR_SMOKE_H,
        0x20100080,
        0,
        SPARK_SPINS | SPARK_GROWS | SPARK_SEMI,
#endif
        Real(0.03),             // Mass
        Real(0.002),            // Resistance
        Real(0.0),              // Friction
        Real(0.0),              // Restitution
        TO_TIME(Real(0.5)),     // Lifetime (ms)
        TO_TIME(Real(0.1)),     // Lifetime variation (ms)
        Real(0.0), Real(6.0),   // Spin rate and variation
        Real(2.0),              // Initial size variation
        Real(60.0), Real(20.0), // Grow rate and variation
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
        #ifdef _PC
        225.0f / 256.0f, 33.0f / 256.0f,    // U, V
        14.0f / 256.0f, 14.0f / 256.0f,     // Usize, Vsize
        225.0f / 256.0f, 33.0f / 256.0f,    // U, V
        0xff, 0xff, 0xff, 0xff,             // ARGB
        TRAIL_FADES | TRAIL_SHRINKS,
        Real(0.3),                          // Lifetime
        Real(5),                            // Width
        TRAIL_MAX_BITS,                     // number of sections
        #elif defined(_N64)     
        0,0,GFX_SPR_LASER_W,GFX_SPR_LASER_H,GFX_SPR_LASERB,GFX_SPR_LASERB,
        // 0,0,GFX_SPR_SMOKE_W, GFX_SPR_SMOKE_H, GFX_SPR_SMOKE, GFX_SPR_SMOKE, // U, V, W, H, Gfx, GfxEnd
        0xff, 0xff, 0xff, 0xff,             // ARGB
        TRAIL_FADES | TRAIL_SHRINKS,
        Real(0.3),                          // Lifetime
        Real(8),                            // Width
        TRAIL_MAX_BITS,                     // number of sections
        #endif

    },
    { // TRAIL_SPARK
        #ifdef _PC
        246.0f / 256.0f, 0.0f / 256.0f, // U, V
        2.0f / 256.0f, 16.0f / 256.0f,      // Usize, Vsize
        246.0f / 256.0f, 0.0f / 256.0f, // U, V
        #elif defined(_N64)
        0,0,GFX_SPR_SMOKE_W, GFX_SPR_SMOKE_H, GFX_SPR_SMOKE, GFX_SPR_SMOKE, // U, V, W, H, Gfx, GfxEnd
        #endif

        0xff, 0xff, 0xff, 0xff,             // ARGB
        TRAIL_FADES | TRAIL_SHRINKS,
        Real(0.03),                         // Lifetime
        Real(1),                            // Width
        2,                                  // number of sections
    },
    { // TRAIL_SMOKE
        #ifdef _PC
        224.0f / 256.0f, 48.0f / 256.0f,    // U, V
        16.0f / 256.0f, 16.0f / 256.0f,     // Usize, Vsize
        240.0f / 256.0f, 48.0f / 256.0f,    // U, V
        0xff, 0x88, 0x88, 0xaa,             // ARGB
        #elif defined(_N64)
        //0,0,GFX_SPR_LASER_W,GFX_SPR_LASER_H,GFX_SPR_LASERB,GFX_SPR_LASERB,
        0, 0, 16,16,GFX_SPR_STRAIL1, GFX_SPR_STRAIL1, // U, V, W, H, Gfx, GfxEnd
        0xff, 0xd8, 0xd8, 0xfa,             // ARGB
        #endif

        TRAIL_FADES | TRAIL_EXPANDS,
        Real(0.4),                          // Lifetime
        Real(10),                           // Width
        TRAIL_MAX_BITS,                     // number of sections
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
    bool collided;

    for (iSpark = 0; iSpark < MAX_SPARKS; iSpark++) {
        spark = &Sparks[iSpark];

        // See if this spark is active
        if (spark->Free) continue;

#ifndef _PSX
        // Check against visimask
        if (CamVisiMask & spark->VisiMask) continue;
#endif

        // See if it is time to kill the spark
        spark->Age += TimeStep;
        if (spark->Age > spark->Data->LifeTime) {
            FreeSpark(spark);
            continue;
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
            fieldData.Priority = 0;

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
        collided = FALSE;
#ifdef _PC
        if (spark->Data->SparkType & SPARK_OBJECT_COLLIDE) {
            collided = collided || SparkObjectCollide(spark);
        }
#endif

#ifdef _PC
        if (spark->Data->SparkType & SPARK_WORLD_COLLIDE) 
#else
        if ((spark->Data->SparkType & SPARK_WORLD_COLLIDE) && (GameSettings.GameType == GAMETYPE_FRONTEND))
#endif  
        {
            collided = collided || SparkWorldCollide(spark, &oldPos);
        }

        if (!collided) {
            // Rotate the spark
            if (spark->Data->SparkType & SPARK_SPINS) {
                spark->Spin += spark->SpinRate * TimeStep;
            }
 
            // Expand the spark
            if (spark->Data->SparkType & SPARK_GROWS) {
                spark->Grow += spark->GrowRate * TimeStep;
                //if (spark->Grow < ZERO) spark->Grow = ZERO;
            }
        }

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

bool SparkWorldCollide(SPARK *spark, VEC *oldPos)
{
    int iPoly;
    REAL time, depth, velDotNorm;
    VEC dPos, wPos;
    BBOX bBox;
    COLLGRID *grid;
    NEWCOLLPOLY *collPoly;
    bool collided = FALSE;

    grid = PosToCollGrid(&spark->Pos);
    if (grid == NULL) return collided;

    for (iPoly = 0; iPoly < grid->NCollPolys; iPoly++)
    {
        collPoly = GetCollPoly(grid->CollPolyIndices[iPoly]);

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

        collided = TRUE;
    }

    return collided;
}



/////////////////////////////////////////////////////////////////////
//
// SparkObjectCollide: check for collisions between sparks and objects
//
/////////////////////////////////////////////////////////////////////

bool SparkObjectCollide(SPARK *spark)
{
    OBJECT  *obj;
    bool collided = FALSE;

    for (obj = OBJ_ObjectHead; obj != NULL; obj = obj->next) {

        // See if this object allows collisions
        if (obj->CollType == COLL_TYPE_NONE) continue;
        if (obj->body.CollSkin.AllowObjColls == FALSE) continue;

        // Quick bounding box test
        if(!PointInBBox(&spark->Pos, &obj->body.CollSkin.BBox)) continue;

        if (IsBodyConvex(&obj->body)) {
            if (SparkConvexCollide(spark, &obj->body)) {
                collided = TRUE;
            }
        } else if (IsBodySphere(&obj->body)) {
            if (SparkSphereCollide(spark, &obj->body)){
                collided = TRUE;
            }
        }
    }

    return collided;
}

bool SparkConvexCollide(SPARK *spark, NEWBODY *body)
{
    int iSkin;
    REAL depth, velDotNorm;
    VEC dVel;
    PLANE plane;
    bool collided = FALSE;

    for (iSkin = 0; iSkin < body->CollSkin.NConvex; iSkin++) {
    
        if (!PointInConvex(&spark->Pos, &body->CollSkin.WorldConvex[iSkin], &plane, &depth)) continue;

        collided = TRUE;

        // move spark out of object and rebound
        VecPlusEqScalarVec(&spark->Pos, -depth + COLL_EPSILON, PlaneNormal(&plane));
        velDotNorm = VecDotVec(&spark->Vel, PlaneNormal(&plane));
        VecPlusEqScalarVec(&spark->Vel, -(ONE + spark->Data->Restitution) * velDotNorm, PlaneNormal(&plane));

        // Add simple friction
        VecMinusVec(&body->Centre.Vel, &spark->Vel, &dVel);
        VecPlusEqScalarVec(&spark->Vel, spark->Data->Friction, &dVel);
    }

    return collided;
}

bool SparkSphereCollide(SPARK *spark, NEWBODY *body) 
{
    return FALSE;
}



/////////////////////////////////////////////////////////////////////
//
// CreateSpark: Create a spark travelling along the look direction
// of the passed matrix, with a random variation (0-1).
//
/////////////////////////////////////////////////////////////////////

SPARK *CreateSpark(SPARK_TYPE type, VEC *pos, VEC *vel, REAL velVar, VISIMASK mask)
{

    
#ifndef _PSX    

    VEC dV;
    SPARK *spark;


    Assert(type < SPARK_NTYPES);

    // Get the next available spark
    spark = GetFreeSpark();

    // make sure there were some left
    if (spark == NULL) return NULL;

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
    spark->Age = frand(SparkData[type].LifeTimeVar);

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
    return spark;
#else
    return NULL;
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

    prob = gSparkDensity * (vel - MIN_SPARK_VEL) / (MAX_SPARK_VEL - MIN_SPARK_VEL);

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
//$REMOVED    poly.DrawFlag = D3DDP_DONOTUPDATEEXTENTS;
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
    


    ZWRITE_OFF();
    FOG_OFF();
    BLEND_ON();
    BLEND_SRC(D3DBLEND_ONE);
    BLEND_DEST(D3DBLEND_ONE);

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
    vert[2].sx = ePos.sx + dx * width * ePos.rhw * RenderSettings.MatScaleX;
    vert[2].sy = ePos.sy + dy * width * ePos.rhw * RenderSettings.MatScaleY;
    vert[2].sz = ePos.sz;
    vert[2].rhw = ePos.rhw;
    vert[3].sx = ePos.sx - dx * width * ePos.rhw * RenderSettings.MatScaleX;
    vert[3].sy = ePos.sy - dy * width * ePos.rhw * RenderSettings.MatScaleY;
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
        vert[0].sx = sPos.sx - dx * width * sPos.rhw * RenderSettings.MatScaleX;
        vert[0].sy = sPos.sy - dy * width * sPos.rhw * RenderSettings.MatScaleY;
        vert[0].sz = sPos.sz;
        vert[0].rhw = sPos.rhw;
        vert[1].sx = sPos.sx + dx * width * sPos.rhw * RenderSettings.MatScaleX;
        vert[1].sy = sPos.sy + dy * width * sPos.rhw * RenderSettings.MatScaleY;
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
        DRAW_PRIM(D3DPT_TRIANGLEFAN, FVF_TEX1, poly.Verts, poly.VertNum, poly.DrawFlag);

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
    MAT mat;
    FACING_POLY poly, *sparkPoly;
    SPARK *spark;

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
                //semi = 0;
                semi = -1;
                per = (long)((100.0f * (spark->Data->LifeTime - spark->Age)) / spark->Data->LifeTime);
                ModelChangeGouraud((MODEL_RGB*)&poly.RGB, per);
                BLEND_ON();
                BLEND_SRC(D3DBLEND_ONE);
                BLEND_DEST(D3DBLEND_ONE);
            } else {
                long alpha;
                semi = -1;
                per = (long)((255.0f * (spark->Data->LifeTime - spark->Age)) / spark->Data->LifeTime);
                alpha = per << 24;
                poly.RGB = (poly.RGB & 0xffffff) | alpha;
                BLEND_ALPHA();
                BLEND_SRC(D3DBLEND_SRCALPHA);
                BLEND_DEST(D3DBLEND_INVSRCALPHA);
            }

            // Rotate the spark if necessary and then draw
            if (spark->Data->SparkType & SPARK_SPINS) {
                RotationZ(&mat, spark->Spin);
                if (spark->Data->SparkType & SPARK_FLAT) {
                    DrawHorizontalPoly(&Sparks[iSpark].Pos, /*&mat, */&poly, semi, 0);
                } else {
                    DrawFacingPolyRot(&Sparks[iSpark].Pos, &mat, &poly, semi, 0);
                }
            } else {
                if (spark->Data->SparkType & SPARK_FLAT) {
                    DrawHorizontalPoly(&Sparks[iSpark].Pos, &poly, semi, 0);
                } else {
                    DrawFacingPoly(&Sparks[iSpark].Pos, &poly, semi, 0);
                }
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
    REAL mod;

    // loop thru all sparks
    for (iSpark = 0; iSpark < MAX_SPARKS; iSpark++)
    {
        if (!Sparks[iSpark].Free)
        {
            mod = (255.0f * Sparks[iSpark].Age) / (Sparks[iSpark].Data->LifeTime);
            alpha =  (Sparks[iSpark].Data->Colour & 0xFF) - (long)mod;
            if (alpha < 0) { alpha = 0; }
            FME_AddFaceMe(Sparks[iSpark].Data->GfxIdx, Sparks[iSpark].Data->u, Sparks[iSpark].Data->v, Sparks[iSpark].Data->w, Sparks[iSpark].Data->h,
                          &Sparks[iSpark].Pos, Sparks[iSpark].Data->XSize + Sparks[iSpark].Grow, Sparks[iSpark].Data->YSize + Sparks[iSpark].Grow, Sparks[iSpark].Spin, (Sparks[iSpark].Data->Colour & 0xFFFFFF00) | (alpha & 0xFF), Sparks[iSpark].Data->Flag);
        }
    }
}




static Gfx  s_StartTrails[] = { gsDPPipeSync(),
                                gsDPSetCycleType(G_CYC_2CYCLE),
                                gsSPClearGeometryMode(G_LIGHTING | G_LOD | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_CULL_FRONT | G_CULL_BACK | G_CULL_BOTH),
                                gsSPSetGeometryMode(G_ZBUFFER | G_FOG | G_SHADE | G_SHADING_SMOOTH),
                                gsSPTexture(0x8000, 0x8000, 0, 0, G_ON),
                                gsDPSetTexturePersp (G_TP_NONE),
                                gsDPSetTextureFilter (G_TF_BILERP),
                                gsSPEndDisplayList(),
                            };
                    

/////////////////////////////////////////////////////////////////////
//
// DrawTrails:
//
/////////////////////////////////////////////////////////////////////
#define OLD_TRAILS
Vtx *v;
void DrawTrails(void)
{
    unsigned uNVAllocated;
    int i;  
    int iTrail;  

#ifndef OLD_TRAILS
    v = AllocVertices(4,&uNVAllocated); 
    if (!v || (uNVAllocated!=4))
        return;

    for (i=0; i<4; i++) { 
        v[i].v.ob[0] = ViewMatrixScaled.m[XZ] * 100.f ;
        v[i].v.ob[1] = ViewMatrixScaled.m[YZ] * 100.f ;
        v[i].v.ob[2] = ViewMatrixScaled.m[ZZ] * 100.f ;
        *((long*)v[i].v.cn) = 0xffffffff;
        }
    guTranslate(mlistp, ViewCameraPos.v[0],  ViewCameraPos.v[1], ViewCameraPos.v[2]);
    gSPMatrix(slistp++, OS_K0_TO_PHYSICAL(mlistp++),G_MTX_MODELVIEW|G_MTX_LOAD|G_MTX_NOPUSH);
    gSPVertex(slistp++,v,4,0);
    gDPPipeSync(slistp++);  
    gSP2Triangles(slistp++, 0,1,3,0,2,1,3,0);
#endif

    GFX_ResetRenderState();
    GG_ResetAll();
    for (iTrail = 0; iTrail < MAX_TRAILS; iTrail++) {
        if (SparkTrail[iTrail].Free) continue;

        DrawSparkTrail(&SparkTrail[iTrail]);
    }
}


/////////////////////////////////////////////////////////////////////
//
// DrawSparkTrails
//
/////////////////////////////////////////////////////////////////////
#ifdef OLD_TRAILS
void DrawSparkTrail(TRAIL *trail)
{
    int     iTrail, thisTrail, lastTrail;
    long    a, r, g, b;
    VEC     sPos, ePos;
    long    sCol, eCol;
    REAL    width;  

    if (!trail->NTrails)
        return;

    {
    sLineSeg TrailSeg   =  { trail->Data->GfxIdx
                           , trail->Data->u,trail->Data->v,trail->Data->w,trail->Data->h
                           , 0 , 0
                            };

    TrailSeg.uNBits = 1;
    TrailSeg.fWidth = trail->Data->Width;
    TrailSeg.uColor = (trail->Data->A      )
                    | (trail->Data->R << 24)
                    | (trail->Data->G << 16)
                    | (trail->Data->B <<  8);   

    thisTrail = trail->FirstTrail;
    lastTrail = (trail->FirstTrail - 1);
    if (lastTrail < 0) lastTrail = trail->MaxTrails - 1;

    CopyVec(&trail->Pos[thisTrail], &TrailSeg.sPos);

    for (iTrail = 1; iTrail < trail->NTrails; iTrail++)
        {
        TrailSeg.DPos.v[X] = trail->Pos[lastTrail].v[X] - TrailSeg.sPos.v[X];
        TrailSeg.DPos.v[Y] = trail->Pos[lastTrail].v[Y] - TrailSeg.sPos.v[Y];
        TrailSeg.DPos.v[Z] = trail->Pos[lastTrail].v[Z] - TrailSeg.sPos.v[Z];

        if (trail->Data->Type & TRAIL_FADES)
            eCol = ( (trail->Data->A - iTrail * (trail->Data->A / trail->NTrails))      )
                 | ( (trail->Data->R - iTrail * (trail->Data->R / trail->NTrails)) << 24)
                 | ( (trail->Data->G - iTrail * (trail->Data->G / trail->NTrails)) << 16)
                 | ( (trail->Data->B - iTrail * (trail->Data->B / trail->NTrails)) <<  8);
        else
            eCol = ( trail->Data->A       )
                 | ( trail->Data->R  << 24)
                 | ( trail->Data->G  << 16)
                 | ( trail->Data->B  <<  8);

        // Draw the line
        FME_AddLineSeg(&TrailSeg);

        // Reduce width for next section
        if (trail->Data->Type & TRAIL_SHRINKS)
            width = trail->Data->Width - (trail->Data->Width * iTrail) / trail->MaxTrails;
        else if (trail->Data->Type & TRAIL_EXPANDS)
            width = trail->Data->Width + (3 * trail->Data->Width * iTrail) / (trail->MaxTrails * 2);
        else
            width = trail->Data->Width;

        TrailSeg.fWidth = width;
        TrailSeg.uColor = eCol;
        TrailSeg.uFlag |= FME_FOLLOWSEG;
        CopyVec(&trail->Pos[lastTrail], &TrailSeg.sPos);

        // move on
        thisTrail = lastTrail;
        if (--lastTrail < 0) lastTrail = trail->MaxTrails - 1;
        }
    }
    }

#else

void DrawSparkTrail(TRAIL *trail)
{
    struct MyVtx 
        {
        float sx,sy,sz,rhw;
        unsigned color;
    } vert[4];

    int iTrail, thisTrail, lastTrail;
    long a, r, g, b;
    REAL dx, dy, dt, dLen, width;
    float sPos[4], ePos[4];
    bool bOut1, bOut2;
    
    float fSx = Camera[CameraCount].Xsize * 4.f;
    float fSy = Camera[CameraCount].Ysize * 4.f;
    float fOx = (Camera[CameraCount].X + Camera[CameraCount].Xsize * .5f) * 4.f;
    float fOy = (Camera[CameraCount].Y + Camera[CameraCount].Ysize * .5f) * 4.f;

    v[3].v.tc[0] = trail->Data->u << 6;
    v[3].v.tc[1] = trail->Data->v << 6;

    v[0].v.tc[0] = (trail->Data->u + trail->Data->w) << 6;
    v[0].v.tc[1] = trail->Data->v << 6;

    v[1].v.tc[0] = (trail->Data->u + trail->Data->w) << 6;
    v[1].v.tc[1] = (trail->Data->v + trail->Data->h) << 6;

    v[2].v.tc[0] = trail->Data->u << 6;
    v[2].v.tc[1] = (trail->Data->v + trail->Data->h) << 6;
    
    GG_SemiSetTexture(trail->Data->GfxIdx,TRUE);
    /*
    gDPSetCombineMode(slistp++, CC_MYMODE_FACEME, CC_MYMODE_FACEME2); 
    gDPSetRenderMode(slistp++, G_RM_PASS, G_RM_AA_ZB_CLD_SURF2); 
    */

    thisTrail = trail->FirstTrail;
    lastTrail = (trail->FirstTrail - 1);
    if (lastTrail < 0) lastTrail = trail->MaxTrails - 1;

    // Calculate first section end coordinates
    N64_RotTransPersVector(&ViewMatrixScaled, &ViewTransScaled, &trail->Pos[thisTrail], ePos);
    N64_RotTransPersVector(&ViewMatrixScaled, &ViewTransScaled, &trail->Pos[lastTrail], sPos);
    dx = ePos[0] - sPos[0];
    dy = ePos[1] - sPos[1];
    dLen = (REAL)sqrt(dx * dx + dy * dy);
    dt = dx;
    dx = dy / dLen;
    dy = -dt / dLen;

    // Set up first end vertices
    width = trail->Data->Width;
    vert[2].sx  = ePos[0] + dx * width * ePos[3] * RenderSettings.MatScaleX;
    vert[2].sy  = ePos[1] + dy * width * ePos[3] * RenderSettings.MatScaleY;
    vert[2].sz  = ePos[2];
    vert[2].rhw = ePos[3];
    vert[3].sx  = ePos[0] - dx * width * ePos[3] * RenderSettings.MatScaleX;
    vert[3].sy  = ePos[1] - dy * width * ePos[3] * RenderSettings.MatScaleY;
    vert[3].sz  = ePos[2];
    vert[3].rhw = ePos[3];

    bOut1  = vert[2].sx < -.5f || vert[3].sx < -.5f || vert[2].sx > .5f || vert[3].sx > .5f;
    bOut1 |= vert[2].sy < -.5f || vert[3].sy < -.5f || vert[2].sy > .5f || vert[3].sy > .5f;
    bOut1 |= vert[2].sz < 1.f || vert[3].sz < 1.f;
    for (iTrail = 1; iTrail < trail->NTrails; iTrail++) {

        // Get coordinates of next section
        N64_RotTransPersVector(&ViewMatrixScaled, &ViewTransScaled, &trail->Pos[lastTrail], sPos);
        dx = ePos[0] - sPos[0];
        dy = ePos[1] - sPos[1];
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
        vert[0].sx  = sPos[0] - dx * width * sPos[3] * RenderSettings.MatScaleX;
        vert[0].sy  = sPos[1] - dy * width * sPos[3] * RenderSettings.MatScaleY;
        vert[0].sz  = sPos[2];
        vert[0].rhw = sPos[3];
        vert[1].sx  = sPos[0] + dx * width * sPos[3] * RenderSettings.MatScaleX;
        vert[1].sy  = sPos[1] + dy * width * sPos[3] * RenderSettings.MatScaleY;
        vert[1].sz  = sPos[2];
        vert[1].rhw = sPos[3];

        bOut2  = vert[0].sx < -.5f || vert[1].sx < -.5f || vert[0].sx > .5f || vert[1].sx > .5f;
        bOut2 |= vert[0].sy < -.5f || vert[1].sy < -.5f || vert[0].sy > .5f || vert[1].sy > .5f;
        bOut2 |= vert[0].sz < 1.f  || vert[1].sz < 1.f;

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

        // draw the poly
        if ( !bOut2 && !bOut1)
            {
            long x1,y1,x2,y2,x3,y3,x4,y4;
            gDPSetPrimColor(slistp++, 0, 0, r, g, b, a);        
            FTOL( vert[0].sx * fSx + fOx,x1);
            FTOL( vert[1].sx * fSx + fOx,x2);
            FTOL( vert[2].sx * fSx + fOx,x3);
            FTOL( vert[3].sx * fSx + fOx,x4);
            FTOL( vert[0].sy * fSy + fOy,y1);
            FTOL( vert[1].sy * fSy + fOy,y2);
            FTOL( vert[2].sy * fSy + fOy,y3);
            FTOL( vert[3].sy * fSy + fOy,y4);
            gDPPipeSync(slistp++);  
            gSPModifyVertex(slistp++,0,G_MWO_POINT_XYSCREEN,(x1<<16) | (y1));
            gSPModifyVertex(slistp++,1,G_MWO_POINT_XYSCREEN,(x2<<16) | (y2));
            gSPModifyVertex(slistp++,2,G_MWO_POINT_XYSCREEN,(x3<<16) | (y3));
            gSPModifyVertex(slistp++,3,G_MWO_POINT_XYSCREEN,(x4<<16) | (y4));
            gSP2Triangles(slistp++, 0,1,2,0, 2,3,0,0 );
            }

        // copy last to next
        ePos[0] = sPos[0];
        ePos[1] = sPos[1];
        ePos[2] = sPos[2];
        ePos[3] = sPos[3];

        vert[2].sx  = vert[1].sx;
        vert[2].sy  = vert[1].sy;
        vert[2].sz  = vert[1].sz;
        vert[2].rhw = vert[1].rhw;
        vert[3].sx  = vert[0].sx;
        vert[3].sy  = vert[0].sy;
        vert[3].sz  = vert[0].sz;
        vert[3].rhw = vert[0].rhw;

        bOut1 = bOut2;

        // move on
        thisTrail = lastTrail;
        if (--lastTrail < 0) lastTrail = trail->MaxTrails - 1;
    }
}
#endif

#endif
