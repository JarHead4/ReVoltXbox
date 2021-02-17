//-----------------------------------------------------------------------------
// File: visibox.h
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef VISIBOX_H
#define VISIBOX_H

#ifdef _N64
#include "model.h"
#endif
#include "world.h"

// macros

#ifdef _N64
#define VISIBOX_MAX 600
#else
#define VISIBOX_MAX 600
#endif
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

#ifdef _PC

extern void LoadVisiBoxes(char *file);
extern void SaveVisiBoxes(char *file);
extern void EditVisiBoxes(void);
extern void DisplayVisiBoxInfo(VISIBOX *visibox);
extern void DisplayCamVisiMask(void);
extern void DrawVisiBoxes(void);
extern VISIBOX *AllocVisiBox(void);
extern void FreeVisiBox(VISIBOX *visibox);

#elif defined(_N64)

void  LoadVisiBoxes(FIL_ID file);
void  DestroyVisiBoxes();

#endif
extern void InitVisiBoxes(void);
extern void SetPermVisiBoxes(void);
extern VISIMASK SetObjectVisiMask(BOUNDING_BOX *box);
extern char TestObjectVisiboxes(BOUNDING_BOX *box);
extern void SetCameraVisiMask(VEC *pos);

// globals

extern VISIMASK CamVisiMask;
extern long CubeVisiBoxCount, VisiPerPoly;
extern VISIBOX *CurrentVisiBox;

#ifdef _PC
extern PERM_VISIBOX CubeVisiBox[VISIBOX_MAX];
#endif


#endif // VISIBOX_H

