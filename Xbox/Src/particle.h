//-----------------------------------------------------------------------------
// File: particle.h
//
// Desc: Describes the state and physics controlling a point-mass particle
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef PARTICLE_H
#define PARTICLE_H

#ifdef _PSX
#include <sys\types.h>
#include <libgte.h>
#endif

typedef struct ParticleStruct {
    REAL    Mass, InvMass;
    VEC OldPos;             // previous world position
    VEC Pos;                // current world position
    VEC Vel;                // velocity
    VEC Acc;                // acceleration
    VEC Impulse;            // net impulse


    QUATERNION  Quat;           // Current orientation quaternion
    //QUATERNION    OldQuat;        // previous quaternion

    MAT WMatrix;            // current orientation matrix
    MAT OldWMatrix;         // previous orientation matrix

    REAL    Hardness;           // coefficient of restitution
    REAL    Resistance;         // "air" resistance
    REAL    Grip;
    REAL    StaticFriction;
    REAL    KineticFriction;
    REAL    Gravity;
    REAL    Boost;

    VEC Shift;              // Used to make sure objects aren't intersecting at end of frame

    // Stuff to remove jitter
    VEC LastVel;
//  bool    IsJitteringx;       // Whether body is jittering
//  bool    IsJitteringy;       // Whether body is jittering
//  bool    IsJitteringz;       // Whether body is jittering
//  bool    IsJittering;        // Whether body is jittering
//  int     JitterCount;        // Number of times the object has "jittered"
//  int     JitterCountMax;     // Max number of jitters allowed
//  int     JitterFrames;       // Number of frames between last and next jitter
//  int     JitterFramesMax;    // Max number of frames between jitters for it to count as a jitter

} PARTICLE;


extern void SetParticleMass(PARTICLE *particle, REAL newMass);
extern void ApplyParticleImpulse(PARTICLE *particle, VEC *impulse);
extern void UpdateParticle(PARTICLE *particle, REAL dt);
extern void ParticleWorldColls(PARTICLE *particle);


#endif // PARTICLE_H

