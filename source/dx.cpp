
#include "revolt.h"
#include "dx.h"
#include "dxerrors.h"
#include "main.h"
#include "texture.h"
#include "input.h"
#include "camera.h"
#include "play.h"
#include "registry.h"
#include "timing.h"

// globals

DX_STATE DxState;
IDirectDraw4 *DD = NULL;
IDirectDrawSurface4 *FrontBuffer = NULL;
IDirectDrawSurface4 *BackBuffer = NULL;
IDirectDrawSurface4 *ZedBuffer = NULL;
IDirectDrawGammaControl *GammaControl = NULL;
IDirect3D3 *D3D = NULL;
IDirect3DDevice3 *D3Ddevice = NULL;
IDirect3DViewport3 *D3Dviewport;
DDCAPS DDcaps;
D3DDEVICEDESC D3Dcaps;
DDPIXELFORMAT ZedBufferFormat;
DWORD ScreenXsize;
DWORD ScreenYsize;
DWORD ScreenBpp;
DWORD ScreenRefresh;
long GammaFlag = GAMMA_UNAVAILABLE;
long NoColorKey = FALSE;
long DrawDeviceNum, CurrentDrawDevice;
long DisplayModeCount;
DRAW_DEVICE DrawDevices[MAX_DRAW_DEVICES];
long RenderStateChange, TextureStateChange;

short RenderTP = -1;
short RenderTP2 = -1;
short RenderFog = FALSE;
short RenderAlpha = FALSE;
short RenderAlphaSrc = -1;
short RenderAlphaDest = -1;
short RenderZbuffer = D3DZB_TRUE;
short RenderZwrite = TRUE;
short RenderZcmp = D3DCMP_LESSEQUAL;

static DWORD TotalScreenMem, TotalTexMem;
static DWORD BackgroundColor;
static long PolyClear;

//////////////////
// Init dx misc //
//////////////////

BOOL InitDD(void)
{
	HRESULT r;
	DDSCAPS2 ddscaps2;
	DWORD temp;

// release

	ReleaseDX();

// create draw device

	DirectDrawEnumerate(CreateDrawDeviceCallback, NULL);
	CurrentDrawDevice = RegistrySettings.DrawDevice;

// get device caps

	ZeroMemory(&DDcaps, sizeof(DDcaps));
	DDcaps.dwSize = sizeof(DDcaps);

	r = DD->GetCaps(&DDcaps, NULL);
	if (r != DD_OK)
	{
		ErrorDX(r, "Can't get DD device caps");
		return FALSE;
	}

// get total screen / texture mem

	ddscaps2.dwCaps = DDSCAPS_PRIMARYSURFACE;
	DD->GetAvailableVidMem(&ddscaps2, &TotalScreenMem, &temp);

	ddscaps2.dwCaps = DDSCAPS_TEXTURE;
	DD->GetAvailableVidMem(&ddscaps2, &TotalTexMem, &temp);

// set exclusive mode

	if (FullScreen)
	{
		r = DD->SetCooperativeLevel(hwnd, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN | DDSCL_ALLOWMODEX | DDSCL_ALLOWREBOOT | DDSCL_FPUSETUP);
		if (r != DD_OK)
		{
			ErrorDX(r, "Can't set coop level");
			return FALSE;
		}
	}
	else
	{
		r = DD->SetCooperativeLevel(hwnd, DDSCL_NORMAL | DDSCL_ALLOWREBOOT | DDSCL_FPUSETUP);
		if (r != DD_OK)
		{
			ErrorDX(r, "Can't set coop level");
			return FALSE;
		}
	}

// get 3D interface

    r = DD->QueryInterface(IID_IDirect3D3, (void**)&D3D);
	if (r != DD_OK)
	{
		ErrorDX(r, "DirectX 6 is not installed");
		return FALSE;
	}

// return OK

	return TRUE;
}

///////////////////
// Init D3D misc //
///////////////////

BOOL InitD3D(DWORD width, DWORD height, DWORD bpp, DWORD refresh)
{
	long i, start, end, time1, time2;
	HRESULT r;
	DDSURFACEDESC2 ddsd2;
	DDSCAPS2 ddscaps2;
	D3DVIEWPORT2 vd;
	D3DDEVICEDESC hal, hel;
	IDirectDrawClipper *clipper;
	char buf[128];

// release

	ReleaseD3D();

// set screen params

	ScreenXsize = width;
	ScreenYsize = height;
	ScreenBpp = bpp;
	ScreenRefresh = refresh;

// enumerate z buffer formats

	ZeroMemory(&ZedBufferFormat, sizeof(ZedBufferFormat));
	D3D->EnumZBufferFormats(IID_IDirect3DHALDevice, EnumZedBufferCallback, NULL);
	if (!ZedBufferFormat.dwZBufferBitDepth)
	{
		Box(NULL, "No Zbuffer available!", MB_OK);
		return FALSE;
	}

// set screen mode

	if (FullScreen)
	{
		r = DD->SetDisplayMode(ScreenXsize, ScreenYsize, ScreenBpp, ScreenRefresh, 0);
		if (r != DD_OK)
		{
			wsprintf(buf, "Can't set display mode %dx%dx%d", ScreenXsize, ScreenYsize, ScreenBpp);
			ErrorDX(r, buf);
			return FALSE;
		}
	}

// create Z buffer first

	ZeroMemory(&ddsd2, sizeof(ddsd2));
	ddsd2.dwSize = sizeof(ddsd2);
	ddsd2.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS | DDSD_PIXELFORMAT;
	ddsd2.dwWidth = ScreenXsize;
	ddsd2.dwHeight = ScreenYsize;
	ddsd2.ddsCaps.dwCaps = DDSCAPS_ZBUFFER | DDSCAPS_VIDEOMEMORY;
	ddsd2.ddsCaps.dwCaps2 = 0;
	ddsd2.ddpfPixelFormat = ZedBufferFormat;

	r = DD->CreateSurface(&ddsd2, &ZedBuffer, NULL);
	if (r != DD_OK)
	{
		ErrorDX(r, "Can't create Z buffer");
		return FALSE;
	}

// create front buffer with 1 or 2 back buffers

	if (FullScreen)
	{
		ZeroMemory(&ddsd2, sizeof(ddsd2));
		ddsd2.dwSize = sizeof(ddsd2);
		ddsd2.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
		ddsd2.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX | DDSCAPS_3DDEVICE | DDSCAPS_VIDEOMEMORY;
		ddsd2.ddsCaps.dwCaps2 = 0;
 
		ddsd2.dwBackBufferCount = 2;
		r = DD->CreateSurface(&ddsd2, &FrontBuffer, NULL);
		if (r != DD_OK)
		{
			ddsd2.dwBackBufferCount = 1;
			r = DD->CreateSurface(&ddsd2, &FrontBuffer, NULL);
			if (r != DD_OK)
			{
				ErrorDX(r, "Can't create draw surfaces!");
				return FALSE;
			}
		}

		ddscaps2.dwCaps = DDSCAPS_BACKBUFFER;
		r = FrontBuffer->GetAttachedSurface(&ddscaps2, &BackBuffer);
		if (r != DD_OK)
		{
			ErrorDX(r, "Can't attach back buffer!");
			return FALSE;
		}
	}
	else
	{
		ZeroMemory(&ddsd2, sizeof(ddsd2));
		ddsd2.dwSize = sizeof(ddsd2);
		ddsd2.dwFlags = DDSD_CAPS;
		ddsd2.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_VIDEOMEMORY;
		ddsd2.ddsCaps.dwCaps2 = 0;

		r = DD->CreateSurface(&ddsd2, &FrontBuffer, NULL);
		if (r != DD_OK)
		{
			ErrorDX(r, "Can't create primary surface!");
			return FALSE;
		}

		ddsd2.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
		ddsd2.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_3DDEVICE | DDSCAPS_VIDEOMEMORY;
		ddsd2.dwWidth = ScreenXsize;
		ddsd2.dwHeight = ScreenYsize;

		r = DD->CreateSurface(&ddsd2, &BackBuffer, NULL);
		if (r != DD_OK)
		{
			ErrorDX(r, "Can't create back buffer!");
			return FALSE;
		}

		r = DD->CreateClipper(0, &clipper, NULL);
		if (r != DD_OK)
		{
			ErrorDX(r, "Can't create clipper!");
			return FALSE;
		}

		clipper->SetHWnd(0, hwnd);
		FrontBuffer->SetClipper(clipper);
		RELEASE(clipper);
	}

// attach Z buffer

	r = BackBuffer->AddAttachedSurface(ZedBuffer);
	if (r != DD_OK)
	{
		ErrorDX(r, "Can't attach Z buffer");
		return FALSE;
	}

// create D3D device

	r = D3D->CreateDevice(IID_IDirect3DHALDevice, BackBuffer, &D3Ddevice, NULL);
	if (r != DD_OK)
	{
		ErrorDX(r, "Can't create a 3D device!");
		return FALSE;
	}

// get D3D device caps

	ZeroMemory(&hal, sizeof(hal));
	hal.dwSize = sizeof(hal);

	ZeroMemory(&hel, sizeof(hel));
	hel.dwSize = sizeof(hel);

	r = D3Ddevice->GetCaps(&hal, &hel);

	if (r != DD_OK)
	{
		ErrorDX(r, "Can't get D3D device caps");
		return FALSE;
	}

	D3Dcaps = hal;

// create / setup viewport

	r = D3D->CreateViewport(&D3Dviewport, NULL);
	if (r != DD_OK)
	{
		ErrorDX(r, "Can't create a viewport");
		return FALSE;
	}

	r = D3Ddevice->AddViewport(D3Dviewport);
	if (r != DD_OK)
	{
		ErrorDX(r, "Can't attach viewport to 3D device");
		return FALSE;
	}

	ZeroMemory(&vd, sizeof(vd));
	vd.dwSize = sizeof(vd);
	vd.dwX = 0;
	vd.dwY = 0;
    vd.dwWidth = ScreenXsize;
    vd.dwHeight = ScreenYsize;

    vd.dvClipX = 0;
    vd.dvClipY = 0;
    vd.dvClipWidth = (float)ScreenXsize;
    vd.dvClipHeight = (float)ScreenYsize;
    vd.dvMinZ = 0;
    vd.dvMaxZ = 1;

    r = D3Dviewport->SetViewport2(&vd);
	if (r != DD_OK)
	{
		ErrorDX(r, "Can't set viewport");
		return FALSE;
	}

    r = D3Ddevice->SetCurrentViewport(D3Dviewport);
	if (r != DD_OK)
	{
		ErrorDX(r, "Can't set current viewport");
		return FALSE;
	}

// get gamma control interface

	if (NoGamma | !(DDcaps.dwCaps2 & DDCAPS2_PRIMARYGAMMA))
	{
		GammaFlag = GAMMA_UNAVAILABLE;
	}
	else
	{
	    r = FrontBuffer->QueryInterface(IID_IDirectDrawGammaControl, (void**)&GammaControl);
		if (r != DD_OK)
		{
			ErrorDX(r, "Can't get gamma interface");
			return FALSE;
		}

//		if (DDcaps.dwCaps2 & DDCAPS2_CANCALIBRATEGAMMA)
//			GammaFlag = GAMMA_AUTO;
//		else
			GammaFlag = GAMMA_AVAILABLE;

		SetGamma(RegistrySettings.Brightness, RegistrySettings.Contrast);
	}

// decide screen / zbuffer clear technique

	BackgroundColor = 0x000000;
	ViewportRect.x1 = 0;
	ViewportRect.y1 = 0;
	ViewportRect.x2 = ScreenXsize;
	ViewportRect.y2 = ScreenYsize;

	PolyClear = FALSE;
	start = CurrentTimer();

	for (i = 0 ; i < 50 ; i++)
	{
		FlipBuffers();
		ClearBuffers();
	}

	end = CurrentTimer();
	time1 = end - start;

	PolyClear = TRUE;
	start = CurrentTimer();

	for (i = 0 ; i < 50 ; i++)
	{
		FlipBuffers();
		ClearBuffers();
	}

	end = CurrentTimer();
	time2 = end - start;

	PolyClear = (time2 < time1);

// set misc render states

	CULL_OFF();
	SPECULAR_OFF();
	TEXTURE_ADDRESS(D3DTADDRESS_CLAMP);
	MIPMAP_LODBIAS(-0.02f);

// return ok

	return TRUE;
}

//////////////////////
// set gamma values //
//////////////////////

void SetGamma(long brightness, long contrast)
{
	long i;
	float step, middle, n;
	DDGAMMARAMP ramp;

// skip if no gamma control

	if (GammaFlag == GAMMA_UNAVAILABLE)
		return;

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
}

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
			QuitGame = TRUE;
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
			QuitGame = TRUE;
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

			if (!texinfo.MipCount)
				LoadTexture(texinfo.File, i, texinfo.Width, texinfo.Height, texinfo.Stage);
			else
				LoadMipTexture(texinfo.File, i, texinfo.Width, texinfo.Height, texinfo.Stage, texinfo.MipCount);
		}
	}
}

//////////////////
// flip buffers //
//////////////////

void FlipBuffers(void)
{
	RECT dest;
	long bx, by, cy;

	if (FullScreen)
	{
		while (FrontBuffer->Flip(NULL, DDFLIP_NOVSYNC) == DDERR_WASSTILLDRAWING);
	}
	else
	{
		GetWindowRect(hwnd, &dest);
		bx = GetSystemMetrics(SM_CXSIZEFRAME);
		by = GetSystemMetrics(SM_CYSIZEFRAME);
		cy = GetSystemMetrics(SM_CYCAPTION);

		dest.left += bx;
		dest.right -= bx;
		dest.top += cy + by;
		dest.bottom -= by;

		FrontBuffer->Blt(&dest, BackBuffer, NULL, DDBLT_WAIT, NULL);
	}
}

/////////////////////////////
// clear zed + back buffer //
/////////////////////////////

void ClearBuffers(void)
{
	float zres, z;

	if (!PolyClear)
	{
		D3Dviewport->Clear2(1, &ViewportRect, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, BackgroundColor, 1.0f, 0);
	}
	else
	{
		zres = (float)(1 << ZedBufferFormat.dwZBufferBitDepth);
		z = (zres - 1.0f) / zres;

		DrawVertsTEX0[0].sx = (float)ViewportRect.x1;
		DrawVertsTEX0[0].sy = (float)ViewportRect.y1;
		DrawVertsTEX0[0].sz = z;
		DrawVertsTEX0[0].rhw = 1.0f;
		DrawVertsTEX0[0].color = BackgroundColor;

		DrawVertsTEX0[1].sx = (float)ViewportRect.x2;
		DrawVertsTEX0[1].sy = (float)ViewportRect.y1;
		DrawVertsTEX0[1].sz = z;
		DrawVertsTEX0[1].rhw = 1.0f;
		DrawVertsTEX0[1].color = BackgroundColor;

		DrawVertsTEX0[2].sx = (float)ViewportRect.x2;
		DrawVertsTEX0[2].sy = (float)ViewportRect.y2;
		DrawVertsTEX0[2].sz = z;
		DrawVertsTEX0[2].rhw = 1.0f;
		DrawVertsTEX0[2].color = BackgroundColor;

		DrawVertsTEX0[3].sx = (float)ViewportRect.x1;
		DrawVertsTEX0[3].sy = (float)ViewportRect.y2;
		DrawVertsTEX0[3].sz = z;
		DrawVertsTEX0[3].rhw = 1.0f;
		DrawVertsTEX0[3].color = BackgroundColor;

		SET_TPAGE(-1);

		ZCMP(D3DCMP_ALWAYS);
		D3Ddevice->DrawPrimitive(D3DPT_TRIANGLEFAN, FVF_TEX0, DrawVertsTEX0, 4, D3DDP_DONOTUPDATEEXTENTS | D3DDP_DONOTCLIP);
		ZCMP(D3DCMP_LESSEQUAL);
	}
}

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

//////////////////////////
// set background color //
//////////////////////////

void SetBackgroundColor(long col)
{
	BackgroundColor = col;
}

////////////////
// Release dx //
////////////////

void ReleaseDX(void)
{
	RELEASE(D3D);
	RELEASE(DD);
}

/////////////////
// Release d3d //
/////////////////

void ReleaseD3D(void)
{
    RELEASE(D3Ddevice);
    RELEASE(GammaControl);
    RELEASE(D3Dviewport);
    RELEASE(GammaControl);
    RELEASE(ZedBuffer);
    RELEASE(BackBuffer);
    RELEASE(FrontBuffer);
}

/////////////////////
// Report DX error //
/////////////////////

void ErrorDX(HRESULT r, char *mess)
{
	ERRORDX *p = ErrorListDX;

	while (p->Result != DD_OK && p->Result != r) p++;
	Box(p->Error, mess, MB_OK);
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

// perspective

	if (D3Dcaps.dwFlags & D3DDD_TRICAPS && D3Dcaps.dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_PERSPECTIVE)
	{
		DxState.PerspectiveEnabled = TRUE;
		DxState.Perspective = TRUE;
	}
	else
	{
		DxState.PerspectiveEnabled = FALSE;
		DxState.Perspective = FALSE;
	}

	PERSPECTIVE_ON();

// texture filtering

	DxState.TextureFilterFlag = 1;

	if (D3Dcaps.dwFlags & D3DDD_TRICAPS && D3Dcaps.dpcTriCaps.dwTextureFilterCaps & D3DPTFILTERCAPS_LINEAR) DxState.TextureFilterFlag |= 2;
	if (D3Dcaps.dwFlags & D3DDD_TRICAPS && D3Dcaps.dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_ANISOTROPY) DxState.TextureFilterFlag |= 4;

	for (i = 0 ; i < 3 ; i++) if (DxState.TextureFilterFlag & (1 << i)) DxState.TextureFilter = i;

	TEXTUREFILTER_ON();

// mip map

	DxState.MipMapFlag = 1;

	if (D3Dcaps.dwFlags & D3DDD_TRICAPS && D3Dcaps.dpcTriCaps.dwTextureFilterCaps & (D3DPTFILTERCAPS_MIPNEAREST | D3DPTFILTERCAPS_MIPLINEAR)) DxState.MipMapFlag |= 2;
	if (D3Dcaps.dwFlags & D3DDD_TRICAPS && D3Dcaps.dpcTriCaps.dwTextureFilterCaps & (D3DPTFILTERCAPS_LINEARMIPNEAREST | D3DPTFILTERCAPS_LINEARMIPLINEAR)) DxState.MipMapFlag |= 4;

	for (i = 0 ; i < 3 ; i++) if (DxState.MipMapFlag & (1 << i)) DxState.MipMap = i;

	MIPMAP_ON();

// fog

	if (D3Dcaps.dwFlags & D3DDD_TRICAPS && D3Dcaps.dpcTriCaps.dwShadeCaps & D3DPSHADECAPS_FOGGOURAUD)
	{
		DxState.FogEnabled = TRUE;
		DxState.Fog = TRUE;
	}
	else
	{
		DxState.FogEnabled = FALSE;
		DxState.Fog = FALSE;
	}

	FOG_OFF();

// dither

	if (D3Dcaps.dwFlags & D3DDD_TRICAPS && D3Dcaps.dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_DITHER)
	{
		DxState.DitherEnabled = TRUE;
		DxState.Dither = TRUE;
	}
	else
	{
		DxState.DitherEnabled = FALSE;
		DxState.Dither = FALSE;
	}

	DITHER_ON();

// color keying

	if (D3Dcaps.dwFlags & D3DDD_TRICAPS && D3Dcaps.dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_TRANSPARENCY && !NoColorKey)
	{
		DxState.ColorKeyEnabled = TRUE;
		DxState.ColorKey = TRUE;
	}
	else
	{
		DxState.ColorKeyEnabled = FALSE;
		DxState.ColorKey = FALSE;
	}

	COLORKEY_ON();

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
}

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
		Box(NULL, "Can't create draw device", MB_OK);
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
		wsprintf(buf, "Can't create draw device '%s'", szName);
		Box(NULL, buf, MB_OK);
		return DDENUMRET_CANCEL;
	}

	r = DD->Initialize(lpGUID);
	if (r != DD_OK)
	{
		wsprintf(buf, "Can't init draw device '%s'", szName);
		ErrorDX(r, buf);
		return DDENUMRET_CANCEL;
	}

    return DDENUMRET_CANCEL;
}

///////////////////////////
// display mode callback //
///////////////////////////

HRESULT CALLBACK DisplayModesCallback(DDSURFACEDESC2 *Mode, void *UserArg)
{

// list full?

	if (DrawDevices[DrawDeviceNum].DisplayModeNum >= MAX_DISPLAY_MODES)
		return DDENUMRET_CANCEL;

// skip if crap display mode

	if (!(Mode->ddpfPixelFormat.dwFlags & DDPF_RGB) || Mode->ddpfPixelFormat.dwRGBBitCount < 16)
		return DDENUMRET_OK;

// store mode in current draw device list

	DrawDevices[DrawDeviceNum].DisplayMode[DrawDevices[DrawDeviceNum].DisplayModeNum].Width = Mode->dwWidth;
	DrawDevices[DrawDeviceNum].DisplayMode[DrawDevices[DrawDeviceNum].DisplayModeNum].Height = Mode->dwHeight;
	DrawDevices[DrawDeviceNum].DisplayMode[DrawDevices[DrawDeviceNum].DisplayModeNum].Bpp = Mode->ddpfPixelFormat.dwRGBBitCount;
	DrawDevices[DrawDeviceNum].DisplayMode[DrawDevices[DrawDeviceNum].DisplayModeNum].Refresh = Mode->dwRefreshRate;
	wsprintf(DrawDevices[DrawDeviceNum].DisplayMode[DrawDevices[DrawDeviceNum].DisplayModeNum].DisplayText, "%dx%dx%d", (short)Mode->dwWidth, (short)Mode->dwHeight, (short)Mode->ddpfPixelFormat.dwRGBBitCount);

// best display mode?

	if (Mode->dwWidth <= RegistrySettings.ScreenWidth && Mode->dwHeight <= RegistrySettings.ScreenHeight && Mode->ddpfPixelFormat.dwRGBBitCount <= RegistrySettings.ScreenBpp)
		DrawDevices[DrawDeviceNum].BestDisplayMode = DrawDevices[DrawDeviceNum].DisplayModeNum;

// next please

	DrawDevices[DrawDeviceNum].DisplayModeNum++;
	return DDENUMRET_OK;
}
