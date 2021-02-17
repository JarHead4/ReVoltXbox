//**********************************************
//
// Track compiler module
//
// All of the code concerned with compiling a
// track description
//
//**********************************************

#include <assert.h>
#include "UnitInfo.h"
#include <world.h>
#include <newcoll.h>
#include <util.h>
#include "fileio.h"
#include "editor.h"
#include "compile.h"
#include "states.h"
#include "modules.h"
#include "editorconstants.h"

#include <d3drm.h>
#include <d3dutil.h>
#include <d3dmath.h>
#include <endstrm.h>

//**********************************************
//
//**********************************************
#define POLY_QUAD 1		
#define TPAGE_NONE -1

//**********************************************
//
//**********************************************
extern COMPILE_STATES		CompileState;
extern U8					ClockFrame;
extern MODULE_CHANGES		ToyChanges[];

//**********************************************
//
//**********************************************
static TRACKDESC*			LocalTrack = NULL;
static const TRACKTHEME*	LocalTheme = NULL;
static CURSORDESC*			LocalCursor = NULL;
static TRACKZONE*			ZoneBuffer = NULL;
static ZONESEQENTRY*		ZoneSequence = NULL;

static U16					ZonesInTrack;
static INDEX				ZonesVisited;
static INDEX				CurrentZone;
static U16					SmallCubeCount;
static SMALLCUBE*			SmallCubes = NULL;
static U16					BigCubeCount;
static BIGCUBE*				BigCubes = NULL;
static CUBE_HEADER_LOAD*	UnitCuboids = NULL;
static INDEX*				LinkTable = NULL;

static D3DVECTOR			StartPos;
static REAL					StartDir;

static NCPDATA				CollisionData = {0, 0, NULL, NULL, {ZERO, ZERO, ZERO, ZERO, ZERO}, NULL};

static D3DVECTOR			UnitRootVects[16];

//********************************************************************************************
//
//	First we have the general helper functions - those which carry out a commonly used task
//
//********************************************************************************************

//**********************************************
//
// NullifyCuboid
//
// Initializes cuboid to an 'inside-out, maximized'
// cuboid. This is not a normal configuration
// as all if the min value are greater than all
// of the max values and is mainly used to
// indicate an illegal cuboid.
//
//**********************************************
void NullifyCuboid(CUBE_HEADER_LOAD* cuboid)
{
	assert(cuboid != NULL);

	cuboid->Xmin = FLT_MAX;
	cuboid->Ymin = FLT_MAX;
	cuboid->Zmin = FLT_MAX;

	cuboid->Xmax = -FLT_MAX;
	cuboid->Ymax = -FLT_MAX;
	cuboid->Zmax = -FLT_MAX;
}

//**********************************************
//
// AddVectorToCuboid
//
// If the vector specified is outside of the
// cuboid, the cuboid is expanded to a size which 
// includes it
//
//**********************************************
void AddVectorToCuboid(const D3DVECTOR* vect,  CUBE_HEADER_LOAD* cuboid)
{
	assert(vect != NULL);
	assert(cuboid != NULL);

	cuboid->Xmin = min(cuboid->Xmin, vect->x);
	cuboid->Ymin = min(cuboid->Ymin, vect->y);
	cuboid->Zmin = min(cuboid->Zmin, vect->z);

	cuboid->Xmax = max(cuboid->Xmax, vect->x);
	cuboid->Ymax = max(cuboid->Ymax, vect->y);
	cuboid->Zmax = max(cuboid->Zmax, vect->z);
}

//**********************************************
//
// VertsInPoly
// 
// Simply returns the number of verts in a poly
//
//**********************************************
U16 VertsInPoly(const BASICPOLY* poly)
{
	assert(poly != NULL);
	return poly->IsTriangle ? (U16)3 : (U16)4;	
}

//**********************************************
//
// CopyCuboid - does exactly what it says on the tin
//
//**********************************************
void CopyCuboid(CUBE_HEADER_LOAD* dest, const CUBE_HEADER_LOAD* src)
{
	assert(dest != NULL);
	assert(src != NULL);

	dest->CentreX = src->CentreX;
	dest->CentreY = src->CentreY;
	dest->CentreZ = src->CentreZ;
	dest->Radius  = src->Radius;
	dest->Xmin	  = src->Xmin;
	dest->Ymin	  = src->Ymin;
	dest->Zmin	  = src->Zmin;
	dest->Xmax	  = src->Xmax;
	dest->Ymax	  = src->Ymax;
	dest->Zmax	  = src->Zmax;
	dest->PolyNum = src->PolyNum;
	dest->VertNum = src->VertNum;
}

//**********************************************
//
// CubesInCuboid
//
// Counts the number of 4 metre cubes within
// a cuboid - at present assumes that the cuboid
// is 4 metres square at the base, so only counts
// the number of cubes in the height (rounded up)
//
//**********************************************
U16 CubesInCuboid(const CUBE_HEADER_LOAD* cuboid)
{
	assert(cuboid != NULL);

	REAL y = cuboid->Ymin;
	U16 count = 0;

	if(y == cuboid->Ymax)
	{
		count = 1;
	}
	else
	{
		while(y < cuboid->Ymax)
		{
			y += SMALL_CUBE_SIZE;
			count++;
		}
	}
	return count;
}

//**********************************************
//
// TranslateCuboid
//
// Simple really - translate the spoecified 
// cuboid by the vector supplied.
//
//**********************************************
void TranslateCuboid(const D3DVECTOR* vect,  CUBE_HEADER_LOAD* cuboid)
{
	assert(vect != NULL);
	assert(cuboid != NULL);

	cuboid->Xmin += vect->x;
	cuboid->Ymin += vect->y;
	cuboid->Zmin += vect->z;
	cuboid->Xmax += vect->x;
	cuboid->Ymax += vect->y;
	cuboid->Zmax += vect->z;
}

//**********************************************
//
// AddPolysToCuboid
//
// Ensures that the cuboid contains all of the
// specified polygons by calling AddVectorToCuboid
// for each vertex of each polygon
//
//**********************************************
void AddPolysToCuboid(const POLYSET* polyset, CUBE_HEADER_LOAD* cuboid)
{
	assert(LocalTheme != NULL);
	assert(polyset != NULL);
	assert(cuboid != NULL);

	for(U32 p = 0; p < polyset->PolygonCount; p++)
	{
		BASICPOLY* poly = &LocalTheme->Polys[polyset->Indices[p]];
		
		U16 vertcount = VertsInPoly(poly);

		for(U16 v=0; v < vertcount; v++)
		{
			AddVectorToCuboid(&LocalTheme->Verts[poly->Vertices[v]], cuboid);
		}
	}
}

//**********************************************
//
// CuboidOfUnit
//
// Calculates the bounding cuboid of the specified
// track unit.
// Note that only the first 2 components of the
// unit are added (PEG & PAN) as the collison
// polygons
//
//**********************************************
void CuboidOfUnit(const TRACKUNIT* unit, CUBE_HEADER_LOAD* cuboid)
{
	assert(LocalTheme != NULL);
	assert(unit != NULL);
	assert(cuboid != NULL);

	//first initialize cuboid to an 'inside-out, maximized' cuboid
	//this assures that the first point added causes the cuboid to be a singularity
	//(any point is 'outside' the cuboid and therefore causes it to change to include it)
	NullifyCuboid(cuboid);

	MESH* mesh = &LocalTheme->Meshes[unit->MeshID];			//get the appropriate mesh

	AddPolysToCuboid(mesh->PolySets[PEG_INDEX], cuboid);	//and make the cuboid big enough to hold
	AddPolysToCuboid(mesh->PolySets[PAN_INDEX], cuboid);	//all the polys from both the peg and the pan
}

//**********************************************
//
// CuboidFromVectors
//
// Calculates the bounding cuboid for the 
// supplied list of vertices
//
//**********************************************
void CuboidFromVectors(const D3DVECTOR* vect, U32 vcount, CUBE_HEADER_LOAD* cuboid)
{
	assert(vect != NULL);
	assert(cuboid != NULL);

	NullifyCuboid(cuboid);

	while(vcount--)
	{
		AddVectorToCuboid(vect++, cuboid);
	}
}

//********************************************************************************************
//
//	Now we have the more task-specific functions
//
//********************************************************************************************

//**********************************************
// CountCollisionPolys
// 
// Counts the total number of collision polys
// which exist in the world
//
//**********************************************
S16 CountCollisionPolys(void)
{
	assert(LocalTrack != NULL);
	assert(LocalTheme != NULL);

	U16 polys = 0;
	U32 m = LocalTrack->Width * LocalTrack->Height;

	while(m--)
	{
		TRACKUNIT*	unit = LocalTheme->Units[LocalTrack->Units[m].UnitID];
		MESH*		mesh = &LocalTheme->Meshes[unit->MeshID];
		U16 ps = mesh->PolySetCount;
		while(ps-- > HULL_INDEX)
		{
			polys += mesh->PolySets[ps]->PolygonCount;
		}
	}
	return polys;
}

//**********************************************
//
// MakeTranslationMatrix
//
// Builds a translation matrix which represents
// the location/orientation of the specified
// unit instance
//
//**********************************************
void MakeTranslationMatrix(const RevoltTrackUnitInstance* instance, D3DMATRIX* matrix)
{
	assert(LocalTheme != NULL);
	assert(instance != NULL);

	D3DMATRIX TransMatrix;
	D3DMATRIX RotMatrix;

	D3DUtil_SetRotateYMatrix(RotMatrix, (REAL)((PI * HALF) * instance->Direction));

	D3DUtil_SetTranslateMatrix(TransMatrix, instance->XPos * SMALL_CUBE_SIZE, instance->Elevation * -ElevationStep, instance->YPos * SMALL_CUBE_SIZE);

	D3DMath_MatrixMultiply(*matrix, TransMatrix, RotMatrix);

}

//**********************************************
//
// MakeModuleTranslationMatrix
//
// Builds a translation matrix which represents
// the location/orientation of the specified
// module instance and xpos/ypos
//
//**********************************************
void MakeModuleTranslationMatrix(const RevoltTrackModuleInstance* instance, S16 xpos, S16 ypos, D3DMATRIX* matrix)
{
	assert(LocalTheme != NULL);
	assert(instance != NULL);

	D3DMATRIX TransMatrix;
	D3DMATRIX RotMatrix;

	D3DUtil_SetRotateYMatrix(RotMatrix, (REAL)((PI * HALF) * instance->Direction));

	D3DUtil_SetTranslateMatrix(TransMatrix, xpos * SMALL_CUBE_SIZE, instance->Elevation * -ElevationStep, ypos * SMALL_CUBE_SIZE);

	D3DMath_MatrixMultiply(*matrix, TransMatrix, RotMatrix);

}

//**********************************************
// CountTrackZones
// 
// Counts the total number of track zones
// placed on the track.
//
//**********************************************
U16 CountTrackZones(void)
{
	assert (LocalTrack != NULL);
	assert (LocalTheme != NULL);

	U16 zonecount = 0;
	U16 n = LocalTrack->Width * LocalTrack->Height;
	while(n--)
	{
		INDEX moduleid = LocalTrack->Modules[n].ModuleID;
		if(moduleid != MAX_INDEX)
		{
			zonecount += LocalTheme->Modules[moduleid]->ZoneCount;
		}
	}
	return zonecount;
}

//**********************************************
//
// RotateRevoltVertex
//
//
//**********************************************
void RotateRevoltVertex(RevoltVertex* vert, D3DMATRIX& matrix)
{
	D3DVECTOR temp;
	temp.x = vert->X;
	temp.y = vert->Y;
	temp.z = vert->Z;
	D3DMath_VectorMatrixMultiply( temp, temp, matrix);
	vert->X = temp.x;
	vert->Y = temp.y;
	vert->Z = temp.z;
	if(fabs(vert->X) < Real(0.25f))
	{
		vert->X = ZERO;
	}
	if(fabs(vert->Y) < Real(0.25f))
	{
		vert->Y = ZERO;
	}
	if(fabs(vert->Z) < Real(0.25f))
	{
		vert->Z = ZERO;
	}
}

//**********************************************
//
// CreateZoneList
//
// Builds a list of zones, transformed to their
// correct positions on the track.
//
//**********************************************
void CreateZoneList(void)
{
	S16 xpos, ypos;
	D3DMATRIX modulematrix;
	ZonesInTrack = CountTrackZones();
	if(ZonesInTrack)
	{
		ZoneBuffer = new TRACKZONE[ZonesInTrack];
		ZoneSequence = new ZONESEQENTRY[ZonesInTrack];
		TRACKZONE* destzone = ZoneBuffer;
		U16 n = LocalTrack->Width * LocalTrack->Height;
		while(n--)
		{
			RevoltTrackModuleInstance* instance = &LocalTrack->Modules[n];
			INDEX moduleid = instance->ModuleID;
			if(moduleid != MAX_INDEX)
			{
				xpos = n % LocalTrack->Width;
				ypos = n / LocalTrack->Width;
				TRACKMODULE* module = LocalTheme->Modules[moduleid];
				MakeModuleTranslationMatrix(instance, xpos, ypos, &modulematrix);
				U16 i = module->ZoneCount;
				while(i--)
				{
					const TRACKZONE* srczone = module->Zones[i];
					destzone->Centre.X = srczone->Centre.X;
					destzone->Centre.Y = srczone->Centre.Y;
					destzone->Centre.Z = srczone->Centre.Z;
					destzone->XSize = srczone->XSize;
					destzone->YSize = srczone->YSize;
					destzone->ZSize = srczone->ZSize;
					destzone->Links[0].Position.X = srczone->Links[0].Position.X;
					destzone->Links[0].Position.Y = srczone->Links[0].Position.Y;
					destzone->Links[0].Position.Z = srczone->Links[0].Position.Z;
					destzone->Links[1].Position.X = srczone->Links[1].Position.X;
					destzone->Links[1].Position.Y = srczone->Links[1].Position.Y;
					destzone->Links[1].Position.Z = srczone->Links[1].Position.Z;
					RotateRevoltVertex(&destzone->Centre, modulematrix);
					RotateRevoltVertex(&destzone->Links[0].Position, modulematrix);
					RotateRevoltVertex(&destzone->Links[1].Position, modulematrix);
					if((instance->Direction == EAST) || (instance->Direction == WEST))
					{
						REAL temp = destzone->XSize;
						destzone->XSize = destzone->ZSize;
						destzone->ZSize = temp;
					}
					destzone++;
				}
			}
		}
	}
}

//**********************************************
// CuboidContainsVector
// 
// Simply returns true/false depending on whether
// the vector is inside the bounds of the sphere
// or not.
//
//**********************************************
bool SphereContainsVector(const BIG_CUBE_HEADER_LOAD* sphere, const D3DVECTOR* vect)
{
	assert(sphere != NULL);
	assert(vect != NULL);

	REAL xdist = fabs(sphere->x - vect->x);	//get the distance from the center
	REAL ydist = fabs(sphere->y - vect->y);	//of the sphere to the vector
	REAL zdist = fabs(sphere->z - vect->z);	//for each of the 3 axes

	//get the square of the radius
	REAL limit = (sphere->Radius * sphere->Radius);
	//now calculate the square of the distance to the vector
	REAL distance = ((xdist * xdist) + (ydist * ydist) + (zdist * zdist));
							
	//then decide if it inside the sphere
	return (distance < limit);
}

//**********************************************
// CuboidContainsVector
// 
// Simply returns true/false depending on whether
// the vector is inside the bounds of the cuboid
// or not.
//
//**********************************************
bool CuboidContainsVector(const CUBE_HEADER_LOAD* cuboid, const D3DVECTOR* vect)
{
	assert(cuboid != NULL);
	assert(vect != NULL);

	if((vect->x >= cuboid->Xmin) && (vect->x <= cuboid->Xmax))
	{
		if((vect->y >= cuboid->Ymin) && (vect->y <= cuboid->Ymax))
		{
			if((vect->z >= cuboid->Zmin) && (vect->z <= cuboid->Zmax))
			{
				return true;
			}
		}
	}
	return false;
}

//**********************************************
// CuboidContainsPoly
// 
// Determines whether a poly falls within the
// bounds of the specified cuboid
// A polygons is deemed to be in the cuboid
// if the average of all of its vertices falls
// inside
//
//**********************************************
bool CuboidContainsPoly(const CUBE_HEADER_LOAD* cuboid, const BASICPOLY* poly, const D3DMATRIX* matrix)
{
	assert(LocalTheme != NULL);
	assert(cuboid != NULL);
	assert(poly != NULL);
	assert(matrix != NULL);

	D3DVECTOR result(ZERO, ZERO, ZERO);
	D3DVECTOR shiftedresult;
	D3DVECTOR *vect;
	
	//find the average of the polygon vertices 
	U16 v = VertsInPoly(poly);
	while(v--)
	{
		vect = &LocalTheme->Verts[poly->Vertices[v]];
		result.x += vect->x;
		result.y += vect->y;
		result.z += vect->z;
	}

	v = VertsInPoly(poly);

	result.x /= v;
	result.y /= v;
	result.z /= v;

	//transform it into world coordinates
	D3DMath_VectorMatrixMultiply( shiftedresult, result, (D3DMATRIX&)*matrix);

	//and check that vector against the cuboid
	return CuboidContainsVector(cuboid, &shiftedresult);
}

//**********************************************
// PolysInCuboid
// 
// Counts the number of polygons from the 
// specified polyset which fall into the
// specified cuboid.
// Also works out how many vertices are involved
// Adds the results to the PolyNum & VertNum
// members of the cuboid so these should be zeroed
// before calling this function for the first time 
// for any given cuboid, subsequent calls will
// accumulate the results
//
//**********************************************
void CountPolysAndVertsInCuboid(const POLYSET* polyset, CUBE_HEADER_LOAD* cuboid, const D3DMATRIX* matrix)
{
	assert(LocalTheme != NULL);
	assert(polyset != NULL);
	assert(cuboid != NULL);
	assert(matrix != NULL);

	S16 polycount = cuboid->PolyNum;
	U16 vertcount = cuboid->VertNum;
	U32 p = polyset->PolygonCount;

	while(p--)
	{
		BASICPOLY* poly = &LocalTheme->Polys[polyset->Indices[p]];
		if(CuboidContainsPoly(cuboid, poly, matrix))
		{
			polycount++;
			vertcount += VertsInPoly(poly);
		}
	}
	cuboid->PolyNum = polycount;
	cuboid->VertNum = vertcount;
}

//**********************************************
//
// CubesInTrack
//
// Calculates how many cubes are required to bound 
// the entire track. This is achieved by working
// out how many cubes need to be used to bound
// each cuboid then adding them all together.
//
//**********************************************
U16 CubesInTrack(void)
{
	assert(LocalTrack != NULL);
	assert(UnitCuboids != NULL);

	static D3DVECTOR translation(ZERO, ZERO, ZERO);

	U16 cubes = 0;
	U32 m = LocalTrack->Width * LocalTrack->Height;
	CUBE_HEADER_LOAD cuboid;

	while(m--)
	{
		CopyCuboid(&cuboid, &UnitCuboids[LocalTrack->Units[m].UnitID]);	//get a copy of the cuboid for the unit at this position
		translation.y = (LocalTrack->Units[m].Elevation * -ElevationStep);	//work out how high off the ground it is
		if(translation.y != ZERO)							//if it is off the ground
		{
			TranslateCuboid(&translation, &cuboid);			//shift the cuboid to the appropriate height
			cuboid.Ymax = ZERO;								//and extend it back down to the ground
		}
		cubes += CubesInCuboid(&cuboid);
	}
	return cubes;
}

//**********************************************
//
//**********************************************
void AddPolyAndVertCountForPegRings(CUBE_HEADER_LOAD* cuboid, S16 Elevation)
{
	if(Elevation != 0)
	{
		cuboid->PolyNum += 4;	//four polygons
		cuboid->VertNum += 16;	//contianing 4 uniques vertices each
	}	
}

//**********************************************
//
//**********************************************
void CalculateNormal(const WORLD_VERTEX_LOAD* p0, const WORLD_VERTEX_LOAD* p1, const WORLD_VERTEX_LOAD* p2, D3DVECTOR* normal)
{
	assert(p0 != NULL);
	assert(p1 != NULL);
	assert(p2 != NULL);
	assert(normal  != NULL);

	D3DVECTOR	vect0;
	D3DVECTOR	vect1;

	vect0.x = p0->x - p1->x;
	vect0.y = p0->y - p1->y;
	vect0.z = p0->z - p1->z;

	vect1.x = p2->x - p1->x;
	vect1.y = p2->y - p1->y;
	vect1.z = p2->z - p1->z;

	D3DRMVectorCrossProduct(normal, &vect0, &vect1);
	D3DRMVectorNormalize(normal);
}

//**********************************************
//
//**********************************************
void CalculateD3DNormal(const D3DVECTOR* p0, const D3DVECTOR* p1, const D3DVECTOR* p2, D3DVECTOR* normal)
{
	assert(p0 != NULL);
	assert(p1 != NULL);
	assert(p2 != NULL);
	assert(normal  != NULL);

	D3DVECTOR	vect0;
	D3DVECTOR	vect1;

	vect0.x = p0->x - p1->x;
	vect0.y = p0->y - p1->y;
	vect0.z = p0->z - p1->z;

	vect1.x = p2->x - p1->x;
	vect1.y = p2->y - p1->y;
	vect1.z = p2->z - p1->z;

	D3DRMVectorCrossProduct(normal, &vect0, &vect1);
	D3DRMVectorNormalize(normal);
}

//**********************************************
//
//**********************************************
void FindCuboidCentreAndRadius(CUBE_HEADER_LOAD* cuboid)
{
	assert(cuboid != NULL);

	cuboid->CentreX = (cuboid->Xmin + cuboid->Xmax) / Real(2.0f);
	cuboid->CentreY = (cuboid->Ymin + cuboid->Ymax) / Real(2.0f);
	cuboid->CentreZ = (cuboid->Zmin + cuboid->Zmax) / Real(2.0f);
	REAL xsize = cuboid->CentreX - cuboid->Xmin;
	REAL ysize = cuboid->CentreY - cuboid->Ymin;
	REAL zsize = cuboid->CentreZ - cuboid->Zmin;

	cuboid->Radius = sqrt((xsize * xsize) + (ysize * ysize) + (zsize * zsize));

	cuboid->Xmin = cuboid->CentreX - xsize;
	cuboid->Ymin = cuboid->CentreY - ysize;
	cuboid->Zmin = cuboid->CentreZ - zsize;
	cuboid->Xmax = cuboid->CentreX + xsize;
	cuboid->Ymax = cuboid->CentreY + ysize;
	cuboid->Zmax = cuboid->CentreZ + zsize;
}

//**********************************************
//
//**********************************************
bool AreCubesAdjacent(const SMALLCUBE* cube1, const SMALLCUBE* cube2)
{
	assert(cube1 != NULL);
	assert(cube2 != NULL);

	bool adjacent = false;
	REAL xdist = fabs(cube1->Header.CentreX - cube2->Header.CentreX);
	REAL zdist = fabs(cube1->Header.CentreZ - cube2->Header.CentreZ);
	xdist *= xdist;
	zdist *= zdist;
	REAL dist = xdist + zdist;
	if(dist > 1)	//if the distance is not small (i.e we are not checking against self or cube with similar Y coord)
	{
		adjacent = (ApproxEqual(dist, SMALL_CUBE_SIZE * SMALL_CUBE_SIZE) == TRUE);	//decide if the cube is adjacent or not
	}

	return adjacent;
}

//**********************************************
//
//**********************************************
void CopyPoly(WORLD_POLY_LOAD* dest, const WORLD_POLY_LOAD* src)
{
	assert(dest != NULL);
	assert(src != NULL);

	dest->Type = src->Type;
	dest->Tpage = src->Tpage;
	dest->vi0 = src->vi0;
	dest->vi1 = src->vi1;
	dest->vi2 = src->vi2;
	dest->vi3 = src->vi3;
	dest->c0 = src->c0;
	dest->c1 = src->c1;
	dest->c2 = src->c2;
	dest->c3 = src->c3;
	dest->u0 = src->u0;
	dest->v0 = src->v0;
	dest->u1 = src->u1;
	dest->v1 = src->v1;
	dest->u2 = src->u2;
	dest->v2 = src->v2;
	dest->u3 = src->u3;
	dest->v3 = src->v3;

}
//**********************************************
// RemoveCubePoly
//
// 'Removes' a polygon from a cube by copying the
// last polygon in the array over it and then
// decrementing the polygon count.
// This method is quick as it doesn't need to
// re-allocate memory or copy large numbers of
// array elements .
// NOTE - if the element to be deleted IS the
// last in the array then there is no need to
// do the copy
//
//**********************************************
void RemoveCubePoly(SMALLCUBE* cube, INDEX index)
{
	assert(cube != NULL);				  //assure that the cube pointer does point somewhere
	assert(index < cube->Header.PolyNum); //only allow indices which are within the size of the array

	if(index < (cube->Header.PolyNum - 1))	//if the poly to be deleted is not the last in the array
	{
		WORLD_POLY_LOAD* srcpoly = &cube->Polys[cube->Header.PolyNum - 1];	//get the address of the last in the array
		WORLD_POLY_LOAD* destpoly = &cube->Polys[index];					//and the address of the poly to be 'removed'
		CopyPoly(destpoly, srcpoly);										//then copy the last poly over the top of the 'removed' one
	}
	cube->Header.PolyNum--; //decrement the polygon count for the cube
}

//**********************************************
//
//**********************************************
bool NormalsOpposite(const WORLD_VERTEX_LOAD* vert1, const WORLD_VERTEX_LOAD* vert2)
{
	if(ApproxEqual(vert1->nx, -vert2->nx) == FALSE)
	{
		return false;
	}
	if(ApproxEqual(vert1->ny, -vert2->ny) == FALSE)
	{
		return false;
	}
	return (ApproxEqual(vert1->nz, -vert2->nz) == TRUE);
}

//**********************************************
//
//**********************************************
bool VertsApprox(const WORLD_VERTEX_LOAD* vert1, const WORLD_VERTEX_LOAD* vert2)
{
	if(ApproxEqual(vert1->x, vert2->x) == FALSE)
	{
		return false;
	}
	if(ApproxEqual(vert1->y, vert2->y) == FALSE)
	{
		return false;
	}
	return (ApproxEqual(vert1->z, vert2->z) == TRUE);
}

//**********************************************
//
//**********************************************
bool PlanesApproxOpposite(const PLANE* plane1, const PLANE* plane2)
{
	if(ApproxEqual(plane1->v[0], -plane2->v[0]) == FALSE)
	{
		return false;
	}
	if(ApproxEqual(plane1->v[1], -plane2->v[1]) == FALSE)
	{
		return false;
	}
	if(ApproxEqual(plane1->v[2], -plane2->v[2]) == FALSE)
	{
		return false;
	}
	return (ApproxEqual(plane1->v[3], -plane2->v[3]) == TRUE);
}

//**********************************************
//
//**********************************************
bool PlanesApprox(const PLANE* plane1, const PLANE* plane2)
{
	if(ApproxEqual(plane1->v[0], plane2->v[0]) == FALSE)
	{
		return false;
	}
	if(ApproxEqual(plane1->v[1], plane2->v[1]) == FALSE)
	{
		return false;
	}
	if(ApproxEqual(plane1->v[2], plane2->v[2]) == FALSE)
	{
		return false;
	}
	return (ApproxEqual(plane1->v[3], plane2->v[3]) == TRUE);
}

//**********************************************
//
//**********************************************
void RemoveMatchingPolys(SMALLCUBE* testcube, SMALLCUBE* currentcube)
{
	INDEX currentindex = currentcube->Header.PolyNum;
	while(currentindex--)
	{
		WORLD_POLY_LOAD* currentpoly = &currentcube->Polys[currentindex];
		if(currentpoly->Tpage == TPAGE_NONE)
		{
			WORLD_VERTEX_LOAD* currentvert = &currentcube->Verts[currentpoly->vi0];
			U16 vertcount = 3;
			if(currentpoly->Type & POLY_QUAD)
			{
				vertcount++;
			}
			INDEX testindex = testcube->Header.PolyNum;
			bool match = false;
			while((testindex--) && (match == false))
			{
				WORLD_POLY_LOAD* testpoly = &testcube->Polys[testindex];
				if(testpoly->Tpage == TPAGE_NONE)
				{
					if((currentpoly->Type & POLY_QUAD) == (testpoly->Type & POLY_QUAD))
					{
						WORLD_VERTEX_LOAD* testvert = &testcube->Verts[testpoly->vi0];
						if(NormalsOpposite(testvert, currentvert))
						{
							short* currentindices = &currentpoly->vi0;
							short* testindices = &testpoly->vi0;
							S16 cv = 0;
							S16 tv = vertcount;
							while(tv--)
							{
								if(VertsApprox(&testcube->Verts[testindices[tv]], &currentcube->Verts[currentindices[cv]]))
								{
									match = true;
									do{
										cv++;
										tv += vertcount-1;
										tv %= vertcount;
										if(VertsApprox(&testcube->Verts[testindices[tv]], &currentcube->Verts[currentindices[cv]]) == false)
										{	
											match = false;
										}
									}while((cv < (vertcount - 1)) && (match == true));

									if(match == true)
									{
										RemoveCubePoly(testcube, testindex);
										RemoveCubePoly(currentcube, currentindex);
									}
								}
							}
						}
					}
				}
			}
		}
	}
}

//**********************************************
//
//**********************************************
bool PolyContainsPlane(const NEWCOLLPOLY* poly, const PLANE* plane)
{
	U16 n = 3;
	if(poly->Type & QUAD)
	{
		n++;
	}
	while(n--)
	{
		if(PlanesApprox(plane, &poly->EdgePlane[n]))
		{
			return true;
		}
	}
	return false;
}

//**********************************************
//
//**********************************************
bool CollPolysMatch(const NEWCOLLPOLY* poly1, const NEWCOLLPOLY* poly2)
{
	bool match = false;
	if((poly1->Type & QUAD) == (poly2->Type & QUAD))
	{
		if(PlanesApproxOpposite(&poly1->Plane, &poly2->Plane))
		{
			U16 n = 3;
			if(poly1->Type & QUAD)
			{
				n++;
			}
			match = true;
			while(n-- && (match == true))
			{
				match = PolyContainsPlane(poly2, &poly1->EdgePlane[n]);
			}
		}
	}
	return match;
}

//**********************************************
//
//**********************************************
void RemoveSurplusWorldPolys(void)
{
	U16 currentcubeindex = SmallCubeCount;
	while(currentcubeindex--)
	{
		U16 testcubeindex = currentcubeindex;
		while(testcubeindex--)
		{
			SMALLCUBE* testcube = &SmallCubes[testcubeindex];
			SMALLCUBE* currentcube = &SmallCubes[currentcubeindex];
			bool adjacent = AreCubesAdjacent(testcube, currentcube);
			if(adjacent == true)
			{
				RemoveMatchingPolys(testcube, currentcube);
			}
		}
	}
}

//**********************************************
//
//**********************************************
void RemoveSurplusCollisionPolys(void)
{
	for(S16 cp = 0; cp < (CollisionData.NumPolys); cp++)
	{
		CollisionData.PolyStatus[cp] = 1;
	}
	NEWCOLLPOLY* currentpoly = CollisionData.Polys;
	for(cp = 0; cp < (CollisionData.NumPolys - 1); cp++)
	{
		if(CollisionData.PolyStatus[cp] == 1)
		{
			NEWCOLLPOLY* testpoly = currentpoly;
			for(S16 tp = cp + 1; tp < CollisionData.NumPolys; tp++)
			{
				testpoly++;
				if(CollisionData.PolyStatus[tp] == 1)
				{
					if(CollPolysMatch(currentpoly, testpoly) == true)
					{
						CollisionData.PolyStatus[cp] = 0;
						CollisionData.PolyStatus[tp] = 0;
						break;
					}
				}
			}
		}
		currentpoly++;
	}
	for(cp = 0; cp < (CollisionData.NumPolys); cp++)
	{
		if(CollisionData.PolyStatus[cp] == 0)
		{
			CollisionData.NumUsedPolys--;
		}
	}
	U32 cellcount = LocalTrack->Width * LocalTrack->Height;
	CELLDATA* cell = CollisionData.Cells;
	for(U32 cellnum = 0; cellnum < cellcount; cellnum++)
	{
		if(cell->NumEntries)
		{
			S32 lastentry = cell->NumEntries - 1;
			while(CollisionData.PolyStatus[cell->CollPolyIndices[lastentry]] == 0)
			{
				lastentry--;
				cell->NumEntries--;
			}
			S32 n = lastentry;
			while(n--)
			{
				S32 index = cell->CollPolyIndices[n];
				if(CollisionData.PolyStatus[index] == 0)
				{
					cell->CollPolyIndices[n] = cell->CollPolyIndices[lastentry];
					lastentry--;
					cell->NumEntries--;
				}
			}
		}
		cell++;
	}
}

//**********************************************
//
//**********************************************
void RemoveSurplusPolys(void)
{
	RemoveSurplusWorldPolys();
	RemoveSurplusCollisionPolys();
}

//**********************************************
//
//**********************************************
void FillSmallCube(SMALLCUBE* cube, const TRACKUNIT* unit, const D3DMATRIX* matrix)
{
	assert(LocalTheme != NULL);
	assert(cube != NULL);
	assert(unit != NULL);
	assert(matrix != NULL);

	MESH* mesh = &LocalTheme->Meshes[unit->MeshID];

	CUBE_HEADER_LOAD adjustedcuboid;				//create a cube for growing into the final size
	CopyCuboid(&adjustedcuboid, &cube->Header);		//initialize it to the size of the actual cube
	cube->Polys = new WORLD_POLY_LOAD[cube->Header.PolyNum];
	cube->Verts = new WORLD_VERTEX_LOAD[cube->Header.VertNum];
		
	WORLD_POLY_LOAD*	currentpoly = cube->Polys;
	WORLD_VERTEX_LOAD*	currentvert = cube->Verts;
	
	U16 vertindex = 0;
	D3DVECTOR normal;

	for(U16 ps = 0; ps <= PAN_INDEX; ps++)
	{
		POLYSET* polyset = mesh->PolySets[ps];
		U32 p = polyset->PolygonCount;
		while(p--)
		{
			BASICPOLY* poly = &LocalTheme->Polys[polyset->Indices[p]];
			if(CuboidContainsPoly(&cube->Header, poly, matrix))
			{
				currentpoly->Type = poly->IsTriangle ? 0 : POLY_QUAD;

				currentpoly->vi0 = vertindex + 2;
				currentpoly->vi1 = vertindex + 1;
				currentpoly->vi2 = vertindex;
				currentpoly->vi3 = vertindex + 3;

				U16 vertcount = VertsInPoly(poly);
				for(U16 v = 0; v < vertcount; v++)
				{
					D3DVECTOR vect;
					D3DMath_VectorMatrixMultiply( vect, LocalTheme->Verts[poly->Vertices[v]], (D3DMATRIX&)*matrix);

					currentvert->x = vect.x;
					currentvert->y = vect.y;
					currentvert->z = vect.z;
					currentvert++;
					AddVectorToCuboid(&vect, &adjustedcuboid);	//update the adjusted cuboid
				}
				currentvert -= vertcount;	//wind back to first vertex

				CalculateNormal(&currentvert[0], &currentvert[1], &currentvert[2], &normal);
				for(v = 0; v < vertcount; v++)
				{
					currentvert->nx = normal.x;
					currentvert->ny = normal.y;
					currentvert->nz = normal.z;
					currentvert++;
				}
				currentvert -= vertcount;	//wind back to first vertex

				if(ps == PAN_INDEX)
				{
					currentpoly->Tpage = unit->UVPolys[p].TPageID-1;
					BASICPOLY* uvpoly = &LocalTheme->UVPolys[unit->UVPolys[p].PolyID];
					U32 uvmodvalue = uvpoly->IsTriangle ? 3 : 4;
					U32 uvindex = unit->UVPolys[p].Rotation;
					U32 uvstep = unit->UVPolys[p].Reversed ? (uvmodvalue - 1) : 1;

					currentpoly->u2  = LocalTheme->UVCoords[uvpoly->Vertices[uvindex]].U;
					currentpoly->v2  = LocalTheme->UVCoords[uvpoly->Vertices[uvindex]].V;
					uvindex += uvstep;
					uvindex %= uvmodvalue;

					currentpoly->u1  = LocalTheme->UVCoords[uvpoly->Vertices[uvindex]].U;
					currentpoly->v1  = LocalTheme->UVCoords[uvpoly->Vertices[uvindex]].V;
					uvindex += uvstep;
					uvindex %= uvmodvalue;

					currentpoly->u0  = LocalTheme->UVCoords[uvpoly->Vertices[uvindex]].U;
					currentpoly->v0  = LocalTheme->UVCoords[uvpoly->Vertices[uvindex]].V;
					uvindex += uvstep;
					uvindex %= uvmodvalue;

					currentpoly->u3  = LocalTheme->UVCoords[uvpoly->Vertices[uvindex]].U;
					currentpoly->v3  = LocalTheme->UVCoords[uvpoly->Vertices[uvindex]].V;
					uvindex += uvstep;
					uvindex %= uvmodvalue;

					long gun = (long)(((normal.y + ONE) / Real(2.0f)) * 255);
					long color = 0xff000000 | gun | (gun << 8) | (gun << 16);
					currentpoly->c0 = currentpoly->c1 = currentpoly->c2 = currentpoly->c3 = color;
				}
				else
				{
					currentpoly->Tpage = TPAGE_NONE;
					currentpoly->u0	= currentpoly->u1 = currentpoly->u2	= currentpoly->u3 = ZERO;
					currentpoly->v0	= currentpoly->v1 = currentpoly->v2	= currentpoly->v3 = ZERO;
					const REAL maximumy = MAX_ELEVATION * ElevationStep;
					long gun;
					long color;
					gun = (long)(((fabs(currentvert[2].y) / maximumy) * 128) + 64);
					color = 0xff000000 | gun | (gun << 8) | (gun << 16);
					currentpoly->c0 = color;
					gun = (long)(((fabs(currentvert[1].y) / maximumy) * 128) + 64);
					color = 0xff000000 | gun | (gun << 8) | (gun << 16);
					currentpoly->c1 = color;
					gun = (long)(((fabs(currentvert[0].y) / maximumy) * 128) + 64);
					color = 0xff000000 | gun | (gun << 8) | (gun << 16);
					currentpoly->c2 = color;
					gun = (long)(((fabs(currentvert[3].y) / maximumy) * 128) + 64);
					color = 0xff000000 | gun | (gun << 8) | (gun << 16);
					currentpoly->c3 = color;
				}
	
				currentvert += vertcount;	//wind back to first vertex
				vertindex += vertcount;

				currentpoly++;
			}
		}
	}
	if(vertindex < cube->Header.VertNum)
	{
		for(U16 v = 0; v < 16; v++)
		{
			D3DVECTOR vect;

			D3DMath_VectorMatrixMultiply( vect, UnitRootVects[v], (D3DMATRIX&)*matrix);

			currentvert->x = vect.x;
			currentvert->y = vect.y;
			currentvert->z = vect.z;
			currentvert++;
			AddVectorToCuboid(&vect, &adjustedcuboid);	//update the adjusted cuboid
		}

		currentvert -= 16;	//wind back to first vertex

		for(U16 p = 0; p < 4; p++)
		{
			CalculateNormal(&currentvert[9], &currentvert[1], &currentvert[0], &normal);
			for(v = 0; v < 2; v++)
			{
				currentvert[0].nx = normal.x;
				currentvert[0].ny = normal.y;
				currentvert[0].nz = normal.z;
				currentvert[8].nx = normal.x;
				currentvert[8].ny = normal.y;
				currentvert[8].nz = normal.z;
				currentvert++;
			}
		}

		for(U16 n = 0; n < 4; n++)
		{
			currentpoly->Type = POLY_QUAD;
			currentpoly->vi0 = vertindex + 9;
			currentpoly->vi1 = vertindex + 8;
			currentpoly->vi2 = vertindex + 0;
			currentpoly->vi3 = vertindex + 1;
			currentpoly->Tpage = TPAGE_NONE;
			currentpoly->u0	= currentpoly->u1 = currentpoly->u2	= currentpoly->u3 = ZERO;
			currentpoly->v0	= currentpoly->v1 = currentpoly->v2	= currentpoly->v3 = ZERO;

			const REAL maximumy = MAX_ELEVATION * ElevationStep;
			long gun;
			long color;
			gun = (long)(((fabs(currentvert->y) / maximumy) * 128) + 64);
			color = 0xff000000 | gun | (gun << 8) | (gun << 16);
			currentpoly->c0 = color;
			currentpoly->c1 = color;
			gun = (long)(64);
			color = 0xff000000 | gun | (gun << 8) | (gun << 16);
			currentpoly->c2 = color;
			currentpoly->c3 = color;

			currentpoly++;
			currentvert +=2;
			vertindex += 2;
		}
	}
	CopyCuboid(&cube->Header, &adjustedcuboid);
	FindCuboidCentreAndRadius(&cube->Header);
}

//**********************************************
//
//**********************************************
void FillSmallCubes(void)
{
	assert(LocalTrack != NULL);
	assert(LocalTheme != NULL);

	static D3DVECTOR cubestep(ZERO, SMALL_CUBE_SIZE, ZERO);
	static D3DVECTOR translation(ZERO, ZERO, ZERO);

	D3DMATRIX	objectmatrix;

	SMALLCUBE*	currentcube = SmallCubes;

	U32 n = 0;
	U32 m = LocalTrack->Width * LocalTrack->Height;
	while(m--)
	{
		MakeTranslationMatrix(&LocalTrack->Units[m], &objectmatrix);

		CUBE_HEADER_LOAD cuboid;
		CUBE_HEADER_LOAD cube;

		CopyCuboid(&cuboid, &UnitCuboids[LocalTrack->Units[m].UnitID]);

		translation.x = LocalTrack->Units[m].XPos * SMALL_CUBE_SIZE;
		translation.y = LocalTrack->Units[m].Elevation * -ElevationStep;
		translation.z = LocalTrack->Units[m].YPos * SMALL_CUBE_SIZE;

		TranslateCuboid(&translation, &cuboid); //move the cuboid to its correct position
		if(translation.y != ZERO)							//if it is off the ground
		{
			cuboid.Ymax = ZERO;	//extend it back to the ground
		}
		for(U16 v = 0; v < 8; v++)
		{
			UnitRootVects[v].y = -translation.y;
		}
		
		CopyCuboid(&cube, &cuboid);					//take a copy of the cuboid
		cube.Ymax = cube.Ymin + SMALL_CUBE_SIZE;	//truncate it (vertically) into a regular cube

		TRACKUNIT* unit = LocalTheme->Units[LocalTrack->Units[m].UnitID];

		MESH* mesh = &LocalTheme->Meshes[unit->MeshID];

		U16 cubecount = CubesInCuboid(&cuboid);
	
		POLYSET* polyset;
		
		while(cubecount--)
		{
			cube.PolyNum = 0;	//assume zero polys in this cube
			cube.VertNum = 0;	//and zero vertices
			polyset = mesh->PolySets[PEG_INDEX];
			CountPolysAndVertsInCuboid(polyset, &cube, &objectmatrix);
			polyset = mesh->PolySets[PAN_INDEX];
			CountPolysAndVertsInCuboid(polyset, &cube, &objectmatrix);
			
			AddPolyAndVertCountForPegRings(&cube, LocalTrack->Units[m].Elevation);

			CopyCuboid(&currentcube->Header, &cube);
			currentcube->Header.PolyNum = cube.PolyNum;
			currentcube->Header.VertNum = cube.VertNum;

			if(currentcube->Header.PolyNum == 0)
			{
				currentcube->Polys = NULL;
				currentcube->Verts = NULL;
			}
			else
			{
				FillSmallCube(currentcube, unit, &objectmatrix);
			}

			currentcube++;
			n++;
			TranslateCuboid(&cubestep, &cube);		//move the working cube up a level
		}
	}
}

//**********************************************
// 
// MakeWorldCuboid
//
// Calculates the bounding cuboid of all of the
// small cubes
// 
//**********************************************
void MakeWorldCuboid(CUBE_HEADER_LOAD* cuboid)
{
	assert(SmallCubes != NULL);
	assert(cuboid != NULL);

	D3DVECTOR vect;
	
	NullifyCuboid(cuboid);
	
	SMALLCUBE* currentcube = SmallCubes;
	U32 n = SmallCubeCount;
	while(n--)
	{
		vect.x = currentcube->Header.Xmin;		
		vect.y = currentcube->Header.Ymin;		
		vect.z = currentcube->Header.Zmin;
		AddVectorToCuboid(&vect, cuboid);
		vect.x = currentcube->Header.Xmax;		
		vect.y = currentcube->Header.Ymax;		
		vect.z = currentcube->Header.Zmax;
		AddVectorToCuboid(&vect, cuboid);
		currentcube++;
	}
}

//********************************************************************************************
//
//	Now we have the buffer allocation / de-allocation routines
//
//********************************************************************************************

//**********************************************
//
//
//
//**********************************************
void LocateStartZone(void)
{
	CurrentZone = MAX_INDEX; //default to no zone found
	U16 n = ZonesInTrack;
	while(n--)
	{
		TRACKZONE* zone = &ZoneBuffer[n];
		REAL xdist = fabs(StartPos.x - zone->Centre.X);
		REAL zdist = fabs(StartPos.z - zone->Centre.Z);
		if((xdist < zone->XSize) && (zdist < zone->ZSize))
		{
			CurrentZone = n;
			break;
		}
	}
}

//**********************************************
//
//
//
//**********************************************
void FindNextZone(INDEX currentzone, U16 currentlink, INDEX* nextzone, U16* nextlink)
{
	*nextzone = MAX_INDEX;	//default to no zone found
	U16 n = ZonesInTrack;
	const TRACKZONE* thiszone = &ZoneBuffer[currentzone];
	TRACKZONE* testzone;
	while(n--)
	{
		if(n != currentzone)	//don't check against self
		{
			testzone = &ZoneBuffer[n];
			U16 link = 2;
			while(link--)
			{
				if(fabs(thiszone->Links[currentlink].Position.X - testzone->Links[link].Position.X) < Real(0.25f))	//x-coords must match
				{
					if(thiszone->Links[currentlink].Position.Y <= testzone->Links[link].Position.Y)	//y-coords can be greater than current one
					{
						if(fabs(thiszone->Links[currentlink].Position.Z - testzone->Links[link].Position.Z) < Real(0.25f))	//z-coords must match
						{
							*nextzone = n;
							*nextlink = link;
						}
					}
				}
			}
		}
	}
}

//**********************************************
//
//
//
//**********************************************
bool CreateZoneInformation(void)
{
	CreateZoneList();
	LocateStartZone();

	INDEX	nextzone;
	U16		exitlink, nextlink;

	ZonesVisited = 0;
	exitlink = 1;	//we are coming out of link b of the start grid
	nextzone = MAX_INDEX;	//default to illegal value

	do
	{
		ZoneSequence[ZonesVisited].ZoneID = CurrentZone;
		ZoneSequence[ZonesVisited].Forwards = (exitlink == 1);
		ZonesVisited++;
		FindNextZone(CurrentZone, exitlink, &nextzone, &nextlink);
		if((nextzone != MAX_INDEX))
		{
			CurrentZone = nextzone;
			exitlink = (1 - nextlink);
		}
		else
		{
			//couldn't find a next zone so we terminate the loop
			break;
		}

	}while((ZonesVisited < ZonesInTrack) && (nextzone != ZoneSequence[0].ZoneID));
	//loop terminates when we have used all of the zones
	//or we reach the start zone again
	return (nextzone == ZoneSequence[0].ZoneID);
}

//**********************************************
//
//
//
//**********************************************
void DestroyZoneInformation(void)
{
	delete[] ZoneSequence;
	ZoneSequence = NULL;
	delete[] ZoneBuffer;
	ZoneBuffer = NULL;
}

//**********************************************
//
//
//
//**********************************************
void MakeModulesDirectional(TRACKDESC* track)
{
	INDEX index = ZonesVisited;
	while(index--)
	{
		TRACKZONE* zone = &ZoneBuffer[ZoneSequence[index].ZoneID];
		S16 xpos = (zone->Centre.X / SMALL_CUBE_SIZE);
		S16 ypos = (zone->Centre.Z / SMALL_CUBE_SIZE);
		U16 n = (ypos * track->Width) + xpos;
		//find the offset to the modules master position
		xpos -= track->Modules[n].XOffset;
		ypos -= track->Modules[n].YOffset;
		
		//calculate the index for this element
		n = (ypos * track->Width) + xpos;
		INDEX moduleid = track->Modules[n].ModuleID;
		if(ZoneSequence[index].Forwards == true)
		{
			track->Modules[n].ModuleID = LocalTheme->Lookup->Changes[moduleid].Forward;
		}
		else
		{
			track->Modules[n].ModuleID = LocalTheme->Lookup->Changes[moduleid].Reverse;
		}
	}
}

//**********************************************
//
//
//
//**********************************************
void CreateUnitCuboids(void)
{
	assert(LocalTheme != NULL);

	U32 n = LocalTheme->UnitCount;

	UnitCuboids = new CUBE_HEADER_LOAD[n];

	TRACKUNIT* unit;
	while(n--)
	{
		unit = LocalTheme->Units[n];			//get a unit
		CuboidOfUnit(unit, &UnitCuboids[n]);
	}
}

//**********************************************
//
//
//
//**********************************************
void DestroyUnitCuboids(void)
{
	delete[] UnitCuboids;
	UnitCuboids = NULL;
}

//**********************************************
//
//
//
//**********************************************
void CreateSmallCubes(void)
{
	SmallCubeCount = CubesInTrack();
	SmallCubes = new SMALLCUBE[SmallCubeCount];
	FillSmallCubes();
}

//**********************************************
//
//
//
//**********************************************
void DestroySmallCubes(void)
{
	if(SmallCubes != NULL)
	{
		for(U16 c = 0; c < SmallCubeCount; c++)
		{
			delete[] SmallCubes[c].Polys;
			delete[] SmallCubes[c].Verts;
		}
		delete[] SmallCubes;
		SmallCubes = NULL;
		SmallCubeCount = 0;
	}
}

//**********************************************
//
//
//
//**********************************************
void CreateBigCubes(void)
{
	CUBE_HEADER_LOAD worldcuboid;
	U16				 xcubes, ycubes, zcubes;
	REAL			 x, y, z;

	MakeWorldCuboid(&worldcuboid);	//create bounding cuboid for the entire world

	BigCubeCount = 0;		//start off with no big cubes

	x = worldcuboid.Xmin;	//start off at left
	y = worldcuboid.Ymin;	//and top
	z = worldcuboid.Zmin;	//and front of world cuboid

	xcubes = ycubes = zcubes = 0;	//zero out number of cubes along each axis

	while(x <= worldcuboid.Xmax)	//if we are not at the right hand edge of the cube
	{
		xcubes ++;					//increment # of cubes on x axis
		x += BIG_CUBE_SIZE;			//move right by one cube
	}
	while(y <= worldcuboid.Ymax)	//if we are not at the bottom edge of the cube
	{
		ycubes ++;					//increment # of cubes on y axis
		y += BIG_CUBE_SIZE;			//move down by one cube
	}
	while(z <= worldcuboid.Zmax)	//if we are not at the back edge of the cube
	{
		zcubes ++;					//increment # of cubes on z axis
		z += BIG_CUBE_SIZE;			//move forward by one cube
	}
	BigCubeCount = xcubes * ycubes * zcubes;	//now calculate the total number of cubes
	BigCubes = new BIGCUBE[BigCubeCount];		//assign memory for that many cubes
	LinkTable = new INDEX[SmallCubeCount];		//assign memory for the link table
												//use a small cube number as an index into this
												//table and you can then read off which big cube
												//that small cube falls into
	for(U16 n = 0; n < SmallCubeCount; n++)		//initialize the table to impossible values
	{
		LinkTable[n] = MAX_INDEX;
	}

	x = worldcuboid.Xmin;						
	y = worldcuboid.Ymin;
	z = worldcuboid.Zmin;

	BIGCUBE* bigcube = BigCubes;
	for(U16 b = 0; b < BigCubeCount; b++)
	{
		bigcube->Header.CubeNum = 0;
		bigcube->Header.x = x + (BIG_CUBE_SIZE / Real(2.0f));
		bigcube->Header.y = y + (BIG_CUBE_SIZE / Real(2.0f));
		bigcube->Header.z = z + (BIG_CUBE_SIZE / Real(2.0f));
		bigcube->Header.Radius = BIG_CUBE_RADIUS;
				
		SMALLCUBE* smallcube = SmallCubes;
		D3DVECTOR testvector;
		for(U16 s = 0; s < SmallCubeCount; s++)
		{
			testvector.x = smallcube->Header.CentreX;
			testvector.y = smallcube->Header.CentreY;
			testvector.z = smallcube->Header.CentreZ;
			if(SphereContainsVector(&bigcube->Header, &testvector) == true)
			{
				if(LinkTable[s] == MAX_INDEX)
				{
					LinkTable[s] = b;
					bigcube->Header.CubeNum++;
				}
			}
			smallcube++;
		}
		bigcube++;
		x += BIG_CUBE_SIZE;
		if(x >= worldcuboid.Xmax)
		{
			x = worldcuboid.Xmin;
			y += BIG_CUBE_SIZE;
			if(y >= worldcuboid.Ymax)
			{
				y = worldcuboid.Ymin;
				z += BIG_CUBE_SIZE;
			}
		}
	}
	bigcube = BigCubes;
	for(b = 0; b < BigCubeCount; b++)
	{
		bigcube->CubeIndices = new U32[bigcube->Header.CubeNum];
		U16 n = 0;
		for(U16 s = 0; s < SmallCubeCount; s++)
		{
			if(LinkTable[s] == b)
			{
				bigcube->CubeIndices[n++] = s;
			}
		}
		bigcube++;
	}
}

//**********************************************
//
//
//
//**********************************************
void DestroyBigCubes(void)
{
	delete[] LinkTable;
	LinkTable = NULL;
	if(BigCubes != NULL)
	{
		while(BigCubeCount--)
		{
			delete[] BigCubes[BigCubeCount].CubeIndices;
		}
		delete[] BigCubes;
		BigCubes = NULL;
	}
}

//**********************************************
//
//
//
//**********************************************
void FillPlane(PLANE* plane, const D3DVECTOR* vect0, const D3DVECTOR* vect1, const D3DVECTOR* vect2)
{
	D3DVECTOR normal;
	//first calculate the normal to the 3 vectors supplied
	CalculateD3DNormal(vect0, vect1, vect2, &normal);	
	
	//now work out the shortest distance from the poly to the world origin
	REAL d = -((vect0->x * normal.x) + (vect0->y * normal.y) + (vect0->z * normal.z));
	
	//stuff the answers into the plane structure
	REAL l = D3DRMVectorDotProduct(&normal, &normal);
	plane->v[0] = normal.x;
	plane->v[1] = normal.y;
	plane->v[2] = normal.z;
	plane->v[3] = d;
}

//**********************************************
//
//
//
//**********************************************
bool NormalIsHorizontal(D3DVECTOR& normal)
{
	bool horizontal = false;
	if(ApproxEqual(fabs(normal.x), ONE) == TRUE)
	{
		if(ApproxEqual(normal.z, ZERO) == TRUE)
		{
			if(ApproxEqual(normal.y, ZERO) == TRUE)
			{
				horizontal = true;
			}
		}
	}
	else
	{
		if(ApproxEqual(fabs(normal.z), ONE) == TRUE)
		{
			if(ApproxEqual(normal.x, ZERO) == TRUE)
			{
				if(ApproxEqual(normal.y, ZERO) == TRUE)
				{
					horizontal = true;
				}
			}
		}
	}
	return horizontal;
}

//**********************************************
//
//
//
//**********************************************
void FillCollisionPoly(NEWCOLLPOLY* collpoly, const BASICPOLY* poly, const D3DMATRIX* matrix)
{
	if(poly->IsTriangle == TRUE)
	{
		collpoly->Type = 0;
	}
	else
	{
		collpoly->Type = QUAD;
	}
	D3DVECTOR vect[4];
	D3DVECTOR tempvect;
	D3DVECTOR normal;
	U16 vertcount = VertsInPoly(poly);
	for(U16 v = 0; v < vertcount; v++)
	{
		tempvect = LocalTheme->Verts[poly->Vertices[v]];
		if(tempvect.y > SMALL_CUBE_SIZE)
		{
			tempvect.y +=SMALL_CUBE_SIZE;
		}
		D3DMath_VectorMatrixMultiply( vect[(vertcount - 1)-v], tempvect, (D3DMATRIX&)*matrix);
	}

	FillPlane(&collpoly->Plane, &vect[0], &vect[1], &vect[2]);
	
	//now take a copy of the normal from the main plane
	normal.x = collpoly->Plane.v[0];
	normal.y = collpoly->Plane.v[1];
	normal.z = collpoly->Plane.v[2];

	if(NormalIsHorizontal(normal))
	{
		for(v = 0; v < vertcount; v++)
		{
			vect[v].y = min(vect[v].y, SMALL_CUBE_SIZE);
		}
	}

	//now use it to form an edge of each of the edgeplanes
	tempvect.x = vect[0].x + normal.x;
	tempvect.y = vect[0].y + normal.y;
	tempvect.z = vect[0].z + normal.z;
	FillPlane(&collpoly->EdgePlane[0], &tempvect, &vect[0], &vect[1]);
	tempvect.x = vect[1].x + normal.x;
	tempvect.y = vect[1].y + normal.y;
	tempvect.z = vect[1].z + normal.z;
	FillPlane(&collpoly->EdgePlane[1], &tempvect, &vect[1], &vect[2]);
	tempvect.x = vect[2].x + normal.x;
	tempvect.y = vect[2].y + normal.y;
	tempvect.z = vect[2].z + normal.z;
	if(poly->IsTriangle == TRUE)
	{
		FillPlane(&collpoly->EdgePlane[2], &tempvect, &vect[2], &vect[0]);
	}
	else
	{
		FillPlane(&collpoly->EdgePlane[2], &tempvect, &vect[2], &vect[3]);
		tempvect.x = vect[3].x + normal.x;
		tempvect.y = vect[3].y + normal.y;
		tempvect.z = vect[3].z + normal.z;
		FillPlane(&collpoly->EdgePlane[3], &tempvect, &vect[3], &vect[0]);
	}

	CUBE_HEADER_LOAD cuboid;
	NullifyCuboid(&cuboid);
	AddVectorToCuboid(&vect[0], &cuboid);
	AddVectorToCuboid(&vect[1], &cuboid);
	AddVectorToCuboid(&vect[2], &cuboid);
	if(poly->IsTriangle == FALSE)
	{
		AddVectorToCuboid(&vect[3], &cuboid);
	}
	collpoly->BBox.XMin = cuboid.Xmin;
	collpoly->BBox.YMin = cuboid.Ymin;
	collpoly->BBox.ZMin = cuboid.Zmin;
	collpoly->BBox.XMax = cuboid.Xmax;
	collpoly->BBox.YMax = cuboid.Ymax;
	collpoly->BBox.ZMax = cuboid.Zmax;
}

//**********************************************
//
//
//
//**********************************************
bool CuboidOverlapsBBox(const CUBE_HEADER_LOAD* cuboid, const BBOX* bbox)
{
	D3DVECTOR vector;
	
	vector.x = bbox->XMin;
	vector.y = bbox->YMin;
	vector.z = bbox->ZMin;
	if(CuboidContainsVector(cuboid, &vector))
	{
		return true;
	}
	
	vector.x = bbox->XMin;
	vector.y = bbox->YMin;
	vector.z = bbox->ZMax;
	if(CuboidContainsVector(cuboid, &vector))
	{
		return true;
	}
	
	vector.x = bbox->XMin;
	vector.y = bbox->YMax;
	vector.z = bbox->ZMin;
	if(CuboidContainsVector(cuboid, &vector))
	{
		return true;
	}
	
	vector.x = bbox->XMin;
	vector.y = bbox->YMax;
	vector.z = bbox->ZMax;
	if(CuboidContainsVector(cuboid, &vector))
	{
		return true;
	}
	
	vector.x = bbox->XMax;
	vector.y = bbox->YMin;
	vector.z = bbox->ZMin;
	if(CuboidContainsVector(cuboid, &vector))
	{
		return true;
	}
	
	vector.x = bbox->XMax;
	vector.y = bbox->YMin;
	vector.z = bbox->ZMax;
	if(CuboidContainsVector(cuboid, &vector))
	{
		return true;
	}
	
	vector.x = bbox->XMax;
	vector.y = bbox->YMax;
	vector.z = bbox->ZMin;
	if(CuboidContainsVector(cuboid, &vector))
	{
		return true;
	}
	
	vector.x = bbox->XMax;
	vector.y = bbox->YMax;
	vector.z = bbox->ZMax;
	if(CuboidContainsVector(cuboid, &vector))
	{
		return true;
	}

	return false;
}

//**********************************************
//
//
//
//**********************************************
void CreateCollisionPolys(void)
{
	CollisionData.Header.XStart = -(SMALL_CUBE_SIZE / 2);
	CollisionData.Header.ZStart = -(SMALL_CUBE_SIZE / 2);
	CollisionData.Header.XNum = LocalTrack->Width;
	CollisionData.Header.ZNum = LocalTrack->Height;
	CollisionData.Header.GridSize = SMALL_CUBE_SIZE;

	CollisionData.NumPolys = CollisionData.NumUsedPolys = CountCollisionPolys();
	CollisionData.Polys = new NEWCOLLPOLY[CollisionData.NumPolys];
	CollisionData.PolyStatus = new S16[CollisionData.NumPolys];

	U32 cellcount = LocalTrack->Width * LocalTrack->Height;
	CollisionData.Cells = new CELLDATA[cellcount];
	
	while(cellcount--)
	{
		CollisionData.Cells[cellcount].NumEntries = 0;
		CollisionData.Cells[cellcount].CollPolyIndices = NULL;
	}

	S16			 collpolyindex = 0;
	U32			 cellindex = 0;
	NEWCOLLPOLY* collpoly  = CollisionData.Polys;
	CELLDATA*	 cell	   = CollisionData.Cells;
	D3DMATRIX	 objectmatrix;

	cellcount = LocalTrack->Width * LocalTrack->Height;
	while(cellindex < cellcount)
	{
		MakeTranslationMatrix(&LocalTrack->Units[cellindex], &objectmatrix);

		TRACKUNIT*	unit = LocalTheme->Units[LocalTrack->Units[cellindex].UnitID];
		MESH*		mesh = &LocalTheme->Meshes[unit->MeshID];
		U16			ps	 = mesh->PolySetCount;

		ps	 = mesh->PolySetCount;
		while(ps-- > HULL_INDEX)
		{
			POLYSET* polyset = mesh->PolySets[ps];
			U16 p = polyset->PolygonCount;
			while(p--)
			{
				BASICPOLY* poly = &LocalTheme->Polys[polyset->Indices[p]];
				collpoly->Material = unit->Surfaces[p];
				FillCollisionPoly(collpoly, poly, &objectmatrix);
				collpoly++;
				collpolyindex++;
			}
		}
		cell++;
		cellindex++;
	}

	CUBE_HEADER_LOAD cuboid;
	const REAL bleedoffset = (SMALL_CUBE_SIZE / 2) + COLLISION_BLEED;

	cellindex = 0;
	cell	  = CollisionData.Cells;
	for(U16 z = 0; z < LocalTrack->Height; z++)
	{
		for(U16 x = 0; x < LocalTrack->Width; x++)
		{
			cuboid.Xmin = (x * SMALL_CUBE_SIZE) - bleedoffset;
			cuboid.Xmax = cuboid.Xmin + bleedoffset + bleedoffset;
			cuboid.Zmin = (z * SMALL_CUBE_SIZE) - bleedoffset;
			cuboid.Zmax = cuboid.Zmin + bleedoffset + bleedoffset;
			cuboid.Ymin = -(SMALL_CUBE_SIZE * 10);
			cuboid.Ymax = -cuboid.Ymin;

			cell->NumEntries = 0;

			collpoly = CollisionData.Polys;					//start at the first collision poly
			collpolyindex = 0;								//initialize loop counter
			while(collpolyindex < CollisionData.NumPolys)	//while there are polys left to check
			{
				if(CuboidOverlapsBBox(&cuboid, &collpoly->BBox))	//if the polys bounding box overlaps the cells box
				{
					cell->NumEntries++;						//increment the count of overlapping polys
				}
				collpoly++;			//point to next poly
				collpolyindex++;	//increment the loop count
			}

			cell->CollPolyIndices = new long[cell->NumEntries];

			collpoly = CollisionData.Polys;					//start at the first collision poly
			collpolyindex = 0;								//initialize loop counter

			long* indices = cell->CollPolyIndices;
			while(collpolyindex < CollisionData.NumPolys)	//while there are polys left to check
			{
				if(CuboidOverlapsBBox(&cuboid, &collpoly->BBox))	//if the polys bounding box overlaps the cells box
				{
					*indices = collpolyindex;				//put the index into the table
					indices++;								//and skip to the next point in the table
				}
				collpoly++;			//point to the next poly
				collpolyindex++;	//increment the loop count
			}

			cell++;
		}
	}
}

//**********************************************
//
//
//
//**********************************************
void DestroyCollisionPolys(void)
{
	U32 cellcount = LocalTrack->Width * LocalTrack->Height;
	if(CollisionData.Cells != NULL)
	{
		while(cellcount--)
		{
			delete[] CollisionData.Cells[cellcount].CollPolyIndices;
			CollisionData.Cells[cellcount].CollPolyIndices = NULL;
		}
		delete[] CollisionData.Cells;
		CollisionData.Cells = NULL;
	}
	delete[] CollisionData.Polys;
	CollisionData.Polys = NULL;
	delete[] CollisionData.PolyStatus;
	CollisionData.PolyStatus = NULL;

}

//**********************************************
//
//
//
//**********************************************
void WriteSmallCubes(EndianOutputStream& os)
{
	os.PutU32(SmallCubeCount);
	SMALLCUBE* cube = SmallCubes;
	for(U32 n = 0; n < SmallCubeCount; n++)
	{
		os.PutFloat(cube->Header.CentreX);
		os.PutFloat(cube->Header.CentreY);
		os.PutFloat(cube->Header.CentreZ);
		os.PutFloat(cube->Header.Radius);
		os.PutFloat(cube->Header.Xmin);
		os.PutFloat(cube->Header.Xmax);
		os.PutFloat(cube->Header.Ymin);
		os.PutFloat(cube->Header.Ymax);
		os.PutFloat(cube->Header.Zmin);
		os.PutFloat(cube->Header.Zmax);
		os.PutS16(cube->Header.PolyNum);
		os.PutS16(cube->Header.VertNum);

		WORLD_POLY_LOAD* poly = cube->Polys;
		for(S16 p = 0; p < cube->Header.PolyNum; p++)
		{
			os.PutS16(poly->Type);
			os.PutS16(poly->Tpage);
			os.PutS16(poly->vi0);
			os.PutS16(poly->vi1);
			os.PutS16(poly->vi2);
			os.PutS16(poly->vi3);
			os.PutS32(poly->c0);
			os.PutS32(poly->c1);
			os.PutS32(poly->c2);
			os.PutS32(poly->c3);
			os.PutFloat(poly->u0);
			os.PutFloat(poly->v0);
			os.PutFloat(poly->u1);
			os.PutFloat(poly->v1);
			os.PutFloat(poly->u2);
			os.PutFloat(poly->v2);
			os.PutFloat(poly->u3);
			os.PutFloat(poly->v3);
			poly++;
		}

		WORLD_VERTEX_LOAD* vert = cube->Verts;
		for(S16 v = 0; v < cube->Header.VertNum; v++)
		{
			os.PutFloat(vert->x);
			os.PutFloat(vert->y);
			os.PutFloat(vert->z);
			os.PutFloat(vert->nx);
			os.PutFloat(vert->ny);
			os.PutFloat(vert->nz);
			vert++;
		}
		cube++;
	}
}

//**********************************************
//
//
//
//**********************************************
void WriteBigCubes(EndianOutputStream& os)
{
	os.PutU32(BigCubeCount);
	BIGCUBE* cube = BigCubes;
	for(U32 n = 0; n < BigCubeCount; n++)
	{
		os.PutFloat(cube->Header.x);
		os.PutFloat(cube->Header.y);
		os.PutFloat(cube->Header.z);
		os.PutFloat(cube->Header.Radius);
		os.PutU32(cube->Header.CubeNum);
		os.write((char*)cube->CubeIndices, sizeof(U32) * cube->Header.CubeNum);
		cube++;
	}
}

//**********************************************
//
//
//
//**********************************************
void WriteWorldFile(const char *filename)
{
	EndianOutputStream os(filename);
	if(os.is_open())
	{
		WriteSmallCubes(os);
		WriteBigCubes(os);
	}
}

//**********************************************
//
//
//
//**********************************************
void WriteInfFile(const char *filename)
{
	char linebuffer[256];
	FILE* fo;
	fo = fopen(filename, "wt");
	if(fo != NULL)
	{
		fputs(";----------------------------\n", fo);
		fputs("; Revolt .INF file structure\n", fo);
		fputs(";----------------------------\n", fo);
		fputs("\n", fo);
		fputs("NAME            \'User-", fo);
		fputs(LocalTrack->Name, fo);
		fputs("\'\n", fo);
		sprintf(linebuffer, "STARTPOS        %f %f %f\n", StartPos.x, StartPos.y, StartPos.z);
		fputs(linebuffer, fo);
		fputs("STARTPOSREV     0 0 0\n", fo);
		sprintf(linebuffer, "STARTROT        %f\n", StartDir);
		fputs(linebuffer, fo);
		fputs("STARTROTREV     0.0\n", fo);
		fputs("STARTGRID       0\n", fo);
		fputs("STARTGRIDREV    0\n", fo);
		fputs("FARCLIP         20000\n", fo);
		fputs("FOGSTART        15000\n", fo);
		fputs("FOGCOLOR        30 40 50\n", fo);
		fputs("VERTFOGSTART    0\n", fo);
		fputs("VERTFOGEND      0\n", fo);
		fputs("WORLDRGBPER     100\n", fo);
		fputs("MODELRGBPER     100\n", fo);
		fputs("INSTANCERGBPER  100\n", fo);
		fclose(fo);
	}
}

//**********************************************
//
//
//
//**********************************************
void CopyBitmaps(const char* levelroot)
{
	char srcfile[MAX_PATH];
	char destfile[MAX_PATH];
	
	SHFILEOPSTRUCT fos;

	fos.hwnd	= NULL;
	fos.wFunc	= FO_COPY;
	fos.fFlags	= FOF_NOCONFIRMMKDIR |
				  FOF_SILENT |
				  FOF_NOCONFIRMATION;
	fos.pFrom	= srcfile;
	fos.pTo		= destfile;

	int n = 0;
	while(n < 16)	//should be enough for all of the bitmaps in a theme
	{
		sprintf(srcfile, "Toy-World\\tpage_%02d.bmp", n);
		srcfile[strlen(srcfile) + 1] = '\0';	// add the extra trailing zero reqd by SHFileOperation
		sprintf(destfile, "%s%c.bmp", levelroot, 'a' + n);
		SHFileOperation(&fos);
		n++;
	}

}

//**********************************************
//
//
//
//**********************************************
void WriteNCPFile(const char* destfile)
{
	EndianOutputStream os(destfile);
	if(os.is_open())
	{
		os.PutS16(CollisionData.NumUsedPolys);
		NEWCOLLPOLY* poly = CollisionData.Polys;
		S16 newindex = 0;
		for(S16 n = 0; n < CollisionData.NumPolys; n++)
		{
			if(CollisionData.PolyStatus[n] == 1)
			{
				os.PutS32(poly->Type);
				os.PutS32(poly->Material);
				os.PutFloat(poly->Plane.v[0]);
				os.PutFloat(poly->Plane.v[1]);
				os.PutFloat(poly->Plane.v[2]);
				os.PutFloat(poly->Plane.v[3]);
				for(U16 v = 0; v < 4; v++)
				{
					os.PutFloat(poly->EdgePlane[v].v[0]);
					os.PutFloat(poly->EdgePlane[v].v[1]);
					os.PutFloat(poly->EdgePlane[v].v[2]);
					os.PutFloat(poly->EdgePlane[v].v[3]);
				}
				os.PutFloat(poly->BBox.XMin);
				os.PutFloat(poly->BBox.XMax);
				os.PutFloat(poly->BBox.YMin);
				os.PutFloat(poly->BBox.YMax);
				os.PutFloat(poly->BBox.ZMin);
				os.PutFloat(poly->BBox.ZMax);
				CollisionData.PolyStatus[n] = newindex;
				newindex++;
			}
			poly++;
		}
#if 1	//make this a zero to make the gane display ALL collision polys
		//this mode is useful ONLY for debugging collision polygon generation
		os.PutFloat(CollisionData.Header.XStart);
		os.PutFloat(CollisionData.Header.ZStart);
		os.PutFloat(CollisionData.Header.XNum);
		os.PutFloat(CollisionData.Header.ZNum);
		os.PutFloat(CollisionData.Header.GridSize);
		U32 cellcount = LocalTrack->Width * LocalTrack->Height;
		CELLDATA* cell = CollisionData.Cells;
		while(cellcount--)
		{
			S32 polycount = cell->NumEntries;
			S32* polyindex = cell->CollPolyIndices;
			
			os.PutS32(polycount);
			while(polycount--)
			{
				os.PutS32(CollisionData.PolyStatus[*polyindex++]);
			}
			cell++;
		}
#endif
	}
}

//**********************************************
//
//
//
//**********************************************
void WriteTAZFile(const char* destfile)
{
	EndianOutputStream os(destfile);
	if(os.is_open())
	{
		os.PutU32(ZonesVisited);
		for(U16 z = 0; z < ZonesVisited; z++)
		{
			TRACKZONE* zone = &ZoneBuffer[ZoneSequence[z].ZoneID];
			os.PutU32(z);
			os.PutFloat(zone->Centre.X);
			os.PutFloat(zone->Centre.Y);
			os.PutFloat(zone->Centre.Z);
			os.PutFloat(ONE);
			os.PutFloat(ZERO);
			os.PutFloat(ZERO);
			os.PutFloat(ZERO);
			os.PutFloat(ONE);
			os.PutFloat(ZERO);
			os.PutFloat(ZERO);
			os.PutFloat(ZERO);
			os.PutFloat(ONE);
			os.PutFloat(zone->XSize);
			os.PutFloat(zone->YSize);
			os.PutFloat(zone->ZSize);
		}
	}
}

//**********************************************
//
//
//
//**********************************************
void CleanUp(void)
{
	DestroyZoneInformation();
	DestroyCollisionPolys();
	DestroyBigCubes();
	DestroySmallCubes();
	DestroyUnitCuboids();
	if(LocalTrack != NULL)
	{
		delete LocalTrack;
		LocalTrack = NULL;
	}
	LocalTheme = NULL;
	LocalCursor = NULL;
}

//**********************************************
//
//
//
//**********************************************
U16 CountStartGrids(void)
{
	assert(LocalTrack != NULL);
	assert(LocalTheme != NULL);

	StartPos.x	= ZERO;
	StartPos.y	= ZERO;
	StartPos.z	= ZERO;
	StartDir	= ZERO;
	U16 count = 0;
	U32 m = LocalTrack->Width * LocalTrack->Height;
	while(m--)
	{
		if(LocalTrack->Units[m].UnitID == MODULE_STARTGRID) 
		{
			count++;
			StartPos.x	= LocalTrack->Units[m].XPos * SMALL_CUBE_SIZE;
			StartPos.y	= LocalTrack->Units[m].Elevation * -ElevationStep;
			StartPos.z	= LocalTrack->Units[m].YPos * SMALL_CUBE_SIZE;
			StartDir	= LocalTrack->Units[m].Direction * Real(0.25f);
		}
	}
	return count;
}

//**********************************************
//
//
//
//**********************************************
bool ValidateTrack(void)
{
	bool trackisvalid = false;
	U16 gridcount = CountStartGrids();
	if(gridcount == 1)
	{
		trackisvalid = CreateZoneInformation();
		if(trackisvalid == false)
		{
			FlagError("Track doesn't form a loop\n\ror it has an upwards step\n\rin it.\n\rThe Cursor will be placed\n\rat the last valid position");
			TRACKZONE* lastzone = &ZoneBuffer[ZoneSequence[ZonesVisited - 1].ZoneID];
			LocalCursor->X = (lastzone->Centre.X / SMALL_CUBE_SIZE);
			LocalCursor->Y = (lastzone->Centre.Z / SMALL_CUBE_SIZE);
		}
	}
	else
	{
		if(gridcount == 0)
		{
			FlagError("Track doesn't have\n\ra Start Grid");
		}
		else
		{
			FlagError("Track has more than\n\rone Start Grid");
		}
	}
	if(trackisvalid == false)
	{
		SetScreenState(ESS_PLACING_MODULE);
	}
	return trackisvalid;
}

//**********************************************
//
//
//
//**********************************************
void CompileTrack(const TRACKDESC* track, const TRACKTHEME* theme, CURSORDESC* cursor)
{
	static char levelname[MAX_PATH];

	static COMPILE_STATES nextstate[] = {
									TCS_UNIT_CUBES,
									TCS_SMALL_CUBES,
									TCS_BIG_CUBES,
									TCS_MAKING_COLLISONS,
									TCS_OPTIMIZING,
									TCS_WRITING_WORLD,
									TCS_WRITING_INF,
									TCS_WRITING_BITMAPS,
									TCS_WRITING_NCP,
									TCS_WRITING_TAZ,
									TCS_CLEANUP,
									TCS_RINGING_CLOCK,
									TCS_VALIDATING
								};

	REAL unitradius = SMALL_CUBE_SIZE / Real(2.0f);
	
	UnitRootVects[0] =  D3DVECTOR(-unitradius, unitradius, unitradius);
	UnitRootVects[1] =  D3DVECTOR(unitradius, unitradius, unitradius);
	UnitRootVects[2] =  D3DVECTOR(unitradius, unitradius, unitradius);
	UnitRootVects[3] =  D3DVECTOR(unitradius, unitradius, -unitradius);
	UnitRootVects[4] =  D3DVECTOR(unitradius, unitradius, -unitradius);
	UnitRootVects[5] =  D3DVECTOR(-unitradius, unitradius, -unitradius);
	UnitRootVects[6] =  D3DVECTOR(-unitradius, unitradius, -unitradius);
	UnitRootVects[7] =  D3DVECTOR(-unitradius, unitradius, unitradius);
	UnitRootVects[8] =  D3DVECTOR(-unitradius, 0, unitradius);
	UnitRootVects[9] =  D3DVECTOR(unitradius, 0, unitradius);
	UnitRootVects[10] = D3DVECTOR(unitradius, 0, unitradius);
	UnitRootVects[11] = D3DVECTOR(unitradius, 0, -unitradius);
	UnitRootVects[12] = D3DVECTOR(unitradius, 0, -unitradius);
	UnitRootVects[13] = D3DVECTOR(-unitradius, 0, -unitradius);
	UnitRootVects[14] = D3DVECTOR(-unitradius, 0, -unitradius);
	UnitRootVects[15] = D3DVECTOR(-unitradius, 0, unitradius);

	if(CompileState == TCS_VALIDATING)
	{
		LocalTrack = new TRACKDESC;
		LocalTheme = theme;
		LocalCursor = cursor;

		char name[MAX_DESCRIPTION_LENGTH];
		NextFreeDescription(name);
		CreateTrack(LocalTrack, DEFAULT_TRACK_WIDTH, DEFAULT_TRACK_HEIGHT, name);
		CloneTrack(LocalTrack, track);
		MakeTrackFromModules(LocalTrack);
		strcpy(levelname, "");	//zero out the levelname
		if(DescribedLevelExists(LocalTrack->Name))
		{
			GetDescribedLevelName(LocalTrack->Name, levelname);
		}
		else
		{
			NextFreeLevel(levelname);
		}

		if(ValidateTrack() == false)
		{
			CleanUp();
			return;
		}
		MakeModulesDirectional(LocalTrack);
		MakeTrackFromModules(LocalTrack);
	}

	if(CompileState == TCS_UNIT_CUBES)
	{
		CreateUnitCuboids();
	}

	if(CompileState == TCS_SMALL_CUBES)
	{
		CreateSmallCubes();	
	}

	if(CompileState == TCS_BIG_CUBES)
	{
		CreateBigCubes();
	}

	if(CompileState == TCS_MAKING_COLLISONS)
	{
		CreateCollisionPolys();
	}

	if(CompileState == TCS_OPTIMIZING)
	{
		RemoveSurplusPolys();
	}

	if(strlen(levelname) > 0)
	{
		char levelpath[MAX_PATH];
		sprintf(levelpath, "..\\game\\levels\\%s", levelname);
		CreateDirectory(levelpath, NULL);

		char levelroot[MAX_PATH];
		sprintf(levelroot, "..\\game\\levels\\%s\\%s", levelname, levelname);
		
		char filepath[MAX_PATH];

		if(CompileState == TCS_WRITING_WORLD)
		{
			sprintf(filepath, "%s.w", levelroot);
			WriteWorldFile(filepath);
		}

		if(CompileState == TCS_WRITING_INF)
		{
			sprintf(filepath, "%s.inf", levelroot);
			WriteInfFile(filepath);
		}

		if(CompileState == TCS_WRITING_BITMAPS)
		{
			CopyBitmaps(levelroot);
		}

		if(CompileState == TCS_WRITING_NCP)
		{
			sprintf(filepath, "%s.ncp", levelroot);
			WriteNCPFile(filepath);
		}

		if(CompileState == TCS_WRITING_TAZ)
		{
			sprintf(filepath, "%s.taz", levelroot);
			WriteTAZFile(filepath);
		}
	}
	else
	{
		CompileState = TCS_CLEANUP;
	}
		
	if(CompileState == TCS_CLEANUP)
	{
		CleanUp();
	}

	if(CompileState == TCS_RINGING_CLOCK)
	{
		ClockFrame-=2;
		if(ClockFrame == 0)
		{
			SetScreenState(ESS_PLACING_MODULE);
			CompileState = nextstate[CompileState];
		}
	}
	else
	{
		CompileState = nextstate[CompileState];
	}
}

//**********************************************
//
//
//
//**********************************************

