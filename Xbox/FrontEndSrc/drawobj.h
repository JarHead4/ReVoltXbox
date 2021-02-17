//-----------------------------------------------------------------------------
// File: DrawObj.h
//
// Desc: Header file for drawing objects like the car, wheels, and aerial.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef DRAWOBJ_H
#define DRAWOBJ_H

#include "ctrlread.h"
#include "object.h"
#include "player.h"
#include "draw.h"

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
extern void DrawCar(PLAYER *player);
extern void DrawCarGhost(PLAYER *player);
extern void DrawAllCarShadows(void);
extern void DrawSkidMarks();
extern void BuildAerialSectionMatrix(AERIALSECTION *section);
extern void DrawCarAerial2(AERIAL *aerial, MODEL *secModel, MODEL *topModel, short flag);
//extern void DrawCarAerial2(CAR *car, short flag);
extern void DrawCarBoundingBoxes(CAR *car);
extern void DrawObjects(void);
extern void RenderObject(OBJECT *obj);
extern bool RenderObjectModel(MAT *mat, VEC *pos, MODEL *model, long envrgb, REAL envoffx, REAL envoffy, REAL envscale, struct renderflags renderflag, long scaled);
extern void RenderPlanet(OBJECT *obj);
extern void RenderSun(OBJECT *obj);
extern void RenderPlane(OBJECT *obj);
extern void RenderCopter(OBJECT *obj);
extern void RenderDragon(OBJECT *obj);
extern void RenderTrolley(OBJECT *obj);
extern void DrawGridCollPolys(COLLGRID *grid);
extern void RenderTrain(OBJECT *obj);
extern void RenderStrobe(OBJECT *obj);
extern void DrawAllPickups();
extern void RenderStar(OBJECT *obj);
extern void RenderFox(OBJECT *obj);
extern void RenderDissolveModel(OBJECT *obj);
extern void RenderLaser(OBJECT *obj);
extern void RenderSplash(OBJECT *obj);
extern void RenderSpeedup(OBJECT *obj);
extern void RenderClouds(OBJECT *obj);
extern void RenderSprinkler(OBJECT *obj);
extern void RenderClock(OBJECT *obj);
extern void DrawCup(OBJECT *obj);
extern void DrawDemoLogo(REAL alphaMod, int posIndex);
extern void DrawDemoMessage();
extern void DrawReplayMessage();
extern void RenderLantern(OBJECT *obj);
extern void RenderSkybox(void);
extern void RenderSlider(OBJECT *obj);
extern void RenderRain(OBJECT *obj);
extern void RenderLightning(OBJECT *obj);
extern void RenderShipLight(OBJECT *obj);
extern void RenderWaterBox(OBJECT *obj);
extern void RenderUnderWaterPoly(void);
extern void RenderRipple(OBJECT *obj);
extern void RenderStream(OBJECT *obj);
extern void RenderDolphin(OBJECT *obj);

extern void MotionBlurInit(void);
extern void MotionBlurAdd(VEC* pP0, VEC* pP1, VEC* pP2, VEC* pP3, long rgb1, long rgb2);
extern void MotionBlurRender(void);
extern void RenderFlag(OBJECT *obj);

// globals

extern FACING_POLY SunFacingPoly, DragonFireFacingPoly;

#endif // DRAWOBJ_H

