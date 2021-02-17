//-----------------------------------------------------------------------------
// File: newcoll.cpp
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "ReVolt.h"
#include "Geom.h"
#include "Util.h"
#include "NewColl.h"
#include "Particle.h"
#include "Body.h"
#include "Aerial.h"
#include "Wheel.h"
#include "Model.h"
#include "Car.h"
#include "Main.h"
#include "LevelLoad.h"
#include "Instance.h"
#include "ctrlread.h"
#include "Object.h"
#include "Field.h"
#include "control.h"
#include "player.h"
#include "spark.h"
#include "ai.h"
#include "obj_init.h"
#include "initplay.h"
#include "input.h"

#define COLLGRID_EXPAND TO_LENGTH(Real(70))

/////////////////////////////////////////////////////////////////////
//
// Prototypes
//

void COL_BodyCollHandler(OBJECT *obj);
void COL_CarCollHandler(OBJECT *obj);
void COL_WaitForCollision(OBJECT *obj);

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
bool SetupConvex(CONVEX *skin, INDEX nFaces);
void DestroyConvex(CONVEX *skin, int nSkins);
void DestroySpheres(SPHERE *spheres);
bool CreateCopyCollSkin(COLLSKIN *collSkin);
void BuildWorldSkin(COLLSKIN *bodySkin, VEC *pos, MAT *mat);
#ifdef _PC
CONVEX *LoadConvex(FILE *fp, INDEX *nSkins);
SPHERE *LoadSpheres(FILE *fp, INDEX *nSpheres);
#elif defined(_N64)
CONVEX *LoadConvex(FIL *fp, INDEX *nSkins);
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
#ifndef _PSX
bool LineOfSightObj(VEC *src, VEC *dest, REAL *minT, OBJECT *objIgnore);
bool LineOfSightBody(NEWBODY *body, VEC *src, VEC *dest, REAL *minDist);
bool LineOfSightSphere(VEC *sphPos, REAL rad, VEC *src, VEC *dest, REAL *minDist);
bool LineOfSightConvex(NEWBODY *body, VEC *src, VEC *dest, REAL *minDist);
#endif

void CopyCollPoly(NEWCOLLPOLY *src, NEWCOLLPOLY *dest);
void RotTransCollPolys(NEWCOLLPOLY *collPoly, int nPolys, MAT *rMat, VEC *dPos);
void TransCollPolys(NEWCOLLPOLY *collPoly, int nPolys, VEC *dPos);
COLLINFO_BODY *NextBodyCollInfo();

void InitCollGrid(COLLGRID *grid);
void UpdateObjectGrid(OBJECT *obj);
void AddObjectToGrid(OBJECT *obj, COLLGRID *grid);
void RemoveObjectFromGrid(OBJECT *obj, COLLGRID *grid);
long GetNearGrids(VEC *pos, COLLGRID **grids);


int NonPlanarCount = 0;
int QuadCollCount = 0;
int TriCollCount = 0;

/////////////////////////////////////////////////////////////////////
//
// Globals 
//

NEWCOLLPOLY *COL_WorldCollPoly = NULL;      // Collision skin for world model
INDEX       COL_NWorldCollPolys = 0;
NEWCOLLPOLY *COL_InstanceCollPoly = NULL;   // Instance skin when placed in world
INDEX       COL_NInstanceCollPolys = 0;
COLLGRID_DATA   COL_CollGridData;       // Gridding information
COLLGRID        *COL_CollGrid;          // Poly pointers and counter for each grid volume
long            COL_NCollGrids;         // Number of grid locations

// Arrays to hold collision info for all objects
COLLINFO_BODY       COL_BodyCollInfo[MAX_COLLS_BODY];
COLLINFO_WHEEL      COL_WheelCollInfo[MAX_COLLS_WHEEL];
int COL_NBodyColls = 0;
int COL_NBodyDone = 0;
int COL_NWheelColls = 0;
int COL_NWheelDone = 0;
int COL_NCollsTested = 0;
int COL_NCollsPassed = 0;
int COL_NBodyBodyTests = 0;

// Car pair collision matrix
CARCOLL_MATRIX CarCollMatrix[MAX_NUM_PLAYERS][MAX_NUM_PLAYERS];

// Dummy Variables
PLANE DummyPlane;
REAL DummyReal;


/////////////////////////////////////////////////////////////////////
//
// Array of pointers to functions which give the function which
// detects collisions between two objects of two types
//
static bool (*COL_ObjObjColl[MAX_COLL_TYPES][MAX_COLL_TYPES])(OBJECT *obj1, OBJECT *obj2) = {
            /*  NONE                BODY                CAR             
    /*NONE*/    {COL_Dummy2Coll,    COL_Dummy2Coll,     COL_Dummy2Coll, },
    /*BODY*/    {COL_Dummy2Coll,    COL_BodyBodyColl,   COL_BodyCarColl,},
    /*CAR*/     {COL_Dummy2Coll,    COL_CarBodyColl,    COL_CarCarColl, },
};

static void (*COL_ObjWorldColl[MAX_COLL_TYPES])(OBJECT *obj) = {
    /*NONE*/    COL_DummyColl,
    /*BODY*/    COL_BodyWorldColl,
    /*CAR*/     COL_CarWorldColl,

};

/////////////////////////////////////////////////////////////////////
//
// Materials for the collision polys
//
MATERIAL    COL_MaterialInfo[MATERIAL_NTYPES] = {
    {   // MATERIAL_DEFAULT,
        MATERIAL_SKID | MATERIAL_SPARK,         // Properties
        Real(1.0),          // Roughness
        Real(1.0),          // Gripiness
        Real(1.0),          // Hardness
        0x707070L,          // SkidColour
        CORRUG_NONE,        // Corrugation number
        DUST_NONE,          // Dust type (using spark engine)
        {ZERO, ZERO, ZERO}  // Velocity
    },
    {   //MATERIAL_MARBLE,
        MATERIAL_SKID | MATERIAL_SPARK,
        Real(0.9),
        Real(0.9),
        Real(0.5),
        0x707070,
        CORRUG_NONE,
        DUST_NONE,
        {ZERO, ZERO, ZERO}
    },
    {   //MATERIAL_STONE,
        MATERIAL_SKID | MATERIAL_SPARK,
        Real(0.9),
        Real(0.9),
        Real(0.5),
        0x909090,
        CORRUG_NONE,
        DUST_NONE,
        {ZERO, ZERO, ZERO}
    },      
    {//MATERIAL_WOOD,
        MATERIAL_SKID,
        Real(0.8),
        Real(0.8),
        Real(0.3),
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
        Real(0.2),
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
        Real(0.7),
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
        Real(0.3),
        0x303030,
        CORRUG_NONE,
        DUST_NONE,
        {ZERO, ZERO, ZERO}
    },
    { //MATERIAL_ICE1,
        MATERIAL_SKID | MATERIAL_SPARK,
        Real(0.4),
        Real(0.4),
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
        MATERIAL_SKID | MATERIAL_CORRUGATED | MATERIAL_DUSTY,
        Real(0.7),
        Real(0.5),
        Real(0.0),
        0x8060FF,
        CORRUG_STEEL,
        DUST_GRASS,
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
        Real(0.8),
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
        {Real(-5 * 57.476f), -Real(-5 * 25.749f), Real(5 * 77.676f) }
    },
    { //MATERIAL_CONVEYOR2,
        MATERIAL_MOVES | MATERIAL_CORRUGATED,
        Real(1.0),
        Real(1.0),
        Real(0.0),
        0x000000,
        CORRUG_STEEL,
        DUST_NONE,
        {Real(5 * 57.476f), Real(5 * 25.749f), Real(-5 * 77.676f) }
    },
    { //MATERIAL_DIRT1,
        MATERIAL_SKID | MATERIAL_CORRUGATED | MATERIAL_DUSTY,
        Real(1.0),
        Real(1.0),
        Real(0.0),
        //0x00FFFF,
        0x88bbdd,
        CORRUG_DIRT1,
        DUST_DIRT,
        {ZERO, ZERO, ZERO}
    },
    { //MATERIAL_DIRT2,
        MATERIAL_SKID | MATERIAL_CORRUGATED | MATERIAL_DUSTY,
        Real(1.0),
        Real(1.0),
        Real(0.0),
        //0x00FFFF,
        0x88bbdd,
        CORRUG_DIRT2,
        DUST_DIRT,
        {ZERO, ZERO, ZERO}
    },
    { //MATERIAL_DIRT3,
        MATERIAL_SKID | MATERIAL_CORRUGATED | MATERIAL_DUSTY,
        Real(1.0),
        Real(1.0),
        Real(0.0),
        //0x00FFFF,
        0x88bbdd,
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
        Real(0.5),
        Real(0.5),
        Real(0.2),
        0x303030,
        CORRUG_NONE,
        DUST_NONE,
        {ZERO, ZERO, ZERO}
    },
    { //MATERIAL_WOOD2,
        MATERIAL_SKID,
        Real(0.8),
        Real(0.8),
        Real(0.4),
        0x303030,
        CORRUG_NONE,
        DUST_NONE,
        {ZERO, ZERO, ZERO}
    },
    { //MATERIAL_CONVEYOR_MARKET1,
        MATERIAL_MOVES | MATERIAL_CORRUGATED,
        Real(1.0),
        Real(1.0),
        Real(0.0),
        0x000000,
        CORRUG_STEEL,
        DUST_NONE,
        {ZERO, ZERO, MPH2OGU_SPEED * 4}
    },
    { //MATERIAL_CONVEYOR_MARKET2,
        MATERIAL_MOVES | MATERIAL_CORRUGATED,
        Real(1.0),
        Real(1.0),
        Real(0.0),
        0x000000,
        CORRUG_STEEL,
        DUST_NONE,
        {-MPH2OGU_SPEED * 4, ZERO, ZERO}
    },
    {   //MATERIAL_PAVING,
        MATERIAL_SKID | MATERIAL_SPARK,
        Real(0.9),
        Real(0.9),
        Real(0.5),
        0x909090,
        CORRUG_NONE,
        DUST_ROAD,
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
        TO_LENGTH(Real(3.0)),
        TO_LENGTH(Real(70.0)),
        TO_LENGTH(Real(70.0))
    },
    { //CORRUG_GRAVEL,
        TO_LENGTH(Real(1.0)),
        TO_LENGTH(Real(40.0)),
        TO_LENGTH(Real(40.0))
    },
    { //CORRUG_STEEL,
        TO_LENGTH(Real(1.0)),
        TO_LENGTH(Real(40.0)),
        TO_LENGTH(Real(40.0))
    },
    { //CORRUG_CONVEYOR,
        TO_LENGTH(Real(1.0)),
        TO_LENGTH(Real(80.0)),
        TO_LENGTH(Real(80.0))
    },
    { //CORRUG_DIRT1,
        TO_LENGTH(Real(1.0)),
        TO_LENGTH(Real(80.0)),
        TO_LENGTH(Real(80.0))
    },
    { //CORRUG_DIRT2,
        TO_LENGTH(Real(1.0)),
        TO_LENGTH(Real(80.0)),
        TO_LENGTH(Real(80.0))
    },
    { //CORRUG_DIRT3,
        TO_LENGTH(Real(1.0)),
        TO_LENGTH(Real(80.0)),
        TO_LENGTH(Real(80.0))
    },
};

#ifndef _PSX
DUST COL_DustInfo[DUST_NTYPES] = {
    { // DUST_NONE
        SPARK_NONE,
        ZERO,
        ZERO,
        SPARK_NONE,
        ZERO,
        ZERO,
    },
    { // DUST_GRAVEL
        SPARK_GRAVEL,       // Type of particle
        Real(0.6),          // Probability of particle generation
        Real(0.6),          // Dust randomisation factor
        SPARK_GRAVELDUST,   // Type of smoke
        Real(0.3),          // Probability of smoke generation
        Real(0.6),          // Smoke randomisation factor
    },
    { // DUST_SAND
        SPARK_SAND,         // Type of particle
        Real(0.2),          // Probability of particle generation
        Real(0.6),          // Randomisation factor
        SPARK_SANDDUST,     // Type of smoke
        Real(0.1),          // Probability of smoke generation
        Real(0.6),          // Smoke randomisation factor
    },
    { // DUST_GRASS
        SPARK_GRASS,        // Type of particle
        Real(0.8),          // Probability of particle generation
        Real(0.6),          // Randomisation factor
        SPARK_GRASSDUST,    // Type of smoke
        Real(0.4),          // Probability of smoke generation
        Real(0.6),          // Smoke randomisation factor
    },
    { // DUST_DIRT
        SPARK_DIRT,         // Type of particle
        Real(0.4),          // Probability of particle generation
        Real(0.8),          // Randomisation factor
        SPARK_SOILDUST,     // Type of smoke
        Real(0.2),          // Probability of smoke generation
        Real(0.8),          // Smoke randomisation factor
    },
    { // DUST_ROAD
        SPARK_NONE,         // Type of particle
        ZERO,               // Probability of particle generation
        ZERO,               // Randomisation factor
        SPARK_ROADDUST,     // Type of smoke
        Real(0.2),          // Probability of smoke generation
        Real(0.8),          // Smoke randomisation factor
    },
};
#endif


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
    OBJECT  *obj, *next;

    COL_AllObjectColls();

    #if DEBUG && defined(_PSX)
    PhysicsCount[4] = GetRCnt(RCntCNT1);
    #endif

    obj = OBJ_ObjectHead;
    while (obj != NULL)
    {
        next = obj->next;
        if (obj->collhandler)
        {
            obj->collhandler(obj);
        }
        obj = next;
    }
    
}


/////////////////////////////////////////////////////////////////////
// COL_AllObjectColls:
/////////////////////////////////////////////////////////////////////

void COL_AllObjectColls(void)
{
    int iGrid;
    register OBJECT *obj1, *obj2, *next1, *next2;
    COLLGRID *grid;

    // initialisation
    COL_NBodyColls = 0;
    COL_NBodyDone = 0;
    COL_NWheelColls = 0;
    COL_NWheelDone = 0;
    COL_NBodyBodyTests = 0;

    ClearCarCollMatrix();

    COL_NCollsTested = COL_NCollsPassed = 0;

    for (obj1 = OBJ_ObjectHead; obj1 != NULL; obj1 = obj1->next) {
        if (obj1->body.CollSkin.AllowWorldColls || obj1->body.CollSkin.AllowObjColls) {
            obj1->body.NWorldContacts = 0;
            obj1->body.NOtherContacts = 0;
            obj1->body.NBodyColls = 0;
            obj1->body.BodyCollHead = NULL;
            if (obj1->CollType == COLL_TYPE_CAR) {
                obj1->player->car.NWheelColls = 0;
                obj1->player->car.WheelCollHead = NULL;
            }
        }
    }

    // loop over all object pairs
    obj1 = OBJ_ObjectHead;
    while (obj1 != NULL) {
        next1 = obj1->next;

        Assert(obj1->CollType >= 0 && obj1->CollType < MAX_COLL_TYPES);

        // object-world collision if allowed for this object
        if (obj1->body.CollSkin.AllowWorldColls) {
            COL_ObjWorldColl[obj1->CollType](obj1);
        }

        // Make sure this object is allowed to collide with objects
        if (!obj1->body.CollSkin.AllowObjColls) {
            obj1 = next1;
            continue;
        }
#if GRIDIFY_OBJECTS

        if (obj1->Grids[0] != NULL) {

            // Object only checks against those in adjacent grids
            for (iGrid = 0; iGrid < 4; iGrid++) {
                grid = obj1->Grids[iGrid];
                if (grid == NULL) break;

                obj2 = grid->ObjectHead;
                while (obj2 != NULL) {
                    next2 = obj2->NextGridObj;

                    if (obj2->body.CollSkin.AllowObjColls == FALSE) {
                        obj2 = next2;
                        continue;
                    }

                    if (obj1->ReplayStoreInfo.ID >= obj2->ReplayStoreInfo.ID) 
                    {
                        obj2 = next2;
                        continue;
                    }

                    if (obj1->body.Stacked && obj2->body.Stacked) {
                        obj2 = next2;
                        continue;
                    }

                    // body-body test count
                    COL_NBodyBodyTests++;

                    // Do the actual collision check
                    if (COL_ObjObjColl[obj1->CollType][obj2->CollType](obj1, obj2)) {

                        // Clear the stacked flags
                        obj1->body.Stacked = obj2->body.Stacked = FALSE;

                        // Set object pair collision flags if necessary
                        if ((obj1->Type == OBJECT_TYPE_CAR) && (obj2->Type == OBJECT_TYPE_CAR)) {
                            SetCarsCollided(obj1, obj2);
                        } 
                        #ifdef _PC      // Sprinkler hose collision
                        else if (obj1->Type == OBJECT_TYPE_SPRINKLER_HOSE) { 
                            SPRINKLER_HOSE_OBJ *hose = (SPRINKLER_HOSE_OBJ*)obj1->Data;
                            //hose->Sprinkler->OnHose = TRUE;
                            hose->Sprinkler->OnHoseTimer = ZERO;
                        }
                        else if (obj2->Type == OBJECT_TYPE_SPRINKLER_HOSE) {
                            SPRINKLER_HOSE_OBJ *hose = (SPRINKLER_HOSE_OBJ*)obj2->Data;
                            //hose->Sprinkler->OnHose = TRUE;
                            hose->Sprinkler->OnHoseTimer = ZERO;
                        }
                        #endif
                    }

                    // Next object
                    obj2 = next2;
                }
            }

        } else {

            obj2 = OBJ_ObjectHead;

#else // GRIDIFY_OBJECTS

        {
            obj2 = next1;

#endif // GRIDIFY_OBJECTS

            // Object must check against all other objects
            while (obj2 != NULL) {
                next2 = obj2->next;
            
                Assert(obj2->CollType >= 0 && obj2->CollType < MAX_COLL_TYPES);

#if GRIDIFY_OBJECTS
                if (obj1 == obj2) {
                    obj2 = next2;
                    continue;
                }
#endif // GRIDIFY_OBJECTS

                // Make sure this object is allowed to collide with objects
                if (!obj2->body.CollSkin.AllowObjColls) {
                    obj2 = next2;
                    break; // All objects which allow object-object collisions are at atart of list
                }

                // body-body test count
                COL_NBodyBodyTests++;

                // Do the actual collision check
                if (COL_ObjObjColl[obj1->CollType][obj2->CollType](obj1, obj2)) {

                    // Clear the stacked flags
                    obj1->body.Stacked = obj2->body.Stacked = FALSE;

                    // Set object pair collision flags if necessary
                    if ((obj1->Type == OBJECT_TYPE_CAR) && (obj2->Type == OBJECT_TYPE_CAR)) {
                        SetCarsCollided(obj1, obj2);
                    } 
                    #ifdef _PC      // Sprinkler hose collision
                    else if (obj1->Type == OBJECT_TYPE_SPRINKLER_HOSE) { 
                        SPRINKLER_HOSE_OBJ *hose = (SPRINKLER_HOSE_OBJ*)obj1->Data;
                        //hose->Sprinkler->OnHose = TRUE;
                        hose->Sprinkler->OnHoseTimer = ZERO;
                    }
                    else if (obj2->Type == OBJECT_TYPE_SPRINKLER_HOSE) {
                        SPRINKLER_HOSE_OBJ *hose = (SPRINKLER_HOSE_OBJ*)obj2->Data;
                        //hose->Sprinkler->OnHose = TRUE;
                        hose->Sprinkler->OnHoseTimer = ZERO;
                    }
                    #endif
                }
                obj2 = next2;
            }
        }

        obj1 = next1;
    }
}

////////////////////////////////////////////////////////////////
//
// Object type collision detection 
//
////////////////////////////////////////////////////////////////

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

    // If both objects stacked, do nothing
    if (obj1->body.Stacked && obj2->body.Stacked) return FALSE;

    // Bounding box test
    if (!BBTestXZY(&obj1->body.CollSkin.BBox, &obj2->body.CollSkin.BBox)) return FALSE;

    // Detect
    if (DetectBodyBodyColls(&obj1->body, &obj2->body) > 0) {
        return TRUE;
    }

    return FALSE;

}

// body-car collision detection
bool COL_BodyCarColl(OBJECT *obj1, OBJECT *obj2)
{
    Assert(obj1->CollType == COLL_TYPE_BODY && obj2->CollType == COLL_TYPE_CAR);
    // Bounding box test
    if (!BBTestXZY(&obj2->player->car.BBox, &obj1->body.CollSkin.BBox)) return FALSE;

    // Detect
    if (DetectCarBodyColls(&obj2->player->car, &obj1->body) > 0) {
        obj2->body.NoMoveTime = ZERO;
        obj2->body.NoContactTime = ZERO;
        return TRUE;
    }
    return FALSE;
}

// car-body collision detection
bool COL_CarBodyColl(OBJECT *obj1, OBJECT *obj2)
{
    Assert(obj1->CollType == COLL_TYPE_CAR && obj2->CollType == COLL_TYPE_BODY);
    // Bounding box test
    if (!BBTestXZY(&obj1->player->car.BBox, &obj2->body.CollSkin.BBox)) return FALSE;

    // Detect
    if (DetectCarBodyColls(&obj1->player->car, &obj2->body) > 0) {
        obj1->body.NoMoveTime = ZERO;
        obj1->body.NoContactTime = ZERO;
        return TRUE;
    }
    return FALSE;
}

// car-car collision detection
bool COL_CarCarColl(OBJECT *obj1, OBJECT *obj2)
{
    Assert(obj1->CollType == COLL_TYPE_CAR && obj2->CollType == COLL_TYPE_CAR);
    // Quick bounding-box test
    if (!BBTestXZY(&obj1->player->car.BBox, &obj2->player->car.BBox)) return FALSE;

    if (DetectCarCarColls(&obj1->player->car, &obj2->player->car) > 0) {
        obj1->body.NoMoveTime = ZERO;
        obj1->body.NoContactTime = ZERO;
        obj2->body.NoMoveTime = ZERO;
        obj2->body.NoContactTime = ZERO;
        return TRUE;
    }
    return FALSE;
}

// no collision detection
bool COL_Dummy2Coll(OBJECT *obj1, OBJECT *obj2)
{
    return FALSE;
}

////////////////////////////////////////////////////////////////
//
// Car pair collision stuff
//
////////////////////////////////////////////////////////////////

void ClearCarCollMatrix()
{
    long i;

    for (i = 0 ; i < StartData.PlayerNum ; i++)
    {
        memset(CarCollMatrix[i], 0, StartData.PlayerNum);
    }
}

void SetCarsCollided(OBJECT *obj1, OBJECT *obj2)
{
    register long slot1, slot2, tmp;

    Assert(obj1->Type == OBJECT_TYPE_CAR && obj2->Type == OBJECT_TYPE_CAR);

    slot1 = obj1->player->Slot;
    slot2 = obj2->player->Slot;

    Assert(slot1 != slot2);

    // Shouldn't be necessary as players are creates in slot order I think...
    if (slot1 > slot2) {
        tmp = slot1;
        slot1 = slot2;
        slot2 = tmp;
    }

    CarCollMatrix[slot1][slot2].Status |= CARS_COLLIDED;
}

bool HaveCarsCollided(OBJECT *obj1, OBJECT *obj2)
{
    register long slot1, slot2, tmp;
    Assert(obj1->Type == OBJECT_TYPE_CAR && obj2->Type == OBJECT_TYPE_CAR);

    slot1 = obj1->player->Slot;
    slot2 = obj2->player->Slot;

    Assert(slot1 != slot2);

    if (slot1 > slot2) {
        tmp = slot1;
        slot1 = slot2;
        slot2 = tmp;
    }

    return (CarCollMatrix[slot1][slot2].Status & CARS_COLLIDED);
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
    //BodyTurboBoost(body);

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
//
// COL_WaitForCollision: don't do anything unles there is a collision
// with another moving object. Then reset object to have default move
// and collision hanlders
// 
/////////////////////////////////////////////////////////////////////

void COL_WaitForCollision(OBJECT *obj)
{
    VEC imp, angImp;
    NEWBODY *body = &obj->body;
    FIELD_DATA fieldData;

    // if no collisions, do nothing
    if (obj->body.NBodyColls == 0) return;

    // see if there is a collision with another body
    PreProcessBodyColls(&obj->body);
    PostProcessBodyColls(&obj->body);

    if (obj->body.NOtherContacts == 0) {
        return;
    }

    SetVecZero(&obj->body.Centre.Impulse);
    SetVecZero(&obj->body.AngImpulse);

    // Collision with another body - reset object handlers
    obj->collhandler = obj->defaultcollhandler;
    obj->movehandler = obj->defaultmovehandler;
    obj->body.NoMoveTime = ZERO;
    obj->body.CollSkin.AllowWorldColls = TRUE;

    // Reset necessary variables
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
        ProcessBodyColls3(body);
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
#if DEBUG && defined(_PSX)
        PhysicsCount[5] = GetRCnt(RCntCNT1);
#endif
        PreProcessCarWheelColls(car);
#if DEBUG && defined(_PSX)
        PhysicsCount[6] = GetRCnt(RCntCNT1);
#endif
        ProcessCarWheelColls(car);
#if DEBUG && defined(_PSX)
        PhysicsCount[7] = GetRCnt(RCntCNT1);
#endif
        PostProcessCarWheelColls(car);
#if DEBUG && defined(_PSX)
        PhysicsCount[8] = GetRCnt(RCntCNT1);
#endif
    }

    // Add aerodynamic downforce
    CarDownForce(car);

    // Add speedup force
#ifdef _PC
    SpeedupImpulse(car);
#endif

    // Process body collisions
    COL_BodyCollHandler(obj);
#if DEBUG && defined(_PSX)
        PhysicsCount[9] = GetRCnt(RCntCNT1);
#endif

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
            newGrids[iGrid].CollPolyIndices = NULL;
        }
    }

    return newGrids;
}

void DestroyCollGrids()
{
    int iGrid;

    if (COL_CollGrid == NULL) return;

    for (iGrid = 0; iGrid < COL_NCollGrids; iGrid++) {
        free(COL_CollGrid[iGrid].CollPolyIndices);
    }

    free(COL_CollGrid);
    COL_NCollGrids = 0;

}

short *CreateCollPolyIndices(int nPolys)
{
    return (short *)malloc(nPolys * sizeof(short));
}

/////////////////////////////////////////////////////////////////////
//
// PositionToGrid: return the grid array index corresponding to 
// the passed position
//
/////////////////////////////////////////////////////////////////////

long PosToCollGridCoords(VEC *pos, long *offsetX, long *offsetZ)
{
    REAL    dX, dZ, xNum, zNum;

    // Make sure the world is gridded (if not, world is one large grid)
    if ((COL_CollGridData.XNum == ZERO) && (COL_CollGridData.ZNum == ZERO)) {
        return 0;
    }

    // Calculate grid index
    dX = pos->v[X] - COL_CollGridData.XStart;
    dZ = pos->v[Z] - COL_CollGridData.ZStart;
    xNum = NearestInt(COL_CollGridData.XNum);
    zNum = NearestInt(COL_CollGridData.ZNum);

    *offsetX = Int(DivScalar(dX, COL_CollGridData.GridSize));
    *offsetZ = Int(DivScalar(dZ, COL_CollGridData.GridSize));

    // Make sure we are not outside the grid boundaries
    if ((*offsetX < 0L) || (*offsetX >= xNum) ||
        (*offsetZ < 0L) || (*offsetZ >= zNum))
    {
        return -1;
    }

    return *offsetX + xNum * *offsetZ;
}


long PosToCollGridNum(VEC *pos)
{
    REAL    dX, dZ, xNum, zNum;
    long    offsetX, offsetZ;

    // Make sure the world is gridded (if not, world is one large grid)
    if ((COL_CollGridData.XNum == ZERO) && (COL_CollGridData.ZNum == ZERO)) {
        return 0;
    }

    // Calculate grid index
    dX = pos->v[X] - COL_CollGridData.XStart;
    dZ = pos->v[Z] - COL_CollGridData.ZStart;
    xNum = NearestInt(COL_CollGridData.XNum);
    zNum = NearestInt(COL_CollGridData.ZNum);

    offsetX = Int(DivScalar(dX, COL_CollGridData.GridSize));
    offsetZ = Int(DivScalar(dZ, COL_CollGridData.GridSize));

    // Make sure we are not outside the grid boundaries
    if ((offsetX < 0L) || (offsetX >= xNum) ||
        (offsetZ < 0L) || (offsetZ >= zNum))
    {
        return -1;
    }


    return offsetX + xNum * offsetZ;
}

COLLGRID *PosToCollGrid(VEC *pos)
{
    int gridNum;

    gridNum = PosToCollGridNum(pos);

    if (gridNum < 0  )
    {
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
#define PAVING_STONE_GAP    3

#if defined(_PC)

NEWCOLLPOLY *LoadNewCollPolys(FILE *fp, short *nPolys)
{
    NEWCOLLPOLYHDR header;
    NEWCOLLPOLY * polys;
    size_t nRead;
    short iPoly;



    // read the header
    nRead = fread(&header, sizeof(NEWCOLLPOLYHDR), 1, fp);

    if (nRead < 1) {
        *nPolys = 0; //$ADDITION(cprince): $PCBUG this missing line also breaks PC version when run under debugger
        return NULL;
    }

    // Allocate space for the polys
    if ((polys = CreateCollPolys(header.NPolys)) == NULL) {
        *nPolys = 0; //$ADDITION(cprince): $PCBUG this missing line also breaks PC version when run under debugger
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

    // Success!
    *nPolys = header.NPolys;


    return polys;

}


bool LoadGridInfo(FILE *fp)
{
    long iPoly, iGrid, iInstPoly, index;
    int nextWorldPoly;
    long nInstPolys;
    int xCount;
    REAL x1, z1;
    BBOX bBox;
    //INSTANCE *instance;
    //int instListSize;
    //int instList[MAX_INSTANCES];

    int nEmptyGrids = 0;

    
    // Read grid header
    if ((fp == NULL) || (fread(&COL_CollGridData, sizeof(COLLGRID_DATA), 1, fp) < 1)) 
    {
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
        COL_CollGrid[0].CollPolyIndices = CreateCollPolyIndices(COL_CollGrid[0].NCollPolys);

        if (COL_CollGrid[0].CollPolyIndices == NULL) {
            DestroyCollGrids();
            DestroyCollPolys(COL_WorldCollPoly);
            COL_WorldCollPoly = NULL;
            COL_NCollGrids = 0;
            COL_NWorldCollPolys = 0;
            return FALSE;
        }

        for (iPoly = 0; iPoly < COL_NWorldCollPolys; iPoly++) {
            COL_CollGrid[0].CollPolyIndices[iPoly] = (short)iPoly;
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


    

        // See if any of the instance polys fall within this grid location and keep note of which ones
#if 0
        instListSize = 0;
        nInstPolys = 0;

        SetBBox(&bBox, x1, x1 + COL_CollGridData.GridSize, -LARGEDIST, LARGEDIST, z1, z1 + COL_CollGridData.GridSize);
        for (iInst = 0; iInst < InstanceNum; iInst++) {

            if (!Instances[iInst].Priority && !RenderSettings.Instance)
                continue;   // skip if turned off

            if (BBTestXZY(&bBox, (BBOX *)&Instances[iInst].Box)) {
                instList[instListSize++] = iInst;
                nInstPolys += Instances[iInst].NCollPolys;
            }
        }
#else
        nInstPolys = 0;
        SetBBox(&bBox, x1 - COLLGRID_EXPAND, x1 + COL_CollGridData.GridSize + COLLGRID_EXPAND, -LARGEDIST, LARGEDIST, z1 - COLLGRID_EXPAND, z1 + COL_CollGridData.GridSize + COLLGRID_EXPAND);
        for (iInstPoly = 0; iInstPoly < COL_NInstanceCollPolys; iInstPoly++) {

            if (BBTestXZY(&COL_InstanceCollPoly[iInstPoly].BBox, &bBox)) {
                nInstPolys++;
            }

        }
#endif

        // Allocate space for the pointers if necessary
        if (COL_CollGrid[iGrid].NCollPolys + nInstPolys == 0) {
            COL_CollGrid[iGrid].CollPolyIndices = NULL;
            COL_CollGrid[iGrid].NWorldPolys = 0;

            nEmptyGrids++;
//          continue;
        } else {
            COL_CollGrid[iGrid].CollPolyIndices = CreateCollPolyIndices(COL_CollGrid[iGrid].NCollPolys + nInstPolys);
            if (COL_CollGrid[iGrid].CollPolyIndices == NULL) {
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
            COL_CollGrid[iGrid].CollPolyIndices[iPoly] = (short)index;
        }




#if 0
        // Now add the instance polys at the end
        nextWorldPoly = COL_CollGrid[iGrid].NCollPolys;
        for (iInst = 0; iInst < instListSize; iInst++) {
            instance = &Instances[instList[iInst]];

            if (!instance->Priority && !RenderSettings.Instance)
                continue;   // skip if turned off

            for (iPoly = 0; iPoly < instance->NCollPolys; iPoly++) {
                COL_CollGrid[iGrid].CollPolyIndices[nextWorldPoly++] = //&instance->CollPoly[iPoly];
                    (instance->CollPoly - COL_InstanceCollPoly) + iPoly + COL_NWorldCollPolys;
            }
        }
        COL_CollGrid[iGrid].NWorldPolys = COL_CollGrid[iGrid].NCollPolys;
        COL_CollGrid[iGrid].NCollPolys += nInstPolys;
#else
        nextWorldPoly = COL_CollGrid[iGrid].NCollPolys;
        //SetBBox(&bBox, x1, x1 + COL_CollGridData.GridSize, -LARGEDIST, LARGEDIST, z1, z1 + COL_CollGridData.GridSize);
        for (iInstPoly = 0; iInstPoly < COL_NInstanceCollPolys; iInstPoly++) {

            if (BBTestXZY(&COL_InstanceCollPoly[iInstPoly].BBox, &bBox)) {
                COL_CollGrid[iGrid].CollPolyIndices[nextWorldPoly++] = (short)(COL_NWorldCollPolys + iInstPoly);
            }

        }
        COL_CollGrid[iGrid].NWorldPolys = COL_CollGrid[iGrid].NCollPolys;
        COL_CollGrid[iGrid].NCollPolys += nInstPolys;
#endif

        // Sort index array and set info bits for each poly
        InitCollGrid(&COL_CollGrid[iGrid]);

        // Move to next grid
        (++xCount) %= NearestInt(COL_CollGridData.XNum);
        if (xCount == 0) {
            x1 = COL_CollGridData.XStart;
            z1 += COL_CollGridData.GridSize;
        } else {
            x1 += COL_CollGridData.GridSize;
        }

    }


#if FALSE //def _PC
    char buf[256];
    sprintf(buf, "%d of %d empty grids\n", nEmptyGrids, COL_NCollGrids);
    DumpMessage("Hey, You");
#endif

    return TRUE;
}
#endif


////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
//
// PSX Version
//
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////

#if defined(_PSX)

#if DEBUG
#define HOW_BIG_IS_COLLISION
int     CollisionSize = 0;
#endif

/*
    LEVEL_NEIGHBOURHOOD1,
    LEVEL_SUPERMARKET2,
    LEVEL_MUSEUM2,
    LEVEL_GARDEN1,
    LEVEL_TOYWORLD1,
    LEVEL_GHOSTTOWN1,
    LEVEL_TOYWORLD2,
    LEVEL_NEIGHBOURHOOD2,
    LEVEL_CRUISE1,
    LEVEL_MUSEUM1,
    LEVEL_SUPERMARKET1,
    LEVEL_GHOSTTOWN2,
    LEVEL_CRUISE2,
*/

int     LevelCollSize[] = {
    236144,     // NEI1   
    212906,     // SUP2
    207856,     // MUS2
    185190,     // BOT1
    272508,     // TOY1
    215438,     // WWW1 
    208124,     // TOY2
    220270,     // NEI2
    178884,     // CRU1 
    224000,     // MUS1                       
    204620,     // SUP1 
    185930,     // WWW2
    178884,     // CRU2
    7414,       // FRON
    9999,       // INTR
    224584,     // STUN
    170744,     // BATT
    9999,       // EDIT
};


NEWCOLLPOLY *LoadNewCollPolys(FILE *fp, short *nPolys)
{
    NEWCOLLPOLYHDR  header;
    NEWCOLLPOLY     *polys;
    size_t          nRead;
    short           iPoly;

    // read the header
    nRead = fread(&header, sizeof(NEWCOLLPOLYHDR), 1, fp);

    if( nRead < 1 )
        return NULL;

    // Allocate space for all collision data...
    if( !(polys = (NEWCOLLPOLY*)(malloc( LevelCollSize[ GameSettings.Level ] ))) )
    {
        printf( "Couldn't allocate space for collision data.\n" );
        exit( -1 );
    }

#ifdef HOW_BIG_IS_COLLISION
    CollisionSize = sizeof( NEWCOLLPOLY ) * header.NPolys;
#endif

    // Load in the poly info
    NonPlanarCount = 0;
    QuadCollCount = 0;
    TriCollCount = 0;
    for (iPoly = 0; iPoly < header.NPolys; iPoly++)
    {
        nRead = fread(&polys[iPoly], sizeof(NEWCOLLPOLY), 1, fp);

        if (nRead < 1)
        {
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
        if (IsPolyQuad(&polys[iPoly])) 
            QuadCollCount++;
        else
            TriCollCount++;
    }

    // Success!
    *nPolys = header.NPolys;

    return polys;
}


bool LoadGridInfo( FILE *fp )
{
    short       iPoly, iGrid, index;
    short       *poly_buffer;

    // Read grid header
    if ((fp == NULL) || (fread(&COL_CollGridData, sizeof(COLLGRID_DATA), 1, fp) < 1)) 
    {
        printf( "No Collision grids!!!\n" );
        // No grid data, so set up one grid system
        COL_CollGridData.XStart     = ZERO;
        COL_CollGridData.ZStart     = ZERO;
        COL_CollGridData.XNum       = ZERO;
        COL_CollGridData.ZNum       = ZERO;
        COL_CollGridData.GridSize   = LARGEDIST;

        COL_NCollGrids  = 1;
        COL_CollGrid    = (COLLGRID*)&COL_WorldCollPoly[ COL_NWorldCollPolys ];

        COL_CollGrid[0].NCollPolys      = COL_NWorldCollPolys;
        COL_CollGrid[0].NWorldPolys     = COL_CollGrid[0].NCollPolys;
        COL_CollGrid[0].CollPolyIndices = (short*)&COL_CollGrid[ COL_NCollGrids ];

        for (iPoly = 0; iPoly < COL_NWorldCollPolys; iPoly++)
            COL_CollGrid[0].CollPolyIndices[iPoly] = (short)iPoly;

        return FALSE;
    }
    

    #if DEBUG
       printf( "Coll Grids: %d * %d = %d\n", NearestInt(COL_CollGridData.XNum), NearestInt(COL_CollGridData.ZNum), NearestInt(COL_CollGridData.XNum) * NearestInt(COL_CollGridData.ZNum) );
    #endif

    // Grid data exists, so read it all in
    COL_NCollGrids = NearestInt(COL_CollGridData.XNum) * NearestInt(COL_CollGridData.ZNum);

    COL_CollGrid = (COLLGRID*)&COL_WorldCollPoly[ COL_NWorldCollPolys ];
    for( iGrid=0 ; iGrid<COL_NCollGrids ; iGrid++ )
    {
        COL_CollGrid[iGrid].NCollPolys = 0;
        COL_CollGrid[iGrid].CollPolyIndices = NULL;
    }
    poly_buffer = (short*)&COL_CollGrid[ COL_NCollGrids ];

#ifdef HOW_BIG_IS_COLLISION
    CollisionSize += sizeof( COLLGRID ) * COL_NCollGrids;
#endif

    // Read in pointer list for each grid location
    for (iGrid = 0; iGrid < COL_NCollGrids; iGrid++)
    {
        // Get number of polys in this grid volume
        if (fread(&COL_CollGrid[iGrid].NCollPolys, sizeof(COL_CollGrid[iGrid].NCollPolys), 1, fp) < 1) 
        {
            printf( "FUCK!!!\n" );
            free( COL_WorldCollPoly );
//          DestroyCollGrids();
//          DestroyCollPolys(COL_WorldCollPoly);
            COL_WorldCollPoly = NULL;
            COL_NCollGrids = 0;
            COL_NWorldCollPolys = 0;
            return FALSE;
        }

        // Allocate space for the pointers if necessary
        if (COL_CollGrid[iGrid].NCollPolys == 0)
        {
            COL_CollGrid[iGrid].CollPolyIndices = NULL;
            COL_CollGrid[iGrid].NWorldPolys = 0;
            continue;
        }
        else
        {
            COL_CollGrid[iGrid].CollPolyIndices = poly_buffer;
            poly_buffer += COL_CollGrid[iGrid].NCollPolys;
        }
        
#ifdef HOW_BIG_IS_COLLISION
        CollisionSize += sizeof( short ) * COL_CollGrid[iGrid].NCollPolys;
#endif

        // Fill the pointer array with pointers to polys in grid volume
        for (iPoly = 0; iPoly < COL_CollGrid[iGrid].NCollPolys; iPoly++)
        {
            if (fread(&index, sizeof(index), 1, fp) < 1)
            {
                printf( "FUCK 3 !!!!\n" );
                DestroyCollGrids();
                DestroyCollPolys(COL_WorldCollPoly);
                COL_WorldCollPoly = NULL;
                COL_NCollGrids = 0;
                COL_NWorldCollPolys = 0;
                return FALSE;
            }
            COL_CollGrid[iGrid].CollPolyIndices[iPoly] = (short)index;
        }

        COL_CollGrid[iGrid].NWorldPolys = COL_CollGrid[iGrid].NCollPolys;

        // Sort index array and set info bits for each poly
        InitCollGrid(&COL_CollGrid[iGrid]);
    }

#if DEBUG
    if( CollisionSize > LevelCollSize[ GameSettings.Level ] )
    {
        printf( "\n*****\n" );
        printf( "You need to increase the amount of space for collision on this level.\n" );
        printf( " Space needed = %d.\n", CollisionSize );
        printf( " Space currently allocated = %d.\n", LevelCollSize[ GameSettings.Level ] );
        printf( "Change 'LevelCollSize[ %d ]' in 'newcoll.cpp'.\n", GameSettings.Level );
        printf( "*****\n" );
        exit( -1 );
    }
#ifdef HOW_BIG_IS_COLLISION
    printf( " CollisionSize = %d : %d allocated\n", CollisionSize, LevelCollSize[ GameSettings.Level ] );
#endif
#endif
    
    return TRUE;
}
#endif

////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
//
// N64 Version
//
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////

#ifdef _N64
NEWCOLLPOLY *LoadNewCollPolys(FIL *Fil, short *nPolys)
{
    NEWCOLLPOLYHDR header;
    NEWCOLLPOLY * polys;
    size_t nRead;
    long    ii;

    // printf("Loading new collision data...\n");

    nRead = FFS_Read((char *)&header, sizeof(NEWCOLLPOLYHDR), Fil);     // read the header
    if (nRead < sizeof(NEWCOLLPOLYHDR))
    {
        return NULL;
    }
    // printf("...number of collison polys is %d\n", header.NPolys);

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
    // printf("...collison polys use %d bytes\n", sizeof(NEWCOLLPOLY) * header.NPolys);

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
    int iPoly, iGrid, iInstPoly, index;
    int nextWorldPoly;
    long nInstPolys;
    int xCount;
    REAL x1, z1;
    BBOX bBox;
    long nGridPolys;
    //int instListSize;
    //int instList[MAX_INSTANCES];
    //INSTANCE *instance;

    printf("Loading collison data grids...\n");
    // Read grid header
    if (FFS_Read((char *)&COL_CollGridData, sizeof(COLLGRID_DATA), Fil) < sizeof(COLLGRID_DATA))
    {
        // No grid data, so set up one grid system
        printf("WARNING: No grid data found, creating grid system.\n");
        COL_CollGridData.XStart = ZERO;
        COL_CollGridData.ZStart = ZERO;
        COL_CollGridData.XNum = ZERO;
        COL_CollGridData.ZNum = ZERO;
        COL_CollGridData.GridSize = LARGEDIST;

        COL_NCollGrids = 1;
        if ((COL_CollGrid = CreateCollGrids(COL_NCollGrids)) == NULL) 
        {
            printf("WARNING: collison grids could not be created!\n");
            DestroyCollPolys(COL_WorldCollPoly);
            COL_NWorldCollPolys = 0;
            return FALSE;
        }
        COL_CollGrid[0].NCollPolys  = COL_NWorldCollPolys;
        COL_CollGrid[0].CollPolyIndices = CreateCollPolyIndices(COL_CollGrid[0].NCollPolys);
        if (COL_CollGrid[0].CollPolyIndices == NULL)
        {
            printf("WARNING: collison poly indices could not be created!\n");
            DestroyCollGrids();
            DestroyCollPolys(COL_WorldCollPoly);
            COL_NCollGrids = 0;
            COL_NWorldCollPolys = 0;
            return FALSE;
        }
        for (iPoly = 0; iPoly < COL_NWorldCollPolys; iPoly++)
        {
            COL_CollGrid[0].CollPolyIndices[iPoly] = iPoly;
        }
        return FALSE;
    }

    // Grid data exists, so read it all in
    COL_NCollGrids = NearestInt(COL_CollGridData.XNum) * NearestInt(COL_CollGridData.ZNum);
    log_printf("...found %d collison grids (%1.0f * %1.0f).\n", COL_NCollGrids, COL_CollGridData.XNum, COL_CollGridData.ZNum);
    if ((COL_CollGrid = CreateCollGrids(COL_NCollGrids)) == NULL)
    {
        printf("WARNING: collison grids could not be created!\n");
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
        if (FFS_Read((char *)&nGridPolys, sizeof(long), Fil) < 1)
        {
            DestroyCollGrids();
            DestroyCollPolys(COL_WorldCollPoly);
            COL_NCollGrids = 0;
            COL_NWorldCollPolys = 0;
            return FALSE;
        }
        COL_CollGrid[iGrid].NCollPolys = (short)nGridPolys;

        // See if any of the instances fall within this grid location and keep note of which ones
#if 0
        instListSize = 0;
        nInstPolys = 0;
        SetBBox(&bBox, x1, x1 + COL_CollGridData.GridSize, -LARGEDIST, LARGEDIST, z1, z1 + COL_CollGridData.GridSize);
        for (iInst = 0; iInst < InstanceNum; iInst++)
        {
            if (BBTestXZY(&bBox, (BBOX *)&Instances[iInst].Box))
            {
                instList[instListSize++] = iInst;
                nInstPolys += Instances[iInst].NCollPolys;
            }
        }
#else
        nInstPolys = 0;
        SetBBox(&bBox, x1 - COLLGRID_EXPAND, x1 + COL_CollGridData.GridSize + COLLGRID_EXPAND, -LARGEDIST, LARGEDIST, z1 - COLLGRID_EXPAND, z1 + COL_CollGridData.GridSize + COLLGRID_EXPAND);
        for (iInstPoly = 0; iInstPoly < COL_NInstanceCollPolys; iInstPoly++) {

            if (BBTestXZY(&COL_InstanceCollPoly[iInstPoly].BBox, &bBox)) {
                nInstPolys++;
            }

        }
#endif

        // Allocate space for the pointers if necessary
        if (COL_CollGrid[iGrid].NCollPolys + nInstPolys == 0)
        {
            COL_CollGrid[iGrid].CollPolyIndices = NULL;
            COL_CollGrid[iGrid].NWorldPolys = 0;
        }
        else
        {
            COL_CollGrid[iGrid].CollPolyIndices = CreateCollPolyIndices(COL_CollGrid[iGrid].NCollPolys + nInstPolys);
            if (COL_CollGrid[iGrid].CollPolyIndices == NULL)
            {
                printf("WARNING: collison poly ptrs could not be created!\n");
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
                printf("WARNING: failed to read from collsion file.\n");
                DestroyCollGrids();
                DestroyCollPolys(COL_WorldCollPoly);
                COL_NCollGrids = 0;
                COL_NWorldCollPolys = 0;
                return FALSE;
            }
            COL_CollGrid[iGrid].CollPolyIndices[iPoly] = (short)index;
        }

#if 0
        nextWorldPoly = COL_CollGrid[iGrid].NCollPolys;
        if (nInstPolys > 0) {
            for (iInst = 0; iInst < instListSize; iInst++)
            {
                instance = &Instances[instList[iInst]];
                for (iPoly = 0; iPoly < instance->NCollPolys; iPoly++)
                {
                    // Indices >= COL_NWorldCollPolys are taken from the instance collision poly array
                    COL_CollGrid[iGrid].CollPolyIndices[nextWorldPoly++] = (instance->CollPoly - COL_InstanceCollPoly) + iPoly + COL_NWorldCollPolys;
                }
            }
        }
        COL_CollGrid[iGrid].NWorldPolys = COL_CollGrid[iGrid].NCollPolys;
        COL_CollGrid[iGrid].NCollPolys += nInstPolys;
#else
        nextWorldPoly = COL_CollGrid[iGrid].NCollPolys;
        for (iInstPoly = 0; iInstPoly < COL_NInstanceCollPolys; iInstPoly++) {

            if (BBTestXZY(&COL_InstanceCollPoly[iInstPoly].BBox, &bBox)) {
                COL_CollGrid[iGrid].CollPolyIndices[nextWorldPoly++] = COL_NWorldCollPolys + iInstPoly;
            }

        }
        COL_CollGrid[iGrid].NWorldPolys = COL_CollGrid[iGrid].NCollPolys;
        COL_CollGrid[iGrid].NCollPolys += nInstPolys;
#endif

        // Sort index array and set info bits for each poly
        InitCollGrid(&COL_CollGrid[iGrid]);

        // Next grid location
        ++xCount;
        xCount %= NearestInt(COL_CollGridData.XNum);
        if (xCount == 0)
        {
            x1 = COL_CollGridData.XStart;
            z1 += COL_CollGridData.GridSize;
        }
        else
        {
            x1 += COL_CollGridData.GridSize;
        }

    }
    return TRUE;
}
#endif


////////////////////////////////////////////////////////////////
//
// GetNearGrids: get the grid locations for an object at the given
// position. Return index of grid car is in.
//
// grids is an array of 4 pointers to collision grids
//
////////////////////////////////////////////////////////////////
#if GRIDIFY_OBJECTS
long GetNearGrids(VEC *pos, COLLGRID **grids)
{
    long    gridIndex, offsetX, offsetZ, xNum, zNum;
    int     ii, jj, kk;
    REAL    dX, dZ;

    kk = 0;

    // Make sure the world is gridded (if not, world is one large grid)
    if ((COL_CollGridData.XNum == ZERO) && (COL_CollGridData.ZNum == ZERO)) {
        grids[0] = &COL_CollGrid[0];
        grids[1] = NULL;
        grids[2] = NULL;
        grids[3] = NULL;
        return 0;
    }

    // Get the grid dimensions
    xNum = NearestInt(COL_CollGridData.XNum);
    zNum = NearestInt(COL_CollGridData.ZNum);

    // Calculate grid index
    dX = pos->v[X] - COL_CollGridData.XStart;
    dZ = pos->v[Z] - COL_CollGridData.ZStart;
    offsetX = Int(DivScalar(dX, COL_CollGridData.GridSize));
    offsetZ = Int(DivScalar(dZ, COL_CollGridData.GridSize));

    Assert((offsetX + xNum * offsetZ) < xNum * zNum);
    gridIndex = offsetX + xNum * offsetZ;

    // Get position of object relative to grid centre
    dX -= offsetX * COL_CollGridData.GridSize + COL_CollGridData.GridSize/2;
    dZ -= offsetZ * COL_CollGridData.GridSize + COL_CollGridData.GridSize/2;

    // Make sure we are not outside the grid boundaries
    if ((offsetX < 0L) || (offsetX >= xNum) ||
        (offsetZ < 0L) || (offsetZ >= zNum))
    {
        grids[0] = grids[1] = grids[2] = grids[3] = NULL;
        return -1;
    }

    // Find top-left of square of grids that object must check against
    if (dX < ZERO) {
        offsetX--;
    }
    if (dZ < ZERO) {
        offsetZ--;
    }

    // Fill in the near-grid array
    kk = 0;
    for (ii = 0; ii < 2; ii++) {
        if ((offsetX < 0) || (offsetX >= xNum)) {
            offsetX++;
            continue;
        }

        for (jj = 0; jj < 2; jj++) {
            if ((offsetZ < 0) || (offsetZ >= zNum)) {
                offsetZ++;
                continue;
            }

            Assert((offsetX + xNum * offsetZ) < xNum * zNum);
            grids[kk++] = &COL_CollGrid[offsetX + xNum * (offsetZ++)];

        }
        offsetZ -= 2;
        offsetX++;
    }

    // Fill in remaining grid array
    while (kk < 4) grids[kk++] = NULL;

    // Done
    return gridIndex;

}
#endif

////////////////////////////////////////////////////////////////
//
// Add an object to a grid list (remove it from previous list
// if necessary)
//
////////////////////////////////////////////////////////////////
void UpdateObjectGrid(OBJECT *obj)
{
#if GRIDIFY_OBJECTS
    long        oldGrid = obj->GridIndex;
    COLLGRID    *grid = obj->Grids[0];

    obj->GridIndex = GetNearGrids(&obj->body.Centre.Pos, obj->Grids);

    // Don't bother if we haven't moved
    if (oldGrid == obj->GridIndex) return;

    // Remove object from grid object list it was in
    if (oldGrid != -1) {
        RemoveObjectFromGrid(obj, &COL_CollGrid[oldGrid]);
    }

    // Add object to its new grid location
    if (obj->GridIndex != -1) {
        AddObjectToGrid(obj, &COL_CollGrid[obj->GridIndex]);
    }

#endif
}


////////////////////////////////////////////////////////////////
//
// Add Object to a grid list
//
////////////////////////////////////////////////////////////////

void AddObjectToGrid(OBJECT *obj, COLLGRID *grid)
{
#if GRIDIFY_OBJECTS
    OBJECT *oldHead = grid->ObjectHead;

    grid->ObjectHead = obj;

    obj->PrevGridObj = NULL;
    obj->NextGridObj = oldHead;

    if (oldHead != NULL) {
        oldHead->PrevGridObj = obj;
    }
#endif
}


////////////////////////////////////////////////////////////////
//
// Remove an object from a grid list
//
////////////////////////////////////////////////////////////////

void RemoveObjectFromGrid(OBJECT *obj, COLLGRID *grid)
{
#if GRIDIFY_OBJECTS
    if (obj->PrevGridObj != NULL) {
        (obj->PrevGridObj)->NextGridObj = obj->NextGridObj;
    } else {
        if (grid != NULL) {
            (grid)->ObjectHead = obj->NextGridObj;
        }
    }

    if (obj->NextGridObj != NULL) {
        (obj->NextGridObj)->PrevGridObj = obj->PrevGridObj;
    }

    obj->PrevGridObj = obj->NextGridObj = NULL;
#endif
}

////////////////////////////////////////////////////////////////
//
// Sort and setup collision grid indices:
//
// Index array is sorted in order of min y extent 
// (i.e. highest point) of the collision polys.
//
// Bits are set to say whether each polygon overlaps any above
// or below it.
//
// A bisection-type search can then be used to decide which
// range of indices to check against
//
// Assumes the max number of collision polys is 8191 and that
// the grid has not been previously processed with this function.
//
////////////////////////////////////////////////////////////////

void InitCollGrid(COLLGRID *grid)
{
    register int ii, jj;
    short tIndex;
    NEWCOLLPOLY *poly1, *poly2;

    // Sort the indices according the to y extents (vertically highest first)
    for (ii = 1; ii < grid->NCollPolys; ii++) {
        for (jj = grid->NCollPolys - 1; jj >= ii; jj--) {

            // Get pointers to the two currently adjacent polys
            poly2 = GetCollPoly(grid->CollPolyIndices[jj]);
            poly1 = GetCollPoly(grid->CollPolyIndices[jj - 1]);

            if (poly1->BBox.YMin > poly2->BBox.YMin) {
                // Not in order, so swap
                tIndex = grid->CollPolyIndices[jj];
                grid->CollPolyIndices[jj] = grid->CollPolyIndices[jj - 1];
                grid->CollPolyIndices[jj - 1] = tIndex;
            }

        }
    }

    // set the status flags for each index
    for (ii = 0; ii < grid->NCollPolys - 1; ii++) {
        poly1 = GetCollPoly(grid->CollPolyIndices[ii]);

        for (jj = ii + 1; jj < grid->NCollPolys; jj++) {
            poly2 = GetCollPoly(grid->CollPolyIndices[jj]);

            // Check for overlap
            if (poly1->BBox.YMax > poly2->BBox.YMin) {
                // Does overlap, so set the flag
                grid->CollPolyIndices[ii] |= GRIDINDEX_OVERLAPS_BELOW;
                grid->CollPolyIndices[jj] |= GRIDINDEX_OVERLAPPED_ABOVE;
            }

        }
    }

#if GRIDIFY_OBJECTS
    // Clear the object flag
    grid->ObjectHead = NULL;
#endif
}


////////////////////////////////////////////////////////////////
//
// Given the y extent of an object (yMin, yMax) fill start and
// end with the range of indices that need to be examined
// closer in the Collision grid
//
////////////////////////////////////////////////////////////////

void GetGridIndexRange(COLLGRID *grid, REAL yMin, REAL yMax, long *start, long *end)
{
    register long e, eMin, eMax;
    register long s, sMin, sMax;
    NEWCOLLPOLY *poly;

    // don't bother with <= 4 polys
    if (grid->NCollPolys < 5) {
        *start = 0;
        *end = grid->NCollPolys;
        return;
    }

    ////////////////////////////////////////////////////////////////
    // Find the end point
    //
    eMin = 0;
    eMax = grid->NCollPolys;

    while ((eMax - eMin) > 1) {
        // Mid point of current search area
        e = (eMax + eMin) >> 1;

        // Get the poly
        poly = GetCollPoly(grid->CollPolyIndices[e]);

        // is this below the object?
        if (poly->BBox.YMin > yMax) {
            // Yes, so can ignore all indices above and including this
            eMax = e;
        } else {
            // No, reduce search area
            eMin = e;
        }

    }
    *end = eMax;

    ////////////////////////////////////////////////////////////////
    // Find the start point
    //
    sMin = 0;
    sMax = eMax;

    while ((sMax - sMin) > 1) {
        // Mid point of current search area
        s = (sMax + sMin) >> 1;

        // Get the poly
        poly = GetCollPoly(grid->CollPolyIndices[s]);

        // is this above the object?
        if (poly->BBox.YMax < yMin) {
            // Yes, so might be able to ignore all indices above and including this
            sMin = s;
        } else {
            // No, reduce search area
            sMax = s;
        }

    }
    
    s = sMin;
    while (grid->CollPolyIndices[s] & GRIDINDEX_OVERLAPPED_ABOVE) {
        s--;
    }
    *start = s;

}


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

#if defined(_PC)
static REAL dist[4];
static bool inside[4];
bool SphereCollPoly(VEC *oldPos, VEC *newPos, REAL radius, NEWCOLLPOLY *collPoly, PLANE *plane, VEC *relPos, VEC *worldPos, REAL *depth, REAL *time)
{
    REAL    oldDist, newDist, dLen;
    int     nSides, outCount, i;

    // Make sure sphere is within "radius" of the plane of the polygon or on the inside
    newDist = VecDotPlane(newPos, &collPoly->Plane);
    if (newDist - radius > COLL_EPSILON) return FALSE;

    // If we were inside the plane last time, probably no collision
    oldDist = VecDotPlane(oldPos, &collPoly->Plane);
    if (oldDist < -(radius + COLL_EPSILON)) return FALSE;
    //if (oldDist < ZERO) return FALSE;
    //if (oldDist < newDist) return FALSE;

    // Count the faces that the sphere centre is outside of, and store distances
    outCount = 0;
    if (IsPolyQuad(collPoly)) {
        nSides = 4;
    } else {
        nSides = 3;
    }
    for (i = 0; i < nSides; i++) {
        inside[i] = ((dist[i] = VecDotPlane(newPos, &collPoly->EdgePlane[i])) < 0);
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
            *time = DivScalar(oldDist - newDist + *depth, (oldDist - newDist));
            if (*time > ONE) *time = ONE;
            if (*time < ZERO) *time = ZERO;
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

#elif defined(_N64)

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
    //ScalarVecPlusScalarVec((ONE - *time), oldPos, *time, newPos, worldPos);
    //VecPlusEqScalarVec(worldPos, -radius, PlaneNormal(&collPoly->Plane));
    VecPlusScalarVec(newPos, -newDist, PlaneNormal(&collPoly->Plane), worldPos);

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
// time < 0     == no collision
// time = 0     == collision at lStart;
// 0 < time < 1 == collision between lStart and lEnd;
// time = 1     == collision at lEnd;
//
/////////////////////////////////////////////////////////////////////

bool LinePlaneIntersect(VEC *lStart, VEC *lEnd, PLANE *plane, REAL *t, REAL *depth)
{
    REAL    startDotPlane;
    REAL    endDotPlane;

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

    //if (ApproxEqual(startDotPlane, endDotPlane)) {
    if (abs(startDotPlane - endDotPlane) < 1) {
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
    REAL    startDotPlane;
    REAL    dirDotPlane;

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

    DestroyCollPolys(collSkin->WorldCollPoly);
    collSkin->WorldCollPoly = NULL;
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
        skin[iSkin].NFaces = 0;
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

bool SetupConvex(CONVEX *skin, INDEX nFaces) 
{
    // Make sure the numbers are valid
    Assert(nFaces > 0);

    // Create space for the Face vertex list
    if (nFaces != 0) {
        if ((skin->Faces = (PLANE *)malloc(sizeof(PLANE) * nFaces)) == NULL) {
            return FALSE;
        }
    }

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
#ifndef _N64
            DestroyConvex(collSkin->WorldConvex, collSkin->NConvex);
#endif
            return FALSE;
        }
    }

    // Allocate space for the collision spheres
    if (collSkin->NSpheres > 0) {
        collSkin->WorldSphere = (SPHERE *)malloc(sizeof(SPHERE) * collSkin->NSpheres);
        if (collSkin->WorldSphere == NULL) {
#ifndef _N64
            DestroyConvex(collSkin->OldWorldConvex, collSkin->NConvex);
            DestroyConvex(collSkin->WorldConvex, collSkin->NConvex);
#endif
            return FALSE;
        }
        collSkin->OldWorldSphere = (SPHERE *)malloc(sizeof(SPHERE) * collSkin->NSpheres);
        if (collSkin->OldWorldSphere == NULL) {
#ifndef _N64
            DestroyConvex(collSkin->OldWorldConvex, collSkin->NConvex);
            DestroyConvex(collSkin->WorldConvex, collSkin->NConvex);
            DestroySpheres(collSkin->WorldSphere);
#endif
            return FALSE;
        }
    }

    // Allocate space for all points, edges and planes for each hull, and copy info across
    for (iSkin = 0; iSkin < collSkin->NConvex; iSkin++) {

        // Create space for all data
        if (!SetupConvex(&collSkin->WorldConvex[iSkin], collSkin->Convex[iSkin].NFaces)) {
#ifndef _N64
            DestroyConvex(collSkin->OldWorldConvex, collSkin->NConvex);
            DestroyConvex(collSkin->WorldConvex, collSkin->NConvex);
            DestroySpheres(collSkin->WorldSphere);
            DestroySpheres(collSkin->OldWorldSphere);
#endif
            return FALSE;
        }
        collSkin->WorldConvex[iSkin].NFaces = collSkin->Convex[iSkin].NFaces;

        if (!SetupConvex(&collSkin->OldWorldConvex[iSkin], collSkin->Convex[iSkin].NFaces)) {
#ifndef _N64
            DestroyConvex(collSkin->OldWorldConvex, collSkin->NConvex);
            DestroyConvex(collSkin->WorldConvex, collSkin->NConvex);
            DestroySpheres(collSkin->OldWorldSphere);
            DestroySpheres(collSkin->WorldSphere);
#endif
            return FALSE;
        }
        collSkin->OldWorldConvex[iSkin].NFaces = collSkin->Convex[iSkin].NFaces;

        // Bounding box
        CopyBBox(&collSkin->Convex[iSkin].BBox, &collSkin->WorldConvex[iSkin].BBox);
        CopyBBox(&collSkin->Convex[iSkin].BBox, &collSkin->OldWorldConvex[iSkin].BBox);

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


    // Create copy of collision polygons if there are any
    if (collSkin->NCollPolys != 0) {
        collSkin->WorldCollPoly = (NEWCOLLPOLY*)malloc(sizeof(NEWCOLLPOLY) * collSkin->NCollPolys);
        if (collSkin->WorldCollPoly != NULL) {
            for (iSkin = 0; iSkin < collSkin->NCollPolys; iSkin++) {
                CopyCollPoly(&collSkin->CollPoly[iSkin], &collSkin->WorldCollPoly[iSkin]);
            }
        } else {
            collSkin->NCollPolys = 0;
        }
    }


    return TRUE;
}


////////////////////////////////////////////////////////////////
//
// CopyCollPoly:
//
////////////////////////////////////////////////////////////////
#ifndef _PSX
void CopyCollPoly(NEWCOLLPOLY *src, NEWCOLLPOLY *dest)
{
    dest->Type = src->Type;
    dest->Material = src->Material;
    CopyPlane(&src->Plane, &dest->Plane);
    CopyPlane(&src->EdgePlane[0], &dest->EdgePlane[0]);
    CopyPlane(&src->EdgePlane[1], &dest->EdgePlane[1]);
    CopyPlane(&src->EdgePlane[2], &dest->EdgePlane[2]);
    CopyPlane(&src->EdgePlane[3], &dest->EdgePlane[3]);
    CopyBBox(&src->BBox, &dest->BBox);  
}
#else
void CopyCollPoly(NEWCOLLPOLY *src, NEWCOLLPOLY *dest)
{
    dest->Type = src->Type;
    dest->Material = src->Material;
    CopyPlane(&src->Plane, &dest->Plane);
    CopyVec(&src->Vertex[0], &dest->Vertex[0]);
    CopyVec(&src->Vertex[1], &dest->Vertex[1]);
    CopyVec(&src->Vertex[2], &dest->Vertex[2]);
    CopyVec(&src->Vertex[3], &dest->Vertex[3]);
    CopyBBox(&src->BBox, &dest->BBox);  
}
#endif


#ifndef _PSX

////////////////////////////////////////////////////////////////
//
// InitWorldSkin: create the world collision skin without
// using the old position to calculate the swept bounding box
//
////////////////////////////////////////////////////////////////

void InitWorldSkin(COLLSKIN *bodySkin, VEC *pos, MAT *mat)
{
    int         iSkin, ii;
    CONVEX      *thisBodySkin, *thisWorldSkin;
    SPHERE      *thisBodySphere, *thisWorldSphere;

    Assert(bodySkin!=NULL);

    SetBBox(&bodySkin->BBox, LARGEDIST, -LARGEDIST, LARGEDIST, -LARGEDIST, LARGEDIST, -LARGEDIST);
    
    // Transform the convex hulls 
    for (iSkin = 0; iSkin < bodySkin->NConvex; iSkin++) {
        thisBodySkin = &bodySkin->Convex[iSkin];
        thisWorldSkin = &bodySkin->WorldConvex[iSkin];

        // Planes
        for (ii = 0; ii < thisBodySkin->NFaces; ii++) {
            RotTransPlane(&thisBodySkin->Faces[ii], mat, pos, &thisWorldSkin->Faces[ii]);
        }

    }

    // Transform the spheres
    for (iSkin = 0; iSkin < bodySkin->NSpheres; iSkin++) {
        thisBodySphere = &bodySkin->Sphere[iSkin];
        thisWorldSphere = &bodySkin->WorldSphere[iSkin];

        // Transfrom the position (radius should already be there)
        VecMulMat(&thisBodySphere->Pos, mat, &thisWorldSphere->Pos);
        VecPlusEqVec(&thisWorldSphere->Pos, pos);

        CopyVec(&thisWorldSphere->Pos, &bodySkin->OldWorldSphere[iSkin].Pos);
    
        AddPosRadToBBox(&bodySkin->BBox, &thisWorldSphere->Pos, thisWorldSphere->Radius);
    }

    // Transform the overall bounding box
    if (bodySkin->NConvex == 0) {
        RotTransBBox(&bodySkin->TightBBox, mat, pos, &bodySkin->BBox);
    }

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
    int         iSkin, ii;
    CONVEX      *thisBodySkin, *thisWorldSkin;
    SPHERE      *thisBodySphere, *thisWorldSphere;

    Assert(bodySkin!=NULL);

    SetBBox(&bodySkin->BBox, LARGEDIST, -LARGEDIST, LARGEDIST, -LARGEDIST, LARGEDIST, -LARGEDIST);
    
    // Transform the convex hulls 
    for (iSkin = 0; iSkin < bodySkin->NConvex; iSkin++) {
        thisBodySkin = &bodySkin->Convex[iSkin];
        thisWorldSkin = &bodySkin->WorldConvex[iSkin];

        // Planes
        for (ii = 0; ii < thisBodySkin->NFaces; ii++) {
            RotTransPlane(&thisBodySkin->Faces[ii], mat, pos, &thisWorldSkin->Faces[ii]);
        }

        // Bounding box
        //RotTransBBox(&thisBodySkin->BBox, mat, pos, &thisWorldSkin->BBox);
    }

    // Transform the spheres
    for (iSkin = 0; iSkin < bodySkin->NSpheres; iSkin++) {
        thisBodySphere = &bodySkin->Sphere[iSkin];
        thisWorldSphere = &bodySkin->WorldSphere[iSkin];

        AddPosRadToBBox(&bodySkin->BBox, &thisWorldSphere->Pos, thisWorldSphere->Radius);

        // Transfrom the position (radius should already be there)
        VecMulMat(&thisBodySphere->Pos, mat, &thisWorldSphere->Pos);
        VecPlusEqVec(&thisWorldSphere->Pos, pos);

        AddPosRadToBBox(&bodySkin->BBox, &thisWorldSphere->Pos, thisWorldSphere->Radius);
    
    }

    // Transform the overall bounding box
    if (bodySkin->NConvex == 0) {
        RotTransBBox(&bodySkin->TightBBox, mat, pos, &bodySkin->BBox);
    }

}

#else  // PSX Version

void InitWorldSkin(COLLSKIN *bodySkin, VEC *pos, MAT *mat)
{
}

void BuildWorldSkin(COLLSKIN *bodySkin, VEC *pos, MAT *mat)
{
    int         iSkin, ii;
    VEC     tmpVec;
    CONVEX      *thisBodySkin, *thisWorldSkin;
    SPHERE      *thisBodySphere, *thisWorldSphere;
    MATRIX PSXMat;
    SVECTOR SVector;
    
    PSXMatrix( mat, &PSXMat );
    PSXMat.t[0] = 0;
    PSXMat.t[1] = 0;
    PSXMat.t[2] = 0;

    Assert(bodySkin!=NULL);

    SetBBox(&bodySkin->BBox, LARGEDIST, -LARGEDIST, LARGEDIST, -LARGEDIST, LARGEDIST, -LARGEDIST);

    // Transform the convex hulls 
    for( iSkin = bodySkin->NConvex - 1; iSkin >= 0; iSkin--)
    {
        thisBodySkin = &bodySkin->Convex[iSkin];
        thisWorldSkin = &bodySkin->WorldConvex[iSkin];

        // Planes
        for (ii = 0; ii < thisBodySkin->NFaces; ii++)
        {
    
            // Note: Body-frame collision skin is already shifted down on PSX...
            SVector.vx = thisBodySkin->Faces[ii].v[0];// >> PSX_PLANE_SHIFT;
            SVector.vy = thisBodySkin->Faces[ii].v[1];// >> PSX_PLANE_SHIFT;
            SVector.vz = thisBodySkin->Faces[ii].v[2];// >> PSX_PLANE_SHIFT;

            gte_ApplyMatrix( &PSXMat, &SVector, &thisWorldSkin->Faces[ii] );

            thisWorldSkin->Faces[ii].v[0] <<= PSX_PLANE_SHIFT;
            thisWorldSkin->Faces[ii].v[1] <<= PSX_PLANE_SHIFT;
            thisWorldSkin->Faces[ii].v[2] <<= PSX_PLANE_SHIFT;

            thisWorldSkin->Faces[ii].v[D] = thisBodySkin->Faces[ii].v[D] - VecDotVec(pos, PlaneNormal(&thisWorldSkin->Faces[ii]));

        }

        // Bounding box
        //RotTransBBoxPSX(&thisBodySkin->BBox, &PSXMat, pos, &thisWorldSkin->BBox);
    }

    // Transform the spheres
    for ( iSkin = bodySkin->NSpheres - 1; iSkin >= 0;  iSkin--)
    {
        thisBodySphere = &bodySkin->Sphere[iSkin];
        thisWorldSphere = &bodySkin->WorldSphere[iSkin];

        // Transfrom the position (radius should already be there)
        // Note: Body-frame collision skin is already shifted down on PSX...

        SVector.vx = thisBodySphere->Pos.v[0];// >> PSX_BODYSPHERE_SHIFT;
        SVector.vy = thisBodySphere->Pos.v[1];// >> PSX_BODYSPHERE_SHIFT;
        SVector.vz = thisBodySphere->Pos.v[2];// >> PSX_BODYSPHERE_SHIFT;

        gte_ApplyMatrix( &PSXMat, &SVector, &thisWorldSphere->Pos );

        thisWorldSphere->Pos.v[0] <<= PSX_BODYSPHERE_SHIFT;
        thisWorldSphere->Pos.v[1] <<= PSX_BODYSPHERE_SHIFT;
        thisWorldSphere->Pos.v[2] <<= PSX_BODYSPHERE_SHIFT;

        VecPlusEqVec(&thisWorldSphere->Pos, pos);

        AddPosRadToBBox(&bodySkin->BBox, &thisWorldSphere->Pos, thisWorldSphere->Radius);
    }

    // Transform the overall bounding box
    //RotTransBBoxPSX(&bodySkin->TightBBox, &PSXMat, pos, &bodySkin->BBox);

}


#endif


/////////////////////////////////////////////////////////////////////
//
// LoadConvex: load a set of convex hulls for an object's
// collision skin
//
/////////////////////////////////////////////////////////////////////

#ifdef _PC
CONVEX *LoadConvex(FILE *fp, INDEX *nSkins)
{
    int iSkin, iEdge, iPt;
    VEC offset;

    COLLSKIN_FILEHDR    fileHdr;
    COLLSKIN_COLLHDR    collHdr;
    CONVEX  *collSkin;

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
        if (!SetupConvex(&collSkin[iSkin], collHdr.NFaces)) {
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

        // Offset  NOTE:// no longer used
        if (fread(&offset, sizeof(VEC), 1, fp) != 1) {
            DestroyConvex(collSkin, fileHdr.NSkins);
            *nSkins = 0;
            return NULL;
        }

        // Read in the vertices  NOTE:// No longer used, so discarded
        for (iPt = 0; iPt < collHdr.NVertices; iPt++) {
            fread(&offset, sizeof(VEC), 1, fp);
        }

        // Read in the edges  NOTE:// No longer used
        for (iEdge = 0; iEdge < collHdr.NEdges; iEdge++) {
            fread(&offset, sizeof(INDEX), 2, fp);
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
    INDEX   iSphere;
    SPHERE  *spheres;
    size_t  nRead;

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
CONVEX *LoadConvex(FILE *fp, INDEX *nSkins)
{
    int iSkin, iEdge, iPt;
    VEC offset;

    COLLSKIN_FILEHDR    fileHdr;
    COLLSKIN_COLLHDR    collHdr;
    CONVEX  *collSkin;

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

#if DEBUG
    printf("Loading %d Convex Hull:\n", fileHdr.NSkins);
#endif




    // Get the individual convex hulls
    for (iSkin = 0; iSkin < fileHdr.NSkins; iSkin++) {
        
        // Read size and allocate space
        if (fread(&collHdr, sizeof(COLLSKIN_COLLHDR), 1, fp) != 1) {
            DestroyConvex(collSkin, fileHdr.NSkins);
            *nSkins = 0;
            return NULL;
        }
    
#if DEBUG
        printf("Hull %d\n"
            "\t%d Points\n"
            "\t%d Faces\n",
            iSkin,
            collHdr.NVertices,
            collHdr.NFaces);
#endif


        
        // Allocate space for the data
        if (!SetupConvex(&collSkin[iSkin], collHdr.NFaces)) {
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




        // offset  NOTE:// no longer used
        if (fread(&offset, sizeof(VEC), 1, fp) != 1) {
            DestroyConvex(collSkin, fileHdr.NSkins);
            *nSkins = 0;
            return NULL;
        }


    
        // Read in the vertices  NOTE:// no longer used
        for (iPt = 0; iPt < collHdr.NVertices; iPt++) {
            if (fread(&offset, sizeof(VEC), 1, fp) != 1) {
                DestroyConvex(collSkin, fileHdr.NSkins);
                *nSkins = 0;
                return NULL;
            }
        }


        
        // Read in the edges  NOTE:// no longer used
        for (iEdge = 0; iEdge < collHdr.NEdges; iEdge++) {
            if (fread(&offset, sizeof(INDEX), 2, fp) != 2) {
                DestroyConvex(collSkin, fileHdr.NSkins);
                *nSkins = 0;
                return NULL;
            }
        }


    

        // Read in the faces
        if (fread(collSkin[iSkin].Faces, sizeof(PLANE), collSkin[iSkin].NFaces, fp) != (size_t)collSkin[iSkin].NFaces)
        {

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
    INDEX   iSphere;
    SPHERE  *spheres;
    size_t  nRead;

    Assert(fp != NULL);

    // Read in the number of spheres
    nRead = fread(nSpheres, sizeof(INDEX), 1, fp);
    if (nRead < 1) {
        *nSpheres = 0;
        return NULL;
    }

#if defined(_PSX) && DEBUG
    printf("Loading %d Spheres\n", *nSpheres);
#endif

    // Allocate space for the spheres
    spheres = (SPHERE *)malloc(sizeof(SPHERE) * *nSpheres);
    if (spheres == NULL) {
        *nSpheres = 0;
        return NULL;
    }

//  *nSpheres = 0;
//  return spheres;

    // Read in the spheres
    for (iSphere = 0; iSphere < *nSpheres; iSphere++) {
        nRead = fread(&spheres[iSphere], sizeof(SPHERE), 1, fp);
        if (nRead < 1) {
            *nSpheres = iSphere;
            return spheres;
        }

        // Shift all positions down to allow GTE to do transform into world space
        spheres[iSphere].Pos.v[X] >>= PSX_BODYSPHERE_SHIFT;
        spheres[iSphere].Pos.v[Y] >>= PSX_BODYSPHERE_SHIFT;
        spheres[iSphere].Pos.v[Z] >>= PSX_BODYSPHERE_SHIFT;

    }

    Assert(iSphere == *nSpheres);

    return spheres;
}

//------------------------------------------------------------------------------
// (N64) CONVEX *LoadConvex(FIL *fp, INDEX *nSkins, int extraPtsPerEdge)
//------------------------------------------------------------------------------
#elif defined(_N64)
CONVEX *LoadConvex(FIL *fp, INDEX *nSkins)
{
    int iSkin, iEdge, iPt;

    COLLSKIN_FILEHDR    fileHdr;
    COLLSKIN_COLLHDR    collHdr;
    CONVEX  *collSkin;
    VEC                 arse;

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

    #ifdef TRACE_LOAD
    printf("Loading Convex Hulls\n%4d Hulls\n", fileHdr.NSkins);
    #endif

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
        
        // Allocate space for the data
        if (!SetupConvex(&collSkin[iSkin], collHdr.NFaces))
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

        // Offset NOT USED
        if (FFS_Read(&arse, sizeof(VEC), fp) != sizeof(VEC))
        {
            DestroyConvex(collSkin, fileHdr.NSkins);
            *nSkins = 0;
            return NULL;
        }

        // Read in the vertices // NOT USED
        for (iPt = 0; iPt < collHdr.NVertices; iPt++) {
            if (FFS_Read(&arse, sizeof(VEC), fp) != sizeof(VEC))
            {
                DestroyConvex(collSkin, fileHdr.NSkins);
                *nSkins = 0;
                return NULL;
            }
        }

        // Read in the edges // NOT USED
        for (iEdge = 0; iEdge < collHdr.NEdges; iEdge++) {
            if (FFS_Read(&arse, sizeof(INDEX) * 2, fp) != sizeof(INDEX) * 2)
            {
                DestroyConvex(collSkin, fileHdr.NSkins);
                *nSkins = 0;
                return NULL;
            }
        }

        // Read in the faces
        if (FFS_Read(collSkin[iSkin].Faces, sizeof(PLANE) * collSkin[iSkin].NFaces, fp) != (sizeof(PLANE) * collSkin[iSkin].NFaces))
        {
            DestroyConvex(collSkin, fileHdr.NSkins);
            *nSkins = 0;
            return NULL;
        }

        #ifdef TRACE_LOAD
        printf("Hull %2d: %4d Faces\n", iSkin, collHdr.NFaces);
        #endif
    }

    *nSkins = fileHdr.NSkins;
    return collSkin;
}


//------------------------------------------------------------------------------
// (N64) SPHERE *LoadSpheres(FIL *fp, INDEX *nSpheres)
//------------------------------------------------------------------------------
SPHERE *LoadSpheres(FIL *fp, INDEX *nSpheres)
{
    INDEX   iSphere;
    SPHERE  *spheres;
    size_t  nRead;

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

    #ifdef TRACE_LOAD
    printf("Loaded %4d Spheres\n", *nSpheres);
    #endif

    return spheres;
}

#endif // _N64



/////////////////////////////////////////////////////////////////////
//
// PointInConvex: Check whether the passed point lies within the
// body (point should be in the frame of the body owning the skin)
//
/////////////////////////////////////////////////////////////////////

bool PointInConvex(VEC *pos, CONVEX *skin, PLANE *plane, REAL *minDepth) 
{
    int     iFace;

    REAL    depth;
    
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
    int     iFace;
    REAL    depth, depth2, minDepth, sDist;
    REAL    minTime, timeDepth;
    VEC     pos, dR;
    PLANE   *nearPlane, *entryPlane, plane;

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
//#ifndef _PSX
    int     iR;
    REAL    newShift;

    for (iR = 0; iR < 3; iR++) {
        newShift = MulScalar(shiftMag, normal->v[iR]) ;
        if (Sign(shift->v[iR]) == Sign(newShift)) {
            if (abs(shift->v[iR]) < abs(newShift)) {
                shift->v[iR] = newShift;
            }
        } else {
            shift->v[iR] += newShift;
        }
    }
/*#else //_PSX
    int     iR;
    REAL    newShift;
    REAL    *normComp, *shiftComp;

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
#endif*/
}


/////////////////////////////////////////////////////////////////////
//
// PointInCollPolyBounds: given a point known to be in the plane
// of the polygon, check if it lies within the bounds of the 
// polygon defined by the vertices in the NEWCOLLPOLY object
//
/////////////////////////////////////////////////////////////////////
#ifndef _PSX
bool PointInCollPolyBounds(VEC *pt, NEWCOLLPOLY *poly)
{
    if (VecDotPlane(pt, &poly->EdgePlane[0]) > COLL_EPSILON) return FALSE;
    if (VecDotPlane(pt, &poly->EdgePlane[1]) > COLL_EPSILON) return FALSE;
    if (VecDotPlane(pt, &poly->EdgePlane[2]) > COLL_EPSILON) return FALSE;
    if (IsPolyQuad(poly)) {
        if (VecDotPlane(pt, &poly->EdgePlane[3]) > COLL_EPSILON) return FALSE;
    }
    return TRUE;
}
#else
bool PointInCollPolyBounds(VEC *pt, NEWCOLLPOLY *collPoly)
{
    // Find axis with a normal component greater than 0.5 (must be at least one component > 1/sqrt(3))
    if (abs(collPoly->Plane.v[A]) > HALF) {
        if (!LineCollPolyYZ(pt, collPoly)) return FALSE;
    } else if(abs(collPoly->Plane.v[B]) > HALF) {
        if (!LineCollPolyXZ(pt, collPoly)) return FALSE;
    } else {
        if (!LineCollPolyXY(pt, collPoly)) return FALSE;
    }
    return TRUE;
}
#endif

/////////////////////////////////////////////////////////////////////
//
// SphereInCollPolyBounds: check whether a sphere is within the 
// infinite prism defined by the edge planes of the collision poly
//
/////////////////////////////////////////////////////////////////////
#ifndef _PSX
bool SphereInCollPolyBounds(VEC *pt, REAL radius, NEWCOLLPOLY *poly)
{
    if (VecDotPlane(pt, &poly->EdgePlane[0]) > (COLL_EPSILON)) return FALSE;
    if (VecDotPlane(pt, &poly->EdgePlane[1]) > (COLL_EPSILON)) return FALSE;
    if (VecDotPlane(pt, &poly->EdgePlane[2]) > (COLL_EPSILON)) return FALSE;
    if (IsPolyQuad(poly)) {
        if (VecDotPlane(pt, &poly->EdgePlane[3]) > (COLL_EPSILON)) return FALSE;
    }
    return TRUE;
}
#else
bool SphereInCollPolyBounds(VEC *pt, REAL radius, NEWCOLLPOLY *collPoly)
{
    // Find axis with a normal component greater than 0.5 (must be at least one component > 1/sqrt(3))
    if (collPoly->Plane.v[A] > HALF) {
        if (!LineCollPolyYZ(pt, collPoly)) return FALSE;
    } else if(collPoly->Plane.v[B] > HALF) {
        if (!LineCollPolyXZ(pt, collPoly)) return FALSE;
    } else {
        if (!LineCollPolyXY(pt, collPoly)) return FALSE;
    }
    return TRUE;
}
#endif

/////////////////////////////////////////////////////////////////////
//
// SphereCollSkin: check for collision between a sphere and a 
// convex hull. Set the passed collision point and  normal and 
// return TRUE for a collision.
//
//  inputs:
//      spherePos       - position of sphere centre in collskin coords
//      sphereRad       - radius of sphere
//      skin            - the collision skin
//
//  outputs:
//      collPos         - collision position in skin coords
//      collNorm        - collision normal in skin coords
//      collShift       - shift to move sphere out of skin
//
/////////////////////////////////////////////////////////////////////

#ifdef _PC
bool SphereConvex(VEC       *spherePos, 
                    REAL        sphereRad, 
                    CONVEX      *skin, 
                    VEC     *collPos,
                    PLANE       *collPlane,
                    REAL        *collDepth)
{
    bool    collided;
    int     iFace;
    REAL    dist, maxDist;

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
bool SphereConvex(VEC       *spherePos, 
                    REAL        sphereRad, 
                    CONVEX      *skin, 
                    VEC     *collPos,
                    PLANE       *collPlane,
                    REAL        *collDepth)
{
    int     iFace;
    REAL    dist, maxDist;

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
    int iSphere, iSkin;


    SetBBox(&collSkin->TightBBox, LARGEDIST, -LARGEDIST, LARGEDIST, -LARGEDIST, LARGEDIST, -LARGEDIST);

    // Loop over the collision spheres
    for (iSphere = 0; iSphere < collSkin->NSpheres; iSphere++) {
        AddSphereToBBox(&collSkin->TightBBox, &collSkin->Sphere[iSphere]);
    }

    // Loop over the skins
    for (iSkin = 0; iSkin < collSkin->NConvex; iSkin++) {

        // Set the BBox to obviously wrong values
        SetBBox(&collSkin->Convex[iSkin].BBox, LARGEDIST, -LARGEDIST, LARGEDIST, -LARGEDIST, LARGEDIST, -LARGEDIST);

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
    int     ii;
    VEC corners[8], tmpVec;

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

    argX = 2.0f * PI * dx / cor->Lx;
    argZ = 2.0f * PI * dy / cor->Ly;

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
    int     iPoly;
    long    gridNum, endGridNum, dX, dZ, xGrid, zGrid;
    REAL    t, depth, minT;
    VEC dR, intersect;
    PLANE   face;
    COLLGRID    *grid;
    NEWCOLLPOLY *poly;
    bool    lastGrid = FALSE;

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
            poly = GetCollPoly(grid->CollPolyIndices[iPoly]);

            if (PolyCameraOnly(poly) || PolyObjectOnly(poly)) continue;

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
    int     iPoly;
    long    gridNum, endGridNum, dX, dZ, xGrid, zGrid;
    REAL    t, depth, minT;
    VEC dR, intersect;
    PLANE   face;
    COLLGRID    *grid;
    NEWCOLLPOLY *poly;
    bool    lastGrid = FALSE;

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
            poly = GetCollPoly(grid->CollPolyIndices[iPoly]);

            // See if there is an intersection
            if (!LinePlaneIntersect(src, dest, &poly->Plane, &t, &depth)) continue;
#ifndef _PSX
            if (depth > ZERO) continue;
#endif
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
                if (dX != 0) {
                    gridNum += dX;
                }
                if (dZ == 1) {
                    gridNum += NearestInt(COL_CollGridData.XNum);
                } else if (dZ == -1) {
                    gridNum -= NearestInt(COL_CollGridData.XNum);
                }
            }
        }

    } while ((gridNum != -1) && (!lastGrid));


    // Got this far, so must have line of sight
    return TRUE;
}

/////////////////////////////////////////////////////////////////////
//
// LineOfSightObj: loop over collidable objects and check for line
// of sight between source and dest points. Ignore the passed object.
//
/////////////////////////////////////////////////////////////////////
#ifndef _PSX
bool LineOfSightObj(VEC *src, VEC *dest, REAL *minDist, OBJECT *ignoreObj)
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

        if (obj == ignoreObj) continue;

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
    } else if (body->CollSkin.CollType == BODY_COLL_CONVEX) {
        los = LineOfSightConvex(body, src, dest, minDist);
        return los;
    }
    return TRUE;
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
#ifdef _PC
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
#endif

/////////////////////////////////////////////////////////////////////
//
// AddSphereToBBox:
//
/////////////////////////////////////////////////////////////////////


void AddPosRadToBBox(BBOX *bBox, VEC *pos, REAL radius) 
{
    register REAL r1, r2, r3, r4, r5;
    r1 = bBox->XMin;
    r2 = pos->v[X];
    r3 = bBox->XMax;
    
    r4 = r2 - radius;
    r5 = Min(r1, r4);   // XMin
    r4 = r2 + radius;
    r4 = Max(r3, r4);   // XMax

    r1 = bBox->YMin;
    r2 = pos->v[Y];
    r3 = bBox->YMax;
    
    bBox->XMin = r5;
    bBox->XMax = r4;

    r4 = r2 - radius;
    r5 = Min(r1, r4);   // YMin
    r4 = r2 + radius;
    r4 = Max(r3, r4);   // YMax

    r1 = bBox->ZMin;
    r2 = pos->v[Z];
    r3 = bBox->ZMax;
    
    bBox->YMin = r5;
    bBox->YMax = r4;

    r4 = r2 - radius;
    r1 = Min(r1, r4);   // ZMin
    r4 = r2 + radius;
    r2 = Max(r3, r4);   // ZMax

    bBox->ZMin = r1;
    bBox->ZMax = r2;

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
#ifndef _PSX

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

#else // PSX Version

void RotTransBBox(BBOX *srcBox, MAT *mat, VEC *pos, BBOX *destBox)
{

    VEC pt, destPt;
    MATRIX PSXMat;
    SVECTOR SVector;

    PSXMat.m[0][0] = mat->m[0]>>4;
    PSXMat.m[0][1] = mat->m[1]>>4;
    PSXMat.m[0][2] = mat->m[2]>>4;
    PSXMat.m[1][0] = mat->m[3]>>4;
    PSXMat.m[1][1] = mat->m[4]>>4;
    PSXMat.m[1][2] = mat->m[5]>>4;
    PSXMat.m[2][0] = mat->m[6]>>4;
    PSXMat.m[2][1] = mat->m[7]>>4;
    PSXMat.m[2][2] = mat->m[8]>>4;

    PSXMat.t[0] = PSXMat.t[1] = PSXMat.t[2] = 0;

    
    // Set destination box to invalid box
    SetBBox(destBox, LARGEDIST, -LARGEDIST, LARGEDIST, -LARGEDIST, LARGEDIST, -LARGEDIST);

    // Transform the corners of the source box into new frame and add to dest box
    
    SVector.vx = srcBox->XMin>>4;
    SVector.vy = srcBox->YMin>>4;
    SVector.vz = srcBox->ZMin>>4;
    ApplyMatrix( &PSXMat, &SVector, &destPt );
    destPt.v[0] <<= 4;
    destPt.v[1] <<= 4;
    destPt.v[2] <<= 4;
    AddPointToBBox(destBox, &destPt);

//  SVector.vx = srcBox->XMin>>4;
//  SVector.vy = srcBox->YMin>>4;
    SVector.vz = srcBox->ZMax>>4;
    ApplyMatrix( &PSXMat, &SVector, &destPt );
    destPt.v[0] <<= 4;
    destPt.v[1] <<= 4;
    destPt.v[2] <<= 4;
    AddPointToBBox(destBox, &destPt);

//  SVector.vx = srcBox->XMin>>4;
    SVector.vy = srcBox->YMax>>4;
    SVector.vz = srcBox->ZMin>>4;
    ApplyMatrix( &PSXMat, &SVector, &destPt );
    destPt.v[0] <<= 4;
    destPt.v[1] <<= 4;
    destPt.v[2] <<= 4;
    AddPointToBBox(destBox, &destPt);

//  SVector.vx = srcBox->XMin>>4;
//  SVector.vy = srcBox->YMax>>4;
    SVector.vz = srcBox->ZMax>>4;
    ApplyMatrix( &PSXMat, &SVector, &destPt );
    destPt.v[0] <<= 4;
    destPt.v[1] <<= 4;
    destPt.v[2] <<= 4;
    AddPointToBBox(destBox, &destPt);


    SVector.vx = srcBox->XMax>>4;
    SVector.vy = srcBox->YMin>>4;
    SVector.vz = srcBox->ZMin>>4;
    ApplyMatrix( &PSXMat, &SVector, &destPt );
    destPt.v[0] <<= 4;
    destPt.v[1] <<= 4;
    destPt.v[2] <<= 4;
    AddPointToBBox(destBox, &destPt);


//  SVector.vx = srcBox->XMax>>4;
//  SVector.vy = srcBox->YMin>>4;
    SVector.vz = srcBox->ZMax>>4;
    ApplyMatrix( &PSXMat, &SVector, &destPt );
    destPt.v[0] <<= 4;
    destPt.v[1] <<= 4;
    destPt.v[2] <<= 4;
    AddPointToBBox(destBox, &destPt);

//  SVector.vx = srcBox->XMax>>4;
    SVector.vy = srcBox->YMax>>4;
    SVector.vz = srcBox->ZMin>>4;
    ApplyMatrix( &PSXMat, &SVector, &destPt );
    destPt.v[0] <<= 4;
    destPt.v[1] <<= 4;
    destPt.v[2] <<= 4;
    AddPointToBBox(destBox, &destPt);

//  SVector.vx = srcBox->XMax>>4;
//  SVector.vy = srcBox->YMax>>4;
    SVector.vz = srcBox->ZMax>>4;
    ApplyMatrix( &PSXMat, &SVector, &destPt );
    destPt.v[0] <<= 4;
    destPt.v[1] <<= 4;
    destPt.v[2] <<= 4;
    AddPointToBBox(destBox, &destPt);

    destBox->XMin += pos->v[0];
    destBox->XMax += pos->v[0];
    destBox->YMin += pos->v[1];
    destBox->YMax += pos->v[1];
    destBox->ZMin += pos->v[2];
    destBox->ZMax += pos->v[2];


}

/////////////////////////////////////////////////////////////////////////
// Special Version passsing a PSX Matrix

void RotTransBBoxPSX(BBOX *srcBox, MATRIX * PSXMat, VEC *pos, BBOX *destBox)
{

    VEC pt, destPt;
    
    SVECTOR SVector;

    
    // Set destination box to invalid box
    SetBBox(destBox, LARGEDIST, -LARGEDIST, LARGEDIST, -LARGEDIST, LARGEDIST, -LARGEDIST);

    // Transform the corners of the source box into new frame and add to dest box
    
    SVector.vx = srcBox->XMin>>4;
    SVector.vy = srcBox->YMin>>4;
    SVector.vz = srcBox->ZMin>>4;
    ApplyMatrix( PSXMat, &SVector, &destPt );
    destPt.v[0] <<= 4;
    destPt.v[1] <<= 4;
    destPt.v[2] <<= 4;
    AddPointToBBox(destBox, &destPt);

//  SVector.vx = srcBox->XMin>>4;
//  SVector.vy = srcBox->YMin>>4;
    SVector.vz = srcBox->ZMax>>4;
    ApplyMatrix( PSXMat, &SVector, &destPt );
    destPt.v[0] <<= 4;
    destPt.v[1] <<= 4;
    destPt.v[2] <<= 4;
    AddPointToBBox(destBox, &destPt);

//  SVector.vx = srcBox->XMin>>4;
    SVector.vy = srcBox->YMax>>4;
    SVector.vz = srcBox->ZMin>>4;
    ApplyMatrix( PSXMat, &SVector, &destPt );
    destPt.v[0] <<= 4;
    destPt.v[1] <<= 4;
    destPt.v[2] <<= 4;
    AddPointToBBox(destBox, &destPt);

//  SVector.vx = srcBox->XMin>>4;
//  SVector.vy = srcBox->YMax>>4;
    SVector.vz = srcBox->ZMax>>4;
    ApplyMatrix( PSXMat, &SVector, &destPt );
    destPt.v[0] <<= 4;
    destPt.v[1] <<= 4;
    destPt.v[2] <<= 4;
    AddPointToBBox(destBox, &destPt);


    SVector.vx = srcBox->XMax>>4;
    SVector.vy = srcBox->YMin>>4;
    SVector.vz = srcBox->ZMin>>4;
    ApplyMatrix( PSXMat, &SVector, &destPt );
    destPt.v[0] <<= 4;
    destPt.v[1] <<= 4;
    destPt.v[2] <<= 4;
    AddPointToBBox(destBox, &destPt);


//  SVector.vx = srcBox->XMax>>4;
//  SVector.vy = srcBox->YMin>>4;
    SVector.vz = srcBox->ZMax>>4;
    ApplyMatrix( PSXMat, &SVector, &destPt );
    destPt.v[0] <<= 4;
    destPt.v[1] <<= 4;
    destPt.v[2] <<= 4;
    AddPointToBBox(destBox, &destPt);

//  SVector.vx = srcBox->XMax>>4;
    SVector.vy = srcBox->YMax>>4;
    SVector.vz = srcBox->ZMin>>4;
    ApplyMatrix( PSXMat, &SVector, &destPt );
    destPt.v[0] <<= 4;
    destPt.v[1] <<= 4;
    destPt.v[2] <<= 4;
    AddPointToBBox(destBox, &destPt);

//  SVector.vx = srcBox->XMax>>4;
//  SVector.vy = srcBox->YMax>>4;
    SVector.vz = srcBox->ZMax>>4;
    ApplyMatrix( PSXMat, &SVector, &destPt );
    destPt.v[0] <<= 4;
    destPt.v[1] <<= 4;
    destPt.v[2] <<= 4;
    AddPointToBBox(destBox, &destPt);

    destBox->XMin += pos->v[0];
    destBox->XMax += pos->v[0];
    destBox->YMin += pos->v[1];
    destBox->YMax += pos->v[1];
    destBox->ZMin += pos->v[2];
    destBox->ZMax += pos->v[2];


}



#endif

////////////////////////////////////////////////////////

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
    BBOX bBox;
    PLANE plane;

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
#ifndef _PSX
            MovePlane(&poly->EdgePlane[iEdge], dPos);
#else
            VecPlusEqVec(&poly->Vertex[iEdge], dPos);
#endif
        }

        TransBBox(&poly->BBox, dPos);
    }
}


/////////////////////////////////////////////////////////////////////
//
// NextBodyCollInfo:
//
/////////////////////////////////////////////////////////////////////

COLLINFO_BODY *NextBodyCollInfo(NEWBODY *body)
{
    if ((body->NBodyColls >= MAX_COLLS_PER_BODY) || (COL_NBodyColls >= MAX_COLLS_BODY)) {
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
    if (COL_NWheelColls >= MAX_COLLS_WHEEL) {
        return NULL;
    } else {
        COL_WheelCollInfo[COL_NWheelColls].Prev = COL_WheelCollInfo[COL_NWheelColls].Next = NULL;
        //COL_WheelCollInfo[COL_NWheelColls].Active = FALSE;
        return &COL_WheelCollInfo[COL_NWheelColls];
    }
}



