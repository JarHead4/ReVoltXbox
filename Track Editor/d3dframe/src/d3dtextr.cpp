//-----------------------------------------------------------------------------
// File: D3DTextr.cpp
//
// Desc: This file contains the member functions for the 
//       CD3DTextureEngine class. The class is responsible for loading and
//       creating textures from files, as well as retrieving the
//       corresponding ptrs and surfaces for the textures, given a 
//       texture's name.
//
//
// Copyright (C) 1997 Microsoft Corporation. All rights reserved
//-----------------------------------------------------------------------------

#define STRICT
#include <tchar.h>
#include "D3DTextr.h"
#include "D3DUtil.h"




//-----------------------------------------------------------------------------
// Name: TextureContainer
// Desc: Linked list tructure to hold info per texture
//-----------------------------------------------------------------------------
struct TextureContainer
{
    HBITMAP              hbmBitmap;      // Bitmap containing texture image
    LPDIRECT3DDEVICE3    pd3dDevice;     // Device used to create the texture
    LPDIRECTDRAWSURFACE4 pddsSurface;    // Surface of the texture
    LPDIRECT3DTEXTURE2   ptexTexture;    // Direct3D texture for the texture
    DWORD                dwStage;        // Texture stage (for multi-textures)
    TCHAR                strName[80];    // Filename of assoc'd bitmap
    TextureContainer     *pPrev, *pNext; // Ptr to next texture in linked list

    ~TextureContainer();
};




//-----------------------------------------------------------------------------
// Macros, function prototypes and static variable
//-----------------------------------------------------------------------------
#define FOREACHTEXTURE(ptc) for( TextureContainer* ptc=g_ptcTextureList; \
                            NULL!=ptc; ptc=ptc->pNext )

HRESULT LoadPPMFile( TCHAR* strFilename, HBITMAP* phbm );
HRESULT LoadBMPFile( TCHAR* strFilename, HBITMAP* phbm ); 
HRESULT LoadPPMFileResource( BYTE* pData, HBITMAP* phbm );
HRESULT FindTextureFile( TCHAR* strFilename, TCHAR* strTexturePath, 
                         TCHAR* strFullPath );

static TextureContainer* g_ptcTextureList = NULL;     // Textures list

static TCHAR g_strTexturePath[512] = TEXT("MEDIA\\"); // Path for files




//-----------------------------------------------------------------------------
// Name: TextureSearchCallback()
// Desc: Enumeration callback routine to find a best-matching texture format. 
//       The param data is the DDPIXELFORMAT of the best-so-far matching
//       texture. Note: the desired BPP is passed in the dwSize field, and the
//       default BPP is passed in the dwFlags field.
//-----------------------------------------------------------------------------
HRESULT CALLBACK TextureSearchCallback( DDPIXELFORMAT* pddpfThis, VOID* param )
{
    if( NULL==pddpfThis || NULL==param )
        return DDENUMRET_OK;

    DDPIXELFORMAT* pddpfBest = &( ((DDSURFACEDESC2*)param)->ddpfPixelFormat );

    DWORD dwBestBPP    = pddpfBest->dwRGBBitCount;
    DWORD dwDesiredBPP = pddpfBest->dwSize;
    DWORD dwDefaultBPP = pddpfBest->dwFlags;
    DWORD dwThisBPP    = pddpfThis->dwRGBBitCount;
    DWORD dwFlags      = pddpfThis->dwFlags;

    // Skip any funky modes
    if( dwFlags & (DDPF_LUMINANCE|DDPF_ALPHAPIXELS) )
        return DDENUMRET_OK;
    if( dwFlags & (DDPF_BUMPLUMINANCE|DDPF_BUMPDUDV) )
        return DDENUMRET_OK;
    if( pddpfThis->dwFourCC != 0 )
        return DDENUMRET_OK;

    // Under 16 bpp, only accept 4- and 8-bit palettized modes
    if( dwThisBPP <= 8 )
        if( !( dwFlags & ( DDPF_PALETTEINDEXED4 | DDPF_PALETTEINDEXED8 ) ) )
            return DDENUMRET_OK;
        
    // Check if we found a good match
    if( dwDesiredBPP == dwThisBPP )
    {
        memcpy( pddpfBest, pddpfThis, sizeof(DDPIXELFORMAT) );
        return DDENUMRET_CANCEL;
    }

    // For less than 4-bit palette modes, we'll accept 4-bit palette modes
    if( ( dwThisBPP == 4 ) && ( dwDesiredBPP < 4 ) )
    {
        memcpy( pddpfBest, pddpfThis, sizeof(DDPIXELFORMAT) );
        ((DDSURFACEDESC2*)param)->ddpfPixelFormat.dwSize = dwDesiredBPP;
    }

    // For less than 8-bit palette modes, we'll accept 8-bit palette modes
    if( ( dwThisBPP == 8 ) && ( dwDesiredBPP < 8 )  )
    {
        memcpy( pddpfBest, pddpfThis, sizeof(DDPIXELFORMAT) );
        ((DDSURFACEDESC2*)param)->ddpfPixelFormat.dwSize = dwDesiredBPP;
    }

    if( ( dwThisBPP == dwDefaultBPP ) )
    {
        memcpy( pddpfBest, pddpfThis, sizeof(DDPIXELFORMAT) );
        ((DDSURFACEDESC2*)param)->ddpfPixelFormat.dwSize = dwDesiredBPP;
    }

    return DDENUMRET_OK;
}




//-----------------------------------------------------------------------------
// Name: ~TextureContainer()
// Desc: Destructs the contents of the texture container
//-----------------------------------------------------------------------------
TextureContainer::~TextureContainer()
{
    SAFE_DELETE(  pNext );
    SAFE_RELEASE( ptexTexture );
    SAFE_RELEASE( pddsSurface );
    DeleteObject( hbmBitmap );
}




//-----------------------------------------------------------------------------
// Name: CD3DStaticTextureEngine
// Desc: Class used to automatically construct and destruct the static
//       texture engine class.
//-----------------------------------------------------------------------------
class CD3DTextureEngine
{
public:
    CD3DTextureEngine();
    ~CD3DTextureEngine();
} g_StaticTextureEngine;




//-----------------------------------------------------------------------------
// Name: CD3DTextureEngine()
// Desc: Constructs the texture engine. Registers some basic file formats
//       for texture loading support.
//-----------------------------------------------------------------------------
CD3DTextureEngine::CD3DTextureEngine()
{
}

    


//-----------------------------------------------------------------------------
// Name: ~CD3DTextureEngine()
// Desc: Deletes the internal list of textures
//-----------------------------------------------------------------------------
CD3DTextureEngine::~CD3DTextureEngine()
{
    // Delete the list of textures
    SAFE_DELETE( g_ptcTextureList );
}       




//-----------------------------------------------------------------------------
// Name: FindTexture()
// Desc: Searches the internal list of textures for a texture specified by
//       its name. Returns the structure associated with that texture.
//-----------------------------------------------------------------------------
TextureContainer* FindTexture( TCHAR* strTextureName )
{
    FOREACHTEXTURE( ptcTexture )
    {
        if( !lstrcmpi( strTextureName, ptcTexture->strName ) )
            return ptcTexture;
    }

    return NULL;
}




//-----------------------------------------------------------------------------
// Name: LoadTextureImage()
// Desc: Loads a texture map file into a BITMAP surface.
//-----------------------------------------------------------------------------
HRESULT LoadTextureImage( TCHAR* strFilename, HBITMAP* phbm )
{
    TCHAR*   strExtension;
    TCHAR    strPathname[256];

    // Check parameters
    if( NULL == strFilename || NULL == phbm )
        return DDERR_INVALIDPARAMS;

    // Get the filename extension
    if( NULL == ( strExtension = _tcsrchr( strFilename, TEXT('.') ) ) )
        return DDERR_UNSUPPORTED;

    // Check the executable's resource. If it's there, we're done!
    if( NULL != ( (*phbm) = (HBITMAP)LoadImage( GetModuleHandle(NULL), 
                                            strFilename, IMAGE_BITMAP, 
                                            0, 0, LR_CREATEDIBSECTION) ) )
        return DD_OK;

    // Check the resource for non-bmp textures
	HRSRC hrsrc = FindResource( NULL, strFilename, "Textures" );
	if( hrsrc )
	{
		HGLOBAL hg = LoadResource( NULL, hrsrc );
		if( hg )
		{
			HRESULT hr = LoadPPMFileResource( (BYTE*)hg, phbm );

			FreeResource(hg);

			return hr;
		}
	}

    // Check the current path and system registry path for the file
    if( FAILED( FindTextureFile( strFilename, g_strTexturePath, strPathname ) ) )
        return DDERR_NOTFOUND;

    if( !lstrcmpi( strExtension, ".bmp" ) )
        return LoadBMPFile( strPathname, phbm );

    if( !lstrcmpi( strExtension, ".ppm" ) )
        return LoadPPMFile( strPathname, phbm );


    return DDERR_UNSUPPORTED;
}




//-----------------------------------------------------------------------------
// Name: RefreshTextureImage() 
// Desc: Loads the bitmap image into the texture. This needs to be done
//       anytime the texture is re-created or the surface needs to be
//       restored.
//-----------------------------------------------------------------------------
HRESULT RefreshTextureImage( TextureContainer* ptcTexture )
{
    LPDIRECT3DTEXTURE2   ptexTexture;
    HRESULT              hr;
    LPDIRECTDRAWPALETTE  pPalette;
    BITMAP               bm; 
    LPDIRECTDRAW4        pDD;
    LPDIRECT3D3          pD3D;
    LPDIRECTDRAWSURFACE4 pddsSurface;
    DDSURFACEDESC2       ddsd;
    D3DUtil_InitSurfaceDesc( ddsd );

    // Check params
    if( NULL == ptcTexture )
        return DDERR_INVALIDPARAMS;


    LPDIRECT3DDEVICE3 pd3dDevice = ptcTexture->pd3dDevice;

    // Get the DD and D3D interfaces needed for creating textures
	if( NULL == ( pDD = D3DUtil_GetDirectDrawFromDevice( pd3dDevice ) ) )
		return E_FAIL;
    pDD->Release();

    if( FAILED( pd3dDevice->GetDirect3D( &pD3D ) ) )
        return E_FAIL;
    pD3D->Release();

    // Setup the new surface desc
    ptcTexture->pddsSurface->GetSurfaceDesc( &ddsd );
    ddsd.ddsCaps.dwCaps  = DDSCAPS_TEXTURE|DDSCAPS_SYSTEMMEMORY;

    // Next two lines are a hack, until DX6 GetSurfaceDesc() works properly
    ddsd.dwFlags        |= DDSD_TEXTURESTAGE;
    ddsd.dwTextureStage  = ptcTexture->dwStage;

    // Create a DirectDrawSurface for this bitmap. 
    if( FAILED( pDD->CreateSurface( &ddsd, &pddsSurface, NULL ) ) )
    {
        // We may have failed due to DX6's erroneous requirement that
        // the DDSD_LPSURFACE flag is not set when the DDSD_PITCH flag
        // is. We'll strip the latter flag, and retry
        ddsd.dwFlags &= (~DDSD_PITCH);
        if( FAILED( pDD->CreateSurface( &ddsd, &pddsSurface, NULL ) ) )
            return DDERR_GENERIC; 
    }

    // For 8-bit surfaces, the GetDC() call below requires a palette
    if( ddsd.ddpfPixelFormat.dwRGBBitCount == 8 )
    {
        if( SUCCEEDED( ptcTexture->pddsSurface->GetPalette( &pPalette ) ) )
        {
            pddsSurface->SetPalette( pPalette );
            pPalette->Release();
        }
    }

    // Get the dimensions of the bitmap. 
    GetObject( ptcTexture->hbmBitmap, sizeof(BITMAP), &bm ); 

    // Copy the bitmap to the surface
    HDC hdcImage = CreateCompatibleDC( NULL );
    HDC hdc;

    if( hdcImage )
    {
        SelectObject( hdcImage, ptcTexture->hbmBitmap );

        // Copy the bitmap image to the surface.
        // Note: 4-bit surfaces don't support BitBlt. Do things manually
        if( ddsd.ddpfPixelFormat.dwRGBBitCount < 8 )
        {
            DDSURFACEDESC2 ddsd;
            D3DUtil_InitSurfaceDesc( ddsd );
            if( SUCCEEDED( pddsSurface->Lock( NULL, &ddsd, 
                                    DDLOCK_SURFACEMEMORYPTR, NULL ) ) )
            {
                BYTE* pSrcPixels  = (BYTE*)bm.bmBits;
                BYTE* pDestPixels = (BYTE*)ddsd.lpSurface;
                int   iWidth      = (bm.bmWidth+1)/2; // 4-bit pixels + 1
                int   iHeight     = bm.bmHeight;
                
                if( bm.bmHeight > 0 )
                {
                    for( int y=0; y<iHeight; y++ )
                        for( int x=0; x<iWidth; x++ )
						{
							BYTE p = *pSrcPixels++;
							BYTE d = (BYTE)(((p&0xf0L)>>4L) + ((p&0x0fL)<<4L));
                            pDestPixels[((iHeight-y-1)*iWidth)+x] = d;
						}
                }
                else
                {
                    for( int y=0; y<(-iHeight); y++ )
                        for( int x=0; x<iWidth; x++ )
						{
							BYTE p = *pSrcPixels++;
							BYTE d = (BYTE)(((p&0xf0L)>>4L) + ((p&0x0fL)<<4L));
                            *pDestPixels++ = d;
						}
                }

                pddsSurface->Unlock( NULL );
            }

        }

        // Note: this call will fail for 8-bit surfaces without palettes!
        else if( SUCCEEDED( pddsSurface->GetDC( &hdc ) ) )
        {
            BitBlt( hdc, 0, 0, bm.bmWidth, bm.bmHeight, hdcImage, 0, 0, 
                    SRCCOPY );
            pddsSurface->ReleaseDC( hdc );
        }
    }
    DeleteDC( hdcImage );

    // Create the destination texture and load the local texture into it
    if( SUCCEEDED( hr = pddsSurface->QueryInterface( IID_IDirect3DTexture2,
                                                     (VOID**)&ptexTexture ) ) )
    {
        hr = ptcTexture->ptexTexture->Load( ptexTexture );
    }

    ptexTexture->Release();
    pddsSurface->Release();

    return hr;
}




//-----------------------------------------------------------------------------
// Name: BuildTextureObjects() 
// Desc: Creates the surface and texture objects needed for the texture.
//       Then, it calls RefreshTextureImage() to load the bitmap image.
//-----------------------------------------------------------------------------
HRESULT BuildTextureObjects( TextureContainer* ptcTexture,
                             LPDIRECT3DDEVICE3 pd3dDevice, DWORD dwBitDepth )
{
    LPDIRECTDRAW4  pDD;
    DWORD          dwMemType;
    LPDIRECT3D3    pD3D;
    BITMAP         bm; 
    DDSURFACEDESC2 ddsd;
    DDSURFACEDESC2 ddsdFind;

    // Check params
    if( NULL == ptcTexture || NULL == pd3dDevice )
        return DDERR_INVALIDPARAMS;

    // Keep a reference to the device for later usage
    ptcTexture->pd3dDevice = pd3dDevice;

    // Get the DD and D3D interfaces needed for creating textures
	if( NULL == ( pDD = D3DUtil_GetDirectDrawFromDevice( pd3dDevice ) ) )
		return E_FAIL;
    pDD->Release();

	dwMemType =D3DUtil_GetDeviceMemoryType( pd3dDevice );

    if( FAILED( pd3dDevice->GetDirect3D( &pD3D ) ) )
        return E_FAIL;
    pD3D->Release();

    // Get the dimensions of the bitmap. 
    GetObject( ptcTexture->hbmBitmap, sizeof(BITMAP), &bm ); 

    // Setup the new surface desc
    D3DUtil_InitSurfaceDesc( ddsd );
    ddsd.dwFlags        = DDSD_CAPS|DDSD_HEIGHT|DDSD_WIDTH|DDSD_PIXELFORMAT|
                          DDSD_TEXTURESTAGE;
    ddsd.ddsCaps.dwCaps = DDSCAPS_TEXTURE|dwMemType;
    ddsd.dwWidth        = bm.bmWidth; 
    ddsd.dwHeight       = bm.bmHeight; 
    ddsd.dwTextureStage = ptcTexture->dwStage;

    // Enumerate the texture formats, and find a best fit. The dwDesired
    // field is used to store the desired bpp
    ddsdFind.ddpfPixelFormat.dwSize        = bm.bmBitsPixel; // desired bpp
    ddsdFind.ddpfPixelFormat.dwFlags       = 16;             // default bpp
    ddsdFind.ddpfPixelFormat.dwRGBBitCount = 256;            // current bpp

    if( dwBitDepth == DDBD_32 )
        ddsdFind.ddpfPixelFormat.dwFlags = 32; // default bpp is 32

    if( FAILED( pd3dDevice->EnumTextureFormats( TextureSearchCallback,
                                                &ddsdFind ) ) )
        return DDERR_GENERIC;

    memcpy( &ddsd.ddpfPixelFormat, &ddsdFind.ddpfPixelFormat,
            sizeof(DDPIXELFORMAT) );

    // Create a new surface for the texture
    if( FAILED( pDD->CreateSurface( &ddsd, &ptcTexture->pddsSurface,
                                    NULL ) ) )
        return DDERR_GENERIC;

    // If palettized, set the palette for the surface
    if( ddsd.ddpfPixelFormat.dwRGBBitCount <= 8 )
    {
        // Create a DC and setup the bitmap
        HDC hdcImage = CreateCompatibleDC( NULL );
        if( NULL == hdcImage )
            return DDERR_GENERIC;

        GetObject( ptcTexture->hbmBitmap, sizeof(BITMAP), &bm ); 
        SelectObject( hdcImage, ptcTexture->hbmBitmap );

        // Create the color table and parse in the palette
        DWORD pe[256];
        WORD  wNumColors = GetDIBColorTable( hdcImage, 0, 256, 
                                             (RGBQUAD*)pe );
        for( WORD i=0; i<wNumColors; i++ )
            pe[i] = RGB( GetBValue(pe[i]), GetGValue(pe[i]), 
                         GetRValue(pe[i]) );

        // Create a new 4- or 8-bit palette with the bitmap's colors
        LPDIRECTDRAWPALETTE pPalette;
        if( ddsd.ddpfPixelFormat.dwRGBBitCount == 8 )
            pDD->CreatePalette( DDPCAPS_8BIT,
                                (PALETTEENTRY*)pe, &pPalette, NULL );
        else
            pDD->CreatePalette( DDPCAPS_4BIT,
                                (PALETTEENTRY*)pe, &pPalette, NULL );
        
        // Attach the palette to the surface
        ptcTexture->pddsSurface->SetPalette( pPalette );
        SAFE_RELEASE( pPalette );

        DeleteDC( hdcImage );
    }

    // Create the texture
    if( FAILED( ptcTexture->pddsSurface->QueryInterface( IID_IDirect3DTexture2,
                                         (VOID**)&ptcTexture->ptexTexture ) ) )
        return DDERR_GENERIC;

    return RefreshTextureImage( ptcTexture );
}

  


//-----------------------------------------------------------------------------
// Name: D3DTextr_GetSurface()
// Desc: Returns a pointer to a d3dSurface from the name of the texture
//-----------------------------------------------------------------------------
LPDIRECTDRAWSURFACE4 D3DTextr_GetSurface( TCHAR* strName )
{
    TextureContainer* ptcTexture = FindTexture( strName );

    return ptcTexture ? ptcTexture->pddsSurface : NULL;
}




//-----------------------------------------------------------------------------
// Name: D3DTextr_GetTexture()
// Desc: Returns a pointer to a d3dTexture from the name of the texture
//-----------------------------------------------------------------------------
LPDIRECT3DTEXTURE2 D3DTextr_GetTexture( TCHAR* strName )
{
    TextureContainer* ptcTexture = FindTexture( strName );

    return ptcTexture ? ptcTexture->ptexTexture: NULL;
}




//-----------------------------------------------------------------------------
// Name: D3DTextr_SetTexturePath()
// Desc: Enumeration callback routine to find a best-matching texture format. 
//-----------------------------------------------------------------------------
VOID D3DTextr_SetTexturePath( TCHAR* strTexturePath )
{
    if( NULL==strTexturePath )
        strTexturePath = TEXT("");
    _tcscpy( g_strTexturePath, strTexturePath );
}




//-----------------------------------------------------------------------------
// Name: D3DTextr_CreateTexture() 
// Desc: Is passed a filename and creates a local Bitmap from that file.
//       The texture can not be used until it is restored, however.
//-----------------------------------------------------------------------------
HRESULT D3DTextr_CreateTexture( TCHAR* strName, DWORD dwStage ) 
{
    // Check first to see if the texture is already loaded
    if( NULL != FindTexture( strName ) )
        return DD_OK;

    // Create a bitmap and load the texture file into it,
    HBITMAP hbm;
    if( FAILED( LoadTextureImage( strName, &hbm ) ) )
        return DDERR_GENERIC;

    // Allocate and add the texture to the linked list of textures;
    TextureContainer* ptcTexture = new TextureContainer();
    if( NULL == ptcTexture )
    {
        DeleteObject( hbm );
        return DDERR_OUTOFMEMORY;
    }

    ZeroMemory( ptcTexture, sizeof(TextureContainer) );
    lstrcpy( ptcTexture->strName, strName );
    ptcTexture->hbmBitmap = hbm;
    ptcTexture->dwStage   = dwStage;

    // Add the texture to the global linked list
    if( g_ptcTextureList )
        g_ptcTextureList->pPrev = ptcTexture;
    ptcTexture->pNext = g_ptcTextureList;
    g_ptcTextureList  = ptcTexture;

    return DD_OK;
}




//-----------------------------------------------------------------------------
// Name: D3DTextr_Restore() 
// Desc: Invalidates the current texture objects and rebuilds new ones
//       using the new device.
//-----------------------------------------------------------------------------
HRESULT D3DTextr_Restore( TCHAR* strName, LPDIRECT3DDEVICE3 pd3dDevice,
                          DWORD dwBitDepth )
{
    TextureContainer* ptcTexture = FindTexture( strName );
    if( NULL == ptcTexture )
        return DDERR_NOTFOUND;

    D3DTextr_Invalidate( strName );
    return BuildTextureObjects( ptcTexture, pd3dDevice, dwBitDepth );
}




//-----------------------------------------------------------------------------
// Name: D3DTextr_RestoreAllTextures() 
// Desc: This function is called when a mode is changed. It updates all
//       texture objects to be valid with the new device.
//-----------------------------------------------------------------------------
HRESULT D3DTextr_RestoreAllTextures( LPDIRECT3DDEVICE3 pd3dDevice,
                                     DWORD dwBitDepth )
{
    FOREACHTEXTURE( ptcTexture )
    {
        D3DTextr_Restore( ptcTexture->strName, pd3dDevice, dwBitDepth );
    }

    return DD_OK;
}




//-----------------------------------------------------------------------------
// Name: D3DTextr_RestoreAllTextureSurfaces() 
// Desc: Restores the surfaces for all textures, in the event that 
//       DirectDraw reports a DDERR_SURFACELOST error.
//-----------------------------------------------------------------------------
HRESULT D3DTextr_RestoreAllTextureSurfaces()
{
    FOREACHTEXTURE( ptcTexture )
    {
        if( ptcTexture->pddsSurface )
        {
            ptcTexture->pddsSurface->Restore();
            RefreshTextureImage( ptcTexture );
        }
    }

    return DD_OK;
}



    
//-----------------------------------------------------------------------------
// Name: D3DTextr_Invalidate()
// Desc: Used to bump a texture out of (video) memory, this function
//       actually destroys the d3dtexture and ddsurface of the texture
//-----------------------------------------------------------------------------
HRESULT D3DTextr_Invalidate( TCHAR* strName )
{
    TextureContainer* ptcTexture = FindTexture( strName );
    if( NULL == ptcTexture )
        return DDERR_NOTFOUND;
    
    SAFE_RELEASE( ptcTexture->ptexTexture );
    SAFE_RELEASE( ptcTexture->pddsSurface );

    return DD_OK;
}




//-----------------------------------------------------------------------------
// Name: D3DTextr_InvalidateAllTextures() 
// Desc: This function is called when a mode is changed. It invalidates
//       all texture objects so their device can be safely released.
//-----------------------------------------------------------------------------
HRESULT D3DTextr_InvalidateAllTextures()
{
    FOREACHTEXTURE( ptcTexture )
    {
        SAFE_RELEASE( ptcTexture->ptexTexture );
        SAFE_RELEASE( ptcTexture->pddsSurface );
    }

    return DD_OK;
}



    
//-----------------------------------------------------------------------------
// Name: D3DTextr_DestroyTexture()
// Desc: Frees the resources for the specified texture container
//-----------------------------------------------------------------------------
HRESULT D3DTextr_DestroyTexture( TCHAR* strName )
{
    TextureContainer* ptcTexture = FindTexture( strName );
    if( NULL == ptcTexture )
        return DDERR_NOTFOUND;

    // Remove the texture container from the global list
    if( ptcTexture->pPrev )
        ptcTexture->pPrev->pNext = ptcTexture->pNext;
    else
        g_ptcTextureList = ptcTexture->pNext;
    ptcTexture->pNext = NULL;

    SAFE_DELETE( ptcTexture );

    return DD_OK;
}




