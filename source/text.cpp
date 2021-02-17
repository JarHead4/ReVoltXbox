
#include "revolt.h"
#include "draw.h"
#include "main.h"
#include "text.h"
#include "input.h"
#include "play.h"
#include "model.h"
#include "world.h"
#include "texture.h"
#include "geom.h"
#include "camera.h"
#include "Particle.h"
#include "NewColl.h"
#include "Body.h"
#include "Aerial.h"
#include "Wheel.h"
#include "level.h"
#include "ctrlread.h"
#include "object.h"
#include "player.h"
#include "gameloop.h"
#include "registry.h"
#include "ghost.h"
#include "ai.h"
#ifdef _PC
#include "spark.h"
#endif
#include "timing.h"

#include "TitleScreen.h"


// globals

static VERTEX_TEX1 TextVert[4];
short MenuCount;

// menu text

char MainMenuAllowed[] = {
	TRUE,
	FALSE,
	FALSE,
	FALSE,
	FALSE,
	TRUE,
	TRUE,
	FALSE,
	TRUE,
	TRUE,
	FALSE,
	FALSE,
	TRUE,
	TRUE,
	FALSE,
	FALSE,

	FALSE,
};

char *MainMenuText[] = {
	"Start Game",
	"Start AI Test",
	"Start Multi Player",
	"Join Multi Player",
	"Track: ",
	"Screen: ",
	"Textures: ",
	"Device: ",
	"Joystick: ",
	"Car: ",
	"Change Name: ",
	"Edit Mode: ",
	"Brightness: ",
	"Contrast: ",
	"Reversed: ",
	"Mirrored: ",

	"New Front End",
};

char *DetailMenuText[] = {
	"Wireframe: ",
	"Collision: ",
	"Perspective Correct: ",
	"Texture Filter: ",
	"Mip Map: ",
	"Fog: ",
	"Dithering: ",
	"Antialias: ",
	"Draw Dist: ",
	"Fog Start: ",
	"Lens: ",
	"Env Mapping: ",
	"Mirrors: ",
	"Shadows: ",
	"Lighting: ",
	"Instances: ",
	"Skidmarks: ",
	"Car BBoxes: "
};

char *EditMenuText[] = {
	"None",
	"Lights",
	"Visiboxes",
	"Objects",
	"Instances",
	"AI Nodes",
	"Track Zones",
	"Triggers",
	"Camera Nodes",
	"Force Fields",
	"Gay Matttt's Tony Portals",
};

char *TextureFilterText[] = {
    "Point",
    "Linear",
    "Anisotropic",
};

char *MipMapText[] = {
    "None",
    "Point",
    "Linear",
};

char *NoYesText[] = {
	"No",
	"Yes",
};

//////////////////////////////
// draw text to back buffer //
//////////////////////////////

void PrintText(short x, short y, char *text)
{
	HDC hdc;

	if (BackBuffer->GetDC(&hdc) == DD_OK)
	{
		SetBkColor(hdc, RGB(0, 0, 0));
		SetTextColor(hdc, RGB(255, 255, 255));
		TextOut(hdc, x, y, text, strlen(text));
		BackBuffer->ReleaseDC(hdc);
	}
}

//////////////////////
// begin text state //
//////////////////////

void BeginTextState(void)
{
	ZBUFFER_OFF();
	ALPHA_OFF();
	FOG_OFF();
	WIREFRAME_OFF();
	SET_TPAGE(TPAGE_FONT);
}

//////////////////////////////
// draw text to back buffer //
//////////////////////////////

void DumpText(short x, short y, short xs, short ys, long color, char *text)
{
	char i;
	float tu, tv;
	float xstart, ystart, xsize, ysize;
	long lu, lv, ch;

// calc size / pos

	xstart = (float)x * RenderSettings.GeomScaleX + ScreenLeftClip;
	ystart = (float)y * RenderSettings.GeomScaleY + ScreenTopClip;

	xsize = (float)xs * RenderSettings.GeomScaleX;
	ysize = (float)ys * RenderSettings.GeomScaleY;

// init vert misc

	for (i = 0 ; i < 4 ; i++)
	{
		TextVert[i].color = color;
		TextVert[i].rhw = 1;
	}

// draw chars

	while (*text)
	{

// get char

		ch = *text - 33;
		if (ch != -1)
		{

// set screen coors

			TextVert[0].sx = xstart;
			TextVert[0].sy = ystart;

			TextVert[1].sx = xstart + xsize;
			TextVert[1].sy = ystart;

			TextVert[2].sx = xstart + xsize;
			TextVert[2].sy = ystart + ysize;

			TextVert[3].sx = xstart;
			TextVert[3].sy = ystart + ysize;

// set uv's

			lu = ch % FONT_PER_ROW;
			lv = ch / FONT_PER_ROW;

			tu = (float)lu * FONT_WIDTH;
			tv = (float)lv * FONT_HEIGHT;

			TextVert[0].tu = tu / 256.0f;
			TextVert[0].tv = tv / 256.0f;

			TextVert[1].tu = (tu + FONT_UWIDTH) / 256.0f;
			TextVert[1].tv = tv / 256.0f;

			TextVert[2].tu = (tu + FONT_UWIDTH) / 256.0f;
			TextVert[2].tv = (tv + FONT_VHEIGHT) / 256.0f;

			TextVert[3].tu = tu / 256.0f;
			TextVert[3].tv = (tv + FONT_VHEIGHT) / 256.0f;

// draw

			D3Ddevice->DrawPrimitive(D3DPT_TRIANGLEFAN, FVF_TEX1, TextVert, 4, D3DDP_DONOTUPDATEEXTENTS);
		}

// next

		xstart += xsize;
		text++;
	}
}

//////////////////////////////////
// draw big text to back buffer //
//////////////////////////////////

void DumpBigText(short x, short y, short xs, short ys, long color, char *text)
{
	char i;
	float tu, tv;
	float xstart, ystart, xsize, ysize;
	long lu, lv, ch;

// calc size / pos

	xstart = (float)x * RenderSettings.GeomScaleX + ScreenLeftClip;
	ystart = (float)y * RenderSettings.GeomScaleY + ScreenTopClip;

	xsize = (float)xs * RenderSettings.GeomScaleX;
	ysize = (float)ys * RenderSettings.GeomScaleY;

// init vert misc

	for (i = 0 ; i < 4 ; i++)
	{
		TextVert[i].color = color;
		TextVert[i].rhw = 1;
	}

// draw chars

	while (*text)
	{

// get char

		ch = *text - 33;
		if (ch != -1)
		{

// set screen coors

			TextVert[0].sx = xstart;
			TextVert[0].sy = ystart;

			TextVert[1].sx = xstart + xsize;
			TextVert[1].sy = ystart;

			TextVert[2].sx = xstart + xsize;
			TextVert[2].sy = ystart + ysize;

			TextVert[3].sx = xstart;
			TextVert[3].sy = ystart + ysize;

// set uv's

			lu = ch % BIG_FONT_PER_ROW;
			lv = ch / BIG_FONT_PER_ROW;

			tu = (float)lu * BIG_FONT_WIDTH;
			tv = (float)lv * BIG_FONT_HEIGHT;

			TextVert[0].tu = tu / 256.0f;
			TextVert[0].tv = tv / 256.0f;

			TextVert[1].tu = (tu + BIG_FONT_UWIDTH) / 256.0f;
			TextVert[1].tv = tv / 256.0f;

			TextVert[2].tu = (tu + BIG_FONT_UWIDTH) / 256.0f;
			TextVert[2].tv = (tv + BIG_FONT_VHEIGHT) / 256.0f;

			TextVert[3].tu = tu / 256.0f;
			TextVert[3].tv = (tv + BIG_FONT_VHEIGHT) / 256.0f;

// draw

			D3Ddevice->DrawPrimitive(D3DPT_TRIANGLEFAN, FVF_TEX1, TextVert, 4, D3DDP_DONOTUPDATEEXTENTS);
		}

// next

		xstart += xsize;
		text++;
	}
}

//////////////////////////////
// draw text to back buffer //
//////////////////////////////

void DumpText3D(VEC *pos, float xs, float ys, long color, char *text)
{
	char i;
	float tu, tv, sz, rhw;
	float x, y;
	long lu, lv, ch;

// set tpage

	SET_TPAGE(TPAGE_FONT);

// calc size / pos

	xs = xs * RenderSettings.GeomPers / pos->v[Z] * RenderSettings.GeomScaleX;
	ys = ys * RenderSettings.GeomPers / pos->v[Z] * RenderSettings.GeomScaleY;
	x = pos->v[X] * RenderSettings.GeomPers / pos->v[Z] * RenderSettings.GeomScaleX + RenderSettings.GeomCentreX;
	y = pos->v[Y] * RenderSettings.GeomPers / pos->v[Z] * RenderSettings.GeomScaleY + RenderSettings.GeomCentreY;

// init vert misc

	sz = GET_ZBUFFER(pos->v[Z]);
	rhw = 1 / pos->v[Z];

	for (i = 0 ; i < 4 ; i++)
	{
		TextVert[i].color = color;
		TextVert[i].sz = sz;
		TextVert[i].rhw = rhw;
		
	}

// draw chars

	while (*text)
	{

// get char

		ch = *text - 33;
		if (ch != -1)
		{

// set screen coors

			TextVert[0].sx = x;
			TextVert[0].sy = y;

			TextVert[1].sx = (x + xs);
			TextVert[1].sy = y;

			TextVert[2].sx = (x + xs);
			TextVert[2].sy = (y + ys);

			TextVert[3].sx = x;
			TextVert[3].sy = (y + ys);

// set uv's

			lu = ch % FONT_PER_ROW;
			lv = ch / FONT_PER_ROW;

			tu = (float)lu * FONT_WIDTH;
			tv = (float)lv * FONT_HEIGHT;

			TextVert[0].tu = tu / 256;
			TextVert[0].tv = tv / 256;

			TextVert[1].tu = (tu + FONT_UWIDTH) / 256;
			TextVert[1].tv = tv / 256;

			TextVert[2].tu = (tu + FONT_UWIDTH) / 256;
			TextVert[2].tv = (tv + FONT_VHEIGHT - 1) / 256;

			TextVert[3].tu = tu / 256;
			TextVert[3].tv = (tv + FONT_VHEIGHT - 1) / 256;

// draw

			D3Ddevice->DrawPrimitive(D3DPT_TRIANGLEFAN, FVF_TEX1, TextVert, 4, D3DDP_DONOTUPDATEEXTENTS);
		}

// next

		x += xs;
		text++;
	}
}

/////////////////
// detail menu //
/////////////////

void DetailMenu(void)
{
	short i;
	static short select = 0;
	unsigned long col;
	char *togg;
	long flag;
	char off[] = "Off";
	char on[] = "On";
	char text[6], lr = 0;

// move

	if (Keys[DIK_UP] && !LastKeys[DIK_UP] && select) select--;
	if (Keys[DIK_DOWN] && !LastKeys[DIK_DOWN] && select < DETAIL_MENU_NUM - 1) select++;
	if (Keys[DIK_LEFT] && !LastKeys[DIK_LEFT]) lr |= 1;
	if (Keys[DIK_RIGHT] && !LastKeys[DIK_RIGHT]) lr |= 2;

// sort each detail setting

	for (i = 0 ; i < DETAIL_MENU_NUM ; i++)
	{
		flag = TRUE;
		togg = off;

		switch (i)
		{

// wireframe

			case DETAIL_MENU_WIREFRAME:
				if ((flag = DxState.WireframeEnabled) && i == select)
				{
					if (lr & 1)	DxState.Wireframe = D3DFILL_SOLID;
					if (lr & 2) DxState.Wireframe = D3DFILL_WIREFRAME;
				}
				if (DxState.Wireframe == D3DFILL_WIREFRAME) togg = on;
			break;

// Collision skin

			case DETAIL_MENU_COLLSKIN_GRID:
				if (i == select)
				{
					if (lr != 0) DrawGridCollSkin = !DrawGridCollSkin;
				}
				flag = TRUE;
				if (DrawGridCollSkin) togg = on;
			break;

// perspective correct

			case DETAIL_MENU_PERSPECTIVECORRECT:
				if ((flag = DxState.PerspectiveEnabled) && i == select)
				{
					if (lr & 1) DxState.Perspective = FALSE;
					if (lr & 2) DxState.Perspective = TRUE;
					PERSPECTIVE_ON();
				}
				if (DxState.Perspective) togg = on;
			break;

// texture filtering

			case DETAIL_MENU_TEXFILTER:
				if (i == select)
				{
					if (lr & 1 && DxState.TextureFilter) DxState.TextureFilter--;
					if (lr & 2 && DxState.TextureFilter < 2) DxState.TextureFilter++;
					TEXTUREFILTER_ON();
				}
				flag = DxState.TextureFilterFlag & (1 << DxState.TextureFilter);
				togg = TextureFilterText[DxState.TextureFilter];
			break;

// mip map

			case DETAIL_MENU_MIPMAP:
				if (i == select)
				{
					if (lr & 1 && DxState.MipMap) DxState.MipMap--;
					if (lr & 2 && DxState.MipMap < 2) DxState.MipMap++;
					MIPMAP_ON();
				}
				flag = DxState.MipMapFlag & (1 << DxState.MipMap);
				togg = MipMapText[DxState.MipMap];
			break;

// fog

			case DETAIL_MENU_FOG:
				if ((flag = DxState.FogEnabled) && i == select)
				{
					if (lr & 1) DxState.Fog = FALSE;
					if (lr & 2) DxState.Fog = TRUE;
				}
				if (DxState.Fog) togg = on;
			break;

// dithering

			case DETAIL_MENU_DITHER:
				if ((flag = DxState.DitherEnabled) && i == select)
				{
					if (lr & 1) DxState.Dither = FALSE;
					if (lr & 2) DxState.Dither = TRUE;
					DITHER_ON();
				}
				if (DxState.Dither) togg = on;
			break;

// antialias

			case DETAIL_MENU_ANTIALIAS:
				if ((flag = DxState.AntiAliasEnabled) && i == select)
				{
					if (lr & 1) DxState.AntiAlias = D3DANTIALIAS_NONE;
					if (lr & 2) DxState.AntiAlias = D3DANTIALIAS_SORTINDEPENDENT;
					ANTIALIAS_ON();
				}
				if (DxState.AntiAlias == D3DANTIALIAS_SORTINDEPENDENT) togg = on;
			break;

// draw dist

			case DETAIL_MENU_DRAWDIST:
				wsprintf(text, "%ld", (long)LevelInf[GameSettings.Level].FarClip);
				togg = text;

				if (i == select)
				{
					flag = TRUE;
					if (Keys[DIK_LEFT] && LevelInf[GameSettings.Level].FarClip > 0) LevelInf[GameSettings.Level].FarClip -= 128;
					if (Keys[DIK_RIGHT] && LevelInf[GameSettings.Level].FarClip < 65536) LevelInf[GameSettings.Level].FarClip += 128;
					if (LevelInf[GameSettings.Level].FogStart > LevelInf[GameSettings.Level].FarClip) LevelInf[GameSettings.Level].FogStart = LevelInf[GameSettings.Level].FarClip;
					SetNearFar(48.0f, LevelInf[GameSettings.Level].FarClip);
					SetFogVars(LevelInf[GameSettings.Level].FogStart, LevelInf[GameSettings.Level].VertFogStart, LevelInf[GameSettings.Level].VertFogEnd);
				}
			break;

// fog start

			case DETAIL_MENU_FOGSTART:
				wsprintf(text, "%ld", (long)LevelInf[GameSettings.Level].FogStart);
				togg = text;

				if (i == select)
				{
					flag = TRUE;
					if (Keys[DIK_LEFT] && LevelInf[GameSettings.Level].FogStart > 0) LevelInf[GameSettings.Level].FogStart -= 128;
					if (Keys[DIK_RIGHT] && LevelInf[GameSettings.Level].FogStart < 65536) LevelInf[GameSettings.Level].FogStart += 128;
					if (LevelInf[GameSettings.Level].FogStart > LevelInf[GameSettings.Level].FarClip) LevelInf[GameSettings.Level].FarClip = LevelInf[GameSettings.Level].FogStart;
					SetNearFar(48.0f, LevelInf[GameSettings.Level].FarClip);
					SetFogVars(LevelInf[GameSettings.Level].FogStart, LevelInf[GameSettings.Level].VertFogStart, LevelInf[GameSettings.Level].VertFogEnd);
				}
			break;

// lens

			case DETAIL_MENU_LENS:
				wsprintf(text, "%d", (short)BaseGeomPers);
				togg = text;

				if (i == select)
				{
					flag = TRUE;
					if (Keys[DIK_LEFT] && BaseGeomPers > 16) BaseGeomPers -= 16;
					if (Keys[DIK_RIGHT] && BaseGeomPers < 4096) BaseGeomPers += 16;
				}
			break;

// env mapping

			case DETAIL_MENU_ENV:
				if (i == select)
				{
					flag = TRUE;
					if (lr & 1) RenderSettings.Env = FALSE;
					if (lr & 2) RenderSettings.Env = TRUE;
				}
				if (RenderSettings.Env) togg = on;
			break;

// mirror

			case DETAIL_MENU_MIRROR:
				if (i == select)
				{
					flag = TRUE;
					if (lr & 1)
					{
						RenderSettings.Mirror = FALSE;
						SetWorldMirror();
					}
					if (lr & 2)
					{
						RenderSettings.Mirror = TRUE;
						SetWorldMirror();
					}
				}
				if (RenderSettings.Mirror) togg = on;
			break;

// shadows

			case DETAIL_MENU_SHADOWS:
				if (i == select)
				{
					flag = TRUE;
					if (lr & 1) RenderSettings.Shadow = FALSE;
					if (lr & 2) RenderSettings.Shadow = TRUE;
				}
				if (RenderSettings.Shadow) togg = on;
			break;

// lights

			case DETAIL_MENU_LIGHTS:
				if (i == select)
				{
					flag = TRUE;
					if (lr & 1) RenderSettings.Light = FALSE;
					if (lr & 2) RenderSettings.Light = TRUE;
				}
				if (RenderSettings.Light) togg = on;
			break;

// instances

			case DETAIL_MENU_INSTANCES:
				if (i == select)
				{
					flag = TRUE;
					if (lr & 1) RenderSettings.Instance = FALSE;
					if (lr & 2) RenderSettings.Instance = TRUE;
				}
				if (RenderSettings.Instance) togg = on;
			break;

// skidmarks

			case DETAIL_MENU_SKIDMARKS:
				if (i == select)
				{
					flag = TRUE;
					if (lr & 1) RenderSettings.Skid = FALSE;
					if (lr & 2) RenderSettings.Skid = TRUE;
				}
				if (RenderSettings.Skid) togg = on;
			break;

// car bboxes

			case DETAIL_MENU_CARBOX:
				if (i == select)
				{
					flag = TRUE;
					if (Keys[DIK_LEFT] && !LastKeys[DIK_LEFT]) CAR_DrawCarBBoxes = TRUE;
					if (Keys[DIK_RIGHT] && !LastKeys[DIK_RIGHT]) CAR_DrawCarBBoxes = FALSE;
					if (CAR_DrawCarBBoxes == TRUE) togg = on;
				}
			break;
		}

// get col

		if (!flag)
		{
			if (i == select) col = 0x800000;
			else col = 0x000000;
		}
		else
		{
			if (i == select) col = 0xff0000;
			else col = 0xc0c0c0;
		}

// dump text

		DumpText(128, i * 16 + 64, 12, 16, col, DetailMenuText[i]);
		DumpText(128 + strlen(DetailMenuText[i]) * 12, i * 16 + 64, 12, 16, col, togg);
	}
}

///////////////
// main menu //
///////////////

void MainMenu(void)
{
	short i, line, flag;
	long col;
	unsigned char c;
	char *text;
	char buf[128];

// buffer flip / clear

	CheckSurfaces();
	FlipBuffers();
	ClearBuffers();

// update pos

	ReadMouse();
	ReadKeyboard();
	UpdateTimeFactor();

	if (Keys[DIK_UP] && !LastKeys[DIK_UP])
	{
		flag = TRUE;
		while ((!MainMenuAllowed[MenuCount] && !Everything) || flag)
		{
			MenuCount--;
			if (MenuCount < 0) MenuCount = MAIN_MENU_NUM - 1;
			flag = FALSE;
		}
	}

	if (Keys[DIK_DOWN] && !LastKeys[DIK_DOWN])
	{
		flag = TRUE;
		while ((!MainMenuAllowed[MenuCount] && !Everything) || flag)
		{
			MenuCount++;
			if (MenuCount > MAIN_MENU_NUM - 1) MenuCount = 0;
			flag = FALSE;
		}
	}

// show menu

	D3Ddevice->BeginScene();

	BlitBitmap(TitleHbm, &BackBuffer);

	BeginTextState();

	line = 8;

	for (i = 0 ; i < MAIN_MENU_NUM ; i++) if (MainMenuAllowed[i] || Everything)
	{
		if (MenuCount == i) col = 0xff0000;
		else col = 0x808080;

		DumpText(224, line * 16, 12, 16, col, MainMenuText[i]);

		if (i == MAIN_MENU_TRACK && Everything)
		{
			if (MenuCount == i)
			{
				if (Keys[DIK_LEFT] && !LastKeys[DIK_LEFT]) GameSettings.Level--;
				if (Keys[DIK_RIGHT] && !LastKeys[DIK_RIGHT]) GameSettings.Level++;
				if (GameSettings.Level == -1) GameSettings.Level = GameSettings.LevelNum - 1;
				if (GameSettings.Level == GameSettings.LevelNum) GameSettings.Level = 0;

				memcpy(RegistrySettings.LevelDir, LevelInf[GameSettings.Level].Dir, MAX_LEVEL_DIR_NAME);
			}
			DumpText(224 + strlen(MainMenuText[i]) * 12, line * 16, 12, 16, col, LevelInf[GameSettings.Level].Name);
		}

		if (i == MAIN_MENU_RES)
		{
			if (MenuCount == i)
			{
				if (Keys[DIK_LEFT] && !LastKeys[DIK_LEFT]) DisplayModeCount--;
				if (Keys[DIK_RIGHT] && !LastKeys[DIK_RIGHT]) DisplayModeCount++;

				if (DisplayModeCount < 0) DisplayModeCount = 0;
				if (DisplayModeCount >= DrawDevices[RegistrySettings.DrawDevice].DisplayModeNum) DisplayModeCount = DrawDevices[RegistrySettings.DrawDevice].DisplayModeNum - 1;

				RegistrySettings.ScreenWidth = DrawDevices[RegistrySettings.DrawDevice].DisplayMode[DisplayModeCount].Width;
				RegistrySettings.ScreenHeight = DrawDevices[RegistrySettings.DrawDevice].DisplayMode[DisplayModeCount].Height;
			}
			DumpText(224 + strlen(MainMenuText[i]) * 12, line * 16, 12, 16, col, DrawDevices[RegistrySettings.DrawDevice].DisplayMode[DisplayModeCount].DisplayText);
		}

		if (i == MAIN_MENU_TEXBPP)
		{
			if (MenuCount == i)
			{
				if (Keys[DIK_LEFT] && !LastKeys[DIK_LEFT]) RegistrySettings.TextureBpp = 16;
				if (Keys[DIK_RIGHT] && !LastKeys[DIK_RIGHT]) RegistrySettings.TextureBpp = 24;
			}
			DumpText(224 + strlen(MainMenuText[i]) * 12, line * 16, 12, 16, col, RegistrySettings.TextureBpp == 16 ? "16 bit" : "24 bit");
		}

		if (i == MAIN_MENU_DEVICE)
		{
			if (MenuCount == i)
			{
				if (Keys[DIK_LEFT] && !LastKeys[DIK_LEFT] && RegistrySettings.DrawDevice)
				{
					RegistrySettings.DrawDevice--;
					DisplayModeCount = DrawDevices[RegistrySettings.DrawDevice].BestDisplayMode;
				}
				if (Keys[DIK_RIGHT] && !LastKeys[DIK_RIGHT] && RegistrySettings.DrawDevice < (DWORD)DrawDeviceNum - 1)
				{
					RegistrySettings.DrawDevice++;
					DisplayModeCount = DrawDevices[RegistrySettings.DrawDevice].BestDisplayMode;
				}
			}
			DumpText(224 + strlen(MainMenuText[i]) * 12, line * 16, 12, 16, col, DrawDevices[RegistrySettings.DrawDevice].Name);
		}

		if (i == MAIN_MENU_JOYSTICK)
		{
			if (JoystickNum && MenuCount == i)
			{
				if (Keys[DIK_LEFT] && !LastKeys[DIK_LEFT] && CurrentJoystick > -1) CurrentJoystick--;
				if (Keys[DIK_RIGHT] && !LastKeys[DIK_RIGHT] && CurrentJoystick < JoystickNum - 1) CurrentJoystick++;
			}

			if (CurrentJoystick == -1)
				text = "None";
			else
				text = Joystick[CurrentJoystick].Name;

			DumpText(224 + strlen(MainMenuText[i]) * 12, line * 16, 12, 16, col, text);
		}

		if (i == MAIN_MENU_CAR)
		{
			if (MenuCount == i)
			{
				if (Keys[DIK_LEFT] && !LastKeys[DIK_LEFT])
				{
					if (Everything)
						GameSettings.CarID = PrevValidCarID(GameSettings.CarID);
					else
						GameSettings.CarID = 2;
				}
				if (Keys[DIK_RIGHT] && !LastKeys[DIK_RIGHT])
				{
					if (Everything)
						GameSettings.CarID = NextValidCarID(GameSettings.CarID);
					else
						GameSettings.CarID = 4;
				}
			}

			text = CarInfo[GameSettings.CarID].Name;
			DumpText(224 + strlen(MainMenuText[i]) * 12, line * 16, 12, 16, col, text);
		}

		if (i == MAIN_MENU_NAME)
		{
			if (MenuCount == i)
			{
				if ((c = GetKeyPress()))
				{
					if (c == 8)
					{
						if (strlen(RegistrySettings.PlayerName))
						{
							RegistrySettings.PlayerName[strlen(RegistrySettings.PlayerName) - 1] = 0;
						}
					}
					else if (c != 13 && c != 27)
					{
						if (strlen(RegistrySettings.PlayerName) < MAX_PLAYER_NAME - 1)
						{
							RegistrySettings.PlayerName[strlen(RegistrySettings.PlayerName)] = c;
							RegistrySettings.PlayerName[strlen(RegistrySettings.PlayerName) + 1] = 0;
						}
					}
				}
				DumpText(224 + strlen(MainMenuText[i]) * 12 + strlen(RegistrySettings.PlayerName) * 12, line * 16, 12, 16, col, "_");
			}
			DumpText(224 + strlen(MainMenuText[i]) * 12, line * 16, 12, 16, col, RegistrySettings.PlayerName);
		}

		if (i == MAIN_MENU_EDIT)
		{
			if (MenuCount == i)
			{
				if (Keys[DIK_LEFT] && !LastKeys[DIK_LEFT]) EditMode--;
				if (Keys[DIK_RIGHT] && !LastKeys[DIK_RIGHT]) EditMode++;
				if (EditMode < 0) EditMode = EDIT_NUM - 1;
				if (EditMode == EDIT_NUM) EditMode = 0;
			}
			DumpText(224 + strlen(MainMenuText[i]) * 12, line * 16, 12, 16, col, EditMenuText[EditMode]);
		}

		if (i == MAIN_MENU_BRIGHTNESS)
		{
			if (GammaFlag == GAMMA_UNAVAILABLE)
			{
				text = "Unavailable";
			}
			else if (GammaFlag == GAMMA_AUTO)
			{
				text = "Auto";
			}
			else
			{
				if (MenuCount == i)
				{
					if (Keys[DIK_LEFT]) RegistrySettings.Brightness -= (long)(TimeStep * 100.0f);
					if (Keys[DIK_RIGHT]) RegistrySettings.Brightness += (long)(TimeStep * 100.0f);

					if (RegistrySettings.Brightness & 0x80000000) RegistrySettings.Brightness = 0;
					if (RegistrySettings.Brightness > 512) RegistrySettings.Brightness = 512;

					SetGamma(RegistrySettings.Brightness, RegistrySettings.Contrast);
				}
				wsprintf(buf, "%d", RegistrySettings.Brightness * 100 / 512);
				text = buf;
			}
			DumpText(224 + strlen(MainMenuText[i]) * 12, line * 16, 12, 16, col, text);
		}

		if (i == MAIN_MENU_CONTRAST)
		{
			if (GammaFlag == GAMMA_UNAVAILABLE)
			{
				text = "Unavailable";
			}
			else if (GammaFlag == GAMMA_AUTO)
			{
				text = "Auto";
			}
			else
			{
				if (MenuCount == i)
				{
					if (Keys[DIK_LEFT]) RegistrySettings.Contrast -= (long)(TimeStep * 100.0f);
					if (Keys[DIK_RIGHT]) RegistrySettings.Contrast += (long)(TimeStep * 100.0f);

					if (RegistrySettings.Contrast & 0x80000000) RegistrySettings.Contrast = 0;
					if (RegistrySettings.Contrast > 512) RegistrySettings.Contrast = 512;

					SetGamma(RegistrySettings.Brightness, RegistrySettings.Contrast);
				}
				wsprintf(buf, "%d", RegistrySettings.Contrast * 100 / 512);
				text = buf;
			}
			DumpText(224 + strlen(MainMenuText[i]) * 12, line * 16, 12, 16, col, text);
		}

		if (i == MAIN_MENU_REVERSED)
		{
			if (MenuCount == i)
			{
				if (Keys[DIK_LEFT] && !LastKeys[DIK_LEFT]) GameSettings.Reversed = FALSE;
				if (Keys[DIK_RIGHT] && !LastKeys[DIK_RIGHT]) GameSettings.Reversed = TRUE;
			}
			DumpText(224 + strlen(MainMenuText[i]) * 12, line * 16, 12, 16, col, NoYesText[GameSettings.Reversed]);
		}

		if (i == MAIN_MENU_MIRRORED)
		{
			if (MenuCount == i)
			{
				if (Keys[DIK_LEFT] && !LastKeys[DIK_LEFT]) GameSettings.Mirrored = FALSE;
				if (Keys[DIK_RIGHT] && !LastKeys[DIK_RIGHT]) GameSettings.Mirrored = TRUE;
			}
			DumpText(224 + strlen(MainMenuText[i]) * 12, line * 16, 12, 16, col, NoYesText[GameSettings.Mirrored]);
		}

	line += 1 + (!Everything);
	}

	D3Ddevice->EndScene();

// selected?

	if (Keys[DIK_RETURN] && !LastKeys[DIK_RETURN] && (MainMenuAllowed[MenuCount] || Everything))
	{
		AI_Testing = FALSE;

		if (MenuCount == MAIN_MENU_SINGLE)
		{
			GameSettings.GameType = GAMETYPE_TRIAL;
			Event = SetupGame;
		}

		if (MenuCount == MAIN_MENU_AI_TEST)
		{
			AI_Testing = TRUE;
			GameSettings.GameType = GAMETYPE_SINGLE;
			Event = SetupGame;
		}

		if (MenuCount == MAIN_MENU_MULTI)
		{
			GameSettings.GameType = GAMETYPE_SERVER;
			MenuCount = 0;
			KillPlay();
			InitPlay();
			Event = ConnectionMenu;
		}

		if (MenuCount == MAIN_MENU_JOIN)
		{
			GameSettings.GameType = GAMETYPE_CLIENT;
			MenuCount = 0;
			KillPlay();
			InitPlay();
			Event = ConnectionMenu;
		}

		if (MenuCount == MAIN_MENU_NEW_TITLE_SCREEN)
		{
			Event = GoTitleScreen;
		}
	}

// quit?

	if (Keys[DIK_ESCAPE] && !LastKeys[DIK_ESCAPE])
		QuitGame = TRUE;
}


/////////////////////////////////////////////////////////////////////
//
// ShowPhysicsInfo: Display info about the passed car on the screen
//
/////////////////////////////////////////////////////////////////////

#if SHOW_PHYSICS_INFO

#if USE_DEBUG_ROUTINES
extern REAL DEBUG_MaxImpulseMag;
extern REAL DEBUG_MaxAngImpulseMag;
#endif

extern GHOST_INFO *GhostInfo;
extern GHOST_INFO *GHO_BestGhostInfo;
extern int COL_NCollsTested;
extern int DEBUG_NIts;
extern bool DEBUG_Converged;
extern REAL DEBUG_Res;

void ShowPhysicsInfo() 
{
	char buf[256];

	// Defines
#if REMOVE_JITTER
	wsprintf(buf, "Jitter: On   %1d", PLR_LocalPlayer->car.Body->IsJittering);
#else 
	wsprintf(buf, "Jitter: Off");
#endif
	DumpText(0, 40, 6, 12, 0xffffff, buf);

	// Engine voltage and steering angle + autobraking
	wsprintf(buf, "Engine: %3d   Steer: %3d   Revs: %5d   AutoBrake: %s", 
		(int) (100.0f * PLR_LocalPlayer->car.EngineVolt),
		(int) (100.0f * PLR_LocalPlayer->car.SteerAngle),
		(int) (100.0f * PLR_LocalPlayer->car.Revs),
		(GameSettings.AutoBrake)? "On": "Off");
	DumpText(100, 40, 8, 16, 0xffffff, buf);

	// TimeStep
	wsprintf(buf, "TimeStep: %4d (%2d)", (int)(10000.0f * TimeStep), NPhysicsLoops);
	DumpText(0, 60, 8, 16, 0xffffff, buf);

	// Ghost Info
	wsprintf(buf, "Ghost frame: %8d / %8d", GHO_BestFrame, GHO_BestGhostInfo->NFrames);
	DumpText(200, 60, 8, 16, 0xffffff, buf);

	// Number of collisions flagged
	wsprintf(buf, "NBodyCols: %3d (%3d)  NWheelColls: %3d (%3d)", COL_NBodyColls - COL_NBodyDone, COL_NBodyColls, COL_NWheelColls - COL_NWheelDone, COL_NWheelColls);
	DumpText(0, 80, 8, 16, 0xffffff, buf);

	// Pos and orientation of car
	wsprintf(buf, "Pos: %5d %5d %5d  Vel: %5d %5d %5d  Ang: %5d %5d %5d", 
		(int) (1.0f * PLR_LocalPlayer->car.Body->Centre.Pos.v[X]),
		(int) (1.0f * PLR_LocalPlayer->car.Body->Centre.Pos.v[Y]),
		(int) (1.0f * PLR_LocalPlayer->car.Body->Centre.Pos.v[Z]),
		(int) (1.0f * PLR_LocalPlayer->car.Body->Centre.Vel.v[X]),
		(int) (1.0f * PLR_LocalPlayer->car.Body->Centre.Vel.v[Y]),
		(int) (1.0f * PLR_LocalPlayer->car.Body->Centre.Vel.v[Z]),
		(int) (100.0f * PLR_LocalPlayer->car.Body->AngVel.v[X]),
		(int) (100.0f * PLR_LocalPlayer->car.Body->AngVel.v[Y]),
		(int) (100.0f * PLR_LocalPlayer->car.Body->AngVel.v[Z]));
	DumpText(0, 100, 8, 16, 0xffffff, buf);
	wsprintf(buf, "Grid: %5ld (%5ld Polys)", DEBUG_CollGrid, COL_CollGrid[DEBUG_CollGrid].NCollPolys);
	DumpText(320, 120, 8, 16, 0xffffff, buf);

#if USE_DEBUG_ROUTINES
	wsprintf(buf, "Its %d", DEBUG_NIts);
	DumpText(320, 140, 8, 16, 0xffffff, buf);
#endif

	wsprintf(buf, "%6d %6d %6d %6d", 
		(int) (1000.0f * PLR_LocalPlayer->car.Body->Centre.WMatrix.m[XX]),
		(int) (1000.0f * PLR_LocalPlayer->car.Body->Centre.WMatrix.m[XY]),
		(int) (1000.0f * PLR_LocalPlayer->car.Body->Centre.WMatrix.m[XZ]),
		(int) (1000.0f * Length(&PLR_LocalPlayer->car.Body->Centre.WMatrix.mv[R])));
	DumpText(0, 120, 8, 16, 0xffffff, buf);
	wsprintf(buf, "%6d %6d %6d %6d", 
		(int) (1000.0f * PLR_LocalPlayer->car.Body->Centre.WMatrix.m[YX]),
		(int) (1000.0f * PLR_LocalPlayer->car.Body->Centre.WMatrix.m[YY]),
		(int) (1000.0f * PLR_LocalPlayer->car.Body->Centre.WMatrix.m[YZ]),
		(int) (1000.0f * Length(&PLR_LocalPlayer->car.Body->Centre.WMatrix.mv[U])));
	DumpText(0, 140, 8, 16, 0xffffff, buf);
	wsprintf(buf, "%6d %6d %6d %6d", 
		(int) (1000.0f * PLR_LocalPlayer->car.Body->Centre.WMatrix.m[ZX]),
		(int) (1000.0f * PLR_LocalPlayer->car.Body->Centre.WMatrix.m[ZY]),
		(int) (1000.0f * PLR_LocalPlayer->car.Body->Centre.WMatrix.m[ZZ]),
		(int) (1000.0f * Length(&PLR_LocalPlayer->car.Body->Centre.WMatrix.mv[L])));
	DumpText(0, 160, 8, 16, 0xffffff, buf);

	// Show impulses
#if USE_DEBUG_ROUTINES
	wsprintf(buf, "imp: %6d %6d %6d  ang: %6d %6d %6d",
		(int) (100.0f * DEBUG_Impulse.v[X]),
		(int) (100.0f * DEBUG_Impulse.v[Y]),
		(int) (100.0f * DEBUG_Impulse.v[Z]),
		(int) (100.0f * DEBUG_AngImpulse.v[X]),
		(int) (100.0f * DEBUG_AngImpulse.v[Y]),
		(int) (100.0f * DEBUG_AngImpulse.v[Z]));
	DumpText(0, 180, 8, 16, 0xffffff, buf);
#endif

	// Show friction mode and coefficients
	wsprintf(buf, "SFric: %3d   KFric: %3d",
		(int) (100.0f * PLR_LocalPlayer->car.Body->Centre.StaticFriction),
		(int) (100.0f * PLR_LocalPlayer->car.Body->Centre.KineticFriction));
	DumpText(0, 200, 8, 16, 0xffffff, buf);

	// Wheel Info
	wsprintf(buf, "Wheels: Pos %4d %4d %4d %4d", 
		(int) (1000.0f * PLR_LocalPlayer->car.Wheel[0].Pos),
		(int) (1000.0f * PLR_LocalPlayer->car.Wheel[1].Pos),
		(int) (1000.0f * PLR_LocalPlayer->car.Wheel[2].Pos),
		(int) (1000.0f * PLR_LocalPlayer->car.Wheel[3].Pos));
	DumpText(0, 220, 8, 16, 0xffffff, buf);
	wsprintf(buf, "        Vel %4d %4d %4d %4d, %4d %4d %4d %4d", 
		(int) (PLR_LocalPlayer->car.Wheel[0].Vel),
		(int) (PLR_LocalPlayer->car.Wheel[1].Vel),
		(int) (PLR_LocalPlayer->car.Wheel[2].Vel),
		(int) (PLR_LocalPlayer->car.Wheel[3].Vel),
		(int) (PLR_LocalPlayer->car.Wheel[0].AngVel),
		(int) (PLR_LocalPlayer->car.Wheel[1].AngVel),
		(int) (PLR_LocalPlayer->car.Wheel[2].AngVel),
		(int) (PLR_LocalPlayer->car.Wheel[3].AngVel));
	DumpText(0, 240, 8, 16, 0xffffff, buf);
	wsprintf(buf, "        Frc %3d/%3d(%3d)  %3d/%3d(%3d)  %3d/%3d(%3d)  %3d/%3d(%3d)", 
		(int) (100.0f * PLR_LocalPlayer->car.Wheel[0].StaticFriction),
		(int) (100.0f * PLR_LocalPlayer->car.Wheel[0].KineticFriction),
		(int) (100.0f * PLR_LocalPlayer->car.Spring[0].Restitution),
		(int) (100.0f * PLR_LocalPlayer->car.Wheel[1].StaticFriction),
		(int) (100.0f * PLR_LocalPlayer->car.Wheel[1].KineticFriction),
		(int) (100.0f * PLR_LocalPlayer->car.Spring[1].Restitution),
		(int) (100.0f * PLR_LocalPlayer->car.Wheel[2].StaticFriction),
		(int) (100.0f * PLR_LocalPlayer->car.Wheel[2].KineticFriction),
		(int) (100.0f * PLR_LocalPlayer->car.Spring[2].Restitution),
		(int) (100.0f * PLR_LocalPlayer->car.Wheel[3].StaticFriction),
		(int) (100.0f * PLR_LocalPlayer->car.Wheel[3].KineticFriction),
		(int) (100.0f * PLR_LocalPlayer->car.Spring[3].Restitution));
	DumpText(0, 260, 8, 16, 0xffffff, buf);
	wsprintf(buf, "        C/S %1d/%1d  %1d/%1d  %1d/%1d  %1d/%1d",
		(IsWheelInContact(&PLR_LocalPlayer->car.Wheel[0]))? 1: 0,
		(IsWheelSkidding(&PLR_LocalPlayer->car.Wheel[0]))? 1: 0,
		(IsWheelInContact(&PLR_LocalPlayer->car.Wheel[1]))? 1: 0,
		(IsWheelSkidding(&PLR_LocalPlayer->car.Wheel[1]))? 1: 0,
		(IsWheelInContact(&PLR_LocalPlayer->car.Wheel[2]))? 1: 0,
		(IsWheelSkidding(&PLR_LocalPlayer->car.Wheel[2]))? 1: 0,
		(IsWheelInContact(&PLR_LocalPlayer->car.Wheel[3]))? 1: 0,
		(IsWheelSkidding(&PLR_LocalPlayer->car.Wheel[3]))? 1: 0);
	DumpText(0, 280, 8, 16, 0xffffff, buf);

	// Number of sparks active
	wsprintf(buf, "Sparks: %5d  Trails: %5d", NActiveSparks, NActiveTrails);
	DumpText(0, 300, 8, 16, 0xffffff, buf);

	// Size of ghost currently stored and being recorded
	wsprintf(buf, "Ghost: %8d (%8d)", GhostInfo->NFrames * sizeof(GHOST_INFO), GHO_BestGhostInfo->NFrames * sizeof(GHOST_INFO));
	DumpText(320, 300, 8, 16, 0xffffff, buf);

	// Equation solver tests
#if USE_DEBUG_ROUTINES
	wsprintf(buf, "Converged: %s   Res = %d", (DEBUG_Converged)? "Yes": "No ", (int)(1000000 * DEBUG_Res));
	DumpText(0, 320, 8, 16, 0xffffff, buf);
#endif

#if USE_DEBUG_ROUTINES
	wsprintf(buf, "Max Imp: %9d    Max Ang Imp: %9d", (int)DEBUG_MaxImpulseMag, (int)DEBUG_MaxAngImpulseMag);
	DumpText(0,340, 8, 16, 0xffffff, buf);
#endif

}
#endif
