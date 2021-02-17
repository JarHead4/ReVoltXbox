
#include "revolt.h"
#include "level.h"
#include "camera.h"
#include "input.h"
#include "geom.h"
#include "text.h"
#include "main.h"
#include "light.h"
#include "draw.h"
#include "visibox.h"
#include "instance.h"
#include "drawobj.h"
#include "newcoll.h"
#include "mirror.h"
#include "registry.h"

// globals

long InstanceNum;
INSTANCE *CurrentInstance;
INSTANCE Instances[MAX_INSTANCES];
long InstanceModelNum;
INSTANCE_MODELS *InstanceModels;

static char InstanceAxis = 0, InstanceAxisType = 0, InstanceRgbType = TRUE;
static unsigned char LastModel;

// misc edit text

static char *InstanceAxisNames[] = {
	"X Y",
	"X Z",
	"Z Y",
	"X",
	"Y",
	"Z",
};

static char *InstanceAxisTypeNames[] = {
	"Camera",
	"World",
};

////////////////////////////
// load / setup instances //
////////////////////////////

void LoadInstances(char *file)
{
	long i, j;
	FILE *fp;
	FILE_INSTANCE fin;

// open instance file

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

// loop thru all instances

	fread(&InstanceNum, sizeof(InstanceNum), 1, fp);

	for (i = 0 ; i < InstanceNum ; i++)
	{

// load one file instance

		fread(&fin, sizeof(fin), 1, fp);

		if (EditMode == EDIT_INSTANCES)
		{
			VecMulScalar(&fin.WorldPos, EditScale);
		}

// find it's model set

		fin.Name[MAX_INSTANCE_FILENAME - 1] = 0;
		for (j = 0 ; j < InstanceModelNum ; j++)
		{
			if (!strcmp(fin.Name, InstanceModels[j].Name))
			{
				Instances[i].Model = (char)j;
				break;
			}
		}

// ignore if can't find model set

		if (j == InstanceModelNum)
		{
			i--;
			InstanceNum--;
			continue;
		}

// setup misc from file

		Instances[i].r = fin.r;
		Instances[i].g = fin.g;
		Instances[i].b = fin.b;
		Instances[i].Priority = fin.Priority;
		Instances[i].Flag = fin.Flag;
		Instances[i].EnvRGB = fin.EnvRGB;
		Instances[i].LodBias = fin.LodBias;
		Instances[i].WorldPos = fin.WorldPos;
		Instances[i].WorldMatrix = fin.WorldMatrix;

// zero model rgb?

		if (!(Instances[i].Flag & INSTANCE_SET_MODEL_RGB))
		{
			Instances[i].r = 0;
			Instances[i].g = 0;
			Instances[i].b = 0;
			Instances[i].Flag |= INSTANCE_SET_MODEL_RGB;
		}

// set bounding boxes + rgb's

		SetInstanceBoundingBoxes(&Instances[i]);
		AllocOneInstanceRGB(&Instances[i]);

// set mirror flags

		if ((Instances[i].MirrorFlag = GetMirrorPlane(&Instances[i].WorldPos)))
			Instances[i].MirrorHeight = MirrorHeight;
	}

// close instance file

	fclose(fp);
}

////////////////////////
// save instance file //
////////////////////////

void SaveInstances(char *file)
{
	long i;
	FILE *fp;
	FILE_INSTANCE finst;
	INSTANCE *inst;
	char bak[256];

// backup old file

	memcpy(bak, file, strlen(file) - 3);
	wsprintf(bak + strlen(file) - 3, "fi-");
	remove(bak);
	rename(file, bak);

// open object file

	fp = fopen(file, "wb");
	if (!fp) return;

// write num

	fwrite(&InstanceNum, sizeof(InstanceNum), 1, fp);

// write out each instance

	inst = Instances;
	for (i = 0 ; i < InstanceNum ; i++, inst++)
	{

// set file instance

		memcpy(finst.Name, InstanceModels[inst->Model].Name, MAX_INSTANCE_FILENAME);

		finst.r = inst->r;
		finst.g = inst->g;
		finst.b = inst->b;
		finst.Priority = inst->Priority;
		finst.Flag = inst->Flag;
		finst.EnvRGB = inst->EnvRGB;
		finst.LodBias = inst->LodBias;
		finst.WorldPos = inst->WorldPos;
		finst.WorldMatrix = inst->WorldMatrix;

// write it

		fwrite(&finst, sizeof(finst), 1, fp);
	}

// close file

	Box("Saved Instance File:", file, MB_OK);
	fclose(fp);
}

//////////////////////
// edit an instance //
//////////////////////

void EditInstances(void)
{
	long i, flag;
	INSTANCE *inst, *ninst;
	VEC vec, vec2;
	MAT mat, mat2;
	VEC r, u, l, r2, u2, l2;
	MODEL_RGB *rgb;
	float xrad, yrad, sx, sy, ndist, dist;

// quit if not in edit mode

	if (CAM_MainCamera->Type != CAM_EDIT)
	{
		CurrentInstance = NULL;
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

// save instances?

	if (Keys[DIK_LCONTROL] && Keys[DIK_F4] && !LastKeys[DIK_F4])
	{
		SaveInstances(GetLevelFilename("fin", FILENAME_MAKE_BODY | FILENAME_GAME_SETTINGS));
	}

// get a current instance?

	if (!CurrentInstance && Keys[DIK_RETURN] && !LastKeys[DIK_RETURN])
	{
		ninst = NULL;
		ndist = 99999;

		inst = Instances;
		for (i = 0 ; i < InstanceNum ; i++, inst++)
		{
			RotTransVector(&ViewMatrix, &ViewTrans, &inst->WorldPos, &vec);

			if (vec.v[Z] < RenderSettings.NearClip || vec.v[Z] >= RenderSettings.FarClip) continue;

			sx = vec.v[X] * RenderSettings.GeomPers / vec.v[Z] + REAL_SCREEN_XHALF;
			sy = vec.v[Y] * RenderSettings.GeomPers / vec.v[Z] + REAL_SCREEN_YHALF;

			xrad = (InstanceModels[inst->Model].Models[0].Radius * RenderSettings.GeomPers) / vec.v[Z];
			yrad = (InstanceModels[inst->Model].Models[0].Radius * RenderSettings.GeomPers) / vec.v[Z];

			if (MouseXpos > sx - xrad && MouseXpos < sx + xrad && MouseYpos > sy - yrad && MouseYpos < sy + yrad)
			{
				dist = (float)sqrt((sx - MouseXpos) * (sx - MouseXpos) + (sy - MouseYpos) * (sy - MouseYpos));
				if (dist < ndist)
				{
					ninst = inst;
					ndist = dist;
				}
			}
		}
		if (ninst)
		{
			CurrentInstance = ninst;
			return;
		}
	}

// new instance?

	if (Keys[DIK_INSERT] && !LastKeys[DIK_INSERT])
	{
		if ((inst = AllocInstance()))
		{
			vec.v[X] = 0;
			vec.v[Y] = 0;
			vec.v[Z] = 256;
			RotVector(&CAM_MainCamera->WMatrix, &vec, &vec2);
			AddVector(&CAM_MainCamera->WPos, &vec2, &inst->WorldPos);

			RotMatrixZYX(&inst->WorldMatrix, 0, 0, 0);

			inst->r = 0;
			inst->g = 0;
			inst->b = 0;
			inst->EnvRGB = 0x808080;
			inst->Model = LastModel;
			inst->Priority = FALSE;
			inst->LodBias = 1024;
			inst->Flag = INSTANCE_SET_MODEL_RGB;

			SetInstanceBoundingBoxes(inst);
			AllocOneInstanceRGB(inst);

			CurrentInstance = inst;
		}
	}

// quit now if no current instance

	if (!CurrentInstance) return;

// set last mode

	LastModel = CurrentInstance->Model;

// set mirror flags

	if ((CurrentInstance->MirrorFlag = GetMirrorPlane(&CurrentInstance->WorldPos)))
		CurrentInstance->MirrorHeight = MirrorHeight;

// exit current instance edit?

	if (Keys[DIK_RETURN] && !LastKeys[DIK_RETURN])
	{
		CurrentInstance = NULL;
		return;
	}

// update bounding box and VisiMask

	SetInstanceBoundingBoxes(CurrentInstance);

// delete current instance?

	if (Keys[DIK_DELETE] && !LastKeys[DIK_DELETE])
	{
		FreeOneInstanceRGB(CurrentInstance);
		FreeInstance(CurrentInstance);
		CurrentInstance = NULL;
		return;
	}

// change axis?

	if (Keys[DIK_TAB] && !LastKeys[DIK_TAB])
	{
		if (Keys[DIK_LSHIFT]) InstanceAxis--;
		else InstanceAxis++;
		if (InstanceAxis == -1) InstanceAxis = 5;
		if (InstanceAxis == 6) InstanceAxis = 0;
	}

// change axis type?

	if (Keys[DIK_LALT] && !LastKeys[DIK_LALT])
		InstanceAxisType ^= 1;

// change model?

	if (Keys[DIK_LSHIFT]) LastKeys[DIK_NUMPADMINUS] = LastKeys[DIK_NUMPADPLUS] = 0;

	if (Keys[DIK_NUMPADMINUS] && !LastKeys[DIK_NUMPADMINUS] && CurrentInstance->Model)
	{
		FreeOneInstanceRGB(CurrentInstance);
		CurrentInstance->Model--;
		AllocOneInstanceRGB(CurrentInstance);
	}

	if (Keys[DIK_NUMPADPLUS] && !LastKeys[DIK_NUMPADPLUS] && CurrentInstance->Model < InstanceModelNum - 1)
	{
		FreeOneInstanceRGB(CurrentInstance);
		CurrentInstance->Model++;
		AllocOneInstanceRGB(CurrentInstance);
	}

// change priority?

	if (Keys[DIK_NUMPADSLASH] && !LastKeys[DIK_NUMPADSLASH]) CurrentInstance->Priority = FALSE;
	if (Keys[DIK_NUMPADSTAR] && !LastKeys[DIK_NUMPADSTAR]) CurrentInstance->Priority = TRUE;

// toggle env?

	if (Keys[DIK_NUMPADENTER] && !LastKeys[DIK_NUMPADENTER]) CurrentInstance->Flag ^= INSTANCE_ENV;

// toggle hide?

	if (Keys[DIK_H] && !LastKeys[DIK_H])
	{
		FreeOneInstanceRGB(CurrentInstance);
		CurrentInstance->Flag ^= INSTANCE_HIDE;
		AllocOneInstanceRGB(CurrentInstance);
	}

// toggle mirror?

	if (Keys[DIK_M] && !LastKeys[DIK_M]) CurrentInstance->Flag ^= INSTANCE_NO_MIRROR;

// toggle rgb type?

	if (Keys[DIK_SPACE] && !LastKeys[DIK_SPACE]) InstanceRgbType ^= TRUE;

// toggle lit

	if (Keys[DIK_L] && !LastKeys[DIK_L]) CurrentInstance->Flag ^= INSTANCE_NO_FILE_LIGHTS;

// change env rgb?

	if (!InstanceRgbType)
	{
		rgb = (MODEL_RGB*)&CurrentInstance->EnvRGB;

		if (Keys[DIK_LSHIFT]) LastKeys[DIK_P] = LastKeys[DIK_SEMICOLON] = LastKeys[DIK_LBRACKET] = LastKeys[DIK_APOSTROPHE] = LastKeys[DIK_RBRACKET] = LastKeys[DIK_BACKSLASH] = 0;

		if (Keys[DIK_P] && !LastKeys[DIK_P] && rgb->r < 255) rgb->r++;
		if (Keys[DIK_SEMICOLON] && !LastKeys[DIK_SEMICOLON] && rgb->r) rgb->r--;

		if (Keys[DIK_LBRACKET] && !LastKeys[DIK_LBRACKET] && rgb->g < 255) rgb->g++;
		if (Keys[DIK_APOSTROPHE] && !LastKeys[DIK_APOSTROPHE] && rgb->g) rgb->g--;

		if (Keys[DIK_RBRACKET] && !LastKeys[DIK_RBRACKET] && rgb->b < 255) rgb->b++;
		if (Keys[DIK_BACKSLASH] && !LastKeys[DIK_BACKSLASH] && rgb->b) rgb->b--;
	}

// change model rgb?

	if (InstanceRgbType)
	{
		flag = FALSE;

		if (Keys[DIK_LSHIFT]) LastKeys[DIK_P] = LastKeys[DIK_SEMICOLON] = LastKeys[DIK_LBRACKET] = LastKeys[DIK_APOSTROPHE] = LastKeys[DIK_RBRACKET] = LastKeys[DIK_BACKSLASH] = 0;

		if (Keys[DIK_P] && !LastKeys[DIK_P] && CurrentInstance->r < 127) CurrentInstance->r++, flag = TRUE;
		if (Keys[DIK_SEMICOLON] && !LastKeys[DIK_SEMICOLON] && CurrentInstance->r > -128) CurrentInstance->r--, flag = TRUE;

		if (Keys[DIK_LBRACKET] && !LastKeys[DIK_LBRACKET] && CurrentInstance->g < 127) CurrentInstance->g++, flag = TRUE;
		if (Keys[DIK_APOSTROPHE] && !LastKeys[DIK_APOSTROPHE] && CurrentInstance->g > -128) CurrentInstance->g--, flag = TRUE;

		if (Keys[DIK_RBRACKET] && !LastKeys[DIK_RBRACKET] && CurrentInstance->b < 127) CurrentInstance->b++, flag = TRUE;
		if (Keys[DIK_BACKSLASH] && !LastKeys[DIK_BACKSLASH] && CurrentInstance->b > -128) CurrentInstance->b--, flag = TRUE;

		if (flag)
		{
			FreeOneInstanceRGB(CurrentInstance);
			AllocOneInstanceRGB(CurrentInstance);
		}
	}

// change LOD bias?

	if (Keys[DIK_MINUS]) CurrentInstance->LodBias -= TimeFactor * 4;
	if (Keys[DIK_EQUALS]) CurrentInstance->LodBias += TimeFactor * 4;

	if (CurrentInstance->LodBias < 64) CurrentInstance->LodBias = 64;
	if (CurrentInstance->LodBias > 8192) CurrentInstance->LodBias = 8192;

// copy instance?

	if (MouseLeft && !MouseLastLeft && Keys[DIK_LSHIFT])
	{
		if ((inst = AllocInstance()))
		{
			memcpy(inst, CurrentInstance, sizeof(INSTANCE));
			CurrentInstance = inst;
			return;
		}
	}

// move instance?

	if (MouseLeft)
	{
		RotTransVector(&ViewMatrix, &ViewTrans, &CurrentInstance->WorldPos, &vec);

		switch (InstanceAxis)
		{
			case INSTANCE_AXIS_XY:
				vec.v[X] = MouseXrel * vec.v[Z] / RenderSettings.GeomPers + CameraEditXrel;
				vec.v[Y] = MouseYrel * vec.v[Z] / RenderSettings.GeomPers + CameraEditYrel;
				vec.v[Z] = CameraEditZrel;
				break;
			case INSTANCE_AXIS_XZ:
				vec.v[X] = MouseXrel * vec.v[Z] / RenderSettings.GeomPers + CameraEditXrel;
				vec.v[Y] = CameraEditYrel;
				vec.v[Z] = -MouseYrel * vec.v[Z] / RenderSettings.GeomPers + CameraEditZrel;
				break;
			case INSTANCE_AXIS_ZY:
				vec.v[X] = CameraEditXrel;
				vec.v[Y] = MouseYrel * vec.v[Z] / RenderSettings.GeomPers + CameraEditYrel;
				vec.v[Z] = MouseXrel * vec.v[Z] / RenderSettings.GeomPers + CameraEditZrel;
				break;
			case INSTANCE_AXIS_X:
				vec.v[X] = MouseXrel * vec.v[Z] / RenderSettings.GeomPers + CameraEditXrel;
				vec.v[Y] = CameraEditYrel;
				vec.v[Z] = CameraEditZrel;
				break;
			case INSTANCE_AXIS_Y:
				vec.v[X] = CameraEditXrel;
				vec.v[Y] = MouseYrel * vec.v[Z] / RenderSettings.GeomPers + CameraEditYrel;
				vec.v[Z] = CameraEditZrel;
				break;
			case INSTANCE_AXIS_Z:
				vec.v[X] = CameraEditXrel;
				vec.v[Y] = CameraEditYrel;
				vec.v[Z] = -MouseYrel * vec.v[Z] / RenderSettings.GeomPers + CameraEditZrel;
				break;
		}

		if (InstanceAxisType == 1) 
		{
			SetVector(&vec2, vec.v[X], vec.v[Y], vec.v[Z]);
		}
		else
		{
			RotVector(&CAM_MainCamera->WMatrix, &vec, &vec2);
		}

		CurrentInstance->WorldPos.v[X] += vec2.v[X];
		CurrentInstance->WorldPos.v[Y] += vec2.v[Y];
		CurrentInstance->WorldPos.v[Z] += vec2.v[Z];
	}

// rotate instance?

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

	if (Keys[DIK_NUMPAD0]) CopyMatrix(&IdentityMatrix, &CurrentInstance->WorldMatrix);

	RotMatrixZYX(&mat, vec.v[X], vec.v[Y], vec.v[Z]);

	if (InstanceAxisType)
	{
		MulMatrix(&mat, &CurrentInstance->WorldMatrix, &mat2);
		CopyMatrix(&mat2, &CurrentInstance->WorldMatrix);
		NormalizeMatrix(&CurrentInstance->WorldMatrix);
	}
	else if (vec.v[X] || vec.v[Y] || vec.v[Z])
	{
		RotVector(&ViewMatrix, &CurrentInstance->WorldMatrix.mv[X], &r);
		RotVector(&ViewMatrix, &CurrentInstance->WorldMatrix.mv[Y], &u);
		RotVector(&ViewMatrix, &CurrentInstance->WorldMatrix.mv[Z], &l);

		RotVector(&mat, &r, &r2);
		RotVector(&mat, &u, &u2);
		RotVector(&mat, &l, &l2);

		RotVector(&CAM_MainCamera->WMatrix, &r2, &CurrentInstance->WorldMatrix.mv[X]);
		RotVector(&CAM_MainCamera->WMatrix, &u2, &CurrentInstance->WorldMatrix.mv[Y]);
		RotVector(&CAM_MainCamera->WMatrix, &l2, &CurrentInstance->WorldMatrix.mv[Z]);

		NormalizeMatrix(&CurrentInstance->WorldMatrix);
	}
}

///////////////////////
// alloc an instance //
///////////////////////

INSTANCE *AllocInstance(void)
{

// full?

	if (InstanceNum >= MAX_INSTANCES)
		return NULL;

// inc counter, return slot

	return &Instances[InstanceNum++];
}

//////////////////////
// free an instance //
//////////////////////

void FreeInstance(INSTANCE *inst)
{
	long idx, i;

// find index into list

	idx = (long)(inst - Instances);

// copy all higher instances down one

	for (i = idx ; i < InstanceNum - 1; i++)
	{
		Instances[i] = Instances[i + 1];
	}

// dec num

	InstanceNum--;
}

////////////////////////
// draw all instances //
////////////////////////

void DrawInstances(void)
{
	short flag;
	long i, lod, visflag, lit;
	float z, flod;
	MODEL *model;
	INSTANCE *inst;
	POLY_RGB *savergb;

// loop thru all instances

	inst = Instances;
	for (i = 0 ; i < InstanceNum ; i++, inst++)
	{

// skip if mirror hide and mirrors off

		if ((inst->Flag & INSTANCE_HIDE) && !RenderSettings.Mirror)
			continue;

// skip if turned off

		if (!RenderSettings.Instance && !inst->Priority && !(inst->Flag & INSTANCE_HIDE))
			continue;

// visicube reject?

		if (inst->VisiMask & CamVisiMask)
			continue;

// reset draw flag

		flag = MODEL_PLAIN;

// skip if offscreen

		model = InstanceModels[inst->Model].Models;

		visflag = TestSphereToFrustum(&inst->WorldPos, model->Radius, &z);
		if (visflag == SPHERE_OUT) continue;
		if (visflag == SPHERE_IN) flag |= MODEL_DONOTCLIP;

// calc lod

		flod = z / inst->LodBias - 1;
		if (flod < 0) flod = 0;
		FTOL(flod, lod);
		if (lod > InstanceModels[inst->Model].Count - 1) lod = InstanceModels[inst->Model].Count - 1;
		
// calc model

		model += lod;

// re-point model rgb

		savergb = model->PolyRGB;
		model->PolyRGB = inst->rgb[lod];

// in fog?

		if (z + model->Radius > RenderSettings.FogStart && DxState.Fog)
		{
			ModelVertFog = (inst->WorldPos.v[1] - RenderSettings.VertFogStart) * RenderSettings.VertFogMul;
			if (ModelVertFog < 0) ModelVertFog = 0;
			if (ModelVertFog > 255) ModelVertFog = 255;

			flag |= MODEL_FOG;
			FOG_ON();
		}

// in light?

		if (!(inst->Flag & INSTANCE_HIDE))
		{
			if (EditMode == EDIT_LIGHTS || EditMode == EDIT_INSTANCES)
				lit = CheckInstanceLightEdit(inst, model->Radius);
			else
				lit = CheckInstanceLight(inst, model->Radius);

			if (lit)
			{
				flag |= MODEL_LIT;
				AddModelLight(model, &inst->WorldPos, &inst->WorldMatrix);
			}
		}

// env map?

		if (inst->Flag & INSTANCE_ENV)
		{
			flag |= MODEL_ENV;
			SetEnvStatic(&inst->WorldPos, &inst->WorldMatrix, inst->EnvRGB, 0.0f, 0.0f, 1.0f);
		}

// reflect?

		if (!(inst->Flag & INSTANCE_NO_MIRROR) && inst->MirrorFlag && RenderSettings.Mirror)
		{
			if (ViewCameraPos.v[Y] < inst->MirrorHeight)
			{
				MirrorHeight = inst->MirrorHeight;
				flag |= MODEL_MIRROR;
			}
		}

// mesh fx?

		CheckModelMeshFx(model, &inst->WorldMatrix, &inst->WorldPos, &flag);

// draw

		DrawModel(model, &inst->WorldMatrix, &inst->WorldPos, flag);

// reset render states?

		if (flag & MODEL_FOG)
			FOG_OFF();

// restore model rgb

		model->PolyRGB = savergb;
	}
}

/////////////////////////////////
// display info on an instance //
/////////////////////////////////

void DisplayInstanceInfo(INSTANCE *inst)
{
	char buf[128];

// model

	DumpText(450, 0, 8, 16, 0xffff00, InstanceModels[inst->Model].Name);

// priority

	wsprintf(buf, "High priority: %s", inst->Priority ? "Yes" : "No");
	DumpText(450, 24, 8, 16, 0x00ffff, buf);

// env

	wsprintf(buf, "Env %s", inst->Flag & INSTANCE_ENV ? "On" : "Off");
	DumpText(450, 48, 8, 16, 0xff00ff, buf);

// LOD bias

	wsprintf(buf, "LOD Bias %d", (long)inst->LodBias);
	DumpText(450, 72, 8, 16, 0xffff00, buf);

// env rgb

	wsprintf(buf, "Env RGB %d %d %d", ((MODEL_RGB*)&inst->EnvRGB)->r, ((MODEL_RGB*)&inst->EnvRGB)->g, ((MODEL_RGB*)&inst->EnvRGB)->b);
	DumpText(450, 96, 8, 16, 0x00ff00, buf);

// model rgb

	wsprintf(buf, "Model RGB %d %d %d", inst->r, inst->g, inst->b);
	DumpText(450, 120, 8, 16, 0x00ffff, buf);

// axis

	wsprintf(buf, "Axis %s - %s", InstanceAxisNames[InstanceAxis], InstanceAxisTypeNames[InstanceAxisType]);
	DumpText(450, 144, 8, 16, 0xff0000, buf);

// rgb type

	wsprintf(buf, "RGB Type: %s", InstanceRgbType? "Model" : "Env");
	DumpText(450, 168, 8, 16, 0xffff00, buf);

// hide

	wsprintf(buf, "Mirror Hide: %s", inst->Flag & INSTANCE_HIDE? "Yes" : "No");
	DumpText(450, 192, 8, 16, 0x00ff00, buf);

// mirror

	wsprintf(buf, "Mirror: %s", inst->Flag & INSTANCE_NO_MIRROR? "No" : "Yes");
	DumpText(450, 216, 8, 16, 0x00ffff, buf);

// lit

	wsprintf(buf, "Fixed lights: %s", inst->Flag & INSTANCE_NO_FILE_LIGHTS? "No" : "Yes");
	DumpText(450, 240, 8, 16, 0xff0000, buf);

// draw axis

	TEXTUREFILTER_ON();
	ZBUFFER_ON();
	ZWRITE_ON();

	if (InstanceAxisType)
		DrawAxis(&IdentityMatrix, &CurrentInstance->WorldPos);
	else
		DrawAxis(&CAM_MainCamera->WMatrix, &CurrentInstance->WorldPos);

	BeginTextState();
}

//////////////////////////
// load instance models //
//////////////////////////

void LoadInstanceModels(void)
{
	long i;
	WIN32_FIND_DATA data;
	HANDLE handle;
	FILE *fp;
	char buf[256];
	char names[MAX_INSTANCE_MODELS][16];

// zero num / ptr

	InstanceModelNum = 0;
	InstanceModels = NULL;

// get first prm

	wsprintf(buf, "levels\\%s\\*.prm", LevelInf[GameSettings.Level].Dir);
	handle = FindFirstFile(buf, &data);
	if (handle == INVALID_HANDLE_VALUE)
		return;

// loop thru each prm

	while (TRUE)
	{

// add to list?

		if (!(data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && InstanceModelNum < MAX_INSTANCE_MODELS)
		{
			for (i = 0 ; i < (long)strlen(data.cFileName) - 4 ; i++) data.cFileName[i] = toupper(data.cFileName[i]);
			data.cFileName[i] = 0;
			memcpy(names[InstanceModelNum], data.cFileName, 16);
			InstanceModelNum++;
		}

// get next

		if (!FindNextFile(handle, &data))
			break;
	}

// close search handle

	FindClose(handle);

// alloc ram for instance models structure

	InstanceModels = (INSTANCE_MODELS*)malloc(sizeof(INSTANCE_MODELS) * InstanceModelNum);
	if (!InstanceModels)
	{
		Box("ERROR", "Can't alloc memory for instance models!", MB_OK);
		QuitGame = TRUE;
		return;
	}

// fill instance model structure

	for (i = 0 ; i < InstanceModelNum ; i++)
	{
		memcpy(InstanceModels[i].Name, names[i], MAX_INSTANCE_FILENAME);
		InstanceModels[i].Name[MAX_INSTANCE_FILENAME - 1] = 0;
		wsprintf(buf, "levels\\%s\\%s.prm", LevelInf[GameSettings.Level].Dir, names[i]);
		InstanceModels[i].Count = LoadModel(buf, InstanceModels[i].Models, 0, MAX_INSTANCE_LOD, LOADMODEL_OFFSET_TPAGE, LevelInf[GameSettings.Level].InstanceRGBper);
		
		// Load collision skin if it exists
		wsprintf(buf, "levels\\%s\\%s.ncp", LevelInf[GameSettings.Level].Dir, names[i]);
		if ((fp = fopen(buf, "rb")) != NULL) {
			InstanceModels[i].CollPoly = LoadNewCollPolys(fp, &InstanceModels[i].NCollPolys);
			fclose(fp);
		} else {
			InstanceModels[i].CollPoly = NULL;
			InstanceModels[i].NCollPolys = 0;
		}

	}
}

//////////////////////////
// free instance models //
//////////////////////////

void FreeInstanceModels(void)
{
	long i;

	for (i = 0 ; i < InstanceModelNum ; i++)
	{
		DestroyCollPolys(InstanceModels[i].CollPoly);
		InstanceModels[i].CollPoly = NULL;
		InstanceModels[i].NCollPolys = 0;
		FreeModel(InstanceModels[i].Models, InstanceModels[i].Count);
	}

	free(InstanceModels);
}

////////////////////////
// free instance rgbs //
////////////////////////

void FreeInstanceRGBs(void)
{
	long i;

	for (i = 0 ; i < InstanceNum ; i++)
	{
		FreeOneInstanceRGB(&Instances[i]);
	}
}

//////////////////////////////
// free one instances RGB's //
//////////////////////////////

void FreeOneInstanceRGB(INSTANCE *inst)
{
	long i;

	for (i = 0 ; i < InstanceModels[inst->Model].Count ; i++)
	{
		free(inst->rgb[i]);
	}
}

/////////////////////////////
// alloc one instances rgb //
/////////////////////////////

void AllocOneInstanceRGB(INSTANCE *inst)
{
	long i, j, k, col;
	MODEL *model;

// step through each LOD

	for (i = 0 ; i < InstanceModels[inst->Model].Count ; i++)
	{

// alloc RGB space

		model = &InstanceModels[inst->Model].Models[i];
		inst->rgb[i] = (POLY_RGB*)malloc(sizeof(POLY_RGB) * model->PolyNum);
		if (!inst->rgb[i])
		{
			Box("ERROR", "Can't alloc memory for Instance RGB", MB_OK);
			QuitGame = TRUE;
			return;
		}

// get model RGB

		for (j = 0 ; j < model->PolyNum ; j++)
		{
			for (k = 0 ; k < 4 ; k++)
			{
				if (inst->Flag & INSTANCE_HIDE)
				{
					*(long*)&inst->rgb[i][j].rgb[k] = LevelInf[GameSettings.Level].FogColor;
				}
				else
				{
					inst->rgb[i][j].rgb[k].a = model->PolyRGB[j].rgb[k].a;

					col = model->PolyRGB[j].rgb[k].r * (inst->r + 128) / 128;
					if (col > 255) col = 255;
					inst->rgb[i][j].rgb[k].r = (unsigned char)col;

					col = model->PolyRGB[j].rgb[k].g * (inst->g + 128) / 128;
					if (col > 255) col = 255;
					inst->rgb[i][j].rgb[k].g = (unsigned char)col;

					col = model->PolyRGB[j].rgb[k].b * (inst->b + 128) / 128;
					if (col > 255) col = 255;
					inst->rgb[i][j].rgb[k].b = (unsigned char)col;
				}
			}
		}
	}
}

////////////////////////////////////////////
// set instance bounding boxes + VisiMask //
////////////////////////////////////////////

void SetInstanceBoundingBoxes(INSTANCE *inst)
{
	long j;
	MODEL *model;
	VEC vec;

// get model

	model = InstanceModels[inst->Model].Models;

// transform all model verts to find bounding box

	inst->Box.Xmin = inst->Box.Ymin = inst->Box.Zmin = 999999;
	inst->Box.Xmax = inst->Box.Ymax = inst->Box.Zmax = -999999;

	for (j = 0 ; j < model->VertNum ; j++)
	{
		RotTransVector(&inst->WorldMatrix, &inst->WorldPos, (VEC*)&model->VertPtr[j].x, &vec);

		if (vec.v[X] < inst->Box.Xmin) inst->Box.Xmin = vec.v[X];
		if (vec.v[X] > inst->Box.Xmax) inst->Box.Xmax = vec.v[X];
		if (vec.v[Y] < inst->Box.Ymin) inst->Box.Ymin = vec.v[Y];
		if (vec.v[Y] > inst->Box.Ymax) inst->Box.Ymax = vec.v[Y];
		if (vec.v[Z] < inst->Box.Zmin) inst->Box.Zmin = vec.v[Z];
		if (vec.v[Z] > inst->Box.Zmax) inst->Box.Zmax = vec.v[Z];
	}

// set visi mask

	inst->VisiMask = SetObjectVisiMask(&inst->Box);
}



/////////////////////////////////////////////////////////////////////
//
// BuildInstanceCollPolys:
//
/////////////////////////////////////////////////////////////////////

void BuildInstanceCollPolys()
{
	int			iInst, iPoly, nInstCollPolys;
	INSTANCE	*instance;
	INSTANCE_MODELS *instModel;
	NEWCOLLPOLY *worldPoly, *instModelPoly;

	COL_NInstanceCollPolys = 0;
	// Count number of collision polys needed for instances
	for (iInst = 0; iInst < InstanceNum; iInst++) {
		instance = &Instances[iInst];

		if (!instance->Priority && !RenderSettings.Instance)
			continue;	// skip if turned off

		COL_NInstanceCollPolys += InstanceModels[instance->Model].NCollPolys;

	}

	// Allocate space for the instance polys
	COL_InstanceCollPoly = CreateCollPolys(COL_NInstanceCollPolys);

	// Transform models collision polys into world coords and store in array
	nInstCollPolys = 0;
	for (iInst = 0; iInst < InstanceNum; iInst++) {
		instance = &Instances[iInst];

		if (!instance->Priority && !RenderSettings.Instance)
			continue;	// skip if turned off

		instModel = &InstanceModels[instance->Model];

		// Store pointer for the world collision polys for this instance
		instance->CollPoly = &COL_InstanceCollPoly[nInstCollPolys];
		instance->NCollPolys = instModel->NCollPolys;


		// Do the actual copying and transformation
		for (iPoly = 0; iPoly < instModel->NCollPolys; iPoly++) {
			worldPoly = &COL_InstanceCollPoly[nInstCollPolys++];
			instModelPoly = &instModel->CollPoly[iPoly];

			worldPoly->Type = instModelPoly->Type;
			worldPoly->Material = instModelPoly->Material;
			RotTransPlane(&instModelPoly->Plane, &instance->WorldMatrix, &instance->WorldPos, &worldPoly->Plane);
			RotTransPlane(&instModelPoly->EdgePlane[0], &instance->WorldMatrix, &instance->WorldPos, &worldPoly->EdgePlane[0]);
			RotTransPlane(&instModelPoly->EdgePlane[1], &instance->WorldMatrix, &instance->WorldPos, &worldPoly->EdgePlane[1]);
			RotTransPlane(&instModelPoly->EdgePlane[2], &instance->WorldMatrix, &instance->WorldPos, &worldPoly->EdgePlane[2]);
			if (instModelPoly->Type == QUAD) {
				RotTransPlane(&instModelPoly->EdgePlane[3], &instance->WorldMatrix, &instance->WorldPos, &worldPoly->EdgePlane[3]);
			}

			RotTransBBox(&instModelPoly->BBox, &instance->WorldMatrix, &instance->WorldPos, &worldPoly->BBox);

		}
	}

}
