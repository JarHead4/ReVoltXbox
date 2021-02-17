
#include "revolt.h"
#include "edfield.h"
#include "main.h"
#include "dx.h"
#include "geom.h"
#include "camera.h"
#include "text.h"
#include "input.h"
#include "level.h"
#include "model.h"

// globals

FILE_FIELD *CurrentField = NULL;

static long FileFieldNum;
static FILE_FIELD *FileFields;
static long FileFieldAxis, FileFieldAxisType;
static long FileFieldSide, FileFieldRotType;
static MODEL FileFieldModel[2];
static long Picker, EnterFlag;
static char EnterString[128];

// misc

static char *FileFieldTypeNames[] = {
	"Linear",
	"Orientation",
	"Velocity",
	"Spherical",
};

static char MaxPicker[] = {
	2,
	2,
	1,
	2,
};

static char *FileFieldRotNames[] = {
	"Box",
	"Dir",
};

static char *FileFieldAxisNames[] = {
	"X Y",
	"X Z",
	"Z Y",
	"X",
	"Y",
	"Z",
};

static char *FileFieldAxisTypeNames[] = {
	"Camera",
	"World",
};

static unsigned short DrawFieldIndex[] = {
	1, 3, 7, 5,
	2, 0, 4, 6,
	4, 5, 7, 6,
	0, 2, 3, 1,
	3, 2, 6, 7,
	0, 1, 5, 4,
};

static long DrawFieldCol[] = {
	0x80ff0000,
	0x8000ff00,
	0x800000ff,
};

//////////////////////
// init file fields //
//////////////////////

void InitFileFields(void)
{
	FileFields = (FILE_FIELD*)malloc(sizeof(FILE_FIELD) * MAX_FILE_FIELDS);
	if (!FileFields)
	{
		Box(NULL, "Can't alloc memory for file fields!", MB_OK);
		QuitGame = TRUE;
	}
}

//////////////////////
// kill file fields //
//////////////////////

void KillFileFields(void)
{
	free(FileFields);
}

//////////////////////
// load file fields //
//////////////////////

void LoadFileFields(char *file)
{
	long i;
	FILE *fp;
	FILE_FIELD ff;

// open field file

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

// loop thru all fields

	fread(&FileFieldNum, sizeof(FileFieldNum), 1, fp);

	for (i = 0 ; i < FileFieldNum ; i++)
	{

// load one file field

		fread(&ff, sizeof(ff), 1, fp);

		VecMulScalar(&ff.Pos, EditScale);
		VecMulScalar((VEC*)ff.Size, EditScale);

// setup edit field

		FileFields[i] = ff;
	}

// close file

	fclose(fp);
}

//////////////////////
// save file fields //
//////////////////////

void SaveFileFields(char *file)
{
	long i;
	FILE *fp;
	FILE_FIELD ff;
	char bak[256];

// backup old file

	memcpy(bak, file, strlen(file) - 3);
	wsprintf(bak + strlen(file) - 3, "fl-");
	remove(bak);
	rename(file, bak);

// open field file

	fp = fopen(file, "wb");
	if (!fp) return;

// write num

	fwrite(&FileFieldNum, sizeof(FileFieldNum), 1, fp);

// write out each field

	for (i = 0 ; i < FileFieldNum ; i++)
	{

// set file field

		ff = FileFields[i];

// write it

		fwrite(&ff, sizeof(ff), 1, fp);
	}

// close file

	Box("Saved Field File:", file, MB_OK);
	fclose(fp);
}

////////////////////////
// alloc a file field //
////////////////////////

FILE_FIELD *AllocFileField(void)
{

// full?

	if (FileFieldNum >= MAX_FILE_FIELDS)
		return NULL;

// inc counter, return slot

	return &FileFields[FileFieldNum++];
}

///////////////////////
// free a file field //
///////////////////////

void FreeFileField(FILE_FIELD *field)
{
	long idx, i;

// find index into list

	idx = (long)(field - FileFields);

// copy all higher fields down one

	for (i = idx ; i < FileFieldNum - 1; i++)
	{
		FileFields[i] = FileFields[i + 1];
	}

// dec num

	FileFieldNum--;
}

//////////////////////
// draw file fields //
//////////////////////

void DrawFields(void)
{
	long i, j, k, col[4];
	float mul;
	FILE_FIELD *field;
	VEC v[8], vpos[8], pos[4];
	MAT mat;
	VEC vec;
	char buf[128];

// set render states

	WIREFRAME_OFF();

// loop thru all fields

	field = FileFields;
	for (i = 0 ; i < FileFieldNum ; i++, field++)
	{

// spherical

		if (field->Type == FILE_FIELD_TYPE_SPHERICAL)
		{
			ALPHA_ON();
			ALPHA_SRC(D3DBLEND_ONE);
			ALPHA_DEST(D3DBLEND_ONE);
			ZBUFFER_ON();

			CopyMatrix(&IdentityMatrix, &mat);
			mul = field->RadStart / FileFieldModel[0].Radius;
			for (j = 0 ; j < 9 ; j++) mat.m[j] *= mul;
			DrawModel(&FileFieldModel[0], &mat, &field->Pos, MODEL_PLAIN);

			CopyMatrix(&IdentityMatrix, &mat);
			mul = field->RadEnd / FileFieldModel[1].Radius;
			for (j = 0 ; j < 9 ; j++) mat.m[j] *= mul;
			DrawModel(&FileFieldModel[1], &mat, &field->Pos, MODEL_PLAIN);

			FlushPolyBuckets();
		}

// hull

		else
		{

// draw dir vec

			ALPHA_OFF();
			ZBUFFER_ON();

			vec.v[X] = field->Pos.v[X] + field->Dir.v[X] * 256;
			vec.v[Y] = field->Pos.v[Y] + field->Dir.v[Y] * 256;
			vec.v[Z] = field->Pos.v[Z] + field->Dir.v[Z] * 256;

			DrawLine(&field->Pos, &vec, 0xffffff, 0xffffff);

// get 8 corners

			SetVector(&v[0], -field->Size[X], -field->Size[Y], -field->Size[Z]);
			SetVector(&v[1], field->Size[X], -field->Size[Y], -field->Size[Z]);
			SetVector(&v[2], -field->Size[X], -field->Size[Y], field->Size[Z]);
			SetVector(&v[3], field->Size[X], -field->Size[Y], field->Size[Z]);

			SetVector(&v[4], -field->Size[X], field->Size[Y], -field->Size[Z]);
			SetVector(&v[5], field->Size[X], field->Size[Y], -field->Size[Z]);
			SetVector(&v[6], -field->Size[X], field->Size[Y], field->Size[Z]);
			SetVector(&v[7], field->Size[X], field->Size[Y], field->Size[Z]);

			for (j = 0 ; j < 8 ; j++)
			{
				RotTransVector(&field->Matrix, &field->Pos, &v[j], &vpos[j]);
			}

// draw

			SET_TPAGE(-1);
			ALPHA_SRC(D3DBLEND_SRCALPHA);
			ALPHA_DEST(D3DBLEND_INVSRCALPHA);
			ALPHA_ON();
			ZBUFFER_ON();

			for (j = 0 ; j < 6 ; j++)
			{
				for (k = 0 ; k < 4 ; k++)
				{
					pos[k] = vpos[DrawFieldIndex[j * 4 + k]];

					if (CurrentField == field && j == FileFieldSide)
						col[k] = 0x80ffffff * (FrameCount & 1);
					else
						col[k] = DrawFieldCol[field->Type % FILE_FIELD_TYPE_MAX];
				}

			DrawNearClipPolyTEX0(pos, col, 4);
			}
		}

// display name

		wsprintf(buf, "%s", FileFieldTypeNames[field->Type]);
		RotTransVector(&ViewMatrix, &ViewTrans, &field->Pos, &vec);
		vec.v[X] -= 32 * strlen(FileFieldTypeNames[field->Type]);
		vec.v[Y] -= 64;
		if (vec.v[Z] > RenderSettings.NearClip)
		{
			ALPHA_OFF();
			ZBUFFER_OFF();
			DumpText3D(&vec, 64, 128, 0x00ffff, buf);
		}

// draw axis?

		if (CurrentField == field)
		{
			ALPHA_OFF();
			ZBUFFER_OFF();

			if (FileFieldAxisType)
				CopyMatrix(&IdentityMatrix, &mat);
			else
				CopyMatrix(&CAM_MainCamera->WMatrix, &mat);

			MatMulScalar(&mat, 2.0f);
			DrawAxis(&mat, &field->Pos);
		}
	}

// reset render states

	WIREFRAME_ON();
}

////////////////////////
// display field info //
////////////////////////

void DisplayFieldInfo(FILE_FIELD *field)
{
	char buf[256];

// Type

	wsprintf(buf, "%s", FileFieldTypeNames[field->Type]);
	DumpText(450, 0, 8, 16, 0xffff00, buf);

// axis

	wsprintf(buf, "Axis %s - %s", FileFieldAxisNames[FileFieldAxis], FileFieldAxisTypeNames[FileFieldAxisType]);
	DumpText(450, 24, 8, 16, 0xff00ff, buf);

// rot

	wsprintf(buf, "Rot %s", FileFieldRotNames[FileFieldRotType]);
	DumpText(450, 48, 8, 16, 0xff0000, buf);

// mag

	if (field->Type != FILE_FIELD_TYPE_SPHERICAL)
	{
		if (EnterFlag && !Picker) wsprintf(buf, "Mag %s_", EnterString);
		else wsprintf(buf, "Mag %ld.%04ld", (long)field->Mag, (long)((field->Mag - (float)(long)field->Mag) * 10000));
		DumpText(450, 72, 8, 16, 0x00ffff, buf);
	}

// damping

	if (field->Type == FILE_FIELD_TYPE_LINEAR || field->Type == FILE_FIELD_TYPE_ORIENTATION)
	{
		if (EnterFlag && Picker == 1) wsprintf(buf, "Damping %s_", EnterString);
		else wsprintf(buf, "Damping %ld.%04ld", (long)field->Damping, (long)((field->Damping - (float)(long)field->Damping) * 10000));
		DumpText(450, 96, 8, 16, 0x00ff00, buf);
	}

// grad start

	if (field->Type == FILE_FIELD_TYPE_SPHERICAL)
	{
		if (EnterFlag && !Picker) wsprintf(buf, "Grad Start %s_", EnterString);
		else wsprintf(buf, "Grad Start %ld.%04ld", (long)field->GradStart, (long)((field->GradStart - (float)(long)field->GradStart) * 10000));
		DumpText(450, 72, 8, 16, 0x00ffff, buf);
	}

// grad end

	if (field->Type == FILE_FIELD_TYPE_SPHERICAL)
	{
		if (EnterFlag && Picker == 1) wsprintf(buf, "Grad End %s_", EnterString);
		else wsprintf(buf, "Grad End %ld.%04ld", (long)field->GradEnd, (long)((field->GradEnd - (float)(long)field->GradEnd) * 10000));
		DumpText(450, 96, 8, 16, 0x00ff00, buf);
	}

// picker

	DumpText(426, Picker * 24 + 72, 8, 16, 0xffffff, "->");
}

////////////////////////
// edit current field //
////////////////////////

void EditFields(void)
{
	long i, j;
	VEC vec, vec2, r, u, l, r2, u2, l2;
	float z, sx, sy, rad, add, f;
	MAT mat, mat2;
	unsigned char c;
	FILE_FIELD *nfield, *field;

// quit if not in edit mode

	if (CAM_MainCamera->Type != CAM_EDIT)
	{
		CurrentField = NULL;
		EnterFlag = FALSE;
		return;
	}

// entering number?

	if (EnterFlag)
	{
		if ((c = GetKeyPress()))
		{
			if (c == 8)
			{
				if (strlen(EnterString))
				{
					EnterString[strlen(EnterString) - 1] = 0;
				}
			}
			else if (c != 13 && c != 27)
			{
				if (strlen(EnterString) < 127)
				{
					EnterString[strlen(EnterString) + 1] = 0;
					EnterString[strlen(EnterString)] = c;
				}
			}
			if (c == 13)
			{
				f = (float)atof(EnterString);
				switch (Picker)
				{
					case 0:
						if (CurrentField->Type == FILE_FIELD_TYPE_SPHERICAL) CurrentField->GradStart = f;
						else CurrentField->Mag = f;
					break;

					case 1:
						if (CurrentField->Type == FILE_FIELD_TYPE_SPHERICAL) CurrentField->GradEnd = f;
						else CurrentField->Damping = f;
					break;
				}

				EnterFlag = FALSE;
			}
		}
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

// save fields?

	if (Keys[DIK_LCONTROL] && Keys[DIK_F4] && !LastKeys[DIK_F4])
	{
		SaveFileFields(GetLevelFilename("fld", FILENAME_MAKE_BODY | FILENAME_GAME_SETTINGS));
	}

// get a current field?

	if (Keys[DIK_RETURN] && !LastKeys[DIK_RETURN])
	{
		nfield = NULL;
		z = RenderSettings.FarClip;

		field = FileFields;
		for (i = 0 ; i < FileFieldNum ; i++, field++)
		{
			for (j = 0 ; j < 2 ; j++)
			{
				RotTransVector(&ViewMatrix, &ViewTrans, &field->Pos, &vec);

				if (vec.v[Z] < RenderSettings.NearClip || vec.v[Z] >= RenderSettings.FarClip) continue;

				sx = vec.v[X] * RenderSettings.GeomPers / vec.v[Z] + REAL_SCREEN_XHALF;
				sy = vec.v[Y] * RenderSettings.GeomPers / vec.v[Z] + REAL_SCREEN_YHALF;

				rad = 128 * RenderSettings.GeomPers / vec.v[Z];

				if (MouseXpos > sx - rad && MouseXpos < sx + rad && MouseYpos > sy - rad && MouseYpos < sy + rad)
				{
					if (vec.v[Z] < z)
					{
						nfield = field;
						z = vec.v[Z];
					}
				}
			}
		}
		if (nfield)
		{
			CurrentField = nfield;
			return;
		}
	}

// new field?

	if (Keys[DIK_INSERT] && !LastKeys[DIK_INSERT])
	{
		if ((field = AllocFileField()))
		{
			vec.v[X] = 0;
			vec.v[Y] = 0;
			vec.v[Z] = 512;
			RotVector(&CAM_MainCamera->WMatrix, &vec, &vec2);
			AddVector(&CAM_MainCamera->WPos, &vec2, &field->Pos);

			CopyMatrix(&IdentityMatrix, &field->Matrix);

			field->Size[X] = field->Size[Y] = field->Size[Z] = 128;
			field->Type = FILE_FIELD_TYPE_LINEAR;

			CopyVec(&UpVec, &field->Dir);
			field->Mag = 0;
			field->Damping = 0;
			field->RadStart = 64;
			field->RadEnd = 128;
			field->GradStart = 0;
			field->GradEnd = 0;

			CurrentField = field;
		}
	}

// quit now if no current file field

	if (!CurrentField) return;

// exit current edit?

	if (Keys[DIK_RETURN] && !LastKeys[DIK_RETURN])
	{
		CurrentField = NULL;
		return;
	}

// delete current field?

	if (Keys[DIK_DELETE] && !LastKeys[DIK_DELETE])
	{
		FreeFileField(CurrentField);
		CurrentField = NULL;
		return;
	}

// change axis?

	if (Keys[DIK_TAB] && !LastKeys[DIK_TAB])
	{
		if (Keys[DIK_LSHIFT]) FileFieldAxis--;
		else FileFieldAxis++;
		if (FileFieldAxis == -1) FileFieldAxis = 5;
		if (FileFieldAxis == 6) FileFieldAxis = 0;
	}

// change axis type?

	if (Keys[DIK_LALT] && !LastKeys[DIK_LALT])
		FileFieldAxisType ^= 1;

// change side?

	if (Keys[DIK_SPACE] && !LastKeys[DIK_SPACE])
		FileFieldSide = (FileFieldSide + 1) % 6;

// change rot type?

	if (Keys[DIK_NUMPADPERIOD] && !LastKeys[DIK_NUMPADPERIOD])
		FileFieldRotType ^= 1;

// change Type?

	if (Keys[DIK_NUMPADMINUS] && !LastKeys[DIK_NUMPADMINUS] && CurrentField->Type)
		CurrentField->Type--;
	if (Keys[DIK_NUMPADPLUS] && !LastKeys[DIK_NUMPADPLUS] && CurrentField->Type < FILE_FIELD_TYPE_MAX - 1)
		CurrentField->Type++;

// change picker

	if (Keys[DIK_UP] && !LastKeys[DIK_UP] && Picker)
		Picker--;

	if (Keys[DIK_DOWN] && !LastKeys[DIK_DOWN])
		Picker++;

	if (Picker > MaxPicker[CurrentField->Type] - 1)
		Picker = MaxPicker[CurrentField->Type] - 1;

// start enter?

	if (!EnterFlag && Keys[DIK_NUMPADENTER] && !LastKeys[DIK_NUMPADENTER])
	{
		EnterFlag = TRUE;
		EnterString[0] = 0;
	}

// resize spherical

	if (CurrentField->Type == FILE_FIELD_TYPE_SPHERICAL)
	{
		add = 0;
		if (Keys[DIK_NUMPADSTAR])
			add = 4 * TimeFactor;
		if (Keys[DIK_NUMPADSLASH])
			add = -4 * TimeFactor;

		if (Keys[DIK_LSHIFT])
		{
			CurrentField->RadEnd += add;
			if (CurrentField->RadEnd > 4096) CurrentField->RadEnd = 4096;
			if (CurrentField->RadEnd < CurrentField->RadStart) CurrentField->RadEnd = CurrentField->RadStart;
		}
		else
		{
			CurrentField->RadStart += add;
			if (CurrentField->RadStart < 32) CurrentField->RadStart = 32;
			if (CurrentField->RadStart > CurrentField->RadEnd) CurrentField->RadStart = CurrentField->RadEnd;
		}
	}

// resize hull

	else
	{
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
				CurrentField->Size[X] += add;
				if (CurrentField->Size[X] < 16) CurrentField->Size[X] = 16;
				CurrentField->Size[Y] += add;
				if (CurrentField->Size[Y] < 16) CurrentField->Size[Y] = 16;
				CurrentField->Size[Z] += add;
				if (CurrentField->Size[Z] < 16) CurrentField->Size[Z] = 16;
			}
			else
			{
				switch (FileFieldSide)
				{
					case 0:
						CurrentField->Size[X] += add;
						if (CurrentField->Size[X] < 16) CurrentField->Size[X] = 16;
						CurrentField->Pos.v[X] += CurrentField->Matrix.m[RX] * add;
						CurrentField->Pos.v[Y] += CurrentField->Matrix.m[RY] * add;
						CurrentField->Pos.v[Z] += CurrentField->Matrix.m[RZ] * add;
						break;
					case 1:
						CurrentField->Size[X] += add;
						if (CurrentField->Size[X] < 16) CurrentField->Size[X] = 16;
						CurrentField->Pos.v[X] -= CurrentField->Matrix.m[RX] * add;
						CurrentField->Pos.v[Y] -= CurrentField->Matrix.m[RY] * add;
						CurrentField->Pos.v[Z] -= CurrentField->Matrix.m[RZ] * add;
						break;
					case 2:
						CurrentField->Size[Y] += add;
						if (CurrentField->Size[Y] < 16) CurrentField->Size[Y] = 16;
						CurrentField->Pos.v[X] += CurrentField->Matrix.m[UX] * add;
						CurrentField->Pos.v[Y] += CurrentField->Matrix.m[UY] * add;
						CurrentField->Pos.v[Z] += CurrentField->Matrix.m[UZ] * add;
						break;
					case 3:
						CurrentField->Size[Y] += add;
						if (CurrentField->Size[Y] < 16) CurrentField->Size[Y] = 16;
						CurrentField->Pos.v[X] -= CurrentField->Matrix.m[UX] * add;
						CurrentField->Pos.v[Y] -= CurrentField->Matrix.m[UY] * add;
						CurrentField->Pos.v[Z] -= CurrentField->Matrix.m[UZ] * add;
						break;
					case 4:
						CurrentField->Size[Z] += add;
						if (CurrentField->Size[Z] < 16) CurrentField->Size[Z] = 16;
						CurrentField->Pos.v[X] += CurrentField->Matrix.m[LX] * add;
						CurrentField->Pos.v[Y] += CurrentField->Matrix.m[LY] * add;
						CurrentField->Pos.v[Z] += CurrentField->Matrix.m[LZ] * add;
						break;
					case 5:
						CurrentField->Size[Z] += add;
						if (CurrentField->Size[Z] < 16) CurrentField->Size[Z] = 16;
						CurrentField->Pos.v[X] -= CurrentField->Matrix.m[LX] * add;
						CurrentField->Pos.v[Y] -= CurrentField->Matrix.m[LY] * add;
						CurrentField->Pos.v[Z] -= CurrentField->Matrix.m[LZ] * add;
						break;
				}
			}
		}
	}

// move?

	if (MouseLeft)
	{
		RotTransVector(&ViewMatrix, &ViewTrans, &CurrentField->Pos, &vec);

		switch (FileFieldAxis)
		{
			case FILE_FIELD_AXIS_XY:
				vec.v[X] = MouseXrel * vec.v[Z] / RenderSettings.GeomPers + CameraEditXrel;
				vec.v[Y] = MouseYrel * vec.v[Z] / RenderSettings.GeomPers + CameraEditYrel;
				vec.v[Z] = CameraEditZrel;
				break;
			case FILE_FIELD_AXIS_XZ:
				vec.v[X] = MouseXrel * vec.v[Z] / RenderSettings.GeomPers + CameraEditXrel;
				vec.v[Y] = CameraEditYrel;
				vec.v[Z] = -MouseYrel * vec.v[Z] / RenderSettings.GeomPers + CameraEditZrel;
				break;
			case FILE_FIELD_AXIS_ZY:
				vec.v[X] = CameraEditXrel;
				vec.v[Y] = MouseYrel * vec.v[Z] / RenderSettings.GeomPers + CameraEditYrel;
				vec.v[Z] = MouseXrel * vec.v[Z] / RenderSettings.GeomPers + CameraEditZrel;
				break;
			case FILE_FIELD_AXIS_X:
				vec.v[X] = MouseXrel * vec.v[Z] / RenderSettings.GeomPers + CameraEditXrel;
				vec.v[Y] = CameraEditYrel;
				vec.v[Z] = CameraEditZrel;
				break;
			case FILE_FIELD_AXIS_Y:
				vec.v[X] = CameraEditXrel;
				vec.v[Y] = MouseYrel * vec.v[Z] / RenderSettings.GeomPers + CameraEditYrel;
				vec.v[Z] = CameraEditZrel;
				break;
			case FILE_FIELD_AXIS_Z:
				vec.v[X] = CameraEditXrel;
				vec.v[Y] = CameraEditYrel;
				vec.v[Z] = -MouseYrel * vec.v[Z] / RenderSettings.GeomPers + CameraEditZrel;
				break;
		}

		if (FileFieldAxisType == 1) 
		{
			SetVector(&vec2, vec.v[X], vec.v[Y], vec.v[Z]);
		}
		else
		{
			RotVector(&CAM_MainCamera->WMatrix, &vec, &vec2);
		}

		CurrentField->Pos.v[X] += vec2.v[X];
		CurrentField->Pos.v[Y] += vec2.v[Y];
		CurrentField->Pos.v[Z] += vec2.v[Z];
	}

// rotate?

	if (CurrentField->Type != FILE_FIELD_TYPE_SPHERICAL)
	{
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

		RotMatrixZYX(&mat, vec.v[X], vec.v[Y], vec.v[Z]);

// dir

		if (FileFieldRotType)
		{
			if (Keys[DIK_NUMPAD0]) CopyVec(&UpVec, &CurrentField->Dir);

			if (FileFieldAxisType)
			{
				RotVector(&mat, &CurrentField->Dir, &l);
				CopyVec(&l, &CurrentField->Dir);
				NormalizeVector(&CurrentField->Dir);
			}
			else if (vec.v[X] || vec.v[Y] || vec.v[Z])
			{
				RotVector(&ViewMatrix, &CurrentField->Dir, &l);
				RotVector(&mat, &l, &l2);
				RotVector(&CAM_MainCamera->WMatrix, &l2, &CurrentField->Dir);
				NormalizeVector(&CurrentField->Dir);
			}
		}

// hull

		else
		{
			if (Keys[DIK_NUMPAD0]) CopyMatrix(&IdentityMatrix, &CurrentField->Matrix);

			if (FileFieldAxisType)
			{
				MulMatrix(&mat, &CurrentField->Matrix, &mat2);
				CopyMatrix(&mat2, &CurrentField->Matrix);
				NormalizeMatrix(&CurrentField->Matrix);
			}
			else if (vec.v[X] || vec.v[Y] || vec.v[Z])
			{
				RotVector(&ViewMatrix, &CurrentField->Matrix.mv[X], &r);
				RotVector(&ViewMatrix, &CurrentField->Matrix.mv[Y], &u);
				RotVector(&ViewMatrix, &CurrentField->Matrix.mv[Z], &l);

				RotVector(&mat, &r, &r2);
				RotVector(&mat, &u, &u2);
				RotVector(&mat, &l, &l2);

				RotVector(&CAM_MainCamera->WMatrix, &r2, &CurrentField->Matrix.mv[X]);
				RotVector(&CAM_MainCamera->WMatrix, &u2, &CurrentField->Matrix.mv[Y]);
				RotVector(&CAM_MainCamera->WMatrix, &l2, &CurrentField->Matrix.mv[Z]);

				NormalizeMatrix(&CurrentField->Matrix);
			}
		}
	}
}

/////////////////
// load models //
/////////////////

void LoadFileFieldModels(void)
{
	LoadModel("edit\\field1.m", &FileFieldModel[0], -1, 1, LOADMODEL_FORCE_TPAGE, 100);
	LoadModel("edit\\field2.m", &FileFieldModel[1], -1, 1, LOADMODEL_FORCE_TPAGE, 100);
}

/////////////////
// free models //
/////////////////

void FreeFileFieldModels(void)
{
	FreeModel(&FileFieldModel[0], 1);
	FreeModel(&FileFieldModel[1], 1);
}
