//-----------------------------------------------------------------------------
// File: player.cpp
//
// Desc: Player handling code
//
// Re-Volt (Generic) Copyright (c) Probe Entertainment 1998
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"
#ifndef _PSX
#include "main.h"
#endif
#include "geom.h"
#include "particle.h"
#include "model.h"
#include "aerial.h"
#include "newcoll.h"
#include "body.h"
#include "car.h"
#include "ctrlread.h"
#include "object.h"
#include "obj_init.h"
#include "control.h"
#include "player.h"
#include "move.h"
#include "ai.h"
#include "ai_car.h"
#include "ai_init.h"
#include "ui_TitleScreen.h"
#include "ui_MenuText.h"
#include "ui_Menu.h"

#ifdef _PC
#include "panel.h"
#include "gamegauge.h"
#endif
#include "field.h"
#include "camera.h"
#include "timing.h"
#include "pickup.h"
#ifdef _N64
#include "panel.h"
#endif

#include "net_XOnline.h"
#ifdef ENABLE_STATISTICS
//$REVISIT: Statistics disabled for July Consumer Beta
#include "net_Statistics.h"
#endif // ENABLE_STATISTICS

//
// Static variables
// 

PLAYER *s_NextFreePlayer;

//
// Global variables
//

PLAYER  Players[MAX_NUM_PLAYERS];
PLAYER *PLR_PlayerHead = NULL;
PLAYER *PLR_PlayerTail = NULL;
//long  MyPlayerNum;                        // Index into Players array - "who you are" :)
FINISH_ENTRY FinishTable[MAX_NUM_PLAYERS];

#ifdef _PC
PLAYER *PLR_LocalPlayer = NULL;
#else
long    AllHumansFinished;
#endif

long    NumPlayers, AllPlayersFinished;
//
// Global function prototypes
//

void PLR_InitPlayers(void);
PLAYER *PLR_CreatePlayer(PLAYER_TYPE Type, CTRL_TYPE CtrlType, CAR_TYPE CarType, VEC *Pos, MAT *Mat);
void PLR_KillPlayer(PLAYER *Player);
void CreatePlayerForceField(PLAYER *player);
long PLR_CreateStartupPlayers();

//--------------------------------------------------------------------------------------------------------------------------

void PLR_InitPlayers(void)
{
    long    ii;

    for (ii = 0; ii < MAX_NUM_PLAYERS; ii++)
    {
        Players[ii].type = PLAYER_NONE;
        Players[ii].score = 0;
        Players[ii].lastscore = 0;
        Players[ii].raceswon = 0;
        Players[ii].ctrlhandler = NULL;
        Players[ii].conhandler = NULL;
        Players[ii].Slot = ii;
    }

    NumPlayers = 0;
    s_NextFreePlayer = Players;

    Players[0].prev = NULL;
    Players[0].next = &(Players[1]);

    for (ii = 1; ii < (MAX_NUM_PLAYERS - 1); ii++)
    {
        Players[ii].prev = &(Players[ii - 1]);
        Players[ii].next = &(Players[ii + 1]);
    }

    Players[MAX_NUM_PLAYERS - 1].prev = &(Players[MAX_NUM_PLAYERS - 2]);
    Players[MAX_NUM_PLAYERS - 1].next = NULL;

    PLR_PlayerHead = NULL;
    PLR_PlayerTail = NULL;
}

//--------------------------------------------------------------------------------------------------------------------------

PLAYER *PLR_CreatePlayer(PLAYER_TYPE Type, CTRL_TYPE CtrlType, CAR_TYPE CarType, VEC *Pos, MAT *Mat)
{
    PLAYER  *newplayer;
    OBJECT  *newobj;

    // Make sure there are some spare player slots (huh huh....I think I'm gay...!)
    if (s_NextFreePlayer == NULL) {
        return NULL;
    }
#ifdef _PC
    Assert(NumPlayers <= MAX_NUM_PLAYERS);
#endif

    // Allocate the object space
    newobj = OBJ_AllocObject();
    if (newobj == NULL)
    {
        return(NULL);                                   // Could not allocate object for player
    }

    // Make object top of list (because it allows body-to-body collisions
    MoveObjectToHead(newobj);

    // Get next empty player
    newplayer = s_NextFreePlayer;                       

    s_NextFreePlayer = s_NextFreePlayer->next;
    if (s_NextFreePlayer != NULL)
    {
        s_NextFreePlayer->prev = NULL;
    }

    newplayer->prev = PLR_PlayerTail;

    if (PLR_PlayerHead == NULL)
    {
        PLR_PlayerHead = newplayer;
    }
    else
    {
        PLR_PlayerTail->next = newplayer;
    }
    PLR_PlayerTail = newplayer;
    
    // Initialise data
    newplayer->next = NULL;

    newplayer->ownobj = newobj;
    newplayer->ownobj->player = newplayer;
    newplayer->car.Body = &newplayer->ownobj->body;
    newplayer->type = Type;
    newplayer->CarAI.WrongWay = FALSE;
#ifdef _N64
    newplayer->CarAI.WrongWayFlag = FALSE;
    newplayer->CarAI.WrongWayTimer = 0;
#endif

    newplayer->ctrltype = CtrlType;                         // Set up controller used by player
    newplayer->ctrlhandler = NULL;
    newplayer->conhandler = NULL;
#ifdef _PC
    newplayer->Spectator = FALSE;
#endif
    newplayer->car.RenderFlag = CAR_RENDER_NORMAL;

    newplayer->ownobj->flag.Draw = FALSE;
    newplayer->ownobj->flag.Move = TRUE;
    newplayer->ownobj->renderhandler = NULL;
    newplayer->ownobj->freehandler = NULL;
    newplayer->ownobj->Type = OBJECT_TYPE_CAR;
    newplayer->ownobj->Field = NULL;
    newplayer->ownobj->replayhandler = NULL;

    newplayer->ownobj->FieldPriority = FIELD_PRIORITY_MID;

#ifdef _PC
    newplayer->ownobj->remotehandler = NULL;
#endif

#ifndef _PSX
    CRD_InitPlayerControl(newplayer, CtrlType);
#endif

    PlayerTargetOff(newplayer);
    newplayer->PickupCycleSpeed = 0;
    newplayer->PickupNum = 0;
    newplayer->PickupType = PICKUP_TYPE_NONE;
    newplayer->PickupCycleType = 0;

    newplayer->ValidRailCamNode = -1;
    newplayer->LastValidRailCamNode = -1;
    newplayer->RaceFinishTime = 0;
    newplayer->DisplaySplitCount = 0;
    newplayer->TrackDirCount = 0;
    newplayer->BombTagTimer = BOMBTAG_MAX_TIME;
#ifdef _PC
//$REMOVED_DEBUGONLY    newplayer->LastPing = 0;
    newplayer->CarPacketCount = 0;
#endif

	newplayer->pNameTexture = NULL;

    InitCar(&newplayer->car);
    SetupCar(newplayer, CarType);
    SetCarPos(&newplayer->car, Pos, Mat);

    PLR_SetPlayerType(newplayer, Type);

    AI_InitPlayerAI(newplayer);
//  CAI_InitCarAI(newplayer, CAI_SK_RACER);
//  CAI_InitCarAI(newplayer, CarType);
    CAI_InitCarAI(newplayer, newplayer->car.CarType);

    //$SINGLEPLAYER - assuming single local player at Players[0]
    newplayer->dwOnlineState = Players[0].dwOnlineState;

    NumPlayers++;
    return(newplayer);              // Success
}

//--------------------------------------------------------------------------------------------------------------------------

void PLR_SetPlayerType(PLAYER *newplayer, PLAYER_TYPE type)
{
    newplayer->type = type;
    newplayer->car.RenderFlag = CAR_RENDER_NORMAL;

    switch (type) 
    {
    case PLAYER_NONE:
        newplayer->car.RenderFlag = CAR_RENDER_OFF;
        newplayer->conhandler = NULL;
        newplayer->ownobj->aihandler = NULL;
        newplayer->ownobj->CollType = NULL;
        newplayer->ownobj->movehandler = NULL;
        newplayer->ownobj->collhandler = NULL;
        newplayer->ownobj->replayhandler = NULL;
#ifdef _PC
        newplayer->ownobj->remotehandler = NULL;
#endif
        break;

    case PLAYER_LOCAL:
        newplayer->conhandler = (CON_HANDLER)CON_LocalCarControl;
        newplayer->ownobj->aihandler = (AI_HANDLER)AI_LocalAiHandler;
        newplayer->ownobj->CollType = COLL_TYPE_CAR;
        newplayer->ownobj->movehandler = (MOVE_HANDLER)MOV_MoveCarNew;
        newplayer->ownobj->collhandler = (COLL_HANDLER)COL_CarCollHandler;
        newplayer->ownobj->replayhandler = (REPLAY_HANDLER)PlayerReplayStoreHandler;
#ifdef _PC
        newplayer->ownobj->remotehandler = (REMOTE_HANDLER)SendCarData;
#endif // _PC
        break;

    case PLAYER_CPU:
        newplayer->conhandler = (CON_HANDLER)CON_LocalCarControl;
        newplayer->ownobj->aihandler = (AI_HANDLER)AI_CarAiHandler; //AI_LocalAiHandler;
        newplayer->ownobj->CollType = COLL_TYPE_CAR;
        newplayer->ownobj->movehandler = (MOVE_HANDLER)MOV_MoveCarNew;
        newplayer->ownobj->collhandler = (COLL_HANDLER)COL_CarCollHandler;
        newplayer->ownobj->replayhandler = (REPLAY_HANDLER)PlayerReplayStoreHandler;
#ifdef _PC
        newplayer->ownobj->remotehandler = (REMOTE_HANDLER)SendCarData;
#endif
        break;

    case PLAYER_GHOST:
#ifdef _PC
        newplayer->car.RenderFlag = GAME_GAUGE ? CAR_RENDER_NORMAL : CAR_RENDER_GHOST;
#endif
        newplayer->ownobj->CollType = COLL_TYPE_NONE;

#ifndef _PSX
        newplayer->ownobj->movehandler = (MOVE_HANDLER)MOV_MoveGhost;
#endif
        newplayer->ownobj->collhandler = NULL;
        newplayer->ownobj->aihandler = (AI_HANDLER)AI_GhostCarAiHandler;
        break;

#ifdef _PC
    case PLAYER_REMOTE:
        CRD_InitPlayerControl(newplayer, CTRL_TYPE_REMOTE);
        newplayer->conhandler = (CON_HANDLER)CON_LocalCarControl;
        newplayer->ownobj->CollType = COLL_TYPE_CAR;
        newplayer->ownobj->movehandler = (MOVE_HANDLER)MOV_MoveCarNew;
        newplayer->ownobj->collhandler = (COLL_HANDLER)COL_CarCollHandler;
        newplayer->ownobj->aihandler = (AI_HANDLER)AI_RemoteAiHandler;
        newplayer->ownobj->replayhandler = (REPLAY_HANDLER)PlayerReplayStoreHandler;
        break;
#endif

    case PLAYER_REPLAY:
#ifndef _PSX
        CRD_InitPlayerControl(newplayer, CTRL_TYPE_REPLAY);
#endif
        newplayer->conhandler = (CON_HANDLER)CON_LocalCarControl;
        newplayer->ownobj->aihandler = (AI_HANDLER)AI_ReplayAiHandler;
        newplayer->ownobj->CollType = COLL_TYPE_CAR;
        newplayer->ownobj->movehandler = (MOVE_HANDLER)MOV_MoveCarNew;
        newplayer->ownobj->collhandler = (COLL_HANDLER)COL_CarCollHandler;
        newplayer->ownobj->replayhandler = NULL;
        break;

    case PLAYER_FRONTEND:
        newplayer->conhandler = (CON_HANDLER)CON_LocalCarControl;
        newplayer->ownobj->aihandler = (AI_HANDLER)AI_LocalAiHandler;//(AI_HANDLER)AI_CarAiHandler;
        newplayer->ownobj->CollType = COLL_TYPE_CAR;
        newplayer->ownobj->movehandler = (MOVE_HANDLER)MOV_MoveCarNew;
        newplayer->ownobj->collhandler = (COLL_HANDLER)COL_CarCollHandler;
        newplayer->DinkyTimer = Real(0);
        break;

    case PLAYER_DISPLAY:
        newplayer->conhandler = (CON_HANDLER)CON_LocalCarControl;
        newplayer->ownobj->aihandler = NULL;//(AI_HANDLER)AI_CarAiHandler;
        newplayer->ownobj->CollType = COLL_TYPE_CAR;
        newplayer->ownobj->movehandler = (MOVE_HANDLER)MOV_DropCar;
        newplayer->ownobj->collhandler = (COLL_HANDLER)COL_CarCollHandler;
        newplayer->DinkyTimer = Real(0);
        break;

    case PLAYER_CALCSTATS:
        newplayer->conhandler = (CON_HANDLER)CON_LocalCarControl;
        newplayer->ownobj->aihandler = (AI_HANDLER)AI_CalcStatsAiHandler;
        newplayer->ownobj->CollType = COLL_TYPE_CAR;
        newplayer->ownobj->movehandler = (MOVE_HANDLER)MOV_MoveCarNew;
        newplayer->ownobj->collhandler = (COLL_HANDLER)COL_CarCollHandler;
        newplayer->ownobj->replayhandler = (REPLAY_HANDLER)PlayerReplayStoreHandler;
        break;

    default:
        newplayer->ownobj->CollType = COLL_TYPE_NONE;
        newplayer->ownobj->movehandler = NULL;
        newplayer->ownobj->collhandler = NULL;
        newplayer->ownobj->aihandler = NULL;
    }

    // Special UFO handlers
#ifdef _PC
    if (newplayer->car.CarType == CARID_UFO) {
        newplayer->ownobj->movehandler = (MOVE_HANDLER)MOV_MoveUFO;
        newplayer->ownobj->body.AngResMod = 40;
    }
#endif

    newplayer->ownobj->defaultmovehandler = newplayer->ownobj->movehandler;
    newplayer->ownobj->defaultcollhandler = newplayer->ownobj->collhandler;
}

//--------------------------------------------------------------------------------------------------------------------------

void PLR_KillPlayer(PLAYER *Player)
{
    // Make sure there are no camera associated with this object
    DetachCamerasFromObject(Player->ownobj);

    // Release the texture for the player's name
    if( Player->pNameTexture )
        Player->pNameTexture->Release();

    // Free the car's RAM
    FreeCar(Player);

    // And the object
    OBJ_FreeObject(Player->ownobj);

    // Remove from active player list
    if (Player->prev != NULL)
    {                        
        (Player->prev)->next = Player->next;
    }
    else
    {
        PLR_PlayerHead = Player->next;
    }

    if (Player->next != NULL)
    {
        (Player->next)->prev = Player->prev;
    }
    else 
    {
        PLR_PlayerTail = Player->prev;
    }


    if (s_NextFreePlayer != NULL)
    {
        s_NextFreePlayer->prev = Player;
    }

    Player->next = s_NextFreePlayer;
    Player->prev = NULL;
    s_NextFreePlayer = Player;
    

    // Reinitialise
    Player->type = PLAYER_NONE;
    Player->ownobj = NULL;

    // Keep count
    NumPlayers--;
}

//--------------------------------------------------------------------------------------------------------------------------

void PLR_KillAllPlayers(void)
{
    PLAYER *player, *next;

    for (player = PLR_PlayerHead ; player ; )
    {
        next = player->next;
        PLR_KillPlayer(player);
        player = next;
    }

}


/////////////////////////////////////////////////////////////////////
//
// CreatePlayerForceField:
//
/////////////////////////////////////////////////////////////////////

void CreatePlayerForceField(PLAYER *player) 
{
    BBOX bBox;
    VEC size;

    SetBBox(&bBox, -1.73f * CAR_RADIUS, 1.73f * CAR_RADIUS, -1.73f * CAR_RADIUS, 1.73f * CAR_RADIUS, -1.73f * CAR_RADIUS, 1.73f * CAR_RADIUS);
    SetVec(&size, CAR_RADIUS, CAR_RADIUS, CAR_RADIUS);

    player->ownobj->Field = AddLocalField(
        player->ownobj->ObjID,
        FIELD_PRIORITY_MAX, 
        &player->car.Body->Centre.Pos,
        &player->car.Body->Centre.WMatrix,
        &bBox, 
        &size,
        &player->car.FieldVec,
        ONE,
        ZERO);
}

///////////////////////////////////
// get a player from a player ID //
///////////////////////////////////
#ifdef _PC

PLAYER *GetPlayerFromPlayerID(unsigned long id)
{
    PLAYER *player;

// find player

    for (player = PLR_PlayerHead ; player ; player = player->next)
    {
        if (player->PlayerID == id)
            return player;
    }

// not found

    return NULL;
}
#endif

/////////////////////////
// set player finished //
/////////////////////////

void SetPlayerFinished(PLAYER *player, unsigned long time)
{
    long i, j, k;

// set players finish time

    player->RaceFinishTime = time;

// add to finish table

    for (i = 0 ; i < MAX_NUM_PLAYERS ; i++)
    {
        if (!FinishTable[i].Time || time < FinishTable[i].Time)
        {
            for (j = MAX_NUM_PLAYERS - 1 ; j > i ; j--)
            {
                FinishTable[j] = FinishTable[j - 1];
                if (FinishTable[j].Time)
                    FinishTable[j].Player->RaceFinishPos = j;
            }

            FinishTable[i].Player = player;
            FinishTable[i].Time = time;
            player->RaceFinishPos = i;

            break;
        }
    }

    // $STATISTICS: no stats for single player online game
    // $MD: duh!  please don't mess with I if it is being used below.
    if ( NumPlayers > 1)
    {
        // How many players have finished?
        long lNumFinished = 0;
        for (k = 0 ; k < MAX_NUM_PLAYERS ; k++)
        {
            if (FinishTable[k].Time)
                lNumFinished++;
        }

#ifdef ENABLE_STATISTICS
//$REVISIT: Statistics disabled for July Consumer Beta
        // $STATISTICS: When all players have finished, update stats
        if ( lNumFinished == NumPlayers &&
             gTitleScreenVars.bUseXOnline )
        {
            StatUpdateEndgamePlayerStats();
        }
#endif // ENABLE_STATISTICS
    }


// set console message
    WCHAR buf1[32], buf2[32];
    swprintf(buf1, TEXT_TABLE(TEXT_PLAYER_CAME_IN_NTH_PLACE), TEXT_TABLE(TEXT_1ST+i), player->PlayerName);
    swprintf(buf2, TEXT_TABLE(TEXT_PLAYERS_FINISH_TIME), MINUTES(time), SECONDS(time), THOUSANDTHS(time));
    SetConsoleMessage(buf1, buf2, MultiPlayerColours[player->Slot % MAX_RACE_CARS], MENU_COLOR_WHITE, 1, CONSOLE_MESSAGE_DEFAULT_TIME);

// set camera to rotate mode

    if (player->type == PLAYER_LOCAL)
    {
#if defined(_PC)
        SetCameraFollow(CAM_MainCamera, player->ownobj, CAM_FOLLOW_ROTATE);
#elif defined(_N64)
        SetCameraFollow(CAM_PlayerCameras[player->Slot], player->ownobj, CAM_FOLLOW_ROTATE);
#endif

        if( gTitleScreenVars.bUseXOnline )
        {
            // $SINGLEPLAYER : for multiplplayer, need to detect when all local players 
            // $SINGLEPLAYER : are done to update all local stats at once
            // $STATISTICS: rating system needs position info on all players in race, must wait until others are done.
            //StatUpdateEndgamePlayerStats();
        }
    }
}

/////////////////////////////////////////////////////////////////////
//
// SetAllPlayersToDrop:
//
/////////////////////////////////////////////////////////////////////

void SetAllPlayerHandlersToDrop()
{
    PLAYER *player;

    for (player = PLR_PlayerHead; player != NULL; player = player->next) {

        if (player->type == PLAYER_GHOST) continue;

        player->ownobj->movehandler = (MOVE_HANDLER)MOV_DropCar;
    }
}


/////////////////////////////////////////////////////////////////////
//
// ResetAllPlayerHandlersToDefault:
//
/////////////////////////////////////////////////////////////////////

void ResetAllPlayerHandlersToDefault()
{
    PLAYER *player;

    for (player = PLR_PlayerHead; player != NULL; player = player->next) {

        player->ownobj->movehandler = (MOVE_HANDLER)player->ownobj->defaultmovehandler;
        player->ownobj->collhandler = (COLL_HANDLER)player->ownobj->defaultcollhandler;
    }
}


