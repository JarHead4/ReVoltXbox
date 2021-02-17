//-----------------------------------------------------------------------------
// File: visibox.h
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef VISIBOX_H
#define VISIBOX_H

#include "world.h"

// macros

#define VISIBOX_MAX 600
#define VISIBOX_MAX_ID 64

#define VISIBOX_CAMERA 1
#define VISIBOX_CUBE 2

enum {
    VISI_AXIS_XY,
    VISI_AXIS_XZ,
    VISI_AXIS_ZY,
    VISI_AXIS_X,
    VISI_AXIS_Y,
    VISI_AXIS_Z,
};

typedef struct {
    char Flag, ID, pad[2];
    float xmin, xmax;
    float ymin, ymax;
    float zmin, zmax;
} VISIBOX;

typedef struct {
    VISIMASK Mask;
    long ID;
    float xmin, xmax;
    float ymin, ymax;
    float zmin, zmax;
} PERM_VISIBOX;

typedef struct {
    long Count;
    PERM_VISIBOX *VisiBoxes;
} PERM_VISIBOX_HEADER;

// prototypes

extern void LoadVisiBoxes(char *file);
extern void SaveVisiBoxes(char *file);
extern VISIBOX *AllocVisiBox(void);
extern void FreeVisiBox(VISIBOX *visibox);

extern void InitVisiBoxes(void);
extern void SetPermVisiBoxes(void);
extern VISIMASK SetObjectVisiMask(BOUNDING_BOX *box);
extern char TestObjectVisiboxes(BOUNDING_BOX *box);
extern void SetCameraVisiMask(VEC *pos);

// globals

extern VISIMASK CamVisiMask;
extern long CubeVisiBoxCount, VisiPerPoly;
extern VISIBOX *CurrentVisiBox;

extern PERM_VISIBOX CubeVisiBox[VISIBOX_MAX];


#endif // VISIBOX_H

