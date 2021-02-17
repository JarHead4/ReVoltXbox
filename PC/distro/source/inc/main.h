
#ifndef MAIN_H
#define MAIN_H

// defines

#define MAX_PLAYER_NAME 64
#define MAX_CHEAT_STRING_BUFFER 16

#define GHOST_TAKEOVER 1
#define RECORD_AVI 0
#define CHECK_IP 0
#define CHECK_ZZZ 0

#define SHOW_PHYSICS_INFO	TRUE

#define MSCOMPILER_FUDGE_OPTIMISATIONS TRUE

#define MINUTES(_t) ((_t) / 60000)
#define SECONDS(_t) (((_t) / 1000) % 60)
#define THOUSANDTHS(_t) ((_t) % 1000)

// edit modes

enum {
	EDIT_NONE,
	EDIT_LIGHTS,
	EDIT_VISIBOXES,
	EDIT_OBJECTS,
	EDIT_INSTANCES,
	EDIT_AINODES,
	EDIT_ZONES,
	EDIT_TRIGGERS,
	EDIT_CAM,
	EDIT_FIELDS,
	EDIT_PORTALS,

	EDIT_NUM
};

// typedefs and structures

typedef struct {
	long GameType, Level, LevelNum;
	unsigned long Reversed, Mirrored;
	unsigned long AutoBrake, CarID, Paws;
} GAME_SETTINGS;

typedef struct {
	REAL GeomPers;
	REAL GeomCentreX;
	REAL GeomCentreY;
	REAL GeomScaleX;
	REAL GeomScaleY;
	REAL MatScaleX;
	REAL MatScaleY;
	REAL NearClip;
	REAL FarClip;
	REAL DrawDist;
	REAL FarDivDist;
	REAL FarMulNear;
	REAL FogStart;
	REAL FogDist;
	REAL FogMul;
	REAL VertFogStart;
	REAL VertFogEnd;
	REAL VertFogDist;
	REAL VertFogMul;
	long Env;
	long Mirror;
	long Shadow;
	long Light;
	long Instance;
	long Skid;
} RENDER_SETTINGS;

// prototypes

extern LRESULT CALLBACK WindowFunc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
extern bool InitWin(HINSTANCE hThisInst, int nWinMode);
extern long Box(char *title, char *mess, long flag);
extern void Vblank(char count);
extern void GameLoop(void);
extern void Go(void);
extern void GoFront(void);
extern void SetupGame(void);
extern void InitPlayersTrial(void);
extern void InitPlayersSingle(void);
extern void InitPlayersNetwork(void);
extern void InitPlayeresFullArray(void);
extern void CheckCheatStrings(void);

// globals

extern char Everything;
extern char NoGamma;
extern char AppRestore;
extern char QuitGame;
extern char FullScreen;
extern unsigned long FrameCount, FrameCountLast, FrameTime, FrameTimeLast, FrameRate;
extern char DetailMenuTogg;
extern long EditMode;
extern REAL TimeFactor;
extern REAL TimeStep;
extern REAL EditScale;
extern HWND hwnd;
extern HBITMAP TitleHbm;
extern void (*Event)(void);
extern GAME_SETTINGS 	GameSettings;
extern RENDER_SETTINGS	RenderSettings;
extern int __argc;
extern char **__argv;

#if SHOW_PHYSICS_INFO
#include "draw.h"
extern long DEBUG_CollGrid;
extern int DEBUG_NCols;
extern int DEBUG_LastNCols;
extern int DEBUG_N2Cols;
extern VEC DEBUG_dR;
extern VEC DEBUG_Impulse;
extern VEC DEBUG_AngImpulse;
extern VEC DEBUG_SNorm[256];
extern FACING_POLY DEBUG_Faces[256];
#endif

#endif
