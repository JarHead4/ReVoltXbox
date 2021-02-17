/////////////////////////////////////////////////////////////////////
//
// Header file for drawing objects like the car, wheels, and aerial
//
/////////////////////////////////////////////////////////////////////

#ifndef __DRAWOBJECTS_H__
#define __DRAWOBJECTS_H__

#include "ctrlread.h"
#include "object.h"

// macros

enum {
	SPHERE_OUT,
	SPHERE_CLIP,
	SPHERE_IN,
};

typedef struct {
	REAL Left, Right, Front, Back, Height;
	REAL tu, tv, twidth, theight;
} CAR_SHADOW_TABLE;

// prototypes

extern long TestSphereToFrustum(VEC *pos, float rad, float *z);
extern void BuildAllCarWorldMatrices(void);
extern void BuildCarMatricesNew(CAR *car);
extern void DrawAllCars(void);
extern void DrawAllGhostCars(void);
extern void DrawCar(CAR *car);
extern void DrawCarGhost(CAR *car);
extern void DrawAllCarShadows(void);
extern void DrawSkidMarks();
extern void BuildAerialSectionMatrix(AERIALSECTION *section);
extern void DrawCarAerial2(AERIAL *aerial, MODEL *secModel, MODEL *topModel, short flag);
//extern void DrawCarAerial2(CAR *car, short flag);
extern void DrawCarBoundingBoxes(CAR *car);
extern void DrawObjects(void);
extern void RenderObject(OBJECT *obj);
extern bool RenderObjectModel(MAT *mat, VEC *pos, MODEL *model, long envrgb, struct renderflags renderflag);
extern void RenderPlanet(OBJECT *obj);
extern void RenderSun(OBJECT *obj);
extern void RenderPlane(OBJECT *obj);
extern void RenderCopter(OBJECT *obj);
extern void RenderDragon(OBJECT *obj);
extern void RenderTrolley(OBJECT *obj);
extern void DrawGridCollPolys(COLLGRID *grid);
extern void RenderTrain(OBJECT *obj);
extern void RenderStrobe(OBJECT *obj);
extern void RenderPickup(OBJECT *obj);
extern void RenderDissolveModel(OBJECT *obj);
extern void RenderLaser(OBJECT *obj);
extern void RenderSplash(OBJECT *obj);
extern void RenderSpeedup(OBJECT *obj);

// globals

extern FACING_POLY SunFacingPoly, DragonFireFacingPoly;

#endif
