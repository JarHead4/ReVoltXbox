//-----------------------------------------------------------------------------
// File: dx.cpp
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"
#include "dx.h"
//$REMOVED#include "dxerrors.h"
#include "main.h"
#include "texture.h"
#include "input.h"
#include "camera.h"
#include "network.h"
#include "settings.h"
#include "timing.h"
#include "draw.h"
#include "gamegauge.h"

// globals

DX_STATE DxState;
//$REMOVEDIDirectDraw4 *DD = NULL;
//$REMOVEDIDirectDrawSurface4 *FrontBuffer = NULL;
//$REMOVEDIDirectDrawSurface4 *BackBuffer = NULL;
//$REMOVEDIDirectDrawSurface4 *ZedBuffer = NULL;
//$REMOVEDIDirectDrawGammaControl *GammaControl = NULL;
IDirect3D8 *D3D = NULL;
IDirect3DDevice8 *D3Ddevice = NULL;
//$REMOVEDIDirect3DViewport3 *D3Dviewport;
//$REMOVEDDDCAPS DDcaps;
//$REMOVEDD3DDEVICEDESC D3Dcaps;
DDPIXELFORMAT ZedBufferFormat;
DWORD ScreenXsize;
DWORD ScreenYsize;
DWORD ScreenBpp;
//$REMOVEDDWORD ScreenRefresh;
long GammaFlag = GAMMA_UNAVAILABLE;
long NoColorKey = FALSE; //$MODIFIED: was originally TRUE
//$REMOVEDlong DrawDeviceNum, CurrentDrawDevice;
//$REMOVEDlong DisplayModeCount;
//$REMOVEDDRAW_DEVICE DrawDevices[MAX_DRAW_DEVICES];
long RenderStateChange, TextureStateChange;
long BackBufferCount;
long BlendSubtract;
DWORD BackgroundColor;

short RenderTP = -1;
short RenderTP2 = -1;
short RenderFog = FALSE;
short RenderBlend = -1;
short RenderBlendSrc = -1;
short RenderBlendDest = -1;
short RenderZbuffer = D3DZB_TRUE;
short RenderZwrite = TRUE;
short RenderZcmp = D3DCMP_LESSEQUAL;

//$REMOVEDstatic DWORD TotalScreenMem, TotalTexMem;
static long PolyClear;

//////////////////
// Init dx misc //
//////////////////

BOOL InitDX(void) //$RENAMED: was InitDD
{
//$MODIFIED
//    HRESULT r;
//    DDSCAPS2 ddscaps2;
//    DWORD temp;
//
//// release
//
//    ReleaseDX();
//
//// create draw device
//
//    DirectDrawEnumerate(CreateDrawDeviceCallback, NULL);
//    CurrentDrawDevice = RegistrySettings.DrawDevice;
//
//// get device caps
//
//    ZeroMemory(&DDcaps, sizeof(DDcaps));
//    DDcaps.dwSize = sizeof(DDcaps);
//
//    r = DD->GetCaps(&DDcaps, NULL);
//    if (r != DD_OK)
//    {
//        ErrorDX(r, "Can't get DD device caps");
//        return FALSE;
//    }
//
//// disable window if voodoo
//
//    if (!FullScreen && !(DDcaps.dwCaps2 & DDCAPS2_CANRENDERWINDOWED))
//    {
//        DumpMessage(NULL,"Cannot run in windowed mode.  Switching to fullscreen.");
//        FullScreen = TRUE;
//    }
//
//// get total screen / texture mem
//
//    ddscaps2.dwCaps = DDSCAPS_PRIMARYSURFACE;
//    DD->GetAvailableVidMem(&ddscaps2, &TotalScreenMem, &temp);
//
//    ddscaps2.dwCaps = DDSCAPS_TEXTURE;
//    DD->GetAvailableVidMem(&ddscaps2, &TotalTexMem, &temp);
//
//// set exclusive mode
//
//    if (FullScreen)
//    {
//        r = DD->SetCooperativeLevel(hwnd, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN | DDSCL_ALLOWMODEX | DDSCL_ALLOWREBOOT | DDSCL_FPUSETUP);
//        if (r != DD_OK)
//        {
//            ErrorDX(r, "Can't set coop level");
//            return FALSE;
//        }
//    }
//    else
//    {
//        r = DD->SetCooperativeLevel(hwnd, DDSCL_NORMAL | DDSCL_ALLOWREBOOT | DDSCL_FPUSETUP);
//        if (r != DD_OK)
//        {
//            ErrorDX(r, "Can't set coop level");
//            return FALSE;
//        }
//    }
//
//// get 3D interface
//
//    r = DD->QueryInterface(IID_IDirect3D3, (void**)&D3D);
//    if (r != DD_OK)
//    {
//        ErrorDX(r, "Can't get 3D interface");
//        return FALSE;
//    }

    D3D = Direct3DCreate8(D3D_SDK_VERSION);
    if( NULL == D3D )
    {
        ErrorDX( 0, "Unable to create Direct3D!\n" );
        return FALSE;
    }

//$END_MODIFICATIONS

// return OK

    return TRUE;
}

///////////////////
// Init D3D misc //
///////////////////

BOOL InitD3D(DWORD width, DWORD height, DWORD bpp, DWORD refresh)
{
    HRESULT r;
    D3DVIEWPORT2 vd;

	// release
    ReleaseD3D();

	// set screen params
    ScreenXsize = width;
    ScreenYsize = height;
    ScreenBpp = bpp;
    ZedBufferFormat = D3DFMT_D24S8;  //$REVISIT: their enum function searched for 16-bit z buffer, but I assume we want a 32-bit depth buffer.

    BackBufferCount = 1;


//$ADDITION
    D3DPRESENT_PARAMETERS d3dpp;
    ZeroMemory( &d3dpp, sizeof(d3dpp) );

    d3dpp.BackBufferWidth        = ScreenXsize;
    d3dpp.BackBufferHeight       = ScreenYsize;
    d3dpp.BackBufferFormat       = (ScreenBpp == 32) ? (D3DFMT_A8R8G8B8) : (  (ScreenBpp == 16) ? (D3DFMT_R5G6B5) : (D3DFMT_UNKNOWN)  );
    d3dpp.BackBufferCount        = BackBufferCount;  //$REVISIT: do we only want 1 back buffer on Xbox, despite what they may request here?
    d3dpp.EnableAutoDepthStencil = TRUE;
    d3dpp.AutoDepthStencilFormat = ZedBufferFormat;
    d3dpp.MultiSampleType        = D3DMULTISAMPLE_NONE;
    d3dpp.SwapEffect             = D3DSWAPEFFECT_DISCARD;
    d3dpp.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE; //$REVISIT: we probably don't want IMMEDIATE when we release.
//$END_ADDITION


	// create D3D device
    r = D3D->CreateDevice(D3DADAPTER_DEFAULT,
                          D3DDEVTYPE_HAL,
                          XBOX_UNUSED_PARAM,
                          D3DCREATE_HARDWARE_VERTEXPROCESSING,
                          &d3dpp,
                          &D3Ddevice);
    if( r != D3D_OK )
    {
        ErrorDX(r, "Can't create a 3D device!");
        return FALSE;
    }

	// create / setup viewport
    vd.X      = 0;
    vd.Y      = 0;
    vd.Width  = ScreenXsize;
    vd.Height = ScreenYsize;
    vd.MinZ   = 0.0f;
    vd.MaxZ   = 1.0f;

    D3DDevice_SetViewport( &vd );

	// get gamma control interface
    if(NoGamma)
    {
        GammaFlag = GAMMA_UNAVAILABLE;
    }
    else
    {
        GammaFlag = GAMMA_AVAILABLE;
        SetGamma(RegistrySettings.Brightness, RegistrySettings.Contrast);
    }

    PolyClear = FALSE; //$NOTE(cprince): can probably just remove 'PolyClear' var eventually!

	// set misc render states
    CULL_OFF();
    SPECULAR_OFF();
    TEXTURE_ADDRESS(D3DTADDRESS_CLAMP);
    MIPMAP_LODBIAS(-0.02f);  //$REVISIT(cprince): do we really want this?

    SET_STAGE_STATE(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    SET_STAGE_STATE(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
    SET_STAGE_STATE(0, D3DTSS_COLOROP, D3DTOP_MODULATE);

    SET_STAGE_STATE(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    SET_STAGE_STATE(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
    SET_STAGE_STATE(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);

    SET_RENDER_STATE(D3DRENDERSTATE_ALPHATESTENABLE, TRUE);
    SET_RENDER_STATE(D3DRENDERSTATE_ALPHAFUNC, D3DCMP_GREATEREQUAL);
    SET_RENDER_STATE(D3DRENDERSTATE_ALPHAREF, AlphaRef);

// set BlendSubtract flag

    BlendSubtract = true;  //$NOTE: set true b/c Xbox supports D3DBLEND_ZERO and D3DBLEND_INVSRCCOLOR

    return TRUE;
}

//////////////////////
// set gamma values //
//////////////////////

void SetGamma(long brightness, long contrast)
{
#ifndef XBOX_NOT_YET_IMPLEMENTED
    long i;
    float step, middle, n;
    DDGAMMARAMP ramp;

// skip if no gamma control

    if (GammaFlag == GAMMA_UNAVAILABLE)
        return;

    //$REVISIT: do we need to modify this to generate values in the range 0..255 (instead of word-sized values; diff b/w Xbox and DX8)?
// set gamma table

    step = (float)brightness * (float)contrast / 256.0f;
    middle = (float)brightness * 128.0f;

    for (i = 0 ; i < 256 ; i++)
    {
        n = ((float)i - 128.0f) * step + middle;

        if (n < 0) n = 0;
        else if (n > 65535.0f) n = 65535.0f;

        ramp.red[i] = (WORD)n;
        ramp.green[i] = (WORD)n;
        ramp.blue[i] = (WORD)n;
    }

    if (GammaFlag == GAMMA_AUTO)
        GammaControl->SetGammaRamp(DDSGR_CALIBRATE, &ramp);
    else
        GammaControl->SetGammaRamp(0, &ramp);
#endif // ! XBOX_NOT_YET_IMPLEMENTED
}

/* $REMOVED
////////////////////////////////
// find a good zbuffer format //
////////////////////////////////

HRESULT CALLBACK EnumZedBufferCallback(DDPIXELFORMAT *ddpf, void *user)
{

// skip if null format

    if (!ddpf)
        return DDENUMRET_CANCEL;

// skip if not a zbuffer!

    if (ddpf->dwFlags != DDPF_ZBUFFER)
        return DDENUMRET_OK;

// take if we haven't got one

    if (!ZedBufferFormat.dwZBufferBitDepth)
    {
        ZedBufferFormat = *ddpf;
        return DDENUMRET_OK;
    }

// take if better than currently got

    if (ddpf->dwZBufferBitDepth == 16)
    {
        ZedBufferFormat = *ddpf;
        return DDENUMRET_CANCEL;
    }

// next please

    return DDENUMRET_OK;
}
$END_REMOVAL */

/* $REMOVED (We should never lose surfaces on Xbox!)
/////////////////////////////////
// check surfaces are not lost //
/////////////////////////////////

void CheckSurfaces(void)
{
    char i;
    HRESULT r;
    TEXINFO texinfo;

// do we need to?

    if (!AppRestore)
        return;

    AppRestore = FALSE;

// check FrontBuffer

    r = FrontBuffer->IsLost();
    if (r == DDERR_SURFACELOST)
    {
        r = FrontBuffer->Restore();
        if (r != DD_OK)
        {
            ErrorDX(r, "Can't restore primary display");
            g_bQuitGame = TRUE;
            return;
        }
    }

// check ZedBuffer

    r = ZedBuffer->IsLost();
    if (r == DDERR_SURFACELOST)
    {
        r = ZedBuffer->Restore();
        if (r != DD_OK)
        {
            ErrorDX(r, "Can't restore zed buffer");
            g_bQuitGame = TRUE;
            return;
        }
    }

// check textures

    for (i = 0 ; i < TPAGE_NUM ; i++) if (TexInfo[i].Active)
    {
        r = TexInfo[i].Surface->IsLost();
        if (r == DDERR_SURFACELOST)
        {
            texinfo = TexInfo[i];
            FreeOneTexture(i);

            if (texinfo.MipCount == -1)
            {
                CreateProceduralTexture(i, texinfo.Width, texinfo.Height);
            }
            else
            {
                LoadMipTexture(texinfo.File, i, texinfo.Width, texinfo.Height, texinfo.Stage, texinfo.MipCount, FALSE);
            }
        }
    }

// flip to GDI surface

    DD->FlipToGDISurface();
}
$END_REMOVAL */

//////////////////
// flip buffers //
//////////////////

void FlipBuffers(void)
{
//$MODIFIED
//    RECT dest;
//    long bx, by, cy;
//    HRESULT r;
//
//// full screen
//
//    if (FullScreen)
//    {
//        r = FrontBuffer->Flip(NULL, ((RegistrySettings.Vsync && !GAME_GAUGE) ? 0 : DDFLIP_NOVSYNC) | DDFLIP_WAIT);
//        if (r == DDERR_SURFACELOST)
//        {
//            r = FrontBuffer->Restore();
//            if (r != DD_OK)
//            {
//                ErrorDX(r, "Can't restore front buffer");
//            }
//        }
//        else if (r != DD_OK)
//        {
//            ErrorDX(r, "Can't flip display buffers");
//        }
//    }
//
//// windowed
//
//    else
//    {
//        GetWindowRect(hwnd, &dest);
//        bx = GetSystemMetrics(SM_CXSIZEFRAME);
//        by = GetSystemMetrics(SM_CYSIZEFRAME);
//        cy = GetSystemMetrics(SM_CYCAPTION);
//
//        dest.left += bx;
//        dest.right -= bx;
//        dest.top += cy + by;
//        dest.bottom -= by;
//
//        FrontBuffer->Blt(&dest, BackBuffer, NULL, DDBLT_WAIT, NULL);
//    }
    D3Ddevice->Present( NULL, NULL, XBOX_UNUSED_PARAM, XBOX_UNUSED_PARAM );
//$END_MODIFICATIONS
}

/////////////////////////////
// clear zed + back buffer //
/////////////////////////////

void ClearBuffers(void)
{
    if (!PolyClear)
    {
        D3Ddevice->Clear( 1, // num rectangles
                          &ViewportRect, // rectangle array
                          D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, // flags
                          BackgroundColor,  // color value
                          1.0f, // z value
                          0 // stencil value
                        );
    }
    else
    {
        DumpMessage( "ERROR", "PolyClear not supported!  Do you really need it?" );
    }
}



//////////////////////////
// set background color //
//////////////////////////

void SetBackgroundColor(long col)
{
    long l;

    if (RenderSettings.Sepia)
    {
        l = SepiaRGB[(col & 0xf80000) >> 9 | (col & 0x00f800) >> 6 | (col & 0x0000f8) >> 3];
        BackgroundColor = (l & 0x7c00) << 9 | (l & 0x3e0) << 6 | (l & 0x1f) << 3;
    }
    else
    {
        BackgroundColor = col;
    }
}

////////////////
// Release dx //
////////////////

void ReleaseDX(void)
{
    RELEASE(D3D);
//$REMOVED    RELEASE(DD);
}

/////////////////
// Release d3d //
/////////////////

void ReleaseD3D(void)
{
    RELEASE(D3Ddevice);
}

/////////////////////
// Report DX error //
/////////////////////

void ErrorDX(HRESULT r, char *mess)
{
    char szTemp[512];
    sprintf( szTemp, "ErrorNameGoesHere (HR=0x%08X)", r ); 
    DumpMessage( szTemp, mess );
}

///////////////////
// setup DxState //
///////////////////

void SetupDxState(void)
{
    long i;

// fill mode

    DxState.WireframeEnabled = TRUE;
    DxState.Wireframe = D3DFILL_SOLID;

    WIREFRAME_ON();

	// texture filtering
    DxState.TextureFilterFlag = 1;

    DxState.TextureFilterFlag |= 2;
    DxState.TextureFilterFlag |= 4;

    for (i = 0 ; i < 3 ; i++) if (DxState.TextureFilterFlag & (1 << i)) DxState.TextureFilter = i;  //$CMP_NOTE: their code here is ugly; using explicit names would feel safer...

    TEXTUREFILTER_ON();

// mip map

    DxState.MipMapFlag = 1;

    DxState.MipMapFlag |= 2; //$NOTE: for D3DTEXF_POINT mipfilter on Xbox
    DxState.MipMapFlag |= 4; //$NOTE: for D3DTEXF_LINEAR mipfilter on Xbox

    for (i = 0 ; i < 3 ; i++) if (DxState.MipMapFlag & (1 << i)) DxState.MipMap = i;  //$CMP_NOTE: their code here is ugly; using explicit names would feel safer...

    MIPMAP_ON();

// fog
    {
        DxState.FogEnabled = TRUE;
        DxState.Fog = TRUE;
    }

    FOG_OFF();

// dither
    {
        DxState.DitherEnabled = TRUE;
        DxState.Dither = TRUE;
    }

    DITHER_ON();

// color keying
    if (!NoColorKey)
    {
        DxState.ColorKeyEnabled = TRUE;
        DxState.ColorKey = D3DTCOLORKEYOP_RGBA;  //$CMP_NOTE: should this be D3DTCOLORKEYOP_RGBA, D3DTCOLORKEYOP_ALPHA, or maybe D3DTCOLORKEYOP_KILL ???
    }
    else
    {
        DxState.ColorKeyEnabled = FALSE;
        DxState.ColorKey = FALSE;
    }
    DxState.ColorKeyColor = 0xFF000000;  //$ADDITION(jedl): 'ColorKeyColor' var ; set to opaque black

    COLORKEY_ON();

#ifndef XBOX_NOT_YET_IMPLEMENTED
// anti alias

    if (D3Dcaps.dwFlags & D3DDD_TRICAPS && D3Dcaps.dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_ANTIALIASSORTINDEPENDENT)
    {
        DxState.AntiAliasEnabled = TRUE;
        DxState.AntiAlias = D3DANTIALIAS_NONE;
    }
    else
    {
        DxState.AntiAliasEnabled = FALSE;
        DxState.AntiAlias = D3DANTIALIAS_NONE;
    }

    ANTIALIAS_ON();
#endif // ! XBOX_NOT_YET_IMPLEMENTED
}





