//-----------------------------------------------------------------------------
// File: shadow.h
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef SHADOW_H
#define SHADOW_H

// macros

typedef struct {
    VEC Pos;
    float tu, tv;
} SHADOW_VERT;

// prototypes

extern void DrawShadow(VEC *p0, VEC *p1, VEC *p2, VEC *p3, REAL tu, REAL tv, REAL twidth, REAL theight, long rgb, REAL yoff, REAL maxy, long semi, long tpage, BOUNDING_BOX *box);
extern void ClipShadowEdge(SHADOW_VERT *sv0, SHADOW_VERT *sv1, float mul, SHADOW_VERT *svout);

#endif // SHADOW_H

