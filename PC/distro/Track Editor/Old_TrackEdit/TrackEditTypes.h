#ifndef _TRACKEDITTYPES_H
#define _TRACKEDITTYPES_H

#define D3D_OVERLOADS
#include <ddraw.h>
#include <d3d.h>
#include <d3drm.h>
#include <bobtypes.h>
#include <primitives.h>

/*
**-----------------------------------------------------------------------------
**	Typedefs for use with the Re-Volt Track Editor
**-----------------------------------------------------------------------------
*/

typedef struct
{
	INDEX   TPageID;
	INDEX   PolyID;
	U8	    Rotation;
	bool	Reversed;
}UVPOLYINSTANCE;

typedef struct
{
	INDEX	MeshID;
	U32		UVPolyCount;
	UVPOLYINSTANCE UVPolys[1];
}TRACKUNIT;

typedef struct
{
	INDEX		Unit;
	D3DMATRIX	Matrix;
}TRACKUNITINSTANCE;

typedef struct
{
	BOOL IsTriangle;
	U32 Vertices[4];
}BASICPOLY;

typedef struct
{
	U32  PolygonCount;
	U32	 VertexCount;
	U32* Indices;
}POLYSET;

typedef struct
{
	U16		  PolySetCount;
	POLYSET** PolySets;
}MESH;			

typedef struct
{
	U32         VertexCount;
	D3DVERTEX* Vertices;
}PRIMITIVE;		//represents a subset of a COMPONENT which share a material

typedef struct
{
	U32 PrimitiveCount;
	PRIMITIVE** Primitives;
}COMPONENT;		//represents the geometry of a single component within a track unit (peg, pan or hull)

typedef struct
{
	U32 ComponentCount;
	COMPONENT** Components;
}DXUNIT;		//represents the geometry of a complete track unit

typedef struct
{
	U16 X;
	U16 Y;
	U16 XMax;
	U16 YMax;
	U32 AbsMax;
	bool Wraps;
}CURSOR_DESC;

#endif //_TRACKEDITTYPES_H