
#ifndef MIRROR_H
#define MIRROR_H

#include "level.h"

// macros

#define MIRROR_PLANE_OVERLAP 200
#define MIRROR_OVERLAP_TOL 2

#define GET_MIRROR_FOG(_n) \
	((_n) * MirrorMul + MirrorAdd);

typedef struct {
	short MirrorPlaneNum;
} MIRROR_PLANE_HEADER;

typedef struct {
	long Flag;
	PLANE Plane;
	float MinX, MaxX, MinY, MaxY, MinZ, MaxZ;
	VEC v0;
	VEC v1;
	VEC v2;
	VEC v3;
} MIRROR_PLANE_LOAD;

typedef struct {
	float Xmin, Xmax;
	float Zmin, Zmax;
	float Height;
} MIRROR_PLANE;

// prototypes

extern bool LoadMirrorPlanes(char *file);
extern void FreeMirrorPlanes(void);
extern void SetMirrorParams(LEVELINFO *lev);
extern long GetMirrorPlane(VEC *pos);

// globals

extern long MirrorPlaneNum;
extern MIRROR_PLANE *MirrorPlanes;
extern long MirrorType, MirrorAlpha;
extern float MirrorHeight, MirrorMul, MirrorAdd, MirrorDist;

#endif
