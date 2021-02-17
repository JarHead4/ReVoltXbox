//-----------------------------------------------------------------------------
// File: texture.h
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef TEXTURE_H
#define TEXTURE_H

#include "competition.h"

#include "XBResource.h" //$ADDITION(jedl) - graphics resources

// macros

#define MAX_MIPMAPS 2  //$REVISIT(cprince): this seems bad; we should fix this!
#define MAX_TPAGE_FILENAME 128
#define MAX_TEXTURE_TEST 1024

#define TPAGE_WORLD_NUM 10
#define TPAGE_SCALE_NUM 6
#define TPAGE_FIXED_NUM 6

enum {
    TPAGE_WORLD_START,
    TPAGE_CAR_START = TPAGE_WORLD_NUM,
    TPAGE_FONT = TPAGE_WORLD_NUM + MAX_RACE_CARS, // fixed
    TPAGE_SPRU, // fixed
    TPAGE_ENVSTILL, // scale
    TPAGE_ENVROLL, // scale
    TPAGE_SHADOW, // scale
    TPAGE_FX1, // fixed
    TPAGE_FX2, // fixed
    TPAGE_FX3, // fixed
    TPAGE_LOADING, // fixed
    TPAGE_MISC1, // scale
    TPAGE_MISC2, // scale
    TPAGE_MISC3, // scale
    TPAGE_MISC4, // scale
    TPAGE_MISC5, // scale
    TPAGE_MISC6, // scale
    TPAGE_MISC7, // scale
    TPAGE_MISC8, // scale

    TPAGE_NUM
};

typedef struct {
    BOOL Active;  //$NOTE(cprince): this should be a bool or BOOL (bActive)
    long Width;
    long Height;
//$REMOVED    long Stage;
//$REMOVED    long MipCount;
    char File[MAX_TPAGE_FILENAME]; //$NOTE(jedl) - this is here to allow easier export

    IDirect3DTexture2 *Texture;
//$REMOVED    IDirectDrawSurface4 *Surface;

    IDirect3DTexture2 *SourceTexture;   //$REVISIT(cprince) - do we really need this guy?
    IDirectDrawSurface4 *SourceSurface; //$REVISIT(cprince) - do we really need this guy?
} TEXINFO;

// prototypes

extern bool CreateTPages(int nPages);
extern void DestroyTPages();
extern void PickTextureFormat(void);  //$RENAMED(cprince): was originally GetTextureFormat
extern void PickTextureSets(long playernum, long worldnum, long scalenum, long fixednum);
extern long MipSize(long size, long set, long count, long mip);
//$REMOVEDextern HRESULT CALLBACK FindTextureCallback(DDPIXELFORMAT *ddpf, void *lParam);
//$REMOVEDextern long CountTexturePixels(long needed, long width, long height);
//$REMOVEDextern long CountMipTexturePixels(long needed, long width, long height);
extern bool LoadTextureClever(char *tex, char tpage, long width, long height, long stage, long set, long mip);
extern bool LoadMipTexture(char *tex, char tpage, long width, long height, long stage, long mipcount, long second);
extern bool CreateProceduralTexture(char tpage, long width, long height);
extern HRESULT LoadTextureGPU(char *strIdentifier, char *tex, int tpage, XBResource *pXBR);
extern void InitTextures(void);
extern void FreeTextures(void);
extern void FreeOneTexture(char tp);  //$REVISIT: should this arg be int or dword instead?

// globals

extern DDPIXELFORMAT TexFormat, TexFormatProcedural;
//$REMOVEDextern DDPIXELFORMAT TexFormat16, TexFormat24;
//$REMOVEDextern char TexturesEnabled, TexturesSquareOnly, TexturesAGP;
extern TEXINFO *TexInfo;
extern int TEX_NTPages;
//$REMOVEDextern long TexturePixels;
extern long WorldTextureSet, CarTextureSet, FxTextureSet;
//$REMOVEDextern DWORD TextureMinWidth, TextureMaxWidth, TextureMinHeight, TextureMaxHeight;

#endif // TEXTURE_H

