//-----------------------------------------------------------------------------
// File: EditAI.cpp
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"
#include "editai.h"
#include "model.h"
#include "main.h"
#include "camera.h"
#include "input.h"
#include "LevelLoad.h"
#include "draw.h"
#include "text.h"
#include "aizone.h"
#include "timing.h"
#include "player.h"
#include "ghost.h"

#include "SoundEffectEngine.h"


// Functions
short CountAiLinkNodes(void);
void TopSlotAiNodeLinks(AINODE *pNode);


// globals

static long CurrentEditAiNodeBro;
static long EditAiStartNode;
static REAL EditAiNodeTotalDist;

AINODE *CurrentEditAiNode = NULL;
AINODE *LastEditAiNode = NULL;
short EditAiNodeNum;
short EditAiLinkNodeNum;
AINODE *EditAiNode;
MODEL EditAiNodeModel[2];


// Ghost racing line info
VEC     gGhostPath[GHOST_PATH_MAX_SAMPLES];
bool    gbCreateGhostPath = FALSE;
bool    gbValidGhostPath = FALSE;
long    gGhostPathSamples;

// Edit racing line variables
#define MAX_CURVE_NODES             200
#define MAX_CURVE_NODES_MULTIPLY    4

int     gcCurveNodes;
AINODE  *gpRacingLineNode[3] = {NULL, NULL, NULL};
REAL    gRacingLinePos[3][2];
REAL    gOvertakingLinePos[3][2];
bool    gfStoreRacingLine = FALSE;
bool    gfStoreOvertakingLine = FALSE;
bool    gfEditCurve = FALSE;

t_RacingLineUndo    gpRacingLineUndo[MAX_CURVE_NODES+1];


// priority enum list

char *PriorityEnum[] = {
    "Racing Line",
    "Pickup Route",
    "Stairs",
    "Bumpy",
    "25mph Slow Down",
    "Soft Suspension Route",
    "Jump Wall",
    "Title Screen Slow Down",
    "Turbo Line",
    "Long Pickup Route",
    "Short Cut",
    "Long Cut",
    "Barrel Block",
    "Off Throttle",
    "Petrol Throttle",
    "Wilderness",
    "15mph Slow Down",
    "20mph Slow Down",
    "30mph Slow Down",

    "",     // Need to leave this line in !!!!
};

long PriorityColors[] =
{
    0xFFFFFF,       // Racing Line      (White)
    0x007FFF,       // Pickup Route     (Slate Blue)
    0x808080,       // Stairs           ()
    0x5C3317,       // Bumpy            (Baker's Chocolate)
    0xFFFF00,       // 25 Slow Down !       (Yellow)
    0xFFC080,       // Soft Suspension  (Reddish Cream)
    0x808080,       // Jump Wall        ()
    0x808080,       // Title Scr Slow   ()
    0x8E2323,       // Turbo Line       (Turbo Brick)
    0x8E236B,       // Long Pickup Rte  (Maroon)
    0xFF6EC7,       // Short Cut        (Neon Pink)
    0xCC9966,       // Long Cut         (Tan)
    0x808080,       // Barrel Block     ()
    0x42426F,       // Off Throttle     (Cornflower Blue)
    0x7093DB,       // Petrol Off Throt (Dark Turquoise)
    0x808080,       // Wilderness       ()
    0xd0d000,       // 15 Slow Down !       (Yellow)
    0xa0a000,       // 20 Slow Down !       (Yellow)
    0x707000,       // 30 Slow Down !       (Yellow)

    0x000000,
    0x000000,
    0x000000,
    0x000000,
    0x000000,
    0x000000,
    0x000000,
    0x000000,
    0x000000,
    0x000000,
    0x000000,
};


////////////////////////
// init edit ai nodes //
////////////////////////

void InitEditAiNodes(void)
{
    EditAiNode = (AINODE*)malloc(sizeof(AINODE) * MAX_AINODES);
    if (!EditAiNode)
    {
        DumpMessage(NULL, "Can't alloc memory for edit AI nodes!");
        QuitGame();
    }

// Barg-o-matic
    FreeOneTexture(TPAGE_ENVSTILL);
    LoadMipTexture("D:\\gfx\\font.bmp", TPAGE_ENVSTILL, 128, 128, 0, 1, TRUE); //$MODIFIED: added "D:\\" at start
}

////////////////////////
// kill edit ai nodes //
////////////////////////

void KillEditAiNodes(void)
{
    free(EditAiNode);
}

///////////////////
// load ai nodes //
///////////////////

void LoadEditAiNodes(char *file)
{
    long i, j;
    FILE *fp;
    FILE_AINODE fan;
    AINODE  *pNode, *pNodeN;
    VEC     uVec;

// open ainode file

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

// loop thru all ainodes

    fread(&EditAiNodeNum, sizeof(EditAiNodeNum), 1, fp);
    fread(&EditAiLinkNodeNum, sizeof(EditAiLinkNodeNum), 1, fp);

    for (i = 0 ; i < EditAiNodeNum ; i++)
    {

// load one file ainode

        fread(&fan, sizeof(fan), 1, fp);

        VecMulScalar(&fan.Node[0].Pos, EditScale);
        VecPlusEqVec(&fan.Node[0].Pos, &EditOffset);
        VecMulScalar(&fan.Node[1].Pos, EditScale);
        VecPlusEqVec(&fan.Node[1].Pos, &EditOffset);

// setup edit ainode

        EditAiNode[i].Node[0].Speed = fan.Node[0].Speed;
        EditAiNode[i].Node[0].Pos = fan.Node[0].Pos;
        EditAiNode[i].Node[1].Speed = fan.Node[1].Speed;
        EditAiNode[i].Node[1].Pos = fan.Node[1].Pos;

        EditAiNode[i].Priority = fan.Priority;
        EditAiNode[i].StartNode = fan.StartNode;
        EditAiNode[i].RacingLine = fan.RacingLine;
        EditAiNode[i].RacingLineSpeed = fan.RacingLineSpeed;
        EditAiNode[i].CentreSpeed = fan.CentreSpeed;
        EditAiNode[i].FinishDist = fan.FinishDist;
        EditAiNode[i].OvertakingLine = fan.OvertakingLine;
//      EditAiNode[i].linkInfo[0].flags = fan.flags[0];
//      EditAiNode[i].linkInfo[1].flags = fan.flags[1];
        EditAiNode[i].link.flags = fan.flags[0];

        for (j = 0 ; j < MAX_AINODE_LINKS ; j++)
        {
            if (fan.Prev[j] != -1)
                EditAiNode[i].Prev[j] = EditAiNode + fan.Prev[j];
            else
                EditAiNode[i].Prev[j] = NULL;

            if (fan.Next[j] != -1)
                EditAiNode[i].Next[j] = EditAiNode + fan.Next[j];
            else
                EditAiNode[i].Next[j] = NULL;
        }

        // Error check
#if 1
//      if (EditAiNode[i].Next[1] && EditAiNode[i].Prev[1])
        if (EditAiNode[i].Next[1])
        {
            int cGazza;
            for (cGazza = 0; cGazza < MAX_AINODE_LINKS; cGazza++)
            {
                if (EditAiNode[i].Prev[cGazza] && (EditAiNode[i].Prev[cGazza]->Next[1]))
                {
                    DumpMessage("AI node has DOUBLE FORWARD & DOUBLE BACK links !!!!", "Helper: Look for the solid nodes");
                    break;
                }
            }
        }

#endif
    }


// Setup plane normals
    pNode = EditAiNode;
    for (i = 0 ; i < EditAiNodeNum ; i++, pNode++)
    {
        for (j = 0 ; j < MAX_AINODE_LINKS ; j++)
//          j   =   0;
        {
            if (pNodeN = pNode->Next[j])
            {
                uVec.v[X] = pNode->Node[0].Pos.v[X];
                uVec.v[Y] = pNode->Node[0].Pos.v[Y] - Real(64);
                uVec.v[Z] = pNode->Node[0].Pos.v[Z];
///             BuildPlane(&pNode->Node[0].Pos, &uVec, &pNodeN->Node[0].Pos, &pNode->Node[0].plane);
//              BuildPlane(&pNode->Node[0].Pos, &pNodeN->Node[0].Pos, &uVec, &pNode->Node[0].plane);
                BuildPlane(&pNode->Node[0].Pos, &pNodeN->Node[0].Pos, &uVec, &pNode->link.planeEdge[(j*2)+0]);

                uVec.v[X] = pNode->Node[1].Pos.v[X];
                uVec.v[Y] = pNode->Node[1].Pos.v[Y] - Real(64);
                uVec.v[Z] = pNode->Node[1].Pos.v[Z];
//              BuildPlane(&pNode->Node[1].Pos, &uVec, &pNodeN->Node[1].Pos, &pNode->Node[1].plane);
                BuildPlane(&pNode->Node[1].Pos, &uVec, &pNodeN->Node[1].Pos, &pNode->link.planeEdge[(j*2)+1]);
            }
        }
    }


// load start node

    fread(&EditAiStartNode, sizeof(EditAiStartNode), 1, fp);

// load total dist

    fread(&EditAiNodeTotalDist, sizeof(EditAiNodeTotalDist), 1, fp);

// close ainode file

    fclose(fp);
}

/////////////////////////////////
// calc edit ai node distances //
/////////////////////////////////

void CalcEditAiNodeDistances(void)
{
    long i, j, k, flag;
    VEC centre, vec;
    AIZONE *zone;
    REAL dist, ndist;

// find start node

    if (!AiZones)
    {
        EditAiStartNode = 0;
    }
    else
    {
        ndist = 1000000.0f;
        for (i = 0 ; i < EditAiNodeNum ; i++)
        {

// get centre point

            AddVector(&EditAiNode[i].Node[0].Pos, &EditAiNode[i].Node[1].Pos, &centre);
            VecMulScalar(&centre, 0.5f);

// in zone id 0?

            zone = AiZoneHeaders[0].Zones;
            for (j = 0 ; j < AiZoneHeaders[0].Count ; j++, zone++)
            {
                flag = FALSE;
                for (k = 0 ; k < 3 ; k++)
                {
                    dist = PlaneDist(&zone->Plane[k], &centre);
                    if (dist < -zone->Size[k] || dist > zone->Size[k])
                    {
                        flag = TRUE;
                        break;
                    }
                }
            }

// yep, nearest to last zone?

            if (!flag)
            {
                SubVector(&LEV_StartPos, &centre, &vec);
                dist = Length(&vec);
                if (dist < ndist)
                {

// yep, save node num

                    ndist = dist;
                    EditAiStartNode = i;
                }
            }
        }
    }

// calc node distances


    EditAiNodeTotalDist = 0.0f;

    for (i = 0 ; i < EditAiNodeNum ; i++)
    {
        EditAiNode[i].FinishDist = 0.0f;
    }

    EditAiNode[EditAiStartNode].FinishDist = 0.0f;
    CalcOneNodeDistance(&EditAiNode[EditAiStartNode], FALSE);
}

/////////////////////////////
// calc one node distances //
/////////////////////////////

void CalcOneNodeDistance(AINODE *node, long flag)
{
    long i;
    VEC centre, vec;
    REAL dist;

// quit?

//  if (flag && node == &EditAiNode[EditAiStartNode])
//      return;

// get my centre

    AddVector(&node->Node[0].Pos, &node->Node[1].Pos, &centre)
    VecMulScalar(&centre, 0.5f);

// loop thru links

    for (i = 0 ; i < MAX_AINODE_LINKS ; i++) if (node->Prev[i])
    {

// get dist

        AddVector(&node->Prev[i]->Node[0].Pos, &node->Prev[i]->Node[1].Pos, &vec)
        VecMulScalar(&vec, 0.5f);

        SubVector(&vec, &centre, &vec);
        dist = Length(&vec) + node->FinishDist;

// start node?

        if (node->Prev[i] == &EditAiNode[EditAiStartNode])
        {
            EditAiNodeTotalDist = dist;
            continue;
        }

// skip if already got lower dist

        if (dist > node->Prev[i]->FinishDist && node->Prev[i]->FinishDist)
        {
            continue;
        }

// set dist

        node->Prev[i]->FinishDist = dist;
        CalcOneNodeDistance(node->Prev[i], TRUE);
    }
}

///////////////////
// save ai nodes //
///////////////////

void SaveEditAiNodes(char *file)
{
    long        i, j, k;
    FILE        *fp;
    FILE_AINODE fan;
    AINODE      *swap;
    char        bak[256];
    VEC         norm, vec, v1, v2;
    REAL        f, a, b;

// backup old file

    memcpy(bak, file, strlen(file) - 3);
    sprintf(bak + strlen(file) - 3, "fa-");
    remove(bak);
    rename(file, bak);

// open node file

    fp = fopen(file, "wb");
    if (!fp) return;

// write num

    EditAiLinkNodeNum = CountAiLinkNodes();

    fwrite(&EditAiNodeNum, sizeof(EditAiNodeNum), 1, fp);
    fwrite(&EditAiLinkNodeNum, sizeof(EditAiLinkNodeNum), 1, fp);

// write out each ainode

    for (i = 0 ; i < EditAiNodeNum ; i++)
    {

// sort links in left-right order from averaged prev link plane

        SetVector(&norm, Real(0), Real(0), Real(0));

        for (j = 0 ; j < MAX_AINODE_LINKS ; j++)
        {
            if (EditAiNode[i].Prev[j])
            {
                SubVector(&EditAiNode[i].Node[0].Pos, &EditAiNode[i].Prev[j]->Node[0].Pos, &vec);
                NormalizeVector(&vec);
                AddVector(&norm, &vec, &norm);
            }
        }

        f = norm.v[X];
        norm.v[X] = -norm.v[Z];
        norm.v[Z] = f;
        NormalizeVector(&norm);

        for (j = MAX_AINODE_LINKS - 1 ; j ; j--)
        {
            for (k = 0 ; k < j ; k++)
            {
                if (EditAiNode[i].Next[j] && EditAiNode[i].Next[k])
                {
                    SubVector(&EditAiNode[i].Next[j]->Node[0].Pos, &EditAiNode[i].Node[0].Pos, &v1);
                    SubVector(&EditAiNode[i].Next[k]->Node[0].Pos, &EditAiNode[i].Node[0].Pos, &v2);
                    a = DotProduct(&norm, &v1);
                    b = DotProduct(&norm, &v2);
                    if (a > b)
                    {
                        swap = EditAiNode[i].Next[j];
                        EditAiNode[i].Next[j] = EditAiNode[i].Next[k];
                        EditAiNode[i].Next[k] = swap;
                    }
                }
                if (EditAiNode[i].Prev[j] && EditAiNode[i].Prev[k])
                {
                    SubVector(&EditAiNode[i].Prev[j]->Node[0].Pos, &EditAiNode[i].Node[0].Pos, &v1);
                    SubVector(&EditAiNode[i].Prev[k]->Node[0].Pos, &EditAiNode[i].Node[0].Pos, &v2);
                    a = DotProduct(&norm, &v1);
                    b = DotProduct(&norm, &v2);
                    if (a > b)
                    {
                        swap = EditAiNode[i].Prev[j];
                        EditAiNode[i].Prev[j] = EditAiNode[i].Prev[k];
                        EditAiNode[i].Prev[k] = swap;
                    }
                }
            }
        }

// push links to top slots
        TopSlotAiNodeLinks(&EditAiNode[i]);

// set file ainode

        fan.Node[0].Speed = EditAiNode[i].Node[0].Speed;
        fan.Node[0].Pos = EditAiNode[i].Node[0].Pos;
        fan.Node[1].Speed = EditAiNode[i].Node[1].Speed;
        fan.Node[1].Pos = EditAiNode[i].Node[1].Pos;

        fan.Priority = EditAiNode[i].Priority;
        fan.StartNode = EditAiNode[i].StartNode;
        fan.RacingLine = EditAiNode[i].RacingLine;
        fan.RacingLineSpeed = EditAiNode[i].RacingLineSpeed;
        fan.CentreSpeed = EditAiNode[i].CentreSpeed;
        fan.FinishDist = EditAiNode[i].FinishDist;
        fan.OvertakingLine = EditAiNode[i].OvertakingLine;
//      fan.flags[0] = EditAiNode[i].linkInfo[0].flags;
//      fan.flags[1] = EditAiNode[i].linkInfo[1].flags;
        fan.flags[0] = EditAiNode[i].link.flags;
        fan.flags[1] = EditAiNode[i].link.flags;

        for (j = 0 ; j < MAX_AINODE_LINKS ; j++)
        {
            if (EditAiNode[i].Prev[j])
                fan.Prev[j] = (long)(EditAiNode[i].Prev[j] - EditAiNode);
            else
                fan.Prev[j] = -1;

            if (EditAiNode[i].Next[j])
                fan.Next[j] = (long)(EditAiNode[i].Next[j] - EditAiNode);
            else
                fan.Next[j] = -1;
        }

// write it

        fwrite(&fan, sizeof(fan), 1, fp);
    }

// write start node

    fwrite(&EditAiStartNode, sizeof(EditAiStartNode), 1, fp);

// write total dist

    fwrite(&EditAiNodeTotalDist, sizeof(EditAiNodeTotalDist), 1, fp);

// close file

    fclose(fp);
    DumpMessage("Saved AI node File:", file);
}

/////////////////////////////
// load edit ainode models //
/////////////////////////////

void LoadEditAiNodeModels(void)
{
//$MODIFIED
//    LoadModel("edit\\ainode2.m", &EditAiNodeModel[0], -1, 1, LOADMODEL_FORCE_TPAGE, 100);
//    LoadModel("edit\\ainode1.m", &EditAiNodeModel[1], -1, 1, LOADMODEL_FORCE_TPAGE, 100);
    LoadModel("D:\\edit\\ainode2.m", &EditAiNodeModel[0], -1, 1, LOADMODEL_FORCE_TPAGE, 100);
    LoadModel("D:\\edit\\ainode1.m", &EditAiNodeModel[1], -1, 1, LOADMODEL_FORCE_TPAGE, 100);
//$END_MODIFICATIONS
}

/////////////////////////////
// free edit ainode models //
/////////////////////////////

void FreeEditAiNodeModels(void)
{
    FreeModel(&EditAiNodeModel[0], 1);
    FreeModel(&EditAiNodeModel[1], 1);
}

///////////////////////////
// alloc an edit ai node //
///////////////////////////

AINODE *AllocEditAiNode(void)
{

// full?

    if (EditAiNodeNum >= MAX_AINODES)
        return NULL;

// inc counter, return slot

    return &EditAiNode[EditAiNodeNum++];
}

//////////////////////////
// free an edit ai node //
//////////////////////////

void FreeEditAiNode(AINODE *node)
{
    long idx, i, j;

// null any links that reference to this node

    for (i = 0 ; i < EditAiNodeNum ; i++)
    {
        for (j = 0 ; j < MAX_AINODE_LINKS ; j++)
        {
            if (EditAiNode[i].Prev[j] == node) EditAiNode[i].Prev[j] = NULL;
            if (EditAiNode[i].Next[j] == node) EditAiNode[i].Next[j] = NULL;
        }
    }

// find index into list

    idx = (long)(node - EditAiNode);

// copy all higher nodes down one

    for (i = idx ; i < EditAiNodeNum - 1; i++)
    {
        EditAiNode[i] = EditAiNode[i + 1];
    }

// dec num

    EditAiNodeNum--;

// fix any links that reference higher nodes

    for (i = 0 ; i < EditAiNodeNum ; i++)
    {
        for (j = 0 ; j < MAX_AINODE_LINKS ; j++)
        {
            if (EditAiNode[i].Prev[j] > node) EditAiNode[i].Prev[j]--;
            if (EditAiNode[i].Next[j] > node) EditAiNode[i].Next[j]--;
        }
    }
}

//////////////////////////
// edit current AI node //
//////////////////////////

void EditAiNodes(void)
{
    long i, j, k, l, nbro;
    AINODE *node, *nnode, *next;
    VEC vec;
//$REMOVED    MAT mat, mat2;
    ONE_AINODE tempnode;
    float rad, z, sx, sy;
    bool linkChanged;
    PLAYER *player;

// Create ghost path ?
    gbCreateGhostPath = (!Keys[DIK_LSHIFT] && !Keys[DIK_LCONTROL] && Keys[DIK_6] && !LastKeys[DIK_6]);
    if (gbCreateGhostPath)
    {
        if (gbValidGhostPath)
        {
            gbValidGhostPath = FALSE;
            gbCreateGhostPath = FALSE;
        }
    }
    if (Keys[DIK_LSHIFT] && !Keys[DIK_LCONTROL] &&  Keys[DIK_6] && !LastKeys[DIK_6])
    {
        if (CreateRacingLineFromGhostData())
            gpRacingLineNode[0] = gpRacingLineNode[1] = gpRacingLineNode[2] = 0;
    }

// quit if not in edit mode

    if (CAM_MainCamera->Type != CAM_EDIT)
    {
        CurrentEditAiNode = NULL;
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



// Edit Racing Line
    EditRacingLine();


// reverse left / right nodes?

    if (Keys[DIK_T] && !LastKeys[DIK_T])
    {
        if (Keys[DIK_LSHIFT])
        {
            for (i = 0 ; i < EditAiNodeNum ; i++)
            {
                tempnode = EditAiNode[i].Node[0];
                EditAiNode[i].Node[0] = EditAiNode[i].Node[1];
                EditAiNode[i].Node[1] = tempnode;

                EditAiNode[i].RacingLine = 1 - EditAiNode[i].RacingLine;
            }
        }
        else if (CurrentEditAiNode)
        {
            tempnode = CurrentEditAiNode->Node[0];
            CurrentEditAiNode->Node[0] = CurrentEditAiNode->Node[1];
            CurrentEditAiNode->Node[1] = tempnode;

            CurrentEditAiNode->RacingLine = 1 - CurrentEditAiNode->RacingLine;
        }
    }

// reverse direction?

    if (Keys[DIK_R] && !LastKeys[DIK_R])
    {
        if (Keys[DIK_LSHIFT]) {
            for (i = 0 ; i < EditAiNodeNum ; i++)
            {
                for (j = 0 ; j < MAX_AINODE_LINKS ; j++)
                {
                    node = EditAiNode[i].Prev[j];
                    EditAiNode[i].Prev[j] = EditAiNode[i].Next[j];
                    EditAiNode[i].Next[j] = node;
                }
            }
        }
        else 
        { // Reverse one node
            node = CurrentEditAiNode;
            for (j = 0 ; j < MAX_AINODE_LINKS ; j++) if (node != NULL) 
            {
                // Turn all next pointers on prev node to prev pointers
                //prev = CurrentEditAiNode->Prev[j];
                /*if (prev != NULL) {
                    for (k = 0; k < MAX_AINODE_LINKS; k++) {
                        if (prev->Next[k] == node) {
                            prev->Next[k] = NULL;
                            for (l = 0; l < MAX_AINODE_LINKS; l++) {
                                if (prev->Prev[l] == NULL) {
                                    prev->Prev[l] = node;
                                }
                            }
                        }
                    }
                }*/

                next = CurrentEditAiNode->Next[j];
                // Turn next node into a prev node (or delete it if no prev slots remaining
                linkChanged = FALSE;
                if (next != NULL) {
                    CurrentEditAiNode->Next[j] = NULL;
                    for (k = 0; k < MAX_AINODE_LINKS; k++) {
                        if (node->Prev[k] == NULL) {
                            node->Prev[k] = next;
                            linkChanged = TRUE;
                            break;
                        }
                    }

                    // Turn all prev pointers on next node to next pointers (if link still exists)
                    for (k = 0; k < MAX_AINODE_LINKS; k++) {
                        if (next->Prev[k] == node) {
                            next->Prev[k] = NULL;
                            if (linkChanged) {
                                for (l = 0; l < MAX_AINODE_LINKS; l++) {
                                    if (next->Next[l] == NULL) {
                                        next->Next[l] = node;
                                        break;
                                    }
                                }
                            }
                        }
                    }

                }

            }
        }
    }

// save ai nodes?

    if (Keys[DIK_LCONTROL] && Keys[DIK_F4] && !LastKeys[DIK_F4])
    {
        CalcEditAiNodeDistances();
        SaveEditAiNodes(GetLevelFilename("fan", FILENAME_MAKE_BODY | FILENAME_GAME_SETTINGS));

        LoadEditAiNodes(GetLevelFilename("fan", FILENAME_MAKE_BODY | FILENAME_GAME_SETTINGS));

        FreeAiNodes();
        LoadAiNodes(GetLevelFilename("fan", FILENAME_MAKE_BODY | FILENAME_GAME_SETTINGS));
        ZoneAiNodes();

        for (player = PLR_PlayerHead ; player ; player = player->next)
        {
//          CAI_InitCarAI(player, CAI_SK_RACER);
            CAI_InitCarAI(player, player->car.CarType);
        }
    }

// get a current or last ai node?

    if ((Keys[DIK_RETURN] && !LastKeys[DIK_RETURN]) || (Keys[DIK_BACKSPACE] && !LastKeys[DIK_BACKSPACE]) || (Keys[DIK_SPACE] && !LastKeys[DIK_SPACE]))
    {
        nnode = NULL;
        z = RenderSettings.FarClip;

        node = EditAiNode;
        for (i = 0 ; i < EditAiNodeNum ; i++, node++)
        {
            for (j = 0 ; j < 2 ; j++)
            {
                RotTransVector(&ViewMatrix, &ViewTrans, &node->Node[j].Pos, &vec);

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
                        nbro = j;
                    }
                }
            }
        }
        if (nnode)
        {
            if (Keys[DIK_SPACE])
            {
                LastEditAiNode = CurrentEditAiNode;
                CurrentEditAiNode = nnode;
                CurrentEditAiNodeBro = nbro;

                Keys[DIK_NUMPADPLUS] = TRUE;
                LastKeys[DIK_NUMPADPLUS] = FALSE;
            }
            else if (Keys[DIK_BACKSPACE])
            {
                LastEditAiNode = nnode;
                return;
            }
            else
            {
                CurrentEditAiNode = nnode;
                CurrentEditAiNodeBro = nbro;
                return;
            }
        }
        else
        {
            if (Keys[DIK_BACKSPACE])
            {
                LastEditAiNode = NULL;
                return;
            }
        }
    }

// new ai node?

    if (Keys[DIK_INSERT] && !LastKeys[DIK_INSERT])
    {
        if ((node = AllocEditAiNode()))
        {
            GetEditNodePos(&CAM_MainCamera->WPos, MouseXpos, MouseYpos, &node->Node[1].Pos);

            node->Node[0].Pos.v[X] = node->Node[1].Pos.v[X];
            node->Node[0].Pos.v[Y] = node->Node[1].Pos.v[Y] - 64;
            node->Node[0].Pos.v[Z] = node->Node[1].Pos.v[Z];

            node->Node[0].Speed = 30;
            node->Node[1].Speed = 30;

            node->Priority = 0;
            node->RacingLine = 0.5f;
            node->RacingLineSpeed = 30;
            node->CentreSpeed = 30;
            node->OvertakingLine = 0.5f;

            node->Prev[0] = NULL;
            node->Prev[1] = NULL;
            node->Next[0] = NULL;
            node->Next[1] = NULL;

            CurrentEditAiNode = node;
            CurrentEditAiNodeBro = 1;
        }
    }


// quit now if no current edit ai node

    if (!CurrentEditAiNode) return;

// exit current edit?

    if (Keys[DIK_RETURN] && !LastKeys[DIK_RETURN])
    {
        CurrentEditAiNode = NULL;
        return;
    }

// delete current edit node?

    if (Keys[DIK_DELETE] && !LastKeys[DIK_DELETE])
    {
        FreeEditAiNode(CurrentEditAiNode);
        CurrentEditAiNode = NULL;
        return;
    }

// change speeds?
#if 0   // GAZZA
    if (Keys[DIK_LSHIFT])
        LastKeys[DIK_1] = LastKeys[DIK_2] = LastKeys[DIK_3] = LastKeys[DIK_4] = LastKeys[DIK_5] = LastKeys[DIK_6] = LastKeys[DIK_7] = LastKeys[DIK_8] = 0;

    if (Keys[DIK_1] && !LastKeys[DIK_1] && CurrentEditAiNode->Node[0].Speed) CurrentEditAiNode->Node[0].Speed--;
    if (Keys[DIK_2] && !LastKeys[DIK_2] && CurrentEditAiNode->Node[0].Speed < 100) CurrentEditAiNode->Node[0].Speed++;

    if (Keys[DIK_3] && !LastKeys[DIK_3] && CurrentEditAiNode->Node[1].Speed) CurrentEditAiNode->Node[1].Speed--;
    if (Keys[DIK_4] && !LastKeys[DIK_4] && CurrentEditAiNode->Node[1].Speed < 100) CurrentEditAiNode->Node[1].Speed++;

    if (Keys[DIK_5] && !LastKeys[DIK_5] && CurrentEditAiNode->RacingLineSpeed) CurrentEditAiNode->RacingLineSpeed--;
    if (Keys[DIK_6] && !LastKeys[DIK_6] && CurrentEditAiNode->RacingLineSpeed < 100) CurrentEditAiNode->RacingLineSpeed++;

    if (Keys[DIK_7] && !LastKeys[DIK_7] && CurrentEditAiNode->CentreSpeed) CurrentEditAiNode->CentreSpeed--;
    if (Keys[DIK_8] && !LastKeys[DIK_8] && CurrentEditAiNode->CentreSpeed < 100) CurrentEditAiNode->CentreSpeed++;
#endif


// change priority

    if (Keys[DIK_NUMPADENTER] && !LastKeys[DIK_NUMPADENTER])
    {
        if (Keys[DIK_LSHIFT] || Keys[DIK_RSHIFT])
        {
            if (CurrentEditAiNode->Priority == 0)
                CurrentEditAiNode->Priority = AIN_TYPE_NUM-1;
            else
                CurrentEditAiNode->Priority--;
        }
        else
        {
            CurrentEditAiNode->Priority++;
        }
    }
    if (!strlen(PriorityEnum[CurrentEditAiNode->Priority])) CurrentEditAiNode->Priority = 0;

// change start node

    if (Keys[DIK_NUMPAD0] && !LastKeys[DIK_NUMPAD0])
    {
        for (i = 0 ; i < EditAiNodeNum ; i++)
            EditAiNode[i].StartNode = FALSE;

        CurrentEditAiNode->StartNode = TRUE;
    }

// default overtaking line?

//  if (Keys[DIK_LSHIFT] && ! Keys[DIK_LCONTROL] && Keys[DIK_SPACE] && !LastKeys[DIK_SPACE])
    if (Keys[DIK_LSHIFT] && !Keys[DIK_LCONTROL] && !Keys[DIK_RSHIFT] && !Keys[DIK_RCONTROL]&& Keys[DIK_SPACE] && !LastKeys[DIK_SPACE])
    {
        for (i = 0 ; i < EditAiNodeNum ; i++)
        {
            EditAiNode[i].OvertakingLine = EditAiNode[i].RacingLine;
        }
    }


// clear walls (and flags)?

//  if (Keys[DIK_LSHIFT] && Keys[DIK_LCONTROL] && Keys[DIK_SPACE] && !LastKeys[DIK_SPACE])
    if (Keys[DIK_LSHIFT] && Keys[DIK_LCONTROL] && !Keys[DIK_RSHIFT] && !Keys[DIK_RCONTROL]&& Keys[DIK_SPACE] && !LastKeys[DIK_SPACE])
    {
        for (i = 0 ; i < EditAiNodeNum ; i++)
        {
//          EditAiNode[i].linkInfo[0].flags = 0;
//          EditAiNode[i].linkInfo[1].flags = 0;
            EditAiNode[i].link.flags = 0;
        }
    }

// Clear priority ?
    if (Keys[DIK_LSHIFT] && Keys[DIK_LCONTROL] && Keys[DIK_RSHIFT] && Keys[DIK_RCONTROL]&& Keys[DIK_SPACE] && !LastKeys[DIK_SPACE])
    {
        for (i = 0 ; i < EditAiNodeNum ; i++)
        {
            EditAiNode[i].Priority = AIN_TYPE_RACINGLINE;
        }
    }

// change racing line?

    if (Keys[DIK_NUMPADSLASH]) CurrentEditAiNode->RacingLine -= 0.004f;
    if (Keys[DIK_NUMPADSTAR]) CurrentEditAiNode->RacingLine += 0.004f;
    if (CurrentEditAiNode->RacingLine < 0.0f) CurrentEditAiNode->RacingLine = 0.0f;
    if (CurrentEditAiNode->RacingLine > 1.0f) CurrentEditAiNode->RacingLine = 1.0f;

// change overtaking line?

    if (Keys[DIK_NUMPAD8]) CurrentEditAiNode->OvertakingLine -= 0.004f;
    if (Keys[DIK_NUMPAD9]) CurrentEditAiNode->OvertakingLine += 0.004f;
    if (CurrentEditAiNode->OvertakingLine < 0.0f) CurrentEditAiNode->OvertakingLine = 0.0f;
    if (CurrentEditAiNode->OvertakingLine > 1.0f) CurrentEditAiNode->OvertakingLine = 1.0f;

// create link?

    if (Keys[DIK_NUMPADPLUS] && !LastKeys[DIK_NUMPADPLUS] && LastEditAiNode && CurrentEditAiNode != LastEditAiNode)
    {
        for (i = j = 0 ; i < MAX_AINODE_LINKS ; i++) if (CurrentEditAiNode->Prev[i] == LastEditAiNode || CurrentEditAiNode->Next[i] == LastEditAiNode) j++;

        if (!j) for (i = 0 ; i < MAX_AINODE_LINKS ; i++) if (!CurrentEditAiNode->Prev[i])
        {
            for (j = 0 ; j < MAX_AINODE_LINKS ; j++) if (!LastEditAiNode->Next[j])
            {
                CurrentEditAiNode->Prev[i] = LastEditAiNode;
                LastEditAiNode->Next[j] = CurrentEditAiNode;
                break;
            }
            break;
        }
    }

// delete link?

    if (Keys[DIK_NUMPADMINUS] && !LastKeys[DIK_NUMPADMINUS] && LastEditAiNode && CurrentEditAiNode != LastEditAiNode)
    {
        for (i = 0 ; i < MAX_AINODE_LINKS ; i++) if (CurrentEditAiNode->Prev[i] == LastEditAiNode)
        {
            for (j = 0 ; j < MAX_AINODE_LINKS ; j++) if (LastEditAiNode->Next[j] == CurrentEditAiNode)
            {
                CurrentEditAiNode->Prev[i] = NULL;
                LastEditAiNode->Next[j] = NULL;

                TopSlotAiNodeLinks(CurrentEditAiNode);
                TopSlotAiNodeLinks(LastEditAiNode);

                break;
            }
            break;
        }
    }

// move?

    if (MouseLeft)
    {
        GetEditNodePos(&CAM_MainCamera->WPos, MouseXpos, MouseYpos, &CurrentEditAiNode->Node[CurrentEditAiNodeBro].Pos);
    }
}

///////////////////
// draw AI nodes //
///////////////////

void DrawAiNodes(void)
{
    long i, j, k;
    ONE_AINODE *node;
    VEC v1, v2;
    WCHAR buf[128];
    short flag;
    VEC     coord[4];
    long    rgb[4];
    int     gazzaFlag;

// Draw racing line edit data
    DrawEditRacingLine();

    EAI_CreateGhostPath(GHO_GhostPlayer);
    EAI_RenderGhostPath();

// loop thru all nodes

    for (i = 0 ; i < EditAiNodeNum ; i++)
    {
        for (j = 0 ; j < 2 ; j++)
        {
            node = &EditAiNode[i].Node[j];

// Draw error data
            // Double Next & Prev Nodes ?
#if 1
            gazzaFlag = 0;
            if (EditAiNode[i].Next[1])
            {
                int cGazza;
                for (cGazza = 0; cGazza < MAX_AINODE_LINKS; cGazza++)
                {
                    if (EditAiNode[i].Prev[cGazza] && (EditAiNode[i].Prev[cGazza]->Next[1]))
                    {
                        gazzaFlag = 1;
                        break;
                    }
                }
            }

            if (gazzaFlag)
            {
                AINODE  *pNode;
                VEC     line[4];
//              VEC     line2[4];
                long    rgb[4];
                int     cLink;

                SET_TPAGE(-1);
                rgb[0] = rgb[1] = rgb[2] = rgb[3] = 0x8080FFc0;

                pNode = &EditAiNode[i];

                for (cLink = 0; cLink < MAX_AINODE_LINKS; cLink++)
                {
                    CopyVec(&pNode->Node[0].Pos, &line[0]);
                    CopyVec(&pNode->Next[cLink]->Node[0].Pos, &line[1]);
                    CopyVec(&pNode->Next[cLink]->Node[1].Pos, &line[2]);
                    CopyVec(&pNode->Node[1].Pos, &line[3]);
                    line[0].v[Y] -= TO_LENGTH(REAL(20));
                    line[1].v[Y] -= TO_LENGTH(REAL(20));
                    line[2].v[Y] -= TO_LENGTH(REAL(20));
                    line[3].v[Y] -= TO_LENGTH(REAL(20));
                    DrawNearClipPolyTEX0(line, rgb, 4);

                    CopyVec(&pNode->Node[0].Pos, &line[0]);
                    CopyVec(&pNode->Node[1].Pos, &line[1]);
                    CopyVec(&pNode->Next[cLink]->Node[1].Pos, &line[2]);
                    CopyVec(&pNode->Next[cLink]->Node[0].Pos, &line[3]);
                    line[0].v[Y] -= TO_LENGTH(REAL(20));
                    line[1].v[Y] -= TO_LENGTH(REAL(20));
                    line[2].v[Y] -= TO_LENGTH(REAL(20));
                    line[3].v[Y] -= TO_LENGTH(REAL(20));
                    DrawNearClipPolyTEX0(line, rgb, 4);
                }
            }
#endif

// draw it

            flag = MODEL_PLAIN;

            if (i == EditAiStartNode)
            {
                flag |= MODEL_SCALE;
                ModelScale = (float)sin((float)TIME2MS(CurrentTimer()) / 300.0f) * 0.5f + 1.0f;
            }

                {
                    AINODE  *pNode, *pNodeN;
                    REAL    d[4];
                    int     s[4];
                    int     sum;
                    int     cG;

                    pNode = &EditAiNode[i];
                    for (cG = 0 ; cG < MAX_AINODE_LINKS ; cG++)
                    {
                        if (pNodeN = pNode->Next[cG])
                        {
                            d[0] = VecDotPlane(&pNode->Node[0].Pos, &pNode->link.planeEdge[(cG*2)+1]);
                            d[1] = VecDotPlane(&pNodeN->Node[0].Pos, &pNode->link.planeEdge[(cG*2)+1]);
                            d[2] = VecDotPlane(&pNode->Node[1].Pos, &pNode->link.planeEdge[(cG*2)+0]);
                            d[3] = VecDotPlane(&pNodeN->Node[1].Pos, &pNode->link.planeEdge[(cG*2)+0]);
                            s[0] = Sign(d[0]);
                            s[1] = Sign(d[1]);
                            s[2] = Sign(d[2]);
                            s[3] = Sign(d[3]);
                            sum = s[0] + s[1] + s[2] + s[3];

                    //      if (Sign(dist[0]) != Sign(dist[1]))
                            if ((sum > -4) && (sum < 4))
                            {
                                flag |= MODEL_SCALE;
                                ModelScale = (float)sin((float)TIME2MS(CurrentTimer()) / 150.0f) * 3.0f + 3.0f;
                            }
                        }
                    }

                    // If no forward link, error !!!!!!!
//                  if (!pNode->Next[0])
//                  if ((pNode->Next[0] == NULL) && (pNode->Next[1] == NULL))
                    if (EditAiNode[i].Next[0] == NULL)
                    {
                        flag |= MODEL_SCALE;
                        ModelScale = (float)sin((float)TIME2MS(CurrentTimer()) / 100.0f) * 2.0f + 2.0f;
                    }
/*
                    REAL dist[2];
                    dist[0] = VecDotPlane(&EditAiNode[i].Node[0].Pos, &EditAiNode[i].link.planeEdge[1]);
                    dist[1] = VecDotPlane(&EditAiNode[i].Node[1].Pos, &EditAiNode[i].link.planeEdge[0]);
                    if (Sign(dist[0]) != Sign(dist[1]))
                    {
                        flag |= MODEL_SCALE;
                        ModelScale = (float)sin((float)TIME2MS(CurrentTimer()) / 150.0f) * 3.0f + 3.0f;
                    }
*/
                }

            if (LastEditAiNode != &EditAiNode[i] || (FrameCount & 4))
            {
                DrawModel(&EditAiNodeModel[j], &IdentityMatrix, &node->Pos, flag);
            }

// draw link?

            for (k = 0 ; k < MAX_AINODE_LINKS ; k++) if (EditAiNode[i].Next[k])
            {
                v1.v[X] = EditAiNode[i].Node[j].Pos.v[X];
                v1.v[Y] = EditAiNode[i].Node[j].Pos.v[Y];
                v1.v[Z] = EditAiNode[i].Node[j].Pos.v[Z];

                v2.v[X] = EditAiNode[i].Next[k]->Node[j].Pos.v[X];
                v2.v[Y] = EditAiNode[i].Next[k]->Node[j].Pos.v[Y];
                v2.v[Z] = EditAiNode[i].Next[k]->Node[j].Pos.v[Z];

//              if (k && (TIME2MS(CurrentTimer()) & 256))
                {
                    if (!j)
                    {
                        DrawLine(&v1, &v2, 0x000000, 0x800000);
                    }
                    else
                    {
                        DrawLine(&v1, &v2, 0x000000, 0x008000);
                    }
                }


                // Draw wall ?
                SET_TPAGE(-1);
//              BLEND_ALPHA();

                if (EditAiNode[i].link.flags & AIN_LF_WALL_LEFT)
                {
                    rgb[0] = rgb[1] = rgb[2] = rgb[3] = 0x80FF80c0;
                    CopyVec(&EditAiNode[i].Node[1].Pos, &coord[0]);
                    CopyVec(&EditAiNode[i].Node[1].Pos, &coord[1]);
///                 CopyVec(&EditAiNode[i].Next[k]->Node[1].Pos, &coord[2]);
///                 CopyVec(&EditAiNode[i].Next[k]->Node[1].Pos, &coord[3]);
//                  CopyVec(&EditAiNode[i].Next[j]->Node[1].Pos, &coord[2]);
//                  CopyVec(&EditAiNode[i].Next[j]->Node[1].Pos, &coord[3]);
                    CopyVec(&EditAiNode[i].Next[0]->Node[1].Pos, &coord[2]);
                    CopyVec(&EditAiNode[i].Next[0]->Node[1].Pos, &coord[3]);
                    coord[1].v[Y] -= Real(100);
                    coord[2].v[Y] -= Real(100);

                    DrawNearClipPolyTEX0(coord, rgb, 4);
                }
                if (EditAiNode[i].Next[j] &&
                    (EditAiNode[i].link.flags & AIN_LF_WALL_RIGHT))
                {
                    rgb[0] = rgb[1] = rgb[2] = rgb[3] = 0x8080FFc0;
                    CopyVec(&EditAiNode[i].Node[0].Pos, &coord[0]);
                    CopyVec(&EditAiNode[i].Node[0].Pos, &coord[1]);
//                  CopyVec(&EditAiNode[i].Next[k]->Node[0].Pos, &coord[2]);
//                  CopyVec(&EditAiNode[i].Next[k]->Node[0].Pos, &coord[3]);
                    CopyVec(&EditAiNode[i].Next[j]->Node[0].Pos, &coord[2]);
                    CopyVec(&EditAiNode[i].Next[j]->Node[0].Pos, &coord[3]);
                    coord[1].v[Y] -= Real(100);
                    coord[2].v[Y] -= Real(100);

                    DrawNearClipPolyTEX0(coord, rgb, 4);
                }
            }

            /*for (k = 0 ; k < MAX_AINODE_LINKS ; k++) if (EditAiNode[i].Prev[k])
            {
                v1.v[X] = EditAiNode[i].Node[j].Pos.v[X];
                v1.v[Y] = EditAiNode[i].Node[j].Pos.v[Y] - 50;
                v1.v[Z] = EditAiNode[i].Node[j].Pos.v[Z];

                v2.v[X] = EditAiNode[i].Prev[k]->Node[j].Pos.v[X];
                v2.v[Y] = EditAiNode[i].Prev[k]->Node[j].Pos.v[Y] - 50;
                v2.v[Z] = EditAiNode[i].Prev[k]->Node[j].Pos.v[Z];

                if (!j)
                {
                    DrawLine(&v1, &v2, 0x000000, 0xff0000);
                }
                else
                {
                    DrawLine(&v1, &v2, 0x000000, 0x00ff00);
                }
            }*/


// draw 'current' axis?

            if (CurrentEditAiNode == &EditAiNode[i] && CurrentEditAiNodeBro == j)
            {
                DrawAxis(&IdentityMatrix, &node->Pos);
            }
        }

// draw overtaking line

        for (j = 0 ; j < MAX_AINODE_LINKS ; j++) if (EditAiNode[i].Next[j])
        {
            v1.v[X] = (EditAiNode[i].Node[0].Pos.v[X] * EditAiNode[i].OvertakingLine) + (EditAiNode[i].Node[1].Pos.v[X] * (1 - EditAiNode[i].OvertakingLine));
            v1.v[Y] = (EditAiNode[i].Node[0].Pos.v[Y] * EditAiNode[i].OvertakingLine) + (EditAiNode[i].Node[1].Pos.v[Y] * (1 - EditAiNode[i].OvertakingLine));
            v1.v[Z] = (EditAiNode[i].Node[0].Pos.v[Z] * EditAiNode[i].OvertakingLine) + (EditAiNode[i].Node[1].Pos.v[Z] * (1 - EditAiNode[i].OvertakingLine));

            v2.v[X] = (EditAiNode[i].Next[j]->Node[0].Pos.v[X] * EditAiNode[i].Next[j]->OvertakingLine) + (EditAiNode[i].Next[j]->Node[1].Pos.v[X] * (1 - EditAiNode[i].Next[j]->OvertakingLine));
            v2.v[Y] = (EditAiNode[i].Next[j]->Node[0].Pos.v[Y] * EditAiNode[i].Next[j]->OvertakingLine) + (EditAiNode[i].Next[j]->Node[1].Pos.v[Y] * (1 - EditAiNode[i].Next[j]->OvertakingLine));
            v2.v[Z] = (EditAiNode[i].Next[j]->Node[0].Pos.v[Z] * EditAiNode[i].Next[j]->OvertakingLine) + (EditAiNode[i].Next[j]->Node[1].Pos.v[Z] * (1 - EditAiNode[i].Next[j]->OvertakingLine));

            DrawLine(&v1, &v2, 0xff00ff, 0xff00ff);
        }

// draw racing line

        for (j = 0 ; j < MAX_AINODE_LINKS ; j++) if (EditAiNode[i].Next[j])
        {
            v1.v[X] = (EditAiNode[i].Node[0].Pos.v[X] * EditAiNode[i].RacingLine) + (EditAiNode[i].Node[1].Pos.v[X] * (1 - EditAiNode[i].RacingLine));
            v1.v[Y] = (EditAiNode[i].Node[0].Pos.v[Y] * EditAiNode[i].RacingLine) + (EditAiNode[i].Node[1].Pos.v[Y] * (1 - EditAiNode[i].RacingLine));
            v1.v[Z] = (EditAiNode[i].Node[0].Pos.v[Z] * EditAiNode[i].RacingLine) + (EditAiNode[i].Node[1].Pos.v[Z] * (1 - EditAiNode[i].RacingLine));

            v2.v[X] = (EditAiNode[i].Next[j]->Node[0].Pos.v[X] * EditAiNode[i].Next[j]->RacingLine) + (EditAiNode[i].Next[j]->Node[1].Pos.v[X] * (1 - EditAiNode[i].Next[j]->RacingLine));
            v2.v[Y] = (EditAiNode[i].Next[j]->Node[0].Pos.v[Y] * EditAiNode[i].Next[j]->RacingLine) + (EditAiNode[i].Next[j]->Node[1].Pos.v[Y] * (1 - EditAiNode[i].Next[j]->RacingLine));
            v2.v[Z] = (EditAiNode[i].Next[j]->Node[0].Pos.v[Z] * EditAiNode[i].Next[j]->RacingLine) + (EditAiNode[i].Next[j]->Node[1].Pos.v[Z] * (1 - EditAiNode[i].Next[j]->RacingLine));

//          DrawLine(&v1, &v2, 0xffffff, 0xffffff);
            DrawLine(&v1, &v2, PriorityColors[EditAiNode[i].Priority], PriorityColors[EditAiNode[i].Priority]);
        }

// dump misc shit

        SET_TPAGE(TPAGE_FONT);
        RotTransVector(&ViewMatrix, &ViewTrans, &v1, &v2);
        v2.v[X] -= 48.0f;
        v2.v[Y] -= 32.0f;
        swprintf(buf, L"%ld", (long)EditAiNode[i].FinishDist);
        DumpText3D(&v2, 16, 32, 0xffffffff, buf);

/*      long p0, p1, n0, n1;
        p0 = (long)(EditAiNode[i].Prev[0] - EditAiNode);
        p1 = (long)(EditAiNode[i].Prev[1] - EditAiNode);
        n0 = (long)(EditAiNode[i].Next[0] - EditAiNode);
        n1 = (long)(EditAiNode[i].Next[1] - EditAiNode);
        if (p0 < 0) p0 = -1;
        if (p1 < 0) p1 = -1;
        if (n0 < 0) n0 = -1;
        if (n1 < 0) n1 = -1;

        v2.v[X] -= 192.0f;
        v2.v[Y] -= 64.0f;
        swprintf(buf, L"%ld: Prev %ld, %ld Next %ld, %ld", i, p0, p1, n0, n1);
        DumpText3D(&v2, 16, 32, 0xff00ffff, buf);*/

// draw 'brother' link

        if (EditAiNode[i].ZoneID == LONG_MAX)
//          DrawLine(&EditAiNode[i].Node[0].Pos, &EditAiNode[i].Node[1].Pos, 0xff0000, 0xff8000);
            DrawLine(&EditAiNode[i].Node[0].Pos, &EditAiNode[i].Node[1].Pos, 0xFFFF00, 0xFFFF00);
        else
//          DrawLine(&EditAiNode[i].Node[0].Pos, &EditAiNode[i].Node[1].Pos, 0xffff00, 0xffff00);
            DrawLine(&EditAiNode[i].Node[0].Pos, &EditAiNode[i].Node[1].Pos, 0x808000, 0x808000);
    }
}

////////////////////////////////////
// display 'current' ai node info //
////////////////////////////////////

void DisplayAiNodeInfo(AINODE *node)
{
    WCHAR buf[128];

// priority

    swprintf(buf, L"Priority: %S", PriorityEnum[node->Priority]);
    DumpText(450, 0, 8, 16, 0xffffff, buf);

// start node

    swprintf(buf, L"Start Node: %s", node->StartNode ? L"Yes" : L"No");
    DumpText(450, 24, 8, 16, 0x00ffff, buf);

// speeds

    swprintf(buf, L"Green Speed: %d", node->Node[0].Speed);
    DumpText(450, 48, 8, 16, 0x00ff00, buf);

    swprintf(buf, L"Red Speed: %d", node->Node[1].Speed);
    DumpText(450, 72, 8, 16, 0xff0000, buf);

    swprintf(buf, L"Racing Speed: %d", node->RacingLineSpeed);
    DumpText(450, 96, 8, 16, 0x0000ff, buf);

    swprintf(buf, L"Centre Speed: %d", node->CentreSpeed);
    DumpText(450, 120, 8, 16, 0xffff00, buf);

// total dist

    swprintf(buf, L"Track dist: %d", (long)EditAiNodeTotalDist);
    DumpText(450, 152, 8, 16, 0xff00ff, buf);
}

///////////////////////
// get edit node pos //
///////////////////////

void GetEditNodePos(VEC *campos, float xpos, float ypos, VEC *nodepos)
{
    long i;
    NEWCOLLPOLY *poly;
    VEC vec, offset, dest;
    float time, depth, ntime;

// get dest vector

    vec.v[X] = xpos - REAL_SCREEN_XHALF;
    vec.v[Y] = ypos - REAL_SCREEN_YHALF;
    vec.v[Z] = RenderSettings.GeomPers;

    RotVector(&CAM_MainCamera->WMatrix, &vec, &offset);
    NormalizeVector(&offset);
    offset.v[X] *= RenderSettings.FarClip;
    offset.v[Y] *= RenderSettings.FarClip;
    offset.v[Z] *= RenderSettings.FarClip;

    AddVector(&offset, &CAM_MainCamera->WPos, &dest);
    DrawLine(campos, &dest, 0xffff00, 0xffff00);

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


/********************************************************************************
* Pick node under cursor
********************************************************************************/
AINODE *PickupScreenNode(REAL pickerX, REAL pickerY)
{
    long    i, j, nbro;
    AINODE  *node, *nnode;
    VEC     vec;
    float   rad, z, sx, sy;


    nnode = NULL;
    z = RenderSettings.FarClip;

    node = EditAiNode;
    for (i = 0 ; i < EditAiNodeNum ; i++, node++)
    {
        for (j = 0 ; j < 2 ; j++)
        {
            RotTransVector(&ViewMatrix, &ViewTrans, &node->Node[j].Pos, &vec);

            if (vec.v[Z] < RenderSettings.NearClip || vec.v[Z] >= RenderSettings.FarClip) continue;

            sx = vec.v[X] * RenderSettings.GeomPers / vec.v[Z] + REAL_SCREEN_XHALF;
            sy = vec.v[Y] * RenderSettings.GeomPers / vec.v[Z] + REAL_SCREEN_YHALF;

            rad = 24 * RenderSettings.GeomPers / vec.v[Z];

            if (pickerX > sx - rad && pickerX < sx + rad && pickerY > sy - rad && pickerY < sy + rad)
            {
                if (vec.v[Z] < z)
                {
                    nnode = node;
                    z = vec.v[Z];
                    nbro = j;
                }
            }
        }
    }

    return nnode;
}


/********************************************************************************
* EditRacingLine
********************************************************************************/
void EditRacingLine(void)
{
    AINODE  *pNode[3];
    int     i;

    gfEditCurve = FALSE;


    if (Keys[DIK_LSHIFT] || Keys[DIK_LCONTROL])
        return;


// Clear Nodes ?
    if (Keys[DIK_GRAVE] && !LastKeys[DIK_GRAVE])
    {
        gpRacingLineNode[0] = gpRacingLineNode[1] = gpRacingLineNode[2] = NULL;
        return;
    }


// Toggle wall
    if (Keys[DIK_7] && !LastKeys[DIK_7])
    {
        if (pNode[0] = PickupScreenNode(MouseXpos, MouseYpos))
            pNode[0]->link.flags ^= AIN_LF_WALL_LEFT;
    }
    if (Keys[DIK_8] && !LastKeys[DIK_8])
    {
        if (pNode[0] = PickupScreenNode(MouseXpos, MouseYpos))
        {
//          if (pNode[0]->Next[1])
//              pNode[0]->linkInfo[1].flags ^= AIN_LF_WALL_RIGHT;
//          else
//              pNode[0]->linkInfo[0].flags ^= AIN_LF_WALL_RIGHT;
            pNode[0]->link.flags ^= AIN_LF_WALL_RIGHT;
        }
    }


// Copy node pointers
    pNode[0] = gpRacingLineNode[0];
    pNode[1] = gpRacingLineNode[1];
    pNode[2] = gpRacingLineNode[2];

// Select 1st Node ?
    if (Keys[DIK_1] && !LastKeys[DIK_1])
        pNode[0] = PickupScreenNode(MouseXpos, MouseYpos);

// Select 2nd Node ?
    if (Keys[DIK_2] && !LastKeys[DIK_2])
        pNode[1] = PickupScreenNode(MouseXpos, MouseYpos);

// Select 3rd Node ?
    if (Keys[DIK_3] && !LastKeys[DIK_3])
        pNode[2] = PickupScreenNode(MouseXpos, MouseYpos);


// Check integrity of racing curve (3 nodes must be sequential AND move forward along the racing line)
    if (pNode[0] && pNode[1] && pNode[2])
    {
        if (0 == (gcCurveNodes = CheckNodePath(pNode[0], pNode[1], pNode[2])))
        {
#ifdef OLD_AUDIO
            PlaySfx(SFX_HONK, SFX_MAX_VOL, SFX_CENTRE_PAN, SFX_SAMPLE_RATE, 0);
#else
            g_SoundEngine.Play2DSound( EFFECT_HonkGood, FALSE );
#endif // OLD_AUDIO
            return;
        }

        if (gcCurveNodes > MAX_CURVE_NODES)
        {
#ifdef OLD_AUDIO
            PlaySfx(SFX_HONK, SFX_MAX_VOL, SFX_CENTRE_PAN, SFX_SAMPLE_RATE, 0);
#else
            g_SoundEngine.Play2DSound( EFFECT_HonkGood, FALSE );
#endif // OLD_AUDIO
            return;
        }
    }


// Set node pointers
    for (i = 0; i < 3; i++)
    {
        if (gpRacingLineNode[i] != pNode[i])
        {
            if (gpRacingLineNode[i] = pNode[i])
            {
                gRacingLinePos[i][0] = gRacingLinePos[i][1] = pNode[i]->RacingLine;
                gOvertakingLinePos[i][0] = gOvertakingLinePos[i][1] = pNode[i]->OvertakingLine;
            }
        }
    }


// Special edit keys
    if (gpRacingLineNode[0] && gpRacingLineNode[1] && gpRacingLineNode[2] && !CurrentEditAiNode)
    {
    // Edit mode on
        gfEditCurve = TRUE;

    // Adjust racing line
        if (Keys[DIK_DIVIDE])
            gRacingLinePos[1][0] -= 0.004f;
        if (Keys[DIK_MULTIPLY])
            gRacingLinePos[1][0] += 0.004f;
        Limit(gRacingLinePos[1][0], ZERO, ONE);

        if (Keys[DIK_NUMPAD8])
            gOvertakingLinePos[1][0] -= 0.004f;
        if (Keys[DIK_NUMPAD9])
            gOvertakingLinePos[1][0] += 0.004f;
        Limit(gOvertakingLinePos[1][0], ZERO, ONE);


    // Create curved racing & overtaking lines ?
        gfStoreRacingLine = (Keys[DIK_4] && !LastKeys[DIK_4]);
        gfStoreOvertakingLine = (Keys[DIK_5] && !LastKeys[DIK_5]);


    // Clear keys that I don't want the other editors to use
        Keys[DIK_DIVIDE] =
        Keys[DIK_MULTIPLY] =
        Keys[DIK_NUMPAD8] =
        Keys[DIK_NUMPAD9] = 0;
    }
}


/********************************************************************************
* CheckNodePath()
* Check if nodes are sequential
********************************************************************************/
int CheckNodePath(AINODE *pNodeS, AINODE *pNodeM, AINODE *pNodeE)
{
    AINODE  *pNode[4], *pNodeCur;
    int     cCmp;
    int     cNodesPassed;

    pNode[0] = pNodeS;
    pNode[1] = pNodeM;
    pNode[2] = pNodeE;
    pNode[3] = pNodeS;
    pNodeCur = pNodeS;
    cCmp = 0;
    cNodesPassed = 1;
    while (pNodeCur)
    {
    // Double link found ?
        if (pNodeCur->Next[1])
        {
#ifdef OLD_AUDIO
            PlaySfx(SFX_COUNTDOWN, SFX_MAX_VOL, SFX_CENTRE_PAN, SFX_SAMPLE_RATE, 0);
#else
            g_SoundEngine.Play2DSound( EFFECT_Countdown, FALSE );
#endif // OLD_AUDIO
            return 0;
        }

    // Found next-next node ?
        if (pNodeCur->Next[0] == pNode[cCmp+2])
        {
#ifdef OLD_AUDIO
            PlaySfx(SFX_SHOCKWAVE, SFX_MAX_VOL, SFX_CENTRE_PAN, SFX_SAMPLE_RATE, 0);
#else
            g_SoundEngine.Play2DSound( EFFECT_Shock, FALSE );
#endif // OLD_AUDIO
            return 0;
        }

    // Found next node ?
        if (pNodeCur->Next[0] == pNode[cCmp+1])
        {
            cCmp++;
            if (cCmp == 2)
                return cNodesPassed+1;
        }

    // Move to next node
        cNodesPassed++;
        pNodeCur = pNodeCur->Next[0];
        if (pNodeCur == pNodeS)
            break;

    // Check if we have passed a large number of nodes
        if (cNodesPassed > MAX_CURVE_NODES)
        {
#ifdef OLD_AUDIO
            PlaySfx(SFX_PUTTYBOMB_BANG, SFX_MAX_VOL, SFX_CENTRE_PAN, SFX_SAMPLE_RATE, 0);
#else
            g_SoundEngine.Play2DSound( EFFECT_PuttyBombBang, FALSE );
#endif // OLD_AUDIO
            return 0;
        }
    }

    return 0;
}


/********************************************************************************
* CreateRacingLine
********************************************************************************/

//////////////////////////////
// Save node values
//////////////////////////////
void SaveRacingLine(void)
{
    t_RacingLineUndo    *pUndo;
    AINODE              *pNode;
    int                 i;

    pUndo = gpRacingLineUndo;
    pNode = gpRacingLineNode[0];

    for (i = 0; i < gcCurveNodes; i++, pNode = pNode->Next[0], pUndo++)
    {
    // Save vars
        pUndo->racingLine = pNode->RacingLine;
        pUndo->overtakingLine = pNode->OvertakingLine;
    }

    for (i = 0; i < 3; i++)
    {
        gpRacingLineNode[i]->RacingLine = gRacingLinePos[i][0];
        gpRacingLineNode[i]->OvertakingLine = gOvertakingLinePos[i][0];
    }
}

//////////////////////////////
// Restore node values
//////////////////////////////
void RestoreRacingLine(void)
{
    t_RacingLineUndo    *pUndo;
    AINODE              *pNode;
    int                 i;

    pUndo = gpRacingLineUndo;
    pNode = gpRacingLineNode[0];

    for (i = 0; i < 3; i++)
    {
        gRacingLinePos[i][0] = gpRacingLineNode[i]->RacingLine;
//      gpRacingLineNode[i]->RacingLine = gRacingLinePos[i][1];
        gOvertakingLinePos[i][0] = gpRacingLineNode[i]->OvertakingLine;
//      gpRacingLineNode[i]->OvertakingLine = gOvertakingLinePos[i][1];
    }

    for (i = 0; i < gcCurveNodes; i++, pNode = pNode->Next[0], pUndo++)
    {
    // Restore vars
        if (!gfStoreRacingLine)
            pNode->RacingLine = pUndo->racingLine;

        if (!gfStoreOvertakingLine)
            pNode->OvertakingLine = pUndo->overtakingLine;
    }

    gfStoreRacingLine = FALSE;
    gfStoreOvertakingLine = FALSE;
}

//////////////////////////////
// Create lines
//////////////////////////////
void CreateRacingLine(void)
{
    VEC                 racingPos[3];
    VEC                 overtakingPos[3];
    int                 cP, cPMax;
    int                 i, j, cLine;
    int                 cPoints;
    REAL                t, Dt;
    VEC                 racingPoints[(MAX_CURVE_NODES*MAX_CURVE_NODES_MULTIPLY)+1];
    VEC                 overtakePoints[(MAX_CURVE_NODES*MAX_CURVE_NODES_MULTIPLY)+1];
    AINODE              *pNode;
    VEC                 *pPos, *pPosP;
    VEC                 delta[2];
    VEC                 fVec[MAX_CURVE_NODES+1], uVec, rVec;
    REAL                dotP[2];

// Create knots
    for (i = 0; i < 3; i++)
    {
        for (j = 0; j < 3; j++)
        {
            racingPos[i].v[j] = gpRacingLineNode[i]->Node[0].Pos.v[j] +
                ((gpRacingLineNode[i]->Node[1].Pos.v[j] - gpRacingLineNode[i]->Node[0].Pos.v[j]) * (ONE - gpRacingLineNode[i]->RacingLine));
            overtakingPos[i].v[j] = gpRacingLineNode[i]->Node[0].Pos.v[j] +
                ((gpRacingLineNode[i]->Node[1].Pos.v[j] - gpRacingLineNode[i]->Node[0].Pos.v[j]) * (ONE - gpRacingLineNode[i]->OvertakingLine));
        }
    }

// Create quadratic
    cPMax = gcCurveNodes * MAX_CURVE_NODES_MULTIPLY;
    Dt = ONE / Real(cPMax);
    for (cP = 0, t = 0; cP < cPMax+1; cP++, t += Dt)
    {
        Interpolate3D(&racingPos[0], &racingPos[1], &racingPos[2], t, &racingPoints[cP]);
        Interpolate3D(&overtakingPos[0], &overtakingPos[1], &overtakingPos[2], t, &overtakePoints[cP]);
    }

// Create forward vector
    pNode = gpRacingLineNode[0]->Next[0];
    for (i = 1; i < gcCurveNodes-1; i++, pNode = pNode->Next[0])
    {
        VecMinusVec(&pNode->Node[0].Pos, &pNode->Node[1].Pos, &rVec);
        VecMinusVec(&pNode->Next[0]->Node[1].Pos, &pNode->Node[1].Pos, &fVec[i]);
        VecCrossVec(&fVec[i], &rVec, &uVec);
        VecCrossVec(&rVec, &uVec, &fVec[i]);
        NormalizeVector(&fVec[i]);
    }

// Create node racing & overtaking lines
    for (cLine = 0; cLine < 2; cLine++)
    {

    // Setup loop vars
        pNode = gpRacingLineNode[0]->Next[0];
        if (cLine == 0)
            pPos = racingPoints;
        else
            pPos = overtakePoints;

        cPoints = cPMax+1;

        for (i = 1; i < gcCurveNodes-1; i++, pNode = pNode->Next[0])
        {
            pPosP = pPos;
            while (cPoints)
            {
                VecMinusVec(pPos, &pNode->Node[0].Pos, &delta[0]);
                dotP[1] = VecDotVec(&delta[0], &fVec[i]);
                if (dotP[1] >= Real(0))
                    break;

                dotP[0] = dotP[1];
                pPosP = pPos;
                pPos++;
                cPoints--;
            }

            cPoints++;
            if (!cPoints)
                break;

            t = CalcNodeIntersectionTime(pNode, pPosP, pPos);
            if (t >= ZERO)
            {
                if (cLine == 0)
                    pNode->RacingLine = t;
                else
                    pNode->OvertakingLine = t;
            }

            pPos--;
        }
    }
}


/********************************************************************************
* Intersect
********************************************************************************/
REAL CalcNodeIntersectionTime(AINODE* pNode, VEC *pS, VEC *pE)
{
    VEC     fVec, uVec, rVec;
    VEC     delta[2];
    VEC     intersect;
    REAL    dotP[2], dist[2];
    int     sign[2];
    REAL    t, l;

    if (!pNode->Next[0])
        return -ONE;

// Create nodes forward vector
    VecMinusVec(&pNode->Node[0].Pos, &pNode->Node[1].Pos, &rVec);
    VecMinusVec(&pNode->Next[0]->Node[1].Pos, &pNode->Node[1].Pos, &fVec);
    VecCrossVec(&fVec, &rVec, &uVec);
    VecCrossVec(&rVec, &uVec, &fVec);
    NormalizeVector(&fVec);

// Calc dist to node's line to start & end points
    VecMinusVec(pS, &pNode->Node[0].Pos, &delta[0]);
    dotP[0] = VecDotVec(&delta[0], &fVec);
    VecMinusVec(pE, &pNode->Node[0].Pos, &delta[1]);
    dotP[1] = VecDotVec(&delta[1], &fVec);

// Does the line intersect the node ?
    sign[0] = Sign(dotP[0]);
    sign[1] = Sign(dotP[1]);
    if (sign[0] == sign[1])
        return -ONE;

    dotP[0] = abs(dotP[0]);
    dotP[1] = abs(dotP[1]);
    t = dotP[0] / (dotP[0] + dotP[1]);
    intersect.v[X] = pS->v[X] + ((pE->v[X] - pS->v[X]) * t);
    intersect.v[Y] = pS->v[Y] + ((pE->v[Y] - pS->v[Y]) * t);
    intersect.v[Z] = pS->v[Z] + ((pE->v[Z] - pS->v[Z]) * t);

    VecMinusVec(&pNode->Node[0].Pos, &intersect, &delta[0]);
    VecMinusVec(&pNode->Node[1].Pos, &intersect, &delta[1]);

    dist[0] = VecDotVec(&delta[0], &rVec);
    if (dist[0] < 0)
        return -ONE;
    dist[1] = -VecDotVec(&delta[1], &rVec);
    if (dist[1] < 0)
        return -ONE;

    l = ONE / VecLen(&rVec);
    dist[0] *= l;
    dist[1] *= l;

    t = dist[1] / (dist[0] + dist[1]);
    Limit(t, ZERO, ONE);

    return t;
}


/********************************************************************************
* EditRacingLine
********************************************************************************/
void DrawEditRacingLine(void)
{
    AINODE          *pNode;
//  VEC             pos[3];
    VEC             posR[2], posO[2];
    int             i,j;
    int             cNodes;
    FACING_POLY     poly;
    long            rgb[3] = {0x8000FF00, 0x80FFFF00, 0x80FF0000};
    long            racingRGB, overtakingRGB;

// Draw nodes
    poly.Xsize =
    poly.Ysize = (float)abs(sin((float)TIME2MS(CurrentTimer()) / 150.0f) * 24.0f) + 24.0f;
    poly.U = 0;
    poly.V = 0;
    poly.Usize = 1;
    poly.Vsize = 1;
    poly.Tpage = TPAGE_ENVSTILL;

    cNodes = 0;
    for (i = 0; i < 3; i++)
    {
        if (gpRacingLineNode[i])
        {
            cNodes++;
            poly.RGB = rgb[i];
            for (j = 0; j < 2; j++)
            {
                DrawFacingPoly(&gpRacingLineNode[i]->Node[j].Pos, &poly, 0, -16);
            }
        }
    }

// Draw lines
    if (cNodes == 3)
    {
        SaveRacingLine();

        gpRacingLineNode[1]->RacingLine = gRacingLinePos[1][0];
        gpRacingLineNode[1]->OvertakingLine = gOvertakingLinePos[1][0];
        CreateRacingLine();

        racingRGB = 0xf0f0f0;
        overtakingRGB = 0xf000f0;

        pNode = gpRacingLineNode[0];
        for (j = 0; j < 3; j++)
        {
            posR[0].v[j] = pNode->Node[0].Pos.v[j] +
                            ((pNode->Node[1].Pos.v[j] - pNode->Node[0].Pos.v[j]) * (ONE - pNode->RacingLine));
            posO[0].v[j] = pNode->Node[0].Pos.v[j] +
                            ((pNode->Node[1].Pos.v[j] - pNode->Node[0].Pos.v[j]) * (ONE - pNode->OvertakingLine));
        }
        pNode = pNode->Next[0];

        for (i = 1; i < gcCurveNodes; i++, pNode = pNode->Next[0])
        {
            for (j = 0; j < 3; j++)
            {
                posR[i&1].v[j] = pNode->Node[0].Pos.v[j] +
                                ((pNode->Node[1].Pos.v[j] - pNode->Node[0].Pos.v[j]) * (ONE - pNode->RacingLine));
                posO[i&1].v[j] = pNode->Node[0].Pos.v[j] +
                                ((pNode->Node[1].Pos.v[j] - pNode->Node[0].Pos.v[j]) * (ONE - pNode->OvertakingLine));
            }

            DrawLine(&posR[(i+1)&1], &posR[i&1], racingRGB, (racingRGB>>1)&0x7f7f7f);
            DrawLine(&posO[(i+1)&1], &posO[i&1], overtakingRGB, (overtakingRGB>>1)&0x7f7f7f);
        }

        RestoreRacingLine();
    }
}


/********************************************************************************
* EAI_CreateGhostPath
********************************************************************************/
void EAI_CreateGhostPath(PLAYER *pPlayer)
{
    unsigned long   a,b,c;
    int             j;

    if (!gbCreateGhostPath || !pPlayer)
        return;

    a = TotalRaceStartTime;
    b = pPlayer->car.CurrentLapStartTime;
    c = TotalRacePhysicsTime;

    gbCreateGhostPath = FALSE;
    gbValidGhostPath = TRUE;

    TotalRaceStartTime = 0;
    pPlayer->car.CurrentLapStartTime = 0;
    for (j = 0, TotalRacePhysicsTime = 0; (int)TotalRacePhysicsTime < GHO_BestGhostInfo->Time[GHOST_LAP_TIME]; j++, TotalRacePhysicsTime += GHOST_PATH_SAMPLE_INTERVAL)
    {
        if (j >= GHOST_PATH_MAX_SAMPLES)
            break;

        InterpGhostData(&pPlayer->car);
        CopyVec(&pPlayer->car.Body->Centre.Pos, &gGhostPath[j]);
    }

    gGhostPathSamples = j;

    TotalRaceStartTime = a;
    pPlayer->car.CurrentLapStartTime = b;
    TotalRacePhysicsTime = c;
}

/********************************************************************************
* EAI_RenderGhostPath
********************************************************************************/
void EAI_RenderGhostPath(void)
{
    VEC     *pPos, *pPosP;
    long    rgb1;
    long    rgb2;
    int     i;

    if (!gbValidGhostPath)
        return;

    rgb1 = 0xff0000;
    rgb2 = (rgb1>>1)&0x7f7f7f;

    pPosP = gGhostPath + gGhostPathSamples-1;
    pPos = gGhostPath;
    for (i = 0, pPos = gGhostPath; i < gGhostPathSamples; i++, pPosP = pPos, pPos++)
    {
        DrawLine(pPosP, pPos, rgb1, rgb2);
    }
}


/********************************************************************************
* EAI_RenderGhostPath
********************************************************************************/
bool CreateRacingLineFromGhostData(void)
{
    VEC     *pPos, *pPosP;
    AINODE  *pNode;
    REAL    t;
    int     cPath, cNode;

#ifndef XBOX_NOT_YET_IMPLEMENTED
    if (IDNO == Box("Create Racing Line From Ghost Car", "Are you sure?", MB_YESNO))
        return FALSE;
#endif
    if (!gbValidGhostPath)
        return FALSE;

    pNode = EditAiNode;
    for (cNode = 0; cNode < EditAiNodeNum; cNode++, pNode++)
    {
        pPosP = gGhostPath + gGhostPathSamples-1;
        pPos = gGhostPath;
        for (cPath = 0, pPos = gGhostPath; cPath < gGhostPathSamples; cPath++, pPosP = pPos, pPos++)
        {
            t = CalcNodeIntersectionTime(pNode, pPosP, pPos);
            if (t >= ZERO)
            {
                pNode->RacingLine = t;
            }
        }
    }

    return TRUE;
}


/********************************************************************************
* TopSlotAiNodeLinks
********************************************************************************/
void TopSlotAiNodeLinks(AINODE *pNode)
{
    int j,k;

    for (j = 0 ; j < MAX_AINODE_LINKS - 1; j++)
    {
        if (!pNode->Prev[j])
        {
            for (k = j + 1 ; k < MAX_AINODE_LINKS ; k++)
            {
                if (pNode->Prev[k])
                {
                    pNode->Prev[j] = pNode->Prev[k];
                    pNode->Prev[k] = NULL;
                    break;
                }
            }
        }
        if (!pNode->Next[j])
        {
            for (k = j + 1 ; k < MAX_AINODE_LINKS ; k++)
            {
                if (pNode->Next[k])
                {
                    pNode->Next[j] = pNode->Next[k];
                    pNode->Next[k] = NULL;
                    break;
                }
            }
        }
    }
}

/********************************************************************************
* Count Link Nodes
********************************************************************************/
short CountAiLinkNodes(void)
{
    AINODE  *pNode;
    short   cLinks;
    short   i;

    cLinks = 0;
    for (i = 0, pNode = EditAiNode; i < EditAiNodeNum; i++, pNode++)
    {
        if (pNode->Next[1])
            cLinks++;
    }

    return cLinks;
}


/********************************************************************************
* IsValidAiNode
********************************************************************************/
bool IsValidAiNode(AINODE* pNode)
{
#if 0
    AINODE* pNodeN;
    VEC     delta;
    REAL    dist[4];
    int     i;

    //for (i = 0; i < MAX_AINODE_LINKS; i++)
        i = 0;
    {
        if (pNodeN = pNode->Next[i])
        {
            dist[0] = VecDotPlane(&pNode->Node[0].Pos, &pNode->Node[1].plane);
            dist[1] = VecDotPlane(&pNode->Node[1].Pos, &pNode->Node[0].plane);
            VecMinusVec(&Vec, &pNode->Node[0].Pos, &delta);
            dist[2] = VecDotPlane(&pNode->Node[0].Pos, &pNode->Node[1].plane);
            dist[3] = VecDotPlane(&pNode->Node[1].Pos, &pNode->Node[0].plane);
        }
    }
#endif

    return TRUE;
}
