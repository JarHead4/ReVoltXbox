//-----------------------------------------------------------------------------
// File: EditCam.h
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef EDITCAM_H
#define EDITCAM_H

// macros

#define MAX_EDIT_CAM_NODES 1024
//#define CAM_LINK_NUM 4

enum {
    EDIT_CAM_NODE_AXIS_XY,
    EDIT_CAM_NODE_AXIS_XZ,
    EDIT_CAM_NODE_AXIS_ZY,
    EDIT_CAM_NODE_AXIS_X,
    EDIT_CAM_NODE_AXIS_Y,
    EDIT_CAM_NODE_AXIS_Z,
};

typedef struct _EDIT_CAM_NODE {
    long Type, ZoomFactor;
    VEC Pos;
    long RailType;
    struct _EDIT_CAM_NODE *Link;
    long ID;
} EDIT_CAM_NODE;

typedef struct {
    long Type;
    long x, y, z, ZoomFactor;
    long Link;
    long UnUsed1;
    long RailType;
    long ID;
} FILE_CAM_NODE;

// prototypes

extern void InitEditCamNodes(void);
extern void KillEditCamNodes(void);
extern void LoadEditCamNodeModels(void);
extern void FreeEditCamNodeModels(void);
extern void LoadEditCamNodes(char *file);
extern void SaveEditCamNodes(char *file);
extern void DrawEditCamNodes(void);
extern void DisplayCamNodeInfo(EDIT_CAM_NODE *node);
extern void EditCamNodes(void);
extern EDIT_CAM_NODE *AllocEditCamNode(void);
extern void FreeEditCamNode(EDIT_CAM_NODE *node);

// globals

extern EDIT_CAM_NODE *CurrentEditCamNode;

#endif // EDITCAM_H

