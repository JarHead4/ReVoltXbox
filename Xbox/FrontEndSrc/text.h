//-----------------------------------------------------------------------------
// File: Text.h
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef TEXT_H
#define TEXT_H

#include "XBFont.h"


// Macros
#define FONT_WIDTH    8.0f
#define FONT_HEIGHT  16.0f
#define FONT_UWIDTH   8.0f
#define FONT_VHEIGHT 16.0f
#define FONT_PER_ROW 32


// Function prototypes
extern HRESULT LoadFonts();
extern VOID DrawSmallGameText( FLOAT sx, FLOAT sy, DWORD color, CHAR* strText );
extern VOID DrawGameText( FLOAT sx, FLOAT sy, DWORD color, CHAR* strText );
extern VOID DrawWideGameText( FLOAT sx, FLOAT sy, DWORD color, CHAR* strText );
extern VOID DrawBigGameText( FLOAT sx, FLOAT sy, DWORD color, CHAR* strText );
extern VOID DrawBigWideGameText( FLOAT sx, FLOAT sy, DWORD color, CHAR* strText );
extern VOID DrawExtraBigGameText( FLOAT sx, FLOAT sy, DWORD color, CHAR* strText );

extern void BeginTextState(void);
extern void DumpText3D(VEC *pos, float xs, float ys, long color, char *text);


// Globals
extern CXBFont* g_pSmallGameFont;     // 8x12 game font
extern CXBFont* g_pGameFont;          // 8x16 game font
extern CXBFont* g_pWideGameFont;      // 12x16 game font
extern CXBFont* g_pBigGameFont;       // 12x24 game font
extern CXBFont* g_pBigWideGameFont;   // 16x24 game font
extern CXBFont* g_pExtraBigGameFont;  // 16x32 game font
extern CXBFont* g_pMenuFont;          // Font for menus
extern CXBFont* g_pMenuTitleFont;     // Font for menu titles


#endif // TEXT_H

