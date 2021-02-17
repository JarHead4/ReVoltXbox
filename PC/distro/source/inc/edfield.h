
#ifndef EDFIELD_H
#define EDFIELD_H

// macros

#define MAX_FILE_FIELDS 128

enum {
	FILE_FIELD_TYPE_LINEAR,
	FILE_FIELD_TYPE_ORIENTATION,
	FILE_FIELD_TYPE_VELOCITY,
	FILE_FIELD_TYPE_SPHERICAL,

	FILE_FIELD_TYPE_MAX
};

enum {
	FILE_FIELD_AXIS_XY,
	FILE_FIELD_AXIS_XZ,
	FILE_FIELD_AXIS_ZY,
	FILE_FIELD_AXIS_X,
	FILE_FIELD_AXIS_Y,
	FILE_FIELD_AXIS_Z,
};

typedef struct {
	long	Type;
	VEC	Pos;
	MAT	Matrix;
	float	Size[3];

	VEC	Dir;
	REAL	Mag;
	REAL	Damping;

	REAL	RadStart;
	REAL	RadEnd;
	REAL	GradStart;
	REAL	GradEnd;
} FILE_FIELD;

// prototypes

extern void InitFileFields(void);
extern void KillFileFields(void);
extern void LoadFileFields(char *file);
extern void SaveFileFields(char *file);
extern FILE_FIELD *AllocFileField(void);
extern void FreeFileField(FILE_FIELD *field);
extern void DrawFields(void);
extern void DisplayFieldInfo(FILE_FIELD *field);
extern void EditFields(void);
extern void LoadFileFieldModels(void);
extern void FreeFileFieldModels(void);

// globals

extern FILE_FIELD *CurrentField;

#endif
