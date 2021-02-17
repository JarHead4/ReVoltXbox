

#ifndef __NEWCAR_H__
#define __NEWCAR_H__

#define NEWCAR_PIECEPRESENT	(0x1)
#define NEWCAR_PICKED_UP	(0x2)

#define NEWCAR_PRMLEVEL		1
#define NEWCAR_NWHEELS		4

#define CAR_SHIFT_MAG	Real(5)

// Structure to hold the info on a piece of a car
typedef struct {
	long	Status;
	int		ModelNumber;
	VECTOR	ModelOffset;
} NEWCAR_PIECEINFO;

// Structure to hold the model and offset info for a car 
typedef struct {
	int		TPage;
	NEWCAR_PIECEINFO	Body;
	NEWCAR_PIECEINFO	Wheel[NEWCAR_NWHEELS];
	NEWCAR_PIECEINFO	Springs[NEWCAR_NWHEELS];
	NEWCAR_PIECEINFO	Pins[NEWCAR_NWHEELS];
	NEWCAR_PIECEINFO	Axles[NEWCAR_NWHEELS];
} NEWCAR_RENDERINFO;


// Storage of each car's physical properties
typedef struct {
	REAL	Mass;
	REAL	Radius;

	REAL	AxleFriction;
	REAL	StaticFriction;
	REAL	KineticFriction;
	REAL	Gravity;
	
	VECTOR	WheelOffset;
} NEWWHEEL_DATA;

typedef struct {
	REAL	Mass;
	MATRIX	Inertia;

	REAL	Hardness;
	REAL	Resistance;
	REAL	StaticFriction;
	REAL	KineticFriction;

	REAL	Gravity;

	VECTOR	BodyOffset;
	NEWWHEEL_DATA	WheelData[NEWCAR_NWHEELS];
} NEWCAR_DATA;


// Structure to hold all the model data which can
// be instanced from within the NEWCAR structure
typedef struct {

	int		NBodies, NWheels, NSprings, NPins, NAxles;
	MODEL	*Bodies;
	MODEL	*Wheels;
	MODEL	*Springs;
	MODEL	*Pins;
	MODEL	*Axles;

} CARSTORE, *PCARSTORE;

typedef struct {
	// In-game car controls
	int	TurnLeft;
	int	TurnRight;
	int	Accelerate;
	int	Decelerate;
	// Put back at home
	int	Reset;
	// Move car freely
	int	Pickup;
	int	Drop;
	int	MoveXPos;
	int	MoveXNeg;
	int	MoveYPos;
	int	MoveYNeg;
	int	MoveZPos;
	int	MoveZNeg;
} NEWCARKEYS;

// Structure to hold all info relevant to a single car
typedef struct {
	long		Status;

	REAL		SteerAngle;			// Angle of steering wheel
	REAL		SteerStep;
	REAL		EngineVolt;			// Power on the engine
	REAL		EngineStep;


	NEWBODY		Body;
	MODEL		*BodyModel;
	VECTOR		BodyOffset;

	AERIAL		Aerial;
	MODEL		*AerialModel[2];
	VECTOR		AerialOffset;

	WHEEL	Wheel[NEWCAR_NWHEELS];
	MODEL		*WheelModel[NEWCAR_NWHEELS];
	VECTOR		WheelOffset[NEWCAR_NWHEELS];

} NEWCAR, *PNEWCAR;

/////////////////////////////////////////////////////////////////////
// Defines
#define IsPiecePresent(piece) ((piece)->Status & NEWCAR_PIECEPRESENT)

#define IsCarPickedUp(pCar) ((pCar)->Status & NEWCAR_PICKED_UP)
#define CarPickup(pCar) ((pCar)->Status |= NEWCAR_PICKED_UP)
#define CarDrop(pCar) ((pCar)->Status &= ~NEWCAR_PICKED_UP)

#define CarTurnLeft(pCar) \
{ \
	(pCar)->SteerAngle -= (pCar)->SteerStep; \
	if ((pCar)->SteerAngle < -ONE) (pCar)->SteerAngle = -ONE; \
}
#define CarTurnRight(pCar) \
{ \
	(pCar)->SteerAngle += (pCar)->SteerStep; \
	if ((pCar)->SteerAngle > ONE) (pCar)->SteerAngle = ONE; \
}

#define CarAccelerate(pCar) \
{ \
	(pCar)->EngineVolt += (pCar)->EngineStep; \
	if ((pCar)->EngineVolt > ONE) (pCar)->EngineVolt = ONE; \
}
#define CarDecelerate(pCar) \
{ \
	(pCar)->EngineVolt -= (pCar)->EngineStep; \
	if ((pCar)->EngineVolt < -ONE) (pCar)->EngineVolt = -ONE; \
}

#define CarTranslate(pCar, trans) \
{ \
	VecPlusEqVec(&(pCar)->Body.Centre.Pos, trans); \
}

#define CarReset(pCar) NULL;

/////////////////////////////////////////////////////////////////////
// external function prototypes
extern NEWCAR *CreateNewCars(unsigned short nCars);
extern void DestroyNewCars(NEWCAR *car);
extern void SetupNewCar(NEWCAR *car, int CarNumber);
extern void SetNewCarPos(NEWCAR *car, VECTOR *pos);

extern void BuildNewCarEyeMatrix(NEWCAR *pCar);
extern void DrawNewCar(NEWCAR *car);

extern CARSTORE *CreateCarStore(int nBodies, 
								int nWheels, 
								int nSprings, 
								int nPins, 
								int nAxles);
extern void DestroyCarStore(CARSTORE *carStore);
extern bool LoadCarStoreModels(CARSTORE *carStore);

extern void NewCarProcessKeys(NEWCAR *pCar, NEWCARKEYS *carKeys);
extern void UpdateNewCars(REAL dt);



/////////////////////////////////////////////////////////////////////
// external variables
extern char PrmLevel;


#endif
