//-----------------------------------------------------------------------------
// File: gamegauge.cpp
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"
#include "gamegauge.h"
#include "LevelInfo.h"
#include "LevelLoad.h"
#include "main.h"
#include "spark.h"
#include "player.h"
#include "initplay.h"
#include "timing.h"
#include "text.h"
#include "ghost.h"
#include "gameloop.h"
#include "draw.h"
#include "settings.h"
#include "ui_TitleScreen.h"
#include "ui_MenuText.h"

// globals

#define MAX_FPS_COUNT 60 * 5

unsigned long GAME_GAUGE = FALSE;
unsigned long START_TIME;
unsigned long FRAMES_ONE_SECOND;
unsigned long FRAMES_TOTAL;
unsigned long MIN_ONE_SECOND;
unsigned long MAX_ONE_SECOND;
unsigned long CUR_ONE_SECOND;
unsigned long TOTAL_START_TIME;

static long FirstSecond;
static long FpsCount;
static short FpsList[MAX_FPS_COUNT];

//$MODIFIED
//static char GameGaugeFile[] = {"levels\\muse2\\Game Gauge Replay.rpl"};
static char GameGaugeFile[] = {"D:\\levels\\muse2\\Game Gauge Replay.rpl"};
//$END_MODIFICATIONS

/////////////////////////////////
// setup game gauge demo level //
/////////////////////////////////

void SetupGameGaugeDemo(void)
{
    LEVELINFO *levelinfo;

    // game settings
//  GameSettings.GameType = GAMETYPE_TRIAL;
//  GameSettings.Level = LEVEL_MUSEUM2;
    GameSettings.DrawRearView = FALSE;
    GameSettings.DrawFollowView = FALSE;
    GameSettings.PlayMode = MODE_ARCADE;
//  GameSettings.Mirrored = FALSE;
//  GameSettings.Reversed = FALSE;
//  GameSettings.NumberOfLaps = -1;
//  GameSettings.AllowPickups = TRUE;

    // render settings
    RenderSettings.Env = TRUE;
    RenderSettings.Light = TRUE;
    RenderSettings.Instance = TRUE;
    RenderSettings.Mirror = TRUE;
    RenderSettings.Shadow = TRUE;
    RenderSettings.Skid = TRUE;
    gSparkDensity = ONE;

            FILE *fp;

            fp = fopen(GameGaugeFile, "rb");
            if (fp == NULL) {
                DumpMessage(GameGaugeFile, "Can't find Game Gauge data");
            } else {
                if (LoadReplayData(fp)) {

                    GameSettings.GameType = GAMETYPE_REPLAY;
                    g_bTitleScreenRunGame = TRUE;

                } else {
                    DumpMessage(GameGaugeFile, "Failed loading Game Gauge data");
                }
                fclose(fp);
            }

            // Set the level settings
            GameSettings.GameType = GAMETYPE_REPLAY;//StartData.GameType;
            GameSettings.Level = GetLevelNum(StartData.LevelDir);
            GameSettings.Mirrored = StartData.Mirrored;
            GameSettings.Reversed = StartData.Reversed;
            GameSettings.NumberOfLaps = StartData.Laps;
            GameSettings.AllowPickups = StartData.AllowPickups;

    // start data
    levelinfo = GetLevelInfo(GameSettings.Level);

//  InitStartData();
//  strncpy(StartData.LevelDir, levelinfo->Dir, MAX_LEVEL_DIR_NAME);
//  AddPlayerToStartData(PLAYER_NONE, 0, CARID_TOYECA, 0, 0, CTRL_TYPE_NONE, 0, "Dummy");
//  StartData.PlayerData[0].GridNum = 0;

    // force rand seed
    srand(0);

    // event
    SET_EVENT(SetupGame);
}

///////////////////////////
// setup game gauge vars //
///////////////////////////

void SetupGameGaugeVars(void)
{
    TOTAL_START_TIME = START_TIME = CurrentTimer();
    FRAMES_ONE_SECOND = 0;
    FRAMES_TOTAL = 0;
    MIN_ONE_SECOND = 0xffffffff;
    MAX_ONE_SECOND = 0;
    CUR_ONE_SECOND = 0;

    FirstSecond = TRUE;
    FpsCount = 0;

    //SetCameraFollow(CAM_MainCamera, GHO_GhostPlayer->ownobj, CAM_FOLLOW_BEHIND);
    SetCameraFollow(CAM_MainCamera, PLR_LocalPlayer->ownobj, CAM_FOLLOW_BEHIND);
//  CountdownTime = 0;
    PLR_LocalPlayer->CarAI.PreLap = FALSE;
}

////////////////////////////
// update game gauge vars //
////////////////////////////

void UpdateGameGaugeVars(void)
{
    unsigned long time;

// update ticks

    FRAMES_ONE_SECOND++;
    FRAMES_TOTAL++;

// new second?

    time = CurrentTimer();
    if (TIME2MS(time - START_TIME) >= 1000)
    {
        if (!FirstSecond)
        {
            if (FRAMES_ONE_SECOND < MIN_ONE_SECOND)
                MIN_ONE_SECOND = FRAMES_ONE_SECOND;

            if (FRAMES_ONE_SECOND > MAX_ONE_SECOND)
                MAX_ONE_SECOND = FRAMES_ONE_SECOND;

            if (FpsCount < MAX_FPS_COUNT)
            {
                FpsList[FpsCount++] = (short)FRAMES_ONE_SECOND;
            }
        }

        FirstSecond = FALSE;

        CUR_ONE_SECOND = FRAMES_ONE_SECOND;
        FRAMES_ONE_SECOND = 0;

        START_TIME = time;
    }

// finished?

    //if (GHO_BestFrame == GHO_BestGhostInfo->NFrames - 1 && GameLoopQuit == GAMELOOP_QUIT_OFF)
    if (ReachedEndOfReplay && GameLoopQuit == GAMELOOP_QUIT_OFF)
    {
        GameLoopQuit = GAMELOOP_QUIT_GAMEGAUGE;
        SetFadeEffect(FADE_DOWN);
        OutputGameGaugeData();
    }
}

//////////////////////////
// draw game gauge info //
//////////////////////////

#define GAME_GAUGE_RGB 0x80ffffff

void DrawGameGaugeInfo(void)
{
    WCHAR buf[256];

// render states

    FOG_OFF();
    ZBUFFER_OFF();
    BLEND_ALPHA();
    BLEND_SRC(D3DBLEND_SRCALPHA);
    BLEND_DEST(D3DBLEND_INVSRCALPHA);
    SET_TPAGE(TPAGE_FONT);

// info

//$MODIFIED
//    swprintf(buf, L"Draw Device: %s", DrawDevices[CurrentDrawDevice].Name);
    swprintf(buf, L"Draw Device: %s", L"Xbox draw device");
//$END_MODIFICATIONS
    DumpText(16, 16, 8, 16, GAME_GAUGE_RGB, buf);

    swprintf(buf, L"Resolution: %ld x %ld x %ld", ScreenXsize, ScreenYsize, ScreenBpp);
    DumpText(16, 40, 8, 16, GAME_GAUGE_RGB, buf);

    swprintf(buf, L"Textures: %s", RegistrySettings.Texture24 ? L"24 Bit" : L"16 Bit");
    DumpText(16, 64, 8, 16, GAME_GAUGE_RGB, buf);

    DumpText(16, 88, 8, 16, GAME_GAUGE_RGB, BackBufferCount == 1 ? L"Double Buffered" : L"Triple Buffered");

    swprintf(buf, L"Time: %.2f", (float)TIME2MS(CurrentTimer() - TOTAL_START_TIME) / 1000.0f);
    DumpText(16, 112, 8, 16, GAME_GAUGE_RGB, buf);

    swprintf(buf, L"Frames: %ld", FRAMES_TOTAL);
    DumpText(16, 134, 8, 16, GAME_GAUGE_RGB, buf);

    swprintf(buf, L"Cur FPS: %ld", CUR_ONE_SECOND);
    DumpText(16, 158, 8, 16, GAME_GAUGE_RGB, buf);

    if (MIN_ONE_SECOND != 0xffffffff)
    {
        swprintf(buf, L"Min FPS: %ld", MIN_ONE_SECOND);
        DumpText(16, 182, 8, 16, GAME_GAUGE_RGB, buf);
    }

    if (MAX_ONE_SECOND)
    {
        swprintf(buf, L"Max FPS: %ld", MAX_ONE_SECOND);
        DumpText(16, 206, 8, 16, GAME_GAUGE_RGB, buf);
    }
}

///////////////////
// ouput fps.txt //
///////////////////

void OutputGameGaugeData(void)
{
    FILE *fp;
    long i;
    float secs, fps;

// open file

//$MODIFIED
//    fp = fopen("fps.txt", "w");
    fp = fopen("D:\\fps.txt", "w");
//$END_MODIFICATIONS
    if (!fp)
    {
        DumpMessage(NULL,"Failed to write 'fps.txt'");
        return;
    }

// write average, min, max fps

    secs = (float)TIME2MS(CurrentTimer() - TOTAL_START_TIME) / 1000.0f;
    fps = FRAMES_TOTAL / secs;
    fprintf(fp, "%.2f Re-Volt v1.00\n", fps);
    fprintf(fp, "%ld Min\n", MIN_ONE_SECOND);
    fprintf(fp, "%ld Max\n", MAX_ONE_SECOND);

// write fps list

    for (i = 0 ; i < FpsCount ; i++)
    {
        fprintf(fp, "%d Second %ld\n", FpsList[i], i + 1);
    }

// close file

    fclose(fp);
}

