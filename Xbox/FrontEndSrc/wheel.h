//-----------------------------------------------------------------------------
// File: wheel.h
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef WHEEL_H
#define WHEEL_H

/////////////////////////////////////////////////////////////////////
// Skid mark stuff
/////////////////////////////////////////////////////////////////////
typedef struct {
    VEC         Corner[4];
    VEC         Centre;
#ifdef _PC
    long            RGB[4];
#else
    long            RGB;
#endif
    VISIMASK        VisiMask;
} SKIDMARK;


typedef struct {
    long    Started;
    REAL    LastSmokeTime;

    VEC     Pos;
    VEC     Dir;
    VEC     Normal;
    REAL    Width;
    REAL    NoSkidTime;

    MATERIAL *Material;
    SKIDMARK *CurrentSkid;
} SKIDMARK_START;

#define SKID_MAX_SKIDS  (1000)              // Max number of skids allowed

#define SKID_MAX_LEN    TO_LENGTH(Real(128))            // Max length of a skidmark
#define SKID_HALF_LEN   TO_LENGTH(Real(64))         // Half the max length
#define SKID_MIN_LEN    TO_LENGTH(Real(10))         // Min length of a skidmark
#define SKID_MAX_DOT    (Real(0.98))        // Dot product of skid direction cannot change below this
#define SKID_RAISE      TO_LENGTH(Real(2.0))            // Distance above floor of skidmarks
#define SKID_FADE_NUM   (100)               // number of skids to fade out
#define SKID_FADE_START (SKID_MAX_SKIDS - SKID_FADE_NUM)
#define SKID_FADE_FACTOR (2)                // Amount to reduce fading skids alpha by
#ifdef _PC
#define SKID_SMOKE_TIME (Real(0.03))        // Amount of time between smoke generations
#endif
#ifdef _N64
#define SKID_SMOKE_TIME (Real(0.01))        // Amount of time between smoke generations
#endif

/////////////////////////////////////////////////////////////////////
// Spring Stuff
/////////////////////////////////////////////////////////////////////
typedef struct {
    REAL    Stiffness;
    REAL    Damping;
    REAL    Restitution;
} SPRING;

#define SpringForce(spring, extension) \
    ( - (spring)->Stiffness * (extension) )


/////////////////////////////////////////////////////////////////////
// WheelInfo: structure to hold initialisation data for a wheel
/////////////////////////////////////////////////////////////////////
typedef struct {
    long    ModelNum;
    VEC Offset1;
    VEC Offset2;
    REAL    Radius;
    REAL    Mass;
    REAL    Gravity;
    REAL    Grip;
    REAL    StaticFriction;
    REAL    KineticFriction;
    REAL    AxleFriction;
    REAL    SteerRatio;
    REAL    EngineRatio;
    REAL    MaxPos;
    REAL    SkidWidth;
    REAL    ToeIn;

    bool    IsPresent;
    bool    IsTurnable;
    bool    IsPowered;

} WHEEL_INFO;

/////////////////////////////////////////////////////////////////////
// SPRING_INFO: hold data for a spring with dampers
/////////////////////////////////////////////////////////////////////
typedef struct {
    long    ModelNum;
    VEC Offset;
    REAL    Length;
    REAL    Stiffness;
    REAL    Damping;
    REAL    Restitution;
} SPRING_INFO;

/////////////////////////////////////////////////////////////////////
// Wheel Stuff
/////////////////////////////////////////////////////////////////////
typedef struct {

    unsigned long Status;   // Status flags

    REAL    Mass, InvMass;
    REAL    Inertia, InvInertia;
    REAL    Radius;
    REAL    Gravity;

    REAL    Grip;
    REAL    StaticFriction;
    REAL    KineticFriction;
    REAL    defaultStaticFriction;
    REAL    defaultKineticFriction;

    REAL    SteerRatio;     // To get turn angle from car's steering wheel angle
    REAL    EngineRatio;    // To get the torque from the engine voltage
    REAL    AxleFriction;   // Guess

#ifndef _PSX
    BBOX    BBox;           // Bounding box
    REAL    BBRadius;       // Bounding sphere radius
#endif

    REAL    Pos;            // Position relative to axle
    REAL    AngPos;         // Rotational angle of the wheel 0 < AngPos < 2 PI
    REAL    Vel;            // Velocity along suspension axis (car's up vector)
    REAL    AngVel;         // Rotational Velocity
    REAL    Acc;            // Linear acceleration along axis
    REAL    AngAcc;         // Rotational acceleration
    REAL    Impulse;        // Linear impulse along axis
    REAL    AngImpulse;     // Rotational impulse

    REAL    MaxPos;         // Maximum offset allowed for wheels
    REAL    MaxAngVel;      // To reduce torque

    REAL    SpinAngImp;     // Angular impulse applied to wheel when it is spinning

#ifndef _PSX
    MAT WMatrix;        // World matrix
    VEC WPos;           // World position of fix point
    VEC OldWPos;
#endif
    VEC CentrePos;      // World position of wheel centre
    VEC OldCentrePos;   
    MAT Axes;           // 

//  MAT EyeMatrix;      // 
//  VEC EyeTrans;       //

    REAL    TurnAngle;      // Angle of wheel in car's look-right plane

    SKIDMARK_START  Skid;   // Skid mark creation data
    REAL    SkidWidth;      // Max width of the skid mark

    REAL    ToeIn;          // The amount of toe-in for this wheel

    REAL    OilTime;        // Time that wheel has been in oil

//#ifndef _PSX
    long    SkidMaterial;   // Material that wheel is skidding on
//#endif

} WHEEL;

#define WHEEL_PRESENT       (1)
#define WHEEL_STEERED       (2)
#define WHEEL_POWERED       (4)
#define WHEEL_UNUSED        (8)
#define WHEEL_SPIN          (16)
#define WHEEL_SLIDE         (32)
#define WHEEL_LOCKED        (64)
#define WHEEL_OIL           (128)
#define WHEEL_CONTACT_FLOOR (256)
#define WHEEL_CONTACT_WALL  (512)
#define WHEEL_CONTACT_SIDE  (1024)
#define WHEEL_CONTACT_OTHER (2048)
#define WHEEL_SKID      (WHEEL_SPIN | WHEEL_SLIDE)


#define IsWheelPresent(wheel) ((wheel)->Status & WHEEL_PRESENT)
#define IsWheelTurnable(wheel) ((wheel)->Status & WHEEL_STEERED)
#define IsWheelPowered(wheel) ((wheel)->Status & WHEEL_POWERED)

#define IsWheelInContact(wheel) ((wheel)->Status & (WHEEL_CONTACT_FLOOR | WHEEL_CONTACT_WALL | WHEEL_CONTACT_SIDE))
#define IsWheelInFloorContact(wheel) ((wheel)->Status & WHEEL_CONTACT_FLOOR)
#define IsWheelInWallContact(wheel) ((wheel)->Status & WHEEL_CONTACT_WALL)
#define IsWheelInSideContact(wheel) ((wheel)->Status & WHEEL_CONTACT_SIDE)
#define IsWheelInOtherContact(wheel) ((wheel)->Status & WHEEL_CONTACT_OTHER)

#define IsWheelSpinning(wheel) ((wheel)->Status & WHEEL_SPIN)
#define IsWheelSliding(wheel) ((wheel)->Status & WHEEL_SLIDE)
#define IsWheelSkidding(wheel) ((wheel)->Status & WHEEL_SKID)
#define IsWheelLocked(wheel) ((wheel)->Status & WHEEL_LOCKED)
#define IsWheelinOil(wheel) ((wheel)->Status & WHEEL_OIL)

#define SetWheelPresent(wheel) {(wheel)->Status |= WHEEL_PRESENT;}
#define SetWheelTurnable(wheel) {(wheel)->Status |= WHEEL_STEERED;}
#define SetWheelPowered(wheel) {(wheel)->Status |= WHEEL_POWERED;}
//#define SetWheelInContact(wheel) {(wheel)->Status |= WHEEL_CONTACT;}

#define SetWheelInFloorContact(wheel) {(wheel)->Status |= WHEEL_CONTACT_FLOOR;}
#define SetWheelInWallContact(wheel) {(wheel)->Status |= WHEEL_CONTACT_WALL;}
#define SetWheelInSideContact(wheel) {(wheel)->Status |= WHEEL_CONTACT_SIDE;}
#define SetWheelInOtherContact(wheel) {(wheel)->Status |= WHEEL_CONTACT_OTHER;}

#define SetWheelSpinning(wheel) {(wheel)->Status |= WHEEL_SPIN;}
#define SetWheelSliding(wheel) {(wheel)->Status |= WHEEL_SLIDE;}
#define SetWheelLocked(wheel) {(wheel)->Status |= WHEEL_LOCKED;}
#define SetWheelInOil(wheel) {(wheel)->Status |= WHEEL_OIL;}
#define SetWheelNotPresent(wheel) {(wheel)->Status &= ~WHEEL_PRESENT;}
#define SetWheelNotTurnable(wheel) {(wheel)->Status &= ~WHEEL_STEERED;}
#define SetWheelNotPowered(wheel) {(wheel)->Status &= ~WHEEL_POWERED;}
#define SetWheelNotInContact(wheel) {(wheel)->Status &= ~WHEEL_CONTACT;}
#define SetWheelNotSpinning(wheel) {(wheel)->Status &= ~WHEEL_SPIN;}
#define SetWheelNotSliding(wheel) {(wheel)->Status &= ~WHEEL_SLIDE;}
#define SetWheelNotLocked(wheel) {(wheel)->Status &= ~WHEEL_LOCKED;}
#define SetWheelNotInOil(wheel) {(wheel)->Status &= ~WHEEL_OIL;}

#define ClearWheelFlags(wheel) {(wheel)->Status &= (WHEEL_PRESENT | WHEEL_POWERED | WHEEL_STEERED | WHEEL_OIL | WHEEL_LOCKED);}

extern void SetupWheel(WHEEL *wheel, WHEEL_INFO *wheelInfo); 
extern void SetupSuspension(SPRING *spring, SPRING_INFO *info);
extern REAL SpringDampedForce(SPRING *spring, REAL extension, REAL velocity);

extern SKIDMARK *AddSkid(SKIDMARK_START *skidStart, SKIDMARK_START *skidEnd, long rgb);
extern void MoveSkidEnd(SKIDMARK *skid, SKIDMARK_START *skidEnd, long rgb);
extern void ClearSkids();
extern void FadeSkidMarks();
extern void TerminateSkid(SKIDMARK *skid);

extern int WHL_NSkids;
extern int WHL_SkidHead;
extern SKIDMARK WHL_SkidMark[];


#endif // WHEEL_H
