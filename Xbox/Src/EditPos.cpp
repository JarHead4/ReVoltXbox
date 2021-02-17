//-----------------------------------------------------------------------------
// File: EditPos.cpp
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"
#include "editpos.h"
#include "model.h"
#include "main.h"
#include "geom.h"
#include "LevelLoad.h"
#include "camera.h"
#include "input.h"
#include "timing.h"
#include "player.h"
#include "text.h"
#include "draw.h"

// globals

POSNODE *CurrentEditPosNode = NULL;
POSNODE *LastEditPosNode = NULL;
long EditPosNodeNum, EditPosStartNode;
float EditPosTotalDist;
POSNODE *EditPosNode;
MODEL EditPosNodeModel;

////////////////////
// init pos nodes //
////////////////////

void InitEditPosNodes(void)
{
    EditPosNode = (POSNODE*)malloc(sizeof(POSNODE) * POSNODE_MAX);
    if (!EditPosNode)
    {
        DumpMessage(NULL, "Can't alloc memory for edit pos nodes!");
        QuitGame();
    }
}

/////////////////////////
// kill edit pos nodes //
/////////////////////////

void FreeEditPosNodes(void)
{
    free(EditPosNode);
}

////////////////////
// load pos nodes //
////////////////////

void LoadEditPosNodes(char *file)
{
    long i, j;
    FILE *fp;
    FILE_POSNODE pan;

// open node file

    fp = fopen(file, "rb");

// if not there create empty one

    if (!fp)
    {
        fp = fopen(file, "wb");
        if (!fp) return;
        i = 0;
        fwrite(&i, sizeof(i), 1, fp);
        fclose(fp);
        fp = fopen(file, "rb");
        if (!fp) return;
    }

// load num

    fread(&EditPosNodeNum, sizeof(EditPosNodeNum), 1, fp);

// load start node

    fread(&EditPosStartNode, sizeof(EditPosStartNode), 1, fp);

// load total dist

    fread(&EditPosTotalDist, sizeof(EditPosTotalDist), 1, fp);

// loop thru all nodes

    for (i = 0 ; i < EditPosNodeNum ; i++)
    {

// load one file node

        fread(&pan, sizeof(pan), 1, fp);
        VecMulScalar(&pan.Pos, EditScale);
        VecPlusEqVec(&pan.Pos, &EditOffset);

// setup edit ainode

        EditPosNode[i].Pos = pan.Pos;
        EditPosNode[i].Dist = pan.Dist;

        for (j = 0 ; j < POSNODE_MAX_LINKS ; j++)
        {
            if (pan.Prev[j] != -1)
                EditPosNode[i].Prev[j] = EditPosNode + pan.Prev[j];
            else
                EditPosNode[i].Prev[j] = NULL;

            if (pan.Next[j] != -1)
                EditPosNode[i].Next[j] = EditPosNode + pan.Next[j];
            else
                EditPosNode[i].Next[j] = NULL;
        }
    }

// close node file

    fclose(fp);
}

//////////////////////////////////
// calc edit pos node distances //
//////////////////////////////////

void CalcEditPosNodeDistances(void)
{
    long i;
    VEC vec;
    REAL dist, ndist;

// find start node

    ndist = FLT_MAX;

    for (i = 0 ; i < EditPosNodeNum ; i++)
    {
        SubVector(&LEV_StartPos, &EditPosNode[i].Pos, &vec);
        dist = Length(&vec);
        if (dist < ndist)
        {
            ndist = dist;
            EditPosStartNode = i;
        }
    }

// calc node distances

    EditPosTotalDist = 0.0f;

    for (i = 0 ; i < EditPosNodeNum ; i++)
    {
        EditPosNode[i].Dist = 0.0f;
    }

    EditPosNode[EditPosStartNode].Dist = 0.0f;
    CalcOnePosNodeDistance(&EditPosNode[EditPosStartNode], FALSE);
}

////////////////////////////
// calc one node distance //
////////////////////////////

void CalcOnePosNodeDistance(POSNODE *node, long flag)
{
    long i;
    VEC vec;
    REAL dist;

// quit?

//  if (flag && node == &EditPosNode[EditPosStartNode])
//      return;

// loop thru links

    for (i = 0 ; i < POSNODE_MAX_LINKS ; i++) if (node->Prev[i])
    {

// get dist

        SubVector(&node->Pos, &node->Prev[i]->Pos, &vec);
        dist = Length(&vec) + node->Dist;

// start node?

        if (node->Prev[i] == &EditPosNode[EditPosStartNode])
        {
            EditPosTotalDist = dist;
            continue;
        }

// skip if already got lower dist

        if (dist > node->Prev[i]->Dist && node->Prev[i]->Dist)
        {
            continue;
        }

// set dist

        node->Prev[i]->Dist = dist;
        CalcOnePosNodeDistance(node->Prev[i], TRUE);
    }
}

////////////////////
// save pos nodes //
////////////////////

void SaveEditPosNodes(char *file)
{
    long        i, j;
    FILE        *fp;
    FILE_POSNODE pan;
    char        bak[256];

// backup old file

    memcpy(bak, file, strlen(file) - 3);
    sprintf(bak + strlen(file) - 3, "pa-");
    remove(bak);
    rename(file, bak);

// open node file

    fp = fopen(file, "wb");
    if (!fp) return;

// write num

    fwrite(&EditPosNodeNum, sizeof(EditPosNodeNum), 1, fp);

// write start node

    fwrite(&EditPosStartNode, sizeof(EditPosStartNode), 1, fp);

// write total dist

    fwrite(&EditPosTotalDist, sizeof(EditPosTotalDist), 1, fp);

// write out each node

    for (i = 0 ; i < EditPosNodeNum ; i++)
    {

// set file ainode

        pan.Pos = EditPosNode[i].Pos;
        pan.Dist = EditPosNode[i].Dist;

        for (j = 0 ; j < POSNODE_MAX_LINKS ; j++)
        {
            if (EditPosNode[i].Prev[j])
                pan.Prev[j] = (long)(EditPosNode[i].Prev[j] - EditPosNode);
            else
                pan.Prev[j] = -1;

            if (EditPosNode[i].Next[j])
                pan.Next[j] = (long)(EditPosNode[i].Next[j] - EditPosNode);
            else
                pan.Next[j] = -1;
        }

// write it

        fwrite(&pan, sizeof(pan), 1, fp);
    }

// close file

    fclose(fp);
    DumpMessage("Saved pos node File:", file);
}

///////////////////////////////
// load edit pos node models //
///////////////////////////////

void LoadEditPosNodeModels(void)
{
    LoadModel("D:\\edit\\ainode1.m", &EditPosNodeModel, -1, 1, LOADMODEL_FORCE_TPAGE, 100); //$MODIFIED: added "D:\\" at start
}

///////////////////////////////
// free edit pos node models //
///////////////////////////////

void FreeEditPosNodeModels(void)
{
    FreeModel(&EditPosNodeModel, 1);
}

////////////////////////////
// alloc an edit pos node //
////////////////////////////

POSNODE *AllocEditPosNode(void)
{

// full?

    if (EditPosNodeNum >= POSNODE_MAX)
        return NULL;

// inc counter, return slot

    return &EditPosNode[EditPosNodeNum++];
}

///////////////////////////
// free an edit pos node //
///////////////////////////

void FreeEditPosNode(POSNODE *node)
{
    long idx, i, j;

// null any links that reference to this node

    for (i = 0 ; i < EditPosNodeNum ; i++)
    {
        for (j = 0 ; j < POSNODE_MAX_LINKS ; j++)
        {
            if (EditPosNode[i].Prev[j] == node) EditPosNode[i].Prev[j] = NULL;
            if (EditPosNode[i].Next[j] == node) EditPosNode[i].Next[j] = NULL;
        }
    }

// find index into list

    idx = (long)(node - EditPosNode);

// copy all higher nodes down one

    for (i = idx ; i < EditPosNodeNum - 1; i++)
    {
        EditPosNode[i] = EditPosNode[i + 1];
    }

// dec num

    EditPosNodeNum--;

// fix any links that reference higher nodes

    for (i = 0 ; i < EditPosNodeNum ; i++)
    {
        for (j = 0 ; j < POSNODE_MAX_LINKS ; j++)
        {
            if (EditPosNode[i].Prev[j] > node) EditPosNode[i].Prev[j]--;
            if (EditPosNode[i].Next[j] > node) EditPosNode[i].Next[j]--;
        }
    }
}

///////////////////////////
// edit current pos node //
///////////////////////////

void EditPosNodes(void)
{
    long i, j;
    POSNODE *node, *nnode;
    VEC vec;
//$REMOVED (tentative!!) (see below)
//    MAT mat, mat2;
//$END_REMOVAL
    float rad, z, sx, sy;

// quit if not in edit mode

    if (CAM_MainCamera->Type != CAM_EDIT)
    {
        CurrentEditPosNode = NULL;
        return;
    }

// rotate camera?

    if (MouseRight)
    {
//$REMOVED (tentative!!)
//        RotMatrixZYX(&mat, (float)-Mouse.lY / 3072, -(float)Mouse.lX / 3072, 0);
//        MulMatrix(&CAM_MainCamera->WMatrix, &mat, &mat2);
//        CopyMatrix(&mat2, &CAM_MainCamera->WMatrix);
//
//        CAM_MainCamera->WMatrix.m[RY] = 0;
//        NormalizeVector(&CAM_MainCamera->WMatrix.mv[X]);
//        CrossProduct(&CAM_MainCamera->WMatrix.mv[Z], &CAM_MainCamera->WMatrix.mv[X], &CAM_MainCamera->WMatrix.mv[Y]);
//        NormalizeVector(&CAM_MainCamera->WMatrix.mv[Y]);
//        CrossProduct(&CAM_MainCamera->WMatrix.mv[X], &CAM_MainCamera->WMatrix.mv[Y], &CAM_MainCamera->WMatrix.mv[Z]);
//$END_REMOVAL
    }

// reverse direction?

    if (Keys[DIK_R] && !LastKeys[DIK_R] && Keys[DIK_LSHIFT])
    {
        for (i = 0 ; i < EditPosNodeNum ; i++)
        {
            for (j = 0 ; j < POSNODE_MAX_LINKS ; j++)
            {
                node = EditPosNode[i].Prev[j];
                EditPosNode[i].Prev[j] = EditPosNode[i].Next[j];
                EditPosNode[i].Next[j] = node;
            }
        }
    }

// save nodes?

    if (Keys[DIK_LCONTROL] && Keys[DIK_F4] && !LastKeys[DIK_F4])
    {
        CalcEditPosNodeDistances();
        SaveEditPosNodes(GetLevelFilename("pan", FILENAME_MAKE_BODY | FILENAME_GAME_SETTINGS));
    }

// get a current or last node?

    if ((Keys[DIK_RETURN] && !LastKeys[DIK_RETURN]) || (Keys[DIK_BACKSPACE] && !LastKeys[DIK_BACKSPACE]) || (Keys[DIK_SPACE] && !LastKeys[DIK_SPACE]))
    {
        nnode = NULL;
        z = RenderSettings.FarClip;

        node = EditPosNode;
        for (i = 0 ; i < EditPosNodeNum ; i++, node++)
        {
            RotTransVector(&ViewMatrix, &ViewTrans, &node->Pos, &vec);

            if (vec.v[Z] < RenderSettings.NearClip || vec.v[Z] >= RenderSettings.FarClip) continue;

            sx = vec.v[X] * RenderSettings.GeomPers / vec.v[Z] + REAL_SCREEN_XHALF;
            sy = vec.v[Y] * RenderSettings.GeomPers / vec.v[Z] + REAL_SCREEN_YHALF;

            rad = 24 * RenderSettings.GeomPers / vec.v[Z];

            if (MouseXpos > sx - rad && MouseXpos < sx + rad && MouseYpos > sy - rad && MouseYpos < sy + rad)
            {
                if (vec.v[Z] < z)
                {
                    nnode = node;
                    z = vec.v[Z];
                }
            }
        }

        if (nnode)
        {
            if (Keys[DIK_SPACE])
            {
                LastEditPosNode = CurrentEditPosNode;
                CurrentEditPosNode = nnode;

                Keys[DIK_NUMPADPLUS] = TRUE;
                LastKeys[DIK_NUMPADPLUS] = FALSE;
            }
            else if (Keys[DIK_BACKSPACE])
            {
                LastEditPosNode = nnode;
                return;
            }
            else
            {
                CurrentEditPosNode = nnode;
                return;
            }
        }
    }

// new node?

    if (Keys[DIK_INSERT] && !LastKeys[DIK_INSERT])
    {
        if ((node = AllocEditPosNode()))
        {
            GetPosNodePos(&CAM_MainCamera->WPos, MouseXpos, MouseYpos, &node->Pos);

            for (i = 0 ; i < POSNODE_MAX_LINKS ; i++)
            {
                node->Prev[i] = NULL;
                node->Next[i] = NULL;
            }

            CurrentEditPosNode = node;
        }
    }

// quit now if no current edit node

    if (!CurrentEditPosNode) return;

// exit current edit?

    if (Keys[DIK_RETURN] && !LastKeys[DIK_RETURN])
    {
        CurrentEditPosNode = NULL;
        return;
    }

// delete current node?

    if (Keys[DIK_DELETE] && !LastKeys[DIK_DELETE])
    {
        FreeEditPosNode(CurrentEditPosNode);
        CurrentEditPosNode = NULL;
        return;
    }

// create link?

    if (Keys[DIK_NUMPADPLUS] && !LastKeys[DIK_NUMPADPLUS] && LastEditPosNode && CurrentEditPosNode != LastEditPosNode)
    {
        for (i = j = 0 ; i < POSNODE_MAX_LINKS ; i++) if (CurrentEditPosNode->Prev[i] == LastEditPosNode || CurrentEditPosNode->Next[i] == LastEditPosNode) j++;

        if (!j) for (i = 0 ; i < POSNODE_MAX_LINKS ; i++) if (!CurrentEditPosNode->Prev[i])
        {
            for (j = 0 ; j < POSNODE_MAX_LINKS ; j++) if (!LastEditPosNode->Next[j])
            {
                CurrentEditPosNode->Prev[i] = LastEditPosNode;
                LastEditPosNode->Next[j] = CurrentEditPosNode;
                break;
            }
            break;
        }
    }

// delete link?

    if (Keys[DIK_NUMPADMINUS] && !LastKeys[DIK_NUMPADMINUS] && LastEditPosNode && CurrentEditPosNode != LastEditPosNode)
    {
        for (i = 0 ; i < POSNODE_MAX_LINKS ; i++) if (CurrentEditPosNode->Prev[i] == LastEditPosNode)
        {
            for (j = 0 ; j < POSNODE_MAX_LINKS ; j++) if (LastEditPosNode->Next[j] == CurrentEditPosNode)
            {
                CurrentEditPosNode->Prev[i] = NULL;
                LastEditPosNode->Next[j] = NULL;
                break;
            }
            break;
        }
    }

// move?

    if (MouseLeft)
    {
        GetPosNodePos(&CAM_MainCamera->WPos, MouseXpos, MouseYpos, &CurrentEditPosNode->Pos);
    }
}

////////////////
// draw nodes //
////////////////

void DrawPosNodes(void)
{
    long i, k;
    VEC v1, v2;
    WCHAR buf[128];
    short flag;
    POSNODE *node;

// loop thru all nodes

    for (i = 0 ; i < EditPosNodeNum ; i++)
    {
        node = &EditPosNode[i];

// draw it

        flag = MODEL_PLAIN;

        if (i == EditPosStartNode)
        {
            flag |= MODEL_SCALE;
            ModelScale = (float)sin((float)TIME2MS(CurrentTimer()) / 300.0f) * 0.5f + 1.0f;
        }

        if (i == PLR_LocalPlayer->CarAI.FinishDistNode)
        {
            flag |= MODEL_SCALE;
            ModelScale = (float)sin((float)TIME2MS(CurrentTimer()) / 300.0f) * 0.5f + 2.0f;
        }

        if (LastEditPosNode != &EditPosNode[i] || (FrameCount & 4))
        {
            DrawModel(&EditPosNodeModel, &IdentityMatrix, &node->Pos, flag);
        }

// draw link?

        for (k = 0 ; k < POSNODE_MAX_LINKS ; k++) if (EditPosNode[i].Next[k])
        {
            v1.v[X] = EditPosNode[i].Pos.v[X];
            v1.v[Y] = EditPosNode[i].Pos.v[Y];
            v1.v[Z] = EditPosNode[i].Pos.v[Z];

            v2.v[X] = EditPosNode[i].Next[k]->Pos.v[X];
            v2.v[Y] = EditPosNode[i].Next[k]->Pos.v[Y];
            v2.v[Z] = EditPosNode[i].Next[k]->Pos.v[Z];

            DrawLine(&v1, &v2, 0x000000, 0x00ffff);
        }

// draw 'current' axis?

        if (CurrentEditPosNode == &EditPosNode[i])
        {
            DrawAxis(&IdentityMatrix, &node->Pos);
        }

// dump finish dist

        SET_TPAGE(TPAGE_FONT);
        swprintf(buf, L"%ld", (long)EditPosNode[i].Dist);
        RotTransVector(&ViewMatrix, &ViewTrans, &v1, &v2);
        v2.v[X] -= 48.0f;
        v2.v[Y] -= 32.0f;
        DumpText3D(&v2, 16, 32, 0xffffffff, buf);
    }
}

///////////////////////
// get edit node pos //
///////////////////////

void GetPosNodePos(VEC *campos, float xpos, float ypos, VEC *nodepos)
{
    long i;
    NEWCOLLPOLY *poly;
    VEC vec, offset, dest;
    float time, depth, ntime;

// get dest vector

    vec.v[X] = xpos - REAL_SCREEN_XHALF;
    vec.v[Y] = ypos - REAL_SCREEN_YHALF;
    vec.v[Z] = RenderSettings.GeomPers + CAM_MainCamera->Lens;

    RotVector(&CAM_MainCamera->WMatrix, &vec, &offset);
    NormalizeVector(&offset);
    VecMulScalar(&offset, RenderSettings.FarClip);
    AddVector(&offset, &CAM_MainCamera->WPos, &dest);

// loop thru all coll polys

    ntime = 1.0f;

    poly = COL_WorldCollPoly;
    for (i = 0 ; i < COL_NWorldCollPolys ; i++, poly++)
    {
        if (LinePlaneIntersect(campos, &dest, &poly->Plane, &time, &depth))
        {
            if (PlaneDist(&poly->Plane, &CAM_MainCamera->WPos) > 0)
            {
                if (time > 0 && time < ntime)
                {
                    vec.v[X] = campos->v[X] + offset.v[X] * time;
                    vec.v[Y] = campos->v[Y] + offset.v[Y] * time;
                    vec.v[Z] = campos->v[Z] + offset.v[Z] * time;
    
                    if (PointInCollPolyBounds(&vec, poly))
                    {
                        CopyVec(&vec, nodepos);
                        ntime = time;
                    }
                }
            }
        }
    }

    poly = COL_InstanceCollPoly;
    for (i = 0 ; i < COL_NInstanceCollPolys ; i++, poly++)
    {
        if (LinePlaneIntersect(campos, &dest, &poly->Plane, &time, &depth))
        {
            if (PlaneDist(&poly->Plane, &CAM_MainCamera->WPos) > 0)
            {
                if (time > 0 && time < ntime)
                {
                    vec.v[X] = campos->v[X] + offset.v[X] * time;
                    vec.v[Y] = campos->v[Y] + offset.v[Y] * time;
                    vec.v[Z] = campos->v[Z] + offset.v[Z] * time;
    
                    if (PointInCollPolyBounds(&vec, poly))
                    {
                        CopyVec(&vec, nodepos);
                        ntime = time;
                    }
                }
            }
        }
    }

// set default?

    if (ntime == 1.0f)
    {
        SetVector(&vec, 0, 0, RenderSettings.GeomPers);
        RotTransVector(&CAM_MainCamera->WMatrix, &CAM_MainCamera->WPos, &vec, nodepos);
    }
}

/////////////////////////////////
// display current finish dist //
/////////////////////////////////

void DisplayCurrentFinishDist(void)
{
    WCHAR buf[128];

    swprintf(buf, L"Finish %d", (long)PLR_LocalPlayer->CarAI.FinishDist);
    DumpText(0, 128, 8, 16, 0xffffff, buf);
}
