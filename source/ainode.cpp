
#include "revolt.h"
#include "editai.h"
#include "car.h"
#include "aizone.h"
#include "ctrlread.h"
#include "object.h"
#include "Control.h"
#include "player.h"
#ifdef _N64
#include "ffs_code.h"
#include "ffs_list.h"
#include "utils.h"
#endif

//
// Global varaibles
//

long AiNodeNum;
AINODE *AiNode;
AINODE_ZONE *AiNodeZone;

//
// Global function declarations
//

#ifdef _PC
void LoadAiNodes(char *file);
#endif
#ifdef _N64
void LoadAiNodes(void);
#endif
void FreeAiNodes(void);
AINODE *AIN_NearestNode(PLAYER *Player, REAL *Dist);
AINODE *AIN_GetForwardNode(PLAYER *Player, REAL MinDist, REAL *Dist);
long AiStartNode;
REAL AiNodeTotalDist;

///////////////////
// load ai nodes //
///////////////////
#ifdef _PC
void LoadAiNodes(char *file)
{
	long 		i, j;
	FILE 		*fp;
	FILE_AINODE fan;

// open ai node file

	AiNode = NULL;
	AiNodeNum = 0;

	fp = fopen(file, "rb");
	if (!fp) return;

// alloc ram

	fread(&AiNodeNum, sizeof(AiNodeNum), 1, fp);
	if (!AiNodeNum) return;

	AiNode = (AINODE*)malloc(sizeof(AINODE) * AiNodeNum);
	if (!AiNode)
	{
		AiNodeNum = 0;
		return;
	}

// loop thru all ai nodes

	for (i = 0 ; i < AiNodeNum ; i++)
	{

// load one file ai node

		fread(&fan, sizeof(fan), 1, fp);

// setup ai node

		AiNode[i].Node[0] = fan.Node[0];
		AiNode[i].Node[1] = fan.Node[1];

		AiNode[i].Centre.v[X] = (fan.Node[0].Pos.v[X] + fan.Node[1].Pos.v[X]) / 2;
		AiNode[i].Centre.v[Y] = (fan.Node[0].Pos.v[Y] + fan.Node[1].Pos.v[Y]) / 2;
		AiNode[i].Centre.v[Z] = (fan.Node[0].Pos.v[Z] + fan.Node[1].Pos.v[Z]) / 2;
		AiNode[i].RVec.v[X] = fan.Node[1].Pos.v[X] - AiNode[i].Centre.v[X];
		AiNode[i].RVec.v[Y] = fan.Node[1].Pos.v[Y] - AiNode[i].Centre.v[Y];
		AiNode[i].RVec.v[Z] = fan.Node[1].Pos.v[Z] - AiNode[i].Centre.v[Z];
		NormalizeVector(&AiNode[i].RVec);

		AiNode[i].Priority = fan.Priority;
		AiNode[i].StartNode = fan.StartNode;
		AiNode[i].RacingLine = fan.RacingLine;
		AiNode[i].RacingLineSpeed = fan.RacingLineSpeed;
		AiNode[i].CentreSpeed = fan.CentreSpeed;
		AiNode[i].FinishDist = fan.FinishDist;

		for (j = 0 ; j < MAX_AINODE_LINKS ; j++)
		{
			if (fan.Prev[j] != -1)
				AiNode[i].Prev[j] = AiNode + fan.Prev[j];
			else
				AiNode[i].Prev[j] = NULL;

			if (fan.Next[j] != -1)
				AiNode[i].Next[j] = AiNode + fan.Next[j];
			else
				AiNode[i].Next[j] = NULL;
		}
	}

// load start node

	fread(&AiStartNode, sizeof(AiStartNode), 1, fp);

// load total dist

	fread(&AiNodeTotalDist, sizeof(AiNodeTotalDist), 1, fp);

// close ai node file

	fclose(fp);
}

#endif

#ifdef _N64
void LoadAiNodes(void)
{
	long			ii, jj, kk, numread;
	FIL				*fp;
	FILE_AINODE		fan;
	long 			Prev, Next;

// open ai node file

	AiNode = NULL;
	AiNodeNum = 0;

	printf("Loading AI nodes...\n");
	fp = FFS_Open(FFS_TYPE_TRACK | TRK_AINODES);
	if (!fp)
	{
		printf("...AI node file not found!\n");
		return;
	}

// alloc ram

	FFS_Read(&AiNodeNum, sizeof(AiNodeNum), fp);
	AiNodeNum = EndConvLong(AiNodeNum);
	if (!AiNodeNum) return;

	AiNode = (AINODE*)malloc(sizeof(AINODE) * AiNodeNum);
	if (!AiNode)
	{
		AiNodeNum = 0;
		return;
	}

// loop thru all ai nodes

	for (ii = 0; ii < AiNodeNum ; ii++)
	{

// load one file ai node
		FFS_Read(&fan, sizeof(fan), fp);
		fan.RacingLine = EndConvReal(fan.RacingLine);
		fan.RacingLineSpeed = EndConvLong(fan.RacingLineSpeed);
		fan.CentreSpeed = EndConvLong(fan.CentreSpeed);
		fan.FinishDist = EndConvReal(fan.FinishDist);

		for (jj = 0; jj < MAX_AINODE_LINKS; jj++)
		{
			fan.Prev[jj] = EndConvLong(fan.Prev[jj]);
			fan.Next[jj] = EndConvLong(fan.Next[jj]);
		}
		fan.Node[0].Speed = EndConvLong(fan.Node[0].Speed);
		fan.Node[0].Pos.v[0] = EndConvReal(fan.Node[0].Pos.v[0]);
		fan.Node[0].Pos.v[1] = EndConvReal(fan.Node[0].Pos.v[1]);
		fan.Node[0].Pos.v[2] = EndConvReal(fan.Node[0].Pos.v[2]);
		fan.Node[1].Speed = EndConvLong(fan.Node[1].Speed);
		fan.Node[1].Pos.v[0] = EndConvReal(fan.Node[1].Pos.v[0]);
		fan.Node[1].Pos.v[1] = EndConvReal(fan.Node[1].Pos.v[1]);
		fan.Node[1].Pos.v[2] = EndConvReal(fan.Node[1].Pos.v[2]);

// setup ai node
		AiNode[ii].Node[0] = fan.Node[0];
		AiNode[ii].Node[1] = fan.Node[1];

		AiNode[ii].Centre.v[X] = (fan.Node[0].Pos.v[X] + fan.Node[1].Pos.v[X]) / 2;
		AiNode[ii].Centre.v[Y] = (fan.Node[0].Pos.v[Y] + fan.Node[1].Pos.v[Y]) / 2;
		AiNode[ii].Centre.v[Z] = (fan.Node[0].Pos.v[Z] + fan.Node[1].Pos.v[Z]) / 2;
		AiNode[ii].RVec.v[X] = fan.Node[1].Pos.v[X] - AiNode[ii].Centre.v[X];
		AiNode[ii].RVec.v[Y] = fan.Node[1].Pos.v[Y] - AiNode[ii].Centre.v[Y];
		AiNode[ii].RVec.v[Z] = fan.Node[1].Pos.v[Z] - AiNode[ii].Centre.v[Z];
		NormalizeVector(&AiNode[ii].RVec);

		AiNode[ii].Priority = fan.Priority;
		AiNode[ii].StartNode = fan.StartNode;
		AiNode[ii].RacingLine = fan.RacingLine;
		AiNode[ii].RacingLineSpeed = fan.RacingLineSpeed;
		AiNode[ii].CentreSpeed = fan.CentreSpeed;
		AiNode[ii].FinishDist = fan.FinishDist;

		for (jj = 0; jj < MAX_AINODE_LINKS; jj++)
		{
			if (fan.Prev[jj] != -1)
				AiNode[ii].Prev[jj] = AiNode + fan.Prev[jj];
			else
				AiNode[ii].Prev[jj] = NULL;

			if (fan.Next[jj] != -1)
				AiNode[ii].Next[jj] = AiNode + fan.Next[jj];
			else
				AiNode[ii].Next[jj] = NULL;
		}
	}

	printf("...read %d AI nodes.\n", AiNodeNum);

// load start node
	FFS_Read(&AiStartNode, sizeof(AiStartNode), fp);
	AiStartNode = EndConvLong(AiStartNode);

// load total dist
	FFS_Read(&AiNodeTotalDist, sizeof(AiNodeTotalDist), fp);
	AiNodeTotalDist = EndConvReal(AiNodeTotalDist);

// close ai node file
	FFS_Close(fp);
}
#endif

///////////////////
// free ai nodes //
///////////////////

void FreeAiNodes(void)
{
	free(AiNode);
	free(AiNodeZone);
}

//////////////////////
// zone up ai nodes //
//////////////////////

void ZoneAiNodes(void)
{
	long i, j, k;
	REAL dist;
	char buf[128];
	AINODE *lastnode;

// quit if no zones

	if (!AiZoneNum)
	{
		AiNodeZone = NULL;
		return;
	}

// alloc ram

	AiNodeZone = (AINODE_ZONE*)malloc(sizeof(AINODE_ZONE) * AiZoneNum);
	if (!AiNodeZone)
	{
#ifdef _PC
		Box("Warning!", "Couldn't alloc memory for AI node zoning!", MB_OK);
#endif
#ifdef _N64
		printf("<!> WARN: Couldn't alloc memory for AI node zoning.\n");
#endif
		return;
	}

// find each nodes zone ID

	for (i = 0 ; i < AiNodeNum ; i++)
	{
		AiNode[i].ZoneID = LONG_MAX;

		for (j = 0 ; j < AiZoneNum ; j++)
		{
			for (k = 0 ; k < 3 ; k++)
			{
				dist = PlaneDist(&AiZones[j].Plane[k], &AiNode[i].Centre);
				if (dist < -AiZones[j].Size[k] || dist > AiZones[j].Size[k]) break;
			}

			if (k == 3 && j < AiNode[i].ZoneID)
			{
				AiNode[i].ZoneID = j;
			}
		}

		if (AiNode[i].ZoneID == LONG_MAX)
		{
#ifdef _PC
			wsprintf(buf, "AI node %ld is outside all AI zones!", i);
			Box(NULL, buf, MB_OK);
#endif
#ifdef _N64
			printf("<!> WARN: AI node %ld is outside all AI zones.\n", i);
#endif
		}
	}

// setup each zone header

	for (i = 0 ; i < AiZoneNum ; i++)
	{
		AiNodeZone[i].Count = 0;
		AiNodeZone[i].FirstNode = NULL;

		for (j = 0 ; j < AiNodeNum ; j++)
		{
			if (AiNode[j].ZoneID == i)
			{
				if (!AiNodeZone[i].Count)
				{
					AiNodeZone[i].FirstNode = &AiNode[j];
					AiNode[j].ZonePrev = NULL;
				}
				else
				{
					lastnode->ZoneNext = &AiNode[j];
					AiNode[j].ZonePrev = lastnode;
				}

				AiNode[j].ZoneNext = NULL;
				AiNodeZone[i].Count++;
				lastnode = &AiNode[j];
			}
		}
	}
}

//--------------------------------------------------------------------------------------------------------------------------

//
// AIN_NearestNode
//
// Locates the nearest node to the specified car, biased towards. The car's grid position is established and all of the nodes in that grid
// location are scanned against the car centre to find the nearest. 
//

AINODE *AIN_NearestNode(PLAYER *Player, REAL *Dist)
{
	AINODE		*NearNode = NULL;
/*	long		Grid;
	VEC			CarPos;
	VEC			temp;
	long   		xShift, zShift;
	long		xOff, zOff;
	AINODE		*Node;
	REAL		tDist, Nearest;
	CAR			*Car;

	Car = &Player->car;
	Nearest = 65535 * 2;
	CarPos = Car->Body->Centre.Pos;
	for (zShift = 0; zShift < MAX_SHIFTS; zShift++)
	{
		zOff = (long) ((REAL)((CarPos.v[Z] + 32768) - (zShift * NODE_SHIFT_OFFSET)) / NODE_UNIT_SIZE);
		for (xShift = 0; xShift < MAX_SHIFTS; xShift++)
		{
			xOff = (long) ((REAL)((CarPos.v[X] + 32768) - (xShift * NODE_SHIFT_OFFSET)) / NODE_UNIT_SIZE);
			Grid = xShift + (zShift * MAX_SHIFTS);

			if (NodeGrid[Grid][xOff + (zOff * NODE_GRID_WIDTH)].NodeHead)
			{
				for (Node = NodeGrid[Grid][xOff + (zOff * NODE_GRID_WIDTH)].NodeHead ; Node; Node = Node->GridNext[Grid])
				{
					temp.v[X] = Node->Centre.v[X] - CarPos.v[X];
					temp.v[Y] = Node->Centre.v[Y] - CarPos.v[Y];
					temp.v[Z] = Node->Centre.v[Z] - CarPos.v[Z];

					tDist = abs(VecLen(&temp));

					if (tDist < Nearest)
					{
						Nearest = tDist;
						NearNode = Node;
					}
				}
			}
		}
	}
	
	*Dist = Nearest;*/
	return(NearNode);
}

//--------------------------------------------------------------------------------------------------------------------------

//
// AIN_GetForwardNode
//
// Returns node "in front of" the specified car's current position at a distance greater than MinDist
//

AINODE *AIN_GetForwardNode(PLAYER *Player, REAL MinDist, REAL *Dist)
{
	AINODE	*NearNode;
//	AINODE NearNodeFwd;
	REAL	Nearest, NearestFwd; 
	REAL	dp;
	long	RChoice;
	VEC		Vec1, Vec2;
	VEC		CarPos;

	NearNode = AIN_NearestNode(Player, &Nearest);
	if (!NearNode) return (NULL);
	CarPos = Player->car.Body->Centre.Pos;
	RChoice = Player->CarAI.RouteChoice;
	
	NearestFwd = -1;

	while(NearestFwd < MinDist)
	{
		if (!NearNode->Next[RChoice])
		{
			*Dist = Nearest;
			return(NearNode);
		}
		Vec1.v[X] = NearNode->Centre.v[X] - CarPos.v[X];
		Vec1.v[Y] = NearNode->Centre.v[Y] - CarPos.v[Y];
		Vec1.v[Z] = NearNode->Centre.v[Z] - CarPos.v[Z];
		Vec2.v[X] = NearNode->Next[RChoice]->Centre.v[X] - NearNode->Centre.v[X];
		Vec2.v[Y] = NearNode->Next[RChoice]->Centre.v[Y] - NearNode->Centre.v[Y];
		Vec2.v[Z] =	NearNode->Next[RChoice]->Centre.v[Z] - NearNode->Centre.v[Z];
	
		dp = DotProduct(&Vec1, &Vec2);
		if (dp > 0.5) 
		{
			Vec2.v[X] = NearNode->Next[RChoice]->Centre.v[X] - CarPos.v[X];
			Vec2.v[Y] = NearNode->Next[RChoice]->Centre.v[Y] - CarPos.v[Y];
			Vec2.v[Z] =	NearNode->Next[RChoice]->Centre.v[Z] - CarPos.v[Z];
			NearestFwd = abs(VecLen(&Vec2)); 
		}
		NearNode = NearNode->Next[RChoice];
	}
	*Dist = NearestFwd;
	return(NearNode->Next[RChoice]);
}


//--------------------------------------------------------------------------------------------------------------------------

long AIN_PriChoice(PLAYER *Player)
{
//	long	Choice;

	return(1);
}

//--------------------------------------------------------------------------------------------------------------------------
