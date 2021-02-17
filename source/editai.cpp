
#include "revolt.h"
#include "editai.h"
#include "model.h"
#include "main.h"
#include "camera.h"
#include "input.h"
#include "level.h"
#include "draw.h"
#include "text.h"
#include "grid.h"
#include "aizone.h"
#include "timing.h"

// globals

static long CurrentEditAiNodeBro;
static long EditAiStartNode;
static REAL EditAiNodeTotalDist;

AINODE *CurrentEditAiNode = NULL;
AINODE *LastEditAiNode = NULL;
long EditAiNodeNum;
AINODE *EditAiNode;
MODEL EditAiNodeModel[2];

////////////////////////
// init edit ai nodes //
////////////////////////

void InitEditAiNodes(void)
{
	EditAiNode = (AINODE*)malloc(sizeof(AINODE) * MAX_AINODES);
	if (!EditAiNode)
	{
		Box(NULL, "Can't alloc memory for edit AI nodes!", MB_OK);
		QuitGame = TRUE;
	}
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

	for (i = 0 ; i < EditAiNodeNum ; i++)
	{

// load one file ainode

		fread(&fan, sizeof(fan), 1, fp);

		VecMulScalar(&fan.Node[0].Pos, EditScale);
		VecMulScalar(&fan.Node[1].Pos, EditScale);

// setup edit ainode

		EditAiNode[i].Node[0] = fan.Node[0];
		EditAiNode[i].Node[1] = fan.Node[1];

		EditAiNode[i].Priority = fan.Priority;
		EditAiNode[i].StartNode = fan.StartNode;
		EditAiNode[i].RacingLine = fan.RacingLine;
		EditAiNode[i].RacingLineSpeed = fan.RacingLineSpeed;
		EditAiNode[i].CentreSpeed = fan.CentreSpeed;
		EditAiNode[i].FinishDist = fan.FinishDist;

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

//	if (flag && node == &EditAiNode[EditAiStartNode])
//		return;

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
	long		i, j;
	FILE		*fp;
	FILE_AINODE fan;
	char		bak[256];

// backup old file

	memcpy(bak, file, strlen(file) - 3);
	wsprintf(bak + strlen(file) - 3, "fa-");
	remove(bak);
	rename(file, bak);

// open node file

	fp = fopen(file, "wb");
	if (!fp) return;

// write num

	fwrite(&EditAiNodeNum, sizeof(EditAiNodeNum), 1, fp);

// write out each ainode

	for (i = 0 ; i < EditAiNodeNum ; i++)
	{

// set file ainode

		fan.Node[0] = EditAiNode[i].Node[0];
		fan.Node[1] = EditAiNode[i].Node[1];

		fan.Priority = EditAiNode[i].Priority;
		fan.StartNode = EditAiNode[i].StartNode;
		fan.RacingLine = EditAiNode[i].RacingLine;
		fan.RacingLineSpeed = EditAiNode[i].RacingLineSpeed;
		fan.CentreSpeed = EditAiNode[i].CentreSpeed;
		fan.FinishDist = EditAiNode[i].FinishDist;

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
	Box("Saved AI node File:", file, MB_OK);
}

/////////////////////////////
// load edit ainode models //
/////////////////////////////

void LoadEditAiNodeModels(void)
{
	LoadModel("edit\\ainode2.m", &EditAiNodeModel[0], -1, 1, LOADMODEL_FORCE_TPAGE, 100);
	LoadModel("edit\\ainode1.m", &EditAiNodeModel[1], -1, 1, LOADMODEL_FORCE_TPAGE, 100);
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
	long i, j, nbro;
	AINODE *node, *nnode;
	VEC vec;
	MAT mat, mat2;
	ONE_AINODE tempnode;
	float rad, z, sx, sy;

// quit if not in edit mode

	if (CAM_MainCamera->Type != CAM_EDIT)
	{
		CurrentEditAiNode = NULL;
		return;
	}

// rotate camera?

	if (MouseRight)
	{
		RotMatrixZYX(&mat, (float)-Mouse.lY / 3072, -(float)Mouse.lX / 3072, 0);
		MulMatrix(&CAM_MainCamera->WMatrix, &mat, &mat2);
		CopyMatrix(&mat2, &CAM_MainCamera->WMatrix);

		CAM_MainCamera->WMatrix.m[RY] = 0;
		NormalizeVector(&CAM_MainCamera->WMatrix.mv[X]);
		CrossProduct(&CAM_MainCamera->WMatrix.mv[Z], &CAM_MainCamera->WMatrix.mv[X], &CAM_MainCamera->WMatrix.mv[Y]);
		NormalizeVector(&CAM_MainCamera->WMatrix.mv[Y]);
		CrossProduct(&CAM_MainCamera->WMatrix.mv[X], &CAM_MainCamera->WMatrix.mv[Y], &CAM_MainCamera->WMatrix.mv[Z]);
	}

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

	if (Keys[DIK_R] && !LastKeys[DIK_R] && Keys[DIK_LSHIFT])
	{
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

// save ai nodes?

	if (Keys[DIK_LCONTROL] && Keys[DIK_F4] && !LastKeys[DIK_F4])
	{
		CalcEditAiNodeDistances();
		SaveEditAiNodes(GetLevelFilename("fan", FILENAME_MAKE_BODY | FILENAME_GAME_SETTINGS));
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

// change priority

	if (Keys[DIK_NUMPADENTER] && !LastKeys[DIK_NUMPADENTER]) CurrentEditAiNode->Priority ^= TRUE;

// change start node

	if (Keys[DIK_NUMPAD0] && !LastKeys[DIK_NUMPAD0])
	{
		for (i = 0 ; i < EditAiNodeNum ; i++)
			EditAiNode[i].StartNode = FALSE;

		CurrentEditAiNode->StartNode = TRUE;
	}

// change racing line?

	if (Keys[DIK_NUMPADSLASH] && CurrentEditAiNode->RacingLine > 0.0f) CurrentEditAiNode->RacingLine -= 0.002f;
	if (Keys[DIK_NUMPADSTAR] && CurrentEditAiNode->RacingLine < 1.0f) CurrentEditAiNode->RacingLine += 0.002f;

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
	char buf[128];
	short flag;

// loop thru all nodes

	for (i = 0 ; i < EditAiNodeNum ; i++)
	{
		for (j = 0 ; j < 2 ; j++)
		{
			node = &EditAiNode[i].Node[j];

// draw it

			flag = MODEL_PLAIN;
			if (i == EditAiStartNode)
			{
				flag |= MODEL_SCALE;
				ModelScale = (float)sin((float)TIME2MS(CurrentTimer()) / 300.0f) * 0.5f + 1.0f;
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

				if (!j)
				{
					DrawLine(&v1, &v2, 0x000000, 0xff0000);
				}
				else
				{
					DrawLine(&v1, &v2, 0x000000, 0x00ff00);
				}
			}

// draw 'current' axis?

			if (CurrentEditAiNode == &EditAiNode[i] && CurrentEditAiNodeBro == j)
			{
				DrawAxis(&IdentityMatrix, &node->Pos);
			}
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

			DrawLine(&v1, &v2, 0xffffff, 0xffffff);
		}

// dump finish dist

		SET_TPAGE(TPAGE_FONT);
		wsprintf(buf, "%ld", (long)EditAiNode[i].FinishDist);
		RotTransVector(&ViewMatrix, &ViewTrans, &v1, &v2);
		v2.v[X] -= 48.0f;
		v2.v[Y] -= 32.0f;
		DumpText3D(&v2, 16, 32, 0xffffffff, buf);

// draw 'brother' link

		DrawLine(&EditAiNode[i].Node[0].Pos, &EditAiNode[i].Node[1].Pos, 0xffff00, 0xffff00);
	}
}

////////////////////////////////////
// display 'current' ai node info //
////////////////////////////////////

void DisplayAiNodeInfo(AINODE *node)
{
	char buf[128];

// priority

	wsprintf(buf, "Priority: %s", node->Priority ? "Yes" : "No");
	DumpText(450, 0, 8, 16, 0xffffff, buf);

// start node

	wsprintf(buf, "Start Node: %s", node->StartNode ? "Yes" : "No");
	DumpText(450, 24, 8, 16, 0x00ffff, buf);

// speeds

	wsprintf(buf, "Green Speed: %d", node->Node[0].Speed);
	DumpText(450, 48, 8, 16, 0x00ff00, buf);

	wsprintf(buf, "Red Speed: %d", node->Node[1].Speed);
	DumpText(450, 72, 8, 16, 0xff0000, buf);

	wsprintf(buf, "Racing Speed: %d", node->RacingLineSpeed);
	DumpText(450, 96, 8, 16, 0x0000ff, buf);

	wsprintf(buf, "Centre Speed: %d", node->CentreSpeed);
	DumpText(450, 120, 8, 16, 0xffff00, buf);

// total dist

	wsprintf(buf, "Track dist: %d", (long)EditAiNodeTotalDist);
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
