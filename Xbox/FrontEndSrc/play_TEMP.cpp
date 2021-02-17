//$MODIFIED
//#include <winsock.h>
#include <xtl.h>
#include <winsockx.h>
//$END_MODIFICATIONS
#include "revolt.h"
#include "network.h"
#include "main.h"
#include "text.h"
#include "model.h"
#include "particle.h"
#include "aerial.h"
#include "NewColl.h"
#include "body.h"
#include "car.h"
#include "ctrlread.h"
#include "object.h"
#include "obj_init.h"
#include "control.h"
#include "player.h"
#include "input.h"
#include "draw.h"
#include "settings.h"
#include "geom.h"
#include "move.h"
#include "timing.h"
#include "weapon.h"
#include "InitPlay.h"
#include "TitleScreen.h"
#include "panel.h"
#include "pickup.h"
#include "gameloop.h"

#include "XBInput.h"
#include "VoiceCommunicator.h"

// menus
#include "Menu2.h"
#include "WaitingRoom.h"
#include "MainMenu.h"

// globals

  #ifndef XBOX_DISABLE_NETWORK
IDirectPlay4A *DP = NULL;
IDirectPlayLobby3 *Lobby = NULL;
  #endif // ! XBOX_DISABLE_NETWORK
DPID FromID, ToID, LocalPlayerID, ServerID, GroupID;
DP_PLAYER PlayerList[MAX_RACE_CARS];
REAL PacketsPerSecond = DP_PACKETS_PER_SEC;
REAL NextPacketTimer, NextSyncTimer, NextPositionTimer, AllReadyTimeout;
  #ifndef XBOX_DISABLE_NETWORK
long SessionFlag = DPSESSION_DIRECTPLAYPROTOCOL | DPSESSION_KEEPALIVE | DPSESSION_OPTIMIZELATENCY;
  #else
long SessionFlag = 0;
  #endif // ! XBOX_DISABLE_NETWORK
long PlayerCount, NextPositionReady, NextPacketReady, NextSyncReady, NextSyncPlayer, AllPlayersReady, GameStarted, HostQuit, LocalPlayerReady;
float PlayersRequestTime, SessionRequestTime;
MESSAGE_HEADER *SendHeader, *ReceiveHeader;
char *MessageQueue;
long MessageQueueSize, TotalDataSent;
  #ifndef XBOX_DISABLE_NETWORK
DPSESSIONDESC2 Session;
char ConnectionCount, SessionCount, SessionPick;
DP_SESSION SessionList[SESSION_MAX];
DPLCONNECTION *LobbyConnection;
DP_CONNECTION Connection[CONNECTION_MAX];
  #endif // ! XBOX_DISABLE_NETWORK
RESTART_DATA RestartData;

static long IPcount;
static char SessionName[MAX_SESSION_NAME];
static unsigned char IP[4][4];
static DP_PLAYER_DATA LocalPlayerData;
static char HostName[MAX_HOST_NAME];
static long JoinFlag;
static float RemoteSyncTimer;
static unsigned long RemoteSyncBestPing, RemoteSyncHostTime, RemoteSyncClientTime;
static HANDLE PlayerEvent = NULL;
static HANDLE KillEvent = NULL;
static HANDLE ReceiveThreadHandle = NULL;
static DWORD ReceiveThreadId = 0;
//$REMOVEDstatic DPCAPS DPcaps;

// allowed IP addresses

LEGAL_IP LegalIP[] = {
//  {255, 255, 255, 0, 100, 103, 0, 0},     // Iguana Texas
//  {255, 255, 255, 0, 100, 104, 0, 0},     // Sculptured US
//  {255, 255, 255, 0, 100, 105, 0, 0},     // Acclaim UK
    {255, 255, 255, 0, 100, 106, 0, 0},     // Probe
//  {255, 255, 255, 0, 100, 107, 0, 0},     // Iguana UK 0
//  {255, 255, 255, 0, 100, 107, 1, 0},     // Iguana UK 1
//  {255, 255, 255, 0, 100, 107, 2, 0},     // Iguana UK 2
//  {255, 255, 255, 0, 100, 108, 0, 0},     // Acclaim Germany
//  {255, 255, 255, 0, 100, 109, 0, 0},     // Acclaim France

//  {255, 255, 255, 0, 38, 240, 105, 0},    // Acclaim red network
//  {255, 255, 255, 0, 100, 0, 7, 0},       // Acclaim NY
//  {255, 255, 255, 0, 100, 0, 5, 0},       // Acclaim NY
//  {255, 255, 255, 0, 194, 129, 18, 0},    // Probe

//  {255, 255, 255, 255, 100, 0, 5, 70},    // Doug Yellin

    {0, 0, 0, 0}
};

// play GUID

GUID DP_GUID = {0x6bb78285, 0x71df, 0x11d2, 0xb4, 0x6c, 0xc, 0x78, 0xc, 0xc1, 0x8, 0x40};

////////////////////////////////
// register for lobby support //
////////////////////////////////

void LobbyInit(void)
{
#ifndef XBOX_DISABLE_NETWORK
    HRESULT r;
    DPAPPLICATIONDESC app;
    char dir[MAX_PATH];

// create lobby object

    r = CoCreateInstance(CLSID_DirectPlayLobby, NULL, CLSCTX_INPROC_SERVER, IID_IDirectPlayLobby3A, (void**)&Lobby);
    if (r != DP_OK)
    {
        ErrorDX(r, "Can't create DirectPlay Lobby object");
        QuitGame = TRUE;
        return;
    }

// register

    GetCurrentDirectory(MAX_PATH, dir);

    app.dwSize = sizeof(app);
    app.dwFlags = 0;
    app.lpszApplicationNameA = "Re-Volt";
    app.guidApplication = DP_GUID;
    app.lpszFilenameA = "revolt.exe";
//  app.lpszFilenameA = "bay\\revolt.exe";
    app.lpszCommandLineA = "";
    app.lpszPathA = dir;
    app.lpszCurrentDirectoryA = dir;
    app.lpszDescriptionA = NULL;
    app.lpszDescriptionW = NULL;

    r = Lobby->RegisterApplication(0, &app);
    if (r != DP_OK)
    {
        ErrorDX(r, "Can't register for lobby support");
        RELEASE(Lobby);
        return;
    }

// attempt to connect

    LobbyConnect();
#endif // ! XBOX_DISABLE_NETWORK
}

//////////////////////
// connect to lobby //
//////////////////////

void LobbyConnect(void)
{
#ifndef XBOX_DISABLE_NETWORK
    HRESULT r;
    DWORD size;

// launched by a lobby?

    r = Lobby->GetConnectionSettings(0, NULL, &size);
    if (r == DPERR_NOTLOBBIED)
    {
        RELEASE(Lobby);
        return;
    }

// get settings

    LobbyConnection = (DPLCONNECTION*)malloc(size);
    if (!LobbyConnection)
    {
        ErrorDX(0, "Can't alloc memory for lobby connection details!");
        RELEASE(Lobby);
        return;
    }

    r = Lobby->GetConnectionSettings(0, LobbyConnection, &size);
    if (r != DP_OK)
    {
        ErrorDX(r, "Can't get lobby connection details!");
        RELEASE(Lobby);
        free(LobbyConnection);
        return;
    }

// modify session?

    LobbyConnection->lpSessionDesc->dwFlags = SessionFlag;
    LobbyConnection->lpSessionDesc->dwMaxPlayers = DEFAULT_RACE_CARS;
    r = Lobby->SetConnectionSettings(0, 0, LobbyConnection);
    if (r != DP_OK)
    {
        ErrorDX(r, "Can't set lobby connection details!");
        RELEASE(Lobby);
        free(LobbyConnection);
        return;
    }

// connect to session

    r = Lobby->ConnectEx(0, IID_IDirectPlay4A, (void**)&DP, NULL);
    if (r != DP_OK)
    {
        ErrorDX(r, "Can't connect to lobby session!");
        RELEASE(Lobby);
        free(LobbyConnection);
        return;
    }

// get caps

    DPcaps.dwSize = sizeof(DPCAPS);
    r = DP->GetCaps(&DPcaps, 0);
    if (r != DP_OK)
    {
        ErrorDX(r, "Can't get DirectPlay caps");
        RELEASE(Lobby);
        free(LobbyConnection);
        return;
    }

// get group ID

#if USE_GROUPS
    if (LobbyConnection->dwFlags == DPLCONNECTION_CREATESESSION)
    {
        r = DP->CreateGroup(&GroupID, NULL, NULL, 0, 0);
        if (r != DP_OK)
        {
            ErrorDX(r, "Can't create DirectPlay group");
            RELEASE(Lobby);
            free(LobbyConnection);
            return;
        }
    }
    else
    {
        GroupID = 0;
        r = DP->EnumGroups(NULL, EnumGroupCallback, NULL, DPENUMGROUPS_ALL);
        if (r != DP_OK)
        {
            ErrorDX(r, "Can't enum DirectPlay groups");
            RELEASE(Lobby);
            free(LobbyConnection);
            return;
        }
    }
#else
    GroupID = DPID_ALLPLAYERS;
#endif

// alloc buffer space

    SendHeader = (MESSAGE_HEADER*)malloc(DP_BUFFER_MAX);
    ReceiveHeader = (MESSAGE_HEADER*)malloc(DP_BUFFER_MAX);
    MessageQueue = (char*)malloc(DP_BUFFER_MAX);
    if (!SendHeader || !ReceiveHeader || !MessageQueue)
    {
        Box(NULL, "Can't alloc memory for DirectPlay packets!", MB_OK);
        RELEASE(Lobby);
        free(LobbyConnection);
        return;
    }

// copy session desc

    memcpy(&Session, LobbyConnection->lpSessionDesc, sizeof(DPSESSIONDESC2));

// set initial session description if host

    if (LobbyConnection->dwFlags == DPLCONNECTION_CREATESESSION)
    {
        LEVELINFO *levinf = GetLevelInfo(g_TitleScreenData.iLevelNum);
        SetSessionDesc(g_TitleScreenData.nameEnter[0], levinf->Dir, FALSE, GAMETYPE_MULTI, g_TitleScreenData.RandomCars, g_TitleScreenData.RandomTrack);
    }

// create player

    CreatePlayer(LobbyConnection->lpPlayerName->lpszShortNameA, LobbyConnection->dwFlags == DPLCONNECTION_CREATESESSION ? DP_SERVER_PLAYER : DP_CLIENT_PLAYER);
#endif // ! XBOX_DISABLE_NETWORK
}

////////////////
// free lobby //
////////////////

void LobbyFree(void)
{
#ifndef XBOX_DISABLE_NETWORK

// already dead?

    if (!Lobby)
    {
        return;
    }

// free buffer mem

    free(SendHeader);
    free(ReceiveHeader);
    free(MessageQueue);

// free connection mem

    if (LobbyConnection)
    {
        free(LobbyConnection);
        LobbyConnection = NULL;
    }

// release DP + lobby

    RELEASE(DP);
    RELEASE(Lobby);
#endif // ! XBOX_DISABLE_NETWORK
}

/////////////////////
// init directplay //
/////////////////////

bool InitPlay(void)
{
#ifndef XBOX_DISABLE_NETWORK
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

/*  PlayerEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (!PlayerEvent)
    {
        ErrorDX(0, "Can't create player event!");
        QuitGame = TRUE;
        return FALSE;
    }

// create kill event handle

    KillEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (!KillEvent)
    {
        ErrorDX(0, "Can't create kill event!");
        QuitGame = TRUE;
        return FALSE;
    }

// create receive thread

    ReceiveThreadHandle = CreateThread(NULL, 0, ReceiveThread, NULL, 0, &ReceiveThreadId);
    if (!ReceiveThreadHandle)
    {
        ErrorDX(0, "Can't create DirectPlay receive thread");
        QuitGame = TRUE;
        return FALSE;
    }*/

// alloc buffer space

    SendHeader = (MESSAGE_HEADER*)malloc(DP_BUFFER_MAX);
    ReceiveHeader = (MESSAGE_HEADER*)malloc(DP_BUFFER_MAX);
    MessageQueue = (char*)malloc(DP_BUFFER_MAX);
    if (!SendHeader || !ReceiveHeader || !MessageQueue)
    {
        Box(NULL, "Can't alloc memory for DirectPlay packets!", MB_OK);
        QuitGame = TRUE;
        return FALSE;
    }

// return OK

    return TRUE;
#else // ! XBOX_DISABLE_NETWORK
    return FALSE;
#endif // ! XBOX_DISABLE_NETWORK
}

/////////////////////
// kill directplay //
/////////////////////

void KillPlay(void)
{
#ifndef XBOX_DISABLE_NETWORK
    char i;

// already dead?

    if (!DP)
    {
        return;
    }

// free receive thread

/*  SetEvent(KillEvent);
    WaitForSingleObject(ReceiveThread, INFINITE);

    CloseHandle(ReceiveThreadHandle);
    ReceiveThreadHandle = NULL;

// free kill event handle

    CloseHandle(KillEvent);
    KillEvent = NULL;

// free player event handle

    CloseHandle(PlayerEvent);
    PlayerEvent = NULL;*/

// free buffer mem

    free(SendHeader);
    free(ReceiveHeader);
    free(MessageQueue);

// free connection ptr's

    for (i = 0 ; i < CONNECTION_MAX ; i++) if (Connection[i].Ptr)
    {
        free(Connection[i].Ptr);
        Connection[i].Ptr = NULL;
    }

// release DP object

    RELEASE(DP);
#endif // ! XBOX_DISABLE_NETWORK
}

/* $REMOVED
//////////////////////////////
// EnumConnections callback //
//////////////////////////////

BOOL FAR PASCAL EnumConnectionsCallback(LPCGUID lpguidSP, LPVOID lpConnection, DWORD dwConnectionSize, LPCDPNAME lpName, DWORD dwFlags, LPVOID lpContext)
{

// skip if reached max connections

    if (ConnectionCount >= CONNECTION_MAX)
    {
        return FALSE;
    }

// always skip serial connection

    if (lpguidSP->Data1 == 0xf1d6860 && lpguidSP->Data2 == 0x88d9 && lpguidSP->Data3 == 0x11cf && *(_int64*)lpguidSP->Data4 == 0x5e4205c9a0004e9c)
    {
        return TRUE;
    }

// only IPX if shareware

//  if (lpguidSP->Data1 == 0x36e95ee0 && lpguidSP->Data2 == 0x8577 && lpguidSP->Data3 == 0x11cf && *(_int64*)lpguidSP->Data4 == 0x824e53c780000c96)

// store shit

    memcpy(&Connection[ConnectionCount].Guid, lpguidSP, sizeof(GUID));
    memcpy(Connection[ConnectionCount].Name, lpName->lpszShortNameA, 128);
    Connection[ConnectionCount].Ptr = malloc(dwConnectionSize);
    memcpy(Connection[ConnectionCount].Ptr, lpConnection, dwConnectionSize);

// return OK

    ConnectionCount++;
    return TRUE;
}
$END_REMOVAL */

///////////////////////
// init a connection //
///////////////////////

bool InitConnection(char num)
{
#ifndef XBOX_DISABLE_NETWORK
    HRESULT r;
    IDirectPlayLobby3 *lobby;
    DPCOMPOUNDADDRESSELEMENT addr[2];
    char compaddr[256];
    DWORD size;
    void *con = Connection[num].Ptr;

// override IP address dialog box?

    if (Connection[num].Guid.Data1 == 0x36e95ee0 && Connection[num].Guid.Data2 == 0x8577 && Connection[num].Guid.Data3 == 0x11cf && *(_int64*)Connection[num].Guid.Data4 == 0x824e53c780000c96)
    {
        r = CoCreateInstance(CLSID_DirectPlayLobby, NULL, CLSCTX_INPROC_SERVER, IID_IDirectPlayLobby3A, (void**)&lobby);
        if (r != DP_OK)
        {
            ErrorDX(r, "Can't create lobby for DirectPlay connection");
            return FALSE;
        }

        addr[0].guidDataType = DPAID_ServiceProvider;
        addr[0].lpData = (void*)&DPSPGUID_TCPIP;
        addr[0].dwDataSize = 16;

        addr[1].guidDataType = DPAID_INet;
        addr[1].lpData = RegistrySettings.HostComputer;
        addr[1].dwDataSize = strlen(RegistrySettings.HostComputer) + 1;

        size = 256;

        r = lobby->CreateCompoundAddress(addr, 2, compaddr, &size);
        if (r != DP_OK)
        {
            ErrorDX(r, "Can't create host computer address");
            return FALSE;
        }

        con = compaddr;

        lobby->Release();
    }

// init connection

    r = DP->InitializeConnection(con, 0);
    if (r != DP_OK)
    {
        ErrorDX(r, "Can't init DirectPlay connection");
        return FALSE;
    }

// return OK

    return TRUE;
#else // ! XBOX_DISABLE_NETWORK
    return FALSE;
#endif // ! XBOX_DISABLE_NETWORK
}

//////////////////////
// create a session //
//////////////////////

bool CreateSession(char *name)
{
#ifndef XBOX_DISABLE_NETWORK
    long modem;
    HRESULT r;

// setup a new session

    ZeroMemory(&Session, sizeof(Session));
    Session.dwSize = sizeof(Session);
    Session.dwFlags = SessionFlag;
    Session.guidApplication = DP_GUID;
    Session.dwMaxPlayers = g_TitleScreenData.numberOfCars;
    Session.lpszSessionNameA = name;

// flip to GDI if modem to modem

    modem = (Connection[g_TitleScreenData.connectionType].Guid.Data1 == 0x44eaa760 && Connection[g_TitleScreenData.connectionType].Guid.Data2 == 0xcb68 && Connection[g_TitleScreenData.connectionType].Guid.Data3 == 0x11cf && *(_int64*)Connection[g_TitleScreenData.connectionType].Guid.Data4 == 0x5e4205c9a0004e9c);
    if (modem)
    {
        DD->FlipToGDISurface();
    }

// open session

    r = DP->Open(&Session, DPOPEN_CREATE);

    if (r != DP_OK)
    {
        if (r != DPERR_USERCANCEL)
        {
            ErrorDX(r, "Can't create DirectPlay session");
        }
        return FALSE;
    }

// create group

#if USE_GROUPS
    r = DP->CreateGroup(&GroupID, NULL, NULL, 0, 0);
    if (r != DP_OK)
    {
        ErrorDX(r, "Can't create DirectPlay group");
        QuitGame = TRUE;
        return FALSE;
    }
#else
    GroupID = DPID_ALLPLAYERS;
#endif

// get caps

    DPcaps.dwSize = sizeof(DPCAPS);
    r = DP->GetCaps(&DPcaps, 0);
    if (r != DP_OK)
    {
        ErrorDX(r, "Can't get DirectPlay caps");
        return FALSE;
    }

// return OK

    return TRUE;
#else // ! XBOX_DISABLE_NETWORK
    return FALSE;
#endif // ! XBOX_DISABLE_NETWORK
}

//////////////////////
// create a session //
//////////////////////

bool SetSessionDesc(char *name, char *track, long started, long gametype, long randomcars, long randomtrack)
{
#ifndef XBOX_DISABLE_NETWORK
    char buf[256];
    HRESULT r;

// set name

    wsprintf(buf, "%s\n%s", name, track);
    Session.lpszSessionNameA = buf;
    Session.dwUser1 = started;
    Session.dwUser2 = gametype;
    Session.dwUser3 = MultiplayerVersion;
    Session.dwUser4 = 0;

    if (randomcars) Session.dwUser4 |= 1;
    if (randomtrack) Session.dwUser4 |= 2;

// set

    r = DP->SetSessionDesc(&Session, NULL);
    if (r != DP_OK)
    {
        ErrorDX(r, "Can't set session description");
    }

// return OK

#endif // ! XBOX_DISABLE_NETWORK
    return TRUE;
}

////////////////////
// join a session //
////////////////////

bool JoinSession(char num)
{
#ifndef XBOX_DISABLE_NETWORK
    HRESULT r;
    DPSESSIONDESC2 session;

// setup a session

    ZeroMemory(&session, sizeof(session));
    session.dwSize = sizeof(session);
    session.guidInstance = SessionList[num].Guid;
//  session.dwFlags = DPSESSION_DIRECTPLAYPROTOCOL;

// open session

    r = DP->Open(&session, DPOPEN_JOIN);
    if (r != DP_OK)
    {
        ErrorDX(r, "Can't join DirectPlay session");
        return FALSE;
    }

// get group ID

#if USE_GROUPS
    GroupID = 0;
    r = DP->EnumGroups(NULL, EnumGroupCallback, NULL, DPENUMGROUPS_ALL);
    if (r != DP_OK)
    {
        ErrorDX(r, "Can't enum DirectPlay groups");
        return FALSE;
    }
#else
    GroupID = DPID_ALLPLAYERS;
#endif

// get caps

    DPcaps.dwSize = sizeof(DPCAPS);
    r = DP->GetCaps(&DPcaps, 0);
    if (r != DP_OK)
    {
        ErrorDX(r, "Can't get DirectPlay caps");
        return FALSE;
    }

// return OK

#endif // ! XBOX_DISABLE_NETWORK
    return TRUE;
}

/* $REMOVED
/////////////////////////
// enum group callback //
/////////////////////////

BOOL FAR PASCAL EnumGroupCallback(DPID dpId, DWORD dwPlayerType, LPCDPNAME lpName, DWORD dwFlags, LPVOID lpContext)
{
#ifndef XBOX_DISABLE_NETWORK

// skip if not a group

    if (dwPlayerType != DPPLAYERTYPE_GROUP)
        return TRUE;

// save group ID

    GroupID = dpId;
#endif // ! XBOX_DISABLE_NETWORK
    return FALSE;
}
$END_REMOVAL */

///////////////////
// list sessions //
///////////////////

void ListSessions(void)
{
#ifndef XBOX_DISABLE_NETWORK
    long modem;
    HRESULT r;
    DPSESSIONDESC2 session;

    SessionCount = 0;

// DP dead?

    if (!DP)
    {
        return;
    }

// flip to GDI if modem to modem

    modem = (Connection[g_TitleScreenData.connectionType].Guid.Data1 == 0x44eaa760 && Connection[g_TitleScreenData.connectionType].Guid.Data2 == 0xcb68 && Connection[g_TitleScreenData.connectionType].Guid.Data3 == 0x11cf && *(_int64*)Connection[g_TitleScreenData.connectionType].Guid.Data4 == 0x5e4205c9a0004e9c);
    if (modem)
    {
        DD->FlipToGDISurface();
    }

// get desc

    ZeroMemory(&session, sizeof(session));
    session.dwSize = sizeof(session);
    session.guidApplication = DP_GUID;

//  r = DP->EnumSessions(&session, 0, EnumSessionsCallback, NULL, DPENUMSESSIONS_ASYNC | DPENUMSESSIONS_ALL);
    r = DP->EnumSessions(&session, 0, EnumSessionsCallback, NULL, DPENUMSESSIONS_ALL);
    if (r != DP_OK)
    {
        if (r != DPERR_USERCANCEL)
        {
            ErrorDX(r, "Can't enum sessions");
        }
    }
#endif // ! XBOX_DISABLE_NETWORK
}

//////////////////////////////
// stop session enumeration //
//////////////////////////////

void StopSessionEnum(void)
{
#ifndef XBOX_DISABLE_NETWORK
    HRESULT r;
    DPSESSIONDESC2 session;

    ZeroMemory(&session, sizeof(session));
    session.dwSize = sizeof(session);
    session.guidApplication = DP_GUID;

    r = DP->EnumSessions(&session, 0, EnumSessionsCallback, NULL, DPENUMSESSIONS_STOPASYNC);
    if (r != DP_OK)
    {
        ErrorDX(r, "Can't stop session enum");
    }
#endif // ! XBOX_DISABLE_NETWORK
}

/* $REMOVED
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
    SessionList[SessionCount].Started = lpSessionDesc->dwUser1;
    SessionList[SessionCount].GameType = lpSessionDesc->dwUser2;
    SessionList[SessionCount].Version = lpSessionDesc->dwUser3;
    SessionList[SessionCount].RandomCars = lpSessionDesc->dwUser4 & 1;
    SessionList[SessionCount].RandomTrack = lpSessionDesc->dwUser4 & 2;
    SessionList[SessionCount].PlayerNum = lpSessionDesc->dwMaxPlayers;

// return OK

    SessionCount++;
    return TRUE;
}
$END_REMOVAL */

/////////////////////
// create a player //
/////////////////////

bool CreatePlayer(char *name, long mode)
{
#ifndef XBOX_DISABLE_NETWORK
    DPNAME dpname;
    HRESULT r;

// setup a name

    ZeroMemory(&dpname, sizeof(dpname));
    dpname.dwSize = sizeof(dpname);
    dpname.lpszShortNameA = name;
    dpname.lpszLongNameA = NULL;

// create a player

    r = DP->CreatePlayer(&LocalPlayerID, &dpname, NULL, &LocalPlayerData, sizeof(LocalPlayerData), mode == DP_SPECTATOR_PLAYER ? DPPLAYER_SPECTATOR : 0);
    if (r != DP_OK)
    {
        ErrorDX(r, "Can't create a player");
        QuitGame = TRUE;
        return FALSE;
    }

    if (mode == DP_SERVER_PLAYER)
        ServerID = LocalPlayerID;

// add player to group

#if USE_GROUPS
    r = DP->AddPlayerToGroup(GroupID, LocalPlayerID);
    if (r != DP_OK)
    {
        ErrorDX(r, "Can't add player to group");
        QuitGame = TRUE;
        return FALSE;
    }
#endif

// set good data

    SetPlayerData();

// return OK

    return TRUE;
#else // ! XBOX_DISABLE_NETWORK
    return FALSE;
#endif // ! XBOX_DISABLE_NETWORK
}

///////////////////////////
// set local player name //
///////////////////////////

void SetPlayerName(char *name)
{
#ifndef XBOX_DISABLE_NETWORK
    DPNAME dpname;
    HRESULT r;

    ZeroMemory(&dpname, sizeof(dpname));
    dpname.dwSize = sizeof(dpname);
    dpname.lpszShortNameA = name;
    dpname.lpszLongNameA = NULL;

    r = DP->SetPlayerName(LocalPlayerID, &dpname, 0);
    if (r != DP_OK)
    {
        ErrorDX(r, "Can't set player name");
    }
#endif // ! XBOX_DISABLE_NETWORK
}

///////////////////////////
// set local player name //
///////////////////////////

void SetPlayerData(void)
{
#ifndef XBOX_DISABLE_NETWORK
    HRESULT r;

// set car

    strncpy(LocalPlayerData.CarName, CarInfo[GameSettings.CarType].Name, CAR_NAMELEN);

// set ready flag

    LocalPlayerData.Ready = LocalPlayerReady;

// set cheating flag

    LocalPlayerData.Cheating = CarInfo[GameSettings.CarType].Modified;

// set

    r = DP->SetPlayerData(LocalPlayerID, &LocalPlayerData, sizeof(LocalPlayerData), DPSET_GUARANTEED);
    if (r != DP_OK)
    {
        ErrorDX(r, "Can't set player data");
    }
#endif // ! XBOX_DISABLE_NETWORK
}

/////////////////////////////////////////////
// queue a messsage in local message queue //
/////////////////////////////////////////////

void QueueMessage(MESSAGE_HEADER *buff, short size)
{
#ifndef XBOX_DISABLE_NETWORK

// init header if first message

    if (!MessageQueueSize)
    {
        ((MESSAGE_HEADER*)MessageQueue)->Type = MESSAGE_QUEUE;
        ((MESSAGE_HEADER*)MessageQueue)->Flag = 0;
        MessageQueueSize = sizeof(MESSAGE_HEADER);
    }

// skip if queue full

    if (MessageQueueSize + size + sizeof(long) > DP_BUFFER_MAX)
    {
        return;
    }

// copy size

    *(long*)&MessageQueue[MessageQueueSize] = (long)size;
    MessageQueueSize += sizeof(long);

// copy message

    memcpy(&MessageQueue[MessageQueueSize], buff, size);
    MessageQueueSize += size;

// inc count

    ((MESSAGE_HEADER*)MessageQueue)->Flag++;
#endif // ! XBOX_DISABLE_NETWORK
}

////////////////////
// send a message //
////////////////////

void SendMessage(MESSAGE_HEADER *buff, short size, DPID to)
{
#ifndef XBOX_DISABLE_NETWORK
    HRESULT r;

    TotalDataSent += size;

    r = DP->SendEx(LocalPlayerID, to, DPcaps.dwFlags & DPCAPS_ASYNCSUPPORTED ? DPSEND_ASYNC | DPSEND_NOSENDCOMPLETEMSG : 0, buff, size, 0, DPcaps.dwFlags & DPCAPS_SENDTIMEOUTSUPPORTED ? DP_SEND_TIMEOUT : 0, NULL, NULL);
//  if (r != DP_OK && r != DPERR_PENDING && r != DPERR_INVALIDPLAYER)
//  {
//      ErrorDX(r, "Can't send DirectPlay message");
//  }
#endif // ! XBOX_DISABLE_NETWORK
}

////////////////////
// send a message //
////////////////////

void SendMessageGuaranteed(MESSAGE_HEADER *buff, short size, DPID to)
{
#ifndef XBOX_DISABLE_NETWORK
    HRESULT r;

    TotalDataSent += size;

    r = DP->SendEx(LocalPlayerID, to, DPSEND_GUARANTEED, buff, size, 0, 0, NULL, NULL);
//  if (r != DP_OK && r != DPERR_PENDING && r != DPERR_INVALIDPLAYER)
//  {
//      ErrorDX(r, "Can't send DirectPlay guaranteed message");
//  }
#endif // ! XBOX_DISABLE_NETWORK
}

//////////////////////
// receive a packet //
//////////////////////

char GetRemoteMessages(void)
{
#ifndef XBOX_DISABLE_NETWORK
    HRESULT r;
    DWORD size, i;
    char *pos, flag = FALSE;

// get all messages

    do
    {

// get one

        size = DP_BUFFER_MAX;
        r = DP->Receive(&FromID, &ToID, DPRECEIVE_ALL, ReceiveHeader, &size);

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

// queue?

            else if (ReceiveHeader->Type == MESSAGE_QUEUE)
            {
                i = ReceiveHeader->Flag;
                memcpy(SendHeader, ReceiveHeader + 1, size - sizeof(MESSAGE_HEADER));
                pos = (char*)SendHeader;

                for ( ; i ; i--)
                {
                    size = *(long*)pos;
                    pos += sizeof(long);
                    memcpy(ReceiveHeader, pos, size);
                    pos += size;
                    ProcessPersonalMessage();
                }
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
#else // ! XBOX_DISABLE_NETWORK
    return FALSE;
#endif // ! XBOX_DISABLE_NETWORK
}

//////////////////////////////
// process personal message //
//////////////////////////////

void ProcessPersonalMessage(void)
{
#ifndef XBOX_DISABLE_NETWORK

// act on message

    switch (ReceiveHeader->Type)
    {

// car data

        case MESSAGE_CAR_DATA:
            ProcessCarData();
            break;

// new car

        case MESSAGE_CAR_NEWCAR:
            ProcessCarNewCar();
            break;

// new car all

        case MESSAGE_CAR_NEWCAR_ALL:
            ProcessCarNewCarAll();
            break;

// weapon data

        case MESSAGE_WEAPON_DATA:
            ProcessWeaponNew();
            break;

// object data

        case MESSAGE_OBJECT_DATA:
            ProcessObjectData();
            break;

// target status

        case MESSAGE_TARGET_STATUS_DATA:
            ProcessTargetStatus();
            break;

// game started

        case MESSAGE_GAME_STARTED:
            ProcessGameStarted();
            break;

// sync request

        case MESSAGE_SYNC_REQUEST:
            ProcessSyncRequest();
            break;

// sync reply

        case MESSAGE_SYNC_REPLY:
            ProcessSyncReply();
            break;

// game loaded

        case MESSAGE_GAME_LOADED:
            ProcessGameLoaded();
            break;

// countdown started

        case MESSAGE_COUNTDOWN_START:
            ProcessCountdownStart();
            break;

// join info

        case MESSAGE_JOIN_INFO:
            ProcessJoinInfo();
            break;

// race finish time

        case MESSAGE_RACE_FINISH_TIME:
            ProcessRaceFinishTime();
            break;

// player sync request

        case MESSAGE_PLAYER_SYNC_REQUEST:
            ProcessPlayerSync1();
            break;

// player sync reply 1

        case MESSAGE_PLAYER_SYNC_REPLY1:
            ProcessPlayerSync2();
            break;

// player sync reply 2

        case MESSAGE_PLAYER_SYNC_REPLY2:
            ProcessPlayerSync3();
            break;

// game restarting

        case MESSAGE_RESTART:
            ProcessMultiplayerRestart();
            break;

// player position

        case MESSAGE_POSITION:
            ProcessPosition();
            break;

// transfer bomb

        case MESSAGE_TRANSFER_BOMB:
            ProcessTransferBomb();
            break;

// chat message

        case MESSAGE_CHAT:
            ProcessChatMessage();
            break;

// bomb tag timer

        case MESSAGE_BOMBTAG_CLOCK:
            ProcessBombTagClock();
            break;

// transfer fox

        case MESSAGE_TRANSFER_FOX:
            ProcessTransferFox();
            break;

// electropulse the world

        case MESSAGE_ELECTROPULSE_THE_WORLD:
            ProcessElectroPulseTheWorld();
            break;

// got global

        case MESSAGE_GOT_GLOBAL:
            ProcessGotGlobal();
            break;

// honka

        case MESSAGE_HONKA:
            ProcessHonka();
            break;
    }
#endif // ! XBOX_DISABLE_NETWORK
}

///////////////////////
// send game started //
///////////////////////

void SendGameStarted(void)
{
    SendHeader->Type = MESSAGE_GAME_STARTED;
    memcpy(SendHeader + 1, &StartData, sizeof(START_DATA));
    SendMessageGuaranteed(SendHeader, sizeof(MESSAGE_HEADER) + sizeof(START_DATA), GroupID);
}

//////////////////////////
// process game started //
//////////////////////////

void ProcessGameStarted(void)
{

// set flags

    GameStarted = TRUE;
    ServerID = FromID;

// save start data

    MultiStartData = *(START_DATA*)(ReceiveHeader + 1);
}

/////////////////////////
// send a sync request //
/////////////////////////

void SendSyncRequest(void)
{
    unsigned long *ptr = (unsigned long*)(SendHeader + 1);

// send request with my time

    SendHeader->Type = MESSAGE_SYNC_REQUEST;
    ptr[0] = CurrentTimer();

    SendMessage(SendHeader, sizeof(MESSAGE_HEADER) + sizeof(unsigned long), ServerID);
}

////////////////////////////
// process a sync request //
////////////////////////////

void ProcessSyncRequest(void)
{
    unsigned long *rptr = (unsigned long*)(ReceiveHeader + 1);
    unsigned long *sptr = (unsigned long*)(SendHeader + 1);

// return client + host time

    SendHeader->Type = MESSAGE_SYNC_REPLY;
    sptr[0] = rptr[0];
    sptr[1] = CurrentTimer();

    SendMessage(SendHeader, sizeof(MESSAGE_HEADER) + sizeof(unsigned long) * 2, FromID);
}

//////////////////////////
// process a sync reply //
//////////////////////////

void ProcessSyncReply(void)
{
    unsigned long *ptr = (unsigned long*)(ReceiveHeader + 1);
    unsigned long time, ping;

// best ping?

    time = CurrentTimer();
    ping = time - ptr[0];

    if (ping < RemoteSyncBestPing)
    {
        RemoteSyncBestPing = ping;
        RemoteSyncHostTime = ptr[1];
        RemoteSyncClientTime = ptr[0] + (ping / 2);
    }
}

//////////////////////
// send game loaded //
//////////////////////

void SendGameLoaded(void)
{

// set timeout counter

    AllReadyTimeout = 60.0f;

// send loaded message

    SendHeader->Type = MESSAGE_GAME_LOADED;
    SendMessageGuaranteed(SendHeader, sizeof(MESSAGE_HEADER), GroupID);
}

/////////////////////////
// process game loaded //
/////////////////////////

void ProcessGameLoaded(void)
{
    PLAYER *player;

// find loaded player

    player = GetPlayerFromPlayerID(FromID);
    if (!player)
        return;

    player->Ready = TRUE;
}

//////////////////////////
// send countdown start //
//////////////////////////

void SendCountdownStart(void)
{
    unsigned long time, *ptr = (unsigned long*)(SendHeader + 1);

// send start time + timer freq

    SendHeader->Type = MESSAGE_COUNTDOWN_START;

    time = CurrentTimer();
    ptr[0] = time;
    ptr[1] = TimerFreq;

    SendMessageGuaranteed(SendHeader, sizeof(MESSAGE_HEADER) + sizeof(long) * 2, GroupID);

// start countdown timer

    CountdownEndTime = time + MS2TIME(COUNTDOWN_START);
}

/////////////////////////////
// process countdown start //
/////////////////////////////

void ProcessCountdownStart(void)
{
    unsigned long *ptr = (unsigned long*)(ReceiveHeader + 1);
    unsigned long time, hoststart, hostfreq, elapsedms, clientstart, clientdiff;

// get current time

    time = CurrentTimer();

// set countdown normally if never got a sync reply

    if (RemoteSyncBestPing == 0xffffffff)
    {
        CountdownEndTime = time + MS2TIME(COUNTDOWN_START);
    }

// calc countdown time using best sync reply

    else
    {
        hoststart = ptr[0];
        hostfreq = ptr[1];

        elapsedms = (unsigned long)((__int64)(hoststart - RemoteSyncHostTime) * 1000 / hostfreq);
        clientstart = RemoteSyncClientTime + MS2TIME(elapsedms);
        clientdiff = time - clientstart;

        CountdownEndTime = time + MS2TIME(COUNTDOWN_START) - clientdiff;
    }

// set all players ready

    AllPlayersReady = TRUE;
}

////////////////////
// send join info //
////////////////////

void SendJoinInfo(DPID id)
{
    JOIN_INFO *data = (JOIN_INFO*)(SendHeader + 1);

// set message type

    SendHeader->Type = MESSAGE_JOIN_INFO;

// set race time

    data->RaceTime = TotalRaceTime;

// set level dir

    strncpy(data->LevelDir, StartData.LevelDir, MAX_LEVEL_DIR_NAME);

// send

    SendMessageGuaranteed(SendHeader, sizeof(MESSAGE_HEADER) + sizeof(JOIN_INFO), id);
}

///////////////////////
// process join info //
///////////////////////

void ProcessJoinInfo(void)
{
    JOIN_INFO *data = (JOIN_INFO*)(ReceiveHeader + 1);

// set flags

    ServerID = FromID;
    GameStarted = TRUE;

// set level dir

    strncpy(StartData.LevelDir, data->LevelDir, MAX_LEVEL_DIR_NAME);
/*  GameSettings.Level = GetLevelNum(data->LevelDir);
    if (GameSettings.Level == -1)
    {
        wsprintf(buf, "Can't find Level directory '%s'", StartData.LevelDir);
        Box(NULL, buf, MB_OK);
        QuitGame = TRUE;
        return;
    }*/

// save race time

    TotalRaceTime = data->RaceTime;
}

///////////////////////////
// send race finish time //
///////////////////////////

void SendRaceFinishTime(void)
{
    RACE_TIME_INFO *info = (RACE_TIME_INFO*)(SendHeader + 1);

// set message type

    SendHeader->Type = MESSAGE_RACE_FINISH_TIME;

// set race time

    info->Time = TotalRaceTime;

// send

    QueueMessage(SendHeader, sizeof(MESSAGE_HEADER) + sizeof(RACE_TIME_INFO));
}

//////////////////////////////
// process race finish time //
//////////////////////////////

void ProcessRaceFinishTime(void)
{
    PLAYER *player;
    RACE_TIME_INFO *info = (RACE_TIME_INFO*)(ReceiveHeader + 1);

// get player

    player = GetPlayerFromPlayerID(FromID);
    if (!player)
        return;

// set race finish time

    SetPlayerFinished(player, info->Time);
}

///////////////////////////////////////////////
// send a 'sync with me' message to a player //
///////////////////////////////////////////////

void SendPlayerSync(void)
{
    unsigned long *ptr = (unsigned long*)(SendHeader + 1);
    long count;
    PLAYER *player;

// find player

    count = NextSyncPlayer;
    for (player = PLR_PlayerHead ; player ; player = player->next)
    {
        if (player != PLR_LocalPlayer)
            count--;

        if (count < 0)
            break;
    }

    if (!player)
        return;

// send sync request + my race time

    SendHeader->Type = MESSAGE_PLAYER_SYNC_REQUEST;
    ptr[0] = TotalRaceTime;

    SendMessage(SendHeader, sizeof(MESSAGE_HEADER) + sizeof(long), player->PlayerID);

// set next sync player

    NextSyncPlayer++;
    NextSyncPlayer %= (NumPlayers - 1);
}

//////////////////////////////////
// reply to player sync message //
//////////////////////////////////

void ProcessPlayerSync1(void)
{
    unsigned long *rptr = (unsigned long*)(ReceiveHeader + 1);
    unsigned long *sptr = (unsigned long*)(SendHeader + 1);

// return host race time + my race time

    SendHeader->Type = MESSAGE_PLAYER_SYNC_REPLY1;
    sptr[0] = rptr[0];
    sptr[1] = TotalRaceTime;

    SendMessage(SendHeader, sizeof(MESSAGE_HEADER) + sizeof(unsigned long) * 2, FromID);
}

//////////////////////////////////
// reply to player sync message //
//////////////////////////////////

void ProcessPlayerSync2(void)
{
    unsigned long *rptr = (unsigned long*)(ReceiveHeader + 1);
    unsigned long *sptr = (unsigned long*)(SendHeader + 1);
    PLAYER *player;

// return my old race time + client race time + my new race time

    SendHeader->Type = MESSAGE_PLAYER_SYNC_REPLY2;
    sptr[0] = rptr[0];
    sptr[1] = rptr[1];
    sptr[2] = TotalRaceTime;

    SendMessage(SendHeader, sizeof(MESSAGE_HEADER) + sizeof(unsigned long) * 3, FromID);

// set host->client->host last ping

    player = GetPlayerFromPlayerID(FromID);
    if (player)
    {
        player->LastPing = sptr[2] - sptr[0];
    }
}

//////////////////////////////////
// reply to player sync message //
//////////////////////////////////

void ProcessPlayerSync3(void)
{
    unsigned long *ptr = (unsigned long*)(ReceiveHeader + 1);
    unsigned long host1, host2, client1, ping, newracetime, adjust;
    PLAYER *player;

// get host / client race times

    host1 = ptr[0];
    host2 = ptr[2];
    client1 = ptr[1];

// get host->client->host ping, quit if over 1 second

    ping = host2 - host1;
    if (ping > DP_SYNC_PING_MAX)
        return;

// calc new race time

    newracetime = host2 + (ping / 2);

// set physics time

    TotalRacePhysicsTime = newracetime;
    TotalRacePhysicsTime -= TotalRacePhysicsTime % PHYSICSTIMESTEP;

// adjust race / lap timers

    if (newracetime > TotalRaceTime)
    {
        adjust = MS2TIME(newracetime - TotalRaceTime);

        TotalRaceStartTime -= adjust;

        for (player = PLR_PlayerHead ; player ; player = player->next)
        {
            player->car.CurrentLapStartTime -= adjust;
        }
    }
    else
    {
        adjust = MS2TIME(TotalRaceTime - newracetime);

        TotalRaceStartTime += adjust;

        for (player = PLR_PlayerHead ; player ; player = player->next)
        {
            player->car.CurrentLapStartTime += adjust;
        }
    }

// set client->host->client last ping

    player = GetPlayerFromPlayerID(FromID);
    if (player)
    {
        player->LastPing = TotalRaceTime - client1;
    }
}

///////////////////////////////
// send race restart message //
///////////////////////////////

void SendMultiplayerRestart(void)
{
    long i;
    LEVELINFO *li;
    RESTART_DATA *data = (RESTART_DATA*)(SendHeader + 1);

// message

    StopAllSfx();
    SetBackgroundColor(0x000000);
    SetViewport(CAM_MainCamera->X, CAM_MainCamera->Y, CAM_MainCamera->Xsize, CAM_MainCamera->Ysize, BaseGeomPers + CAM_MainCamera->Lens);
    SetCameraView(&CAM_MainCamera->WMatrix, &CAM_MainCamera->WPos, CAM_MainCamera->Shake);

    for (i = 0 ; i < 3 ; i++)
    {
        D3Ddevice->BeginScene();

        ClearBuffers();
        InitRenderStates();
        BeginTextState();
        SET_TPAGE(TPAGE_FONT);
        DrawGameText(268, 232, 0xffffff, "Restarting...");

        D3Ddevice->EndScene();

        FlipBuffers();
    }

    SetBackgroundColor(CurrentLevelInfo.FogColor);

// set random car if relevant

    if (GameSettings.RandomCars)
    {
        long car = PickRandomCar();

        for (i = 0 ; i < StartData.PlayerNum ; i++)
            StartData.PlayerData[i].CarType = car;

        strncpy(data->CarName, CarInfo[car].Name, CAR_NAMELEN);
    }

// set new track if relevant

    data->NewTrack = FALSE;

    if (GameSettings.RandomTrack)
    {
        GameSettings.Level = PickRandomTrack();
        li = GetLevelInfo(GameSettings.Level);
        strncpy(StartData.LevelDir, li->Dir, MAX_LEVEL_DIR_NAME);
        data->NewTrack = TRUE;
    }
    else
    {
        long track = GetLevelNum(StartData.LevelDir);
        if (track != -1 && track != GameSettings.Level)
        {
            GameSettings.Level = track;
            data->NewTrack = TRUE;
        }
    }

    if (data->NewTrack)
    {
        LEV_EndLevelStageTwo();
        LEV_EndLevelStageOne();

        LoadMipTexture("D:\\gfx\\font.bmp", TPAGE_FONT, 256, 256, 0, 1, FALSE); //$MODIFIED: added "D:\\" at start
        LoadMipTexture("D:\\gfx\\loading.bmp", TPAGE_LOADING, 256, 256, 0, 1, FALSE); //$MODIFIED: added "D:\\" at start
        LoadMipTexture("D:\\gfx\\spru.bmp", TPAGE_SPRU, 256, 256, 0, 1, FALSE); //$MODIFIED: added "D:\\" at start

        strncpy(data->LevelDir, StartData.LevelDir, MAX_LEVEL_DIR_NAME);
    }

// set new rand seed

    StartData.Seed = rand();
    data->Seed = StartData.Seed;

// copy restart data to global

    RestartData = *(RESTART_DATA*)(SendHeader + 1);

// send restart

    SendHeader->Type = MESSAGE_RESTART;
    SendMessageGuaranteed(SendHeader, sizeof(MESSAGE_HEADER) + sizeof(RESTART_DATA), GroupID);

// sync

    RemoteSyncHost();
}

/////////////////////////////////
// process multiplayer restart //
/////////////////////////////////

void ProcessMultiplayerRestart(void)
{

// save data

    RestartData = *(RESTART_DATA*)(ReceiveHeader + 1);

// start fade down

    GameLoopQuit = GAMELOOP_QUIT_RESTART;
    SetFadeEffect(FADE_DOWN);
}

/////////////////////////
// restart race client //
/////////////////////////

void ClientMultiplayerRestart(void)
{
    long i;

// message

    StopAllSfx();
    SetBackgroundColor(0x000000);
    SetViewport(CAM_MainCamera->X, CAM_MainCamera->Y, CAM_MainCamera->Xsize, CAM_MainCamera->Ysize, BaseGeomPers + CAM_MainCamera->Lens);
    SetCameraView(&CAM_MainCamera->WMatrix, &CAM_MainCamera->WPos, CAM_MainCamera->Shake);

    for (i = 0 ; i < 3 ; i++)
    {
        D3Ddevice->BeginScene();

        ClearBuffers();
        InitRenderStates();
        BeginTextState();
        SET_TPAGE(TPAGE_FONT);
        DrawGameText(248, 232, 0xffffff, "Host Restarting...");

        D3Ddevice->EndScene();

        FlipBuffers();
    }

    SetBackgroundColor(CurrentLevelInfo.FogColor);

// set random cars if relevant

    if (GameSettings.RandomCars)
    {
        long car = GetCarTypeFromName(RestartData.CarName);

        for (i = 0 ; i < StartData.PlayerNum ; i++)
            StartData.PlayerData[i].CarType = car;
    }

// set random track if relevant

    if (RestartData.NewTrack)
    {
        LEV_EndLevelStageTwo();
        LEV_EndLevelStageOne();

        LoadMipTexture("D:\\gfx\\font.bmp", TPAGE_FONT, 256, 256, 0, 1, FALSE); //$MODIFIED: added "D:\\" at start
        LoadMipTexture("D:\\gfx\\loading.bmp", TPAGE_LOADING, 256, 256, 0, 1, FALSE); //$MODIFIED: added "D:\\" at start
        LoadMipTexture("D:\\gfx\\spru.bmp", TPAGE_SPRU, 256, 256, 0, 1, FALSE); //$MODIFIED: added "D:\\" at start

        strncpy(StartData.LevelDir, RestartData.LevelDir, MAX_LEVEL_DIR_NAME);
        GameSettings.Level = GetLevelNum(StartData.LevelDir);
    }

// set rand seed

    StartData.Seed = RestartData.Seed;

// clear menu

    gMenuHeader.pMenu = NULL;

// sync

    RemoteSyncClient();
}

//////////////////////////
// send player position //
//////////////////////////

void SendPosition(void)
{
    long *ptr = (long*)(SendHeader + 1);

    SendHeader->Type = MESSAGE_POSITION;
    ptr[0] = PLR_LocalPlayer->CarAI.FinishDistNode;
    ptr[1] = PLR_LocalPlayer->CarAI.ZoneID;
    QueueMessage(SendHeader, sizeof(MESSAGE_HEADER) + sizeof(long) * 2);
}

/////////////////////////////
// process player position //
/////////////////////////////

void ProcessPosition(void)
{
    PLAYER *player;
    long *ptr = (long*)(ReceiveHeader + 1);

    player = GetPlayerFromPlayerID(FromID);
    if (player)
    {
        player->CarAI.FinishDistNode = ptr[0];
        player->CarAI.ZoneID = ptr[1];
    }
}

////////////////////////
// send bomb transfer //
////////////////////////

void SendTransferBomb(LARGE_INTEGER objid, unsigned long player1id, unsigned long player2id)
{
    BOMB_TRANSFER_DATA *data = (BOMB_TRANSFER_DATA*)(SendHeader + 1);

    SendHeader->Type = MESSAGE_TRANSFER_BOMB;
    data->ObjID = objid;
    data->Player1ID = player1id;
    data->Player2ID = player2id;

    QueueMessage(SendHeader, sizeof(MESSAGE_HEADER) + sizeof(BOMB_TRANSFER_DATA));
}

///////////////////////////
// process bomb transfer //
///////////////////////////

void ProcessTransferBomb(void)
{
    BOMB_TRANSFER_DATA *data = (BOMB_TRANSFER_DATA*)(ReceiveHeader + 1);
    PLAYER *player1, *player2;
    OBJECT *obj;

    obj = GetObjectFromGlobalID(data->ObjID);
    player1 = GetPlayerFromPlayerID(data->Player1ID);
    player2 = GetPlayerFromPlayerID(data->Player2ID);

    if (obj && player1 && player2)
    {
        TransferBomb(obj, player1, player2);
    }
}

////////////////////////
// send fox transfer //
////////////////////////

void SendTransferFox(unsigned long player1id, unsigned long player2id)
{
    FOX_TRANSFER_DATA *data = (FOX_TRANSFER_DATA*)(SendHeader + 1);

    SendHeader->Type = MESSAGE_TRANSFER_FOX;
    data->Player1ID = player1id;
    data->Player2ID = player2id;

    QueueMessage(SendHeader, sizeof(MESSAGE_HEADER) + sizeof(FOX_TRANSFER_DATA));
}

///////////////////////////
// process bomb transfer //
///////////////////////////

void ProcessTransferFox(void)
{
    FOX_TRANSFER_DATA *data = (FOX_TRANSFER_DATA*)(ReceiveHeader + 1);
    PLAYER *player1, *player2;
    OBJECT *obj;

    obj = NextObjectOfType(OBJ_ObjectHead, OBJECT_TYPE_FOX);
    player1 = GetPlayerFromPlayerID(data->Player1ID);
    player2 = GetPlayerFromPlayerID(data->Player2ID);

    if (player2)
    {
        TransferFox(obj, player1, player2);
    }
}

/////////////////////////
// send bomb tag clock //
/////////////////////////

void SendBombTagClock(void)
{
    SendHeader->Type = MESSAGE_BOMBTAG_CLOCK;
    *(unsigned long*)(SendHeader + 1) = PLR_LocalPlayer->BombTagTimer;
    QueueMessage(SendHeader, sizeof(MESSAGE_HEADER) + sizeof(long));
}

/////////////////////////////
// processs bomb tag clock //
/////////////////////////////

void ProcessBombTagClock(void)
{
    PLAYER *player;

    player = GetPlayerFromPlayerID(FromID);
    if (player)
    {
        player->BombTagTimer = *(unsigned long*)(ReceiveHeader + 1);
    }
}

/////////////////////////////////
// send electropulse the world //
/////////////////////////////////

void SendElectroPulseTheWorld(long slot)
{
    SendHeader->Type = MESSAGE_ELECTROPULSE_THE_WORLD;
    *(long*)(SendHeader + 1) = slot;
    QueueMessage(SendHeader, sizeof(MESSAGE_HEADER) + sizeof(long));
}

////////////////////////////////////
// process electropulse the world //
////////////////////////////////////

void ProcessElectroPulseTheWorld(void)
{
    long slot;

    slot = *(long*)(ReceiveHeader + 1);
    ElectroPulseTheWorld(slot);
}

/////////////////////
// send got global //
/////////////////////

void SendGotGlobal(void)
{
    SendHeader->Type = MESSAGE_GOT_GLOBAL;
    QueueMessage(SendHeader, sizeof(MESSAGE_HEADER));
}

////////////////////////
// process got global //
///////////////////////

void ProcessGotGlobal(void)
{
    PLAYER *player;
    OBJECT *obj;

// give dummy pickup to player

    player = GetPlayerFromPlayerID(FromID);
    if (player)
    {
        GivePickupToPlayer(player, PICKUP_TYPE_DUMMY);
    }

// find and kill global pickup

    for (obj = OBJ_ObjectHead ; obj ; obj = obj->next)
    {
        if (obj->Type == OBJECT_TYPE_STAR)
        {
            STAR_OBJ *star = (STAR_OBJ*)obj->Data;

            star->Mode = 1;
            star->Timer = 0.0f;

            SetVector(&star->Vel, player->ownobj->body.Centre.Vel.v[X] / 2.0f, -64.0f, player->ownobj->body.Centre.Vel.v[Z] / 2.0f);

            PlaySfx3D(SFX_PICKUP, SFX_MAX_VOL, 22050, &obj->body.Centre.Pos, 2);

            break;
        }
    }
}

////////////////
// send honka //
////////////////

void SendHonka(void)
{
    SendHeader->Type = MESSAGE_HONKA;
    QueueMessage(SendHeader, sizeof(MESSAGE_HEADER));
}

///////////////////
// process honka //
///////////////////

void ProcessHonka(void)
{
    PLAYER *player;

// honk from player

    player = GetPlayerFromPlayerID(FromID);
    if (player)
    {
        PlaySfx3D(SFX_HONKA, SFX_MAX_VOL, SFX_SAMPLE_RATE, &player->car.Body->Centre.Pos, 2);
    }
}

/////////////////////////
// send a chat message //
/////////////////////////

void SendChatMessage(char *message, DPID to)
{
    SendHeader->Type = MESSAGE_CHAT;
    strncpy((char*)(SendHeader + 1), message, DP_BUFFER_MAX - sizeof(MESSAGE_HEADER));
    SendMessage(SendHeader, sizeof(MESSAGE_HEADER) + strlen(message) + 1, to);
}

////////////////////////////
// process a chat message //
////////////////////////////

void ProcessChatMessage(void)
{
    long i, j;

    if (!LocalPlayerReady)
        return;

    for (i = 0 ; i < WAITING_ROOM_MESSAGE_NUM - 1 ; i++)
    {
        strcpy(WaitingRoomMessages[i], WaitingRoomMessages[i + 1]);
        WaitingRoomMessageRGB[i] = WaitingRoomMessageRGB[i + 1];
    }

    strncpy(WaitingRoomMessages[i], (char*)(ReceiveHeader + 1), WAITING_ROOM_MESSAGE_LEN + MAX_PLAYER_NAME + 2);

    WaitingRoomMessageRGB[i] = 0xffffff;
    for (j = 0 ; j < PlayerCount ; j++)
    {
        if (PlayerList[j].PlayerID == FromID)
        {
            WaitingRoomMessageRGB[i] = MultiPlayerColours[j];
            break;
        }
    }

    WaitingRoomMessageTimer = 5.0f;
}

////////////////////////////
// process system message //
////////////////////////////

void ProcessSystemMessage(void)
{
#ifndef XBOX_DISABLE_NETWORK
    DPMSG_GENERIC *Message = (DPMSG_GENERIC*)ReceiveHeader;

    switch (Message->dwType)
    {

// new player joining

        case DPSYS_CREATEPLAYERORGROUP:
            ProcessPlayerJoining();
            break;

// existing player leaving

        case DPSYS_DESTROYPLAYERORGROUP:
            ProcessPlayerLeaving();
            break;

// session lost

        case DPSYS_SESSIONLOST:
            HostQuit = TRUE;
//          Box(NULL, "Multiplayer Session Was Lost!", MB_OK);
            break;

// become the host if game started

        case DPSYS_HOST:
            ProcessBecomeHost();
            break;
    }
#endif // XBOX_DISABLE_NETWORK
}

//////////////////
// list players //
//////////////////

void ListPlayers(GUID *guid)
{
#ifndef XBOX_DISABLE_NETWORK
    HRESULT r;

    PlayerCount = 0;
    if (!guid) r = DP->EnumPlayers(NULL, EnumPlayersCallback, NULL, DPENUMPLAYERS_ALL);
    else r = DP->EnumPlayers(guid, EnumPlayersCallback, NULL, DPENUMPLAYERS_SESSION);
    if (r != DP_OK && Version == VERSION_DEV)
    {
        ErrorDX(r, "Can't enumerate player list");
    }
#endif // XBOX_DISABLE_NETWORK
}

/* $REMOVED
//////////////////////////
// EnumPlayers callback //
//////////////////////////

BOOL FAR PASCAL EnumPlayersCallback(DPID dpId, DWORD dwPlayerType, LPCDPNAME lpName, DWORD dwFlags, LPVOID lpContext)
{
    DWORD size;
    DPCAPS caps;
    HRESULT r;

// stop if max players

    if (PlayerCount >= MAX_RACE_CARS)
        return FALSE;

// skip if not a player

    if (dwPlayerType != DPPLAYERTYPE_PLAYER)
        return TRUE;

// get caps

    caps.dwSize = sizeof(caps);
    r = DP->GetPlayerCaps(dpId, &caps, NULL);
    if (r != DP_OK)
    {
        ErrorDX(r, "Can't get player caps");
    }

// store spectator / ID / name

    PlayerList[PlayerCount].Spectator = dwFlags & DPENUMPLAYERS_SPECTATOR;
    PlayerList[PlayerCount].Host = caps.dwFlags & DPCAPS_ISHOST;
    PlayerList[PlayerCount].PlayerID = dpId;
    strncpy(PlayerList[PlayerCount].Name, lpName->lpszShortNameA, MAX_PLAYER_NAME);

// get player data

    size = sizeof(DP_PLAYER_DATA);
    r = DP->GetPlayerData(PlayerList[PlayerCount].PlayerID, &PlayerList[PlayerCount].Data, &size, DPGET_REMOTE);
    if (r != DP_OK)
    {
        ErrorDX(r, "Can't get player data");
    }

// return OK

    PlayerCount++;
    return TRUE;
}
$END_REMOVAL */

////////////////////////
// get send queue num //
////////////////////////

unsigned long GetSendQueue(DPID id)
{
#ifndef XBOX_DISABLE_NETWORK
    unsigned long num, size;
    HRESULT r;

    r = DP->GetMessageQueue(0, id, DPMESSAGEQUEUE_SEND, &num, &size);
    if (r != DP_OK)
    {
        num = 0;
    }

    return num;
#else
    return 0;
#endif // XBOX_DISABLE_NETWORK
}

///////////////////////////
// get receive queue num //
///////////////////////////

unsigned long GetReceiveQueue(DPID id)
{
#ifndef XBOX_DISABLE_NETWORK
    unsigned long num, size;
    HRESULT r;

    r = DP->GetMessageQueue(id, 0, DPMESSAGEQUEUE_RECEIVE, &num, &size);
    if (r != DP_OK)
    {
        num = 0;
    }

    return num;
#else
    return 0;
#endif // XBOX_DISABLE_NETWORK
}

/////////////////////////////
// update next packet info //
/////////////////////////////

void UpdatePacketInfo(void)
{

// update next packet ready flag

    NextPacketTimer -= TimeStep;
    if (NextPacketTimer < 0.0f)
    {
        NextPacketTimer = 1.0f / PacketsPerSecond;
        NextPacketReady = TRUE;
    }
    else
    {
        NextPacketReady = FALSE;
    }

// update next sync ready flag

    NextSyncTimer -= TimeStep;
    if (NextSyncTimer < 0.0f)
    {
        NextSyncTimer = DP_SYNC_TIME;
        NextSyncReady = TRUE;
    }
    else
    {
        NextSyncReady = FALSE;
    }

// update next position ready flag

    NextPositionTimer -= TimeStep;
    if (NextPositionTimer < 0.0f)
    {
        NextPositionTimer = DP_POSITION_TIME;
        NextPositionReady = TRUE;
    }
    else
    {
        NextPositionReady = FALSE;
    }
}

//////////////////////////
// check for all loaded //
//////////////////////////

void CheckAllPlayersReady(void)
{
    PLAYER *player;

// appropriate?

    if (AllPlayersReady)
        return;

    if (!IsServer())
        return;

// force if timeout expired

    AllReadyTimeout -= TimeStep;
    if (AllReadyTimeout < 0.0f)
    {
        AllPlayersReady = TRUE;
        SendCountdownStart();
        return;
    }

// check

    for (player = PLR_PlayerHead ; player ; player = player->next)
    {
        if (player->type != PLAYER_NONE && !player->Ready)
            return;
    }

// all ready

    AllPlayersReady = TRUE;
    SendCountdownStart();
}

///////////////////////////////
// choose network connection //
///////////////////////////////

void ConnectionMenu(void)
{
#ifndef XBOX_DISABLE_NETWORK
    short i;
    long col;

// buffer flip / clear

//$REMOVED    CheckSurfaces();
    FlipBuffers();
    ClearBuffers();

// update pos

//$REMOVED    ReadMouse();
//$REMOVED    ReadKeyboard();
    ReadJoystick(); //$ADDITION

    // Process void
	CVoiceCommunicator::ProcessVoice();

    if (Keys[DIK_UP] && !LastKeys[DIK_UP] && MenuCount) MenuCount--;
    if (Keys[DIK_DOWN] && !LastKeys[DIK_DOWN] && MenuCount < ConnectionCount - 1) MenuCount++;

// show menu

    D3Ddevice->BeginScene();

//  BlitBitmap(TitleHbm, &BackBuffer);

    BeginTextState();

    DrawBigGameText(128, 112, 0x808000, "Select Connection:");

    for (i = 0 ; i < ConnectionCount ; i++)
    {
        if (MenuCount == i) col = 0xff0000;
        else col = 0x808080;
        DrawBigGameText(128, i * 48 + 176, col, Connection[i].Name);
    }

    D3Ddevice->EndScene();

// selected?

    if (Keys[DIK_ESCAPE] && !LastKeys[DIK_ESCAPE])
    {
        KillPlay();
        MenuCount = 0;
        SET_EVENT(MainMenu);
    }       

    if (Keys[DIK_RETURN] && !LastKeys[DIK_RETURN])
    {
        InitConnection((char)MenuCount);

        if (IsServer())
        {
            MenuCount = 0;
            SET_EVENT(GetSessionName);
        }

        else
        {
            MenuCount = 0;
            SessionCount = 0;
            SessionRequestTime = 0.0f;
            SET_EVENT(LookForSessions);
        }
    }
#endif // XBOX_DISABLE_NETWORK
}

//////////////////////
// get session name //
//////////////////////

void GetSessionName(void)
{
    unsigned char c;

// buffer flip / clear

//$REMOVED    CheckSurfaces();
    FlipBuffers();
    ClearBuffers();

// get a key

//$REMOVED    ReadMouse();
//$REMOVED    ReadKeyboard();
    ReadJoystick(); //$ADDITION

    // Process void
	CVoiceCommunicator::ProcessVoice();

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
            CreatePlayer(RegistrySettings.PlayerName, DP_SERVER_PLAYER);
            GameStarted = FALSE;
            PlayersRequestTime = 0.0f;
            SET_EVENT(HostWait);
        }

// escape

        else if (c == 27)
        {
            MenuCount = 0;
            KillPlay();
            InitPlay();
            SET_EVENT(ConnectionMenu);
        }

// normal key

        else if (MenuCount < MAX_SESSION_NAME - 2)
        {
            SessionName[MenuCount++] = c;
        }
    }

// print name

    D3Ddevice->BeginScene();

//  BlitBitmap(TitleHbm, &BackBuffer);

    BeginTextState();

    DrawBigGameText(208, 224, 0x808000, "Enter Game Name:");
    SessionName[MenuCount] = '_';
    SessionName[MenuCount + 1] = 0;
    DrawBigGameText(128, 276, 0x808080, SessionName);

    D3Ddevice->EndScene();
}

///////////////////////////////
// look for / join a session //
///////////////////////////////

void LookForSessions(void)
{
#ifndef XBOX_DISABLE_NETWORK
    short i;
    long col;
    char state[128];
    char buf[128];

// buffer flip / clear

//$REMOVED    CheckSurfaces();
    FlipBuffers();
    ClearBuffers();

// read keyboard / mouse / timers

    UpdateTimeStep();
//$REMOVED    ReadMouse();
//$REMOVED    ReadKeyboard();
   ReadJoystick(); //$ADDITION

    // Process void
	CVoiceCommunicator::ProcessVoice();

// dump back piccy

//  BlitBitmap(TitleHbm, &BackBuffer);

// begin scene

    D3Ddevice->BeginScene();

// request sessions?

    SessionRequestTime -= TimeStep;
    if (SessionRequestTime < 0.0f)
    {
        ListSessions();
        SessionRequestTime = 1.0f;

        if (SessionCount)
        {
            if (MenuCount > SessionCount - 1) MenuCount = SessionCount - 1;
            ListPlayers(&SessionList[MenuCount].Guid);
        }
    }

// display sessions

    BeginTextState();

    DrawGameText(264, 16, 0x808000, "Choose a Game:");

    for (i = 0 ; i < SessionCount ; i++)
    {
        if (MenuCount == i) col = 0xff0000;
        else col = 0x808080;
        DrawGameText(168, i * 16 + 48, col, SessionList[i].Name);
        if (SessionList[i].Started) memcpy(state, "STARTED", sizeof(state));
        else memcpy(state, "OPEN", sizeof(state));
        DrawGameText(432, i * 16 + 48, 0x808000, state);
    }

// list players in selected session

    if (!SessionCount) PlayerCount = 0;

    if (PlayerCount)
    {
        DrawGameText(288, 192, 0x808000, "Players:");
        for (i = 0 ; i < PlayerCount ; i++)
        {
            DrawGameText(168, i * 16 + 224, 0xff0000, PlayerList[i].Name);
            wsprintf(buf, "%s", PlayerList[i].Data.CarName);
            DrawGameText(400, i * 16 + 224, 0xff0000, buf);
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
        SET_EVENT(ConnectionMenu);
    }

// join - back to only at start - 10/01/98

//  if (Keys[DIK_RETURN] && !LastKeys[DIK_RETURN] && SessionCount)
    if (Keys[DIK_RETURN] && !LastKeys[DIK_RETURN] && SessionCount && !SessionList[MenuCount].Started)
    {
        GameStarted = FALSE;

        JoinSession((char)MenuCount);

// game not started

        if (!SessionList[MenuCount].Started)
        {
            CreatePlayer(RegistrySettings.PlayerName, DP_CLIENT_PLAYER);
            PlayersRequestTime = 0.0f;
            SET_EVENT(ClientWait);
        }

// game already started

        else
        {
            CreatePlayer(RegistrySettings.PlayerName, DP_SPECTATOR_PLAYER);
            ClientJoinLate();
            SET_EVENT(SetupGame);
        }
    }
#endif // XBOX_DISABLE_NETWORK
}

/////////////////////////
// client joining late //
/////////////////////////

void ClientJoinLate(void)
{
#ifndef XBOX_DISABLE_NETWORK
    long i;
    unsigned long jointime, time, ping;

// save join time

    jointime = CurrentTimer();

// wait for join info

    while (!GameStarted)
    {
        GetRemoteMessages();

        if (TIME2MS(CurrentTimer() - jointime) > 10000)
        {
            DumpMessage(NULL,"No response from host!");

            DP->Close();
            MenuCount = 0;
            GameSettings.MultiType = MULTITYPE_CLIENT;
            SET_EVENT(LookForSessions);
            return;
        }
    }

// calc proper race start time

    time = CurrentTimer();
    ping = time - jointime;
    TotalRaceStartTime = time - MS2TIME(TotalRaceTime) - (ping / 2);
    TotalRaceTime = TotalRacePhysicsTime = TIME2MS(time - TotalRaceStartTime);

// setup start data 

    ListPlayers(NULL);
    StartData.PlayerNum = 0;

    for (i = 0 ; i < PlayerCount ; i++)
    {
        if (PlayerList[i].PlayerID == LocalPlayerID) {
            AddPlayerToStartData(PLAYER_LOCAL, PlayerCount - 1, GetCarTypeFromName(PlayerList[i].Data.CarName), PlayerList[i].Spectator, TotalRacePhysicsTime, CTRL_TYPE_LOCAL, PlayerList[i].PlayerID, PlayerList[i].Name);
            StartData.LocalPlayerNum = i;
        } else {
            AddPlayerToStartData(PLAYER_REMOTE, PlayerCount - 1, GetCarTypeFromName(PlayerList[i].Data.CarName), PlayerList[i].Spectator, TotalRacePhysicsTime, CTRL_TYPE_REMOTE, PlayerList[i].PlayerID, PlayerList[i].Name);
        }
    }
#endif // XBOX_DISABLE_NETWORK
}

//////////////////////////////
// host waiting for players //
//////////////////////////////

void HostWait(void)
{
#ifndef XBOX_DISABLE_NETWORK
    long i, j, k, gridused[MAX_RACE_CARS];
    char buf[128];
    LEVELINFO *levelInfo;

// buffer flip / clear

//$REMOVED    CheckSurfaces();
    FlipBuffers();
    ClearBuffers();

// read keyboard / mouse / timers

    UpdateTimeStep();
//$REMOVED    ReadMouse();
//$REMOVED    ReadKeyboard();
    ReadJoystick(); //$ADDITION

    // Process void
	CVoiceCommunicator::ProcessVoice();

// display current players

    D3Ddevice->BeginScene();

//  BlitBitmap(TitleHbm, &BackBuffer);

    BeginTextState();

    DrawGameText(288, 64, 0x808000, "Players:");
    DrawGameText(216, 400, 0x808000, "Hit Enter To Start Game...");

    PlayersRequestTime -= TimeStep;
    if (PlayersRequestTime < 0.0f)
    {
        ListPlayers(NULL);
        PlayersRequestTime = 2.0f;
    }

    GetRemoteMessages();

    for (i = 0 ; i < PlayerCount ; i++)
    {
        DrawGameText(168, i * 16 + 96, 0xff0000, PlayerList[i].Name);
        wsprintf(buf, "%s", PlayerList[i].Data.CarName);
        DrawGameText(400, i * 16 + 96, 0xff0000, buf);
    }

    D3Ddevice->EndScene();

// quit?

    if (Keys[DIK_ESCAPE] && !LastKeys[DIK_ESCAPE])
    {
        DP->Close();
        SET_EVENT(GetSessionName);
    }

// start game?

    if (Keys[DIK_RETURN] && !LastKeys[DIK_RETURN])
    {

// yep, set session to 'started'

        GameStarted = TRUE;

        Session.dwUser1 = TRUE;
        DP->SetSessionDesc(&Session, NULL);

// setup start data

        ListPlayers(NULL);
        StartData.PlayerNum = 0;
        levelInfo = GetLevelInfo(GameSettings.Level);
        strncpy(StartData.LevelDir, levelInfo->Dir, MAX_LEVEL_DIR_NAME);

        for (i = 0 ; i < PlayerCount ; i++)
            gridused[i] = FALSE;

        for (i = 0 ; i < PlayerCount ; i++)
        {
            k = (rand() % (PlayerCount - i)) + 1;
            for (j = 0 ; j < PlayerCount ; j++)
            {
                if (!gridused[j])
                    k--;

                if (!k)
                    break;
            }

            gridused[j] = TRUE;
            if (PlayerList[i].PlayerID == LocalPlayerID) {
                AddPlayerToStartData(PLAYER_LOCAL, j, GetCarTypeFromName(PlayerList[i].Data.CarName), PlayerList[i].Spectator, TotalRacePhysicsTime, CTRL_TYPE_LOCAL, PlayerList[i].PlayerID, PlayerList[i].Name);
                StartData.LocalPlayerNum = i;
            } else {
                AddPlayerToStartData(PLAYER_REMOTE, j, GetCarTypeFromName(PlayerList[i].Data.CarName), PlayerList[i].Spectator, TotalRacePhysicsTime, CTRL_TYPE_REMOTE, PlayerList[i].PlayerID, PlayerList[i].Name);
            }
            //AddPlayerToStartData(PLAYER_REMOTE, j, PlayerList[i].Data.CarType, PlayerList[i].Spectator, 0, CTRL_TYPE_REMOTE, PlayerList[i].PlayerID, PlayerList[i].Name);
        }

// go!

        SendGameStarted();
        RemoteSyncHost();
        SET_EVENT(SetupGame);
    }
#endif // XBOX_DISABLE_NETWORK
}

/////////////////////////////
// client waiting for host //
/////////////////////////////

void ClientWait(void)
{
#ifndef XBOX_DISABLE_NETWORK
    short i;
    char buf[128];

// buffer flip / clear

//$REMOVED    CheckSurfaces();
    FlipBuffers();
    ClearBuffers();

// read keyboard / mouse / timers

    UpdateTimeStep();
//$REMOVED    ReadMouse();
//$REMOVED    ReadKeyboard();
    ReadJoystick(); //$ADDITION

    // Process void
	CVoiceCommunicator::ProcessVoice();

// display current players

    D3Ddevice->BeginScene();

//  BlitBitmap(TitleHbm, &BackBuffer);

    BeginTextState();

    DrawGameText(288, 64, 0x808000, "Players:");
    DrawGameText(240, 400, 0x808000, "Waiting For Host...");

    PlayersRequestTime -= TimeStep;
    if (PlayersRequestTime < 0.0f)
    {
        ListPlayers(NULL);
        PlayersRequestTime = 2.0f;
    }

    GetRemoteMessages();

    for (i = 0 ; i < PlayerCount ; i++)
    {
        DrawGameText(168, i * 16 + 96, 0xff0000, PlayerList[i].Name);
        wsprintf(buf, "%s", PlayerList[i].Data.CarName);
        DrawGameText(400, i * 16 + 96, 0xff0000, buf);
    }

    D3Ddevice->EndScene();

// quit?

    if ((Keys[DIK_ESCAPE] && !LastKeys[DIK_ESCAPE]) || !IsClient())
    {
        DP->Close();
        MenuCount = 0;
        GameSettings.MultiType = MULTITYPE_CLIENT;
        SET_EVENT(LookForSessions);
    }

// host started?

    if (GameStarted)
    {
        RemoteSyncClient();
        SET_EVENT(SetupGame);
    }
#endif // XBOX_DISABLE_NETWORK
}

//////////////////////
// get host details //
//////////////////////

char GetHostDetails(void)
{
#ifndef XBOX_DISABLE_NETWORK
    int r, i;
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

    if (he->h_addrtype != PF_INET)
        return FALSE;

// store host details

    memcpy(HostName, he->h_name, MAX_HOST_NAME);

    IPcount = 0;
    while (he->h_addr_list[IPcount] && IPcount < 4)
    {
        for (i = 0 ; i < 4 ; i++)
        IP[IPcount][i] = he->h_addr_list[IPcount][i];
        IPcount++;
    }

    if (!IPcount)
        return FALSE;

// return OK

    return TRUE;
#else
    return FALSE;
#endif // XBOX_DISABLE_NETWORK
}

/* $REMOVED
////////////////////////
// check for legal IP //
////////////////////////

char CheckLegalIP(void)
{
    long i;
    LEGAL_IP *lip;

// init play

    if (!InitPlay())
        return FALSE;

// connect to IPX

    for (i = 0 ; i < ConnectionCount ; i++)
    {
        if (Connection[i].Guid.Data1 == 0x685BC400 && Connection[i].Guid.Data2 == 0x9d2c && Connection[i].Guid.Data3 == 0x11cf && *(_int64*)Connection[i].Guid.Data4 == 0xe3866800aa00cda9)
            break;
    }

    if (i == ConnectionCount)
    {
        KillPlay();
        return FALSE;
    }

    if (!InitConnection((char)i))
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
        if ((IP[0][0] & lip->Mask[0]) == lip->IP[0] &&
            (IP[0][1] & lip->Mask[1]) == lip->IP[1] &&
            (IP[0][2] & lip->Mask[2]) == lip->IP[2] &&
            (IP[0][3] & lip->Mask[3]) == lip->IP[3])
                return TRUE;
    }

// illegal IP

    return FALSE;
}
$END_REMOVAL */

///////////////////////////
// get IP address string //
///////////////////////////

long GetIPString(char *str)
{

// get IP address

    if (!GetHostDetails())
    {
        sprintf(str, "IP: Unknown");
        return FALSE;
    }

// build string

    if (IPcount == 1)
        sprintf(str, "IP: %ld.%ld.%ld.%ld", IP[0][0], IP[0][1], IP[0][2], IP[0][3]);
    else
        sprintf(str, "IP: %ld.%ld.%ld.%ld / %ld.%ld.%ld.%ld", IP[0][0], IP[0][1], IP[0][2], IP[0][3], IP[1][0], IP[1][1], IP[1][2], IP[1][3]);

// return OK

    return TRUE;
}

///////////////////////
// sync with clients //
///////////////////////

void RemoteSyncHost(void)
{

// start synchronize

    RemoteSyncTimer = 6.0f;

// loop

    while (RemoteSyncTimer > 0.0f)
    {

// dec timer

        UpdateTimeStep();
        RemoteSyncTimer -= TimeStep;

// get packets

        GetRemoteMessages();
    }
}

////////////////////
// sync with host //
////////////////////

void RemoteSyncClient(void)
{

// start synchronize

    RemoteSyncTimer = 5.0f;
    RemoteSyncBestPing = 0xffffffff;

    NextPacketTimer = 0.0f;
    PacketsPerSecond = 3.0f;

// loop

    while (RemoteSyncTimer > 0.0f)
    {

// dec timer

        UpdateTimeStep();
        RemoteSyncTimer -= TimeStep;

// send sync request?

        UpdatePacketInfo();
        if (NextPacketReady)
        {
            SendSyncRequest();
        }

// get packets

        GetRemoteMessages();
    }
}

////////////////////////////
// process player joining //
////////////////////////////

void ProcessPlayerJoining(void)
{
/*  DPMSG_CREATEPLAYERORGROUP *data;
    DP_PLAYER_DATA *playerdata;
    PLAYER_START_DATA player;
    
// ignore if game not started

    if (!GameStarted)
        return;

// ignore if group

    data = (DPMSG_CREATEPLAYERORGROUP*)ReceiveHeader;
    if (data->dwPlayerType != DPPLAYERTYPE_PLAYER)
        return;

// create player as spectator

    playerdata = (DP_PLAYER_DATA*)data->lpData;

    player.PlayerType = PLAYER_REMOTE;
    player.GridNum = NumPlayers;
    player.CarType = playerdata->CarType;
    player.Spectator = data->dwFlags & DPENUMPLAYERS_SPECTATOR;
    player.StartTime = TotalRacePhysicsTime;
    player.CtrlType = CTRL_TYPE_REMOTE;
    player.PlayerID = data->dpId;
    strncpy(player.Name, data->dpnName.lpszShortNameA, MAX_PLAYER_NAME);

    InitOneStartingPlayer(&player);
    //InitOnePlayerNetwork(NumPlayers, playerdata->CarType, data->dwFlags & DPENUMPLAYERS_SPECTATOR, data->dpId, data->dpnName.lpszShortNameA);

// reply with start data if host

    if (IsServer())
    {
        SendJoinInfo(data->dpId);
    }*/
}

////////////////////////////
// process player leaving //
////////////////////////////

void ProcessPlayerLeaving(void)
{
#ifndef XBOX_DISABLE_NETWORK
    DPMSG_DESTROYPLAYERORGROUP *data;
    PLAYER *player;
    long i;

// game started?

    if (!GameStarted)
        return;

// get data

    data = (DPMSG_DESTROYPLAYERORGROUP*)ReceiveHeader;

// ignore if group

    if (data->dwPlayerType != DPPLAYERTYPE_PLAYER)
        return;

// kill player

    player = GetPlayerFromPlayerID(data->dpId);
    if (player)
    {
        PLR_SetPlayerType(player, PLAYER_NONE);
        CRD_InitPlayerControl(player, CTRL_TYPE_NONE);
        player->ownobj->CollType = COLL_TYPE_NONE;

        for (i = 0 ; i < StartData.PlayerNum ; i++)
        {
            if (StartData.PlayerData[i].PlayerID == player->PlayerID)
            {
                StartData.PlayerData[i].PlayerType = PLAYER_NONE;
            }
        }
    }
#endif // XBOX_DISABLE_NETWORK
}

////////////////////////////
// process player leaving //
////////////////////////////

void ProcessBecomeHost(void)
{

// set me to server if in game

    if (GameStarted)
    {
        GameSettings.MultiType = MULTITYPE_SERVER;
        ServerID = LocalPlayerID;
    }

// else set host quit flag

    else
    {
        HostQuit = TRUE;
    }
}

//////////////////////////////
// message receiving thread //
//////////////////////////////

unsigned long WINAPI ReceiveThread(void *param)
{
#ifndef XBOX_DISABLE_NETWORK
    HANDLE event[2];
    DWORD size;
    HRESULT r;

// set event array

    event[0] = PlayerEvent;
    event[1] = KillEvent;

// loop waiting for player or kill event

    while (WaitForMultipleObjects(2, event, FALSE, INFINITE) == WAIT_OBJECT_0)
    {
        do
        {

// get next message

            size = DP_BUFFER_MAX;
            r = DP->Receive(&FromID, &ToID, DPRECEIVE_ALL, ReceiveHeader, &size);

// save message in receive queue

            if (r == DP_OK)
            {
            }

// until none left

        } while (r != DPERR_NOMESSAGES);
    }

// kill thread

    ExitThread(0);
#endif // XBOX_DISABLE_NETWORK
    return 0;
}

//////////////////////////
// refresh session list //
//////////////////////////

void RefreshSessions(void)
{
#ifndef XBOX_DISABLE_NETWORK
    long i;
    GUID guid;

    if (SessionCount)
    {
        guid = SessionList[SessionPick].Guid;
    }
    else
    {
        ZeroMemory(&guid, sizeof(GUID));
    }

    SessionPick = 0;
    ListSessions();

    if (SessionCount)
    {
        for (i = 0 ; i < SessionCount ; i++)
        {
            if (SessionList[i].Guid == guid)
                SessionPick = (char)i;
        }
        ListPlayers(&SessionList[SessionPick].Guid);
    }
    else
    {
        PlayerCount = 0;
    }
#endif // XBOX_DISABLE_NETWORK
}

/////////////////////////////////
// update multiplayer messages //
/////////////////////////////////

void UpdateMessages(void)
{
    long i;
    char c;

// enable?

    if (!WaitingRoomMessageActive && ( g_Controllers[0].dwMenuInput == XBINPUT_WHITE_BUTTON ) )
    {
        WaitingRoomMessageActive = TRUE;
    }

// dec life counter

    WaitingRoomMessageTimer -= TimeStep;
    if (WaitingRoomMessageTimer < 0.0f)
        WaitingRoomMessageTimer = 0.0f;

// input?

    if (WaitingRoomMessageActive && (c = GetKeyPress()))
    {
        if (c == 27 || c == 9) // escape / tab
        {
            return;
        }

        if (c == 8) // delete
        {
            if (WaitingRoomMessagePos)
                {WaitingRoomMessagePos--;
                WaitingRoomCurrentMessage[WaitingRoomMessagePos] = 0;
            }
        }

        else if (c == 13) // enter
        {
            for (i = 0 ; i < WAITING_ROOM_MESSAGE_NUM - 1 ; i++)
            {
                strcpy(WaitingRoomMessages[i], WaitingRoomMessages[i + 1]);
                WaitingRoomMessageRGB[i] = WaitingRoomMessageRGB[i + 1];
            }

            wsprintf(WaitingRoomMessages[i], "%s: %s", g_TitleScreenData.nameEnter, WaitingRoomCurrentMessage);
            SendChatMessage(WaitingRoomMessages[i], GroupID);

            WaitingRoomMessageRGB[i] = MultiPlayerColours[PLR_LocalPlayer->Slot];
            WaitingRoomMessagePos = 0;
            WaitingRoomCurrentMessage[0] = 0;

            WaitingRoomMessageActive = FALSE;
            WaitingRoomMessageTimer = 5.0f;
        }

        else if (WaitingRoomMessagePos < WAITING_ROOM_MESSAGE_LEN - 1) // normal char
        {
            WaitingRoomCurrentMessage[WaitingRoomMessagePos++] = c;
            WaitingRoomCurrentMessage[WaitingRoomMessagePos] = 0;
        }
    }
}
