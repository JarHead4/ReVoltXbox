//-----------------------------------------------------------------------------
// File: text.h
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef TEXT_H
#define TEXT_H

#include "XBFont.h"

// macros

#define FONT_WIDTH 8.0f
#define FONT_HEIGHT 16.0f
#define FONT_UWIDTH 8.0f
#define FONT_VHEIGHT 16.0f
#define FONT_PER_ROW 32

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


// prototypes
extern HRESULT LoadFonts();
extern void DrawMenuTextWithArrows( BOOL bDrawArrows, FLOAT x, FLOAT y, DWORD color, WCHAR* strText, FLOAT fMaxWidth = -1.0f );
extern void DrawMenuText( FLOAT x, FLOAT y, DWORD color, WCHAR* text, FLOAT fMaxWidth = -1.0f );

extern void BeginTextState(void);
extern void DumpText(int x, int y, int xs, int ys, long color, WCHAR *text);
extern void DumpTextReal(REAL x, REAL y, REAL xs, REAL ys, long color, WCHAR *text);
extern void DumpText3D(VEC *pos, float xs, float ys, long color, WCHAR *text);
//$REMOVED_UNREACHABLEextern void MainMenu(void);

#if SHOW_PHYSICS_INFO
extern void ShowPhysicsInfo();
#endif

// globals

extern short  MenuCount;

extern CXBFont* g_pFont;


#endif // TEXT_H

