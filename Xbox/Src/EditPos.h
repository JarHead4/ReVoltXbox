//-----------------------------------------------------------------------------
// File: EditPos.h
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef EDITPOS_H
#define EDITPOS_H

#include "posnode.h"

// prototypes

extern void InitEditPosNodes(void);
extern void FreeEditPosNodes(void);
extern void LoadEditPosNodes(char *file);
extern void CalcEditPosNodeDistances(void);
extern void CalcEditPosNodeDistances(void);
extern void CalcOnePosNodeDistance(POSNODE *node, long flag);
extern void SaveEditPosNodes(char *file);
extern void LoadEditPosNodeModels(void);
extern void FreeEditPosNodeModels(void);
extern POSNODE *AllocEditPosNode(void);
extern void FreeEditPosNode(POSNODE *node);
extern void EditPosNodes(void);
extern void DrawPosNodes(void);
extern void GetPosNodePos(VEC *campos, float xpos, float ypos, VEC *nodepos);
extern void DisplayCurrentFinishDist(void);

#endif // EDITPOS_H

