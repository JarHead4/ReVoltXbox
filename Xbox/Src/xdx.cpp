//-----------------------------------------------------------------------------
//  
//  File: xdx.cpp
//  Copyright (C) 2001-2002 Microsoft Corporation
//  All rights reserved.
//  
//  Convert Re-Volt data to XDX file.
//  
//-----------------------------------------------------------------------------
#ifdef SHIPPING
// Shipping version does not need exporter code.
#else
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
#include "visibox.h"

#define D3DVS_MAX_DECL_COUNT 100
#define D3DVS_MAXINSTRUCTIONCOUNT D3DVS_MAXINSTRUCTIONCOUNT_V1_0
#define D3DVS_CONSTREG_COUNT D3DVS_CONSTREG_COUNT_XBOX

struct XDX {
    FILE *m_fp; // buffered stream
    CHAR buf[4096]; // stream buffer
	INT m_indent;	// current indentation
	BOOL m_bJustAfterNewline;	// indentation should be output before next non-newline character
} *s_pXDX = NULL;

static HRESULT XDXOutRaw(CONST CHAR *str, UINT len)
{
    if (s_pXDX == NULL)
        return E_FAIL;
    if (fwrite(str, 1, len, s_pXDX->m_fp) < 0)
        return E_FAIL;
    else
        return S_OK;
}

static HRESULT XDXIndentRaw(int indent)
{
    if (s_pXDX == NULL)
        return E_FAIL;
    while (indent-- > 0)
        XDXOutRaw("\t", 1);
    return S_OK;
}

static HRESULT XDXIndentBy(int indentDelta)
{
	if (s_pXDX == NULL)
		return E_FAIL;
	s_pXDX->m_indent += indentDelta;
	if (s_pXDX->m_indent < 0)
		s_pXDX->m_indent = 0;
	return S_OK;
}

static HRESULT XDXOut(CONST CHAR *str)
{
	HRESULT hr;
    if (s_pXDX == NULL)
        return E_FAIL;

	// Look through string for newlines.  On the next character after a newline,
	// do the indentation.  That way, the indentation can be changed between lines.
	CONST CHAR *pStart = str;
	CONST CHAR *pEnd = str;
	while (*pEnd)
	{
		if (*pEnd == '\n')
		{
			hr = XDXOutRaw(pStart, pEnd - pStart + 1);
			if (FAILED(hr))
				return hr;
			pEnd++;
			pStart = pEnd;
			s_pXDX->m_bJustAfterNewline = true;
		}
		else
		{
			if (s_pXDX->m_bJustAfterNewline)
			{
				hr = XDXIndentRaw(s_pXDX->m_indent);
				if (FAILED(hr))
					return hr;
				s_pXDX->m_bJustAfterNewline = false;
			}
			pEnd++;
		}
	}
	if (pEnd != pStart)
	{
		hr = XDXOutRaw(pStart, pEnd - pStart);
		if (FAILED(hr))
			return hr;
	}
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
	s_pXDX->m_indent = 0;
	s_pXDX->m_bJustAfterNewline = false;
    
    // Write the header
    XDXOut("<?xml version=\"1.0\" ?>\n");
    XDXOut("<XDX version=\"0.6\" >\n");
    
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
    
    if (strcmp(drive2, drive) == 0	// drives match
		|| (strlen(drive2) == 0 || strlen(drive) == 0))	// or the drive was not specified (assume match)
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
                                  CONST CHAR *strName, CONST CHAR *strParameter)
{
    if (ConstantCount == 0)
        return E_INVALIDARG;
    CONST FLOAT *pfConstant = (CONST FLOAT *)pConstantData;
    XDXPrintf("<Constant index=\"%d\"", Register);
    if (strName != NULL)
        XDXPrintf(" name=\"%s\"", strName);
    if (ConstantCount > 1)
        XDXPrintf(" count=\"%d\"", ConstantCount);
    if (strParameter != NULL)
        XDXPrintf(" parameter=\"%s\"", strParameter);
    XDXPrintf(">");
	XDXIndentBy(1);
    for (UINT i = 0; i < ConstantCount; i++)
    {
        if (ConstantCount > 1)
            XDXPrintf("\n");
        XDXPrintf("%g %g %g %g", pfConstant[0], pfConstant[1], pfConstant[2], pfConstant[3]);
        pfConstant += 4;
    }
	XDXIndentBy(-1);
    XDXPrintf("</Constant>\n");
    return S_OK;
}

//////////////////////////////////////////////////////////////////////
//
// Set alpha to 1 in D3DCOLOR (no other re-arranging needed.)
//
D3DCOLOR ConvertToD3DCOLOR(long c)
{
    return (D3DCOLOR)c | (0xff<<24);        // set alpha to 1
}

D3DCOLOR ConvertToD3DCOLOR(MODEL_RGB *pRGB)
{
	// Convert alpha = 0 to alpha = 1;
	// $TODO: handle non-zero and non-one alpha
	// Assert(pRGB->a == 0);
	return (*(D3DCOLOR *)pRGB) | (0xff<<24);
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
// Print a vertex
//
HRESULT PrintModelVertex(UINT *piVertex, float *pPosition, float *pNormal, MODEL_RGB *pModelRGB, float *pUV, bool bFlipNormal)
{
    XDXPrintf("<v index=\"%d\">", (*piVertex)++);
    XDXPrintf("%g %g %g", pPosition[0], pPosition[1], pPosition[2]);
    if (bFlipNormal)
        XDXPrintf(" %g %g %g", -pNormal[0], -pNormal[1], -pNormal[2]);
    else
        XDXPrintf(" %g %g %g", pNormal[0], pNormal[1], pNormal[2]);
    XDXPrintf(" 0x%.08x", ConvertToD3DCOLOR(pModelRGB));
    if (pUV != NULL)
        XDXPrintf(" %g %g", pUV[0], pUV[1]);
    else
        XDXPrintf(" 0 0");
    XDXPrintf("</v>\n");
    return S_OK;
}
    
//////////////////////////////////////////////////////////////////////
//
// Print a polygon
//
HRESULT PrintModelPolyVertices(MODEL_POLY *mp, POLY_RGB *mrgb, UINT *piVertex, bool bReverseWinding, bool bFlipNormal)
{
    ValidatePoly(mp);
    if (!bReverseWinding)
    {
        PrintModelVertex(piVertex, &mp->v2->x, &mp->v2->nx, &(mrgb->rgb[2]), (mp->Tpage == -1) ? NULL : &mp->tu2, bFlipNormal);
        PrintModelVertex(piVertex, &mp->v1->x, &mp->v1->nx, &(mrgb->rgb[1]), (mp->Tpage == -1) ? NULL : &mp->tu1, bFlipNormal);
        PrintModelVertex(piVertex, &mp->v0->x, &mp->v0->nx, &(mrgb->rgb[0]), (mp->Tpage == -1) ? NULL : &mp->tu0, bFlipNormal);
        if (mp->Type & POLY_QUAD)
            PrintModelVertex(piVertex, &mp->v3->x, &mp->v3->nx, &(mrgb->rgb[3]), (mp->Tpage == -1) ? NULL : &mp->tu3, bFlipNormal);
    }
    else
    {
        PrintModelVertex(piVertex, &mp->v0->x, &mp->v0->nx, &(mrgb->rgb[0]), (mp->Tpage == -1) ? NULL : &mp->tu0, bFlipNormal);
        PrintModelVertex(piVertex, &mp->v1->x, &mp->v1->nx, &(mrgb->rgb[1]), (mp->Tpage == -1) ? NULL : &mp->tu1, bFlipNormal);
        PrintModelVertex(piVertex, &mp->v2->x, &mp->v2->nx, &(mrgb->rgb[2]), (mp->Tpage == -1) ? NULL : &mp->tu2, bFlipNormal);
        if (mp->Type & POLY_QUAD)
            PrintModelVertex(piVertex, &mp->v3->x, &mp->v3->nx, &(mrgb->rgb[3]), (mp->Tpage == -1) ? NULL : &mp->tu3, bFlipNormal);
    }
    return S_OK;
}

/*
//////////////////////////////////////////////////////////////////////
//
// Print a polygon
//
HRESULT PrintPolyVertices(MODEL_POLY *mp, POLY_RGB *mrgb, int iVertex)
{
    ValidatePoly(mp);
    
    XDXPrintf("<v index=\"%d\">", iVertex++);
    XDXPrintf("%g %g %g", mp->v0->x, mp->v0->y, mp->v0->z);
    XDXPrintf("\t%g %g %g", mp->v0->nx, mp->v0->ny, mp->v0->nz);
    XDXPrintf("\t0x%.08x", D3DCOLOR_RGBA(mrgb->rgb[0].r, mrgb->rgb[0].g, mrgb->rgb[0].b, mrgb->rgb[0].a));
    if (mp->Tpage != -1)
        XDXPrintf("\t%g %g", mp->tu0, mp->tv0);
    XDXPrintf("</v>\n");

    XDXPrintf("<v index=\"%d\">", iVertex++);
    XDXPrintf("%g %g %g", mp->v1->x, mp->v1->y, mp->v1->z);
    XDXPrintf("\t%g %g %g", mp->v1->nx, mp->v1->ny, mp->v1->nz);
    XDXPrintf("\t0x%.08x", D3DCOLOR_RGBA(mrgb->rgb[1].r, mrgb->rgb[1].g, mrgb->rgb[1].b, mrgb->rgb[1].a));
    if (mp->Tpage != -1)
        XDXPrintf("\t%g %g", mp->tu1, mp->tv1);
    XDXPrintf("</v>\n");
    
    XDXPrintf("<v index=\"%d\">", iVertex++);
    XDXPrintf("%g %g %g", mp->v2->x, mp->v2->y, mp->v2->z);
    XDXPrintf("\t%g %g %g", mp->v2->nx, mp->v2->ny, mp->v2->nz);
    XDXPrintf("\t0x%.08x", D3DCOLOR_RGBA(mrgb->rgb[2].r, mrgb->rgb[2].g, mrgb->rgb[2].b, mrgb->rgb[2].a));
    if (mp->Tpage != -1)
        XDXPrintf("\t%g %g", mp->tu2, mp->tv2);
    XDXPrintf("</v>\n");

    if (mp->Type & POLY_QUAD)
    {
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
*/

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
//  if (!_strnicmp(drive, "D:", 2))
//      strcpy(drive, "U:");    // replace D: with U:
    _makepath( path, drive, dir, name, ".xdx");

    if (XDXBegin( path ) == S_OK)
    {
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
				int Increment;
				if (mp->Type & POLY_DOUBLE)
					Increment = 2;
				else
					Increment = 1;
                if (mp->Type & POLY_SEMITRANS)
                    rQuadCount[TransparentTextured] += Increment;
                else
                    rQuadCount[OpaqueTextured] += Increment;
            }
            for (i = m->TriNumTex ; i ; i--, mrgb++, mp++)
            {
                Assert(mp->Tpage != -1);
                TextureSet.insert(mp->Tpage);
				int Increment;
				if (mp->Type & POLY_DOUBLE)
					Increment = 2;
				else
					Increment = 1;
                if (mp->Type & POLY_SEMITRANS)
                    rTriCount[TransparentTextured] += Increment;
                else
                    rTriCount[OpaqueTextured] += Increment;
            }
            for (i = m->QuadNumRGB ; i ; i--, mrgb++, mp++)
            {
                Assert(mp->Tpage == -1);    // RGB quads should not have tpages
				int Increment;
				if (mp->Type & POLY_DOUBLE)
					Increment = 2;
				else
					Increment = 1;
                if (mp->Type & POLY_SEMITRANS)
                    rQuadCount[TransparentRGB] += Increment;
                else
                    rQuadCount[OpaqueRGB] += Increment;
            }
            for (i = m->TriNumRGB ; i ; i--, mrgb++, mp++)
            {
                Assert(mp->Tpage == -1);    // RGB tris should not have tpages
				int Increment;
				if (mp->Type & POLY_DOUBLE)
					Increment = 2;
				else
					Increment = 1;
                if (mp->Type & POLY_SEMITRANS)
                    rTriCount[TransparentRGB] += Increment;
                else
                    rTriCount[OpaqueRGB] += Increment;
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
                    XDXPrintf("<VertexBuffer id=\"%s_VertexBuffer_%s\"\n", strModelName, PassName[iPass]);
                    
                    // NOTE: Re-volt seems to be using specular only to hold
                    // the current fog value.  So, do not output specular to
                    // the vertex buffer.
                    XDXPrintf(" names=\"POSITION NORMAL DIFFUSE TEXCOORD0\"\n");
                    XDXPrintf(" format=\"FLOAT3 FLOAT3 D3DCOLOR FLOAT2\"");
                    XDXPrintf(">\n");
					XDXIndentBy(1);
                    
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
                            PrintModelPolyVertices(mp, mrgb, &iVertex, false, false);
							if (mp->Type & POLY_DOUBLE)
                            {
								rQuadCountPerTexture[(int)bTransparent][iTexture]++;
                                XDXPrintf("<!-- double-sided begin -->\n");
                                PrintModelPolyVertices(mp, mrgb, &iVertex, true, true);
                                XDXPrintf("<!-- double-sided end -->\n");
							}
                        }
                        for (i = TriNum ; i ; i--, mrgb++, mp++)
                        {
                            if (mp->Tpage != Tpage) continue;
                            if (!!(mp->Type & POLY_SEMITRANS) != bTransparent) continue;
                            rTriCountPerTexture[(int)bTransparent][iTexture]++;
                            PrintModelPolyVertices(mp, mrgb, &iVertex, false, false);
							if (mp->Type & POLY_DOUBLE)
                            {
								rTriCountPerTexture[(int)bTransparent][iTexture]++;
                                XDXPrintf("<!-- double-sided begin -->\n");
                                PrintModelPolyVertices(mp, mrgb, &iVertex, true, true);
                                XDXPrintf("<!-- double-sided end -->\n");
							}
                        }
                    }
                    XDXIndentBy(-1);
                    XDXPrintf("</VertexBuffer>\n");
                }
                else    // non-textured RGB-per-vertex quads and tris
                {
                    XDXPrintf("<VertexBuffer id=\"%s_VertexBuffer_%s\"\n", strModelName, PassName[iPass]);
                    
                    // NOTE: Re-volt seems to be using specular only to hold
                    // the current fog value.  So, do not output specular to
                    // the vertex buffer.
                    XDXPrintf(" names=\"POSITION NORMAL DIFFUSE\"\n");
                    XDXPrintf(" format=\"FLOAT3 FLOAT3 D3DCOLOR\"");
                    XDXPrintf(">\n");
					XDXIndentBy(1);
    
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
                        PrintModelPolyVertices(mp, mrgb, &iVertex, false, false);
						if (mp->Type & POLY_DOUBLE)
						{
							XDXPrintf("<!-- double-sided begin -->\n");
							PrintModelPolyVertices(mp, mrgb, &iVertex, true, true);
							XDXPrintf("<!-- double-sided end -->\n");
						}
                    }
                    for (i = TriNum ; i ; i--, mrgb++, mp++)
                    {
                        if (!!(mp->Type & POLY_SEMITRANS) != bTransparent) continue;
                        PrintModelPolyVertices(mp, mrgb, &iVertex, false, false);
						if (mp->Type & POLY_DOUBLE)
						{
							XDXPrintf("<!-- double-sided begin -->\n");
							PrintModelPolyVertices(mp, mrgb, &iVertex, true, true);
							XDXPrintf("<!-- double-sided end -->\n");
						}
                    }
                    XDXIndentBy(-1);
                    XDXPrintf("</VertexBuffer>\n");
                }
            }

            // XDXPrintf("<!-- TODO: Index Buffer goes here -->\n");
            
            XDXPrintf("<Model id=\"%s\">\n", strModelName);
            XDXIndentBy(1);

            // Custom model attributes
            XDXPrintf("<!--\n");    // TODO: remove these comments when custom extensions to the compiler are in place
            XDXPrintf("<Revolt");
            XDXPrintf(" Radius=\"%f\"", m->Radius);
            XDXPrintf(" Xmin=\"%f\"", m->Xmin);
            XDXPrintf(" Xmax=\"%f\"", m->Xmax);
            XDXPrintf(" Ymin=\"%f\"", m->Ymin);
            XDXPrintf(" Ymax=\"%f\"", m->Ymax);
            XDXPrintf(" Zmin=\"%f\"", m->Zmin);
            XDXPrintf(" Zmax=\"%f\"\n", m->Zmax);
            XDXPrintf(" PolyNum=\"%d\"", m->PolyNum);
            XDXPrintf(" VertNum=\"%d\"", m->VertNum);
            XDXPrintf(" QuadNumTex=\"%d\"", m->QuadNumTex);
            XDXPrintf(" TriNumTex=\"%d\"", m->TriNumTex);
            XDXPrintf(" QuadNumRGB=\"%d\"", m->QuadNumRGB);
            XDXPrintf(" TriNumRGB=\"%d\"", m->TriNumRGB);
            XDXPrintf(" />\n");
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
                        XDXPrintf("<Pass>\n");
                        XDXIndentBy(1);

                        // NOTE: The current XDX format doesn't easily
                        // let you do things like switch a single
                        // texture or a single vertex shader constant.
                        // We have to output all of this redundant
                        // goo.  Perhaps XDX should be more free-form
                        // for this kind of stuff, more like a
                        // pushbuffer.
                
                        // Vertex Shader
                        XDXPrintf("<VertexShader idref=\"%s\">\n", PassVertexShader[iPass]);
                        XDXIndentBy(1);
                        XDXPrintf("<decl>\n");
                        XDXPrintf("<stream index=\"0\" idref=\"%s_VertexBuffer_%s\" />\n", strModelName, PassName[iPass]);
                        XDXPrintf("</decl>\n");
                        XDXPrintf("<!-- TODO: track down the proper source of the model's material color -->\n");
                        XDXSetVertexShaderConstant(0, &diffuse, 1, "Diffuse", NULL);
                        XDXSetVertexShaderConstant(1, &lightdir, 1, "LightDirection", NULL);
                        XDXSetVertexShaderConstant(4, &matCamera, 4, "MWORLDVIEWPROJECTION", strCameraName);
                        XDXSetVertexShaderConstant(8, &matIdentity, 3, "MWORLDINVERSE", NULL);
                        XDXIndentBy(-1);
                        XDXPrintf("</VertexShader>\n");

                        // Render State
                        XDXPrintf("<RenderState>\n");
                        XDXPrintf("<TextureState Stage=\"0\" idref=\"%s\"\n", TextureIdentifier(Tpage));
                        XDXIndentBy(1);
                        XDXPrintf("ColorOp=\"Modulate\"\n");
                        XDXPrintf("ColorArg1=\"Texture\"\n");
                        XDXPrintf("ColorArg2=\"Diffuse\"\n");
                        XDXPrintf("AlphaOp=\"Modulate\"\n");
                        XDXPrintf("AlphaArg1=\"Texture\"\n");
                        XDXPrintf("AlphaArg2=\"Diffuse\"\n");
                        XDXPrintf("/>\n");
                        XDXIndentBy(-1);
                        XDXPrintf("<TextureState Stage=\"1\"");
                        XDXPrintf(" ColorOp=\"Disable\"");
                        XDXPrintf(" AlphaOp=\"Disable\"");
                        XDXPrintf("/>\n");
                        XDXPrintf("</RenderState>\n");

                        // Draw commands.  The start and count attributes are vertex counts, not primitive counts.
                        if (QuadCount > 0)
                        {
                            XDXPrintf("<Draw primitive=\"QUADLIST\" start=\"%d\" count=\"%d\" />\n",
                                      iVertex, QuadCount * 4);
                            iVertex += QuadCount * 4;
                        }
                        if (TriCount > 0)
                        {
                            XDXPrintf("<Draw primitive=\"TRILIST\" start=\"%d\" count=\"%d\" />\n",
                                      iVertex, TriCount * 3);
                            iVertex += TriCount * 3;
                        }
						
                        XDXIndentBy(-1);
                        XDXPrintf("</Pass>\n");
                    }
                }
                else    // non-textured rgb-per-pixel quads and tris
                {
                    XDXPrintf("<Pass>\n");
                    XDXIndentBy(1);
    
                    // Vertex Shader
                    XDXPrintf("<VertexShader idref=\"%s\">\n", PassVertexShader[iPass]);
                    XDXIndentBy(1);
                    XDXPrintf("<decl>\n");
                    XDXPrintf("<stream index=\"0\" idref=\"%s_VertexBuffer_%s\" />\n", strModelName, PassName[iPass]);
                    XDXPrintf("</decl>\n");
                    XDXPrintf("<!-- TODO: track down the proper source of the model's material color -->\n");
                    XDXSetVertexShaderConstant(0, &diffuse, 1, "Diffuse", NULL);
                    XDXSetVertexShaderConstant(1, &lightdir, 1, "LightDirection", NULL);
                    XDXSetVertexShaderConstant(4, &matCamera, 4, "World * View * Projection", strCameraName);
                    XDXSetVertexShaderConstant(8, &matIdentity, 3, "Transpose(Inverse(World))", NULL);
                    XDXIndentBy(-1);
                    XDXPrintf("</VertexShader>\n");
    
                    // Render State
                    XDXPrintf("<RenderState>\n");
                    XDXPrintf("<TextureState Stage=\"0\"");
                    XDXPrintf(" ColorOp=\"SelectArg1\"");
                    XDXPrintf(" ColorArg1=\"Diffuse\"");
                    XDXPrintf(" AlphaOp=\"SelectArg1\"");
                    XDXPrintf(" AlphaArg1=\"Diffuse\"");
                    XDXPrintf("/>\n");
                    XDXPrintf("<TextureState Stage=\"1\"");
                    XDXPrintf(" ColorOp=\"Disable\"");
                    XDXPrintf(" AlphaOp=\"Disable\"");
                    XDXPrintf("/>\n");
                    XDXPrintf("</RenderState>\n");
    
                    // Draw commands
                    if (rQuadCount[iPass] > 0)
                    {
                        XDXPrintf("<Draw primitive=\"QUADLIST\" start=\"0\" count=\"%d\" />\n",
                                  rQuadCount[iPass] * 4);
                    }
                    if (rTriCount[iPass] > 0)
                    {
                        XDXPrintf("<Draw primitive=\"TRILIST\" start=\"%d\" count=\"%d\" />\n",
                                  rQuadCount[iPass] * 4, rTriCount[iPass] * 3);
                    }
                    XDXIndentBy(-1);
                    XDXPrintf("</Pass>\n");
                }
            }
			XDXIndentBy(-1);
            XDXPrintf("</Model>\n");

            /*
            // Set the preview camera to scale the object to full-screen,
            // and to rotate around the Y axis
            XDXPrintf("<Frame id=\"%s\">\n", strCameraName);
			XDXIndentBy(1);
            XDXPrintf("<Matrix>\n");
			XDXIndentBy(1);
            XDXPrintf("<Translate value=\"0 0 0.5\" />\n");
            XDXPrintf("<Scale value=\"1 1 0.5\" />\n");
            XDXPrintf("<!-- Rotate around Y axis for preview -->\n");
            XDXPrintf("<Rotate angle=\"0\" axis=\"0 1 0\">\n");
			XDXIndentBy(1);
            XDXPrintf("<animate attribute=\"angle\" begin=\"0\" dur=\"2\" values=\"0;3.141592654;6.283185307;\" />\n");
			XDXIndentBy(-1);
            XDXPrintf("</Rotate>\n");
            XDXPrintf("<!-- Point Y up -->\n");
            XDXPrintf("<Rotate angle=\"3.141592654\" axis=\"1 0 0\" />\n");
            XDXPrintf("<!-- Scale model to unit sphere 1/radius = 1/%f -->\n", m->Radius);
            XDXPrintf("<Scale value=\"%f %f %f\" />\n", fScale, fScale, fScale);
            XDXPrintf("<!-- Translate model center to origin -->\n");
            XDXPrintf("<Translate value=\"%f %f %f\" />\n", -vCenter.x, -vCenter.y, -vCenter.z);
			XDXIndentBy(-1);
            XDXPrintf("</Matrix>\n");
			XDXIndentBy(-1);
            XDXPrintf("</Frame>\n");
            */
        }
        XDXEnd();
    }
    return S_OK;
}



/*
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
*/

/*
//////////////////////////////////////////////////////////////////////
//
// Print a polygon
//
HRESULT PrintPolyVertices(MODEL_POLY_LOAD *pPoly, int *piVert, MODEL_VERTEX_LOAD *rVert)
{
    ValidatePoly(pPoly, rVert);
    
    XDXPrintf("<v index=\"%d\">", (*piVert)++);
    XDXPrintf("%g %g %g", rVert[pPoly->vi0].x, rVert[pPoly->vi0].y, rVert[pPoly->vi0].z);
    XDXPrintf(" %g %g %g", rVert[pPoly->vi0].nx, rVert[pPoly->vi0].ny, rVert[pPoly->vi0].nz);
    XDXPrintf(" 0x%.08x", ConvertToD3DCOLOR(pPoly->c0));
    if (pPoly->Tpage != -1)
        XDXPrintf(" %g %g", pPoly->u0, pPoly->v0);
    XDXPrintf("</v>\n");

    XDXPrintf("<v index=\"%d\">", (*piVert)++);
    XDXPrintf("%g %g %g", rVert[pPoly->vi1].x, rVert[pPoly->vi1].y, rVert[pPoly->vi1].z);
    XDXPrintf(" %g %g %g", rVert[pPoly->vi1].nx, rVert[pPoly->vi1].ny, rVert[pPoly->vi1].nz);
    XDXPrintf(" 0x%.08x", ConvertToD3DCOLOR(pPoly->c1));
    if (pPoly->Tpage != -1)
        XDXPrintf(" %g %g", pPoly->u1, pPoly->v1);
    XDXPrintf("</v>\n");
    
    XDXPrintf("<v index=\"%d\">", (*piVert)++);
    XDXPrintf("%g %g %g", rVert[pPoly->vi2].x, rVert[pPoly->vi2].y, rVert[pPoly->vi2].z);
    XDXPrintf(" %g %g %g", rVert[pPoly->vi2].nx, rVert[pPoly->vi2].ny, rVert[pPoly->vi2].nz);
    XDXPrintf(" 0x%.08x", ConvertToD3DCOLOR(pPoly->c2));
    if (pPoly->Tpage != -1)
        XDXPrintf(" %g %g", pPoly->u2, pPoly->v2);
    XDXPrintf("</v>\n");

    if (pPoly->Type & POLY_QUAD)
    {
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
        if (prm == 0)
            XDXPrintf("<VertexBuffer id=\"%s\" >\n", name);
        else
            XDXPrintf("<VertexBuffer id=\"%s%d\"\n", name, prm);
        XDXIndentBy(1);
        int iVert = 0;
        for (i = 0 ; i < mh.PolyNum ; i++)
        {
            if (rPoly[i].Tpage != -1)
            {
                if (loadflag & LOADMODEL_FORCE_TPAGE) rPoly[i].Tpage = tpage;
                if (loadflag & LOADMODEL_OFFSET_TPAGE) rPoly[i].Tpage += tpage;
            }
            PrintPolyVertices(&rPoly[i], &iVert, rVert);
        }
        XDXIndentBy(-1);
        XDXPrintf("</VertexBuffer>");

        // dump the mesh faces, with material attributes
        if (prm == 0)
            XDXPrintf("<Mesh id=\"%s\" >\n", name);
        else
            XDXPrintf("<Mesh id=\"%s%d\"\n", name, prm);
        XDXIndentBy(1);
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
        XDXIndentBy(-1);
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

*/

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
// Print a vertex
//
HRESULT PrintWorldVertex(int *piVertex, float *pPosition, float *pNormal, long rgb, float *pUV, bool bFlipNormal)
{
    XDXPrintf("<v index=\"%d\">", (*piVertex)++);
    XDXPrintf("%g %g %g", pPosition[0], pPosition[1], pPosition[2]);
    if (bFlipNormal)
        XDXPrintf(" %g %g %g", -pNormal[0], -pNormal[1], -pNormal[2]);
    else
        XDXPrintf(" %g %g %g", pNormal[0], pNormal[1], pNormal[2]);
    XDXPrintf(" 0x%.08x", ConvertToD3DCOLOR(rgb));
    if (pUV != NULL)
        XDXPrintf(" %g %g", pUV[0], pUV[1]);
    else
        XDXPrintf(" 0 0");
    XDXPrintf("</v>\n");
    return S_OK;
}
    
//////////////////////////////////////////////////////////////////////
//
// Print a polygon
//
HRESULT PrintWorldPolyVertices(WORLD_POLY *mp, int *piVertex, bool bReverseWinding, bool bFlipNormal)
{
    ValidatePoly(mp);
    if (!bReverseWinding)
    {
        PrintWorldVertex(piVertex, &mp->v2->x, &mp->v2->nx, mp->rgb2, (mp->Tpage == -1) ? NULL : &mp->tu2, bFlipNormal);
        PrintWorldVertex(piVertex, &mp->v1->x, &mp->v1->nx, mp->rgb1, (mp->Tpage == -1) ? NULL : &mp->tu1, bFlipNormal);
        PrintWorldVertex(piVertex, &mp->v0->x, &mp->v0->nx, mp->rgb0, (mp->Tpage == -1) ? NULL : &mp->tu0, bFlipNormal);
        if (mp->Type & POLY_QUAD)
            PrintWorldVertex(piVertex, &mp->v3->x, &mp->v3->nx, mp->rgb3, (mp->Tpage == -1) ? NULL : &mp->tu3, bFlipNormal);
    }
    else
    {
        PrintWorldVertex(piVertex, &mp->v0->x, &mp->v0->nx, mp->rgb0, (mp->Tpage == -1) ? NULL : &mp->tu0, bFlipNormal);
        PrintWorldVertex(piVertex, &mp->v1->x, &mp->v1->nx, mp->rgb1, (mp->Tpage == -1) ? NULL : &mp->tu1, bFlipNormal);
        PrintWorldVertex(piVertex, &mp->v2->x, &mp->v2->nx, mp->rgb2, (mp->Tpage == -1) ? NULL : &mp->tu2, bFlipNormal);
        if (mp->Type & POLY_QUAD)
            PrintWorldVertex(piVertex, &mp->v3->x, &mp->v3->nx, mp->rgb3, (mp->Tpage == -1) ? NULL : &mp->tu3, bFlipNormal);
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

    WorldExporter(CHAR *name);
    HRESULT CountVerticesAndFacesPerTexture();
    HRESULT ExportTextureReference(CHAR *strIdentifier, CHAR *strPath, INT levels);
    HRESULT ExportTextureReferences();
    HRESULT ExportModelReference(CHAR *strPath);
    HRESULT ExportModelReferences();
    HRESULT ExportVertices();
    HRESULT ExportDefaultWorldCubeMaterial();
    HRESULT ExportWorldBigCubes();
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
//  if (!_strnicmp(drive, "D:", 2))
//      strcpy(drive, "U:");    // replace D: with U:
    _makepath( m_path, drive, dir, name, ".xdx");
    
    strncpy(m_strName, name, MaxNameLength);
    m_strName[MaxNameLength - 1] = 0;
    strncpy(m_strCameraName, name, MaxNameLength);
    strncat(m_strCameraName, "_Camera", MaxNameLength);
    m_strCameraName[MaxNameLength - 1] = 0;
    m_TextureTracker.clear();
    m_TotalCount.clear();
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
HRESULT ExportTextureReference(CHAR *strBasePath, CHAR *strIdentifier, CHAR *strPath, INT levels)
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
    return ::ExportTextureReference(m_path, strIdentifier, strPath, levels);
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
        sprintf(strIdentifier, "%s%c", CurrentLevelInfo.szDir, ii + 'a');
        sprintf(buf, "D:\\levels\\%s\\%s%c.bmp", CurrentLevelInfo.szDir, CurrentLevelInfo.szDir, ii + 'a');
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
//  if (!_strnicmp(drive, "D:", 2))
//      strcpy(drive, "U:");    // replace D: with U:
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
//  if (!_strnicmp(drive, "D:", 2))
//      strcpy(drive, "U:");    // replace D: with U:
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
/*
    // static models
    extern CHAR *LevelModelList[];
    CHAR buf[_MAX_PATH];
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

*/
    // level models
	int i;
    extern CHAR *LevelModelList[];
    XDXPrintf("<!-- level models -->\n");
    for (i = 0 ; i < MAX_LEVEL_MODELS ; i++)
    {
        if (LevelModel[i].ID != -1)
        {
			CHAR buf[_MAX_PATH];
			sprintf(buf, "%s.xdx", LevelModelList[LevelModel[i].ID]);
			ExportModelReference(buf);
		}
	}

    // instance models
    XDXPrintf("<!-- InstanceModelNum=\"%d\" -->\n", InstanceModelNum);
    for (i = 0 ; i < InstanceModelNum ; i++)
        ExportModelReference(g_InstanceModelPaths[i]);
    return S_OK;
}



            
//////////////////////////////////////////////////////////////////////
// 
// Count the quads and tris per texture within this world big cube.
//
HRESULT CountCubeTextures(BIG_CUBE_HEADER *pBigCube, TextureTracker *pCubeTextureTracker, TextureCounts *pCubeTextureCounts)
{
    pCubeTextureCounts->clear();
    pCubeTextureTracker->clear();
    for (int iCube = 0; iCube < pBigCube->CubeNum; iCube++)
    {
        WORLD_MODEL *m = &pBigCube->Cubes[iCube]->Model;
        WORLD_POLY *mp = m->PolyPtr;
        for (int iPoly = 0; iPoly < m->PolyNum; iPoly++, mp++)
        {
            TextureCounts *pCurrentTextureCounts = &(*pCubeTextureTracker)[ mp->Tpage ];
            int Increment;
            if (mp->Type & POLY_DOUBLE)
                Increment = 2;
            else
                Increment = 1;
            if (mp->Type & POLY_QUAD)
            {
                if (mp->Type & POLY_SEMITRANS)
                {
                    pCubeTextureCounts->TransparentQuad += Increment;
                    pCurrentTextureCounts->TransparentQuad += Increment;
                }
                else
                {
                    pCubeTextureCounts->OpaqueQuad += Increment;
                    pCurrentTextureCounts->OpaqueQuad += Increment;
                }
            }
            else
            {
                if (mp->Type & POLY_SEMITRANS)
                {
                    pCubeTextureCounts->TransparentTri += Increment;
                    pCurrentTextureCounts->TransparentTri += Increment;
                }
                else
                {
                    pCubeTextureCounts->OpaqueTri += Increment;
                    pCurrentTextureCounts->OpaqueTri += Increment;
                }
            }
        }
    }
    return S_OK;
}
            
//////////////////////////////////////////////////////////////////////
// 
// Export world model vertices to XDX
//
HRESULT WorldExporter::ExportVertices()
{
    XDXPrintf("<!-- vertices fit in one big vertex buffer -->\n");
    XDXPrintf("<VertexBuffer id=\"%s_VertexBuffer\"\n", m_strName);
	XDXIndentBy(1);
    XDXPrintf(" names=\"POSITION NORMAL DIFFUSE TEXCOORD0\"\n");
    XDXPrintf(" format=\"FLOAT3 FLOAT3 D3DCOLOR FLOAT2\"");
    XDXPrintf(">\n");
    int iVertex = 0;
    for (int IsTransparent = 0; IsTransparent < 2; IsTransparent++)
    {
        // Dump opaque vertices first, then transparent ones,
        // to match rendering order.
        if (IsTransparent)
            XDXPrintf("<!-- Transparent -->\n");
        else
            XDXPrintf("<!-- Opaque -->\n");
        for (int iBigCube = 0 ; iBigCube < World.BigCubeNum ; iBigCube++)
        {
            BIG_CUBE_HEADER *pBigCube = &World.BigCube[iBigCube];
            TextureTracker CubeTextureTracker;
            TextureCounts CubeTextureCounts;
            CountCubeTextures(pBigCube, &CubeTextureTracker, &CubeTextureCounts);
            
            // Dump vertices sorted by opaque/transparent and by texture and by quad/tri
            XDXPrintf("<!-- WorldCube%d -->\n", iBigCube);
            if (IsTransparent)
            {
                if (CubeTextureCounts.TransparentQuad == 0
                    && CubeTextureCounts.TransparentTri == 0)
                    continue;
            }
            else
            {
                if (CubeTextureCounts.OpaqueQuad == 0
                    && CubeTextureCounts.OpaqueTri == 0)
                    continue;
            }
            TextureTracker::iterator iTextureTracker;
            for (iTextureTracker = CubeTextureTracker.begin();
                 iTextureTracker != CubeTextureTracker.end();
                 iTextureTracker++)
            {
                short Tpage = (*iTextureTracker).first; // current texture page
                TextureCounts *pCurrentTextureCounts = &(*iTextureTracker).second;  // count of quads and tris
                for (int IsTriangle = 0; IsTriangle < 2; IsTriangle++)
                {
                    // Print comment
                    if (IsTransparent)
                    {
                        if (!IsTriangle)
                        {
                            if (pCurrentTextureCounts->TransparentQuad == 0)
                                continue;
                            XDXPrintf("<!-- Tpage=\"%d\" TransparentQuad=\"%d\" -->\n", Tpage, pCurrentTextureCounts->TransparentQuad);
                        }
                        else
                        {
                            if (pCurrentTextureCounts->TransparentTri == 0)
                                continue;
                            XDXPrintf("<!-- Tpage=\"%d\" TransparentTri=\"%d\" -->\n", Tpage, pCurrentTextureCounts->TransparentTri);
                        }
                    }
                    else
                    {
                        if (!IsTriangle)
                        {
                            if (pCurrentTextureCounts->OpaqueQuad == 0)
                                continue;
                            XDXPrintf("<!-- Tpage=\"%d\" OpaqueQuad=\"%d\" -->\n", Tpage, pCurrentTextureCounts->OpaqueQuad);
                        }
                        else 
                        {
                            if (pCurrentTextureCounts->OpaqueTri == 0)
                                continue;
                            XDXPrintf("<!-- Tpage=\"%d\" OpaqueTri=\"%d\" -->\n", Tpage, pCurrentTextureCounts->OpaqueTri);
                        }
                    }

                    // Dump vertices that match texture and draw mode
                    for (int iCube = 0; iCube < pBigCube->CubeNum; iCube++)
                    {
                        WORLD_MODEL *m = &pBigCube->Cubes[iCube]->Model;
                        WORLD_POLY *mp = m->PolyPtr;
                        for (int iPoly = 0; iPoly < m->PolyNum; iPoly++, mp++)
                        {
                            if (mp->Tpage != Tpage) continue;   // skip if doesn't match current texture
                            if (!IsTransparent && (mp->Type & POLY_SEMITRANS)) continue;
                            if (IsTransparent && !(mp->Type & POLY_SEMITRANS)) continue;
                            if (!IsTriangle && !(mp->Type & POLY_QUAD)) continue;
                            if (IsTriangle && (mp->Type & POLY_QUAD)) continue;
                            PrintWorldPolyVertices(mp, &iVertex, false, false);
                            if (mp->Type & POLY_DOUBLE)
                            {
                                XDXPrintf("<!-- double-sided begin -->\n");
                                PrintWorldPolyVertices(mp, &iVertex, true, true);
                                XDXPrintf("<!-- double-sided end -->\n");
                            }
                        }
                    }
                }
            }
        }
    }
	XDXIndentBy(-1);
    XDXPrintf("</VertexBuffer>\n");
    return S_OK;
}

//////////////////////////////////////////////////////////////////////
// 
// Export default cube material
//
HRESULT WorldExporter::ExportDefaultWorldCubeMaterial()
{
    extern WORLD World;
    D3DXMATRIX matIdentity;
    D3DXMatrixIdentity(&matIdentity);
    D3DXMATRIX matCamera = matIdentity;
    D3DXVECTOR4 diffuse(1.f, 1.f, 1.f, 1.f);
    D3DXVECTOR4 lightdir(0.f, 0.f, 1.f, 1.f);

    XDXPrintf("<Material id=\"DefaultWorldCubeMaterial\">\n");
    XDXIndentBy(1);
    XDXPrintf("<Pass>\n");
    XDXIndentBy(1);
    XDXPrintf("<VertexShader idref=\"VSLightDiffuseTexture\">\n");
    XDXIndentBy(1);
    XDXPrintf("<decl>\n");
    XDXPrintf("<stream index=\"0\" idref=\"%s_VertexBuffer\" />\n", m_strName);
    XDXPrintf("</decl>\n");
    XDXSetVertexShaderConstant(0, &diffuse, 1, "Diffuse", NULL);
    XDXSetVertexShaderConstant(1, &lightdir, 1, "LightDirection", NULL);
    XDXSetVertexShaderConstant(4, &matCamera, 4, "MWorldViewProjection", m_strCameraName);
    XDXSetVertexShaderConstant(8, &matIdentity, 3, "MWorldInverseTranspose", NULL);
    XDXIndentBy(-1);
    XDXPrintf("</VertexShader>\n");
    /*
    XDXPrintf("<RenderState>\n");
    XDXPrintf("<TextureState Stage=\"0\" idref=\"NULL\"\n");
    XDXIndentBy(1);
    XDXPrintf("MinFilter=\"Anisotropic\"\n"
    		  "MagFilter=\"Anisotropic\"\n"
    		  "MaxAnisotropy=\"4\"\n"
    		  "MipFilter=\"Linear\"\n"
    		  "AddressU=\"Wrap\"\n"
    		  "AddressV=\"Wrap\"\n"
    		  "/>\n");
    XDXIndentBy(-1);
    XDXPrintf("<TextureState Stage=\"1\"");
    XDXPrintf(" ColorOp=\"Disable\"");
    XDXPrintf(" AlphaOp=\"Disable\"");
    XDXPrintf("/>\n");
    XDXPrintf("</RenderState>\n");
    */
    XDXIndentBy(-1);
    XDXPrintf("</Pass>\n");
    XDXIndentBy(-1);
    XDXPrintf("</Material>\n");

    return S_OK;
 }


//////////////////////////////////////////////////////////////////////
// 
// Export each world cube as a separate model
//
HRESULT WorldExporter::ExportWorldBigCubes()
{
    {
        XDXPrintf("<!--\n");

        // Dump out revolt cube data
        XDXPrintf("<revolt:bigcubes>\n");
		XDXIndentBy(1);
        for (int iBigCube = 0 ; iBigCube < World.BigCubeNum ; iBigCube++)
        {
            BIG_CUBE_HEADER *pBigCube;
            pBigCube = &World.BigCube[iBigCube];
            XDXPrintf("<revolt:bigcube count=\"%d\" center=\"%g %g %g\" radius=\"%g\">\n",
                      pBigCube->CubeNum, pBigCube->x, pBigCube->y, pBigCube->z, pBigCube->Radius);
            XDXIndentBy(1);
            for (int iCube = 0 ; iCube < pBigCube->CubeNum ; iCube++)
            {
                CUBE_HEADER *pCube = pBigCube->Cubes[iCube];
                // Find cube pointer in main world list
                for (int jCube = 0; jCube < World.CubeNum; jCube++)
                {
                    if (pCube == &World.Cube[jCube])
                        break;
                }
                Assert(jCube != World.CubeNum);
                XDXPrintf("<revolt:cube index=\"%d\" center=\"%g %g %g\" radius=\"%g\" min=\"%g %g %g\" max=\"%g %g %g\" />\n",
                          jCube, pCube->CentreX, pCube->CentreY, pCube->CentreZ, pCube->Radius,
                          pCube->Xmin, pCube->Ymin, pCube->Zmin,
                          pCube->Xmax, pCube->Ymax, pCube->Zmax );
            }
            XDXIndentBy(-1);
            XDXPrintf("</revolt:bigcube>\n");
        }
        XDXIndentBy(-1);
        XDXPrintf("</revolt:cubes>\n");

        // Dump out revolt visibox data
        XDXPrintf("<revolt:visiboxes>\n");
        XDXIndentBy(1);
        XDXPrintf("<revolt:camera_visiboxes>\n");
        extern long CamVisiBoxCount;
        extern PERM_VISIBOX CamVisiBox[];
        XDXIndentBy(1);
        for (int iCamVisiBox = 0 ; iCamVisiBox < CamVisiBoxCount ; iCamVisiBox++)
        {
            PERM_VISIBOX *pCamVisiBox = &CamVisiBox[iCamVisiBox];
            XDXPrintf("<revolt:camera_visibox Mask=\"0x%I64x\" ID=\"%d\" min=\"%g %g %g\" max=\"%g %g %g\" />\n",
                      pCamVisiBox->Mask,
                      pCamVisiBox->ID,
                      pCamVisiBox->xmin, pCamVisiBox->ymin, pCamVisiBox->zmin,
                      pCamVisiBox->xmax, pCamVisiBox->ymax, pCamVisiBox->zmax);
        }
        XDXIndentBy(-1);
        XDXPrintf("</revolt:camera_visiboxes>\n");
        XDXPrintf("<revolt:cube_visiboxes>\n");
        extern long CubeVisiBoxCount;
        extern PERM_VISIBOX CubeVisiBox[];
        XDXIndentBy(1);
        for (int iCubeVisiBox = 0 ; iCubeVisiBox < CubeVisiBoxCount ; iCubeVisiBox++)
        {
            PERM_VISIBOX *pCubeVisiBox = &CubeVisiBox[iCubeVisiBox];
            XDXPrintf("<revolt:cube_visibox Mask=\"0x%I64x\" ID=\"%d\" min=\"%g %g %g\" max=\"%g %g %g\" />\n",
                      pCubeVisiBox->Mask,
                      pCubeVisiBox->ID,
                      pCubeVisiBox->xmin, pCubeVisiBox->ymin, pCubeVisiBox->zmin,
                      pCubeVisiBox->xmax, pCubeVisiBox->ymax, pCubeVisiBox->zmax);
        }
        XDXIndentBy(-1);
        XDXPrintf("</revolt:cube_visiboxes>\n");
        XDXIndentBy(-1);
        XDXPrintf("</revolt:visiboxes>\n");
		
        XDXPrintf("-->\n");
    }

	XDXPrintf("<Model id=\"%s\" >\n", m_strName);
	XDXIndentBy(1);
	XDXPrintf("<Material idref=\"DefaultWorldCubeMaterial\" />\n" );

    // Dump opaque primitives first, then transparent ones.
    int iVertex = 0;
    for (int IsTransparent = 0; IsTransparent < 2; IsTransparent++)
    {
        for (int iBigCube = 0 ; iBigCube < World.BigCubeNum ; iBigCube++)
        {
            BIG_CUBE_HEADER *pBigCube = &World.BigCube[iBigCube];
            TextureTracker CubeTextureTracker;
            TextureCounts CubeTextureCounts;
            CountCubeTextures(pBigCube, &CubeTextureTracker, &CubeTextureCounts);
            if (IsTransparent)
            {
                if (CubeTextureCounts.TransparentQuad == 0
                    && CubeTextureCounts.TransparentTri == 0)
                    continue;
            }
            else
            {
                if (CubeTextureCounts.OpaqueQuad == 0
                    && CubeTextureCounts.OpaqueTri == 0)
                    continue;
            }
            
            XDXPrintf("<Model id=\"WorldCube%d_%s\">\n", iBigCube, IsTransparent ? "alpha" : "opaque");
            XDXIndentBy(1);
			
			// $BEGIN Deep Exploration Beta1 compatibility
			XDXPrintf("<!-- Material idref=\"DefaultWorldCubeMaterial\" / -->\n" );
			//bool bMaterialDumped = false;
			// $END Deep Exploration Beta1 compatibility

            // One pass for each texture
            short TpagePrevious = -1;
            TextureTracker::iterator iTextureTracker;
            for (iTextureTracker = CubeTextureTracker.begin();
                 iTextureTracker != CubeTextureTracker.end();
                 iTextureTracker++)
            {
                short Tpage = (*iTextureTracker).first; // current texture page to output
                TextureCounts *pCurrentTextureCounts = &(*iTextureTracker).second;  // count of quads and tris
                UINT QuadCount, TriCount;
                if (IsTransparent)
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
                XDXPrintf("<Pass>\n");
                XDXIndentBy(1);

				// $BEGIN Deep Exploration Beta1 compatibility
				/*
				if (!bMaterialDumped)
				{
					bMaterialDumped = true;
					{
						D3DXMATRIX matIdentity;
						D3DXMatrixIdentity(&matIdentity);
						D3DXMATRIX matCamera = matIdentity;
						D3DXVECTOR4 diffuse(1.f, 1.f, 1.f, 1.f);
						D3DXVECTOR4 lightdir(0.f, 0.f, 1.f, 1.f);
	
						XDXPrintf("<!-- Remove this when material inheritance works. -->\n");
						XDXPrintf("<VertexShader idref=\"VSLightDiffuseTexture\">\n");
						XDXIndentBy(1);
						XDXPrintf("<decl>\n");
						XDXPrintf("<stream index=\"0\" idref=\"%s_VertexBuffer\" />\n", m_strName);
						XDXPrintf("</decl>\n");
						XDXSetVertexShaderConstant(0, &diffuse, 1, "Diffuse", NULL);
						XDXSetVertexShaderConstant(1, &lightdir, 1, "LightDirection", NULL);
						XDXSetVertexShaderConstant(4, &matCamera, 4, "MWorldViewProjection", m_strCameraName);
						XDXSetVertexShaderConstant(8, &matIdentity, 3, "MWorldInverseTranspose", NULL);
						XDXIndentBy(-1);
						XDXPrintf("</VertexShader>\n");
					}
				}
				*/
				// $END Deep Exploration Beta1 compatibility
	
                XDXPrintf("<RenderState>\n");
                XDXIndentBy(1);
                if (Tpage == -1)
                {
                    XDXPrintf("<TextureState Stage=\"0\" idref=\"NULL\"\n");
                    XDXIndentBy(1);
                    XDXPrintf("ColorOp=\"SelectArg1\"\n");
                    XDXPrintf("ColorArg1=\"Diffuse\"\n");
                    XDXPrintf("AlphaOp=\"SelectArg1\"\n");
                    XDXPrintf("AlphaArg1=\"Diffuse\"\n");
                    XDXIndentBy(-1);
                    XDXPrintf("/>\n");
                }
                else
                {
                    XDXPrintf("<TextureState Stage=\"0\" idref=\"%s\"", TextureIdentifier(Tpage));
                    if (TpagePrevious == -1)
                    {
                        XDXPrintf("\n");
                        XDXIndentBy(1);
                        XDXPrintf("ColorOp=\"Modulate\"\n");
                        XDXPrintf("ColorArg1=\"Diffuse\"\n");
                        XDXPrintf("ColorArg2=\"Texture\"\n");
                        XDXPrintf("AlphaOp=\"Modulate\"\n");
                        XDXPrintf("AlphaArg1=\"Diffuse\"\n");
                        XDXPrintf("AlphaArg2=\"Texture\"\n");
                        XDXIndentBy(-1);
                    }
                    XDXPrintf("/>\n");
                }
                TpagePrevious = Tpage;
                XDXIndentBy(-1);
                XDXPrintf("</RenderState>\n");
    
                // Draw commands.  
                if (QuadCount > 0)
                {
                    XDXPrintf("<Draw primitive=\"QUADLIST\" start=\"%d\" count=\"%d\" />\n",
                              iVertex, QuadCount * 4);
                    iVertex += QuadCount * 4;
                }
                if (TriCount > 0)
                {
                    XDXPrintf("<Draw primitive=\"TRILIST\" start=\"%d\" count=\"%d\" />\n",
                              iVertex, TriCount * 3);
                    iVertex += TriCount * 3;
                }
                
                XDXIndentBy(-1);
                XDXPrintf("</Pass>\n");
            }
            XDXIndentBy(-1);
            XDXPrintf("</Model>\n");
        }
    }
	
	XDXIndentBy(-1);
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
    XDXPrintf("<Frame id=\"%s\">\n", m_strCameraName);
    XDXIndentBy(1);
    XDXPrintf("<Matrix>\n");
    XDXIndentBy(1);
    XDXPrintf("<Translate value=\"0 0 0.5\" />\n");
    XDXPrintf("<Scale value=\"1 1 0.5\" />\n");
    XDXPrintf("<!-- Rotate around Y axis for preview -->\n");
    XDXPrintf("<Rotate angle=\"0\" axis=\"0 1 0\">\n");
    XDXIndentBy(1);
    XDXPrintf("<animate attribute=\"angle\" begin=\"0\" dur=\"1\" values=\"0;3.141592654;6.283185307;\" />\n");
    XDXIndentBy(-1);
    XDXPrintf("</Rotate>\n");
    XDXPrintf("<!-- Point Y up -->\n");
    XDXPrintf("<Rotate angle=\"3.141592654\" axis=\"1 0 0\" />\n");
    
    XDXPrintf("<!-- Scale cube by 1/radius = 1/%f -->\n", World.Cube[0].Radius);
    float fScale = 1.f / World.Cube[0].Radius;
    XDXPrintf("<Scale value=\"%f %f %f\">\n", fScale, fScale, fScale);
    XDXIndentBy(1);
    XDXPrintf("<animate begin=\"0\" dur=\"%f\" values=\"\n", 1.f * World.CubeNum);
    XDXIndentBy(1);
    for (iCube = 0 ; iCube < World.CubeNum ; iCube++)
    {
        CUBE_HEADER *pCube = &World.Cube[iCube];
        float fScale = 1.f / pCube->Radius;
        XDXPrintf("%f %f %f;\n", fScale, fScale, fScale);
    }
    XDXPrintf("\"/>\n");
    XDXIndentBy(-1);
    XDXIndentBy(-1);
    XDXPrintf("</Scale>\n");
    
    XDXPrintf("<!-- Translate cube center to origin -->\n");
    XDXPrintf("<Translate value=\"%f %f %f\">\n", -World.Cube[0].CentreX, -World.Cube[0].CentreY, -World.Cube[0].CentreZ);
    XDXIndentBy(1);
    XDXPrintf("<animate begin=\"0\" dur=\"%f\" values=\"\n", 1.f * World.CubeNum);
    XDXIndentBy(1);
    for (iCube = 0 ; iCube < World.CubeNum ; iCube++)
    {
        CUBE_HEADER *pCube = &World.Cube[iCube];
        XDXIndent(m_indent);
        XDXPrintf("%f %f %f;\n", -pCube->CentreX, -pCube->CentreY, -pCube->CentreZ );
    }
    XDXPrintf("\"/>\n");
    XDXIndentBy(-1);
    XDXIndentBy(-1);
    XDXPrintf("</Translate>\n");
    
    XDXIndentBy(-1);
    XDXPrintf("</Matrix>\n");
    XDXIndentBy(-1);
    XDXPrintf("</Frame>\n");

    */

    /*
    // Rotate around entire level
    XDXPrintf("<Frame id=\"%s\">\n", m_strCameraName);
    XDXIndentBy(1);
    XDXPrintf("<Matrix>\n");
    XDXIndentBy(1);
    XDXPrintf("<Translate value=\"0 0 0.5\" />\n");
    XDXPrintf("<Scale value=\"1 1 0.5\" />\n");
    XDXPrintf("<!-- Rotate around Y axis for preview -->\n");
    XDXPrintf("<Rotate angle=\"0\" axis=\"0 1 0\">\n");
	XDXIndentBy(1);
    XDXPrintf("<animate attribute=\"angle\" begin=\"0\" dur=\"1\" values=\"0;3.141592654;6.283185307;\" />\n");
	XDXIndentBy(-1);
    XDXPrintf("</Rotate>\n");
    XDXPrintf("<!-- Point Y up -->\n");
    XDXPrintf("<Rotate angle=\"3.141592654\" axis=\"1 0 0\" />\n");
    
    XDXIndent(m_indent);
    float fRadius = World.BigCube->Radius;
    XDXPrintf("<!-- Scale by 1/radius = 1/%f -->\n", fRadius);
    float fScale = 1.f / fRadius;
    XDXPrintf("<Scale value=\"%f %f %f\" />\n", fScale, fScale, fScale);
    
    XDXPrintf("<!-- Translate center to origin -->\n");
    XDXPrintf("<Translate value=\"%f %f %f\" />\n", -World.BigCube->x, -World.BigCube->y, -World.BigCube->z);
    
    XDXIndentBy(-1);
    XDXPrintf("</Matrix>\n");
    XDXIndentBy(-1);
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
//  if (!_strnicmp(drive, "D:", 2))
//      strcpy(drive, "U:");    // replace D: with U:
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
    WE.ExportDefaultWorldCubeMaterial();
    WE.ExportWorldBigCubes();
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
// Helper class for car exporting
//
struct CarExporter {
    CHAR m_path[_MAX_PATH];
    CHAR m_strName[MaxNameLength];
    TextureTracker m_TextureTracker;
    TextureCounts m_TotalCount;
    CAR *m_pCar;
    CAR_INFO *m_pCarInfo;
    CAR_MODEL *m_pCarModel;
    
    CarExporter(CHAR *file, CAR *pCar, CAR_INFO *pCarInfo, CAR_MODEL *pCarModel);
    
    HRESULT PrintCarVertex(int *piVertex, float *pPosition, float *pNormal, MODEL_RGB *pRGBA, float *pUV, UINT iMatrix, bool bFlipNormal);
    HRESULT PrintCarPolyVertices(MODEL_POLY *pModelPoly, POLY_RGB *pRGB, UINT iMatrix, int *piVertex, bool bReverseWinding, bool bFlipNormal);
    HRESULT PrintCarModelVertices(MODEL *pModel, UINT *piMatrix, short Tpage, BOOL IsTransparent, BOOL IsTriangle, int *piVertex);
    HRESULT PrintCarAerialVertices(AERIAL *aerial, MODEL *secModel, MODEL *topModel,
                                   UINT *piMatrix, short Tpage, BOOL IsTransparent, BOOL IsTriangle, int *piVertex);

    HRESULT CountCarModel(MODEL *pModel);
    HRESULT CountCarAerial(AERIAL *aerial, MODEL *secModel, MODEL *topModel);

    HRESULT CountVerticesAndFacesPerTexture();
    HRESULT ExportCarVertices();
    HRESULT ExportCarModels();
	HRESULT ExportCarSkeleton();
};

CarExporter::CarExporter(CHAR *file, CAR *pCar, CAR_INFO *pCarInfo, CAR_MODEL *pCarModel)
{
    m_pCar = pCar;
    m_pCarInfo = pCarInfo;
    m_pCarModel = pCarModel;
    
    // get the path of the output file
    CHAR drive[_MAX_DRIVE];
    CHAR dir[_MAX_DIR];
    CHAR name[_MAX_PATH];
    CHAR ext[_MAX_EXT];
    _splitpath( file, drive, dir, name, ext );
//  if (!_strnicmp(drive, "D:", 2))
//      strcpy(drive, "U:");    // replace D: with U:
    _makepath( m_path, drive, dir, name, ".xdx");
    strncpy(m_strName, name, MaxNameLength);
    m_strName[MaxNameLength - 1] = 0;
    m_TextureTracker.clear();
    m_TotalCount.clear();
}

/*
UINT ExportCarAerialVertices(AERIAL *aerial,    // aerial data
                             UINT iMatrix)      // current matrix palette index
{
    int iSec;
    VEC thisPos;
    VEC lastPos;
    MAT wMatrix;

    // Calculate the positions of the non-control sections by interpolating from the control sections
    CopyVec(&aerial->Section[0].Pos, &lastPos);

    for (iSec = AERIAL_START; iSec < AERIAL_NTOTSECTIONS; iSec++) {
        
        // calculate the position of the interpolated node
        Interpolate3D(
            &aerial->Section[0].Pos,
            &aerial->Section[AERIAL_SKIP].Pos,
            &aerial->Section[AERIAL_LASTSECTION].Pos,
            iSec * AERIAL_UNITLEN,
            &thisPos);

        // Set the up vector of the node (already scaled to give correct length)
        VecMinusVec(&thisPos, &lastPos, &wMatrix.mv[U]);

        // Build the world Matrix for the section
        BuildMatrixFromUp(&wMatrix);
        // Must normalise look vector when passed up vector was not normalised
        NormalizeVec(&wMatrix.mv[L]);

        // Set the matrix palette entry and increment the count
        Assert(iMatrix < MatrixCount);
        ComputeCarMatrix(&rMatrix[iMatrix++], &wMatrix, &thisPos);

        // Go to the next point along the aerial
        CopyVec(&thisPos, &lastPos);
    }

    return iMatrix;
}
*/

//////////////////////////////////////////////////////////////////////
//
// Print a car vertex
//
HRESULT CarExporter::PrintCarVertex(int *piVertex, float *pPosition, float *pNormal, MODEL_RGB *pRGBA, float *pUV, UINT iMatrix, bool bFlipNormal)
{
    XDXPrintf("<v index=\"%d\">", (*piVertex)++);
    XDXPrintf("%g %g %g", pPosition[0], pPosition[1], pPosition[2]);
    if (bFlipNormal)
        XDXPrintf(" %g %g %g", -pNormal[0], -pNormal[1], -pNormal[2]);
    else
        XDXPrintf(" %g %g %g", pNormal[0], pNormal[1], pNormal[2]);
    XDXPrintf(" 0x%.08x", ConvertToD3DCOLOR(pRGBA));
    if (pUV != NULL)
        XDXPrintf(" %g %g", pUV[0], pUV[1]);
    else
        XDXPrintf(" 0 0");
    XDXPrintf(" %d", iMatrix);  // Matrix palette index.
    XDXPrintf("</v>\n");
    return S_OK;
}
    
//////////////////////////////////////////////////////////////////////
//
// Print a car polygon
//
HRESULT CarExporter::PrintCarPolyVertices(MODEL_POLY *pModelPoly, POLY_RGB *pRGB, UINT iMatrix, 
                                          int *piVertex, bool bReverseWinding, bool bFlipNormal)
{
    ValidatePoly(pModelPoly);
    if (!bReverseWinding)
    {
        PrintCarVertex(piVertex, &pModelPoly->v2->x, &pModelPoly->v2->nx, &(pRGB->rgb[2]), (pModelPoly->Tpage == -1) ? NULL : &pModelPoly->tu2, iMatrix, bFlipNormal);
        PrintCarVertex(piVertex, &pModelPoly->v1->x, &pModelPoly->v1->nx, &(pRGB->rgb[1]), (pModelPoly->Tpage == -1) ? NULL : &pModelPoly->tu1, iMatrix, bFlipNormal);
        PrintCarVertex(piVertex, &pModelPoly->v0->x, &pModelPoly->v0->nx, &(pRGB->rgb[0]), (pModelPoly->Tpage == -1) ? NULL : &pModelPoly->tu0, iMatrix, bFlipNormal);
        if (pModelPoly->Type & POLY_QUAD)
            PrintCarVertex(piVertex, &pModelPoly->v3->x, &pModelPoly->v3->nx, &(pRGB->rgb[3]), (pModelPoly->Tpage == -1) ? NULL : &pModelPoly->tu3, iMatrix, bFlipNormal);
    }
    else
    {
        PrintCarVertex(piVertex, &pModelPoly->v0->x, &pModelPoly->v0->nx, &(pRGB->rgb[0]), (pModelPoly->Tpage == -1) ? NULL : &pModelPoly->tu0, iMatrix, bFlipNormal);
        PrintCarVertex(piVertex, &pModelPoly->v1->x, &pModelPoly->v1->nx, &(pRGB->rgb[1]), (pModelPoly->Tpage == -1) ? NULL : &pModelPoly->tu1, iMatrix, bFlipNormal);
        PrintCarVertex(piVertex, &pModelPoly->v2->x, &pModelPoly->v2->nx, &(pRGB->rgb[2]), (pModelPoly->Tpage == -1) ? NULL : &pModelPoly->tu2, iMatrix, bFlipNormal);
        if (pModelPoly->Type & POLY_QUAD)
            PrintCarVertex(piVertex, &pModelPoly->v3->x, &pModelPoly->v3->nx, &(pRGB->rgb[3]), (pModelPoly->Tpage == -1) ? NULL : &pModelPoly->tu3, iMatrix, bFlipNormal);
    }
    return S_OK;
}


//////////////////////////////////////////////////////////////////////
//
// Helper for ExportCarVertices
//
HRESULT CarExporter::PrintCarModelVertices(MODEL *pModel, 
                                           UINT *piMatrix, 
                                           short Tpage, BOOL IsTransparent, BOOL IsTriangle,
                                           int *piVertex)
{
    MODEL_POLY *pModelPoly = pModel->PolyPtr;
    POLY_RGB *pRGB = pModel->PolyRGB;
    for (int iPoly = 0; iPoly < pModel->PolyNum; iPoly++, pModelPoly++, pRGB++)
    {
        if (pModelPoly->Tpage != Tpage) continue;   // skip if doesn't match current texture
        if (!IsTransparent && (pModelPoly->Type & POLY_SEMITRANS)) continue;
        if (IsTransparent && !(pModelPoly->Type & POLY_SEMITRANS)) continue;
        if (!IsTriangle && !(pModelPoly->Type & POLY_QUAD)) continue;
        if (IsTriangle && (pModelPoly->Type & POLY_QUAD)) continue;
        PrintCarPolyVertices(pModelPoly, pRGB, *piMatrix, piVertex, false, false);
        if (pModelPoly->Type & POLY_DOUBLE)
        {
            XDXPrintf("<!-- double-sided begin -->\n");
            PrintCarPolyVertices(pModelPoly, pRGB, *piMatrix, piVertex, true, true);
            XDXPrintf("<!-- double-sided end -->\n");
        }
    }
    (*piMatrix)++;
    return S_OK;
}

//////////////////////////////////////////////////////////////////////
//
// Helper for ExportCarVertices
//
HRESULT CarExporter::PrintCarAerialVertices(AERIAL *aerial, MODEL *secModel, MODEL *topModel,
                                            UINT *piMatrix,
                                            short Tpage, BOOL IsTransparent, BOOL IsTriangle, 
                                            int *piVertex)
{
    for (int iSec = AERIAL_START; iSec < AERIAL_NTOTSECTIONS; iSec++)
    {
        if (iSec != AERIAL_NTOTSECTIONS - 1) {
			XDXPrintf("<!-- aerial %d -->\n", iSec);
            PrintCarModelVertices(secModel, piMatrix, Tpage, IsTriangle, IsTriangle, piVertex);
       } else {
			XDXPrintf("<!-- aerial top -->\n");
            PrintCarModelVertices(topModel, piMatrix, Tpage, IsTriangle, IsTriangle, piVertex);
        }
    }
    return S_OK;
}

//////////////////////////////////////////////////////////////////////
//
// Export car vertices in one vertex buffer, grouped
// by matrix palette entry.
//
//
HRESULT CarExporter::ExportCarVertices()
{
    XDXPrintf("<!-- vertices fit in one big vertex buffer -->\n");
    XDXPrintf("<VertexBuffer id=\"%s_VertexBuffer\"\n", m_strName);
    XDXIndentBy(1);
    XDXPrintf(" names=\"POSITION NORMAL DIFFUSE TEXCOORD0 INDEX\"\n");
    XDXPrintf(" format=\"FLOAT3 FLOAT3 D3DCOLOR FLOAT2 SHORT1\"");
    XDXPrintf(">\n");
    
    int iVertex = 0;
    for (int IsTransparent = 0; IsTransparent < 2; IsTransparent++)
    {
        // Dump opaque vertices first, then transparent ones,
        // to match rendering order.
        if (!IsTransparent)
            XDXPrintf("<!-- Opaque -->\n");
        else
            XDXPrintf("<!-- Transparent -->\n");
        
        // Dump vertices sorted by opaque/transparent and by texture and by quad/tri
        if (IsTransparent)
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
        TextureTracker::iterator iTextureTracker;
        for (iTextureTracker = m_TextureTracker.begin();
             iTextureTracker != m_TextureTracker.end();
             iTextureTracker++)
        {
            short Tpage = (*iTextureTracker).first; // current texture page
            TextureCounts *pCurrentTextureCounts = &(*iTextureTracker).second;  // count of quads and tris
            for (int IsTriangle = 0; IsTriangle < 2; IsTriangle++)
            {
                // Print comment
                if (IsTransparent)
                {
                    if (!IsTriangle)
                    {
                        if (pCurrentTextureCounts->TransparentQuad == 0)
                            continue;
                        XDXPrintf("<!-- Tpage=\"%d\" TransparentQuad=\"%d\" -->\n", Tpage, pCurrentTextureCounts->TransparentQuad);
                    }
                    else
                    {
                        if (pCurrentTextureCounts->TransparentTri == 0)
                            continue;
                        XDXPrintf("<!-- Tpage=\"%d\" TransparentTri=\"%d\" -->\n", Tpage, pCurrentTextureCounts->TransparentTri);
                    }
                }
                else
                {
                    if (!IsTriangle)
                    {
                        if (pCurrentTextureCounts->OpaqueQuad == 0)
                            continue;
                        XDXPrintf("<!-- Tpage=\"%d\" OpaqueQuad=\"%d\" -->\n", Tpage, pCurrentTextureCounts->OpaqueQuad);
                    }
                    else 
                    {
                        if (pCurrentTextureCounts->OpaqueTri == 0)
                            continue;
                        XDXPrintf("<!-- Tpage=\"%d\" OpaqueTri=\"%d\" -->\n", Tpage, pCurrentTextureCounts->OpaqueTri);
                    }
                }

                // dump vertices for body
                UINT iMatrix = 0;
                BYTE lod = 0;  // NOTE: We only output the highest level of detail models for each part.
				XDXPrintf("<!-- body -->\n");
                PrintCarModelVertices(&m_pCar->Models->Body[lod], &iMatrix, Tpage, IsTransparent, IsTriangle, &iVertex);
                for (int ii = 0; ii < CAR_NWHEELS; ii++)
                {
                    // dump vertices for wheel
                    if (CarHasWheel(m_pCar, ii))
                    {
						XDXPrintf("<!-- wheel %d -->\n", ii);
                        PrintCarModelVertices(&m_pCar->Models->Wheel[ii][lod], &iMatrix, Tpage, IsTransparent, IsTriangle, &iVertex);
                    }

                    // dump vertices for spring
                    if (CarHasSpring(m_pCar, ii))
                    {
						XDXPrintf("<!-- spring %d -->\n", ii);
                        PrintCarModelVertices(&m_pCar->Models->Spring[ii][lod], &iMatrix, Tpage, IsTransparent, IsTriangle, &iVertex);
                    }

                    // dump vertices for axle
                    if (CarHasAxle(m_pCar, ii))
                    {
						XDXPrintf("<!-- axle %d -->\n", ii);
                        PrintCarModelVertices(&m_pCar->Models->Axle[ii][lod], &iMatrix, Tpage, IsTransparent, IsTriangle, &iVertex);
                    }

                    // dump vertices for pin
                    if (CarHasPin(m_pCar, ii))
                    {
						XDXPrintf("<!-- pin %d -->\n", ii);
                        PrintCarModelVertices(&m_pCar->Models->Pin[ii][lod], &iMatrix, Tpage, IsTransparent, IsTriangle, &iVertex);
                    }
                }

                // dump vertices for spinner
                if (CarHasSpinner(m_pCar))
                {
					XDXPrintf("<!-- spinner -->\n");
                    PrintCarModelVertices(&m_pCar->Models->Spinner[lod], &iMatrix, Tpage, IsTransparent, IsTriangle, &iVertex);
                }

                // dump vertices for aerial
                if (CarHasAerial(m_pCar)) 
                {
                    PrintCarAerialVertices(&m_pCar->Aerial, m_pCar->Models->Aerial[0], m_pCar->Models->Aerial[1],
                                           &iMatrix, Tpage, IsTransparent, IsTriangle, &iVertex);
                }
            }
        }
    }
    XDXIndentBy(-1);
    XDXPrintf("</VertexBuffer>\n");
    return S_OK;
}

//////////////////////////////////////////////////////////////////////
//
// Export car model with default matrix palette.
//
HRESULT CarExporter::ExportCarModels()
{
    const UINT CAR_MATRIX_PALETTE_SIZE = (1 + CAR_NWHEELS * 4 + 1 + AERIAL_NTOTSECTIONS);
    extern HRESULT FillCarMatrixPalette(CAR *pCar, XGMATRIX *rMatrixPalette, UINT *pMatrixCount);
    XGMATRIX rMatrixPalette[CAR_MATRIX_PALETTE_SIZE];
    UINT MatrixCount = CAR_MATRIX_PALETTE_SIZE;
    FillCarMatrixPalette(m_pCar, rMatrixPalette, &MatrixCount);
    extern HRESULT FillCarMatrixPaletteNames(CAR *pCar, CHAR *rMatrixPalette[], UINT *pMatrixCount);
	CHAR *rMatrixPaletteNames[CAR_MATRIX_PALETTE_SIZE];
	MatrixCount = CAR_MATRIX_PALETTE_SIZE;
	FillCarMatrixPaletteNames(m_pCar, rMatrixPaletteNames, &MatrixCount);

    // Dump opaque primitives first, then transparent ones.
    int iVertex = 0;
    for (int IsTransparent = 0; IsTransparent < 2; IsTransparent++)
    {
        if (IsTransparent)
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
            
        XDXPrintf("<Model id=\"%s%s\">\n", "main" /*m_strName*/, IsTransparent ? "_transparent" : "");
		XDXIndentBy(1);
        XDXPrintf("<Pass>\n");
		XDXIndentBy(1);
		XDXPrintf("<VertexShader id=\"%s_VertexShader\">\n", m_strName);
		XDXIndentBy(1);
        XDXPrintf("<decl>\n");
		XDXIndentBy(1);
        XDXPrintf("<stream index=\"0\" idref=\"%s_VertexBuffer\">\n", m_strName);
		XDXIndentBy(1);
		XDXPrintf("<vreg index=\"0\" name=\"POSITION\" format=\"FLOAT3\" />\n"
                  "<vreg index=\"1\" name=\"NORMAL\" format=\"FLOAT3\" />\n"
                  "<vreg index=\"2\" name=\"DIFFUSE\" format=\"D3DCOLOR\" />\n"
                  "<vreg index=\"3\" name=\"TEXCOORD0\" format=\"FLOAT2\" />\n"
                  "<vreg index=\"4\" name=\"INDEX\" format=\"SHORT1\" />\n");
		XDXIndentBy(-1);
		XDXPrintf("</stream>\n");
		XDXIndentBy(-1);
		XDXPrintf("</decl>\n");
        XDXPrintf("<asm>\n"
				  "vs.1.1\n"
				  "mad r0.x, v4.x, c1.x, c1.y	// get matrix palette index\n"
				  "mov a0.x, r0.x\n"
				  "m4x4 oPos, v0, c[a0.x]	    // transform point to projection space\n"
				  "// ignore normal, for now\n"
				  "mov oD0, v2		// copy vertex diffuse\n"
				  "mov oT0, v3		// texture coords\n"
                  "</asm>\n"
                  "<Constant index=\"0\" name=\"Diffuse\">1 1 1 1</Constant>\n"
                  "<Constant index=\"1\" name=\"MatrixPaletteScaleOffset\">4 4 1 1</Constant>\n");
        for (UINT iMatrix = 0; iMatrix < MatrixCount; iMatrix++)
        {
            CONST FLOAT *pf = (CONST FLOAT *)(rMatrixPalette[iMatrix]);
            XDXPrintf("<Constant index=\"%d\" parameter=\"%s_%s\" count=\"4\">\n"
                      "%g %g %g %g\n"
                      "%g %g %g %g\n"
                      "%g %g %g %g\n"
                      "%g %g %g %g\n"
                      "</Constant>\n",
                      iMatrix * 4 + 4,
					  m_strName, 
					  rMatrixPaletteNames[iMatrix],
                      pf[0], pf[1], pf[2], pf[3],
                      pf[4], pf[5], pf[6], pf[7],
                      pf[8], pf[9], pf[10], pf[11],
                      pf[12], pf[13], pf[14], pf[15]);
        }
		XDXIndentBy(-1);
        XDXPrintf("</VertexShader>\n");
		XDXIndentBy(-1);
		XDXPrintf("</Pass>\n");

		// This routine gloms all the geometry into one big model,
		// which is not necessarily what we want for different
		// materials on the tires, etc.

        // One pass for each texture
        short TpagePrevious = -1;
        TextureTracker::iterator iTextureTracker;
        for (iTextureTracker = m_TextureTracker.begin();
             iTextureTracker != m_TextureTracker.end();
             iTextureTracker++)
        {
            short Tpage = (*iTextureTracker).first; // current texture page to output
            TextureCounts *pCurrentTextureCounts = &(*iTextureTracker).second;  // count of quads and tris
            UINT QuadCount, TriCount;
            if (IsTransparent)
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
            XDXPrintf("<Pass>\n");
            XDXIndentBy(1);
            XDXPrintf("<RenderState>\n");
            XDXIndentBy(1);
            if (Tpage == -1)
            {
                XDXPrintf("<TextureState Stage=\"0\" idref=\"NULL\"\n");
                XDXIndentBy(1);
                XDXPrintf("ColorOp=\"SelectArg1\"\n");
                XDXPrintf("ColorArg1=\"Diffuse\"\n");
                XDXPrintf("AlphaOp=\"SelectArg1\"\n");
                XDXPrintf("AlphaArg1=\"Diffuse\"\n");
                XDXIndentBy(-1);
                XDXPrintf("/>\n");
            }
            else
            {
                XDXPrintf("<TextureState Stage=\"0\" idref=\"%s\"", TextureIdentifier(Tpage));
                if (TpagePrevious == -1)
                {
                    XDXPrintf("\n");
                    XDXIndentBy(1);
                    XDXPrintf("ColorOp=\"Modulate\"\n");
                    XDXPrintf("ColorArg1=\"Diffuse\"\n");
                    XDXPrintf("ColorArg2=\"Texture\"\n");
                    XDXPrintf("AlphaOp=\"Modulate\"\n");
                    XDXPrintf("AlphaArg1=\"Diffuse\"\n");
                    XDXPrintf("AlphaArg2=\"Texture\"\n");
                    XDXIndentBy(-1);
                }
                XDXPrintf("/>\n");
            }
            TpagePrevious = Tpage;
            XDXIndentBy(-1);
            XDXPrintf("</RenderState>\n");
    
            // Draw commands.  
            if (QuadCount > 0)
            {
                XDXPrintf("<Draw primitive=\"QUADLIST\" start=\"%d\" count=\"%d\" />\n",
                          iVertex, QuadCount * 4);
                iVertex += QuadCount * 4;
            }
            if (TriCount > 0)
            {
                XDXPrintf("<Draw primitive=\"TRILIST\" start=\"%d\" count=\"%d\" />\n",
                          iVertex, TriCount * 3);
                iVertex += TriCount * 3;
            }
                
            XDXIndentBy(-1);
            XDXPrintf("</Pass>\n");
        }
        XDXIndentBy(-1);
        XDXPrintf("</Model>\n");
    }

    return S_OK;
}

//////////////////////////////////////////////////////////////////////
//
// Export car matrix frame hierarchy
//
HRESULT CarExporter::ExportCarSkeleton()
{
    const UINT CAR_MATRIX_PALETTE_SIZE = (1 + CAR_NWHEELS * 4 + 1 + AERIAL_NTOTSECTIONS);
    extern HRESULT FillCarMatrixPalette(CAR *pCar, XGMATRIX *rMatrixPalette, UINT *pMatrixCount);
    XGMATRIX rMatrixPalette[CAR_MATRIX_PALETTE_SIZE];
    UINT MatrixCount = CAR_MATRIX_PALETTE_SIZE;
    FillCarMatrixPalette(m_pCar, rMatrixPalette, &MatrixCount);
    extern HRESULT FillCarMatrixPaletteNames(CAR *pCar, CHAR *rMatrixPalette[], UINT *pMatrixCount);
	CHAR *rMatrixPaletteNames[CAR_MATRIX_PALETTE_SIZE];
	MatrixCount = CAR_MATRIX_PALETTE_SIZE;
	FillCarMatrixPaletteNames(m_pCar, rMatrixPaletteNames, &MatrixCount);
	XDXPrintf("<Frame id=\"%s_Skeleton\">\n", m_strName);
	XDXIndentBy(1);
	for (UINT iMatrix = 0; iMatrix < MatrixCount; iMatrix++)
	{
		XDXPrintf("<Frame id=\"%s_%s\">\n", m_strName, rMatrixPaletteNames[iMatrix]);
		CONST FLOAT *pf = (CONST FLOAT *)(rMatrixPalette[iMatrix]);
		XDXPrintf("<Matrix value=\"\n"
				  "%g %g %g %g\n"
				  "%g %g %g %g\n"
				  "%g %g %g %g\n"
				  "%g %g %g %g\" />\n",
				  // Transpose matrix
				  pf[0], pf[4], pf[8], pf[12],
				  pf[1], pf[5], pf[9], pf[13],
				  pf[2], pf[6], pf[10], pf[14],
				  pf[3], pf[7], pf[11], pf[15]);
		XDXPrintf("</Frame>\n");
	}
	XDXIndentBy(-1);
	XDXPrintf("</Frame>\n");
    return S_OK;
}

//////////////////////////////////////////////////////////////////////
//
// Count number of quads and tris per car model.
//
// Helper for CountVerticesAndFacesPerTexture
//
HRESULT CarExporter::CountCarModel(MODEL *pModel)
{
    MODEL_POLY *pModelPoly = pModel->PolyPtr;
    TextureCounts *pCurrentTextureCounts;
    for (int iPoly = 0; iPoly < pModel->PolyNum; iPoly++, pModelPoly++)
    {
        pCurrentTextureCounts = &m_TextureTracker[ pModelPoly->Tpage ];
		int Increment;
		if (pModelPoly->Type & POLY_DOUBLE)
			Increment = 2;
		else
			Increment = 1;
        if (pModelPoly->Type & POLY_QUAD)
        {
            if (pModelPoly->Type & POLY_SEMITRANS)
            {
                m_TotalCount.TransparentQuad += Increment;
                pCurrentTextureCounts->TransparentQuad += Increment;
            }
            else
            {
                m_TotalCount.OpaqueQuad += Increment;
                pCurrentTextureCounts->OpaqueQuad += Increment;
            }
        }
        else
        {
            if (pModelPoly->Type & POLY_SEMITRANS)
            {
                m_TotalCount.TransparentTri += Increment;
                pCurrentTextureCounts->TransparentTri += Increment;
            }
            else
            {
                m_TotalCount.OpaqueTri += Increment;
                pCurrentTextureCounts->OpaqueTri += Increment;
            }
        }
    }
    return S_OK;
}

//////////////////////////////////////////////////////////////////////
//
// Count number of quads and tris for entire aerial, by adding
// up the aerial instance counts.
//
// Helper for CountVerticesAndFacesPerTexture
//
HRESULT CarExporter::CountCarAerial(AERIAL *aerial, MODEL *secModel, MODEL *topModel)
{
    for (int iSec = AERIAL_START; iSec < AERIAL_NTOTSECTIONS; iSec++)
    {
        if (iSec != AERIAL_NTOTSECTIONS - 1) {
            CountCarModel(secModel);
        } else {
            CountCarModel(topModel);
        }
    }
    return S_OK;
}

//////////////////////////////////////////////////////////////////////
// 
// Count the number of vertices, primitives,
// and unique textures used in each pass
//
HRESULT CarExporter::CountVerticesAndFacesPerTexture()
{
    m_TotalCount.clear();
    m_TextureTracker.clear();
    
    // body
    BYTE lod = 0;  // NOTE: We only output the highest level of detail models for each part.
    CountCarModel(&m_pCar->Models->Body[lod]);
    for (int ii = 0; ii < CAR_NWHEELS; ii++)
    {
        // wheel
        if (CarHasWheel(m_pCar, ii))
        {
            CountCarModel(&m_pCar->Models->Wheel[ii][lod]);
        }

        // spring
        if (CarHasSpring(m_pCar, ii))
        {
            CountCarModel(&m_pCar->Models->Spring[ii][lod]);
        }

        // axle
        if (CarHasAxle(m_pCar, ii))
        {
            CountCarModel(&m_pCar->Models->Axle[ii][lod]);
        }

        // pin
        if (CarHasPin(m_pCar, ii))
        {
            CountCarModel(&m_pCar->Models->Pin[ii][lod]);
        }
    }

    // spinner
    if (CarHasSpinner(m_pCar))
    {
        CountCarModel(&m_pCar->Models->Spinner[lod]);
    }

    // aerial
    if (CarHasAerial(m_pCar)) 
    {
        CountCarAerial(&m_pCar->Aerial, m_pCar->Models->Aerial[0], m_pCar->Models->Aerial[1]);
    }
    
    return S_OK;
}

//////////////////////////////////////////////////////////////////////
//
// Export car resources in XDX format, with a single vertex buffer.
//
HRESULT ExportCar(CAR *pCar, CAR_INFO *pCarInfo, CAR_MODEL *pCarModel, INT Tpage)
{
    // Get path for xdx file
    CHAR path[_MAX_PATH];
    extern CHAR *GetCarResourcePath( CHAR path_xbr[], CAR_INFO *pCarInfo );
    GetCarResourcePath(path, pCarInfo);

    CarExporter CE(path, pCar, pCarInfo, pCarModel);
    CE.CountVerticesAndFacesPerTexture();
    
    CHAR strMessage[1000];
    sprintf(strMessage, "Exporting car \"%s\" to \"%s\".", pCarInfo->Name, CE.m_path);
    DumpMessage(NULL, strMessage);

    if (XDXBegin( CE.m_path ) != S_OK)
        return E_FAIL;
    XDXPrintf("<include href=\"Default.xdx\" />\n");
    ExportTextureReference(path, NULL, TexInfo[Tpage].File, 0);      // main car texture
    ExportTextureReference(path, NULL, TexInfo[TPAGE_FX1].File, 0);  // aerial texture
    CE.ExportCarVertices();
    CE.ExportCarModels();
	CE.ExportCarSkeleton();
    return XDXEnd();
}


/*
//////////////////////////////////////////////////////////////////////
//
// Export car resources in XDX format, with a separate model
// for each part of the car.
//
HRESULT ExportCar(CAR *pCar, CAR_INFO *pCarInfo, CAR_MODEL *pCarModel, INT Tpage)
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
//  if (!_strnicmp(drive, "D:", 2))
//      strcpy(drive, "U:");    // replace D: with U:
    _makepath( path, drive, dir, name, ".xdx");
    CHAR strMessage[1000];
    sprintf(strMessage, "Exporting car \"%s\" to \"%s\".", pCarInfo->Name, path);
    DumpMessage(NULL, strMessage);
    
    // Output car models
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
//          if (!_strnicmp(drive2, "D:", 2))
//              strcpy(drive2, "U:");   // replace D: with U:
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
//          if (!_strnicmp(drive2, "D:", 2))
//              strcpy(drive2, "U:");   // replace D: with U:
            _makepath(path2, drive2, dir2, name2, ".xdx");
            XDXPrintf("<include href=\"%s\" />\n", RelativePath(path, path2));
        }
    }
    return XDXEnd();
}
*/

#endif // !SHIPPING
