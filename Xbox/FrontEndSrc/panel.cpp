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
#include "menudraw.h"
#include "ai.h"
#include "pickup.h"
#include "InitPlay.h"
#include "settings.h"
#include "menutext.h"

// menus
#include "WaitingRoom.h"

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
long AllowCredits = FALSE;
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
static char ConsoleMessage1[CONSOLE_MESSAGE_MAX];
static char ConsoleMessage2[CONSOLE_MESSAGE_MAX];
static float ChampionshipTableOffset, RaceTableOffset;

char *SpeedUnitText[SPEED_NTYPES] = {
    "mph",
    "mph",
    "fpm",
    "kph",
    "kph",
};

static REAL SpeedUnitScale[SPEED_NTYPES] = {
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
    0, 224,     // shockwave
    64, 225,    // firework
    96, 224,    // firework pack
    32, 224,    // putty bomb
    128, 224,   // water bomb
    224, 224,   // electro pulse
    192, 224,   // oil slick
    0, 192,     // chrome ball
    32, 192,    // turbo
    160, 224,   // clone
    96, 192,    // global
};

// multiplayer colours

long MultiPlayerColours[] = {
    0xff0000,
    0x00ff00,
    0x0000ff,

    0xff00ff,
    0x00ffff,
    0xffff00,

    0xff0080,
    0x80ff00,
    0x0080ff,

    0xff80ff,
    0x80ffff,
    0xffff80,
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




////////////////////////
// draw control panel //
////////////////////////
void DrawControlPanel(void)
{
    float x, y, xsize, ysize, tu, tv, twidth, theight, *p, frevs, dist, nearestdistahead, nearestdistbehind, localdist, f;
    long i, j, k, col, speed, revs, revdest, revper, revadd, pickup, pickup2, pos, time;
    unsigned long lastping;
    char buf[128];
    PLAYER *player, *playerahead, *playerbehind;
    VEC vec, vec2;

// skip if editing

    if (CAM_MainCamera->Type == CAM_EDIT)
        return;

// zbuffer off

    ZBUFFER_OFF();

// draw spru boxes

    if (GameSettings.GameType != GAMETYPE_PRACTICE && GameSettings.GameType != GAMETYPE_TRAINING)
        DrawSpruBox(12.0f * RenderSettings.GeomScaleX, 428.0f * RenderSettings.GeomScaleY, 160.0f * RenderSettings.GeomScaleX, 40.0f * RenderSettings.GeomScaleY, 0, 0);

    if (PLR_LocalPlayer->RaceFinishTime && (ChampionshipEndMode == CHAMPIONSHIP_END_FINISHED || ChampionshipEndMode == CHAMPIONSHIP_END_WAITING_FOR_FINISH || ChampionshipEndMode == CHAMPIONSHIP_END_QUALIFIED || ChampionshipEndMode == CHAMPIONSHIP_END_FAILED))
    {
        if (GameSettings.GameType == GAMETYPE_CLOCKWORK)
        {
            y = 60.0f;
            ysize = (30.0f + 12.0f * StartData.PlayerNum);
        }
        else
        {
            y = 154.0f;
            ysize = (38.0f + 16.0f * StartData.PlayerNum);
        }

        DrawSpruBox(
            216.0f * RenderSettings.GeomScaleX,
            (y + RaceTableOffset) * RenderSettings.GeomScaleY,
            208.0f * RenderSettings.GeomScaleX,
            ysize * RenderSettings.GeomScaleY,
            0, 0);
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
            xsize = (float)(strlen(TEXT_TABLE(TEXT_CHAMP_FINISHED1)) > strlen(TEXT_TABLE(TEXT_CHAMP_FINISHED2)) ? strlen(TEXT_TABLE(TEXT_CHAMP_FINISHED1)) : strlen(TEXT_TABLE(TEXT_CHAMP_FINISHED2)));
        else
            xsize = (float)(strlen(TEXT_TABLE(TEXT_CHAMP_QUALIFIED1)) > strlen(TEXT_TABLE(TEXT_CHAMP_QUALIFIED2)) ? strlen(TEXT_TABLE(TEXT_CHAMP_QUALIFIED1)) : strlen(TEXT_TABLE(TEXT_CHAMP_QUALIFIED2)));
        
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
            ((640 - strlen(TEXT_TABLE(TEXT_CHAMP_FAILURE)) * 8.0f) / 2 - 4.0f - ChampionshipTableOffset + 8) * RenderSettings.GeomScaleX,
            96 * RenderSettings.GeomScaleY,
            (strlen(TEXT_TABLE(TEXT_CHAMP_FAILURE)) * 8.0f + 8.0f) * RenderSettings.GeomScaleX,
            24.0f * RenderSettings.GeomScaleY,
            0, 0);
    }

    if (ChallengeFlash != CHALLENGE_FLASH_TIME && ChallengeFlash & 128)
    {
        DrawSpruBox(
            (CENTRE_POS(TEXT_PANEL_CHALLENGE_BEATEN) - 8) * RenderSettings.GeomScaleX,
            232 * RenderSettings.GeomScaleY,
            (strlen(TEXT_TABLE(TEXT_PANEL_CHALLENGE_BEATEN)) * 8 + 16) * RenderSettings.GeomScaleX,
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
                DrawPanelSprite((float)i * 20 + 16, 16, 16, 16, 65.0f / 256.0f, 241.0f / 256.0f, 14.0f / 256.0f, 14.0f / 256.0f, 0xc0ffffff);
            else
                DrawPanelSprite((float)i * 20 + 16, 16, 16, 16, 81.0f / 256.0f, 241.0f / 256.0f, 14.0f / 256.0f, 14.0f / 256.0f, 0xc0ffffff);
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
            DrawPanelSprite(288, 16, 64, 64, 129.0f / 256.0f, 65.0f / 256.0f, 62.0f / 256.0f, 62.0f / 256.0f, 0xc0ffffff);

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

            DrawPanelSprite(288, 16, 64, 64, tu, tv, twidth, theight, col);
        }

        if ((PLR_LocalPlayer->TrackDirCount -= (long)(TimeStep * 720)) < 0) PLR_LocalPlayer->TrackDirCount = 0;
    }

// normal game dir arrow!

    if (GameSettings.GameType != GAMETYPE_PRACTICE  && GameSettings.GameType != GAMETYPE_TRAINING && GameSettings.GameType != GAMETYPE_BATTLE)
    {
        for (i = 0 ; i < 4 ; i++)
        {
            DrawVertsTEX1[i].sx = (-(float)sin(PLR_LocalPlayer->CarAI.AngleToNextNode + ((float)i / 4.0f * RAD)) * 32.0f * (GameSettings.Mirrored ? -1.0f : 1.0f) + 96.0f) * RenderSettings.GeomScaleX + ScreenLeftClip;
            DrawVertsTEX1[i].sy = ((float)cos(PLR_LocalPlayer->CarAI.AngleToNextNode + ((float)i / 4.0f * RAD)) * 32.0f + 396.0f) * RenderSettings.GeomScaleY + ScreenTopClip;
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

    if (GameSettings.GameType == GAMETYPE_BATTLE && FoxObj && FoxObj->player != PLR_LocalPlayer && !PLR_LocalPlayer->RaceFinishTime)
    {
        SubVector(&FoxObj->player->car.Body->Centre.Pos, &PLR_LocalPlayer->car.Body->Centre.Pos, &vec);
        TransposeRotVector(&PLR_LocalPlayer->car.Body->Centre.WMatrix, &vec, &vec2);
        f = (float)atan2(vec2.v[X], vec2.v[Z]) + RAD * 3.0f / 8.0f;

        for (i = 0 ; i < 4 ; i++)
        {
            DrawVertsTEX1[i].sx = (-(float)sin(f + ((float)i / 4.0f * RAD)) * 48.0f + 320.0f) * RenderSettings.GeomScaleX + ScreenLeftClip;
            DrawVertsTEX1[i].sy = ((float)cos(f + ((float)i / 4.0f * RAD)) * 48.0f + 48.0f) * RenderSettings.GeomScaleY + ScreenTopClip;
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

    if (GameSettings.GameType != GAMETYPE_TRIAL && GameSettings.GameType != GAMETYPE_PRACTICE && GameSettings.GameType != GAMETYPE_TRAINING && GameSettings.AllowPickups)
    {
        DrawPanelSprite(16, 56, 64, 64, 190.0f / 256.0f, 157.0f / 256.0f, 64.0f / 256.0f, 64.0f / 256.0f, 0xe0ffffff);

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
                DrawPanelSprite(32, 72, 32, 32, tu, tv, 32.0f / 256.0f, 32.0f / 256.0f, 0x808080 - col);
            } else {
                DrawPanelSprite(32, 72, 32, 32, tu, tv, 32.0f / 256.0f, 32.0f / 256.0f, 0xffffff - col);
            }

            if (col)
            {
                tu = (PickupUV[pickup2 * 2]) / 256.0f;
                tv = (PickupUV[pickup2 * 2 + 1]) / 256.0f;
                DrawPanelSprite(32, 72, 32, 32, tu, tv, 32.0f / 256.0f, 32.0f / 256.0f, col);
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
                DrawPanelSprite(20.0f, 128.0f, 32.0f, 32.0f, 64.0f / 256.0f, 192.0f / 256.0f, 32.0f / 256.0f, 32.0f / 256.0f, 0xc0ffffff);
                break;
            }
        }
    }

// normal font

    SET_TPAGE(TPAGE_FONT);

// stunt arena star count

    if (GameSettings.GameType == GAMETYPE_TRAINING)
    {
        sprintf(buf, "%2.2ld   %2.2ld", StarList.NumFound, StarList.NumTotal);
        DrawGameText(640 - 72, 16, 0xffffffff, buf);
        DrawGameText(640 - 48, 16, 0xff00ffff, "/");
    }

// pickup num

    if (GameSettings.GameType != GAMETYPE_TRIAL && GameSettings.GameType != GAMETYPE_PRACTICE  && GameSettings.GameType != GAMETYPE_TRAINING && PLR_LocalPlayer->PickupNum && (PLR_LocalPlayer->PickupType == PICKUP_TYPE_FIREWORKPACK) || (PLR_LocalPlayer->PickupType == PICKUP_TYPE_WATERBOMB))
    {
        sprintf(buf, "%d", PLR_LocalPlayer->PickupNum);
        DrawSmallGameText(60, 102, 0xffffaa22, buf);
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

        pos = (640 - (strlen(ConsoleMessage1) + strlen(ConsoleMessage2)) * 8) / 2;
        DrawGameText( (FLOAT)pos, 448, ConsoleRGB1 | i << 24, ConsoleMessage1);
        DrawGameText( (FLOAT)pos + strlen(ConsoleMessage1) * 8, 448, ConsoleRGB2 | i << 24, ConsoleMessage2);
    }

	// split time?
    if (PLR_LocalPlayer->DisplaySplitCount && GameSettings.GameType != GAMETYPE_PRACTICE && GameSettings.GameType != GAMETYPE_TRAINING)
    {
        PLR_LocalPlayer->DisplaySplitCount -= TimeStep;
        if (PLR_LocalPlayer->DisplaySplitCount < 0) PLR_LocalPlayer->DisplaySplitCount = 0;

        DrawGameText(CENTRE_POS(TEXT_PANEL_GHOSTSPLIT), 128, 0xff00ffff, TEXT_TABLE(TEXT_PANEL_GHOSTSPLIT));
        if (PLR_LocalPlayer->DisplaySplitTime > 0) wsprintf(buf, "+%02d:%02d:%03d", MINUTES(PLR_LocalPlayer->DisplaySplitTime), SECONDS(PLR_LocalPlayer->DisplaySplitTime), THOUSANDTHS(PLR_LocalPlayer->DisplaySplitTime));
        else wsprintf(buf, "-%02d:%02d:%03d", MINUTES(-PLR_LocalPlayer->DisplaySplitTime), SECONDS(-PLR_LocalPlayer->DisplaySplitTime), THOUSANDTHS(-PLR_LocalPlayer->DisplaySplitTime));
        DrawGameText(280, 144, 0xffffffff, buf);
    }

	// lap time
    if (PLR_LocalPlayer->RaceFinishTime == 0  && GameSettings.GameType != GAMETYPE_BATTLE && GameSettings.GameType != GAMETYPE_PRACTICE && GameSettings.GameType != GAMETYPE_TRAINING)
    {
        DrawGameText(RIGHT_JUSTIFY_POS(TEXT_PANEL_LAPTIME, 624), 96, 0xff00ffff, TEXT_TABLE(TEXT_PANEL_LAPTIME));
        wsprintf(buf, "%02d:%02d:%03d", MINUTES(PLR_LocalPlayer->car.CurrentLapTime), SECONDS(PLR_LocalPlayer->car.CurrentLapTime), THOUSANDTHS(PLR_LocalPlayer->car.CurrentLapTime));
        DrawGameText(552, 112, 0xffffffff, buf);
    }

	// bomb tag timer
    if (GameSettings.GameType == GAMETYPE_BATTLE)
    {
        DrawGameText(RIGHT_JUSTIFY_POS(TEXT_PANEL_TAGTIME, 624), 16, 0xff00ffff, TEXT_TABLE(TEXT_PANEL_TAGTIME));
        wsprintf(buf, "%02d:%02d:%03d", MINUTES(PLR_LocalPlayer->BombTagTimer), SECONDS(PLR_LocalPlayer->BombTagTimer), THOUSANDTHS(PLR_LocalPlayer->BombTagTimer));
        DrawGameText(552, 32, 0xffffffff, buf);

        if (FoxObj)
        {
            DrawGameText(16, 16, 0xffff0000, FoxObj->player->PlayerName);
            wsprintf(buf, "%02d:%02d:%03d", MINUTES(FoxObj->player->BombTagTimer), SECONDS(FoxObj->player->BombTagTimer), THOUSANDTHS(FoxObj->player->BombTagTimer));
            DrawGameText(16, 32, 0xffffffff, buf);
        }
    }

// race time

    if (GameSettings.GameType != GAMETYPE_TRIAL && GameSettings.GameType != GAMETYPE_BATTLE && GameSettings.GameType != GAMETYPE_PRACTICE && GameSettings.GameType != GAMETYPE_TRAINING)
    {
        if (PLR_LocalPlayer->RaceFinishTime == 0) {
            DrawGameText(RIGHT_JUSTIFY_POS(TEXT_PANEL_RACETIME, 624), 136, 0xff00ffff, TEXT_TABLE(TEXT_PANEL_RACETIME));
            wsprintf(buf, "%02d:%02d:%03d", MINUTES(TotalRaceTime), SECONDS(TotalRaceTime), THOUSANDTHS(TotalRaceTime));
            DrawGameText(552, 152, 0xffffffff, buf);
        } else {
            DrawGameText(RIGHT_JUSTIFY_POS(TEXT_PANEL_RACETIME, 624), 96, 0xff00ffff, TEXT_TABLE(TEXT_PANEL_RACETIME));
            wsprintf(buf, "%02d:%02d:%03d", MINUTES(PLR_LocalPlayer->RaceFinishTime), SECONDS(PLR_LocalPlayer->RaceFinishTime), THOUSANDTHS(PLR_LocalPlayer->RaceFinishTime));
            DrawGameText(552, 112, 0xffffffff, buf);
        }
    }

// record lap

    if (GameSettings.GameType == GAMETYPE_TRIAL && TrackRecords.RecordLap[0].Time != MAX_LAP_TIME)
    {
        DrawGameText(RIGHT_JUSTIFY_POS(TEXT_RECORD, 624), 136, 0xffffff00, TEXT_TABLE(TEXT_RECORD));
        wsprintf(buf, "%02d:%02d:%03d", MINUTES(TrackRecords.RecordLap[0].Time), SECONDS(TrackRecords.RecordLap[0].Time), THOUSANDTHS(TrackRecords.RecordLap[0].Time));
        DrawGameText(552, 152, 0xffffffff, buf);
    }

// ghost lap

    if (GameSettings.GameType == GAMETYPE_TRIAL && GHO_GhostDataExists)
    {
        DrawGameText(RIGHT_JUSTIFY_POS(TEXT_PANEL_GHOSTTIME, 624), 176, 0xffffff00, TEXT_TABLE(TEXT_PANEL_GHOSTTIME));
        wsprintf(buf, "%02d:%02d:%03d", MINUTES(GHO_BestGhostInfo->Time[MAX_SPLIT_TIMES]), SECONDS(GHO_BestGhostInfo->Time[MAX_SPLIT_TIMES]), THOUSANDTHS(GHO_BestGhostInfo->Time[MAX_SPLIT_TIMES]));
        DrawGameText(552, 192, 0xffffffff, buf);
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
                DrawGameText(RIGHT_JUSTIFY_POS(TEXT_PANEL_CHALLENGETIME, 624), 216, 0xffffff00, TEXT_TABLE(TEXT_PANEL_CHALLENGETIME));
                wsprintf(buf, "%02d:%02d:%03d", MINUTES(time), SECONDS(time), THOUSANDTHS(time));
                DrawGameText(552, 232, 0xffffffff, buf);
            }
            if (ChallengeFlash != CHALLENGE_FLASH_TIME && ChallengeFlash & 128)
            {
                DrawGameText(CENTRE_POS(TEXT_PANEL_CHALLENGE_BEATEN), 236, 0xffffffff, TEXT_TABLE(TEXT_PANEL_CHALLENGE_BEATEN));
            }
        }
        else
        {
        }
    }

// speed

    DrawGameText(600, 448, 0xff00ffff, SpeedUnitText[SpeedUnits]);
    speed = (long)(SpeedUnitScale[SpeedUnits] * VecLen(&PLR_LocalPlayer->car.Body->Centre.Vel));
    wsprintf(buf, "%4d", speed);
    DrawGameText(560, 448, 0xffffffff, buf);

    if ((SpeedUnits == SPEED_SCALEDMPH) || (SpeedUnits == SPEED_SCALEDKPH)) {
        DrawSmallGameText(RIGHT_JUSTIFY_POS(TEXT_SCALED, 624), 464, 0x4000ffff, TEXT_TABLE(TEXT_SCALED));
    }

// best acc
//  wsprintf(buf, "0-15: %3dms", (PLR_LocalPlayer->car.Best0to15 > ZERO)? (int)(1000 * PLR_LocalPlayer->car.Best0to15): 0);
//  DrawWideGameText(100, 50, 0xc0ffffff, buf);
//  wsprintf(buf, "0-25: %3dms", (PLR_LocalPlayer->car.Best0to25 > ZERO)? (int)(1000 * PLR_LocalPlayer->car.Best0to25): 0);
//  DrawWideGameText(100, 70, 0xc0ffffff, buf);
//  wsprintf(buf, "ValidNode: %d (%d)", PLR_LocalPlayer->ValidRailCamNode, (PLR_LocalPlayer->ValidRailCamNode == -1)? -1: CAM_CameraNode[PLR_LocalPlayer->ValidRailCamNode].ID);
//  DrawWideGameText(100, 50, 0xffff0000, buf);
//  wsprintf(buf, "BangMag: %d", (int)(100 * PLR_LocalPlayer->car.Body->BangMag));
//  DrawWideGameText(100, 50, 0xffff0000, buf);
//  wsprintf(buf, "Down: %8d %8d %8d (%8d)", (int)(100 * DEBUG_DownForce.v[X]), (int)(100 * DEBUG_DownForce.v[Y]), (int)(100 * DEBUG_DownForce.v[Z]), (int)(100 * PLR_LocalPlayer->car.DownForceMod));
//  DrawSmallGameText(100, 50, 0xffff0000, buf);
//  wsprintf(buf, "StuckTime: %8d", (int)(1000.0f * PLR_LocalPlayer->CarAI.StuckCnt));
//  DrawSmallGameText(100, 70, 0xffff0000, buf);
//  wsprintf(buf, "RPL Buffer %8d (%8d)", ReplayBufferBytesStored, ReplayDataBufSize);
//  DrawSmallGameText(100, 50, 0xffff0000, buf);
//  wsprintf(buf, "RaceTime: %8d   OverlapTime: %8d", TotalRacePhysicsTime, OverlapTime);
//  DrawSmallGameText(100, 70, 0xffff0000, buf);
//  wsprintf(buf, "Timeout: %s", (PLR_LocalPlayer->car.RemoteTimeout)? "TRUE": "FALSE");
//  DrawSmallGameText(100, 70, 0xffff0000, buf);
//  sprintf(buf, "%f %f %f %f", time0, time1, time2, timeNow);
//  DrawGameText(100, 70, 0xffff0000, buf);
//  sprintf(buf, "%d", ServerID);
//  DrawGameText(100, 70, 0xffff0000, buf);
//  sprintf(buf, "%f", PLR_LocalPlayer->car.LastHitTimer);
//  DrawGameText(100, 70, 0xffff0000, buf);

//  sprintf(buf, "Tests: %4d Passes: %4d (Max: %4d)", COL_NCollsTested, COL_NCollsPassed, PosToCollGrid(&PLR_LocalPlayer->car.Body->Centre.Pos)->NCollPolys);
//  DrawGameText(100, 70, 0xffff0000, buf);

//  sprintf(buf, "Body-Body: %4d", COL_NBodyBodyTests);
//  DrawGameText(100, 90, 0xffff0000, buf);

/*  // Wheel contact information
    sprintf(buf, "Floor: %3s;  Wall: %3s;  Side: %3s  Other: %3s",
        (IsWheelInFloorContact(&PLR_LocalPlayer->car.Wheel[0]))? "Yes": "No",
        (IsWheelInWallContact(&PLR_LocalPlayer->car.Wheel[0]))? "Yes": "No",
        (IsWheelInSideContact(&PLR_LocalPlayer->car.Wheel[0]))? "Yes": "No",
        (IsWheelInOtherContact(&PLR_LocalPlayer->car.Wheel[0]))? "Yes": "No");
    DrawSmallGameText(50, 50, 0xffffff00, buf);

    sprintf(buf, "Floor: %3s;  Wall: %3s;  Side: %3s  Other: %3s",
        (IsWheelInFloorContact(&PLR_LocalPlayer->car.Wheel[1]))? "Yes": "No",
        (IsWheelInWallContact(&PLR_LocalPlayer->car.Wheel[1]))? "Yes": "No",
        (IsWheelInSideContact(&PLR_LocalPlayer->car.Wheel[1]))? "Yes": "No",
        (IsWheelInOtherContact(&PLR_LocalPlayer->car.Wheel[1]))? "Yes": "No");
    DrawSmallGameText(50, 70, 0xffffff00, buf);

    sprintf(buf, "Floor: %3s;  Wall: %3s;  Side: %3s  Other: %3s",
        (IsWheelInFloorContact(&PLR_LocalPlayer->car.Wheel[2]))? "Yes": "No",
        (IsWheelInWallContact(&PLR_LocalPlayer->car.Wheel[2]))? "Yes": "No",
        (IsWheelInSideContact(&PLR_LocalPlayer->car.Wheel[2]))? "Yes": "No",
        (IsWheelInOtherContact(&PLR_LocalPlayer->car.Wheel[2]))? "Yes": "No");
    DrawSmallGameText(50, 90, 0xffffff00, buf);

    sprintf(buf, "Floor: %3s;  Wall: %3s;  Side: %3s  Other: %3s",
        (IsWheelInFloorContact(&PLR_LocalPlayer->car.Wheel[3]))? "Yes": "No",
        (IsWheelInWallContact(&PLR_LocalPlayer->car.Wheel[3]))? "Yes": "No",
        (IsWheelInSideContact(&PLR_LocalPlayer->car.Wheel[3]))? "Yes": "No",
        (IsWheelInOtherContact(&PLR_LocalPlayer->car.Wheel[3]))? "Yes": "No");
    DrawSmallGameText(50, 110, 0xffffff00, buf);
*/

    if (GameSettings.Level == GetLevelNum("frontend")) {
        VEC look;
        sprintf(buf, "Pos:  %f %f %f", CAM_MainCamera->WPos.v[X], CAM_MainCamera->WPos.v[Y], CAM_MainCamera->WPos.v[Z]);
        DrawGameText(100, 70, 0xffffffff, buf);
        VecPlusScalarVec(&CAM_MainCamera->WPos, 1000, &CAM_MainCamera->WMatrix.mv[L], &look);
        sprintf(buf, "Look: %f %f %f", look.v[X], look.v[Y], look.v[Z]);
        DrawGameText(100, 90, 0xffffffff, buf);
    }

// laps

    if (GameSettings.GameType != GAMETYPE_TRIAL && GameSettings.GameType != GAMETYPE_BATTLE && GameSettings.GameType != GAMETYPE_PRACTICE && GameSettings.GameType != GAMETYPE_TRAINING)
    {
        DrawGameText(16, 16, 0xff00ffff, TEXT_TABLE(TEXT_PANEL_LAPCOUNTER));
        if (PLR_LocalPlayer->car.Laps < GameSettings.NumberOfLaps) {
            wsprintf(buf, "%d/%d", PLR_LocalPlayer->car.Laps + 1, GameSettings.NumberOfLaps);
        } else {
            sprintf(buf, TEXT_TABLE(TEXT_FINISHED));
        }
        DrawGameText(16, 32, 0xffffffff, buf);
    }

// calc position, ahead, behind players

    if (GameSettings.GameType != GAMETYPE_PRACTICE && GameSettings.GameType != GAMETYPE_TRAINING)
    {
        pos = 1;
        nearestdistahead = FLT_MAX;
        nearestdistbehind = -FLT_MAX;

        if (GameSettings.GameType == GAMETYPE_TRIAL)
            localdist = -PLR_LocalPlayer->CarAI.FinishDistPanel - (float)PLR_LocalPlayer->CarAI.BackTracking;
        else if (GameSettings.GameType == GAMETYPE_BATTLE)
            localdist = (float)(BOMBTAG_MAX_TIME - PLR_LocalPlayer->BombTagTimer) / 1000.0f;
        else
            localdist = (float)PLR_LocalPlayer->car.Laps - PLR_LocalPlayer->CarAI.FinishDistPanel - (float)PLR_LocalPlayer->CarAI.BackTracking;

        AllPlayersFinished = (long)PLR_LocalPlayer->RaceFinishTime;

        for (player = PLR_PlayerHead ; player ; player = player->next) if (player != PLR_LocalPlayer && player->type != PLAYER_NONE)
        {
            if (GameSettings.GameType == GAMETYPE_BATTLE)
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
                AllPlayersFinished = FALSE;
            }
        }

        if (PLR_LocalPlayer->RaceFinishTime)
            LocalRacePos = PLR_LocalPlayer->RaceFinishPos + 1;
        else
            LocalRacePos = pos;

// show pos, ahead, behind players

        wsprintf(buf, "%d", LocalRacePos);
        DrawBigGameText(16, 392, 0xffffffff, buf);
        DrawSmallGameText(LocalRacePos >= 10 ? 40.0f : 28.0f, 392.0f, 0xff00ffff, LocalRacePos < 4 ? TEXT_TABLE(TEXT_ST_NUMABBREV + LocalRacePos - 1) : TEXT_TABLE(TEXT_TH_NUMABBREV));

        if (GameSettings.GameType == GAMETYPE_BATTLE)
        {
            if (nearestdistahead != FLT_MAX)
            {
                dist = (nearestdistahead - localdist);
                FTOL(dist * 12.75f, col);
                if (col > 511) col = 511;
                if (col < 256) col = col << 16 | 0xff00ff00;
                else col = (511 - col) << 8 | 0xffff0000;

                wsprintf(buf, "-%lds", (long)dist);
                DrawGameText(16, 432, col, buf);
                wsprintf(buf, "%0.12s", playerahead->PlayerName);
                DrawGameText(72, 432, 0xffffffff, buf);
            }

            if (nearestdistbehind != -FLT_MAX)
            {
                dist = (localdist - nearestdistbehind);
                FTOL(dist * 12.75f, col);
                if (col > 511) col = 511;
                if (col < 256) col = col << 8 | 0xffff0000;
                else col = (511 - col) << 16 | 0xff00ff00;

                wsprintf(buf, "+%lds", (long)dist);
                DrawGameText(16, 448, col, buf);
                wsprintf(buf, "%0.12s", playerbehind->PlayerName);
                DrawGameText(72, 448, 0xffffffff, buf);
            }
        }
        else
        {
            if (nearestdistahead != FLT_MAX)
            {
                dist = (nearestdistahead - localdist) * PosTotalDist / 200.0f;
                FTOL(dist * 10.24f, col);
                if (col > 511) col = 511;
                if (col < 256) col = col << 16 | 0xff00ff00;
                else col = (511 - col) << 8 | 0xffff0000;

                wsprintf(buf, "-%ldm", (long)dist);
                DrawGameText(16, 432, col, buf);
                wsprintf(buf, "%0.12s", playerahead->PlayerName);
                DrawGameText(72, 432, 0xffffffff, buf);
            }

            if (nearestdistbehind != -FLT_MAX)
            {
                dist = (localdist - nearestdistbehind) * PosTotalDist / 200.0f;
                FTOL(dist * 10.24f, col);
                if (col > 511) col = 511;
                if (col < 256) col = col << 8 | 0xffff0000;
                else col = (511 - col) << 16 | 0xff00ff00;

                wsprintf(buf, "+%ldm", (long)dist);
                DrawGameText(16, 448, col, buf);
                wsprintf(buf, "%0.12s", playerbehind->PlayerName);
                DrawGameText(72, 448, 0xffffffff, buf);
            }
        }
    }

// final position

    if (PLR_LocalPlayer->RaceFinishTime)
    {
        LocalRacePosScale -= TimeStep * 500.0f;
        if (LocalRacePosScale < 32.0f)
            LocalRacePosScale = 32.0f;

        if (LocalRacePosScale < 300.0f - 256.0f)
            col = 0xffffffff;
        else
            col = (long)((300.0f - LocalRacePosScale)) << 24 | 0xffffff;

        xsize = LocalRacePosScale;
        ysize = xsize * 2.0f;
        x = 320.0f - xsize * 0.5f;
        y = 0.0f;

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

        DrawPanelSprite(x, y, xsize, ysize, tu, tv, 32.0f / 256.0f, 32.0f / 256.0f, col);

// Remove the rearview and other player camera

        if (CAM_RearCamera != NULL)
        {
            RemoveCamera(CAM_RearCamera);
            CAM_RearCamera = NULL;
        }
        if (CAM_OtherCamera != NULL && GameSettings.GameType != GAMETYPE_BATTLE)
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

    if (GameSettings.GameType != GAMETYPE_BATTLE && GameSettings.GameType != GAMETYPE_PRACTICE && GameSettings.GameType != GAMETYPE_TRAINING)
    {
        DrawGameText(RIGHT_JUSTIFY_POS(TEXT_PANEL_LASTLAP, 624), 16, 0xff00ffff, TEXT_TABLE(TEXT_PANEL_LASTLAP));
        wsprintf(buf, "%02d:%02d:%03d", MINUTES(PLR_LocalPlayer->car.LastLapTime), SECONDS(PLR_LocalPlayer->car.LastLapTime), THOUSANDTHS(PLR_LocalPlayer->car.LastLapTime));
        DrawGameText(552, 32, 0xffffffff, buf);
    }

// best lap

    if (GameSettings.GameType != GAMETYPE_BATTLE && GameSettings.GameType != GAMETYPE_PRACTICE && GameSettings.GameType != GAMETYPE_TRAINING)
    {
        DrawGameText(RIGHT_JUSTIFY_POS(TEXT_PANEL_BESTLAP, 624), 56, 0xff00ffff, TEXT_TABLE(TEXT_PANEL_BESTLAP));
        wsprintf(buf, "%02d:%02d:%03d", MINUTES(PLR_LocalPlayer->car.BestLapTime), SECONDS(PLR_LocalPlayer->car.BestLapTime), THOUSANDTHS(PLR_LocalPlayer->car.BestLapTime));
        DrawGameText(552, 72, 0xffffffff, buf);
    }

// finish times

    if (PLR_LocalPlayer->RaceFinishTime && (ChampionshipEndMode == CHAMPIONSHIP_END_FINISHED || ChampionshipEndMode == CHAMPIONSHIP_END_WAITING_FOR_FINISH || ChampionshipEndMode == CHAMPIONSHIP_END_QUALIFIED || ChampionshipEndMode == CHAMPIONSHIP_END_FAILED))
    {
        if (GameSettings.GameType == GAMETYPE_CLOCKWORK)
        {
            j = 12;
            k = 86 + (short)RaceTableOffset;
        }
        else
        {
            j = 16;
            k = 190 + (short)RaceTableOffset;
        }

        for (i = pos = 0 ; i < StartData.PlayerNum ; i++) if (FinishTable[i].Time)
        {
            if (!pos)
			{
		        if (GameSettings.GameType == GAMETYPE_CLOCKWORK)
			        DrawSmallGameText(218, (FLOAT)(k - j * 2), 0xff00ffff, GameSettings.GameType == GAMETYPE_BATTLE ? TEXT_TABLE(TEXT_PANEL_BATTLE_RESULTS) : TEXT_TABLE(TEXT_PANEL_RACE_RESULTS));
				else
			        DrawGameText(218, (FLOAT)(k - j * 2), 0xff00ffff, GameSettings.GameType == GAMETYPE_BATTLE ? TEXT_TABLE(TEXT_PANEL_BATTLE_RESULTS) : TEXT_TABLE(TEXT_PANEL_RACE_RESULTS));
			}

            if (FinishTable[i].Player == PLR_LocalPlayer)
            {
                col = (long)(sin((float)TIME2MS(TimerCurrent) / 200.0f) * 64.0f + 192.0f);
                LocalRacePos = pos + 1;
            }
            else
            {
                col = 255;
            }

	        if (GameSettings.GameType == GAMETYPE_CLOCKWORK)
			{
				wsprintf(buf, "%2.2d", i + 1);
				DrawSmallGameText(218, (short)(pos * j + k), col | col << 8 | col << 16 | col << 24, buf);
				wsprintf(buf, "%0.12s", FinishTable[i].Player->PlayerName);
				DrawSmallGameText(242, (short)(pos * j + k), col | col << 8 | col << 24, buf);
				wsprintf(buf, "%02d:%02d:%03d", MINUTES(FinishTable[i].Time), SECONDS(FinishTable[i].Time), THOUSANDTHS(FinishTable[i].Time));
				DrawSmallGameText(242 + 8 * 13, (short)(pos * j + k), col | col << 8 | col << 16 | col << 24, buf);
			}
			else
			{
				wsprintf(buf, "%2.2d", i + 1);
				DrawGameText(218, (short)(pos * j + k), col | col << 8 | col << 16 | col << 24, buf);
				wsprintf(buf, "%0.12s", FinishTable[i].Player->PlayerName);
				DrawGameText(242, (short)(pos * j + k), col | col << 8 | col << 24, buf);
				wsprintf(buf, "%02d:%02d:%03d", MINUTES(FinishTable[i].Time), SECONDS(FinishTable[i].Time), THOUSANDTHS(FinishTable[i].Time));
				DrawGameText(242 + 8 * 13, (short)(pos * j + k), col | col << 8 | col << 16 | col << 24, buf);
			}
			pos++;
        }

        if (gMenuHeader.pMenu || ChampionshipEndMode == CHAMPIONSHIP_END_QUALIFIED || ChampionshipEndMode == CHAMPIONSHIP_END_FAILED)
        {
            RaceTableOffset -= TimeStep * MENU_MOVE_VEL;
            if (RaceTableOffset < -480.0f) RaceTableOffset = -480.0f;
        }
        else
        {
            RaceTableOffset += TimeStep * MENU_MOVE_VEL;
            if (RaceTableOffset > 0.0f) RaceTableOffset = 0.0f;
        }
    }
    else
    {
        RaceTableOffset = -480.0f;
    }

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

    if (IsMultiPlayer() && !AllPlayersReady)
    {
        i = 64;
        for (player = PLR_PlayerHead ; player ; player = player->next)
        {
            if (player->type != PLAYER_NONE && !player->Ready)
            {
                DrawGameText((FLOAT)(640 - strlen(player->PlayerName) * 8) / 2, (short)i, 0xffffffff, player->PlayerName);
                i += 16;
            }
        }

        if (i == 64 && !IsServer())
            DrawGameText(CENTRE_POS(TEXT_WAITINGFORHOST), 32, 0xff00ffff, TEXT_TABLE(TEXT_WAITINGFORHOST));
        else
            DrawGameText(CENTRE_POS(TEXT_WAITINGFOR), 32, 0xff00ffff, TEXT_TABLE(TEXT_WAITINGFOR));

        if (IsServer())
        {
            wsprintf(buf, "%ld", (long)AllReadyTimeout);
            DrawGameText(272 + 8 * 13, 32, 0xffff0000, buf);
        }
    }

// host quit?

    if (HostQuit && IsMultiPlayer())
    {
        if (!(TIME2MS(TimerCurrent) & 256))
        {
            DrawGameText(CENTRE_POS(TEXT_MULTIGAMETERMINATED), 160, 0xff00ff00, TEXT_TABLE(TEXT_MULTIGAMETERMINATED));
        }
    }

// show fps?

    if (RegistrySettings.ShowFPS)
    {
        DrawSmallGameText(144, 392, 0xff00ffff, "fps");
//$MODIFIED:
//        wsprintf(buf, "%d", FrameRate);
        wsprintf(buf, "%d", (int)FrameRate_GetFPS());
//$END_MODIFICATIONS
        DrawSmallGameText(144, 404, 0xffffffff, buf);
    }

// multi messages

    if (IsMultiPlayer())
    {
        if (WaitingRoomMessageTimer)
        {
            FTOL(WaitingRoomMessageTimer * 256.0f, j);
            if (j > 255) j = 255;
            j <<= 24;

            for (i = 2 ; i < WAITING_ROOM_MESSAGE_NUM ; i++)
            {
                DrawSmallGameText((FLOAT)(640 - strlen(WaitingRoomMessages[i]) * 8) / 2, 56.0f + i * 12, WaitingRoomMessageRGB[i] | j, WaitingRoomMessages[i]);
            }
        }

        if (WaitingRoomMessageActive)
        {
            i = strlen(WaitingRoomCurrentMessage) * 8;
            DrawSmallGameText((FLOAT)(640 - i) / 2, 56 + 12 * 6, 0xffffffff, WaitingRoomCurrentMessage);
            if (!(TIME2MS(TimerCurrent) & 256)) 
				DrawSmallGameText((FLOAT)(320 + i / 2), 56 + 12 * 6, 0xffffffff, "_");
        }
    }

// multiplayer info?

#if MULTIPLAYER_DEBUG
    static long ShowMultiDebug = false;
    if (Keys[DIK_NUMPADENTER] && !LastKeys[DIK_NUMPADENTER])
        ShowMultiDebug = !ShowMultiDebug;

    if (IsMultiPlayer() && ShowMultiDebug)
    {
        wsprintf(buf, "Send %ld", GetSendQueue(0));
        DrawGameText(0, 128, 0xff00ffff, buf);

        wsprintf(buf, "PPS %ld", (long)PacketsPerSecond);
        DrawGameText(0, 144, 0xff00ffff, buf);

        wsprintf(buf, "Data Sent %ld - Per Sec %ld", TotalDataSent, TotalDataSent / (TotalRaceTime / 1000));
        DrawGameText(0, 160, 0xff00ffff, buf);

        i = 192;
        for (player = PLR_PlayerHead ; player ; player = player->next) if (player->type == PLAYER_REMOTE)
        {
            unsigned long remtime = (unsigned long)player->car.RemoteData[player->car.NewDat].Time;

            wsprintf(buf, "%s = %02d:%02d:%03d, ping %ld, packets %ld", player->PlayerName, MINUTES(remtime), SECONDS(remtime), THOUSANDTHS(remtime), player->LastPing, player->CarPacketCount);
            DrawGameText(0, (short)i, 0xc0ffff00, buf);
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

    //$REVISIT -- might need to put back this z-set code if clipping bug wasn't fixed in Acclaim's latest code drop.
    //
    ////$ADDITION(jedl) - set screen z to 0.  Polygons were being clipped randomly near and far, so the control panel was flickering
    //DrawVertsTEX1[0].sz
    // = DrawVertsTEX1[1].sz
    //    = DrawVertsTEX1[2].sz
    //       = DrawVertsTEX1[3].sz = 0.f;
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
        g_bQuitGame = TRUE;
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
    VEC vec, vec2;
    PLAYER *player;
    MODEL_RGB col;
    char buf[128];
    float xsize, ysize;

    BLEND_ALPHA();
    BLEND_SRC(D3DBLEND_SRCALPHA);
    BLEND_DEST(D3DBLEND_INVSRCALPHA);

    for (player = PLR_PlayerHead ; player ; player = player->next) if (player->type != PLAYER_NONE)
    {
        if (player == PLR_LocalPlayer) continue;

        wsprintf(buf, "%s%s", player->PlayerName, StartData.PlayerData[player->Slot].Cheating ? " (Cheat)" : "");

        CopyVec(&player->car.Body->Centre.Pos, &vec2);
        vec2.v[Y] -= (player->car.CarType == CARID_PANGA ? 100.0f : 72.0f);
        RotTransVector(&ViewMatrix, &ViewTrans, &vec2, &vec);
        if (vec.v[Z] < 0.0f || vec.v[Z] > 4095.0f) continue;
        if (GameSettings.Mirrored) vec.v[X] = -vec.v[X];

        xsize = vec.v[Z] / 192.0f + 12.0f;
        ysize = vec.v[Z] / 128.0f + 18.0f;

        vec.v[X] -= strlen(buf) * xsize * 0.5f;

        *(long*)&col = MultiPlayerColours[player->Slot];
        if (vec.v[Z] > 3072.0f) col.a = (unsigned char)((4095.0f - vec.v[Z]) / 4.0f);
        else col.a = 255;

        DumpText3D(&vec, xsize, ysize, *(long*)&col, buf);
    }

    BLEND_OFF();
}

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

void SetConsoleMessage(char *message1, char *message2, long rgb1, long rgb2, long pri, REAL time)
{
    if (!ConsoleTimer || pri >= ConsolePri)
    {
        ConsoleRGB1 = rgb1;
        ConsoleRGB2 = rgb2;
        ConsolePri = pri;
        ConsoleTimer = time;//2.0f;
        ConsoleStartTime = time;
        if (message1 != NULL) 
            strncpy(ConsoleMessage1, message1, CONSOLE_MESSAGE_MAX);
        else 
            ConsoleMessage1[0] = '\0';
        if (message2 != NULL) 
            strncpy(ConsoleMessage2, message2, CONSOLE_MESSAGE_MAX);
        else 
            ConsoleMessage2[0] = '\0';
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
    char buf[128];
    short x;
    float offmax;

// draw

    x = (short)(640 - (CHAMP_TABLE_NAME_WIDTH + (CupTable.RaceNum + 1) * CHAMP_TABLE_POS_WIDTH + CHAMP_TABLE_PTS_WIDTH)) / 2 + (short)ChampionshipTableOffset;

    DrawGameText(x, 176, 0xff00ffff, TEXT_TABLE(TEXT_CAR));

    for (i = 0 ; i < CupTable.RaceNum + 1 ; i++)
    {
        wsprintf(buf, "%ld", i + 1);
        DrawGameText((FLOAT)x + CHAMP_TABLE_NAME_WIDTH + i * CHAMP_TABLE_POS_WIDTH + CHAMP_TABLE_POS_WIDTH / 2 - 4, 176, 0xff00ffff, buf);
    }

    DrawGameText((FLOAT)x + CHAMP_TABLE_NAME_WIDTH + (CupTable.RaceNum + 1) * CHAMP_TABLE_POS_WIDTH + CHAMP_TABLE_PTS_WIDTH / 2 - (strlen(TEXT_TABLE(TEXT_CHAMP_ROWTITLE_POINTS)) * 8) / 2, 176.0f, 0xff00ffff, TEXT_TABLE(TEXT_CHAMP_ROWTITLE_POINTS));

    for (i = 0 ; i < StartData.PlayerNum ; i++)
    {
        wsprintf(buf, "%0.15s", Players[CupTable.PlayerOrder[i].PlayerSlot].PlayerName);
        DrawGameText((FLOAT)x, 208.0f + i * 16, 0xffffffff, buf);

        for (j = 0 ; j < CupTable.RaceNum + 1 ; j++)
        {
            wsprintf(buf, "%ld%s", CupTable.PlayerOrder[i].FinishPos[j] + 1, CupTable.PlayerOrder[i].FinishPos[j] < 3 ? TEXT_TABLE(TEXT_ST_NUMABBREV + CupTable.PlayerOrder[i].FinishPos[j]) : TEXT_TABLE(TEXT_TH_NUMABBREV));
            DrawGameText((FLOAT)x + CHAMP_TABLE_NAME_WIDTH + j * CHAMP_TABLE_POS_WIDTH + CHAMP_TABLE_POS_WIDTH / 2 - 12, 208.0f + i * 16, 0xff00ff00, buf);
        }

        wsprintf(buf, "%2.2ld", CupTable.PlayerOrder[i].Points);
        DrawGameText((FLOAT)x + CHAMP_TABLE_NAME_WIDTH + j * CHAMP_TABLE_POS_WIDTH + CHAMP_TABLE_PTS_WIDTH / 2 - 8, 208.0f + i * 16, 0xffffff00, buf);

        if (CupTable.PlayerOrder[i].NewPoints)
        {
            wsprintf(buf, "+%ld", CupTable.PlayerOrder[i].NewPoints);
            DrawGameText((FLOAT)x + CHAMP_TABLE_NAME_WIDTH + j * CHAMP_TABLE_POS_WIDTH + CHAMP_TABLE_PTS_WIDTH / 2 + 16, 208.0f + i * 16, 0xffff0000, buf);
        }
    }

// draw 'congrats' message

    if (CupTable.RaceNum == CupData[CupTable.CupType].NRaces - 1)
    {
        DrawGameText(CENTRE_POS(TEXT_CHAMP_FINISHED1) - (short)ChampionshipTableOffset + 8, 100, 0xffffffff, TEXT_TABLE(TEXT_CHAMP_FINISHED1));
        DrawGameText(CENTRE_POS(TEXT_CHAMP_FINISHED2) - (short)ChampionshipTableOffset + 8, 116, 0xffffffff, TEXT_TABLE(TEXT_CHAMP_FINISHED2));
    }
    else
    {
        DrawGameText(CENTRE_POS(TEXT_CHAMP_QUALIFIED1) - (short)ChampionshipTableOffset + 8, 100, 0xffffffff, TEXT_TABLE(TEXT_CHAMP_QUALIFIED1));
        DrawGameText(CENTRE_POS(TEXT_CHAMP_QUALIFIED2) - (short)ChampionshipTableOffset + 8, 116, 0xffffffff, TEXT_TABLE(TEXT_CHAMP_QUALIFIED2));
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

    DrawGameText((FLOAT)(640 - strlen(TEXT_TABLE(TEXT_CHAMP_FAILURE)) * 8) / 2 - (short)ChampionshipTableOffset + 8, 100, 0xffffffff, TEXT_TABLE(TEXT_CHAMP_FAILURE));

// set offset

    if (ChampionshipEndMode == CHAMPIONSHIP_END_FAILED) offmax = 0.0f;
    else offmax = 640.0f;

    ChampionshipTableOffset += TimeStep * MENU_MOVE_VEL;
    if (ChampionshipTableOffset > offmax)
            ChampionshipTableOffset = offmax;
}
