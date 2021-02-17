//-----------------------------------------------------------------------------
// File: Gamepad.cpp
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "gamepad.h"



// globals

GamepadState g_Gamepads[4];




//-----------------------------------------------------------------------------
// Name: InitGamepads()
// Desc: Initializes the gamepad devices.
//-----------------------------------------------------------------------------
void InitGamepads()
{
    // Get a mask of all currently available devices
    DWORD dwDeviceMask = XGetDevices( XDEVICE_TYPE_GAMEPAD );

    // Open the devices
    for( DWORD i=0; i < XGetPortCount(); i++ )
    {
        ZeroMemory( &g_Gamepads[i], sizeof(GamepadState) );
        if( dwDeviceMask & (1<<i) ) 
        {
            // Get a handle to the device  <<TCR 1-14 Device Types>>
            g_Gamepads[i].hDevice = XInputOpen( XDEVICE_TYPE_GAMEPAD, i, 
                                                XDEVICE_NO_SLOT, NULL );
        }
    }
}

//-----------------------------------------------------------------------------
// Name: UnInitGamepads()
// Desc: De-initializes the gamepad devices.
//-----------------------------------------------------------------------------
void UnInitGamepads()
{
//$CMP_NOTE: do we really need this function, or can we kill it ???
    // Loop through all gamepads
    for( DWORD i=0; i < XGetPortCount(); i++ )
    {
        if( NULL != g_Gamepads[i].hDevice )
        {
            XInputClose( g_Gamepads[i].hDevice );
            g_Gamepads[i].hDevice = NULL;
        }
    }
}

//-----------------------------------------------------------------------------
// Name: ReadGamepads()
// Desc: Processes input from the gamepad devices.
//-----------------------------------------------------------------------------
void ReadGamepads()
{
    // TCR 3-21 Controller Discovery
    // Get status about gamepad insertions and removals. Note that, in order
    // to not miss devices, we will check for removed device BEFORE checking
    // for insertions.
    DWORD dwInsertions, dwRemovals;
    XGetDeviceChanges( XDEVICE_TYPE_GAMEPAD, &dwInsertions, &dwRemovals );

    // Loop through all gamepads
    for( DWORD i=0; i < XGetPortCount(); i++ )
    {
        // Handle removed devices.
        if( dwRemovals & (1<<i) )
        {
            XInputClose( g_Gamepads[i].hDevice );
            g_Gamepads[i].hDevice = NULL;
        }

        // Handle inserted devices
        if( dwInsertions & (1<<i) ) 
        {
            // TCR 1-14 Device Types
            g_Gamepads[i].hDevice = XInputOpen( XDEVICE_TYPE_GAMEPAD, i, 
                                               XDEVICE_NO_SLOT, NULL );
        }

        // If we have a valid device, poll its state
        if( NULL != g_Gamepads[i].hDevice )
        {
            // Read the input state
            XINPUT_STATE xiState;
            XInputGetState( g_Gamepads[i].hDevice, &xiState );
            
//$REMOVED
//            // Clamp values
//            // (change min from -32768 to -32767, so min/max have same absolute value)
//            // (another option is to use floating-point, and add +0.5f to value read)
//            if( xiState.sThumbLX < -32767 )  { xiState.sThumbLX = -32767; }
//            if( xiState.sThumbLY < -32767 )  { xiState.sThumbLY = -32767; }
//            if( xiState.sThumbRX < -32767 )  { xiState.sThumbRX = -32767; }
//            if( xiState.sThumbRY < -32767 )  { xiState.sThumbRY = -32767; }

            // Copy gamepad to local structure
            memcpy( &g_Gamepads[i].status, &xiState.Gamepad, sizeof(XINPUT_GAMEPAD) );

        }
    }
}

