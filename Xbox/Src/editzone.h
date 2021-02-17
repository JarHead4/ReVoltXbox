//-----------------------------------------------------------------------------
// File: EditZone.h
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef EDITZONE_H
#define EDITZONE_H

// macros

#define MAX_FILE_ZONES 128

enum {
    FILE_ZONE_AXIS_XY,
    FILE_ZONE_AXIS_XZ,
    FILE_ZONE_AXIS_ZY,
    FILE_ZONE_AXIS_X,
    FILE_ZONE_AXIS_Y,
    FILE_ZONE_AXIS_Z,
};

typedef struct {
    long ID;
    VEC Pos;
    MAT Matrix;
    REAL Size[3];
} FILE_ZONE;

// prototypes

extern void InitFileZones(void);
extern void KillFileZones(void);
extern void LoadFileZones(char *file);
extern void SaveFileZones(char *file);
extern FILE_ZONE *AllocFileZone(void);
extern void EditFileZones(void);
extern void DrawFileZones(void);
extern void DisplayZoneInfo(FILE_ZONE *zone);
extern void DisplayCurrentTrackZone(void);

// globals

extern FILE_ZONE *CurrentFileZone;

#endif // EDITZONE_H

