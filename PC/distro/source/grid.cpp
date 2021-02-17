/*********************************************************************************************
 *
 * grid.cpp
 *
 * Re-Volt (PC) Copyright (c) Probe Entertainment 1998
 * 
 * Contents:
 *			Object and AI node grid system, incorporating object test mark-off system.
 *
 *********************************************************************************************
 *
 * 22/07/98 MTaylor
 *	File inception. Basic grid and object mark-off functions created.
 * 
 *********************************************************************************************/

#include "revolt.h"
#include "main.h"
#include "geom.h"
#include "particle.h"
#include "model.h"
#include "aerial.h"
#include "newcoll.h"
#include "body.h"
#include "car.h"
#include "ctrlread.h"
#include "object.h"
#include "ainode.h"
#include "editai.h"

//
// Static variables
//

static char		*s_ObjPair;


//
// Global variables
//

OBJGRIDUNIT		*ObjGrid[MAX_GRIDS];
NODEGRIDUNIT	*NodeGrid[MAX_GRIDS];


//
// Static Functions
//

static long s_LineIntersect(REAL x0, REAL z0, REAL x1, REAL z1, REAL x2, REAL z2, REAL x3, REAL z3);
static REAL s_PointLine(REAL x, REAL y, REAL x0, REAL y0, REAL x1, REAL y1);
static long	s_PointInNodePoly(REAL x, REAL y, AINODE *Node, long Path);


//
// Global functions
//

long GRD_AllocGrids(void);
void GRD_FreeGrids(void);
void GRD_UpdateObjGrid(void);
void GRD_RemoveObject(OBJECT *Obj);
void GRD_ResetObjPairs(void);
long GRD_ObjectPair(OBJECT *Obj1, OBJECT *Obj2);
void GRD_GridNodes(AINODE *Src, long Num);


//--------------------------------------------------------------------------------------------------------------------------

//
// GRD_AllocGrids
//
// Allocates buffers for the grids and object pair system
//

long GRD_AllocGrids(void)
{
	long	ii;

	for (ii = 0; ii < MAX_GRIDS; ii++)
	{
		ObjGrid[ii] = (OBJGRIDUNIT *)malloc(OBJ_GRID_SIZE * sizeof(OBJGRIDUNIT));
		if (!ObjGrid[ii])
		{
			Error("GRD", "GRD_AllocGrid", "Could not alloc memory for object grid system", 1);		
			return(FALSE);
		}
		memset(ObjGrid[ii], 0, OBJ_GRID_SIZE * sizeof(OBJGRIDUNIT));		// Initialise pointers to NULL (memset, rather than memclr for N64 compatability)

		NodeGrid[ii] = (NODEGRIDUNIT *)malloc(NODE_GRID_SIZE * sizeof(NODEGRIDUNIT));
		if (!NodeGrid[ii])
		{
			Error("GRD", "GRD_AllocGrid", "Could not alloc memory for object grid system", 1);		
			return(FALSE);
		}
		memset(NodeGrid[ii], 0, NODE_GRID_SIZE * sizeof(NODEGRIDUNIT));		// Initialise pointers to NULL (memset, rather than memclr for N64 compatability)
	}

	s_ObjPair = (char *)malloc(MAX_OBJECTS * MAX_OBJECTS);
	if (!s_ObjPair)
	{
		Error("GRD", "GRD_AllocGrid", "Could not alloc memory for object pair buffer", 1);		
		return(FALSE);
	}
	memset(s_ObjPair, 0, MAX_OBJECTS * MAX_OBJECTS);

	return (TRUE);
}

//--------------------------------------------------------------------------------------------------------------------------

//
// GRD_FreeGrid
//
// Deallocates the buffers used by the grid system
//

void GRD_FreeGrids(void)
{
	long	ii;

	for (ii = 0; ii < MAX_GRIDS; ii++)
	{
		free(ObjGrid[ii]);
		free(NodeGrid[ii]);
	}
	free(s_ObjPair);
}

//--------------------------------------------------------------------------------------------------------------------------

//
// GRD_UpdateObjGrid
//
// Checks all active objects to see if they have changed grid location for each grid, updating if necessary.
// Also adds new objects into grid.
//

void GRD_UpdateObjGrid(void)
{
	long	x, z;
	long   	xShift, zShift;
	long 	xOff, zOff;
	long   	Grid;
	OBJECT 	*Obj;
	OBJECT 	*TempObj;

	for (zShift = 0; zShift < MAX_SHIFTS; zShift++)
	{
		zOff = zShift * OBJ_SHIFT_OFFSET;
		for (xShift = 0; xShift < MAX_SHIFTS; xShift++)
		{
			xOff = xShift * OBJ_SHIFT_OFFSET;
			Grid = xShift + (zShift * MAX_SHIFTS);
			for (Obj = OBJ_ObjectHead; Obj; Obj = Obj->next)
			{										   									// First off, generate x & z indices for grid access
				x = (long)(Obj->body.Centre.Pos.v[X] + 32768 - xOff) / OBJ_UNIT_SIZE;  	// !MT! This needs to be done with shifts on PSX
				z = (long)(Obj->body.Centre.Pos.v[Z] + 32768 - zOff) / OBJ_UNIT_SIZE;  	// !MT! So OBJ_UNIT_SIZE would need to be a power of 2
				if (x < 0) continue;													// Bounds check x & z
				if (z < 0) continue;
				if (x > (OBJ_GRID_WIDTH - 1)) continue;
				if (z > (OBJ_GRID_WIDTH - 1)) continue;
				if ((Obj->flag.IsInGrid) && (x == Obj->GridX[Grid])
				   && (z == Obj->GridZ[Grid])) continue;

				if (ObjGrid[Grid][x + (z * OBJ_GRID_WIDTH)].ObjHead)
				{
					TempObj = ObjGrid[Grid][x + (z * OBJ_GRID_WIDTH)].ObjHead;
					ObjGrid[Grid][x + (z * OBJ_GRID_WIDTH)].ObjHead = Obj;
					Obj->GridNext[Grid] = TempObj;
					Obj->GridPrev[Grid] = NULL;
					TempObj->GridPrev[Grid] = Obj;
				}
				else
				{
					ObjGrid[Grid][x + (z * OBJ_GRID_WIDTH)].ObjHead = Obj;
					Obj->GridPrev[Grid] = NULL;
					Obj->GridNext[Grid] = NULL;
				}
				Obj->GridX[Grid] = (unsigned short)x;							// Update current grid location
				Obj->GridZ[Grid] = (unsigned short)z;
				Obj->flag.IsInGrid = 1;											// Mark object as being in grid
			}
		}
	}
}

//--------------------------------------------------------------------------------------------------------------------------

void GRD_RemoveObject(OBJECT *Obj)
{
#if 0
	long	ii;
	long	Offset;

	Assert((bool)Obj->flag.IsInGrid);

	for (ii = 0; ii < MAX_GRIDS; ii++)
	{
		Offset = Obj->GridX[ii] + (Obj->GridZ[ii] * OBJ_GRID_WIDTH);
		if (ObjGrid[ii][Offset].ObjHead)
		{																				// Remove object from this list
			if (Obj->GridPrev[ii] == NULL)
			{																			// Object is at head of list
				if (Obj->GridNext[ii] == NULL)
				{																		// Object is only one in list
					ObjGrid[ii][Offset].ObjHead = NULL;
				}
				else
				{
					Obj->GridNext[ii]->GridPrev[ii] = NULL;
					ObjGrid[ii][Offset].ObjHead = Obj->GridNext[ii];
				}
			}
			else
			{																			// Object is mid-list
				Obj->GridPrev[ii]->GridNext[ii] = Obj->GridNext[ii];
				if (Obj->GridNext[ii] != NULL)
				{
					Obj->GridNext[ii]->GridPrev[ii] = Obj->GridPrev[ii];
				}
			}
		}
	}
	Obj->flag.IsInGrid = 0;															// Mark object as not in grid yet
#endif
}

//--------------------------------------------------------------------------------------------------------------------------

//
// GRD_ResetObjPairs
//
// Resets object pair buffer
//

void GRD_ResetObjPairs(void)
{
	memset(s_ObjPair, 0, MAX_OBJECTS * MAX_OBJECTS);
}

//--------------------------------------------------------------------------------------------------------------------------

long GRD_ObjectPair(OBJECT *Obj1, OBJECT *Obj2)
{
	long	ii, jj;

	ii = Obj1->ObjID;
	jj = Obj2->ObjID;

	if (s_ObjPair[ii + (jj * MAX_OBJECTS)])
	{
		return(TRUE);
	}
	else
	{
		s_ObjPair[ii + (jj * MAX_OBJECTS)] = (char)0xFF;
		return(FALSE);
	}
}

//--------------------------------------------------------------------------------------------------------------------------

//
// GRD_GridNodes
//
// Builds grid lists of nodes.
// There are 3 cases we need to test WRT each node:
// 
//	1. Is one of the node vertices within a grid square
//	2. Does the span between the two node vertices intersect a grid square
//	3. Does any part of the grid square intersect with a polygon built from the two node vertices and forward node vertices
//

void GRD_GridNodes(AINODE *Src, long Num)
{
/*	long	ii;
	long	Include;
	REAL	xUnit, zUnit;
	REAL	x0, z0, x1, z1;
	long	xCnt, zCnt, nodeCnt;
	long   	xShift, zShift;
	long 	xOff, zOff;
	long   	Grid;
	AINODE 	*Node;
	AINODE 	*TempNode;

	for (zShift = 0; zShift < MAX_SHIFTS; zShift++)
	{
		zOff = zShift * NODE_SHIFT_OFFSET;
		for (xShift = 0; xShift < MAX_SHIFTS; xShift++)
		{
			xOff = xShift * NODE_SHIFT_OFFSET;
			Grid = xShift + (zShift * MAX_SHIFTS);
			for (zCnt = 0; zCnt < NODE_GRID_WIDTH; zCnt++)
			{
				zUnit = (REAL)(zCnt * NODE_UNIT_SIZE) - 32768 + zOff;
				for (xCnt = 0; xCnt < NODE_GRID_WIDTH; xCnt++)
				{
					xUnit = (REAL)(xCnt * NODE_UNIT_SIZE) - 32768 + xOff;
					for (nodeCnt = 0, Node = Src; nodeCnt < Num; nodeCnt++, Node++)
					{
						Include = FALSE;

						// Check if either node vertex is inside this grid square
						x0 = Node->Node[0].Pos.v[X];
						z0 = Node->Node[0].Pos.v[Z];
						x1 = Node->Node[1].Pos.v[X];
						z1 = Node->Node[1].Pos.v[Z];
						if ((x0 >= xUnit) && (x0 < (xUnit + NODE_UNIT_SIZE)) && (z0 >= zUnit) && (z0 < (zUnit + NODE_UNIT_SIZE))) Include = TRUE;
						if ((x1 >= xUnit) && (x1 < (xUnit + NODE_UNIT_SIZE)) && (z1 >= zUnit) && (z1 < (zUnit + NODE_UNIT_SIZE))) Include = TRUE;

						if(!Include)
						{
							// Check for node/grid line intersections
							if (s_LineIntersect(x0, z0, x1, z1, xUnit, zUnit, xUnit + NODE_GRID_WIDTH, zUnit)) { Include = TRUE; }
							if (s_LineIntersect(x0, z0, x1, z1, xUnit, zUnit + NODE_GRID_WIDTH, xUnit + NODE_GRID_WIDTH, zUnit + NODE_GRID_WIDTH)) { Include = TRUE; }
							if (s_LineIntersect(x0, z0, x1, z1, xUnit, zUnit, xUnit, zUnit + NODE_GRID_WIDTH)) { Include = TRUE; }
							if (s_LineIntersect(x0, z0, x1, z1, xUnit + NODE_GRID_WIDTH, zUnit, xUnit + NODE_GRID_WIDTH, zUnit + NODE_GRID_WIDTH)) { Include = TRUE; }

							for (ii = 0; ii < MAX_AINODE_LINKS; ii++)
							{	
								if (!Node->Next[ii]) continue;
								if(!Include)
								{
									x0 = Node->Node[0].Pos.v[X];
									z0 = Node->Node[0].Pos.v[Z];
									x1 = Node->Next[ii]->Node[1].Pos.v[X];
									z1 = Node->Next[ii]->Node[1].Pos.v[Z];
									if (s_LineIntersect(x0, z0, x1, z1, xUnit, zUnit, xUnit + NODE_GRID_WIDTH, zUnit)) { Include = TRUE; }
									if (s_LineIntersect(x0, z0, x1, z1, xUnit, zUnit + NODE_GRID_WIDTH, xUnit + NODE_GRID_WIDTH, zUnit + NODE_GRID_WIDTH)) { Include = TRUE; }
									if (s_LineIntersect(x0, z0, x1, z1, xUnit, zUnit, xUnit, zUnit + NODE_GRID_WIDTH)) { Include = TRUE; }
									if (s_LineIntersect(x0, z0, x1, z1, xUnit + NODE_GRID_WIDTH, zUnit, xUnit + NODE_GRID_WIDTH, zUnit + NODE_GRID_WIDTH)) { Include = TRUE; }
								}
								if(!Include)
								{
									x0 = Node->Next[ii]->Node[0].Pos.v[X];
									z0 = Node->Next[ii]->Node[0].Pos.v[Z];
									x1 = Node->Next[ii]->Node[1].Pos.v[X];
									z1 = Node->Next[ii]->Node[1].Pos.v[Z];
									if (s_LineIntersect(x0, z0, x1, z1, xUnit, zUnit, xUnit + NODE_GRID_WIDTH, zUnit)) { Include = TRUE; }
									if (s_LineIntersect(x0, z0, x1, z1, xUnit, zUnit + NODE_GRID_WIDTH, xUnit + NODE_GRID_WIDTH, zUnit + NODE_GRID_WIDTH)) { Include = TRUE; }
									if (s_LineIntersect(x0, z0, x1, z1, xUnit, zUnit, xUnit, zUnit + NODE_GRID_WIDTH)) { Include = TRUE; }
									if (s_LineIntersect(x0, z0, x1, z1, xUnit + NODE_GRID_WIDTH, zUnit, xUnit + NODE_GRID_WIDTH, zUnit + NODE_GRID_WIDTH)) { Include = TRUE; }
								}
								if(!Include)
								{
									x0 = Node->Next[ii]->Node[1].Pos.v[X];
									z0 = Node->Next[ii]->Node[1].Pos.v[Z];
									x1 = Node->Node[1].Pos.v[X];
									z1 = Node->Node[1].Pos.v[Z];
									if (s_LineIntersect(x0, z0, x1, z1, xUnit, zUnit, xUnit + NODE_GRID_WIDTH, zUnit)) { Include = TRUE; }
									if (s_LineIntersect(x0, z0, x1, z1, xUnit, zUnit + NODE_GRID_WIDTH, xUnit + NODE_GRID_WIDTH, zUnit + NODE_GRID_WIDTH)) { Include = TRUE; }
									if (s_LineIntersect(x0, z0, x1, z1, xUnit, zUnit, xUnit, zUnit + NODE_GRID_WIDTH)) { Include = TRUE; }
									if (s_LineIntersect(x0, z0, x1, z1, xUnit + NODE_GRID_WIDTH, zUnit, xUnit + NODE_GRID_WIDTH, zUnit + NODE_GRID_WIDTH)) { Include = TRUE; }
								}
							}
						}

						if(!Include)
						{
							for (ii = 0; ii < MAX_AINODE_LINKS; ii++)
							{
								Include = FALSE;
								// This node and each forward node is used to form a polygon, and each grid square vertex is tested against
								// it to see if the two intersect - if they do then the nodes are added.
								if (!Node->Next[ii]) continue;
								if (s_PointInNodePoly(xUnit, zUnit, Node, ii))
								{
									Include = TRUE;
									break;
								}
								if (s_PointInNodePoly(xUnit + NODE_UNIT_SIZE, zUnit, Node, ii))
								{
									Include = TRUE;
									break;
								}
								if (s_PointInNodePoly(xUnit, zUnit + NODE_UNIT_SIZE, Node, ii))
								{
									Include = TRUE;
									break;
								}
								if (s_PointInNodePoly(xUnit + NODE_UNIT_SIZE, zUnit + NODE_UNIT_SIZE, Node, ii))
								{
									Include = TRUE;
									break;
								}
							}
						}
						
						if (Include)
						{
							if (NodeGrid[Grid][xCnt + (zCnt * NODE_GRID_WIDTH)].NodeHead)
							{
 								for (TempNode = NodeGrid[Grid][xCnt + (zCnt * NODE_GRID_WIDTH)].NodeHead; TempNode; TempNode = TempNode->GridNext[Grid])
								{
									if (TempNode == Node)
									{
										Include = FALSE;
									}
								}
							}

							if (Include)
							{
								if (NodeGrid[Grid][xCnt + (zCnt * NODE_GRID_WIDTH)].NodeHead)
								{
									TempNode = NodeGrid[Grid][xCnt + (zCnt * NODE_GRID_WIDTH)].NodeHead;
									NodeGrid[Grid][xCnt + (zCnt * NODE_GRID_WIDTH)].NodeHead = Node;
									Node->GridNext[Grid] = TempNode;
									Node->GridPrev[Grid] = NULL;
									TempNode->GridPrev[Grid] = Node;
								}
								else
								{
									NodeGrid[Grid][xCnt + (zCnt * NODE_GRID_WIDTH)].NodeHead = Node;
									Node->GridPrev[Grid] = NULL;
									Node->GridNext[Grid] = NULL;
								}
							}
						}
					}
				}
			}
		}
	}*/
}

//--------------------------------------------------------------------------------------------------------------------------

// s_LineIntersect
//
// Check if two lines (x0,z0)->(x1,z1) and (x2,z2)->(x3,z3) intersect
//
//     (Ay-Cy)(Dx-Cx)-(Ax-Cx)(Dy-Cy)
// r = -----------------------------  (eqn 1)
//     (Bx-Ax)(Dy-Cy)-(By-Ay)(Dx-Cx)
//
//     (Ay-Cy)(Bx-Ax)-(Ax-Cx)(By-Ay)
// s = -----------------------------  (eqn 2)
//     (Bx-Ax)(Dy-Cy)-(By-Ay)(Dx-Cx)
//
// If 0<=r<=1 & 0<=s<=1, intersection exists


long s_LineIntersect(REAL x0, REAL z0, REAL x1, REAL z1, REAL x2, REAL z2, REAL x3, REAL z3)
{
	REAL	ieq1, ieq2;
	REAL	ieqdiv;

	ieqdiv = ((x1 - x0) * (z3 - z2)) - ((z1 - z0) * (x3 - x2));
	ieq1   = (((z0 - z2) * (x3 - x2)) - ((x0 - x2) * (z3 - z2))) / ieqdiv;
	ieq2   = (((z0 - z2) * (x1 - x0)) - ((x0 - x2) * (z1 - z0))) / ieqdiv;

	if (ieq1 < 0) return (FALSE);
	if (ieq1 > 1) return (FALSE);
	if (ieq2 < 0) return (FALSE);
	if (ieq2 > 1) return (FALSE);
	return(TRUE);
}

//--------------------------------------------------------------------------------------------------------------------------

//
// s_PointLine
//
// Determines which 'side' of a vector the specified point resides
//
// result > 0   point is to the left of vector
// result < 0   point is to the right of vector
// result = 0   point is on vector
//

static REAL s_PointLine(REAL x, REAL y, REAL x0, REAL y0, REAL x1, REAL y1)
{
	REAL	result;

	result = ((y - y0) * (x1 - x0)) - ((x - x0) * (y1 - y0));

	return(result);
}

//--------------------------------------------------------------------------------------------------------------------------

//
// s_PointInNodePoly
//
// Determines if the specified point resides within a polygon built forwards from the specified node to the forward node
// specified by Path
//
// Returns TRUE if point is within poly, FALSE otherwise.
//

static long	s_PointInNodePoly(REAL x, REAL y, AINODE *Node, long Path)
{
	// Check first clockwise edge
	if (s_PointLine(x, y,
					Node->Node[1].Pos.v[X], Node->Node[1].Pos.v[Z],
					Node->Next[Path]->Node[1].Pos.v[X], Node->Next[Path]->Node[1].Pos.v[Z]) > 0) return(FALSE);

	// Check second clockwise edge
	if (s_PointLine(x, y,
					Node->Next[Path]->Node[1].Pos.v[X], Node->Next[Path]->Node[1].Pos.v[Z],
					Node->Next[Path]->Node[0].Pos.v[X], Node->Next[Path]->Node[0].Pos.v[Z]) > 0) return(FALSE);

	// Check third clockwise edge
	if (s_PointLine(x, y,
					Node->Next[Path]->Node[0].Pos.v[X], Node->Next[Path]->Node[0].Pos.v[Z],
					Node->Node[0].Pos.v[X], Node->Node[0].Pos.v[Z]) > 0) return(FALSE);

	// Check fouth clockwise edge
	if (s_PointLine(x, y,
					Node->Node[0].Pos.v[X], Node->Node[0].Pos.v[Z],
					Node->Node[1].Pos.v[X], Node->Node[1].Pos.v[Z]) > 0) return(FALSE);

	return(TRUE);
}

//--------------------------------------------------------------------------------------------------------------------------
