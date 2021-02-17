//-----------------------------------------------------------------------------
// File: D3DTextr.h
//
// Desc: Functions to manage textures, including creating (loading from a
//       file), restoring lost surfaces, invalidating, and destroying.
//
//       Note: the implementation of these fucntions maintain an internal list
//       of loaded textures. After creation, individual textures are referenced
//       via their ASCII names.       
//
//
// Copyright (C) 1997 Microsoft Corporation. All rights reserved
//-----------------------------------------------------------------------------

#ifndef D3DTEXTR_H
#define D3DTEXTR_H

#include <ddraw.h>
#include <d3d.h>




//-----------------------------------------------------------------------------
// Access functions for loaded textures. Note: these functions search
// an internal list of the textures, and use the texture associated with the
// ASCII name.
//-----------------------------------------------------------------------------
LPDIRECTDRAWSURFACE4 D3DTextr_GetSurface( TCHAR* strName );
LPDIRECT3DTEXTURE2   D3DTextr_GetTexture( TCHAR* strName );




//-----------------------------------------------------------------------------
// Texture invalidation and restoration functions
//-----------------------------------------------------------------------------
HRESULT D3DTextr_Invalidate( TCHAR* strName );
HRESULT D3DTextr_Restore( TCHAR* strName, LPDIRECT3DDEVICE3 pd3dDevice, 
                          DWORD dwBitDepth = 0L );
HRESULT D3DTextr_InvalidateAllTextures();
HRESULT D3DTextr_RestoreAllTextures( LPDIRECT3DDEVICE3 pd3dDevice,
                                     DWORD dwBitDepth = 0L );
HRESULT D3DTextr_RestoreAllTextureSurfaces();




//-----------------------------------------------------------------------------
// Texture creation and deletion functions
//-----------------------------------------------------------------------------
VOID    D3DTextr_SetTexturePath( TCHAR* strTexturePath );
HRESULT D3DTextr_DestroyTexture( TCHAR* strName );
HRESULT D3DTextr_CreateTexture( TCHAR* strName, DWORD dwStage=0L );




#endif // D3DTEXTR_H
