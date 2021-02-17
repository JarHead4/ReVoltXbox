//-----------------------------------------------------------------------------
// File: intro.h
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef INTRO_H
#define INTRO_H

////////////////////////////////////////////////////////////////
//
// State and state variables for the intro sequence
//
////////////////////////////////////////////////////////////////

typedef enum IntroStateEnum {
    INTRO_ACLM_INITIAL_PAUSE,
    INTRO_ACLM_INITIAL_UFO_FLIGHT,
    INTRO_ACLM_EXIT_UFO,
    INTRO_ACLM_FINAL_PAUSE,
    INTRO_PROBE_INITIAL_WAIT,
    INTRO_PROBE_INITIAL_UFO_FLIGHT,
    INTRO_PROBE_DROP_LOGO,
    INTRO_PROBE_CAR_DRIVE_BY,
    INTRO_EXIT,

    INTRO_NSTATES
} INTRO_STATE;


typedef struct IntroStateVarsStruct {
    REAL    Timer;
} INTRO_STATE_VARS;

typedef void (INTRO_STATE_FUNCTION)(void);

enum {
    INTRO_CAM_ACLM_START,
    INTRO_CAM_ACLM_END,
    INTRO_CAM_PROBE_START,
    INTRO_CAM_PROBE_END,

    INTRO_CAMPOS_NUM
};


////////////////////////////////////////////////////////////////
//
// Data structure for the coloured stripe info
//
////////////////////////////////////////////////////////////////

typedef struct StripeDataStruct {
    long NSections;
    VEC *Pos;
    long *RGB;
} STRIPE_DATA;


////////////////////////////////////////////////////////////////
//
// intro players
//
////////////////////////////////////////////////////////////////

enum IntroPlayerEnum {
    INTRO_PLAYER_UFO,
    INTRO_PLAYER_CAR1,
    INTRO_PLAYER_CAR2,

    INTRO_NPLAYERS
};

////////////////////////////////////////////////////////////////
//
// Smoke-screen stuff
//
////////////////////////////////////////////////////////////////

// Smoke particle
typedef struct s_SmokeParticle
{
    VEC     pos;                                // Position
    VEC     vel;                                // Velocity
    VEC     acc;                                // Acceleration
    VEC     impulse;                            // Impulses

    REAL    intensity;                          // RGB intensity

    REAL    mass, invMass;
    REAL    gravity;                            // Gravity
    REAL    airResistance;                      // Air resistance

} t_SmokeParticle;

// Smoke screen
typedef struct s_SmokeScreen
{
    int             width, height;              // Width & height of screen (in particles)
    int             cParticles;                 // Particle #
    t_SmokeParticle *pParticles;                // Particles

    REAL            lengthMin;                  // Min. Length of particle
    REAL            lengthMinD;
    REAL            damping;

    REAL            expand;                     // Mesh expansion speed

} t_SmokeScreen;


////////////////////////////////////////////////////////////////
//
// External function prototypes
//
////////////////////////////////////////////////////////////////

extern void GoIntroSequence(void);


#endif // INTRO_H

