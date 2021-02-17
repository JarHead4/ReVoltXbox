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
#include "network.h"
#include "main.h"
#include "TitleScreen.h"
#include "credits.h"
#include "LevelLoad.h"
#include "Settings.h"
#include "Spark.h"
#include "Text.h"
#include "Timing.h"
#include "Menu2.h"
#include "Competition.h"


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
        wsprintf(buf, "Can't create player %s", startData->Name);
        DumpMessage(NULL, buf);
        g_bQuitGame = TRUE;
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
    if (CreditVars.State != CREDIT_STATE_INACTIVE) {
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
    nCPU = topCar - g_TitleScreenData.numberOfPlayers;
    nSelectable = 0;

    // Build a list of the available cars
    for (iCar = 0; iCar < NCarTypes; iCar++) {
        carAllowed = TRUE;

        // Cars already selected for a player not allowed
        if (GameSettings.GameType != GAMETYPE_DEMO) {
            for (iPlayer = 0; iPlayer < g_TitleScreenData.numberOfPlayers; iPlayer++) {
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
        if (i == g_TitleScreenData.iCarNum[0])
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

    for (i = CarInfo[g_TitleScreenData.iCarNum[0]].Rating ; i > -1 ; i--)
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
    for (i = g_TitleScreenData.numberOfPlayers ; i < StartData.PlayerNum; i++)
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




// clockwork names
#ifdef _PC
char *ClockworkNames[] = {
    "Zed",
    "Def Stef",
    "Doc C",
    "Gaztastic",
    "CT Baynes",
    "Fodge",
    "Eoin",
    "Roland",
    "Matt DC",
    "Tri Si",
    "Goombah",
    "Nutnut",
    "Bliz",
    "Rippance",
    "Shehad",
    "Jimlad",
    "Kev",
    "Big Mike",
    "New York Jez",
    "Traf",
    "J",
    "Eggy",
    "Big Rich",
    "Super Turk",
    "Pab",
    "Twan'Mobile",
    "Berb",
    "Gregm",
    "Fungus",
    "Liddon",
};
#endif




//-----------------------------------------------------------------------------
// Name: SetRaceData()
// Desc: Set Race Data from Titlescreen settings
//-----------------------------------------------------------------------------
void SetRaceData()
{
    int iPlayer;
    long i, j, k, car, gridused[MAX_RACE_CARS];
    char *carname;
    LEVELINFO *levelInfo = GetLevelInfo(g_TitleScreenData.iLevelNum);

    // Set the render settings
#ifdef _PC
    RenderSettings.Env = RegistrySettings.EnvFlag = g_TitleScreenData.shinyness;
    RenderSettings.Light = RegistrySettings.LightFlag = g_TitleScreenData.lights;
    RenderSettings.Instance = RegistrySettings.InstanceFlag = g_TitleScreenData.instances;
    RenderSettings.Mirror = RegistrySettings.MirrorFlag = g_TitleScreenData.reflections;
    RenderSettings.Shadow = RegistrySettings.ShadowFlag = g_TitleScreenData.shadows;
    RenderSettings.Skid = RegistrySettings.SkidFlag = g_TitleScreenData.skidmarks;
    gSparkDensity = g_TitleScreenData.sparkLevel * HALF;
#endif

#ifdef _N64
    RenderSettings.Env      = g_TitleScreenData.shinyness = TRUE;
    RenderSettings.Lighting = TRUE;
#endif


#ifdef _PSX
    SplitScreenMode = g_TitleScreenData.numberOfPlayers;
    SplitScreenType = (g_TitleScreenData.Vertical)? 0: 1;
    Rumble = g_TitleScreenData.Rumble;
#endif

    // Settings for all race modes
    GameSettings.PlayMode = g_TitleScreenData.playMode;
#ifdef _PC
    GameSettings.DrawRearView = g_TitleScreenData.rearview;
    GameSettings.DrawFollowView = FALSE;
#endif
    SetDefaultDifficulty();

    switch (GameSettings.GameType)
    {

///////////
// trial //
///////////

        case GAMETYPE_TRIAL:

            // Init starting data
            InitStartData();
            StartData.GameType = GameSettings.GameType;
            GameSettings.NumberOfLaps = 0;
            
            // Setup the player
            AddPlayerToStartData(PLAYER_LOCAL, MAX_RACE_CARS - 1, g_TitleScreenData.iCarNum[0], 0, 0, CTRL_TYPE_LOCAL, 0, g_TitleScreenData.nameEnter[0]);
        
#ifdef _PSX
            AddPlayerToStartData(PLAYER_GHOST, 0, GhostCarType, 0, 0, CTRL_TYPE_NONE, 0, g_TitleScreenData.nameEnter[0]);
            
#endif
            
            // Set the level
#if defined(_PC)
            levelInfo = GetLevelInfo(g_TitleScreenData.iLevelNum);
            strncpy(StartData.LevelDir, levelInfo->Dir, MAX_LEVEL_DIR_NAME);
#endif

            // Set the level settings
            GameSettings.Level = g_TitleScreenData.iLevelNum;
            GameSettings.Mirrored = g_TitleScreenData.mirror;
            GameSettings.Reversed = g_TitleScreenData.reverse;
            StartData.Laps = GameSettings.NumberOfLaps = g_TitleScreenData.numberOfLaps;
            StartData.AllowPickups = GameSettings.AllowPickups = g_TitleScreenData.pickUps;
#ifdef _PC
            GameSettings.LocalGhost = g_TitleScreenData.LocalGhost;
#endif
            break;

////////////
// single //
////////////

        case GAMETYPE_SINGLE:
#ifdef _PSX
        case GAMETYPE_CLOCKWORK:
#endif

            // Init starting data
            InitStartData();
            StartData.GameType = GameSettings.GameType;
            StartData.Laps = GameSettings.NumberOfLaps;

            #ifdef _N64
            RenderSettings.Env      = g_TitleScreenData.shinyness = g_TitleScreenData.numberOfPlayers < 3;
            RenderSettings.Lighting = g_TitleScreenData.numberOfPlayers < 2;
            RenderSettings.Shadow   = g_TitleScreenData.numberOfPlayers != 4;
            #endif


#if TRUE //ndef _PSX
            if (GameSettings.RandomCars)
            {
                car = PickRandomCar();
            }

            if (GameSettings.GameType == GAMETYPE_SINGLE) {
                for (iPlayer = 0; iPlayer < g_TitleScreenData.numberOfPlayers; iPlayer++)
                {
                    if (!GameSettings.RandomCars) car = g_TitleScreenData.iCarNum[iPlayer];
                    AddPlayerToStartData(PLAYER_LOCAL, iPlayer, car, 0, 0, CTRL_TYPE_LOCAL, 0, g_TitleScreenData.nameEnter[iPlayer]);
                }
            } else {
//$MODIFIED
//                AddPlayerToStartData(PLAYER_LOCAL, iPlayer, CARID_KEY4, 0, 0, CTRL_TYPE_LOCAL, 0, g_TitleScreenData.nameEnter[iPlayer]);
                iPlayer = 0;  //$NOTE(cprince): to avoid "var used before init" error.
                              //$NOTE(cprince): but probably doesn't matter; looks like we'll only execute this switch case when GameType is GAMETYPE_SINGLE.
                AddPlayerToStartData(PLAYER_LOCAL, iPlayer, CARID_KEY4, 0, 0, CTRL_TYPE_LOCAL, 0, g_TitleScreenData.nameEnter[iPlayer]);
//$END_MODIFICATIONS
            }

#ifdef _PSX
            if ((g_TitleScreenData.numberOfPlayers == 1) || (g_TitleScreenData.MultiplayerCPU))
#endif
            {
                if (GameSettings.GameType == GAMETYPE_SINGLE) {
                    for (iPlayer = g_TitleScreenData.numberOfPlayers; iPlayer < g_TitleScreenData.numberOfCars; iPlayer++)
                    {
                        AddPlayerToStartData(PLAYER_CPU, iPlayer, car, 0, 0, CTRL_TYPE_CPU_AI, 0, CarInfo[car].Name);
                    }

                    if (!GameSettings.RandomCars)
                    {   
                        RandomizeSingleRaceCars();
                    }
                } else {
                    for (iPlayer = 1; iPlayer < 4; iPlayer++) {
                        AddPlayerToStartData(PLAYER_CPU, iPlayer, CARID_KEY4, 0, 0, CTRL_TYPE_CPU_AI, 0, CarInfo[CARID_KEY4].Name);
                    }
                }
            }

            RandomizeStartingGrid();
#else       
            // JCC - for profiling and optimisation
            for (iPlayer = 0; iPlayer < g_TitleScreenData.numberOfPlayers; iPlayer++) {
                AddPlayerToStartData(PLAYER_LOCAL, iPlayer, 3, 0, 0, CTRL_TYPE_LOCAL, 0, g_TitleScreenData.nameEnter[iPlayer]);
            }

            for (iPlayer = g_TitleScreenData.numberOfPlayers; iPlayer < g_TitleScreenData.numberOfCars; iPlayer++) {
                AddPlayerToStartData(PLAYER_CPU, iPlayer, 3, 0, 0, CTRL_TYPE_CPU_AI, 0, "");
            }
#endif

#ifdef _PSX
            // PSX chav to fudge Clockwork carnage in for the really clever post-turnover
            // changes that we have to make.
            GameSettings.GameType = GAMETYPE_SINGLE;
#endif

            // Set the level settings
            if (GameSettings.RandomTrack)
                GameSettings.Level = PickRandomTrack();
            else
                GameSettings.Level = g_TitleScreenData.iLevelNum;

            GameSettings.Mirrored = g_TitleScreenData.mirror;
            GameSettings.Reversed = g_TitleScreenData.reverse;
            StartData.Laps = GameSettings.NumberOfLaps = g_TitleScreenData.numberOfLaps;
            StartData.AllowPickups = GameSettings.AllowPickups = g_TitleScreenData.pickUps;

            // Set the level
#if defined(_PC)
            levelInfo = GetLevelInfo(GameSettings.Level);
            strncpy(StartData.LevelDir, levelInfo->Dir, MAX_LEVEL_DIR_NAME);
#else
#endif
            break;

///////////////
// clockwork //
///////////////
#ifdef _PC
        case GAMETYPE_CLOCKWORK:
            // Init starting data
            InitStartData();
            StartData.GameType = GameSettings.GameType;
            StartData.Laps = GameSettings.NumberOfLaps;

            AddPlayerToStartData(PLAYER_LOCAL, 0, CARID_KEY4, 0, 0, CTRL_TYPE_LOCAL, 0, g_TitleScreenData.nameEnter[0]);

            for (iPlayer = 1 ; iPlayer < MAX_NUM_PLAYERS ; iPlayer++) {
                AddPlayerToStartData(PLAYER_CPU, iPlayer, CARID_KEY4, 0, 0, CTRL_TYPE_CPU_AI, 0, ClockworkNames[(iPlayer - 1) % 30]);
            }

            RandomizeStartingGrid();

            // Set the level settings
            if (GameSettings.RandomTrack)
                GameSettings.Level = PickRandomTrack();
            else
                GameSettings.Level = g_TitleScreenData.iLevelNum;

            GameSettings.Mirrored = g_TitleScreenData.mirror;
            GameSettings.Reversed = g_TitleScreenData.reverse;
            StartData.Laps = GameSettings.NumberOfLaps = g_TitleScreenData.numberOfLaps;
            GameSettings.AllowPickups = g_TitleScreenData.pickUps;

            // Set the level
            levelInfo = GetLevelInfo(GameSettings.Level);
            strncpy(StartData.LevelDir, levelInfo->Dir, MAX_LEVEL_DIR_NAME);
            break;
#endif

/////////////////
// multiplayer //
/////////////////

#ifndef _PC // CONSOLES
        case GAMETYPE_BATTLE:
            // Init starting data

            #ifdef _N64
            RenderSettings.Env   = g_TitleScreenData.shinyness = FALSE;
            RenderSettings.Lighting = FALSE;
            #endif

            InitStartData();
            StartData.GameType = GameSettings.GameType;
            StartData.Laps = GameSettings.NumberOfLaps;

            for (iPlayer = 0; iPlayer < g_TitleScreenData.numberOfPlayers; iPlayer++) {
                AddPlayerToStartData(PLAYER_LOCAL, iPlayer, g_TitleScreenData.iCarNum[iPlayer], 0, 0, CTRL_TYPE_LOCAL, 0, g_TitleScreenData.nameEnter[iPlayer]);
            }

            //RandomizeStartingGrid();

            // Set the level settings
            GameSettings.Level = g_TitleScreenData.iLevelNum;
            GameSettings.Mirrored = g_TitleScreenData.mirror;
            GameSettings.Reversed = g_TitleScreenData.reverse;
            StartData.Laps = GameSettings.NumberOfLaps = g_TitleScreenData.numberOfLaps;
            GameSettings.AllowPickups = g_TitleScreenData.pickUps;

            break;
#endif

#ifdef _PC  //  PC ONLY
        case GAMETYPE_BATTLE:
        case GAMETYPE_MULTI:

            // force arcade mode
            GameSettings.PlayMode = PLAYMODE_ARCADE;

            // show sync message
//$MODIFIED
//            LoadMipTexture("gfx\\font.bmp", TPAGE_FONT, 256, 256, 0, 1, FALSE);
            LoadMipTexture("D:\\gfx\\font.bmp", TPAGE_FONT, 256, 256, 0, 1, FALSE);
//$END_MODIFICATIONS

            for (i = 0 ; i < 3 ; i++)
            {
//$REMOVED                D3Ddevice->BeginScene();

                ClearBuffers();
                InitRenderStates();
                BeginTextState();
                SET_TPAGE(TPAGE_FONT);
                DrawGameText(256, 232, 0xffffff, "Synchronizing...");

//$REMOVED                D3Ddevice->EndScene();

                FlipBuffers();
            }

            FreeOneTexture(TPAGE_FONT);

            // force instances on
            RenderSettings.Instance = TRUE;

            // server
            if( IsServer() )
            {

                // Set the level settings
                if (GameSettings.RandomTrack)
                    GameSettings.Level = PickRandomTrack();
                else
                    GameSettings.Level = g_TitleScreenData.iLevelNum;

                GameSettings.Mirrored = g_TitleScreenData.mirror;
                GameSettings.Reversed = g_TitleScreenData.reverse;
                GameSettings.NumberOfLaps = g_TitleScreenData.numberOfLaps;
                GameSettings.AllowPickups = g_TitleScreenData.pickUps;

                // setup start data
                InitStartData();

                StartData.Mirrored = GameSettings.Mirrored;
                StartData.Reversed = GameSettings.Reversed;
                StartData.GameType = GameSettings.GameType;
                StartData.Laps = g_TitleScreenData.numberOfLaps;
                StartData.AllowPickups = GameSettings.AllowPickups;

                StartData.Seed = rand();

                // set level name
                levelInfo = GetLevelInfo(GameSettings.Level);
                strncpy(StartData.LevelDir, levelInfo->Dir, MAX_LEVEL_DIR_NAME);

                // set session to 'started'
                bGameStarted = TRUE;
//$REMOVED_NOTREQD                SetSessionDesc(g_TitleScreenData.nameEnter[0], levelInfo->Dir, TRUE, GameSettings.GameType, GameSettings.RandomCars, GameSettings.RandomTrack);

                // setup players
//$REMOVED - server maintains player list and updates clients
//                ListPlayers(NULL);
//$END_REMOVAL

                if (GameSettings.RandomCars)
                    carname = CarInfo[PickRandomCar()].Name;

                for (i = 0 ; i < PlayerCount ; i++)
                    gridused[i] = FALSE;

                for (i = 0 ; i < PlayerCount ; i++)
                {
                    // randomly distribute the N players among the first N "grid" slots.
                    // (First player gets random slot 0..N, 2nd player gets random slot 0..N-1 <skipping used slots>, etc)
                    k = (rand() % (PlayerCount - i)) + 1;  // generates rand number between (1) and (NumPlayersLeftToBeProcessed), inclusive.
                    for (j = 0 ; j < PlayerCount ; j++)    // selects the 'k'th untaken RaceStartIndex
                    {
                        if (!gridused[j])
                            k--;
                        if (!k)
                            break;
                    }

                    gridused[j] = TRUE;

                    if (!GameSettings.RandomCars)
                    {
//$MODIFIED $BUG (tentative!!) - Planning on using CarType instead of CarName eventually..
//                        carname = PlayerList[i].Data.CarName;
                        carname = "RC";
                        //$BUG: we'll switch to use CarType (instead of CarName) eventually
//$END_MODIFICATIONS
                    }

//$REMOVED_DONTCARE                    StartData.PlayerData[StartData.PlayerNum].Cheating = PlayerList[i].Data.Cheating;
//$REMOVED $BUG (tentative!!) - Planning on using CarType instead of CarName eventually..
//                    strncpy(StartData.PlayerData[StartData.PlayerNum].CarName, carname, CAR_NAMELEN);
                    //$BUG: we'll switch to use CarType (instead of CarName) eventually
//$END_REMOVAL


                    if (PlayerList[i].PlayerID == LocalPlayerID) {
//$MODIFIED
//                        AddPlayerToStartData(PLAYER_LOCAL, j, GetCarTypeFromName(carname), PlayerList[i].Spectator, TotalRacePhysicsTime, CTRL_TYPE_LOCAL, PlayerList[i].PlayerID, PlayerList[i].Name);
                        AddPlayerToStartData(PLAYER_LOCAL, j, PlayerList[i].CarType, PlayerList[i].Spectator, TotalRacePhysicsTime, CTRL_TYPE_LOCAL, PlayerList[i].PlayerID, PlayerList[i].Name);
//$END_MODIFICATIONS
                        StartData.LocalPlayerNum = i;
                    } else {
//$MODIFIED
//                        AddPlayerToStartData(PLAYER_REMOTE, j, GetCarTypeFromName(carname), PlayerList[i].Spectator, TotalRacePhysicsTime, CTRL_TYPE_REMOTE, PlayerList[i].PlayerID, PlayerList[i].Name);
                        AddPlayerToStartData(PLAYER_REMOTE, j, PlayerList[i].CarType, PlayerList[i].Spectator, TotalRacePhysicsTime, CTRL_TYPE_REMOTE, PlayerList[i].PlayerID, PlayerList[i].Name);
//$END_MODIFICATIONS
                    }
                }

                // send game started
                SendGameStarted();

                // sync
                RemoteSyncHost();
            }

            // client
            else
            {
                // get start data
                StartData = MultiStartData;

                // set level settings
                GameSettings.NumberOfLaps = StartData.Laps;
                GameSettings.Mirrored = StartData.Mirrored;
                GameSettings.Reversed = StartData.Reversed;
                GameSettings.GameType = StartData.GameType;
                GameSettings.Level = GetLevelNum(StartData.LevelDir);
                GameSettings.AllowPickups = StartData.AllowPickups;

                // setup player info
                for (i = 0; i < StartData.PlayerNum; i++) 
                {
                    if (StartData.PlayerData[i].PlayerID == LocalPlayerID) 
                    {
                        StartData.PlayerData[i].PlayerType = PLAYER_LOCAL;
                        StartData.PlayerData[i].CtrlType = CTRL_TYPE_LOCAL;
                        StartData.LocalPlayerNum = i;
                    } 
                    else
                    {
                        StartData.PlayerData[i].PlayerType = PLAYER_REMOTE;
                        StartData.PlayerData[i].CtrlType = CTRL_TYPE_REMOTE;
                    }

//$MODIFIED $BUG (tentative!!)
//                    StartData.PlayerData[i].CarType = GetCarTypeFromName(StartData.PlayerData[i].CarName);
                    StartData.PlayerData[i].CarType = CARID_RC;
                    //$BUG: we'll switch to use CarType (instead of CarName) eventually
//$END_MODIFICATIONS
                }

                // sync
                RemoteSyncClient();
            }
            break;

        case GAMETYPE_REPLAY:
            FILE *fp;

            fp = fopen("Replay.rpl", "rb");
            if (fp == NULL) {
                SetMenuMessage("Could not open replay file");
            } else {
                if (LoadReplayData(fp)) {

                    GameSettings.GameType = GAMETYPE_REPLAY;
                    g_bTitleScreenRunGame = TRUE;

                } else {
                    SetMenuMessage("Could not read replay data");
                }
                fclose(fp);
            }

            // Set the level settings
            GameSettings.GameType = GAMETYPE_REPLAY;//StartData.GameType;
            GameSettings.Level = GetLevelNum(StartData.LevelDir);
            GameSettings.Mirrored = StartData.Mirrored;
            GameSettings.Reversed = StartData.Reversed;
            GameSettings.NumberOfLaps = StartData.Laps;
            GameSettings.AllowPickups = StartData.AllowPickups;
            
            break;

#endif

////////////////////
// champion chip! //
////////////////////

        case GAMETYPE_CHAMPIONSHIP:

            // Set difficulty
            SetCupDifficulty(g_TitleScreenData.CupType);

            // setup cup table
            InitCupTable();

            // init 1st level
            InitOneCupLevel();
            break;

//////////////
// practice //
//////////////

        case GAMETYPE_PRACTICE:
            InitStartData();
            StartData.GameType = GameSettings.GameType;

            AddPlayerToStartData(PLAYER_LOCAL, 0, g_TitleScreenData.iCarNum[0], 0, 0, CTRL_TYPE_LOCAL, 0, g_TitleScreenData.nameEnter[0]);

            // Set the level settings
            GameSettings.Level = g_TitleScreenData.iLevelNum;
            GameSettings.Mirrored = g_TitleScreenData.mirror;
            GameSettings.Reversed = g_TitleScreenData.reverse;
            StartData.Laps = GameSettings.NumberOfLaps = g_TitleScreenData.numberOfLaps;
            GameSettings.AllowPickups = FALSE;
#ifdef _PC
            // Set the level
            levelInfo = GetLevelInfo(g_TitleScreenData.iLevelNum);
            strncpy(StartData.LevelDir, levelInfo->Dir, MAX_LEVEL_DIR_NAME);
#endif
            break;


//////////////
// training //
//////////////

        case GAMETYPE_TRAINING:
            InitStartData();
            StartData.GameType = GameSettings.GameType;

            AddPlayerToStartData(PLAYER_LOCAL, 0, g_TitleScreenData.iCarNum[0], 0, 0, CTRL_TYPE_LOCAL, 0, g_TitleScreenData.nameEnter[0]);

            // Set the level settings
            GameSettings.Level = g_TitleScreenData.iLevelNum;
            GameSettings.Mirrored = FALSE;
            GameSettings.Reversed = FALSE;
            StartData.Laps = GameSettings.NumberOfLaps = g_TitleScreenData.numberOfLaps;
            GameSettings.AllowPickups = FALSE;
            GameSettings.PlayMode = PLAYMODE_ARCADE;

#ifdef _PC
            // Set the level
            levelInfo = GetLevelInfo(g_TitleScreenData.iLevelNum);
            strncpy(StartData.LevelDir, levelInfo->Dir, MAX_LEVEL_DIR_NAME);
#endif
            break;

/////////////////////
// not working yet //
/////////////////////

        case GAMETYPE_NONE:
        case GAMETYPE_FRONTEND:
        case GAMETYPE_INTRO:
        default:
            return;
    }
}
