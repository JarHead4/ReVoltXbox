//-----------------------------------------------------------------------------
// File: texture.cpp
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"
#include "settings.h"
#include "texture.h"
#include "dx.h"
#include "main.h"

// globals

DDPIXELFORMAT TexFormat, TexFormatProcedural;
//$REMOVEDDDPIXELFORMAT TexFormat16, TexFormat24;
//$REMOVEDchar TexturesEnabled, TexturesSquareOnly, TexturesAGP;
TEXINFO *TexInfo;
int TEX_NTPages = 0;
//$REMOVEDlong TexturePixels;
long WorldTextureSet, CarTextureSet, FxTextureSet;
//$REMOVEDDWORD TextureMinWidth, TextureMaxWidth, TextureMinHeight, TextureMaxHeight;

//$REMOVEDstatic long TexBppRequest;

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

void PickTextureFormat(void)  //$RENAMED(cprince): was originally GetTextureFormat
{
//$MODIFIED
//
//// set texture state to off
//
//    TexFormat.dwFlags = 0;
//    TexFormat16.dwFlags = 0;
//    TexFormat24.dwFlags = 0;
//    TexFormatProcedural.dwFlags = 0;
//
//// enumerate / find good texture format
//
//    D3Ddevice->EnumTextureFormats(FindTextureCallback, NULL);
//
//    if (RegistrySettings.Texture24 && TexFormat24.dwFlags)
//        TexFormat = TexFormat24;
//    else
//        TexFormat = TexFormat16;
//
//    TexturesEnabled = (TexFormat.dwFlags != 0);
//
//// set TextureSquare flag
//
//    if (D3Dcaps.dwFlags & D3DDD_TRICAPS && D3Dcaps.dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_SQUAREONLY)
//        TexturesSquareOnly = TRUE;
//    else
//        TexturesSquareOnly = FALSE;
//
//// set TextureAGP flag
//
//    if (D3Dcaps.dwDevCaps & D3DDEVCAPS_TEXTURENONLOCALVIDMEM)
//        TexturesAGP = TRUE;
//    else
//        TexturesAGP = FALSE;
//
//// get tex min / max size
//
//    TextureMinWidth = D3Dcaps.dwMinTextureWidth;
//    TextureMaxWidth = D3Dcaps.dwMaxTextureWidth;
//    TextureMinHeight = D3Dcaps.dwMinTextureWidth;
//    TextureMaxHeight = D3Dcaps.dwMaxTextureWidth;

    //$NOTE: Ah, screw this -- you're getting A8R8G8B8.  Quit yer whinin'.
    TexFormat = D3DFMT_A8R8G8B8;
    TexFormatProcedural = D3DFMT_A8R8G8B8;
//$END_MODIFICATIONS
}

//////////////////////////////////////////
// decide texture sets for all textures //
//////////////////////////////////////////

void PickTextureSets(long playernum, long worldnum, long scalenum, long fixednum)
{
//$MODIFIED
//    long max;
//
//// get max texture pixels
//
//    max = worldnum + playernum + scalenum + fixednum;
//    max += max / 3;
//
//    TexturePixels = CountTexturePixels(max, 256, 256);
//
//// pick texture sets
//
//    WorldTextureSet = 0;
//    CarTextureSet = 0;
//    FxTextureSet = 0;
//
//    if (MipSize(256, WorldTextureSet, worldnum, TRUE) + MipSize(256, CarTextureSet, playernum, TRUE) + MipSize(256, FxTextureSet, scalenum, TRUE) + MipSize(256, 0, fixednum, FALSE) > TexturePixels)
//        FxTextureSet++;
//
//    if (MipSize(256, WorldTextureSet, worldnum, TRUE) + MipSize(256, CarTextureSet, playernum, TRUE) + MipSize(256, FxTextureSet, scalenum, TRUE) + MipSize(256, 0, fixednum, FALSE) > TexturePixels)
//        CarTextureSet++;
//
//    if (MipSize(256, WorldTextureSet, worldnum, TRUE) + MipSize(256, CarTextureSet, playernum, TRUE) + MipSize(256, FxTextureSet, scalenum, TRUE) + MipSize(256, 0, fixednum, FALSE) > TexturePixels)
//        WorldTextureSet++;

    //$HACK: use highest-quality texture sets everywhere
    WorldTextureSet = 0;
    CarTextureSet   = 0;
    FxTextureSet    = 0;
//$END_MODIFICATIONS
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


/* $REMOVED

/////////////////////////////
// texture format callback //
/////////////////////////////

HRESULT CALLBACK FindTextureCallback(DDPIXELFORMAT *ddpf, void *lParam)
{

// skip if z buffer only

    if (ddpf->dwFlags & DDPF_ZBUFFER)
        return DDENUMRET_OK;

// skip if alpha only

    if (ddpf->dwFlags & DDPF_ALPHA)
        return DDENUMRET_OK;

// skip if not RGB

    if (!(ddpf->dwFlags & DDPF_RGB))
        return DDENUMRET_OK;

// take this format for procedural if 8 bit palettized

    if (ddpf->dwFlags & DDPF_PALETTEINDEXED8)
        TexFormatProcedural = *ddpf;

// skip if incorrect alpha

    if (!(ddpf->dwFlags & DDPF_ALPHAPIXELS))
        return DDENUMRET_OK;

    if (ddpf->dwRGBAlphaBitMask != 0x8000 && ddpf->dwRGBAlphaBitMask != 0xff000000)
        return DDENUMRET_OK;

// skip if less than 16 bpp

    if (ddpf->dwRGBBitCount < 16)
        return DDENUMRET_OK;

// take this format for procedural if 24 bit and we have nothing yet

    if (!TexFormatProcedural.dwFlags && ddpf->dwRGBBitCount >= 24)
        TexFormatProcedural = *ddpf;

// take this format for 16 or 24 bpp

    if (ddpf->dwRGBBitCount == 16)
        TexFormat16 = *ddpf;

    if (ddpf->dwRGBBitCount >= 24)
        TexFormat24 = *ddpf;

// return OK

    return DDENUMRET_OK;
}
$END_REMOVAL */


/* $REMOVED

//////////////////////////
// count texture pixels //
//////////////////////////

long CountTexturePixels(long needed, long width, long height)
{
    long i, max;
    DDSURFACEDESC2 ddsd2;
    HRESULT r;
    IDirectDrawSurface4 *sourcesurface;
    IDirect3DTexture2 *sourcetexture;
    IDirectDrawSurface4 *surface[MAX_TEXTURE_TEST];
    IDirect3DTexture2 *texture[MAX_TEXTURE_TEST];

// zero count

    max = 0;

// create source surface

    ZeroMemory(&ddsd2, sizeof(ddsd2));
    ddsd2.dwSize = sizeof(ddsd2);
    ddsd2.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
    ddsd2.ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_SYSTEMMEMORY;
    ddsd2.ddsCaps.dwCaps2 = DDSCAPS2_OPAQUE;
    ddsd2.dwWidth = width;
    ddsd2.dwHeight = height;
    ddsd2.ddpfPixelFormat = TexFormat;

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
        if (!Software) ddsd2.ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_VIDEOMEMORY | DDSCAPS_LOCALVIDMEM | DDSCAPS_ALLOCONLOAD;
        else ddsd2.ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_SYSTEMMEMORY | DDSCAPS_ALLOCONLOAD;
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

$END_REMOVAL */


/* $REMOVED

//////////////////////////////
// count mip texture pixels //
//////////////////////////////

long CountMipTexturePixels(long needed, long width, long height)
{
    long i, max;
    DDSURFACEDESC2 ddsd2;
    HRESULT r;
    IDirectDrawSurface4 *sourcesurface;
    IDirect3DTexture2 *sourcetexture;
    IDirectDrawSurface4 *surface[MAX_TEXTURE_TEST];
    IDirect3DTexture2 *texture[MAX_TEXTURE_TEST];

// zero count

    max = 0;

// create source surface

    ZeroMemory(&ddsd2, sizeof(ddsd2));
    ddsd2.dwSize = sizeof(ddsd2);
    ddsd2.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT | DDSD_MIPMAPCOUNT;
    ddsd2.ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_SYSTEMMEMORY | DDSCAPS_MIPMAP | DDSCAPS_COMPLEX;
    ddsd2.ddsCaps.dwCaps2 = DDSCAPS2_OPAQUE;
    ddsd2.dwWidth = width;
    ddsd2.dwHeight = height;
    ddsd2.ddpfPixelFormat = TexFormat;
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
        if (!Software) ddsd2.ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_VIDEOMEMORY | DDSCAPS_LOCALVIDMEM | DDSCAPS_ALLOCONLOAD | DDSCAPS_MIPMAP | DDSCAPS_COMPLEX;
        else ddsd2.ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_SYSTEMMEMORY | DDSCAPS_ALLOCONLOAD | DDSCAPS_MIPMAP | DDSCAPS_COMPLEX;
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
$END_REMOVAL */

////////////////////////////////
// load texture intelligently //
////////////////////////////////

bool LoadTextureClever(char *tex, int tpage, long width, long height, long stage, long set, long mip)
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
        buf[len - 1] = (char)i + 'p';
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
    buf[len - 1] = (char)set + 'p';

// get mip count

    if (!mip)
        mipcount = 1;
    else
        mipcount -= set;

// load

    return LoadMipTexture(buf, tpage, width, height, stage, mipcount, FALSE);
}

/////////////////////////
// Load mipmap texture //
/////////////////////////

bool LoadMipTexture(char *tex, int tpage, long width, long height, long stage, long mipcount, long second)
{
//$REMOVED
//    HBITMAP hbm;
//    BITMAP bm;
//    DDSURFACEDESC2 ddsd2;
//    DDSCAPS2 ddscaps2;
//    HRESULT r;
//    HDC dcimage, dc;
//    long i, sepia;
//    char buf[256];
//    char buf2[256];
//    DWORD adw[256];
//    DDCOLORKEY ck;
//    short red, green, blue;
//    IDirect3DTexture2 *texsource;
//    IDirectDrawSurface4 *sourcesurface;
//    IDirectDrawSurface4 *mipsource, *mipdest, *mipsourcenext, *mipdestnext;
//    DWORD y, x;
//    IDirectDrawPalette *palette;
//    unsigned short cols, miploop;
//$END_REMOVAL

#ifndef XBOX_NOT_YET_IMPLEMENTED
// sepia?

    sepia = (RenderSettings.Sepia && tpage != TPAGE_FONT && tpage != TPAGE_SPRU);
#endif // ! XBOX_NOT_YET_IMPLEMENTED

// null file?

    if (!tex || tex[0] == '\0')
        return FALSE;

// check valid tpage

    if (tpage > TEX_NTPages)
        return FALSE;

//$REMOVED
//// return if textures not enabled
//
//    if (!TexturesEnabled)
//        return FALSE;
//$END_REMOVAL

// check mipcount

    if (mipcount < 0 || mipcount > MAX_MIPMAPS)
        return FALSE;

// free texture slot if already used

    if (TexInfo[tpage].Active)
        FreeOneTexture(tpage);

//$REMOVED
//// create texture source surfaces
//
//    ZeroMemory(&ddsd2, sizeof(ddsd2));
//    ddsd2.dwSize = sizeof(ddsd2);
//    ddsd2.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT | DDSD_MIPMAPCOUNT;
//    ddsd2.ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_SYSTEMMEMORY | DDSCAPS_MIPMAP | DDSCAPS_COMPLEX;
//    ddsd2.ddsCaps.dwCaps2 = DDSCAPS2_OPAQUE;
//    ddsd2.dwWidth = width;
//    ddsd2.dwHeight = height;
//    ddsd2.ddpfPixelFormat = TexFormat;
//    ddsd2.dwMipMapCount = mipcount;
//
//    r = DD->CreateSurface(&ddsd2, &sourcesurface, NULL);
//    if (r != DD_OK)
//    {
//        sprintf(buf, "Can't create mipmap source surfaces for '%s'!", tex);
//        ErrorDX(r, buf);
//        return FALSE;
//    }
//
//// create texture dest surfaces
//
//    ZeroMemory(&ddsd2, sizeof(ddsd2));
//    ddsd2.dwSize = sizeof(ddsd2);
//    ddsd2.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT | DDSD_MIPMAPCOUNT;
//    if (!Software) ddsd2.ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_VIDEOMEMORY | DDSCAPS_LOCALVIDMEM | DDSCAPS_ALLOCONLOAD | DDSCAPS_MIPMAP | DDSCAPS_COMPLEX;
//    else ddsd2.ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_SYSTEMMEMORY | DDSCAPS_ALLOCONLOAD | DDSCAPS_MIPMAP | DDSCAPS_COMPLEX;
//    ddsd2.ddsCaps.dwCaps2 = DDSCAPS2_OPAQUE;
//    ddsd2.dwWidth = width;
//    ddsd2.dwHeight = height;
//    ddsd2.ddpfPixelFormat = TexFormat;
//    ddsd2.dwMipMapCount = mipcount;
//
//    r = DD->CreateSurface(&ddsd2, &TexInfo[tpage].Surface, NULL);
//    if (r != DD_OK)
//    {
//        sprintf(buf, "Can't create mipmap dest surfaces for '%s'!", tex);
//        ErrorDX(r, buf);
//        return FALSE;
//    }
//$END_REMOVAL

//$MODIFIED
//// load each mipmap into source, set palette for source and dest
//
//    mipsource = sourcesurface;
//    mipsource->AddRef();
//
//    mipdest = TexInfo[tpage].Surface;
//    mipdest->AddRef();
//
//    for (miploop = 0 ; miploop < mipcount ; miploop++)
//    {
//
//// load bitmap
//
//        memcpy(buf, tex, MAX_TPAGE_FILENAME);
//        buf[strlen(buf) - 1] += miploop;
//
//        if (!second)
//        {
//            hbm = (HBITMAP)LoadImage(NULL, buf, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
//        }
//        else
//        {
//            long l;
//            FILE *fp = fopen(buf, "rb");
//            FILE *fp2 = fopen("c:\\__temp__", "wb");
//
//            fseek(fp, 196664, SEEK_SET);
//            while (!feof(fp))
//            {
//                fread(&l, sizeof(l), 1, fp);
//                fwrite(&l, sizeof(l), 1, fp2);
//            }
//
//            fclose(fp);
//            fclose(fp2);
//
//            hbm = (HBITMAP)LoadImage(NULL, "c:\\__temp__", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
//            remove("c:\\__temp__");
//        }
//
//        if (!hbm)
//        {
//            sprintf(buf2, "Can't load mipmap texture: '%s'", buf);
//            DumpMessage("ERROR", buf2);
//            return FALSE;
//        }
//        GetObject(hbm, sizeof(bm), &bm);
//
//// create a palette if 8 bit textures
//
//        if (TexFormat.dwRGBBitCount == 8)
//        {
//            if (!miploop)
//            {
//                dcimage = CreateCompatibleDC(NULL);
//                SelectObject(dcimage, hbm);
//                cols = GetDIBColorTable(dcimage, 0, 256, (RGBQUAD*)adw);
//                DeleteDC(dcimage);
//
//                for (i = 0 ; i < cols ; i++)
//                {
//                    red = GetRValue(adw[i]);
//                    green = GetGValue(adw[i]);
//                    blue = GetBValue(adw[i]);
//
//                    adw[i] = RGB_MAKE(red, green, blue);
//                }
//            }
//
//            r = DD->CreatePalette(DDPCAPS_8BIT, (PALETTEENTRY*)adw, &palette, NULL);
//            if (r != DD_OK)
//            {
//                ErrorDX(r, "Can't create texture palette");
//                return FALSE;
//            }
//
//            r = mipsource->SetPalette(palette);
//            if (r != DD_OK)
//            {
//                ErrorDX(r, "Can't attach palette to texture source surface");
//                return FALSE;
//            }
//
//            r = mipdest->SetPalette(palette);
//            if (r != DD_OK)
//            {
//                ErrorDX(r, "Can't attach palette to texture dest surface");
//                return FALSE;
//            }
//
//            RELEASE(palette);
//        }
//
//// copy bitmap to source surface
//
//        dcimage = CreateCompatibleDC(NULL);
//        SelectObject(dcimage, hbm);
//
//        r = mipsource->GetDC(&dc);
//        if (r != DD_OK)
//        {
//            ErrorDX(r, "Can't get texture surface DC");
//            return FALSE;
//        }
//
//        r = StretchBlt(dc, 0, 0, width >> miploop, height >> miploop, dcimage, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
//        if (!r)
//        {
//            ErrorDX(r, "Can't blit to texture surface");
//            return FALSE;
//        }
//
//        mipsource->ReleaseDC(dc);
//        DeleteDC(dcimage);
#ifndef XBOX_NOT_YET_IMPLEMENTED
// set alpha

        dcimage = CreateCompatibleDC(NULL);
        SelectObject(dcimage, hbm);

        ddsd2.dwSize = sizeof(ddsd2);
        r = mipsource->Lock(NULL, &ddsd2, DDLOCK_WAIT, NULL);
        if (r != DD_OK)
        {
            ErrorDX(r, "Can't lock texture source surface");
        }

        // 16 bit
        if (TexFormat.dwRGBBitCount == 16)
        {
            unsigned short *ptr = (unsigned short*)ddsd2.lpSurface;
            for (y = 0 ; y < ddsd2.dwHeight ; y++)
            {
                for (x = 0 ; x < ddsd2.dwWidth ; x++)
                {
                    if (ptr[x] || GetPixel(dcimage, x, y))
                    {
                        if (sepia)
                        {
                            ptr[x] = SepiaRGB[ptr[x]];
                        }

                        ptr[x] |= 0x8000;
                    }
                    else
                    {
                        ptr[x] &= 0x7fff;
                    }
                }
                ptr += ddsd2.lPitch / 2;
            }
        }

        // 24 bit
        else
        {
            unsigned long *ptr = (unsigned long*)ddsd2.lpSurface;
            for (y = 0 ; y < ddsd2.dwHeight ; y++)
            {
                for (x = 0 ; x < ddsd2.dwWidth ; x++)
                {
                    if (ptr[x] & 0xffffff)
                    {
                        if (sepia)
                        {
                            long rgb = (ptr[x] & 0xf8) >> 3 | (ptr[x] & 0xf800) >> 6 | (ptr[x] & 0xf80000) >> 9;
                            rgb = SepiaRGB[rgb];
                            ptr[x] = (rgb & 0x1f) << 3 | (rgb & 0x3e0) << 6 | (rgb & 0x7c00) << 9;
                        }

                        ptr[x] |= 0xff000000;
                    }
                    else
                    {
                        ptr[x] = 0;
                    }
                }
                ptr += ddsd2.lPitch / 4;
            }
        }

        mipsource->Unlock(NULL);
        DeleteDC(dcimage);
#endif // ! XBOX_NOT_YET_IMPLEMENTED
//// set color key for dest surface
//
//        if (DxState.ColorKey)
//        {
//            ck.dwColorSpaceLowValue = 0;
//            ck.dwColorSpaceHighValue = 0;
//            mipdest->SetColorKey(DDCKEY_SRCBLT, &ck);
//        }
//
//// kill bitmap object
//
//        DeleteObject(hbm);
//
//// get child surfaces if not last mipmap
//
//        if (miploop != mipcount - 1)
//        {
//            ddscaps2.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_MIPMAP | DDSCAPS_SYSTEMMEMORY;
//            r = mipsource->GetAttachedSurface(&ddscaps2, &mipsourcenext);
//            if (r != DD_OK)
//            {
//                ErrorDX(r, "Can't get attached surface for source mipmap");
//                return FALSE;
//            }
//
//            ddscaps2.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_MIPMAP | DDSCAPS_ALLOCONLOAD;
//            r = mipdest->GetAttachedSurface(&ddscaps2, &mipdestnext);
//            if (r != DD_OK)
//            {
//                ErrorDX(r, "Can't get attached surface for dest mipmap");
//                return FALSE;
//            }
//        }
//
//        RELEASE(mipsource);
//        RELEASE(mipdest);
//
//        if (miploop != mipcount - 1)
//        {
//            mipsource = mipsourcenext;
//            mipdest = mipdestnext;
//        }
//    }
//
//// query interface for surfaces
//
//    r = TexInfo[tpage].Surface->QueryInterface(IID_IDirect3DTexture2, (void**)&TexInfo[tpage].Texture);
//    if (r != DD_OK)
//    {
//        ErrorDX(r, "Can't query interface for texture dest surface");
//        return FALSE;
//    }
//
//    r = sourcesurface->QueryInterface(IID_IDirect3DTexture2, (void**)&texsource);
//    if (r != DD_OK)
//    {
//        ErrorDX(r, "Can't query interface for texture source surface");
//        return FALSE;
//    }
//
//// load dest with source
//
//    r = TexInfo[tpage].Texture->Load(texsource);
//    if (r != DD_OK)
//    {
//        sprintf(buf, "Can't load dest texture for '%s'", tex);
//        ErrorDX(r, buf);
//        return FALSE;
//    }

    HRESULT hr =
    //D3DXCreateTextureFromFileA( D3D, tex, &TexInfo[tpage].Texture );
    D3DXCreateTextureFromFileExA( D3Ddevice,
                                  tex,  // filename
                                  width, height, mipcount, // dimensions
                                  0, // usage
                                  D3DFMT_UNKNOWN, // format
                                  XBOX_UNUSED_PARAM, // pool
                                  D3DX_DEFAULT, D3DX_DEFAULT, // filtering
                                  0x00000000, // 0xAARRGGBB color-key value
                                  NULL, // [out] info about source image
                                  NULL, // [out] palette to fill
                                  &TexInfo[tpage].Texture
                                );
    if( D3D_OK != hr )
    {
        DumpMessage("Couldn't load texture", tex);
        return FALSE;
    }
    //$CMP_NOTE: should we use D3DXCreateTextureFromFileExA, so that we can manually specify the width/height/etc based on function args? (But looks like those args will match the file data, so they don't really matter.)
    //$TODO: revisit colorkey stuff, and see what we need to do (maybe just SetRenderState the color-key value to black ???)
      //$HEY: can specify color-key info when loading, if use D3DXCreateTextureFromFileExA !?!
    //$CMP_NOTE: not clear whether we want/need to bother supporting palettized textures for the port.
      //$HEY: looks like can also handle palettes using D3DXCreateTextureFromFileExA !?
    //$TODO: see if we need to implement the stuff commented "set alpha" above.

//$END_MODIFICATIONS

// set info flags

    TexInfo[tpage].Active = TRUE;
    TexInfo[tpage].Width = width;
    TexInfo[tpage].Height = height;
//$REMOVED    TexInfo[tpage].Stage = stage;
//$REMOVED    TexInfo[tpage].MipCount = mipcount;
    sprintf(TexInfo[tpage].File, "%s", tex); //$NOTE(jedl) - this is here to allow easier export

//$REMOVED
//// release source tex + surface
//
//    RELEASE(texsource);
//    RELEASE(sourcesurface);
//$END_REMOVAL

// return OK

    return TRUE;
}



//$ADDITION(jedl) - first pass at resource loading

// $TODO get rid of tpages altogether

///////////////////
// lookup texture resource
///////////////////
HRESULT LoadTextureGPU(char *strIdentifier, char *strTexFile, int tpage, XBResource *pXBR)
{
    CHAR name[_MAX_PATH];
    if (strIdentifier == NULL)
    {
        CHAR drive[_MAX_DRIVE];
        CHAR dir[_MAX_DIR];
        CHAR ext[_MAX_EXT];
       _splitpath( strTexFile, drive, dir, name, ext );
       strIdentifier = name;
    }
    TexInfo[tpage].Texture = pXBR->GetTexture(strIdentifier);
    if (TexInfo[tpage].Texture == NULL)
    {
//$MD
#ifdef _DEBUG
        char szBuff[3*MAX_PATH];
        sprintf( szBuff,
                 "Warning -- Missing Texture.  ID: %s, File: %s\n",
                 strIdentifier ? strIdentifier : "NULL",
                 strTexFile ? strTexFile : "NULL");
        
        OutputDebugString(szBuff);
#endif
        return E_FAIL;
    }
    TexInfo[tpage].Texture->AddRef();
    D3DSURFACE_DESC desc;
    TexInfo[tpage].Texture->GetLevelDesc(0, &desc);
    TexInfo[tpage].Active = TRUE;
    TexInfo[tpage].Width = desc.Width;
    TexInfo[tpage].Height = desc.Height;
    strncpy(TexInfo[tpage].File, strTexFile, MAX_TPAGE_FILENAME);
    TexInfo[tpage].File[MAX_TPAGE_FILENAME - 1] = 0;
    return S_OK;
}
//$END_ADDITION



////////////////////////////////////////
// Create procedural texture surfaces //
////////////////////////////////////////

bool CreateProceduralTexture(int tpage, long width, long height)
{
//    long i;
//    DDSURFACEDESC2 ddsd2;
//    HRESULT r;
//    IDirectDrawPalette *palette;
//    DWORD pal[256];
//    char buf[256];

// check valid tpage

    if (tpage > TEX_NTPages)
        return FALSE;

//$REMOVED
//// return if textures not enabled
//
//    if (!TexturesEnabled)
//        return FALSE;
//$END_REMOVAL

//$REMOVED
//// return if no texture format
//
//    if (!TexFormatProcedural.dwFlags)
//        return FALSE;
//$END_REMOVAL

// free texture slot if already used

    if (TexInfo[tpage].Active)
        FreeOneTexture(tpage);

#ifndef XBOX_NOT_YET_IMPLEMENTED
// create texture source surface

    ZeroMemory(&ddsd2, sizeof(ddsd2));
    ddsd2.dwSize = sizeof(ddsd2);
    ddsd2.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
    ddsd2.ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_SYSTEMMEMORY;
    ddsd2.ddsCaps.dwCaps2 = DDSCAPS2_HINTDYNAMIC;
    ddsd2.dwWidth = width;
    ddsd2.dwHeight = height;
    ddsd2.ddpfPixelFormat = TexFormatProcedural;

    r = DD->CreateSurface(&ddsd2, &TexInfo[tpage].SourceSurface, NULL);
    if (r != DD_OK)
    {
        sprintf(buf, "Can't create procedural source surface!");
        ErrorDX(r, buf);
        return FALSE;
    }

// create texture dest surface

    ZeroMemory(&ddsd2, sizeof(ddsd2));
    ddsd2.dwSize = sizeof(ddsd2);
    ddsd2.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
    if (!Software) ddsd2.ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_VIDEOMEMORY | DDSCAPS_LOCALVIDMEM;
    else ddsd2.ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_SYSTEMMEMORY;
    ddsd2.ddsCaps.dwCaps2 = DDSCAPS2_OPAQUE;
    ddsd2.dwWidth = width;
    ddsd2.dwHeight = height;
    ddsd2.ddpfPixelFormat = TexFormatProcedural;

    r = DD->CreateSurface(&ddsd2, &TexInfo[tpage].Surface, NULL);
    if (r != DD_OK)
    {
        sprintf(buf, "Can't create procedural dest surface!");
        ErrorDX(r, buf);
        return FALSE;
    }

//$REMOVED
//// create a palette if 8 bit textures
//
//    if (TexFormatProcedural.dwRGBBitCount == 8)
//    {
//        for (i = 0 ; i < 255 ; i++)
//        {
//            pal[i] = i | i << 8 | i << 16 | 0xff000000;
//        }
//
//        r = DD->CreatePalette(DDPCAPS_8BIT, (PALETTEENTRY*)pal, &palette, NULL);
//        if (r != DD_OK)
//        {
//            ErrorDX(r, "Can't create procedural texture palette");
//            return FALSE;
//        }
//
//        r = TexInfo[tpage].SourceSurface->SetPalette(palette);
//        if (r != DD_OK)
//        {
//            ErrorDX(r, "Can't attach palette to procedural texture source surface");
//            return FALSE;
//        }
//
//        r = TexInfo[tpage].Surface->SetPalette(palette);
//        if (r != DD_OK)
//        {
//            ErrorDX(r, "Can't attach palette to procedural texture dest surface");
//            return FALSE;
//        }
//
//        RELEASE(palette);
//    }
//$END_REMOVAL

//$REMOVED
//// query interface for surfaces
//
//    r = TexInfo[tpage].Surface->QueryInterface(IID_IDirect3DTexture2, (void**)&TexInfo[tpage].Texture);
//    if (r != DD_OK)
//    {
//        ErrorDX(r, "Can't query interface for procedural texture dest surface");
//        return FALSE;
//    }
//
//    r = TexInfo[tpage].SourceSurface->QueryInterface(IID_IDirect3DTexture2, (void**)&TexInfo[tpage].SourceTexture);
//    if (r != DD_OK)
//    {
//        ErrorDX(r, "Can't query interface for procedural texture source surface");
//        return FALSE;
//    }
//$END_REMOVAL

// set info flags

    TexInfo[tpage].Active = TRUE;
    TexInfo[tpage].Width = width;
    TexInfo[tpage].Height = height;
    TexInfo[tpage].MipCount = -1;
#endif // XBOX_NOT_YET_IMPLEMENTED

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

        TexInfo[i].Texture = NULL;
//$REMOVED        TexInfo[i].Surface = NULL;

        TexInfo[i].SourceTexture = NULL;
//$REMOVED        TexInfo[i].SourceSurface = NULL;
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

void FreeOneTexture(int tpage)
{

// used texture?

    if (!TexInfo[tpage].Active)
        return;

// free

    TexInfo[tpage].Active = FALSE;  //$MODIFIED(cprince): originally set value to NULL

    RELEASE(TexInfo[tpage].Texture);
//$REMOVED    RELEASE(TexInfo[tp].Surface);

    RELEASE(TexInfo[tpage].SourceTexture);
//$REMOVED    RELEASE(TexInfo[tp].SourceSurface);
}

