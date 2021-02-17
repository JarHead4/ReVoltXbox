//-----------------------------------------------------------------------------
//  
//  File: mmxr_xdx.cpp
//  Copyright (C) 2001 Microsoft Corporation
//  All rights reserved.
//  
//  Convert mmx data to XDX file.
//  
//-----------------------------------------------------------------------------
#pragma warning(disable: 4786)  // STL identifier truncation

#include <xtl.h>
#include <map>
#include <set>
//#include "XBUtil.h"
#include <float.h>

#define _PC
#include "revolt.h"
#include "draw.h"
#include "LevelLoad.h"
#include "instance.h"
#include "model.h"
#include "settings.h"

#define D3DVS_MAX_DECL_COUNT 100
#define D3DVS_MAXINSTRUCTIONCOUNT D3DVS_MAXINSTRUCTIONCOUNT_V1_0
#define D3DVS_CONSTREG_COUNT D3DVS_CONSTREG_COUNT_XBOX

struct XDX {
    FILE *m_fp; // buffered stream
    CHAR buf[4096]; // stream buffer
} *s_pXDX = NULL;

static HRESULT XDXOut(CONST CHAR *str)
{
    if (s_pXDX == NULL)
        return E_FAIL;
    if (fputs(str, s_pXDX->m_fp) < 0)
        return E_FAIL;
    else
        return S_OK;
}

static HRESULT XDXPrintf(CONST CHAR *strFormat, ...)
{
    if (s_pXDX == NULL)
        return E_FAIL;
    va_list val;
    va_start(val, strFormat);
    const buflen = 1000;
    CHAR buf[buflen];
    _vsnprintf(buf, buflen, strFormat, val);
    return XDXOut(buf);
}

static HRESULT XDXIndent(int indent)
{
    if (s_pXDX == NULL)
        return E_FAIL;
    while (indent-- > 0)
        XDXOut("\t");
    return S_OK;
}

HRESULT XDXBegin(CONST CHAR *strFile)
{
    if (s_pXDX != NULL)
        return E_FAIL;  // fail if already inside XDXBegin block

    // Open the file
    FILE *fp = fopen(strFile, "wb");
    if (fp == NULL)
        return E_FAIL;

    // Fill in the XDX structure
    s_pXDX = new XDX;
    if (s_pXDX == NULL)
    {
        fclose(fp);
        return E_OUTOFMEMORY;
    }
    setvbuf(fp, s_pXDX->buf, _IOFBF, sizeof(s_pXDX->buf));  // use a larger buffer than the default
    s_pXDX->m_fp = fp;
    
    // Write the header
    XDXOut("<?xml version=\"1.0\" ?>\n");
    XDXOut("<XDX version=\"0.2\" >\n");
    
    return S_OK;
}
    
HRESULT XDXEnd()
{
    if (s_pXDX == NULL)
        return E_FAIL;

    // write the footer
    XDXOut("</XDX>\n");

    // Close the file
    fclose(s_pXDX->m_fp);
    
    // Cleanup the XDX structure
    delete s_pXDX;
    s_pXDX = NULL;
    
    return S_OK;
}

//////////////////////////////////////////////////////////////////////
// 
// Make a relative path from strCurrent to strPath
//
CHAR *RelativePath(CHAR *strCurrent, CHAR *strPath)
{
    CHAR drive[_MAX_DRIVE];
    CHAR dir[_MAX_DIR];
    CHAR name[_MAX_PATH];
    CHAR ext[_MAX_EXT];
    _splitpath( strCurrent, drive, dir, name, ext );

    static CHAR path2[_MAX_PATH];
    CHAR drive2[_MAX_DRIVE];
    CHAR dir2[_MAX_DIR];
    CHAR name2[_MAX_PATH];
    CHAR ext2[_MAX_EXT];
    _splitpath( strPath, drive2, dir2, name2, ext2 );
    
    if (strcmp(drive2, drive) == 0)
    {
        // strip off common directory elements
        if (strcmp(dir2, dir) == 0)
        {
            // if drive and directory match, use short path
            _makepath(path2, "", "", name2, ext2);
        }
        else
        {
            // strip off common directory prefix
            CHAR *p = dir;
            CHAR *pslash = NULL;
            CHAR *p2 = dir2;
            while (*p && *p2 && *p == *p2)
            {
                if (*p == '/' || *p == '\\')
                    pslash = p;
                p++;
                p2++;
            }
            if (pslash)
            {
                // move to just beyond the slash
                INT LastSlashOffset = p - pslash - 1;
                p -= LastSlashOffset;
                p2 -= LastSlashOffset;
            }

            // add "../" for each of the subdirectories left
            CHAR dir3[_MAX_PATH];
            CHAR *p3 = dir3;
            while (*p)
            {
                if (*p == '/' || *p == '\\')
                {
                    *p3++ = '.';
                    *p3++ = '.';
                    *p3++ = '/';
                }
                p++;
            }
            
            // concat remaining directories, replacing back slashes with forward slashes
            while (*p2)
            {
                if (*p2 == '\\')
                {
                    *p3++ = '/';
                    p2++;
                }
                else
                    *p3++ = *p2++;
            }
            *p3 = 0;
            
            _makepath(path2, "", dir3, name2, ext2);
        }
    }
    else
    {
        // Drives don't match, make full path
        _makepath( path2, drive2, dir2, name2, ext2);
    }
    return path2;
}

//////////////////////////////////////////////////////////////////////
// 
// Make an identifier from a path.
// Currently, we ignore the directory. This might cause name collisions.
//
CHAR *TextureIdentifier(CHAR *strPath)
{
    CHAR drive[_MAX_DRIVE];
    CHAR dir[_MAX_DIR];
    static CHAR name[_MAX_PATH];
    CHAR ext[_MAX_EXT];
    _splitpath( strPath, drive, dir, name, ext );
    return name;
}

//////////////////////////////////////////////////////////////////////
// 
// Make an identifier from a texture page.
//
CHAR *TextureIdentifier(SHORT Tpage)
{
    return TextureIdentifier(TexInfo[Tpage].File);
}


//////////////////////////////////////////////////////////////////////
// Write a vertex shader constant 
//
HRESULT XDXSetVertexShaderConstant(INT Register, CONST void *pConstantData, DWORD ConstantCount, 
                                  CONST CHAR *strName, CONST CHAR *strParameter, int indent)
{
    if (ConstantCount == 0)
        return E_INVALIDARG;
    CONST FLOAT *pfConstant = (CONST FLOAT *)pConstantData;
    XDXIndent(indent);
    XDXPrintf("<Constant index=\"%d\"", Register);
    if (strName != NULL)
        XDXPrintf(" name=\"%s\"", strName);
    if (ConstantCount > 1)
        XDXPrintf(" count=\"%d\"", ConstantCount);
    if (strParameter != NULL)
        XDXPrintf(" parameter=\"%s\"", strParameter);
    XDXPrintf(">");
    for (UINT i = 0; i < ConstantCount; i++)
    {
        if (ConstantCount > 1)
        {
            XDXPrintf("\n");
            XDXIndent(indent + 1);
        }
        XDXPrintf("%g %g %g %g", pfConstant[0], pfConstant[1], pfConstant[2], pfConstant[3]);
        pfConstant += 4;
    }
    XDXPrintf("</Constant>\n");
    return S_OK;
}

//////////////////////////////////////////////////////////////////////
//
// Compute face normal
//
HRESULT ComputeFaceNormal(float *pnormal, MODEL_POLY *mp)
{
    D3DXVECTOR3 v0( mp->v0->x, mp->v0->y, mp->v0->z );
    D3DXVECTOR3 v1( mp->v1->x, mp->v1->y, mp->v1->z );
    D3DXVECTOR3 v2( mp->v2->x, mp->v2->y, mp->v2->z );
    D3DXVECTOR3 e1 = v1 - v0;
    D3DXVECTOR3 e2 = v2 - v0;
    D3DXVECTOR3 normal;
    D3DXVec3Cross(&normal, &e1, &e2);
    D3DXVec3Normalize(&normal, &normal);
    pnormal[0] = normal.x;
    pnormal[1] = normal.y;
    pnormal[2] = normal.z;
    return S_OK;
}

//////////////////////////////////////////////////////////////////////
//
// Check if polygon normal is invalid and fix if possible.
//
HRESULT ValidateNormal(float *pnormal, MODEL_POLY *mp)
{
    if (_isnan(pnormal[0]) 
        || _isnan(pnormal[1]) 
        || _isnan(pnormal[2]))
    {
        // Try using face normal instead.
        XDXPrintf("<!-- Replacing invalid normal %g %g %g -->\n",
            pnormal[0], pnormal[1], pnormal[2]);
        ComputeFaceNormal(pnormal, mp);
        Assert(!_isnan(pnormal[0]) && !_isnan(pnormal[1]) && !_isnan(pnormal[0]));
        return S_FALSE;
    }
    return S_OK;
}


//////////////////////////////////////////////////////////////////////
//
// Check a polygon for invalid data, and fix if possible.
//
HRESULT ValidatePoly(MODEL_POLY *mp)
{
    HRESULT hr = S_OK;
    Assert(!_isnan(mp->v0->x) && !_isnan(mp->v0->y) && !_isnan(mp->v0->z));
    if (ValidateNormal(&mp->v0->nx, mp) != S_OK) hr = S_FALSE;
    if (mp->Tpage != -1)
        Assert(!_isnan(mp->tu0) && !_isnan(mp->tv0));
    
    Assert(!_isnan(mp->v1->x) && !_isnan(mp->v1->y) && !_isnan(mp->v1->z));
    if (ValidateNormal(&mp->v1->nx, mp) != S_OK) hr = S_FALSE;
    if (mp->Tpage != -1)
        Assert(!_isnan(mp->tu1) && !_isnan(mp->tv1));
    
    Assert(!_isnan(mp->v2->x) && !_isnan(mp->v2->y) && !_isnan(mp->v2->z));
    if (ValidateNormal(&mp->v2->nx, mp) != S_OK) hr = S_FALSE;
    if (mp->Tpage != -1)
        Assert(!_isnan(mp->tu2) && !_isnan(mp->tv2));
    
    if (mp->Type & POLY_QUAD)
    {
        Assert(!_isnan(mp->v3->x) && !_isnan(mp->v3->y) && !_isnan(mp->v3->z));
        if (ValidateNormal(&mp->v3->nx, mp) != S_OK) hr = S_FALSE;
        if (mp->Tpage != -1)
            Assert(!_isnan(mp->tu3) && !_isnan(mp->tv3));
    }

    // TODO: Validate colors
    // TODO: If vertex alpha's are zero, make sure this is not a transparent polygon
    return hr;
}

//////////////////////////////////////////////////////////////////////
//
// Print a polygon
//
HRESULT PrintPolyVertices(MODEL_POLY *mp, POLY_RGB *mrgb, int iVertex, int indent)
{
    ValidatePoly(mp);
    
    XDXIndent(indent);
    XDXPrintf("<v index=\"%d\">", iVertex++);
    XDXPrintf("%g %g %g", mp->v0->x, mp->v0->y, mp->v0->z);
    XDXPrintf("\t%g %g %g", mp->v0->nx, mp->v0->ny, mp->v0->nz);
    XDXPrintf("\t0x%.08x", D3DCOLOR_RGBA(mrgb->rgb[0].r, mrgb->rgb[0].g, mrgb->rgb[0].b, mrgb->rgb[0].a));
    if (mp->Tpage != -1)
        XDXPrintf("\t%g %g", mp->tu0, mp->tv0);
    XDXPrintf("</v>\n");

    XDXIndent(indent);
    XDXPrintf("<v index=\"%d\">", iVertex++);
    XDXPrintf("%g %g %g", mp->v1->x, mp->v1->y, mp->v1->z);
    XDXPrintf("\t%g %g %g", mp->v1->nx, mp->v1->ny, mp->v1->nz);
    XDXPrintf("\t0x%.08x", D3DCOLOR_RGBA(mrgb->rgb[1].r, mrgb->rgb[1].g, mrgb->rgb[1].b, mrgb->rgb[1].a));
    if (mp->Tpage != -1)
        XDXPrintf("\t%g %g", mp->tu1, mp->tv1);
    XDXPrintf("</v>\n");
    
    XDXIndent(indent);
    XDXPrintf("<v index=\"%d\">", iVertex++);
    XDXPrintf("%g %g %g", mp->v2->x, mp->v2->y, mp->v2->z);
    XDXPrintf("\t%g %g %g", mp->v2->nx, mp->v2->ny, mp->v2->nz);
    XDXPrintf("\t0x%.08x", D3DCOLOR_RGBA(mrgb->rgb[2].r, mrgb->rgb[2].g, mrgb->rgb[2].b, mrgb->rgb[2].a));
    if (mp->Tpage != -1)
        XDXPrintf("\t%g %g", mp->tu2, mp->tv2);
    XDXPrintf("</v>\n");

    if (mp->Type & POLY_QUAD)
    {
        XDXIndent(indent);
        XDXPrintf("<v index=\"%d\">", iVertex++);
        XDXPrintf("%g %g %g", mp->v3->x, mp->v3->y, mp->v3->z);
        XDXPrintf("\t%g %g %g", mp->v3->nx, mp->v3->ny, mp->v3->nz);
        XDXPrintf("\t0x%.08x", D3DCOLOR_RGBA(mrgb->rgb[3].r, mrgb->rgb[3].g, mrgb->rgb[3].b, mrgb->rgb[3].a));
        if (mp->Tpage != -1)
            XDXPrintf("\t%g %g", mp->tu3, mp->tv3);
        XDXPrintf("</v>\n");
    }
    
    return S_OK;
}

// Split model up the same way the draw routines work
// (except for the environment map, mirrored, and ghost
// passes which are just copies of the same data, but with
// different transforms or overriding colors).
enum Passes {
    OpaqueTextured = 0,         // Opaque textured quads and tris
    OpaqueRGB = 1,              // Opaque rgb-per-vertex quads and tris
    TransparentTextured = 2,    // Transparent textured quads and tris
    TransparentRGB = 3          // Transparent rgb-per-vertex quads and tris
};

static CHAR *PassName[] =
{
    "OpaqueTextured",
    "OpaqueRGB",
    "TransparentTextured",
    "TransparentRGB"
};

// Vertex shaders are defined in Default.xdx
static CHAR *PassVertexShader[] =
{
    "VSLightDiffuseTexture",
    "VSLightDiffuse",
    "VSLightDiffuseTexture",
    "VSLightDiffuse",
};

//////////////////////////////////////////////////////////////////////
//
// Export model in XDX resource format
//
HRESULT ExportModel(char *file, MODEL *m, int ModelCount)
{
    // get the name of the output file
    CHAR path[_MAX_PATH];
    CHAR drive[_MAX_DRIVE];
    CHAR dir[_MAX_DIR];
    CHAR name[_MAX_PATH];
    CHAR ext[_MAX_EXT];
    _splitpath( file, drive, dir, name, ext );
    _makepath( path, drive, dir, name, ".xdx");

    if (XDXBegin( path ) == S_OK)
    {
        int indent = 0;
        XDXPrintf("<!-- converted from %s\n", file );
        XDXPrintf("ModelCount=\"%d\"\n", ModelCount);
        XDXOut("<include href=\"Default.xdx\" />\n");
        XDXPrintf("-->\n");
        for (int iModel = 0; iModel < ModelCount; iModel++, m++)
        {
            // Skip NULL models
            if (m->AllocPtr == NULL)
                continue;
            const int maxName = 1000;
            CHAR strModelName[maxName];
            if (ModelCount > 1)
                _snprintf(strModelName, maxName, "%s%d", name, iModel);
            else
                strncpy(strModelName, name, maxName);
            strModelName[maxName - 1] = 0;  // make sure string is terminated
            CHAR strCameraName[maxName];
            strcpy(strCameraName, strModelName);
            strncat(strCameraName, "_PreviewCamera", maxName);
            strCameraName[maxName - 1] = 0; // make sure string is terminated
            
            long i;
            POLY_RGB *mrgb;
            MODEL_POLY *mp;

            // Count the number of vertices, primitives,
            // and unique textures used in each pass
            UINT rQuadCount[4] = { 0, 0, 0, 0 };
            UINT rTriCount[4] = { 0, 0, 0, 0 };
            std::set<short> TextureSet;
            std::set<short>::iterator TextureSetIterator;
            mrgb = m->PolyRGB;
            mp = m->PolyPtr;
            for (i = m->QuadNumTex ; i ; i--, mrgb++, mp++)
            {
                Assert(mp->Tpage != -1);
                TextureSet.insert(mp->Tpage);
                if (mp->Type & POLY_SEMITRANS)
                    rQuadCount[TransparentTextured]++;
                else
                    rQuadCount[OpaqueTextured]++;
            }
            for (i = m->TriNumTex ; i ; i--, mrgb++, mp++)
            {
                Assert(mp->Tpage != -1);
                TextureSet.insert(mp->Tpage);
                if (mp->Type & POLY_SEMITRANS)
                    rTriCount[TransparentTextured]++;
                else
                    rTriCount[OpaqueTextured]++;
            }
            for (i = m->QuadNumRGB ; i ; i--, mrgb++, mp++)
            {
                Assert(mp->Tpage == -1);    // RGB quads should not have tpages
                if (mp->Type & POLY_SEMITRANS)
                    rQuadCount[TransparentRGB]++;
                else
                    rQuadCount[OpaqueRGB]++;
            }
            for (i = m->TriNumRGB ; i ; i--, mrgb++, mp++)
            {
                Assert(mp->Tpage == -1);    // RGB tris should not have tpages
                if (mp->Type & POLY_SEMITRANS)
                    rTriCount[TransparentRGB]++;
                else
                    rTriCount[OpaqueRGB]++;
            }
            UINT TextureCount = TextureSet.size();
#define MAX_TEXTURE_COUNT 100
            Assert(TextureCount <= MAX_TEXTURE_COUNT);

#if 1
            XDXPrintf("<!--\n");

            // Output the textures
            for (TextureSetIterator = TextureSet.begin();
                 TextureSetIterator != TextureSet.end();
                 TextureSetIterator++)
            {
                short Tpage = *TextureSetIterator;  // current texture page
                CHAR path2[_MAX_PATH];
                CHAR drive2[_MAX_DRIVE];
                CHAR dir2[_MAX_DIR];
                CHAR name2[_MAX_PATH];
                CHAR ext2[_MAX_EXT];
                _splitpath( TexInfo[Tpage].File, drive2, dir2, name2, ext2 );
                if (strcmp(drive2, drive) == 0
                    && strcmp(dir2, dir) == 0)
                    // if drive and directory match, use short path
                    _makepath(path2, "", "", name2, ext2);
                else
                    _makepath( path2, drive2, dir2, name2, ext2);
                XDXIndent(indent);
                XDXPrintf("<Texture id=\"%s\" source=\"%s\" />\n", TextureIdentifier(Tpage), RelativePath(path, path2));
            }
            XDXPrintf("-->\n");
#endif
            
            // Set per-texture primitive counts to zero
            UINT rQuadCountPerTexture[2][MAX_TEXTURE_COUNT];    // one count for transparent, one for opaque
            UINT rTriCountPerTexture[2][MAX_TEXTURE_COUNT];
            for (UINT j = 0; j < 2; j++)
            {
                for (UINT iTexture = 0; iTexture < TextureCount; iTexture++)
                {
                    rQuadCountPerTexture[j][iTexture] = 0;
                    rTriCountPerTexture[j][iTexture] = 0;
                }
            }

            // Output the vertex buffers, while counting the number of per-texture primitives
            UINT iPass;
            for (iPass = 0; iPass < 4; iPass++)
            {
                bool bTransparent = (iPass == TransparentTextured) || (iPass == TransparentRGB);
                if (rQuadCount[iPass] + rTriCount[iPass] == 0) continue;
                
                if (iPass == TransparentTextured || iPass == OpaqueTextured)
                {
                    XDXIndent(indent);
                    XDXPrintf("<VertexBuffer id=\"%s_VertexBuffer_%s\"\n", strModelName, PassName[iPass]);
                    
                    // NOTE: Re-volt seems to be using specular only to hold
                    // the current fog value.  So, do not output specular to
                    // the vertex buffer.
                    XDXIndent(indent);
                    XDXPrintf(" names=\"POSITION NORMAL DIFFUSE TEXCOORD0\"\n");
                    XDXIndent(indent);
                    XDXPrintf(" format=\"FLOAT3 FLOAT3 D3DCOLOR FLOAT2\"");
                    XDXPrintf(">\n");
                    indent++;
                    
                    // Go through the data once for each texture, to group
                    // the primitives by texture.
                    UINT iVertex = 0;
                    UINT iTexture = 0;  // keep primitive counts per texture
                    for (TextureSetIterator = TextureSet.begin();
                         TextureSetIterator != TextureSet.end();
                         TextureSetIterator++, iTexture++)
                    {
                        short Tpage = *TextureSetIterator;  // current texture page to output
                        mrgb = m->PolyRGB;
                        mp = m->PolyPtr;
                        long QuadNum = m->QuadNumTex;
                        long TriNum = m->TriNumTex;
                        for (i = QuadNum ; i ; i--, mrgb++, mp++)
                        {
                            if (mp->Tpage != Tpage) continue;
                            if (!!(mp->Type & POLY_SEMITRANS) != bTransparent) continue;
                            rQuadCountPerTexture[(int)bTransparent][iTexture]++;
                            PrintPolyVertices(mp, mrgb, iVertex, indent);
                            iVertex += 4;
                        }
                        for (i = TriNum ; i ; i--, mrgb++, mp++)
                        {
                            if (mp->Tpage != Tpage) continue;
                            if (!!(mp->Type & POLY_SEMITRANS) != bTransparent) continue;
                            rTriCountPerTexture[(int)bTransparent][iTexture]++;
                            PrintPolyVertices(mp, mrgb, iVertex, indent);
                            iVertex += 3;
                        }
                    }
                    indent--;
                    XDXIndent(indent);
                    XDXPrintf("</VertexBuffer>\n");
                }
                else    // non-textured RGB-per-vertex quads and tris
                {
                    XDXIndent(indent);
                    XDXPrintf("<VertexBuffer id=\"%s_VertexBuffer_%s\"\n", strModelName, PassName[iPass]);
                    
                    // NOTE: Re-volt seems to be using specular only to hold
                    // the current fog value.  So, do not output specular to
                    // the vertex buffer.
                    XDXIndent(indent);
                    XDXPrintf(" names=\"POSITION NORMAL DIFFUSE\"\n");
                    XDXIndent(indent);
                    XDXPrintf(" format=\"FLOAT3 FLOAT3 D3DCOLOR\"");
                    XDXPrintf(">\n");
                    indent++;
    
                    UINT iVertex = 0;
                    mrgb = m->PolyRGB;
                    mp = m->PolyPtr;
                    long QuadNum, TriNum;
                    QuadNum = m->QuadNumRGB;
                    TriNum = m->TriNumRGB;
                        
                    // Skip to RGB polys
                    long SkipCount = m->QuadNumTex + m->TriNumTex;
                    mrgb += SkipCount;
                    mp += SkipCount;
                    
                    for (i = QuadNum ; i ; i--, mrgb++, mp++)
                    {
                        if (!!(mp->Type & POLY_SEMITRANS) != bTransparent) continue;
                        PrintPolyVertices(mp, mrgb, iVertex, indent);
                        iVertex += 4;
                    }
                    for (i = TriNum ; i ; i--, mrgb++, mp++)
                    {
                        if (!!(mp->Type & POLY_SEMITRANS) != bTransparent) continue;
                        PrintPolyVertices(mp, mrgb, iVertex, indent);
                        iVertex += 3;
                    }
                    indent--;
                    XDXIndent(indent);
                    XDXPrintf("</VertexBuffer>\n");
                }
            }

            // XDXIndent(indent);
            // XDXPrintf("<!-- TODO: Index Buffer goes here -->\n");
            
            XDXIndent(indent);
            XDXPrintf("<Model id=\"%s\">\n", strModelName);
            indent++;

            // Custom model attributes
            XDXIndent(indent);
            XDXPrintf("<!--\n");    // TODO: remove these comments when custom extensions to the compiler are in place
            XDXIndent(indent);
            XDXPrintf("<Revolt");
            XDXPrintf(" Radius=\"%f\"", m->Radius);
            XDXPrintf(" Xmin=\"%f\"", m->Xmin);
            XDXPrintf(" Xmax=\"%f\"", m->Xmax);
            XDXPrintf(" Ymin=\"%f\"", m->Ymin);
            XDXPrintf(" Ymax=\"%f\"", m->Ymax);
            XDXPrintf(" Zmin=\"%f\"", m->Zmin);
            XDXPrintf(" Zmax=\"%f\"\n", m->Zmax);
            XDXIndent(indent);
            XDXPrintf(" PolyNum=\"%d\"", m->PolyNum);
            XDXPrintf(" VertNum=\"%d\"", m->VertNum);
            XDXPrintf(" QuadNumTex=\"%d\"", m->QuadNumTex);
            XDXPrintf(" TriNumTex=\"%d\"", m->TriNumTex);
            XDXPrintf(" QuadNumRGB=\"%d\"", m->QuadNumRGB);
            XDXPrintf(" TriNumRGB=\"%d\"", m->TriNumRGB);
            XDXPrintf(" />\n");
            XDXIndent(indent);
            XDXPrintf("-->\n");

            // Set the preview camera to scale the object to full-screen
            D3DXVECTOR3 vCenter(0.5f * (m->Xmax + m->Xmin), 0.5f * (m->Ymax + m->Ymin), 0.5f * (m->Zmax + m->Zmin));
            D3DXMATRIX matCenter;                               // translate object center to origin
            D3DXMatrixTranslation(&matCenter, -vCenter.x, -vCenter.y, -vCenter.z);
            FLOAT fScale = 1.f / m->Radius;
            D3DXMATRIX matScale;
            D3DXMatrixScaling(&matScale, fScale, fScale, fScale);   // scale object to -1,1 range
            D3DXMATRIX matCameraScale;
            D3DXMatrixScaling(&matCameraScale, 1.f, 1.f, 0.5f); // scale from z -1,1 to z -0.5,0.5
            D3DXMATRIX matCameraTranslate;
            D3DXMatrixTranslation(&matCameraTranslate, 0.f, 0.f, 0.5f); // translate from z -0.5,0.5 to z 0,1
            D3DXMATRIX matCamera = matCenter * matScale * matCameraScale * matCameraTranslate;
            
            D3DXMATRIX matIdentity;
            D3DXMatrixIdentity(&matIdentity);
            D3DXVECTOR4 diffuse(1.f, 1.f, 1.f, 1.f);
            D3DXVECTOR4 lightdir(0.f, 0.f, 1.f, 1.f);

            // Dump out draw passes
            for (iPass = 0; iPass < 4; iPass++)
            {
                if (rQuadCount[iPass] + rTriCount[iPass] == 0) continue;
                bool bTransparent = (iPass == TransparentTextured) || (iPass == TransparentRGB);
                if (iPass == TransparentTextured || iPass == OpaqueTextured)
                {
                    // One pass for each texture
                    UINT iVertex = 0;
                    UINT iTexture = 0;  // use primitive counts per texture
                    for (TextureSetIterator = TextureSet.begin();
                         TextureSetIterator != TextureSet.end();
                         TextureSetIterator++, iTexture++)
                    {
                        UINT QuadCount = rQuadCountPerTexture[(int)bTransparent][iTexture];
                        UINT TriCount = rTriCountPerTexture[(int)bTransparent][iTexture];
                        if (QuadCount + TriCount == 0)
                            continue;   // no primitives use the texture this pass
                        
                        short Tpage = *TextureSetIterator;  // current texture page to output
                        XDXIndent(indent);
                        XDXPrintf("<Pass>\n");
                        indent++;

                        // NOTE: The current XDX format doesn't easily
                        // let you do things like switch a single
                        // texture or a single vertex shader constant.
                        // We have to output all of this redundant
                        // goo.  Perhaps XDX should be more free-form
                        // for this kind of stuff, more like a
                        // pushbuffer.
                
                        // Vertex Shader
                        XDXIndent(indent);
                        XDXPrintf("<VertexShader idref=\"%s\">\n", PassVertexShader[iPass]);
                        indent++;
                        XDXIndent(indent);
                        XDXPrintf("<decl>\n");
                        XDXIndent(indent);
                        XDXPrintf("<stream index=\"0\" idref=\"%s_VertexBuffer_%s\" />\n", strModelName, PassName[iPass]);
                        XDXIndent(indent);
                        XDXPrintf("</decl>\n");
                        XDXIndent(indent);
                        XDXPrintf("<!-- TODO: track down the proper source of the model's material color -->\n");
                        XDXSetVertexShaderConstant(0, &diffuse, 1, "Diffuse", NULL, indent);
                        XDXSetVertexShaderConstant(1, &lightdir, 1, "LightDirection", NULL, indent);
                        XDXSetVertexShaderConstant(4, &matCamera, 4, "World * View * Projection", strCameraName, indent);
                        XDXSetVertexShaderConstant(8, &matIdentity, 3, "Transpose(Inverse(World))", NULL, indent);
                        indent--;
                        XDXIndent(indent);
                        XDXPrintf("</VertexShader>\n");

                        // Render State
                        XDXIndent(indent);
                        XDXPrintf("<RenderState>\n");
                        XDXIndent(indent);
                        XDXPrintf("<TextureState Stage=\"0\" idref=\"%s\"\n", TextureIdentifier(Tpage));
                        indent++;
                        XDXIndent(indent);
                        XDXPrintf("ColorOp=\"Modulate\"\n");
                        XDXIndent(indent);
                        XDXPrintf("ColorArg1=\"Texture\"\n");
                        XDXIndent(indent);
                        XDXPrintf("ColorArg2=\"Diffuse\"\n");
                        XDXIndent(indent);
                        XDXPrintf("AlphaOp=\"Modulate\"\n");
                        XDXIndent(indent);
                        XDXPrintf("AlphaArg1=\"Texture\"\n");
                        XDXIndent(indent);
                        XDXPrintf("AlphaArg2=\"Diffuse\"\n");
                        XDXIndent(indent);
                        XDXPrintf("/>\n");
                        indent--;
                        XDXIndent(indent);
                        XDXPrintf("<TextureState Stage=\"1\"");
                        XDXPrintf(" ColorOp=\"Disable\"");
                        XDXPrintf(" AlphaOp=\"Disable\"");
                        XDXPrintf("/>\n");
                        XDXIndent(indent);
                        XDXPrintf("</RenderState>\n");

                        // Draw commands.  The start and count attributes are vertex counts, not primitive counts.
                        if (QuadCount > 0)
                        {
                            XDXIndent(indent);
                            XDXPrintf("<Draw primitive=\"QUADLIST\" start=\"%d\" count=\"%d\" />\n",
                                      iVertex, QuadCount * 4);
                            iVertex += QuadCount * 4;
                        }
                        if (TriCount > 0)
                        {
                            XDXIndent(indent);
                            XDXPrintf("<Draw primitive=\"TRILIST\" start=\"%d\" count=\"%d\" />\n",
                                      iVertex, TriCount * 3);
                            iVertex += TriCount * 3;
                        }
                        
                        indent--;
                        XDXIndent(indent);
                        XDXPrintf("</Pass>\n");
                    }
                }
                else    // non-textured rgb-per-pixel quads and tris
                {
                    XDXIndent(indent);
                    XDXPrintf("<Pass>\n");
                    indent++;
    
                    // Vertex Shader
                    XDXIndent(indent);
                    XDXPrintf("<VertexShader idref=\"%s\">\n", PassVertexShader[iPass]);
                    indent++;
                    XDXIndent(indent);
                    XDXPrintf("<decl>\n");
                    XDXIndent(indent);
                    XDXPrintf("<stream index=\"0\" idref=\"%s_VertexBuffer_%s\" />\n", strModelName, PassName[iPass]);
                    XDXIndent(indent);
                    XDXPrintf("</decl>\n");
                    XDXIndent(indent);
                    XDXPrintf("<!-- TODO: track down the proper source of the model's material color -->\n");
                    XDXSetVertexShaderConstant(0, &diffuse, 1, "Diffuse", NULL, indent);
                    XDXSetVertexShaderConstant(1, &lightdir, 1, "LightDirection", NULL, indent);
                    XDXSetVertexShaderConstant(4, &matCamera, 4, "World * View * Projection", strCameraName, indent);
                    XDXSetVertexShaderConstant(8, &matIdentity, 3, "Transpose(Inverse(World))", NULL, indent);
                    indent--;
                    XDXIndent(indent);
                    XDXPrintf("</VertexShader>\n");
    
                    // Render State
                    XDXIndent(indent);
                    XDXPrintf("<RenderState>\n");
                    XDXIndent(indent);
                    XDXPrintf("<TextureState Stage=\"0\"");
                    XDXPrintf(" ColorOp=\"SelectArg1\"");
                    XDXPrintf(" ColorArg1=\"Diffuse\"");
                    XDXPrintf(" AlphaOp=\"SelectArg1\"");
                    XDXPrintf(" AlphaArg1=\"Diffuse\"");
                    XDXPrintf("/>\n");
                    XDXIndent(indent);
                    XDXPrintf("<TextureState Stage=\"1\"");
                    XDXPrintf(" ColorOp=\"Disable\"");
                    XDXPrintf(" AlphaOp=\"Disable\"");
                    XDXPrintf("/>\n");
                    XDXIndent(indent);
                    XDXPrintf("</RenderState>\n");
    
                    // Draw commands
                    if (rQuadCount[iPass] > 0)
                    {
                        XDXIndent(indent);
                        XDXPrintf("<Draw primitive=\"QUADLIST\" start=\"0\" count=\"%d\" />\n",
                                  rQuadCount[iPass] * 4);
                    }
                    if (rTriCount[iPass] > 0)
                    {
                        XDXIndent(indent);
                        XDXPrintf("<Draw primitive=\"TRILIST\" start=\"%d\" count=\"%d\" />\n",
                                  rQuadCount[iPass] * 4, rTriCount[iPass] * 3);
                    }
                    indent--;
                    XDXIndent(indent);
                    XDXPrintf("</Pass>\n");
                }
            }
            indent--;
            XDXIndent(indent);
            XDXPrintf("</Model>\n");

            /*
            // Set the preview camera to scale the object to full-screen,
            // and to rotate around the Y axis
            XDXIndent(indent);
            XDXPrintf("<Frame id=\"%s\">\n", strCameraName);
            indent++;
            XDXIndent(indent);
            XDXPrintf("<Matrix>\n");
            indent++;
            XDXIndent(indent);
            XDXPrintf("<Translate value=\"0 0 0.5\" />\n");
            XDXIndent(indent);
            XDXPrintf("<Scale value=\"1 1 0.5\" />\n");
            XDXIndent(indent);
            XDXPrintf("<!-- Rotate around Y axis for preview -->\n");
            XDXIndent(indent);
            XDXPrintf("<Rotate angle=\"0\" axis=\"0 1 0\">\n");
            XDXIndent(indent + 1);
            XDXPrintf("<animate attribute=\"angle\" begin=\"0\" dur=\"2\" values=\"0;3.141592654;6.283185307;\" />\n");
            XDXIndent(indent);
            XDXPrintf("</Rotate>\n");
            XDXIndent(indent);
            XDXPrintf("<!-- Point Y up -->\n");
            XDXIndent(indent);
            XDXPrintf("<Rotate angle=\"3.141592654\" axis=\"1 0 0\" />\n");
            XDXIndent(indent);
            XDXPrintf("<!-- Scale model to unit sphere 1/radius = 1/%f -->\n", m->Radius);
            XDXIndent(indent);
            XDXPrintf("<Scale value=\"%f %f %f\" />\n", fScale, fScale, fScale);
            XDXIndent(indent);
            XDXPrintf("<!-- Translate model center to origin -->\n");
            XDXIndent(indent);
            XDXPrintf("<Translate value=\"%f %f %f\" />\n", -vCenter.x, -vCenter.y, -vCenter.z);
            indent--;
            XDXIndent(indent);
            XDXPrintf("</Matrix>\n");
            indent--;
            XDXIndent(indent);
            XDXPrintf("</Frame>\n");
            */
        }
        XDXEnd();
    }
    return S_OK;
}





//////////////////////////////////////////////////////////////////////
//
// Compute face normal
//
HRESULT ComputeFaceNormal(float *pnormal, MODEL_POLY_LOAD *pPoly, MODEL_VERTEX_LOAD *rVert)
{
    D3DXVECTOR3 v0( rVert[pPoly->vi0].x, rVert[pPoly->vi0].y, rVert[pPoly->vi0].z );
    D3DXVECTOR3 v1( rVert[pPoly->vi1].x, rVert[pPoly->vi1].y, rVert[pPoly->vi1].z );
    D3DXVECTOR3 v2( rVert[pPoly->vi2].x, rVert[pPoly->vi2].y, rVert[pPoly->vi2].z );
    D3DXVECTOR3 e1 = v1 - v0;
    D3DXVECTOR3 e2 = v2 - v0;
    D3DXVECTOR3 normal;
    D3DXVec3Cross(&normal, &e1, &e2);
    D3DXVec3Normalize(&normal, &normal);
    pnormal[0] = normal.x;
    pnormal[1] = normal.y;
    pnormal[2] = normal.z;
    return S_OK;
}

//////////////////////////////////////////////////////////////////////
//
// Check if polygon normal is invalid and fix if possible.
//
HRESULT ValidateNormal(float *pnormal, MODEL_POLY_LOAD *pPoly, MODEL_VERTEX_LOAD *rVert)
{
    if (_isnan(pnormal[0]) 
        || _isnan(pnormal[1]) 
        || _isnan(pnormal[2]))
    {
        // Try using face normal instead.
        XDXPrintf("<!-- Replacing invalid normal %g %g %g -->\n",
            pnormal[0], pnormal[1], pnormal[2]);
        ComputeFaceNormal(pnormal, pPoly, rVert);
        Assert(!_isnan(pnormal[0]) && !_isnan(pnormal[1]) && !_isnan(pnormal[0]));
        return S_FALSE;
    }
    return S_OK;
}


//////////////////////////////////////////////////////////////////////
//
// Check a polygon for invalid data, and fix if possible.
//
HRESULT ValidatePoly(MODEL_POLY_LOAD *pPoly, MODEL_VERTEX_LOAD *rVert)
{
    HRESULT hr = S_OK;
    Assert(!_isnan(rVert[pPoly->vi0].x) && !_isnan(rVert[pPoly->vi0].y) && !_isnan(rVert[pPoly->vi0].z));
    if (ValidateNormal(&rVert[pPoly->vi0].nx, pPoly, rVert) != S_OK) hr = S_FALSE;
    if (pPoly->Tpage != -1)
        Assert(!_isnan(pPoly->u0) && !_isnan(pPoly->v0));
    
    Assert(!_isnan(rVert[pPoly->vi1].x) && !_isnan(rVert[pPoly->vi1].y) && !_isnan(rVert[pPoly->vi1].z));
    if (ValidateNormal(&rVert[pPoly->vi1].nx, pPoly, rVert) != S_OK) hr = S_FALSE;
    if (pPoly->Tpage != -1)
        Assert(!_isnan(pPoly->u1) && !_isnan(pPoly->v1));
    
    Assert(!_isnan(rVert[pPoly->vi2].x) && !_isnan(rVert[pPoly->vi2].y) && !_isnan(rVert[pPoly->vi2].z));
    if (ValidateNormal(&rVert[pPoly->vi2].nx, pPoly, rVert) != S_OK) hr = S_FALSE;
    if (pPoly->Tpage != -1)
        Assert(!_isnan(pPoly->u2) && !_isnan(pPoly->v2));
    
    if (pPoly->Type & POLY_QUAD)
    {
        Assert(!_isnan(rVert[pPoly->vi3].x) && !_isnan(rVert[pPoly->vi3].y) && !_isnan(rVert[pPoly->vi3].z));
        if (ValidateNormal(&rVert[pPoly->vi3].nx, pPoly, rVert) != S_OK) hr = S_FALSE;
        if (pPoly->Tpage != -1)
            Assert(!_isnan(pPoly->u3) && !_isnan(pPoly->v3));
    }

    // TODO: Validate colors
    // TODO: If vertex alpha's are zero, make sure this is not a transparent polygon
    return hr;
}

//////////////////////////////////////////////////////////////////////
//
// Re-arrange to D3DCOLOR format
//
D3DCOLOR ConvertToD3DCOLOR(long c)
{
    return (D3DCOLOR)c | (0xff<<24);        // set alpha to 1
}

//////////////////////////////////////////////////////////////////////
//
// Print a polygon
//
HRESULT PrintPolyVertices(MODEL_POLY_LOAD *pPoly, int *piVert, MODEL_VERTEX_LOAD *rVert, int indent)
{
    ValidatePoly(pPoly, rVert);
    
    XDXIndent(indent);
    XDXPrintf("<v index=\"%d\">", (*piVert)++);
    XDXPrintf("%g %g %g", rVert[pPoly->vi0].x, rVert[pPoly->vi0].y, rVert[pPoly->vi0].z);
    XDXPrintf(" %g %g %g", rVert[pPoly->vi0].nx, rVert[pPoly->vi0].ny, rVert[pPoly->vi0].nz);
    XDXPrintf(" 0x%.08x", ConvertToD3DCOLOR(pPoly->c0));
    if (pPoly->Tpage != -1)
        XDXPrintf(" %g %g", pPoly->u0, pPoly->v0);
    XDXPrintf("</v>\n");

    XDXIndent(indent);
    XDXPrintf("<v index=\"%d\">", (*piVert)++);
    XDXPrintf("%g %g %g", rVert[pPoly->vi1].x, rVert[pPoly->vi1].y, rVert[pPoly->vi1].z);
    XDXPrintf(" %g %g %g", rVert[pPoly->vi1].nx, rVert[pPoly->vi1].ny, rVert[pPoly->vi1].nz);
    XDXPrintf(" 0x%.08x", ConvertToD3DCOLOR(pPoly->c1));
    if (pPoly->Tpage != -1)
        XDXPrintf(" %g %g", pPoly->u1, pPoly->v1);
    XDXPrintf("</v>\n");
    
    XDXIndent(indent);
    XDXPrintf("<v index=\"%d\">", (*piVert)++);
    XDXPrintf("%g %g %g", rVert[pPoly->vi2].x, rVert[pPoly->vi2].y, rVert[pPoly->vi2].z);
    XDXPrintf(" %g %g %g", rVert[pPoly->vi2].nx, rVert[pPoly->vi2].ny, rVert[pPoly->vi2].nz);
    XDXPrintf(" 0x%.08x", ConvertToD3DCOLOR(pPoly->c2));
    if (pPoly->Tpage != -1)
        XDXPrintf(" %g %g", pPoly->u2, pPoly->v2);
    XDXPrintf("</v>\n");

    if (pPoly->Type & POLY_QUAD)
    {
        XDXIndent(indent);
        XDXPrintf("<v index=\"%d\">", (*piVert)++);
        XDXPrintf("%g %g %g", rVert[pPoly->vi3].x, rVert[pPoly->vi3].y, rVert[pPoly->vi3].z);
        XDXPrintf(" %g %g %g", rVert[pPoly->vi3].nx, rVert[pPoly->vi3].ny, rVert[pPoly->vi3].nz);
        XDXPrintf(" 0x%.08x", ConvertToD3DCOLOR(pPoly->c3));
        if (pPoly->Tpage != -1)
            XDXPrintf(" %g %g", pPoly->u3, pPoly->v3);
        XDXPrintf("</v>\n");
    }
    
    return S_OK;
}

HRESULT ExportModel(char *file, char tpage, char prmlevel, char loadflag, long RgbPer)
{
    // get the name of the output file
    CHAR path[_MAX_PATH];
    CHAR drive[_MAX_DRIVE];
    CHAR dir[_MAX_DIR];
    CHAR name[_MAX_PATH];
    CHAR ext[_MAX_EXT];
    _splitpath( file, drive, dir, name, ext );
    _makepath( path, drive, dir, name, ".xdx");

    // Open the XDX file
    if (XDXBegin( path ) != S_OK)
        return E_FAIL;

    // Read the .prm or .m file
    FILE *fp;
    MODEL_HEADER mh;
    MODEL_POLY_LOAD *rPoly;
    MODEL_VERTEX_LOAD *rVert;
    long i, count;
    char prm;

    // open file for reading
    if (file == NULL || file[0] == '\0')
        return E_FAIL;
    fp = fopen(file, "rb");
    if (fp == NULL)
        return E_FAIL;

    // loop thru prm count
    int indent = 0;
    count = 0;
    for (prm = 0 ; prm < prmlevel ; prm++, count++)
    {
        // get header info
        fread(&mh, sizeof(mh), 1, fp);
        
        // break on end of file
        if (feof(fp))
            break;

        // load the polygons
        rPoly = new MODEL_POLY_LOAD [ mh.PolyNum ];
        Assert(rPoly != NULL);
        fread(rPoly, sizeof(MODEL_POLY_LOAD), mh.PolyNum, fp);

        // load the vertices
        rVert = new MODEL_VERTEX_LOAD [ mh.VertNum ];
        Assert(rVert != NULL);
        fread(rVert, sizeof(MODEL_VERTEX_LOAD), mh.VertNum, fp);

        // dump the mesh vertices, combining position with texture coords and colors
        XDXIndent(indent);
        if (prm == 0)
            XDXPrintf("<VertexBuffer id=\"%s\" >\n", name);
        else
            XDXPrintf("<VertexBuffer id=\"%s%d\"\n", name, prm);
        indent++;
        int iVert = 0;
        for (i = 0 ; i < mh.PolyNum ; i++)
        {
            if (rPoly[i].Tpage != -1)
            {
                if (loadflag & LOADMODEL_FORCE_TPAGE) rPoly[i].Tpage = tpage;
                if (loadflag & LOADMODEL_OFFSET_TPAGE) rPoly[i].Tpage += tpage;
            }
            PrintPolyVertices(&rPoly[i], &iVert, rVert, indent);
        }
        indent--;
        XDXPrintf("</VertexBuffer>");

        // dump the mesh faces, with material attributes
        XDXIndent(indent);
        if (prm == 0)
            XDXPrintf("<Mesh id=\"%s\" >\n", name);
        else
            XDXPrintf("<Mesh id=\"%s%d\"\n", name, prm);
        indent++;
        iVert = 0;
        for (i = 0 ; i < mh.PolyNum ; i++)
        {
            CHAR *strMaterial;
            if (rPoly[i].Tpage == -1)
            {
                if (rPoly[i].Type & POLY_SEMITRANS)
                    strMaterial = "transparent_rgb";
                else
                    strMaterial = "opaque_rgb";
            }
            else
            {
                static CHAR buf[100];
                if (rPoly[i].Type & POLY_SEMITRANS)
                    sprintf(buf, "transparent_texture%d", rPoly[i].Tpage);
                else
                    sprintf(buf, "opaque_texture%d", rPoly[i].Tpage);
                strMaterial = buf;
            }
            XDXIndent(indent);
            if (rPoly[i].Type & POLY_QUAD)
            {
                XDXPrintf("<face material=\"%s\">%d %d %d %d</face>\n",
                          strMaterial, iVert, iVert + 1, iVert + 2, iVert + 3);
                iVert += 4;
            }
            else
            {
                XDXPrintf("<face material=\"%s\">%d %d %d</face>\n",
                          strMaterial, iVert, iVert + 1, iVert + 2);
                iVert += 3;
            }
        }

        indent--;
        XDXIndent(indent);
        XDXPrintf("</Mesh>\n");

        delete [] rVert;
        delete [] rPoly;
    }

    // close file
    fclose(fp);

    // close the XDX file
    XDXEnd();

    return S_OK;
}


//////////////////////////////////////////////////////////////////////
//
// Compute face normal
//
HRESULT ComputeFaceNormal(float *pnormal, WORLD_POLY *mp)
{
    D3DXVECTOR3 v0( mp->v0->x, mp->v0->y, mp->v0->z );
    D3DXVECTOR3 v1( mp->v1->x, mp->v1->y, mp->v1->z );
    D3DXVECTOR3 v2( mp->v2->x, mp->v2->y, mp->v2->z );
    D3DXVECTOR3 e1 = v1 - v0;
    D3DXVECTOR3 e2 = v2 - v0;
    D3DXVECTOR3 normal;
    D3DXVec3Cross(&normal, &e1, &e2);
    D3DXVec3Normalize(&normal, &normal);
    pnormal[0] = normal.x;
    pnormal[1] = normal.y;
    pnormal[2] = normal.z;
    return S_OK;
}

//////////////////////////////////////////////////////////////////////
//
// Check if polygon normal is invalid and fix if possible.
//
HRESULT ValidateNormal(float *pnormal, WORLD_POLY *mp)
{
    if (_isnan(pnormal[0]) 
        || _isnan(pnormal[1]) 
        || _isnan(pnormal[2]))
    {
        // Try using face normal instead.
        XDXPrintf("<!-- Replacing invalid normal %g %g %g -->\n",
            pnormal[0], pnormal[1], pnormal[2]);
        ComputeFaceNormal(pnormal, mp);
        Assert(!_isnan(pnormal[0]) && !_isnan(pnormal[1]) && !_isnan(pnormal[0]));
        return S_FALSE;
    }
    return S_OK;
}


//////////////////////////////////////////////////////////////////////
//
// Check a polygon for invalid data, and fix if possible.
//
HRESULT ValidatePoly(WORLD_POLY *mp)
{
    HRESULT hr = S_OK;
    Assert(!_isnan(mp->v0->x) && !_isnan(mp->v0->y) && !_isnan(mp->v0->z));
    if (ValidateNormal(&mp->v0->nx, mp) != S_OK) hr = S_FALSE;
    if (mp->Tpage != -1)
        Assert(!_isnan(mp->tu0) && !_isnan(mp->tv0));
    
    Assert(!_isnan(mp->v1->x) && !_isnan(mp->v1->y) && !_isnan(mp->v1->z));
    if (ValidateNormal(&mp->v1->nx, mp) != S_OK) hr = S_FALSE;
    if (mp->Tpage != -1)
        Assert(!_isnan(mp->tu1) && !_isnan(mp->tv1));
    
    Assert(!_isnan(mp->v2->x) && !_isnan(mp->v2->y) && !_isnan(mp->v2->z));
    if (ValidateNormal(&mp->v2->nx, mp) != S_OK) hr = S_FALSE;
    if (mp->Tpage != -1)
        Assert(!_isnan(mp->tu2) && !_isnan(mp->tv2));
    
    if (mp->Type & POLY_QUAD)
    {
        Assert(!_isnan(mp->v3->x) && !_isnan(mp->v3->y) && !_isnan(mp->v3->z));
        if (ValidateNormal(&mp->v3->nx, mp) != S_OK) hr = S_FALSE;
        if (mp->Tpage != -1)
            Assert(!_isnan(mp->tu3) && !_isnan(mp->tv3));
    }

    // TODO: Validate colors
    // TODO: If vertex alpha's are zero, make sure this is not a transparent polygon
    return hr;
}

//////////////////////////////////////////////////////////////////////
//
// Print a polygon
//
HRESULT PrintPolyVertices(WORLD_POLY *mp, int *piVertex, int indent)
{
    ValidatePoly(mp);
    
    XDXIndent(indent);
    XDXPrintf("<v index=\"%d\">", (*piVertex)++);
    XDXPrintf("%g %g %g", mp->v0->x, mp->v0->y, mp->v0->z);
    XDXPrintf(" %g %g %g", mp->v0->nx, mp->v0->ny, mp->v0->nz);
    XDXPrintf(" 0x%.08x", ConvertToD3DCOLOR(mp->rgb0));
    if (mp->Tpage != -1)
        XDXPrintf(" %g %g", mp->tu0, mp->tv0);
    else
        XDXPrintf(" 0 0");
    XDXPrintf("</v>\n");

    XDXIndent(indent);
    XDXPrintf("<v index=\"%d\">", (*piVertex)++);
    XDXPrintf("%g %g %g", mp->v1->x, mp->v1->y, mp->v1->z);
    XDXPrintf(" %g %g %g", mp->v1->nx, mp->v1->ny, mp->v1->nz);
    XDXPrintf(" 0x%.08x", ConvertToD3DCOLOR(mp->rgb1));
    if (mp->Tpage != -1)
        XDXPrintf(" %g %g", mp->tu1, mp->tv1);
    else
        XDXPrintf(" 0 0");
    XDXPrintf("</v>\n");
    
    XDXIndent(indent);
    XDXPrintf("<v index=\"%d\">", (*piVertex)++);
    XDXPrintf("%g %g %g", mp->v2->x, mp->v2->y, mp->v2->z);
    XDXPrintf(" %g %g %g", mp->v2->nx, mp->v2->ny, mp->v2->nz);
    XDXPrintf(" 0x%.08x", ConvertToD3DCOLOR(mp->rgb2));
    if (mp->Tpage != -1)
        XDXPrintf(" %g %g", mp->tu2, mp->tv2);
    else
        XDXPrintf(" 0 0");
    XDXPrintf("</v>\n");

    if (mp->Type & POLY_QUAD)
    {
        XDXIndent(indent);
        XDXPrintf("<v index=\"%d\">", (*piVertex)++);
        XDXPrintf("%g %g %g", mp->v3->x, mp->v3->y, mp->v3->z);
        XDXPrintf(" %g %g %g", mp->v3->nx, mp->v3->ny, mp->v3->nz);
        XDXPrintf(" 0x%.08x", ConvertToD3DCOLOR(mp->rgb3));
        if (mp->Tpage != -1)
            XDXPrintf(" %g %g", mp->tu3, mp->tv3);
        else
            XDXPrintf(" 0 0");
        XDXPrintf("</v>\n");
    }
    
    return S_OK;
}

//////////////////////////////////////////////////////////////////////
// 
// Helper classes for counting texture use as opaque or as transparent
//
struct TextureCounts
{
    // Set by CountVerticesAndFacesPerTexture:
    UINT OpaqueQuad;
    UINT OpaqueTri;
    UINT TransparentQuad;
    UINT TransparentTri;
    TextureCounts()
    {
        clear();
    }
    void clear()
    {
        OpaqueQuad = 0;
        OpaqueTri = 0;
        TransparentQuad = 0;
        TransparentTri = 0;
    }
};
typedef std::map<short, TextureCounts> TextureTracker;
const int MaxNameLength = 1000;

//////////////////////////////////////////////////////////////////////
// 
// Helper class for world exporting
//
struct WorldExporter {
    CHAR m_path[_MAX_PATH];
    CHAR m_strName[MaxNameLength];
    CHAR m_strCameraName[MaxNameLength];
    TextureTracker m_TextureTracker;
    TextureCounts m_TotalCount;
    int m_indent;

    WorldExporter(CHAR *name);
    HRESULT CountVerticesAndFacesPerTexture();
    HRESULT ExportTextureReference(CHAR *strIdentifier, CHAR *strPath, INT levels);
    HRESULT ExportTextureReferences();
    HRESULT ExportModelReference(CHAR *strPath);
    HRESULT ExportModelReferences();
    HRESULT ExportVertices();
    HRESULT ExportPasses();
    HRESULT ExportWorldCamera();
};

//////////////////////////////////////////////////////////////////////
// 
// Constructor
//
WorldExporter::WorldExporter(CHAR *file)
{
    // get the path of the output file
    CHAR drive[_MAX_DRIVE];
    CHAR dir[_MAX_DIR];
    CHAR name[_MAX_PATH];
    CHAR ext[_MAX_EXT];
    _splitpath( file, drive, dir, name, ext );
    _makepath( m_path, drive, dir, name, ".xdx");
    
    strncpy(m_strName, name, MaxNameLength);
    m_strName[MaxNameLength - 1] = 0;
    strncpy(m_strCameraName, name, MaxNameLength);
    strncat(m_strCameraName, "_Camera", MaxNameLength);
    m_strCameraName[MaxNameLength - 1] = 0;
    m_TextureTracker.clear();
    m_TotalCount.clear();
    m_indent = 0;
}

//////////////////////////////////////////////////////////////////////
// 
// Count the number of vertices, primitives,
// and unique textures used in each pass
//
HRESULT WorldExporter::CountVerticesAndFacesPerTexture()
{
    m_TotalCount.clear();
    m_TextureTracker.clear();
    for (int iCube = 0 ; iCube < World.CubeNum ; iCube++)
    {
        WORLD_MODEL *m = &World.Cube[iCube].Model;
        WORLD_POLY *mp = m->PolyPtr;
        TextureCounts *pCurrentTextureCounts;
        for (int i = 0; i < m->PolyNum; i++, mp++)
        {
            pCurrentTextureCounts = &m_TextureTracker[ mp->Tpage ];
            if (mp->Type & POLY_QUAD)
            {
                if (mp->Type & POLY_SEMITRANS)
                {
                    m_TotalCount.TransparentQuad++;
                    pCurrentTextureCounts->TransparentQuad++;
                }
                else
                {
                    m_TotalCount.OpaqueQuad++;
                    pCurrentTextureCounts->OpaqueQuad++;
                }
            }
            else
            {
                if (mp->Type & POLY_SEMITRANS)
                {
                    m_TotalCount.TransparentTri++;
                    pCurrentTextureCounts->TransparentTri++;
                }
                else
                {
                    m_TotalCount.OpaqueTri++;
                    pCurrentTextureCounts->OpaqueTri++;
                }
            }
        }
    }
    return S_OK;
}


//////////////////////////////////////////////////////////////////////
// 
// Output a texture reference
//
HRESULT ExportTextureReference(CHAR *strBasePath, INT indent, CHAR *strIdentifier, CHAR *strPath, INT levels)
{
    CHAR name2[_MAX_PATH];
    if (strIdentifier == NULL)
    {
        CHAR drive2[_MAX_DRIVE];
        CHAR dir2[_MAX_DIR];
        CHAR ext2[_MAX_EXT];
        _splitpath( strPath, drive2, dir2, name2, ext2 );
        strIdentifier = name2;
    }
    XDXIndent(indent);
    XDXPrintf("<Texture id=\"%s\" source=\"%s\" levels=\"%d\" colorkey=\"0x%0x\" />\n",
        strIdentifier, RelativePath(strBasePath, strPath), levels, 0xff000000 );
    return S_OK;
}


//////////////////////////////////////////////////////////////////////
// 
// Output a texture reference
//
HRESULT WorldExporter::ExportTextureReference(char *strIdentifier, char *strPath, int levels)
{
    return ::ExportTextureReference(m_path, m_indent, strIdentifier, strPath, levels);
}


//////////////////////////////////////////////////////////////////////
// 
// Output texture references
//
HRESULT WorldExporter::ExportTextureReferences()
{
    // Export misc textures
    ExportTextureReference("font1",    "D:\\gfx\\font1.bmp",   1);
    ExportTextureReference("font2",    "D:\\gfx\\font2.bmp",   1);
    ExportTextureReference("envstill", CurrentLevelInfo.EnvStill, 0);
    ExportTextureReference("envroll",  CurrentLevelInfo.EnvRoll, 0);
    ExportTextureReference("shadow",   "D:\\gfx\\shadow.bmp",  0);
    ExportTextureReference("fxpage1",  "D:\\gfx\\fxpage1.bmp", 0);
    ExportTextureReference("fxpage2",  "D:\\gfx\\fxpage2.bmp", 0);
    ExportTextureReference("fxpage3",  "D:\\gfx\\fxpage3.bmp", 0);

    // Export world textures
    for (int ii = 0 ; ii < TPAGE_WORLD_NUM ; ii++)
    {
        char strIdentifier[128];
        char buf[128];
        wsprintf(strIdentifier, "%s%c", CurrentLevelInfo.Dir, ii + 'a');
        wsprintf(buf, "D:\\levels\\%s\\%s%c.bmp", CurrentLevelInfo.Dir, CurrentLevelInfo.Dir, ii + 'a');
        DWORD FileAttrib = GetFileAttributes( buf );
        if (FileAttrib == 0xffffffff)
            break;
        ExportTextureReference(strIdentifier, buf, 0);
    }

    // Check to see if all the world textures are used
    TextureTracker::iterator iTextureTracker;
    for (iTextureTracker = m_TextureTracker.begin();
         iTextureTracker != m_TextureTracker.end();
         iTextureTracker++)
    {
        short Tpage = (*iTextureTracker).first; // current texture page
        TextureCounts *pCurrentTextureCounts = &(*iTextureTracker).second;  // count of quads and tris
        XDXIndent(m_indent);
        XDXPrintf("<!-- Tpage=\"%d\" File=\"%s\" OpaqueQuad=\"%d\" OpaqueTri=\"%d\" TransparentQuad=\"%d\" TransparentTri=\"%d\" -->\n", 
            Tpage, TexInfo[Tpage].File, pCurrentTextureCounts->OpaqueQuad, pCurrentTextureCounts->OpaqueTri, pCurrentTextureCounts->TransparentQuad, pCurrentTextureCounts->TransparentTri);
        if (Tpage == -1)
            continue;           // skip NULL texture
    }
    return S_OK;
}
            



//////////////////////////////////////////////////////////////////////
// 
// Output model references
//
HRESULT WorldExporter::ExportModelReference(CHAR *strPath)
{
    XDXIndent(m_indent);
    XDXPrintf("<include href=\"%s\" />\n", RelativePath(m_path, strPath));
    return S_OK;
}




//////////////////////////////////////////////////////////////////////
// 
// Keep track of full instance model paths
//
CHAR g_InstanceModelPaths[MAX_INSTANCE_MODELS][_MAX_PATH];  // $MODIFIED jedl - keep full instance model paths around for exporter

HRESULT ExportInstanceModel(INT InstanceModelIndex, CHAR *file, MODEL *pModel, INT ModelCount)
{
    CHAR path[_MAX_PATH];
    CHAR drive[_MAX_DRIVE];
    CHAR dir[_MAX_DIR];
    CHAR name[_MAX_PATH];
    CHAR ext[_MAX_EXT];
    _splitpath( file, drive, dir, name, ext );
    _makepath( path, drive, dir, name, ".xdx");
    CHAR strMessage[1000];
    sprintf(strMessage, "Exporting instance model \"%s\".", path);
    DumpMessage(NULL, strMessage);
    strcpy(g_InstanceModelPaths[InstanceModelIndex], path);
    return ExportModel(path, pModel, ModelCount);
}




//////////////////////////////////////////////////////////////////////
// 
// Export level model
//
HRESULT ExportLevelModel(INT LevelModelIndex, CHAR *file, MODEL *pModel, INT ModelCount)
{
    CHAR path[_MAX_PATH];
    CHAR drive[_MAX_DRIVE];
    CHAR dir[_MAX_DIR];
    CHAR name[_MAX_PATH];
    CHAR ext[_MAX_EXT];
    _splitpath( file, drive, dir, name, ext );
    _makepath( path, drive, dir, name, ".xdx");
    CHAR strMessage[1000];
    sprintf(strMessage, "Exporting level model \"%s\".", path);
    DumpMessage(NULL, strMessage);
    return ExportModel(path, pModel, 1);
}



//////////////////////////////////////////////////////////////////////
// 
// Output model references
//
HRESULT WorldExporter::ExportModelReferences()
{
    CHAR buf[_MAX_PATH];
    
    // static models
    extern CHAR *LevelModelList[];
    XDXIndent(m_indent);
    XDXPrintf("<!-- static level models -->\n");
    sprintf(buf, "%s.xdx", LevelModelList[LEVEL_MODEL_PICKUP]);
    ExportModelReference(buf);
    sprintf(buf, "%s.xdx", LevelModelList[LEVEL_MODEL_FIREWORK]);
    ExportModelReference(buf);
    sprintf(buf, "%s.xdx", LevelModelList[LEVEL_MODEL_WATERBOMB]);
    ExportModelReference(buf);
    sprintf(buf, "%s.xdx", LevelModelList[LEVEL_MODEL_CHROMEBALL]);
    ExportModelReference(buf);
// Apparently, bombball and chromeball are the same model.
//  sprintf(buf, "%s.xdx", LevelModelList[LEVEL_MODEL_BOMBBALL]);
//  ExportModelReference(buf);

    // TODO: what about all the other level models?

    // instance models
    XDXIndent(m_indent);
    XDXPrintf("<!-- InstanceModelNum=\"%d\" -->\n", InstanceModelNum);
    for (int i = 0 ; i < InstanceModelNum ; i++)
        ExportModelReference(g_InstanceModelPaths[i]);
    return S_OK;
}


//////////////////////////////////////////////////////////////////////
// 
// Export world model vertices to XDX
//
HRESULT WorldExporter::ExportVertices()
{
    // Dump vertices sorted by opaque/transparent and by texture and by quad/tri
    for (int iTransparent = 0; iTransparent < 2; iTransparent++)
    {
        if (iTransparent)
        {
            if (m_TotalCount.TransparentQuad == 0
                && m_TotalCount.TransparentTri == 0)
                continue;
        }
        else
        {
            if (m_TotalCount.OpaqueQuad == 0
                && m_TotalCount.OpaqueTri == 0)
                continue;
        }
        XDXIndent(m_indent);
        XDXPrintf("<VertexBuffer id=\"%s_VertexBuffer_%s\"\n", m_strName, iTransparent ? "Transparent" : "Opaque" );
        XDXIndent(m_indent);
        XDXPrintf(" names=\"POSITION NORMAL DIFFUSE TEXCOORD0\"\n");
        XDXIndent(m_indent);
        XDXPrintf(" format=\"FLOAT3 FLOAT3 D3DCOLOR FLOAT2\"");
        XDXPrintf(">\n");
        int iVertex = 0;
        TextureTracker::iterator iTextureTracker;
        for (iTextureTracker = m_TextureTracker.begin();
             iTextureTracker != m_TextureTracker.end();
             iTextureTracker++)
        {
            short Tpage = (*iTextureTracker).first; // current texture page
            TextureCounts *pCurrentTextureCounts = &(*iTextureTracker).second;  // count of quads and tris
            for (int iTri = 0; iTri < 2; iTri++)
            {
                // Print comment
                if (iTransparent)
                {
                    if (!iTri)
                    {
                        if (pCurrentTextureCounts->TransparentQuad == 0)
                            continue;
                        XDXIndent(m_indent);
                        XDXPrintf("<!-- Tpage=\"%d\" TransparentQuad=\"%d\" -->\n", Tpage, pCurrentTextureCounts->TransparentQuad);
                    }
                    else
                    {
                        if (pCurrentTextureCounts->TransparentTri == 0)
                            continue;
                        XDXIndent(m_indent);
                        XDXPrintf("<!-- Tpage=\"%d\" TransparentTri=\"%d\" -->\n", Tpage, pCurrentTextureCounts->TransparentTri);
                    }
                }
                else
                {
                    if (!iTri)
                    {
                        if (pCurrentTextureCounts->OpaqueQuad == 0)
                            continue;
                        XDXIndent(m_indent);
                        XDXPrintf("<!-- Tpage=\"%d\" OpaqueQuad=\"%d\" -->\n", Tpage, pCurrentTextureCounts->OpaqueQuad);
                    }
                    else 
                    {
                        if (pCurrentTextureCounts->OpaqueTri == 0)
                            continue;
                        XDXIndent(m_indent);
                        XDXPrintf("<!-- Tpage=\"%d\" OpaqueTri=\"%d\" -->\n", Tpage, pCurrentTextureCounts->OpaqueTri);
                    }
                }

                // Dump vertices that match texture and draw mode
                for (int iCube = 0 ; iCube < World.CubeNum ; iCube++)
                {
                    WORLD_MODEL *m = &World.Cube[iCube].Model;
                    WORLD_POLY *mp = m->PolyPtr;
                    for (int i = 0; i < m->PolyNum; i++, mp++)
                    {
                        if (mp->Tpage != Tpage) continue;   // skip if doesn't match current texture
                        if (!iTransparent && (mp->Type & POLY_SEMITRANS)) continue;
                        if (iTransparent && !(mp->Type & POLY_SEMITRANS)) continue;
                        if (!iTri && !(mp->Type & POLY_QUAD)) continue;
                        if (iTri && (mp->Type & POLY_QUAD)) continue;
                        PrintPolyVertices(mp, &iVertex, m_indent + 1);
                    }
                }
            }
        }
        XDXIndent(m_indent);
        XDXPrintf("</VertexBuffer>\n");
    }
    return S_OK;
}

//////////////////////////////////////////////////////////////////////
// 
// Export a world model to XDX
//
HRESULT WorldExporter::ExportPasses()
{
    extern WORLD World;
    XDXIndent(m_indent);
    XDXPrintf("<Model id=\"%s\">\n", m_strName);
    m_indent++;

    D3DXMATRIX matIdentity;
    D3DXMatrixIdentity(&matIdentity);
    D3DXMATRIX matCamera = matIdentity;
    D3DXVECTOR4 diffuse(1.f, 1.f, 1.f, 1.f);
    D3DXVECTOR4 lightdir(0.f, 0.f, 1.f, 1.f);

    // Dump out draw passes
    for (int iTransparent = 0; iTransparent < 2; iTransparent++)
    {
        if (iTransparent)
        {
            if (m_TotalCount.TransparentQuad == 0
                && m_TotalCount.TransparentTri == 0)
                continue;
        }
        else
        {
            if (m_TotalCount.OpaqueQuad == 0
                && m_TotalCount.OpaqueTri == 0)
                continue;
        }

        // Set default state
        XDXIndent(m_indent);
        XDXPrintf("<!-- Set default state -->\n");
        XDXIndent(m_indent);
        XDXPrintf("<Pass>\n");
        m_indent++;
        XDXIndent(m_indent);
        XDXPrintf("<VertexShader idref=\"VSLightDiffuseTexture\">\n");
        m_indent++;
        XDXIndent(m_indent);
        XDXPrintf("<decl>\n");
        XDXIndent(m_indent);
        XDXPrintf("<stream index=\"0\" idref=\"%s_VertexBuffer_%s\" />\n", m_strName, iTransparent ? "Transparent" : "Opaque" );
        XDXIndent(m_indent);
        XDXPrintf("</decl>\n");
        XDXSetVertexShaderConstant(0, &diffuse, 1, "Diffuse", NULL, m_indent);
        XDXSetVertexShaderConstant(1, &lightdir, 1, "LightDirection", NULL, m_indent);
        XDXSetVertexShaderConstant(4, &matCamera, 4, "World * View * Projection", m_strCameraName, m_indent);
        XDXSetVertexShaderConstant(8, &matIdentity, 3, "Transpose(Inverse(World))", NULL, m_indent);
        m_indent--;
        XDXIndent(m_indent);
        XDXPrintf("</VertexShader>\n");
        /*
        XDXIndent(m_indent);
        XDXPrintf("<RenderState>\n");
        XDXIndent(m_indent);
        XDXPrintf("<TextureState Stage=\"0\" idref=\"NULL\"\n");
        m_indent++;
        XDXIndent(m_indent);
        XDXPrintf("MinFilter=\"Anisotropic\"\n");
        XDXIndent(m_indent);
        XDXPrintf("MagFilter=\"Anisotropic\"\n");
        XDXIndent(m_indent);
        XDXPrintf("MaxAnisotropy=\"4\"\n");
        XDXIndent(m_indent);
        XDXPrintf("MipFilter=\"Linear\"\n");
        XDXIndent(m_indent);
        XDXPrintf("AddressU=\"Wrap\"\n");
        XDXIndent(m_indent);
        XDXPrintf("AddressV=\"Wrap\"\n");
        XDXIndent(m_indent);
        XDXPrintf("/>\n");
        m_indent--;
        XDXIndent(m_indent);
        XDXPrintf("<TextureState Stage=\"1\"");
        XDXPrintf(" ColorOp=\"Disable\"");
        XDXPrintf(" AlphaOp=\"Disable\"");
        XDXPrintf("/>\n");
        XDXIndent(m_indent);
        XDXPrintf("</RenderState>\n");
        */
        m_indent--;
        XDXIndent(m_indent);
        XDXPrintf("</Pass>\n");

        // One pass for each texture
        UINT iVertex = 0;
        TextureTracker::iterator iTextureTracker;
        for (iTextureTracker = m_TextureTracker.begin();
             iTextureTracker != m_TextureTracker.end();
             iTextureTracker++)
        {
            short Tpage = (*iTextureTracker).first; // current texture page to output
            TextureCounts *pCurrentTextureCounts = &(*iTextureTracker).second;  // count of quads and tris
            UINT QuadCount, TriCount;
            if (iTransparent)
            {
                QuadCount = pCurrentTextureCounts->TransparentQuad;
                TriCount = pCurrentTextureCounts->TransparentTri;
            }
            else
            {
                QuadCount = pCurrentTextureCounts->OpaqueQuad;
                TriCount = pCurrentTextureCounts->OpaqueTri;
            }
            if (QuadCount == 0
                && TriCount == 0)
                continue;
            XDXIndent(m_indent);
            XDXPrintf("<Pass>\n");
            m_indent++;

            XDXIndent(m_indent);
            XDXPrintf("<RenderState>\n");
            m_indent++;
            if (Tpage == -1)
            {
                XDXIndent(m_indent);
                XDXPrintf("<TextureState Stage=\"0\" idref=\"NULL\"\n");
                m_indent++;
                XDXIndent(m_indent);
                XDXPrintf("ColorOp=\"SelectArg1\"\n");
                XDXIndent(m_indent);
                XDXPrintf("ColorArg1=\"Diffuse\"\n");
                XDXIndent(m_indent);
                XDXPrintf("AlphaOp=\"SelectArg1\"\n");
                XDXIndent(m_indent);
                XDXPrintf("AlphaArg1=\"Diffuse\"\n");
                m_indent--;
                XDXIndent(m_indent);
                XDXPrintf("/>\n");
            }
            else
            {
                XDXIndent(m_indent);
                XDXPrintf("<TextureState Stage=\"0\" idref=\"%s\"\n", TextureIdentifier(Tpage));
                m_indent++;
                XDXIndent(m_indent);
                XDXPrintf("ColorOp=\"Modulate\"\n");
                XDXIndent(m_indent);
                XDXPrintf("ColorArg1=\"Diffuse\"\n");
                XDXIndent(m_indent);
                XDXPrintf("ColorArg2=\"Texture\"\n");
                XDXIndent(m_indent);
                XDXPrintf("AlphaOp=\"Modulate\"\n");
                XDXIndent(m_indent);
                XDXPrintf("AlphaArg1=\"Diffuse\"\n");
                XDXIndent(m_indent);
                XDXPrintf("AlphaArg2=\"Texture\"\n");
                m_indent--;
                XDXIndent(m_indent);
                XDXPrintf("/>\n");
            }
            m_indent--;
            XDXIndent(m_indent);
            XDXPrintf("</RenderState>\n");

            // Draw commands.  
            //XDXPrintf("<!-- The start and count attributes are vertex counts, not primitive counts. -->\n");
            if (QuadCount > 0)
            {
                XDXIndent(m_indent);
                XDXPrintf("<Draw primitive=\"QUADLIST\" start=\"%d\" count=\"%d\" />\n",
                          iVertex, QuadCount * 4);
                iVertex += QuadCount * 4;
            }
            if (TriCount > 0)
            {
                XDXIndent(m_indent);
                XDXPrintf("<Draw primitive=\"TRILIST\" start=\"%d\" count=\"%d\" />\n",
                          iVertex, TriCount * 3);
                iVertex += TriCount * 3;
            }
            
            m_indent--;
            XDXIndent(m_indent);
            XDXPrintf("</Pass>\n");
        }
    }
    m_indent--;
    XDXIndent(m_indent);
    XDXPrintf("</Model>\n");

    return S_OK;
}

//////////////////////////////////////////////////////////////////////
// 
// Export a world model to XDX
//
HRESULT WorldExporter::ExportWorldCamera()
{
    /*
    // Jump to the center of each cube, then do a 360 degree view
    int iCube;
    XDXIndent(m_indent);
    XDXPrintf("<Frame id=\"%s\">\n", m_strCameraName);
    m_indent++;
    XDXIndent(m_indent);
    XDXPrintf("<Matrix>\n");
    m_indent++;
    XDXIndent(m_indent);
    XDXPrintf("<Translate value=\"0 0 0.5\" />\n");
    XDXIndent(m_indent);
    XDXPrintf("<Scale value=\"1 1 0.5\" />\n");
    XDXIndent(m_indent);
    XDXPrintf("<!-- Rotate around Y axis for preview -->\n");
    XDXIndent(m_indent);
    XDXPrintf("<Rotate angle=\"0\" axis=\"0 1 0\">\n");
    XDXIndent(m_indent + 1);
    XDXPrintf("<animate attribute=\"angle\" begin=\"0\" dur=\"1\" values=\"0;3.141592654;6.283185307;\" />\n");
    XDXIndent(m_indent);
    XDXPrintf("</Rotate>\n");
    XDXIndent(m_indent);
    XDXPrintf("<!-- Point Y up -->\n");
    XDXIndent(m_indent);
    XDXPrintf("<Rotate angle=\"3.141592654\" axis=\"1 0 0\" />\n");
    
    XDXIndent(m_indent);
    XDXPrintf("<!-- Scale cube by 1/radius = 1/%f -->\n", World.Cube[0].Radius);
    float fScale = 1.f / World.Cube[0].Radius;
    XDXIndent(m_indent);
    XDXPrintf("<Scale value=\"%f %f %f\">\n", fScale, fScale, fScale);
    m_indent++;
    XDXIndent(m_indent);
    XDXPrintf("<animate begin=\"0\" dur=\"%f\" values=\"\n", 1.f * World.CubeNum);
    m_indent++;
    for (iCube = 0 ; iCube < World.CubeNum ; iCube++)
    {
        CUBE_HEADER *pCube = &World.Cube[iCube];
        float fScale = 1.f / pCube->Radius;
        XDXIndent(m_indent);
        XDXPrintf("%f %f %f;\n", fScale, fScale, fScale);
    }
    XDXIndent(m_indent);
    XDXPrintf("\"/>\n");
    m_indent--;
    m_indent--;
    XDXIndent(m_indent);
    XDXPrintf("</Scale>\n");
    
    XDXIndent(m_indent);
    XDXPrintf("<!-- Translate cube center to origin -->\n");
    XDXIndent(m_indent);
    XDXPrintf("<Translate value=\"%f %f %f\">\n", -World.Cube[0].CentreX, -World.Cube[0].CentreY, -World.Cube[0].CentreZ);
    m_indent++;
    XDXIndent(m_indent);
    XDXPrintf("<animate begin=\"0\" dur=\"%f\" values=\"\n", 1.f * World.CubeNum);
    m_indent++;
    for (iCube = 0 ; iCube < World.CubeNum ; iCube++)
    {
        CUBE_HEADER *pCube = &World.Cube[iCube];
        XDXIndent(m_indent);
        XDXPrintf("%f %f %f;\n", -pCube->CentreX, -pCube->CentreY, -pCube->CentreZ );
    }
    XDXIndent(m_indent);
    XDXPrintf("\"/>\n");
    m_indent--;
    m_indent--;
    XDXIndent(m_indent);
    XDXPrintf("</Translate>\n");
    
    m_indent--;
    XDXIndent(m_indent);
    XDXPrintf("</Matrix>\n");
    m_indent--;
    XDXIndent(m_indent);
    XDXPrintf("</Frame>\n");

    */

    /*
    // Rotate around entire level
    XDXIndent(m_indent);
    XDXPrintf("<Frame id=\"%s\">\n", m_strCameraName);
    m_indent++;
    XDXIndent(m_indent);
    XDXPrintf("<Matrix>\n");
    m_indent++;
    XDXIndent(m_indent);
    XDXPrintf("<Translate value=\"0 0 0.5\" />\n");
    XDXIndent(m_indent);
    XDXPrintf("<Scale value=\"1 1 0.5\" />\n");
    XDXIndent(m_indent);
    XDXPrintf("<!-- Rotate around Y axis for preview -->\n");
    XDXIndent(m_indent);
    XDXPrintf("<Rotate angle=\"0\" axis=\"0 1 0\">\n");
    XDXIndent(m_indent + 1);
    XDXPrintf("<animate attribute=\"angle\" begin=\"0\" dur=\"1\" values=\"0;3.141592654;6.283185307;\" />\n");
    XDXIndent(m_indent);
    XDXPrintf("</Rotate>\n");
    XDXIndent(m_indent);
    XDXPrintf("<!-- Point Y up -->\n");
    XDXIndent(m_indent);
    XDXPrintf("<Rotate angle=\"3.141592654\" axis=\"1 0 0\" />\n");
    
    XDXIndent(m_indent);
    float fRadius = World.BigCube->Radius;
    XDXPrintf("<!-- Scale by 1/radius = 1/%f -->\n", fRadius);
    float fScale = 1.f / fRadius;
    XDXIndent(m_indent);
    XDXPrintf("<Scale value=\"%f %f %f\" />\n", fScale, fScale, fScale);
    
    XDXIndent(m_indent);
    XDXPrintf("<!-- Translate center to origin -->\n");
    XDXIndent(m_indent);
    XDXPrintf("<Translate value=\"%f %f %f\" />\n", -World.BigCube->x, -World.BigCube->y, -World.BigCube->z);
    
    m_indent--;
    XDXIndent(m_indent);
    XDXPrintf("</Matrix>\n");
    m_indent--;
    XDXIndent(m_indent);
    XDXPrintf("</Frame>\n");
    */

    return S_OK;
}
    


//////////////////////////////////////////////////////////////////////
//
// Export world in XDX format
//
HRESULT ExportWorld(char *file)
{
    HRESULT hr = S_OK;
    
    extern WORLD World;
    extern short WorldBigCubeCount, WorldCubeCount, WorldPolyCount, WorldDrawnCount;
//  extern long TexAnimNum;
//  extern TEXANIM_HEADER TexAnim[MAX_TEXANIMS];

    WorldExporter WE(file);

    // Put vertices into a separate file
    CHAR strVertexPath[_MAX_PATH];
    CHAR drive[_MAX_DRIVE];
    CHAR dir[_MAX_DIR];
    CHAR name[_MAX_PATH];
    CHAR ext[_MAX_EXT];
    _splitpath( WE.m_path, drive, dir, name, ext );
    strcat(name, "_vertex");
    _makepath( strVertexPath, drive, dir, name, ".xdx");

    // Output main world file
    if (XDXBegin( WE.m_path ) != S_OK)
        return E_FAIL;
    XDXPrintf("<!-- converted from %s \n", file );
    XDXPrintf("WorldBigCubeCount=\"%d\" \n", WorldBigCubeCount);
    XDXPrintf("WorldCubeCount=\"%d\" \n", WorldCubeCount);
    XDXPrintf("WorldPolyCount=\"%d\" \n", WorldPolyCount);
    XDXPrintf("WorldCubeNum=\"%d\" \n", World.CubeNum);
    XDXPrintf("-->\n");
    XDXOut("<include href=\"Default.xdx\" />\n");
    WE.ExportTextureReferences();
    WE.ExportModelReferences();
    WE.CountVerticesAndFacesPerTexture();
    XDXPrintf("<!-- Put world vertices in a separate file. -->\n");
    XDXPrintf("<include href=\"%s\" />\n", RelativePath(file, strVertexPath));
    WE.ExportPasses();
    WE.ExportWorldCamera();
    XDXEnd();

    // Output vertices
    if (XDXBegin( strVertexPath ) != S_OK)
        return E_FAIL;
    WE.ExportVertices();
    XDXEnd();
    
    return hr;
}


//////////////////////////////////////////////////////////////////////
//
// Export car resources in XDX format
//
HRESULT ExportCar(CAR_INFO *pCarInfo, CAR_MODEL *pCarModel, INT Tpage)
{
    // Get path for xdx file
    CHAR path[_MAX_PATH];
    extern CHAR *GetCarResourcePath( CHAR path_xbr[], CAR_INFO *pCarInfo );
    GetCarResourcePath(path, pCarInfo);
    CHAR drive[_MAX_DRIVE];
    CHAR dir[_MAX_DIR];
    CHAR name[_MAX_PATH];
    CHAR ext[_MAX_EXT];
    _splitpath( path, drive, dir, name, ext );
    _makepath( path, drive, dir, name, ".xdx");

    // Output car models
    CHAR strMessage[1000];
    sprintf(strMessage, "Exporting car \"%s\" to \"%s\".", pCarInfo->Name, path);
    DumpMessage(NULL, strMessage);
    for (int i = 0 ; i < MAX_CAR_MODEL_TYPES; i++)
    {
        if (strlen(pCarInfo->ModelFile[i]))
        {
            CHAR path2[_MAX_PATH];
            CHAR drive2[_MAX_DRIVE];
            CHAR dir2[_MAX_DIR];
            CHAR name2[_MAX_PATH];
            CHAR ext2[_MAX_EXT];
            _splitpath( pCarInfo->ModelFile[i], drive2, dir2, name2, ext2 );
            _makepath( path2, drive2, dir2, name2, ".xdx");
            sprintf(strMessage, "Exporting car model \"%s\".", path2);
            DumpMessage(NULL, strMessage);
            ExportModel(pCarInfo->ModelFile[i], pCarModel->Model[i], MAX_CAR_LOD);
        }
    }

    // Output references to car textures and models
    if (XDXBegin( path ) != S_OK)
        return E_FAIL;
    XDXPrintf("<include href=\"Default.xdx\" />\n");
    ExportTextureReference(path, 0, NULL, TexInfo[Tpage].File, 0);      // main car texture
    ExportTextureReference(path, 0, NULL, TexInfo[TPAGE_FX1].File, 0);  // aerial texture
    for (i = 0 ; i < MAX_CAR_MODEL_TYPES; i++)
    {
        CHAR *strPath = pCarInfo->ModelFile[i];
        if (strlen(strPath))
        {
            CHAR path2[_MAX_PATH];
            CHAR drive2[_MAX_DRIVE];
            CHAR dir2[_MAX_DIR];
            CHAR name2[_MAX_PATH];
            CHAR ext2[_MAX_EXT];
            _splitpath( strPath, drive2, dir2, name2, ext2 );
            _makepath(path2, drive2, dir2, name2, ".xdx");
            XDXPrintf("<include href=\"%s\" />\n", RelativePath(path, path2));
        }
    }
    return XDXEnd();
}


