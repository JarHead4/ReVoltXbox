//-----------------------------------------------------------------------------
// File: MenuDraw.cpp
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------

//$TODO(cprince): I think there are places in this file where they use
/// screen-space coords, but they don't do the necessary half-pixel offset.
/// Need to look into this.

#include "Revolt.h"
#include "MenuDraw.h"
#include "MenuText.h"
#include "TitleScreen.h"
#include "Timing.h"
#include "Texture.h"
#include "Input.h"
#include "DrawObj.h"
#include "SoundEffectEngine.h"
#include "SoundEffects.h"
#include "MainMenu.h"
#include "Text.h"
#include "XBFont.h"



#define MENU_LOGO_WIDTH     160
#define MENU_LOGO_HEIGHT    160
#define MENU_LOGO_YPOS      20
#define MENU_LOGO_FADE_TIME TO_TIME(Real(0.5))

#define MENU_IMAGE_WIDTH    64

#pragma message( "Hey: why did z=300 work in DX6 PC version?" )
#define MENU_SPRU_Z         0 //$MODIFIED: was originally 300
#define MENU_SPRU_RHW       1
#define MENU_SPRU_WIDTH     4
#define MENU_CORNER_WIDTH   16
#define MENU_CORNER_HEIGHT  16

// prototypes

void DrawSpruBox(FLOAT xPos, FLOAT yPos, FLOAT xSize, FLOAT ySize, int col, int trans);
void DrawSpruArrow(FLOAT xPos, FLOAT yPos, long Type, long flags);
void DrawMenuDirectionArrows( MENU_HEADER* pMenuHeader );
void DrawMenuTitleImage(FLOAT xPos, FLOAT yPos, FLOAT xSize, FLOAT ySize, int colIndex, int imageIndex);
void DrawMenuTitleImageAdjoiningBar(FLOAT xPos, FLOAT yPos, FLOAT width, int colIndex);
void DrawSliderDataSlider(FLOAT xPos, FLOAT yPos, long value, long min, long max, bool active);

// Globals
char MenuBuffer[256];
char MenuBuffer2[256];

FLOAT gMenuWidthScale  = 1.0f;
FLOAT gMenuHeightScale = 1.0f;





//-----------------------------------------------------------------------------
// Name: DrawScreenSpaceQuad()
// Desc: Renders a screen space, textured quad at the given offset. The
//       width and height are taken from the texture, which is assumed to be
//       linear.
//-----------------------------------------------------------------------------
VOID DrawScreenSpaceQuad( FLOAT sx, FLOAT sy, D3DTexture* pTexture )
{
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
    D3DDevice_SetTextureStageState( 0, D3DTSS_COLORKEYOP, D3DTCOLORKEYOP_DISABLE );
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
}





////////////////////////////////////////////////////////////////
//
// DrawMenuText: Draw menu text, but add three dots to end of
// string if it will be bigger than the passed "maxLen".
//
////////////////////////////////////////////////////////////////
VOID DrawMenuText( FLOAT sx, FLOAT sy, DWORD color, CHAR* strText )
{
	WCHAR strBuffer[128];
	swprintf( strBuffer, L"%S", strText );
	g_pMenuFont->DrawText( sx, sy, 0xff000000|color, strBuffer );

	// Restore state after drawing text
	D3DDevice_SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
    D3DDevice::SetRenderState( D3DRS_CULLMODE,        D3DCULL_NONE );
	RenderZbuffer = D3DZB_FALSE;
	RenderBlend   = 0;
}


VOID DrawMenuText( FLOAT sx, FLOAT sy, DWORD color, WCHAR* strText )
{
	g_pMenuFont->DrawText( sx, sy, 0xff000000|color, strText );

	// Restore state after drawing text
	D3DDevice_SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
    D3DDevice::SetRenderState( D3DRS_CULLMODE,        D3DCULL_NONE );
	RenderZbuffer = D3DZB_FALSE;
	RenderBlend   = 0;
}




////////////////////////////////////////////////////////////////
//
// DrawMenuTitle:
//
////////////////////////////////////////////////////////////////
void DrawMenuTitle( MENU_HEADER* pMenuHeader )
{
	// Set state for drawing menus
	D3DDevice_SetTextureStageState( 0, D3DTSS_COLORKEYOP, D3DTCOLORKEYOP_RGBA );

    FLOAT xPos, yPos, xSize;
    if (pMenuHeader->strTitle == NULL) return;
    if (pMenuHeader->pMenu == NULL) return;

    xSize = strlen(pMenuHeader->strTitle) * MENU_TEXT_WIDTH * 2.0f;
    xPos = 24 + MENU_IMAGE_WIDTH + MENU_TEXT_GAP + 20;//(640 - xSize) / 2;
    yPos = 24;

    DrawSpruBox(
        gMenuWidthScale * (xPos - MENU_TEXT_GAP), 
        gMenuHeightScale * (yPos - MENU_TEXT_VSKIP + (MENU_IMAGE_WIDTH - MENU_TEXT_HEIGHT * 2)/2),
        gMenuWidthScale * (xSize + MENU_TEXT_GAP * 2),
        gMenuHeightScale * (MENU_TEXT_HEIGHT * 2 + MENU_TEXT_VSKIP * 2), 
        pMenuHeader->SpruColIndex, 0);

    //xPos -= MENU_IMAGE_WIDTH + MENU_TEXT_GAP + 12;
    xPos = 24;

    if (pMenuHeader->pMenu->ImageIndex != MENU_IMAGE_NONE) {
        DrawSpruBox(
            gMenuWidthScale * (xPos - MENU_TEXT_HSKIP), 
            gMenuHeightScale * (yPos - MENU_TEXT_VSKIP),
            gMenuWidthScale * (MENU_IMAGE_WIDTH + MENU_TEXT_HSKIP * 2),
            gMenuHeightScale * (MENU_IMAGE_WIDTH + MENU_TEXT_VSKIP * 2), 
            pMenuHeader->SpruColIndex, 1);

        DrawMenuTitleImage(
            gMenuWidthScale * (xPos), 
            gMenuHeightScale * (yPos),
            gMenuWidthScale * (MENU_IMAGE_WIDTH),
            gMenuHeightScale * (MENU_IMAGE_WIDTH), 
            pMenuHeader->SpruColIndex,
            pMenuHeader->pMenu->ImageIndex);

        DrawMenuTitleImageAdjoiningBar(
            gMenuWidthScale * (xPos + MENU_IMAGE_WIDTH + MENU_TEXT_HSKIP * 2),
            gMenuHeightScale * 50,
            gMenuWidthScale * (20 - MENU_TEXT_HSKIP * 2),
            pMenuHeader->SpruColIndex);

    }

    xPos = 24 + MENU_IMAGE_WIDTH + MENU_TEXT_GAP + 20;//(640 - xSize) / 2;
    yPos = 24 + 4;

	WCHAR strTitle[128];
	swprintf( strTitle, L"%S", pMenuHeader->strTitle );
	g_pMenuTitleFont->DrawText( xPos, yPos + (MENU_IMAGE_WIDTH - MENU_TEXT_HEIGHT * 2)/2, 
                                0xff000000|MENU_TEXT_RGB_TITLE, strTitle );

	// Restore state after drawing text
	D3DDevice_SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
    D3DDevice::SetRenderState( D3DRS_CULLMODE,        D3DCULL_NONE );
	RenderZbuffer = D3DZB_FALSE;
	RenderBlend   = 0;
}


////////////////////////////////////////////////////////////////
//
// DrawLogo
//
////////////////////////////////////////////////////////////////
void DrawMenuLogo( MENU_HEADER* pMenuHeader )
{
    static FLOAT menuLogoFadeTimer = ZERO;//MENU_LOGO_FADE_TIME;

    // Fade timing
    if (pMenuHeader->pMenu != &Menu_TopLevel) {
        menuLogoFadeTimer -= TimeStep;
    } else {
        menuLogoFadeTimer += TimeStep;
    }

    if (menuLogoFadeTimer < ZERO) {
        menuLogoFadeTimer = ZERO;
        return;
    }
    if (menuLogoFadeTimer > MENU_LOGO_FADE_TIME) {
        menuLogoFadeTimer = MENU_LOGO_FADE_TIME;
    }

    DrawDemoLogo(menuLogoFadeTimer / MENU_LOGO_FADE_TIME, 1);
}


////////////////////////////////////////////////////////////////
//
// Draw Menu Direction Arrows
//
////////////////////////////////////////////////////////////////
#define MENU_DIR_ARROW_WIDTH        40
#define MENU_DIR_ARROW_HEIGHT       20
#define MENU_DIR_BUTTON_WIDTH       44
#define MENU_DIR_BUTTON_HEIGHT      44
#define MENU_DIR_SPACE              0

enum {
    MENU_DIR_ARROW_UP,
    MENU_DIR_ARROW_DOWN,
    MENU_DIR_ARROW_LEFT,
    MENU_DIR_ARROW_RIGHT,
    MENU_DIR_BUTTON,

    MENU_DIR_NTYPES
};


void DrawMenuDirectionArrows( MENU_HEADER* pMenuHeader )
{
    FLOAT xPos, yPos;

    // Left Arrow
    xPos = 320.0f * gMenuWidthScale;
    yPos = 400 * gMenuHeightScale;
    DrawSpruArrow(xPos, yPos, MENU_DIR_BUTTON, 0);
    DrawSpruArrow(xPos, yPos, MENU_DIR_ARROW_LEFT, 0);
    DrawSpruArrow(xPos, yPos, MENU_DIR_ARROW_RIGHT, 0);
    DrawSpruArrow(xPos, yPos, MENU_DIR_ARROW_UP, 0);
    DrawSpruArrow(xPos, yPos, MENU_DIR_ARROW_DOWN, 0);
}

////////////////////////////////////////////////////////////////
//
// Draw data integer
//
////////////////////////////////////////////////////////////////
VOID DrawMenuDataInt( MENU_HEADER* pMenuHeader, MENU* pMenu, 
					  MENU_ITEM* pMenuItem, int itemIndex )
{
	WCHAR strBuffer[10];
    swprintf( strBuffer, L"%d", *(int*)pMenuItem->pData );

    FLOAT sx = pMenuHeader->XPos + pMenuHeader->ItemTextWidth + MENU_TEXT_GAP;;
    FLOAT sy = pMenuHeader->YPos + (MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP) * itemIndex;
    
	if( pMenuItem->ActiveFlags & MENU_ITEM_ACTIVE )
        DrawMenuText( sx, sy, MENU_TEXT_RGB_NORMAL, strBuffer );
    else
        DrawMenuText( sx, sy, MENU_TEXT_RGB_LOLITE, strBuffer );
}




////////////////////////////////////////////////////////////////
//
// Draw data On/Off
//
////////////////////////////////////////////////////////////////
VOID DrawMenuDataOnOff( MENU_HEADER* pMenuHeader, MENU* pMenu, 
					    MENU_ITEM* pMenuItem, int itemIndex )
{
    BOOL b = *(BOOL*)pMenuItem->pData;

    FLOAT sx = pMenuHeader->XPos + pMenuHeader->ItemTextWidth + MENU_TEXT_GAP;
    FLOAT sy = pMenuHeader->YPos + (MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP) * itemIndex;
    
	if( pMenuItem->ActiveFlags & MENU_ITEM_ACTIVE ) 
        DrawMenuText( sx, sy, MENU_TEXT_RGB_NORMAL, (b)? gTitleScreen_Text[TEXT_ON]: gTitleScreen_Text[TEXT_OFF] );
    else 
        DrawMenuText( sx, sy, MENU_TEXT_RGB_LOLITE, (b)? gTitleScreen_Text[TEXT_ON]: gTitleScreen_Text[TEXT_OFF] );
}

VOID DrawMenuDataYesNo( MENU_HEADER* pMenuHeader, MENU* pMenu, 
					    MENU_ITEM* pMenuItem, int itemIndex )
{
    BOOL b = *(BOOL*)pMenuItem->pData;

    FLOAT sx = pMenuHeader->XPos + pMenuHeader->ItemTextWidth + MENU_TEXT_GAP;
    FLOAT sy = pMenuHeader->YPos + (MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP) * itemIndex;

    if( pMenuItem->ActiveFlags & MENU_ITEM_ACTIVE ) 
        DrawMenuText( sx, sy, MENU_TEXT_RGB_NORMAL, (b)? TEXT_TABLE(TEXT_YES): TEXT_TABLE(TEXT_NO) );
    else 
        DrawMenuText( sx, sy, MENU_TEXT_RGB_LOLITE, (b)? TEXT_TABLE(TEXT_YES): TEXT_TABLE(TEXT_NO) );
}




////////////////////////////////////////////////////////////////
//
// Draw data DWORD
//
////////////////////////////////////////////////////////////////
VOID DrawMenuDataDWORD( MENU_HEADER* pMenuHeader, MENU* pMenu, 
					    MENU_ITEM* pMenuItem, int itemIndex )
{
	WCHAR strBuffer[10];
    swprintf( strBuffer, L"%d", *(DWORD*)pMenuItem->pData );

    FLOAT sx = pMenuHeader->XPos + pMenuHeader->ItemTextWidth + MENU_TEXT_GAP;
    FLOAT sy = pMenuHeader->YPos + (MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP) * itemIndex;

    DrawMenuText( sx, sy, MENU_TEXT_RGB_NORMAL, strBuffer );
}




////////////////////////////////////////////////////////////////
//
// Draw Slider (long)
//
////////////////////////////////////////////////////////////////
VOID DrawSliderDataLong( MENU_HEADER* pMenuHeader, MENU* pMenu, 
					     MENU_ITEM* pMenuItem, int itemIndex )
{
    SLIDER_DATA_LONG *sliderData = (SLIDER_DATA_LONG*)pMenuItem->pData;

    FLOAT sx = pMenuHeader->XPos + pMenuHeader->ItemTextWidth + MENU_TEXT_GAP;
    FLOAT sy = pMenuHeader->YPos + (MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP) * itemIndex;

    if (sliderData->bDrawSlider) 
	{
        DrawSliderDataSlider( sx, sy, *sliderData->pData, sliderData->Min, sliderData->Max, (pMenuItem->ActiveFlags & MENU_ITEM_ACTIVE) != 0);
        sx += MENU_DATA_WIDTH_SLIDER + MENU_TEXT_HSKIP;
        sprintf(MenuBuffer, "%d%%", *(sliderData->pData));
    } 
	else 
	{
        sprintf(MenuBuffer, "%d", *(sliderData->pData));
    }


    if( pMenuItem->ActiveFlags & MENU_ITEM_ACTIVE )
        DrawMenuText( sx, sy, MENU_TEXT_RGB_NORMAL, MenuBuffer );
    else 
        DrawMenuText( sx, sy, MENU_TEXT_RGB_LOLITE, MenuBuffer );
}

// unsigned long
void DrawSliderDataULong( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem, int itemIndex )
{
    SLIDER_DATA_ULONG *sliderData = (SLIDER_DATA_ULONG*)pMenuItem->pData;

    FLOAT sx = pMenuHeader->XPos + pMenuHeader->ItemTextWidth + MENU_TEXT_GAP;
    FLOAT sy = pMenuHeader->YPos + (MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP) * itemIndex;

    if (sliderData->bDrawSlider) 
	{
        DrawSliderDataSlider(sx, sy, *sliderData->pData, sliderData->Min, sliderData->Max, (pMenuItem->ActiveFlags & MENU_ITEM_ACTIVE) != 0);
        sx += MENU_DATA_WIDTH_SLIDER + MENU_TEXT_HSKIP;
        sprintf(MenuBuffer, "%d%%", *(sliderData->pData));
    } 
	else 
	{
        sprintf(MenuBuffer, "%d", *(sliderData->pData));
    }

    if (pMenuItem->ActiveFlags & MENU_ITEM_ACTIVE)
        DrawMenuText( sx, sy, MENU_TEXT_RGB_NORMAL, MenuBuffer );
    else 
        DrawMenuText( sx, sy, MENU_TEXT_RGB_LOLITE, MenuBuffer );
}

// string table
void DrawSliderDataStringTable( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem, int itemIndex )
{
    char *string;
    
	SLIDER_DATA_STRING_TABLE* pSliderData = (SLIDER_DATA_STRING_TABLE*)pMenuItem->pData;

    string = pSliderData->pstrStringTable[pSliderData->dwCurrentString];

    FLOAT sx = pMenuHeader->XPos + pMenuHeader->ItemTextWidth + MENU_TEXT_GAP;
    FLOAT sy = pMenuHeader->YPos + (MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP) * itemIndex;

    DrawMenuText( sx, sy, MENU_TEXT_RGB_NORMAL, string );
}

////////////////////////////////////////////////////////////////
//
// Menu Not Implemented
//
////////////////////////////////////////////////////////////////

void DrawMenuNotImplemented( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem, int itemIndex )
{
    FLOAT sx = pMenuHeader->XPos + pMenuHeader->ItemTextWidth + MENU_TEXT_GAP;
    FLOAT sy = pMenuHeader->YPos + (MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP) * itemIndex;

    DrawMenuText( sx, sy, 0x000088ff, L"Not Implemented" );
}




/////////////////////////////////////////////////////////////////////////////////
//
// LoadFrontEndTextures
//
/////////////////////////////////////////////////////////////////////////////////
void LoadFrontEndTextures()
{
	// Revolt logo
    LoadMipTexture( "D:\\levels\\frontend\\carbox1.bmp", TPAGE_FX1, 256, 256, 0, 1, FALSE );
    LoadMipTexture( "D:\\levels\\frontend\\carbox2.bmp", TPAGE_FX2, 256, 256, 0, 1, FALSE );
    LoadMipTexture( "D:\\levels\\frontend\\carbox3.bmp", TPAGE_FX3, 256, 256, 0, 1, FALSE );
}




////////////////////////////////////////////////////////////////
//
// Free FrontEnd textures
//
////////////////////////////////////////////////////////////////
void FreeFrontEndTextures()
{
}


////////////////////////////////////////////////////////////////
//
// Draw menu title bar icon
//
////////////////////////////////////////////////////////////////
long MenuTitleImageRGB[3] = 
{
    0x802f3fb6,
    0x80af353b,
    0x80e6c200
};

void DrawMenuTitleImageAdjoiningBar( FLOAT xPos, FLOAT yPos, FLOAT width, int colIndex )
{
    VERTEX_TEX1 verts[4];
    FLOAT offU, offV;

    verts[0].sx = xPos - MENU_SPRU_WIDTH * gMenuWidthScale / 4;
    verts[0].sy = yPos;
    verts[1].sx = xPos + width - MENU_SPRU_WIDTH * gMenuWidthScale * 3 / 4;
    verts[1].sy = yPos;
    verts[2].sx = xPos + width - MENU_SPRU_WIDTH * gMenuWidthScale * 3 / 4;
    verts[2].sy = yPos + MENU_SPRU_WIDTH * 2 * gMenuHeightScale;
    verts[3].sx = xPos - MENU_SPRU_WIDTH * 2 * gMenuWidthScale / 2;
    verts[3].sy = yPos + MENU_SPRU_WIDTH * 2 * gMenuHeightScale;

    verts[0].sz = verts[1].sz = verts[2].sz = verts[3].sz = MENU_SPRU_Z;
    verts[0].rhw = verts[1].rhw = verts[2].rhw = verts[3].rhw = MENU_SPRU_RHW;

    verts[0].color = verts[1].color = verts[2].color = verts[3].color = MENU_SPRU_RGB;

    offU = 0.0f;
    offV = (colIndex * 66.0f) / 256.0f;

    verts[0].tu = 32.0f/256.0f + offU; verts[0].tv = 1.0f/256.0f + offV;
    verts[1].tu = 32.0f/256.0f + offU; verts[1].tv = 1.0f/256.0f + offV;
    verts[2].tu = 34.0f/256.0f + offU; verts[2].tv = 6.0f/256.0f + offV;
    verts[3].tu = 34.0f/256.0f + offU; verts[3].tv = 6.0f/256.0f + offV;

    ZBUFFER_OFF();
    FOG_OFF();
    BLEND_OFF();
    SET_TPAGE(TPAGE_SPRU);

    DRAW_PRIM(D3DPT_TRIANGLEFAN, FVF_TEX1, verts, 4, D3DDP_DONOTUPDATEEXTENTS);
}

void DrawMenuTitleImage(FLOAT xPos, FLOAT yPos, FLOAT xSize, FLOAT ySize, int colIndex, int imageIndex)
{
    VERTEX_TEX1 verts[4];
    FLOAT offU, offV;

    verts[0].sx = xPos;
    verts[0].sy = yPos;
    verts[1].sx = xPos + xSize;
    verts[1].sy = yPos;
    verts[2].sx = xPos + xSize;
    verts[2].sy = yPos + ySize;
    verts[3].sx = xPos;
    verts[3].sy = yPos + ySize;

    verts[0].sz = verts[1].sz = verts[2].sz = verts[3].sz = MENU_SPRU_Z;
    verts[0].rhw = verts[1].rhw = verts[2].rhw = verts[3].rhw = MENU_SPRU_RHW;

    offU = (imageIndex % 5) * 48.0f/256.0f;
    offV = (imageIndex / 5) * 48.0f/256.0f;

    verts[0].tu = 1.0f/256.0f + offU; verts[0].tv = 1.0f/256.0f + offV;
    verts[1].tu = 48.0f/256.0f + offU; verts[1].tv = 1.0f/256.0f + offV;
    verts[2].tu = 48.0f/256.0f + offU; verts[2].tv = 48.0f/256.0f + offV;
    verts[3].tu = 1.0f/256.0f + offU; verts[3].tv = 48.0f/256.0f + offV;

    Assert((colIndex >= 0) && (colIndex <3));
    verts[0].color = verts[1].color = verts[2].color = verts[3].color = 0x80ffffff;//MenuTitleImageRGB[colIndex];

    ZBUFFER_OFF();
    FOG_OFF();

    SET_TPAGE(TPAGE_WORLD_START+6);
    BLEND_SRC(D3DBLEND_ONE);
    BLEND_DEST(D3DBLEND_ONE);
    BLEND_ON();

    DRAW_PRIM(D3DPT_TRIANGLEFAN, FVF_TEX1, verts, 4, D3DDP_DONOTUPDATEEXTENTS);


    SET_TPAGE(TPAGE_WORLD_START+5);
    BLEND_SRC(D3DBLEND_ZERO);
    BLEND_DEST(D3DBLEND_INVSRCCOLOR);
    BLEND_ON();

    DRAW_PRIM(D3DPT_TRIANGLEFAN, FVF_TEX1, verts, 4, D3DDP_DONOTUPDATEEXTENTS);

}

////////////////////////////////////////////////////////////////
//
// Draw Spru Box: the position and width given are the inside
// pos and dimensions.
//
////////////////////////////////////////////////////////////////

void DrawSpruBox(FLOAT xPos, FLOAT yPos, FLOAT xSize, FLOAT ySize, int col, int trans)
{
    VERTEX_TEX1 verts[4];
    FLOAT tlX, tlY, trX, trY, blX, blY, brX, brY, offU, offV;
    FLOAT menuSpruWidth, menuSpruHeight, menuCornerWidth, menuCornerHeight;

    // get border dimensions
    menuSpruWidth = MENU_SPRU_WIDTH * gMenuWidthScale;
    menuSpruHeight = MENU_SPRU_WIDTH * gMenuHeightScale;
    menuCornerWidth = MENU_CORNER_WIDTH * gMenuWidthScale;
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

    if (trans == 0) {
        ////////////////////////////////////////////////////////////////
        // Draw background transparent poly
        verts[0].sx = xPos;
        verts[0].sy = yPos;
        verts[1].sx = xPos + xSize;
        verts[1].sy = yPos;
        verts[2].sx = xPos + xSize;
        verts[2].sy = yPos + ySize;
        verts[3].sx = xPos;
        verts[3].sy = yPos + ySize;

        verts[0].sz = verts[1].sz = verts[2].sz = verts[3].sz = MENU_SPRU_Z;
        verts[0].rhw = verts[1].rhw = verts[2].rhw = verts[3].rhw = MENU_SPRU_RHW;

        // Set colours
        verts[0].color = verts[1].color = verts[2].color = verts[3].color = MENU_SPRU_RGB_BACK;
        BLEND_ALPHA();
        BLEND_SRC(D3DBLEND_SRCALPHA);
        BLEND_DEST(D3DBLEND_INVSRCALPHA);

        // Draw background poly
        SET_TPAGE(-1);

        DRAW_PRIM(D3DPT_TRIANGLEFAN, FVF_TEX1, verts, 4, D3DDP_DONOTUPDATEEXTENTS);

    } else if (trans == 1) {
        ////////////////////////////////////////////////////////////////
        // Draw background solid poly
        verts[0].sx = xPos;
        verts[0].sy = yPos;
        verts[1].sx = xPos + xSize;
        verts[1].sy = yPos;
        verts[2].sx = xPos + xSize;
        verts[2].sy = yPos + ySize;
        verts[3].sx = xPos;
        verts[3].sy = yPos + ySize;

        verts[0].sz = verts[1].sz = verts[2].sz = verts[3].sz = MENU_SPRU_Z;
        verts[0].rhw = verts[1].rhw = verts[2].rhw = verts[3].rhw = MENU_SPRU_RHW;

        verts[0].tu = 85.0f/256.0f; verts[0].tv = 20.0f/256.0f + offV;
        verts[1].tu = 110.0f/256.0f;    verts[1].tv = 20.0f/256.0f + offV;
        verts[2].tu = 110.0f/256.0f;    verts[2].tv = 50.0f/256.0f + offV;
        verts[3].tu = 85.0f/256.0f; verts[3].tv = 50.0f/256.0f + offV;

        verts[0].color = verts[1].color = verts[2].color = verts[3].color = 0xffffffff;

        BLEND_OFF();
        SET_TPAGE(TPAGE_SPRU);
        DRAW_PRIM(D3DPT_TRIANGLEFAN, FVF_TEX1, verts, 4, D3DDP_DONOTUPDATEEXTENTS);
    }

    ////////////////////////////////////////////////////////////////
    // Draw border
    SET_TPAGE(TPAGE_SPRU);
    BLEND_OFF();

    tlX = xPos - menuSpruWidth;
    tlY = yPos - menuSpruHeight;
    trX = xPos + xSize + menuSpruWidth;
    trY = yPos - menuSpruHeight;

    blX = xPos - menuSpruWidth;
    blY = yPos + ySize + menuSpruHeight;
    brX = xPos + xSize + menuSpruWidth;
    brY = yPos + ySize + menuSpruHeight;

    // Top left
    verts[0].sx = tlX;
    verts[0].sy = tlY;
    verts[1].sx = tlX + menuCornerWidth;
    verts[1].sy = tlY;
    verts[2].sx = tlX + menuCornerWidth;
    verts[2].sy = tlY + menuCornerHeight;
    verts[3].sx = tlX;
    verts[3].sy = trY + menuCornerWidth;

    verts[0].sz = verts[1].sz = verts[2].sz = verts[3].sz = MENU_SPRU_Z;
    verts[0].rhw = verts[1].rhw = verts[2].rhw = verts[3].rhw = MENU_SPRU_RHW;

    verts[0].color = verts[1].color = verts[2].color = verts[3].color = MENU_SPRU_RGB;

    verts[0].tu = 1.0f/256.0f + offU; verts[0].tv = 1.0f/256.0f + offV;
    verts[1].tu = 33.0f/256.0f + offU; verts[1].tv = 1.0f/256.0f + offV;
    verts[2].tu = 33.0f/256.0f + offU; verts[2].tv = 33.0f/256.0f + offV;
    verts[3].tu = 1.0f/256.0f + offU; verts[3].tv = 33.0f/256.0f + offV;

    DRAW_PRIM(D3DPT_TRIANGLEFAN, FVF_TEX1, verts, 4, D3DDP_DONOTUPDATEEXTENTS);


    // Top right
    verts[0].sx = trX - menuCornerWidth;
    verts[0].sy = trY;
    verts[1].sx = trX;
    verts[1].sy = trY;
    verts[2].sx = trX;
    verts[2].sy = trY + menuCornerHeight;
    verts[3].sx = trX - menuCornerWidth;
    verts[3].sy = trY + menuCornerHeight;

    verts[0].sz = verts[1].sz = verts[2].sz = verts[3].sz = MENU_SPRU_Z;
    verts[0].rhw = verts[1].rhw = verts[2].rhw = verts[3].rhw = MENU_SPRU_RHW;

    verts[0].color = verts[1].color = verts[2].color = verts[3].color = MENU_SPRU_RGB;

    verts[0].tu = 33.0f/256.0f + offU; verts[0].tv = 1.0f/256.0f + offV;
    verts[1].tu = 65.0f/256.0f + offU; verts[1].tv = 1.0f/256.0f + offV;
    verts[2].tu = 65.0f/256.0f + offU; verts[2].tv = 33.0f/256.0f + offV;
    verts[3].tu = 33.0f/256.0f + offU; verts[3].tv = 33.0f/256.0f + offV;

    DRAW_PRIM(D3DPT_TRIANGLEFAN, FVF_TEX1, verts, 4, D3DDP_DONOTUPDATEEXTENTS);


    // bottom right
    verts[0].sx = brX - menuCornerWidth;
    verts[0].sy = brY - menuCornerHeight;
    verts[1].sx = brX;
    verts[1].sy = brY - menuCornerHeight;
    verts[2].sx = brX;
    verts[2].sy = brY;
    verts[3].sx = brX - menuCornerWidth;
    verts[3].sy = brY;

    verts[0].sz = verts[1].sz = verts[2].sz = verts[3].sz = MENU_SPRU_Z;
    verts[0].rhw = verts[1].rhw = verts[2].rhw = verts[3].rhw = MENU_SPRU_RHW;

    verts[0].color = verts[1].color = verts[2].color = verts[3].color = MENU_SPRU_RGB;

    verts[0].tu = 33.0f/256.0f + offU; verts[0].tv = 33.0f/256.0f + offV;
    verts[1].tu = 65.0f/256.0f + offU; verts[1].tv = 33.0f/256.0f + offV;
    verts[2].tu = 65.0f/256.0f + offU; verts[2].tv = 65.0f/256.0f + offV;
    verts[3].tu = 33.0f/256.0f + offU; verts[3].tv = 65.0f/256.0f + offV;

    DRAW_PRIM(D3DPT_TRIANGLEFAN, FVF_TEX1, verts, 4, D3DDP_DONOTUPDATEEXTENTS);


    // Bottom left
    verts[0].sx = blX;
    verts[0].sy = blY - menuCornerHeight;
    verts[1].sx = blX + menuCornerWidth;
    verts[1].sy = blY - menuCornerHeight;
    verts[2].sx = blX + menuCornerWidth;
    verts[2].sy = blY;
    verts[3].sx = blX;
    verts[3].sy = blY;

    verts[0].sz = verts[1].sz = verts[2].sz = verts[3].sz = MENU_SPRU_Z;
    verts[0].rhw = verts[1].rhw = verts[2].rhw = verts[3].rhw = MENU_SPRU_RHW;

    verts[0].color = verts[1].color = verts[2].color = verts[3].color = MENU_SPRU_RGB;

    verts[0].tu = 1.0f/256.0f + offU; verts[0].tv = 33.0f/256.0f + offV;
    verts[1].tu = 33.0f/256.0f + offU; verts[1].tv = 33.0f/256.0f + offV;
    verts[2].tu = 33.0f/256.0f + offU; verts[2].tv = 65.0f/256.0f + offV;
    verts[3].tu = 1.0f/256.0f + offU; verts[3].tv = 65.0f/256.0f + offV;

    DRAW_PRIM(D3DPT_TRIANGLEFAN, FVF_TEX1, verts, 4, D3DDP_DONOTUPDATEEXTENTS);


    // top bar
    verts[0].sx = tlX + menuCornerWidth;
    verts[0].sy = tlY;
    verts[1].sx = trX - menuCornerWidth;
    verts[1].sy = trY;
    verts[2].sx = trX - menuCornerWidth;
    verts[2].sy = trY + menuCornerHeight;
    verts[3].sx = tlX + menuCornerWidth;
    verts[3].sy = trY + menuCornerHeight;

    verts[0].sz = verts[1].sz = verts[2].sz = verts[3].sz = MENU_SPRU_Z;
    verts[0].rhw = verts[1].rhw = verts[2].rhw = verts[3].rhw = MENU_SPRU_RHW;

    verts[0].color = verts[1].color = verts[2].color = verts[3].color = MENU_SPRU_RGB;

    verts[0].tu = 32.0f/256.0f + offU; verts[0].tv = 1.0f/256.0f + offV;
    verts[1].tu = 32.0f/256.0f + offU; verts[1].tv = 1.0f/256.0f + offV;
    verts[2].tu = 34.0f/256.0f + offU; verts[2].tv = 33.0f/256.0f + offV;
    verts[3].tu = 34.0f/256.0f + offU; verts[3].tv = 33.0f/256.0f + offV;

    DRAW_PRIM(D3DPT_TRIANGLEFAN, FVF_TEX1, verts, 4, D3DDP_DONOTUPDATEEXTENTS);


    // bottom bar
    verts[0].sx = blX + menuCornerWidth;
    verts[0].sy = blY - menuCornerHeight;
    verts[1].sx = brX - menuCornerWidth;
    verts[1].sy = brY - menuCornerHeight;
    verts[2].sx = brX - menuCornerWidth;
    verts[2].sy = brY;
    verts[3].sx = blX + menuCornerWidth;
    verts[3].sy = brY;

    verts[0].sz = verts[1].sz = verts[2].sz = verts[3].sz = MENU_SPRU_Z;
    verts[0].rhw = verts[1].rhw = verts[2].rhw = verts[3].rhw = MENU_SPRU_RHW;

    verts[0].color = verts[1].color = verts[2].color = verts[3].color = MENU_SPRU_RGB;

    verts[0].tu = 32.0f/256.0f + offU; verts[0].tv = 33.0f/256.0f + offV;
    verts[1].tu = 32.0f/256.0f + offU; verts[1].tv = 33.0f/256.0f + offV;
    verts[2].tu = 34.0f/256.0f + offU; verts[2].tv = 65.0f/256.0f + offV;
    verts[3].tu = 34.0f/256.0f + offU; verts[3].tv = 65.0f/256.0f + offV;

    DRAW_PRIM(D3DPT_TRIANGLEFAN, FVF_TEX1, verts, 4, D3DDP_DONOTUPDATEEXTENTS);


    // left bar
    verts[0].sx = tlX;
    verts[0].sy = tlY + menuCornerHeight;
    verts[1].sx = tlX + menuCornerWidth;
    verts[1].sy = tlY + menuCornerHeight;
    verts[2].sx = blX + menuCornerWidth;
    verts[2].sy = blY - menuCornerHeight;
    verts[3].sx = blX;
    verts[3].sy = blY - menuCornerHeight;

    verts[0].sz = verts[1].sz = verts[2].sz = verts[3].sz = MENU_SPRU_Z;
    verts[0].rhw = verts[1].rhw = verts[2].rhw = verts[3].rhw = MENU_SPRU_RHW;

    verts[0].color = verts[1].color = verts[2].color = verts[3].color = MENU_SPRU_RGB;

    verts[0].tu = 1.0f/256.0f + offU; verts[0].tv = 32.0f/256.0f + offV;
    verts[1].tu = 33.0f/256.0f + offU; verts[1].tv = 32.0f/256.0f + offV;
    verts[2].tu = 33.0f/256.0f + offU; verts[2].tv = 34.0f/256.0f + offV;
    verts[3].tu = 1.0f/256.0f + offU; verts[3].tv = 34.0f/256.0f + offV;

    DRAW_PRIM(D3DPT_TRIANGLEFAN, FVF_TEX1, verts, 4, D3DDP_DONOTUPDATEEXTENTS);


    // right bar
    verts[0].sx = trX - menuCornerWidth;
    verts[0].sy = trY + menuCornerHeight;
    verts[1].sx = trX;
    verts[1].sy = trY + menuCornerHeight;
    verts[2].sx = brX;
    verts[2].sy = brY - menuCornerHeight;
    verts[3].sx = brX - menuCornerWidth;
    verts[3].sy = brY - menuCornerHeight;

    verts[0].sz = verts[1].sz = verts[2].sz = verts[3].sz = MENU_SPRU_Z;
    verts[0].rhw = verts[1].rhw = verts[2].rhw = verts[3].rhw = MENU_SPRU_RHW;

    verts[0].color = verts[1].color = verts[2].color = verts[3].color = MENU_SPRU_RGB;

    verts[0].tu = 33.0f/256.0f + offU; verts[0].tv = 32.0f/256.0f + offV;
    verts[1].tu = 65.0f/256.0f + offU; verts[1].tv = 32.0f/256.0f + offV;
    verts[2].tu = 65.0f/256.0f + offU; verts[2].tv = 34.0f/256.0f + offV;
    verts[3].tu = 33.0f/256.0f + offU; verts[3].tv = 34.0f/256.0f + offV;

    DRAW_PRIM(D3DPT_TRIANGLEFAN, FVF_TEX1, verts, 4, D3DDP_DONOTUPDATEEXTENTS);
}


////////////////////////////////////////////////////////////////
//
// Draw Spru Arrow:
//
////////////////////////////////////////////////////////////////



typedef struct SpruArrowStruct {
    FLOAT XOff, YOff;
    FLOAT Width, Height;
    struct UVStruct {
        FLOAT tu, tv;
    } UVs[4];
} SPRU_ARROW_DATA;
    
SPRU_ARROW_DATA SpruArrowData[MENU_DIR_NTYPES] = {
    { // Up Arrow
        -MENU_DIR_ARROW_WIDTH/2, -MENU_DIR_BUTTON_HEIGHT/2 - MENU_DIR_ARROW_HEIGHT + 2, // Position relative to centre of arrow array
        MENU_DIR_ARROW_WIDTH, MENU_DIR_ARROW_HEIGHT,                                // Width, height
        0.0f/256.0f, 1.0f/256.0f + 213.0f/256.0f,                                   // UVs
        63.0f/256.0f, 1.0f/256.0f + 213.0f/256.0f,
        63.0f/256.0f, 41.0f/256.0f + 213.0f/256.0f,
        0.0f/256.0f, 41.0f/256.0f + 213.0f/256.0f,
    },
    { // Down Arrow
        -MENU_DIR_ARROW_WIDTH/2, MENU_DIR_BUTTON_HEIGHT/2 - 2,                      // Position relative to centre of arrow array
        MENU_DIR_ARROW_WIDTH, MENU_DIR_ARROW_HEIGHT,                                // Width, height
        0.0f/256.0f + 62.0f/256.0f, 41.0f/256.0f + 213.0f/256.0f,
        63.0f/256.0f + 62.0f/256.0f, 41.0f/256.0f + 213.0f/256.0f,
        63.0f/256.0f + 62.0f/256.0f, 1.0f/256.0f + 213.0f/256.0f,
        0.0f/256.0f + 62.0f/256.0f, 1.0f/256.0f + 213.0f/256.0f,                    // UVs
    },
    { // Left Arrow
        -MENU_DIR_ARROW_HEIGHT - MENU_DIR_BUTTON_WIDTH/2 + 2, -MENU_DIR_ARROW_WIDTH/2,  // Position relative to centre of arrow array
        MENU_DIR_ARROW_HEIGHT, MENU_DIR_ARROW_WIDTH,                                // Width, height
        63.0f/256.0f + 124.0f/256.0f, 1.0f/256.0f + 213.0f/256.0f,
        63.0f/256.0f + 124.0f/256.0f, 41.0f/256.0f + 213.0f/256.0f,
        0.0f/256.0f + 124.0f/256.0f, 41.0f/256.0f + 213.0f/256.0f,
        0.0f/256.0f + 124.0f/256.0f, 1.0f/256.0f + 213.0f/256.0f,                   // UVs
    },
    { // Right Arrow
        MENU_DIR_BUTTON_HEIGHT/2 - 2, -MENU_DIR_ARROW_WIDTH/2,                      // Position relative to centre of arrow array
        MENU_DIR_ARROW_HEIGHT, MENU_DIR_ARROW_WIDTH,                                // Width, height
        63.0f/256.0f + 124.0f/256.0f, 41.0f/256.0f + 213.0f/256.0f,                 // UVs
        63.0f/256.0f + 124.0f/256.0f, 1.0f/256.0f + 213.0f/256.0f,
        0.0f/256.0f + 124.0f/256.0f, 1.0f/256.0f + 213.0f/256.0f,
        0.0f/256.0f + 124.0f/256.0f, 41.0f/256.0f + 213.0f/256.0f,
    },
    { // Button
        -MENU_DIR_BUTTON_WIDTH/2, -MENU_DIR_BUTTON_HEIGHT/2,                // Position relative to centre of arrow array
        MENU_DIR_BUTTON_WIDTH, MENU_DIR_BUTTON_HEIGHT,                              // Width, height
        184.0f/256.0f, 71.0f/256.0f,
        254.0f/256.0f, 71.0f/256.0f,
        254.0f/256.0f, 143.0f/256.0f,
        184.0f/256.0f, 143.0f/256.0f,
    },
};


void DrawSpruArrow(FLOAT xPos, FLOAT yPos, long type, long flags)
{
    VERTEX_TEX1 verts[4];
    FLOAT xSize, ySize, offU, offV;

    xSize = 32;
    ySize = 32;
    offU = 0.0f;//(1.0f + 1.0f * 65.0f) / 256.0f;
    offV = 213.0f / 256.0f;

    verts[0].sx = xPos + SpruArrowData[type].XOff;
    verts[0].sy = yPos + SpruArrowData[type].YOff;
    verts[1].sx = xPos + SpruArrowData[type].XOff + SpruArrowData[type].Width;
    verts[1].sy = yPos + SpruArrowData[type].YOff;
    verts[2].sx = xPos + SpruArrowData[type].XOff + SpruArrowData[type].Width;
    verts[2].sy = yPos + SpruArrowData[type].YOff + SpruArrowData[type].Height;
    verts[3].sx = xPos + SpruArrowData[type].XOff;
    verts[3].sy = yPos + SpruArrowData[type].YOff + SpruArrowData[type].Height;

    verts[0].tu = SpruArrowData[type].UVs[0].tu;    verts[0].tv = SpruArrowData[type].UVs[0].tv;
    verts[1].tu = SpruArrowData[type].UVs[1].tu;    verts[1].tv = SpruArrowData[type].UVs[1].tv;
    verts[2].tu = SpruArrowData[type].UVs[2].tu;    verts[2].tv = SpruArrowData[type].UVs[2].tv;
    verts[3].tu = SpruArrowData[type].UVs[3].tu;    verts[3].tv = SpruArrowData[type].UVs[3].tv;

    verts[0].sz = verts[1].sz = verts[2].sz = verts[3].sz = MENU_SPRU_Z;
    verts[0].rhw = verts[1].rhw = verts[2].rhw = verts[3].rhw = MENU_SPRU_RHW;

    // Set colours
    verts[0].color = verts[1].color = verts[2].color = verts[3].color = MENU_SPRU_RGB;

    // Draw background poly
    SET_TPAGE(TPAGE_SPRU);
    BLEND_OFF();

    DRAW_PRIM(D3DPT_TRIANGLEFAN, FVF_TEX1, verts, 4, D3DDP_DONOTUPDATEEXTENTS);

}



////////////////////////////////////////////////////////////////
//
// DrawScale:
//
////////////////////////////////////////////////////////////////

void DrawScale(FLOAT percent, FLOAT xPos, FLOAT yPos, FLOAT xSize, FLOAT ySize)
{
    VERTEX_TEX1 verts[4];
    VERTEX_TEX1 *vert;
    long col, r, g, b;
    static FLOAT timer = ZERO;

    vert = verts;

    xPos *= gMenuWidthScale;
    yPos *= gMenuHeightScale;
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

    vert[0].sz = vert[1].sz = vert[2].sz = vert[3].sz = 300;
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
    vert[0].tu = 0.0f / 256.0f;
    vert[0].tv = 230.0f / 256.0f;
    vert[1].tu = 182.0f / 256.0f;
    vert[1].tv = 230.0f / 256.0f;
    vert[2].tu = 182.0f / 256.0f;
    vert[2].tv = 256.0f / 256.0f;
    vert[3].tu = 0.0f / 256.0f;
    vert[3].tv = 256.0f / 256.0f;

    // Draw background poly
    SET_TPAGE(-1);
    FOG_OFF();
    BLEND_SRC(D3DBLEND_ONE);
    BLEND_DEST(D3DBLEND_ONE);
    BLEND_ON();

    DRAW_PRIM(D3DPT_TRIANGLEFAN, FVF_TEX1, vert, 4, D3DDP_DONOTUPDATEEXTENTS);

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

    
    fraction = ((FLOAT)(value - min)) / max;

    // Draw background poly

    xPos *= gMenuWidthScale;
    yPos = (yPos + MENU_TEXT_HEIGHT) * gMenuHeightScale;
    xSize = gMenuWidthScale * MENU_DATA_WIDTH_SLIDER;
    ySize = gMenuHeightScale * MENU_TEXT_HEIGHT;


    vert[0].sx = xPos;
    vert[0].sy = yPos - 2;
    vert[1].sx = xPos + xSize;
    vert[1].sy = yPos - ySize - 2;
    vert[2].sx = xPos + xSize;
    vert[2].sy = yPos;
    vert[3].sx = xPos;
    vert[3].sy = yPos;

    vert[0].sz = vert[1].sz = vert[2].sz = vert[3].sz = 300;
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

    if (active) {
        BLEND_SRC(D3DBLEND_ONE);
        BLEND_DEST(D3DBLEND_ONE);
        BLEND_ON();
    }

    DRAW_PRIM(D3DPT_TRIANGLEFAN, FVF_TEX1, vert, 4, D3DDP_DONOTUPDATEEXTENTS);


}




