#ifndef _COMPILE_H
#define _COMPILE_H

#include "UnitInfo.h"
#include <typedefs.h>
#include <world.h>
#include <newcoll.h>

//**********************************************
//
//**********************************************
typedef struct
{
	CUBE_HEADER_LOAD	Header;
	WORLD_POLY_LOAD*	Polys;
	WORLD_VERTEX_LOAD*	Verts;
}SMALLCUBE;

typedef struct
{
	BIG_CUBE_HEADER_LOAD Header;		//describes location and radius of cube
	U32*				 CubeIndices;	//indices of the small cubes which are within this big cube
}BIGCUBE;

typedef struct
{
	S32		NumEntries;
	S32*	CollPolyIndices;
}CELLDATA;

typedef struct
{
	S16				NumPolys;
	S16				NumUsedPolys;
	NEWCOLLPOLY*	Polys;
	S16*			PolyStatus;
	COLLGRID_DATA	Header;
	CELLDATA*		Cells;
}NCPDATA;

typedef struct
{
	INDEX	ZoneID;
	bool	Forwards;
}ZONESEQENTRY;
//**********************************************
//
//**********************************************
void CompileTrack(const TRACKDESC* track, const TRACKTHEME* theme, CURSORDESC* cursor);

#endif
