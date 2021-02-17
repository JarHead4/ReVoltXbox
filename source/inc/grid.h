/*********************************************************************************************
 *
 * grid.h
 *
 * Re-Volt (Generic) Copyright (c) Probe Entertainment 1998
 * 
 * Contents:
 *			Object and AI node grid system, incorporating object test mark-off system.
 *
 *********************************************************************************************
 *
 * 22/07/98 MTaylor
 *	File inception.
 *
 *********************************************************************************************/

#ifndef _GRID_H_
#define _GRID_H_

//
// Defines and macros
//

#define	MAX_GRIDS			4									// Number of grids used by the system (must be factor of 2)
#define MAX_SHIFTS			2									// Number of times the grid is "shifted" (offset) on each axis

#define OBJ_GRID_WIDTH		32									// X and Y width of each grid
#define OBJ_GRID_SIZE		(OBJ_GRID_WIDTH * OBJ_GRID_WIDTH)	// Number of grid units making up entire grid
#define OBJ_UNIT_SIZE		(65536 / OBJ_GRID_WIDTH)  			// Size in world units of one side of a grid square
#define OBJ_SHIFT_OFFSET	(OBJ_UNIT_SIZE / MAX_SHIFTS)		// Amount added to grid centre for each shift

#define NODE_GRID_WIDTH		32									// X and Y width of each grid
#define NODE_GRID_SIZE		(NODE_GRID_WIDTH * NODE_GRID_WIDTH)	// Number of grid units making up entire grid
#define NODE_UNIT_SIZE		(65536 / NODE_GRID_WIDTH) 			// Size in world units of one side of a grid square
#define NODE_SHIFT_OFFSET	(NODE_UNIT_SIZE / MAX_SHIFTS)		// Amount added to grid centre for each shift


//
// Typedefs and structures
//

typedef struct _OBJGRIDUNIT
{
	struct object_def	*ObjHead;
} OBJGRIDUNIT;


typedef struct _NODEGRIDUNIT
{
	struct _AINODE		*NodeHead;
} NODEGRIDUNIT;


//
// External global variables
//

extern OBJGRIDUNIT	*ObjGrid[MAX_GRIDS];
extern NODEGRIDUNIT	*NodeGrid[MAX_GRIDS];

//
// External function prototypes
//

extern long GRD_AllocGrids(void);
extern void GRD_FreeGrids(void);
extern void GRD_UpdateObjGrid(void);
extern void GRD_RemoveObject(struct object_def *Obj);
extern void GRD_ResetObjPairs(void);
extern long GRD_ObjectPair(struct object_def *Obj1, struct object_def *Obj2);
extern void GRD_GridNodes(struct _AINODE *Src, long Num);

#endif