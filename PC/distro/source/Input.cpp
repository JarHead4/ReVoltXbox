
#include "revolt.h"
#include "input.h"
#include "dx.h"
#include "main.h"
#include "ctrlread.h"
#include "control.h"

// globals

char Keys[256];
char LastKeys[256];
DIMOUSESTATE Mouse;

static IDirectInput8 *DI;
static IDirectInputDevice8 *KeyboardDevice;
static IDirectInputDevice8 *MouseDevice;
long JoystickNum, CurrentJoystick;
JOYSTICK Joystick[MAX_JOYSTICKS];
DIJOYSTATE JoystickState;

/////////////////////
// key shift table //
/////////////////////

unsigned char ShiftKey[] = {
	'0', ')',
	'1', '!',
	'2', '"',
	'3', '£',
	'4', '$',
	'5', '%',
	'6', '^',
	'7', '&',
	'8', '*',
	'9', '(',

	'-', '_',
	'=', '+',
	92 , '|',

	'[', '{',
	']', '}',
	';', ':',
	39 , '@',
	'#', '~',
	',', '<',
	'.', '>',
	'/', '?',
	'`', '¬',

	255,
};

////////////////////////
// init input devices //
////////////////////////

long InitInput(HINSTANCE inst)
{
	HRESULT r;

// create input object

	r = DirectInput8Create(inst, DIRECTINPUT_VERSION, IID_IDirectInput8A, (LPVOID *) &DI, NULL);
	if (r != DI_OK)
	{
		ErrorDX(r, "Can't create input object!");
		QuitGame = TRUE;
		return FALSE;
	}

// create keyboard device

	r = DI->CreateDevice(GUID_SysKeyboard, &KeyboardDevice, NULL);
	if (r != DI_OK)
	{
		ErrorDX(r, "Can't create keyboard device");
		QuitGame = TRUE;
		return FALSE;
	}

	r = KeyboardDevice->SetDataFormat(&c_dfDIKeyboard);
	if (r != DI_OK)
	{
		ErrorDX(r, "Can't set keyboard data format");
		QuitGame = TRUE;
		return FALSE;
	}

	r = KeyboardDevice->SetCooperativeLevel(hwnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
	if (r != DI_OK)
	{
		ErrorDX(r, "Can't set keyboard coop level");
		return FALSE;
	}

// create mouse device

	r = DI->CreateDevice(GUID_SysMouse, &MouseDevice, NULL);
	if (r != DI_OK)
	{
		ErrorDX(r, "Can't create mouse device");
		QuitGame = TRUE;
		return FALSE;
	}

	r = MouseDevice->SetDataFormat(&c_dfDIMouse);
	if (r != DI_OK)
	{
		ErrorDX(r, "Can't set mouse data format");
		QuitGame = TRUE;
		return FALSE;
	}


	r = MouseDevice->SetCooperativeLevel(hwnd, DISCL_EXCLUSIVE | DISCL_FOREGROUND);
	if (r != DI_OK)
	{
		ErrorDX(r, "Can't set mouse coop level");
		return FALSE;
	}

// enumerate joysticks

	CurrentJoystick = -1;
	JoystickNum = 0;

	r = DI->EnumDevices(DI8DEVCLASS_GAMECTRL, (LPDIENUMDEVICESCALLBACK)EnumJoystickCallback, NULL, DIEDFL_ATTACHEDONLY);
	if (r != DI_OK)
	{
		ErrorDX(r, "Can't enumerate joysticks!");
		QuitGame = TRUE;
		return FALSE;
	}

// return OK

	return TRUE;
}
/////////////////////////
// set mouse exclusive //
/////////////////////////

void SetMouseExclusive(long flag)
{
	MouseDevice->Unacquire();
	MouseDevice->SetCooperativeLevel(hwnd, flag ? DISCL_EXCLUSIVE | DISCL_FOREGROUND : DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
}

///////////////////////////////////
// joystick enumeration callback //
///////////////////////////////////

BOOL CALLBACK EnumJoystickCallback(DIDEVICEINSTANCE *inst, void *user)
{
	long i;
	HRESULT r;
	JOYSTICK *joy = &Joystick[JoystickNum];
	DIPROPRANGE range;
	DIPROPDWORD deadzone, saturation;
	IDirectInputDevice8 *dev;

// create this device

	r = DI->CreateDevice(inst->guidInstance, &dev, NULL);
	if (r != DI_OK)
	{
		return DIENUM_CONTINUE;
	}

	r = dev->QueryInterface(IID_IDirectInputDevice2, (void**)&joy->Device);
	RELEASE(dev);
	if (r != DI_OK)
	{
		return DIENUM_CONTINUE;
	}

	r = joy->Device->SetDataFormat(&c_dfDIJoystick);
	if (r != DI_OK)
	{
		RELEASE(joy->Device);
		return DIENUM_CONTINUE;
	}

	r = joy->Device->SetCooperativeLevel(hwnd, DISCL_EXCLUSIVE | DISCL_FOREGROUND);
	if (r != DI_OK)
	{
		RELEASE(joy->Device);
		return DIENUM_CONTINUE;
	}

	joy->Caps.dwSize = sizeof(joy->Caps);
	r = joy->Device->GetCapabilities(&joy->Caps);
	if (r != DI_OK)
	{
		RELEASE(joy->Device);
		return DIENUM_CONTINUE;
	}

// set axis range

	range.diph.dwSize = sizeof(range);
	range.diph.dwHeaderSize = sizeof(range.diph);
	range.diph.dwObj = 0;
	range.diph.dwHow = DIPH_DEVICE;
    range.lMin = -CTRL_RANGE_MAX; 
    range.lMax = CTRL_RANGE_MAX; 

	r = joy->Device->SetProperty(DIPROP_RANGE, &range.diph);
	if (r != DI_OK)
	{
		RELEASE(joy->Device);
		return DIENUM_CONTINUE;
	}

// set dead zone

	deadzone.diph.dwSize = sizeof(deadzone);
	deadzone.diph.dwHeaderSize = sizeof(deadzone.diph);
	deadzone.diph.dwObj = 0;
	deadzone.diph.dwHow = DIPH_DEVICE;
	deadzone.dwData = 1000;

	r = joy->Device->SetProperty(DIPROP_DEADZONE, &deadzone.diph);
	if (r != DI_OK)
	{
		RELEASE(joy->Device);
		return DIENUM_CONTINUE;
	}

// set saturation

	saturation.diph.dwSize = sizeof(saturation);
	saturation.diph.dwHeaderSize = sizeof(saturation.diph);
	saturation.diph.dwObj = 0;
	saturation.diph.dwHow = DIPH_DEVICE;
	saturation.dwData = 9000;

	r = joy->Device->SetProperty(DIPROP_SATURATION, &saturation.diph);
	if (r != DI_OK)
	{
		RELEASE(joy->Device);
		return DIENUM_CONTINUE;
	}

// enumerate objects

	for (i = 0 ; i < MAX_AXIS ; i++) joy->Axis[i] = FALSE;

	r = joy->Device->EnumObjects((LPDIENUMDEVICEOBJECTSCALLBACK)EnumObjectsCallback, (void*)joy, DIDFT_ALL);
	if (r != DI_OK)
	{
		RELEASE(joy->Device);
		return DIENUM_CONTINUE;
	}

// copy name

	memcpy(joy->Name, inst->tszProductName, MAX_PATH);

// next please

	JoystickNum++;
	if (JoystickNum == MAX_JOYSTICKS) return DIENUM_STOP;
	else return DIENUM_CONTINUE;
}

//////////////////////////////////////////
// joystick object enumeration callback //
//////////////////////////////////////////

BOOL CALLBACK EnumObjectsCallback(DIDEVICEOBJECTINSTANCE *inst, void *user)
{
	JOYSTICK *joy = (JOYSTICK*)user;

// axis?

	if (inst->dwType & DIDFT_ABSAXIS)
	{
		if (inst->guidType == GUID_XAxis) joy->Axis[X_AXIS] = TRUE;
		if (inst->guidType == GUID_YAxis) joy->Axis[Y_AXIS] = TRUE;
		if (inst->guidType == GUID_ZAxis) joy->Axis[Z_AXIS] = TRUE;
		if (inst->guidType == GUID_RxAxis) joy->Axis[ROTX_AXIS] = TRUE;
		if (inst->guidType == GUID_RyAxis) joy->Axis[ROTY_AXIS] = TRUE;
		if (inst->guidType == GUID_RzAxis) joy->Axis[ROTZ_AXIS] = TRUE;
	}

// return OK

	return DIENUM_CONTINUE;
}

////////////////////////
// kill input devices //
////////////////////////

void KillInput(void)
{
	long i;

// kill keyboard

	KeyboardDevice->Unacquire();
	RELEASE(KeyboardDevice);

// kill mouse

	MouseDevice->Unacquire();
	RELEASE(MouseDevice);

// kill joysticks

	for (i = 0 ; i < JoystickNum ; i++)
	{
		Joystick[i].Device->Unacquire();
		RELEASE(Joystick[i].Device);
	}

// kill input object

	RELEASE(DI);
}

///////////////////
// read keyboard //
///////////////////

void ReadKeyboard(void)
{
	long i;

// copy current to last

	for (i = 0 ; i < 256 ; i++)
		LastKeys[i] = Keys[i];

// read current

	KeyboardDevice->Acquire();
	KeyboardDevice->GetDeviceState(sizeof(Keys), &Keys);
}

////////////////
// read mouse //
////////////////

void ReadMouse(void)
{

// read current

	MouseDevice->Acquire();
	MouseDevice->GetDeviceState(sizeof(Mouse), &Mouse);
}

///////////////////
// read joystick //
///////////////////

void ReadJoystick(void)
{

// current joystick?

	if (CurrentJoystick == -1)
		return;

// yep, read current

	Joystick[CurrentJoystick].Device->Acquire();
	Joystick[CurrentJoystick].Device->Poll();
	Joystick[CurrentJoystick].Device->GetDeviceState(sizeof(JoystickState), &JoystickState);
}

///////////////////////
// get a key pressed //
///////////////////////

unsigned char GetKeyPress(void)
{
	short i;
	unsigned char *p;
	unsigned long vk, ch;

// loop thru all keys, any new presses?

	for (i = 0 ; i < 255 ; i++) if (Keys[i] && !LastKeys[i])
	{

// yep, get ascii value

		vk = MapVirtualKey(i, 1);
		if (!vk) continue;

		ch = MapVirtualKey(vk, 2);
		if (!ch) continue;

// shift?

		if (Keys[DIK_LSHIFT] || Keys[DIK_RSHIFT])
		{
			p = ShiftKey;
			while (*p != 255 && *p != ch) p += 2;
			if (*p == ch) ch = *(p + 1);
		}

// no shift

		else
		{
			if (ch >= 'A' && ch <= 'Z' && !(GetKeyState(VK_CAPITAL) & 1)) ch += ('a' - 'A');
		}

		return (unsigned char)ch;
	}

// return none

	return FALSE;
}
