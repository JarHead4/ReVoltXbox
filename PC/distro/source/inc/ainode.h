
#ifndef AINODE_H
#define AINODE_H

#include "grid.h"

//
// Defines and macros
//

#define MAX_AINODES 1024
#define MAX_AINODE_LINKS 2
#define MAX_AINODE_SPEED 100
#define MAX_AINODE_PRIORITY 1


//
// Typedefs and structures
//

typedef struct {
	long Speed;
	VEC Pos;
} ONE_AINODE;


typedef struct _AINODE {
	char Priority, StartNode, pad[2];
	REAL RacingLine, FinishDist;
	long RacingLineSpeed, CentreSpeed;
	struct _AINODE *Prev[MAX_AINODE_LINKS];
	struct _AINODE *Next[MAX_AINODE_LINKS];
	ONE_AINODE Node[2];

	long ZoneID;
	VEC	Centre, RVec;

	struct _AINODE *ZonePrev;
	struct _AINODE *ZoneNext;
} AINODE;

typedef struct {
	long Count;
	AINODE *FirstNode;
} AINODE_ZONE;

//
// External variables
//

extern AINODE *AiNode;
extern AINODE_ZONE *AiNodeZone;
extern long		AiNodeNum;
extern long AiStartNode;
extern REAL AiNodeTotalDist;

//
// External function prototypes
//

#ifdef _PC
extern void		LoadAiNodes(char *file);
#endif
#ifdef _N64
extern void		LoadAiNodes(void);
#endif
extern void		FreeAiNodes(void);
extern void ZoneAiNodes(void);
extern AINODE *AIN_NearestNode(struct PlayerStruct *Player, REAL *Dist);
extern AINODE *AIN_GetForwardNode(struct PlayerStruct *Player, REAL MinDist, REAL *Dist);

#endif
