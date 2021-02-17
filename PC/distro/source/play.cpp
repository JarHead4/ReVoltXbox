
#include <winsock.h>
#include "revolt.h"
#include "dx.h"
#include "dxerrors.h"
#include "main.h"
#include "text.h"
#include "model.h"
#include "particle.h"
#include "aerial.h"
#include "NewColl.h"
#include "Body.h"
#include "car.h"
#include "ctrlread.h"
#include "object.h"
#include "control.h"
#include "player.h"
#include "play.h"
#include "input.h"
#include "draw.h"
#include "registry.h"
#include "geom.h"
#include "move.h"
#include "timing.h"


// globals

IDirectPlay4A *DP = NULL;
IDirectPlayLobby3 *Lobby = NULL;
DPID FromID, ToID, LocalPlayerID, ServerID;
DP_PLAYER PlayerList[MAX_NUM_PLAYERS];
long PlayerCount;
char ReceiveBuff[1024], TransmitBuff[1024];
START_DATA StartData;

static char SessionName[MAX_SESSION_NAME];
static unsigned char IP[4];
static char ConnectionCount, SessionCount;
static DP_PLAYER_DATA LocalPlayerData;
static DPSESSIONDESC2 Session, SessionEnum, SessionJoin;
static DP_SESSION SessionList[SESSION_MAX];
static DP_CONNECTION Connection[CONNECTION_MAX];
static char HostName[MAX_HOST_NAME];
static HANDLE PlayerEvent = NULL;
static long GameStarted, ReadyFlag;
static float PingRequestTime, SessionRequestTime;

// allowed IP addresses

LEGAL_IP LegalIP[] = {
	{255, 255, 255, 0, 100, 103, 0, 0},		// Iguana Texas
//	{255, 255, 255, 0, 100, 104, 0, 0},		// Sculptured US
	{255, 255, 255, 0, 100, 105, 0, 0},		// Acclaim UK
	{255, 255, 255, 0, 100, 106, 0, 0},		// Probe
//	{255, 255, 255, 0, 100, 107, 0, 0},		// Iguana UK 0
//	{255, 255, 255, 0, 100, 107, 1, 0},		// Iguana UK 1
//	{255, 255, 255, 0, 100, 107, 2, 0},		// Iguana UK 2
//	{255, 255, 255, 0, 100, 108, 0, 0},		// Acclaim Germany
//	{255, 255, 255, 0, 100, 109, 0, 0},		// Acclaim France

	{255, 255, 255, 0, 38, 240, 105, 0},	// Acclaim red network
	{255, 255, 255, 0, 100, 0, 7, 0},		// Acclaim NY
	{255, 255, 255, 0, 100, 0, 5, 0},		// Acclaim NY
	{255, 255, 255, 0, 194, 129, 18, 0},	// Probe

	{255, 255, 255, 255, 100, 0, 5, 70},	// Doug Yellin

	{0, 0, 0, 0}
};

// play GUID

GUID DP_GUID = {0x6bb78285, 0x71df, 0x11d2, 0xb4, 0x6c, 0xc, 0x78, 0xc, 0xc1, 0x8, 0x40};

////////////////////////////////
// register for lobby support //
////////////////////////////////

void LobbyRegister(void)
{
	HRESULT r;
	DPAPPLICATIONDESC app;
	char dir[256];

// create lobby object

	r = CoCreateInstance(CLSID_DirectPlayLobby, NULL, CLSCTX_INPROC_SERVER, IID_IDirectPlayLobby3A, (void**)&Lobby);
	if (r != DP_OK)
	{
		ErrorDX(r, "Can't create DirectPlay Lobby object");
		QuitGame = TRUE;
		return;
	}

// register

	GetCurrentDirectory(256, dir);

	app.dwSize = sizeof(app);
	app.dwFlags = 0;
	app.lpszApplicationNameA = "Revolt";
	app.guidApplication = DP_GUID;
	app.lpszFilenameA = "revolt.exe";
	app.lpszCommandLineA = "";
	app.lpszPathA = dir;
	app.lpszCurrentDirectoryA = dir;
	app.lpszDescriptionA = NULL;
	app.lpszDescriptionW = NULL;

	r = Lobby->RegisterApplication(0, &app);
	if (r != DP_OK)
	{
		ErrorDX(r, "Can't register for lobby support");
		QuitGame = TRUE;
		return;
	}

// release

	RELEASE(Lobby);
}

/////////////////////
// init directplay //
/////////////////////

bool InitPlay(void)
{
	HRESULT r;
	char i;

// Create an IDirectPlay4 interface

	r = CoCreateInstance(CLSID_DirectPlay, NULL, CLSCTX_INPROC_SERVER, IID_IDirectPlay4A, (void**)&DP);
	if (r != DP_OK)
	{
		ErrorDX(r, "Can't create DirectPlay object");
		QuitGame = TRUE;
		return FALSE;
	}

// enum connection types

	ConnectionCount = 0;
	for (i = 0 ; i < CONNECTION_MAX ; i++) Connection[i].Ptr = NULL;

	r = DP->EnumConnections(&DP_GUID, EnumConnectionsCallback, NULL, DPCONNECTION_DIRECTPLAY);
	if (r != DP_OK)
	{
		ErrorDX(r, "Can't enumerate DirectPlay connections");
		QuitGame = TRUE;
		return FALSE;
	}

// create player event handle

	PlayerEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (!PlayerEvent)
	{
		ErrorDX(0, "Can't create player event!");
		QuitGame = TRUE;
		return FALSE;
	}

// return OK

	return TRUE;
}

/////////////////////
// kill directplay //
/////////////////////

void KillPlay(void)
{
	char i;

// free player event handle

	CloseHandle(PlayerEvent);

// free connection ptr's

	for (i = 0 ; i < CONNECTION_MAX ; i++) if (Connection[i].Ptr)
	{
		free(Connection[i].Ptr);
		Connection[i].Ptr = NULL;
	}

// release DP object

	RELEASE(DP);
}

//////////////////////////////
// EnumConnections callback //
//////////////////////////////

BOOL FAR PASCAL EnumConnectionsCallback(LPCGUID lpguidSP, LPVOID lpConnection, DWORD dwConnectionSize, LPCDPNAME lpName, DWORD dwFlags, LPVOID lpContext)
{

// skip if reached max connections

	if (ConnectionCount >= CONNECTION_MAX) return FALSE;

// store connection name / info

	memcpy(Connection[ConnectionCount].Name, lpName->lpszShortNameA, 128);
	Connection[ConnectionCount].Ptr = malloc(dwConnectionSize);
	memcpy(Connection[ConnectionCount].Ptr, lpConnection, dwConnectionSize);

// return OK

	ConnectionCount++;
	return TRUE;
}

///////////////////////
// init a connection //
///////////////////////

bool InitConnection(char num)
{
	HRESULT r;

// init connection

	r = DP->InitializeConnection(Connection[num].Ptr, 0);
	if (r != DP_OK)
	{
		ErrorDX(r, "Can't init DirectPlay connection");
		QuitGame = TRUE;
		return FALSE;
	}

// return OK

	return TRUE;
}

//////////////////////
// create a session //
//////////////////////

bool CreateSession(char *name)
{
	HRESULT r;

// setup a new session

	ZeroMemory(&Session, sizeof(Session));
	Session.dwSize = sizeof(Session);
//	Session.dwFlags = DPSESSION_DIRECTPLAYPROTOCOL | DPSESSION_MIGRATEHOST | DPSESSION_KEEPALIVE;
	Session.dwFlags = DPSESSION_DIRECTPLAYPROTOCOL | DPSESSION_MULTICASTSERVER | DPSESSION_KEEPALIVE;
	Session.guidApplication = DP_GUID;
	Session.dwMaxPlayers = MAX_NUM_PLAYERS;
	Session.lpszSessionNameA = name;

// open session

	r = DP->Open(&Session, DPOPEN_CREATE);
	if (r != DP_OK)
	{
		ErrorDX(r, "Can't create DirectPlay session");
		QuitGame = TRUE;
		return FALSE;
	}

// return OK

	return TRUE;
}

////////////////////
// join a session //
////////////////////

bool JoinSession(char num)
{
	HRESULT r;

// setup a session

	ZeroMemory(&SessionJoin, sizeof(SessionJoin));
	SessionJoin.dwSize = sizeof(SessionJoin);
	SessionJoin.guidInstance = SessionList[num].Guid;

// open session

	r = DP->Open(&SessionJoin, DPOPEN_JOIN);
	if (r != DP_OK)
	{
		ErrorDX(r, "Can't join DirectPlay session");
		QuitGame = TRUE;
		return FALSE;
	}

// return OK

	return TRUE;
}

///////////////////
// list sessions //
///////////////////

void ListSessions(void)
{
	SessionCount = 0;

	ZeroMemory(&SessionEnum, sizeof(SessionEnum));
	SessionEnum.dwSize = sizeof(SessionEnum);
	SessionEnum.guidApplication = DP_GUID;

	DP->EnumSessions(&SessionEnum, 0, EnumSessionsCallback, NULL, DPENUMSESSIONS_ASYNC | DPENUMSESSIONS_ALL);
}

//////////////////////////////
// stop session enumeration //
//////////////////////////////

void StopSessionEnum(void)
{
	ZeroMemory(&SessionEnum, sizeof(SessionEnum));
	SessionEnum.dwSize = sizeof(SessionEnum);
	SessionEnum.guidApplication = DP_GUID;

	DP->EnumSessions(&SessionEnum, 0, EnumSessionsCallback, NULL, DPENUMSESSIONS_STOPASYNC);
}

///////////////////////////
// EnumSessions callback //
///////////////////////////

BOOL FAR PASCAL EnumSessionsCallback(LPCDPSESSIONDESC2 lpSessionDesc, LPDWORD lpdwTimeOut, DWORD dwFlags, LPVOID lpContext)
{

// skip if reached max sessions

	if (SessionCount >= SESSION_MAX) return FALSE;

// skip if timed out

	if (dwFlags & DPESC_TIMEDOUT) return FALSE;

// store session name / info

	memcpy(SessionList[SessionCount].Name, lpSessionDesc->lpszSessionNameA, MAX_SESSION_NAME);
	SessionList[SessionCount].Guid = lpSessionDesc->guidInstance;
	SessionList[SessionCount].Flags = lpSessionDesc->dwFlags;

// return OK

	SessionCount++;
	return TRUE;
}

/////////////////////
// create a player //
/////////////////////

bool CreatePlayer(char *name, long server)
{
	DPNAME dpname;
	HRESULT r;

// setup a name

	ZeroMemory(&dpname, sizeof(dpname));
	dpname.dwSize = sizeof(dpname);
	dpname.lpszShortNameA = name;
	dpname.lpszLongNameA = NULL;

// setup player data

	LocalPlayerData.CarID = GameSettings.CarID;

// create a player

	r = DP->CreatePlayer(&LocalPlayerID, &dpname, PlayerEvent, &LocalPlayerData, sizeof(LocalPlayerData), 0);
	if (r != DP_OK)
	{
		ErrorDX(r, "Can't create a player");
		QuitGame = TRUE;
		return FALSE;
	}

// return OK

	return TRUE;
}

//////////////////////////
// display player names //
//////////////////////////

void DisplayPlayers(void)
{
	short j;
	PLAYER *player;

	j = 128;
	for (player = PLR_PlayerHead ; player ; player = player->next)
	{
		DumpText(0, j, 8, 16, 0x000080, player->PlayerName);
		j += 16;
	}
}

////////////////////
// send a message //
////////////////////

void TransmitMessage(char *buff, short size, DPID to, long pri)
{
	HRESULT r;

	r = DP->SendEx(LocalPlayerID, to, DPSEND_ASYNC | DPSEND_NOSENDCOMPLETEMSG, buff, size, pri, 0, NULL, NULL);
//	r = DP->SendEx(LocalPlayerID, to, 0, buff, size, pri, 0, NULL, NULL);
}

////////////////////
// send a message //
////////////////////

void TransmitMessageGuaranteed(char *buff, short size, DPID to, long pri)
{
	HRESULT r;

	r = DP->SendEx(LocalPlayerID, to, DPSEND_GUARANTEED, buff, size, pri, 0, NULL, NULL);
}

/////////////////////////////////////////
// cancel messages of a given priority //
/////////////////////////////////////////

void CancelPriority(long pri)
{
	HRESULT r;

	r = DP->CancelPriority(pri, pri, 0);
	if (r != DP_OK)
		ErrorDX(r, "can't cancel priority!");
}

//////////////////////
// receive a packet //
//////////////////////

char GetRemoteMessages(void)
	{
	HRESULT r;
	DWORD size;
	char flag = FALSE;

// get all messages

	do
	{

// get one

		size = 1024;
		r = DP->Receive(&FromID, &ToID, DPRECEIVE_ALL, ReceiveBuff, &size);

// valid message?

		if (r == DP_OK)
		{

// set 'got message' flag

			flag = TRUE;

// system message?

			if (FromID == DPID_SYSMSG)
			{
				ProcessSystemMessage();
			}

// personal message?

			else
			{
				ProcessPersonalMessage();
			}
		}

// next message

	} while (r != DPERR_NOMESSAGES);

// return

	return flag;
}

//////////////////////////////
// process personal message //
//////////////////////////////

void ProcessPersonalMessage(void)
{
	long i, flag;
	char buf[128];
	MESSAGE_HEADER *header = (MESSAGE_HEADER*)ReceiveBuff;

// act on message

	switch (header->Type)
	{

// game started

		case MESSAGE_GAME_STARTED:

			GameStarted = TRUE;
			ServerID = FromID;

			StartData = *(START_DATA*)(header + 1);

			flag = FALSE;
			for (i = 0 ; i < GameSettings.LevelNum ; i++)
			{
				if (!strcmp(StartData.LevelDir, LevelInf[i].Dir))
				{
					GameSettings.Level = i;
					flag = TRUE;
					break;
				}
			}

			if (!flag)
			{
				wsprintf(buf, "Can't find Level directory '%s'", StartData.LevelDir);
				Box(NULL, buf, MB_OK);
				QuitGame = TRUE;
			}

		break;

// car data

		case MESSAGE_CAR_DATA:
			ProcessCarMessage();
		break;

// ping request

		case MESSAGE_PING_REQUEST:
			ProcessPingRequest();
		break;

// ping return

		case MESSAGE_PING_RETURN:
			ProcessPingReturn();
		break;

// client ready

		case MESSAGE_PLAYER_READY:
			ProcessPlayerReady();
		break;

// syncing timers 1

		case MESSAGE_SYNC_TIMERS1:
			ProcessSyncTimers1();
		break;

// syncing timers 2

		case MESSAGE_SYNC_TIMERS2:
			ProcessSyncTimers2();
		break;

// syncing timers 3

		case MESSAGE_SYNC_TIMERS3:
			ProcessSyncTimers3();
		break;
	}
}

//////////////////
// get car data //
//////////////////

void ProcessCarMessage()
{
	REMOTE_DATA *rem;
	CAR *car;
	PLAYER *player;
	MESSAGE_HEADER *header = (MESSAGE_HEADER*)ReceiveBuff;
	short *sh;
	char *ptr = (char*)(header + 1);

// get relevant car

	for (player = PLR_PlayerHead ; player ; player = player->next) if ((player->type == PLAYER_REMOTE) && (FromID == player->PlayerID))
	{
		car = &player->car;
		break;
	}

// get remote data struct to fill

	rem = NextRemoteData(car);

	rem->PacketInfo = 0;

// set packet arrival time

	rem->PacketInfo |= REMOTE_TIME;

// get pos?

	if (header->Contents & MESSAGE_CONTENTS_CAR_POS)
	{
		CopyVec((VEC*)ptr, &rem->Pos);
		rem->PacketInfo |= REMOTE_POS;
		ptr += sizeof(VEC);
	}

// get quat?

	if (header->Contents & MESSAGE_CONTENTS_CAR_QUAT)
	{
		rem->Quat.v[VX] = ((REAL)*ptr++ / CAR_REMOTE_QUAT_SCALE);
		rem->Quat.v[VY] = ((REAL)*ptr++ / CAR_REMOTE_QUAT_SCALE);
		rem->Quat.v[VZ] = ((REAL)*ptr++ / CAR_REMOTE_QUAT_SCALE);
		rem->Quat.v[S] = ((REAL)*ptr++ / CAR_REMOTE_QUAT_SCALE);
		NormalizeQuat(&rem->Quat);
		rem->PacketInfo |= REMOTE_QUAT;
	}

// get vel?

	if (header->Contents & MESSAGE_CONTENTS_CAR_VEL)
	{
		sh = (short*)ptr;
		rem->Vel.v[X] = ((REAL)*sh++ / CAR_REMOTE_VEL_SCALE);
		rem->Vel.v[Y] = ((REAL)*sh++ / CAR_REMOTE_VEL_SCALE);
		rem->Vel.v[Z] = ((REAL)*sh++ / CAR_REMOTE_VEL_SCALE);
		rem->PacketInfo |= REMOTE_VEL;
		ptr = (char*)sh;
	}

// get ang vel?

	if (header->Contents & MESSAGE_CONTENTS_CAR_ANGVEL)
	{
		sh = (short*)ptr;
		rem->AngVel.v[X] = ((REAL)*sh++ / CAR_REMOTE_ANGVEL_SCALE);
		rem->AngVel.v[Y] = ((REAL)*sh++ / CAR_REMOTE_ANGVEL_SCALE);
		rem->AngVel.v[Z] = ((REAL)*sh++ / CAR_REMOTE_ANGVEL_SCALE);
		rem->PacketInfo |= REMOTE_ANGVEL;
		ptr = (char*)sh;

	}

// get control input?

	if (header->Contents & MESSAGE_CONTENTS_CAR_CONTROL)
	{
		rem->dx = *ptr++;
		rem->dy = *ptr++;
		rem->PacketInfo |= REMOTE_CONTROL;
	}

	rem->NewData = TRUE;

}

/////////////////////////////////////////
// request ping times from each player //
/////////////////////////////////////////

void RequestPings(void)
{
	long i;
	MESSAGE_HEADER *header = (MESSAGE_HEADER*)TransmitBuff;
	unsigned long *ptr = (unsigned long*)(header + 1);

// zero local player ping

	for (i = 0 ; i < PlayerCount ; i++)
	{
		if (PlayerList[i].PlayerID == LocalPlayerID)
			PlayerList[i].Ping = 0;
	}

// setup ping packet

	header->Type = MESSAGE_PING_REQUEST;
	ptr[0] = CurrentTimer();

// send

	TransmitMessage(TransmitBuff, sizeof(MESSAGE_HEADER) + sizeof(long), DPID_ALLPLAYERS, MESSAGE_PRIORITY_NORMAL);
}

//////////////////////////
// process ping request //
//////////////////////////

void ProcessPingRequest(void)
{
	MESSAGE_HEADER *rheader = (MESSAGE_HEADER*)ReceiveBuff;
	MESSAGE_HEADER *theader = (MESSAGE_HEADER*)TransmitBuff;
	long *rptr = (long*)(rheader + 1);
	long *tptr = (long*)(theader + 1);

// setup return packet

	theader->Type = MESSAGE_PING_RETURN;
	tptr[0] = rptr[0];

// send

	TransmitMessage(TransmitBuff, sizeof(MESSAGE_HEADER) + sizeof(long) * 2, FromID, MESSAGE_PRIORITY_NORMAL);
}

/////////////////////////
// process ping return //
/////////////////////////

void ProcessPingReturn(void)
{
	MESSAGE_HEADER *header = (MESSAGE_HEADER*)ReceiveBuff;
	long i;
	unsigned long ping, *ptr = (unsigned long*)(header + 1);

// get ping time

	ping = TIME2MS(CurrentTimer() - ptr[0]);

// set player ping

	for (i = 0 ; i < PlayerCount ; i++) if (PlayerList[i].PlayerID == FromID)
	{
		PlayerList[i].Ping = ping;
	}
}

//////////////////////////
// process player ready //
//////////////////////////

void ProcessPlayerReady(void)
{
	PLAYER *player;

// set player ready flag

	if (!ReadyFlag)
		return;

	for (player = PLR_PlayerHead ; player ; player = player->next) if ((player->type == PLAYER_REMOTE) && (FromID == player->PlayerID))
	{
		player->Ready = TRUE;
	}
}

///////////////////////////
// process sync timers 1 //
///////////////////////////

void ProcessSyncTimers1(void)
{
	MESSAGE_HEADER *rh = (MESSAGE_HEADER*)ReceiveBuff;
	MESSAGE_HEADER *th = (MESSAGE_HEADER*)TransmitBuff;
	unsigned long *rptr = (unsigned long*)(rh + 1);
	unsigned long *tptr = (unsigned long*)(th + 1);

// set everyone ready flag

	ReadyFlag = TRUE;

// send back server timer with client timer

	th->Type = MESSAGE_SYNC_TIMERS2;
	tptr[0] = rptr[0];
	tptr[1] = CurrentTimer();

	TransmitMessage(TransmitBuff, sizeof(MESSAGE_HEADER) + sizeof(unsigned long) * 2, FromID, MESSAGE_PRIORITY_NORMAL);
}

///////////////////////////
// process sync timers 2 //
///////////////////////////

void ProcessSyncTimers2(void)
{
	MESSAGE_HEADER *rh = (MESSAGE_HEADER*)ReceiveBuff;
	MESSAGE_HEADER *th = (MESSAGE_HEADER*)TransmitBuff;
	unsigned long *rptr = (unsigned long*)(rh + 1);
	unsigned long *tptr = (unsigned long*)(th + 1);

// calc server to client ping in MS, send it back with client timer

	th->Type = MESSAGE_SYNC_TIMERS3;
	tptr[0] = TIME2MS(CurrentTimer() - rptr[0]);
	tptr[1] = rptr[1];

	TransmitMessage(TransmitBuff, sizeof(MESSAGE_HEADER) + sizeof(unsigned long) * 2, FromID, MESSAGE_PRIORITY_NORMAL);
}

///////////////////////////
// process sync timers 3 //
///////////////////////////

void ProcessSyncTimers3(void)
{
	MESSAGE_HEADER *header = (MESSAGE_HEADER*)ReceiveBuff;
	unsigned long *ptr = (unsigned long*)(header + 1), ping;

// guess total 3-way ping

	ping = TIME2MS(CurrentTimer() - ptr[1]) + (ptr[0] / 2);

// set countdown timer

	CountdownEndTime = CurrentTimer() + MS2TIME(COUNTDOWN_START - ping);
	CountdownTime = TRUE;

// set ready

	ReadyFlag = TRUE;
}

////////////////////////////
// process system message //
////////////////////////////

void ProcessSystemMessage(void)
{
	DPMSG_GENERIC *Message = (DPMSG_GENERIC*)ReceiveBuff;
	DPMSG_CREATEPLAYERORGROUP *Player;
	PLAYER *player, *next;

	switch (Message->dwType)
	{

// new player joining

		case DPSYS_CREATEPLAYERORGROUP:
		break;

// existing player leaving

		case DPSYS_DESTROYPLAYERORGROUP:
			Player = (DPMSG_CREATEPLAYERORGROUP*)ReceiveBuff;
			if (Player->dwPlayerType == DPPLAYERTYPE_PLAYER)
			{
				for (player = PLR_PlayerHead ; player ; )
				{
					next = player->next;
					if ((player->type == PLAYER_REMOTE) && player->PlayerID == Player->dpId)
					{
						PLR_KillPlayer(player);
					}
					player = next;
				}
			}
 		break;

// session lost

		case DPSYS_SESSIONLOST:
			Box(NULL, "Session lost!", MB_OK);
			GameSettings.GameType = GAMETYPE_SESSIONLOST;
 		break;

// become the host

		case DPSYS_HOST:
			GameSettings.GameType = GAMETYPE_SERVER;
			ServerID = LocalPlayerID;
 		break;
	}
}

//////////////////
// list players //
//////////////////

void ListPlayers(GUID *guid)
{
	PlayerCount = 0;
	if (!guid) DP->EnumPlayers(NULL, EnumPlayersCallback, NULL, DPENUMPLAYERS_ALL);
	else DP->EnumPlayers(guid, EnumPlayersCallback, NULL, DPENUMPLAYERS_SESSION);
}

//////////////////////////
// EnumPlayers callback //
//////////////////////////

BOOL FAR PASCAL EnumPlayersCallback(DPID dpId, DWORD dwPlayerType, LPCDPNAME lpName, DWORD dwFlags, LPVOID lpContext)
{
	DWORD size;

// skip if max players

	if (PlayerCount >= MAX_NUM_PLAYERS) return FALSE;

// store ID / name

	PlayerList[PlayerCount].PlayerID = dpId;
	strncpy(PlayerList[PlayerCount].Name, lpName->lpszShortNameA, MAX_PLAYER_NAME);
	DP->GetPlayerCaps(PlayerList[PlayerCount].PlayerID, &PlayerList[PlayerCount].Caps, NULL);

// get player data

	size = sizeof(DP_PLAYER_DATA);
	DP->GetPlayerData(PlayerList[PlayerCount].PlayerID, &PlayerList[PlayerCount].Data, &size, DPGET_REMOTE);

// return OK

	PlayerCount++;
	return TRUE;
}

///////////////////////////////
// choose network connection //
///////////////////////////////

void ConnectionMenu(void)
{
	short i;
	long col;

// buffer flip / clear

	CheckSurfaces();
	FlipBuffers();
	ClearBuffers();

// update pos

	ReadMouse();
	ReadKeyboard();

	if (Keys[DIK_UP] && !LastKeys[DIK_UP] && MenuCount) MenuCount--;
	if (Keys[DIK_DOWN] && !LastKeys[DIK_DOWN] && MenuCount < ConnectionCount - 1) MenuCount++;

// show menu

	D3Ddevice->BeginScene();

	BlitBitmap(TitleHbm, &BackBuffer);

	BeginTextState();

	DumpText(128, 112, 12, 24, 0x808000, "Select Connection:");

	for (i = 0 ; i < ConnectionCount ; i++)
	{
		if (MenuCount == i) col = 0xff0000;
		else col = 0x808080;
		DumpText(128, i * 48 + 176, 12, 24, col, Connection[i].Name);
	}

	D3Ddevice->EndScene();

// selected?

	if (Keys[DIK_ESCAPE] && !LastKeys[DIK_ESCAPE])
	{
		KillPlay();
		MenuCount = 0;
		Event = MainMenu;
	}		

	if (Keys[DIK_RETURN] && !LastKeys[DIK_RETURN])
	{
		InitConnection((char)MenuCount);

		if (GameSettings.GameType == GAMETYPE_SERVER)
		{
			MenuCount = 0;
			Event = GetSessionName;
		}

		else
		{
			MenuCount = 0;
			SessionCount = 0;
			SessionRequestTime = 0.0f;
			Event = LookForSessions;
		}
	}
}

//////////////////////
// get session name //
//////////////////////

void GetSessionName(void)
{
	unsigned char c;

// buffer flip / clear

	CheckSurfaces();
	FlipBuffers();
	ClearBuffers();

// get a key

	ReadMouse();
	ReadKeyboard();

	if ((c = GetKeyPress()))
	{

// backspace

		if (c == 8)
		{
			if (MenuCount) MenuCount--;
		}

// tab

		else if (c == 9)
		{
			MenuCount = 0;
		}

// enter

		else if (c == 13)
		{
			SessionName[MenuCount] = 0;
			CreateSession(SessionName);
			CreatePlayer(RegistrySettings.PlayerName, TRUE);
			PingRequestTime = 0.0f;
			Event = HostWait;
		}

// escape

		else if (c == 27)
		{
			MenuCount = 0;
			KillPlay();
			InitPlay();
			Event = ConnectionMenu;
		}

// normal key

		else if (MenuCount < MAX_SESSION_NAME - 2)
		{
			SessionName[MenuCount++] = c;
		}
	}

// print name

	D3Ddevice->BeginScene();

	BlitBitmap(TitleHbm, &BackBuffer);

	BeginTextState();

	DumpText(208, 224, 12, 24, 0x808000, "Enter Game Name:");
	SessionName[MenuCount] = '_';
	SessionName[MenuCount + 1] = 0;
	DumpText(128, 276, 12, 24, 0x808080, SessionName);

	D3Ddevice->EndScene();
}

///////////////////////////////
// look for / join a session //
///////////////////////////////

void LookForSessions(void)
{
	short i;
	long col;
	char state[7];

// buffer flip / clear

	CheckSurfaces();
	FlipBuffers();
	ClearBuffers();

// read keyboard / mouse / timers

	UpdateTimeFactor();
	ReadMouse();
	ReadKeyboard();

// dump back piccy

	BlitBitmap(TitleHbm, &BackBuffer);

// begin scene

	D3Ddevice->BeginScene();

// request sessions?

	SessionRequestTime -= TimeStep;
	if (SessionRequestTime < 0.0f)
	{
		ListSessions();
		SessionRequestTime = 2.0f;

		if (SessionCount)
		{
			if (MenuCount > SessionCount - 1) MenuCount = SessionCount - 1;
			ListPlayers(&SessionList[MenuCount].Guid);
		}
	}

// display sessions

	BeginTextState();

	DumpText(264, 16, 8, 16, 0x808000, "Choose a Game:");

	for (i = 0 ; i < SessionCount ; i++)
	{
		if (MenuCount == i) col = 0xff0000;
		else col = 0x808080;
		DumpText(168, i * 16 + 48, 8, 16, col, SessionList[i].Name);
		if (SessionList[i].Flags & DPSESSION_JOINDISABLED) memcpy(state, "CLOSED", sizeof(state));
		else memcpy(state, "OPEN", sizeof(state));
		DumpText(432, i * 16 + 48, 8, 16, 0x808000, state);
	}

// list players in selected session

	if (!SessionCount) PlayerCount = 0;

	if (PlayerCount)
	{
		DumpText(288, 192, 8, 16, 0x808000, "Players:");
		for (i = 0 ; i < PlayerCount ; i++)
		{
			DumpText(168, i * 16 + 224, 8, 16, 0xff0000, PlayerList[i].Name);
		}
	}

// end scene

	D3Ddevice->EndScene();

// up / down

	if (Keys[DIK_UP] && !LastKeys[DIK_UP] && MenuCount) MenuCount--;
	if (Keys[DIK_DOWN] && !LastKeys[DIK_DOWN]) MenuCount++;
	if (SessionCount && MenuCount >= SessionCount) MenuCount = SessionCount - 1;

// quit

	if (Keys[DIK_ESCAPE] && !LastKeys[DIK_ESCAPE])
	{
		StopSessionEnum();
		KillPlay();
		InitPlay();
		MenuCount = 0;
		Event = ConnectionMenu;
	}

// join

	if (Keys[DIK_RETURN] && !LastKeys[DIK_RETURN] && SessionCount && !(SessionList[MenuCount].Flags & DPSESSION_JOINDISABLED))
	{
		JoinSession((char)MenuCount);
		CreatePlayer(RegistrySettings.PlayerName, FALSE);
		GameStarted = FALSE;
		PingRequestTime = 0.0f;
		Event = ClientWait;
	}
}

//////////////////////////////
// host waiting for players //
//////////////////////////////

void HostWait(void)
{
	long i, j, k, gridused[MAX_NUM_PLAYERS];
	MESSAGE_HEADER *header;
	char buf[128];

// buffer flip / clear

	CheckSurfaces();
	FlipBuffers();
	ClearBuffers();

// read keyboard / mouse / timers

	UpdateTimeFactor();
	ReadMouse();
	ReadKeyboard();

// display current players

	D3Ddevice->BeginScene();

	BlitBitmap(TitleHbm, &BackBuffer);

	BeginTextState();

	DumpText(288, 64, 8, 16, 0x808000, "Players:");
	DumpText(216, 400, 8, 16, 0x808000, "Hit Enter To Start Game...");

	ListPlayers(NULL);

	PingRequestTime -= TimeStep;
	if (PingRequestTime < 0.0f)
	{
		RequestPings();
		PingRequestTime = 2.0f;
	}

	GetRemoteMessages();

	for (i = 0 ; i < PlayerCount ; i++)
	{
		DumpText(192, i * 16 + 96, 8, 16, 0xff0000, PlayerList[i].Name);
		wsprintf(buf, "%ld", PlayerList[i].Ping);
		DumpText(448, i * 16 + 96, 8, 16, 0xff0000, buf);
	}

	D3Ddevice->EndScene();

// quit?

	if (Keys[DIK_ESCAPE] && !LastKeys[DIK_ESCAPE])
	{
		DP->Close();
		Event = GetSessionName;
	}

// start game?

	if (Keys[DIK_RETURN] && !LastKeys[DIK_RETURN])
	{

// yep, disable game for new players

		Session.dwFlags |= DPSESSION_JOINDISABLED;
		DP->SetSessionDesc(&Session, NULL);

// setup start data

		StartData.PlayerNum = PlayerCount;
		strncpy(StartData.LevelDir, LevelInf[GameSettings.Level].Dir, MAX_LEVEL_DIR_NAME);

		for (i = 0 ; i < StartData.PlayerNum ; i++)
			gridused[i] = FALSE;

		for (i = 0 ; i < StartData.PlayerNum ; i++)
		{
			StartData.PlayerData[i].PlayerID = PlayerList[i].PlayerID;
			StartData.PlayerData[i].CarID = PlayerList[i].Data.CarID;
			strncpy(StartData.PlayerData[i].Name, PlayerList[i].Name, MAX_PLAYER_NAME);

			k = (rand() % (StartData.PlayerNum - i)) + 1;
			for (j = 0 ; j < StartData.PlayerNum ; j++)
			{
				if (!gridused[j])
					k--;

				if (!k)
				{
					StartData.PlayerData[i].GridNum = j;
					gridused[j] = TRUE;
					break;
				}
			}
		}

// go!

		header = (MESSAGE_HEADER*)TransmitBuff;
		header->Type = MESSAGE_GAME_STARTED;

		memcpy(header + 1, &StartData, sizeof(START_DATA));

		TransmitMessageGuaranteed(TransmitBuff, sizeof(MESSAGE_HEADER) + sizeof(START_DATA), DPID_ALLPLAYERS, MESSAGE_PRIORITY_NORMAL);
		ReadyFlag = FALSE;

		Event = SetupGame;
	}
}

/////////////////////////////
// client waiting for host //
/////////////////////////////

void ClientWait(void)
{
	short i;
	char buf[128];

// buffer flip / clear

	CheckSurfaces();
	FlipBuffers();
	ClearBuffers();

// read keyboard / mouse / timers

	UpdateTimeFactor();
	ReadMouse();
	ReadKeyboard();

// display current players

	D3Ddevice->BeginScene();

	BlitBitmap(TitleHbm, &BackBuffer);

	BeginTextState();

	DumpText(288, 64, 8, 16, 0x808000, "Players:");
	DumpText(240, 400, 8, 16, 0x808000, "Waiting For Host...");

	ListPlayers(NULL);

	PingRequestTime -= TimeStep;
	if (PingRequestTime < 0.0f)
	{
		RequestPings();
		PingRequestTime = 2.0f;
	}

	GetRemoteMessages();

	for (i = 0 ; i < PlayerCount ; i++)
	{
		DumpText(192, i * 16 + 96, 8, 16, 0xff0000, PlayerList[i].Name);
		wsprintf(buf, "%ld", PlayerList[i].Ping);
		DumpText(448, i * 16 + 96, 8, 16, 0xff0000, buf);
	}

	D3Ddevice->EndScene();

// quit?

	if ((Keys[DIK_ESCAPE] && !LastKeys[DIK_ESCAPE]) || GameSettings.GameType != GAMETYPE_CLIENT)
	{
		DP->Close();
		MenuCount = 0;
		GameSettings.GameType = GAMETYPE_CLIENT;
		Event = LookForSessions;
	}

// host started?

	if (GameStarted)
	{
		Event = SetupGame;
	}
}

//////////////////////
// get host details //
//////////////////////

char GetHostDetails(void)
{
	int r;
	static char hostname[1024];
	HOSTENT *he;

// get host name

	r = gethostname(hostname, sizeof(hostname));
	if (r == SOCKET_ERROR)
		return FALSE;

// get host details

	he = gethostbyname(hostname);
	if (!he)
		return FALSE;

// store host details

	memcpy(HostName, he->h_name, MAX_HOST_NAME);

	IP[0] = (*he->h_addr_list)[0];
	IP[1] = (*he->h_addr_list)[1];
	IP[2] = (*he->h_addr_list)[2];
	IP[3] = (*he->h_addr_list)[3];

// return OK

	return TRUE;
}

//////////////////////
// get for legal IP //
//////////////////////

char CheckLegalIP(void)
{
	LEGAL_IP *lip;

// init play

	if (!InitPlay())
		return FALSE;

	if (!InitConnection(0))
	{
		KillPlay();
		return FALSE;
	}

// get host details

	if (!GetHostDetails())
	{
		KillPlay();
		return FALSE;
	}

// kill play

	KillPlay();

// check for a legal IP

	for (lip = LegalIP ; *(long*)lip->Mask ; lip++)
	{
		if ((IP[0] & lip->Mask[0]) == lip->IP[0] &&
			(IP[1] & lip->Mask[1]) == lip->IP[1] &&
			(IP[2] & lip->Mask[2]) == lip->IP[2] &&
			(IP[3] & lip->Mask[3]) == lip->IP[3])
				return TRUE;
	}

// illegal IP

	return FALSE;
}

//////////////////////////////
// sync with remote players //
//////////////////////////////

void RemoteSync(void)
{
	unsigned long flag, quit, time, starttime, sendtime;
	PLAYER *player;
	MESSAGE_HEADER *header = (MESSAGE_HEADER*)TransmitBuff;

// server

	if (GameSettings.GameType == GAMETYPE_SERVER)
	{

// zero ready flags

		for (player = PLR_PlayerHead ; player ; player = player->next)
			player->Ready = FALSE;

		ReadyFlag = TRUE;

// wait for all loaded

		quit = FALSE;
		starttime = CurrentTimer();

		while (!quit)
		{
			GetRemoteMessages();

			time = TIME2MS(CurrentTimer() - starttime);
			if (time > (1000 * 60))
			{
				Box(NULL, "Timed out waiting for players to load!", MB_OK);
				quit = TRUE;
			}

			flag = 0;
			for (player = PLR_PlayerHead ; player ; player = player->next)
				if (player->type == PLAYER_REMOTE && !player->Ready)
					flag++;

			if (!flag)
				quit = TRUE;

			if (GameSettings.GameType == GAMETYPE_SESSIONLOST)
			{
				QuitGame = TRUE;
				return;
			}
		}

// all loaded, start game

		CountdownEndTime = CurrentTimer() + MS2TIME(COUNTDOWN_START);
		CountdownTime = TRUE;

		header->Type = MESSAGE_SYNC_TIMERS1;
		*(unsigned long*)(header + 1) = CurrentTimer();
		TransmitMessage(TransmitBuff, sizeof(MESSAGE_HEADER) + sizeof(unsigned long), DPID_ALLPLAYERS, MESSAGE_PRIORITY_NORMAL);
	}

// client

	else
	{

// send ready messages until everyone loaded

		ReadyFlag = FALSE;
		starttime = CurrentTimer();
		sendtime = 0;

		while (!ReadyFlag)
		{
			GetRemoteMessages();

			time = TIME2MS(CurrentTimer() - starttime);
			if (time > sendtime)
			{
				header->Type = MESSAGE_PLAYER_READY;
				TransmitMessage(TransmitBuff, sizeof(MESSAGE_HEADER), ServerID, MESSAGE_PRIORITY_NORMAL);
				sendtime += 1000;
			}

			if (time > (1000 * 60))
			{
				Box(NULL, "Timed out waiting for server to start!", MB_OK);
				ReadyFlag = TRUE;
			}

			if (GameSettings.GameType == GAMETYPE_SESSIONLOST)
			{
				QuitGame = TRUE;
				return;
			}
		}

// wait until countdown timer set

		ReadyFlag = FALSE;

		while (!ReadyFlag)
		{
			GetRemoteMessages();
		}
	}
}
