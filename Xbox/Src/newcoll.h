//-----------------------------------------------------------------------------
// File: NewColl.h
//
// Desc: Collision stuff: three types of collision skins
//       1)   Convex Hulls used for the cars
//       2)   Planes with delimiting corners (2 or 3) for the world
//       3)   Simple bounding sphere (axis-aligned bounding box) for
//            other game objects (e.g. missiles)
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef NEWCOLL_H
#define NEWCOLL_H

#if defined(_PC)
#define GRIDIFY_OBJECTS     TRUE
#elif defined (_N64)
#define GRIDIFY_OBJECTS     FALSE
#elif defined(_PSX)
#define GRIDIFY_OBJECTS     FALSE
#endif

#include "Units.h"
#include "Util.h"


struct object_def;


#define USE_CONVEX_HULLS    FALSE

#define COLL_EPSILON    TO_LENGTH(Real(2.0f))
#define COLL_HALFEPSILON    (COLL_EPSILON / 2 )

#ifndef _PSX
#define SMALL_IMPULSE_COMPONENT TO_IMP(Real(0.005))
#else
#define SMALL_IMPULSE_COMPONENT 10
#define SMALL_TORQUE            10
#endif

#define QUAD            (0x1)
#define TWOSIDED        (0x2)
#define OBJECT_ONLY     (0x4)
#define CAMERA_ONLY     (0x8)
#define NON_PLANAR      (0x10)
#define NO_SKID         (0x20)


#ifdef _PC
#define MAX_COLLS_BODY      700
#define MAX_COLLS_PER_BODY  32
#define MAX_COLLS_WHEEL     500
#else
#define MAX_COLLS_BODY      64
#define MAX_COLLS_PER_BODY  16
#define MAX_COLLS_WHEEL     32
#endif


////////////////////////////////////////////////////////////////
//
// Shift values for PSX BuildWorldSkin
//
////////////////////////////////////////////////////////////////

#define PSX_BODYSPHERE_SHIFT        4
#define PSX_PLANE_SHIFT             2


/////////////////////////////////////////////////////////////////////
// Different types of collision
//
enum {
    COLL_TYPE_NONE,
    COLL_TYPE_BODY,
    COLL_TYPE_CAR,

    MAX_COLL_TYPES
};

/////////////////////////////////////////////////////////////////////
// Axis-aligned bounding box
/////////////////////////////////////////////////////////////////////
typedef struct {

    REAL    XMin, XMax;
    REAL    YMin, YMax;
    REAL    ZMin, ZMax;

} BBOX;


/////////////////////////////////////////////////////////////////////
// Material type for collision polygons
/////////////////////////////////////////////////////////////////////

typedef struct MaterialStruct {
    long    Type;
    REAL    Roughness;
    REAL    Gripiness;
    REAL    Hardness;
    long    SkidColour;
    long    Corrugation;
    long    DustType;
    VEC     Vel;
} MATERIAL;

typedef struct CorrugationStruct {
    REAL    Amp;            // Corrugation amplitude
    REAL    Lx, Ly;         // Corrugation wavelength along x and y axes (in poly plane)
} CORRUGATION;

typedef struct DustStruct {
    long    SparkType;
    REAL    SparkProbability;
    REAL    SparkVar;
    long    SmokeType;
    REAL    SmokeProbability;
    REAL    SmokeVar;
} DUST;

#define MATERIAL_SPARK          1
#define MATERIAL_SKID           2
#define MATERIAL_OUTOFBOUNDS    4
#define MATERIAL_CORRUGATED     8
#define MATERIAL_MOVES          16
#define MATERIAL_DUSTY          32

#define MaterialAllowsSkids(material) ((material)->Type & MATERIAL_SKID)
#define SetMaterialAllowsSkids(material) ((material)->Type |= MATERIAL_SKID)
#define MaterialAllowsSparks(material) ((material)->Type & MATERIAL_SPARK)
#define SetMaterialAllowsSparks(material) ((material)->Type |= MATERIAL_SPARK)
#define MaterialOutOfBounds(material) ((material)->Type & MATERIAL_OUTOFBOUNDS)
#define SetMaterialOutOfBounds(material) ((material)->Type |= MATERIAL_OUTOFBOUNDS)
#define MaterialCorrugated(material) ((material)->Type & MATERIAL_CORRUGATED)
#define SetMaterialCorrugated(material) ((material)->Type |= MATERIAL_CORRUGATED)
#define MaterialMoves(material) ((material)->Type & MATERIAL_MOVES)
#define SetMaterialMoves(material) ((material)->Type |= MATERIAL_MOVES)
#define MaterialDusty(material) ((material)->Type & MATERIAL_DUSTY)
#define SetMaterialDusty(material) ((material)->Type |= MATERIAL_DUSTY)

#define PolyAllowsSkid(poly) (!((poly)->Type & NO_SKID))

enum MaterialEnum {
    MATERIAL_NONE = -1,
    MATERIAL_DEFAULT = 0,
    MATERIAL_MARBLE,
    MATERIAL_STONE,
    MATERIAL_WOOD,
    MATERIAL_SAND,
    MATERIAL_PLASTIC,
    MATERIAL_CARPETTILE,
    MATERIAL_CARPETSHAG,
    MATERIAL_BOUNDARY,
    MATERIAL_GLASS,
    MATERIAL_ICE1,
    MATERIAL_METAL,
    MATERIAL_GRASS,
    MATERIAL_BUMPMETAL,
    MATERIAL_PEBBLES,
    MATERIAL_GRAVEL,
    MATERIAL_CONVEYOR1,
    MATERIAL_CONVEYOR2,
    MATERIAL_DIRT1,
    MATERIAL_DIRT2,
    MATERIAL_DIRT3,
    MATERIAL_ICE2,
    MATERIAL_ICE3,
    MATERIAL_WOOD2,
    MATERIAL_CONVEYOR_MARKET1,
    MATERIAL_CONVEYOR_MARKET2,
    MATERIAL_PAVING,

    MATERIAL_NTYPES
};

enum CorrugationEnum {
    CORRUG_NONE,
    CORRUG_PEBBLES,
    CORRUG_GRAVEL,
    CORRUG_STEEL,
    CORRUG_CONVEYOR,
    CORRUG_DIRT1,
    CORRUG_DIRT2,
    CORRUG_DIRT3,

    CORRUG_NTYPES
};

enum DustEnum {
    DUST_NONE,
    DUST_GRAVEL,
    DUST_SAND,
    DUST_GRASS,
    DUST_DIRT,
    DUST_ROAD,

    DUST_NTYPES
};

/////////////////////////////////////////////////////////////////////
// Collision polygon (plane and corners, plus a bounding box)
/////////////////////////////////////////////////////////////////////

#ifndef _PSX
typedef struct CollPolyStruct {

    long    Type;           // Bitmask TRIangle or QUADrilateral
    long    Material;
    
    PLANE   Plane;          // Plane describing the surface of the poly
    PLANE   EdgePlane[4];   // Plane describing the edges of the poly
    BBOX    BBox;

} NEWCOLLPOLY;
#else
typedef struct CollPolyStruct {
    short   Type;
    short   Material;
    PLANE   Plane;
    VEC     Vertex[4];
    BBOX    BBox;
} NEWCOLLPOLY;
#endif

/////////////////////////////////////////////////////////////////////
// Info required to respond to a collision
/////////////////////////////////////////////////////////////////////

struct NewBodyStruct;
struct CarStruct;

typedef struct BodyCollInfoStruct {
    bool    Active;
    
    struct NewBodyStruct    *Body1;
    VEC Pos1;
    struct NewBodyStruct    *Body2;
    VEC Pos2;

    VEC WorldPos;
    VEC Vel;
    PLANE   Plane;
    REAL    Depth;
    REAL    Time;
    REAL    Grip;
    REAL    StaticFriction;
    REAL    KineticFriction;
    REAL    Restitution;
    MATERIAL *Material;         // Material type for this collision
    NEWCOLLPOLY *CollPoly;      // The collision poly

    struct BodyCollInfoStruct *Next;
    struct BodyCollInfoStruct *Prev;


} COLLINFO_BODY;

typedef struct WheelCollInfoStruct {
    //bool  Ignore;             // has this collision been seen to?
    struct CarStruct *Car;
    int     IWheel;             // Wheel number of the collided wheel
    struct NewBodyStruct    *Body2;
    VEC Pos;                // Car-relative position of coliision
    VEC Pos2;
    VEC WorldPos;           // World-relative position of collision
    VEC Vel;                // Collision velocity
    REAL    VelDotNorm;     // Collision velocity along collision plane normal
    REAL    UpDotNorm;      // Car's UP vector dotted with collision plane normal
    REAL    SlideVel;       // Sideways sliding velocity (for skid calculations)
    PLANE   Plane;              // Collision plane
    REAL    Depth;              // Penetration Depth
    REAL    Time;               // Collision "time" (fraction along velocity vector)
    REAL    Grip;
    REAL    StaticFriction;
    REAL    KineticFriction;
    REAL    Restitution;
    MATERIAL *Material;         // Material type for this collision
    NEWCOLLPOLY *CollPoly;      // The collision poly

    struct WheelCollInfoStruct *Next;
    struct WheelCollInfoStruct *Prev;

} COLLINFO_WHEEL;

/////////////////////////////////////////////////////////////////////
// Convex-hull collision skin
/////////////////////////////////////////////////////////////////////
typedef enum HullPriorityEnum {
    HULL_PRIORITY_MIN,
    HULL_PRIORITY_MED,
    HULL_PRIORITY_MAX,

} HULL_PRIORITY;

typedef struct {
    INDEX   NSkins;
} COLLSKIN_FILEHDR;

typedef struct {
    INDEX   NVertices;
    INDEX   NEdges;
    INDEX   NFaces;
} COLLSKIN_COLLHDR;

typedef struct ConvexStruct{

    INDEX   NFaces;         // Number of faces
    PLANE   *Faces;         // array of 4 element arrays giving indices of face corvers

    BBOX    BBox;           // Axis-aligned tight bounding-box


} CONVEX;

typedef struct CollSkinStruct {

    CONVEX  *Convex;                // Pointer to array of convex hulls
    CONVEX  *WorldConvex;
    CONVEX  *OldWorldConvex;
    INDEX   NConvex;                        // Number of convex hulls

    SPHERE  *Sphere;                    // Pointer to array of spheres
    SPHERE  *WorldSphere;
    SPHERE  *OldWorldSphere;
    INDEX   NSpheres;                   // Number of spheres

    NEWCOLLPOLY *CollPoly;
    NEWCOLLPOLY *WorldCollPoly;
    INDEX   NCollPolys;

    BBOX    TightBBox;                  // Tight-fitting local-frame axis-aligned bounding box
    BBOX    BBox;                       // Tight-fitting world-frame axis-aligned bounding box
    REAL    Radius;                     // All-encompassing radius for frustum tests;

    long    CollType;                   // Collision type (SPHERE or CONVEX)
    HULL_PRIORITY   HullPriority;       // For body-body collision (determines which one uses spheres and which one uses hull)
    bool    AllowWorldColls;
    bool    AllowObjColls;

} COLLSKIN;

typedef COLLSKIN COLLSKIN_INFO;


/////////////////////////////////////////////////////////////////////
// Collision Poly file header
/////////////////////////////////////////////////////////////////////
typedef struct {

    short NPolys;

} NEWCOLLPOLYHDR;



/////////////////////////////////////////////////////////////////////
// Collision gridding stuff
/////////////////////////////////////////////////////////////////////

typedef struct {
    REAL    XStart, ZStart;
    REAL    XNum, ZNum;
    REAL    GridSize;
} COLLGRID_DATA;

typedef struct CollGridStruct {
#ifdef _PC
    long    NCollPolys;                 // Total number of collision polys in grid
    long    NWorldPolys;                // Number of polys that are from the world (not the instances)
#else
    short   NCollPolys;                 // Total number of collision polys in grid
    short   NWorldPolys;                // Number of polys that are from the world (not the instances)
#endif

#if GRIDIFY_OBJECTS
    struct object_def *ObjectHead;      // Pointer to first object in this grid
#endif

    short   *CollPolyIndices;           // List of indices into collision poly array
} COLLGRID;


#define GRIDINDEX_MASK                  16383           // First 12 bits give the poly index (means max of 8191 collision polys)
#define GRIDINDEX_OVERLAPPED_ABOVE      16384           // 13th bit is whether this poly is overlapped by one above here
#define GRIDINDEX_OVERLAPS_BELOW        32768           // 14th bit is whether this poly overlaps any below it

#if defined(_N64)

#define GetCollPoly(_iPoly)                                 \
    ({                                                      \
        register long index = (_iPoly) & GRIDINDEX_MASK;    \
        register long diff = index - COL_NWorldCollPolys;   \
        (diff < 0)?                                         \
        &COL_WorldCollPoly[index]:                          \
        &COL_InstanceCollPoly[diff];                        \
    })

#elif defined(_PC)

#define GetCollPoly(_iPoly)                                                             \
    (                                                                                   \
        (((_iPoly) & GRIDINDEX_MASK) < COL_NWorldCollPolys)?                            \
        &COL_WorldCollPoly[((_iPoly) & GRIDINDEX_MASK)]:                                \
        &COL_InstanceCollPoly[((_iPoly) & GRIDINDEX_MASK) - COL_NWorldCollPolys]        \
    )


#else

#define GetCollPoly(_iPoly) (&COL_WorldCollPoly[(_iPoly) & GRIDINDEX_MASK] )

#endif

////////////////////////////////////////////////////////////////
//
// Car pair collision stuff (in here so it is near the function
// where it is used most)
//
////////////////////////////////////////////////////////////////

typedef struct CarPairCollMatrixStruct {
    unsigned char Status;
} CARCOLL_MATRIX;

#define CARS_COLLIDED 0x1

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
//
// Defined Functions
//
/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////
// Set bounding box from passed REALs
/////////////////////////////////////////////////////////////////////
#define SetBBox(bBox, xMin, xMax, yMin, yMax, zMin, zMax) \
{ \
    (bBox)->XMin = xMin; \
    (bBox)->XMax = xMax; \
    (bBox)->YMin = yMin; \
    (bBox)->YMax = yMax; \
    (bBox)->ZMin = zMin; \
    (bBox)->ZMax = zMax; \
}

#define CopyBBox(src, dest) \
{ \
    (dest)->XMin = (src)->XMin; \
    (dest)->XMax = (src)->XMax; \
    (dest)->YMin = (src)->YMin; \
    (dest)->YMax = (src)->YMax; \
    (dest)->ZMin = (src)->ZMin; \
    (dest)->ZMax = (src)->ZMax; \
}

#define ClearBBox(box)          \
{                               \
    (box)->XMin = LARGEDIST;    \
    (box)->XMax = -LARGEDIST;   \
    (box)->YMin = LARGEDIST;    \
    (box)->YMax = -LARGEDIST;   \
    (box)->ZMin = LARGEDIST;    \
    (box)->ZMax = -LARGEDIST;   \
}

/////////////////////////////////////////////////////////////////////
// Check two bounding boxes for overlap. dR is vector from
// bBox1 to bBox2
/////////////////////////////////////////////////////////////////////

#define BBTestXZY(bBox1, bBox2) \
    ((  ((bBox1)->XMin > (bBox2)->XMax) || \
        ((bBox1)->XMax < (bBox2)->XMin) || \
        ((bBox1)->ZMin > (bBox2)->ZMax) || \
        ((bBox1)->ZMax < (bBox2)->ZMin) || \
        ((bBox1)->YMin > (bBox2)->YMax) || \
        ((bBox1)->YMax < (bBox2)->YMin) ) ? FALSE: TRUE)

#define BBTestYXZ(bBox1, bBox2) \
    ((  ((bBox1)->YMin > (bBox2)->YMax) || \
        ((bBox1)->YMax < (bBox2)->YMin) || \
        ((bBox1)->XMin > (bBox2)->XMax) || \
        ((bBox1)->XMax < (bBox2)->XMin) || \
        ((bBox1)->ZMin > (bBox2)->ZMax) || \
        ((bBox1)->ZMax < (bBox2)->ZMin) ) ? FALSE: TRUE)

#define SphereBBTest(sPos, sRad, bBox) \
    ((  ((bBox)->XMin - (sPos)->v[X] > (sRad)) || \
        ((sPos)->v[X] - (bBox)->XMax > (sRad)) || \
        ((bBox)->ZMin - (sPos)->v[Z] > (sRad)) || \
        ((sPos)->v[Z] - (bBox)->ZMax > (sRad)) || \
        ((bBox)->YMin - (sPos)->v[Y] > (sRad)) || \
        ((sPos)->v[Y] - (bBox)->YMax > (sRad)) ) ? FALSE: TRUE)



/////////////////////////////////////////////////////////////////////
// Check if a point lies within a bounding box
/////////////////////////////////////////////////////////////////////
#define PointInBBox(pos, bBox) \
    ((  ((bBox)->XMin > (pos)->v[X]) || \
        ((bBox)->XMax < (pos)->v[X]) || \
        ((bBox)->ZMin > (pos)->v[Z]) || \
        ((bBox)->ZMax < (pos)->v[Z]) || \
        ((bBox)->YMin > (pos)->v[Y]) || \
        ((bBox)->YMax < (pos)->v[Y])    ) ? FALSE: TRUE)


/////////////////////////////////////////////////////////////////////
// Set bbox to include sphere
/////////////////////////////////////////////////////////////////////
#define AddSphereToBBox(bBox, sphere) AddPosRadToBBox((bBox), &(sphere)->Pos, (sphere)->Radius)

/*#define AddPosRadToBBox(__bBox, __pos, __radius)      \
{                                       \
    register REAL real1, real2, real3, real4, real5;    \
    real1 = (__bBox)->XMin;             \
    real2 = (__pos)->v[X];              \
    real3 = (__bBox)->XMax;             \   
    real4 = real2 - (__radius);         \
    real5 = Min(real1, real4);          \
    real4 = real2 + (__radius);         \
    real4 = Max(real3, real4);          \
    real1 = (__bBox)->YMin;             \
    real2 = (__pos)->v[Y];              \
    real3 = (__bBox)->YMax;             \
    (__bBox)->XMin = real5;             \
    (__bBox)->XMax = real4;             \
    real4 = real2 - (__radius);         \
    real5 = Min(real1, real4);          \
    real4 = real2 + (__radius);         \
    real4 = Max(real3, real4);          \
    real1 = (__bBox)->ZMin;             \
    real2 = (__pos)->v[Z];              \
    real3 = (__bBox)->ZMax;             \
    (__bBox)->YMin = real5;             \
    (__bBox)->YMax = real4;             \
    real4 = real2 - (__radius);         \
    real1 = Min(real1, real4);          \
    real4 = real2 + (__radius);         \
    real2 = Max(real3, real4);          \
    (__bBox)->ZMin = real1;             \
    (__bBox)->ZMax = real2;             \
}*/



/////////////////////////////////////////////////////////////////////
// Global Function prototypes

// Stuff to do with collision polygons...

#define IsPolyTwoSided(poly) ((poly)->Type & TWOSIDED)
#define IsPolyQuad(poly) ((poly)->Type & QUAD)
#define IsPolyTriangle(poly) (!((poly)->Type & QUAD))
#define PolyObjectOnly(poly) ((poly)->Type & OBJECT_ONLY)
#define PolyCameraOnly(poly) ((poly)->Type & CAMERA_ONLY)

#define SetPolyTwoSided(poly) ((poly)->Type |= TWOSIDED)
#define SetPolyQuad(poly) ((poly)->Type |= QUAD)
#define SetPolyTriangle(poly) ((poly)->Type &= !QUAD)


#ifndef _PSX
extern NEWCOLLPOLY *CreateCollPolys(short nPolys);
#else
extern short *CreateCollPolyIndices(int nPolys);
#endif

extern void DestroyCollPolys(NEWCOLLPOLY *polys);
extern void DestroyCollGrids();
extern COLLGRID *PosToCollGrid(VEC *pos);
extern long PosToCollGridCoords(VEC *pos, long *offsetX, long *offsetZ);
extern long PosToCollGridNum(VEC *pos);
#ifdef _N64
extern NEWCOLLPOLY *LoadNewCollPolys(FIL *Fil, short *nPolys);
extern bool LoadGridInfo(FIL *Fil);
extern CONVEX *LoadConvex(FIL *fp, INDEX *nSkins);
extern SPHERE *LoadSpheres(FIL *fp, INDEX *nSpheres);
#else
extern NEWCOLLPOLY *LoadNewCollPolys(FILE *fp, short *nPolys);
extern bool LoadGridInfo(FILE *fp);
extern CONVEX *LoadConvex(FILE *fp, INDEX *nSkins);
extern SPHERE *LoadSpheres(FILE *fp, INDEX *nSpheres);
#endif

extern bool PointInCollPolyBounds(VEC *pt, NEWCOLLPOLY *poly);
extern bool SphereInCollPolyBounds(VEC *pt, REAL radius, NEWCOLLPOLY *poly);
extern bool SphereCollPoly(VEC *oldPos, VEC *newPos, REAL radius, NEWCOLLPOLY *collPoly, PLANE *plane, VEC *relPos, VEC *worldPos, REAL *depth, REAL *time);



// Stuff to do with convex hull collisions...
extern void FreeCollSkin(COLLSKIN *collSkin);

extern CONVEX *CreateConvex(INDEX nConvex);
extern bool SetupConvex(CONVEX *skin, INDEX nFaces);
extern void DestroyConvex(CONVEX *skin, int nSkins);
extern bool CreateCopyCollSkin(COLLSKIN *collSkin);
extern void BuildWorldSkin(COLLSKIN *bodySkin, VEC *pos, MAT *mat);
extern void DestroySpheres(SPHERE *spheres);


extern bool PointInConvex(VEC *pt, CONVEX *skin, PLANE *plane, REAL *depth);
extern bool SphereConvex(VEC *spherePos, REAL sphereRad, CONVEX *convex, VEC *collPos, PLANE *collPlane, REAL *collDepth);
extern PLANE *LineToConvexColl(VEC *sPos, VEC *ePos, CONVEX *convex, REAL *penDepth, REAL *time);

// General collision stuff...
extern void ModifyShift(VEC *shift, REAL shiftMag, VEC *normal);
extern bool LinePlaneIntersect(VEC *lStart, VEC *lEnd, PLANE *plane, REAL *t, REAL *depth);
extern bool PosDirPlaneIntersect(VEC *lStart, VEC *dir, PLANE *plane, REAL *t);
extern void RotTransCollPolys(NEWCOLLPOLY *collPoly, int nPolys, MAT *rMat, VEC *dPos);
extern void TransCollPolys(NEWCOLLPOLY *collPoly, int nPolys, VEC *dPos);


extern void ExpandBBox(BBOX *bBox, REAL delta);
extern void MakeTightLocalBBox(COLLSKIN *collSkin);
extern void RotTransBBox(BBOX *bBoxIn, MAT *rotMat, VEC *dR, BBOX *bBoxOut);

extern bool LineOfSight(VEC *dest, VEC *src);
extern bool LineOfSightDist(VEC *src, VEC *dest, REAL *minT, PLANE **plane);
#ifndef _PSX
extern bool LineOfSightObj(VEC *src, VEC *dest, REAL *minT, struct object_def *ignoreObj);
#endif

extern COLLINFO_BODY *NextBodyCollInfo(struct NewBodyStruct *body);
extern COLLINFO_WHEEL *NextWheelCollInfo();
extern void AdjustBodyColl(COLLINFO_BODY *collInfo, MATERIAL *material);
extern void AdjustWheelColl(COLLINFO_WHEEL *collInfo, MATERIAL *material);
extern int GetCollPolyVertices(NEWCOLLPOLY *poly, VEC *v0, VEC *v1, VEC *v2, VEC *v3);

extern void AddPosRadToBBox(BBOX *bBox, VEC *pos, REAL radius);
extern void AddPointToBBox(BBOX *bBox, VEC *pos);

struct object_def;
// Object collision stuff...
extern void COL_DoObjectCollisions(void);
extern void COL_BodyCollHandler(struct object_def *obj);
extern void COL_CarCollHandler(struct object_def *obj);
extern void COL_WaitForCollision(struct object_def *obj);
extern bool HaveCarsCollided(struct object_def *obj1, struct object_def *obj2);
extern void SetCarsCollided(struct object_def *obj1, struct object_def *obj2);
extern void ClearCarCollMatrix(void);
extern void InitWorldSkin(COLLSKIN *bodySkin, VEC *pos, MAT *mat);

extern void GetGridIndexRange(COLLGRID *grid, REAL yMin, REAL yMax, long *start, long *end);
extern long GetNearGrids(VEC *pos, COLLGRID **grids);
extern void UpdateObjectGrid(struct object_def *obj);
extern void RemoveObjectFromGrid(struct object_def *obj, COLLGRID *grid);



/////////////////////////////////////////////////////////////////////
// GLOBAL VARIABLES


// World collision skin
extern NEWCOLLPOLY  *COL_WorldCollPoly;
extern INDEX        COL_NWorldCollPolys;
extern NEWCOLLPOLY  *COL_InstanceCollPoly;
extern INDEX        COL_NInstanceCollPolys;
extern COLLGRID_DATA    COL_CollGridData;       // Gridding information
extern COLLGRID     *COL_CollGrid;              // Poly pointers and counter for each grid volume
extern long         COL_NCollGrids;             // Number of grid locations

// Collision lists
//extern COLLINFO_WHEEL COL_WheelCollInfo[MAX_COLLS_WHEEL];
extern int COL_NWheelColls;
extern int COL_NWheelDone;

//extern COLLINFO_BODY COL_BodyCollInfo[MAX_COLLS_BODY];
extern int COL_NBodyColls;
extern int COL_NBodyDone;

//extern COLLINFO_BODY  *COL_ThisBodyColl[MAX_COLLS_PER_BODY];
//extern int COL_NThisBodyColls;

extern int COL_NCollsTested;
extern int COL_NCollsPassed;



extern MATERIAL COL_MaterialInfo[MATERIAL_NTYPES];
extern CORRUGATION COL_CorrugationInfo[CORRUG_NTYPES];
extern DUST COL_DustInfo[DUST_NTYPES];

extern CARCOLL_MATRIX CarCollMatrix[MAX_NUM_PLAYERS][MAX_NUM_PLAYERS];


#ifdef _PSX
extern bool LineCollPolyXY(VEC *pos, NEWCOLLPOLY *collPoly); 
extern bool LineCollPolyYZ(VEC *pos, NEWCOLLPOLY *collPoly);
extern bool LineCollPolyXZ(VEC *pos, NEWCOLLPOLY *collPoly); 
#endif


#endif // NEWCOLL_H

