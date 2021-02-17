//-----------------------------------------------------------------------------
// File: gamegauge.h
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef GAMEGAUGE_H
#define GAMEGAUGE_H

// globals

extern unsigned long START_TIME;
extern unsigned long FRAMES_ONE_SECOND;
extern unsigned long FRAMES_TOTAL;
extern unsigned long MIN_ONE_SECOND;
extern unsigned long MAX_ONE_SECOND;
extern unsigned long CUR_ONE_SECOND;

// prototypes

extern void SetupGameGaugeDemo(void);
extern void SetupGameGaugeVars(void);
extern void UpdateGameGaugeVars(void);
extern void DrawGameGaugeInfo(void);
extern void OutputGameGaugeData(void);

#endif // GAMEGAUGE_H

