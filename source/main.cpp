
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
#include "sfx.h"
#include "text.h"
#include "shadow.h"
#include "camera.h"
#include "light.h"
#include "world.h"
#include "draw.h"
#include "DrawObj.h"
#include "visibox.h"
#include "editobj.h"
#include "level.h"
#include "ReadInit.h"
#include "gameloop.h"
#include "gaussian.h"
#include "timing.h"
#include "registry.h"
#include "ctrlread.h"
#include "object.h"
#include "control.h"
#include "player.h"
#include "Ghost.h"

#include "TitleScreen.h"


// DEBUGGING VARIABLE
#if SHOW_PHYSICS_INFO
long DEBUG_CollGrid = 0;
int DEBUG_NCols = 0;
int DEBUG_LastNCols = 0;
int DEBUG_N2Cols = 0;
VEC DEBUG_dR = {0, 0, 0};
VEC DEBUG_Impulse = {0, 0, 0};
VEC DEBUG_AngImpulse = {0, 0, 0};
VEC DEBUG_SNorm[256];
FACING_POLY DEBUG_Faces[256];
#endif

// globals

char Everything = TRUE;
char NoGamma = FALSE;
char AppRestore = FALSE;
char QuitGame = FALSE;
char FullScreen = TRUE;
unsigned long FrameCount, FrameCountLast, FrameTime, FrameTimeLast, FrameRate;
long EditMode = 0;
char DetailMenuTogg = 0;
REAL TimeFactor;
REAL TimeStep;
REAL EditScale = 1.0f;
HWND hwnd;
HBITMAP TitleHbm;
GAME_SETTINGS GameSettings;
RENDER_SETTINGS	RenderSettings;
char *CarInfoFile = "CarInfo.txt";

static WNDCLASS wcl;
static char WinName[] = "Revolt";
static char AppActive = TRUE;
static char CheatStringBuffer[MAX_CHEAT_STRING_BUFFER + 1];
static long ActivePriority = HIGH_PRIORITY_CLASS;

// cheat strings

char *CheatStrings[] = {
	"yak",

	NULL
};

// event ptr

void (*Event)(void);

/////////////
// WinMain //
/////////////

int WINAPI WinMain(HINSTANCE hThisInst, HINSTANCE hPrevInst, LPSTR lpszArgs, int nWinMode)
{
	char i;
	MSG msg;
	HRESULT r;
	LARGE_INTEGER time;

// parse command line args

	for (i = 1 ; i < __argc ; i++)
	{

// no sound

		if (!strcmp(__argv[i], "-nosound"))
		{
			SoundOff = TRUE;
			continue;
		}

// no colorkey

		if (!strcmp(__argv[i], "-nocolorkey"))
		{
			NoColorKey = TRUE;
			continue;
		}

// carinfo file

		if (!strcmp(__argv[i], "-carinfo"))
		{
			CarInfoFile = __argv[++i];
			continue;
		}

// everything

		if (!strcmp(__argv[i], "-rightaboutnow"))
		{
			Everything = TRUE;
			continue;
		}

// window

		if (!strcmp(__argv[i], "-window"))
		{
			FullScreen = FALSE;
			continue;
		}

// normal priority

		if (!strcmp(__argv[i], "-normalpriority"))
		{
			ActivePriority = NORMAL_PRIORITY_CLASS;
			continue;
		}

// no gamma control

		if (!strcmp(__argv[i], "-paulsgotnewbansheedrivers"))
		{
			NoGamma = TRUE;
			continue;
		}

// no visicock per poly

		if (!strcmp(__argv[i], "-matttneedsvisicockingperpolyturnedoff"))
		{
			VisiPerPoly = FALSE;
			continue;
		}

// edit scale

		if (!strcmp(__argv[i], "-editscale"))
		{
			EditScale = (float)atof(__argv[++i]);
			continue;
		}
	}

// revolt already running?

	if (FindWindow(WinName, WinName) && FullScreen)
	{
		Box(NULL, "Revolt is already running!", MB_OK);
		return FALSE;
	}

// init log file?

#if USE_DEBUG_ROUTINES
	DBG_LogFile = "C:\\Windows\\Temp\\ReVolt.log";
	InitLogFile();
#endif

// Initialize COM library

	r = CoInitialize(NULL);
	if (r != S_OK)
	{
		Box(NULL, "Can't initialize COM library!", MB_OK);
		return FALSE;
	}

// register for lobby support

	LobbyRegister();

// get registry settings

	GetRegistrySettings();

// init window

	if (!InitWin(hThisInst, nWinMode))
		return FALSE;

// find levels

	FindLevels();
	GameSettings.Level = GetLevelNum(RegistrySettings.LevelDir);
	if (GameSettings.Level == -1)
		GameSettings.Level = 0;

// read car info

	if (!ReadAllCarInfo(CarInfoFile))
		return FALSE;

	SetAllCarCoMs();

// create tpage mem

	if (!CreateTPages(TPAGE_NUM))
		return FALSE;

// get timer freq

	QueryPerformanceFrequency(&time);
	TimerFreq = time.LowPart;

// set rand seed

	srand(CurrentTimer());

// get all available draw devices

	GetDrawDevices();

// init DX misc

	InitInput(hThisInst);

	if (!InitDD())
	{
		QuitGame = TRUE;
	}

// init sound system

	InitSound();

// check for legal IP

	#if CHECK_IP
	if (!CheckLegalIP())
	{
		Box(NULL, "Illegal copy of Revolt!", MB_OK);
		QuitGame = TRUE;
	}
	#endif

// set start event

	Event = Go;

// main loop

	while (!QuitGame)
	{

// message?

		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				QuitGame = TRUE;
			}
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}

// handle current event

		if (AppActive)
			Event();
	}

// release DX misc

	DD->RestoreDisplayMode();
	FreeTextures();
	KillInput();
	KillPlay();
	ReleaseD3D();
	ReleaseDX();

// release sound system

	ReleaseSound();

// destroy tpage mem

	DestroyTPages();

// kill car info

	DestroyCarInfo();

// free levels

	FreeLevels();

// save registry settings

	SetRegistrySettings();

// free COM library

	CoUninitialize();

// See if we have forgotten to release anything

#if USE_DEBUG_ROUTINES
	CheckMemoryAllocation();
#endif

// return

	return msg.wParam;
}

//////////////////////////////////////
// define win class, create window  //
// return TRUE if sucessful         //
//////////////////////////////////////

bool InitWin(HINSTANCE hThisInst, int nWinMode)
{
	long bx, by, cy;

// define / register windows class

	wcl.style = CS_HREDRAW | CS_VREDRAW;
	wcl.lpfnWndProc = WindowFunc;
	wcl.hInstance = hThisInst;
	wcl.lpszClassName = WinName;

	wcl.hIcon = NULL;
	wcl.hCursor = LoadCursor(NULL, IDC_CROSS);
	wcl.lpszMenuName = NULL;

	wcl.cbClsExtra = 0;
	wcl.cbWndExtra = 0;
	wcl.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);

	if (!RegisterClass(&wcl)) return FALSE;

// create / show a window

	if (FullScreen)
	{
		hwnd = CreateWindow(WinName, WinName, WS_POPUP,
			0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN),
			NULL, NULL, hThisInst, NULL);
	}
	else
	{
		bx = GetSystemMetrics(SM_CXSIZEFRAME);
		by = GetSystemMetrics(SM_CYSIZEFRAME);
		cy = GetSystemMetrics(SM_CYCAPTION);

		hwnd = CreateWindow(WinName, WinName, WS_OVERLAPPED | WS_CAPTION | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX,
			(GetSystemMetrics(SM_CXSCREEN) - RegistrySettings.ScreenWidth) / 2 - bx, (GetSystemMetrics(SM_CYSCREEN) - RegistrySettings.ScreenHeight) / 2 - by - cy / 2,
			RegistrySettings.ScreenWidth + bx + bx, RegistrySettings.ScreenHeight + by + by + cy,
			NULL, NULL, hThisInst, NULL);
	}

	if (!hwnd) return FALSE;

	ShowWindow(hwnd, nWinMode);
	UpdateWindow(hwnd);
	SetCursor(NULL);

// return ok

	return TRUE;
}

/////////////////////////
// display message box //
/////////////////////////

long Box(char *title, char *mess, long flag)
{
	if (DD)
		DD->FlipToGDISurface();
	return MessageBox(hwnd, mess, title, flag);
}

////////////////////////////
// wait for 'count' vbl's //
////////////////////////////

void Vblank(char count)
{
	for ( ; count ; count--)
	{
		while (DD->WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN, NULL) != DD_OK);
	}
}

/////////////////////
// Win95 callbacks //
/////////////////////

LRESULT CALLBACK WindowFunc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HANDLE handle;

// handle message

	switch (message)
	{

// set AppActive

		case WM_ACTIVATEAPP:
			AppActive = wParam;
			AppRestore = wParam;

			handle = GetCurrentProcess();

			if (AppActive)
			{
				SetPriorityClass(handle, ActivePriority);
				ResumeAllSfx();
			}
			else
			{
				SetPriorityClass(handle, IDLE_PRIORITY_CLASS);
				PauseAllSfx();
			}

			return TRUE;

// no cursor

		case WM_SETCURSOR:
			if (FullScreen)
			{
				SetCursor(NULL);
				return TRUE;
			}
			else
			{
				SetCursor(wcl.hCursor);
				break;
			}

// terminating

		case WM_DESTROY:
			PostQuitMessage(0);

			return TRUE;
	}

// default windows processing

	return DefWindowProc(hwnd, message, wParam, lParam);
}

/////////////
// go game //
/////////////

void Go(void)
{

// setup front end

	Event = GoFront;
}

/////////////////////
// setup front end //
/////////////////////

void GoFront(void)
{

// test linear problem solver??? (eh?)

#if DEBUG_SOLVER
	TestConjGrad();
#endif

// init D3D

	if (!InitD3D(DrawDevices[RegistrySettings.DrawDevice].DisplayMode[DisplayModeCount].Width, DrawDevices[RegistrySettings.DrawDevice].DisplayMode[DisplayModeCount].Height, DrawDevices[RegistrySettings.DrawDevice].DisplayMode[DisplayModeCount].Bpp, 0))
	{
		QuitGame = TRUE;
		return;
	}

	GetTextureFormat(RegistrySettings.TextureBpp);
	InitTextures();

// setup states

	SetupDxState();

// set geom vars

	RenderSettings.GeomPers = BaseGeomPers;
	SetNearFar(48.0f, 4096.0f);
	SetViewport(0, 0, (float)ScreenXsize, (float)ScreenYsize, RenderSettings.GeomPers);

// setup main menu

	LoadMipTexture("gfx\\font1.bmp", TPAGE_FONT, 256, 256, 0, 1);

	LoadBitmap("gfx\\title.bmp", &TitleHbm);

	MenuCount = 0;
	Event = MainMenu;
}

//////////////////
// setup a game //
//////////////////

void SetupGame(void)
{

// kill title bitmap + textures

	FreeBitmap(TitleHbm);
	FreeTextures();

// draw device changed?

	if (RegistrySettings.DrawDevice != (DWORD)CurrentDrawDevice)
	{
		DD->FlipToGDISurface();
		FreeTextures();
		ReleaseD3D();
		InitDD();
	}

// init D3D

	if (!InitD3D(DrawDevices[RegistrySettings.DrawDevice].DisplayMode[DisplayModeCount].Width, DrawDevices[RegistrySettings.DrawDevice].DisplayMode[DisplayModeCount].Height, DrawDevices[RegistrySettings.DrawDevice].DisplayMode[DisplayModeCount].Bpp, 0))
	{
		QuitGame = TRUE;
		return;
	}

	GetTextureFormat(RegistrySettings.TextureBpp);
	InitTextures();

// setup states

	SetupDxState();

// pick texture sets

	if (GameSettings.GameType == GAMETYPE_TRIAL)
		PickTextureSets(2);
	else
		PickTextureSets(MAX_NUM_PLAYERS);

// initialise object grid system

	if (!GRD_AllocGrids())
	{
		QuitGame = TRUE;
	}

// init level

	LEV_InitLevel();

// init players

	if (GameSettings.GameType == GAMETYPE_TRIAL) {
		InitPlayersTrial();
		//InitPlayeresFullArray();
	} else if (GameSettings.GameType == GAMETYPE_SINGLE) {
		InitPlayersSingle();
	} else {
		InitPlayersNetwork();
	}

// Set camera to follow car

	SetCameraFollow(CAM_MainCamera, PLR_LocalPlayer->ownobj, CAM_FOLLOW_BEHIND);

// go game loop

	TotalRaceTime = 0;
	TimeQueue = 0;
	UpdateTimeFactor();
	Event = GLP_GameLoop;
}


/////////////////////////////////////////////////////////////////////
//
// InitPlayersFullArray: create a grid formation including all the
// car available
//
/////////////////////////////////////////////////////////////////////

void InitPlayeresFullArray(void)
{
	int		playerCarID;
	REAL	sep;
	MAT	mat;
	VEC	pos, sepVec;
	PLAYER	*anotherPlayer;

	SetVec(&sepVec, ONE, ZERO, ZERO);
	sep = Real(100);

	for (playerCarID = 0; playerCarID < 6; playerCarID++) {
		
		GetCarGrid(playerCarID, &pos, &mat);
		//VecPlusScalarVec(&LEV_StartPos, playerCarID * sep, &sepVec, &pos);
		anotherPlayer =  PLR_CreatePlayer(PLAYER_LOCAL, CTRL_TYPE_KBD, 1, &pos, &mat);
		if (anotherPlayer == NULL) {
			return;
		}
		if (playerCarID == 0) {
			PLR_LocalPlayer = anotherPlayer;
		}
	}

	// Create the ghost
	GHO_GhostExists = LoadGhostData(&LevelInf[GameSettings.Level]);

	if (GHO_GhostExists)
		playerCarID = GHO_BestGhostInfo->CarID;
	else
		playerCarID = 0;

	GetCarGrid(0, &pos, &mat);
	GHO_GhostPlayer = PLR_CreatePlayer(PLAYER_GHOST, CTRL_TYPE_NONE, playerCarID, &pos, &mat);
	if (GHO_GhostPlayer == NULL)
	{
		Box(NULL, "Can't create ghost player!", MB_OK | MB_ICONERROR);
		QuitGame = TRUE;
		return;
	}

	if (!GHO_GhostExists)
		ClearBestGhostData();

	InitGhostData(PLR_LocalPlayer);
	InitBestGhostData();
	InitGhostLight();

	// get countdown timer
	CountdownEndTime = CurrentTimer() + MS2TIME(COUNTDOWN_START);
	CountdownTime = TRUE;
}

/////////////////////////////
// init time trial players //
/////////////////////////////

void InitPlayersTrial(void)
{
	int playerCarID;
	MAT mat;
	VEC pos;

// Set up local player

	if (GameSettings.CarID < (DWORD)NCarTypes)
		playerCarID = GameSettings.CarID;
	else
		playerCarID = 0;

	if (CurrentJoystick != -1) PLR_LocalCtrlType = CTRL_TYPE_JOY;
	else PLR_LocalCtrlType = CTRL_TYPE_KBD;

	GetCarGrid(0, &pos, &mat);

	PLR_LocalPlayer = PLR_CreatePlayer(PLAYER_LOCAL, PLR_LocalCtrlType, playerCarID, &pos, &mat);
	if (PLR_LocalPlayer == NULL)
	{
		Box(NULL, "Can't create local player!", MB_OK | MB_ICONERROR);
		QuitGame = TRUE;
		return;
	}

// Setup ghost car

	GHO_GhostExists = LoadGhostData(&LevelInf[GameSettings.Level]);

	if (GHO_GhostExists)
		playerCarID = GHO_BestGhostInfo->CarID;
	else
		playerCarID = 0;

	GetCarGrid(0, &pos, &mat);
	GHO_GhostPlayer = PLR_CreatePlayer(PLAYER_GHOST, CTRL_TYPE_NONE, playerCarID, &pos, &mat);
	if (GHO_GhostPlayer == NULL)
	{
		Box(NULL, "Can't create ghost player!", MB_OK | MB_ICONERROR);
		QuitGame = TRUE;
		return;
	}

	if (!GHO_GhostExists)
		ClearBestGhostData();

	InitGhostData(PLR_LocalPlayer);
	InitBestGhostData();
	InitGhostLight();

// get countdown timer

	CountdownEndTime = CurrentTimer() + MS2TIME(COUNTDOWN_START);
	CountdownTime = TRUE;
}

//////////////////////////////
// init single game players //
//////////////////////////////

void InitPlayersSingle(void)
{
	int playerCarID;
	MAT mat;
	VEC pos;

// Set up the local player

	if (GameSettings.CarID < (DWORD)NCarTypes)
		playerCarID = GameSettings.CarID;
	else
		playerCarID = 0;

	if (CurrentJoystick != -1) PLR_LocalCtrlType = CTRL_TYPE_JOY;
	else PLR_LocalCtrlType = CTRL_TYPE_KBD;

	GetCarGrid(0, &pos, &mat);
	PLR_LocalPlayer = PLR_CreatePlayer(PLAYER_LOCAL, CTRL_TYPE_KBD, playerCarID, &pos, &mat);
	if (PLR_LocalPlayer == NULL)
	{
		Box(NULL, "Can't create local player!", MB_OK | MB_ICONERROR);
		QuitGame = TRUE;
		return;
	}

	GetCarGrid(1, &pos, &mat);
	if(!PLR_CreatePlayer(PLAYER_CPU, CTRL_TYPE_CPU, 0, &pos, &mat))
	{
		Box(NULL, "Can't create computer player!", MB_OK | MB_ICONERROR);
		QuitGame = TRUE;
		return;
	}


// no ghost

	GHO_GhostExists = FALSE;
	GHO_GhostPlayer = NULL;

// get countdown timer

	CountdownEndTime = CurrentTimer() + MS2TIME(COUNTDOWN_START);
	CountdownTime = TRUE;
}

//////////////////////////
// init network players //
//////////////////////////

void InitPlayersNetwork(void)
{
	int i;
	PLAYER *player;
	MAT mat;
	VEC pos;
	char buf[256];

// setup local player control type

	if (CurrentJoystick != -1) PLR_LocalCtrlType = CTRL_TYPE_JOY;
	else PLR_LocalCtrlType = CTRL_TYPE_KBD;

// create all players

	for (i = 0 ; i < StartData.PlayerNum ; i++)
	{
		GetCarGrid(StartData.PlayerData[i].GridNum, &pos, &mat);

		if (StartData.PlayerData[i].PlayerID == LocalPlayerID)
			PLR_LocalPlayer = player = PLR_CreatePlayer(PLAYER_LOCAL, PLR_LocalCtrlType, StartData.PlayerData[i].CarID, &pos, &mat);
		else
			player = PLR_CreatePlayer(PLAYER_REMOTE, CTRL_TYPE_NONE, StartData.PlayerData[i].CarID, &pos, &mat);

		if (!player)
		{
			wsprintf(buf, "Can't create player %s", StartData.PlayerData[i].Name);
			Box(NULL, buf, MB_OK);
			QuitGame = TRUE;
			return;
		}

		player->PlayerID = StartData.PlayerData[i].PlayerID;
		strncpy(player->PlayerName, StartData.PlayerData[i].Name, MAX_PLAYER_NAME);
	}

// no ghost

	GHO_GhostExists = FALSE;
	GHO_GhostPlayer = NULL;

// get countdown timer

//	RemoteSync();
	CountdownEndTime = CurrentTimer() + MS2TIME(COUNTDOWN_START);
	CountdownTime = TRUE;
}

/////////////////////////
// check cheat strings //
/////////////////////////

void CheckCheatStrings(void)
{
	long flag;
	unsigned char ch;

// update buffer

	ch = GetKeyPress();
	if (!ch) return;

	memcpy(CheatStringBuffer, CheatStringBuffer + 1, MAX_CHEAT_STRING_BUFFER - 1);
	CheatStringBuffer[MAX_CHEAT_STRING_BUFFER - 1] = ch;
	CheatStringBuffer[MAX_CHEAT_STRING_BUFFER] = 0;

// look for cheat string

	flag = 0;

	while (1)
	{
		if (!CheatStrings[flag])
			return;

		if (!strcmp(&CheatStringBuffer[16 - strlen(CheatStrings[flag])], CheatStrings[flag]))
			break;

		flag++;
	}

// act

	ZeroMemory(CheatStringBuffer, MAX_CHEAT_STRING_BUFFER);
	PlaySfx(SFX_HONK, SFX_MAX_VOL, SFX_CENTRE_PAN, 22050);

	switch (flag)
	{
		case 0:
			Everything = !Everything;
		break;
	}
}
