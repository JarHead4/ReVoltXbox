//-----------------------------------------------------------------------------
// File: Text.cpp
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "Revolt.h"
#include "Draw.h"
#include "DX.h"
#include "Main.h"
#include "Geom.h"
#include "Text.h"
#include "XBFont.h"


// globals
CXBFont* g_pSmallGameFont    = NULL;  // 8x12 game font
CXBFont* g_pGameFont         = NULL;  // 8x16 game font
CXBFont* g_pWideGameFont     = NULL;  // 12x16 game font
CXBFont* g_pBigGameFont      = NULL;  // 12x24 game font
CXBFont* g_pBigWideGameFont  = NULL;  // 16x24 game font
CXBFont* g_pExtraBigGameFont = NULL;  // 16x32 game font
CXBFont* g_pMenuFont         = NULL;  // Font for menus
CXBFont* g_pMenuTitleFont    = NULL;  // Font for menu titles


// ASCII conversion characters
static unsigned char AsciiOld[] = "àÀèÈìÌòÒùÙáÁéÉíÍóÓúÚâÂêÊîÎôÔûÛäÄëËïÏöÖüÜçÇñÑº¡¿ß°";
static unsigned char AsciiNew[] = "aAeEiIoOuUaAeEiIoOuUaAeEiIoOuUaAeEiIoOuU\x87\x86nN\x88\x89\x8a\x8d\x88";

static unsigned char AsciiOldBig[] = "áÁéÉíÍóÓúÚàÀèÈìÌòÒùÙâÂêÊîÎôÔûÛäÄëËïÏöÖüÜçÇñÑº¡¿ß°";
static unsigned char AsciiNewBig[] = "AAEEIIOOUUAAEEIIOOUUAAEEIIOOUUAAEEIIOOUU\x60\x60NN\x61\x62\x63\x65\x61";




//////////////////////
// begin text state //
//////////////////////
VOID BeginTextState()
{
    ZBUFFER_OFF();
    BLEND_OFF();
    FOG_OFF();
    SET_TPAGE(TPAGE_FONT);
}




//-----------------------------------------------------------------------------
// Name: LoadFonts()
// Desc: 
//-----------------------------------------------------------------------------
HRESULT LoadFonts()
{
	// Construct the font classes
	g_pSmallGameFont     = new CXBFont();
	g_pGameFont          = new CXBFont();
	g_pWideGameFont      = new CXBFont();
	g_pBigGameFont       = new CXBFont();
	g_pBigWideGameFont   = new CXBFont();
	g_pExtraBigGameFont  = new CXBFont();
	g_pMenuFont          = new CXBFont();
	g_pMenuTitleFont     = new CXBFont();

	// Create the fonts from their resource files
	if( FAILED( g_pSmallGameFont->Create( "d:\\gfx\\GameFont.xpr" ) ) )
		return E_FAIL;
	if( FAILED( g_pGameFont->Create( "d:\\gfx\\GameFont.xpr" ) ) )
		return E_FAIL;
	if( FAILED( g_pWideGameFont->Create( "d:\\gfx\\GameFont.xpr" ) ) )
		return E_FAIL;
	if( FAILED( g_pBigGameFont->Create( "d:\\gfx\\GameFont.xpr" ) ) )
		return E_FAIL;
	if( FAILED( g_pBigWideGameFont->Create( "d:\\gfx\\GameFont.xpr" ) ) )
		return E_FAIL;
	if( FAILED( g_pExtraBigGameFont->Create( "d:\\gfx\\GameFont.xpr" ) ) )
		return E_FAIL;
	if( FAILED( g_pMenuFont->Create( "d:\\gfx\\NarrowFont.xpr" ) ) )
		return E_FAIL;
	if( FAILED( g_pMenuTitleFont->Create( "d:\\gfx\\BigFont.xpr" ) ) )
		return E_FAIL;

	// Patch up the menu font, which contains special characters
	for( DWORD i=0; i<8; i++ )
	{
		g_pMenuFont->m_Glyphs[128-32+i].left     = (19*(i%4)+0)/127.0f;
		g_pMenuFont->m_Glyphs[128-32+i].right    = (19*(i%4)+16+1)/127.0f;
		g_pMenuFont->m_Glyphs[128-32+i].top      = (22*(i/4)+164)/255.0f;
		g_pMenuFont->m_Glyphs[128-32+i].bottom   = (22*(i/4)+164+19+1)/255.0f;
		g_pMenuFont->m_Glyphs[128-32+i].wAdvance = 18;
		g_pMenuFont->m_Glyphs[128-32+i].wOffset  = 2;
		g_pMenuFont->m_Glyphs[128-32+i].wWidth   = 16;
	}

	return S_OK;
}




// This is for drawing game text that is normally scaled at 8x16 pixels
VOID DrawSmallGameText( FLOAT sx, FLOAT sy, DWORD color, CHAR* strText )
{
	WCHAR strBuffer[80];
	swprintf( strBuffer, L"%S", strText );
	g_pSmallGameFont->DrawText( sx, sy, color, strBuffer );

	// Restore state after drawing text
	D3DDevice_SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
    D3DDevice::SetRenderState( D3DRS_CULLMODE,        D3DCULL_NONE );
    D3DDevice::SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR );
    D3DDevice::SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );
	RenderZbuffer = D3DZB_FALSE;
	RenderBlend   = 0;
}

// This is for drawing game text that is normally scaled at 8x16 pixels
VOID DrawGameText( FLOAT sx, FLOAT sy, DWORD color, CHAR* strText )
{
	WCHAR strBuffer[80];
	swprintf( strBuffer, L"%S", strText );
	g_pGameFont->DrawText( sx, sy, color, strBuffer );

	// Restore state after drawing text
	D3DDevice_SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
    D3DDevice::SetRenderState( D3DRS_CULLMODE,        D3DCULL_NONE );
    D3DDevice::SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR );
    D3DDevice::SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );
	RenderZbuffer = D3DZB_FALSE;
	RenderBlend   = 0;
}

// This is for drawing game text that is normally scaled at 12x16 pixels
VOID DrawWideGameText( FLOAT sx, FLOAT sy, DWORD color, CHAR* strText )
{
	WCHAR strBuffer[80];
	swprintf( strBuffer, L"%S", strText );
	g_pWideGameFont->DrawText( sx, sy, color, strBuffer );

	// Restore state after drawing text
	D3DDevice_SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
    D3DDevice::SetRenderState( D3DRS_CULLMODE,        D3DCULL_NONE );
    D3DDevice::SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR );
    D3DDevice::SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );
	RenderZbuffer = D3DZB_FALSE;
	RenderBlend   = 0;
}

// This is for drawing game text that is normally scaled at 12x24 pixels
VOID DrawBigGameText( FLOAT sx, FLOAT sy, DWORD color, CHAR* strText )
{
	WCHAR strBuffer[80];
	swprintf( strBuffer, L"%S", strText );
	g_pBigGameFont->DrawText( sx, sy, color, strBuffer );

	// Restore state after drawing text
	D3DDevice_SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
    D3DDevice::SetRenderState( D3DRS_CULLMODE,        D3DCULL_NONE );
    D3DDevice::SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR );
    D3DDevice::SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );
	RenderZbuffer = D3DZB_FALSE;
	RenderBlend   = 0;
}

// This is for drawing game text that is normally scaled at 12x24 pixels
VOID DrawBigWideGameText( FLOAT sx, FLOAT sy, DWORD color, CHAR* strText )
{
	WCHAR strBuffer[80];
	swprintf( strBuffer, L"%S", strText );
	g_pBigWideGameFont->DrawText( sx, sy, color, strBuffer );

	// Restore state after drawing text
	D3DDevice_SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
    D3DDevice::SetRenderState( D3DRS_CULLMODE,        D3DCULL_NONE );
    D3DDevice::SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR );
    D3DDevice::SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );
	RenderZbuffer = D3DZB_FALSE;
	RenderBlend   = 0;
}

// This is for drawing game text that is normally scaled at 16x32 pixels
VOID DrawExtraBigGameText( FLOAT sx, FLOAT sy, DWORD color, CHAR* strText )
{
	WCHAR strBuffer[80];
	swprintf( strBuffer, L"%S", strText );
	g_pExtraBigGameFont->DrawText( sx, sy, color, strBuffer );

	// Restore state after drawing text
	D3DDevice_SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
    D3DDevice::SetRenderState( D3DRS_CULLMODE,        D3DCULL_NONE );
    D3DDevice::SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR );
    D3DDevice::SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );
	RenderZbuffer = D3DZB_FALSE;
	RenderBlend   = 0;
}




//////////////////////////////
// draw text to back buffer //
//////////////////////////////
void DumpText3D(VEC *pos, float xs, float ys, long color, char *text)
{
    char i;
    float tu, tv, sz, rhw;
    float x, y;
    long lu, lv, ch;
	static VERTEX_TEX1 TextVert[4];

	// set tpage
    SET_TPAGE(TPAGE_FONT);

	// calc size / pos
    xs = xs * RenderSettings.GeomPers / pos->v[Z] * RenderSettings.GeomScaleX;
    ys = ys * RenderSettings.GeomPers / pos->v[Z] * RenderSettings.GeomScaleY;
    x = pos->v[X] * RenderSettings.GeomPers / pos->v[Z] * RenderSettings.GeomScaleX + RenderSettings.GeomCentreX;
    y = pos->v[Y] * RenderSettings.GeomPers / pos->v[Z] * RenderSettings.GeomScaleY + RenderSettings.GeomCentreY;

	// init vert misc
    sz = GET_ZBUFFER(pos->v[Z]);
    rhw = 1 / pos->v[Z];

    for (i = 0 ; i < 4 ; i++)
    {
        TextVert[i].color = color;
        TextVert[i].sz = sz;
        TextVert[i].rhw = rhw;
    }

	// draw chars
    while (*text)
    {
		// get char
        ch = *text - 33;
        if (ch < -1) ch += 256;
        if (ch != -1)
        {
			// set screen coors
            TextVert[0].sx = x;
            TextVert[0].sy = y;

            TextVert[1].sx = (x + xs);
            TextVert[1].sy = y;

            TextVert[2].sx = (x + xs);
            TextVert[2].sy = (y + ys);

            TextVert[3].sx = x;
            TextVert[3].sy = (y + ys);

			// set uv's
            lu = ch % FONT_PER_ROW;
            lv = ch / FONT_PER_ROW;

            tu = (float)lu * FONT_WIDTH + 1.0f;
            tv = (float)lv * FONT_HEIGHT + 1.0f;

            TextVert[0].tu = tu / 256;
            TextVert[0].tv = tv / 256;

            TextVert[1].tu = (tu + FONT_UWIDTH - 1.0f) / 256;
            TextVert[1].tv = tv / 256;

            TextVert[2].tu = (tu + FONT_UWIDTH - 1.0f) / 256;
            TextVert[2].tv = (tv + FONT_VHEIGHT) / 256;

            TextVert[3].tu = tu / 256;
            TextVert[3].tv = (tv + FONT_VHEIGHT) / 256;

			// draw
            DRAW_PRIM(D3DPT_TRIANGLEFAN, FVF_TEX1, TextVert, 4, D3DDP_DONOTUPDATEEXTENTS);
        }

		// next
        x += xs;
        text++;
    }
}









