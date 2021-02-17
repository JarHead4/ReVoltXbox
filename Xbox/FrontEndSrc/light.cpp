//-----------------------------------------------------------------------------
// File: light.cpp
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"
#include "NewColl.h"
#include "light.h"
#include "dx.h"
#include "geom.h"
#include "particle.h"
#include "aerial.h"
#include "network.h"
#include "Body.h"
#include "car.h"
#include "camera.h"
#include "text.h"
#include "main.h"
#include "draw.h"
#include "input.h"
#include "LevelLoad.h"
#include "model.h"
#include "settings.h"

#include "SoundEffectEngine.h"
#include "SoundEffects.h"

// globals

static VEC LightDelta;
static char LightAxis = 0, LightAxisType = 0;
static short AffectFixedLightCount, AffectMovingLightCount, AffectObjectLightCount;
static LIGHT *AffectFixedLights[LIGHT_MAX];
static LIGHT *AffectMovingLights[LIGHT_MAX];
static LIGHT *AffectObjectLights[LIGHT_MAX];
static char InsideShadowFlags[LIGHT_MAX];
static MODEL LightOmniModel, LightSpotModel;
static char CurrentShadowSide = 0;

short TotalLightCount;
LIGHT Light[LIGHT_MAX];
LIGHT *CurrentEditLight;

long ShadowRgb;

// edit shit

static unsigned short DrawShadowIndex[] = {
    1, 3, 7, 5,
    2, 0, 4, 6,
    4, 5, 7, 6,
    0, 2, 3, 1,
    3, 2, 6, 7,
    0, 1, 5, 4,
};

static char *LightNames[] = {
    "Omni",
    "Omni Normal",
    "Spot",
    "Spot Normal",
    "Square Shadow",
};

static char *LightFlags[] = {
    "World Only",
    "Objects Only",
    "World and Objects",
};

static char *LightAxisNames[] = {
    "X Y",
    "X Z",
    "Z Y",
    "X",
    "Y",
    "Z",
};

static char *LightAxisTypeNames[] = {
    "Camera",
    "World",
};

static char *LightFlickerNames[] = {
    "Off",
    "On",
};

/////////////////
// init lights //
/////////////////

void InitLights(void)
{
    short i;

    for (i = 0 ; i < LIGHT_MAX ; i++) Light[i].Flag = 0;
}

/////////////////
// alloc light //
/////////////////

LIGHT *AllocLight(void)
{
    short i;

// find free slot

    for (i = 0 ; i < LIGHT_MAX ; i++) if (!Light[i].Flag)
    {
        return &Light[i];
    }

// no slots

    return NULL;
}

////////////////
// free light //
////////////////

void FreeLight(LIGHT *light)
{
    light->Flag = 0;
}

///////////////////////////
// process active lights //
///////////////////////////

void ProcessLights(void)
{
    short i;
    LIGHT *l;
    long t;

// not if lights off

    if (!RenderSettings.Light)
        return;

// zero fixed / moving / total count

    AffectFixedLightCount = 0;
    AffectMovingLightCount = 0;
    TotalLightCount = 0;

// loop thru active

    t = (long)(TimeStep * 1000.0f);
    l = Light;

    for (i = 0 ; i < LIGHT_MAX ; i++, l++) if (l->Flag)
    {

// inc total count

        TotalLightCount++;

// flicker?

        if (l->Flag & LIGHT_FLICKER)
        {
            if (((rand() % l->Speed) * 1000) < t)
            {
                l->Flag ^= LIGHT_OFF;
                PlaySfx3D(SFX_LIGHT_FLICKER, SFX_MAX_VOL, SFX_SAMPLE_RATE, (VEC*)&l->x, 0);
            }
        }

// off?

        if (l->Flag & LIGHT_OFF)
            continue;

// update tables

        if (l->Flag & LIGHT_FIXED)
            AffectFixedLights[AffectFixedLightCount++] = l;

        if (l->Flag & LIGHT_MOVING)
            AffectMovingLights[AffectMovingLightCount++] = l;

// update square reach / bounding box / cone mul

        ProcessOneLight(l);
    }
}

///////////////////////
// process one light //
///////////////////////

void ProcessOneLight(LIGHT *l)
{

// set square reach

    l->SquareReach = l->Reach * l->Reach;

// set bounding box

    l->Xmin = l->x - l->Reach;
    l->Xmax = l->x + l->Reach;
    l->Ymin = l->y - l->Reach;
    l->Ymax = l->y + l->Reach;
    l->Zmin = l->z - l->Reach;
    l->Zmax = l->z + l->Reach;

// set cone mul

    l->ConeMul = 180 / l->Cone;
}

///////////////////////////////
// add a perm light to world //
///////////////////////////////

void AddPermLight(LIGHT *light)
{
    short i;
    float dx, dy, dz;
    CUBE_HEADER *cube;

// valid light?

    if (!light->Flag) return;

// make sure all vars up to date

    ProcessOneLight(light);

// loop thru cubes

    cube = World.Cube;

    for (i = 0 ; i < World.CubeNum ; i++, cube++)
    {

// bounding boxes qualify?

        if (cube->Xmin > light->Xmax || cube->Xmax < light->Xmin || cube->Ymin > light->Ymax || cube->Ymax < light->Ymin || cube->Zmin > light->Zmax || cube->Zmax < light->Zmin) continue;

// radius test qualify?

        dx = cube->CentreX - light->x;
        dy = cube->CentreY - light->y;
        dz = cube->CentreZ - light->z;
        if ((float)sqrt(dx * dx + dy * dy + dz * dz) > light->Reach + cube->Radius) continue;

// yep, do it

        switch (light->Type)
        {
            case LIGHT_OMNI:
                AddCubeLightPermOmni(cube, light);
            break;
            case LIGHT_OMNINORMAL:
                AddCubeLightPermOmniNormal(cube, light);
            break;
            case LIGHT_SPOT:
                AddCubeLightPermSpot(cube, light);
            break;
            case LIGHT_SPOTNORMAL:
                AddCubeLightPermSpotNormal(cube, light);
            break;
        }
    }
}

///////////////////////////
// add cube light - perm //
///////////////////////////

void AddCubeLightPermOmni(CUBE_HEADER *cube, LIGHT *light)
{
    WORLD_POLY *p;
    WORLD_VERTEX **v;
    float squaredist;
    long mul;
    short i, j;
    long rgb[3];
    MODEL_RGB *col;

// loop thru polys

    p = cube->Model.PolyPtr;
    for (i = 0 ; i < cube->Model.PolyNum ; i++, p++)
    {
        v = &p->v0;
        col = (MODEL_RGB*)&p->rgb0;

        for (j = (p->Type & POLY_QUAD) ? 4 : 3 ; j ; j--, v++, col++)
        {
            LightDelta.v[X] = light->x - (*v)->x;
            LightDelta.v[Y] = light->y - (*v)->y;
            LightDelta.v[Z] = light->z - (*v)->z;

            squaredist = DotProduct(&LightDelta, &LightDelta);
            if (squaredist < light->SquareReach)
            {
                FTOL((1 - squaredist / light->SquareReach) * 256, mul);

                rgb[0] = (light->r * mul) >> 8;
                rgb[1] = (light->g * mul) >> 8;
                rgb[2] = (light->b * mul) >> 8;

                ModelAddGouraud(col, rgb, col);
            }
        }
    }
}

///////////////////////////
// add cube light - perm //
///////////////////////////

void AddCubeLightPermOmniNormal(CUBE_HEADER *cube, LIGHT *light)
{
    WORLD_POLY *p;
    WORLD_VERTEX **v;
    float squaredist, ang, dist;
    long mul;
    short i, j;
    long rgb[3];
    MODEL_RGB *col;

// loop thru polys

    p = cube->Model.PolyPtr;
    for (i = 0 ; i < cube->Model.PolyNum ; i++, p++)
    {
        v = &p->v0;
        col = (MODEL_RGB*)&p->rgb0;

        for (j = (p->Type & POLY_QUAD) ? 4 : 3 ; j ; j--, v++, col++)
        {
            LightDelta.v[X] = light->x - (*v)->x;
            LightDelta.v[Y] = light->y - (*v)->y;
            LightDelta.v[Z] = light->z - (*v)->z;

            ang = DotProduct(&LightDelta, (VEC*)&(*v)->nx);
            if (ang > 0)
            {
                squaredist = DotProduct(&LightDelta, &LightDelta);
                if (squaredist < light->SquareReach)
                {
                    dist = (float)sqrt(squaredist);
                    FTOL(((1 - squaredist / light->SquareReach) * (ang / dist)) * 256, mul);
        
                    rgb[0] = (light->r * mul) >> 8;
                    rgb[1] = (light->g * mul) >> 8;
                    rgb[2] = (light->b * mul) >> 8;

                    ModelAddGouraud(col, rgb, col);
                }
            }
        }
    }
}

///////////////////////////
// add cube light - perm //
///////////////////////////

void AddCubeLightPermSpot(CUBE_HEADER *cube, LIGHT *light)
{
    WORLD_POLY *p;
    WORLD_VERTEX **v;
    float squaredist, cone, dist;
    long mul;
    short i, j;
    long rgb[3];
    MODEL_RGB *col;

// loop thru polys

    p = cube->Model.PolyPtr;
    for (i = 0 ; i < cube->Model.PolyNum ; i++, p++)
    {
        v = &p->v0;
        col = (MODEL_RGB*)&p->rgb0;

        for (j = (p->Type & POLY_QUAD) ? 4 : 3 ; j ; j--, v++, col++)
        {
            LightDelta.v[X] = light->x - (*v)->x;
            LightDelta.v[Y] = light->y - (*v)->y;
            LightDelta.v[Z] = light->z - (*v)->z;

            squaredist = DotProduct(&LightDelta, &LightDelta);
            if (squaredist < light->SquareReach)
            {
                dist = (float)sqrt(squaredist);
                cone = (-DotProduct(&light->DirMatrix.mv[Z], &LightDelta) / dist - 1) * light->ConeMul + 1;
                if (cone > 0)
                {
                    FTOL(((1 - squaredist / light->SquareReach) * cone) * 256, mul);
        
                    rgb[0] = (light->r * mul) >> 8;
                    rgb[1] = (light->g * mul) >> 8;
                    rgb[2] = (light->b * mul) >> 8;

                    ModelAddGouraud(col, rgb, col);
                }
            }
        }
    }
}

///////////////////////////
// add cube light - perm //
///////////////////////////

void AddCubeLightPermSpotNormal(CUBE_HEADER *cube, LIGHT *light)
{
    WORLD_POLY *p;
    WORLD_VERTEX **v;
    float squaredist, ang, cone, dist;
    long mul;
    short i, j;
    long rgb[3];
    MODEL_RGB *col;

// loop thru polys

    p = cube->Model.PolyPtr;
    for (i = 0 ; i < cube->Model.PolyNum ; i++, p++)
    {
        v = &p->v0;
        col = (MODEL_RGB*)&p->rgb0;

        for (j = (p->Type & POLY_QUAD) ? 4 : 3 ; j ; j--, v++, col++)
        {
            LightDelta.v[X] = light->x - (*v)->x;
            LightDelta.v[Y] = light->y - (*v)->y;
            LightDelta.v[Z] = light->z - (*v)->z;

            ang = DotProduct(&LightDelta, (VEC*)&(*v)->nx);
            if (ang > 0)
            {
                squaredist = DotProduct(&LightDelta, &LightDelta);
                if (squaredist < light->SquareReach)
                {
                    dist = (float)sqrt(squaredist);
                    cone = (-DotProduct(&light->DirMatrix.mv[Z], &LightDelta) / dist - 1) * light->ConeMul + 1;
                    if (cone > 0)
                    {
                        FTOL(((1 - squaredist / light->SquareReach) * (ang / dist) * cone) * 256, mul);
        
                        rgb[0] = (light->r * mul) >> 8;
                        rgb[1] = (light->g * mul) >> 8;
                        rgb[2] = (light->b * mul) >> 8;

                        ModelAddGouraud(col, rgb, col);
                    }
                }
            }
        }
    }
}

///////////////////////////////
// check / add light to cube //
///////////////////////////////

char CheckCubeLight(CUBE_HEADER *cube)
{
    long i, flag;
    float dist, cone, conemul;
    LIGHT *l;

// not if lights off

    if (!RenderSettings.Light)
        return FALSE;

// loop thru all fixed lights

    flag = 0;

    for (i = 0 ; i < AffectFixedLightCount ; i++)
    {
        l = AffectFixedLights[i];

// bounding box test

        if (cube->Xmin > l->Xmax || cube->Xmax < l->Xmin || cube->Ymin > l->Ymax || cube->Ymax < l->Ymin || cube->Zmin > l->Zmax || cube->Zmax < l->Zmin) continue;

// radius test

        LightDelta.v[X] = cube->CentreX - l->x;
        LightDelta.v[Y] = cube->CentreY - l->y;
        LightDelta.v[Z] = cube->CentreZ - l->z;

        dist = Length(&LightDelta);
        if (dist > l->Reach + cube->Radius) continue;

// cone test if spot

        if ((l->Type == LIGHT_SPOT || l->Type == LIGHT_SPOTNORMAL) && dist > cube->Radius)
        {
            conemul = 1 / (cube->Radius / dist + l->Cone / 180);
            cone = (DotProduct(&l->DirMatrix.mv[Z], &LightDelta) / dist - 1) * conemul + 1;
            if (cone <= 0) continue;
        }

// yep, do it

        switch (l->Type)
        {
            case LIGHT_OMNI:
                if (flag) AddCubeLightOmni(cube, l);
                else SetCubeLightOmni(cube, l);
            break;
            case LIGHT_OMNINORMAL:
                if (flag) AddCubeLightOmniNormal(cube, l);
                else SetCubeLightOmniNormal(cube, l);
            break;
            case LIGHT_SPOT:
                if (flag) AddCubeLightSpot(cube, l);
                else SetCubeLightSpot(cube, l);
            break;
            case LIGHT_SPOTNORMAL:
                if (flag) AddCubeLightSpotNormal(cube, l);
                else SetCubeLightSpotNormal(cube, l);
            break;
        }
        flag++;
    }

// return result

    return (char)flag;
}

////////////////////
// set cube light //
////////////////////

void SetCubeLightOmni(CUBE_HEADER *cube, LIGHT *light)
{
    WORLD_VERTEX *v;
    float squaredist;
    long mul;
    short i;

// loop thru verts

    v = cube->Model.VertPtr;
    for (i = cube->Model.VertNum ; i ; i--, v++)
        {
        LightDelta.v[X] = light->x - v->x;
        LightDelta.v[Y] = light->y - v->y;
        LightDelta.v[Z] = light->z - v->z;

        squaredist = DotProduct(&LightDelta, &LightDelta);
        if (squaredist >= light->SquareReach)
        {
            v->r = v->g = v->b = 0;
            continue;
        }

        FTOL((1 - squaredist / light->SquareReach) * 256, mul);

        v->r = (light->r * mul) >> 8;
        v->g = (light->g * mul) >> 8;
        v->b = (light->b * mul) >> 8;
    }
}

////////////////////
// add cube light //
////////////////////

void AddCubeLightOmni(CUBE_HEADER *cube, LIGHT *light)
{
    WORLD_VERTEX *v;
    float squaredist;
    long mul;
    short i;

// loop thru verts

    v = cube->Model.VertPtr;
    for (i = cube->Model.VertNum ; i ; i--, v++)
    {
        LightDelta.v[X] = light->x - v->x;
        LightDelta.v[Y] = light->y - v->y;
        LightDelta.v[Z] = light->z - v->z;

        squaredist = DotProduct(&LightDelta, &LightDelta);
        if (squaredist >= light->SquareReach)
            continue;

        FTOL((1 - squaredist / light->SquareReach) * 256, mul);
        
        v->r += (light->r * mul) >> 8;
        v->g += (light->g * mul) >> 8;
        v->b += (light->b * mul) >> 8;
    }
}

////////////////////
// set cube light //
////////////////////

void SetCubeLightOmniNormal(CUBE_HEADER *cube, LIGHT *light)
{
    WORLD_VERTEX *v;
    float squaredist, ang, dist;
    long mul;
    short i;

// loop thru verts

    v = cube->Model.VertPtr;
    for (i = cube->Model.VertNum ; i ; i--, v++)
    {
        LightDelta.v[X] = light->x - v->x;
        LightDelta.v[Y] = light->y - v->y;
        LightDelta.v[Z] = light->z - v->z;

        ang = DotProduct(&LightDelta, (VEC*)&v->nx);
        if (ang <= 0)
        {
            v->r = v->g = v->b = 0;
            continue;
        }

        squaredist = DotProduct(&LightDelta, &LightDelta);
        if (squaredist >= light->SquareReach)
        {
            v->r = v->g = v->b = 0;
            continue;
        }

        dist = (float)sqrt(squaredist);
        FTOL(((1 - squaredist / light->SquareReach) * (ang / dist)) * 256, mul);

        v->r = (light->r * mul) >> 8;
        v->g = (light->g * mul) >> 8;
        v->b = (light->b * mul) >> 8;
    }
}

////////////////////
// add cube light //
////////////////////

void AddCubeLightOmniNormal(CUBE_HEADER *cube, LIGHT *light)
{
    WORLD_VERTEX *v;
    float squaredist, ang, dist;
    long mul;
    short i;

// loop thru verts

    v = cube->Model.VertPtr;
    for (i = cube->Model.VertNum ; i ; i--, v++)
    {
        LightDelta.v[X] = light->x - v->x;
        LightDelta.v[Y] = light->y - v->y;
        LightDelta.v[Z] = light->z - v->z;

        ang = DotProduct(&LightDelta, (VEC*)&v->nx);
        if (ang <= 0)
            continue;

        squaredist = DotProduct(&LightDelta, &LightDelta);
        if (squaredist >= light->SquareReach)
            continue;

        dist = (float)sqrt(squaredist);
        FTOL(((1 - squaredist / light->SquareReach) * (ang / dist)) * 256, mul);

        v->r += (light->r * mul) >> 8;
        v->g += (light->g * mul) >> 8;
        v->b += (light->b * mul) >> 8;
    }
}

////////////////////
// set cube light //
////////////////////

void SetCubeLightSpot(CUBE_HEADER *cube, LIGHT *light)
{
    WORLD_VERTEX *v;
    float squaredist, cone, dist;
    long mul;
    short i;

// loop thru verts

    v = cube->Model.VertPtr;
    for (i = cube->Model.VertNum ; i ; i--, v++)
    {
        LightDelta.v[X] = light->x - v->x;
        LightDelta.v[Y] = light->y - v->y;
        LightDelta.v[Z] = light->z - v->z;

        squaredist = DotProduct(&LightDelta, &LightDelta);
        if (squaredist >= light->SquareReach)
        {
            v->r = v->g = v->b = 0;
            continue;
        }

        dist = (float)sqrt(squaredist);

        cone = (-DotProduct(&light->DirMatrix.mv[Z], &LightDelta) / dist - 1) * light->ConeMul + 1;
        if (cone <= 0)
        {
            v->r = v->g = v->b = 0;
            continue;
        }

        FTOL(((1 - squaredist / light->SquareReach) * cone) * 256, mul);

        v->r = (light->r * mul) >> 8;
        v->g = (light->g * mul) >> 8;
        v->b = (light->b * mul) >> 8;
    }
}

////////////////////
// add cube light //
////////////////////

void AddCubeLightSpot(CUBE_HEADER *cube, LIGHT *light)
{
    WORLD_VERTEX *v;
    float squaredist, cone, dist;
    long mul;
    short i;

// loop thru verts

    v = cube->Model.VertPtr;
    for (i = cube->Model.VertNum ; i ; i--, v++)
    {
        LightDelta.v[X] = light->x - v->x;
        LightDelta.v[Y] = light->y - v->y;
        LightDelta.v[Z] = light->z - v->z;

        squaredist = DotProduct(&LightDelta, &LightDelta);
        if (squaredist >= light->SquareReach)
            continue;

        dist = (float)sqrt(squaredist);

        cone = (-DotProduct(&light->DirMatrix.mv[Z], &LightDelta) / dist - 1) * light->ConeMul + 1;
        if (cone <= 0)
            continue;

        FTOL(((1 - squaredist / light->SquareReach) * cone) * 256, mul);

        v->r += (light->r * mul) >> 8;
        v->g += (light->g * mul) >> 8;
        v->b += (light->b * mul) >> 8;
    }
}

////////////////////
// set cube light //
////////////////////

void SetCubeLightSpotNormal(CUBE_HEADER *cube, LIGHT *light)
{
    WORLD_VERTEX *v;
    float squaredist, ang, cone, dist;
    long mul;
    short i;

// loop thru verts

    v = cube->Model.VertPtr;
    for (i = cube->Model.VertNum ; i ; i--, v++)
    {
        LightDelta.v[X] = light->x - v->x;
        LightDelta.v[Y] = light->y - v->y;
        LightDelta.v[Z] = light->z - v->z;

        ang = DotProduct(&LightDelta, (VEC*)&v->nx);
        if (ang <= 0)
        {
            v->r = v->g = v->b = 0;
            continue;
        }

        squaredist = DotProduct(&LightDelta, &LightDelta);
        if (squaredist >= light->SquareReach)
        {
            v->r = v->g = v->b = 0;
            continue;
        }

        dist = (float)sqrt(squaredist);

        cone = (-DotProduct(&light->DirMatrix.mv[Z], &LightDelta) / dist - 1) * light->ConeMul + 1;
        if (cone <= 0)
        {
            v->r = v->g = v->b = 0;
            continue;
        }

        FTOL(((1 - squaredist / light->SquareReach) * (ang / dist) * cone) * 256, mul);

        v->r = (light->r * mul) >> 8;
        v->g = (light->g * mul) >> 8;
        v->b = (light->b * mul) >> 8;
    }
}

////////////////////
// add cube light //
////////////////////

void AddCubeLightSpotNormal(CUBE_HEADER *cube, LIGHT *light)
{
    WORLD_VERTEX *v;
    float squaredist, ang, cone, dist;
    long mul;
    short i;

// loop thru verts

    v = cube->Model.VertPtr;
    for (i = cube->Model.VertNum ; i ; i--, v++)
    {
        LightDelta.v[X] = light->x - v->x;
        LightDelta.v[Y] = light->y - v->y;
        LightDelta.v[Z] = light->z - v->z;

        ang = DotProduct(&LightDelta, (VEC*)&v->nx);
        if (ang <= 0)
            continue;

        squaredist = DotProduct(&LightDelta, &LightDelta);
        if (squaredist >= light->SquareReach)
            continue;

        dist = (float)sqrt(squaredist);

        cone = (-DotProduct(&light->DirMatrix.mv[Z], &LightDelta) / dist - 1) * light->ConeMul + 1;
        if (cone <= 0)
            continue;

        FTOL(((1 - squaredist / light->SquareReach) * (ang / dist) * cone) * 256, mul);

        v->r += (light->r * mul) >> 8;
        v->g += (light->g * mul) >> 8;
        v->b += (light->b * mul) >> 8;
    }
}

/////////////////////////
// check if object lit //
/////////////////////////

short CheckObjectLight(VEC *pos, BOUNDING_BOX *box, float rad)
{
    long i;
    LIGHT *l;
    float dist, cone, conemul, dx, dy, dz, x, y, z;
    VEC vec;

// not if lights off

    if (!RenderSettings.Light)
        return FALSE;

// zero affecting lights list

    AffectObjectLightCount = 0;

// loop thru all active lights

    for (i = 0 ; i < AffectMovingLightCount ; i++)
    {
        l = AffectMovingLights[i];

// bounding box test

        if (box->Xmin > l->Xmax || box->Xmax < l->Xmin ||
            box->Ymin > l->Ymax || box->Ymax < l->Ymin ||
            box->Zmin > l->Zmax || box->Zmax < l->Zmin) continue;

// radius test

        LightDelta.v[X] = pos->v[X] - l->x;
        LightDelta.v[Y] = pos->v[Y] - l->y;
        LightDelta.v[Z] = pos->v[Z] - l->z;
        dist = Length(&LightDelta);

        if (dist > l->Reach + rad) continue;

// cone test if spot

        if ((l->Type == LIGHT_SPOT || l->Type == LIGHT_SPOTNORMAL) && dist > rad)
        {
            conemul = 1 / (rad / dist + l->Cone / 180);
            cone = (DotProduct(&l->DirMatrix.mv[Z], &LightDelta) / dist - 1) * conemul + 1;
            if (cone <= 0) continue;
        }

// hull test if shadow

        if (l->Type == LIGHT_SHADOW)
        {
            SubVector(pos, (VEC*)&l->x, &vec);

            x = (float)l->r + rad;
            dx = DotProduct(&vec, &l->DirMatrix.mv[X]);
            if (dx < -x || dx > x) continue;

            y = (float)l->b + rad;
            dy = DotProduct(&vec, &l->DirMatrix.mv[Z]);
            if (dy < -y || dy > y) continue;

            z = (float)l->g + rad;
            dz = DotProduct(&vec, &l->DirMatrix.mv[Y]);
            if (dz < -z || dz > z) continue;

            x -= rad * 2;
            y -= rad * 2;
            z -= rad * 2;

            InsideShadowFlags[AffectObjectLightCount] = (dx > -x && dx < x && dy > -y && dy < y && dz > -z && dz < z);
        }

// ok, add light to list

        AffectObjectLights[AffectObjectLightCount++] = l;
    }

// return affecting count

    return AffectObjectLightCount;
}

///////////////////////////
// check if instance lit //
///////////////////////////

short CheckInstanceLight(INSTANCE *inst, float rad)
{
    long i;
    LIGHT *l;
    float dist, cone, conemul;
    BOUNDING_BOX *box = &inst->Box;
    VEC *pos = &inst->WorldPos;

// not if lights off

    if (!RenderSettings.Light)
        return FALSE;

// zero affecting lights list

    AffectObjectLightCount = 0;

// loop thru 'affect fixed' lights

    for (i = 0 ; i < AffectFixedLightCount ; i++)
    {
        l = AffectFixedLights[i];

// bounding box test

        if (box->Xmin > l->Xmax || box->Xmax < l->Xmin ||
            box->Ymin > l->Ymax || box->Ymax < l->Ymin ||
            box->Zmin > l->Zmax || box->Zmax < l->Zmin) continue;

// radius test

        LightDelta.v[X] = pos->v[X] - l->x;
        LightDelta.v[Y] = pos->v[Y] - l->y;
        LightDelta.v[Z] = pos->v[Z] - l->z;
        dist = Length(&LightDelta);

        if (dist > l->Reach + rad) continue;

// cone test if spot

        if ((l->Type == LIGHT_SPOT || l->Type == LIGHT_SPOTNORMAL) && dist > rad)
        {
            conemul = 1 / (rad / dist + l->Cone / 180);
            cone = (DotProduct(&l->DirMatrix.mv[Z], &LightDelta) / dist - 1) * conemul + 1;
            if (cone <= 0) continue;
        }

// yep, add light to list

        AffectObjectLights[AffectObjectLightCount++] = l;
    }

// return affecting count

    return AffectObjectLightCount;
}

///////////////////////////////////////
// check if instance lit - edit mode //
///////////////////////////////////////

short CheckInstanceLightEdit(INSTANCE *inst, float rad)
{
    long i;
    LIGHT *l;
    float dist, cone, conemul;
    BOUNDING_BOX *box = &inst->Box;
    VEC *pos = &inst->WorldPos;

// not if lights off

    if (!RenderSettings.Light)
        return FALSE;

// zero affecting lights list

    AffectObjectLightCount = 0;

// loop thru all lights

    l = Light;
    for (i = 0 ; i < LIGHT_MAX ; i++, l++) if (l->Flag & ~LIGHT_OFF)
    {

// file light skip?

        if ((inst->Flag & INSTANCE_NO_FILE_LIGHTS) && (l->Flag & LIGHT_FILE))
            continue;

// bounding box test

        if (box->Xmin > l->Xmax || box->Xmax < l->Xmin ||
            box->Ymin > l->Ymax || box->Ymax < l->Ymin ||
            box->Zmin > l->Zmax || box->Zmax < l->Zmin) continue;

// radius test

        LightDelta.v[X] = pos->v[X] - l->x;
        LightDelta.v[Y] = pos->v[Y] - l->y;
        LightDelta.v[Z] = pos->v[Z] - l->z;
        dist = Length(&LightDelta);

        if (dist > l->Reach + rad) continue;

// cone test if spot

        if ((l->Type == LIGHT_SPOT || l->Type == LIGHT_SPOTNORMAL) && dist > rad)
        {
            conemul = 1 / (rad / dist + l->Cone / 180);
            cone = (DotProduct(&l->DirMatrix.mv[Z], &LightDelta) / dist - 1) * conemul + 1;
            if (cone <= 0) continue;
        }

// yep, add light to list

        AffectObjectLights[AffectObjectLightCount++] = l;
    }

// return affecting count

    return AffectObjectLightCount;
}

////////////////////////////
// add model light - perm //
////////////////////////////

void AddModelLightPermOmni(MODEL *model, LIGHT *light, VEC *pos)
{
    MODEL_POLY *p;
    MODEL_VERTEX **v;
    float squaredist;
    long mul, vcount;
    short i, j;
    long rgb[3];
    POLY_RGB *col;

// loop thru polys

    col = model->PolyRGB;
    p = model->PolyPtr;

    for (i = 0 ; i < model->PolyNum ; i++, p++, col++)
    {
        v = &p->v0;

        vcount = (p->Type & POLY_QUAD) ? 4 : 3;
        for (j = 0 ; j < vcount ; j++, v++)
        {
            LightDelta.v[X] = pos->v[X] - (*v)->x;
            LightDelta.v[Y] = pos->v[Y] - (*v)->y;
            LightDelta.v[Z] = pos->v[Z] - (*v)->z;

            squaredist = DotProduct(&LightDelta, &LightDelta);
            if (squaredist < light->SquareReach)
            {
                FTOL((1 - squaredist / light->SquareReach) * 256, mul);

                rgb[0] = (light->r * mul) >> 8;
                rgb[1] = (light->g * mul) >> 8;
                rgb[2] = (light->b * mul) >> 8;

                ModelAddGouraud(&col->rgb[j], rgb, &col->rgb[j]);
            }
        }
    }
}

////////////////////////////
// add model light - perm //
////////////////////////////

void AddModelLightPermOmniNormal(MODEL *model, LIGHT *light, VEC *pos)
{
    MODEL_POLY *p;
    MODEL_VERTEX **v;
    float squaredist, ang, dist;
    long mul, vcount;
    short i, j;
    long rgb[3];
    POLY_RGB *col;

// loop thru polys

    col = model->PolyRGB;
    p = model->PolyPtr;

    for (i = 0 ; i < model->PolyNum ; i++, p++, col++)
    {
        v = &p->v0;

        vcount = (p->Type & POLY_QUAD) ? 4 : 3;
        for (j = 0 ; j < vcount ; j++, v++)
        {
            LightDelta.v[X] = pos->v[X] - (*v)->x;
            LightDelta.v[Y] = pos->v[Y] - (*v)->y;
            LightDelta.v[Z] = pos->v[Z] - (*v)->z;

            ang = DotProduct(&LightDelta, (VEC*)&(*v)->nx);
            if (ang > 0)
            {
                squaredist = DotProduct(&LightDelta, &LightDelta);
                if (squaredist < light->SquareReach)
                {
                    dist = (float)sqrt(squaredist);
                    FTOL(((1 - squaredist / light->SquareReach) * (ang / dist)) * 256, mul);

                    rgb[0] = (light->r * mul) >> 8;
                    rgb[1] = (light->g * mul) >> 8;
                    rgb[2] = (light->b * mul) >> 8;

                    ModelAddGouraud(&col->rgb[j], rgb, &col->rgb[j]);
                }
            }
        }
    }
}

////////////////////////////
// add model light - perm //
////////////////////////////

void AddModelLightPermSpot(MODEL *model, LIGHT *light, VEC *pos, VEC *dir)
{
    MODEL_POLY *p;
    MODEL_VERTEX **v;
    float squaredist, dist, cone;
    long mul, vcount;
    short i, j;
    long rgb[3];
    POLY_RGB *col;

// loop thru polys

    col = model->PolyRGB;
    p = model->PolyPtr;

    for (i = 0 ; i < model->PolyNum ; i++, p++, col++)
    {
        v = &p->v0;

        vcount = (p->Type & POLY_QUAD) ? 4 : 3;
        for (j = 0 ; j < vcount ; j++, v++)
        {
            LightDelta.v[X] = pos->v[X] - (*v)->x;
            LightDelta.v[Y] = pos->v[Y] - (*v)->y;
            LightDelta.v[Z] = pos->v[Z] - (*v)->z;

            squaredist = DotProduct(&LightDelta, &LightDelta);
            if (squaredist < light->SquareReach)
            {
                dist = (float)sqrt(squaredist);
                cone = (-DotProduct(dir, &LightDelta) / dist - 1) * light->ConeMul + 1;
                if (cone > 0)
                {
                    FTOL(((1 - squaredist / light->SquareReach) * cone) * 256, mul);

                    rgb[0] = (light->r * mul) >> 8;
                    rgb[1] = (light->g * mul) >> 8;
                    rgb[2] = (light->b * mul) >> 8;

                    ModelAddGouraud(&col->rgb[j], rgb, &col->rgb[j]);
                }
            }
        }
    }
}

////////////////////////////
// add model light - perm //
////////////////////////////

void AddModelLightPermSpotNormal(MODEL *model, LIGHT *light, VEC *pos, VEC *dir)
{
    MODEL_POLY *p;
    MODEL_VERTEX **v;
    float squaredist, ang, dist, cone;
    long mul, vcount;
    short i, j;
    long rgb[3];
    POLY_RGB *col;

// loop thru polys

    col = model->PolyRGB;
    p = model->PolyPtr;

    for (i = 0 ; i < model->PolyNum ; i++, p++, col++)
    {
        v = &p->v0;

        vcount = (p->Type & POLY_QUAD) ? 4 : 3;
        for (j = 0 ; j < vcount ; j++, v++)
        {
            LightDelta.v[X] = pos->v[X] - (*v)->x;
            LightDelta.v[Y] = pos->v[Y] - (*v)->y;
            LightDelta.v[Z] = pos->v[Z] - (*v)->z;

            ang = DotProduct(&LightDelta, (VEC*)&(*v)->nx);
            if (ang > 0)
            {
                squaredist = DotProduct(&LightDelta, &LightDelta);
                if (squaredist < light->SquareReach)
                {
                    dist = (float)sqrt(squaredist);
                    cone = (-DotProduct(dir, &LightDelta) / dist - 1) * light->ConeMul + 1;
                    if (cone > 0)
                    {
                        FTOL(((1 - squaredist / light->SquareReach) * (ang / dist) * cone) * 256, mul);

                        rgb[0] = (light->r * mul) >> 8;
                        rgb[1] = (light->g * mul) >> 8;
                        rgb[2] = (light->b * mul) >> 8;

                        ModelAddGouraud(&col->rgb[j], rgb, &col->rgb[j]);
                    }
                }
            }
        }
    }
}

////////////////////////
// add light to model //
////////////////////////

void AddModelLight(MODEL *model, VEC *pos, MAT *mat)
{
    long i, shadowcount;
    LIGHT *l;
    VEC vec, out, dir;
    MAT mat2;

// loop thru all affecting lights

    l = Light;
    shadowcount = 0;
    for (i = 0 ; i < AffectObjectLightCount ; i++)
    {
        l = AffectObjectLights[i];

// get relative position

        SubVector((VEC*)&l->x, pos, &vec);
        TransposeRotVector(mat, &vec, &out);

// set or add

        switch (l->Type)
        {
            case LIGHT_OMNI:
                if (i) AddModelLightOmni(model, l, &out);
                else SetModelLightOmni(model, l, &out);
                break;
            case LIGHT_OMNINORMAL:
                if (i) AddModelLightOmniNormal(model, l, &out);
                else SetModelLightOmniNormal(model, l, &out);
                break;
            case LIGHT_SPOT:
                TransposeRotVector(mat, &l->DirMatrix.mv[Z], &dir);
                if (i) AddModelLightSpot(model, l, &out, &dir);
                else SetModelLightSpot(model, l, &out, &dir);
                break;
            case LIGHT_SPOTNORMAL:
                TransposeRotVector(mat, &l->DirMatrix.mv[Z], &dir);
                if (i) AddModelLightSpotNormal(model, l, &out, &dir);
                else SetModelLightSpotNormal(model, l, &out, &dir);
                break;
            case LIGHT_SHADOW:
                MatMulTransMat(&l->DirMatrix, mat, &mat2);
                if (i) AddModelLightShadow(model, l, &out, &mat2, shadowcount, InsideShadowFlags[i]);
                else SetModelLightShadow(model, l, &out, &mat2, InsideShadowFlags[i]);
                shadowcount++;
                break;
        }
    }
}

/////////////////////
// set model light //
/////////////////////

void SetModelLightOmni(MODEL *model, LIGHT *light, VEC *pos)
{
    MODEL_VERTEX *v;
    float squaredist;
    long mul;
    short i;

// loop thru verts

    v = model->VertPtr;
    for (i = 0 ; i < model->VertNum ; i++, v++)
    {
        LightDelta.v[X] = pos->v[X] - v->x;
        LightDelta.v[Y] = pos->v[Y] - v->y;
        LightDelta.v[Z] = pos->v[Z] - v->z;

        squaredist = DotProduct(&LightDelta, &LightDelta);
        if (squaredist >= light->SquareReach)
        {
            v->r = v->g = v->b = 0;
            continue;
        }

        FTOL((1 - squaredist / light->SquareReach) * 256, mul);

        v->r = (light->r * mul) >> 8;
        v->g = (light->g * mul) >> 8;
        v->b = (light->b * mul) >> 8;
    }
}

/////////////////////
// add model light //
/////////////////////

void AddModelLightOmni(MODEL *model, LIGHT *light, VEC *pos)
{
    MODEL_VERTEX *v;
    float squaredist;
    long mul;
    short i;

// loop thru verts

    v = model->VertPtr;
    for (i = 0 ; i < model->VertNum ; i++, v++)
    {
        LightDelta.v[X] = pos->v[X] - v->x;
        LightDelta.v[Y] = pos->v[Y] - v->y;
        LightDelta.v[Z] = pos->v[Z] - v->z;

        squaredist = DotProduct(&LightDelta, &LightDelta);
        if (squaredist >= light->SquareReach)
            continue;

        FTOL((1 - squaredist / light->SquareReach) * 256, mul);

        v->r += (light->r * mul) >> 8;
        v->g += (light->g * mul) >> 8;
        v->b += (light->b * mul) >> 8;
    }
}

/////////////////////
// set model light //
/////////////////////

void SetModelLightOmniNormal(MODEL *model, LIGHT *light, VEC *pos)
{
    MODEL_VERTEX *v;
    float squaredist, dist, ang;
    long mul;
    short i;

// loop thru verts

    v = model->VertPtr;
    for (i = 0 ; i < model->VertNum ; i++, v++)
    {
        LightDelta.v[X] = pos->v[X] - v->x;
        LightDelta.v[Y] = pos->v[Y] - v->y;
        LightDelta.v[Z] = pos->v[Z] - v->z;

        ang = DotProduct(&LightDelta, (VEC*)&v->nx);
        if (ang <= 0)
        {
            v->r = v->g = v->b = 0;
            continue;
        }

        squaredist = DotProduct(&LightDelta, &LightDelta);
        if (squaredist >= light->SquareReach)
        {
            v->r = v->g = v->b = 0;
            continue;
        }

        dist = (float)sqrt(squaredist);
        FTOL(((1 - squaredist / light->SquareReach) * (ang / dist)) * 256, mul);

        v->r = (light->r * mul) >> 8;
        v->g = (light->g * mul) >> 8;
        v->b = (light->b * mul) >> 8;
    }
}

/////////////////////
// add model light //
/////////////////////

void AddModelLightOmniNormal(MODEL *model, LIGHT *light, VEC *pos)
{
    MODEL_VERTEX *v;
    float squaredist, dist, ang;
    long mul;
    short i;

// loop thru verts

    v = model->VertPtr;
    for (i = 0 ; i < model->VertNum ; i++, v++)
    {
        LightDelta.v[X] = pos->v[X] - v->x;
        LightDelta.v[Y] = pos->v[Y] - v->y;
        LightDelta.v[Z] = pos->v[Z] - v->z;

        ang = DotProduct(&LightDelta, (VEC*)&v->nx);
        if (ang <= 0)
            continue;

        squaredist = DotProduct(&LightDelta, &LightDelta);
        if (squaredist >= light->SquareReach)
            continue;

        dist = (float)sqrt(squaredist);
        FTOL(((1 - squaredist / light->SquareReach) * (ang / dist)) * 256, mul);

        v->r += (light->r * mul) >> 8;
        v->g += (light->g * mul) >> 8;
        v->b += (light->b * mul) >> 8;
    }
}

/////////////////////
// set model light //
/////////////////////

void SetModelLightSpot(MODEL *model, LIGHT *light, VEC *pos, VEC *dir)
{
    MODEL_VERTEX *v;
    float squaredist, dist, cone;
    long mul;
    short i;

// loop thru verts

    v = model->VertPtr;
    for (i = 0 ; i < model->VertNum ; i++, v++)
    {
        LightDelta.v[X] = pos->v[X] - v->x;
        LightDelta.v[Y] = pos->v[Y] - v->y;
        LightDelta.v[Z] = pos->v[Z] - v->z;

        squaredist = DotProduct(&LightDelta, &LightDelta);
        if (squaredist >= light->SquareReach)
        {
            v->r = v->g = v->b = 0;
            continue;
        }

        dist = (float)sqrt(squaredist);

        cone = (-DotProduct(dir, &LightDelta) / dist - 1) * light->ConeMul + 1;
        if (cone <= 0)
        {
            v->r = v->g = v->b = 0;
            continue;
        }

        FTOL(((1 - squaredist / light->SquareReach) * cone) * 256, mul);

        v->r = (light->r * mul) >> 8;
        v->g = (light->g * mul) >> 8;
        v->b = (light->b * mul) >> 8;
    }
}

/////////////////////
// add model light //
/////////////////////

void AddModelLightSpot(MODEL *model, LIGHT *light, VEC *pos, VEC *dir)
{
    MODEL_VERTEX *v;
    float squaredist, dist, cone;
    long mul;
    short i;

// loop thru verts

    v = model->VertPtr;
    for (i = 0 ; i < model->VertNum ; i++, v++)
    {
        LightDelta.v[X] = pos->v[X] - v->x;
        LightDelta.v[Y] = pos->v[Y] - v->y;
        LightDelta.v[Z] = pos->v[Z] - v->z;

        squaredist = DotProduct(&LightDelta, &LightDelta);
        if (squaredist >= light->SquareReach)
            continue;

        dist = (float)sqrt(squaredist);

        cone = (-DotProduct(dir, &LightDelta) / dist - 1) * light->ConeMul + 1;
        if (cone <= 0)
            continue;

        FTOL(((1 - squaredist / light->SquareReach) * cone) * 256, mul);

        v->r += (light->r * mul) >> 8;
        v->g += (light->g * mul) >> 8;
        v->b += (light->b * mul) >> 8;
    }
}

/////////////////////
// set model light //
/////////////////////

void SetModelLightSpotNormal(MODEL *model, LIGHT *light, VEC *pos, VEC *dir)
{
    MODEL_VERTEX *v;
    float squaredist, dist, ang, cone;
    long mul;
    short i;

// loop thru verts

    v = model->VertPtr;
    for (i = 0 ; i < model->VertNum ; i++, v++)
    {
        LightDelta.v[X] = pos->v[X] - v->x;
        LightDelta.v[Y] = pos->v[Y] - v->y;
        LightDelta.v[Z] = pos->v[Z] - v->z;

        ang = DotProduct(&LightDelta, (VEC*)&v->nx);
        if (ang <= 0)
        {
            v->r = v->g = v->b = 0;
            continue;
        }

        squaredist = DotProduct(&LightDelta, &LightDelta);
        if (squaredist >= light->SquareReach)
        {
            v->r = v->g = v->b = 0;
            continue;
        }

        dist = (float)sqrt(squaredist);

        cone = (-DotProduct(dir, &LightDelta) / dist - 1) * light->ConeMul + 1;
        if (cone <= 0)
        {
            v->r = v->g = v->b = 0;
            continue;
        }

        FTOL(((1 - squaredist / light->SquareReach) * (ang / dist) * cone) * 256, mul);

        v->r = (light->r * mul) >> 8;
        v->g = (light->g * mul) >> 8;
        v->b = (light->b * mul) >> 8;
    }
}

/////////////////////
// add model light //
/////////////////////

void AddModelLightSpotNormal(MODEL *model, LIGHT *light, VEC *pos, VEC *dir)
{
    MODEL_VERTEX *v;
    float squaredist, dist, ang, cone;
    long mul;
    short i;

// loop thru verts

    v = model->VertPtr;
    for (i = 0 ; i < model->VertNum ; i++, v++)
    {
        LightDelta.v[X] = pos->v[X] - v->x;
        LightDelta.v[Y] = pos->v[Y] - v->y;
        LightDelta.v[Z] = pos->v[Z] - v->z;

        ang = DotProduct(&LightDelta, (VEC*)&v->nx);
        if (ang <= 0)
            continue;

        squaredist = DotProduct(&LightDelta, &LightDelta);
        if (squaredist >= light->SquareReach)
            continue;

        dist = (float)sqrt(squaredist);

        cone = (-DotProduct(dir, &LightDelta) / dist - 1) * light->ConeMul + 1;
        if (cone <= 0)
            continue;

        FTOL(((1 - squaredist / light->SquareReach) * (ang / dist) * cone) * 256, mul);

        v->r += (light->r * mul) >> 8;
        v->g += (light->g * mul) >> 8;
        v->b += (light->b * mul) >> 8;
    }
}

/////////////////////
// set model light //
/////////////////////

void SetModelLightShadow(MODEL *model, LIGHT *light, VEC *pos, MAT *mat, char inside)
{
    MODEL_VERTEX *v;
    float dist, x, y, z;
    long i;

// set xyz sizes

    x = (float)light->r;
    y = (float)light->g;
    z = (float)light->b;

// inside job?

    if (inside)
    {
        v = model->VertPtr;
        for (i = 0 ; i < model->VertNum ; i++, v++)
        {
            v->r = ShadowRgb;
            v->g = ShadowRgb;
            v->b = ShadowRgb;
            v->Clip = TRUE;
        }
        return;
    }

// loop thru verts

    v = model->VertPtr;
    for (i = 0 ; i < model->VertNum ; i++, v++)
    {

// inside?

        v->r = v->g = v->b = 0;
        v->Clip = FALSE;

        LightDelta.v[X] = v->x - pos->v[X];
        LightDelta.v[Y] = v->y - pos->v[Y];
        LightDelta.v[Z] = v->z - pos->v[Z];

        dist = DotProduct(&LightDelta, &mat->mv[X]);
        if (dist < -x || dist > x) continue;

        dist = DotProduct(&LightDelta, &mat->mv[Z]);
        if (dist < -z || dist > z) continue;

        dist = DotProduct(&LightDelta, &mat->mv[Y]);
        if (dist < -y || dist > y) continue;

// yep

        v->r = ShadowRgb;
        v->g = ShadowRgb;
        v->b = ShadowRgb;
        v->Clip = TRUE;
    }
}

/////////////////////
// add model light //
/////////////////////

void AddModelLightShadow(MODEL *model, LIGHT *light, VEC *pos, MAT *mat, long shadowcount, char inside)
{
    MODEL_VERTEX *v;
    float dist, x, y, z;
    long i;

// set xyz sizes

    x = (float)light->r;
    y = (float)light->g;
    z = (float)light->b;

// inside job?

    if (inside)
    {
        v = model->VertPtr;
        for (i = 0 ; i < model->VertNum ; i++, v++)
        {
            if (shadowcount && v->Clip) continue;

            v->r += ShadowRgb;
            v->g += ShadowRgb;
            v->b += ShadowRgb;
            v->Clip = TRUE;
        }
        return;
    }

// loop thru verts

    v = model->VertPtr;
    for (i = 0 ; i < model->VertNum ; i++, v++)
    {

// skip if already done by overlapping shadow

        if (shadowcount && v->Clip) continue;

// inside?

        if (!shadowcount) v->Clip = FALSE;

        LightDelta.v[X] = v->x - pos->v[X];
        LightDelta.v[Y] = v->y - pos->v[Y];
        LightDelta.v[Z] = v->z - pos->v[Z];

        dist = DotProduct(&LightDelta, &mat->mv[X]);
        if (dist < -x || dist > x) continue;

        dist = DotProduct(&LightDelta, &mat->mv[Z]);
        if (dist < -z || dist > z) continue;

        dist = DotProduct(&LightDelta, &mat->mv[Y]);
        if (dist < -y || dist > y) continue;

// yep

        v->r += ShadowRgb;
        v->g += ShadowRgb;
        v->b += ShadowRgb;
        v->Clip = TRUE;
    }
}

/////////////////////////////////
// add light to model - simple //
/////////////////////////////////

void AddModelLightSimple(MODEL *model, VEC *pos)
{
    long i, j, mul, shadowflag;
    LIGHT *l;
    float squaredist, dist, cone, f;
    long r, g, b;
    MODEL_VERTEX *v;
    VEC vec;

// loop thru all affecting lights

    shadowflag = FALSE;
    l = Light;
    for (i = 0 ; i < AffectObjectLightCount ; i++)
    {
        l = AffectObjectLights[i];

// get light rgb

        r = g = b = 0;

        LightDelta.v[X] = l->x - pos->v[X];
        LightDelta.v[Y] = l->y - pos->v[Y];
        LightDelta.v[Z] = l->z - pos->v[Z];

        squaredist = DotProduct(&LightDelta, &LightDelta);
        if (squaredist < l->SquareReach)
        {
            if (l->Type == LIGHT_OMNI || l->Type == LIGHT_OMNINORMAL)
            {
                FTOL((1 - squaredist / l->SquareReach) * 256, mul);

                r = (l->r * mul) >> 8;
                g = (l->g * mul) >> 8;
                b = (l->b * mul) >> 8;
            }
            else if (l->Type == LIGHT_SPOT || l->Type == LIGHT_SPOTNORMAL)
            {
                dist = (float)sqrt(squaredist);

                cone = (-DotProduct(&l->DirMatrix.mv[Z], &LightDelta) / dist - 1) * l->ConeMul + 1;
                if (cone > 0)
                {
                    FTOL(((1 - squaredist / l->SquareReach) * cone) * 256, mul);

                    r = (l->r * mul) >> 8;
                    g = (l->g * mul) >> 8;
                    b = (l->b * mul) >> 8;
                }
            }
            else if (l->Type == LIGHT_SHADOW)
            {
                if (!shadowflag)
                {
                    SetVector(&vec, -LightDelta.v[X], -LightDelta.v[Y], -LightDelta.v[Z]);

                    f = (float)l->r;
                    dist = DotProduct(&vec, &l->DirMatrix.mv[X]);
                    if (dist > -f && dist < f)
                    {
                        f = (float)l->b;
                        dist = DotProduct(&vec, &l->DirMatrix.mv[Z]);
                        if (dist > -f && dist < f)
                        {
                            f = (float)l->g;
                            dist = DotProduct(&vec, &l->DirMatrix.mv[Y]);
                            if (dist > -f && dist < f)
                            {
                                r = g = b = ShadowRgb;
                                shadowflag = TRUE;
                            }
                        }
                    }
                }
            }
        }

// set rgb

        if (!i)
        {
            v = model->VertPtr;
            for (j = 0 ; j < model->VertNum ; j++, v++)
            {
                v->r = r;
                v->g = g;
                v->b = b;
            }
        }

// add to rgb

        else if (r | g | b)
        {
            v = model->VertPtr;
            for (j = 0 ; j < model->VertNum ; j++, v++)
            {
                v->r += r;
                v->g += g;
                v->b += b;
            }
        }
    }
}

/////////////////////////////////
// add perm light to instances //
/////////////////////////////////

void AddPermLightInstance(LIGHT *light)
{
    short i, j;
    INSTANCE *inst;
    VEC vec, pos, dir;
    MODEL *model;
    POLY_RGB *savergb;

// valid light?

    if (!light->Flag) return;

// make sure all vars up to date

    ProcessOneLight(light);

// loop thru instances

    inst = Instances;

    for (i = 0 ; i < InstanceNum ; i++, inst++)
    {

// skip if 'mirror hide' instance

        if (inst->Flag & INSTANCE_HIDE)
            continue;

// skip if not file lit

        if (inst->Flag & INSTANCE_NO_FILE_LIGHTS)
            continue;

// loop thru LOD's for instance

        for (j = 0 ; j < InstanceModels[inst->Model].Count ; j++)
        {

// bounding boxes qualify?

            if (inst->Box.Xmin > light->Xmax || inst->Box.Xmax < light->Xmin || inst->Box.Ymin > light->Ymax || inst->Box.Ymax < light->Ymin || inst->Box.Zmin > light->Zmax || inst->Box.Zmax < light->Zmin)
                continue;

// yep, go!

            model = &InstanceModels[inst->Model].Models[j];
            savergb = model->PolyRGB;
            model->PolyRGB = inst->rgb[j];

            SubVector((VEC*)&light->x, &inst->WorldPos, &vec);
            TransposeRotVector(&inst->WorldMatrix, &vec, &pos);

            switch (light->Type)
            {
                case LIGHT_OMNI:
                    AddModelLightPermOmni(model, light, &pos);
                    break;
                case LIGHT_OMNINORMAL:
                    AddModelLightPermOmniNormal(model, light, &pos);
                    break;
                case LIGHT_SPOT:
                    TransposeRotVector(&inst->WorldMatrix, &light->DirMatrix.mv[Z], &dir);
                    AddModelLightPermSpot(model, light, &pos, &dir);
                    break;
                case LIGHT_SPOTNORMAL:
                    TransposeRotVector(&inst->WorldMatrix, &light->DirMatrix.mv[Z], &dir);
                    AddModelLightPermSpotNormal(model, light, &pos, &dir);
                    break;
            }
            model->PolyRGB = savergb;
        }
    }
}

///////////////////////////////
// load / setup track lights //
///////////////////////////////

void LoadLights(char *file)
{
    long i;
    FILE *fp;
    LIGHT *light;
    FILELIGHT fl;

// open light file

    fp = fopen(file, "rb");

// if not there create empty one

    if (!fp)
    {
        fp = fopen(file, "wb");
        if (!fp) return;
        i = 0;
        fwrite(&i, sizeof(i), 1, fp);
        fclose(fp);
        fp = fopen(file, "rb");
        if (!fp) return;
    }

// loop thru all lights

    fread(&i, sizeof(i), 1, fp);

    for ( ; i ; i--)
    {

// alloc light

        light = AllocLight();
        if (!light) break;

// setup from file

        fread(&fl, sizeof(fl), 1, fp);

        light->Flag = fl.Flag;
        light->Type = fl.Type;
        light->Speed = fl.Speed;

        light->x = fl.x;
        light->y = fl.y;
        light->z = fl.z;

        light->Reach = fl.Reach;

        CopyMatrix(&fl.DirMatrix, &light->DirMatrix);

        light->Cone = fl.Cone;

        light->r = (long)fl.r;
        light->g = (long)fl.g;
        light->b = (long)fl.b;

// no flicker on shadows

        if (light->Type == LIGHT_SHADOW)
            light->Flag &= ~LIGHT_FLICKER;

// do perm affect if not flickering

        if ( !(light->Flag & LIGHT_FLICKER))
        {

// affect world if LIGHT_FIXED flag set

            if (light->Flag & LIGHT_FIXED)
            {
                AddPermLight(light);
                light->Flag &= ~LIGHT_FIXED;
            }

// always affect instances

            AddPermLightInstance(light);

// then kill if not affecting moving objects

            if (!(light->Flag & LIGHT_MOVING))
                FreeLight(light);
        }
    }

// close light file

    fclose(fp);
}

