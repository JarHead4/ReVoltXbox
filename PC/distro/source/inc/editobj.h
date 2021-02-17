
#ifndef EDITOBJ_H
#define EDITOBJ_H

#include "geom.h"

// macros

#define MAX_EDIT_OBJECTS 128
#define FILE_OBJECT_FLAG_NUM 4

enum {
	FILE_OBJECT_AXIS_XY,
	FILE_OBJECT_AXIS_XZ,
	FILE_OBJECT_AXIS_ZY,
	FILE_OBJECT_AXIS_X,
	FILE_OBJECT_AXIS_Y,
	FILE_OBJECT_AXIS_Z,
};

typedef struct _EDIT_OBJECT {
	long ID;
	long Flag[FILE_OBJECT_FLAG_NUM];

	VEC Pos;
	MAT Mat;

	struct _EDIT_OBJECT *Prev;
	struct _EDIT_OBJECT *Next;
} EDIT_OBJECT;

typedef struct {
	long ID;
	long Flag[FILE_OBJECT_FLAG_NUM];

	VEC Pos;
	VEC Up;
	VEC Look;
} FILE_OBJECT;

typedef struct {
	char *Name;
	char **Type;
	long Min;
	long Max;
} FLAG_INFO;

typedef struct {
	long ModelID;
	char *ObjName;
	FLAG_INFO FlagInfo[FILE_OBJECT_FLAG_NUM];
} FILE_OBJECT_INFO;

// prototypes

extern long InitFileObjects(void);
extern void KillFileObjects(void);
extern EDIT_OBJECT *AllocFileobject(void);
extern void FreeFileobject(EDIT_OBJECT *obj);
extern void LoadFileObjects(char *file);
extern void SaveFileObjects(char *file);
extern void EditFileObjects(void);
extern void DrawFileObjects(void);
extern void DisplayFileObjectInfo(EDIT_OBJECT *obj);
extern void LoadFileObjectModels(void);
extern void FreeFileObjectModels(void);

// globals

extern EDIT_OBJECT *CurrentFileObject;
extern char *FileObjectModelList[];
extern FILE_OBJECT_INFO FileObjectInfo[];

#endif
