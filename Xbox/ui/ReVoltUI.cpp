//-----------------------------------------------------------------------------
// File: ReVoltUI.cpp
//
// Desc: Illustrates example UI menuing system for Re-Volt.
//
// Hist: 01.29.02 - Created 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "ReVoltUI.h"
#include "MenuItem.h"




//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------

enum // Menu IDs
{
    MENU_MAIN,
    MENU_ACCOUNT,
    MENU_PASSCODE,
    MENU_OPTIONS,
    MENU_SELECT_TRACK,
    MENU_NOT_IMPL
};

enum // Menu Item IDs
{
    MAIN_PLAYLIVE,
    MAIN_SINGLE_PLAYER,
    MAIN_MULTIPLAYER,
    MAIN_SYSTEM_LINK,
    MAIN_OPTIONS,

    CREATE_ACCOUNT,

    BEST_TRIAL_TIMES,
    GAME_SETTINGS,
    VIDEO_SETTINGS,
    AUDIO_SETTINGS,
    CONTROLLER_SETTINGS,
    VIEW_CREDITS,
    VIEW_GALLERY,
};

const DWORD USER_ACCOUNT = 1024;




//-----------------------------------------------------------------------------
// Structs
//-----------------------------------------------------------------------------
struct RevoltMenuItem
{
    WCHAR* str;
    DWORD  id;
};




//-----------------------------------------------------------------------------
// Name: main()
// Desc: Entry point to the program
//-----------------------------------------------------------------------------
VOID __cdecl main()
{
    CXBoxSample xbApp;
    if( FAILED( xbApp.Create() ) )
        return;
    xbApp.Run();
}




//-----------------------------------------------------------------------------
// Name: CXBoxSample()
// Desc: Constructor
//-----------------------------------------------------------------------------
CXBoxSample::CXBoxSample()
:
    CXBApplication(),
    m_UI( m_strFrameRate )
{
}




//-----------------------------------------------------------------------------
// Name: Initialize()
// Desc: Initialize device-dependant objects
//-----------------------------------------------------------------------------
HRESULT CXBoxSample::Initialize()
{
    if( !m_UI.Initialize() )
        return E_FAIL;

    BeginMain();
    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: FrameMove()
// Desc: Called once per frame, the call is the entry point for animating
//       the scene.
//-----------------------------------------------------------------------------
HRESULT CXBoxSample::FrameMove()
{
    Event ev = GetEvent();

    switch( m_Menu.top().GetID() )
    {
    case MENU_PASSCODE: UpdateStatePassCode( ev ); break;
    case MENU_ACCOUNT:  UpdateStateAccount( ev ); break;
    case MENU_NOT_IMPL: UpdateStateNotImpl( ev ); break;
    default:            UpdateStateMenu( ev ); break;
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: Render()
// Desc: Called once per frame, the call is the entry point for 3D
//       rendering. This function sets up render states, clears the
//       viewport, and renders the scene.
//-----------------------------------------------------------------------------
HRESULT CXBoxSample::Render()
{
    // Clear the viewport
    m_pd3dDevice->Clear( 0L, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER|D3DCLEAR_STENCIL, 
                         0x000A0A6A, 1.0f, 0L );
    
    switch( m_Menu.top().GetID() )
    {
    case MENU_PASSCODE: m_UI.RenderPassCode( m_Menu.top(), m_PinEntry ); break;
    case MENU_NOT_IMPL: m_UI.RenderNotImpl( m_Menu.top() );  break;
    default:            m_UI.RenderMenu( m_Menu.top() ); break;
    }
    
    // Present the scene
    m_pd3dDevice->Present( NULL, NULL, NULL, NULL );
    
    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: GetEvent()
// Desc: Return the state of the controller
//-----------------------------------------------------------------------------
CXBoxSample::Event CXBoxSample::GetEvent() const
{
    // "A" or "Start"
    if( m_DefaultGamepad.bPressedAnalogButtons[ XINPUT_GAMEPAD_A ] ||
        m_DefaultGamepad.wPressedButtons & XINPUT_GAMEPAD_START )
    {
        return EV_BUTTON_A;
    }
    
    // "B"
    if( m_DefaultGamepad.bPressedAnalogButtons[ XINPUT_GAMEPAD_B ] )
        return EV_BUTTON_B;
    
    // "Back"
    if( m_DefaultGamepad.wPressedButtons & XINPUT_GAMEPAD_BACK )
        return EV_BUTTON_BACK;
    
    // Movement
    if( m_DefaultGamepad.wPressedButtons & XINPUT_GAMEPAD_DPAD_UP )
        return EV_UP;
    if( m_DefaultGamepad.wPressedButtons & XINPUT_GAMEPAD_DPAD_DOWN )
        return EV_DOWN;
    
    return EV_NULL;
}




//-----------------------------------------------------------------------------
// Name: BeginMain()
// Desc: Begin main menu
//-----------------------------------------------------------------------------
VOID CXBoxSample::BeginMain()
{
    m_Menu.push( Menu( MENU_MAIN, L"Re-Volt" ) );

    RevoltMenuItem items[] =
    {
        { L"Play Live!",    MAIN_PLAYLIVE      },
        { L"Single Player", MAIN_SINGLE_PLAYER },
        { L"Multiplayer",   MAIN_MULTIPLAYER   },
        { L"System Link",   MAIN_SYSTEM_LINK   },
        { L"Options",       MAIN_OPTIONS       },
        { NULL },
    };

    // Build main menu
    for( DWORD i = 0; items[i].str; ++i )
        m_Menu.top().AddItem( MenuItem( items[i].str, items[i].id ) );

    m_Menu.top().SetCurrByID( MAIN_PLAYLIVE );
}




//-----------------------------------------------------------------------------
// Name: BeginAccount()
// Desc: Begin user account menu
//-----------------------------------------------------------------------------
VOID CXBoxSample::BeginAccount()
{
    m_Menu.push( Menu( MENU_ACCOUNT, L"Select Account" ) );

    // TODO Account code would invoke XOnline API to build this list

    RevoltMenuItem items[] =
    {
        { L"LightSleeper",       USER_ACCOUNT+0 },
        { L"Bob",                USER_ACCOUNT+1 },
        { L"Timmmay",            USER_ACCOUNT+2 },
        { L"Bubba",              USER_ACCOUNT+3 },
        { L"Your mom",           USER_ACCOUNT+4 },
        { L"Create New Account", CREATE_ACCOUNT },
        { NULL },
    };

    // Build list of accounts
    for( DWORD i = 0; items[i].str; ++i )
        m_Menu.top().AddItem( MenuItem( items[i].str, items[i].id ) );

    m_Menu.top().SetCurrByID( USER_ACCOUNT );
}




//-----------------------------------------------------------------------------
// Name: BeginAccountNone()
// Desc: Show what would happen if there were no accounts
//-----------------------------------------------------------------------------
VOID CXBoxSample::BeginAccountNone()
{
    // Assume no accounts found ...

    BeginSelectTrack();
}




//-----------------------------------------------------------------------------
// Name: BeginOptions()
// Desc: Begin options menu
//-----------------------------------------------------------------------------
VOID CXBoxSample::BeginOptions()
{
    m_Menu.push( Menu( MENU_OPTIONS, L"Options" ) );

    RevoltMenuItem items[] =
    {
        { L"Best Trial Times",    BEST_TRIAL_TIMES    },
        { L"Game Settings",       GAME_SETTINGS       },
        { L"Video Settings",      VIDEO_SETTINGS      },
        { L"Audio Settings",      AUDIO_SETTINGS      },
        { L"Controller Settings", CONTROLLER_SETTINGS },
        { L"View Credits",        VIEW_CREDITS        },
        { L"View Gallery",        VIEW_GALLERY        },
        { NULL },
    };

    // Build list of accounts
    for( DWORD i = 0; items[i].str; ++i )
        m_Menu.top().AddItem( MenuItem( items[i].str, items[i].id ) );

    m_Menu.top().SetCurrByID( BEST_TRIAL_TIMES );
}




//-----------------------------------------------------------------------------
// Name: BeginPassCode()
// Desc: Begin user pass code entry
//-----------------------------------------------------------------------------
VOID CXBoxSample::BeginPassCode()
{
    WCHAR strTitle[ 256 + XONLINE_USERNAME_SIZE ];
    wsprintfW( strTitle, L"Enter Pass Code\n"
                         L"'%.*s'", XONLINE_USERNAME_SIZE,
                         m_Menu.top().GetCurrItem().GetText() );
    m_Menu.push( Menu( MENU_PASSCODE, strTitle ) );
}




//-----------------------------------------------------------------------------
// Name: BeginCreateAccount()
// Desc: Begin user account creation
//-----------------------------------------------------------------------------
VOID CXBoxSample::BeginCreateAccount()
{
    m_Menu.push( Menu( MENU_NOT_IMPL, L"Account creation not implemented\nPress B to Cancel" ) );
}




//-----------------------------------------------------------------------------
// Name: BeginSignOn()
// Desc: Begin user sign on process
//-----------------------------------------------------------------------------
VOID CXBoxSample::BeginSignOn()
{
    m_Menu.push( Menu( MENU_NOT_IMPL, L"Sign On not implemented\nPress B to Cancel" ) );
}

VOID CXBoxSample::BeginBestTrialTimes()
{
    m_Menu.push( Menu( MENU_NOT_IMPL, L"Not implemented\nPress B to Cancel" ) );
}
VOID CXBoxSample::BeginGameSettings()
{
    m_Menu.push( Menu( MENU_NOT_IMPL, L"Not implemented\nPress B to Cancel" ) );
}
VOID CXBoxSample::BeginVideoSettings()
{
    m_Menu.push( Menu( MENU_NOT_IMPL, L"Not implemented\nPress B to Cancel" ) );
}
VOID CXBoxSample::BeginAudioSettings()
{
    m_Menu.push( Menu( MENU_NOT_IMPL, L"Not implemented\nPress B to Cancel" ) );
}
VOID CXBoxSample::BeginControllerSettings()
{
    m_Menu.push( Menu( MENU_NOT_IMPL, L"Not implemented\nPress B to Cancel" ) );
}
VOID CXBoxSample::BeginViewCredits()
{
    m_Menu.push( Menu( MENU_NOT_IMPL, L"Not implemented\nPress B to Cancel" ) );
}
VOID CXBoxSample::BeginViewGallery()
{
    m_Menu.push( Menu( MENU_NOT_IMPL, L"Not implemented\nPress B to Cancel" ) );
}
VOID CXBoxSample::BeginSelectTrack()
{
    m_Menu.push( Menu( MENU_NOT_IMPL, L"Select Track not implemented\nPress B to Cancel" ) );
}

//-----------------------------------------------------------------------------
// Name: UpdateStateMenu()
// Desc: Menu state handler
//-----------------------------------------------------------------------------
VOID CXBoxSample::UpdateStateMenu( Event ev )
{
    switch( ev )
    {
    case EV_BUTTON_A: // Menu item selected
    {
        // Extract the menu ID
        DWORD dwID = m_Menu.top().GetCurrItem().GetID();

        switch( dwID )
        {
            case MAIN_PLAYLIVE:       BeginAccount(); break;
            case MAIN_SINGLE_PLAYER:  BeginAccount(); break;
            case MAIN_MULTIPLAYER:    BeginAccountNone(); break;
            case MAIN_SYSTEM_LINK:    BeginAccount(); break;
            case MAIN_OPTIONS:        BeginOptions(); break;
            case CREATE_ACCOUNT:      BeginCreateAccount();  break;
            case BEST_TRIAL_TIMES:    BeginBestTrialTimes(); break;
            case GAME_SETTINGS:       BeginGameSettings();   break;
            case VIDEO_SETTINGS:      BeginVideoSettings();  break;
            case AUDIO_SETTINGS:      BeginAudioSettings();  break;
            case CONTROLLER_SETTINGS: BeginControllerSettings(); break;
            case VIEW_CREDITS:        BeginViewCredits();    break;
            case VIEW_GALLERY:        BeginViewGallery();    break;
        }

        if( dwID >= USER_ACCOUNT )
            BeginPassCode();

        break;
    }
    case EV_BUTTON_B:
        if( m_Menu.size() > 1 )
            m_Menu.pop();
        break;
    case EV_UP:
        m_Menu.top().MoveUp();
        break;
    case EV_DOWN:
        m_Menu.top().MoveDown();
        break;
    }
}





//-----------------------------------------------------------------------------
// Name: UpdateStateAccount()
// Desc: User account menu state handler
//-----------------------------------------------------------------------------
VOID CXBoxSample::UpdateStateAccount( Event ev )
{
    // TODO Final code would invoke XOnline API to update the account list
    // as MUs were inserted/removed
    UpdateStateMenu( ev );
}




//-----------------------------------------------------------------------------
// Name: UpdateStatePassCode()
// Desc: User pass code entry handler
//-----------------------------------------------------------------------------
VOID CXBoxSample::UpdateStatePassCode( Event ev )
{
    // TODO Final code would build up the pass code like the Auth sample
    switch( ev )
    {
    case EV_BUTTON_A:
        BeginSignOn();
        break;
    case EV_BUTTON_B:
        if( m_Menu.size() > 1 )
            m_Menu.pop();
        break;
    }
}




//-----------------------------------------------------------------------------
// Name: UpdateStateNotImpl()
// Desc: State hasn't been implemented yet
//-----------------------------------------------------------------------------
VOID CXBoxSample::UpdateStateNotImpl( Event ev )
{
    // TODO Shouldn't appear in final code
    switch( ev )
    {
    case EV_BUTTON_B:
        if( m_Menu.size() > 1 )
            m_Menu.pop();
        break;
    }
}




