//-----------------------------------------------------------------------------
// File: Aerial.cpp
//
// Desc: Wobbly aerial implementation
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"
#include "aerial.h"
#include "geom.h"

/////////////////////////////////////////////////////////////////////
//
// CreateAerial:    create an aerial with the specified number of section
//
/////////////////////////////////////////////////////////////////////
#ifdef _PC
AERIAL *CreateAerial(void)
{
    // Allocate space for the aerial structure
    return (AERIAL *)malloc(sizeof(AERIAL));
}
#endif

/////////////////////////////////////////////////////////////////////
//
// DestroyAerial: deallocate the space created for the aerial object
//
/////////////////////////////////////////////////////////////////////
#ifdef _PC
void DestroyAerial(AERIAL *aerial)
{
    free(aerial);
}
#endif


/////////////////////////////////////////////////////////////////////
//
// SetAerialSprings: setup the aerial's springs
//
/////////////////////////////////////////////////////////////////////

void SetAerialSprings(AERIAL *aerial, REAL stiffness, REAL damping, REAL antiGrav)
{
    aerial->Stiffness = stiffness;
    aerial->Damping = damping;
    aerial->AntiGrav = antiGrav;
}


/////////////////////////////////////////////////////////////////////
//
// InitCarAerial: initialise the aerial data structure
//
// inputs:
//      car         - the car whose aerial is to be initalised
//      pos         - the initial position of the aerial base 
//                      in world coords
//      direction   - the direction in which the aerial points
//      secLength   - the length of the aerial sections
//
//  note:
//      initialises ALL aerial sections (not just the ones used)
/////////////////////////////////////////////////////////////////////

void InitAerial
    (AERIAL *aerial, 
    VEC *direction, 
    REAL secLength, 
    REAL mass, 
    REAL hardness, 
    REAL resistance,
    REAL gravity)
{
    int iSec;
    PARTICLE *pSection;

    aerial->Length = secLength;

    CopyVec(direction, &aerial->Direction);

    // Set the mass, hardness and resistance
    for (iSec = 0; iSec < AERIAL_NSECTIONS; iSec++) {
        // Store a pointer to the current section (avoid excessive dereferencing)
        pSection = &aerial->Section[iSec];

        SetParticleMass(pSection, mass);

        pSection->Hardness = hardness;
        pSection->Resistance = resistance;
        pSection->Gravity = gravity;
    }

}


