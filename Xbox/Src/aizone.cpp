//-----------------------------------------------------------------------------
// File: AIZone.cpp
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"
#include "car.h"
#include "ctrlread.h"
#include "object.h"
#include "player.h"
#ifndef _PSX
#include "main.h"
#endif
#include "editzone.h"
#include "geom.h"
#include "aizone.h"
#include "ainode.h"
#include "trigger.h"
#ifdef _N64
 #include "ffs_code.h"
 #include "ffs_list.h"
 #include "utils.h"
#endif

// globals

long AiZoneNum, AiZoneNumID;
AIZONE *AiZones;
AIZONE_HEADER *AiZoneHeaders;

///////////////////
// load ai zones //
///////////////////
#ifdef _PC
void LoadAiZones(char *file)
{
    long        i, j, k;
    FILE        *fp;
    FILE_ZONE   taz;
    AIZONE      zone;
    VEC         pos[2];
    int         c1, c2;

// zero mem ptrs

    AiZones = NULL;
    AiZoneHeaders = NULL;
    AiZoneNum = 0;
    AiZoneNumID = 0;

// open zone file

    fp = fopen(file, "rb");
    if (!fp)
    {
        return;
    }

// read zone num

    fread(&AiZoneNum, sizeof(AiZoneNum), 1, fp);
    if (!AiZoneNum)
    {
        fclose(fp);
        return;
    }

// alloc ram for zones

    AiZones = (AIZONE*)malloc(sizeof(AIZONE) * AiZoneNum);
    if (!AiZones)
    {
        fclose(fp);
        DumpMessage(NULL, "Can't alloc memory for AI zones!");
        QuitGame();
        return;
    }

// load and convert each file zone

    for (i = 0 ; i < AiZoneNum ; i++)
    {

// read file zone

        fread(&taz, sizeof(taz), 1, fp);

// set ID

        AiZones[i].ID = taz.ID;
        AiZones[i].Index = i;

// set XYZ size
        AiZones[i].Size[0] = TO_LENGTH(taz.Size[0]);
        AiZones[i].Size[1] = TO_LENGTH(taz.Size[1]);
        AiZones[i].Size[2] = TO_LENGTH(taz.Size[2]);
        //AiZones[i].Size[0] = (taz.Size[0]);
        //AiZones[i].Size[1] = (taz.Size[1]);
        //AiZones[i].Size[2] = (taz.Size[2]);

// save Ccentre pos
        AiZones[i].Pos.v[0] = TO_LENGTH(taz.Pos.v[0]);
        AiZones[i].Pos.v[1] = TO_LENGTH(taz.Pos.v[1]);
        AiZones[i].Pos.v[2] = TO_LENGTH(taz.Pos.v[2]);

// build 3 planes

        AiZones[i].Plane[0].v[A] = taz.Matrix.m[RX];
        AiZones[i].Plane[0].v[B] = taz.Matrix.m[RY];
        AiZones[i].Plane[0].v[C] = taz.Matrix.m[RZ];
        AiZones[i].Plane[0].v[D] = -DotProduct(&taz.Matrix.mv[R], &taz.Pos);

        AiZones[i].Plane[1].v[A] = taz.Matrix.m[UX];
        AiZones[i].Plane[1].v[B] = taz.Matrix.m[UY];
        AiZones[i].Plane[1].v[C] = taz.Matrix.m[UZ];
        AiZones[i].Plane[1].v[D] = -DotProduct(&taz.Matrix.mv[U], &taz.Pos);

        AiZones[i].Plane[2].v[A] = taz.Matrix.m[LX];
        AiZones[i].Plane[2].v[B] = taz.Matrix.m[LY];
        AiZones[i].Plane[2].v[C] = taz.Matrix.m[LZ];
        AiZones[i].Plane[2].v[D] = -DotProduct(&taz.Matrix.mv[L], &taz.Pos);

// Create axis aligned bounding box
        AiZones[i].boundsMin.v[0] = LARGEDIST;
        AiZones[i].boundsMin.v[1] = LARGEDIST;
        AiZones[i].boundsMin.v[2] = LARGEDIST;
        AiZones[i].boundsMax.v[0] = -LARGEDIST;
        AiZones[i].boundsMax.v[1] = -LARGEDIST;
        AiZones[i].boundsMax.v[2] = -LARGEDIST;

        for (c1 = 0; c1 < 8; c1++)
        {
            if (c1 & 1)     pos[0].v[0] = -AiZones[i].Size[0];
            else            pos[0].v[0] = AiZones[i].Size[0];

            if (c1 & 2)     pos[0].v[1] = -AiZones[i].Size[1];
            else            pos[0].v[1] = AiZones[i].Size[1];

            if (c1 & 4)     pos[0].v[2] = -AiZones[i].Size[2];
            else            pos[0].v[2] = AiZones[i].Size[2];

            RotVector(&taz.Matrix, &pos[0], &pos[1]);

            pos[1].v[0] += AiZones[i].Pos.v[0];
            pos[1].v[1] += AiZones[i].Pos.v[1];
            pos[1].v[2] += AiZones[i].Pos.v[2];

            for (c2 = 0; c2 < 3; c2++)
            {
                if (AiZones[i].boundsMin.v[c2] > pos[1].v[c2])
                    AiZones[i].boundsMin.v[c2] = pos[1].v[c2];
                if (AiZones[i].boundsMax.v[c2] < pos[1].v[c2])
                    AiZones[i].boundsMax.v[c2] = pos[1].v[c2];
            }
        }
    }

// sort zones

    for (i = AiZoneNum - 1 ; i ; i--) for (j = 0 ; j < i ; j++) if (AiZones[j].ID > AiZones[j + 1].ID)
    {
        zone = AiZones[j];
        AiZones[j] = AiZones[j + 1];
        AiZones[j + 1] = zone;
    }

// set zone ID num

    AiZoneNumID = AiZones[AiZoneNum - 1].ID + 1;

// alloc memory for zone headers

    AiZoneHeaders = (AIZONE_HEADER*)malloc(sizeof(AIZONE_HEADER) * AiZoneNumID);
    if (!AiZoneHeaders)
    {
        fclose(fp);
        DumpMessage(NULL, "Can't alloc memory for AI zone headers!");
        QuitGame();
        return;
    }

// setup zone headers

    for (i = 0 ; i < AiZoneNumID ; i++)
    {
        j = 0;
        while (AiZones[j].ID != i && j < AiZoneNum) j++;
        if (j == AiZoneNum)
        {
            AiZoneHeaders[i].Zones = AiZones;
            AiZoneHeaders[i].Count = 0;
        }
        else
        {   
            AiZoneHeaders[i].Zones = &AiZones[j];
            k = 0;
            while ((j < AiZoneNum) && (AiZones[j].ID == i)) j++, k++;
            AiZoneHeaders[i].Count = k;
        }
    }

// close file

    fclose(fp);
}
#endif

//
// N64 version
//

#ifdef _N64
void LoadAiZones(void)
{
    long        i, j, k;
    FIL         *fp;
    FILE_ZONE   taz;
    AIZONE      zone;
    VEC         pos[2];
    int         c1, c2;

// zero mem ptrs

    AiZones = NULL;
    AiZoneHeaders = NULL;
    AiZoneNum = 0;
    AiZoneNumID = 0;

// open zone file

    printf("Loading AI zones.\n");
    fp = FFS_Open(FFS_TYPE_TRACK | (GameSettings.Reversed ? TRK_REV_AIZONES : TRK_AIZONES) );
    if (!fp)
    {
        printf("WARNING: Failed to open AI zone file.\n");
        return;
    }

// Read zone num
    FFS_Read(&AiZoneNum, sizeof(AiZoneNum), fp);
    if (!AiZoneNum)
    {
        printf("WARNING: Number of AI Zones is zero, skipping.\n");
        FFS_Close(fp);
        return;
    }
    AiZoneNum = EndConvLong(AiZoneNum);

// Alloc ram for zones
    AiZones = (AIZONE*)malloc(sizeof(AIZONE) * AiZoneNum);
    if (!AiZones)
    {
        FFS_Close(fp);  
        ERROR("AIZ", "LoadAiZones", "Failed to alloc memory for AI zones", 1);
    }

// Load and convert each file zone
    for (i = 0 ; i < AiZoneNum ; i++)
    {

// Read file zone
        FFS_Read(&taz, sizeof(taz), fp);
        taz.ID = EndConvLong(taz.ID);
        taz.Pos.v[0] = EndConvReal(taz.Pos.v[0]);
        taz.Pos.v[1] = EndConvReal(taz.Pos.v[1]);
        taz.Pos.v[2] = EndConvReal(taz.Pos.v[2]);
        for (j = 0; j < 9; j++)
        {
            taz.Matrix.m[j] = EndConvReal(taz.Matrix.m[j]);
        }
        taz.Size[0] = EndConvReal(taz.Size[0]);
        taz.Size[1] = EndConvReal(taz.Size[1]);
        taz.Size[2] = EndConvReal(taz.Size[2]);
    
// Set ID
        AiZones[i].ID = taz.ID;
        AiZones[i].Index = i;

// Set XYZ size
        AiZones[i].Size[0] = taz.Size[0];
        AiZones[i].Size[1] = taz.Size[1];
        AiZones[i].Size[2] = taz.Size[2];

// save centre pos
        AiZones[i].Pos.v[X] = taz.Pos.v[X];
        AiZones[i].Pos.v[Y] = taz.Pos.v[Y];
        AiZones[i].Pos.v[Z] = taz.Pos.v[Z];

// Build 3 planes
        AiZones[i].Plane[0].v[A] = taz.Matrix.m[RX];
        AiZones[i].Plane[0].v[B] = taz.Matrix.m[RY];
        AiZones[i].Plane[0].v[C] = taz.Matrix.m[RZ];
        AiZones[i].Plane[0].v[D] = -DotProduct(&taz.Matrix.mv[R], &taz.Pos);

        AiZones[i].Plane[1].v[A] = taz.Matrix.m[UX];
        AiZones[i].Plane[1].v[B] = taz.Matrix.m[UY];
        AiZones[i].Plane[1].v[C] = taz.Matrix.m[UZ];
        AiZones[i].Plane[1].v[D] = -DotProduct(&taz.Matrix.mv[U], &taz.Pos);

        AiZones[i].Plane[2].v[A] = taz.Matrix.m[LX];
        AiZones[i].Plane[2].v[B] = taz.Matrix.m[LY];
        AiZones[i].Plane[2].v[C] = taz.Matrix.m[LZ];
        AiZones[i].Plane[2].v[D] = -DotProduct(&taz.Matrix.mv[L], &taz.Pos);

// Create axis aligned bounding box
        AiZones[i].boundsMin.v[0] = LARGEDIST;
        AiZones[i].boundsMin.v[1] = LARGEDIST;
        AiZones[i].boundsMin.v[2] = LARGEDIST;
        AiZones[i].boundsMax.v[0] = -LARGEDIST;
        AiZones[i].boundsMax.v[1] = -LARGEDIST;
        AiZones[i].boundsMax.v[2] = -LARGEDIST;

        for (c1 = 0; c1 < 8; c1++)
        {
            if (c1 & 1)     pos[0].v[0] = -AiZones[i].Size[0];
            else            pos[0].v[0] = AiZones[i].Size[0];

            if (c1 & 2)     pos[0].v[1] = -AiZones[i].Size[1];
            else            pos[0].v[1] = AiZones[i].Size[1];

            if (c1 & 4)     pos[0].v[2] = -AiZones[i].Size[2];
            else            pos[0].v[2] = AiZones[i].Size[2];

            RotVector(&taz.Matrix, &pos[0], &pos[1]);

            pos[1].v[0] += AiZones[i].Pos.v[0];
            pos[1].v[1] += AiZones[i].Pos.v[1];
            pos[1].v[2] += AiZones[i].Pos.v[2];

            for (c2 = 0; c2 < 3; c2++)
            {
                if (AiZones[i].boundsMin.v[c2] > pos[1].v[c2])
                    AiZones[i].boundsMin.v[c2] = pos[1].v[c2];
                if (AiZones[i].boundsMax.v[c2] < pos[1].v[c2])
                    AiZones[i].boundsMax.v[c2] = pos[1].v[c2];
            }
        }
    }

// sort zones
    for (i = AiZoneNum - 1 ; i ; i--) for (j = 0 ; j < i ; j++) if (AiZones[j].ID > AiZones[j + 1].ID)
    {
        zone = AiZones[j];
        AiZones[j] = AiZones[j + 1];
        AiZones[j + 1] = zone;
    }

// Set zone ID num
    AiZoneNumID = AiZones[AiZoneNum - 1].ID + 1;

// Alloc memory for zone headers
    AiZoneHeaders = (AIZONE_HEADER*)malloc(sizeof(AIZONE_HEADER) * AiZoneNumID);
    if (!AiZoneHeaders)
    {
        FFS_Close(fp);  
        ERROR("AIZ", "LoadAiZones", "Unable to alloc memory for AI zone headers", 1);
    }

// Setup zone headers
    for (i = 0 ; i < AiZoneNumID ; i++)
    {
        j = 0;
        while (AiZones[j].ID != i && j < AiZoneNum) j++;
        if (j == AiZoneNum)
        {
            AiZoneHeaders[i].Zones = AiZones;
            AiZoneHeaders[i].Count = 0;
        }
        else
        {   
            AiZoneHeaders[i].Zones = &AiZones[j];
            k = 0;
            while ((j < AiZoneNum) && (AiZones[j].ID == i)) j++, k++;
            AiZoneHeaders[i].Count = k;
        }
    }

// Close file
    FFS_Close(fp);
}
#endif

///////////////////
// free ai zones //
///////////////////

void FreeAiZones(void)
{
    free(AiZones);
    free(AiZoneHeaders);

    AiZones = NULL;
    AiZoneHeaders = NULL;
}

////////////////////////////////////////////////////
// update car zone - return TRUE if completed lap //
////////////////////////////////////////////////////

char UpdateCarAiZone(PLAYER *Player)
{
    long i, j, flag, lastzone, nextzone;
    REAL dist;
    AIZONE *zone;
    CAR *car = &Player->car;


// quit if no zones

    if (!AiZones)
        return FALSE;


// Is player in same zone ?
    zone = AiZoneHeaders[Player->CarAI.ZoneID].Zones;
    for (i = 0 ; i < AiZoneHeaders[Player->CarAI.ZoneID].Count ; i++, zone++)
    {
        if (AIZ_IsPointInZone(&car->Body->Centre.Pos, zone))
            return TRUE;
    }


// loop thru all zones with next ID

    nextzone = (Player->CarAI.ZoneID + 1) % AiZoneNumID;
    zone = AiZoneHeaders[nextzone].Zones;

    for (i = 0 ; i < AiZoneHeaders[nextzone].Count ; i++, zone++)
    {

// test car against next zones

        flag = FALSE;
        for (j = 0 ; j < 3 ; j++)
        {
            dist = PlaneDist(&zone->Plane[j], &car->Body->Centre.Pos);
        
            if ( (dist < -zone->Size[j]) || (dist > zone->Size[j]) )
            {
                flag = TRUE;
                break;
            }
        }

// entered next zone?


        if (!flag)
        {

// yep, update zone ID

            Player->CarAI.ZoneID = nextzone;
            return TRUE;
        }
    }

// loop thru all zones with prev ID


    lastzone = (Player->CarAI.ZoneID + AiZoneNumID - 1) % AiZoneNumID;
    zone = AiZoneHeaders[lastzone].Zones;

    for (i = 0 ; i < AiZoneHeaders[lastzone].Count ; i++, zone++)
    {

// test car against last zones

        flag = FALSE;
        for (j = 0 ; j < 3 ; j++)
        {
        
            dist = PlaneDist(&zone->Plane[j], &car->Body->Centre.Pos);
            if (dist < -zone->Size[j] || dist > zone->Size[j])
            {
                flag = TRUE;
                break;
            }
        }

// entered last zone?

        if (!flag)
        {

// yep, update zone ID

            Player->CarAI.ZoneID = lastzone;
            return TRUE;
        }
    }

//  return false

    return FALSE;
}




long FindAiZone( VEC *Pos )
{
    long i, j, ret, z;
    REAL dist;
    AIZONE *zone;

    VEC PSXPos;

    PSXPos.v[0] = TO_LENGTH( Pos->v[0] );
    PSXPos.v[1] = TO_LENGTH( Pos->v[1] );
    PSXPos.v[2] = TO_LENGTH( Pos->v[2] );

    ret = -1;

    if (!AiZones)
        return -1;

// loop thru all zones with next ID

    
    zone = AiZoneHeaders[0].Zones;

    for (i = 0 ; i < AiZoneNumID ; i++, zone++)
    {


        z = 0;

        for (j = 0 ; j < 3 ; j++)
        {
            dist = PlaneDist(&zone->Plane[j], &PSXPos );
        
            if ( (dist < -zone->Size[j]) || (dist > zone->Size[j]) )
            {
                z = 1;
                break;
            }
        }


        if( !z )
            ret = i;
        
    }

    return ret;

}



/********************************************************************************
* AIZ_IsPointInZone()
*
* Checks if the specified point is in a zone or not.
********************************************************************************/
bool AIZ_IsPointInZone(VEC *pPos, AIZONE *pZone)
{
    long    kk;
    REAL    dist;

// Is point in zone bounds ?
    for (kk = 0; kk < 3; kk++)
    {
        if (pPos->v[kk] < pZone->boundsMin.v[kk])
            return FALSE;
        if (pPos->v[kk] > pZone->boundsMax.v[kk])
            return FALSE;
    }

// Is point in zone planes ?
    for (kk = 0; kk < 3; kk++)
    {
        dist = PlaneDist(&pZone->Plane[kk], pPos);
        if ((dist < -pZone->Size[kk]) || (dist > pZone->Size[kk]))
            return FALSE;
    }

    return TRUE;
}


/********************************************************************************
* AIZ_IsCarInZoneID()
*
* Checks if the specified point is in a zone ID or not.
********************************************************************************/
bool AIZ_IsCarInZoneID(PLAYER *pPlayer, int ID)
{
    long    jj;
    AIZONE  *zone;

    for (jj = 0; jj < AiZoneHeaders[ID].Count; jj++)
    {
        zone = AiZoneHeaders[ID].Zones + jj;

        if (AIZ_IsPointInZone(&pPlayer->car.Body->Centre.Pos, zone))
        {
            pPlayer->CarAI.LastValidZone = pPlayer->CarAI.CurZone;
            pPlayer->CarAI.CurZone     = zone->Index;
            pPlayer->CarAI.CurZoneID   = ID;
            pPlayer->CarAI.CurZoneBBox = jj;
            return TRUE;
        }
    }

    return FALSE;
}
