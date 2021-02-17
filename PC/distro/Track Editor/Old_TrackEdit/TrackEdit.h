#ifndef TRACKEDIT_H
#define TRACKEDIT_H
/*
**-----------------------------------------------------------------------------
**  File:       TrackEdit.h
**  Purpose:    Sample showing DrawPrimitive functionality 
**
**	Copyright (C) 1995 - 1997 Microsoft Corporation. All Rights Reserved.
**-----------------------------------------------------------------------------
*/

/*
**-----------------------------------------------------------------------------
**	Include files
**-----------------------------------------------------------------------------
*/

#include <string>
#define D3D_OVERLOADS
#include <ddraw.h>
#include <d3d.h>
#include <d3drm.h>
#include "Common.h"
#include "d3dscene.h"
#include "d3dutils.h"
#include "d3dtex.h"

#include "debug.h"
#include <bobtypes.h>
#include <endstrm.h>
#include <primitives.h>

/*
**-----------------------------------------------------------------------------
**	Typedefs
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
}CURSOR_DESC;

/*
**-----------------------------------------------------------------------------
**	class definition
**-----------------------------------------------------------------------------
*/

#define PI 3.141592653
#define TRACK_WIDTH 6
#define TRACK_HEIGHT 6

using namespace std;

class TrackEdit: public D3DScene
{
public:
	 TrackEdit();
	virtual void OnKeyStroke(int vKeyCode);
	virtual HRESULT Init (LPD3DWindow lpd3dWindow);
	virtual HRESULT Fini (void);

	virtual HRESULT AttachViewport (void);
	virtual HRESULT DetachViewport (void);

	virtual HRESULT Render (void);
protected:
	void ReadUnits(EndianInputStream& is);
	void EraseGeometry(void);
	void ReadMeshes(EndianInputStream& is);
	void ReadPolygons(EndianInputStream& is);
	void ReadVertices(EndianInputStream& is);
	void ReadUVCoords(EndianInputStream& is);
	void ReadUVPolygons(EndianInputStream& is);
	HRESULT ParseCommandline(void);
	void PlaceUnit(void);
	void ToggleStatus(void);
	void DestroyTrack(void);
	bool CreateTrack(U32 width, U32 height);
	void MoveCursor(CURSOR_DESC* cursor, int vKeyCode);
	HRESULT DrawComponent(LPDIRECT3DDEVICE2 lpDev, const COMPONENT* component);
	HRESULT DrawUnits(LPDIRECT3DDEVICE2 lpDev);
	void CreatePrimitives(void);
	void ReadAscFile(void);
	void ReadRTUFile(const string& filename);

	string				RTUFileName;
	D3DMATRIX			m_World, m_View, m_Projection;
	D3DVECTOR*			Verts;
	BASICPOLY*			Polys;
	BASICPOLY*			UVPolys;
	RevoltUVCoord*		UVCoords;
	MESH*				Meshes;
	TRACKUNITINSTANCE*	Instances;
	TRACKUNIT**			Units;
	DXUNIT**			DXUnits;
	U32					VertCount;
	U32					PolyCount;
	U32					UVPolyCount;
	U32					UVCoordCount;
	U32					MeshCount;
	U32					UnitCount;
	float				Spin;
	CURSOR_DESC			UnitCursor;
	CURSOR_DESC			TrackCursor;
	INDEX				CurrentMat;
	RevoltTrackUnitInstance* TrackGrid;
	U32					TrackWidth;
	U32					TrackHeight;
	enum EDIT_STATUS {CHOOSING, PLACING};
	EDIT_STATUS			Edit_Status;
	enum {PEG_INDEX, PAN_INDEX, HULL_INDEX};
};




/*
**----------------------------------------------------------------------------
** End of File
**----------------------------------------------------------------------------
*/
#endif // TRACKEDIT_H


