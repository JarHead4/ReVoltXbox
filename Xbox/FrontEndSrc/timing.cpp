//-----------------------------------------------------------------------------
// File: timing.cpp
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"
#include "timing.h"
#include "ctrlread.h"
#include "object.h"
#include "player.h"
#include "aizone.h"
#ifdef _PC
#include "settings.h"
#endif
#include "camera.h"
#ifdef _PC
#include "input.h"
//#include "gamegauge.h"
#endif
#include "NewColl.h"
#include "Particle.h"
#include "Body.h"
#include "Wheel.h"
#include "Car.h"
#include "Geom.h"
#include "Main.h"
#include "Control.h"
#include "trigger.h"
#include "Ghost.h"
#include "panel.h"
#ifdef _N64
#include "gfx.h"
#include "ISound.h"
#endif
#include "timing.h"
#include "posnode.h"
#include "TitleScreen.h"

#include "SoundEffectEngine.h"
#include "SoundEffects.h"

#include "Menu2.h"


//#define GAZZA_TEACH_CAR_HANDLING

#ifdef _N64
#define SFX_PRIORITY_COUNTDOWN 4
#else
#define SFX_PRIORITY_COUNTDOWN 2
#endif

// globals

unsigned long TimerLast, RealTimerDiff, TimerDiff, TimerFreq, TotalRaceTime, TotalRacePhysicsTime, TotalRaceStartTime, CountdownTime, CountdownEndTime;
long RaceStartModifier = 0;
REAL TimerSlowDownPercentage = ZERO;
unsigned long OverlapTime = 0;

#ifdef _PC
unsigned long TimerCurrent = CurrentTimer();
RECORD_ENTRY TrackRecords;
#else
unsigned long TimerCurrent = 0;
RECORD_ENTRY TrackRecords[LEVEL_NCUP_LEVELS];
#endif

static long RecordLapFlag[MAX_RECORD_TIMES];
static long RecordRaceFlag[MAX_RECORD_TIMES];


////////////////////////////////////////////////////////////////
//
// init track records array
//
////////////////////////////////////////////////////////////////
#ifndef _PC
void InitTrackRecords()
{
    int iLevel, ii, jj;

    for (iLevel = 0; iLevel < LEVEL_NCUP_LEVELS; iLevel++) {

        for (ii = 0; ii < MAX_SPLIT_TIMES; ii++) {
            TrackRecords[iLevel].SplitTime[ii] = MAX_LAP_TIME;
        }

        for (ii = 0; ii < MAX_RECORD_TIMES; ii++) {
            TrackRecords[iLevel].RecordLap[ii].Time = MAX_LAP_TIME;
            TrackRecords[iLevel].RecordLap[ii].CarType = CARID_RC;
            strcpy(TrackRecords[iLevel].RecordLap[ii].Player, "Player");
        }
    }
}
#endif

/////////////////////
// update TimeStep //
/////////////////////

#ifdef _N64
void UpdateTimeStep(void)
{
// set last / current / diff timer

    TimerLast = TimerCurrent;
    TimerCurrent = CurrentTimer();
    TimerDiff = TimerCurrent - TimerLast;

// save real time diff

    RealTimerDiff = TimerDiff;

// Cap "frame rate" to min of 15fps 

    if (TimerDiff > MS2TIME(67)) TimerDiff = MS2TIME(67);

// Slow down time and calculate the modification needed to player's race/lap start times

    if (!GameSettings.Paws) 
    {
        unsigned long slowPercent;
        FTOL(TimerSlowDownPercentage, slowPercent)

        /*
        if (RenderSettings.Sepia)
        {
            slowPercent -= 20;
        }
        */

        RaceStartModifier = TimerDiff;
        TimerDiff = (TimerDiff * (100 - slowPercent))/100;
        RaceStartModifier -= TimerDiff;
        TimerLast = TimerCurrent - TimerDiff;
    }
    else 
    {
        RaceStartModifier = 0;
    }

// set TimeFactor
    TimeFactor = (REAL)(TimerDiff) / 1000 * PhysicalFramesPerSecond;
    if (TimeFactor > 15) TimeFactor = 15;

    TimeStep = TimeFactor / PhysicalFramesPerSecond;

    //TimeStep = ((REAL)TimerDiff) / ((REAL)TimerFreq);
}
#endif

/////////////////////
// update TimeStep //
/////////////////////

#ifdef _PC
void UpdateTimeStep(void)
{

// set last / current / diff timer
    {
        TimerLast = TimerCurrent;
        TimerCurrent = CurrentTimer();
        TimerDiff = TimerCurrent - TimerLast;
    }

// save real time diff

    RealTimerDiff = TimerDiff;

// cap time diff to 100 ms

    if (TimerDiff > MS2TIME(100))
    {
        TimerDiff = MS2TIME(100);
        TimerLast = TimerCurrent - TimerDiff;
    }

// save real timestep

    RealTimeStep = ((REAL)TimerDiff) / ((REAL)TimerFreq);

// Slow down time and calculate the modification needed to player's race/lap start times

    if (!GameSettings.Paws) 
    {
        unsigned long slowPercent;
        FTOL(TimerSlowDownPercentage, slowPercent)

        if (RenderSettings.Sepia)
        {
            slowPercent -= 20;
        }

#ifdef GAZZA_TEACH_CAR_HANDLING
        slowPercent -= 500;
#endif

        RaceStartModifier = TimerDiff;
        TimerDiff = (TimerDiff * (100 - slowPercent))/100;
        RaceStartModifier -= TimerDiff;
        TimerLast = TimerCurrent - TimerDiff;
    }
    else 
    {
        RaceStartModifier = 0;
    }

// set time step

#if FIXED_TIME_STEP
    TimeStep = (REAL)1.0f / 100.0f;
#else
    TimeStep = ((REAL)TimerDiff) / ((REAL)TimerFreq);
#endif
}
#endif


//////////////////////////
// return current timer //
//////////////////////////

#ifdef _PC
unsigned long CurrentTimer(void)
{
    __int64 time;
    QueryPerformanceCounter((LARGE_INTEGER*)&time);
    return (unsigned long)(time >> TIMER_SHIFT);
}
#endif

#ifdef _N64
unsigned long CurrentTimer(void)
{
    return(OS_CYCLES_TO_USEC(osGetTime()) / 1000);
}
#endif

////////////////////////////////////////////////////////////////
// InitCountDown:
////////////////////////////////////////////////////////////////

void InitCountDown()
{
    CountdownTime = COUNTDOWN_START;
    TimerCurrent = CurrentTimer();
    CountdownEndTime = TimerCurrent + MS2TIME(COUNTDOWN_START);
    TotalRaceStartTime = TimerCurrent;
    TotalRaceTime = TotalRacePhysicsTime = OverlapTime = 0;
    TimerSlowDownPercentage = 0;
    TimeStep = Real(0);
}

// set timers with an additional time already elapsed in milliseconds
void InitCountDownDelta(unsigned long deltaTime)
{
    if (deltaTime < COUNTDOWN_START) {
        CountdownTime = COUNTDOWN_START - deltaTime;
    } else {
        CountdownTime = 0;
    }
    TimerCurrent = CurrentTimer();
    CountdownEndTime = TimerCurrent + MS2TIME(CountdownTime);
    TotalRaceStartTime = TimerCurrent - MS2TIME(deltaTime);
    TotalRaceTime = TotalRacePhysicsTime = deltaTime;
    TimerSlowDownPercentage = 0;
    OverlapTime = 0;
    TimeStep = Real(0);
}

// set timers with no countdown
void InitCountDownNone()
{
    CountdownTime = 0;
    TimerCurrent = CurrentTimer();
    CountdownEndTime = TimerCurrent;
    TotalRaceStartTime = TimerCurrent;
    TotalRaceTime = TotalRacePhysicsTime = OverlapTime = 0;
    TimerSlowDownPercentage = 0;
    TimeStep = Real(0);
}

////////////////////////
// update race timers //
////////////////////////

void UpdateRaceTimers(void)
{
    unsigned long time, countdown, lastcountdowntime, timedelta;
    CAR *car;
    PLAYER *player;
    LEVELINFO *levinfo;
    long newlap, wasprelap;
    bool unlock;

// waiting for players?
#ifdef _PC
    if (!AllPlayersReady)
    {
        return;
    }
#endif
// get current timer

    time = TimerCurrent;

// counting down?

    if (CountdownTime)
    {
        if (GameSettings.Paws && !IsMultiPlayer())
        {
            CountdownEndTime += RealTimerDiff;
        }

        countdown = CountdownEndTime - time;
        lastcountdowntime = CountdownTime;
        CountdownTime = TIME2MS(countdown);
        if (!CountdownTime) CountdownTime = 1;

        if (IsMultiPlayer() || GameSettings.GameType == GAMETYPE_TRIAL)
        {
            TotalRacePhysicsTime = 0;
        }

#ifndef _PSX
        if (CountdownTime < 3500 && (lastcountdowntime / 1000) > (CountdownTime / 1000))
        {
 #ifdef OLD_AUDIO
            PlaySfx(SFX_COUNTDOWN, SFX_MAX_VOL, SFX_CENTRE_PAN, 22050, SFX_PRIORITY_COUNTDOWN);
 #else //OLD_AUDIO
            #pragma message( "JHarding needs to review this quick-and-dirty CPrince port." )
            g_SoundEngine.Play2DSound( EFFECT_Countdown, FALSE );
 #endif //OLD_AUDIO
        }
#endif

// countdown end?

        if (countdown & 0x80000000)
        {
            TotalRaceStartTime = time + countdown;
            if (IsMultiPlayer() || GameSettings.GameType == GAMETYPE_TRIAL)
            {
                TotalRacePhysicsTime = TIME2MS(0 - countdown);
            }
            CountdownTime = 0;

#ifndef _PSX
 #ifdef OLD_AUDIO
            PlaySfx(SFX_COUNTDOWN, SFX_MAX_VOL, SFX_CENTRE_PAN, 31000, SFX_PRIORITY_COUNTDOWN);
            PlaySfx(SFX_COUNTDOWN, SFX_MAX_VOL, SFX_CENTRE_PAN, 37000, SFX_PRIORITY_COUNTDOWN);
 #else //OLD_AUDIO
            #pragma message( "JHarding needs to review this quick-and-dirty CPrince port." )
            g_SoundEngine.Play2DSound( EFFECT_Countdown, FALSE );
 #endif //OLD_AUDIO
#endif

// start lap now if car started over finish line (bad mapping!)

#ifdef _PC
            if (!PLR_LocalPlayer->CarAI.PreLap)
            {
                if (Version == VERSION_DEV && GameSettings.GameType != GAMETYPE_REPLAY)
                    DumpMessage(NULL,"Car started over finish line!");

                for (player = PLR_PlayerHead ; player ; player = player->next)
                {
                    player->car.CurrentLapStartTime = TotalRaceStartTime;
                }
            }
#endif

// reset player handlers

            ResetAllPlayerHandlersToDefault();
        }

// update finish dists

        for (player = PLR_PlayerHead ; player ; player = player->next)
        {
            UpdateCarAiZone(player);
            UpdateCarFinishDist(player, NULL);
        }

        return;
    }

// update race time

    if (GameSettings.Paws && !IsMultiPlayer())
    {
        TotalRaceStartTime += RealTimerDiff;                // Modifier for game paused
    }
    else 
    {
        TotalRaceStartTime += RaceStartModifier;        // Modifier for time slow-down
    }


    TotalRaceTime = TIME2MS(time - TotalRaceStartTime);

// loop thru cars

    for (player = PLR_PlayerHead ; player ; player = player->next)
    {
        car = &player->car;

        switch (player->type)
        {

//////////////////////////////////////////////////////////////////////////////////////////////////
// LOCAL
//////////////////////////////////////////////////////////////////////////////////////////////////

        case PLAYER_LOCAL:

// modify lap start time?

            if (GameSettings.Paws && !IsMultiPlayer())
            {
                car->CurrentLapStartTime += RealTimerDiff;
            }
            else 
            {
                car->CurrentLapStartTime += RaceStartModifier;      // Modify start time if time is slowed down
            }

// zero lap + physics time if prelap

            wasprelap = FALSE;

            if (player->CarAI.PreLap)
            {
                wasprelap = TRUE;
                car->CurrentLapStartTime = time;

                if (GameSettings.GameType == GAMETYPE_TRIAL)
                {
                    TotalRacePhysicsTime = 0;
                }
            }

// update zone / lap pos

            UpdateCarAiZone(player);
            newlap = UpdateCarFinishDist(player, &timedelta);

// adjust lap + physics start time with 'crossed line' time delta if was prelap - this happens only once!

            if (wasprelap)
            {
                car->CurrentLapStartTime -= MS2TIME(timedelta);
                if (GameSettings.GameType == GAMETYPE_TRIAL)
                {
                    TotalRacePhysicsTime = timedelta;
                }
            }

// get current lap time

            car->CurrentLapTime = TIME2MS(time - car->CurrentLapStartTime);

// new lap?

            if (newlap)
            {

// yep, deduct 'crossed line' time delta from lap + physics time

                car->CurrentLapTime -= timedelta;
                if (GameSettings.GameType == GAMETYPE_TRIAL)
                {
                    TotalRacePhysicsTime -= timedelta;
                }

// trigger end split time

                TriggerSplit(player, 0, -1, NULL);//&LookVec);

// update misc lap info

                car->Laps++;
                car->NextSplit = 0;
                car->NextTrackDir = 0;

                car->LastLapTime = car->CurrentLapTime;
                car->CurrentLapTime = 0;
                car->CurrentLapStartTime = time - MS2TIME(timedelta);

// Store times in ghosts data info
            
#ifndef _PSX
                EndGhostData(&Players[0]);
#endif

// best lap?

                if (car->LastLapTime < car->BestLapTime)
                    car->BestLapTime = car->LastLapTime;

                if (car->AllowedBestTime && GameSettings.GameType == GAMETYPE_TRIAL) {
                    CheckForBestLap(car);
                }

// reinitalise the ghost car
#ifndef _PSX
                InitGhostData(&Players[0]);
                InitBestGhostData();
#endif

// reset physics start times using 'crossed line' time delta

                if (GameSettings.GameType == GAMETYPE_TRIAL)
                {
                    TotalRacePhysicsTime = timedelta;
                }

// end of race?

                if (GameSettings.GameType != GAMETYPE_TRIAL && GameSettings.GameType != GAMETYPE_PRACTICE && car->Laps == GameSettings.NumberOfLaps)
                {
                    SetPlayerFinished(player, TotalRaceTime);
#ifdef _PC
                    if (IsMultiPlayer())
                    {
                        SendRaceFinishTime();
                    }
#endif

                    car->LastRaceTime = TotalRaceTime;

                    if (car->LastRaceTime < car->BestRaceTime)
                        car->BestRaceTime = car->LastRaceTime;

                    if (car->AllowedBestTime && GameSettings.GameType == GAMETYPE_TRIAL) {
                        CheckForBestRace(car);
                    }

                    // Bring up end game menu
                    //ClearMenuHeader(&gMenuHeader);
                    //SetNextMenu(&gMenuHeader, &Menu_InGame);
                    //char buf1[32], buf2[32];
                    //wsprintf(buf1, "%d%s: %s: ", i + 1, i < 3 ? PosText[i] : "th", player->PlayerName);
                    //wsprintf(buf2, "%02d:%02d:%03d", MINUTES(time), SECONDS(time), THOUSANDTHS(time));
#ifdef _PC
                    SetConsoleMessage("Press Pause to continue", NULL, 0xff8811, 0xffffff, 10, CONSOLE_MESSAGE_FOREVER);
#endif
//#ifdef _N64
//                  SetConsoleMessage("Press Start BUTTON to continue", NULL, 0xff8811, 0xffffff, 10, CONSOLE_MESSAGE_FOREVER);
//#endif

// set secret flag?
                    if (player == &Players[0] && GameSettings.GameType == GAMETYPE_SINGLE && !player->RaceFinishPos && GameSettings.Level < LEVEL_NCUP_LEVELS)
                    {
                        levinfo = GetLevelInfo(GameSettings.Level);
                        unlock = IsCupWonSingleRaces(levinfo->LevelClass);

                        #ifdef _N64
                        if ( g_TitleScreenData.numberOfPlayers == 1 )
                        #endif
                        {
                        SetSecretWonSingleRace(GameSettings.Level);
                        }
                        if (!unlock && IsCupWonSingleRaces(levinfo->LevelClass))
                            InitialMenuMessage = MENU_MESSAGE_NEWCARS;
                    }
                }
            }

            break;

//////////////////////////////////////////////////////////////////////////////////////////////////
// CPU / REMOTE
//////////////////////////////////////////////////////////////////////////////////////////////////

        case PLAYER_REMOTE:
        case PLAYER_CPU:

// modify lap start time?

            if (GameSettings.Paws && !IsMultiPlayer())
            {
                car->CurrentLapStartTime += RealTimerDiff;
            }
            else 
            {
                car->CurrentLapStartTime += RaceStartModifier;      // Modify start time if time is slowed down
            }

// zero lap time if prelap

            wasprelap = FALSE;

            if (player->CarAI.PreLap)
            {
                wasprelap = TRUE;
                car->CurrentLapStartTime = time;
            }

// update zone / lap pos

            UpdateCarAiZone(player);
            newlap = UpdateCarFinishDist(player, &timedelta);

// adjust lap start time with 'crossed line' time delta if was prelap - this happens only once!

            if (wasprelap)
            {
                car->CurrentLapStartTime -= MS2TIME(timedelta);
            }

// get current lap time

            car->CurrentLapTime = TIME2MS(time - car->CurrentLapStartTime);

// new lap?

            if (newlap)
            {

// yep, deduct 'crossed line' time delta from lap time

                car->CurrentLapTime -= timedelta;

// update misc lap info

                car->Laps++;

                car->LastLapTime = car->CurrentLapTime;
                car->CurrentLapTime = 0;
                car->CurrentLapStartTime = time - MS2TIME(timedelta);

// best lap?

                if (car->LastLapTime < car->BestLapTime)
                    car->BestLapTime = car->LastLapTime;

// end of race?

                if (car->Laps == GameSettings.NumberOfLaps)
                {

// yep, set player finished

                    if (player->type != PLAYER_REMOTE)
                    {
                        SetPlayerFinished(player, TotalRaceTime);
                    }
                }
            }

            break;

//////////////////////////////////////////////////////////////////////////////////////////////////
// GHOST
//////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _PSX
        case PLAYER_GHOST:

// modify lap start time?

            if (GameSettings.Paws && !IsMultiPlayer())
            {
                car->CurrentLapStartTime += RealTimerDiff;
            }
            else 
            {
                car->CurrentLapStartTime += RaceStartModifier;      // Modify start time if time is slowed down
            }

// zero lap time if prelap

            wasprelap = FALSE;

            if (player->CarAI.PreLap)
            {
                wasprelap = TRUE;
                car->CurrentLapStartTime = time;
            }

// update zone / lap pos

            UpdateCarAiZone(player);
            newlap = UpdateCarFinishDist(player, &timedelta);


// adjust lap start time with 'crossed line' time delta if was prelap - this happens only once!

            if (wasprelap)
            {
                car->CurrentLapStartTime -= MS2TIME(timedelta);
            }

// get current lap time

            car->CurrentLapTime = TIME2MS(time - car->CurrentLapStartTime);

            break;
#endif

// REPLAY PLAYER

        case PLAYER_REPLAY:

// update race pos

            UpdateCarAiZone(player);
            UpdateCarFinishDist(player, NULL);

            break;

// DEFAULT PLAYER

        default:
            break;

        }
    }

}

/////////////////////////////
// check for best lap time //
/////////////////////////////
extern GHOST_INFO *GhostInfo;

void CheckForBestLap(CAR *car)
{
    bool newGhostCar, unlock;
    unsigned long i, j, start, end;
    LEVELINFO *levinfo;
    RECORD_ENTRY *trackRecords;

#ifdef _PC
    trackRecords = &TrackRecords;
    start = 0;
    end = MAX_RECORD_TIMES;
#else
    trackRecords = &TrackRecords[GameSettings.Level];
    if (GameSettings.Reversed) {
        start = MAX_RECORD_TIMES / 2;
        end = MAX_RECORD_TIMES;
    } else {
        start = 0;
        end = MAX_RECORD_TIMES / 2;
    }
#endif

// new ghost car?

    if (car->LastLapTime < (long)GHO_BestGhostInfo->Time[GHOST_LAP_TIME]) {
        newGhostCar = TRUE;
    } else {
        newGhostCar = FALSE;
    }




// loop thru best times

    //for (i = 0 ; i < MAX_RECORD_TIMES ; i++)
    for (i = start ; i < end; i++)
    {

// check against car last

        if (car->LastLapTime < trackRecords->RecordLap[i].Time)
        {

// beat it, copy lower ones down one

#ifndef _PSX
//          PlaySfx(SFX_RECORD, SFX_MAX_VOL, SFX_CENTRE_PAN, 22050, 2);
#endif
            //for (j = MAX_RECORD_TIMES - 1 ; j > i ; j--)
            for (j = end - 1 ; j > i ; j--)
            {
                trackRecords->RecordLap[j] = trackRecords->RecordLap[j - 1];
                RecordLapFlag[j] = RecordLapFlag[j - 1];
            }

// set new record

            trackRecords->RecordLap[i].Time = car->LastLapTime;
#ifdef _PC
            memcpy(trackRecords->RecordLap[i].Car, CarInfo[car->CarType].Name, CAR_NAMELEN);
            memcpy(trackRecords->RecordLap[i].Player, Players[0].PlayerName, MAX_PLAYER_NAME);
#else
            trackRecords->RecordLap[i].CarType = car->CarType;
            memcpy(trackRecords->RecordLap[i].Player, Players[0].PlayerName, MAX_PLAYER_NAME);
#endif
            RecordLapFlag[i] = TRUE;

// best time?

            if (i == start)
            {
                for (j = 0 ; j < MAX_SPLIT_TIMES ; j++)
                    trackRecords->SplitTime[j] = car->SplitTime[j];

                newGhostCar = TRUE;
            }

            break;
        }
    }


// store ghost car as best ghost car

    if (newGhostCar || (GHO_GhostAllowed && !GHO_GhostDataExists)) {
        SwitchGhostDataStores();

        for (j = 0 ; j < MAX_SPLIT_TIMES ; j++)
            GHO_BestGhostInfo->Time[j] = /*(unsigned short)*/trackRecords->SplitTime[j];
    }

// beaten challenge time?

#ifndef _PSX
    if (GameSettings.GameType == GAMETYPE_TRIAL && GameSettings.Level < LEVEL_NCUP_LEVELS)
    {
        levinfo = GetLevelInfo(GameSettings.Level);
        if (levinfo)
        {
            // set cup class
            CupTable.CupType = levinfo->LevelClass;

            // normal
            if (!GameSettings.Reversed && !GameSettings.Mirrored)
            {
#if defined(CHEATS)         
                if (1)
#else
                if (car->LastLapTime < levinfo->ChallengeTimeNormal)
#endif
                {
                    unlock = IsCupBeatTimeTrials(levinfo->LevelClass);
                    SetSecretBeatTimeTrial(GameSettings.Level);
                    if (!unlock && IsCupBeatTimeTrials(levinfo->LevelClass))
                    {
                        InitialMenuMessage = MENU_MESSAGE_REVERSE;
                    }
                }
            }
            // reversed
            if (GameSettings.Reversed && !GameSettings.Mirrored)
            {
#if defined(CHEATS)         
                if (1)
#else
                if (car->LastLapTime < levinfo->ChallengeTimeReversed)
#endif
                {
                    unlock = IsCupBeatTimeTrialsReversed(levinfo->LevelClass);
                    SetSecretBeatTimeTrialReverse(GameSettings.Level);
                    if (!unlock && IsCupBeatTimeTrialsReversed(levinfo->LevelClass))
                    {
                        InitialMenuMessage = MENU_MESSAGE_MIRROR;
                    }
                }
            }
            // mirrored
            if (!GameSettings.Reversed && GameSettings.Mirrored)
            {
#if defined(CHEATS)         
                if (1)
#else
                if (car->LastLapTime < levinfo->ChallengeTimeNormal)
#endif  
                {
                    unlock = IsCupBeatTimeTrialsMirrored(levinfo->LevelClass);
                    SetSecretBeatTimeTrialMirror(GameSettings.Level);
                    if (!unlock && IsCupBeatTimeTrialsMirrored(levinfo->LevelClass))
                    {
                        InitialMenuMessage = MENU_MESSAGE_REVMIR;
                    }
                }
            }
        }
    }
#endif
}

//////////////////////////////
// check for best race time //
//////////////////////////////

void CheckForBestRace(CAR *car)
{
#if 0
    unsigned long i, j;

// loop thru best times

    for (i = 0 ; i < MAX_RECORD_TIMES ; i++)
    {

// check against car last

        if (car->LastRaceTime < TrackRecords.RecordRace[i].Time)
        {

// beat it, copy lower ones down one

            for (j = MAX_RECORD_TIMES - 1 ; j > i ; j--)
            {
                TrackRecords.RecordRace[j] = TrackRecords.RecordRace[j - 1];
                RecordRaceFlag[j] = RecordRaceFlag[j - 1];
            }

// set new record

            TrackRecords.RecordRace[i].Time = car->LastRaceTime;
            memcpy(TrackRecords.RecordRace[i].Car, CarInfo[car->CarType].Name, CAR_NAMELEN);
#ifdef _PC
            memcpy(TrackRecords.RecordRace[i].Player, RegistrySettings.PlayerName, MAX_PLAYER_NAME);
#endif
            RecordRaceFlag[i] = TRUE;

            break;
        }
    }
#endif
}


//////////////////////
// load track times //
//////////////////////
void LoadTrackTimes(long level, long mirrored, long reversed)
{
    long i;
#ifdef _PC
    FILE *fp;
#endif
    RECORD_ENTRY *trackRecords;

#ifdef _PC
    trackRecords = &TrackRecords;
#else
    trackRecords = &TrackRecords[GameSettings.Level];
#endif
    

// set time defaults

    for (i = 0 ; i < MAX_SPLIT_TIMES ; i++)
    {
        trackRecords->SplitTime[i] = MAX_LAP_TIME;
    }

    for (i = 0 ; i < MAX_RECORD_TIMES ; i++)
    {
        trackRecords->RecordLap[i].Time = MAX_LAP_TIME;
        sprintf(trackRecords->RecordLap[i].Player, "----------------");
#ifdef _PC
        sprintf(trackRecords->RecordLap[i].Car, "----------------");
#else
        trackRecords->RecordLap[i].CarType = CARID_RC;
#endif

#ifdef _PC
//      trackRecords->RecordRace[i].Time = MAKE_TIME(60, 0, 0);
//      sprintf(trackRecords->RecordRace[i].Player, "----------------");
//      sprintf(trackRecords->RecordRace[i].Car, "----------------");
#endif

        RecordLapFlag[i] = FALSE;
        RecordRaceFlag[i] = FALSE;
    }

// read in record file
#ifdef _PC

//  if (mirrored)
//      fp = fopen(GetAnyLevelFilename(g_TitleScreenData.iLevelNum, reversed, RECORDS_FILENAME_MIRRORED, FILENAME_GAME_SETTINGS), "rb");
//  else
//      fp = fopen(GetAnyLevelFilename(g_TitleScreenData.iLevelNum, reversed, RECORDS_FILENAME, FILENAME_GAME_SETTINGS), "rb");
    fp = fopen(GetGhostFilename(level, reversed, mirrored, "times"), "rb");
    if (!fp) return;

    fread(&TrackRecords, sizeof(TrackRecords), 1, fp);
    fclose(fp);
#endif
}

//////////////////////
// save track times //
//////////////////////
#ifdef _PC
void SaveTrackTimes()
{
    long j, k, l, update, updateGhost;
    FILE *fp;
    RECORD_ENTRY record;

// open records file

//  if (GameSettings.Mirrored)
//      fp = fopen(GetLevelFilename(RECORDS_FILENAME_MIRRORED, FILENAME_GAME_SETTINGS), "rb+");
//  else
//      fp = fopen(GetLevelFilename(RECORDS_FILENAME, FILENAME_GAME_SETTINGS), "rb+");
    fp = fopen(GetGhostFilename(GameSettings.Level, GameSettings.Reversed, GameSettings.Mirrored, BEST_TIMES_EXT), "rb+");

// if none, create new

    if (!fp)
    {
//      if (GameSettings.Mirrored)
//          fp = fopen(GetLevelFilename(RECORDS_FILENAME_MIRRORED, FILENAME_GAME_SETTINGS), "wb");
//      else
//          fp = fopen(GetLevelFilename(RECORDS_FILENAME, FILENAME_GAME_SETTINGS), "wb");
        fp = fopen(GetGhostFilename(GameSettings.Level, GameSettings.Reversed, GameSettings.Mirrored, BEST_TIMES_EXT), "wb");

        if (!fp)
        {
            DumpMessage(NULL, "Failed to create record file");
            return;
        }

        fwrite(&TrackRecords, sizeof(TrackRecords), 1, fp);
        fclose(fp);
        SaveGhostData();
        return;
    }

// else munge times with file

    updateGhost = FALSE;
    update = FALSE;
    fread(&record, sizeof(record), 1, fp);

// check lap times

    for (j = 0 ; j < MAX_RECORD_TIMES ; j++)
    {
        if (RecordLapFlag[j])
        {
            RecordLapFlag[j] = FALSE;

            for (k = 0 ; k < MAX_RECORD_TIMES ; k++)
            {
                if (TrackRecords.RecordLap[j].Time < record.RecordLap[k].Time)
                {
                    for (l = MAX_RECORD_TIMES - 1 ; l > k ; l--) record.RecordLap[l] = record.RecordLap[l - 1];
                    record.RecordLap[k] = TrackRecords.RecordLap[j];
                    update = TRUE;

// best time?

                    if (k == 0)
                    {
                        for (l = 0 ; l < MAX_SPLIT_TIMES ; l++)
                            record.SplitTime[l] = TrackRecords.SplitTime[l];

                        updateGhost = TRUE;
                    }
                    break;
                }
            }
        }
    }

// check race times

/*  for (j = 0 ; j < MAX_RECORD_TIMES ; j++)
    {
        if (RecordRaceFlag[j])
        {
            RecordRaceFlag[j] = FALSE;

            for (k = 0 ; k < MAX_RECORD_TIMES ; k++)
            {
                if (TrackRecords.RecordRace[j].Time < record.RecordRace[k].Time)
                {
                    for (l = MAX_RECORD_TIMES - 1 ; l > k ; l--) record.RecordRace[l] = record.RecordRace[l - 1];
                    record.RecordRace[k] = TrackRecords.RecordRace[j];
                    update = TRUE;
                    break;
                }
            }
        }
    }
*/

// update record?

    if (update)
    {
        fseek(fp, 0, SEEK_SET);
        fwrite(&record, sizeof(record), 1, fp);
    }

// close file

    fclose(fp);

// update the ghost if required

    if (updateGhost) 
    {
        SaveGhostData();
    } else {
        LoadGhostData();
    }
}

#endif

