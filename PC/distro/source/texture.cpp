
#include "revolt.h"
#include "texture.h"
#include "dx.h"
#include "main.h"

// globals

int TEX_NTPages = 0;
TEXINFO *TexInfo;
DDPIXELFORMAT TexFormat;
char TexturesEnabled, TexturesSquareOnly, TexturesAGP;
long TexturePixels, WorldTextureSet, CarTextureSet, FxTextureSet;
DWORD TextureMinWidth, TextureMaxWidth, TextureMinHeight, TextureMaxHeight;

static long TexBppRequest;

/////////////////////////////////////////////////////////////////////
// CreateTPages: allocate space required for the tpages
/////////////////////////////////////////////////////////////////////
bool CreateTPages(int nPages)
{
	if ((TexInfo = (TEXINFO *)malloc(sizeof(TEXINFO) * nPages)) == NULL) {
		return FALSE;
	}
	
	TEX_NTPages = nPages;

	return TRUE;
}

////////////////////////////////////////
// destroy tpages + associated memory //
////////////////////////////////////////

void DestroyTPages()
{
	free(TexInfo);
	TEX_NTPages = 0;
}

////////////////////////
// get texture format //
////////////////////////

void GetTextureFormat(long bpp)
{

// set texture state to off

	TexturesEnabled = FALSE;
	TexFormat.dwRGBBitCount = 0;

// enumerate / find good texture format

	TexBppRequest = bpp;
	D3Ddevice->EnumTextureFormats(FindTextureCallback, NULL);

	if (TexFormat.dwRGBBitCount)
		TexturesEnabled = TRUE;
	else
		TexturesEnabled = FALSE;

// set TextureSquare flag

	if (D3Dcaps.dwFlags & D3DDD_TRICAPS && D3Dcaps.dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_SQUAREONLY)
		TexturesSquareOnly = TRUE;
	else
		TexturesSquareOnly = FALSE;

// set TextureAGP flag

	if (D3Dcaps.dwDevCaps & D3DDEVCAPS_TEXTURENONLOCALVIDMEM)
		TexturesAGP = TRUE;
	else
		TexturesAGP = FALSE;

// get tex min / max size

	TextureMinWidth = D3Dcaps.dwMinTextureWidth;
	TextureMaxWidth = D3Dcaps.dwMaxTextureWidth;
	TextureMinHeight = D3Dcaps.dwMinTextureWidth;
	TextureMaxHeight = D3Dcaps.dwMaxTextureWidth;

}

//////////////////////////////////////////
// decide texture sets for all textures //
//////////////////////////////////////////

void PickTextureSets(long playernum)
{
	long max;

// get max texture pixels

	max = TPAGE_WORLD_NUM + playernum + TPAGE_SCALE_NUM + TPAGE_FIXED_NUM;
	max += max / 3;

	TexturePixels = CountTexturePixels(max, 256, 256);

// pick texture sets

	WorldTextureSet = 0;
	CarTextureSet = 0;
	FxTextureSet = 0;

	if (MipSize(256, WorldTextureSet, TPAGE_WORLD_NUM, TRUE) + MipSize(256, CarTextureSet, playernum, TRUE) + MipSize(256, FxTextureSet, TPAGE_SCALE_NUM, TRUE) + MipSize(256, 0, TPAGE_FIXED_NUM, FALSE) > TexturePixels)
		FxTextureSet++;

	if (MipSize(256, WorldTextureSet, TPAGE_WORLD_NUM, TRUE) + MipSize(256, CarTextureSet, playernum, TRUE) + MipSize(256, FxTextureSet, TPAGE_SCALE_NUM, TRUE) + MipSize(256, 0, TPAGE_FIXED_NUM, FALSE) > TexturePixels)
		CarTextureSet++;

	if (MipSize(256, WorldTextureSet, TPAGE_WORLD_NUM, TRUE) + MipSize(256, CarTextureSet, playernum, TRUE) + MipSize(256, FxTextureSet, TPAGE_SCALE_NUM, TRUE) + MipSize(256, 0, TPAGE_FIXED_NUM, FALSE) > TexturePixels)
		WorldTextureSet++;
}

///////////////////////////////////////////////////////
// calc number of pixels needed for a mipmap texture //
///////////////////////////////////////////////////////

long MipSize(long size, long set, long count, long mip)
{
	long pixels = 0;

	for ( ; set ; set--)
		size /= 2;

	do {
		pixels += size * size;
		size /= 2;
	} while ((size >= 128) && mip);

	return pixels * count;
}

/////////////////////////////
// texture format callback //
/////////////////////////////

HRESULT CALLBACK FindTextureCallback(DDPIXELFORMAT *ddpf, void *lParam)
{

// skip if z buffer only

	if (ddpf->dwFlags & DDPF_ZBUFFER) return DDENUMRET_OK;

// skip if alpha only

	if (ddpf->dwFlags & DDPF_ALPHA) return DDENUMRET_OK;

// skip if has alpha channel

	if (ddpf->dwFlags & DDPF_ALPHAPIXELS) return DDENUMRET_OK;

// skip if less than 16 bpp

    if (ddpf->dwRGBBitCount < 16) return DDENUMRET_OK;

// skip if not RGB

    if (ddpf->dwRGBBitCount > 8 && !(ddpf->dwFlags & DDPF_RGB)) return DDENUMRET_OK;

// take this format if we have nothing so far

	if (TexFormat.dwRGBBitCount == 0) TexFormat = *ddpf;

// take this format if equals request bpp

	if (ddpf->dwRGBBitCount == 16 && TexBppRequest == 16) TexFormat = *ddpf;
	if (ddpf->dwRGBBitCount >= 24 && TexBppRequest == 24) TexFormat = *ddpf;

// return OK

	return DDENUMRET_OK;
}

//////////////////////////
// count texture pixels //
//////////////////////////

long CountTexturePixels(long needed, long width, long height)
{
	long i, max;
	DDSURFACEDESC2 ddsd2;
	HRESULT	r;
	IDirectDrawSurface4 *sourcesurface;
	IDirect3DTexture2 *sourcetexture;
	IDirectDrawSurface4 *surface[MAX_TEXTURE_TEST];
	IDirect3DTexture2 *texture[MAX_TEXTURE_TEST];

// zero count

	max = 0;

// create source surface

	ZeroMemory(&ddsd2, sizeof(ddsd2));
	ddsd2.dwSize = sizeof(ddsd2);
	ddsd2.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT | DDSD_TEXTURESTAGE;
	ddsd2.ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_SYSTEMMEMORY;
	ddsd2.ddsCaps.dwCaps2 = DDSCAPS2_OPAQUE;
	ddsd2.dwWidth = width;
	ddsd2.dwHeight = height;
	ddsd2.ddpfPixelFormat = TexFormat;
	ddsd2.dwTextureStage = 0;

	r = DD->CreateSurface(&ddsd2, &sourcesurface, NULL);
	if (r != DD_OK)
	{
		return 0;
	}

	r = sourcesurface->QueryInterface(IID_IDirect3DTexture2, (void**)&sourcetexture);
	if (r != DD_OK)
	{
		RELEASE(sourcesurface);
		return 0;
	}

// count textures

	if (needed > MAX_TEXTURE_TEST) needed = MAX_TEXTURE_TEST;

	for (i = 0 ; i < needed ; i++)
	{
		ZeroMemory(&ddsd2, sizeof(ddsd2));
		ddsd2.dwSize = sizeof(ddsd2);
		ddsd2.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT | DDSD_TEXTURESTAGE;
		ddsd2.ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_VIDEOMEMORY | DDSCAPS_LOCALVIDMEM | DDSCAPS_ALLOCONLOAD;
		ddsd2.ddsCaps.dwCaps2 = DDSCAPS2_OPAQUE;
		ddsd2.dwWidth = width;
		ddsd2.dwHeight = height;
		ddsd2.ddpfPixelFormat = TexFormat;
		ddsd2.dwTextureStage = 0;

		r = DD->CreateSurface(&ddsd2, &surface[i], NULL);
		if (r != DD_OK)
		{
			break;
		}

		r = surface[i]->QueryInterface(IID_IDirect3DTexture2, (void**)&texture[i]);
		if (r != DD_OK)
		{
			RELEASE(surface[i]);
			break;
		}

		r = texture[i]->Load(sourcetexture);
		if (r != DD_OK)
		{
			RELEASE(texture[i]);
			RELEASE(surface[i]);
			break;
		}

		max++;
	}

// kill textures

	RELEASE(sourcesurface);
	RELEASE(sourcetexture);

	for (i = 0 ; i < max ; i++)
	{
		RELEASE(texture[i]);
		RELEASE(surface[i]);
	}

// return count

	return max * width * height;
}

//////////////////////////////
// count mip texture pixels //
//////////////////////////////

long CountMipTexturePixels(long needed, long width, long height)
{
	long i, max;
	DDSURFACEDESC2 ddsd2;
	HRESULT	r;
	IDirectDrawSurface4 *sourcesurface;
	IDirect3DTexture2 *sourcetexture;
	IDirectDrawSurface4 *surface[MAX_TEXTURE_TEST];
	IDirect3DTexture2 *texture[MAX_TEXTURE_TEST];

// zero count

	max = 0;

// create source surface

	ZeroMemory(&ddsd2, sizeof(ddsd2));
	ddsd2.dwSize = sizeof(ddsd2);
	ddsd2.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT | DDSD_TEXTURESTAGE | DDSD_MIPMAPCOUNT;
	ddsd2.ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_SYSTEMMEMORY | DDSCAPS_MIPMAP | DDSCAPS_COMPLEX;
	ddsd2.ddsCaps.dwCaps2 = DDSCAPS2_OPAQUE;
	ddsd2.dwWidth = width;
	ddsd2.dwHeight = height;
	ddsd2.ddpfPixelFormat = TexFormat;
	ddsd2.dwTextureStage = 0;
	ddsd2.dwMipMapCount = MAX_MIPMAPS;

	r = DD->CreateSurface(&ddsd2, &sourcesurface, NULL);
	if (r != DD_OK)
	{
		return 0;
	}

	r = sourcesurface->QueryInterface(IID_IDirect3DTexture2, (void**)&sourcetexture);
	if (r != DD_OK)
	{
		RELEASE(sourcesurface);
		return 0;
	}

// count textures

	if (needed > MAX_TEXTURE_TEST) needed = MAX_TEXTURE_TEST;

	for (i = 0 ; i < needed ; i++)
	{
		ZeroMemory(&ddsd2, sizeof(ddsd2));
		ddsd2.dwSize = sizeof(ddsd2);
		ddsd2.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT | DDSD_TEXTURESTAGE | DDSD_MIPMAPCOUNT;
		ddsd2.ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_VIDEOMEMORY | DDSCAPS_LOCALVIDMEM | DDSCAPS_ALLOCONLOAD | DDSCAPS_MIPMAP | DDSCAPS_COMPLEX;
		ddsd2.ddsCaps.dwCaps2 = DDSCAPS2_OPAQUE;
		ddsd2.dwWidth = width;
		ddsd2.dwHeight = height;
		ddsd2.ddpfPixelFormat = TexFormat;
		ddsd2.dwTextureStage = 0;
		ddsd2.dwMipMapCount = MAX_MIPMAPS;

		r = DD->CreateSurface(&ddsd2, &surface[i], NULL);
		if (r != DD_OK)
		{
			break;
		}

		r = surface[i]->QueryInterface(IID_IDirect3DTexture2, (void**)&texture[i]);
		if (r != DD_OK)
		{
			RELEASE(surface[i]);
			break;
		}

		r = texture[i]->Load(sourcetexture);
		if (r != DD_OK)
		{
			RELEASE(texture[i]);
			RELEASE(surface[i]);
			break;
		}

		max++;
	}

// kill textures

	RELEASE(sourcesurface);
	RELEASE(sourcetexture);

	for (i = 0 ; i < max ; i++)
	{
		RELEASE(texture[i]);
		RELEASE(surface[i]);
	}

// return count

	return max * width * height;
}

////////////////////////////////
// load texture intelligently //
////////////////////////////////

bool LoadTextureClever(char *tex, char tpage, long width, long height, long stage, long set, long mip)
{
	long i, len, mipcount;
	FILE *fp;
	char buf[256];

// skip if zero length filename

	len = strlen(tex);
	if (!len) return FALSE;

// count mip levels

	mipcount = 0;
	strcpy(buf, tex);

	for (i = 0 ; i < MAX_MIPMAPS ; i++)
	{
		buf[len - 1] = i + 'p';
		fp = fopen(buf, "rb");
		if (!fp) break;
		fclose(fp);
		mipcount++;
	}

	if (!mipcount)
		return FALSE;

// get size

	width >>= set;
	height >>= set;

// get filename

	if (set > mipcount - 1) set = mipcount - 1;
	buf[len - 1] = set + 'p';

// get mip count

	if (!mip)
		mipcount = 1;
	else
		mipcount -= set;

// load

	return LoadMipTexture(buf, tpage, width, height, stage, mipcount);
}

//////////////////
// Load texture //
//////////////////

bool LoadTexture(char *tex, char tpage, long width, long height, long stage)
{
	HBITMAP hbm;
	BITMAP bm;
	DDSURFACEDESC2 ddsd2;
	HRESULT r;
	HDC dcimage, dc;
	unsigned short cols, i;
	char buff[256];
    DWORD adw[256];
	DDCOLORKEY ck;
	short red, green, blue;
	IDirect3DTexture2 *texsource;
	IDirectDrawSurface4 *sourcesurface;

// null file?

	if (!tex || tex[0] == '\0')
		return FALSE;

// check valid tpage

	if (tpage > TEX_NTPages)
		return FALSE;

// return if textures not enabled

	if (!TexturesEnabled)
	{
		TexInfo[tpage].Active = FALSE;
		return FALSE;
	}

// load bitmap

	hbm = (HBITMAP)LoadImage(NULL, tex, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
	if (!hbm)
	{
		wsprintf(buff, "Can't load texture: '%s'", tex);
		Box("ERROR", buff, MB_OK);
		return FALSE;
	}
	GetObject(hbm, sizeof(bm), &bm);

// create texture source surface

	ZeroMemory(&ddsd2, sizeof(ddsd2));
	ddsd2.dwSize = sizeof(ddsd2);
	ddsd2.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT | DDSD_TEXTURESTAGE;
	ddsd2.ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_SYSTEMMEMORY;
	ddsd2.ddsCaps.dwCaps2 = DDSCAPS2_OPAQUE;
	ddsd2.dwWidth = width;
	ddsd2.dwHeight = height;
	ddsd2.ddpfPixelFormat = TexFormat;
	ddsd2.dwTextureStage = stage;

	r = DD->CreateSurface(&ddsd2, &sourcesurface, NULL);
	if (r != DD_OK)
	{
		wsprintf(buff, "Can't create texture source surface for '%s'!", tex);
		ErrorDX(r, buff);
		return FALSE;
	}

// create texture dest surface

	ZeroMemory(&ddsd2, sizeof(ddsd2));
	ddsd2.dwSize = sizeof(ddsd2);
	ddsd2.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT | DDSD_TEXTURESTAGE;
	ddsd2.ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_VIDEOMEMORY | DDSCAPS_LOCALVIDMEM | DDSCAPS_ALLOCONLOAD;
	ddsd2.ddsCaps.dwCaps2 = DDSCAPS2_OPAQUE;
	ddsd2.dwWidth = width;
	ddsd2.dwHeight = height;
	ddsd2.ddpfPixelFormat = TexFormat;
	ddsd2.dwTextureStage = stage;

	r = DD->CreateSurface(&ddsd2, &TexInfo[tpage].Surface, NULL);
	if (r != DD_OK)
	{
		wsprintf(buff, "Can't create texture dest surface for '%s'!", tex);
		ErrorDX(r, buff);
		return FALSE;
	}

// create a palette if 8 bit textures

	if (TexFormat.dwRGBBitCount == 8)
	{
		dcimage = CreateCompatibleDC(NULL);
		SelectObject(dcimage, hbm);
		cols = GetDIBColorTable(dcimage, 0, 256, (RGBQUAD*)adw);
		DeleteDC(dcimage);

		for (i = 0 ; i < cols ; i++)
		{
			red = GetRValue(adw[i]);
			green = GetGValue(adw[i]);
			blue = GetBValue(adw[i]);

			adw[i] = RGB_MAKE(red, green, blue);
		}

		r = DD->CreatePalette(DDPCAPS_8BIT, (PALETTEENTRY*)adw, &TexInfo[tpage].Palette, NULL);
		if (r != DD_OK)
		{
			ErrorDX(r, "Can't create texture palette");
			return FALSE;
		}

		r = sourcesurface->SetPalette(TexInfo[tpage].Palette);
		if (r != DD_OK)
		{
			ErrorDX(r, "Can't attach palette to texture source surface");
			return FALSE;
		}

		r = TexInfo[tpage].Surface->SetPalette(TexInfo[tpage].Palette);
		if (r != DD_OK)
		{
			ErrorDX(r, "Can't attach palette to texture dest surface");
			return FALSE;
		}
	}

// copy bitmap to source surface

	dcimage = CreateCompatibleDC(NULL);
	SelectObject(dcimage, hbm);

	r = sourcesurface->GetDC(&dc);
	if (r != DD_OK)
	{
		ErrorDX(r, "Can't get texture surface DC");
		return FALSE;
	}

	r = StretchBlt(dc, 0, 0, width, height, dcimage, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
	if (!r)
	{
		ErrorDX(r, "Can't blit to texture surface");
		return FALSE;
	}

	sourcesurface->ReleaseDC(dc);
	DeleteDC(dcimage);

// query interface for surfaces

	r = TexInfo[tpage].Surface->QueryInterface(IID_IDirect3DTexture2, (void**)&TexInfo[tpage].Texture);
	if (r != DD_OK)
	{
		ErrorDX(r, "Can't query interface for texture dest surface");
		return FALSE;
	}

	r = sourcesurface->QueryInterface(IID_IDirect3DTexture2, (void**)&texsource);
	if (r != DD_OK)
	{
		ErrorDX(r, "Can't query interface for texture source surface");
		return FALSE;
	}

// load dest with source

	r = TexInfo[tpage].Texture->Load(texsource);
	if (r != DD_OK)
	{
		ErrorDX(r, "Can't load dest texture");
		return FALSE;
	}

// set color key for dest surface

	if (DxState.ColorKey)
	{
		ck.dwColorSpaceLowValue = 0;
		ck.dwColorSpaceHighValue = 0;
		TexInfo[tpage].Surface->SetColorKey(DDCKEY_SRCBLT, &ck);
	}

// release source tex + surface

	RELEASE(texsource);
	RELEASE(sourcesurface);

// kill bitmap object

	DeleteObject(hbm);

// set info flags

	TexInfo[tpage].Active = TRUE;
	TexInfo[tpage].Width = width;
	TexInfo[tpage].Height = height;
	TexInfo[tpage].Stage = stage;
	TexInfo[tpage].MipCount = 0;
	wsprintf(TexInfo[tpage].File, "%s", tex);

// return OK

	return TRUE;
}

/////////////////////////
// Load mipmap texture //
/////////////////////////

bool LoadMipTexture(char *tex, char tpage, long width, long height, long stage, long mipcount)
{
	HBITMAP hbm;
	BITMAP bm;
	DDSURFACEDESC2 ddsd2;
	DDSCAPS2 ddscaps2;
	HRESULT r;
	HDC dcimage, dc;
	unsigned short cols, i, miploop;
	char buff[256];
    DWORD adw[256];
	DDCOLORKEY ck;
	short red, green, blue;
	IDirect3DTexture2 *texsource;
	IDirectDrawSurface4 *sourcesurface;
	IDirectDrawSurface4 *mipsource, *mipdest, *mipsourcenext, *mipdestnext;

// null file?

	if (!tex || tex[0] == '\0')
		return FALSE;

// check valid tpage

	if (tpage > TEX_NTPages)
		return FALSE;

// return if textures not enabled

	if (!TexturesEnabled)
	{
		TexInfo[tpage].Active = FALSE;
		return FALSE;
	}

// check mipcount

	if (mipcount < 0 || mipcount > MAX_MIPMAPS)
		return FALSE;

// create texture source surfaces

	ZeroMemory(&ddsd2, sizeof(ddsd2));
	ddsd2.dwSize = sizeof(ddsd2);
	ddsd2.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT | DDSD_MIPMAPCOUNT | DDSD_TEXTURESTAGE;
	ddsd2.ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_SYSTEMMEMORY | DDSCAPS_MIPMAP | DDSCAPS_COMPLEX;
	ddsd2.ddsCaps.dwCaps2 = DDSCAPS2_OPAQUE;
	ddsd2.dwWidth = width;
	ddsd2.dwHeight = height;
	ddsd2.ddpfPixelFormat = TexFormat;
	ddsd2.dwMipMapCount = mipcount;
	ddsd2.dwTextureStage = stage;

	r = DD->CreateSurface(&ddsd2, &sourcesurface, NULL);
	if (r != DD_OK)
	{
		wsprintf(buff, "Can't create mipmap source surfaces for '%s'!", tex);
		ErrorDX(r, buff);
		return FALSE;
	}

// create texture dest surfaces

	ZeroMemory(&ddsd2, sizeof(ddsd2));
	ddsd2.dwSize = sizeof(ddsd2);
	ddsd2.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT | DDSD_MIPMAPCOUNT | DDSD_TEXTURESTAGE;
	ddsd2.ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_VIDEOMEMORY | DDSCAPS_LOCALVIDMEM | DDSCAPS_ALLOCONLOAD | DDSCAPS_MIPMAP | DDSCAPS_COMPLEX;
	ddsd2.ddsCaps.dwCaps2 = DDSCAPS2_OPAQUE;
	ddsd2.dwWidth = width;
	ddsd2.dwHeight = height;
	ddsd2.ddpfPixelFormat = TexFormat;
	ddsd2.dwMipMapCount = mipcount;
	ddsd2.dwTextureStage = stage;

	r = DD->CreateSurface(&ddsd2, &TexInfo[tpage].Surface, NULL);
	if (r != DD_OK)
	{
		wsprintf(buff, "Can't create mipmap dest surfaces for '%s'!", tex);
		ErrorDX(r, buff);
		return FALSE;
	}

// load each mipmap into source, set palette for source and dest

	mipsource = sourcesurface;
	mipsource->AddRef();

	mipdest = TexInfo[tpage].Surface;
	mipdest->AddRef();

	for (miploop = 0 ; miploop < mipcount ; miploop++)
	{

// load bitmap

		memcpy(buff, tex, MAX_TPAGE_FILENAME);
		buff[strlen(buff) - 1] += miploop;
		hbm = (HBITMAP)LoadImage(NULL, buff, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
		if (!hbm)
		{
			wsprintf(buff, "Can't load mipmap texture: '%s'", buff);
			Box("ERROR", buff, MB_OK);
			return FALSE;
		}
		GetObject(hbm, sizeof(bm), &bm);

// create a palette if 8 bit textures

		if (TexFormat.dwRGBBitCount == 8)
		{
			if (!miploop)
			{
				dcimage = CreateCompatibleDC(NULL);
				SelectObject(dcimage, hbm);
				cols = GetDIBColorTable(dcimage, 0, 256, (RGBQUAD*)adw);
				DeleteDC(dcimage);

				for (i = 0 ; i < cols ; i++)
				{
					red = GetRValue(adw[i]);
					green = GetGValue(adw[i]);
					blue = GetBValue(adw[i]);

					adw[i] = RGB_MAKE(red, green, blue);
				}

				r = DD->CreatePalette(DDPCAPS_8BIT, (PALETTEENTRY*)adw, &TexInfo[tpage].Palette, NULL);
				if (r != DD_OK)
				{
					ErrorDX(r, "Can't create texture palette");
					return FALSE;
				}
			}

			r = mipsource->SetPalette(TexInfo[tpage].Palette);
			if (r != DD_OK)
			{
				ErrorDX(r, "Can't attach palette to texture source surface");
				return FALSE;
			}

			r = mipdest->SetPalette(TexInfo[tpage].Palette);
			if (r != DD_OK)
			{
				ErrorDX(r, "Can't attach palette to texture dest surface");
				return FALSE;
			}
		}

// copy bitmap to source surface

		dcimage = CreateCompatibleDC(NULL);
		SelectObject(dcimage, hbm);

		r = mipsource->GetDC(&dc);
		if (r != DD_OK)
		{
			ErrorDX(r, "Can't get texture surface DC");
			return FALSE;
		}

		r = StretchBlt(dc, 0, 0, width >> miploop, height >> miploop, dcimage, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
		if (!r)
		{
			ErrorDX(r, "Can't blit to texture surface");
			return FALSE;
		}

		mipsource->ReleaseDC(dc);
		DeleteDC(dcimage);

// set color key for dest surface

		if (DxState.ColorKey)
		{
			ck.dwColorSpaceLowValue = 0;
			ck.dwColorSpaceHighValue = 0;
			mipdest->SetColorKey(DDCKEY_SRCBLT, &ck);
		}

// kill bitmap object

		DeleteObject(hbm);

// get child surfaces if not last mipmap

		if (miploop != mipcount - 1)
		{
			ddscaps2.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_MIPMAP | DDSCAPS_SYSTEMMEMORY;
			r = mipsource->GetAttachedSurface(&ddscaps2, &mipsourcenext);
			if (r != DD_OK)
			{
				ErrorDX(r, "Can't get attached surface for source mipmap");
				return FALSE;
			}

			ddscaps2.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_MIPMAP | DDSCAPS_ALLOCONLOAD;
			r = mipdest->GetAttachedSurface(&ddscaps2, &mipdestnext);
			if (r != DD_OK)
			{
				ErrorDX(r, "Can't get attached surface for dest mipmap");
				return FALSE;
			}
		}

		RELEASE(mipsource);
		mipsource = mipsourcenext;

		RELEASE(mipdest);
		mipdest = mipdestnext;
	}

// query interface for surfaces

	r = TexInfo[tpage].Surface->QueryInterface(IID_IDirect3DTexture2, (void**)&TexInfo[tpage].Texture);
	if (r != DD_OK)
	{
		ErrorDX(r, "Can't query interface for texture dest surface");
		return FALSE;
	}

	r = sourcesurface->QueryInterface(IID_IDirect3DTexture2, (void**)&texsource);
	if (r != DD_OK)
	{
		ErrorDX(r, "Can't query interface for texture source surface");
		return FALSE;
	}

// load dest with source

	r = TexInfo[tpage].Texture->Load(texsource);
	if (r != DD_OK)
	{
		ErrorDX(r, "Can't load dest texture");
		return FALSE;
	}

// set info flags

	TexInfo[tpage].Active = TRUE;
	TexInfo[tpage].Width = width;
	TexInfo[tpage].Height = height;
	TexInfo[tpage].Stage = stage;
	TexInfo[tpage].MipCount = mipcount;
	wsprintf(TexInfo[tpage].File, "%s", tex);

// release source tex + surface

	RELEASE(texsource);
	RELEASE(sourcesurface);

// return OK

	return TRUE;
}

///////////////////
// init textures //
///////////////////

void InitTextures(void)
{
	char i;

	RenderTP = -2;
	RenderTP2 = -2;

	for (i = 0 ; i < TEX_NTPages ; i++)
	{
		TexInfo[i].Active = FALSE;
		TexInfo[i].Palette = NULL;
		TexInfo[i].Texture = NULL;
		TexInfo[i].Surface = NULL;
	}
}

///////////////////////
// Free all textures //
///////////////////////

void FreeTextures(void)
{
	char i;

	for (i = 0 ; i < TEX_NTPages ; i++)
	{
		if (TexInfo[i].Active)
			FreeOneTexture(i);
	}
}

//////////////////////
// free one texture //
//////////////////////

void FreeOneTexture(char tp)
{

// used texture?

	if (!TexInfo[tp].Active)
		return;

// free

	TexInfo[tp].Active = NULL;
	RELEASE(TexInfo[tp].Palette);
	RELEASE(TexInfo[tp].Texture);
	RELEASE(TexInfo[tp].Surface);
}
