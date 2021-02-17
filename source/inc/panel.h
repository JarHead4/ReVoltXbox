
#ifndef PANEL_H
#define PANEL_H

#include "car.h"

// macros

#define SPLIT_COUNT 2.0f
#define TRACK_DIR_COUNT 640
#define TRACK_DIR_FADE_COUNT 128
#define WRONG_WAY_TOLERANCE 3.0f
#define REV_LIT_NUM 7
#define REV_LIT_MAX 1024
#define MAX_MAP_DIR_NAME 16

// prototypes

extern void TriggerTrackDir(struct PlayerStruct *player, long flag, long n, VEC *vec);
extern void TriggerSplit(struct PlayerStruct *player, long flag, long n, VEC *vec);
extern void DrawControlPanel(void);
extern void DrawPanelSprite(float x, float y, float width, float height, float tu, float tv, float twidth, float theight, long rgba);

// globals

enum {
	SPEED_MPH,	// Miles Per Hour
	SPEED_FPM,	// Feet Per Minute
	SPEED_KPH,	// Kilometers Per Hour

	SPEED_NTYPES
};

typedef struct {
	char Dir[MAX_MAP_DIR_NAME];
	REAL x, y, xscale, yscale;
	REAL tu, tv, tw, th;
} MAP_INFO;

extern long SpeedUnits;
extern char *SpeedUnitText[];
extern REAL SpeedUnitScale[];



#endif
