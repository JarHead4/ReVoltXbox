//-----------------------------------------------------------------------------
// File: input.h
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef INPUT_H
#define INPUT_H

// macros

#define MAX_JOYSTICKS 4
#define MAX_AXIS 6
#define MAX_BUTTONS 16

//$REVISIT - might actually want some of this code, depending on how we implement vibration
/* $REMOVED
// Force feedback types
enum
{
    FORCEFEEDBACK_ConstantForce = 0x0001,
    FORCEFEEDBACK_RampForce     = 0x0002,
    FORCEFEEDBACK_Square        = 0x0004,
    FORCEFEEDBACK_Sine          = 0x0008,
    FORCEFEEDBACK_Triangle      = 0x0010,
    FORCEFEEDBACK_SawtoothUp    = 0x0020,
    FORCEFEEDBACK_SawtoothDown  = 0x0040,
    FORCEFEEDBACK_Spring        = 0x0080,
    FORCEFEEDBACK_Damper        = 0x0100,
    FORCEFEEDBACK_Inertia       = 0x0200,
    FORCEFEEDBACK_Friction      = 0x0400,
    FORCEFEEDBACK_CustomForce   = 0x0800,
};

// Force feedback structure
typedef struct s_ForceFeedback
{
    long                flags;      // Flags

    IDirectInputEffect* pVibration;
    IDirectInputEffect* pConstant;
    IDirectInputEffect* pSpring;
    IDirectInputEffect* pFriction;

    REAL                durVibration;

} t_ForceFeedback;
$END_REMOVAL */

//$REMOVED
//typedef struct {
//    IDirectInputDevice2 *Device;
//    long AxisNum, ButtonNum;
//    long AxisDisable[MAX_AXIS];
//    DWORD AxisID[MAX_AXIS];
//    DWORD AxisIndex[MAX_AXIS];
//    char AxisName[MAX_AXIS][MAX_PATH];
//    char ButtonName[MAX_BUTTONS][MAX_PATH];
//    char Name[MAX_PATH];
//    DIDEVCAPS Caps;
//
//    t_ForceFeedback forceFeedback; //$REVISIT - might actually want this var, depending on how we implement vibration
//
//} JOYSTICK;
//$END_REMOVAL

enum {
    X_AXIS,
    Y_AXIS,
    Z_AXIS,
    ROTX_AXIS,
    ROTY_AXIS,
    ROTZ_AXIS,
};

// prototypes

extern long InitInput(void);  //$MODIFIED: used to take a param "HINSTANCE hThisInst"
//$REMOVEDextern BOOL CALLBACK EnumJoystickCallback(DIDEVICEINSTANCE *inst, void *user);
//$REMOVEDextern BOOL CALLBACK EnumObjectsCallback(DIDEVICEOBJECTINSTANCE *inst, void *user);
//$REMOVEDextern BOOL CALLBACK EnumEffectsCallback(DIDEVICEOBJECTINSTANCE *pdei, void *user);
//$REMOVEDextern void SetAxisProperties(long axis, long dz, long sat);
extern void KillInput(void);
extern void FlushKeyboard(void);
extern void ReadKeyboard(void);
extern void ReadMouse(void);
extern void ReadJoystick(void);
//$REMOVEDextern void SetMouseExclusive(long flag);
extern unsigned char GetKeyPress(void);

//$REVISIT - might actually want some of this code, depending on how we implement vibration
//$REMOVED
//extern HRESULT InitForceFeedbackJoystick(JOYSTICK* pJoy);
//extern HRESULT FreeForceFeedbackJoystick(JOYSTICK* pJoy);
//extern void SetSafeAllJoyForces(void);
//extern void SetSafeJoyForces(JOYSTICK* pJoy);
//extern HRESULT SetJoyVibration(JOYSTICK* pJoy, int force, float pan, int period, int duration);
//extern void SetJoyConstant(JOYSTICK* pJoy, REAL force, REAL dur);
//extern void SetJoyFriction(JOYSTICK* pJoy, REAL friction);
//$END_REMOVAL

// globals

extern char Keys[256];
extern char LastKeys[256];
//$REMOVEDextern DIMOUSESTATE Mouse;
extern long JoystickNum;
//$REMOVEDextern JOYSTICK Joystick[MAX_JOYSTICKS];
extern DIJOYSTATE JoystickState, LastJoystickState;



#endif // INPUT_H

