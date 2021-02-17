//-----------------------------------------------------------------------------
// File: AINode.cpp
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"
#include "main.h"
#include "ainode.h"
#include "aizone.h"
#include "editai.h"
#include "car.h"
#include "ctrlread.h"
#include "object.h"
#include "Control.h"
#include "player.h"
#include "weapon.h"
#include "obj_init.h"

#ifdef _N64
#include "ffs_code.h"
#include "ffs_list.h"
#include "utils.h"
#endif

#ifdef _PSX

#include "util.h"
#include "LevelLoad.h"
#endif

//#define AIN_RECURSIVE_SEARCH
                 
//
// Global varaibles
//

short AiNodeNum;
short AiSingleNodeNum;
short AiLinkNodeNum;
AINODE *AiNode;
AINODE_ZONE *AiNodeZone;


//
// Global function declarations
//
#ifndef _N64
void LoadAiNodes(char *file);
#endif
#ifdef _N64
void LoadAiNodes(void);
#endif
void FreeAiNodes(void);

AINODE *AIN_NearestNode(PLAYER *Player, REAL *Dist);
//AINODE *AIN_GetForwardNode(PLAYER *Player, REAL MinDist, REAL *Dist);

void PostLoadProcessAiNodes(void);

int AIN_IsNodeInsideZones(AINODE *pNode, AIZONE **ppZonesInside, int *piZonesInside);

long AiStartNode;
REAL AiNodeTotalDist;

///////////////////
// load ai nodes //
///////////////////
#ifndef _N64
void LoadAiNodes(char *file)
{
    long        i, j;
    FILE        *fp;
    FILE_AINODE fan;
    AINODE      *pNode;

// open ai node file

    AiNode = NULL;
    AiNodeNum = 0;
    AiSingleNodeNum = 0;
    AiLinkNodeNum = 0;

    fp = fopen(file, "rb");
    if (!fp) return;


// alloc ram

    fread(&AiSingleNodeNum, sizeof(AiSingleNodeNum), 1, fp);
    fread(&AiLinkNodeNum, sizeof(AiLinkNodeNum), 1, fp);
    AiNodeNum = AiSingleNodeNum + AiLinkNodeNum;
    if (!AiNodeNum) return;

    AiNode = (AINODE*)malloc(sizeof(AINODE) * AiNodeNum);
    if (!AiNode)
    {
        AiNodeNum = 0;
        return;
    }

// loop thru all ai nodes
    pNode = AiNode;
//  for (i = 0; i < AiNodeNum ; i++)
    for (i = 0; i < AiSingleNodeNum ; i++)
    {
// load one file ai node
        fread(&fan, sizeof(fan), 1, fp);

// PSX conversion
#ifdef _PSX
        fan.Node[0].Pos.v[X] = TO_LENGTH(fan.Node[0].Pos.v[X]);
        fan.Node[0].Pos.v[Y] = TO_LENGTH(fan.Node[0].Pos.v[Y]);
        fan.Node[0].Pos.v[Z] = TO_LENGTH(fan.Node[0].Pos.v[Z]);
        fan.Node[1].Pos.v[X] = TO_LENGTH(fan.Node[1].Pos.v[X]);
        fan.Node[1].Pos.v[Y] = TO_LENGTH(fan.Node[1].Pos.v[Y]);
        fan.Node[1].Pos.v[Z] = TO_LENGTH(fan.Node[1].Pos.v[Z]);
#endif

// setup ai node
        pNode->Node[0].Pos      = fan.Node[1].Pos;
#ifndef _PSX
        pNode->Node[0].Speed    = fan.Node[1].Speed;
#endif
        pNode->Node[1].Pos      = fan.Node[0].Pos;
#ifndef _PSX
        pNode->Node[1].Speed    = fan.Node[0].Speed;
#endif
        pNode->Priority         = fan.Priority;
        pNode->StartNode        = fan.StartNode;
        pNode->RacingLine       = fan.RacingLine;
        pNode->RacingLineSpeed  = fan.RacingLineSpeed;
        pNode->CentreSpeed      = fan.CentreSpeed;
        pNode->FinishDist       = fan.FinishDist;
        pNode->OvertakingLine   = fan.OvertakingLine;
        pNode->link.flags       = fan.flags[0];
//      pNode->linkInfo[0].flags    = fan.flags[0];
//      pNode->linkInfo[1].flags    = fan.flags[1];
    
        pNode->Centre.v[X] = pNode->Node[0].Pos.v[X] + MulScalar((pNode->Node[1].Pos.v[X] - pNode->Node[0].Pos.v[X]), pNode->RacingLine);
        pNode->Centre.v[Y] = pNode->Node[0].Pos.v[Y] + MulScalar((pNode->Node[1].Pos.v[Y] - pNode->Node[0].Pos.v[Y]), pNode->RacingLine);
        pNode->Centre.v[Z] = pNode->Node[0].Pos.v[Z] + MulScalar((pNode->Node[1].Pos.v[Z] - pNode->Node[0].Pos.v[Z]), pNode->RacingLine);

        for (j = 0; j < MAX_AINODE_LINKS; j++)
        {
            if (fan.Prev[j] != -1)
                pNode->Prev[j] = AiNode + fan.Prev[j];
            else
                pNode->Prev[j] = NULL;

            if (fan.Next[j] != -1)
                pNode->Next[j] = AiNode + fan.Next[j];
            else
                pNode->Next[j] = NULL;
        }

//      printf( "%2d : %d %d %d\n", i, pNode->Centre.v[0], pNode->Centre.v[1], pNode->Centre.v[2] );

        if (pNode->Next[1])
            pNode->ZoneID = -1;
        else
            pNode->ZoneID = LONG_MAX;

        pNode++;
    }

// load start node

    fread(&AiStartNode, sizeof(AiStartNode), 1, fp);

// load total dist

    fread(&AiNodeTotalDist, sizeof(AiNodeTotalDist), 1, fp);

// close ai node file

    fclose(fp);


// Post Process
    PostLoadProcessAiNodes();
}


#ifdef _PSX

void MakeEditorAiNodes( void )
{
    long            i, j;
    AINODE          *pNode;
    TE_AINODEINFO   *node;
    VECTOR          centre, old_centre;
    VECTOR          dir, old_dir;
    VECTOR          a, b, temp;
    int             dot, separation;

// open ai node file

    AiNode          = NULL;
    AiNodeNum       = TE_AINodesPlaced;
    AiSingleNodeNum = TE_AINodesPlaced;
    AiLinkNodeNum   = 0;

    if (!AiNodeNum) return;

#if 0
    node = &TE_AINodes[ AiNodeNum-1 ];
    old_centre.vx = node->Ends[ AI_GREEN_NODE ].vx + (( node->Ends[ AI_RED_NODE ].vx - node->Ends[ AI_GREEN_NODE ].vx ) >> 1 );
    old_centre.vy = node->Ends[ AI_GREEN_NODE ].vy + (( node->Ends[ AI_RED_NODE ].vy - node->Ends[ AI_GREEN_NODE ].vy ) >> 1 );
    old_centre.vz = node->Ends[ AI_GREEN_NODE ].vz + (( node->Ends[ AI_RED_NODE ].vz - node->Ends[ AI_GREEN_NODE ].vz ) >> 1 );

    node = &TE_AINodes[ 0 ];
    centre.vx = node->Ends[ AI_GREEN_NODE ].vx + (( node->Ends[ AI_RED_NODE ].vx - node->Ends[ AI_GREEN_NODE ].vx ) >> 1 );
    centre.vy = node->Ends[ AI_GREEN_NODE ].vy + (( node->Ends[ AI_RED_NODE ].vy - node->Ends[ AI_GREEN_NODE ].vy ) >> 1 );
    centre.vz = node->Ends[ AI_GREEN_NODE ].vz + (( node->Ends[ AI_RED_NODE ].vz - node->Ends[ AI_GREEN_NODE ].vz ) >> 1 );

    temp.vx = centre.vx - old_centre.vx;
    temp.vy = centre.vy - old_centre.vy;
    temp.vz = centre.vz - old_centre.vz;
    VectorNormal( &temp, &old_dir );

    for( i=0 ; i<AiNodeNum ; i++, node++ )
    {
        node = &TE_AINodes[ i ];
        centre.vx = node->Ends[ AI_GREEN_NODE ].vx + (( node->Ends[ AI_RED_NODE ].vx - node->Ends[ AI_GREEN_NODE ].vx ) >> 1 );
        centre.vy = node->Ends[ AI_GREEN_NODE ].vy + (( node->Ends[ AI_RED_NODE ].vy - node->Ends[ AI_GREEN_NODE ].vy ) >> 1 );
        centre.vz = node->Ends[ AI_GREEN_NODE ].vz + (( node->Ends[ AI_RED_NODE ].vz - node->Ends[ AI_GREEN_NODE ].vz ) >> 1 );

        node = &TE_AINodes[ ( i+1 ) % AiNodeNum ];
        temp.vx = node->Ends[ AI_GREEN_NODE ].vx + (( node->Ends[ AI_RED_NODE ].vx - node->Ends[ AI_GREEN_NODE ].vx ) >> 1 );
        temp.vy = node->Ends[ AI_GREEN_NODE ].vy + (( node->Ends[ AI_RED_NODE ].vy - node->Ends[ AI_GREEN_NODE ].vy ) >> 1 );
        temp.vz = node->Ends[ AI_GREEN_NODE ].vz + (( node->Ends[ AI_RED_NODE ].vz - node->Ends[ AI_GREEN_NODE ].vz ) >> 1 );

        temp.vx -= centre.vx;
        temp.vy -= centre.vy;
        temp.vz -= centre.vz;
        VectorNormal( &temp, &dir );
        dot = ( ( dir.vx*old_dir.vx )>>12) + (( dir.vy*old_dir.vy )>>12) + (( dir.vz*old_dir.vz )>>12);

        temp.vx = centre.vx - old_centre.vx;
        temp.vy = centre.vy - old_centre.vy;
        temp.vz = centre.vz - old_centre.vz;
        separation = SquareRoot0( temp.vx*temp.vx + temp.vy*temp.vy + temp.vz*temp.vz );
        printf( "centre = %d %d %d : ", centre.vx, centre.vy, centre.vz );
        printf( "dot = %d : separation = %d\n", dot, separation );

        temp.vx = centre.vx - old_centre.vx;
        temp.vy = centre.vy - old_centre.vy;
        temp.vz = centre.vz - old_centre.vz;
        VectorNormal( &temp, &old_dir );
        old_centre = centre;
    }
#endif






    AiNode = (AINODE*)malloc(sizeof(AINODE) * AiNodeNum);
//  AiNode = (AINODE*)malloc(sizeof(AINODE) * (AiNodeNum*2));
    if (!AiNode)
    {
        AiNodeNum = 0;
        return;
    }

// loop thru all ai nodes
    pNode = AiNode;
    j = 0;
    for (i = 0; i < AiNodeNum ; i++)
    {
        node = &TE_AINodes[ i ];

// PSX conversion
#ifndef _PSX
        pNode->Node[0].Speed    = 30;
#endif
        pNode->Node[0].Pos.v[0] = TO_LENGTH( node->Ends[ AI_GREEN_NODE ].vx << 16 );
        pNode->Node[0].Pos.v[1] = TO_LENGTH( node->Ends[ AI_GREEN_NODE ].vy << 16 );
        pNode->Node[0].Pos.v[2] = TO_LENGTH( node->Ends[ AI_GREEN_NODE ].vz << 16 );
#ifndef _PSX
        pNode->Node[1].Speed    = 30;
#endif
        pNode->Node[1].Pos.v[0] = TO_LENGTH( node->Ends[ AI_RED_NODE ].vx << 16 );
        pNode->Node[1].Pos.v[1] = TO_LENGTH( node->Ends[ AI_RED_NODE ].vy << 16 );
        pNode->Node[1].Pos.v[2] = TO_LENGTH( node->Ends[ AI_RED_NODE ].vz << 16 );
        pNode->Priority         = node->Type;
        pNode->StartNode        = 1;
        pNode->RacingLine       = 65535 - node->RacingLine;
        pNode->RacingLineSpeed  = 30;
        pNode->CentreSpeed      = 30;
        pNode->FinishDist       = 0;
        pNode->OvertakingLine   = 49152;
        pNode->link.flags       = 0;
    
        pNode->Centre.v[X] = pNode->Node[0].Pos.v[X] + MulScalar((pNode->Node[1].Pos.v[X] - pNode->Node[0].Pos.v[X]), pNode->RacingLine);
        pNode->Centre.v[Y] = pNode->Node[0].Pos.v[Y] + MulScalar((pNode->Node[1].Pos.v[Y] - pNode->Node[0].Pos.v[Y]), pNode->RacingLine);
        pNode->Centre.v[Z] = pNode->Node[0].Pos.v[Z] + MulScalar((pNode->Node[1].Pos.v[Z] - pNode->Node[0].Pos.v[Z]), pNode->RacingLine);

        pNode->Prev[0] = AiNode + ((j + (TE_AINodesPlaced - 1)) % TE_AINodesPlaced);
        pNode->Prev[1] = NULL;
        pNode->Next[0] = AiNode + ((j + 1) % TE_AINodesPlaced);
        pNode->Next[1] = NULL;

        if (pNode->Next[1])
            pNode->ZoneID = -1;
        else
            pNode->ZoneID = LONG_MAX;

        pNode++;
        j++;
    }

// load start node

    AiStartNode = 1;

// load total dist

    AiNodeTotalDist = LapDistance;

// Post Process
    PostLoadProcessAiNodes();

}

#endif



#endif

#ifdef _N64
void LoadAiNodes(void)
{
    long            ii, jj, kk, numread;
    FIL             *fp;
    FILE_AINODE     fan;
    long            Prev, Next;

// open ai node file

    AiNode = NULL;
    AiNodeNum = 0;
    AiSingleNodeNum = 0;
    AiLinkNodeNum = 0;

    printf("Loading AI nodes...\n");
    fp = FFS_Open(FFS_TYPE_TRACK | (GameSettings.Reversed ? TRK_REV_AINODES : TRK_AINODES ) );
    if (!fp)
    {
        printf("WARNING: AI node file not found!\n");
        return;
    }

// alloc ram
    FFS_Read(&AiSingleNodeNum, sizeof(AiSingleNodeNum), fp);
    FFS_Read(&AiLinkNodeNum, sizeof(AiLinkNodeNum), fp);
    AiSingleNodeNum = EndConvShort(AiSingleNodeNum);
    AiLinkNodeNum = EndConvShort(AiLinkNodeNum);
    AiNodeNum = AiSingleNodeNum + AiLinkNodeNum;
    if (!AiNodeNum){
        FFS_Close(fp);  
        return;
    }
    AiNode = (AINODE*)malloc(sizeof(AINODE) * AiNodeNum);
    if (!AiNode)
    {
        FFS_Close(fp);  
        AiNodeNum = 0;
        return;
    }

// loop thru all ai nodes

//  for (ii = 0; ii < AiNodeNum ; ii++)
    for (ii = 0; ii < AiSingleNodeNum ; ii++)
    {

// load one file ai node
        BOOL bError = FALSE;
        FFS_Read(&fan, sizeof(fan), fp);
        MyEndConvReal(&fan.RacingLine);
        fan.RacingLineSpeed = EndConvLong(fan.RacingLineSpeed);
        fan.CentreSpeed = EndConvLong(fan.CentreSpeed);
        MyEndConvReal(&fan.FinishDist);
        MyEndConvReal(&fan.OvertakingLine);

            
        for (jj = 0; jj < MAX_AINODE_LINKS; jj++)
        {
            fan.Prev[jj] = EndConvLong(fan.Prev[jj]);
            fan.Next[jj] = EndConvLong(fan.Next[jj]);
            bError |= (fan.Prev[jj] < -1) || (fan.Prev[jj] >= AiNodeNum) || (fan.Next[jj] < -1)  || (fan.Next[jj] >= AiNodeNum);
            if (bError) { printf ("WARNING: Bad Data in Node %d... exiting\n",ii); break; }
        }
        if ( bError ) break;

        fan.Node[0].Speed = EndConvLong(fan.Node[0].Speed);
        MyEndConvReal(&fan.Node[0].Pos.v[0]);
        MyEndConvReal(&fan.Node[0].Pos.v[1]);
        MyEndConvReal(&fan.Node[0].Pos.v[2]);
        fan.Node[1].Speed = EndConvLong(fan.Node[1].Speed);
        MyEndConvReal(&fan.Node[1].Pos.v[0]);
        MyEndConvReal(&fan.Node[1].Pos.v[1]);
        MyEndConvReal(&fan.Node[1].Pos.v[2]);

// setup ai node
        AiNode[ii].Node[0].Pos = fan.Node[1].Pos;
        AiNode[ii].Node[0].Speed = fan.Node[1].Speed;
        AiNode[ii].Node[1].Pos = fan.Node[0].Pos;
        AiNode[ii].Node[1].Speed = fan.Node[0].Speed;

        AiNode[ii].Priority = fan.Priority;
        AiNode[ii].StartNode = fan.StartNode;
        AiNode[ii].RacingLine = fan.RacingLine;
        AiNode[ii].RacingLineSpeed = fan.RacingLineSpeed;
        AiNode[ii].CentreSpeed = fan.CentreSpeed;
        AiNode[ii].FinishDist = fan.FinishDist;
        AiNode[ii].OvertakingLine = fan.OvertakingLine;
//      AiNode[ii].linkInfo[0].flags    = fan.flags[0];
//      AiNode[ii].linkInfo[1].flags    = fan.flags[1];
        AiNode[ii].link.flags   = fan.flags[0];


        AiNode[ii].Centre.v[X] = AiNode[ii].Node[0].Pos.v[X] + MulScalar(( AiNode[ii].Node[1].Pos.v[X] -  AiNode[ii].Node[0].Pos.v[X]),  AiNode[ii].RacingLine);
        AiNode[ii].Centre.v[Y] = AiNode[ii].Node[0].Pos.v[Y] + MulScalar(( AiNode[ii].Node[1].Pos.v[Y] -  AiNode[ii].Node[0].Pos.v[Y]),  AiNode[ii].RacingLine);
        AiNode[ii].Centre.v[Z] = AiNode[ii].Node[0].Pos.v[Z] + MulScalar(( AiNode[ii].Node[1].Pos.v[Z] -  AiNode[ii].Node[0].Pos.v[Z]),  AiNode[ii].RacingLine);

        for (jj = 0; jj < MAX_AINODE_LINKS; jj++)
        {
            if (fan.Prev[jj] != -1)
                AiNode[ii].Prev[jj] = AiNode + fan.Prev[jj];
            else
                AiNode[ii].Prev[jj] = NULL;

            if (fan.Next[jj] != -1)
                AiNode[ii].Next[jj] = AiNode + fan.Next[jj];
            else
                AiNode[ii].Next[jj] = NULL;
        }

        if (AiNode[ii].Next[1])
            AiNode[ii].ZoneID = -1;
        else
            AiNode[ii].ZoneID = LONG_MAX;
    }

    log_printf("...read %d AI nodes.\n", AiNodeNum);

// load start node
    FFS_Read(&AiStartNode, sizeof(AiStartNode), fp);
    AiStartNode = EndConvLong(AiStartNode);

// load total dist
    FFS_Read(&AiNodeTotalDist, sizeof(AiNodeTotalDist), fp);
    MyEndConvReal(&AiNodeTotalDist);

// close ai node file
    FFS_Close(fp);


// Post Process
    PostLoadProcessAiNodes();
}
#endif

///////////////////
// free ai nodes //
///////////////////

void FreeAiNodes(void)
{
    free(AiNode);
    free(AiNodeZone);

    AiNode = NULL;
    AiNodeZone = NULL;
}


/********************************************************************************
* SetupNewMethodLinkNodes()
********************************************************************************/
void SetupNewMethodLinkNodes(void)
{
    AINODE  *pNode, *pNodeNew;
    int     i;
    int     cLink;

// Is this the new format ?
#if 0
    if (!AiLinkNodeNum)
    {
        #ifdef _PC
        if (Version == VERSION_DEV)
        {
            DumpMessage("Old AI Node File!!!", "Please save the AI Nodes & you all will be pukka.");
        }
        #else 
        #ifdef _N64
        printf("WARNING: Old AI Node File!!! \n");
        #endif //_N64
        #endif //_PC

        return;
    }
#else
    if (!AiLinkNodeNum)
        return;
#endif


// Re-organize nodes
    pNode = AiNode;
    pNodeNew = AiNode + AiSingleNodeNum;
    for (i = 0 ; i < AiSingleNodeNum ; i++, pNode++)
    {
    // Does this node have a double link ?
        if ((pNode->ZoneID == -1) && (pNode->Next[1]))
        {
            pNode->ZoneID = LONG_MAX;
            *pNodeNew = *pNode;

            pNodeNew->Next[0] = pNode->Next[1];
            pNodeNew->Next[1] = pNode->Next[1] = NULL;

            for (cLink = 0; cLink < MAX_AINODE_LINKS; cLink++)
            {
                if (pNodeNew->Next[0]->Prev[cLink] == pNode)
                    pNodeNew->Next[0]->Prev[cLink] = pNodeNew;

                // Setup links from previous node
                if ((pNode->Prev[cLink]) && (pNode->Prev[cLink]->Next[0] == pNode))
                {
                    Assert(!pNode->Prev[cLink]->Next[1]);
                    pNode->Prev[cLink]->Next[1] = pNodeNew;
                    pNode->Prev[cLink]->ZoneID = LONG_MAX;
                }
            }

            pNodeNew->Priority = pNodeNew->Next[0]->Priority;
            pNode->Priority = pNode->Next[0]->Priority;

            // Move to next new node
            pNodeNew++;
        }
    }
}

/********************************************************************************
* PostLoadProcessAiNodes()
********************************************************************************/
void PostLoadProcessAiNodes(void)
{
    AINODE      *pNode;
    AINODE      *pNodeN;
    VEC         rVec, uVec, tVec;
    REAL        dist[2];
    int         i, j, cNode;
    int         convexFlag = 0;
#ifdef _PSX
    VEC         fVec;
#endif

// Setup link nodes - New method
    SetupNewMethodLinkNodes();

// Create node linkinfo data
    pNode = AiNode;
    for (i = 0 ; i < AiNodeNum ; i++, pNode++)
    {
        // Set default overtaking line
#if 0
        if (pNode->RacingLine > Real(0.5))
            pNode->OvertakingLine = MulScalar(pNode->RacingLine, Real(0.5));
        else
            pNode->OvertakingLine = pNode->RacingLine + MulScalar(ONE - pNode->RacingLine, Real(0.5));
#endif

//      for (j = 0 ; j < MAX_AINODE_LINKS ; j++)
        j = 0;
        {
            if (pNodeN = pNode->Next[j])
            {
                // Setup node bounds
                pNode->link.boundsMin[X] = FLT_MAX;
                pNode->link.boundsMin[Y] = FLT_MAX;
                pNode->link.boundsMax[X] = -FLT_MAX;
                pNode->link.boundsMax[Y] = -FLT_MAX;

                for (cNode = 0; cNode < 2; cNode++)
                {
                    if (pNode->link.boundsMin[X] > pNode->Node[cNode].Pos.v[X])
                        pNode->link.boundsMin[X] = pNode->Node[cNode].Pos.v[X];
                    if (pNode->link.boundsMax[X] < pNode->Node[cNode].Pos.v[X])
                        pNode->link.boundsMax[X] = pNode->Node[cNode].Pos.v[X];

                    if (pNode->link.boundsMin[Y] > pNode->Node[cNode].Pos.v[Z])
                        pNode->link.boundsMin[Y] = pNode->Node[cNode].Pos.v[Z];
                    if (pNode->link.boundsMax[Y] < pNode->Node[cNode].Pos.v[Z])
                        pNode->link.boundsMax[Y] = pNode->Node[cNode].Pos.v[Z];

                    if (pNode->link.boundsMin[X] > pNodeN->Node[cNode].Pos.v[X])
                        pNode->link.boundsMin[X] = pNodeN->Node[cNode].Pos.v[X];
                    if (pNode->link.boundsMax[X] < pNodeN->Node[cNode].Pos.v[X])
                        pNode->link.boundsMax[X] = pNodeN->Node[cNode].Pos.v[X];

                    if (pNode->link.boundsMin[Y] > pNodeN->Node[cNode].Pos.v[Z])
                        pNode->link.boundsMin[Y] = pNodeN->Node[cNode].Pos.v[Z];
                    if (pNode->link.boundsMax[Y] < pNodeN->Node[cNode].Pos.v[Z])
                        pNode->link.boundsMax[Y] = pNodeN->Node[cNode].Pos.v[Z];
                }

                // Calc. forward vector & distance to next node
                VecMinusVec(&pNodeN->Centre, &pNode->Centre, &pNode->link.forwardVec);
#ifndef _PSX
                pNode->link.dist = VecLen(&pNode->link.forwardVec);
#else
                tVec.v[0] = pNode->link.forwardVec.v[X] >> 4;
                tVec.v[1] = pNode->link.forwardVec.v[Y] >> 4;
                tVec.v[2] = pNode->link.forwardVec.v[Z] >> 4;
                pNode->link.dist = VecLen(&tVec) << 4;
#endif
#ifdef _N64//A>S>
                if(!pNode->link.dist) continue;
#endif

#ifndef _PSX
                pNode->link.invDist = DivScalar(Real(1), pNode->link.dist);
#endif

                pNode->link.forwardVec.v[Y] = 0;
#ifdef _PSX
                pNode->link.forwardVec.v[X] >>= 4;
                pNode->link.forwardVec.v[Y] >>= 4;
                pNode->link.forwardVec.v[Z] >>= 4;
#endif
                NormalizeVector(&pNode->link.forwardVec);

                // Calc. right vector
                VecMinusVec(&pNode->Node[1].Pos, &pNode->Node[0].Pos, &rVec);
                    NormalizeVector(&rVec);

                VecCrossVec(&pNode->link.forwardVec, &rVec, &uVec);
                    NormalizeVector(&uVec);
                VecCrossVec(&uVec, &pNode->link.forwardVec, &pNode->link.rightVec);
                NormalizeVector(&pNode->link.rightVec);

//              pNode->linkInfo[j].rightVec.v[X] = pNode->Node[1].Pos.v[X] - pNode->Node[0].Pos.v[X];
//              pNode->linkInfo[j].rightVec.v[Y] = pNode->Node[1].Pos.v[Y] - pNode->Node[0].Pos.v[Y];
//              pNode->linkInfo[j].rightVec.v[Z] = pNode->Node[1].Pos.v[Z] - pNode->Node[0].Pos.v[Z];
//              NormalizeVector(&pNode->linkInfo[j].rightVec);

            // Create plane equations for left/right edges of node box
#ifndef _PSX
                uVec.v[X] = pNode->Node[0].Pos.v[X];
                uVec.v[Y] = pNode->Node[0].Pos.v[Y] - Real(64);
                uVec.v[Z] = pNode->Node[0].Pos.v[Z];
///             BuildPlane(&pNode->Node[0].Pos, &uVec, &pNodeN->Node[0].Pos, &pNode->Node[0].plane);
//              BuildPlane(&pNode->Node[0].Pos, &pNodeN->Node[0].Pos, &uVec, &pNode->Node[0].plane);
                BuildPlane(&pNode->Node[0].Pos, &pNodeN->Node[0].Pos, &uVec, &pNode->link.planeEdge[0]);

                uVec.v[X] = pNode->Node[1].Pos.v[X];
                uVec.v[Y] = pNode->Node[1].Pos.v[Y] - Real(64);
                uVec.v[Z] = pNode->Node[1].Pos.v[Z];
//              BuildPlane(&pNode->Node[1].Pos, &uVec, &pNodeN->Node[1].Pos, &pNode->Node[1].plane);
                BuildPlane(&pNode->Node[1].Pos, &uVec, &pNodeN->Node[1].Pos, &pNode->link.planeEdge[1]);
#else
                VecMinusVec(&pNodeN->Node[0].Pos, &pNode->Node[0].Pos, &fVec);
                fVec.v[X] >>= 4;
                fVec.v[Y] >>= 4;
                fVec.v[Z] >>= 4;
                VecPlusEqVec(&fVec, &pNode->Node[0].Pos);
                uVec.v[X] = pNode->Node[0].Pos.v[X];
                uVec.v[Y] = pNode->Node[0].Pos.v[Y] - Real(64);
                uVec.v[Z] = pNode->Node[0].Pos.v[Z];
                BuildPlane(&pNode->Node[0].Pos, &fVec, &uVec, &pNode->link.planeEdge[0]);

                VecMinusVec(&pNodeN->Node[1].Pos, &pNode->Node[1].Pos, &fVec);
                fVec.v[X] >>= 4;
                fVec.v[Y] >>= 4;
                fVec.v[Z] >>= 4;
                VecPlusEqVec(&fVec, &pNode->Node[1].Pos);
                uVec.v[X] = pNode->Node[1].Pos.v[X];
                uVec.v[Y] = pNode->Node[1].Pos.v[Y] - Real(64);
                uVec.v[Z] = pNode->Node[1].Pos.v[Z];
                BuildPlane(&pNode->Node[1].Pos, &uVec, &fVec, &pNode->link.planeEdge[1]);
#endif
            }
            else
            {
                pNode->link.dist = Real(0);
            }

            // Calc. width of track at node
            VecMinusVec(&pNode->Node[1].Pos, &pNode->Node[0].Pos, &tVec);
            pNode->trackWidth = VecLen(&tVec);
            pNode->trackWidthL = MulScalar(pNode->trackWidth, pNode->RacingLine);
            pNode->trackWidthR = pNode->trackWidth - pNode->trackWidthL;

            pNode->trackHotDist[1] = MulScalar(Real(0.25), pNode->trackWidth);
            pNode->trackHotDist[0] = -pNode->trackHotDist[1];

            dist[0] = -pNode->trackWidthL + TO_LENGTH(Real(200));
            if (dist[0] > Real(0.0))
                dist[0] = Real(0.0);
            dist[1] = pNode->trackWidthR - TO_LENGTH(Real(200));
            if (dist[1] < Real(0.0))
                dist[1] = Real(0.0);

            if (pNode->trackHotDist[0] < dist[0])
                pNode->trackHotDist[0] = dist[0];
            if (pNode->trackHotDist[1] > dist[1])
                pNode->trackHotDist[1] = dist[1];
        }
    }


// Search for Jank nodes
#ifdef _PC
#if 1
    pNode = AiNode;
    for (i = 0 ; i < AiNodeNum ; i++, pNode++)
    {
        REAL    d[4];
        int     s[4];
        int     sum;

        if (pNodeN = pNode->Next[0])
        {
            d[0] = VecDotPlane(&pNode->Node[0].Pos, &pNode->link.planeEdge[1]);
            d[1] = VecDotPlane(&pNodeN->Node[0].Pos, &pNode->link.planeEdge[1]);
            d[2] = VecDotPlane(&pNode->Node[1].Pos, &pNode->link.planeEdge[0]);
            d[3] = VecDotPlane(&pNodeN->Node[1].Pos, &pNode->link.planeEdge[0]);
            s[0] = Sign(d[0]);
            s[1] = Sign(d[1]);
            s[2] = Sign(d[2]);
            s[3] = Sign(d[3]);
            sum = s[0] + s[1] + s[2] + s[3];
#if 0
    //      if (Sign(dist[0]) != Sign(dist[1]))
            if ((sum > -4) && (sum < 4))
            {
                char string[256];
                if (convexFlag == 0)
                {
                    convexFlag = 1;
//                      sprintf(string, "AI Node %d (in Zone %d) is NON-CONVEX !!!!", i,pNode->ZoneID);
                    sprintf(string, "You have Jank NON-CONVEX  Nodes !!!!  Look for the big flashy nodes & sort it out!");
                    DumpMessage(NULL, string);
                }
            }
#endif
        }
    }
#endif
#endif
}


/********************************************************************************
* ZoneAiNodes()
*
* zone up ai node
********************************************************************************/
#define PSX_ZONE_SHIFT      2

void ZoneAiNodes(void)
{
    AINODE  *pNode, *pNodeN, *pNodeP;
    AIZONE  *pZone;
    AIZONE  *pZonesInside[10];
    int     iZonesInside[10];
    int     cZonesInside;
    int     cNode, cZone;

// Quit if no zones or nodes
    if ((!AiZones) || (!AiNode))
        return;


// Find which zones the node is in
    for (pNode = AiNode, cNode = 0; cNode < AiNodeNum; cNode++, pNode++)
    {
        pNode->ZoneID = LONG_MAX;

        for (cZone = 0; cZone < MAX_AINODE_LINKS; cZone++)
            pNode->CheckNext[cZone] = FALSE;

        cZonesInside = AIN_IsNodeInsideZones(pNode, pZonesInside, iZonesInside);

        // If the node was only in one zone, it is valid
        if (cZonesInside == 1)
        {
            pNode->ZoneID = iZonesInside[0];
        }
    }


// Setup nodes that are inside multiple zones
    for (pNode = AiNode, cNode = 0; cNode < AiNodeNum; cNode++, pNode++)
    {
        if (pNode->ZoneID == LONG_MAX)
        {
//          pNode->CheckNext[0] = TRUE;

            pNodeN = AIN_FindFirstValidNodeIDN(pNode);
            pNodeP = AIN_FindFirstValidNodeIDP(pNode);

            if (pNodeN && pNodeP)
            {
                cZonesInside = AIN_IsNodeInsideZones(pNode, pZonesInside, iZonesInside);

                for (cZone = 0; cZone < cZonesInside; cZone++)
                {
                    if ((iZonesInside[cZone] >= pNodeP->ZoneID) && (iZonesInside[cZone] <= pNodeN->ZoneID))
                    {
                        if ((pNode->ZoneID == LONG_MAX) || (pNode->ZoneID < iZonesInside[cZone]))
                            pNode->ZoneID = iZonesInside[cZone];
                    }
                }
            }
        }

        if (pNode->ZoneID == LONG_MAX)
        {
            for (cZone = 0; cZone < cZonesInside; cZone++)
            {
                if (pNode->ZoneID > iZonesInside[cZone])
                    pNode->ZoneID = iZonesInside[cZone];
            }
        }
    }


// Setup nodes CheckNext vars
#if 0
    for (pNode = AiNode, cNode = 0; cNode < AiNodeNum; cNode++, pNode++)
    {
        if (pNode->CheckNext[0])
        {
            pNode->CheckNext[0] = FALSE;

            for (cZone = 0; cZone < MAX_AINODE_LINKS; cZone++)
            {
                if (pNode->Next[cZone])
                {
                    if (pNode->Next[cZone]->ZoneID != pNode->ZoneID)
                        pNode->CheckNext[cZone] = TRUE;
                }
            }
        }
    }
#endif


// Setup zone headers
    for (pZone = AiZones, cZone = 0; cZone < AiZoneNum; cZone++, pZone++)
    {
        pZone->Count = 0;
        pZone->FirstNode = NULL;

        pNodeP = NULL;
        for (pNode = AiNode, cNode = 0; cNode < AiNodeNum; cNode++, pNode++)
        {
            if (pNode->ZoneID == cZone)
            {
                if (!pZone->FirstNode)
                    pZone->FirstNode = pNode;

                if (pNodeP)
                    pNodeP->ZoneNext = pNode;

                pNode->ZonePrev = pNodeP;
                pNode->ZoneNext = NULL;

                pNodeP = pNode;

                pZone->Count++;
            }
        }
    }

// Check nodes are valid
#if 0//def _PC
    for (pNode = AiNode, cNode = 0 ; cNode < AiNodeNum ; cNode++, pNode++)
    {
        if ((pNode->Next[0] == NULL) && (pNode->Next[1] == NULL) && Version == VERSION_DEV)
        {
            char string[256];
            sprintf(string, "AI Node %d (in Zone %d) has no forward links !!!!", cNode,pNode->ZoneID);
            DumpMessage(NULL, string);
        }
    }
#endif
}


/********************************************************************************
* AIN_IsNodeInsideZones
********************************************************************************/
int AIN_IsNodeInsideZones(AINODE *pNode, AIZONE **ppZonesInside, int *piZonesInside)
{
    AIZONE  *pZone;
    int     cZonesInside;
    int     cZone, cPlane;
    REAL    dist;
#ifdef _PSX
    VEC     tmpPos;
    PLANE   tmpPlane;
#endif

    cZonesInside = 0;

    for (pZone = AiZones, cZone = 0; cZone < AiZoneNum; cZone++, pZone++)
    {
        // Check if node is inside the zones bounds
        for (cPlane = 0; cPlane < 3; cPlane++)
        {
#ifndef _PSX
            dist = PlaneDist(&pZone->Plane[cPlane], &pNode->Centre);
            if ( (dist < -pZone->Size[cPlane]) || (dist > pZone->Size[cPlane]) )
                break;
#else
            tmpPos.v[0] = pNode->Centre.v[0] >> PSX_ZONE_SHIFT;
            tmpPos.v[1] = pNode->Centre.v[1] >> PSX_ZONE_SHIFT;
            tmpPos.v[2] = pNode->Centre.v[2] >> PSX_ZONE_SHIFT;
            tmpPlane.v[0] = pZone->Plane[cPlane].v[0];
            tmpPlane.v[1] = pZone->Plane[cPlane].v[1];
            tmpPlane.v[2] = pZone->Plane[cPlane].v[2];
            tmpPlane.v[3] = pZone->Plane[cPlane].v[3] >> PSX_ZONE_SHIFT;
            dist = PlaneDist(&tmpPlane, &tmpPos);
            if ( (dist < (-pZone->Size[cPlane] >> PSX_ZONE_SHIFT)) || (dist > (pZone->Size[cPlane] >> PSX_ZONE_SHIFT)) )
                break;
#endif
        }

        // Is the node inside the zone ?
        if (cPlane == 3)
        {
            ppZonesInside[cZonesInside] = pZone;
            piZonesInside[cZonesInside] = cZone;
            cZonesInside++;
        }
    }

    return cZonesInside;
}


/********************************************************************************
* AIN_FindFirstValidNodeIDN
********************************************************************************/
AINODE *AIN_FindFirstValidNodeIDN(AINODE *pNode)
{
    AINODE  *pNodeFound;
    int     i;

//#ifdef AIN_RECURSIVE_SEARCH
#ifndef _PSX

// Is node's zone ID valid
    if (pNode->ZoneID != LONG_MAX)
        return pNode;

// Check link ?
    for (i = 0; i < MAX_AINODE_LINKS; i++)
    {
        if (pNode->Next[i])
        {
            if (pNodeFound = AIN_FindFirstValidNodeIDN(pNode->Next[i]))
                return pNodeFound;
        }
    }

#else
    long    dist, flag;

    dist = pNode->FinishDist;
    flag = 0;
    pNodeFound = pNode;
    while (1)
    {
    // Is node's zone ID valid
        if (pNodeFound->ZoneID != LONG_MAX)
            return pNodeFound;

        pNodeFound = pNodeFound->Next[0];

            #if 0
        if (!flag)
        {
            if (dist < pNode->FinishDist)
                flag = 1;
        }
        else
        {
            if (dist >= pNode->FinishDist)
                break;
        }
            #endif

        if ((!pNodeFound) || (pNode == pNodeFound))
            break;
    }

#endif

// None found
    return NULL;
}


/********************************************************************************
* AIN_FindFirstValidNodeIDP
********************************************************************************/
AINODE *AIN_FindFirstValidNodeIDP(AINODE *pNode)
{
    AINODE  *pNodeFound;
    int     i;

//#ifdef AIN_RECURSIVE_SEARCH
#ifndef _PSX

// Is node's zone ID valid
    if (pNode->ZoneID != LONG_MAX)
        return pNode;

// Check link ?
    for (i = 0; i < MAX_AINODE_LINKS; i++)
    {
        if (pNode->Prev[i])
        {
            if (pNodeFound = AIN_FindFirstValidNodeIDP(pNode->Prev[i]))
                return pNodeFound;
        }
    }

#else
    long    dist, flag;

    dist = pNode->FinishDist;
    flag = 0;
    pNodeFound = pNode;
    while (1)
    {
    // Is node's zone ID valid
        if (pNodeFound->ZoneID != LONG_MAX)
            return pNodeFound;

        pNodeFound = pNodeFound->Prev[0];

            #if 0
        if (!flag)
        {
            if (dist > pNode->FinishDist)
                flag = 1;
        }
        else
        {
            if (dist <= pNode->FinishDist)
                break;
        }
            #endif

        if ((!pNodeFound) || (pNode == pNodeFound))
            break;
    }

#endif

// None found
    return NULL;
}

//--------------------------------------------------------------------------------------------------------------------------
// AIN_NearestNode
//
// Locates the nearest node to the specified car, biased to the x and z plane, with nearest y.
// The car's grid position is established and all of the nodes in that grid location are scanned against the car centre
//  to find the nearest. 
//

AINODE *AIN_NearestNode(PLAYER *Player, REAL *Dist)
{
    long        ii, kk;
    AINODE      *NearNode = NULL;
    VEC         CarPos;
    VEC         temp;
    AINODE      *Node;
    AIZONE      *Zone;
    REAL        tDist, Nearest;
    CAR         *Car;

    if (Player->CarAI.CurZone == -1)
        return NULL;

    Car = &Player->car;
#ifndef _PSX
    Nearest = FLT_MAX;
#else 
    Nearest = 0x7FFFFFFF;
#endif

    CarPos = Car->Body->Centre.Pos;
    Zone = &AiZones[Player->CarAI.CurZone];
    Node = Zone->FirstNode;

    for (ii = 0; ii < Zone->Count; ii++, Node = Node->ZoneNext)
    {
        SubVector(&Node->Centre, &CarPos, &temp);
#ifdef _PSX
        temp.v[0] = PSX_LENGTH(temp.v[0]);
        temp.v[1] = PSX_LENGTH(temp.v[1]);
        temp.v[2] = PSX_LENGTH(temp.v[2]);
#endif
        tDist = VecLen2(&temp);

        if (tDist < Nearest)
        {
            Nearest = tDist;
            NearNode = Node;
        }

        for (kk = 0; kk < MAX_AINODE_LINKS; kk++)
        {
            if (Node->CheckNext[kk])
            {
                SubVector(&Node->Next[kk]->Centre, &CarPos, &temp);
#ifdef _PSX
                temp.v[0] = PSX_LENGTH(temp.v[0]);
                temp.v[1] = PSX_LENGTH(temp.v[1]);
                temp.v[2] = PSX_LENGTH(temp.v[2]);
#endif
                tDist = VecLen2(&temp);

                if (tDist < Nearest)
                {
                    Nearest = tDist;
                    NearNode = Node->Next[kk];
                }
            }
        }
    }
    
//  *Dist = Nearest;
    *Dist = (REAL)sqrt(Nearest);
    return(NearNode);
}

//--------------------------------------------------------------------------------------------------------------------------

//
// AIN_GetForwardNode
//
// Returns node "in front of" the specified car's current position at a distance greater than MinDist
//
#if 0
AINODE *AIN_GetForwardNode(PLAYER *Player, REAL MinDist, REAL *Dist)
{

    
    AINODE  *NearNode;
//  AINODE NearNodeFwd;
    REAL    Nearest, NearestFwd; 
    REAL    dp;
    long    RChoice;
    VEC     Vec1, Vec2;
    VEC     CarPos;
    REAL    minDist2;               // Min distance sqaured

#ifndef _PSX
    minDist2 = MinDist * MinDist;
#else
    minDist2 = Int(MinDist);
    minDist2 *= minDist2;
#endif

    NearNode = AIN_NearestNode(Player, &Nearest);
    if (!NearNode)
        return (NULL);

//  return(NearNode);       // ????????????????? GAZZA

    CarPos = Player->car.Body->Centre.Pos;
    RChoice = Player->CarAI.RouteChoice;
    
    NearestFwd = -1;

//  while(NearestFwd < MinDist)
    while(NearestFwd < minDist2)
    {
        if (!NearNode->Next[RChoice])
        {
//          *Dist = Nearest;
            *Dist = (REAL)sqrt(Nearest);
            return(NearNode);
        }

        Vec1.v[X] = NearNode->Centre.v[X] - CarPos.v[X];
        Vec1.v[Y] = NearNode->Centre.v[Y] - CarPos.v[Y];
        Vec1.v[Z] = NearNode->Centre.v[Z] - CarPos.v[Z];
        Vec2.v[X] = NearNode->Next[RChoice]->Centre.v[X] - NearNode->Centre.v[X];
        Vec2.v[Y] = NearNode->Next[RChoice]->Centre.v[Y] - NearNode->Centre.v[Y];
        Vec2.v[Z] = NearNode->Next[RChoice]->Centre.v[Z] - NearNode->Centre.v[Z];
    
        dp = DotProduct(&Vec1, &Vec2);
        if (dp > Real(0.5)) 
        {
            Vec2.v[X] = NearNode->Next[RChoice]->Centre.v[X] - CarPos.v[X];
            Vec2.v[Y] = NearNode->Next[RChoice]->Centre.v[Y] - CarPos.v[Y];
            Vec2.v[Z] = NearNode->Next[RChoice]->Centre.v[Z] - CarPos.v[Z];
//          NearestFwd = abs(VecLen(&Vec2)); 
            NearestFwd = VecLen2(&Vec2); 
        }

        NearNode = NearNode->Next[RChoice];
    }

//  *Dist = NearestFwd;
    *Dist = (REAL)sqrt(NearestFwd);
    return(NearNode->Next[RChoice]);

    

}
#endif

/********************************************************************************
* AINODE *AIN_GetForwardNodeInRange(PLAYER *Player, REAL MaxDist, REAL *Dist)
********************************************************************************/
AINODE *AIN_GetForwardNodeInRange(PLAYER *Player, REAL MaxDist, REAL *Dist)
{
    AINODE  *pNode;
    AINODE  *pNodeN;
    REAL    maxDist2;               // Max distance squared

// Is the car out of nodes ?
    if (!(pNode = Player->CarAI.pCurNode))
    {
    // Is the last valid node valid ?
        if (!(pNode = Player->CarAI.pLastValidNode))
            return NULL;

        MaxDist += MaxDist + MaxDist;   // Extend search distance
    }

    maxDist2 = MaxDist;
    if (pNodeN = AIN_IsPlayerInNodeRecurseN(Player, pNode, Player->CarAI.iRouteCurNode, &maxDist2))
        return pNodeN;

    maxDist2 = MaxDist;
    if (pNodeN = AIN_IsPlayerInNodeRecurseP(Player, pNode, Player->CarAI.iRouteCurNode, &maxDist2))
        return pNodeN;

    return NULL;
}


/********************************************************************************
*
********************************************************************************/

/////////////////////////////
// Track Next Nodes
AINODE *AIN_IsPlayerInNodeRecurseN(PLAYER *pPlayer, AINODE *pNode, long iRoute, REAL *pDist)
{
#if 0
    AINODE  *pNodeFound;

// Is point in node ?
    iRoute = CAI_ChooseNodeRoute(pPlayer, pNode);
    if (AIN_IsPointInNodeBounds(&pPlayer->car.Body->Centre.Pos, pNode, iRoute))
        return pNode;

// Continue searching ?
    if (*pDist > Real(0))
    {
    // Is point in link node route #1 ?
        iRoute = CAI_ChooseNodeRoute(pPlayer, pNode);
        //if (pNode->Next[iRoute])
        {
            *pDist -= pNode->link.dist;
            if (pNodeFound = AIN_IsPlayerInNodeRecurseN(pPlayer, pNode->Next[iRoute], iRoute, pDist))
                return pNodeFound;
            *pDist += pNode->link.dist;
        }
    }

// Not in these nodes
    return NULL;
#else
    while (pNode && (*pDist > Real(0)))
    {
    // Is point in node ?
        iRoute = CAI_ChooseNodeRoute(pPlayer, pNode);
        if (AIN_IsPointInNodeBounds(&pPlayer->car.Body->Centre.Pos, pNode, iRoute))
            return pNode;

    // Is point in link node route #1 ?
        iRoute = CAI_ChooseNodeRoute(pPlayer, pNode);

        *pDist -= pNode->link.dist;
        if (pNode->Next[iRoute])
            pNode = pNode->Next[iRoute];
        else
            pNode = pNode->Next[!iRoute];
    }

// Not in these nodes
    return NULL;
#endif
}

/////////////////////////////
// Track Previous Nodes
AINODE *AIN_IsPlayerInNodeRecurseP(PLAYER *pPlayer, AINODE *pNode, long iRoute, REAL *pDist)
{
//#ifndef _PSX
#if 0
    AINODE  *pNodeFound;

// Is point in node ?
    if (AIN_IsPointInNodeBounds(&pPlayer->car.Body->Centre.Pos, pNode, iRoute))
        return pNode;

// Continue searching ?
    if (*pDist > Real(0))
    {
    // Is point in link node route #1 ?
        if (pNode->Prev[iRoute])
        {
//          *pDist -= pNode->link.dist;
            *pDist -= pNode->Prev[iRoute]->link.dist;
            if (pNodeFound = AIN_IsPlayerInNodeRecurseP(pPlayer, pNode->Prev[iRoute], iRoute, pDist))
                return pNodeFound;
//          *pDist += pNode->link.dist;
            *pDist += pNode->Prev[iRoute]->link.dist;
        }

    // Is point in link node route #2 ?
        if (pNode->Prev[!iRoute])
        {
//          *pDist -= pNode->link.dist;
            *pDist -= pNode->Prev[!iRoute]->link.dist;
            if (pNodeFound = AIN_IsPlayerInNodeRecurseP(pPlayer, pNode->Prev[!iRoute], iRoute, pDist))
                return pNodeFound;
//          *pDist += pNode->link.dist;
            *pDist += pNode->Prev[!iRoute]->link.dist;
        }
    }

// Not in these nodes
    return NULL;
#else

// Continue searching ?
    while (pNode && (*pDist > Real(0)))
    {
    // Is point in node ?
        if (AIN_IsPointInNodeBounds(&pPlayer->car.Body->Centre.Pos, pNode, iRoute))
            return pNode;

    // Is point in link node route #1 ?
        if (pNode->Prev[iRoute])
        {
            pNode = pNode->Prev[iRoute];
            *pDist -= pNode->link.dist;
        }
        else
        if (pNode->Prev[!iRoute])
        {
            pNode = pNode->Prev[!iRoute];
            *pDist -= pNode->link.dist;
        }
        else
            return NULL;
    }

// Not in these nodes
    return NULL;
#endif
}


/********************************************************************************
* AINODE *AIN_FindNodeAhead(PLAYER *pPlayer, int iRoute, REAL dist)
********************************************************************************/
AINODE *AIN_FindNodeAhead(PLAYER *pPlayer, REAL dist, VEC *pPos)
{
    AINODE  *pNode, *pNodeN, *pNodeS;
    int     iRoute, iRouteN, iRouteP;
    VEC     center[2];
    REAL    a,b,t;

    iRoute = pPlayer->CarAI.iRouteCurNode;

// Get current node
    if (!(pNode = pPlayer->CarAI.pCurNode))
    {
    // Get last valid node
        if (!(pNode = pPlayer->CarAI.pLastValidNode))
        {
            CopyVec(&pPlayer->car.Body->Centre.Pos, pPos);
            return NULL;
        }
    }
    else
    {
        dist += pPlayer->CarAI.distAlongNode;
    }

    pNodeS = pNode;
    iRouteN = iRoute;
    while (1)
    {
        iRouteP = CAI_ChooseNodeRoute(pPlayer, pNode);
        Assert(pNode->Next[iRouteP] != NULL);   // Shouldn't need this !!!

        iRouteN = iRouteP;
        dist -= pNode->link.dist;
        if (dist < ZERO)
            break;

        // GAZZA NEW
//      if (pNode->Next[iRouteN]->link.flags & (AIN_LF_WALL_LEFT | AIN_LF_WALL_RIGHT))
//          break;

        pNode = pNode->Next[iRouteN];
    }

// Setup position
    pNodeN = pNode->Next[iRouteN];

#if 0
    if (pPlayer->CarAI.bOvertake)
    {
        a = pNode->OvertakingLine;
        b = pNodeN->OvertakingLine;
/*
        if ((pPlayer->CarAI.pNearestCarInFront) && (pPlayer->CarAI.pNearestCarInFront->CarAI.pCurNode))
        {
            if (pPlayer->CarAI.pNearestCarInFront->CarAI.pCurNode->RacingLine < Real(0.5))
            {
                a = pNodeS->RacingLine + Real(0.2);
                if (a > Real(0.8))
                    a = Real(0.8);
                b = a;
            }
            else
            {
                a = pNodeS->RacingLine - Real(0.2);
                if (a < Real(0.2))
                    a = Real(0.2);
                b = a;
            }
        }
        else
        {
            a = pNode->OvertakingLine;
            b = pNodeN->OvertakingLine;
        }
*/
    }
    else
    {
        a = pNode->RacingLine;
        b = pNodeN->RacingLine;
    }
#else
//  a = pPlayer->CarAI.curRacingLine;
//  b = pPlayer->CarAI.curRacingLine;
    a = pNode->RacingLine + MulScalar(pNode->OvertakingLine - pNode->RacingLine, pPlayer->CarAI.curRacingLine);
    b = pNodeN->RacingLine + MulScalar(pNodeN->OvertakingLine - pNodeN->RacingLine, pPlayer->CarAI.curRacingLine);

#endif

    dist += pNode->link.dist;
    if (pNode->link.dist != 0)
        t = DivScalar(dist, pNode->link.dist);
    else
        t = dist;

    InterpVec(&pNode->Node[0].Pos, &pNode->Node[1].Pos, a, &center[0]);
    InterpVec(&pNodeN->Node[0].Pos, &pNodeN->Node[1].Pos, b, &center[1]);
    InterpVec(&center[0], &center[1], t, pPos);

// Return found node
    return pNode;
}


/********************************************************************************
* AINODE *AIN_FindNodeAheadNoWall(PLAYER *pPlayer, int iRoute, REAL dist)
********************************************************************************/
AINODE *AIN_FindNodeAheadNoWall(PLAYER *pPlayer, REAL dist, VEC *pPos)
{
    AINODE  *pNode, *pNodeN;
    int     iRoute, iRouteN;
    VEC     center[2];
    REAL    a,b,t;

    iRoute = pPlayer->CarAI.iRouteCurNode;

// Get current node
    if (!(pNode = pPlayer->CarAI.pCurNode))
    {
    // Get last valid node
        if (!(pNode = pPlayer->CarAI.pLastValidNode))
            return NULL;
    }
    else
    {
        dist += pPlayer->CarAI.distAlongNode;
    }

    while (1)
    {
        iRouteN = CAI_ChooseNodeRoute(pPlayer, pNode);

        dist -= pNode->link.dist;
        if (dist < ZERO)
            break;

        if (pNode->Next[iRouteN]->link.flags & (AIN_LF_WALL_LEFT | AIN_LF_WALL_RIGHT))
            break;

        pNode = pNode->Next[iRouteN];
    }

// Setup position
    pNodeN = pNode->Next[iRouteN];

    if (pPlayer->CarAI.bOvertake)
    {
        a = pNode->OvertakingLine;
        b = pNodeN->OvertakingLine;
    }
    else
    {
        a = pNode->RacingLine;
        b = pNodeN->RacingLine;
    }

    if (pNode->Next[iRouteN]->link.flags & (AIN_LF_WALL_LEFT | AIN_LF_WALL_RIGHT))
    {
        InterpVec(&pNodeN->Node[0].Pos, &pNodeN->Node[1].Pos, b, pPos);
    }
    else
    {
        dist += pNode->link.dist;
        t = DivScalar(dist, pNode->link.dist);

        InterpVec(&pNode->Node[0].Pos, &pNode->Node[1].Pos, a, &center[0]);
        InterpVec(&pNodeN->Node[0].Pos, &pNodeN->Node[1].Pos, b, &center[1]);
        InterpVec(&center[0], &center[1], t, pPos);
    }

// Return found node
    return pNode;
}


/********************************************************************************
* AIN_GetAngleToRacingLine(PLAYER *pPlayer)
*
* Calculates the angle from the car to the racing line.
*
* Inputs:
*   pPlayer     Player structure
*   pDir        Location to store racing direction.
*
* Outputs:
*   Return      1-Cos of the angle of the car's direction to racing line ((-65536 to 65536) neg = left of racing line)
*   pDir         1 = Car facing in the direction of the racing line
*                0 = Car facing about 90 degress to the racing line
*               -1 = Car facing in the opposite direction of the racing line
*
********************************************************************************/
REAL CAI_GetAngleToRacingLine(PLAYER *pPlayer, VEC *pFVec, VEC *pRVec, REAL* pSide)
{
    AINODE          *pNodeAhead;
    REAL            angle;
    VEC             tVec, destPos;
    PLAYER          *pPlayer2;
//  REAL            dist;
    REAL            nodeEdgeDist;

// Has the CPU player got a putty bomb ?
    if ((pPlayer->type == PLAYER_CPU) && (pPlayer->car.IsBomb))
//      if ((pPlayer->PickupType == PICKUP_PUTTYBOMB))
    {
#if 0
        if ((pPlayer->CarAI.pNearestCarBehind) && (pPlayer->CarAI.distNearestCarBehind < TO_LENGTH(Real(200*6))))
        {
            pPlayer2 = pPlayer->CarAI.pNearestCarBehind;

        // Set destination position to the target car's position
            CopyVec(&pPlayer2->car.Body->Centre.Pos, &pPlayer->CarAI.destPos);
            VecMinusVec(&pPlayer2->car.Body->Centre.Pos, &pPlayer->car.Body->Centre.Pos, &tVec)

            dist = VecDotVec(&tVec, &pPlayer->car.Body->Centre.WMatrix.mv[R]);

            VecPlusEqScalarVec(&pPlayer->CarAI.destPos, dist, &pPlayer->car.Body->Centre.WMatrix.mv[R]);


        // Set forward vector
            pFVec->v[Y] = 0;
            pFVec->v[X] = pPlayer->CarAI.destPos.v[X] - pPlayer->car.Body->Centre.Pos.v[X];
            pFVec->v[Z] = pPlayer->CarAI.destPos.v[Z] - pPlayer->car.Body->Centre.Pos.v[Z];
        #ifdef _PSX
            pFVec->v[X] >>= 4;
            pFVec->v[Z] >>= 4;
        #endif
            NormalizeVec(pFVec);

        // Set right vector
            pRVec->v[X] = pFVec->v[Z];
            pRVec->v[Y] = 0;
            pRVec->v[Z] = -pFVec->v[X];

        // Get cosine of angle between car's forward vector & racing line right vector
            *pSide = VecDotVec(pRVec, &pPlayer->CarAI.forwardVec);

        // Get cosine of angle between car's forward vector & racing line forward vector
            return (abs(VecDotVec(pFVec, &pPlayer->CarAI.forwardVec)));

        }
#else
        pPlayer2 = NULL;
//      if ((pPlayer->CarAI.pNearestCarBehind) && (pPlayer->CarAI.distNearestCarBehind < TO_LENGTH(Real(200*3))))
//          pPlayer2 = pPlayer->CarAI.pNearestCarBehind;
//      else
        if ((pPlayer->CarAI.pNearestCarInFront) && (pPlayer->CarAI.distNearestCarInFront < TO_LENGTH(Real(200*3))))
            pPlayer2 = pPlayer->CarAI.pNearestCarInFront;

//          if ((pPlayer->CarAI.pNearestCarInFront) && (pPlayer->CarAI.distNearestCarInFront < TO_LENGTH(Real(200*10))))
//              pPlayer2 = pPlayer->CarAI.pNearestCarInFront;

        if (pPlayer2)
        {
        // Set destination position to the target car's position
#if 1
            CopyVec(&pPlayer2->car.Body->Centre.Pos, &pPlayer->CarAI.destPos);
#else
            CopyVec(&pPlayer->car.Body->Centre.Pos, &pPlayer->CarAI.destPos);
            VecPlusEqScalarVec(&pPlayer->CarAI.destPos, TO_LENGTH(Real(200*4)), &pPlayer->CarAI.forwardVec);

            VecMinusVec(&pPlayer2->car.Body->Centre.Pos, &pPlayer->car.Body->Centre.Pos, &tVec)
            dist = VecDotVec(&tVec, &pPlayer->car.Body->Centre.WMatrix.mv[R]);
            VecPlusEqScalarVec(&pPlayer->CarAI.destPos, dist, &pPlayer->car.Body->Centre.WMatrix.mv[R]);
#endif

        // Set forward vector
            pFVec->v[Y] = 0;
            pFVec->v[X] = pPlayer->CarAI.destPos.v[X] - pPlayer->car.Body->Centre.Pos.v[X];
            pFVec->v[Z] = pPlayer->CarAI.destPos.v[Z] - pPlayer->car.Body->Centre.Pos.v[Z];
        #ifdef _PSX
            pFVec->v[X] >>= 4;
            pFVec->v[Z] >>= 4;
        #endif
            NormalizeVec(pFVec);

        // Set right vector
            pRVec->v[X] = pFVec->v[Z];
            pRVec->v[Y] = 0;
            pRVec->v[Z] = -pFVec->v[X];

        // Get cosine of angle between car's forward vector & racing line right vector
            *pSide = VecDotVec(pRVec, &pPlayer->CarAI.forwardVec);

        // Get cosine of angle between car's forward vector & racing line forward vector
            return (abs(VecDotVec(pFVec, &pPlayer->CarAI.forwardVec)));
        }
#endif
    }



// Get vector to destination
    if (pPlayer->CarAI.AIState != CAI_S_RACE)
        pPlayer->CarAI.lookAheadDist = TO_LENGTH(Real(200*1));
    else if (pPlayer->CarAI.pCurNode)
        pPlayer->CarAI.lookAheadDist = TO_LENGTH(Real(200*3)) + MulScalar(pPlayer->CarAI.speedCur, Real(0.1));
    else
        pPlayer->CarAI.lookAheadDist = TO_LENGTH(Real(200*10));

    if (!(pPlayer->CarAI.pDestNode = pNodeAhead = AIN_FindNodeAhead(pPlayer, pPlayer->CarAI.lookAheadDist, &pPlayer->CarAI.destPos)))
    {
        *pSide = 0;
        return ONE;
    }


// Set forward vector
    pFVec->v[Y] = 0;
    pFVec->v[X] = pPlayer->CarAI.destPos.v[X] - pPlayer->car.Body->Centre.Pos.v[X];
    pFVec->v[Z] = pPlayer->CarAI.destPos.v[Z] - pPlayer->car.Body->Centre.Pos.v[Z];
#ifdef _PSX
    pFVec->v[X] >>= 4;
    pFVec->v[Z] >>= 4;
#endif
    NormalizeVec(pFVec);

// Set right vector
    pRVec->v[X] = pFVec->v[Z];
    pRVec->v[Y] = 0;
    pRVec->v[Z] = -pFVec->v[X];
//  tVec.v[X] = Real(0);
//  tVec.v[Y] = Real(-1);
//  tVec.v[Z] = Real(0);
//  VecCrossVec(pFVec, &tVec, pRVec);   // *** Should probably do a special case as I know which elements are zero !!!
//  NormalizeVec(pRVec);

#ifdef OLD_AI_VERSION

// Get cosine of angle between car's forward vector & racing line right vector
    *pSide = VecDotVec(pRVec, &pPlayer->CarAI.forwardVec);

// Get cosine of angle between car's forward vector & racing line forward vector
    angle = VecDotVec(pFVec, &pPlayer->CarAI.forwardVec);

#else

// Calc. distance to plane edges.
#ifndef _PSX
    pPlayer->CarAI.dstNodeEdgeDist[0] = VecDotPlane(&pPlayer->CarAI.destPos, &pNodeAhead->link.planeEdge[0]);
    pPlayer->CarAI.dstNodeEdgeDist[1] = VecDotPlane(&pPlayer->CarAI.destPos, &pNodeAhead->link.planeEdge[1]);
#else
    tVec.v[X] = pPlayer->CarAI.destPos.v[X] - pNodeAhead->Node[0].Pos.v[X];
    tVec.v[Y] = pPlayer->CarAI.destPos.v[Y] - pNodeAhead->Node[0].Pos.v[Y];
    tVec.v[Z] = pPlayer->CarAI.destPos.v[Z] - pNodeAhead->Node[0].Pos.v[Z];
    pPlayer->CarAI.dstNodeEdgeDist[0] = VecDotVec(&tVec, PlaneNormal(&pNodeAhead->link.planeEdge[0]));
    tVec.v[X] = pPlayer->CarAI.destPos.v[X] - pNodeAhead->Node[1].Pos.v[X];
    tVec.v[Y] = pPlayer->CarAI.destPos.v[Y] - pNodeAhead->Node[1].Pos.v[Y];
    tVec.v[Z] = pPlayer->CarAI.destPos.v[Z] - pNodeAhead->Node[1].Pos.v[Z];
    pPlayer->CarAI.dstNodeEdgeDist[1] = VecDotVec(&tVec, PlaneNormal(&pNodeAhead->link.planeEdge[1]));
#endif
    if (pPlayer->CarAI.dstNodeEdgeDist[0] < pPlayer->CarAI.dstNodeEdgeDist[1])
        nodeEdgeDist = pPlayer->CarAI.dstNodeEdgeDist[0];
    else
        nodeEdgeDist = pPlayer->CarAI.dstNodeEdgeDist[1];
    nodeEdgeDist = abs(nodeEdgeDist - pPlayer->CarAI.biasEdge);
    if (nodeEdgeDist > pPlayer->CarAI.biasMaxSize)
        nodeEdgeDist = pPlayer->CarAI.biasMaxSize;

// Get cosine of angle between car's forward vector & racing line right vector
    tVec.v[Y] = 0;
    tVec.v[X] = MulScalar(pPlayer->CarAI.lookAheadDist, pPlayer->CarAI.forwardVec.v[X]) + pPlayer->car.Body->Centre.Pos.v[X];
    tVec.v[Z] = MulScalar(pPlayer->CarAI.lookAheadDist, pPlayer->CarAI.forwardVec.v[Z]) + pPlayer->car.Body->Centre.Pos.v[Z];
    tVec.v[X] -= pPlayer->CarAI.destPos.v[X];
    tVec.v[Z] -= pPlayer->CarAI.destPos.v[Z];

    *pSide = VecDotVec(pRVec, &tVec);

    if ((*pSide < -pPlayer->CarAI.biasSize) ||
        (*pSide > pPlayer->CarAI.biasSize))
    {
        pPlayer->CarAI.biasSize += MulScalar(TimeStep, pPlayer->CarAI.biasExpandRate);
        if (pPlayer->CarAI.biasSize > nodeEdgeDist)
            pPlayer->CarAI.biasSize = nodeEdgeDist;

        if (*pSide > pPlayer->CarAI.biasSize)
        {
            destPos.v[X] = pPlayer->CarAI.destPos.v[X] + MulScalar(pPlayer->CarAI.biasSize, pRVec->v[X]);
            destPos.v[Z] = pPlayer->CarAI.destPos.v[Z] + MulScalar(pPlayer->CarAI.biasSize, pRVec->v[Z]);
        }
        else
        {
            destPos.v[X] = pPlayer->CarAI.destPos.v[X] - MulScalar(pPlayer->CarAI.biasSize, pRVec->v[X]);
            destPos.v[Z] = pPlayer->CarAI.destPos.v[Z] - MulScalar(pPlayer->CarAI.biasSize, pRVec->v[Z]);
        }

        pFVec->v[Y] = 0;
        pFVec->v[X] = pPlayer->CarAI.destPos.v[X] - pPlayer->car.Body->Centre.Pos.v[X];
        pFVec->v[Z] = pPlayer->CarAI.destPos.v[Z] - pPlayer->car.Body->Centre.Pos.v[Z];
#ifdef _PSX
        pFVec->v[X] >>= 4;
        pFVec->v[Z] >>= 4;
#endif
        NormalizeVec(pFVec);

        pRVec->v[X] = pFVec->v[Z];
        pRVec->v[Y] = 0;
        pRVec->v[Z] = -pFVec->v[X];

    // Get cosine of angle between car's forward vector & racing line forward vector
        angle = VecDotVec(pFVec, &pPlayer->CarAI.forwardVec);
    }
    else
    {
        pPlayer->CarAI.biasSize -= MulScalar(TimeStep, pPlayer->CarAI.biasShrinkRate);
        if (pPlayer->CarAI.biasSize < TO_LENGTH(Real(50)))
            pPlayer->CarAI.biasSize = TO_LENGTH(Real(50));

    // Get cosine of angle between car's forward vector & racing line forward vector
//      angle = Real(0.990);//ONE;

        angle = MulScalar(VecDotVec(pFVec, &pPlayer->CarAI.forwardVec), Real(1.5));
    }
#endif
    if (angle < -ONE)
        angle = -ONE;
    else if (angle > ONE)
        angle = ONE;

    return angle;
}


/********************************************************************************
* AINODE *AIN_FindNodePlayerIsIn(PLAYER *pPlayer)
*
* Finds which node the player is in.  This function assumes that the player is
* inside a valid Zone & checks the nodes inside it.
********************************************************************************/
AINODE *AIN_FindNodePlayerIsIn(PLAYER *pPlayer)
{   
    AIZONE  *pZone;
    AINODE  *pNode;
    int     i;

    if (pPlayer->CarAI.ZoneID >= 0)
    {
        if (pZone = &AiZones[pPlayer->CarAI.ZoneID])
        {
            pNode = pZone->FirstNode;
            for (i = 0; i < pZone->Count; i++, pNode = pNode->ZoneNext)
            {
                if (AIN_IsPointInNodeBounds(&pPlayer->car.Body->Centre.Pos, pNode, -1))
                    return pNode;
            }
        }
    }

#if 0
    pNode = AiNode;
    for (i = 0; i < AiNodeNum; i++, pNode++)
    {
        if (AIN_IsPointInNodeBounds(&pPlayer->car.Body->Centre.Pos, pNode))
            return pNode;
    }
#endif

    return NULL;
}


/********************************************************************************
* AINODE *AIN_FindNodePlayerIsIn(PLAYER *pPlayer)
*
* Finds which node the player is in.  This function checks the ZoneID (0,1,-1)
********************************************************************************/
AINODE *AIN_FindZoneNodePlayerIsIn(PLAYER *pPlayer)
{   
    AIZONE  *pZone;
    AINODE  *pNode;
    int     i,j;
    int     cZone, iZone;
    static int  zoneOffset[] = {0,1,-1};

    if (pPlayer->CarAI.ZoneID >= 0)
    {
        for (cZone = 0; cZone < 3; cZone++)
        {
            iZone = pPlayer->CarAI.ZoneID + zoneOffset[cZone];
            Wrap(iZone, 0,AiZoneNumID);

            pZone = AiZoneHeaders[iZone].Zones;
            for (j = 0; j < AiZoneHeaders[iZone].Count; j++, pZone++)
            {
                pNode = pZone->FirstNode;
                for (i = 0; i < pZone->Count; i++, pNode = pNode->ZoneNext)
                {
                    if (AIN_IsPointInNodeBounds(&pPlayer->car.Body->Centre.Pos, pNode, -1))
                        return pNode;
                }
            }
        }
    }

    return NULL;
}



/********************************************************************************
* int AIN_IsPointInNodeBounds(VEC *pPos, AINODE *pNode)
*
* Return Value:
*   0 = Not in node bounds.
*   1 = In Left link bounds
*   2 = In Right link bounds
*   3 = In Left AND Right link bounds
********************************************************************************/
int AIN_IsPointInNodeBounds(VEC *pPos, AINODE *pNode, int iRoute)
{
    AINODE  *pNodeN;
    VEC     *pV[4];
//  int     cLink, 
    int     cV, cVP, cHits;
    int     x, y, z;
    REAL    midX;
    REAL    dist[4];
    int     side[4];
#ifdef _PSX
    int     delta;
#endif

    pV[2] = &pNode->Node[1].Pos;
    pV[3] = &pNode->Node[0].Pos;
    //for (cLink = 0; cLink < MAX_AINODE_LINKS; cLink++)
    {
//      if ((pNodeN = pNode->Next[cLink]) && ((iRoute == cLink) || (iRoute == -1)))
        if (pNodeN = pNode->Next[0])
        {
        // Is the point outside the nodes bounds ?
            if ((pPos->v[X] >= pNode->link.boundsMin[X]) &&
                (pPos->v[X] <= pNode->link.boundsMax[X]) &&
                (pPos->v[Z] >= pNode->link.boundsMin[Y]) &&
                (pPos->v[Z] <= pNode->link.boundsMax[Y]))
            {
            // Check if point is inside polygon
                pV[0] = &pNodeN->Node[0].Pos;
                pV[1] = &pNodeN->Node[1].Pos;
                x = X;
                y = Z;
                z = Y;

                for (cV = 0; cV < 4; cV++)
                {
                    dist[cV] = pV[cV]->v[y] - pPos->v[y];
                    if (dist[cV] < 0)   side[cV] = -1;
                    else                side[cV] = 1;
                }

                cHits = 0;
                for (cVP = 3, cV = 0; cV < 4; cVP = cV, cV++)
                {
                    // Is point within y limit of line ?
                    if (side[cV] != side[cVP])
                    {
                        // Is point to the left of both line points ?
                        if ((pPos->v[x] < pV[cV]->v[x]) && (pPos->v[x] < pV[cVP]->v[x]))
                        {
                            cHits++;
                        }
                        else if ((pPos->v[x] < pV[cV]->v[x]) || (pPos->v[x] < pV[cVP]->v[x]))
                        {
#ifndef _PSX
    //                      midX = pV[cV]->v[x] + ( ((pPos->v[y] - pV[cV]->v[y]) * (pV[cVP]->v[x] - pV[cV]->v[x])) /
    //                                                                             (pV[cVP]->v[y] - pV[cV]->v[y]) );
                            midX = pV[cV]->v[x] + DivScalar( MulScalar(pPos->v[y] - pV[cV]->v[y], pV[cVP]->v[x] - pV[cV]->v[x]),
                                                                        pV[cVP]->v[y] - pV[cV]->v[y] );
                            if (pPos->v[x] < midX)
                                cHits++;
#else
                            if (delta = ((pV[cVP]->v[y] - pV[cV]->v[y]) >> 16))
                            {
                                midX = (pV[cV]->v[x] >> 16) +
                                ( (((pPos->v[y] - pV[cV]->v[y]) >> 16) * ((pV[cVP]->v[x] - pV[cV]->v[x])) >> 16) / delta );

                                if ((pPos->v[x] >> 16) < midX)
                                    cHits++;
                            }
/*
                            else
                            {
                                midX = (pV[cV]->v[x] >> 16);

                                if ((pPos->v[x] >> 16) < midX)
                                    cHits++;
                            }
*/
#endif
                        }
                    }
                }

                // Odd number of hits means point is inside the quad
                if (cHits & 1)
                {
                    return 1;//(cLink+1);
                }
            }
        }
    }

    return 0;
}


