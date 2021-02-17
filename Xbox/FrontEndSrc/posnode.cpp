//-----------------------------------------------------------------------------
// File: posnode.cpp
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"
#include "posnode.h"
#include "aizone.h"
#include "geom.h"
#include "timing.h"
#include "text.h"
#include "draw.h"
#include "panel.h"

// globals

long PosNodeNum, PosStartNode;
POSNODE *PosNode;
REAL PosTotalDist;

REAL FinishDistPanelMaxStep;

#ifdef _PC
extern long WrongWayFlag;
extern float WrongWayTimer;
#endif

////////////////////
// load pos nodes //
////////////////////
#ifndef _N64
void LoadPosNodes(char *file)
{
    long i, j;
    FILE *fp;
    FILE_POSNODE pan;

// open node file

    PosNode = NULL;
    PosNodeNum = 0;

    fp = fopen(file, "rb");
    if (!fp) return;


// read num

    fread(&PosNodeNum, sizeof(PosNodeNum), 1, fp);
    if (!PosNodeNum) return;

// load start node

    fread(&PosStartNode, sizeof(PosStartNode), 1, fp);

// load total dist

    fread(&PosTotalDist, sizeof(PosTotalDist), 1, fp);
    
// alloc mem


    
    PosNode = (POSNODE*)malloc(sizeof(POSNODE) * PosNodeNum);


    if (!PosNode)
    {
        PosNodeNum = 0;
        return;
    }


// loop thru all nodes

    for (i = 0 ; i < PosNodeNum ; i++)
    {

// load one file node

        fread(&pan, sizeof(pan), 1, fp);

// setup node

        PosNode[i].Pos = pan.Pos;
        PosNode[i].Dist = pan.Dist;

        for (j = 0 ; j < POSNODE_MAX_LINKS ; j++)
        {
            if (pan.Prev[j] != -1)
                PosNode[i].Prev[j] = PosNode + pan.Prev[j];
            else
                PosNode[i].Prev[j] = NULL;

            if (pan.Next[j] != -1)
                PosNode[i].Next[j] = PosNode + pan.Next[j];
            else
                PosNode[i].Next[j] = NULL;
        }
    }

// close node file

    fclose(fp);

// set finish dist panel max step

    FinishDistPanelMaxStep = DivScalar(Real(2000), PosTotalDist);
    if (FinishDistPanelMaxStep < Real(0.1))
        FinishDistPanelMaxStep = Real(0.1);



}

#endif




////////////////////
// kill pos nodes //
////////////////////

void FreePosNodes(void)
{
    free(PosNode);
}


#ifndef _PSX

////////////////////////////////
// update car finish distance //
////////////////////////////////

long UpdateCarFinishDist(PLAYER *player, unsigned long *timedelta)
{
    POSNODE *currentnode, *nearestnode;
    REAL cdist, ndist, dist, add, max, angle, distprev, distnext, lenprev, lennext;
    VEC vec, norm, dir, normprev, normnext, posprev, posnext;
    AIZONE *zone;
    long i, j, flag, outzone;
    REAL lastdist;
    VEC vec1, vec2;
    REAL ctime, ptime;

// zero time delta

    if (timedelta)
    {
        *timedelta = 0;
    }

// not if no nodes or zones

    if (!PosNodeNum)
        return FALSE;

    if (!AiZoneNum)
        return FALSE;

// less to do if I'm outside my current ai zone

    outzone = FALSE;

    zone = AiZoneHeaders[player->CarAI.ZoneID].Zones;
    flag = FALSE;

    for (i = 0 ; i < AiZoneHeaders[player->CarAI.ZoneID].Count ; i++, zone++)
    {
        for (j = 0 ; j < 3 ; j++)
        {
            dist = PlaneDist(&zone->Plane[j], &player->car.Body->Centre.Pos);
            if (dist < -zone->Size[j] || dist > zone->Size[j])
            {
                break;
            }
        }

        if (j == 3)
            flag = TRUE;
    }


    if (!flag)
    {
        player->CarAI.WrongWay = TRUE;
/*#ifdef _PC
        WrongWayFlag = TRUE;
        WrongWayTimer = WRONG_WAY_TOLERANCE;
#endif*/
        outzone = TRUE;
    }

// get dist to current node 

    currentnode = &PosNode[player->CarAI.FinishDistNode];

    if (!outzone)
    {
        SubVector(&currentnode->Pos, &player->car.Body->Centre.Pos, &vec);
        cdist = MulScalar(vec.v[X], vec.v[X]) + MulScalar(vec.v[Y], vec.v[Y]) + MulScalar(vec.v[Z], vec.v[Z]);

#ifndef _PSX
        ndist = FLT_MAX;
#else 
        ndist = LARGEDIST;
#endif

// look for nearer node

        for (i = 0 ; i < POSNODE_MAX_LINKS ; i++)
        {

// check previous links

            if (currentnode->Prev[i])
            {
                SubVector(&currentnode->Prev[i]->Pos, &player->car.Body->Centre.Pos, &vec);
                dist = MulScalar(vec.v[X], vec.v[X]) + MulScalar(vec.v[Y], vec.v[Y]) + MulScalar(vec.v[Z], vec.v[Z]);

                if (dist < ndist)
                {
                    ndist = dist;
                    nearestnode = currentnode->Prev[i];
                }
            }

// check next links

            if (currentnode->Next[i])
            {
                SubVector(&currentnode->Next[i]->Pos, &player->car.Body->Centre.Pos, &vec);
                dist = MulScalar(vec.v[X], vec.v[X]) + MulScalar(vec.v[Y], vec.v[Y]) + MulScalar(vec.v[Z], vec.v[Z]);

                if (dist < ndist)
                {
                    ndist = dist;
                    nearestnode = currentnode->Next[i];
                }
            }
        }

// update current node?

        if (ndist < cdist)
        {
            player->CarAI.FinishDistNode = (long)(nearestnode - PosNode);
            currentnode = nearestnode;
        }

// calc average normal from forward links

        SetVector(&norm, 0, 0, 0 );

        for (i = 0 ; i < POSNODE_MAX_LINKS ; i++)
        {
            if (currentnode->Next[i])
            {
                SubVector(&currentnode->Pos, &currentnode->Next[i]->Pos, &vec);
                NormalizeVector(&vec);
                AddVector(&norm, &vec, &norm);
            }
        }

        NormalizeVector(&norm);

// update finish dist

        SubVector(&player->car.Body->Centre.Pos, &currentnode->Pos, &vec);
        player->CarAI.FinishDist = currentnode->Dist + DotProduct(&norm, &vec);

// update 'wrong way' flag

        player->CarAI.WrongWay = (DotProduct(&norm, &player->car.Body->Centre.WMatrix.mv[L]) > Real(0.6f) );

// update panel finish dist

        lastdist = player->CarAI.FinishDistPanel;

        dist = DivScalar(player->CarAI.FinishDist , PosTotalDist);
        add = dist - player->CarAI.FinishDistPanel;

        if (add > Real(0.5)) add -= Real(1);
        else if (add < Real(-0.5)) add += Real(1);

//      max = MulScalar(Real(FinishDistPanelMaxStep), TimeStep);
//      if (add > max) add = max;
//      else if (add < -max) add = -max;

        player->CarAI.FinishDistPanel += add;
        if (player->CarAI.FinishDistPanel >= Real(1)) player->CarAI.FinishDistPanel -= Real(1);
        else if (player->CarAI.FinishDistPanel < 0 ) player->CarAI.FinishDistPanel += Real(1);
    }

// get dir vector to destination - in zones

    if (player->type == PLAYER_LOCAL)
    {
        if (!outzone)
        {

// get dist along average prev nodes planes

            distprev = 0.0f;
            SetVector(&posprev, 0.0f, 0.0f, 0.0f);

            for (i = 0 ; i < POSNODE_MAX_LINKS ; i++) if (currentnode->Prev[i])
            {
                AddVector(&currentnode->Prev[i]->Pos, &posprev, &posprev);
                distprev++;
            }

            VecMulScalar(&posprev, (1.0f / distprev));

            SubVector(&currentnode->Pos, &posprev, &normprev);
            lenprev = Length(&normprev);

            SubVector(&player->car.Body->Centre.Pos, &posprev, &vec);
            distprev = DotProduct(&normprev, &vec) / lenprev;
            if (distprev < 0.0f) distprev = 0.0f;
            else if (distprev > lenprev) distprev = lenprev;

// get dist along average next nodes planes

            distnext = 0.0f;
            SetVector(&posnext, 0.0f, 0.0f, 0.0f);

            for (i = 0 ; i < POSNODE_MAX_LINKS ; i++) if (currentnode->Next[i])
            {
                AddVector(&currentnode->Next[i]->Pos, &posnext, &posnext);
                distnext++;
            }

            VecMulScalar(&posnext, (1.0f / distnext));

            SubVector(&posnext, &currentnode->Pos, &normnext);
            lennext = Length(&normnext);

            SubVector(&player->car.Body->Centre.Pos, &currentnode->Pos, &vec);
            distnext = DotProduct(&normnext, &vec) / lennext;
            if (distnext < 0.0f) distnext = 0.0f;
            else if (distnext > lennext) distnext = lennext;

// get dir

            ctime = lenprev / (lenprev + lennext);
            ptime = (distprev + distnext) / (lenprev + lennext);

            QuadInterpVec(&posprev, 0.0f, &currentnode->Pos, ctime, &posnext, 1.0f, ptime - 0.1f, &vec1);
            QuadInterpVec(&posprev, 0.0f, &currentnode->Pos, ctime, &posnext, 1.0f, ptime + 0.1f, &vec2);

            SubVector(&vec1, &vec2, &dir);
            NormalizeVector(&dir);
        }

// get dir vector to destination - out of zones

        else
        {
            SubVector(&player->car.Body->Centre.Pos, &currentnode->Pos, &dir);
            NormalizeVector(&dir);
        }

// update dest angle from dir vector

        SetVector(&vec, player->car.Body->Centre.WMatrix.m[LZ], 0.0f, -player->car.Body->Centre.WMatrix.m[LX]);
        if (vec.v[X] || vec.v[Z])
        {
            NormalizeVector(&vec);
#ifdef _PC
            angle = (float)atan2(-DotProduct(&vec, &dir), -DotProduct(&player->car.Body->Centre.WMatrix.mv[Z], &dir)) + RAD * 3.0f / 8.0f;
#else
            angle = (float)Myatan2(-DotProduct(&vec, &dir), -DotProduct(&player->car.Body->Centre.WMatrix.mv[Z], &dir)) + RAD * 3.0f / 8.0f;
#endif
            add = angle - player->CarAI.AngleToNextNode;
            if (add > PI) add -= RAD;
            else if (add < -PI) add += RAD;

            max = 3.0f * TimeStep;
            if (add > max) add = max;
            else if (add < -max) add = -max;

            player->CarAI.AngleToNextNode += add;
            if (player->CarAI.AngleToNextNode >= RAD) player->CarAI.AngleToNextNode -= RAD;
            if (player->CarAI.AngleToNextNode <0) player->CarAI.AngleToNextNode += RAD;

        }
    }

// quit now if out of zone

    if (outzone)
        return FALSE;

// crossed line?

    if (lastdist < Real(0.25) && player->CarAI.FinishDistPanel > Real(0.75))
    {

// calc time delta

        if (timedelta && (!player->CarAI.BackTracking || player->CarAI.PreLap))
        {
            dist = Real(1) - player->CarAI.FinishDistPanel;
            *timedelta = (unsigned long)(dist / (dist + lastdist) * TimeStep * 1000);
        }

// zero prelap?

        player->CarAI.PreLap = FALSE;

// was backtracking?

        if (player->CarAI.BackTracking)
        {
            player->CarAI.BackTracking = FALSE;
        }

// nope, new lap

        else
        {
            return TRUE;
        }
    }

// start backtracking?

    else if (lastdist > Real(0.75) && player->CarAI.FinishDistPanel < Real(0.25))
    {
        player->CarAI.BackTracking = TRUE;
    }

// return false

    return FALSE;
}


#else  // PSX Version

////////////////////////////////
// update car finish distance //
////////////////////////////////

long UpdateCarFinishDist(PLAYER *player, unsigned long *timedelta)
{
    POSNODE *currentnode, *nearestnode;
    REAL cdist, ndist, dist, add, max;
    VEC vec, norm;

    VEC PlayerPos;

    AIZONE *zone;
    long i, j, flag, outzone;
    REAL lastdist, quarterdist, threequarterdist;

    REAL angle;
    VEC dir;


    // zero time delta

    if (timedelta)
    {
        *timedelta = 0;
    }

    // Using a much reduced scale

    PlayerPos.v[0] = FROM_LENGTH( player->car.Body->Centre.Pos.v[0] >> 8) >>8;
    PlayerPos.v[1] = FROM_LENGTH( player->car.Body->Centre.Pos.v[1] >> 8) >>8;
    PlayerPos.v[2] = FROM_LENGTH( player->car.Body->Centre.Pos.v[2] >> 8) >>8;



// not if no nodes or zones

    if (!PosNodeNum)
        return FALSE;

    if (!AiZoneNum)
        return FALSE;

// not if I'm outside my current ai zone


    outzone = FALSE;

    zone = AiZoneHeaders[player->CarAI.ZoneID].Zones;
    flag = FALSE;

    for (i = 0 ; i < AiZoneHeaders[player->CarAI.ZoneID].Count ; i++, zone++)
    {
        for (j = 0 ; j < 3 ; j++)
        {
            dist = PlaneDist(&zone->Plane[j], &player->car.Body->Centre.Pos );
            if (dist < -zone->Size[j] || dist > zone->Size[j])
            {
                break;
            }
        }

        if (j == 3)
            flag = TRUE;
    }

//  if (!flag)
//      return FALSE;


    if (!flag)
    {
        player->CarAI.WrongWay = TRUE;
        outzone = TRUE;
    }



// get dist to current node 


    currentnode = &PosNode[player->CarAI.FinishDistNode];
    
    if (!outzone)
    {

        
        SubVector(&currentnode->Pos, &PlayerPos , &vec);
        cdist = MulScalar(vec.v[X], vec.v[X]) + MulScalar(vec.v[Y], vec.v[Y]) + MulScalar(vec.v[Z], vec.v[Z]);


        ndist = LARGEDIST;

    // look for nearer node

        for (i = 0 ; i < POSNODE_MAX_LINKS ; i++)
        {

    // check previous links

            if (currentnode->Prev[i])
            {
                SubVector(&currentnode->Prev[i]->Pos, &PlayerPos , &vec);
                dist = MulScalar(vec.v[X], vec.v[X]) + MulScalar(vec.v[Y], vec.v[Y]) + MulScalar(vec.v[Z], vec.v[Z]);

                if (dist < ndist)
                {
                    ndist = dist;
                    nearestnode = currentnode->Prev[i];
                }
            }

    // check next links

            if (currentnode->Next[i])
            {
                SubVector(&currentnode->Next[i]->Pos, &PlayerPos , &vec);
                dist = MulScalar(vec.v[X], vec.v[X]) + MulScalar(vec.v[Y], vec.v[Y]) + MulScalar(vec.v[Z], vec.v[Z]);

                if (dist < ndist)
                {
                    ndist = dist;
                    nearestnode = currentnode->Next[i];
                }
            }
        }


    

    // update current node?

        if (ndist < cdist)
        {
            player->CarAI.FinishDistNode = (long)(nearestnode - PosNode);
            currentnode = nearestnode;
        }


    }

// calc average normal from forward links

    SetVector(&norm, 0, 0, 0 );

    for (i = 0 ; i < POSNODE_MAX_LINKS ; i++)
    {
        if (currentnode->Next[i])
        {
            SubVector(&currentnode->Pos, &currentnode->Next[i]->Pos, &vec);
            NormalizeVector(&vec);
            AddVector(&norm, &vec, &norm);
        }
    }

    NormalizeVector(&norm);


        // update finish dist

    if (!outzone)
    {
    
        SubVector(&PlayerPos, &currentnode->Pos, &vec);

        lastdist = player->CarAI.FinishDist;
        player->CarAI.FinishDist = currentnode->Dist + DotProduct(&norm, &vec);
        if (player->CarAI.FinishDist < 0) player->CarAI.FinishDist += PosTotalDist;
        if (player->CarAI.FinishDist > PosTotalDist) player->CarAI.FinishDist -= PosTotalDist;

        quarterdist = PosTotalDist >> 2;
        threequarterdist = (PosTotalDist * 3) >> 2;
        
        // update 'wrong way' flag

        player->CarAI.WrongWay = (DotProduct(&norm, &player->car.Body->Centre.WMatrix.mv[L]) > Real(0.6f) );

    }



    // Angle to racing line;


    if( !outzone)
    {
        CopyVec(&norm, &dir);
    }
    else
    {
        SubVector(&player->car.Body->Centre.Pos, &currentnode->Pos, &dir);
        NormalizeVector(&dir);

    }


        
    if( GameSettings.GameType == GAMETYPE_BATTLE  )
    {
        if( player == &Players[0] )
        {
            SubVector(&Players[0].car.Body->Centre.Pos, &Players[1].car.Body->Centre.Pos, &dir);
        }
        else
        {
            SubVector(&Players[1].car.Body->Centre.Pos, &Players[0].car.Body->Centre.Pos, &dir);
        }

        NormalizeVector(&dir);
    }


    SetVector(&vec, player->car.Body->Centre.WMatrix.m[LZ], 0, -player->car.Body->Centre.WMatrix.m[LX]);



    if (vec.v[X] || vec.v[Z])
    {
        NormalizeVector(&vec);
        angle = ratan21616(-DotProduct(&vec, &dir)>>4, -DotProduct(&player->car.Body->Centre.WMatrix.mv[Z], &dir)>>4 ) ;// + DivScalar( MulScalar(4096, Real(3.0f) ) , Real(8.0f) );

        angle >>= 4;

        add = angle - player->CarAI.AngleToNextNode;
        if (add > 2048) add -= 4096;
        else if (add < -2048) add += 4096;

        max = TimeFactor*32;//MulScalar( Real(3.0f), TimeStep);
        if (add > max) add = max;
        else if (add < -max) add = -max;

        player->CarAI.AngleToNextNode += add;
        if (player->CarAI.AngleToNextNode >= 4096)
            player->CarAI.AngleToNextNode -= 4096;
        if (player->CarAI.AngleToNextNode <0)
            player->CarAI.AngleToNextNode += 4096;

    
    }
    

    // quid now if out of zone

    if( outzone )
        return FALSE;


// crossed line?

    if (lastdist < quarterdist && player->CarAI.FinishDist > threequarterdist)
    {

// zero prelap?

        player->CarAI.PreLap = FALSE;

// was backtracking?

        if (player->CarAI.BackTracking)
        {
            player->CarAI.BackTracking = FALSE;
        }

// nope, new lap

        else
        {
                return TRUE;
        }
    }

// start backtracking?

    else if (lastdist > threequarterdist && player->CarAI.FinishDist < quarterdist)
    {
        player->CarAI.BackTracking = TRUE;
    }
     
// return false

    return FALSE;
}


#endif

