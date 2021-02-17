//-----------------------------------------------------------------------------
// File: XbMediaDebug.cpp
//
// Desc: Miscellaneous functions to aid debugging of media/graphics/etc.
//
// Hist: 11.01.01 - New for November 2001 XDK release
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "XbMediaDebug.h"
#include <stdio.h>

// forward declarations
CHAR *DebugFormat(DWORD format);

//-----------------------------------------------------------------------------
// Macros to set new state, then restore old state upon leaving scope.
//-----------------------------------------------------------------------------
#define SET_D3DTSS(STAGE, NAME, VALUE)\
    struct D3DTSS_STAGE##STAGE##_##NAME {\
        DWORD m_dw; \
        D3DTSS_STAGE##STAGE##_##NAME(DWORD dw) { /* ctor saves curr value and sets new value */ \
            D3DDevice_GetTextureStageState(STAGE, NAME, &m_dw);\
            D3DDevice_SetTextureStageState(STAGE, NAME, dw);\
        }\
        ~D3DTSS_STAGE##STAGE##_##NAME() { /* on scope exit, saved value is restored */ \
            D3DDevice_SetTextureStageState(STAGE, NAME, m_dw);\
        }\
    } _D3DTSS_STAGE##STAGE##_##NAME(VALUE)

#define SET_D3DTEXTURE(STAGE, TEXTURE)\
    struct D3DTEXTURE_STAGE##STAGE {\
        D3DBaseTexture *m_pTexture; \
        D3DTEXTURE_STAGE##STAGE(D3DBaseTexture *pTexture) { /* ctor saves curr value and sets new value */ \
            m_pTexture = D3DDevice_GetTexture2(STAGE);\
            D3DDevice_SetTexture(STAGE, pTexture);\
        }\
        ~D3DTEXTURE_STAGE##STAGE() { /* on scope exit, saved value is restored */ \
            D3DDevice_SetTexture(STAGE, m_pTexture);\
            if (m_pTexture != NULL) m_pTexture->Release();\
        }\
    } _D3DTEXTURE_STAGE##STAGE(TEXTURE)

#define SET_D3DRS(NAME, VALUE)\
    struct D3DRS_##NAME {\
        DWORD m_dw; \
        D3DRS_##NAME(DWORD dw) { /* ctor saves curr value and sets new value */ \
            D3DDevice_GetRenderState(NAME, &m_dw);\
            D3DDevice_SetRenderState(NAME, dw);\
        }\
        ~D3DRS_##NAME() { /* on scope exit, saved value is restored */ \
            D3DDevice_SetRenderState(NAME, m_dw);\
        }\
    } _D3DRS_##NAME(VALUE)

#define SET_D3DVERTEXSHADER(VALUE)\
    struct D3DVERTEXSHADER {\
        DWORD m_dw; \
        D3DVERTEXSHADER(DWORD dw) { /* ctor saves curr value and sets new value */ \
            D3DDevice_GetVertexShader(&m_dw);\
            D3DDevice_SetVertexShader(dw);\
        }\
        ~D3DVERTEXSHADER() { /* on scope exit, saved value is restored */ \
            D3DDevice_SetVertexShader(m_dw);\
        }\
    } _D3DVERTEXSHADER(VALUE)

#define SET_D3DPIXELSHADER(VALUE)\
    struct D3DPIXELSHADER {\
        DWORD m_dw; \
        D3DPIXELSHADER(DWORD dw) { /* ctor saves curr value and sets new value */ \
            D3DDevice_GetPixelShader(&m_dw);\
            D3DDevice_SetPixelShader(dw);\
        }\
        ~D3DPIXELSHADER() { /* on scope exit, saved value is restored */ \
            D3DDevice_SetPixelShader(m_dw);\
        }\
    } _D3DPIXELSHADER(VALUE)

#define SET_D3DVIEWPORT(PVIEWPORT)\
    struct D3DVIEWPORT {\
        D3DVIEWPORT8 m_viewport; \
        D3DVIEWPORT(D3DVIEWPORT *pViewport) { /* ctor saves curr value and sets new value */ \
            D3DDevice_GetViewport(&m_viewport);\
            D3DDevice_SetViewport(pViewport);\
        }\
        ~D3DVIEWPORT() { /* on scope exit, saved value is restored */ \
            D3DDevice_SetViewport(&m_viewport);\
        }\
    } _D3DVIEWPORT(PVIEWPORT)

#define SET_D3DTRANSFORM(NAME, PTRANSFORM)\
    struct D3DTRANSFORM_##NAME {\
        D3DMATRIX m_transform; \
        D3DTRANSFORM_##NAME(D3DMATRIX *pTransform) { /* ctor saves curr value and sets new value */ \
            D3DDevice_GetTransform(NAME, &m_transform);\
            D3DDevice_SetTransform(NAME, pTransform);\
        }\
        ~D3DTRANSFORM_##NAME() { /* on scope exit, saved value is restored */ \
            D3DDevice_SetTransform(NAME, &m_transform);\
        }\
    } _D3DTRANSFORM_##NAME(PTRANSFORM)


#define SETUP_RENDER_TARGET() \
    struct DebugSetupRenderTarget \
    { \
        IDirect3DSurface8* m_pPrevColorBuffer; \
        IDirect3DSurface8* m_pPrevDepthBuffer; \
      \
        DebugSetupRenderTarget() \
        { \
            /* Save info about the current render target */ \
            m_pPrevColorBuffer = D3DDevice_GetRenderTarget2(); \
            m_pPrevDepthBuffer = D3DDevice_GetDepthStencilSurface2(); \
          \
            /* Make the front buffer the new render target */ \
            IDirect3DSurface8* pFrontBuffer = NULL; \
            pFrontBuffer = D3DDevice_GetBackBuffer2( -1 ); \
            D3DDevice_SetRenderTarget( pFrontBuffer, NULL ); \
            pFrontBuffer->Release(); \
          \
            /* Make sure all the commands so far are done */ \
            D3DDevice_BlockUntilIdle(); \
        } \
      \
        ~DebugSetupRenderTarget() \
        { \
            /* Restore render target, etc. */ \
            D3DDevice_SetRenderTarget( m_pPrevColorBuffer, m_pPrevDepthBuffer ); \
            if( m_pPrevColorBuffer )  { m_pPrevColorBuffer->Release(); } \
            if( m_pPrevDepthBuffer )  { m_pPrevDepthBuffer->Release(); } \
        } \
    } __DebugSetupRenderTarget;



//-----------------------------------------------------------------------------
// Name: DebugPresent
// Desc: Forces all GPU commands to be processed, so that data will appear
//       on-screen.
//-----------------------------------------------------------------------------
HRESULT __cdecl DebugPresent()
{
    D3DDevice_KickPushBuffer();
    D3DDevice_BlockUntilIdle();
    return S_OK;
}



///////////////////////////////////////////////////////////////////////////////
// FORMAT REMAPPING
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
// Name: MapLinearToSwizzledFormat
// Desc: Convert from a linear D3DFORMAT to the equivalent swizzled format.
//-----------------------------------------------------------------------------
D3DFORMAT __cdecl MapLinearToSwizzledFormat( D3DFORMAT fmt )
{
    switch( fmt )
    {
        case D3DFMT_LIN_A1R5G5B5:   return D3DFMT_A1R5G5B5;
        case D3DFMT_LIN_A4R4G4B4:   return D3DFMT_A4R4G4B4;
        case D3DFMT_LIN_A8:         return D3DFMT_A8;
        case D3DFMT_LIN_A8B8G8R8:   return D3DFMT_A8B8G8R8;
        case D3DFMT_LIN_A8R8G8B8:   return D3DFMT_A8R8G8B8;
        case D3DFMT_LIN_B8G8R8A8:   return D3DFMT_B8G8R8A8;
        case D3DFMT_LIN_G8B8:       return D3DFMT_G8B8;
        case D3DFMT_LIN_R4G4B4A4:   return D3DFMT_R4G4B4A4;
        case D3DFMT_LIN_R5G5B5A1:   return D3DFMT_R5G5B5A1;
        case D3DFMT_LIN_R5G6B5:     return D3DFMT_R5G6B5;
        case D3DFMT_LIN_R6G5B5:     return D3DFMT_R6G5B5;
        case D3DFMT_LIN_R8B8:       return D3DFMT_R8B8;
        case D3DFMT_LIN_R8G8B8A8:   return D3DFMT_R8G8B8A8;
        case D3DFMT_LIN_X1R5G5B5:   return D3DFMT_X1R5G5B5;
        case D3DFMT_LIN_X8R8G8B8:   return D3DFMT_X8R8G8B8;
        case D3DFMT_LIN_A8L8:       return D3DFMT_A8L8;
        case D3DFMT_LIN_AL8:        return D3DFMT_AL8;
        case D3DFMT_LIN_L16:        return D3DFMT_L16;
        case D3DFMT_LIN_L8:         return D3DFMT_L8;
        case D3DFMT_LIN_V16U16:     return D3DFMT_V16U16;
//These constants have same value as other constants above.
//        case D3DFMT_LIN_V8U8:       return D3DFMT_V8U8;
//        case D3DFMT_LIN_L6V5U5:     return D3DFMT_L6V5U5;
//        case D3DFMT_LIN_X8L8V8U8:   return D3DFMT_X8L8V8U8;
//        case D3DFMT_LIN_Q8W8V8U8:   return D3DFMT_Q8W8V8U8;
        case D3DFMT_LIN_D24S8:      return D3DFMT_D24S8;
        case D3DFMT_LIN_F24S8:      return D3DFMT_F24S8;
        case D3DFMT_LIN_D16:        return D3DFMT_D16;
        case D3DFMT_LIN_F16:        return D3DFMT_F16;
        default:
            return fmt;
    }
}



//-----------------------------------------------------------------------------
// Name: MapSwizzledToLinearFormat
// Desc: Convert from a swizzled D3DFORMAT to the equivalent linear format.
//-----------------------------------------------------------------------------
D3DFORMAT __cdecl MapSwizzledToLinearFormat( D3DFORMAT fmt )
{
    switch( fmt )
    {
        case D3DFMT_A1R5G5B5:   return D3DFMT_LIN_A1R5G5B5;
        case D3DFMT_A4R4G4B4:   return D3DFMT_LIN_A4R4G4B4;
        case D3DFMT_A8:         return D3DFMT_LIN_A8;
        case D3DFMT_A8B8G8R8:   return D3DFMT_LIN_A8B8G8R8;
        case D3DFMT_A8R8G8B8:   return D3DFMT_LIN_A8R8G8B8;
        case D3DFMT_B8G8R8A8:   return D3DFMT_LIN_B8G8R8A8;
        case D3DFMT_G8B8:       return D3DFMT_LIN_G8B8;
        case D3DFMT_R4G4B4A4:   return D3DFMT_LIN_R4G4B4A4;
        case D3DFMT_R5G5B5A1:   return D3DFMT_LIN_R5G5B5A1;
        case D3DFMT_R5G6B5:     return D3DFMT_LIN_R5G6B5;
        case D3DFMT_R6G5B5:     return D3DFMT_LIN_R6G5B5;
        case D3DFMT_R8B8:       return D3DFMT_LIN_R8B8;
        case D3DFMT_R8G8B8A8:   return D3DFMT_LIN_R8G8B8A8;
        case D3DFMT_X1R5G5B5:   return D3DFMT_LIN_X1R5G5B5;
        case D3DFMT_X8R8G8B8:   return D3DFMT_LIN_X8R8G8B8;
        case D3DFMT_A8L8:       return D3DFMT_LIN_A8L8;
        case D3DFMT_AL8:        return D3DFMT_LIN_AL8;
        case D3DFMT_L16:        return D3DFMT_LIN_L16;
        case D3DFMT_L8:         return D3DFMT_LIN_L8;
        case D3DFMT_V16U16:     return D3DFMT_LIN_V16U16;
//These constants have same value as other constants above.
//        case D3DFMT_V8U8:       return D3DFMT_LIN_V8U8;
//        case D3DFMT_L6V5U5:     return D3DFMT_LIN_L6V5U5;
//        case D3DFMT_X8L8V8U8:   return D3DFMT_LIN_X8L8V8U8;
//        case D3DFMT_Q8W8V8U8:   return D3DFMT_LIN_Q8W8V8U8;
        case D3DFMT_D24S8:      return D3DFMT_LIN_D24S8;
        case D3DFMT_F24S8:      return D3DFMT_LIN_F24S8;
        case D3DFMT_D16:        return D3DFMT_LIN_D16;
        case D3DFMT_F16:        return D3DFMT_LIN_F16;
        default:
            return fmt;
    }
}



//-----------------------------------------------------------------------------
// Name: MapToColorFormat
// Desc: Given a D3DFORMAT, return a format with which the data can be
//       displayed visually.  For example, we might convert from D24S8 to
//       R8G8B8A8.  Useful for debugging texture/surface contents visually.
// Note: For some formats, it's not clear what the "right" choice is, but we
//       try to pick something reasonable.
//-----------------------------------------------------------------------------
D3DFORMAT __cdecl MapToColorFormat( D3DFORMAT fmt )
{
    switch( fmt )
    {
        // Swizzled formats
        //case D3DFMT_P8:  // don't remap; just use current palette
        case D3DFMT_A8:
        case D3DFMT_AL8:
            return D3DFMT_L8;

        case D3DFMT_A8L8:
        case D3DFMT_L16:
        case D3DFMT_V8U8:
            return D3DFMT_G8B8;

        case D3DFMT_L6V5U5:
            return D3DFMT_R6G5B5;

        case D3DFMT_X8L8V8U8:
        case D3DFMT_Q8W8V8U8:
        case D3DFMT_V16U16:
            return D3DFMT_A8R8G8B8;

        case D3DFMT_D16:
        case D3DFMT_F16:
            return D3DFMT_R5G6B5;

        case D3DFMT_D24S8:
        case D3DFMT_F24S8:
            return D3DFMT_R8G8B8A8;


        // Linear formats
        case D3DFMT_LIN_A8:
        case D3DFMT_LIN_AL8:
            return D3DFMT_LIN_L8;

        case D3DFMT_LIN_A8L8:
        case D3DFMT_LIN_L16:
        case D3DFMT_LIN_V8U8:
            return D3DFMT_LIN_G8B8;

        case D3DFMT_LIN_L6V5U5:
            return D3DFMT_LIN_R6G5B5;

        case D3DFMT_LIN_X8L8V8U8:
        case D3DFMT_LIN_Q8W8V8U8:
        case D3DFMT_LIN_V16U16:
            return D3DFMT_LIN_A8R8G8B8;

        case D3DFMT_LIN_D16:
        case D3DFMT_LIN_F16:
            return D3DFMT_LIN_R5G6B5;
    
        case D3DFMT_LIN_D24S8:
        case D3DFMT_LIN_F24S8:
            return D3DFMT_LIN_R8G8B8A8;

        default:
            return fmt;
    }
}



///////////////////////////////////////////////////////////////////////////////
// BUFFER CLEARING
///////////////////////////////////////////////////////////////////////////////

D3DXCOLOR g_DebugClearColor(0.5f, 0.5f, 0.5f, 1.0f); // bkgrnd color when this lib clears screen


//-----------------------------------------------------------------------------
// Name: DebugClear
// Desc: Clears the current color buffer (but not z or stencil), using the
//       value specified in 'g_DebugClearColor'.
//-----------------------------------------------------------------------------
void __cdecl DebugClear()
{
    D3DDevice_Clear( 0L, NULL, D3DCLEAR_TARGET, g_DebugClearColor, 1.0f, 0 );
}



///////////////////////////////////////////////////////////////////////////////
// TEXTURE/SURFACE DEBUGGING
///////////////////////////////////////////////////////////////////////////////

bool g_bDebugShrinkToFit = true;   // decrease size (if necess) to fit screen
bool g_bDebugExpandToFit = false;  // increase size (if necess) to fill screen
bool g_bDebugAlpha = false;  // view the alpha channel when rendering


//-----------------------------------------------------------------------------
// Name: DebugPixels
// Desc: Displays a pTexture or pSurface (you can pass-in either one here).
//       For convenience, you can also pass-in special pre-defined values for
//       convenient access to common surfaces:
//          1 == 1st back-color-buffer (note: differs from GetBackBuffer param)
//          2 == 2nd back-color-buffer
//          N == Nth back-color-buffer
//         -1 == depth-stencil-buffer
// Note: This function can be called via the debugger watch window, in which
//       case it will be executed with each step through the code.
//-----------------------------------------------------------------------------
HRESULT __cdecl DebugPixels( void* pTextureOrSurface, UINT iLevel )
{
    D3DPixelContainer* pPixelContainer = (D3DPixelContainer*)pTextureOrSurface;
    
    //
    // Check for special-code argument
    //
    bool bSpecialCodeSurface = false;
    if( int(pTextureOrSurface) >= 1  &&  int(pTextureOrSurface) <= 8 )
    {
        // requested a back color buffer
        bSpecialCodeSurface = true;
        pPixelContainer = D3DDevice_GetBackBuffer2( int(pTextureOrSurface) - 1 );
    }
    else if( int(pTextureOrSurface) == -1 )
    {
        // requested depth-stencil buffer
        bSpecialCodeSurface = true;
        pPixelContainer = D3DDevice_GetDepthStencilSurface2();
    }

    if( NULL == pPixelContainer )  { return S_FALSE; }


    //
    // Get info about the width, height, format, etc. of the texture/surface
    //
    D3DSURFACE_DESC desc;
    DWORD dwPixelContainerType = (pPixelContainer->Common & D3DCOMMON_TYPE_MASK);
    switch( dwPixelContainerType )
    {
        case D3DCOMMON_TYPE_TEXTURE:
        {
            D3DTexture* pTexture = (D3DTexture*)pPixelContainer;

            DWORD nLevels = pTexture->GetLevelCount();
            if(iLevel >= nLevels)
            {
                return S_FALSE;
            }

            pTexture->GetLevelDesc(iLevel, &desc);
        }
        break;

        case D3DCOMMON_TYPE_SURFACE:
        {
            D3DSurface* pSurface = (D3DSurface*)pPixelContainer;

            pSurface->GetDesc(&desc);
            iLevel = 0;  // no other choice for surface; don't let caller specify something else
        }
        break;

        default:
            return S_FALSE; // we only handle textures and surfaces
    }

    // Dump description
#ifdef _DEBUG
    CHAR buf[1000];
    sprintf(buf, "DebugPixels(0x%0x, %d) Width=\"%d\" Height=\"%d\" Format=\"%s\"\n",
            pTextureOrSurface, iLevel, desc.Width, desc.Height, DebugFormat(desc.Format));
    OutputDebugString(buf);
#endif

    //
    // In case the texture/surface was in tiled memory, make a temp copy here
    // (w/o tiling), so that we will always view "real" contents w/o distortion
    // caused by tiling. (Yeah, this isn't optimal, and it isn't always
    // required, but it makes this function much more robust. And this is only
    // a debug function anyway....)
    //
    D3DTexture* pTexCopy = NULL;
    D3DSurface* pSurfSrc  = NULL;
    D3DSurface* pSurfDest = NULL;

    // Copy the desired level

    pTexCopy = D3DDevice_CreateTexture2( desc.Width, desc.Height, 1, 1, 0, desc.Format, D3DRTYPE_TEXTURE );
    if( NULL == pTexCopy )  { return S_FALSE; }
    {
        pTexCopy->GetSurfaceLevel( 0, &pSurfDest );
        switch(dwPixelContainerType)
        {
          case D3DCOMMON_TYPE_TEXTURE:
            ((D3DTexture*)pPixelContainer)->GetSurfaceLevel(iLevel, &pSurfSrc);
            break;
          case D3DCOMMON_TYPE_SURFACE:
            pSurfSrc = (D3DSurface*)pPixelContainer;
            break;
        }

        D3DDevice_CopyRects( pSurfSrc, NULL, 0, pSurfDest, NULL );

        pSurfDest->Release();
        switch(dwPixelContainerType)
        {
          case D3DCOMMON_TYPE_TEXTURE:  
            pSurfSrc->Release();
            break;
          case D3DCOMMON_TYPE_SURFACE:
            break;
        }
    }


    //
    // Hack the format info so that it's a viewable color-based format
    // (in case it was z-based, or whatever)
    //
    D3DFORMAT fmtOld = (D3DFORMAT)((pPixelContainer->Format & D3DFORMAT_FORMAT_MASK) >> D3DFORMAT_FORMAT_SHIFT);
    D3DFORMAT fmtTemp = MapToColorFormat(fmtOld); // remap it
    pTexCopy->Format = (pTexCopy->Format & ~D3DFORMAT_FORMAT_MASK) | (fmtTemp << D3DFORMAT_FORMAT_SHIFT);


    //
    // Calculate output size and position
    //
    UINT outpSizeX, outpSizeY;
    if(    (g_bDebugShrinkToFit && (desc.Width > 400 || desc.Height > 400))
        || (g_bDebugExpandToFit && (desc.Width < 400 && desc.Height < 400)) )
    {
        // modify size to fit well on screen (but maintain aspect ratio)
        if( desc.Width > desc.Height )
        {
            outpSizeX = 400;
            outpSizeY = int( desc.Height * (400.0f / float(desc.Width))  );
        }
        else
        {
            outpSizeY = 400;
            outpSizeX = int( desc.Width  * (400.0f / float(desc.Height)) );
        }
    }
    else
    {
        // don't modify size of texture/surface when displaying on screen
        outpSizeX = desc.Width;
        outpSizeY = desc.Height;
    }

    D3DDISPLAYMODE DisplayMode;
    D3DDevice_GetDisplayMode(&DisplayMode);
    UINT outpOffsetX = (DisplayMode.Width  - outpSizeX) / 2;  // center texture
    UINT outpOffsetY = (DisplayMode.Height - outpSizeY) / 2;

    // Account for linear vs swizzled tex coords.
    float maxU, maxV;
    if( XGIsSwizzledFormat(desc.Format) )
    {
        maxU = 1.0f;
        maxV = 1.0f;
    } else {
        maxU = (float)(desc.Width);
        maxV = (float)(desc.Height);
    }

    // Setup geometry for displaying texture/surface on screen.
    struct
    {
        float x, y, z, w;
        float u, v;
    }
    rgQuad[4] =
    {
        {outpOffsetX - 0.5f,             outpOffsetY - 0.5f,             1.0f, 1.0f, 0.0f, 0.0f },
        {outpOffsetX + outpSizeX - 0.5f, outpOffsetY - 0.5f,             1.0f, 1.0f, maxU, 0.0f },
        {outpOffsetX - 0.5f,             outpOffsetY + outpSizeY - 0.5f, 1.0f, 1.0f, 0.0f, maxV },
        {outpOffsetX + outpSizeX - 0.5f, outpOffsetY + outpSizeY - 0.5f, 1.0f, 1.0f, maxU, maxV }
    };


    //
    // Setup render states and texture states as necessary
    //
    // Notice that we set a lot of state here, because we may be called at any
    // time.  The device could be set to some arbitrary state, which could
    // cause rendering problems unless we set all states that we rely upon.
    //
    SETUP_RENDER_TARGET();

    SET_D3DVERTEXSHADER( D3DFVF_XYZRHW | D3DFVF_TEX1 );
    SET_D3DPIXELSHADER( 0 );

    SET_D3DRS( D3DRS_FILLMODE,                     D3DFILL_SOLID );
    SET_D3DRS( D3DRS_BACKFILLMODE,                 D3DFILL_SOLID );
    SET_D3DRS( D3DRS_CULLMODE,                     D3DCULL_NONE );
    SET_D3DRS( D3DRS_DITHERENABLE,                 TRUE );
    SET_D3DRS( D3DRS_ALPHATESTENABLE,              FALSE );
    SET_D3DRS( D3DRS_ALPHABLENDENABLE,             FALSE );
    SET_D3DRS( D3DRS_FOGENABLE,                    FALSE );
    SET_D3DRS( D3DRS_EDGEANTIALIAS,                FALSE );
    SET_D3DRS( D3DRS_STENCILENABLE,                FALSE );
    SET_D3DRS( D3DRS_LIGHTING,                     FALSE );
    SET_D3DRS( D3DRS_MULTISAMPLEMASK,              0xffffffff );
    SET_D3DRS( D3DRS_LOGICOP,                      D3DLOGICOP_NONE );
    SET_D3DRS( D3DRS_COLORWRITEENABLE,             D3DCOLORWRITEENABLE_ALL );
    SET_D3DRS( D3DRS_YUVENABLE,                    FALSE );
    SET_D3DRS( D3DRS_SPECULARENABLE,               FALSE );
    SET_D3DRS( D3DRS_ZBIAS,                        0 );
    SET_D3DRS( D3DRS_MULTISAMPLERENDERTARGETMODE,  D3DMULTISAMPLEMODE_1X );

    SET_D3DRS( D3DRS_ZENABLE,                      D3DZB_FALSE );

    SET_D3DTEXTURE( 0, pTexCopy );
    SET_D3DTSS( 0, D3DTSS_COLOROP,       D3DTOP_SELECTARG1 );
    SET_D3DTSS( 0, D3DTSS_COLORARG1,     g_bDebugAlpha ? D3DTA_TEXTURE|D3DTA_ALPHAREPLICATE : D3DTA_TEXTURE );
    SET_D3DTSS( 0, D3DTSS_ALPHAOP,       D3DTOP_SELECTARG1 );
    SET_D3DTSS( 0, D3DTSS_ALPHAARG1,     D3DTA_TEXTURE );
    SET_D3DTSS( 0, D3DTSS_TEXCOORDINDEX, 0 );
    SET_D3DTSS( 0, D3DTSS_ADDRESSU,      D3DTADDRESS_CLAMP );
    SET_D3DTSS( 0, D3DTSS_ADDRESSV,      D3DTADDRESS_CLAMP );
    SET_D3DTSS( 0, D3DTSS_ADDRESSW,      D3DTADDRESS_CLAMP );
    SET_D3DTSS( 0, D3DTSS_COLORKEYOP,    D3DTCOLORKEYOP_DISABLE );
    SET_D3DTSS( 0, D3DTSS_COLORSIGN,     0 );
    SET_D3DTSS( 0, D3DTSS_ALPHAKILL,     D3DTALPHAKILL_DISABLE );
    SET_D3DTSS( 0, D3DTSS_MINFILTER,     D3DTEXF_POINT );
    SET_D3DTSS( 0, D3DTSS_MAGFILTER,     D3DTEXF_POINT );

    float fMipBias = -1000.f; // bias mipmap toward the more detailed level
    SET_D3DTSS( 0, D3DTSS_MIPMAPLODBIAS, *((DWORD*) (&fMipBias)) );
    SET_D3DTSS( 0, D3DTSS_MAXMIPLEVEL,   0 );
    SET_D3DTSS( 0, D3DTSS_MIPFILTER,     D3DTEXF_POINT );
    SET_D3DTEXTURE( 1, NULL );
    SET_D3DTSS( 1, D3DTSS_COLOROP,       D3DTOP_DISABLE );
    SET_D3DTSS( 1, D3DTSS_ALPHAOP,       D3DTOP_DISABLE );
    SET_D3DTSS( 1, D3DTSS_ALPHAKILL,     D3DTALPHAKILL_DISABLE );
    SET_D3DTEXTURE( 2, NULL );
    SET_D3DTSS( 2, D3DTSS_COLOROP,       D3DTOP_DISABLE );
    SET_D3DTSS( 2, D3DTSS_ALPHAOP,       D3DTOP_DISABLE );
    SET_D3DTSS( 2, D3DTSS_ALPHAKILL,     D3DTALPHAKILL_DISABLE );
    SET_D3DTEXTURE( 3, NULL );
    SET_D3DTSS( 3, D3DTSS_COLOROP,       D3DTOP_DISABLE );
    SET_D3DTSS( 3, D3DTSS_ALPHAOP,       D3DTOP_DISABLE );
    SET_D3DTSS( 3, D3DTSS_ALPHAKILL,     D3DTALPHAKILL_DISABLE );


    //
    // Draw the texture/surface
    //
    DebugClear();
    D3DDevice_DrawVerticesUP(D3DPT_TRIANGLESTRIP, 4, rgQuad, sizeof(rgQuad[0]));
    DebugPresent();

    D3DDevice_SetTexture(0,NULL); // so D3D won't complain when we reset tex-addressing modes
                                  // (eg, to avoid case where linear tex + wrap mode gets set)

    //
    // Cleanup
    //
    if( NULL != pTexCopy )  { pTexCopy->Release(); }
    if( bSpecialCodeSurface  &&  NULL != pPixelContainer )
    {
        pPixelContainer->Release();
    }

    return S_OK;
}




//////////////////////////////////////////////////////////////////////
// Return string for texture format
//

CHAR *DebugFormat(DWORD format)
{
#define FORMAT_CASE(FORMAT) case FORMAT: return #FORMAT
    switch (format) {
        FORMAT_CASE(D3DFMT_UNKNOWN);
        FORMAT_CASE(D3DFMT_A8R8G8B8);
        FORMAT_CASE(D3DFMT_X8R8G8B8);
        FORMAT_CASE(D3DFMT_R5G6B5);
        FORMAT_CASE(D3DFMT_R6G5B5);
        FORMAT_CASE(D3DFMT_X1R5G5B5);
        FORMAT_CASE(D3DFMT_A1R5G5B5);
        FORMAT_CASE(D3DFMT_A4R4G4B4);
        FORMAT_CASE(D3DFMT_A8);
        FORMAT_CASE(D3DFMT_A8B8G8R8);
        FORMAT_CASE(D3DFMT_B8G8R8A8);
        FORMAT_CASE(D3DFMT_R4G4B4A4);
        FORMAT_CASE(D3DFMT_R5G5B5A1);
        FORMAT_CASE(D3DFMT_R8G8B8A8);
        FORMAT_CASE(D3DFMT_R8B8);
        FORMAT_CASE(D3DFMT_G8B8);
        FORMAT_CASE(D3DFMT_P8);
        FORMAT_CASE(D3DFMT_L8);
        FORMAT_CASE(D3DFMT_A8L8);
        FORMAT_CASE(D3DFMT_AL8);
        FORMAT_CASE(D3DFMT_L16);
//      FORMAT_CASE(D3DFMT_V8U8);
//      FORMAT_CASE(D3DFMT_L6V5U5);
//      FORMAT_CASE(D3DFMT_X8L8V8U8);
//      FORMAT_CASE(D3DFMT_Q8W8V8U8);
        FORMAT_CASE(D3DFMT_V16U16);
//      FORMAT_CASE(D3DFMT_D16_LOCKABLE);
        FORMAT_CASE(D3DFMT_D16);
        FORMAT_CASE(D3DFMT_D24S8);
        FORMAT_CASE(D3DFMT_F16);
        FORMAT_CASE(D3DFMT_F24S8);
        FORMAT_CASE(D3DFMT_YUY2);
        FORMAT_CASE(D3DFMT_UYVY);
        FORMAT_CASE(D3DFMT_DXT1);
        FORMAT_CASE(D3DFMT_DXT2);
        // FORMAT_CASE(D3DFMT_DXT3);
        FORMAT_CASE(D3DFMT_DXT4);
        // FORMAT_CASE(D3DFMT_DXT5);
        FORMAT_CASE(D3DFMT_LIN_A1R5G5B5);
        FORMAT_CASE(D3DFMT_LIN_A4R4G4B4);
        FORMAT_CASE(D3DFMT_LIN_A8);
        FORMAT_CASE(D3DFMT_LIN_A8B8G8R8);
        FORMAT_CASE(D3DFMT_LIN_A8R8G8B8);
        FORMAT_CASE(D3DFMT_LIN_B8G8R8A8);
        FORMAT_CASE(D3DFMT_LIN_G8B8);
        FORMAT_CASE(D3DFMT_LIN_R4G4B4A4);
        FORMAT_CASE(D3DFMT_LIN_R5G5B5A1);
        FORMAT_CASE(D3DFMT_LIN_R5G6B5);
        FORMAT_CASE(D3DFMT_LIN_R6G5B5);
        FORMAT_CASE(D3DFMT_LIN_R8B8);
        FORMAT_CASE(D3DFMT_LIN_R8G8B8A8);
        FORMAT_CASE(D3DFMT_LIN_X1R5G5B5);
        FORMAT_CASE(D3DFMT_LIN_X8R8G8B8);
        FORMAT_CASE(D3DFMT_LIN_A8L8);
        FORMAT_CASE(D3DFMT_LIN_AL8);
        FORMAT_CASE(D3DFMT_LIN_L16);
        FORMAT_CASE(D3DFMT_LIN_L8);
        FORMAT_CASE(D3DFMT_LIN_V16U16);
//      FORMAT_CASE(D3DFMT_LIN_V8U8);
//      FORMAT_CASE(D3DFMT_LIN_L6V5U5);
//      FORMAT_CASE(D3DFMT_LIN_X8L8V8U8);
//      FORMAT_CASE(D3DFMT_LIN_Q8W8V8U8);
        FORMAT_CASE(D3DFMT_LIN_D24S8);
        FORMAT_CASE(D3DFMT_LIN_F24S8);
        FORMAT_CASE(D3DFMT_LIN_D16);
        FORMAT_CASE(D3DFMT_LIN_F16);
        FORMAT_CASE(D3DFMT_VERTEXDATA);
        FORMAT_CASE(D3DFMT_INDEX16);
        
    default:
        static CHAR buf[100];
        sprintf(buf, "<unknown format 0x%0x>", format);
        return buf;
    }
#undef FORMAT_CASE
}




//////////////////////////////////////////////////////////////////////
// Return string for render state type
//

CHAR *DebugRS(DWORD rs)
{
#define RS_CASE(RS) case RS: return #RS 
    switch (rs) {
    RS_CASE(D3DRS_PSALPHAINPUTS0);    // Pixel shader, Stage 0 alpha inputs                         
    RS_CASE(D3DRS_PSALPHAINPUTS1);    // Pixel shader, Stage 1 alpha inputs                         
    RS_CASE(D3DRS_PSALPHAINPUTS2);    // Pixel shader, Stage 2 alpha inputs                         
    RS_CASE(D3DRS_PSALPHAINPUTS3);    // Pixel shader, Stage 3 alpha inputs                         
    RS_CASE(D3DRS_PSALPHAINPUTS4);    // Pixel shader, Stage 4 alpha inputs                         
    RS_CASE(D3DRS_PSALPHAINPUTS5);    // Pixel shader, Stage 5 alpha inputs                         
    RS_CASE(D3DRS_PSALPHAINPUTS6);    // Pixel shader, Stage 6 alpha inputs                         
    RS_CASE(D3DRS_PSALPHAINPUTS7);    // Pixel shader, Stage 7 alpha inputs                         
    RS_CASE(D3DRS_PSFINALCOMBINERINPUTSABCD);    // Pixel shader, Final combiner inputs ABCD
    RS_CASE(D3DRS_PSFINALCOMBINERINPUTSEFG);    // Pixel shader, Final combiner inputs EFG
    RS_CASE(D3DRS_PSCONSTANT0_0);   // Pixel shader, C0 in stage 0
    RS_CASE(D3DRS_PSCONSTANT0_1);   // Pixel shader, C0 in stage 1
    RS_CASE(D3DRS_PSCONSTANT0_2);   // Pixel shader, C0 in stage 2
    RS_CASE(D3DRS_PSCONSTANT0_3);   // Pixel shader, C0 in stage 3
    RS_CASE(D3DRS_PSCONSTANT0_4);   // Pixel shader, C0 in stage 4
    RS_CASE(D3DRS_PSCONSTANT0_5);   // Pixel shader, C0 in stage 5
    RS_CASE(D3DRS_PSCONSTANT0_6);   // Pixel shader, C0 in stage 6
    RS_CASE(D3DRS_PSCONSTANT0_7);   // Pixel shader, C0 in stage 7
    RS_CASE(D3DRS_PSCONSTANT1_0);   // Pixel shader, C1 in stage 0
    RS_CASE(D3DRS_PSCONSTANT1_1);   // Pixel shader, C1 in stage 1
    RS_CASE(D3DRS_PSCONSTANT1_2);   // Pixel shader, C1 in stage 2
    RS_CASE(D3DRS_PSCONSTANT1_3);   // Pixel shader, C1 in stage 3
    RS_CASE(D3DRS_PSCONSTANT1_4);   // Pixel shader, C1 in stage 4
    RS_CASE(D3DRS_PSCONSTANT1_5);   // Pixel shader, C1 in stage 5
    RS_CASE(D3DRS_PSCONSTANT1_6);   // Pixel shader, C1 in stage 6
    RS_CASE(D3DRS_PSCONSTANT1_7);   // Pixel shader, C1 in stage 7
    RS_CASE(D3DRS_PSALPHAOUTPUTS0);   // Pixel shader, Stage 0 alpha outputs                        
    RS_CASE(D3DRS_PSALPHAOUTPUTS1);   // Pixel shader, Stage 1 alpha outputs                        
    RS_CASE(D3DRS_PSALPHAOUTPUTS2);   // Pixel shader, Stage 2 alpha outputs                        
    RS_CASE(D3DRS_PSALPHAOUTPUTS3);   // Pixel shader, Stage 3 alpha outputs                        
    RS_CASE(D3DRS_PSALPHAOUTPUTS4);   // Pixel shader, Stage 4 alpha outputs                        
    RS_CASE(D3DRS_PSALPHAOUTPUTS5);   // Pixel shader, Stage 5 alpha outputs                        
    RS_CASE(D3DRS_PSALPHAOUTPUTS6);   // Pixel shader, Stage 6 alpha outputs                        
    RS_CASE(D3DRS_PSALPHAOUTPUTS7);   // Pixel shader, Stage 7 alpha outputs                        
    RS_CASE(D3DRS_PSRGBINPUTS0);   // Pixel shader, Stage 0 RGB inputs                           
    RS_CASE(D3DRS_PSRGBINPUTS1);   // Pixel shader, Stage 1 RGB inputs                           
    RS_CASE(D3DRS_PSRGBINPUTS2);   // Pixel shader, Stage 2 RGB inputs                           
    RS_CASE(D3DRS_PSRGBINPUTS3);   // Pixel shader, Stage 3 RGB inputs                           
    RS_CASE(D3DRS_PSRGBINPUTS4);   // Pixel shader, Stage 4 RGB inputs                           
    RS_CASE(D3DRS_PSRGBINPUTS5);   // Pixel shader, Stage 5 RGB inputs                           
    RS_CASE(D3DRS_PSRGBINPUTS6);   // Pixel shader, Stage 6 RGB inputs                           
    RS_CASE(D3DRS_PSRGBINPUTS7);   // Pixel shader, Stage 7 RGB inputs                           
    RS_CASE(D3DRS_PSCOMPAREMODE);   // Pixel shader, Compare modes for clipplane texture mode     
    RS_CASE(D3DRS_PSFINALCOMBINERCONSTANT0);   // Pixel shader, C0 in final combiner
    RS_CASE(D3DRS_PSFINALCOMBINERCONSTANT1);   // Pixel shader, C1 in final combiner
    RS_CASE(D3DRS_PSRGBOUTPUTS0);   // Pixel shader, Stage 0 RGB outputs                          
    RS_CASE(D3DRS_PSRGBOUTPUTS1);   // Pixel shader, Stage 1 RGB outputs                          
    RS_CASE(D3DRS_PSRGBOUTPUTS2);   // Pixel shader, Stage 2 RGB outputs                          
    RS_CASE(D3DRS_PSRGBOUTPUTS3);   // Pixel shader, Stage 3 RGB outputs                          
    RS_CASE(D3DRS_PSRGBOUTPUTS4);   // Pixel shader, Stage 4 RGB outputs                          
    RS_CASE(D3DRS_PSRGBOUTPUTS5);   // Pixel shader, Stage 5 RGB outputs                          
    RS_CASE(D3DRS_PSRGBOUTPUTS6);   // Pixel shader, Stage 6 RGB outputs                          
    RS_CASE(D3DRS_PSRGBOUTPUTS7);   // Pixel shader, Stage 7 RGB outputs                          
    RS_CASE(D3DRS_PSCOMBINERCOUNT);   // Pixel shader, Active combiner count (Stages 0-7)           
                                            // Pixel shader, Reserved
    RS_CASE(D3DRS_PSDOTMAPPING);   // Pixel shader, Input mapping for dot product modes          
    RS_CASE(D3DRS_PSINPUTTEXTURE);   // Pixel shader, Texture source for some texture modes        

    RS_CASE(D3DRS_ZFUNC);   // D3DCMPFUNC 
    RS_CASE(D3DRS_ALPHAFUNC);   // D3DCMPFUNC 
    RS_CASE(D3DRS_ALPHABLENDENABLE);   // TRUE to enable alpha blending 
    RS_CASE(D3DRS_ALPHATESTENABLE);   // TRUE to enable alpha tests 
    RS_CASE(D3DRS_ALPHAREF);   // BYTE
    RS_CASE(D3DRS_SRCBLEND);   // D3DBLEND 
    RS_CASE(D3DRS_DESTBLEND);   // D3DBLEND 
    RS_CASE(D3DRS_ZWRITEENABLE);   // TRUE to enable Z writes 
    RS_CASE(D3DRS_DITHERENABLE);   // TRUE to enable dithering 
    RS_CASE(D3DRS_SHADEMODE);   // D3DSHADEMODE 
    RS_CASE(D3DRS_COLORWRITEENABLE);   // D3DCOLORWRITEENABLE_ALPHA, etc. per-channel write enable
    RS_CASE(D3DRS_STENCILZFAIL);   // D3DSTENCILOP to do if stencil test passes and Z test fails 
    RS_CASE(D3DRS_STENCILPASS);   // D3DSTENCILOP to do if both stencil and Z tests pass 
    RS_CASE(D3DRS_STENCILFUNC);   // D3DCMPFUNC
    RS_CASE(D3DRS_STENCILREF);   // BYTE reference value used in stencil test 
    RS_CASE(D3DRS_STENCILMASK);   // BYTE mask value used in stencil test 
    RS_CASE(D3DRS_STENCILWRITEMASK);   // BYTE write mask applied to values written to stencil buffer 
    RS_CASE(D3DRS_BLENDOP);   // D3DBLENDOP setting
    RS_CASE(D3DRS_BLENDCOLOR);   // D3DCOLOR for D3DBLEND_CONSTANT, etc. (Xbox extension)
    RS_CASE(D3DRS_SWATHWIDTH);   // D3DSWATHWIDTH (Xbox extension)
    RS_CASE(D3DRS_POLYGONOFFSETZSLOPESCALE);   // float Z factor for shadow maps (Xbox extension)
    RS_CASE(D3DRS_POLYGONOFFSETZOFFSET);   // float bias for polygon offset (Xbox extension)
    RS_CASE(D3DRS_POINTOFFSETENABLE);   // TRUE to enable polygon offset for points (Xbox extension)
    RS_CASE(D3DRS_WIREFRAMEOFFSETENABLE);   // TRUE to enable polygon offset for lines (Xbox extension)
    RS_CASE(D3DRS_SOLIDOFFSETENABLE);   // TRUE to enable polygon offset for fills (Xbox extension)
    RS_CASE(D3DRS_DEPTHCLIPCONTROL);  // D3DDEPTHCLIPPRIMITIVE | D3DDEPTHCLIPCULLORCLAMP | D3DDEPTHCLIPUSEWSIGN  (Xbox extension)

    // State whose handling is deferred until the next Draw[Indexed]Vertices
    // call because of interdependencies on other states:

    RS_CASE(D3DRS_FOGENABLE);   // TRUE to enable fog blending 
    RS_CASE(D3DRS_FOGTABLEMODE);   // D3DFOGMODE 
    RS_CASE(D3DRS_FOGSTART);   // float fog start (for both vertex and pixel fog) 
    RS_CASE(D3DRS_FOGEND);   // float fog end      
    RS_CASE(D3DRS_FOGDENSITY);   // float fog density  
    RS_CASE(D3DRS_RANGEFOGENABLE);   // TRUE to enable range-based fog 
    RS_CASE(D3DRS_WRAP0);   // D3DWRAP* flags (D3DWRAP_U, D3DWRAPCOORD_0, etc.) for 1st texture coord.
    RS_CASE(D3DRS_WRAP1);   // D3DWRAP* flags (D3DWRAP_U, D3DWRAPCOORD_0, etc.) for 2nd texture coord. 
    RS_CASE(D3DRS_WRAP2);   // D3DWRAP* flags (D3DWRAP_U, D3DWRAPCOORD_0, etc.) for 3rd texture coord. 
    RS_CASE(D3DRS_WRAP3);   // D3DWRAP* flags (D3DWRAP_U, D3DWRAPCOORD_0, etc.) for 4th texture coord. 
    RS_CASE(D3DRS_LIGHTING);   // TRUE to enable lighting
    RS_CASE(D3DRS_SPECULARENABLE);   // TRUE to enable specular 
    RS_CASE(D3DRS_LOCALVIEWER);   // TRUE to enable camera-relative specular highlights
    RS_CASE(D3DRS_COLORVERTEX);   // TRUE to enable per-vertex color
    RS_CASE(D3DRS_BACKSPECULARMATERIALSOURCE);   // D3DMATERIALCOLORSOURCE (Xbox extension)
    RS_CASE(D3DRS_BACKDIFFUSEMATERIALSOURCE);   // D3DMATERIALCOLORSOURCE (Xbox extension)
    RS_CASE(D3DRS_BACKAMBIENTMATERIALSOURCE);   // D3DMATERIALCOLORSOURCE (Xbox extension)
    RS_CASE(D3DRS_BACKEMISSIVEMATERIALSOURCE);   // D3DMATERIALCOLORSOURCE (Xbox extension)
    RS_CASE(D3DRS_SPECULARMATERIALSOURCE);  // D3DMATERIALCOLORSOURCE 
    RS_CASE(D3DRS_DIFFUSEMATERIALSOURCE);  // D3DMATERIALCOLORSOURCE 
    RS_CASE(D3DRS_AMBIENTMATERIALSOURCE);  // D3DMATERIALCOLORSOURCE 
    RS_CASE(D3DRS_EMISSIVEMATERIALSOURCE);  // D3DMATERIALCOLORSOURCE 
    RS_CASE(D3DRS_BACKAMBIENT);  // D3DCOLOR (Xbox extension)
    RS_CASE(D3DRS_AMBIENT);  // D3DCOLOR 
    RS_CASE(D3DRS_POINTSIZE);  // float point size 
    RS_CASE(D3DRS_POINTSIZE_MIN);  // float point size min threshold 
    RS_CASE(D3DRS_POINTSPRITEENABLE);  // TRUE to enable point sprites
    RS_CASE(D3DRS_POINTSCALEENABLE);  // TRUE to enable point size scaling
    RS_CASE(D3DRS_POINTSCALE_A);  // float point attenuation A value 
    RS_CASE(D3DRS_POINTSCALE_B);  // float point attenuation B value 
    RS_CASE(D3DRS_POINTSCALE_C);  // float point attenuation C value 
    RS_CASE(D3DRS_POINTSIZE_MAX);  // float point size max threshold 
    RS_CASE(D3DRS_PATCHEDGESTYLE);  // D3DPATCHEDGESTYLE
    RS_CASE(D3DRS_PATCHSEGMENTS);  // DWORD number of segments per edge when drawing patches
    RS_CASE(D3DRS_SWAPFILTER);
    
    // Complex state that has immediate processing:

    RS_CASE(D3DRS_PSTEXTUREMODES);  // Pixel shader, Texture addressing modes (Xbox extension)
    RS_CASE(D3DRS_VERTEXBLEND);  // D3DVERTEXBLENDFLAGS
    RS_CASE(D3DRS_FOGCOLOR);  // D3DCOLOR 
    RS_CASE(D3DRS_FILLMODE);  // D3DFILLMODE        
    RS_CASE(D3DRS_BACKFILLMODE);  // D3DFILLMODE (Xbox extension)
    RS_CASE(D3DRS_TWOSIDEDLIGHTING);  // TRUE to enable two-sided lighting (Xbox extension)
    RS_CASE(D3DRS_NORMALIZENORMALS);  // TRUE to enable automatic normalization
    RS_CASE(D3DRS_ZENABLE);  // D3DZBUFFERTYPE (or TRUE/FALSE for legacy) 
    RS_CASE(D3DRS_STENCILENABLE);  // TRUE to enable stenciling
    RS_CASE(D3DRS_STENCILFAIL);  // D3DSTENCILOP to do if stencil test fails 
    RS_CASE(D3DRS_FRONTFACE);  // D3DFRONT (Xbox extension)
    RS_CASE(D3DRS_CULLMODE);  // D3DCULL 
    RS_CASE(D3DRS_TEXTUREFACTOR);  // D3DCOLOR used for multi-texture blend 
    RS_CASE(D3DRS_ZBIAS);  // LONG Z bias 
    RS_CASE(D3DRS_LOGICOP);  // D3DLOGICOP (Xbox extension)
    RS_CASE(D3DRS_EDGEANTIALIAS);  // TRUE to enable edge antialiasing (Xbox extension)
    RS_CASE(D3DRS_MULTISAMPLEANTIALIAS);  // TRUE to enable multisample antialiasing
    RS_CASE(D3DRS_MULTISAMPLEMASK);  // DWORD per-pixel and sample enable/disable
    RS_CASE(D3DRS_MULTISAMPLEMODE);  // D3DMULTISAMPLEMODE for the backbuffer (Xbox extension)
    RS_CASE(D3DRS_MULTISAMPLERENDERTARGETMODE); // D3DMULTISAMPLEMODE for non-backbuffer render targets (Xbox extension)
    RS_CASE(D3DRS_SHADOWFUNC);  // D3DCMPFUNC (Xbox extension)
    RS_CASE(D3DRS_LINEWIDTH);  // float (Xbox extension)
    RS_CASE(D3DRS_DXT1NOISEENABLE);  // TRUE to enable DXT1 decompression noise (Xbox extension)
    RS_CASE(D3DRS_YUVENABLE);  // TRUE to enable use of D3DFMT_YUY2 and D3DFMT_UYVY texture formats
    RS_CASE(D3DRS_OCCLUSIONCULLENABLE);  // TRUE to enable Z occlusion culling
    RS_CASE(D3DRS_STENCILCULLENABLE);  // TRUE to enable stencil culling
    RS_CASE(D3DRS_ROPZCMPALWAYSREAD);  // TRUE to always read target packet when Z enabled
    RS_CASE(D3DRS_ROPZREAD);  // TRUE to always read Z
    RS_CASE(D3DRS_DONOTCULLUNCOMPRESSED);  // TRUE to never attempt occlusion culling (stencil or Z) on uncompressed packets

    default:
        static CHAR buf[100];
        sprintf(buf, "<unknown render state index %d>", rs);
        return buf;
    }
#undef RS_CASE  
}

static CHAR *DebugD3DBLEND(DWORD value)
{
#define D3DBLEND_CASE(BLEND) case BLEND: return #BLEND
	switch (value)
	{
    D3DBLEND_CASE(D3DBLEND_ZERO);
    D3DBLEND_CASE(D3DBLEND_ONE);
    D3DBLEND_CASE(D3DBLEND_SRCCOLOR);
    D3DBLEND_CASE(D3DBLEND_INVSRCCOLOR);
    D3DBLEND_CASE(D3DBLEND_SRCALPHA);
    D3DBLEND_CASE(D3DBLEND_INVSRCALPHA);
    D3DBLEND_CASE(D3DBLEND_DESTALPHA);
    D3DBLEND_CASE(D3DBLEND_INVDESTALPHA);
    D3DBLEND_CASE(D3DBLEND_DESTCOLOR);
    D3DBLEND_CASE(D3DBLEND_INVDESTCOLOR);
    D3DBLEND_CASE(D3DBLEND_SRCALPHASAT);
    D3DBLEND_CASE(D3DBLEND_CONSTANTCOLOR);
    D3DBLEND_CASE(D3DBLEND_INVCONSTANTCOLOR);
    D3DBLEND_CASE(D3DBLEND_CONSTANTALPHA);
    D3DBLEND_CASE(D3DBLEND_INVCONSTANTALPHA);
	default:
        static CHAR buf[100];
        sprintf(buf, "<unknown D3DBLEND 0x%x>", value);
        return buf;
	}
#undef D3DBLEND_CASE
}

static CHAR *DebugD3DBLENDOP(DWORD value)
{
#define D3DBLENDOP_CASE(BLENDOP) case BLENDOP: return #BLENDOP
	switch (value)
	{
    D3DBLENDOP_CASE(D3DBLENDOP_ADD);
    D3DBLENDOP_CASE(D3DBLENDOP_SUBTRACT);
    D3DBLENDOP_CASE(D3DBLENDOP_REVSUBTRACT);
    D3DBLENDOP_CASE(D3DBLENDOP_MIN);
    D3DBLENDOP_CASE(D3DBLENDOP_MAX);
    D3DBLENDOP_CASE(D3DBLENDOP_ADDSIGNED);
    D3DBLENDOP_CASE(D3DBLENDOP_REVSUBTRACTSIGNED);
	default:
        static CHAR buf[100];
        sprintf(buf, "<unknown D3DBLENDOP 0x%x>", value);
        return buf;
	}
#undef D3DBLENDOP_CASE
}

static CHAR *DebugD3DCMPFUNC(DWORD value)
{
#define D3DCMPFUNC_CASE(CMPFUNC) case CMPFUNC: return #CMPFUNC
	switch (value)
	{
	D3DCMPFUNC_CASE(D3DCMP_NEVER);
    D3DCMPFUNC_CASE(D3DCMP_LESS);
    D3DCMPFUNC_CASE(D3DCMP_EQUAL);
    D3DCMPFUNC_CASE(D3DCMP_LESSEQUAL);
    D3DCMPFUNC_CASE(D3DCMP_GREATER);
    D3DCMPFUNC_CASE(D3DCMP_NOTEQUAL);
    D3DCMPFUNC_CASE(D3DCMP_GREATEREQUAL);
    D3DCMPFUNC_CASE(D3DCMP_ALWAYS);
	default:
        static CHAR buf[100];
        sprintf(buf, "<unknown D3DCMPFUNC 0x%x>", value);
        return buf;
	}
#undef D3DCMPFUNC_CASE
}

static CHAR *DebugD3DCULL(DWORD value)
{
#define D3DCULL_CASE(CULL) case CULL: return #CULL
	switch (value)
	{
    D3DCULL_CASE(D3DCULL_NONE);
    D3DCULL_CASE(D3DCULL_CW);
    D3DCULL_CASE(D3DCULL_CCW);
	default:
        static CHAR buf[100];
        sprintf(buf, "<unknown D3DCULL 0x%x>", value);
        return buf;
	}
#undef D3DCULL_CASE
}

static CHAR *DebugD3DFILLMODE(DWORD value)
{
#define D3DFILLMODE_CASE(FILLMODE) case FILLMODE: return #FILLMODE
	switch (value)
	{
    D3DFILLMODE_CASE(D3DFILL_POINT);
    D3DFILLMODE_CASE(D3DFILL_WIREFRAME);
    D3DFILLMODE_CASE(D3DFILL_SOLID);
	default:
        static CHAR buf[100];
        sprintf(buf, "<unknown D3DFILLMODE 0x%x>", value);
        return buf;
	}
#undef D3DFILLMODE_CASE
}

static CHAR *DebugD3DFRONT(DWORD value)
{
#define D3DFRONT_CASE(FRONT) case FRONT: return #FRONT
	switch (value)
	{
    D3DFRONT_CASE(D3DFRONT_CW);
    D3DFRONT_CASE(D3DFRONT_CCW);
	default:
        static CHAR buf[100];
        sprintf(buf, "<unknown D3DFRONT 0x%x>", value);
        return buf;
	}
#undef D3DFRONT_CASE
}

static CHAR *DebugD3DLOGICOP(DWORD value)
{
#define D3DLOGICOP_CASE(LOGICOP) case LOGICOP: return #LOGICOP
	switch (value)
	{
    D3DLOGICOP_CASE(D3DLOGICOP_NONE);
    D3DLOGICOP_CASE(D3DLOGICOP_CLEAR);
    D3DLOGICOP_CASE(D3DLOGICOP_AND);
    D3DLOGICOP_CASE(D3DLOGICOP_AND_REVERSE);
    D3DLOGICOP_CASE(D3DLOGICOP_COPY);
    D3DLOGICOP_CASE(D3DLOGICOP_AND_INVERTED);
    D3DLOGICOP_CASE(D3DLOGICOP_NOOP);
    D3DLOGICOP_CASE(D3DLOGICOP_XOR);
    D3DLOGICOP_CASE(D3DLOGICOP_OR);
    D3DLOGICOP_CASE(D3DLOGICOP_NOR);
    D3DLOGICOP_CASE(D3DLOGICOP_EQUIV);
    D3DLOGICOP_CASE(D3DLOGICOP_INVERT);
    D3DLOGICOP_CASE(D3DLOGICOP_OR_REVERSE);
    D3DLOGICOP_CASE(D3DLOGICOP_COPY_INVERTED);
    D3DLOGICOP_CASE(D3DLOGICOP_OR_INVERTED);
    D3DLOGICOP_CASE(D3DLOGICOP_NAND);
    D3DLOGICOP_CASE(D3DLOGICOP_SET);
	default:
        static CHAR buf[100];
        sprintf(buf, "<unknown D3DLOGICOP 0x%x>", value);
        return buf;
	}
#undef D3DLOGICOP_CASE
}

static CHAR *DebugD3DSTENCILOP(DWORD value)
{
#define D3DSTENCILOP_CASE(STENCILOP) case STENCILOP: return #STENCILOP
	switch (value)
	{
    D3DSTENCILOP_CASE(D3DSTENCILOP_KEEP);
    D3DSTENCILOP_CASE(D3DSTENCILOP_ZERO);
    D3DSTENCILOP_CASE(D3DSTENCILOP_REPLACE);
    D3DSTENCILOP_CASE(D3DSTENCILOP_INCRSAT);
    D3DSTENCILOP_CASE(D3DSTENCILOP_DECRSAT);
    D3DSTENCILOP_CASE(D3DSTENCILOP_INVERT);
    D3DSTENCILOP_CASE(D3DSTENCILOP_INCR);
    D3DSTENCILOP_CASE(D3DSTENCILOP_DECR);
	default:
        static CHAR buf[100];
        sprintf(buf, "<unknown D3DSTENCILOP 0x%x>", value);
        return buf;
	}
#undef D3DSTENCILOP_CASE
}

static CHAR *DebugD3DSHADEMODE(DWORD value)
{
#define D3DSHADEMODE_CASE(SHADEMODE) case SHADEMODE: return #SHADEMODE
	switch (value)
	{
    D3DSHADEMODE_CASE(D3DSHADE_FLAT);
    D3DSHADEMODE_CASE(D3DSHADE_GOURAUD);
	default:
        static CHAR buf[100];
        sprintf(buf, "<unknown D3DSHADEMODE 0x%x>", value);
        return buf;
	}
#undef D3DSHADEMODE_CASE
}

//////////////////////////////////////////////////////////////////////
// Return formatted string for render state value
//
// TODO: make prettier strings for the enumerated types
//
CHAR *DebugRSValue(DWORD rs, DWORD value)
{
    const int c_nbuf = 5;
    static CHAR s_rbuf[c_nbuf][100];
    static int s_ibuf = 0;
    CHAR *buf = s_rbuf[s_ibuf];
    s_ibuf++; if (s_ibuf >= c_nbuf) s_ibuf = 0;
    
#define RS_CASE(RS)               case RS: sprintf(buf, "0x%x", value); return buf
#define RS_CASE_FLOAT(RS)         case RS: sprintf(buf, "%g", *((float*) (&value))); return buf
#define RS_CASE_D3DBLEND(RS)      case RS: return DebugD3DBLEND(value)
#define RS_CASE_D3DBLENDOP(RS)    case RS: return DebugD3DBLENDOP(value)
#define RS_CASE_D3DCMPFUNC(RS)    case RS: return DebugD3DCMPFUNC(value)
#define RS_CASE_D3DCULL(RS)       case RS: return DebugD3DCULL(value)
#define RS_CASE_D3DFILLMODE(RS)   case RS: return DebugD3DFILLMODE(value)
#define RS_CASE_D3DFRONT(RS)      case RS: return DebugD3DFRONT(value)
#define RS_CASE_D3DLOGICOP(RS)    case RS: return DebugD3DLOGICOP(value)
#define RS_CASE_D3DSTENCILOP(RS)  case RS: return DebugD3DSTENCILOP(value)
#define RS_CASE_D3DSHADEMODE(RS)  case RS: return DebugD3DSHADEMODE(value)
    switch (rs) {
    RS_CASE(D3DRS_PSALPHAINPUTS0);    // Pixel shader, Stage 0 alpha inputs                         
    RS_CASE(D3DRS_PSALPHAINPUTS1);    // Pixel shader, Stage 1 alpha inputs                         
    RS_CASE(D3DRS_PSALPHAINPUTS2);    // Pixel shader, Stage 2 alpha inputs                         
    RS_CASE(D3DRS_PSALPHAINPUTS3);    // Pixel shader, Stage 3 alpha inputs                         
    RS_CASE(D3DRS_PSALPHAINPUTS4);    // Pixel shader, Stage 4 alpha inputs                         
    RS_CASE(D3DRS_PSALPHAINPUTS5);    // Pixel shader, Stage 5 alpha inputs                         
    RS_CASE(D3DRS_PSALPHAINPUTS6);    // Pixel shader, Stage 6 alpha inputs                         
    RS_CASE(D3DRS_PSALPHAINPUTS7);    // Pixel shader, Stage 7 alpha inputs                         
    RS_CASE(D3DRS_PSFINALCOMBINERINPUTSABCD);    // Pixel shader, Final combiner inputs ABCD
    RS_CASE(D3DRS_PSFINALCOMBINERINPUTSEFG);    // Pixel shader, Final combiner inputs EFG
    RS_CASE(D3DRS_PSCONSTANT0_0);   // Pixel shader, C0 in stage 0
    RS_CASE(D3DRS_PSCONSTANT0_1);   // Pixel shader, C0 in stage 1
    RS_CASE(D3DRS_PSCONSTANT0_2);   // Pixel shader, C0 in stage 2
    RS_CASE(D3DRS_PSCONSTANT0_3);   // Pixel shader, C0 in stage 3
    RS_CASE(D3DRS_PSCONSTANT0_4);   // Pixel shader, C0 in stage 4
    RS_CASE(D3DRS_PSCONSTANT0_5);   // Pixel shader, C0 in stage 5
    RS_CASE(D3DRS_PSCONSTANT0_6);   // Pixel shader, C0 in stage 6
    RS_CASE(D3DRS_PSCONSTANT0_7);   // Pixel shader, C0 in stage 7
    RS_CASE(D3DRS_PSCONSTANT1_0);   // Pixel shader, C1 in stage 0
    RS_CASE(D3DRS_PSCONSTANT1_1);   // Pixel shader, C1 in stage 1
    RS_CASE(D3DRS_PSCONSTANT1_2);   // Pixel shader, C1 in stage 2
    RS_CASE(D3DRS_PSCONSTANT1_3);   // Pixel shader, C1 in stage 3
    RS_CASE(D3DRS_PSCONSTANT1_4);   // Pixel shader, C1 in stage 4
    RS_CASE(D3DRS_PSCONSTANT1_5);   // Pixel shader, C1 in stage 5
    RS_CASE(D3DRS_PSCONSTANT1_6);   // Pixel shader, C1 in stage 6
    RS_CASE(D3DRS_PSCONSTANT1_7);   // Pixel shader, C1 in stage 7
    RS_CASE(D3DRS_PSALPHAOUTPUTS0);   // Pixel shader, Stage 0 alpha outputs                        
    RS_CASE(D3DRS_PSALPHAOUTPUTS1);   // Pixel shader, Stage 1 alpha outputs                        
    RS_CASE(D3DRS_PSALPHAOUTPUTS2);   // Pixel shader, Stage 2 alpha outputs                        
    RS_CASE(D3DRS_PSALPHAOUTPUTS3);   // Pixel shader, Stage 3 alpha outputs                        
    RS_CASE(D3DRS_PSALPHAOUTPUTS4);   // Pixel shader, Stage 4 alpha outputs                        
    RS_CASE(D3DRS_PSALPHAOUTPUTS5);   // Pixel shader, Stage 5 alpha outputs                        
    RS_CASE(D3DRS_PSALPHAOUTPUTS6);   // Pixel shader, Stage 6 alpha outputs                        
    RS_CASE(D3DRS_PSALPHAOUTPUTS7);   // Pixel shader, Stage 7 alpha outputs                        
    RS_CASE(D3DRS_PSRGBINPUTS0);   // Pixel shader, Stage 0 RGB inputs                           
    RS_CASE(D3DRS_PSRGBINPUTS1);   // Pixel shader, Stage 1 RGB inputs                           
    RS_CASE(D3DRS_PSRGBINPUTS2);   // Pixel shader, Stage 2 RGB inputs                           
    RS_CASE(D3DRS_PSRGBINPUTS3);   // Pixel shader, Stage 3 RGB inputs                           
    RS_CASE(D3DRS_PSRGBINPUTS4);   // Pixel shader, Stage 4 RGB inputs                           
    RS_CASE(D3DRS_PSRGBINPUTS5);   // Pixel shader, Stage 5 RGB inputs                           
    RS_CASE(D3DRS_PSRGBINPUTS6);   // Pixel shader, Stage 6 RGB inputs                           
    RS_CASE(D3DRS_PSRGBINPUTS7);   // Pixel shader, Stage 7 RGB inputs                           
    RS_CASE(D3DRS_PSCOMPAREMODE);   // Pixel shader, Compare modes for clipplane texture mode     
    RS_CASE(D3DRS_PSFINALCOMBINERCONSTANT0);   // Pixel shader, C0 in final combiner
    RS_CASE(D3DRS_PSFINALCOMBINERCONSTANT1);   // Pixel shader, C1 in final combiner
    RS_CASE(D3DRS_PSRGBOUTPUTS0);   // Pixel shader, Stage 0 RGB outputs                          
    RS_CASE(D3DRS_PSRGBOUTPUTS1);   // Pixel shader, Stage 1 RGB outputs                          
    RS_CASE(D3DRS_PSRGBOUTPUTS2);   // Pixel shader, Stage 2 RGB outputs                          
    RS_CASE(D3DRS_PSRGBOUTPUTS3);   // Pixel shader, Stage 3 RGB outputs                          
    RS_CASE(D3DRS_PSRGBOUTPUTS4);   // Pixel shader, Stage 4 RGB outputs                          
    RS_CASE(D3DRS_PSRGBOUTPUTS5);   // Pixel shader, Stage 5 RGB outputs                          
    RS_CASE(D3DRS_PSRGBOUTPUTS6);   // Pixel shader, Stage 6 RGB outputs                          
    RS_CASE(D3DRS_PSRGBOUTPUTS7);   // Pixel shader, Stage 7 RGB outputs                          
    RS_CASE(D3DRS_PSCOMBINERCOUNT);   // Pixel shader, Active combiner count (Stages 0-7)           
                                            // Pixel shader, Reserved
    RS_CASE(D3DRS_PSDOTMAPPING);   // Pixel shader, Input mapping for dot product modes          
    RS_CASE(D3DRS_PSINPUTTEXTURE);   // Pixel shader, Texture source for some texture modes        

    RS_CASE_D3DCMPFUNC(D3DRS_ZFUNC);   // D3DCMPFUNC 
    RS_CASE_D3DCMPFUNC(D3DRS_ALPHAFUNC);   // D3DCMPFUNC 
    RS_CASE(D3DRS_ALPHABLENDENABLE);   // TRUE to enable alpha blending 
    RS_CASE(D3DRS_ALPHATESTENABLE);   // TRUE to enable alpha tests 
    RS_CASE(D3DRS_ALPHAREF);   // BYTE
    RS_CASE_D3DBLEND(D3DRS_SRCBLEND);   // D3DBLEND 
    RS_CASE_D3DBLEND(D3DRS_DESTBLEND);   // D3DBLEND 
    RS_CASE(D3DRS_ZWRITEENABLE);   // TRUE to enable Z writes 
    RS_CASE(D3DRS_DITHERENABLE);   // TRUE to enable dithering 
    RS_CASE_D3DSHADEMODE(D3DRS_SHADEMODE);   // D3DSHADEMODE 
    RS_CASE(D3DRS_COLORWRITEENABLE);   // D3DCOLORWRITEENABLE_ALPHA, etc. per-channel write enable
    RS_CASE_D3DSTENCILOP(D3DRS_STENCILZFAIL);   // D3DSTENCILOP to do if stencil test passes and Z test fails 
    RS_CASE_D3DSTENCILOP(D3DRS_STENCILPASS);   // D3DSTENCILOP to do if both stencil and Z tests pass 
    RS_CASE_D3DCMPFUNC(D3DRS_STENCILFUNC);   // D3DCMPFUNC
    RS_CASE(D3DRS_STENCILREF);   // BYTE reference value used in stencil test 
    RS_CASE(D3DRS_STENCILMASK);   // BYTE mask value used in stencil test 
    RS_CASE(D3DRS_STENCILWRITEMASK);   // BYTE write mask applied to values written to stencil buffer 
    RS_CASE_D3DBLENDOP(D3DRS_BLENDOP);   // D3DBLENDOP setting
    RS_CASE(D3DRS_BLENDCOLOR);   // D3DCOLOR for D3DBLEND_CONSTANT, etc. (Xbox extension)
    RS_CASE(D3DRS_SWATHWIDTH);   // D3DSWATHWIDTH (Xbox extension)
    RS_CASE_FLOAT(D3DRS_POLYGONOFFSETZSLOPESCALE);   // float Z factor for shadow maps (Xbox extension)
    RS_CASE_FLOAT(D3DRS_POLYGONOFFSETZOFFSET);   // float bias for polygon offset (Xbox extension)
    RS_CASE(D3DRS_POINTOFFSETENABLE);   // TRUE to enable polygon offset for points (Xbox extension)
    RS_CASE(D3DRS_WIREFRAMEOFFSETENABLE);   // TRUE to enable polygon offset for lines (Xbox extension)
    RS_CASE(D3DRS_SOLIDOFFSETENABLE);   // TRUE to enable polygon offset for fills (Xbox extension)
    RS_CASE(D3DRS_DEPTHCLIPCONTROL);  // D3DDEPTHCLIPPRIMITIVE | D3DDEPTHCLIPCULLORCLAMP | D3DDEPTHCLIPUSEWSIGN  (Xbox extension)
	
    // State whose handling is deferred until the next Draw[Indexed]Vertices
    // call because of interdependencies on other states:

    RS_CASE(D3DRS_FOGENABLE);   // TRUE to enable fog blending 
    RS_CASE(D3DRS_FOGTABLEMODE);   // D3DFOGMODE 
    RS_CASE_FLOAT(D3DRS_FOGSTART);   // float fog start (for both vertex and pixel fog) 
    RS_CASE_FLOAT(D3DRS_FOGEND);   // float fog end      
    RS_CASE_FLOAT(D3DRS_FOGDENSITY);   // float fog density  
    RS_CASE(D3DRS_RANGEFOGENABLE);   // TRUE to enable range-based fog 
    RS_CASE(D3DRS_WRAP0);   // D3DWRAP* flags (D3DWRAP_U, D3DWRAPCOORD_0, etc.) for 1st texture coord.
    RS_CASE(D3DRS_WRAP1);   // D3DWRAP* flags (D3DWRAP_U, D3DWRAPCOORD_0, etc.) for 2nd texture coord. 
    RS_CASE(D3DRS_WRAP2);   // D3DWRAP* flags (D3DWRAP_U, D3DWRAPCOORD_0, etc.) for 3rd texture coord. 
    RS_CASE(D3DRS_WRAP3);   // D3DWRAP* flags (D3DWRAP_U, D3DWRAPCOORD_0, etc.) for 4th texture coord. 
    RS_CASE(D3DRS_LIGHTING);   // TRUE to enable lighting
    RS_CASE(D3DRS_SPECULARENABLE);   // TRUE to enable specular 
    RS_CASE(D3DRS_LOCALVIEWER);   // TRUE to enable camera-relative specular highlights
    RS_CASE(D3DRS_COLORVERTEX);   // TRUE to enable per-vertex color
    RS_CASE(D3DRS_BACKSPECULARMATERIALSOURCE);   // D3DMATERIALCOLORSOURCE (Xbox extension)
    RS_CASE(D3DRS_BACKDIFFUSEMATERIALSOURCE);   // D3DMATERIALCOLORSOURCE (Xbox extension)
    RS_CASE(D3DRS_BACKAMBIENTMATERIALSOURCE);   // D3DMATERIALCOLORSOURCE (Xbox extension)
    RS_CASE(D3DRS_BACKEMISSIVEMATERIALSOURCE);   // D3DMATERIALCOLORSOURCE (Xbox extension)
    RS_CASE(D3DRS_SPECULARMATERIALSOURCE);  // D3DMATERIALCOLORSOURCE 
    RS_CASE(D3DRS_DIFFUSEMATERIALSOURCE);  // D3DMATERIALCOLORSOURCE 
    RS_CASE(D3DRS_AMBIENTMATERIALSOURCE);  // D3DMATERIALCOLORSOURCE 
    RS_CASE(D3DRS_EMISSIVEMATERIALSOURCE);  // D3DMATERIALCOLORSOURCE 
    RS_CASE(D3DRS_BACKAMBIENT);  // D3DCOLOR (Xbox extension)
    RS_CASE(D3DRS_AMBIENT);  // D3DCOLOR 
    RS_CASE_FLOAT(D3DRS_POINTSIZE);  // float point size 
    RS_CASE_FLOAT(D3DRS_POINTSIZE_MIN);  // float point size min threshold 
    RS_CASE(D3DRS_POINTSPRITEENABLE);  // TRUE to enable point sprites
    RS_CASE(D3DRS_POINTSCALEENABLE);  // TRUE to enable point size scaling
    RS_CASE_FLOAT(D3DRS_POINTSCALE_A);  // float point attenuation A value 
    RS_CASE_FLOAT(D3DRS_POINTSCALE_B);  // float point attenuation B value 
    RS_CASE_FLOAT(D3DRS_POINTSCALE_C);  // float point attenuation C value 
    RS_CASE_FLOAT(D3DRS_POINTSIZE_MAX);  // float point size max threshold 
    RS_CASE(D3DRS_PATCHEDGESTYLE);  // D3DPATCHEDGESTYLE
    RS_CASE(D3DRS_PATCHSEGMENTS);  // DWORD number of segments per edge when drawing patches
    RS_CASE(D3DRS_SWAPFILTER);

    // Complex state that has immediate processing:

    RS_CASE(D3DRS_PSTEXTUREMODES);  // Pixel shader, Texture addressing modes (Xbox extension)
    RS_CASE(D3DRS_VERTEXBLEND);  // D3DVERTEXBLENDFLAGS
    RS_CASE(D3DRS_FOGCOLOR);  // D3DCOLOR 
    RS_CASE_D3DFILLMODE(D3DRS_FILLMODE);  // D3DFILLMODE        
    RS_CASE_D3DFILLMODE(D3DRS_BACKFILLMODE);  // D3DFILLMODE (Xbox extension)
    RS_CASE(D3DRS_TWOSIDEDLIGHTING);  // TRUE to enable two-sided lighting (Xbox extension)
    RS_CASE(D3DRS_NORMALIZENORMALS);  // TRUE to enable automatic normalization
    RS_CASE(D3DRS_ZENABLE);  // D3DZBUFFERTYPE (or TRUE/FALSE for legacy) 
    RS_CASE(D3DRS_STENCILENABLE);  // TRUE to enable stenciling
    RS_CASE_D3DSTENCILOP(D3DRS_STENCILFAIL);  // D3DSTENCILOP to do if stencil test fails 
    RS_CASE_D3DFRONT(D3DRS_FRONTFACE);  // D3DFRONT (Xbox extension)
    RS_CASE_D3DCULL(D3DRS_CULLMODE);  // D3DCULL 
    RS_CASE(D3DRS_TEXTUREFACTOR);  // D3DCOLOR used for multi-texture blend 
    RS_CASE(D3DRS_ZBIAS);  // LONG Z bias 
    RS_CASE_D3DLOGICOP(D3DRS_LOGICOP);  // D3DLOGICOP (Xbox extension)
    RS_CASE(D3DRS_EDGEANTIALIAS);  // TRUE to enable edge antialiasing (Xbox extension)
    RS_CASE(D3DRS_MULTISAMPLEANTIALIAS);  // TRUE to enable multisample antialiasing
    RS_CASE(D3DRS_MULTISAMPLEMASK);  // DWORD per-pixel and sample enable/disable
    RS_CASE(D3DRS_MULTISAMPLEMODE);  // D3DMULTISAMPLEMODE for the backbuffer (Xbox extension)
    RS_CASE(D3DRS_MULTISAMPLERENDERTARGETMODE); // D3DMULTISAMPLEMODE for non-backbuffer render targets (Xbox extension)
    RS_CASE_D3DCMPFUNC(D3DRS_SHADOWFUNC);  // D3DCMPFUNC (Xbox extension)
    RS_CASE_FLOAT(D3DRS_LINEWIDTH);  // float (Xbox extension)
    RS_CASE(D3DRS_DXT1NOISEENABLE);  // TRUE to enable DXT1 decompression noise (Xbox extension)
    RS_CASE(D3DRS_YUVENABLE);  // TRUE to enable use of D3DFMT_YUY2 and D3DFMT_UYVY texture formats
    RS_CASE(D3DRS_OCCLUSIONCULLENABLE);  // TRUE to enable Z occlusion culling
    RS_CASE(D3DRS_STENCILCULLENABLE);  // TRUE to enable stencil culling
    RS_CASE(D3DRS_ROPZCMPALWAYSREAD);  // TRUE to always read target packet when Z enabled
    RS_CASE(D3DRS_ROPZREAD);  // TRUE to always read Z
    RS_CASE(D3DRS_DONOTCULLUNCOMPRESSED);  // TRUE to never attempt occlusion culling (stencil or Z) on uncompressed packets
    default:
        sprintf(buf, "0x%x", value);
        return buf;
    }
#undef RS_CASE  
#undef RS_CASE_FLOAT
#undef RS_CASE_D3DBLEND
#undef RS_CASE_D3DBLENDOP
#undef RS_CASE_D3DCMPFUNC
#undef RS_CASE_D3DCULL
#undef RS_CASE_D3DFILLMODE
#undef RS_CASE_D3DFRONT
#undef RS_CASE_D3DLOGICOP
#undef RS_CASE_D3DSTENCILOP
#undef RS_CASE_D3DSHADEMODE
}
    
CHAR *DebugTSS(DWORD tss)
{
#define TSS_CASE(TSS) case TSS: return #TSS 
    switch (tss)
    {
    TSS_CASE(D3DTSS_COLOROP);  // D3DTEXTUREOP - per-stage blending controls for color channels 
    TSS_CASE(D3DTSS_COLORARG0);  // D3DTA_* (D3DTA_TEXTURE etc.) third arg for triadic ops 
    TSS_CASE(D3DTSS_COLORARG1);  // D3DTA_* (D3DTA_TEXTURE etc.) texture arg
    TSS_CASE(D3DTSS_COLORARG2);  // D3DTA_* (D3DTA_TEXTURE etc.) texture arg 
    TSS_CASE(D3DTSS_ALPHAOP);  // D3DTEXTUREOP - per-stage blending controls for alpha channel 
    TSS_CASE(D3DTSS_ALPHAARG0);  // D3DTA_* (D3DTA_TEXTURE etc.) third arg for triadic ops 
    TSS_CASE(D3DTSS_ALPHAARG1);  // D3DTA_* (D3DTA_TEXTURE etc.) texture arg
    TSS_CASE(D3DTSS_ALPHAARG2);  // D3DTA_* (D3DTA_TEXTURE etc.) texture arg) 
    TSS_CASE(D3DTSS_RESULTARG);  // D3DTA_* (D3DTA_TEXTURE etc.) arg for result (CURRENT or TEMP) 
    TSS_CASE(D3DTSS_TEXTURETRANSFORMFLAGS);  // D3DTEXTURETRANSFORMFLAGS controls texture transform 
    TSS_CASE(D3DTSS_ADDRESSU);  // D3DTEXTUREADDRESS for U coordinate 
    TSS_CASE(D3DTSS_ADDRESSV);  // D3DTEXTUREADDRESS for V coordinate 
    TSS_CASE(D3DTSS_ADDRESSW);  // D3DTEXTUREADDRESS for W coordinate 
    TSS_CASE(D3DTSS_MAGFILTER);  // D3DTEXF_* (D3DTEXF_LINEAR etc.) filter to use for magnification 
    TSS_CASE(D3DTSS_MINFILTER);  // D3DTEXF_* (D3DTEXF_LINEAR etc.) filter to use for minification 
    TSS_CASE(D3DTSS_MIPFILTER);  // D3DTEXF_* (D3DTEXF_LINEAR etc.) filter to use between mipmaps during minification 
    TSS_CASE(D3DTSS_MIPMAPLODBIAS);  // float mipmap LOD bias 
    TSS_CASE(D3DTSS_MAXMIPLEVEL);  // DWORD 0..(n-1) LOD index of largest map to use (0 == largest) 
    TSS_CASE(D3DTSS_MAXANISOTROPY);  // DWORD maximum anisotropy 
    TSS_CASE(D3DTSS_COLORKEYOP);  // D3DTEXTURECOLORKEYOP (Xbox extension)
    TSS_CASE(D3DTSS_COLORSIGN);  // D3DTSIGN_* (D3DTSIGN_ASIGNED etc.) for color channels (xbox extension)
    TSS_CASE(D3DTSS_ALPHAKILL);  // D3DTEXTUREALPHAKILL (Xbox extension)
    TSS_CASE(D3DTSS_BUMPENVMAT00);  // float (bump mapping matrix) 
    TSS_CASE(D3DTSS_BUMPENVMAT01);  // float (bump mapping matrix) 
    TSS_CASE(D3DTSS_BUMPENVMAT11);  // float (bump mapping matrix) 
    TSS_CASE(D3DTSS_BUMPENVMAT10);  // float (bump mapping matrix) 
    TSS_CASE(D3DTSS_BUMPENVLSCALE);  // float scale for bump map luminance 
    TSS_CASE(D3DTSS_BUMPENVLOFFSET);  // float offset for bump map luminance 
    TSS_CASE(D3DTSS_TEXCOORDINDEX);  // DWORD identifies which set of texture coordinates index this texture 
    TSS_CASE(D3DTSS_BORDERCOLOR);  // D3DCOLOR 
    TSS_CASE(D3DTSS_COLORKEYCOLOR);  // D3DCOLOR value for color key (Xbox extension)
    default:
        static CHAR buf[100];
        sprintf(buf, "<unknown texture stage state %d>", tss);
        return buf;
    }
#undef TSS_CASE
}

static CHAR *DebugD3DCOLORKEYOP(DWORD value)
{
#define D3DCOLORKEYOP_CASE(COLORKEYOP) case COLORKEYOP: return #COLORKEYOP
	switch (value)
	{
    D3DCOLORKEYOP_CASE(D3DTCOLORKEYOP_DISABLE);
    D3DCOLORKEYOP_CASE(D3DTCOLORKEYOP_ALPHA);
    D3DCOLORKEYOP_CASE(D3DTCOLORKEYOP_RGBA);
    D3DCOLORKEYOP_CASE(D3DTCOLORKEYOP_KILL);
	default:
        static CHAR buf[100];
        sprintf(buf, "<unknown D3DCOLORKEYOP 0x%x>", value);
        return buf;
	}
#undef D3DCOLORKEYOP_CASE
}

static CHAR *DebugD3DTEXTUREOP(DWORD value)
{
#define D3DTEXTUREOP_CASE(TOP) case TOP: return #TOP
	switch ((D3DTEXTUREOP)value)
	{
    D3DTEXTUREOP_CASE(D3DTOP_DISABLE);
    D3DTEXTUREOP_CASE(D3DTOP_SELECTARG1);
    D3DTEXTUREOP_CASE(D3DTOP_SELECTARG2);
    D3DTEXTUREOP_CASE(D3DTOP_MODULATE);
    D3DTEXTUREOP_CASE(D3DTOP_MODULATE2X);
    D3DTEXTUREOP_CASE(D3DTOP_MODULATE4X);
    D3DTEXTUREOP_CASE(D3DTOP_ADD);
    D3DTEXTUREOP_CASE(D3DTOP_ADDSIGNED);
    D3DTEXTUREOP_CASE(D3DTOP_ADDSIGNED2X);
    D3DTEXTUREOP_CASE(D3DTOP_SUBTRACT);
    D3DTEXTUREOP_CASE(D3DTOP_ADDSMOOTH);
    D3DTEXTUREOP_CASE(D3DTOP_BLENDDIFFUSEALPHA);
    D3DTEXTUREOP_CASE(D3DTOP_BLENDTEXTUREALPHA);
    D3DTEXTUREOP_CASE(D3DTOP_BLENDFACTORALPHA);
    D3DTEXTUREOP_CASE(D3DTOP_BLENDTEXTUREALPHAPM);
    D3DTEXTUREOP_CASE(D3DTOP_BLENDCURRENTALPHA);
    D3DTEXTUREOP_CASE(D3DTOP_PREMODULATE);
    D3DTEXTUREOP_CASE(D3DTOP_MODULATEALPHA_ADDCOLOR);
    D3DTEXTUREOP_CASE(D3DTOP_MODULATECOLOR_ADDALPHA);
    D3DTEXTUREOP_CASE(D3DTOP_MODULATEINVALPHA_ADDCOLOR);
    D3DTEXTUREOP_CASE(D3DTOP_MODULATEINVCOLOR_ADDALPHA);
    D3DTEXTUREOP_CASE(D3DTOP_DOTPRODUCT3);
    D3DTEXTUREOP_CASE(D3DTOP_MULTIPLYADD);
    D3DTEXTUREOP_CASE(D3DTOP_LERP);
    D3DTEXTUREOP_CASE(D3DTOP_BUMPENVMAP);
    D3DTEXTUREOP_CASE(D3DTOP_BUMPENVMAPLUMINANCE);
	default:
        static CHAR buf[100];
        sprintf(buf, "<unknown D3DTEXTUREOP 0x%x>", value);
        return buf;
	}
#undef D3DTEXTUREOP_CASE	
}

static CHAR *DebugD3DTA(DWORD value)
{
#define D3DTA_CASE(TA) case TA: return #TA
	switch (value)
	{
	D3DTA_CASE(D3DTA_DIFFUSE);
	D3DTA_CASE(D3DTA_CURRENT);
	D3DTA_CASE(D3DTA_TEXTURE);
	D3DTA_CASE(D3DTA_TFACTOR);
	D3DTA_CASE(D3DTA_SPECULAR);
	D3DTA_CASE(D3DTA_TEMP);
	
	D3DTA_CASE(D3DTA_DIFFUSE | D3DTA_COMPLEMENT);
	D3DTA_CASE(D3DTA_CURRENT | D3DTA_COMPLEMENT);
	D3DTA_CASE(D3DTA_TEXTURE | D3DTA_COMPLEMENT);
	D3DTA_CASE(D3DTA_TFACTOR | D3DTA_COMPLEMENT);
	D3DTA_CASE(D3DTA_SPECULAR | D3DTA_COMPLEMENT);
	D3DTA_CASE(D3DTA_TEMP | D3DTA_COMPLEMENT);
	
	D3DTA_CASE(D3DTA_DIFFUSE | D3DTA_ALPHAREPLICATE);
	D3DTA_CASE(D3DTA_CURRENT | D3DTA_ALPHAREPLICATE);
	D3DTA_CASE(D3DTA_TEXTURE | D3DTA_ALPHAREPLICATE);
	D3DTA_CASE(D3DTA_TFACTOR | D3DTA_ALPHAREPLICATE);
	D3DTA_CASE(D3DTA_SPECULAR | D3DTA_ALPHAREPLICATE);
	D3DTA_CASE(D3DTA_TEMP | D3DTA_ALPHAREPLICATE);
	
	D3DTA_CASE(D3DTA_DIFFUSE | D3DTA_COMPLEMENT | D3DTA_ALPHAREPLICATE);
	D3DTA_CASE(D3DTA_CURRENT | D3DTA_COMPLEMENT | D3DTA_ALPHAREPLICATE);
	D3DTA_CASE(D3DTA_TEXTURE | D3DTA_COMPLEMENT | D3DTA_ALPHAREPLICATE);
	D3DTA_CASE(D3DTA_TFACTOR | D3DTA_COMPLEMENT | D3DTA_ALPHAREPLICATE);
	D3DTA_CASE(D3DTA_SPECULAR | D3DTA_COMPLEMENT | D3DTA_ALPHAREPLICATE);
	D3DTA_CASE(D3DTA_TEMP | D3DTA_COMPLEMENT | D3DTA_ALPHAREPLICATE);
	
	default:
        static CHAR buf[100];
        sprintf(buf, "<unknown D3DTA 0x%x>", value);
        return buf;
	}
#undef D3DTA_CASE
}

static CHAR *DebugD3DTEXTUREADDRESS(DWORD value)
{
#define D3DTEXTUREADDRESS_CASE(TOP) case TOP: return #TOP
	switch ((D3DTEXTUREADDRESS)value)
	{
	D3DTEXTUREADDRESS_CASE(D3DTADDRESS_WRAP);
	D3DTEXTUREADDRESS_CASE(D3DTADDRESS_MIRROR);
	D3DTEXTUREADDRESS_CASE(D3DTADDRESS_CLAMP);
	D3DTEXTUREADDRESS_CASE(D3DTADDRESS_BORDER);
	D3DTEXTUREADDRESS_CASE(D3DTADDRESS_CLAMPTOEDGE);
	default:
        static CHAR buf[100];
        sprintf(buf, "<unknown D3DTEXTUREADDRESS 0x%x>", value);
        return buf;
	}
#undef D3DTEXTUREADDRESS_CASE	
}

static CHAR *DebugD3DTEXF(DWORD value)
{
#define D3DTEXF_CASE(TEXF) case TEXF: return #TEXF
	switch (value)
	{
    D3DTEXF_CASE(D3DTEXF_NONE);
    D3DTEXF_CASE(D3DTEXF_POINT);
    D3DTEXF_CASE(D3DTEXF_LINEAR);
    D3DTEXF_CASE(D3DTEXF_ANISOTROPIC);
    D3DTEXF_CASE(D3DTEXF_QUINCUNX);
    D3DTEXF_CASE(D3DTEXF_GAUSSIANCUBIC);
	default:
        static CHAR buf[100];
        sprintf(buf, "<unknown D3DTEXF 0x%x>", value);
        return buf;
	}
#undef D3DTEXF_CASE
}

CHAR *DebugTSSValue(DWORD tss, DWORD value)
{
    const int c_nbuf = 5;
    static CHAR s_rbuf[c_nbuf][100];
    static int s_ibuf = 0;
    CHAR *buf = s_rbuf[s_ibuf];
    s_ibuf++; if (s_ibuf >= c_nbuf) s_ibuf = 0;
#define TSS_CASE(TSS) case TSS: sprintf(buf, "0x%x", value); return buf
#define TSS_CASE_FLOAT(TSS) case TSS:   sprintf(buf, "%g", *((float*) (&value))); return buf
#define TSS_CASE_D3DCOLORKEYOP(TSS) case TSS: return DebugD3DCOLORKEYOP(value)
#define TSS_CASE_D3DTEXTUREOP(TSS) case TSS: return DebugD3DTEXTUREOP(value)
#define TSS_CASE_D3DTA(TSS) case TSS: return DebugD3DTA(value)
#define TSS_CASE_D3DTEXTUREADDRESS(TSS) case TSS: return DebugD3DTEXTUREADDRESS(value)
#define TSS_CASE_D3DTEXF(TSS) case TSS: return DebugD3DTEXF(value)
    switch (tss)
    {
    TSS_CASE_D3DTEXTUREOP(D3DTSS_COLOROP);  // D3DTEXTUREOP - per-stage blending controls for color channels 
    TSS_CASE_D3DTA(D3DTSS_COLORARG0);  // D3DTA_* (D3DTA_TEXTURE etc.) third arg for triadic ops 
    TSS_CASE_D3DTA(D3DTSS_COLORARG1);  // D3DTA_* (D3DTA_TEXTURE etc.) texture arg
    TSS_CASE_D3DTA(D3DTSS_COLORARG2);  // D3DTA_* (D3DTA_TEXTURE etc.) texture arg 
    TSS_CASE_D3DTEXTUREOP(D3DTSS_ALPHAOP);  // D3DTEXTUREOP - per-stage blending controls for alpha channel 
    TSS_CASE_D3DTA(D3DTSS_ALPHAARG0);  // D3DTA_* (D3DTA_TEXTURE etc.) third arg for triadic ops 
    TSS_CASE_D3DTA(D3DTSS_ALPHAARG1);  // D3DTA_* (D3DTA_TEXTURE etc.) texture arg
    TSS_CASE_D3DTA(D3DTSS_ALPHAARG2);  // D3DTA_* (D3DTA_TEXTURE etc.) texture arg) 
    TSS_CASE_D3DTA(D3DTSS_RESULTARG);  // D3DTA_* (D3DTA_TEXTURE etc.) arg for result (CURRENT or TEMP) 
    TSS_CASE(D3DTSS_TEXTURETRANSFORMFLAGS);  // D3DTEXTURETRANSFORMFLAGS controls texture transform 
    TSS_CASE_D3DTEXTUREADDRESS(D3DTSS_ADDRESSU);  // D3DTEXTUREADDRESS for U coordinate 
    TSS_CASE_D3DTEXTUREADDRESS(D3DTSS_ADDRESSV);  // D3DTEXTUREADDRESS for V coordinate 
    TSS_CASE_D3DTEXTUREADDRESS(D3DTSS_ADDRESSW);  // D3DTEXTUREADDRESS for W coordinate 
    TSS_CASE_D3DTEXF(D3DTSS_MAGFILTER);  // D3DTEXF_* (D3DTEXF_LINEAR etc.) filter to use for magnification 
    TSS_CASE_D3DTEXF(D3DTSS_MINFILTER);  // D3DTEXF_* (D3DTEXF_LINEAR etc.) filter to use for minification 
    TSS_CASE_D3DTEXF(D3DTSS_MIPFILTER);  // D3DTEXF_* (D3DTEXF_LINEAR etc.) filter to use between mipmaps during minification 
    TSS_CASE_FLOAT(D3DTSS_MIPMAPLODBIAS);  // float mipmap LOD bias 
    TSS_CASE(D3DTSS_MAXMIPLEVEL);  // DWORD 0..(n-1) LOD index of largest map to use (0 == largest) 
    TSS_CASE(D3DTSS_MAXANISOTROPY);  // DWORD maximum anisotropy 
    TSS_CASE(D3DTSS_COLORKEYOP);  // D3DTEXTURECOLORKEYOP (Xbox extension)
    TSS_CASE(D3DTSS_COLORSIGN);  // D3DTSIGN_* (D3DTSIGN_ASIGNED etc.) for color channels (xbox extension)
    TSS_CASE(D3DTSS_ALPHAKILL);  // D3DTEXTUREALPHAKILL (Xbox extension)
    TSS_CASE_FLOAT(D3DTSS_BUMPENVMAT00);  // float (bump mapping matrix) 
    TSS_CASE_FLOAT(D3DTSS_BUMPENVMAT01);  // float (bump mapping matrix) 
    TSS_CASE_FLOAT(D3DTSS_BUMPENVMAT11);  // float (bump mapping matrix) 
    TSS_CASE_FLOAT(D3DTSS_BUMPENVMAT10);  // float (bump mapping matrix) 
    TSS_CASE_FLOAT(D3DTSS_BUMPENVLSCALE);  // float scale for bump map luminance 
    TSS_CASE_FLOAT(D3DTSS_BUMPENVLOFFSET);  // float offset for bump map luminance 
    TSS_CASE(D3DTSS_TEXCOORDINDEX);  // DWORD identifies which set of texture coordinates index this texture 
    TSS_CASE(D3DTSS_BORDERCOLOR);  // D3DCOLOR 
    TSS_CASE(D3DTSS_COLORKEYCOLOR);  // D3DCOLOR value for color key (Xbox extension)
    default:
        sprintf(buf, "<unknown texture stage state %d>", tss);
        return buf;
    }
#undef TSS_CASE
#undef TSS_CASE_FLOAT
#undef TSS_CASE_D3DTEXTUREOP
#undef TSS_CASE_D3DTA
#undef TSS_CASE_D3DTEXTUREADDRESS
#undef TSS_CASE_D3DTEXF
}
                                 


//-----------------------------------------------------------------------------
// Name: g_strDebugRenderStateFile
// Desc: Name of file to dump render states to.
//-----------------------------------------------------------------------------
extern CHAR *g_strDebugRenderStateFile = "D:\\DebugRenderState.xdx";




//-----------------------------------------------------------------------------
// Name: DebugRenderState
// Desc: Dumps current render state settings to a file. 
//-----------------------------------------------------------------------------
HRESULT __cdecl DebugRenderState()
{
    // Make sure the GPU is done before proceeding
    D3DDevice_BlockUntilIdle();
    
    // Open the file
    FILE *fp = fopen(g_strDebugRenderStateFile, "wb");
    if (fp == NULL)
        return E_FAIL;
    CHAR buf[4000];
    setvbuf(fp, buf, _IOFBF, sizeof(buf));  // use a larger buffer than the default

    // Dump all the render states in XDX format
    fprintf(fp, "<?xml version=\"1.0\" ?>\n");
    fprintf(fp, "<XDX version=\"0.2\" >\n");
    fprintf(fp, "<Material>\n");
    fprintf(fp, "<RenderState\n");
    DWORD i;
    DWORD dw;
    for (i = D3DRS_PS_MAX; i < D3DRS_MAX; i++)
    {
        D3DDevice_GetRenderState((D3DRENDERSTATETYPE) i, &dw);
        fprintf(fp, "\t%s=\"%s\"\n", DebugRS(i), DebugRSValue(i, dw));
    }
    fprintf(fp, "\t>\n");
#define D3DTSS_MAX_ACTUAL 31    
    for (DWORD stage = 0; stage < D3DTSS_MAXSTAGES; stage++)
    {
        fprintf(fp, "\t<TextureState Stage=\"%d\"\n", stage);
        for (i = 0; i < D3DTSS_MAX_ACTUAL; i++)
        {
            D3DDevice_GetTextureStageState(stage, (D3DTEXTURESTAGESTATETYPE) i, &dw);
            fprintf(fp, "\t\t%s=\"%s\"\n", DebugTSS(i), DebugTSSValue(i, dw));
        }
        fprintf(fp, "\t\t/>\n");
    }
    fprintf(fp, "</RenderState>\n");
    fprintf(fp, "</Material>\n");
    fprintf(fp, "</XDX>\n");
    fclose(fp);
    return S_OK;
}
