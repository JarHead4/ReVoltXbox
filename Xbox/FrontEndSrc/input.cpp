//-----------------------------------------------------------------------------
// File: input.cpp
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"
#include "input.h"
#include "dx.h"
#include "main.h"
#include "ctrlread.h"
#include "control.h"
#include "settings.h"
//#include "gamegauge.h"

#include "XBInput.h" //$ADDITION(mwetzel)
#include "VoiceCommunicator.h"
#include "player.h" //$ADDITION(jedl) - to access PLR_LocalPlayer

// globals

char Keys[256];
char LastKeys[256];
//$REMOVED DIMOUSESTATE Mouse;

//$REMOVEDstatic IDirectInput *DI;
//$REMOVEDstatic IDirectInputDevice *KeyboardDevice;
//$REMOVEDstatic IDirectInputDevice *MouseDevice;
long JoystickNum;
//$REMOVEDJOYSTICK Joystick[MAX_JOYSTICKS];
DIJOYSTATE JoystickState, LastJoystickState;


/////////////////////
// key shift table //
/////////////////////

static unsigned char ShiftKey[] = {
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
// special case table //
////////////////////////

static unsigned char SpecialKey[] = {
    DIK_NUMPAD0, '0',
    DIK_NUMPAD1, '1',
    DIK_NUMPAD2, '2',
    DIK_NUMPAD3, '3',
    DIK_NUMPAD4, '4',
    DIK_NUMPAD5, '5',
    DIK_NUMPAD6, '6',
    DIK_NUMPAD7, '7',
    DIK_NUMPAD8, '8',
    DIK_NUMPAD9, '9',

    DIK_DECIMAL, '.',
    DIK_NUMPADENTER, 13,

    255, 255
};

////////////////////////
// init input devices //
////////////////////////

long InitInput(void)  //$MODIFIED: used to take a param "HINSTANCE hThisInst"
{
//$REMOVED
//    HRESULT r;
//
//// create input object
//
//    r = DirectInputCreate(inst, DIRECTINPUT_VERSION, &DI, NULL);
//    if (r != DI_OK)
//    {
//        ErrorDX(r, "Can't create input object!");
//        g_bQuitGame = TRUE;
//        return FALSE;
//    }
//
//// create keyboard device
//
//    r = DI->CreateDevice(GUID_SysKeyboard, &KeyboardDevice, NULL);
//    if (r != DI_OK)
//    {
//        ErrorDX(r, "Can't create keyboard device");
//        g_bQuitGame = TRUE;
//        return FALSE;
//    }
//
//    r = KeyboardDevice->SetDataFormat(&c_dfDIKeyboard);
//    if (r != DI_OK)
//    {
//        ErrorDX(r, "Can't set keyboard data format");
//        g_bQuitGame = TRUE;
//        return FALSE;
//    }
//
//    r = KeyboardDevice->SetCooperativeLevel(hwnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
//    if (r != DI_OK)
//    {
//        ErrorDX(r, "Can't set keyboard coop level");
//        return FALSE;
//    }
//
//// create mouse device
//
//    r = DI->CreateDevice(GUID_SysMouse, &MouseDevice, NULL);
//    if (r != DI_OK)
//    {
//        ErrorDX(r, "Can't create mouse device");
//        g_bQuitGame = TRUE;
//        return FALSE;
//    }
//
//    r = MouseDevice->SetDataFormat(&c_dfDIMouse);
//    if (r != DI_OK)
//    {
//        ErrorDX(r, "Can't set mouse data format");
//        g_bQuitGame = TRUE;
//        return FALSE;
//    }
//
//
//    r = MouseDevice->SetCooperativeLevel(hwnd, DISCL_EXCLUSIVE | DISCL_FOREGROUND);
//    if (r != DI_OK)
//    {
//        ErrorDX(r, "Can't set mouse coop level");
//        return FALSE;
//    }
//$END_REMOVAL

// enumerate joysticks

//$MODIFIED
//    JoystickNum = 0;
//
//    r = DI->EnumDevices(DIDEVTYPE_JOYSTICK, (LPDIENUMDEVICESCALLBACK)EnumJoystickCallback, NULL, DIEDFL_ATTACHEDONLY);
//    if (r != DI_OK)
//    {
//        ErrorDX(r, "Can't enumerate joysticks!");
//        g_bQuitGame = TRUE;
//        return FALSE;
//    }
//
//    if (RegistrySettings.Joystick > JoystickNum - 1)
//    {
//        RegistrySettings.Joystick = JoystickNum - 1;
//    }
    XBInput_InitControllers();
    CVoiceCommunicator::InitCommunicators();
//$END_MODIFICATIONS

// return OK

    return TRUE;
}


/* $REMOVED
/////////////////////////
// set mouse exclusive //
/////////////////////////

void SetMouseExclusive(long flag)
{
    MouseDevice->Unacquire();
    MouseDevice->SetCooperativeLevel(hwnd, flag ? DISCL_EXCLUSIVE | DISCL_FOREGROUND : DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
}
$END_REMOVAL */


/* $REMOVED
///////////////////////////////////
// joystick enumeration callback //
///////////////////////////////////

BOOL CALLBACK EnumJoystickCallback(DIDEVICEINSTANCE *inst, void *user)
{
    HRESULT r;
    JOYSTICK *joy = &Joystick[JoystickNum];
    DIPROPRANGE range;
    DIPROPDWORD deadzone, saturation;
    IDirectInputDevice *dev;

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
    deadzone.dwData = 1500;

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

    joy->AxisNum = 0;
    joy->ButtonNum = 0;

    r = joy->Device->EnumObjects((LPDIENUMDEVICEOBJECTSCALLBACK)EnumObjectsCallback, (void*)joy, DIDFT_ALL);
    if (r != DI_OK)
    {
        RELEASE(joy->Device);
        return DIENUM_CONTINUE;
    }


// enumerate effects (force feedback)
    joy->forceFeedback.flags = 0;

    r = joy->Device->EnumEffects((LPDIENUMEFFECTSCALLBACK)EnumEffectsCallback, (void*)joy, DIEFT_ALL);
    if (r != DI_OK)
    {
        RELEASE(joy->Device);
        return DIENUM_CONTINUE;
    }

    InitForceFeedbackJoystick(joy);


// copy name

    memcpy(joy->Name, inst->tszProductName, MAX_PATH);

// next please

    JoystickNum++;
    if (JoystickNum == MAX_JOYSTICKS) return DIENUM_STOP;
    else return DIENUM_CONTINUE;
}
$END_REMOVAL */


/* $REMOVED
//////////////////////////////////////////
// joystick object enumeration callback //
//////////////////////////////////////////

BOOL CALLBACK EnumObjectsCallback(DIDEVICEOBJECTINSTANCE *inst, void *user)
{
    JOYSTICK *joy = (JOYSTICK*)user;

// axis?

    if (inst->dwType & DIDFT_ABSAXIS && joy->AxisNum < MAX_AXIS)
    {
        strncpy(joy->AxisName[joy->AxisNum], inst->tszName, MAX_PATH);
        joy->AxisID[joy->AxisNum] = inst->dwType;
        joy->AxisIndex[joy->AxisNum] = inst->dwOfs / sizeof(long);
        joy->AxisNum++;
    }

// button?

    else if (inst->dwType & DIDFT_BUTTON && joy->ButtonNum < MAX_BUTTONS)
    {
        strncpy(joy->ButtonName[joy->ButtonNum], inst->tszName, MAX_PATH);
        joy->ButtonNum++;
    }

// return OK

    return DIENUM_CONTINUE;
}
$END_REMOVAL */


//$REVISIT - might actually want some of this code, depending on how we implement vibration
/* $REMOVED
/////////////////////////////////////////////////
// joystick forcefeedback enumeration callback //
/////////////////////////////////////////////////

BOOL CALLBACK EnumEffectsCallback(DIDEVICEOBJECTINSTANCE *pdei, void *user)
{
    JOYSTICK *joy = (JOYSTICK*)user;

// Constant force
    if (pdei->guidType == GUID_ConstantForce)
    {
        joy->forceFeedback.flags |= FORCEFEEDBACK_ConstantForce;
    }

// Ramp force
    else if (pdei->guidType == GUID_RampForce)
    {
        joy->forceFeedback.flags |= FORCEFEEDBACK_RampForce;
    }

// Square
    else if (pdei->guidType == GUID_Square)
    {
        joy->forceFeedback.flags |= FORCEFEEDBACK_Square;
    }

// Sine
    else if (pdei->guidType == GUID_Sine)
    {
        joy->forceFeedback.flags |= FORCEFEEDBACK_Sine;
    }

// Triangle
    else if (pdei->guidType == GUID_Triangle)
    {
        joy->forceFeedback.flags |= FORCEFEEDBACK_Triangle;
    }

// Sawtooth up
    else if (pdei->guidType == GUID_SawtoothUp)
    {
        joy->forceFeedback.flags |= FORCEFEEDBACK_SawtoothUp;
    }

// Sawtooth down
    else if (pdei->guidType == GUID_SawtoothDown)
    {
        joy->forceFeedback.flags |= FORCEFEEDBACK_SawtoothDown;
    }

// Spring
    else if (pdei->guidType == GUID_Spring)
    {
        joy->forceFeedback.flags |= FORCEFEEDBACK_Spring;
    }

// Damper
    else if (pdei->guidType == GUID_Damper)
    {
        joy->forceFeedback.flags |= FORCEFEEDBACK_Damper;
    }

// Inertia
    else if (pdei->guidType == GUID_Inertia)
    {
        joy->forceFeedback.flags |= FORCEFEEDBACK_Inertia;
    }

// Friction
    else if (pdei->guidType == GUID_Friction)
    {
        joy->forceFeedback.flags |= FORCEFEEDBACK_Friction;
    }

// Custom force
    else if (pdei->guidType == GUID_CustomForce)
    {
        joy->forceFeedback.flags |= FORCEFEEDBACK_CustomForce;
    }

    return DIENUM_CONTINUE;
}
$END_REMOVAL */

 
/* $REMOVED
///////////////////////////////////
// set deadzone / range for axis //
///////////////////////////////////

void SetAxisProperties(long axis, long dz, long sat)
{
    DIPROPDWORD deadzone, saturation;
    HRESULT r;
    JOYSTICK *joy;

// get joystick

    if (RegistrySettings.Joystick == -1)
        return;

    joy = &Joystick[RegistrySettings.Joystick];

// set dead zone

    deadzone.diph.dwSize = sizeof(deadzone);
    deadzone.diph.dwHeaderSize = sizeof(deadzone.diph);
    deadzone.diph.dwObj = Joystick[RegistrySettings.Joystick].AxisID[axis];
    deadzone.diph.dwHow = DIPH_BYID;
    deadzone.dwData = dz;

    r = joy->Device->SetProperty(DIPROP_DEADZONE, &deadzone.diph);
    if (r != DI_OK)
    {
        ErrorDX(r, "Can't set axis deadzone");
    }

// set saturation

    saturation.diph.dwSize = sizeof(saturation);
    saturation.diph.dwHeaderSize = sizeof(saturation.diph);
    saturation.diph.dwObj = Joystick[RegistrySettings.Joystick].AxisID[axis];
    saturation.diph.dwHow = DIPH_BYID;
    saturation.dwData = sat;

    r = joy->Device->SetProperty(DIPROP_SATURATION, &saturation.diph);
    if (r != DI_OK)
    {
        ErrorDX(r, "Can't set axis range");
    }
}
$END_REMOVAL */


////////////////////////
// kill input devices //
////////////////////////

void KillInput(void)
{
//$MODIFIED
//    long i;
//
//// kill keyboard
//
//    KeyboardDevice->Unacquire();
//    RELEASE(KeyboardDevice);
//
//// kill mouse
//
//    MouseDevice->Unacquire();
//    RELEASE(MouseDevice);
//
//// kill joysticks
//
//    for (i = 0 ; i < JoystickNum ; i++)
//    {
//        FreeForceFeedbackJoystick(&Joystick[i]);
//        Joystick[i].Device->Unacquire();
//        RELEASE(Joystick[i].Device);
//    }
//
//// kill input object
//
//    RELEASE(DI);

    //$REVISIT: is there anything to be done here?

//$END_MODIFICATIONS
}

///////////////////
// read keyboard //
///////////////////

void ReadKeyboard(void)
{
//$REMOVED_NOTUSED
//    long i;
//
//// not if game gauge
//
//// copy current to last
//
//    for (i = 0 ; i < 256 ; i++)
//        LastKeys[i] = Keys[i];
//
//// read current
//
//    KeyboardDevice->Acquire();
//    KeyboardDevice->GetDeviceState(sizeof(Keys), &Keys);
//$END_REMOVAL
//$NOTE(cprince): for now, we update LastKeys[] and Keys[] in ReadJoystick()
}

////////////////////
// flush keyboard //
////////////////////

void FlushKeyboard(void)
{
    ZeroMemory(Keys, 256);
    ZeroMemory(LastKeys, 256);
}

////////////////
// read mouse //
////////////////

void ReadMouse(void)
{
//$REMOVED_NOTUSED
//
//// not if game gauge
//
//// read current
//
//    MouseDevice->Acquire();
//    MouseDevice->GetDeviceState(sizeof(Mouse), &Mouse);
//$END_REMOVAL
}

///////////////////
// read joystick //
///////////////////

void ReadJoystick(void)
{
// current joystick?

    if (RegistrySettings.Joystick == -1)
        return;

// yep, save last, read current

    LastJoystickState = JoystickState;

//$MODIFIED
//    Joystick[RegistrySettings.Joystick].Device->Acquire();
//    Joystick[RegistrySettings.Joystick].Device->Poll();
//    Joystick[RegistrySettings.Joystick].Device->GetDeviceState(sizeof(JoystickState), &JoystickState);
//

    //$BUGBUG: we're assuming only controller0 here.
    XBInput_GetInput();

    // Update the 'Keys' and 'LastKeys' arrays
    //$CMP_NOTE: (1) maybe this isn't the best place to be doing this, (2) instead of a "keys" array, we should probably use a more generalized input struct (that tells whether up/down/etc is pressed)
    memcpy( LastKeys, Keys, sizeof(Keys) );
    ZeroMemory( Keys, sizeof(Keys) );

	if( g_Controllers[0].dwMenuInput == XBINPUT_LEFT )  Keys[DIK_LEFT]  = 1;
	if( g_Controllers[0].dwMenuInput == XBINPUT_RIGHT ) Keys[DIK_RIGHT] = 1;
	if( g_Controllers[0].dwMenuInput == XBINPUT_UP )    Keys[DIK_UP]    = 1;
	if( g_Controllers[0].dwMenuInput == XBINPUT_DOWN )  Keys[DIK_DOWN]  = 1;

	if( g_Controllers[0].dwMenuInput == XBINPUT_A_BUTTON )     Keys[DIK_RETURN]  = 1;
	if( g_Controllers[0].dwMenuInput == XBINPUT_B_BUTTON )     Keys[DIK_ESCAPE]  = 1;
//	if( g_Controllers[0].dwMenuInput == XBINPUT_X_BUTTON )     Keys[?]  = 1;
//	if( g_Controllers[0].dwMenuInput == XBINPUT_Y_BUTTON )     Keys[?]  = 1;
//	if( g_Controllers[0].dwMenuInput == XBINPUT_WHITE_BUTTON ) Keys[?] = TRUE;
//	if( g_Controllers[0].dwMenuInput == XBINPUT_BLACK_BUTTON ) Keys[?] = TRUE;

	if( g_Controllers[0].dwMenuInput == XBINPUT_START_BUTTON ) Keys[DIK_RETURN]  = 1;
	if( g_Controllers[0].dwMenuInput == XBINPUT_BACK_BUTTON )  Keys[DIK_ESCAPE]  = 1;

    ZeroMemory( &JoystickState, sizeof(JoystickState) );

    const FLOAT THUMB_MAX = 32767.5f;
    const FLOAT TRIGGER_MAX = 255.0f;

    // Read input if the device type is a gamepad
    if( g_Controllers[0].dwDeviceType == XBINPUT_CONTROLLER_GAMEPAD )
    {
        float fx = (float(g_Controllers[0].sThumbLX) + 0.5f) / THUMB_MAX;
        if( fx < 0.24f && fx > -0.24f ) { fx = 0.0f; }  // thumbstick deadzone
        JoystickState.lX = (LONG)(fx * (CTRL_RANGE_MAX));

        float fy = (float(g_Controllers[0].sThumbRY) + 0.5f) / THUMB_MAX;
        if( fy < 0.24f && fy > -0.24f ) { fy = 0.0f; }  // thumbstick deadzone
        fy += g_Controllers[0].bAnalogButtons[XINPUT_GAMEPAD_RIGHT_TRIGGER] / TRIGGER_MAX; // forward
        fy -= g_Controllers[0].bAnalogButtons[XINPUT_GAMEPAD_LEFT_TRIGGER] / TRIGGER_MAX; // reverse
        fy = ( (fy < -1.0f) ? -1.0f : (fy > +1.0f) ? +1.0f : fy ); // clamp to -1.0 .. +1.0
        JoystickState.lY = (LONG)(fy * (-CTRL_RANGE_MAX));

        if( g_Controllers[0].bAnalogButtons[XINPUT_GAMEPAD_A] > XINPUT_GAMEPAD_MAX_CROSSTALK )
            JoystickState.rgbButtons[0] = 1;
        if (g_Controllers[0].wButtons & XINPUT_GAMEPAD_RIGHT_THUMB)
            JoystickState.rgbButtons[0] = 1;  // use right thumbstuck button to fire, too.
        if( g_Controllers[0].bAnalogButtons[XINPUT_GAMEPAD_B] > XINPUT_GAMEPAD_MAX_CROSSTALK )
            JoystickState.rgbButtons[1] = 1;
    }

    // Read input if the device type is a steering wheel
    if( g_Controllers[0].dwDeviceType == XBINPUT_CONTROLLER_WHEEL )
    {
        const FLOAT WHEEL_MAX = 32767.5f;

        // Map X using steering wheel
        float fx = g_Controllers[0].sThumbLX / WHEEL_MAX;
        JoystickState.lX = (char)(127.f * fx);

        // Map Y using gas and brake pedals
        float fy = 0.0f;
        fy += g_Controllers[0].bAnalogButtons[XINPUT_GAMEPAD_LEFT_TRIGGER];
        fy -= g_Controllers[0].bAnalogButtons[XINPUT_GAMEPAD_RIGHT_TRIGGER];
        JoystickState.lY = (char)(0.5f * fy);

        // Fire button
        if( g_Controllers[0].bAnalogButtons[XINPUT_GAMEPAD_A] > XINPUT_GAMEPAD_MAX_CROSSTALK )
            JoystickState.rgbButtons[0] = 1;

        // Reset button
        if( g_Controllers[0].bAnalogButtons[XINPUT_GAMEPAD_B] > XINPUT_GAMEPAD_MAX_CROSSTALK )
            JoystickState.rgbButtons[1] = 1;
    }

    //$ADDED(jedl) - for debugging, toggle new graphics pipeline
    if( g_Controllers[0].dwMenuInput == XBINPUT_WHITE_BUTTON )
    {
        // Toggle use of the new pipeline
        RegistrySettings.bUseGPU = !RegistrySettings.bUseGPU;
    }

    //$ADDED(jedl) - for debugging, reset and slew mode for car position
    if( g_Controllers[0].dwMenuInput == XBINPUT_Y_BUTTON )
    {
        VEC pos;
        MAT mat;
        if ( g_Controllers[0].bAnalogButtons[XINPUT_GAMEPAD_LEFT_TRIGGER] > 255 - XINPUT_GAMEPAD_MAX_CROSSTALK
            && g_Controllers[0].bAnalogButtons[XINPUT_GAMEPAD_RIGHT_TRIGGER] > 255 - XINPUT_GAMEPAD_MAX_CROSSTALK )
        {
            // Reset to default starting location
            GetCarStartGrid(0, &pos, &mat);
        }
        else
        {
            // Get planar direction
            pos = PLR_LocalPlayer->car.Body->Centre.Pos;
            mat = PLR_LocalPlayer->car.Body->Centre.WMatrix;
            XGVECTOR3 vLook(CAM_MainCamera->WorldPosOffset.v);
            vLook.y = 0.f; // remove y component
            XGVECTOR3 vLookPerp(vLook.z, 0.f, -vLook.x);    // perpendicular direction to view
            XGVECTOR3 vDelta(0.f, 0.f, 0.f);
            static FLOAT fScale = -0.01f;
            if (g_Controllers[0].wButtons & XINPUT_GAMEPAD_DPAD_UP)
                vDelta += fScale * vLook;
            if (g_Controllers[0].wButtons & XINPUT_GAMEPAD_DPAD_DOWN)
                vDelta -= fScale * vLook;
            if (g_Controllers[0].wButtons & XINPUT_GAMEPAD_DPAD_RIGHT)
                vDelta += fScale * vLookPerp;
            if (g_Controllers[0].wButtons & XINPUT_GAMEPAD_DPAD_LEFT)
                vDelta -= fScale * vLookPerp;

            // Force car to move by delta
            *(XGVECTOR3 *)&pos += vDelta;

            // Put car above ground
            static FLOAT fOffset = -100.0;
            VEC start = pos;
            VEC end = start;
            start.v[1] += fOffset;
            end.v[1] -= fOffset;
            REAL fFraction;
            PLANE *pPlane;
            LineOfSightDist(&start, &end, &fFraction, &pPlane);
            if (fFraction < 1.f)
            {
                static FLOAT fExtra = -100.f;   // extra amount to put car above ground intersection
                pos.v[1] = start.v[1] + fFraction * (end.v[1] - start.v[1]) + fExtra;
            }

            // Rotate car with left stick
            static FLOAT fRotAzimScale = -0.005f;
            float fx = g_Controllers[0].sThumbLX / THUMB_MAX;
            // cube controls for finer accuracy
            FLOAT fRotAzim = fRotAzimScale * fx * fx * fx;
            MAT matRot;
            RotMatrixY(&matRot, fRotAzim);
            MAT matOut;
            MulMatrix(&matRot, &mat, &matOut);
            mat = matOut;
        }
        SetCarPos(&PLR_LocalPlayer->car, &pos, &mat);
    }
}

///////////////////////
// get a key pressed //
///////////////////////

unsigned char GetKeyPress(void)
{
//$REMOVED
//    short i;
//    unsigned char *p;
//    unsigned long vk, ch;
//
//// loop thru all keys, any new presses?
//
//    for (i = 0 ; i < 255 ; i++) if (Keys[i] && !LastKeys[i])
//    {
//
//// handle special cases
//
//        p = SpecialKey;
//        while (p[0] != 255 && p[0] != i) p += 2;
//        if (p[0] == i && p[1] != i)
//        {
//            return p[1];
//        }
//
//// yep, get ascii value
//
//        vk = MapVirtualKey(i, 1);
//        if (!vk) continue;
//
//        ch = MapVirtualKey(vk, 2);
//        if (!ch) continue;
//
//// shift?
//
//        if (Keys[DIK_LSHIFT] || Keys[DIK_RSHIFT])
//        {
//            p = ShiftKey;
//            while (p[0] != 255 && p[0] != ch) p += 2;
//            if (p[0] == ch) ch = p[1];
//        }
//
//// no shift
//
//        else
//        {
//            if (ch >= 'A' && ch <= 'Z' && !(GetKeyState(VK_CAPITAL) & 1)) ch += ('a' - 'A');
//        }
//
//        return (unsigned char)ch;
//    }

// return none

    return FALSE;
}


//$REVISIT - might actually want some of this code, depending on how we implement vibration
/* $REMOVED (tentative!!)
////////////////////////////////
// InitForceFeedbackJoystick()
////////////////////////////////
HRESULT InitForceFeedbackJoystick(JOYSTICK* pJoy)
{
    HRESULT hr;

// Clear pointers
    pJoy->forceFeedback.pVibration  = NULL;
    pJoy->forceFeedback.pConstant   = NULL;
    pJoy->forceFeedback.pSpring     = NULL;
    pJoy->forceFeedback.pFriction   = NULL;

    pJoy->forceFeedback.durVibration = 0;

// Set Autocentering on
    DIPROPDWORD dipdw;
    dipdw.diph.dwSize       = sizeof(DIPROPDWORD);
    dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
    dipdw.diph.dwObj        = 0;
    dipdw.diph.dwHow        = DIPH_DEVICE;
//  dipdw.dwData            = DIPROPAUTOCENTER_ON;
    dipdw.dwData            = DIPROPAUTOCENTER_OFF;

    hr = pJoy->Device->SetProperty(DIPROP_AUTOCENTER, &dipdw.diph);
    if (FAILED(hr))
        return hr;

// This application needs only one effect:  Applying raw forces.
    DIEFFECT        eff;
    DWORD           rgdwAxes[2] = {DIJOFS_X, DIJOFS_Y};
    LONG            rglDirection[2] = {0, 0};
    DICONSTANTFORCE cf = {0};

    eff.dwSize                  = sizeof(DIEFFECT);
    eff.dwFlags                 = DIEFF_CARTESIAN | DIEFF_OBJECTOFFSETS;
    eff.dwDuration              = INFINITE;
    eff.dwSamplePeriod          = 0;
    eff.dwGain                  = DI_FFNOMINALMAX;
    eff.dwTriggerButton         = DIEB_NOTRIGGER;
    eff.dwTriggerRepeatInterval = 0;
    eff.cAxes                   = 2;
    eff.rgdwAxes                = rgdwAxes;
    eff.rglDirection            = rglDirection;
    eff.lpEnvelope              = 0;
    eff.cbTypeSpecificParams    = sizeof(cf);
    eff.lpvTypeSpecificParams   = &cf;

    hr = pJoy->Device->CreateEffect(GUID_ConstantForce, &eff, &pJoy->forceFeedback.pConstant, NULL);
    if (FAILED(hr)) 
        pJoy->forceFeedback.pConstant = NULL;

// Periodic effect - Sine wave etc...
    DIPERIODIC      periodic = {DI_FFNOMINALMAX,0,0,1000};

    eff.dwSize                  = sizeof(DIEFFECT);
    eff.dwFlags                 = DIEFF_CARTESIAN | DIEFF_OBJECTOFFSETS;
    eff.dwDuration              = INFINITE;
    eff.dwSamplePeriod          = 0;
    eff.dwGain                  = DI_FFNOMINALMAX;
    eff.dwTriggerButton         = DIEB_NOTRIGGER;
    eff.dwTriggerRepeatInterval = 0;
    eff.cAxes                   = 2;
    eff.rgdwAxes                = rgdwAxes;
    eff.rglDirection            = rglDirection;
    eff.lpEnvelope              = 0;
    eff.cbTypeSpecificParams    = sizeof(periodic);
    eff.lpvTypeSpecificParams   = &periodic;

    hr = pJoy->Device->CreateEffect(GUID_Triangle, &eff, &pJoy->forceFeedback.pVibration, NULL);
    if (FAILED(hr)) 
        pJoy->forceFeedback.pVibration = NULL;

// Friction effect
    DICONDITION condition = {0, 0,0, 0,0, 0};

    eff.dwSize                  = sizeof(DIEFFECT);
    eff.dwFlags                 = DIEFF_CARTESIAN | DIEFF_OBJECTOFFSETS;
    eff.dwDuration              = INFINITE;
    eff.dwSamplePeriod          = 0;
    eff.dwGain                  = DI_FFNOMINALMAX;
    eff.dwTriggerButton         = DIEB_NOTRIGGER;
    eff.dwTriggerRepeatInterval = 0;
    eff.cAxes                   = 2;
    eff.rgdwAxes                = rgdwAxes;
    eff.rglDirection            = rglDirection;
    eff.lpEnvelope              = 0;
    eff.cbTypeSpecificParams    = sizeof(condition);
    eff.lpvTypeSpecificParams   = &condition;

    hr = pJoy->Device->CreateEffect(GUID_Friction, &eff, &pJoy->forceFeedback.pFriction, NULL);
    if (FAILED(hr)) 
        pJoy->forceFeedback.pFriction = NULL;

    return S_OK;
}

////////////////////////////////////////
// Function: FreeForceFeedbackJoystick
////////////////////////////////////////
HRESULT FreeForceFeedbackJoystick(JOYSTICK* pJoy)
{
// Set force feedback to something safe
    SetSafeAllJoyForces();

// Release Effect objects.
    if (NULL != pJoy->forceFeedback.pVibration) 
    {
        pJoy->forceFeedback.pVibration->Release();
    }

    if (NULL != pJoy->forceFeedback.pConstant) 
    {
        pJoy->forceFeedback.pConstant->Release();
    }

    if (NULL != pJoy->forceFeedback.pSpring) 
    {
        pJoy->forceFeedback.pSpring->Release();
    }

    if (NULL != pJoy->forceFeedback.pFriction) 
    {
        pJoy->forceFeedback.pFriction->Release();
    }

// Reset pointers
    pJoy->forceFeedback.pVibration  = NULL;
    pJoy->forceFeedback.pConstant   = NULL;
    pJoy->forceFeedback.pSpring     = NULL;
    pJoy->forceFeedback.pFriction   = NULL;

    return S_OK;
}

///////////////////////////////////////////
// Function: SetSafeJoyForces
///////////////////////////////////////////
void SetSafeAllJoyForces(void)
{
    int i;
    for (i = 0; i < JoystickNum; i++)
        SetSafeJoyForces(&Joystick[i]);
}

void SetSafeJoyForces(JOYSTICK* pJoy)
{
    SetJoyVibration(pJoy, 0,0, 0,0);
    SetJoyConstant(pJoy, 0,0);
    SetJoyFriction(pJoy, 0);
}


///////////////////////////////////////////
// Function: SetJoyForcesXY
//
// Input:
//  pJoy        Joystick structure
//  force       0-10000
//  pan         -1 to +1
//  duration    In milliseconds
//  period      In some strange format !!
///////////////////////////////////////////
HRESULT SetJoyVibration(JOYSTICK* pJoy, int force, float pan, int period, int duration)
{
    HRESULT hr;

    if (pJoy->forceFeedback.pVibration)
    {
        DIEFFECT        eff;
        DIPERIODIC      periodic;
        LONG            rglDirection[2];

        if (pan < -1)       pan = -1;
        else if (pan > 1)   pan = 1;

        // Setup period effect
        periodic.dwMagnitude        = abs(force);
        periodic.lOffset            = Int(pan * 10000);
        periodic.dwPhase            = 0;
        periodic.dwPeriod           = period;

        // Setup direction
        rglDirection[0]             = 0;
        rglDirection[1]             = 0;

        // Setup effect
        eff.dwSize                  = sizeof(DIEFFECT);
        eff.dwFlags                 = DIEFF_CARTESIAN | DIEFF_OBJECTOFFSETS;
        eff.cAxes                   = 2;
        eff.rglDirection            = rglDirection;
        eff.lpEnvelope              = 0;
        eff.cbTypeSpecificParams    = sizeof(periodic);
        eff.lpvTypeSpecificParams   = &periodic;
        eff.dwDuration              = Int(duration * DI_SECONDS);

        // Set the new parameters and start the effect immediately.
        hr = pJoy->forceFeedback.pVibration->SetParameters(&eff, 
                                                            DIEP_DIRECTION |
                                                            DIEP_DURATION |
                                                            DIEP_TYPESPECIFICPARAMS |
                                                            DIEP_START
                                                            );

        if (FAILED(hr)) 
            hr = hr;
    }

    return hr;
}


/////////////////////////////
// Function: SetJoyConstant
//
// force        -1 to +1
// dur          In Seconds.
/////////////////////////////
void SetJoyConstant(JOYSTICK* pJoy, REAL force, REAL dur)
{
    HRESULT         hr;
    DIEFFECT        eff;
    DWORD           rgdwAxes[2];
    DICONSTANTFORCE cf = {0};
    LONG            rglDirection[2];

    if (!pJoy->forceFeedback.pConstant)
        return;

    force *= 10000;

    rgdwAxes[0]                 = DIJOFS_X;
    rgdwAxes[1]                 = DIJOFS_Y;

    rglDirection[0]             = Int(force);
    rglDirection[1]             = 0;

    cf.lMagnitude               = Int(abs(force));
    eff.dwSize                  = sizeof(DIEFFECT);
    eff.dwFlags                 = DIEFF_CARTESIAN | DIEFF_OBJECTOFFSETS;
    eff.cAxes                   = 2;
    eff.rglDirection            = rglDirection;
    eff.lpEnvelope              = 0;
    eff.cbTypeSpecificParams    = sizeof(DICONSTANTFORCE);
    eff.lpvTypeSpecificParams   = &cf;
    eff.dwDuration              = Int(dur * DI_SECONDS);

    hr = pJoy->forceFeedback.pConstant->SetParameters(&eff, 
                                                        DIEP_DIRECTION |
                                                        DIEP_DURATION |
                                                        DIEP_TYPESPECIFICPARAMS |
                                                        DIEP_START );
}


/////////////////////////////
// Function: SetJoyFriction
//
/////////////////////////////
void SetJoyFriction(JOYSTICK* pJoy, REAL friction)
{
// Friction effect
    HRESULT     hr;
    DIEFFECT    eff;
    DICONDITION condition;

    if (!pJoy->forceFeedback.pVibration)
        return;

    condition.lOffset               = 0;
    condition.lPositiveCoefficient  = Int(friction * 10000);
    condition.lNegativeCoefficient  = Int(friction * 10000);
    condition.dwNegativeSaturation  = 10000;
    condition.dwNegativeSaturation  = 10000;
    condition.lDeadBand             = 0;

    eff.dwSize                  = sizeof(DIEFFECT);
    eff.cbTypeSpecificParams    = sizeof(condition);
    eff.lpvTypeSpecificParams   = &condition;
    eff.dwDuration              = INFINITE;
    eff.dwFlags                 = DIEFF_CARTESIAN | DIEFF_OBJECTOFFSETS;
    eff.cAxes                   = 2;
    eff.lpEnvelope              = 0;

    hr = pJoy->forceFeedback.pFriction->SetParameters(&eff, 
                                                        DIEP_TYPESPECIFICPARAMS |
                                                        DIEP_DURATION |
//                                                      DIEP_DIRECTION |
                                                        DIEP_START );
}
$END_REMOVAL */

