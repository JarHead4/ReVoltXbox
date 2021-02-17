//-----------------------------------------------------------------------------
// File: XBResource.cpp
//
// Desc: Loads resources from an XPR (Xbox Packed Resource) file.  
//
// Hist: 2001.03.12 - New for April XDK release
//       2001.12.28 - Added asynchronous reading and effect types.
//       2002.01.02 - Symbol table included in resource bundle.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include <xtl.h>
#include <xgraphics.h>
#include <stdio.h>
#include "XBResource.h"




//-----------------------------------------------------------------------------
// Name: XBResource_SizeOf()
// Desc: Determines the byte size of a D3DResource
//-----------------------------------------------------------------------------
DWORD XBResource_SizeOf( LPDIRECT3DRESOURCE8 pResource )
{
    switch( pResource->GetType() )
    {
        case D3DRTYPE_TEXTURE:       return sizeof(D3DTexture);
        case D3DRTYPE_VOLUMETEXTURE: return sizeof(D3DVolumeTexture);
        case D3DRTYPE_CUBETEXTURE:   return sizeof(D3DCubeTexture);
        case D3DRTYPE_VERTEXBUFFER:  return sizeof(D3DVertexBuffer);
        case D3DRTYPE_INDEXBUFFER:   return sizeof(D3DIndexBuffer);
        case D3DRTYPE_PALETTE:       return sizeof(D3DPalette);
    }
    return 0;
}




//-----------------------------------------------------------------------------
// Name: CXBPackedResource()
// Desc: Constructor
//-----------------------------------------------------------------------------
CXBPackedResource::CXBPackedResource()
{
    m_pSysMemData    = NULL;
    m_pVidMemData    = NULL;
    m_dwNumResources = 0L;
    m_pResourceTags  = NULL;
}




//-----------------------------------------------------------------------------
// Name: ~CXBPackedResource()
// Desc: Destructor
//-----------------------------------------------------------------------------
CXBPackedResource::~CXBPackedResource()
{
    Destroy();
}




//-----------------------------------------------------------------------------
// Name: GetData()
// Desc: Loads all the texture resources from the given XPR.
//-----------------------------------------------------------------------------
VOID* CXBPackedResource::GetData( const CHAR* strName ) const
{
    if( NULL==m_pResourceTags || NULL==strName )
        return NULL;

    for( DWORD i=0; i<m_dwNumResources; i++ )
    {
        if( !_stricmp( strName, m_pResourceTags[i].strName ) )
            return &m_pSysMemData[m_pResourceTags[i].dwOffset];
    }

    return NULL;
}




//-----------------------------------------------------------------------------
// Name: Create()
// Desc: Loads all the texture resources from the given XPR.
//-----------------------------------------------------------------------------
HRESULT CXBPackedResource::Create( const CHAR *strResourcePath, 
                                   DWORD dwNumResources,
                                   XBRESOURCETAG *pResourceTags )
{
    // Open the fileto read the XPR headers
    FILE* file = fopen( strResourcePath, "rb" );
    if( NULL == file )
        return E_FAIL;

    // Read in and verify the XPR magic header
    XPR_HEADER xprh;
    fread( &xprh, sizeof(XPR_HEADER), 1, file );
    if( xprh.dwMagic != XPR_MAGIC_VALUE )
    {
        OutputDebugString( "Invalid Xbox Packed Resource file" );
        fclose( file );
        return E_INVALIDARG;
    }

    // Compute memory requirements
    DWORD dwSysMemDataSize = xprh.dwHeaderSize - sizeof(XPR_HEADER);
    DWORD dwVidMemDataSize = xprh.dwTotalSize - xprh.dwHeaderSize;

    // Allocate memory
    m_pSysMemData = new BYTE[dwSysMemDataSize];
    m_pVidMemData = (BYTE*)D3D_AllocContiguousMemory( dwVidMemDataSize, D3DTEXTURE_ALIGNMENT );

    // Read in the data from the file
    fread( m_pSysMemData, dwSysMemDataSize, 1, file );
    fread( m_pVidMemData, dwVidMemDataSize, 1, file );

    // Done with the file
    fclose( file );
    
    // Store number of resources and the resource tags
    m_dwNumResources = dwNumResources;
    m_pResourceTags  = pResourceTags;

    // Fixup the resources
    return PatchAll();
}

//-----------------------------------------------------------------------------
// Name: PatchAll()
// Desc: Patch and register all the loaded resources
//-----------------------------------------------------------------------------
HRESULT CXBPackedResource::PatchAll()
{
    // Loop over resources, calling Register()
    BYTE* pData = m_pSysMemData;

    for( DWORD i = 0; i < m_dwNumResources; i++ )
    {
        // Check for userdata
        if( *((DWORD*)pData) & 0x80000000 )
        {
            DWORD dwType = ((DWORD*)pData)[0];
            DWORD dwSize = ((DWORD*)pData)[1];
            pData += sizeof(DWORD) * 2;

            (VOID)dwType; // not used
            pData += dwSize;
        }
        else
        {
            // Get the resource
            LPDIRECT3DRESOURCE8 pResource = (LPDIRECT3DRESOURCE8)pData;
    
            // Register the resource
            pResource->Register( m_pVidMemData );
        
            // Advance the pointer
            pData += XBResource_SizeOf( pResource );
        }
    }
    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: Destroy()
// Desc: Tears down the packed resource data
//-----------------------------------------------------------------------------
VOID CXBPackedResource::Destroy() 
{
    if( m_pSysMemData != NULL )
    {
        delete[] m_pSysMemData;
        m_pSysMemData = NULL;
    }
    if( m_pVidMemData != NULL )
    {
        D3D_FreeContiguousMemory( m_pVidMemData );
        m_pVidMemData = NULL;
    }
    m_dwNumResources = 0L;
    m_pResourceTags  = NULL;
}










//-----------------------------------------------------------------------------
// Name: XBResource()
// Desc: Constructor
//-----------------------------------------------------------------------------
XBResource::XBResource()
{
    m_cbHeaders     = 0;
    m_cbData        = 0;
    m_LoadingState = LOADING_NOTSTARTED;
}

//-----------------------------------------------------------------------------
// Name: ~Resource()
// Desc: Destructor
//-----------------------------------------------------------------------------
XBResource::~XBResource()
{
    Unload();
}

//-----------------------------------------------------------------------------
// Name: PollLoadingState()
// Desc: Check for IO completion
//-----------------------------------------------------------------------------
LOADINGSTATE XBResource::PollLoadingState()
{
    if ( m_LoadingState == LOADING_HEADER
         || m_LoadingState == LOADING_DATA )
    {
        if ( HasOverlappedIoCompleted( &m_overlapped ) )
        {
            HRESULT hr = OnIOComplete();
            if (FAILED(hr))
                m_LoadingState = LOADING_FAILED;
        }
    }
    return m_LoadingState;
}


//-----------------------------------------------------------------------------
// Name: StartLoading()
// Desc: Loads all the resources from the given XBR. ppResources should 
//       be large enough to hold all the LPDIRECT3DRESOURCE8 pointers.  
//       The read is performed asynchronously, so the data isn't available
//       until OnIOComplete().
//-----------------------------------------------------------------------------
HRESULT XBResource::StartLoading( LPSTR strFileName )
{
    HRESULT     hr = S_OK;
    XPR_HEADER  xprh;
    HANDLE      hfHeader = INVALID_HANDLE_VALUE;
    DWORD       cb;

    // Make sure previous resources are unloaded
    Unload();

    // Read the headers first
    hfHeader = CreateFile( strFileName, GENERIC_READ, FILE_SHARE_READ, NULL, 
                           OPEN_EXISTING, 0, NULL );
    if( hfHeader == INVALID_HANDLE_VALUE )
    {
        hr = E_FAIL;
        goto Done;
    }

    // Verify the XPR magic header
    if( !ReadFile( hfHeader, &xprh, sizeof( XPR_HEADER), &cb, NULL ) )
    {
        hr = E_FAIL; 
        goto Done;
    }
    if( xprh.dwMagic != XPR_MAGIC_VALUE )
    {
        OutputDebugString( "Invalid Xbox Resource file" );
        hr = E_INVALIDARG;
        goto Done;
    }

    // TODO: make header reading asynchronous, too, with LOADING_HEADER state
    
    // Allocate memory for the headers
    m_cbHeaders = xprh.dwHeaderSize - 3 * sizeof( DWORD );
    m_pSysMemData = new BYTE[m_cbHeaders];
    if( !m_pSysMemData )
    {
        hr = E_OUTOFMEMORY;
        goto Done;
    }

    // Read in the headers
    if( !ReadFile( hfHeader, m_pSysMemData, m_cbHeaders, &cb, NULL ) )
    {
        hr = E_FAIL; 
        goto Done;
    }
    CloseHandle( hfHeader );

    // Now read the data
    // File is opened with overlapped i/o and no buffering
    m_hfXPR = CreateFile( strFileName, GENERIC_READ, FILE_SHARE_READ, NULL, 
                          OPEN_EXISTING, FILE_FLAG_OVERLAPPED | FILE_FLAG_NO_BUFFERING, NULL );
    if( m_hfXPR == INVALID_HANDLE_VALUE )
    {
        hr = E_FAIL;
        goto Done;
    }

    // Allocate contiguous memory for the texture data
    m_cbData = xprh.dwTotalSize - xprh.dwHeaderSize;
    m_pVidMemData = (BYTE *)D3D_AllocContiguousMemory( m_cbData, D3DTEXTURE_ALIGNMENT );
    if( !m_pVidMemData )
    {
        hr = E_OUTOFMEMORY;
        goto Done;
    }

    // Set up our overlapped i/o struct
    ZeroMemory( &m_overlapped, sizeof( OVERLAPPED ) );
    m_overlapped.Offset = xprh.dwHeaderSize;

    // Start the read of the texture data
    if( !ReadFile( m_hfXPR, m_pVidMemData, m_cbData, &cb, &m_overlapped ) )
    {
        if( GetLastError() == ERROR_IO_PENDING )
            m_LoadingState = LOADING_DATA;
        else
        {
            // Error we weren't expecting
            hr = E_FAIL;
            goto Done;
        }
    }

Done:
    // Lots of cleanup to do
    if( INVALID_HANDLE_VALUE != hfHeader )
        CloseHandle( hfHeader );

    // If we had an error condition, we need to 
    // free memory and close the XPR file
    if( FAILED( hr ) )
    {
        m_LoadingState = LOADING_FAILED;
        if( INVALID_HANDLE_VALUE != m_hfXPR )
            CloseHandle( m_hfXPR );

        delete[] m_pSysMemData;
        if( m_pVidMemData )
            D3D_FreeContiguousMemory( m_pVidMemData );
    }

    return hr;
}


//-----------------------------------------------------------------------------
// Name: Unload()
// Desc: Cleans up resource state that is external to the bulk allocation.
//-----------------------------------------------------------------------------
HRESULT XBResource::Unload()
{
    HRESULT hr = S_OK;

    if (m_LoadingState == LOADING_HEADER
        || m_LoadingState == LOADING_DATA )
    {
        // Abort loading
        CloseHandle( m_hfXPR );
        m_hfXPR = INVALID_HANDLE_VALUE;
        m_LoadingState = LOADING_NOTSTARTED;
    }

    // Loop over resources, unloading as needed
    for( UINT i = 0; i < m_dwNumResources; i++ )
    {
        DWORD dwType = m_rdwTypes[i];
        BYTE *pResource = m_rpResources[i];
        HRESULT hr2 = this->Cleanup(dwType, pResource);
        if (FAILED(hr2))
            hr = hr2;   // keep final result

    }
    Destroy();      // call CXBPackedResource cleanup routine
    return hr;
}





//-----------------------------------------------------------------------------
// Name: OnIOComplete()
// Desc: Called when async i/o is complete, so that we can copy the
//       texture data to video memory and register the resources.
//-----------------------------------------------------------------------------
HRESULT XBResource::OnIOComplete()
{
    HRESULT hr = S_OK;
    BYTE      * pHeader;

   // Done with async XPR load
    CloseHandle( m_hfXPR );
    m_hfXPR = INVALID_HANDLE_VALUE;

    // TODO: integrate this with the PatchAll base class
    
    // Fill in type and resource index arrays
    pHeader = m_pSysMemData;
    UINT i;
    for( i = 0; i < MAX_NUM_RESOURCES; i++ )
    {
        DWORD dwType = *(DWORD*)pHeader;
        // Check for end token
        if ( dwType == 0xffffffff )
            break;
        // Check for userdata, which has size in header
        else if ( dwType & 0x80000000 )
        {
            DWORD dwSize = ((DWORD*)pHeader)[1];
            pHeader += 8;
            m_rpResources[i] = pHeader; // pointer is to actual user data, not wrapper type and size
            m_rdwTypes[i] = dwType;     // keep the type in a separate array
            pHeader += dwSize;
        }
        else
        {
            // Standard Resource
            m_rdwTypes[i] = dwType & D3DCOMMON_TYPE_MASK;
            m_rpResources[i] = pHeader;
            pHeader += XBResource_SizeOf( (LPDIRECT3DRESOURCE8)pHeader ); // standard resource types use standard sizes
        }
    }
    m_dwNumResources = i; // Number of loaded resources

    // Call the patch routine on all the resources
    for (i = 0; i < m_dwNumResources; i++)
    {
        hr = this->Patch(m_rdwTypes[i], m_rpResources[i]);
        if (FAILED(hr))
            break;
    }
    
    if (FAILED(hr))
        m_LoadingState = LOADING_FAILED;
    else
        m_LoadingState = LOADING_DONE;
    return hr;
}





//-----------------------------------------------------------------------------
// Name: Patch()
// Desc: Convert file offsets and resources indices to pointers.
//-----------------------------------------------------------------------------

// TODO: Fix xbrc to get rid of indices.  Just use file offsets, and
// full symbol table for importing from other resource bundles.
// Remove the special case code below and use a list of offsets that
// need to be patched.

HRESULT XBResource::Patch(DWORD dwType, // resource type
                          BYTE *pHeader)    // pointer to resource header
{
    if (!(dwType & 0x80000000)) // not user data
    {
        // Texture and VertexBuffer handling
        D3DResource *pResource = (D3DResource *)pHeader;
        pResource->Register( m_pVidMemData );
    }
    else if (dwType == XBR_INDEXBUFFER)
    {
        // Patch index data offset
        D3DIndexBuffer *pIB = (D3DIndexBuffer *)pHeader;
        pIB->Data += (DWORD)pIB;    // convert structure offset to pointer
    }
    else if (dwType == XBR_VERTEXSHADER)
    {
        VertexShader *pVertexShader = (VertexShader *)pHeader;
        pVertexShader->Declaration = (DWORD *)(pHeader + (DWORD)pVertexShader->Declaration);
        pVertexShader->Function = (DWORD *)(pHeader + (DWORD)pVertexShader->Function);
        if (pVertexShader->pVertexShader != 0)
        {
            DWORD Index = *((DWORD *)&(pVertexShader->pVertexShader)) - 1;
            pVertexShader->pVertexShader = (VertexShader *)GetResourceByIndex(Index);
        }
        for (int iStream = 0; iStream < VERTEXSHADER_MAX_STREAMS; iStream++)
        {
            if (pVertexShader->rStreamInput[iStream].pVertexBuffer != 0)
            {
                DWORD Index = *((DWORD *)&(pVertexShader->rStreamInput[iStream].pVertexBuffer)) - 1;
                pVertexShader->rStreamInput[iStream].pVertexBuffer = (D3DVertexBuffer *)GetResourceByIndex(Index);
            }
        }
    }
    else if (dwType == XBR_PIXELSHADER)
    {
        // Pixel shaders are defined inline and do not need patching.
    }
    else if (dwType == XBR_EFFECT)
    {
        // Patch the file offsets to memory offsets
        Effect *pEffect = (Effect *)pHeader;
        if (pEffect->pRenderTarget != 0)
        {
            DWORD Index = *((DWORD *)&(pEffect->pRenderTarget)) - 1;
            pEffect->pRenderTarget = (D3DSurface *)GetResourceByIndex(Index);
        }
        pEffect->rPass = (Pass *)(pHeader + (DWORD)pEffect->rPass); // overwrite rPass
        for (UINT iPass = 0; iPass < pEffect->PassCount; iPass++)
        {
            Pass *pPass = &pEffect->rPass[iPass];
            if (pPass->pVertexShader != 0)
            {
                DWORD Index = *((DWORD *)&(pPass->pVertexShader)) - 1;
                pPass->pVertexShader = (VertexShader *)GetResourceByIndex(Index);
            }
            if (pPass->pPixelShader != 0)
            {
                DWORD Index = *((DWORD *)&(pPass->pPixelShader)) - 1;
                pPass->pPixelShader = (PixelShader *)GetResourceByIndex(Index);
            }
            pPass->rRenderState = (RenderState *)(pHeader + (DWORD)pPass->rRenderState);
            pPass->rTextureStage = (TextureStage *)(pHeader + (DWORD)pPass->rTextureStage);
            for (UINT iTextureStage = 0; iTextureStage < pPass->TextureStageCount; iTextureStage++)
            {
                D3DBaseTexture **ppTexture = &pPass->rTextureStage[iTextureStage].pTexture;
                if (*ppTexture != 0)
                {
                    DWORD Index = *((DWORD *)ppTexture) - 1;
                    *ppTexture = (D3DBaseTexture *)GetResourceByIndex(Index);
                }
                TextureStage *pTextureStage = &pPass->rTextureStage[iTextureStage];
                pTextureStage->rTextureState = (TextureState *)(pHeader + (DWORD)pTextureStage->rTextureState);
            }
            pPass->rDraw = (Draw *)(pHeader + (DWORD)pPass->rDraw);
            for (UINT iDraw = 0; iDraw < pPass->DrawCount; iDraw++)
            {
                D3DIndexBuffer **ppIndexBuffer = &pPass->rDraw[iDraw].pIndexBuffer;
                if (*ppIndexBuffer != 0)
                {
                    DWORD Index = *((DWORD *)ppIndexBuffer) - 1;
                    *ppIndexBuffer = (D3DIndexBuffer *)GetResourceByIndex(Index);
                }
            }
        }
    }
    else if (dwType == XBR_SKELETON)
    {
        // ignore skeletons
    }
    else if (dwType == XBR_ANIMATION)
    {
        // ignore animations
    }
    else if (dwType == XBR_SYMBOLTABLE)
    {
        // Use symbol table only if user did not specify a resource tag array
        if (m_pResourceTags != NULL)
            return S_OK;

        // Get symbol count and resource tag pointer
        DWORD SymbolCount = *(DWORD *)pHeader;
        m_pResourceTags = (XBRESOURCETAG *)(pHeader + sizeof(DWORD));
        
        // Convert file offsets to string pointers
        for (UINT iSymbol = 0; iSymbol < SymbolCount; iSymbol++)
            *(DWORD *)&m_pResourceTags[iSymbol].strName += *(DWORD *)&m_pSysMemData;
    }
    else
    {
        // Assume resource does not need patching.
    }
    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: Cleanup()
// Desc: Called by Unload to release handles, etc. to cleanup the resource before unloading.
//-----------------------------------------------------------------------------
HRESULT XBResource::Cleanup(DWORD dwType, BYTE *pHeader)
{
    // Currently, VertexShaders are the only resource with dangling references
    if (dwType == XBR_VERTEXSHADER)
    {
        VertexShader *pVertexShader = (VertexShader *)pHeader;
        if (pVertexShader->Handle)
            D3DDevice_DeleteVertexShader(pVertexShader->Handle);
    }
    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: Printf
// Desc: Print a formatted string to the debug display
//-----------------------------------------------------------------------------
static VOID Printf(CONST WCHAR *strFormat, ...)
{
    va_list val;
    va_start(val, strFormat);
    const buflen = 1000;
    WCHAR buf[buflen];
    _vsnwprintf(buf, buflen, strFormat, val);
    OutputDebugStringW(buf);
}



//-----------------------------------------------------------------------------
// Name: SetVertexShaderConstant
// Desc: Look through VertexShader Declaration and set constant
//-----------------------------------------------------------------------------
static HRESULT SetVertexShaderConstant(VertexShader *pVertexShader,
                                       INT Register,
                                       CONST VOID *pConstantData,
                                       DWORD ConstantCount)
{
    if (pVertexShader->DeclarationByteCount == 0)
        return E_INVALIDARG;    // no declaration to parse
    DWORD *pToken = (DWORD *)pVertexShader->Declaration;
    while (*pToken != D3DVSD_END() )
    {
        switch (*pToken & D3DVSD_TOKENTYPEMASK)
        {
        case D3DVSD_MAKETOKENTYPE(D3DVSD_TOKEN_CONSTMEM):
        {
            // parse the token
            INT addr = ((*pToken & D3DVSD_CONSTADDRESSMASK) >> D3DVSD_CONSTADDRESSSHIFT) - 96;
            UINT count = (*pToken & D3DVSD_CONSTCOUNTMASK) >> D3DVSD_CONSTCOUNTSHIFT;
            FLOAT *pValue = (float *)(pToken + 1);
            if (Register < (INT)(addr + count) && addr < (INT)(Register + ConstantCount))
            {
                // get the range to set
                INT addrStart = Register;
                if (addrStart < addr)
                    addrStart = addr;
                INT addrEnd = Register + ConstantCount;
                if (addrEnd > (INT)(addr + count))
                    addrEnd = addr + count;
                
                // set the constants
                FLOAT *pValueSrc = (FLOAT *)pConstantData + 4 * (addrStart - Register);
                FLOAT *pValueDst = pValue + 4 * (addrStart - addr);
                memcpy(pValueDst, pValueSrc, 4 * (addrEnd - addrStart) * sizeof(float));
            }
            pToken += 4 * count;
            break;
        }

        // ignore stream and nop tokens
        case D3DVSD_MAKETOKENTYPE(D3DVSD_TOKEN_STREAM):
        case D3DVSD_MAKETOKENTYPE(D3DVSD_TOKEN_STREAMDATA):
        case D3DVSD_MAKETOKENTYPE(D3DVSD_TOKEN_NOP):
            break;
            
        case D3DVSD_MAKETOKENTYPE(D3DVSD_TOKEN_END):
            goto loop_break;
            
        default:
            Printf(L"Vertex shader declaration token (0x%x) not handled.\n", *pToken );
            return E_FAIL;
        }

        // Go to the next token in the stream
        pToken++;
        if ((BYTE *)pToken - (BYTE *)pVertexShader->Declaration > (INT)pVertexShader->DeclarationByteCount)
        {
            OutputDebugString("Vertex shader declaration missing end token.\n");
            goto loop_break;
        }
    }
 loop_break:
    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: ApplyVertexShaderConstants
// Desc: Call D3DDevice_SetVertexShaderConstants on all the constants
//       in the declaration.
//-----------------------------------------------------------------------------
static HRESULT ApplyVertexShaderConstants(VertexShader *pVertexShader,
                                          bool *pbStreamDeclaration)
{
    if (pbStreamDeclaration)
        *pbStreamDeclaration = false;
    if (pVertexShader->DeclarationByteCount == 0)
        return S_FALSE; // no declaration to parse
    static bool bDumpConstants = false; // debug
    DWORD *pToken = (DWORD *)pVertexShader->Declaration;
    while (*pToken != D3DVSD_END() )
    {
        switch (*pToken & D3DVSD_TOKENTYPEMASK)
        {
        case D3DVSD_MAKETOKENTYPE(D3DVSD_TOKEN_CONSTMEM):
        {
            // parse the token
            int addr = ((*pToken & D3DVSD_CONSTADDRESSMASK) >> D3DVSD_CONSTADDRESSSHIFT) - 96;
            UINT count = (*pToken & D3DVSD_CONSTCOUNTMASK) >> D3DVSD_CONSTCOUNTSHIFT;
                    
            // set the constants
            D3DDevice_SetVertexShaderConstant(addr, pToken + 1, count);

            if (bDumpConstants) // debug
            {
                // print out the constants for debugging
                float *pValue = (float *)(pToken + 1);
                for (UINT i = 0; i < count; i++)
                {
                    Printf(L"c%d %g %g %g %g\n",
                           addr + i, 
                           *(float *)&pValue[i*4], 
                           *(float *)&pValue[i*4+1],
                           *(float *)&pValue[i*4+2], 
                           *(float *)&pValue[i*4+3]);
                }
            }
            
            pToken += 4 * count;
            break;
        }
        
        case D3DVSD_MAKETOKENTYPE(D3DVSD_TOKEN_STREAM):
            // Ignore stream tokens
            break;
                    
        case D3DVSD_MAKETOKENTYPE(D3DVSD_TOKEN_STREAMDATA):
            if (pbStreamDeclaration)
                *pbStreamDeclaration = true;
            break;
                    
        case D3DVSD_MAKETOKENTYPE(D3DVSD_TOKEN_NOP):
            break;
            
        case D3DVSD_MAKETOKENTYPE(D3DVSD_TOKEN_END):
            goto loop_break;
            
        default:
            Printf(L"Vertex shader declaration token (0x%x) not handled.\n", *pToken );
            return E_FAIL;
        }

        // Go to the next token in the stream
        pToken++;
        if ((BYTE *)pToken - (BYTE *)pVertexShader->Declaration > (INT)pVertexShader->DeclarationByteCount)
        {
            OutputDebugString("Vertex shader declaration missing end token.\n");
            goto loop_break;
        }
    }
 loop_break:
    bDumpConstants = false;     // debug
    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: SetVertexShader
// Desc: Create a vertex shader from a resource and set it to be current
//-----------------------------------------------------------------------------
HRESULT VertexShader::SetVertexShader()
{
    HRESULT hr;
    VertexShader *pVSRef = this->pVertexShader;
    if (pVSRef == 0)
    {
        // No vertex shader reference, so simply set the shader.
        if (this->Handle == 0)
        {
            // Create the vertex shader
            DWORD flags = 0;
            hr = D3DDevice_CreateVertexShader( this->Declaration, this->Function, &this->Handle, flags );
            if (FAILED(hr))
                return hr;
        }
        // new version of d3d complains if SetVertexShaderInput is active when we
        // call SetVertexShader, so turn it off first
        D3DDevice_SetVertexShaderInput(0, 0, 0);
        D3DDevice_SetVertexShader(this->Handle);

        // Set any vertex shader constants that may be animated
        // TODO: FIX ME!
        // TODO: Fix redundancy for non-animated vertex shader constants.
        bool bStreamDeclaration;
        hr = ApplyVertexShaderConstants(this, &bStreamDeclaration);
        if (FAILED(hr))
            return hr;
        
        // Map vertex buffers to streams
        D3DSTREAM_INPUT rStreamInput[VERTEXSHADER_MAX_STREAMS];
        UINT StreamCount = 0;
        for (UINT iStream = 0; iStream < VERTEXSHADER_MAX_STREAMS; iStream++)
        {
            D3DVertexBuffer *pVB = this->rStreamInput[iStream].pVertexBuffer;
            if (pVB != 0)
            {
                rStreamInput[iStream].VertexBuffer = pVB;
                rStreamInput[iStream].Stride = this->rStreamInput[iStream].Stride;
                rStreamInput[iStream].Offset = this->rStreamInput[iStream].Offset;
                StreamCount = iStream + 1;  // use maximum stream index to set count
            }
            else
            {
                rStreamInput[iStream].VertexBuffer = NULL;
                rStreamInput[iStream].Stride = 0;
                rStreamInput[iStream].Offset = 0;
            }
        }
        if (StreamCount)
        {
            D3DDevice_SetVertexShaderInput(this->Handle, StreamCount, rStreamInput);
        }
    }
    else
    {
        // Lookup vertex shader reference
        
        // TODO: Resolve all this stuff at compile time.

        // We should probably move away from using the vertex shader
        // reference business, and separate out the declaration from
        // the constants and stream mapping.

        // Right now, the only reference supported is a change in the
        // stream mapping and a change in the constants.
        hr = pVSRef->SetVertexShader();
        if (FAILED(hr))
            return hr;
        
        if (this->FunctionByteCount != 0)
            return E_FAIL;

        // Look through our own declaration for constants. For
        // shaders that reference other shaders, this is the
        // way that constants are over-ridden from the default.
        bool bStreamDeclaration = false;    // and check to see if the declaration has changed
        hr = ApplyVertexShaderConstants(this, &bStreamDeclaration );
        if (FAILED(hr))
            return hr;
        if (bStreamDeclaration)
        {
            // TODO: We redeclared the shader declaration, so we need to create a new version of the vertex shader
        }

        // Map vertex buffers to streams
        UINT StreamCount = 0;
        D3DSTREAM_INPUT rStreamInput[VERTEXSHADER_MAX_STREAMS];     // TODO: include stream count in VertexShader class
        for (UINT iStream = 0; iStream < VERTEXSHADER_MAX_STREAMS; iStream++)
        {
            // Override stream mapping using the new vertex shader
            D3DVertexBuffer *pVB = this->rStreamInput[iStream].pVertexBuffer;
            if (pVB == 0)
                pVB = pVSRef->rStreamInput[iStream].pVertexBuffer;
            if (pVB != 0)
            {
                rStreamInput[iStream].VertexBuffer = pVB;
                if (this->rStreamInput[iStream].Stride != 0)
                {
                    // use the new stream
                    rStreamInput[iStream].Stride = this->rStreamInput[iStream].Stride;
                    rStreamInput[iStream].Offset = this->rStreamInput[iStream].Offset;
                }
                else
                {
                    // use the reference stream
                    rStreamInput[iStream].Stride = pVSRef->rStreamInput[iStream].Stride;
                    rStreamInput[iStream].Offset = pVSRef->rStreamInput[iStream].Offset;
                }
                StreamCount = iStream + 1;  // use maximum stream touched as stream count
            }
            else
            {
                rStreamInput[iStream].VertexBuffer = NULL;
                rStreamInput[iStream].Stride = 0;
                rStreamInput[iStream].Offset = 0;
            }
        }
        if (StreamCount > 0)
        {
            D3DDevice_SetVertexShaderInput(pVSRef->Handle, StreamCount, rStreamInput);
        }
    }
    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: DrawEffect
// Desc: Execute the effect
//-----------------------------------------------------------------------------
HRESULT Effect::DrawEffect()
{
    D3DDevice_SetShaderConstantMode(D3DSCM_192CONSTANTS | D3DSCM_NORESERVEDCONSTANTS);

    // ignore pRenderTarget for now
    for (UINT iPass = 0; iPass < this->PassCount; iPass++)
    {
        Pass *pPass = &this->rPass[iPass];

        if (pPass->pVertexShader != 0)
            pPass->pVertexShader->SetVertexShader();

        if (pPass->pPixelShader != NULL)
            D3DDevice_SetPixelShaderProgram(pPass->pPixelShader);

        for (UINT iRS = 0; iRS < pPass->RenderStateCount; iRS++)
        {
            RenderState *pRS = &pPass->rRenderState[iRS];
            D3DDevice_SetRenderStateNotInline((D3DRENDERSTATETYPE)pRS->State, pRS->Value);
        }
        for (UINT iTS = 0; iTS < pPass->TextureStageCount; iTS++)
        {
            TextureStage *pTS = &pPass->rTextureStage[iTS];
            if (pTS->pTexture != NULL)
                D3DDevice_SetTexture( iTS, pTS->pTexture );
            for (UINT iTSS = 0; iTSS < pTS->TextureStateCount; iTSS++)
            {
                TextureState *pTSS = &pTS->rTextureState[iTSS];
                D3DDevice_SetTextureStageStateNotInline(iTS, (D3DTEXTURESTAGESTATETYPE)pTSS->State, pTSS->Value);
            }
        }

        // $TODO FIX THIS: The communication from the app to the draw procedure is not yet worked out.
        {
            extern XGMATRIX g_Proj;
            extern XGVECTOR4 g_vScale;
            extern XGVECTOR4 g_vOffset; 
            D3DDevice_SetVertexShaderConstant(4, &g_Proj, 4);
            D3DDevice_SetVertexShaderConstant(-38, &g_vScale, 1);
            D3DDevice_SetVertexShaderConstant(-37, &g_vOffset, 1);
        }

        for (UINT iDraw = 0; iDraw < pPass->DrawCount; iDraw++)
        {
            Draw *pDraw = &pPass->rDraw[iDraw];
            if (pDraw->pIndexBuffer != NULL)
            {
                const UINT BaseVertexIndex = 0; // TODO: should we include this in the <Draw> tag?
                D3DDevice_SetIndices(pDraw->pIndexBuffer, BaseVertexIndex);
                D3DDevice_DrawIndexedVertices((D3DPRIMITIVETYPE)pDraw->Primitive, pDraw->Count, D3D__IndexData + pDraw->Start);
            }
            else
            {
                D3DDevice_DrawVertices((D3DPRIMITIVETYPE)pDraw->Primitive, pDraw->Start, pDraw->Count);
            }
        }
    }

    D3DDevice_SetShaderConstantMode(D3DSCM_96CONSTANTS);
    return S_OK;
}


//-----------------------------------------------------------------------------
// Name: GetEffect
// Desc: Get an effect by name
//-----------------------------------------------------------------------------
Effect *XBResource::GetEffect( const CHAR* strName ) const
{
    // NOTE: this is different than the rest of the GetVertexBuffer, etc. routines
    // in that we look inside the embedded Effect class for the identifier, rather
    // than using the symbol table.
    
    // $TODO: Replace this with symbol table lookup
    for (DWORD ResourceIndex = 0; ResourceIndex < m_dwNumResources; ResourceIndex++)
    {
        if (m_rdwTypes[ResourceIndex] == XBR_EFFECT)
        {
            Effect *pEffect = (Effect *)m_rpResources[ResourceIndex];
            if (_stricmp(pEffect->Identifier, strName) == 0)
                return pEffect;
        }
    }
    return NULL;
}
