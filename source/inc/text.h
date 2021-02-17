
#ifndef TEXT_H
#define TEXT_H

// macros

#define FONT_WIDTH 13.0f
#define FONT_HEIGHT 17.0f
#define FONT_UWIDTH 12.0f
#define FONT_VHEIGHT 16.0f
#define FONT_PER_ROW 19

#define BIG_FONT_WIDTH 24.0f
#define BIG_FONT_HEIGHT 32.0f
#define BIG_FONT_UWIDTH 24.0f
#define BIG_FONT_VHEIGHT 32.0f
#define BIG_FONT_PER_ROW 10

enum MAIN_MENU_OPTIONS {
	MAIN_MENU_SINGLE,
	MAIN_MENU_AI_TEST,
	MAIN_MENU_MULTI,
	MAIN_MENU_JOIN,
	MAIN_MENU_TRACK,
	MAIN_MENU_RES,
	MAIN_MENU_TEXBPP,
	MAIN_MENU_DEVICE,
	MAIN_MENU_JOYSTICK,
	MAIN_MENU_CAR,
	MAIN_MENU_NAME,
	MAIN_MENU_EDIT,
	MAIN_MENU_BRIGHTNESS,
	MAIN_MENU_CONTRAST,
	MAIN_MENU_REVERSED,
	MAIN_MENU_MIRRORED,

	MAIN_MENU_NEW_TITLE_SCREEN,

	MAIN_MENU_NUM,
};

enum DETAIL_MENU_OPTIONS {
	DETAIL_MENU_WIREFRAME,
	DETAIL_MENU_COLLSKIN_GRID,
	DETAIL_MENU_PERSPECTIVECORRECT,
	DETAIL_MENU_TEXFILTER,
	DETAIL_MENU_MIPMAP,
	DETAIL_MENU_FOG,
	DETAIL_MENU_DITHER,
	DETAIL_MENU_ANTIALIAS,
	DETAIL_MENU_DRAWDIST,
	DETAIL_MENU_FOGSTART,
	DETAIL_MENU_LENS,
	DETAIL_MENU_ENV,
	DETAIL_MENU_MIRROR,
	DETAIL_MENU_SHADOWS,
	DETAIL_MENU_LIGHTS,
	DETAIL_MENU_INSTANCES,
	DETAIL_MENU_SKIDMARKS,
	DETAIL_MENU_CARBOX,
	DETAIL_MENU_NUM,
};

// prototypes

extern void PrintText(short x, short y, char *text);
extern void BeginTextState(void);
extern void DumpText(short x, short y, short xs, short ys, long color, char *text);
extern void DumpBigText(short x, short y, short xs, short ys, long color, char *text);
extern void DumpText3D(VEC *pos, float xs, float ys, long color, char *text);
extern void DetailMenu(void);
extern void MainMenu(void);

#if SHOW_PHYSICS_INFO
extern void ShowPhysicsInfo();
#endif

// globals

extern short MenuCount;

#endif
