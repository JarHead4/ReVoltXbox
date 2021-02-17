//$REVISIT(cprince): should this file be renamed EditLight.cpp?  (Same for light.h vs EditLight.h)
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
#ifdef OLD_AUDIO
                PlaySfx3D(SFX_LIGHT_FLICKER, SFX_MAX_VOL, SFX_SAMPLE_RATE, (VEC*)&l->x, 0);
#else
                g_SoundEngine.Play3DSound( EFFECT_LightFlk, 
                                           FALSE, 
                                           l->x,
                                           l->y,
                                           l->z );
#endif // OLD_AUDIO
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

        if (EditMode == EDIT_LIGHTS)
        {
            VecMulScalar((VEC*)&fl.x, EditScale);
            VecPlusEqVec((VEC*)&fl.x, &EditOffset);
            fl.Reach *= EditScale;
        }

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

// do perm affect if not light or instance edit mode and not flickering

        if (EditMode != EDIT_LIGHTS && EditMode != EDIT_INSTANCES && !(light->Flag & LIGHT_FLICKER))
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

/////////////////
// save lights //
/////////////////

void SaveLights(char *file)
{
    long num, i;
    FILE *fp;
    LIGHT *light;
    FILELIGHT fl;
    char bak[256];

// backup old file

    memcpy(bak, file, strlen(file) - 3);
    sprintf(bak + strlen(file) - 3, "li-");
    remove(bak);
    rename(file, bak);

// open light file

    fp = fopen(file, "wb");
    if (!fp) return;

// count num

    for (i = num = 0 ; i < LIGHT_MAX ; i++) if (Light[i].Flag & LIGHT_FILE) num++;

// write num

    fwrite(&num, sizeof(num), 1, fp);

// write out each file light

    light = Light;
    for (i = 0 ; i < LIGHT_MAX ; i++, light++) if (light->Flag & LIGHT_FILE)
    {

// set file light

        fl.Flag = light->Flag & ~LIGHT_OFF;
        fl.Type = light->Type;
        fl.Speed = light->Speed;

        fl.x = light->x;
        fl.y = light->y;
        fl.z = light->z;

        fl.Reach = light->Reach;

        CopyMatrix(&light->DirMatrix, &fl.DirMatrix);

        fl.Cone = light->Cone;

        fl.r = (float)light->r;
        fl.g = (float)light->g;
        fl.b = (float)light->b;

// write it

        fwrite(&fl, sizeof(fl), 1, fp);
    }

// close file

    DumpMessage("Saved Light File:", file);
    fclose(fp);
}

//////////////////////////
// draw all file lights //
//////////////////////////

void DrawFileLights(void)
{
    long i, j, k, col, scol[4];
    LIGHT *light;
    MAT *lmat;
    VEC tempvec, v[8], vpos[8], pos[4];
    MODEL *model;

// loop thru all file lights

    light = Light;
    for (i = 0 ; i < LIGHT_MAX ; i++, light++) if (light->Flag & LIGHT_FILE)
    {

// draw

        if (light->Type == LIGHT_SPOT || light->Type == LIGHT_SPOTNORMAL)
        {
            model = &LightSpotModel;
            lmat = &light->DirMatrix;
        }
        else
        {
            model = &LightOmniModel;
            lmat = &IdentityMatrix;
        }

        DrawModel(model, lmat, (VEC*)&light->x, MODEL_PLAIN);

// draw axis

        if (light == CurrentEditLight)
        {
            if (LightAxisType)
                DrawAxis(&IdentityMatrix, (VEC*)&light->x);
            else
                DrawAxis(&CAM_MainCamera->WMatrix, (VEC*)&light->x);
        }

// if spot, draw dir

        if (light->Type == LIGHT_SPOT || light->Type == LIGHT_SPOTNORMAL)
        {
            tempvec.v[X] = light->x + light->DirMatrix.m[LX] * light->Reach;
            tempvec.v[Y] = light->y + light->DirMatrix.m[LY] * light->Reach;
            tempvec.v[Z] = light->z + light->DirMatrix.m[LZ] * light->Reach;

            col = (long)(light->r * 65536 + light->g * 256 + light->b);
            DrawLine((VEC*)&light->x, &tempvec, col, 0);
        }

// if shadow, draw box

        if (light->Type == LIGHT_SHADOW)
        {
            SetVector(&v[0], (float)-light->r, (float)-light->g, (float)-light->b);
            SetVector(&v[1], (float)light->r, (float)-light->g, (float)-light->b);
            SetVector(&v[2], (float)-light->r, (float)-light->g, (float)light->b);
            SetVector(&v[3], (float)light->r, (float)-light->g, (float)light->b);

            SetVector(&v[4], (float)-light->r, (float)light->g, (float)-light->b);
            SetVector(&v[5], (float)light->r, (float)light->g, (float)-light->b);
            SetVector(&v[6], (float)-light->r, (float)light->g, (float)light->b);
            SetVector(&v[7], (float)light->r, (float)light->g, (float)light->b);

            for (j = 0 ; j < 8 ; j++)
            {
                RotTransVector(&light->DirMatrix, (VEC*)&light->x, &v[j], &vpos[j]);
            }

            FlushPolyBuckets();
            SET_TPAGE(-1);
            BLEND_ALPHA();
            BLEND_SRC(D3DBLEND_SRCALPHA);
            BLEND_DEST(D3DBLEND_INVSRCALPHA);
            ZWRITE_OFF();

            for (j = 0 ; j < 6 ; j++)
            {
                for (k = 0 ; k < 4 ; k++)
                {
                    pos[k] = vpos[DrawShadowIndex[j * 4 + k]];
                    if (j == CurrentShadowSide && light == CurrentEditLight)
                        scol[k] = (rand() & 255) | (rand() & 255) << 8 | (rand() & 255) << 16 | 0x80000000;
                    else
                        scol[k] = 0x80000000;
                }

                DrawNearClipPolyTEX0(pos, scol, 4);
            }

            ZWRITE_ON();
            BLEND_OFF();
        }
    }
}

//////////////////////
// edit file lights //
//////////////////////

void EditFileLights(void)
{
    short i, n;
    float z, xrad, yrad, sx, sy, add;
    VEC vec, vec2;
    VEC r, u, l, r2, u2, l2;
    MAT mat, mat2;
    LIGHT *light;

// quit if not in edit mode

    if (CAM_MainCamera->Type != CAM_EDIT)
    {
        CurrentEditLight = NULL;
        return;
    }

// rotate camera?

//$REMOVED (tentative!!)
//    if (MouseRight && (!CurrentEditLight || (CurrentEditLight->Type != LIGHT_SPOT && CurrentEditLight->Type != LIGHT_SPOTNORMAL)))
//    {
//        RotMatrixZYX(&mat, (float)-Mouse.lY / 3072, -(float)Mouse.lX / 3072, 0);
//        MulMatrix(&CAM_MainCamera->WMatrix, &mat, &mat2);
//        CopyMatrix(&mat2, &CAM_MainCamera->WMatrix);
//
//        CAM_MainCamera->WMatrix.m[RY] = 0;
//        NormalizeVector(&CAM_MainCamera->WMatrix.mv[X]);
//        CrossProduct(&CAM_MainCamera->WMatrix.mv[Z], &CAM_MainCamera->WMatrix.mv[X], &CAM_MainCamera->WMatrix.mv[Y]);
//        NormalizeVector(&CAM_MainCamera->WMatrix.mv[Y]);
//        CrossProduct(&CAM_MainCamera->WMatrix.mv[X], &CAM_MainCamera->WMatrix.mv[Y], &CAM_MainCamera->WMatrix.mv[Z]);
//    }
//$END_REMOVAL

// save lights?

    if (Keys[DIK_LCONTROL] && Keys[DIK_F4] && !LastKeys[DIK_F4])
    {
        SaveLights(GetLevelFilename("lit", FILENAME_MAKE_BODY));
    }

// get a current light?

    if (!CurrentEditLight && Keys[DIK_RETURN] && !LastKeys[DIK_RETURN])
    {
        n = -1;
        z = RenderSettings.FarClip;

        for (i = 0 ; i < LIGHT_MAX ; i++) if (Light[i].Flag & LIGHT_FILE)
        {
            RotTransVector(&ViewMatrix, &ViewTrans, (VEC*)&Light[i].x, &vec);

            if (vec.v[Z] < RenderSettings.NearClip || vec.v[Z] >= RenderSettings.FarClip) continue;

            sx = vec.v[X] * RenderSettings.GeomPers / vec.v[Z] + REAL_SCREEN_XHALF;
            sy = vec.v[Y] * RenderSettings.GeomPers / vec.v[Z] + REAL_SCREEN_YHALF;

            xrad = (24 * RenderSettings.GeomPers) / vec.v[Z];
            yrad = (24 * RenderSettings.GeomPers) / vec.v[Z];

            if (MouseXpos > sx - xrad && MouseXpos < sx + xrad && MouseYpos > sy - yrad && MouseYpos < sy + yrad)
            {
                if (vec.v[Z] < z)
                {
                    n = i;
                    z = vec.v[Z];
                }
            }
        }
        if (n != -1)
        {
            CurrentEditLight = &Light[n];
            return;
        }
    }

// new light?

    if (Keys[DIK_INSERT] && !LastKeys[DIK_INSERT])
    {
        if ((light = AllocLight()))
        {
            vec.v[X] = 0;
            vec.v[Y] = 0;
            vec.v[Z] = 256;
            RotVector(&CAM_MainCamera->WMatrix, &vec, &vec2);
            light->x = CAM_MainCamera->WPos.v[X] + vec2.v[X];
            light->y = CAM_MainCamera->WPos.v[Y] + vec2.v[Y];
            light->z = CAM_MainCamera->WPos.v[Z] + vec2.v[Z];

            light->Reach = 512;

            RotMatrixZYX(&light->DirMatrix, 0.25f, 0, 0);

            light->Cone = 90;

            light->r = 0;
            light->g = 0;
            light->b = 0;

            light->Flag = LIGHT_FILE | LIGHT_FIXED | LIGHT_MOVING;
            light->Type = LIGHT_OMNI;
            light->Speed = 1;

            CurrentEditLight = light;
        }
    }

// quit now if no current light

    if (!CurrentEditLight) return;

// exit current light edit?

    if (Keys[DIK_RETURN] && !LastKeys[DIK_RETURN])
    {
        CurrentEditLight = NULL;
        return;
    }

// delete light?

    if (Keys[DIK_DELETE] && !LastKeys[DIK_DELETE])
    {
        FreeLight(CurrentEditLight);
        CurrentEditLight = NULL;
        return;
    }

// change light type?

    if (Keys[DIK_SPACE] && !LastKeys[DIK_SPACE])
    {
        if (Keys[DIK_LSHIFT])
            CurrentEditLight->Type--;
        else
            CurrentEditLight->Type++;

        if (CurrentEditLight->Type > 5) CurrentEditLight->Type = 4;
        if (CurrentEditLight->Type == 5) CurrentEditLight->Type = 0;
    }

// change light flag?

    if (Keys[DIK_NUMPADENTER] && !LastKeys[DIK_NUMPADENTER])
    {
        switch (CurrentEditLight->Flag & (LIGHT_FIXED | LIGHT_MOVING))
        {
            case LIGHT_FIXED | LIGHT_MOVING:
                CurrentEditLight->Flag &= ~LIGHT_MOVING;
                break;
            case LIGHT_FIXED:
                CurrentEditLight->Flag &= ~LIGHT_FIXED;
                CurrentEditLight->Flag |= LIGHT_MOVING;
                break;
            case LIGHT_MOVING:
                CurrentEditLight->Flag |= LIGHT_FIXED;
                break;
        }
    }

    if (CurrentEditLight->Type == LIGHT_SHADOW)
        CurrentEditLight->Flag &= ~LIGHT_FIXED;

// change reach?

    if (CurrentEditLight->Type != LIGHT_SHADOW)
    {
        if (Keys[DIK_NUMPADPLUS] && CurrentEditLight->Reach < 65536) CurrentEditLight->Reach += 16;
        if (Keys[DIK_NUMPADMINUS] && CurrentEditLight->Reach > 0) CurrentEditLight->Reach -= 16;
    }

// set reach if shadow

    if (CurrentEditLight->Type == LIGHT_SHADOW)
    {
        CurrentEditLight->Reach = (float)sqrt((float)CurrentEditLight->r * (float)CurrentEditLight->r + (float)CurrentEditLight->g * (float)CurrentEditLight->g + (float)CurrentEditLight->b * (float)CurrentEditLight->b);
    }

// change cone?

    if (CurrentEditLight->Type == LIGHT_SPOT || CurrentEditLight->Type == LIGHT_SPOTNORMAL)
    {
        if (Keys[DIK_NUMPADSTAR] && CurrentEditLight->Cone < 180) CurrentEditLight->Cone++;
        if (Keys[DIK_NUMPADSLASH] && CurrentEditLight->Cone > 1) CurrentEditLight->Cone--;
    }

// change shadow side?

    if (CurrentEditLight->Type == LIGHT_SHADOW)
    {
        if (Keys[DIK_NUMPADPLUS] && !LastKeys[DIK_NUMPADPLUS])
            CurrentShadowSide = (CurrentShadowSide + 1) % 6;

        if (Keys[DIK_NUMPADMINUS] && !LastKeys[DIK_NUMPADMINUS])
        {
            CurrentShadowSide--;
            if (CurrentShadowSide == -1) CurrentShadowSide = 5;
        }   
    }

// change rgb?

    if (CurrentEditLight->Type != LIGHT_SHADOW)
    {
        if (Keys[DIK_LSHIFT]) LastKeys[DIK_NUMPAD4] = LastKeys[DIK_NUMPAD5] = LastKeys[DIK_NUMPAD6] = LastKeys[DIK_NUMPAD7] = LastKeys[DIK_NUMPAD8] = LastKeys[DIK_NUMPAD9] = 0;

        if (Keys[DIK_NUMPAD4] && !LastKeys[DIK_NUMPAD4] && CurrentEditLight->r > -1024) CurrentEditLight->r--;
        if (Keys[DIK_NUMPAD7] && !LastKeys[DIK_NUMPAD7] && CurrentEditLight->r < 1024) CurrentEditLight->r++;

        if (Keys[DIK_NUMPAD5] && !LastKeys[DIK_NUMPAD5] && CurrentEditLight->g > -1024) CurrentEditLight->g--;
        if (Keys[DIK_NUMPAD8] && !LastKeys[DIK_NUMPAD8] && CurrentEditLight->g < 1024) CurrentEditLight->g++;

        if (Keys[DIK_NUMPAD6] && !LastKeys[DIK_NUMPAD6] && CurrentEditLight->b > -1024) CurrentEditLight->b--;
        if (Keys[DIK_NUMPAD9] && !LastKeys[DIK_NUMPAD9] && CurrentEditLight->b < 1024) CurrentEditLight->b++;
    }

// change axis?

    if (Keys[DIK_TAB] && !LastKeys[DIK_TAB])
    {
        if (Keys[DIK_LSHIFT]) LightAxis--;
        else LightAxis++;
        if (LightAxis == -1) LightAxis = 5;
        if (LightAxis == 6) LightAxis = 0;
    }

// change axis type?

    if (Keys[DIK_LALT] && !LastKeys[DIK_LALT])
        LightAxisType ^= 1;

// toggle off / on

    if (Keys[DIK_O] && !LastKeys[DIK_O])
        CurrentEditLight->Flag ^= LIGHT_OFF;

// toggle flicker off / on

    if (Keys[DIK_F] && !LastKeys[DIK_F])
        CurrentEditLight->Flag ^= LIGHT_FLICKER;

// change flicker speed

    if (Keys[DIK_EQUALS] && CurrentEditLight->Speed < 255) CurrentEditLight->Speed++;
    if (Keys[DIK_MINUS] && CurrentEditLight->Speed > 1) CurrentEditLight->Speed--;

// copy light?

    if (MouseLeft && !MouseLastLeft && Keys[DIK_LSHIFT])
    {
        if ((light = AllocLight()))
        {
            memcpy(light, CurrentEditLight, sizeof(LIGHT));
            CurrentEditLight = light;
            return;
        }
    }

// move light?

    if (MouseLeft)
    {
        RotTransVector(&ViewMatrix, &ViewTrans, (VEC*)&CurrentEditLight->x, &vec);

        switch (LightAxis)
        {
            case LIGHT_AXIS_XY:
                vec.v[X] = MouseXrel * vec.v[Z] / RenderSettings.GeomPers + CameraEditXrel;
                vec.v[Y] = MouseYrel * vec.v[Z] / RenderSettings.GeomPers + CameraEditYrel;
                vec.v[Z] = CameraEditZrel;
                break;
            case LIGHT_AXIS_XZ:
                vec.v[X] = MouseXrel * vec.v[Z] / RenderSettings.GeomPers + CameraEditXrel;
                vec.v[Y] = CameraEditYrel;
                vec.v[Z] = -MouseYrel * vec.v[Z] / RenderSettings.GeomPers + CameraEditZrel;
                break;
            case LIGHT_AXIS_ZY:
                vec.v[X] = CameraEditXrel;
                vec.v[Y] = MouseYrel * vec.v[Z] / RenderSettings.GeomPers + CameraEditYrel;
                vec.v[Z] = MouseXrel * vec.v[Z] / RenderSettings.GeomPers + CameraEditZrel;
                break;
            case LIGHT_AXIS_X:
                vec.v[X] = MouseXrel * vec.v[Z] / RenderSettings.GeomPers + CameraEditXrel;
                vec.v[Y] = CameraEditYrel;
                vec.v[Z] = CameraEditZrel;
                break;
            case LIGHT_AXIS_Y:
                vec.v[X] = CameraEditXrel;
                vec.v[Y] = MouseYrel * vec.v[Z] / RenderSettings.GeomPers + CameraEditYrel;
                vec.v[Z] = CameraEditZrel;
                break;
            case LIGHT_AXIS_Z:
                vec.v[X] = CameraEditXrel;
                vec.v[Y] = CameraEditYrel;
                vec.v[Z] = -MouseYrel * vec.v[Z] / RenderSettings.GeomPers + CameraEditZrel;
                break;
        }

        if (LightAxisType == 1)
        {
            SetVector(&vec2, vec.v[X], vec.v[Y], vec.v[Z]);
        }
        else
        {
            RotVector(&CAM_MainCamera->WMatrix, &vec, &vec2);
        }

        CurrentEditLight->x += vec2.v[X];
        CurrentEditLight->y += vec2.v[Y];
        CurrentEditLight->z += vec2.v[Z];
    }

// rotate light?

    if (MouseRight && CurrentEditLight->Type == LIGHT_SPOT || CurrentEditLight->Type == LIGHT_SPOTNORMAL)
    {
        RotVector(&ViewMatrix, &CurrentEditLight->DirMatrix.mv[X], &r);
        RotVector(&ViewMatrix, &CurrentEditLight->DirMatrix.mv[Y], &u);
        RotVector(&ViewMatrix, &CurrentEditLight->DirMatrix.mv[Z], &l);

        RotMatrixZYX(&mat, MouseYrel / 1024, -MouseXrel / 1024, 0);

        RotVector(&mat, &r, &r2);
        RotVector(&mat, &u, &u2);
        RotVector(&mat, &l, &l2);

        RotVector(&CAM_MainCamera->WMatrix, &r2, &CurrentEditLight->DirMatrix.mv[X]);
        RotVector(&CAM_MainCamera->WMatrix, &u2, &CurrentEditLight->DirMatrix.mv[Y]);
        RotVector(&CAM_MainCamera->WMatrix, &l2, &CurrentEditLight->DirMatrix.mv[Z]);

        NormalizeMatrix(&CurrentEditLight->DirMatrix);
    }

// rotate shadow?

    if (CurrentEditLight->Type == LIGHT_SHADOW)
    {
        vec.v[X] = vec.v[Y] = vec.v[Z] = 0;

        if (Keys[DIK_NUMPAD7]) vec.v[X] -= 0.001f;
        if (Keys[DIK_NUMPAD4]) vec.v[X] += 0.001f;
        if (Keys[DIK_NUMPAD8]) vec.v[Y] -= 0.001f;
        if (Keys[DIK_NUMPAD5]) vec.v[Y] += 0.001f;
        if (Keys[DIK_NUMPAD9]) vec.v[Z] -= 0.001f;
        if (Keys[DIK_NUMPAD6]) vec.v[Z] += 0.001f;

        if (Keys[DIK_NUMPAD1] && !LastKeys[DIK_NUMPAD1]) vec.v[X] += 0.125f;
        if (Keys[DIK_NUMPAD2] && !LastKeys[DIK_NUMPAD2]) vec.v[Y] += 0.125f;
        if (Keys[DIK_NUMPAD3] && !LastKeys[DIK_NUMPAD3]) vec.v[Z] += 0.125f;

        if (Keys[DIK_NUMPAD0]) CopyMatrix(&IdentityMatrix, &CurrentEditLight->DirMatrix);

        RotMatrixZYX(&mat, vec.v[X], vec.v[Y], vec.v[Z]);

        MulMatrix(&mat, &CurrentEditLight->DirMatrix, &mat2);
        CopyMatrix(&mat2, &CurrentEditLight->DirMatrix);
        NormalizeMatrix(&CurrentEditLight->DirMatrix);
    }

// move a shadow side?

    if (CurrentEditLight->Type == LIGHT_SHADOW)
    {
        add = 0;
        if (Keys[DIK_NUMPADSTAR])
            add = 576 * TimeStep;
        if (Keys[DIK_NUMPADSLASH])
            add = -576 * TimeStep;

        if (Keys[DIK_LCONTROL]) add *= 4;

        add = (float)(long)add;
    
        if (add)
        {
            switch (CurrentShadowSide)
            {
                case 0:
                    CurrentEditLight->r += (long)add;
                    VecPlusEqScalarVec((VEC*)&CurrentEditLight->x, add, &CurrentEditLight->DirMatrix.mv[R]);
                    break;
                case 1:
                    CurrentEditLight->r += (long)add;
                    VecPlusEqScalarVec((VEC*)&CurrentEditLight->x, -add, &CurrentEditLight->DirMatrix.mv[R]);
                    break;
                case 2:
                    CurrentEditLight->g += (long)add;
                    VecPlusEqScalarVec((VEC*)&CurrentEditLight->x, add, &CurrentEditLight->DirMatrix.mv[U]);
                    break;
                case 3:
                    CurrentEditLight->g += (long)add;
                    VecPlusEqScalarVec((VEC*)&CurrentEditLight->x, -add, &CurrentEditLight->DirMatrix.mv[U]);
                    break;
                case 4:
                    CurrentEditLight->b += (long)add;
                    VecPlusEqScalarVec((VEC*)&CurrentEditLight->x, add, &CurrentEditLight->DirMatrix.mv[L]);
                    break;
                case 5:
                    CurrentEditLight->b += (long)add;
                    VecPlusEqScalarVec((VEC*)&CurrentEditLight->x, -add, &CurrentEditLight->DirMatrix.mv[L]);
                    break;
            }
        }

        if (CurrentEditLight->r < 16) CurrentEditLight->r = 16;
        if (CurrentEditLight->g < 16) CurrentEditLight->g = 16;
        if (CurrentEditLight->b < 16) CurrentEditLight->b = 16;
    }
}

////////////////////////
// display light info //
////////////////////////

void DisplayLightInfo(LIGHT *light)
{
    WCHAR buf[128];

// type

	swprintf( buf, L"%S", LightNames[light->Type]);
    DumpText(450, 0, 8, 16, 0xffff00, buf);

// flag

	swprintf( buf, L"%S", LightFlags[(light->Flag & (LIGHT_FIXED | LIGHT_MOVING)) - 1]);
    DumpText(450, 24, 8, 16, 0x00ffff, buf );

// rgb

    swprintf(buf, light->Type == LIGHT_SHADOW ? L"Size %ld %ld %ld" : L"RGB %ld %ld %ld", light->r, light->g, light->b);
    DumpText(450, 48, 8, 16, 0x00ff00, buf);

// reach

    swprintf(buf, L"Reach %d", (short)light->Reach);
    DumpText(450, 72, 8, 16, 0xff0000, buf);

// flicker

    swprintf(buf, L"Flicker %S", LightFlickerNames[(light->Flag & LIGHT_FLICKER) != 0]);
    DumpText(450, 96, 8, 16, 0xff00ff, buf);

// flicker speed

    swprintf(buf, L"Flicker Speed %d", light->Speed);
    DumpText(450, 120, 8, 16, 0x0000ff, buf);

// axis

    swprintf(buf, L"Axis %S - %S", LightAxisNames[LightAxis], LightAxisTypeNames[LightAxisType]);
    DumpText(450, 144, 8, 16, 0xffff00, buf);

// cone

    if (light->Type == LIGHT_SPOT || light->Type == LIGHT_SPOTNORMAL)
    {
        swprintf(buf, L"Cone %d", (short)light->Cone);
        DumpText(450, 168, 8, 16, 0x00ffff, buf);
    }
}

////////////////////////////
// load edit light models //
////////////////////////////

void LoadEditLightModels(void)
{
//$MODIFIED
//    LoadModel("edit\\omni.m", &LightOmniModel, -1, 1, LOADMODEL_FORCE_TPAGE, 100);
//    LoadModel("edit\\spot.m", &LightSpotModel, -1, 1, LOADMODEL_FORCE_TPAGE, 100);
    LoadModel("D:\\edit\\omni.m", &LightOmniModel, -1, 1, LOADMODEL_FORCE_TPAGE, 100);
    LoadModel("D:\\edit\\spot.m", &LightSpotModel, -1, 1, LOADMODEL_FORCE_TPAGE, 100);
//$END_MODIFICATIONS
}

////////////////////////////
// free edit light models //
////////////////////////////

void FreeEditLightModels(void)
{
    FreeModel(&LightOmniModel, 1);
    FreeModel(&LightSpotModel, 1);
}

////////////////////////////////////////////////////
// test if a vertex is visible from a light point //
////////////////////////////////////////////////////

// Commented out by JCC when coll.h removed (no more COLL_POLY) 30/6/98

/*char LightVertexVisible(LIGHT *light, VEC *v)
{
    short i;
    COLL_POLY *p;
    float MinX, MaxX, MinY, MaxY, MinZ, MaxZ;
    float ldist, vdist;
    VEC inter;

// build bounding box

    MinX = v->v[X];
    MaxX = v->v[X];
    MinY = v->v[Y];
    MaxY = v->v[Y];
    MinZ = v->v[Z];
    MaxZ = v->v[Z];

    if (light->x < MinX) MinX = light->x;
    if (light->x > MaxX) MaxX = light->x;
    if (light->y < MinY) MinY = light->y;
    if (light->y > MaxY) MaxY = light->y;
    if (light->z < MinZ) MinZ = light->z;
    if (light->z > MaxZ) MaxZ = light->z;

// loop thru all coll polys

    p = CollPtr;

    for (i = 0 ; i < CollNum ; i++, p++)
    {

// skip?

        if (MinX > p->MaxX || MaxX < p->MinX || MinY > p->MaxY || MaxY < p->MinY || MinZ > p->MaxZ || MaxZ < p->MinZ)
            continue;

// test

        vdist = PlaneDist(&p->Plane, v);
        if (abs(vdist) < 1) continue;
        ldist = PlaneDist(&p->Plane, (VEC*)&light->x);

        FindIntersection((VEC*)&light->x, ldist, v, vdist, &inter);

        if (PointInsidePlane(&inter, p))
            return FALSE;
    }

// return OK

    return TRUE;
}*/
