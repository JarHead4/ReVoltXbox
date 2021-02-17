
#ifndef CAR_H
#define CAR_H

#include "newcoll.h"
#include "particle.h"
#include "body.h"
#include "model.h"
#include "aerial.h"
#include "Wheel.h"

#ifdef _PC
#ifndef _CARCONV
#include "sfx.h"
#include "play.h"
#endif

#endif

// macros

#define MAX_CAR_FILENAME	64

#define MAX_CARS 15
#define MAX_CAR_LOD 5
#define CAR_LOD_BIAS 384
#define CAR_RADIUS 70
#define CAR_SQUARE_RADIUS (CAR_RADIUS * CAR_RADIUS)

#define CAR_MODEL_SPRING	1
#define CAR_MODEL_AXLE		2
#define CAR_MODEL_PIN		4

#define CAR_MODEL_SPINNER	1
#define CAR_MODEL_AERIAL	2

#define CAR_MODEL_NONE	-1

#define CAR_NWHEELS		4
#define CAR_NRESPONSE	100
#define CAR_NAMELEN		64

#define CAR_REMOTE_QUAT_SCALE 100.0f
#define CAR_REMOTE_VEL_SCALE 3.0f
#define CAR_REMOTE_ANGVEL_SCALE 1000.0f

#define CLOSE_WHEEL_COLL_ANGLE	0.5f


// Car Status Flags

#define CAR_ACTIVE	1

#define FL	0
#define FR	1
#define BL	2
#define BR	3

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

	long EnvRGB;

	COLLSKIN_INFO			CollSkin;			// coll skin
} CAR_MODEL;


#endif



//
// CAR_INFO
//
// Car initialisation structure
//
typedef struct {
	long	ModelNum;
	VEC	Offset;
	REAL	Mass;
	MAT	Inertia;
	REAL	Gravity;
	REAL	Hardness;
	REAL	Resistance;
	REAL	AngResistance;
	REAL	ResModifier;
	REAL	Grip;
	REAL	StaticFriction;
	REAL	KineticFriction;
} BODY_INFO;

typedef struct {
	long	ModelNum;
	VEC	Offset;
	REAL	Length;
} AXLE_INFO;

typedef struct {
	long	ModelNum;
	VEC	Offset;
	REAL	Length;
} PIN_INFO;

typedef struct {
	long	SecModelNum;
	long	TopModelNum;
	VEC	Offset;
	VEC	Direction;
	REAL	SecLen;
	REAL	Stiffness;
	REAL	Damping;
} AERIAL_INFO;

typedef struct SpinnerInfoStruct {
	long	ModelNum;
	VEC	Offset;
	VEC	Axis;
	REAL	AngVel;
} SPINNER_INFO;


typedef struct
{
#ifdef _N64
	FIL_ID	TextureFile[MAX_CAR_MODEL_TYPES];
	FIL_ID	ModelFile[MAX_CAR_MODEL_TYPES];
	FIL_ID	CollFile;
#else
	char ModelFile[MAX_CAR_MODEL_TYPES][MAX_CAR_FILENAME];
	char TPageFile[MAX_CAR_FILENAME];
	char CollFile[MAX_CAR_FILENAME];
#endif

	long	EnvRGB;
	char	Name[CAR_NAMELEN];

	REAL	SteerRate;
	REAL	SteerModifier;
	REAL	EngineRate;
	REAL	TopSpeed;
	REAL	MaxRevs;
	REAL	DownForceMod;
	VEC		CoMOffset;
	bool	AllowedBestTime;
	bool	Selectable;
	VEC		WeaponOffset;

	BODY_INFO	Body;
	WHEEL_INFO	Wheel[CAR_NWHEELS];
	SPRING_INFO	Spring[CAR_NWHEELS];
	AXLE_INFO	Axle[CAR_NWHEELS];
	PIN_INFO	Pin[CAR_NWHEELS];
	SPINNER_INFO Spinner;
	AERIAL_INFO	Aerial;

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
	MAT	CarMatrix;
	MAT	Matrix;
	VEC	WorldPos;
	VEC	Axis;
	REAL	AngVel;
} SPINNER;


/////////////////////////////////////////////////////////////////////
//
// Remote car interp data
//

#define REMOTE_POS		1l
#define REMOTE_VEL		2l
#define REMOTE_ANGVEL	4l
#define REMOTE_QUAT		8l
#define REMOTE_WHLANG	16l
#define REMOTE_WHLPOS	32l
#define REMOTE_TIME		64l
#define REMOTE_CONTROL	128l

typedef struct RemoteDataStruct {

	long		PacketInfo;		// Bits set for items present

	bool		NewData;

	VEC		Pos;			// Probably should be passed as full vector
	VEC		Vel;			// Could be passed as three signed shorts
	VEC		AngVel;			// Also three signed shorts
	QUATERNION	Quat;			// Four signed chars should do it

	char		dx, dy;			// control inputs

} REMOTE_DATA;

#define RemoteTimePresent(info) ((info)->PacketInfo & REMOTE_TIME)
#define RemotePosPresent(info) ((info)->PacketInfo & REMOTE_POS)
#define RemoteVelPresent(info) ((info)->PacketInfo & REMOTE_VEL)
#define RemoteAngVelPresent(info) ((info)->PacketInfo & REMOTE_ANGVEL)
#define RemoteQuatPresent(info) ((info)->PacketInfo & REMOTE_QUAT)
#define RemoteWhlAngPresent(info) ((info)->PacketInfo & REMOTE_WHLANG)
#define RemoteWhlPosPresent(info) ((info)->PacketInfo & REMOTE_WHLPOS)

// grid position structure

typedef struct {
	REAL xoff, zoff, rotoff;
} GRID_POS;

//
// CAR
//
// Main car data structure
//

typedef struct CarStruct {

	SUSPENSION		Sus[4];

	// NewCar stuff ported across
	// Almost everything above here should be removed...eventually

	long			CarID;

	VEC				BodyWorldPos;		// world pos of car body for rendering
	MAT				EnvMatrix;			// env matrix 

	CAR_MODEL		*Models;

	REAL			SteerAngle;			// Angle of steering wheel (clamped to fixed precision)
	REAL			SteerRate;
	REAL			SteerModifier;
	REAL			EngineVolt;			// Power on the engine (clamped to fixed precision)
	REAL			EngineRate;
	REAL			TopSpeed;			// Top speed of car
	REAL			DefaultTopSpeed;
	REAL			DownForceMod;		// Down force modifier

	NEWBODY			*Body;
	AERIAL			Aerial;
	WHEEL			Wheel[CAR_NWHEELS];
	SPRING			Spring[CAR_NWHEELS];
	SPINNER			Spinner;

	BBOX			BBox;						// Bounding box including wheels etc

	VEC				BodyOffset;					// Body model offset relative to CoM
	VEC				WheelOffset[CAR_NWHEELS];	// Wheel fix point relative to car CoM
	VEC				WheelCentre[CAR_NWHEELS];	// Wheel centre relative to wheel fix point
	VEC				SuspOffset[CAR_NWHEELS];	// Suspension fix point
	VEC				AxleOffset[CAR_NWHEELS];	// Axle fix point
	VEC				SpinnerOffset;
	VEC				AerialOffset;				// Aerial fixed point relative to car CoM
	VEC				WeaponOffset;				// Offset where weapon fired from

	long			Rendered;					// car was rendered this frame

	long			NextSplit;					// next split ID
	long			Laps;						// laps completed
	long			CurrentLapTime;				// current lap time
	long			LastLapTime;				// last lap time
	long			BestLapTime;				// best lap time
	long			LastRaceTime;				// last race time
	long			BestRaceTime;				// best race time
	long			SplitTime[MAX_SPLIT_TIMES];	// split times

	REAL			Current0to15;
	REAL			Best0to15;					// best 0-20mph time
	REAL			Current0to25;
	REAL			Best0to25;					// best 0-20mph time

	unsigned long	CurrentLapStartTime;		// start time for current lap

	REAL			Revs;						// Engine Revs
	REAL			MaxRevs;					// For the tachometer

	REAL			PowerTimer;					// Seconds until power restored (electropulsed!)
	long			AddLit;						// Light to add when rendered
	REAL			DrawScale;					// global draw scale
	long			IsBomb;						// TRUE if car is a bomb
	long			WillDetonate;				// TRUE if car is a bomb and just about to detonate
	REAL			NoReturnTimer;				// timer to stop instant return of bomb

	// Selp-righting stuff
	VEC				DestPos;
	QUATERNION		DestQuat;

	VEC				FieldVec;

	// Collision information stuff
	COLLINFO_WHEEL	*WheelCollHead;
	int				NWheelColls;				// Number of wheel collisions on this car

	int				NWheelsInContact;			// Number of wheels in contact with something
	int				NWheelFloorContacts;		// Number of contacts with the floor (< 60 degrees to horizontal)

#ifdef _PC
#ifndef _CARCONV
	SAMPLE_3D		*SfxEngine;					// engine sfx
	SAMPLE_3D		*SfxScrape;					// scrape sfx
	SAMPLE_3D		*SfxScreech;				// screech sfx
	long			ScrapeMaterial;				// scrape material num

	REMOTE_DATA		RemoteData[3];				// Most recent remote data recieved from the network
	int				OldDat, Dat, NewDat;
#endif
#endif


#ifdef _PSX
	MATRIX			Matrix;
	SVECTOR			Pos, LastPos;
	VEC				AerialPos[5];
	short			Type;
	short			sPad;
#endif 

	bool			AllowedBestTime;
	bool			Selectable;
	bool			Righting;
	bool			RightingCollide;
	bool			RightingReachDest;
	bool			Timing0to15;
	bool			Timing0to25;
	bool			bPad;


} CAR;

#define SetCarHasSpring(car, i) ((car)->Models->WheelPartsFlag[i] |= CAR_MODEL_SPRING)
#define SetCarHasAxle(car, i) ((car)->Models->WheelPartsFlag[i] |= CAR_MODEL_AXLE)
#define SetCarHasPin(car, i) ((car)->Models->WheelPartsFlag[i] |= CAR_MODEL_PIN)
#define CarHasSpring(car, i) ((car)->Models->WheelPartsFlag[i] & CAR_MODEL_SPRING)
#define CarHasAxle(car, i) ((car)->Models->WheelPartsFlag[i] & CAR_MODEL_AXLE)
#define CarHasPin(car, i) ((car)->Models->WheelPartsFlag[i] & CAR_MODEL_PIN)

#define SetCarHasSpinner(car) ((car)->Models->BodyPartsFlag[i] != CAR_MODEL_SPINNER)
#define CarHasSpinner(car) ((car)->Models->BodyPartsFlag & CAR_MODEL_SPINNER)
#define SetCarHasAerial(car) ((car)->Models->BodyPartsFlag[i] != CAR_MODEL_AERIAL)
#define CarHasAerial(car) ((car)->Models->BodyPartsFlag & CAR_MODEL_AERIAL)


//
// External function prototypes
//
struct PlayerStruct;

extern CAR_INFO *CreateCarInfo(long nInfo);
extern void DestroyCarInfo();
extern CAR_MODEL *CreateCarModels(long nModels);
extern void DestroyCarModels(CAR_MODEL *models);


extern void InitCar(CAR *car);
extern void SetupCar(struct PlayerStruct *player, int carType);
extern void FreeCar(struct PlayerStruct *player);
extern void GetCarGrid(long position, VEC *pos, MAT *mat);
extern void SetCarPos(CAR *car, VEC *pos, MAT *mat);
//extern void BuildTurnMatrices(CAR *car);
//extern void BuildCarFromWheels(CAR *car);
//extern void FixWheel(WHEEL *w1, WHEEL *w2, REAL diag);
//extern void CarBuildCollision(CAR *car);
//extern void CarBuildWheelCollision(CAR *car, WHEEL *w);
//extern short CarWheelCollision(WHEEL *w);
//extern void CarMove(CAR *car);
//extern void CarWheelMove(WHEEL *w);
//extern void Car2Car(CAR *MyCar);
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
extern void MoveCarCoM(CAR_INFO *carInfo, VEC *dR);
extern void CarWheelImpulse(CAR *car, COLLINFO_WHEEL *collInfo, VEC *impulse);
extern void CarWheelImpulse2(CAR *car, COLLINFO_WHEEL *collInfo, VEC *impulse);
extern void SetCarAngResistance(CAR *car);
extern void CAR_AllCarColls();
extern REMOTE_DATA *NextRemoteData(CAR *car);
extern REMOTE_DATA *NextRemoteData(CAR *car);
extern void SendLocalCarData(void);
extern void UpdateRemotePlayer(struct PlayerStruct *player);
extern void LoadOneCarModelSet(struct PlayerStruct *player, long car);
extern void FreeOneCarModelSet(struct PlayerStruct *player);
extern int NextValidCarID(int currentID);
extern int PrevValidCarID(int currentID);
extern void CheckCarSituation(CAR *car);
extern void CarAccTimings(CAR *car);
extern void CarDownForce(CAR *car);
extern void RemoveWheelColl(CAR *car, COLLINFO_WHEEL *collInfo);
extern COLLINFO_WHEEL *AddWheelColl(CAR *car, COLLINFO_WHEEL *newHead);





//
// External global variables
//

extern CAR_INFO		*CarInfo;
extern long			NCarTypes;

extern bool CAR_DrawCarBBoxes;
extern bool CAR_DrawCarAxes;
extern bool	CAR_WheelsHaveSuspension;

extern GRID_POS CarGridStarts[][MAX_NUM_PLAYERS];

#endif
