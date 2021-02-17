#ifndef __STRUCTS_H
#define __STRUCTS_H

#include <limits.h>
#include <bobtypes.h>

#define FOR_PC  1		// Bit values representing intended target machines
#define FOR_PSX 2       // These are for use with the VALID_TARGETS data type
#define FOR_N64 4       // An instance of this type can be found at the start of a track unit file

typedef U16 VALID_TARGETS;	// Bitfield indicating which target machines a file is valid for

const S16 MAX_ELEVATION = 16;
// number of 0.5 metre steps in 8 metres

#ifndef INDEX
typedef short INDEX;
const INDEX MAX_INDEX = SHRT_MAX;
#endif

//-------------------------------------------------------------------------------------------------
// Unit orientation data type
//
//-------------------------------------------------------------------------------------------------
typedef enum DIRECTION{NORTH, EAST, SOUTH, WEST};

//-------------------------------------------------------------------------------------------------
// Type definition for  atype used to express a portion - used in UV coords for example
//-------------------------------------------------------------------------------------------------
#ifdef _PSX
    typedef U8 REALPORTION;		//PSX portions expressed in the range of 0 to 255
#else
    typedef float REALPORTION;	//Other platforms use range 0.0 to 1.0 (or higher for tiling)
#endif

//-------------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------------
typedef struct
{
    REALPORTION U;
    REALPORTION V;
}RevoltUVCoord;

typedef struct 
{
	float X;
	float Y;
	float Z;
}RevoltVertex;

typedef struct
{
    INDEX		UnitID;	 // Index into the TrackUnit array
    DIRECTION	Direction; 
    S16			XPos;
    S16			YPos;
    S16			Elevation;
	D3DMATRIX	Matrix;
}RevoltTrackUnitInstance;

typedef struct
{
    INDEX		ModuleID;	 // Index into the Module array
    DIRECTION	Direction;	 // which direction the module is facing
    S16			Elevation;
	S16			XOffset;
	S16			YOffset;
}RevoltTrackModuleInstance;

typedef struct
{
	RevoltVertex	Position;
}TRACKLINK;

typedef struct
{
	RevoltVertex	Centre;
	float			XSize;
	float			YSize;
	float			ZSize;
	TRACKLINK		Links[2];
}TRACKZONE;

#endif
