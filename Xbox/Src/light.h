//-----------------------------------------------------------------------------
// File: light.h
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef LIGHT_H
#define LIGHT_H

#include "model.h"
#include "world.h"
#include "instance.h"

// macros

#define LIGHT_MAX 250

#define LIGHT_FIXED 1
#define LIGHT_MOVING 2
#define LIGHT_FILE 4
#define LIGHT_OFF 8
#define LIGHT_FLICKER 16

enum {
    LIGHT_OMNI,
    LIGHT_OMNINORMAL,
    LIGHT_SPOT,
    LIGHT_SPOTNORMAL,
    LIGHT_SHADOW,
};

enum {
    LIGHT_AXIS_XY,
    LIGHT_AXIS_XZ,
    LIGHT_AXIS_ZY,
    LIGHT_AXIS_X,
    LIGHT_AXIS_Y,
    LIGHT_AXIS_Z,
};

typedef struct LightStruct {
    float x, y, z, Reach, SquareReach;
    float Xmin, Xmax, Ymin, Ymax, Zmin, Zmax;
    MAT DirMatrix;
    float Cone, ConeMul;
    long r, g, b;
    unsigned char Flag, Type, Speed, pad2;
} LIGHT;

typedef struct {
    float x, y, z, Reach;
    MAT DirMatrix;
    float Cone;
    float r, g, b;
    unsigned char Flag, Type, Speed, pad2;
} FILELIGHT;

// prototypes

extern void InitLights(void);
extern LIGHT *AllocLight(void);
extern void FreeLight(LIGHT *light);
extern void ProcessLights(void);
extern void ProcessOneLight(LIGHT *l);
extern void AddPermLight(LIGHT *light);
extern char CheckCubeLight(CUBE_HEADER *cube);
extern void AddCubeLightPermOmni(CUBE_HEADER *cube, LIGHT *light);
extern void AddCubeLightPermOmniNormal(CUBE_HEADER *cube, LIGHT *light);
extern void AddCubeLightPermSpot(CUBE_HEADER *cube, LIGHT *light);
extern void AddCubeLightPermSpotNormal(CUBE_HEADER *cube, LIGHT *light);
extern void SetCubeLightOmni(CUBE_HEADER *cube, LIGHT *light);
extern void AddCubeLightOmni(CUBE_HEADER *cube, LIGHT *light);
extern void SetCubeLightOmniNormal(CUBE_HEADER *cube, LIGHT *light);
extern void AddCubeLightOmniNormal(CUBE_HEADER *cube, LIGHT *light);
extern void SetCubeLightSpot(CUBE_HEADER *cube, LIGHT *light);
extern void AddCubeLightSpot(CUBE_HEADER *cube, LIGHT *light);
extern void SetCubeLightSpotNormal(CUBE_HEADER *cube, LIGHT *light);
extern void AddCubeLightSpotNormal(CUBE_HEADER *cube, LIGHT *light);
extern short CheckObjectLight(VEC *pos, BOUNDING_BOX *box, float rad);
extern short CheckInstanceLight(INSTANCE *inst, float rad);
extern short CheckInstanceLightEdit(INSTANCE *inst, float rad);
extern void AddModelLightPermOmni(MODEL *model, LIGHT *light, VEC *pos);
extern void AddModelLightPermOmniNormal(MODEL *model, LIGHT *light, VEC *pos);
extern void AddModelLightPermSpot(MODEL *model, LIGHT *light, VEC *pos, VEC *dir);
extern void AddModelLightPermSpotNormal(MODEL *model, LIGHT *light, VEC *pos, VEC *dir);
extern void AddModelLight(MODEL *model, VEC *pos, MAT *mat);
extern void SetModelLightOmni(MODEL *model, LIGHT *light, VEC *pos);
extern void AddModelLightOmni(MODEL *model, LIGHT *light, VEC *pos);
extern void SetModelLightOmniNormal(MODEL *model, LIGHT *light, VEC *pos);
extern void AddModelLightOmniNormal(MODEL *model, LIGHT *light, VEC *pos);
extern void SetModelLightSpot(MODEL *model, LIGHT *light, VEC *pos, VEC *dir);
extern void AddModelLightSpot(MODEL *model, LIGHT *light, VEC *pos, VEC *dir);
extern void SetModelLightSpotNormal(MODEL *model, LIGHT *light, VEC *pos, VEC *dir);
extern void AddModelLightSpotNormal(MODEL *model, LIGHT *light, VEC *pos, VEC *dir);
extern void SetModelLightShadow(MODEL *model, LIGHT *light, VEC *pos, MAT *mat, char inside);
extern void AddModelLightShadow(MODEL *model, LIGHT *light, VEC *pos, MAT *mat, long shadowcount, char inside);
extern void AddModelLightSimple(MODEL *model, VEC *pos);
extern void AddPermLightInstance(LIGHT *light);
extern void LoadLights(char *file);
extern void SaveLights(char *file);
extern void DrawFileLights(void);
extern void EditFileLights(void);
extern void DisplayLightInfo(LIGHT *light);
extern void LoadEditLightModels(void);
extern void FreeEditLightModels(void);
extern char LightVertexVisible(LIGHT *light, float *v);

// globals

extern LIGHT Light[];
extern LIGHT *CurrentEditLight;
extern short TotalLightCount;

#endif // LIGHT_H

