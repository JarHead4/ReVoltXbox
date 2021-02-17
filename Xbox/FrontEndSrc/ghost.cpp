//-----------------------------------------------------------------------------
// File: ghost.cpp
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "ReVolt.h"
#include "NewColl.h"
#include "Particle.h"
#include "Body.h"
#include "Wheel.h"
#include "Car.h"
#include "Geom.h"
#include "Main.h"
#include "Ctrlread.h"
#include "Object.h"
#include "Control.h"
#include "Player.h"
#include "LevelLoad.h"
#include "Ghost.h"
#include "main.h"
#include "light.h"
#include "timing.h"
#include "initplay.h"
#include "TitleScreen.h"
//#include "gamegauge.h"

static GHOST_INFO GhostInfoStore1;
static GHOST_DATA GhostDataStore1[GHOST_DATA_MAX];
static GHOST_INFO GhostInfoStore2;
static GHOST_DATA GhostDataStore2[GHOST_DATA_MAX];

#ifdef _PC
GHOST_HEADER GHO_GhostFileHeader;
#endif

GHOST_INFO *GHO_BestGhostInfo = &GhostInfoStore1;
GHOST_DATA *BestGhostData = GhostDataStore1;
long GHO_BestFrame = 0;

GHOST_INFO *GhostInfo = &GhostInfoStore2;
GHOST_DATA *GhostData = GhostDataStore2;

bool    gGotNewGhost = FALSE;
bool    GHO_GhostDataExists = FALSE;  //$CMP_NOTE: this was originally named GHO_GhostExists
bool    GHO_GhostAllowed = FALSE;
PLAYER  *GHO_GhostPlayer = NULL;

bool    GHO_ShowGhost = FALSE;

LIGHT *GhostLight = NULL;

REAL DBG_dt = ZERO;

/////////////////////////////////////////////////////////////////////
//
// InitWhiteweeData:
//
/////////////////////////////////////////////////////////////////////

void InitGhostData(PLAYER *player)
{
    if (!GHO_GhostAllowed) return;

    GhostInfo->CarType = player->car.CarType;
    strncpy(GhostInfo->PlayerName, player->PlayerName, MAX_PLAYER_NAME); 
    GhostInfo->PlayerName[MAX_PLAYER_NAME-1]=0;
    GhostInfo->NFrames = 0;

}

void EndGhostData(PLAYER *player)
{
    GHOST_DATA *data;
    CAR *car = &player->car;
    unsigned long lapTime;

    if (!GHO_GhostAllowed) return;

    // Store final time
    GhostInfo->Time[GHOST_LAP_TIME] = /*(unsigned short)*/player->car.LastLapTime;

    // Store final position
    if (GhostInfo->NFrames >= GHOST_DATA_MAX) {
        return;
    }
    
    data = &GhostData[GhostInfo->NFrames];

    // Calculate lap time in milliseconds
    if (player->CarAI.PreLap)
    {
        return;
    }
    lapTime = MS2TIME(TotalRacePhysicsTime) + TotalRaceStartTime;
    if (lapTime < car->CurrentLapStartTime) {
        return;
    }
    lapTime -= car->CurrentLapStartTime;
    lapTime = TIME2MS(lapTime);

    data->Time = /*(unsigned short)*/lapTime;
    data->PosX = (short)car->Body->Centre.Pos.v[X];
    data->PosY = (short)car->Body->Centre.Pos.v[Y];
    data->PosZ = (short)car->Body->Centre.Pos.v[Z];

    data->Quat.v[VX] = (char)(GHOST_VECTOR_SCALE * car->Body->Centre.Quat.v[VX]);
    data->Quat.v[VY] = (char)(GHOST_VECTOR_SCALE * car->Body->Centre.Quat.v[VY]);
    data->Quat.v[VZ] = (char)(GHOST_VECTOR_SCALE * car->Body->Centre.Quat.v[VZ]);
    data->Quat.v[S] = (char)(GHOST_VECTOR_SCALE * car->Body->Centre.Quat.v[S]);

    data->WheelAngle = (char)(GHOST_WHEEL_SCALE * car->Wheel[FL].TurnAngle);

    GhostInfo->NFrames++;
}

void InitBestGhostData()
{
    VEC pos;
    MAT mat;
    QUATERNION quat;

    // Do nothing if the ghost doesn't exist or is not allowed
    if (!GHO_GhostDataExists || !GHO_GhostAllowed) return;

    GHO_BestFrame = 0;

    // Get the starting position and matrix of the ghost car
    pos.v[X] = BestGhostData[0].PosX;
    pos.v[Y] = BestGhostData[0].PosY;
    pos.v[Z] = BestGhostData[0].PosZ;

    quat.v[VX] = GHOST_VECTOR_INVSCALE * (REAL)(BestGhostData[0].Quat.v[VX]);
    quat.v[VY] = GHOST_VECTOR_INVSCALE * (REAL)(BestGhostData[0].Quat.v[VY]);
    quat.v[VZ] = GHOST_VECTOR_INVSCALE * (REAL)(BestGhostData[0].Quat.v[VZ]);
    quat.v[S] = GHOST_VECTOR_INVSCALE * (REAL)(BestGhostData[0].Quat.v[S]);
    QuatToMat(&quat, &mat);

    // Create the ghost player if necessary
    if (GHO_GhostPlayer == NULL) {
        // Create the player
        GHO_GhostPlayer = PLR_CreatePlayer(PLAYER_GHOST, CTRL_TYPE_NONE, GHO_BestGhostInfo->CarType, &pos, &mat);
        if (GHO_GhostPlayer == NULL) return;
    } else {
        // Change ghost car type if necessary
        if (GHO_GhostPlayer->car.CarType != GHO_BestGhostInfo->CarType) {
            SetupCar(GHO_GhostPlayer, GHO_BestGhostInfo->CarType);
        }
        // Set the starting pos for the ghost car
        SetCarPos(&GHO_GhostPlayer->car, &pos, &mat);
    }

    // Set up the car
    GHO_GhostPlayer->CarAI.BackTracking = FALSE;
    GHO_GhostPlayer->car.CurrentLapTime = 0;
    GHO_GhostPlayer->car.CurrentLapStartTime = TimerCurrent;
    strncpy(GHO_GhostPlayer->PlayerName, GHO_BestGhostInfo->PlayerName, MAX_PLAYER_NAME);
    GHO_GhostPlayer->PlayerName[MAX_PLAYER_NAME-1] = 0;

}


void ClearBestGhostData()
{
    // initialise ghost data
    GHO_BestFrame = 0;
    GHO_BestGhostInfo->Time[GHOST_LAP_TIME] = /*(unsigned short)*/MAKE_TIME(10, 0, 0);
    GHO_BestGhostInfo->CarType = 0;
    GHO_BestGhostInfo->PlayerName[0] = '\0';
    GHO_BestGhostInfo->NFrames = 1;

    BestGhostData[0].Time = /*(unsigned short)*/MAKE_TIME(0, 0, 0);
    BestGhostData[1].Time = /*(unsigned short)*/MAKE_TIME(10, 0, 0);

    BestGhostData[0].WheelAngle = 0;
    BestGhostData[0].WheelPos = 0;

    // Kill the ghost car if necessary
    if (GHO_GhostPlayer != NULL) {
        PLR_KillPlayer(GHO_GhostPlayer);
        GHO_GhostPlayer = NULL;
    }

    GHO_GhostDataExists = FALSE;
}

/////////////////////////////////////////////////////////////////////
//
// SwitchGhostDataStores: use to swap ghost data array pointers
// when a new best time is recorded
//
/////////////////////////////////////////////////////////////////////

void SwitchGhostDataStores()
{
    GHOST_DATA *tmpData;
    GHOST_INFO *tmpInfo;

    if (!GHO_GhostAllowed) return;

    if (GhostInfo->NFrames == 0) return;

    // Not if watching Downloaded ghost
    #ifdef _PC
    if (!GameSettings.LocalGhost) return;
    #endif


    tmpData = GhostData;
    tmpInfo = GhostInfo;
    GhostData = BestGhostData;
    GhostInfo = GHO_BestGhostInfo;
    BestGhostData = tmpData;
    GHO_BestGhostInfo = tmpInfo;
    if (GHO_BestGhostInfo->NFrames < GHOST_DATA_MAX) {
        BestGhostData[GHO_BestGhostInfo->NFrames].Time = GHO_BestGhostInfo->Time[GHOST_LAP_TIME];
    }


    GHO_GhostDataExists = TRUE;
    gGotNewGhost = TRUE;
}

/////////////////////////////////////////////////////////////////////
//
// StoreGhostData:
//
/////////////////////////////////////////////////////////////////////

#define GHOST_POS_DEVIATION     1000.0f
#define GHOST_QUAT_DEVIATION    0.6f

bool StoreGhostData(CAR *car)
{
    GHOST_DATA *data;
    unsigned long lapTime;

    // Calculate lap time in milliseconds
    if (Players[0].CarAI.PreLap)
    {
        return FALSE;
    }
    /*lapTime = MS2TIME(TotalRacePhysicsTime) + TotalRaceStartTime;
    if (lapTime < car->CurrentLapStartTime) {
        return FALSE;
    }
    lapTime -= car->CurrentLapStartTime;
    lapTime = TIME2MS(lapTime);*/
    lapTime = TotalRacePhysicsTime;

    if (!GHO_GhostAllowed) return FALSE;

    // Make sure there is enough space in the data array
    if (GhostInfo->NFrames >= GHOST_DATA_MAX) {
        return FALSE;
    }

    // Make sure enough time has passed
    if (GhostInfo->NFrames > 0) {
        if (lapTime - GhostData[GhostInfo->NFrames - 1].Time < GHOST_MIN_TIMESTEP) {
            return FALSE;
        }
    }

    // Store current car data for later replay
    data = &GhostData[GhostInfo->NFrames];

    data->Time = /*(unsigned short)*/lapTime;
    //CopyVec(&car->Body->Centre.Pos, &data->Pos);
    data->PosX = (short)car->Body->Centre.Pos.v[X];
    data->PosY = (short)car->Body->Centre.Pos.v[Y];
    data->PosZ = (short)car->Body->Centre.Pos.v[Z];

    data->Quat.v[VX] = (char)(GHOST_VECTOR_SCALE * car->Body->Centre.Quat.v[VX]);
    data->Quat.v[VY] = (char)(GHOST_VECTOR_SCALE * car->Body->Centre.Quat.v[VY]);
    data->Quat.v[VZ] = (char)(GHOST_VECTOR_SCALE * car->Body->Centre.Quat.v[VZ]);
    data->Quat.v[S] = (char)(GHOST_VECTOR_SCALE * car->Body->Centre.Quat.v[S]);

    data->WheelAngle = (char)(GHOST_WHEEL_SCALE * car->Wheel[FL].TurnAngle);
    
    //data->WheelPos = 0;
    //for (iWheel = 0; iWheel < CAR_NWHEELS; iWheel++) {
    //  char angle = 2 + (char)((car->Wheel[iWheel].TurnAngle / car->SteerAngle) * 2);
    //  data->WheelPos |= angle << 
    //}

    // See if there was sufficient deviation from the interpolated values obtained from previous stored data
    if (GhostInfo->NFrames > 1) {
        REAL dt;
        VEC intPos, dR, lastPos, nextPos;
        QUATERNION intQuat, lastQuat, nextQuat;
        GHOST_DATA *lastData = &GhostData[GhostInfo->NFrames - 2];
        GHOST_DATA *nextData = &GhostData[GhostInfo->NFrames - 1];
        bool posOkay = FALSE;
        bool quatOkay = FALSE;

        // position
        lastPos.v[X] = lastData->PosX;
        lastPos.v[Y] = lastData->PosY;
        lastPos.v[Z] = lastData->PosZ;
        nextPos.v[X] = nextData->PosX;
        nextPos.v[Y] = nextData->PosY;
        nextPos.v[Z] = nextData->PosZ;

        VecMinusVec(&nextPos, &lastPos, &dR);
        dt = Real(nextData->Time - lastData->Time);
        if (dt > SMALL_REAL) {
            dt = (Real(data->Time) - Real(lastData->Time)) / dt;
        } else {
            dt = ZERO;
        }

        VecPlusScalarVec(&lastPos, dt, &dR, &intPos);

        VecMinusVec(&intPos, &car->Body->Centre.Pos, &dR);
        if (VecDotVec(&dR, &dR) < GHOST_POS_DEVIATION) {
            posOkay = TRUE;
        }

        // quaternion
        lastQuat.v[VX] = GHOST_VECTOR_INVSCALE * (REAL)(lastData->Quat.v[VX]);
        lastQuat.v[VY] = GHOST_VECTOR_INVSCALE * (REAL)(lastData->Quat.v[VY]);
        lastQuat.v[VZ] = GHOST_VECTOR_INVSCALE * (REAL)(lastData->Quat.v[VZ]);
        lastQuat.v[S] = GHOST_VECTOR_INVSCALE * (REAL)(lastData->Quat.v[S]);
        nextQuat.v[VX] = GHOST_VECTOR_INVSCALE * (REAL)(nextData->Quat.v[VX]);
        nextQuat.v[VY] = GHOST_VECTOR_INVSCALE * (REAL)(nextData->Quat.v[VY]);
        nextQuat.v[VZ] = GHOST_VECTOR_INVSCALE * (REAL)(nextData->Quat.v[VZ]);
        nextQuat.v[S] = GHOST_VECTOR_INVSCALE * (REAL)(nextData->Quat.v[S]);
        LerpQuat(&lastQuat, &nextQuat, dt, &intQuat);
        NormalizeQuat(&intQuat);
        if (QuatDotQuat(&intQuat, &car->Body->Centre.Quat) > GHOST_QUAT_DEVIATION) {
            quatOkay = TRUE;
        }

    
        if (posOkay && quatOkay) return FALSE;

    }

    GhostInfo->NFrames++;

    return TRUE;

}


/////////////////////////////////////////////////////////////////////
//
// InterpGhostData:
//
/////////////////////////////////////////////////////////////////////

void InterpGhostData(CAR *car)
{
    REAL dt;
    VEC dR, lastPos, nextPos;
    MAT tmpMat;
    QUATERNION lastQuat, nextQuat;
    int iWheel;
    unsigned long lapTime;
    GHOST_DATA *lastData, *nextData;


    // get lap time
    if (Players[0].CarAI.PreLap)
    {
        lapTime = 0;
    }
    else
    {
        /*lapTime = MS2TIME(TotalRacePhysicsTime) + TotalRaceStartTime;
        if (lapTime < car->CurrentLapStartTime) {
            lapTime = 0;
        } else {
            lapTime -= car->CurrentLapStartTime;
            lapTime = TIME2MS(lapTime);
        }*/
        lapTime = TotalRacePhysicsTime;
    }

    // Figure out which element of the data is just behind current lap time
    while ((GHO_BestFrame > 0) && (lapTime < BestGhostData[GHO_BestFrame].Time)) {
        GHO_BestFrame--;
    }
    while ((GHO_BestFrame < GHO_BestGhostInfo->NFrames - 1) && (lapTime > BestGhostData[GHO_BestFrame + 1].Time)) {
        GHO_BestFrame++;
    }
    lastData = &BestGhostData[GHO_BestFrame];
    nextData = &BestGhostData[GHO_BestFrame + 1];

    // Keep frame number within bounds of data (last item is used as a buffer)
    if (GHO_BestFrame > GHO_BestGhostInfo->NFrames - 2) {
        lastData = &BestGhostData[GHO_BestGhostInfo->NFrames - 2];
        nextData = &BestGhostData[GHO_BestGhostInfo->NFrames - 1];
        GHO_BestFrame = GHO_BestGhostInfo->NFrames - 1;
    }

    // Calculate new position
    CopyVec(&car->Body->Centre.Pos, &car->Body->Centre.OldPos);

    lastPos.v[X] = lastData->PosX;
    lastPos.v[Y] = lastData->PosY;
    lastPos.v[Z] = lastData->PosZ;
    nextPos.v[X] = nextData->PosX;
    nextPos.v[Y] = nextData->PosY;
    nextPos.v[Z] = nextData->PosZ;

    VecMinusVec(&nextPos, &lastPos, &dR);
    dt = Real(nextData->Time - lastData->Time);
    if (dt > SMALL_REAL) {
        dt = (Real(lapTime) - Real(lastData->Time)) / dt;
    } else {
        dt = ZERO;
    }
    if (dt < ZERO) {
        dt = ZERO;
    }
    if (dt > ONE) {
        dt = ONE;
    }

    DBG_dt = dt;

    VecPlusScalarVec(&lastPos, dt, &dR, &car->Body->Centre.Pos);

    // Calculate ghost velocity
    if (TimeStep > SMALL_REAL) {
        VecMinusVec(&car->Body->Centre.Pos, &car->Body->Centre.OldPos, &car->Body->Centre.Vel);
        VecDivScalar(&car->Body->Centre.Vel, TimeStep);
    } else {
        SetVecZero(&car->Body->Centre.Vel);
    }

    lastQuat.v[VX] = GHOST_VECTOR_INVSCALE * (REAL)(lastData->Quat.v[VX]);
    lastQuat.v[VY] = GHOST_VECTOR_INVSCALE * (REAL)(lastData->Quat.v[VY]);
    lastQuat.v[VZ] = GHOST_VECTOR_INVSCALE * (REAL)(lastData->Quat.v[VZ]);
    lastQuat.v[S] = GHOST_VECTOR_INVSCALE * (REAL)(lastData->Quat.v[S]);
    nextQuat.v[VX] = GHOST_VECTOR_INVSCALE * (REAL)(nextData->Quat.v[VX]);
    nextQuat.v[VY] = GHOST_VECTOR_INVSCALE * (REAL)(nextData->Quat.v[VY]);
    nextQuat.v[VZ] = GHOST_VECTOR_INVSCALE * (REAL)(nextData->Quat.v[VZ]);
    nextQuat.v[S] = GHOST_VECTOR_INVSCALE * (REAL)(nextData->Quat.v[S]);
//  SLerpQuat(&lastQuat, &nextQuat, dt, &car->Body->Centre.Quat);
//  CopyQuat(&lastQuat, &car->Body->Centre.Quat);
    LerpQuat(&lastQuat, &nextQuat, dt, &car->Body->Centre.Quat);
    NormalizeQuat(&car->Body->Centre.Quat);
    QuatToMat(&car->Body->Centre.Quat, &car->Body->Centre.WMatrix);

    // Set the wheel positions
    car->Revs = ZERO;
    for (iWheel = 0; iWheel < CAR_NWHEELS; iWheel++) {
        //car->Wheel[iWheel].Pos = GHOST_WHEEL_INVSCALE * (REAL)lastData->WheelPos[iWheel];
        //car->Wheel[iWheel].TurnAngle = GHOST_ANGLE_INVSCALE * (REAL)lastData->WheelAngle[iWheel];
        car->Wheel[iWheel].Pos = ZERO;
        car->Wheel[iWheel].TurnAngle = ZERO;
        if (iWheel < 2) {
            car->Wheel[iWheel].TurnAngle = GHOST_ANGLE_INVSCALE * (REAL)lastData->WheelAngle;
        }
        
        // Wheel position
        //VecPlusScalarVec(&car->WheelOffset[iWheel], car->Wheel[iWheel].Pos, &DownVec, &dR);
        SetVec(&dR, car->WheelOffset[iWheel].v[X], car->WheelOffset[iWheel].v[Y] + car->Wheel[iWheel].Pos, car->WheelOffset[iWheel].v[Z]);
        VecMulMat(&dR, &car->Body->Centre.WMatrix, &car->Wheel[iWheel].WPos);
        VecPlusEqVec(&car->Wheel[iWheel].WPos, &car->Body->Centre.Pos);

        // Calculate wheel angular velocity
        car->Wheel[iWheel].AngVel = VecDotVec(&car->Body->Centre.Vel, &car->Body->Centre.WMatrix.mv[L]) / car->Wheel[iWheel].Radius;
        car->Wheel[iWheel].AngPos += TimeStep * car->Wheel[iWheel].AngVel;
        GoodWrap(&car->Wheel[iWheel].AngPos, ZERO, FULL_CIRCLE);
        car->Revs += car->Wheel[iWheel].AngVel;
        
        // Wheel matrix
        RotationY(&tmpMat, car->Wheel[iWheel].TurnAngle);
        MatMulMat(&tmpMat, &car->Body->Centre.WMatrix, &car->Wheel[iWheel].Axes);
        RotationX(&tmpMat, car->Wheel[iWheel].AngPos);
        MatMulMat(&tmpMat, &car->Wheel[iWheel].Axes, &car->Wheel[iWheel].WMatrix);
    }
    car->Revs *= Real(0.25) * 90;

    // set light params
    if( GhostLight )
    {
        CopyVec(&car->Body->Centre.Pos, (VEC*)&GhostLight->x);
        GhostLight->r = 0;
        GhostLight->g = 32;
        GhostLight->b = 64;
    }

}


/////////////////////////////////////////////////////////////////////
//
// LoadGhostData:
//
/////////////////////////////////////////////////////////////////////

bool LoadGhostData()
{
#ifdef _PC
    size_t  nRead;
    FILE    *fp;
    unsigned long crcRead, crcCalc;
    char    levelDir[MAX_LEVEL_DIR_NAME];

    if (!GHO_GhostAllowed) return FALSE;

    // open the ghost data filename

    if (GameSettings.LocalGhost) {
//      if (GameSettings.Mirrored)
//          fp = fopen(GetLevelFilename(GHOSTLOCAL_FILENAME_MIRRORED, FILENAME_GAME_SETTINGS), "rb");
//      else
//          fp = fopen(GetLevelFilename(GHOSTLOCAL_FILENAME, FILENAME_GAME_SETTINGS), "rb");
        fp = fopen(GetGhostFilename(GameSettings.Level, GameSettings.Reversed, GameSettings.Mirrored, GHOSTLOCAL_EXT), "rb");
    } else {
//      if (GameSettings.Mirrored)
//          fp = fopen(GetLevelFilename(GHOSTDOWNLOAD_FILENAME_MIRRORED, FILENAME_GAME_SETTINGS), "rb");
//      else
//          fp = fopen(GetLevelFilename(GHOSTDOWNLOAD_FILENAME, FILENAME_GAME_SETTINGS), "rb");
        fp = fopen(GetGhostFilename(GameSettings.Level, GameSettings.Reversed, GameSettings.Mirrored, GHOSTDOWNLOAD_EXT), "rb");
    }

    if (fp == NULL) {
        return FALSE;
    }

    // Calculate the file's checksum
    crcCalc = GetStreamChecksum(fp, TRUE, TRUE);


    // Read the file header
    nRead = fread(&GHO_GhostFileHeader, sizeof(GHOST_HEADER), 1, fp);
    if (nRead < 1) {
        fclose(fp);
        return FALSE;
    }

    // Read the ghost info header
    nRead = fread(GHO_BestGhostInfo, sizeof(GHOST_INFO), 1, fp);
    if (nRead < 1) {
        fclose(fp);
        return FALSE;
    }

    // Read the ghost data
    nRead = fread(BestGhostData, sizeof(GHOST_DATA), GHOST_DATA_MAX, fp);
    if (nRead < GHOST_DATA_MAX) {
        fclose(fp);
        return FALSE;
    }

    // Read the level dir name
    nRead = fread(levelDir, sizeof(char), MAX_LEVEL_DIR_NAME, fp);
    if (nRead < MAX_LEVEL_DIR_NAME) {
        fclose(fp);
        return FALSE;
    }

    // Verify level is correct
    if (strncmp(levelDir, StartData.LevelDir, MAX_LEVEL_DIR_NAME) != 0) {
        fclose(fp);
        return FALSE;
    }

    // Read in the checksum
    nRead = fread(&crcRead, sizeof(unsigned long), 1, fp);
    if (nRead < 1) {
        fclose(fp);
        return FALSE;
    }

    // Validate checksum
    if (crcRead != crcCalc) {
        DumpMessage("Dodgy Ghost File", "Stop Cheating or else...");
        return FALSE;
    }

    // Set the ghost car ID from the saved name to allow downloaded cars to get best times
    char c, buf[64];
    int i = 0;
    while ((c = GHO_GhostFileHeader.CarName[i]) != ';') buf[i++] = c;
    buf[i] = '\0';
    GHO_BestGhostInfo->CarType = GetCarTypeFromName(buf);

    // Success!
    fclose(fp);
    return TRUE;
#endif
}



/////////////////////////////////////////////////////////////////////
//
// SaveGhostData
//
/////////////////////////////////////////////////////////////////////

bool SaveGhostData()
{
#ifdef _PC
    int     nWritten;
    FILE    *fp;
    unsigned long crc;

    // Make sure there is some ghost data
    if (!GHO_GhostDataExists) {
        return FALSE;
    }

    // open the ghost data filename
//  if (GameSettings.Mirrored) {
//      fp = fopen(GetLevelFilename(GHOSTLOCAL_FILENAME_MIRRORED, FILENAME_GAME_SETTINGS), "wb+");
//  } else {
//      fp = fopen(GetLevelFilename(GHOSTLOCAL_FILENAME, FILENAME_GAME_SETTINGS), "wb+");
//  }
    fp = fopen(GetGhostFilename(GameSettings.Level, GameSettings.Reversed, GameSettings.Mirrored, GHOSTLOCAL_EXT), "wb+");

    if (fp == NULL) {
        return FALSE;
    }

    // Clear the header
    memset(&GHO_GhostFileHeader, ' ', sizeof(GHO_GhostFileHeader));

    // Write the Header
    sprintf(GHO_GhostFileHeader.Header, "%-20s%1.2f;", "ReVolt Ghost File", 1.01f);
    GHO_GhostFileHeader.Header[strlen(GHO_GhostFileHeader.Header)] = ' ';

    sprintf(GHO_GhostFileHeader.LevelName, "%s;", GetLevelInfo(GameSettings.Level)->Name);
    GHO_GhostFileHeader.LevelName[strlen(GHO_GhostFileHeader.LevelName)] = ' ';

    sprintf(GHO_GhostFileHeader.LevelFlags, "%s%s%s;", 
        (GameSettings.Mirrored)? "Mir:": "",
        (GameSettings.Reversed)? "Rev:": "",
        (CarInfo[GHO_BestGhostInfo->CarType].Modified || CarInfo[GHO_BestGhostInfo->CarType].Moved || ModifiedCarInfo)? "Cht:": "");
    GHO_GhostFileHeader.LevelFlags[strlen(GHO_GhostFileHeader.LevelFlags)] = ' ';

    sprintf(GHO_GhostFileHeader.CarName, "%s;", CarInfo[StartData.PlayerData[0].CarType].Name);
    GHO_GhostFileHeader.CarName[strlen(GHO_GhostFileHeader.CarName)] = ' ';

    sprintf(GHO_GhostFileHeader.PlayerName, "%s;", StartData.PlayerData[0].Name);
    GHO_GhostFileHeader.PlayerName[strlen(GHO_GhostFileHeader.PlayerName)] = ' ';

    long time = GHO_BestGhostInfo->Time[GHOST_LAP_TIME];
    sprintf(GHO_GhostFileHeader.Time, "%2d:%2d:%3d;", MINUTES(time), SECONDS(time), THOUSANDTHS(time));
    GHO_GhostFileHeader.Time[strlen(GHO_GhostFileHeader.Time)] = ' ';
    
    GHO_GhostFileHeader.Date[0] = ';';
    GHO_GhostFileHeader.Unused[0] = ';';
    

    nWritten = fwrite(&GHO_GhostFileHeader, sizeof(GHOST_HEADER), 1, fp);
    if (nWritten < 1) {
        fclose(fp);
        return FALSE;
    }

    // write the info 
    nWritten = fwrite(GHO_BestGhostInfo, sizeof(GHOST_INFO), 1, fp);
    if (nWritten < 1) {
        fclose(fp);
        return FALSE;
    }

    // write the data
    nWritten = fwrite(BestGhostData, sizeof(GHOST_DATA), GHOST_DATA_MAX, fp);
    if (nWritten < GHOST_DATA_MAX) {
        fclose(fp);
        return FALSE;
    }

    // Output level
    nWritten = fwrite(StartData.LevelDir, sizeof(char), MAX_LEVEL_DIR_NAME, fp);
    if (nWritten < MAX_LEVEL_DIR_NAME) {
        fclose(fp);
        return FALSE;
    }

    // Calculate Checksum
    fflush(fp);
    crc = GetStreamChecksum(fp, FALSE, TRUE);

    // Append the checksum
    nWritten = fwrite(&crc, sizeof(unsigned long), 1, fp);
    if (nWritten < 1) {
        fclose(fp);
        return FALSE;
    }

    // Success!
    fclose(fp);
    return TRUE;
#endif
}

//////////////////////
//                  //
// init ghost light //
//                  //
//////////////////////

void InitGhostLight(void)
{
    if (!GHO_GhostAllowed) return;
    if (!GHO_ShowGhost) return;

    if ((GhostLight = AllocLight()))
    {
        GhostLight->Reach = 768;
        GhostLight->Flag = LIGHT_FIXED | LIGHT_MOVING;
        GhostLight->Type = LIGHT_OMNI;
    }

}

void ReleaseGhostLight(void)
{
    if (GhostLight != NULL) {
        FreeLight(GhostLight);
        GhostLight = NULL;
    
    }
}

