//-----------------------------------------------------------------------------
// File: UserInterface.cpp
//
// Desc: ReVoltUI
//
// Hist: 01.29.02 - Created
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "UserInterface.h"
#include "Resource.h"
#include "XbApp.h"
#include "XbOnline.h"
#include "Menu.h"
#include "MenuItem.h"




//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------
const D3DCOLOR COLOR_HIGHLIGHT = 0xffffff00; // Yellow
const D3DCOLOR COLOR_GREEN     = 0xff00ff00; // Green
const D3DCOLOR COLOR_NORMAL    = 0xffffffff; // White

// How often (per second) the caret blinks during PIN entry
const FLOAT fCARET_BLINK_RATE = 1.0f;

// During the blink period, the amount the caret is visible. 0.5 equals
// half the time, 0.75 equals 3/4ths of the time, etc.
const FLOAT fCARET_ON_RATIO = 0.75f;




//-----------------------------------------------------------------------------
// Name: UserInterface()
// Desc: Constructor
//-----------------------------------------------------------------------------
UserInterface::UserInterface( WCHAR* strFrameRate )
:
    m_strHeader( L"ReVoltUI" ),
    m_strFrameRate( strFrameRate ),
    m_CaretTimer( TRUE )
{
    m_ptMenuSel = NULL;
}




//-----------------------------------------------------------------------------
// Name: Initialize()
// Desc: Initialize device-dependant objects
//-----------------------------------------------------------------------------
BOOL UserInterface::Initialize()
{
    // Create a font
    if( FAILED( m_Font.Create( g_pd3dDevice, "Font.xpr" ) ) )
    {
        OutputDebugStringA( "Failed to load fonts\n" );
        return FALSE;
    }

    // Initialize the help system
    if( FAILED( m_Help.Create( g_pd3dDevice, "Gamepad.xpr" ) ) )
    {
        OutputDebugStringA( "Failed to load help\n" );
        return FALSE;
    }

    // Load our textures
    if( FAILED( m_xprResource.Create( g_pd3dDevice, "Resource.xpr", 
                                      Resource_NUM_RESOURCES ) ) )
    {
        OutputDebugStringA( "Failed to load textures\n" );
        return FALSE;
    }

    // Set up texture ptrs
    m_ptMenuSel = m_xprResource.GetTexture( Resource_MenuSelect_OFFSET );

    // Set projection transform
    D3DXMATRIX matProj;
    D3DXMatrixPerspectiveFovLH( &matProj, D3DX_PI/4, 640.0f/480.0f, 0.1f, 100.0f );
    g_pd3dDevice->SetTransform( D3DTS_PROJECTION, &matProj );

    // Set view position
    D3DXMATRIX matView;
    D3DXMatrixTranslation( &matView, 0.0f, 0.0f, 40.0f);
    g_pd3dDevice->SetTransform( D3DTS_VIEW, &matView );

    return TRUE;
}




//-----------------------------------------------------------------------------
// Name: RenderMenu()
// Desc: Display menu
//-----------------------------------------------------------------------------
VOID UserInterface::RenderMenu( const Menu& menu ) const
{
    RenderHeader();

    m_Font.DrawText( 320, 140, COLOR_NORMAL, menu.GetTitle(), 
                     XBFONT_CENTER_X );

    FLOAT fYtop = 220.0f;
    FLOAT fYdelta = 30.0f;

    // Show menu
    for( DWORD i = 0; i < menu.GetCount(); ++i )
    {
        DWORD dwColor = ( menu.GetCurrPos() == i ) ? COLOR_HIGHLIGHT : COLOR_NORMAL;

        m_Font.DrawText( 160, fYtop + (fYdelta * i), dwColor, 
                         menu.GetItem( i ).GetText() );
    }

    // Show selected item with little triangle
    RenderMenuSelector( 120.0f, fYtop + (fYdelta * menu.GetCurrPos() ) );
}




//-----------------------------------------------------------------------------
// Name: RenderPassCode()
// Desc: Display pass code entry screen
//-----------------------------------------------------------------------------
VOID UserInterface::RenderPassCode( const Menu& menu, 
                                    const CXBPinEntry& PinEntry ) const 
{
    RenderHeader();

    m_Font.DrawText( 320, 140, COLOR_NORMAL, menu.GetTitle(), 
                     XBFONT_CENTER_X );

    // Build PIN text string (stars)
    WCHAR strPIN[ XONLINE_PIN_LENGTH * 2 + 1 ] = { 0 };
    for( DWORD i = 0; i < PinEntry.GetPinLength() * 2; i += 2 )
    {
        strPIN[ i   ] = L'*';
        strPIN[ i+1 ] = L' ';
    }

    // Determine caret location
    FLOAT fWidth;
    FLOAT fHeight;
    m_Font.GetTextExtent( strPIN, &fWidth, &fHeight );

    // Display text "cursor"
    if( fmod( m_CaretTimer.GetElapsedSeconds(), fCARET_BLINK_RATE ) <
        fCARET_ON_RATIO )
    {
        m_Font.DrawText( 300.0f + fWidth - 2.0f, 300.0f, COLOR_HIGHLIGHT, L"|" );
    }

    // Display "PIN"
    m_Font.DrawText( 300, 300, COLOR_NORMAL, strPIN );
}




//-----------------------------------------------------------------------------
// Name: RenderNotImpl()
// Desc: These graphics not yet implemented
//-----------------------------------------------------------------------------
VOID UserInterface::RenderNotImpl( const Menu& menu ) const
{
    RenderHeader();

    m_Font.DrawText( 320, 140, COLOR_NORMAL, menu.GetTitle(), 
                     XBFONT_CENTER_X );
}




//-----------------------------------------------------------------------------
// Name: RenderHeader()
// Desc: Display standard text
//-----------------------------------------------------------------------------
VOID UserInterface::RenderHeader() const
{
    m_Font.DrawText(  64, 50, COLOR_NORMAL,    m_strHeader );
    m_Font.DrawText( 450, 50, COLOR_HIGHLIGHT, m_strFrameRate );
}




//-----------------------------------------------------------------------------
// Name: RenderMenuSelector()
// Desc: Display menu selector
//-----------------------------------------------------------------------------
VOID UserInterface::RenderMenuSelector( FLOAT fLeft, FLOAT fTop ) const
{
    D3DXVECTOR4 rc( fLeft, fTop, fLeft + 20.0f, fTop + 20.0f );

    // Show selected item
    struct TILEVERTEX
    {
        D3DXVECTOR4 p;
        D3DXVECTOR2 t;
    };
    TILEVERTEX* pVertices;

    LPDIRECT3DVERTEXBUFFER8 pvbTemp;
    g_pd3dDevice->CreateVertexBuffer( 4 * sizeof( TILEVERTEX ), 
                                      D3DUSAGE_WRITEONLY, 
                                      D3DFVF_XYZRHW | D3DFVF_TEX1, 
                                      D3DPOOL_MANAGED, &pvbTemp );

    // Create a quad for us to render our texture on
    pvbTemp->Lock( 0, 0, (BYTE **)&pVertices, 0L );
    pVertices[0].p = D3DXVECTOR4( rc.x - 0.5f, rc.w - 0.5f, 1.0f, 1.0f );  pVertices[0].t = D3DXVECTOR2( 0.0f, 1.0f ); // Bottom Left
    pVertices[1].p = D3DXVECTOR4( rc.x - 0.5f, rc.y - 0.5f, 1.0f, 1.0f );  pVertices[1].t = D3DXVECTOR2( 0.0f, 0.0f ); // Top    Left
    pVertices[2].p = D3DXVECTOR4( rc.z - 0.5f, rc.w - 0.5f, 1.0f, 1.0f );  pVertices[2].t = D3DXVECTOR2( 1.0f, 1.0f ); // Bottom Right
    pVertices[3].p = D3DXVECTOR4( rc.z - 0.5f, rc.y - 0.5f, 1.0f, 1.0f );  pVertices[3].t = D3DXVECTOR2( 1.0f, 0.0f ); // Top    Right
    pvbTemp->Unlock();

    // Set up our state
    g_pd3dDevice->SetVertexShader( D3DFVF_XYZRHW|D3DFVF_TEX1 );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1 );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
    g_pd3dDevice->SetStreamSource( 0, pvbTemp, sizeof( TILEVERTEX ) );

    // Render the quad with our texture
    g_pd3dDevice->SetTexture( 0, m_ptMenuSel );
    g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 );
    g_pd3dDevice->SetTexture( 0, NULL );
    pvbTemp->Release();
}




//-----------------------------------------------------------------------------
// Name: DrawText()
// Desc: Display text using UI font
//-----------------------------------------------------------------------------
BOOL UserInterface::DrawText( FLOAT sx, FLOAT sy, DWORD dwColor, 
                              const WCHAR* strText, DWORD dwFlags ) const
{
    return( m_Font.DrawText( sx, sy, dwColor, strText, dwFlags ) == S_OK );
}
