//-----------------------------------------------------------------------------
// File: ui_MenuDraw.h
//
// Desc: UI implementation
//
// Re-Volt (Generic) Copyright (c) Probe Entertainment 1998
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef MENUDRAW_H
#define MENUDRAW_H

#include "ui_Menu.h"

struct object_def;

extern void DrawSpruBox(FLOAT xPos, FLOAT yPos, FLOAT xSize, FLOAT ySize, int col, int trans);
extern void DrawNewSpruBox( FLOAT xPos, FLOAT yPos, FLOAT xSize, FLOAT ySize );
extern void DrawNewSpruBoxWithTabs(FLOAT xPos, FLOAT yPos, FLOAT xSize, FLOAT ySize );
extern void DrawScale(FLOAT percent, FLOAT xPos, FLOAT yPos, FLOAT xSize, FLOAT ySize);
extern void DrawSliderDataSlider(FLOAT xPos, FLOAT yPos, long value, long min, long max, bool active);

extern void DrawMenuDataInt(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex);
extern void DrawMenuDataOnOff(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex);
extern void DrawMenuDataYesNo(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex);
extern void DrawMenuDataDWORD(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex);
extern void DrawSliderDataLong(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex);
extern void DrawSliderDataULong(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex);
extern void DrawMenuNotImplemented(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex);

extern VOID    DrawScreenSpaceQuad( FLOAT sx, FLOAT sy, D3DTexture* pTexture );

extern WCHAR MenuBuffer[256];
extern WCHAR MenuBuffer2[256];

#define MENU_FRAME_WIDTH  20
#define MENU_FRAME_HEIGHT 20

const FLOAT MENU_LEFT_PAD   = 20.0f;
const FLOAT MENU_RIGHT_PAD  = 20.0f;
const FLOAT MENU_TOP_PAD    = 15.0f;
const FLOAT MENU_BOTTOM_PAD = 25.0f;



#endif // MENUDRAW_H

