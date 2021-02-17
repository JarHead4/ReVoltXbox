
#ifndef SMOKE_H
#define SMOKE_H

#include "draw.h"

// macros

#define MAX_SMOKES 256
#define RAND_SMOKE_SPINSTART (REAL)rand() / RAND_MAX
#define RAND_SMOKE_SPINRATE (REAL)(rand() - (RAND_MAX >> 1)) / (RAND_MAX * 96)

typedef struct _SMOKE {
	REAL AgeStart, Age;
	REAL Spin, SpinRate;
	REAL GrowRate;
	VEC Pos, Dir;
	MAT Matrix;
	FACING_POLY Poly;
	long rgb;

	struct _SMOKE *Prev;
	struct _SMOKE *Next;
} SMOKE;

// prototypes

extern long InitSmoke(void);
extern void KillSmoke(void);
extern SMOKE *AllocSmoke(void);
extern void FreeSmoke(SMOKE *smoke);
extern long CreateSmoke(VEC *pos, VEC *dir, REAL size, REAL growrate, REAL age, REAL spinstart, REAL spinrate, long rgb);
extern void ProcessSmoke(void);
extern void DrawSmoke(void);

// globals

#endif
