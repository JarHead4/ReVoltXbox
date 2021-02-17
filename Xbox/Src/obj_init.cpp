//-----------------------------------------------------------------------------
// File: obj_init.cpp
//
// Desc: Initialization (and destruction) functions for objects.
//       This is a companion file to ai_init.cpp that intializes the object's
//       AI variables.
//
// Re-Volt (Generic) Copyright (c) Probe Entertainment 1998
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"
#include "main.h"
#include "geom.h"
#include "particle.h"
#include "model.h"
#include "aerial.h"
#include "newcoll.h"
#include "body.h"
#include "car.h"
#include "ctrlread.h"
#include "object.h"
#include "light.h"
#include "obj_init.h"
#include "player.h"
#include "ai.h"
#include "ai_init.h"
#include "EditObject.h"
#include "drawobj.h"
#include "move.h"
#include "timing.h"
#include "visibox.h"
#include "spark.h"
#include "field.h"
#include "weapon.h"
#ifdef _PC
#include "ui_TitleScreen.h"
#include "ui_MenuDraw.h"
#include "input.h"
#endif
#ifdef _N64
#include "ui_TitleScreen.h"
#include "ffs_code.h"
#include "ffs_list.h"
#include "utils.h"
#include "gamegfx.h"
#endif
#include "initplay.h"
#include "pickup.h"

#include "SoundEffectEngine.h"

void TrolleyAIHandler(OBJECT *obj);

// globals

#ifdef _N64

extern MODEL    *g_BoxFaceModels[64];
extern u32      g_uNBoxFaceModels;

#endif


long TotalStarNum, StarModelNum;

#ifdef _PC

static char *DragonMorphFrames[] = {
//$MODIFIED
//    "models\\dragon2.m",
//    "models\\dragon3.m",
    "D:\\models\\dragon2.m",
    "D:\\models\\dragon3.m",
//$END_MODIFICATIONS
};

static VEC DragonFireOffset = {-494.2f, 208.0f, 96.0f};
static VEC DragonFireDir = {-0.45f, 2.0f, 6.0f};
static VEC SprinklerHeadOffset = {0.0f, -38.0f, 0.0f};

#endif

STROBE_TABLE StrobeTable[] = {
    5, 10, 1024, {192, 192, 0}, {0, 0, 0},      // muse post
    5, 10, 1024, {0, 0, 128}, {0, 12, 0},       // muse wall
    5, 10, 1024, {0, 64, 128}, {0, 12, 0},      // hood tunnel
};

// object 3d sound list
#ifdef OLD_AUDIO
#ifndef _PSX
static long Sound3D[] = {
    SFX_HOOD_BIRDS1,
    SFX_HOOD_DOGBARK,
    SFX_HOOD_KIDS,
    SFX_HOOD_TV,
    SFX_HOOD_LAWNMOWER,
    SFX_HOOD_DIGGER,
    SFX_HOOD_BIRDS2,
    SFX_HOOD_BIRDS3,
    SFX_TOY_ARCADE,
    SFX_HOOD_STREAM,
    SFX_GARDEN_TROPICS2,
    SFX_GARDEN_TROPICS3,
    SFX_GARDEN_TROPICS4,
    SFX_MUSE_AMB,
    SFX_MARKET_AIRCOND,
    SFX_MARKET_CABINET,
    SFX_MARKET_CARPARK,
    SFX_MARKET_FREEZER,
    SFX_MARKET_ICY,
    SFX_MUSE_ESCALATOR,
    SFX_MUSE_BARREL,
    SFX_MUSE_DOOR,
    SFX_GARDEN_STREAM,
    SFX_GHOST_COYOTE,
    SFX_GHOST_BATS,
    SFX_GHOST_EAGLE,
    SFX_GHOST_DRIP,
    SFX_GHOST_RATTLER,
    SFX_SHIP_INTAMB,
    SFX_SHIP_SEAGULLS,
    SFX_SHIP_FOGHORN,
    SFX_SHIP_THUNDER,
    SFX_SHIP_STORM,
    SFX_SHIP_CALM,
#ifdef _N64
    SFX_GARDEN_ANIMAL2, // Animal 1 crashes the Audio... too long?
#else
    SFX_GARDEN_ANIMAL1,
#endif
    SFX_GARDEN_ANIMAL2,
    SFX_GARDEN_ANIMAL3,
    SFX_GARDEN_ANIMAL4,
    SFX_HOOD_AMB,
#ifndef _N64
    SFX_GHOST_BELL,
#endif
};
#endif
#else // !OLD_AUDIO
static long Sound3D[] = {
    EFFECT_hood_birds1,
    EFFECT_hood_dogbark,
    EFFECT_hood_kids,
    EFFECT_hood_tv,
    EFFECT_hood_lawnmower,
    EFFECT_hood_digger,
    EFFECT_hood_birds2,
    EFFECT_hood_birds3,
    EFFECT_toy_arcade,
    EFFECT_hood_hoodstream,
    EFFECT_garden_tropics1,
    EFFECT_garden_tropics2,
    EFFECT_garden_tropics3,
    EFFECT_museum_museumam,
    EFFECT_market_aircond1,
    EFFECT_market_cabnhum2,
    EFFECT_market_carpark,
    EFFECT_market_freezer1,
    EFFECT_market_iceyarea,
    EFFECT_museum_escalate,
    EFFECT_museum_rotating, // hope this corresponds to barrel
    EFFECT_museum_largdoor,
    EFFECT_garden_stream,
    EFFECT_ghost_coyote1,
    EFFECT_ghost_bats,
    EFFECT_ghost_eagle1,
    EFFECT_ghost_minedrip,
    EFFECT_ghost_rattler,
    EFFECT_ship_intamb1,
    EFFECT_ship_seagulls,
    EFFECT_ship_shiphorn,
    EFFECT_ship_thunder1,
    EFFECT_ship_strmrain,
    EFFECT_ship_seagulls,   // nothing to correspond to calm
    EFFECT_garden_animal1,
    EFFECT_garden_animal2,
    EFFECT_garden_animal3,
    EFFECT_garden_animal4,
    EFFECT_hood_cityamb2,
    EFFECT_ghost_townbell,
};
#endif // OLD_AUDIO



///////////////////////////
// object init functions //
///////////////////////////

static long InitBarrel(OBJECT *obj, long *flags);
static long InitFootball(OBJECT *obj, long *flags);
static long InitBeachball(OBJECT *obj, long *flags);
static long InitPlanet(OBJECT *obj, long *flags);
static long InitPlane(OBJECT *obj, long *flags);
static long InitCopter(OBJECT *obj, long *flags);
static long InitDragon(OBJECT *obj, long *flags);
static long InitWater(OBJECT *obj, long *flags);
static long InitTrolley(OBJECT *obj, long *flags);
static void FreeTrolley(OBJECT *obj);
static long InitBoat(OBJECT *obj, long *flags);
static long InitSpeedup(OBJECT *obj, long *flags);
static long InitRadar(OBJECT *obj, long *flags);
static long InitBalloon(OBJECT *obj, long *flags);
static long InitHorse(OBJECT *obj, long *flags);
static long InitTrain(OBJECT *obj, long *flags);
static long InitStrobe(OBJECT *obj, long *flags);
static long InitSparkGen(OBJECT *obj, long *flags);
static long InitSpaceman(OBJECT *obj, long *flags);
long InitPickup(OBJECT *obj, long *flags);
static long InitDissolveModel(OBJECT *obj, long *flags);
static long InitFlap(OBJECT *obj, long *flags);
static long InitLaser(OBJECT *obj, long *flags);
static long InitSplash(OBJECT *obj, long *flags);
static long InitWeebel(OBJECT *obj, long *flags);
static long InitProbeLogo(OBJECT *obj, long *flags);
static long InitClouds(OBJECT *obj, long *flags);
//$MOVED(mwetzel)static long InitNameWheel(OBJECT *obj, long *flags);
//$MOVED(mwetzel)static void UpdateNameWheel(OBJECT *obj);
//$MOVED(mwetzel)static void FreeNameWheel(OBJECT *obj);
static long InitSprinkler(OBJECT *obj, long *flags);
static long InitSprinklerHose(OBJECT *obj, long *flags);
static long InitObjectThrower(OBJECT *obj, long *flags);
static long InitBasketBall(OBJECT *obj, long *flags);
//$MOVED(mwetzel)static long InitTrackScreen(OBJECT *obj, long *flags);
static long InitClock(OBJECT *obj, long *flags);
//$MOVED(mwetzel)static long InitCarBox(OBJECT *obj, long *flags);
static long InitStream(OBJECT *obj, long *flags);
static long InitCup(OBJECT *obj, long *flags);
static long Init3dSound(OBJECT *obj, long *flags);
static long InitStar(OBJECT *obj, long *flags);
static long InitFox(OBJECT *obj, long *flags);
static long InitTumbleweed(OBJECT *obj, long *flags);
//$MOVED(mwetzel)static long InitSmallScreen(OBJECT *obj, long *flags);
static long InitLantern(OBJECT *obj, long *flags);
static long InitSkybox(OBJECT *obj, long *flags);
static long InitSlider(OBJECT *obj, long *flags);
static long InitBottle(OBJECT *obj, long *flags);
static long InitBucket(OBJECT *obj, long *flags);
static long InitCone(OBJECT *obj, long *flags);
static long InitCan(OBJECT *obj, long *flags);
static long InitLilo(OBJECT *obj, long *flags);
static long InitGlobal(OBJECT *obj, long *flags);
static long InitRain(OBJECT *obj, long *flags);
static long InitLightning(OBJECT *obj, long *flags);
static long InitShipLight(OBJECT *obj, long *flags);
static long InitPacket(OBJECT *obj, long *flags);
static long InitABC(OBJECT *obj, long *flags);
static long InitWaterBox(OBJECT *obj, long *flags);
static long InitRipple(OBJECT *obj, long *flags);
static long InitDolphin(OBJECT *obj, long *flags);
static long InitGardenFog(OBJECT *obj, long *flags);
static long InitFogBox(OBJECT *obj, long *flags);

#ifndef _N64
static long InitFlag(OBJECT *obj, long *flags); 
static void FreeFlag(OBJECT *obj);
#else
// Can't be InitFlag as there is already one in TitleScreen.h
static long InitOFlag(OBJECT *obj, long *flags); 
static void Free0Flag(OBJECT *obj);
#endif

// ***** PC Object init list ******

#ifdef _PC
/*static*/ OBJECT_INIT_DATA ObjInitData[] = {  //$MODIFIED(mwetzel): made non-static
    InitBarrel, sizeof(BARREL_OBJ),
    InitBeachball, 0,
    InitPlanet, 0,
    InitPlane, sizeof(PLANE_OBJ),
    InitCopter, sizeof(COPTER_OBJ),
    InitDragon, sizeof(DRAGON_OBJ),
    InitWater, 0,
    InitTrolley, sizeof(PLAYER),
    InitBoat, sizeof(BOAT_OBJ),
    InitSpeedup, sizeof(SPEEDUP_OBJ),
    InitRadar, sizeof(RADAR_OBJ),
    InitBalloon, sizeof(BALLOON_OBJ),
    InitHorse, sizeof(HORSE_OBJ),
    InitTrain, sizeof(TRAIN_OBJ),
    InitStrobe, sizeof(STROBE_OBJ),
    InitFootball, 0,
    InitSparkGen, sizeof(SPARK_GEN),
    InitSpaceman, sizeof(SPACEMAN_OBJ),

    InitShockwave, sizeof(SHOCKWAVE_OBJ),
    InitFirework, sizeof(FIREWORK_OBJ),
    InitPuttyBomb, 0,
    InitWaterBomb, sizeof(WATERBOMB_OBJ),
    InitElectroPulse, 0,
    InitOilSlick, sizeof(OILSLICK_OBJ),
    InitOilSlickDropper, sizeof(OILSLICK_DROPPER_OBJ),
    InitChromeBall, sizeof(CHROMEBALL_OBJ),
    InitClone, sizeof(CLONE_OBJ),
    InitTurbo2, 0,
    InitElectroZapped, 0,
    InitSpring, sizeof(SPRING_OBJ),

    InitPickup, sizeof(PICKUP_OBJ),
    InitDissolveModel, 0,

    InitFlap, 0,
    InitLaser, sizeof(LASER_OBJ),
    InitSplash, sizeof(SPLASH_OBJ),
    InitBombGlow, 0,
    InitWeebel, 0,
    InitProbeLogo, 0,
    InitClouds, sizeof(CLOUDS_OBJ),
    NULL, 0, //$MODIFIED(mwetzel) - was: InitNameWheel, sizeof(NAMEWHEEL_OBJ),
    InitSprinkler, sizeof(SPRINKLER_OBJ),
    InitSprinklerHose, sizeof(SPRINKLER_HOSE_OBJ),
    InitObjectThrower, sizeof(OBJECT_THROWER_OBJ),
    InitBasketBall, 0,
    NULL, 0, //$MODIFIED(mwetzel) - was: InitTrackScreen, sizeof(OBJECT_TRACKSCREEN_OBJ),
    InitClock, sizeof(OBJECT_CLOCK_OBJ),
    NULL, 0, //$MODIFIED(mwetzel) - was: InitCarBox, sizeof(OBJECT_CARBOX_OBJ),
    InitStream, 0,
    InitCup, sizeof(OBJECT_CUP_OBJ),
    Init3dSound, sizeof(SOUND3D_OBJ),
    InitStar, sizeof(STAR_OBJ),
    InitFox, 0,
    InitTumbleweed, 0,
    NULL, 0, //$MODIFIED(mwetzel) - was: InitSmallScreen, sizeof(SMALLSCREEN_OBJ),
    InitLantern, sizeof(LANTERN_OBJ),
    InitSkybox, 0,
    InitSlider, sizeof(SLIDER_OBJ),
    InitBottle, 0,
    InitBucket, 0,
    InitCone, 0,
    InitCan, 0,
    InitLilo, 0,
    InitGlobal, 0,
    InitRain, sizeof(RAIN_OBJ),
    InitLightning, sizeof(LIGHTNING_OBJ),
    InitShipLight, 0,
    InitPacket, 0,
    InitABC, 0,
    InitWaterBox, sizeof(WATERBOX_OBJ),
    InitRipple, sizeof(RIPPLE_OBJ),
    InitFlag, sizeof(FLAG_DATA_OBJ),
    InitDolphin, sizeof(DOLPHIN_OBJ),
    InitGardenFog, 0,
    InitFogBox, sizeof(FOGBOX_OBJ),
};
#endif

// ****** N64 Object init list ******

#ifdef _N64
static OBJECT_INIT_DATA ObjInitData[OBJECT_TYPE_MAX] = {
    InitBarrel, sizeof(BARREL_OBJ),                                                                                 
    InitBeachball, 0,
    InitPlanet, 0,
    InitPlane, sizeof(PLANE_OBJ),
    InitCopter, sizeof(COPTER_OBJ),
    NULL, 0,                                                        // InitDragon, sizeof(DRAGON_OBJ),
    InitWater, 0,
    InitTrolley, sizeof(PLAYER),
    InitBoat, sizeof(BOAT_OBJ),
    InitSpeedup, sizeof(SPEEDUP_OBJ),
    InitRadar, sizeof(RADAR_OBJ),
    InitBalloon, sizeof(BALLOON_OBJ),
    InitHorse, sizeof(HORSE_OBJ),
    InitTrain, sizeof(TRAIN_OBJ),
    InitStrobe, sizeof(STROBE_OBJ),
    NULL, 0,                                                        // InitFootball, 0,
    InitSparkGen, sizeof(SPARK_GEN),
    NULL, 0,                                                        // InitSpaceman, sizeof(SPACEMAN_OBJ),

    InitShockwave, sizeof(SHOCKWAVE_OBJ),
    InitFirework, sizeof(FIREWORK_OBJ),
    InitPuttyBomb, 0,
    InitWaterBomb, sizeof(WATERBOMB_OBJ),
    InitElectroPulse, 0,
    InitOilSlick, sizeof(OILSLICK_OBJ),
    InitOilSlickDropper, sizeof(OILSLICK_DROPPER_OBJ),
    InitChromeBall, sizeof(CHROMEBALL_OBJ),
    NULL, 0,                                                        // InitClone, sizeof(CLONE_OBJ),
    InitTurbo2, 0,
    NULL, 0,                                                        // InitElectroZapped, 0,
    NULL, 0,                                                        // InitSpring, sizeof(SPRING_OBJ),

    InitPickup, sizeof(PICKUP_OBJ),
    NULL, 0,                                                        // InitDissolveModel, 0,

    NULL, 0,                                                        // InitFlap, 0,
    InitLaser, sizeof(LASER_OBJ),
    InitSplash, sizeof(SPLASH_OBJ),
    NULL, 0,                                                        // InitBombGlow, 0,
    InitWeebel, 0,
    NULL, 0,                                                        // InitProbeLogo, 0,

    NULL, 0,                                                        // InitClouds, 0,
    NULL, 0, //$MODIFIED(mwetzel) - was: InitNameWheel, sizeof(NAMEWHEEL_OBJ),
    NULL, 0,                                                        //InitSprinkler, sizeof(SPRINKLER_OBJ),
    NULL, 0,                                                        //InitSprinklerHose, sizeof(SPRINKLER_HOSE_OBJ),
    NULL, 0,                                                        // InitObjectThrower, sizeof(OBJECT_THROWER_OBJ),
    NULL, 0,                                                        // InitBasketBall, 0,
    NULL, 0, //$MODIFIED(mwetzel) - was: InitTrackScreen, sizeof(OBJECT_TRACKSCREEN_OBJ),
    InitClock, sizeof(OBJECT_CLOCK_OBJ),
    NULL, 0, //$MODIFIED(mwetzel) - was: InitCarBox, sizeof(OBJECT_CARBOX_OBJ),
    NULL, 0,                                                        // InitStream, 0,
    InitCup, sizeof(OBJECT_CUP_OBJ),
    Init3dSound, sizeof(SOUND3D_OBJ),
    InitStar, sizeof(STAR_OBJ),
    InitFox, 0,
    InitTumbleweed, 0,
    NULL, 0, //$MODIFIED(mwetzel) - was: InitSmallScreen, sizeof(SMALLSCREEN_OBJ),
    InitLantern, sizeof(LANTERN_OBJ),
    NULL, 0,                                                        // InitSkybox, 0,
    NULL, 0,                                                        // InitSlider, sizeof(SLIDER_OBJ),
    InitBottle, 0,
    InitBucket, 0,
    InitCone, 0,
    NULL, 0,                                                        // InitCan, 0,
    NULL, 0,                                                        // InitLilo, 0,
    InitGlobal, 0,
    NULL, 0,                                                        // InitRain, sizeof(RAIN_OBJ),
    InitLightning, sizeof(LIGHTNING_OBJ),
    NULL, 0,                                                        // InitShipLight, 0,
    InitPacket, 0,
    InitABC, 0,
    NULL, 0,                                                        // InitWaterBox, sizeof(WATERBOX_OBJ),
    NULL, 0,                                                        // InitRipple, sizeof(RIPPLE_OBJ),
    NULL, 0,                                                        // InitOFlag, sizeof(FLAG_DATA_OBJ)
    NULL, 0,                                                        // InitDolphin, sizeof(DOLPHIN_OBJ),
    NULL, 0,                                                        // InitGardenFog, 0,
    InitFogBox, sizeof(FOGBOX_OBJ),
};
#endif


/////////////////////////////////
// load and init level objects //
/////////////////////////////////

#ifdef _PC
void LoadObjects(char *file)
{
    long i;
    FILE *fp;
    FILE_OBJECT fileobj;
    MAT mat;

// quit if in object edit mode

    if (EditMode == EDIT_OBJECTS)
        return;

// zero total pickup / star count

    NPickups = 0;
    TotalStarNum = 0;

// open object file

    fp = fopen(file, "rb");
    if (!fp)
        return;

// loop thru all objects

    fread(&i, sizeof(i), 1, fp);

    for ( ; i ; i--)
    {

// read file obj

        fread(&fileobj, sizeof(fileobj), 1, fp);

// DEBUGGING
#if USE_DEBUG_ROUTINES && defined(_PC)
        char buf[MALLOC_STRING_LENGTH];
        sprintf(buf, "Obj: %d ID: %d\n", i, fileobj.ID);
        SetMallocString(buf);
#endif


// skip if pickup and pickups off

        if ((fileobj.ID == OBJECT_TYPE_PICKUP) && !GameSettings.AllowPickups)
            continue;

// init object

        CopyVec(&fileobj.Up, &mat.mv[U]);
        CopyVec(&fileobj.Look, &mat.mv[L]);
        CrossProduct(&mat.mv[U], &mat.mv[L], &mat.mv[R]);

        CreateObject(&fileobj.Pos, &mat, fileobj.ID, fileobj.Flag);
    }

// close file

    fclose(fp);

// DEBUGGING
#if USE_DEBUG_ROUTINES && defined(_PC)
        SetMallocString("");
#endif

}
#endif

////////////////////////////
// count stars in a level //
////////////////////////////
#ifdef _PC
long CountFileStars(long levnum)
{
    FILE *fp;
    long i, num;
    FILE_OBJECT fileobj;

// open object file

    fp = fopen(GetAnyLevelFilename(levnum, FALSE, "fob", FILENAME_MAKE_BODY), "rb");
    if (!fp)
        return 0;

// loop thru all objects

    fread(&i, sizeof(i), 1, fp);

    num = 0;
    for ( ; i ; i--)
    {
        fread(&fileobj, sizeof(fileobj), 1, fp);
        if (fileobj.ID == OBJECT_TYPE_STAR)
            num++;

    }

// return num

    return num;
}
#endif
////////////////////////////////////////////////////////////////
//
// InitStars:  Turn on 1 random star for battle or all for rest
//
////////////////////////////////////////////////////////////////

void InitStars(void) 
{
    long i, j, star;
    OBJECT *obj;
    STAR_OBJ *starobj;

// no stars?

    if (!TotalStarNum)
        return;

// update total num if training / stunt

    if (GameSettings.GameType == GAMETYPE_TRAINING)
    {
        StarList.NumTotal = TotalStarNum;
    }

// pick random star if battle, or all if not

    star = GameSettings.GameType == GAMETYPE_NETWORK_BATTLETAG ? rand() % TotalStarNum : -1;

// turn on required stars

    i = -1;
    for (obj = OBJ_ObjectHead ; obj ; obj = obj->next) if (obj->Type == OBJECT_TYPE_STAR)
    {
        i++;
        if (star == i || star == -1)
        {

// collected training / stunt star?

            if (GameSettings.GameType == GAMETYPE_TRAINING)
            {
                for (j = 0 ; j < StarList.NumFound ; j++)
                {
                    if (i == StarList.ID[j])
                        break;
                }

                if (j != StarList.NumFound)
                    continue;
            }

// ok, turn on

            starobj = (STAR_OBJ*)obj->Data;
            starobj->Mode = 0;
            starobj->ID = i;
            obj->body.Centre.Pos = starobj->Pos;
        }
    }
}

//===========================================
// Load and init level objects - N64 version
//===========================================

#ifdef _N64

#if DEBUG 
static unsigned s_uNErrors;
static char *   s_apErrors[OBJECT_TYPE_MAX];
#endif

void LoadObjects()
{
    long        i, j;
    FIL         *fp;
    FILE_OBJECT fileobj;
    MAT         mat;
    long        Mem1, Mem2;
    unsigned    uNLasers = 0;

    Mem1 = MEM_GetMemUsed();

// zero total pickup / star count

    /*
    if (gTitleScreenVars.iLevelNum == LEVEL_NEIGHBOURHOOD1)
        return;
    */

#if DEBUG 
    s_uNErrors = 0;
#endif


    NPickups = 0;
    TotalStarNum = 0;

// open object file
    printf("Loading level objects...\n");
    fp = FFS_Open(FFS_TYPE_TRACK | TRK_OBJECTS);
    if (!fp) 
    {
        printf("WARNING: could not open object file.\n");
        return;
    }

// loop thru all objects

    FFS_Read(&i, sizeof(i), fp);
    i = EndConvLong(i);
    if (i > MAX_OBJECTS)
    {
        log_printf("WARNING: number of objects in file greater than N64 maximum.\n");
        i = MAX_OBJECTS;
    }
    log_printf("...loading %d objects.\n", i);

    for ( ; i ; i--)
    {

// read file obj
        FFS_Read(&fileobj, sizeof(fileobj), fp);
        fileobj.ID = EndConvLong(fileobj.ID);
        for (j = 0; j < FILE_OBJECT_FLAG_NUM; j++)
        {
            fileobj.Flag[j] = EndConvLong(fileobj.Flag[j]);
        }       
        fileobj.Pos.v[0] = EndConvReal(fileobj.Pos.v[0]);
        fileobj.Pos.v[1] = EndConvReal(fileobj.Pos.v[1]);
        fileobj.Pos.v[2] = EndConvReal(fileobj.Pos.v[2]);
        fileobj.Up.v[0] = EndConvReal(fileobj.Up.v[0]);
        fileobj.Up.v[1] = EndConvReal(fileobj.Up.v[1]);
        fileobj.Up.v[2] = EndConvReal(fileobj.Up.v[2]);
        fileobj.Look.v[0] = EndConvReal(fileobj.Look.v[0]);
        fileobj.Look.v[1] = EndConvReal(fileobj.Look.v[1]);
        fileobj.Look.v[2] = EndConvReal(fileobj.Look.v[2]);

// skip if pickup and pickups off

        if (GameSettings.GameType == GAMETYPE_TRIAL && fileobj.ID == OBJECT_TYPE_PICKUP)
            continue;
        
// init object

        CopyVec(&fileobj.Up, &mat.mv[U]);
        CopyVec(&fileobj.Look, &mat.mv[L]);
        CrossProduct(&mat.mv[U], &mat.mv[L], &mat.mv[R]);

        if (fileobj.ID == OBJECT_TYPE_LASER)
            uNLasers++;
        
        if ((fileobj.ID != OBJECT_TYPE_LASER) || uNLasers<=8)
            {
            CreateObject(&fileobj.Pos, &mat, fileobj.ID, fileobj.Flag);
            }

    }

// close file
    FFS_Close(fp);
    Mem2 = MEM_GetMemUsed();
    log_printf("...level objects use %d bytes.\n", Mem2 - Mem1);

// turn on random selection of pickups
    if (GameSettings.AllowPickups) {
        InitPickups();
    }
// init stars

    InitStars();
}
#endif

///////////////////
// create object //
///////////////////
#if DEBUG && defined(_N64)
char * s_NameObjectType[OBJECT_TYPE_MAX] = 
    {"OBJECT_TYPE_BARREL"
    ,"OBJECT_TYPE_BEACHBALL"
    ,"OBJECT_TYPE_PLANET"
    ,"OBJECT_TYPE_PLANE"
    ,"OBJECT_TYPE_COPTER"
    ,"OBJECT_TYPE_DRAGON"
    ,"OBJECT_TYPE_WATER"
    ,"OBJECT_TYPE_TROLLEY"
    ,"OBJECT_TYPE_BOAT"
    ,"OBJECT_TYPE_SPEEDUP"
    ,"OBJECT_TYPE_RADAR"
    ,"OBJECT_TYPE_BALLOON"
    ,"OBJECT_TYPE_HORSE"
    ,"OBJECT_TYPE_TRAIN"
    ,"OBJECT_TYPE_STROBE"
    ,"OBJECT_TYPE_FOOTBALL"
    ,"OBJECT_TYPE_SPARKGEN"
    ,"OBJECT_TYPE_SPACEMAN"

    ,"OBJECT_TYPE_SHOCKWAVE"
    ,"OBJECT_TYPE_FIREWORK"
    ,"OBJECT_TYPE_PUTTYBOMB"
    ,"OBJECT_TYPE_WATERBOMB"
    ,"OBJECT_TYPE_ELECTROPULSE"
    ,"OBJECT_TYPE_OILSLICK"
    ,"OBJECT_TYPE_OILSLICK_DROPPER"
    ,"OBJECT_TYPE_CHROMEBALL"
    ,"OBJECT_TYPE_CLONE"
    ,"OBJECT_TYPE_TURBO"
    ,"OBJECT_TYPE_ELECTROZAPPED"
    ,"OBJECT_TYPE_SPRING"
    ,"OBJECT_TYPE_PICKUP"
    ,"OBJECT_TYPE_DISSOLVEMODEL"
    ,"OBJECT_TYPE_FLAP"
    ,"OBJECT_TYPE_LASER"
    ,"OBJECT_TYPE_SPLASH"
    ,"OBJECT_TYPE_BOMBGLOW"
    ,"OBJECT_TYPE_WEEBEL"
    ,"OBJECT_TYPE_PROBELOGO"
    ,"OBJECT_TYPE_CLOUDS"
    ,"OBJECT_TYPE_NAMEWHEEL"
    ,"OBJECT_TYPE_SPRINKLER"
    ,"OBJECT_TYPE_SPRINKLER_HOSE"
    ,"OBJECT_TYPE_OBJECT_THROWER"
    ,"OBJECT_TYPE_BASKETBALL"
    ,"OBJECT_TYPE_TRACKSCREEN"
    ,"OBJECT_TYPE_CLOCK"
    ,"OBJECT_TYPE_CARBOX"
    ,"OBJECT_TYPE_STREAM"
    ,"OBJECT_TYPE_CUP"
    ,"OBJECT_TYPE_3DSOUND"
    ,"OBJECT_TYPE_STAR"
    ,"OBJECT_TYPE_FOX"
    ,"OBJECT_TYPE_TUMBLEWEED"
    ,"OBJECT_TYPE_SMALLSCREEN"
    ,"OBJECT_TYPE_LINTERN"
    ,"OBJECT_TYPE_SKYBOX"
    ,"OBJECT_TYPE_SLIDER"
    ,"OBJECT_TYPE_BOTTLE"
    ,"OBJECT_TYPE_BUCKET"
    ,"OBJECT_TYPE_CONE"
    ,"OBJECT_TYPE_CAN"
    ,"OBJECT_TYPE_LILO"
    ,"OBJECT_TYPE_GLOBAL"
    ,"OBJECT_TYPE_RAIN"
    ,"OBJECT_TYPE_LIGHTNING"
    ,"OBJECT_TYPE_SHIPLIGHT"
    ,"OBJECT_TYPE_PACKET"
    ,"OBJECT_TYPE_ABC"
    ,"OBJECT_TYPE_WATERBOX"
    ,"OBJECT_TYPE_RIPPLE"
    ,"OBJECT_TYPE_FLAG"
    ,"OBJECT_TYPE_DOLPHIN"
    ,"OBJECT_TYPE_GARDEN_FOG"

    };

#endif // DEBUG && N64

OBJECT *CreateObject(VEC *pos, MAT *mat, long ID, long *flags)
{
    OBJECT *obj;

// legal object?

    if (ID >= OBJECT_TYPE_MAX)
        {
        #if DEBUG && defined(_N64)
        printf ("... object ID:%d not recognized.\n",ID);
        #endif
        return NULL;
        }
// get object slot

    obj = OBJ_AllocObject();
    if (!obj) return NULL;

// set defaults

    obj->flag.Draw = TRUE;
    obj->flag.Move = TRUE;

    obj->renderflag.envmap = TRUE;
    obj->renderflag.envgood = FALSE;
    obj->renderflag.envonly = FALSE;
    obj->renderflag.light = TRUE;
    obj->renderflag.litsimple = FALSE;
    obj->renderflag.reflect = TRUE;
    obj->renderflag.fog = TRUE;
    obj->renderflag.glare = FALSE;
    obj->renderflag.meshfx = TRUE;
    obj->renderflag.shadow = FALSE;

    obj->EnvOffsetX = 0.0f;
    obj->EnvOffsetY = 0.0f;
    obj->EnvScale = 1.0f;

    obj->objref = NULL;
    obj->priority = 0;
    obj->Type = ID;
    obj->EnvRGB = 0x808080;
    obj->DefaultModel = -1;
    obj->CollType = COLL_TYPE_NONE;
    obj->Light = NULL;
    obj->Light2 = NULL;
    obj->Field = NULL;
#ifndef _PSX
 #ifdef OLD_AUDIO
    obj->Sfx3D = NULL;
 #else // !OLD_AUDIO
    obj->pSfxInstance = NULL;
 #endif // !OLD_AUDIO
#endif

    obj->aihandler = NULL;
    obj->collhandler = NULL;
    obj->movehandler = NULL;
    obj->renderhandler = (RENDER_HANDLER)RenderObject;
    obj->freehandler = NULL;
#ifdef _PC
    obj->remotehandler = NULL;
    obj->ServerControlled = FALSE;
    obj->GlobalID = INVALID_GLOBAL_ID;
#endif

// Set up safe default values for the body

    InitBodyDefault(&obj->body);

// initialise position and orientation from mapped object

    SetBodyPos(&obj->body, pos, mat);

// alloc required data

    obj->Data = NULL;
    if (ObjInitData[ID].AllocSize)
    {
        obj->Data = malloc(ObjInitData[ID].AllocSize);
        if (!obj->Data)
        {
            OBJ_FreeObject(obj);
            return NULL;
        }
    }

// call setup function

    if (  (!ObjInitData[ID].InitFunc)
       || (!ObjInitData[ID].InitFunc(obj, flags))
       )
        {
            // Debug Code to find out differents error.
            //-----------------------------------------
            #if DEBUG && defined(_N64)      
            if (  (ID != OBJECT_TYPE_PICKUP)
               && (ID != OBJECT_TYPE_3DSOUND)
               && (ID != OBJECT_TYPE_STAR)
               )
                {
                unsigned i;
                char *pcError = s_NameObjectType[ID];
                for (i =0; i<s_uNErrors && s_apErrors[i] != pcError; i++);
                if (i==s_uNErrors)
                    {
                    s_apErrors[s_uNErrors++] = pcError;
                    printf ("WARNING: object %s not created. ID %d\n",s_NameObjectType[ID],ID);
                    }
                }
            #endif
            OBJ_FreeObject(obj);
            return NULL;
        }

// Move to head of list if object can collide with others
    if (obj->body.CollSkin.AllowObjColls) 
    {
        MoveObjectToHead(obj);
    }

// return created object

    return obj;
}


/////////////////
// init barrel //
/////////////////

static long InitBarrel(OBJECT *obj, long *flags)
{
    BARREL_OBJ *barrel = (BARREL_OBJ*)obj->Data;

    //return FALSE;

// set render flags

    obj->renderflag.envmap = FALSE;
    obj->renderflag.reflect = FALSE;
#ifdef _N64
    obj->renderflag.light = FALSE;
#endif

// set spin speed

    barrel->SpinSpeed = (REAL)flags[0] / 32768.0f;

// set ai handler

    obj->aihandler = (AI_HANDLER)AI_BarrelHandler;

// set default model

    obj->DefaultModel = LoadOneLevelModel(LEVEL_MODEL_BARREL, TRUE, obj->renderflag, 0);
    if (obj->DefaultModel == -1) return FALSE;


// No collision

    obj->CollType = COLL_TYPE_NONE;
    obj->body.CollSkin.AllowObjColls = FALSE;
    obj->body.CollSkin.AllowWorldColls = FALSE;
    
// return OK

    return TRUE;
}


#ifdef _PC
///////////////////
// init football //
///////////////////

static long InitFootball(OBJECT *obj, long *flags)
{

    // not if time trial
    if (GameSettings.GameType == GAMETYPE_TRIAL)
        return FALSE;

    // set default model
    obj->DefaultModel = LoadOneLevelModel(LEVEL_MODEL_FOOTBALL, TRUE, obj->renderflag, 0);
    if (obj->DefaultModel == -1) return FALSE;


    // set collision handler and type
    obj->CollType = COLL_TYPE_BODY;
    obj->collhandler = (COLL_HANDLER)COL_BodyCollHandler;

    // set move handler
    obj->movehandler = (MOVE_HANDLER)MOV_MoveBody;
#ifndef _PSX
    obj->replayhandler = (REPLAY_HANDLER)DefaultObjectReplayStoreHandler;
#endif

    // setup remote handler
#ifdef _PC
    if (IsMultiPlayer())
    {
        obj->GlobalID = GetGlobalID(ServerID);
        InitRemoteObjectData(obj);

        if (IsServer())
        {
            obj->remotehandler = (REMOTE_HANDLER)SendObjectData;
        }
        else
        {
            obj->ServerControlled = TRUE;
        }
    }
#endif

    // Physical properties
    obj->body.Centre.Mass = Real(0.2f);
    obj->body.Centre.InvMass = ONE / Real(0.2f);
    SetMat(&obj->body.BodyInertia, Real(100), ZERO, ZERO, ZERO, Real(100), ZERO, ZERO, ZERO, Real(100));
    SetMat(&obj->body.BodyInvInertia, ONE / Real(100), ZERO, ZERO, ZERO, ONE / Real(100), ZERO, ZERO, ZERO, ONE / Real(100));
    GetFrameInertia(&obj->body.BodyInvInertia, &obj->body.Centre.WMatrix, &obj->body.WorldInvInertia);

    obj->body.Centre.Hardness = Real(0.6);
    obj->body.Centre.Resistance = Real(0.001);
    obj->body.DefaultAngRes = Real(0.005);
    obj->body.AngResistance = Real(0.005);
    obj->body.AngResMod = Real(1.0);
    obj->body.Centre.Grip = Real(0.005);
    obj->body.Centre.StaticFriction = Real(1.3);
    obj->body.Centre.KineticFriction = Real(0.8);

    // Collision skin
    SetBodySphere(&obj->body);
#ifdef _N64
    obj->body.CollSkin.Sphere = &obj->Sphere;
#else
    obj->body.CollSkin.Sphere = (SPHERE *)malloc(sizeof(SPHERE));
#endif
    SetVecZero(&obj->body.CollSkin.Sphere[0].Pos);
    obj->body.CollSkin.Sphere[0].Radius = Real(30);
    obj->body.CollSkin.NSpheres = 1;
    CreateCopyCollSkin(&obj->body.CollSkin);
    MakeTightLocalBBox(&obj->body.CollSkin);
    InitWorldSkin(&obj->body.CollSkin, &obj->body.Centre.Pos, &obj->body.Centre.WMatrix);
    BuildWorldSkin(&obj->body.CollSkin, &obj->body.Centre.Pos, &obj->body.Centre.WMatrix);

    return TRUE;
}
#endif

/////////////////////////////////////////////////////////////////////
// init beachball
/////////////////////////////////////////////////////////////////////

static long InitBeachball(OBJECT *obj, long *flags)
{

    // not if time trial
    if (GameSettings.GameType == GAMETYPE_TRIAL)
        return FALSE;

    // set env rgb
    obj->EnvRGB = 0x202020;
#ifdef _N64
    obj->renderflag.envmap = FALSE;
    obj->renderflag.envonly = FALSE;
#endif

#ifdef _PC
    obj->renderflag.shadow = TRUE;
    obj->EnvOffsetY = 0.1f;
    obj->EnvScale = 0.5f;
#endif

    // set default model
    obj->DefaultModel = LoadOneLevelModel(LEVEL_MODEL_BEACHBALL, TRUE, obj->renderflag, TPAGE_FX1);
    if (obj->DefaultModel == -1) return FALSE;

    // set collision handler and type
    obj->CollType = COLL_TYPE_BODY;
    obj->collhandler = (COLL_HANDLER)COL_BodyCollHandler;

    // setup ai handler
    obj->aihandler = (AI_HANDLER)AI_BangNoiseHandler;

    // set move handler
    obj->movehandler = (MOVE_HANDLER)MOV_MoveBody;
#ifndef _PSX
    obj->replayhandler = (REPLAY_HANDLER)DefaultObjectReplayStoreHandler;
#endif

    // setup remote handler
#ifdef _PC
    if (IsMultiPlayer())
    {
        obj->GlobalID = GetGlobalID(ServerID);
        InitRemoteObjectData(obj);

        if (IsServer())
        {
            obj->remotehandler = (REMOTE_HANDLER)SendObjectData;
        }
        else
        {
            obj->ServerControlled = TRUE;
        }
    }
#endif

    // Physical properties
    obj->body.Centre.Mass = Real(0.1f);
    obj->body.Centre.InvMass = ONE / Real(0.1f);
    SetMat(&obj->body.BodyInertia, Real(100), ZERO, ZERO, ZERO, Real(100), ZERO, ZERO, ZERO, Real(100));
    SetMat(&obj->body.BodyInvInertia, ONE / Real(100), ZERO, ZERO, ZERO, ONE / Real(100), ZERO, ZERO, ZERO, ONE / Real(100));
    GetFrameInertia(&obj->body.BodyInvInertia, &obj->body.Centre.WMatrix, &obj->body.WorldInvInertia);

    obj->body.Centre.Hardness = Real(0.6);
    obj->body.Centre.Resistance = Real(0.005);
    obj->body.DefaultAngRes = Real(0.005);
    obj->body.AngResistance = Real(0.005);
    obj->body.AngResMod = Real(1.0);
    obj->body.Centre.Grip = Real(0.005);
    obj->body.Centre.StaticFriction = Real(1.0);
    obj->body.Centre.KineticFriction = Real(0.5);
    obj->body.Centre.Boost = ZERO;

    // Collision skin
    SetBodySphere(&obj->body);
#ifdef _N64
    obj->body.CollSkin.Sphere = &obj->Sphere;
#else
    obj->body.CollSkin.Sphere = (SPHERE *)malloc(sizeof(SPHERE));
#endif
    SetVecZero(&obj->body.CollSkin.Sphere[0].Pos);
    obj->body.CollSkin.Sphere[0].Radius = Real(100);
    obj->body.CollSkin.NSpheres = 1;
    CreateCopyCollSkin(&obj->body.CollSkin);
    MakeTightLocalBBox(&obj->body.CollSkin);
    InitWorldSkin(&obj->body.CollSkin, &obj->body.Centre.Pos, &obj->body.Centre.WMatrix);
    BuildWorldSkin(&obj->body.CollSkin, &obj->body.Centre.Pos, &obj->body.Centre.WMatrix);

    return TRUE;
}


/////////////////
// init planet //
/////////////////

static long InitPlanet(OBJECT *obj, long *flags)
{
    long i;
    VEC vec;
    MAT mat;
    PLANET_OBJ *planet;
    SUN_OBJ *sun;
    BOUNDING_BOX box;

#ifdef _N64
// Crashes in N64's battle... crap fix to be able to release it now.
    if (GameSettings.GameType == GAMETYPE_NETWORK_BATTLETAG)
        return FALSE;

// No rings for saturn... 
    if (flags[0] == (LEVEL_MODEL_RINGS - LEVEL_MODEL_MERCURY) )
        return FALSE;

#endif

// alloc memory

    if (flags[0] != PLANET_SUN)
        obj->Data = malloc(sizeof(PLANET_OBJ));
    else
        obj->Data = malloc(sizeof(SUN_OBJ));

    if (!obj->Data)
        return FALSE;

    planet = (PLANET_OBJ*)obj->Data;
    sun = (SUN_OBJ*)obj->Data;

// set render flags

    obj->renderflag.envmap = FALSE;
    obj->renderflag.reflect = FALSE;

// set me / orbit planets

    planet->OwnPlanet = flags[0];
    planet->OrbitPlanet = flags[1];

// set orbit speed

    planet->OrbitSpeed = (REAL)abs(flags[2]) / 32768.0f;

// set spin speed

    planet->SpinSpeed = (REAL)flags[3] / 4096.0f;

// set ai handler

    obj->aihandler = (AI_HANDLER)AI_PlanetHandler;

// No collision

    obj->CollType = COLL_TYPE_NONE;
    obj->body.CollSkin.AllowObjColls = FALSE;
    obj->body.CollSkin.AllowWorldColls = FALSE;
    
// set default model?

    if (planet->OwnPlanet != PLANET_SUN)
    {
        obj->renderhandler = (RENDER_HANDLER)RenderPlanet;
        obj->DefaultModel = LoadOneLevelModel(LEVEL_MODEL_MERCURY + planet->OwnPlanet, TRUE, obj->renderflag, 0);
        if (obj->DefaultModel == -1) return FALSE;

    }

// setup sun?

    else
    {
        obj->renderhandler = (RENDER_HANDLER)RenderSun;
        obj->Light = AllocLight();
        if (obj->Light)
        {
            obj->Light->x = obj->body.Centre.Pos.v[X];
            obj->Light->y = obj->body.Centre.Pos.v[Y];
            obj->Light->z = obj->body.Centre.Pos.v[Z];
            obj->Light->Reach = 12000;
            obj->Light->Flag = LIGHT_MOVING;
            obj->Light->Type= LIGHT_OMNINORMAL;
            obj->Light->r = 256;
            obj->Light->g = 256;
            obj->Light->b = 256;
        }

#ifdef _PC
        SunFacingPoly.Xsize = 1750;
        SunFacingPoly.Ysize = 1750;
        SunFacingPoly.U = 0.0f;
        SunFacingPoly.V = 0.0f;
        SunFacingPoly.Usize = 1.0f;
        SunFacingPoly.Vsize = 1.0f;
        SunFacingPoly.Tpage = TPAGE_MISC1;
        SunFacingPoly.RGB = 0xffffff;

//$MODIFIED
//        LoadTextureClever("levels\\muse2\\sun.bmp", TPAGE_MISC1, 256, 256, 0, FxTextureSet, FALSE);
        LoadTextureClever("D:\\levels\\muse2\\sun.bmp", TPAGE_MISC1, 256, 256, 0, FxTextureSet, FALSE);
//$END_MODIFICATIONS
#endif
        for (i = 0 ; i < SUN_OVERLAY_NUM ; i++)
        {
            sun->Overlay[i].Rot = frand(1.0f);
            sun->Overlay[i].RotVel = frand(0.001f) - 0.0005f;
        }

        for (i = 0 ; i < SUN_STAR_NUM ; i++)
        {
            SetVector(&vec, 0, 0, 6144);
            RotMatrixZYX(&mat, frand(0.5f) - 0.25f, frand(1.0f), 0);
            RotVector(&mat, &vec, &sun->Star[i].Pos);
            sun->Star[i].rgb = ((rand() & 127) + 128) | ((rand() & 127) + 128) << 8 | ((rand() & 127) + 128) << 16;
        }

        box.Xmin = obj->body.Centre.Pos.v[X] - 3072;
        box.Xmax = obj->body.Centre.Pos.v[X] + 3072;
        box.Ymin = obj->body.Centre.Pos.v[Y] - 3072;
        box.Ymax = obj->body.Centre.Pos.v[Y] + 3072;
        box.Zmin = obj->body.Centre.Pos.v[Z] - 3072;
        box.Zmax = obj->body.Centre.Pos.v[Z] + 3072;

        sun->VisiMask = SetObjectVisiMask(&box);
    }

// return OK

    return TRUE;
}


////////////////
// init plane //
////////////////

static long InitPlane(OBJECT *obj, long *flags)
{
    MAT mat, mat2;
    PLANE_OBJ *plane = (PLANE_OBJ*)obj->Data;

// set render flags

    obj->renderflag.reflect = FALSE;

// save pos

    CopyVec(&obj->body.Centre.Pos, &plane->GenPos);

// set speed

    plane->Speed = (REAL)flags[0] / 16384.0f;
    plane->Rot = 0;

// set radius

    SetVector(&plane->Offset, 0, 0, (REAL)flags[1]);

// set bank

    if (plane->Speed > 0)
        RotMatrixY(&mat2, 0.25f);
    else
        RotMatrixY(&mat2, 0.75f);

    RotMatrixZ(&mat, (REAL)flags[2] / 512.0f);
    MulMatrix(&mat2, &mat, &plane->BankMatrix);
    
// set ai handler

    obj->aihandler = (AI_HANDLER)AI_PlaneHandler;

// set render handler

    obj->renderhandler = (RENDER_HANDLER)RenderPlane;

// set default model

    obj->DefaultModel = LoadOneLevelModel(LEVEL_MODEL_PLANE, TRUE, obj->renderflag, 0);
    if (obj->DefaultModel == -1) return FALSE;


// load propellor model

    plane->PropModel = LoadOneLevelModel(LEVEL_MODEL_PLANE_PROPELLOR, TRUE, obj->renderflag, 0);

// No collision

    obj->CollType = COLL_TYPE_NONE;
    obj->body.CollSkin.AllowObjColls = FALSE;
    obj->body.CollSkin.AllowWorldColls = FALSE;
    
// create 3D sfx
#ifndef _PSX
 #ifdef OLD_AUDIO
    obj->Sfx3D = CreateSfx3D(SFX_TOY_PLANE, SFX_MAX_VOL, 22050, TRUE, &obj->body.Centre.Pos, 0);
 #else // !OLD_AUDIO
    g_SoundEngine.Play3DSound( g_dwLevelSoundsOffset + EFFECT_toy_plane, TRUE, obj, &obj->pSfxInstance );
    //$TODO(JHarding): need to add toy plane sound
 #endif // !OLD_AUDIO
#endif
// return OK

    return TRUE;
}

/////////////////
// init copter //
/////////////////

static long InitCopter(OBJECT *obj, long *flags)
{
    BBOX    bBox = {-200, 200, -1000, 1000, -200, 200};
    VEC size = {200, 2000, 200};

    COPTER_OBJ *copter; 


    // Not present in Practice mode to avoid collision after GO.
    if ((GameSettings.Level    == LEVEL_TOYWORLD1)   &&
        ((GameSettings.GameType == GAMETYPE_PRACTICE ) || (GameSettings.GameType == GAMETYPE_TRIAL))
        )
    {
        return FALSE;
    }

// set render flags

    copter = (COPTER_OBJ*)obj->Data;
    obj->renderflag.reflect = FALSE;

// set ai handler

    obj->aihandler = (AI_HANDLER)AI_CopterHandler;

// set render handler

    obj->renderhandler = (RENDER_HANDLER)RenderCopter;

// set default model

    obj->DefaultModel = LoadOneLevelModel(LEVEL_MODEL_COPTER, TRUE, obj->renderflag, 0);
    if (obj->DefaultModel == -1) return FALSE;

// load extra models

    copter->BladeModel1 = LoadOneLevelModel(LEVEL_MODEL_COPTER_BLADE1, TRUE, obj->renderflag, 0);
    copter->BladeModel2 = LoadOneLevelModel(LEVEL_MODEL_COPTER_BLADE2, TRUE, obj->renderflag, 0);

// create 3D sfx
#ifndef _PSX
 #ifdef OLD_AUDIO
    obj->Sfx3D = CreateSfx3D(SFX_TOY_COPTER, SFX_MAX_VOL, 22050, TRUE, &obj->body.Centre.Pos, 0);
 #else // !OLD_AUDIO
    g_SoundEngine.Play3DSound( g_dwLevelSoundsOffset + EFFECT_toy_copter, TRUE, obj, &obj->pSfxInstance );
    //$TODO(JHarding): need to add toy copter sound
 #endif // !OLD_AUDIO
#endif

// set bounding box for flying around in

    copter->FlyBox.XMin = obj->body.Centre.Pos.v[X] - (float)flags[0] * 10;
    copter->FlyBox.XMax = obj->body.Centre.Pos.v[X] + (float)flags[0] * 10;
    copter->FlyBox.YMin = obj->body.Centre.Pos.v[Y] - (float)flags[1] * 10 - (float)flags[3] * 50;
    copter->FlyBox.YMax = obj->body.Centre.Pos.v[Y] + (float)flags[1] * 10 - (float)flags[3] * 50;
    copter->FlyBox.ZMin = obj->body.Centre.Pos.v[Z] - (float)flags[2] * 10;
    copter->FlyBox.ZMax = obj->body.Centre.Pos.v[Z] + (float)flags[2] * 10;

// set motion properties of copter
    copter->State = COPTER_WAIT;
    copter->TurnTime = ZERO;
    copter->Acc = 100;
    copter->MaxVel = 300;

    obj->movehandler = NULL;

// Physical properties
    obj->body.Centre.Mass = ZERO;
    obj->body.Centre.InvMass = ZERO;
    SetMat(&obj->body.BodyInertia, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO);
    SetMat(&obj->body.BodyInvInertia, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO);
    GetFrameInertia(&obj->body.BodyInvInertia, &obj->body.Centre.WMatrix, &obj->body.WorldInvInertia);

    obj->body.Centre.Hardness = Real(0.5);
    obj->body.Centre.Resistance = Real(0.0);
    obj->body.DefaultAngRes = Real(0.0);
    obj->body.AngResistance = Real(0.0);
    obj->body.AngResMod = Real(0.0);
    obj->body.Centre.Grip = Real(0.01);
    obj->body.Centre.StaticFriction = Real(1.0);
    obj->body.Centre.KineticFriction = Real(0.5);

// Store initial orientation
    MatToQuat(&obj->body.Centre.WMatrix, &copter->InitialQuat);
    CopyQuat(&copter->InitialQuat, &copter->CurrentUpQuat);

// No collision

    obj->CollType = COLL_TYPE_NONE;
    obj->body.CollSkin.AllowObjColls = FALSE;
    obj->body.CollSkin.AllowWorldColls = FALSE;
    
// Collision skin
    /*SetBodySphere(&obj->body);
    obj->body.CollSkin.AllowWorldColls = FALSE;
    SetBBox(&obj->body.CollSkin.BBox, Real(-150), Real(150), Real(-150), Real(150), Real(-150), Real(150));
    obj->body.CollSkin.Radius = Real(150);*/

// Force field
        obj->Field = AddLocalField(
        obj->ObjID,
        FIELD_PRIORITY_MAX, 
        &obj->body.Centre.Pos,
        &obj->body.Centre.WMatrix,
        &bBox,
        &size,
        &obj->body.Centre.WMatrix.mv[U],
        500,
        ZERO);

// return OK

    return TRUE;
}

#ifdef _PC
/////////////////
// init dragon //
/////////////////

static long InitDragon(OBJECT *obj, long *flags)
{
    long i;
    DRAGON_OBJ *dragon = (DRAGON_OBJ*)obj->Data;

// set render flags

    obj->renderflag.reflect = FALSE;

// set ai handler

    obj->aihandler = (AI_HANDLER)AI_DragonHandler;

// set render handler

    obj->renderhandler = (RENDER_HANDLER)RenderDragon;

// load body

    dragon->BodyModel = LoadOneLevelModel(LEVEL_MODEL_DRAGON1, TRUE, obj->renderflag, TPAGE_MISC1);

// load head models

    dragon->HeadModel = LoadOneLevelModel(LEVEL_MODEL_DRAGON2, TRUE, obj->renderflag, TPAGE_MISC1);
    if (dragon->HeadModel != -1)
    {
        SetModelFrames(&LevelModel[dragon->HeadModel].Model, DragonMorphFrames, 2);
    }

// load texture

//$MODIFIED
//    LoadTextureClever("levels\\toy2\\dragon.bmp", TPAGE_MISC1, 256, 256, 0, FxTextureSet, TRUE);
    LoadTextureClever("D:\\levels\\toy2\\dragon.bmp", TPAGE_MISC1, 256, 256, 0, FxTextureSet, TRUE);
//$END_MODIFICATIONS

// set anim count

    dragon->Count = 0;

// init fire

    for (i = 0 ; i < DRAGON_FIRE_NUM ; i++)
    {
        dragon->Fire[i].Time = 0;
    }

// set fire gen point, normal

    RotTransVector(&obj->body.Centre.WMatrix, &obj->body.Centre.Pos, &DragonFireOffset, &dragon->FireGenPoint);
    RotVector(&obj->body.Centre.WMatrix, &DragonFireDir, &dragon->FireGenDir);

// setup fire facing poly

    DragonFireFacingPoly.U = 64.0f / 256.0f;
    DragonFireFacingPoly.V = 0.0f;
    DragonFireFacingPoly.Usize = 64.0f / 256.0f;
    DragonFireFacingPoly.Vsize = 64.0f / 256.0f;
    DragonFireFacingPoly.Tpage = TPAGE_FX1;

// set fire gen time stamp

    dragon->FireGenTime = CurrentTimer();

// collision

    obj->CollType = COLL_TYPE_BODY;
    obj->body.CollSkin.CollType = BODY_COLL_POLY;
    obj->body.CollSkin.AllowWorldColls = FALSE;
    obj->body.CollSkin.AllowObjColls = TRUE;

    obj->body.CollSkin.NConvex = LevelModel[dragon->BodyModel].CollSkin.NConvex;
    obj->body.CollSkin.NSpheres = LevelModel[dragon->BodyModel].CollSkin.NSpheres;
    obj->body.CollSkin.NCollPolys = LevelModel[dragon->BodyModel].CollSkin.NCollPolys;
    obj->body.CollSkin.Convex = LevelModel[dragon->BodyModel].CollSkin.Convex;
    obj->body.CollSkin.Sphere = LevelModel[dragon->BodyModel].CollSkin.Sphere;
    obj->body.CollSkin.CollPoly = LevelModel[dragon->BodyModel].CollSkin.CollPoly;

    SetBBox(&obj->body.CollSkin.TightBBox, -750, 750, -800, 200, -650, 650);

    CreateCopyCollSkin(&obj->body.CollSkin);
    InitWorldSkin(&obj->body.CollSkin, &obj->body.Centre.Pos, &obj->body.Centre.WMatrix);
    BuildWorldSkin(&obj->body.CollSkin, &obj->body.Centre.Pos, &obj->body.Centre.WMatrix);
    TransCollPolys(obj->body.CollSkin.WorldCollPoly, obj->body.CollSkin.NCollPolys, &obj->body.Centre.Pos);
    
// return OK

    return TRUE;
}
#endif

////////////////
// init water //
////////////////

#ifdef _PC
static long InitWater(OBJECT *obj, long *flags)
{
    long i;
    WATER_OBJ *water;
    MODEL *model;

// set render flags

    obj->renderflag.reflect = FALSE;
    obj->renderflag.envmap = FALSE;

// set ai handler

    obj->aihandler = (AI_HANDLER)AI_WaterHandler;

// load default model
    obj->DefaultModel = LoadOneLevelModel(LEVEL_MODEL_WATER, TRUE, obj->renderflag, 0);
    if (obj->DefaultModel == -1) return FALSE;
    model = &LevelModel[obj->DefaultModel].Model;

// alloc ram

    obj->Data = (WATER_OBJ*)malloc(sizeof(WATER_OBJ) + sizeof(WATER_VERTEX) * (model->VertNum - 1));
    if (!obj->Data) return FALSE;

// load texture

//$MODIFIED
//    LoadTextureClever("levels\\toylite\\water.bmp", TPAGE_MISC1, 256, 256, 0, FxTextureSet, TRUE);
    LoadTextureClever("D:\\levels\\toylite\\water.bmp", TPAGE_MISC1, 256, 256, 0, FxTextureSet, TRUE);
//$END_MODIFICATIONS

// setup water

    water = (WATER_OBJ*)obj->Data;
    
    water->Scale = 5;

    water->VertNum = model->VertNum;

    for (i = 0 ; i < water->VertNum ; i++)
    {
        water->Vert[i].Height = model->VertPtr[i].y;
        water->Vert[i].Time = 0;
        water->Vert[i].TotalTime = frand(2.0f) + 1.0f;
    }

// force model poly attribs
    model->QuadNumTex = 0;
    model->TriNumTex = model->PolyNum;
    model->QuadNumRGB = 0;
    model->TriNumRGB = 0;

    for (i = 0 ; i < model->PolyNum ; i++)
    {
        model->PolyPtr[i].Type = POLY_SEMITRANS;
        model->PolyPtr[i].Tpage = TPAGE_MISC1;

        *(long*)&model->PolyRGB[i].rgb[0] = 0xc0808080;
        *(long*)&model->PolyRGB[i].rgb[1] = 0xc0808080;
        *(long*)&model->PolyRGB[i].rgb[2] = 0xc0808080;
    }

// return OK

    return TRUE;
}
#endif

#ifdef _N64
static long InitWater(OBJECT *obj, long *flags)
{
    long i;
    long j;
    WATER_OBJ *water;
    MODEL *model;

// set render flags

    obj->renderflag.reflect = FALSE;
    obj->renderflag.envmap = FALSE;
    obj->renderflag.envonly = TRUE;
    obj->renderflag.litsimple = FALSE;
    obj->renderhandler = (RENDER_HANDLER)RenderWater;

// set ai handler

    obj->aihandler = (AI_HANDLER)AI_WaterHandler;

// load default model
    if (!GG_WaterModelLoaded)
    {
        GG_WaterModel = LoadOneLevelModel(LEVEL_MODEL_WATER, TRUE, obj->renderflag, 0);
    }
    obj->DefaultModel = GG_WaterModel;
    if (obj->DefaultModel == -1) return FALSE;
    model = &LevelModel[obj->DefaultModel].Model;

// alloc ram

    obj->Data = (WATER_OBJ*)malloc(sizeof(WATER_OBJ) + sizeof(WATER_VERTEX) * (model->hdr->vtxnum - 1));
    if (!obj->Data) return FALSE;

// setup water

    water = (WATER_OBJ*)obj->Data;
    
    water->Scale = 5;
    water->VertNum = model->hdr->vtxnum;

    for (i = 0 ; i < water->VertNum ; i++)
    {
        water->Vert[i].Height = model->hdr->evtxptr[i].v.ob[1];
        water->Vert[i].Time = 0;
        water->Vert[i].TotalTime = frand(2.0f) + 1.0f;
        for (j = 0; j < water->VertNum; j++)
        {
            if (i == j) continue;
            if ((model->hdr->evtxptr[i].v.ob[0] == model->hdr->evtxptr[j].v.ob[0]) &&
                (model->hdr->evtxptr[i].v.ob[2] == model->hdr->evtxptr[j].v.ob[2]))
            {
                water->Vert[j].TotalTime = water->Vert[i].TotalTime;
            }
        }
    }

// No collision

    obj->CollType = COLL_TYPE_NONE;
    obj->body.CollSkin.AllowObjColls = FALSE;
    obj->body.CollSkin.AllowWorldColls = FALSE;
    
// return OK

    return TRUE;
}
#endif


#ifndef _PSX
/////////////////////////////////////////////////////////////////////
//
// InitTrolley: shopping trolley
//
/////////////////////////////////////////////////////////////////////

static long InitTrolley(OBJECT *obj, long *flags)
{
    VEC pos;
    MAT mat;

    // not if multiplayer
#ifdef _PC
    if (IsMultiPlayer())
    {
        return FALSE;
    }
#endif

    // How nice it's to have averything initialized to 0...
    // Hopefully this is the last hidden bug...
    #ifdef _N64
    memset(obj->Data,0,sizeof(PLAYER));
    #endif

    // setup misc
    obj->player = (PLAYER *)obj->Data;
    
    obj->player->car.Body = &obj->body;
    obj->player->type = PLAYER_CPU;
    obj->player->ctrltype = CTRL_TYPE_NONE;
    obj->player->ownobj = obj;
    obj->player->Slot = 0;
    obj->player->conhandler = NULL;
    obj->player->car.RenderFlag = CAR_RENDER_NORMAL;

    obj->CollType = COLL_TYPE_CAR;
    obj->movehandler = (MOVE_HANDLER)MOV_MoveCarNew;
    obj->collhandler = (COLL_HANDLER)COL_CarCollHandler;
    obj->defaultmovehandler = (MOVE_HANDLER)MOV_MoveCarNew;
    obj->defaultcollhandler = (COLL_HANDLER)COL_CarCollHandler;
    obj->replayhandler = (REPLAY_HANDLER)DefaultObjectReplayStoreHandler;
    obj->aihandler = (AI_HANDLER)TrolleyAIHandler;
    obj->renderhandler = (RENDER_HANDLER)RenderTrolley;
    obj->freehandler  = (FREE_HANDLER)FreeTrolley;

    // setup remote handler
#ifdef _PC
    if (IsMultiPlayer())
    {
        obj->GlobalID = GetGlobalID(ServerID);
        InitRemoteObjectData(obj);

        if (IsServer())
        {
            obj->remotehandler = (REMOTE_HANDLER)SendObjectData;
        }
        else
        {
            obj->ServerControlled = TRUE;
        }
    }
#endif

    obj->flag.Draw = TRUE;
    obj->flag.Move = TRUE;
    obj->Type = OBJECT_TYPE_TROLLEY;
    obj->Field = NULL;

    // Store positions so that they can be restores after car initialisation
    CopyVec(&obj->body.Centre.Pos, &pos);
    CopyMat(&obj->body.Centre.WMatrix, &mat);

    InitCar(&obj->player->car);
    SetupCar(obj->player, CARID_TROLLEY);
    SetCarPos(&obj->player->car, &pos, &mat);

    return TRUE;
}

static void FreeTrolley(OBJECT *obj)
{
    FreeCar(obj->player);
}
#endif

void TrolleyAIHandler(OBJECT *obj)
{

    // Don't let trolley fall over
    if (obj->body.Centre.WMatrix.m[UY] < Real(0.5)) {
        obj->player->car.RightingCollide = TRUE;
        obj->player->car.RightingReachDest = FALSE;
        obj->movehandler = (MOVE_HANDLER)MOV_RightCar;
    }
}


///////////////
// init boat //
///////////////
#ifndef _PSX
static long InitBoat(OBJECT *obj, long *flags)
{
    BOAT_OBJ *boat = (BOAT_OBJ*)obj->Data;

// set render flags

    obj->renderflag.reflect = FALSE;
    obj->EnvRGB = 0x404080;

// set ai handler

    obj->aihandler = (AI_HANDLER)AI_BoatHandler;

// load default model

    obj->DefaultModel = LoadOneLevelModel(LEVEL_MODEL_BOAT1 + flags[0], TRUE, obj->renderflag, 0);
    if (obj->DefaultModel == -1) return FALSE;

// setup boat

    boat->Height = obj->body.Centre.Pos.v[Y];

    CopyMat(&obj->body.Centre.WMatrix, &boat->Ori);

    boat->TimeX = boat->TimeHeight = boat->TimeZ = 0;

    boat->TotalTimeX = frand(2.0f) + 4.0f;
    boat->TotalTimeHeight = frand(1.0f) + 2.0f;
    boat->TotalTimeZ = frand(2.0f) + 4.0f;
    boat->SteamTime = ZERO;

// No collision

    obj->CollType = COLL_TYPE_NONE;
    obj->body.CollSkin.AllowObjColls = FALSE;
    obj->body.CollSkin.AllowWorldColls = FALSE;
    
// return OK

    return TRUE;
}

////////////////
// init radar //
////////////////

static long InitRadar(OBJECT *obj, long *flags)
{
    RADAR_OBJ *radar = (RADAR_OBJ*)obj->Data;

// set render flags

    obj->renderflag.reflect = FALSE;

// set ai handler

    obj->aihandler = (AI_HANDLER)AI_RadarHandler;

// load default model

    obj->DefaultModel = LoadOneLevelModel(LEVEL_MODEL_RADAR, TRUE, obj->renderflag, 0);
    if (obj->DefaultModel == -1) return FALSE;

// setup radar

    radar->Time = 0;

    obj->Light = AllocLight();
    if (obj->Light)
    {
        obj->Light->x = obj->body.Centre.Pos.v[X];
        obj->Light->y = obj->body.Centre.Pos.v[Y];
        obj->Light->z = obj->body.Centre.Pos.v[Z];
        obj->Light->Reach = 2500;
        obj->Light->Flag = LIGHT_FIXED | LIGHT_MOVING;
        obj->Light->Type= LIGHT_SPOTNORMAL;
        obj->Light->Cone = 60.0f;
        obj->Light->r = 0;
        obj->Light->g = 0;
        obj->Light->b = 512;
    }

    obj->Light2 = AllocLight();
    if (obj->Light2)
    {
        obj->Light2->x = obj->body.Centre.Pos.v[X];
        obj->Light2->y = obj->body.Centre.Pos.v[Y];
        obj->Light2->z = obj->body.Centre.Pos.v[Z];
        obj->Light2->Reach = 2500;
        obj->Light2->Flag = LIGHT_FIXED | LIGHT_MOVING;
        obj->Light2->Type= LIGHT_SPOTNORMAL;
        obj->Light2->Cone = 60.0f;
        obj->Light2->r = 512;
        obj->Light2->g = 0;
        obj->Light2->b = 0;
    }

// No collision

    obj->CollType = COLL_TYPE_NONE;
    obj->body.CollSkin.AllowObjColls = FALSE;
    obj->body.CollSkin.AllowWorldColls = FALSE;
    
// return OK

    return TRUE;
}

/////////////////////////////////////////////////////////////////////
// InitSpeedup:
/////////////////////////////////////////////////////////////////////

static long InitSpeedup(OBJECT *object, long *flags)
{
    VEC     pos;

    MAT         *objMat = &object->body.Centre.WMatrix;
    SPEEDUP_OBJ *speedup = (SPEEDUP_OBJ *)object->Data;
    NEWCOLLPOLY *collPoly = &speedup->CollPoly;

    // not if time trial
    #if defined(_N64) && defined(DEBUG)
    if (GameSettings.GameType == GAMETYPE_TRIAL)
        return FALSE;
    #endif

    // Model and rendering...
    object->DefaultModel = LoadOneLevelModel(LEVEL_MODEL_SPEEDUP, TRUE, object->renderflag, TPAGE_FX1);
    if (object->DefaultModel == -1) return FALSE;

    // AI and handlers
    object->aihandler = (AI_HANDLER)AI_SpeedupAIHandler;
    object->renderhandler = (RENDER_HANDLER)RenderSpeedup;
    object->CollType = COLL_TYPE_NONE;
    //object->body.CollSkin.AllowWorldColls = FALSE;
    //object->body.CollSkin.AllowObjectColls = FALSE:

    // Other stuff
    speedup->Width = 5.0f * (REAL)flags[0];
    speedup->LoSpeed = MPH2OGU_SPEED * (REAL)flags[1];
    speedup->HiSpeed = MPH2OGU_SPEED * (REAL)flags[2];
    speedup->Speed = speedup->LoSpeed;
    speedup->ChangeTime = (REAL)flags[3];
    speedup->Time = ZERO;
    speedup->Height = 120.0f;
    SetBBox(&collPoly->BBox, LARGEDIST, -LARGEDIST, LARGEDIST, -LARGEDIST, LARGEDIST, -LARGEDIST);

    // positions of the posts
    VecPlusScalarVec(&object->body.Centre.Pos, -speedup->Width, &objMat->mv[R], &speedup->PostPos[0]);
    VecPlusScalarVec(&object->body.Centre.Pos, speedup->Width, &objMat->mv[R], &speedup->PostPos[1]);

    // Build the collision poly
    collPoly->Plane.v[A] = objMat->m[LX];
    collPoly->Plane.v[B] = objMat->m[LY];
    collPoly->Plane.v[C] = objMat->m[LZ];
    collPoly->Plane.v[D] = -VecDotVec(&objMat->mv[L], &object->body.Centre.Pos);

    VecPlusScalarVec(&object->body.Centre.Pos, -speedup->Width, &objMat->mv[R], &pos);
    collPoly->EdgePlane[0].v[A] = -objMat->m[RX];
    collPoly->EdgePlane[0].v[B] = -objMat->m[RY];
    collPoly->EdgePlane[0].v[C] = -objMat->m[RZ];
    collPoly->EdgePlane[0].v[D] = VecDotVec(&objMat->mv[R], &pos);
    AddPointToBBox(&collPoly->BBox, &pos);

    VecPlusScalarVec(&object->body.Centre.Pos, -speedup->Height, &objMat->mv[U], &pos);
    collPoly->EdgePlane[1].v[A] = -objMat->m[UX];
    collPoly->EdgePlane[1].v[B] = -objMat->m[UY];
    collPoly->EdgePlane[1].v[C] = -objMat->m[UZ];
    collPoly->EdgePlane[1].v[D] = VecDotVec(&objMat->mv[U], &pos);
    AddPointToBBox(&collPoly->BBox, &pos);

    VecPlusScalarVec(&object->body.Centre.Pos, speedup->Width, &objMat->mv[R], &pos);
    collPoly->EdgePlane[2].v[A] = objMat->m[RX];
    collPoly->EdgePlane[2].v[B] = objMat->m[RY];
    collPoly->EdgePlane[2].v[C] = objMat->m[RZ];
    collPoly->EdgePlane[2].v[D] = -VecDotVec(&objMat->mv[R], &pos);
    AddPointToBBox(&collPoly->BBox, &pos);

    CopyVec(&object->body.Centre.Pos, &pos);
    collPoly->EdgePlane[3].v[A] = objMat->m[UX];
    collPoly->EdgePlane[3].v[B] = objMat->m[UY];
    collPoly->EdgePlane[3].v[C] = objMat->m[UZ];
    collPoly->EdgePlane[3].v[D] = -VecDotVec(&objMat->mv[U], &pos);
    AddPointToBBox(&collPoly->BBox, &pos);

    #ifndef _N64
    collPoly->Type = POLY_QUAD;
    #else
    collPoly->Type = 1;
    #endif

    collPoly->Material = MATERIAL_DEFAULT;

    return TRUE;
}


//////////////////
// init balloon //
//////////////////

static long InitBalloon(OBJECT *obj, long *flags)
{
    BALLOON_OBJ *balloon = (BALLOON_OBJ*)obj->Data;

// set ai handler

    obj->aihandler = (AI_HANDLER)AI_BalloonHandler;

// load default model

    obj->DefaultModel = LoadOneLevelModel(LEVEL_MODEL_BALLOON, TRUE, obj->renderflag, 0);
    if (obj->DefaultModel == -1) return FALSE;

// init balloon

    balloon->Time = frand(1.0f);
    balloon->Height = obj->body.Centre.Pos.v[Y];

// No collision

    obj->CollType = COLL_TYPE_NONE;
    obj->body.CollSkin.AllowObjColls = FALSE;
    obj->body.CollSkin.AllowWorldColls = FALSE;
    
// return OK

    return TRUE;
}

////////////////
// init horse //
////////////////

static long InitHorse(OBJECT *obj, long *flags)
{
    HORSE_OBJ *horse = (HORSE_OBJ*)obj->Data;

// set render flags

    obj->renderflag.reflect = FALSE;

// set ai handler

    obj->aihandler = (AI_HANDLER)AI_HorseRipper;

// load default model

    obj->DefaultModel = LoadOneLevelModel(LEVEL_MODEL_HORSE, TRUE, obj->renderflag, 0);
    if (obj->DefaultModel == -1) return FALSE;

// init horse

    horse->Time = frand(1.0f);
    horse->Mat = obj->body.Centre.WMatrix;
    horse->CreakFlag = 0.01f;

// return OK

    return TRUE;
}
#endif

////////////////
// init train //
////////////////

static long InitTrain(OBJECT *obj, long *flags)
{
    TRAIN_OBJ *train = (TRAIN_OBJ*)obj->Data;

// not if time trial
    if (GameSettings.GameType == GAMETYPE_TRIAL)
        return FALSE;

// set ai handler

    obj->aihandler = (AI_HANDLER)AI_TrainHandler;
    obj->movehandler = (MOVE_HANDLER)MOV_MoveTrain;

// set render handler

    obj->renderhandler = (RENDER_HANDLER)RenderTrain;

// load default model

    obj->DefaultModel = LoadOneLevelModel(LEVEL_MODEL_TRAIN, TRUE, obj->renderflag, 0);
    if (obj->DefaultModel == -1) return FALSE;

// load wheels

    train->BackWheel = LoadOneLevelModel(LEVEL_MODEL_TRAIN2, TRUE, obj->renderflag, 0);
    train->FrontWheel = LoadOneLevelModel(LEVEL_MODEL_TRAIN3, TRUE, obj->renderflag, 0);

// replay ID - also used to determine collision precedence

    obj->ReplayStoreInfo.ID = --MinReplayID;

// create sfx

#ifndef _PSX
 #ifdef OLD_AUDIO
    obj->Sfx3D = CreateSfx3D(SFX_TOY_TRAIN, SFX_MAX_VOL, 22050, TRUE, &obj->body.Centre.Pos, 0);
 #else // !OLD_AUDIO
    g_SoundEngine.Play3DSound( g_dwLevelSoundsOffset + EFFECT_toy_train, TRUE, obj, &obj->pSfxInstance );
    //$TODO(JHarding): need to add toy train sound
 #endif // !OLD_AUDIO
#endif

// init train

    train->SteamTime   = 0;
    train->WhistleFlag = TRUE;
    train->TimeFront   =
    train->TimeBack    = 0.f;

// init physics stuff

    obj->body.Centre.Mass = ZERO;
    obj->body.Centre.InvMass = ZERO;
    SetMat(&obj->body.BodyInertia, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO);
    SetMat(&obj->body.BodyInvInertia, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO);
    GetFrameInertia(&obj->body.BodyInvInertia, &obj->body.Centre.WMatrix, &obj->body.WorldInvInertia);

    SetVector(&obj->body.AngVel, 0, 0, 0);
    SetVector(&obj->body.Centre.Vel, 0, 0, -200.0f);

    obj->CollType = COLL_TYPE_BODY;
    obj->body.CollSkin.CollType = BODY_COLL_POLY;
    obj->body.CollSkin.AllowWorldColls = FALSE;
    obj->body.CollSkin.AllowObjColls = TRUE;

    obj->body.CollSkin.NConvex = LevelModel[obj->DefaultModel].CollSkin.NConvex;
    obj->body.CollSkin.NSpheres = LevelModel[obj->DefaultModel].CollSkin.NSpheres;
    obj->body.CollSkin.NCollPolys = LevelModel[obj->DefaultModel].CollSkin.NCollPolys;
    obj->body.CollSkin.Convex = LevelModel[obj->DefaultModel].CollSkin.Convex;
    obj->body.CollSkin.Sphere = LevelModel[obj->DefaultModel].CollSkin.Sphere;
    obj->body.CollSkin.CollPoly = LevelModel[obj->DefaultModel].CollSkin.CollPoly;
    CopyBBox(&LevelModel[obj->DefaultModel].CollSkin.TightBBox, &obj->body.CollSkin.TightBBox);

    SetBBox(&obj->body.CollSkin.TightBBox, -2000, 2000, -2000, 2000, -2000, 2000);

    CreateCopyCollSkin(&obj->body.CollSkin);
    InitWorldSkin(&obj->body.CollSkin, &obj->body.Centre.Pos, &obj->body.Centre.WMatrix);
    BuildWorldSkin(&obj->body.CollSkin, &obj->body.Centre.Pos, &obj->body.Centre.WMatrix);
    TransCollPolys(obj->body.CollSkin.WorldCollPoly, obj->body.CollSkin.NCollPolys, &obj->body.Centre.Pos);

// return OK

    return TRUE;
}

/////////////////
// init strobe //
/////////////////
#ifndef _PSX
static long InitStrobe(OBJECT *obj, long *flags)
{
    STROBE_OBJ *strobe = (STROBE_OBJ*)obj->Data;

// set render flags

    obj->renderflag.reflect = FALSE;
    obj->renderflag.envmap = FALSE;

// set ai handler

    obj->aihandler = (AI_HANDLER)AI_StrobeHandler;
    obj->renderhandler = (RENDER_HANDLER)RenderStrobe;

// load default model

    obj->DefaultModel = LoadOneLevelModel(LEVEL_MODEL_LIGHT1 + flags[0], TRUE, obj->renderflag, 0);
    if (obj->DefaultModel == -1) return FALSE;

// setup strobe

    strobe->StrobeNum = flags[1];
    strobe->StrobeCount = flags[2];

    strobe->FadeUp = StrobeTable[flags[0]].FadeUp;
    strobe->FadeDown = StrobeTable[flags[0]].FadeDown;

    strobe->Range = StrobeTable[flags[0]].Range;

    strobe->r = StrobeTable[flags[0]].rgb.r;
    strobe->g = StrobeTable[flags[0]].rgb.g;
    strobe->b = StrobeTable[flags[0]].rgb.b;

// No collision

    obj->CollType = COLL_TYPE_NONE;
    obj->body.CollSkin.AllowObjColls = FALSE;
    obj->body.CollSkin.AllowWorldColls = FALSE;
    
// setup light pos

    RotTransVector(&obj->body.Centre.WMatrix, &obj->body.Centre.Pos, &StrobeTable[flags[0]].GlowOffset, &strobe->LightPos);

// return OK

    return TRUE;
}
#endif


/////////////////////////////////////////////////////////////////////
//
// Init Spark Generator
//
/////////////////////////////////////////////////////////////////////
#ifndef _PSX
static long InitSparkGen(OBJECT *obj, long *flags)
{
    SPARK_GEN *sparkGen = (SPARK_GEN *)obj->Data;

    obj->aihandler = (AI_HANDLER)SparkGenHandler;

    sparkGen->Type = (enum SparkTypeEnum)flags[0];
    sparkGen->Parent = NULL;
    VecEqScalarVec(&sparkGen->SparkVel, (REAL)(flags[1] * 10), &obj->body.Centre.WMatrix.mv[L]);
    sparkGen->SparkVelVar = (REAL)(flags[2] * 10);
    sparkGen->MaxTime = Real(5) / Real(flags[3]);
    sparkGen->Time = 0.f;
    AddPointToBBox(&obj->body.CollSkin.BBox, &obj->body.Centre.Pos);
    sparkGen->VisiMask = SetObjectVisiMask((BOUNDING_BOX *)&obj->body.CollSkin.BBox);

// No collision

    obj->CollType = COLL_TYPE_NONE;
    obj->body.CollSkin.AllowObjColls = FALSE;
    obj->body.CollSkin.AllowWorldColls = FALSE;
    
    return TRUE;
}
#endif


/////////////////
// init strobe //
/////////////////
#ifdef _PC
static long InitSpaceman(OBJECT *obj, long *flags)
{
    SPACEMAN_OBJ *spaceman = (SPACEMAN_OBJ*)obj->Data;

// set render flags

    obj->renderflag.reflect = FALSE;

// set ai handler

    obj->aihandler = (AI_HANDLER)AI_SpacemanHandler;

// load default model

    obj->DefaultModel = LoadOneLevelModel(LEVEL_MODEL_SPACEMAN, TRUE, obj->renderflag, 0);
    if (obj->DefaultModel == -1) return FALSE;

// No collision

    obj->CollType = COLL_TYPE_NONE;
    obj->body.CollSkin.AllowObjColls = FALSE;
    obj->body.CollSkin.AllowWorldColls = FALSE;
    
// return OK

    return TRUE;
}


#endif

///////////////////////////
// init pickup generator //
///////////////////////////

long InitPickup(OBJECT *obj, long *flags)
{
    PICKUP *pickup;
    PLAYER *player;

    // Pointer to pickup structure
    pickup = AllocOnePickup();
    if (pickup == NULL) return FALSE;

    // Set the model
    pickup->DefaultModel = LoadOneLevelModel(LEVEL_MODEL_PICKUP, FALSE, pickup->RenderFlags, 0);

    if (!flags[0])
    {
        // Ordinary pickup
        pickup->Mode = PICKUP_STATE_INACTIVE;
        pickup->Clone = FALSE;
        CopyVec(&obj->body.Centre.Pos, &pickup->GenPos);
        CopyVec(&obj->body.Centre.Pos, &pickup->Pos);
    }
    else
    {
        REAL time;
        VEC relPos;
        PLANE *planePtr;

        pickup->Mode = PICKUP_STATE_GENERATING;
        pickup->Timer = 0.0f;
        pickup->Clone = TRUE;
        player = (PLAYER*)flags[0];
        VecPlusScalarVec(&obj->body.Centre.Pos, player->car.Body->CollSkin.TightBBox.ZMin - 128.0f, &obj->body.Centre.WMatrix.mv[L], &pickup->GenPos);
        pickup->GenPos.v[Y] -= 40.0f;

        // Generate inside car if too close to wall
        LineOfSightDist(&player->car.Body->Centre.Pos, &pickup->GenPos, &time, &planePtr);
        if (time == ONE) {
            LineOfSightObj(&player->car.Body->Centre.Pos, &pickup->GenPos, &time, player->ownobj);
        }
        if (time != ONE) {
            VecMinusVec(&pickup->GenPos, &player->car.Body->Centre.Pos, &relPos);
            VecPlusScalarVec(&player->car.Body->Centre.Pos, time, &relPos, &pickup->GenPos);
            VecPlusEqScalarVec(&pickup->GenPos, 30, &player->car.Body->Centre.WMatrix.mv[L]);

        }

        CopyVec(&pickup->GenPos, &pickup->Pos);
    }

    // setup collision info
    SetBBox(&pickup->BBox, 
        pickup->Pos.v[X] - 30,
        pickup->Pos.v[X] + 30,
        pickup->Pos.v[Y] - 25,
        pickup->Pos.v[Y] + 55,
        pickup->Pos.v[Z] - 30,
        pickup->Pos.v[Z] + 30);

    // No default collision
    obj->CollType = COLL_TYPE_NONE;
    obj->body.CollSkin.AllowObjColls = FALSE;
    obj->body.CollSkin.AllowWorldColls = FALSE;
    
    // return FALSE to destroy the object
    return FALSE;
}

#ifdef _PC
/////////////////////////
// init dissolve model //
/////////////////////////

static long InitDissolveModel(OBJECT *obj, long *flags)
{
    long i, j, k, l, polynum, vertnum, mem, vcount, ltime, lrgb[4];
    MODEL *model = (MODEL*)flags[0];
    DISSOLVE_OBJ *dissolve;
    DISSOLVE_PARTICLE *particle;
    MODEL_POLY *mpsource, *mpdest;
    POLY_RGB *mrgbsource, *mrgbdest;
    MODEL_RGB crgb, rgb[4];
    MODEL_VERTEX **mvsource, *mvdest, cvert, vert[4];
    REAL *uvsource, time, cuv[2], uv[8];
    VEC centre;

// get mem needed

    polynum = (model->QuadNumTex + model->QuadNumRGB) * 4 + (model->TriNumTex + model->TriNumRGB) * 3;
    vertnum = (model->QuadNumTex + model->QuadNumRGB) * 16 + (model->TriNumTex + model->TriNumRGB) * 12;

    mem = sizeof(DISSOLVE_OBJ);
    mem += sizeof(MODEL_POLY) * polynum;
    mem += sizeof(POLY_RGB) * polynum;
    mem += sizeof(MODEL_VERTEX) * vertnum;

    mem += sizeof(DISSOLVE_PARTICLE) * polynum;

    obj->Data = malloc(mem);
    if (!obj->Data) return FALSE;
    dissolve = (DISSOLVE_OBJ*)obj->Data;

// setup copy model

    memcpy(&dissolve->Model, model, sizeof(MODEL));

    dissolve->Model.PolyNum = (short)polynum;
    dissolve->Model.VertNum = (short)vertnum;

    dissolve->Model.QuadNumTex = model->QuadNumTex * 4 + model->TriNumTex * 3;
    dissolve->Model.QuadNumRGB = model->QuadNumRGB * 4 + model->TriNumRGB * 3;
    dissolve->Model.TriNumTex = 0;
    dissolve->Model.TriNumRGB = 0;

    dissolve->Model.PolyPtr = (MODEL_POLY*)(dissolve + 1);
    dissolve->Model.PolyRGB = (POLY_RGB*)(dissolve->Model.PolyPtr + dissolve->Model.PolyNum);
    dissolve->Model.VertPtr = (MODEL_VERTEX*)(dissolve->Model.PolyRGB + dissolve->Model.PolyNum);

// create new polys + verts - quads then tri's

    mpdest = dissolve->Model.PolyPtr;
    mrgbdest = dissolve->Model.PolyRGB;
    mvdest = dissolve->Model.VertPtr;

    for (l = 0 ; l < 2 ; l++)
    {
        mpsource = model->PolyPtr;
        mrgbsource = model->PolyRGB;

        for (i = 0 ; i < model->PolyNum ; i++, mpsource++, mrgbsource++)
        {
            if ((l + (mpsource->Type & POLY_QUAD)) != 1) continue;

            vcount = 3 + (mpsource->Type & POLY_QUAD);
            mvsource = &mpsource->v0;
            uvsource = &mpsource->tu0;

// get new points on poly

            ZeroMemory(&cvert, sizeof(cvert));
            cuv[0] = cuv[1] = 0.0f;
            lrgb[0] = lrgb[1] = lrgb[2] = lrgb[3] = 0;

            for (j = 0 ; j < vcount ; j++)
            {
                k = (j + 1) % vcount;
                time = frand(0.5f) + 0.25f;
                FTOL(time * 256.0f, ltime);

// get edge point

                vert[j].x = mvsource[j]->x + (mvsource[k]->x - mvsource[j]->x) * time;
                vert[j].y = mvsource[j]->y + (mvsource[k]->y - mvsource[j]->y) * time;
                vert[j].z = mvsource[j]->z + (mvsource[k]->z - mvsource[j]->z) * time;

                vert[j].nx = mvsource[j]->nx + (mvsource[k]->nx - mvsource[j]->nx) * time;
                vert[j].ny = mvsource[j]->ny + (mvsource[k]->ny - mvsource[j]->ny) * time;
                vert[j].nz = mvsource[j]->nz + (mvsource[k]->nz - mvsource[j]->nz) * time;
                NormalizeVector((VEC*)&vert[j].nx);

                uv[j * 2] = uvsource[j * 2] + (uvsource[k * 2] - uvsource[j * 2]) * time;
                uv[j * 2 + 1] = uvsource[j * 2 + 1] + (uvsource[k * 2 + 1] - uvsource[j * 2] + 1) * time;

                rgb[j].r = mrgbsource->rgb[j].r + (unsigned char)(((mrgbsource->rgb[k].r - mrgbsource->rgb[j].r) * ltime) >> 8);
                rgb[j].g = mrgbsource->rgb[j].g + (unsigned char)(((mrgbsource->rgb[k].g - mrgbsource->rgb[j].g) * ltime) >> 8);
                rgb[j].b = mrgbsource->rgb[j].b + (unsigned char)(((mrgbsource->rgb[k].b - mrgbsource->rgb[j].b) * ltime) >> 8);
                rgb[j].a = mrgbsource->rgb[j].a + (unsigned char)(((mrgbsource->rgb[k].b - mrgbsource->rgb[j].b) * ltime) >> 8);

// add to centre point

                AddVector((VEC*)&cvert.x, (VEC*)&mvsource[j]->x, (VEC*)&cvert.x);
                AddVector((VEC*)&cvert.nx, (VEC*)&mvsource[j]->nx, (VEC*)&cvert.nx);

                cuv[0] += uvsource[j * 2];
                cuv[1] += uvsource[j * 2 + 1];

                lrgb[0] += mrgbsource->rgb[j].b;
                lrgb[1] += mrgbsource->rgb[j].g;
                lrgb[2] += mrgbsource->rgb[j].r;
                lrgb[3] += mrgbsource->rgb[j].a;
            }

// normalize centre point

            cvert.x /= (float)vcount;
            cvert.y /= (float)vcount;
            cvert.z /= (float)vcount;

            NormalizeVector((VEC*)&cvert.nx);

            cuv[0] /= (float)vcount;
            cuv[1] /= (float)vcount;

            crgb.b = (unsigned char)(lrgb[0] / vcount);
            crgb.g = (unsigned char)(lrgb[1] / vcount);
            crgb.r = (unsigned char)(lrgb[2] / vcount);
            crgb.a = (unsigned char)(lrgb[3] / vcount);

// build new polys from new points

            if (mpsource->Type & POLY_QUAD)
            {
// quad 1
                mpdest->Type = (mpsource->Type | POLY_DOUBLE | POLY_SEMITRANS) & ~POLY_SEMITRANS_ONE;
                mpdest->Tpage = mpsource->Tpage;

                mpdest->tu0 = mpsource->tu0;
                mpdest->tv0 = mpsource->tv0;
                mpdest->tu1 = uv[0];
                mpdest->tv1 = uv[1];
                mpdest->tu2 = cuv[0];
                mpdest->tv2 = cuv[1];
                mpdest->tu3 = uv[6];
                mpdest->tv3 = uv[7];

                mrgbdest->rgb[0] = mrgbsource->rgb[0];
                mrgbdest->rgb[1] = rgb[0];
                mrgbdest->rgb[2] = crgb;
                mrgbdest->rgb[3] = rgb[3];

                mpdest->v0 = mvdest;
                *mvdest++ = *mpsource->v0;
                mpdest->v1 = mvdest;
                *mvdest++ = vert[0];
                mpdest->v2 = mvdest;
                *mvdest++ = cvert;
                mpdest->v3 = mvdest;
                *mvdest++ = vert[3];

                mpdest++;
                mrgbdest++;
// quad 2
                mpdest->Type = (mpsource->Type | POLY_DOUBLE | POLY_SEMITRANS) & ~POLY_SEMITRANS_ONE;
                mpdest->Tpage = mpsource->Tpage;

                mpdest->tu0 = uv[0];
                mpdest->tv0 = uv[1];
                mpdest->tu1 = mpsource->tu1;
                mpdest->tv1 = mpsource->tv1;
                mpdest->tu2 = uv[2];
                mpdest->tv2 = uv[3];
                mpdest->tu3 = cuv[0];
                mpdest->tv3 = cuv[1];

                mrgbdest->rgb[0] = rgb[0];
                mrgbdest->rgb[1] = mrgbsource->rgb[1];
                mrgbdest->rgb[2] = rgb[1];
                mrgbdest->rgb[3] = crgb;

                mpdest->v0 = mvdest;
                *mvdest++ = vert[0];
                mpdest->v1 = mvdest;
                *mvdest++ = *mpsource->v1;
                mpdest->v2 = mvdest;
                *mvdest++ = vert[1];
                mpdest->v3 = mvdest;
                *mvdest++ = cvert;

                mpdest++;
                mrgbdest++;
// quad 3
                mpdest->Type = (mpsource->Type | POLY_DOUBLE | POLY_SEMITRANS) & ~POLY_SEMITRANS_ONE;
                mpdest->Tpage = mpsource->Tpage;

                mpdest->tu0 = cuv[0];
                mpdest->tv0 = cuv[1];
                mpdest->tu1 = uv[2];
                mpdest->tv1 = uv[3];
                mpdest->tu2 = mpsource->tu2;
                mpdest->tv2 = mpsource->tv2;
                mpdest->tu3 = uv[4];
                mpdest->tv3 = uv[5];

                mrgbdest->rgb[0] = crgb;
                mrgbdest->rgb[1] = rgb[1];
                mrgbdest->rgb[2] = mrgbsource->rgb[2];
                mrgbdest->rgb[3] = rgb[2];

                mpdest->v0 = mvdest;
                *mvdest++ = cvert;
                mpdest->v1 = mvdest;
                *mvdest++ = vert[1];
                mpdest->v2 = mvdest;
                *mvdest++ = *mpsource->v2;
                mpdest->v3 = mvdest;
                *mvdest++ = vert[2];

                mpdest++;
                mrgbdest++;
// quad 4
                mpdest->Type = (mpsource->Type | POLY_DOUBLE | POLY_SEMITRANS) & ~POLY_SEMITRANS_ONE;
                mpdest->Tpage = mpsource->Tpage;

                mpdest->tu0 = uv[6];
                mpdest->tv0 = uv[7];
                mpdest->tu1 = cuv[0];
                mpdest->tv1 = cuv[1];
                mpdest->tu2 = uv[4];
                mpdest->tv2 = uv[5];
                mpdest->tu3 = mpsource->tu3;
                mpdest->tv3 = mpsource->tv3;

                mrgbdest->rgb[0] = rgb[3];
                mrgbdest->rgb[1] = crgb;
                mrgbdest->rgb[2] = rgb[2];
                mrgbdest->rgb[3] = mrgbsource->rgb[3];

                mpdest->v0 = mvdest;
                *mvdest++ = vert[3];
                mpdest->v1 = mvdest;
                *mvdest++ = cvert;
                mpdest->v2 = mvdest;
                *mvdest++ = vert[2];
                mpdest->v3 = mvdest;
                *mvdest++ = *mpsource->v3;

                mpdest++;
                mrgbdest++;
            }
            else
            {
// tri 1
                mpdest->Type = (mpsource->Type | POLY_DOUBLE | POLY_SEMITRANS | POLY_QUAD) & ~POLY_SEMITRANS_ONE;
                mpdest->Tpage = mpsource->Tpage;

                mpdest->tu0 = mpsource->tu0;
                mpdest->tv0 = mpsource->tv0;
                mpdest->tu1 = uv[0];
                mpdest->tv1 = uv[1];
                mpdest->tu2 = cuv[0];
                mpdest->tv2 = cuv[1];
                mpdest->tu3 = uv[4];
                mpdest->tv3 = uv[5];

                mrgbdest->rgb[0] = mrgbsource->rgb[0];
                mrgbdest->rgb[1] = rgb[0];
                mrgbdest->rgb[2] = crgb;
                mrgbdest->rgb[3] = rgb[2];

                mpdest->v0 = mvdest;
                *mvdest++ = *mpsource->v0;
                mpdest->v1 = mvdest;
                *mvdest++ = vert[0];
                mpdest->v2 = mvdest;
                *mvdest++ = cvert;
                mpdest->v3 = mvdest;
                *mvdest++ = vert[2];

                mpdest++;
                mrgbdest++;
// tri 2
                mpdest->Type = (mpsource->Type | POLY_DOUBLE | POLY_SEMITRANS | POLY_QUAD) & ~POLY_SEMITRANS_ONE;
                mpdest->Tpage = mpsource->Tpage;

                mpdest->tu0 = uv[0];
                mpdest->tv0 = uv[1];
                mpdest->tu1 = mpsource->tu1;
                mpdest->tv1 = mpsource->tv1;
                mpdest->tu2 = uv[2];
                mpdest->tv2 = uv[3];
                mpdest->tu3 = cuv[0];
                mpdest->tv3 = cuv[1];

                mrgbdest->rgb[0] = rgb[0];
                mrgbdest->rgb[1] = mrgbsource->rgb[1];
                mrgbdest->rgb[2] = rgb[1];
                mrgbdest->rgb[3] = crgb;

                mpdest->v0 = mvdest;
                *mvdest++ = vert[0];
                mpdest->v1 = mvdest;
                *mvdest++ = *mpsource->v1;
                mpdest->v2 = mvdest;
                *mvdest++ = vert[1];
                mpdest->v3 = mvdest;
                *mvdest++ = cvert;

                mpdest++;
                mrgbdest++;
// tri 3
                mpdest->Type = (mpsource->Type | POLY_DOUBLE | POLY_SEMITRANS | POLY_QUAD) & ~POLY_SEMITRANS_ONE;
                mpdest->Tpage = mpsource->Tpage;

                mpdest->tu0 = cuv[0];
                mpdest->tv0 = cuv[1];
                mpdest->tu1 = uv[2];
                mpdest->tv1 = uv[3];
                mpdest->tu2 = mpsource->tu2;
                mpdest->tv2 = mpsource->tv2;
                mpdest->tu3 = uv[4];
                mpdest->tv3 = uv[5];

                mrgbdest->rgb[0] = crgb;
                mrgbdest->rgb[1] = rgb[1];
                mrgbdest->rgb[2] = mrgbsource->rgb[2];
                mrgbdest->rgb[3] = rgb[2];

                mpdest->v0 = mvdest;
                *mvdest++ = cvert;
                mpdest->v1 = mvdest;
                *mvdest++ = vert[1];
                mpdest->v2 = mvdest;
                *mvdest++ = *mpsource->v2;
                mpdest->v3 = mvdest;
                *mvdest++ = vert[2];

                mpdest++;
                mrgbdest++;
            }
        }
    }

// setup particles

    particle = (DISSOLVE_PARTICLE*)(dissolve->Model.VertPtr + vertnum);
    mpsource = dissolve->Model.PolyPtr;

    for (i = 0 ; i < polynum ; i++, particle++, mpsource++)
    {
        centre.v[X] = (mpsource->v0->x + mpsource->v1->x + mpsource->v2->x + mpsource->v3->x) / 4.0f;
        centre.v[Y] = (mpsource->v0->y + mpsource->v1->y + mpsource->v2->y + mpsource->v3->y) / 4.0f;
        centre.v[Z] = (mpsource->v0->z + mpsource->v1->z + mpsource->v2->z + mpsource->v3->z) / 4.0f;

        NormalizeVector(&centre);

        particle->Vel.v[X] = centre.v[X] * (frand(96.0f) + 96.0f);
        particle->Vel.v[Y] = centre.v[Y] * frand(96.0f) - 192.0f;
        particle->Vel.v[Z] = centre.v[Z] * (frand(96.0f) + 96.0f);

        particle->Rot.v[X] = frand(1.0f) - 0.5f;
        particle->Rot.v[Y] = frand(1.0f) - 0.5f;
        particle->Rot.v[Z] = frand(1.0f) - 0.5f;
    }

// setup dissolve

    dissolve->Age = 0.0f;

    CopyMatrix(&Identity, &obj->body.Centre.WMatrix);

// set render flag

    obj->renderflag.envmap = FALSE;
    obj->renderflag.light = FALSE;
    obj->renderflag.reflect = FALSE;

// set handlers

    obj->aihandler = (AI_HANDLER)AI_DissolveModelHandler;
    obj->renderhandler = (RENDER_HANDLER)RenderDissolveModel;

// No collision

    obj->CollType = COLL_TYPE_NONE;
    obj->body.CollSkin.AllowObjColls = FALSE;
    obj->body.CollSkin.AllowWorldColls = FALSE;
    
// return OK

    return TRUE;
}


/////////////////////////////////////////////////////////////////////
//
// InitFlap:
//
/////////////////////////////////////////////////////////////////////

long InitFlap(OBJECT *obj, long *flags)
{

    return TRUE;
}


/////////////////////////////////////////////////////////////////////
//
// InitLaser:
//
/////////////////////////////////////////////////////////////////////
#endif

#ifndef _PSX
long InitLaser(OBJECT *obj, long *flags)
{
    REAL dist;
    VEC dest;
    LASER_OBJ *laser = (LASER_OBJ *)obj->Data;
    laser->Dist = 0.f;
    obj->aihandler = (AI_HANDLER)AI_LaserHandler;
    obj->renderhandler = (RENDER_HANDLER)RenderLaser;

    // Find the default destination of the laser beam
    //return FALSE;

    VecPlusScalarVec(&obj->body.Centre.Pos, 10000, &obj->body.Centre.WMatrix.mv[L], &dest);
    if (!LineOfSightDist(&obj->body.Centre.Pos, &dest, &dist, NULL)) {
#ifdef _PC
        DumpMessage("Error", "Laser goes on for ever.........");
#else
        printf("** Error : Laser goes on for ever.........\n");
#endif

    }
    VecPlusScalarVec(&obj->body.Centre.Pos, 10000 * dist, &obj->body.Centre.WMatrix.mv[L], &laser->Dest);
    VecMinusVec(&laser->Dest, &obj->body.Centre.Pos, &laser->Delta);
    SetBBox(&obj->body.CollSkin.BBox, LARGEDIST, -LARGEDIST, LARGEDIST, -LARGEDIST, LARGEDIST, -LARGEDIST);
    AddPointToBBox(&obj->body.CollSkin.BBox, &obj->body.Centre.Pos);
    AddPointToBBox(&obj->body.CollSkin.BBox, &laser->Dest);
    obj->body.CollSkin.Radius = obj->body.CollSkin.BBox.XMax - obj->body.CollSkin.BBox.XMin;
    obj->body.CollSkin.Radius = Max(obj->body.CollSkin.Radius, obj->body.CollSkin.BBox.YMax - obj->body.CollSkin.BBox.YMin);
    obj->body.CollSkin.Radius = Max(obj->body.CollSkin.Radius, obj->body.CollSkin.BBox.ZMax - obj->body.CollSkin.BBox.ZMin);

    laser->Width = (REAL)flags[0];
    laser->RandWidth = (REAL)flags[1];
    laser->Length = VecLen(&laser->Delta);
#ifdef _PC
#pragma warning(disable:4800)
#endif
    laser->ObjectCollide = (bool)flags[2];
#ifdef _PC
#pragma warning(default:4800)
#endif
    laser->Phase = (long)frand(1000);
    laser->VisiMask = SetObjectVisiMask((BOUNDING_BOX *)&obj->body.CollSkin.BBox);

// create hum sfx
#ifndef _PSX
    laser->AlarmTimer = Real(0);
    InterpVec(&obj->body.Centre.Pos, &laser->Dest, Real(0.5), &dest);
 #ifdef OLD_AUDIO
    obj->Sfx3D = CreateSfx3D(SFX_MUSE_LASER, SFX_MAX_VOL, SFX_SAMPLE_RATE, TRUE, &dest, 0);
 #else // !OLD_AUDIO
    g_SoundEngine.Play3DSound( g_dwLevelSoundsOffset + EFFECT_museum_laserhum, 
                               TRUE, 
                               dest.v[0], 
                               dest.v[1], 
                               dest.v[2], 
                               &obj->pSfxInstance );
 #endif // !OLD_AUDIO
#ifdef _N64
    obj->Sfx3D->RangeMul = .2f;
#endif
#endif
    return TRUE;

// No default collision

    obj->CollType = COLL_TYPE_NONE;
    obj->body.CollSkin.AllowObjColls = FALSE;
    obj->body.CollSkin.AllowWorldColls = FALSE;
    
}

#endif


/////////////////////
// init splash obj //
/////////////////////

long InitSplash(OBJECT *obj, long *flags)
{
    SPLASH_OBJ *splash = (SPLASH_OBJ*)obj->Data;
    SPLASH_POLY *spoly;
    long i;
    REAL rad, size, mul;
    MAT mat, mat2;
    VEC v0, v1, v2, v3, vel;

// set ai handler

    obj->aihandler = (AI_HANDLER)AI_SplashHandler;
    obj->renderhandler = (RENDER_HANDLER)RenderSplash;

// setup splash

    splash->Count = SPLASH_POLY_NUM;

// setup each poly

    spoly = splash->Poly;

    for (i = 0 ; i < SPLASH_POLY_NUM ; i++, spoly++)
    {
        spoly->Frame = 0.0f;
        spoly->FrameAdd = (frand(0.5f) + 1.0f) * 16.0f;

        RotMatrixY(&mat, frand(1.0f));
        MulMatrix(&obj->body.Centre.WMatrix, &mat, &mat2);

        rad = frand(80.0f);
        size = frand(rad * 0.666f) + 8.0f;

        SetVector(&v0, -size, -size * 4.0f, rad * 1.8f);
        SetVector(&v1, size, -size * 4.0f, rad * 1.8f);
        SetVector(&v2, size, 0.0f, rad);
        SetVector(&v3, -size, 0.0f, rad);

        RotTransVector(&mat2, &obj->body.Centre.Pos, &v0, &spoly->Pos[0]);
        RotTransVector(&mat2, &obj->body.Centre.Pos, &v1, &spoly->Pos[1]);
        RotTransVector(&mat2, &obj->body.Centre.Pos, &v2, &spoly->Pos[2]);
        RotTransVector(&mat2, &obj->body.Centre.Pos, &v3, &spoly->Pos[3]);

        SubVector(&spoly->Pos[0], &spoly->Pos[3], &vel);
        mul = (frand(256.0f) + 128.0f) / Length(&vel);
        VecMulScalar(&vel, mul);

        SetVector(&spoly->Vel[0], vel.v[X] + frand(32.0f) - 16.0f, vel.v[Y] + frand(32.0f) - 16.0f, vel.v[Z] + frand(32.0f) - 16.0f);
        SetVector(&spoly->Vel[1], vel.v[X] + frand(32.0f) - 16.0f, vel.v[Y] + frand(32.0f) - 16.0f, vel.v[Z] + frand(32.0f) - 16.0f);
        SetVector(&spoly->Vel[2], vel.v[X] + frand(32.0f) - 16.0f, vel.v[Y] + frand(32.0f) - 16.0f, vel.v[Z] + frand(32.0f) - 16.0f);
        SetVector(&spoly->Vel[3], vel.v[X] + frand(32.0f) - 16.0f, vel.v[Y] + frand(32.0f) - 16.0f, vel.v[Z] + frand(32.0f) - 16.0f);
    }

// No collision

    obj->CollType = COLL_TYPE_NONE;
    obj->body.CollSkin.AllowObjColls = FALSE;
    obj->body.CollSkin.AllowWorldColls = FALSE;
    
// return OK

    return TRUE;
}



#ifdef _N64
//
// LoadOneLevelModel
//
// Loads the specified object from the level model list
//

long LoadOneLevelModel(long id, long UseMRGBper, struct renderflags renderflag, long tpage)
{
    long    ii;
    FIL     *fp;
    long    rgbper;
    long    Flag = 0;

// look for existing model
    for (ii = 0; ii < MAX_LEVEL_MODELS ; ii++)
    {
        if (LevelModel[ii].ID == id)
        {
            LevelModel[ii].RefCount++;
            return ii;
        }
    }

    if (!LevelModelList[id].Model)
        {
        return -1;
        }

// find new slot
    for (ii = 0; ii < MAX_LEVEL_MODELS; ii++)
    {
        if (LevelModel[ii].ID == -1)
        {
// load model
            if (UseMRGBper)
                rgbper = CurrentLevelInfo.ModelRGBper;
            else
                rgbper = 100;

            if (renderflag.envmap)  Flag |= MODEL_ENV;
            if (renderflag.envonly) Flag |= MODEL_ENVONLY;
            if (renderflag.light) Flag |= MODEL_LIT;
            if (id == LEVEL_MODEL_CARBOX) Flag |= MODEL_FORCELIT;
            MOD_LoadModel(LevelModelList[id].Model, LevelModelList[id].Tex, &LevelModel[ii].Model, 0x808080, rgbper, Flag);
    
// load coll skin
            if (LevelModelList[id].Coll)
            {
                if ((fp = FFS_Open(LevelModelList[id].Coll)) != NULL) 
                {
                    if (LevelModelList[id].Flag & MODEL_COLSIMPLE)
                    {
                        LevelModel[ii].CollSkin.CollPoly = LoadNewCollPolys(fp, &LevelModel[ii].CollSkin.NCollPolys);
                    }
                    else
                    {
                        if ((LevelModel[ii].CollSkin.Convex = LoadConvex(fp, &LevelModel[ii].CollSkin.NConvex)) != NULL)
                        {
                            if ((LevelModel[ii].CollSkin.Sphere = LoadSpheres(fp, &LevelModel[ii].CollSkin.NSpheres)) != NULL) 
                            {
                                LevelModel[ii].CollSkin.CollType = BODY_COLL_CONVEX;
                                MakeTightLocalBBox(&LevelModel[ii].CollSkin);
                            }
                        }
                    }

                    FFS_Close(fp);
                }
            }

// set ID / ref count
            LevelModel[ii].ID = id;
            LevelModel[ii].RefCount = 1;
            return ii;
        }
    }

// slots full

    printf ("WARNING: NMAXMODELS already reached.\n");
    return -1;
}


///////////////////////////
// free all level models //
///////////////////////////

void FreeLevelModels(void)
{
    long i;

    for (i = 0 ; i < MAX_LEVEL_MODELS ; i++)
    {
        if (LevelModel[i].ID != -1)
        {
            LevelModel[i].RefCount = 1;
            FreeOneLevelModel(i);
        }
    }
}

//////////////////////////
// free one level model //
//////////////////////////

void FreeOneLevelModel(long slot)
{

// skip if empty

    if (LevelModel[slot].ID == -1)
        return;

// dec ref count

    LevelModel[slot].RefCount--;

// free model + coll if no owners

    if (LevelModel[slot].RefCount < 1)
    {
        MOD_FreeModel(&LevelModel[slot].Model);
        DestroyConvex(LevelModel[slot].CollSkin.Convex, LevelModel[slot].CollSkin.NConvex);
        LevelModel[slot].CollSkin.Convex = NULL;
        LevelModel[slot].CollSkin.NConvex = 0;
        DestroySpheres(LevelModel[slot].CollSkin.Sphere);
        LevelModel[slot].CollSkin.Sphere = NULL;
        LevelModel[slot].CollSkin.NSpheres = 0;
        LevelModel[slot].ID = -1;
        DestroyCollPolys(LevelModel[slot].CollSkin.CollPoly);
        LevelModel[slot].CollSkin.CollPoly = NULL;
        LevelModel[slot].CollSkin.NCollPolys = 0;
    }
}

#endif


////////////////////////////////////////////////////////////////
//
// Wobbly cone:
//
////////////////////////////////////////////////////////////////
#ifndef _PSX
long InitWeebel(OBJECT *obj, long *flags)
{

    // not if time trial
    if (GameSettings.GameType == GAMETYPE_TRIAL)
        return FALSE;

    // set default model
    obj->DefaultModel = LoadOneLevelModel(LEVEL_MODEL_WEEBEL, TRUE, obj->renderflag, 0);
    if (obj->DefaultModel == -1) return FALSE;


    // set collision handler and type
    obj->CollType = COLL_TYPE_BODY;
    obj->collhandler = (COLL_HANDLER)COL_BodyCollHandler;

    // set move handler
    //obj->movehandler = (MOVE_HANDLER)MOV_MoveBodyClever;
    obj->movehandler = (MOVE_HANDLER)MOV_MoveBody;
    obj->replayhandler = (REPLAY_HANDLER)DefaultObjectReplayStoreHandler;

    // setup remote handler
#ifdef _PC
    if (IsMultiPlayer())
    {
        obj->GlobalID = GetGlobalID(ServerID);
        InitRemoteObjectData(obj);

        if (IsServer())
        {
            obj->remotehandler = (REMOTE_HANDLER)SendObjectData;
        }
        else
        {
            obj->ServerControlled = TRUE;
        }
    }
#endif

    // Physical properties
    obj->body.Centre.Mass = Real(0.5f);
    obj->body.Centre.InvMass = ONE / Real(0.5f);
    SetMat(&obj->body.BodyInertia, Real(100), ZERO, ZERO, ZERO, Real(100), ZERO, ZERO, ZERO, Real(100));
    SetMat(&obj->body.BodyInvInertia, ONE / Real(100), ZERO, ZERO, ZERO, ONE / Real(100), ZERO, ZERO, ZERO, ONE / Real(100));
    GetFrameInertia(&obj->body.BodyInvInertia, &obj->body.Centre.WMatrix, &obj->body.WorldInvInertia);

    obj->body.Centre.Hardness = Real(0.6);
    obj->body.Centre.Resistance = Real(0.001);
    obj->body.DefaultAngRes = Real(0.005);
    obj->body.AngResistance = Real(0.005);
    obj->body.AngResMod = Real(1.0);
    obj->body.Centre.Grip = Real(0.005);
    obj->body.Centre.StaticFriction = Real(1.3);
    obj->body.Centre.KineticFriction = Real(0.8);

    // Collision skin
    SetBodyConvex(&obj->body);
    obj->body.CollSkin.AllowObjColls = TRUE;
    obj->body.CollSkin.NConvex = LevelModel[obj->DefaultModel].CollSkin.NConvex;
    obj->body.CollSkin.NSpheres = LevelModel[obj->DefaultModel].CollSkin.NSpheres;
    obj->body.CollSkin.Convex = LevelModel[obj->DefaultModel].CollSkin.Convex;
    obj->body.CollSkin.Sphere = LevelModel[obj->DefaultModel].CollSkin.Sphere;
    CreateCopyCollSkin(&obj->body.CollSkin);
    MakeTightLocalBBox(&obj->body.CollSkin);
    InitWorldSkin(&obj->body.CollSkin, &obj->body.Centre.Pos, &obj->body.Centre.WMatrix);
    BuildWorldSkin(&obj->body.CollSkin, &obj->body.Centre.Pos, &obj->body.Centre.WMatrix);

    return TRUE;
}
#endif

////////////////////////////////////////////////////////////////
//
// InitProbeLogo:
//
////////////////////////////////////////////////////////////////
#ifdef _PC
long InitProbeLogo(OBJECT *obj, long *flags)
{

    // set default model
    obj->DefaultModel = LoadOneLevelModel(LEVEL_MODEL_PROBELOGO, TRUE, obj->renderflag, 0);
    if (obj->DefaultModel == -1) return FALSE;

    obj->renderflag.envmap = FALSE;
    obj->renderflag.envgood = TRUE;
    obj->EnvRGB = 0xffffff;

    // set collision handler and type
    obj->CollType = COLL_TYPE_BODY;
    obj->collhandler = (COLL_HANDLER)COL_BodyCollHandler;

    // set move handler
    obj->movehandler = (MOVE_HANDLER)MOV_DropBody;

    // Physical properties
    obj->body.Centre.Mass = Real(1.0f);
    obj->body.Centre.InvMass = ONE / Real(1.0f);
    SetMat(&obj->body.BodyInertia, Real(2000), ZERO, ZERO, ZERO, Real(42500), ZERO, ZERO, ZERO, Real(41000));
    SetMat(&obj->body.BodyInvInertia, ONE / Real(2000), ZERO, ZERO, ZERO, ONE / Real(42500), ZERO, ZERO, ZERO, ONE / Real(41000));
    GetFrameInertia(&obj->body.BodyInvInertia, &obj->body.Centre.WMatrix, &obj->body.WorldInvInertia);

    obj->body.Centre.Hardness = Real(0.3);
    obj->body.Centre.Resistance = Real(0.005);
    obj->body.DefaultAngRes = Real(0.0);
    obj->body.AngResistance = Real(0.0);
    obj->body.AngResMod = Real(1.0);
    obj->body.Centre.Grip = Real(0.0);
    obj->body.Centre.StaticFriction = Real(0.0);
    obj->body.Centre.KineticFriction = Real(0.0);

    // Collision skin
    SetBodyConvex(&obj->body);
    obj->body.CollSkin.AllowObjColls = TRUE;
    obj->body.CollSkin.NConvex = LevelModel[obj->DefaultModel].CollSkin.NConvex;
    obj->body.CollSkin.NSpheres = LevelModel[obj->DefaultModel].CollSkin.NSpheres;
    obj->body.CollSkin.Convex = LevelModel[obj->DefaultModel].CollSkin.Convex;
    obj->body.CollSkin.Sphere = LevelModel[obj->DefaultModel].CollSkin.Sphere;
    CreateCopyCollSkin(&obj->body.CollSkin);
    MakeTightLocalBBox(&obj->body.CollSkin);
    InitWorldSkin(&obj->body.CollSkin, &obj->body.Centre.Pos, &obj->body.Centre.WMatrix);
    BuildWorldSkin(&obj->body.CollSkin, &obj->body.Centre.Pos, &obj->body.Centre.WMatrix);

    return TRUE;
}
#endif

/////////////////
// init clouds //
/////////////////
#ifdef _PC
static long InitClouds(OBJECT *obj, long *flags)
{
    long i, j;
    REAL width, height, rot;
    CLOUDS_OBJ *clouds = (CLOUDS_OBJ*)obj->Data;
    VEC vec, pos[4];
    MAT mat;

// set handlers

    obj->renderhandler = (RENDER_HANDLER)RenderClouds;

// setup each cloud

    int nAttempts = 4*CLOUD_NUM;  //$ADDITION(cprince) - to prevent infinite loop bug
    for (i = 0 ; i < CLOUD_NUM ; i++)
    {
        nAttempts--;
        if( nAttempts <= 0 )  break;

        if (!i)
            clouds->Cloud[i].Type = 3;
        else
            clouds->Cloud[i].Type = rand() % 2;

        rot = frand(0.25f) - 0.25f;

        if (!i)
        {
            width = 192.0f;
            height = 192.0f;
        }
        else
        {
            width = frand(80.0f) + 112.0f;
            if (rand() & 1) width = -width;

            height = frand(80.0f) + 64.0f;
            height *= (-rot * 2.0f + 0.5f);
        }

//      clouds->Cloud[i].Radius = (REAL)sqrt(width * width + height * height);
        clouds->Cloud[i].Radius = abs(width);
        if (height > clouds->Cloud[i].Radius) clouds->Cloud[i].Radius = height;

        SetVector(&vec, 0, 0, CLOUD_DIST);
        SetVector(&pos[0], -width, -height, CLOUD_DIST);
        SetVector(&pos[1], width , -height, CLOUD_DIST);
        SetVector(&pos[2], width ,  height, CLOUD_DIST);
        SetVector(&pos[3], -width,  height, CLOUD_DIST);

        if (!i)
        {
            #ifdef _PC
            RotMatrixZYX(&mat, -0.03f, (float)atan2(obj->body.Centre.WMatrix.m[LZ], obj->body.Centre.WMatrix.m[LX]) / RAD + 0.75f, 0);
            #else
            CopyMatrix(&obj->body.Centre.WMatrix, &mat);
            #endif

        }
        else
        {
            RotMatrixZYX(&mat, rot, frand(1.0f), 0);
        }

        RotVector(&mat, &vec, &clouds->Cloud[i].Centre);
        RotVector(&mat, &pos[0], &clouds->Cloud[i].Pos[0]);
        RotVector(&mat, &pos[1], &clouds->Cloud[i].Pos[1]);
        RotVector(&mat, &pos[2], &clouds->Cloud[i].Pos[2]);
        RotVector(&mat, &pos[3], &clouds->Cloud[i].Pos[3]);

// check pos isn't near others

        for (j = 0 ; j < i ; j++)
        {
            SubVector(&clouds->Cloud[i].Centre, &clouds->Cloud[j].Centre, &vec);
            if (Length(&vec) < clouds->Cloud[i].Radius + clouds->Cloud[j].Radius)
            {
                i--;
                break;
            }
        }
    }
#ifdef _DEBUG
    if( i < CLOUD_NUM )
    {
        char szTemp[128];
        sprintf( szTemp, "WARNING: only generated %d clouds (out of %d) before giving up.\n", i, CLOUD_NUM );
        OutputDebugString( szTemp );
    }
#endif //_DEBUG

// load cloud texture
#ifdef _PC
//$MODIFIED
//    LoadTextureClever("gfx\\clouds.bmp", TPAGE_MISC1, 256, 256, 0, FxTextureSet, TRUE);
    LoadTextureClever("D:\\gfx\\clouds.bmp", TPAGE_MISC1, 256, 256, 0, FxTextureSet, TRUE);
//$END_MODIFICATIONS
#endif

// No collision

    obj->CollType = COLL_TYPE_NONE;
    obj->body.CollSkin.AllowObjColls = FALSE;
    obj->body.CollSkin.AllowWorldColls = FALSE;
    
// return OK

    return TRUE;
}
#endif


////////////////////
// init sprinkler //
////////////////////
#ifdef _PC
static long InitSprinkler(OBJECT *obj, long *flags)
{
    SPRINKLER_OBJ *sprinkler = (SPRINKLER_OBJ*)obj->Data;
    REAL rad;

// set render flags

    obj->renderflag.reflect = FALSE;

// set handlers

    obj->movehandler = (MOVE_HANDLER)AI_SprinklerHandler;
    obj->renderhandler = (RENDER_HANDLER)RenderSprinkler;

// load models

    if (GameSettings.Level == LEVEL_GARDEN1)
    {
        sprinkler->BaseModel = LoadOneLevelModel(LEVEL_MODEL_SPRINKLER_BASE_GARDEN, TRUE, obj->renderflag, 0);
        sprinkler->HeadModel = LoadOneLevelModel(LEVEL_MODEL_SPRINKLER_HEAD_GARDEN, TRUE, obj->renderflag, 0);
    }
    else
    {
        sprinkler->BaseModel = LoadOneLevelModel(LEVEL_MODEL_SPRINKLER_BASE, TRUE, obj->renderflag, 0);
        sprinkler->HeadModel = LoadOneLevelModel(LEVEL_MODEL_SPRINKLER_HEAD, TRUE, obj->renderflag, 0);
    }

// set head pos

    RotTransVector(&obj->body.Centre.WMatrix, &obj->body.Centre.Pos, &SprinklerHeadOffset, &sprinkler->HeadPos);

// set head rot

    sprinkler->HeadRot = frand(1.0f);

// set ID

    sprinkler->ID = flags[0];
    obj->ReplayStoreInfo.ID = --MinReplayID;

// set sfx flag

    sprinkler->NextSfx = FALSE;

// set on hose flag

    //sprinkler->OnHose = FALSE;
    sprinkler->OnHoseTimer = TO_TIME(Real(1));

// set reach

    sprinkler->Reach = 1.0f;

// set sine

    sprinkler->Sine = 0.0f;

// init physics stuff

    obj->body.Centre.Mass = ZERO;
    obj->body.Centre.InvMass = ZERO;
    SetMat(&obj->body.BodyInertia, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO);
    SetMat(&obj->body.BodyInvInertia, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO);
    GetFrameInertia(&obj->body.BodyInvInertia, &obj->body.Centre.WMatrix, &obj->body.WorldInvInertia);

    SetVector(&obj->body.AngVel, 0, 0, 0);
    SetVector(&obj->body.Centre.Vel, 0, 0, 0);

    obj->CollType = COLL_TYPE_BODY;
    obj->body.CollSkin.CollType = BODY_COLL_POLY;
    obj->body.CollSkin.AllowWorldColls = FALSE;
    obj->body.CollSkin.AllowObjColls = TRUE;

    obj->body.CollSkin.NConvex = LevelModel[sprinkler->BaseModel].CollSkin.NConvex;
    obj->body.CollSkin.NSpheres = LevelModel[sprinkler->BaseModel].CollSkin.NSpheres;
    obj->body.CollSkin.NCollPolys = LevelModel[sprinkler->BaseModel].CollSkin.NCollPolys;
    obj->body.CollSkin.Convex = LevelModel[sprinkler->BaseModel].CollSkin.Convex;
    obj->body.CollSkin.Sphere = LevelModel[sprinkler->BaseModel].CollSkin.Sphere;
    obj->body.CollSkin.CollPoly = LevelModel[sprinkler->BaseModel].CollSkin.CollPoly;
    CopyBBox(&LevelModel[sprinkler->BaseModel].CollSkin.TightBBox, &obj->body.CollSkin.TightBBox);

    rad= LevelModel[sprinkler->BaseModel].Model.Radius;
    SetBBox(&obj->body.CollSkin.TightBBox, -rad, rad, -rad, rad, -rad, rad);

    CreateCopyCollSkin(&obj->body.CollSkin);
    InitWorldSkin(&obj->body.CollSkin, &obj->body.Centre.Pos, &obj->body.Centre.WMatrix);
    BuildWorldSkin(&obj->body.CollSkin, &obj->body.Centre.Pos, &obj->body.Centre.WMatrix);
    RotTransCollPolys(obj->body.CollSkin.WorldCollPoly, obj->body.CollSkin.NCollPolys, &obj->body.Centre.WMatrix, &obj->body.Centre.Pos);

// return OK

    return TRUE;
}

/////////////////////////
// init sprinkler hose //
/////////////////////////
static long InitSprinklerHose(OBJECT *obj, long *flags)
{
    SPRINKLER_HOSE_OBJ *hose = (SPRINKLER_HOSE_OBJ*)obj->Data;
    float rad;

// set render flags

    obj->renderflag.reflect = FALSE;

// set handlers

    obj->movehandler = (MOVE_HANDLER)AI_SprinklerHoseHandler;

// load model

    obj->DefaultModel = LoadOneLevelModel(LEVEL_MODEL_SPRINKLER_HOSE, TRUE, obj->renderflag, 0);
    if (obj->DefaultModel == -1) return FALSE;


// set ID

    hose->ID = flags[0];
    obj->ReplayStoreInfo.ID = --MinReplayID;

// zero sprinkler ptr

    hose->Sprinkler = NULL;

// init physics stuff

    obj->body.Centre.Mass = ZERO;
    obj->body.Centre.InvMass = ZERO;
    SetMat(&obj->body.BodyInertia, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO);
    SetMat(&obj->body.BodyInvInertia, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO);
    GetFrameInertia(&obj->body.BodyInvInertia, &obj->body.Centre.WMatrix, &obj->body.WorldInvInertia);

    SetVector(&obj->body.AngVel, 0, 0, 0);
    SetVector(&obj->body.Centre.Vel, 0, 0, 0);

    obj->CollType = COLL_TYPE_BODY;
    obj->body.CollSkin.CollType = BODY_COLL_POLY;
    obj->body.CollSkin.AllowWorldColls = FALSE;
    obj->body.CollSkin.AllowObjColls = TRUE;

    obj->body.CollSkin.NConvex = LevelModel[obj->DefaultModel].CollSkin.NConvex;
    obj->body.CollSkin.NSpheres = LevelModel[obj->DefaultModel].CollSkin.NSpheres;
    obj->body.CollSkin.NCollPolys = LevelModel[obj->DefaultModel].CollSkin.NCollPolys;
    obj->body.CollSkin.Convex = LevelModel[obj->DefaultModel].CollSkin.Convex;
    obj->body.CollSkin.Sphere = LevelModel[obj->DefaultModel].CollSkin.Sphere;
    obj->body.CollSkin.CollPoly = LevelModel[obj->DefaultModel].CollSkin.CollPoly;
    CopyBBox(&LevelModel[obj->DefaultModel].CollSkin.TightBBox, &obj->body.CollSkin.TightBBox);

    rad= LevelModel[obj->DefaultModel].Model.Radius;
    SetBBox(&obj->body.CollSkin.TightBBox, -rad, rad, -rad, rad, -rad, rad);

    CreateCopyCollSkin(&obj->body.CollSkin);
    InitWorldSkin(&obj->body.CollSkin, &obj->body.Centre.Pos, &obj->body.Centre.WMatrix);
    BuildWorldSkin(&obj->body.CollSkin, &obj->body.Centre.Pos, &obj->body.Centre.WMatrix);
    RotTransCollPolys(obj->body.CollSkin.WorldCollPoly, obj->body.CollSkin.NCollPolys, &obj->body.Centre.WMatrix, &obj->body.Centre.Pos);

// return OK

    return TRUE;
}
#endif


////////////////////////////////////////////////////////////////
//
// Object Thrower
//
////////////////////////////////////////////////////////////////
#ifndef _PSX
static long InitObjectThrower(OBJECT *obj, long *flags)
{
    OBJECT_THROWER_OBJ *objThrower = (OBJECT_THROWER_OBJ*)obj->Data;

    // not if time trial
    if (GameSettings.GameType == GAMETYPE_TRIAL)
        return FALSE;

    // init data
    objThrower->ID = flags[0];
    objThrower->ObjectType = flags[1];
    objThrower->Speed = (REAL)flags[2];
    objThrower->ReUse = flags[3];

    // No collision
    obj->CollType = COLL_TYPE_NONE;
    obj->body.CollSkin.AllowObjColls = FALSE;
    obj->body.CollSkin.AllowWorldColls = FALSE;
    
    return TRUE;
}
#endif

////////////////////////////////////////////////////////////////
//
// BasketBall
//
////////////////////////////////////////////////////////////////
#ifdef _PC
static long InitBasketBall(OBJECT *obj, long *flags)
{
    // not if time trial
    if (GameSettings.GameType == GAMETYPE_TRIAL)
        return FALSE;

    // set default model
    obj->DefaultModel = LoadOneLevelModel(LEVEL_MODEL_BASKETBALL, TRUE, obj->renderflag, 0);
    if (obj->DefaultModel == -1) return FALSE;

    // render shit, silver plait!
    obj->renderflag.shadow = TRUE;
    obj->EnvRGB = 0x404040;
    obj->EnvOffsetY = 0.1f;
    obj->EnvScale = 0.5f;

    // set collision handler and type
    obj->CollType = COLL_TYPE_BODY;
    obj->collhandler = (COLL_HANDLER)COL_BodyCollHandler;

    // set move handler
    obj->movehandler = (MOVE_HANDLER)MOV_MoveBodyClever;

    // set replay handler
#ifndef _PSX
    obj->replayhandler = (REPLAY_HANDLER)DefaultObjectReplayStoreHandler;
#endif

    // setup remote handler
#ifdef _PC
    if (IsMultiPlayer())
    {
        obj->GlobalID = GetGlobalID(ServerID);
        InitRemoteObjectData(obj);

        if (IsServer())
        {
            obj->remotehandler = (REMOTE_HANDLER)SendObjectData;
        }
        else
        {
            obj->ServerControlled = TRUE;
        }
    }
#endif

    // setup ai handler
    obj->aihandler = (AI_HANDLER)AI_BangNoiseHandler;

    // Physical properties
    obj->body.Centre.Mass = Real(0.3f);
    obj->body.Centre.InvMass = ONE / Real(0.3f);
    SetMat(&obj->body.BodyInertia, Real(270), ZERO, ZERO, ZERO, Real(270), ZERO, ZERO, ZERO, Real(270));
    SetMat(&obj->body.BodyInvInertia, ONE / Real(270), ZERO, ZERO, ZERO, ONE / Real(270), ZERO, ZERO, ZERO, ONE / Real(270));
    GetFrameInertia(&obj->body.BodyInvInertia, &obj->body.Centre.WMatrix, &obj->body.WorldInvInertia);

    obj->body.Centre.Hardness = Real(0.8);
    obj->body.Centre.Resistance = Real(0.001);
    obj->body.DefaultAngRes = Real(0.001);
    obj->body.AngResistance = Real(0.001);
    obj->body.AngResMod = Real(1.0);
    obj->body.Centre.Grip = Real(0.01);
    obj->body.Centre.StaticFriction = Real(2.0);
    obj->body.Centre.KineticFriction = Real(2.0);

    // Collision skin
    SetBodySphere(&obj->body);
#ifdef _N64
    obj->body.CollSkin.Sphere = &obj->Sphere;
#else
    obj->body.CollSkin.Sphere = (SPHERE *)malloc(sizeof(SPHERE));
#endif
    SetVecZero(&obj->body.CollSkin.Sphere[0].Pos);
    obj->body.CollSkin.Sphere[0].Radius = Real(48);
    obj->body.CollSkin.NSpheres = 1;
    CreateCopyCollSkin(&obj->body.CollSkin);
    MakeTightLocalBBox(&obj->body.CollSkin);
    InitWorldSkin(&obj->body.CollSkin, &obj->body.Centre.Pos, &obj->body.Centre.WMatrix);
    BuildWorldSkin(&obj->body.CollSkin, &obj->body.Centre.Pos, &obj->body.Centre.WMatrix);

    return TRUE;
}

#endif //_PC

#ifndef _PSX

////////////////////////////////////////////////////////////////
//
// Init Clock
//
////////////////////////////////////////////////////////////////
static long InitClock(OBJECT *obj, long *flags)
{
    OBJECT_CLOCK_OBJ *clock = (OBJECT_CLOCK_OBJ*)obj->Data;

    // set default model
    clock->BodyModel = LoadOneLevelModel(LEVEL_MODEL_CLOCKBODY, TRUE, obj->renderflag, 0);
    clock->SmallHandModel = LoadOneLevelModel(LEVEL_MODEL_CLOCKHANDSMALL, TRUE, obj->renderflag, 0);
    clock->LargeHandModel = LoadOneLevelModel(LEVEL_MODEL_CLOCKHANDLARGE, TRUE, obj->renderflag, 0);
#ifndef _N64
    clock->DiscModel = LoadOneLevelModel(LEVEL_MODEL_CLOCKDISC, TRUE, obj->renderflag, 0);
#endif
    obj->renderflag.envmap = FALSE;
    obj->renderflag.envgood = TRUE;
    obj->EnvRGB = 0xA0A0A0;

    // render handler
    obj->renderhandler = (RENDER_HANDLER)RenderClock;

    // ai handler
    obj->aihandler = (AI_HANDLER)ClockHandler;

    // Init data
    clock->LargeHandAngle = ZERO;
    clock->SmallHandAngle = ZERO;
    clock->DiscAngle = ZERO;

    // No collision
    obj->CollType = COLL_TYPE_NONE;
    obj->body.CollSkin.AllowObjColls = FALSE;
    obj->body.CollSkin.AllowWorldColls = FALSE;
    
    return TRUE;
}

#endif // !_PSX

/////////////////
// init stream //
/////////////////
#ifdef _PC

typedef struct {
    long Model;
    long rgb;
    float Scale;
    char *Tex;
} STREAM_INFO;


static STREAM_INFO StreamInfo[] =
{
    {
        LEVEL_MODEL_STREAM,
        0x404040,
        2.0f,
//$MODIFIED
//        "levels\\nhood2\\water.bmp",
        "D:\\levels\\nhood2\\water.bmp",
//$END_MODIFICATIONS
    },
    {
        LEVEL_MODEL_SHIP_POOL,
        0xffffff,
        2.0f,
//$MODIFIED
//        "levels\\ship1\\water.bmp",
        "D:\\levels\\ship1\\water.bmp",
//$END_MODIFICATIONS
    },
    {
        LEVEL_MODEL_GARDEN_WATER1,
        0x606060,
        0.2f,
//$MODIFIED
//        "levels\\ship1\\water.bmp",
        "D:\\levels\\ship1\\water.bmp",
//$END_MODIFICATIONS
    },
    {
        LEVEL_MODEL_GARDEN_WATER2,
        0x606060,
        0.2f,
//$MODIFIED
//        "levels\\ship1\\water.bmp",
        "D:\\levels\\ship1\\water.bmp",
//$END_MODIFICATIONS
    },
    {
        LEVEL_MODEL_GARDEN_WATER3,
        0xffc0c0,
        0.2f,
//$MODIFIED
//        "levels\\ship1\\water.bmp",
        "D:\\levels\\ship1\\water.bmp",
//$END_MODIFICATIONS
    },
    {
        LEVEL_MODEL_GARDEN_WATER4,
        0xffc0c0,
        0.2f,
//$MODIFIED
//        "levels\\ship1\\water.bmp",
        "D:\\levels\\ship1\\water.bmp",
//$END_MODIFICATIONS
    },
};

static long InitStream(OBJECT *obj, long *flags)
{
    long i, x, y;
    STREAM_OBJ *stream;
    MODEL *model;
    VEC *pos;
    BBOX box;

// set render flags

    obj->renderflag.reflect = FALSE;
    obj->renderflag.envmap = FALSE;

// set handlers

    obj->aihandler = (AI_HANDLER)AI_StreamHandler;
    obj->renderhandler = (RENDER_HANDLER)RenderStream;

// load default model

    obj->DefaultModel = LoadOneLevelModel(StreamInfo[flags[0]].Model, TRUE, obj->renderflag, 0);
    if (obj->DefaultModel == -1) return FALSE;
    model = &LevelModel[obj->DefaultModel].Model;

// alloc ram

    obj->Data = (STREAM_OBJ*)malloc(sizeof(STREAM_OBJ) + sizeof(STREAM_VERTEX) * (model->VertNum - 1));
    if (!obj->Data) return FALSE;

// load texture

    LoadTextureClever(StreamInfo[flags[0]].Tex, TPAGE_MISC2, 256, 256, 0, FxTextureSet, TRUE);

// setup water

    stream = (STREAM_OBJ*)obj->Data;
    stream->Scale = StreamInfo[flags[0]].Scale;
    stream->VertNum = model->VertNum;

    for (i = 0 ; i < stream->VertNum ; i++)
    {
        stream->Vert[i].Height = model->VertPtr[i].y;
        stream->Vert[i].Time = 0;
        stream->Vert[i].TotalTime = frand(2.0f) + 1.0f;

        stream->Vert[i].Uoff = (-model->VertPtr[i].nz * 5.0f) + 0.5f;
        stream->Vert[i].Voff = (-model->VertPtr[i].nx * 5.0f) + 0.5f;

        x = (long)((model->VertPtr[i].x - model->Xmin) * 0.01f + 0.5f);
        y = (long)((model->VertPtr[i].z - model->Zmin) * 0.01f + 0.5f);

        if (x > RIPPLE_TABLE_DIM - 1) x = RIPPLE_TABLE_DIM - 1;
        if (y > RIPPLE_TABLE_DIM - 1) y = RIPPLE_TABLE_DIM - 1;
    }

    pos = &obj->body.Centre.Pos;
    SetBBox(&box, pos->v[X] + model->Xmin, pos->v[X] + model->Xmax, pos->v[Y] + model->Ymin, pos->v[Y] + model->Ymax, pos->v[Z] + model->Zmin, pos->v[Z] + model->Zmax);
    stream->VisiMask = SetObjectVisiMask((BOUNDING_BOX*)&box);

// force model poly attribs

    model->QuadNumTex = 0;
    model->TriNumTex = model->PolyNum;
    model->QuadNumRGB = 0;
    model->TriNumRGB = 0;

    for (i = 0 ; i < model->PolyNum ; i++)
    {
        model->PolyPtr[i].Type = POLY_SEMITRANS | POLY_SEMITRANS_ONE | POLY_DOUBLE;
        model->PolyPtr[i].Tpage = TPAGE_MISC2;

        *(long*)&model->PolyRGB[i].rgb[0] = StreamInfo[flags[0]].rgb;
        *(long*)&model->PolyRGB[i].rgb[1] = StreamInfo[flags[0]].rgb;
        *(long*)&model->PolyRGB[i].rgb[2] = StreamInfo[flags[0]].rgb;
    }

// No collision

    obj->CollType = COLL_TYPE_NONE;
    obj->body.CollSkin.AllowObjColls = FALSE;
    obj->body.CollSkin.AllowWorldColls = FALSE;

// return OK

    return TRUE;
}
#endif

//////////////
// init cup //
//////////////

static long InitCup(OBJECT *obj, long *flags)
{
    OBJECT_CUP_OBJ *cup = (OBJECT_CUP_OBJ*)obj->Data;

    // Cup type and model
    cup->Type = flags[0];
    Assert((cup->Type >=0) && (cup->Type < 4));

    obj->DefaultModel = LoadOneLevelModel(LEVEL_MODEL_CUP1 + cup->Type, TRUE, obj->renderflag, 0);
    if (obj->DefaultModel == -1) return FALSE;

    // Render stuff
    obj->renderflag.envmap = FALSE;
    obj->renderflag.envgood = TRUE;
    obj->renderflag.reflect = TRUE;
    obj->EnvRGB = 0xffffff;
    obj->renderhandler = (RENDER_HANDLER)DrawCup;

    // No collision
    obj->CollType = COLL_TYPE_NONE;
    obj->body.CollSkin.AllowObjColls = FALSE;
    obj->body.CollSkin.AllowWorldColls = FALSE;
    
    return TRUE;
}

///////////////////
// init 3d sound //
///////////////////

#ifndef _PSX
static long Init3dSound(OBJECT *obj, long *flags)
{
 #ifdef OLD_AUDIO
    SAMPLE_3D *sample;
 #endif
    SOUND3D_OBJ *sound = (SOUND3D_OBJ*)obj->Data;

// N64 patch to remove the stream sound from NHood2 
// so just in case there is time to do the stream...
#ifdef _N64
    if (  (GameSettings.Level == LEVEL_NEIGHBOURHOOD2 )
       && (Sound3D[flags[0]]  == SFX_HOOD_STREAM      )
        )
        return FALSE;
#endif

// continuous

    if (!flags[2])
    {
 #ifdef OLD_AUDIO
        sample = CreateSfx3D(Sound3D[flags[0]], SFX_MAX_VOL, 22050, TRUE, &obj->body.Centre.Pos, 0);
        if (sample)
        {
            sample->RangeMul = (float)flags[1] * 0.1f;
        }
 #else // !OLD_AUDIO
        g_SoundEngine.Play3DSound( Sound3D[flags[0]] + g_dwLevelSoundsOffset,
                                   TRUE,
                                   obj,
                                   &obj->pSfxInstance );
 #endif // !OLD_AUDIO

        // NOTE (JHarding): This used to return FALSE, but that would just nuke the object
        return TRUE;
    }

// random

    else
    {
        obj->aihandler = (AI_HANDLER)AI_3DSoundHandler;
        obj->renderhandler = NULL;

        sound->Sfx   = Sound3D[flags[0]];
        sound->Mode  = 0;
        sound->Range = (float)flags[1] * 0.1f;
        sound->Timer = frand(SOUND_3D_MAX_WAIT);

        return TRUE;
    }
}
#endif

//////////////////////
// init star pickup //
//////////////////////
#ifndef _PSX
static long InitStar(OBJECT *obj, long *flags)
{
    long game;
    STAR_OBJ *star = (STAR_OBJ*)obj->Data;

// quit if wrong type for this game

    if (GameSettings.GameType == GAMETYPE_REPLAY)
        game = StartDataStorage.GameType;
    else
        game = GameSettings.GameType;

    if (game == GAMETYPE_TRIAL)
        return FALSE;

    if (game == GAMETYPE_PRACTICE && !flags[0])
        return FALSE;

    if (game == GAMETYPE_PRACTICE && GameSettings.Level < LEVEL_NCUP_LEVELS && IsSecretFoundPractiseStars(GameSettings.Level))
        return FALSE;

    if (game != GAMETYPE_PRACTICE && game != GAMETYPE_TRAINING && game != GAMETYPE_NETWORK_BATTLETAG && (flags[0] || !GameSettings.AllowPickups))
        return FALSE;

// set render flags

    obj->renderflag.light = FALSE;
    obj->renderflag.glare = TRUE;
    obj->EnvRGB = 0xffff80;

// set handlers

    obj->aihandler = (AI_HANDLER)AI_StarHandler;
    obj->renderhandler = (RENDER_HANDLER)RenderStar;

// set default model

    obj->DefaultModel = LoadOneLevelModel(LEVEL_MODEL_STAR, FALSE, obj->renderflag, 0);
    if (obj->DefaultModel == -1)
    {
#ifdef _PC
        DumpMessage(NULL,"Can't load star model!");
#endif
        return FALSE;
    }

// setup star

    TotalStarNum++;

    star->Mode = 2;
    obj->player = NULL;
    star->Pos = obj->body.Centre.Pos;


// setup collision info

    SetBBox(&obj->body.CollSkin.BBox, 
        obj->body.Centre.Pos.v[X] - 30,
        obj->body.Centre.Pos.v[X] + 30,
        obj->body.Centre.Pos.v[Y] - 30,
        obj->body.Centre.Pos.v[Y] + 30,
        obj->body.Centre.Pos.v[Z] - 30,
        obj->body.Centre.Pos.v[Z] + 30);


// No default collision
    obj->CollType = COLL_TYPE_NONE;
    obj->body.CollSkin.AllowObjColls = FALSE;
    obj->body.CollSkin.AllowWorldColls = FALSE;
    
// return OK

    return TRUE;
}
#endif

/////////////////////
// init fox effect //
/////////////////////
#ifndef _PSX
static long InitFox(OBJECT *obj, long *flags)
{
    long i, ram, off;
    REAL mul;
    FOX_OBJ *fox;
    MODEL *smodel, *dmodel;
    FOX_VERT *fvert;

// set render flags

    obj->renderflag.envmap = FALSE;
    obj->renderflag.light = FALSE;
    obj->renderflag.reflect = FALSE;
    obj->renderflag.meshfx = FALSE;

// setup handlers

    obj->aihandler = (AI_HANDLER)AI_FoxHandler;
    obj->renderhandler = (RENDER_HANDLER)RenderFox;

// remember owner player

    obj->player = (PLAYER*)flags[0];

// calc + alloc ram

    
    ram = sizeof(FOX_OBJ);

#ifndef _N64
    smodel = &obj->player->car.Models->Body[0];
    ram += sizeof(MODEL_POLY) * smodel->PolyNum;
    ram += sizeof(POLY_RGB) * smodel->PolyNum;
    ram += sizeof(MODEL_VERTEX) * smodel->VertNum;
    ram += sizeof(FOX_VERT) * smodel->VertNum;
#endif

    obj->Data = malloc(ram);
    if (!obj->Data)
        return FALSE;

// setup fox

    fox = (FOX_OBJ*)obj->Data;
    fox->Timer = 0.0f;
    fox->JumpFlag = 0;
    fox->FromPlayer = (PLAYER*)flags[1];

#ifndef _N64

// setup model
    dmodel = &fox->Model;

    memcpy(dmodel, smodel, sizeof(MODEL));
    dmodel->PolyPtr = (MODEL_POLY*)(fox + 1);
    dmodel->PolyRGB = (POLY_RGB*)(dmodel->PolyPtr + dmodel->PolyNum);
    dmodel->VertPtr = (MODEL_VERTEX*)(dmodel->PolyRGB + dmodel->PolyNum);

    off = (long)dmodel->VertPtr - (long)smodel->VertPtr;

    for (i = 0 ; i < dmodel->PolyNum ; i++)
    {
        dmodel->PolyPtr[i] = smodel->PolyPtr[i];

        dmodel->PolyPtr[i].Type |= POLY_SEMITRANS | POLY_SEMITRANS_ONE;
        dmodel->PolyPtr[i].Tpage = TPAGE_FX1;

        dmodel->PolyPtr[i].v0 = (MODEL_VERTEX*)((long)dmodel->PolyPtr[i].v0 + off);
        dmodel->PolyPtr[i].v1 = (MODEL_VERTEX*)((long)dmodel->PolyPtr[i].v1 + off);
        dmodel->PolyPtr[i].v2 = (MODEL_VERTEX*)((long)dmodel->PolyPtr[i].v2 + off);
        dmodel->PolyPtr[i].v3 = (MODEL_VERTEX*)((long)dmodel->PolyPtr[i].v3 + off);

        *(long*)&dmodel->PolyRGB[i].rgb[0] = 0;
        *(long*)&dmodel->PolyRGB[i].rgb[1] = 0;
        *(long*)&dmodel->PolyRGB[i].rgb[2] = 0;
        *(long*)&dmodel->PolyRGB[i].rgb[3] = 0;
    }

    for (i = 0 ; i < dmodel->VertNum ; i++)
    {
        dmodel->VertPtr[i] = smodel->VertPtr[i];

        mul = 2.0f / Length((VEC*)&dmodel->VertPtr[i].x) + 1.0f;
        VecMulScalar((VEC*)&dmodel->VertPtr[i], mul);
    }

// setup fox verts

    fvert = (FOX_VERT*)(dmodel->VertPtr + dmodel->VertNum);
    for (i = 0 ; i < dmodel->VertNum ; i++)
    {
        fvert[i].Time = frand(RAD);
        fvert[i].TimeAdd = frand(5.0f) + 1.0f;
        if (rand() & 1) fvert[i].TimeAdd = -fvert[i].TimeAdd;
    }

#endif

// setup light

    obj->Light = AllocLight();
    if (obj->Light)
    {
        obj->Light->Reach = 2048;
        obj->Light->Flag = LIGHT_FIXED | LIGHT_MOVING;
        obj->Light->Type = LIGHT_OMNI;
        obj->Light->r = 0;
        obj->Light->g = 128;
        obj->Light->b = 0;
        if (obj->player)
            CopyVec(&obj->player->car.Body->Centre.Pos, (VEC*)&obj->Light->x)
    }


// No collision

    obj->CollType = COLL_TYPE_NONE;
    obj->body.CollSkin.AllowObjColls = FALSE;
    obj->body.CollSkin.AllowWorldColls = FALSE;
    
// return OK
    return TRUE;
}
#endif

////////////////////////////////////////////////////////////////
//
// Tumbleweed
//
////////////////////////////////////////////////////////////////
#ifndef _PSX
static long InitTumbleweed(OBJECT *obj, long *flags)
{
    // not if time trial
    if (GameSettings.GameType == GAMETYPE_TRIAL)
        return FALSE;

    // set default model
    obj->DefaultModel = LoadOneLevelModel(LEVEL_MODEL_TUMBLEWEED, TRUE, obj->renderflag, 0);
    if (obj->DefaultModel == -1) return FALSE;

    // render shit, silver plait!
    obj->renderflag.envmap = FALSE;

    // set collision handler and type
    obj->CollType = COLL_TYPE_BODY;
    obj->collhandler = (COLL_HANDLER)COL_BodyCollHandler;

    // set move handler
    obj->movehandler = (MOVE_HANDLER)MOV_MoveBody;//Clever;

    // set replay handler
    obj->replayhandler = (REPLAY_HANDLER)DefaultObjectReplayStoreHandler;

    // setup remote handler
#ifdef _PC
    if (IsMultiPlayer())
    {
        obj->GlobalID = GetGlobalID(ServerID);
        InitRemoteObjectData(obj);

        if (IsServer())
        {
            obj->remotehandler = (REMOTE_HANDLER)SendObjectData;
        }
        else
        {
            obj->ServerControlled = TRUE;
        }
    }
#endif

    // setup ai handler
    obj->aihandler = (AI_HANDLER)AI_TumbleweedHandler;

    // Physical properties
    obj->body.Centre.Mass = Real(0.05f);
    obj->body.Centre.InvMass = ONE / Real(0.05f);
    SetMat(&obj->body.BodyInertia, Real(98), ZERO, ZERO, ZERO, Real(98), ZERO, ZERO, ZERO, Real(98));
    SetMat(&obj->body.BodyInvInertia, ONE / Real(98), ZERO, ZERO, ZERO, ONE / Real(98), ZERO, ZERO, ZERO, ONE / Real(98));
    GetFrameInertia(&obj->body.BodyInvInertia, &obj->body.Centre.WMatrix, &obj->body.WorldInvInertia);

    obj->body.Centre.Hardness = Real(0.1);
    obj->body.Centre.Resistance = Real(0.02);
    obj->body.DefaultAngRes = Real(0.01);
    obj->body.AngResistance = Real(0.01);
    obj->body.AngResMod = Real(1.0);
    obj->body.Centre.Grip = Real(0.01);
    obj->body.Centre.StaticFriction = Real(2.0);
    obj->body.Centre.KineticFriction = Real(2.0);

    // Collision skin
    SetBodySphere(&obj->body);
#ifdef _N64
    obj->body.CollSkin.Sphere = &obj->Sphere;
#else
    obj->body.CollSkin.Sphere = (SPHERE *)malloc(sizeof(SPHERE));
#endif
    SetVecZero(&obj->body.CollSkin.Sphere[0].Pos);
    obj->body.CollSkin.Sphere[0].Radius = Real(70);
    obj->body.CollSkin.NSpheres = 1;
    CreateCopyCollSkin(&obj->body.CollSkin);
    MakeTightLocalBBox(&obj->body.CollSkin);
    InitWorldSkin(&obj->body.CollSkin, &obj->body.Centre.Pos, &obj->body.Centre.WMatrix);
    BuildWorldSkin(&obj->body.CollSkin, &obj->body.Centre.Pos, &obj->body.Centre.WMatrix);

    return TRUE;
}
#endif

////////////////////////////////////////////////////////////////
//
// Lantern
//
////////////////////////////////////////////////////////////////
#ifndef _PSX
static long InitLantern(OBJECT *obj, long *flags)
{
    LANTERN_OBJ *lantern = (LANTERN_OBJ*)obj->Data;

    // set default model
    obj->DefaultModel = LoadOneLevelModel(LEVEL_MODEL_LANTERN, TRUE, obj->renderflag, 0);
    if (obj->DefaultModel == -1) return FALSE;

    // render shit, silver plait!
    obj->renderflag.envmap = FALSE;

    // setup handlers
    obj->aihandler = (AI_HANDLER)AI_LanternHandler;
    obj->renderhandler = (RENDER_HANDLER)RenderLantern;

    // create light
    obj->Light = AllocLight();
    if (obj->Light)
    {
        obj->Light->x = obj->body.Centre.Pos.v[X];
        obj->Light->y = obj->body.Centre.Pos.v[Y];
        obj->Light->z = obj->body.Centre.Pos.v[Z];
        obj->Light->Reach = 1000;
        obj->Light->Flag = LIGHT_FIXED | LIGHT_MOVING;
        obj->Light->Type= LIGHT_OMNINORMAL;
        obj->Light->r = 363;
        obj->Light->g = 298;
        obj->Light->b = 173;
    }

    // setup lantern
    lantern->Brightness = Real(0.5);

    // return OK
    return TRUE;
}
#endif

////////////////////////////////////////////////////////////////
//
// Skybox
//
////////////////////////////////////////////////////////////////
#ifdef _PC

static char *SkyboxFiles[][6] = {
//$MODIFIED
//    {
//        "levels\\ship1\\sky_ft.bmp",
//        "levels\\ship1\\sky_rt.bmp",
//        "levels\\ship1\\sky_bk.bmp",
//        "levels\\ship1\\sky_lt.bmp",
//        "levels\\ship1\\sky_tp.bmp",
//        "levels\\ship1\\sky_bt.bmp",
//    },
//    {
//        "levels\\ship2\\sky_ft.bmp",
//        "levels\\ship2\\sky_rt.bmp",
//        "levels\\ship2\\sky_bk.bmp",
//        "levels\\ship2\\sky_lt.bmp",
//        "levels\\ship2\\sky_tp.bmp",
//        "levels\\ship2\\sky_bt.bmp",
//    },
//    {
//        "levels\\wild_west1\\sky_ft.bmp",
//        "levels\\wild_west1\\sky_rt.bmp",
//        "levels\\wild_west1\\sky_bk.bmp",
//        "levels\\wild_west1\\sky_lt.bmp",
//        "levels\\wild_west1\\sky_tp.bmp",
//        "levels\\wild_west1\\sky_bt.bmp",
//    },
//    {
//        "levels\\nhood1\\sky_ft.bmp",
//        "levels\\nhood1\\sky_rt.bmp",
//        "levels\\nhood1\\sky_bk.bmp",
//        "levels\\nhood1\\sky_lt.bmp",
//        "levels\\nhood1\\sky_tp.bmp",
//        "levels\\nhood1\\sky_bt.bmp",
//    }
    {
        "D:\\levels\\ship1\\sky_ft.bmp",
        "D:\\levels\\ship1\\sky_rt.bmp",
        "D:\\levels\\ship1\\sky_bk.bmp",
        "D:\\levels\\ship1\\sky_lt.bmp",
        "D:\\levels\\ship1\\sky_tp.bmp",
        "D:\\levels\\ship1\\sky_bt.bmp",
    },
    {
        "D:\\levels\\ship2\\sky_ft.bmp",
        "D:\\levels\\ship2\\sky_rt.bmp",
        "D:\\levels\\ship2\\sky_bk.bmp",
        "D:\\levels\\ship2\\sky_lt.bmp",
        "D:\\levels\\ship2\\sky_tp.bmp",
        "D:\\levels\\ship2\\sky_bt.bmp",
    },
    {
        "D:\\levels\\wild_west1\\sky_ft.bmp",
        "D:\\levels\\wild_west1\\sky_rt.bmp",
        "D:\\levels\\wild_west1\\sky_bk.bmp",
        "D:\\levels\\wild_west1\\sky_lt.bmp",
        "D:\\levels\\wild_west1\\sky_tp.bmp",
        "D:\\levels\\wild_west1\\sky_bt.bmp",
    },
    {
        "D:\\levels\\nhood1\\sky_ft.bmp",
        "D:\\levels\\nhood1\\sky_rt.bmp",
        "D:\\levels\\nhood1\\sky_bk.bmp",
        "D:\\levels\\nhood1\\sky_lt.bmp",
        "D:\\levels\\nhood1\\sky_tp.bmp",
        "D:\\levels\\nhood1\\sky_bt.bmp",
    }
//$END_MODIFICATIONS
};

static long InitSkybox(OBJECT *obj, long *flags)
{
    long i;

    // load textures
    for (i = 0 ; i < 6 ; i++)
    {
        LoadTextureClever(SkyboxFiles[flags[0]][i], TPAGE_MISC3 + (char)i, 256, 256, 0, FxTextureSet, FALSE);
    }

    // turn on skybox
    Skybox = TRUE;

    // kill myself
    return FALSE;
}
#endif

////////////////////////////////////////////////////////////////
//
// Sliding doors
//
////////////////////////////////////////////////////////////////
#ifdef _PC

static long InitSlider(OBJECT *obj, long *flags)
{
    SLIDER_OBJ *slider = (SLIDER_OBJ*)obj->Data;

// set default model

    obj->DefaultModel = LoadOneLevelModel(LEVEL_MODEL_SLIDER, TRUE, obj->renderflag, 0);
    if (obj->DefaultModel == -1) return FALSE;

// setup handlers

    obj->movehandler = (MOVE_HANDLER)AI_SliderHandler;
    obj->renderhandler = (RENDER_HANDLER)RenderSlider;

// setup slider

    slider->ID = flags[0];
    CopyVec(&obj->body.Centre.Pos, &slider->Origin);

// init physics stuff

    obj->body.Centre.Mass = ZERO;
    obj->body.Centre.InvMass = ZERO;
    SetMat(&obj->body.BodyInertia, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO);
    SetMat(&obj->body.BodyInvInertia, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO);
    GetFrameInertia(&obj->body.BodyInvInertia, &obj->body.Centre.WMatrix, &obj->body.WorldInvInertia);

    SetVector(&obj->body.AngVel, 0, 0, 0);
    SetVector(&obj->body.Centre.Vel, 0, 0, 0.0f);

    obj->CollType = COLL_TYPE_BODY;
    obj->body.CollSkin.CollType = BODY_COLL_POLY;
    obj->body.CollSkin.AllowWorldColls = FALSE;
    obj->body.CollSkin.AllowObjColls = TRUE;

    obj->body.CollSkin.NConvex = LevelModel[obj->DefaultModel].CollSkin.NConvex;
    obj->body.CollSkin.NSpheres = LevelModel[obj->DefaultModel].CollSkin.NSpheres;
    obj->body.CollSkin.NCollPolys = LevelModel[obj->DefaultModel].CollSkin.NCollPolys;
    obj->body.CollSkin.Convex = LevelModel[obj->DefaultModel].CollSkin.Convex;
    obj->body.CollSkin.Sphere = LevelModel[obj->DefaultModel].CollSkin.Sphere;
    obj->body.CollSkin.CollPoly = LevelModel[obj->DefaultModel].CollSkin.CollPoly;
    CopyBBox(&LevelModel[obj->DefaultModel].CollSkin.TightBBox, &obj->body.CollSkin.TightBBox);

    SetBBox(&obj->body.CollSkin.TightBBox, -2000, 2000, -2000, 2000, -2000, 2000);

    CreateCopyCollSkin(&obj->body.CollSkin);
    InitWorldSkin(&obj->body.CollSkin, &obj->body.Centre.Pos, &obj->body.Centre.WMatrix);
    BuildWorldSkin(&obj->body.CollSkin, &obj->body.Centre.Pos, &obj->body.Centre.WMatrix);
    RotTransCollPolys(obj->body.CollSkin.WorldCollPoly, obj->body.CollSkin.NCollPolys, &obj->body.Centre.WMatrix, &obj->body.Centre.Pos);

// return OK

    return TRUE;
}
#endif

////////////////////////////////////////////////////////////////
//
// Bottle:
//
////////////////////////////////////////////////////////////////
#ifndef _PSX

static long InitBottle(OBJECT *obj, long *flags)
{
    // Don't create in multiplayer (too many packets) ot timetrial
    if ((GameSettings.GameType == GAMETYPE_TRIAL)
#ifdef _PC
        || IsMultiPlayer()
#endif
        )
    {
        return FALSE;
    }

    // set default model
    obj->DefaultModel = LoadOneLevelModel(LEVEL_MODEL_BOTTLE, TRUE, obj->renderflag, 0);
    if (obj->DefaultModel == -1) return FALSE;

    obj->renderflag.envmap = TRUE;
    obj->EnvRGB = 0x308020;
    obj->EnvScale = 0.7f;

    // set collision handler and type
    obj->CollType = COLL_TYPE_BODY;
    obj->collhandler = (COLL_HANDLER)COL_BodyCollHandler;

    // set move handler
    obj->movehandler = (MOVE_HANDLER)MOV_MoveBodyClever;

    // Replay handler
    obj->replayhandler = (REPLAY_HANDLER)DefaultObjectReplayStoreHandler;

    // setup remote handler
#ifdef _PC
    if (IsMultiPlayer())
    {
        obj->GlobalID = GetGlobalID(ServerID);
        InitRemoteObjectData(obj);

        if (IsServer())
        {
            obj->remotehandler = (REMOTE_HANDLER)SendObjectData;
        }
        else
        {
            obj->ServerControlled = TRUE;
        }
    }
#endif

    // setup ai handler
    obj->aihandler = (AI_HANDLER)AI_BangNoiseHandler;

    // Physical properties
    obj->body.Centre.Mass = Real(1.2f);
    obj->body.Centre.InvMass = ONE / Real(1.2f);
    SetMat(&obj->body.BodyInertia, Real(1500), ZERO, ZERO, ZERO, Real(500), ZERO, ZERO, ZERO, Real(1500));
    SetMat(&obj->body.BodyInvInertia, ONE / Real(1500), ZERO, ZERO, ZERO, ONE / Real(500), ZERO, ZERO, ZERO, ONE / Real(1500));
    GetFrameInertia(&obj->body.BodyInvInertia, &obj->body.Centre.WMatrix, &obj->body.WorldInvInertia);

    obj->body.Centre.Hardness = Real(0.0);
    obj->body.Centre.Resistance = Real(0.008);
    obj->body.DefaultAngRes = Real(0.001);
    obj->body.AngResistance = Real(0.002);
    obj->body.AngResMod = Real(1.0);
    obj->body.Centre.Grip = Real(0.008);
    obj->body.Centre.StaticFriction = Real(0.1);
    obj->body.Centre.KineticFriction = Real(0.05);

    if (flags[0])
        obj->body.NoMoveTime = 2 * MOVE_MAX_NOMOVETIME;

    // Collision skin
    SetBodyConvex(&obj->body);
    obj->body.CollSkin.AllowObjColls = TRUE;
    obj->body.CollSkin.HullPriority = HULL_PRIORITY_MAX;

    obj->body.CollSkin.NConvex = LevelModel[obj->DefaultModel].CollSkin.NConvex;
    obj->body.CollSkin.NSpheres = LevelModel[obj->DefaultModel].CollSkin.NSpheres;
    obj->body.CollSkin.Convex = LevelModel[obj->DefaultModel].CollSkin.Convex;
    obj->body.CollSkin.Sphere = LevelModel[obj->DefaultModel].CollSkin.Sphere;
    CreateCopyCollSkin(&obj->body.CollSkin);
    MakeTightLocalBBox(&obj->body.CollSkin);
    InitWorldSkin(&obj->body.CollSkin, &obj->body.Centre.Pos, &obj->body.Centre.WMatrix);
    BuildWorldSkin(&obj->body.CollSkin, &obj->body.Centre.Pos, &obj->body.Centre.WMatrix);

    return TRUE;
}
#endif


////////////////////////////////////////////////////////////////
//
// Bucket
//
////////////////////////////////////////////////////////////////
#ifndef _PSX

static long InitBucket(OBJECT *obj, long *flags)
{
    // Don't create in timetrial
    if (GameSettings.GameType == GAMETYPE_TRIAL)
    {
        return FALSE;
    }
    // set default model
    obj->DefaultModel = LoadOneLevelModel(LEVEL_MODEL_BUCKET, TRUE, obj->renderflag, 0);
    if (obj->DefaultModel == -1) return FALSE;

    obj->renderflag.envmap = TRUE;
    obj->EnvRGB = 0x404040;

    // set collision handler and type
    obj->CollType = COLL_TYPE_BODY;
    obj->collhandler = (COLL_HANDLER)COL_BodyCollHandler;

    // set move handler
    obj->movehandler = (MOVE_HANDLER)MOV_MoveBodyClever;

    // Replay handler
    obj->replayhandler = (REPLAY_HANDLER)DefaultObjectReplayStoreHandler;

    // setup remote handler
#ifdef _PC
    if (IsMultiPlayer())
    {
        obj->GlobalID = GetGlobalID(ServerID);
        InitRemoteObjectData(obj);

        if (IsServer())
        {
            obj->remotehandler = (REMOTE_HANDLER)SendObjectData;
        }
        else
        {
            obj->ServerControlled = TRUE;
        }
    }
#endif

    // Physical properties
    obj->body.Centre.Mass = Real(1.5f);
    obj->body.Centre.InvMass = ONE / Real(1.5f);
    SetMat(&obj->body.BodyInertia, Real(2900), ZERO, ZERO, ZERO, Real(920), ZERO, ZERO, ZERO, Real(2900));
    SetMat(&obj->body.BodyInvInertia, ONE / Real(2900), ZERO, ZERO, ZERO, ONE / Real(920), ZERO, ZERO, ZERO, ONE / Real(2900));
    GetFrameInertia(&obj->body.BodyInvInertia, &obj->body.Centre.WMatrix, &obj->body.WorldInvInertia);

    obj->body.Centre.Hardness = Real(0.0);
    obj->body.Centre.Resistance = Real(0.008);
    obj->body.DefaultAngRes = Real(0.001);
    obj->body.AngResistance = Real(0.001);
    obj->body.AngResMod = Real(1.0);
    obj->body.Centre.Grip = Real(0.008);
    obj->body.Centre.StaticFriction = Real(0.1);
    obj->body.Centre.KineticFriction = Real(0.05);

    obj->body.NoMoveTime = 2 * MOVE_MAX_NOMOVETIME;

    // Collision skin
    SetBodyConvex(&obj->body);
    obj->body.CollSkin.AllowObjColls = TRUE;
    obj->body.CollSkin.HullPriority = HULL_PRIORITY_MAX;

    obj->body.CollSkin.NConvex = LevelModel[obj->DefaultModel].CollSkin.NConvex;
    obj->body.CollSkin.NSpheres = LevelModel[obj->DefaultModel].CollSkin.NSpheres;
    obj->body.CollSkin.Convex = LevelModel[obj->DefaultModel].CollSkin.Convex;
    obj->body.CollSkin.Sphere = LevelModel[obj->DefaultModel].CollSkin.Sphere;
    CreateCopyCollSkin(&obj->body.CollSkin);
    MakeTightLocalBBox(&obj->body.CollSkin);
    InitWorldSkin(&obj->body.CollSkin, &obj->body.Centre.Pos, &obj->body.Centre.WMatrix);
    BuildWorldSkin(&obj->body.CollSkin, &obj->body.Centre.Pos, &obj->body.Centre.WMatrix);

    return TRUE;
}
#endif


////////////////////////////////////////////////////////////////
//
// Cone
//
////////////////////////////////////////////////////////////////
#ifndef _PSX

static long InitCone(OBJECT *obj, long *flags)
{
    // set default model
#ifdef _N64
    obj->DefaultModel = LoadOneLevelModel(LEVEL_MODEL_CONE, TRUE, obj->renderflag, 0);
#else
    obj->DefaultModel = LoadOneLevelModel(GameSettings.Level == LEVEL_NEIGHBOURHOOD2 ? LEVEL_MODEL_CONE2 : LEVEL_MODEL_CONE, TRUE, obj->renderflag, 0);
#endif
    if (obj->DefaultModel == -1) return FALSE;

    obj->renderflag.envmap = TRUE;
    obj->EnvRGB = 0x404040;

    // set collision handler and type
    obj->CollType = COLL_TYPE_BODY;
    obj->collhandler = (COLL_HANDLER)COL_BodyCollHandler;

    // set move handler
    obj->movehandler = (MOVE_HANDLER)MOV_MoveBodyClever;

    // Replay handler
    obj->replayhandler = (REPLAY_HANDLER)DefaultObjectReplayStoreHandler;

    // setup remote handler
#ifdef _PC
    if (IsMultiPlayer())
    {
        obj->GlobalID = GetGlobalID(ServerID);
        InitRemoteObjectData(obj);

        if (IsServer())
        {
            obj->remotehandler = (REMOTE_HANDLER)SendObjectData;
        }
        else
        {
            obj->ServerControlled = TRUE;
        }
    }
#endif

    // setup ai handler
    obj->aihandler = (AI_HANDLER)AI_BangNoiseHandler;

    // Physical properties
    obj->body.Centre.Mass = Real(1.6f);
    obj->body.Centre.InvMass = ONE / Real(1.6f);
    SetMat(&obj->body.BodyInertia, Real(2040), ZERO, ZERO, ZERO, Real(2350), ZERO, ZERO, ZERO, Real(2040));
    SetMat(&obj->body.BodyInvInertia, ONE / Real(2040), ZERO, ZERO, ZERO, ONE / Real(2350), ZERO, ZERO, ZERO, ONE / Real(2040));
    GetFrameInertia(&obj->body.BodyInvInertia, &obj->body.Centre.WMatrix, &obj->body.WorldInvInertia);

    obj->body.Centre.Hardness = Real(0.0);
    obj->body.Centre.Resistance = Real(0.008);
    obj->body.DefaultAngRes = Real(0.001);
    obj->body.AngResistance = Real(0.001);
    obj->body.AngResMod = Real(1.0);
    obj->body.Centre.Grip = Real(0.008);
    obj->body.Centre.StaticFriction = Real(0.6);
    obj->body.Centre.KineticFriction = Real(0.4);

    //obj->body.NoMoveTime = 2 * MOVE_MAX_NOMOVETIME;
    obj->body.NoMoveTime = ZERO;

    // Collision skin
    SetBodyConvex(&obj->body);
    obj->body.CollSkin.AllowObjColls = TRUE;
    obj->body.CollSkin.HullPriority = HULL_PRIORITY_MAX;

    obj->body.CollSkin.NConvex = LevelModel[obj->DefaultModel].CollSkin.NConvex;
    obj->body.CollSkin.NSpheres = LevelModel[obj->DefaultModel].CollSkin.NSpheres;
    obj->body.CollSkin.Convex = LevelModel[obj->DefaultModel].CollSkin.Convex;
    obj->body.CollSkin.Sphere = LevelModel[obj->DefaultModel].CollSkin.Sphere;
    CreateCopyCollSkin(&obj->body.CollSkin);
    MakeTightLocalBBox(&obj->body.CollSkin);
    InitWorldSkin(&obj->body.CollSkin, &obj->body.Centre.Pos, &obj->body.Centre.WMatrix);
    BuildWorldSkin(&obj->body.CollSkin, &obj->body.Centre.Pos, &obj->body.Centre.WMatrix);

    return TRUE;
}
#endif

////////////////////////////////////////////////////////////////
//
// Can: NOT IMPLEMENTED YET
//
////////////////////////////////////////////////////////////////
#ifdef _PC

static long InitCan(OBJECT *obj, long *flags)
{
    // Don't create in multiplayer (too many packets) ot timetrial
    if ((GameSettings.GameType == GAMETYPE_TRIAL)
#ifdef _PC
        || IsMultiPlayer()
#endif
        )
    {
        return FALSE;
    }

    // set default model
    obj->DefaultModel = LoadOneLevelModel(LEVEL_MODEL_CAN, TRUE, obj->renderflag, 0);
    if (obj->DefaultModel == -1) return FALSE;

    obj->renderflag.envmap = TRUE;
    obj->EnvRGB = 0xffffff;

    // set collision handler and type
    obj->CollType = COLL_TYPE_BODY;
    obj->collhandler = (COLL_HANDLER)COL_BodyCollHandler;

    // set move handler
    obj->movehandler = (MOVE_HANDLER)MOV_MoveBodyClever;

    // Replay handler
    obj->replayhandler = (REPLAY_HANDLER)DefaultObjectReplayStoreHandler;

    // setup remote handler
#ifdef _PC
    if (IsMultiPlayer())
    {
        obj->GlobalID = GetGlobalID(ServerID);
        InitRemoteObjectData(obj);

        if (IsServer())
        {
            obj->remotehandler = (REMOTE_HANDLER)SendObjectData;
        }
        else
        {
            obj->ServerControlled = TRUE;
        }
    }
#endif

    // Physical properties
    obj->body.Centre.Mass = Real(0.6f);
    obj->body.Centre.InvMass = ONE / Real(0.6f);
    SetMat(&obj->body.BodyInertia, Real(380), ZERO, ZERO, ZERO, Real(120), ZERO, ZERO, ZERO, Real(380));
    SetMat(&obj->body.BodyInvInertia, ONE / Real(380), ZERO, ZERO, ZERO, ONE / Real(120), ZERO, ZERO, ZERO, ONE / Real(380));
    GetFrameInertia(&obj->body.BodyInvInertia, &obj->body.Centre.WMatrix, &obj->body.WorldInvInertia);

    obj->body.Centre.Hardness = Real(0.0);
    obj->body.Centre.Resistance = Real(0.008);
    obj->body.DefaultAngRes = Real(0.001);
    obj->body.AngResistance = Real(0.001);
    obj->body.AngResMod = Real(1.0);
    obj->body.Centre.Grip = Real(0.008);
    obj->body.Centre.StaticFriction = Real(0.1);
    obj->body.Centre.KineticFriction = Real(0.05);

    obj->body.NoMoveTime = 2 * MOVE_MAX_NOMOVETIME;

    // Collision skin
    SetBodyConvex(&obj->body);
    obj->body.CollSkin.AllowObjColls = TRUE;
    obj->body.CollSkin.HullPriority = HULL_PRIORITY_MAX;
    obj->body.Stacked = TRUE;

    obj->body.CollSkin.NConvex = LevelModel[obj->DefaultModel].CollSkin.NConvex;
    obj->body.CollSkin.NSpheres = LevelModel[obj->DefaultModel].CollSkin.NSpheres;
    obj->body.CollSkin.Convex = LevelModel[obj->DefaultModel].CollSkin.Convex;
    obj->body.CollSkin.Sphere = LevelModel[obj->DefaultModel].CollSkin.Sphere;
    CreateCopyCollSkin(&obj->body.CollSkin);
    MakeTightLocalBBox(&obj->body.CollSkin);
    InitWorldSkin(&obj->body.CollSkin, &obj->body.Centre.Pos, &obj->body.Centre.WMatrix);
    BuildWorldSkin(&obj->body.CollSkin, &obj->body.Centre.Pos, &obj->body.Centre.WMatrix);

    return TRUE;
}
#endif

////////////////////////////////////////////////////////////////
//
// Lilo: NOT IMPLEMENTED YET
//
////////////////////////////////////////////////////////////////
#ifdef _PC

static long InitLilo(OBJECT *obj, long *flags)
{
    // set default model
    obj->DefaultModel = LoadOneLevelModel(LEVEL_MODEL_LILO, TRUE, obj->renderflag, 0);
    if (obj->DefaultModel == -1) return FALSE;

    obj->renderflag.envmap = TRUE;
    obj->EnvRGB = 0xffffff;

    // set collision handler and type
    obj->CollType = COLL_TYPE_BODY;
    obj->collhandler = (COLL_HANDLER)COL_BodyCollHandler;

    // set move handler
    obj->movehandler = (MOVE_HANDLER)MOV_MoveBodyClever;

    // Replay handler
    obj->replayhandler = (REPLAY_HANDLER)DefaultObjectReplayStoreHandler;

    // setup remote handler
#ifdef _PC
    if (IsMultiPlayer())
    {
        obj->GlobalID = GetGlobalID(ServerID);
        InitRemoteObjectData(obj);

        if (IsServer())
        {
            obj->remotehandler = (REMOTE_HANDLER)SendObjectData;
        }
        else
        {
            obj->ServerControlled = TRUE;
        }
    }
#endif

    // Physical properties
    obj->body.Centre.Mass = Real(0.6f);
    obj->body.Centre.InvMass = ONE / Real(0.6f);
    SetMat(&obj->body.BodyInertia, Real(380), ZERO, ZERO, ZERO, Real(120), ZERO, ZERO, ZERO, Real(380));
    SetMat(&obj->body.BodyInvInertia, ONE / Real(380), ZERO, ZERO, ZERO, ONE / Real(120), ZERO, ZERO, ZERO, ONE / Real(380));
    GetFrameInertia(&obj->body.BodyInvInertia, &obj->body.Centre.WMatrix, &obj->body.WorldInvInertia);

    obj->body.Centre.Hardness = Real(0.0);
    obj->body.Centre.Resistance = Real(0.008);
    obj->body.DefaultAngRes = Real(0.001);
    obj->body.AngResistance = Real(0.001);
    obj->body.AngResMod = Real(1.0);
    obj->body.Centre.Grip = Real(0.008);
    obj->body.Centre.StaticFriction = Real(0.1);
    obj->body.Centre.KineticFriction = Real(0.05);

    obj->body.NoMoveTime = 2 * MOVE_MAX_NOMOVETIME;

    // Collision skin
    SetBodyConvex(&obj->body);
    obj->body.CollSkin.AllowObjColls = TRUE;
    obj->body.CollSkin.HullPriority = HULL_PRIORITY_MAX;

    obj->body.CollSkin.NConvex = LevelModel[obj->DefaultModel].CollSkin.NConvex;
    obj->body.CollSkin.NSpheres = LevelModel[obj->DefaultModel].CollSkin.NSpheres;
    obj->body.CollSkin.Convex = LevelModel[obj->DefaultModel].CollSkin.Convex;
    obj->body.CollSkin.Sphere = LevelModel[obj->DefaultModel].CollSkin.Sphere;
    CreateCopyCollSkin(&obj->body.CollSkin);
    MakeTightLocalBBox(&obj->body.CollSkin);
    InitWorldSkin(&obj->body.CollSkin, &obj->body.Centre.Pos, &obj->body.Centre.WMatrix);
    BuildWorldSkin(&obj->body.CollSkin, &obj->body.Centre.Pos, &obj->body.Centre.WMatrix);

    return TRUE;
}
#endif

////////////////////////////////////////////////////////////////
//
// Global weapon
//
////////////////////////////////////////////////////////////////

static long InitGlobal(OBJECT *obj, long *flags)
{
    PLAYER *player = (PLAYER*)flags[0];

    // electropulse everyone if i'm local or cpu
    if (player->type == PLAYER_LOCAL || player->type == PLAYER_CPU)
    {
        ElectroPulseTheWorld(player->Slot);

        // send 'pulse everyone' message to others if multiplayer
#ifdef _PC
        if ( IsMultiPlayer() )
        {
            SendElectroPulseTheWorld(player->Slot);
        }
#endif
    }

    // kill myself
    return FALSE;
}

////////////////////////////////////////////////////////////////
//
// Rain obj
//
////////////////////////////////////////////////////////////////
#ifdef _PC

static long InitRain(OBJECT *obj, long *flags)
{
    long i;
    VERTEX_TEX0 *vert;
    RAIN_OBJ *rain = (RAIN_OBJ*)obj->Data;
    RAINDROP *raindrop;

// setup handlers

    obj->aihandler = (AI_HANDLER)AI_RainHandler;
    obj->renderhandler = (RENDER_HANDLER)RenderRain;

// setup rain

    raindrop = rain->Drop;
    for (i = 0 ; i < RAINDROP_NUM ; i++, raindrop++)
    {
        raindrop->Mode = RAINDROP_SLEEP;
        raindrop->Timer = (REAL)i / RAINDROP_NUM * 0.5f;
    }

// setup draw verts

    vert = rain->Vert;
    for (i = 0 ; i < RAINDROP_NUM ; i++)
    {
        vert->sx = -100.0f;
        vert->sy = -100.0f;
        vert->sz = -1.0f;
        vert->color = 0x00000000;
        vert++;

        vert->sx = -100.0f;
        vert->sy = -100.0f;
        vert->sz = -1.0f;
        vert->color = 0xff606060;
        vert++;
    }

// return OK

    return TRUE;
}
#endif

////////////////////////////////////////////////////////////////
//
// Lightning obj
//
////////////////////////////////////////////////////////////////
#ifndef _PSX

static long InitLightning(OBJECT *obj, long *flags)
{
    LIGHTNING_OBJ *lightning = (LIGHTNING_OBJ*)obj->Data;

// setup handlers

    obj->aihandler = (AI_HANDLER)AI_LightningHandler;

#ifdef _PC
    obj->renderhandler = (RENDER_HANDLER)RenderLightning;
#else
    obj->renderhandler = NULL;
#endif

// setup lightning

    lightning->Mode = 1;
    lightning->Timer = Real(0);

// return ok

    return TRUE;
}
#endif

////////////////////////////////////////////////////////////////
//
// Ship Light obj
//
////////////////////////////////////////////////////////////////
#ifdef _PC

static long InitShipLight(OBJECT *obj, long *flags)
{

    // set default model
    obj->DefaultModel = LoadOneLevelModel(LEVEL_MODEL_SHIPLIGHT, TRUE, obj->renderflag, 0);
    if (obj->DefaultModel == -1) return FALSE;

    // render shit, silver plait!
    obj->renderflag.envmap = FALSE;

    // setup handlers
    obj->aihandler = (AI_HANDLER)AI_ShipLightHandler;
    obj->renderhandler = (RENDER_HANDLER)RenderShipLight;

    // create light
    obj->Light = AllocLight();
    if (obj->Light)
    {
        obj->Light->x = obj->body.Centre.Pos.v[X];
        obj->Light->y = obj->body.Centre.Pos.v[Y];
        obj->Light->z = obj->body.Centre.Pos.v[Z];
        obj->Light->Reach = 1500;
        obj->Light->Flag = LIGHT_FIXED | LIGHT_MOVING;
        obj->Light->Type= LIGHT_OMNINORMAL;
        obj->Light->r = 363;
        obj->Light->g = 298;
        obj->Light->b = 173;
    }

// return ok

    return TRUE;
}
#endif


////////////////////////////////////////////////////////////////
//
// Packet
//
////////////////////////////////////////////////////////////////
#ifndef _PSX

static long InitPacket(OBJECT *obj, long *flags)
{
    // Don't create in multiplayer (too many packets) ot timetrial
    if ((GameSettings.GameType == GAMETYPE_TRIAL)
#ifdef _PC
        || IsMultiPlayer()
#endif
        )
    {
        return FALSE;
    }

    // set default model
#ifdef _N64
    obj->DefaultModel = LoadOneLevelModel(LEVEL_MODEL_PACKET, TRUE, obj->renderflag, 0);
#else
    obj->DefaultModel = LoadOneLevelModel(GameSettings.Level == LEVEL_SUPERMARKET2 ? LEVEL_MODEL_PACKET1 : LEVEL_MODEL_PACKET, TRUE, obj->renderflag, 0);
#endif
    if (obj->DefaultModel == -1) return FALSE;

    obj->renderflag.envmap = TRUE;
    obj->EnvRGB = 0x404040;

    // set collision handler and type
    obj->CollType = COLL_TYPE_BODY;
    obj->collhandler = (COLL_HANDLER)COL_BodyCollHandler;

    // set move handler
    obj->movehandler = (MOVE_HANDLER)MOV_MoveBodyClever;

    // Replay handler
    obj->replayhandler = (REPLAY_HANDLER)DefaultObjectReplayStoreHandler;

    // setup remote handler
#ifdef _PC
    if (IsMultiPlayer())
    {
        obj->GlobalID = GetGlobalID(ServerID);
        InitRemoteObjectData(obj);

        if (IsServer())
        {
            obj->remotehandler = (REMOTE_HANDLER)SendObjectData;
        }
        else
        {
            obj->ServerControlled = TRUE;
        }
    }
#endif

    // setup ai handler
    obj->aihandler = (AI_HANDLER)AI_BangNoiseHandler;

    // Physical properties
    obj->body.Centre.Mass = Real(1.6f);
    obj->body.Centre.InvMass = ONE / Real(1.6f);
    SetMat(&obj->body.BodyInertia, Real(3270), ZERO, ZERO, ZERO, Real(1300), ZERO, ZERO, ZERO, Real(3270));
    SetMat(&obj->body.BodyInvInertia, ONE / Real(3270), ZERO, ZERO, ZERO, ONE / Real(1300), ZERO, ZERO, ZERO, ONE / Real(3270));
    GetFrameInertia(&obj->body.BodyInvInertia, &obj->body.Centre.WMatrix, &obj->body.WorldInvInertia);

    obj->body.Centre.Hardness = Real(0.0);
    obj->body.Centre.Resistance = Real(0.008);
    obj->body.DefaultAngRes = Real(0.001);
    obj->body.AngResistance = Real(0.001);
    obj->body.AngResMod = Real(1.0);
    obj->body.Centre.Grip = Real(0.008);
    obj->body.Centre.StaticFriction = Real(0.6);
    obj->body.Centre.KineticFriction = Real(0.4);

    obj->body.NoMoveTime = 2 * MOVE_MAX_NOMOVETIME;

    // Collision skin
    SetBodyConvex(&obj->body);
    obj->body.CollSkin.AllowObjColls = TRUE;
    obj->body.CollSkin.HullPriority = HULL_PRIORITY_MAX;
    obj->body.Stacked = TRUE;

    obj->body.CollSkin.NConvex = LevelModel[obj->DefaultModel].CollSkin.NConvex;
    obj->body.CollSkin.NSpheres = LevelModel[obj->DefaultModel].CollSkin.NSpheres;
    obj->body.CollSkin.Convex = LevelModel[obj->DefaultModel].CollSkin.Convex;
    obj->body.CollSkin.Sphere = LevelModel[obj->DefaultModel].CollSkin.Sphere;
    CreateCopyCollSkin(&obj->body.CollSkin);
    MakeTightLocalBBox(&obj->body.CollSkin);
    InitWorldSkin(&obj->body.CollSkin, &obj->body.Centre.Pos, &obj->body.Centre.WMatrix);
    BuildWorldSkin(&obj->body.CollSkin, &obj->body.Centre.Pos, &obj->body.Centre.WMatrix);

    return TRUE;
}
#endif

////////////////////////////////////////////////////////////////
//
// ABC Block
//
////////////////////////////////////////////////////////////////
#ifndef _PSX

static long InitABC(OBJECT *obj, long *flags)
{
    // Don't create in multiplayer (too many packets) ot timetrial
    if ((GameSettings.GameType == GAMETYPE_TRIAL)
#ifdef _PC
        || IsMultiPlayer()
#endif
        )
    {
        return FALSE;
    }
    
    // set default model
    obj->DefaultModel = LoadOneLevelModel(LEVEL_MODEL_ABC, TRUE, obj->renderflag, 0);
    if (obj->DefaultModel == -1) return FALSE;

    obj->renderflag.envmap = TRUE;
    obj->EnvRGB = 0x404040;

    // set collision handler and type
    obj->CollType = COLL_TYPE_BODY;
    obj->collhandler = (COLL_HANDLER)COL_BodyCollHandler;

    // set move handler
    obj->movehandler = (MOVE_HANDLER)MOV_MoveBodyClever;

    // Replay handler
    obj->replayhandler = (REPLAY_HANDLER)DefaultObjectReplayStoreHandler;

    // setup remote handler
#ifdef _PC
    if (IsMultiPlayer())
    {
        obj->GlobalID = GetGlobalID(ServerID);
        InitRemoteObjectData(obj);

        if (IsServer())
        {
            obj->remotehandler = (REMOTE_HANDLER)SendObjectData;
        }
        else
        {
            obj->ServerControlled = TRUE;
        }
    }
#endif

    // setup ai handler
    obj->aihandler = (AI_HANDLER)AI_BangNoiseHandler;

    // Physical properties
    obj->body.Centre.Mass = Real(0.8f);
    obj->body.Centre.InvMass = ONE / Real(0.8f);
    SetMat(&obj->body.BodyInertia, Real(600), ZERO, ZERO, ZERO, Real(600), ZERO, ZERO, ZERO, Real(600));
    SetMat(&obj->body.BodyInvInertia, ONE / Real(600), ZERO, ZERO, ZERO, ONE / Real(600), ZERO, ZERO, ZERO, ONE / Real(600));
    GetFrameInertia(&obj->body.BodyInvInertia, &obj->body.Centre.WMatrix, &obj->body.WorldInvInertia);

    obj->body.Centre.Hardness = Real(0.0);
    obj->body.Centre.Resistance = Real(0.008);
    obj->body.DefaultAngRes = Real(0.001);
    obj->body.AngResistance = Real(0.001);
    obj->body.AngResMod = Real(1.0);
    obj->body.Centre.Grip = Real(0.008);
    obj->body.Centre.StaticFriction = Real(0.6);
    obj->body.Centre.KineticFriction = Real(0.4);

    obj->body.NoMoveTime = 2 * MOVE_MAX_NOMOVETIME;

    // Collision skin
    SetBodyConvex(&obj->body);

    obj->body.CollSkin.AllowObjColls = TRUE;
    obj->body.CollSkin.HullPriority = HULL_PRIORITY_MAX;
    obj->body.Stacked = TRUE;

    obj->body.CollSkin.NConvex = LevelModel[obj->DefaultModel].CollSkin.NConvex;
    obj->body.CollSkin.NSpheres = LevelModel[obj->DefaultModel].CollSkin.NSpheres;
    obj->body.CollSkin.Convex = LevelModel[obj->DefaultModel].CollSkin.Convex;
    obj->body.CollSkin.Sphere = LevelModel[obj->DefaultModel].CollSkin.Sphere;
    CreateCopyCollSkin(&obj->body.CollSkin);
    MakeTightLocalBBox(&obj->body.CollSkin);
    InitWorldSkin(&obj->body.CollSkin, &obj->body.Centre.Pos, &obj->body.Centre.WMatrix);
    BuildWorldSkin(&obj->body.CollSkin, &obj->body.Centre.Pos, &obj->body.Centre.WMatrix);

    return TRUE;
}
#endif


////////////////////////////////////////////////////////////////
//
// Water box
//
////////////////////////////////////////////////////////////////
#ifdef _PC

static long InitWaterBox(OBJECT *obj, long *flags)
{
    WATERBOX_OBJ *wb = (WATERBOX_OBJ*)obj->Data;

// handlers

    obj->aihandler = (AI_HANDLER)AI_WaterBoxHandler;
    obj->renderhandler = (RENDER_HANDLER)RenderWaterBox;

// setup waterbox obj

    wb->Box.Xmin = obj->body.Centre.Pos.v[X] - (REAL)flags[0];
    wb->Box.Xmax = obj->body.Centre.Pos.v[X] + (REAL)flags[0];
    wb->Box.Ymin = obj->body.Centre.Pos.v[Y] - (REAL)flags[1];
    wb->Box.Ymax = obj->body.Centre.Pos.v[Y] + (REAL)flags[1];
    wb->Box.Zmin = obj->body.Centre.Pos.v[Z] - (REAL)flags[2];
    wb->Box.Zmax = obj->body.Centre.Pos.v[Z] + (REAL)flags[2];

// No collision

    obj->CollType = COLL_TYPE_NONE;
    obj->body.CollSkin.AllowObjColls = FALSE;
    obj->body.CollSkin.AllowWorldColls = FALSE;

// return OK

    return TRUE;
}
#endif

/////////////////
// init ripple //
/////////////////
#ifdef _PC

typedef struct {
    long Model;
    long Tpage;
    long Master;
    long OffsetX, OffsetY;
} RIPPLE_INFO;

static RIPPLE_INFO RippleInfo[] = {
    {
        LEVEL_MODEL_STREAM,
        TPAGE_MISC2,
        TRUE, 0, 0,
    },
    {
        LEVEL_MODEL_SHIP_POOL,
        TPAGE_MISC2,
        TRUE, 0, 0,
    },
    {
        LEVEL_MODEL_GARDEN_WATER1,
        TPAGE_MISC2,
        TRUE, 0, 0,
    },
    {
        LEVEL_MODEL_GARDEN_WATER2,
        TPAGE_MISC2,
        FALSE, 0, 96,
    },
    {
        LEVEL_MODEL_GARDEN_WATER3,
        TPAGE_MISC3,
        FALSE, 64, 0,
    },
    {
        LEVEL_MODEL_GARDEN_WATER4,
        TPAGE_MISC3,
        TRUE, 0, 0,
    },
};

static long InitRipple(OBJECT *obj, long *flags)
{
    long i;
//  long i, x, y;
    REAL xdim, zdim, scale;
    RIPPLE_OBJ *ripple = (RIPPLE_OBJ*)obj->Data;
    MODEL *model;
    VEC *pos, vec1, vec2;
//  DDSURFACEDESC2 ddsd2;
//  HRESULT r;

//$REMOVED
//// quit if no precedural textures
//
//    if (!TexFormatProcedural.dwFlags)
//        return FALSE;
//$END_REMOVAL

// set render flags

    obj->renderflag.reflect = FALSE;
    obj->renderflag.envmap = FALSE;

// set handlers

    obj->aihandler = (AI_HANDLER)AI_RippleHandler;
    obj->renderhandler = (AI_HANDLER)RenderRipple;

// load model

    obj->DefaultModel = LoadOneLevelModel(RippleInfo[flags[0]].Model, TRUE, obj->renderflag, 0);
    if (obj->DefaultModel == -1) return FALSE;
    model = &LevelModel[obj->DefaultModel].Model;

// setup ripple

    xdim = (model->Xmax - model->Xmin);
    zdim = (model->Zmax - model->Zmin);

    if (xdim > zdim) scale = 1.0f / xdim;
    else scale = 1.0f / zdim;

    ripple->Width = (long)(xdim * scale * RIPPLE_TABLE_DIM);
    ripple->Height = (long)(zdim * scale * RIPPLE_TABLE_DIM);
    ripple->Scale = scale * RIPPLE_TABLE_DIM;

    ripple->Timer = 0.0f;
    ripple->Tpage = RippleInfo[flags[0]].Tpage;
    ripple->Master = RippleInfo[flags[0]].Master;
    ripple->OffsetX = RippleInfo[flags[0]].OffsetX;
    ripple->OffsetY = RippleInfo[flags[0]].OffsetY;

    ripple->WaterTableCurrent = ripple->WaterTable1;
    ripple->WaterTableLast = ripple->WaterTable2;

    pos = &obj->body.Centre.Pos;
    SetBBox(&ripple->Box, pos->v[X] + model->Xmin, pos->v[X] + model->Xmax, pos->v[Y] + model->Ymin, pos->v[Y] + model->Ymax, pos->v[Z] + model->Zmin, pos->v[Z] + model->Zmax);
    ripple->VisiMask = SetObjectVisiMask((BOUNDING_BOX*)&ripple->Box);
    ripple->Box.XMin -= CAR_RADIUS;
    ripple->Box.XMax += CAR_RADIUS;
    ripple->Box.YMin -= CAR_RADIUS;
    ripple->Box.YMax += CAR_RADIUS;
    ripple->Box.ZMin -= CAR_RADIUS;
    ripple->Box.ZMax += CAR_RADIUS;

    SetVector(&vec1, model->Xmin, model->Ymin, model->Zmin);
    RotTransVector(&obj->body.Centre.WMatrix, &obj->body.Centre.Pos, &vec1, &vec2);

    CopyVec(&obj->body.Centre.WMatrix.mv[X], (VEC*)&ripple->PlaneX);
    ripple->PlaneX.v[D] = -DotProduct((VEC*)&ripple->PlaneX, &vec2);

    CopyVec(&obj->body.Centre.WMatrix.mv[Z], (VEC*)&ripple->PlaneZ);
    ripple->PlaneZ.v[D] = -DotProduct((VEC*)&ripple->PlaneZ, &vec2);

    for (i = 0 ; i < RIPPLE_TABLE_DIM * RIPPLE_TABLE_DIM ; i++)
    {
        ripple->WaterTable1[i] = 0;
        ripple->WaterTable2[i] = 0;
    }

    ripple->Dolphin = (RippleInfo[flags[0]].Model == LEVEL_MODEL_GARDEN_WATER1);
    ripple->DolphinCount = 0;

#ifndef XBOX_NOT_YET_IMPLEMENTED
// create procedural texture

    if (ripple->Master)
    {
        CreateProceduralTexture((char)ripple->Tpage, RIPPLE_TABLE_DIM, RIPPLE_TABLE_DIM);

        ddsd2.dwSize = sizeof(ddsd2);

        r = TexInfo[ripple->Tpage].SourceSurface->Lock(NULL, &ddsd2, DDLOCK_WAIT, NULL);
        if (r != DD_OK)
        {
            ErrorDX(r, "Can't lock procedural texture source surface");
        }

//$REMOVED
//        if (TexFormatProcedural.dwRGBBitCount == 8)
//        {
//            unsigned char *ptr = (unsigned char*)ddsd2.lpSurface;
//            for (y = 0 ; y < RIPPLE_TABLE_DIM ; y++)
//            {
//                for (x = 0 ; x < RIPPLE_TABLE_DIM ; x++)
//                {
//                    ptr[x] = 128;
//                }
//                ptr += ddsd2.lPitch;
//            }
//        }
//        else
//$END_REMOVAL
        {
            unsigned long *ptr = (unsigned long*)ddsd2.lpSurface;
            for (y = 0 ; y < RIPPLE_TABLE_DIM ; y++)
            {
                for (x = 0 ; x < RIPPLE_TABLE_DIM ; x++)
                {
                    ptr[x] = 0xff808080;
                }
                ptr += ddsd2.lPitch / sizeof(long);
            }
        }

        TexInfo[ripple->Tpage].SourceSurface->Unlock(NULL);
    }
#endif // XBOX_NOT_YET_IMPLEMENTED

// force model poly attribs

    model->QuadNumTex = 0;
    model->TriNumTex = model->PolyNum;
    model->QuadNumRGB = 0;
    model->TriNumRGB = 0;

    for (i = 0 ; i < model->PolyNum ; i++)
    {
        model->PolyPtr[i].Type = POLY_SEMITRANS | POLY_SEMITRANS_ONE | POLY_DOUBLE;
        model->PolyPtr[i].Tpage = (short)ripple->Tpage;

        model->PolyPtr[i].tu0 = (model->PolyPtr[i].v0->x - model->Xmin) * scale + (REAL)ripple->OffsetX / RIPPLE_TABLE_DIM;
        model->PolyPtr[i].tv0 = (model->PolyPtr[i].v0->z - model->Zmin) * scale + (REAL)ripple->OffsetY / RIPPLE_TABLE_DIM;
        model->PolyPtr[i].tu1 = (model->PolyPtr[i].v1->x - model->Xmin) * scale + (REAL)ripple->OffsetX / RIPPLE_TABLE_DIM;
        model->PolyPtr[i].tv1 = (model->PolyPtr[i].v1->z - model->Zmin) * scale + (REAL)ripple->OffsetY / RIPPLE_TABLE_DIM;
        model->PolyPtr[i].tu2 = (model->PolyPtr[i].v2->x - model->Xmin) * scale + (REAL)ripple->OffsetX / RIPPLE_TABLE_DIM;
        model->PolyPtr[i].tv2 = (model->PolyPtr[i].v2->z - model->Zmin) * scale + (REAL)ripple->OffsetY / RIPPLE_TABLE_DIM;

        *(long*)&model->PolyRGB[i].rgb[0] = 0xff408080;
        *(long*)&model->PolyRGB[i].rgb[1] = 0xff408080;
        *(long*)&model->PolyRGB[i].rgb[2] = 0xff408080;
    }

// No collision

    obj->CollType = COLL_TYPE_NONE;
    obj->body.CollSkin.AllowObjColls = FALSE;
    obj->body.CollSkin.AllowWorldColls = FALSE;

// return OK

    return TRUE;
}
#endif


////////////////////////////////////////////////////////////////
//
// Init flappy flag
//
////////////////////////////////////////////////////////////////
#ifdef _PC
static long InitFlag(OBJECT *obj, long *flags)
{
    FLAG_DATA_OBJ *pFlag = (FLAG_DATA_OBJ*)obj->Data;
    int     cW, cH;
    REAL    u,v, Du,Dv;
    VEC     posXY;
    FLAG_PARTICLE*  pP;
    VEC     posY, posDX, posDY;
    VERTEX_TEX1*    pV;
    unsigned short* pI;
    unsigned short  index;
    REAL    uTL,vTL, uS,vS;
    BOUNDING_BOX    bbox;

    pFlag->pIndex = NULL;
    pFlag->pVert = NULL;
    pFlag->pParticle = NULL;

// set handlers
    obj->aihandler      = (AI_HANDLER)AI_FlagHandler;
    obj->renderhandler  = (AI_HANDLER)RenderFlag;
    obj->freehandler    = (FREE_HANDLER)FreeFlag;

// setup variables
    pFlag->w = 1+9; //flags[0];
    pFlag->h = 1+6; //flags[1];
    pFlag->length = Real(450) / (REAL)pFlag->w;
    pFlag->lengthD = (float)sqrt((pFlag->length * pFlag->length) * 2);

    switch (GameSettings.Level)
    {
        default:
        case LEVEL_GHOSTTOWN1:
            pFlag->tpage = TPAGE_WORLD_START + 6;
            uTL = 64;
            vTL = 128;
            uS  = 73;
            vS  = 49;
            break;
        case LEVEL_GHOSTTOWN2:
            pFlag->tpage = TPAGE_WORLD_START + 2;
            uTL = 0;
            vTL = 198;
            uS  = 73;
            vS  = 49;
            break;
    }

    pFlag->cVert = pFlag->w * pFlag->h;
    pFlag->cTris = 6 * (pFlag->w-1) * (pFlag->h-1);

    pFlag->cxRipple = frand(DEG2RAD*360);
    pFlag->cyRipple = frand(DEG2RAD*360);
    pFlag->czRipple = frand(DEG2RAD*360);
    pFlag->amplitude = frand(DEG2RAD*360);

    pFlag->pParticle = (FLAG_PARTICLE*)malloc(sizeof(FLAG_PARTICLE) * pFlag->cVert);
    Assert((long)pFlag->pParticle);
    pFlag->pVert = (VERTEX_TEX1*)malloc(sizeof(VERTEX_TEX1) * pFlag->cVert);
    Assert((long)pFlag->pVert);
    pFlag->pIndex = (unsigned short*)malloc(sizeof(unsigned short) * pFlag->cTris);
    Assert((long)pFlag->pIndex);

// Setup positions & velocities
    pP = pFlag->pParticle;
    CopyVec(&obj->body.Centre.Pos, &posY);
    VecEqScalarVec(&posDX, pFlag->length, &obj->body.Centre.WMatrix.mv[R]);
    VecEqScalarVec(&posDY, pFlag->length, &obj->body.Centre.WMatrix.mv[U]);
    for (cH = 0; cH < pFlag->h; cH++)
    {
        CopyVec(&posY, &posXY);

        for (cW = 0; cW < pFlag->w; cW++)
        {
            CopyVec(&posXY, &pP->pos);
            SetVec(&pP->vel, 0,0,0);
            SetVec(&pP->imp, 0,0,0);
            pP++;
            VecPlusEqVec(&posXY, &posDX);
        }

        VecPlusEqVec(&posY, &posDY);
    }

// Setup vertices
    pV = pFlag->pVert;
    Du = ONE / ((REAL)pFlag->w-1);
    Dv = ONE / ((REAL)pFlag->h-1);
    for (cH = 0, v = 0; cH < pFlag->h; cH++, v += Dv)
    {
        for (cW = 0, u = 0; cW < pFlag->w; cW++, u += Du)
        {
            pV->tu = (uTL + (uS * u)) / 256;
            pV->tv = (vTL + (vS * v)) / 256;
            pV->color = 0x00FFFFFF;

            pV++;
        }
    }

// Setup indices
    pI = pFlag->pIndex;
    index = 0;
    for (cH = 0; cH < pFlag->h-1; cH++)
    {
        for (cW = 0; cW < pFlag->w-1; cW++)
        {
            if ((cH & 1) ^ (cW & 1))
            {
                *pI++ = index;
                *pI++ = index + 1;
                *pI++ = index + pFlag->w;

                *pI++ = index + 1;
                *pI++ = index + 1 + pFlag->w;
                *pI++ = index + pFlag->w;
            }
            else
            {
                *pI++ = index;
                *pI++ = index + 1;
                *pI++ = index + 1 + pFlag->w;

                *pI++ = index;
                *pI++ = index + 1 + pFlag->w;
                *pI++ = index + pFlag->w;
            }

            index++;
        }

        index++;
    }

// Setup visimask
    bbox.Xmin = obj->body.Centre.Pos.v[X] - 300;
    bbox.Ymin = obj->body.Centre.Pos.v[Y] - 100;
    bbox.Zmin = obj->body.Centre.Pos.v[Z] - 300;
    bbox.Xmax = obj->body.Centre.Pos.v[X] + 300;
    bbox.Ymax = obj->body.Centre.Pos.v[Y] + 100;
    bbox.Zmax = obj->body.Centre.Pos.v[Z] + 300;
    pFlag->VisiMask = SetObjectVisiMask(&bbox);

    return TRUE;
}

static void FreeFlag(OBJECT *obj)
{
    FLAG_DATA_OBJ *pFlag = (FLAG_DATA_OBJ*)obj->Data;

    if (pFlag->pParticle)
    {
        free(pFlag->pParticle);
        pFlag->pParticle = NULL;
    }
    if (pFlag->pVert)
    {
        free(pFlag->pVert);
        pFlag->pVert = NULL;
    }
    if (pFlag->pIndex)
    {
        free(pFlag->pIndex);
        pFlag->pIndex = NULL;
    }
}

#endif

////////////////////////////////////////////////////////////////
//
// Init dolphin
//
////////////////////////////////////////////////////////////////
#ifdef _PC

static long InitDolphin(OBJECT *obj, long *flags)
{
    DOLPHIN_OBJ *dolphin = (DOLPHIN_OBJ*)obj->Data;
    BOUNDING_BOX box;

// setup handlers

    obj->renderhandler = (RENDER_HANDLER)RenderDolphin;
    obj->movehandler = (AI_HANDLER)AI_DolphinMoveHandler;

// setup dolphin

    dolphin->Time = Real(0);

    box.Xmin = obj->body.Centre.Pos.v[X] - 64;
    box.Xmax = obj->body.Centre.Pos.v[X] + 64;
    box.Ymin = obj->body.Centre.Pos.v[Y] - 64;
    box.Ymax = obj->body.Centre.Pos.v[Y] + 256;
    box.Zmin = obj->body.Centre.Pos.v[Z] - 64;
    box.Zmax = obj->body.Centre.Pos.v[Z] + 64;

    dolphin->VisiMask = SetObjectVisiMask(&box);

// return ok

    return TRUE;
}
#endif

////////////////////////////////////////////////////////////////
//
// Init garden fog
//
////////////////////////////////////////////////////////////////
#ifdef _PC

static long InitGardenFog(OBJECT *obj, long *flags)
{

// kill myself

    return FALSE;
}
#endif

////////////////////////////////////////////////////////////////
//
// Fog box - sets fog to black
//
////////////////////////////////////////////////////////////////
#ifndef _PSX

static long InitFogBox(OBJECT *obj, long *flags)
{
    FOGBOX_OBJ *fb = (FOGBOX_OBJ*)obj->Data;

// handlers

    obj->aihandler = (AI_HANDLER)AI_FogBoxHandler;

// setup fogbox obj

    fb->Box.Xmin = obj->body.Centre.Pos.v[X] - (REAL)flags[0];
    fb->Box.Xmax = obj->body.Centre.Pos.v[X] + (REAL)flags[0];
    fb->Box.Ymin = obj->body.Centre.Pos.v[Y] - (REAL)flags[1];
    fb->Box.Ymax = obj->body.Centre.Pos.v[Y] + (REAL)flags[1];
    fb->Box.Zmin = obj->body.Centre.Pos.v[Z] - (REAL)flags[2];
    fb->Box.Zmax = obj->body.Centre.Pos.v[Z] + (REAL)flags[2];

// save real fog rgb

    fb->RealFogRGB = CurrentLevelInfo.FogColor;

// No collision

    obj->CollType = COLL_TYPE_NONE;
    obj->body.CollSkin.AllowObjColls = FALSE;
    obj->body.CollSkin.AllowWorldColls = FALSE;

// return OK

    return TRUE;
}
#endif
