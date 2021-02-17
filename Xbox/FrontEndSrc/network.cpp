/*
MAYBE_KEEPERS:

* Session: dwMaxPlayers ?? (== DEFAULT_RACE_CARS)

* LocalPlayerData: Ready flag -- do we want it?  (If so, need to update it.)
  --> The 'Ready' flag only gets used in a couple places (SendGameLoaded, ProcessGameLoaded, CheckAllPlayersReady)
  --> I'm not even sure we need that flag; maybe assume that load will take same amount of time on all machines?  (And sync messages account for time diffs, right?)
  --> But if do use it, seems like might want per-machine, NOT per-player (b/c only used to tell when game is loaded, right?)

NON-KEEPER NOTES:

* Session: dwUser1 == bStarted

$HEY: GoFront() isn't reachable, and as a result, neither are a bunch of other things, like MainMenu(), ConnectionMenu(), etc, etc!!  (basically most of the old UI system).
      Most of that stuff is easy to remove, but let's wait on net-related funcs b/c we might need to refer back to how we did net setup there...

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
/// when we receive a MESSAGE_CAR_DATA.  Other than that, the original
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
#include "menudat2.h"
#include "panel.h"
#include "pickup.h"
#include "MainMenu.h"
#include "GameLoop.h"


#include "VoiceCommunicator.h"
#include <winsockx.h>
#include <assert.h>


// globals

#define GAME_PORT 15000  // could be any port

//$REVISIT(cprince): should all these buffers be allocated at runtime, instead of statically?
/// (That's what Acclaim did in Aug99 drop of Re-Volt.  But if we need to support worst-case conditions, maybe it doesn't matter, and static allocation reduces chance of mem fragmentation.)

char  SendMsgBuffer[PACKET_BUFFER_SIZE];
char* RecvMsgBuffer;  // pointer to actual buffer, b/c need different receive buffers for each TCP connection, and one for UDP
char  RecvMsgBufferUDP[PACKET_BUFFER_SIZE];
MESSAGE_HEADER* SendHeader = (MESSAGE_HEADER*)&SendMsgBuffer[0]; //$REVISIT: need? (It's only for convenience, esp porting Acclaim code)
MESSAGE_HEADER* ReceiveHeader = NULL;       //$REVISIT: need? (It's only for convenience, esp porting Acclaim code) <<Will get set whenever we set RecvMsgBuffer>>
int   RecvMsgSize;  // $ADDED (JHarding):  Looks like all existing packets are fixed size, but voice is not.

static NET_MACHINE MachineList[MAX_NUM_MACHINES];  // NOTE: [0] is always the server machine
static long        MachineCount;
static NET_MACHINE* const pMachineServer = &MachineList[0]; // for readability of client code (since clients only use MachineList[0], but server may use all entries)

NET_PLAYER PlayerList[MAX_RACE_CARS];  //$TODO(cprince) better name for this #define  //$NOTE: this is modified; was MAX_NUM_PLAYERS before!  (and all code below has been modified as well)
                                       //$TODO(cprince) This var used to be static, but ugly Acclaim Aug99 code broke that.  Would be nice to fix this...
long       PlayerCount;  //$TODO(cprince) This var used to be static, but ugly Acclaim Aug99 code broke that.  Would be nice to fix this...

NET_SESSION SessionList[MAX_NUM_SESSIONS];
                                       //$TODO(cprince) This var used to be static, but ugly Acclaim Aug99 code broke that.  Would be nice to fix this...
/*static*/ char    SessionCount;  //$CMP_NOTE: (might want to un-expose this) removal of ConnectionMenu means we had to access this from MainMenu, in another file

static NET_SESSION Session;
static char SessionName[MAX_SESSION_NAME];

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

char  MessageQueue[PACKET_BUFFER_SIZE];  //$REVISIT: should we rename this?  should we dynamically allocate (same issue as with SendMsgBuffer)
long MessageQueueSize;
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
DWORD FromID, ServerID;  //$NOTE: type was DPID originally  //$REVISIT: using FromID and ServerID is kind of ugly (leftover from porting Acclaim DPlay code); I'd like to remove them eventually.
//$REMOVEDDPID ToID;
//$REMOVED DPID GroupID;  // was always set to DPID_ALLPLAYERS when not using DPlay groups (which we're not)
//$BUG: not setting GroupID=DPID_ALLPLAYERS (which is what they use if no DPlay groups)







// macros to extract/check the IN_ADDR part of a SOCKADDR_IN
#define SAME_INADDR(x,y)      ( (x).sin_addr.S_un.S_addr == (y).sin_addr.S_un.S_addr )

/*static*/ SOCKET      soUDP = INVALID_SOCKET;  //$CMP_NOTE: (might want to un-expose this) only used in SendLocalCarData()
static     SOCKET      soListenTCP = INVALID_SOCKET;  // only used by server

SOCKADDR_IN  SoAddrFrom;
SOCKET       SocketFrom_TCP;  // only valid when processing TCP data; used for sending replies
XNADDR       XnAddrLocal;

const XNKID ZeroXNKID = { {0,0,0,0,0,0,0,0} };
const SOCKADDR_IN SoAddrBroadcast =
{
    AF_INET,
    GAME_PORT,
    { ((INADDR_BROADCAST & 0x000000FF) >>  0) ,
      ((INADDR_BROADCAST & 0x0000FF00) >>  8) ,
      ((INADDR_BROADCAST & 0x00FF0000) >> 16) ,
      ((INADDR_BROADCAST & 0xFF000000) >> 24)
    },
    {0,0,0,0,0,0,0,0}
};

DWORD dwLocalPlayerCount = 1;  //$BUGBUG: this should be set elsewhere, and using "1" wrongly assumes only a single local player !!

DWORD        LocalPlayerID = INVALID_PLAYER_ID;  //$BUGBUG: this assumes only a single local player !!


BOOL bGameStarted;  //$HEY: this var used to be static (before Aug99 merge); I wonder if we can go back to that...


//$REMOVED float PlayersRequestTime;
/*static*/ float SessionRequestTime; //$CMP_NOTE: (might want to un-expose this) removal of ConnectionMenu means we had to access this from MainMenu, in another file




//-----------------------------------------------------------------------------
// Name: InitNetwork
// Desc: Initializes network functionality.
//-----------------------------------------------------------------------------
bool InitNetwork(void)
{
    //$TODO: this function isn't so robust (but it's clean!) -- should do better job of checking return codes, avoiding multiple-init, avoiding partial-init, etc.

    int iRetval;

#if 0  // set to 1 to use non-secure xnet packets (but then startup takes longer; bug maybe?)
    XNetStartupParams xnsp;
    ZeroMemory( &xnsp, sizeof(xnsp) );
    xnsp.cfgSizeOfStruct = sizeof(xnsp);
    xnsp.cfgFlags |= XNET_STARTUP_BYPASS_SECURITY;
    xnsp.cfgFlags |= XNET_STARTUP_BYPASS_ENCRYPTION;
//$DEBUG_BEGIN (should probably remove this!?) (but it really decreases time req'd for XNetGetTitleXnAddr to succeed, unless that has been fixed)
xnsp.cfgFlags |= XNET_STARTUP_BYPASS_DHCP;
//$DEBUG_END
    iRetval = XNetStartup( &xnsp );
#else
    iRetval = XNetStartup( NULL );  // pass NULL for default parameters
#endif
    if( iRetval != 0 )
    {
        return FALSE;
    }

    WSADATA wsaData;
    iRetval = WSAStartup( MAKEWORD(2,2), &wsaData );
    if( iRetval != 0 )
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
    sa.sin_port        = GAME_PORT;
    bind( soUDP, (sockaddr*)&sa, sizeof(SOCKADDR_IN) );

    // put socket in non-blocking mode
    DWORD dwNonBlocking = 1;
    ioctlsocket( soUDP, FIONBIO, &dwNonBlocking );

    // enable broadcasting for socket
    int iBoolSockOpt = 1;
    setsockopt( soUDP, SOL_SOCKET, SO_BROADCAST, (char*)&iBoolSockOpt, sizeof(int) );

    //$BUGBUG: do we need htons() / ntohs() ??  It's used for some fields in SNL white paper and XDK snippets, but I thought I read something about it being removed (or at least unnecessary).
    /// If we need it, where exactly, and where not ??

    // Get the XNADDR of this machine
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
//    LEVELINFO *levinf = GetLevelInfo(g_TitleScreenData.iLevelNum); //$NOTE: ONLY NEEDED FOR THE CALL TO SetSessionDesc
//    SetSessionDesc(g_TitleScreenData.nameEnter[0], levinf->Dir, FALSE, GAMETYPE_MULTI, g_TitleScreenData.RandomCars, g_TitleScreenData.RandomTrack);  //$NOTE: don't need this call
//}
//$END_REMOVAL_AUG99

//$AUG99 ADDITION
// create player
//$MODIFIEDCreatePlayer(LobbyConnection->lpPlayerName->lpszShortNameA, LobbyConnection->dwFlags == DPLCONNECTION_CREATESESSION ? DP_SERVER_PLAYER : DP_CLIENT_PLAYER);
//$NOTE: not clear where lpPlayerName comes from...
//$BUG: assumes only 1 local player!
CreatePlayer("foobar_Joe", 0);
//$END_ADDITION_AUG99


    // return OK
    return TRUE;
}


//-----------------------------------------------------------------------------
// Name: KillNetwork
// Desc: Un-initializes network functionality.
//-----------------------------------------------------------------------------
void KillNetwork(void)
{
    // Close any open sockets
    if( soUDP != INVALID_SOCKET )
    {
        closesocket( soUDP );
        soUDP = INVALID_SOCKET;
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
bool CreateSession( char* strName )
{
//$HACK! (until we have UI to enter player name)
sprintf( RegistrySettings.PlayerName, "ServerPlayer%02d", rand() % 100 ); 
strncpy( g_TitleScreenData.nameEnter[0], RegistrySettings.PlayerName, MAX_INPUT_NAME );

    //$REVISIT: do we need to chech return values here?  (In other words, can this ever fail, and if it does, can we recover from it?)

    // Create/register session keys, and fill-in the Session structure
    XNetCreateKey( &(Session.keyID), &(Session.key) );
    XNetRegisterKey( &(Session.keyID), &(Session.key) );
    Session.XnAddrServer = XnAddrLocal;
    strcpy( Session.name, strName );  //$TODO: make sure we protect against buffer overruns somehow (if not here, then maybe as arg constraint, etc)

    // Add ourselves to the global MachineList
    MachineList[0].Socket = INVALID_SOCKET;
    MachineList[0].XnAddr = XnAddrLocal;
    MachineList[0].SoAddr.sin_family = AF_INET;
    MachineList[0].SoAddr.sin_port   = GAME_PORT;
    XNetXnAddrToInAddr( &(XnAddrLocal),
                        &(Session.keyID),
                        &(MachineList[0].SoAddr.sin_addr) );
    MachineCount = 1;

    // Create TCP socket to listen for new connections from clients
    soListenTCP = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );

    SOCKADDR_IN sa;
    sa.sin_family      = AF_INET;
    sa.sin_addr.s_addr = INADDR_ANY;
    sa.sin_port        = GAME_PORT;
    bind( soListenTCP, (sockaddr*)&sa, sizeof(SOCKADDR_IN) );

    listen( soListenTCP, SOMAXCONN );

    // put socket in non-blocking mode
    DWORD dwNonBlocking = 1;
    ioctlsocket( soListenTCP, FIONBIO, &dwNonBlocking );

//$REMOVED
//HEY -- ACCLAIM AUG99 ADDITION
//* if set Session.dwMaxPlayers, set it to g_TitleScreenData.numberOfCars here
//$END_REMOVAL

    return TRUE;
}


//-----------------------------------------------------------------------------
// Name: DestroySession
// Desc: Destroys the session that was previously created.
//-----------------------------------------------------------------------------
void DestroySession(void)
{
    assert( IsServer() );  //$CMP_NOTE: this isn't really necessary...


    // Close the socket that was listening for new connections from clients
    assert( INVALID_SOCKET != soListenTCP );
    closesocket( soListenTCP );
    soListenTCP = INVALID_SOCKET;

    // Close any sockets connected to client machines
    for( int i = 1 ; i < MachineCount ; i++ )  // start at [1] b/c [0] is local machine (ie, the server)
    {
        //$BUGBUG: do we need to check Socket is valid before trying to close it (in case player left earlier) ??
        closesocket( MachineList[i].Socket );
        MachineList[i].Socket = INVALID_SOCKET;  //$CMP_NOTE: probably not necessary...
    }
    MachineCount = 0;

    // Unregister the session key, and clear the Session structure
    XNetUnregisterKey( &(Session.keyID) );
    ZeroMemory( &Session, sizeof(Session) );
}


//-----------------------------------------------------------------------------
// Name: JoinSession
// Desc: Joins an existing sessions (which has been enumerated).
//-----------------------------------------------------------------------------
BOOL JoinSession(int iSession)
{
//$HACK! (until we have UI to enter player name)
sprintf( RegistrySettings.PlayerName, "ClientPlayer%02d", rand() % 100 );
strncpy( g_TitleScreenData.nameEnter[0], RegistrySettings.PlayerName, MAX_INPUT_NAME );

    // Register the session key information
    Session = SessionList[iSession];
    XNetRegisterKey( &(Session.keyID), &(Session.key) );
    //$BUGBUG(API): we shouldn't have to do XNetRegisterKey before XNetXnAddrToInAddr (where key data is explicit args!!)

    // Compute server's address info, using its XNADDR/XNKID
    XNetXnAddrToInAddr( &(Session.XnAddrServer), &(Session.keyID),
                        &(pMachineServer->SoAddr.sin_addr) );
    pMachineServer->SoAddr.sin_family = AF_INET;
    pMachineServer->SoAddr.sin_port   = GAME_PORT;
    pMachineServer->XnAddr = Session.XnAddrServer;

    //$REVISIT: should we zero-out 'pMachineServer->SoAddr' before filling-in ?? (to avoid problems when struct used multiple times, sequentially)
    //$REVISIT: should we zero-out 'Session' if the join fails !?!  (Probably not necessary.)


    // Try to join the session by initiating a TCP connection with the server.
    // (Notice: no explicit binding; will implicitly bind during connect call.)
    // (Notice: socket is initially in blocking mode, for connection phase.)
    pMachineServer->Socket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
    
    int iRetVal = connect( pMachineServer->Socket,
                           (sockaddr*)&pMachineServer->SoAddr,
                           sizeof(SOCKADDR_IN) );
    //$BUGBUG: I was seeing WSAECONNREFUSED here after I had connected/disconnected a few times, but I haven't tried to track it down yet.  (Maybe there's a max-connections limit on Xbox?)
    if( 0 != iRetVal )
    {
        // connection failed, so join was rejected
        LeaveSession();  //$REVISIT: is this always safe to do here ??
        return FALSE;
    }

    // now that we're connected, put the socket into non-blocking mode
    DWORD dwNonBlocking = 1;
    ioctlsocket( pMachineServer->Socket, FIONBIO, &dwNonBlocking );

    assert( 0 == PlayerCount );

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
    closesocket( pMachineServer->Socket );
    pMachineServer->Socket = INVALID_SOCKET;

    // Unregister the session key, and clear the Session structure
    XNetUnregisterKey( &(Session.keyID) );
    ZeroMemory( &Session, sizeof(Session) );

    // Note: we aren't sending a "REMOVE_PLAYERS" message because server
    /// needs to handle non-graceful player removal anyway.
    //$TODO: implement server handling of non-graceful player removal! :)
}


//-----------------------------------------------------------------------------
// Name: RequestSessionList
// Desc: Requests a list of active sessions that this app can join.
//-----------------------------------------------------------------------------
void RequestSessionList(void)
{
    //$REVISIT: can we get here even if net init failed? If so, should check for valid init, and immed-return if didn't happen.  (That check was added in Acclaim Aug99 code.)

    // Broadcast a session-discovery message.  (Responses are handled elsewhere.)
    MESSAGE_HEADER* pHeaderSend = (MESSAGE_HEADER*)SendMsgBuffer;
    BYTE* pbDataSend = (BYTE*)(pHeaderSend + 1);

    pHeaderSend->Type = MESSAGE_FIND_SESSION;
    *((XNADDR*)pbDataSend) = XnAddrLocal; // Use client XNADDR as a nonce so we know
    pbDataSend += sizeof(XNADDR);         /// whether the broadcast reply is directed at us
            
    int cbSend = ((BYTE*)pbDataSend) - ((BYTE*)pHeaderSend);
    SendHeader->PlayerID = LocalPlayerID;
    sendto( soUDP, SendMsgBuffer, cbSend, 0,
            (sockaddr*)&SoAddrBroadcast, sizeof(SOCKADDR_IN) );
}


//$REVISIT(cprince): not the most elegant function name, and function mainly here so that we can call it from outside network.cpp (but assumes you only call it once)
//$TODO(cprince): how to we handle new *local* players joining the session, and making sure clients hear about it?  (Maybe can breakout functionality of REQUEST_ADDPLAYERS handler, and make it not specific to clients.)
void CreateLocalServerPlayers(void)
{
    // Add all local players to PlayerList
    PlayerCount = 0;
    for( DWORD i = 0 ; i < dwLocalPlayerCount ; i++ )
    {
        NET_PLAYER* pNewPlayer = &( PlayerList[PlayerCount] );

        //$BUGBUG: we're not handling multiple local players here yet!  (Still assuming only 1 local player.)

        pNewPlayer->CarType = GameSettings.CarType;
        memcpy( &(pNewPlayer->Name), RegistrySettings.PlayerName, sizeof(RegistrySettings.PlayerName) );
        //$TODO: should make sure (here or elsewhere) that string is guaranteed to be null-terminated!
        // (Protocol would be nice: how about do it wherever else a char buffer gets written to.)
        pNewPlayer->PlayerID = NextPlayerID();
        pNewPlayer->XnAddr = XnAddrLocal;
        pNewPlayer->SoAddr.sin_family = AF_INET;
        pNewPlayer->SoAddr.sin_port   = GAME_PORT;
        ZeroMemory( &(pNewPlayer->SoAddr.sin_addr), sizeof(IN_ADDR) );  //$CMP_NOTE: the IN_ADDR we set here should never get used (unless it's overwritten), so this probably isn't necessary?
        pNewPlayer->Ping = 0;  //$CMP_NOTE: probably don't need to do this here...

        LocalPlayerID = pNewPlayer->PlayerID;
        ServerID = LocalPlayerID;
        PlayerCount++;
    }
}


//-----------------------------------------------------------------------------
// Name: RequestAddPlayers
// Desc: Tells the server that we want to add player(s) to the session.
//-----------------------------------------------------------------------------
void RequestAddPlayers( DWORD dwPlayers )
{
    assert( IsClient() ); 

    MESSAGE_HEADER* pHeaderSend = (MESSAGE_HEADER*)SendMsgBuffer;
    BYTE* pbDataSend = (BYTE*)(pHeaderSend + 1);

    pHeaderSend->Type = MESSAGE_REQUEST_ADDPLAYERS;

    *((DWORD*)pbDataSend) = dwPlayers;
    pbDataSend += sizeof(DWORD);

    for( DWORD i = 0 ; i < dwPlayers ; i++ )
    {
        //$BUGBUG: we're not handling multiple local players here yet!  (Still assuming only 1 local player.)

        *((long*)pbDataSend) = GameSettings.CarType;
        pbDataSend += sizeof(long);

        memcpy( pbDataSend, RegistrySettings.PlayerName, sizeof(RegistrySettings.PlayerName) );
        pbDataSend += sizeof(RegistrySettings.PlayerName);
    }

    int cbSend = ((BYTE*)pbDataSend) - ((BYTE*)pHeaderSend);
    pHeaderSend->Contents = (short)cbSend; // specify size for all TCP messages
    SendHeader->PlayerID = LocalPlayerID;
    send( pMachineServer->Socket, SendMsgBuffer, cbSend, 0 );
}


//-----------------------------------------------------------------------------
// Name: NextPlayerID
// Desc: Returns a unique player ID.
//-----------------------------------------------------------------------------
DWORD NextPlayerID(void)
{
    //NOTE: function only used by server
    
    static DWORD NextPlayerID = INVALID_PLAYER_ID;
    NextPlayerID++;  // will break after 4 billion calls; oh well.
    return NextPlayerID;
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







//$REVISIT: figure out whether we really want to queue outgoing messages this way.
/// (Maybe only used for bundling frame's data, which is better than separate packet for each, but still has header-bloat issues.)

/////////////////////////////////////////////
// queue a messsage in local message queue //
/////////////////////////////////////////////
void QueueMessage(MESSAGE_HEADER *buff, short size)
{
    buff->PlayerID = LocalPlayerID; //$ADDITION

// init header if first message

    if (!MessageQueueSize)
    {
        ((MESSAGE_HEADER*)MessageQueue)->Type = MESSAGE_QUEUE;
        ((MESSAGE_HEADER*)MessageQueue)->Contents = 0;
        MessageQueueSize = sizeof(MESSAGE_HEADER);
    }

// skip if queue full

    if (MessageQueueSize + size + sizeof(long) > PACKET_BUFFER_SIZE)  //$MODIFIED: changed DP_BUFFER_MAX to PACKET_BUFFER_SIZE
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

    ((MESSAGE_HEADER*)MessageQueue)->Contents++;
}


//$NOTE: This is a new function; code came from TransmitRemoteObjectData in object.cpp, which is only place where they sent contents of message queue.
void TransmitMessageQueue(void)
{
//$MODIFIED 
//    SendMessage((MESSAGE_HEADER*)MessageQueue, (short)MessageQueueSize, GroupID);
    ((MESSAGE_HEADER*)MessageQueue)->PlayerID = LocalPlayerID;
    sendto( soUDP, MessageQueue, MessageQueueSize, 0,
            (sockaddr*)&SoAddrBroadcast, sizeof(SOCKADDR_IN) );
//$END_MODIFICATIONS
    MessageQueueSize = 0;
}


//$REMOVED -- directly calling 'sendto' instead
//////////////////////
//// send a message //
//////////////////////
//
//void SendMessage(MESSAGE_HEADER *buff, short size, DPID to)
//{
//    TotalDataSent += size;
//
//    HRESULT r;
//
//    r = DP->SendEx(LocalPlayerID, to, DPcaps.dwFlags & DPCAPS_ASYNCSUPPORTED ? DPSEND_ASYNC | DPSEND_NOSENDCOMPLETEMSG : 0, buff, size, 0, DPcaps.dwFlags & DPCAPS_SENDTIMEOUTSUPPORTED ? DP_SEND_TIMEOUT : 0, NULL, NULL);
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
//void SendMessageGuaranteed(MESSAGE_HEADER *buff, short size, DPID to)
//{
//    TotalDataSent += size;
//
////$MODIFIED
////    HRESULT r;
////
////    r = DP->SendEx(LocalPlayerID, to, DPSEND_GUARANTEED, buff, size, 0, 0, NULL, NULL);
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
//-----------------------------------------------------------------------------
BOOL GetSingleMessageTCP( SOCKET socket, char* pRecvBuffer,
                          short* pMsgCurrBytes, short* pMsgTotalBytes )
{
    assert( socket != INVALID_SOCKET );

    // If we don't have all of message header, try to get it
    if( *pMsgCurrBytes < sizeof(MESSAGE_HEADER) )
    {
        int cbHeaderRemaining = sizeof(MESSAGE_HEADER) - *pMsgCurrBytes;

        // try to fetch remaining bytes
        int iRetVal = recv( socket, &pRecvBuffer[*pMsgCurrBytes], cbHeaderRemaining, 0 );

        // failed?
        if( 0 == iRetVal  ||  SOCKET_ERROR == iRetVal )  { return FALSE; }
        //$BUGBUG: not sure about this error checking; check for robustness....
        /// (Note: for TCP, recv() returns 0 if connection has been closed gracefully; cannot send a TCP payload of zero bytes using send() )

        // update values
        *pMsgCurrBytes += (short)iRetVal;
        if( *pMsgCurrBytes < sizeof(MESSAGE_HEADER) ) {
            return FALSE;
        } else {
            *pMsgTotalBytes = ((MESSAGE_HEADER*)pRecvBuffer)->Contents;
        }
    }
    
    assert( *pMsgTotalBytes > 0 );  // if we reach this point, pMsgTotalBytes should be valid

    // If we don't have all of message body, try to get it
    if( *pMsgCurrBytes < *pMsgTotalBytes )
    {
        int cbTotalRemaining = *pMsgTotalBytes - *pMsgCurrBytes;

        // try to fetch remaining bytes
        int iRetVal = recv( socket, &pRecvBuffer[*pMsgCurrBytes], cbTotalRemaining, 0 );

        // failed?
        if( 0 == iRetVal  ||  SOCKET_ERROR == iRetVal )  { return FALSE; }
        //$BUGBUG: not sure about this error checking; check for robustness....
        /// (Note: for TCP, recv() returns 0 if connection has been closed gracefully; cannot send a TCP payload of zero bytes using send() )

        // update values
        *pMsgCurrBytes += (short)iRetVal;
    }
    
    // Do we have entire message now?
    if( *pMsgCurrBytes >= *pMsgTotalBytes )
    {
        assert( *pMsgCurrBytes == *pMsgTotalBytes ); // shouldn't be larger

//$DEBUG_BEGIN (remove when done!)
{
char szTemp[1024];
sprintf( szTemp, "Extracted TCP message (%d bytes)\n", *pMsgTotalBytes );
OutputDebugString( szTemp );
}
//$DEBUG_END
        *pMsgCurrBytes  = 0;
        *pMsgTotalBytes = 0;

        return TRUE;
    }
    //$BUGBUG: where should we check for closed TCP connections, and where/how should we handle them?

    return FALSE;
}


//-----------------------------------------------------------------------------
// Name: GetRemoteMessages
// Desc: Processes all messages in the incoming message queues (TCP and UDP).
//   Return value tells how many messages we processed.
//-----------------------------------------------------------------------------
int GetRemoteMessages(void)
{
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
            BOOL bSuccess = GetSingleMessageTCP( pMachineServer->Socket,
                                                 pMachineServer->RecvMsgBuffer,
                                                 &pMachineServer->RecvMsgCurrBytes,
                                                 &pMachineServer->RecvMsgTotalBytes );
            if( !bSuccess ) { break; }

            SoAddrFrom     = pMachineServer->SoAddr; // set for convenience elsewhere
            SocketFrom_TCP = pMachineServer->Socket; // set for convenience elsewhere ($CMP_NOTE: but not actually used by clients !?!)

            // process the message
            RecvMsgBuffer = pMachineServer->RecvMsgBuffer;
            ReceiveHeader = (MESSAGE_HEADER*)RecvMsgBuffer;
            RecvMsgSize   = pMachineServer->RecvMsgCurrBytes;
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
                BOOL bSuccess = GetSingleMessageTCP( soMachine,
                                                     MachineList[i].RecvMsgBuffer,
                                                     &MachineList[i].RecvMsgCurrBytes,
                                                     &MachineList[i].RecvMsgTotalBytes );
                if( !bSuccess ) { break; }

                // process the message
                RecvMsgBuffer = MachineList[i].RecvMsgBuffer;
                ReceiveHeader = (MESSAGE_HEADER*)RecvMsgBuffer;
                RecvMsgSize   = MachineList[i].RecvMsgCurrBytes;
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
        ReceiveHeader = (MESSAGE_HEADER*)RecvMsgBuffer;
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
    //$BUGBUG: not currently using ProcessSystemMessage
    /// (so either (A) move that functionality elsewhere, or (B) determine which function handles message based on Socket or Port it came in on.)

    MESSAGE_HEADER* pHeaderRecv = (MESSAGE_HEADER*)RecvMsgBuffer;
    BYTE* pbDataRecv = (BYTE*)(pHeaderRecv + 1);

    MESSAGE_HEADER* pHeaderSend = (MESSAGE_HEADER*)SendMsgBuffer;
    BYTE* pbDataSend = (BYTE*)(pHeaderSend + 1);
    int   cbSend;

    FromID = pHeaderRecv->PlayerID;

    // act on message
    switch( ReceiveHeader->Type )
    {

        //$ADDITION - handle message queue here (rather than at each place that calls ProcessMessage)
        case MESSAGE_QUEUE:
        {
            unsigned short nMsgs = ReceiveHeader->Contents;
            //$PERF: all this copying they do (effectively just to shift RecvMsgBuffer) is ugly.
            char buf[PACKET_BUFFER_SIZE];  // temp buffer
            memcpy( buf, ReceiveHeader + 1, RecvMsgSize - sizeof(MESSAGE_HEADER) );
            char* pPosn = &buf[0];
    
            while( nMsgs > 0 )
            {
                long size = *(long*)pPosn;
                pPosn += sizeof(long);
                memcpy(ReceiveHeader, pPosn, size);
                pPosn += size;
                ProcessMessage();
    
                nMsgs--;
            }
        }
        //$END_ADDITION


//$HEY: ACCLAIM AUG99 ADDITIONS IN THIS BLOCK...
    case MESSAGE_CAR_DATA:               ProcessCarData();  break;  // car data
    case MESSAGE_CAR_NEWCAR:             ProcessCarNewCar(); break; // new car
    case MESSAGE_CAR_NEWCAR_ALL:         ProcessCarNewCarAll(); break; // new car all
    case MESSAGE_WEAPON_DATA:            ProcessWeaponNew(); break; // weapon data
    case MESSAGE_OBJECT_DATA:            ProcessObjectData(); break; // object data
    case MESSAGE_TARGET_STATUS_DATA:     ProcessTargetStatus(); break; // object data
    case MESSAGE_GAME_STARTED:           ProcessGameStarted(); break; // game started
    case MESSAGE_SYNC_REQUEST:           ProcessSyncRequest(); break; // sync request
    case MESSAGE_SYNC_REPLY:             ProcessSyncReply(); break; // sync reply
    case MESSAGE_GAME_LOADED:            ProcessGameLoaded(); break; // game loaded
    case MESSAGE_COUNTDOWN_START:        ProcessCountdownStart(); break; // countdown started
//$REMOVED_UNREACHABLEcase MESSAGE_JOIN_INFO: ProcessJoinInfo(); break; // join info
    case MESSAGE_RACE_FINISH_TIME:       ProcessRaceFinishTime(); break; // race finish time
    case MESSAGE_PLAYER_SYNC_REQUEST:    ProcessPlayerSync1(); break; // player sync request
    case MESSAGE_PLAYER_SYNC_REPLY1:     ProcessPlayerSync2(); break; // player sync reply 1
    case MESSAGE_PLAYER_SYNC_REPLY2:     ProcessPlayerSync3(); break; // player sync reply 2
    case MESSAGE_RESTART:                ProcessMultiplayerRestart(); break; // game restarting
    case MESSAGE_POSITION:               ProcessPosition(); break; // player position
    case MESSAGE_TRANSFER_BOMB:          ProcessTransferBomb(); break; // transfer bomb
    case MESSAGE_CHAT:                   ProcessChatMessage(); break; // chat message
    case MESSAGE_BOMBTAG_CLOCK:          ProcessBombTagClock(); break; // bomb tag timer
    case MESSAGE_TRANSFER_FOX:           ProcessTransferFox(); break; // transfer fox
    case MESSAGE_ELECTROPULSE_THE_WORLD: ProcessElectroPulseTheWorld(); break; // electropulse the world
    case MESSAGE_GOT_GLOBAL:             ProcessGotGlobal(); break; // got global
    case MESSAGE_HONKA:                  ProcessHonka(); break; // honka



//$REMOVED (tentative)
//
//HEY -- THE *Ping* FUNCS HAVE BEEN REMOVED; I THINK THEY'VE BEEN DEPRECATED BY *Sync* FUNCS
//        // ping request
//        case MESSAGE_PING_REQUEST:
//            ProcessPingRequest();
//        break;
//
//HEY -- THE *Ping* FUNCS HAVE BEEN REMOVED; I THINK THEY'VE BEEN DEPRECATED BY *Sync* FUNCS
//        // ping return
//        case MESSAGE_PING_RETURN:
//            ProcessPingReturn();
//        break;
//$END_REMOVAL




    case MESSAGE_VOICE_PACKET:           ProcessVoiceMessage(); break;

    // Someone is looking for sessions, so respond if we're hosting one
    case MESSAGE_FIND_SESSION:
    {
        // make sure we're hosting, and that we've started a session
        if( IsClient() )  { break; }
        if( 0 == memcmp(&Session.keyID, &ZeroXNKID, sizeof(XNKID)) )  { break; }
        //$REVISIT: do we need to check that they're requesting our game (and not some other game that happens to use same port)?  Or does XNet automatically take care of that?

        // send back a message with info about this session
        pHeaderSend->Type = MESSAGE_SESSION_FOUND;

        memcpy( pbDataSend, pbDataRecv, sizeof(XNADDR) ); // include XNADDR of sender, so recipients know who message is intended for
        pbDataSend += sizeof(XNADDR);

        memcpy( pbDataSend, &Session, sizeof(Session) );
        pbDataSend += sizeof(Session);

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

        cbSend = ((BYTE*)pbDataSend) - ((BYTE*)pHeaderSend);
        SendHeader->PlayerID = LocalPlayerID;
        sendto( soUDP, SendMsgBuffer, cbSend, 0, (sockaddr*)&SoAddrBroadcast, sizeof(SOCKADDR_IN) );
    }
    break;

    // A server is telling someone about an existing session
    case MESSAGE_SESSION_FOUND:
    {
        // Ignore if we've reached max sessions
        if( SessionCount >= MAX_NUM_SESSIONS )  { break; }

        // Ignore if this message isn't intended for us
        if( 0 != memcmp(pbDataRecv, &XnAddrLocal, sizeof(XNADDR)) )  { break; }
        pbDataRecv += sizeof(XNADDR);

        // Make sure this session isn't already in our list
        bool bKnownSession = false;
        for( int i=0 ; i < SessionCount ; i++ )
        {
            if( 0 == memcmp(&SessionList[i], pbDataRecv, sizeof(NET_SESSION)) )
            {
                bKnownSession = true;
            }
        }
        if( bKnownSession )  { break; }

        // Add this session to our list
        memcpy( &SessionList[SessionCount], pbDataRecv, sizeof(NET_SESSION) );
        SessionCount++;
    }
    break;


    // Some player is sending us their info
    case MESSAGE_REQUEST_ADDPLAYERS:
    {
        assert( IsServer() );

        int i;
        DWORD dwNewPlayers = *((DWORD*)pbDataRecv);
        pbDataRecv += sizeof(DWORD);

        // Deny request if exceeds max players.
        if( (PlayerCount+dwNewPlayers) > MAX_RACE_CARS )
        {
            pHeaderSend->Type = MESSAGE_ADDPLAYERS_REJECTED;

            cbSend = ((BYTE*)pbDataSend) - ((BYTE*)pHeaderSend);
            pHeaderSend->Contents = (short)cbSend; // specify size for all TCP messages
            SendHeader->PlayerID = LocalPlayerID;
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
                pbDataRecv += sizeof(MAX_PLAYER_NAME);
                //$TODO: should make sure (here or elsewhere) that string is guaranteed to be null-terminated!
                // (Protocol would be nice: how about do it wherever else a char buffer gets written to.)

                pNewPlayer->PlayerID = NextPlayerID();
                pNewPlayer->SoAddr = SoAddrFrom;
                XNetInAddrToXnAddr( pNewPlayer->SoAddr.sin_addr,
                                    &(pNewPlayer->XnAddr),
                                    &(Session.keyID) );
                pNewPlayer->Ping = 0;  //$CMP_NOTE: probably don't need to do this here...

                PlayerCount++;
            }


            // send a "join accepted" reply
            // (contains assigned PlayerID for each accepted player)
            pbDataSend = (BYTE*)(pHeaderSend + 1);  // reset pointer
            pHeaderSend->Type = MESSAGE_ADDPLAYERS_ACCEPTED;

            *((DWORD*)pbDataSend) = dwNewPlayers;
            pbDataSend += sizeof(DWORD);

            for( i = dwNewPlayers ; i > 0 ; i-- )
            {
                *((DWORD*)pbDataSend) = PlayerList[PlayerCount-i].PlayerID;
                pbDataSend += sizeof(DWORD);
            }

            cbSend = ((BYTE*)pbDataSend) - ((BYTE*)pHeaderSend);
            pHeaderSend->Contents = (short)cbSend; // specify size for all TCP messages
            SendHeader->PlayerID = LocalPlayerID;
            send( SocketFrom_TCP, SendMsgBuffer, cbSend, 0 );


            // send the updated player list to every connected client machine
            pbDataSend = (BYTE*)(pHeaderSend + 1);  // reset pointer
            pHeaderSend->Type = MESSAGE_PLAYER_LIST;

            *((DWORD*)pbDataSend) = PlayerCount;
            pbDataSend += sizeof(DWORD);

            for( i = 0 ; i < PlayerCount ; i++ )
            {
                *((NET_PLAYER*)pbDataSend) = PlayerList[i];
                pbDataSend += sizeof(NET_PLAYER);
                // Note: recipients cannot use the SoAddr directly;
                /// they must get it from the XnAddr in this message.
            }

            cbSend = ((BYTE*)pbDataSend) - ((BYTE*)pHeaderSend);
            pHeaderSend->Contents = (short)cbSend; // specify size for all TCP messages
            SendHeader->PlayerID = LocalPlayerID;
            for( i = 1 ; i < MachineCount ; i++ )  // start at [1] b/c [0] is local machine (ie, the server)
            {
                SOCKET so = MachineList[i].Socket;
                send( so, SendMsgBuffer, cbSend, 0 );
            }
        }

    }
    break;


    // We're being told our requested #players CAN join the session
    case MESSAGE_ADDPLAYERS_ACCEPTED:
    {
        assert( IsClient() );

//$DEBUG_BEGIN (remove when done!)
OutputDebugString( "Got MESSAGE_ADDPLAYERS_ACCEPTED\n" );
//$DEBUG_END
        int i;
        DWORD dwNewPlayers = *((DWORD*)pbDataRecv);
        pbDataRecv += sizeof(DWORD);

        // Extract our assigned PlayerID(s) from the message
        for( i = dwNewPlayers ; i > 0 ; i-- )
        {
            //$BUGBUG: we're not handling multiple local players here yet!  (Still assuming only 1 local player.)
            //$BUGBUG: might need more work here to handle the case where some requests are accepted and others are rejected (so that PlayerIDs get assigned to correct requesters)

            LocalPlayerID = *((DWORD*)pbDataRecv);
            pbDataRecv += sizeof(DWORD);
        }
    }
    break;


    // We're being told our requested #players CANNOT join the session
    case MESSAGE_ADDPLAYERS_REJECTED:
    {
//$DEBUG_BEGIN (remove when done!)
OutputDebugString( "Got MESSAGE_ADDPLAYERS_REJECTED\n" );
//$DEBUG_END
        ErrorDX( 0, "Can't join session." );
    }
    break;


    // We're receiving the list of players for some session
    case MESSAGE_PLAYER_LIST:
    {
//$DEBUG_BEGIN (remove when done!)
OutputDebugString( "Got MESSAGE_PLAYER_LIST\n" );
//$DEBUG_END
        //$CMP_NOTE: should we ignore this message if join-accept not received yet ?? (Probably not required.)

        assert( IsClient() );
        assert( SAME_INADDR(SoAddrFrom,pMachineServer->SoAddr) );

        // Get the list of players
        PlayerCount = *((DWORD*)pbDataRecv);
        pbDataRecv += sizeof(DWORD);

        for( int i=0 ; i < PlayerCount ; i++ )
        {
            PlayerList[i] = *((NET_PLAYER*)pbDataRecv);
            pbDataRecv += sizeof(NET_PLAYER);

            // convert players' XNADDR to IN_ADDR
            // (other fields of the SOCKADDR_IN should already be valid)
            XNetXnAddrToInAddr( &(PlayerList[i].XnAddr), &(Session.keyID),
                                &(PlayerList[i].SoAddr.sin_addr) );
        }
    }
    break;


    }
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
//    DrawBigGameText(128, 112, 0x808000, "Select Connection:");
//
//    for (i = 0 ; i < ConnectionCount ; i++)
//    {
//        if (MenuCount == i) col = 0xff0000;
//        else col = 0x808080;
//        DrawBigGameText(128, i * 48 + 176, col, Connection[i].Name);
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
//            SessionCount = 0;
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

    SendHeader->Type = MESSAGE_GAME_STARTED;
    memcpy(SendHeader + 1, &StartData, sizeof(START_DATA));

//$MODIFIED
//    SendMessageGuaranteed(SendHeader, sizeof(MESSAGE_HEADER) + sizeof(START_DATA), GroupID);
//

    //$BUGBUG: really, we only want to send to machines that have connected players (and only send once per machine probably)

    // send message to each connected machine
    int cbSend = sizeof(MESSAGE_HEADER) + sizeof(START_DATA);
    SendHeader->Contents = (short)cbSend; // specify size for all TCP messages
    SendHeader->PlayerID = LocalPlayerID;
    for( int i = 1 ; i < MachineCount ; i++ )  // start at [1] b/c [0] is local machine (ie, the server)
    {
        SOCKET so = MachineList[i].Socket;
        send( so, SendMsgBuffer, cbSend, 0 ); // guaranteed send
    }
//$END_MODIFICATIONS
}

//////////////////////////
// process game started //
//////////////////////////

void ProcessGameStarted(void)
{

// set flags

    bGameStarted = TRUE;
    ServerID = FromID;

// save start data

    MultiStartData = *(START_DATA*)(ReceiveHeader + 1);
}

/////////////////////////
// send a sync request //
/////////////////////////

void SendSyncRequest(void)
{
    assert( IsClient() );  //$ADDITION

    unsigned long *ptr = (unsigned long*)(SendHeader + 1);

// send request with my time

    SendHeader->Type = MESSAGE_SYNC_REQUEST;
    ptr[0] = CurrentTimer();

//$MODIFIED:
//    SendMessage(SendHeader, sizeof(MESSAGE_HEADER) + sizeof(unsigned long), ServerID);

    int cbSend = sizeof(MESSAGE_HEADER) + sizeof(unsigned long);
    SendHeader->PlayerID = LocalPlayerID;
    sendto( soUDP, SendMsgBuffer, cbSend, 0,
            (sockaddr*)&(pMachineServer->SoAddr), sizeof(SOCKADDR_IN) );
//$END_MODIFICATIONS
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

//$MODIFIED
//    SendMessage(SendHeader, sizeof(MESSAGE_HEADER) + sizeof(unsigned long) * 2, FromID);

    int cbSend = sizeof(MESSAGE_HEADER) + 2*sizeof(unsigned long);
    SendHeader->PlayerID = LocalPlayerID;
    sendto( soUDP, SendMsgBuffer, cbSend, 0,
            (sockaddr*)&SoAddrFrom, sizeof(SOCKADDR_IN) );  // direct-send reply
//$END_MODIFICATIONS
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
//$MODIFIED
//// set timeout counter
//
//    AllReadyTimeout = 60.0f;
//
//// send loaded message
//
//    SendHeader->Type = MESSAGE_GAME_LOADED;
//    SendMessageGuaranteed(SendHeader, sizeof(MESSAGE_HEADER), GroupID);

    if( IsServer() )
    {
        AllReadyTimeout = 60.0f;

        PLR_LocalPlayer->Ready = TRUE; //$BUG: assumes only 1 local player
    }
    else
    {
        assert( IsClient() );

        SendHeader->Type = MESSAGE_GAME_LOADED;

        int cbSend = sizeof(MESSAGE_HEADER);
        SendHeader->Contents = (short)cbSend; // specify size for all TCP messages
        SendHeader->PlayerID = LocalPlayerID;
        send( pMachineServer->Socket, SendMsgBuffer, cbSend, 0 ); // guaranteed send, to server
    }
//$END_MODIFICATIONS
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
    assert( IsServer() );  //$ADDITION

    unsigned long time, *ptr = (unsigned long*)(SendHeader + 1);

// send start time + timer freq

    SendHeader->Type = MESSAGE_COUNTDOWN_START;

    time = CurrentTimer();
    ptr[0] = time;
    ptr[1] = TimerFreq;

//$MODIFIED:
//    SendMessageGuaranteed(SendHeader, sizeof(MESSAGE_HEADER) + sizeof(long) * 2, GroupID);

    //$BUGBUG: really, we only want to send to machines that have connected players (and only send once per machine probably)

    // send message to each connected machine
    int cbSend = sizeof(MESSAGE_HEADER) + 2*sizeof(long);
    SendHeader->Contents = (short)cbSend; // specify size for all TCP messages
    SendHeader->PlayerID = LocalPlayerID;
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
//    SendHeader->Type = MESSAGE_JOIN_INFO;
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
//        wsprintf(buf, "Can't find Level directory '%s'", StartData.LevelDir);
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

void SendPlayerSync(void)  //$HEY: should probably rename this SendMachineSync !!
{
    assert( IsServer() ); //$ADDITION

    unsigned long *ptr = (unsigned long*)(SendHeader + 1);
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
//    SendHeader->Type = MESSAGE_PLAYER_SYNC_REQUEST;
//    ptr[0] = TotalRaceTime;
//
//    SendMessage(SendHeader, sizeof(MESSAGE_HEADER) + sizeof(long), player->PlayerID);

    int cbSend = sizeof(MESSAGE_HEADER) + sizeof(long);
    SendHeader->PlayerID = LocalPlayerID;
    sendto( soUDP, SendMsgBuffer, cbSend, 0,
            (sockaddr*)&MachineList[NextSyncMachine].SoAddr, sizeof(SOCKADDR_IN) );
//$END_MODIFICATIONS

// set next sync player

    NextSyncMachine++;
//$MODIFIED
//    NextSyncMachine %= (MachineCount - 1);
    if( NextSyncMachine >= MachineCount )
    {
        NextSyncMachine = 1;  // start at [1], b/c [0] is server (this machine)
    }
//$END_MODIFICATIONS
}

//////////////////////////////////
// reply to player sync message //
//////////////////////////////////

void ProcessPlayerSync1(void)  //$HEY: should probably rename this ProcessMachineSync1 !!
{
    unsigned long *rptr = (unsigned long*)(ReceiveHeader + 1);
    unsigned long *sptr = (unsigned long*)(SendHeader + 1);

// return host race time + my race time

    SendHeader->Type = MESSAGE_PLAYER_SYNC_REPLY1;
    sptr[0] = rptr[0];
    sptr[1] = TotalRaceTime;

//$MODIFIED
//    SendMessage(SendHeader, sizeof(MESSAGE_HEADER) + sizeof(unsigned long) * 2, FromID);

    int cbSend = sizeof(MESSAGE_HEADER) + 2*sizeof(unsigned long);
    SendHeader->PlayerID = LocalPlayerID;
    sendto( soUDP, SendMsgBuffer, cbSend, 0,
            (sockaddr*)&SoAddrFrom, sizeof(SOCKADDR_IN) );  // direct-send reply
//$END_MODIFICATIONS
}

//////////////////////////////////
// reply to player sync message //
//////////////////////////////////

void ProcessPlayerSync2(void)  //$HEY: should probably rename this ProcessMachineSync2 !!
{
    unsigned long *rptr = (unsigned long*)(ReceiveHeader + 1);
    unsigned long *sptr = (unsigned long*)(SendHeader + 1);
//$REMOVED    PLAYER *player;

// return my old race time + client race time + my new race time

    SendHeader->Type = MESSAGE_PLAYER_SYNC_REPLY2;
    sptr[0] = rptr[0];
    sptr[1] = rptr[1];
    sptr[2] = TotalRaceTime;

//$MODIFIED
//    SendMessage(SendHeader, sizeof(MESSAGE_HEADER) + sizeof(unsigned long) * 3, FromID);

    int cbSend = sizeof(MESSAGE_HEADER) + 3*sizeof(unsigned long);
    SendHeader->PlayerID = LocalPlayerID;
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
}

//////////////////////////////////
// reply to player sync message //
//////////////////////////////////

void ProcessPlayerSync3(void)  //$HEY: should probably rename this ProcessMachineSync3 !!
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

//$REMOVED_DEBUGONLY
//// set client->host->client last ping
//
//    player = GetPlayerFromPlayerID(FromID);
//    if (player)
//    {
//        player->LastPing = TotalRaceTime - client1;
//    }
//$END_REMOVAL
}

///////////////////////////////
// send race restart message //
///////////////////////////////

void SendMultiplayerRestart(void)
{
    assert( IsServer() ); //$ADDITION

    long i;
    LEVELINFO *li;
    RESTART_DATA *data = (RESTART_DATA*)(SendHeader + 1); //$CLEANUP: how about modifying RestartData directly, and copying to send buffer afterward (more readable that way).
                                                          /// (Only concern if there's some situation where send-buffer gets set, but early exit or something prevents copying into RestartData struct.)

// message

#ifdef OLD_AUDIO
    StopAllSfx();
#else
    g_SoundEngine.StopAll();
#endif // OLD_AUDIO
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
//$MODIFIED
//    SendMessageGuaranteed(SendHeader, sizeof(MESSAGE_HEADER) + sizeof(RESTART_DATA), GroupID);

    //$BUGBUG: really, we only want to send to machines that still have connected players (and only send once per machine probably)

    // send message to each connected machine
    int cbSend = sizeof(MESSAGE_HEADER) + sizeof(RESTART_DATA);
    SendHeader->Contents = (short)cbSend; // specify size for all TCP messages
    SendHeader->PlayerID = LocalPlayerID;
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

void ProcessMultiplayerRestart(void)
{

// save data

    RestartData = *(RESTART_DATA*)(ReceiveHeader + 1);

// start fade down

    GameLoopQuit = GAMELOOP_QUIT_RESTART;  //NOTE: this will cause us to call ClientMultiplayerRestart (see below; where we use RestartData values) next time thru gameloop.
    SetFadeEffect(FADE_DOWN);
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

#ifdef OLD_AUDIO
            PlaySfx3D(SFX_PICKUP, SFX_MAX_VOL, 22050, &obj->body.Centre.Pos, 2);
#else
            g_SoundEngine.PlaySubmixedSound( EFFECT_Pickup, FALSE, player->car.pSourceMix );
#endif // OLD_AUDIO

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
#ifdef OLD_AUDIO
        PlaySfx3D(SFX_HONKA, SFX_MAX_VOL, SFX_SAMPLE_RATE, &player->car.Body->Centre.Pos, 2);
#else
        g_SoundEngine.PlaySubmixedSound( EFFECT_Honka, FALSE, player->car.pSourceMix );
#endif // OLD_AUDIO
    }
}

/////////////////////////
// send a chat message //
/////////////////////////

//$TODO: probably will remove this func and all places where called (b/c not supporting text chat)
void SendChatMessage(char *message, DPID to)
{
//$REMOVED
//    SendHeader->Type = MESSAGE_CHAT;
//    strncpy((char*)(SendHeader + 1), message, PACKET_BUFFER_SIZE - sizeof(MESSAGE_HEADER));  //$MODIFIED: changed DP_BUFFER_MAX to PACKET_BUFFER_SIZE
//    SendMessage(SendHeader, sizeof(MESSAGE_HEADER) + strlen(message) + 1, to);
//$END_REMOVAL
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
// Desc: Extracts the data from a MESSAGE_VOICE_PACKET packet.
//-----------------------------------------------------------------------------
void ProcessVoiceMessage()
{
    // NOTE (JHarding): Modeled after ProcessCarMessage.  Right now, all
    // voice is broadcast to all players
    MESSAGE_HEADER* pHeaderRecv = (MESSAGE_HEADER*)RecvMsgBuffer;
    VOID*           pvData = (VOID *)( pHeaderRecv + 1 );
    PLAYER *player;
    INT     nDataSize = RecvMsgSize - sizeof( MESSAGE_HEADER );

    for( player = PLR_PlayerHead; NULL != player; player = player->next )
    {
        if( player->type == PLAYER_LOCAL )
        {
            // TODO (JHarding): Figure out player->communicator mapping
            g_VoiceCommunicators[0].ReceivePacket( pvData, nDataSize );
        }
    }
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
//    MESSAGE_HEADER* pHeaderSend = (MESSAGE_HEADER*)SendMsgBuffer;
//    pHeaderSend->Type = MESSAGE_PING_REQUEST;
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
//    MESSAGE_HEADER* pHeaderRecv = (MESSAGE_HEADER*)RecvMsgBuffer;
//    MESSAGE_HEADER* pHeaderSend = (MESSAGE_HEADER*)SendMsgBuffer;
//    BYTE* pbDataRecv = (BYTE*)(pHeaderRecv + 1);
//    BYTE* pbDataSend = (BYTE*)(pHeaderSend + 1);
//
//    // setup return packet
//    pHeaderSend->Type = MESSAGE_PING_RETURN;
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

    DrawBigGameText(208, 224, 0x808000, "Enter Game Name:");
    SessionName[MenuCount] = '_';
    SessionName[MenuCount + 1] = 0;
    DrawBigGameText(128, 276, 0x808080, SessionName);
}


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

    DrawGameText(264, 16, 0x808000, "Choose a Game:");

    GetRemoteMessages(); //$ADDITION (cprince; they didn't need this b/c was handled via DPlay/EnumSessionsCallback)

    short i;
    for (i = 0 ; i < SessionCount ; i++)
    {
        long color;

        if (MenuCount == i) color = 0xff0000;
        else color = 0x808080;
        DrawGameText(168, i * 16 + 48, color, SessionList[i].name);
        //$CMP_NOTE: might want to dump more detailed info here (eg, Acclaim's code output string saying if session was open or closed/started)
    }

//$CMP_NOTE:  Not currently supported. Right now, we don't get PlayerList/PlayerCount until we join a session.
//    // list players in selected session
//    if (!SessionCount) PlayerCount = 0;
//
//    if (PlayerCount)
//    {
//        DrawGameText(288, 192, 0x808000, "Players:");
//        for (i = 0 ; i < PlayerCount ; i++)
//        {
//            DrawGameText(168, i * 16 + 224, 0xff0000, PlayerList[i].Name);
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
            DrawGameText(288, 192, 0x808000, "Join request FAILED!");
        }
    }
}



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

    // If we're still accepting new machine connections, see if there are
    // any such connections requested
    while( MachineCount < MAX_NUM_MACHINES )
    {
        NET_MACHINE* pNewMachine = &( MachineList[MachineCount] );

        int cbAddr = sizeof(pNewMachine->SoAddr);
        SOCKET socket = accept( soListenTCP, (sockaddr*)&pNewMachine->SoAddr, &cbAddr );

        // exit loop if no pending connections
        if( INVALID_SOCKET == socket )  { break; }

        // store connection info
        pNewMachine->SoAddr.sin_port = GAME_PORT;  // need to set; else it's whatever random port the client used for outbound connection!!
        pNewMachine->Socket = socket;
        XNetInAddrToXnAddr( pNewMachine->SoAddr.sin_addr,
                            &(pNewMachine->XnAddr),
                            &(Session.keyID) );
        MachineCount++;
    }
}

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

    DrawGameText(288, 64, 0x808000, "Players:");
    DrawGameText(216, 400, 0x808000, "Hit Enter To Start Game...");

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
        DrawGameText(192, (short)(i * 16 + 96), 0xff0000, PlayerList[i].Name);
        wsprintf(buf, "%ld", PlayerList[i].Ping);  //$TODO: ACCLAIM AUG99 CHANGED THIS TO PRINT CAR NAME, _INSTEAD_OF_ PING
        DrawGameText(448, (short)(i * 16 + 96), 0xff0000, buf);
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
//        Session.dwUser1 = TRUE;  //$NOTE: bStarted flag
//        DP->SetSessionDesc(&Session, NULL);

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

    DrawGameText(288, 64, 0x808000, "Players:");
    DrawGameText(240, 400, 0x808000, "Waiting For Host...");

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
        DrawGameText(192, i * 16 + 96, 0xff0000, PlayerList[i].Name);
        wsprintf(buf, "%ld", PlayerList[i].Ping);  //$TODO -- ACCLAIM AUG99 CHANGED THIS TO PRINT CAR NAME, _INSTEAD_OF_ PING
        DrawGameText(448, i * 16 + 96, 0xff0000, buf);
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




//$HEY -- DO WE NEED THIS (SEEMS ONLY FOR WAITING-ROOM CHAT)

/////////////////////////////////
// update multiplayer messages //
/////////////////////////////////

void UpdateMessages(void)  //$NOTE(cprince): better name would be HandleChatMessages
{
    long i;
    char c;

// enable?

    if (!WaitingRoomMessageActive && Keys[DIK_F12] && !LastKeys[DIK_F12])
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
//$REMOVED (tentative!!)
//            SendChatMessage(WaitingRoomMessages[i], GroupID);
//$END_REMOVAL

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


//$REMOVED - NOT USING THE FIELDS FROM "DP_SESSION Session"
//HEY - AUG99 ADDITION  (fills-in 'Session' var, and calls DP::SetSessionDesc
//NOTE: ONLY PLACE WHERE RESULTS OF DPlay::SetSessionDesc GETS USED IS IN MENUDRAW FUNCS (GetSessionDesc in DrawJoinedPlayerList)
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
//    wsprintf(buf, "%s\n%s", name, track);
//    Session.lpszSessionNameA = buf;
//    Session.dwUser1 = started;
//    Session.dwUser2 = gametype;
//    Session.dwUser3 = MultiplayerVersion;
//    Session.dwUser4 = 0;
//
//    if (randomcars) Session.dwUser4 |= 1;
//    if (randomtrack) Session.dwUser4 |= 2;
//
//// set
//
//    r = DP->SetSessionDesc(&Session, NULL);
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



