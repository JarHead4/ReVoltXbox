//-----------------------------------------------------------------------------
// File: ReVoltUI.h
//
// Desc: Illustrates Re-Volt UI.
//
// Hist: 01.29.02 - Created
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef REVOLTUI_H
#define REVOLTUI_H

#include "Common.h"
#include "XbApp.h"
#include "XbOnline.h"
#include "Menu.h"
#include "UserInterface.h"
#include <stack>




//-----------------------------------------------------------------------------
// Name: class CXBoxSample
// Desc: Main class to run this application. Most functionality is inherited
//       from the CXBApplication base class.
//-----------------------------------------------------------------------------
class CXBoxSample : public CXBApplication
{
    enum Event
    {
        EV_BUTTON_A,
        EV_BUTTON_B,
        EV_BUTTON_BACK,
        EV_UP,
        EV_DOWN,
        EV_NULL
    };

    UserInterface m_UI; // rendering functions
    std::stack< Menu > m_Menu;
    CXBPinEntry m_PinEntry;

public:

    virtual HRESULT Initialize();
    virtual HRESULT FrameMove();
    virtual HRESULT Render();

    CXBoxSample();

private:

    Event GetEvent() const;

    VOID BeginMain();
    VOID BeginAccount();
    VOID BeginAccountNone();
    VOID BeginOptions();
    VOID BeginPassCode();
    VOID BeginCreateAccount();
    VOID BeginSignOn();
    VOID BeginBestTrialTimes();
    VOID BeginGameSettings();
    VOID BeginVideoSettings();
    VOID BeginAudioSettings();
    VOID BeginControllerSettings();
    VOID BeginViewCredits();
    VOID BeginViewGallery();
    VOID BeginSelectTrack();

    VOID MoveUp();
    VOID MoveDown();

    VOID UpdateStateMenu( Event );
    VOID UpdateStateAccount( Event );
    VOID UpdateStatePassCode( Event );
    VOID UpdateStateNotImpl( Event );

};




#endif // REVOLTUI_H
