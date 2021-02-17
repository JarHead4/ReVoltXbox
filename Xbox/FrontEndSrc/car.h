//-----------------------------------------------------------------------------
// File: car.h
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef CAR_H
#define CAR_H

#include "newcoll.h"
#include "particle.h"
#include "body.h"
#include "model.h"
#include "aerial.h"
#include "Wheel.h"
#include "sfx.h"  //$ADDITION

#include "XBResource.h"  //$ADDED(jedl) - graphics resources

#include "SoundEffectEngine.h"
class CSoundEffectInstance;


#ifndef _CARCONV
#include "competition.h"
#endif

#ifdef _PC
#ifndef _CARCONV
#include "network.h"
#endif
#endif

// macros


#ifndef _PSX
#define CALC_GOOD_CAR_BBOXES    TRUE
#else
#define CALC_GOOD_CAR_BBOXES    FALSE
#endif

#define MAX_CAR_FILENAME    64

#define MAX_CAR_LOD 5
#define CAR_LOD_BIAS 384
#define CAR_RADIUS 70
#define CAR_SQUARE_RADIUS (CAR_RADIUS * CAR_RADIUS)

#define CAR_MODEL_SPRING    1
#define CAR_MODEL_AXLE      2
#define CAR_MODEL_PIN       4
#define CAR_MODEL_WHEEL     8

#define CAR_MODEL_SPINNER   1
#define CAR_MODEL_AERIAL    2

#define CAR_MODEL_NONE  -1

#define CAR_NWHEELS     4
#define CAR_NRESPONSE   100
#define CAR_NAMELEN     20

#define CAR_REPOS_TIMER TO_TIME(Real(2.0))

#define CLOSE_WHEEL_COLL_ANGLE  Real(0.5f)

/////////////////////
// car render flag //
/////////////////////

enum {
    CAR_RENDER_OFF,
    CAR_RENDER_NORMAL,
    CAR_RENDER_GHOST,
};

////////////////////////////////////////////////////////////////
// Type of engine the car has
////////////////////////////////////////////////////////////////
typedef enum CarClassEnum {
    CAR_CLASS_ELEC,
    CAR_CLASS_GLOW,
    CAR_CLASS_OTHER,
    
    CAR_NCLASSES
} CAR_CLASS;

////////////////////////////////////////////////////////////////
// Transmission
////////////////////////////////////////////////////////////////
typedef enum CarTransEnum {
    CAR_TRANS_4WD,
    CAR_TRANS_FWD,
    CAR_TRANS_RWD,

    CAR_TRANS_NTYPES
} CAR_TRANS;

////////////////////////////////////////////////////////////////
// How the car is obtained
////////////////////////////////////////////////////////////////
typedef enum CarObtainEnum {
    CAR_OBTAIN_NEVER = -1,
    CAR_OBTAIN_DEFAULT = 0,
    CAR_OBTAIN_CHAMPIONSHIP,
    CAR_OBTAIN_TIMETRIAL,
    CAR_OBTAIN_PRACTICE,
    CAR_OBTAIN_SINGLE,
    CAR_OBTAIN_TRAINING,

    CAR_NOBTAINS
} CAR_OBTAIN;

////////////////////////////////////////////////////////////////
// Difficulty rating of the car
////////////////////////////////////////////////////////////////
typedef enum CarRating {
    CAR_RATING_ROOKIE = 0,      LEVEL_CLASS_DEFAULT = 0,
    CAR_RATING_AMATEUR = 1,     LEVEL_CLASS_BRONZE = 1,
    CAR_RATING_ADVANCED = 2,    LEVEL_CLASS_SILVER = 2,
    CAR_RATING_SEMIPRO = 3,     LEVEL_CLASS_GOLD = 3,
    CAR_RATING_PRO = 4,         LEVEL_CLASS_SPECIAL = 4,

    CAR_NRATINGS
} CAR_RATING;


////////////////////////////////////////////////////////////////
//
// Car Name enumerations
//
////////////////////////////////////////////////////////////////

enum CarTypeEnum {
    CARID_RC,
    CARID_DUSTMITE,
    CARID_PHATSLUG,
    CARID_COLMOSS,
    CARID_HARVESTER,
    CARID_DOCGRUDGE,
    CARID_VOLKEN,
    CARID_SPRINTER,
    CARID_DYNAMO,
    CARID_CANDY,
    CARID_GENGHIS,
    CARID_FISH,
    CARID_MOUSE,
    CARID_FLAG,
    CARID_PANGATC,
    CARID_R5,
    CARID_LOADED,
    CARID_BERTHA,
    CARID_INSECTO,
    CARID_ADEON,
    CARID_FONE,
    CARID_ZIPPER,
    CARID_ROTOR,
    CARID_COUGAR,
    CARID_SUGO,
    CARID_TOYECA,
    CARID_AMW,
    CARID_PANGA,
    CARID_TROLLEY,
    CARID_KEY1,
    CARID_KEY2,
    CARID_KEY3,
    CARID_KEY4,
#ifndef _N64
    CARID_UFO,
#endif
#ifdef _PC
    CARID_MYSTERY,
#endif

    CARID_NTYPES
};

// Car Status Flags

#define CAR_ACTIVE  1

#define FL  0
#define FR  1
#define BL  2
#define BR  3

typedef long CAR_TYPE;

// car model types
// - no longer have to put the models in the correct places...

enum {
    CAR_MODEL_BODY,
    CAR_MODEL_WHEEL1,
    CAR_MODEL_WHEEL2,
    CAR_MODEL_WHEEL3,
    CAR_MODEL_WHEEL4,
    CAR_MODEL_SPRING1,
    CAR_MODEL_SPRING2,
    CAR_MODEL_SPRING3,
    CAR_MODEL_SPRING4,
    CAR_MODEL_AXLE1,
    CAR_MODEL_AXLE2,
    CAR_MODEL_AXLE3,
    CAR_MODEL_AXLE4,
    CAR_MODEL_PIN1,
    CAR_MODEL_PIN2,
    CAR_MODEL_PIN3,
    CAR_MODEL_PIN4,
    CAR_MODEL_AERIAL_SEC1,
    CAR_MODEL_AERIAL_TOP1,
    MAX_CAR_MODEL_TYPES
};

#ifdef _PC

#define CAR_CLOTH_VERTS_MAX     128

typedef struct
{
    MODEL_VERTEX*   pV;             // vertex
    REAL            length;         // length
    VEC             vel;            // velocity
    VEC             imp;            // impulse
    REAL            damping;
    VEC             pos;            // original position

} CAR_CLOTH_POINT;

typedef struct
{
    int             cVerts;
    CAR_CLOTH_POINT pVerts[CAR_CLOTH_VERTS_MAX];

} CAR_CLOTH;

#endif


#ifndef _PSX 

typedef struct
{
    char WheelPartsFlag[4];
    char BodyPartsFlag;

    VEC OffBody;

    VEC OffWheel[4];
    VEC OffSpring[4];
    VEC OffAxle[4];
    VEC OffPin[4];
    VEC OffWheelColl[4];
    VEC OffSpinner;
    VEC OffAerial;
    VEC DirAerial;

    REAL WheelRad[4];
    REAL SpringLen[4];
    REAL AxleLen[4];
    REAL PinLen[4];
    REAL AerialLen;

    MODEL Model[MAX_CAR_MODEL_TYPES][MAX_CAR_LOD];

    MODEL *Body;
    MODEL *Wheel[4];
    MODEL *Spring[4];
    MODEL *Axle[4];
    MODEL *Pin[4];
    MODEL *Spinner;
    MODEL *Aerial[2];

#ifndef _N64
    CAR_CLOTH   cloth;
#endif

    long EnvRGB;

    COLLSKIN_INFO           CollSkin;           // coll skin
} CAR_MODEL;


#endif



//
// CAR_INFO
//
// Car initialisation structure
//
typedef struct {
    long    ModelNum;
    VEC Offset;
    REAL    Mass;
    MAT Inertia;
    REAL    Gravity;
    REAL    Hardness;
    REAL    Resistance;
    REAL    AngResistance;
    REAL    ResModifier;
    REAL    Grip;
    REAL    StaticFriction;
    REAL    KineticFriction;
} BODY_INFO;

typedef struct {
    long    ModelNum;
    VEC Offset;
    REAL    Length;
} AXLE_INFO;

typedef struct {
    long    ModelNum;
    VEC Offset;
    REAL    Length;
} PIN_INFO;

typedef struct {
    long    SecModelNum;
    long    TopModelNum;
    VEC Offset;
    VEC Direction;
    REAL    SecLen;
    REAL    Stiffness;
    REAL    Damping;
} AERIAL_INFO;

typedef struct SpinnerInfoStruct {
    long    ModelNum;
    VEC Offset;
    VEC Axis;
    REAL    AngVel;
} SPINNER_INFO;

typedef struct AIInfoStruct {
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

    int     pickupBias;
    int     blockBias;
    int     overtakeBias;
    int     suspension;
    int     aggression;
} AI_INFO;


typedef struct
{
#if defined (_PC) || defined (_CARCONV)
    char ModelFile[MAX_CAR_MODEL_TYPES][MAX_CAR_FILENAME];
    char TPageFile[MAX_CAR_FILENAME];
    char CollFile[MAX_CAR_FILENAME];
#endif

    long    EnvRGB;
    char    Name[CAR_NAMELEN];

    REAL    SteerRate;
    REAL    SteerModifier;
    REAL    EngineRate;
    REAL    TopSpeed;
    REAL    MaxRevs;
    REAL    DownForceMod;
    VEC     CoMOffset;
    bool    AllowedBestTime;
    bool    Selectable;
    VEC     WeaponOffset;

    BODY_INFO   Body;
    WHEEL_INFO  Wheel[CAR_NWHEELS];
    SPRING_INFO Spring[CAR_NWHEELS];
    AXLE_INFO   Axle[CAR_NWHEELS];
    PIN_INFO    Pin[CAR_NWHEELS];
    SPINNER_INFO Spinner;
    AERIAL_INFO Aerial;
    AI_INFO     AI;

    // Accessibility
    CAR_CLASS       Class;
    CAR_RATING      Rating;
    CAR_OBTAIN      ObtainMethod;

    // Car statistics
    REAL            TopEnd;
    REAL            Acc;
    REAL            Handling;
    REAL            Weight;
    long            Trans;

//$ADDITION_BEGIN(jedl) - new art resources
    XBResource*     m_pXBR;
    UINT            m_RefCountXBR; // keep track of how many cars are using this resource bundle
//$ADDITION_END

#ifdef _PC
    // Cheat flags
    bool            Modified;       // Is this car info modified?
    bool            Moved;          // Has the Info file been moved?
#endif

} CAR_INFO;



typedef struct
{
    REAL SpringLen;
    REAL AxleLen;
    REAL PinLen;

    VEC SpringOffset;
    VEC AxleOffset;
    VEC FixOffset;

    VEC SpringWorldPos;
    VEC AxleWorldPos;
    VEC PinWorldPos;

    MAT SpringCarMatrix;
    MAT AxleCarMatrix;
    MAT PinCarMatrix;
} SUSPENSION;


typedef struct SpinnerStruct {
    MAT CarMatrix;
    MAT Matrix;
    VEC WorldPos;
    VEC Axis;
    REAL    AngVel;
} SPINNER;


/////////////////////////////////////////////////////////////////////
//
// Remote car interp data
//

#define REMOTE_POS      1l
#define REMOTE_VEL      2l
#define REMOTE_ANGVEL   4l
#define REMOTE_QUAT     8l
#define REMOTE_WHLANG   16l
#define REMOTE_WHLPOS   32l
#define REMOTE_TIME     64l
#define REMOTE_CONTROL  128l


typedef struct RemoteDataStruct {

    long    NewData;

    VEC     Pos;                // Probably should be passed as full vector
    VEC     Vel;                // Could be passed as three signed shorts
    VEC     AngVel;             // Also three signed shorts
    QUATERNION  Quat;           // Four signed chars should do it

    char            dx, dy;     // control inputs
    unsigned short  digital;    // digital inputs
#ifdef _PC
    DWORD   Time;               // senders race time
#else
    unsigned long Time;
#endif
} CAR_REMOTE_DATA;


#ifdef _PC

typedef struct {

    VEC             Pos;                        // car pos
    short           VelX, VelY, VelZ;           // car vel
    short           AngVelX, AngVelY, AngVelZ;  // car ang vel
    char            QuatX, QuatY, QuatZ, QuatW; // car orientation

    char    dx, dy;                     // control inputs
    unsigned short  digital;                    // digital inputs

    DWORD           Time;                       // race time

} CAR_REMOTE_DATA_SEND;

typedef struct {
    long Car;           // car id
} CAR_REMOTE_NEWCAR;
#endif

// grid position structure

typedef struct {
    REAL xoff, yoff, zoff, rotoff;
} GRID_POS;

//
// CAR
//
// Main car data structure
//

typedef struct CarStruct {

    SUSPENSION      Sus[4];

    // NewCar stuff ported across
    // Almost everything above here should be removed...eventually

    long            CarType; //$CMP_NOTE: this was originally named "CarID"

    VEC             BodyWorldPos;       // world pos of car body for rendering
    MAT             EnvMatrix;          // env matrix 

    CAR_MODEL       *Models;

    REAL            SteerAngle;         // Angle of steering wheel (clamped to fixed precision)
    REAL            LastSteerAngle;     // Angle of steering wheel (clamped to fixed precision)
    REAL            SteerRate;
    REAL            fullLockAngle;      // Max steering angle (full-lock)
    REAL            AISteerConvert;     // For AI steering
    REAL            SteerModifier;
    REAL            EngineVolt;         // Power on the engine (clamped to fixed precision)
    REAL            LastEngineVolt;     // Last EngineVolt
    REAL            EngineRate;
    REAL            TopSpeed;           // Top speed of car
    REAL            DefaultTopSpeed;
    REAL            DownForceMod;       // Down force modifier

    NEWBODY         *Body;
    AERIAL          Aerial;
    WHEEL           Wheel[CAR_NWHEELS];
    SPRING          Spring[CAR_NWHEELS];
#ifndef _PSX
    SPINNER         Spinner;
#endif

    BBOX            BBox;                       // Bounding box including wheels etc
    REAL            CollRadius;                 // Collision radius for Arcade Mode

    int             NBodySpheres;
    SPHERE          BodySphere[2];
    SPHERE          CollSphere[2];

    VEC             BodyOffset;                 // Body model offset relative to CoM
    VEC             WheelOffset[CAR_NWHEELS];   // Wheel fix point relative to car CoM
    VEC             WheelCentre[CAR_NWHEELS];   // Wheel centre relative to wheel fix point
    VEC             SuspOffset[CAR_NWHEELS];    // Suspension fix point
    VEC             AxleOffset[CAR_NWHEELS];    // Axle fix point
    VEC             SpinnerOffset;
    VEC             AerialOffset;               // Aerial fixed point relative to car CoM
    VEC             WeaponOffset;               // Offset where weapon fired from

    long            Rendered;                   // car was rendered on last camera
    long            RenderedAll;                // car was rendered at least once last frame
    long            RenderFlag;                 // render car as ghost

    long            NextSplit;                  // next split ID
    long            NextTrackDir;               // next track dir
    long            Laps;                       // laps completed
    long            CurrentLapTime;             // current lap time
    long            LastLapTime;                // last lap time
    long            BestLapTime;                // best lap time
    long            LastRaceTime;               // last race time
    long            BestRaceTime;               // best race time
    long            SplitTime[MAX_SPLIT_TIMES]; // split times

    REAL            Current0to15;
    REAL            Best0to15;                  // best 0-20mph time
    REAL            Current0to25;
    REAL            Best0to25;                  // best 0-20mph time

    unsigned long   CurrentLapStartTime;        // start time for current lap

    REAL            Revs;                       // Engine Revs
    REAL            MaxRevs;                    // For the tachometer

    REAL            PowerTimer;                 // Seconds until power restored (electropulsed!)
    REAL            AerialTimer;                // Timer to "grow" the aerial when car first generated
    long            AddLit;                     // Light to add when rendered
    REAL            DrawScale;                  // global draw scale
    long            IsBomb;                     // TRUE if car is a bomb
    long            WillDetonate;               // TRUE if car is a bomb and just about to detonate
    REAL            NoReturnTimer;              // timer to stop instant return of bomb
    long            IsFox;                      // TRUE if car is fox in tag multiplayer
    REAL            FoxReturnTimer;             // timer to stop instant return of 'foxness'
    REAL            RepositionTimer;            // repositioning timer or 0
    REAL            RepositionHalf;             // repositioning halfway flag for move handler
    REAL            LastHitTimer;               // Time since last hit by a weapon

    // Selp-righting stuff
    VEC             DestPos;
    QUATERNION      DestQuat;

    VEC             FieldVec;

    // Collision information stuff
    COLLINFO_WHEEL  *WheelCollHead;
    int             NWheelColls;                // Number of wheel collisions on this car

    int             NWheelsInContact;           // Number of wheels in contact with something
    int             NWheelFloorContacts;        // Number of contacts with the floor (< 60 degrees to horizontal)

#if !defined(_PSX) && !defined(_CARCONV)
 #ifdef OLD_AUDIO
    SAMPLE_3D       *SfxEngine;                 // engine sfx
    SAMPLE_3D       *SfxScrape;                 // scrape sfx
    SAMPLE_3D       *SfxScreech;                // screech sfx
    SAMPLE_3D       *SfxServo;                  // servo sfx
 #else // !OLD_AUDIO
    CSoundEffectInstance*   pSourceMix;
    CSoundEffectInstance*   pEngineSound;
    CSoundEffectInstance*   pScrapeSound;
    CSoundEffectInstance*   pScreechSound;
    //$TODO: JHarding probably needs to add pServoSound.
 #endif // !OLD_AUDIO
    long            ScrapeMaterial;             // scrape material num
    long            SkidMaterial;               // scrape material num for wheels
    REAL            ServoFlag;
    long            InWater;                    // true if car is in water
#endif

    long            RemoteTimeout;              // Whether the remote data has timed out
    int             RemoteNStored;              
    CAR_REMOTE_DATA RemoteData[3];              // Most recent remote data recieved from the network
    int             OldDat, Dat, NewDat;

    // Accessibility
    CAR_CLASS       Class;
    CAR_RATING      Rating;
    CAR_OBTAIN      ObtainMethod;

    // Flags
    bool            AllowedBestTime;
    bool            Selectable;
    bool            Righting;
    bool            RightingCollide;
    bool            RightingReachDest;
    bool            Timing0to15;
    bool            Timing0to25;
    bool            bPad;


} CAR;

#ifndef _PSX
#define SetCarHasSpring(car, i) ((car)->Models->WheelPartsFlag[i] |= CAR_MODEL_SPRING)
#define SetCarHasAxle(car, i) ((car)->Models->WheelPartsFlag[i] |= CAR_MODEL_AXLE)
#define SetCarHasPin(car, i) ((car)->Models->WheelPartsFlag[i] |= CAR_MODEL_PIN)
#define SetCarHasWheel(car, i) ((car)->Models->WheelPartsFlag[i] |= CAR_MODEL_WHEEL)
#define CarHasSpring(car, i) ((car)->Models->WheelPartsFlag[i] & CAR_MODEL_SPRING)
#define CarHasAxle(car, i) ((car)->Models->WheelPartsFlag[i] & CAR_MODEL_AXLE)
#define CarHasPin(car, i) ((car)->Models->WheelPartsFlag[i] & CAR_MODEL_PIN)
#define CarHasWheel(car, i) ((car)->Models->WheelPartsFlag[i] & CAR_MODEL_WHEEL)

#define SetCarHasSpinner(car) ((car)->Models->BodyPartsFlag[i] != CAR_MODEL_SPINNER)
#define CarHasSpinner(car) ((car)->Models->BodyPartsFlag & CAR_MODEL_SPINNER)
#define SetCarHasAerial(car) ((car)->Models->BodyPartsFlag[i] != CAR_MODEL_AERIAL)
#define CarHasAerial(car) ((car)->Models->BodyPartsFlag & CAR_MODEL_AERIAL)
#else
#define CarHasAerial(car) ((car)->Models->Aerial1 != -1)
#endif


//
// External function prototypes
//
struct PlayerStruct;
struct object_def;

extern CAR_INFO *CreateCarInfo(long nInfo);
extern void DestroyCarInfo();
extern CAR_MODEL *CreateCarModels(long nModels);
extern void DestroyCarModels(CAR_MODEL *models);


extern void InitCar(CAR *car);
extern void SetupCar(struct PlayerStruct *player, int carType);
extern void FreeCar(struct PlayerStruct *player);
extern void GetCarStartGrid(long position, VEC *pos, MAT *mat);  //$CMP_NOTE: this was originally named GetCarGrid()
extern void SetCarPos(CAR *car, VEC *pos, MAT *mat);
extern void SetCarAerialPos(CAR *car);
extern void UpdateCarAerial2(CAR *car, REAL dt);
extern void ResetCarWheelPos(CAR *car, int iWheel);
extern void UpdateCarWheel(CAR *car, int iWheel, REAL dt);

extern void DetectCarWorldColls(CAR *car);
extern int DetectCarBodyColls(CAR *car, NEWBODY *body);
extern int DetectCarCarColls(CAR *car1, CAR *car2);
extern void DetectCarWheelColls(CAR *car, int iWheel, COLLSKIN *worldSkin);
extern void DetectCarWheelColls2(CAR *car, int iWheel, NEWCOLLPOLY *worldPoly);
//extern void DetectCarWheelColls2(CAR *car, int iWheel, COLL_POLY *worldPoly);
extern void PreProcessCarWheelColls(CAR *car);
extern void ProcessCarWheelColls(CAR *car);
extern void PostProcessCarWheelColls(CAR *car);
extern void SetAllCarCoMs(void);
extern void UnsetAllCarCoMs(void);
extern void MoveCarCoM(CAR_INFO *carInfo, VEC *dR);
extern void CarWheelImpulse(CAR *car, COLLINFO_WHEEL *collInfo, VEC *impulse);
extern void CarWheelImpulse2(CAR *car, COLLINFO_WHEEL *collInfo, VEC *impulse);
extern void SetCarAngResistance(CAR *car);
extern void CAR_AllCarColls();
extern CAR_REMOTE_DATA *NextRemoteData(CAR *car);
#ifdef _PC
extern void SendCarData(struct object_def *obj);
extern void ProcessCarData(void);
extern void SendCarNewCar(long car);
extern void ProcessCarNewCar(void);
extern void SendCarNewCarAll(long car);
extern void ProcessCarNewCarAll(void);
#endif
extern void LoadOneCarModelSet(struct PlayerStruct *player, long car);
extern void FreeOneCarModelSet(struct PlayerStruct *player);
extern int NextValidCarType(int iCurrType);  //$CMP_NOTE: this was originally named NextValidCarID(int currentID)
extern int PrevValidCarType(int iCurrType);  //$CMP_NOTE: this was originally named PrevValidCarID(int currentID)
extern void CheckCarSituation(CAR *car);
extern void CarAccTimings(CAR *car);
extern void CarDownForce(CAR *car);
extern void RemoveWheelColl(CAR *car, COLLINFO_WHEEL *collInfo);
extern COLLINFO_WHEEL *AddWheelColl(CAR *car, COLLINFO_WHEEL *newHead);
extern void SetAllCarSelect();
extern void UpdateCarPos(CAR *car, VEC *pos, MAT *mat);
extern void CalcCarStats();
extern void StartCarReposition(struct PlayerStruct *player);
extern void UpdatePlayerForceFeedback(struct PlayerStruct * pPlayer);
extern void UpdateReplayForceFeedback(struct PlayerStruct * pPlayer);
extern long GetCarTypeFromName(char *name);

//
// External global variables
//

extern CAR_INFO     *CarInfo;
extern long         NCarTypes;

#ifdef _PC
extern char         *CarDirs[];
#endif

extern bool CAR_DrawCarBBoxes;
extern bool CAR_DrawCarAxes;
extern bool CAR_WheelsHaveSuspension;

#ifndef _CARCONV
extern GRID_POS CarGridStarts[][MAX_RACE_CARS];
#endif

#endif // CAR_H

