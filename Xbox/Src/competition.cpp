//-----------------------------------------------------------------------------
// File: competition.cpp
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"
#include "competition.h"
#include "LevelInfo.h"
#include "geom.h"
#include "camera.h"
#include "player.h"
#include "timing.h"
#include "initplay.h"
#include "posnode.h"
#include "LevelLoad.h"

#include "ui_TitleScreen.h"
#include "ui_menu.h"
#include "ui_MenuText.h"
#include "ui_Confirm.h"

#include "MusicManager.h"
#include "SoundEffectEngine.h"

#ifdef _PSX
#include "pad.h"
#include "overlay.h"
#endif


#ifdef _PC
#include "panel.h"
#include "XBInput.h"
#endif

#ifdef _N64
#include "ISound.h"
#include "panel.h"

#endif


// globals

CHAMPIONSHIP_END_MODE ChampionshipEndMode;
static REAL AwardPointsTimer;

extern MENU Menu_InGame;
extern WCHAR MenuBuffer[];
// cup info table

CUP_TABLE CupTable;

// tracks per cup

CUP_INFO CupData[RACE_CLASS_NTYPES] = {
    { // DEFAULT
        0,
    },
    { // BRONZE CUP
        4,                          // Number of races
        
        LEVEL_NEIGHBOURHOOD1, 3, FALSE, FALSE,
        LEVEL_SUPERMARKET2, 4, FALSE, FALSE,
        LEVEL_MUSEUM2, 3, FALSE, FALSE,
        LEVEL_GARDEN1, 5, FALSE, FALSE,
    },
    { // SILVER CUP
        4,                          // Number of races
        LEVEL_TOYWORLD1, 6, FALSE, FALSE,
        LEVEL_NEIGHBOURHOOD1, 4, TRUE, FALSE,
        LEVEL_GHOSTTOWN1, 4, FALSE, FALSE,
        LEVEL_TOYWORLD2, 4, FALSE, FALSE,
    },
    { // GOLD CUP
        4,                          // Number of races
        LEVEL_NEIGHBOURHOOD2, 5, FALSE, FALSE,
        LEVEL_TOYWORLD1, 8, TRUE, FALSE,
        LEVEL_CRUISE1, 5, FALSE, FALSE,
        LEVEL_MUSEUM1, 5, FALSE, FALSE,
    },
    { // SPECIAL CUP
        5,                          // Number of races
        LEVEL_SUPERMARKET1, 6, FALSE, FALSE,
        LEVEL_GHOSTTOWN2, 6, FALSE, FALSE,
        LEVEL_TOYWORLD1, 10, TRUE, TRUE,
        LEVEL_MUSEUM1, 6, TRUE, FALSE,
        LEVEL_CRUISE2, 6, FALSE, FALSE,
    },
};

// cup name text


// car classes allowed per cup

static long CupCarPick[RACE_CLASS_NTYPES][RACE_CLASS_NTYPES] = {
#ifdef _PC
    {0, 0, 0, 0, 0},        // default
    {7, 0, 0, 0, 0},        // bronze
    {0, 4, 3, 0, 0},        // silver
    {0, 0, 4, 3, 0},        // gold
    {0, 0, 1, 3, 3},        // special
#else
    {0, 0, 0, 0, 0},        // default
    {3, 0, 0, 0, 0},        // bronze
    {0, 2, 1, 0, 0},        // silver
    {0, 0, 2, 1, 0},        // gold
    {0, 0, 0, 1, 2},        // special
#endif
};

// points awarded per position

#ifdef _PC
  #ifdef SHIPPING
  //$WARNING: these values have not been adjusted; I just truncted the list.
  // Seems okay for now (May 2002) since CupPointsAwarded[] isn't used in shipping versions, right?  But should $REVISIT later.
static long CupPointsAwarded[DEFAULT_RACE_CARS] = {10, 6, 4, 3, 2, 1};
  #else
static long CupPointsAwarded[DEFAULT_RACE_CARS] = {10, 6, 4, 3, 2, 1, 0, 0};
  #endif //SHIPPING
#else
#ifdef _N64
static long CupPointsAwarded[DEFAULT_RACE_CARS] = {5, 4, 3, 0};
#else
static long CupPointsAwarded[DEFAULT_RACE_CARS] = {5, 3, 2, 1};
#endif
#endif

//////////////////////////////////
// init cup table with car id's //
//////////////////////////////////

static long carnum[RACE_CLASS_NTYPES];
static long classid[RACE_CLASS_NTYPES][CARID_NTYPES];
static long carpicked[DEFAULT_RACE_CARS];


void InitCupTable(void)
{
    long i, j, k, car, numpicked;

// set cup info

    CupTable.CupType = gTitleScreenVars.CupType;
    CupTable.RaceNum = 0;
    CupTable.TriesLeft = CUP_NUM_TRIES;
    CupTable.CupCompleted = FALSE;

    CupTable.LocalPlayerPos = 0;
    CupTable.WinLoseCarType[0] = CARID_RC;
    CupTable.WinLoseCarType[1] = CARID_RC;
    CupTable.WinLoseCarType[2] = CARID_RC;
    CupTable.WinLoseCarType[3] = CARID_RC;

// count car num for each class

    for (i = 0 ; i < RACE_CLASS_NTYPES ; i++)
        carnum[i] = 0;

    for (i = 0 ; i < CARID_NTYPES ; i++)
    {
#ifndef _N64
        if ((i == CARID_TROLLEY) || (i == CARID_KEY1) || (i == CARID_KEY2) || (i == CARID_KEY3) || (i == CARID_KEY4) || (i == CARID_ROTOR) || (i == CARID_PANGA) || (i == CARID_UFO))
#else
        if ((i == CARID_TROLLEY) || (i == CARID_KEY1) || (i == CARID_KEY2) || (i == CARID_KEY3) || (i == CARID_KEY4) || (i == CARID_ROTOR) || (i == CARID_PANGA))
#endif
            continue;

#ifdef _PC
        if (i == CARID_MYSTERY)
            continue;
#endif

        classid[CarInfo[i].Rating][carnum[CarInfo[i].Rating]++] = i;
    }

// pick random cars for each class
#ifndef _PSX
    carpicked[0] = gTitleScreenVars.PlayerData[0].iCarNum;
    numpicked = 1;
#else
    carpicked[0] = gTitleScreenVars.PlayerData[0].iCarNum;

    switch (CupTable.CupType)
    {
        case RACE_CLASS_BRONZE:
            carpicked[1] = CARID_VOLKEN;
            if (carpicked[0] == carpicked[1])
                carpicked[1] = CARID_PHATSLUG;
            break;
        case RACE_CLASS_SILVER:
            carpicked[1] = CARID_PANGATC;
            if (carpicked[0] == carpicked[1])
                carpicked[1] = CARID_BERTHA;
            break;
        case RACE_CLASS_GOLD:
            carpicked[1] = CARID_ADEON;
            if (carpicked[0] == carpicked[1])
                carpicked[1] = CARID_COUGAR;
            break;
        case RACE_CLASS_SPECIAL:
            carpicked[1] = CARID_TOYECA;
            if (carpicked[0] == carpicked[1])
                carpicked[1] = CARID_COUGAR;
            break;
    }

    numpicked = 2;
#endif

// loop thru each class

    for (i = 0 ; i < RACE_CLASS_NTYPES ; i++)
    {

// pick correct amount of cars for said class

        for (j = 0 ; j < CupCarPick[CupTable.CupType][i] ; j++)
        {
            if (!carnum[i])
            {
                car = 0;
                k = numpicked;
            }
            else
            {
                car = classid[i][rand() % carnum[i]];

                for (k = 0 ; k < numpicked ; k++)
                    if (car == carpicked[k])
                        break;
            }

            if (k == numpicked)
                carpicked[numpicked++] = car;
            else
                j--;
        }
    }

// set car ID's

    for (i = 0 ; i < DEFAULT_RACE_CARS ; i++)
    {
        CupTable.Player[i].PlayerSlot = i;
        CupTable.Player[i].CarType = carpicked[i];
        CupTable.Player[i].Points = 0;
    }
}

////////////////////////
// init one cup level //
////////////////////////

void InitOneCupLevel(void)
{
    long i;

// set game settings

    GameSettings.GameType = GAMETYPE_CHAMPIONSHIP;
    GameSettings.Level = CupData[CupTable.CupType].Level[CupTable.RaceNum].Num;
    GameSettings.Mirrored = CupData[CupTable.CupType].Level[CupTable.RaceNum].Mir;
    GameSettings.Reversed = CupData[CupTable.CupType].Level[CupTable.RaceNum].Rev;
    GameSettings.AllowPickups = gTitleScreenVars.pickUps;

// setup start data

    InitStartData();
    StartData.GameType = GAMETYPE_CHAMPIONSHIP;
    StartData.Laps = GameSettings.NumberOfLaps = CupData[CupTable.CupType].Level[CupTable.RaceNum].Laps;

#ifdef _PC
    LEVELINFO *li = GetLevelInfo(GameSettings.Level);
    strncpy(StartData.LevelDir, li->szDir, MAX_PATH);
#endif

    AddPlayerToStartData(PLAYER_LOCAL, 0, CupTable.Player[0].CarType, 0, 0, CTRL_TYPE_LOCAL, 0, gTitleScreenVars.PlayerData[0].nameEnter);

    for (i = 1 ; i < DEFAULT_RACE_CARS ; i++)
    {
        AddPlayerToStartData(PLAYER_CPU, i, CupTable.Player[i].CarType, 0, 0, CTRL_TYPE_CPU_AI, 0, CarInfo[CupTable.Player[i].CarType].Name);
    }

// randomize start grid

    RandomizeStartingGrid();
}


////////////////////////////////////////////////////////////////
//
// have all time trial ghosts been beaten on this cup?
//
////////////////////////////////////////////////////////////////

bool IsCupBeatTimeTrials(int cup)
{
    long i;

    for (i = 0 ; i < LEVEL_NCUP_LEVELS ; i++) if (GetLevelInfo(i)->LevelClass == cup)
    {
        if (!IsSecretBeatTimeTrial(i))
        {
            return FALSE;
        }
    }

    return TRUE;
}

////////////////////////////////////////////////////////////////
//
// have all reversed time trial ghosts been beaten on this cup?
//
////////////////////////////////////////////////////////////////

bool IsCupBeatTimeTrialsReversed(int cup)
{
    long i;

    for (i = 0 ; i < LEVEL_NCUP_LEVELS ; i++) if (GetLevelInfo(i)->LevelClass == cup)
    {
        if (!IsSecretBeatTimeTrialReverse(i))
        {
            return FALSE;
        }
    }

    return TRUE;
}

////////////////////////////////////////////////////////////////
//
// have all mirrored time trial ghosts been beaten on this cup?
//
////////////////////////////////////////////////////////////////

bool IsCupBeatTimeTrialsMirrored(int cup)
{
    long i;

    for (i = 0 ; i < LEVEL_NCUP_LEVELS ; i++) if (GetLevelInfo(i)->LevelClass == cup)
    {
        if (!IsSecretBeatTimeTrialMirror(i))
        {
            return FALSE;
        }
    }

    return TRUE;
}

////////////////////////////////////////////////////////////////
//
// found cup practise stars
//
////////////////////////////////////////////////////////////////

bool IsCupFoundPractiseStars(int cup)
{
    long i;

    for (i = 0 ; i < LEVEL_NCUP_LEVELS ; i++) if (GetLevelInfo(i)->LevelClass == cup)
    {
        if (!IsSecretFoundPractiseStars(i))
        {
            return FALSE;
        }
    }

    return TRUE;
}

////////////////////////////////////////////////////////////////
//
// won cup single races
//
////////////////////////////////////////////////////////////////

bool IsCupWonSingleRaces(int cup)
{
    long i;

    for (i = 0 ; i < LEVEL_NCUP_LEVELS ; i++) if (GetLevelInfo(i)->LevelClass == cup)
    {
        if (!IsSecretWonSingleRace(i))
        {
            return FALSE;
        }
    }

    return TRUE;
}

////////////////////////////////////////////////////////////////
//
// is cup completed
//
////////////////////////////////////////////////////////////////

bool IsCupCompleted(int cup)
{

    long i;

    for (i = 0 ; i < LEVEL_NCUP_LEVELS ; i++) if (GetLevelInfo(i)->LevelClass == cup)
    {
        if (!IsSecretCupCompleted(i))
        {
            return FALSE;
        }
    }

    return TRUE;
}

////////////////////////////////////////////////////////////////
//
// set cup completed
//
////////////////////////////////////////////////////////////////

void SetCupCompleted(int cup)
{
    long i;

    for (i = 0 ; i < LEVEL_NCUP_LEVELS ; i++) if (GetLevelInfo(i)->LevelClass == cup)
    {
        SetSecretCupCompleted(i);
    }
}

////////////////////////////////////////////////////////////////
//
// force all cars to finish
//
////////////////////////////////////////////////////////////////

void ForceAllCarFinish(void)
{

    PLAYER *player;
    REAL TotalRaceDist, dist, findist;

// calc total race distance

#ifndef _PSX
    TotalRaceDist = PosTotalDist * GameSettings.NumberOfLaps;
#else
    TotalRaceDist = MulScalar( PosTotalDist, GameSettings.NumberOfLaps<<16 );
#endif

// find all unfinished players

    for (player = PLR_PlayerHead ; player ; player = player->next) if (player->type == PLAYER_CPU && !player->RaceFinishTime)
    {

// calc my distance travelled

        findist = player->CarAI.FinishDist;
        if (findist < 0) findist += PosTotalDist;
        if (findist >= PosTotalDist) findist = PosTotalDist;

#ifndef _PSX
        dist = (player->car.Laps + 1) * PosTotalDist - findist;
#else
        dist = MulScalar( (player->car.Laps + 1)<<16, PosTotalDist ) - findist;
#endif

// 'invent' my finish time

#ifndef _PSX
        SetPlayerFinished(player, (long)(TotalRaceTime / dist * TotalRaceDist));
#else
        SetPlayerFinished(player, (long)( MulScalar( DivScalar( TotalRaceTime, dist ), TotalRaceDist )));
#endif

    }

}

////////////////////////////////////////////////////////////////
//
// maintain championship end screens
//
////////////////////////////////////////////////////////////////

#ifdef _PSX
long ChampTableOff;
long TableTableOff;
#endif

void MaintainChampionshipEndScreens(void)
{
    long i, j, flag;
    CUP_TABLE_PLAYER swap;
#ifdef _PSX //<-- $ADDITION
    int x;
    char *pStr;
#endif //<-- $ADDITION

// what to do?


    switch (ChampionshipEndMode)
    {

// have we finished?

        case CHAMPIONSHIP_END_WAITING_FOR_FINISH:

            if (AllPlayersFinished)
            {
#ifdef _PC
                SetConsoleMessage( TEXT_TABLE(TEXT_PRESS_ANY_BUTTON), NULL, MENU_COLOR_GREEN, MENU_COLOR_WHITE, 0x7ffffffe, CONSOLE_MESSAGE_FOREVER );
#endif
#ifdef _PSX
                ChampTableOff = 640;
                SFX_KillChannels();
#endif 

                ChampionshipEndMode = CHAMPIONSHIP_END_FINISHED;
            }
            break;
    
// finished, wait for any key

        case CHAMPIONSHIP_END_FINISHED:
        case CHAMPIONSHIP_END_GAVEUP:
        
            #ifdef _PSX
            if( ChampionshipEndMode != CHAMPIONSHIP_END_GAVEUP )
            {
//              DrawFont( 320-(10*13), 180, "Press   to Continue", 64, 64, 128, 64, 64, 128, &FrontEndOrderingTable[Toggle][0], 1 );
//              DrawPSXButton( 260, 177 , 2 );

                x = (640-(strlen(DrawOverlayStr[OVERLAYSTR_CONTINUE])*13))/2;
                DrawFont( x, 180, DrawOverlayStr[OVERLAYSTR_CONTINUE], 64, 64, 128, 64, 64, 128, &FrontEndOrderingTable[Toggle][0], 1 );

                pStr = strstr(DrawOverlayStr[OVERLAYSTR_CONTINUE], "   ");
                if (pStr != NULL)
                {
                    x += ((pStr -  DrawOverlayStr[OVERLAYSTR_CONTINUE]) * 13);
                    x += 4;
                }

                DrawPSXButton( x, 177, 2 );
            }
            #endif


#ifdef _PSX
            if( ( (PadInfo[0].Button & STEF_PAD_RD) && !(PadInfo[0].ButtonLast & STEF_PAD_RD) )  || (ChampionshipEndMode == CHAMPIONSHIP_END_GAVEUP && g_pMenuHeader->m_State == MENU_STATE_OFFSCREEN) )
#else
            if (g_bAnyButtonPressed || (ChampionshipEndMode == CHAMPIONSHIP_END_GAVEUP && g_pMenuHeader->m_State == MENU_STATE_OFFSCREEN))
#endif
            {
                GameSettings.Paws = TRUE;


                

                AwardPointsTimer = TO_TIME(Real(2));
                UpdateChampionshipData();

#ifndef _PSX
#ifdef OLD_AUDIO
                PauseAllSfx();
                PlaySfx(SFX_MENU_FORWARD, SFX_MAX_VOL, SFX_CENTRE_PAN, 44100, 0x7fffffff);
#else
                g_SoundEngine.PauseAll();
                g_MusicManager.Pause();
                g_SoundEngine.Play2DSound( EFFECT_MenuNext, FALSE );
#endif // OLD_AUDIO
#endif

#ifndef _PSX
//$REVISIT - might actually to keep this, depending on how we implement vibration
//$REMOVED(tentative!!)                SetSafeAllJoyForces();
#endif
                // qualified
                if (Players[0].RaceFinishPos < CUP_QUALIFY_POS)
                {
                    CupTable.QualifyFlag = CUP_QUALIFY_YES;
                    ChampionshipEndMode = CHAMPIONSHIP_END_QUALIFIED;
                }
                // failed to qualify
                else
                {
                    // no tries, game over
                    if (!CupTable.TriesLeft)
                    {
                        SetConfirmMenuStrings(TEXT_TABLE(TEXT_COMPETE_GAMEOVER), L"", L"", CONFIRM_YES);
                        CupTable.QualifyFlag = CUP_QUALIFY_GAMEOVER;

                        if (ChampionshipEndMode == CHAMPIONSHIP_END_GAVEUP)
                        {
                            for (i = 0 ; i < 3 ; i++)
                            {
                                CupTable.WinLoseCarType[i] = CupTable.Player[i + 1].CarType;
                            }
                            CupTable.LocalPlayerPos = DEFAULT_RACE_CARS;

                        }
                        else
                        {
                            for (i = 0 ; i < DEFAULT_RACE_CARS ; i++) if (CupTable.Player[i].FinishPos[CupTable.RaceNum] < 3)
                            {
                                CupTable.WinLoseCarType[CupTable.Player[i].FinishPos[CupTable.RaceNum]] = CupTable.Player[i].CarType;
                            }
                            CupTable.LocalPlayerPos = CupTable.Player[0].FinishPos[CupTable.RaceNum] + 1;
                        }

                        CupTable.WinLoseCarType[3] = CupTable.Player[0].CarType;

                        g_bShowWinLoseSequence = TRUE;

                        g_pMenuHeader->ClearMenuHeader();
                        Menu_ConfirmYesNo.ParentMenu = NULL;
                        g_pMenuHeader->SetNextMenu( &Menu_ConfirmYesNo);
                    }
                    // use another try
                    else
                    {
                        if (ChampionshipEndMode != CHAMPIONSHIP_END_GAVEUP)
                        {
                            if( CupTable.TriesLeft == 1 )
                                swprintf( MenuBuffer, TEXT_TABLE(TEXT_CONFIRM_GIVEUP_YOUHAVE_1_TRYLEFT) );
                            else
                                swprintf( MenuBuffer, TEXT_TABLE(TEXT_CONFIRM_GIVEUP_YOUHAVE_N_TRIESLEFT), CupTable.TriesLeft );
                            wcscat( MenuBuffer, L" " );
                            wcscat( MenuBuffer, TEXT_TABLE(TEXT_TRYAGAIN) );

                            SetConfirmMenuStrings( MenuBuffer, TEXT_TABLE(TEXT_YES), TEXT_TABLE(TEXT_NO), CONFIRM_YES );

                            g_pMenuHeader->ClearMenuHeader();
                            Menu_ConfirmYesNo.ParentMenu = NULL;
                            g_pMenuHeader->SetNextMenu( &Menu_ConfirmYesNo);
                        }
#ifndef _PSX
                        SetConsoleMessage( NULL, NULL, 0x00000000, 0x00000000, 0x7fffffff, CONSOLE_MESSAGE_FOREVER );
#endif
                        CupTable.QualifyFlag = CUP_QUALIFY_TRIESLEFT;
                    }

                    ChampionshipEndMode = CHAMPIONSHIP_END_FAILED;
                }
            }
            break;

// failed to qualify

        case CHAMPIONSHIP_END_FAILED:

            // progress?
#ifdef _PSX
            if( !CupTable.TriesLeft && ( (PadInfo[0].Button & STEF_PAD_RD) && !(PadInfo[0].ButtonLast & STEF_PAD_RD) )   )
                MenuGoBack(g_pMenuHeader, g_pMenuHeader->m_pMenu, NULL);
#else
            if ( !CupTable.TriesLeft && g_bAnyButtonPressed)
                MenuGoBack(g_pMenuHeader, g_pMenuHeader->m_pMenu, NULL);
#endif

            if ((g_pMenuHeader->m_State == MENU_STATE_OFFSCREEN) || (CupTable.QualifyFlag == CUP_QUALIFY_TRIESLEFT && ChampionshipEndMode == CHAMPIONSHIP_END_GAVEUP))
            {
                if (gConfirmMenuReturnVal == CONFIRM_NO) 
                    CupTable.QualifyFlag = CUP_QUALIFY_GAMEOVER;

                g_pMenuHeader->ClearMenuHeader();
                g_pMenuHeader->SetNextMenu( &Menu_InGame);
#ifndef _PSX
                SetConsoleMessage( NULL, NULL, 0, 0, 0x7fffffff, CONSOLE_MESSAGE_FOREVER );
#endif
                ChampionshipEndMode = CHAMPIONSHIP_END_MENU;
            }

            break;

// showing league table, give points + wait for any key

        case CHAMPIONSHIP_END_QUALIFIED:


            // award points
            AwardPointsTimer -= TimeStep;
            if (AwardPointsTimer < Real(0))
            {
                AwardPointsTimer = TO_TIME(Real(0.2));

                flag = FALSE;
                for (i = 0 ; i < StartData.PlayerNum ; i++) if (CupTable.Player[i].NewPoints)
                {
                    CupTable.Player[i].Points++;
                    CupTable.Player[i].NewPoints--;
                    flag = TRUE;
                }

#ifndef _PSX
                if (flag)
#ifdef OLD_AUDIO
                    PlaySfx(SFX_MENU_LEFTRIGHT, SFX_MAX_VOL, SFX_CENTRE_PAN, SFX_SAMPLE_RATE, 0x7fffffff);
#else
                    g_SoundEngine.Play2DSound( EFFECT_MenuLeftRight, FALSE );
#endif // OLD_AUDIO
#endif
            }

            // finish awarding or go to menu

            
#ifdef _PSX

//          DrawFont( 320-(10*13), 180, "Press   to Continue", 64, 64, 128, 64, 64, 128, &FrontEndOrderingTable[Toggle][0], 1 );
//          DrawPSXButton( 260, 177 , 2 );

            x = (640-(strlen(DrawOverlayStr[OVERLAYSTR_CONTINUE])*13))/2;
            DrawFont( x, 180, DrawOverlayStr[OVERLAYSTR_CONTINUE], 64, 64, 128, 64, 64, 128, &FrontEndOrderingTable[Toggle][0], 1 );

            pStr = strstr(DrawOverlayStr[OVERLAYSTR_CONTINUE], "   ");
            if (pStr != NULL)
            {
                x += ((pStr -  DrawOverlayStr[OVERLAYSTR_CONTINUE]) * 13);
                x += 4;
            }

            DrawPSXButton( x, 177, 2 );

#endif

#ifdef _N64
            g_pMenuHeader->ClearMenuHeader();
#endif

            
#ifdef _PSX
            if( ( (PadInfo[0].Button & STEF_PAD_RD) && !(PadInfo[0].ButtonLast & STEF_PAD_RD) )  )
#else
            if (g_bAnyButtonPressed)
#endif

            {
                flag = FALSE;
                for (i = 0 ; i < StartData.PlayerNum ; i++) if (CupTable.Player[i].NewPoints)
                {
                    CupTable.Player[i].Points += CupTable.Player[i].NewPoints;
                    CupTable.Player[i].NewPoints = 0;
                    flag = TRUE;
                }

                if (!flag)
                {
                    g_pMenuHeader->ClearMenuHeader();
                    g_pMenuHeader->SetNextMenu( &Menu_InGame);
#ifndef _PSX
                    SetConsoleMessage( NULL, NULL, 0, 0, 0x7fffffff, CONSOLE_MESSAGE_FOREVER );
#endif
#ifndef _PSX
#ifdef OLD_AUDIO
                    PlaySfx(SFX_MENU_BACK, SFX_MAX_VOL, SFX_CENTRE_PAN, 44100, 0x7fffffff);
#else
                    g_SoundEngine.Play2DSound( EFFECT_MenuPrev, FALSE );
#endif // OLD_AUDIO
#endif
                    ChampionshipEndMode = CHAMPIONSHIP_END_MENU;
                }
            }

            // sort player order
            for (i = 0 ; i < StartData.PlayerNum ; i++)
            {
                CupTable.PlayerOrder[i] = CupTable.Player[i];
            }

            for (i = StartData.PlayerNum - 1 ; i ; i--) for (j = 0 ; j < i ; j++)
            {
                if (CupTable.PlayerOrder[j].Points < CupTable.PlayerOrder[j + 1].Points ||
//                  (CupTable.PlayerOrder[j].Points == CupTable.PlayerOrder[j + 1].Points && CupTable.PlayerOrder[j].FinishPos[CupTable.RaceNum] < CupTable.PlayerOrder[j + 1].FinishPos[CupTable.RaceNum]))
                    (CupTable.PlayerOrder[j].Points == CupTable.PlayerOrder[j + 1].Points && Players[CupTable.PlayerOrder[j].PlayerSlot].RaceFinishPos < Players[CupTable.PlayerOrder[j + 1].PlayerSlot].RaceFinishPos))
                {
                    swap = CupTable.PlayerOrder[j];
                    CupTable.PlayerOrder[j] = CupTable.PlayerOrder[j + 1];
                    CupTable.PlayerOrder[j + 1] = swap;
                }
            }

            for (i = 0 ; i < StartData.PlayerNum ; i++) if (!CupTable.PlayerOrder[i].PlayerSlot)
            {
                CupTable.LocalPlayerPos = i + 1;
                break;
            }   

            break;
    }
}

////////////////////////////////////////////////////////////////
//
// award championship points etc
//
////////////////////////////////////////////////////////////////

void UpdateChampionshipData(void)
{
    long i;

// save pos + times, add points

    for (i = 0 ; i < DEFAULT_RACE_CARS ; i++)
    {
        CupTable.Player[i].FinishPos[CupTable.RaceNum] = Players[i].RaceFinishPos;
        CupTable.Player[i].FinishTime[CupTable.RaceNum] = Players[i].RaceFinishTime;
        CupTable.Player[i].NewPoints = CupPointsAwarded[Players[i].RaceFinishPos];
    }
}

//////////////////////////////////
// go to next championship race //
//////////////////////////////////

void GoToNextChampionshipRace(void)
{
    long i;
    bool unlock;

// unload level

    LEV_EndLevelStageTwo();
    LEV_EndLevelStageOne();

// inc race num if not re-trying

    if (CupTable.QualifyFlag == CUP_QUALIFY_YES)
    {
        CupTable.RaceNum++;
    }
    else
    {
        CupTable.TriesLeft--;
    }

// finished cup?

    if (CupTable.RaceNum == CupData[CupTable.CupType].NRaces)
    {

// yep, set cup completed

        CupTable.CupCompleted = TRUE;

// set podium data

        for (i = 0 ; i < 3 ; i++)
            CupTable.WinLoseCarType[i] = CupTable.PlayerOrder[i].CarType;

        CupTable.WinLoseCarType[3] = CupTable.Player[0].CarType;

// unlock next cup if we won championship

        if (CupTable.LocalPlayerPos == 1)
        {
            unlock = IsCupCompleted(CupTable.CupType);
            SetCupCompleted(CupTable.CupType);
            if (!unlock && IsCupCompleted(CupTable.CupType))
            {
                InitialMenuMessage = MENU_MESSAGE_NEWCARS;
            }
        }

// enable podium

        g_bShowWinLoseSequence = TRUE;

// go front end

#ifndef _N64
        SET_EVENT(GoTitleScreen);
#else
        GameState = GS_INIT_TITLESCREEN;
#endif

    }

// nope, next race

    else
    {
        InitOneCupLevel();
        SetupGame(); //$REVISIT: should this call SetupGame directly?  Most other code uses SET_EVENT(SetupGame).
    }

}

#ifdef _PSX

/////////////////////////////
// draw championship table //
/////////////////////////////

void DrawChampionshipTable(void)
{
    long i, j;
    char buf[128];
    short x;


    // set ChampTableOffset

    if (ChampionshipEndMode == CHAMPIONSHIP_END_QUALIFIED)
    {
    
        ChampTableOff -= 34*TimeFactor;
        if( ChampTableOff < 0 )
            ChampTableOff = 0;

    }
    else
    {

        ChampTableOff += 34*TimeFactor;
        if( ChampTableOff > 640 )
            ChampTableOff = 640;
    }


    if( ChampTableOff == 640 )
        return;


    DrawSpruBox( 80+64+ChampTableOff, 80, 480-128, 61-((4-StartData.PlayerNum)*9), 0, 0 );  
    
//  DrawFont( 320-(9*13)+ChampTableOff, 86, "Championship Table", 64, 64, 128, 64, 64, 128, &FrontEndOrderingTable[Toggle][0], 1 );
    DrawFont( (640-(wcslen(TEXT_TABLE(TEXT_CHAMPIONSHIP))*13))/2, 86, TEXT_TABLE(TEXT_CHAMPIONSHIP), 64, 64, 128, 64, 64, 128, &FrontEndOrderingTable[Toggle][0], 1 );


    for (i = 0 ; i < StartData.PlayerNum ; i++)
    {
        sprintf(buf, "%s", Players[CupTable.PlayerOrder[i].PlayerSlot].PlayerName);
        DrawFont( 96+64+ChampTableOff, 100+(9*i), buf, 128, 128, 128, 128, 128, 128, &FrontEndOrderingTable[Toggle][0], 1 );

        sprintf(buf, "%2ld", CupTable.PlayerOrder[i].Points );
        DrawFont( 96+64+(20*12)+ChampTableOff, 100+(9*i), buf, 128, 128, 128, 128, 128, 128, &FrontEndOrderingTable[Toggle][0], 1 );

        if( CupTable.PlayerOrder[i].NewPoints )
        {
            sprintf(buf, "+%ld", CupTable.PlayerOrder[i].NewPoints );
            DrawFont( 96+64+(23*12)+ChampTableOff, 100+(9*i), buf, 128, 0, 0, 128, 0, 0, &FrontEndOrderingTable[Toggle][0], 1 );
        }

    }
        


}

#endif