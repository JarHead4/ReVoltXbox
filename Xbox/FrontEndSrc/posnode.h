//-----------------------------------------------------------------------------
// File: posnode.h
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef POSNODE_H
#define POSNODE_H

#include "player.h"

// macros / structures

#define POSNODE_MAX 1024
#define POSNODE_MAX_LINKS 4

typedef struct {
    VEC Pos;
    REAL Dist;
    long Prev[POSNODE_MAX_LINKS];
    long Next[POSNODE_MAX_LINKS];
} FILE_POSNODE;

typedef struct _POSNODE {
    VEC Pos;
    REAL Dist;
    struct _POSNODE *Prev[POSNODE_MAX_LINKS];
    struct _POSNODE *Next[POSNODE_MAX_LINKS];
} POSNODE;

// prototypes

#ifdef _PC
extern void LoadPosNodes(char *file);
#endif
#ifdef _N64
void LoadPosNodes(void);
#endif
extern void FreePosNodes(void);
extern long UpdateCarFinishDist(PLAYER *player, unsigned long *timedelta);

// globals

extern long PosNodeNum, PosStartNode;
extern POSNODE *PosNode;
extern REAL PosTotalDist;


#endif // POSNODE_H

