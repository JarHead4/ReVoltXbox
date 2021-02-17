//-----------------------------------------------------------------------------
// File: Credits.h
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef CREDITS_H
#define CREDITS_H




//-----------------------------------------------------------------------------
// Credit Entry structure
//-----------------------------------------------------------------------------
struct NEW_CREDIT_PAGE
{
    WCHAR*  strTitle;
    DWORD   dwFlags;

    FLOAT   fXPos;
    FLOAT   fYPos;
    FLOAT   fWidth;
    FLOAT   fHeight;

    DWORD   dwNumStrings;
    WCHAR** pStrings;
    DWORD*  pStringTypes;

    DWORD   dwNumTextLines;
    FLOAT   fScrollSpeed;
};




//-----------------------------------------------------------------------------
// Credit control structure
//-----------------------------------------------------------------------------
enum CREDIT_STATE
{
    CREDIT_STATE_INACTIVE,
    CREDIT_STATE_INIT,
    CREDIT_STATE_MOVING,
    CREDIT_STATE_FADE_IN,
    CREDIT_STATE_SHOWING,
    CREDIT_STATE_FADE_OUT,
    CREDIT_STATE_DONE,

    CREDIT_NSTATES
};


struct CREDIT_VARS
{
    CREDIT_STATE    State;

    FLOAT           Timer;                      // State timer
    FLOAT           MaxTime;                    // Time allowed in current state

    FLOAT           ScrollTimer;                // Scroller timer
    FLOAT           ScrollMaxTime;

    FLOAT           XPos;
    FLOAT           YPos;
    FLOAT           XSize;
    FLOAT           YSize;

    FLOAT           DestXPos;
    FLOAT           DestYPos;
    FLOAT           DestXSize;
    FLOAT           DestYSize;

    FLOAT           XVel;
    FLOAT           YVel;
    FLOAT           XGrow;
    FLOAT           YGrow;


    FLOAT           Alpha;                      // Transparency value
    int             ColIndex;

    long            CurrentPage;

    unsigned long   TimerCurrent;
    unsigned long   TimerLast;
};




//-----------------------------------------------------------------------------
// Externs
//-----------------------------------------------------------------------------
extern void InitCreditEntries();
extern void InitCreditStateInactive();
extern void InitCreditStateActive();
extern void ProcessCredits();
extern void DrawCredits();

extern CREDIT_VARS  g_CreditVars;


#endif // CREDITS_H

