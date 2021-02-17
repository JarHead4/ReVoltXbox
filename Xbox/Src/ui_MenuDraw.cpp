//-----------------------------------------------------------------------------
// File: ui_MenuDraw.cpp
//
// Desc: UI implementation
//
// Re-Volt (Generic) Copyright (c) Probe Entertainment 1998
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------

//$TODO(cprince): I think there are places in this file where they use
/// screen-space coords, but they don't do the necessary half-pixel offset.
/// Need to look into this.

#include "revolt.h"
#include "input.h"
#include "geom.h"
#include "camera.h"
#include "drawobj.h"
#include "player.h"
#include "spark.h"
#include "timing.h"
#include "obj_init.h"
#include "LevelInfo.h"
#include "SoundEffectEngine.h"
#include "text.h"
#include "network.h"
#include "draw.h"
#include "settings.h"
#include "panel.h"
#include "initplay.h"
#include "ui_TitleScreen.h"
#include "ui_MenuDraw.h"
#include "ui_MenuText.h"
//#include "ui_TopLevelMenu.h"
#include "XBFont.h"



#define MENU_SPRU_Z         0 //$MODIFIED: was originally 300  //$NOTE(cprince): was only used in places where ZBUFFER_OFF was called.
#define MENU_SPRU_RHW       1
#define MENU_SPRU_WIDTH     4
#define MENU_CORNER_WIDTH   16
#define MENU_CORNER_HEIGHT  16

#define MENU_IMAGE_WIDTH    64

// Globals
WCHAR  MenuBuffer[256];
WCHAR  MenuBuffer2[256];

FLOAT gMenuWidthScale  = ONE;
FLOAT gMenuHeightScale = ONE;

extern long SessionRefreshFlag;




//-----------------------------------------------------------------------------
// Name: DrawScreenSpaceQuad()
// Desc: Renders a screen space, textured quad at the given offset. The
//       width and height are taken from the texture, which is assumed to be
//       linear.
//-----------------------------------------------------------------------------
VOID DrawScreenSpaceQuad( FLOAT sx, FLOAT sy, D3DTexture* pTexture )
{
    // BUGBUG (JHarding): Someone's assuming COLOROP will be set the way
    // they want it.  To find out who, comment out the restoring of this
    // state and load up the friends menu.
    DWORD dwColorOp0;
    D3DDevice_GetTextureStageState( 0, D3DTSS_COLOROP,   &dwColorOp0 );

    D3DDevice_SetTexture( 0, pTexture );
    D3DDevice_SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
    D3DDevice_SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
    D3DDevice_SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
    D3DDevice_SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
    D3DDevice_SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE );
    D3DDevice_SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );
    D3DDevice_SetTextureStageState( 0, D3DTSS_ADDRESSU,  D3DTADDRESS_CLAMP );
    D3DDevice_SetTextureStageState( 0, D3DTSS_ADDRESSV,  D3DTADDRESS_CLAMP );
    D3DDevice_SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );
    D3DDevice_SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR );
    D3DDevice_SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
    D3DDevice_SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA );
    D3DDevice_SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );

    D3DSURFACE_DESC desc;
    pTexture->GetLevelDesc( 0, &desc );
    FLOAT x1  = floorf(sx) - 0.5f;
    FLOAT y1  = floorf(sy) - 0.5f;
    FLOAT x2  = floorf(sx) - 0.5f + (FLOAT)desc.Width;
    FLOAT y2  = floorf(sy) - 0.5f + (FLOAT)desc.Height;
    FLOAT tu1 = 0.0f;
    FLOAT tv1 = 0.0f;
    FLOAT tu2 = (FLOAT)desc.Width;
    FLOAT tv2 = (FLOAT)desc.Height;

    struct VERTEX { D3DXVECTOR4 p; FLOAT tu, tv; };
    VERTEX v[4];
    v[0].p  = D3DXVECTOR4( x1, y1, 0, 0 );  v[0].tu = tu1; v[0].tv = tv1;
    v[1].p  = D3DXVECTOR4( x2, y1, 0, 0 );  v[1].tu = tu2; v[1].tv = tv1;
    v[2].p  = D3DXVECTOR4( x2, y2, 0, 0 );  v[2].tu = tu2; v[2].tv = tv2;
    v[3].p  = D3DXVECTOR4( x1, y2, 0, 0 );  v[3].tu = tu1; v[3].tv = tv2;

    // Draw the box
    D3DDevice_SetVertexShader( D3DFVF_XYZRHW|D3DFVF_TEX1 );
    D3DDevice_DrawVerticesUP( D3DPT_QUADLIST, 4, v, sizeof(VERTEX) );

    // Restore state
    D3DDevice_SetTexture( 0, NULL );
    D3DDevice_SetTextureStageState( 0, D3DTSS_COLOROP,   dwColorOp0 );
}


extern void DrawNewTitleBox(FLOAT xPos, FLOAT yPos, FLOAT xSize, FLOAT ySize, int col, int trans);


FLOAT g_fMenuIn = 1.0f;

//-----------------------------------------------------------------------------
// Name: DrawMenuTitle()
// Desc: 
//-----------------------------------------------------------------------------
void MENU_HEADER::DrawMenuTitle()
{
    if( m_strTitle == NULL ) return;
    if( m_pMenu == NULL ) return;

    // Draw the box
    DrawNewTitleBox( 0, 23, 640, 48, 0xffffffff, 0 );

    // Draw the menu title string
    BeginTextState();
    
    g_pFont->SetScaleFactors( 1.5f, 1.5f );
    g_pFont->DrawText( 320, 30, MENU_TEXT_RGB_TITLE, m_strTitle, XBFONT_CENTER_X );
    g_pFont->SetScaleFactors( 1.0f, 1.0f );
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
void MENU_HEADER::DrawMenuBox()
{
    FLOAT xPos  = gMenuWidthScale  * (m_XPos - MENU_LEFT_PAD );
    FLOAT yPos  = gMenuHeightScale * (m_YPos - MENU_TOP_PAD );
    FLOAT xSize = gMenuWidthScale  * (m_XSize + MENU_LEFT_PAD + MENU_RIGHT_PAD );
    FLOAT ySize = gMenuHeightScale * (m_YSize + MENU_TOP_PAD + MENU_BOTTOM_PAD );

    DrawNewSpruBox( xPos, yPos, xSize, ySize );
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
void MENU_HEADER::DrawMenuItemText( MENU* pMenu, MENU_ITEM* pMenuItem, int itemIndex)
{
    MENU_ITEM *currentItem = m_pMenuItem[pMenu->CurrentItemIndex];

    // If no text, don't draw!
    if( pMenuItem->TextIndex == TEXT_NONE && 0 == (pMenuItem->ActiveFlags&MENU_ITEM_HARDCODEDTEXT) )
        return;

    FLOAT xPos = m_XPos;
    FLOAT yPos = m_YPos + (MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP) * itemIndex;

    // Choose text colour
    long col = MENU_TEXT_RGB_NORMAL;

    if( currentItem == pMenuItem )
    {
        col = MENU_TEXT_RGB_HILITE;
    }
    if( !(m_pMenuItem[itemIndex]->ActiveFlags & MENU_ITEM_ACTIVE) )
    {
        if( currentItem == pMenuItem )
            col = MENU_TEXT_RGB_MIDLITE;
        else 
            col = MENU_TEXT_RGB_LOLITE;
    }

    // Draw item name text
    if( pMenuItem->ActiveFlags & MENU_ITEM_HARDCODEDTEXT )
        DrawMenuText(xPos, yPos, col, (WCHAR*)pMenuItem->Data, m_ItemTextWidth );
    else
        DrawMenuText(xPos, yPos, col, gTitleScreen_Text[pMenuItem->TextIndex], m_ItemTextWidth);
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
void DrawMenuDataInt( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem, int itemIndex)
{
    swprintf(MenuBuffer, L"%d", *(int*)pMenuItem->Data);

    FLOAT xPos = pMenuHeader->m_XPos + pMenuHeader->m_ItemTextWidth + MENU_TEXT_GAP;;
    FLOAT yPos = pMenuHeader->m_YPos + (MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP) * itemIndex;

    if( pMenuItem->ActiveFlags & MENU_ITEM_ACTIVE )
        DrawMenuText(xPos, yPos, MENU_TEXT_RGB_NORMAL, MenuBuffer, pMenuHeader->m_ItemDataWidth);
    else
        DrawMenuText(xPos, yPos, MENU_TEXT_RGB_LOLITE, MenuBuffer, pMenuHeader->m_ItemDataWidth);
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
void DrawMenuDataOnOff( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem, int itemIndex)
{
    FLOAT xPos = pMenuHeader->m_XPos + pMenuHeader->m_ItemTextWidth + MENU_TEXT_GAP;
    FLOAT yPos = pMenuHeader->m_YPos + (MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP) * itemIndex;

    WCHAR* strMenuItemText = *(bool*)pMenuItem->Data ? gTitleScreen_Text[TEXT_ON]: gTitleScreen_Text[TEXT_OFF];
    DWORD  color = pMenuItem->ActiveFlags & MENU_ITEM_ACTIVE ? MENU_TEXT_RGB_NORMAL : MENU_TEXT_RGB_LOLITE;

    DrawMenuTextWithArrows( (pMenuItem == pMenuHeader->m_pCurrentItem) ? TRUE : FALSE,
                            xPos, yPos, color, strMenuItemText, 
                            pMenuHeader->m_ItemDataWidth );
}

void DrawMenuDataYesNo( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem, int itemIndex)
{
    FLOAT xPos = pMenuHeader->m_XPos + pMenuHeader->m_ItemTextWidth + MENU_TEXT_GAP;
    FLOAT yPos = pMenuHeader->m_YPos + (MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP) * itemIndex;

    WCHAR* strMenuItemText = *(bool*)pMenuItem->Data ? TEXT_TABLE(TEXT_YES): TEXT_TABLE(TEXT_NO);
    DWORD  color = pMenuItem->ActiveFlags & MENU_ITEM_ACTIVE ? MENU_TEXT_RGB_NORMAL : MENU_TEXT_RGB_LOLITE;

    DrawMenuTextWithArrows( (pMenuItem == pMenuHeader->m_pCurrentItem) ? TRUE : FALSE,
                            xPos, yPos, color, strMenuItemText, 
                            pMenuHeader->m_ItemDataWidth );
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
void DrawMenuDataDWORD( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem, int itemIndex)
{
    FLOAT xPos, yPos;

    swprintf(MenuBuffer, L"%d", *(DWORD*)pMenuItem->Data);

    xPos = pMenuHeader->m_XPos + pMenuHeader->m_ItemTextWidth + MENU_TEXT_GAP;
    yPos = pMenuHeader->m_YPos + (MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP) * itemIndex;
    DrawMenuText(xPos, yPos, MENU_TEXT_RGB_NORMAL, MenuBuffer, pMenuHeader->m_ItemDataWidth);
}


////////////////////////////////////////////////////////////////
//
// Draw Slider (long)
//
////////////////////////////////////////////////////////////////
void DrawSliderDataLong( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem, int itemIndex)
{
    FLOAT xPos, yPos;
    SLIDER_DATA_LONG *sliderData = (SLIDER_DATA_LONG*)pMenuItem->Data;

    xPos = pMenuHeader->m_XPos + pMenuHeader->m_ItemTextWidth + MENU_TEXT_GAP;
    yPos = pMenuHeader->m_YPos + (MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP) * itemIndex;


    if( sliderData->DrawSlider ) 
    {
        DrawSliderDataSlider(xPos, yPos, *sliderData->Data, sliderData->Min, sliderData->Max, (pMenuItem->ActiveFlags & MENU_ITEM_ACTIVE) != 0);
        xPos += MENU_DATA_WIDTH_SLIDER + MENU_TEXT_HSKIP;

        swprintf(MenuBuffer, L"%d%%", (int)(100*((FLOAT)(*(sliderData->Data)-sliderData->Min))/(sliderData->Max-sliderData->Min)));
    } 
    else 
    {
        swprintf(MenuBuffer, L"%d", *(sliderData->Data));
    }

    BeginTextState();

    if( pMenuItem == pMenuHeader->m_pCurrentItem )
    {
        static WCHAR strLeftArrowString[3]  = { 0x2190, 0x0020, 0x0000 };
        static WCHAR strRightArrowString[3] = { 0x0020, 0x2192, 0x0000 };

        wcscat( MenuBuffer, strRightArrowString );

        if( sliderData->DrawSlider ) 
            g_pFont->DrawText( xPos-(MENU_DATA_WIDTH_SLIDER + MENU_TEXT_HSKIP), yPos, MENU_TEXT_RGB_NORMAL, strLeftArrowString, XBFONT_RIGHT );
        else
            g_pFont->DrawText( xPos, yPos, MENU_TEXT_RGB_NORMAL, strLeftArrowString, XBFONT_RIGHT );
    }

    if( pMenuItem->ActiveFlags & MENU_ITEM_ACTIVE )
        DrawMenuText( xPos, yPos, MENU_TEXT_RGB_NORMAL, MenuBuffer, pMenuHeader->m_ItemDataWidth );
    else 
        DrawMenuText( xPos, yPos, MENU_TEXT_RGB_LOLITE, MenuBuffer, pMenuHeader->m_ItemDataWidth );
}

// unsigned long
void DrawSliderDataULong( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem, int itemIndex)
{
    FLOAT xPos, yPos;
    SLIDER_DATA_ULONG *sliderData = (SLIDER_DATA_ULONG*)pMenuItem->Data;

    xPos = pMenuHeader->m_XPos + pMenuHeader->m_ItemTextWidth + MENU_TEXT_GAP;
    yPos = pMenuHeader->m_YPos + (MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP) * itemIndex;

    if (sliderData->DrawSlider) 
    {
        DrawSliderDataSlider(xPos, yPos, *sliderData->Data, sliderData->Min, sliderData->Max, (pMenuItem->ActiveFlags & MENU_ITEM_ACTIVE) != 0);
        xPos += MENU_DATA_WIDTH_SLIDER + MENU_TEXT_HSKIP;
        swprintf(MenuBuffer, L"%d%%", *(sliderData->Data));
    } 
    else 
    {
        swprintf(MenuBuffer, L"%d", *(sliderData->Data));
    }

    BeginTextState();

    if( pMenuItem == pMenuHeader->m_pCurrentItem )
    {
        static WCHAR strLeftArrowString[3]  = { 0x2190, 0x0020, 0x0000 };
        static WCHAR strRightArrowString[3] = { 0x0020, 0x2192, 0x0000 };

        wcscat( MenuBuffer, strRightArrowString );

        if( sliderData->DrawSlider ) 
            g_pFont->DrawText( xPos-(MENU_DATA_WIDTH_SLIDER + MENU_TEXT_HSKIP), yPos, MENU_TEXT_RGB_NORMAL, strLeftArrowString, XBFONT_RIGHT );
        else
            g_pFont->DrawText( xPos, yPos, MENU_TEXT_RGB_NORMAL, strLeftArrowString, XBFONT_RIGHT );
    }

    if (pMenuItem->ActiveFlags & MENU_ITEM_ACTIVE)
        DrawMenuText(xPos,yPos, MENU_TEXT_RGB_NORMAL, MenuBuffer, pMenuHeader->m_ItemDataWidth);
    else 
        DrawMenuText(xPos,yPos, MENU_TEXT_RGB_LOLITE, MenuBuffer, pMenuHeader->m_ItemDataWidth);
}




////////////////////////////////////////////////////////////////
//
// Menu Not Implemented
//
////////////////////////////////////////////////////////////////
void DrawMenuNotImplemented( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem, int itemIndex)
{
    FLOAT xPos, yPos;

    xPos = pMenuHeader->m_XPos + pMenuHeader->m_ItemTextWidth + MENU_TEXT_GAP;
    yPos = pMenuHeader->m_YPos + (MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP) * itemIndex;

    DrawMenuText(xPos,yPos, 0x0088ff, L"Not Implemented", pMenuHeader->m_ItemDataWidth);
}




////////////////////////////////////////////////////////////////
//
// Draw Spru Box: the position and width given are the inside
// pos and dimensions.
//
////////////////////////////////////////////////////////////////
extern D3DTexture* g_pBoxTexture;
extern D3DTexture* g_pTitleBoxTexture;

void DrawNewTitleBox(FLOAT xPos, FLOAT yPos, FLOAT xSize, FLOAT ySize, int col, int trans)
{
    VERTEX_TEX1 verts[4*2];

    // Keep box' pixels aligned with box' texels
    xPos  = floorf(xPos)-0.5f;
    yPos  = floorf(yPos)-0.5f;
    xSize = floorf(xSize);
    ySize = floorf(ySize);

    ZBUFFER_OFF();
    FOG_OFF();
    D3DDevice_SetVertexShader( FVF_TEX1 );

    SET_TPAGE(-1);
    D3DDevice_SetTexture( 0, g_pTitleBoxTexture );
    BLEND_OFF();

    D3DDevice_SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
    D3DDevice_SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
    D3DDevice_SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
    D3DDevice_SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA );
    D3DDevice_SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );

    FLOAT x1 = xPos - 46.0f;
    FLOAT x2 = xPos + 140;
    FLOAT x3 = xPos + xSize - 140;
    FLOAT x4 = xPos + xSize + 46.0f;

    FLOAT y1 = yPos;
    FLOAT y2 = yPos + 48;

    FLOAT tu1 =   0.0f;
    FLOAT tu2 = 186.0f;
    FLOAT tu3 = 186.0f;
    FLOAT tu4 =   0.0f;
    FLOAT tv1 =   0.0f;
    FLOAT tv2 =  48.0f;

    WORD indices[] = 
    {
         0, 1, 2, 3, // Left piece
         1, 4, 7, 2, // Center portion
         4, 5, 6, 7, // Right piece
    };

    for( DWORD i=0; i<8; i++ )
    {
        verts[i].sz    = MENU_SPRU_Z;
        verts[i].rhw   = MENU_SPRU_RHW;
        verts[i].color = 0xffffffff;
    }

    // Left piece
    verts[0].sx = x1;    verts[0].sy = y1;
    verts[1].sx = x2;    verts[1].sy = y1;
    verts[2].sx = x2;    verts[2].sy = y2;
    verts[3].sx = x1;    verts[3].sy = y2;

    verts[0].tu = tu1;   verts[0].tv = tv1;
    verts[1].tu = tu2;   verts[1].tv = tv1;
    verts[2].tu = tu2;   verts[2].tv = tv2;
    verts[3].tu = tu1;   verts[3].tv = tv2;

    // Right piece
    verts[4].sx = x3;    verts[4].sy = y1;
    verts[5].sx = x4;    verts[5].sy = y1;
    verts[6].sx = x4;    verts[6].sy = y2;
    verts[7].sx = x3;    verts[7].sy = y2;

    verts[4].tu = tu3;   verts[4].tv = tv1;
    verts[5].tu = tu4;   verts[5].tv = tv1;
    verts[6].tu = tu4;   verts[6].tv = tv2;
    verts[7].tu = tu3;   verts[7].tv = tv2;

    D3DDevice_DrawIndexedVerticesUP( D3DPT_QUADLIST, 12, indices, verts, sizeof(verts[0]) );
}

void DrawNewSpruBox(FLOAT xPos, FLOAT yPos, FLOAT xSize, FLOAT ySize )
{
    VERTEX_TEX1 verts[4*5];

    const FLOAT BORDERSIZE = 16.0f;

    // Keep box' pixels aligned with box' texels
    xPos  = floorf(xPos)-0.5f;
    yPos  = floorf(yPos)-0.5f;
    xSize = floorf(xSize);
    ySize = floorf(ySize);

    ZBUFFER_OFF();
    FOG_OFF();
    D3DDevice_SetVertexShader( FVF_TEX1 );

    {
        // Draw background transparent poly
        verts[0].sx = xPos + BORDERSIZE;
        verts[0].sy = yPos + BORDERSIZE;
        verts[1].sx = xPos + xSize - BORDERSIZE;
        verts[1].sy = yPos + BORDERSIZE;
        verts[2].sx = xPos + xSize - BORDERSIZE;
        verts[2].sy = yPos + ySize - BORDERSIZE;
        verts[3].sx = xPos + BORDERSIZE;
        verts[3].sy = yPos + ySize - BORDERSIZE;

        verts[0].sz    = verts[1].sz    = verts[2].sz    = verts[3].sz    = MENU_SPRU_Z;
        verts[0].rhw   = verts[1].rhw   = verts[2].rhw   = verts[3].rhw   = MENU_SPRU_RHW;
        verts[0].color = verts[1].color = verts[2].color = verts[3].color = 0xffffffff;

        verts[0].tu = 34 + 15.0f;    verts[0].tv = 15.0f;
        verts[1].tu = 34 + 16.0f;    verts[1].tv = 15.0f;
        verts[2].tu = 34 + 16.0f;    verts[2].tv = 16.0f;
        verts[3].tu = 34 + 15.0f;    verts[3].tv = 16.0f;

        // Set colours
        BLEND_ALPHA();
        BLEND_SRC(D3DBLEND_SRCALPHA);
        BLEND_DEST(D3DBLEND_INVSRCALPHA);

        // Draw background poly
        SET_TPAGE(-1);
        D3DDevice_SetTexture( 0, g_pBoxTexture );
        D3DDevice_SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
        D3DDevice_SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
        D3DDevice_SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
        D3DDevice_SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA );
        D3DDevice_SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
        D3DDevice_DrawVerticesUP( D3DPT_QUADLIST, 4, verts, sizeof(verts[0]) );
    } 

    ////////////////////////////////////////////////////////////////
    // Draw border
    SET_TPAGE(-1);
    D3DDevice_SetTexture( 0, g_pBoxTexture );
    BLEND_OFF();

    D3DDevice_SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
    D3DDevice_SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
    D3DDevice_SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
    D3DDevice_SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA );
    D3DDevice_SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );

    FLOAT x1 = xPos;
    FLOAT x2 = xPos + BORDERSIZE;
    FLOAT x3 = xPos + xSize - BORDERSIZE;
    FLOAT x4 = xPos + xSize;

    FLOAT y1 = yPos;
    FLOAT y2 = yPos + BORDERSIZE;
    FLOAT y3 = yPos + ySize - BORDERSIZE;
    FLOAT y4 = yPos + ySize;

    FLOAT tu1 = 34 +  0.0f;
    FLOAT tu2 = 34 + 16.0f;
    FLOAT tu3 = 34 + 16.0f;
    FLOAT tu4 = 34 + 32.0f;
    FLOAT tv1 =  0.0f;
    FLOAT tv2 = 16.0f;
    FLOAT tv3 = 16.0f;
    FLOAT tv4 = 32.0f;

    WORD indices[] = 
    {
         4,  5,  6,  7, // Top left corner
         5,  8, 11,  6, // Top bar
         8,  9, 10, 11, // Top right corner
         7,  6, 17, 16, // Left bar
        11, 10, 13, 12, // Right bar
        16, 17, 18, 19, // Bottom left corner
        17, 12, 15, 18, // Bottom bar
        12, 13, 14, 15, // Bottom right corner
    };

    for( DWORD i=0; i<16; i++ )
    {
        verts[i+4].sz    = MENU_SPRU_Z;
        verts[i+4].rhw   = MENU_SPRU_RHW;
        verts[i+4].color = 0xffffffff;
    }

    // Top left
    verts[4].sx  = x1;    verts[4].sy  = y1;
    verts[5].sx  = x2;    verts[5].sy  = y1;
    verts[6].sx  = x2;    verts[6].sy  = y2;
    verts[7].sx  = x1;    verts[7].sy  = y2;

    verts[4].tu  = tu1;   verts[4].tv  = tv1;
    verts[5].tu  = tu2;   verts[5].tv  = tv1;
    verts[6].tu  = tu2;   verts[6].tv  = tv2;
    verts[7].tu  = tu1;   verts[7].tv  = tv2;

    // Top right
    verts[8].sx  = x3;    verts[8].sy  = y1;
    verts[9].sx  = x4;    verts[9].sy  = y1;
    verts[10].sx = x4;    verts[10].sy = y2;
    verts[11].sx = x3;    verts[11].sy = y2;

    verts[8].tu  = tu3;   verts[8].tv  = tv1;
    verts[9].tu  = tu4;   verts[9].tv  = tv1;
    verts[10].tu = tu4;   verts[10].tv = tv2;
    verts[11].tu = tu3;   verts[11].tv = tv2;

    // bottom right
    verts[12].sx = x3;    verts[12].sy = y3;
    verts[13].sx = x4;    verts[13].sy = y3;
    verts[14].sx = x4;    verts[14].sy = y4;
    verts[15].sx = x3;    verts[15].sy = y4;

    verts[12].tu = tu3;   verts[12].tv = tv3;
    verts[13].tu = tu4;   verts[13].tv = tv3;
    verts[14].tu = tu4;   verts[14].tv = tv4;
    verts[15].tu = tu3;   verts[15].tv = tv4;

    // Bottom left
    verts[16].sx = x1;    verts[16].sy = y3;
    verts[17].sx = x2;    verts[17].sy = y3;
    verts[18].sx = x2;    verts[18].sy = y4;
    verts[19].sx = x1;    verts[19].sy = y4;

    verts[16].tu = tu1;   verts[16].tv = tv3;
    verts[17].tu = tu2;   verts[17].tv = tv3;
    verts[18].tu = tu2;   verts[18].tv = tv4;
    verts[19].tu = tu1;   verts[19].tv = tv4;

    D3DDevice_DrawIndexedVerticesUP( D3DPT_QUADLIST, 32, indices, verts, sizeof(verts[0]) );
}

void DrawNewSpruOutlineBox(FLOAT xPos, FLOAT yPos, FLOAT xSize, FLOAT ySize )
{
    VERTEX_TEX1 verts[4*5];

    const FLOAT BORDERSIZE = 16.0f;

    // Keep box' pixels aligned with box' texels
    xPos  = floorf(xPos)-0.5f;
    yPos  = floorf(yPos)-0.5f;
    xSize = floorf(xSize);
    ySize = floorf(ySize);

    ZBUFFER_OFF();
    FOG_OFF();
    D3DDevice_SetVertexShader( FVF_TEX1 );

    // Draw border
    SET_TPAGE(-1);
    D3DDevice_SetTexture( 0, g_pBoxTexture );
    BLEND_OFF();

    D3DDevice_SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
    D3DDevice_SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
    D3DDevice_SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
    D3DDevice_SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA );
    D3DDevice_SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );

    FLOAT x1 = xPos;
    FLOAT x2 = xPos + BORDERSIZE;
    FLOAT x3 = xPos + xSize - BORDERSIZE;
    FLOAT x4 = xPos + xSize;

    FLOAT y1 = yPos;
    FLOAT y2 = yPos + BORDERSIZE;
    FLOAT y3 = yPos + ySize - BORDERSIZE;
    FLOAT y4 = yPos + ySize;

    FLOAT tu1 =  0.0f;
    FLOAT tu2 = 16.0f;
    FLOAT tu3 = 16.0f;
    FLOAT tu4 = 32.0f;
    FLOAT tv1 =  0.0f;
    FLOAT tv2 = 16.0f;
    FLOAT tv3 = 16.0f;
    FLOAT tv4 = 32.0f;

    WORD indices[] = 
    {
         4,  5,  6,  7, // Top left corner
         5,  8, 11,  6, // Top bar
         8,  9, 10, 11, // Top right corner
         7,  6, 17, 16, // Left bar
        11, 10, 13, 12, // Right bar
        16, 17, 18, 19, // Bottom left corner
        17, 12, 15, 18, // Bottom bar
        12, 13, 14, 15, // Bottom right corner
    };

    for( DWORD i=0; i<16; i++ )
    {
        verts[i+4].sz    = MENU_SPRU_Z;
        verts[i+4].rhw   = MENU_SPRU_RHW;
        verts[i+4].color = 0xffffffff;
    }

    // Top left
    verts[4].sx  = x1;    verts[4].sy  = y1;
    verts[5].sx  = x2;    verts[5].sy  = y1;
    verts[6].sx  = x2;    verts[6].sy  = y2;
    verts[7].sx  = x1;    verts[7].sy  = y2;

    verts[4].tu  = tu1;   verts[4].tv  = tv1;
    verts[5].tu  = tu2;   verts[5].tv  = tv1;
    verts[6].tu  = tu2;   verts[6].tv  = tv2;
    verts[7].tu  = tu1;   verts[7].tv  = tv2;

    // Top right
    verts[8].sx  = x3;    verts[8].sy  = y1;
    verts[9].sx  = x4;    verts[9].sy  = y1;
    verts[10].sx = x4;    verts[10].sy = y2;
    verts[11].sx = x3;    verts[11].sy = y2;

    verts[8].tu  = tu3;   verts[8].tv  = tv1;
    verts[9].tu  = tu4;   verts[9].tv  = tv1;
    verts[10].tu = tu4;   verts[10].tv = tv2;
    verts[11].tu = tu3;   verts[11].tv = tv2;

    // bottom right
    verts[12].sx = x3;    verts[12].sy = y3;
    verts[13].sx = x4;    verts[13].sy = y3;
    verts[14].sx = x4;    verts[14].sy = y4;
    verts[15].sx = x3;    verts[15].sy = y4;

    verts[12].tu = tu3;   verts[12].tv = tv3;
    verts[13].tu = tu4;   verts[13].tv = tv3;
    verts[14].tu = tu4;   verts[14].tv = tv4;
    verts[15].tu = tu3;   verts[15].tv = tv4;

    // Bottom left
    verts[16].sx = x1;    verts[16].sy = y3;
    verts[17].sx = x2;    verts[17].sy = y3;
    verts[18].sx = x2;    verts[18].sy = y4;
    verts[19].sx = x1;    verts[19].sy = y4;

    verts[16].tu = tu1;   verts[16].tv = tv3;
    verts[17].tu = tu2;   verts[17].tv = tv3;
    verts[18].tu = tu2;   verts[18].tv = tv4;
    verts[19].tu = tu1;   verts[19].tv = tv4;

    D3DDevice_DrawIndexedVerticesUP( D3DPT_QUADLIST, 32, indices, verts, sizeof(verts[0]) );
}

void DrawNewSpruBoxWithTabs(FLOAT xPos, FLOAT yPos, FLOAT xSize, FLOAT ySize )
{
    VERTEX_TEX1 verts[4*5];

    const FLOAT BORDERSIZE = 16.0f;

    // Keep box' pixels aligned with box' texels
    xPos  = floorf(xPos)-0.5f;
    yPos  = floorf(yPos)-0.5f;
    xSize = floorf(xSize);
    ySize = floorf(ySize);

    ZBUFFER_OFF();
    FOG_OFF();
    D3DDevice_SetVertexShader( FVF_TEX1 );

    {
        // Draw background transparent poly
        verts[0].sx = xPos + BORDERSIZE;
        verts[0].sy = yPos + BORDERSIZE;
        verts[1].sx = xPos + xSize - BORDERSIZE;
        verts[1].sy = yPos + BORDERSIZE;
        verts[2].sx = xPos + xSize - BORDERSIZE;
        verts[2].sy = yPos + ySize - BORDERSIZE;
        verts[3].sx = xPos + BORDERSIZE;
        verts[3].sy = yPos + ySize - BORDERSIZE;

        verts[0].sz    = verts[1].sz    = verts[2].sz    = verts[3].sz    = MENU_SPRU_Z;
        verts[0].rhw   = verts[1].rhw   = verts[2].rhw   = verts[3].rhw   = MENU_SPRU_RHW;
        verts[0].color = verts[1].color = verts[2].color = verts[3].color = 0xffffffff;

        verts[0].tu = 34 + 15.0f;    verts[0].tv = 15.0f;
        verts[1].tu = 34 + 16.0f;    verts[1].tv = 15.0f;
        verts[2].tu = 34 + 16.0f;    verts[2].tv = 16.0f;
        verts[3].tu = 34 + 15.0f;    verts[3].tv = 16.0f;

        // Set colours
        BLEND_ALPHA();
        BLEND_SRC(D3DBLEND_SRCALPHA);
        BLEND_DEST(D3DBLEND_INVSRCALPHA);

        // Draw background poly
        SET_TPAGE(-1);
        D3DDevice_SetTexture( 0, g_pBoxTexture );
        D3DDevice_SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
        D3DDevice_SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
        D3DDevice_SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
        D3DDevice_SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA );
        D3DDevice_SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
        D3DDevice_DrawVerticesUP( D3DPT_QUADLIST, 4, verts, sizeof(verts[0]) );
    } 

    ////////////////////////////////////////////////////////////////
    // Draw border
    SET_TPAGE(-1);
    D3DDevice_SetTexture( 0, g_pBoxTexture );
    BLEND_OFF();

    D3DDevice_SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
    D3DDevice_SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
    D3DDevice_SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
    D3DDevice_SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA );
    D3DDevice_SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );

    FLOAT x1 = xPos;
    FLOAT x2 = xPos + BORDERSIZE;
    FLOAT x3 = xPos + xSize - BORDERSIZE;
    FLOAT x4 = xPos + xSize;

    FLOAT y1 = yPos;
    FLOAT y2 = yPos + BORDERSIZE;
    FLOAT y3 = yPos + ySize - BORDERSIZE;
    FLOAT y4 = yPos + ySize;

    FLOAT tu1 = 34 +  0.0f;
    FLOAT tu2 = 34 + 16.0f;
    FLOAT tu3 = 34 + 16.0f;
    FLOAT tu4 = 34 + 32.0f;
    FLOAT tv1 =  0.0f;
    FLOAT tv2 = 16.0f;
    FLOAT tv3 = 16.0f;
    FLOAT tv4 = 32.0f;

    WORD indices[] = 
    {
         4,  5,  6,  7, // Top left corner
         5,  8, 11,  6, // Top bar
         8,  9, 10, 11, // Top right corner
         7,  6, 17, 16, // Left bar
        11, 10, 13, 12, // Right bar
        16, 17, 18, 19, // Bottom left corner
        17, 12, 15, 18, // Bottom bar
        12, 13, 14, 15, // Bottom right corner
    };

    for( DWORD i=0; i<16; i++ )
    {
        verts[i+4].sz    = MENU_SPRU_Z;
        verts[i+4].rhw   = MENU_SPRU_RHW;
        verts[i+4].color = 0xffffffff;
    }

    // Top left
    verts[4].sx  = x1+20;    verts[4].sy  = y1-9;
    verts[5].sx  = x2+20;    verts[5].sy  = y1-9;
    verts[6].sx  = x2+20;    verts[6].sy  = y2-9-3;
    verts[7].sx  = x1+20;    verts[7].sy  = y2-9-3;

    verts[4].tu  = tu1-34;   verts[4].tv  = tv1+34;
    verts[5].tu  = tu2-34;   verts[5].tv  = tv1+34;
    verts[6].tu  = tu2-34;   verts[6].tv  = tv2+34-3;
    verts[7].tu  = tu1-34;   verts[7].tv  = tv2+34-3;

    // Top right
    verts[8].sx  = x3-20;    verts[8].sy  = y1-9;
    verts[9].sx  = x4-20;    verts[9].sy  = y1-9;
    verts[10].sx = x4-20;    verts[10].sy = y2-9-3;
    verts[11].sx = x3-20;    verts[11].sy = y2-9-3;

    verts[8].tu  = tu3-34;   verts[8].tv  = tv1+34;
    verts[9].tu  = tu4-34;   verts[9].tv  = tv1+34;
    verts[10].tu = tu4-34;   verts[10].tv = tv2+34-3;
    verts[11].tu = tu3-34;   verts[11].tv = tv2+34-3;

    D3DDevice_DrawIndexedVerticesUP( D3DPT_QUADLIST, 12, &indices[0], verts, sizeof(verts[0]) );

    // Bottom left
    verts[16].sx = x1+20;    verts[16].sy = y3+9+3;
    verts[17].sx = x2+20;    verts[17].sy = y3+9+3;
    verts[18].sx = x2+20;    verts[18].sy = y4+9;
    verts[19].sx = x1+20;    verts[19].sy = y4+9;

    verts[16].tu = tu1-34;   verts[16].tv = tv3+34+3;
    verts[17].tu = tu2-34;   verts[17].tv = tv3+34+3;
    verts[18].tu = tu2-34;   verts[18].tv = tv4+34;
    verts[19].tu = tu1-34;   verts[19].tv = tv4+34;

    // Bottom right
    verts[12].sx = x3-20;    verts[12].sy = y3+9+3;
    verts[13].sx = x4-20;    verts[13].sy = y3+9+3;
    verts[14].sx = x4-20;    verts[14].sy = y4+9;
    verts[15].sx = x3-20;    verts[15].sy = y4+9;

    verts[12].tu = tu3-34;   verts[12].tv = tv3+34+3;
    verts[13].tu = tu4-34;   verts[13].tv = tv3+34+3;
    verts[14].tu = tu4-34;   verts[14].tv = tv4+34;
    verts[15].tu = tu3-34;   verts[15].tv = tv4+34;

    D3DDevice_DrawIndexedVerticesUP( D3DPT_QUADLIST, 12, &indices[20], verts, sizeof(verts[0]) );

    // Top left
    verts[4].sx  = x1;    verts[4].sy  = y1;
    verts[5].sx  = x2;    verts[5].sy  = y1;
    verts[6].sx  = x2;    verts[6].sy  = y2;
    verts[7].sx  = x1;    verts[7].sy  = y2;

    verts[4].tu  = tu1;   verts[4].tv  = tv1;
    verts[5].tu  = tu2;   verts[5].tv  = tv1;
    verts[6].tu  = tu2;   verts[6].tv  = tv2;
    verts[7].tu  = tu1;   verts[7].tv  = tv2;

    // Top right
    verts[8].sx  = x3;    verts[8].sy  = y1;
    verts[9].sx  = x4;    verts[9].sy  = y1;
    verts[10].sx = x4;    verts[10].sy = y2;
    verts[11].sx = x3;    verts[11].sy = y2;

    verts[8].tu  = tu3;   verts[8].tv  = tv1;
    verts[9].tu  = tu4;   verts[9].tv  = tv1;
    verts[10].tu = tu4;   verts[10].tv = tv2;
    verts[11].tu = tu3;   verts[11].tv = tv2;

    // bottom right
    verts[12].sx = x3;    verts[12].sy = y3;
    verts[13].sx = x4;    verts[13].sy = y3;
    verts[14].sx = x4;    verts[14].sy = y4;
    verts[15].sx = x3;    verts[15].sy = y4;

    verts[12].tu = tu3;   verts[12].tv = tv3;
    verts[13].tu = tu4;   verts[13].tv = tv3;
    verts[14].tu = tu4;   verts[14].tv = tv4;
    verts[15].tu = tu3;   verts[15].tv = tv4;

    // Bottom left
    verts[16].sx = x1;    verts[16].sy = y3;
    verts[17].sx = x2;    verts[17].sy = y3;
    verts[18].sx = x2;    verts[18].sy = y4;
    verts[19].sx = x1;    verts[19].sy = y4;

    verts[16].tu = tu1;   verts[16].tv = tv3;
    verts[17].tu = tu2;   verts[17].tv = tv3;
    verts[18].tu = tu2;   verts[18].tv = tv4;
    verts[19].tu = tu1;   verts[19].tv = tv4;

    D3DDevice_DrawIndexedVerticesUP( D3DPT_QUADLIST, 32, indices, verts, sizeof(verts[0]) );
}

void DrawSpruBox(FLOAT xPos, FLOAT yPos, FLOAT xSize, FLOAT ySize, int col, int trans)
{
    VERTEX_TEX1 verts[4*5];
    FLOAT offU, offV;
    FLOAT menuSpruWidth, menuSpruHeight, menuCornerWidth, menuCornerHeight;

    // get border dimensions
    menuSpruWidth    = MENU_SPRU_WIDTH * gMenuWidthScale;
    menuSpruHeight   = MENU_SPRU_WIDTH * gMenuHeightScale;
    menuCornerWidth  = MENU_CORNER_WIDTH * gMenuWidthScale;
    menuCornerHeight = MENU_CORNER_HEIGHT * gMenuHeightScale;

    // Tpage offsets
    offV = (col * 66.0f) / 256.0f;
    if (trans == 1) {
        offU = 65.0f/256.0f;
    } else {
        offU = 0.0f/256.0f;
    }

    ZBUFFER_OFF();
    FOG_OFF();
    D3DDevice_SetVertexShader( FVF_TEX1 );

    if (trans == 0) 
    {
        // Draw background transparent poly
        verts[0].sx = xPos;
        verts[0].sy = yPos;
        verts[1].sx = xPos + xSize;
        verts[1].sy = yPos;
        verts[2].sx = xPos + xSize;
        verts[2].sy = yPos + ySize;
        verts[3].sx = xPos;
        verts[3].sy = yPos + ySize;

        verts[0].sz    = verts[1].sz    = verts[2].sz    = verts[3].sz    = MENU_SPRU_Z;
        verts[0].rhw   = verts[1].rhw   = verts[2].rhw   = verts[3].rhw   = MENU_SPRU_RHW;
        verts[0].color = verts[1].color = verts[2].color = verts[3].color = 0xb0181818;

        // Set colours
        BLEND_ALPHA();
        BLEND_SRC(D3DBLEND_SRCALPHA);
        BLEND_DEST(D3DBLEND_INVSRCALPHA);

        // Draw background poly
        SET_TPAGE(-1);
        D3DDevice_DrawVerticesUP( D3DPT_QUADLIST, 4, verts, sizeof(verts[0]) );
    } 
    else if (trans == 1) 
    {
        // Draw background solid poly
        verts[0].sx = xPos;
        verts[0].sy = yPos;
        verts[1].sx = xPos + xSize;
        verts[1].sy = yPos;
        verts[2].sx = xPos + xSize;
        verts[2].sy = yPos + ySize;
        verts[3].sx = xPos;
        verts[3].sy = yPos + ySize;

        verts[0].sz    = verts[1].sz    = verts[2].sz    = verts[3].sz    = MENU_SPRU_Z;
        verts[0].rhw   = verts[1].rhw   = verts[2].rhw   = verts[3].rhw   = MENU_SPRU_RHW;
        verts[0].color = verts[1].color = verts[2].color = verts[3].color = 0xffffffff;

        verts[0].tu = 85.0f/256.0f;     verts[0].tv = 20.0f/256.0f + offV;
        verts[1].tu = 110.0f/256.0f;    verts[1].tv = 20.0f/256.0f + offV;
        verts[2].tu = 110.0f/256.0f;    verts[2].tv = 50.0f/256.0f + offV;
        verts[3].tu = 85.0f/256.0f;     verts[3].tv = 50.0f/256.0f + offV;

        BLEND_OFF();
        SET_TPAGE(TPAGE_SPRU);
        D3DDevice_DrawVerticesUP( D3DPT_QUADLIST, 4, verts, sizeof(verts[0]) );
    }

    ////////////////////////////////////////////////////////////////
    // Draw border
    SET_TPAGE(TPAGE_SPRU);
    BLEND_OFF();

    FLOAT x1 = xPos - menuSpruWidth;
    FLOAT x2 = xPos - menuSpruWidth + menuCornerWidth;
    FLOAT x3 = xPos + menuSpruWidth + xSize - menuCornerWidth;
    FLOAT x4 = xPos + menuSpruWidth + xSize;

    FLOAT y1 = yPos - menuSpruHeight;
    FLOAT y2 = yPos - menuSpruHeight + menuCornerHeight;
    FLOAT y3 = yPos + menuSpruHeight + ySize - menuCornerHeight;
    FLOAT y4 = yPos + menuSpruHeight + ySize;

    FLOAT tu1 =  1.0f/256.0f + offU;
    FLOAT tu2 = 33.0f/256.0f + offU;
    FLOAT tu3 = 33.0f/256.0f + offU;
    FLOAT tu4 = 65.0f/256.0f + offU;
    FLOAT tv1 =  1.0f/256.0f + offV;
    FLOAT tv2 = 33.0f/256.0f + offV;
    FLOAT tv3 = 33.0f/256.0f + offV;
    FLOAT tv4 = 65.0f/256.0f + offV;

    WORD indices[] = 
    {
         4,  5,  6,  7, // Top left corner
         5,  8, 11,  6, // Top bar
         8,  9, 10, 11, // Top right corner
         7,  6, 17, 16, // Left bar
        11, 10, 13, 12, // Right bar
        16, 17, 18, 19, // Bottom left corner
        17, 12, 15, 18, // Bottom bar
        12, 13, 14, 15, // Bottom right corner
    };

    for( DWORD i=0; i<16; i++ )
    {
        verts[i+4].sz    = MENU_SPRU_Z;
        verts[i+4].rhw   = MENU_SPRU_RHW;
        verts[i+4].color = 0xffffffff;
    }

    // Top left
    verts[4].sx  = x1;    verts[4].sy  = y1;
    verts[5].sx  = x2;    verts[5].sy  = y1;
    verts[6].sx  = x2;    verts[6].sy  = y2;
    verts[7].sx  = x1;    verts[7].sy  = y2;

    verts[4].tu  = tu1;   verts[4].tv  = tv1;
    verts[5].tu  = tu2;   verts[5].tv  = tv1;
    verts[6].tu  = tu2;   verts[6].tv  = tv2;
    verts[7].tu  = tu1;   verts[7].tv  = tv2;

    // Top right
    verts[8].sx  = x3;    verts[8].sy  = y1;
    verts[9].sx  = x4;    verts[9].sy  = y1;
    verts[10].sx = x4;    verts[10].sy = y2;
    verts[11].sx = x3;    verts[11].sy = y2;

    verts[8].tu  = tu3;   verts[8].tv  = tv1;
    verts[9].tu  = tu4;   verts[9].tv  = tv1;
    verts[10].tu = tu4;   verts[10].tv = tv2;
    verts[11].tu = tu3;   verts[11].tv = tv2;

    // bottom right
    verts[12].sx = x3;    verts[12].sy = y3;
    verts[13].sx = x4;    verts[13].sy = y3;
    verts[14].sx = x4;    verts[14].sy = y4;
    verts[15].sx = x3;    verts[15].sy = y4;

    verts[12].tu = tu3;   verts[12].tv = tv3;
    verts[13].tu = tu4;   verts[13].tv = tv3;
    verts[14].tu = tu4;   verts[14].tv = tv4;
    verts[15].tu = tu3;   verts[15].tv = tv4;

    // Bottom left
    verts[16].sx = x1;    verts[16].sy = y3;
    verts[17].sx = x2;    verts[17].sy = y3;
    verts[18].sx = x2;    verts[18].sy = y4;
    verts[19].sx = x1;    verts[19].sy = y4;

    verts[16].tu = tu1;   verts[16].tv = tv3;
    verts[17].tu = tu2;   verts[17].tv = tv3;
    verts[18].tu = tu2;   verts[18].tv = tv4;
    verts[19].tu = tu1;   verts[19].tv = tv4;

    D3DDevice_DrawIndexedVerticesUP( D3DPT_QUADLIST, 32, indices, verts, sizeof(verts[0]) );
}




////////////////////////////////////////////////////////////////
//
// DrawScale:
//
////////////////////////////////////////////////////////////////
void DrawScale( FLOAT percent, FLOAT xPos, FLOAT yPos, FLOAT xSize, FLOAT ySize )
{
    VERTEX_TEX1 verts[4];
    VERTEX_TEX1 *vert;
    long col, r, g, b;
    static FLOAT timer = ZERO;

    vert = verts;

    xPos  *= gMenuWidthScale;
    yPos  *= gMenuHeightScale;
    xSize *= gMenuWidthScale * percent / 100.0f;
    ySize *= gMenuHeightScale;


    // Calculate coordinates of box corners
    vert[0].sx = xPos;
    vert[0].sy = yPos;
    vert[1].sx = xPos + xSize;
    vert[1].sy = yPos;
    vert[2].sx = xPos + xSize;
    vert[2].sy = yPos + ySize;
    vert[3].sx = xPos;
    vert[3].sy = yPos + ySize;

//$MODIFIED(cprince) - near/far clipping occurs even if ZENABLE is false, so set reasonable z value
//    vert[0].sz = vert[1].sz = vert[2].sz = vert[3].sz = 300;
    vert[0].sz = vert[1].sz = vert[2].sz = vert[3].sz = 0;
//$END_MODIFICATIONS
    vert[0].rhw = vert[1].rhw = vert[2].rhw = vert[3].rhw = 1;

    // Set colours
    //timer += TimeStep;
    //col = (long)(255.0 * (1.0 + sin(timer * 2)) / 2);
    //col = col << 24 | 0xffffff;
    r = (long)(255.0f - percent * 2.55f);
    if (r < 0) r = 0;

    g = (long)(percent * 2.55f);
    if (g > 255) g = 255;

    b = 0;

    col = 0xbb << 24 | r << 16 | g << 8 | b;

    vert[0].color = vert[3].color = 0xbbff0000;
    vert[1].color = vert[2].color = col;

    // set uvs
    vert[0].tu =   0.0f / 256.0f;
    vert[0].tv = 230.0f / 256.0f;
    vert[1].tu = 182.0f / 256.0f;
    vert[1].tv = 230.0f / 256.0f;
    vert[2].tu = 182.0f / 256.0f;
    vert[2].tv = 256.0f / 256.0f;
    vert[3].tu =   0.0f / 256.0f;
    vert[3].tv = 256.0f / 256.0f;

    // Draw background poly
    SET_TPAGE(-1);
    FOG_OFF();
    BLEND_SRC(D3DBLEND_ONE);
    BLEND_DEST(D3DBLEND_ONE);
    BLEND_ON();

    D3DDevice_SetVertexShader( FVF_TEX1 );
    D3DDevice_DrawVerticesUP( D3DPT_QUADLIST, 4, vert, sizeof(vert[0]) );
}




////////////////////////////////////////////////////////////////
//
// Draw Slider
//
////////////////////////////////////////////////////////////////
void DrawSliderDataSlider(FLOAT xPos, FLOAT yPos, long value, long min, long max, bool active)
{
    FLOAT fraction, xSize, ySize;
    VERTEX_TEX1 vert[4];
    long col, r, g, b;
    static FLOAT timer = ZERO;

    SET_TPAGE(-1);
    FOG_OFF();
    ZBUFFER_OFF();
    BLEND_SRC(D3DBLEND_SRCALPHA);
    BLEND_DEST(D3DBLEND_INVSRCALPHA);
    BLEND_ALPHA();

    fraction = ((FLOAT)(value - min)) / (max-min);

    // Draw background poly

    xPos *= gMenuWidthScale;
    yPos = (yPos + MENU_TEXT_HEIGHT) * gMenuHeightScale;
    xSize = gMenuWidthScale * MENU_DATA_WIDTH_SLIDER;
    ySize = gMenuHeightScale * MENU_TEXT_HEIGHT;

    yPos += 3;

    vert[0].sx = xPos;
    vert[0].sy = yPos - 2;
    vert[1].sx = xPos + xSize;
    vert[1].sy = yPos - ySize - 2;
    vert[2].sx = xPos + xSize;
    vert[2].sy = yPos;
    vert[3].sx = xPos;
    vert[3].sy = yPos;

//$MODIFIED(cprince) - near/far clipping occurs even if ZENABLE is false, so set reasonable z value
//    vert[0].sz = vert[1].sz = vert[2].sz = vert[3].sz = 300;
    vert[0].sz = vert[1].sz = vert[2].sz = vert[3].sz = 0;
//$END_MODIFICATIONS
    vert[0].rhw = vert[1].rhw = vert[2].rhw = vert[3].rhw = 1;

    col = 0x80000000;
    vert[0].color = vert[1].color = vert[2].color = vert[3].color = col;

    DRAW_PRIM(D3DPT_TRIANGLEFAN, FVF_TEX1, vert, 4, D3DDP_DONOTUPDATEEXTENTS);


    // Foreground poly

    xSize = gMenuWidthScale * fraction * MENU_DATA_WIDTH_SLIDER;
    ySize = gMenuHeightScale * MENU_TEXT_HEIGHT * fraction;


    // Calculate coordinates of box corners
    vert[0].sx = xPos;
    vert[0].sy = yPos - 2;
    vert[1].sx = xPos + xSize;
    vert[1].sy = yPos - ySize - 2;
    vert[2].sx = xPos + xSize;
    vert[2].sy = yPos;
    vert[3].sx = xPos;
    vert[3].sy = yPos;

    r = (long)(255.0f - fraction * 255.0f);
    if (r < 0) r = 0;

    g = (long)(fraction * 255.0f);
    if (g > 255) g = 255;

    b = 0;

    col = 0xbb << 24 | r << 16 | g << 8 | b;

    vert[0].color = vert[3].color = 0xbbff0000;
    vert[1].color = vert[2].color = col;

    if (active) 
    {
        BLEND_SRC(D3DBLEND_ONE);
        BLEND_DEST(D3DBLEND_ONE);
        BLEND_ON();
    }

    DRAW_PRIM(D3DPT_TRIANGLEFAN, FVF_TEX1, vert, 4, D3DDP_DONOTUPDATEEXTENTS);
}



