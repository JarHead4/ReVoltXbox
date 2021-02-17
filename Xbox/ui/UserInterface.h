//-----------------------------------------------------------------------------
// File: UserInterface.h
//
// Desc: ReVoltUI rendering functions
//
// Hist: 01.29.02 - Created
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef REVOLTUI_UI_H
#define REVOLTUI_UI_H

#include "Common.h"
#include "XbResource.h"
#include "XbFont.h"
#include "XbHelp.h"
#include "XbStopWatch.h"

// Forward declarations
class Menu;
class CXBPinEntry;




//-----------------------------------------------------------------------------
// Name: class UserInterface
// Desc: User interface
//-----------------------------------------------------------------------------
class UserInterface
{

    CXBPackedResource   m_xprResource;               // app resources
    mutable CXBFont     m_Font;                      // game font
    CXBHelp             m_Help;                      // help screen
    LPDIRECT3DTEXTURE8  m_ptMenuSel;                 // menu selection image
    WCHAR*              m_strFrameRate;              // from CXBApp
    WCHAR*              m_strHeader;                 // header string
    CXBStopWatch        m_CaretTimer;                // for PIN entry

public:

    UserInterface( WCHAR* strFrameRate );

    BOOL Initialize();

    VOID RenderMenu( const Menu& menu ) const;
    VOID RenderPassCode( const Menu& menu, const CXBPinEntry& PinEntry ) const;
    VOID RenderNotImpl( const Menu& menu ) const;

private:

    VOID RenderHeader() const;
    VOID RenderMenuSelector( FLOAT fLeft, FLOAT fTop ) const;
    BOOL DrawText( FLOAT sx, FLOAT sy, DWORD dwColor, 
                   const WCHAR* strText, DWORD dwFlags ) const;

private:

    // Disabled
    UserInterface();
    UserInterface( const UserInterface& );
    UserInterface& operator=( const UserInterface& );

};

#endif // REVOLTUI_UI_H
