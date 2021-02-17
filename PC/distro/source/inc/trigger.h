
#ifndef TRIGGER_H
#define TRIGGER_H

#include "car.h"

// macros

#define TRIGGER_GLOBAL_FIRST 1
#define TRIGGER_FRAME_FIRST 2

typedef struct {
	unsigned long GlobalFirst, FrameStamp;
	long ID, Flag, LocalPlayerOnly;
	float Size[3];
	PLANE Plane[3];
	VEC Vector;
	void (*Function)(struct PlayerStruct *player, long flag, long n, VEC *vec);
} TRIGGER;

typedef struct {
	void (*Func)(struct PlayerStruct *player, long flag, long n, VEC *vec);
	long LocalPlayerOnly;
} TRIGGER_INFO;

enum {
	TRIGGER_PIANO,
	TRIGGER_SPLIT,
	TRIGGER_TRACK_DIR,
#ifdef _N64
	TRIGGER_NUM,
#endif
	TRIGGER_CAMCHANGE,
	TRIGGER_AIHOME,
#ifndef _N64
	TRIGGER_NUM
#endif
};

// prototypes

extern void FreeTriggers(void);
extern void CheckTriggers(void);
extern void ResetTriggerFlags(long ID);
#ifdef _PC
extern void LoadTriggers(char *file);
#endif
#ifdef _N64
extern void LoadTriggers(void);
#endif

// globals

extern long TriggerNum;
extern TRIGGER *Triggers;

#endif
