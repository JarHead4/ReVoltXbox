//-----------------------------------------------------------------------------
// File: timing.h
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef TIMING_H
#define TIMING_H

#include "revolt.h"
#ifndef _PSX
#include "main.h"
#endif
#include "LevelLoad.h"

// timer shift

#define TIMER_SHIFT 6

// macros

#define FIXED_TIME_STEP FALSE

#ifndef _N64
#define PHYSICSTIMESTEP     (8)         // milliseconds
#else
#define PHYSICSTIMESTEP     (10)            // milliseconds
#endif

#define MAX_LAP_TIME        MAKE_TIME(30,0,0)

#ifndef _PSX
#define MAKE_TIME(_m, _s, _t) \
    ((_m) * 60000 + (_s) * 1000 + (_t))
#else
#if NTSC
#define MAKE_TIME(_m, _s, _t) \
    ((_m) * 60 * 60 + (_s) * 60 + ((_t) * 60) / 1000)
#else
#define MAKE_TIME(_m, _s, _t) \
    ((_m) * 60 * 50 + (_s) * 50 + ((_t) * 50) / 1000)
#endif
#endif


#ifdef _PC
 #define TIME2MS(_t) \
    ((unsigned long)((__int64)(_t) * 1000 / TimerFreq))

 #define MS2TIME(_t) \
    ((unsigned long)((__int64)(_t) * TimerFreq / 1000))

 #define BEST_TIMES_EXT     "Times"
#endif

#define COUNTDOWN_START (1000 * 5)

#define BOMBTAG_MAX_TIME (1000 * 60 * 2)

// prototypes

extern unsigned long CurrentTimer(void);
extern void InitCountDown(void);
extern void InitCountDownNone(void);
extern void InitCountDownDelta(unsigned long deltaTime);
extern void UpdateTimeStep(void);
extern void UpdateRaceTimers(void);
extern void LoadTrackTimes(long level, long mirrored, long reversed);
extern void SaveTrackTimes();
extern void CheckForBestLap(CAR *car);
extern void CheckForBestRace(CAR *car);
extern void InitTrackRecords();


// globals

extern unsigned long TimerLast, TimerCurrent, RealTimerDiff, TimerDiff, TimerFreq, TotalRaceTime, TotalRacePhysicsTime, TotalRaceStartTime, CountdownTime, CountdownEndTime, LastCountdownTime;
extern REAL TimerSlowDownPercentage;
extern unsigned long OverlapTime;

#ifndef _PC
extern RECORD_ENTRY TrackRecords[];
#else
extern RECORD_ENTRY TrackRecords;
#endif

#define GetTimeStep() TimeStep



#endif // TIMING_H

