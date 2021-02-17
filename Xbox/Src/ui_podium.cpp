//-----------------------------------------------------------------------------
// File: ui_podium.cpp
//
// Desc: UI implementation
//
// Re-Volt (Generic) Copyright (c) Probe Entertainment 1998
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"
#include "competition.h"
#include "player.h"
#include "text.h"
#include "main.h"
#include "InitPlay.h"
#include "move.h"
#include "weapon.h"
#include "Obj_Init.h"
#include "spark.h"
#include "panel.h"

#include "ui_Menu.h"
#include "ui_TitleScreen.h"
#include "ui_StateEngine.h"
#include "ui_podium.h"
#include "ui_MenuDraw.h"
#include "ui_MenuText.h"
#include "ui_Options.h"
#include "ui_TopLevelMenu.h"

void DropSparkles();
void DropNextCarOnPodium();
void DisplayWinLoseMessage();
void DisplayWinPosition();
void MakeTheCarsDance();
void UpdateFireworkDisplay();


// Podium Positions
VEC PodiumCarDropPos[PODIUM_NPOS] = 
{
    TO_LENGTH(-1628.0f), TO_LENGTH(-1000.0f), TO_LENGTH(1763.0f),
    TO_LENGTH(-1801.0f), TO_LENGTH(-1000.0f), TO_LENGTH(1588.0f),
    TO_LENGTH(-1448.0f), TO_LENGTH(-1000.0f), TO_LENGTH(1950.0f),
    TO_LENGTH(-1515.0f), TO_LENGTH(-1000.0f), TO_LENGTH(1665.0f),
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

    RotationY(&PodiumCarDropMat, PI);
}


////////////////////////////////////////////////////////////////
//
// Process Win/lose sequence
//
////////////////////////////////////////////////////////////////

#define WINLOSE_MESSAGE_TIME      TO_TIME(3.0f)
#define WINLOSE_CARDROP_TIME      TO_TIME(2.5f)
#define WINLOSE_CARDROP_LOSE_TIME TO_TIME(7.0f)
#define WINLOSE_BOMB_TIME         TO_TIME(8.0f)

#define WINLOSE_EXIT_TIME         TO_TIME(3.0f)
#define WINLOSE_EXIT_LOSE_TIME    TO_TIME(5.5f)
#define WINLOSE_POS_STARTSIZE     300.0f
#define WINLOSE_POS_SHRINKSPEED   500.0f
#define WINLOSE_POS_ENDSIZE        32.0f


void ProcessWinLoseSequence()
{
    // Keep track of amount of time in current state
    PodiumVars.StateTimer += TimeStep;

    // Process sequence state
    switch( PodiumVars.State ) 
    {

    case PODIUM_STATE_INIT:
        // Showing initial win/lose message

        DisplayWinLoseMessage();

        if( PodiumVars.StateTimer > WINLOSE_MESSAGE_TIME ) 
        {
            PodiumVars.StateTimer = ZERO;
            PodiumVars.State = PODIUM_STATE_DROP;
            PodiumVars.CarDropCount = 0;
            PodiumVars.FireworkMaxTime = ZERO;
            PodiumVars.FadeStarted = FALSE;
            g_pTitleScreenCamera->SetMoveTime( TS_CAMERA_MOVE_TIME );
        }
        break;


    case PODIUM_STATE_DROP:
        // Drop the next car
        DropNextCarOnPodium();

        PodiumVars.State = PODIUM_STATE_DROPWAIT;
        break;


    case PODIUM_STATE_DROPWAIT:
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
                g_pTitleScreenCamera->SetNewPos( TITLESCREEN_CAMPOS_PODIUM );
            }
            else if (PodiumVars.StateTimer > PodiumVars.CarDropTimer / 4) 
            {
                int playerNum;
                if (PodiumVars.CarDropCount < 4) {
                    playerNum = 3 - PodiumVars.CarDropCount;
                } else {
                    playerNum = 3;
                }
                g_pTitleScreenCamera->SetNewPos( TITLESCREEN_CAMPOS_PODIUMVIEW1 + playerNum );
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
        // Done with the win/lose sequence

        if( !PodiumVars.FadeStarted && 
            (PodiumVars.StateTimer > ((CupTable.LocalPlayerPos > 3)? WINLOSE_EXIT_LOSE_TIME: WINLOSE_EXIT_TIME)) ) 
        {
            SetFadeEffect(FADE_DOWN);
            PodiumVars.FadeStarted = TRUE;
        }

        if (PodiumVars.FadeStarted && (GetFadeEffect() == FADE_DOWN_DONE))
        {
            PodiumVars.StateTimer = 0.0f;

            // Reset titlescreen
            g_fTitleScreenTimer    = -1.0f;
            g_bShowWinLoseSequence = FALSE;

            if ((CupTable.LocalPlayerPos == 1) && (CupTable.CupType == RACE_CLASS_SPECIAL))
            {
                // Show credits at end of Platinum cup
                SetRaceCredits();
                g_bTitleScreenFadeStarted = TRUE;
            } 
            else 
            {
                // init Camera for menu system
                InitFrontendMenuCamera();
                g_pTitleScreenCamera->SetMoveTime( TS_CAMERA_MOVE_TIME );
        
                // Setup initial menu
				//$TODO - mwetzel - this seems incorrect and unsafe
                g_TopLevelMenuStateEngine.MakeActive( NULL );

                // Setup the dinky cars
                PLR_KillAllPlayers();

                ts_SetupCar();
                InitStartingPlayers();

                // Load carbox textures etc...
                LoadFrontEndTextures();

                // Start fade-in
                SetFadeEffect(FADE_UP);
#ifdef OLD_AUDIO
                // TODO (JHarding): Understand what they're trying to do here - do we
                // need to play with the background music manager here?  I think they're
                // just setting up music for the front end..
                SetRedbookFade(REDBOOK_FADE_UP);
                PlayRedbookTrackRandom(CurrentLevelInfo.RedbookStartTrack, CurrentLevelInfo.RedbookEndTrack, TRUE);
#endif // OLD_AUDIO
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
    // Is it time to generate a new firework
    PodiumVars.FireworkTimer += TimeStep;
    if (PodiumVars.FireworkTimer > PodiumVars.FireworkMaxTime) {
        // Yep, generate a new firework
        VEC pos;
        long flags[4] = {0,0,0,0};

        SetVec(&pos,
            TO_LENGTH(-1628.0f) + frand(TO_LENGTH(512.0f)) - TO_LENGTH(256.0f), 
            TO_LENGTH(-600.0f) - frand(TO_LENGTH(100.0f)), 
            TO_LENGTH(1763.0f) + frand(TO_LENGTH(512.0f)) - TO_LENGTH(256.0f));
        CreateObject(&pos, &Identity, OBJECT_TYPE_FIREWORK, flags);

        PodiumVars.FireworkMaxTime = 0.1f + frand(2.5f);
        PodiumVars.FireworkTimer = ZERO;
    }
}


////////////////////////////////////////////////////////////////
//
// Make the cars dance
//
////////////////////////////////////////////////////////////////

void MakeTheCarsDance()
{
    int iCar;
    FLOAT rightDown, lookDown;
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
                -MulScalar(player->car.Body->Centre.Mass, TO_VEL(600.0f)),
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
            SetVec(&player->car.Body->AngVel,
                player->car.Body->AngVel.v[X],
                PI * (FLOAT)sin(10 * PI * PodiumVars.StateTimer / PodiumVars.CarDropTimer),
                player->car.Body->AngVel.v[Z]);
        }

        // Keep the cars upright
        rightDown = player->car.Body->Centre.WMatrix.m[RY];
        if (abs(rightDown) > 0.1f) {
            if (rightDown < ZERO) {
                VecPlusEqScalarVec(&player->car.Body->AngImpulse, 20000 * TimeStep, &player->car.Body->Centre.WMatrix.mv[L]);
            } else {
                VecPlusEqScalarVec(&player->car.Body->AngImpulse, -20000 * TimeStep, &player->car.Body->Centre.WMatrix.mv[L]);
            }
        }
        lookDown = player->car.Body->Centre.WMatrix.m[LY];
        if (abs(lookDown) > 0.1f) {
            if (lookDown < ZERO) {
                VecPlusEqScalarVec(&player->car.Body->AngImpulse, -20000 * TimeStep, &player->car.Body->Centre.WMatrix.mv[R]);
            } else {
                VecPlusEqScalarVec(&player->car.Body->AngImpulse, 20000 * TimeStep, &player->car.Body->Centre.WMatrix.mv[R]);
            }
        }

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
        Players[0].car.Body->DefaultAngRes = 0.005f;

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
        Players[playerNum].car.Body->DefaultAngRes = 0.005f;

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
        g_pTitleScreenCamera->SetNewPos( TITLESCREEN_CAMPOS_PODIUMLOSE );
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
    int playerNum;
    FLOAT prob;
    VEC pos;

    if (PodiumVars.CarDropCount < 4) {
        playerNum = 3 - PodiumVars.CarDropCount;
    } else {
        playerNum = 3;
    }

    prob = (FLOAT)sin(PodiumVars.StateTimer * PI / PodiumVars.CarDropTimer) ;

    while (frand(ONE) < prob) {
        SetVec(&pos, 
            PodiumCarDropPos[playerNum].v[X] + TO_LENGTH(70.0f) - frand(TO_LENGTH(140.0f)),
            PodiumCarDropPos[playerNum].v[Y],
            PodiumCarDropPos[playerNum].v[Z] + TO_LENGTH(70.0f) - frand(TO_LENGTH(140.0f)));
        
        CreateSpark(SPARK_SPARK3, &pos, &ZeroVector, TO_VEL(100.0f), 0);

        prob -= ONE;
    }
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
    FLOAT xPos, yPos, xSize, ySize, x, y;

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
    swprintf(MenuBuffer, L"%s %s!", TEXT_TABLE(TEXT_YOU_FINISHED),
                                    TEXT_TABLE(TEXT_FIRST + CupTable.LocalPlayerPos - 1) );

    x = xPos + (xSize - wcslen(MenuBuffer) * (FLOAT)(PODIUM_TEXT_WIDTH)) / 2;

    DumpTextReal(x, y, PODIUM_TEXT_WIDTH, PODIUM_TEXT_HEIGHT, MENU_TEXT_RGB_CHOICE, MenuBuffer);
    y += PODIUM_TEXT_HEIGHT + MENU_TEXT_VSKIP;

    if (InitialMenuMessage != MENU_MESSAGE_NEWCARS) {
        return;
    }

    if (CupTable.LocalPlayerPos == 1) {
        // Player Won
        if (CupTable.CupType != RACE_CLASS_SPECIAL) {
            // Chanpion Chip unlocked message
            swprintf(MenuBuffer, L"%s", TEXT_TABLE(TEXT_SILVER_CUP + (CupTable.CupType - RACE_CLASS_BRONZE)));
            x = xPos + (xSize - wcslen(MenuBuffer) * (FLOAT)(PODIUM_TEXT_WIDTH)) / 2;
            DumpTextReal(x, y, PODIUM_TEXT_WIDTH, PODIUM_TEXT_HEIGHT, MENU_TEXT_RGB_CHOICE, MenuBuffer);
            y += PODIUM_TEXT_HEIGHT + MENU_TEXT_VSKIP;

            swprintf(MenuBuffer, L"%s", TEXT_TABLE(TEXT_IS_UNLOCKED));
            x = xPos + (xSize - wcslen(MenuBuffer) * (FLOAT)(PODIUM_TEXT_WIDTH)) / 2;
            DumpTextReal(x, y, PODIUM_TEXT_WIDTH, PODIUM_TEXT_HEIGHT, MENU_TEXT_RGB_CHOICE, MenuBuffer);
            y += PODIUM_TEXT_HEIGHT + MENU_TEXT_VSKIP;
        } else {
            // Now I am the master
            g_pFont->DrawText( xPos, y, MENU_TEXT_RGB_CHOICE, TEXT_TABLE(TEXT_PODIUM_FINISHED), XBFONT_CENTER_X );
            y += PODIUM_TEXT_HEIGHT + MENU_TEXT_VSKIP;
            y += PODIUM_TEXT_HEIGHT + MENU_TEXT_VSKIP;

        }
    } 
    else 
    {
        // Player gets a chufty badge, but bugger all else
        if (CupTable.CupType != RACE_CLASS_SPECIAL) 
        {
            x = xPos + (xSize - wcslen(TEXT_TABLE(TEXT_TRY_TO_COME_HIGHER_TO)) * (FLOAT)(PODIUM_TEXT_WIDTH)) / 2;
            DumpTextReal(x, y, PODIUM_TEXT_WIDTH, PODIUM_TEXT_HEIGHT, MENU_TEXT_RGB_CHOICE, TEXT_TABLE(TEXT_TRY_TO_COME_HIGHER_TO));
            y += PODIUM_TEXT_HEIGHT + MENU_TEXT_VSKIP;
            
            x = xPos + (xSize - wcslen(TEXT_TABLE(TEXT_UNLOCK_THE_NEXT_CHALLENGE)) * (FLOAT)(PODIUM_TEXT_WIDTH)) / 2;
            DumpTextReal(x, y, PODIUM_TEXT_WIDTH, PODIUM_TEXT_HEIGHT, MENU_TEXT_RGB_CHOICE, TEXT_TABLE(TEXT_UNLOCK_THE_NEXT_CHALLENGE));
            y += PODIUM_TEXT_HEIGHT + MENU_TEXT_VSKIP;
        } 
        else 
        {
            g_pFont->DrawText( xPos, y, MENU_TEXT_RGB_CHOICE, TEXT_TABLE(TEXT_PODIUM_DOBETTER), XBFONT_CENTER_X );
            y += PODIUM_TEXT_HEIGHT + MENU_TEXT_VSKIP;
            y += PODIUM_TEXT_HEIGHT + MENU_TEXT_VSKIP;
        }
    }       
}


////////////////////////////////////////////////////////////////
//
// Display Win position
//
////////////////////////////////////////////////////////////////

void DisplayWinPosition()
{
    int racePos;
    long col;
    FLOAT x, y, xsize, ysize, tu, tv;

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
}
