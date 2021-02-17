//$REVISIT: should this file be named "InitGame" instead?
//-----------------------------------------------------------------------------
// File: InitPlay.cpp
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"
#include "InitPlay.h"
#ifdef _PC
#include "network.h"
#endif
#include "main.h"
#include "LevelLoad.h"
#include "ui_TitleScreen.h"
#include "credits.h"
// $BEGIN_TEMPORARY(jedl) - set initial position and orientation to where we were last time
#include "settings.h"
// $END_TEMPORARY



START_DATA StartData, StartDataStorage;

#ifdef _PC
START_DATA MultiStartData;
#endif

////////////////////////////////////////////////////////////////
//
// Prototypes
void InitStartData();
void InitStartingPlayers();
PLAYER *InitOneStartingPlayer(PLAYER_START_DATA *startData);
void RandomizeStartingGrid();


////////////////////////////////////////////////////////////////
//
// InitStartData:
//
////////////////////////////////////////////////////////////////

void InitStartData()
{
#ifdef _PC
    StartData.LevelDir[0] = '\0';
#endif
    StartData.PlayerNum = 0;
    StartData.LocalPlayerNum = 0;
    StartData.LocalNum = 0;
}

////////////////////////////////////////////////////////////////
//
// AddPlayerToStartData:
//
////////////////////////////////////////////////////////////////

#ifdef _PC
extern bool AddPlayerToStartData(PLAYER_TYPE playerType, long grid, long carType, long spectator, unsigned long time, CTRL_TYPE ctrlType, DPID id, char *name)
{
#else
extern bool AddPlayerToStartData(PLAYER_TYPE playerType, long grid, long carType, long spectator, unsigned long time, CTRL_TYPE ctrlType, long unUsed, char *name)
{
    long id = 0;
#endif
    // not if full
    if (StartData.PlayerNum >= MAX_NUM_PLAYERS)
        return FALSE;

    // Set the data
    SetStartingPlayerData(StartData.PlayerNum, playerType, grid, carType, spectator, time, ctrlType, id, name);

    // Increase player count
    StartData.PlayerNum++;
    return TRUE;
}

////////////////////////////////////////////////////////////////
//
// SetStartingPlayerData:
//
////////////////////////////////////////////////////////////////
#ifdef _PC
void SetStartingPlayerData(long playerNum, PLAYER_TYPE playerType, long grid, long carType, long spectator, unsigned long time, CTRL_TYPE ctrlType, DPID id, char *name)
#else
void SetStartingPlayerData(long playerNum, PLAYER_TYPE playerType, long grid, long carType, long spectator, unsigned long time, CTRL_TYPE ctrlType, long id, char *name)
#endif
{
    StartData.PlayerData[StartData.PlayerNum].PlayerType = playerType;
    StartData.PlayerData[StartData.PlayerNum].GridNum = grid;
    StartData.PlayerData[StartData.PlayerNum].CarType = carType;
    StartData.PlayerData[StartData.PlayerNum].StartTime = time;
    StartData.PlayerData[StartData.PlayerNum].CtrlType = ctrlType;
#ifdef _PC
    StartData.PlayerData[StartData.PlayerNum].Spectator = spectator;
    StartData.PlayerData[StartData.PlayerNum].PlayerID = id;
#endif
    strncpy(StartData.PlayerData[StartData.PlayerNum].Name, name, MAX_PLAYER_NAME);
    StartData.PlayerData[StartData.PlayerNum].Name[MAX_PLAYER_NAME-1] = 0;

    if (playerType == PLAYER_LOCAL) {
        StartData.LocalNum++;
    }
}


////////////////////////////////////////////////////////////////
//
// InitStartingPlayers:
//
////////////////////////////////////////////////////////////////

void InitStartingPlayers()
{
    int iPlayer;
    PLAYER *player;

#ifdef _PSX
    VEC pos;
#endif

#ifdef _PC
    PLR_LocalPlayer = NULL;
#endif
    
    // create and initialise all the players
    for (iPlayer = 0; iPlayer < StartData.PlayerNum; iPlayer++) {

        player = InitOneStartingPlayer(&StartData.PlayerData[iPlayer]);
#ifdef _PC      
        if (iPlayer == StartData.LocalPlayerNum) {
            PLR_LocalPlayer = player;
			
			// $BEGIN_TEMPORARY(jedl) - set initial position and orientation to where we were last time
	  	  	if (RegistrySettings.PositionSave)
	  	  	{
	 	 		CHAR buf[_MAX_PATH];
	 	 		sprintf(buf, "%s\\SavedPosition.txt", CurrentLevelInfo.szDir);
	 	 		FILE *fp = fopen(buf, "rb");
	 	 		if (fp != NULL)
	 	 		{
	 	 			// Read position and orientation from file
				    VEC pos;
					MAT mat;
 	 	 			for (int j = 0; j < 3; j++)
	 	 				if (fscanf(fp, "%g", &pos.v[j]) != 1)
							goto cleanup;	// bad read, skip setting position
	 	 			for (int i = 0; i < 3; i++)
	 	 				for (int j = 0; j < 3; j++)
	 	 					if (fscanf(fp, "%g", &mat.m[i * 3 + j]) != 1)
								goto cleanup;	// bad read, skip setting orientation
					SetCarPos(&PLR_LocalPlayer->car, &pos, &mat);
				cleanup:
	 	 			fclose(fp);
	 	 		}
			}
			// $END_TEMPORARY(jedl)
        }

        IncLoadThreadUnitCount();
#endif
    }


}


////////////////////////////////////////////////////////////////
//
// InitOneStartingPlayer:
//
////////////////////////////////////////////////////////////////

PLAYER *InitOneStartingPlayer(PLAYER_START_DATA *startData)
{

//#ifndef _PSX
    VEC pos;
    MAT mat;
    PLAYER *player;
    // get grid pos
    GetCarStartGrid(startData->GridNum, &pos, &mat);

        
    // create player
    player = PLR_CreatePlayer(startData->PlayerType, startData->CtrlType, startData->CarType, &pos, &mat);

    
    // make sure that was okay
    if (player == NULL) {
#ifdef _PC
        char buf[256];
        sprintf(buf, "Can't create player %s", startData->Name);
        DumpMessage(NULL, buf);
        QuitGame();
        return NULL;
#endif
#ifdef _N64
        ERROR("IPL", "InitOneStartingPlayer", "Failed to create player", 1);
#endif
    }

    // ID, spectator
#ifdef _PC
    player->PlayerID = startData->PlayerID;
    if (startData->Spectator) {
        player->Spectator = TRUE;
        player->car.RenderFlag = CAR_RENDER_GHOST;
        player->ownobj->body.CollSkin.AllowObjColls = FALSE;
    }

    // set not ready
    player->Ready = FALSE;
#endif

    // Name
    strncpy(player->PlayerName, startData->Name, MAX_PLAYER_NAME);
    player->PlayerName[MAX_PLAYER_NAME-1] = 0;

    return player;
//#endif
}


////////////////////////////////////////////////////////////////
//
// Randomize starting grid
//
////////////////////////////////////////////////////////////////

void RandomizeStartingGrid()
{
    int iPlayer, iGrid, playerGrid;
    int *gridsLeft;
    int nGridsLeft;

    // put local player at back, init remaining positions
    for (iPlayer = 0 ; iPlayer < StartData.LocalNum ; iPlayer++)
    {
        StartData.PlayerData[iPlayer].GridNum = StartData.PlayerNum - iPlayer - 1;
    }

    nGridsLeft = StartData.PlayerNum - StartData.LocalNum;

#ifdef _PSX  // Just to fix around my malloc exception handler if we end up mallocing 0 bytes -Stef
    if( !nGridsLeft )
        return;

#endif

    gridsLeft = (int *)malloc(sizeof(int) * nGridsLeft);
    if (gridsLeft == NULL) return;

    for (iGrid = 0; iGrid < nGridsLeft; iGrid++) {
        gridsLeft[iGrid] = iGrid;
    }

    // Choose a grid location for each player
    for (iPlayer = StartData.LocalNum ; iPlayer < StartData.PlayerNum ; iPlayer++) {

        // Get random grid slot
        playerGrid = rand() % nGridsLeft;
        StartData.PlayerData[iPlayer].GridNum = gridsLeft[playerGrid];

        // Shift grid slot list down
        for (iGrid = playerGrid; iGrid < nGridsLeft - 1; iGrid++) {
            gridsLeft[iGrid] = gridsLeft[iGrid + 1];
        }

        // Reduce number of available grid spaces
        nGridsLeft--;
    }

    free(gridsLeft);
}


////////////////////////////////////////////////////////////////
//
// RandomizeCPUCarType: select random cars for the CPU players
// with a max CARID of topCar (calculated from class of players
// car). If there are not  enough cars of same or lower class,
// select one randomly from the higher classes
//
////////////////////////////////////////////////////////////////

void RandomizeCPUCarType()
{
    int nCPU, iCar, topCar, nSelectable, idSlot, iPlayer;
    int *carList;
    bool carAllowed;

    // special case if credits
    if (g_CreditVars.State != CREDIT_STATE_INACTIVE) {
        for (iPlayer = 0; iPlayer < StartData.PlayerNum; iPlayer++) {
            do {
                carAllowed = TRUE;
                idSlot = rand() % (CARID_AMW + 1);
                for (iCar = 0; iCar < iPlayer; iCar++) {
                    if (StartData.PlayerData[iCar].CarType == idSlot) {
                        carAllowed = FALSE;
                        break;
                    }
                }
                StartData.PlayerData[iPlayer].CarType = idSlot;
            } while(!carAllowed);
        }
        return;
    }


    // Find the highest car number allowed for the CPU
    topCar = 8;

    // Init car list
    carList = (int *)malloc(sizeof(int) * NCarTypes);
    nCPU = topCar - gTitleScreenVars.numberOfPlayers;
    nSelectable = 0;

    // Build a list of the available cars
    for (iCar = 0; iCar < NCarTypes; iCar++) {
        carAllowed = TRUE;

        // Cars already selected for a player not allowed
        if (GameSettings.GameType != GAMETYPE_DEMO) {
            for (iPlayer = 0; iPlayer < gTitleScreenVars.numberOfPlayers; iPlayer++) {
                if (StartData.PlayerData[iPlayer].CarType == iCar) {
                    carAllowed = FALSE;
                }
            }
        }

        // Cannot have trolley, key cars, or UFO
#ifndef _N64
        if ((iCar == CARID_TROLLEY) || (iCar == CARID_KEY1) || (iCar == CARID_KEY2) || (iCar == CARID_KEY3) || (iCar == CARID_KEY4) || (iCar == CARID_ROTOR) || (iCar == CARID_PANGA) || (iCar == CARID_UFO)) {
#else
        if ((iCar == CARID_TROLLEY) || (iCar == CARID_KEY1) || (iCar == CARID_KEY2) || (iCar == CARID_KEY3) || (iCar == CARID_KEY4) || (iCar == CARID_ROTOR) || (iCar == CARID_PANGA)) {
#endif
            carAllowed = FALSE;
        }

        // Set car selectable
        if (carAllowed) {
            carList[nSelectable++] = iCar;
        }
    }


    // Choose cars randomly for the CPU players
    for (iPlayer = 0; iPlayer < StartData.PlayerNum; iPlayer++) {
        if (StartData.PlayerData[iPlayer].PlayerType != PLAYER_LOCAL) {

            // Choose from allowed list
            idSlot = rand() % nCPU;
            StartData.PlayerData[iPlayer].CarType = carList[idSlot];
            strncpy(StartData.PlayerData[iPlayer].Name, CarInfo[carList[idSlot]].Name, MAX_PLAYER_NAME);
            StartData.PlayerData[iPlayer].Name[MAX_PLAYER_NAME - 1] = '\0';

            
            // Shift allowed list down
            for (iCar = idSlot; iCar < nSelectable - 1; iCar++) {
                carList[iCar] = carList[iCar + 1];
            }

            // 
            nCPU--;
            nSelectable--;
            if (nCPU == 0) nCPU = nSelectable;
        }
    }

    free(carList);
}

////////////////////////////////////////
// randomize CPU cars for single race //
////////////////////////////////////////

void RandomizeSingleRaceCars(void)
{
#ifdef _PSX
    static long carnum[RACE_CLASS_NTYPES];
    static long picktable[RACE_CLASS_NTYPES];
    static long classid[RACE_CLASS_NTYPES][CARID_NTYPES];
    static long carpicked[MAX_RACE_CARS];
#else
    long carnum[RACE_CLASS_NTYPES];
    long picktable[RACE_CLASS_NTYPES];
    long classid[RACE_CLASS_NTYPES][CARID_NTYPES];
    long carpicked[MAX_RACE_CARS];
#endif
    long i, j, k, numpicked, car;
    
// init tables

    for (i = 0 ; i < RACE_CLASS_NTYPES ; i++)
    {
        carnum[i] = 0;
        picktable[i] = 0;
    }

// count car num for each class

    for (i = 0 ; i < NCarTypes ; i++)
    {
        if (i == gTitleScreenVars.PlayerData[0].iCarNum)
            continue;

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

// setup pick table

    j = StartData.PlayerNum - 1;

    for (i = CarInfo[gTitleScreenVars.PlayerData[0].iCarNum].Rating ; i > -1 ; i--)
    {
        picktable[i] = carnum[i];

        j -= carnum[i];
        if (j <= 0)
        {
            picktable[i] += j;
            break;
        }
    }

// loop thru each class

    numpicked = 0;

    for (i = 0 ; i < RACE_CLASS_NTYPES ; i++)
    {

// pick correct amount of cars for said class

        for (j = 0 ; j < picktable[i] ; j++)
        {
            car = classid[i][rand() % carnum[i]];

            for (k = 0 ; k < numpicked ; k++)
                if (car == carpicked[k])
                    break;

            if (k == numpicked)
                carpicked[numpicked++] = car;
            else
                j--;
        }
    }

// any left to do?

    if (numpicked < StartData.PlayerNum - 1)
    {
        for (i = 0 ; i < StartData.PlayerNum - 1 - numpicked ; i++)
        {
            carpicked[numpicked + i] = carpicked[i];
        }
        numpicked += i;
    }

// set car ID's
#ifndef _PC
    for (i = gTitleScreenVars.numberOfPlayers ; i < StartData.PlayerNum; i++)
    {
        StartData.PlayerData[i].CarType = carpicked[i - 1];
        strncpy(StartData.PlayerData[i].Name, CarInfo[StartData.PlayerData[i].CarType].Name, MAX_PLAYER_NAME);
        StartData.PlayerData[i].Name[MAX_PLAYER_NAME - 1] = '\0';
    }
#else
    for (i = 1 ; i < StartData.PlayerNum; i++)
    {
        StartData.PlayerData[i].CarType = carpicked[i - 1];
        strncpy(StartData.PlayerData[i].Name, CarInfo[StartData.PlayerData[i].CarType].Name, MAX_PLAYER_NAME);
        StartData.PlayerData[i].Name[MAX_PLAYER_NAME - 1] = '\0';
    }
#endif
}

/////////////////////////
// return a random car //
/////////////////////////

long PickRandomCar(void)
{
    long id, count;

// count valid ID's

    id = 0;
    count = 0;

    // $MD why can't this function return a user car?
    do {
        id = NextValidCarType(id);
        count++;
    } while (id && id < CARID_NTYPES);

// pick random car

    id = 0;
    count = rand() % count;

    while (count--)
    {
        id = NextValidCarType(id);
    }

// return picked id

    return id;
}

///////////////////////////
// return a random track //
///////////////////////////

long PickRandomTrack(void)
{
    long i, count;

// count available tracks

    count = 0;

    for (i = 0 ; i < LEVEL_NSHIPPED_LEVELS ; i++)
    {
        if (IsLevelSelectable(i) && IsLevelAvailable(i))
        {
            count++;
        }
    }

// pick random track

    count = (rand() % count) + 1;
    i = -1;

    while (count)
    {
        i++;

        if (IsLevelSelectable(i) && IsLevelAvailable(i))
        {
            count--;
        }
    }

// return track

    return i;
}
