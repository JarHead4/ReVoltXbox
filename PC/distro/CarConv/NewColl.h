/////////////////////////////////////////////////////////////////////
//
// Collision stuff: three types of collision skins
//
// 1)	Convex Hulls used for the cars
// 2)	Planes with delimiting corners (2 or 3) for the world
// 3)	Simple bounding sphere (axis-aligned bounding box) for
//		other game objects (e.g. missiles)
//
/////////////////////////////////////////////////////////////////////

#ifndef __NEWCOLL_H__
#define __NEWCOLL_H__

#include "Units.h"


#define USE_CONVEX_HULLS	FALSE

#define COLL_EPSILON	TO_LENGTH(Real(2.0f))
#define COLL_HALFEPSILON	(COLL_EPSILON / 2 )

#ifndef _PSX
#define SMALL_IMPULSE_COMPONENT	TO_IMP(Real(0.005))
#else
#define SMALL_IMPULSE_COMPONENT	10
#define SMALL_TORQUE			10
#endif

#define QUAD			(0x1)
#define TWOSIDED		(0x2)
#define OBJECT_ONLY		(0x4)
#define CAMERA_ONLY		(0x8)
#define NON_PLANAR		(0x10)


#ifdef _PC
#define MAX_COLLS_BODY		500
#define MAX_COLLS_PER_BODY	32
#define MAX_COLLS_WHEEL		100
#else
#define MAX_COLLS_BODY		64
#define MAX_COLLS_PER_BODY	16
#define MAX_COLLS_WHEEL		32
#endif


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
/////////////////////////////////////////////////////////////////////
//
// New types
//
/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////


typedef VEC VERTEX;


/////////////////////////////////////////////////////////////////////
// Edge in terms of offsets into a vertex list
/////////////////////////////////////////////////////////////////////
typedef struct {

	INDEX	Vtx[2];

} EDGE;

/////////////////////////////////////////////////////////////////////
// Axis-aligned bounding box
/////////////////////////////////////////////////////////////////////
typedef struct {

	REAL	XMin, XMax;
	REAL	YMin, YMax;
	REAL	ZMin, ZMax;

} BBOX;


/////////////////////////////////////////////////////////////////////
// Material type for collision polygons
/////////////////////////////////////////////////////////////////////

typedef struct MaterialStruct {
	long	Type;
	REAL	Roughness;
	REAL	Gripiness;
	REAL	Hardness;
	long	SkidColour;
	long	Corrugation;
	long	DustType;
	VEC		Vel;
} MATERIAL;

typedef struct CorrugationStruct {
	REAL	Amp;			// Corrugation amplitude
	REAL	Lx, Ly;			// Corrugation wavelength along x and y axes (in poly plane)
} CORRUGATION;

typedef struct DustStruct {
	long	SparkType;
	REAL	SparkProbability;
	REAL	SparkVar;
} DUST;

#define MATERIAL_SPARK			1
#define MATERIAL_SKID			2
#define MATERIAL_OUTOFBOUNDS	4
#define MATERIAL_CORRUGATED		8
#define MATERIAL_MOVES			16
#define MATERIAL_DUSTY			32

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
	DUST_MUD,
	DUST_DIRT,

	DUST_NTYPES
};

/////////////////////////////////////////////////////////////////////
// Info required to respond to a collision
/////////////////////////////////////////////////////////////////////

struct NewBodyStruct;
struct CarStruct;

typedef struct BodyCollInfoStruct {
	bool	Active;
	
	struct NewBodyStruct	*Body1;
	VEC	Pos1;
	struct NewBodyStruct	*Body2;
	VEC	Pos2;

	VEC	WorldPos;
	VEC	Vel;
	PLANE	Plane;
	REAL	Depth;
	REAL	Time;
	REAL	Grip;
	REAL	StaticFriction;
	REAL	KineticFriction;
	REAL	Restitution;
	MATERIAL *Material;			// Material type for this collision

	struct BodyCollInfoStruct *Next;
	struct BodyCollInfoStruct *Prev;


} COLLINFO_BODY;

typedef struct WheelCollInfoStruct {
	//bool	Ignore;				// has this collision been seen to?
	struct CarStruct *Car;
	int		IWheel;				// Wheel number of the collided wheel
	struct NewBodyStruct	*Body2;
	VEC	Pos;				// Car-relative position of coliision
	VEC	Pos2;
	VEC	WorldPos;			// World-relative position of collision
	VEC	Vel;				// Collision velocity
	PLANE	Plane;				// Collision plane
	REAL	Depth;				// Penetration Depth
	REAL	Time;				// Collision "time" (fraction along velocity vector)
	REAL	Grip;
	REAL	StaticFriction;
	REAL	KineticFriction;
	REAL	Restitution;
	MATERIAL *Material;			// Material type for this collision

	struct WheelCollInfoStruct *Next;
	struct WheelCollInfoStruct *Prev;

} COLLINFO_WHEEL;

/////////////////////////////////////////////////////////////////////
// Collision polygon (plane and corners, plus a bounding box)
/////////////////////////////////////////////////////////////////////

typedef struct CollPolyStruct {

	long	Type;			// Bitmask TRIangle or QUADrilateral
	long	Material;
	
	PLANE	Plane;			// Plane describing the surface of the poly
	PLANE	EdgePlane[4];	// Plane describing the edges of the poly
	BBOX	BBox;

} NEWCOLLPOLY;

/////////////////////////////////////////////////////////////////////
// Convex-hull collision skin
/////////////////////////////////////////////////////////////////////
typedef struct {
	INDEX	NSkins;
} COLLSKIN_FILEHDR;

typedef struct {
	INDEX	NVertices;
	INDEX	NEdges;
	INDEX	NFaces;
} COLLSKIN_COLLHDR;

typedef struct ConvexStruct{

	INDEX	NPts;			// Number of points
	INDEX	NEdges;			// Number of edges
	INDEX	NFaces;			// Number of faces

	BBOX	BBox;			// Axis-aligned tight bounding-box
	//VEC	Offset;			// Offset of bounding box from body's CoM
	
	VERTEX	*Pts;			// collision skin corners (stored relative to body CoM)
	EDGE	*Edges;			// array of 2 element arrays giving index of edge vertices
	PLANE	*Faces;			// array of 4 element arrays giving indices of face corvers

} CONVEX;

typedef struct CollSkinStruct {

	CONVEX	*Convex;				// Pointer to array of convex hulls
	CONVEX	*WorldConvex;
	CONVEX	*OldWorldConvex;
	INDEX	NConvex;						// Number of convex hulls

	SPHERE	*Sphere;					// Pointer to array of spheres
	SPHERE	*WorldSphere;
	SPHERE	*OldWorldSphere;
	INDEX	NSpheres;					// Number of spheres

	NEWCOLLPOLY *CollPoly;
	INDEX	NCollPolys;

	BBOX	TightBBox;					// Tight-fitting local-frame axis-aligned bounding box
	BBOX	BBox;						// Tight-fitting world-frame axis-aligned bounding box
	REAL	Radius;						// All-encompassing radius for frustum tests;

	long	CollType;					// Collision type (SPHERE or CONVEX)
	bool	AllowWorldColls;
	bool	AllowObjColls;

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
	REAL	XStart, ZStart;
	REAL	XNum, ZNum;
	REAL	GridSize;
} COLLGRID_DATA;

typedef struct {
#ifndef _PSX
	long	NCollPolys;					// Total numner of collision polys in grid
	long	NWorldPolys;				// Number of polys that are from the world (not the instances)
	NEWCOLLPOLY **CollPolyPtr;			// List of pointers to polys in grid
#else
	short	NCollPolys;					// Total numner of collision polys in grid
	short	NWorldPolys;				// Number of polys that are from the world (not the instances)
	short	*CollPolyIndices;
#endif
} COLLGRID;


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

/////////////////////////////////////////////////////////////////////
// Check two bounding boxes for overlap. dR is vector from
// bBox1 to bBox2
/////////////////////////////////////////////////////////////////////

#define BBTestXZY(bBox1, bBox2) \
	(( 	((bBox1)->XMin > (bBox2)->XMax) || \
		((bBox1)->XMax < (bBox2)->XMin) || \
		((bBox1)->ZMin > (bBox2)->ZMax) || \
		((bBox1)->ZMax < (bBox2)->ZMin) || \
		((bBox1)->YMin > (bBox2)->YMax) || \
		((bBox1)->YMax < (bBox2)->YMin)	) ? FALSE: TRUE)

#define BBTestYXZ(bBox1, bBox2) \
	(( 	((bBox1)->YMin > (bBox2)->YMax) || \
		((bBox1)->YMax < (bBox2)->YMin) || \
		((bBox1)->XMin > (bBox2)->XMax) || \
		((bBox1)->XMax < (bBox2)->XMin) || \
		((bBox1)->ZMin > (bBox2)->ZMax) || \
		((bBox1)->ZMax < (bBox2)->ZMin) ) ? FALSE: TRUE)

/////////////////////////////////////////////////////////////////////
// Check if a point lies within a bounding box
/////////////////////////////////////////////////////////////////////
#define PointInBBox(pos, bBox) \
	((	((bBox)->XMin > (pos)->v[X]) || \
		((bBox)->XMax < (pos)->v[X]) || \
		((bBox)->ZMin > (pos)->v[Z]) || \
		((bBox)->ZMax < (pos)->v[Z]) || \
		((bBox)->YMin > (pos)->v[Y]) || \
		((bBox)->YMax < (pos)->v[Y])	) ? FALSE: TRUE)


/////////////////////////////////////////////////////////////////////
// Set bbox to include sphere
/////////////////////////////////////////////////////////////////////
#define AddSphereToBBox(bBox, sphere) AddPosRadToBBox((bBox), &(sphere)->Pos, (sphere)->Radius)


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
extern CONVEX *LoadConvex(FIL *fp, INDEX *nSkins, int extraPtsPerEdge);
extern SPHERE *LoadSpheres(FIL *fp, INDEX *nSpheres);
#else
extern NEWCOLLPOLY *LoadNewCollPolys(FILE *fp, short *nPolys);
extern bool LoadGridInfo(FILE *fp);
extern CONVEX *LoadConvex(FILE *fp, INDEX *nSkins, int extraPtsPerEdge);
extern SPHERE *LoadSpheres(FILE *fp, INDEX *nSpheres);
#endif

extern bool PointInCollPolyBounds(VEC *pt, NEWCOLLPOLY *poly);
extern bool SphereInCollPolyBounds(VEC *pt, REAL radius, NEWCOLLPOLY *poly);
extern bool SphereCollPoly(VEC *oldPos, VEC *newPos, REAL radius, NEWCOLLPOLY *collPoly, PLANE *plane, VEC *relPos, VEC *worldPos, REAL *depth, REAL *time);



// Stuff to do with convex hull collisions...
extern void FreeCollSkin(COLLSKIN *collSkin);

extern CONVEX *CreateConvex(INDEX nConvex);
extern bool SetupConvex(CONVEX *skin, INDEX nPts, INDEX extraPts, INDEX nEdges, INDEX nFaces);
extern void	DestroyConvex(CONVEX *skin, int nSkins);
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
#ifdef _PC
extern bool LineOfSightObj(VEC *src, VEC *dest, REAL *minT);
#endif

extern COLLINFO_BODY *NextBodyCollInfo();
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

/////////////////////////////////////////////////////////////////////
// GLOBAL VARIABLES


// World collision skin
extern NEWCOLLPOLY	*COL_WorldCollPoly;
extern INDEX		COL_NWorldCollPolys;
extern NEWCOLLPOLY	*COL_InstanceCollPoly;
extern INDEX		COL_NInstanceCollPolys;
extern COLLGRID_DATA	COL_CollGridData;		// Gridding information
extern COLLGRID		*COL_CollGrid;				// Poly pointers and counter for each grid volume
extern long			COL_NCollGrids;				// Number of grid locations

// Collision lists
//extern COLLINFO_WHEEL COL_WheelCollInfo[MAX_COLLS_WHEEL];
extern int COL_NWheelColls;
extern int COL_NWheelDone;

//extern COLLINFO_BODY COL_BodyCollInfo[MAX_COLLS_BODY];
extern int COL_NBodyColls;
extern int COL_NBodyDone;

//extern COLLINFO_BODY	*COL_ThisBodyColl[MAX_COLLS_PER_BODY];
//extern int COL_NThisBodyColls;

extern int COL_NCollsTested;

extern MATERIAL COL_MaterialInfo[MATERIAL_NTYPES];
extern CORRUGATION COL_CorrugationInfo[CORRUG_NTYPES];
extern DUST COL_DustInfo[DUST_NTYPES];

#endif
