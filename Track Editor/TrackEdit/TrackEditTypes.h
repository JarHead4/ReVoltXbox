#ifndef _TRACKEDITTYPES_H
#define _TRACKEDITTYPES_H

#define D3D_OVERLOADS
#include <ddraw.h>
#include <d3d.h>
#include <d3drm.h>
#include <structs.h>
#include <constants.h>
#include <typedefs.h>

#include "editorconstants.h"

/*
**-----------------------------------------------------------------------------
**	Typedefs for use with the Re-Volt Track Editor
**-----------------------------------------------------------------------------
*/

enum MAIN_STATES
{
	MLS_IN_MENU,		//user is in the menu which allows them to choose what to do
	MLS_NEW_TRACK,		//user is selecting a theme to use for a new track
	MLS_LOAD_TRACK,		//user is selecting a pre-created track
	MLS_DELETE_TRACK,	//user is deleting a pre-created track
	MLS_EDIT_TRACK,		//user is editing a track
	MLS_MANAGE_TRACK	//user is managing a track (saving, resizing, shifting)
};

enum MAIN_MENU_STATES
{
	MLM_NEW_TRACK,
	MLM_LOAD_TRACK,
	MLM_DELETE_TRACK,
	MLM_QUIT
};

enum NEW_TRACK_STATES
{
	NTS_TOY_WORLD,
	NTS_RC_WORLD,
	NTS_CANCEL
};

enum LOAD_TRACK_STATES
{
	LTS_CHOOSING,
	LTS_CANCEL
};

enum SAVE_TRACK_STATES
{
	STS_ENTERING,
	STS_SAVE,
	STS_CANCEL
};

enum PLACEMENT_CAMERA_DIR
{
	PCD_NORTH,
	PCD_EAST,
	PCD_SOUTH,
	PCD_WEST
};

enum MODULE_ORIENTATION
{
	MO_NORTH,
	MO_EAST,
	MO_SOUTH,
	MO_WEST
};

enum THEME_TYPE
{
	TT_TOY,
	TT_RC
};

enum EDIT_SCREEN_STATES
{
	ESS_CHOOSING_MODULE,
	ESS_PLACING_PICKUPS,
	ESS_ADJUSTING_TRACK,
	ESS_SAVING_TRACK,
	ESS_CREATING_TRACK,
	ESS_LOADING_TRACK,
	ESS_EXPORTING_TRACK,
	ESS_QUITTING,
	ESS_PLACING_MODULE
};

enum POPUP_MENU_STATES
{
	PMS_INACTIVE,
	PMS_APPEARING,
	PMS_DISAPPEARING,
	PMS_ACTIVE,
	PMS_MOVING_UP,
	PMS_MOVING_DOWN
};

enum POPUP_MENU_ACTIONS
{
	PMA_DISMISSED,
	PMA_SELECTED
};

enum COMPILE_STATES
{
	TCS_VALIDATING,
	TCS_UNIT_CUBES,
	TCS_SMALL_CUBES,
	TCS_BIG_CUBES,
	TCS_MAKING_COLLISONS,
	TCS_OPTIMIZING,
	TCS_WRITING_WORLD,
	TCS_WRITING_INF,
	TCS_WRITING_BITMAPS,
	TCS_WRITING_NCP,
	TCS_WRITING_TAZ,
	TCS_CLEANUP,
	TCS_RINGING_CLOCK
};

enum {SECTION_MODULE_GRID = 1};	//specifies the bit values for each of the possible sections within a TDF file

const U8 MAX_DESCRIPTION_LENGTH = 40; //longest size of description field within a TDF file
const U8 MAX_FILES_IN_WINDOW = 12; //number of files visible within the file selection window
const U16 FILE_WINDOW_LEFT = 48;
const U16 FILE_WINDOW_TOP = 200;
const U16 CANCEL_WINDOW_LEFT = FILE_WINDOW_LEFT + 328;
const U16 CANCEL_WINDOW_TOP = FILE_WINDOW_TOP;

const REAL SMALL_CUBE_SIZE = GameScale * 4;
const REAL SMALL_CUBE_SPACING = SMALL_CUBE_SIZE * Real(1.25f);
const REAL BIG_CUBE_SIZE = SMALL_CUBE_SIZE * 4;
const REAL BIG_CUBE_RADIUS = ((REAL)sqrt((BIG_CUBE_SIZE * BIG_CUBE_SIZE) * Real(3.0f))) / Real(2.0f);
const REAL COLLISION_BLEED = Real(100.0f);

typedef struct
{
	INDEX	TPageID;
	INDEX	PolyID;
	U8		Rotation;
	bool	Reversed;
}UVPOLYINSTANCE;

typedef struct
{
	INDEX			MeshID;
	U32				UVPolyCount;
	UVPOLYINSTANCE* UVPolys;
	U32				SurfaceCount;
	U32*			Surfaces;

}TRACKUNIT;

typedef struct
{
	char						Name[80];
	U16							InstanceCount;
	RevoltTrackUnitInstance**	Instances;
	U16							ZoneCount;
	TRACKZONE**					Zones;
}TRACKMODULE;

typedef struct
{
	BOOL	IsTriangle;
	U32		Vertices[4];
}BASICPOLY;

typedef struct
{
	U32		PolygonCount;
	U32		VertexCount;
	U32*	Indices;
}POLYSET;

typedef struct
{
	U16		  PolySetCount;
	POLYSET** PolySets;
}MESH;			

typedef struct
{
	U32         VertexCount;
	D3DVERTEX*	Vertices;
}PRIMITIVE;		//represents a subset of a COMPONENT which share a material

typedef struct
{
	U32			PrimitiveCount;
	PRIMITIVE** Primitives;
}COMPONENT;		//represents the geometry of a single component within a track unit (peg, pan or hull)

typedef struct
{
	U32			ComponentCount;
	COMPONENT** Components;
}DXUNIT;		//represents the geometry of a complete track unit

typedef struct
{
	U16 X;
	U16 Y;
	U16 XMax;
	U16 YMax;
	U32 AbsMax;
	S16 XSize;
	S16 YSize;
}CURSORDESC;

typedef struct
{
	U32							Width;
	U32							Height;
	char						Name[MAX_DESCRIPTION_LENGTH];
	RevoltTrackUnitInstance*	Units;
	RevoltTrackModuleInstance*	Modules;
	THEME_TYPE					ThemeType;
}TRACKDESC;

#endif //_TRACKEDITTYPES_H