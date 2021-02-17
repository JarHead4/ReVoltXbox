
#include "revolt.h"
#include "edittrig.h"
#include "main.h"
#include "dx.h"
#include "geom.h"
#include "camera.h"
#include "text.h"
#include "input.h"
#include "level.h"
#include "trigger.h"

// globals

FILE_TRIGGER *CurrentTrigger = NULL;

static long FileTriggerNum;
static FILE_TRIGGER *FileTriggers;
static long FileTriggerAxis, FileTriggerAxisType;
static long FileTriggerSide, LastFileTriggerID;

// misc edit text

static char *FileTriggerAxisNames[] = {
	"X Y",
	"X Z",
	"Z Y",
	"X",
	"Y",
	"Z",
};

static char *FileTriggerAxisTypeNames[] = {
	"Camera",
	"World",
};

// misc draw info

static unsigned short DrawTriggerIndex[] = {
	1, 3, 7, 5,
	2, 0, 4, 6,
	4, 5, 7, 6,
	0, 2, 3, 1,
	3, 2, 6, 7,
	0, 1, 5, 4,
};

static long DrawTriggerCol[] = {
	0x80ff0000,
	0x8000ff00,
	0x800000ff,
	0x80ffff00,
	0x80ff00ff,
	0x8000ffff,
};

// enums

char *TriggerEnumTrackDir[] = {
	"Chicane left",
	"180 left",
	"90 left",
	"45 left",
	"Chicane right",
	"180 Right",
	"90 Right",
	"45 Right",
	"Ahead",
	"Danger",
	"Fork",

	NULL
};

char *TriggerEnumCameraRail[] = {
	"Collision: None",
	"Collision: Full",

	NULL
};

// ID names

static char *TriggerNames[] = {
	"Piano",
	"Split",
	"Track dir",
	"CameraRail",
	"AI Home",

	NULL
};

static char **TriggerEnums[] = {
	NULL,
	NULL,
	TriggerEnumTrackDir,
	NULL,
	NULL,
};

////////////////////////
// init file triggers //
////////////////////////

void InitFileTriggers(void)
{
	FileTriggers = (FILE_TRIGGER*)malloc(sizeof(FILE_TRIGGER) * MAX_FILE_TRIGGERS);
	if (!FileTriggers)
	{
		Box(NULL, "Can't alloc memory for file triggers!", MB_OK);
		QuitGame = TRUE;
	}
}

////////////////////////
// kill file triggers //
////////////////////////

void KillFileTriggers(void)
{
	free(FileTriggers);
}

////////////////////////
// load file triggers //
////////////////////////

void LoadFileTriggers(char *file)
{
	long i;
	FILE *fp;
	FILE_TRIGGER ftri;

// open trigger file

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

// loop thru all triggers

	fread(&FileTriggerNum, sizeof(FileTriggerNum), 1, fp);

	for (i = 0 ; i < FileTriggerNum ; i++)
	{

// load one file trigger

		fread(&ftri, sizeof(ftri), 1, fp);

		VecMulScalar(&ftri.Pos, EditScale);

// setup edit trigger

		FileTriggers[i] = ftri;
	}

// close trigger file

	fclose(fp);
}

////////////////////////
// save file triggers //
////////////////////////

void SaveFileTriggers(char *file)
{
	long i;
	FILE *fp;
	FILE_TRIGGER ftri;
	char bak[256];

// backup old file

	memcpy(bak, file, strlen(file) - 3);
	wsprintf(bak + strlen(file) - 3, "tr-");
	remove(bak);
	rename(file, bak);

// open trigger file

	fp = fopen(file, "wb");
	if (!fp) return;

// write num

	fwrite(&FileTriggerNum, sizeof(FileTriggerNum), 1, fp);

// write out each trigger

	for (i = 0 ; i < FileTriggerNum ; i++)
	{

// set file trigger

		ftri = FileTriggers[i];

// write it

		fwrite(&ftri, sizeof(ftri), 1, fp);
	}

// close file

	Box("Saved Trigger File:", file, MB_OK);
	fclose(fp);
}

//////////////////////////
// alloc a file trigger //
//////////////////////////

FILE_TRIGGER *AllocFileTrigger(void)
{

// full?

	if (FileTriggerNum >= MAX_FILE_TRIGGERS)
		return NULL;

// inc counter, return slot

	return &FileTriggers[FileTriggerNum++];
}

/////////////////////////
// free a file trigger //
/////////////////////////

void FreeFileTrigger(FILE_TRIGGER *trigger)
{
	long idx, i;

// find index into list

	idx = (long)(trigger - FileTriggers);

// copy all higher triggers down one

	for (i = idx ; i < FileTriggerNum - 1; i++)
	{
		FileTriggers[i] = FileTriggers[i + 1];
	}

// dec num

	FileTriggerNum--;
}

////////////////////////
// draw file triggers //
////////////////////////

void DrawTriggers(void)
{
	long i, j, k, col[4];
	FILE_TRIGGER *trigger;
	REAL time;
	VEC v[8], vpos[8], pos[4];
	MAT mat;
	VEC vec, *v1, *v2, *v3, *v4;
	char buf[128];

// set render states

	ALPHA_SRC(D3DBLEND_SRCALPHA);
	ALPHA_DEST(D3DBLEND_INVSRCALPHA);
	WIREFRAME_OFF();
	ZWRITE_OFF();

// loop thru all triggers

	trigger = FileTriggers;
	for (i = 0 ; i < FileTriggerNum ; i++, trigger++)
	{

// get 8 corners

		SetVector(&v[0], -trigger->Size[X], -trigger->Size[Y], -trigger->Size[Z]);
		SetVector(&v[1], trigger->Size[X], -trigger->Size[Y], -trigger->Size[Z]);
		SetVector(&v[2], -trigger->Size[X], -trigger->Size[Y], trigger->Size[Z]);
		SetVector(&v[3], trigger->Size[X], -trigger->Size[Y], trigger->Size[Z]);

		SetVector(&v[4], -trigger->Size[X], trigger->Size[Y], -trigger->Size[Z]);
		SetVector(&v[5], trigger->Size[X], trigger->Size[Y], -trigger->Size[Z]);
		SetVector(&v[6], -trigger->Size[X], trigger->Size[Y], trigger->Size[Z]);
		SetVector(&v[7], trigger->Size[X], trigger->Size[Y], trigger->Size[Z]);

		for (j = 0 ; j < 8 ; j++)
		{
			RotTransVector(&trigger->Matrix, &trigger->Pos, &v[j], &vpos[j]);
		}

// draw

		SET_TPAGE(-1);
		ALPHA_ON();
		ZBUFFER_ON();

		for (j = 0 ; j < 6 ; j++)
		{
			for (k = 0 ; k < 4 ; k++)
			{
				pos[k] = vpos[DrawTriggerIndex[j * 4 + k]];

				if (CurrentTrigger == trigger && j == FileTriggerSide)
					col[k] = 0x80ffffff * (FrameCount & 1);
				else
					col[k] = DrawTriggerCol[trigger->ID % 6];
			}

		DrawNearClipPolyTEX0(pos, col, 4);
		}

// display name

		wsprintf(buf, "%s", TriggerNames[trigger->ID]);
		RotTransVector(&ViewMatrix, &ViewTrans, &trigger->Pos, &vec);
		vec.v[X] -= 32 * strlen(TriggerNames[trigger->ID]);
		vec.v[Y] -= 64;
		if (vec.v[Z] > RenderSettings.NearClip)
		{
			ALPHA_OFF();
			ZBUFFER_OFF();
			DumpText3D(&vec, 64, 128, 0x00ffff, buf);
		}

// draw axis?

		if (CurrentTrigger == trigger)
		{
			ALPHA_OFF();
			ZBUFFER_OFF();

			if (FileTriggerAxisType)
				CopyMatrix(&IdentityMatrix, &mat);
			else
				CopyMatrix(&CAM_MainCamera->WMatrix, &mat);

			MatMulScalar(&mat, 2.0f);
			DrawAxis(&mat, &trigger->Pos);
		}

// draw 'special case' AI home line

		if (!strcmp(TriggerNames[trigger->ID], "AI Home"))
		{
			trigger->Flag &= 31;

			if (trigger->Flag < 8)
			{
				v1 = &v[0];
				v2 = &v[2];
				v3 = &v[4];
				v4 = &v[6];
				time = (float)trigger->Flag / 8.0f;
			}
			else if (trigger->Flag < 16)
			{
				v1 = &v[2];
				v2 = &v[3];
				v3 = &v[6];
				v4 = &v[7];
				time = (float)(trigger->Flag - 8) / 8.0f;
			}
			else if (trigger->Flag < 24)
			{
				v1 = &v[3];
				v2 = &v[1];
				v3 = &v[7];
				v4 = &v[5];
				time = (float)(trigger->Flag - 16) / 8.0f;
			}
			else if (trigger->Flag < 32)
			{
				v1 = &v[1];
				v2 = &v[0];
				v3 = &v[5];
				v4 = &v[4];
				time = (float)(trigger->Flag - 24) / 8.0f;
			}

			v[0].v[X] = v1->v[X] + (v2->v[X] - v1->v[X]) * time;
			v[0].v[Y] = v1->v[Y] + (v2->v[Y] - v1->v[Y]) * time;
			v[0].v[Z] = v1->v[Z] + (v2->v[Z] - v1->v[Z]) * time;

			v[1].v[X] = v3->v[X] + (v4->v[X] - v3->v[X]) * time;
			v[1].v[Y] = v3->v[Y] + (v4->v[Y] - v3->v[Y]) * time;
			v[1].v[Z] = v3->v[Z] + (v4->v[Z] - v3->v[Z]) * time;

			RotTransVector(&trigger->Matrix, &trigger->Pos, &v[0], &v[2]);
			RotTransVector(&trigger->Matrix, &trigger->Pos, &v[1], &v[3]);

			ALPHA_OFF();
			ZBUFFER_OFF();

			DrawLine(&v[2], &v[3], 0x00ff00, 0x00ff00);
		}
	}

// reset render states

	WIREFRAME_ON();
	ZBUFFER_ON();
	ZWRITE_ON();
	ALPHA_OFF();
}

//////////////////////////
// display trigger info //
//////////////////////////

void DisplayTriggerInfo(FILE_TRIGGER *trigger)
{
	char buf[128];

// ID

	wsprintf(buf, "%s", TriggerNames[trigger->ID]);
	DumpText(450, 0, 8, 16, 0xffff00, buf);

// Flag

	if (!TriggerEnums[trigger->ID])
	{
		wsprintf(buf, "Flag %d", trigger->Flag);
		DumpText(450, 24, 8, 16, 0x00ffff, buf);
	}
	else
	{
		wsprintf(buf, "Flag %s", TriggerEnums[trigger->ID][trigger->Flag]);
		DumpText(450, 24, 8, 16, 0x00ffff, buf);
	}

// axis

	wsprintf(buf, "Axis %s - %s", FileTriggerAxisNames[FileTriggerAxis], FileTriggerAxisTypeNames[FileTriggerAxisType]);
	DumpText(450, 48, 8, 16, 0xff00ff, buf);
}

//////////////////////////
// edit current trigger //
//////////////////////////

void EditTriggers(void)
{
	long i, j;
	VEC vec, vec2, r, u, l, r2, u2, l2;
	float z, sx, sy, rad, add;
	MAT mat, mat2;
	FILE_TRIGGER *ntrigger, *trigger;

// quit if not in edit mode

	if (CAM_MainCamera->Type != CAM_EDIT)
	{
		CurrentTrigger = NULL;
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

// save triggers?

	if (Keys[DIK_LCONTROL] && Keys[DIK_F4] && !LastKeys[DIK_F4])
	{
		SaveFileTriggers(GetLevelFilename("tri", FILENAME_MAKE_BODY | FILENAME_GAME_SETTINGS));
		FreeTriggers();
		LoadTriggers(GetLevelFilename("tri", FILENAME_MAKE_BODY | FILENAME_GAME_SETTINGS));
	}

// get a current trigger?

	if (!CurrentTrigger && Keys[DIK_RETURN] && !LastKeys[DIK_RETURN])
	{
		ntrigger = NULL;
		z = RenderSettings.FarClip;

		trigger = FileTriggers;
		for (i = 0 ; i < FileTriggerNum ; i++, trigger++)
		{
			for (j = 0 ; j < 2 ; j++)
			{
				RotTransVector(&ViewMatrix, &ViewTrans, &trigger->Pos, &vec);

				if (vec.v[Z] < RenderSettings.NearClip || vec.v[Z] >= RenderSettings.FarClip) continue;

				sx = vec.v[X] * RenderSettings.GeomPers / vec.v[Z] + REAL_SCREEN_XHALF;
				sy = vec.v[Y] * RenderSettings.GeomPers / vec.v[Z] + REAL_SCREEN_YHALF;

				rad = 128 * RenderSettings.GeomPers / vec.v[Z];

				if (MouseXpos > sx - rad && MouseXpos < sx + rad && MouseYpos > sy - rad && MouseYpos < sy + rad)
				{
					if (vec.v[Z] < z)
					{
						ntrigger = trigger;
						z = vec.v[Z];
					}
				}
			}
		}
		if (ntrigger)
		{
			CurrentTrigger = ntrigger;
			return;
		}
	}

// new trigger?

	if (Keys[DIK_INSERT] && !LastKeys[DIK_INSERT])
	{
		if ((trigger = AllocFileTrigger()))
		{
			vec.v[X] = 0;
			vec.v[Y] = 0;
			vec.v[Z] = 512;
			RotVector(&CAM_MainCamera->WMatrix, &vec, &vec2);
			AddVector(&CAM_MainCamera->WPos, &vec2, &trigger->Pos);

			CopyMatrix(&IdentityMatrix, &trigger->Matrix);

			trigger->Size[X] = trigger->Size[Y] = trigger->Size[Z] = 128;
			trigger->ID = LastFileTriggerID;
			trigger->Flag = 0;

			CurrentTrigger = trigger;
		}
	}

// quit now if no current file trigger

	if (!CurrentTrigger) return;

// save ID to last

	LastFileTriggerID = CurrentTrigger->ID;

// exit current edit?

	if (Keys[DIK_RETURN] && !LastKeys[DIK_RETURN])
	{
		CurrentTrigger = NULL;
		return;
	}

// delete current trigger?

	if (Keys[DIK_DELETE] && !LastKeys[DIK_DELETE])
	{
		FreeFileTrigger(CurrentTrigger);
		CurrentTrigger = NULL;
		return;
	}

// change axis?

	if (Keys[DIK_TAB] && !LastKeys[DIK_TAB])
	{
		if (Keys[DIK_LSHIFT]) FileTriggerAxis--;
		else FileTriggerAxis++;
		if (FileTriggerAxis == -1) FileTriggerAxis = 5;
		if (FileTriggerAxis == 6) FileTriggerAxis = 0;
	}

// change axis type?

	if (Keys[DIK_LALT] && !LastKeys[DIK_LALT])
		FileTriggerAxisType ^= 1;

// change ID?

	if (Keys[DIK_NUMPADMINUS] && !LastKeys[DIK_NUMPADMINUS] && CurrentTrigger->ID)
		CurrentTrigger->ID--;
	if (Keys[DIK_NUMPADPLUS] && !LastKeys[DIK_NUMPADPLUS] && TriggerNames[CurrentTrigger->ID + 1])
		CurrentTrigger->ID++;

// change Flag?

	if (Keys[DIK_MINUS] && !LastKeys[DIK_MINUS])
	{
		CurrentTrigger->Flag--;
	}
	if (Keys[DIK_EQUALS] && !LastKeys[DIK_EQUALS])
	{
		CurrentTrigger->Flag++;
	}

	if (CurrentTrigger->Flag < 0)
		CurrentTrigger->Flag = 0;

	if (TriggerEnums[CurrentTrigger->ID])
	{
		i = 0;
		while (TriggerEnums[CurrentTrigger->ID][i]) i++;
		if (CurrentTrigger->Flag > i - 1)
			CurrentTrigger->Flag = i - 1;
	}

// change side?

	if (Keys[DIK_SPACE] && !LastKeys[DIK_SPACE])
		FileTriggerSide = (FileTriggerSide + 1) % 6;

// resize?

	add = 0;
	if (Keys[DIK_NUMPADSTAR])
		add = 4 * TimeFactor;
	if (Keys[DIK_NUMPADSLASH])
		add = -4 * TimeFactor;

	if (Keys[DIK_LCONTROL]) add *= 4;

	if (add)
	{
		if (Keys[DIK_LSHIFT])
		{
			CurrentTrigger->Size[X] += add;
			if (CurrentTrigger->Size[X] < 16) CurrentTrigger->Size[X] = 16;
			CurrentTrigger->Size[Y] += add;
			if (CurrentTrigger->Size[Y] < 16) CurrentTrigger->Size[Y] = 16;
			CurrentTrigger->Size[Z] += add;
			if (CurrentTrigger->Size[Z] < 16) CurrentTrigger->Size[Z] = 16;
		}
		else
		{
			switch (FileTriggerSide)
			{
				case 0:
					CurrentTrigger->Size[X] += add;
					if (CurrentTrigger->Size[X] < 16) CurrentTrigger->Size[X] = 16;
					CurrentTrigger->Pos.v[X] += CurrentTrigger->Matrix.m[RX] * add;
					CurrentTrigger->Pos.v[Y] += CurrentTrigger->Matrix.m[RY] * add;
					CurrentTrigger->Pos.v[Z] += CurrentTrigger->Matrix.m[RZ] * add;
					break;
				case 1:
					CurrentTrigger->Size[X] += add;
					if (CurrentTrigger->Size[X] < 16) CurrentTrigger->Size[X] = 16;
					CurrentTrigger->Pos.v[X] -= CurrentTrigger->Matrix.m[RX] * add;
					CurrentTrigger->Pos.v[Y] -= CurrentTrigger->Matrix.m[RY] * add;
					CurrentTrigger->Pos.v[Z] -= CurrentTrigger->Matrix.m[RZ] * add;
					break;
				case 2:
					CurrentTrigger->Size[Y] += add;
					if (CurrentTrigger->Size[Y] < 16) CurrentTrigger->Size[Y] = 16;
					CurrentTrigger->Pos.v[X] += CurrentTrigger->Matrix.m[UX] * add;
					CurrentTrigger->Pos.v[Y] += CurrentTrigger->Matrix.m[UY] * add;
					CurrentTrigger->Pos.v[Z] += CurrentTrigger->Matrix.m[UZ] * add;
					break;
				case 3:
					CurrentTrigger->Size[Y] += add;
					if (CurrentTrigger->Size[Y] < 16) CurrentTrigger->Size[Y] = 16;
					CurrentTrigger->Pos.v[X] -= CurrentTrigger->Matrix.m[UX] * add;
					CurrentTrigger->Pos.v[Y] -= CurrentTrigger->Matrix.m[UY] * add;
					CurrentTrigger->Pos.v[Z] -= CurrentTrigger->Matrix.m[UZ] * add;
					break;
				case 4:
					CurrentTrigger->Size[Z] += add;
					if (CurrentTrigger->Size[Z] < 16) CurrentTrigger->Size[Z] = 16;
					CurrentTrigger->Pos.v[X] += CurrentTrigger->Matrix.m[LX] * add;
					CurrentTrigger->Pos.v[Y] += CurrentTrigger->Matrix.m[LY] * add;
					CurrentTrigger->Pos.v[Z] += CurrentTrigger->Matrix.m[LZ] * add;
					break;
				case 5:
					CurrentTrigger->Size[Z] += add;
					if (CurrentTrigger->Size[Z] < 16) CurrentTrigger->Size[Z] = 16;
					CurrentTrigger->Pos.v[X] -= CurrentTrigger->Matrix.m[LX] * add;
					CurrentTrigger->Pos.v[Y] -= CurrentTrigger->Matrix.m[LY] * add;
					CurrentTrigger->Pos.v[Z] -= CurrentTrigger->Matrix.m[LZ] * add;
					break;
			}
		}
	}

// move?

	if (MouseLeft)
	{
		RotTransVector(&ViewMatrix, &ViewTrans, &CurrentTrigger->Pos, &vec);

		switch (FileTriggerAxis)
		{
			case FILE_TRIGGER_AXIS_XY:
				vec.v[X] = MouseXrel * vec.v[Z] / RenderSettings.GeomPers + CameraEditXrel;
				vec.v[Y] = MouseYrel * vec.v[Z] / RenderSettings.GeomPers + CameraEditYrel;
				vec.v[Z] = CameraEditZrel;
				break;
			case FILE_TRIGGER_AXIS_XZ:
				vec.v[X] = MouseXrel * vec.v[Z] / RenderSettings.GeomPers + CameraEditXrel;
				vec.v[Y] = CameraEditYrel;
				vec.v[Z] = -MouseYrel * vec.v[Z] / RenderSettings.GeomPers + CameraEditZrel;
				break;
			case FILE_TRIGGER_AXIS_ZY:
				vec.v[X] = CameraEditXrel;
				vec.v[Y] = MouseYrel * vec.v[Z] / RenderSettings.GeomPers + CameraEditYrel;
				vec.v[Z] = MouseXrel * vec.v[Z] / RenderSettings.GeomPers + CameraEditZrel;
				break;
			case FILE_TRIGGER_AXIS_X:
				vec.v[X] = MouseXrel * vec.v[Z] / RenderSettings.GeomPers + CameraEditXrel;
				vec.v[Y] = CameraEditYrel;
				vec.v[Z] = CameraEditZrel;
				break;
			case FILE_TRIGGER_AXIS_Y:
				vec.v[X] = CameraEditXrel;
				vec.v[Y] = MouseYrel * vec.v[Z] / RenderSettings.GeomPers + CameraEditYrel;
				vec.v[Z] = CameraEditZrel;
				break;
			case FILE_TRIGGER_AXIS_Z:
				vec.v[X] = CameraEditXrel;
				vec.v[Y] = CameraEditYrel;
				vec.v[Z] = -MouseYrel * vec.v[Z] / RenderSettings.GeomPers + CameraEditZrel;
				break;
		}

		if (FileTriggerAxisType == 1) 
		{
			SetVector(&vec2, vec.v[X], vec.v[Y], vec.v[Z]);
		}
		else
		{
			RotVector(&CAM_MainCamera->WMatrix, &vec, &vec2);
		}

		CurrentTrigger->Pos.v[X] += vec2.v[X];
		CurrentTrigger->Pos.v[Y] += vec2.v[Y];
		CurrentTrigger->Pos.v[Z] += vec2.v[Z];
	}

// rotate?

	vec.v[X] = vec.v[Y] = vec.v[Z] = 0;

	if (Keys[DIK_NUMPAD7]) vec.v[X] -= 0.005f;
	if (Keys[DIK_NUMPAD4]) vec.v[X] += 0.005f;
	if (Keys[DIK_NUMPAD8]) vec.v[Y] -= 0.005f;
	if (Keys[DIK_NUMPAD5]) vec.v[Y] += 0.005f;
	if (Keys[DIK_NUMPAD9]) vec.v[Z] -= 0.005f;
	if (Keys[DIK_NUMPAD6]) vec.v[Z] += 0.005f;

	if (Keys[DIK_NUMPAD1] && !LastKeys[DIK_NUMPAD1]) vec.v[X] += 0.25f;
	if (Keys[DIK_NUMPAD2] && !LastKeys[DIK_NUMPAD2]) vec.v[Y] += 0.25f;
	if (Keys[DIK_NUMPAD3] && !LastKeys[DIK_NUMPAD3]) vec.v[Z] += 0.25f;

	if (Keys[DIK_NUMPAD0]) CopyMatrix(&IdentityMatrix, &CurrentTrigger->Matrix);

	RotMatrixZYX(&mat, vec.v[X], vec.v[Y], vec.v[Z]);

	if (FileTriggerAxisType)
	{
		MulMatrix(&mat, &CurrentTrigger->Matrix, &mat2);
		CopyMatrix(&mat2, &CurrentTrigger->Matrix);
		NormalizeMatrix(&CurrentTrigger->Matrix);
	}
	else if (vec.v[X] || vec.v[Y] || vec.v[Z])
	{
		RotVector(&ViewMatrix, &CurrentTrigger->Matrix.mv[X], &r);
		RotVector(&ViewMatrix, &CurrentTrigger->Matrix.mv[Y], &u);
		RotVector(&ViewMatrix, &CurrentTrigger->Matrix.mv[Z], &l);

		RotVector(&mat, &r, &r2);
		RotVector(&mat, &u, &u2);
		RotVector(&mat, &l, &l2);

		RotVector(&CAM_MainCamera->WMatrix, &r2, &CurrentTrigger->Matrix.mv[X]);
		RotVector(&CAM_MainCamera->WMatrix, &u2, &CurrentTrigger->Matrix.mv[Y]);
		RotVector(&CAM_MainCamera->WMatrix, &l2, &CurrentTrigger->Matrix.mv[Z]);

		NormalizeMatrix(&CurrentTrigger->Matrix);
	}
}
