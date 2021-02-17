
#include "ReVolt.h"
#include "Geom.h"
#include "NewColl.h"
#include "Particle.h"
#include "Body.h"
#include "Aerial.h"
#include "Wheel.h"
#ifndef _PSX
#include "Model.h"
#endif
#include "Car.h"
#ifndef _PSX
#include "Main.h"
#include "Level.h"
#endif

#ifdef _PC
#include "Instance.h"
#endif

#include "ctrlread.h"
#include "Object.h"
#include "Field.h"
#include "control.h"
#include "player.h"
#include "spark.h"
#ifdef _PC
#include "ai.h"
#endif


/////////////////////////////////////////////////////////////////////
//
// Prototypes
//

#if defined(_PSX)
COLLSKIN *LoadCollSkin();
#endif

void COL_BodyCollHandler(OBJECT *obj);
void COL_CarCollHandler(OBJECT *obj);

static void COL_AllObjectColls(void);
static void COL_DummyColl(OBJECT *obj);
static bool COL_Dummy2Coll(OBJECT *obj1, OBJECT *obj2);
static void COL_BodyWorldColl(OBJECT *obj);
static void COL_CarWorldColl(OBJECT *obj);
static bool COL_BodyBodyColl(OBJECT *obj1, OBJECT *obj2);
static bool COL_CarBodyColl(OBJECT *obj1, OBJECT *obj2);
static bool COL_BodyCarColl(OBJECT *obj1, OBJECT *obj2);
static bool COL_CarCarColl(OBJECT *obj1, OBJECT *obj2);
static REAL CorrugationAmp(CORRUGATION *cor, REAL dx, REAL dy);

void AdjustBodyColl(COLLINFO_BODY *collInfo, MATERIAL *material);
CONVEX *CreateConvex(INDEX nConvex);
void FreeCollSkin(COLLSKIN *collSkin);
bool SetupConvex(CONVEX *skin, INDEX nPts, INDEX extraPts, INDEX nEdges, INDEX nFaces);
void DestroyConvex(CONVEX *skin, int nSkins);
void DestroySpheres(SPHERE *spheres);
bool CreateCopyCollSkin(COLLSKIN *collSkin);
void BuildWorldSkin(COLLSKIN *bodySkin, VEC *pos, MAT *mat);
#ifdef _PC
CONVEX *LoadConvex(FILE *fp, INDEX *nSkins, int extraPtsPerEdge);
SPHERE *LoadSpheres(FILE *fp, INDEX *nSpheres);
#elif defined(_N64)
CONVEX *LoadConvex(FIL *fp, INDEX *nSkins, int extraPtsPerEdge);
SPHERE *LoadSpheres(FIL *fp, INDEX *nSpheres);
#endif
bool PointInConvex(VEC *pos, CONVEX *skin, PLANE *plane, REAL *minDepth);
PLANE *LineToConvexColl(VEC *sPos, VEC *ePos, CONVEX *skin, REAL *penDepth, REAL *time);
void ModifyShift(VEC *shift, REAL shiftMag, VEC *normal);
bool PointInCollPolyBounds(VEC *pt, NEWCOLLPOLY *poly);
bool SphereInCollPolyBounds(VEC *pt, REAL radius, NEWCOLLPOLY *poly);
bool SphereConvex(VEC *spherePos, REAL sphereRad, CONVEX *skin,VEC *collPos, PLANE *collPlane, REAL *collDepth);
void MakeAxisAlignedBBox(COLLSKIN *collSkin, BBOX *totBBox);
void RotTransBBox(BBOX *bBoxIn, MAT *rotMat, VEC *dR, BBOX *bBoxOut);
void TransBBox(BBOX *bBox, VEC *sPos);
REAL CorrugationAmp(CORRUGATION *cor, REAL dx, REAL dy);
int GetCollPolyVertices(NEWCOLLPOLY *poly, VEC *v0, VEC *v1, VEC *v2, VEC *v3);
void AddPointToBBox(BBOX *bBox, VEC *pos);
void AddPosRadToBBox(BBOX *bBox, VEC *pos, REAL radius);
void RotTransBBox(BBOX *srcBox, MAT *mat, VEC *vec, BBOX *destBox);

bool LineOfSight(VEC *src, VEC *dest);
bool LineOfSightDist(VEC *src, VEC *dest, REAL *minT, PLANE **plane);
#ifdef _PC
bool LineOfSightObj(VEC *src, VEC *dest, REAL *minT);
bool LineOfSightBody(NEWBODY *body, VEC *src, VEC *dest, REAL *minDist);
bool LineOfSightSphere(VEC *sphPos, REAL rad, VEC *src, VEC *dest, REAL *minDist);
bool LineOfSightConvex(NEWBODY *body, VEC *src, VEC *dest, REAL *minDist);
#endif

void RotTransCollPolys(NEWCOLLPOLY *collPoly, int nPolys, MAT *rMat, VEC *dPos);
void TransCollPolys(NEWCOLLPOLY *collPoly, int nPolys, VEC *dPos);
COLLINFO_BODY *NextBodyCollInfo();


int NonPlanarCount = 0;
int QuadCollCount = 0;
int TriCollCount = 0;

/////////////////////////////////////////////////////////////////////
//
// Globals 
//

NEWCOLLPOLY	*COL_WorldCollPoly = NULL;		// Collision skin for world model
INDEX		COL_NWorldCollPolys = 0;
NEWCOLLPOLY	*COL_InstanceCollPoly = NULL;	// Instance skin when placed in world
INDEX		COL_NInstanceCollPolys = 0;
COLLGRID_DATA	COL_CollGridData;		// Gridding information
COLLGRID		*COL_CollGrid;			// Poly pointers and counter for each grid volume
long			COL_NCollGrids;			// Number of grid locations

// Arrays to hold collision info for all objects
COLLINFO_BODY		COL_BodyCollInfo[MAX_COLLS_BODY];
COLLINFO_WHEEL		COL_WheelCollInfo[MAX_COLLS_WHEEL];
int COL_NBodyColls = 0;
int COL_NBodyDone = 0;
int COL_NWheelColls = 0;
int COL_NWheelDone = 0;

// Array of pointers to collisions on currently considered body
COLLINFO_BODY	*COL_ThisBodyColl[MAX_COLLS_PER_BODY];
int				COL_NThisBodyColls = 0;
int				COL_NCollsTested = 0;


// Dummy Variables
PLANE DummyPlane;
REAL DummyReal;


/////////////////////////////////////////////////////////////////////
//
// Array of pointers to functions which give the function which
// detects collisions between two objects of two types
//
static bool (*COL_ObjObjColl[MAX_COLL_TYPES][MAX_COLL_TYPES])(OBJECT *obj1, OBJECT *obj2) = {
			/*	NONE				BODY				CAR	*/
	/*NONE*/	{COL_Dummy2Coll,	COL_Dummy2Coll,		COL_Dummy2Coll, },
	/*BODY*/	{COL_Dummy2Coll,	COL_BodyBodyColl,	COL_BodyCarColl, },
	/*CAR*/		{COL_Dummy2Coll,	COL_CarBodyColl,	COL_CarCarColl, }
};

static void (*COL_ObjWorldColl[MAX_COLL_TYPES])(OBJECT *obj) = {
	/*NONE*/	COL_DummyColl,
	/*BODY*/	COL_BodyWorldColl,
	/*CAR*/		COL_CarWorldColl,
};

/////////////////////////////////////////////////////////////////////
//
// Materials for the collision polys
//
MATERIAL	COL_MaterialInfo[MATERIAL_NTYPES] = {
	{	// MATERIAL_DEFAULT,
		MATERIAL_SKID | MATERIAL_SPARK,			// Properties
		Real(1.0),			// Roughness
		Real(1.0),			// Gripiness
		Real(0.0),			// Hardness
		0x707070L,			// SkidColour
		CORRUG_NONE,		// Corrugation number
		DUST_NONE,			// Dust type (using spark engine)
		{ZERO, ZERO, ZERO}	// Velocity
	},
	{	//MATERIAL_MARBLE,
		MATERIAL_SKID | MATERIAL_SPARK,
		Real(0.8),
		Real(0.9),
		Real(0.2),
		0x707070,
		CORRUG_NONE,
		DUST_NONE,
		{ZERO, ZERO, ZERO}
	},
	{	//MATERIAL_STONE,
		MATERIAL_SKID | MATERIAL_SPARK,
		Real(0.9),
		Real(0.9),
		Real(0.2),
		0x909090,
		CORRUG_NONE,
		DUST_NONE,
		{ZERO, ZERO, ZERO}
	},		
	{//MATERIAL_WOOD,
		MATERIAL_SKID,
		Real(1.0),
		Real(1.0),
		Real(0.0),
		0x404040,
		CORRUG_NONE,
		DUST_NONE,
		{ZERO, ZERO, ZERO}
	},
	{//MATERIAL_SAND,
		MATERIAL_SKID | MATERIAL_DUSTY,
		Real(0.5),
		Real(0.6),
		Real(0.0),
		0x402040,
		CORRUG_NONE,
		DUST_SAND,
		{ZERO, ZERO, ZERO}
	},
	{//MATERIAL_PLASTIC,
		MATERIAL_SKID,
		Real(0.7),
		Real(0.9),
		Real(0.1),
		0x404040,
		CORRUG_NONE,
		DUST_NONE,
		{ZERO, ZERO, ZERO}
	},
	{//MATERIAL_CARPET1,
		MATERIAL_SKID,
		Real(1.0),
		Real(0.7),
		Real(0.0),
		0x202020,
		CORRUG_NONE,
		DUST_NONE,
		{ZERO, ZERO, ZERO}
	},
	{//MATERIAL_CARPET2,
		MATERIAL_SKID,
		Real(1.0),
		Real(0.5),
		Real(0.0),
		0x202020,
		CORRUG_NONE,
		DUST_NONE,
		{ZERO, ZERO, ZERO}
	},
	{//MATERIAL_BOUNDARY,
		MATERIAL_OUTOFBOUNDS,
		Real(1.0),
		Real(1.0),
		Real(0.0),
		0x0,
		CORRUG_NONE,
		DUST_NONE,
		{ZERO, ZERO, ZERO}
	},
	{ //MATERIAL_GLASS,
		MATERIAL_SKID | MATERIAL_SPARK,
		Real(0.7),
		Real(0.8),
		Real(0.2),
		0x303030,
		CORRUG_NONE,
		DUST_NONE,
		{ZERO, ZERO, ZERO}
	},
	{ //MATERIAL_ICE1,
		MATERIAL_SKID | MATERIAL_SPARK,
		Real(0.3),
		Real(0.3),
		Real(0.2),
		0x303030,
		CORRUG_NONE,
		DUST_NONE,
		{ZERO, ZERO, ZERO}
	},
	{ //MATERIAL_METAL,
		MATERIAL_SKID | MATERIAL_SPARK,
		Real(0.8),
		Real(0.8),
		Real(0.4),
		0x505050,
		CORRUG_NONE,
		DUST_NONE,
		{ZERO, ZERO, ZERO}
	}, 
	{ //MATERIAL_GRASS,
		MATERIAL_SKID | MATERIAL_SPARK | MATERIAL_CORRUGATED | MATERIAL_DUSTY,
		Real(0.7),
		Real(0.5),
		Real(0.0),
		0x207010,
		CORRUG_STEEL,
		DUST_MUD,
		{ZERO, ZERO, ZERO}
	},
	{ //MATERIAL_BUMPMETAL,
		MATERIAL_SKID | MATERIAL_SPARK | MATERIAL_CORRUGATED,
		Real(0.8),
		Real(0.8),
		Real(0.4),
		0x505050,
		CORRUG_STEEL,
		DUST_NONE,
		{ZERO, ZERO, ZERO}
	},
	{ //MATERIAL_PEBBLES
		MATERIAL_SKID | MATERIAL_SPARK | MATERIAL_CORRUGATED | MATERIAL_DUSTY,
		Real(0.7),
		Real(0.7),
		Real(0.2),
		0x303030,
		CORRUG_PEBBLES,
		DUST_GRAVEL,
		{ZERO, ZERO, ZERO}
	},
	{ //MATERIAL_GRAVEL,
		MATERIAL_SKID | MATERIAL_SPARK | MATERIAL_CORRUGATED | MATERIAL_DUSTY,
		Real(0.9),
		Real(0.6),
		Real(0.2),
		0x303030,
		CORRUG_GRAVEL,
		DUST_GRAVEL,
		{ZERO, ZERO, ZERO}
	},
	{ //MATERIAL_CONVEYOR1,
		MATERIAL_MOVES | MATERIAL_CORRUGATED,
		Real(1.0),
		Real(1.0),
		Real(0.0),
		0x000000,
		CORRUG_STEEL,
		DUST_NONE,
		{-5 * 57.476f, -5 * 25.749f, 5 * 77.676f}
	},
	{ //MATERIAL_CONVEYOR2,
		MATERIAL_MOVES | MATERIAL_CORRUGATED,
		Real(1.0),
		Real(1.0),
		Real(0.0),
		0x000000,
		CORRUG_STEEL,
		DUST_NONE,
		{5 * 57.476f, 5 * 25.749f, -5 * 77.676f}
	},
	{ //MATERIAL_DIRT1,
		MATERIAL_SKID | MATERIAL_CORRUGATED | MATERIAL_DUSTY,
		Real(0.9),
		Real(0.6),
		Real(0.2),
		0x303030,
		CORRUG_DIRT1,
		DUST_DIRT,
		{ZERO, ZERO, ZERO}
	},
	{ //MATERIAL_DIRT2,
		MATERIAL_SKID | MATERIAL_CORRUGATED | MATERIAL_DUSTY,
		Real(0.9),
		Real(0.6),
		Real(0.2),
		0x303030,
		CORRUG_DIRT2,
		DUST_DIRT,
		{ZERO, ZERO, ZERO}
	},
	{ //MATERIAL_DIRT3,
		MATERIAL_SKID | MATERIAL_CORRUGATED | MATERIAL_DUSTY,
		Real(0.9),
		Real(0.6),
		Real(0.2),
		0x303030,
		CORRUG_DIRT3,
		DUST_DIRT,
		{ZERO, ZERO, ZERO}
	},
	{ //MATERIAL_ICE2,
		MATERIAL_SKID | MATERIAL_SPARK,
		Real(0.45),
		Real(0.45),
		Real(0.2),
		0x303030,
		CORRUG_NONE,
		DUST_NONE,
		{ZERO, ZERO, ZERO}
	},
	{ //MATERIAL_ICE3,
		MATERIAL_SKID | MATERIAL_SPARK,
		Real(0.6),
		Real(0.6),
		Real(0.2),
		0x303030,
		CORRUG_NONE,
		DUST_NONE,
		{ZERO, ZERO, ZERO}
	},

};


CORRUGATION COL_CorrugationInfo[CORRUG_NTYPES] = {
	{ //CORRUG_NONE,
		ZERO,
		ZERO,
		ZERO
	},
	{ //CORRUG_PEBBLES,
		Real(4.0),
		Real(70.0),
		Real(70.0)
	},
	{ //CORRUG_GRAVEL,
		Real(2.5),
		Real(40.0),
		Real(40.0)
	},
	{ //CORRUG_STEEL,
		Real(1.0),
		Real(40.0),
		Real(40.0)
	},
	{ //CORRUG_CONVEYOR,
		Real(1.0),
		Real(80.0),
		Real(80.0)
	},
	{ //CORRUG_DIRT1,
		Real(1.0),
		Real(40.0),
		Real(40.0)
	},
	{ //CORRUG_DIRT2,
		Real(1.5),
		Real(80.0),
		Real(80.0)
	},
	{ //CORRUG_DIRT3,
		Real(2.0),
		Real(100.0),
		Real(100.0)
	},
};

DUST COL_DustInfo[DUST_NTYPES] = {
	{ // DUST_NONE
		SPARK_NONE,
		ZERO,
		ZERO
	},
	{ // DUST_GRAVEL
		SPARK_GRAVEL,		// Type of particle
		Real(0.2),			// Probability of particle generation
		Real(0.6)			// Randomisation factor
	},
	{ // DUST_SAND
		SPARK_SAND,		// Type of particle
		Real(0.2),			// Probability of particle generation
		Real(0.6)			// Randomisation factor
	},
	{ // DUST_MUD
		SPARK_MUD,		// Type of particle
		Real(0.2),			// Probability of particle generation
		Real(0.6)			// Randomisation factor
	},
	{ // DUST_DIRT
		SPARK_DIRT,
		Real(0.6),
		Real(0.8)
	}
};

/////////////////////////////////////////////////////////////////////
//
// AdjustBodyColl: adjust collision info according to the material 
// type which has been collided with.
//
/////////////////////////////////////////////////////////////////////

void AdjustBodyColl(COLLINFO_BODY *collInfo, MATERIAL *material)
{
	if (material == NULL) return;

	if (MaterialMoves(material)) {
		VecMinusEqVec(&collInfo->Vel, &material->Vel);
	}
}

/////////////////////////////////////////////////////////////////////
//
// AdjustBodyColl: adjust collision info according to the material 
// type which has been collided with.
//
/////////////////////////////////////////////////////////////////////

void AdjustWheelColl(COLLINFO_WHEEL *wheelColl, MATERIAL *material)
{
	if (material == NULL) return;

	// Is material a conveyor belt?
	if (MaterialMoves(material)) {
		VecMinusEqVec(&wheelColl->Vel, &material->Vel);
	}

	// Is material corrugated?
	if (MaterialCorrugated(material)) {
		wheelColl->Depth += CorrugationAmp(&COL_CorrugationInfo[material->Corrugation], wheelColl->WorldPos.v[X], wheelColl->WorldPos.v[Z]);
	}
}

/////////////////////////////////////////////////////////////////////
// COL_DoObjectCollisions: call the collision detection and response
// functions for all objects in the game
/////////////////////////////////////////////////////////////////////

void COL_DoObjectCollisions(void)
{
	OBJECT	*obj;

	COL_AllObjectColls();

	//CAR_AllCarColls();

	for (obj = OBJ_ObjectHead; obj; obj = obj->next)
	{
		if (obj->collhandler)
		{
			obj->collhandler(obj);
		}
	}
	
}


/////////////////////////////////////////////////////////////////////
// COL_AllObjectColls:
/////////////////////////////////////////////////////////////////////

void COL_AllObjectColls(void)
{
	OBJECT *obj1, *obj2;

	// initialisation
	COL_NBodyColls = 0;
	COL_NBodyDone = 0;
	COL_NWheelColls = 0;
	COL_NWheelDone = 0;
	ClearActivePairInfo();
	for (obj1 = OBJ_ObjectHead; obj1 != NULL; obj1 = obj1->next) {
		obj1->body.NBodyColls = 0;
		obj1->body.BodyCollHead = NULL;
		if (obj1->CollType == COLL_TYPE_CAR) {
			obj1->player->car.NWheelColls = 0;
			obj1->player->car.WheelCollHead = NULL;
		}
	}
	
	// loop over all object pairs
	for (obj1 = OBJ_ObjectHead; obj1 != NULL; obj1 = obj1->next) {

		Assert(obj1->CollType >= 0 && obj1->CollType < MAX_COLL_TYPES);

		// object-world collision if allowed for this object
		if (obj1->body.CollSkin.AllowWorldColls) {
			COL_ObjWorldColl[obj1->CollType](obj1);
		}

		// Make sure this object is allowed to collide with objects
		if (!obj1->body.CollSkin.AllowObjColls) continue;

		for (obj2 = obj1->next; obj2 != NULL; obj2 = obj2->next) {
		
			Assert(obj2->CollType >= 0 && obj2->CollType < MAX_COLL_TYPES);

			// Make sure this object is allowed to collide with objects
			if (!obj2->body.CollSkin.AllowObjColls) continue;

			// Do not test the same object pair twice
			if (IsPairTested(obj1, obj2)) continue;
			SetPairTested(obj1, obj2);

			// Do the actual collision check
			if (COL_ObjObjColl[obj1->CollType][obj2->CollType](obj1, obj2)) {
				SetPairCollided(obj1, obj2);
			}
		}
	}
}

// No collision detection
void COL_DummyColl(OBJECT *obj)
{
}

// Body-world collision detection
void COL_BodyWorldColl(OBJECT *obj)
{
	Assert(obj->CollType == COLL_TYPE_BODY);
	DetectBodyWorldColls(&obj->body);
}

// Car-world collision detection
void COL_CarWorldColl(OBJECT *obj)
{
	Assert(obj->CollType == COLL_TYPE_CAR);
	DetectCarWorldColls(&obj->player->car);
}

// body-body collision detection
bool COL_BodyBodyColl(OBJECT *obj1, OBJECT *obj2)
{
	Assert(obj1->CollType == COLL_TYPE_BODY && obj2->CollType == COLL_TYPE_BODY);
	if (DetectBodyBodyColls(&obj1->body, &obj2->body) > 0) {
		return TRUE;
	}
	return FALSE;

}

// body-car collision detection
bool COL_BodyCarColl(OBJECT *obj1, OBJECT *obj2)
{
	Assert(obj1->CollType == COLL_TYPE_BODY && obj2->CollType == COLL_TYPE_CAR);
	if (DetectCarBodyColls(&obj2->player->car, &obj1->body) > 0) {
		return TRUE;
	}
	return FALSE;
}

// car-body collision detection
bool COL_CarBodyColl(OBJECT *obj1, OBJECT *obj2)
{
	Assert(obj1->CollType == COLL_TYPE_CAR && obj2->CollType == COLL_TYPE_BODY);
	if (DetectCarBodyColls(&obj1->player->car, &obj2->body) > 0) {
		return TRUE;
	}
	return FALSE;
}

// car-car collision detection
bool COL_CarCarColl(OBJECT *obj1, OBJECT *obj2)
{
	Assert(obj1->CollType == COLL_TYPE_CAR && obj2->CollType == COLL_TYPE_CAR);
	if (DetectCarCarColls(&obj1->player->car, &obj2->player->car) > 0) {
		return TRUE;
	}
	return FALSE;
}

// no collision detection
bool COL_Dummy2Coll(OBJECT *obj1, OBJECT *obj2)
{
	return FALSE;
}

/////////////////////////////////////////////////////////////////////
// COL_DummyCollHandler: a do-nothing default collision handler
/////////////////////////////////////////////////////////////////////

void COL_DummyCollHandler(OBJECT *obj)
{
	// Errr.... no collisions probably
}


/////////////////////////////////////////////////////////////////////
// COL_BodyCollHandler: deal with a body's collisions
/////////////////////////////////////////////////////////////////////

void COL_BodyCollHandler(OBJECT *obj)
{
	VEC imp, angImp;
	NEWBODY *body = &obj->body;
	FIELD_DATA fieldData;

	// Reset necessary variables
	body->NWorldContacts = 0;
	body->NOtherContacts = 0;
	if (body->LastScrapeTime > MAX_SCRAPE_TIME) {
		body->ScrapeMaterial = MATERIAL_NONE;
		body->LastScrapeTime = ZERO;
	} else {
		body->LastScrapeTime += TimeStep;
	}

	// Apply Force fields
	fieldData.ObjectID = obj->ObjID;
	fieldData.Priority = obj->FieldPriority;
	fieldData.Mass = body->Centre.Mass;
	fieldData.Pos = &body->Centre.Pos;
	fieldData.Vel = &body->Centre.Vel;
	fieldData.Mat = &body->Centre.WMatrix;
	fieldData.AngVel = &body->AngVel;
	fieldData.Quat = &body->Centre.Quat;
	AllFieldImpulses(&fieldData, &imp, &angImp);
	VecPlusEqVec(&body->Centre.Impulse, &imp);
	VecPlusEqVec(&body->AngImpulse, &angImp);


	// Apply Turbo boost
	BodyTurboBoost(body);
	
	// Process collisions
	if (obj->body.NBodyColls > 0) {
		PreProcessBodyColls(body);
		if (obj->body.NBodyColls > 0) {
			ProcessBodyColls3(body);
			PostProcessBodyColls(body);
		}
	}

}

/////////////////////////////////////////////////////////////////////
// COL_CarCollHandler: deal with all the collision for a car
/////////////////////////////////////////////////////////////////////
void COL_CarCollHandler(OBJECT *obj)
{
	CAR *car = &obj->player->car;

	// Reinitialise necessary stuff
	car->NWheelFloorContacts = 0;
	car->NWheelsInContact = 0;

	// Process wheel collisions
	if (obj->player->car.NWheelColls > 0) {
		PreProcessCarWheelColls(car);
		ProcessCarWheelColls(car);
//#ifndef _PSX
		PostProcessCarWheelColls(car);
//#endif
	}

	// Add aerodynamic downforce
	CarDownForce(car);

	// Add speedup force
#ifdef _PC
	SpeedupImpulse(car);
#endif

	// Process body collisions
	COL_BodyCollHandler(obj);

	// Adjust air resistance if no wheels in contact with floor
	SetCarAngResistance(car);
}


/////////////////////////////////////////////////////////////////////
//
// CreateNewCollPolys: allocate space for the given number of polys
// DestroyCollPolys: deallocate the space for the collision polys
//
/////////////////////////////////////////////////////////////////////

NEWCOLLPOLY *CreateCollPolys(short nPolys)
{
	return ((NEWCOLLPOLY *)malloc(sizeof(NEWCOLLPOLY) * nPolys));
}

void DestroyCollPolys(NEWCOLLPOLY *polys)
{
	free(polys);
}


/////////////////////////////////////////////////////////////////////
//
// CreateCollGrids: allocate space for and initialise collision grid
// DestroyCollGrids: 
//
/////////////////////////////////////////////////////////////////////

COLLGRID *CreateCollGrids(long nGrids)
{
	int iGrid;
	COLLGRID *newGrids;

	newGrids = (COLLGRID *)malloc(sizeof(COLLGRID) * nGrids);

	// initialise the grids
	if (newGrids != NULL) {
		for (iGrid = 0; iGrid < nGrids; iGrid++) {
			newGrids[iGrid].NCollPolys = 0;
#ifndef _PSX
			newGrids[iGrid].CollPolyPtr = NULL;
#else
			newGrids[iGrid].CollPolyIndices = NULL;
#endif
		}
	}

	return newGrids;
}

void DestroyCollGrids()
{
	int iGrid;

	if (COL_CollGrid == NULL) return;

	for (iGrid = 0; iGrid < COL_NCollGrids; iGrid++) {
#ifndef _PSX
		free(COL_CollGrid[iGrid].CollPolyPtr);
#else
		free(COL_CollGrid[iGrid].CollPolyIndices);
#endif
	}

	free(COL_CollGrid);
	COL_NCollGrids = 0;

}

#ifndef _PSX
NEWCOLLPOLY **CreateCollPolyPtrs(int nPtrs)
{
	return (NEWCOLLPOLY **)malloc(nPtrs * sizeof(NEWCOLLPOLY *));
}
#else 
short *CreateCollPolyIndices(int nPolys)
{
	return (short *)malloc(nPolys * sizeof(short));
}
#endif

/////////////////////////////////////////////////////////////////////
//
// PositionToGrid: return the grid array index corresponding to 
// the passed position
//
/////////////////////////////////////////////////////////////////////

long PosToCollGridCoords(VEC *pos, long *offsetX, long *offsetZ)
{
	REAL	dX, dZ;

	// Make sure the world is gridded (if not, world is one large grid)
	if ((COL_CollGridData.XNum == ZERO) && (COL_CollGridData.ZNum == ZERO)) {
		return 0;
	}

	// Calculate grid index
	dX = pos->v[X] - COL_CollGridData.XStart;
	dZ = pos->v[Z] - COL_CollGridData.ZStart;
	*offsetX = Int(DivScalar(dX, COL_CollGridData.GridSize));
	*offsetZ = Int(DivScalar(dZ, COL_CollGridData.GridSize));

	// Make sure we are not outside the grid boundaries
	if ((*offsetX < 0L) || (*offsetX >= COL_CollGridData.XNum) ||
		(*offsetZ < 0L) || (*offsetZ >= COL_CollGridData.ZNum))
	{
		return -1;
	}

	return *offsetX + NearestInt(COL_CollGridData.XNum) * *offsetZ;
}


long PosToCollGridNum(VEC *pos)
{
	REAL	dX, dZ;
	long	offsetX, offsetZ;

	// Make sure the world is gridded (if not, world is one large grid)
	if ((COL_CollGridData.XNum == ZERO) && (COL_CollGridData.ZNum == ZERO)) {
		return 0;
	}

	// Calculate grid index
	dX = pos->v[X] - COL_CollGridData.XStart;
	dZ = pos->v[Z] - COL_CollGridData.ZStart;
	offsetX = Int(DivScalar(dX, COL_CollGridData.GridSize));
	offsetZ = Int(DivScalar(dZ, COL_CollGridData.GridSize));

	// Make sure we are not outside the grid boundaries
	if ((offsetX < 0L) || (offsetX >= COL_CollGridData.XNum) ||
		(offsetZ < 0L) || (offsetZ >= COL_CollGridData.ZNum))
	{
		return -1;
	}

	return offsetX + NearestInt(COL_CollGridData.XNum) * offsetZ;
}

COLLGRID *PosToCollGrid(VEC *pos)
{
	int gridNum;

	gridNum = PosToCollGridNum(pos);
	if (gridNum < 0 ) {
		return NULL;
	} else {
		return &COL_CollGrid[gridNum];
	}
}


/////////////////////////////////////////////////////////////////////
//
// LoadNewCollPolys: load in a collision mesh, return the number
// of points loaded
//
/////////////////////////////////////////////////////////////////////
#ifndef _N64
NEWCOLLPOLY *LoadNewCollPolys(FILE *fp, short *nPolys)
{
	NEWCOLLPOLYHDR header;
	NEWCOLLPOLY * polys;
	size_t nRead;
	short iPoly;

	// read the header
	nRead = fread(&header, sizeof(NEWCOLLPOLYHDR), 1, fp);
	if (nRead < 1) {
		return NULL;
	}

	// Allocate space for the polys
	if ((polys = CreateCollPolys(header.NPolys)) == NULL) {
		return NULL;
	}

	// Load in the poly info
	NonPlanarCount = 0;
	QuadCollCount = 0;
	TriCollCount = 0;
	for (iPoly = 0; iPoly < header.NPolys; iPoly++) {
		nRead = fread(&polys[iPoly], sizeof(NEWCOLLPOLY), 1, fp);
		if (nRead < 1) {
			*nPolys = iPoly;
			return polys;
		}
		
		// Expand the polys bounding box by the collision skin thickness
		ExpandBBox(&polys[iPoly].BBox, COLL_EPSILON);

#if USE_DEBUG_ROUTINES
		// Make sure material type is valid (Debug)
		Assert((polys[iPoly].Material < MATERIAL_NTYPES) && (polys[iPoly].Material >= 0));
		if (polys[iPoly].Material >= MATERIAL_NTYPES || polys[iPoly].Material < 0) {
			polys[iPoly].Material = MATERIAL_DEFAULT;
		}

#endif

		// Count tris and quads
		if (IsPolyQuad(&polys[iPoly])) {
			QuadCollCount++;
		} else {
			TriCollCount++;
		}


	}

	//wsprintf(buf, "Quads: %d\nTris:  %d", QuadCollCount, TriCollCount);
	//Box("Oy", buf, MB_OK);

	// Success!
	*nPolys = header.NPolys;

	
	return polys;

}


bool LoadGridInfo(FILE *fp)
{
#ifndef _PSX
	long iPoly, iGrid, iInst, index;
	int instListSize, nextWorldPoly;
	long nInstPolys;
#else
	short iPoly, iGrid, iInst, index;
	short instListSize, nextWorldPoly;
	short nInstPolys;
#endif
	INSTANCE *instance;
	int xCount;
	REAL x1, z1;
	BBOX bBox;
#ifndef _PSX
	int instList[MAX_INSTANCES];
#endif


	// Read grid header
	if (fread(&COL_CollGridData, sizeof(COLLGRID_DATA), 1, fp) < 1) {

		// No grid data, so set up one grid system
		COL_CollGridData.XStart = ZERO;
		COL_CollGridData.ZStart = ZERO;
		COL_CollGridData.XNum = ZERO;
		COL_CollGridData.ZNum = ZERO;
		COL_CollGridData.GridSize = LARGEDIST;

		COL_NCollGrids = 1;
		if ((COL_CollGrid = CreateCollGrids(COL_NCollGrids)) == NULL) {
			DestroyCollPolys(COL_WorldCollPoly);
			COL_WorldCollPoly = NULL;
			COL_NWorldCollPolys = 0;
			return FALSE;
		}
		COL_CollGrid[0].NCollPolys = COL_NWorldCollPolys;
		COL_CollGrid[0].NWorldPolys = COL_CollGrid[0].NCollPolys;
#ifndef _PSX
		COL_CollGrid[0].CollPolyPtr = CreateCollPolyPtrs(COL_CollGrid[0].NCollPolys);
		if (COL_CollGrid[0].CollPolyPtr == NULL) {
#else
		COL_CollGrid[0].CollPolyIndices = CreateCollPolyIndices(COL_CollGrid[0].NCollPolys);
		if (COL_CollGrid[0].CollPolyIndices == NULL) {
#endif
			DestroyCollGrids();
			DestroyCollPolys(COL_WorldCollPoly);
			COL_WorldCollPoly = NULL;
			COL_NCollGrids = 0;
			COL_NWorldCollPolys = 0;
			return FALSE;
		}
		for (iPoly = 0; iPoly < COL_NWorldCollPolys; iPoly++) {
#ifndef _PSX
			COL_CollGrid[0].CollPolyPtr[iPoly] = &COL_WorldCollPoly[iPoly];
#else
			COL_CollGrid[0].CollPolyIndices[iPoly] = iPoly;
#endif
		}

		return FALSE;
	}
	

	// Grid data exists, so read it all in
	COL_NCollGrids = NearestInt(COL_CollGridData.XNum) * NearestInt(COL_CollGridData.ZNum);
	if ((COL_CollGrid = CreateCollGrids(COL_NCollGrids)) == NULL) {
		DestroyCollPolys(COL_WorldCollPoly);
		COL_WorldCollPoly = NULL;
		COL_NCollGrids = 0;
		COL_NWorldCollPolys = 0;
		return FALSE;
	}

	x1 = COL_CollGridData.XStart;
	z1 = COL_CollGridData.ZStart;
	xCount = 0;
	// Read in pointer list for each grid location
	for (iGrid = 0; iGrid < COL_NCollGrids; iGrid++) {

		// Get number of polys in this grid volume
		if (fread(&COL_CollGrid[iGrid].NCollPolys, sizeof(COL_CollGrid[iGrid].NCollPolys), 1, fp) < 1) {
			DestroyCollGrids();
			DestroyCollPolys(COL_WorldCollPoly);
			COL_WorldCollPoly = NULL;
			COL_NCollGrids = 0;
			COL_NWorldCollPolys = 0;
			return FALSE;
		}

		// See if any of the instances fall within this grid location and keep note of which ones
		instListSize = 0;
		nInstPolys = 0;
#ifndef _PSX
		SetBBox(&bBox, x1, x1 + COL_CollGridData.GridSize, -LARGEDIST, LARGEDIST, z1, z1 + COL_CollGridData.GridSize);
		for (iInst = 0; iInst < InstanceNum; iInst++) {
			if (BBTestXZY(&bBox, (BBOX *)&Instances[iInst].Box)) {
				instList[instListSize++] = iInst;
				nInstPolys += Instances[iInst].NCollPolys;
			}
		}
		(++xCount) %= NearestInt(COL_CollGridData.XNum);
		if (xCount == 0) {
			x1 = COL_CollGridData.XStart;
			z1 += COL_CollGridData.GridSize;
		} else {
			x1 += COL_CollGridData.GridSize;
		}
#endif

		// Allocate space for the pointers if necessary
		if (COL_CollGrid[iGrid].NCollPolys + nInstPolys == 0) {
#ifndef _PSX
			COL_CollGrid[iGrid].CollPolyPtr = NULL;
#else
			COL_CollGrid[iGrid].CollPolyIndices = NULL;
#endif
			continue;
		} else {
#ifndef _PSX
			COL_CollGrid[iGrid].CollPolyPtr = CreateCollPolyPtrs(COL_CollGrid[iGrid].NCollPolys + nInstPolys);
			if (COL_CollGrid[iGrid].CollPolyPtr == NULL) {
#else
			COL_CollGrid[iGrid].CollPolyIndices = CreateCollPolyIndices(COL_CollGrid[iGrid].NCollPolys);
			if (COL_CollGrid[iGrid].CollPolyIndices == NULL) {
#endif
				DestroyCollGrids();
				DestroyCollPolys(COL_WorldCollPoly);
				COL_WorldCollPoly = NULL;
				COL_NCollGrids = 0;
				COL_NWorldCollPolys = 0;
				return FALSE;
			}
		}
		
		// Fill the pointer array with pointers to polys in grid volume
		for (iPoly = 0; iPoly < COL_CollGrid[iGrid].NCollPolys; iPoly++) {
			if (fread(&index, sizeof(index), 1, fp) < 1) {
				DestroyCollGrids();
				DestroyCollPolys(COL_WorldCollPoly);
				COL_WorldCollPoly = NULL;
				COL_NCollGrids = 0;
				COL_NWorldCollPolys = 0;
				return FALSE;
			}
#ifndef _PSX
			COL_CollGrid[iGrid].CollPolyPtr[iPoly] = &COL_WorldCollPoly[index];
#else
			COL_CollGrid[iGrid].CollPolyIndices[iPoly] = index;
#endif
		}

#ifndef _PSX
		// Now add the instance polys at the end
		nextWorldPoly = COL_CollGrid[iGrid].NCollPolys;
		for (iInst = 0; iInst < instListSize; iInst++) {
			instance = &Instances[instList[iInst]];
			for (iPoly = 0; iPoly < instance->NCollPolys; iPoly++) {
				COL_CollGrid[iGrid].CollPolyPtr[nextWorldPoly++] = instance->CollPoly + iPoly;
			}
		}
		COL_CollGrid[iGrid].NWorldPolys = COL_CollGrid[iGrid].NCollPolys;
		COL_CollGrid[iGrid].NCollPolys += nInstPolys;
#else
		COL_CollGrid[iGrid].NWorldPolys = COL_CollGrid[iGrid].NCollPolys;
#endif

	}

	return TRUE;
}
#else

#ifdef _N64
NEWCOLLPOLY *LoadNewCollPolys(FIL *Fil, short *nPolys)
{
	NEWCOLLPOLYHDR header;
	NEWCOLLPOLY * polys;
	size_t nRead;
	long	ii;

	printf("Loading new collision data...\n");

	nRead = FFS_Read((char *)&header, sizeof(NEWCOLLPOLYHDR), Fil);		// read the header
	if (nRead < sizeof(NEWCOLLPOLYHDR))
	{
		return NULL;
	}
	printf("...number of collison polys is %d\n", header.NPolys);

	// Allocate space for the polys
	if ((polys = CreateCollPolys(header.NPolys)) == NULL)
	{
		return NULL;
	}

	// Load in the poly info
	nRead = FFS_Read((char *)polys, sizeof(NEWCOLLPOLY) * header.NPolys, Fil);
	if (nRead < (sizeof(NEWCOLLPOLY) * header.NPolys))
	{
		DestroyCollPolys(polys);
		return NULL;
	}
	printf("...collison polys use %d bytes\n", sizeof(NEWCOLLPOLY) * header.NPolys);

	// Check polys for valid surface types
	for (ii = 0; ii < header.NPolys; ii++)
	{
		// Expand the polys bounding box by the collision skin thickness
		ExpandBBox(&polys[ii].BBox, COLL_EPSILON);

		// Make sure material type is valid (Debug)
#if USE_DEBUG_ROUTINES
		Assert((polys[ii].Material < MATERIAL_NTYPES) && (polys[ii].Material >= 0));
		if (polys[ii].Material >= MATERIAL_NTYPES || polys[ii].Material < 0) {
			polys[ii].Material = MATERIAL_DEFAULT;
		}
#endif
	}

	// Success!
	*nPolys = header.NPolys;
	return polys;
}


bool LoadGridInfo(FIL *Fil)
{
	int iPoly, iGrid, iInst, index;
#ifdef 0									//!MT! TEMPOUT until instances added
	int instList[MAX_INSTANCES];
	INSTANCE *instance;
#endif
	int instListSize, nextWorldPoly;
	long nInstPolys;
	int xCount;
	REAL x1, z1;
	BBOX bBox;

	printf("Load collison data grids...\n");
	// Read grid header
	if (FFS_Read((char *)&COL_CollGridData, sizeof(COLLGRID_DATA), Fil) < sizeof(COLLGRID_DATA))
	{
		// No grid data, so set up one grid system
		printf("...No grid data found, creating grid system.\n");
		COL_CollGridData.XStart = ZERO;
		COL_CollGridData.ZStart = ZERO;
		COL_CollGridData.XNum = ZERO;
		COL_CollGridData.ZNum = ZERO;
		COL_CollGridData.GridSize = LARGEDIST;

		COL_NCollGrids = 1;
		if ((COL_CollGrid = CreateCollGrids(COL_NCollGrids)) == NULL) 
		{
			printf("...collison grids could not be created!\n");
			DestroyCollPolys(COL_WorldCollPoly);
			COL_NWorldCollPolys = 0;
			return FALSE;
		}
		COL_CollGrid[0].NCollPolys = COL_NWorldCollPolys;
		COL_CollGrid[0].CollPolyPtr = CreateCollPolyPtrs(COL_CollGrid[0].NCollPolys);
		if (COL_CollGrid[0].CollPolyPtr == NULL)
		{
			printf("...collison poly ptrs could not be created!\n");
			DestroyCollGrids();
			DestroyCollPolys(COL_WorldCollPoly);
			COL_NCollGrids = 0;
			COL_NWorldCollPolys = 0;
			return FALSE;
		}
		for (iPoly = 0; iPoly < COL_NWorldCollPolys; iPoly++)
		{
			COL_CollGrid[0].CollPolyPtr[iPoly] = &COL_WorldCollPoly[iPoly];
		}
		return FALSE;
	}

	// Grid data exists, so read it all in
	COL_NCollGrids = NearestInt(COL_CollGridData.XNum) * NearestInt(COL_CollGridData.ZNum);
	printf("...found %d collison grids (%1.0f * %1.0f).\n", COL_NCollGrids, COL_CollGridData.XNum, COL_CollGridData.ZNum);
	if ((COL_CollGrid = CreateCollGrids(COL_NCollGrids)) == NULL)
	{
		printf("...collison grids could not be created!\n");
		DestroyCollPolys(COL_WorldCollPoly);
		COL_NCollGrids = 0;
		COL_NWorldCollPolys = 0;
		return FALSE;
	}

	x1 = COL_CollGridData.XStart;
	z1 = COL_CollGridData.ZStart;
	xCount = 0;
	// Read in pointer list for each grid location
	for (iGrid = 0; iGrid < COL_NCollGrids; iGrid++) {

		// Get number of polys in this grid volume
		if (FFS_Read((char *)&COL_CollGrid[iGrid].NCollPolys, sizeof(long), Fil) < 1)
		{
			DestroyCollGrids();
			DestroyCollPolys(COL_WorldCollPoly);
			COL_NCollGrids = 0;
			COL_NWorldCollPolys = 0;
			return FALSE;
		}

		// See if any of the instances fall within this grid location and keep note of which ones
		instListSize = 0;
		nInstPolys = 0;
#if 0 //!MT! No instance stuff at the moment
		SetBBox(&bBox, x1, x1 + COL_CollGridData.GridSize, -LARGEDIST, LARGEDIST, z1, z1 + COL_CollGridData.GridSize);
		for (iInst = 0; iInst < InstanceNum; iInst++)
		{
			if (BBTestXZY(&bBox, (BBOX *)&Instances[iInst].Box))
			{
				instList[instListSize++] = iInst;
				nInstPolys += Instances[iInst].NCollPolys;
			}
		}
		(++xCount) %= NearestInt(COL_CollGridData.XNum);
		if (xCount == 0)
		{
			x1 = COL_CollGridData.XStart;
			z1 += COL_CollGridData.GridSize;
		}
		else
		{
			x1 += COL_CollGridData.GridSize;
		}
#endif
		// Allocate space for the pointers if necessary
		if (COL_CollGrid[iGrid].NCollPolys + nInstPolys == 0)
		{
			COL_CollGrid[iGrid].CollPolyPtr = NULL;
			continue;
		}
		else
		{
			COL_CollGrid[iGrid].CollPolyPtr = CreateCollPolyPtrs(COL_CollGrid[iGrid].NCollPolys + nInstPolys);
			if (COL_CollGrid[iGrid].CollPolyPtr == NULL)
			{
				printf("...collison poly ptrs could not be created!\n");
				DestroyCollGrids();
				DestroyCollPolys(COL_WorldCollPoly);
				COL_NCollGrids = 0;
				COL_NWorldCollPolys = 0;
				return FALSE;
			}
		}
		
		// Fill the pointer array with pointers to polys in grid volume
		for (iPoly = 0; iPoly < COL_CollGrid[iGrid].NCollPolys; iPoly++)
		{
			if (FFS_Read(&index, sizeof(long), Fil) < 1)
			{
				printf("...failed to read from collsion file.\n");
				DestroyCollGrids();
				DestroyCollPolys(COL_WorldCollPoly);
				COL_NCollGrids = 0;
				COL_NWorldCollPolys = 0;
				return FALSE;
			}
			COL_CollGrid[iGrid].CollPolyPtr[iPoly] = &COL_WorldCollPoly[index];
		}

#if 0 // !MT! no instancing yet
		// Now add the instance polys at the end
		nextWorldPoly = COL_CollGrid[iGrid].NCollPolys;
		for (iInst = 0; iInst < instListSize; iInst++)
		{
			instance = &Instances[instList[iInst]];
			for (iPoly = 0; iPoly < instance->NCollPolys; iPoly++)
			{
				COL_CollGrid[iGrid].CollPolyPtr[nextWorldPoly++] = instance->CollPoly + iPoly;
			}
		}
		COL_CollGrid[iGrid].NCollPolys += nInstPolys;
#endif
	}
	return TRUE;
}
#endif
#endif

/////////////////////////////////////////////////////////////////////
//
// ExpandBBox: expand the bounding box
// to account for the collision tolerance
//
/////////////////////////////////////////////////////////////////////

void ExpandBBox(BBOX *bBox, REAL delta)
{
	bBox->XMin -= delta;
	bBox->XMax += delta;
	bBox->YMin -= delta;
	bBox->YMax += delta;
	bBox->ZMin -= delta;
	bBox->ZMax += delta;
}


/////////////////////////////////////////////////////////////////////
//
// SphereCollPoly: detect collisions between a sphere and the passed
// collision polygon
//
/////////////////////////////////////////////////////////////////////

#ifdef _PC
static REAL	dist[4];
static bool	inside[4];
bool SphereCollPoly(VEC *oldPos, VEC *newPos, REAL radius, NEWCOLLPOLY *collPoly, PLANE *plane, VEC *relPos, VEC *worldPos, REAL *depth, REAL *time)
{
	REAL	oldDist, newDist, dLen;
	int		nSides, outCount, i;

	// Make sure sphere is within "radius" of the plane of the polygon or on the inside
	newDist = VecDotPlane(newPos, &collPoly->Plane);
	if (newDist - radius > COLL_EPSILON) return FALSE;

	// If we were inside the plane last time, probably no collision
	oldDist = VecDotPlane(oldPos, &collPoly->Plane);
	if (oldDist < -(radius + COLL_EPSILON)) return FALSE;

	// Count the faces that the sphere centre is outside of, and store distances
	outCount = 0;
	if (IsPolyQuad(collPoly)) {
		nSides = 4;
	} else {
		nSides = 3;
	}
	for (i = 0; i < nSides; i++) {
		inside[i] = ((dist[i] = VecDotPlane(newPos, &collPoly->EdgePlane[i])) < COLL_EPSILON);
		if (!inside[i]) {
			outCount++;
		}
	}


	// See if the centre is within the poly bounds
	if (outCount == 0) {

		// Get the collision depth
		*depth = newDist - radius;

		// Get rough estimate of collision "time" (fraction of distance travelled at which collision occurred)
		if (ApproxEqual(oldDist, newDist)) {
			*time = ZERO;
		} else {
			*time = DivScalar(oldDist, (oldDist - newDist));
		}

		// Set the return parameters
		CopyPlane(&collPoly->Plane, plane);
		VecEqScalarVec(relPos, -radius, PlaneNormal(plane));
		VecPlusScalarVec(newPos, -newDist, PlaneNormal(plane), worldPos);

		return TRUE;
	}

	// Collision with an edge?
	if (outCount == 1) {

		// find the edge which is on the inside of the centre
		for (i = 0; i < nSides; i++) {
			if (inside[i]) continue;

			// Calculate the coordinate of the point on the edge, nearest the centre
			VecEqScalarVec(PlaneNormal(plane), -dist[i], PlaneNormal(&collPoly->EdgePlane[i]));
			VecPlusEqScalarVec(PlaneNormal(plane), -newDist, PlaneNormal(&collPoly->Plane));
			dLen = VecLen(PlaneNormal(plane));
			if (dLen > (radius)) {
				return FALSE;
			}
			VecPlusVec(PlaneNormal(plane), newPos, worldPos);

			// Calculate the collision depth and time
			*depth = dLen - radius;

			// Get rough estimate of collision "time" (fraction of distance travelled at which collision occurred)
			if (ApproxEqual(oldDist, newDist)) { //dLen)) {
				*time = ZERO;
			} else {
				*time = DivScalar(oldDist, (oldDist - newDist));
			}
			
			break;
		}

		// Chack that the point is within the bounding box 
		// (approximation - eliminates need for voronoi regions)
		if (!PointInBBox(worldPos, &collPoly->BBox)) {
			return FALSE;
		}

		// See if it is inside the tri or quad
		for (i = 0; i < nSides; i++) {
			if (!inside[i]) continue;
			
			if (VecDotPlane(worldPos, &collPoly->EdgePlane[i]) > ZERO) {
				return FALSE;
			}
		}

		// Collision with edge occurred
		if (dLen > SMALL_REAL) {
			//CopyPlane(&collPoly->Plane, plane);
			VecDivScalar(PlaneNormal(plane), -dLen);
		} else {
			CopyPlane(&collPoly->Plane, plane);
		}
		VecEqScalarVec(relPos, -radius, PlaneNormal(plane));


		return TRUE;
	}

	// Collision with vertex ignored

	return FALSE;

}

#else // _PC

bool SphereCollPoly(VEC *oldPos, VEC *newPos, REAL radius, NEWCOLLPOLY *collPoly, PLANE *plane, VEC *relPos, VEC *worldPos, REAL *depth, REAL *time)
{
	int ii, nSides;
	REAL oldDist, newDist;

	// Make sure sphere is within "radius" of the plane of the polygon or on the inside
	newDist = VecDotPlane(newPos, &collPoly->Plane);
	if (newDist - radius > COLL_EPSILON) return FALSE;

	// If we were inside the plane last time, probably no collision
	oldDist = VecDotPlane(oldPos, &collPoly->Plane);
	if (oldDist < -(radius + COLL_EPSILON)) return FALSE;

	// Calculate the time at which the sphere was radius away from the poly
	if (oldDist - newDist > SMALL_REAL) {
		*time = DivScalar((oldDist - radius), (oldDist - newDist));
	} else {
		*time = ZERO;
	}

	// No collision if end point too far from poly
	//if (*time > ONE) return FALSE;

	// Cannot have collision before last frame
	if (*time < ZERO) *time = ZERO;

	// Calculate the coordinates of the collision point
	ScalarVecPlusScalarVec((ONE - *time), oldPos, *time, newPos, worldPos);

	// See if it is within the bounds of the collision poly
	nSides = (IsPolyQuad(collPoly))? 4: 3;
	for (ii = 0; ii < nSides; ii++) {
		if (VecDotPlane(worldPos, &collPoly->EdgePlane[ii]) > COLL_EPSILON) return FALSE;
	}

	// Set the return parameters
	*depth = newDist - radius;
	CopyPlane(&collPoly->Plane, plane);
	VecEqScalarVec(relPos, -radius, PlaneNormal(plane));

	return TRUE;
}

#endif

/////////////////////////////////////////////////////////////////////
//
// LinePlaneIntersect: see if a line intersects a plane
// and return the fraction of the distance
// along the line that the intersection occurred
// time < 0		== no collision
// time = 0		== collision at lStart;
// 0 < time < 1	== collision between lStart and lEnd;
// time = 1		== collision at lEnd;
//
/////////////////////////////////////////////////////////////////////

bool LinePlaneIntersect(VEC *lStart, VEC *lEnd, PLANE *plane, REAL *t, REAL *depth)
{
	REAL	startDotPlane;
	REAL	endDotPlane;

	startDotPlane = VecDotPlane(lStart, plane);
	endDotPlane = VecDotPlane(lEnd, plane);

	// No collision if both points on same side of plane and outside tolerance
	if ((Sign(startDotPlane) == Sign(endDotPlane)) &&
		(abs(startDotPlane) > COLL_EPSILON) && 
		(abs(endDotPlane) > COLL_EPSILON)) return FALSE;

	// No collision if moving away from plane normal
	/*if (Sign(startDotPlane) == ONE) {
		if (startDotPlane < endDotPlane) return FALSE;
	} else {
		if (startDotPlane > endDotPlane) return FALSE;
	}*/

	// Calculate the "time" of the collision

	if (ApproxEqual(startDotPlane, endDotPlane)) {
		*t = ZERO;
	} else {
		*t = DivScalar(startDotPlane, (startDotPlane - endDotPlane));
	}
	*depth = endDotPlane;

	return TRUE;
}



/////////////////////////////////////////////////////////////////////
//
// PosDirPlaneIntersect: calculate the intersection "time" of a line
// with a plane, given the lines starting point and velocity
//
/////////////////////////////////////////////////////////////////////

bool PosDirPlaneIntersect(VEC *lStart, VEC *dir, PLANE *plane, REAL *t)
{
	REAL	startDotPlane;
	REAL	dirDotPlane;

	startDotPlane = VecDotPlaneNorm(lStart, plane);
	dirDotPlane = VecDotPlaneNorm(dir, plane);

	// No collision for lines parallel to plane surface
	if (abs(dirDotPlane) < SIMILAR_REAL) return FALSE;

	// Calculate the "time" of the collision
	*t = - DivScalar((plane->v[D] + startDotPlane), dirDotPlane);
	return TRUE;
}


/////////////////////////////////////////////////////////////////////
//
// FreeCollSkin: free all ram allocated for a collision skin
//
/////////////////////////////////////////////////////////////////////

void FreeCollSkin(COLLSKIN *collSkin)
{

	DestroyConvex(collSkin->WorldConvex, collSkin->NConvex);
	DestroyConvex(collSkin->OldWorldConvex, collSkin->NConvex);
	collSkin->WorldConvex = NULL;
	collSkin->OldWorldConvex = NULL;

	DestroySpheres(collSkin->WorldSphere);
	DestroySpheres(collSkin->OldWorldSphere);
	collSkin->WorldSphere = NULL;
	collSkin->OldWorldSphere = NULL;

	DestroyCollPolys(collSkin->CollPoly);
	collSkin->CollPoly = NULL;
	collSkin->NCollPolys = 0;

}

/////////////////////////////////////////////////////////////////////
//
// CreateConvex: allocate the space required for the body collision skin
//
/////////////////////////////////////////////////////////////////////

CONVEX *CreateConvex(INDEX nConvex)
{
	int iSkin;
	CONVEX *skin;

	Assert(nConvex > 0);

	// Create the space for the required number of convex objects
	if ((skin = (CONVEX *)malloc(sizeof(CONVEX) * nConvex)) == NULL) {
		return NULL;
	}

	for (iSkin = 0; iSkin < nConvex; iSkin++) {
		skin[iSkin].NPts = 0;
		skin[iSkin].NEdges = 0;
		skin[iSkin].NFaces = 0;
		skin[iSkin].Pts = NULL;
		skin[iSkin].Edges = NULL;
		skin[iSkin].Faces = NULL;
	}

	return skin;
}


/////////////////////////////////////////////////////////////////////
//
// SetupConvex: allocate the space for the components of the 
// collision skin passed
//
/////////////////////////////////////////////////////////////////////

bool SetupConvex(CONVEX *skin, INDEX nPts, INDEX extraPts, INDEX nEdges, INDEX nFaces) 
{
	// Make sure the numbers are valid
	Assert((nPts > 0) && (nEdges > 0) && (nFaces > 0));

	// Create space for vertex info
	if (nPts != 0) {
		if ((skin->Pts = (VERTEX *)malloc(sizeof(VERTEX) * (nPts + extraPts))) == NULL) {
			return FALSE;
		}
	}

	// Create space for Edge vertex list (gives offset into point array)
	if (nEdges != 0) {
		if ((skin->Edges = (EDGE *)malloc(sizeof(EDGE) * nEdges)) == NULL) {
			free(skin->Pts);
			return FALSE;
		}
	}

	// Create space for the Face vertex list
	if (nFaces != 0) {
		if ((skin->Faces = (PLANE *)malloc(sizeof(PLANE) * nFaces)) == NULL) {
			free(skin->Edges);
			free(skin->Pts);
			return FALSE;
		}
	}

	skin->NPts = nPts + extraPts;
	skin->NEdges = nEdges;
	skin->NFaces = nFaces;

	return TRUE;
}


/////////////////////////////////////////////////////////////////////
//
// DestroyConvex: deallocate the collision skin space
//
/////////////////////////////////////////////////////////////////////

void DestroyConvex(CONVEX *skin, int nSkins)
{
	int iSkin;

	if ((skin == NULL) || (nSkins ==0)) return;

	for (iSkin = 0; iSkin < nSkins; iSkin++) {
		free(skin[iSkin].Faces);
		free(skin[iSkin].Edges);
		free(skin[iSkin].Pts);
	}
	free(skin);
}

void DestroySpheres(SPHERE *spheres)
{
	free(spheres);
}

/////////////////////////////////////////////////////////////////////
//
// CreateCopyConvex: create a new collision skin and copy
// an existing one across
//
/////////////////////////////////////////////////////////////////////

bool CreateCopyCollSkin(COLLSKIN *collSkin)
{
	int iSkin, ii;

	// Allocate space for convex hulls
	if (collSkin->NConvex > 0) {
		collSkin->WorldConvex = CreateConvex(collSkin->NConvex);
		if (collSkin->WorldConvex == NULL) {
			return FALSE;
		}
		collSkin->OldWorldConvex = CreateConvex(collSkin->NConvex);
		if (collSkin->OldWorldConvex == NULL) {
			DestroyConvex(collSkin->WorldConvex, collSkin->NConvex);
			return FALSE;
		}
	}

	// Allocate space for the collision spheres
	if (collSkin->NSpheres > 0) {
		collSkin->WorldSphere = (SPHERE *)malloc(sizeof(SPHERE) * collSkin->NSpheres);
		if (collSkin->WorldSphere == NULL) {
			DestroyConvex(collSkin->OldWorldConvex, collSkin->NConvex);
			DestroyConvex(collSkin->WorldConvex, collSkin->NConvex);
			return FALSE;
		}
		collSkin->OldWorldSphere = (SPHERE *)malloc(sizeof(SPHERE) * collSkin->NSpheres);
		if (collSkin->OldWorldSphere == NULL) {
			DestroyConvex(collSkin->OldWorldConvex, collSkin->NConvex);
			DestroyConvex(collSkin->WorldConvex, collSkin->NConvex);
			DestroySpheres(collSkin->WorldSphere);
			return FALSE;
		}
	}

	// Allocate space for all points, edges and planes for each hull, and copy info across
	for (iSkin = 0; iSkin < collSkin->NConvex; iSkin++) {

		// Create space for all data
		if (!SetupConvex(&collSkin->WorldConvex[iSkin], collSkin->Convex[iSkin].NPts, 0, collSkin->Convex[iSkin].NEdges, collSkin->Convex[iSkin].NFaces)) {
			DestroyConvex(collSkin->OldWorldConvex, collSkin->NConvex);
			DestroyConvex(collSkin->WorldConvex, collSkin->NConvex);
			DestroySpheres(collSkin->WorldSphere);
			DestroySpheres(collSkin->OldWorldSphere);
			return FALSE;
		}
		collSkin->WorldConvex[iSkin].NPts = collSkin->Convex[iSkin].NPts;
		collSkin->WorldConvex[iSkin].NEdges = collSkin->Convex[iSkin].NEdges;
		collSkin->WorldConvex[iSkin].NFaces = collSkin->Convex[iSkin].NFaces;

		if (!SetupConvex(&collSkin->OldWorldConvex[iSkin], collSkin->Convex[iSkin].NPts, 0, collSkin->Convex[iSkin].NEdges, collSkin->Convex[iSkin].NFaces)) {
			DestroyConvex(collSkin->OldWorldConvex, collSkin->NConvex);
			DestroyConvex(collSkin->WorldConvex, collSkin->NConvex);
			DestroySpheres(collSkin->OldWorldSphere);
			DestroySpheres(collSkin->WorldSphere);
			return FALSE;
		}
		collSkin->OldWorldConvex[iSkin].NPts = collSkin->Convex[iSkin].NPts;
		collSkin->OldWorldConvex[iSkin].NEdges = collSkin->Convex[iSkin].NEdges;
		collSkin->OldWorldConvex[iSkin].NFaces = collSkin->Convex[iSkin].NFaces;

		// Bounding box and offset
		CopyBBox(&collSkin->Convex[iSkin].BBox, &collSkin->WorldConvex[iSkin].BBox);
		//CopyVec(&collSkin->Convex[iSkin].Offset, &collSkin->WorldConvex[iSkin].Offset);
		CopyBBox(&collSkin->Convex[iSkin].BBox, &collSkin->OldWorldConvex[iSkin].BBox);
		//CopyVec(&collSkin->Convex[iSkin].Offset, &collSkin->OldWorldConvex[iSkin].Offset);

		// Vertices
		for (ii = 0; ii < collSkin->Convex[iSkin].NPts; ii++) {
			CopyVec(&collSkin->Convex[iSkin].Pts[ii], &collSkin->WorldConvex[iSkin].Pts[ii]);
			CopyVec(&collSkin->Convex[iSkin].Pts[ii], &collSkin->OldWorldConvex[iSkin].Pts[ii]);
		}

		// Edges (can be removed as never changes - just requires initialisation)
		for (ii = 0; ii < collSkin->Convex[iSkin].NEdges; ii++) {
			collSkin->WorldConvex[iSkin].Edges[ii].Vtx[0] = collSkin->Convex[iSkin].Edges[ii].Vtx[0];
			collSkin->WorldConvex[iSkin].Edges[ii].Vtx[1] = collSkin->Convex[iSkin].Edges[ii].Vtx[1];
			collSkin->OldWorldConvex[iSkin].Edges[ii].Vtx[0] = collSkin->Convex[iSkin].Edges[ii].Vtx[0];
			collSkin->OldWorldConvex[iSkin].Edges[ii].Vtx[1] = collSkin->Convex[iSkin].Edges[ii].Vtx[1];
		}

		// Planes
		for (ii = 0; ii < collSkin->Convex[iSkin].NFaces; ii++) {
			CopyPlane(&collSkin->Convex[iSkin].Faces[ii], &collSkin->WorldConvex[iSkin].Faces[ii]);
			CopyPlane(&collSkin->Convex[iSkin].Faces[ii], &collSkin->OldWorldConvex[iSkin].Faces[ii]);
		}
	}

	// Copy all the spheres
	for (iSkin = 0; iSkin < collSkin->NSpheres; iSkin++) {

		CopyVec(&collSkin->Sphere[iSkin].Pos, &collSkin->WorldSphere[iSkin].Pos);
		collSkin->WorldSphere[iSkin].Radius = collSkin->Sphere[iSkin].Radius;
		CopyVec(&collSkin->Sphere[iSkin].Pos, &collSkin->OldWorldSphere[iSkin].Pos);
		collSkin->OldWorldSphere[iSkin].Radius = collSkin->Sphere[iSkin].Radius;

	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////
//
// BuildWorldSkin: transform the collision skin from the body 
// frame to the world frame. Assumes world skin already allocated
// to be same size as body skin with CreateCopyCollSkin.
//
/////////////////////////////////////////////////////////////////////

void BuildWorldSkin(COLLSKIN *bodySkin, VEC *pos, MAT *mat)
{
	int			iSkin, ii;
	VEC		tmpVec;
	CONVEX		*thisBodySkin, *thisWorldSkin;
	SPHERE		*thisBodySphere, *thisWorldSphere;

	// Transform the convex hulls 
	for (iSkin = 0; iSkin < bodySkin->NConvex; iSkin++) {
		thisBodySkin = &bodySkin->Convex[iSkin];
		thisWorldSkin = &bodySkin->WorldConvex[iSkin];

		// Transfrom the offset
		//VecMulMat(&thisBodySkin->Offset, mat, &thisWorldSkin->Offset);

		// Vertices
		for (ii = 0; ii < thisBodySkin->NPts; ii++) {
			VecMulMat(&thisBodySkin->Pts[ii], mat, &tmpVec);
			VecPlusVec(&tmpVec, pos, &thisWorldSkin->Pts[ii]);
		}

		// Planes
		for (ii = 0; ii < thisBodySkin->NFaces; ii++) {
			RotTransPlane(&thisBodySkin->Faces[ii], mat, pos, &thisWorldSkin->Faces[ii]);
		}

		// Bounding box
		RotTransBBox(&thisBodySkin->BBox, mat, pos, &thisWorldSkin->BBox);
	}

	// Transform the spheres
	for (iSkin = 0; iSkin < bodySkin->NSpheres; iSkin++) {
		thisBodySphere = &bodySkin->Sphere[iSkin];
		thisWorldSphere = &bodySkin->WorldSphere[iSkin];

		// Transfrom the position (radius should already be there)
		VecMulMat(&thisBodySphere->Pos, mat, &thisWorldSphere->Pos);
		VecPlusEqVec(&thisWorldSphere->Pos, pos);
	}

	// Transform the overall bounding box
	RotTransBBox(&bodySkin->TightBBox, mat, pos, &bodySkin->BBox);

}



/////////////////////////////////////////////////////////////////////
//
// LoadConvex: load a set of convex hulls for an object's
// collision skin
//
/////////////////////////////////////////////////////////////////////

#ifdef _PC
CONVEX *LoadConvex(FILE *fp, INDEX *nSkins, int extraPtsPerEdge)
{
	int iSkin, iEdge, iPt, nExtraPts, ptNum;
	VEC offset;

	COLLSKIN_FILEHDR	fileHdr;
	COLLSKIN_COLLHDR	collHdr;
	CONVEX	*collSkin;

	*nSkins = 0;

	if (fp == NULL) {
		*nSkins = 0;
		return NULL;
	}

	// Read in how many convex hulls are in this object and create it
	if (fread(&fileHdr, sizeof(COLLSKIN_FILEHDR), 1, fp) != 1) {
		*nSkins = 0;
		return NULL;
	}
	if ((collSkin = CreateConvex(fileHdr.NSkins)) == NULL) {
		*nSkins = 0;
		return NULL;
	}

	// Get the individual convex hulls
	for (iSkin = 0; iSkin < fileHdr.NSkins; iSkin++) {
		
		// Read size and allocate space
		if (fread(&collHdr, sizeof(COLLSKIN_COLLHDR), 1, fp) != 1) {
			DestroyConvex(collSkin, fileHdr.NSkins);
			*nSkins = 0;
			return NULL;
		}
		
		// Calculate the number of points required 
		// (may be more than in the file if extra points are inserted along each edge)
		nExtraPts = collHdr.NEdges * extraPtsPerEdge;

		// Allocate space for the data
		if (!SetupConvex(&collSkin[iSkin], collHdr.NVertices, nExtraPts, collHdr.NEdges, collHdr.NFaces)) {
			DestroyConvex(collSkin, fileHdr.NSkins);
			*nSkins = 0;
			return NULL;
		}

		// Bounding Box and offset
		if (fread(&collSkin[iSkin].BBox, sizeof(BBOX), 1, fp) != 1) {
			DestroyConvex(collSkin, fileHdr.NSkins);
			*nSkins = 0;
			return NULL;
		}
		//if (fread(&collSkin[iSkin].Offset, sizeof(VEC), 1, fp) != 1) {
		if (fread(&offset, sizeof(VEC), 1, fp) != 1) {
			DestroyConvex(collSkin, fileHdr.NSkins);
			*nSkins = 0;
			return NULL;
		}

		// Read in the vertices
		if (fread(collSkin[iSkin].Pts, sizeof(VERTEX), collHdr.NVertices, fp) != (size_t)collHdr.NVertices) {
			DestroyConvex(collSkin, fileHdr.NSkins);
			*nSkins = 0;
			return NULL;
		}

		// Shrink the skin about the centre
		/*REAL	dist;
		for (iPt = 0; iPt < collHdr.NVertices; iPt++) {
			dist = VecLen(&collSkin[iSkin].Pts[iPt]);
			if (dist > SMALL_REAL) {
				VecMulScalar(&collSkin[iSkin].Pts[iPt], (dist - COL_PointRadius) / dist);
			}
		}*/


		// Read in the edges
		if (fread(collSkin[iSkin].Edges, sizeof(EDGE), collSkin[iSkin].NEdges, fp) != (size_t)collSkin[iSkin].NEdges) {
			DestroyConvex(collSkin, fileHdr.NSkins);
			*nSkins = 0;
			return NULL;
		}

		// Read in the faces
		if (fread(collSkin[iSkin].Faces, sizeof(PLANE), collSkin[iSkin].NFaces, fp) != (size_t)collSkin[iSkin].NFaces) {
			DestroyConvex(collSkin, fileHdr.NSkins);
			*nSkins = 0;
			return NULL;
		}

		// Add the extra points
		ptNum = collHdr.NVertices;
		for (iEdge = 0; iEdge < collHdr.NEdges; iEdge++) {
			for (iPt = 0; iPt < extraPtsPerEdge; iPt++) {
				VecPlusVec(
					&collSkin[iSkin].Pts[collSkin[iSkin].Edges[iEdge].Vtx[0]], 
					&collSkin[iSkin].Pts[collSkin[iSkin].Edges[iEdge].Vtx[1]], 
					&collSkin[iSkin].Pts[ptNum]);
				VecMulScalar(&collSkin[iSkin].Pts[ptNum], HALF);
				ptNum++;
			}
		}
		Assert(ptNum == collSkin[iSkin].NPts);

	}

	*nSkins = fileHdr.NSkins;
	return collSkin;
}

SPHERE *LoadSpheres(FILE *fp, INDEX *nSpheres)
{
	INDEX	iSphere;
	SPHERE	*spheres;
	size_t	nRead;

	Assert(fp != NULL);

	// Read in the number of spheres
	nRead = fread(nSpheres, sizeof(INDEX), 1, fp);
	if (nRead < 1) {
		*nSpheres = 0;
		return NULL;
	}

	// Allocate space for the spheres
	spheres = (SPHERE *)malloc(sizeof(SPHERE) * *nSpheres);
	if (spheres == NULL) {
		*nSpheres = 0;
		return NULL;
	}

//	*nSpheres = 0;
//	return spheres;

	// Read in the spheres
	for (iSphere = 0; iSphere < *nSpheres; iSphere++) {
		nRead = fread(&spheres[iSphere], sizeof(SPHERE), 1, fp);
		if (nRead < 1) {
			*nSpheres = iSphere;
			return spheres;
		}
	}

	Assert(iSphere == *nSpheres);

	return spheres;
}

#elif defined(_PSX)
CONVEX *LoadConvex(FILE *fp, INDEX *nSkins, int extraPtsPerEdge)
{
	int iSkin, iEdge, iPt, ptNum;
	VEC offset;

	COLLSKIN_FILEHDR	fileHdr;
	COLLSKIN_COLLHDR	collHdr;
	CONVEX	*collSkin;

	*nSkins = 0;

	if (fp == NULL) {
		*nSkins = 0;
		return NULL;
	}

	// Read in how many convex hulls are in this object and create it
	if (fread(&fileHdr, sizeof(COLLSKIN_FILEHDR), 1, fp) != 1) {
		*nSkins = 0;
		return NULL;
	}
	if ((collSkin = CreateConvex(fileHdr.NSkins)) == NULL) {
		*nSkins = 0;
		return NULL;
	}

	// Get the individual convex hulls
	for (iSkin = 0; iSkin < fileHdr.NSkins; iSkin++) {
		
		// Read size and allocate space
		if (fread(&collHdr, sizeof(COLLSKIN_COLLHDR), 1, fp) != 1) {
			DestroyConvex(collSkin, fileHdr.NSkins);
			*nSkins = 0;
			return NULL;
		}
		
		// Allocate space for the data
		if (!SetupConvex(&collSkin[iSkin], collHdr.NVertices, 0, collHdr.NEdges, collHdr.NFaces)) {
			DestroyConvex(collSkin, fileHdr.NSkins);
			*nSkins = 0;
			return NULL;
		}

		// Bounding Box and offset
		if (fread(&collSkin[iSkin].BBox, sizeof(BBOX), 1, fp) != 1) {
			DestroyConvex(collSkin, fileHdr.NSkins);
			*nSkins = 0;
			return NULL;
		}
		//if (fread(&collSkin[iSkin].Offset, sizeof(VEC), 1, fp) != 1) {
		if (fread(&offset, sizeof(VEC), 1, fp) != 1) {
			DestroyConvex(collSkin, fileHdr.NSkins);
			*nSkins = 0;
			return NULL;
		}

		// Read in the vertices
		if (fread(collSkin[iSkin].Pts, sizeof(VERTEX), collHdr.NVertices, fp) != (size_t)collHdr.NVertices) {
			DestroyConvex(collSkin, fileHdr.NSkins);
			*nSkins = 0;
			return NULL;
		}

		// Read in the edges
		if (fread(collSkin[iSkin].Edges, sizeof(EDGE), collSkin[iSkin].NEdges, fp) != (size_t)collSkin[iSkin].NEdges) {
			DestroyConvex(collSkin, fileHdr.NSkins);
			*nSkins = 0;
			return NULL;
		}

		// Read in the faces
		if (fread(collSkin[iSkin].Faces, sizeof(PLANE), collSkin[iSkin].NFaces, fp) != (size_t)collSkin[iSkin].NFaces) {
			DestroyConvex(collSkin, fileHdr.NSkins);
			*nSkins = 0;
			return NULL;
		}


	}

	*nSkins = fileHdr.NSkins;
	return collSkin;
}

SPHERE *LoadSpheres(FILE *fp, INDEX *nSpheres)
{
	INDEX	iSphere;
	SPHERE	*spheres;
	size_t	nRead;

	Assert(fp != NULL);

	// Read in the number of spheres
	nRead = fread(nSpheres, sizeof(INDEX), 1, fp);
	if (nRead < 1) {
		*nSpheres = 0;
		return NULL;
	}

	// Allocate space for the spheres
	spheres = (SPHERE *)malloc(sizeof(SPHERE) * *nSpheres);
	if (spheres == NULL) {
		*nSpheres = 0;
		return NULL;
	}

//	*nSpheres = 0;
//	return spheres;

	// Read in the spheres
	for (iSphere = 0; iSphere < *nSpheres; iSphere++) {
		nRead = fread(&spheres[iSphere], sizeof(SPHERE), 1, fp);
		if (nRead < 1) {
			*nSpheres = iSphere;
			return spheres;
		}
	}

	Assert(iSphere == *nSpheres);

	return spheres;
}

#elif defined(_N64)
CONVEX *LoadConvex(FIL *fp, INDEX *nSkins, int extraPtsPerEdge)
{
	int iSkin, iEdge, iPt, nExtraPts, ptNum;

	COLLSKIN_FILEHDR	fileHdr;
	COLLSKIN_COLLHDR	collHdr;
	CONVEX	*collSkin;
	VEC					arse;

	*nSkins = 0;

	if (fp == NULL)
	{
		*nSkins = 0;
		return NULL;
	}

	// Read in how many convex hulls are in this object and create it
	if (FFS_Read(&fileHdr, sizeof(COLLSKIN_FILEHDR), fp) < sizeof(COLLSKIN_FILEHDR))
	{
		*nSkins = 0;
		return NULL;
	}

	if ((collSkin = CreateConvex(fileHdr.NSkins)) == NULL)
	{
		*nSkins = 0;
		return NULL;
	}

	// Get the individual convex hulls
	for (iSkin = 0; iSkin < fileHdr.NSkins; iSkin++)
	{
		// Read size and allocate space
		if (FFS_Read(&collHdr, sizeof(COLLSKIN_COLLHDR), fp) != sizeof(COLLSKIN_COLLHDR))
		{
			DestroyConvex(collSkin, fileHdr.NSkins);
			*nSkins = 0;
			return NULL;
		}
		
		// Calculate the number of points required 
		// (may be more than in the file if extra points are inserted along each edge)
		nExtraPts = collHdr.NEdges * extraPtsPerEdge;

		// Allocate space for the data
		if (!SetupConvex(&collSkin[iSkin], collHdr.NVertices, nExtraPts, collHdr.NEdges, collHdr.NFaces))
		{
			DestroyConvex(collSkin, fileHdr.NSkins);
			*nSkins = 0;
			return NULL;
		}

		// Bounding Box and offset
		if (FFS_Read(&collSkin[iSkin].BBox, sizeof(BBOX), fp) != sizeof(BBOX))
		{
			DestroyConvex(collSkin, fileHdr.NSkins);
			*nSkins = 0;
			return NULL;
		}
		if (FFS_Read(&arse, sizeof(VEC), fp) != sizeof(VEC))
		{
			DestroyConvex(collSkin, fileHdr.NSkins);
			*nSkins = 0;
			return NULL;
		}

		// Read in the vertices
		if (FFS_Read(collSkin[iSkin].Pts, sizeof(VERTEX) * collHdr.NVertices, fp) != (sizeof(VERTEX) * collHdr.NVertices))
		{
			DestroyConvex(collSkin, fileHdr.NSkins);
			*nSkins = 0;
			return NULL;
		}

		// Read in the edges
		if (FFS_Read(collSkin[iSkin].Edges, sizeof(EDGE) * collSkin[iSkin].NEdges, fp) != (sizeof(EDGE) * collSkin[iSkin].NEdges))
		{
			DestroyConvex(collSkin, fileHdr.NSkins);
			*nSkins = 0;
			return NULL;
		}

		// Read in the faces
		if (FFS_Read(collSkin[iSkin].Faces, sizeof(PLANE) * collSkin[iSkin].NFaces, fp) != (sizeof(PLANE) * collSkin[iSkin].NFaces))
		{
			DestroyConvex(collSkin, fileHdr.NSkins);
			*nSkins = 0;
			return NULL;
		}

		// Add the extra points
		ptNum = collHdr.NVertices;
		for (iEdge = 0; iEdge < collHdr.NEdges; iEdge++)
		{
			for (iPt = 0; iPt < extraPtsPerEdge; iPt++)
			{
				VecPlusVec(
					&collSkin[iSkin].Pts[collSkin[iSkin].Edges[iEdge].Vtx[0]], 
					&collSkin[iSkin].Pts[collSkin[iSkin].Edges[iEdge].Vtx[1]], 
					&collSkin[iSkin].Pts[ptNum]);
				VecMulScalar(&collSkin[iSkin].Pts[ptNum], HALF);
				ptNum++;
			}
		}
		Assert(ptNum == collSkin[iSkin].NPts);
	}

	*nSkins = fileHdr.NSkins;
	return collSkin;
}


SPHERE *LoadSpheres(FIL *fp, INDEX *nSpheres)
{
	INDEX	iSphere;
	SPHERE	*spheres;
	size_t	nRead;

	Assert(fp != NULL);

	// Read in the number of spheres
	nRead = FFS_Read(nSpheres, sizeof(INDEX), fp);
	if (nRead < sizeof(INDEX))
	{
		*nSpheres = 0;
		return NULL;
	}

	// Allocate space for the spheres
	spheres = (SPHERE *)malloc(sizeof(SPHERE) * *nSpheres);
	if (spheres == NULL)
	{
		*nSpheres = 0;
		return NULL;
	}

//	*nSpheres = 0;
//	return spheres;

	// Read in the spheres
	for (iSphere = 0; iSphere < *nSpheres; iSphere++)
	{
		nRead = FFS_Read(&spheres[iSphere], sizeof(SPHERE), fp);
		if (nRead < sizeof(SPHERE)) {
			*nSpheres = iSphere;
			return spheres;
		}
	}

	Assert(iSphere == *nSpheres);

	return spheres;
}

#endif



/////////////////////////////////////////////////////////////////////
//
// PointInConvex: Check whether the passed point lies within the
// body (point should be in the frame of the body owning the skin)
//
/////////////////////////////////////////////////////////////////////

bool PointInConvex(VEC *pos, CONVEX *skin, PLANE *plane, REAL *minDepth) 
{
	int		iFace;

	REAL	depth;
	
	*minDepth = -LARGEDIST;

	//VecMinusVec(pos, &skin->Offset, &localPos);

	for (iFace = 0; iFace < skin->NFaces; iFace++) {
		depth = VecDotPlane(pos, &skin->Faces[iFace]);
		if (depth > COLL_EPSILON) return FALSE;
		if (depth > *minDepth) {
			*minDepth = depth;
			CopyPlane(&skin->Faces[iFace], plane);
		}
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////
//
// LineToConvexColl: see if a line intersects a convex hull. Return a 
// pointer to the plane through which the line passed, or the
// nearest plane to the end point if both points inside hull.
// Also give the penetration depth in *penDepth.
//
/////////////////////////////////////////////////////////////////////

PLANE *LineToConvexColl(VEC *sPos, VEC *ePos, CONVEX *skin, REAL *penDepth, REAL *time)
{
	int		iFace;
	REAL	depth, depth2, minDepth, sDist;
	REAL	minTime, timeDepth;
	VEC		pos, dR;
	PLANE	*nearPlane, *entryPlane, plane;

	VecMinusVec(ePos, sPos, &dR);

	// See if the end-point is within the hull and remember the nearest plane
	minDepth = -LARGEDIST;
	for (iFace = 0; iFace < skin->NFaces; iFace++) {
		depth = VecDotPlane(ePos, &skin->Faces[iFace]);
		if (depth > COLL_EPSILON) return NULL;
		if (depth > minDepth) {
			minDepth = depth;
			nearPlane = &skin->Faces[iFace];
		}
	}

	// Find the point of entry if there is one
	minTime = ZERO;
	for (iFace = 0; iFace < skin->NFaces; iFace++) {
		if (LinePlaneIntersect(sPos, ePos, &skin->Faces[iFace], time, &depth)) {
			if (*time > minTime) {
				VecPlusScalarVec(sPos, *time, &dR, &pos);
				if (PointInConvex(&pos, skin, &plane, &depth2)) {
					minTime = *time;
					timeDepth = depth;
					entryPlane = &skin->Faces[iFace];
				}
			}
		}
	}

	// Decide which result to use
	if (minTime < ONE) {
		// most recent face
		*penDepth = timeDepth;
		*time = minTime;
		return entryPlane;
	} else {
		// nearest face
		sDist = VecDotPlane(sPos, nearPlane);
		*penDepth = minDepth;
		if (ApproxEqual(sDist, minDepth)) {
			*time = ZERO;
		} else {
			*time = DivScalar(sDist, (sDist - minDepth));
		}
		return nearPlane;
	}
}
		



/////////////////////////////////////////////////////////////////////
//
// ModifyShift: recalculate the shift required to extract one
// skin from another
//
/////////////////////////////////////////////////////////////////////

void ModifyShift(VEC *shift, REAL shiftMag, VEC *normal)
{
#ifndef _PSX
	int		iR;
	REAL	newShift;

	for (iR = 0; iR < 3; iR++) {
		newShift = (shiftMag) * normal->v[iR] ;
		if (Sign(shift->v[iR]) == Sign(newShift)) {
			if (abs(shift->v[iR]) < abs(newShift)) {
				shift->v[iR] = newShift;
			}
		} else {
			shift->v[iR] += newShift;
		}
	}
#else //_PSX
	int		iR;
	REAL	newShift;
	REAL	*normComp, *shiftComp;

	normComp = &normal->v[0];
	shiftComp = &shift->v[0];

	for (iR = 0; iR < 3; iR++) {
		newShift = MulScalar(shiftMag, *normComp);
		if (Sign(*shiftComp) == Sign(newShift)) {
			if (abs(*shiftComp) < abs(newShift)) {
				*shiftComp = newShift;
			}
		} else {
			*shiftComp += newShift;
		}
		normComp++;
		shiftComp++;
	}
#endif
}


/////////////////////////////////////////////////////////////////////
//
// PointInCollPolyBounds: given a point known to be in the plane
// of the polygon, check if it lies within the bounds of the 
// polygon defined by the vertices in the NEWCOLLPOLY object
//
/////////////////////////////////////////////////////////////////////

bool PointInCollPolyBounds(VEC *pt, NEWCOLLPOLY *poly)
{
	if (VecDotPlane(pt, &poly->EdgePlane[0]) > COLL_EPSILON) return FALSE;
	if (VecDotPlane(pt, &poly->EdgePlane[1]) > COLL_EPSILON) return FALSE;
	if (VecDotPlane(pt, &poly->EdgePlane[2]) > COLL_EPSILON) return FALSE;
	if (IsPolyQuad(poly)) {
		if (VecDotPlane(pt, &poly->EdgePlane[3]) > COLL_EPSILON) return FALSE;
	}
	/*if (VecDotPlane(pt, &poly->EdgePlane[0]) > ZERO) return FALSE;
	if (VecDotPlane(pt, &poly->EdgePlane[1]) > ZERO) return FALSE;
	if (VecDotPlane(pt, &poly->EdgePlane[2]) > ZERO) return FALSE;
	if (IsPolyQuad(poly)) {
		if (VecDotPlane(pt, &poly->EdgePlane[3]) > ZERO) return FALSE;
	}*/

	return TRUE;
}

/////////////////////////////////////////////////////////////////////
//
// SphereInCollPolyBounds: check whether a sphere is within the 
// infinite prism defined by the edge planes of the collision poly
//
/////////////////////////////////////////////////////////////////////

bool SphereInCollPolyBounds(VEC *pt, REAL radius, NEWCOLLPOLY *poly)
{
#if TRUE
	if (VecDotPlane(pt, &poly->EdgePlane[0]) > (COLL_EPSILON)) return FALSE;
	if (VecDotPlane(pt, &poly->EdgePlane[1]) > (COLL_EPSILON)) return FALSE;
	if (VecDotPlane(pt, &poly->EdgePlane[2]) > (COLL_EPSILON)) return FALSE;
	if (IsPolyQuad(poly)) {
		if (VecDotPlane(pt, &poly->EdgePlane[3]) > (COLL_EPSILON)) return FALSE;
	}
#else
	if (VecDotPlane(pt, &poly->EdgePlane[0]) > (COLL_EPSILON + radius)) return FALSE;
	if (VecDotPlane(pt, &poly->EdgePlane[1]) > (COLL_EPSILON + radius)) return FALSE;
	if (VecDotPlane(pt, &poly->EdgePlane[2]) > (COLL_EPSILON + radius)) return FALSE;
	if (IsPolyQuad(poly)) {
		if (VecDotPlane(pt, &poly->EdgePlane[3]) > (COLL_EPSILON + radius)) return FALSE;
	}
#endif

	return TRUE;
}


/////////////////////////////////////////////////////////////////////
//
// SphereCollSkin: check for collision between a sphere and a 
// convex hull. Set the passed collision point and  normal and 
// return TRUE for a collision.
//
//	inputs:
//		spherePos		- position of sphere centre in collskin coords
//		sphereRad		- radius of sphere
//		skin			- the collision skin
//
//	outputs:
//		collPos			- collision position in skin coords
//		collNorm		- collision normal in skin coords
//		collShift		- shift to move sphere out of skin
//
/////////////////////////////////////////////////////////////////////

#ifdef _PC
bool SphereConvex(VEC		*spherePos, 
					REAL		sphereRad, 
					CONVEX		*skin, 
					VEC		*collPos,
					PLANE		*collPlane,
					REAL		*collDepth)
{
	bool	collided;
	int		iFace;
	REAL	dist, maxDist;

	maxDist = -LARGEDIST;
	collided = FALSE;

	for (iFace = 0; iFace < skin->NFaces; iFace++) {
		
		// Check that sphere's centre falls within "sphereRadius" of skin
		if ((dist = (VecDotPlane(spherePos, &skin->Faces[iFace]) - sphereRad)) > COLL_EPSILON) return FALSE;

		if (maxDist < dist) {
			
			// Point on sphere nearest surface in world coords (well, not always...)
			VecMinusScalarVec(spherePos, sphereRad, PlaneNormal(&skin->Faces[iFace]), collPos);

			maxDist = dist;
			collided = TRUE;

			// The shift required to extract sphere from skin...
			*collDepth = maxDist;
			// Collision Plane
			CopyPlane(&skin->Faces[iFace], collPlane);
		}
	}

	//if (*collDepth > ZERO) *collDepth = ZERO;

	return collided;
}
#else
bool SphereConvex(VEC		*spherePos, 
					REAL		sphereRad, 
					CONVEX		*skin, 
					VEC		*collPos,
					PLANE		*collPlane,
					REAL		*collDepth)
{
	int		iFace;
	REAL	dist, maxDist;

	maxDist = -LARGEDIST;

	for (iFace = 0; iFace < skin->NFaces; iFace++) {
		
		// Check that sphere's centre falls within "sphereRadius" of skin
		if ((dist = (VecDotPlane(spherePos, &skin->Faces[iFace]) - sphereRad)) > COLL_EPSILON) return FALSE;

		if (maxDist < dist) {
			maxDist = dist;

			// Point on sphere nearest surface in world coords (well, not always...)
			VecMinusScalarVec(spherePos, sphereRad, PlaneNormal(&skin->Faces[iFace]), collPos);
			// The shift required to extract sphere from skin...
			*collDepth = maxDist;
			// Collision Plane
			CopyPlane(&skin->Faces[iFace], collPlane);
		}
	}

	//if (*collDepth > ZERO) *collDepth = ZERO;

	return TRUE;
}
#endif

/////////////////////////////////////////////////////////////////////
//
// MakeTightLocalBBox: calculate the tight-fitting local-frame
// axis-aligned bounding-box for the passed collision skin
//
/////////////////////////////////////////////////////////////////////

void MakeTightLocalBBox(COLLSKIN *collSkin)
{
	int iSphere, iSkin, iPt;


	SetBBox(&collSkin->TightBBox, LARGEDIST, -LARGEDIST, LARGEDIST, -LARGEDIST, LARGEDIST, -LARGEDIST);

	// Loop over the collision spheres
	for (iSphere = 0; iSphere < collSkin->NSpheres; iSphere++) {
		AddSphereToBBox(&collSkin->TightBBox, &collSkin->Sphere[iSphere]);
	}

	// Loop over the skins
	for (iSkin = 0; iSkin < collSkin->NConvex; iSkin++) {

		// Set the BBox to obviously wrong values
		SetBBox(&collSkin->Convex[iSkin].BBox, LARGEDIST, -LARGEDIST, LARGEDIST, -LARGEDIST, LARGEDIST, -LARGEDIST);

		// Loop over points in collision skin
		for (iPt = 0; iPt < collSkin->Convex[iSkin].NPts; iPt++) {

			// get the bounding box in local coordinates (as if offset were zero)
			AddPointToBBox(&collSkin->Convex[iSkin].BBox, &collSkin->Convex[iSkin].Pts[iPt]);

		}

	}

}



/////////////////////////////////////////////////////////////////////
//
// RotTransBBox: rotate and translate a bounding box, then make it
// axis-aligned again
//
/////////////////////////////////////////////////////////////////////

/*void RotTransBBox(BBOX *bBoxIn, MAT *rotMat, VEC *dR, BBOX *bBoxOut)
{
	int		ii;
	VEC	corners[8], tmpVec;

	// get the corners of the box
	SetVec(&corners[0], bBoxIn->XMin, bBoxIn->YMin, bBoxIn->ZMin);
	SetVec(&corners[1], bBoxIn->XMin, bBoxIn->YMin, bBoxIn->ZMax);
	SetVec(&corners[2], bBoxIn->XMin, bBoxIn->YMax, bBoxIn->ZMin);
	SetVec(&corners[3], bBoxIn->XMin, bBoxIn->YMax, bBoxIn->ZMax);

	SetVec(&corners[4], bBoxIn->XMax, bBoxIn->YMin, bBoxIn->ZMin);
	SetVec(&corners[5], bBoxIn->XMax, bBoxIn->YMin, bBoxIn->ZMax);
	SetVec(&corners[6], bBoxIn->XMax, bBoxIn->YMax, bBoxIn->ZMin);
	SetVec(&corners[7], bBoxIn->XMax, bBoxIn->YMax, bBoxIn->ZMax);

	// Transform the corners
	for (ii = 0; ii < 8; ii++) {
		VecMulMat(&corners[ii], rotMat, &tmpVec);
		VecPlusVec(&tmpVec, dR, &corners[ii]);
	}

	// Recalculate axis-aligned box

	bBoxOut->XMin = corners[0].v[X];
	bBoxOut->XMax = corners[0].v[X];
	bBoxOut->YMin = corners[0].v[Y];
	bBoxOut->YMax = corners[0].v[Y];
	bBoxOut->ZMin = corners[0].v[Z];
	bBoxOut->ZMax = corners[0].v[Z];
	for (ii = 1; ii < 8; ii++) {
		bBoxOut->XMin = Min(bBoxOut->XMin, corners[ii].v[X]);
		bBoxOut->XMax = Max(bBoxOut->XMax, corners[ii].v[X]);
		bBoxOut->YMin = Min(bBoxOut->YMin, corners[ii].v[Y]);
		bBoxOut->YMax = Max(bBoxOut->YMax, corners[ii].v[Y]);
		bBoxOut->ZMin = Min(bBoxOut->ZMin, corners[ii].v[Z]);
		bBoxOut->ZMax = Max(bBoxOut->ZMax, corners[ii].v[Z]);
	}
}*/


/////////////////////////////////////////////////////////////////////
//
// CorrugationAmp: return the corrugation height of the 
// passed material for the passed x and y offsets
//
/////////////////////////////////////////////////////////////////////

REAL CorrugationAmp(CORRUGATION *cor, REAL dx, REAL dy) 
{
	REAL argX, argZ;

#ifndef _PSX
	argX = 2.0f * PI * dx / cor->Lx;
	argZ = 2.0f * PI * dy / cor->Ly;
#else 
	argX = DivScalar(dx, cor->Lx) >> (FIXED_PRECISION - 12);
	argZ = DivScalar(dy, cor->Ly) >> (FIXED_PRECISION - 12);
#endif

	return MulScalar(cor->Amp, (REAL)MulScalar(cos(argX), cos(argZ)));
}



/////////////////////////////////////////////////////////////////////
//
// LineOfSight: check whether the passed point is in the line
// of sight of the passed position.
//
/////////////////////////////////////////////////////////////////////

bool LineOfSight(VEC *src, VEC *dest)
{
	int		iPoly;
	long	gridNum, endGridNum, dX, dZ, xGrid, zGrid;
	REAL	t, depth, minT;
	VEC	dR, intersect;
	PLANE	face;
	COLLGRID	*grid;
	NEWCOLLPOLY *poly;
	bool	lastGrid = FALSE;

	// Vector separation of the two points
	VecMinusVec(dest, src, &dR);

	// Get the starting grid location
	gridNum = PosToCollGridCoords(src, &xGrid, &zGrid);
	endGridNum = PosToCollGridNum(dest);

	// Loop over grids between src and dest points
	do {

		// make sure the grid location is valid
		if ((xGrid >= NearestInt(COL_CollGridData.XNum)) || (zGrid >= NearestInt(COL_CollGridData.ZNum)) || (xGrid < 0) || (zGrid < 0))  {
			return TRUE;
		}

		grid = &COL_CollGrid[gridNum];

		// Loop over grid locations which are between the start and end points
		for (iPoly = 0; iPoly < grid->NCollPolys; iPoly++) {
#ifndef _PSX
			poly = grid->CollPolyPtr[iPoly];
#else 
			poly = &COL_WorldCollPoly[grid->CollPolyIndices[iPoly]];
#endif
			// See if there is an intersection
			if (!LinePlaneIntersect(src, dest, &poly->Plane, &t, &depth)) continue;
			VecPlusScalarVec(src, t, &dR, &intersect);
			if (!PointInCollPolyBounds(&intersect, poly)) continue;

			// Line of sight is obstructed
			return FALSE;
		}

		// See if this is the last grid or move on to next position
		if (gridNum == endGridNum) {
			lastGrid = TRUE;
		} else {

			// Check each face of the grid to see which the line passes through first

			minT = Real(2.0);
			if (dR.v[X] < ZERO) {

				// Left
				SetPlane(&face, ONE, ZERO, ZERO, -(COL_CollGridData.XStart + xGrid * COL_CollGridData.GridSize));
				if (LinePlaneIntersect(src, dest, &face, &t, &depth)) {
					if (t < minT) {
						minT = t;
						dX = -1;
						dZ = 0;
					}
				}
			} else {
				// Right
				SetPlane(&face, -ONE, ZERO, ZERO, (COL_CollGridData.XStart + (xGrid + 1) * COL_CollGridData.GridSize));
				if (LinePlaneIntersect(src, dest, &face, &t, &depth)) {
					if (t < minT) {
						minT = t;
						dX = 1;
						dZ = 0;
					}
				}
			}
			if (dR.v[Z] < ZERO) {

				// Back
				SetPlane(&face, ZERO, ZERO, ONE, -(COL_CollGridData.ZStart + zGrid * COL_CollGridData.GridSize));
				if (LinePlaneIntersect(src, dest, &face, &t, &depth)) {
					if (t < minT) {
						minT = t;
						dX = 0;
						dZ = -1;
					}
				}
			} else {
				// Forward
				SetPlane(&face, ZERO, ZERO, -ONE, (COL_CollGridData.ZStart + (zGrid + 1) * COL_CollGridData.GridSize));
				if (LinePlaneIntersect(src, dest, &face, &t, &depth)) {
					if (t < minT) {
						minT = t;
						dX = 0;
						dZ = 1;
					}
				}
			}

			xGrid += dX;
			zGrid += dZ;
			if ((xGrid >= COL_CollGridData.XNum) || (zGrid >= COL_CollGridData.ZNum)) {
				gridNum = -1;
			} else {
				gridNum += dX + NearestInt(COL_CollGridData.XNum) * dZ;
			}
		}

	} while ((gridNum != -1) && (!lastGrid));


	// Got this far, so must have line of sight
	return TRUE;
}


/////////////////////////////////////////////////////////////////////
//
// LineOfSightDist: see if there is line of sight between source
// and dest. Fill minT with the fraction along the line that 
// the obstruction occurred + fill plane with ptr to plane
/////////////////////////////////////////////////////////////////////

bool LineOfSightDist(VEC *src, VEC *dest, REAL *minDist, PLANE **plane)
{
	int		iPoly;
	long	gridNum, endGridNum, dX, dZ, xGrid, zGrid;
	REAL	t, depth, minT;
	VEC	dR, intersect;
	PLANE	face;
	COLLGRID	*grid;
	NEWCOLLPOLY *poly;
	bool	lastGrid = FALSE;

	// Vector separation of the two points
	VecMinusVec(dest, src, &dR);
	*minDist = ONE;

	// Get the starting grid location
	gridNum = PosToCollGridCoords(src, &xGrid, &zGrid);
	endGridNum = PosToCollGridNum(dest);

	// Loop over grids between src and dest points
	do {

		// make sure the grid location is valid
		if ((xGrid >= NearestInt(COL_CollGridData.XNum)) || (zGrid >= NearestInt(COL_CollGridData.ZNum)) || (xGrid < 0) || (zGrid < 0))  {
			return TRUE;
		}

		grid = &COL_CollGrid[gridNum];

		// Loop over grid locations which are between the start and end points
		for (iPoly = 0; iPoly < grid->NCollPolys; iPoly++) {
#ifndef _PSX
			poly = grid->CollPolyPtr[iPoly];
#else
			poly = &COL_WorldCollPoly[grid->CollPolyIndices[iPoly]];
#endif
			// See if there is an intersection
			if (!LinePlaneIntersect(src, dest, &poly->Plane, &t, &depth)) continue;
			if (depth > ZERO) continue;
			VecPlusScalarVec(src, t, &dR, &intersect);
			if (!PointInCollPolyBounds(&intersect, poly)) continue;

			// Line of sight is obstructed - is this the nearest
			if (t < *minDist) {
				*minDist = t;
				if (plane) {
					*plane = &poly->Plane;
				}
			}
		}

		// See if this is the last grid or move on to next position
		if (gridNum == endGridNum) {
			lastGrid = TRUE;
		} else {

			// Check each face of the grid to see which the line passes through first

			minT = Real(2.0);
			if (dR.v[X] < ZERO) {

				// Left
				SetPlane(&face, ONE, ZERO, ZERO, -(COL_CollGridData.XStart + xGrid * COL_CollGridData.GridSize));
				if (LinePlaneIntersect(src, dest, &face, &t, &depth)) {
					if (t < minT) {
						minT = t;
						dX = -1;
						dZ = 0;
					}
				}
			} else {
				// Right
				SetPlane(&face, -ONE, ZERO, ZERO, (COL_CollGridData.XStart + (xGrid + 1) * COL_CollGridData.GridSize));
				if (LinePlaneIntersect(src, dest, &face, &t, &depth)) {
					if (t < minT) {
						minT = t;
						dX = 1;
						dZ = 0;
					}
				}
			}
			if (dR.v[Z] < ZERO) {

				// Back
				SetPlane(&face, ZERO, ZERO, ONE, -(COL_CollGridData.ZStart + zGrid * COL_CollGridData.GridSize));
				if (LinePlaneIntersect(src, dest, &face, &t, &depth)) {
					if (t < minT) {
						minT = t;
						dX = 0;
						dZ = -1;
					}
				}
			} else {
				// Forward
				SetPlane(&face, ZERO, ZERO, -ONE, (COL_CollGridData.ZStart + (zGrid + 1) * COL_CollGridData.GridSize));
				if (LinePlaneIntersect(src, dest, &face, &t, &depth)) {
					if (t < minT) {
						minT = t;
						dX = 0;
						dZ = 1;
					}
				}
			}

			xGrid += dX;
			zGrid += dZ;
			if ((xGrid >= COL_CollGridData.XNum) || (zGrid >= COL_CollGridData.ZNum)) {
				gridNum = -1;
			} else {
				gridNum += dX + NearestInt(COL_CollGridData.XNum) * dZ;
			}
		}

	} while ((gridNum != -1) && (!lastGrid));


	// Got this far, so must have line of sight
	return TRUE;
}


/////////////////////////////////////////////////////////////////////
//
// LineOfSightObj: loop over collidable objects and check for line
// of sitgh between source and dest points
//
/////////////////////////////////////////////////////////////////////
#ifdef _PC
bool LineOfSightObj(VEC *src, VEC *dest, REAL *minDist)
{
	bool los;
	int iWheel;
	REAL dist;
	BBOX bBox;
	OBJECT *obj;

	// Initialise
	los = TRUE;
	*minDist = ONE;

	SetBBox(&bBox, LARGEDIST, -LARGEDIST, LARGEDIST, -LARGEDIST, LARGEDIST, -LARGEDIST);
	AddPointToBBox(&bBox, src);
	AddPointToBBox(&bBox, dest);

	// Loop over objects
	for (obj = OBJ_ObjectHead; obj != NULL; obj = obj->next) {

		// Check for LoS through the collision skins
		switch (obj->CollType) {

		case COLL_TYPE_CAR:
			if (!BBTestXZY(&obj->player->car.BBox, &bBox)) continue;
			for (iWheel = 0; iWheel < CAR_NWHEELS; iWheel++) {
				if (IsWheelPresent(&obj->player->car.Wheel[iWheel])) {
					if (!LineOfSightSphere(&obj->player->car.Wheel[iWheel].WPos, obj->player->car.Wheel[iWheel].Radius, src, dest, &dist)) {
						los = FALSE;
						if (dist < *minDist) {
							*minDist = dist;
						}
					}
				}
			}
		case COLL_TYPE_BODY:
			if (!BBTestXZY(&obj->body.CollSkin.BBox, &bBox)) continue;
			if (!LineOfSightBody(&obj->body, src, dest, &dist)) {
				los = FALSE;
				if (dist < *minDist) {
					*minDist = dist;
				}
			}
			break;

		default:
			break;
		}
	}

	return los;
}

bool LineOfSightBody(NEWBODY *body, VEC *src, VEC *dest, REAL *minDist)
{
	bool los;
	if (body->CollSkin.CollType == BODY_COLL_SPHERE) {
		los = LineOfSightSphere(&body->CollSkin.WorldSphere[0].Pos, body->CollSkin.WorldSphere[0].Radius, src, dest, minDist);
		return los;
	} else {
		Assert(body->CollSkin.CollType == BODY_COLL_CONVEX);
		los = LineOfSightConvex(body, src, dest, minDist);
		return los;
	}
}


bool LineOfSightConvex(NEWBODY *body, VEC *src, VEC *dest, REAL *minDist)
{
	int iSkin;
	REAL dist, depth, time, dRLen, dR2Len, minDepth;
	VEC nearPt, dR, dR2;
	bool los;

	los = TRUE;
	*minDist = ONE;
	minDepth = LARGEDIST;

	dist = NearPointOnLine(src, dest, &body->Centre.Pos, &nearPt);
	if (!PointInBBox(&nearPt, &body->CollSkin.BBox)) return TRUE;
	VecMinusVec(dest, src, &dR2);
	dR2Len = VecLen(&dR2);
	VecMinusVec(&nearPt, src, &dR);
	dRLen = VecLen(&dR);

	for (iSkin = 0; iSkin < body->CollSkin.NConvex; iSkin++) {
		if (LineToConvexColl(src, &nearPt, &body->CollSkin.WorldConvex[iSkin], &depth, &time) == NULL) continue;
		
		los = FALSE;
		if (depth < minDepth) {
			minDepth = depth;
		}
	}

	*minDist = dist + (minDepth / dR2Len);

	return los;
}

bool LineOfSightSphere(VEC *sphPos, REAL rad, VEC *src, VEC *dest, REAL *minDist)
{
	REAL b, c, d, dRLen;
	VEC dR, dS;


	VecMinusVec(dest, src, &dR);
	dRLen = VecLen(&dR);
	VecDivScalar(&dR, dRLen);
	VecMinusVec(src, sphPos, &dS);

	b = Real(2) * VecDotVec(&dR, &dS);
	c = VecDotVec(&dS, &dS) - rad * rad;

	d = b * b - (Real(4)  * c);

	if (d < 0) {
		*minDist = ONE;
		return TRUE;
	}

	if (b > 0) {
		*minDist = HALF * (-b + (REAL)sqrt(d)) / dRLen;
	} else {
		*minDist = HALF * (-b - (REAL)sqrt(d)) / dRLen;
	}
	return FALSE;

}
#endif

/////////////////////////////////////////////////////////////////////
//
// GetCollPolyVertices:
//
/////////////////////////////////////////////////////////////////////

int GetCollPolyVertices(NEWCOLLPOLY *poly, VEC *v0, VEC *v1, VEC *v2, VEC *v3)
{

	PlaneIntersect3(&poly->Plane, &poly->EdgePlane[0], &poly->EdgePlane[1], v0);
	PlaneIntersect3(&poly->Plane, &poly->EdgePlane[1], &poly->EdgePlane[2], v1);
	if (IsPolyQuad(poly)) {
		PlaneIntersect3(&poly->Plane, &poly->EdgePlane[2], &poly->EdgePlane[3], v2);
		PlaneIntersect3(&poly->Plane, &poly->EdgePlane[3], &poly->EdgePlane[0], v3);
	} else {
		PlaneIntersect3(&poly->Plane, &poly->EdgePlane[2], &poly->EdgePlane[0], v2);
	}

	return (IsPolyQuad(poly))? 4: 3;
}


/////////////////////////////////////////////////////////////////////
//
// AddSphereToBBox:
//
/////////////////////////////////////////////////////////////////////

void AddPosRadToBBox(BBOX *bBox, VEC *pos, REAL radius) 
{
	bBox->XMin = Min(bBox->XMin, pos->v[X] - radius);
	bBox->XMax = Max(bBox->XMax, pos->v[X] + radius);
	bBox->YMin = Min(bBox->YMin, pos->v[Y] - radius);
	bBox->YMax = Max(bBox->YMax, pos->v[Y] + radius);
	bBox->ZMin = Min(bBox->ZMin, pos->v[Z] - radius);
	bBox->ZMax = Max(bBox->ZMax, pos->v[Z] + radius);
}

/////////////////////////////////////////////////////////////////////
//
// AddPointToBBox:
//
/////////////////////////////////////////////////////////////////////

void AddPointToBBox(BBOX *bBox, VEC *pos)
{
	bBox->XMin = Min(bBox->XMin, pos->v[X]);
	bBox->XMax = Max(bBox->XMax, pos->v[X]);
	bBox->YMin = Min(bBox->YMin, pos->v[Y]);
	bBox->YMax = Max(bBox->YMax, pos->v[Y]);
	bBox->ZMin = Min(bBox->ZMin, pos->v[Z]);
	bBox->ZMax = Max(bBox->ZMax, pos->v[Z]);
}


/////////////////////////////////////////////////////////////////////
//
// RotTransBBox:
//
/////////////////////////////////////////////////////////////////////

void RotTransBBox(BBOX *srcBox, MAT *mat, VEC *pos, BBOX *destBox)
{
	VEC pt, destPt;

	// Set destination box to invalid box
	SetBBox(destBox, LARGEDIST, -LARGEDIST, LARGEDIST, -LARGEDIST, LARGEDIST, -LARGEDIST);

	// Transform the corners of the source box into new frame and add to dest box
	SetVec(&pt, srcBox->XMin, srcBox->YMin, srcBox->ZMin);
	VecMulMat(&pt, mat, &destPt);
	VecPlusEqVec(&destPt, pos);
	AddPointToBBox(destBox, &destPt);

	SetVec(&pt, srcBox->XMin, srcBox->YMin, srcBox->ZMax);
	VecMulMat(&pt, mat, &destPt);
	VecPlusEqVec(&destPt, pos);
	AddPointToBBox(destBox, &destPt);

	SetVec(&pt, srcBox->XMin, srcBox->YMax, srcBox->ZMin);
	VecMulMat(&pt, mat, &destPt);
	VecPlusEqVec(&destPt, pos);
	AddPointToBBox(destBox, &destPt);

	SetVec(&pt, srcBox->XMin, srcBox->YMax, srcBox->ZMax);
	VecMulMat(&pt, mat, &destPt);
	VecPlusEqVec(&destPt, pos);
	AddPointToBBox(destBox, &destPt);

	SetVec(&pt, srcBox->XMax, srcBox->YMin, srcBox->ZMin);
	VecMulMat(&pt, mat, &destPt);
	VecPlusEqVec(&destPt, pos);
	AddPointToBBox(destBox, &destPt);

	SetVec(&pt, srcBox->XMax, srcBox->YMin, srcBox->ZMax);
	VecMulMat(&pt, mat, &destPt);
	VecPlusEqVec(&destPt, pos);
	AddPointToBBox(destBox, &destPt);

	SetVec(&pt, srcBox->XMax, srcBox->YMax, srcBox->ZMin);
	VecMulMat(&pt, mat, &destPt);
	VecPlusEqVec(&destPt, pos);
	AddPointToBBox(destBox, &destPt);

	SetVec(&pt, srcBox->XMax, srcBox->YMax, srcBox->ZMax);
	VecMulMat(&pt, mat, &destPt);
	VecPlusEqVec(&destPt, pos);
	AddPointToBBox(destBox, &destPt);

}

void TransBBox(BBOX *bBox, VEC *sPos)
{
	bBox->XMin += sPos->v[X];
	bBox->XMax += sPos->v[X];
	bBox->YMin += sPos->v[Y];
	bBox->YMax += sPos->v[Y];
	bBox->ZMin += sPos->v[Z];
	bBox->ZMax += sPos->v[Z];
}

/////////////////////////////////////////////////////////////////////
//
// RotTransCollPoly:
//
/////////////////////////////////////////////////////////////////////

void RotTransCollPolys(NEWCOLLPOLY *collPoly, int nPolys, MAT *rMat, VEC *dPos)
{
	int iPoly, iEdge, nEdges;
	NEWCOLLPOLY *poly;
	PLANE plane;
	BBOX bBox;

	for (iPoly = 0; iPoly < nPolys; iPoly++) {
		poly = &collPoly[iPoly];
		nEdges = (IsPolyQuad(poly))? 4: 3;

		RotTransPlane(&poly->Plane, rMat, dPos, &plane);
		CopyPlane(&plane, &poly->Plane);

		for (iEdge = 0; iEdge < nEdges; iEdge++) {

			RotTransPlane(&poly->EdgePlane[iEdge], rMat, dPos, &plane);
			CopyPlane(&plane, &poly->EdgePlane[iEdge]);

		}

		RotTransBBox(&poly->BBox, rMat, dPos, &bBox);
		CopyBBox(&bBox, &poly->BBox);
	}
}

/////////////////////////////////////////////////////////////////////
//
// TransCollPolys:
//
/////////////////////////////////////////////////////////////////////

void TransCollPolys(NEWCOLLPOLY *collPoly, int nPolys, VEC *dPos)
{
	int iPoly, iEdge, nEdges;
	NEWCOLLPOLY *poly;

	for (iPoly = 0; iPoly < nPolys; iPoly++) {
		poly = &collPoly[iPoly];
		nEdges = (IsPolyQuad(poly))? 4: 3;

		MovePlane(&poly->Plane, dPos);

		for (iEdge = 0; iEdge < nEdges; iEdge++) {

			MovePlane(&poly->EdgePlane[iEdge], dPos);

		}

		TransBBox(&poly->BBox, dPos);
	}
}


/////////////////////////////////////////////////////////////////////
//
// NextBodyCollInfo:
//
/////////////////////////////////////////////////////////////////////

COLLINFO_BODY *NextBodyCollInfo()
{
	if (COL_NBodyColls == MAX_COLLS_BODY) {
		return NULL;
	} else {
		COL_BodyCollInfo[COL_NBodyColls].Prev = COL_BodyCollInfo[COL_NBodyColls].Next = NULL;
		COL_BodyCollInfo[COL_NBodyColls].Active = FALSE;
		return &COL_BodyCollInfo[COL_NBodyColls];
	}
}

/////////////////////////////////////////////////////////////////////
//
// NextWheelCollInfo:
//
/////////////////////////////////////////////////////////////////////

COLLINFO_WHEEL *NextWheelCollInfo()
{
	if (COL_NWheelColls == MAX_COLLS_BODY) {
		return NULL;
	} else {
		COL_WheelCollInfo[COL_NWheelColls].Prev = COL_WheelCollInfo[COL_NWheelColls].Next = NULL;
		//COL_WheelCollInfo[COL_NWheelColls].Active = FALSE;
		return &COL_WheelCollInfo[COL_NWheelColls];
	}
}

