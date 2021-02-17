
#ifndef SFX_H
#define SFX_H

#include "mss.h"

// macros

#define SFX_NUM_CHANNELS 2
#define SFX_SAMPLE_RATE 22050
#define SFX_BITS_PER_SAMPLE 16

#define SFX_MAX_LOAD 256
#define SFX_MAX_SAMPLES 16
#define SFX_MAX_SAMPLES_3D 64

#define SFX_MIN_VOL 0
#define SFX_MAX_VOL 127
#define SFX_LEFT_PAN 0
#define SFX_CENTRE_PAN 64
#define SFX_RIGHT_PAN 127

#define SFX_3D_PAN_MUL 64
#define SFX_3D_MIN_DIST 600
#define SFX_3D_SUB_DIST (8.0f / (float)SFX_MAX_VOL)
#define SFX_3D_SOS 1024

typedef struct {
	HSAMPLE Handle;
} SAMPLE_SFX;

typedef struct {
	void *Pos;
	long Size;
} SFX_LOAD;

typedef struct {
	long Alive, Num, Vol, Freq, Loop;
	float LastDist;
	SAMPLE_SFX *Sample;
	VEC Pos;
} SAMPLE_3D;

typedef struct {
	char *Name;
	char **Files;
} LEVEL_SFX;

// prototypes

extern long InitSound(void);
extern void ReleaseSound(void);
extern long LoadSfx(char *levelname);
extern void FreeSfx(void);
extern void PlaySfx(long num, long vol, long pan, long freq);
extern void StopSfx(SAMPLE_SFX *sample);
extern void PauseAllSfx();
extern void ResumeAllSfx();
extern void PlaySfx3D(long num, long vol, long freq, VEC *pos);
extern SAMPLE_3D *CreateSfx3D(long num, long vol, long freq, long loop, VEC *pos);
extern void FreeSfx3D(SAMPLE_3D *sample3d);
extern void GetSfxSettings3D(long *vol, long *freq, long *pan, VEC *pos, float vel);
extern void ChangeSfxSample3D(SAMPLE_3D *sample3d, long sfx);
extern void MaintainAllSfx(void);
extern void PlayMP3(char *file);
extern void StopMP3();

// globals

extern long SoundOff;

// generic sfx list

enum {
	SFX_ENGINE,
	SFX_HONK,
	SFX_SCRAPE1,
	SFX_SCREECH,
	SFX_RECORD,
	SFX_PICKUP,
	SFX_PICKUP_CLONE,
	SFX_SHOCKWAVE,
	SFX_ELECTROPULSE,
	SFX_FIREWORK,
	SFX_FIREWORK_BANG,
	SFX_CHROMEBALL,
	SFX_CHROMEBALL_HIT,
	SFX_WATERBOMB,
	SFX_WATERBOMB_HIT,
	SFX_PUTTYBOMB_BANG,
	SFX_FUSE,

	SFX_GENERIC_NUM
};

// toy sfx

enum {
	SFX_TOY_PIANO = SFX_GENERIC_NUM,
	SFX_TOY_PLANE,
	SFX_TOY_COPTER,
	SFX_TOY_DRAGON,
	SFX_TOY_CREAK,
	SFX_TOY_TRAIN,
	SFX_TOY_WHISTLE,
};

#endif
