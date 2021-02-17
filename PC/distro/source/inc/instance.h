#ifndef INSTANCE_H
#define INSTANCE_H

#include "model.h"

// macros

#define MAX_INSTANCES 200
#define MAX_INSTANCE_MODELS 64
#define MAX_INSTANCE_FILENAME 9
#define MAX_INSTANCE_LOD 5

#define INSTANCE_ENV 1
#define INSTANCE_HIDE 2
#define INSTANCE_NO_MIRROR 4
#define INSTANCE_NO_FILE_LIGHTS 8
#define INSTANCE_SET_MODEL_RGB 16

enum {
	INSTANCE_AXIS_XY,
	INSTANCE_AXIS_XZ,
	INSTANCE_AXIS_ZY,
	INSTANCE_AXIS_X,
	INSTANCE_AXIS_Y,
	INSTANCE_AXIS_Z,
};

typedef struct {
	unsigned char Model, Priority, Flag, pad;
	char r, g, b, pad2;
	unsigned long EnvRGB, MirrorFlag;
	float LodBias, MirrorHeight;
	VISIMASK VisiMask;
	VEC WorldPos;
	MAT WorldMatrix;
	POLY_RGB *rgb[MAX_INSTANCE_LOD];
	BOUNDING_BOX Box;
	NEWCOLLPOLY *CollPoly;					// Pointer to first collision poly in world coll poly list
	short NCollPolys;						// Number of collisions polys
} INSTANCE;

typedef struct {
	char Name[MAX_INSTANCE_FILENAME];
	char r, g, b;
	unsigned long EnvRGB;
	unsigned char Priority, Flag, pad[2];
	float LodBias;
	VEC WorldPos;
	MAT WorldMatrix;
} FILE_INSTANCE;

typedef struct {
	long Count;
	char Name[MAX_INSTANCE_FILENAME];
	MODEL Models[MAX_INSTANCE_LOD];
	NEWCOLLPOLY *CollPoly;
	short NCollPolys;
} INSTANCE_MODELS;

// prototypes

extern void LoadInstances(char *file);
extern void SaveInstances(char *file);
extern void EditInstances(void);
extern void DrawInstances(void);
extern void DisplayInstanceInfo(INSTANCE *inst);
extern INSTANCE *AllocInstance(void);
extern void FreeInstance(INSTANCE *inst);
extern void LoadInstanceModels(void);
extern void FreeInstanceModels(void);
extern void FreeInstanceRGBs(void);
extern void FreeOneInstanceRGB(INSTANCE *inst);
extern void AllocOneInstanceRGB(INSTANCE *inst);
extern void SetInstanceBoundingBoxes(INSTANCE *inst);
extern void BuildInstanceCollPolys();

// globals

extern long InstanceNum;
extern INSTANCE *CurrentInstance;
extern INSTANCE Instances[];
extern long InstanceModelNum;
extern INSTANCE_MODELS *InstanceModels;

#endif
