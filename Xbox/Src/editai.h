//-----------------------------------------------------------------------------
// File: EditAI.h
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef EDITAI_H
#define EDITAI_H

#include "geom.h"
#include "car.h"
#include "ainode.h"
#include "model.h"


#define GHOST_PATH_MAX_TIME             (1000*60*6)
#define GHOST_PATH_SAMPLE_INTERVAL      (50)
#define GHOST_PATH_MAX_SAMPLES          (GHOST_PATH_MAX_TIME/GHOST_PATH_SAMPLE_INTERVAL)


// macros

enum {
    EDIT_AINODE_AXIS_XY,
    EDIT_AINODE_AXIS_XZ,
    EDIT_AINODE_AXIS_ZY,
    EDIT_AINODE_AXIS_X,
    EDIT_AINODE_AXIS_Y,
    EDIT_AINODE_AXIS_Z,
};

// Used for left & right points of node
typedef struct
{
    long    Speed;
    VEC     Pos;                                    // Position on node edge

} FILE_ONE_AINODE;

typedef struct {
    char Priority, StartNode;
    char flags[2];
    REAL RacingLine, FinishDist, OvertakingLine, fpad;
    long RacingLineSpeed, CentreSpeed;
    long Prev[MAX_AINODE_LINKS];
    long Next[MAX_AINODE_LINKS];
    FILE_ONE_AINODE Node[2];
} FILE_AINODE;

// Node racing lines save structure
typedef struct s_RacingLineUndo
{
    REAL    racingLine;
    REAL    overtakingLine;

} t_RacingLineUndo;


// prototypes

extern void InitEditAiNodes(void);
extern void KillEditAiNodes(void);
extern void LoadEditAiNodes(char *file);
extern void CalcEditAiNodeDistances(void);
extern void CalcOneNodeDistance(AINODE *node, long flag);
extern void SaveEditAiNodes(char *file);
extern void LoadEditAiNodeModels(void);
extern void FreeEditAiNodeModels(void);
extern AINODE *AllocEditAiNode(void);
extern void FreeEditAiNode(AINODE *node);
extern void EditAiNodes(void);
extern void DrawAiNodes(void);
extern void DisplayAiNodeInfo(AINODE *node);
extern void GetEditNodePos(VEC *campos, float xpos, float ypos, VEC *nodepos);
extern AINODE *PickupScreenNode(REAL pickerX, REAL pickerY);

extern void EditRacingLine(void);
extern int CheckNodePath(AINODE *pNodeS, AINODE *pNodeM, AINODE *pNodeE);
extern void SaveRacingLine(void);
extern void RestoreRacingLine(void);
extern void CreateRacingLine(void);
extern REAL CalcNodeIntersectionTime(AINODE* pNode, VEC *pS, VEC *pE);
extern void DrawEditRacingLine(void);

extern void EAI_CreateGhostPath(struct PlayerStruct *pPlayer);
extern void EAI_RenderGhostPath(void);

extern bool CreateRacingLineFromGhostData(void);


// globals

extern AINODE *CurrentEditAiNode;
extern AINODE *LastEditAiNode;
extern short EditAiNodeNum;
extern short EditAiLinkNodeNum;
extern AINODE *EditAiNode;
extern MODEL EditAiNodeModel[2];

#endif // EDITAI_H

