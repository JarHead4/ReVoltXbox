//-----------------------------------------------------------------------------
// File: particle.cpp
//
// Desc: Describes the state and physics controlling a point-mass particle
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"
#include "particle.h"
#include "geom.h"
#include "util.h"
#include "NewColl.h"
#include "main.h"

void ParticleWorldColls(PARTICLE *particle);

/////////////////////////////////////////////////////////////////////
//
// SetMass: change the mass of a particle, and set the inverse mass
//
// inputs:
//      particle    - a pointer to the particle in question
//      newMass     - the new mass
/////////////////////////////////////////////////////////////////////

void SetParticleMass(PARTICLE *particle, REAL newMass)
{
    particle->Mass = newMass;
    particle->InvMass = DivScalar(ONE, newMass);
}


/////////////////////////////////////////////////////////////////////
//
// ApplyParticleImpulse: apply an impulse to the particle
//
// inputs:
//      particle    - the particle to act on
//      impulse     - the impulse to apply
/////////////////////////////////////////////////////////////////////

void ApplyParticleImpulse(PARTICLE *particle, VEC *impulse)
{
    VecPlusEqVec(&particle->Impulse, impulse);
}


/////////////////////////////////////////////////////////////////////
//
// UpdateParticle: update the position of a particle according
// to the impulses acting on it
//
// inputs:
//      particle    - the particle to act on
//      dt          - the timestep
/////////////////////////////////////////////////////////////////////


void UpdateParticle(PARTICLE *particle, REAL dt)
{
    VEC oldVel;
    REAL    tReal;

    // Shift body out of contacts
    VecPlusEqVec(&particle->Pos, &particle->Shift);
    SetVecZero(&particle->Shift);

    // Store the old position and world matrices
    CopyVec(&particle->Pos, &particle->OldPos);
    CopyVec(&particle->Vel, &oldVel);

    // Update particle velocity from the acceleration
    VecPlusEqScalarVec(&particle->Vel, particle->InvMass, &particle->Impulse);

    // Damp the velocity from the air resistance
    tReal = MulScalar(particle->Resistance, MulScalar(FRICTION_TIME_SCALE, dt));
    VecMulScalar(&particle->Vel, ONE - tReal);

    // Update particle position from the velocity
    VecPlusEqScalarVec(&particle->Pos, dt, &particle->Vel);

    // Reset the impulse for the next timestep
    SetVecZero(&particle->Impulse);

    // Recalculate acceleration from change in velocity
    VecMinusVec(&particle->Vel, &oldVel, &particle->Acc);

    // Catch stuff that has fallen off the world...
#ifndef _N64
    Assert(particle->Pos.v[Y] < TO_LENGTH(Real(30000)));
#endif

}


/////////////////////////////////////////////////////////////////////
//
// ParticleWorldColls:
//
/////////////////////////////////////////////////////////////////////

void ParticleWorldColls(PARTICLE *particle)
{
    int iPoly;
    REAL time, depth, velDotNorm;
    VEC dPos, wPos;
    BBOX bBox;
    NEWCOLLPOLY *collPoly;
    COLLGRID *grid;

    // Get the current collision grid
    grid = PosToCollGrid(&particle->Pos);
    if (grid == NULL) return;

    // loop over all polys in the grid
    for (iPoly = 0; iPoly < grid->NCollPolys; iPoly++) {
        collPoly = GetCollPoly(grid->CollPolyIndices[iPoly]);

        if (PolyCameraOnly(collPoly)) continue;

        // Quick bounding-box test
        SetBBox(&bBox, 
            Min(particle->Pos.v[X], particle->OldPos.v[X]),
            Max(particle->Pos.v[X], particle->OldPos.v[X]),
            Min(particle->Pos.v[Y], particle->OldPos.v[Y]),
            Max(particle->Pos.v[Y], particle->OldPos.v[Y]),
            Min(particle->Pos.v[Z], particle->OldPos.v[Z]),
            Max(particle->Pos.v[Z], particle->OldPos.v[Z]));
        if(!BBTestYXZ(&bBox, &collPoly->BBox)) continue;

        // Check for point passing through collision polygon
        if (!LinePlaneIntersect(&particle->OldPos, &particle->Pos, &collPoly->Plane, &time, &depth)) {
            continue;
        }

        // Calculate the intersection point
        VecMinusVec(&particle->Pos, &particle->OldPos, &dPos);
        VecPlusScalarVec(&particle->OldPos, time, &dPos, &wPos);

        // Make sure the particle is travelling towards the poly
        velDotNorm = VecDotVec(&particle->Vel, PlaneNormal(&collPoly->Plane));
        if (velDotNorm > ZERO) continue;


        // Check intersection point is within the polygon boundary
        if (!PointInCollPolyBounds(&wPos, collPoly)) {
            continue;
        }

        // Keep particle on inside of the poly
        VecPlusEqScalarVec(&particle->Pos, -depth + COLL_EPSILON, PlaneNormal(&collPoly->Plane));

        // Rebound
        VecPlusEqScalarVec(&particle->Vel, -velDotNorm, PlaneNormal(&collPoly->Plane));
        VecMulScalar(&particle->Vel, (ONE - particle->KineticFriction));
        VecPlusEqScalarVec(&particle->Vel, -(particle->Hardness * velDotNorm), PlaneNormal(&collPoly->Plane));


    }
}


