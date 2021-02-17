//-----------------------------------------------------------------------------
// File: wheel.cpp
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "ReVolt.h"
#include "NewColl.h"
#include "Wheel.h"
#include "Geom.h"
#include "Util.h"
#ifndef _PSX
#include "visibox.h"
#endif
#ifdef _PC
#include "Draw.h"
#endif
#ifdef _N64
#include "car.h"
#include "drawobj.h"
#endif
#include "weapon.h"
#include "main.h"

#ifdef _N64
long SKID_MAX_SKIDS = GAME_SKID_MAX_SKIDS;
SKIDMARK    WHL_SkidMark[GAME_SKID_MAX_SKIDS];
#else
SKIDMARK    WHL_SkidMark[SKID_MAX_SKIDS];
#endif

int         WHL_SkidHead = 0;
int         WHL_NSkids = 0;


/////////////////////////////////////////////////////////////////////
//
// AddSkid: add a skid to the array for drawing
//
/////////////////////////////////////////////////////////////////////

SKIDMARK *AddSkid(SKIDMARK_START *skidStart, SKIDMARK_START *skidEnd, long rgb)
{
    VEC         right;
    SKIDMARK        *skid;
    BOUNDING_BOX    box;

    skid = &WHL_SkidMark[WHL_SkidHead++];
    Wrap(WHL_SkidHead, 0, SKID_MAX_SKIDS);
    WHL_NSkids++;
    Limit(WHL_NSkids, 0, SKID_MAX_SKIDS);

    // Calculate the position of the four corners
    VecCrossVec(&skidStart->Dir, &skidStart->Normal, &right);
    VecPlusScalarVec(&skidStart->Pos, skidStart->Width, &right, &skid->Corner[0]);
    VecPlusScalarVec(&skidStart->Pos, -skidStart->Width, &right, &skid->Corner[1]);

    // Set colour
#ifdef _PC
    skid->RGB[0] = skid->RGB[1] = rgb;
#endif
#ifdef _N64
    skid->RGB = (rgb << 8) | (rgb & 0xFF);
#endif
#ifdef _PSX
    skid->RGB = rgb;
#endif

    MoveSkidEnd(skid, skidEnd, rgb);
    //VecCrossVec(&skidEnd->Dir, &skidEnd->Normal, &right);
    //VecPlusScalarVec(&skidEnd->Pos, -skidEnd->Width, &right, &skid->Corner[2]);
    //VecPlusScalarVec(&skidEnd->Pos, skidEnd->Width, &right, &skid->Corner[3]);

    // set visibox mask
    box.Xmin = box.Xmax = skidStart->Pos.v[X];
    box.Ymin = box.Ymax = skidStart->Pos.v[Y];
    box.Zmin = box.Zmax = skidStart->Pos.v[Z];

#ifndef _PSX
    skid->VisiMask = SetObjectVisiMask(&box);

    // Fade the old SkidMarks
    FadeSkidMarks();
#endif

    return skid;
}


/////////////////////////////////////////////////////////////////////
//
// MoveSkidEnd: move the endpoint of the skid
//
/////////////////////////////////////////////////////////////////////

void MoveSkidEnd(SKIDMARK *skid, SKIDMARK_START *skidEnd, long rgb)
{
    VEC right;

    VecCrossVec(&skidEnd->Dir, &skidEnd->Normal, &right);
    VecPlusScalarVec(&skidEnd->Pos, -skidEnd->Width, &right, &skid->Corner[2]);
    VecPlusScalarVec(&skidEnd->Pos, skidEnd->Width, &right, &skid->Corner[3]);

    skid->Centre.v[X] = (skid->Corner[0].v[X] + skid->Corner[2].v[X]) / 2;
    skid->Centre.v[Y] = (skid->Corner[0].v[Y] + skid->Corner[2].v[Y]) / 2;
    skid->Centre.v[Z] = (skid->Corner[0].v[Z] + skid->Corner[2].v[Z]) / 2;

#ifdef _PC
    skid->RGB[2] = skid->RGB[3] = rgb;//skidEnd->Material->SkidColour;
#endif
}


////////////////////////////////////////////////////////////////
//
// TerminateSkid:
//
////////////////////////////////////////////////////////////////

void TerminateSkid(SKIDMARK *skid)
{
#ifdef _PC
    skid->RGB[2] = skid->RGB[3] = 0;
#endif
}

/////////////////////////////////////////////////////////////////////
//
// FadeSkidMarks: fade the old skidmarks
//
/////////////////////////////////////////////////////////////////////

#ifndef _PSX
void FadeSkidMarks()
{
    int ii;
    int iSkid, currentSkid;
    int nFade;
    unsigned char r, g, b;
    SKIDMARK *skid;

    if (WHL_NSkids < SKID_FADE_START) return;

    nFade = WHL_NSkids - SKID_FADE_START;

    currentSkid = WHL_SkidHead - WHL_NSkids + 1;
    for (iSkid = 0; iSkid < nFade; iSkid++)
    {
        Wrap(currentSkid, 0, SKID_MAX_SKIDS);
        skid = &WHL_SkidMark[currentSkid++];

        // Calculate new RGB
#ifdef _PC
        for (ii = 0; ii < 4; ii++) {
            r = (unsigned char)((skid->RGB[ii] & RGB_RED_MASK) >> 16);
            g = (unsigned char)((skid->RGB[ii] & RGB_GREEN_MASK) >> 8);
            b = (unsigned char)(skid->RGB[ii] & RGB_BLUE_MASK);

            if (r > SKID_FADE_FACTOR) {
                r -= SKID_FADE_FACTOR;
            } else {
                r = 0;
            }
            if (g > SKID_FADE_FACTOR) {
                g -= SKID_FADE_FACTOR;
            } else {
                g = 0;
            }
            if (b > SKID_FADE_FACTOR) {
                b -= SKID_FADE_FACTOR;
            } else {
                b = 0;
            }

            skid->RGB[ii] = (r << 16) | (g << 8) | (b);
        }
#endif
#ifdef _N64
        r = skid->RGB & 0xFF;
        if (r > SKID_FADE_FACTOR) {
            r -= SKID_FADE_FACTOR;
        } else {
            r = 0;
        }
        skid->RGB = (skid->RGB & 0xFFFFFF00) | (unsigned char)r;
#endif
    }
}


#endif

/////////////////////////////////////////////////////////////////////
//
// ClearSkids: remove all skids from list
//
/////////////////////////////////////////////////////////////////////

void ClearSkids()
{
    WHL_NSkids = 0;
    WHL_SkidHead = 0;
}

/////////////////////////////////////////////////////////////////////
//
// InitWheel: set the wheels mass, radius, inertia...
//
/////////////////////////////////////////////////////////////////////

void SetupWheel(WHEEL *wheel, WHEEL_INFO *wheelInfo)
{
    // Status
    wheel->Status = 0;
    if (wheelInfo->IsPresent) {
        SetWheelPresent(wheel);
    }
    if (wheelInfo->IsTurnable) {
        SetWheelTurnable(wheel);
    }
    if (wheelInfo->IsPowered) {
        SetWheelPowered(wheel);
    }

    // Mass
    wheel->Mass = wheelInfo->Mass;
    wheel->InvMass = DivScalar(ONE, wheelInfo->Mass);

    // Radius
    wheel->Radius = wheelInfo->Radius;

    // Inertia
    wheel->Inertia = MulScalar(wheelInfo->Mass, MulScalar(wheelInfo->Radius, wheelInfo->Radius)) / 2;
    wheel->InvInertia = DivScalar(ONE, wheel->Inertia);
    
    // Gravity
    wheel->Gravity = wheelInfo->Gravity;

    // Friction
    wheel->Grip = wheelInfo->Grip;
    wheel->defaultStaticFriction = wheel->StaticFriction = wheelInfo->StaticFriction;
    wheel->defaultKineticFriction = wheel->KineticFriction = wheelInfo->KineticFriction;
    wheel->AxleFriction = wheelInfo->AxleFriction;

    // Steering ratio
    wheel->SteerRatio = wheelInfo->SteerRatio;

    // Engine Ratio
#ifndef _PSX
    wheel->EngineRatio = wheelInfo->EngineRatio;
#else
    wheel->EngineRatio = DivScalar(wheelInfo->EngineRatio, wheel->Radius);
#endif


    // Bounding box
#ifndef _PSX
    SetBBox(&wheel->BBox, 
        -wheelInfo->Radius, wheelInfo->Radius, 
        -wheelInfo->Radius, wheelInfo->Radius, 
        -wheelInfo->Radius, wheelInfo->Radius);
    wheel->BBRadius = wheelInfo->Radius;
#endif

    // Max displacement
    wheel->MaxPos = wheelInfo->MaxPos;

    // Skid Width
    wheel->SkidWidth = wheelInfo->SkidWidth;

    // Skid Material
    wheel->SkidMaterial = MATERIAL_NONE;

    // Oil time
    wheel->OilTime = OILY_WHEEL_TIME;

    // Spinning impulse
    wheel->SpinAngImp = MulScalar(MulScalar(wheel->Mass, TO_ACC(Real(15000))), wheel->Radius);

    // No skid time
    wheel->Skid.NoSkidTime = ZERO;

    // Toein
    wheel->ToeIn = 0;//wheelInfo->ToeIn;

    // Some initialisation stuff to stop people moaning about back wheels being turned when they change cars which
    // they won't be able to do the final game anyway. Moaning gits.
    wheel->TurnAngle = ZERO;

}


/////////////////////////////////////////////////////////////////////
//
// SetupSuspension:
//
/////////////////////////////////////////////////////////////////////

void SetupSuspension(SPRING *spring, SPRING_INFO *info)
{
#ifdef _N64
    spring->Stiffness = info->Stiffness * 0.8f;
#else
    spring->Stiffness = info->Stiffness;
#endif
    spring->Damping = info->Damping;
    spring->Restitution = info->Restitution;

}


/////////////////////////////////////////////////////////////////////
//
// SpringDampedForce: return the force on a spring for the 
// passed extension and velocity
//
/////////////////////////////////////////////////////////////////////

REAL SpringDampedForce(SPRING *spring, REAL extension, REAL velocity) 
{
    REAL force;

#if defined(_PC)
    force = - MulScalar(spring->Stiffness, extension) - MulScalar(spring->Damping, velocity);
#elif defined(_N64)
    force = - MulScalar(spring->Stiffness, extension) * 0.8f - MulScalar(spring->Damping, velocity);
#elif defined(_PSX)
    //force = - (MulScalar(spring->Stiffness, extension << 4) >> 5) - MulScalar(spring->Damping, velocity);
    force = - MulScalar(spring->Stiffness, extension) - MulScalar(spring->Damping, velocity);
#endif

    if (Sign(force) == Sign(extension)) {
        force = ZERO;
    }

    return force;
}
