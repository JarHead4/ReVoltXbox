//-----------------------------------------------------------------------------
// File: AI_Car.h
//
// Desc: Car AI
//
// Re-Volt (Generic) Copyright (c) Probe Entertainment 1998
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef AI_CAR_H
#define AI_CAR_H

#include "ainode.h"
#include "ai.h"
#ifndef _N64
#include "camera.h"
#endif
//
// Defines and macros
//

//#define OLD_AI_VERSION

#define MAX_OUT_OF_NODE_CNT         TO_TIME(Real(10))   // Max time allowed to be out of nodes (in seconds)
#define MAX_OUT_OF_ZONE_CNT         TO_TIME(Real(1))    // Max time allowed to be out of zone (in seconds)

// Difficulty levels
typedef enum
{
    DIFFICULTY_VERY_EASY,
    DIFFICULTY_EASY,
    DIFFICULTY_MEDIUM,
    DIFFICULTY_HARD,
    DIFFICULTY_NUM,

} DIFFICULTY_LEVEL;

// AI States
typedef enum
{
    CAI_S_NULL = 0,                 // No state
    CAI_S_STARTING_GRID,            // Car is on the starting grid
    CAI_S_GREEN_LIGHT,              // Green light activated
    CAI_S_RACE,                     // Racing
    CAI_S_BRAKING_ZONE,             // Car is in the braking zone
    CAI_S_REVERSE_CORRECT,          // Reverse correction
    CAI_S_FORWARD_CORRECT,          // Forward correction
    CAI_S_FIND_WAY_BACK_TO_NODE,    // Find your way back to the nodes
    CAI_S_IN_THE_AIR,               // Car is Duke's of Hazzard-ing it
    CAI_S_LANDED,                   // Car has just landed from a jump
    CAI_S_CORRECT_FORWARDLEFT,      // Forward & Left
    CAI_S_CORRECT_FORWARDRIGHT,     // Forward & Right
    CAI_S_CORRECT_REVERSELEFT,      // Reverse & Left
    CAI_S_CORRECT_REVERSERIGHT,     // Reverse & Right

} CAI_STATE;



typedef enum
{
    CAI_T_FORWARD = 0,
    CAI_T_BACKWARD,
    CAI_T_REVERSE_F,
    CAI_T_REVERSE_B
} CAI_TMODE;


typedef enum
{
    CAI_SK_RACER,               // Boy racer

    MAX_SKILL_TYPES
} CAI_SK_TYPES;

//
// Typedefs and structures
//
// Values are fixed point (0.8.8)

typedef struct _CAI_SKILLS
{
    int     pickupBias;
    int     blockBias;
    int     overtakeBias;
    int     suspension;

//  REAL    pickupBias;
//  REAL    blockBias;
//  REAL    overtakeBias;
//  REAL    suspension;

//  REAL    RaceBias;                           // Bias for lerp between racing line and track centre
//  REAL    PriBias;                            // Bias for random priority choices (0-15)
//  REAL    PickupBias;                         // Base desire for pick-ups
//  REAL    BlockBias;                          // Base desire to block
//  REAL    OvertakeBias;                       // Base desire to overtake
} CAI_SKILLS;

// Macro to setup skills
//#define SKILL_SETTING(pickup, block, overtake)\
//  {(int)(65536.0 * (pickup)),\
//   (int)(65536.0 * (block)),\
//   (int)(65536.0 * (overtake))},

//#define NEURAL_NET
#ifdef NEURAL_NET
#define NEURAL_INPUT_NUM    3
#define NEURAL_LAYER_NUM    10
#define NEURAL_OUTPUT_NUM   2

typedef struct s_NeuralNet
{
    REAL    input[NEURAL_LAYER_NUM][NEURAL_INPUT_NUM];
    REAL    inputWeight[NEURAL_LAYER_NUM];
    REAL    layer[NEURAL_LAYER_NUM];
    REAL    outputWeight[NEURAL_LAYER_NUM];
    REAL    output[NEURAL_LAYER_NUM][NEURAL_OUTPUT_NUM];

} t_NeuralNet;
#endif


// CatchUp Vars
typedef struct s_CatchUp
{
    REAL    lenSpeedUp;
    REAL    lenSlowDown;
    REAL    distSpeedUpAdjust;
    bool    bSpeedUpOvertake;
    bool    bSlowDownOvertake;
    REAL*   pSpeedUpTable;
    REAL*   pSlowDownTable;

    REAL    beachMaxTime;
    REAL    beachMaxDist;
    REAL    beachMaxDist2;
    REAL    flipCarTime;

} t_CatchUp;


// Car Ai structure
typedef struct _CAR_AI
{
    long            IsInZone;                   // true if car is within a zone
    long            IsOnTrack;                  // true if car is on valid track

    long            ZoneID;                     // current race zone ID
    long            CurZone;                    // current zone occupied by the car
    long            CurZoneID;                  // current zone ID occupied by the car
    long            CurZoneBBox;                // current zone hull occupied by the car
    long            LastValidZone;              // last valid zone ID
    REAL            exitSideDist;               // node exit side distance (negative for left, positive for right edge)

    AINODE*         pCurNode;                   // current AI the car is in
    AINODE*         pLastValidNode;             // last valid node
    AINODE*         pLastSplitNode;             // last node that split into 2 paths
    AINODE*         pDestNode;                  // current AI node the car is traking (heading for)
    AINODE*         pLastDestNode;              // last AI node the car is traking (heading for)
    AINODE*         pFutureNode;                // future AI node the car is traking (heading for)

    REAL            NodeDist;                   // distance to nearest forward node
    REAL            curNodeEdgeDist[2];         // Distance to left & right edges Car's Position
    REAL            lastCurNodeEdgeDist[2];     // Last distance to left & right edges Car's Position
    REAL            futureCurNodeEdgeDist[2];   // Future distance to left & right edges Car's Position
    REAL            dstNodeEdgeDist[2];         // Distance to left & right edges Destination Position

    short           iRouteCurNode;              // current choice (0 to MAX_AINODE_LINKS-1) for route splits
    short           iRouteDestNode;             // current choice (0 to MAX_AINODE_LINKS-1) for route splits
    int             routeRandom[MAX_AINODE_LINKS];
    bool            bRouteTaken;                // Zero if route was taken.

                                                // "Intelligence"
    CAI_STATE       AIState, lastAIState;       // Current & last AI state
    REAL            AIStateCount;               // Misc. state counter (eg. do this state for this amount of time)
    CAI_TMODE       TrackMode;                  // Current node tracking mode
    CAI_SKILLS      Skills;                     // Skill bias structure

    REAL            cZoneReset;                 // counter for car "resetting" (out of zone)
    REAL            cNodeReset;                 // counter for car "resetting" (off track)
    REAL            StuckCnt;                   // counter for car "resetting" (stuck)

#ifdef _N64
    long            WrongWayFlag;
    float           WrongWayTimer;
#endif

    long            WrongWay;                   // true if car facing wrong way
    long            BackTracking;               // true if player has gone back over the start line
    long            PreLap;                     // true if player has never crossed the line
    long            FinishDistNode;             // current 'finish dist' node
    REAL            FinishDist;                 // distance to finish line
    REAL            FinishDistPanel;            // distance to finish line for control panel
    REAL            AngleToNextNode;            // distance to finish line for control panel

    REAL            cosAngleToRacingLine;       // Cosine of angle of car's forward vector to the racing line's forward vector
    REAL            absCosAngleToRacingLine;    // ABS Cosine of angle of car's forward vector to the racing line's forward vector
    int             raceLineFaceDir;            // racing direction (1 = with racing dir, -1 = against racing dir)
    REAL            raceLineSide;               // Which side of the racing line the car is on (-1 or 1)
    REAL            dstRaceLineSide;                // Which side of the racing line the car is on (-1 or 1)
    VEC             rightVec;                   // Car's current right vector destination.
    VEC             forwardVec;                 // Car's current forward vector (on XZ plane.  Y=0).
    VEC             forwardVecDest;             // Car's destination forward vector. (on XZ plane.  Y=0).
    VEC             destPos;                    // Car's destination position

    VEC             beechedPos;                 // Car's position 'x' seconds ago
    REAL            beechedPosCount;

    REAL            steerOverUnder;             // Over or under steering (pos = oversteer, neg = understeer)

    REAL            biasSize;
    REAL            biasMaxSize;
    REAL            biasEdge;
    REAL            biasExpandRate;
    REAL            biasShrinkRate;

    REAL            lookAheadDist;              // Look ahead distance

    int             usePickup;
    REAL            pickupDuration;             // How long the car has had a pickup for

    REAL            speedMax;                   // Maximum speed
    REAL            speedCur;                   // Current speed
    REAL            distAlongNode;              // Distance along current node
    VEC             futurePos;


    struct PlayerStruct *pNearestCarInFront;    // Nearest car infront
    struct PlayerStruct *pNearestCarBehind;     // Nearest car behind
    REAL                distNearestCarInFront;
    REAL                distNearestCarBehind;

    int             racePosition;               // Position in race
    int             raceDistance;               // Distance in race

    // Control settings
    signed char     dx, dy;
    short           digital;

    // Current AI states
    int             catchUpMode;                // Catchup mode (0,-1,1 = None,SlowDown, CatchUp)
    short           bOvertake, bOvertakeLast;   // Overtake opponent
    REAL            timeOvertake;
    REAL            curRacingLine;              // Current racing line (0-1)
    REAL            dstRacingLine;              // Destination racing line (0-1)

    // Under/oversteer values
    REAL            understeerThreshold;
    REAL            understeerRange;
    REAL            understeerFront;
    REAL            understeerRear;
    REAL            understeerMax;
    REAL            oversteerThreshold;
    REAL            oversteerRange;
    REAL            oversteerMax;
    REAL            oversteerAccelThreshold;
    REAL            oversteerAccelRange;


#ifdef GAZZA_TEACH_CAR_HANDLING
        int lastLap;
        int cLaps;
        REAL bestTime;
        bool fBestTime;
        REAL neuralCur[10];
        REAL neuralBest[10];
        REAL neuralMin[10];
        REAL neuralMax[10];
        REAL neuralLastMod[10];
        int iLastNeuron;
#endif

#ifdef NEURAL_NET
        t_NeuralNet neuralNet;
#endif

} CAR_AI;

//
// External global variables
//
extern t_CatchUp    *gpCatchUpVars;


//
// External function prototypes
//
struct PlayerStruct;
struct CameraStruct;

extern void CAI_InitCarAI(struct PlayerStruct *Player, long Skill);

extern void CAI_CalcCarRacePositions(void);
extern void CAI_InitPickupWeights(void);

extern void CAI_CarHelper(struct PlayerStruct *Player);
extern void CAI_ResetCar(struct PlayerStruct *Player);
extern bool CAI_IsCarStuck(struct PlayerStruct *Player);
extern bool CAI_IsCarInZone(struct PlayerStruct *Player);
extern bool CAI_IsCarOnTrack(struct PlayerStruct *Player);
extern void CAI_TriggerAiHome(struct PlayerStruct *Player, long flag, long n, PLANE *planes);

extern int CAI_ChooseNodeRoute(struct PlayerStruct *pPlayer, AINODE *pNode);

extern void CAI_FindClosestPlayer(struct PlayerStruct *pPlayer, struct PlayerStruct **pFront, struct PlayerStruct **pBehind, REAL *pDistF, REAL *pDistB);
extern void CAI_CalcCarRaceDistances(void);
extern void CAI_InitCatchUp(DIFFICULTY_LEVEL iLevel);
extern void CAI_CatchUp(struct PlayerStruct *pPlayer);

extern void CAI_InitTimeStep(void);
extern void CAI_UpdateTimeStep(int dT);
extern void CAI_Process(struct PlayerStruct *pPlayer);
extern void CAI_UpdateNodeData(struct PlayerStruct *pPlayer);
extern int CAI_UpdateAiParameters(struct PlayerStruct *pPlayer);

extern void RenderAiDebugInfo(struct PlayerStruct *pPlayer, long *pOT, struct CameraStruct *pCam);

extern void InitCarInfoAIData();
extern void ReSaveCarInfoAIData(struct PlayerStruct* Player);



extern char *gCAI_StateDebugStrings[];
extern REAL gCAI_TimeStep;
extern int gCAI_cProcessAI;


#endif // AI_CAR_H

