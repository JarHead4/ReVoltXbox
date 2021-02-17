
#ifndef COLL_H
#define COLL_H

// macros

#define PlaneDist(plane, point) \
	(((plane)->v[A] * (point)->v[X] + (plane)->v[B] * (point)->v[Y] + (plane)->v[C] * (point)->v[Z] + (plane)->v[D]))

typedef struct {
	short CollNum;
} COLL_HEADER;

typedef struct {
	long Flag;
	PLANE Plane;
	float MinX, MaxX, MinY, MaxY, MinZ, MaxZ;
	VECTOR v0;
	VECTOR v1;
	VECTOR v2;
	VECTOR v3;
} COLL_POLY;

// prototypes

#ifdef _N64
extern void COL_LoadCollision(FIL_ID Fil);
extern void COL_FreeCollision(void);
#else
extern bool LoadCollision(char *file);
extern void FreeCollision(void);
#endif
extern char PointInsidePlane(VECTOR *point, COLL_POLY *p);
extern float PointToPolyEdge(VECTOR *point, COLL_POLY *p, VECTOR *out);
extern float FindNearestPointOnLine(VECTOR *lp1, VECTOR *lp2, VECTOR *point, VECTOR *out);
extern void FindIntersection(VECTOR *point1, float dist1, VECTOR *point2, float dist2, VECTOR *out);
extern short SpheresToPlane(VECTOR *o, VECTOR *n, VECTOR *out, float rad, COLL_POLY *p);
extern short SpheresToPlaneNorm(VECTOR *o, VECTOR *n, VECTOR *out, float rad, COLL_POLY *p);
extern char SphereCollTest(VECTOR *from, VECTOR *to, float radius, VECTOR *result, float *friction, VECTOR *norm);
extern char SphereCollTestNorm(VECTOR *from, VECTOR *to, float radius, VECTOR *result, float *friction, VECTOR *norm);
//extern void BuildPlane(VECTOR *a, VECTOR *b, VECTOR *c, PLANE *p);

// globals

extern COLL_POLY *CollPtr;
#ifdef _N64
extern long CollNum;
#else
extern short CollNum;
#endif
#endif
