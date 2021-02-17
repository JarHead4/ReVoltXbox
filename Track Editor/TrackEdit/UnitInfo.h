#ifndef __UNITINFO_H
#define __UNITINFO_H

#include <string>
#include "modules.h"

typedef struct
{
	TRACKMODULE**		Modules;
	TRACKUNIT**			Units;
	MESH*				Meshes;
	BASICPOLY*			Polys;
	BASICPOLY*			UVPolys;
	D3DVECTOR*			Verts;
	RevoltUVCoord*		UVCoords;
	MODULE_LOOKUP_INFO*	Lookup;
	U32					UnitCount;
	U32					UVCoordCount;
	U32					UVPolyCount;
	U32					VertCount;
	U32					PolyCount;
	U32					MeshCount;
	U32					ModuleCount;
	U32					TPageCount;
}TRACKTHEME;

enum {PEG_INDEX, PAN_INDEX, HULL_INDEX};
enum {MODULE_SPACER, MODULE_STARTGRID};
enum {UNIT_SPACER, UNIT_STARTGRID};

#endif //__UNITINFO_H