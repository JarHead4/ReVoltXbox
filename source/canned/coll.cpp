
#include "revolt.h"
#include "coll.h"
#include "geom.h"
#ifdef _PC
#include "text.h"
#include "main.h"
#endif
#include "NewColl.h"

// globals
#ifdef _PC
short CollNum;
#else
long CollNum;
#endif

COLL_POLY *CollPtr;

#ifdef _N64

//
// COL_LoadCollision
//
// Loads the collision skin data into RAM
//

void COL_LoadCollision(FIL_ID Fil)
{
	COLL_HEADER	 ch;
	FIL			*skin;

	printf("Loading collision data...\n");
	skin = FFS_Open(Fil);
	if (skin == NULL)
	{
		ERROR("COL", "COL_LoadCollision", "Failed to open collision skin file", 1);
	}

	FFS_Read((char *)&CollNum, sizeof(long), skin);					// Read in number of skin polys

	printf("...collision skin has %d polys.\n", CollNum);
	
	CollPtr = malloc(CollNum * sizeof(COLL_POLY));
	if (CollPtr == NULL)
	{
		ERROR("COL", "COL_LoadCollision", "Could not allocate memory for collision skin", 1);
	}

	FFS_Read((char *)CollPtr, sizeof(COLL_POLY) * CollNum, skin);	// Read in skin poly defs

	FFS_Close(skin);
}


//
// COL_FreeCollision
//
// Frees the memory used by the collision skin data
//

void COL_FreeCollision(void)
{
	free(CollPtr);
}

#else

///////////////////////////
// load a collision skin //
///////////////////////////

bool LoadCollision(char *file)
{
	COLL_HEADER ch;
	FILE *fp;

// open file for reading

	fp = fopen(file, "rb");
	if (fp == NULL)
	{
		char buff[128];
		wsprintf(buff, "Can't load collision file: '%s'", file);
		Box("ERROR", buff, MB_OK);
		return FALSE;
	 }

// get header info, alloc coll memory

	fread(&ch, sizeof(ch), 1, fp);
	CollNum = ch.CollNum;
	CollPtr = (COLL_POLY*)malloc(CollNum * sizeof(COLL_POLY));
	if (CollPtr == NULL) return FALSE;

// read in coll poly's

	fread(CollPtr, sizeof(COLL_POLY), CollNum, fp);

// close file

	fclose(fp);

// return OK

	return TRUE;
}

/////////////////////////
// free collision skin //
/////////////////////////

void FreeCollision(void)
{
	free(CollPtr);
	CollPtr = NULL;
	CollNum = 0;
}

#endif  //_N64

//////////////////////////////
// check point inside plane //
//////////////////////////////

char PointInsidePlane(VECTOR *point, COLL_POLY *p)
{
	VECTOR vec1, vec2;

// point inside poly?

	if (p->Flag & 1)
	{
		SubVector(&p->v1, &p->v0, &vec1);
		SubVector(point, &p->v0, &vec2);
		if (CrossProduct3(&vec1, &vec2, (VECTOR*)&p->Plane) > 0) return FALSE;

		SubVector(&p->v2, &p->v1, &vec1);
		SubVector(point, &p->v1, &vec2);
		if (CrossProduct3(&vec1, &vec2, (VECTOR*)&p->Plane) > 0) return FALSE;

		SubVector(&p->v3, &p->v2, &vec1);
		SubVector(point, &p->v2, &vec2);
		if (CrossProduct3(&vec1, &vec2, (VECTOR*)&p->Plane) > 0) return FALSE;

		SubVector(&p->v0, &p->v3, &vec1);
		SubVector(point, &p->v3, &vec2);
		if (CrossProduct3(&vec1, &vec2, (VECTOR*)&p->Plane) > 0) return FALSE;
	}
	else
	{
		SubVector(&p->v1, &p->v0, &vec1);
		SubVector(point, &p->v0, &vec2);
		if (CrossProduct3(&vec1, &vec2, (VECTOR*)&p->Plane) > 0) return FALSE;

		SubVector(&p->v2, &p->v1, &vec1);
		SubVector(point, &p->v1, &vec2);
		if (CrossProduct3(&vec1, &vec2, (VECTOR*)&p->Plane) > 0) return FALSE;

		SubVector(&p->v0, &p->v2, &vec1);
		SubVector(point, &p->v2, &vec2);
		if (CrossProduct3(&vec1, &vec2, (VECTOR*)&p->Plane) > 0) return FALSE;
	}

// yes

	return TRUE;
}

////////////////////////////////////////////////
// find nearest point on poly edge to a point //
////////////////////////////////////////////////

float PointToPolyEdge(VECTOR *point, COLL_POLY *p, VECTOR *out)
{
	VECTOR linepoint;
	float linedist, dist;

// quad

	if (p->Flag & 1)
	{
		dist = FindNearestPointOnLine(&p->v0, &p->v1, point, out);

		linedist = FindNearestPointOnLine(&p->v1, &p->v2, point, &linepoint);
		if (linedist < dist)
		{
			dist = linedist;
			SetVector(out, linepoint.v[X], linepoint.v[Y], linepoint.v[Z]);
		}

		linedist = FindNearestPointOnLine(&p->v2, &p->v3, point, &linepoint);
		if (linedist < dist)
		{
			dist = linedist;
			SetVector(out, linepoint.v[X], linepoint.v[Y], linepoint.v[Z]);
		}

		linedist = FindNearestPointOnLine(&p->v3, &p->v0, point, &linepoint);
		if (linedist < dist)
		{
			dist = linedist;
			SetVector(out, linepoint.v[X], linepoint.v[Y], linepoint.v[Z]);
		}
	}

// tri

	else
	{
		dist = FindNearestPointOnLine(&p->v0, &p->v1, point, out);

		linedist = FindNearestPointOnLine(&p->v1, &p->v2, point, &linepoint);
		if (linedist < dist)
		{
			dist = linedist;
			SetVector(out, linepoint.v[X], linepoint.v[Y], linepoint.v[Z]);
		}

		linedist = FindNearestPointOnLine(&p->v2, &p->v0, point, &linepoint);
		if (linedist < dist)
		{
			dist = linedist;
			SetVector(out, linepoint.v[X], linepoint.v[Y], linepoint.v[Z]);
		}
	}

// return dist

	return dist;
}

/////////////////////////////////////////////
// find nearest point on a line to a point //
/////////////////////////////////////////////

float FindNearestPointOnLine(VECTOR *lp1, VECTOR *lp2, VECTOR *point, VECTOR *out)
{
	PLANE plane;
	VECTOR diff;
	float linedist, pointdist;

// get difference vector + line dist

	SubVector(lp2, lp1, &diff);
	linedist = Length(&diff);

// build plane, get point distance

	SetVector((VECTOR*)&plane, diff.v[X] / linedist, diff.v[Y] / linedist, diff.v[Z] / linedist);
	plane.v[D] = -DotProduct(lp1, (VECTOR*)&plane);
	pointdist = PlaneDist(&plane, point);

// clip point dist

	if (pointdist < 0) pointdist = 0;
	else if (pointdist > linedist) pointdist = linedist;

// get nearest point along line

	out->v[X] = lp1->v[X] + (diff.v[X] / linedist * pointdist);
	out->v[Y] = lp1->v[Y] + (diff.v[Y] / linedist * pointdist);
	out->v[Z] = lp1->v[Z] + (diff.v[Z] / linedist * pointdist);

// return dist

	SubVector(out, point, &diff);
	return Length(&diff);
}

////////////////////////////////////////////////////////////////
// find intersection of a plane from two points and distances //
////////////////////////////////////////////////////////////////

void FindIntersection(VECTOR *point1, float dist1, VECTOR *point2, float dist2, VECTOR *out)
{
	float mul;
	VECTOR diff;

// get diff vector, mul

	SubVector(point2, point1, &diff);
	mul = -dist1 / (dist2 - dist1);

// get intersection

	out->v[X] = point1->v[X] + diff.v[X] * mul;
	out->v[Y] = point1->v[Y] + diff.v[Y] * mul;
	out->v[Z] = point1->v[Z] + diff.v[Z] * mul;
}

//////////////////////////////////////////
// test old / new spheres against plane //
//////////////////////////////////////////

short SpheresToPlane(VECTOR *o, VECTOR *n, VECTOR *out, float rad, COLL_POLY *p)
{
	char flag = FALSE;
	float odist, ndist, mul;
	VECTOR inter, diff;

// get plane dists, check for skip

	ndist = -PlaneDist(&p->Plane, n);
	if (ndist <= -rad) return FALSE;

	odist = -PlaneDist(&p->Plane, o);
	if (odist >= 0) return FALSE;

	if (ndist <= odist) return FALSE;

// get old / new diff vector

	SubVector(n, o, &diff);

// hit if new inside poly

//	if (PointInsidePlane(n, p)) flag = TRUE;

// hit if old inside and closer than rad

//	if (!flag && odist >= -rad && PointInsidePlane(o, p)) flag = TRUE;

// hit if old / new ray intersects poly

	if (!flag)
	{
		if (!ndist) memcpy(&inter, n, sizeof(VECTOR));
		else
		{
			mul = -odist / (ndist - odist);
			inter.v[X] = o->v[X] + diff.v[X] * mul;
			inter.v[Y] = o->v[Y] + diff.v[Y] * mul;
			inter.v[Z] = o->v[Z] + diff.v[Z] * mul;
		}
		if (PointInsidePlane(&inter, p)) flag = TRUE;
	}

// skip if didn't hit

	if (!flag) return FALSE;

// ok, get out

	odist += rad;
	ndist += rad;

	if (!odist) memcpy(out, o, sizeof(VECTOR));
	else
	{
		mul = -odist / (ndist - odist);
		out->v[X] = o->v[X] + diff.v[X] * mul;
		out->v[Y] = o->v[Y] + diff.v[Y] * mul;
		out->v[Z] = o->v[Z] + diff.v[Z] * mul;
	}

	return TRUE;
}

//////////////////////////////////////////
// test old / new spheres against plane //
//////////////////////////////////////////

short SpheresToPlaneNorm(VECTOR *o, VECTOR *n, VECTOR *out, float rad, COLL_POLY *p)
{
	float odist, ndist;

// get old / new plane dists, check for skip

	ndist = -PlaneDist(&p->Plane, n);
	if (ndist <= -rad) return FALSE;

	odist = -PlaneDist(&p->Plane, o);
	if (odist >= 0) return FALSE;

	if (ndist <= odist) return FALSE;

// inside poly?

	if (PointInsidePlane(n, p))
	{

// yep, return new info

		ndist += rad;

		out->v[X] = n->v[X] + p->Plane.v[A] * ndist;
		out->v[Y] = n->v[Y] + p->Plane.v[B] * ndist;
		out->v[Z] = n->v[Z] + p->Plane.v[C] * ndist;

		return TRUE;
	}

// get plane intersection

//	FindIntersection(o, odist, n, ndist, &inter);

// near enough to nearest edge?

//	edgedist = PointToPolyEdge(&inter, p, &edge);
//	if (edgedist < rad)
//	{
//		ndist += rad;
//
//		out->v[X] = n->v[X] + p->Plane.v[A] * ndist;
//		out->v[Y] = n->v[Y] + p->Plane.v[B] * ndist;
//		out->v[Z] = n->v[Z] + p->Plane.v[C] * ndist;
//
//		return TRUE;
//	}

// no collisions

	return FALSE;
}

//////////////////////
// sphere coll test //
//////////////////////

char SphereCollTest(VECTOR *from, VECTOR *to, float radius, VECTOR *result, float *friction, VECTOR *norm)
{
	short i;
	char flag;
	COLL_POLY *p;
	VECTOR vec, old;
	float MinX, MaxX, MinY, MaxY, MinZ, MaxZ;

// zero friction

	*friction = 0;

// build bounding box

	MinX = from->v[X] - radius;
	MaxX = from->v[X] + radius;
	MinY = from->v[Y] - radius;
	MaxY = from->v[Y] + radius;
	MinZ = from->v[Z] - radius;
	MaxZ = from->v[Z] + radius;

	if (to->v[X] - radius < MinX) MinX = to->v[X] - radius;
	if (to->v[X] + radius > MaxX) MaxX = to->v[X] + radius;
	if (to->v[Y] - radius < MinY) MinY = to->v[Y] - radius;
	if (to->v[Y] + radius > MaxY) MaxY = to->v[Y] + radius;
	if (to->v[Z] - radius < MinZ) MinZ = to->v[Z] - radius;
	if (to->v[Z] + radius > MaxZ) MaxZ = to->v[Z] + radius;

// loop thru all coll polys

	flag = FALSE;
	p = CollPtr;
	SetVector(&old, from->v[X], from->v[Y], from->v[Z]);
	SetVector(result, to->v[X], to->v[Y], to->v[Z]);

	for (i = 0 ; i < CollNum ; i++, p++)
	{

// skip?

		if (MinX > p->MaxX || MaxX < p->MinX ||	MinY > p->MaxY || MaxY < p->MinY ||	MinZ > p->MaxZ || MaxZ < p->MinZ) continue;

// test

		if (SpheresToPlane(&old, result, &vec, radius, p))
		{
			SubVector(&vec, result, &vec);
			AddVector(&old, &vec, &old);
			AddVector(result, &vec, result);
			*friction = 1;
			memcpy(norm, &p->Plane, sizeof(VECTOR));
			flag = TRUE;
		}
	}

// return

	return flag;
}

//////////////////////
// sphere coll test //
//////////////////////

char SphereCollTestNorm(VECTOR *from, VECTOR *to, float radius, VECTOR *result, float *friction, VECTOR *norm)
{
	short i;
	char flag;
	COLL_POLY *p;
	VECTOR vec, old;
	float MinX, MaxX, MinY, MaxY, MinZ, MaxZ;

// zero friction

	*friction = 0;

// build bounding box

	MinX = from->v[X] - radius;
	MaxX = from->v[X] + radius;
	MinY = from->v[Y] - radius;
	MaxY = from->v[Y] + radius;
	MinZ = from->v[Z] - radius;
	MaxZ = from->v[Z] + radius;

	if (to->v[X] - radius < MinX) MinX = to->v[X] - radius;
	if (to->v[X] + radius > MaxX) MaxX = to->v[X] + radius;
	if (to->v[Y] - radius < MinY) MinY = to->v[Y] - radius;
	if (to->v[Y] + radius > MaxY) MaxY = to->v[Y] + radius;
	if (to->v[Z] - radius < MinZ) MinZ = to->v[Z] - radius;
	if (to->v[Z] + radius > MaxZ) MaxZ = to->v[Z] + radius;

// loop thru all coll polys

	flag = FALSE;
	p = CollPtr;
	SetVector(&old, from->v[X], from->v[Y], from->v[Z]);
	SetVector(result, to->v[X], to->v[Y], to->v[Z]);

	for (i = 0 ; i < CollNum ; i++, p++)
	{

// skip?

		if (MinX > p->MaxX || MaxX < p->MinX ||	MinY > p->MaxY || MaxY < p->MinY ||	MinZ > p->MaxZ || MaxZ < p->MinZ) continue;

// test

		if (SpheresToPlaneNorm(&old, result, &vec, radius, p))
		{
			SubVector(&vec, result, &vec);
			AddVector(&old, &vec, &old);
			AddVector(result, &vec, result);
			*friction = 1;
			memcpy(norm, &p->Plane, sizeof(VECTOR));
			flag = TRUE;
		}
	}

// return

	return flag;
}

