
#ifndef EDITAI_H
#define EDITAI_H

#include "geom.h"
#include "car.h"
#include "ainode.h"
#include "model.h"

// macros

enum {
	EDIT_AINODE_AXIS_XY,
	EDIT_AINODE_AXIS_XZ,
	EDIT_AINODE_AXIS_ZY,
	EDIT_AINODE_AXIS_X,
	EDIT_AINODE_AXIS_Y,
	EDIT_AINODE_AXIS_Z,
};

typedef struct {
	char Priority, StartNode, pad[2];
	float RacingLine, FinishDist, fpad[2];
	long RacingLineSpeed, CentreSpeed;
	long Prev[MAX_AINODE_LINKS];
	long Next[MAX_AINODE_LINKS];
	ONE_AINODE Node[2];
} FILE_AINODE;

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

// globals

extern AINODE *CurrentEditAiNode;
extern AINODE *LastEditAiNode;
extern long    EditAiNodeNum;
extern AINODE *EditAiNode;
extern MODEL EditAiNodeModel[2];

#endif
