
#ifndef INPUT_H
#define INPUT_H

// macros

#define MAX_JOYSTICKS 4
#define MAX_AXIS 6

typedef struct {
	IDirectInputDevice2 *Device;
	long Axis[MAX_AXIS];
	DIDEVCAPS Caps;
	char Name[MAX_PATH];
} JOYSTICK;

enum {
	X_AXIS,
	Y_AXIS,
	Z_AXIS,
	ROTX_AXIS,
	ROTY_AXIS,
	ROTZ_AXIS,
};

// prototypes

extern long InitInput(HINSTANCE inst);
extern BOOL CALLBACK EnumJoystickCallback(DIDEVICEINSTANCE *inst, void *user);
extern BOOL CALLBACK EnumObjectsCallback(DIDEVICEOBJECTINSTANCE *inst, void *user);
extern void KillInput(void);
extern void ReadKeyboard(void);
extern void ReadMouse(void);
extern void ReadJoystick(void);
extern void SetMouseExclusive(long flag);
extern unsigned char GetKeyPress(void);

// globals

extern char Keys[256];
extern char LastKeys[256];
extern DIMOUSESTATE Mouse;
extern long JoystickNum, CurrentJoystick;
extern JOYSTICK Joystick[MAX_JOYSTICKS];
extern DIJOYSTATE JoystickState;

#endif