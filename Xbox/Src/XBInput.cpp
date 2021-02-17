//-----------------------------------------------------------------------------
// File: XBInput.cpp
//
// Desc: Gathers input from Xbox controllers
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "XBInput.h"
#include "net_xonline.h"
#include "revolt.h"
#include "ui_showmessage.h"
#include "ui_ingamemenu.h"
#include "network.h"


// External time step from the app
extern FLOAT TimeStep; 

// Global variable containing state for all controllers (max of 4)
XBINPUT_CONTROLLER g_Controllers[4];

// Global flag that indicates whether any input occured on any gamepad
BOOL g_bAnyButtonPressed = FALSE;

// Global flag to control thumbstick behavior
BOOL g_bMapThumbstickToDpadControls = TRUE;

// Deadzone for analog thumbsticks
const FLOAT XBINPUT_DEADZONE = 0.24f;




//-----------------------------------------------------------------------------
// Name: GetControllerType()
// Desc: Checks a controller's caps and returns the type of device it is.
//-----------------------------------------------------------------------------
DWORD GetControllerType( HANDLE hDevice )
{
    // Determine the controller type, by checking the device capabilities
    XINPUT_CAPABILITIES caps;
    XInputGetCapabilities( hDevice, &caps );

    // Verify the device type is something we support
    switch( caps.SubType )
    {
        case XINPUT_DEVSUBTYPE_GC_GAMEPAD:      // US Gamepad
        case XINPUT_DEVSUBTYPE_GC_GAMEPAD_ALT:  // Japan gamepad
            return XBINPUT_CONTROLLER_GAMEPAD;

        case XINPUT_DEVSUBTYPE_GC_WHEEL:        // Steering wheel
            return XBINPUT_CONTROLLER_WHEEL;

        case XINPUT_DEVSUBTYPE_GC_ARCADE_STICK: // Arcade stick
            return XBINPUT_CONTROLLER_ARCADESTICK;

        case XINPUT_DEVSUBTYPE_GC_DIGITAL_ARCADE_STICK:
        case XINPUT_DEVSUBTYPE_GC_FLIGHT_STICK:
        case XINPUT_DEVSUBTYPE_GC_SNOWBOARD:
            // Device type is treated as a gamepad
            return XBINPUT_CONTROLLER_GAMEPAD;
    }

    return XBINPUT_CONTROLLER_UNSUPPORTED;
}




//-----------------------------------------------------------------------------
// Name: XBInput_InitControllers()
// Desc: Initializes the controllers.
//-----------------------------------------------------------------------------
HRESULT XBInput_InitControllers()
{
    // Initialize core peripheral port support. Note: If these parameters
    // are 0 and NULL, respectively, then the default number and types of 
    // controllers will be initialized.
    XInitDevices( 0, NULL );

    // Get a mask of all currently available gamepad-type devices. Note that
    // this includes steering wheels (which we do care about) and other device
    // types (that we don't care about).
    DWORD dwDeviceMask = XGetDevices( XDEVICE_TYPE_GAMEPAD );

    // Iniatialize all the controllers
    ZeroMemory( g_Controllers, sizeof(g_Controllers) );

    for( DWORD i = 0; i < XGetPortCount(); i++ )
    {
        // Initialize the controller, if present
        if( dwDeviceMask & (1<<i) ) 
        {
            // Get a handle to the device  <<TCR 1-14 Device Types>>
            g_Controllers[i].hDevice = XInputOpen( XDEVICE_TYPE_GAMEPAD, i, 
                                                   XDEVICE_NO_SLOT, NULL );
            
            // Determine the controller type, by checking the device capabilities
            g_Controllers[i].dwDeviceType = GetControllerType( g_Controllers[i].hDevice );
        }
    }

    // Return OK for now. Could return FAIL, to detect no controllers are
    // inserted, or maybe return a DWORD of the # of inserted controllers
    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: XBInput_ReadControllers()
// Desc: Processes input from the input controller devices.
//-----------------------------------------------------------------------------
HRESULT XBInput_ReadControllers()
{
    // TCR 3-21 Controller Discovery
    // Get status about gamepad insertions and removals. Note that, in order
    // to not miss devices, we will check for removed device BEFORE checking
    // for insertions.
    DWORD dwInsertions, dwRemovals;
    XGetDeviceChanges( XDEVICE_TYPE_GAMEPAD, &dwInsertions, &dwRemovals );

    // Loop through all controller ports
    for( DWORD i=0; i < XGetPortCount(); i++ )
    {
        // Convenient access to the i'th controller
        XBINPUT_CONTROLLER* pController = &g_Controllers[i];

        // If the device was just removed, close it
        if( dwRemovals & (1<<i) )
        {
            XInputClose( pController->hDevice );
            ZeroMemory( &g_Controllers[i], sizeof(g_Controllers[i]) );
        }

        // If the device was just inserted, initialize it.
        if( dwInsertions & (1<<i) ) 
        {
            // Get a handle to the device  <<TCR 1-14 Device Types>>
            pController->hDevice = XInputOpen( XDEVICE_TYPE_GAMEPAD, i, 
                                               XDEVICE_NO_SLOT, NULL );
            
            // Determine the controller type, by checking the device capabilities
            pController->dwDeviceType = GetControllerType( pController->hDevice );

            // Reset the input values
            pController->dwMenuInput     = XBINPUT_NONE;
            pController->dwLastMenuInput = XBINPUT_NONE;
        }

        // If we have a valid device, poll its state
        if( XBINPUT_CONTROLLER_UNSUPPORTED != pController->dwDeviceType )
        {
            // Read the input state for the controller
            XInputGetState( pController->hDevice, (XINPUT_STATE*)&g_Controllers[i] );

            // Put Xbox device input for the gamepad into our custom format
            FLOAT fX1 = (pController->sThumbLX+0.5f)/32767.5f;
            pController->fX1 = ( fX1 >= 0.0f ? 1.0f : -1.0f ) *
                                     max( 0.0f, (fabsf(fX1)-XBINPUT_DEADZONE)/(1.0f-XBINPUT_DEADZONE) );

            FLOAT fY1 = (pController->sThumbLY+0.5f)/32767.5f;
            pController->fY1 = ( fY1 >= 0.0f ? 1.0f : -1.0f ) *
                                     max( 0.0f, (fabsf(fY1)-XBINPUT_DEADZONE)/(1.0f-XBINPUT_DEADZONE) );

            FLOAT fX2 = (pController->sThumbRX+0.5f)/32767.5f;
            pController->fX2 = ( fX2 >= 0.0f ? 1.0f : -1.0f ) *
                                     max( 0.0f, (fabsf(fX2)-XBINPUT_DEADZONE)/(1.0f-XBINPUT_DEADZONE) );

            FLOAT fY2 = (pController->sThumbRY+0.5f)/32767.5f;
            pController->fY2 = ( fY2 >= 0.0f ? 1.0f : -1.0f ) *
                                     max( 0.0f, (fabsf(fY2)-XBINPUT_DEADZONE)/(1.0f-XBINPUT_DEADZONE) );

            // Get the boolean buttons that have been pressed since the last
            // call. Each button is represented by one bit.
            pController->wPressedButtons = ( pController->wLastButtons ^ pController->wButtons ) & pController->wButtons;
            pController->wLastButtons    = pController->wButtons;

            // Get the analog buttons that have been pressed or released since
            // the last call.
            for( DWORD b=0; b<8; b++ )
            {
                // Turn the 8-bit polled value into a boolean value
                BOOL bPressed = ( pController->bAnalogButtons[b] > XINPUT_GAMEPAD_MAX_CROSSTALK );

                if( bPressed )
                    pController->bPressedAnalogButtons[b] = !pController->bLastAnalogButtons[b];
                else
                    pController->bPressedAnalogButtons[b] = FALSE;
                
                // Store the current state for the next time
                pController->bLastAnalogButtons[b] = bPressed;
            }
        }

        // Initiate  in game menu, ensure no parent menu present
        if( IsLoggedIn( 0 ) && 
            i == g_dwSignedInController && 
            g_Controllers[i].hDevice == NULL )
        {
            // If we're not in the right state, that means
            // either this is the first time through, or 
            // someone whacked the state engine from underneath us
            if( g_pActiveStateEngine != &g_ControllerRemoved ||
                ( g_pMenuHeader->m_pMenu != &Menu_Message &&
                  g_pMenuHeader->m_pNextMenu != &Menu_Message ) )
            {
                if( IsInGameSession() && bGameStarted )
                {
                    g_pMenuHeader->ClearMenuHeader();
                    g_InGameMenuStateEngine.MakeActive(NULL);
                }
                g_ControllerRemoved.Begin( i );
            }
        }
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: XBInput_GetInput()
// Desc: 
//-----------------------------------------------------------------------------
VOID XBInput_GetInput()
{
    // Clear the global flag that indicates whether any input occured on any
    // gamepad
    g_bAnyButtonPressed = FALSE;

    // Read input from the controller
    XBInput_ReadControllers();

    for( DWORD i = 0; i < 4; i++ )
    {
        // Once they're signed in, that's their controller.
        if( IsLoggedIn( 0 ) && i != g_dwSignedInController )
        {
            g_Controllers[i].dwLastMenuInput = XBINPUT_NONE;
            g_Controllers[i].dwMenuInput = XBINPUT_NONE;
            ZeroMemory( &g_Controllers[i], sizeof(XINPUT_STATE) );
            continue;
        }

        // Convenient access to the i'th controller
        XBINPUT_CONTROLLER* pController = &g_Controllers[i];

        // Determine menu input
        long input = XBINPUT_NONE;

        if( g_bMapThumbstickToDpadControls )
        {
            // Use gamepad left thumbstick to mimic DPad
            if( pController->dwDeviceType == XBINPUT_CONTROLLER_GAMEPAD )
            {
                // Check thumbstick values for menu input
                if( pController->fY1 > +XBINPUT_DEADZONE )   input = XBINPUT_UP;
                if( pController->fY1 < -XBINPUT_DEADZONE )   input = XBINPUT_DOWN;
                if( pController->fX1 < -XBINPUT_DEADZONE )   input = XBINPUT_LEFT;
                if( pController->fX1 > +XBINPUT_DEADZONE )   input = XBINPUT_RIGHT;
            }

            // Use steering wheel to mimic DPad
            if( pController->dwDeviceType == XBINPUT_CONTROLLER_WHEEL )
            {
                // Check wheel values for menu input
                if( pController->fX1 < -XBINPUT_DEADZONE )   input = XBINPUT_UP;
                if( pController->fX1 > +XBINPUT_DEADZONE )   input = XBINPUT_DOWN;
            }
        }

        // Check DPAD values for menu input
        if( pController->wButtons & XINPUT_GAMEPAD_DPAD_UP )      input = XBINPUT_UP;
        if( pController->wButtons & XINPUT_GAMEPAD_DPAD_DOWN )    input = XBINPUT_DOWN;
        if( pController->wButtons & XINPUT_GAMEPAD_DPAD_LEFT )    input = XBINPUT_LEFT;
        if( pController->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT )   input = XBINPUT_RIGHT;

        // Check other buttons
        if ( pController->bAnalogButtons[XINPUT_GAMEPAD_LEFT_TRIGGER] > XINPUT_GAMEPAD_MAX_CROSSTALK )
            input = XBINPUT_LEFT_TRIGGER;
        if( pController->bAnalogButtons[XINPUT_GAMEPAD_RIGHT_TRIGGER] > XINPUT_GAMEPAD_MAX_CROSSTALK )
            input = XBINPUT_RIGHT_TRIGGER;
        if ( pController->bAnalogButtons[XINPUT_GAMEPAD_WHITE] > XINPUT_GAMEPAD_MAX_CROSSTALK )
            input = XBINPUT_WHITE_BUTTON;
        if( pController->bAnalogButtons[XINPUT_GAMEPAD_BLACK] > XINPUT_GAMEPAD_MAX_CROSSTALK )
            input = XBINPUT_BLACK_BUTTON;
        if ( pController->bAnalogButtons[XINPUT_GAMEPAD_X] > XINPUT_GAMEPAD_MAX_CROSSTALK )
            input = XBINPUT_X_BUTTON;
        if( pController->bAnalogButtons[XINPUT_GAMEPAD_Y] > XINPUT_GAMEPAD_MAX_CROSSTALK )
            input = XBINPUT_Y_BUTTON;
        if ( pController->bAnalogButtons[XINPUT_GAMEPAD_B] > XINPUT_GAMEPAD_MAX_CROSSTALK )
            input = XBINPUT_B_BUTTON;
        if( pController->bAnalogButtons[XINPUT_GAMEPAD_A] > XINPUT_GAMEPAD_MAX_CROSSTALK )
            input = XBINPUT_A_BUTTON;

        // Check START and BACK buttons
        if( pController->wButtons & XINPUT_GAMEPAD_START )
            input = XBINPUT_START_BUTTON;
        if( pController->wButtons & XINPUT_GAMEPAD_BACK )
            input = XBINPUT_BACK_BUTTON;

        switch( input )
        {
            // Handle buttons that have auto-repeat
            case XBINPUT_UP:
            case XBINPUT_DOWN:
            case XBINPUT_LEFT:
            case XBINPUT_RIGHT:
            {
                if( input != pController->dwLastMenuInput )
                {
                    // If a button was pressed since last time, report it
                    pController->dwLastMenuInput  = input;
                    pController->fTimeUntilRepeat = XBINPUT_REPEAT_1ST_DELAY;
                    pController->dwMenuInput = input;
                }
                else if( input == XBINPUT_NONE )
                {
                    // No input to report
                    pController->fTimeUntilRepeat = 0.0f;
                    pController->dwMenuInput = XBINPUT_NONE;
                }
                else
                {
                    // Countdown the repeat timer
                    pController->fTimeUntilRepeat -= TimeStep;

                    if( pController->fTimeUntilRepeat < 0.0f )
                    {
                        // Button is held down long enough, so repeat it
                        pController->fTimeUntilRepeat += XBINPUT_REPEAT_NEXT_DELAY;
                        pController->dwMenuInput = input;
                    }
                    else
                    {
                        // Button is held down, but not long enough to repeat yet
                        pController->dwMenuInput = XBINPUT_NONE;
                    }
                }
                break;
            }

            // Handle non-repeating buttons
            default:
            {
                if( input != pController->dwLastMenuInput )
                {
                    // If a button was pressed since last time, report it
                    pController->dwLastMenuInput = input;
                    pController->dwMenuInput = input;
                }
                else
                {
                    // Either there's no input, or the button did not change
                    // (i.e. the button is held down) so don't report it
                    pController->dwMenuInput = XBINPUT_NONE;
                }
                break;
            }
        }

        // If any input occurred on any joystick, set a global flag
        if( input != XBINPUT_NONE )
            g_bAnyButtonPressed = TRUE;
    }
}



