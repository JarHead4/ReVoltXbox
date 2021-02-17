
#include "revolt.h"
#include "editcam.h"
#include "main.h"
#include "model.h"
#include "geom.h"
#include "text.h"
#include "camera.h"
#include "input.h"
#include "level.h"

// globals

EDIT_CAM_NODE *CurrentEditCamNode = NULL;

static MODEL EditCamNodeModel[CAMNODE_NTYPES];
static long EditCamNodeNum;
static EDIT_CAM_NODE *EditCamNode;
static long EditCamNodeAxis, EditCamNodeAxisType;
static EDIT_CAM_NODE *LastEditCamNode = NULL;

static long MaxEditCamNodeID = -1;

static char *EditCamNodeModelNames[CAMNODE_NTYPES] = {
	"edit\\camnode.m",
	"models\\football.m",
};


// misc text

static char *EditCamNodeAxisNames[] = {
	"X Y",
	"X Z",
	"Z Y",
	"X",
	"Y",
	"Z",
};

static char *EditCamNodeAxisTypeNames[] = {
	"Camera",
	"World",
};

char *CamNodeTypeText[] = {
	"Monorail",
	"Static",
};

/////////////////////////
// init edit cam nodes //
/////////////////////////

void InitEditCamNodes(void)
{
	EditCamNode = (EDIT_CAM_NODE*)malloc(sizeof(EDIT_CAM_NODE) * MAX_EDIT_CAM_NODES);
	MaxEditCamNodeID = -1;
	if (!EditCamNode)
	{
		Box(NULL, "Can't alloc memory for edit camera nodes!", MB_OK);
		QuitGame = TRUE;
	}
}

/////////////////////////
// kill edit cam nodes //
/////////////////////////

void KillEditCamNodes(void)
{
	free(EditCamNode);
}

///////////////////////////////
// load edit cam node models //
///////////////////////////////

void LoadEditCamNodeModels(void)
{
	int iModel;

	for (iModel = 0; iModel < CAMNODE_NTYPES; iModel++) {
		LoadModel(EditCamNodeModelNames[iModel], &EditCamNodeModel[iModel], -1, 1, LOADMODEL_FORCE_TPAGE, 100);
	}
}

///////////////////////////////
// free edit cam node models //
///////////////////////////////

void FreeEditCamNodeModels(void)
{
	int iModel;

	for (iModel = 0; iModel < CAMNODE_NTYPES; iModel++) {
		FreeModel(&EditCamNodeModel[iModel], 1);
	}
}

/////////////////////////
// load edit cam nodes //
/////////////////////////

void LoadEditCamNodes(char *file)
{
	long i;
	FILE *fp;
	FILE_CAM_NODE fnode;

// open cam node file

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

// loop thru all cam nodes

	fread(&EditCamNodeNum, sizeof(EditCamNodeNum), 1, fp);

	for (i = 0 ; i < EditCamNodeNum ; i++)
	{

// load one file cam node

		fread(&fnode, sizeof(fnode), 1, fp);

		fnode.x = (long)((float)fnode.x * EditScale);
		fnode.y = (long)((float)fnode.y * EditScale);
		fnode.z = (long)((float)fnode.z * EditScale);

// setup edit cam node

		EditCamNode[i].Type = fnode.Type;
		EditCamNode[i].ZoomFactor = fnode.ZoomFactor;

		EditCamNode[i].Pos.v[X] = (float)fnode.x / 65536;
		EditCamNode[i].Pos.v[Y] = (float)fnode.y / 65536;
		EditCamNode[i].Pos.v[Z] = (float)fnode.z / 65536;

		if (fnode.Link != -1)
		{
			EditCamNode[i].Link = &EditCamNode[fnode.Link];
		}
		else 
		{
			EditCamNode[i].Link = NULL;
		}

		if (fnode.ID == -1) {
			EditCamNode[i].ID = ++MaxEditCamNodeID;
		} else {
			EditCamNode[i].ID = fnode.ID;
			MaxEditCamNodeID = Max(MaxEditCamNodeID, fnode.ID);
		}
	}

// close node file

	fclose(fp);
}

/////////////////////////
// save edit cam nodes //
/////////////////////////

void SaveEditCamNodes(char *file)
{
	long i;
	FILE *fp;
	FILE_CAM_NODE fnode;
	char bak[256];

// backup old file

	memcpy(bak, file, strlen(file) - 3);
	wsprintf(bak + strlen(file) - 3, "ca-");
	remove(bak);
	rename(file, bak);

// open node file

	fp = fopen(file, "wb");
	if (!fp) return;

// write num

	fwrite(&EditCamNodeNum, sizeof(EditCamNodeNum), 1, fp);

// write out each cam node

	for (i = 0 ; i < EditCamNodeNum ; i++)
	{

// set file cam node

		fnode.Type = EditCamNode[i].Type;
		fnode.ZoomFactor = EditCamNode[i].ZoomFactor;

		fnode.x = (long)(EditCamNode[i].Pos.v[X] * 65536);
		fnode.y = (long)(EditCamNode[i].Pos.v[Y] * 65536);
		fnode.z = (long)(EditCamNode[i].Pos.v[Z] * 65536);

		if (EditCamNode[i].Link)
		{
			fnode.Link = (long)(EditCamNode[i].Link - EditCamNode);
		}
		else
		{
			fnode.Link = -1;
		}

		fnode.ID = EditCamNode[i].ID;

// write it

		fwrite(&fnode, sizeof(fnode), 1, fp);
	}

// close file

	Box("Saved camera node File:", file, MB_OK);
	fclose(fp);
}

////////////////////
// draw cam nodes //
////////////////////
extern VEC CAM_NodeCamPos;
void DrawEditCamNodes(void)
{
	long i;
	VEC pos;
	char buf[128];
	EDIT_CAM_NODE *node;

// loop thru all nodes

	node = EditCamNode;
	for (i = 0 ; i < EditCamNodeNum ; i++, node++)
	{

// draw it

		if (LastEditCamNode != &EditCamNode[i] || (FrameCount & 4))
			DrawModel(&EditCamNodeModel[node->Type], &IdentityMatrix, &node->Pos, MODEL_PLAIN);

// draw links?

		if (node->Link)
		{
			DrawLine(&node->Pos, &node->Link->Pos, 0xffff00, 0xffff00);
		}

// draw 'current' axis?

		if (CurrentEditCamNode == node)
		{
			if (EditCamNodeAxisType)
				DrawAxis(&IdentityMatrix, &node->Pos);
			else
				DrawAxis(&CAM_MainCamera->WMatrix, &node->Pos);
		}

// draw ID

		wsprintf(buf, "%d", EditCamNode[i].ID);
		RotTransVector(&ViewMatrix, &ViewTrans, &EditCamNode[i].Pos, &pos);
		pos.v[X] -= strlen(buf) * 4.0f;
		pos.v[Y] -= 48.0f;

		if (pos.v[Z] > RenderSettings.NearClip)
			DumpText3D(&pos, 8, 16, 0xffff00, buf);

	}

// draw nearest node pos
//	DrawModel(&EditCamNodeModel[1], &IdentityMatrix, &CAM_NodeCamPos, MODEL_PLAIN);

}

/////////////////////////////////////
// display 'current' cam node info //
/////////////////////////////////////

void DisplayCamNodeInfo(EDIT_CAM_NODE *node)
{
	char buf[128];

// type

	wsprintf(buf, "Type %s", CamNodeTypeText[node->Type]);
	DumpText(450, 0, 8, 16, 0xff0000, buf);

// flag

	wsprintf(buf, "Zoom %d", node->ZoomFactor);
	DumpText(450, 24, 8, 16, 0x00ff00, buf);

// axis

	wsprintf(buf, "Axis %s - %s", EditCamNodeAxisNames[EditCamNodeAxis], EditCamNodeAxisTypeNames[EditCamNodeAxisType]);
	DumpText(450, 48, 8, 16, 0x0000ff, buf);

// pos

	wsprintf(buf, "Pos  %d %d %d", (long)node->Pos.v[X],	(long)node->Pos.v[Y], (long)node->Pos.v[Z]);
	DumpText(450, 72, 8, 16, 0xffffff, buf);

// ID

	wsprintf(buf, "ID   %d", node->ID);
	DumpText(450, 96, 8, 16, 0x00ff00, buf);

}

//////////////////////////
// edit current camnode //
//////////////////////////

void EditCamNodes(void)
{
	long i;
	VEC vec, vec2;
	float z, sx, sy, rad;
	MAT mat, mat2;
	EDIT_CAM_NODE *nnode, *node;
	FILE *fp;

// quit if not in edit mode

	if (CAM_MainCamera->Type != CAM_EDIT)
	{
		CurrentEditCamNode = NULL;
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

// save nodes?

	if (Keys[DIK_LCONTROL] && Keys[DIK_F4] && !LastKeys[DIK_F4])
	{
		SaveEditCamNodes(GetLevelFilename("cam", FILENAME_MAKE_BODY | FILENAME_GAME_SETTINGS));
		if ((fp = fopen(GetLevelFilename("cam", FILENAME_MAKE_BODY), "rb")) != NULL)
		{
			CAM_NCameraNodes = LoadCameraNodes(fp);
			fclose(fp);
		}
	}

// get a current node?

	if ((Keys[DIK_RETURN] && !LastKeys[DIK_RETURN]) || (Keys[DIK_BACKSPACE] && !LastKeys[DIK_BACKSPACE]))
	{
		nnode = NULL;
		z = RenderSettings.FarClip;

		node = EditCamNode;
		for (i = 0 ; i < EditCamNodeNum ; i++, node++)
		{
			RotTransVector(&ViewMatrix, &ViewTrans, &node->Pos, &vec);

			if (vec.v[Z] < RenderSettings.NearClip || vec.v[Z] >= RenderSettings.FarClip) continue;

			sx = vec.v[X] * RenderSettings.GeomPers / vec.v[Z] + REAL_SCREEN_XHALF;
			sy = vec.v[Y] * RenderSettings.GeomPers / vec.v[Z] + REAL_SCREEN_YHALF;

			rad = 32 * RenderSettings.GeomPers / vec.v[Z];

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
			if (Keys[DIK_RETURN])
				CurrentEditCamNode = nnode;
			else
				LastEditCamNode = nnode;
			return;
		}
	}

// new node?

	if (Keys[DIK_INSERT] && !LastKeys[DIK_INSERT])
	{
		if ((node = AllocEditCamNode()))
		{
			vec.v[X] = 0;
			vec.v[Y] = 0;
			vec.v[Z] = 512;
			RotVector(&CAM_MainCamera->WMatrix, &vec, &vec2);
			AddVector(&CAM_MainCamera->WPos, &vec2, &node->Pos);

			node->Type = 0;
			node->ZoomFactor = 500;

			node->Link = NULL;

			node->ID = ++MaxEditCamNodeID;

			CurrentEditCamNode = node;
		}
	}

// quit now if no current edit node

	if (!CurrentEditCamNode) return;

// exit current edit?

	if (Keys[DIK_RETURN] && !LastKeys[DIK_RETURN])
	{
		CurrentEditCamNode = NULL;
		return;
	}

// delete current edit node?

	if (Keys[DIK_DELETE] && !LastKeys[DIK_DELETE])
	{
		FreeEditCamNode(CurrentEditCamNode);
		CurrentEditCamNode = NULL;
		return;
	}

// change axis?

	if (Keys[DIK_TAB] && !LastKeys[DIK_TAB])
	{
		if (Keys[DIK_LSHIFT]) EditCamNodeAxis--;
		else EditCamNodeAxis++;
		if (EditCamNodeAxis == -1) EditCamNodeAxis = 5;
		if (EditCamNodeAxis == 6) EditCamNodeAxis = 0;
	}

// change axis type?

	if (Keys[DIK_LALT] && !LastKeys[DIK_LALT])
		EditCamNodeAxisType ^= 1;

// change type?

	if (Keys[DIK_NUMPADENTER] && !LastKeys[DIK_NUMPADENTER])
		CurrentEditCamNode->Type ^= 1;

// change flag?

	if (Keys[DIK_NUMPADSLASH] && !LastKeys[DIK_NUMPADSLASH])
		CurrentEditCamNode->ZoomFactor -= 50;

	if (Keys[DIK_NUMPADSTAR] && !LastKeys[DIK_NUMPADSTAR])
		CurrentEditCamNode->ZoomFactor += 50;

	if (CurrentEditCamNode->ZoomFactor < -1000) CurrentEditCamNode->ZoomFactor = -1000;
	if (CurrentEditCamNode->ZoomFactor > 1000) CurrentEditCamNode->ZoomFactor = 1000;

// add link?

	if (Keys[DIK_NUMPADPLUS] && !LastKeys[DIK_NUMPADPLUS] && LastEditCamNode && LastEditCamNode != CurrentEditCamNode)
	{
		CurrentEditCamNode->Link = LastEditCamNode;
		LastEditCamNode->Link = CurrentEditCamNode;
		CurrentEditCamNode->Type = LastEditCamNode->Type = CAMNODE_MONORAIL;
	}

// delete link?

	if (Keys[DIK_NUMPADMINUS] && !LastKeys[DIK_NUMPADMINUS] && LastEditCamNode && CurrentEditCamNode != LastEditCamNode)
	{
		CurrentEditCamNode->Link = NULL;
		LastEditCamNode->Link = NULL;
		CurrentEditCamNode->Type = LastEditCamNode->Type = CAMNODE_STATIC;
	}

// move?

	if (MouseLeft)
	{
		RotTransVector(&ViewMatrix, &ViewTrans, &CurrentEditCamNode->Pos, &vec);

		switch (EditCamNodeAxis)
		{
			case EDIT_CAM_NODE_AXIS_XY:
				vec.v[X] = MouseXrel * vec.v[Z] / RenderSettings.GeomPers + CameraEditXrel;
				vec.v[Y] = MouseYrel * vec.v[Z] / RenderSettings.GeomPers + CameraEditYrel;
				vec.v[Z] = CameraEditZrel;
				break;
			case EDIT_CAM_NODE_AXIS_XZ:
				vec.v[X] = MouseXrel * vec.v[Z] / RenderSettings.GeomPers + CameraEditXrel;
				vec.v[Y] = CameraEditYrel;
				vec.v[Z] = -MouseYrel * vec.v[Z] / RenderSettings.GeomPers + CameraEditZrel;
				break;
			case EDIT_CAM_NODE_AXIS_ZY:
				vec.v[X] = CameraEditXrel;
				vec.v[Y] = MouseYrel * vec.v[Z] / RenderSettings.GeomPers + CameraEditYrel;
				vec.v[Z] = MouseXrel * vec.v[Z] / RenderSettings.GeomPers + CameraEditZrel;
				break;
			case EDIT_CAM_NODE_AXIS_X:
				vec.v[X] = MouseXrel * vec.v[Z] / RenderSettings.GeomPers + CameraEditXrel;
				vec.v[Y] = CameraEditYrel;
				vec.v[Z] = CameraEditZrel;
				break;
			case EDIT_CAM_NODE_AXIS_Y:
				vec.v[X] = CameraEditXrel;
				vec.v[Y] = MouseYrel * vec.v[Z] / RenderSettings.GeomPers + CameraEditYrel;
				vec.v[Z] = CameraEditZrel;
				break;
			case EDIT_CAM_NODE_AXIS_Z:
				vec.v[X] = CameraEditXrel;
				vec.v[Y] = CameraEditYrel;
				vec.v[Z] = -MouseYrel * vec.v[Z] / RenderSettings.GeomPers + CameraEditZrel;
				break;
		}

		if (EditCamNodeAxisType == 1) 
		{
			SetVector(&vec2, vec.v[X], vec.v[Y], vec.v[Z]);
		}
		else
		{
			RotVector(&CAM_MainCamera->WMatrix, &vec, &vec2);
		}

		CurrentEditCamNode->Pos.v[X] += vec2.v[X];
		CurrentEditCamNode->Pos.v[Y] += vec2.v[Y];
		CurrentEditCamNode->Pos.v[Z] += vec2.v[Z];
	}
}

////////////////////////////
// alloc an edit cam node //
////////////////////////////

EDIT_CAM_NODE *AllocEditCamNode(void)
{

// full?

	if (EditCamNodeNum >= MAX_EDIT_CAM_NODES)
		return NULL;

// inc counter, return slot

	return &EditCamNode[EditCamNodeNum++];
}

///////////////////////////
// free an edit cam node //
///////////////////////////

void FreeEditCamNode(EDIT_CAM_NODE *node)
{
	long idx, i;

// null any links that reference to this node

	for (i = 0 ; i < EditCamNodeNum ; i++)
	{
		if (EditCamNode[i].Link == node)
		{
			EditCamNode[i].Link = NULL;
			EditCamNode[i].Type = CAMNODE_STATIC;

		}
	}

// find index into list

	idx = (long)(node - EditCamNode);

// copy all higher nodes down one

	for (i = idx ; i < EditCamNodeNum - 1; i++)
	{
		EditCamNode[i] = EditCamNode[i + 1];
	}

// dec num

	EditCamNodeNum--;

// fix any links that reference higher nodes

	for (i = 0 ; i < EditCamNodeNum ; i++)
	{
		if (EditCamNode[i].Link > node) EditCamNode[i].Link--;
	}
}
