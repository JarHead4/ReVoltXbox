//-----------------------------------------------------------------------------
// File: AI_Car.cpp
//
// Desc: Car AI
//
// Re-Volt (Generic) Copyright (c) Probe Entertainment 1998
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"
#include "ai.h"
#include "ai_car.h"
#include "geom.h"
#include "player.h"
#include "aizone.h"
#include "main.h"
#include "move.h"
#include "weapon.h"
#include "obj_init.h"
#include "util.h"
#include "posnode.h"
#include "timing.h"
#include "pickup.h"
#include "ui_TitleScreen.h"
#include "InitPlay.h"
#include "LevelLoad.h"
#include "Ghost.h"
#ifdef _PC
#include "input.h"
#endif


/////////////////////////////////
// Temp Shat
/////////////////////////////////
//#undef GAZZA_TEACH_CAR_HANDLING
//#define GAZZA_TEACH_CAR_UNDERSTEER

REAL gGazzaQQQ = Real(1);   //(1);
REAL gGazzaWWW = TO_LENGTH(Real(20));   //800));
REAL gGazzaEEE = TO_LENGTH(Real(0.35));//Real(100));    //(250));


/////////////////////////////////
// AI TimeStep
/////////////////////////////////
#ifdef _PC
#define CAI_TIME_STEP_MS        (1000/30)
#define CAI_TIME_STEP_SECS      TO_TIME(ONE/30) //(Real(CAI_TIME_STEP_MS)/Real(1000))
#define CAI_CORRECT_ANGLE       (Real(0.2))
#endif

#ifdef _N64
#define CAI_TIME_STEP_MS        (1000/30)
#define CAI_TIME_STEP_SECS      TO_TIME(ONE/30) //(Real(CAI_TIME_STEP_MS)/Real(1000))
#define CAI_CORRECT_ANGLE       (Real(0.2))
#endif

#ifdef _PSX
#define CAI_TIME_STEP_MS        (1000/30)
#define CAI_TIME_STEP_SECS      TO_TIME(ONE/30) //(Real(CAI_TIME_STEP_MS)/Real(1000))
#define CAI_CORRECT_ANGLE       (Real(0.2))
#endif

#define OVERTAKE_MIN_TIME       TO_TIME(Real(4))
#define CATCHUP_RESTART_DIST    TO_LENGTH(Real(200*300))

/////////////////////////////////
// CatchUp vars
/////////////////////////////////

#ifndef _PSX    // PC / N64 Catchup

// Speed Up / Slow Down - VERY EASY
REAL gCatchUp_SpeedUpVeryEasy[]  = {(MPH2OGU_SPEED * 100) / 100,
                                    (MPH2OGU_SPEED * 150) / 100,
                                    (MPH2OGU_SPEED * 200) / 100,
                                    (MPH2OGU_SPEED * 300) / 100};
REAL gCatchUp_SlowDownVeryEasy[] = {-MPH2OGU_SPEED * 2,
                                    -MPH2OGU_SPEED * 3,
                                    -MPH2OGU_SPEED * 4};

// Speed Up / Slow Down - EASY
REAL gCatchUp_SpeedUpEasy[]  = {(MPH2OGU_SPEED * 100) / 100,
                                (MPH2OGU_SPEED * 150) / 100,
                                (MPH2OGU_SPEED * 200) / 100,
                                (MPH2OGU_SPEED * 300) / 100};
REAL gCatchUp_SlowDownEasy[] = {-MPH2OGU_SPEED * 1,
                                -MPH2OGU_SPEED * 2,
                                -MPH2OGU_SPEED * 3};

// Speed Up / Slow Down - MEDIUM
REAL gCatchUp_SpeedUpMedium[]  = {(MPH2OGU_SPEED * 100) / 100,
                                  (MPH2OGU_SPEED * 200) / 100,
                                  (MPH2OGU_SPEED * 300) / 100,
                                  (MPH2OGU_SPEED * 400) / 100};
REAL gCatchUp_SlowDownMedium[] = {-MPH2OGU_SPEED * 0,
                                  -MPH2OGU_SPEED * 1,
                                  -MPH2OGU_SPEED * 2};

// Speed Up / Slow Down - HARD
REAL gCatchUp_SpeedUpHard[]  = {(MPH2OGU_SPEED * 100) / 100,
                                (MPH2OGU_SPEED * 200) / 100,
                                (MPH2OGU_SPEED * 300) / 100,
                                (MPH2OGU_SPEED * 400) / 100};
REAL gCatchUp_SlowDownHard[] = {-MPH2OGU_SPEED * 0,
                                -MPH2OGU_SPEED * 0,
                                -MPH2OGU_SPEED * 1};

t_CatchUp   gCatchUp[] =
{
    {200*10,200*3,  200*3, FALSE, TRUE,  gCatchUp_SpeedUpVeryEasy, gCatchUp_SlowDownVeryEasy,
                TO_TIME(Real(5)), TO_LENGTH(Real(200)), TO_LENGTH(Real(200*200)), TO_TIME(Real(3))},
    {200*10,200*7,  200*5, FALSE, TRUE,  gCatchUp_SpeedUpEasy, gCatchUp_SlowDownEasy,
                TO_TIME(Real(4)), TO_LENGTH(Real(100)), TO_LENGTH(Real(100*100)), TO_TIME(Real(2))},
    {200*7, 200*10, 200*5, TRUE,  FALSE, gCatchUp_SpeedUpMedium, gCatchUp_SlowDownMedium,
                TO_TIME(Real(2.5)), TO_LENGTH(Real(50)), TO_LENGTH(Real(50*50)), TO_TIME(Real(2))},
    {200*5, 200*13, 200*5, TRUE,  FALSE, gCatchUp_SpeedUpHard, gCatchUp_SlowDownHard,
                TO_TIME(Real(1)), TO_LENGTH(Real(20)), TO_LENGTH(Real(20*20)), TO_TIME(Real(1))},
};

#else   //PSX Catchup

    #if 0   // 20/7/99

// Speed Up / Slow Down - VERY EASY
REAL gCatchUp_SpeedUpVeryEasy[]  = {(MPH2OGU_SPEED * 100) / 100,
                                    (MPH2OGU_SPEED * 200) / 100,
                                    (MPH2OGU_SPEED * 250) / 100,
                                    (MPH2OGU_SPEED * 300) / 100};
REAL gCatchUp_SlowDownVeryEasy[] = {-MPH2OGU_SPEED * 2,
                                    -MPH2OGU_SPEED * 5,
                                    -MPH2OGU_SPEED * 8};

// Speed Up / Slow Down - EASY
REAL gCatchUp_SpeedUpEasy[]  = {(MPH2OGU_SPEED * 100) / 100,
                                (MPH2OGU_SPEED * 200) / 100,
                                (MPH2OGU_SPEED * 250) / 100,
                                (MPH2OGU_SPEED * 300) / 100};
REAL gCatchUp_SlowDownEasy[] = {-MPH2OGU_SPEED * 2,
                                -MPH2OGU_SPEED * 5,
                                -MPH2OGU_SPEED * 7};

// Speed Up / Slow Down - MEDIUM
REAL gCatchUp_SpeedUpMedium[]  = {(MPH2OGU_SPEED * 100) / 100,
                                  (MPH2OGU_SPEED * 200) / 100,
                                  (MPH2OGU_SPEED * 300) / 100,
                                  (MPH2OGU_SPEED * 400) / 100};
REAL gCatchUp_SlowDownMedium[] = {-MPH2OGU_SPEED * 2,
                                  -MPH2OGU_SPEED * 4,
                                  -MPH2OGU_SPEED * 6};

// Speed Up / Slow Down - HARD
REAL gCatchUp_SpeedUpHard[]  = {(MPH2OGU_SPEED * 100) / 100,
                                (MPH2OGU_SPEED * 200) / 100,
                                (MPH2OGU_SPEED * 300) / 100,
                                (MPH2OGU_SPEED * 400) / 100};
REAL gCatchUp_SlowDownHard[] = {-MPH2OGU_SPEED * 1,
                                -MPH2OGU_SPEED * 2,
                                -MPH2OGU_SPEED * 3};

t_CatchUp   gCatchUp[] =
{
    {200*5,200*7, 200*2, TRUE, TRUE,  gCatchUp_SpeedUpVeryEasy, gCatchUp_SlowDownVeryEasy,
            TO_TIME(Real(5)), TO_LENGTH(Real(150)), 150*150, TO_TIME(Real(3))},
    {200*5,200*7, 200*2, TRUE, TRUE,  gCatchUp_SpeedUpEasy, gCatchUp_SlowDownEasy,
            TO_TIME(Real(4)), TO_LENGTH(Real(750)), 75*75, TO_TIME(Real(2))},
    {200*5,200*7, 200*2, TRUE,  FALSE, gCatchUp_SpeedUpMedium, gCatchUp_SlowDownMedium,
            TO_TIME(Real(2.5)), TO_LENGTH(Real(30)), 30*30, TO_TIME(Real(2))},
    {200*5,200*7, 200*2, FALSE,  FALSE, gCatchUp_SpeedUpHard, gCatchUp_SlowDownHard,
            TO_TIME(Real(1)), TO_LENGTH(Real(10)), 10*10, TO_TIME(Real(1))},
};
    #endif  // 20/7/99

    #if 1   // 21/7/99

// Speed Up / Slow Down - VERY EASY
REAL gCatchUp_SpeedUpVeryEasy[]  = {(MPH2OGU_SPEED * 100) / 100,
                                    (MPH2OGU_SPEED * 200) / 100,
                                    (MPH2OGU_SPEED * 250) / 100,
                                    (MPH2OGU_SPEED * 300) / 100};
REAL gCatchUp_SlowDownVeryEasy[] = {-MPH2OGU_SPEED * 2,
                                    -MPH2OGU_SPEED * 5,
                                    -MPH2OGU_SPEED * 8};

// Speed Up / Slow Down - EASY
REAL gCatchUp_SpeedUpEasy[]  = {(MPH2OGU_SPEED * 100) / 100,
                                (MPH2OGU_SPEED * 200) / 100,
                                (MPH2OGU_SPEED * 300) / 100,
                                (MPH2OGU_SPEED * 400) / 100};
REAL gCatchUp_SlowDownEasy[] = {-MPH2OGU_SPEED * 1,
                                -MPH2OGU_SPEED * 2,
                                -MPH2OGU_SPEED * 3};

// Speed Up / Slow Down - MEDIUM
REAL gCatchUp_SpeedUpMedium[]  = {(MPH2OGU_SPEED * 100) / 100,
                                  (MPH2OGU_SPEED * 150) / 100,
                                  (MPH2OGU_SPEED * 200) / 100,
                                  (MPH2OGU_SPEED * 250) / 100};
REAL gCatchUp_SlowDownMedium[] = {-MPH2OGU_SPEED * 0,
                                  -MPH2OGU_SPEED * 1,
                                  -MPH2OGU_SPEED * 2};

// Speed Up / Slow Down - HARD
REAL gCatchUp_SpeedUpHard[]  = {(MPH2OGU_SPEED * 100) / 100,
                                (MPH2OGU_SPEED * 150) / 100,
                                (MPH2OGU_SPEED * 200) / 100,
                                (MPH2OGU_SPEED * 250) / 100};
REAL gCatchUp_SlowDownHard[] = {-MPH2OGU_SPEED * 0,
                                -MPH2OGU_SPEED * 1,
                                -MPH2OGU_SPEED * 2};

t_CatchUp   gCatchUp[] =
{
    {200*5,200*7, 200*2, TRUE, TRUE,  gCatchUp_SpeedUpVeryEasy, gCatchUp_SlowDownVeryEasy,
            TO_TIME(Real(5)), TO_LENGTH(Real(150)), 150*150, TO_TIME(Real(3))},
    {200*5,200*7, 200*2, TRUE, TRUE,  gCatchUp_SpeedUpEasy, gCatchUp_SlowDownEasy,
            TO_TIME(Real(4)), TO_LENGTH(Real(750)), 75*75, TO_TIME(Real(2))},
    {200*5,200*7, 200*2, TRUE,  FALSE, gCatchUp_SpeedUpMedium, gCatchUp_SlowDownMedium,
            TO_TIME(Real(2.5)), TO_LENGTH(Real(30)), 30*30, TO_TIME(Real(2))},
    {200*5,200*7, 200*2, TRUE,  FALSE, gCatchUp_SpeedUpHard, gCatchUp_SlowDownHard,
            TO_TIME(Real(1)), TO_LENGTH(Real(10)), 10*10, TO_TIME(Real(1))},
};
    #endif  // 20/7/99

#endif

t_CatchUp   *gpCatchUpVars = &gCatchUp[0];

/////////////////////////////////
//
/////////////////////////////////
REAL gCAI_TimeStep = 0;
int gCAI_cProcessAI = FALSE;


/////////////////////////////////
// Static variables
/////////////////////////////////
//  long    pickupBias;
//  long    blockBias;
//  long    overtakeBias;
//  long    softSuspension;


#define SKILL_SETTING(pickup, block, overtake, suspension) \
    { \
/*      Real((pickup)),         \
        Real((block)),          \
        Real((overtake)),       \
        Real((suspension)),     \
*/\
        (int)(32767 * (pickup)),        \
        (int)(32767 * (block)),         \
        (int)(32767 * (overtake)),      \
        (int)(32767 * (suspension)),    \
    },

static CAI_SKILLS   gSkills[] =
{
    SKILL_SETTING(0.7, 0.3, 0.5, 0.7)           //  CARID_RC,
    SKILL_SETTING(0.5, 0.5, 0.5, 0.8)           //  CARID_DUSTMITE,
    SKILL_SETTING(0.9, 0.5, 0.1, 0.9)           //  CARID_PHATSLUG,
    SKILL_SETTING(0.3, 0.3, 0.7, 0.6)           //  CARID_COLMOSS,
    SKILL_SETTING(0.9, 0.5, 0.5, 0.9)           //  CARID_HARVESTER,
    SKILL_SETTING(0.4, 0.7, 0.6, 0.5)           //  CARID_DOCGRUDGE,
    SKILL_SETTING(0.1, 0.2, 0.7, 0.0)           //  CARID_VOLKEN,
    SKILL_SETTING(0.2, 0.2, 0.7, 0.0)           //  CARID_SPRINTER,

    SKILL_SETTING(0.5, 0.5, 0.5, 0.3)           //  CARID_DYNAMO,
    SKILL_SETTING(0.9, 0.5, 0.5, 0.7)           //  CARID_CANDY,
    SKILL_SETTING(0.5, 0.5, 0.5, 0.3)           //  CARID_GENGHIS,
    SKILL_SETTING(0.5, 0.5, 0.5, 0.3)           //  CARID_FISH,
    SKILL_SETTING(0.5, 0.5, 0.5, 0.3)           //  CARID_MOUSE,
    SKILL_SETTING(0.5, 0.5, 0.5, 0.3)           //  CARID_FLAG,
    SKILL_SETTING(0.5, 0.5, 0.5, 0.3)           //  CARID_PANGATC,
    SKILL_SETTING(0.5, 0.5, 0.5, 0.1)           //  CARID_R5,
    SKILL_SETTING(0.5, 0.5, 0.5, 0.3)           //  CARID_LOADED,
    SKILL_SETTING(0.5, 0.5, 0.5, 0.3)           //  CARID_BERTHA,
    SKILL_SETTING(0.5, 0.5, 0.5, 0.3)           //  CARID_INSECTO,

    SKILL_SETTING(0.1, 0.1, 0.5, 0.0)           //  CARID_ADEON,
    SKILL_SETTING(0.1, 0.1, 0.5, 0.0)           //  CARID_FONE,
    SKILL_SETTING(0.1, 0.1, 0.5, 0.0)           //  CARID_ZIPPER,
    SKILL_SETTING(0.5, 0.5, 0.5, 0.0)           //  CARID_ROTOR,
    SKILL_SETTING(0.1, 0.1, 0.5, 0.0)           //  CARID_COUGAR,
    SKILL_SETTING(0.1, 0.1, 0.5, 0.0)           //  CARID_SUGO,
    SKILL_SETTING(0.1, 0.1, 0.5, 0.0)           //  CARID_TOYECA,
    SKILL_SETTING(0.1, 0.1, 0.5, 0.0)           //  CARID_AMW,
    SKILL_SETTING(0.5, 0.5, 0.5, 0.0)           //  CARID_PANGA,

    SKILL_SETTING(0.5, 0.5, 0.5, 0.0)           //  CARID_TROLLEY,
    SKILL_SETTING(0.5, 0.5, 0.5, 0.0)           //  CARID_KEY1,
    SKILL_SETTING(0.5, 0.5, 0.5, 0.0)           //  CARID_KEY2,
    SKILL_SETTING(0.5, 0.5, 0.5, 0.0)           //  CARID_KEY3,
    SKILL_SETTING(0.5, 0.5, 0.5, 0.0)           //  CARID_KEY4,
#ifndef _N64
    SKILL_SETTING(0.5, 0.5, 0.5, 0.0)           //  CARID_UFO,
#endif
#ifdef _PC
    SKILL_SETTING(0.5, 0.5, 0.5, 0.0)           //  CARID_MYSTERY,
#endif
};


// Car attributes
typedef struct s_CarUnderOverSteer
{
    REAL    understeerThreshold;
    REAL    understeerRange;
    REAL    understeerFront;
    REAL    understeerRear;
    REAL    understeerMax;
    REAL    oversteerThreshold;
    REAL    oversteerRange;
    REAL    oversteerMax;
    REAL    oversteerAccelThreshold;
    REAL    oversteerAccelRange;
} t_CarUnderOverSteer;

// $MD: do we need this?
#if 0
t_CarUnderOverSteer gCarUnderOverSteerTable[CARID_NTYPES] =
{
    {Real(64.66), Real(1915.36), Real(450), Real(192.8), Real(0.83), Real(166.01), Real(134.34), Real(1), Real(64.55), Real(531.25)},   // RC
    {Real(5), Real(1783.44), Real(401.94), Real(321.52), Real(0.14), Real(394.77), Real(672.25), Real(1), Real(131.03), Real(1928.01)}, // DustMite
    {Real(194.58), Real(1500), Real(724.56), Real(335), Real(0.95), Real(377.96), Real(1391), Real(1), Real(10), Real(569.19)},         // PhatSlug
    {Real(105.74), Real(1500), Real(450), Real(255.86), Real(0.35), Real(580.81), Real(1232.44), Real(1), Real(10), Real(344.9)},           // ColMoss
    {Real(150), Real(1500), Real(450), Real(335), Real(0.95), Real(100), Real(1391), Real(0.73), Real(10), Real(865.48)},               // Harvester
    {Real(93.82), Real(1500), Real(197.1), Real(335), Real(0.95), Real(462.21), Real(725.13), Real(1), Real(37.34), Real(604.99)},      // DocGrudge
    {Real(172.69), Real(1500), Real(221.55), Real(335), Real(0.83), Real(100), Real(1391), Real(1), Real(10), Real(400)},               // Volken
    {Real(207.72), Real(1500), Real(450), Real(386.41), Real(0.95), Real(100), Real(1403.79), Real(0.35), Real(154.38), Real(611.05)},  // Sprinter
    {Real(150), Real(1500), Real(450), Real(144.98), Real(0.95), Real(281.02), Real(1391), Real(0.52), Real(10), Real(400)},            // Dynamo
    {Real(150), Real(1493.26), Real(372), Real(335), Real(0.95), Real(100), Real(1391), Real(0.36), Real(10), Real(400)},               // Candy
    {Real(57.11), Real(1500), Real(299.25), Real(335), Real(0.95), Real(641.36), Real(770.6), Real(1), Real(10), Real(400)},            // Genghis
    {Real(150), Real(1500), Real(450), Real(159.74), Real(0.95), Real(546.81), Real(1391), Real(1), Real(10), Real(709.9)}, // Fidh
    {Real(148.97), Real(1500), Real(450), Real(335), Real(0.95), Real(100), Real(1391), Real(1), Real(10), Real(400)},  // Mouse
    {Real(150), Real(1010.82), Real(450), Real(335), Real(0.61), Real(100), Real(1391), Real(0.93), Real(27.7), Real(185.44)},  // Flag
    {Real(150), Real(1500), Real(450), Real(292.15), Real(0.95), Real(275.24), Real(1312.02), Real(0.25), Real(10), Real(671.25)},  // Pangatc
    {Real(142.46), Real(1500), Real(450), Real(193.55), Real(0.91), Real(100), Real(1391), Real(0.95), Real(93.66), Real(400)}, // R5
    {Real(150), Real(1500), Real(611.51), Real(335), Real(0.95), Real(389.66), Real(1391), Real(0.39), Real(10), Real(400)},    // Loaded
    {Real(82.15), Real(1500), Real(251.52), Real(335), Real(0.95), Real(412.13), Real(1391), Real(1), Real(113.93), Real(400)}, // Bertha Ballistics
    {Real(196.19), Real(2233.67), Real(450), Real(335), Real(0.95), Real(100), Real(1436.58), Real(0.79), Real(10), Real(400)}, // Pest Control
    {Real(150), Real(1500), Real(450), Real(335), Real(0.32), Real(171.29), Real(1831.65), Real(0.9), Real(89.17), Real(400)},  // Odeon
    {Real(137.11), Real(1500), Real(450), Real(335), Real(0.95), Real(104.08), Real(1391), Real(1), Real(10), Real(331.59)},    // Fone
    {Real(150), Real(1500), Real(450), Real(335), Real(0.95), Real(100), Real(899.26), Real(1), Real(10), Real(400)},   // Zipper
    {Real(150), Real(1500), Real(450), Real(335), Real(0.95), Real(100), Real(1391), Real(1), Real(10), Real(400)}, // Rotor
    {Real(150), Real(1500), Real(460.67), Real(335), Real(0.95), Real(100), Real(1167.56), Real(1), Real(10), Real(400)},   // Couger
    {Real(215.57), Real(1500), Real(450), Real(335), Real(0.95), Real(430.2), Real(772.5), Real(1), Real(10), Real(400)},   // Humma
    {Real(150), Real(1390.94), Real(450), Real(310.26), Real(0.87), Real(705.87), Real(1345.93), Real(1), Real(83.45), Real(400)},  // Toyeca
    {Real(150), Real(1500), Real(450), Real(335), Real(0.95), Real(100), Real(1391), Real(1), Real(156.53), Real(420.34)},  // AMW
    {Real(150), Real(1500), Real(450), Real(335), Real(0.95), Real(100), Real(1391), Real(1), Real(10), Real(400)}, // Panga
    {Real(150), Real(1500), Real(450), Real(335), Real(0.95), Real(100), Real(1391), Real(1), Real(10), Real(400)}, // Trolley
    {Real(150), Real(1500), Real(450), Real(335), Real(0.95), Real(100), Real(1391), Real(1), Real(10), Real(400)}, // Key1
    {Real(150), Real(1500), Real(450), Real(335), Real(0.95), Real(100), Real(1391), Real(1), Real(10), Real(400)}, // Key2
    {Real(150), Real(1500), Real(450), Real(335), Real(0.95), Real(100), Real(1391), Real(1), Real(10), Real(400)}, // Key3
    {Real(150), Real(1500), Real(450), Real(335), Real(0.95), Real(100), Real(1391), Real(1), Real(10), Real(400)}, // Key4
#ifndef _N64
    {Real(150), Real(1500), Real(450), Real(335), Real(0.95), Real(100), Real(1391), Real(1), Real(10), Real(400)}, // UFO
#endif
#ifdef _PC
    {Real(150), Real(1500), Real(450), Real(335), Real(0.95), Real(100), Real(1391), Real(1), Real(10), Real(400)}, // MYSTERY
#endif
};
#endif


// Player in 1st place
PLAYER  *gpPlayer1st;
PLAYER  *gpCarRaceOrder[MAX_NUM_PLAYERS];               // Car race order

//
// Global function prototypes
//

void CAI_CarHelper(PLAYER *Player);
void CAI_FlipCar(PLAYER *Player);
void CAI_ResetCar(PLAYER *Player);
bool CAI_IsCarStuck(PLAYER *Player);
bool CAI_IsCarInZone(PLAYER *Player);
bool CAI_IsCarOnTrack(PLAYER *Player);

void CAI_ProcessCall(PLAYER *pPlayer);
bool CAI_IsCarInTrouble(PLAYER *pPlayer);
void CAI_ProcessUnderOverSteer(PLAYER *pPlayer);
void ProcessNearWallAvoidance(PLAYER* pPlayer);
void CAI_ChooseRoute(PLAYER *pPlayer);
void CAI_ProcessCarSteering(PLAYER *pPlayer);
bool CAI_CalcBrakingParameters(PLAYER *pPlayer, REAL *pBrakeDist, REAL *pSpeed);
void CAI_ProcessWeapons(PLAYER *pPlayer);

void CAI_State_Race(PLAYER *pPlayer);
void CAI_State_BrakeIntoCorner(PLAYER *pPlayer);
void CAI_State_ReverseCorrect(PLAYER *pPlayer);
void CAI_State_ForwardCorrect(PLAYER *pPlayer);
void CAI_State_CorrectForwardLeft(PLAYER *pPlayer);
void CAI_State_CorrectForwardRight(PLAYER *pPlayer);
void CAI_State_CorrectReverseLeft(PLAYER *pPlayer);
void CAI_State_CorrectReverseRight(PLAYER *pPlayer);
void CAI_State_InTheAir(PLAYER *pPlayer);
void CAI_State_Landed(PLAYER *pPlayer);

void CAI_QuantizeControls(CTRL *pControls);


#ifdef NEURAL_NET
void CAI_NeuralNetworkInit(PLAYER *pPlayer)
void CAI_NeuralNetworkTrain(PLAYER *pPlayer)
#endif


//
// Car AI State Jump Table
//
//QQQQ gpCAI_StateFunc[] =
//{
//  dddd
//};

REAL gCAI_StateTimer[] =
{
    TO_TIME(Real(0.0)),     // CAI_S_NULL
    TO_TIME(Real(0.0)),     // CAI_S_STARTING_GRID
    TO_TIME(Real(0.0)),     // CAI_S_GREEN_LIGHT
    TO_TIME(Real(0.0)),     // CAI_S_RACE
    TO_TIME(Real(0.0)),     // CAI_S_BRAKING_ZONE
    TO_TIME(Real(2.0)),     // CAI_S_REVERSE_CORRECT
    TO_TIME(Real(2.0)),     // CAI_S_FORWARD_CORRECT
    TO_TIME(Real(0.0)),     // CAI_S_FIND_WAY_BACK_TO_NODE
    TO_TIME(Real(0.0)),     // CAI_S_IN_THE_AIR
    TO_TIME(Real(0.1)),     // CAI_S_LANDED
    TO_TIME(Real(3.0)),     // CAI_S_CORRECT_FORWARDLEFT
    TO_TIME(Real(3.0)),     // CAI_S_CORRECT_FORWARDRIGHT
    TO_TIME(Real(3.0)),     // CAI_S_CORRECT_REVERSELEFT
    TO_TIME(Real(3.0)),     // CAI_S_CORRECT_REVERSERIGHT
};

// Car AI Debug Strings
char *gCAI_StateDebugStrings[] =
{
    "NULL",
    "Starting Grid",
    "Green Light",
    "Race",
    "Braking Zone",
    "Reverse Correct",
    "Forward Correct",
    "Find Way Back To Node",
    "In the Air",
    "Landed",
    "Correct FL",
    "Correct FR",
    "Correct RL",
    "Correct RR",
    "",
    "",
    "",
    "",
    "",
};


//--------------------------------------------------------------------------------------------------------------------------


// Temporary function to put AI stuff into carinfo
#ifdef _PC
// #MD: removed
#if 0
void InitCarInfoAIData()
{
    int carID;

    for (carID = 0; carID < CARID_NTYPES; carID++) {
        CarInfo[carID].AI.understeerThreshold = TO_VEL(gCarUnderOverSteerTable[carID].understeerThreshold);
        CarInfo[carID].AI.understeerRange = TO_VEL(gCarUnderOverSteerTable[carID].understeerRange);
        CarInfo[carID].AI.understeerFront = TO_VEL(gCarUnderOverSteerTable[carID].understeerFront);
        CarInfo[carID].AI.understeerRear = TO_VEL(gCarUnderOverSteerTable[carID].understeerRear);
        CarInfo[carID].AI.understeerMax = gCarUnderOverSteerTable[carID].understeerMax;

        CarInfo[carID].AI.oversteerThreshold = TO_VEL(gCarUnderOverSteerTable[carID].oversteerThreshold);
        CarInfo[carID].AI.oversteerRange = TO_VEL(gCarUnderOverSteerTable[carID].oversteerRange);
        CarInfo[carID].AI.oversteerMax = gCarUnderOverSteerTable[carID].oversteerMax;
        CarInfo[carID].AI.oversteerAccelThreshold = TO_VEL(gCarUnderOverSteerTable[carID].oversteerAccelThreshold);
        CarInfo[carID].AI.oversteerAccelRange = TO_VEL(gCarUnderOverSteerTable[carID].oversteerAccelRange);

        CarInfo[carID].AI.pickupBias = gSkills[carID].pickupBias;
        CarInfo[carID].AI.blockBias = gSkills[carID].blockBias;
        CarInfo[carID].AI.overtakeBias = gSkills[carID].overtakeBias;
        CarInfo[carID].AI.suspension = gSkills[carID].suspension;
        CarInfo[carID].AI.aggression = 0; // gSkills[carID].aggression;
    }
}

void ReSaveCarInfoAIData(PLAYER* Player)
{
    int carID = Player->car.CarType;

    CarInfo[carID].AI.understeerThreshold       = Player->CarAI.understeerThreshold;
    CarInfo[carID].AI.understeerRange           = Player->CarAI.understeerRange;
    CarInfo[carID].AI.understeerFront           = Player->CarAI.understeerFront;
    CarInfo[carID].AI.understeerRear            = Player->CarAI.understeerRear;
    CarInfo[carID].AI.understeerMax             = Player->CarAI.understeerMax;
    
    CarInfo[carID].AI.oversteerThreshold        = Player->CarAI.oversteerThreshold;
    CarInfo[carID].AI.oversteerRange            = Player->CarAI.oversteerRange;
    CarInfo[carID].AI.oversteerMax              = Player->CarAI.oversteerMax;
    CarInfo[carID].AI.oversteerAccelThreshold   = Player->CarAI.oversteerAccelThreshold;
    CarInfo[carID].AI.oversteerAccelRange       = Player->CarAI.oversteerAccelRange;

    CarInfo[carID].AI.pickupBias                = Player->CarAI.Skills.pickupBias;
    CarInfo[carID].AI.blockBias                 = Player->CarAI.Skills.blockBias;
    CarInfo[carID].AI.overtakeBias              = Player->CarAI.Skills.overtakeBias;
    CarInfo[carID].AI.suspension                = Player->CarAI.Skills.suspension;
    CarInfo[carID].AI.aggression                = 0;
}

#endif
#endif
        

//
// CAI_InitCarAI
//
// Initialises the AI variables for the specified player
// Call after loading AI Zones & Nodes

void CAI_InitCarAI(PLAYER *Player, long Skill)
{
    int i;

    Player->CarAI.CurZone = -1;
    Player->CarAI.CurZoneID = -1;
    Player->CarAI.CurZoneBBox = -1;

    Player->CarAI.iRouteCurNode = 0;
    Player->CarAI.iRouteDestNode = 0;

//  Player->CarAI.IsInZone = TRUE;//CAI_IsCarInZone(Player);
    Player->CarAI.IsInZone = FALSE;
    for (i = 0; i < AiZoneNumID; i++)
    {
        if (AIZ_IsCarInZoneID(Player, i))
        {
            Player->CarAI.IsInZone = TRUE;
            break;
        }
    }

    Player->CarAI.pCurNode = AIN_FindNodePlayerIsIn(Player);
    Player->CarAI.pLastValidNode = Player->CarAI.pCurNode;
    Player->CarAI.pLastSplitNode = NULL;
    Player->CarAI.pDestNode = NULL;
    Player->CarAI.pLastDestNode = NULL;
    Player->CarAI.pFutureNode = NULL;

    Player->CarAI.lastAIState   = CAI_S_NULL;
    Player->CarAI.AIState       = CAI_S_STARTING_GRID;
    Player->CarAI.AIStateCount  = Real(0.0);
    Player->CarAI.TrackMode     = CAI_T_FORWARD;
//  Player->CarAI.Skills        = gSkills[Skill];
    Player->CarAI.Skills.blockBias = CarInfo[Player->car.CarType].AI.blockBias;
    Player->CarAI.Skills.overtakeBias = CarInfo[Player->car.CarType].AI.overtakeBias;
    Player->CarAI.Skills.pickupBias = CarInfo[Player->car.CarType].AI.pickupBias;
    Player->CarAI.Skills.suspension = CarInfo[Player->car.CarType].AI.suspension;
    //Player->CarAI.Skills.aggression = CarInfo[Player->car.CarType].AI.aggression;

    Player->CarAI.pNearestCarInFront = NULL;
    Player->CarAI.pNearestCarBehind= NULL;

    Player->CarAI.catchUpMode   = 0;
    Player->CarAI.bOvertake     = FALSE;
    Player->CarAI.bOvertakeLast = -1;
    Player->CarAI.timeOvertake  = 0;
    Player->CarAI.curRacingLine = Real(0.5);
    Player->CarAI.dstRacingLine = Real(0.5);

    Player->CarAI.biasSize       = TO_LENGTH(Real(50));
    Player->CarAI.biasMaxSize    = TO_LENGTH(Real(100));
    Player->CarAI.biasEdge       = TO_LENGTH(Real(150));
    Player->CarAI.biasExpandRate = TO_LENGTH(Real(50));
    Player->CarAI.biasShrinkRate = TO_LENGTH(Real(300));

    Player->CarAI.usePickup = -1;
    Player->CarAI.bRouteTaken = TRUE;

    Player->CarAI.speedMax = Player->car.TopSpeed;
    Player->CarAI.speedCur = 0;

    Player->CarAI.cZoneReset = 0;
    Player->CarAI.cNodeReset = 0;
    Player->CarAI.StuckCnt = 0;

    Player->CarAI.dx = 0;
    Player->CarAI.dy = 0;

    Player->CarAI.curNodeEdgeDist[0] = Player->CarAI.curNodeEdgeDist[1] = 0;
    Player->CarAI.lastCurNodeEdgeDist[0] = Player->CarAI.lastCurNodeEdgeDist[1] = 0;
    Player->CarAI.dstNodeEdgeDist[0] = Player->CarAI.dstNodeEdgeDist[1] = 0;

// Beeched variables
    CopyVec(&Player->car.Body->Centre.Pos, &Player->CarAI.beechedPos);
    Player->CarAI.beechedPosCount = gpCatchUpVars->beachMaxTime;

// Under/oversteer values
//      Real(64.66), Real(1915.36), Real(450), Real(192.8), Real(0.83),
//      Real(166.01), Real(134.34), Real(1), Real(64.55), Real(531.25)},    // RC
/*  Player->CarAI.understeerThreshold       = TO_VEL(gCarUnderOverSteerTable[Player->car.CarType].understeerThreshold);
    Player->CarAI.understeerRange           = TO_VEL(gCarUnderOverSteerTable[Player->car.CarType].understeerRange);
    Player->CarAI.understeerFront           = TO_VEL(gCarUnderOverSteerTable[Player->car.CarType].understeerFront);
    Player->CarAI.understeerRear            = TO_VEL(gCarUnderOverSteerTable[Player->car.CarType].understeerRear);
    Player->CarAI.understeerMax             = gCarUnderOverSteerTable[Player->car.CarType].understeerMax;
    Player->CarAI.oversteerThreshold        = TO_VEL(gCarUnderOverSteerTable[Player->car.CarType].oversteerThreshold);
    Player->CarAI.oversteerRange            = TO_VEL(gCarUnderOverSteerTable[Player->car.CarType].oversteerRange);
    Player->CarAI.oversteerMax              = gCarUnderOverSteerTable[Player->car.CarType].oversteerMax;
    Player->CarAI.oversteerAccelThreshold   = TO_VEL(gCarUnderOverSteerTable[Player->car.CarType].oversteerAccelThreshold);
    Player->CarAI.oversteerAccelRange       = TO_VEL(gCarUnderOverSteerTable[Player->car.CarType].oversteerAccelRange);*/

    Player->CarAI.understeerThreshold       = CarInfo[Player->car.CarType].AI.understeerThreshold;
    Player->CarAI.understeerRange           = CarInfo[Player->car.CarType].AI.understeerRange;
    Player->CarAI.understeerFront           = CarInfo[Player->car.CarType].AI.understeerFront;
    Player->CarAI.understeerRear            = CarInfo[Player->car.CarType].AI.understeerRear;
    Player->CarAI.understeerMax             = CarInfo[Player->car.CarType].AI.understeerMax;
    Player->CarAI.oversteerThreshold        = CarInfo[Player->car.CarType].AI.oversteerThreshold;
    Player->CarAI.oversteerRange            = CarInfo[Player->car.CarType].AI.oversteerRange;
    Player->CarAI.oversteerMax              = CarInfo[Player->car.CarType].AI.oversteerMax;
    Player->CarAI.oversteerAccelThreshold   = CarInfo[Player->car.CarType].AI.oversteerAccelThreshold;
    Player->CarAI.oversteerAccelRange       = CarInfo[Player->car.CarType].AI.oversteerAccelRange;

// Misc
    Player->CarAI.racePosition = -1;
    Player->CarAI.raceDistance = 0;


#ifdef GAZZA_TEACH_CAR_HANDLING
        Player->CarAI.lastLap = 0;
        Player->CarAI.cLaps = 2;
        Player->CarAI.bestTime = FLT_MAX;
        Player->CarAI.fBestTime = 0;
        Player->CarAI.iLastNeuron = -1;

        for (i = 0; i < 10; i++)
            Player->CarAI.neuralLastMod[i] = 0;

#if 1
#ifdef GAZZA_TEACH_CAR_UNDERSTEER
        Player->CarAI.neuralMin[0] = 0;     Player->CarAI.neuralMax[0] = 20;
        Player->CarAI.neuralMin[1] = 0;     Player->CarAI.neuralMax[1] = 100;
        Player->CarAI.neuralMin[2] = 0;     Player->CarAI.neuralMax[2] = 2;
#else
        Player->CarAI.neuralMin[0] = 0;     Player->CarAI.neuralMax[0] = 2000;  //300;
        Player->CarAI.neuralMin[1] = 0;     Player->CarAI.neuralMax[1] = 5000;  //2500;
        Player->CarAI.neuralMin[2] = 0;     Player->CarAI.neuralMax[2] = 4000;  //1000;
#endif
        Player->CarAI.neuralMin[3] = 0;     Player->CarAI.neuralMax[3] = 2000;  //500;
        Player->CarAI.neuralMin[4] = 0;     Player->CarAI.neuralMax[4] = 1;
        Player->CarAI.neuralMin[5] = 0;     Player->CarAI.neuralMax[5] = 4000;  //1000;
        Player->CarAI.neuralMin[6] = 0;     Player->CarAI.neuralMax[6] = 4000;  //2000;
        Player->CarAI.neuralMin[7] = 0;     Player->CarAI.neuralMax[7] = 1;
        Player->CarAI.neuralMin[8] = 0;     Player->CarAI.neuralMax[8] = 1000;  //250;
        Player->CarAI.neuralMin[9] = 0;     Player->CarAI.neuralMax[9] = 4000;  //1000;
#else
        Player->CarAI.neuralMin[0] = 3.32;      Player->CarAI.neuralMax[0] = 7.74;
        Player->CarAI.neuralMin[1] = 1597.51;   Player->CarAI.neuralMax[1] = 1899.13;
        Player->CarAI.neuralMin[2] = 504.03;    Player->CarAI.neuralMax[2] = 568.03;
        Player->CarAI.neuralMin[3] = 83.02;     Player->CarAI.neuralMax[3] = 128.3;
        Player->CarAI.neuralMin[4] = 0.4;       Player->CarAI.neuralMax[4] = 0.95;
        Player->CarAI.neuralMin[5] = 342.17;    Player->CarAI.neuralMax[5] = 454.72;
        Player->CarAI.neuralMin[6] = 172.6;     Player->CarAI.neuralMax[6] = 178.95;
        Player->CarAI.neuralMin[7] = 0.17;      Player->CarAI.neuralMax[7] = 1;
        Player->CarAI.neuralMin[8] = 6.72;      Player->CarAI.neuralMax[8] = 18.95;
        Player->CarAI.neuralMin[9] = 686.68;    Player->CarAI.neuralMax[9] = 727.45;
#endif

#ifdef GAZZA_TEACH_CAR_UNDERSTEER
        Player->CarAI.neuralCur[0] = Player->CarAI.neuralBest[0] = gGazzaQQQ;
        Player->CarAI.neuralCur[1] = Player->CarAI.neuralBest[1] = gGazzaWWW;
        Player->CarAI.neuralCur[2] = Player->CarAI.neuralBest[2] = gGazzaEEE;
#else
        Player->CarAI.neuralCur[0] = Player->CarAI.neuralBest[0] = Player->CarAI.understeerThreshold;
        Player->CarAI.neuralCur[1] = Player->CarAI.neuralBest[1] = Player->CarAI.understeerRange;
        Player->CarAI.neuralCur[2] = Player->CarAI.neuralBest[2] = Player->CarAI.understeerFront;
#endif
        Player->CarAI.neuralCur[3] = Player->CarAI.neuralBest[3] = Player->CarAI.understeerRear;
        Player->CarAI.neuralCur[4] = Player->CarAI.neuralBest[4] = Player->CarAI.understeerMax;
        Player->CarAI.neuralCur[5] = Player->CarAI.neuralBest[5] = Player->CarAI.oversteerThreshold;
        Player->CarAI.neuralCur[6] = Player->CarAI.neuralBest[6] = Player->CarAI.oversteerRange;
        Player->CarAI.neuralCur[7] = Player->CarAI.neuralBest[7] = Player->CarAI.oversteerMax;
        Player->CarAI.neuralCur[8] = Player->CarAI.neuralBest[8] = Player->CarAI.oversteerAccelThreshold;
        Player->CarAI.neuralCur[9] = Player->CarAI.neuralBest[9] = Player->CarAI.oversteerAccelRange;
#endif
}



/********************************************************************************
* CAI_NeuralNetwork()
********************************************************************************/
#ifdef NEURAL_NET
void CAI_NeuralNetworkInit(PLAYER *pPlayer)
{
    int cL;

    for (cL = 0; cL < NEURAL_LAYER_NUM; cL++)
    {
        pPlayer->CarAI.neuralNet.inputWeight[cL] = frand(1);
        pPlayer->CarAI.neuralNet.outputWeight[cL] = frand(1);
    }
}

void CAI_NeuralNetworkTrain(PLAYER *pPlayer)
{

    /*
    REAL    input[NEURAL_LAYER_NUM][NEURAL_INPUT_NUM];
    REAL    inputWeight[NEURAL_LAYER_NUM];
    REAL    layer[NEURAL_LAYER_NUM];
    REAL    outputWeight[NEURAL_LAYER_NUM];
    REAL    output[NEURAL_LAYER_NUM][NEURAL_OUTPUT_NUM];
    */
}

#endif

void CAI_NeuralNetwork(PLAYER *pPlayer)
{
#ifdef GAZZA_TEACH_CAR_HANDLING
    if (pPlayer->CarAI.lastLap != pPlayer->car.Laps)
    {
        pPlayer->CarAI.lastLap = pPlayer->car.Laps;
        pPlayer->CarAI.cLaps--;

        if (pPlayer->CarAI.bestTime > pPlayer->car.LastLapTime)
        {
            if (pPlayer->CarAI.bestTime != FLT_MAX)
                pPlayer->CarAI.fBestTime++;
            pPlayer->CarAI.bestTime = pPlayer->car.LastLapTime;
        }

        if (pPlayer->CarAI.cLaps <= 0)
        {
            REAL bias, last, range, value;
            int e, flag, count;

            pPlayer->CarAI.cLaps = 1;

            if (pPlayer->CarAI.fBestTime)
            {
                if ((e = pPlayer->CarAI.iLastNeuron) >= 0)
                {
                    if (pPlayer->CarAI.neuralCur[e] < pPlayer->CarAI.neuralBest[e])
                        pPlayer->CarAI.neuralMax[e] = pPlayer->CarAI.neuralBest[e];
                    else
                        pPlayer->CarAI.neuralMin[e] = pPlayer->CarAI.neuralBest[e];

                    bias = Sign(pPlayer->CarAI.neuralLastMod[e]);
                }
                else
                    bias = 0;

#ifdef GAZZA_TEACH_CAR_UNDERSTEER
                pPlayer->CarAI.neuralBest[0] = gGazzaQQQ;
                pPlayer->CarAI.neuralBest[1] = gGazzaWWW;
                pPlayer->CarAI.neuralBest[2] = gGazzaEEE;
#else
                pPlayer->CarAI.neuralBest[0] = pPlayer->CarAI.understeerThreshold;
                pPlayer->CarAI.neuralBest[1] = pPlayer->CarAI.understeerRange;
                pPlayer->CarAI.neuralBest[2] = pPlayer->CarAI.understeerFront;
#endif
                pPlayer->CarAI.neuralBest[3] = pPlayer->CarAI.understeerRear;
                pPlayer->CarAI.neuralBest[4] = pPlayer->CarAI.understeerMax;
                pPlayer->CarAI.neuralBest[5] = pPlayer->CarAI.oversteerThreshold;
                pPlayer->CarAI.neuralBest[6] = pPlayer->CarAI.oversteerRange;
                pPlayer->CarAI.neuralBest[7] = pPlayer->CarAI.oversteerMax;
                pPlayer->CarAI.neuralBest[8] = pPlayer->CarAI.oversteerAccelThreshold;
                pPlayer->CarAI.neuralBest[9] = pPlayer->CarAI.oversteerAccelRange;

                ReSaveCarInfoAIData(pPlayer);
            }
            else
            {
                if (pPlayer->CarAI.iLastNeuron >= 0)
                {
                    e = pPlayer->CarAI.iLastNeuron;
                    if (pPlayer->CarAI.neuralCur[e] < pPlayer->CarAI.neuralBest[e])
//                      pPlayer->CarAI.neuralMin[e] -= (pPlayer->CarAI.neuralMin[e] - pPlayer->CarAI.neuralCur[e]) * 0.5;
                        pPlayer->CarAI.neuralMin[e] = (pPlayer->CarAI.neuralMin[e] * 0.25) + (pPlayer->CarAI.neuralCur[e] * 0.75);
                    else
//                      pPlayer->CarAI.neuralMax[e] -= (pPlayer->CarAI.neuralMax[e] - pPlayer->CarAI.neuralCur[e]) * 0.5;
                        pPlayer->CarAI.neuralMax[e] = (pPlayer->CarAI.neuralMax[e] * 0.25) + (pPlayer->CarAI.neuralCur[e] * 0.75);

                    pPlayer->CarAI.neuralCur[e] = pPlayer->CarAI.neuralBest[e];
                }
                else
                {
                    bias = 0;
                }
            }

            if (bias == 0)
                bias = 1;

            pPlayer->CarAI.fBestTime = 0;

            count = 20;
            flag = 1;
            while (flag && (count-- > 0))
            {
#ifdef GAZZA_TEACH_CAR_UNDERSTEER
                e = rand() % 3;
                range = pPlayer->CarAI.neuralMax[e] - pPlayer->CarAI.neuralMin[e];
                if (range > 0.1)
#else
                e = rand() % 10;
                range = pPlayer->CarAI.neuralMax[e] - pPlayer->CarAI.neuralMin[e];
                if ((range > 1) || (e == 4) || (e == 7))
#endif
                {
                    last = pPlayer->CarAI.neuralCur[e];
//                  value = pPlayer->CarAI.neuralMin[e] + frand(range);
                    value = pPlayer->CarAI.neuralMin[e] + frand(range * 0.5) + (range * 0.25);

                    if (value <= pPlayer->CarAI.neuralMin[e])
                        value = pPlayer->CarAI.neuralMin[e];
                    else
                    if (value >= pPlayer->CarAI.neuralMax[e])
                        value = pPlayer->CarAI.neuralMax[e];
                    else
                    {
                        pPlayer->CarAI.neuralCur[e] = value;
                        pPlayer->CarAI.neuralLastMod[e] = pPlayer->CarAI.neuralCur[e] - last;
                        pPlayer->CarAI.iLastNeuron = e;
                        flag = 0;
                    }
                }
            }

#ifdef GAZZA_TEACH_CAR_UNDERSTEER
            gGazzaQQQ   = pPlayer->CarAI.neuralCur[0];
            gGazzaWWW   = pPlayer->CarAI.neuralCur[1];
            gGazzaEEE   = pPlayer->CarAI.neuralCur[2];
#else
            pPlayer->CarAI.understeerThreshold      = pPlayer->CarAI.neuralCur[0];
            pPlayer->CarAI.understeerRange          = pPlayer->CarAI.neuralCur[1];
            pPlayer->CarAI.understeerFront          = pPlayer->CarAI.neuralCur[2];
#endif
            pPlayer->CarAI.understeerRear           = pPlayer->CarAI.neuralCur[3];
            pPlayer->CarAI.understeerMax            = pPlayer->CarAI.neuralCur[4];
            pPlayer->CarAI.oversteerThreshold       = pPlayer->CarAI.neuralCur[5];
            pPlayer->CarAI.oversteerRange           = pPlayer->CarAI.neuralCur[6];
            pPlayer->CarAI.oversteerMax             = pPlayer->CarAI.neuralCur[7];
            pPlayer->CarAI.oversteerAccelThreshold  = pPlayer->CarAI.neuralCur[8];
            pPlayer->CarAI.oversteerAccelRange      = pPlayer->CarAI.neuralCur[9];
        }
    }
#endif
}


/********************************************************************************
* CAI_CalcCarRacePositions()
********************************************************************************/
void CAI_CalcCarRacePositions(void)
{
    PLAYER  *pPlayer;
    int     i;
    int     flag;

    if (GameSettings.GameType == GAMETYPE_CALCSTATS) return;

// Calculate race distance travelled for all cars
    CAI_CalcCarRaceDistances();

// Set up car race distance's
            #if 0
    for (pPlayer = PLR_PlayerHead; pPlayer; pPlayer = pPlayer->next)
    {
#ifndef _PSX
        if (GameSettings.GameType == GAMETYPE_NETWORK_BATTLETAG)
            pPlayer->CarAI.raceDistance = (REAL)(BOMBTAG_MAX_TIME - pPlayer->BombTagTimer);
        else
//          pPlayer->CarAI.raceDistance = (REAL)pPlayer->car.Laps - pPlayer->CarAI.FinishDistPanel - (REAL)pPlayer->CarAI.BackTracking;
            pPlayer->CarAI.raceDistance = (pPlayer->car.Laps << 16) - (pPlayer->CarAI.FinishDistPanel * 65536) - (pPlayer->CarAI.BackTracking << 16);
#else
        if (GameSettings.GameType == GAMETYPE_NETWORK_BATTLETAG)
//          pPlayer->CarAI.raceDistance = (REAL)(BOMBTAG_MAX_TIME - player->BombTagTimer) / 1000.0f;
            pPlayer->CarAI.raceDistance = 0;//(REAL)(BOMBTAG_MAX_TIME - player->BombTagTimer);
        else
            pPlayer->CarAI.raceDistance = (pPlayer->car.Laps << 16) - (pPlayer->CarAI.FinishDistPanel) - (pPlayer->CarAI.BackTracking << 16);
#endif
    }
            #endif


// Sort car race table
    if (PLR_PlayerHead->CarAI.racePosition == -1)
    {
        i = 0;
        for (pPlayer = PLR_PlayerHead; pPlayer; pPlayer = pPlayer->next)
        {
            gpCarRaceOrder[i] = pPlayer;
            pPlayer->CarAI.racePosition = i;
            i++;
        }
    }
    else
    {
        for (pPlayer = PLR_PlayerHead; pPlayer; pPlayer = pPlayer->next)
        {
            flag = 0;

        // Sort up
            i = pPlayer->CarAI.racePosition - 1;
            while (i >= 0)
            {
                if (pPlayer->CarAI.raceDistance > gpCarRaceOrder[i]->CarAI.raceDistance)
                {
#ifdef _PC
//                  if (gpCarRaceOrder[i]->type == PLAYER_LOCAL)
//                      PlaySfx3D(SFX_HONK, SFX_MAX_VOL, 22050, &pPlayer->car.Body->Centre.Pos, 2);
#endif
                    gpCarRaceOrder[i+1] = gpCarRaceOrder[i];
                    gpCarRaceOrder[i+1]->CarAI.racePosition++;
                    pPlayer->CarAI.racePosition--;
                    flag++;
                }
                else
                {
                    gpCarRaceOrder[i+1] = pPlayer;
                    break;
                }

                i--;
            }

            if ((i < 0) && (flag))
                gpCarRaceOrder[0] = pPlayer;

        // Sort down
            if (!flag)
            {
                i = pPlayer->CarAI.racePosition + 1;
                while (i < StartData.PlayerNum)
                {
                    if (pPlayer->CarAI.raceDistance < gpCarRaceOrder[i]->CarAI.raceDistance)
                    {
#ifdef _PC
//                  if (pPlayer->type == PLAYER_LOCAL)
//                      PlaySfx3D(SFX_HONK, SFX_MAX_VOL, 22050, &pPlayer->car.Body->Centre.Pos, 2);
#endif
                        gpCarRaceOrder[i-1] = gpCarRaceOrder[i];
                        gpCarRaceOrder[i-1]->CarAI.racePosition--;
                        pPlayer->CarAI.racePosition++;
                        flag++;
                    }
                    else
                    {
                        gpCarRaceOrder[i-1] = pPlayer;
                        break;
                    }

                    i++;
                }

                if ((i == StartData.PlayerNum) && (flag))
                    gpCarRaceOrder[StartData.PlayerNum-1] = pPlayer;
            }
        }
    }

            // DEBUG
    #if 0
#ifdef _PC
            {
                int pos[32];
                REAL dist[32];

                for (i = 0; i < StartData.PlayerNum; i++)
                {
                    pos[i] = gpCarRaceOrder[i]->CarAI.racePosition;
                    dist[i] = gpCarRaceOrder[i]->CarAI.raceDistance;
                }
            }
#endif
    #endif
}


/********************************************************************************
* CAI_InitPickupWeights()
********************************************************************************/
void CAI_InitPickupWeights(void)
{
//  dd
}


//--------------------------------------------------------------------------------------------------------------------------

//
// CAI_CarHelper
//
// Checks the cars "condition" to see if it needs correcting. This includes, checking if the car is on valid track, in a valid
// zone and facing the right way, etc
//

void CAI_CarHelper(PLAYER *Player)
{
    int flag;

    Player->CarAI.IsOnTrack = CAI_IsCarOnTrack(Player);


    flag = 0;
    if (GetLevelInfo(GameSettings.Level)->TrackType == TRACK_TYPE_USER)
        flag = 1;
    else
    {
#ifdef _PC
        if ( ((Player->type != PLAYER_LOCAL) || ((Version == VERSION_DEV) && (gGazzasAICar))) &&
            (Player->type != PLAYER_REPLAY))
            flag = 1;
#else
        if ( (Player->type != PLAYER_LOCAL) && (Player->type != PLAYER_FRONTEND) && (Player->type != PLAYER_REPLAY))
            flag = 1;
#endif
    }

    if (flag)
    {

// Is the car in a zone ?
        if (Player->type != PLAYER_FRONTEND) {
            if (!Player->CarAI.IsInZone)
            {
                Player->CarAI.cZoneReset += TimeStep;
                if (Player->CarAI.cZoneReset > MAX_OUT_OF_ZONE_CNT)
                {
    //              CAI_ResetCar(Player);
    //              StartCarReposition(Player);

                    Player->controls.digital |= CTRL_REPOSITION;
                    Player->CarAI.cZoneReset = 0;
                }
            }
            else
            {
                Player->CarAI.cZoneReset = 0;
            }

// Is the car in a node ?
            if (!Player->CarAI.pCurNode)
            {
                Player->CarAI.cNodeReset += TimeStep;
                if (Player->CarAI.cNodeReset > MAX_OUT_OF_NODE_CNT)
                {
                    Player->controls.digital |= CTRL_REPOSITION;
                    Player->CarAI.cNodeReset = 0;
                }
            }
            else
            {
                Player->CarAI.cNodeReset = 0;
            }
        }

// Is the car stuck ?
        if (CAI_IsCarStuck(Player))
        {
            Player->CarAI.StuckCnt += TimeStep;
            if (Player->CarAI.StuckCnt > gpCatchUpVars->flipCarTime)
            {
                CAI_FlipCar(Player);
                Player->CarAI.StuckCnt = ZERO;
            }
        }
        else
        {
            Player->CarAI.StuckCnt = ZERO;
        }
    }
}

//--------------------------------------------------------------------------------------------------------------------------

//
// CAI_FlipCar
//
// turns the car over
//

void CAI_FlipCar(PLAYER *Player)
{
    // Don't do anything for Rotor
    if (Player->car.CarType == CARID_ROTOR) return;

    // Set car to turn itself over
    //Player->car.RightingCollide = TRUE;
    //Player->car.RightingReachDest = FALSE;
    //Player->ownobj->movehandler = (MOVE_HANDLER)MOV_RightCar; 
    Player->controls.digital |= CTRL_RESET;

// Set ai state
    Player->CarAI.AIState = CAI_S_RACE;
}

//
// CAI_ResetCar
//
// Places car back on track at last valid node and zone, facing in direction of tavel
//

void CAI_ResetCar(PLAYER *Player)
{
    PLAYER *Player2;
    MAT     mat;
    AINODE  *pNode, *pNodeN;
    VEC     pos[2];
//  int     i;

// Set ai state
    Player->CarAI.AIState = CAI_S_RACE;
    Player->CarAI.beechedPosCount = gpCatchUpVars->beachMaxTime;

// reset to lap start if time trial

    if (GameSettings.GameType == GAMETYPE_TRIAL)
    {
        Player->car.NextSplit = 0;
        Player->car.NextTrackDir = 0;

        GetCarStartGrid(MAX_RACE_CARS - 1, pos, &mat);
        SetCarPos(&Player->car, pos, &mat);

        Player->CarAI.ZoneID = 0;
        Player->CarAI.LastValidZone = 0;
        CAI_IsCarInZone(Player);

        Player->CarAI.FinishDistNode = PosStartNode;
        Player->CarAI.FinishDist = Real(0);
        Player->CarAI.FinishDistPanel = Real(0);
        Player->CarAI.BackTracking = TRUE;
        Player->CarAI.PreLap = TRUE;
        UpdateCarAiZone(Player);
        UpdateCarFinishDist(Player, NULL);

#ifdef _PSX

        TotalRaceTime  = 0;
        Player->car.CurrentLapTime= 0;
#endif


#ifdef _PC
        if (GHO_GhostPlayer)
        {
            GHO_GhostPlayer->CarAI.ZoneID = 0;
            GHO_GhostPlayer->CarAI.LastValidZone = 0;
            CAI_IsCarInZone(GHO_GhostPlayer);

            GHO_GhostPlayer->CarAI.FinishDistNode = PosStartNode;
            GHO_GhostPlayer->CarAI.FinishDist = Real(0);
            GHO_GhostPlayer->CarAI.FinishDistPanel = Real(0);
            GHO_GhostPlayer->CarAI.BackTracking = TRUE;
            GHO_GhostPlayer->CarAI.PreLap = TRUE;
            UpdateCarAiZone(GHO_GhostPlayer);
            UpdateCarFinishDist(GHO_GhostPlayer, NULL);

            InitGhostData(&Players[0]);
        }
#endif

        return;
    }

// reset to start pos if stunt arena

//#ifndef _PSX
    if (GameSettings.GameType == GAMETYPE_TRAINING)
    {
        RotMatrixY(&mat, -LEV_StartRot);
        SetCarPos(&Player->car, &LEV_StartPos, &mat);
        return;
    }
//#endif

// Too far behind player's car ?
    Player2 = &Players[0];
//  if ((Player->type == PLAYER_CPU) && ((Players[0].CarAI.raceDistance - Player->CarAI.raceDistance) > CATCHUP_RESTART_DIST))
    if (0)
    {
        int         iPos;
#if 0
        POSNODE*    pPosNode;
        int         finishLineFlag;
        REAL        distRef;

        // Setup posnode close to local player
        pPosNode = &PosNode[Player2->CarAI.FinishDistNode];
        distRef = pPosNode->Dist;
        if (pPosNode->Dist == 0)
            finishLineFlag = 1;
        else
            finishLineFlag = 0;

        while (1)
        {
            pPosNode = pPosNode->Prev[0];
            if (pPosNode->Dist == 0)
                finishLineFlag = 1;

            if (abs(pPosNode->Dist - distRef) >= CATCHUP_RESTART_DIST)
                break;
        }

        CopyVec(&pPosNode->Pos, &Player->car.Body->Centre.Pos);

        // Find zone car is in
        for (i = 0; i < AiZoneNumID; i++)
        {
            if (AIZ_IsCarInZoneID(Player, i))
            {
                Player->CarAI.IsInZone = TRUE;
                Player->CarAI.ZoneID = Player->CarAI.CurZoneID;
                break;
            }
        }

        // Find node car is in
        pNode = AIN_FindZoneNodePlayerIsIn(Player);

        if (pNode == NULL)
        {
            if (!(pNode = Player->CarAI.pCurNode))
                pNode = Player->CarAI.pLastValidNode;
        }

        // Setup current lap#
        Player->CarAI.FinishDistNode = (long)(pPosNode - PosNode);
        Player->car.Laps = Player2->car.Laps - finishLineFlag;
#else
        // Find car to place me at.
        iPos = Players[0].CarAI.racePosition + 2;
        if (iPos > StartData.PlayerNum-1)
            iPos = StartData.PlayerNum-1;
        Player2 = gpCarRaceOrder[iPos];

        // Copy position info
        Player->CarAI.FinishDistNode    = Player2->CarAI.FinishDistNode;
        Player->CarAI.FinishDist        = Player2->CarAI.FinishDist;
        Player->CarAI.FinishDistPanel   = Player2->CarAI.FinishDistPanel;
        Player->car.Laps                = Player2->car.Laps;
        Player->CarAI.pCurNode          = Player2->CarAI.pCurNode;
        Player->CarAI.pLastValidNode    = Player2->CarAI.pLastValidNode;
        Player->CarAI.ZoneID            = Player2->CarAI.ZoneID;
        Player->CarAI.CurZone           = Player2->CarAI.CurZone;
        Player->CarAI.CurZoneBBox       = Player2->CarAI.CurZoneBBox;
        Player->CarAI.CurZoneID         = Player2->CarAI.CurZoneID;
        Player->CarAI.LastValidZone     = Player2->CarAI.LastValidZone;
        Player->CarAI.BackTracking      = Player2->CarAI.BackTracking;
        Player->CarAI.PreLap            = Player2->CarAI.PreLap;
        Player->car.CurrentLapStartTime = Player2->car.CurrentLapStartTime;

        if (!(pNode = Player2->CarAI.pCurNode))
            pNode = Player2->CarAI.pLastValidNode;
#endif
    }
    else
    {
        if (!(pNode = Player->CarAI.pCurNode))
            pNode = Player->CarAI.pLastValidNode;
    }


    if (pNode)
    {
    // Don't start on a JUMPWALL LINK !!!
        while (pNode->Next[0]->Priority == AIN_TYPE_JUMPWALL)
        {
            if (!(pNode = pNode->Next[0]))
                return;
        }

    // Set car's node to the last valid node.  If CPU car far behind, place closer to player
        Player->CarAI.pCurNode = Player->CarAI.pLastValidNode = pNode;

    // Create position on racing line
        //NOTE: Should move along racing line about 1-2 meters as not to get stuck on stairs etc...  BUT must check
        //      that we are still inside the node (or call. GetNearestNode() which is easier!)
        pNodeN = pNode->Next[0];
        InterpVec(&pNode->Node[0].Pos, &pNode->Node[1].Pos, pNode->RacingLine, &pos[0]);
        InterpVec(&pNodeN->Node[0].Pos, &pNodeN->Node[1].Pos, pNodeN->RacingLine, &pos[1]);
        InterpVec(&pos[0], &pos[1], DivScalar(TO_LENGTH(Real(100)), pNode->link.dist), &Player->car.Body->Centre.Pos);

//      InterpVec(&Player->CarAI.pCurNode->Node[0].Pos, &Player->CarAI.pCurNode->Node[1].Pos,
//                 Player->CarAI.pCurNode->RacingLine, &Player->car.Body->Centre.Pos);
        Player->car.Body->Centre.Pos.v[Y] -= TO_LENGTH(Real(100));

    // Create matrix from the node's forward vector & the world up vector
        CopyVec(&Player->CarAI.pCurNode->link.forwardVec, &mat.mv[L]);
        SetVec(&mat.mv[U], ZERO, -ONE, ZERO);
        CrossProduct(&mat.mv[L], &mat.mv[U], &mat.mv[R]);
        NormalizeVector(&mat.mv[R]);
        CrossProduct(&mat.mv[L], &mat.mv[R], &mat.mv[U]);

    // Set car position
        SetCarPos(&Player->car, &Player->car.Body->Centre.Pos, &mat);
    }
}


/********************************************************************************
* CAI_IsCarStuck()
*
* Checks if the specified car is in a zone or not, returning TRUE if in a zone
* CAI_IsCarStuck
*
* Checks the cars orientation, velocity and state to determine if the car is "stuck", eg. crashed
*
********************************************************************************/
bool CAI_IsCarStuck(PLAYER *Player)
{
    CAR     *car;
    VEC     delta;
    REAL    dist;

    car = &Player->car;

    if (car->Righting)
        return FALSE;

    if (car->NWheelFloorContacts == 0)
    {
        if ((car->Body->NWorldContacts != 0) || (car->Body->NoContactTime < TO_TIME(Real(0.1))) || (car->NWheelColls > 0))
        {
            return TRUE;
        }
    }

// Is car beeched ?
    if ((Player->type == PLAYER_FRONTEND) || (Player->type == PLAYER_LOCAL))
        return FALSE;

    if ((CountdownTime == ZERO) && (!Player->RaceFinishTime))
    {
        Player->CarAI.beechedPosCount -= TimeStep;
        if (Player->CarAI.beechedPosCount <= ZERO)
        {
            Player->CarAI.beechedPosCount = gpCatchUpVars->beachMaxTime;

#ifndef _PSX
            delta.v[X] = Player->car.Body->Centre.Pos.v[X] - Player->CarAI.beechedPos.v[X];
            delta.v[Y] = Player->car.Body->Centre.Pos.v[Y] - Player->CarAI.beechedPos.v[Y];
            delta.v[Z] = Player->car.Body->Centre.Pos.v[Z] - Player->CarAI.beechedPos.v[Z];
            CopyVec(&Player->car.Body->Centre.Pos, &Player->CarAI.beechedPos);
            dist = VecDotVec(&delta, &delta);

            if (dist <= gpCatchUpVars->beachMaxDist2)
#else
            delta.v[X] = (Player->car.Body->Centre.Pos.v[X] - Player->CarAI.beechedPos.v[X]) >> (16+PSX_LENGTH_SHIFT);
            delta.v[Y] = (Player->car.Body->Centre.Pos.v[Y] - Player->CarAI.beechedPos.v[Y]) >> (16+PSX_LENGTH_SHIFT);
            delta.v[Z] = (Player->car.Body->Centre.Pos.v[Z] - Player->CarAI.beechedPos.v[Z]) >> (16+PSX_LENGTH_SHIFT);
            CopyVec(&Player->car.Body->Centre.Pos, &Player->CarAI.beechedPos);
            dist = (delta.v[X] * delta.v[X]) + (delta.v[Y] * delta.v[Y]) + (delta.v[Z] * delta.v[Z]);

            if (dist <= gpCatchUpVars->beachMaxDist2)
#endif
            {
                //StartCarReposition(Player);
                Player->controls.digital |= CTRL_REPOSITION;
                return FALSE;
            }
        }
    }
    else
        Player->CarAI.beechedPosCount = gpCatchUpVars->beachMaxTime;

    return(FALSE);
}


/********************************************************************************
* CAI_IsCarInZone()
*
* Checks if the specified car is in a zone or not, returning TRUE if in a zone
********************************************************************************/
bool CAI_IsCarInZone(PLAYER *Player)
{
    long    ii, cZ;
    long    curzone = 0;
    CAR *car;

    if (!AiZones)
    {
        Player->CarAI.LastValidZone = Player->CarAI.CurZone;
        Player->CarAI.CurZone = -1;
        Player->CarAI.CurZoneID = -1;
        Player->CarAI.CurZoneBBox = -1;
        return(FALSE);
    }

    car = &Player->car;

    for (cZ = 0; cZ < AiZoneNumID; cZ++)                            // Check car centre against all AI zones
    {
        ii = cZ;
        if (AIZ_IsCarInZoneID(Player, ii))
            return TRUE;
    }
    return(FALSE);
}



//--------------------------------------------------------------------------------------------------------------------------

//
// CAI_IsCarOnTrack
//
// Checks if the specified car is in contact with an OOB collison poly. Returns FALSE if yes.
//

bool CAI_IsCarOnTrack(PLAYER *Player)
{
    CAR *car;

    car = &Player->car;

    return(TRUE);
}

//--------------------------------------------------------------------------------------------------------------------------

// AI Home trigger func

void CAI_TriggerAiHome(PLAYER *Player, long flag, long n, PLANE *planes)
{
    CAR *car;

    car = &Player->car;
}



/********************************************************************************
* CAI_CalcCarRaceDistances()
********************************************************************************/
void CAI_CalcCarRaceDistances(void)
{
    PLAYER  *pPlayer;
    int     largestDist;
    PLAYER  *pPlayer1st;

    largestDist = -INT_MAX;

    for (pPlayer = PLR_PlayerHead; pPlayer ; pPlayer = pPlayer->next)
    {
        if (pPlayer->type != PLAYER_NONE)
        {
#ifndef _PSX
            pPlayer->CarAI.raceDistance = Int(((pPlayer->car.Laps - pPlayer->CarAI.BackTracking) * PosTotalDist) - pPlayer->CarAI.FinishDist);
#else
            pPlayer->CarAI.raceDistance = ((pPlayer->car.Laps - pPlayer->CarAI.BackTracking) * PosTotalDist) - pPlayer->CarAI.FinishDist;
#endif
            if (pPlayer->CarAI.raceDistance > largestDist)
            {
                largestDist = pPlayer->CarAI.raceDistance;
                pPlayer1st = pPlayer;
            }
        }
    }

    gpPlayer1st = pPlayer1st;
}


/********************************************************************************
* CAI_FindClosestPlayer()
*
* Find the closest player infront & behind.
*
* Note: CAI_UpdateAiParameters must be called before this as it needs some of the
*       variables that this function creates.
********************************************************************************/
void CAI_FindClosestPlayer(PLAYER *pPlayer, PLAYER **pFront, PLAYER **pBehind, REAL *pDistF, REAL *pDistB)
{
    PLAYER  *pTarget;
    VEC     delta;
    REAL    dist;
    REAL    side;
    long    zoneDiff;

//  OBJECT  *OBJ_ObjectHead;

    // Loop over other players
    *pDistF = *pDistB = LARGEDIST;
    *pFront = *pBehind = NULL;
    for (pTarget = PLR_PlayerHead; pTarget != NULL; pTarget = pTarget->next)
    {
        // Only target other players and CPU cars
        if ((pTarget != pPlayer) && (pTarget->type != PLAYER_GHOST) && (pTarget->type != PLAYER_NONE))
        {
            // Does the target have the same or adjacent zone ID ?
            zoneDiff = pTarget->CarAI.CurZoneID - pPlayer->CarAI.CurZoneID;
            if ((zoneDiff >= -1) && (zoneDiff <= 1))
            {
                VecMinusVec(&pTarget->car.Body->Centre.Pos, &pPlayer->car.Body->Centre.Pos, &delta);
                //dist = VecLen2(&delta);
                FastLength(&delta, &dist);

                // Front or behind ?
                side = VecDotVec(&delta, &pPlayer->CarAI.forwardVec);

                if (side >= Real(0))
                {
                    if (dist < *pDistF)
                    {
                        *pDistF = dist;
                        *pFront = pTarget;
                    }
                }
                else
                {
                    if (dist < *pDistB)
                    {
                        *pDistB = dist;
                        *pBehind = pTarget;
                    }
                }
            }
        }
    }
}

/********************************************************************************
* CAI_ChooseRoute(PLAYER *pPlayer)
*
* Choose best route to take (ie. Racing line, pickup route, bumpy route.
********************************************************************************/
void CAI_ChooseRoute(PLAYER *pPlayer)
{
// Editing stuff
#ifdef _PC

    if ((Version == VERSION_DEV) && gGazzasRouteChoice)
    {
        if (Keys[DIK_LCONTROL] && Keys[DIK_LSHIFT])
        {
            if (Keys[DIK_O] && !LastKeys[DIK_O])
                gGazzasOvertake = !gGazzasOvertake;

            if (Keys[DIK_COMMA] && !LastKeys[DIK_COMMA])
            {
//                  SetJoyForces(-10000);

                if (giGazzaForceRoute == LONG_MAX)
                    giGazzaForceRoute = 0;
                else
                {
                    giGazzaForceRoute -= 1;
                    if (giGazzaForceRoute < 0)
                        giGazzaForceRoute = AIN_TYPE_NUM-1;
                }
            }
            if (Keys[DIK_PERIOD] && !LastKeys[DIK_PERIOD])
            {
//                  SetJoyForces(10000);

                if (giGazzaForceRoute == LONG_MAX)
                    giGazzaForceRoute = 0;
                else
                {
                    giGazzaForceRoute += 1;
                    if (giGazzaForceRoute >= AIN_TYPE_NUM)
                        giGazzaForceRoute = 0;
                }
            }
            if (Keys[DIK_SLASH])
            {
//                  SetJoyForces(0);
                giGazzaForceRoute = LONG_MAX;
            }
        }
    }

#endif

// Choose route
    if (pPlayer->CarAI.pCurNode)
    {
        // Choose route
        pPlayer->CarAI.iRouteCurNode = (short)CAI_ChooseNodeRoute(pPlayer, pPlayer->CarAI.pCurNode);

        // If there was a split node, a route was taken
        if (pPlayer->CarAI.pCurNode->Next[1])
            pPlayer->CarAI.bRouteTaken = TRUE;
    }
    else
        pPlayer->CarAI.iRouteCurNode = 0;
}

///////////////////////////////////////////////////////////
// Given a node, choose the route based on the players state
int CAI_ChooseNodeRoute(PLAYER *pPlayer, AINODE *pNode)
{
    int i;
    int rating[MAX_AINODE_LINKS];
    int iBest;

// Quick get out if only 1 route !
    if (!pNode->Next[1])
        return 0;

// DEBUG/EDITOR STUFF
#ifdef _PC
    if ((Version == VERSION_DEV) && gGazzasRouteChoice)
    {
//      pPlayer->CarAI.bOvertake = 0;
//      pPlayer->CarAI.iRouteCurNode = 0;

        if (!pNode)
            return 0;

//      if (gGazzasOvertake)
//          pPlayer->CarAI.bOvertake = gGazzasOvertake;

        if (giGazzaForceRoute != LONG_MAX)
        {
//          pPlayer->CarAI.RouteChoice = CAI_ChooseNodeRoute(pPlayer, pNode);
            for (i = 0; i < MAX_AINODE_LINKS; i++)
            {
                if (pNode->Next[i])
                {
                    if (pNode->Next[i]->Priority == giGazzaForceRoute)
                    {
                        return i;
                    }
                }
            }
        }
    }
#endif


// Find the priority route
    for (i = 0; i < MAX_AINODE_LINKS; i++)
    {
        if (pNode->Next[i])
        {
            switch (pNode->Next[i]->Priority)
            {
                default:
                case AIN_TYPE_RACINGLINE:
                case AIN_TYPE_TURBOLINE:
                case AIN_TYPE_OFFTHROTTLE:
                case AIN_TYPE_OFFTHROTTLEPETROL:
                case AIN_TYPE_SLOWDOWN_15:
                case AIN_TYPE_SLOWDOWN_20:
                case AIN_TYPE_SLOWDOWN_25:
                case AIN_TYPE_SLOWDOWN_30:
                case AIN_TYPE_TITLESCR_SLOWDOWN:
                    rating[i] = 0;
                    break;

                case AIN_TYPE_WILDERNESS:
                case AIN_TYPE_STAIRS:
                    rating[i] = -1000;
                    break;

                case AIN_TYPE_JUMPWALL:
                    rating[i] = -100;
                    break;

                case AIN_TYPE_LONGCUT:
                    rating[i] = -500;
                    break;

                case AIN_TYPE_SHORTCUT:
                    rating[i] = 100;
                    break;

                case AIN_TYPE_LONGPICKUP:
                case AIN_TYPE_PICKUP:
                    rating[i] = pPlayer->CarAI.Skills.pickupBias - pPlayer->CarAI.routeRandom[i];
                    break;

                case AIN_TYPE_BUMPY:
                case AIN_TYPE_SOFTSUSPENSION:
                    rating[i] = pPlayer->CarAI.Skills.suspension - pPlayer->CarAI.routeRandom[i];
                    break;

                case AIN_TYPE_BARRELBLOCK:
                    rating[i] = -INT_MAX;
                    break;
            }
        }
        else
            rating[i] = -INT_MAX;
    }

    if (rating[0] >= rating[1])
        iBest = 0;
    else
        iBest = 1;

//#ifdef _N64
    Assert(pNode->Next[iBest] != 0);
//#endif

    return iBest;

}


/********************************************************************************
* CAI_ProcessCarSteering(PLAYER *pPlayer)
*
* Process car steering determined by it's AI variables.
*
* AI Vars needed:
*   angleToRacingLine
*   SteerRateSin
********************************************************************************/
void CAI_ProcessCarSteering(PLAYER *pPlayer)
{
    REAL    angle;
    REAL    steer;

    angle = Real(1) - pPlayer->CarAI.absCosAngleToRacingLine;

#ifdef _N64
    if (angle < 0.f)
        {
        angle = 0.f;
        printf ("WARNING: absCosAngleToRacingLine > 1.f\n");
        }
    else if (angle > 1.f)
        {
        angle = 1.f;
        printf ("WARNING: absCosAngleToRacingLine < 0.f\n");
        }
#endif

    angle = (REAL)sqrt(angle);
    angle = MulScalar(angle, pPlayer->car.AISteerConvert);


#ifdef OLD_AI_VERSION
    if (angle < ZERO)           angle = ZERO;
    else if (angle > ONE)       angle = ONE;
#else
    if (angle < ZERO)
        angle = ZERO;
    else
    {
        angle = MulScalar(angle, Real(0.5));
        if (angle > ONE)
            angle = ONE;
    }
#endif

#ifndef _PSX
    steer = MulScalar(Real(CTRL_RANGE_MAX), angle);

    if (pPlayer->CarAI.dstRaceLineSide >= ZERO)
        steer = -steer;

    if (steer < Real(-CTRL_RANGE_MAX))  steer = Real(-CTRL_RANGE_MAX);
    if (steer > Real(CTRL_RANGE_MAX))   steer = Real(CTRL_RANGE_MAX);

    pPlayer->controls.dx = Int(steer);
#else
    steer = (CTRL_RANGE_MAX * angle) >> 16;

    if (pPlayer->CarAI.dstRaceLineSide >= ZERO)
        steer = -steer;

    if (steer < -CTRL_RANGE_MAX)    steer = -CTRL_RANGE_MAX;
    if (steer > CTRL_RANGE_MAX)     steer = CTRL_RANGE_MAX;

    pPlayer->controls.dx = steer;
#endif
}


/********************************************************************************
* CAI_CalcBrakingParameters(PLAYER *pPlayer)
*
* Calculate braking distance and speed
********************************************************************************/
bool CAI_CalcBrakingParameters(PLAYER *pPlayer, REAL *pBrakeDist, REAL *pSpeed)
{
#ifndef _PSX
    AINODE          *pNode;
    AINODE_LINKINFO *pLinkInfo;
    REAL            brakeDistMin;
    REAL            brakeDistLen;

    if (!(pNode = pPlayer->CarAI.pCurNode))
    {
        *pSpeed = Real(1);
        return FALSE;
    }
    pLinkInfo = &pNode->link;

//  brakeDistMin = TO_LENGTH(Real(200*2));
//  brakeDistLen = TO_LENGTH(Real(200*15));

    brakeDistMin = TO_LENGTH(Real(200*2));
    brakeDistLen = TO_LENGTH(Real(200*10));

//  brakeDistMin = TO_LENGTH(Real(200*1));
//  brakeDistLen = brakeDistMin + DivScalar(MulScalar(TO_LENGTH(Real(200*10)), pPlayer->CarAI.speedCur), pPlayer->car.TopSpeed);

#if 1
    *pBrakeDist = pLinkInfo->dist - brakeDistMin - MulScalar(brakeDistLen, Real(1) - pLinkInfo->speed);
    if (pPlayer->CarAI.distAlongNode >= *pBrakeDist)
    {
        *pSpeed = pLinkInfo->speed;
        return TRUE;
    }
#else
    distToGo = TO_LENGTH(Real(200*10));                             // Dist. we want to scan
    distToGo -= pLinkInfo->dist - pPlayer->CarAI.distAlongNode;     // Minus distance to go along the current node
    speed = pLinkInfo->speed[pPlayer->CarAI.RouteChoice];
    while (pNode && (distToGo >= Real(0.0)))
    {
        pNode = pNode->Next[pPlayer->CarAI.RouteChoice];
        pLinkInfo = &pNode->linkInfo[pPlayer->CarAI.RouteChoice];
        distToGo -= pLinkInfo->dist;
        if (speed > pLinkInfo->speed[pPlayer->CarAI.RouteChoice])
            speed = pLinkInfo->speed[pPlayer->CarAI.RouteChoice];
    }

    *pBrakeDist = pLinkInfo->dist - brakeDistMin - MulScalar(brakeDistLen, Real(1) - speed);
    if (pPlayer->CarAI.distAlongNode >= *pBrakeDist)
    {
        *pSpeed = speed;
        return TRUE;
    }
#endif
#endif  // #ifndef _PSX

    return FALSE;
}


/********************************************************************************
* CAI_QuantizeControls(CTRL *pControls)
*
* Quantize controls
********************************************************************************/
void CAI_QuantizeControls(CTRL *pControls)
{
    int tempI;

    // Quantize controller dx
    if (pControls->dx >= 0)
    {
//      tempI = (pControls->dx + 8) & ~15;
        tempI = (pControls->dx + 4) & ~7;
        if (tempI > CTRL_RANGE_MAX)
            tempI = CTRL_RANGE_MAX;
    }
    else
    {
//      tempI = -((-pControls->dx + 8) & ~15);
        tempI = -((-pControls->dx + 4) & ~7);
        if (tempI < -CTRL_RANGE_MAX)
            tempI = -CTRL_RANGE_MAX;
    }
    pControls->dx = (char)tempI;

// Quantize controller dy
    if (pControls->dy >= 0)
    {
//      tempI = (pControls->dy + 8) & ~15;
        tempI = (pControls->dy + 4) & ~7;
        if (tempI > CTRL_RANGE_MAX)
            tempI = CTRL_RANGE_MAX;
    }
    else
    {
//      tempI = -((-pControls->dy + 8) & ~15);
        tempI = -((-pControls->dy + 4) & ~7);
        if (tempI < -CTRL_RANGE_MAX)
            tempI = -CTRL_RANGE_MAX;
    }
    pControls->dy = (char)tempI;
}


/********************************************************************************
* CAI_UpdateTimeStep(PLAYER *pPlayer)
*
* Update time step
********************************************************************************/
void CAI_InitTimeStep(void)
{
    gCAI_TimeStep = 0;
    gCAI_cProcessAI = FALSE;
}

void CAI_UpdateTimeStep(int dT)
{
    gCAI_TimeStep -= dT;
    if (gCAI_TimeStep < ZERO)
    {
        gCAI_TimeStep += CAI_TIME_STEP_MS;
        gCAI_cProcessAI = TRUE;
    }
    else
    {
        gCAI_cProcessAI = FALSE;
    }
}


/********************************************************************************
* CAI_Process(PLAYER *pPlayer)
*
* Car AI Processing
********************************************************************************/
void CAI_Process(PLAYER *pPlayer)
{
    REAL timeStepSave;

    if (gCAI_cProcessAI)
    {
        timeStepSave = TimeStep;
        TimeStep = CAI_TIME_STEP_SECS;

        CAI_ProcessCall(pPlayer);

        TimeStep = timeStepSave;
    }
    else
    {
        pPlayer->controls.dx = pPlayer->CarAI.dx;
        pPlayer->controls.dy = pPlayer->CarAI.dy;
        pPlayer->controls.digital |= pPlayer->CarAI.digital;// & ~CTRL_CATCHUP_MASK;
     }
}


/********************************************************************************
* CAI_ProcessCall(PLAYER *pPlayer)
*
* Car AI Processing
********************************************************************************/
void CAI_ProcessCall(PLAYER *pPlayer)
{
    AINODE  *pNode;

    pNode = pPlayer->CarAI.pCurNode;

#ifdef GAZZA_TEACH_CAR_HANDLING
    CAI_NeuralNetwork(pPlayer);
#endif

// Update racing/overtaking line bias
    if (pNode)
    {
        if (pPlayer->CarAI.bOvertake != pPlayer->CarAI.bOvertakeLast)
        {
            pPlayer->CarAI.bOvertakeLast = pPlayer->CarAI.bOvertake;

            if (pPlayer->CarAI.bOvertake)
            {
                pPlayer->CarAI.dstRacingLine = ONE;
            }
            else
            {
                if (gTitleScreenVars.CupType == RACE_CLASS_BRONZE)
                {
#ifndef _PSX
                    pPlayer->CarAI.dstRacingLine = (REAL)frand(0.25);
#else
                    pPlayer->CarAI.dstRacingLine = rand() & 16383;
#endif
                }
                else
                {
                    pPlayer->CarAI.dstRacingLine = ZERO;
                }
            }
        }
    }

    if (pPlayer->CarAI.curRacingLine < pPlayer->CarAI.dstRacingLine)
    {
        pPlayer->CarAI.curRacingLine += MulScalar(TimeStep, Real(0.1));
        if (pPlayer->CarAI.curRacingLine > pPlayer->CarAI.dstRacingLine)
            pPlayer->CarAI.curRacingLine = pPlayer->CarAI.dstRacingLine;
    }
    else if (pPlayer->CarAI.curRacingLine > pPlayer->CarAI.dstRacingLine)
    {
        pPlayer->CarAI.curRacingLine -= MulScalar(TimeStep, Real(0.1));
        if (pPlayer->CarAI.curRacingLine < pPlayer->CarAI.dstRacingLine)
            pPlayer->CarAI.curRacingLine = pPlayer->CarAI.dstRacingLine;
    }


// CatchUp code
    CAI_CatchUp(pPlayer);

// Update AI parameters
    CAI_UpdateAiParameters(pPlayer);

// Find priority path
    CAI_ChooseRoute(pPlayer);

// Get angle to racing line
    pPlayer->CarAI.cosAngleToRacingLine = CAI_GetAngleToRacingLine(pPlayer, &pPlayer->CarAI.forwardVecDest,
                                                                            &pPlayer->CarAI.rightVec,
                                                                            &pPlayer->CarAI.dstRaceLineSide);
    pPlayer->CarAI.absCosAngleToRacingLine = abs(pPlayer->CarAI.cosAngleToRacingLine);


// Exit if data is bad
//  if (!pPlayer->CarAI.pCurNode)
//      return;

// Setup new state ?
    if (pPlayer->CarAI.AIState != pPlayer->CarAI.lastAIState)
    {
        pPlayer->CarAI.AIStateCount = gCAI_StateTimer[pPlayer->CarAI.AIState];
        pPlayer->CarAI.lastAIState = pPlayer->CarAI.AIState;
    }

    if (pPlayer->CarAI.AIStateCount > Real(0.0))
        pPlayer->CarAI.AIStateCount -= TimeStep;

// State process
    switch (pPlayer->CarAI.AIState)
    {
        // Starting line
        case CAI_S_STARTING_GRID:
            pPlayer->controls.dx = 0;
            pPlayer->controls.dy = 0;

            if (1)
            {
                pPlayer->CarAI.AIState = CAI_S_GREEN_LIGHT;
            }
            break;

        // Green light on!!  GO!!!!!
        case CAI_S_GREEN_LIGHT:
            if (1)
            {
                pPlayer->CarAI.AIState = CAI_S_RACE;
            }
            break;

        // Race
        case CAI_S_RACE:
            CAI_State_Race(pPlayer);
            break;

        // Brake for corner
        case CAI_S_BRAKING_ZONE:
            CAI_State_BrakeIntoCorner(pPlayer);
            break;

        // Reverse Correction
        case CAI_S_REVERSE_CORRECT:
        case CAI_S_FORWARD_CORRECT:
            CAI_State_ReverseCorrect(pPlayer);
            break;

        // Forward left correct
        case CAI_S_CORRECT_FORWARDLEFT:
            CAI_State_CorrectForwardLeft(pPlayer);
            break;

        // Forward right correct
        case CAI_S_CORRECT_FORWARDRIGHT:
            CAI_State_CorrectForwardRight(pPlayer);
            break;

        // Reverse left correct
        case CAI_S_CORRECT_REVERSELEFT:
            CAI_State_CorrectReverseLeft(pPlayer);
            break;

        // Reverse right correct
        case CAI_S_CORRECT_REVERSERIGHT:
            CAI_State_CorrectReverseRight(pPlayer);
            break;

        // Forward Correction
//      case CAI_S_FORWARD_CORRECT:
//          CAI_State_ForwardCorrect(pPlayer);
//          break;

        // Yeeee-haaaaa!
        case CAI_S_IN_THE_AIR:
            CAI_State_InTheAir(pPlayer);
            break;

        // Just landed
        case CAI_S_LANDED:
            CAI_State_Landed(pPlayer);
            break;
    }


// Process weapons
    CAI_ProcessWeapons(pPlayer);

// Under/oversteer & Avoid wall
    if (pPlayer->CarAI.AIState == CAI_S_RACE)
    {
    // Correct Under & oversteer
        CAI_ProcessUnderOverSteer(pPlayer);

    // Near-to-wall avoidance
        ProcessNearWallAvoidance(pPlayer);

    // Slow down ?
        if (pNode = pPlayer->CarAI.pCurNode)
        {
            switch (pNode->Priority)
            {
                case AIN_TYPE_SLOWDOWN_15:
                    if (pPlayer->CarAI.speedCur > MPH2OGU_SPEED*15)
                        pPlayer->controls.dy = CTRL_RANGE_MAX;
                    break;

                case AIN_TYPE_SLOWDOWN_20:
                    if (pPlayer->CarAI.speedCur > MPH2OGU_SPEED*20)
                        pPlayer->controls.dy = CTRL_RANGE_MAX;
                    break;

                case AIN_TYPE_SLOWDOWN_25:
                    if (pPlayer->CarAI.speedCur > MPH2OGU_SPEED*25)
                        pPlayer->controls.dy = CTRL_RANGE_MAX;
                    break;

                case AIN_TYPE_SLOWDOWN_30:
                    if (pPlayer->CarAI.speedCur > MPH2OGU_SPEED*30)
                        pPlayer->controls.dy = CTRL_RANGE_MAX;
                    break;

                case AIN_TYPE_TITLESCR_SLOWDOWN:
                    if (pPlayer->CarAI.speedCur > MulScalar(pPlayer->CarAI.speedMax, Real(0.25)))
                        pPlayer->controls.dy = CTRL_RANGE_MAX;
                    break;

                case AIN_TYPE_OFFTHROTTLEPETROL:
                    if (CarInfo[pPlayer->car.CarType].Class != CAR_CLASS_GLOW)
                        break;
                case AIN_TYPE_OFFTHROTTLE:
    //              if (pPlayer->CarAI.speedCur > MulScalar(pPlayer->CarAI.speedMax, Real(0.25)))   //0.75
                    if (pPlayer->CarAI.speedCur > MPH2OGU_SPEED*20) //0.75
                        pPlayer->controls.dy = 0;
                    else
                        pPlayer->controls.dy = (int)(-CTRL_RANGE_MAX * 0.5);    //0.75
                    break;
            }
        }
    }


// Quantize controls
    CAI_QuantizeControls(&pPlayer->controls);

// Copy movement vars
    pPlayer->CarAI.dx = pPlayer->controls.dx;
    pPlayer->CarAI.dy = pPlayer->controls.dy;
    pPlayer->CarAI.digital = pPlayer->controls.digital;
}


/********************************************************************************
* CAI_InitCatchUp()
********************************************************************************/
void CAI_InitCatchUp(DIFFICULTY_LEVEL iLevel)
{
    Assert((iLevel >= 0) && (iLevel < DIFFICULTY_NUM));

    gpCatchUpVars = &gCatchUp[iLevel];
}

/********************************************************************************
* CAI_CatchUp()
********************************************************************************/
void CAI_CatchUp(PLAYER *pPlayer)
{
    int     dist;
    int     iTemp, relPos;
    PLAYER  *pPlayerRel;

// Is there a 1st place car ?
//  pPlayerRel = gpPlayer1st;
    pPlayerRel = &Players[0];
    if (pPlayerRel && (pPlayerRel->type != PLAYER_GHOST) && (pPlayer != pPlayerRel))
    {
        // Get distance between player & car
        dist = (pPlayerRel->CarAI.raceDistance - pPlayer->CarAI.raceDistance);

        // Modify distance based on position
        relPos = pPlayerRel->CarAI.racePosition - pPlayer->CarAI.racePosition;
        if (relPos > 1)
        {
            dist += (int)gpCatchUpVars->distSpeedUpAdjust * (relPos - 1);
            if (dist > 0)
                dist = 0;
        }

        // Speed up trailing cars
        if (dist > gpCatchUpVars->lenSpeedUp)
        {
            dist = dist / (int)gpCatchUpVars->lenSpeedUp;
            iTemp = dist;
            if (iTemp > 0)
            {
                if (iTemp > (CTRL_CATCHUP_INC-1))
                    iTemp = (CTRL_CATCHUP_INC-1);
                pPlayer->controls.digital |= CTRL_CATCHUP_DIR | (iTemp << CTRL_CATCHUP_SHIFT);

                pPlayer->CarAI.catchUpMode = 1;
            }
        }
        // Slow down leading cars
        else if (dist < -gpCatchUpVars->lenSlowDown)
        {
            dist = -dist / (int)gpCatchUpVars->lenSlowDown;
            iTemp = dist;
            if (iTemp > 0)
            {
                if (iTemp > (CTRL_CATCHUP_DEC))
                    iTemp = (CTRL_CATCHUP_DEC);
                pPlayer->controls.digital |= iTemp << CTRL_CATCHUP_SHIFT;

                pPlayer->CarAI.catchUpMode = -1;
            }
        }
        else
        {
        // Was the car in catchup & has now just overtaken ?
            if ((pPlayer->CarAI.catchUpMode == 1) && gpCatchUpVars->bSpeedUpOvertake)
                pPlayer->controls.digital |= CTRL_CATCHUP_DIR | (0 << CTRL_CATCHUP_SHIFT);
        // Was the car in slowdown ?
            else if ((pPlayer->CarAI.catchUpMode == -1) && gpCatchUpVars->bSlowDownOvertake)
                pPlayer->controls.digital |= 1 << CTRL_CATCHUP_SHIFT;
            else
                pPlayer->CarAI.catchUpMode = 0;
        }
    }
}


/********************************************************************************
* CAI_IsCarInTrouble(PLAYER *pPlayer)
*
* Check to see if the car is in trouble.  IE. Facing the wrong way, hitting a wall etc...
********************************************************************************/
bool CAI_IsCarInTrouble(PLAYER *pPlayer)
{
// Is the car colliding with the world or any objects ?
#if 1
    if ((pPlayer->car.NWheelColls - pPlayer->car.NWheelFloorContacts) || pPlayer->car.Body->NBodyColls)
    {
        if (pPlayer->CarAI.speedCur < MPH2OGU_SPEED*3)
        {
            pPlayer->CarAI.AIState = CAI_S_REVERSE_CORRECT;
            return TRUE;
        }
    }

// Pointing in the correct direction ?
    if (pPlayer->CarAI.raceLineFaceDir < 0)
    {
        pPlayer->CarAI.AIState = CAI_S_REVERSE_CORRECT;
        return TRUE;
    }
#endif

    return FALSE;
}


/********************************************************************************
* CAI_ProcessUnderOverSteer()
********************************************************************************/
void CAI_ProcessUnderOverSteer(PLAYER *pPlayer)
{
    VEC     pos;
    REAL    steer, steer2;
    REAL    steerUnclamped;
    int     sign[2];
    int     tempI;
//  REAL    speed;

// Only do this if the car is in AI RACE State
    if (pPlayer->CarAI.AIState != CAI_S_RACE)
        return;

// Check if car is on the ground
    if (pPlayer->car.NWheelFloorContacts < 3)
    {
        return;
    }


// Calc. front left over/understeer
    VecMinusVec (&pPlayer->car.Wheel[FL].CentrePos, &pPlayer->car.Body->Centre.Pos, &pos);
    BodyPointVel(pPlayer->car.Body, &pos, &pPlayer->car.Body->FrontPointVel);
    pPlayer->car.Body->FrontPointVelDotRight = VecDotVec(&pPlayer->car.Wheel[FL].Axes.mv[R], &pPlayer->car.Body->FrontPointVel);

// Calc. rear left over/understeer
    VecMinusVec (&pPlayer->car.Wheel[BL].CentrePos, &pPlayer->car.Body->Centre.Pos, &pos);
    BodyPointVel(pPlayer->car.Body, &pos, &pPlayer->car.Body->RearPointVel);
    pPlayer->car.Body->RearPointVelDotRight = VecDotVec(&pPlayer->car.Wheel[BL].Axes.mv[R], &pPlayer->car.Body->RearPointVel);


// AAAA
    sign[0] = Sign(pPlayer->car.Body->FrontPointVelDotRight);
    sign[1] = Sign(pPlayer->car.Body->RearPointVelDotRight);
    if (sign[0] == sign[1])
    {
    // Check for oversteer (rear velocity is significantly larger than the front)
        steer = abs(pPlayer->car.Body->RearPointVelDotRight) - abs(pPlayer->car.Body->FrontPointVelDotRight);
        steer -= pPlayer->CarAI.oversteerThreshold;
        if (steer > ZERO)
        {
            steerUnclamped = steer = DivScalar(steer, pPlayer->CarAI.oversteerRange);
            if (steer > pPlayer->CarAI.oversteerMax)
                steer = pPlayer->CarAI.oversteerMax;
        }
        else
        {
        // Check for understeer
            steer = abs(pPlayer->car.Body->FrontPointVelDotRight + pPlayer->car.Body->RearPointVelDotRight);
            steer -= pPlayer->CarAI.understeerThreshold;
            if (steer > ZERO)
            {
                steerUnclamped = steer = DivScalar(steer, pPlayer->CarAI.understeerRange);
                if (steer > pPlayer->CarAI.understeerMax)
                    steer = pPlayer->CarAI.understeerMax;
                steer = -steer;
            }
            else
            {
                steer = 0;
            }
        }
    }
    else
    {
    // Is the rear of the car snapping out ?
        steer = abs(pPlayer->car.Body->FrontPointVelDotRight - pPlayer->car.Body->RearPointVelDotRight);
        steer -= pPlayer->CarAI.oversteerThreshold;
        if (steer > ZERO)
        {
            steerUnclamped = steer = DivScalar(steer, pPlayer->CarAI.oversteerRange);
            if (steer > pPlayer->CarAI.oversteerMax)
                steer = pPlayer->CarAI.oversteerMax;
        }
        else
            steer = 0;
    }

    pPlayer->CarAI.steerOverUnder = steer;


// Process understeer ?
    if (steer < ZERO)
    {
        steer = -steer;
        steer2 = ONE - steer;

    // Adjust steering
#ifndef _PSX
        tempI = Int(steer * CTRL_RANGE_MAX);
#else
        tempI = (steer * CTRL_RANGE_MAX) >> 16;
#endif
        if (pPlayer->controls.dx < 0)
        {
            tempI = pPlayer->controls.dx - tempI;
            if (tempI < -CTRL_RANGE_MAX)
                tempI = -CTRL_RANGE_MAX;
            pPlayer->controls.dx = (char)tempI;
        }
        else
        {
            tempI = pPlayer->controls.dx + tempI;
            if (tempI > CTRL_RANGE_MAX)
                tempI = CTRL_RANGE_MAX;
            pPlayer->controls.dx = (char)tempI;
        }

    // Adjust acceleration
        if ((abs(pPlayer->car.Body->FrontPointVelDotRight) > pPlayer->CarAI.understeerFront) &&
            (abs(pPlayer->car.Body->RearPointVelDotRight) > pPlayer->CarAI.understeerRear))
        {
//          pPlayer->controls.dy = CTRL_RANGE_MAX;
        }
        else
        {
            pPlayer->controls.dy = Int(MulScalar(Real(-CTRL_RANGE_MAX), steer2));
        }
    }
// Process oversteer ?
    else if (steer > ZERO)
    {
    // Adjust steering
        if (pPlayer->car.Body->FrontPointVelDotRight < ZERO)    // Spinning right
        {
#ifndef _PSX
            tempI = Int((pPlayer->car.SteerAngle - steer) * CTRL_RANGE_MAX);
#else
            tempI = ((pPlayer->car.SteerAngle - steer) * CTRL_RANGE_MAX) >> 16;
#endif
            if (tempI < -CTRL_RANGE_MAX)
                tempI = -CTRL_RANGE_MAX;
//          if (tempI < 0)
//              tempI >>= 1;
            pPlayer->controls.dx = (char)tempI;
        }
        else                                                    // Spinning left
        {
#ifndef _PSX
            tempI = Int((pPlayer->car.SteerAngle + steer) * CTRL_RANGE_MAX);
#else
            tempI = ((pPlayer->car.SteerAngle + steer) * CTRL_RANGE_MAX) >> 16;
#endif
            if (tempI > CTRL_RANGE_MAX)
                tempI = CTRL_RANGE_MAX;
//          if (tempI > 0)
//              tempI >>= 1;
            pPlayer->controls.dx = (char)tempI;
        }


    // Adjust acceleration
        if (abs(pPlayer->car.Body->FrontPointVelDotRight) > abs(pPlayer->car.Body->RearPointVelDotRight))
            steer = abs(pPlayer->car.Body->FrontPointVelDotRight);
        else
            steer = abs(pPlayer->car.Body->RearPointVelDotRight);

        steer -= pPlayer->CarAI.oversteerAccelThreshold;
        if (steer > 0)
        {
            steer = DivScalar(steer, pPlayer->CarAI.oversteerAccelRange);
            if (steer > Real(1.5))
                steer = Real(1.5);

            pPlayer->controls.dy = (signed char)(MulScalar((ONE - steer), -CTRL_RANGE_MAX));
        }
    }
}


/********************************************************************************
* CAI_ProcessNearWallAvoidance()
********************************************************************************/
void ProcessNearWallAvoidance(PLAYER* pPlayer)
{
    AINODE  *pNode;
    int     c;
//  int     temp;
    int     x;
    REAL    mod, range;
#ifdef _PSX
    VEC     tVec;
#endif

// Check for valid node
    if ((!pPlayer->CarAI.pCurNode) ||
        (pPlayer->car.NWheelFloorContacts <= 2))
        return;


// Calc. distance to node edges
    pPlayer->CarAI.lastCurNodeEdgeDist[0] = pPlayer->CarAI.curNodeEdgeDist[0];
    pPlayer->CarAI.lastCurNodeEdgeDist[1] = pPlayer->CarAI.curNodeEdgeDist[1];

#ifndef _PSX
    pPlayer->CarAI.curNodeEdgeDist[0] = VecDotPlane(&pPlayer->car.Body->Centre.Pos, &pPlayer->CarAI.pCurNode->link.planeEdge[0]);
    pPlayer->CarAI.curNodeEdgeDist[1] = VecDotPlane(&pPlayer->car.Body->Centre.Pos, &pPlayer->CarAI.pCurNode->link.planeEdge[1]);
#else
    VecMinusVec(&pPlayer->car.Body->Centre.Pos, &pPlayer->CarAI.pCurNode->Node[0].Pos, &tVec);
    pPlayer->CarAI.curNodeEdgeDist[0] = VecDotVec(&tVec, PlaneNormal(&pPlayer->CarAI.pCurNode->link.planeEdge[0]));
    VecMinusVec(&pPlayer->car.Body->Centre.Pos, &pPlayer->CarAI.pCurNode->Node[1].Pos, &tVec);
    pPlayer->CarAI.curNodeEdgeDist[1] = VecDotVec(&tVec, PlaneNormal(&pPlayer->CarAI.pCurNode->link.planeEdge[1]));
#endif


// Calc. where car will be in the future
//      VecPlusScalarVec(&pPlayer->car.Body->Centre.Pos, TO_TIME(Real(0.35)), &pPlayer->car.Body->Centre.Vel, &pPlayer->CarAI.futurePos);
    VecPlusScalarVec(&pPlayer->car.Body->Centre.Pos, TO_TIME(gGazzaEEE), &pPlayer->car.Body->Centre.Vel, &pPlayer->CarAI.futurePos);

// What node will the car be in ?
    pNode = pPlayer->CarAI.pCurNode;
    c = 10;
    while (pNode && (c-- > 0))
    {
        if (AIN_IsPointInNodeBounds(&pPlayer->CarAI.futurePos, pNode, -1))
            break;

        pNode = pNode->Next[0];
    }

    pPlayer->CarAI.pFutureNode = pNode;
    if (pNode && (c > 0))
    {
    // Is car going to hit the wall any time soon ?
#ifndef _PSX
        pPlayer->CarAI.futureCurNodeEdgeDist[0] = VecDotPlane(&pPlayer->CarAI.futurePos, &pNode->link.planeEdge[0]);
        pPlayer->CarAI.futureCurNodeEdgeDist[1] = VecDotPlane(&pPlayer->CarAI.futurePos, &pNode->link.planeEdge[1]);
#else
        VecMinusVec(&pPlayer->CarAI.futurePos, &pNode->Node[0].Pos, &tVec);
        pPlayer->CarAI.futureCurNodeEdgeDist[0] = VecDotVec(&tVec, PlaneNormal(&pNode->link.planeEdge[0]));
        VecMinusVec(&pPlayer->CarAI.futurePos, &pNode->Node[1].Pos, &tVec);
        pPlayer->CarAI.futureCurNodeEdgeDist[1] = VecDotVec(&tVec, PlaneNormal(&pNode->link.planeEdge[1]));
#endif
        if (pPlayer->CarAI.futureCurNodeEdgeDist[0] < gGazzaWWW)//ZERO)
        {
////                range = DivScalar(delta, MulScalar(gGazzaWWW, TimeStep));
//          range = DivScalar(-pPlayer->CarAI.futureCurNodeEdgeDist[0], gGazzaWWW);
            range = DivScalar(-(pPlayer->CarAI.futureCurNodeEdgeDist[0] - gGazzaWWW), gGazzaWWW);
            mod = CTRL_RANGE_MAX * range;
            x = pPlayer->controls.dx + Int(mod);
            if (x > CTRL_RANGE_MAX)
            {
                pPlayer->controls.dx = CTRL_RANGE_MAX;

                // Adjust accelerator / brake
                x -= CTRL_RANGE_MAX;
                x = pPlayer->controls.dy + Int(MulScalar(x, gGazzaQQQ));
                if (x > CTRL_RANGE_MAX)
                    pPlayer->controls.dy = CTRL_RANGE_MAX;
                else
                    pPlayer->controls.dy = (char)x;
            }
            else
                pPlayer->controls.dx = (char)x;

//              pPlayer->controls.dy = CTRL_RANGE_MAX;
        }
        else
        if (pPlayer->CarAI.futureCurNodeEdgeDist[1] < gGazzaWWW)//ZERO)
        {
////                range = DivScalar(delta, MulScalar(gGazzaWWW, TimeStep));
//          range = DivScalar(-pPlayer->CarAI.futureCurNodeEdgeDist[1], gGazzaWWW);
            range = DivScalar(-(pPlayer->CarAI.futureCurNodeEdgeDist[1] - gGazzaWWW), gGazzaWWW);
            mod = CTRL_RANGE_MAX * range;
            x = pPlayer->controls.dx - Int(mod);
            if (x < -CTRL_RANGE_MAX)
            {
                pPlayer->controls.dx = -CTRL_RANGE_MAX;

                // Adjust accelerator / brake
                x = -(x + CTRL_RANGE_MAX);
                x = pPlayer->controls.dy + Int(MulScalar(x, gGazzaQQQ));
                if (x > CTRL_RANGE_MAX)
                    pPlayer->controls.dy = CTRL_RANGE_MAX;
                else
                    pPlayer->controls.dy = (char)x;
            }
            else
                pPlayer->controls.dx = (char)x;

//              pPlayer->controls.dy = CTRL_RANGE_MAX;
        }
    }
    else    // The car is going to exit the path !!!!!
    {
    // Brake in a straight line ?
        if (pPlayer->CarAI.speedCur > MPH2OGU_SPEED*30)
        {
            pPlayer->controls.dx = 0;
            pPlayer->controls.dy = CTRL_RANGE_MAX;
        }
        else
        if (pPlayer->CarAI.speedCur > MPH2OGU_SPEED*20)
        {
            pPlayer->controls.dy = CTRL_RANGE_MAX;
        }
    }
}


/********************************************************************************
* CAI_State_Race(PLAYER *pPlayer)
*
* General racing AI.
********************************************************************************/
void CAI_State_Race(PLAYER *pPlayer)
{
    int dist;


// Is the car in trouble ?
    if (CAI_IsCarInTrouble(pPlayer))
        return;

// Is the car in the air
    if ((pPlayer->car.NWheelFloorContacts == 0) && (pPlayer->car.NWheelsInContact < 3))
    {
        pPlayer->CarAI.AIState = CAI_S_IN_THE_AIR;
        return;
    }


// Setup steering
    CAI_ProcessCarSteering(pPlayer);


// Is the car in the brake zone ?
    if (0)  //(CAI_CalcBrakingParameters(pPlayer, &brakeDist, &speed))
    {
        pPlayer->CarAI.AIState = CAI_S_BRAKING_ZONE;
    }
    else
    {
    // Setup acceleration
//      if (pPlayer->controls.dx < 0)   temp = -pPlayer->controls.dx;
//      else                            temp = pPlayer->controls.dx;
//      pPlayer->controls.dy = -CTRL_RANGE_MAX + (temp - (temp >> 1));
        pPlayer->controls.dy = -CTRL_RANGE_MAX;

#if 0
        if (pPlayer->CarAI.pCurNode)
        {
            int flag;

            flag = 0;
            if (pPlayer->CarAI.pCurNode->Priority == AIN_TYPE_OFFTHROTTLE)
                flag = 1;
            else
            if ((pPlayer->CarAI.pCurNode->Priority == AIN_TYPE_OFFTHROTTLEPETROL) &&
                (CarInfo[pPlayer->car.CarType].Class == CAR_CLASS_GLOW))
                flag = 1;

            if (flag)
            {
//              if (pPlayer->CarAI.speedCur > MulScalar(pPlayer->CarAI.speedMax, Real(0.25)))   //0.75
                if (pPlayer->CarAI.speedCur > MPH2OGU_SPEED*20) //0.75
                    pPlayer->controls.dy = 0;
                else
                    pPlayer->controls.dy = (int)(-CTRL_RANGE_MAX * 0.5);    //0.75
            }
        }
#endif

    // Overtake opponent ?
        if (pPlayer->CarAI.pNearestCarInFront)
        {
            dist = pPlayer->CarAI.pNearestCarInFront->CarAI.raceDistance - pPlayer->CarAI.raceDistance;
            if (dist < (500))
            {
                pPlayer->CarAI.bOvertake = !pPlayer->CarAI.pNearestCarInFront->CarAI.bOvertake;
//              pPlayer->CarAI.bOvertake = 1;

                pPlayer->CarAI.timeOvertake = OVERTAKE_MIN_TIME;
            }
            else
            {
                if (pPlayer->CarAI.timeOvertake > ZERO)
                    pPlayer->CarAI.timeOvertake -= TimeStep;
                else
                    pPlayer->CarAI.bOvertake = 0;
            }
        }
    // Block opponent from overtaking me ?
/*
        else if (pPlayer->CarAI.pNearestCarBehind)
        {
            if (pPlayer->CarAI.distNearestCarBehind < 400)
            {
                pPlayer->CarAI.bOvertake = pPlayer->CarAI.pNearestCarBehind->CarAI.bOvertake;
                pPlayer->CarAI.timeOvertake = OVERTAKE_MIN_TIME;
            }
        }
*/
        else
        {
            if (pPlayer->CarAI.timeOvertake > ZERO)
                pPlayer->CarAI.timeOvertake -= TimeStep;
            else
                pPlayer->CarAI.bOvertake = 0;
        }

#ifdef _PC
        // DEBUG STUFF
        if (gGazzasOvertake)
            pPlayer->CarAI.bOvertake = TRUE;
#endif
    }
}

/********************************************************************************
* CAI_State_InTheAir(PLAYER *pPlayer)
*
* Car is in the air.
********************************************************************************/
void CAI_State_InTheAir(PLAYER *pPlayer)
{
// Set controlls
    pPlayer->controls.dx = 0;
    pPlayer->controls.dy = -CTRL_RANGE_MAX;

// Is the car on the ground (ie. it has landed) ?
    if (pPlayer->car.NWheelFloorContacts)
//  if (pPlayer->car.NWheelFloorContacts || pPlayer->car.NWheelFloorContacts)
    {
//      pPlayer->CarAI.AIState = CAI_S_LANDED;
        pPlayer->CarAI.AIState = CAI_S_RACE;
        return;
    }
}

/********************************************************************************
* CAI_State_Landed(PLAYER *pPlayer)
*
* Car has landed
********************************************************************************/
void CAI_State_Landed(PLAYER *pPlayer)
{
// Is the car in the air ?
//  if (pPlayer->car.NWheelFloorContacts == 0)
//  if (pPlayer->car.NWheelFloorContacts < 3)
    if (pPlayer->car.NWheelsInContact < 3)
    {
        pPlayer->CarAI.AIState = CAI_S_IN_THE_AIR;
        return;
    }
//      pPlayer->CarAI.AIStateCount = gCAI_StateTimer[pPlayer->CarAI.AIState];

// Process race state AI but cancel steering.
    CAI_State_Race(pPlayer);
//  pPlayer->controls.dx = 0;
    pPlayer->controls.dx = Int(MulScalar(pPlayer->controls.dx, Real(0.25)));

// Revert to race state ?
    if (pPlayer->CarAI.AIStateCount <= Real(0.0))
        pPlayer->CarAI.AIState = CAI_S_RACE;
}


/********************************************************************************
* CAI_State_BrakeIntoCorner(PLAYER *pPlayer)
*
* Brake into the corner
********************************************************************************/
void CAI_State_BrakeIntoCorner(PLAYER *pPlayer)
{
    REAL    brakeDist, speed, speedDiff;
    int     temp;

// Is the car in trouble ?
    if (CAI_IsCarInTrouble(pPlayer))
        return;

// Pointing in the correct direction ?
    if (pPlayer->CarAI.raceLineFaceDir < 0)
    {
        pPlayer->CarAI.AIState = CAI_S_REVERSE_CORRECT;
        return;
    }


// Setup steering
    CAI_ProcessCarSteering(pPlayer);


// Are we in the brake zone ?
    if (CAI_CalcBrakingParameters(pPlayer, &brakeDist, &speed))
    {
        // Do we need to brake ?
        speedDiff = pPlayer->CarAI.speedCur - MulScalar(speed, pPlayer->CarAI.speedMax);
        if (speedDiff > Real(0.0))
        {
#ifndef _PSX
            speedDiff *= 8;
#else
            speedDiff <<= 3;
#endif
            temp = Int(DivScalar(MulScalar(speedDiff, Real(CTRL_RANGE_MAX)), pPlayer->CarAI.speedMax));
            if (temp > CTRL_RANGE_MAX)
                temp = CTRL_RANGE_MAX;
            pPlayer->controls.dy = (char)temp;
            pPlayer->controls.dx = 0;       // Brake in a straight line
        }
        else
        {
            pPlayer->controls.dy = -CTRL_RANGE_MAX;
        }
    }
    else
    {
        pPlayer->CarAI.AIState = CAI_S_RACE;
    }
}

/********************************************************************************
* CAI_State_ReverseCorrect(PLAYER *pPlayer)
*
* Correct car when spun out etc...
********************************************************************************/
void CAI_State_ReverseCorrect(PLAYER *pPlayer)
{
    AINODE  *pNode;

// Scraping left side along wall ?
    if (IsWheelInSideContact(&pPlayer->car.Wheel[FL]) && IsWheelInSideContact(&pPlayer->car.Wheel[BL]))
    {
        if (pPlayer->CarAI.raceLineFaceDir >= 0)
            pPlayer->controls.dx = CTRL_RANGE_MAX >> 1;
        else
            pPlayer->controls.dx = CTRL_RANGE_MAX;

        pPlayer->controls.dy = -CTRL_RANGE_MAX;
        return;
    }

    if (IsWheelInSideContact(&pPlayer->car.Wheel[FR]) && IsWheelInSideContact(&pPlayer->car.Wheel[BR]))
    {
        if (pPlayer->CarAI.raceLineFaceDir >= ZERO)
            pPlayer->controls.dx = -CTRL_RANGE_MAX >> 1;
        else
            pPlayer->controls.dx = -CTRL_RANGE_MAX;

        pPlayer->controls.dy = -CTRL_RANGE_MAX;
        return;
    }

// Pointing in the correct direction ?
    if ((pPlayer->CarAI.cosAngleToRacingLine > Real(0.707)) &&
        (pPlayer->CarAI.raceLineFaceDir > ZERO))
    {
        pPlayer->CarAI.AIState = CAI_S_RACE;
        return;
    }

// Is the track narrow ?
    pNode = pPlayer->CarAI.pCurNode;
    if (pNode && (pNode->trackWidth < TO_LENGTH(Real(200*2))))
    {
//      return;
    }

// Which way should we go ?
    if (pPlayer->CarAI.curNodeEdgeDist[0] < pPlayer->CarAI.curNodeEdgeDist[1])
    {
        pPlayer->controls.dx = -CTRL_RANGE_MAX;
        pPlayer->controls.dy = -CTRL_RANGE_MAX;

        pPlayer->CarAI.AIState = CAI_S_CORRECT_FORWARDLEFT;
        return;
    }
    else
    {
        pPlayer->controls.dx = CTRL_RANGE_MAX;
        pPlayer->controls.dy = -CTRL_RANGE_MAX;

        pPlayer->CarAI.AIState = CAI_S_CORRECT_FORWARDRIGHT;
        return;
    }
/*
    if (pPlayer->CarAI.dstRaceLineSide < 0)
        pPlayer->controls.dx = -CTRL_RANGE_MAX;
    else
        pPlayer->controls.dx =  CTRL_RANGE_MAX;

    pPlayer->controls.dy = CTRL_RANGE_MAX;
*/
}


/********************************************************************************
* CAI_State_CorrectForwardLeft(PLAYER *pPlayer)
*
********************************************************************************/
#define BETTER_CORRECTION

void CAI_State_CorrectForwardLeft(PLAYER *pPlayer)
{
// Pointing in the correct direction ?
//  if ((pPlayer->CarAI.cosAngleToRacingLine > CAI_CORRECT_ANGLE) &&
//      (pPlayer->CarAI.raceLineFaceDir >= 0))
    if (pPlayer->CarAI.cosAngleToRacingLine > CAI_CORRECT_ANGLE)
    {
        pPlayer->CarAI.AIState = CAI_S_RACE;
        return;
    }

// Are the car's front wheel hitting something ?
    if (IsWheelInWallContact(&pPlayer->car.Wheel[FL]) || IsWheelInWallContact(&pPlayer->car.Wheel[FR]) ||
        ((pPlayer->CarAI.AIStateCount < ZERO) && pPlayer->CarAI.speedCur < MPH2OGU_SPEED*3))
    {
        pPlayer->CarAI.AIState = CAI_S_CORRECT_REVERSERIGHT;
        return;
    }

#ifndef BETTER_CORRECTION
// Is the car scraping along the wall ?
    if (IsWheelInSideContact(&pPlayer->car.Wheel[FL]) || IsWheelInSideContact(&pPlayer->car.Wheel[BL]))
    {
        pPlayer->CarAI.AIState = CAI_S_CORRECT_FORWARDRIGHT;
        return;
    }
#endif

// Steer
    pPlayer->controls.dx = -CTRL_RANGE_MAX;
    if (pPlayer->CarAI.speedCur < MPH2OGU_SPEED*20)
        pPlayer->controls.dy = -CTRL_RANGE_MAX>>1;
    else
        pPlayer->controls.dy = -CTRL_RANGE_MAX>>4;
}

/********************************************************************************
* CAI_State_CorrectForwardRight(PLAYER *pPlayer)
*
********************************************************************************/
void CAI_State_CorrectForwardRight(PLAYER *pPlayer)
{
// Pointing in the correct direction ?
//  if ((pPlayer->CarAI.cosAngleToRacingLine > CAI_CORRECT_ANGLE) &&
//      (pPlayer->CarAI.raceLineFaceDir >= 0))
    if (pPlayer->CarAI.cosAngleToRacingLine > CAI_CORRECT_ANGLE)
    {
        pPlayer->CarAI.AIState = CAI_S_RACE;
        return;
    }

// Are the car's front wheel hitting something ?
    if (IsWheelInWallContact(&pPlayer->car.Wheel[FL]) || IsWheelInWallContact(&pPlayer->car.Wheel[FR]) ||
        ((pPlayer->CarAI.AIStateCount < ZERO) && pPlayer->CarAI.speedCur < MPH2OGU_SPEED*3))
    {
        pPlayer->CarAI.AIState = CAI_S_CORRECT_REVERSELEFT;
        return;
    }

#ifndef BETTER_CORRECTION
// Is the car scraping along the wall ?
    if (IsWheelInSideContact(&pPlayer->car.Wheel[FR]) || IsWheelInSideContact(&pPlayer->car.Wheel[BR]))
    {
        pPlayer->CarAI.AIState = CAI_S_CORRECT_FORWARDLEFT;
        return;
    }
#endif


// Steer
    pPlayer->controls.dx = CTRL_RANGE_MAX;
    if (pPlayer->CarAI.speedCur < MPH2OGU_SPEED*20)
        pPlayer->controls.dy = -CTRL_RANGE_MAX>>1;
    else
        pPlayer->controls.dy = -CTRL_RANGE_MAX>>4;
}


/********************************************************************************
* CAI_State_CorrectReverseLeft(PLAYER *pPlayer)
*
********************************************************************************/
void CAI_State_CorrectReverseLeft(PLAYER *pPlayer)
{
// Pointing in the correct direction ?
//  if ((pPlayer->CarAI.cosAngleToRacingLine > CAI_CORRECT_ANGLE) &&
//      (pPlayer->CarAI.raceLineFaceDir >= 0))
    if (pPlayer->CarAI.cosAngleToRacingLine > CAI_CORRECT_ANGLE)
    {
        pPlayer->CarAI.AIState = CAI_S_RACE;
        return;
    }

// Are the car's front wheel hitting something ?
    if (IsWheelInWallContact(&pPlayer->car.Wheel[BL]) || IsWheelInWallContact(&pPlayer->car.Wheel[BR]) ||
        ((pPlayer->CarAI.AIStateCount < ZERO) && pPlayer->CarAI.speedCur < MPH2OGU_SPEED*3))
    {
        pPlayer->CarAI.AIState = CAI_S_CORRECT_FORWARDRIGHT;
        return;
    }

#ifndef BETTER_CORRECTION
// Is the car scraping along the wall ?
    if (IsWheelInSideContact(&pPlayer->car.Wheel[FR]) || IsWheelInSideContact(&pPlayer->car.Wheel[BR]))
    {
        pPlayer->CarAI.AIState = CAI_S_CORRECT_REVERSERIGHT;
        return;
    }
#endif

// Steer
    pPlayer->controls.dx = -CTRL_RANGE_MAX;
    if (pPlayer->CarAI.speedCur < MPH2OGU_SPEED*20)
        pPlayer->controls.dy = CTRL_RANGE_MAX>>1;
    else
        pPlayer->controls.dy = CTRL_RANGE_MAX>>4;
}

/********************************************************************************
* CAI_State_CorrectReverseRight(PLAYER *pPlayer)
*
********************************************************************************/
void CAI_State_CorrectReverseRight(PLAYER *pPlayer)
{
// Pointing in the correct direction ?
//  if ((pPlayer->CarAI.cosAngleToRacingLine > CAI_CORRECT_ANGLE) &&
//      (pPlayer->CarAI.raceLineFaceDir >= 0))
    if (pPlayer->CarAI.cosAngleToRacingLine > CAI_CORRECT_ANGLE)
    {
        pPlayer->CarAI.AIState = CAI_S_RACE;
        return;
    }

// Are the car's front wheel hitting something ?
    if (IsWheelInWallContact(&pPlayer->car.Wheel[BL]) || IsWheelInWallContact(&pPlayer->car.Wheel[BR]) ||
        ((pPlayer->CarAI.AIStateCount < ZERO) && pPlayer->CarAI.speedCur < MPH2OGU_SPEED*3))
    {
        pPlayer->CarAI.AIState = CAI_S_CORRECT_FORWARDLEFT;
        return;
    }

#ifndef BETTER_CORRECTION
// Is the car scraping along the wall ?
    if (IsWheelInSideContact(&pPlayer->car.Wheel[FL]) || IsWheelInSideContact(&pPlayer->car.Wheel[BL]))
    {
        pPlayer->CarAI.AIState = CAI_S_CORRECT_REVERSELEFT;
        return;
    }
#endif

// Steer
    pPlayer->controls.dx = CTRL_RANGE_MAX;
    if (pPlayer->CarAI.speedCur < MPH2OGU_SPEED*20)
        pPlayer->controls.dy = CTRL_RANGE_MAX>>1;
    else
        pPlayer->controls.dy = CTRL_RANGE_MAX>>4;
}


/********************************************************************************
* CAI_State_ForwardCorrect(PLAYER *pPlayer)
*
* Correct car when spun out etc...
********************************************************************************/
void CAI_State_ForwardCorrect(PLAYER *pPlayer)
{
//  REAL    angle;
//  int     dir;
//  REAL    side;
    int     collision;

    if ((pPlayer->car.NWheelColls - pPlayer->car.NWheelFloorContacts) || pPlayer->car.Body->NBodyColls)
        collision = TRUE;
    else
    {
        collision = FALSE;
//      pPlayer->CarAI.AIStateCount = Real(0.0);
    }

// Get angle to racing line
//  angle = pPlayer->CarAI.angleToRacingLine;
//  dir   = pPlayer->CarAI.raceLineFaceDir;
//  side  = pPlayer->CarAI.raceLineSide;

// Is the car colliding with the world or any objects ?
    if (pPlayer->CarAI.AIStateCount <= Real(0.0))
    {
    // Colliding something ?
        if (collision)
        {
            pPlayer->CarAI.AIState = CAI_S_REVERSE_CORRECT;
            return;
        }
    // Ready to race ?
        if (pPlayer->CarAI.raceLineFaceDir >= 0)
        {
            pPlayer->CarAI.AIState = CAI_S_RACE;
            return;
        }
    }

// Turn & reverse
    if (pPlayer->CarAI.cosAngleToRacingLine < 0)
        pPlayer->controls.dx =  CTRL_RANGE_MAX;
    else
        pPlayer->controls.dx = -CTRL_RANGE_MAX;

    pPlayer->controls.dy = -CTRL_RANGE_MAX;
}


/********************************************************************************
* CAI_ProcessWeapons(PLAYER *pPlayer)
*
* Calculate braking distance and speed
********************************************************************************/
void CAI_ProcessWeapons(PLAYER *pPlayer)
{
    int frontFlag, behindFlag, targetFlag;
    int playerType;

// Exit if the car has no pickup
    if ((pPlayer->PickupType == PICKUP_TYPE_NONE) ||
        (pPlayer->car.PowerTimer > ZERO))
        return;

// Update pikcup duration & use if 'x' seconds have elapsed
    pPlayer->CarAI.pickupDuration += TimeStep;
//  if ((pPlayer->CarAI.pickupDuration > TO_TIME(Real(10)) &&
//      (pPlayer->car.LastHitTimer >= TO_TIME(Real(10))))

    if (Players[0].car.LastHitTimer <= TO_TIME(Real(10)))
        playerType = PLAYER_LOCAL;
    else
        playerType = PLAYER_NONE;

    // InFront Flag
    if (pPlayer->CarAI.pNearestCarInFront && (pPlayer->CarAI.pNearestCarInFront->type != playerType))
        frontFlag = TRUE;
    else
        frontFlag = FALSE;

    // Behind Flag
    if (pPlayer->CarAI.pNearestCarBehind && (pPlayer->CarAI.pNearestCarBehind->type != playerType))
        behindFlag = TRUE;
    else
        behindFlag = FALSE;

    // Target Flag
    if (pPlayer->PickupTarget && (pPlayer->PickupTarget->player->type != playerType))
        targetFlag = TRUE;
    else
        targetFlag = FALSE;


    switch (pPlayer->PickupType)
    {
        // None
        default:
        case PICKUP_TYPE_NONE:
            break;

        // Shockwave
        case PICKUP_TYPE_SHOCKWAVE:
            if (frontFlag && (pPlayer->CarAI.distNearestCarInFront < TO_LENGTH(Real(200*20))))
            {
                pPlayer->controls.digital |= CTRL_FIRE;     // Fire!
            }
            break;

        // Firework
        case PICKUP_TYPE_FIREWORK:
        case PICKUP_TYPE_FIREWORKPACK:

            if ((targetFlag) &&
                (pPlayer->CarAI.distNearestCarInFront < TO_LENGTH(Real(200*20))) &&
                (pPlayer->CarAI.distNearestCarInFront > TO_LENGTH(Real(300))))
            {
                pPlayer->controls.digital |= CTRL_FIRE;     // Fire!
            }

            break;

        // Putty bomb
        case PICKUP_TYPE_PUTTYBOMB:
            break;

        // Water bomb
        case PICKUP_TYPE_WATERBOMB:
            if (frontFlag && (pPlayer->CarAI.distNearestCarInFront < TO_LENGTH(Real(200*20))) &&
                (pPlayer->CarAI.distNearestCarInFront > TO_LENGTH(Real(200*1))))
            {
                pPlayer->controls.digital |= CTRL_FIRE;     // Fire!
            }
            break;

        // Electro pulse
        case PICKUP_TYPE_ELECTROPULSE:

            // Close enough for an electro pulse ?
            if (frontFlag && (pPlayer->CarAI.distNearestCarInFront < TO_LENGTH(Real(500))))
                pPlayer->controls.digital |= CTRL_FIRE;     // Fire!
            else
            if (behindFlag && (pPlayer->CarAI.distNearestCarBehind < TO_LENGTH(Real(600))))
                pPlayer->controls.digital |= CTRL_FIRE;     // Fire!

            break;

        // Chrome ball
        case PICKUP_TYPE_CHROMEBALL:
        // Oil slick
        case PICKUP_TYPE_OILSLICK:
        // Pickup Clone
        case PICKUP_TYPE_CLONE:
//          if (pPlayer->CarAI.pCurNode && (pPlayer->CarAI.pCurNode->trackWidth <= TO_LENGTH(Real(200*4))) &&
//              (pPlayer->CarAI.racePosition > Players[0].CarAI.racePosition))
            if (pPlayer->CarAI.pCurNode && (pPlayer->CarAI.pCurNode->trackWidth <= TO_LENGTH(Real(200*4))) &&
                (pPlayer->CarAI.speedCur > (MPH2OGU_SPEED*10)))
            {
                if (pPlayer->CarAI.pCurNode->trackWidth <= TO_LENGTH(Real(200*2)))
                    pPlayer->controls.digital |= CTRL_FIRE;     // Fire!
                else if ((pPlayer->CarAI.pCurNode->link.flags & (AIN_LF_WALL_LEFT | AIN_LF_WALL_RIGHT)))
                    pPlayer->controls.digital |= CTRL_FIRE;     // Fire!
            }
            else    // Let'em rip if player is right behind me !!!
            if (behindFlag && (pPlayer->CarAI.distNearestCarBehind < TO_LENGTH(Real(200*3))))
            {
                pPlayer->controls.digital |= CTRL_FIRE;     // Fire!
            }

            break;

        // Turbo
        case PICKUP_TYPE_TURBO:
//          if ((pPlayer->CarAI.pCurNode) &&
//              (pPlayer->CarAI.pCurNode->Priority == AIN_TYPE_TURBOLINE) &&
            if ((pPlayer->CarAI.cosAngleToRacingLine >= Real(0.9)) &&
                (pPlayer->CarAI.AIState == CAI_S_RACE))
                pPlayer->controls.digital |= CTRL_FIRE;     // Fire!
            break;

        // Global pulse
        case PICKUP_TYPE_GLOBAL:
            pPlayer->controls.digital |= CTRL_FIRE;     // Fire!
            break;
    }
}


/********************************************************************************
* CAI_UpdateNodeData(PLAYER *pPlayer)
*
* Updates useful node pointers (current node, last valid node, last split node etc...)
********************************************************************************/
void CAI_UpdateNodeData(PLAYER *pPlayer)
{
    VEC tVec;
    int i;

// Calc. velocity
    pPlayer->CarAI.speedCur = VecLen(&pPlayer->car.Body->Centre.Vel);


// Find zone player is in
#if 0
    if (!(pPlayer->CarAI.IsInZone = CAI_IsCarInZone(pPlayer)))
        return;
#else
    if (pPlayer->CarAI.CurZoneID >= 0)
    {
        pPlayer->CarAI.IsInZone = TRUE;

        if (!AIZ_IsCarInZoneID(pPlayer, pPlayer->CarAI.CurZoneID))
        {
            if (pPlayer->CarAI.CurZoneID >= AiZoneNumID-1)
                i = 0;
            else        
                i = pPlayer->CarAI.CurZoneID+1;

            if (!AIZ_IsCarInZoneID(pPlayer, i))
            {
                if (pPlayer->CarAI.CurZoneID <= 0)
                    i = AiZoneNumID-1;
                else        
                    i = pPlayer->CarAI.CurZoneID-1;

                if (!AIZ_IsCarInZoneID(pPlayer, i))
                {
                    if (!(pPlayer->CarAI.IsInZone = CAI_IsCarInZone(pPlayer)))
                        return;
                }
            }
        }
    }
    else
    {
        if (!(pPlayer->CarAI.IsInZone = CAI_IsCarInZone(pPlayer)))
            return;
    }
#endif


// Find node player is in
#ifdef _PC
    if (0)//((pPlayer->type == PLAYER_LOCAL) && (!gGazzasAICar))
#else
    if (0)  //((pPlayer->type == PLAYER_LOCAL) )
#endif
    {
//      pPlayer->CarAI.pCurNode = AIN_FindNodePlayerIsIn(pPlayer);
//      if (!pPlayer->CarAI.pCurNode)
//          pPlayer->CarAI.pCurNode = AIN_FindZoneNodePlayerIsIn(pPlayer);

        pPlayer->CarAI.pCurNode = AIN_FindZoneNodePlayerIsIn(pPlayer);
    }
    else
    {
        pPlayer->CarAI.pCurNode = AIN_GetForwardNodeInRange(pPlayer, TO_LENGTH(Real(200*10)), (REAL *)&pPlayer->CarAI.NodeDist);

        if (!pPlayer->CarAI.pCurNode)
            pPlayer->CarAI.pCurNode = AIN_FindZoneNodePlayerIsIn(pPlayer);
    }

// Setup node pointers
    if (pPlayer->CarAI.pCurNode)
    {
    // Set last valid node
        pPlayer->CarAI.pLastValidNode = pPlayer->CarAI.pCurNode;

    // Set last split node ?
        if (pPlayer->CarAI.pCurNode->Next[1])
            pPlayer->CarAI.pLastSplitNode = pPlayer->CarAI.pCurNode;

    // Calc. distance along node
        VecMinusVec(&pPlayer->car.Body->Centre.Pos, &pPlayer->CarAI.pCurNode->Centre, &tVec);
        pPlayer->CarAI.distAlongNode = VecDotVec(&pPlayer->CarAI.pCurNode->link.forwardVec, &tVec);

    // Calc. whie side of the racing line the car is on
        pPlayer->CarAI.raceLineSide = VecDotVec(&pPlayer->CarAI.pCurNode->link.rightVec, &tVec);

    // Face direction to racing line
        pPlayer->CarAI.raceLineFaceDir = Sign(VecDotVec(&pPlayer->CarAI.forwardVec, &pPlayer->CarAI.pCurNode->link.forwardVec));
    }
    else
    {
        pPlayer->CarAI.distAlongNode = 0;
        pPlayer->CarAI.raceLineSide = 0;
        pPlayer->CarAI.raceLineFaceDir = 0;
    }


// Setup track node & random choice
    if (pPlayer->CarAI.pLastDestNode != pPlayer->CarAI.pDestNode)
    {
        pPlayer->CarAI.pLastDestNode = pPlayer->CarAI.pDestNode;

        if (pPlayer->CarAI.pLastDestNode->Next[1] && pPlayer->CarAI.bRouteTaken)
        {
            pPlayer->CarAI.bRouteTaken = FALSE;

            for (i = 0; i < MAX_AINODE_LINKS; i++)
//              pPlayer->CarAI.routeRandom[i] = 32767;
                pPlayer->CarAI.routeRandom[i] = rand() & 32767;
        }
    }
}

/********************************************************************************
* CAI_UpdateAiParameters(PLAYER *pPlayer)
*
* Updates useful AI parameters
********************************************************************************/
int CAI_UpdateAiParameters(PLAYER *pPlayer)
{
// Setup various data
    CAI_UpdateNodeData(pPlayer);

// Calc. cars forward vector on XZ plane and zero Y component.
    pPlayer->CarAI.forwardVec.v[X] = pPlayer->car.Body->Centre.WMatrix.m[LX];
    pPlayer->CarAI.forwardVec.v[Y] = 0;
    pPlayer->CarAI.forwardVec.v[Z] = pPlayer->car.Body->Centre.WMatrix.m[LZ];
    NormalizeVector(&pPlayer->CarAI.forwardVec);

// Find closest players
    CAI_FindClosestPlayer(pPlayer, &pPlayer->CarAI.pNearestCarInFront, &pPlayer->CarAI.pNearestCarBehind,
                                   &pPlayer->CarAI.distNearestCarInFront, &pPlayer->CarAI.distNearestCarBehind);

    return TRUE;
}


/********************************************************************************
* CAI_()
********************************************************************************/




/********************************************************************************
* Render Ai Debug Info
********************************************************************************/
#ifdef __AI_DEBUG_INFO

#ifdef _PSX

void RenderAiDebugInfo(PLAYER *pPlayer, long *pOT, CAMERA *pCam)
{
    AINODE          *pNode, *pNodeN;
    AINODE_LINKINFO *pLinkInfo;
    SVECTOR         SV0;
    SVECTOR         v[4];
    MATRIX          TMatrix, UseMatrix; 
    SVECTOR         TempSV;
    VECTOR          TempV;
    VECTOR          UsePos;
    char            string[256];

// Make sure the pointers are setup
    if (!(pNode = pPlayer->CarAI.pCurNode))
        return;
    if (!(pNodeN = pNode->Next[0]))
        return;

    pLinkInfo = &pNode->link;

// Setup matrix
//           NEW_RenderWorld2( &OrderingTable[Toggle+(a<<1)][0], &Camera[a].WMatrixPSX, &Camera[a].WPosPSX );  

    // Transpose Camera Matrix and scale y for 512*256 mode
    TMatrix.m[0][0] = pCam->WMatrixPSX.m[0][0];
    TMatrix.m[0][1] = pCam->WMatrixPSX.m[1][0];
    TMatrix.m[0][2] = pCam->WMatrixPSX.m[2][0];
    TMatrix.m[1][0] = pCam->WMatrixPSX.m[0][1] >> 1;
    TMatrix.m[1][1] = pCam->WMatrixPSX.m[1][1] >> 1;
    TMatrix.m[1][2] = pCam->WMatrixPSX.m[2][1] >> 1;
    TMatrix.m[2][0] = pCam->WMatrixPSX.m[0][2];
    TMatrix.m[2][1] = pCam->WMatrixPSX.m[1][2];
    TMatrix.m[2][2] = pCam->WMatrixPSX.m[2][2];

    // Set Translation
    UsePos.vx = -pCam->WPosPSX.vx;
    UsePos.vy = -pCam->WPosPSX.vy;
    UsePos.vz = -pCam->WPosPSX.vz;

    ApplyMatrixLV(&TMatrix, &UsePos, &TempV);

//  UsePos.vx = TempV.vx;
//  UsePos.vy = TempV.vy;
//  UsePos.vz = TempV.vz;

//  TransMatrix(&UseMatrix, &TempV); 
//  SetTransMatrix(&UseMatrix);
        TransMatrix(&TMatrix, &TempV); 
        SetTransMatrix(&TMatrix);
    SetRotMatrix(&TMatrix);



//&pCam->WMatrixPSX, &pCam->WPosPSX

#if 1
// Current zone
    sprintf(string, "CZone: %d", pPlayer->CarAI.CurZone);
    DrawFont(32, 60, string, 128,128,128, 128,64,64, pOT, 1);

// Current zone ID
    sprintf(string, "CZoneID: %d", pPlayer->CarAI.CurZoneID);
    DrawFont(32, 70, string, 128,128,128, 128,64,64, pOT, 1);

// Speed
    sprintf(string, "CSpd: %d, NSpd: %d%%", pPlayer->CarAI.speedCur, (pLinkInfo->speed * 100) >> 16);
    DrawFont(32, 80, string, 128,128,128, 128,64,64, pOT, 1);

// Speed
    sprintf(string, "ND: %d, CD: %d, D: %d", pLinkInfo->dist, pPlayer->CarAI.distAlongNode, pLinkInfo->dist - pPlayer->CarAI.distAlongNode);
    DrawFont(32, 90, string, 128,128,128, 128,64,64, pOT, 1);

// AI vars
    sprintf(string, "Ang,S,D: %d,%d,%d", pPlayer->CarAI.cosAngleToRacingLine,
                                                    pPlayer->CarAI.raceLineSide,
                                                    pPlayer->CarAI.raceLineFaceDir);
    DrawFont(32, 100, string, 128,128,128, 128,64,64, pOT, 1);

// Steering
    sprintf(string, "Dx, Dy: %d,%d", pPlayer->CarAI.dx, pPlayer->CarAI.dy);
    DrawFont(32, 110, string, 128,128,128, 128,64,64, pOT, 1);
    


#endif


// Render nodes
//  CopyVec(&pNode->Node[0].Pos, &line[0]);
//  CopyVec(&pNode->Node[1].Pos, &line[1]);
#if 0
    SV0.vx = pNode->Node[0].Pos.v[0] >> 16;
    SV0.vy = pNode->Node[0].Pos.v[1] >> 16;
    SV0.vz = pNode->Node[0].Pos.v[2] >> 16;
    RenderFaceMeGT4(pOT, &SV0, 64, 64, (0<<8)+(194), (0<<8)+(255), (32<<8)+(194), (32<<8)+(255), 0X007F00,
                    &pCam->WMatrixPSX, &pCam->WPosPSX, 0, 0, 121);

    SV0.vx = pNode->Node[1].Pos.v[0] >> 16;
    SV0.vy = pNode->Node[1].Pos.v[1] >> 16;
    SV0.vz = pNode->Node[1].Pos.v[2] >> 16;
    RenderFaceMeGT4(pOT, &SV0, 64, 64, (0<<8)+(194), (0<<8)+(255), (32<<8)+(194), (32<<8)+(255), 0X00007F,
                    &pCam->WMatrixPSX, &pCam->WPosPSX, 0, 0, 121);
#endif

    v[0].vx = PSX_LENGTH(pNode->Node[0].Pos.v[0]);
    v[0].vy = PSX_LENGTH(pNode->Node[0].Pos.v[1]);
    v[0].vz = PSX_LENGTH(pNode->Node[0].Pos.v[2]);
    v[1].vx = PSX_LENGTH(pNode->Node[1].Pos.v[0]);
    v[1].vy = PSX_LENGTH(pNode->Node[1].Pos.v[1]);
    v[1].vz = PSX_LENGTH(pNode->Node[1].Pos.v[2]);
    v[2].vx = PSX_LENGTH(pNodeN->Node[0].Pos.v[0]);
    v[2].vy = PSX_LENGTH(pNodeN->Node[0].Pos.v[1]);
    v[2].vz = PSX_LENGTH(pNodeN->Node[0].Pos.v[2]);
    v[3].vx = PSX_LENGTH(pNodeN->Node[1].Pos.v[0]);
    v[3].vy = PSX_LENGTH(pNodeN->Node[1].Pos.v[1]);
    v[3].vz = PSX_LENGTH(pNodeN->Node[1].Pos.v[2]);
    RenderLineF2(pOT, &v[0], &v[1], 0xFF0000);
    RenderLineF2(pOT, &v[1], &v[3], 0xFF0000);
    RenderLineF2(pOT, &v[3], &v[2], 0xFF0000);
    RenderLineF2(pOT, &v[2], &v[0], 0xFF0000);

// Draw node forward vector
    v[0].vx = PSX_LENGTH(pNode->Centre.v[0]);
    v[0].vy = PSX_LENGTH(pNode->Centre.v[1]);
    v[0].vz = PSX_LENGTH(pNode->Centre.v[2]);
    v[1].vx = v[0].vx + (pLinkInfo->forwardVec.v[0] >> (12-4));
    v[1].vy = v[0].vy + (pLinkInfo->forwardVec.v[1] >> (12-4));
    v[1].vz = v[0].vz + (pLinkInfo->forwardVec.v[2] >> (12-4));
    RenderLineF2(pOT, &v[0], &v[1], 0xFFFFFF);

// Draw node right vector
    v[0].vx = PSX_LENGTH(pNode->Centre.v[0]);
    v[0].vy = PSX_LENGTH(pNode->Centre.v[1]);
    v[0].vz = PSX_LENGTH(pNode->Centre.v[2]);
    v[1].vx = v[0].vx + (pLinkInfo->rightVec.v[0] >> (12-4));
    v[1].vy = v[0].vy + (pLinkInfo->rightVec.v[1] >> (12-4));
    v[1].vz = v[0].vz + (pLinkInfo->rightVec.v[2] >> (12-4));
    RenderLineF2(pOT, &v[0], &v[1], 0xFFFFFF);

// Draw racing line
    v[0].vx = PSX_LENGTH(pNode->Centre.v[0]);
    v[0].vy = PSX_LENGTH(pNode->Centre.v[1]);
    v[0].vz = PSX_LENGTH(pNode->Centre.v[2]);
    v[1].vx = PSX_LENGTH(pNodeN->Centre.v[0]);
    v[1].vy = PSX_LENGTH(pNodeN->Centre.v[1]);
    v[1].vz = PSX_LENGTH(pNodeN->Centre.v[2]);
    RenderLineF2(pOT, &v[0], &v[1], 0xa0a0a0);

// Draw right vector

// Draw car's destination forward vector
    v[0].vx = PSX_LENGTH(pNode->Centre.v[0]);
    v[0].vy = PSX_LENGTH(pNode->Centre.v[1]);
    v[0].vz = PSX_LENGTH(pNode->Centre.v[2]);
    v[1].vx = v[0].vx + (pPlayer->CarAI.forwardVecDest.v[0] >> (12-4));
    v[1].vy = v[0].vy + (pPlayer->CarAI.forwardVecDest.v[1] >> (12-4));
    v[1].vz = v[0].vz + (pPlayer->CarAI.forwardVecDest.v[2] >> (12-4));
    RenderLineF2(pOT, &v[0], &v[1], 0x00FFFF);
}

#endif  //_PSX


#endif  //__AI_DEBUG_INFO
