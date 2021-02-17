/*********************************************************************************************
 *
 * level.cpp
 *
 * Re-Volt (PC) Copyright (c) Probe Entertainment 1998
 * 
 * Contents:
 *			Level specific loading code 
 *
 *********************************************************************************************
 *
 * 28/02/98 Matt Taylor
 *  File inception.
 *
 *********************************************************************************************/

#include "revolt.h"
#ifndef _PSX
#include "main.h"
#include "dx.h"
#include "geom.h"
#include "particle.h"
#include "texture.h"
#include "model.h"
#include "aerial.h"
#include "NewColl.h"
#include "Body.h"
#include "car.h"
#include "camera.h"
#include "light.h"
#include "world.h"
#include "draw.h"
#include "visibox.h"
#include "texture.h"
#include "ctrlread.h"
#include "object.h"
#include "Control.h"
#include "level.h"
#include "player.h"
#include "editobj.h"
#include "instance.h"
#include "editai.h"
#include "editzone.h"
#include "aizone.h"
#include "registry.h"
#include "field.h"
#include "Ghost.h"
#include "mirror.h"
#include "timing.h"
#include "trigger.h"
#include "edittrig.h"
#include "editcam.h"
#include "edfield.h"
#include "sfx.h"
#include "aizone.h"
#include "ainode.h"
#include "input.h"
#include "obj_init.h"
#include "ai.h"
#include "edportal.h"
#endif

#ifdef _PC
#include "spark.h"
#endif
//
// Static function prototypes
//

static void s_LoadTrackData(void);
static void LoadStaticLevelModels(void);

//
// Global variables
//

LEVELINFO	*LevelInf;								// level info pointer
VEC		LEV_StartPos;
REAL		LEV_StartRot; 
long		LEV_StartGrid;
VEC		*LEV_LevelFieldPos = NULL;
MAT		*LEV_LevelFieldMat = NULL;

static char LevelFilenameBuffer[128];
//
// Global function prototypes
//

void LEV_InitLevel(void);
void LEV_EndLevel(void);

//////////////////////////////
// load static level models //
//////////////////////////////

static void LoadStaticLevelModels(void)
{
	struct renderflags rflag;

	LoadOneLevelModel(LEVEL_MODEL_PICKUP, FALSE, rflag, 0);
	LoadOneLevelModel(LEVEL_MODEL_FIREWORK, FALSE, rflag, TPAGE_FX1);
	LoadOneLevelModel(LEVEL_MODEL_WATERBOMB, FALSE, rflag, 0);
	LoadOneLevelModel(LEVEL_MODEL_CHROMEBALL, FALSE, rflag, 0);
	LoadOneLevelModel(LEVEL_MODEL_BOMBBALL, FALSE, rflag, 0);
}

//////////////////////////////////////////////////////////////////////////////////////
// Loads and initialises level specific data (ie. data required for game play only) //
//////////////////////////////////////////////////////////////////////////////////////

void LEV_InitLevel(void)
{

	// init object system
	if (!OBJ_InitObjSys())
	{
		Box(NULL, "Can't initialise object system!", MB_OK);
		QuitGame = TRUE;
		return;
	}

	// init player system
	PLR_InitPlayers();

	// init level model system
	InitLevelModels();
	LoadStaticLevelModels();

	// Set near / far + fog
	SetNearFar(48.0f, LevelInf[GameSettings.Level].FarClip);
	SetFogVars(LevelInf[GameSettings.Level].FogStart, LevelInf[GameSettings.Level].VertFogStart, LevelInf[GameSettings.Level].VertFogEnd);
	FOG_COLOR(LevelInf[GameSettings.Level].FogColor);
	SetBackgroundColor(LevelInf[GameSettings.Level].FogColor);

	// Init spark engine
	InitSparks();

	// init poly buckets
	InitPolyBuckets();

	// init cameras
	InitCameras();
	CAM_MainCamera = AddCamera(0, 0, 0, 0, CAMERA_FLAG_PRIMARY);

	// init skidmarks
	ClearSkids();

	// setup track
	s_LoadTrackData();

	// init jump spark offsets
	InitJumpSparkOffsets();

	// Add gravity
	FLD_GravityField = AddLinearField(FIELD_PARENT_NONE, FIELD_PRIORITY_MAX, &ZeroVector, &Identity, &FLD_GlobalBBox, &FLD_GlobalSize, &DownVec, FLD_Gravity, ZERO);

	// misc littlies
	GameSettings.Paws = FALSE;
}

////////////////////////////////////////////////////
// Free up all of the resources used by the level //
////////////////////////////////////////////////////

void LEV_EndLevel(void)
{
	// kill world collision
	DestroyCollGrids();
	DestroyCollPolys(COL_WorldCollPoly);
	COL_WorldCollPoly = NULL;
	COL_NWorldCollPolys = 0;
	DestroyCollPolys(COL_InstanceCollPoly);
	COL_InstanceCollPoly = NULL;
	COL_NInstanceCollPolys = 0;

	// Free any force field stuff
	FreeForceFields();

	// kill textures
	FreeTextures();

	// kill world mesh
	FreeWorld();

	// kill car info
//	DestroyCarInfo();
//	NCarTypes = 0;

	// kill poly buckets
	KillPolyBuckets();

	// free mirror planes
	FreeMirrorPlanes();

	// free instance stuff
	FreeInstanceRGBs();
	FreeInstanceModels();

	// free sfx
	FreeSfx();

	// free ai nodes
	FreeAiNodes();

	// free ai zones
	FreeAiZones();

	// free triggers
	FreeTriggers();

	// save best track times
	SaveTrackTimes(&LevelInf[GameSettings.Level]);

	// free edit stuff
	if (AI_Testing)
	{
		FreeEditAiNodeModels();							// !MT! Temp moved out during AI testing
	}

	if (EditMode == EDIT_VISIBOXES)
	{
		if (!AI_Testing) FreeEditAiNodeModels();
	}

	if (EditMode == EDIT_LIGHTS)
	{
		FreeEditLightModels();
	}

	if (EditMode == EDIT_OBJECTS)
	{
		KillFileObjects();
		FreeFileObjectModels();
	}

	if (EditMode == EDIT_AINODES)
	{
		KillEditAiNodes();
		if (!AI_Testing) FreeEditAiNodeModels();
	}

	if (EditMode == EDIT_ZONES)
	{
		KillFileZones();
		if (!AI_Testing) FreeEditAiNodeModels();
	}

	if (EditMode == EDIT_TRIGGERS)
	{
		KillFileTriggers();
	}

	if (EditMode == EDIT_CAM)
	{
		KillEditCamNodes();
		FreeEditCamNodeModels();
	}

	if (EditMode == EDIT_FIELDS)
	{
		KillFileFields();
		FreeFileFieldModels();
	}

	if (EditMode == EDIT_PORTALS)
	{
		KillEditPortals();
	}

	// free level models
	FreeLevelModels();

	// Kill player system
	PLR_KillAllPlayers();

	// kill object system
	OBJ_KillObjSys();

	// Kill grid buffers
	GRD_FreeGrids();
}

//////////////////////////
// Load misc track data //
//////////////////////////

static void s_LoadTrackData(void)
{
	char	ii;
	char	buf[128];
	FILE	*fp;

	// Initialise the field array (must be done before any objects with fields are loaded or initialised)
	InitFields();

	// set start pos / rot /grid
	if (GameSettings.Reversed)
	{
		LEV_StartPos = LevelInf[GameSettings.Level].ReverseStartPos;
		LEV_StartRot = LevelInf[GameSettings.Level].ReverseStartRot;
		LEV_StartGrid = LevelInf[GameSettings.Level].ReverseStartGrid;
	}
	else
	{
		LEV_StartPos = LevelInf[GameSettings.Level].NormalStartPos;
		LEV_StartRot = LevelInf[GameSettings.Level].NormalStartRot;
		LEV_StartGrid = LevelInf[GameSettings.Level].NormalStartGrid;
	}

	// Load misc textures
	LoadMipTexture("gfx\\font1.bmp", TPAGE_FONT, 256, 256, 0, 1);
	LoadMipTexture("gfx\\font2.bmp", TPAGE_BIGFONT, 256, 256, 0, 1);

	LoadTextureClever(LevelInf[GameSettings.Level].EnvStill, TPAGE_ENVSTILL, 256, 256, 0, FxTextureSet, TRUE);
	LoadTextureClever(LevelInf[GameSettings.Level].EnvRoll, TPAGE_ENVROLL, 256, 256, 0, FxTextureSet, TRUE);
	LoadTextureClever("gfx\\shadow.bmp", TPAGE_SHADOW, 256, 256, 0, FxTextureSet, TRUE);

	LoadMipTexture("gfx\\fxpage1.bmp", TPAGE_FX1, 256, 256, 0, 1);
	LoadMipTexture("gfx\\fxpage2.bmp", TPAGE_FX2, 256, 256, 0, 1);
	LoadMipTexture("gfx\\fxpage3.bmp", TPAGE_FX3, 256, 256, 0, 1);

	// Load all world textures
	for (ii = 0 ; ii < TPAGE_WORLD_NUM ; ii++)
	{
		wsprintf(buf, "levels\\%s\\%s%c.bmp", LevelInf[GameSettings.Level].Dir, LevelInf[GameSettings.Level].Dir, ii + 'a');
		LoadTextureClever(buf, ii, 256, 256, 0, WorldTextureSet, TRUE);
	}

	// Load world model
	SetMirrorParams(&LevelInf[GameSettings.Level]);
	LoadWorld(GetLevelFilename("w", FILENAME_MAKE_BODY));

	// Load visiboxes (must be before instances!)
	InitVisiBoxes();
	LoadVisiBoxes(GetLevelFilename("vis", FILENAME_MAKE_BODY | FILENAME_GAME_SETTINGS));
	SetPermVisiBoxes();

	// load mirror planes (must be before instances)
	LoadMirrorPlanes(GetLevelFilename("rim", FILENAME_MAKE_BODY));
	MirrorWorldPolys();
	SetWorldMirror();

	// load instances (must be before lights!)
	LoadInstanceModels();
	LoadInstances(GetLevelFilename("fin", FILENAME_MAKE_BODY | FILENAME_GAME_SETTINGS));
	BuildInstanceCollPolys();

	// Load collision polygon data (must be after instances!)
	if ((fp = fopen(GetLevelFilename("ncp", FILENAME_MAKE_BODY), "rb")) == NULL) {
		COL_WorldCollPoly = NULL;
		COL_NWorldCollPolys = 0;
		Box("Error", "Level has no Collision Polygon data", MB_OK | MB_ICONSTOP);
		QuitGame = TRUE;
		return;
	}
	if ((COL_WorldCollPoly = LoadNewCollPolys(fp, &COL_NWorldCollPolys)) == NULL) {
		Box("Error", "Collision polygon data corrupt", MB_OK | MB_ICONSTOP);
		fclose(fp);
		QuitGame = TRUE;
		return;
	}
	// Grid up the collision polygons (and include the instance collisions)
	if (!LoadGridInfo(fp)) {
		Box("Warning", "No collision grid information", MB_OK | MB_ICONEXCLAMATION);
		fclose(fp);
		return;
	}
	fclose(fp);

	// Load lights
	InitLights();
	LoadLights(GetLevelFilename("lit", FILENAME_MAKE_BODY | FILENAME_GAME_SETTINGS));

	// load sfx
	LoadSfx(LevelInf[GameSettings.Level].Dir);

	// load objects
	LoadObjects(GetLevelFilename("fob", FILENAME_MAKE_BODY | FILENAME_GAME_SETTINGS));

	// load ai zones
	LoadAiZones(GetLevelFilename("taz", FILENAME_MAKE_BODY | FILENAME_GAME_SETTINGS));

	// load ai nodes (must be after ai zones)
	LoadAiNodes(GetLevelFilename("fan", FILENAME_MAKE_BODY | FILENAME_GAME_SETTINGS));
	ZoneAiNodes();

	// load triggers
	LoadTriggers(GetLevelFilename("tri", FILENAME_MAKE_BODY | FILENAME_GAME_SETTINGS));

	// load force fields
	LoadLevelFields(&LevelInf[GameSettings.Level]);

	// load best track times
	LoadTrackTimes(&LevelInf[GameSettings.Level]);

	// Load the camera nodes if there are any
	if ((fp = fopen(GetLevelFilename("cam", FILENAME_MAKE_BODY), "rb")) != NULL) {
		CAM_NCameraNodes = LoadCameraNodes(fp);
		fclose(fp);
	} else {
		CAM_NCameraNodes = 0;
	}

	// load edit visicock models
	if (EditMode == EDIT_VISIBOXES)
	{
		LoadEditAiNodeModels();
	}

	// Load edit lights
	if (EditMode == EDIT_LIGHTS)
	{
		LoadEditLightModels();
	}

	// Load edit objects
	if (EditMode == EDIT_OBJECTS)
	{
		InitFileObjects();
		LoadFileObjects(GetLevelFilename("fob", FILENAME_MAKE_BODY | FILENAME_GAME_SETTINGS));
		LoadFileObjectModels();
	}

	// load edit visicock
	if (EditMode == EDIT_VISIBOXES)
	{
		if (!AI_Testing) LoadEditAiNodeModels();
	}

	// Load edit ai nodes
	if (EditMode == EDIT_AINODES)
	{
		InitEditAiNodes();
		LoadEditAiNodes(GetLevelFilename("fan", FILENAME_MAKE_BODY | FILENAME_GAME_SETTINGS));
		if (!AI_Testing) LoadEditAiNodeModels();
	}

	// Load edit track zones
	if (EditMode == EDIT_ZONES)
	{
		InitFileZones();
		LoadFileZones(GetLevelFilename("taz", FILENAME_MAKE_BODY | FILENAME_GAME_SETTINGS));
		if (!AI_Testing) LoadEditAiNodeModels();
	}

	// Load edit triggers
	if (EditMode == EDIT_TRIGGERS)
	{
		InitFileTriggers();
		LoadFileTriggers(GetLevelFilename("tri", FILENAME_MAKE_BODY | FILENAME_GAME_SETTINGS));
	}

	// Load edit cam nodes
	if (EditMode == EDIT_CAM || EditMode == EDIT_TRIGGERS)
	{
		InitEditCamNodes();
		LoadEditCamNodes(GetLevelFilename("cam", FILENAME_MAKE_BODY | FILENAME_GAME_SETTINGS));
		LoadEditCamNodeModels();
	}

	// Load edit fields
	if (EditMode == EDIT_FIELDS)
	{
		InitFileFields();
		LoadFileFields(GetLevelFilename("fld", FILENAME_MAKE_BODY | FILENAME_GAME_SETTINGS));
		LoadFileFieldModels();
	}

	// Load edit portals
	if (EditMode == EDIT_PORTALS)
	{
		LoadEditPortals(GetLevelFilename("por", FILENAME_MAKE_BODY | FILENAME_GAME_SETTINGS));
	}
}

//////////////////////////////////////////////////
// find all levels and init LEVELINFO structure //
//////////////////////////////////////////////////

void FindLevels(void)
{
	long i, j;
	long r, g, b;
	int ch;
	WIN32_FIND_DATA data;
	HANDLE handle;
	FILE *fp;
	LEVELINFO templevel;
	char buf[256];
	char string[256];
	char names[MAX_LEVELS][MAX_LEVEL_DIR_NAME];

// get first directory

	handle = FindFirstFile("levels\\*.*", &data);
	if (handle == INVALID_HANDLE_VALUE)
	{
		Box("ERROR", "Can't find any level directories", MB_OK);
		QuitGame = TRUE;
		return;
	}

// loop thru each subsequent directory

	GameSettings.LevelNum = 0;

	while (TRUE)
	{
		if (!FindNextFile(handle, &data))
			break;

// skip if not a good dir

		if (!(data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			continue;

		if (!strcmp(data.cFileName, "."))
			continue;

		if (!strcmp(data.cFileName, ".."))
			continue;

// add to dir list if valid info file in this dir

		wsprintf(buf, "levels\\%s\\%s.inf", data.cFileName, data.cFileName);
		fp = fopen(buf, "rb");

		if (fp && GameSettings.LevelNum < MAX_LEVELS)
		{
			for (i = 0 ; i < (long)strlen(data.cFileName) ; i++) data.cFileName[i] = toupper(data.cFileName[i]);
			memcpy(names[GameSettings.LevelNum], data.cFileName, MAX_LEVEL_DIR_NAME);
			GameSettings.LevelNum++;
			fclose(fp);
		}
	}

// close search handle

	FindClose(handle);

// alloc LEVELINFO structure

	LevelInf = (LEVELINFO*)malloc(sizeof(LEVELINFO) * GameSettings.LevelNum);
	if (!LevelInf)
	{
		Box("ERROR", "Can't alloc memory for level info", MB_OK);
		QuitGame = TRUE;
		return;
	}

// fill in LEVELINFO structure

	for (i = 0 ; i < GameSettings.LevelNum ; i++)
	{

// set dir name

		memcpy(LevelInf[i].Dir, names[i], MAX_LEVEL_DIR_NAME);

// set defaults

		wsprintf(LevelInf[i].Name, "Untitled");
		wsprintf(LevelInf[i].EnvStill, "gfx\\envstill.bmp");
		wsprintf(LevelInf[i].EnvRoll, "gfx\\envroll.bmp");
		SetVector(&LevelInf[i].NormalStartPos, 0, 0, 0);
		SetVector(&LevelInf[i].ReverseStartPos, 0, 0, 0);
		LevelInf[i].NormalStartRot = 0;
		LevelInf[i].ReverseStartRot = 0;
		LevelInf[i].NormalStartGrid = 0;
		LevelInf[i].ReverseStartGrid = 0;
		LevelInf[i].FarClip = 6144;
		LevelInf[i].FogStart = 5120;
		LevelInf[i].FogColor = 0x000000;
		LevelInf[i].VertFogStart = 0;
		LevelInf[i].VertFogEnd = 0;
		LevelInf[i].WorldRGBper = 100;
		LevelInf[i].ModelRGBper = 100;
		LevelInf[i].InstanceRGBper = 100;
		LevelInf[i].MirrorType = 0;
		LevelInf[i].MirrorMix = 0.75f;
		LevelInf[i].MirrorIntensity = 1;
		LevelInf[i].MirrorDist = 256;

// open .INF file

		wsprintf(buf, "levels\\%s\\%s.inf", names[i], names[i]);
		fp = fopen(buf, "r");

// scan all strings

		while (TRUE)
		{

// get a string

			if (fscanf(fp, "%s", string) == EOF)
				break;

// comment?

			if (string[0] == ';')
			{
				do
				{
					ch = fgetc(fp);
				} while (ch != '\n' && ch != EOF);
				continue;
			}

// name?

			if (!strcmp(string, "NAME"))
			{
				do
				{
					ch = fgetc(fp);
				} while (ch != '\'' && ch != EOF);

				j = 0;
				while (TRUE)
				{
					ch = fgetc(fp);
					if (ch != '\'' && ch != EOF && j < MAX_LEVEL_NAME - 1)
					{
						LevelInf[i].Name[j] = (char)ch;
						j++;
					}
					else
					{
						LevelInf[i].Name[j] = 0;
						break;
					}
				}
				continue;
			}

// env still?

			if (!strcmp(string, "ENVSTILL"))
			{
				do
				{
					ch = fgetc(fp);
				} while (ch != '\'' && ch != EOF);

				j = 0;
				while (TRUE)
				{
					ch = fgetc(fp);
					if (ch != '\'' && ch != EOF && j < MAX_ENV_NAME - 1)
					{
						LevelInf[i].EnvStill[j] = (char)ch;
						j++;
					}
					else
					{
						LevelInf[i].EnvStill[j] = 0;
						break;
					}
				}
				continue;
			}

// env roll?

			if (!strcmp(string, "ENVROLL"))
			{
				do
				{
					ch = fgetc(fp);
				} while (ch != '\'' && ch != EOF);

				j = 0;
				while (TRUE)
				{
					ch = fgetc(fp);
					if (ch != '\'' && ch != EOF && j < MAX_ENV_NAME - 1)
					{
						LevelInf[i].EnvRoll[j] = (char)ch;
						j++;
					}
					else
					{
						LevelInf[i].EnvRoll[j] = 0;
						break;
					}
				}
				continue;
			}

// start pos

			if (!strcmp(string, "STARTPOS"))
			{
				fscanf(fp, "%f %f %f", &LevelInf[i].NormalStartPos.v[X], &LevelInf[i].NormalStartPos.v[Y], &LevelInf[i].NormalStartPos.v[Z]);
				continue;
			}

// reverse start pos

			if (!strcmp(string, "STARTPOSREV"))
			{
				fscanf(fp, "%f %f %f", &LevelInf[i].ReverseStartPos.v[X], &LevelInf[i].ReverseStartPos.v[Y], &LevelInf[i].ReverseStartPos.v[Z]);
				continue;
			}

// start rot

			if (!strcmp(string, "STARTROT"))
			{
				fscanf(fp, "%f", &LevelInf[i].NormalStartRot);
				continue;
			}

// reverse start rot

			if (!strcmp(string, "STARTROTREV"))
			{
				fscanf(fp, "%f", &LevelInf[i].ReverseStartRot);
				continue;
			}

// start grid

			if (!strcmp(string, "STARTGRID"))
			{
				fscanf(fp, "%d", &LevelInf[i].NormalStartGrid);
				continue;
			}

// reverse start grid

			if (!strcmp(string, "STARTGRIDREV"))
			{
				fscanf(fp, "%d", &LevelInf[i].ReverseStartGrid);
				continue;
			}

// far clip

			if (!strcmp(string, "FARCLIP"))
			{
				fscanf(fp, "%f", &LevelInf[i].FarClip);
				continue;
			}

// fog start

			if (!strcmp(string, "FOGSTART"))
			{
				fscanf(fp, "%f", &LevelInf[i].FogStart);
				continue;
			}

// fog color

			if (!strcmp(string, "FOGCOLOR"))
			{
				fscanf(fp, "%ld %ld %ld", &r, &g, &b);
				LevelInf[i].FogColor = (r << 16) | (g << 8) | b;
				continue;
			}

// vert fog start

			if (!strcmp(string, "VERTFOGSTART"))
			{
				fscanf(fp, "%f", &LevelInf[i].VertFogStart);
				continue;
			}

// vert fog end

			if (!strcmp(string, "VERTFOGEND"))
			{
				fscanf(fp, "%f", &LevelInf[i].VertFogEnd);
				continue;
			}

// world rgb percent

			if (!strcmp(string, "WORLDRGBPER"))
			{
				fscanf(fp, "%ld", &LevelInf[i].WorldRGBper);
				continue;
			}

// model rgb percent

			if (!strcmp(string, "MODELRGBPER"))
			{
				fscanf(fp, "%ld", &LevelInf[i].ModelRGBper);
				continue;
			}

// instance rgb percent

			if (!strcmp(string, "INSTANCERGBPER"))
			{
				fscanf(fp, "%ld", &LevelInf[i].InstanceRGBper);
				continue;
			}

// mirror info

			if (!strcmp(string, "MIRRORS"))
			{
				fscanf(fp, "%ld %f %f %f", &LevelInf[i].MirrorType, &LevelInf[i].MirrorMix, &LevelInf[i].MirrorIntensity, &LevelInf[i].MirrorDist);
				continue;
			}
		}

// close .INF file

		fclose(fp);
	}

// sort alphabetically

	for (i = GameSettings.LevelNum - 1 ; i ; i--) for (j = 0 ; j < i ; j++)
	{
		if (strcmp(LevelInf[j].Name, LevelInf[j + 1].Name) > 0)
		{
			templevel = LevelInf[j];
			LevelInf[j] = LevelInf[j + 1];
			LevelInf[j + 1] = templevel;
		}
	}
}

/////////////////////////
// free level info mem //
/////////////////////////

void FreeLevels(void)
{
	free(LevelInf);
}

//////////////////////////////////////////////////////////////
// set level name - return corresponging level number or -1 //
//////////////////////////////////////////////////////////////

long GetLevelNum(char *dir)
{
	long i;

// look for level

	for (i = 0 ; i < GameSettings.LevelNum ; i++)
	{
		if (!strcmp(LevelInf[i].Dir, dir))
		{
			return i;
		}
	}

// not found!

	return -1;
}

////////////////////////
// get level filename //
////////////////////////

char *GetLevelFilename(char *filename, long flag)
{

// make body

	if (flag & FILENAME_MAKE_BODY)
	{
		if (flag & FILENAME_GAME_SETTINGS && GameSettings.Reversed)
			wsprintf(LevelFilenameBuffer, "levels\\%s\\reversed\\%s.%s", LevelInf[GameSettings.Level].Dir, LevelInf[GameSettings.Level].Dir, filename);
		else
			wsprintf(LevelFilenameBuffer, "levels\\%s\\%s.%s", LevelInf[GameSettings.Level].Dir, LevelInf[GameSettings.Level].Dir, filename);
	}

// don't make body

	else
	{
		if (flag & FILENAME_GAME_SETTINGS && GameSettings.Reversed)
			wsprintf(LevelFilenameBuffer, "levels\\%s\\reversed\\%s", LevelInf[GameSettings.Level].Dir, filename);
		else
			wsprintf(LevelFilenameBuffer, "levels\\%s\\%s", LevelInf[GameSettings.Level].Dir, filename);
	}

	return LevelFilenameBuffer;
}


/////////////////////////////////////////////////////////////////////
//
// LoadLevelFields: load in the static fields associtaed with the 
// passed level.
//
/////////////////////////////////////////////////////////////////////
#ifdef _PC
bool LoadLevelFields(LEVELINFO *levelInfo)
{
	int			iField;
	long		nFields;
	REAL		rad;
	BBOX		bBox;
	FILE		*fp;
	FILE_FIELD	fileField;

	// open the file
	fp = fopen(GetLevelFilename("fld", FILENAME_MAKE_BODY | FILENAME_GAME_SETTINGS), "rb");
	if (fp == NULL) {
		return FALSE;
	}

	// get the number of fields
	if (fread(&nFields, sizeof(long), 1, fp) < 1) {
		fclose(fp);
		return FALSE;
	}

	if (nFields < 1) {
		fclose(fp);
		return TRUE;
	}

	// Allocate space for the positions and matrices
	LEV_LevelFieldPos = (VEC *)malloc(sizeof(VEC) * nFields);
	LEV_LevelFieldMat = (MAT *)malloc(sizeof(MAT) * nFields);

	// Read in all the fields
	for (iField = 0; iField < nFields; iField++) {

		if (fread(&fileField, sizeof(FILE_FIELD), 1, fp) < 1) {
			fclose(fp);
			return FALSE;
		}

		// Set the position and matrix in the array
		CopyVec(&fileField.Pos, &LEV_LevelFieldPos[iField]); 
		CopyMat(&fileField.Matrix, &LEV_LevelFieldMat[iField]);

		// Create the field
		switch(fileField.Type) {

		case FILE_FIELD_TYPE_LINEAR:
			rad = (REAL)sqrt(fileField.Size[0] * fileField.Size[0] + fileField.Size[1] * fileField.Size[1] + fileField.Size[2] * fileField.Size[2]);
			SetBBox(&bBox, -rad, rad, -rad, rad, -rad, rad);
			AddLinearField(FIELD_PARENT_NONE, FIELD_PRIORITY_MAX, &LEV_LevelFieldPos[iField], &LEV_LevelFieldMat[iField], &bBox, (VEC *)&fileField.Size, &fileField.Dir, fileField.Mag, fileField.Damping);
			break;

		case FILE_FIELD_TYPE_ORIENTATION:
			rad = (REAL)sqrt(fileField.Size[0] * fileField.Size[0] + fileField.Size[1] * fileField.Size[1] + fileField.Size[2] * fileField.Size[2]);
			SetBBox(&bBox, -rad, rad, -rad, rad, -rad, rad);
			AddOrientationField(FIELD_PARENT_NONE, FIELD_PRIORITY_MAX, &LEV_LevelFieldPos[iField], &LEV_LevelFieldMat[iField], &bBox, (VEC *)&fileField.Size, &fileField.Dir, fileField.Mag, fileField.Damping);
			break;

		case FILE_FIELD_TYPE_VELOCITY:
			rad = (REAL)sqrt(fileField.Size[0] * fileField.Size[0] + fileField.Size[1] * fileField.Size[1] + fileField.Size[2] * fileField.Size[2]);
			SetBBox(&bBox, -rad, rad, -rad, rad, -rad, rad);
			AddVelocityField(FIELD_PARENT_NONE, FIELD_PRIORITY_MAX, &LEV_LevelFieldPos[iField], &LEV_LevelFieldMat[iField], &bBox, (VEC *)&fileField.Size, &fileField.Dir, fileField.Mag);
			break;

		case FILE_FIELD_TYPE_SPHERICAL:
			rad = fileField.RadEnd;
			SetBBox(&bBox, -rad, rad, -rad, rad, -rad, rad);
			AddSphericalField(FIELD_PARENT_NONE, FIELD_PRIORITY_MAX, &LEV_LevelFieldPos[iField], fileField.RadStart, fileField.RadEnd, fileField.GradStart, fileField.GradEnd);
			break;
		
		default:
			break;
		}
	}

	fclose(fp);
	return TRUE;
}
#endif

