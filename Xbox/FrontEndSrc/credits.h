//-----------------------------------------------------------------------------
// File: credits.h
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef CREDITS_H
#define CREDITS_H


////////////////////////////////////////////////////////////////
//
// Text size defines
//
////////////////////////////////////////////////////////////////

#if defined(_PC)

#define CREDIT_TEXT_WIDTH   8
#define CREDIT_TEXT_HEIGHT  16

#define CREDIT_TEXT_INDENT  16

#elif defined(_N64)

#define CREDIT_TEXT_WIDTH   16
#define CREDIT_TEXT_HEIGHT  16

#define CREDIT_TEXT_INDENT  32

#elif defined(_PSX)

#define CREDIT_TEXT_WIDTH   13
#define CREDIT_TEXT_HEIGHT  10

#define CREDIT_TEXT_INDENT  26

#endif // MACHINE TYPE


////////////////////////////////////////////////////////////////
//
// Credit type defines
//
////////////////////////////////////////////////////////////////

#define CREDIT_CENTRE_X     0x1
#define CREDIT_CENTRE_Y     0x2
#define CREDIT_SCROLL       0x4
#define CREDIT_CENTRE_TEXT  0x8
#define CREDIT_RANDOM_POS   0x10
 

////////////////////////////////////////////////////////////////
//
// Credit Entry structure
//
////////////////////////////////////////////////////////////////

typedef char* CREDIT_TEXT;

typedef struct OneCreditPageStruct {

    char        *SubTitle;
    char        **NameList;

    // Stuff below here is automatically generated

    int         NNames;                     // Number of names in this sub-heading

} CREDIT_PAGE_DATA;


typedef struct CreditPageStruct {

    char        *Title;
    CREDIT_PAGE_DATA    *PageData;
    long        PageType;

    REAL        XPos;                       // Pos of  top left os text box
    REAL        YPos;

    REAL        ScrollSpeed;                // Speed (pixels/sec) that text scrolls


    // Stuff below here is automatically generated

    int         NTextLines;                 // Number of lines in the Text array
    REAL        XSize;                      // Min box width required to fit the entry in
    REAL        YSize;                      // Min box height required to fit the entry in

    int         NSubSections;               // Number of sub headings in page

} CREDIT_PAGE;


////////////////////////////////////////////////////////////////
//
// Credit control structure
//
////////////////////////////////////////////////////////////////

typedef enum CreditStateEnum {
    CREDIT_STATE_INACTIVE,
    CREDIT_STATE_INIT,
    CREDIT_STATE_MOVING,
    CREDIT_STATE_FADE_IN,
    CREDIT_STATE_SHOWING,
    CREDIT_STATE_FADE_OUT,
    CREDIT_STATE_DONE,

    CREDIT_NSTATES
} CREDIT_STATE;

typedef struct CreditVarsStruct {
    CREDIT_STATE    State;

    REAL            Timer;                      // State timer
    REAL            MaxTime;                    // Time allowed in current state

    REAL            ScrollTimer;                // Scroller timer
    REAL            ScrollMaxTime;

    REAL            XPos;
    REAL            YPos;
    REAL            XSize;
    REAL            YSize;

    REAL            DestXPos;
    REAL            DestYPos;
    REAL            DestXSize;
    REAL            DestYSize;

    REAL            XVel;
    REAL            YVel;
    REAL            XGrow;
    REAL            YGrow;


    REAL            Alpha;                      // Transparency value
    int             ColIndex;

    long            CurrentPage;

    unsigned long   TimerCurrent;
    unsigned long   TimerLast;
} CREDIT_VARS;



////////////////////////////////////////////////////////////////
//
// Externs
//
////////////////////////////////////////////////////////////////

extern void InitCreditEntries();
extern void InitCreditStateInactive();
extern void InitCreditStateActive();
extern void ProcessCredits();
extern void DrawCredits();







extern CREDIT_VARS  CreditVars;
extern long         NCreditPages;


#endif // CREDITS_H

