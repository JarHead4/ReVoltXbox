
#include "revolt.h"
#include "panel.h"
#include "ctrlread.h"
#include "object.h"
#include "player.h"
#include "timing.h"
#include "geom.h"
#include "trigger.h"
#include "text.h"
#include "input.h"
#include "camera.h"

// globals

#if USE_DEBUG_ROUTINES
extern VEC DEBUG_DownForce;
#endif


long SpeedUnits = SPEED_MPH;

static long TrackDirType;
static long TrackDirCount = 0;
static long DisplaySplitTime;
static float DisplaySplitCount;
static long RevLit[REV_LIT_MAX];
static long WrongWayFlag;
static float WrongWayTimer;

static char *SpeedUnitText[SPEED_NTYPES] = {
	"mph",
	"fpm",
	"kph"
};
static REAL SpeedUnitScale[SPEED_NTYPES] = {
	OGU2MPH_SPEED,
	OGU2FPM_SPEED,
	OGU2KPH_SPEED
};

static MAP_INFO MapInfo[] = {
	{"MARKET1", 24.0f, 400.0f, 0.01f, 0.01f, 0.0f, 128.0f, 64.0f, 64.0f},

	{""},
};

// track dir UV's

static float TrackDirUV[] = {
	128, 0, -64, 64,	// chicane left
	64, 0, -64, 64,		// 180 left
	128, 64, -64, 62,	// 90 left
	256, 0, -64, 64,	// 45 left
	64, 0, 64, 64,		// chicane right
	0, 0, 64, 64,		// 180 right
	64, 64, 64, 62,		// 90 right
	192, 0, 64, 64,		// 45 right
	0, 64, 64, 62,		// ahead
	128, 0, 64, 64,		// danger
	192, 64, 64, 62,	// fork
};

// rev light UV's

static float RevsUV[] = {
	143, 128, 15, 14,
	119, 128, 16, 16,
	95, 128, 19, 18,
	71, 128, 19, 19,
	47, 128, 21, 21,
	23, 128, 22, 22,
	0, 128, 22, 24,
};

// rev light positions

static float RevsPositions[] = {
	492, 451,
	502, 435,
	517, 422,
	536, 412,
	558, 405,
	581, 400,
	606, 399,
};

// pickup UV's

static float PickupUV[] = {
	0, 224,		// shockwave
	64, 224,	// firework
	96, 224,	// firework pack
	32, 224,	// putty bomb
	128, 224,	// water bomb
	224, 224,	// electro pulse
	192, 224,	// oil slick
	0, 192,		// chrome ball
	32, 192,	// turbo
};

///////////////////
// split trigger //
///////////////////

void TriggerSplit(PLAYER *player, long flag, long n, VEC *vec)
{
	CAR	*car;

// ignore if wrong car

	if (player != PLR_LocalPlayer)
		return;

	car = &player->car;

// special end of lap split?

	if (n == -1)
	{
		DisplaySplitCount = SPLIT_COUNT;
		DisplaySplitTime = car->CurrentLapTime - TrackRecords.RecordLap[0].Time;
		car->NextSplit++;
	}

// ignore if gay number

	if (n >= MAX_SPLIT_TIMES)
		return;

// ignore if wrong split

	if (n != car->NextSplit)
		return;

// set car split time

	car->SplitTime[n] = car->CurrentLapTime;

// display split time

	DisplaySplitCount = SPLIT_COUNT;
	DisplaySplitTime = car->SplitTime[n] - TrackRecords.SplitTime[n];
	car->NextSplit++;
}

///////////////////////
// trigger track dir //
///////////////////////

void TriggerTrackDir(PLAYER *player, long flag, long n, VEC *vec)
{

// fuck off everyone else

	if (player != PLR_LocalPlayer)
		return;

// ignore if not first trigger

	if (!(flag & TRIGGER_GLOBAL_FIRST))
		return;

// set track dir flags

	TrackDirType = n;
	TrackDirCount = TRACK_DIR_COUNT;
}

/////////////////////
// revs pos editor //
/////////////////////

static void EditRevsPos(void)
{
	long i;
	char buf[128];
	static long edit = 0;

	if (Keys[DIK_MINUS] && !LastKeys[DIK_MINUS] && edit > 0) edit--;
	if (Keys[DIK_EQUALS] && !LastKeys[DIK_EQUALS] && edit < 6) edit++;

	if (Keys[DIK_LEFT] && !LastKeys[DIK_LEFT]) RevsPositions[edit * 2]--;
	if (Keys[DIK_RIGHT] && !LastKeys[DIK_RIGHT]) RevsPositions[edit * 2]++;

	if (Keys[DIK_UP] && !LastKeys[DIK_UP]) RevsPositions[edit * 2 + 1]--;
	if (Keys[DIK_DOWN] && !LastKeys[DIK_DOWN]) RevsPositions[edit * 2 + 1]++;

	SET_TPAGE(TPAGE_FONT);

	for (i = 0 ; i < 7 ; i++)
	{
		wsprintf(buf, "%d = %d, %d", i, (long)RevsPositions[i * 2], (long)RevsPositions[i * 2 + 1]);
		DumpText(0, i * 32, 24, 32, 0xffffffff, buf);
	}

	SET_TPAGE(TPAGE_FX2);
}

////////////////////////
// draw control panel //
////////////////////////

void DrawControlPanel(void)
{
	float x, y, xsize, ysize, tu, tv, twidth, theight, *p, frevs, dist;
	long i, col, speed, revs, revdest, revper, revadd, pickup, pickup2;
	char buf[128];
	PLAYER *player;

// skip if editing

	if (CAM_MainCamera->Type == CAM_EDIT)
		return;

// set misc render states

	ZBUFFER_OFF();

	ALPHA_ON();
	ALPHA_SRC(D3DBLEND_SRCALPHA);
	ALPHA_DEST(D3DBLEND_INVSRCALPHA);

// fx 1

	SET_TPAGE(TPAGE_FX1);

// position ring

	DrawPanelSprite(16, 368, 96, 96, 1.0f / 256.0f, 129.0f / 256.0f, 62.0f / 256.0f, 62.0f / 256.0f, 0xc0ffffff);
	for (player = PLR_PlayerHead ; player ; player = player->next)
	{
		dist = player->CarAI.FinishDistPanel * RAD;

		x = -(float)sin(dist) * 46.0f + 58.0f;
		y = -(float)cos(dist) * 46.0f + 412.0f;

		tu = 208.0f / 256.0f;
		tv = 216.0f / 256.0f;

		DrawPanelSprite(x, y, 10.0f, 10.0f, tu, tv, 8.0f / 256.0f, 8.0f / 256.0f, 0xc0ffffff);
	}

// fx 2

	SET_TPAGE(TPAGE_FX2);

// wrong way?

	if (WrongWayFlag != PLR_LocalPlayer->CarAI.WrongWay)
		WrongWayTimer += TimeStep;
	else
		WrongWayTimer = 0.0f;

	if (WrongWayTimer >= WRONG_WAY_TOLERANCE) WrongWayFlag = !WrongWayFlag;

	if (WrongWayFlag && TIME2MS(TimerCurrent) & 256)
	{
		DrawPanelSprite(256, 0, 128, 128, 128.0f / 256.0f, 64.0f / 256.0f, 64.0f / 256.0f, 64.0f / 256.0f, 0xc0ffffff);
	}

// track dir?

	if (TrackDirCount)
	{
		if (!WrongWayFlag)
		{
			p = &TrackDirUV[TrackDirType * 4];
			tu = *p++ / 256.0f;
			tv = *p++ / 256.0f;
			twidth = *p++ / 256.0f;
			theight = *p++ / 256.0f;

			if (TrackDirCount > TRACK_DIR_COUNT - TRACK_DIR_FADE_COUNT) col = (TRACK_DIR_COUNT - TrackDirCount) << 25;
			else if (TrackDirCount < TRACK_DIR_FADE_COUNT) col = (TrackDirCount) << 25;
			else col = 255 << 24;
			col |= 0xffffff;

			DrawPanelSprite(256, 0, 128, 128, tu, tv, twidth, theight, col);
		}

		if ((TrackDirCount -= (long)(TimeFactor * 10)) < 0) TrackDirCount = 0;
	}

// revs

	frevs = abs(PLR_LocalPlayer->car.Revs / PLR_LocalPlayer->car.MaxRevs);
	FTOL(frevs, revs);
	FTOL((frevs - (float)revs) * REV_LIT_MAX, revper);
	FTOL(TimeStep * 1024, revadd);

	for (i = 0 ; i < REV_LIT_NUM ; i++)
	{
		if (revs > i) revdest = REV_LIT_MAX;
		else if (revs < i) revdest = 0;
		else revdest = revper;

		if (RevLit[i] < revdest)
		{
			RevLit[i] += revadd * 4;
			if (RevLit[i] > REV_LIT_MAX) RevLit[i] = REV_LIT_MAX;
		}
		else if (RevLit[i] > revdest)
		{
			RevLit[i] -= revadd;
			if (RevLit[i] < 0) RevLit[i] = 0;
		}
	}

	p = RevsUV;

	for (i = 0 ; i < REV_LIT_NUM ; i++)
	{
		tu = *p++ / 256.0f;
		tv = *p++ / 256.0f;
		twidth = *p++ / 256.0f;
		theight = *p++ / 256.0f;

		x = RevsPositions[i * 2];
		y = RevsPositions[i * 2 + 1];

		xsize = twidth * 256;
		ysize = theight * 256;

		if (!RevLit[i])
			DrawPanelSprite(x, y, xsize, ysize, tu, tv, twidth, theight, 0xc0ffffff);
		else if (RevLit[i] == REV_LIT_MAX)
			DrawPanelSprite(x, y, xsize, ysize, tu, tv + (25.0f / 256.0f), twidth, theight, 0xc0ffffff);
		else
		{
			col = RevLit[i] * 0xc0 / REV_LIT_MAX;
			DrawPanelSprite(x, y, xsize, ysize, tu, tv, twidth, theight, ((0xc0 - col) << 24) | 0xffffff);
			DrawPanelSprite(x, y, xsize, ysize, tu, tv + (25.0f / 256.0f), twidth, theight, (col << 24) | 0xffffff);
		}
	}

// pickup

	DrawPanelSprite(4, 56, 64, 64, 190.0f / 256.0f, 157.0f / 256.0f, 64.0f / 256.0f, 64.0f / 256.0f, 0xe0ffffff);

	if (PLR_LocalPlayer->PickupCycleSpeed)
	{
		pickup = (long)PLR_LocalPlayer->PickupCycleType;
		pickup2 = (pickup + 1) % PICKUP_NUM;

		FTOL((PLR_LocalPlayer->PickupCycleType - (float)pickup) * 255.0f, col);
		col |= (col << 8) | (col << 16);
	}
	else if (PLR_LocalPlayer->PickupNum)
	{
		pickup = PLR_LocalPlayer->PickupType;
		col = 0;
	}
	else
	{
		pickup = -1;
	}

	if (pickup != -1)
	{
		ALPHA_SRC(D3DBLEND_ONE);
		ALPHA_DEST(D3DBLEND_ONE);

		tu = (PickupUV[pickup * 2]) / 256.0f;
		tv = (PickupUV[pickup * 2 + 1]) / 256.0f;
		DrawPanelSprite(20, 72, 32, 32, tu, tv, 32.0f / 256.0f, 32.0f / 256.0f, 0xffffff - col);

		if (col)
		{
			tu = (PickupUV[pickup2 * 2]) / 256.0f;
			tv = (PickupUV[pickup2 * 2 + 1]) / 256.0f;
			DrawPanelSprite(20, 72, 32, 32, tu, tv, 32.0f / 256.0f, 32.0f / 256.0f, col);
		}

		ALPHA_SRC(D3DBLEND_SRCALPHA);
		ALPHA_DEST(D3DBLEND_INVSRCALPHA);
	}

// big font

	SET_TPAGE(TPAGE_BIGFONT);

// countdown?

	if (CountdownTime && CountdownTime < (1000 * 3))
	{
		wsprintf(buf, "%d", (CountdownTime / 1000 + 1));
		DumpBigText(224, 0, 192, 256, 0xc0ffffff, buf);
	}

// normal font

	SET_TPAGE(TPAGE_FONT);

// split time?

	if (DisplaySplitCount)
	{
		DisplaySplitCount -= TimeStep;
		if (DisplaySplitCount < 0) DisplaySplitCount = 0;

		DumpText(290, 128, 12, 16, 0xc000ffff, "split");
		if (DisplaySplitTime > 0) wsprintf(buf, "+%02d:%02d:%03d", MINUTES(DisplaySplitTime), SECONDS(DisplaySplitTime), THOUSANDTHS(DisplaySplitTime));
		else wsprintf(buf, "-%02d:%02d:%03d", MINUTES(-DisplaySplitTime), SECONDS(-DisplaySplitTime), THOUSANDTHS(-DisplaySplitTime));
		DumpText(260, 144, 12, 16, 0xc0ffffff, buf);
	}

// lap time

	DumpText(588, 96, 12, 16, 0xc000ffff, "lap");
	wsprintf(buf, "%02d:%02d:%03d", MINUTES(PLR_LocalPlayer->car.CurrentLapTime), SECONDS(PLR_LocalPlayer->car.CurrentLapTime), THOUSANDTHS(PLR_LocalPlayer->car.CurrentLapTime));
	DumpText(516, 112, 12, 16, 0xc0ffffff, buf);

// race time

	DumpText(576, 136, 12, 16, 0xc000ffff, "race");
	wsprintf(buf, "%02d:%02d:%03d", MINUTES(TotalRaceTime), SECONDS(TotalRaceTime), THOUSANDTHS(TotalRaceTime));
	DumpText(516, 152, 12, 16, 0xc0ffffff, buf);

// speed

	DumpText(588, 448, 12, 16, 0xc000ffff, SpeedUnitText[SpeedUnits]);
	speed = (long)(SpeedUnitScale[SpeedUnits] * VecLen(&PLR_LocalPlayer->car.Body->Centre.Vel));
	wsprintf(buf, "%4d", speed);
	DumpText(528, 448, 12, 16, 0xc0ffffff, buf);

// best acc
//	wsprintf(buf, "0-15: %3dms", (PLR_LocalPlayer->car.Best0to15 > ZERO)? (int)(1000 * PLR_LocalPlayer->car.Best0to15): 0);
//	DumpText(100, 50, 12, 16, 0xc0ffffff, buf);
//	wsprintf(buf, "0-25: %3dms", (PLR_LocalPlayer->car.Best0to25 > ZERO)? (int)(1000 * PLR_LocalPlayer->car.Best0to25): 0);
//	DumpText(100, 70, 12, 16, 0xc0ffffff, buf);
//	wsprintf(buf, "ValidNode: %d (%d)", PLR_LocalPlayer->ValidRailCamNode, (PLR_LocalPlayer->ValidRailCamNode == -1)? -1: CAM_CameraNode[PLR_LocalPlayer->ValidRailCamNode].ID);
//	DumpText(100, 50, 12, 16, 0xffff0000, buf);
//	wsprintf(buf, "BangMag: %d", (int)(100 * PLR_LocalPlayer->car.Body->BangMag));
//	DumpText(100, 50, 12, 16, 0xffff0000, buf);
//	wsprintf(buf, "Down: %8d %8d %8d (%8d)", (int)(100 * DEBUG_DownForce.v[X]), (int)(100 * DEBUG_DownForce.v[Y]), (int)(100 * DEBUG_DownForce.v[Z]), (int)(100 * PLR_LocalPlayer->car.DownForceMod));
//	DumpText(100, 50, 8, 12, 0xffff0000, buf);

// laps

	DumpText(16, 16, 12, 16, 0xc000ffff, "lap");
	wsprintf(buf, "%d/%d", PLR_LocalPlayer->car.Laps + 1, 5);
	DumpText(16, 32, 12, 16, 0xc0ffffff, buf);

// position

	if (GameSettings.GameType != GAMETYPE_TRIAL)
	{
		DumpText(58, 416, 12, 16, 0xc0ffffff, "1");
	}

// last lap

	DumpText(528, 16, 12, 16, 0xc000ffff, "last lap");
	wsprintf(buf, "%02d:%02d:%03d", MINUTES(PLR_LocalPlayer->car.LastLapTime), SECONDS(PLR_LocalPlayer->car.LastLapTime), THOUSANDTHS(PLR_LocalPlayer->car.LastLapTime));
	DumpText(516, 32, 12, 16, 0xc0ffffff, buf);

// best lap

	DumpText(528, 56, 12, 16, 0xc000ffff, "best lap");
	wsprintf(buf, "%02d:%02d:%03d", MINUTES(PLR_LocalPlayer->car.BestLapTime), SECONDS(PLR_LocalPlayer->car.BestLapTime), THOUSANDTHS(PLR_LocalPlayer->car.BestLapTime));
	DumpText(516, 72, 12, 16, 0xc0ffffff, buf);

// bump test

return;
	VEC vec;
	static VEC light = {0, 0, -128};
	static VEC tl = {-64, -64, 0};
	static VEC tr = {64, -64, 0};
	static VEC br = {64, 64, 0};
	static VEC bl = {-64, 64, 0};
	static float tu0, tv0, tu1, tv1, tu2, tv2, tu3, tv3;

	if (Keys[DIK_LEFT]) light.v[X]--;
	if (Keys[DIK_RIGHT]) light.v[X]++;
	if (Keys[DIK_UP]) light.v[Y]--;
	if (Keys[DIK_DOWN]) light.v[Y]++;

	SubVector(&tl, &light, &vec);
	NormalizeVector(&vec);
	tu0 = 0.0f / 256.0f - vec.v[X] / 64.0f;
	tv0 = 0.0f / 256.0f - vec.v[Y] / 64.0f;

	SubVector(&tr, &light, &vec);
	NormalizeVector(&vec);
	tu1 = 64.0f / 256.0f - vec.v[X] / 64.0f;
	tv1 = 0.0f / 256.0f - vec.v[Y] / 64.0f;

	SubVector(&br, &light, &vec);
	NormalizeVector(&vec);
	tu2 = 64.0f / 256.0f - vec.v[X] / 64.0f;
	tv2 = 64.0f / 256.0f - vec.v[Y] / 64.0f;

	SubVector(&bl, &light, &vec);
	NormalizeVector(&vec);
	tu3 = 0.0f / 256.0f - vec.v[X] / 64.0f;
	tv3 = 64.0f / 256.0f - vec.v[Y] / 64.0f;

	DrawVertsTEX2[0].sx = 128;
	DrawVertsTEX2[0].sy = 128;
	DrawVertsTEX2[0].sz = 0.01f;
	DrawVertsTEX2[0].rhw = 1;
	DrawVertsTEX2[0].color = 0x808080;
	DrawVertsTEX2[0].tu = 0.0f / 256.0f;
	DrawVertsTEX2[0].tv = 0.0f / 256.0f;

	DrawVertsTEX2[1].sx = 256;
	DrawVertsTEX2[1].sy = 128;
	DrawVertsTEX2[1].sz = 0.01f;
	DrawVertsTEX2[1].rhw = 1;
	DrawVertsTEX2[1].color = 0x808080;
	DrawVertsTEX2[1].tu = 64.0f / 256.0f;
	DrawVertsTEX2[1].tv = 0.0f / 256.0f;

	DrawVertsTEX2[2].sx = 256;
	DrawVertsTEX2[2].sy = 256;
	DrawVertsTEX2[2].sz = 0.01f;
	DrawVertsTEX2[2].rhw = 1;
	DrawVertsTEX2[2].color = 0x808080;
	DrawVertsTEX2[2].tu = 64.0f / 256.0f;
	DrawVertsTEX2[2].tv = 64.0f / 256.0f;

	DrawVertsTEX2[3].sx = 128;
	DrawVertsTEX2[3].sy = 256;
	DrawVertsTEX2[3].sz = 0.01f;
	DrawVertsTEX2[3].rhw = 1;
	DrawVertsTEX2[3].color = 0x808080;
	DrawVertsTEX2[3].tu = 0.0f / 256.0f;
	DrawVertsTEX2[3].tv = 64.0f / 256.0f;

	DrawVertsTEX2[0].tu2 = tu0;
	DrawVertsTEX2[0].tv2 = tv0;

	DrawVertsTEX2[1].tu2 = tu1;
	DrawVertsTEX2[1].tv2 = tv1;

	DrawVertsTEX2[2].tu2 = tu2;
	DrawVertsTEX2[2].tv2 = tv2;

	DrawVertsTEX2[3].tu2 = tu3;
	DrawVertsTEX2[3].tv2 = tv3;

	ALPHA_OFF();
	SET_TPAGE(TPAGE_FX1);
	SET_TPAGE2(TPAGE_FX1);

	SET_STAGE_STATE(1, D3DTSS_TEXCOORDINDEX, 1);
	SET_STAGE_STATE(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	SET_STAGE_STATE(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
	SET_STAGE_STATE(1, D3DTSS_COLOROP, D3DTOP_SUBTRACT);

	D3Ddevice->DrawPrimitive(D3DPT_TRIANGLEFAN, FVF_TEX2, DrawVertsTEX2, 4, D3DDP_DONOTUPDATEEXTENTS);

	SET_STAGE_STATE(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
}

/////////////////////////////////
// draw a control panel sprite //
/////////////////////////////////

void DrawPanelSprite(float x, float y, float width, float height, float tu, float tv, float twidth, float theight, long rgba)
{
	long i;
	float xstart, ystart, xsize, ysize;

// scale

	xstart = x * RenderSettings.GeomScaleX + ScreenLeftClip;
	ystart = y * RenderSettings.GeomScaleY + ScreenTopClip;

	xsize = width * RenderSettings.GeomScaleX;
	ysize = height * RenderSettings.GeomScaleY;

// init vert misc

	for (i = 0 ; i < 4 ; i++)
	{
		DrawVertsTEX1[i].color = rgba;
		DrawVertsTEX1[i].rhw = 1;
	}

// set screen coors

	DrawVertsTEX1[0].sx = DrawVertsTEX1[3].sx = xstart;
	DrawVertsTEX1[1].sx = DrawVertsTEX1[2].sx = xstart + xsize;
	DrawVertsTEX1[0].sy = DrawVertsTEX1[1].sy = ystart;
	DrawVertsTEX1[2].sy = DrawVertsTEX1[3].sy = ystart + ysize;

// set uv's

	DrawVertsTEX1[0].tu = DrawVertsTEX1[3].tu = tu;
	DrawVertsTEX1[1].tu = DrawVertsTEX1[2].tu = tu + twidth;
	DrawVertsTEX1[0].tv = DrawVertsTEX1[1].tv = tv;
	DrawVertsTEX1[2].tv = DrawVertsTEX1[3].tv = tv + theight;

// draw

	D3Ddevice->DrawPrimitive(D3DPT_TRIANGLEFAN, FVF_TEX1, DrawVertsTEX1, 4, D3DDP_DONOTUPDATEEXTENTS | D3DDP_DONOTCLIP);
}
