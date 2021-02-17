//-----------------------------------------------------------------------------
// File: MenuDraw.h
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef MENUDRAW_H
#define MENUDRAW_H

#include "Menu2.h"

struct object_def;

extern void DrawMenuLogo(MENU_HEADER *menuHeader);
extern void DrawMenuTitle(MENU_HEADER *menuHeader);


extern void DrawMenuText( FLOAT sx, FLOAT sy, DWORD color, CHAR* strText );
extern void DrawMenuText( FLOAT sx, FLOAT sy, DWORD color, WCHAR* strText );

extern void DrawScale(REAL percent, REAL xPos, REAL yPos, REAL xSize, REAL ySize);
extern void DrawMenuDataInt(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex);
extern void DrawMenuDataOnOff(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex);
extern void DrawMenuDataYesNo(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex);
extern void DrawMenuDataDWORD(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex);
extern void DrawSliderDataLong(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex);
extern void DrawSliderDataULong(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex);
extern void DrawSliderDataStringTable(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex);
extern void DrawMenuNotImplemented(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex);
extern void DrawSpruBox(REAL xPos, REAL yPos, REAL xSize, REAL ySize, int col, int trans);

extern void LoadFrontEndTextures();
extern void FreeFrontEndTextures();


extern char MenuBuffer[256];
extern char MenuBuffer2[256];

#define MENU_FRAME_WIDTH 20
#define MENU_FRAME_HEIGHT 20

extern FLOAT               gMenuWidthScale;
extern FLOAT               gMenuHeightScale;

#endif // MENUDRAW_H

