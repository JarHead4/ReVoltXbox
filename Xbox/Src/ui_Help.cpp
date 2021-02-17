//-----------------------------------------------------------------------------
// File: ui_ContentDownload.cpp
//
// Desc: UI for content download
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"
#include "Settings.h"
#include "Text.h"
#include "ui_Menu.h"
#include "ui_menudraw.h"
#include "ui_MenuText.h"
#include "ui_TitleScreen.h"
#include "ui_StateEngine.h"
#include "ui_Help.h"
#include "draw.h"
#include "ui_confirm.h"
#include "readinit.h"
#include "xbhelp.h"
#include <xgraphics.h>

//-----------------------------------------------------------------------------
// global instance of helpcontroler engine
//-----------------------------------------------------------------------------
CHelpControllerEngine g_HelpControllerEngine;

//-----------------------------------------------------------------------------
// global instance of helpweapons engine
//-----------------------------------------------------------------------------
CHelpWeaponsEngine g_HelpWeaponsEngine;

//-----------------------------------------------------------------------------
// forward declares of MENU structs
//-----------------------------------------------------------------------------
extern MENU HelpContoller;
extern MENU HelpWeapons;


//-----------------------------------------------------------------------------
// help globals
//-----------------------------------------------------------------------------
XBHELP_CALLOUT g_HelpCallouts[] = 
{
    { XBHELP_LEFT_BUTTON,  XBHELP_PLACEMENT_2, NULL },
    { XBHELP_RIGHT_BUTTON, XBHELP_PLACEMENT_2, NULL },
    { XBHELP_LEFTSTICK,   XBHELP_PLACEMENT_1,  NULL },
    { XBHELP_A_BUTTON,    XBHELP_PLACEMENT_1,  NULL },
    { XBHELP_B_BUTTON,    XBHELP_PLACEMENT_1,  NULL },
    { XBHELP_Y_BUTTON,    XBHELP_PLACEMENT_1,  NULL },
};

#define NUM_HELP_CALLOUTS ( sizeof( g_HelpCallouts ) / sizeof( g_HelpCallouts[0] ) )

CXBHelp     Help;   // Help object
BOOL        bGamePadExists = FALSE;


//-----------------------------------------------------------------------------
// Name: HelpController
// Desc: The general menu for help controller
//-----------------------------------------------------------------------------
static BOOL HandleHelpController( MENU_HEADER* pMenuHeader, DWORD input );
static VOID DrawHelpController( MENU_HEADER* pMenuHeader, MENU* pMenu );
static void CreateHelpController(MENU_HEADER *menuHeader, MENU *menu);

extern MENU HelpController = 
{
    TEXT_DEFAULTCONTROLS,
    MENU_DEFAULT | MENU_CENTRE_X | MENU_CENTRE_Y ,               // Menu type
    CreateHelpController,                                                   // Create menu function
    HandleHelpController,                                                   // Input handler function
    DrawHelpController,                                                     // Menu draw function
    0,                                                           // X coord
    0,                                                           // Y Coord
};

//-----------------------------------------------------------------------------
// Menu_Msg Create, Handle, and Draw functions
//-----------------------------------------------------------------------------
void CreateHelpController(MENU_HEADER *pMenuHeader, MENU *pMenu)
{
    
    // nothing is selectable
    pMenu->CurrentItemIndex = long(-1);

    Help.Create("D:\\gfx\\gamepad.xbr");
    bGamePadExists = TRUE;
}
BOOL HandleHelpController( MENU_HEADER* pMenuHeader, DWORD dwInput )
{
    // NOTE: only continue or back out if possible for this 
    // menu
    switch( dwInput )
    {
        case MENU_INPUT_SELECT:
            g_pActiveStateEngine->Call( &g_HelpWeaponsEngine );
            return TRUE;
            break;
        case MENU_INPUT_BACK:
            Help.Destroy();
            bGamePadExists = FALSE;
            g_HelpControllerEngine.Return();
            return TRUE;
            break;
    }
    return FALSE;
}
VOID DrawHelpController( MENU_HEADER* pMenuHeader, MENU* pMenu )
{
    FLOAT fX = pMenuHeader->m_XPos - 640.0f/2.0f;
    FLOAT fY = pMenuHeader->m_YPos - 480.0f/2.0f;
    
    FLOAT DivX = 640.0f/40.0f;
    FLOAT DivY = 480.0f/20.0f;
    DrawNewSpruBox(fX + DivX * 3.5f, fY + DivY * 3.0f, fX + DivX * 33.0f, fY + DivY*14.0f); 


    
    
    if(bGamePadExists)
    {
        g_HelpCallouts[0].strText = TEXT_TABLE(TEXT_HELP_LEFTTRIGGER);
        g_HelpCallouts[1].strText = TEXT_TABLE(TEXT_HELP_RIGHTTRIGGER) ;
        g_HelpCallouts[2].strText = TEXT_TABLE(TEXT_HELP_DPAD);
        g_HelpCallouts[3].strText = TEXT_TABLE(TEXT_HELP_ABUTTON);
        g_HelpCallouts[4].strText = TEXT_TABLE(TEXT_HELP_BBUTTON);
        g_HelpCallouts[5].strText = TEXT_TABLE(TEXT_HELP_YBUTTON);

        Help.Render(g_pFont, g_HelpCallouts, NUM_HELP_CALLOUTS, fX, fY);
    }

    static WCHAR strBuf[100];
    swprintf(strBuf, L"\200%s \201%s", TEXT_TABLE(TEXT_WEAPONS), TEXT_TABLE(TEXT_BACK));
    g_pFont->DrawText(fX + 640.0f/2.0f, fY + 360.0f, MENU_COLOR_OPAQUE | MENU_COLOR_WHITE, strBuf, XBFONT_CENTER_X);

    


    
}




//-----------------------------------------------------------------------------
// Name: HelpWeapons
// Desc: The general menu for help controller
//-----------------------------------------------------------------------------
static BOOL HandleHelpWeapons( MENU_HEADER* pMenuHeader, DWORD input );
static VOID DrawHelpWeapons( MENU_HEADER* pMenuHeader, MENU* pMenu );
static void CreateHelpWeapons(MENU_HEADER *menuHeader, MENU *menu);

extern MENU HelpWeapons = 
{
    TEXT_WEAPONS,
    MENU_DEFAULT | MENU_CENTRE_X | MENU_CENTRE_Y ,               // Menu type
    CreateHelpWeapons,                                                   // Create menu function
    HandleHelpWeapons,                                                   // Input handler function
    DrawHelpWeapons,                                                     // Menu draw function
    0,                                                           // X coord
    0,                                                           // Y Coord
};

//-----------------------------------------------------------------------------
// Menu_Msg Create, Handle, and Draw functions
//-----------------------------------------------------------------------------
static int  iIndex = 0;
IDirect3DTexture8* pWeaponsTexture = NULL;
#define NUM_WEAPONS 11

void CreateHelpWeapons(MENU_HEADER *pMenuHeader, MENU *pMenu)
{
    D3DXCreateTextureFromFileEx(D3Ddevice, "D:\\gfx\\fxpage2.bmp",
                                D3DX_DEFAULT, D3DX_DEFAULT, 1,
                                0, D3DFMT_R5G6B5, 0, 
                                D3DX_FILTER_NONE, D3DX_FILTER_NONE,
                                0x00000000, NULL, NULL, &pWeaponsTexture);
    // nothing is selectable
    pMenu->CurrentItemIndex = long(-1);
    iIndex = 0;
}
BOOL HandleHelpWeapons( MENU_HEADER* pMenuHeader, DWORD dwInput )
{
    // NOTE: only continue or back out if possible for this 
    // menu
    switch( dwInput )
    {
        case MENU_INPUT_LEFT:
            iIndex = (iIndex + NUM_WEAPONS - 1) % NUM_WEAPONS;
            return TRUE;
            break;
        case MENU_INPUT_RIGHT:
            iIndex = (iIndex + 1) % NUM_WEAPONS;
            return TRUE;
            break;
        case MENU_INPUT_BACK:
            pWeaponsTexture->Release();
            pWeaponsTexture = NULL;
            g_HelpWeaponsEngine.Return();
            return TRUE;
            break;
    }
    return FALSE;
}
VOID DrawHelpWeapons( MENU_HEADER* pMenuHeader, MENU* pMenu )
{
    FLOAT fX = pMenuHeader->m_XPos - 640.0f/2.0f;
    FLOAT fY = pMenuHeader->m_YPos - 480.0f/2.0f;

    FLOAT DivX = 640.0f/8.0f;
    FLOAT DivY = 480.0f/20.0f;
    DrawNewSpruBox(DivX * 1.0f + fX, DivY * 3.0f + fY, DivX * 6.0f, DivY*14.0f); 

    const WCHAR* strWeaponName = TEXT_TABLE(TEXT_SHOCKWAVE + 2*iIndex);
    g_pFont->DrawText(640.0f/2.0f + fX, 80.0f + fY, MENU_COLOR_OPAQUE | MENU_COLOR_WHITE,strWeaponName , XBFONT_CENTER_X);



    const WCHAR* strWeaponDesc = TEXT_TABLE(TEXT_SHOCKWAVE + 2*iIndex + 1);
    g_pFont->DrawText(640.0f/2.0f + fX, 220.0f + fY, MENU_COLOR_OPAQUE | MENU_COLOR_WHITE,strWeaponDesc , XBFONT_CENTER_X);

    static WCHAR strBuf[100];
    swprintf(strBuf, L"\x2190%s\x2192 \201%s", TEXT_TABLE(TEXT_SCROLL), TEXT_TABLE(TEXT_BACK));
    g_pFont->DrawText(640.0f/2.0f + fX, 370.0f + fY, MENU_COLOR_OPAQUE | MENU_COLOR_WHITE, strBuf, XBFONT_CENTER_X);

    int iU = 0;
    int iV = 0;

    switch(iIndex)
    {
    case 0: // shockwave
        iU = 0;
        iV = 7;
        break;
    case 1: // firework
        iU = 2;
        iV = 7;
        break;
    case 2: // firework pack
        iU = 3;
        iV = 7;
        break;
    case 3: // electo pulse
        iU = 7;
        iV = 7;
        break;
    case 4: // bomb
        iU = 1;
        iV = 7;
        break;
    case 5: // oil slick
        iU = 6;
        iV = 7;
        break;
    case 6: // water ballons
        iU = 4;
        iV = 7;
        break;
    case 7: // ball brearking
        iU = 0;
        iV = 6;
        break;
    case 8: // clone pickup
        iU = 5;
        iV = 7;
        break;
    case 9: // trubo batery
        iU = 1;
        iV = 6;
        break;
    case 10:// global pusle
        iU = 3;
        iV = 6;
        break;
    default:
        assert( FALSE );
    }

    
    // render texture
    if(pWeaponsTexture)
    {
        FLOAT fCX = pMenuHeader->m_XPos;
        FLOAT fCY = pMenuHeader->m_YPos - 80;

        FLOAT fUVSize = 1.0/8.0f;
        FLOAT fU = fUVSize * iU;
        FLOAT fV = fUVSize * iV;
        
        SET_TEXTURE(0, pWeaponsTexture);
        FOG_OFF();
        BLEND_ON();
        D3DDevice_SetVertexShader( FVF_TEX1 );

        // dimetions of texture
        FLOAT fWidth   = 100.0f;
        FLOAT fHeight = 100.0f;
    
        VERTEX_TEX1 Verts[4];
        for( DWORD i=0; i<4; i++ )
        {
            Verts[i].sz    = 0.0f;
            Verts[i].rhw   = 1.0f;
            Verts[i].color =  0xffffffff;
            Verts[i].specular = 0x00000000;
        }

        FLOAT x0 = fCX - fWidth/2.0f;
        FLOAT x1 = fCX + fWidth/2.0f;
        FLOAT y0 = fCY - fHeight/2.0f;
        FLOAT y1 = fCY + fHeight/2.0f;

        FLOAT u0 = fU;
        FLOAT u1 = fU + fUVSize;
        FLOAT v0 = fV;
        FLOAT v1 = fV + fUVSize;


    
        Verts[0].sx = x0;
        Verts[0].sy = y0;
        Verts[0].tu = u0;
        Verts[0].tv = v0;


        Verts[1].sx = x1;
        Verts[1].sy = y0;
        Verts[1].tu = u1;
        Verts[1].tv = v0;

        Verts[2].sx = x1;
        Verts[2].sy = y1;
        Verts[2].tu = u1;
        Verts[2].tv = v1;

        Verts[3].sx = x0;
        Verts[3].sy = y1;
        Verts[3].tu = u0;
        Verts[3].tv = v1;

        // draw outlined box
        D3DDevice_DrawVerticesUP( D3DPT_LINELOOP, 4, Verts, sizeof(Verts[0]) );

        // draw textured quad
        D3DDevice_DrawVerticesUP( D3DPT_QUADLIST, 4, Verts, sizeof(Verts[0]) );

        // clear T page
        // $MD: required?

        SET_TPAGE(-1);
    }
    
}




//-----------------------------------------------------------------------------
// Name: Process()
// Desc: Main control function for the state engine
//-----------------------------------------------------------------------------
HRESULT CHelpControllerEngine::Process()
{
    switch( m_State )
    {
        case HELP_STATE_BEGIN:
            // set menu
            g_pMenuHeader->SetNextMenu( &HelpController );

            m_State = HELP_STATE_LOOP;
            // fallthrough
        case HELP_STATE_LOOP:
            
            break;
    }
    
    // Handle the menus
    g_pMenuHeader->HandleMenus();

    return S_FALSE;
}



//-----------------------------------------------------------------------------
// Name: Process()
// Desc: Main control function for the state engine
//-----------------------------------------------------------------------------
HRESULT CHelpWeaponsEngine::Process()
{
    switch( m_State )
    {
        case HELP_STATE_BEGIN:

            // set menu
            g_pMenuHeader->SetNextMenu( &HelpWeapons );

            m_State = HELP_STATE_LOOP;
            // fallthrough
        case HELP_STATE_LOOP:
            
            break;
    }
    
    // Handle the menus
    g_pMenuHeader->HandleMenus();

    return S_FALSE;
}



    

