/*
MAYBE_KEEPERS:

* Session: dwMaxPlayers ?? (== DEFAULT_RACE_CARS)

* LocalPlayerData: Ready flag -- do we want it?  (If so, need to update it.)
  --> The 'Ready' flag only gets used in a couple places (SendGameLoaded, ProcessGameLoaded, CheckAllPlayersReady)
  --> I'm not even sure we need that flag; maybe assume that load will take same amount of time on all machines?  (And sync messages account for time diffs, right?)
  --> But if do use it, seems like might want per-machine, NOT per-player (b/c only used to tell when game is loaded, right?)

NON-KEEPER NOTES:

* Session: dwUser1 == bStarted
*/


/*

$CMP_NOTE:  Right now, we don't get PlayerList/PlayerCount until we join a session.  (So seeing list of players before joining session is not yet supported.  Maybe matchmaking will fix this for the non-system-link case.)

$BUG: don't really want PlayerID in the message header. Need a way to determine player w/o that...

$HEY: can we remove CarName globally, and just use CarType? (b/c user won't be creating arbitrary cars; we'll be telling them what cars are available) (but maybe an issue if user plays against someone with new car, which he hasn't downloaded yet, and which doesn't exist locally!?)

$TODO: remove, in all files, all XBOX_DISABLE_NETWORK #ifdef lines!!

$HEY: TotalDataSent isn't being updated (b/c calling winsock stuff directly, instead of going through SendMessage / SendMessageGuaranteed!)

$HEY: replace SendMessage**() calls with send/sendto !? (but they also update a TotalDataSent var in those funcs)
 --> So maybe update send/sendto call to use SendMessage** ??

$HEY - (Acclaim Aug99) here are some new fields in SessionList[] that get set in EnumSessionsCallback <what about elsewhere?>
(But note that SessionList is a static var, so at least effects aren't far-reaching, HOPEFULLY.)
    SessionList[SessionCount].Started = lpSessionDesc->dwUser1;
    SessionList[SessionCount].GameType = lpSessionDesc->dwUser2;
    SessionList[SessionCount].Version = lpSessionDesc->dwUser3;
    SessionList[SessionCount].RandomCars = lpSessionDesc->dwUser4 & 1;
    SessionList[SessionCount].RandomTrack = lpSessionDesc->dwUser4 & 2;
    SessionList[SessionCount].PlayerNum = lpSessionDesc->dwMaxPlayers;
*/    


//-----------------------------------------------------------------------------
// File: network.cpp
//
// Desc: Low-level networking code for Re-Volt.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------


// NOTES: LOW-LEVEL NETWORKING IN RE-VOLT (CPrince ; Dec 2001)
//
// Here is the high-level protocol currently used during the connection phase:
// (1) SERVER: Create session.  (Listen for TCP connections, and respond to
//       UDP find-session requests.)
// (2) CLIENT: Find sessions, via broadcast UDP request.
// (3) CLIENT: Join session by making a TCP connection to the server.
// (4) CLIENT: Add players to the session by sending TCP requests to server.
// (5) SERVER: Respond to add-players requests via TCP.
// (6) BOTH:   <players send UDP packets to get ping times of all others>
// (7) SERVER: Start game.  (Send level name, final player list, etc to all
//       players, via TCP.)
//
// Basically we use UDP for session-discovery and ping-times, and TCP for all
// other connection setup.  (But we use UDP for in-game messages.)  Using TCP
// during setup is convenient because it gives us things like guaranteed and
// in-order delivery.  We bind UDP sockets to a port on all machines (because
// these sockets get used for peer-to-peer broadcast messaging, etc), but we
// only bind TCP sockets to a port on the server.
// $REVISIT: IS THE STATEMENT ABOUT BINDING STILL TRUE?  I DON'T THINK SO.
//
// Note that we might have multiple players on a single machine.  As a result,
// we track players and machines separately.  Sending a message to every
// player might result in unnecessary overhead, because we only need to send
// each message to each *machine* one time.



//$CMP_NOTE: Should we move all the low-level protocol stuff (eg, filling message header and bodies, and processing it) into separate file ??
/// (This would hide the complexity of that stuff, similar to how DPlay hid that part.  But not clear that we want to hide that stuff.)
/// (Maybe better would be to make send/receive funcs, to hide that complexity.  EG: SendJoinRequest() / RecvJoinRequest(), and other code just calls these guys.)

//$CMP_NOTE: we're currently giving each player a PlayerID.  Once the game
/// starts, we only use this for determining which player struct to update
/// when we receive a MSG_CAR_DATA.  Other than that, the original
/// codebase only used PlayerID as follows: (a) game code used it DURING
/// session-init, and (b) DPlay used it when indicating a removed player.
/// So we might not need the PlayerID once the game starts, and instead we
/// could just use the player's index in PlayerList, which will be the same
/// on all machines.  Just a thought.

//$CMP_NOTE:  Random thought: if the Xbox net connection is secure,
/// does that mean it's safe to use more client-side logic, as opposed to
/// an authoritative server model?  Though there are still some things you
/// might need to coordinate (eg, car-to-car collisions, two players
/// picking up same object, etc).  But you might be able to use clever
/// game design to work around some of these contention issues.

     

#include "revolt.h"
#include "network.h"
#include "net_xonline.h"
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
#include "panel.h"
#include "pickup.h"
#include "gameloop.h"

#include "ui_Menu.h"
#include "ui_MenuText.h"
#include "ui_TitleScreen.h"
#include "ui_StateEngine.h"
#include "ui_WaitingRoom.h"
#include "ui_ShowMessage.h"  // for displaying error messages to user
#include "ui_players.h"
#include "ui_friends.h"
#include "ui_options.h" // for voice mask preset

#include "VoiceManager.h"
#include "FriendsManager.h"
#include <winsockx.h>


// globals


//$REVISIT(cprince): I added some code to detect the "TCP socket terminated" return values from recv().  But a LONG time can elapse between when one side dies and when the other side detects it.
/// So might need heartbeat messages anyway?  (And if so, do we want/need the extra code to check for special return values from recv()?  See GetSingleMessageTCP -- inside function body, and at locations where it's called.)

//$REVISIT(cprince): should all these buffers be allocated at runtime, instead of statically?
/// (That's what Acclaim did in Aug99 drop of Re-Volt.  But if we need to support worst-case conditions, maybe it doesn't matter, and static allocation reduces chance of mem fragmentation.)

char  SendMsgBuffer[PACKET_BUFFER_SIZE];
char  RecvMsgBufferUDP[PACKET_BUFFER_SIZE];
char* RecvMsgBuffer;  // pointer to actual buffer, b/c need different receive buffers for each TCP connection, and one for UDP
int   RecvMsgSize;

NET_MACHINE MachineList[MAX_NUM_MACHINES];  // NOTE: [0] is always the server machine
long        MachineCount;  //$NOTE: MachineList/MachineCount were static until we started accessing from online task pump.
static NET_MACHINE* const pMachineServer = &MachineList[0]; // for readability of client code (since clients only use MachineList[0], but server may use all entries)
NET_MACHINE* pMachineLocal = NULL;  // makes it easier to skip self when looping through machine list

                                       //$TODO(cprince) better name for MAX_RACE_CARS  //$NOTE: this is modified; was MAX_NUM_PLAYERS before!  (and all code below has been modified as well)
NET_PLAYER PlayerList[MAX_RACE_CARS];  //$TODO(cprince) This var used to be static, but ugly Acclaim Aug99 code broke that.  Would be nice to fix this...
long       PlayerCount;  //$TODO(cprince) This var used to be static, but ugly Acclaim Aug99 code broke that.  Would be nice to fix this...
//$REVISIT(cprince): should verify that PlayerList and PlayerCount are only used for network games.  (I'm pretty sure they are, but should check Acclaim Aug99 code.  Interesting note: what about split-screen?  Probably not used there...)

PLAYER_LIST DepartedPlayerList;

NET_SESSION SessionList[MAX_SESSION_SEARCH_RESULTS];
                                       //$TODO(cprince) This var used to be static, but ugly Acclaim Aug99 code broke that.  Would be nice to fix this...
static char SessionCount;
char GetSessionCount() { return SessionCount; } // accessor to prevent external code from modifying value
bool DisableSessionListUpdate = false;  // to avoid changing list entries (eg, user selects a session but session-discovery message comes back and updates list behind the scenes)

NET_SESSION SessionCurr;  //$NOTE: this was originally named Session  //$NOTE: this was originally static (until we needed to access from online task pump)
//static char SessionName[MAX_SESSION_NAME];  //$REVISIT: might want to put this back, if we allow players to pick a session name for network games.


//$MOVED_ELSEWHERE START_DATA StartData;  HEY -- THIS IS IN InitPlay.* NOW.
/// (BUT THERE'S StartData, StartDataStorage, and MultiStartData !!!)
  //$HEY: Do we really need MultiStartData?  Only gets written in ProcessGameStarted, and only gets read in SetRaceData [titlescreen.cpp] where they do "StartData = MultiStartData"
  //$NOTE: StartDataStorage only seems to be used for Replay mode ?!
  //$NOTE: RestartData is a subset of StartData that (only) gets used for minor updates when restarting race.




//
// Acclaim Aug99 stuff
//
REAL PacketsPerSecond = DP_PACKETS_PER_SEC;
REAL NextPacketTimer, NextSyncTimer, NextPositionTimer;
REAL AllReadyTimeout;
//$NOTE: the default latency vs bandwidth value (for SessionFlag) was DPSESSION_OPTIMIZELATENCY
long NextPositionReady, NextPacketReady, NextSyncReady, AllPlayersReady, HostQuit, LocalPlayerReady;
long NextSyncMachine;  //$MODIFIED: changed NextSyncPlayer to NextSyncMachine

static char  MessageQueue[PACKET_BUFFER_SIZE];  //$REVISIT: should we rename this?  should we dynamically allocate (same issue as with SendMsgBuffer)
long MessageQueueSize;  // global var, so 0 initially    //$TODO: cleanup code so we don't have to expose this (and can make static)
//$REMOVEDlong TotalDataSent;

//$REMOVED
//DPSESSIONDESC2 Session; //$NOTE: no reason for this to be exposed!  (not used outside of play.cpp/network.cpp)
//$END_REMOVAL
//$REMOVED (tentative!!)
//char ConnectionCount, SessionCount;
//DP_SESSION SessionList[SESSION_MAX];
//DP_CONNECTION Connection[CONNECTION_MAX];
//DPLCONNECTION *LobbyConnection;
//$END_REMOVAL (tentative!!)
char SessionPick;
RESTART_DATA RestartData;
static long IPcount;
static long JoinFlag;
static float RemoteSyncTimer;
static unsigned long RemoteSyncBestPing, RemoteSyncHostTime, RemoteSyncClientTime;
static HANDLE KillEvent = NULL;
//$REMOVED_UNREACHANGE (tentative!! do we want a receive thread?)
//static HANDLE ReceiveThreadHandle = NULL;
//static DWORD ReceiveThreadId = 0;
//$END_REMOVAL
//$REMOVEDDPID ToID;
//$REMOVED DPID GroupID;  // was always set to DPID_ALLPLAYERS when not using DPlay groups (which we're not)
//$BUG: not setting GroupID=DPID_ALLPLAYERS (which is what they use if no DPlay groups)

BYTE g_bInvitedByFriend = 0;  // non-zero if joining session due to XOnline Friend invitation; else zero.

//$HACK(Apr02_GameBash) - matchmaking parameters are colliding everywhere
LONG g_lMatchmakingLevelNum;





// macros to extract/check the IN_ADDR part of a SOCKADDR_IN
#define SAME_INADDR(x,y)      ( (x).sin_addr.S_un.S_addr == (y).sin_addr.S_un.S_addr )

/*static*/ SOCKET      soUDP = INVALID_SOCKET;  //$CMP_NOTE: (might want to un-expose this) only used in SendLocalCarData()
static     SOCKET      soListenTCP = INVALID_SOCKET;  // only used by server

SOCKADDR_IN  SoAddrFrom;
SOCKET       SocketFrom_TCP;  // only valid when processing TCP data; used for sending replies
XNADDR       XnAddrLocal;

// Union of zero for all types
const ZERO_UNION g_Zero = {0};

// Note: for security reasons, SoAddrBroadcast can only be used when
// initializing SysLink games.
const SOCKADDR_IN SoAddrBroadcast =
{
    AF_INET,
    htons(GAME_PORT),
    { ((INADDR_BROADCAST & 0x000000FF) >>  0) ,
      ((INADDR_BROADCAST & 0x0000FF00) >>  8) ,
      ((INADDR_BROADCAST & 0x00FF0000) >> 16) ,
      ((INADDR_BROADCAST & 0xFF000000) >> 24)
    },
    {0,0,0,0,0,0,0,0}
};

DWORD dwLocalPlayerCount = 1;  //$BUGBUG: this should be set elsewhere, and using "1" wrongly assumes only a single local player !!

WORD LocalPlayerID = INVALID_PLAYER_ID;  //$BUGBUG: this assumes only a single local player !!
DWORD FromID, ServerID;  //$NOTE: type was DPID originally  //$REVISIT: using FromID and ServerID is kind of ugly (leftover from porting Acclaim DPlay code); I'd like to remove them eventually.
//$TODO: to match LocalPlayerID, should probably change FromID and ServerID to WORD (or whatever it currently is)
/// (Consider using a PLAYER_ID typedef for these things, similar to what we did for GLOBAL_ID, so that we can change it globally more easily.)


BOOL bGameStarted;  //$HEY: this var used to be static (before Aug99 merge); I wonder if we can go back to that...
                    //$REVISIT: this is a bad var name; g_bNetGameStated would be better (notice *NET*), though still doesn't differentiate b/w pre-waiting-room and post-waiting-room.  (This var is true when we're post-waiting-room.)


//$REMOVED float PlayersRequestTime;
//$REMOVED float SessionRequestTime; //$CMP_NOTE: might want to un-expose this eventually



// Function prototypes (functions local to current file)

int GetSingleMessageTCP( SOCKET socket, char* pRecvBuffer,
                          short* pMsgCurrBytes, short* pMsgTotalBytes );

int ProcessFindSession(void);
int ProcessSessionFound(void);
int ProcessRequestAddPlayers(void);
int ProcessAddPlayersAccepted(void);
int ProcessAddPlayersRejected(void);

void SendPlayerList(void);
int  ProcessPlayerList(void);
void SendChatterList(void);
int  ProcessChatterList(void);

void OnPlayerAdded( NET_PLAYER* pPlayer, BOOL bPlaySound );
void OnPlayerRemoved( NET_PLAYER* pPlayer, BOOL bPlaySound );
void OnSessionEntered();
void OnSessionExited();



static int s_NetInitRefCount = 0;



//-----------------------------------------------------------------------------
// Name: InitNetwork
// Desc: Initializes network functionality.
//-----------------------------------------------------------------------------
bool InitNetwork(void)
{
    //$TODO: this function isn't so robust (but it's clean!) -- should do better job of checking return codes, avoiding multiple-init, avoiding partial-init, etc.

    int i;
    int iRetVal;

    // Already initialized? (Return if ref count is positive.)
    if( s_NetInitRefCount > 0 )
    {
        s_NetInitRefCount++;
        return TRUE;  // return success
    }

#if 1  // set to 1 to use non-secure xnet packets (but then startup takes longer; bug maybe?)
    XNetStartupParams xnsp;
    ZeroMemory( &xnsp, sizeof(xnsp) );
    xnsp.cfgSizeOfStruct = sizeof(xnsp);
    xnsp.cfgFlags |= XNET_STARTUP_BYPASS_SECURITY;
    //$BUGBUG - did this to fix 6-box case from dropping packets.  But isn't clear whether problem was fixed by doing this, or by not sending message queue when length is zero.  Need to investigate (because maybe this change had no effect and thus is wasteful).  Or might actually want to be setting different param (see comments in WinsockX.h).
    xnsp.cfgPrivatePoolSizeInPages = 24;
//$DEBUG_BEGIN (should probably remove this!?) (but it really decreases time req'd for XNetGetTitleXnAddr to succeed, unless that has been fixed)
//xnsp.cfgFlags |= XNET_STARTUP_BYPASS_DHCP;
//$DEBUG_END
    iRetVal = XNetStartup( &xnsp );
#else
    iRetVal = XNetStartup( NULL );  // pass NULL for default parameters
#endif
    if( iRetVal != 0 )
    {
        return FALSE;
    }

    WSADATA wsaData;
    iRetVal = WSAStartup( MAKEWORD(2,2), &wsaData );
    if( iRetVal != 0 )
    {
        return FALSE;
    }


    //
    // Setup a broadcast-enabled UDP socket.
    //
    soUDP = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );

    SOCKADDR_IN sa;
    sa.sin_family      = AF_INET;
    sa.sin_addr.s_addr = INADDR_ANY;
    sa.sin_port        = htons(GAME_PORT);
    iRetVal = bind( soUDP, (sockaddr*)&sa, sizeof(SOCKADDR_IN) );
    assert( 0 == iRetVal );

    // put socket in non-blocking mode
    DWORD dwNonBlocking = 1;
    ioctlsocket( soUDP, FIONBIO, &dwNonBlocking );

    // enable broadcasting for socket
    int iBoolSockOpt = 1;
    setsockopt( soUDP, SOL_SOCKET, SO_BROADCAST, (char*)&iBoolSockOpt, sizeof(int) );

    //$BUGBUG: do we need htons() / ntohs() ??  It's used for some fields in SNL white paper and XDK snippets, but I thought I read something about it being removed (or at least unnecessary).
    /// If we need it, where exactly, and where not ??


    //
    // Get the XNADDR of this machine
    //
    while( XNET_GET_XNADDR_PENDING == XNetGetTitleXnAddr(&XnAddrLocal) )
    {
        // loop while pending; OK to do other work here
    }


//$REMOVED -- DON'T NEED "DP_SESSION Session"
//HEY -- AUG99 ADDITION (from LobbyConnect; this *is* where snippet belongs)
//
//LobbyConnection->lpSessionDesc->dwMaxPlayers = DEFAULT_RACE_CARS;
//
//// copy session desc
//memcpy(&Session, LobbyConnection->lpSessionDesc, sizeof(DPSESSIONDESC2));
//
//// set initial session description if host
//if (LobbyConnection->dwFlags == DPLCONNECTION_CREATESESSION)
//{
//    LEVELINFO *levinf = GetLevelInfo(gTitleScreenVars.iLevelNum); //$NOTE: ONLY NEEDED FOR THE CALL TO SetSessionDesc
//    SetSessionDesc(gTitleScreenVars.nameEnter[0], levinf->Dir, FALSE, GAMETYPE_NETWORK_RACE, gTitleScreenVars.RandomCars, gTitleScreenVars.RandomTrack);  //$NOTE: don't need this call
//}
//$END_REMOVAL_AUG99


//$AUG99 ADDITION
// create player
//$MODIFIEDCreatePlayer(LobbyConnection->lpPlayerName->lpszShortNameA, LobbyConnection->dwFlags == DPLCONNECTION_CREATESESSION ? DP_SERVER_PLAYER : DP_CLIENT_PLAYER);
//$NOTE: not clear where lpPlayerName comes from...
//$BUG: assumes only 1 local player!
//$BUG: do we really want/need this line here?
  //$REMOVED_DOESNOTHING CreatePlayer("foobar_Joe", 0);
//$END_ADDITION_AUG99



    // Make sure other socket vars are init'd correctly
    for( i=0 ; i < MAX_NUM_MACHINES ; i++ )
    {
        MachineList[i].Socket = INVALID_SOCKET;  // b/c INVALID_SOCKET != 0
    }



    // Return success
    s_NetInitRefCount++;  // increment ref count
    return TRUE;
}


//-----------------------------------------------------------------------------
// Name: KillNetwork
// Desc: Un-initializes network functionality.
//-----------------------------------------------------------------------------
void KillNetwork(void)
{
    int i;

    // Decrement reference count, and return if ref count isn't zero
    assert( s_NetInitRefCount > 0 );
    s_NetInitRefCount--;
    if( s_NetInitRefCount > 0 )  return;


    // Close any open sockets
    if( soUDP != INVALID_SOCKET )
    {
        shutdown( soUDP, SD_BOTH );
        closesocket( soUDP );
        soUDP = INVALID_SOCKET;
    }

    // Make sure other sockets were un-init'd correctly
    for( i=0 ; i < MAX_NUM_MACHINES ; i++ )
    {
        assert( MachineList[i].Socket == INVALID_SOCKET );
    }

    //$REVISIT: do we want/need to wait for pending socket I/O to finish ??  (Recall shutdown() before closehandle(), and/or the SO_LINGER,SO_DONTLINGER options.)

    // Uninitialize the network
    // (OK if never initialized; will just produce a different return value.)
    WSACleanup();
    XNetCleanup();

    //$REVISIT: should we do a better job of checking return codes, etc. here??  (Doesn't seem necessary for network uninit, according to docs.)
}



//-----------------------------------------------------------------------------
// Name: CreateSession
// Desc: Creates a new session (which others can enumerate and join).
//-----------------------------------------------------------------------------
bool CreateSession()
{
    InitNetwork();

    // Fill some fields of the SessionCurr structure
    SessionCurr.XnAddrServer = XnAddrLocal;
    swprintf( SessionCurr.wstrHostNickname, L"%S", gTitleScreenVars.PlayerData[0].nameEnter );  //$BUG: assumes player [0] is the host
    SessionCurr.Started = 0;
    SessionCurr.GameType = GameSettings.GameType;
    SessionCurr.Version = MultiplayerVersion;
    SessionCurr.RandomCars  = 0;  //$TODO: not yet implemented (clients can't get the info via matchmaking server)
    SessionCurr.RandomTrack = 0;  //$TODO: not yet implemented (clients can't get the info via matchmaking server)
    DWORD dwNumLocalPlayers = gTitleScreenVars.numberOfPlayers;
    SessionCurr.nMaxPlayersPublic  = gTitleScreenVars.numberOfPublicSlots;
    SessionCurr.nMaxPlayersPrivate = gTitleScreenVars.numberOfPrivateSlots;
    // Note: SessionCurr.nCurrPlayers** will be set in CreateLocalServerPlayers below.
    SessionCurr.iLevelNum = gTitleScreenVars.iLevelNum;

    // Setup PlayerList/PlayerCount and do other minor work to create server players
    PlayerCount = 0;  //$TODO: can probably remove most instances of "PlayerCount = 0" in the code, since we're doing it here.
    SessionCurr.nCurrPlayersPublic  = 0;
    SessionCurr.nCurrPlayersPrivate = 0;
    CreateLocalServerPlayers();  //$NOTE: call b/4 SendLocalChatters b/c that func assumes LocalPlayerID has already been initialized.
                                 //$NOTE: also should do this (esp. init PlayerList/PlayerCount) before we listen() for new connections, so we don't run out of room for local server players!
    assert( SessionCurr.nCurrPlayersPublic == min(dwNumLocalPlayers, (DWORD)gTitleScreenVars.numberOfPublicSlots) );  // put local players in public slots if possible
    assert( SessionCurr.nCurrPlayersPrivate == (dwNumLocalPlayers - SessionCurr.nCurrPlayersPublic) );  // put remaining local players in private slots


    //$REVISIT: do we need to check return values below?  (In other words, can this ever fail, and if it does, can we recover from it?)

    // Do work specific to SysLink or XOnline networking.
    // Generate and register session keys, update MachineList/MachineCount,
    // and finish updating the SessionCurr structure.
    if( ! gTitleScreenVars.bUseXOnline )
    {
        // *** SYSLINK CREATE SESSION ***

        SessionCurr.wstrHostUsername[0] = '\0';

        //$REVISIT: do we need to check return values here?  (In other words, can this ever fail, and if it does, can we recover from it?)

        // Create/register session keys, and fill-in the SessionCurr structure
        XNetCreateKey( &(SessionCurr.keyID), &(SessionCurr.key) );
        XNetRegisterKey( &(SessionCurr.keyID), &(SessionCurr.key) );

        // When we know our session ID, start listening for QoS probes from clients
        XNetQosListen( &(SessionCurr.keyID), // (ptr to) session XNKID
                       NULL, // (used with _SET_DATA) ptr to game data block
                       0,    // (used with _SET_DATA) num bytes in game data block
                       0,    // (used with _SET_BITSPERSEC) bits/sec value
                       XNET_QOS_LISTEN_ENABLE // subset of the XNET_QOS_LISTEN_ flags
                     );

        // Add ourselves to the global MachineList
        assert( 0 == MachineCount );
        MachineList[0].Socket = INVALID_SOCKET;
        MachineList[0].XnAddr = XnAddrLocal;
        MachineList[0].SoAddr.sin_family = AF_INET;
        MachineList[0].SoAddr.sin_port   = htons(GAME_PORT);
        XNetXnAddrToInAddr( &(XnAddrLocal),
                            &(SessionCurr.keyID),
                            &(MachineList[0].SoAddr.sin_addr) );
        MachineCount = 1;
        OnSessionEntered();
    }
    else
    {
        // *** XONLINE CREATE SESSION ***
    
        swprintf( SessionCurr.wstrHostUsername, L"%S",
                  //$SINGLEPLAYER: Assuming single player at Players[0]
                  Players[0].XOnlineInfo.pXOnlineUser->szGamertag ); //$BUG: assumes player [0] is the host

        // Setup session attribute info
        if( SessionCurr.GameType == GAMETYPE_NETWORK_RACE ) {
            g_rgAttribs_MatchCreate[0].info.integer.qwValue = MATCH_GAMEMODE_RACE;
        } else {
            assert( SessionCurr.GameType == GAMETYPE_NETWORK_BATTLETAG );
            g_rgAttribs_MatchCreate[0].info.integer.qwValue = MATCH_GAMEMODE_BATTLETAG;
        }
        g_rgAttribs_MatchCreate[1].info.integer.qwValue = SessionCurr.iLevelNum; // track ID
        g_rgAttribs_MatchCreate[2].info.string.lpValue = SessionCurr.wstrHostUsername;
        g_rgAttribs_MatchCreate[3].info.string.lpValue = SessionCurr.wstrHostNickname;

        DWORD dwNumFilledSlotsPublic = SessionCurr.nCurrPlayersPublic;
        DWORD dwNumOpenSlotsPublic   = SessionCurr.nMaxPlayersPublic - SessionCurr.nCurrPlayersPublic;
        DWORD dwNumFilledSlotsPrivate = SessionCurr.nCurrPlayersPrivate;
        DWORD dwNumOpenSlotsPrivate   = SessionCurr.nMaxPlayersPrivate - SessionCurr.nCurrPlayersPrivate;

        // Create the XOnline session
        XONLINETASK_HANDLE hTask;
        HRESULT hr;
        hr = XOnlineMatchSessionCreate(
                 dwNumFilledSlotsPublic,  // public slots filled
                 dwNumOpenSlotsPublic,    // public slots open
                 dwNumFilledSlotsPrivate, // private slots filled
                 dwNumOpenSlotsPrivate,   // private slots open
                 NUM_XATTRIB_MATCHCREATE, // num session attributes (for session-create)
                 g_rgAttribs_MatchCreate, // array of session attributes (for session-create)
                 NULL,                    // [optional] event to signal when processing reqd
                 &hTask                   // ptr to task handle for session-create
             );
        assert( hr == S_OK );  //$TODO: more robust error handling

        //$REVISIT: if the above function reached into g_rgAttribs_MatchCreate and modified values (especially setting fChanged to TRUE), then undo that here.

        // Add session-create task to the list of pending tasks
        OnlineTasks_Add( hTask, TASK_MATCH_CREATE );

        // Note: will register session keys and update MachineList/MachineCount
        // when the session-create task has completed.
    }


    //$REVISIT: if TCP connections are only used for the syslink case (and not with XOnline matchmaking), then move the following code.

    // Create TCP socket to listen for new connections from clients
    soListenTCP = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );

    SOCKADDR_IN sa;
    sa.sin_family      = AF_INET;
    sa.sin_addr.s_addr = INADDR_ANY;
    sa.sin_port        = htons(GAME_PORT);
    int iRetVal = bind( soListenTCP, (sockaddr*)&sa, sizeof(SOCKADDR_IN) );
    assert( 0 == iRetVal );

    listen( soListenTCP, SOMAXCONN );

    // put socket in non-blocking mode
    DWORD dwNonBlocking = 1;
    ioctlsocket( soListenTCP, FIONBIO, &dwNonBlocking );


    // Initialize other values
    pMachineLocal = pMachineServer;


    return TRUE;
}


//-----------------------------------------------------------------------------
// Name: DestroySession
// Desc: Destroys the session that was previously created.
//-----------------------------------------------------------------------------
void DestroySession(void)
{
    assert( IsServer() );  //$CMP_NOTE: this isn't really necessary...

    // Do work specific to SysLink or XOnline networking
    if( ! gTitleScreenVars.bUseXOnline )
    {
        // *** SYSLINK DESTROY SESSION ***
        ; // No work required that's specific to system-link play
    }
    else
    {
        // *** XONLINE DESTROY SESSION ***

        // If session was created but game wasn't started, then delete session
        // from matchmaking server, to stop advertising it.
        if( soListenTCP != INVALID_SOCKET )  //$REVISIT: should we be checking '!bGameStarted' instead?  (Should only delete if MatchCreate was called?  Or only if it succeeded?)
        {
            XONLINETASK_HANDLE hTask;
            HRESULT hr;
            hr = XOnlineMatchSessionDelete(
                     SessionCurr.keyID, // session XNKID
                     NULL,          // [optional] event to signal when processing reqd
                     &hTask         // ptr to task handle for session-delete
                 );
            assert( hr == S_OK );  //$TODO: more robust error handling

            //$HEY: If delete fails, do we need to avoid clearing the keyID below, and try again later?  Sheesh, I hope not...

            OnlineTasks_Add( hTask, TASK_MATCH_DELETE );
        }
    }


    //$REVISIT: if TCP connections are only used for the syslink case (and not with XOnline matchmaking), then move the following code.

    // Close the TCP socket that was listening for new connections from clients
    // (if it's still open)
    if( soListenTCP != INVALID_SOCKET )
    {
        shutdown( soListenTCP, SD_BOTH );
        closesocket( soListenTCP );
        soListenTCP = INVALID_SOCKET;
    }
    // Also stop responding to QoS probes from clients
    XNetQosListen( &(SessionCurr.keyID), // (ptr to) session XNKID
                   NULL, // (used with _SET_DATA) ptr to game data block
                   0,    // (used with _SET_DATA) num bytes in game data block
                   0,    // (used with _SET_BITSPERSEC) bits/sec value
                   XNET_QOS_LISTEN_RELEASE // subset of the XNET_QOS_LISTEN_ flags
                 );
    //$REVISIT: do we need to check that the keyID is valid before calling this?


    // Close any TCP sockets connected to client machines
    for( int i = 1 ; i < MachineCount ; i++ )  // start at [1] b/c [0] is local machine (ie, the server)
    {
        assert( MachineList[i].Socket != INVALID_SOCKET ); //$REVISIT: do we need to check Socket is valid before trying to close it (in case player left earlier) ??
        shutdown( MachineList[i].Socket, SD_BOTH );
        closesocket( MachineList[i].Socket );
        MachineList[i].Socket = INVALID_SOCKET;  //$CMP_NOTE: probably not necessary...
    }
    MachineCount = 0;

    OnSessionExited();

    // Unregister the session key, and clear the SessionCurr structure
    XNetUnregisterKey( &(SessionCurr.keyID) );
    ZeroMemory( &SessionCurr, sizeof(SessionCurr) );

    //$HEY: should we set PlayerCount=0 here?  (Depends on whether external code -- eg UI -- reads that value.)
    PlayerCount = 0;
    pMachineLocal = NULL;
    bGameStarted = FALSE;  // make sure the 'started' flag is cleared

    KillNetwork();
}


//-----------------------------------------------------------------------------
// Name: JoinSession
// Desc: Joins an existing sessions (which has been enumerated).
//-----------------------------------------------------------------------------
BOOL JoinSession(int iSession)
{
    int iRetVal;

    InitNetwork();

    // Register the session key information
    SessionCurr = SessionList[iSession];
    XNetRegisterKey( &(SessionCurr.keyID), &(SessionCurr.key) );
    //$BUGBUG(API): we shouldn't have to do XNetRegisterKey before XNetXnAddrToInAddr (where key data is explicit args!!)

    // Compute server's address info, using its XNADDR/XNKID
    XNetXnAddrToInAddr( &(SessionCurr.XnAddrServer), &(SessionCurr.keyID),
                        &(pMachineServer->SoAddr.sin_addr) );
    pMachineServer->SoAddr.sin_family = AF_INET;
    pMachineServer->SoAddr.sin_port   = htons(GAME_PORT);
    pMachineServer->XnAddr = SessionCurr.XnAddrServer;

    //$REVISIT: should we zero-out 'pMachineServer->SoAddr' before filling-in ?? (to avoid problems when struct used multiple times, sequentially)
    //$REVISIT: should we zero-out 'SessionCurr' if the join fails !?!  (Probably not necessary.)


    // Try to join the session by initiating a TCP connection with the server.
    // (Notice: no explicit binding; will implicitly bind during connect call.)

//$TODO: should probably have a timeout here, so that we don't waste too much
/// time trying to connect to non-existent games.

    // (Notice: socket is initially in blocking mode, for connection phase.)
    pMachineServer->Socket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
    pMachineServer->RecvMsgCurrBytes  = 0;  // Must reset these values when .Socket is init'ed (TCP only),
    pMachineServer->RecvMsgTotalBytes = 0;  /// b/c could contain garbage if old connection was terminated
    
    SOCKADDR_IN sa;
    sa.sin_family      = AF_INET;
    sa.sin_addr.s_addr = INADDR_ANY;
    sa.sin_port        = htons(GAME_PORT);
    iRetVal = bind( pMachineServer->Socket, (sockaddr*)&sa, sizeof(SOCKADDR_IN) );
    assert( 0 == iRetVal );

    //$REVISIT: even though we're in blocking mode, the connect() call will only
    // block if the remote machine isn't listen()'ing for incoming connections.
    // Because after the server calls listen(), all incoming connections are
    // immediately accepted (the SYN is ACK'd).  On the server, accept() doesn't
    // do anything except move the connection from one queue to another.
    //
    // So can't use the return value to tell you whether the remote machine
    // called accept() on your connection.
    //
    // CMP: is our technique of closing the listening socket on the server
    // a good solution?  Remember that we eventually want to support async
    // connection.  (So we might need to use select() ?)
    iRetVal = connect( pMachineServer->Socket,
                       (sockaddr*)&pMachineServer->SoAddr,
                       sizeof(SOCKADDR_IN) );
    //$BUGBUG: <old> I was seeing WSAECONNREFUSED (WSAGetLastError==0x274D) here after I had connected/disconnected a few times, but I haven't tried to track it down yet.  (Maybe there's a max-connections limit on Xbox?)
    //$UPDATE(Apr 2002): Actually, that might have been due to not closing sockets correctly, or the online-vs-offline net stack bug that was fixed in May release.
    if( 0 != iRetVal )
    {
        // connection failed, so join was rejected
        LeaveSession();  //$REVISIT: is this always safe to do here ??
        // display message to user
        g_ShowSimpleMessage.Begin( TEXT_TABLE(TEXT_NETWORK_NOTICE), 
                                   TEXT_TABLE(TEXT_NETWORK_NO_SERVER),
                                   NULL, TEXT_TABLE(TEXT_BUTTON_B_BACK) );
        return FALSE;
    }

    // now that we're connected, put the socket into non-blocking mode
    DWORD dwNonBlocking = 1;
    ioctlsocket( pMachineServer->Socket, FIONBIO, &dwNonBlocking );


    // Initialize other values
    PlayerCount = 0;
    pMachineLocal = NULL;  // clients will set pMachineLocal to non-NULL when they receive player list

    //$ADDITION(cprince): copy values from MultiStartData
    //$REVISIT(cprince): this was necessary b/c matchmaking query sets iLevelNum and GameType to invalid values when wants to match against "any".
    /// Maybe a better solution would be to store query parameters in a separate struct, instead of overwriting them.  (Or maybe not, but could set values when session is joined, instead of here.)
    GameSettings.GameType = SessionList[iSession].GameType;
    gTitleScreenVars.iLevelNum = SessionList[iSession].iLevelNum;

    OnSessionEntered();

    // successfully connected to server
    return TRUE;
}


//-----------------------------------------------------------------------------
// Name: LeaveSession
// Desc: Leaves the session that was previously joined.
//-----------------------------------------------------------------------------
void LeaveSession(void)
{
    // Close our TCP connection to the server
    if( pMachineServer->Socket != INVALID_SOCKET )
    {
        shutdown( pMachineServer->Socket, SD_BOTH );
        closesocket( pMachineServer->Socket );
        pMachineServer->Socket = INVALID_SOCKET;
    }

    OnSessionExited();

    // Unregister the session key, and clear the SessionCurr structure
    XNetUnregisterKey( &(SessionCurr.keyID) );
    ZeroMemory( &SessionCurr, sizeof(SessionCurr) );

    // Note: we aren't sending a "REMOVE_PLAYERS" message to server because
    /// server needs to handle non-graceful player removal anyway.
    //$TODO: implement server handling of non-graceful player removal! :)
    //$REVISIT: might want to send REMOVE_PLAYERS message for quicker notification of dropped players.

    //$HEY: should we set PlayerCount=0 here?  (Depends on whether external code -- eg UI -- reads that value.)

    MachineCount = 0;
    //$HEY: is this necessary?  And should we also clear the MachineList array?
    //$HEY: do we need to clear pMachineLocal here?  (For clients, I don't think so, b/c gets cleared during JoinSession)

    bGameStarted = FALSE;  // make sure the 'started' flag is cleared

    KillNetwork();
}


//-----------------------------------------------------------------------------
// Name: RequestSessionList
// Desc: Requests a list of active sessions that this app can join.
//-----------------------------------------------------------------------------
void RequestSessionList(void)
{
    //$REVISIT: can we get here even if net init failed? If so, should check for valid init, and immed-return if didn't happen.  (That check was added in Acclaim Aug99 code.)
    //$REVISIT: do we need to be careful about maintaining correct SessionPick value when session order/list changes?

    //$TODO(cprince): SysLink session discovery should have some form of timeout, so that we clear SessionList entries that are no longer responding

    if( DisableSessionListUpdate ) { return; }

    // Do work specific to SysLink or XOnline networking
    if( ! gTitleScreenVars.bUseXOnline )
    {
        // *** SYSLINK FIND SESSIONS ***
    
        // Broadcast a session-discovery message.  (Responses are handled elsewhere.)
        const int cbHeader = sizeof(MSG_HEADER_EXT);
        BYTE* pbDataSend = (BYTE*)(SendMsgBuffer + cbHeader);

        *((XNADDR*)pbDataSend) = XnAddrLocal; // Use client XNADDR as a nonce so we know
        pbDataSend += sizeof(XNADDR);         /// whether the broadcast reply is directed at us

        int cbSend = ((BYTE*)pbDataSend) - ((BYTE*)SendMsgBuffer);
        SetSendMsgHeaderExt( MSGEXT_FIND_SESSION );
        sendto( soUDP, SendMsgBuffer, cbSend, 0,
                (sockaddr*)&SoAddrBroadcast, sizeof(SOCKADDR_IN) );

        // Clear the session list, in preparation for receiving new results.
        ClearSessionList();  //$REVISIT: will search results briefly disappear if we do this?  Maybe should skip it, and use a keepalive value instead (removing session list entry when hits zero).

        // Note: SessionList/SessionCount will get updated in
        // handler for MSGEXT_SESSION_FOUND.
    }
    else
    {
        // *** XONLINE FIND SESSIONS ***
    
        if( GameSettings.GameType == GAMETYPE_NETWORK_RACE )
            g_rgAttribs_MatchSearch[0].info.integer.qwValue = MATCH_GAMEMODE_RACE;
        else if( GameSettings.GameType == GAMETYPE_NETWORK_BATTLETAG )
            g_rgAttribs_MatchSearch[0].info.integer.qwValue = MATCH_GAMEMODE_BATTLETAG;
        else {
            assert( GameSettings.GameType == GAMETYPE_NONE );
            //g_rgAttribs_MatchSearch[0].info.integer.qwValue = MATCH_GAMEMODE_ANY;
            g_rgAttribs_MatchSearch[0].dwAttributeID = X_ATTRIBUTE_DATATYPE_NULL;
        }

        if( g_lMatchmakingLevelNum == MATCH_TRACKID_ANY ) {
            //g_rgAttribs_MatchSearch[1].info.integer.qwValue = g_lMatchmakingLevelNum;
            g_rgAttribs_MatchSearch[1].dwAttributeID = X_ATTRIBUTE_DATATYPE_NULL;
        } else {
            g_rgAttribs_MatchSearch[1].info.integer.qwValue = g_lMatchmakingLevelNum;
        }

        //$BUG(XOnline): they expect certain bits to be zero right now.  Probably(hopefully?) will change their minds later, and then we can remove the following lines.
        g_rgAttribs_MatchSearch[0].dwAttributeID &= ~X_ATTRIBUTE_ID_MASK;
        g_rgAttribs_MatchSearch[1].dwAttributeID &= ~X_ATTRIBUTE_ID_MASK;
    
        //$TODO: revisit this. We just need a way to distinguish between
        // "No matches found" and "Still searching"
        g_bXOnlineSessionSearchComplete = FALSE;

        XONLINETASK_HANDLE hTask;
        HRESULT hr;
        DWORD dwResultsLen = XOnlineMatchSearchResultsLen(MAX_SESSION_SEARCH_RESULTS, NUM_XATTRIB_MATCHRESULT, g_rgAttribs_MatchResult);
        hr = XOnlineMatchSearch(
                 MATCH_SEARCH_PROC_INDEX,    // index of server's stored procedure for match-search
                 MAX_SESSION_SEARCH_RESULTS, // max num results to return
                 NUM_XATTRIB_MATCHSEARCH,    // num session attributes (for session-search)
                 g_rgAttribs_MatchSearch,    // array of session attributes (for session-search)
                 dwResultsLen,               // results buffer length (probably from XOnlineMatchSearchResultsLen)
                 NULL,                       // [optional] event to signal when processing reqd
                 &hTask                      // ptr to task handle for session-search
             );
        assert( hr == S_OK );  //$TODO: more robust error handling
    
        OnlineTasks_Add( hTask, TASK_MATCH_SEARCH );

        // reset some values to default state, as appropriate
        g_rgAttribs_MatchSearch[0].dwAttributeID = XATTRIB_GAME_MODE;
        g_rgAttribs_MatchSearch[1].dwAttributeID = XATTRIB_TRACK_ID;


        // Note: SessionList/SessionCount will get updated when
        // the session-find task has completed.
    }

}


//-----------------------------------------------------------------------------
// Name: InsertSessionListEntry
// Desc: Updates an existing entry in the session list with new info, or
//   creates a new entry if session isn't already in the list.
//-----------------------------------------------------------------------------
void InsertSessionListEntry( NET_SESSION* pSession )
{
    if( DisableSessionListUpdate ) { return; }

    int i;
    int iSessionEntry = SessionCount;  // where to store this session entry in list

    // See if this session is already in our list
    for( i=0 ; i < SessionCount ; i++ )
    {
        if( IsSameXNKID(SessionList[i].keyID, pSession->keyID) )
        {
            assert( 0 == memcmp(&SessionList[i].key, &pSession->key, sizeof(XNKEY)) );  //$HEY: silly/unnecessary?
            iSessionEntry = i;
            break;
        }
    }

    // Ignore if we've reached max sessions
    if( iSessionEntry >= MAX_SESSION_SEARCH_RESULTS )  { return; }

    // Add/update this session in our list
    memcpy( &SessionList[iSessionEntry], pSession, sizeof(NET_SESSION) );

    // Update SessionCount if this is a new entry
    if( iSessionEntry == SessionCount )
    {
        SessionCount++;
    }

    // Do some work to keep the list sorted (requires at most one pass)
    for( i = SessionCount-1 ; i > 0 ; i-- )
    {
        int iCompare = wcscmp( SessionList[i-1].wstrHostNickname, SessionList[i].wstrHostNickname );
        if( iCompare == 0 ) // tie-breaker
            iCompare = memcmp( &SessionList[i-1].keyID, &SessionList[i].keyID, sizeof(XNKID) );

        if( iCompare < 0 )  break;  // done; they're in correct order

        // else they're in wrong order, so swap
        NET_SESSION tempSession = SessionList[i];
        SessionList[i] = SessionList[i-1];
        SessionList[i-1] = tempSession;
    }
}


//-----------------------------------------------------------------------------
// Name: DeleteSessionListEntry
// Desc: Given an index into the session list, deletes that entry from the
//   list, after doing any cleanup work that's required.
//-----------------------------------------------------------------------------
void DeleteSessionListEntry( int index )
{
    assert( index >= 0  &&  index < SessionCount );

    // Do required cleanup
    if( SessionList[index].pQosResults != NULL )
    {
        XNetQosRelease( SessionList[index].pQosResults );
        SessionList[index].pQosResults = NULL;
    }
    //$NOTE: because we need to do proper cleanup of session list entries,
    // code outside of network.cpp should using the Insert/Delete functions,
    // and should never modify SessionCount directly.

    // Swap deleted slot with last item in list
    // (Note: we swap instead of just overwriting [index] posn so that [SessionCount] entry gets cleaned up correctly)
    SessionCount--;
    NET_SESSION tempSession = SessionList[index];
    SessionList[index] = SessionList[SessionCount];
    SessionList[SessionCount] = tempSession;
}


//-----------------------------------------------------------------------------
// Name: ClearSessionList
// Desc: Resets the session list.
//-----------------------------------------------------------------------------
void ClearSessionList( void )
{
    if( DisableSessionListUpdate ) { return; }

    while( SessionCount > 0 )
    {
        DeleteSessionListEntry( SessionCount - 1 );
    }
    assert( SessionCount == 0 );
    SessionPick = 0;

    //$NOTE: since ClearSessionList releases the QoS probe allocations/setup, need to make sure it gets called when QoS stuff is no longer used.  (Else wastes resources!)
}



// Server-only helper function to do net-related work that's required when starting a session.
void SetGameStarted(void)
{
    assert( IsServer() );


    // Close the TCP socket that was listening for incoming client connections
    // (after closing any incoming connections that were pending on that socket)
    //
    //$REVISIT: is this a clean order for doing things?  (Seems like there's
    // potential for someone to connect() between the last accept() and the
    // closesocket() call.)  Maybe need to do partial shutdown() on the listening
    // socket before we do all this other stuff?
    SOCKADDR_IN SoAddr;
    int    cbAddr = sizeof(SoAddr);
    SOCKET socket = accept( soListenTCP, (sockaddr*)&SoAddr, &cbAddr );
    while( socket != INVALID_SOCKET )
    {
        // close the socket
        shutdown( socket, SD_BOTH );
        closesocket( socket );

        // any more pending connections?
        cbAddr = sizeof(SoAddr);
        socket = accept( soListenTCP, (sockaddr*)&SoAddr, &cbAddr );
    }

    shutdown( soListenTCP, SD_BOTH );
    closesocket( soListenTCP );
    soListenTCP = INVALID_SOCKET;

    // Also stop responding to QoS probes from clients
    XNetQosListen( &(SessionCurr.keyID), // (ptr to) session XNKID
                   NULL, // (used with _SET_DATA) ptr to game data block
                   0,    // (used with _SET_DATA) num bytes in game data block
                   0,    // (used with _SET_BITSPERSEC) bits/sec value
                   XNET_QOS_LISTEN_RELEASE // subset of the XNET_QOS_LISTEN_ flags
                 );


    // For XOnline games, delete session from matchmaking server, to
    // stop advertising it.
    if( gTitleScreenVars.bUseXOnline )
    {
        XONLINETASK_HANDLE hTask;
        HRESULT hr;
        hr = XOnlineMatchSessionDelete(
                 SessionCurr.keyID, // session XNKID
                 NULL,          // [optional] event to signal when processing reqd
                 &hTask         // ptr to task handle for session-delete
             );
        assert( hr == S_OK );  //$TODO: more robust error handling

        //$HEY: If delete fails, do we need to avoid clearing the keyID below, and try again later?  Sheesh, I hope not...
    
        OnlineTasks_Add( hTask, TASK_MATCH_DELETE );
    }


    // Now that everything has succeeded, set the 'started' flag
    bGameStarted = TRUE;

    // Revoke any pending game invitations
    if( IsLoggedIn(0) ) //$SINGLEPLAYER - Assuming single local player
    {
        RevokeAllInvites();
    }
}

//$REVISIT(cprince): not the most elegant function name, and function mainly here so that we can call it from outside network.cpp (but assumes you only call it once)
//$TODO(cprince): how to we handle new *local* players joining the session, and making sure clients hear about it?  (Maybe can breakout functionality of REQUEST_ADDPLAYERS handler, and make it not specific to clients.)
void CreateLocalServerPlayers(void)
{
    assert( IsServer() );

    // Add all local players to PlayerList
    assert( 0 == PlayerCount );
    for( DWORD i = 0 ; i < dwLocalPlayerCount ; i++ )
    {
        // Fill an entry in PlayerList[]
        NET_PLAYER* pNewPlayer = &( PlayerList[PlayerCount] );

        //$BUGBUG: we're not handling multiple local players here yet!  (Still assuming only 1 local player.)

        pNewPlayer->CarType = GameSettings.CarType;
        memcpy( &(pNewPlayer->Name), RegistrySettings.PlayerName, sizeof(RegistrySettings.PlayerName) );
        if( gTitleScreenVars.bUseXOnline )
        {
            memcpy( &(pNewPlayer->GamerTag), Players[0].XOnlineInfo.pXOnlineUser->szGamertag, XONLINE_GAMERTAG_SIZE );
        }
        else
        {
            memset( pNewPlayer->GamerTag, 0, XONLINE_GAMERTAG_SIZE );
        }
        //$TODO: should make sure (here or elsewhere) that string is guaranteed to be null-terminated!
        // (Protocol would be nice: how about do it wherever else a char buffer gets written to.)
        pNewPlayer->PlayerID = GetUnusedPlayerID();
        pNewPlayer->XnAddr = XnAddrLocal;
        pNewPlayer->SoAddr.sin_family = AF_INET;
        pNewPlayer->SoAddr.sin_port   = htons(GAME_PORT);

        //$SINGLEPLAYER: Assuming single player at Players[0]
        if( Players[0].XOnlineInfo.pXOnlineUser )
        {
            pNewPlayer->xuid = Players[0].XOnlineInfo.pXOnlineUser->xuid;
        }
        else
        {
            pNewPlayer->xuid.qwUserID = pNewPlayer->PlayerID;
            pNewPlayer->xuid.dwUserFlags = 0;
        }
        ZeroMemory( &(pNewPlayer->SoAddr.sin_addr), sizeof(IN_ADDR) );  //$CMP_NOTE: the IN_ADDR we set here should never get used (unless it's overwritten), so this probably isn't necessary?
        pNewPlayer->Ping = 0;  //$CMP_NOTE: probably don't need to do this here...


        // Update player IDs
        assert( (pNewPlayer->PlayerID & 0x0000FFFF) == pNewPlayer->PlayerID );
        LocalPlayerID = (WORD)pNewPlayer->PlayerID;
        ServerID = LocalPlayerID;


        // Update variables that track #players in session
        PlayerCount++;
        if( SessionCurr.nCurrPlayersPublic < SessionCurr.nMaxPlayersPublic ) {
            // put local server players in public slots if possible
            pNewPlayer->bInvitedByFriend = 0; // needed if local player leaves session
            SessionCurr.nCurrPlayersPublic++;
        } else {
            // put remaining local server players in private slots
            assert( SessionCurr.nCurrPlayersPrivate < SessionCurr.nMaxPlayersPrivate ); //$BUG: should explicitly check for this case in UI code for setting up session!
            pNewPlayer->bInvitedByFriend = 1; // needed if local player leaves session
            SessionCurr.nCurrPlayersPrivate++;
        }
        //$BUG: if we ever support removing local players on server, we should
        // shuffle any local private players to public slots (if public slots available)


        // Do other work that's required when we add a player
        OnPlayerAdded( pNewPlayer, FALSE );
    }
}


//-----------------------------------------------------------------------------
// Name: RequestAddPlayers
// Desc: Tells the server that we want to add player(s) to the session.
//-----------------------------------------------------------------------------
void RequestAddPlayers( DWORD dwPlayers )
{
    assert( IsClient() ); 

    const int cbHeader = sizeof(MSG_HEADER_TCP);
    BYTE* pbDataSend = (BYTE*)(SendMsgBuffer + cbHeader);

    *((DWORD*)pbDataSend) = dwPlayers;
    pbDataSend += sizeof(DWORD);

    *((BYTE*)pbDataSend) = g_bInvitedByFriend;
    pbDataSend += sizeof(BYTE);

    for( DWORD i = 0 ; i < dwPlayers ; i++ )
    {
        //$BUGBUG: we're not handling multiple local players here yet!  (Still assuming only 1 local player.)

        *((long*)pbDataSend) = GameSettings.CarType;
        pbDataSend += sizeof(long);

        memcpy( pbDataSend, RegistrySettings.PlayerName, sizeof(RegistrySettings.PlayerName) );
        pbDataSend += sizeof(RegistrySettings.PlayerName);

        if( gTitleScreenVars.bUseXOnline )
        {
            memcpy( pbDataSend, Players[0].XOnlineInfo.pXOnlineUser->szGamertag, XONLINE_GAMERTAG_SIZE );
        }
        else
        {
            memset( pbDataSend, 0, XONLINE_GAMERTAG_SIZE );
        }
        pbDataSend += XONLINE_GAMERTAG_SIZE;

        //$SINGLEPLAYER: Assuming single player at Players[0]
        if( Players[0].XOnlineInfo.pXOnlineUser )
        {
            memcpy( pbDataSend, &( Players[0].XOnlineInfo.pXOnlineUser->xuid ), sizeof( XUID ) );
        }
        else
        {
            // We don't have a XUID, so send 0 - the server 
            // will give us a playerID, and we'll use that
            // for our XUID
            ZeroMemory( pbDataSend, sizeof( XUID ) );
        }
        pbDataSend += sizeof( XUID );
    }

    int cbSend = ((BYTE*)pbDataSend) - ((BYTE*)SendMsgBuffer);
    SetSendMsgHeaderTcp( MSG_OTHER, MSGEXT_REQUEST_ADDPLAYERS, cbSend );
    send( pMachineServer->Socket, SendMsgBuffer, cbSend, 0 );
}


//-----------------------------------------------------------------------------
// Name: GetUnusedPlayerID
// Desc: Returns a unique player ID.
//-----------------------------------------------------------------------------
BYTE GetUnusedPlayerID(void)
{
    //$REVISIT: why can't we just return PlayerCount? (ie, player ID is index
    // in the PlayerList struct).  Maybe we shuffle this array when deleting
    // entries...

    //NOTE: INVALID_PLAYER_ID is 0, so range of valid player IDs is 1..N
    // (instead of 0..N-1).  To handle this, we subtract 1 from existing
    // player IDs when reading them here, and then we add 1 to our final
    // result before returning it.

    assert( IsServer() );

    assert( MAX_RACE_CARS < 32 ); // is the mask var large enough?
    DWORD maskUnused = (1 << MAX_RACE_CARS) - 1;  // set bits 0..N-1
    BYTE i;

    // Determine which IDs are already in use
    for( i=0 ; i < PlayerCount ; i++ )
    {
        assert( PlayerList[i].PlayerID >= 1 
                && PlayerList[i].PlayerID <= MAX_RACE_CARS );
        DWORD maskUsed =  1 << (PlayerList[i].PlayerID - 1);
        assert( maskUnused & maskUsed ); // bit shouldn't be clear yet
        maskUnused ^= maskUsed;  // clear the bit
    }

    // Find an ID that isn't in use
    for( i=0 ; i < MAX_RACE_CARS ; i++ )
    {
        DWORD maskTest = (1 << i);
        if( maskUnused & maskTest )
        {
            return i+1;  // found an unused player ID
        }
    }

    // Oops, didn't find an unused player ID
    assert( 0 && "Couldn't find unused PlayerID!" );
    return INVALID_PLAYER_ID;
}



//-----------------------------------------------------------------------------
// Name: PlayerIndexFromPlayerID
// Desc: Returns the player index into PlayerList for the given PlayerID,
//          or -1 if not found
//-----------------------------------------------------------------------------
LONG PlayerIndexFromPlayerID( DWORD dwPlayerID )
{
    for( LONG p = 0; p < PlayerCount; p++ )
    {
        if( PlayerList[p].PlayerID == dwPlayerID )
            return p;
    }

    return -1;
}



//-----------------------------------------------------------------------------
// Name: PlayerIndexfromXUID
// Desc: Returns the player index into PlayerList for the given xuid
//          or -1 if not found
//-----------------------------------------------------------------------------
LONG PlayerIndexFromXUID( XUID xuid )
{
    for( LONG p = 0; p < PlayerCount; p++ )
    {
        if( IsSameXUID( PlayerList[p].xuid, xuid ) )
            return p;
    }

    return -1;
}

//$HEY: this func was removed by CPrince in orig net code rewrite, but Acclaim updated it to do more in Aug99 drop.
//$TODO: decide whether we can safely remove this function, and if so, remove all calls to it.
bool CreatePlayer(char *name, long mode)
{
//
//    //$HEY: this function does nothing for now.  We'll determine later whether we
//    /// really need to set PlayerName and client/server setting here.
//
//    (other unnecessary DPlay stuff was here)
//

//$REMOVED - we set server's LocalPlayerID/ServerID elsewhere (in EnterSessionName as of Feb. 2002)
//    //$HEY: IS THIS LINE NECESSARY, OR CAN WE SET ServerID IN ANOTHER CODE PATH (OR GET RID OF IT, OR USE SOMETHING ELSE)?
//    if (mode == DP_SERVER_PLAYER)
//        ServerID = LocalPlayerID;
//$END_REMOVAL

//    //$HEY: should we be storing the name somewhere here !?!
//
//// set good data
//
//    SetPlayerData();

    return TRUE; // return OK
}



//$TODO: decide whether we can safely remove this function, and if so, remove all calls to it.

//$NOTE: var LocalPlayerData is only used 1 place outside this func (in CreatePlayer, before calling SetPlayerData, so not initialized at that point!?!)
/// And DP::SetPlayerData just makes sure contents of that struct get propagated to other players in session.
///
/// We will handle that propagation ourselves, so func temporarily commented out for now
/// (but still need to make sure whatever data we do xfer between players gets set using source values here...)
//ACCLAIM AUG99 ADDITION
void SetPlayerData(void)
{
//$HEY: temporarily commented out (not using yet, but might need later)
//    HRESULT r;
//
//// set car
//
//    strncpy(LocalPlayerData.CarName, CarInfo[GameSettings.CarType].Name, CAR_NAMELEN);
//
//// set ready flag
//
//    LocalPlayerData.Ready = LocalPlayerReady;
//
////$REMOVED -- we're not worried about cheating
////// set cheating flag
////
////    LocalPlayerData.Cheating = CarInfo[GameSettings.CarType].Modified;
////$END_REMOVAL
//
//// set
//
//    r = DP->SetPlayerData(LocalPlayerID, &LocalPlayerData, sizeof(LocalPlayerData), DPSET_GUARANTEED);
//    if (r != DP_OK)
//    {
//        ErrorDX(r, "Can't set player data");
//    }
}


//$REVISIT: if not storing player names via DPlay, then where are they being stored?  (I think in PlayerList that gets sent out at start of game...)

//$REMOVED (tentative!!) -- 
//ACCLAIM AUG99 ADDITION
//  (probably don't need this; only affects DPlay; game never calls GetPlayerName)
//void SetPlayerName(char *name)
//{
//    DPNAME dpname;
//    HRESULT r;
//
//    ZeroMemory(&dpname, sizeof(dpname));
//    dpname.dwSize = sizeof(dpname);
//    dpname.lpszShortNameA = name;
//    dpname.lpszLongNameA = NULL;
//
//    r = DP->SetPlayerName(LocalPlayerID, &dpname, 0);
//    if (r != DP_OK)
//    {
//        ErrorDX(r, "Can't set player name");
//    }
//}
//$END_REMOVAL



inline BYTE PackMsgTypeAndID( MSG_TYPE MsgType, DWORD ID )
{
    // NOTE: valid player IDs are in range 1..N and 0 is INVALID_PLAYER_ID
    assert( MAX_RACE_CARS < 25 );  // can we encode a unique player ID for every PlayerList entry?
    assert( ID      >= 0  &&  ID      < 25 );
    assert( MsgType >= 0  &&  MsgType < 10 );
    BYTE bTypeAndID = 0;
    bTypeAndID = (bTypeAndID * 25) + (BYTE)ID;
    bTypeAndID = (bTypeAndID * 10) + MsgType;
    return bTypeAndID;
}

// NOTE: also uses global 'LocalPlayerID'
void SetSendMsgHeader( MSG_TYPE MsgType )
{
    MSG_HEADER* pMsgHeader = (MSG_HEADER*)SendMsgBuffer;
    pMsgHeader->TypeAndID = PackMsgTypeAndID( MsgType, LocalPlayerID );
}

// NOTE: also uses global 'LocalPlayerID'
void SetSendMsgHeaderExt( MSG_TYPE_EXT MsgTypeExt )
{
    MSG_HEADER_EXT* pMsgHeaderExt = (MSG_HEADER_EXT*)SendMsgBuffer;
    pMsgHeaderExt->TypeAndID    = PackMsgTypeAndID( MSG_OTHER, LocalPlayerID );
    pMsgHeaderExt->ExtendedType = MsgTypeExt;
}

// NOTE: also uses global 'LocalPlayerID'
void SetSendMsgHeaderTcp( MSG_TYPE MsgType, MSG_TYPE_EXT MsgTypeExt, int size )
{
    MSG_HEADER_TCP* pMsgHeaderTcp = (MSG_HEADER_TCP*)SendMsgBuffer;
    assert( size == (WORD)size );
    pMsgHeaderTcp->Size         = (WORD)size;
    pMsgHeaderTcp->TypeAndID    = PackMsgTypeAndID( MsgType, LocalPlayerID );
    pMsgHeaderTcp->ExtendedType = MsgTypeExt;
}

// NOTE: also sets global 'FromID'
void GetRecvMsgHeader( MSG_TYPE* pMsgType, MSG_TYPE_EXT* pMsgTypeExt )
{
    BYTE bTypeAndID = RecvMsgBuffer[0];
    *pMsgType = (MSG_TYPE)(bTypeAndID % 10);  bTypeAndID = bTypeAndID / 10;
    FromID    = (bTypeAndID % 25);            bTypeAndID = bTypeAndID / 25;
    assert( 0 == bTypeAndID );

    if( *pMsgType == MSG_OTHER )
        *pMsgTypeExt = (MSG_TYPE_EXT)RecvMsgBuffer[1];
    //else
    //    *pMsgTypeExt = 0;
}




// Separate queue for voice packets, so that multiple packets
// in the main queue don't all have message header (which would waste bandwidth).
#include <pshpack1.h> // one-byte alignment for structs sent over network frequently
struct VOICE_PAYLOAD_DELTA_HEADER
{
    BYTE bMsgNum:4;         // 4-bit Message number
    BYTE bSeqNumDelta:4;    // 4-bit delta from last sequence number
};
#include <poppack.h>

static const DWORD VOICE_FULL_HEADER_SIZE       = sizeof(XVOICE_CODEC_HEADER);
static const DWORD VOICE_DELTA_HEADER_SIZE      = sizeof(VOICE_PAYLOAD_DELTA_HEADER);
static const DWORD VOICE_DATA_PER_PACKET        = 16;
static const DWORD VOICE_BYTES_PER_FULL_PACKET  = VOICE_DATA_PER_PACKET + VOICE_FULL_HEADER_SIZE;
static const DWORD VOICE_BYTES_PER_DELTA_PACKET = VOICE_DATA_PER_PACKET + VOICE_DELTA_HEADER_SIZE;
static const DWORD VOICE_MAX_QUEUE_PACKETS      = 25;   // enough for 1 second

BYTE SendVoiceBuffer[sizeof(MSG_HEADER) + sizeof(BYTE) + VOICE_BYTES_PER_FULL_PACKET + VOICE_BYTES_PER_DELTA_PACKET * ( VOICE_MAX_QUEUE_PACKETS - 1 )];
BYTE NumVoicePackets = 0;

// Helper function for getting a pointer to the appropriate 
// voice payload inside the voice queue
BYTE* GetVoicePayloadPointer( DWORD dwPayloadNumber )
{
    if( dwPayloadNumber == 0 )
        return SendVoiceBuffer + 
               sizeof(MSG_HEADER) + 
               sizeof(BYTE);
    else
        return SendVoiceBuffer + 
               sizeof(MSG_HEADER) + 
               sizeof(BYTE) + 
               VOICE_BYTES_PER_FULL_PACKET + // First, full packet
               ( dwPayloadNumber - 1 ) * VOICE_BYTES_PER_DELTA_PACKET;
}

void QueueVoiceMessage(void* buf, WORD size, DWORD dwPlayerID )
{
    static WORD wLastSequenceNumber;
    const int cbHeader = sizeof(MSG_HEADER);

    assert( size == VOICE_BYTES_PER_FULL_PACKET );          // expected size?
    assert( NumVoicePackets+1 <= VOICE_MAX_QUEUE_PACKETS ); // queue full?

    // init header if first voice packet
    if( 0 == NumVoicePackets )
    {
        ((MSG_HEADER*)SendVoiceBuffer)->TypeAndID = PackMsgTypeAndID( MSG_VOICE_PACKET, dwPlayerID );
        //$REVISIT: could set MSG_VOICE_PACKET once at init time (instead of on each send).  But maybe need to set the other fields (eg, PlayerID) each time.
    }

    XVOICE_CODEC_HEADER* pHeader = (XVOICE_CODEC_HEADER*)buf;
    BYTE* pDest = GetVoicePayloadPointer( NumVoicePackets );
    if( 0 == NumVoicePackets )
    {
        // For the first packet, just copy the whole thing in
        memcpy( pDest,
                buf,
                VOICE_BYTES_PER_FULL_PACKET );
    }
    else
    {
        // For each subsequent packet, delta-encode the sequence
        // number, and then copy in the actual compressed voice data
        VOICE_PAYLOAD_DELTA_HEADER* pDelta = (VOICE_PAYLOAD_DELTA_HEADER*)pDest;

        pDelta->bMsgNum      = pHeader->bMsgNo;
        if( pHeader->wSeqNo - wLastSequenceNumber > 15 )
        {
            DumpMessageVar( "Voice", "Sequence number too large for delta encoding!" );

            // If the change in sequence numbers was too large, we'll set the
            // delta to 0.  When received, they'll end up with duplicate sequence
            // numbers, and the voice queue will throw them out.  The next full 
            // sequence number we send should get everyone back on track.
            pDelta->bSeqNumDelta = 0;
        }
        else
        {
            pDelta->bSeqNumDelta = pHeader->wSeqNo - wLastSequenceNumber;
        }
        memcpy( pDest + VOICE_DELTA_HEADER_SIZE, (BYTE*)buf + VOICE_FULL_HEADER_SIZE, VOICE_DATA_PER_PACKET );
    }

    wLastSequenceNumber = pHeader->wSeqNo;

    // increment packet count
    NumVoicePackets++;
}

VOID OnVoicePacket( DWORD dwPort, DWORD dwSize, VOID* pvData, VOID* pContext )
{
    // $SINGLEPLAYER - Assuming single local player
    assert( dwSize <= 0xFFFF );
    QueueVoiceMessage( pvData, (WORD)dwSize, LocalPlayerID );
}


//$REVISIT: figure out whether we really want to queue outgoing messages this way.
/// (Maybe only used for bundling frame's data, which is better than separate packet for each, but still has header-bloat issues.)

/////////////////////////////////////////////
// queue a messsage in local message queue //
/////////////////////////////////////////////
void QueueMessage(void* buf, WORD size)
{

// skip if queue full

    if (MessageQueueSize + size > PACKET_BUFFER_SIZE)  //$MODIFIED: changed DP_BUFFER_MAX to PACKET_BUFFER_SIZE
    {
        OutputDebugString( "WARNING: Dropped data! Message queue full.\n" );
        return;
    }

// copy message

    memcpy(&MessageQueue[MessageQueueSize], buf, size);
    MessageQueueSize += size;
}


//$NOTE: This is a new function; code came from TransmitRemoteObjectData in object.cpp, which is only place where they sent contents of message queue.
void TransmitMessageQueue(void)
{
//$MODIFIED 
//    SendMessage((MESSAGE_HEADER*)MessageQueue, (short)MessageQueueSize, GroupID);

    // Insert data from voice message queue
    if( NumVoicePackets > 0 ) // optimization: don't need to send if no useful data
    {
        SendVoiceBuffer[sizeof(MSG_HEADER)] = NumVoicePackets;  // store num packets after header
        QueueMessage( SendVoiceBuffer, sizeof(MSG_HEADER) + sizeof(BYTE) + VOICE_BYTES_PER_FULL_PACKET + VOICE_BYTES_PER_DELTA_PACKET * ( NumVoicePackets - 1 ));
        NumVoicePackets = 0;
    }

    // BUGBUG: Needed to do this or 6 boxes would start dropping packets
    if( MessageQueueSize == 0 )
       return;

    // send contents to each remote machine
    for( int i=0 ; i < MachineCount ; i++ )
    {
        if( &MachineList[i] != pMachineLocal )
        {
            sendto( soUDP, MessageQueue, MessageQueueSize, 0,
                    (sockaddr*)&MachineList[i].SoAddr, sizeof(SOCKADDR_IN) );
        }
    }
//$END_MODIFICATIONS
    MessageQueueSize = 0;
}


//$REMOVED -- directly calling 'sendto' instead
//////////////////////
//// send a message //
//////////////////////
//
//void SendMessage(MESSAGE_HEADER *buf, short size, DPID to)
//{
//    TotalDataSent += size;
//
//    HRESULT r;
//
//    r = DP->SendEx(LocalPlayerID, to, DPcaps.dwFlags & DPCAPS_ASYNCSUPPORTED ? DPSEND_ASYNC | DPSEND_NOSENDCOMPLETEMSG : 0, buf, size, 0, DPcaps.dwFlags & DPCAPS_SENDTIMEOUTSUPPORTED ? DP_SEND_TIMEOUT : 0, NULL, NULL);
////  if (r != DP_OK && r != DPERR_PENDING && r != DPERR_INVALIDPLAYER)
////  {
////      ErrorDX(r, "Can't send DirectPlay message");
////  }
//}
//$END_REMOVAL

//$REMOVED -- directly calling 'send' instead
//////////////////////
//// send a message //
//////////////////////
//
////$NOTE: function isn't called from outside of this file
//void SendMessageGuaranteed(MESSAGE_HEADER *buf, short size, DPID to)
//{
//    TotalDataSent += size;
//
////$MODIFIED
////    HRESULT r;
////
////    r = DP->SendEx(LocalPlayerID, to, DPSEND_GUARANTEED, buf, size, 0, 0, NULL, NULL);
//////  if (r != DP_OK && r != DPERR_PENDING && r != DPERR_INVALIDPLAYER)
//////  {
//////      ErrorDX(r, "Can't send DirectPlay guaranteed message");
//////  }
//
//    //$NOTE: SendMessageGuaranteed is only being called with a 'to' value of GroupID (which is always DPID_ALLPLAYERS when groups not used)
//
//    
////$END_MODIFICATIONS
//}
//$END_REMOVAL





//-----------------------------------------------------------------------------
// Name: GetSingleMessageTCP
// Desc: Tries to extract a single message from the given TCP socket.
//   Correctly handles the case where there are partial messages or multiple
//   messages in the input queue.  Returns TRUE if a full message was
//   obtained, or FALSE otherwise.  Message gets stored at the start of the
//   given buffer.
//
// Return value:  positive if message received, zero if no messages, or
//   negative if socket has been terminated.
//-----------------------------------------------------------------------------
int GetSingleMessageTCP( SOCKET socket, char* pRecvBuffer,
                         short* pMsgCurrBytes, short* pMsgTotalBytes )
{
    assert( socket != INVALID_SOCKET );

    // If we don't have all of message header, try to get it
    if( *pMsgCurrBytes < sizeof(MSG_HEADER_TCP) )
    {
        int cbHeaderRemaining = sizeof(MSG_HEADER_TCP) - *pMsgCurrBytes;

        // try to fetch remaining bytes
        int iRetVal = recv( socket, &pRecvBuffer[*pMsgCurrBytes], cbHeaderRemaining, 0 );

        // failed?
        if( iRetVal == 0 ) { return -1; }  // remote side closed socket gracefully, and all data has been received
        if( iRetVal == SOCKET_ERROR )
        {
            iRetVal = WSAGetLastError();
            switch(iRetVal)
            {
            //$REVISIT: can we also get WSAENOTCONN if other side closes gracefully (via FIN)?
            case WSAECONNRESET:  // remote side terminated connection ungracefully
            case WSAETIMEDOUT:   // connection dropped because timed out
                return -1;
            case WSAEWOULDBLOCK:
                return 0;   // no data available
            default:
              #ifdef SHIPPING
                //Don't want this debug spew in shipping versions.
              #else
                char szTemp[256];
                sprintf( szTemp, "recv() failed; WSAGetLastError() == 0x%08X\n", iRetVal );
                OutputDebugString( szTemp );
              #endif
                return 0;
            }
        }

        // update values
        *pMsgCurrBytes += (short)iRetVal;
        if( *pMsgCurrBytes < sizeof(MSG_HEADER_TCP) ) {
            return 0;
        } else {
            *pMsgTotalBytes = ((MSG_HEADER_TCP*)pRecvBuffer)->Size;
        }
    }
    
    assert( *pMsgTotalBytes > 0 );  // if we reach this point, pMsgTotalBytes should be valid
    assert( *pMsgTotalBytes < PACKET_BUFFER_SIZE );  // make sure message fits in buffer

    // If we don't have all of message body, try to get it
    if( *pMsgCurrBytes < *pMsgTotalBytes )
    {
        int cbTotalRemaining = *pMsgTotalBytes - *pMsgCurrBytes;

        // try to fetch remaining bytes
        int iRetVal = recv( socket, &pRecvBuffer[*pMsgCurrBytes], cbTotalRemaining, 0 );

        // failed?
        if( iRetVal == 0 ) { return -1; }  // remote side closed socket gracefully, and all data has been received
        if( iRetVal == SOCKET_ERROR )
        {
            iRetVal = WSAGetLastError();
            switch(iRetVal)
            {
            //$REVISIT: can we also get WSAENOTCONN if other side closes gracefully (via FIN)?
            case WSAECONNRESET:  // remote side terminated connection ungracefully
            case WSAETIMEDOUT:   // connection dropped because timed out
                return -1;
            case WSAEWOULDBLOCK:
                return 0;   // no data available
            default:
              #ifdef SHIPPING
                //Don't want this debug spew in shipping versions.
              #else
                char szTemp[256];
                sprintf( szTemp, "recv() failed; WSAGetLastError() == 0x%08X\n", iRetVal );
                OutputDebugString( szTemp );
              #endif
                return 0;
            }
        }

        // update values
        *pMsgCurrBytes += (short)iRetVal;
    }
    
    // Do we have entire message now?
    if( *pMsgCurrBytes >= *pMsgTotalBytes )
    {
        assert( *pMsgCurrBytes == *pMsgTotalBytes ); // shouldn't be larger

        *pMsgCurrBytes  = 0;
        *pMsgTotalBytes = 0;

        return 1;
    }

    return 0;
}


//-----------------------------------------------------------------------------
// Name: GetRemoteMessages
// Desc: Processes all messages in the incoming message queues (TCP and UDP).
//   Return value tells how many messages we processed.
//-----------------------------------------------------------------------------
int GetRemoteMessages(void)
{
    if( s_NetInitRefCount <= 0 )  return 0;

    int iMessagesProcessed = 0;

    //
    // Process any TCP messages received
    //
    if( IsClient() )
    {
        // check client's connection with server for incoming messages
        while(1)
        {
            // do we have a connection to the server yet?
            if( pMachineServer->Socket == INVALID_SOCKET )  { break; }

            // try to get next message
            int iRetVal = GetSingleMessageTCP( pMachineServer->Socket,
                                               pMachineServer->RecvMsgBuffer,
                                               &pMachineServer->RecvMsgCurrBytes,
                                               &pMachineServer->RecvMsgTotalBytes );
            if( iRetVal == 0 ) { break; }  // no message received; no error
            if( iRetVal < 0 )  // socket to server machine is terminated
            {
                //$REVISIT: eventually might want to support host migration
                /// here, and in that case we shouldn't do a full LeaveSession.
                LeaveSession();
                // display message to user
                g_ShowSimpleMessage.Begin( TEXT_TABLE(TEXT_NETWORK_NOTICE), 
                                           TEXT_TABLE(TEXT_NETWORK_SERVER_LOST_OR_BLOCKED),
                                           NULL, TEXT_TABLE(TEXT_BUTTON_B_BACK) );
                break;
            }

            SoAddrFrom     = pMachineServer->SoAddr; // set for convenience elsewhere
            SocketFrom_TCP = pMachineServer->Socket; // set for convenience elsewhere ($CMP_NOTE: but not actually used by clients !?!)

            // process the message
            RecvMsgBuffer = pMachineServer->RecvMsgBuffer;
            RecvMsgSize   = ((MSG_HEADER_TCP*)RecvMsgBuffer)->Size;
            int cbSkip = offsetof(MSG_HEADER_TCP, TypeAndID);
            RecvMsgBuffer += cbSkip; // point at the MSG_HEADER_EXT part (b/c first
            RecvMsgSize   -= cbSkip; /// header in TCP messages contains extra info)
            ProcessMessage();
            iMessagesProcessed++;
        }
    }
    else
    {
        // check connection with each machine for incoming messages
        for( int i = 1 ; i < MachineCount ; i++ )  // start at [1] b/c [0] is local machine (ie, the server)
        {
            SOCKET soMachine = MachineList[i].Socket;

            // handle all messages from this machine
            SoAddrFrom     = MachineList[i].SoAddr; // set for convenience elsewhere
            SocketFrom_TCP = soMachine;             // set for convenience elsewhere
            while(1)
            {
                // try to get next message
                int iRetVal = GetSingleMessageTCP( soMachine,
                                                   MachineList[i].RecvMsgBuffer,
                                                   &MachineList[i].RecvMsgCurrBytes,
                                                   &MachineList[i].RecvMsgTotalBytes );
                if( iRetVal == 0 ) { break; }  // no message received; no error
                if( iRetVal < 0 )  // socket to client machine is terminated
                {
                    // remove all players on that machine
                    for( int p = 0 ; p < PlayerCount ; p++ )
                    {
                        if( IsSameMachine(PlayerList[p].XnAddr, MachineList[i].XnAddr) )
                        {
                            OnPlayerRemoved( &PlayerList[p], TRUE );

                            if( PlayerList[p].bInvitedByFriend )
                                SessionCurr.nCurrPlayersPrivate--;
                            else
                                SessionCurr.nCurrPlayersPublic--;
                            PlayerCount--;
                            PlayerList[p] = PlayerList[PlayerCount];
                            p--;  // avoid accidentally skipping a player
                        }
                    }

                    // remove that machine
                    assert( MachineList[i].Socket != INVALID_SOCKET );
                    shutdown( MachineList[i].Socket, SD_BOTH );
                    closesocket( MachineList[i].Socket );
                    MachineList[i].Socket = INVALID_SOCKET;
                    // swap deleted slot with last item in list
                    // (Note: we swap instead of just overwriting [i] posn so that [MachineCount] entry gets cleaned up correctly)
                    MachineCount--;
                    NET_MACHINE tempMachine = MachineList[i];
                    MachineList[i] = MachineList[MachineCount];
                    MachineList[MachineCount] = tempMachine;
                    //$REVISIT: Do we need to do anything else to clean up
                    // the old slot of the machine that we swap with the
                    // machine that just got removed?
                    i--;  // avoid accidentally skipping a machine

                    //$TODO: there is more work to be done here!  Network code uses PlayerList, but game has different structs!!

                    // notify all other clients of the changes
                    SendPlayerList();  //$REVISIT: this assumes that TCP connection will remain active during entire session.

                    //$REVISIT: will reordering MachineList/PlayerList on server cause any problems?  (I don't think we maintain any pointers into these arrays...)
                    break;
                }

                // process the message
                RecvMsgBuffer = MachineList[i].RecvMsgBuffer;
                RecvMsgSize   = ((MSG_HEADER_TCP*)RecvMsgBuffer)->Size;
                int cbSkip = offsetof(MSG_HEADER_TCP, TypeAndID);
                RecvMsgBuffer += cbSkip; // point at the MSG_HEADER_EXT part (b/c first
                RecvMsgSize   -= cbSkip; /// header in TCP messages contains extra info)
                ProcessMessage();
                iMessagesProcessed++;
            }
        }
    }


    //
    // Process any UDP messages received
    //
    while(1)
    {
        // get next message
        int iAddrLength = sizeof(SoAddrFrom);
        int iRetVal = recvfrom( soUDP, RecvMsgBufferUDP, PACKET_BUFFER_SIZE, 0,
                                (sockaddr*)&SoAddrFrom, &iAddrLength );

        // are we done?
        if( 0 == iRetVal  ||  SOCKET_ERROR == iRetVal )  { break; }
        //$BUGBUG: not sure about this error checking; check for robustness....
        /// (Note: not sure we need the check "0 == iRetVal", b/c can send a message without a payload (header+data in our case), which is what this checks for)

        // process the message
        RecvMsgBuffer = RecvMsgBufferUDP;
        RecvMsgSize   = iRetVal;
        ProcessMessage();
        iMessagesProcessed++;
    }


    // return
    return iMessagesProcessed;
}


//-----------------------------------------------------------------------------
// Name: ProcessMessage
// Desc: Looks at the message header, and processes the message appropriately.
//-----------------------------------------------------------------------------
void ProcessMessage(void)
{
    // Process all messages bundled inside this packet.
    while( RecvMsgSize > 0 )
    {
        int cbMessage;
        MSG_TYPE     MsgType;
        MSG_TYPE_EXT MsgTypeExt;
        GetRecvMsgHeader( &MsgType, &MsgTypeExt );

        switch(MsgType)
        {
        // BASE MESSAGE TYPES
        case MSG_CAR_DATA:            cbMessage = ProcessCarData();  break;
        case MSG_VOICE_PACKET:        cbMessage = ProcessVoiceMessage(); break;
        case MSG_OBJECT_DATA:         cbMessage = ProcessObjectData(); break;
        case MSG_WEAPON_DATA:         cbMessage = ProcessWeaponNew(); break;
        case MSG_TARGET_STATUS_DATA:  cbMessage = ProcessTargetStatus(); break;
        case MSG_PLAYER_SYNC_REQUEST: cbMessage = ProcessPlayerSync1(); break;
        case MSG_PLAYER_SYNC_REPLY1:  cbMessage = ProcessPlayerSync2(); break;
        case MSG_PLAYER_SYNC_REPLY2:  cbMessage = ProcessPlayerSync3(); break;
        case MSG_POSITION:            cbMessage = ProcessPosition(); break; // player position
        // EXTENDED MESSAGE TYPES           
        case MSG_OTHER:
            switch(MsgTypeExt)
            {
            case MSGEXT_CAR_NEWCAR:             cbMessage = ProcessCarNewCar(); break;
            case MSGEXT_CAR_NEWCAR_ALL:         cbMessage = ProcessCarNewCarAll(); break;
            case MSGEXT_GAME_LOADED:            cbMessage = ProcessGameLoaded(); break;
            case MSGEXT_GAME_STARTED:           cbMessage = ProcessGameStarted(); break;
            case MSGEXT_COUNTDOWN_START:        cbMessage = ProcessCountdownStart(); break;
            case MSGEXT_RACE_FINISH_TIME:       cbMessage = ProcessRaceFinishTime(); break;
            case MSGEXT_SYNC_REQUEST:           cbMessage = ProcessSyncRequest(); break;
            case MSGEXT_SYNC_REPLY:             cbMessage = ProcessSyncReply(); break;
            case MSGEXT_RESTART:                cbMessage = ProcessMultiplayerRestart(); break; // game restarting
            // Weapon-related messages          
            case MSGEXT_TRANSFER_BOMB:          cbMessage = ProcessTransferBomb(); break;
            case MSGEXT_BOMBTAG_CLOCK:          cbMessage = ProcessBombTagClock(); break; // bomb tag timer
            case MSGEXT_TRANSFER_FOX:           cbMessage = ProcessTransferFox(); break;
            case MSGEXT_ELECTROPULSE_THE_WORLD: cbMessage = ProcessElectroPulseTheWorld(); break;
            case MSGEXT_GOT_GLOBAL:             cbMessage = ProcessGotGlobal(); break;
            case MSGEXT_HONKA:                  cbMessage = ProcessHonka(); break;
            // Session-setup messages           
            case MSGEXT_FIND_SESSION:           cbMessage = ProcessFindSession(); break;
            case MSGEXT_SESSION_FOUND:          cbMessage = ProcessSessionFound(); break;
            case MSGEXT_REQUEST_ADDPLAYERS:     cbMessage = ProcessRequestAddPlayers(); break;
            case MSGEXT_ADDPLAYERS_ACCEPTED:    cbMessage = ProcessAddPlayersAccepted(); break;
            case MSGEXT_ADDPLAYERS_REJECTED:    cbMessage = ProcessAddPlayersRejected(); break;
            case MSGEXT_PLAYER_LIST:            cbMessage = ProcessPlayerList(); break; // received list of players for some session
            case MSGEXT_CHATTER_LIST:           cbMessage = ProcessChatterList(); break;
            case MSGEXT_VOICE_INFO:             cbMessage = ProcessVoiceInfoMessage(); break;
            default:
              assert( 0 && "Unhandled MSGEXT type!" ); break;
            } //END: switch(MsgTypeExt)
        break;
        default:
          assert( 0 && "Unhandled MSG type!" ); break;
        } //END: switch(MsgType)


        // Update the receive message buffer
        RecvMsgBuffer += cbMessage;
        RecvMsgSize   -= cbMessage;
        assert( RecvMsgSize >= 0 );
        assert( cbMessage > 0 );

    } //END: while(RecvMsgSize > 0)
}


//$REMOVED_NOTUSED (server maintains player list, and keeps all clients updated)
////////////////////
//// list players //
////////////////////
//
//void ListPlayers(GUID *guid)
//{
//    HRESULT r;
//
//    PlayerCount = 0;
//    if (!guid) r = DP->EnumPlayers(NULL, EnumPlayersCallback, NULL, DPENUMPLAYERS_ALL);
//    else r = DP->EnumPlayers(guid, EnumPlayersCallback, NULL, DPENUMPLAYERS_SESSION);
//    if (r != DP_OK && Version == VERSION_DEV)
//    {
//        ErrorDX(r, "Can't enumerate player list");
//    }
//}
//$END_REMOVAL

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
    PlayerList[PlayerCount].MsgPlayerID = dpId;
    strncpy(PlayerList[PlayerCount].Name, lpName->lpszShortNameA, MAX_PLAYER_NAME);

// get player data

    size = sizeof(DP_PLAYER_DATA);
    r = DP->GetPlayerData(PlayerList[PlayerCount].MsgPlayerID, &PlayerList[PlayerCount].Data, &size, DPGET_REMOTE);
    if (r != DP_OK)
    {
        ErrorDX(r, "Can't get player data");
    }

// return OK

    PlayerCount++;
    return TRUE;
}
$END_REMOVAL */



//-----------------------------------------------------------------------------
// Name: ProcessFindSession
// Desc: Handles receipt of the MSGEXT_FIND_SESSION message.  (Means someone
//       is looking for SysLink sessions, so respond if we're hosting one.)
//-----------------------------------------------------------------------------
int ProcessFindSession(void)
{
    const int cbHeader = sizeof(MSG_HEADER_EXT); // same size for send and recv header
    const int cbMessageRecv = cbHeader + sizeof(XNADDR);
    BYTE* pbDataRecv = (BYTE*)(RecvMsgBuffer + cbHeader);
    BYTE* pbDataSend = (BYTE*)(SendMsgBuffer + cbHeader);

    //
    // Make sure we're hosting a SysLink game, and we have a valid session
    // to reply with, and it's not too late to add new players
    //
    if(    IsServer()
        && !gTitleScreenVars.bUseXOnline
        && !IsZeroXNKID(SessionCurr.keyID)
        && !bGameStarted )
        //$REVISIT: do we need to check that they're requesting a Re-Volt game (and not some other game that happens to use same port)?  Or does XNet automatically take care of that?
    {
        //
        // Send back a message with info about this session
        //
        SetSendMsgHeaderExt( MSGEXT_SESSION_FOUND );

        memcpy( pbDataSend, pbDataRecv, sizeof(XNADDR) ); // include XNADDR of sender, so recipients know who message is intended for
        pbDataSend += sizeof(XNADDR);

        memcpy( pbDataSend, &SessionCurr, sizeof(SessionCurr) );
        pbDataSend += sizeof(SessionCurr);

        // NOTE: we cannot reply directly to this 'from' address, because
        // on Xbox it will be invalid (b/c it was sent via broadcast, and
        // not directly).  This has to do with how the Xbox handles
        // security and authentication between machines.

        //$BUGBUG(cprince): we need to give people a better explanation of
        /// this process.  It differs from what happens in "normal"
        /// Winsock. Even I had trouble understanding why the 'from'
        /// address would be invalid here.  The Xbox documentation doesn't
        /// describe what's happening either.
        ///
        /// So we need to add info about the whole process of IN_ADDR
        /// translation, how Xbox sets up a secure connection between
        /// machines (which happens automatically, under the covers),
        /// *when* an IN_ADDR will be valid (and how to get a valid one),
        /// etc.  We should add that info here and/or in the Xbox docs.
        /// (Probably just a brief summary here, and refer to details in
        /// the docs.)

        int cbSend = ((BYTE*)pbDataSend) - ((BYTE*)SendMsgBuffer);
        sendto( soUDP, SendMsgBuffer, cbSend, 0, (sockaddr*)&SoAddrBroadcast, sizeof(SOCKADDR_IN) );
    }


    // Return num bytes in message received
    return cbMessageRecv;
}


//-----------------------------------------------------------------------------
// Name: ProcessSessionFound
// Desc: Handles receipt of the MSGEXT_SESSION_FOUND message.  (Means a server
//       is telling someone about an existing SysLink session.)
//-----------------------------------------------------------------------------
int ProcessSessionFound(void)
{
    // Note: this is only for SysLink networking; XOnline uses matchmaking server.

    const int cbHeader = sizeof(MSG_HEADER_EXT);
    const int cbMessageRecv = cbHeader + sizeof(XNADDR) + sizeof(SessionCurr);
    BYTE* pbDataRecv = (BYTE*)(RecvMsgBuffer + cbHeader);

    // Ignore unless this message is intended for us
    if( IsSameMachine(XnAddrLocal, *(XNADDR*)pbDataRecv) )
    {
        pbDataRecv += sizeof(XNADDR);

        InsertSessionListEntry( (NET_SESSION*)pbDataRecv );
    }

    // Return num bytes in message processed
    return cbMessageRecv;
}


//-----------------------------------------------------------------------------
// Name: ProcessRequestAddPlayers
// Desc: Handles receipt of the MSGEXT_REQUEST_ADDPLAYERS message.  (Means
//       some machine is wants to add players to our session.)
//-----------------------------------------------------------------------------
int ProcessRequestAddPlayers(void)
{
    assert( IsServer() );

    const int cbHeaderRecv = sizeof(MSG_HEADER_EXT);
    const int cbHeaderSend = sizeof(MSG_HEADER_TCP);
    BYTE* pbDataRecv = (BYTE*)(RecvMsgBuffer + cbHeaderRecv);
    BYTE* pbDataSend = (BYTE*)(SendMsgBuffer + cbHeaderSend);

    int i;
    DWORD dwNewPlayers = *((DWORD*)pbDataRecv);
    pbDataRecv += sizeof(DWORD);
    BYTE bInvitedByFriend = *((BYTE*)pbDataRecv);
    pbDataRecv += sizeof(BYTE);

    const int cbMessageRecv = cbHeaderRecv + sizeof(DWORD) + sizeof(BYTE) +
                              dwNewPlayers*( sizeof(long) + sizeof(RegistrySettings.PlayerName) + XONLINE_GAMERTAG_SIZE + sizeof(XUID) );

    // Deny request if it's too late to add new players, or if there are
    // not enough empty slots to add requested players.
    //
    // Note that non-invitation players can only go into public slots, but
    // for invited players, we try private slots first and overflow into
    // public slots if necessary.
    assert( (SessionCurr.nMaxPlayersPublic+SessionCurr.nMaxPlayersPrivate) <= MAX_RACE_CARS );  // is PlayerList large enough?
    bool bRejectJoin = bGameStarted
                         || ( !bInvitedByFriend && (SessionCurr.nCurrPlayersPublic+dwNewPlayers) > SessionCurr.nMaxPlayersPublic )
                         || (  bInvitedByFriend && (PlayerCount+dwNewPlayers) > (SessionCurr.nMaxPlayersPublic+SessionCurr.nMaxPlayersPrivate) )
                       ;
    if( bRejectJoin )
    {
        int cbSend = ((BYTE*)pbDataSend) - ((BYTE*)SendMsgBuffer);
        SetSendMsgHeaderTcp( MSG_OTHER, MSGEXT_ADDPLAYERS_REJECTED, cbSend );
        send( SocketFrom_TCP, SendMsgBuffer, cbSend, 0 );
    }
    // Else store player info, and send appropriate replies
    else
    {
        // store player info
        for( i = 0 ; i < (int)dwNewPlayers ; i++ )
        {
            NET_PLAYER* pNewPlayer = &( PlayerList[PlayerCount] );

            pNewPlayer->CarType = *((long*)pbDataRecv);
            pbDataRecv += sizeof(long);

            memcpy( &(pNewPlayer->Name), pbDataRecv, sizeof(RegistrySettings.PlayerName) );
            pbDataRecv += MAX_PLAYER_NAME;

            // If this is not an online session, the client should have already
            // NULLed out the gamertag when they sent the add player request
            memcpy( &(pNewPlayer->GamerTag), pbDataRecv, XONLINE_GAMERTAG_SIZE );
            pbDataRecv += XONLINE_GAMERTAG_SIZE;

            memcpy( &( pNewPlayer->xuid ), pbDataRecv, sizeof( XUID ) );
            pbDataRecv += sizeof( XUID );

            //$TODO: should make sure (here or elsewhere) that string is guaranteed to be null-terminated!
            // (Protocol would be nice: how about do it wherever else a char buffer gets written to.)

            pNewPlayer->PlayerID = GetUnusedPlayerID();
            if( IsZeroXUID( pNewPlayer->xuid ) )
            {
                // This should only happen in a system link game - in an
                // online game, the player MUST have a valid XUID to send
                assert( !gTitleScreenVars.bUseXOnline );
                pNewPlayer->xuid.qwUserID = pNewPlayer->PlayerID;
                pNewPlayer->xuid.dwUserFlags = 0;
            }
            pNewPlayer->SoAddr = SoAddrFrom;
            XNetInAddrToXnAddr( pNewPlayer->SoAddr.sin_addr,
                                &(pNewPlayer->XnAddr),
                                &(SessionCurr.keyID) );
            pNewPlayer->Ping = 0;  //$CMP_NOTE: probably don't need to do this here...

            PlayerCount++;
            if( bInvitedByFriend && 
               SessionCurr.nCurrPlayersPrivate < SessionCurr.nMaxPlayersPrivate ) {
                // put invited players in private slots if available
                pNewPlayer->bInvitedByFriend = 1; // needed if player leaves session
                SessionCurr.nCurrPlayersPrivate++;
            } else {
                // put remaining players in public slots
                assert( SessionCurr.nCurrPlayersPublic < SessionCurr.nMaxPlayersPublic ); //$BUG: should explicitly check for this case in UI code for setting up session!
                pNewPlayer->bInvitedByFriend = 0; // needed if player leaves session
                SessionCurr.nCurrPlayersPublic++;
            }
            //$REVISIT: if invited players leaves session, would ideally shuffle
            // invited players that got put in public slots back into private
            // slot, right?  But no way to do that now, because the bInvitedByFriend
            // flag tells current state, and we don't have separate bit for
            // original *requested* state.

            // Note: we update SessionCurr.nCurrPlayers** below.
            OnPlayerAdded( pNewPlayer, TRUE );
        }

        assert( PlayerCount == (SessionCurr.nCurrPlayersPublic+SessionCurr.nCurrPlayersPrivate) );

        // send a "join accepted" reply
        // (contains assigned PlayerID for each accepted player)
        *((DWORD*)pbDataSend) = dwNewPlayers;
        pbDataSend += sizeof(DWORD);

        for( i = dwNewPlayers ; i > 0 ; i-- )
        {
            assert( PlayerList[PlayerCount-i].PlayerID == (PlayerList[PlayerCount-i].PlayerID & 0x0000FFFF) );
            *((WORD*)pbDataSend) = (WORD)PlayerList[PlayerCount-i].PlayerID;
            pbDataSend += sizeof(WORD);
        }

        int cbSend = ((BYTE*)pbDataSend) - ((BYTE*)SendMsgBuffer);
        SetSendMsgHeaderTcp( MSG_OTHER, MSGEXT_ADDPLAYERS_ACCEPTED, cbSend );
        send( SocketFrom_TCP, SendMsgBuffer, cbSend, 0 );


        // send the updated list of players and machines to every connected client machine
        SendPlayerList();

        // send the current list of chatters
        SendChatterList();
    }

    // Return num bytes in message received
    return cbMessageRecv;
}


//-----------------------------------------------------------------------------
// Name: ProcessAddPlayersAccepted
// Desc: Handles receipt of the MSGEXT_ADDPLAYERS_ACCEPTED message.  (Means
//       we're being told our requested #players CAN join the session.)
//-----------------------------------------------------------------------------
int ProcessAddPlayersAccepted(void)
{
    assert( IsClient() );

    const int cbHeader = sizeof(MSG_HEADER_EXT);
    BYTE* pbDataRecv = (BYTE*)(RecvMsgBuffer + cbHeader);

    DWORD dwNewPlayers = *((DWORD*)pbDataRecv);
    pbDataRecv += sizeof(DWORD);

    // Extract our assigned PlayerID(s) from the message
    for( int i = dwNewPlayers ; i > 0 ; i-- )
    {
        //$BUGBUG: we're not handling multiple local players here yet!  (Still assuming only 1 local player.)
        //$BUGBUG: might need more work here to handle the case where some requests are accepted and others are rejected (so that PlayerIDs get assigned to correct requesters)

        LocalPlayerID = *((WORD*)pbDataRecv);
        pbDataRecv += sizeof(WORD);
    }

    // Return num bytes in message received
    return ((BYTE*)pbDataRecv) - ((BYTE*)RecvMsgBuffer);
}


//-----------------------------------------------------------------------------
// Name: ProcessAddPlayersRejected
// Desc: Handles receipt of the MSGEXT_ADDPLAYERS_REJECTED message.  (Means
//       we're being told our requested #players CANNOT join the session.)
//-----------------------------------------------------------------------------
int ProcessAddPlayersRejected(void)
{
    assert( IsClient() );

    const int cbHeader = sizeof(MSG_HEADER_EXT);
    const int cbMessageRecv = cbHeader + 0;

    // Leave the session
    LeaveSession();

    // Display message to user
    g_ShowSimpleMessage.Begin( TEXT_TABLE(TEXT_NETWORK_NOTICE),
                               TEXT_TABLE(TEXT_NETWORK_CANNOT_JOIN),
                               NULL, TEXT_TABLE(TEXT_BUTTON_B_BACK) );

    // Return num bytes in message received
    return cbMessageRecv;
}



//-----------------------------------------------------------------------------
// Name: SendPlayerList
// Desc: Sends a list of the current players and machines to all clients.
//-----------------------------------------------------------------------------
void SendPlayerList(void)
{
    assert( IsServer() );

    const int cbHeader = sizeof(MSG_HEADER_TCP);
    BYTE* pbDataSend = (BYTE*)(SendMsgBuffer + cbHeader);
    int i;


    *((DWORD*)pbDataSend) = PlayerCount;
    pbDataSend += sizeof(DWORD);

    for( i = 0 ; i < PlayerCount ; i++ )
    {
        *((NET_PLAYER*)pbDataSend) = PlayerList[i];
        pbDataSend += sizeof(NET_PLAYER);
        // Note: recipients cannot use the SoAddr directly;
        /// they must get it from the XnAddr in this message.
    }


    *((DWORD*)pbDataSend) = MachineCount;
    pbDataSend += sizeof(DWORD);

    for( i = 1 ; i < MachineCount ; i++ )  // skip [0] (ie, the server) b/c clients already have info for that machine
    {
        // only send XnAddr; other fields aren't relevant or useful
        *((XNADDR*)pbDataSend) = MachineList[i].XnAddr;
        pbDataSend += sizeof(XNADDR);
    }


    int cbSend = ((BYTE*)pbDataSend) - ((BYTE*)SendMsgBuffer);
    SetSendMsgHeaderTcp( MSG_OTHER, MSGEXT_PLAYER_LIST, cbSend );
    for( i = 1 ; i < MachineCount ; i++ )  // start at [1] b/c [0] is local machine (ie, the server)
    {
        send( MachineList[i].Socket, SendMsgBuffer, cbSend, 0 );
    }


    // We're probably calling SendPlayerList because a player has been added or removed.
    // So if this is an XOnline game that is still being advertised, update the #players info on the matchmaking server.
    if( gTitleScreenVars.bUseXOnline && !bGameStarted )
    {
        //$REVISIT: or might want to merge this code into Update/CreateLocalServerPlayers ?

        //$NOTE: if we were modifying the session attributes here, we would also
        /// need to set some 'fChanged' flags before calling this function, and
        /// then clear them afterward.

        DWORD dwPublicFilled  = SessionCurr.nCurrPlayersPublic;
        DWORD dwPrivateFilled = SessionCurr.nCurrPlayersPrivate;
        DWORD dwPublicOpen    = SessionCurr.nMaxPlayersPublic - SessionCurr.nCurrPlayersPublic;
        DWORD dwPrivateOpen   = SessionCurr.nMaxPlayersPrivate - SessionCurr.nCurrPlayersPrivate;
    
        XONLINETASK_HANDLE hTask;
        HRESULT hr;
        hr = XOnlineMatchSessionUpdate(
                 SessionCurr.keyID, // session XNKID
                 dwPublicFilled,  // public slots filled
                 dwPublicOpen,    // public slots open
                 dwPrivateFilled, // private slots filled
                 dwPrivateOpen,   // private slots open
                 NUM_XATTRIB_MATCHCREATE, // num session attributes
                 g_rgAttribs_MatchCreate, // array of session attributes
                 NULL, // [optional] event to signal when processing reqd
                 &hTask // ptr to task handle for session-update
             );
        assert( hr == S_OK );  //$TODO: more robust error handling
    
        OnlineTasks_Add( hTask, TASK_MATCH_UPDATE );
    }
}


//-----------------------------------------------------------------------------
// Name: ProcessPlayerList
// Desc: Handles a MSGEXT_PLAYER_LIST message received from server.
//-----------------------------------------------------------------------------
int ProcessPlayerList(void)
{
    assert( IsClient() );
    assert( SAME_INADDR(SoAddrFrom,pMachineServer->SoAddr) );

    const int cbHeader = sizeof(MSG_HEADER_EXT);
    BYTE* pbDataRecv = (BYTE*)(RecvMsgBuffer + cbHeader);
    int i;

    //$CMP_NOTE: should we ignore this message if join-accept not received yet ?? (Probably not required.)


    // Get the list of players
    LONG lOldPlayerCount = PlayerCount;
    PlayerCount = *((DWORD*)pbDataRecv);
    pbDataRecv += sizeof(DWORD);

    NET_PLAYER* OldPlayerList = new NET_PLAYER[lOldPlayerCount];
    memcpy( OldPlayerList, PlayerList, lOldPlayerCount * sizeof( NET_PLAYER ) );

    LONG lNewPlayerCount = PlayerCount;
    NET_PLAYER* NewPlayerList = (NET_PLAYER*)pbDataRecv;
    // First, walk the old list looking for removals
    for( LONG lOld = 0; lOld < lOldPlayerCount; lOld++ )
    {
        BOOL bWasRemoved = TRUE;
        for( LONG lNew = 0; lNew < PlayerCount; lNew++ )
        {
            if( NewPlayerList[ lNew ].PlayerID == OldPlayerList[ lOld ].PlayerID )
                bWasRemoved = FALSE;
        }
        if( bWasRemoved )
        {
            OnPlayerRemoved( &OldPlayerList[ lOld ], TRUE );
        }
    }

    for( i = 0 ; i < PlayerCount ; i++ )
    {
        PlayerList[i] = *((NET_PLAYER*)pbDataRecv);
        pbDataRecv += sizeof(NET_PLAYER);

        // convert players' XNADDR to IN_ADDR
        // (other fields of the SOCKADDR_IN should already be valid)
        XNetXnAddrToInAddr( &(PlayerList[i].XnAddr), &(SessionCurr.keyID),
                            &(PlayerList[i].SoAddr.sin_addr) );
        PlayerList[i].SoAddr.sin_family = AF_INET;
        PlayerList[i].SoAddr.sin_port = htons(GAME_PORT);
        // ZeroMemory( PlayerList[i].SoAddr.sin_zero, sizeof( PlayerList[i].SoAddr.sin_zero );
    }

    // Now, walk the new list looking for additions
    NewPlayerList = PlayerList;
    for( LONG lNew = 0; lNew < PlayerCount; lNew++ )
    {
        BOOL bWasAdded = TRUE;
        for( LONG lOld = 0; lOld < lOldPlayerCount; lOld++ )
        {
            if( NewPlayerList[ lNew ].PlayerID == OldPlayerList[ lOld ].PlayerID )
                bWasAdded = FALSE;
        }
        if( bWasAdded )
        {
            OnPlayerAdded( &NewPlayerList[ lNew ], lOldPlayerCount != 0 );
        }
    }

    delete[] OldPlayerList;

    // Get the list of machines
    MachineCount = *((DWORD*)pbDataRecv);
    pbDataRecv += sizeof(DWORD);

    pMachineLocal = NULL;
    for( i = 1 ; i < MachineCount ; i++ )  // start at [1] b/c already have info for server
    {
        // fill the MachineList entry
        MachineList[i].XnAddr = *((XNADDR*)pbDataRecv);
        pbDataRecv += sizeof(XNADDR);

        XNetXnAddrToInAddr( &(MachineList[i].XnAddr), &(SessionCurr.keyID),
                            &(MachineList[i].SoAddr.sin_addr) );
        MachineList[i].SoAddr.sin_family = AF_INET;
        MachineList[i].SoAddr.sin_port = htons(GAME_PORT);
        // ZeroMemory( MachineList[i].SoAddr.sin_zero, sizeof( MachineList[i].SoAddr.sin_zero );

        //$REVISIT - do we need to set the following? (maybe never used)
        MachineList[i].Socket = INVALID_SOCKET;
        MachineList[i].RecvMsgCurrBytes = 0;
        MachineList[i].RecvMsgTotalBytes = 0;

        // find ourself in the list
        if( IsSameMachine(MachineList[i].XnAddr, XnAddrLocal) )
        {
            assert( NULL == pMachineLocal );
            pMachineLocal = &MachineList[i];
        }
    }
    assert( NULL != pMachineLocal );

    // Return num bytes in message received
    return ((BYTE*)pbDataRecv) - ((BYTE*)RecvMsgBuffer);
}



//-----------------------------------------------------------------------------
// Name: SendChatterList
// Desc: ####################################
//-----------------------------------------------------------------------------
void SendChatterList(void)
{
    assert( IsServer() );

    const int cbHeader = sizeof(MSG_HEADER_TCP);
    DWORD* pdwChatterCount  = (DWORD*)(SendMsgBuffer + cbHeader);
    XUID*  pxuidChatterList = (XUID *)( pdwChatterCount + 1 );

    *pdwChatterCount = 0;
    for( LONG p = 0; p < PlayerCount; p++ )
    {
        // For each player that we know has voice
        if( g_VoiceManager.DoesPlayerHaveVoice( PlayerList[p].xuid ) )
        {
            // Add the player ID to the list and increment counter
            *pdwChatterCount += 1;
            memcpy( pxuidChatterList, &PlayerList[p].xuid, sizeof( XUID ) );
            pxuidChatterList++;
        }
    }

    int cbSend = ( (BYTE *)pxuidChatterList ) - ((BYTE*)SendMsgBuffer);
    SetSendMsgHeaderTcp( MSG_OTHER, MSGEXT_CHATTER_LIST, cbSend );
    send( SocketFrom_TCP, SendMsgBuffer, cbSend, 0 );

    DumpMessageVar( "Voice", "Sending chatter list (%d) %d chatters", LocalPlayerID, *pdwChatterCount );
}



//-----------------------------------------------------------------------------
// Name: OnCommunicatorEvent
// Desc: Callback from voice code for certain communicator events
//-----------------------------------------------------------------------------
VOID OnCommunicatorEvent( DWORD dwPort, VOICE_COMMUNICATOR_EVENT event, VOID* pContext )
{
    switch( event )
    {
    case VOICE_COMMUNICATOR_INSERTED:
        assert( dwPort == g_dwSignedInController );
        if( g_VoiceManager.IsInChatSession() )
        {
            if( IsServer() )
                g_VoiceManager.AddChatter( PlayerList[ PlayerIndexFromPlayerID( LocalPlayerID ) ].xuid );
            SendVoiceInfoMessage( VOICEINFO_ADDCHATTER, 0 );

            // Whenever we send our addchatter, we need to send out 
            // remote mute messages for whoever we have muted, because they threw
            // out my records when I removed my communicator.
            if( IsLoggedIn( 0 ) )
            {

                // Make sure anyone that's in our mute list gets muted
                for( LONG i = 0; i < PlayerCount; i++ )
                {
                    if( g_VoiceManager.DoesPlayerHaveVoice( PlayerList[i].xuid ) &&
                        g_FriendsManager.IsPlayerInMuteList( 0, PlayerList[i].xuid ) )
                    {
                        g_VoiceManager.MutePlayer( PlayerList[i].xuid, g_dwSignedInController );
                        SendVoiceInfoMessage( VOICEINFO_ADDREMOTEMUTE, PlayerList[ i ].PlayerID );
                    }
                }
            }
        }

        //$SINGPLAYER: Assuming single player's registry settings here
        // Set the voice mask as soon as the communicator gets inserted.
        g_VoiceManager.SetVoiceMask( dwPort, g_VoiceMaskPresets[ RegistrySettings.VoiceMaskPreset ].mask );
        //$HACK: We're always telling the online APIs the user signed in on controller 0.
        AddOnlinePresenceFlag( 0, XONLINE_FRIENDSTATE_FLAG_VOICE );
        break;
    case VOICE_COMMUNICATOR_REMOVED:
        if( g_VoiceManager.IsInChatSession() )
        {
            if( IsServer() )
                g_VoiceManager.RemoveChatter( PlayerList[ PlayerIndexFromPlayerID( LocalPlayerID ) ].xuid );
            SendVoiceInfoMessage( VOICEINFO_REMOVECHATTER, 0 );
        }

        //$HACK: We're always telling the online APIs the user signed in on controller 0.
        RemoveOnlinePresenceFlag( 0, XONLINE_FRIENDSTATE_FLAG_VOICE );
        break;
    }
}


//-----------------------------------------------------------------------------
// Name: ProcessChatterList
// Desc: ####################################
//-----------------------------------------------------------------------------
int ProcessChatterList(void)
{
    DumpMessageVar( "Network", "Got MSGEXT_CHATTER_LIST" );

    assert( IsClient() );
    assert( SAME_INADDR(SoAddrFrom,pMachineServer->SoAddr) );

    const int cbHeader = sizeof(MSG_HEADER_EXT);
    DWORD* pdwChatterCount  = (DWORD *)(RecvMsgBuffer + cbHeader);
    XUID*  pxuidChatterList = (XUID *)( pdwChatterCount + 1 );

    // Now we're officially in the chat session
    g_VoiceManager.EnterChatSession();

    // Check which communicators are inserted
    for( DWORD i = 0; i < XGetPortCount(); i++ )
    {
        if( g_VoiceManager.IsCommunicatorInserted( i ) )
            OnCommunicatorEvent( i, VOICE_COMMUNICATOR_INSERTED, NULL );
    }

    // Do an ADDCHATTER on each
    for( DWORD i = 0; i < *pdwChatterCount; i++ )
    {
        g_VoiceManager.AddChatter( *pxuidChatterList );
        //$HACK: Spoofing controller 0
        if( g_VoiceManager.IsCommunicatorInserted( g_dwSignedInController ) &&
            g_FriendsManager.IsPlayerInMuteList( 0, *pxuidChatterList ) )
        {
            g_VoiceManager.MutePlayer( *pxuidChatterList, g_dwSignedInController );
            SendVoiceInfoMessage( VOICEINFO_ADDREMOTEMUTE, PlayerList[ PlayerIndexFromXUID( *pxuidChatterList ) ].PlayerID );
        }
        pxuidChatterList++;
    }

    // Return num bytes in message received
    return ((BYTE*)pxuidChatterList) - ((BYTE*)RecvMsgBuffer);
}



//-----------------------------------------------------------------------------
// Name: OnPlayerAdded
// Desc: Put anything here that should be done whenever a player is added
//          to the game.  This is called from the appropriate spots on both
//          the client and the server.
//-----------------------------------------------------------------------------
void OnPlayerAdded( NET_PLAYER* pPlayer, BOOL bPlaySound )
{
    g_PlayersStateEngine.m_bPlayersListChanged = TRUE;

    // For online matches, keep track of the dearly departed
    if( gTitleScreenVars.bUseXOnline )
    {
        for( PLAYER_LIST::iterator i = DepartedPlayerList.begin(); i < DepartedPlayerList.end(); ++i )
        {
            if( IsSameXUID( pPlayer->xuid, i->xuid ) )
            {
                DepartedPlayerList.erase( i );
            }
        }
    }

    if( bPlaySound )
        g_SoundEngine.Play2DSound( EFFECT_Honka, FALSE );
}



//-----------------------------------------------------------------------------
// Name: OnPlayerRemoved
// Desc: Put anything here that should be done whenever a player is removed
//          from the game.  This is called from the appropriate spots on both
//          the client and the server.
//-----------------------------------------------------------------------------
void OnPlayerRemoved( NET_PLAYER* pPlayer, BOOL bPlaySound )
{
    g_PlayersStateEngine.m_bPlayersListChanged = TRUE;
    if( g_VoiceManager.DoesPlayerHaveVoice( pPlayer->xuid ) )
    {
        g_VoiceManager.RemoveChatter( pPlayer->xuid );
    }

    // For online matches, keep track of the dearly departed.
    if( gTitleScreenVars.bUseXOnline )
    {
#if _DEBUG
        // Verify they're not already in the list
        for( PLAYER_LIST::iterator i = DepartedPlayerList.begin(); i < DepartedPlayerList.end(); ++i )
        {
            if( IsSameXUID( pPlayer->xuid, i->xuid ) )
            {
                assert( FALSE );
            }
        }
#endif // _DEBUG

        // If we're at max, erase the first guy in the list
        if( DepartedPlayerList.size() == MAX_DEPARTED_PLAYERS )
        {
            DepartedPlayerList.erase( DepartedPlayerList.begin() );
        }

        assert( DepartedPlayerList.size() < MAX_DEPARTED_PLAYERS );
        DepartedPlayerList.push_back( *pPlayer );
    }

    if( bPlaySound )
        g_SoundEngine.Play2DSound( EFFECT_BoxSlide, FALSE );
}



//-----------------------------------------------------------------------------
// Name: OnSessionEntered
// Desc: Put anything here that should be done whenever a player enters a
//          multiplayer game session.  This is called from the appropriate
//          spots on both the client and the server.
//-----------------------------------------------------------------------------
void OnSessionEntered()
{
    //$HACK: We're always telling the online APIs the user signed in on controller 0.
    if( gTitleScreenVars.bUseXOnline )
        AddOnlinePresenceFlag( 0, XONLINE_FRIENDSTATE_FLAG_JOINABLE );

    //$REVISIT: it's kind of ugly to do this here, since it's a server-only thing,
    // and this function is supposedly for things done on the server *and* client.
    if( IsServer() )
    {
        // Enter the chat session
        g_VoiceManager.EnterChatSession();

        // Check which communicators are inserted
        for( DWORD i = 0; i < XGetPortCount(); i++ )
        {
            if( g_VoiceManager.IsCommunicatorInserted( i ) )
                OnCommunicatorEvent( i, VOICE_COMMUNICATOR_INSERTED, NULL );
        }
    }
}



//-----------------------------------------------------------------------------
// Name: OnSessionExited
// Desc: Put anything here that should be done whenever a player exits a 
//          multiplayer game session.  This is called from the appropriate
//          spots on both the client and the server.
//-----------------------------------------------------------------------------
void OnSessionExited()
{
    //$HACK: We're always telling the online APIs the user signed in on controller 0.
    RemoveOnlinePresenceFlag( 0, XONLINE_FRIENDSTATE_FLAG_JOINABLE );

    //$SINGLEPLAYER - Assuming single local player
    if( IsLoggedIn(0) )
    {
        RevokeAllInvites();
    }

    // Leaving the session - no more voice.  Chatters will all get cleaned
    // up below when we call OnPlayerRemoved. 
    g_VoiceManager.LeaveChatSession();

    // remove all players from the game
    for( int p = 0 ; p < PlayerCount ; p++ )
    {
        OnPlayerRemoved( &PlayerList[p], FALSE );
    }
    DepartedPlayerList.clear();
}



#ifndef XBOX_DISABLE_NETWORK //$NOTE: not supporting dynamic packets/sec right now...
////////////////////////
// get send queue num //
////////////////////////

unsigned long GetSendQueueLength(void) //$NOTE: was originally GetSendQueue(DPID id)
{
    unsigned long num, size;
    HRESULT r;

    r = DP->GetMessageQueue(0, id, DPMESSAGEQUEUE_SEND, &num, &size);
    if (r != DP_OK)
    {
        num = 0;
    }

    return num;
}
#endif // !XBOX_DISABLE_NETWORK

//$REMOVED_NOTUSED
/////////////////////////////
//// get receive queue num //
/////////////////////////////
//
//unsigned long GetReceiveQueue(DPID id) //$NOTE: should be called GetReceiveQueueLength()
//{
//    unsigned long num, size;
//    HRESULT r;
//
//    r = DP->GetMessageQueue(id, 0, DPMESSAGEQUEUE_RECEIVE, &num, &size);
//    if (r != DP_OK)
//    {
//        num = 0;
//    }
//
//    return num;
//}
//$END_REMOVAL


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
//$ADDITION
    //$NOTE: maybe this func should be called CheckAllMachinesReady() or CheckAllClientsReady()

    assert( IsServer() );  // only server should do be calling us; clients will set AllPlayersReady in ProcessCountdownStart(), after server calls SetCountdownStart() here.

    //$NOTE: end result of calling this function is that when all players are ready,
    /// sets AllPlayersReady=TRUE and calls SendCountdownStart()
    /// Clients will set AllPlayerReady=TRUE in ProcessCountdownStart()
//$END_ADDITION

    PLAYER *player;

// appropriate?

    if (AllPlayersReady)
        return;

//$REMOVED
//    if (!IsServer())
//        return;
//$END_REMOVAL

// force if timeout expired

    AllReadyTimeout -= TimeStep;
    if (AllReadyTimeout < 0.0f)
    {
        AllPlayersReady = TRUE;
        SendCountdownStart();
        return;
    }

// check

//$BUGBUG: should probably only check per-machine here, rather than per-(client-)player
    for (player = PLR_PlayerHead ; player ; player = player->next)
    {
        if (player->type != PLAYER_NONE && !player->Ready)
            return;
    }

// all ready

    AllPlayersReady = TRUE;
    SendCountdownStart();
}



//$REMOVED -- we're bypassing this b/c no need to select connection on Xbox
//
/////////////////////////////////
//// choose network connection //
/////////////////////////////////
//
//void ConnectionMenu(void)
//{
//    short i;
//    long col;
//
//// buffer flip / clear
//
////$REMOVED    CheckSurfaces();
//    FlipBuffers();
//    ClearBuffers();
//
//// update pos
//
////$REMOVED    ReadMouse();
////$REMOVED    ReadKeyboard();
//    ReadJoystick(); //$ADDITION
//
//    if (Keys[DIK_UP] && !LastKeys[DIK_UP] && MenuCount) MenuCount--;
//    if (Keys[DIK_DOWN] && !LastKeys[DIK_DOWN] && MenuCount < ConnectionCount - 1) MenuCount++;
//
//// show menu
//
//    D3Ddevice->BeginScene();
//
////  BlitBitmap(TitleHbm, &BackBuffer);
//
//    BeginTextState();
//
//    DumpText(128, 112, 12, 24, 0x808000, "Select Connection:");
//
//    for (i = 0 ; i < ConnectionCount ; i++)
//    {
//        if (MenuCount == i) col = 0xff0000;
//        else col = 0x808080;
//        DumpText(128, i * 48 + 176, 12, 24, col, Connection[i].Name);
//    }
//
//    D3Ddevice->EndScene();
//
//// selected?
//
//    if (Keys[DIK_ESCAPE] && !LastKeys[DIK_ESCAPE])
//    {
//        KillNetwork();
//        MenuCount = 0;
//        SET_EVENT(MainMenu);
//    }       
//
//    if (Keys[DIK_RETURN] && !LastKeys[DIK_RETURN])
//    {
//        InitConnection((char)MenuCount);
//
//        if (IsServer())
//        {
//            MenuCount = 0;
//            SET_EVENT(EnterSessionName);  //$MODIFIED: was GetSessionName
//        }
//
//        else
//        {
//            MenuCount = 0;
//            SessionCount = 0;  Need to call ClearSessionList() instead here, to do proper cleanup.
//            SessionRequestTime = 0.0f;
//            SET_EVENT(BrowseSessions);  //$MODIFIED: was LookForSessions
//        }
//    }
//}
//$END_REMOVAL






///////////////////////
// send game started //
///////////////////////

void SendGameStarted(void)
{
    assert( IsServer() );  // $ADDITION

    const int cbHeader = sizeof(MSG_HEADER_TCP);
    BYTE* pbDataSend = (BYTE*)(SendMsgBuffer + cbHeader);

    memcpy( pbDataSend, &StartData, sizeof(START_DATA) );

//$MODIFIED
//    SendMessageGuaranteed(SendHeader, sizeof(MESSAGE_HEADER) + sizeof(START_DATA), GroupID);
//

    //$BUGBUG: really, we only want to send to machines that have connected players (and only send once per machine probably)

    // send message to each connected machine
    int cbSend = cbHeader + sizeof(START_DATA);
    SetSendMsgHeaderTcp( MSG_OTHER, MSGEXT_GAME_STARTED, cbSend );
    for( int i = 1 ; i < MachineCount ; i++ )  // start at [1] b/c [0] is local machine (ie, the server)
    {
        SOCKET so = MachineList[i].Socket;
        send( so, SendMsgBuffer, cbSend, 0 ); // guaranteed send
    }
//$END_MODIFICATIONS

    //$HACK(Apr02_GameBash) - if we're starting the game,
    // kick the user out of the players/friends menu
    if( g_pActiveStateEngine == &g_FriendsStateEngine )
    {
        g_FriendsStateEngine.Return( STATEENGINE_TERMINATED );
    }
    else if( g_pActiveStateEngine == &g_PlayersStateEngine )
    {
        g_PlayersStateEngine.Return( STATEENGINE_TERMINATED );
    }
    g_FriendsManager.StopUpdatingFriends( 0 );
}

//////////////////////////
// process game started //
//////////////////////////

int ProcessGameStarted(void)
{
    assert( IsClient() );  //$ADDITION

// set flags

    bGameStarted = TRUE;
    ServerID = FromID;

// save start data

    const int cbHeader = sizeof(MSG_HEADER_EXT);
    const int cbMessageRecv = cbHeader + sizeof(START_DATA);
    MultiStartData = *(START_DATA*)(RecvMsgBuffer + cbHeader);

    //$HACK(Apr02_GameBash) - if we're starting the game,
    // kick the user out of the players/friends menu
    if( g_pActiveStateEngine == &g_FriendsStateEngine )
    {
        g_FriendsStateEngine.Return( STATEENGINE_TERMINATED );
    }
    else if( g_pActiveStateEngine == &g_PlayersStateEngine )
    {
        g_PlayersStateEngine.Return( STATEENGINE_TERMINATED );
    }
    g_FriendsManager.StopUpdatingFriends( 0 );

// revoke any pending game invitations

    if( IsLoggedIn(0) ) //$SINGLEPLAYER - Assuming single local player
    {
        RevokeAllInvites();
    }

// return num bytes in message received
    return cbMessageRecv;
}

/////////////////////////
// send a sync request //
/////////////////////////

void SendSyncRequest(void)
{
    assert( IsClient() );  //$ADDITION

    const int cbHeader = sizeof(MSG_HEADER_EXT);
    unsigned long *ptr = (unsigned long*)(SendMsgBuffer + cbHeader);

// send request with my time

    ptr[0] = CurrentTimer();

//$MODIFIED:
//    SendMessage(SendHeader, sizeof(MESSAGE_HEADER) + sizeof(unsigned long), ServerID);

    int cbSend = cbHeader + sizeof(unsigned long);
    SetSendMsgHeaderExt( MSGEXT_SYNC_REQUEST );
    sendto( soUDP, SendMsgBuffer, cbSend, 0,
            (sockaddr*)&(pMachineServer->SoAddr), sizeof(SOCKADDR_IN) );
//$END_MODIFICATIONS
}

////////////////////////////
// process a sync request //
////////////////////////////

int ProcessSyncRequest(void)
{
    const int cbHeader = sizeof(MSG_HEADER_EXT); // same size for send and recv header
    const int cbMessageRecv = cbHeader + sizeof(unsigned long);
    unsigned long *rptr = (unsigned long*)(RecvMsgBuffer + cbHeader);
    unsigned long *sptr = (unsigned long*)(SendMsgBuffer + cbHeader);

// return client + host time

    sptr[0] = rptr[0];
    sptr[1] = CurrentTimer();

//$MODIFIED
//    SendMessage(SendHeader, sizeof(MESSAGE_HEADER) + sizeof(unsigned long) * 2, FromID);

    int cbSend = cbHeader + 2*sizeof(unsigned long);
    SetSendMsgHeaderExt( MSGEXT_SYNC_REPLY );
    sendto( soUDP, SendMsgBuffer, cbSend, 0,
            (sockaddr*)&SoAddrFrom, sizeof(SOCKADDR_IN) );  // direct-send reply
//$END_MODIFICATIONS

// return num bytes in message received
    return cbMessageRecv;
}

//////////////////////////
// process a sync reply //
//////////////////////////

int ProcessSyncReply(void)
{
    const int cbHeader = sizeof(MSG_HEADER_EXT);
    const int cbMessageRecv = cbHeader + 2*sizeof(unsigned long);
    unsigned long *ptr = (unsigned long*)(RecvMsgBuffer + cbHeader);
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

// return num bytes in message received
    return cbMessageRecv;
}

//////////////////////
// send game loaded //
//////////////////////

void SendGameLoaded(void)
{
//$MODIFIED
//// set timeout counter
//
//    AllReadyTimeout = 60.0f;
//
//// send loaded message
//
//    SendHeader->MsgType = MSGEXT_GAME_LOADED;
//    SendMessageGuaranteed(SendHeader, sizeof(MESSAGE_HEADER), GroupID);

    if( IsServer() )
    {
        AllReadyTimeout = 20.0f;  //$MODIFIED(cprince): was 60.0f originally, but that felt like too long.
                                  //$REVISIT(cprince): need to decide on the final value for this timeout.

        PLR_LocalPlayer->Ready = TRUE; //$BUG: assumes only 1 local player
    }
    else
    {
        assert( IsClient() );

        const int cbHeader = sizeof(MSG_HEADER_TCP);
        int cbSend = cbHeader + 0;
        SetSendMsgHeaderTcp( MSG_OTHER, MSGEXT_GAME_LOADED, cbSend );
        send( pMachineServer->Socket, SendMsgBuffer, cbSend, 0 ); // guaranteed send, to server
    }
//$END_MODIFICATIONS
}

/////////////////////////
// process game loaded //
/////////////////////////

int ProcessGameLoaded(void)
{
    const int cbMessageRecv = sizeof(MSG_HEADER_EXT);
    PLAYER* pPlayer;

    // find loaded player
    pPlayer = GetPlayerFromPlayerID(FromID);
    if (!pPlayer)
        goto Return;

    pPlayer->Ready = TRUE;

Return:
    // return num bytes in message received
    return cbMessageRecv;
}

//////////////////////////
// send countdown start //
//////////////////////////

void SendCountdownStart(void)
{
    assert( IsServer() );  //$ADDITION

    const int cbHeader = sizeof(MSG_HEADER_TCP);
    unsigned long *ptr = (unsigned long*)(SendMsgBuffer + cbHeader);
    unsigned long time = CurrentTimer();

// send start time + timer freq

    ptr[0] = time;
    ptr[1] = TimerFreq;

//$MODIFIED:
//    SendMessageGuaranteed(SendHeader, sizeof(MESSAGE_HEADER) + sizeof(long) * 2, GroupID);

    //$BUGBUG: really, we only want to send to machines that have connected players (and only send once per machine probably)

    // send message to each connected machine
    int cbSend = cbHeader + 2*sizeof(unsigned long);
    SetSendMsgHeaderTcp( MSG_OTHER, MSGEXT_COUNTDOWN_START, cbSend );
    for( int i = 1 ; i < MachineCount ; i++ )  // start at [1] b/c [0] is local machine (ie, the server)
    {
        SOCKET so = MachineList[i].Socket;
        send( so, SendMsgBuffer, cbSend, 0 ); // guaranteed send
    }
//$END_MODIFICATIONS

// start countdown timer

    CountdownEndTime = time + MS2TIME(COUNTDOWN_START);
}

/////////////////////////////
// process countdown start //
/////////////////////////////

int ProcessCountdownStart(void)
{
    const int cbHeader = sizeof(MSG_HEADER_EXT);
    const int cbMessageRecv = cbHeader + 2*sizeof(unsigned long);
    unsigned long *ptr = (unsigned long*)(RecvMsgBuffer + cbHeader);
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

// return num bytes in message received
    return cbMessageRecv;
}


//$REMOVED_UNREACHABLE
//////////////////////
//// send join info //
//////////////////////
//
//void SendJoinInfo(DPID id)
//{
//    JOIN_INFO *data = (JOIN_INFO*)(SendHeader + 1);
//
//// set message type
//
//    SendHeader->MsgType = MSGEXT_JOIN_INFO;
//
//// set race time
//
//    data->RaceTime = TotalRaceTime;
//
//// set level dir
//
//    strncpy(data->LevelDir, StartData.LevelDir, MAX_LEVEL_DIR_NAME);
//
//// send
//
//    SendMessageGuaranteed(SendHeader, sizeof(MESSAGE_HEADER) + sizeof(JOIN_INFO), id);
//}
//$END_REMOVAL

//$REMOVED_UNREACHABLE
/////////////////////////
//// process join info //
/////////////////////////
//
//void ProcessJoinInfo(void)
//{
//    JOIN_INFO *data = (JOIN_INFO*)(ReceiveHeader + 1);
//
//// set flags
//
//    ServerID = FromID;
//    bGameStarted = TRUE;
//
//// set level dir
//
//    strncpy(StartData.LevelDir, data->LevelDir, MAX_LEVEL_DIR_NAME);
///*  GameSettings.Level = GetLevelNum(data->LevelDir);
//    if (GameSettings.Level == -1)
//    {
//        sprintf(buf, "Can't find Level directory '%s'", StartData.LevelDir);
//        Box(NULL, buf, MB_OK);
//        QuitGame = TRUE;
//        return;
//    }*/
//
//// save race time
//
//    TotalRaceTime = data->RaceTime;
//}
//$END_REMOVAL

///////////////////////////
// send race finish time //
///////////////////////////

void SendRaceFinishTime(void)
{
    const int cbHeader = sizeof(MSG_HEADER_EXT);
    RACE_TIME_INFO* pInfo = (RACE_TIME_INFO*)(SendMsgBuffer + cbHeader);

// set message type

    SetSendMsgHeaderExt( MSGEXT_RACE_FINISH_TIME );

// set race time

    pInfo->Time = TotalRaceTime;

// send

    QueueMessage( SendMsgBuffer, cbHeader + sizeof(RACE_TIME_INFO) );
}

//////////////////////////////
// process race finish time //
//////////////////////////////

int ProcessRaceFinishTime(void)
{
    const int cbHeader = sizeof(MSG_HEADER_EXT);
    const int cbMessageRecv = cbHeader + sizeof(RACE_TIME_INFO);
    RACE_TIME_INFO* pInfo = (RACE_TIME_INFO*)(RecvMsgBuffer + cbHeader);
    PLAYER* pPlayer;

    // get player
    pPlayer = GetPlayerFromPlayerID(FromID);
    if( pPlayer )
    {
        // set race finish time
        SetPlayerFinished(pPlayer, pInfo->Time);
    }

    // return num bytes in message received
    return cbMessageRecv;
}

///////////////////////////////////////////////
// send a 'sync with me' message to a player //
///////////////////////////////////////////////

void SendPlayerSync(void)  //$HEY: should probably rename this SendMachineSync !!
{
    assert( IsServer() ); //$ADDITION

    const int cbHeader = sizeof(MSG_HEADER);
    unsigned long *ptr = (unsigned long*)(SendMsgBuffer + cbHeader);
//$MODIFIED - we do per-machine, not per-player
//    long count;
//    PLAYER *player;
//
//// find player
//
//    count = NextSyncMachine;
//    for (player = PLR_PlayerHead ; player ; player = player->next)
//    {
//        if (player != PLR_LocalPlayer)
//            count--;
//
//        if (count < 0)
//            break;
//    }
//
//    if (!player)
//        return;
//
//// send sync request + my race time
//
//    SendHeader->MsgType = MSG_PLAYER_SYNC_REQUEST;
//    ptr[0] = TotalRaceTime;
//
//    SendMessage(SendHeader, sizeof(MESSAGE_HEADER) + sizeof(long), player->PlayerID);
//
//// set next sync player
//
//    NextSyncMachine++;
//    NextSyncMachine %= (MachineCount - 1);

    assert( NextSyncMachine > 0 );  // [0] is server (this machine), and don't want to send sync to self

    if( NextSyncMachine < MachineCount )  // protect against the (rare) case where there are no client machines, or clients have dropped
    {
// send sync request + my race time
        ptr[0] = TotalRaceTime;
    
        int cbSend = cbHeader + sizeof(unsigned long);
        SetSendMsgHeader( MSG_PLAYER_SYNC_REQUEST );
        sendto( soUDP, SendMsgBuffer, cbSend, 0,
                (sockaddr*)&MachineList[NextSyncMachine].SoAddr, sizeof(SOCKADDR_IN) );
    }

// set next sync player
    NextSyncMachine++;
    if( NextSyncMachine >= MachineCount )
    {
        NextSyncMachine = 1;  // start at [1], b/c [0] is server (this machine)
    }
//$END_MODIFICATIONS
}

//////////////////////////////////
// reply to player sync message //
//////////////////////////////////

int ProcessPlayerSync1(void)  //$HEY: should probably rename this ProcessMachineSync1 !!
{
    const int cbHeader = sizeof(MSG_HEADER); // same size for send and recv header
    const int cbMessageRecv = cbHeader + sizeof(unsigned long);
    unsigned long *rptr = (unsigned long*)(RecvMsgBuffer + cbHeader);
    unsigned long *sptr = (unsigned long*)(SendMsgBuffer + cbHeader);

// send back host race time + my race time

    sptr[0] = rptr[0];
    sptr[1] = TotalRaceTime;

//$MODIFIED
//    SendMessage(SendHeader, sizeof(MESSAGE_HEADER) + sizeof(unsigned long) * 2, FromID);

    int cbSend = cbHeader + 2*sizeof(unsigned long);
    SetSendMsgHeader( MSG_PLAYER_SYNC_REPLY1 );
    sendto( soUDP, SendMsgBuffer, cbSend, 0,
            (sockaddr*)&SoAddrFrom, sizeof(SOCKADDR_IN) );  // direct-send reply
//$END_MODIFICATIONS

// return num bytes in message received
    return cbMessageRecv;
}

//////////////////////////////////
// reply to player sync message //
//////////////////////////////////

int ProcessPlayerSync2(void)  //$HEY: should probably rename this ProcessMachineSync2 !!
{
    const int cbHeader = sizeof(MSG_HEADER);  // same size for send and recv header
    const int cbMessageRecv = cbHeader + 2*sizeof(unsigned long);
    unsigned long *rptr = (unsigned long*)(RecvMsgBuffer + cbHeader);
    unsigned long *sptr = (unsigned long*)(SendMsgBuffer + cbHeader);
//$REMOVED    PLAYER *player;

// return my old race time + client race time + my new race time

    sptr[0] = rptr[0];
    sptr[1] = rptr[1];
    sptr[2] = TotalRaceTime;

//$MODIFIED
//    SendMessage(SendHeader, sizeof(MESSAGE_HEADER) + sizeof(unsigned long) * 3, FromID);

    int cbSend = cbHeader + 3*sizeof(unsigned long);
    SetSendMsgHeader( MSG_PLAYER_SYNC_REPLY2 );
    sendto( soUDP, SendMsgBuffer, cbSend, 0,
            (sockaddr*)&SoAddrFrom, sizeof(SOCKADDR_IN) );  // direct-send reply
//$END_MODIFICATIONS

//$REMOVED
//// set host->client->host last ping
//
//    player = GetPlayerFromPlayerID(FromID);
//    if (player)
//    {
//        player->LastPing = sptr[2] - sptr[0];
//    }
//$END_REMOVAL

// return num bytes in message received
    return cbMessageRecv;
}

//////////////////////////////////
// reply to player sync message //
//////////////////////////////////

int ProcessPlayerSync3(void)  //$HEY: should probably rename this ProcessMachineSync3 !!
{
    const int cbHeader = sizeof(MSG_HEADER);
    const int cbMessageRecv = cbHeader + 3*sizeof(unsigned long);
    unsigned long *ptr = (unsigned long*)(RecvMsgBuffer + cbHeader);
    unsigned long host1, host2, client1, ping, newracetime, adjust;
    PLAYER* pPlayer;

// get host / client race times

    host1 = ptr[0];
    host2 = ptr[2];
    client1 = ptr[1];

// get host->client->host ping, quit if over 1 second

    ping = host2 - host1;
    if (ping > DP_SYNC_PING_MAX)
        goto Return;

    //$REVISIT(cprince): the following code looks a bit scary...
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

        for (pPlayer = PLR_PlayerHead ; pPlayer ; pPlayer = pPlayer->next)
        {
            pPlayer->car.CurrentLapStartTime -= adjust;
        }
    }
    else
    {
        adjust = MS2TIME(TotalRaceTime - newracetime);

        TotalRaceStartTime += adjust;

        for (pPlayer = PLR_PlayerHead ; pPlayer ; pPlayer = pPlayer->next)
        {
            pPlayer->car.CurrentLapStartTime += adjust;
        }
    }

//$REMOVED_DEBUGONLY
//// set client->host->client last ping
//
//    pPlayer = GetPlayerFromPlayerID(FromID);
//    if (pPlayer)
//    {
//        pPlayer->LastPing = TotalRaceTime - client1;
//    }
//$END_REMOVAL

Return:
// return num bytes in message received
    return cbMessageRecv;
}

///////////////////////////////
// send race restart message //
///////////////////////////////

void SendMultiplayerRestart(void)
{
    assert( IsServer() ); //$ADDITION

    long i;
    LEVELINFO *li;
    const int cbHeader = sizeof(MSG_HEADER_TCP);
    RESTART_DATA *data = (RESTART_DATA*)(SendMsgBuffer + cbHeader); //$CLEANUP: how about modifying RestartData directly, and copying to send buffer afterward (more readable that way).
                                                                    /// (Only concern if there's some situation where send-buffer gets set, but early exit or something prevents copying into RestartData struct.)

// message

#ifdef OLD_AUDIO
    StopAllSfx();
#else
    g_SoundEngine.StopAll();
#endif // OLD_AUDIO
    SetBackgroundColor(MENU_COLOR_BLACK);
    SetViewport(CAM_MainCamera->X, CAM_MainCamera->Y, CAM_MainCamera->Xsize, CAM_MainCamera->Ysize, BaseGeomPers + CAM_MainCamera->Lens);
    SetCameraView(&CAM_MainCamera->WMatrix, &CAM_MainCamera->WPos, CAM_MainCamera->Shake);

    for (i = 0 ; i < 3 ; i++)
    {
        ClearBuffers();
        InitRenderStates();
        BeginTextState();
        SET_TPAGE(TPAGE_FONT);
        g_pFont->SetScaleFactors( 1.5f, 1.5f );
        g_pFont->DrawText( 320, 240, MENU_COLOR_OPAQUE|MENU_COLOR_WHITE, TEXT_TABLE(TEXT_RESTARTING), XBFONT_CENTER_X|XBFONT_CENTER_Y );
        g_pFont->SetScaleFactors( 1.0f, 1.0f );
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
        strncpy(StartData.LevelDir, li->szDir, MAX_PATH);
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

        strncpy(data->LevelDir, StartData.LevelDir, MAX_PATH);
    }

// set new rand seed

    StartData.Seed = rand();
    data->Seed = StartData.Seed;

// copy restart data to global

    RestartData = *(RESTART_DATA*)(SendMsgBuffer + cbHeader);

// send restart

//$MODIFIED
//    SendMessageGuaranteed(SendHeader, sizeof(MESSAGE_HEADER) + sizeof(RESTART_DATA), GroupID);

    //$BUGBUG: really, we only want to send to machines that still have connected players (and only send once per machine probably)

    // send message to each connected machine
    int cbSend = cbHeader + sizeof(RESTART_DATA);
    SetSendMsgHeaderTcp( MSG_OTHER, MSGEXT_RESTART, cbSend );
    for( i = 1 ; i < MachineCount ; i++ )  // start at [1] b/c [0] is local machine (ie, the server)
    {
        SOCKET so = MachineList[i].Socket;
        send( so, SendMsgBuffer, cbSend, 0 ); // guaranteed send
    }
//$END_MODIFICATIONS

// sync

    RemoteSyncHost();
}

/////////////////////////////////
// process multiplayer restart //
/////////////////////////////////

int ProcessMultiplayerRestart(void)
{

// save data

    const int cbHeader = sizeof(MSG_HEADER_EXT);
    const int cbMessageRecv = cbHeader + sizeof(RESTART_DATA);
    RestartData = *(RESTART_DATA*)(RecvMsgBuffer + cbHeader);

// start fade down

    GameLoopQuit = GAMELOOP_QUIT_RESTART;  //NOTE: this will cause us to call ClientMultiplayerRestart (see below; where we use RestartData values) next time thru gameloop.
    SetFadeEffect(FADE_DOWN);

// return num bytes in message received
    return cbMessageRecv;
}

/////////////////////////
// restart race client //
/////////////////////////

void ClientMultiplayerRestart(void)
{
    long i;

// message

#ifdef OLD_AUDIO
    StopAllSfx();
#else
    g_SoundEngine.StopAll();
#endif // OLD_AUDIO
    SetBackgroundColor( MENU_COLOR_BLACK );
    SetViewport(CAM_MainCamera->X, CAM_MainCamera->Y, CAM_MainCamera->Xsize, CAM_MainCamera->Ysize, BaseGeomPers + CAM_MainCamera->Lens);
    SetCameraView(&CAM_MainCamera->WMatrix, &CAM_MainCamera->WPos, CAM_MainCamera->Shake);

    for (i = 0 ; i < 3 ; i++)
    {
        ClearBuffers();
        InitRenderStates();
        BeginTextState();
        SET_TPAGE(TPAGE_FONT);
        g_pFont->SetScaleFactors( 1.5f, 1.5f );
        g_pFont->DrawText( 320, 240, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_HOST_RESTARTING), XBFONT_CENTER_X|XBFONT_CENTER_Y );
        g_pFont->SetScaleFactors( 1.0f, 1.0f );
        FlipBuffers();
    }

    SetBackgroundColor(CurrentLevelInfo.FogColor);

// set random cars if relevant

    if (GameSettings.RandomCars)
    {
        long car = GetCarTypeFromName(RestartData.CarName);

        for (i = 0 ; i < StartData.PlayerNum ; i++)
            StartData.PlayerData[i].CarType = car;  //$REVISIT: they set all cars to same type during races!?  Seems like we don't want to do this.  (Maybe only applicable if RandomCars is true?)
    }

// set random track if relevant

    if (RestartData.NewTrack)
    {
        LEV_EndLevelStageTwo();
        LEV_EndLevelStageOne();

        LoadMipTexture("D:\\gfx\\font.bmp", TPAGE_FONT, 256, 256, 0, 1, FALSE); //$MODIFIED: added "D:\\" at start
        LoadMipTexture("D:\\gfx\\loading.bmp", TPAGE_LOADING, 256, 256, 0, 1, FALSE); //$MODIFIED: added "D:\\" at start
        LoadMipTexture("D:\\gfx\\spru.bmp", TPAGE_SPRU, 256, 256, 0, 1, FALSE); //$MODIFIED: added "D:\\" at start

        strncpy(StartData.LevelDir, RestartData.LevelDir, MAX_PATH);
        GameSettings.Level = GetLevelNum(StartData.LevelDir);
    }

// set rand seed

    StartData.Seed = RestartData.Seed;

// clear menu
    // July 2002 Consumer Beta Hack:
    // If a client is in the audio options menu, their communicator
    // is set to loopback mode, and won't be cleared unless they
    // go back to it after the restart.
    if( g_pMenuHeader->m_pMenu == &Menu_AudioSettings )
    {
        for( DWORD i = 0; i < XGetPortCount(); i++ )
        {
            if( g_VoiceManager.IsCommunicatorInserted( i ) )
                g_VoiceManager.SetLoopback( i, FALSE );
        }
    }
    g_pMenuHeader->m_pMenu = NULL;

// sync

    RemoteSyncClient();
}

//////////////////////////
// send player position //
//////////////////////////

void SendPosition(void)
{
    const int cbHeader = sizeof(MSG_HEADER);
    long *ptr = (long*)(SendMsgBuffer + cbHeader);

    SetSendMsgHeader( MSG_POSITION );

    ptr[0] = PLR_LocalPlayer->CarAI.FinishDistNode;
    ptr[1] = PLR_LocalPlayer->CarAI.ZoneID;
//$ADDITION (temporary)
        assert( ptr[1] >= 0  &&  ptr[1] < 128 );
//$END_ADDITION
    QueueMessage( SendMsgBuffer, cbHeader + 2*sizeof(long) );
}

/////////////////////////////
// process player position //
/////////////////////////////

int ProcessPosition(void)
{
    const int cbHeader = sizeof(MSG_HEADER);
    const int cbMessageRecv = cbHeader + 2*sizeof(long);
    long *ptr = (long*)(RecvMsgBuffer + cbHeader);
    PLAYER* pPlayer;

    pPlayer = GetPlayerFromPlayerID(FromID);
    if (pPlayer)
    {
        pPlayer->CarAI.FinishDistNode = ptr[0];
        pPlayer->CarAI.ZoneID = ptr[1];
//$ADDITION (temporary)
        assert( ptr[1] >= 0  &&  ptr[1] < 128 );
//$END_ADDITION
    }

// return num bytes in message received
    return cbMessageRecv;
}

////////////////////////
// send bomb transfer //
////////////////////////

void SendTransferBomb(GLOBAL_ID objid, unsigned long player1id, unsigned long player2id)
{
    const int cbHeader = sizeof(MSG_HEADER_EXT);
    BOMB_TRANSFER_DATA* pData = (BOMB_TRANSFER_DATA*)(SendMsgBuffer + cbHeader);

    SetSendMsgHeaderExt( MSGEXT_TRANSFER_BOMB );

    pData->ObjID     = objid;
    pData->Player1ID = player1id;
    pData->Player2ID = player2id;

    QueueMessage( SendMsgBuffer, cbHeader + sizeof(BOMB_TRANSFER_DATA) );
}

///////////////////////////
// process bomb transfer //
///////////////////////////

int ProcessTransferBomb(void)
{
    const int cbHeader = sizeof(MSG_HEADER_EXT);
    const int cbMessageRecv = cbHeader + sizeof(BOMB_TRANSFER_DATA);
    BOMB_TRANSFER_DATA* pData = (BOMB_TRANSFER_DATA*)(RecvMsgBuffer + cbHeader);
    PLAYER *player1, *player2;
    OBJECT *obj;

    obj = GetObjectFromGlobalID(pData->ObjID);
    player1 = GetPlayerFromPlayerID(pData->Player1ID);
    player2 = GetPlayerFromPlayerID(pData->Player2ID);

    if (obj && player1 && player2)
    {
        TransferBomb(obj, player1, player2);
    }

// return num bytes in message received
    return cbMessageRecv;
}

////////////////////////
// send fox transfer //
////////////////////////

void SendTransferFox(unsigned long player1id, unsigned long player2id)
{
    const int cbHeader = sizeof(MSG_HEADER_EXT);
    FOX_TRANSFER_DATA* pData = (FOX_TRANSFER_DATA*)(SendMsgBuffer + cbHeader);

    SetSendMsgHeaderExt( MSGEXT_TRANSFER_FOX );

    pData->Player1ID = player1id;
    pData->Player2ID = player2id;

    QueueMessage( SendMsgBuffer, cbHeader + sizeof(FOX_TRANSFER_DATA) );
}

///////////////////////////
// process bomb transfer //
///////////////////////////

int ProcessTransferFox(void)
{
    const int cbHeader = sizeof(MSG_HEADER_EXT);
    const int cbMessageRecv = cbHeader + sizeof(FOX_TRANSFER_DATA);
    FOX_TRANSFER_DATA* pData = (FOX_TRANSFER_DATA*)(RecvMsgBuffer + cbHeader);
    PLAYER *pPlayer1, *pPlayer2;
    OBJECT *pObj;

    pObj = NextObjectOfType(OBJ_ObjectHead, OBJECT_TYPE_FOX);
    pPlayer1 = GetPlayerFromPlayerID(pData->Player1ID);
    pPlayer2 = GetPlayerFromPlayerID(pData->Player2ID);

    if (pPlayer2)
    {
        TransferFox(pObj, pPlayer1, pPlayer2);
    }

// return num bytes in message received
    return cbMessageRecv;
}

/////////////////////////
// send bomb tag clock //
/////////////////////////

void SendBombTagClock(void)
{
    const int cbHeader = sizeof(MSG_HEADER_EXT);
    unsigned long* ptr = (unsigned long*)(SendMsgBuffer + cbHeader);

    SetSendMsgHeaderExt( MSGEXT_BOMBTAG_CLOCK );

    ptr[0] = PLR_LocalPlayer->BombTagTimer;

    QueueMessage( SendMsgBuffer, cbHeader + sizeof(unsigned long) );
}

/////////////////////////////
// processs bomb tag clock //
/////////////////////////////

int ProcessBombTagClock(void)
{
    const int cbHeader = sizeof(MSG_HEADER_EXT);
    const int cbMessageRecv = cbHeader + sizeof(unsigned long);
    unsigned long* ptr = (unsigned long*)(RecvMsgBuffer + cbHeader);
    PLAYER* pPlayer;

    pPlayer = GetPlayerFromPlayerID(FromID);
    if (pPlayer)
    {
        pPlayer->BombTagTimer = ptr[0];
    }

// return num bytes in message received
    return cbMessageRecv;
}

/////////////////////////////////
// send electropulse the world //
/////////////////////////////////

void SendElectroPulseTheWorld(long slot)
{
    const int cbHeader = sizeof(MSG_HEADER_EXT);
    long* ptr = (long*)(SendMsgBuffer + cbHeader);

    SetSendMsgHeaderExt( MSGEXT_ELECTROPULSE_THE_WORLD );

    ptr[0] = slot;

    QueueMessage( SendMsgBuffer, cbHeader + sizeof(long) );
}

////////////////////////////////////
// process electropulse the world //
////////////////////////////////////

int ProcessElectroPulseTheWorld(void)
{
    const int cbHeader = sizeof(MSG_HEADER_EXT);
    const int cbMessageRecv = cbHeader + sizeof(long);
    long* ptr = (long*)(RecvMsgBuffer + cbHeader);

    long slot = ptr[0];
    ElectroPulseTheWorld(slot);

// return num bytes in message received
    return cbMessageRecv;
}

/////////////////////
// send got global //
/////////////////////

void SendGotGlobal(void)
{
    SetSendMsgHeaderExt( MSGEXT_GOT_GLOBAL );
    QueueMessage( SendMsgBuffer, sizeof(MSG_HEADER_EXT) );
}

////////////////////////
// process got global //
///////////////////////

int ProcessGotGlobal(void)
{
    const int cbHeader = sizeof(MSG_HEADER_EXT);
    const int cbMessageRecv = cbHeader + 0;
    PLAYER* pPlayer;
    OBJECT* pObj;

// give dummy pickup to player

    pPlayer = GetPlayerFromPlayerID(FromID);
    if (pPlayer)
    {
        GivePickupToPlayer(pPlayer, PICKUP_TYPE_DUMMY);
    }

// find and kill global pickup

    for (pObj = OBJ_ObjectHead ; pObj ; pObj = pObj->next)
    {
        if (pObj->Type == OBJECT_TYPE_STAR)
        {
            STAR_OBJ* pStar = (STAR_OBJ*)pObj->Data;

            pStar->Mode = 1;
            pStar->Timer = 0.0f;

            SetVector(&pStar->Vel, pPlayer->ownobj->body.Centre.Vel.v[X] / 2.0f, -64.0f, pPlayer->ownobj->body.Centre.Vel.v[Z] / 2.0f);

#ifdef OLD_AUDIO
            PlaySfx3D(SFX_PICKUP, SFX_MAX_VOL, 22050, &pObj->body.Centre.Pos, 2);
#else
            g_SoundEngine.PlaySubmixedSound( EFFECT_Pickup, FALSE, pPlayer->car.pSourceMix );
#endif // OLD_AUDIO

            break;
        }
    }

// return num bytes in message received
    return cbMessageRecv;
}

////////////////
// send honka //
////////////////

void SendHonka(void)
{
    SetSendMsgHeaderExt( MSGEXT_HONKA );
    QueueMessage( SendMsgBuffer, sizeof(MSG_HEADER_EXT) );
}

///////////////////
// process honka //
///////////////////

int ProcessHonka(void)
{
    const int cbHeader = sizeof(MSG_HEADER_EXT);
    const int cbMessageRecv = cbHeader + 0;
    PLAYER* pPlayer;

// honk from player

    pPlayer = GetPlayerFromPlayerID(FromID);
    if (pPlayer)
    {
#ifdef OLD_AUDIO
        PlaySfx3D(SFX_HONKA, SFX_MAX_VOL, SFX_SAMPLE_RATE, &pPlayer->car.Body->Centre.Pos, 2);
#else
        g_SoundEngine.PlaySubmixedSound( EFFECT_Honka, FALSE, pPlayer->car.pSourceMix );
#endif // OLD_AUDIO
    }

// return num bytes in message received
    return cbMessageRecv;
}


////////////////////////////
// process system message //
////////////////////////////

    //$BUGBUG: not currently using ProcessSystemMessage
    /// (so either (A) move that functionality elsewhere, or (B) determine which function handles message based on Socket or Port it came in on.)

//$REMOVED (Tentative!!  We haven't attempted to moved this functionality elsewhere!!)
//void ProcessSystemMessage(void)
//{
//    DPMSG_GENERIC *Message = (DPMSG_GENERIC*)ReceiveHeader;
//
//    switch (Message->dwType)
//    {
//
//// new player joining
//
//        case DPSYS_CREATEPLAYERORGROUP:
//            ProcessPlayerJoining();
//            break;
//
//// existing player leaving
//
//        case DPSYS_DESTROYPLAYERORGROUP:
//            ProcessPlayerLeaving();
//            break;
//
//// session lost
//
//        case DPSYS_SESSIONLOST:
//            HostQuit = TRUE;
////          Box(NULL, "Multiplayer Session Was Lost!", MB_OK);
//            break;
//
//#ifndef XBOX_NOT_YET_IMPLEMENTED // we don't support host migration yet
//// become the host if game started
//
//        case DPSYS_HOST:
//            ProcessBecomeHost();
//            break;
//#endif // XBOX_NOT_YET_IMPLEMENTED
//    }
//}
//$END_REMOVAL









//$NOTE: ProcessCarMessage (network.cpp) --> ProcessCarData (car.cpp) in Aug99 drop

//-----------------------------------------------------------------------------
// Name: ProcessVoiceMessage
// Desc: Extracts the data from a MSG_VOICE_PACKET packet.
//-----------------------------------------------------------------------------
int ProcessVoiceMessage()
{
    //$NOTE(JHarding): Modeled after ProcessCarMessage.  Right now, all
    // voice is broadcast to all players
    WORD wLastSequenceNumber;
    const int cbHeader = sizeof(MSG_HEADER);
    BYTE* pbDataRecv = (BYTE*)(RecvMsgBuffer + cbHeader);

    BYTE nVoicePackets = *((BYTE*)pbDataRecv);
    pbDataRecv += sizeof(BYTE);

    // Parse the bundled packets and process each one individually
    for( int i=0 ; i < nVoicePackets ; i++ )
    {
        DWORD dwDataSize;
        VOID* pvData;
        DWORD p = PlayerIndexFromPlayerID( FromID );
        //$REVISIT: We should reject all incoming messages after
        // we've left the session

        BYTE bTemp[VOICE_BYTES_PER_FULL_PACKET];
        if( i == 0 )
        {
            pvData              = pbDataRecv;
            dwDataSize          = VOICE_BYTES_PER_FULL_PACKET;
            wLastSequenceNumber = ((XVOICE_CODEC_HEADER*)pbDataRecv)->wSeqNo;
        }
        else
        {
            VOICE_PAYLOAD_DELTA_HEADER* pDelta = (VOICE_PAYLOAD_DELTA_HEADER*)pbDataRecv;
            XVOICE_CODEC_HEADER*        pFull  = (XVOICE_CODEC_HEADER*)bTemp;

            // Translate the delta-encoded header to a full header
            // and copy in the voice data
            pFull->bMsgNo = pDelta->bMsgNum;
            pFull->wSeqNo = wLastSequenceNumber + pDelta->bSeqNumDelta;
            memcpy( bTemp + VOICE_FULL_HEADER_SIZE, pbDataRecv + VOICE_DELTA_HEADER_SIZE, VOICE_DATA_PER_PACKET );

            pvData              = bTemp;
            dwDataSize          = VOICE_BYTES_PER_DELTA_PACKET;
            wLastSequenceNumber = pFull->wSeqNo;
        }

        if( g_VoiceManager.IsInChatSession() )
        {
            g_VoiceManager.ReceivePacket( PlayerList[p].xuid, pvData, VOICE_BYTES_PER_FULL_PACKET );
        }
        pbDataRecv += dwDataSize;
    }

    // Return num bytes in message received
    return ((BYTE*)pbDataRecv) - ((BYTE*)RecvMsgBuffer);
}


//-----------------------------------------------------------------------------
// Name: SendVoiceInfoMessage
// Desc: Sends a MSGEXT_VOICE_INFO packet to everyone in the session.
//-----------------------------------------------------------------------------
void SendVoiceInfoMessage( DWORD dwOperation, DWORD dwDestPlayerID )
{
    assert( s_NetInitRefCount > 0 );

    const int cbHeader = sizeof(MSG_HEADER_TCP);
    VOICE_INFO* pInfo = (VOICE_INFO*)(SendMsgBuffer + cbHeader);

    pInfo->dwOperation    = dwOperation;
    pInfo->dwDestPlayerID = dwDestPlayerID;

    int cbSend = cbHeader + sizeof(VOICE_INFO);
    SetSendMsgHeaderTcp( MSG_OTHER, MSGEXT_VOICE_INFO, cbSend );

    if( IsClient() )
    {
        assert( pMachineServer->Socket );
        send( pMachineServer->Socket, SendMsgBuffer, cbSend, 0 );
    }
    else
    {
        assert( IsServer() );
        for( INT i = 1; i < MachineCount; i++ )
        {
            send( MachineList[i].Socket, SendMsgBuffer, cbSend, 0 );
        }
    }
}



//-----------------------------------------------------------------------------
// Name: ProcessVoiceInfoMessage
// Desc: Handles a MSGEXT_VOICE_INFO packet, which contains status update 
//          about how to handle voice
//-----------------------------------------------------------------------------
int ProcessVoiceInfoMessage()
{
    const int cbHeader = sizeof(MSG_HEADER_EXT);
    const int cbMessageRecv = cbHeader + sizeof(VOICE_INFO);
    VOICE_INFO* pInfoRecv = (VOICE_INFO*)(RecvMsgBuffer + cbHeader);

    DWORD p = PlayerIndexFromPlayerID( FromID );
    switch( pInfoRecv->dwOperation )
    {
    case VOICEINFO_ADDCHATTER:
        g_VoiceManager.AddChatter( PlayerList[p].xuid );

        //$HACK: Spoofing controller 0
        if( g_VoiceManager.IsCommunicatorInserted( g_dwSignedInController ) &&
            g_FriendsManager.IsPlayerInMuteList( 0, PlayerList[p].xuid ) )
        {
            g_VoiceManager.MutePlayer( PlayerList[p].xuid, g_dwSignedInController );
            SendVoiceInfoMessage( VOICEINFO_ADDREMOTEMUTE, FromID );
        }
        break;
    case VOICEINFO_REMOVECHATTER:
        g_VoiceManager.RemoveChatter( PlayerList[p].xuid );
        break;
    case VOICEINFO_ADDREMOTEMUTE:
        if( pInfoRecv->dwDestPlayerID == LocalPlayerID )
        {
            assert( g_VoiceManager.IsCommunicatorInserted( g_dwSignedInController ) );
            g_VoiceManager.RemoteMutePlayer( PlayerList[p].xuid, g_dwSignedInController );
        }
        break;
    case VOICEINFO_REMOVEREMOTEMUTE:
        if( pInfoRecv->dwDestPlayerID == LocalPlayerID )
        {
            assert( g_VoiceManager.IsCommunicatorInserted( g_dwSignedInController ) );
            g_VoiceManager.UnRemoteMutePlayer( PlayerList[p].xuid, g_dwSignedInController );
        }
        break;
    default:
        assert( FALSE );
    }

    // Forward the message to all other clients
    // (but need to change header type from MSG_HEADER_EXT to MSG_HEADER_TCP)
    if( IsServer() )
    {
        const int cbHeaderSend = sizeof(MSG_HEADER_TCP);
        BYTE* pbDataSend = (BYTE*)(SendMsgBuffer + cbHeaderSend);

        memcpy( pbDataSend, pInfoRecv, cbMessageRecv - cbHeader );

        int cbSend = (cbMessageRecv - cbHeader) + cbHeaderSend;
        SetSendMsgHeaderTcp( MSG_OTHER, MSGEXT_VOICE_INFO, cbSend );
        // Now change the player ID (in TypeAndID) from the local player ID
        // back to the player ID stored in the message we received.
        ((MSG_HEADER_TCP*)SendMsgBuffer)->TypeAndID = PackMsgTypeAndID( MSG_OTHER, FromID );

        for( int i = 1 ; i < MachineCount ; i++ )  // start at [1] b/c [0] is local machine (ie, the server)
        {
            send( MachineList[i].Socket, SendMsgBuffer, cbSend, 0 ); // guaranteed send
        }
    }

    // Return num bytes in message received
    return cbMessageRecv;
}



//$REMOVED (tentative!!)
//HEY -- THE *Ping* FUNCS HAVE BEEN REMOVED; I THINK THEY'VE BEEN DEPRECATED BY *Sync* FUNCS
//
////-----------------------------------------------------------------------------
//// Name: RequestPings
//// Desc: Requests ping times from each remote machine.
////-----------------------------------------------------------------------------
//void RequestPings(void)
//{
//    //$PERF: pings only need to be requested per-machine, instead of per-player !!
//
//    TODO: if we re-enable this function, then make the message-packing code match all other functions (SetSendMsgHeader**, const int cbHeader, etc)
//
//    MESSAGE_HEADER* pHeaderSend = (MESSAGE_HEADER*)SendMsgBuffer;
//    pHeaderSend->MsgType = MSGEXT_PING_REQUEST;
//
//    for( int i = 0 ; i < PlayerCount ; i++ )
//    {
//        if( PlayerList[i].PlayerID == LocalPlayerID )
//        {
//            // zero local player ping
//            PlayerList[i].Ping = 0;
//        }
//        else
//        {
//            // setup ping packet
//            BYTE* pbDataSend = (BYTE*)(pHeaderSend + 1);
//
//            *((long*)pbDataSend) = CurrentTimer();
//            pbDataSend += sizeof(long);
//
//            // send
//            int cbSend = ((BYTE*)pbDataSend) - ((BYTE*)pHeaderSend);
//            sendto( soUDP, SendMsgBuffer, cbSend, 0,
//                    (sockaddr*)&PlayerList[i].SoAddr, sizeof(SOCKADDR_IN) );  // direct-send (b/c makes direct-reply -- which we want -- much easier)
//        }
//    }
//}
//
//HEY -- THE *Ping* FUNCS HAVE BEEN REMOVED; I THINK THEY'VE BEEN DEPRECATED BY *Sync* FUNCS
////-----------------------------------------------------------------------------
//// Name: ProcessPingRequest
//// Desc: Sends a ping-reply for a ping-request message.
////-----------------------------------------------------------------------------
//void ProcessPingRequest(void)
//{
//    TODO: if we re-enable this function, then make the message-packing code match all other functions (SetSendMsgHeader**, const int cbHeader, etc)
//
//    MESSAGE_HEADER* pHeaderRecv = (MESSAGE_HEADER*)RecvMsgBuffer;
//    MESSAGE_HEADER* pHeaderSend = (MESSAGE_HEADER*)SendMsgBuffer;
//    BYTE* pbDataRecv = (BYTE*)(pHeaderRecv + 1);
//    BYTE* pbDataSend = (BYTE*)(pHeaderSend + 1);
//
//    // setup return packet
//    pHeaderSend->MsgType = MSGEXT_PING_RETURN;
//    *((long*)pbDataSend) = *((long*)pbDataRecv); // send back same value
//    pbDataSend += sizeof(long);
//    pbDataRecv += sizeof(long);
//
//    // send
//    int cbSend = ((BYTE*)pbDataSend) - ((BYTE*)pHeaderSend);
//    sendto( soUDP, SendMsgBuffer, cbSend, 0,
//            (sockaddr*)&SoAddrFrom, sizeof(SOCKADDR_IN) );  // direct-send reply
//}
//
//HEY -- THE *Ping* FUNCS HAVE BEEN REMOVED; I THINK THEY'VE BEEN DEPRECATED BY *Sync* FUNCS
////-----------------------------------------------------------------------------
//// Name: ProcessPingReturn
//// Desc: Handles the ping-reply message.
////-----------------------------------------------------------------------------
//void ProcessPingReturn(void)
//{
//    MESSAGE_HEADER* pHeaderRecv = (MESSAGE_HEADER*)RecvMsgBuffer;
//    BYTE* pbDataRecv = (BYTE*)(pHeaderRecv + 1);
//
//    // calculate ping time
//    long timeSent = *((long*)pbDataRecv);
//    pbDataRecv += sizeof(long);
//
//    long ping = TIME2MS( CurrentTimer() - timeSent );
//
//    // find correct player and store ping
//    for( int i = 0 ; i < PlayerCount ; i++ )
//    {
//        if( SAME_INADDR(SoAddrFrom, PlayerList[i].SoAddr) )
//        {
//            PlayerList[i].Ping = ping;
//            break;
//        }
//    }
//}
//$END_REMOVAL (tentative!!)




/* $REMOVED_UNREACHABLE
//-----------------------------------------------------------------------------
// Name: EnterSessionName
// Desc: Displays UI for entering/modifying a session name.
//-----------------------------------------------------------------------------
  //$TODO: We need XOnline UI specs before we can implement this fully...
void EnterSessionName(void)
{
    // buffer flip / clear
    FlipBuffers();
    ClearBuffers();

    // read input
    ReadJoystick();

//$MODIFIED (NOTE: this is a temporary hack, using orig codebase, until we get specs XOnline for how this should be implemented.)
//    if ((c = GetKeyPress()))
    strcpy( SessionName, "MySession" ); //$HACK: hard-coded session name
    MenuCount = strlen( SessionName );
    unsigned char c = 0;
    if( Keys[DIK_RETURN] && !LastKeys[DIK_RETURN] )  { c = 13; }
    else if( Keys[DIK_ESCAPE] && !LastKeys[DIK_ESCAPE] )  { c = 27; }
    if(c)
//$END_MODIFICATION
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

            CreateLocalServerPlayers();  //$HEY: this was a call to CreatePlayer in Aug99 drop (but cprince changed it earlier)

            bGameStarted = FALSE;
//$REMOVED_NOTUSED            PlayersRequestTime = 0.0f;
            SET_EVENT(HostWait);
        }
        // escape
        else if (c == 27)
        {
            MenuCount = 0;
            KillNetwork();
            SET_EVENT(MainMenu);
        }
        // normal key
        else if (MenuCount < MAX_SESSION_NAME - 2)
        {
            SessionName[MenuCount++] = c;
        }
    }

    // print name


    BeginTextState();

    DumpText(208, 224, 12, 24, 0x808000, "Enter Game Name:");
    SessionName[MenuCount] = '_';
    SessionName[MenuCount + 1] = 0;
    DumpText(128, 276, 12, 24, 0x808080, SessionName);
}
$END_REMOVAL */


/* $REMOVED_UNREACHABLE
//-----------------------------------------------------------------------------
// Name: BrowseSessions
// Desc: Displays a list of sessions, and lets user select a session to join.
//-----------------------------------------------------------------------------
  //$TODO: We need XOnline UI specs before we can implement this fully...
void BrowseSessions(void)
{
    // buffer flip / clear
    FlipBuffers();
    ClearBuffers();

    // read input / timers
    UpdateTimeStep();
    ReadJoystick();

    // dump back piccy

    // request sessions?
    SessionRequestTime -= TimeStep;
    if (SessionRequestTime < 0.0f)
    {
        RequestSessionList();
        SessionRequestTime = 1.0f;  //$REVISIT: what's the right delay to use here?

        if (SessionCount)
        {
            if (MenuCount > SessionCount - 1)
            {
                MenuCount = SessionCount - 1;
            }
        }
    }

    // display sessions
    BeginTextState();

    DumpText(264, 16, 8, 16, 0x808000, "Choose a Game:");

    GetRemoteMessages(); //$ADDITION (cprince; they didn't need this b/c was handled via DPlay/EnumSessionsCallback)

    short i;
    for (i = 0 ; i < SessionCount ; i++)
    {
        long color;

        if (MenuCount == i) color = 0xff0000;
        else color = 0x808080;
        DumpText(168, i * 16 + 48, 8, 16, color, SessionList[i].name);
        //$CMP_NOTE: might want to dump more detailed info here (eg, Acclaim's code output string saying if session was open or closed/started)
    }

//$CMP_NOTE:  Not currently supported. Right now, we don't get PlayerList/PlayerCount until we join a session.
//    // list players in selected session
//    if (!SessionCount) PlayerCount = 0;
//
//    if (PlayerCount)
//    {
//        DumpText(288, 192, 8, 16, 0x808000, "Players:");
//        for (i = 0 ; i < PlayerCount ; i++)
//        {
//            DumpText(168, i * 16 + 224, 8, 16, 0xff0000, PlayerList[i].Name);
//            HEY -- In Acclaim Aug99 code, they also name of each player's car here!
//        }
//    }

    // up / down
    if (Keys[DIK_UP] && !LastKeys[DIK_UP] && MenuCount) MenuCount--;
    if (Keys[DIK_DOWN] && !LastKeys[DIK_DOWN]) MenuCount++;
    if (SessionCount && MenuCount >= SessionCount) MenuCount = SessionCount - 1;

    // quit
    if (Keys[DIK_ESCAPE] && !LastKeys[DIK_ESCAPE])
    {
        MenuCount = 0;
        KillNetwork();
        SET_EVENT(MainMenu);
    }

    // join
    if( Keys[DIK_RETURN] && !LastKeys[DIK_RETURN] && SessionCount )
    {
        PlayerCount = 0;
        if( JoinSession(MenuCount) )
        {
            LocalPlayerID = INVALID_PLAYER_ID; // to know when server acknowledges our Join request
            RequestAddPlayers( dwLocalPlayerCount );  //$CMP_NOTE: should we do this elsewhere?
            bGameStarted = FALSE;
//$REMOVED_NOTUSED            PlayersRequestTime = 0.0f;
            SET_EVENT(ClientWait);
        }
        else
        {
            //$BUGBUG: this "join failed" message won't appear until next flip, and even then will only appear briefly !!
            //$TODO: print message like "press A to continue", and loop -- reading input -- until that happens
            /// Maybe should implement this via another menu function !!
            FlipBuffers();
            ClearBuffers();
            DumpText(288, 192, 8, 16, 0x808000, "Join request FAILED!");
        }
    }
}
$END_REMOVAL */



//$REMOVED (tentative!!) -- only currently used for spectator players, which we don't support
/// But might have some useful code if we want to support late-join later (eg, via another REQUEST_ADDPLAYERS message)
//
///////////////////////////
//// client joining late //
///////////////////////////
//
//void ClientJoinLate(void)
//{
//    long i;
//    unsigned long jointime, time, ping;
//
//// save join time
//
//    jointime = CurrentTimer();
//
//// wait for join info
//
//    while (!bGameStarted)
//    {
//        GetRemoteMessages();
//
//        if (TIME2MS(CurrentTimer() - jointime) > 10000)
//        {
//            DumpMessage(NULL,"No response from host!");
//
//            DP->Close();
//            MenuCount = 0;
//            GameSettings.MultiType = MULTITYPE_CLIENT;
//            SET_EVENT(BrowseSessions); //$MODIFIED: was LookForSessions
//            return;
//        }
//    }
//
//// calc proper race start time
//
//    time = CurrentTimer();
//    ping = time - jointime;
//    TotalRaceStartTime = time - MS2TIME(TotalRaceTime) - (ping / 2);
//    TotalRaceTime = TotalRacePhysicsTime = TIME2MS(time - TotalRaceStartTime);
//
//// setup start data 
//
////$REMOVED - server maintains player list and updates clients
////    ListPlayers(NULL);
////$END_REMOVAL
//    StartData.PlayerNum = 0;
//
//    for (i = 0 ; i < PlayerCount ; i++)
//    {
//        if (PlayerList[i].PlayerID == LocalPlayerID) {
//            AddPlayerToStartData(PLAYER_LOCAL, PlayerCount - 1, GetCarTypeFromName(PlayerList[i].Data.CarName), PlayerList[i].Spectator, TotalRacePhysicsTime, CTRL_TYPE_LOCAL, PlayerList[i].PlayerID, PlayerList[i].Name);
//            StartData.LocalPlayerNum = i;
//        } else {
//            AddPlayerToStartData(PLAYER_REMOTE, PlayerCount - 1, GetCarTypeFromName(PlayerList[i].Data.CarName), PlayerList[i].Spectator, TotalRacePhysicsTime, CTRL_TYPE_REMOTE, PlayerList[i].PlayerID, PlayerList[i].Name);
//        }
//    }
//}
//$END_REMOVAL


void LookForClientConnections(void)
{
    assert( IsServer() );
    assert( !bGameStarted );

    // Handle all new connections that have been requested
    SOCKADDR_IN SoAddr;
    int    cbAddr = sizeof(SoAddr);
    SOCKET socket = accept( soListenTCP, (sockaddr*)&SoAddr, &cbAddr );
    while( socket != INVALID_SOCKET )
    {
        // If we're still accepting connections, add this one to our list.
        // Else close the connection; don't leave the client hanging.
        if( MachineCount < MAX_NUM_MACHINES )
        {
            // store connection info
            NET_MACHINE* pNewMachine = &( MachineList[MachineCount] );

            pNewMachine->SoAddr = SoAddr;
            pNewMachine->SoAddr.sin_port = htons(GAME_PORT);  // need to set; else it's whatever random port the client used for outbound connection!!
            XNetInAddrToXnAddr( pNewMachine->SoAddr.sin_addr,
                                &(pNewMachine->XnAddr),
                                &(SessionCurr.keyID) );
            pNewMachine->Socket = socket;
            pNewMachine->RecvMsgCurrBytes  = 0;  // Must reset these values when .Socket is init'ed (TCP only),
            pNewMachine->RecvMsgTotalBytes = 0;  /// b/c could contain garbage if old connection was terminated
            MachineCount++;
        }
        else
        {
            // close the socket
            shutdown( socket, SD_BOTH );
            closesocket( socket );
        }

        // Any more pending connections?
        cbAddr = sizeof(SoAddr);
        socket = accept( soListenTCP, (sockaddr*)&SoAddr, &cbAddr );
    }
}

/* $REMOVED_UNREACHABLE
//-----------------------------------------------------------------------------
// Name: HostWait
// Desc: Displays list of players who have joined session, and lets this
//  host/server start the game when ready.
//-----------------------------------------------------------------------------
  //$TODO: We need XOnline UI specs before we can implement this fully...
void HostWait(void)
{
    long i;
    long gridused[MAX_RACE_CARS];

    MESSAGE_HEADER* pHeaderSend = (MESSAGE_HEADER*)SendMsgBuffer;
    BYTE* pbDataSend = (BYTE*)(pHeaderSend + 1);

    // buffer flip / clear
    FlipBuffers();
    ClearBuffers();

    // read input / timers
    UpdateTimeStep();
    ReadJoystick();



    BeginTextState();


    LookForClientConnections();  //$CMP


    // display current players

    DumpText(288, 64, 8, 16, 0x808000, "Players:");
    DumpText(216, 400, 8, 16, 0x808000, "Hit Enter To Start Game...");

//$REMOVED_NOTUSED - ACCLAIM AUG99 CHANGED RequestPings TO ListPlayers/GetPlayerList (WHICH WE REMOVED; NOT NEEDED)
//    PlayersRequestTime -= TimeStep;
//    if (PlayersRequestTime < 0.0f)
//    {
//        GetPlayerList();
//        PlayersRequestTime = 2.0f;
//    }
//$END_REMOVAL

    GetRemoteMessages();  // we'll respond to session-discovery messages here

    for (i = 0 ; i < PlayerCount ; i++)
    {
        char buf[128];
        DumpText(192, i * 16 + 96, 8, 16, 0xff0000, PlayerList[i].Name);
        sprintf(buf, "%ld", PlayerList[i].Ping);  //$TODO: ACCLAIM AUG99 CHANGED THIS TO PRINT CAR NAME, _INSTEAD_OF_ PING
        DumpText(448, i * 16 + 96, 8, 16, 0xff0000, buf);
    }

    // quit?
    if (Keys[DIK_ESCAPE] && !LastKeys[DIK_ESCAPE])
    {
        DestroySession();
        SET_EVENT(EnterSessionName);
    }

    // start game?
    if( Keys[DIK_RETURN] && !LastKeys[DIK_RETURN] )
    {
        bGameStarted = TRUE;
//$REVISIT: do we need to implement similar functionality ?!
//        // yep, set session to 'started'
//        SessionCurr.dwUser1 = TRUE;  //$NOTE: bStarted flag
//        DP->SetSessionDesc(&SessionCurr, NULL);

        // setup start data
        StartData.PlayerNum = 0;  //$HEY: why do they set this, instead of setting to 'PlayerCount' like before?
        LEVELINFO* levelInfo = GetLevelInfo(GameSettings.Level);
        strncpy(StartData.LevelDir, levelInfo->Dir, MAX_LEVEL_DIR_NAME);

        for (i = 0 ; i < PlayerCount ; i++)
        {
            gridused[i] = FALSE;
        }

        for (i = 0 ; i < PlayerCount ; i++)
        {
            // randomly distribute the N players among the first N "grid" slots.
            // (First player gets random slot 0..N, 2nd player gets random slot 0..N-1 <skipping used slots>, etc)
            long j, k;
            k = (rand() % (PlayerCount - i)) + 1;  // generates rand number between (1) and (NumPlayersLeftToBeProcessed), inclusive.
            for (j = 0 ; j < PlayerCount ; j++)    // selects the 'k'th untaken RaceStartIndex
            {
                if (!gridused[j])
                    k--;

                if (!k)
                {
                    break;
                }
            }

            gridused[j] = TRUE;
            if (PlayerList[i].PlayerID == LocalPlayerID) {
//$MODIFIED
//                AddPlayerToStartData(PLAYER_LOCAL, j, GetCarTypeFromName(PlayerList[i].Data.CarName), PlayerList[i].Spectator, TotalRacePhysicsTime, CTRL_TYPE_LOCAL, PlayerList[i].PlayerID, PlayerList[i].Name);
                AddPlayerToStartData(PLAYER_LOCAL, j, PlayerList[i].CarType, PlayerList[i].Spectator, TotalRacePhysicsTime, CTRL_TYPE_LOCAL, PlayerList[i].PlayerID, PlayerList[i].Name);
//$END_MODIFICATIONS
                StartData.LocalPlayerNum = i;
            } else {
//$MODIFIED
//                AddPlayerToStartData(PLAYER_REMOTE, j, GetCarTypeFromName(PlayerList[i].Data.CarName), PlayerList[i].Spectator, TotalRacePhysicsTime, CTRL_TYPE_REMOTE, PlayerList[i].PlayerID, PlayerList[i].Name);
                AddPlayerToStartData(PLAYER_REMOTE, j, PlayerList[i].CarType, PlayerList[i].Spectator, TotalRacePhysicsTime, CTRL_TYPE_REMOTE, PlayerList[i].PlayerID, PlayerList[i].Name);
//$END_MODIFICATIONS
            }
            //AddPlayerToStartData(PLAYER_REMOTE, j, PlayerList[i].Data.CarID, PlayerList[i].Spectator, 0, CTRL_TYPE_REMOTE, PlayerList[i].PlayerID, PlayerList[i].Name);
        }

        SendGameStarted();
        RemoteSyncHost();
        SET_EVENT(SetupGame);
    }
}
$END_REMOVAL */

/* $REMOVED_UNREACHABLE
//-----------------------------------------------------------------------------
// Name: ClientWait
// Desc: Displays list of players who have joined session, and lets this
//  client exit the session if desired.
//-----------------------------------------------------------------------------
  //$TODO: We need XOnline UI specs before we can implement this fully...
void ClientWait(void)
{
    short i;

    // buffer flip / clear
    FlipBuffers();
    ClearBuffers();

    // read input / timers
    UpdateTimeStep();
    ReadJoystick();



    BeginTextState();

    // display current players
    //$REVISIT: should we wait for join-accept before showing this stuff ??

    DumpText(288, 64, 8, 16, 0x808000, "Players:");
    DumpText(240, 400, 8, 16, 0x808000, "Waiting For Host...");

//$REMOVED_NOTUSED - ACCLAIM AUG99 CHANGED RequestPings TO ListPlayers/GetPlayerList (WHICH WE REMOVED; NOT NEEDED)
//    PlayersRequestTime -= TimeStep;
//    if (PlayersRequestTime < 0.0f)
//    {
//        GetPlayerList();
//        PlayersRequestTime = 2.0f;
//    }
//$END_REMOVAL

    GetRemoteMessages();

    for (i = 0 ; i < PlayerCount ; i++)
    {
        char buf[128];
        DumpText(192, i * 16 + 96, 8, 16, 0xff0000, PlayerList[i].Name);
        sprintf(buf, "%ld", PlayerList[i].Ping);  //$TODO -- ACCLAIM AUG99 CHANGED THIS TO PRINT CAR NAME, _INSTEAD_OF_ PING
        DumpText(448, i * 16 + 96, 8, 16, 0xff0000, buf);
    }

    // quit?
    if ((Keys[DIK_ESCAPE] && !LastKeys[DIK_ESCAPE]) || IsServer())
    {
        LeaveSession();
        MenuCount = 0;
        GameSettings.MultiType = MULTITYPE_CLIENT;
        SET_EVENT(BrowseSessions);
    }

    // host started?
    if (bGameStarted)
    {
        RemoteSyncClient();
        SET_EVENT(SetupGame);
    }
}
$END_REMOVAL */



/* $REMOVED
//////////////////////
// get host details //
//////////////////////

char GetHostDetails(void)
{
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
}
$END_REMOVAL */

/* $REMOVED
////////////////////////
// check for legal IP //
////////////////////////

char CheckLegalIP(void)
{
    long i;
    LEGAL_IP *lip;

// init play

    if (!InitNetwork())
        return FALSE;

// connect to IPX

    for (i = 0 ; i < ConnectionCount ; i++)
    {
        if (Connection[i].Guid.Data1 == 0x685BC400 && Connection[i].Guid.Data2 == 0x9d2c && Connection[i].Guid.Data3 == 0x11cf && *(_int64*)Connection[i].Guid.Data4 == 0xe3866800aa00cda9)
            break;
    }

    if (i == ConnectionCount)
    {
        KillNetwork();
        return FALSE;
    }

    if (!InitConnection((char)i))
    {
        KillNetwork();
        return FALSE;
    }

// get host details

    if (!GetHostDetails())
    {
        KillNetwork();
        return FALSE;
    }

// kill play

    KillNetwork();

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

/* $REMOVED
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
$END_REMOVAL */


//$TODO(cprince): revisit whether we really need the RemoteSync** functions.

///////////////////////
// sync with clients //
///////////////////////

void RemoteSyncHost(void)  //$NOTE(cprince): better name might be RemoteSyncWithClients
{
    assert( IsServer() );  //$ADDITION(cprince)

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

void RemoteSyncClient(void)  //$NOTE(cprince): better name might be RemoteSyncWithHost/Server
{
    assert( IsClient() );  //$ADDITION(cprince)

// start synchronize

    RemoteSyncTimer = 5.0f;
    RemoteSyncBestPing = 0xffffffff;

    NextPacketTimer = 0.0f;
//$REMOVED    PacketsPerSecond = 3.0f; //$NOTE: we'll use same value as server (gets set in global var, so don't set here)

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


//$HEY -- ACCLAIM AUG99 DROP HAD THIS FUNC BODY COMMENTED OUT (SEE IF THERE'S ANYTHING WE NEED TO KEEP; RECALL THAT SOME STUFF CHANGED DURING MIGRATION FROM DPLAY TO WINSOCK)

////////////////////////////
// process player joining //
////////////////////////////

void ProcessPlayerJoining(void)
{
/*  DPMSG_CREATEPLAYERORGROUP *data;
    DP_PLAYER_DATA *playerdata;
    PLAYER_START_DATA player;
    
// ignore if game not started

    if (!bGameStarted)
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
#ifndef XBOX_NOT_YET_IMPLEMENTED
    DPMSG_DESTROYPLAYERORGROUP *data;
    PLAYER *player;
    long i;

// game started?

    if (!bGameStarted)
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
#endif // !XBOX_NOT_YET_IMPLEMENTED
}

#ifndef XBOX_NOT_YET_IMPLEMENTED // we don't support host migration yet
////////////////////////////
// process player leaving //
////////////////////////////
void ProcessBecomeHost(void)
{

// set me to server if in game

    if (bGameStarted)
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
#endif // ! XBOX_NOT_YET_IMPLEMENTED

/* $REMOVED_UNREACHABLE (tentative!! do we want a receive thread?)
//////////////////////////////
// message receiving thread //
//////////////////////////////

unsigned long WINAPI ReceiveThread(void *param)
{
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

            size = PACKET_BUFFER_SIZE;  //$MODIFIED: was originally set to DP_BUFFER_MAX
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
    return 0;
}
$END_REMOVAL_UNREACHABLE (tentative!!) */


/* $REMOVED - people who were calling RefreshSessions should instead call RequestSessionList
//////////////////////////
// refresh session list //
//////////////////////////

void RefreshSessions(void)
{
//$REMOVED
//    long i;
//    GUID guid;
//
//    if (SessionCount)
//    {
//        guid = SessionList[SessionPick].Guid;
//    }
//    else
//    {
//        ZeroMemory(&guid, sizeof(GUID));
//    }
//$END_REMOVAL

//$MODIFIED
//    SessionPick = 0;
//    RequestSessionList();  //$MODIFIED: was ListSessions()
//
//    if (SessionCount)
//    {
//        for (i = 0 ; i < SessionCount ; i++)
//        {
//            if (SessionList[i].Guid == guid)
//                SessionPick = (char)i;
//        }
////$REMOVED - server maintains player list and updates clients
////        ListPlayers(&SessionList[SessionPick].Guid);
////$END_REMOVAL
//    }
//    else
//    {
//        PlayerCount = 0;
//    }

    RequestSessionList();  //$MODIFIED: was ListSessions()
    //$TODO: maintain correct SessionPick value when session order/list changes

//$END_MODIFICATIONS
}

$END_REMOVAL */




//$REMOVED - NOT USING THE FIELDS FROM "DP_SESSION Session"
//HEY - AUG99 ADDITION  (fills-in 'SessionCurr' var, and calls DP::SetSessionDesc
//NOTE: ONLY PLACE WHERE RESULTS OF DPlay::SetSessionDesc GETS USED IS IN MENUDRAW FUNCS (GetSessionDesc in DrawWaitingRoom)
//
////////////////////////
//// 
////////////////////////
//bool SetSessionDesc(char *name, char *track, long started, long gametype, long randomcars, long randomtrack)
//{
//    char buf[256];
//    HRESULT r;
//
//// set name
//
//    sprintf(buf, "%s\n%s", name, track);
//    SessionCurr.lpszSessionNameA = buf;
//    SessionCurr.dwUser1 = started;
//    SessionCurr.dwUser2 = gametype;
//    SessionCurr.dwUser3 = MultiplayerVersion;
//    SessionCurr.dwUser4 = 0;
//
//    if (randomcars) SessionCurr.dwUser4 |= 1;
//    if (randomtrack) SessionCurr.dwUser4 |= 2;
//
//// set
//
//    r = DP->SetSessionDesc(&SessionCurr, NULL);
//    if (r != DP_OK)
//    {
//        ErrorDX(r, "Can't set session description");
//    }
//
//// return OK
//
//    return TRUE;
//}
//$END_REMOVAL



