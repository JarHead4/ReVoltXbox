//-----------------------------------------------------------------------------
// File: units.h
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef UNITS_H
#define UNITS_H

/////////////////////////////////////////////////////////////////////
//
// Unit conversion stuff
//
/////////////////////////////////////////////////////////////////////


#if defined(_PSX) || defined(_RCONV)

// Playstation rescaling due to shittiness of not having a faggoting MPU
#define SCALE(x, scale)     (((scale) < 0)? (x) >> -(scale): (x) << (scale))

#define PSX_LENGTH_SHIFT    (-4)
#define PSX_TIME_SHIFT      (7)
#define PSX_MASS_SHIFT      (0)

#define SMALL_SHIFT         4

#define TO_LENGTH(x)        SCALE((x), PSX_LENGTH_SHIFT)
#define TO_TIME(x)          SCALE((x), PSX_TIME_SHIFT)
#define TO_MASS(x)          SCALE((x), PSX_MASS_SHIFT)

#define TO_FREQ(x)          SCALE((x), -PSX_TIME_SHIFT)
#define TO_VEL(x)           SCALE((x), PSX_LENGTH_SHIFT - PSX_TIME_SHIFT)
#define TO_ACC(x)           SCALE((x), PSX_LENGTH_SHIFT - PSX_TIME_SHIFT - PSX_TIME_SHIFT)
#define TO_FORCE(x)         SCALE((x), PSX_MASS_SHIFT + PSX_LENGTH_SHIFT - PSX_TIME_SHIFT - PSX_TIME_SHIFT)
#define TO_TORQUE(x)        SCALE((x), PSX_MASS_SHIFT + PSX_LENGTH_SHIFT + PSX_LENGTH_SHIFT - PSX_TIME_SHIFT - PSX_TIME_SHIFT)
#define TO_IMP(x)           SCALE((x), PSX_MASS_SHIFT + PSX_LENGTH_SHIFT - PSX_TIME_SHIFT)
#define TO_INERTIA(x)       SCALE((x), PSX_MASS_SHIFT + PSX_LENGTH_SHIFT + PSX_LENGTH_SHIFT)
#define TO_ANGVEL(x)        SCALE((x), -PSX_TIME_SHIFT)
#define TO_ANGACC(x)        SCALE((x), -PSX_TIME_SHIFT - PSX_TIME_SHIFT)
#define TO_GRIP(x)          SCALE((x), -PSX_LENGTH_SHIFT + PSX_TIME_SHIFT)
#define TO_STIFFNESS(x)     SCALE((x), PSX_MASS_SHIFT - PSX_TIME_SHIFT - PSX_TIME_SHIFT)
#define TO_DAMPING(x)       SCALE((x), PSX_MASS_SHIFT - PSX_TIME_SHIFT)
#define TO_AXLEFRICTION(x)  SCALE((x), PSX_MASS_SHIFT + PSX_LENGTH_SHIFT)

#define FROM_LENGTH(x)      SCALE((x), -PSX_LENGTH_SHIFT)
#define FROM_TIME(x)        SCALE((x), -PSX_TIME_SHIFT)
#define FROM_MASS(x)        SCALE((x), -PSX_MASS_SHIFT)

#define FROM_FREQ(x)        SCALE((x), PSX_TIME_SHIFT)
#define FROM_VEL(x)         SCALE((x), -PSX_LENGTH_SHIFT + PSX_TIME_SHIFT)
#define FROM_ACC(x)         SCALE((x), -PSX_LENGTH_SHIFT + PSX_TIME_SHIFT + PSX_TIME_SHIFT)
#define FROM_FORCE(x)       SCALE((x), -PSX_MASS_SHIFT + PSX_LENGTH_SHIFT + PSX_TIME_SHIFT + PSX_TIME_SHIFT)
#define FROM_TORQUE(x)      SCALE((x), -PSX_MASS_SHIFT - PSX_LENGTH_SHIFT - PSX_LENGTH_SHIFT + PSX_TIME_SHIFT + PSX_TIME_SHIFT)
#define FROM_IMP(x)         SCALE((x), -PSX_MASS_SHIFT - PSX_LENGTH_SHIFT + PSX_TIME_SHIFT)
#define FROM_INERTIA(x)     SCALE((x), -PSX_MASS_SHIFT - PSX_LENGTH_SHIFT - PSX_LENGTH_SHIFT)
#define FROM_ANGVEL(x)      SCALE((x), PSX_TIME_SHIFT)
#define FROM_ANGACC(x)      SCALE((x), PSX_TIME_SHIFT + PSX_TIME_SHIFT)
#define FROM_GRIP(x)        SCALE((x), PSX_LENGTH_SHIFT - PSX_TIME_SHIFT)
#define FROM_STIFFNESS(x)   SCALE((x), -PSX_MASS_SHIFT + PSX_TIME_SHIFT + PSX_TIME_SHIFT)
#define FROM_DAMPING(x)     SCALE((x), -PSX_MASS_SHIFT + PSX_TIME_SHIFT)

#define PSX_LENGTH(x)       SCALE((x), -PSX_LENGTH_SHIFT - 16)
#define PSX_MAT(x)          SCALE((x), -4)

#define OGU2MPH_SPEED   (TO_VEL(Real(0.01118)))
#define MPH2OGU_SPEED   (TO_VEL(Real(89.4454)))
#define OGU2FPM_SPEED   (TO_VEL(Real(0.1016)))
#define FPM2OGU_SPEED   (TO_VEL(Real(9.84252)))
#define OGU2KPH_SPEED   (TO_VEL(Real(0.018)))
#define KPH2OGU_SPEED   (TO_VEL(Real(55.5555)))

#else // _PSX

#define OGU2MPH_SPEED   (Real(0.01118))
#define MPH2OGU_SPEED   (ONE / OGU2MPH_SPEED)
#define OGU2FPM_SPEED   (Real(0.1016))
#define FPI2OGU_SPEED   (ONE / OGU2FPM_SPEED)
#define OGU2KPH_SPEED   (Real(0.018))
#define KPH2OGU_SPEED   (ONE / OGU2KPH_SPEED)

// Lovely N64 anc PC non-scaling
#define TO_LENGTH(x)        (x)
#define TO_TIME(x)          (x)
#define TO_MASS(x)          (x)

#define TO_FREQ(x)          (x)
#define TO_VEL(x)           (x)
#define TO_ACC(x)           (x)
#define TO_FORCE(x)         (x)
#define TO_TORQUE(x)        (x)
#define TO_IMP(x)           (x)
#define TO_INERTIA(x)       (x)
#define TO_ANGVEL(x)        (x)
#define TO_ANGACC(x)        (x)
#define TO_GRIP(x)          (x)
#define TO_STIFFNESS(x)     (x)
#define TO_DAMPING(x)       (x)

#define FROM_LENGTH(x)      (x)
#define FROM_TIME(x)        (x)
#define FROM_MASS(x)        (x)

#define FROM_FREQ(x)        (x)
#define FROM_VEL(x)         (x)
#define FROM_ACC(x)         (x)
#define FROM_FORCE(x)       (x)
#define FROM_IMP(x)         (x)
#define FROM_INERTIA(x)     (x)
#define FROM_ANGVEL(x)      (x)
#define FROM_ANGACC(x)      (x)
#define FROM_GRIP(x)        (x)
#define FROM_STIFFNESS(x)   (x)
#define FROM_DAMPING(x)     (x)

#endif


/////////////////////////////////////////////////////////////////////
//
// Vector and Matrix conversion stuff
//
/////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////
// Pi and related constants
/////////////////////////////////////////////////////////////////////


#define PI Real(3.141592654f)
#define RAD (PI * 2)
#define FULL_CIRCLE (PI * 2)
#define RAD2DEG (360 / RAD)
#define DEG2RAD (RAD / 360)
#define FRICTION_TIME_SCALE     FROM_TIME(Real(120))


/////////////////////////////////////////////////////////////////////
// Other misc stuff
/////////////////////////////////////////////////////////////////////




#endif // UNITS_H

