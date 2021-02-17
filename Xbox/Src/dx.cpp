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
//$REMOVEDIDirect3D3 *D3D = NULL;
IDirect3DDevice3 *D3Ddevice = NULL;
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

int   RenderTP = -1;
int   RenderTP2 = -1;
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

//$REMOVED - this function does nothing on Xbox
//BOOL InitDX(void) //$RENAMED: was InitDD
//{
////$MODIFIED
////    HRESULT r;
////    DDSCAPS2 ddscaps2;
////    DWORD temp;
////
////// release
////
////    ReleaseDX();
////
////// create draw device
////
////    DirectDrawEnumerate(CreateDrawDeviceCallback, NULL);
////    CurrentDrawDevice = RegistrySettings.DrawDevice;
////
////// get device caps
////
////    ZeroMemory(&DDcaps, sizeof(DDcaps));
////    DDcaps.dwSize = sizeof(DDcaps);
////
////    r = DD->GetCaps(&DDcaps, NULL);
////    if (r != DD_OK)
////    {
////        ErrorDX(r, "Can't get DD device caps");
////        return FALSE;
////    }
////
////// disable window if voodoo
////
////    if (!FullScreen && !(DDcaps.dwCaps2 & DDCAPS2_CANRENDERWINDOWED))
////    {
////        DumpMessage(NULL,"Cannot run in windowed mode.  Switching to fullscreen.");
////        FullScreen = TRUE;
////    }
////
////// get total screen / texture mem
////
////    ddscaps2.dwCaps = DDSCAPS_PRIMARYSURFACE;
////    DD->GetAvailableVidMem(&ddscaps2, &TotalScreenMem, &temp);
////
////    ddscaps2.dwCaps = DDSCAPS_TEXTURE;
////    DD->GetAvailableVidMem(&ddscaps2, &TotalTexMem, &temp);
////
////// set exclusive mode
////
////    if (FullScreen)
////    {
////        r = DD->SetCooperativeLevel(hwnd, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN | DDSCL_ALLOWMODEX | DDSCL_ALLOWREBOOT | DDSCL_FPUSETUP);
////        if (r != DD_OK)
////        {
////            ErrorDX(r, "Can't set coop level");
////            return FALSE;
////        }
////    }
////    else
////    {
////        r = DD->SetCooperativeLevel(hwnd, DDSCL_NORMAL | DDSCL_ALLOWREBOOT | DDSCL_FPUSETUP);
////        if (r != DD_OK)
////        {
////            ErrorDX(r, "Can't set coop level");
////            return FALSE;
////        }
////    }
////
////// get 3D interface
////
////    r = DD->QueryInterface(IID_IDirect3D3, (void**)&D3D);
////    if (r != DD_OK)
////    {
////        ErrorDX(r, "Can't get 3D interface");
////        return FALSE;
////    }
//
////$END_MODIFICATIONS
//
////$MODIFIED - We don't need the D3D object on Xbox    
////    D3D = Direct3DCreate8(D3D_SDK_VERSION);
////    if( NULL == D3D )
////    {
////        ErrorDX( 0, "Unable to create Direct3D!\n" );
////        return FALSE;
////    }
/////$END_MODIFICATIONS
//
//// return OK
//
//    return TRUE;
//}

///////////////////
// Init D3D misc //
///////////////////

BOOL InitD3D(DWORD width, DWORD height, DWORD bpp, DWORD refresh)
{
//$REMOVED    long i, start, end, time1, time2;
    HRESULT r;
//$REMOVED    DDSURFACEDESC2 ddsd2;
//$REMOVED    DDSCAPS2 ddscaps2;
    D3DVIEWPORT2 vd;
//$REMOVED    D3DDEVICEDESC hal, hel;
//$REMOVED    IDirectDrawClipper *clipper;
//$REMOVED    char buf[128];

// release

    ReleaseD3D();

// set screen params

    ScreenXsize = width;
    ScreenYsize = height;
    ScreenBpp = bpp;
//$REMOVED    ScreenRefresh = refresh;

// enumerate z buffer formats
//$MODIFIED
//    ZeroMemory(&ZedBufferFormat, sizeof(ZedBufferFormat));
//
//    if (Software == 1) D3D->EnumZBufferFormats(IID_IDirect3DRGBDevice, EnumZedBufferCallback, NULL);
//    else if (Software == 2) D3D->EnumZBufferFormats(IID_IDirect3DMMXDevice, EnumZedBufferCallback, NULL);
//    else D3D->EnumZBufferFormats(IID_IDirect3DHALDevice, EnumZedBufferCallback, NULL);
//
//    if (!ZedBufferFormat.dwZBufferBitDepth)
//    {
//        DumpMessage(NULL, "No Zbuffer available!");
//        return FALSE;
//    }
    ZedBufferFormat = D3DFMT_D24S8;  //$REVISIT: their enum function searched for 16-bit z buffer, but I assume we want a 32-bit depth buffer.
//$END_MODIFICATIONS

//$REMOVED (on xbox, we set video mode by specifying Width, Height, Progressive, Widescreen in present-parameters)
//// set screen mode
//
//    if (FullScreen)
//    {
//        r = DD->SetDisplayMode(ScreenXsize, ScreenYsize, ScreenBpp, ScreenRefresh, 0);
//        if (r != DD_OK)
//        {
//            sprintf(buf, "Can't set display mode %dx%dx%d", ScreenXsize, ScreenYsize, ScreenBpp);
//            ErrorDX(r, buf);
//            return FALSE;
//        }
//    }
//$END_REMOVAL

//$REMOVED
//// create Z buffer first
//
//    ZeroMemory(&ddsd2, sizeof(ddsd2));
//    ddsd2.dwSize = sizeof(ddsd2);
//    ddsd2.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS | DDSD_PIXELFORMAT;
//    ddsd2.dwWidth = ScreenXsize;
//    ddsd2.dwHeight = ScreenYsize;
//    if (!Software) ddsd2.ddsCaps.dwCaps = DDSCAPS_ZBUFFER | DDSCAPS_VIDEOMEMORY;
//    else ddsd2.ddsCaps.dwCaps = DDSCAPS_ZBUFFER | DDSCAPS_SYSTEMMEMORY;
//    ddsd2.ddsCaps.dwCaps2 = 0;
//    ddsd2.ddpfPixelFormat = ZedBufferFormat;
//
//    r = DD->CreateSurface(&ddsd2, &ZedBuffer, NULL);
//    if (r != DD_OK)
//    {
//        if (ScreenXsize != 640 || ScreenYsize != 480 || ScreenBpp != 16)
//        {
//            RegistrySettings.ScreenWidth = 640;
//            RegistrySettings.ScreenHeight = 480;
//            RegistrySettings.ScreenBpp = 16;
//            return InitD3D(640, 480, 16, 0);
//        }
//        else
//        {
//            ErrorDX(r, "Can't create Z buffer");
//            return FALSE;
//        }
//    }
//$END_REMOVAL

//$MODIFIED
//// create front buffer with 1 or 2 back buffers - fullscreen
//
//    if (FullScreen)
//    {
//        ZeroMemory(&ddsd2, sizeof(ddsd2));
//        ddsd2.dwSize = sizeof(ddsd2);
//        ddsd2.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
//        if (!Software) ddsd2.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX | DDSCAPS_3DDEVICE | DDSCAPS_VIDEOMEMORY;
//        else ddsd2.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX | DDSCAPS_3DDEVICE | DDSCAPS_SYSTEMMEMORY;
//        ddsd2.ddsCaps.dwCaps2 = 0;
//        BackBufferCount = ddsd2.dwBackBufferCount = ForceDoubleBuffer ? 1 : 2;
//        r = DD->CreateSurface(&ddsd2, &FrontBuffer, NULL);
//        if (r != DD_OK)
//        {
//            BackBufferCount = ddsd2.dwBackBufferCount = 1;
//            r = DD->CreateSurface(&ddsd2, &FrontBuffer, NULL);
//            if (r != DD_OK)
//            {
//                if (ScreenXsize != 640 || ScreenYsize != 480 || ScreenBpp != 16)
//                {
//                    RegistrySettings.ScreenWidth = 640;
//                    RegistrySettings.ScreenHeight = 480;
//                    RegistrySettings.ScreenBpp = 16;
//                    return InitD3D(640, 480, 16, 0);
//                }
//                else
//                {
//                    ErrorDX(r, "Can't create draw surfaces!");
//                    return FALSE;
//                }
//            }
//        }
//
//        ddscaps2.dwCaps = DDSCAPS_BACKBUFFER;
//        r = FrontBuffer->GetAttachedSurface(&ddscaps2, &BackBuffer);
//        if (r != DD_OK)
//        {
//            ErrorDX(r, "Can't attach back buffer!");
//            return FALSE;
//        }
//    }
//
//// create front buffer with 1 back buffer - window
//
//    else
//    {
//        ZeroMemory(&ddsd2, sizeof(ddsd2));
//        ddsd2.dwSize = sizeof(ddsd2);
//        ddsd2.dwFlags = DDSD_CAPS;
//        if (!Software) ddsd2.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_VIDEOMEMORY;
//        else ddsd2.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_SYSTEMMEMORY;
//        ddsd2.ddsCaps.dwCaps2 = 0;
//
//        r = DD->CreateSurface(&ddsd2, &FrontBuffer, NULL);
//        if (r != DD_OK)
//        {
//            ErrorDX(r, "Can't create primary surface!");
//            return FALSE;
//        }
//
//        ddsd2.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
//        if (!Software) ddsd2.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_3DDEVICE | DDSCAPS_VIDEOMEMORY;
//        else ddsd2.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_3DDEVICE | DDSCAPS_SYSTEMMEMORY;
//        ddsd2.dwWidth = ScreenXsize;
//        ddsd2.dwHeight = ScreenYsize;
//
//        BackBufferCount = 1;
//        r = DD->CreateSurface(&ddsd2, &BackBuffer, NULL);
//        if (r != DD_OK)
//        {
//            ErrorDX(r, "Can't create back buffer!");
//            return FALSE;
//        }
//
//        r = DD->CreateClipper(0, &clipper, NULL);
//        if (r != DD_OK)
//        {
//            ErrorDX(r, "Can't create clipper!");
//            return FALSE;
//        }
//
//        clipper->SetHWnd(0, hwnd);
//        FrontBuffer->SetClipper(clipper);
//        RELEASE(clipper);
//    }

    BackBufferCount = ForceDoubleBuffer ? 1 : 2;
//$END_MODIFICATIONS

//$REMOVED
//// attach Z buffer
//
//    r = BackBuffer->AddAttachedSurface(ZedBuffer);
//    if (r != DD_OK)
//    {
//        ErrorDX(r, "Can't attach Z buffer");
//        return FALSE;
//    }
//$END_REMOVAL

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
    d3dpp.FullScreen_PresentationInterval = (RegistrySettings.Vsync ? D3DPRESENT_INTERVAL_ONE : D3DPRESENT_INTERVAL_IMMEDIATE); //$REVISIT: we probably don't want to allow IMMEDIATE when we release.
//$END_ADDITION

    // $BEGIN_ADDITION - jedl
    // $REVISIT - we need antialias turned on, but what kind?
    // Multisampling, supersampling, edge-antialiasing only, scaled back buffer?
    // It would be nice to compare all of these visually and performance-wise
    //
    // Turn on antialiasing
    // d3dpp.MultiSampleType        = D3DMULTISAMPLE_4_SAMPLES_SUPERSAMPLE_GAUSSIAN;
    // $END_ADDITION

// create D3D device
//$MODIFIED
//
//    if (Software == 1)
//        r = D3D->CreateDevice(IID_IDirect3DRGBDevice, BackBuffer, &D3Ddevice, NULL);
//    else if (Software == 2)
//        r = D3D->CreateDevice(IID_IDirect3DMMXDevice, BackBuffer, &D3Ddevice, NULL);
//    else
//        r = D3D->CreateDevice(IID_IDirect3DHALDevice, BackBuffer, &D3Ddevice, NULL);
//
//    if (r != DD_OK)
//    {
//        if (ScreenXsize != 640 || ScreenYsize != 480 || ScreenBpp != 16)
//        {
//            RegistrySettings.ScreenWidth = 640;
//            RegistrySettings.ScreenHeight = 480;
//            RegistrySettings.ScreenBpp = 16;
//            return InitD3D(640, 480, 16, 0);
//        }
//        else
//        {
//            ErrorDX(r, "Can't create a 3D device!");
//            return FALSE;
//        }
//    }

    r = Direct3D_CreateDevice( D3DADAPTER_DEFAULT,
                               D3DDEVTYPE_HAL,
                               XBOX_UNUSED_PARAM,
                               D3DCREATE_HARDWARE_VERTEXPROCESSING,
                               &d3dpp,
                               &D3Ddevice );
    if( r != D3D_OK )
    {
        ErrorDX(r, "Can't create a 3D device!");
        return FALSE;
    }

//$END_MODIFICATIONS

    // $BEGIN_ADDITION
    //  TODO: jedl - revisit the backbuffer scaling
    // Antialias test
    // static FLOAT BackBufferScale = 0.5f;
    // D3Ddevice->SetBackBufferScale(BackBufferScale, BackBufferScale);
    // $END_ADDITION

//$REMOVED
//// get D3D device caps
//
//    ZeroMemory(&hal, sizeof(hal));
//    hal.dwSize = sizeof(hal);
//
//    ZeroMemory(&hel, sizeof(hel));
//    hel.dwSize = sizeof(hel);
//
//    r = D3Ddevice->GetCaps(&hal, &hel);
//
//    if (r != DD_OK)
//    {
//        ErrorDX(r, "Can't get D3D device caps");
//        return FALSE;
//    }
//
//    if (!Software) D3Dcaps = hal;
//    else D3Dcaps = hel;
//$END_REMOVAL

// create / setup viewport
//$REMOVED
//    r = D3D->CreateViewport(&D3Dviewport, NULL);
//    if (r != DD_OK)
//    {
//        ErrorDX(r, "Can't create a viewport");
//        return FALSE;
//    }
//
//    r = D3Ddevice->AddViewport(D3Dviewport);
//    if (r != DD_OK)
//    {
//        ErrorDX(r, "Can't attach viewport to 3D device");
//        return FALSE;
//    }
//$END_REMOVAL

//$MODIFIED
//    ZeroMemory(&vd, sizeof(vd));
//    vd.dwSize = sizeof(vd);
//    vd.dwX = 0;
//    vd.dwY = 0;
//    vd.dwWidth = ScreenXsize;
//    vd.dwHeight = ScreenYsize;
//
//    vd.dvClipX = 0;
//    vd.dvClipY = 0;
//    vd.dvClipWidth = (float)ScreenXsize;
//    vd.dvClipHeight = (float)ScreenYsize;
//    vd.dvMinZ = 0;
//    vd.dvMaxZ = 1;
    vd.X      = 0;
    vd.Y      = 0;
    vd.Width  = ScreenXsize;
    vd.Height = ScreenYsize;
    vd.MinZ   = 0.0f;
    vd.MaxZ   = 1.0f;
//$END_MODIFICATION

//$MODIFIED
//    r = D3Dviewport->SetViewport2(&vd);
//    if (r != DD_OK)
//    {
//        ErrorDX(r, "Can't set viewport");
//        return FALSE;
//    }
//
//    r = D3Ddevice->SetCurrentViewport(D3Dviewport);
//    if (r != DD_OK)
//    {
//        ErrorDX(r, "Can't set current viewport");
//        return FALSE;
//    }
    D3DDevice_SetViewport( &vd );
//$END_MODIFICATIONS

// get gamma control interface

//$MODIFIED
//    if (NoGamma || !(DDcaps.dwCaps2 & DDCAPS2_PRIMARYGAMMA))
//    {
//        GammaFlag = GAMMA_UNAVAILABLE;
//    }
//    else
//    {
//        r = FrontBuffer->QueryInterface(IID_IDirectDrawGammaControl, (void**)&GammaControl);
//        if (r != DD_OK)
//        {
//            ErrorDX(r, "Can't get gamma interface");
//            return FALSE;
//        }
//
////      if (DDcaps.dwCaps2 & DDCAPS2_CANCALIBRATEGAMMA)
////          GammaFlag = GAMMA_AUTO;
////      else
////          GammaFlag = GAMMA_AVAILABLE;
//        GammaFlag = GAMMA_AVAILABLE;
//
//        SetGamma(RegistrySettings.Brightness, RegistrySettings.Contrast);
//    }

//$MODIFIED - This was moved to use default values until the registry settings
//            are loaded
    if(NoGamma)
    {
        GammaFlag = GAMMA_UNAVAILABLE;
    }
    else
    {
        GammaFlag = GAMMA_AVAILABLE;
        SetGamma(256, 256);
    }

//$END_MODIFICATIONS

// decide screen / zbuffer clear technique
//$REMOVED
//    PolyClear = FALSE;
//
//    BackgroundColor = 0x000000;
//    ViewportRect.x1 = 0;
//    ViewportRect.y1 = 0;
//    ViewportRect.x2 = ScreenXsize;
//    ViewportRect.y2 = ScreenYsize;
//
//    start = CurrentTimer();
//
//    for (i = 0 ; i < 50 ; i++)
//    {
//        FlipBuffers();
//        D3Ddevice->BeginScene();
//        ClearBuffers();
//        D3Ddevice->EndScene();
//    }
//
//    end = CurrentTimer();
//    time1 = end - start;
//
//    PolyClear = TRUE;
//    start = CurrentTimer();
//
//    for (i = 0 ; i < 50 ; i++)
//    {
//        FlipBuffers();
//        D3Ddevice->BeginScene();
//        ClearBuffers();
//        D3Ddevice->EndScene();
//    }
//
//    end = CurrentTimer();
//    time2 = end - start;
//
//    if (!Software)
//        PolyClear = (time2 < time1);
//    else
//$END_REMOVAL
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

//$MODIFIED
//    BlendSubtract = (D3Dcaps.dwFlags & D3DDD_TRICAPS &&
//        D3Dcaps.dpcTriCaps.dwSrcBlendCaps & D3DPBLENDCAPS_ZERO &&
//        D3Dcaps.dpcTriCaps.dwDestBlendCaps & D3DPBLENDCAPS_INVSRCCOLOR);
    BlendSubtract = true;  //$NOTE: set true b/c Xbox supports D3DBLEND_ZERO and D3DBLEND_INVSRCCOLOR
//$END_MODIFICATIONS

//$REMOVED
//// flip to GDI surface to fix gay detonator drivers!
//
//    DD->FlipToGDISurface();
//$END_REMOVAL

// return ok

    return TRUE;
}

//////////////////////
// set gamma values //
//////////////////////

//$NOTE(cprince): arguments here have values in the range 32..512 (inclusive)
//$REVISIT(cprince): was Acclaim's function correct?
void SetGamma(long brightness, long contrast)
{
    long i;
    float step, middle, n;
    D3DGAMMARAMP ramp;  //$MODIFIED - was DDGAMMARAMP

// skip if no gamma control

    if (GammaFlag == GAMMA_UNAVAILABLE)
        return;

// set gamma table

    step = (float)brightness * (float)contrast / 256.0f;
    middle = (float)brightness * 128.0f;

    for (i = 0 ; i < 256 ; i++)
    {
        n = ((float)i - 128.0f) * step + middle;

//$MODIFIED - Xbox D3D wants values in the range 0..255, instead of word-sized values (differs from DX8!)
//        if (n < 0) n = 0;
//        else if (n > 65535.0f) n = 65535.0f;
//
//        ramp.red[i]   = (WORD)n;
//        ramp.green[i] = (WORD)n;
//        ramp.blue[i]  = (WORD)n;

        int nn = (int)(n / 256.0f);  // scale from word-sized to byte-sized values (and change float -> integral)
        if( nn < 0 )   nn = 0;
        if( nn > 255 ) nn = 255;

        ramp.red[i]   = (BYTE)nn;
        ramp.green[i] = (BYTE)nn;
        ramp.blue[i]  = (BYTE)nn;
//$END_MODIFICATIONS
    }

//$MODIFIED
//    if (GammaFlag == GAMMA_AUTO)
//        GammaControl->SetGammaRamp(DDSGR_CALIBRATE, &ramp);
//    else
//        GammaControl->SetGammaRamp(0, &ramp);

    // On Xbox, D3DSGR_CALIBRATE is not available.
    D3DDevice_SetGammaRamp( 0, &ramp );
//$END_MODIFICATIONS
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
            QuitGame();
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
            QuitGame();
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
//$REMOVED    float zres, z;

    if (!PolyClear)
    {
//$MODIFIED
//        D3Dviewport->Clear2(1, &ViewportRect, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, BackgroundColor, 1.0f, 0);
        D3Ddevice->Clear( 1, // num rectangles
                          &ViewportRect, // rectangle array
                          D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, // flags
                          BackgroundColor,  // color value
                          1.0f, // z value
                          0 // stencil value
                        );
        //$REVISIT: do we really need the rectangles here?  Or can we just clear the entire current viewport?  Is rectangle ever anything else?
//$END_MODIFICATIONS
    }
    else
    {
//$MODIFIED
//        zres = (float)(1 << ZedBufferFormat.dwZBufferBitDepth);
//        z = (zres - 1.0f) / zres;
//
//        DrawVertsTEX0[0].sx = (float)ViewportRect.x1;
//        DrawVertsTEX0[0].sy = (float)ViewportRect.y1;
//        DrawVertsTEX0[0].sz = z;
//        DrawVertsTEX0[0].rhw = 1.0f;
//        DrawVertsTEX0[0].color = BackgroundColor;
//
//        DrawVertsTEX0[1].sx = (float)ViewportRect.x2;
//        DrawVertsTEX0[1].sy = (float)ViewportRect.y1;
//        DrawVertsTEX0[1].sz = z;
//        DrawVertsTEX0[1].rhw = 1.0f;
//        DrawVertsTEX0[1].color = BackgroundColor;
//
//        DrawVertsTEX0[2].sx = (float)ViewportRect.x2;
//        DrawVertsTEX0[2].sy = (float)ViewportRect.y2;
//        DrawVertsTEX0[2].sz = z;
//        DrawVertsTEX0[2].rhw = 1.0f;
//        DrawVertsTEX0[2].color = BackgroundColor;
//
//        DrawVertsTEX0[3].sx = (float)ViewportRect.x1;
//        DrawVertsTEX0[3].sy = (float)ViewportRect.y2;
//        DrawVertsTEX0[3].sz = z;
//        DrawVertsTEX0[3].rhw = 1.0f;
//        DrawVertsTEX0[3].color = BackgroundColor;
//
//        SET_TPAGE(-1);
//
//        ZCMP(D3DCMP_ALWAYS);
//        SET_RENDER_STATE(D3DRENDERSTATE_ALPHATESTENABLE, FALSE);
//        DRAW_PRIM(D3DPT_TRIANGLEFAN, FVF_TEX0, DrawVertsTEX0, 4, D3DDP_DONOTUPDATEEXTENTS | D3DDP_DONOTCLIP);
//        SET_RENDER_STATE(D3DRENDERSTATE_ALPHATESTENABLE, TRUE);
//        ZCMP(D3DCMP_LESSEQUAL);

        DumpMessage( "ERROR", "PolyClear not supported!  Do you really need it?" );
        //$NOTE(cprince): probably okay not to support poly-clear

//$END_MODIFICATIONS
    }
}

/* $REMOVED
//////////////////////////
// set front buffer rgb //
//////////////////////////

void SetFrontBufferRGB(long rgb)
{
    DDBLTFX bltfx;

    bltfx.dwSize = sizeof(bltfx);
    bltfx.dwFillColor = rgb;
    FrontBuffer->Blt(NULL, NULL, NULL, DDBLT_COLORFILL, &bltfx);
}
$END_REMOVAL */

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

//$REMOVED - This function does nothing on Xbox
//void ReleaseDX(void)
//{
////$REMOVED    RELEASE(D3D);
////$REMOVED    RELEASE(DD);
//}

/////////////////
// Release d3d //
/////////////////

void ReleaseD3D(void)
{
    RELEASE(D3Ddevice);
//$REMOVED    RELEASE(GammaControl);
//$REMOVED    RELEASE(D3Dviewport);
//$REMOVED    RELEASE(ZedBuffer);
//$REMOVED    RELEASE(BackBuffer);
//$REMOVED    RELEASE(FrontBuffer);
}

/////////////////////
// Report DX error //
/////////////////////

void ErrorDX(HRESULT r, char *mess)
{
//$MODIFIED
//    ERRORDX *p = ErrorListDX;
//
//    while (p->Result != DD_OK && p->Result != r) p++;
//    Box(p->Error, mess, MB_OK);
    char szTemp[512];
    sprintf( szTemp, "ErrorNameGoesHere (HR=0x%08X)", r ); 
    DumpMessage( szTemp, mess );
//$END_MODIFICATIONS
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

//$REMOVED (perspective-correction for textures is on by default in DX8)
//// perspective
//
//    if (D3Dcaps.dwFlags & D3DDD_TRICAPS && D3Dcaps.dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_PERSPECTIVE)
//    {
//        DxState.PerspectiveEnabled = TRUE;
//        DxState.Perspective = TRUE;
//    }
//    else
//    {
//        DxState.PerspectiveEnabled = FALSE;
//        DxState.Perspective = FALSE;
//    }
//
//    PERSPECTIVE_ON();
//$END_REMOVAL

// texture filtering

    DxState.TextureFilterFlag = 1;

//$MODIFIED
//    if (D3Dcaps.dwFlags & D3DDD_TRICAPS && D3Dcaps.dpcTriCaps.dwTextureFilterCaps & D3DPTFILTERCAPS_LINEAR) DxState.TextureFilterFlag |= 2;
//    if (D3Dcaps.dwFlags & D3DDD_TRICAPS && D3Dcaps.dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_ANISOTROPY) DxState.TextureFilterFlag |= 4;
    DxState.TextureFilterFlag |= 2;
    DxState.TextureFilterFlag |= 4;
//$END_MODIFICATIONS

    for (i = 0 ; i < 3 ; i++) if (DxState.TextureFilterFlag & (1 << i)) DxState.TextureFilter = i;  //$CMP_NOTE: their code here is ugly; using explicit names would feel safer...

    TEXTUREFILTER_ON();

// mip map

    DxState.MipMapFlag = 1;

//$MODIFIED
//    if (D3Dcaps.dwFlags & D3DDD_TRICAPS && D3Dcaps.dpcTriCaps.dwTextureFilterCaps & (D3DPTFILTERCAPS_MIPNEAREST | D3DPTFILTERCAPS_MIPLINEAR)) DxState.MipMapFlag |= 2;
//    if (D3Dcaps.dwFlags & D3DDD_TRICAPS && D3Dcaps.dpcTriCaps.dwTextureFilterCaps & (D3DPTFILTERCAPS_LINEARMIPNEAREST | D3DPTFILTERCAPS_LINEARMIPLINEAR)) DxState.MipMapFlag |= 4;
    DxState.MipMapFlag |= 2; //$NOTE: for D3DTEXF_POINT mipfilter on Xbox
    DxState.MipMapFlag |= 4; //$NOTE: for D3DTEXF_LINEAR mipfilter on Xbox
    //$NOTE: Xbox doesn't support higher mipfilter values (eg, anisotropic not supported)
//$END_MODIFICATIONS

    for (i = 0 ; i < 3 ; i++) if (DxState.MipMapFlag & (1 << i)) DxState.MipMap = i;  //$CMP_NOTE: their code here is ugly; using explicit names would feel safer...

    MIPMAP_ON();

// fog

//$REMOVED    if (D3Dcaps.dwFlags & D3DDD_TRICAPS && D3Dcaps.dpcTriCaps.dwShadeCaps & D3DPSHADECAPS_FOGGOURAUD)
    {
        DxState.FogEnabled = TRUE;
        DxState.Fog = TRUE;
    }
//$REMOVED    else
//$REMOVED    {
//$REMOVED        DxState.FogEnabled = FALSE;
//$REMOVED        DxState.Fog = FALSE;
//$REMOVED    }

    FOG_OFF();

// dither

//$REMOVED    if (D3Dcaps.dwFlags & D3DDD_TRICAPS && D3Dcaps.dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_DITHER)
    {
        DxState.DitherEnabled = TRUE;
        DxState.Dither = TRUE;
    }
//$REMOVED    else
//$REMOVED    {
//$REMOVED        DxState.DitherEnabled = FALSE;
//$REMOVED        DxState.Dither = FALSE;
//$REMOVED    }

    DITHER_ON();

// color keying

//$MODIFIED
//    if (D3Dcaps.dwFlags & D3DDD_TRICAPS && D3Dcaps.dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_TRANSPARENCY && !NoColorKey)
    if (!NoColorKey)
//$END_MODIFICATIONS
    {
        DxState.ColorKeyEnabled = TRUE;
//$MODIFIED:
//        DxState.ColorKey = TRUE;
        DxState.ColorKey = D3DTCOLORKEYOP_RGBA;  //$CMP_NOTE: should this be D3DTCOLORKEYOP_RGBA, D3DTCOLORKEYOP_ALPHA, or maybe D3DTCOLORKEYOP_KILL ???
//$END_MODIFICATIONS
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

/* $REMOVED
//////////////////////////
// get all draw devices //
//////////////////////////

void GetDrawDevices(void)
{

// enumerate devices

    DrawDeviceNum = 0;
    DirectDrawEnumerate(GetDrawDeviceCallback, NULL);

// set request device to registry setting

    if (RegistrySettings.DrawDevice >= (DWORD)DrawDeviceNum)
        RegistrySettings.DrawDevice = 0;

    DisplayModeCount = DrawDevices[RegistrySettings.DrawDevice].BestDisplayMode;
}
$END_REMOVAL */

/* $REMOVED
//////////////////////////////
// get draw device callback //
//////////////////////////////

BOOL CALLBACK GetDrawDeviceCallback(GUID *lpGUID, LPSTR szName, LPSTR szDevice, LPVOID lParam)
{
    HRESULT r;

// stop if reached max

    if (DrawDeviceNum == MAX_DRAW_DEVICES)
    {
        return DDENUMRET_CANCEL;
    }

// create this device

    r = CoCreateInstance(CLSID_DirectDraw, NULL, CLSCTX_ALL, IID_IDirectDraw4, (void**)&DD);
    if (r != S_OK)
    {
        DumpMessage(NULL, "Can't create draw device");
        return DDENUMRET_CANCEL;
    }

    r = DD->Initialize(lpGUID);
    if (r != DD_OK)
    {
        ErrorDX(r, "Can't init draw device");
        return DDENUMRET_CANCEL;
    }

// get name

    memcpy(DrawDevices[DrawDeviceNum].Name, szName, MAX_DRAW_DEVICE_NAME);

// get display modes

    DrawDevices[DrawDeviceNum].DisplayModeNum = 0;
    DD->EnumDisplayModes(0, NULL, NULL, DisplayModesCallback);

// inc count

    DrawDeviceNum++;

// kill device

    RELEASE(DD);

// next please

    return DDENUMRET_OK;
}
$END_REMOVAL */

/* $REMOVED
/////////////////////////////////
// create draw device callback //
/////////////////////////////////

BOOL CALLBACK CreateDrawDeviceCallback(GUID *lpGUID, LPSTR szName, LPSTR szDevice, LPVOID lParam)
{
    HRESULT r;
    char buf[128];

// skip if wrong name

    if (strcmp(szName, DrawDevices[RegistrySettings.DrawDevice].Name))
        return DDENUMRET_OK;

// create this device

    r = CoCreateInstance(CLSID_DirectDraw, NULL, CLSCTX_ALL, IID_IDirectDraw4, (void**)&DD);
    if (r != S_OK)
    {
        sprintf(buf, "Can't create draw device '%s'", szName);
        DumpMessage(NULL, buf);
        return DDENUMRET_CANCEL;
    }

    r = DD->Initialize(lpGUID);
    if (r != DD_OK)
    {
        sprintf(buf, "Can't init draw device '%s'", szName);
        ErrorDX(r, buf);
        return DDENUMRET_CANCEL;
    }

    return DDENUMRET_CANCEL;
}
$END_REMOVAL */

/* $REMOVED
///////////////////////////
// display mode callback //
///////////////////////////

HRESULT CALLBACK DisplayModesCallback(DDSURFACEDESC2 *Mode, void *UserArg)
{

// list full?

    if (DrawDevices[DrawDeviceNum].DisplayModeNum >= MAX_DISPLAY_MODES)
        return DDENUMRET_CANCEL;

// skip if crap display mode

    if (!(Mode->ddpfPixelFormat.dwFlags & DDPF_RGB) || Mode->ddpfPixelFormat.dwRGBBitCount < 16 || Mode->dwWidth < 400 || Mode->dwHeight < 300)
        return DDENUMRET_OK;

// store mode in current draw device list

    DrawDevices[DrawDeviceNum].DisplayMode[DrawDevices[DrawDeviceNum].DisplayModeNum].Width = Mode->dwWidth;
    DrawDevices[DrawDeviceNum].DisplayMode[DrawDevices[DrawDeviceNum].DisplayModeNum].Height = Mode->dwHeight;
    DrawDevices[DrawDeviceNum].DisplayMode[DrawDevices[DrawDeviceNum].DisplayModeNum].Bpp = Mode->ddpfPixelFormat.dwRGBBitCount;
    DrawDevices[DrawDeviceNum].DisplayMode[DrawDevices[DrawDeviceNum].DisplayModeNum].Refresh = Mode->dwRefreshRate;
    sprintf(DrawDevices[DrawDeviceNum].DisplayMode[DrawDevices[DrawDeviceNum].DisplayModeNum].DisplayText, "%dx%dx%d", (short)Mode->dwWidth, (short)Mode->dwHeight, (short)Mode->ddpfPixelFormat.dwRGBBitCount);

// best display mode?

    if (Mode->dwWidth <= RegistrySettings.ScreenWidth && Mode->dwHeight <= RegistrySettings.ScreenHeight && Mode->ddpfPixelFormat.dwRGBBitCount <= RegistrySettings.ScreenBpp)
        DrawDevices[DrawDeviceNum].BestDisplayMode = DrawDevices[DrawDeviceNum].DisplayModeNum;

// next please

    DrawDevices[DrawDeviceNum].DisplayModeNum++;
    return DDENUMRET_OK;
}
$END_REMOVAL */

