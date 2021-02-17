//-----------------------------------------------------------------------------
// File: podium.cpp
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"
#include "podium.h"
#include "competition.h"
#include "player.h"
#include "TitleScreen.h"
#include "Menu2.h"
#include "MenuDraw.h"
#include "MenuText.h"
#include "main.h"
#include "InitPlay.h"
#include "move.h"
#include "weapon.h"
#include "Obj_Init.h"
#include "text.h"
#ifndef _PSX
#include "spark.h"
#endif

#ifdef _PC
#include "panel.h"
#endif

#ifdef _N64
#include "gfx.h"
#endif

// menus
#include "Options.h"
#include "MainMenu.h"

void DropSparkles();
void DropNextCarOnPodium();
void DisplayWinLoseMessage();
void DisplayWinPosition();
void MakeTheCarsDance();
void UpdateFireworkDisplay();


// Podium Positions
VEC PodiumCarDropPos[PODIUM_NPOS] = {
#ifndef _PSX
    TO_LENGTH(Real(-1628)), TO_LENGTH(Real(-1000)), TO_LENGTH(Real(1763)),
    TO_LENGTH(Real(-1801)), TO_LENGTH(Real(-1000)), TO_LENGTH(Real(1588)),
    TO_LENGTH(Real(-1448)), TO_LENGTH(Real(-1000)), TO_LENGTH(Real(1950)),
    TO_LENGTH(Real(-1515)), TO_LENGTH(Real(-1000)), TO_LENGTH(Real(1665)),
#else
    TO_LENGTH(Real(-1628)), TO_LENGTH(Real(-700)), TO_LENGTH(Real(1763)),
    TO_LENGTH(Real(-1801)), TO_LENGTH(Real(-700)), TO_LENGTH(Real(1588)),
    TO_LENGTH(Real(-1448)), TO_LENGTH(Real(-700)), TO_LENGTH(Real(1950)),
    TO_LENGTH(Real(-1515)), TO_LENGTH(Real(-700)), TO_LENGTH(Real(1665)),
#endif
};

// Car drop matrix
MAT PodiumCarDropMat;

PODIUM_VARS PodiumVars;


////////////////////////////////////////////////////////////////
//
// initialise win/lose sequence variables
//
////////////////////////////////////////////////////////////////

void InitPodiumVars()
{
    if (CupTable.LocalPlayerPos <= 3) {
        // Player in top three
        PodiumVars.NCarsToDrop = 3;
    } else {
        // Player not in top three
        PodiumVars.NCarsToDrop = 4;
    }
    PodiumVars.State = PODIUM_STATE_INIT;
    PodiumVars.StateTimer = ZERO;
    PodiumVars.CarDropCount = 0;

#ifndef _PSX
    RotationY(&PodiumCarDropMat, PI);
#else
    RotationY(&PodiumCarDropMat, HALF);
#endif
}


////////////////////////////////////////////////////////////////
//
// Process Win/lose sequence
//
////////////////////////////////////////////////////////////////

#define WINLOSE_MESSAGE_TIME    TO_TIME(Real(3))
#define WINLOSE_CARDROP_TIME    TO_TIME(Real(2.5))
#define WINLOSE_CARDROP_LOSE_TIME TO_TIME(Real(7))
#define WINLOSE_BOMB_TIME       TO_TIME(Real(8))

#ifdef _PC
#define WINLOSE_EXIT_TIME       TO_TIME(Real(3))
#define WINLOSE_EXIT_LOSE_TIME  TO_TIME(Real(5.5))
#else
#define WINLOSE_EXIT_TIME       TO_TIME(Real(5))
#define WINLOSE_EXIT_LOSE_TIME  TO_TIME(Real(7.5))
#endif
#define WINLOSE_POS_STARTSIZE   Real(300)
#define WINLOSE_POS_SHRINKSPEED Real(500)
#define WINLOSE_POS_ENDSIZE     Real(32)


void ProcessWinLoseSequence()
{
    // Keep track of amount of time in current state
    PodiumVars.StateTimer += TimeStep;

    ////////////////////////////////////////////////////////////////
    // Process sequence state
    switch (PodiumVars.State) {

    case PODIUM_STATE_INIT:
        ////////////////////////////////////////////////////////////////
        // Showing initial win/lose message

        DisplayWinLoseMessage();

        if (PodiumVars.StateTimer > WINLOSE_MESSAGE_TIME) {
            gMenuHeader.NavFlags = 0;
            PodiumVars.StateTimer = ZERO;
            PodiumVars.State = PODIUM_STATE_DROP;
            PodiumVars.CarDropCount = 0;
            PodiumVars.FireworkMaxTime = ZERO;
            PodiumVars.FadeStarted = FALSE;
            g_pTitleScreenCamera->SetMoveTime( TS_CAMERA_MOVE_TIME * 2 );
        }
        break;


    case PODIUM_STATE_DROP:
        ////////////////////////////////////////////////////////////////
        // Drop the next car
        DropNextCarOnPodium();

        PodiumVars.State = PODIUM_STATE_DROPWAIT;
        break;


    case PODIUM_STATE_DROPWAIT:
        ////////////////////////////////////////////////////////////////
        // Waiting for next car to drop

        // Draw current car position on screen
        if (PodiumVars.DisplayWinPos) {
            DisplayWinPosition();
        }

        // Drop sparkly things on the car
        if (PodiumVars.DropSparkles) {
            DropSparkles();
        }

        // "Make the cars dance"   G. Biasillo 4/5/99
        MakeTheCarsDance();

        // Fire the fireworks
        UpdateFireworkDisplay();

        // Change camera viewpoint
        if (CupTable.LocalPlayerPos <= 3) {
            if (PodiumVars.StateTimer > (PodiumVars.CarDropTimer * 3) / 4) 
            {
                g_pTitleScreenCamera->SetNewCameraPos( TITLESCREEN_CAMPOS_PODIUM );
            }
            else if (PodiumVars.StateTimer > PodiumVars.CarDropTimer / 4) 
            {
                int playerNum;
                if (PodiumVars.CarDropCount < 4) {
                    playerNum = 3 - PodiumVars.CarDropCount;
                } else {
                    playerNum = 3;
                }
                g_pTitleScreenCamera->SetNewCameraPos( TITLESCREEN_CAMPOS_PODIUMVIEW1 + playerNum );
            }
        }

        // Change State?
        if (PodiumVars.StateTimer > PodiumVars.CarDropTimer) {
            PodiumVars.StateTimer = ZERO;

            if (PodiumVars.CarDropCount < PodiumVars.NCarsToDrop) {
                // Drop the rest of the cars
                PodiumVars.State = PODIUM_STATE_DROP;
            } else {
                // All cars dropped
                PodiumVars.State = PODIUM_STATE_EXIT;
            }
            PodiumVars.DisplayWinPos = FALSE;
            PodiumVars.DropSparkles = FALSE;

        }
        break;


    case PODIUM_STATE_EXIT:
        ////////////////////////////////////////////////////////////////
        // Done with the win/lose sequence


        if (!PodiumVars.FadeStarted && 
            (PodiumVars.StateTimer > ((CupTable.LocalPlayerPos > 3)? WINLOSE_EXIT_LOSE_TIME: WINLOSE_EXIT_TIME))
            ) {
            SetFadeEffect(FADE_DOWN);
            PodiumVars.FadeStarted = TRUE;
        }

        if (PodiumVars.FadeStarted && (GetFadeEffect() == FADE_DOWN_DONE))
        {
            PodiumVars.StateTimer = ZERO;

            // Reset titlescreen
            g_fTitleScreenTimer = -ONE;
            g_bShowWinLoseSequence = FALSE;

            if ((CupTable.LocalPlayerPos == 1) && (CupTable.CupType == RACE_CLASS_SPECIAL)) {
                // Show credits at end of Platinum cup
                SetRaceCredits(&gMenuHeader, &Menu_TopLevel, &MenuItem_ShowCredits);
                g_bTitleScreenFadeStarted = TRUE;

            } else {

                // init Camera for menu system
                InitFrontendMenuCamera();
                g_pTitleScreenCamera->SetMoveTime( TS_CAMERA_MOVE_TIME );
        
                // Setup initial menu
                SetTitleScreenInitialMenu();

                // Setup the dinky cars
                PLR_KillAllPlayers();

                SetupTitleScreenCar();
                InitStartingPlayers();

                // Load carbox textures etc...
                LoadFrontEndTextures();

                // Start fade-in
                SetFadeEffect(FADE_UP);

                SetRedbookFade(REDBOOK_FADE_UP);
                PlayRedbookTrackRandom(CurrentLevelInfo.RedbookStartTrack, CurrentLevelInfo.RedbookEndTrack, TRUE);
            }
        }
        break;

    }


}


////////////////////////////////////////////////////////////////
//
// UpdateFireworkDisplay:
//
////////////////////////////////////////////////////////////////

void UpdateFireworkDisplay()
{
#ifndef _PSX
    // Is it time to generate a new firework
    PodiumVars.FireworkTimer += TimeStep;
    if (PodiumVars.FireworkTimer > PodiumVars.FireworkMaxTime) {
        // Yep, generate a new firework
        VEC pos;
        long flags[4] = {0,0,0,0};

        SetVec(&pos,
            TO_LENGTH(Real(-1628)) + frand(TO_LENGTH(Real(512))) - TO_LENGTH(Real(256)), 
            TO_LENGTH(Real(-600)) - frand(TO_LENGTH(Real(100))), 
            TO_LENGTH(Real(1763)) + frand(TO_LENGTH(Real(512))) - TO_LENGTH(Real(256)));
        CreateObject(&pos, &Identity, OBJECT_TYPE_FIREWORK, flags);

        PodiumVars.FireworkMaxTime = 0.1f + frand(2.5f);
        PodiumVars.FireworkTimer = ZERO;
    }
#endif
}


////////////////////////////////////////////////////////////////
//
// Make the cars dance
//
////////////////////////////////////////////////////////////////

void MakeTheCarsDance()
{
    int iCar;
    REAL rightDown, lookDown;
    PLAYER *player;

    for (iCar = 0; iCar < PodiumVars.CarDropCount; iCar++)
    //iCar = PodiumVars.CarDropCount - 1;
    {

        int playerNum, minContacts;
        if (iCar < 3) {
            playerNum = 2 - iCar;
        } else {
            playerNum = 3;
            return;
        }
        player = &Players[playerNum];

        minContacts = 3;
        if ((player->car.NWheelFloorContacts > minContacts) || (player->car.Body->NBodyColls > 0)) {
            // If all four wheels on the floor, make it jump
            VEC imp;
            SetVec(&imp, 
                ZERO, 
                -MulScalar(player->car.Body->Centre.Mass, TO_VEL(Real(600))),
                ZERO);
            ApplyParticleImpulse(&player->car.Body->Centre, &imp);
            SetVec(&player->car.Body->AngVel,
                ZERO,
                ZERO,
                ZERO);
        }
        else 
        {
            // Make the car spin
#ifndef _PSX
            SetVec(&player->car.Body->AngVel,
                player->car.Body->AngVel.v[X],
                PI * (REAL)sin(10 * PI * PodiumVars.StateTimer / PodiumVars.CarDropTimer),
                player->car.Body->AngVel.v[Z]);
#else
            REAL angle = DivScalar(10 * PodiumVars.StateTimer, PodiumVars.CarDropTimer);
            SetVec(&player->car.Body->AngVel,
                player->car.Body->AngVel.v[X],
                FROM_TIME(MulScalar(PI, sin1616(angle))),
                player->car.Body->AngVel.v[Z]);
#endif
        }


#ifndef _PSX
        // Keep the cars upright
        rightDown = player->car.Body->Centre.WMatrix.m[RY];
        if (abs(rightDown) > Real(0.1)) {
            if (rightDown < ZERO) {
                VecPlusEqScalarVec(&player->car.Body->AngImpulse, 20000 * TimeStep, &player->car.Body->Centre.WMatrix.mv[L]);
            } else {
                VecPlusEqScalarVec(&player->car.Body->AngImpulse, -20000 * TimeStep, &player->car.Body->Centre.WMatrix.mv[L]);
            }
        }
        lookDown = player->car.Body->Centre.WMatrix.m[LY];
        if (abs(lookDown) > Real(0.1)) {
            if (lookDown < ZERO) {
                VecPlusEqScalarVec(&player->car.Body->AngImpulse, -20000 * TimeStep, &player->car.Body->Centre.WMatrix.mv[R]);
            } else {
                VecPlusEqScalarVec(&player->car.Body->AngImpulse, 20000 * TimeStep, &player->car.Body->Centre.WMatrix.mv[R]);
            }
        }
#else
        // Keep the cars upright
        rightDown = player->car.Body->Centre.WMatrix.m[RY];
        if (abs(rightDown) > Real(0.1)) {
            if (rightDown < ZERO) {
                VecPlusEqScalarVec(&player->car.Body->AngImpulse, TimeStep/32, &player->car.Body->Centre.WMatrix.mv[L]);
            } else {
                VecPlusEqScalarVec(&player->car.Body->AngImpulse, -TimeStep/32, &player->car.Body->Centre.WMatrix.mv[L]);
            }
        }
        lookDown = player->car.Body->Centre.WMatrix.m[LY];
        if (abs(lookDown) > Real(0.1)) {
            if (lookDown < ZERO) {
                VecPlusEqScalarVec(&player->car.Body->AngImpulse, -TimeStep/32, &player->car.Body->Centre.WMatrix.mv[R]);
            } else {
                VecPlusEqScalarVec(&player->car.Body->AngImpulse, TimeStep/32, &player->car.Body->Centre.WMatrix.mv[R]);
            }
        }
#endif

        // keep the cars moving
        player->car.Body->NoMoveTime = ZERO;
        player->car.Body->NoContactTime = ZERO;
        player->car.Body->Stacked = FALSE;

    }
}




////////////////////////////////////////////////////////////////
//
// Drop the next cars(s) on the podium
//
////////////////////////////////////////////////////////////////

void DropNextCarOnPodium()
{
    if ((CupTable.LocalPlayerPos > 3) && (PodiumVars.CarDropCount < 3)) {
        // If player not in top three, drop all winners at once
        SetCarPos(&Players[0].car, &PodiumCarDropPos[0], &PodiumCarDropMat);
        SetCarPos(&Players[1].car, &PodiumCarDropPos[1], &PodiumCarDropMat);
        SetCarPos(&Players[2].car, &PodiumCarDropPos[2], &PodiumCarDropMat);
        PodiumVars.CarDropCount = 3;
        PodiumVars.CarDropTimer = WINLOSE_CARDROP_LOSE_TIME;

        // Increase cars's angular resistance to make dancing better
        Players[0].car.Body->DefaultAngRes = Real(0.005);

        // Don't display any positions
        PodiumVars.DisplayWinPos = FALSE;
        PodiumVars.RacePosScale = WINLOSE_POS_STARTSIZE;

    } else {
        int playerNum;
        // Drop cars one by one, if player in top three
        if (PodiumVars.CarDropCount < 3) {
            playerNum = 2 - PodiumVars.CarDropCount;
        } else {
            playerNum = 3;
        }

        SetCarPos(&Players[playerNum].car, &PodiumCarDropPos[playerNum], &PodiumCarDropMat);
        PodiumVars.CarDropCount++;

        // Increase cars's angular resistance to make dancing better
        Players[playerNum].car.Body->DefaultAngRes = Real(0.005);

        // Display this cars position
        PodiumVars.DisplayWinPos = TRUE;
        PodiumVars.RacePosScale = WINLOSE_POS_STARTSIZE;
        PodiumVars.CarDropTimer = WINLOSE_CARDROP_TIME * 2;

        // If this is the player, do some sparkly stuff
        if ((playerNum == 0)) {// + 1 == CupTable.LocalPlayerPos) && (CupTable.LocalPlayerPos != 4)) {
            PodiumVars.DropSparkles = TRUE;
        }

        // Stop the previous car rotating
        if (playerNum < 2) {
            SetVecZero(&Players[playerNum + 1].car.Body->AngVel);
        }

    }

    // If this is the loser - do some stuff...
    if (PodiumVars.CarDropCount == 4) {
        OBJECT *bombObj;
        PUTTYBOMB_OBJ *bombData;
        long flags[4];

        // Change camera viewpoint
        g_pTitleScreenCamera->SetNewCameraPos( TITLESCREEN_CAMPOS_PODIUMLOSE );
        Players[3].ownobj->movehandler = (MOVE_HANDLER)MOV_MoveCarNew;

        // Set car to blow...
        flags[0] = (long)&Players[3];
        flags[1] = FALSE;
        bombObj = CreateObject(&Players[3].car.Body->Centre.Pos, &Players[3].car.Body->Centre.WMatrix, OBJECT_TYPE_PUTTYBOMB, flags);       
        bombData = (PUTTYBOMB_OBJ*)bombObj->Data;
        bombData->Timer = WINLOSE_BOMB_TIME;
    }


}


////////////////////////////////////////////////////////////////
//
// DropSparkles:
//
////////////////////////////////////////////////////////////////

void DropSparkles()
{
#ifndef _PSX
    int playerNum;
    REAL prob;
    VEC pos;

    if (PodiumVars.CarDropCount < 4) {
        playerNum = 3 - PodiumVars.CarDropCount;
    } else {
        playerNum = 3;
    }

    prob = (REAL)sin(PodiumVars.StateTimer * PI / PodiumVars.CarDropTimer) ;

    while (frand(ONE) < prob) {
        SetVec(&pos, 
            PodiumCarDropPos[playerNum].v[X] + TO_LENGTH(Real(70)) - frand(TO_LENGTH(Real(140))),
            PodiumCarDropPos[playerNum].v[Y],
            PodiumCarDropPos[playerNum].v[Z] + TO_LENGTH(Real(70)) - frand(TO_LENGTH(Real(140))));
        
        CreateSpark(SPARK_SPARK3, &pos, &ZeroVector, TO_VEL(Real(100)), 0);

        prob -= ONE;
    }
#endif
}

////////////////////////////////////////////////////////////////
//
// Display Win/Lose Message
//
////////////////////////////////////////////////////////////////

#define PODIUM_TEXT_WIDTH   12
#define PODIUM_TEXT_HEIGHT  24

#define WINLOSE_MESSAGE_WIDTH (PODIUM_TEXT_WIDTH * 30)
#define WINLOSE_MESSAGE_HEIGHT ((PODIUM_TEXT_HEIGHT + MENU_TEXT_VSKIP)* 3)

void DisplayWinLoseMessage()
{
#ifdef _PC
    REAL xPos, yPos, xSize, ySize, x, y;

    xPos = (640 - WINLOSE_MESSAGE_WIDTH) / 2;
    yPos = (480 - WINLOSE_MESSAGE_HEIGHT) / 2;
    xSize = WINLOSE_MESSAGE_WIDTH;
    if (InitialMenuMessage != MENU_MESSAGE_NEWCARS) {
        ySize = PODIUM_TEXT_HEIGHT + MENU_TEXT_VSKIP;
    } else {
        ySize = WINLOSE_MESSAGE_HEIGHT;
    }

    y = yPos;

    // Build the message
    if (CupTable.LocalPlayerPos > 3) return;

    // Box around the text
    DrawSpruBox(
        gMenuWidthScale * (xPos - MENU_TEXT_GAP), 
        gMenuHeightScale * (yPos - MENU_TEXT_GAP),
        gMenuWidthScale * (xSize + MENU_TEXT_GAP * 2),
        gMenuHeightScale * (ySize + MENU_TEXT_GAP * 2), 
        0, 0);  

    // Draw message text
    BeginTextState();


    // Position message
    sprintf(MenuBuffer, "%s %s!", 
        TEXT_TABLE(TEXT_YOU_CAME),
        TEXT_TABLE(TEXT_FIRST + CupTable.LocalPlayerPos - 1));

    x = xPos + (xSize - strlen(MenuBuffer) * Real(PODIUM_TEXT_WIDTH)) / 2;

    DrawBigGameText(x, y, MENU_TEXT_RGB_CHOICE, MenuBuffer);
    y += PODIUM_TEXT_HEIGHT + MENU_TEXT_VSKIP;

    if (InitialMenuMessage != MENU_MESSAGE_NEWCARS) {
        return;
    }

    if (CupTable.LocalPlayerPos == 1) {
        // Player Won
        if (CupTable.CupType != RACE_CLASS_SPECIAL) {
            // Chanpion Chip unlocked message
            sprintf(MenuBuffer, "%s", TEXT_TABLE(TEXT_SILVER_CUP + (CupTable.CupType - RACE_CLASS_BRONZE)));
            x = xPos + (xSize - strlen(MenuBuffer) * Real(PODIUM_TEXT_WIDTH)) / 2;
            DrawBigGameText(x, y, MENU_TEXT_RGB_CHOICE, MenuBuffer);
            y += PODIUM_TEXT_HEIGHT + MENU_TEXT_VSKIP;

            sprintf(MenuBuffer, "%s", TEXT_TABLE(TEXT_IS_UNLOCKED));
            x = xPos + (xSize - strlen(MenuBuffer) * Real(PODIUM_TEXT_WIDTH)) / 2;
            DrawBigGameText(x, y, MENU_TEXT_RGB_CHOICE, MenuBuffer);
            y += PODIUM_TEXT_HEIGHT + MENU_TEXT_VSKIP;
        } else {
            // Now I am the master
            sprintf(MenuBuffer, "Well done, you have mastered");
            x = xPos + (xSize - strlen(MenuBuffer) * Real(PODIUM_TEXT_WIDTH)) / 2;
            DrawBigGameText(x, y, MENU_TEXT_RGB_CHOICE, MenuBuffer);
            y += PODIUM_TEXT_HEIGHT + MENU_TEXT_VSKIP;
            
            sprintf(MenuBuffer, "all Re-Volt Championships!");
            x = xPos + (xSize - strlen(MenuBuffer) * Real(PODIUM_TEXT_WIDTH)) / 2;
            DrawBigGameText(x, y, MENU_TEXT_RGB_CHOICE, MenuBuffer);
            y += PODIUM_TEXT_HEIGHT + MENU_TEXT_VSKIP;

        }
    } else {
        // Player gets a chufty badge, but bugger all else
        if (CupTable.CupType != RACE_CLASS_SPECIAL) {
            x = xPos + (xSize - strlen(TEXT_TABLE(TEXT_TRY_TO_COME_HIGHER_TO)) * Real(PODIUM_TEXT_WIDTH)) / 2;
            DrawBigGameText(x, y, MENU_TEXT_RGB_CHOICE, TEXT_TABLE(TEXT_TRY_TO_COME_HIGHER_TO));
            y += PODIUM_TEXT_HEIGHT + MENU_TEXT_VSKIP;
            
            x = xPos + (xSize - strlen(TEXT_TABLE(TEXT_UNLOCK_THE_NEXT_CHALLENGE)) * Real(PODIUM_TEXT_WIDTH)) / 2;
            DrawBigGameText(x, y, MENU_TEXT_RGB_CHOICE, TEXT_TABLE(TEXT_UNLOCK_THE_NEXT_CHALLENGE));
            y += PODIUM_TEXT_HEIGHT + MENU_TEXT_VSKIP;
        } else {
            sprintf(MenuBuffer, "You'll have to do better than that");
            x = xPos + (xSize - strlen(MenuBuffer) * Real(PODIUM_TEXT_WIDTH)) / 2;
            DrawBigGameText(x, y, MENU_TEXT_RGB_CHOICE, MenuBuffer);
            y += PODIUM_TEXT_HEIGHT + MENU_TEXT_VSKIP;
            
            sprintf(MenuBuffer, "to beat the Re-Volt Championships!");
            x = xPos + (xSize - strlen(MenuBuffer) * Real(PODIUM_TEXT_WIDTH)) / 2;
            DrawBigGameText(x, y, MENU_TEXT_RGB_CHOICE, MenuBuffer);
            y += PODIUM_TEXT_HEIGHT + MENU_TEXT_VSKIP;
        }
    }       
#endif
}


////////////////////////////////////////////////////////////////
//
// Display Win position
//
////////////////////////////////////////////////////////////////

void DisplayWinPosition()
{
#ifdef _PC
    int racePos;
    long col;
    REAL x, y, xsize, ysize, tu, tv;

    // Get current car's finish position
    if (PodiumVars.CarDropCount <= 3) {
        racePos = 4 - PodiumVars.CarDropCount;
    } else {
        racePos = CupTable.LocalPlayerPos;
    }

    // Get draw scale for the number
    PodiumVars.RacePosScale -= TimeStep * WINLOSE_POS_SHRINKSPEED;
    if (PodiumVars.RacePosScale < WINLOSE_POS_ENDSIZE)
        PodiumVars.RacePosScale = WINLOSE_POS_ENDSIZE;

    if (PodiumVars.RacePosScale < WINLOSE_POS_STARTSIZE - 256.0f)
        col = 0xffffffff;
    else
        col = (long)((WINLOSE_POS_STARTSIZE - PodiumVars.RacePosScale)) << 24 | 0xffffff;

    xsize = PodiumVars.RacePosScale;
    ysize = xsize * 2.0f;
    x = 320.0f - xsize * 0.5f;
    y = 0.0f;

    BeginTextState();

    // Draw the tens...
    if ((PodiumVars.CarDropCount > 3) && (CupTable.LocalPlayerPos >= 10))
    {
        x -= xsize * 0.5f;
        tu = 33.0f / 256.0f;
        tv = 129.0f / 256.0f;
        DrawPanelSprite(x, y, xsize, ysize, tu, tv, 32.0f / 256.0f, 64.0f / 256.0f, col);
        x += xsize;
    }

    // Draw the units..
    tu = (((racePos % 10) % 8) * 32.0f + 1.0f) / 256.0f;
    tv = (((racePos % 10) / 8) * 64.0f + 129.0f) / 256.0f;
    DrawPanelSprite(x, y, xsize, ysize, tu, tv, 30.0f / 256.0f, 62.0f / 256.0f, col);
    x += xsize;
    y += ysize * 0.08f;
    ysize *= 0.5f;

    // Draw the th, nd, or st
    if (racePos <= 2)
        tu = 64.0f / 256.0f;
    else
        tu = 96.0f / 256.0f;

    if (racePos <= 4 && racePos & 1)
        tv = 192.0f / 256.0f;
    else
        tv = 224.0f / 256.0f;

    DrawPanelSprite(x, y, xsize, ysize, tu, tv, 32.0f / 256.0f, 32.0f / 256.0f, col);
#endif
}