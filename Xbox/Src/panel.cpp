//-----------------------------------------------------------------------------
// File: panel.cpp
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------

//$TODO(cprince): I think there are lots more places in this file where they
/// use screen-space coords, but they don't do the necessary half-pixel offset.
/// Need to look into this.

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
#include "posnode.h"
#include "obj_init.h"
#include "ghost.h"
#include "ai.h"
#include "pickup.h"
#include "InitPlay.h"
#include "settings.h"
#include "cheats.h"
#include "ui_MenuDraw.h"
#include "ui_MenuText.h"
#include "ui_TitleScreen.h"
#include "ui_StateEngine.h"
#include "ui_WaitingRoom.h"

// globals

static void DrawChampionshipTable(void);
static void DrawChampionshipFailed(void);

#if USE_DEBUG_ROUTINES
extern VEC DEBUG_DownForce;
extern long DBG_CtrlCurrent;
extern long DBG_PosCurrent;
extern unsigned long ReplayBufferBytesStores, ReplayDataBufSize;
extern REAL time0, time1, time2, timeNow;
#endif

extern unsigned long NPhysicsLoops;
extern int  COL_NBodyBodyTests;

long SpeedUnits = SPEED_MPH;
long PracticeStarFlash;
long WrongWayFlag;
float WrongWayTimer;

static long RevLit[REV_LIT_MAX];
static MODEL CountdownModels[4];
static MODEL DrumModel;
static long LocalRacePos;
static float LocalRacePosScale;
static float ConsoleTimer, ConsoleStartTime;
static long ConsolePri;
static long ConsoleRGB1;
static long ConsoleRGB2;
static WCHAR ConsoleMessage1[CONSOLE_MESSAGE_MAX];
static WCHAR ConsoleMessage2[CONSOLE_MESSAGE_MAX];
static float ChampionshipTableOffset, RaceTableOffset;

static int SpeedUnitText[SPEED_NTYPES] = 
{
    TEXT_MPH,
    TEXT_MPH,
    TEXT_FPM,
    TEXT_KPH,
    TEXT_KPH,
};

static FLOAT SpeedUnitScale[SPEED_NTYPES] = 
{
    OGU2MPH_SPEED,
    OGU2MPH_SPEED * 10,
    OGU2FPM_SPEED,
    OGU2KPH_SPEED,
    OGU2KPH_SPEED * 10,
};

//static char CongratsText[] = "CONGRATULATIONS!  YOU QUALIFIED FOR THE NEXT RACE!";
//static char CongratsText2[] = "CONGRATULATIONS!  YOU FINISHED THIS CHAMPIONSHIP!";
//static char FailedText[] = "YOU FAILED TO QUALIFY!";

// champ table sizes

#define CHAMP_TABLE_NAME_WIDTH 128
#define CHAMP_TABLE_POS_WIDTH 56
#define CHAMP_TABLE_PTS_WIDTH 56

// countdown pos offset

static VEC CountdownOffset = {0, -32.0f, 256.0f};

// challenge shit

#define CHALLENGE_FLASH_TIME 2000

long ChallengeFlash;

// drum shit

#define DRUM_XRES 30
#define DRUM_YRES 66
#define DRUM_POLY_NUM (DRUM_XRES * DRUM_YRES)
#define DRUM_ONE_ROT (1.0f / DRUM_YRES)

struct CREDIT
{
    long rgb1;
    long rgb2;
    long len;
    char text[4];
};

static float DrumRot, DrumRotNext;
static VEC DrumPos = {0, 0.0f, 283.0f};
static char *DrumCredits;
static long DrumCreditNum, DrumCurrentCreditNum, DrumRotPos;
CREDIT *DrumCurrentCredit;

// track dir UV's

static float TrackDirUV[] = {
    127, 1, -62, 62,    // chicane left
    63, 1, -62, 62,     // 180 left
    126, 65, -61, 62,   // 90 left
    255, 1, -62, 62,    // 45 left
    65, 1, 62, 62,      // chicane right
    1, 1, 62, 62,       // 180 right
    65, 65, 62, 62,     // 90 right
    193, 1, 62, 62,     // 45 right
    1, 65, 62, 62,      // ahead
    129, 1, 62, 62,     // danger
    193, 65, 62, 62,    // fork
};

// rev light UV's

static float RevsUV[] = 
{
    143, 128, 15, 14,
    119, 128, 16, 16,
    95, 128, 19, 18,
    71, 128, 19, 19,
    47, 128, 21, 21,
    23, 128, 22, 22,
    0, 128, 22, 24,
};

// rev light positions

static float RevsPositions[] = 
{
    492, 466,
    504, 449,
    521, 435,
    542, 424,
    566, 416,
    591, 410,
    618, 408,
};


// pickup UV's
static float PickupUV[] = 
{
      0, 224,    // Shockwave
     64, 225,    // Firework
     96, 224,    // Firework pack
     32, 224,    // Putty bomb
    128, 224,    // Water bomb
    224, 224,    // Electro pulse
    192, 224,    // Oil slick
      0, 192,    // Chrome ball
     32, 192,    // Turbo
    160, 224,    // Clone
     96, 192,    // Global
};


// Multiplayer colors
// Note: These colors are now NTSC-safe, so please keep 'em that way
long MultiPlayerColours[] = 
{
    0x00eb1010,
    0x0010eb10,
    0x001010eb,

    0x00eb10eb,
    0x0010ebeb,
    0x00ebeb10,

    0x00eb107d,
    0x007deb10,
    0x00107deb,

    0x00eb7deb,
    0x007debeb,
    0x00ebeb7d,
};


///////////////////
// split trigger //
///////////////////

void TriggerSplit(PLAYER *player, long flag, long n, PLANE *planes)
{
    CAR *car = &player->car;

// quit if not time trial

    if (GameSettings.GameType != GAMETYPE_TRIAL)
        return;

// special end of lap split?

    if (n == -1)
    {
        if (GHO_GhostDataExists && GHO_BestGhostInfo)
        {
            player->DisplaySplitCount = SPLIT_COUNT;
//          player->DisplaySplitTime = car->CurrentLapTime - TrackRecords.RecordLap[0].Time;
            player->DisplaySplitTime = car->CurrentLapTime - GHO_BestGhostInfo->Time[MAX_SPLIT_TIMES];
        }
        return;
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

    if (GHO_GhostDataExists && GHO_BestGhostInfo)
    {
        player->DisplaySplitCount = SPLIT_COUNT;
//      player->DisplaySplitTime = car->SplitTime[n] - TrackRecords.SplitTime[n];
        player->DisplaySplitTime = car->SplitTime[n] - GHO_BestGhostInfo->Time[n];
    }
    car->NextSplit++;
}

///////////////////////
// trigger track dir //
///////////////////////

void TriggerTrackDir(PLAYER *player, long flag, long n, PLANE *planes)
{
    CAR *car = &player->car;

// ignore if wrong track dir

    if ((n >> 16) != car->NextTrackDir)
        return;

// set track dir flags

    if ((n & 0xffff) != 11)
    {
        player->TrackDirType = (n & 0xffff);
        player->TrackDirCount = TRACK_DIR_COUNT;
    }

// inc next track dir count

    car->NextTrackDir++;
}

/////////////////////
// revs pos editor //
/////////////////////

static void EditRevsPos(void)
{
    long i;
    WCHAR buf[128];
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
        swprintf(buf, L"%d = %d, %d", i, (long)RevsPositions[i * 2], (long)RevsPositions[i * 2 + 1]);
        DumpText(0, i * 32, 24, 32, MENU_TEXT_RGB_NORMAL, buf);
    }

    SET_TPAGE(TPAGE_FX2);
}

////////////////////////
// draw control panel //
////////////////////////

void DrawControlPanel(void)
{
    float x, y, xsize, ysize, tu, tv, twidth, theight, *p, frevs, dist, nearestdistahead, nearestdistbehind, localdist, f;
    long i, j, col, speed, revs, revdest, revper, revadd, pickup, pickup2, pos, time;
    unsigned long lastping;
    WCHAR buf[128];
    PLAYER *player, *playerahead, *playerbehind;
    VEC vec, vec2;
//$ADDITION: to support safe areas
    //$TODO: need to verify SafeArea offsets are added to *ALL* positions in function calls here!

    //$REVISIT: are these the correct values?
    //$REVISIT: should these values change depending on resolution, regular-vs-HDTV, etc?
    FLOAT fSafeAreaLeft   = +48.0f;
    FLOAT fSafeAreaRight  = -48.0f;
    FLOAT fSafeAreaTop    = +36.0f - 16.0f;
    FLOAT fSafeAreaBottom = -36.0f;
    FLOAT fSafeAreaCenter =   0.0f;  //$REVISIT: should check things that are using this value (or using none of these values)
    int SafeAreaLeft   = +48 ;
    int SafeAreaRight  = -48;
    int SafeAreaTop    = +36 - 16;
    int SafeAreaBottom = -36;
    int SafeAreaCenter = 0;  //$REVISIT: should check things that are using this value (or using none of these values)
//$END_ADDITION

// skip if editing

    if (CAM_MainCamera->Type == CAM_EDIT)
        return;

// zbuffer off

    ZBUFFER_OFF();

// draw spru boxes

    if (GameSettings.GameType != GAMETYPE_PRACTICE && GameSettings.GameType != GAMETYPE_TRAINING)
        DrawSpruBox( 52, 394, 200, 46, 0, 0 );

    if( PLR_LocalPlayer->RaceFinishTime && ( ChampionshipEndMode == CHAMPIONSHIP_END_FINISHED || ChampionshipEndMode == CHAMPIONSHIP_END_WAITING_FOR_FINISH || ChampionshipEndMode == CHAMPIONSHIP_END_QUALIFIED || ChampionshipEndMode == CHAMPIONSHIP_END_FAILED ) )
    {
        FLOAT x, y, xsize, ysize;

        if( GameSettings.GameType == GAMETYPE_CLOCKWORK )
        {
            xsize = (248.0f+10.0f);
            ysize = (30.0f + 8.0f + 20.0f * StartData.PlayerNum);
            x     = (((640.0f - xsize)/2.0f) );
            y     = 60.0f + RaceTableOffset;
        }
        else
        {
            xsize = (248.0f+18.0f);
            ysize = (20.0f + 28.0f + 20.0f * StartData.PlayerNum);
            x     = (((640.0f - xsize)/2.0f) );
            y     = 150.0f + RaceTableOffset;
        }

        // Draw box for the race results
        DrawSpruBox( RenderSettings.GeomScaleX * x,
                     RenderSettings.GeomScaleY * y,
                     RenderSettings.GeomScaleX * xsize,
                     RenderSettings.GeomScaleY * ysize,
                     0, 0 );
    }

    if (GameSettings.GameType == GAMETYPE_CHAMPIONSHIP && (ChampionshipEndMode == CHAMPIONSHIP_END_QUALIFIED || ChampionshipEndMode == CHAMPIONSHIP_END_MENU) && PLR_LocalPlayer->RaceFinishPos < CUP_QUALIFY_POS)
    {
        DrawSpruBox(
            ((640 - (CHAMP_TABLE_NAME_WIDTH + (CupTable.RaceNum + 1) * CHAMP_TABLE_POS_WIDTH + CHAMP_TABLE_PTS_WIDTH)) / 2 - 4.0f + ChampionshipTableOffset) * RenderSettings.GeomScaleX,
            (176.0f - 4.0f) * RenderSettings.GeomScaleY,
            ((CHAMP_TABLE_NAME_WIDTH + (CupTable.RaceNum + 1) * CHAMP_TABLE_POS_WIDTH + CHAMP_TABLE_PTS_WIDTH) + 20.0f) * RenderSettings.GeomScaleX,
            (160.0f + 8.0f) * RenderSettings.GeomScaleY,
            0, 0);

        if (CupTable.RaceNum == CupData[CupTable.CupType].NRaces - 1)
            xsize = (float)(wcslen(TEXT_TABLE(TEXT_CHAMP_FINISHED1)) > wcslen(TEXT_TABLE(TEXT_CHAMP_FINISHED2)) ? wcslen(TEXT_TABLE(TEXT_CHAMP_FINISHED1)) : wcslen(TEXT_TABLE(TEXT_CHAMP_FINISHED2)));
        else
            xsize = (float)(wcslen(TEXT_TABLE(TEXT_CHAMP_QUALIFIED1)) > wcslen(TEXT_TABLE(TEXT_CHAMP_QUALIFIED2)) ? wcslen(TEXT_TABLE(TEXT_CHAMP_QUALIFIED1)) : wcslen(TEXT_TABLE(TEXT_CHAMP_QUALIFIED2)));
        
        DrawSpruBox(
            ((640 - xsize * 8.0f) / 2 - 4.0f - ChampionshipTableOffset + 8) * RenderSettings.GeomScaleX,
            96 * RenderSettings.GeomScaleY,
            (xsize * 8.0f + 8.0f) * RenderSettings.GeomScaleX,
            40.0f * RenderSettings.GeomScaleY,
            0, 0);
    }

    if (GameSettings.GameType == GAMETYPE_CHAMPIONSHIP && (ChampionshipEndMode == CHAMPIONSHIP_END_FAILED || ChampionshipEndMode == CHAMPIONSHIP_END_MENU) && PLR_LocalPlayer->RaceFinishPos >= CUP_QUALIFY_POS && Players[0].RaceFinishPos != CRAZY_FINISH_POS)
    {
        DrawSpruBox(
            ((640 - wcslen(TEXT_TABLE(TEXT_CHAMP_FAILURE)) * 8.0f) / 2 - 4.0f - ChampionshipTableOffset + 8) * RenderSettings.GeomScaleX,
            96 * RenderSettings.GeomScaleY,
            (wcslen(TEXT_TABLE(TEXT_CHAMP_FAILURE)) * 8.0f + 8.0f) * RenderSettings.GeomScaleX,
            24.0f * RenderSettings.GeomScaleY,
            0, 0);
    }

    if (ChallengeFlash != CHALLENGE_FLASH_TIME && ChallengeFlash & 128)
    {
        DrawSpruBox(
            (CENTRE_POS(TEXT_PANEL_CHALLENGE_BEATEN) - 8) * RenderSettings.GeomScaleX,
            232 * RenderSettings.GeomScaleY,
            (wcslen(TEXT_TABLE(TEXT_PANEL_CHALLENGE_BEATEN)) * 8 + 16) * RenderSettings.GeomScaleX,
            24 * RenderSettings.GeomScaleY,
            0, 0);
    }

// alpha

    BLEND_ALPHA();
    BLEND_SRC(D3DBLEND_SRCALPHA);
    BLEND_DEST(D3DBLEND_INVSRCALPHA);

// fx 1

    SET_TPAGE(TPAGE_FX1);

// stunt arena stars

    if (GameSettings.GameType == GAMETYPE_TRAINING)
    {
        for (i = 0 ; i < StarList.NumTotal ; i++)
        {
            for (j = 0 ; j < StarList.NumFound ; j++)
            {
                if (StarList.ID[j] == i)
                    break;
            }
            if (j == StarList.NumFound)
                DrawPanelSprite((float)i * 20 + 16 + SafeAreaLeft, 16+(float)SafeAreaTop, 16, 16, 65.0f / 256.0f, 241.0f / 256.0f, 14.0f / 256.0f, 14.0f / 256.0f, 0xc0ffffff);
            else
                DrawPanelSprite((float)i * 20 + 16 + SafeAreaLeft, 16+(float)SafeAreaTop, 16, 16, 81.0f / 256.0f, 241.0f / 256.0f, 14.0f / 256.0f, 14.0f / 256.0f, 0xc0ffffff);
        }
    }

// position ring

/*  DrawPanelSprite(16, 368, 96, 96, 1.0f / 256.0f, 129.0f / 256.0f, 62.0f / 256.0f, 62.0f / 256.0f, 0xc0ffffff);
    for (player = PLR_PlayerHead ; player ; player = player->next)
    {
        dist = player->CarAI.FinishDistPanel * RAD;

        x = -(float)sin(dist) * 46.0f + 58.0f;
        y = -(float)cos(dist) * 46.0f + 412.0f;

        tu = 193.0f / 256.0f;
        tv = 49.0f / 256.0f;

        DrawPanelSprite(x, y, 10.0f, 10.0f, tu, tv, 8.0f / 256.0f, 8.0f / 256.0f, MultiPlayerColours[player->Slot] | 0xf0000000);
    }*/

// fx 2

    SET_TPAGE(TPAGE_FX2);

// wrong way?

    if (GameSettings.GameType != GAMETYPE_PRACTICE && GameSettings.GameType != GAMETYPE_TRAINING)
    {
        if (WrongWayFlag != PLR_LocalPlayer->CarAI.WrongWay)
            WrongWayTimer += TimeStep;
        else
            WrongWayTimer = 0.0f;

        if (WrongWayTimer >= WRONG_WAY_TOLERANCE)
            WrongWayFlag = !WrongWayFlag;

        if (WrongWayFlag && TIME2MS(TimerCurrent) & 256 && !GameSettings.Paws && !PLR_LocalPlayer->RaceFinishTime && !CountdownTime)
        {
            DrawPanelSprite(288+(float)SafeAreaCenter, 16+(float)SafeAreaTop, 64, 64, 129.0f / 256.0f, 65.0f / 256.0f, 62.0f / 256.0f, 62.0f / 256.0f, 0xc0ffffff);

//          if (!(TIME2MS(TimerLast) & 256))
//              PlaySfx(SFX_WRONGWAY, SFX_MAX_VOL, SFX_CENTRE_PAN, 16384, 2);
        }
    }

// track dir?

    if (GameSettings.GameType != GAMETYPE_PRACTICE  && GameSettings.GameType != GAMETYPE_TRAINING && PLR_LocalPlayer->TrackDirCount && (PLR_LocalPlayer->RaceFinishTime == 0))
    {
        if (!WrongWayFlag)
        {
            p = &TrackDirUV[PLR_LocalPlayer->TrackDirType * 4];
            tu = *p++ / 256.0f;
            tv = *p++ / 256.0f;
            twidth = *p++ / 256.0f;
            theight = *p++ / 256.0f;

            if (PLR_LocalPlayer->TrackDirCount > TRACK_DIR_COUNT - TRACK_DIR_FADE_COUNT) col = (TRACK_DIR_COUNT - PLR_LocalPlayer->TrackDirCount) << 25;
            else if (PLR_LocalPlayer->TrackDirCount < TRACK_DIR_FADE_COUNT) col = (PLR_LocalPlayer->TrackDirCount) << 25;
            else col = 255 << 24;
            col |= 0xffffff;

            if (GameSettings.Mirrored)
            {
                tu += twidth;
                twidth = -twidth;
            }

            DrawPanelSprite(288+(float)SafeAreaCenter, 16+(float)SafeAreaTop, 64, 64, tu, tv, twidth, theight, col);
        }

        if ((PLR_LocalPlayer->TrackDirCount -= (long)(TimeStep * 720)) < 0) PLR_LocalPlayer->TrackDirCount = 0;
    }

// normal game dir arrow!

    if (GameSettings.GameType != GAMETYPE_PRACTICE  && GameSettings.GameType != GAMETYPE_TRAINING && GameSettings.GameType != GAMETYPE_NETWORK_BATTLETAG)
    {
        for (i = 0 ; i < 4 ; i++)
        {
            DrawVertsTEX1[i].sx = SafeAreaLeft   + (-(float)sin(PLR_LocalPlayer->CarAI.AngleToNextNode + ((float)i / 4.0f * RAD)) * 32.0f * (GameSettings.Mirrored ? -1.0f : 1.0f) + 96.0f) * RenderSettings.GeomScaleX + ScreenLeftClip;
            DrawVertsTEX1[i].sy = SafeAreaBottom + ((float)cos(PLR_LocalPlayer->CarAI.AngleToNextNode + ((float)i / 4.0f * RAD)) * 32.0f + 396.0f) * RenderSettings.GeomScaleY + ScreenTopClip;
            DrawVertsTEX1[i].sz = 0; //$ADDITION: explicitly set z (even though z-buffer off) to avoid near/far clipping
            DrawVertsTEX1[i].rhw = 1.0f;
            DrawVertsTEX1[i].color = 0xc0ffffff;
        }

        DrawVertsTEX1[0].tu = DrawVertsTEX1[3].tu = 2.0f / 256.0f;
        DrawVertsTEX1[1].tu = DrawVertsTEX1[2].tu = 62.0f / 256.0f;
        DrawVertsTEX1[0].tv = DrawVertsTEX1[1].tv = 66.0f / 256.0f;
        DrawVertsTEX1[2].tv = DrawVertsTEX1[3].tv = 126.0f / 256.0f;

        DRAW_PRIM(D3DPT_TRIANGLEFAN, FVF_TEX1, DrawVertsTEX1, 4, D3DDP_DONOTUPDATEEXTENTS | D3DDP_DONOTCLIP);
    }

// bomb tag dir arrow

    if (GameSettings.GameType == GAMETYPE_NETWORK_BATTLETAG && FoxObj && FoxObj->player != PLR_LocalPlayer && !PLR_LocalPlayer->RaceFinishTime)
    {
        SubVector(&FoxObj->player->car.Body->Centre.Pos, &PLR_LocalPlayer->car.Body->Centre.Pos, &vec);
        TransposeRotVector(&PLR_LocalPlayer->car.Body->Centre.WMatrix, &vec, &vec2);
        f = (float)atan2(vec2.v[X], vec2.v[Z]) + RAD * 3.0f / 8.0f;

        for (i = 0 ; i < 4 ; i++)
        {
            DrawVertsTEX1[i].sx = SafeAreaLeft   + (-(float)sin(f + ((float)i / 4.0f * RAD)) * 48.0f + 320.0f) * RenderSettings.GeomScaleX + ScreenLeftClip;
            DrawVertsTEX1[i].sy = SafeAreaBottom + ((float)cos(f + ((float)i / 4.0f * RAD)) * 48.0f + 48.0f) * RenderSettings.GeomScaleY + ScreenTopClip;
            DrawVertsTEX1[i].sz = 0; //$ADDITION: explicitly set z (even though z-buffer off) to avoid near/far clipping
            DrawVertsTEX1[i].rhw = 1.0f;
            DrawVertsTEX1[i].color = 0xffffffff;
        }

        DrawVertsTEX1[0].tu = DrawVertsTEX1[3].tu = 2.0f / 256.0f;
        DrawVertsTEX1[1].tu = DrawVertsTEX1[2].tu = 62.0f / 256.0f;
        DrawVertsTEX1[0].tv = DrawVertsTEX1[1].tv = 66.0f / 256.0f;
        DrawVertsTEX1[2].tv = DrawVertsTEX1[3].tv = 126.0f / 256.0f;

        DRAW_PRIM(D3DPT_TRIANGLEFAN, FVF_TEX1, DrawVertsTEX1, 4, D3DDP_DONOTUPDATEEXTENTS | D3DDP_DONOTCLIP);
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
            DrawPanelSprite(x+SafeAreaRight, y+SafeAreaBottom, xsize, ysize, tu, tv, twidth, theight, 0xc0ffffff);
        else if (RevLit[i] == REV_LIT_MAX)
            DrawPanelSprite(x+SafeAreaRight, y+SafeAreaBottom, xsize, ysize, tu, tv + (25.0f / 256.0f), twidth, theight, 0xc0ffffff);
        else
        {
            col = RevLit[i] * 0xc0 / REV_LIT_MAX;
            DrawPanelSprite(x+SafeAreaRight, y+SafeAreaBottom, xsize, ysize, tu, tv, twidth, theight, ((0xc0 - col) << 24) | 0xffffff);
            DrawPanelSprite(x+SafeAreaRight, y+SafeAreaBottom, xsize, ysize, tu, tv + (25.0f / 256.0f), twidth, theight, (col << 24) | 0xffffff);
        }
    }

// pickup

    if (GameSettings.GameType != GAMETYPE_TRIAL && GameSettings.GameType != GAMETYPE_PRACTICE && GameSettings.GameType != GAMETYPE_TRAINING && GameSettings.AllowPickups)
    {
        DrawPanelSprite(0+(float)SafeAreaLeft, 60+(float)SafeAreaTop, 64, 64, 190.0f / 256.0f, 157.0f / 256.0f, 64.0f / 256.0f, 64.0f / 256.0f, 0xe0ffffff);

        if (PLR_LocalPlayer->PickupNum)
        {
            pickup = PLR_LocalPlayer->PickupType;
            col = 0;
        }
        else if (PLR_LocalPlayer->PickupCycleSpeed)
        {
            pickup = (long)PLR_LocalPlayer->PickupCycleType;
            pickup2 = (pickup + 1) % PICKUP_NTYPES;
    
            FTOL((PLR_LocalPlayer->PickupCycleType - (float)pickup) * Real(0x80), col);
            col |= (col << 8) | (col << 16);
        }
        else
        {
            pickup = -1;
        }

        if (pickup != -1)
        {
            BLEND_ON();
            BLEND_SRC(D3DBLEND_ONE);
            BLEND_DEST(D3DBLEND_ONE);

            tu = (PickupUV[pickup * 2]) / 256.0f;
            tv = (PickupUV[pickup * 2 + 1]) / 256.0f;
            if (PLR_LocalPlayer->PickupCycleSpeed) {
                DrawPanelSprite(16+(float)SafeAreaLeft, 76+(float)SafeAreaTop, 32, 32, tu, tv, 32.0f / 256.0f, 32.0f / 256.0f, 0x808080 - col);
            } else {
                DrawPanelSprite(16+(float)SafeAreaLeft, 76+(float)SafeAreaTop, 32, 32, tu, tv, 32.0f / 256.0f, 32.0f / 256.0f, 0xffffff - col);
            }

            if (col)
            {
                tu = (PickupUV[pickup2 * 2]) / 256.0f;
                tv = (PickupUV[pickup2 * 2 + 1]) / 256.0f;
                DrawPanelSprite(16+(float)SafeAreaLeft, 76+(float)SafeAreaTop, 32, 32, tu, tv, 32.0f / 256.0f, 32.0f / 256.0f, col);
            }

            BLEND_ALPHA();
            BLEND_SRC(D3DBLEND_SRCALPHA);
            BLEND_DEST(D3DBLEND_INVSRCALPHA);
        }
    }

// lag

    if (IsMultiPlayer() && !CountdownTime)
    {
        for (player = PLR_PlayerHead ; player ; player = player->next) if (player->type == PLAYER_REMOTE)
        {
            lastping = TotalRacePhysicsTime - player->car.RemoteData[player->car.NewDat].Time;
            if (lastping >= 1000 && !(lastping & 0x80000000))
            {
                DrawPanelSprite(20+(float)SafeAreaLeft, 128+(float)SafeAreaTop, 32.0f, 32.0f, 64.0f / 256.0f, 192.0f / 256.0f, 32.0f / 256.0f, 32.0f / 256.0f, 0xc0ffffff);
                break;
            }
        }
    }

// normal font

    SET_TPAGE(TPAGE_FONT);

// stunt arena star count

    if (GameSettings.GameType == GAMETYPE_TRAINING)
    {
        swprintf(buf, L"%2.2ld   %2.2ld", StarList.NumFound, StarList.NumTotal);
        DumpText(640+SafeAreaRight - 72, 16+SafeAreaTop, 8, 16, MENU_TEXT_RGB_NORMAL, buf);
        DumpText(640+SafeAreaRight - 48, 16+SafeAreaTop, 8, 16, MENU_COLOR_OPAQUE|MENU_COLOR_CYAN, L"/");
    }

// pickup num

    if (GameSettings.GameType != GAMETYPE_TRIAL && GameSettings.GameType != GAMETYPE_PRACTICE  && GameSettings.GameType != GAMETYPE_TRAINING && PLR_LocalPlayer->PickupNum && (PLR_LocalPlayer->PickupType == PICKUP_TYPE_FIREWORKPACK) || (PLR_LocalPlayer->PickupType == PICKUP_TYPE_WATERBOMB))
    {
        swprintf(buf, L"%d", PLR_LocalPlayer->PickupNum);
        DumpText(50+SafeAreaLeft, 108+SafeAreaTop, 6, 8, MENU_COLOR_OPAQUE|MENU_COLOR_YELLOW, buf);
    }

// console

    if (ConsoleTimer)
    {
        ConsoleTimer -= TimeStep;
        if (ConsoleTimer < 0.0f)
            ConsoleTimer = 0.0f;

        if (ConsoleTimer > ConsoleStartTime - 0.5f) //1.5f)
        {
            //FTOL((2.0f - ConsoleTimer) * 511.0f, i);
            FTOL((ConsoleStartTime - ConsoleTimer) * 511.0f, i);
        }
        else if (ConsoleTimer < 0.5f)
        {
            FTOL(ConsoleTimer * 511.0f, i);
        }
        else
        {
            i = 255;
        }


        if( ConsoleMessage2 )
        {
            // Display a 2-line message
            g_pFont->DrawText( 320, 349, ConsoleRGB1 | i << 24, ConsoleMessage1, XBFONT_CENTER_X );
            g_pFont->DrawText( 320, 369, ConsoleRGB2 | i << 24, ConsoleMessage2, XBFONT_CENTER_X );
        }
        else
        {
            // Display a 1-line message
            g_pFont->DrawText( 320, 369, ConsoleRGB1 | i << 24, ConsoleMessage1, XBFONT_CENTER_X );
        }

        // Restore tpage setting which got obliterated by the new font code
        RenderTP = -1;
        SET_TPAGE(TPAGE_FONT);
    }

// split time?

    if (PLR_LocalPlayer->DisplaySplitCount && GameSettings.GameType != GAMETYPE_PRACTICE && GameSettings.GameType != GAMETYPE_TRAINING)
    {
        PLR_LocalPlayer->DisplaySplitCount -= TimeStep;
        if (PLR_LocalPlayer->DisplaySplitCount < 0) PLR_LocalPlayer->DisplaySplitCount = 0;

        DumpText(CENTRE_POS(TEXT_PANEL_GHOSTSPLIT), 128+SafeAreaTop, 8, 16, MENU_COLOR_OPAQUE|MENU_COLOR_CYAN, TEXT_TABLE(TEXT_PANEL_GHOSTSPLIT));
        if (PLR_LocalPlayer->DisplaySplitTime > 0) 
            swprintf(buf, L"+%02d:%02d:%03d", MINUTES(PLR_LocalPlayer->DisplaySplitTime), SECONDS(PLR_LocalPlayer->DisplaySplitTime), THOUSANDTHS(PLR_LocalPlayer->DisplaySplitTime));
        else 
            swprintf(buf, L"-%02d:%02d:%03d", MINUTES(-PLR_LocalPlayer->DisplaySplitTime), SECONDS(-PLR_LocalPlayer->DisplaySplitTime), THOUSANDTHS(-PLR_LocalPlayer->DisplaySplitTime));
        DumpText(280+SafeAreaCenter, 144+SafeAreaTop, 8, 16, MENU_TEXT_RGB_NORMAL, buf);
    }

// lap time

    if (PLR_LocalPlayer->RaceFinishTime == 0  && GameSettings.GameType != GAMETYPE_NETWORK_BATTLETAG && GameSettings.GameType != GAMETYPE_PRACTICE && GameSettings.GameType != GAMETYPE_TRAINING)
    {
        g_pFont->DrawText(592, 96+fSafeAreaTop, MENU_COLOR_OPAQUE|MENU_COLOR_CYAN, TEXT_TABLE(TEXT_PANEL_LAPTIME), XBFONT_RIGHT );
        swprintf(buf, L"%02d:%02d:%03d", MINUTES(PLR_LocalPlayer->car.CurrentLapTime), SECONDS(PLR_LocalPlayer->car.CurrentLapTime), THOUSANDTHS(PLR_LocalPlayer->car.CurrentLapTime));
        g_pFont->DrawText(592, 116+fSafeAreaTop, MENU_TEXT_RGB_NORMAL, buf, XBFONT_RIGHT );

        // Restore tpage setting which got obliterated by the new font code
        RenderTP = -1;
        SET_TPAGE(TPAGE_FONT);
    }

// DEBUG - Timestep and Number of physics loops
#if 0
    swprintf(buf, L"%3.5f   %3d  (%10d)\n", TimeStep, NPhysicsLoops, OverlapTime);
    DumpText(200, 100+SafeAreaTop, 18, 24, MENU_TEXT_RGB_NORMAL, buf);
    WriteLogEntry(buf);
#endif

// bomb tag timer

    if (GameSettings.GameType == GAMETYPE_NETWORK_BATTLETAG)
    {
        g_pFont->DrawText(592, 16+fSafeAreaTop, MENU_COLOR_OPAQUE|MENU_COLOR_CYAN, TEXT_TABLE(TEXT_PANEL_TAGTIME), XBFONT_RIGHT );
        swprintf(buf, L"%02d:%02d:%03d", MINUTES(PLR_LocalPlayer->BombTagTimer), SECONDS(PLR_LocalPlayer->BombTagTimer), THOUSANDTHS(PLR_LocalPlayer->BombTagTimer));
        g_pFont->DrawText(592, 36+fSafeAreaTop, MENU_TEXT_RGB_NORMAL, buf, XBFONT_RIGHT );

        // Restore tpage setting which got obliterated by the new font code
        RenderTP = -1;
        SET_TPAGE(TPAGE_FONT);

        if (FoxObj)
        {
            swprintf(buf, L"%S", FoxObj->player->PlayerName);
            DumpText(16+SafeAreaLeft, 16+SafeAreaTop, 8, 16, MENU_COLOR_OPAQUE|MENU_COLOR_RED, buf);
            swprintf(buf, L"%02d:%02d:%03d", MINUTES(FoxObj->player->BombTagTimer), SECONDS(FoxObj->player->BombTagTimer), THOUSANDTHS(FoxObj->player->BombTagTimer));
            DumpText(16+SafeAreaLeft, 32+SafeAreaTop, 8, 16, MENU_TEXT_RGB_NORMAL, buf);
        }
    }

// race time

    if (GameSettings.GameType != GAMETYPE_TRIAL && GameSettings.GameType != GAMETYPE_NETWORK_BATTLETAG && GameSettings.GameType != GAMETYPE_PRACTICE && GameSettings.GameType != GAMETYPE_TRAINING)
    {
        if (PLR_LocalPlayer->RaceFinishTime == 0) 
        {
            g_pFont->DrawText(592, 136+fSafeAreaTop, MENU_COLOR_OPAQUE|MENU_COLOR_CYAN, TEXT_TABLE(TEXT_PANEL_RACETIME), XBFONT_RIGHT );
            swprintf(buf, L"%02d:%02d:%03d", MINUTES(TotalRaceTime), SECONDS(TotalRaceTime), THOUSANDTHS(TotalRaceTime));
            g_pFont->DrawText(592, 156+fSafeAreaTop, MENU_TEXT_RGB_NORMAL, buf, XBFONT_RIGHT );
        } 
        else 
        {
            g_pFont->DrawText(592, 96+fSafeAreaTop, MENU_COLOR_OPAQUE|MENU_COLOR_CYAN, TEXT_TABLE(TEXT_PANEL_RACETIME), XBFONT_RIGHT );
            swprintf(buf, L"%02d:%02d:%03d", MINUTES(PLR_LocalPlayer->RaceFinishTime), SECONDS(PLR_LocalPlayer->RaceFinishTime), THOUSANDTHS(PLR_LocalPlayer->RaceFinishTime));
            g_pFont->DrawText(592, 116+fSafeAreaTop, MENU_TEXT_RGB_NORMAL, buf, XBFONT_RIGHT );
        }

        // Restore tpage setting which got obliterated by the new font code
        RenderTP = -1;
        SET_TPAGE(TPAGE_FONT);
    }

// record lap

    if (GameSettings.GameType == GAMETYPE_TRIAL && TrackRecords.RecordLap[0].Time != MAX_LAP_TIME)
    {
        g_pFont->DrawText(592, 136+fSafeAreaTop, MENU_COLOR_OPAQUE|MENU_COLOR_YELLOW, TEXT_TABLE(TEXT_RECORD), XBFONT_RIGHT );
        swprintf(buf, L"%02d:%02d:%03d", MINUTES(TrackRecords.RecordLap[0].Time), SECONDS(TrackRecords.RecordLap[0].Time), THOUSANDTHS(TrackRecords.RecordLap[0].Time));
        g_pFont->DrawText(592, 156+fSafeAreaTop, MENU_TEXT_RGB_NORMAL, buf, XBFONT_RIGHT );

        // Restore tpage setting which got obliterated by the new font code
        RenderTP = -1;
        SET_TPAGE(TPAGE_FONT);
    }

// ghost lap

    if (GameSettings.GameType == GAMETYPE_TRIAL && GHO_GhostDataExists)
    {
        g_pFont->DrawText(592, 176+fSafeAreaTop, MENU_COLOR_OPAQUE|MENU_COLOR_YELLOW, TEXT_TABLE(TEXT_PANEL_GHOSTTIME), XBFONT_RIGHT );
        swprintf(buf, L"%02d:%02d:%03d", MINUTES(GHO_BestGhostInfo->Time[MAX_SPLIT_TIMES]), SECONDS(GHO_BestGhostInfo->Time[MAX_SPLIT_TIMES]), THOUSANDTHS(GHO_BestGhostInfo->Time[MAX_SPLIT_TIMES]));
        g_pFont->DrawText(592, 196+fSafeAreaTop, MENU_TEXT_RGB_NORMAL, buf, XBFONT_RIGHT );

        // Restore tpage setting which got obliterated by the new font code
        RenderTP = -1;
        SET_TPAGE(TPAGE_FONT);
    }

// challenge time

    if (GameSettings.GameType == GAMETYPE_TRIAL && GameSettings.Level < LEVEL_NCUP_LEVELS)
    {
        i = 0;
        time = 0;
        LEVELINFO *levinfo = GetLevelInfo(GameSettings.Level);

        if (!GameSettings.Mirrored && !GameSettings.Reversed)
        {
            i = IsSecretBeatTimeTrial(GameSettings.Level);
            time = levinfo->ChallengeTimeNormal;
        }

        else if (!GameSettings.Mirrored && GameSettings.Reversed)
        {
            i = IsSecretBeatTimeTrialReverse(GameSettings.Level);
            time = levinfo->ChallengeTimeReversed;
        }

        else if (GameSettings.Mirrored && !GameSettings.Reversed)
        {
            i = IsSecretBeatTimeTrialMirror(GameSettings.Level);
            time = levinfo->ChallengeTimeNormal;
        }

        if (time)
        {
            if (i)
            {   
                ChallengeFlash -= (long)(TimeStep * 1000.0f);
                if (ChallengeFlash < 0) ChallengeFlash = 0;
            }
            else
            {
                ChallengeFlash = CHALLENGE_FLASH_TIME;
            }

            if (!i || (ChallengeFlash & 128))
            {
                g_pFont->DrawText(592, 216+fSafeAreaTop, MENU_COLOR_OPAQUE|MENU_COLOR_YELLOW, TEXT_TABLE(TEXT_PANEL_CHALLENGETIME), XBFONT_RIGHT );
                swprintf(buf, L"%02d:%02d:%03d", MINUTES(time), SECONDS(time), THOUSANDTHS(time));
                g_pFont->DrawText(592, 236+fSafeAreaTop, MENU_TEXT_RGB_NORMAL, buf, XBFONT_RIGHT );

                // Restore tpage setting which got obliterated by the new font code
                RenderTP = -1;
                SET_TPAGE(TPAGE_FONT);
            }
            if (ChallengeFlash != CHALLENGE_FLASH_TIME && ChallengeFlash & 128)
            {
                DumpText(CENTRE_POS(TEXT_PANEL_CHALLENGE_BEATEN), 236+SafeAreaTop, 8, 16, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_PANEL_CHALLENGE_BEATEN));
            }
        }
        else
        {
        }
    }

// speed

    g_pFont->DrawText( 592, 404, MENU_COLOR_OPAQUE|MENU_COLOR_CYAN, TEXT_TABLE(SpeedUnitText[SpeedUnits]), XBFONT_RIGHT );
    FLOAT fUnitsStringWidth = g_pFont->GetTextWidth( TEXT_TABLE(SpeedUnitText[SpeedUnits]) );

    speed = (long)(SpeedUnitScale[SpeedUnits] * VecLen(&PLR_LocalPlayer->car.Body->Centre.Vel));
    swprintf(buf, L"%4d ", speed);
    g_pFont->DrawText( 592-fUnitsStringWidth, 404, MENU_TEXT_RGB_NORMAL, buf, XBFONT_RIGHT );

    if ((SpeedUnits == SPEED_SCALEDMPH) || (SpeedUnits == SPEED_SCALEDKPH)) 
        g_pFont->DrawText( 592, 424, MENU_COLOR_SEETHROUGH|MENU_COLOR_CYAN, TEXT_TABLE(TEXT_SCALED), XBFONT_RIGHT );

    // Restore tpage setting which got obliterated by the new font code
    RenderTP = -1;
    SET_TPAGE(TPAGE_FONT);

// best acc
//  swprintf(buf, L"0-15: %3dms", (PLR_LocalPlayer->car.Best0to15 > ZERO)? (int)(1000 * PLR_LocalPlayer->car.Best0to15): 0);
//  DumpText(100, 50, 12, 16, 0xc0ffffff, buf);
//  swprintf(buf, L"0-25: %3dms", (PLR_LocalPlayer->car.Best0to25 > ZERO)? (int)(1000 * PLR_LocalPlayer->car.Best0to25): 0);
//  DumpText(100, 70, 12, 16, 0xc0ffffff, buf);
//  swprintf(buf, L"ValidNode: %d (%d)", PLR_LocalPlayer->ValidRailCamNode, (PLR_LocalPlayer->ValidRailCamNode == -1)? -1: CAM_CameraNode[PLR_LocalPlayer->ValidRailCamNode].ID);
//  DumpText(100, 50, 12, 16, MENU_COLOR_OPAQUE|MENU_COLOR_RED, buf);
//  swprintf(buf, L"BangMag: %d", (int)(100 * PLR_LocalPlayer->car.Body->BangMag));
//  DumpText(100, 50, 12, 16, MENU_COLOR_OPAQUE|MENU_COLOR_RED, buf);
//  swwprintf(buf,L "Down: %8d %8d %8d (%8d)", (int)(100 * DEBUG_DownForce.v[X]), (int)(100 * DEBUG_DownForce.v[Y]), (int)(100 * DEBUG_DownForce.v[Z]), (int)(100 * PLR_LocalPlayer->car.DownForceMod));
//  DumpText(100, 50, 8, 12, MENU_COLOR_OPAQUE|MENU_COLOR_RED, buf);
//  swprintf(buf, L"StuckTime: %8d", (int)(1000.0f * PLR_LocalPlayer->CarAI.StuckCnt));
//  DumpText(100, 70, 8, 12, MENU_COLOR_OPAQUE|MENU_COLOR_RED, buf);
//  swprintf(buf, L"RPL Buffer %8d (%8d)", ReplayBufferBytesStored, ReplayDataBufSize);
//  DumpText(100, 50, 8, 12, MENU_COLOR_OPAQUE|MENU_COLOR_RED, buf);
//  swprintf(buf, L"RaceTime: %8d   OverlapTime: %8d", TotalRacePhysicsTime, OverlapTime);
//  DumpText(100, 70, 8, 12, MENU_COLOR_OPAQUE|MENU_COLOR_RED, buf);
//  swprintf(buf, L"Timeout: %s", (PLR_LocalPlayer->car.RemoteTimeout)? l"TRUE": l"FALSE");
//  DumpText(100, 70, 8, 12, MENU_COLOR_OPAQUE|MENU_COLOR_RED, buf);
//  swprintf(buf, L"%f %f %f %f", time0, time1, time2, timeNow);
//  DumpText(100, 70, 8, 16, MENU_COLOR_OPAQUE|MENU_COLOR_RED, buf);
//  swprintf(buf, L"%d", ServerID);
//  DumpText(100, 70, 8, 16, MENU_COLOR_OPAQUE|MENU_COLOR_RED, buf);
//  swprintf(buf, L"%f", PLR_LocalPlayer->car.LastHitTimer);
//  DumpText(100, 70, 8, 16, MENU_COLOR_OPAQUE|MENU_COLOR_RED, buf);

//  swprintf(buf, L"Tests: %4d Passes: %4d (Max: %4d)", COL_NCollsTested, COL_NCollsPassed, PosToCollGrid(&PLR_LocalPlayer->car.Body->Centre.Pos)->NCollPolys);
//  DumpText(100, 70, 8, 16, MENU_COLOR_OPAQUE|MENU_COLOR_RED, buf);

//  swprintf(buf, L"Body-Body: %4d", COL_NBodyBodyTests);
//  DumpText(100, 90, 8, 16, MENU_COLOR_OPAQUE|MENU_COLOR_RED, buf);

/*  // Wheel contact information
    swprintf(buf, L"Floor: %3s;  Wall: %3s;  Side: %3s  Other: %3s",
        (IsWheelInFloorContact(&PLR_LocalPlayer->car.Wheel[0]))? L"Yes": L"No",
        (IsWheelInWallContact(&PLR_LocalPlayer->car.Wheel[0]))? L"Yes": L"No",
        (IsWheelInSideContact(&PLR_LocalPlayer->car.Wheel[0]))? L"Yes": L"No",
        (IsWheelInOtherContact(&PLR_LocalPlayer->car.Wheel[0]))? L"Yes": L"No");
    DumpText(50, 50, 8, 12, MENU_COLOR_OPAQUE|MENU_COLOR_YELLOW, buf);

    swprintf(buf, L"Floor: %3s;  Wall: %3s;  Side: %3s  Other: %3s",
        (IsWheelInFloorContact(&PLR_LocalPlayer->car.Wheel[1]))? L"Yes": L"No",
        (IsWheelInWallContact(&PLR_LocalPlayer->car.Wheel[1]))? L"Yes": L"No",
        (IsWheelInSideContact(&PLR_LocalPlayer->car.Wheel[1]))? L"Yes": L"No",
        (IsWheelInOtherContact(&PLR_LocalPlayer->car.Wheel[1]))? L"Yes": L"No");
    DumpText(50, 70, 8, 12, MENU_COLOR_OPAQUE|MENU_COLOR_YELLOW, buf);

    swprintf(buf, L"Floor: %3s;  Wall: %3s;  Side: %3s  Other: %3s",
        (IsWheelInFloorContact(&PLR_LocalPlayer->car.Wheel[2]))? L"Yes": L"No",
        (IsWheelInWallContact(&PLR_LocalPlayer->car.Wheel[2]))? L"Yes": L"No",
        (IsWheelInSideContact(&PLR_LocalPlayer->car.Wheel[2]))? L"Yes": L"No",
        (IsWheelInOtherContact(&PLR_LocalPlayer->car.Wheel[2]))? L"Yes": L"No");
    DumpText(50, 90, 8, 12, MENU_COLOR_OPAQUE|MENU_COLOR_YELLOW, buf);

    swprintf(buf, L"Floor: %3s;  Wall: %3s;  Side: %3s  Other: %3s",
        (IsWheelInFloorContact(&PLR_LocalPlayer->car.Wheel[3]))? L"Yes": L"No",
        (IsWheelInWallContact(&PLR_LocalPlayer->car.Wheel[3]))? L"Yes": L"No",
        (IsWheelInSideContact(&PLR_LocalPlayer->car.Wheel[3]))? L"Yes": L"No",
        (IsWheelInOtherContact(&PLR_LocalPlayer->car.Wheel[3]))? L"Yes": L"No");
    DumpText(50, 110, 8, 12, MENU_COLOR_OPAQUE|MENU_COLOR_YELLOW, buf);
*/

    if (GameSettings.Level == GetLevelNum("frontend")) {
        VEC look;
        swprintf(buf, L"Pos:  %f %f %f", CAM_MainCamera->WPos.v[X], CAM_MainCamera->WPos.v[Y], CAM_MainCamera->WPos.v[Z]);
        DumpText(100+SafeAreaLeft, 70+SafeAreaTop, 8, 16, MENU_TEXT_RGB_NORMAL, buf);
        VecPlusScalarVec(&CAM_MainCamera->WPos, 1000, &CAM_MainCamera->WMatrix.mv[L], &look);
        swprintf(buf, L"Look: %f %f %f", look.v[X], look.v[Y], look.v[Z]);
        DumpText(100+SafeAreaLeft, 90+SafeAreaTop, 8, 16, MENU_TEXT_RGB_NORMAL, buf);
    }

// laps

    if (GameSettings.GameType != GAMETYPE_TRIAL && GameSettings.GameType != GAMETYPE_NETWORK_BATTLETAG && GameSettings.GameType != GAMETYPE_PRACTICE && GameSettings.GameType != GAMETYPE_TRAINING)
    {
        g_pFont->DrawText( 48, 36, MENU_COLOR_OPAQUE|MENU_COLOR_CYAN, TEXT_TABLE(TEXT_PANEL_LAPCOUNTER));
        if (PLR_LocalPlayer->car.Laps < GameSettings.NumberOfLaps)
            swprintf(buf, L"%d/%d", PLR_LocalPlayer->car.Laps + 1, GameSettings.NumberOfLaps);
        else
            swprintf(buf, TEXT_TABLE(TEXT_FINISHED));
        g_pFont->DrawText( 48, 56, MENU_TEXT_RGB_NORMAL, buf);

        // Restore tpage setting which got obliterated by the new font code
        RenderTP = -1;
        SET_TPAGE(TPAGE_FONT);
    }

// calc position, ahead, behind players

    if (GameSettings.GameType != GAMETYPE_PRACTICE && GameSettings.GameType != GAMETYPE_TRAINING)
    {
        pos = 1;
        nearestdistahead = FLT_MAX;
        nearestdistbehind = -FLT_MAX;

        if (GameSettings.GameType == GAMETYPE_TRIAL)
            localdist = -PLR_LocalPlayer->CarAI.FinishDistPanel - (float)PLR_LocalPlayer->CarAI.BackTracking;
        else if (GameSettings.GameType == GAMETYPE_NETWORK_BATTLETAG)
            localdist = (float)(BOMBTAG_MAX_TIME - PLR_LocalPlayer->BombTagTimer) / 1000.0f;
        else
            localdist = (float)PLR_LocalPlayer->car.Laps - PLR_LocalPlayer->CarAI.FinishDistPanel - (float)PLR_LocalPlayer->CarAI.BackTracking;

        // $STATISTICS: following vars added to clarify stat games states
        // $SINGLEPAYER: check to make sure all local players are finished (perhaps) then update stats.
        int nOpponentPlayers = 0;
        int nOpponentPlayersStillRacing = 0;
        bool bLocalPlayerFinished = (bool)PLR_LocalPlayer->RaceFinishTime;

        AllPlayersFinished = (long)PLR_LocalPlayer->RaceFinishTime;


        for (player = PLR_PlayerHead ; player ; player = player->next) if (player != PLR_LocalPlayer && player->type != PLAYER_NONE)
        {
            nOpponentPlayers++;

            if (GameSettings.GameType == GAMETYPE_NETWORK_BATTLETAG)
                dist = (float)(BOMBTAG_MAX_TIME - player->BombTagTimer) / 1000.0f;
            else
                dist = (float)player->car.Laps - player->CarAI.FinishDistPanel - (float)player->CarAI.BackTracking;

            if (dist > localdist)
            {
                pos++;
                if (dist < nearestdistahead)
                {
                    nearestdistahead = dist;
                    playerahead = player;
                }
            }
            else
            {
                if (dist > nearestdistbehind)
                {
                    nearestdistbehind = dist;
                    playerbehind = player;
                }
            }

            if (!player->RaceFinishTime)
            {
                nOpponentPlayersStillRacing++;
                AllPlayersFinished = FALSE;
            }
        }

        if (PLR_LocalPlayer->RaceFinishTime)
            LocalRacePos = PLR_LocalPlayer->RaceFinishPos + 1;
        else
            LocalRacePos = pos;

        // show pos, ahead, behind players
        g_pFont->SetScaleFactors( 1.5f, 1.5f );
        g_pFont->DrawText( 48, 356, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_1ST + LocalRacePos - 1) );
        g_pFont->SetScaleFactors( 1.0f, 1.0f );

        // Restore tpage setting which got obliterated by the new font code
        RenderTP = -1;
        SET_TPAGE(TPAGE_FONT);

        if (GameSettings.GameType == GAMETYPE_NETWORK_BATTLETAG)
        {
            if (nearestdistahead != FLT_MAX)
            {
                dist = (nearestdistahead - localdist);
                FTOL(dist * 12.75f, col);
                if (col > 511) col = 511;
                if (col < 256) col = col << 16 | (MENU_COLOR_OPAQUE|MENU_COLOR_GREEN);
                else col = (511 - col) << 8 | (MENU_COLOR_OPAQUE|MENU_COLOR_RED);

                swprintf(buf, L"-%lds ", (long)dist);
                g_pFont->DrawText(122, 396, col, buf, XBFONT_RIGHT);
                swprintf(buf, L"%0.12S", playerahead->PlayerName);
                g_pFont->DrawText(122, 396, MENU_TEXT_RGB_NORMAL, buf, XBFONT_TRUNCATED, 126.0f );
            }

            if (nearestdistbehind != -FLT_MAX)
            {
                dist = (localdist - nearestdistbehind);
                FTOL(dist * 12.75f, col);
                if (col > 511) col = 511;
                if (col < 256) col = col << 8 | (MENU_COLOR_OPAQUE|MENU_COLOR_RED);
                else col = (511 - col) << 16 | (MENU_COLOR_OPAQUE|MENU_COLOR_GREEN);

                swprintf(buf, L"+%lds ", (long)dist);
                g_pFont->DrawText(122, 416, col, buf, XBFONT_RIGHT);
                swprintf(buf, L"%0.12S", playerbehind->PlayerName);
                g_pFont->DrawText(122, 416, MENU_TEXT_RGB_NORMAL, buf, XBFONT_TRUNCATED, 126.0f );
            }
        }
        else
        {
            if (nearestdistahead != FLT_MAX)
            {
                dist = (nearestdistahead - localdist) * PosTotalDist / 200.0f;
                FTOL(dist * 10.24f, col);
                if (col > 511) col = 511;
                if (col < 256) col = col << 16 | (MENU_COLOR_OPAQUE|MENU_COLOR_GREEN);
                else col = (511 - col) << 8 | (MENU_COLOR_OPAQUE|MENU_COLOR_RED);

                swprintf(buf, L"-%ld%s ", (long)dist, TEXT_TABLE(TEXT_METERS_ABREV));
                g_pFont->DrawText(122, 396, col, buf, XBFONT_RIGHT);
                swprintf(buf, L"%0.12S", playerahead->PlayerName);
                g_pFont->DrawText(122, 396, MENU_TEXT_RGB_NORMAL, buf, XBFONT_TRUNCATED, 126.0f );
            }

            if (nearestdistbehind != -FLT_MAX)
            {
                dist = (localdist - nearestdistbehind) * PosTotalDist / 200.0f;
                FTOL(dist * 10.24f, col);
                if (col > 511) col = 511;
                if (col < 256) col = col << 8 | (MENU_COLOR_OPAQUE|MENU_COLOR_RED);
                else col = (511 - col) << 16 | (MENU_COLOR_OPAQUE|MENU_COLOR_GREEN);

                swprintf(buf, L"+%ld%s ", (long)dist, TEXT_TABLE(TEXT_METERS_ABREV));
                g_pFont->DrawText(122, 416, col, buf, XBFONT_RIGHT);
                swprintf(buf, L"%0.12S", playerbehind->PlayerName);
                g_pFont->DrawText(122, 416, MENU_TEXT_RGB_NORMAL, buf, XBFONT_TRUNCATED, 126.0f );
            }
        }
    }

    // Restore tpage setting which got obliterated by the new font code
    RenderTP = -1;
    SET_TPAGE(TPAGE_FONT);

    // final position
    if( PLR_LocalPlayer->RaceFinishTime )
    {
        LocalRacePosScale -= TimeStep * 500.0f;
        if (LocalRacePosScale < 32.0f)
            LocalRacePosScale = 32.0f;

        if (LocalRacePosScale < 300.0f - 256.0f)
            col = MENU_TEXT_RGB_NORMAL;
        else
            col = (long)((300.0f - LocalRacePosScale)) << 24 | MENU_COLOR_WHITE;

        xsize = LocalRacePosScale;
        ysize = xsize * 2.0f;
        x = 320.0f - xsize * 0.5f;
        y = 36.0f;

        if (LocalRacePos >= 10)
        {
            x -= xsize * 0.5f;
            tu = ((float)(LocalRacePos / 10) * 32.0f + 1.0f) / 256.0f;
            tv = 129.0f / 256.0f;
            DrawPanelSprite(x, y, xsize, ysize, tu, tv, 30.0f / 256.0f, 62.0f / 256.0f, col);
            x += xsize;
        }

        tu = (((LocalRacePos % 10) % 8) * 32.0f + 1.0f) / 256.0f;
        tv = (((LocalRacePos % 10) / 8) * 64.0f + 129.0f) / 256.0f;
        DrawPanelSprite(x, y, xsize, ysize, tu, tv, 30.0f / 256.0f, 62.0f / 256.0f, col);
        x += xsize;
        y += ysize * 0.08f;
        ysize *= 0.5f;

        if (LocalRacePos <= 2)
            tu = 64.0f / 256.0f;
        else
            tu = 96.0f / 256.0f;

        if (LocalRacePos <= 4 && LocalRacePos & 1)
            tv = 192.0f / 256.0f;
        else
            tv = 224.0f / 256.0f;

        g_pFont->SetScaleFactors( 1.5f, 1.5f );
        g_pFont->SetSlantFactor( 10.0f );
        g_pFont->DrawText( x-2, y-6, 0xffe4d590, TEXT_TABLE(TEXT_1ST + LocalRacePos - 1)+1 );
        g_pFont->SetScaleFactors( 1.0f, 1.0f );
        g_pFont->SetSlantFactor( 0.0f );

// Remove the rearview and other player camera

        if (CAM_RearCamera != NULL)
        {
            RemoveCamera(CAM_RearCamera);
            CAM_RearCamera = NULL;
        }
        if (CAM_OtherCamera != NULL && GameSettings.GameType != GAMETYPE_NETWORK_BATTLETAG)
        {
            RemoveCamera(CAM_OtherCamera);
            CAM_OtherCamera = NULL;
        }
    }
    else
    {
        LocalRacePosScale = 300.0f;
    }

// last lap

    if (GameSettings.GameType != GAMETYPE_NETWORK_BATTLETAG && GameSettings.GameType != GAMETYPE_PRACTICE && GameSettings.GameType != GAMETYPE_TRAINING)
    {
        g_pFont->DrawText(592, 16+fSafeAreaTop, MENU_COLOR_OPAQUE|MENU_COLOR_CYAN, TEXT_TABLE(TEXT_PANEL_LASTLAP), XBFONT_RIGHT );
        swprintf(buf, L"%02d:%02d:%03d", MINUTES(PLR_LocalPlayer->car.LastLapTime), SECONDS(PLR_LocalPlayer->car.LastLapTime), THOUSANDTHS(PLR_LocalPlayer->car.LastLapTime));
        g_pFont->DrawText(592, 36+fSafeAreaTop, MENU_TEXT_RGB_NORMAL, buf, XBFONT_RIGHT );
    }

// best lap

    if (GameSettings.GameType != GAMETYPE_NETWORK_BATTLETAG && GameSettings.GameType != GAMETYPE_PRACTICE && GameSettings.GameType != GAMETYPE_TRAINING)
    {
        g_pFont->DrawText(592, 56+fSafeAreaTop, MENU_COLOR_OPAQUE|MENU_COLOR_CYAN, TEXT_TABLE(TEXT_PANEL_BESTLAP), XBFONT_RIGHT );
        swprintf(buf, L"%02d:%02d:%03d", MINUTES(PLR_LocalPlayer->car.BestLapTime), SECONDS(PLR_LocalPlayer->car.BestLapTime), THOUSANDTHS(PLR_LocalPlayer->car.BestLapTime));
        g_pFont->DrawText(592, 76+fSafeAreaTop, MENU_TEXT_RGB_NORMAL, buf, XBFONT_RIGHT );
    }

// finish times

    if (PLR_LocalPlayer->RaceFinishTime && (ChampionshipEndMode == CHAMPIONSHIP_END_FINISHED || ChampionshipEndMode == CHAMPIONSHIP_END_WAITING_FOR_FINISH || ChampionshipEndMode == CHAMPIONSHIP_END_QUALIFIED || ChampionshipEndMode == CHAMPIONSHIP_END_FAILED))
    {
        FLOAT sx = ( 640.0f - 248.0f )  /  2;
        FLOAT sy;

        if (GameSettings.GameType == GAMETYPE_CLOCKWORK)
        {
            sy = 86.0f + RaceTableOffset;
        }
        else
        {
            sy = 190.0f + RaceTableOffset;
        }

        for( i = pos = 0 ; i < StartData.PlayerNum ; i++)
        {
            if( FinishTable[i].Time )
            {
                if( !pos )
                {
                    g_pFont->SetScaleFactors( 1.5f, 1.5f );
                    g_pFont->DrawText( 320.0f, sy - 35, MENU_COLOR_OPAQUE|MENU_COLOR_CYAN, GameSettings.GameType == GAMETYPE_NETWORK_BATTLETAG ? TEXT_TABLE(TEXT_PANEL_BATTLE_RESULTS) : TEXT_TABLE(TEXT_PANEL_RACE_RESULTS), XBFONT_CENTER_X);
                    g_pFont->SetScaleFactors( 1.0f, 1.0f );
                }

                DWORD dwAlpha = 0xff000000;

                if( FinishTable[i].Player == PLR_LocalPlayer )
                {
                    LocalRacePos = pos + 1;

                    dwAlpha = ((long)(sinf((FLOAT)TIME2MS(TimerCurrent)/200.0f) * 64.0f + 192.0f))<<24L;
                }

                swprintf(buf, L"%2.2d", i + 1);
                g_pFont->DrawText( sx,  sy + 20*pos, dwAlpha|MENU_COLOR_WHITE, buf);
                swprintf(buf, L"%0.16S", FinishTable[i].Player->PlayerName);
                g_pFont->DrawText( sx + 26,  sy + 20*pos, dwAlpha|MENU_COLOR_CYAN, buf, XBFONT_TRUNCATED, 126 );
                swprintf(buf, L"%02d:%02d:%03d", MINUTES(FinishTable[i].Time), SECONDS(FinishTable[i].Time), THOUSANDTHS(FinishTable[i].Time));
                g_pFont->DrawText( sx + 156,  sy + 20*pos, dwAlpha|MENU_COLOR_WHITE, buf);
                pos++;
            }
        }

        if( g_pMenuHeader->m_pMenu || ChampionshipEndMode == CHAMPIONSHIP_END_QUALIFIED || ChampionshipEndMode == CHAMPIONSHIP_END_FAILED )
        {
            RaceTableOffset -= TimeStep * MENU_MOVE_VEL;
            if( RaceTableOffset < -480.0f ) RaceTableOffset = -480.0f;
        }
        else
        {
            RaceTableOffset += TimeStep * MENU_MOVE_VEL;
            if( RaceTableOffset > 0.0f ) RaceTableOffset = 0.0f;
        }
    }
    else
    {
        RaceTableOffset = -480.0f;
    }

    // Restore tpage setting which got obliterated by the new font code
    RenderTP = -1;
    SET_TPAGE(TPAGE_FONT);

    // championship table
    if (GameSettings.GameType == GAMETYPE_CHAMPIONSHIP && (ChampionshipEndMode == CHAMPIONSHIP_END_QUALIFIED || ChampionshipEndMode == CHAMPIONSHIP_END_MENU) && PLR_LocalPlayer->RaceFinishPos < CUP_QUALIFY_POS)
    {
        DrawChampionshipTable();
    }
    else if (GameSettings.GameType == GAMETYPE_CHAMPIONSHIP && (ChampionshipEndMode == CHAMPIONSHIP_END_FAILED || ChampionshipEndMode == CHAMPIONSHIP_END_MENU) && PLR_LocalPlayer->RaceFinishPos >= CUP_QUALIFY_POS && Players[0].RaceFinishPos != CRAZY_FINISH_POS)
    {
        DrawChampionshipFailed();
    }
    else
    {
        ChampionshipTableOffset = -640.0f;
    }

    // waiting for network players?
    if( IsMultiPlayer() && !AllPlayersReady )
    {
        FLOAT sy = 84.0f;
        for( player = PLR_PlayerHead ; player ; player = player->next )
        {
            if( player->type != PLAYER_NONE && !player->Ready )
            { 
                swprintf( buf, L"%S", player->PlayerName );
                g_pFont->DrawText( 320, sy, MENU_TEXT_RGB_NORMAL, buf, XBFONT_CENTER_X );
                sy += 20.0f;
            }
        }

        if( sy == 84.0f && !IsServer() )
            g_pFont->DrawText( 320, 52, MENU_COLOR_OPAQUE|MENU_COLOR_CYAN, TEXT_TABLE(TEXT_WAITINGFORHOST), XBFONT_CENTER_X );
        else
            g_pFont->DrawText( 320, 52, MENU_COLOR_OPAQUE|MENU_COLOR_CYAN, TEXT_TABLE(TEXT_WAITINGFOR), XBFONT_CENTER_X );

        if( IsServer() )
        {
            swprintf( buf, L"%ld", (long)AllReadyTimeout );
            g_pFont->DrawText( 320 + g_pFont->GetTextWidth(TEXT_TABLE(TEXT_WAITINGFOR))/2 + 8, 52, MENU_COLOR_OPAQUE|MENU_COLOR_RED, buf );
        }
    }

    // host quit?
    if( HostQuit && IsMultiPlayer() )
    {
        if( !(TIME2MS(TimerCurrent) & 256) )
        {
            g_pFont->DrawText( 320, 160+fSafeAreaTop, MENU_COLOR_OPAQUE|MENU_COLOR_GREEN, TEXT_TABLE(TEXT_MULTIGAMETERMINATED), XBFONT_CENTER_X );
        }
    }

    // show fps?
    if( RegistrySettings.ShowFPS )
    {
        g_pFont->SetScaleFactors( 0.7f, 0.7f );
        g_pFont->DrawText( 144+fSafeAreaLeft, 392+fSafeAreaBottom, MENU_COLOR_OPAQUE|MENU_COLOR_CYAN, L"fps" );
//$MODIFIED:
//        swprintf(buf, L"%d", FrameRate);
        swprintf(buf, L"%d", (int)FrameRate_GetFPS());
//$END_MODIFICATIONS
        g_pFont->DrawText( 144+fSafeAreaLeft, 404+fSafeAreaBottom, MENU_TEXT_RGB_NORMAL, buf);
        g_pFont->SetScaleFactors( 1.0f, 1.0f );
    }

    // Restore tpage setting which got obliterated by the new font code
    RenderTP = -1;
    SET_TPAGE(TPAGE_FONT);

// multiplayer info?

#if MULTIPLAYER_DEBUG
    static long ShowMultiDebug = false;
    if (Keys[DIK_NUMPADENTER] && !LastKeys[DIK_NUMPADENTER])
        ShowMultiDebug = !ShowMultiDebug;

    if (IsMultiPlayer() && ShowMultiDebug)
    {
        swprintf(buf, L"Send %ld", GetSendQueue(0));
        DumpText(0, 128, 8, 16, MENU_COLOR_OPAQUE|MENU_COLOR_CYAN, buf);

        swprintf(buf, L"PPS %ld", (long)PacketsPerSecond);
        DumpText(0, 144, 8, 16, MENU_COLOR_OPAQUE|MENU_COLOR_CYAN, buf);

//$MODIFIED
//        swprintf(buf, L"Data Sent %ld - Per Sec %ld", TotalDataSent, TotalDataSent / (TotalRaceTime / 1000));
        swprintf(buf, L"Data Sent %s - Per Sec %s", L"???", L"???" );
//$END_MODIFICATIONS
        DumpText(0, 160, 8, 16, MENU_COLOR_OPAQUE|MENU_COLOR_CYAN, buf);

        i = 192;
        for (player = PLR_PlayerHead ; player ; player = player->next) if (player->type == PLAYER_REMOTE)
        {
            unsigned long remtime = (unsigned long)player->car.RemoteData[player->car.NewDat].Time;

//$MODIFIED
//            swprintf(buf, L"%S = %02d:%02d:%03d, ping %ld, packets %ld", player->PlayerName, MINUTES(remtime), SECONDS(remtime), THOUSANDTHS(remtime), player->LastPing, player->CarPacketCount);
            swprintf(buf, L"%S = %02d:%02d:%03d, ping %s, packets %ld", player->PlayerName, MINUTES(remtime), SECONDS(remtime), THOUSANDTHS(remtime), L"???", player->CarPacketCount);
//$END_MODIFICATIONS
            DumpText(0, i, 8, 16, 0xc0ffff00, buf);
            i += 16;
        }
    }
#endif

// bump test
return;
    static VEC light = {0, 0, -128};
    static VEC tl = {-64, -64, 0};
    static VEC tr = {64, -64, 0};
    static VEC br = {64, 64, 0};
    static VEC bl = {-64, 64, 0};
    static float tu0, tv0, tu1, tv1, tu2, tv2, tu3, tv3;

    if (Keys[DIK_LEFT]) light.v[X] -= TimeStep * 100.0f;
    if (Keys[DIK_RIGHT]) light.v[X] += TimeStep * 100.0f;
    if (Keys[DIK_UP]) light.v[Y] -= TimeStep * 100.0f;
    if (Keys[DIK_DOWN]) light.v[Y] += TimeStep * 100.0f;

    SubVector(&tl, &light, &vec);
    NormalizeVector(&vec);
    tu0 = 0.0f / 256.0f + vec.v[X] / 64.0f;
    tv0 = 0.0f / 256.0f + vec.v[Y] / 64.0f;

    SubVector(&tr, &light, &vec);
    NormalizeVector(&vec);
    tu1 = 256.0f / 256.0f + vec.v[X] / 64.0f;
    tv1 = 0.0f / 256.0f + vec.v[Y] / 64.0f;

    SubVector(&br, &light, &vec);
    NormalizeVector(&vec);
    tu2 = 256.0f / 256.0f + vec.v[X] / 64.0f;
    tv2 = 256.0f / 256.0f + vec.v[Y] / 64.0f;

    SubVector(&bl, &light, &vec);
    NormalizeVector(&vec);
    tu3 = 0.0f / 256.0f + vec.v[X] / 64.0f;
    tv3 = 256.0f / 256.0f + vec.v[Y] / 64.0f;

    DrawVertsTEX2[0].sx = 192;
    DrawVertsTEX2[0].sy = 112;
    DrawVertsTEX2[0].sz = 0.01f;
    DrawVertsTEX2[0].rhw = 1;
    DrawVertsTEX2[0].color = 0x808080;
    DrawVertsTEX2[0].tu = 0.0f / 256.0f;
    DrawVertsTEX2[0].tv = 0.0f / 256.0f;

    DrawVertsTEX2[1].sx = 192 + 256;
    DrawVertsTEX2[1].sy = 112;
    DrawVertsTEX2[1].sz = 0.01f;
    DrawVertsTEX2[1].rhw = 1;
    DrawVertsTEX2[1].color = 0x808080;
    DrawVertsTEX2[1].tu = 256.0f / 256.0f;
    DrawVertsTEX2[1].tv = 0.0f / 256.0f;

    DrawVertsTEX2[2].sx = 192 + 256;
    DrawVertsTEX2[2].sy = 112 + 256;
    DrawVertsTEX2[2].sz = 0.01f;
    DrawVertsTEX2[2].rhw = 1;
    DrawVertsTEX2[2].color = 0x808080;
    DrawVertsTEX2[2].tu = 256.0f / 256.0f;
    DrawVertsTEX2[2].tv = 256.0f / 256.0f;

    DrawVertsTEX2[3].sx = 192;
    DrawVertsTEX2[3].sy = 112 + 256;
    DrawVertsTEX2[3].sz = 0.01f;
    DrawVertsTEX2[3].rhw = 1;
    DrawVertsTEX2[3].color = 0x808080;
    DrawVertsTEX2[3].tu = 0.0f / 256.0f;
    DrawVertsTEX2[3].tv = 256.0f / 256.0f;

    DrawVertsTEX2[0].tu2 = tu0;
    DrawVertsTEX2[0].tv2 = tv0;

    DrawVertsTEX2[1].tu2 = tu1;
    DrawVertsTEX2[1].tv2 = tv1;

    DrawVertsTEX2[2].tu2 = tu2;
    DrawVertsTEX2[2].tv2 = tv2;

    DrawVertsTEX2[3].tu2 = tu3;
    DrawVertsTEX2[3].tv2 = tv3;

    BLEND_OFF();
    SET_TPAGE(TPAGE_FX1);
    SET_TPAGE2(TPAGE_FX2);

    SET_STAGE_STATE(1, D3DTSS_TEXCOORDINDEX, 1);
    SET_STAGE_STATE(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    SET_STAGE_STATE(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
    SET_STAGE_STATE(1, D3DTSS_COLOROP, D3DTOP_SUBTRACT);

    DRAW_PRIM(D3DPT_TRIANGLEFAN, FVF_TEX2, DrawVertsTEX2, 4, D3DDP_DONOTUPDATEEXTENTS);

    SET_STAGE_STATE(1, D3DTSS_COLOROP, D3DTOP_DISABLE);

    BLEND_ON();
    BLEND_SRC(D3DBLEND_ONE);
    BLEND_DEST(D3DBLEND_ONE);

    DrawPanelSprite(light.v[X] - 16 + 320, light.v[Y] - 16 + 240, 16, 16, 0, 0, 1, 1, 0xffffff);
}

/////////////////////////////////
// draw a control panel sprite //
/////////////////////////////////

//$REVISIT: are values passed-in for x/y/width/height always whole numbers (seemed to be).  If so, maybe modify argument types here...
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
        DrawVertsTEX1[i].sz = 0; //$ADDITION: explicitly set z (even though z-buffer off) to avoid near/far clipping
        DrawVertsTEX1[i].rhw = 1;
    }

// set screen coors

//$MODIFIED
//    DrawVertsTEX1[0].sx = DrawVertsTEX1[3].sx = xstart;
//    DrawVertsTEX1[1].sx = DrawVertsTEX1[2].sx = xstart + xsize;
//    DrawVertsTEX1[0].sy = DrawVertsTEX1[1].sy = ystart;
//    DrawVertsTEX1[2].sy = DrawVertsTEX1[3].sy = ystart + ysize;
    DrawVertsTEX1[0].sx = DrawVertsTEX1[3].sx = xstart - 0.5f;
    DrawVertsTEX1[1].sx = DrawVertsTEX1[2].sx = xstart - 0.5f + xsize;
    DrawVertsTEX1[0].sy = DrawVertsTEX1[1].sy = ystart - 0.5f;
    DrawVertsTEX1[2].sy = DrawVertsTEX1[3].sy = ystart - 0.5f + ysize;
    //$TODO(cprince): need to look at rest of codebase, and see whether this screenspace coord shift is necessary anywhere else!!
    //(Also, should verify that tex coords don't already account for shift, here and elsewhere.)
//$END_MODIFICATIONS

// set uv's

    DrawVertsTEX1[0].tu = DrawVertsTEX1[3].tu = tu;
    DrawVertsTEX1[1].tu = DrawVertsTEX1[2].tu = tu + twidth;
    DrawVertsTEX1[0].tv = DrawVertsTEX1[1].tv = tv;
    DrawVertsTEX1[2].tv = DrawVertsTEX1[3].tv = tv + theight;

// draw

    DRAW_PRIM(D3DPT_TRIANGLEFAN, FVF_TEX1, DrawVertsTEX1, 4, D3DDP_DONOTUPDATEEXTENTS | D3DDP_DONOTCLIP);
}

/////////////////////////
// load and setup drum //
/////////////////////////

void LoadDrum(void)
{
    long i;
    FILE *fp;
    long size;

// load model

    if (!LoadModel("D:\\models\\drum.m", &DrumModel, TPAGE_FONT, 1, LOADMODEL_FORCE_TPAGE, 100)) //$MODIFIED: added "D:\\" at start
        return;

// set up polys

    for (i = 0 ; i < DrumModel.PolyNum ; i++)
    {
        DrumModel.PolyPtr[i].Tpage = TPAGE_FONT;
    }

// load credit list

    fp = fopen("D:\\models\\mt.bin", "rb"); //$MODIFIED: added "D:\\" at start
    if (!fp)
    {
        DrumCredits = 0;
        return;
    }

// alloc credit space

    fseek(fp, -4, SEEK_END);
    size = ftell(fp);
    fread(&DrumCreditNum, sizeof(long), 1, fp);
    DrumCredits = (char*)malloc(size);

// read credits

    fseek(fp, 0, SEEK_SET);
    fread(DrumCredits, size, 1, fp);
    fclose(fp);

// set vars

    DrumRot = 0.0f;
    DrumRotNext = 0.0f;
    DrumRotPos = DRUM_YRES / 4;
    DrumCurrentCredit = (CREDIT*)DrumCredits;
    DrumCurrentCreditNum = 0;
}

/////////////////////
// free drum model //
/////////////////////

void FreeDrum(void)
{
    FreeModel(&DrumModel, 1);
    free(DrumCredits);
}

/////////////////
// update drum //
/////////////////

void UpdateDrum(void)
{
    long i, off;
    MODEL_POLY *mp;
    long lu, lv;
    float tu, tv;
    POLY_RGB *mrgb;
    char ch;

// um?

    if (!AllowCredits)
        return;

// timers

    DrumRot += TimeStep * 0.02f;
    DrumRotNext += TimeStep * 0.02f;

// next line

    if (DrumRotNext >= DRUM_ONE_ROT)
    {
        DrumRotNext -= DRUM_ONE_ROT;
        DrumRotPos = (DrumRotPos + 1) % DRUM_YRES;

        mp = &DrumModel.PolyPtr[DrumRotPos];
        mrgb = &DrumModel.PolyRGB[DrumRotPos];
        off = (DRUM_XRES - DrumCurrentCredit->len) / 2;

        for (i = 0 ; i < DRUM_XRES ; i++, mp += DRUM_YRES, mrgb += DRUM_YRES)
        {
            if (i < off || i > off + DrumCurrentCredit->len)
                ch = 100;
            else
                ch = (DrumCurrentCredit->text[i - off] ^ 255) - 33;

            lu = ch % FONT_PER_ROW;
            lv = ch / FONT_PER_ROW;

            tu = (float)lu * FONT_WIDTH + 1.0f;
            tv = (float)lv * FONT_HEIGHT + 1.0f;

            mp->tu1 = tu / 256.0f;
            mp->tv1 = tv / 256.0f;

            mp->tu2 = (tu + FONT_UWIDTH - 1.0f) / 256.0f;
            mp->tv2 = tv / 256.0f;

            mp->tu3 = (tu + FONT_UWIDTH - 1.0f) / 256.0f;
            mp->tv3 = (tv + FONT_VHEIGHT) / 256.0f;

            mp->tu0 = tu / 256.0f;
            mp->tv0 = (tv + FONT_VHEIGHT) / 256.0f;

            *(long*)&mrgb->rgb[1] = DrumCurrentCredit->rgb1;
            *(long*)&mrgb->rgb[2] = DrumCurrentCredit->rgb1;
            *(long*)&mrgb->rgb[3] = DrumCurrentCredit->rgb2;
            *(long*)&mrgb->rgb[0] = DrumCurrentCredit->rgb2;
        }

        DrumCurrentCredit = (CREDIT*)((char*)DrumCurrentCredit + sizeof(CREDIT) + DrumCurrentCredit->len - 4);
        DrumCurrentCreditNum++;
        if (DrumCurrentCreditNum == DrumCreditNum)
        {
            DrumCurrentCreditNum = 0;
            DrumCurrentCredit = (CREDIT*)DrumCredits;
        }
    }
}

///////////////
// draw drum //
///////////////

void DrawDrum(void)
{
    MAT wmat;

// um?

    if (!AllowCredits)
        return;

// draw

    SetViewport(Camera[CameraCount].X, Camera[CameraCount].Y, Camera[CameraCount].Xsize, Camera[CameraCount].Ysize, 640.0f);
    SetCameraView(&Identity, &ZeroVector, 0.0f);

    RotMatrixX(&wmat, DrumRot);

    ZBUFFER_OFF();
    BLEND_ALPHA();
    BLEND_SRC(D3DBLEND_SRCALPHA);
    BLEND_DEST(D3DBLEND_INVSRCALPHA);
    DrawModel(&DrumModel, &wmat, &DrumPos, 0);
    BLEND_OFF();
    ZBUFFER_ON();

    SetViewport(Camera[CameraCount].X, Camera[CameraCount].Y, Camera[CameraCount].Xsize, Camera[CameraCount].Ysize, BaseGeomPers + Camera[CameraCount].Lens);
    SetCameraView(&Camera[CameraCount].WMatrix, &Camera[CameraCount].WPos, Camera[CameraCount].Shake);
}

///////////////////////////
// load countdown models //
///////////////////////////

void LoadCountdownModels(void)
{
    long i;

    i = LoadModel("D:\\models\\go3.m", &CountdownModels[0], -1, 1, LOADMODEL_FORCE_TPAGE, 100); //$MODIFIED: added "D:\\" at start
    i += LoadModel("D:\\models\\go2.m", &CountdownModels[1], -1, 1, LOADMODEL_FORCE_TPAGE, 100); //$MODIFIED: added "D:\\" at start
    i += LoadModel("D:\\models\\go1.m", &CountdownModels[2], -1, 1, LOADMODEL_FORCE_TPAGE, 100); //$MODIFIED: added "D:\\" at start
    i += LoadModel("D:\\models\\gogo.m", &CountdownModels[3], -1, 1, LOADMODEL_FORCE_TPAGE, 100); //$MODIFIED: added "D:\\" at start

    if (i < 4)
    {
        QuitGame();
    }
}

///////////////////////////
// free countdown models //
///////////////////////////

void FreeCountdownModels(void)
{
    FreeModel(&CountdownModels[0], 1);
    FreeModel(&CountdownModels[1], 1);
    FreeModel(&CountdownModels[2], 1);
    FreeModel(&CountdownModels[3], 1);
}

////////////////////
// draw countdown //
////////////////////

void DrawCountdown(void)
{
    long count, spin;
    REAL f;
    MAT wmat;
    VEC wpos;

// skip if not ready

    if ((!CountdownTime && TotalRaceTime > 1000) || !AllPlayersReady || CountdownTime >= 3000)
        return;

// get counter

    if (CountdownTime) count = CountdownTime + 1000;
    else count = 1000 - TotalRaceTime;

// set mat + pos

    if (count > 3500 || count < 500) spin = 500;
    else spin = count % 1000;

    if (spin < 100) f = (float)(spin - 100) / 400.0f;
    else if (spin > 900) f = (float)(spin - 900) / 400.0f;
    else f = 0.0f;

    RotMatrixY(&wmat, f);

    CopyVec(&CountdownOffset, &wpos);
    if (count > 3800) wpos.v[Z] += (3800 - count);
    else if (count < 200) wpos.v[Z] -= (200 - count);

// draw

    SetViewport(Camera[CameraCount].X, Camera[CameraCount].Y, Camera[CameraCount].Xsize, Camera[CameraCount].Ysize, 640.0f);
    SetCameraView(&Identity, &ZeroVector, 0.0f);

    SetEnvStatic(&wpos, &wmat, 0x404040, 0.0f, 0.0f, 1.0f);
    DrawModel(&CountdownModels[3 - (count / 1000)], &wmat, &wpos, MODEL_ENV);

    SetViewport(Camera[CameraCount].X, Camera[CameraCount].Y, Camera[CameraCount].Xsize, Camera[CameraCount].Ysize, BaseGeomPers + Camera[CameraCount].Lens);
    SetCameraView(&Camera[CameraCount].WMatrix, &Camera[CameraCount].WPos, Camera[CameraCount].Shake);
}

///////////////
// draw drum //
///////////////

void DrawPracticeStars(void)
{
    MAT wmat;
    VEC wpos;
    long i;
    MODEL *model = &LevelModel[StarModelNum].Model;

// any?

    if (!IsSecretFoundPractiseStars(GameSettings.Level))
        return;

    PracticeStarFlash -= (long)(TimeStep * 1000.0f);
    if (PracticeStarFlash < 0) PracticeStarFlash = 0;
    if (PracticeStarFlash & 64)
        return;

// draw

    for (i = 0 ; i < model->PolyNum ; i++)
    {
        model->PolyPtr[i].Type |= POLY_SEMITRANS;
        model->PolyRGB[i].rgb[0].a = 128;
        model->PolyRGB[i].rgb[1].a = 128;
        model->PolyRGB[i].rgb[2].a = 128;
        model->PolyRGB[i].rgb[3].a = 128;
    }

    SetViewport(Camera[CameraCount].X, Camera[CameraCount].Y, Camera[CameraCount].Xsize, Camera[CameraCount].Ysize, 640.0f);
    SetCameraView(&Identity, &ZeroVector, 0.0f);

    RotMatrixY(&wmat, (float)TIME2MS(TimerCurrent) / 2000.0f);

    ResetSemiList();

    SetVector(&wpos, 0.0f, -30.0f, 200.0f);
    SetEnvStatic(&wpos, &wmat, 0xffff00, 0.0f, 0.0f, 1.0f);
    DrawModel(model, &wmat, &wpos, MODEL_ENV | MODEL_GLARE);

    ZBUFFER_OFF();
    DrawSemiList();
    FlushEnvBuckets();
    ZBUFFER_ON();

    SetViewport(Camera[CameraCount].X, Camera[CameraCount].Y, Camera[CameraCount].Xsize, Camera[CameraCount].Ysize, BaseGeomPers + Camera[CameraCount].Lens);
    SetCameraView(&Camera[CameraCount].WMatrix, &Camera[CameraCount].WPos, Camera[CameraCount].Shake);
}

//////////////////////////
// display player names //
//////////////////////////

void DisplayPlayers(void)
{
    BLEND_ALPHA();
    BLEND_SRC(D3DBLEND_SRCALPHA);
    BLEND_DEST(D3DBLEND_INVSRCALPHA);
    ZWRITE_OFF();
    SET_TPAGE(-1);

    for( PLAYER* player = PLR_PlayerHead; player; player = player->next )
    {
        if( player->type == PLAYER_NONE )
            continue;
        if( player == PLR_LocalPlayer )
            continue;

        // Get the player name
        WCHAR strPlayerName[128];
        swprintf( strPlayerName, L"%S%s", player->PlayerName, StartData.PlayerData[player->Slot].Cheating ? L" (Cheat)" : L"" );

        // Calculate the screen position to place the name tag
        VEC vec, vec2;
        CopyVec( &player->car.Body->Centre.Pos, &vec2 );
        vec2.v[Y] -= ( player->car.CarType == CARID_PANGA ? 100.0f : 72.0f );
        RotTransVector( &ViewMatrix, &ViewTrans, &vec2, &vec );
        if( vec.v[Z] < 0.0f || vec.v[Z] > 8191.0f )
            continue;
        if( GameSettings.Mirrored )
            vec.v[X] = -vec.v[X];

        // Pick a color for the player
        MODEL_RGB col;
        *(long*)&col = MultiPlayerColours[player->Slot];
        if( vec.v[Z] > 3072.0f ) 
            col.a = (unsigned char)((4095.0f - vec.v[Z]) / 4.0f);
        else 
            col.a = 255;
        DWORD dwColor = *(long*)&col;

        // Write the name into a texture
        //$TODO - we should create this texture elsewhere
        if( player->pNameTexture == NULL )
        {
            player->pNameTexture = g_pFont->CreateTexture( strPlayerName, 0x00000000, dwColor );
        }
        D3DDevice_SetTexture( 0, player->pNameTexture );

        // Compute some vector positioning numbers
        D3DSURFACE_DESC desc;
        player->pNameTexture->GetLevelDesc( 0, &desc );
        FLOAT fWidth  = (FLOAT)desc.Width;
        FLOAT fHeight = (FLOAT)desc.Height;

        FLOAT sx = vec.v[X] * RenderSettings.GeomPers / vec.v[Z] * RenderSettings.GeomScaleX + RenderSettings.GeomCentreX;
        FLOAT sy = vec.v[Y] * RenderSettings.GeomPers / vec.v[Z] * RenderSettings.GeomScaleY + RenderSettings.GeomCentreY;
        FLOAT sz = GET_ZBUFFER(vec.v[Z]);
        FLOAT rhw = 1 / vec.v[Z];
        FLOAT s = 1.1f - vec.v[Z]/12000.0f;

        // Draw a quad with the texture of the player's name on it
        struct VERTEX
        {
            D3DXVECTOR4 p;
            FLOAT       tu, tv;
        };

        VERTEX v[4];
        v[0].p = D3DXVECTOR4( sx-s*fWidth/2, sy-s*fHeight, sz, rhw ); v[0].tu =  0.0f;  v[0].tv =  0.0f;
        v[1].p = D3DXVECTOR4( sx+s*fWidth/2, sy-s*fHeight, sz, rhw ); v[1].tu = fWidth; v[1].tv =  0.0f;
        v[2].p = D3DXVECTOR4( sx+s*fWidth/2,     sy,       sz, rhw ); v[2].tu = fWidth; v[2].tv = fHeight;
        v[3].p = D3DXVECTOR4( sx-s*fWidth/2,     sy,       sz, rhw ); v[3].tu =  0.0f;  v[3].tv = fHeight;

        // draw
        DRAW_PRIM(D3DPT_TRIANGLEFAN, D3DFVF_XYZRHW|D3DFVF_TEX1, v, 4, D3DDP_DONOTUPDATEEXTENTS);
    };

    BLEND_OFF();
	ZWRITE_ON();
;

//////////////////
// init console //
//////////////////

void InitConsole(void)
{
    ConsoleTimer = 0.0f;
}

/////////////////////////
// set console message //
/////////////////////////

void SetConsoleMessage(WCHAR *message1, WCHAR *message2, long rgb1, long rgb2, long pri, REAL time)
{
    if (!ConsoleTimer || pri >= ConsolePri)
    {
        ConsoleRGB1 = rgb1;
        ConsoleRGB2 = rgb2;
        ConsolePri = pri;
        ConsoleTimer = time;//2.0f;
        ConsoleStartTime = time;
        if (message1 != NULL) 
            wcsncpy(ConsoleMessage1, message1, CONSOLE_MESSAGE_MAX);
        else 
            ConsoleMessage1[0] = L'\0';
        if (message2 != NULL) 
            wcsncpy(ConsoleMessage2, message2, CONSOLE_MESSAGE_MAX);
        else 
            ConsoleMessage2[0] = L'\0';
        ConsoleMessage1[CONSOLE_MESSAGE_MAX - 1] = 0;
        ConsoleMessage2[CONSOLE_MESSAGE_MAX - 1] = 0;
    }
}

/////////////////////////////
// draw championship table //
/////////////////////////////

static void DrawChampionshipTable(void)
{
    long i, j;
    WCHAR buf[128];
    int  x;
    float offmax;

// draw

    x = (640 - (CHAMP_TABLE_NAME_WIDTH + (CupTable.RaceNum + 1) * CHAMP_TABLE_POS_WIDTH + CHAMP_TABLE_PTS_WIDTH)) / 2 + (short)ChampionshipTableOffset;

    DumpText(x, 176, 8, 16, MENU_COLOR_OPAQUE|MENU_COLOR_CYAN, TEXT_TABLE(TEXT_CAR));

    for (i = 0 ; i < CupTable.RaceNum + 1 ; i++)
    {
        swprintf(buf, L"%ld", i + 1);
        DumpText(x + CHAMP_TABLE_NAME_WIDTH + i * CHAMP_TABLE_POS_WIDTH + CHAMP_TABLE_POS_WIDTH / 2 - 4, 176, 8, 16, MENU_COLOR_OPAQUE|MENU_COLOR_CYAN, buf);
    }

    DumpText(x + CHAMP_TABLE_NAME_WIDTH + (CupTable.RaceNum + 1) * CHAMP_TABLE_POS_WIDTH + CHAMP_TABLE_PTS_WIDTH / 2 - (wcslen(TEXT_TABLE(TEXT_CHAMP_ROWTITLE_POINTS)) * 8) / 2, 176, 8, 16, MENU_COLOR_OPAQUE|MENU_COLOR_CYAN, TEXT_TABLE(TEXT_CHAMP_ROWTITLE_POINTS));

    for (i = 0 ; i < StartData.PlayerNum ; i++)
    {
        swprintf(buf, L"%0.15S", Players[CupTable.PlayerOrder[i].PlayerSlot].PlayerName);
        DumpText(x, 208 + i * 16, 8, 16, MENU_TEXT_RGB_NORMAL, buf);

        for (j = 0 ; j < CupTable.RaceNum + 1 ; j++)
        {
            swprintf( buf, L"%s", TEXT_TABLE(TEXT_1ST + CupTable.PlayerOrder[i].FinishPos[j]) );
            DumpText(x + CHAMP_TABLE_NAME_WIDTH + j * CHAMP_TABLE_POS_WIDTH + CHAMP_TABLE_POS_WIDTH / 2 - 12, 208 + i * 16, 8, 16, MENU_COLOR_OPAQUE|MENU_COLOR_GREEN, buf);
        }

        swprintf(buf, L"%2.2ld", CupTable.PlayerOrder[i].Points);
        DumpText(x + CHAMP_TABLE_NAME_WIDTH + j * CHAMP_TABLE_POS_WIDTH + CHAMP_TABLE_PTS_WIDTH / 2 - 8, 208 + i * 16, 8, 16, MENU_COLOR_OPAQUE|MENU_COLOR_YELLOW, buf);

        if (CupTable.PlayerOrder[i].NewPoints)
        {
            swprintf(buf, L"+%ld", CupTable.PlayerOrder[i].NewPoints);
            DumpText(x + CHAMP_TABLE_NAME_WIDTH + j * CHAMP_TABLE_POS_WIDTH + CHAMP_TABLE_PTS_WIDTH / 2 + 16, 208 + i * 16, 8, 16, MENU_COLOR_OPAQUE|MENU_COLOR_RED, buf);
        }
    }

// draw 'congrats' message

    if (CupTable.RaceNum == CupData[CupTable.CupType].NRaces - 1)
    {
        DumpText(CENTRE_POS(TEXT_CHAMP_FINISHED1) - (int)ChampionshipTableOffset + 8, 100, 8, 16, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_CHAMP_FINISHED1));
        DumpText(CENTRE_POS(TEXT_CHAMP_FINISHED2) - (int)ChampionshipTableOffset + 8, 116, 8, 16, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_CHAMP_FINISHED2));
    }
    else
    {
        DumpText(CENTRE_POS(TEXT_CHAMP_QUALIFIED1) - (int)ChampionshipTableOffset + 8, 100, 8, 16, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_CHAMP_QUALIFIED1));
        DumpText(CENTRE_POS(TEXT_CHAMP_QUALIFIED2) - (int)ChampionshipTableOffset + 8, 116, 8, 16, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_CHAMP_QUALIFIED2));
    }

// set offset

    if (ChampionshipEndMode == CHAMPIONSHIP_END_QUALIFIED) offmax = 0.0f;
    else offmax = 640.0f;

    ChampionshipTableOffset += TimeStep * MENU_MOVE_VEL;
    if (ChampionshipTableOffset > offmax)
            ChampionshipTableOffset = offmax;
}

/////////////////////////////
// draw championship table //
/////////////////////////////

static void DrawChampionshipFailed(void)
{
    float offmax;

// draw 'failed' message

    DumpText((640 - wcslen(TEXT_TABLE(TEXT_CHAMP_FAILURE)) * 8) / 2 - (int)ChampionshipTableOffset + 8, 100, 8, 16, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_CHAMP_FAILURE));

// set offset

    if (ChampionshipEndMode == CHAMPIONSHIP_END_FAILED) offmax = 0.0f;
    else offmax = 640.0f;

    ChampionshipTableOffset += TimeStep * MENU_MOVE_VEL;
    if (ChampionshipTableOffset > offmax)
            ChampionshipTableOffset = offmax;
}
}
