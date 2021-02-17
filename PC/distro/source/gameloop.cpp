/*********************************************************************************************
 *
 * gameloop.cpp
 *
 * Re-Volt (PC) Copyright (c) Probe Entertainment 1998
 * 
 * Contents:
 *			Er, the main game loop :)
 *
 *********************************************************************************************
 *
 * 05/04/98 Matt Taylor
 *  File inception.
 *	Gameloop code moved out of main.cpp.
 *
 *********************************************************************************************/

#include "revolt.h"
#include "main.h"
#include "dx.h"
#include "geom.h"
#include "model.h"
#include "texture.h"
#include "particle.h"
#include "aerial.h"
#include "play.h"
#include "NewColl.h"
#include "Body.h"
#include "car.h"
#include "input.h"
#include "text.h"
#include "shadow.h"
#include "camera.h"
#include "light.h"
#include "world.h"
#include "draw.h"
#include "DrawObj.h"
#include "visibox.h"
#include "level.h"
#include "ctrlread.h"
#include "obj_init.h"
#include "object.h"
#include "control.h"
#include "move.h"
#include "gameloop.h"
#include "level.h"
#include "editai.h"
#include "editobj.h"
#include "editzone.h"
#include "instance.h"
#include "Player.h"
#include "timing.h"
#include "Ghost.h"
#include "Registry.h"
#include "edittrig.h"
#include "trigger.h"
#include "editcam.h"
#include "edfield.h"
#include "sfx.h"
#include "panel.h"
#include "ai.h"
#include "edportal.h"
#include "weapon.h"
#ifdef _PC
#include "spark.h"
#endif

// globals
					  
bool DrawGridCollSkin = FALSE;
int		NPhysicsLoops = 0;

static bool DrawRearView = FALSE;
static CAMERA *RearCamera;
static bool DrawGhostView = FALSE;
static CAMERA *GhostCamera;
static long PawsSelect;

#if SHOW_PHYSICS_INFO
static bool PhysicsInfoTog = FALSE;
#endif

#if USE_DEBUG_ROUTINES
extern REAL DBG_dt;
#endif

// record avi stuff

#if RECORD_AVI
static void RecordAvi(void);
long AviFrame = 0, AviRecord = FALSE;
#endif

// ghost takeover stuff

#if GHOST_TAKEOVER
static void GhostTakeover(void);
long GhostTakeoverFlag = FALSE, GhostTakeoverTime = 0;
REAL CamTime = ZERO;
REAL ChangeCamTime = Real(5);
#endif

#if CHECK_ZZZ
extern void CheckZZZ(void);
#endif

// paws menu

char *PawsMenuText[] = {
	"RESUME",
	" QUIT",
};

//--------------------------------------------------------------------------------------------------------------------------

void GLP_GameLoop(void)
{
	int iStep;
	long j;
	char buf[256];

// ghost take over?

#if GHOST_TAKEOVER
	GhostTakeover();
#endif

// cheat?

	CheckCheatStrings();

// reset 3d poly list

	Reset3dPolyList();

// Update game timers

	FrameCount++;
	UpdateTimeFactor();
	UpdateRaceTimers();

// Update the ghost car

	StoreGhostData(&PLR_LocalPlayer->car);

// update keyboard / mouse

	ReadMouse();
	ReadKeyboard();
	ReadJoystick();

// maintain sounds

	MaintainAllSfx();

// Get control inputs

	CON_DoPlayerControl();

// Check local keys for home, camera, etc

	CRD_CheckLocalKeys();

// reset mesh fx list

	if (!GameSettings.Paws)
	{
		ResetMeshFxList();
	}

// reset oil slick list

	ResetOilSlickList();

// perform AI

	if (!GameSettings.Paws)
	{
		AI_ProcessAllAIs();
	}

// move objects

	if (!GameSettings.Paws)
	{
		NPhysicsLoops = 1 + (int) (TimeStep * MAX_TIMESTEP);
		TimeStep /= Real(NPhysicsLoops);
		for (iStep = 0; iStep < NPhysicsLoops; iStep++) {
			//GRD_UpdateObjGrid();

			// Deal with the collisions of the objects
			COL_DoObjectCollisions();

			// Move game objects
			MOV_MoveObjects();
		}
		TimeStep *= Real(NPhysicsLoops);
	}

// Send packets

	if (!GameSettings.Paws)
	{
		if (GameSettings.GameType == GAMETYPE_SERVER || GameSettings.GameType == GAMETYPE_CLIENT)
		{
			SendLocalCarData();
			GetRemoteMessages();
		}
	}
		
// Build car world matrices

	BuildAllCarWorldMatrices();

// Process lights

	ProcessLights();

// Process sparks

	if (!GameSettings.Paws)
	{
		ProcessSparks();
	}

// texture animations

	ProcessTextureAnimations();

// check triggers

	CheckTriggers();

// toggle rear view

	if (Keys[DIK_R] && !LastKeys[DIK_R])
	{
		DrawRearView = !DrawRearView;
		if (DrawRearView) {
			RearCamera = AddCamera(32, 32, 128, 128, CAMERA_FLAG_SECONDARY);
			SetCameraFollow(RearCamera, PLR_LocalPlayer->ownobj, CAM_FOLLOW_FRONT);
		} else {
			RemoveCamera(RearCamera);
		}
	}

// toggle ghost view

	if (Keys[DIK_G] && !LastKeys[DIK_G])
	{
		DrawGhostView = !DrawGhostView;
		if (DrawGhostView) {
			GhostCamera = AddCamera(480, 32, 128, 128, CAMERA_FLAG_SECONDARY);
			SetCameraFollow(GhostCamera, GHO_GhostPlayer->ownobj, CAM_FOLLOW_CLOSE);
		} else {
			RemoveCamera(GhostCamera);
		}
	}

// surface check + flip

	CheckSurfaces();
	FlipBuffers();

// record avi?

#if RECORD_AVI
	RecordAvi();
#endif

// Begin render

	D3Ddevice->BeginScene();

// render all cameras

	for (CameraCount = 0 ; CameraCount < MAX_CAMERAS ; CameraCount++) if (Camera[CameraCount].Flag != CAMERA_FLAG_FREE)
	{

// update camera + set camera view vars

		UpdateCamera(&Camera[CameraCount]);

// set and clear viewport

		SetViewport(Camera[CameraCount].X, Camera[CameraCount].Y, Camera[CameraCount].Xsize, Camera[CameraCount].Ysize, BaseGeomPers + Camera[CameraCount].Lens);
		InitRenderStates();
		ClearBuffers();

		SetCameraView(&Camera[CameraCount].WMatrix, &Camera[CameraCount].WPos, Camera[CameraCount].Shake);
		SetCameraVisiMask(&Camera[CameraCount].WPos);

// render opaque polys

		ResetSemiList();

		if (DrawGridCollSkin)
		{
			DrawGridCollPolys(PosToCollGrid(&PLR_LocalPlayer->car.Body->Centre.Pos));
		}
		else
		{
			DrawWorld();
			DrawInstances();
		}

		DrawObjects();
		DrawAllCars();
		Draw3dPolyList();

// DRAW object at line-following camera pos
		//long nodeNum, linkNum;
		//FindNearestCameraPath(&PLR_LocalPlayer->car.Body->Centre.Pos, &nodeNum, &linkNum);
		//DrawModel(&PLR_LocalPlayer->car.Models->Body[0], &Identity, &CAM_NodeCamPos, MODEL_PLAIN);

// render edit models?

		if (EditMode != EDIT_NONE)
		{
			FlushPolyBuckets();

			if (EditMode == EDIT_LIGHTS) DrawFileLights();
			if (EditMode == EDIT_VISIBOXES) DrawVisiBoxes();
			if (EditMode == EDIT_OBJECTS) DrawFileObjects();
			if (EditMode == EDIT_AINODES) DrawAiNodes();
			if (EditMode == EDIT_ZONES) DrawFileZones();
			if (EditMode == EDIT_TRIGGERS) DrawTriggers();
			if (EditMode == EDIT_CAM || EditMode == EDIT_TRIGGERS) DrawEditCamNodes();
			if (EditMode == EDIT_FIELDS) DrawFields();
			if (EditMode == EDIT_PORTALS) DrawPortals();
		}

// draw ai 'current node'

		if (AI_Testing)
		{
			if (PLR_LocalPlayer->CarAI.CurNode)
			{
				DrawModel(&EditAiNodeModel[0], &IdentityMatrix, &PLR_LocalPlayer->CarAI.CurNode->Node[0].Pos, MODEL_PLAIN);
				DrawModel(&EditAiNodeModel[1], &IdentityMatrix, &PLR_LocalPlayer->CarAI.CurNode->Node[1].Pos, MODEL_PLAIN);
			}
		}

// flush poly buckets

		FlushPolyBuckets();
		FlushEnvBuckets();

// render semi polys

		DrawSemiList();
		DrawSparks();
		DrawTrails();
		DrawSkidMarks();
		DrawAllCarShadows();
		DrawAllGhostCars();

// begin primary camera stuff

		if (Camera[CameraCount].Flag == CAMERA_FLAG_PRIMARY)
		{

// edit mode?

			if (EditMode != EDIT_NONE)
			{
				if (EditMode == EDIT_LIGHTS) EditFileLights();
				if (EditMode == EDIT_VISIBOXES) EditVisiBoxes();
				if (EditMode == EDIT_OBJECTS) EditFileObjects();
				if (EditMode == EDIT_INSTANCES) EditInstances();
				if (EditMode == EDIT_AINODES) EditAiNodes();
				if (EditMode == EDIT_ZONES) EditFileZones();
				if (EditMode == EDIT_TRIGGERS) EditTriggers();
				if (EditMode == EDIT_CAM) EditCamNodes();
				if (EditMode == EDIT_FIELDS) EditFields();
				if (EditMode == EDIT_PORTALS) EditPortals();
			}

// draw control panel

			DrawControlPanel();

// begin text state

			BeginTextState();

// display network players

			if (GameSettings.GameType == GAMETYPE_SERVER || GameSettings.GameType == GAMETYPE_CLIENT)
				DisplayPlayers();

// display edit info?

			if (CAM_MainCamera->Type == CAM_EDIT)
			{
				if (EditMode == EDIT_LIGHTS && CurrentEditLight) DisplayLightInfo(CurrentEditLight);
				if (EditMode == EDIT_VISIBOXES && CurrentVisiBox) DisplayVisiBoxInfo(CurrentVisiBox);
				if (EditMode == EDIT_OBJECTS && CurrentFileObject) DisplayFileObjectInfo(CurrentFileObject);
				if (EditMode == EDIT_INSTANCES && CurrentInstance) DisplayInstanceInfo(CurrentInstance);
				if (EditMode == EDIT_AINODES && CurrentEditAiNode) DisplayAiNodeInfo(CurrentEditAiNode);
				if (EditMode == EDIT_ZONES && CurrentFileZone) DisplayZoneInfo(CurrentFileZone);
				if (EditMode == EDIT_TRIGGERS && CurrentTrigger) DisplayTriggerInfo(CurrentTrigger);
				if (EditMode == EDIT_CAM && CurrentEditCamNode) DisplayCamNodeInfo(CurrentEditCamNode);
				if (EditMode == EDIT_FIELDS && CurrentField) DisplayFieldInfo(CurrentField);
				if (EditMode == EDIT_PORTALS && CurrentPortal) DisplayPortalInfo(CurrentPortal);

				DrawMousePointer(0xffffff);
			}

			if (EditMode == EDIT_VISIBOXES) DisplayCamVisiMask();
			if (EditMode == EDIT_ZONES) DisplayCurrentTrackZone();


// debug text

			//wsprintf(buf, "MinDist = %d", (int)(1000.0f * LOSMinDist1));
			//DumpText(100, 50, 16, 24, 0xffffff, buf);

#if SCREEN_DEBUG
			FrameTime = TimerCurrent;
			if (FrameTime - FrameTimeLast > TimerFreq / 2)
			{
				FrameRate = (FrameCount - FrameCountLast) * TimerFreq / (FrameTime - FrameTimeLast);
				FrameTimeLast = FrameTime;
				FrameCountLast = FrameCount;
			}

			wsprintf(buf, "%d", FrameRate);
			DumpText(0, 0, 8, 16, 0xffff00, buf);

			static long screendebugshow = 0;
			if (Keys[DIK_F9] && !LastKeys[DIK_F9] && Everything) screendebugshow = !screendebugshow;

			if (screendebugshow)
			{
				wsprintf(buf, "State %d, %d  Lights %d  Car %d, %d, %d", RenderStateChange, TextureStateChange, TotalLightCount, (long)PLR_LocalPlayer->car.Body->Centre.Pos.v[X], (long)PLR_LocalPlayer->car.Body->Centre.Pos.v[Y], (long)PLR_LocalPlayer->car.Body->Centre.Pos.v[Z]);
				DumpText(176, 0, 8, 16, 0xffffff, buf);

				wsprintf(buf, "Cube %d, %d  World %d, %d  Model %d, %d", WorldBigCubeCount, WorldCubeCount, WorldPolyCount, WorldDrawnCount, ModelPolyCount, ModelDrawnCount);
				DumpText(0, 464, 8, 16, 0xffffff, buf);
			}

#endif

// record times

#if SCREEN_TIMES
			static long recordshow = 0;
			if (Keys[DIK_F10] && !LastKeys[DIK_F10] && Everything) recordshow = (recordshow + 1) % 3;

			if (recordshow == 1)
			{
				for (j = 0 ; j < MAX_RECORD_TIMES ; j++)
				{
					wsprintf(buf, "%02d:%02d:%03d %6.6s - %6.6s", MINUTES(TrackRecords.RecordLap[j].Time), SECONDS(TrackRecords.RecordLap[j].Time), THOUSANDTHS(TrackRecords.RecordLap[j].Time), TrackRecords.RecordLap[j].Player, TrackRecords.RecordLap[j].Car);
					DumpText(16, 128 + j * 16, 8, 16, 0x00ffff, buf);
				}
			}
			if (recordshow == 2)
			{
				for (j = 0 ; j < MAX_RECORD_TIMES ; j++)
				{
					wsprintf(buf, "%02d:%02d:%03d %6.6s - %6.6s", MINUTES(TrackRecords.RecordRace[j].Time), SECONDS(TrackRecords.RecordRace[j].Time), THOUSANDTHS(TrackRecords.RecordRace[j].Time), TrackRecords.RecordRace[j].Player, TrackRecords.RecordRace[j].Car);
					DumpText(16, 128 + j * 16, 8, 16, 0x00ffff, buf);
				}
			}
#endif

// detail menu

			if (Keys[DIK_F12] && !LastKeys[DIK_F12] && !DetailMenuTogg && (Keys[DIK_LSHIFT] || Everything))
			{
				DetailMenuTogg = TRUE;
			}
			if (Keys[DIK_ESCAPE] && !LastKeys[DIK_ESCAPE] && DetailMenuTogg)
			{
				DetailMenuTogg = FALSE;
				LastKeys[DIK_ESCAPE] = TRUE;
			}
			if (DetailMenuTogg) DetailMenu();


// paws?

			if (GameSettings.Paws)
			{
				if (PawsMenu())
				{
					LEV_EndLevel();
					Event = GoFront;
				}
			}

			if (Keys[DIK_ESCAPE] && !LastKeys[DIK_ESCAPE] && !DetailMenuTogg && !GameSettings.Paws)
			{
				GameSettings.Paws = TRUE;
				PawsSelect = 0;
				SetMouseExclusive(FALSE);
				PauseAllSfx();
			}

// physics info

#if SHOW_PHYSICS_INFO
			if (Keys[DIK_F11] && !LastKeys[DIK_F11] && Everything) PhysicsInfoTog = !PhysicsInfoTog;
			if (PhysicsInfoTog) ShowPhysicsInfo();
#endif

// read zzz

#if CHECK_ZZZ
			CheckZZZ();
#endif

		}
	}

// end render

	D3Ddevice->EndScene();
}

///////////////
// paws menu //
///////////////

long PawsMenu(void)
{
	long i, col;

// move

	if (Keys[DIK_UP] && !LastKeys[DIK_UP] && PawsSelect) PawsSelect--;
	if (Keys[DIK_DOWN] && !LastKeys[DIK_DOWN] && PawsSelect < PAWS_MENU_NUM - 1) PawsSelect++;

// sort each detail setting

	for (i = 0 ; i < PAWS_MENU_NUM ; i++)
	{
		if (i == PawsSelect)
			col = 0xffffff;
		else
			col = 0x808080;

// dump text

		SET_TPAGE(TPAGE_BIGFONT);
		DumpBigText(248, i * 32 + 208, 24, 32, col, PawsMenuText[i]);
		SET_TPAGE(TPAGE_FONT);
	}

// selected?

	if (Keys[DIK_RETURN] && !LastKeys[DIK_RETURN])
	{
		SetMouseExclusive(TRUE);

		if (PawsSelect == PAWS_MENU_RESUME)
		{
			GameSettings.Paws = FALSE;
			ResumeAllSfx();
		}

		if (PawsSelect == PAWS_MENU_QUIT)
		{
			return TRUE;
		}
	}

// return OK

	return FALSE;
}

////////////////
// record avi //
////////////////
#if RECORD_AVI

static void RecordAvi(void)
{
	char buf[128];

// start recording?

	if (!AviRecord && Keys[DIK_1])
	{
		AviRecord = TRUE;
	}

// stop recording?

	if (AviRecord && Keys[DIK_2])
	{
		AviRecord = FALSE;
	}

// recording?

	if (AviRecord)
	{
		wsprintf(buf, "..\\avi\\scr%05d.bmp", AviFrame);
		SaveFrontBuffer(buf);
		AviFrame++;
	}
}
#endif

//////////////////////
// ghost takeover ? //
//////////////////////
#if GHOST_TAKEOVER

extern GHOST_DATA *BestGhostData;

long GhostWeapon[] = {
	OBJECT_TYPE_SHOCKWAVE,
	OBJECT_TYPE_FIREWORK,
	OBJECT_TYPE_WATERBOMB,
	OBJECT_TYPE_ELECTROPULSE,
	OBJECT_TYPE_CHROMEBALL,
};

static void GhostTakeover(void)
{
	long i, keys;
	static bool ForceGhostCam = FALSE;

// cockdex

/*	GhostSolid = TRUE;
	ForceGhostCam = TRUE;

	if (!GhostTakeoverFlag)
	{
		GhostTakeoverFlag = TRUE;
		SetCameraRail(CAM_MainCamera, GHO_GhostPlayer->ownobj, 1);
	}

	if (!(rand() & 1023))
	{
		long flag, weapon;
		flag = (long)GHO_GhostPlayer;
		weapon = GhostWeapon[rand() % 5];
		CreateObject(&GHO_GhostPlayer->car.Body->Centre.Pos, &GHO_GhostPlayer->car.Body->Centre.WMatrix, weapon, &flag);
	}*/

// Don't fuck up edit mode

	if ((CAM_MainCamera->Type == CAM_EDIT) || (CAM_MainCamera->Type == CAM_FREEDOM))
		return;

// get key state

	keys = (PLR_LocalPlayer->controls.dx | PLR_LocalPlayer->controls.dy);

	if (!keys) for (i = 0 ; i < 256 ; i++)
	{
		if ((keys = Keys[i])) break;
	}

	if (Keys[DIK_F6] && !LastKeys[DIK_F6]) {
		ForceGhostCam = !ForceGhostCam;
		if (ForceGhostCam) {
			SetCameraRail(CAM_MainCamera, GHO_GhostPlayer->ownobj, 1);
		} else {
			GhostTakeoverFlag = FALSE;
			GhostTakeoverTime = 0;
			SetCameraFollow(CAM_MainCamera, PLR_LocalPlayer->ownobj, 0);
		}
	}

	if (Keys[DIK_F7] && !LastKeys[DIK_F7]) {
		if (CAM_MainCamera->Object == GHO_GhostPlayer->ownobj) {
			CAM_MainCamera->Object = PLR_LocalPlayer->ownobj;
		} else {
			CAM_MainCamera->Object = GHO_GhostPlayer->ownobj;
		}
		InitCamPos(CAM_MainCamera);
	}

// human

	if (!GhostTakeoverFlag && !ForceGhostCam)
	{
		GhostTakeoverTime += (long)(TimeStep * 1000.0f);
		if (keys) GhostTakeoverTime = 0;
		if (GhostTakeoverTime > GHOST_TAKEOVER_TIME)
		{
			GhostTakeoverFlag = TRUE;
			SetCameraRail(CAM_MainCamera, GHO_GhostPlayer->ownobj, 1);
		}
	}

// ghost

	else
	{
		if (GHO_BestFrame > GHO_BestGhostInfo->NFrames - 3)
		{
			GHO_BestFrame = 0;
			GHO_GhostPlayer->car.CurrentLapStartTime = TimerCurrent - 
				(GHO_GhostPlayer->car.CurrentLapTime - BestGhostData[GHO_BestGhostInfo->NFrames - 1].Time);

		}

// Change camera?

		if (CamTime > ChangeCamTime) {
			REAL choice;

			CamTime = ZERO;
			ChangeCamTime = Real(2) + frand(6);

			choice = frand(100);
			if (choice < 50) {	
				// Static camera
				SetCameraRail(CAM_MainCamera, GHO_GhostPlayer->ownobj, CAM_RAIL_STATIC_NEAREST);
			} else if (choice < 58) {
				// Follow camera
				SetCameraFollow(CAM_MainCamera, GHO_GhostPlayer->ownobj, CAM_FOLLOW_BEHIND);
			} else if (choice < 66) {
				// Follow camera close
				SetCameraFollow(CAM_MainCamera, GHO_GhostPlayer->ownobj, CAM_FOLLOW_CLOSE);
			} else if (choice < 74) {
				// Side Camera
				SetCameraFollow(CAM_MainCamera, GHO_GhostPlayer->ownobj, CAM_FOLLOW_LEFT);
			} else if (choice < 82) {
				// Side Camera
				SetCameraFollow(CAM_MainCamera, GHO_GhostPlayer->ownobj, CAM_FOLLOW_RIGHT);
			} else if (choice < 90) {
				// Rear View Camera
				SetCameraFollow(CAM_MainCamera, GHO_GhostPlayer->ownobj, CAM_FOLLOW_FRONT);
			} else {
				// In car camera
				SetCameraAttached(CAM_MainCamera, GHO_GhostPlayer->ownobj, CAM_ATTACHED_INCAR);
			}
		} else {
			CamTime += TimeStep;
		}

		if (keys && !ForceGhostCam)
		{
			GhostTakeoverFlag = FALSE;
			GhostTakeoverTime = 0;
			SetCameraFollow(CAM_MainCamera, PLR_LocalPlayer->ownobj, 0);
		}
	}
}
#endif

//////////////
// read zzz //
//////////////

#if CHECK_ZZZ

static void CheckZZZ(void)
{
	FILE *fp;
	char name[256], action[256], mess[256], user[256];
	static char lastmp3[256], lastmess[256];
	static float time = 0.0f;
	short ch, i;
	DWORD size;

// last mess

	DumpText(0, 128, 16, 32, 0xffff00, lastmess);

// not all the time!

	time -= TimeStep;
	if (time > 0.0f)
		return;

	time = 10.0f;

	lastmess[0] = 0;

	size = 256;
	GetUserName(user, &size);

// file exists?

	if ((fp = fopen("n:\\!\\!!!!!!!!", "r")) == NULL)
		return;

// parse

	while (TRUE)
	{

// get a name

		if (fscanf(fp, "%s%s", name, action) == EOF)
			break;

// me?

		if (!strcmp(name, "ALL") || !strcmp(name, user))
		{

// quit?

			if (!strcmp(action, "QUIT"))
			{
				QuitGame = TRUE;
				return;
			}

// get message

			do {
				ch = fgetc(fp);
			} while (ch != '\'' && ch != EOF);

			i = 0;
			while (TRUE)
			{
				ch = fgetc(fp);
				if (ch != '\'' && ch != EOF && i < 255)
				{
					mess[i] = (char)ch;
					i++;
				}
				else
				{
					mess[i] = 0;
					break;
				}
			}

// play MP3?

			if (!strcmp(action, "MP3"))
			{
				if (strcmp(lastmp3, mess))
				{
					strcpy(lastmp3, mess);
					StopMP3();
					PlayMP3(mess);
				}
			}

// text?

			else if (!strcmp(action, "TEXT"))
			{
				strcpy(lastmess, mess);
			}
		}

// next line

		do {
			ch = fgetc(fp);
		} while (ch != '\n' && ch != EOF);
	}

// close

	fclose(fp);

}
#endif
