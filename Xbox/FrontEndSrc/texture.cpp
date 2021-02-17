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
    TexFormat = D3DFMT_A8R8G8B8;
}

//////////////////////////////////////////
// decide texture sets for all textures //
//////////////////////////////////////////

void PickTextureSets(long playernum, long worldnum, long scalenum, long fixednum)
{
    //$HACK: use highest-quality texture sets everywhere
    WorldTextureSet = 0;
    CarTextureSet   = 0;
    FxTextureSet    = 0;
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

bool LoadMipTexture(char *tex, char tpage, long width, long height, long stage, long mipcount, long second)
{
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
    wsprintf(TexInfo[tpage].File, "%s", tex); //$NOTE(jedl) - this is here to allow easier export

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
        return E_FAIL;
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

bool CreateProceduralTexture(char tpage, long width, long height)
{
#pragma message( "Still need to port CreateProceduralTexture.  I nominamte Jed. :-b" )
#ifndef XBOX_NOT_YET_IMPLEMENTED
    long i;
    DDSURFACEDESC2 ddsd2;
    HRESULT r;
    IDirectDrawPalette *palette;
    DWORD pal[256];
    char buff[256];

// check valid tpage

    if (tpage > TEX_NTPages)
        return FALSE;

// return if textures not enabled

    if (!TexturesEnabled)
        return FALSE;

// return if no texture format

    if (!TexFormatProcedural.dwFlags)
        return FALSE;

// free texture slot if already used

    if (TexInfo[tpage].Active)
        FreeOneTexture(tpage);

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
        wsprintf(buff, "Can't create procedural source surface!");
        ErrorDX(r, buff);
        return FALSE;
    }

// create texture dest surface

    ZeroMemory(&ddsd2, sizeof(ddsd2));
    ddsd2.dwSize = sizeof(ddsd2);
    ddsd2.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
    ddsd2.ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_VIDEOMEMORY | DDSCAPS_LOCALVIDMEM;
    else ddsd2.ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_SYSTEMMEMORY;
    ddsd2.ddsCaps.dwCaps2 = DDSCAPS2_OPAQUE;
    ddsd2.dwWidth = width;
    ddsd2.dwHeight = height;
    ddsd2.ddpfPixelFormat = TexFormatProcedural;

    r = DD->CreateSurface(&ddsd2, &TexInfo[tpage].Surface, NULL);
    if (r != DD_OK)
    {
        wsprintf(buff, "Can't create procedural dest surface!");
        ErrorDX(r, buff);
        return FALSE;
    }

// create a palette if 8 bit textures

    if (TexFormatProcedural.dwRGBBitCount == 8)
    {
        for (i = 0 ; i < 255 ; i++)
        {
            pal[i] = i | i << 8 | i << 16 | 0xff000000;
        }

        r = DD->CreatePalette(DDPCAPS_8BIT, (PALETTEENTRY*)pal, &palette, NULL);
        if (r != DD_OK)
        {
            ErrorDX(r, "Can't create procedural texture palette");
            return FALSE;
        }

        r = TexInfo[tpage].SourceSurface->SetPalette(palette);
        if (r != DD_OK)
        {
            ErrorDX(r, "Can't attach palette to procedural texture source surface");
            return FALSE;
        }

        r = TexInfo[tpage].Surface->SetPalette(palette);
        if (r != DD_OK)
        {
            ErrorDX(r, "Can't attach palette to procedural texture dest surface");
            return FALSE;
        }

        RELEASE(palette);
    }

// query interface for surfaces

    r = TexInfo[tpage].Surface->QueryInterface(IID_IDirect3DTexture2, (void**)&TexInfo[tpage].Texture);
    if (r != DD_OK)
    {
        ErrorDX(r, "Can't query interface for procedural texture dest surface");
        return FALSE;
    }

    r = TexInfo[tpage].SourceSurface->QueryInterface(IID_IDirect3DTexture2, (void**)&TexInfo[tpage].SourceTexture);
    if (r != DD_OK)
    {
        ErrorDX(r, "Can't query interface for procedural texture source surface");
        return FALSE;
    }

// set info flags

    TexInfo[tpage].Active = TRUE;
    TexInfo[tpage].Width = width;
    TexInfo[tpage].Height = height;
    TexInfo[tpage].MipCount = -1;

// return OK

    return TRUE;
#else
    return FALSE;
#endif // XBOX_NOT_YET_IMPLEMENTED
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
        TexInfo[i].SourceSurface = NULL;
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

    TexInfo[tp].Active = FALSE;  //$MODIFIED(cprince): originally set value to NULL

    RELEASE(TexInfo[tp].Texture);
//$REMOVED    RELEASE(TexInfo[tp].Surface);

    RELEASE(TexInfo[tp].SourceTexture);
    RELEASE(TexInfo[tp].SourceSurface);
}

