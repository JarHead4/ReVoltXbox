//-----------------------------------------------------------------------------
// File: XBResource.h
//
// Desc: Loads resources from an XPR (Xbox Packed Resource) file.  
//
// Hist: 03.12.01 - New for April XDK release
//       12.28.01 - Added asynchronous reading and user data types.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef XBRESOURCE_H
#define XBRESOURCE_H


//-----------------------------------------------------------------------------
//
//  Resource types
//
//-----------------------------------------------------------------------------

#define XBR_USER_DATA_FLAG 0x80000000

// D3D resources
//#define XBR_SURFACE       D3DRTYPE_SURFACE
//#define XBR_VOLUME        D3DRTYPE_VOLUME
#define XBR_TEXTURE         D3DRTYPE_TEXTURE
#define XBR_VOLUMETEXTURE   D3DRTYPE_VOLUMETEXTURE
#define XBR_CUBETEXTURE     D3DRTYPE_CUBETEXTURE  
#define XBR_VERTEXBUFFER    D3DRTYPE_VERTEXBUFFER
//#define XBR_INDEXBUFFER   D3DRTYPE_INDEXBUFFER
//#define XBR_PUSHBUFFER    D3DRTYPE_PUSHBUFFER
//#define XBR_PALETTE       D3DRTYPE_PALETTE
//#define XBR_FIXUP         D3DRTYPE_FIXUP

// Additional resources
#define XBR_INDEXBUFFER             (XBR_USER_DATA_FLAG|D3DCOMMON_TYPE_INDEXBUFFER)
//#define XBR_INDEXBUFFER           (XBR_USER_DATA_FLAG|D3DRTYPE_INDEXBUFFER)
#define XBR_VERTEXSHADER            (XBR_USER_DATA_FLAG|0x010)
#define XBR_PIXELSHADER             (XBR_USER_DATA_FLAG|0x020)

#define XBR_EFFECT                  (XBR_USER_DATA_FLAG|0x040)
#define XBR_SKELETON                (XBR_USER_DATA_FLAG|0x050)
#define XBR_ANIMATION               (XBR_USER_DATA_FLAG|0x060)

// The symbol table maps strings to offsets.
// If present, this is the first resource in the file.
#define XBR_SYMBOLTABLE             (XBR_USER_DATA_FLAG|0x100)





//-----------------------------------------------------------------------------
// Name: XBResource_SizeOf()
// Desc: Determines the byte size of a D3DResource.  For resources
//       with the XBR_USER_DATA_FLAG set, the header size is stored in the
//       resource file.
//-----------------------------------------------------------------------------
DWORD XBResource_SizeOf( LPDIRECT3DRESOURCE8 pResource );





//-----------------------------------------------------------------------------
// Name: struct XBRESOURCETAG
// Desc: Name tag for resources. An app may initialize this structure, and pass
//       it to the resource's Create() function. From then on, the app may call
//       GetResource() to retrieve a resource using an ascii name.
//
//       If the resource file has a XBR_SYMBOLTABLE, then that will be
//       used if the XBRESOURCENAME array is set to NULL at creation time.
//-----------------------------------------------------------------------------
struct XBRESOURCETAG
{
    CHAR* strName;
    DWORD dwOffset;
};





//-----------------------------------------------------------------------------
// Name: class CXBPackedResource
// Desc: 
//-----------------------------------------------------------------------------
class CXBPackedResource
{
protected:
    BYTE*       m_pSysMemData;    // Alloc'ed mem for resource headers etc.
    BYTE*       m_pVidMemData;    // Alloc'ed mem for resource data, etc.

    DWORD       m_dwNumResources; // Number of loaded resources
 
    XBRESOURCETAG* m_pResourceTags;  // Tags to associate names with the resources

    HRESULT PatchAll();           // Patch and register all the loaded resources

public:
    // Loads the resources out of the specified bundle
    HRESULT Create( const CHAR* strFilename, DWORD dwNumResources, XBRESOURCETAG* pResourceTags = NULL );

    VOID Destroy();

    // Functions to retrieve resources by their offset
    VOID* GetData( DWORD dwOffset ) const
    { return &m_pSysMemData[dwOffset]; }

    LPDIRECT3DRESOURCE8 GetResource( DWORD dwOffset ) const
    { return (LPDIRECT3DRESOURCE8)GetData(dwOffset); }

    LPDIRECT3DTEXTURE8 GetTexture( DWORD dwOffset ) const
    { return (LPDIRECT3DTEXTURE8)GetData( dwOffset ); }

    LPDIRECT3DCUBETEXTURE8 GetCubemap( DWORD dwOffset ) const
    { return (LPDIRECT3DCUBETEXTURE8)GetData( dwOffset ); }

    LPDIRECT3DVOLUMETEXTURE8 GetVolumeTexture( DWORD dwOffset ) const
    { return (LPDIRECT3DVOLUMETEXTURE8)GetData( dwOffset ); }

    LPDIRECT3DVERTEXBUFFER8 GetVertexBuffer( DWORD dwOffset ) const
    { return (LPDIRECT3DVERTEXBUFFER8)GetData( dwOffset ); }

    // Functions to retrieve resources by their name
    VOID* GetData( const CHAR* strName ) const;

    LPDIRECT3DRESOURCE8 GetResource( const CHAR* strName ) const
    { return (LPDIRECT3DRESOURCE8)GetData( strName ); }

    LPDIRECT3DTEXTURE8 GetTexture( const CHAR* strName ) const
    { return (LPDIRECT3DTEXTURE8)GetResource( strName ); }

    LPDIRECT3DCUBETEXTURE8 GetCubemap( const CHAR* strName ) const
    { return (LPDIRECT3DCUBETEXTURE8)GetResource( strName ); }

    LPDIRECT3DVOLUMETEXTURE8 GetVolumeTexture( const CHAR* strName ) const
    { return (LPDIRECT3DVOLUMETEXTURE8)GetResource( strName ); }

    LPDIRECT3DVERTEXBUFFER8 GetVertexBuffer( const CHAR* strName ) const
    { return (LPDIRECT3DVERTEXBUFFER8)GetResource( strName ); }

    // Constructor/destructor
    CXBPackedResource();
    ~CXBPackedResource();
};


//-----------------------------------------------------------------------------
// Name: class XBResource
// Desc: Supports asynchronous reading and typed resources.
//-----------------------------------------------------------------------------

// forward references
struct Effect;
struct VertexShader;

// $REVISIT: MAX_NUM_RESOURCES should not be hard-coded, but should instead depend
// on the number of resources in the file.  In fact, the resource
// and type arrays should be replaced by the symbol table included
// in each XBR file.
#define MAX_NUM_RESOURCES 10000

enum LOADINGSTATE { LOADING_NOTSTARTED, LOADING_HEADER, LOADING_DATA, LOADING_DONE, LOADING_FAILED };

class XBResource : public CXBPackedResource
{
 public:
    XBResource();
    ~XBResource();

    // Accessors
    DWORD Count()                   { return m_dwNumResources; }
    DWORD GetTypeByIndex(DWORD ResourceIndex) 
        { if (ResourceIndex >= m_dwNumResources) return 0; else return m_rdwTypes[ResourceIndex]; }
    VOID *GetResourceByIndex(DWORD ResourceIndex)
        { if (ResourceIndex >= m_dwNumResources) return NULL; else return m_rpResources[ResourceIndex]; }
    LOADINGSTATE CurrentLoadingState() { return m_LoadingState; }

    // Starts asynchronous loading of the resources out of the specified bundle
    HRESULT StartLoading( LPSTR strFileBase );

    // Update status of loading state and call OnIOComplete if done.
    LOADINGSTATE PollLoadingState();

    // Called when Async I/O is complete to register the resources
    HRESULT OnIOComplete();

    // Unloads the resources.
    HRESULT Unload();   

    // Called by Unload to release handles, etc. to cleanup the resource before unloading.
    virtual HRESULT Cleanup(DWORD dwType,   // resource type
                            BYTE *pHeader); // pointer to resource header

    // Called by PatchAll to patch pointers within resources.
    // Default Patch implementation calls register on standard resource types.
    virtual HRESULT Patch(DWORD dwType,     // resource type
                          BYTE *pHeader);   // pointer to resource header

    
    // Look for effect with the given name
    Effect *GetEffect( const CHAR* strName ) const;

 protected:
    BYTE *                  m_rpResources[MAX_NUM_RESOURCES]; // Array of pointers to resources
    DWORD                   m_rdwTypes[MAX_NUM_RESOURCES]; // Array of resource types
    HANDLE                  m_hfXPR;                // File handle for async i/o
    DWORD                   m_cbHeaders;            // Count of bytes of resource headers
    DWORD                   m_cbData;               // Count of bytes of data
    OVERLAPPED              m_overlapped;           // OVERLAPPED structure for async I/O
    LOADINGSTATE            m_LoadingState;             // current loading state
};




//-----------------------------------------------------------------------------
//
//  An effect is a collection of rendering passes.  Each pass needs
//  vertex shaders (programs and constants), pixel shaders (programs
//  and constants), render states (including texture states),
//  textures, vertex buffers, index buffers, and draw lists.
//  
//-----------------------------------------------------------------------------

// $TODO: consider separate VertexShader and BoundVertexShader structures

// $TODO: break vertex shader and pixel shader constants out from declaration

#define VERTEXSHADER_MAX_STREAMS 16
struct VertexShader {
    DWORD Handle;
    DWORD DeclarationByteCount;     // size of Declaration
    DWORD *Declaration;             // the declaration maps streams to vertex registers, and defines constants
    DWORD FunctionByteCount;        // size of Function
    DWORD *Function;                // function microcode maps vertex registers to transformed vertices
    VertexShader *pVertexShader;    // base vertex shader to specialize
    struct StreamInput {            // mapping of vertex buffer references to vertex streams
        D3DVertexBuffer *pVertexBuffer;
        DWORD Stride;
        DWORD Offset;
    } rStreamInput[VERTEXSHADER_MAX_STREAMS];

    // Set the current vertex shader
    HRESULT SetVertexShader();
    // $TODO: Split out vertex shader function from vertex shader constants
};

typedef D3DPIXELSHADERDEF PixelShader;

struct RenderState {
    D3DRENDERSTATETYPE State;
    DWORD Value;
};

struct TextureState {
    D3DTEXTURESTAGESTATETYPE State;
    DWORD Value;
};

struct TextureStage {
    D3DBaseTexture *pTexture;
    DWORD TextureStateCount;
    TextureState *rTextureState;
};

struct Draw {
    D3DIndexBuffer *pIndexBuffer;
    D3DPRIMITIVETYPE Primitive;
    DWORD Start;
    DWORD Count;
};

struct Pass {
    VertexShader *pVertexShader;
    // $TODO: Add vertex shader constants here instead of in a separate
    // $TODO: vertex shader resource.  Leave the vertex shader resource just
    // $TODO: for defining the function and declaration.
    // $TODO: Add separate stream mapping here, too.
    PixelShader *pPixelShader;  // $TODO: Consider inlining the pixel shader definition
    // $TODO: Add pixel shader constants here, too.
    DWORD RenderStateCount;
    RenderState *rRenderState;
    DWORD TextureStageCount;
    TextureStage *rTextureStage;
    DWORD DrawCount;
    Draw *rDraw;
};

#define EFFECT_IDENTIFIER_SIZE 128
struct Effect {
    CHAR Identifier[EFFECT_IDENTIFIER_SIZE];
    D3DSurface *pRenderTarget;
    DWORD PassCount;
    Pass *rPass;

    // Draw effect
    HRESULT DrawEffect(VOID *rParameter = NULL, UINT ParameterCount = 0);
    
    // $TODO: get rid of BeginDraw and EndDraw.  These
    // should be set just once during initialization to set
    // up the expected defaults.
    static HRESULT BeginDraw(); // set default drawing state (vertex shader input, etc.)
    static HRESULT EndDraw();   // cleanup
};


#endif XBRESOURCE_H
