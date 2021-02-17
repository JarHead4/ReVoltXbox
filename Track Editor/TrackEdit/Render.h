#ifndef _RENDER_H
#define _RENDER_H

#define STRICT
#define D3D_OVERLOADS
#include <ddraw.h>
#include <d3d.h>
#include <d3drm.h>

#include "D3DUtil.h"
#include "D3DFrame.h"
#include "D3DMath.h"
#include "D3DClass.h"

#include "UnitInfo.h"
#include "text.h"


const U32 PURE_RED = 0xFF0000L;
const U32 PURE_GREEN = 0x00FF00L;
const U32 PURE_BLUE = 0x0000FFL;
const U32 PURE_WHITE = 0xFFFFFFL;

void	SetCurrentTexture(LPDIRECT3DDEVICE3 pd3dDevice, LPDIRECT3DTEXTURE2	newtexture);
void	SetSpriteColor(U32 color);
void	DrawErrorMessage(LPDIRECT3DDEVICE3 pd3dDevice);
void	DrawExportingScreen(LPDIRECT3DDEVICE3 pd3dDevice);
void	DrawNewTrackMenu(LPDIRECT3DDEVICE3 pd3dDevice);
void	DrawMainSprue(LPDIRECT3DDEVICE3 pd3dDevice);
void	DrawMenuSprue(LPDIRECT3DDEVICE3 pd3dDevice);
void	DrawLoadMenu(LPDIRECT3DDEVICE3 pd3dDevice);
void	DrawPopupMenu(LPDIRECT3DDEVICE3 pd3dDevice);
void	DrawSaveMenu(LPDIRECT3DDEVICE3 pd3dDevice);
void	DrawSprite(LPDIRECT3DDEVICE3 pd3dDevice, REAL left, REAL top, REAL right, REAL bottom, REAL u0, REAL v0, REAL u1, REAL v1);
void	DrawPlacingScreen(LPDIRECT3DDEVICE3 pd3dDevice, TRACKTHEME* theme);
void	DrawChoosingScreen(LPDIRECT3DDEVICE3 pd3dDevice, TRACKTHEME* theme);
void	DrawAdjustmentScreen(LPDIRECT3DDEVICE3 pd3dDevice, TRACKTHEME* theme);
void	DrawComponent(LPDIRECT3DDEVICE3 pd3dDevice, const COMPONENT* component);
void	DrawUnitRoot(LPDIRECT3DDEVICE3 pd3dDevice, REAL h);
void	DrawFrame(LPDIRECT3DDEVICE3 pd3dDevice, REAL xpix, REAL ypix, U16 cellwidth, U16 cellheight);
void	InitializeCursorPrimitive(void);
void	InitializeUnitRootPrimitive(void);
void	InitializeMenuPolys(void);
void	SpriteClippingOff(void);
void	SpriteClippingOn(void);

#endif //_RENDER_H