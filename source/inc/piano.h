
#ifndef PIANO_H
#define PIANO_H

#include "car.h"

// macros

#define PIANO_KEY_NUM 32
#define PIANO_KEY_SET 16

#define PIANO_XPOS 1040.0f
#define PIANO_YPOS -342.5f
#define PIANO_ZPOS -6525.0f

#define PIANO_WIDTH 2285.0f
#define PIANO_DEPTH 250.0f
#define PIANO_KEY_WIDTH (PIANO_WIDTH / (float)PIANO_KEY_NUM)

#define PIANO_TU1 (140.0f / 256.0f)
#define PIANO_TU2 (150.0f / 256.0f)
#define PIANO_TU3 (160.0f / 256.0f)
#define PIANO_TV (210.0f / 256.0f)
#define PIANO_TEX_WIDTH (8.0f / 256.0f)
#define PIANO_TEX_HEIGHT (25.0f / 256.0f)

#define BLACK_PIANO_KEY_NUM 23
#define BLACK_PIANO_PER_WIDTH 0.25f
#define BLACK_PIANO_WIDTH (PIANO_KEY_WIDTH * (BLACK_PIANO_PER_WIDTH * 2.0f))
#define BLACK_PIANO_DEPTH 137.5f
#define BLACK_PIANO_XOFFSET (PIANO_KEY_WIDTH - (PIANO_KEY_WIDTH * BLACK_PIANO_PER_WIDTH))

#define BLACK_PIANO_TU (171.0f / 256.0f)
#define BLACK_PIANO_TV (216.0f / 256.0f)
#define BLACK_PIANO_TEX_WIDTH (4.0f / 256.0f)
#define BLACK_PIANO_TEX_HEIGHT (15.0f / 256.0f)

// prototypes

extern void TriggerPiano(PlayerStruct *player, long flag, long n, VEC *vec);
extern void PlayPianoNote(VEC *pos, long key, long black);

#endif
