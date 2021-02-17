//$CMP_NOTE: not yet clear whether we need some of this in Xbox version...

//-----------------------------------------------------------------------------
// File: shareware.cpp
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"
#include "shareware.h"
#include "main.h"
#include "draw.h"
#include "settings.h"
#include "timing.h"
#include "input.h"
#include "geom.h"
#include "camera.h"
#include "player.h"
#include "dx.h"
#include "intro.h"
#include "ui_TitleScreen.h"

void DrawSharewarePage();
void ReleaseSharewareIntro();


REAL gSharewareTimer = ZERO;
long gSharewareImage = 0;
long gSharewareCount = 0;
bool gQuitShareware = FALSE;
bool gLastImage = FALSE;

HBITMAP gSharewareHbm[SHAREWARE_MAX_PAGES];

SLIDE IntroSlides[] = {
//$MODIFIED
//    {"gfx\\intro1.bmp", 3.0f, SLIDE_WAIT},
//    {"gfx\\intro2.bmp", 3.0f, SLIDE_WAIT | SLIDE_SKIP},
//    {"gfx\\intro3.bmp", 3.0f, SLIDE_WAIT | SLIDE_SKIP},
    {"D:\\gfx\\intro1.bmp", 3.0f, SLIDE_WAIT},
    {"D:\\gfx\\intro2.bmp", 3.0f, SLIDE_WAIT | SLIDE_SKIP},
    {"D:\\gfx\\intro3.bmp", 3.0f, SLIDE_WAIT | SLIDE_SKIP},
//$END_MODIFICATIONS
    {NULL, 0.0f, 0},
};

SLIDE OutroSlides[] = {
//$MODIFIED
//    {"gfx\\outro1.bmp", 30.0f, SLIDE_WAIT | SLIDE_SKIP},
//    {"gfx\\outro2.bmp", 30.0f, SLIDE_WAIT | SLIDE_SKIP},
//    {"gfx\\outro3.bmp", 30.0f, SLIDE_WAIT | SLIDE_SKIP},
    {"D:\\gfx\\outro1.bmp", 30.0f, SLIDE_WAIT | SLIDE_SKIP},
    {"D:\\gfx\\outro2.bmp", 30.0f, SLIDE_WAIT | SLIDE_SKIP},
    {"D:\\gfx\\outro3.bmp", 30.0f, SLIDE_WAIT | SLIDE_SKIP},
//$END_MODIFICATIONS
    {NULL, 0.0f, 0},
};

SLIDE GermanSponsorSlide[] = {
//$MODIFIED
//    {"gfx\\SponsorsGermany.bmp", 10.0f, SLIDE_WAIT | SLIDE_SKIP},
    {"D:\\gfx\\SponsorsGermany.bmp", 10.0f, SLIDE_WAIT | SLIDE_SKIP},
//$END_MODIFICATIONS
    {NULL, 0.0f, 0},
};

SLIDE *gSharewareSlides = IntroSlides;


////////////////////////////////////////////////////////////////
//
// Setup Shareware Intro
//
////////////////////////////////////////////////////////////////

void SetupSharewareIntro()
{
//$REMOVED(jedl) - this should done just once at the beginning
//    // init D3D
//    if (!InitD3D(640, 480, 16, 0))
//    {
//        QuitGame();
//        return;
//    }
//$END_REMOVAL

//$MODIFIED(tentative!!) - bypass intro/transition logo screens
//    PickTextureFormat();
//    InitTextures();
//    InitFadeShit();
//    SetupDxState();
//
//    // Scale factors dependent on screen mode
//    gMenuWidthScale = ScreenXsize / 640.0f;
//    gMenuHeightScale = ScreenYsize / 480.0f;
//
//
//    // pick texture sets
//    PickTextureSets(0, 3, 0, 0);
//
//    // load intro tpages
//    char *filename;
//    gSharewareCount = 0;
//    while ((filename = gSharewareSlides[gSharewareCount].BitmapFilename) != NULL) {
//#ifndef XBOX_NOT_YET_IMPLEMENTED
//        LoadBitmap(filename, &gSharewareHbm[gSharewareCount]);
//#endif
//        gSharewareCount++;
//    }
//
//    // Go
//    gSharewareTimer = -ONE;
//    gQuitShareware = FALSE;
//    gSharewareImage = 0;
//    SetFadeEffect(FADE_UP);
//
//    SET_EVENT(ProcessSharewareIntro);

    SET_EVENT(GoTitleScreen);
//$END_MODIFICATIONS
}

////////////////////////////////////////////////////////////////
//
// Process Shareware Intro
//
////////////////////////////////////////////////////////////////

void ProcessSharewareIntro()
{
    bool nextImage = FALSE;

    // Time to quit?
    if (gQuitShareware && (GetFadeEffect() == FADE_DOWN_DONE)) {
        ReleaseSharewareIntro();
        if (gSharewareSlides == OutroSlides || gSharewareSlides == GermanSponsorSlide) {
            QuitGame();
        } else if (GoStraightToDemo) {
            SetDemoData();
            SET_EVENT(SetupGame);
            gSharewareSlides = OutroSlides;
        } else {
            SET_EVENT(GoTitleScreen);
            gSharewareSlides = OutroSlides;
        }
        return;
    }


    // update keyboard / mouse
//$REMOVED    ReadMouse();
//$REMOVED    ReadKeyboard();
    ReadJoystick();


    // Key press to skip?
    if ((Keys[DIK_SPACE] && !LastKeys[DIK_SPACE]) ||
        (Keys[DIK_RETURN] && !LastKeys[DIK_RETURN]) ||
        (Keys[DIK_ESCAPE] && !LastKeys[DIK_ESCAPE])) {
        if (gSharewareSlides[gSharewareImage].Type & SLIDE_SKIP) {
            nextImage = TRUE;
        }
    }


    // Increment timer
    UpdateTimeStep();
    if (gSharewareTimer < ZERO) gSharewareTimer = ZERO;
    gSharewareTimer += TimeStep;


    // Time to change?
    if (gSharewareTimer > gSharewareSlides[gSharewareImage].DisplayTime) {
        if (gSharewareSlides[gSharewareImage].Type & SLIDE_WAIT) {
            nextImage = TRUE;
        }
    }


    // Change Image?
    if (nextImage && (GetFadeEffect() == FADE_NONE)) {
        SetFadeEffect(FADE_DOWN);
    }

    if (GetFadeEffect() == FADE_DOWN_DONE) {
        if (gSharewareImage < gSharewareCount - 1) {
            gSharewareImage++;
            gSharewareTimer = ZERO;
            SetFadeEffect(FADE_UP);
        } else {
            gQuitShareware = TRUE;
        }
    }

    if (GetFadeEffect() == FADE_UP_DONE) {
        SetFadeEffect(FADE_NONE);
    }

    // surface check + flip
//$REMOVED    CheckSurfaces();
    FlipBuffers();
    ClearBuffers();


    ////////////////////////////////////////////////////////////////
    // Begin render
    D3Ddevice->BeginScene();

    // set and clear viewport
    SetViewport(0, 0, (float)ScreenXsize, (float)ScreenYsize, RenderSettings.GeomPers);

    // Draw image
    //DrawSharewarePage();
    if ((GetFadeEffect() != FADE_UP_DONE) && (GetFadeEffect() != FADE_DOWN_DONE)) {
#ifndef XBOX_NOT_YET_IMPLEMENTED
        BlitBitmap(gSharewareHbm[gSharewareImage], &BackBuffer);
#endif
    }

    InitRenderStates();
    DrawFadeShit();


    // Done 
    D3Ddevice->EndScene();
    ////////////////////////////////////////////////////////////////
}


////////////////////////////////////////////////////////////////
//
// Release Shareware Intro
//
////////////////////////////////////////////////////////////////

void ReleaseSharewareIntro()
{
    long ii;

    for (ii = 0; ii < gSharewareCount; ii++) {
#ifndef XBOX_NOT_YET_IMPLEMENTED
        FreeBitmap(gSharewareHbm[ii]);
#endif
    }
}

