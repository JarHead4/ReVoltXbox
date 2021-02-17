//-----------------------------------------------------------------------------
// File: EditTrigger.h
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef EDITTRIGGER_H
#define EDITTRIGGER_H

// macros

#define MAX_FILE_TRIGGERS 64

enum {
    FILE_TRIGGER_AXIS_XY,
    FILE_TRIGGER_AXIS_XZ,
    FILE_TRIGGER_AXIS_ZY,
    FILE_TRIGGER_AXIS_X,
    FILE_TRIGGER_AXIS_Y,
    FILE_TRIGGER_AXIS_Z,
};

typedef struct {
    long ID, Flag;
    VEC Pos;
    MAT Matrix;
    float Size[3];
} FILE_TRIGGER;

// prototypes

extern void InitFileTriggers(void);
extern void KillFileTriggers(void);
extern void LoadFileTriggers(char *file);
extern void SaveFileTriggers(char *file);
extern FILE_TRIGGER *AllocFileTrigger(void);
extern void FreeFileTrigger(FILE_TRIGGER *zone);
extern void DrawTriggers(void);
extern void DisplayTriggerInfo(FILE_TRIGGER *trigger);
extern void EditTriggers(void);

// globals

extern FILE_TRIGGER *CurrentTrigger;

#endif // EDITTRIGGER_H
