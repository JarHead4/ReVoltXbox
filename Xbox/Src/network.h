//-----------------------------------------------------------------------------
// File: network.h
//
// Desc: Low-level networking code for Re-Volt.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef NETWORK_H
#define NETWORK_H

#include <xtl.h>
#include <xonline.h>

#include "dx.h"
#include "LevelInfo.h"
#include "main.h"
#include "object.h"


// macros

#define GAME_PORT 1000  // could be any port ; use port 1000 for XNet bandwidth optimizations

//$TODO - SEE IF WE NEED TO REMOVE ANYTHING FROM THESE AUG99 #DEFINE ADDITIONS
//$REMOVED#define USE_GROUPS 1
#define MAX_HOST_NAME 1024
#define MAX_HOST_COMPUTER 33

#define DP_PACKETS_PER_SEC 10  //$MODIFIED: was originally 6
#define DP_SEND_TIMEOUT 2000
#define DP_SYNC_TIME 5.0f
#define DP_SYNC_PING_MAX 1000
#define DP_POSITION_TIME 20.0f
#define DP_BUFFER_MAX 2500

//$NOTE(cprince): these scale values make me nervous (floating-pt mul/div by same value can change result!)
#define REMOTE_QUAT_SCALE     100.0f
#define REMOTE_VEL_SCALE        3.0f
#define REMOTE_ANGVEL_SCALE  1000.0f


enum {
    DP_CLIENT_PLAYER,  //$HEY: rename to NET_PLAYERTYPE_CLIENT
    DP_SERVER_PLAYER,  //$HEY: rename to NET_PLAYERTYPE_SERVER
    DP_SPECTATOR_PLAYER, //$HEY: CAN PROBABLY REMOVE; NO SUPPORT FOR SPECTATORS
};

enum {
    NETWORK_GAME_STARTING,
    NETWORK_GAME_JOINING,
};

#define IsMultiPlayer() \
    (GameSettings.GameType == GAMETYPE_NETWORK_RACE || GameSettings.GameType == GAMETYPE_NETWORK_BATTLETAG)

#define IsServer() \
    (GameSettings.MultiType == MULTITYPE_SERVER)

#define IsClient() \
    (GameSettings.MultiType == MULTITYPE_CLIENT)


#define PACKET_BUFFER_SIZE  1300  //$NOTE: our max message size can't be larger than this buffer size
//$REVISIT(cprince): what max-buffer size do we want here?  Was DP_BUFFER_MAX (2500) in Acclaim Aug99 code.


//$NOTE: transferred this to here from LevelLoad.h
enum {
    MULTITYPE_NONE,
    MULTITYPE_SERVER,
    MULTITYPE_CLIENT,
};



#define MAX_NUM_MACHINES 6  //$TODO: should MAX_NUM_PLAYERS be derived from this?  (And should they be defined in the same place?)
//#define MAX_SESSION_NAME 128  //$REVISIT: might want to put this back, if we allow players to pick a session name for network games.
#define MAX_SESSION_SEARCH_RESULTS 16
//$REMOVED#define MAX_CONNECTION_NAME 128
//#define MAX_LEVEL_DIR_NAME 16  DELETED IN AUG99 DROP
#define MAX_DEPARTED_PLAYERS 10


//
// Message types: base and extended (separated for network bandwidth reduction)
// "Base" message types are sent more frequently, and they require less bandwidth.
// "Extended" message types are sent less frequently, and they require more bandwidth.
//

// Base message types
// (Store these inside the message header.)
enum MSG_TYPE {
    MSG_CAR_DATA,                           // 0
    MSG_VOICE_PACKET,                       // 1
    MSG_OBJECT_DATA,                        // 2
    MSG_WEAPON_DATA,                        // 3
    MSG_TARGET_STATUS_DATA,                 // 4
    MSG_PLAYER_SYNC_REQUEST,                // 5
    MSG_PLAYER_SYNC_REPLY1,                 // 6
    MSG_PLAYER_SYNC_REPLY2,                 // 7
    MSG_POSITION,                           // 8
    MSG_OTHER = 9  // this is the max base msg type (else won't fit in message header)
};

// Extended message types
// (Store these *after* the message header, and put MSG_OTHER in the message header.)
enum MSG_TYPE_EXT {
    MSGEXT_CAR_NEWCAR,                         // 0
    MSGEXT_CAR_NEWCAR_ALL,                     // 1
    MSGEXT_GAME_LOADED,                        // 2
    MSGEXT_GAME_STARTED,                       // 3
    MSGEXT_COUNTDOWN_START,                    // 4
    MSGEXT_RACE_FINISH_TIME,                   // 5
    MSGEXT_SYNC_REQUEST,                       // 6
    MSGEXT_SYNC_REPLY,                         // 7
    MSGEXT_RESTART,                            // 8
    // Weapon-related messages
    MSGEXT_TRANSFER_BOMB,                      // 9
    MSGEXT_BOMBTAG_CLOCK,                      // 10
    MSGEXT_TRANSFER_FOX,                       // 11
    MSGEXT_ELECTROPULSE_THE_WORLD,             // 12
    MSGEXT_GOT_GLOBAL,                         // 13
    MSGEXT_HONKA,                              // 14
    // Session-setup messages
    MSGEXT_FIND_SESSION,                       // 15
    MSGEXT_SESSION_FOUND,                      // 16
    MSGEXT_REQUEST_ADDPLAYERS,                 // 17
    MSGEXT_ADDPLAYERS_ACCEPTED,                // 18
    MSGEXT_ADDPLAYERS_REJECTED,                // 19
    MSGEXT_PLAYER_LIST,                        // 20
    MSGEXT_CHATTER_LIST,                       // 21
    MSGEXT_VOICE_INFO,                         // 22
//$REMOVED_MAYBEDEPRECATED    MSGEXT_PING_REQUEST,
//$REMOVED_MAYBEDEPRECATED    MSGEXT_PING_RETURN,
};

//$HEY -- REMOVED THESE IN AUG99 DROP
//#define MESSAGE_CONTENTS_CAR_TIME    (1<<0) //  1
//#define MESSAGE_CONTENTS_CAR_POS     (1<<1) //  2
//#define MESSAGE_CONTENTS_CAR_QUAT    (1<<2) //  4
//#define MESSAGE_CONTENTS_CAR_VEL     (1<<3) //  8
//#define MESSAGE_CONTENTS_CAR_ANGVEL  (1<<4) // 16
//#define MESSAGE_CONTENTS_CAR_CONTROL (1<<5) // 32


//
// Message header structs
//
#include <pshpack1.h> // one-byte alignment for structs sent over network frequently
  typedef struct {
      BYTE TypeAndID;
  } MSG_HEADER;

  typedef struct {
      BYTE TypeAndID;
      BYTE ExtendedType;
  } MSG_HEADER_EXT;

  // First message (only) in TCP "packet" must use the following struct
  // ('Size' followed by a MSG_HEADER_EXT struct.)
  typedef struct {
      WORD Size;
      BYTE TypeAndID;
      BYTE ExtendedType;
  } MSG_HEADER_TCP;
#include <poppack.h>


//
// Functions to fill/parse message headers
//
void SetSendMsgHeader( MSG_TYPE MsgType );
void SetSendMsgHeaderExt( MSG_TYPE_EXT MsgTypeExt );
void SetSendMsgHeaderTcp( MSG_TYPE MsgType, MSG_TYPE_EXT MsgTypeExt, int size );

void GetRecvMsgHeader( MSG_TYPE* pMsgType, MSG_TYPE_EXT* pMsgTypeExt );




enum
{
    VOICEINFO_ADDCHATTER,
    VOICEINFO_REMOVECHATTER,
    VOICEINFO_ADDREMOTEMUTE,
    VOICEINFO_REMOVEREMOTEMUTE,
};

struct VOICE_INFO {
    DWORD dwOperation;
    DWORD dwDestPlayerID;
};




typedef struct {
    XNADDR       XnAddr;
    SOCKADDR_IN  SoAddr;
    SOCKET       Socket;
    //$CMP_NOTE: don't send the following over the network (very large, and not necessary)
    short        RecvMsgCurrBytes;                  // for TCP connection
    short        RecvMsgTotalBytes;                 // for TCP connection
    char         RecvMsgBuffer[PACKET_BUFFER_SIZE]; // for TCP connection (need buffer per TCP connection)
} NET_MACHINE;

typedef struct {
    DWORD        PlayerID;  //$REVISIT: should PlayerID be changed to a BYTE throughout the code?
    SOCKADDR_IN  SoAddr;  //$CMP_NOTE: no need to send this over net.  Options: (a) store this in a parallel array, (b) change net protocol to not send entire struct, (c) don't worry about it (b/c small, and only used during session-init)
    XNADDR       XnAddr;
    XUID         xuid;    //$ADDED by jharding 3/27/02
    long         Ping;    //$CMP_NOTE: no need to send this over net.  Options: (a) store this in a parallel array, (b) change net protocol to not send entire struct, (c) don't worry about it (b/c small, and only used during session-init)
//$HEY: AUG99 DROP REMOVED THE 'Ping' FIELD; CAN WE DO THE SAME?
    char         Name[MAX_PLAYER_NAME];
    char         GamerTag[XONLINE_GAMERTAG_SIZE];
    long         CarType;
    BYTE         bInvitedByFriend;  //$REVISIT: must be a more space-efficient way to pack this with rest of struct (but isn't critical)
//$HEY: AUG99 DROP ADDED THE FOLLOWING FIELDS
long Spectator;
long Host;
} NET_PLAYER;

typedef struct {
    XNKID   keyID;
    XNKEY   key;  //$REVISIT: check whether we really use/need this guy
    XNADDR  XnAddrServer;
    XNQOS*  pQosResults;
    //char    strSessionName[MAX_SESSION_NAME];  //$REVISIT: might want to put this back, if we allow players to pick a session name for network games.
    WCHAR   wstrHostNickname[MAX_PLAYER_NAME];
    WCHAR   wstrHostUsername[XONLINE_GAMERTAG_SIZE];  // only valid for XOnline sessions (not SysLink), right?
//$HEY -- THE FOLLOWING WERE NEW FIELDS IN AUG99 DROP
DWORD Started;  //$HEY: can we get rid of this field?  (Will we ever broadcast sessions that have already started.  Maybe we'll notify everyone right when game starts...)
DWORD GameType;
DWORD Version;  //$REVISIT: can we remove this field?  (Will people always have latest XBE version before they can play online?)
DWORD RandomCars;
DWORD RandomTrack;
DWORD nCurrPlayersPublic; //$NOTE: was originally named PlayerNum
DWORD nMaxPlayersPublic;  //$ADDITION
DWORD nCurrPlayersPrivate;  //$NEW
DWORD nMaxPlayersPrivate;   //$NEW
long  iLevelNum;   //$ADDITION  //$NOTE: probably still want to send level with MultiStartData, in case host changes it quickly and client doesn't get notified via matchmaking server.
} NET_SESSION;



//$HEY: are there any fields here that we still want to propagate to other players?
////FROM AUG99 DROP (CAN WE REMOVE THIS STRUCT AGAIN, LIKE WE DID BEFORE?) (NOTE THAT ALL FIELDS ARE NEW; AND REMOVED THE LONE 'CarID' FIELD FROM BEFORE)
//#define CAR_NAMELEN 20
//typedef struct {
//    long Ready, Cheating;
//    char CarName[CAR_NAMELEN];
//} DP_PLAYER_DATA;


//$HEY -- NEW IN AUG99 DROP
typedef struct {
    unsigned long RaceTime;
    char LevelDir[MAX_LEVEL_INF_NAME];
} JOIN_INFO;

//$HEY -- NEW IN AUG99 DROP
typedef struct {
    unsigned long Time;
} RACE_TIME_INFO;



/*
//$TODO - NEED TO CHECK CHANGES IN THESE STRUCTS, AND UPDATE MY NET CODE TO USE/SET NEW FIELDS!!
//$HEY -- AUG99 MOVED THESE 2 STRUCTS TO InitPlay.h (GRR...)
typedef struct {
    long   RaceStartIndex;
    long   CarType;
    DWORD  PlayerID;
    char   Name[MAX_PLAYER_NAME];
} PLAYER_START_DATA;
typedef struct {
    long              PlayerNum;  //$TODO: should rename this to NumPlayers
    char              LevelDir[MAX_LEVEL_DIR_NAME];
    PLAYER_START_DATA PlayerData[MAX_NUM_PLAYERS];
} START_DATA;
*/

//$HEY -- NEW STRUCT IN AUG99 DROP
#define CAR_NAMELEN 20
typedef struct {
    long Seed, NewTrack;
    char CarName[CAR_NAMELEN];  //$BUG: switch to using CarType instead of CarName[] eventually...
    char LevelDir[MAX_PATH];
} RESTART_DATA;
//$NOTE: For the most part, the RESTART_DATA struct only gets used in SendMultiplayerRestart, ProcessMultiplayerRestart, and ClientMultiplayerRestart
/// (Only one other place outside these funcs: in GLP_GameLoop for determining argument to EnableLoadThread)
///
/// NewTrack -- is a bool (telling whether track has changed)
/// LevelDir -- only valid if NewTrack is set; tells name of new track
/// CarName -- somehow tied to GameSettings.RandomCars var (why?); all cars get set to this type!
/// Seed -- a rand-num-gen seed (sync'd between server/clients)
///
/// Fields from RestartData are used ONLY for: updating StartData struct (on client), and sending RestartData to client (on server)
/// These updates have 1:1 correspondence with GAMELOOP_QUIT_RESTART.


//struct object_def;  //$HEY: AUG99 ADDITION



// prototypes
//$TODO - CLEANUP FUNCTION PROTOTYPES BELOW (ADD MISSING ONES, AND REMOVE NONEXISTENT ONES)

//$REVISIT: Could move lots of these function prototypes into network.cpp,
// because they're only accessed from that file.  But then might fragment
// the code b/c Send** and Process** funcs would be in different locations.

bool  InitNetwork(void);
void  KillNetwork(void);

int   GetRemoteMessages(void);

bool  CreateSession();  //$REVISIT: might want to put back the strSessionName argument, if we allow players to pick a session name for network games.
void  DestroySession(void);
BOOL  JoinSession(int iSession);
void  LeaveSession(void);

void  RequestSessionList(void);
void  InsertSessionListEntry( NET_SESSION* pSession );
void  DeleteSessionListEntry( int index );
void  ClearSessionList(void);

void  SetGameStarted(void);  // server-only function

void  QueueMessage(void* buf, WORD size);
void  QueueVoiceMessage(void* buf, WORD size, DWORD dwPlayerID );
void  TransmitMessageQueue(void);

void  RequestAddPlayers(DWORD dwPlayers);
BYTE  GetUnusedPlayerID(void);
LONG  PlayerIndexFromPlayerID( DWORD dwPlayerID );
LONG  PlayerIndexFromXUID( XUID xuid );

//REMOVED_TENTATIVEvoid  RequestPings(void);
//REMOVED_TENTATIVEvoid  ProcessPingRequest(void);
//REMOVED_TENTATIVEvoid  ProcessPingReturn(void);

//$REMOVED_UNREACHABLEvoid  EnterSessionName(void);
//$REMOVED_UNREACHABLEvoid  HostWait(void);
//$REMOVED_UNREACHABLEvoid  BrowseSessions(void);
//$REMOVED_UNREACHABLEvoid  ClientWait(void);
void  LookForClientConnections(void);
void  CreateLocalServerPlayers(void);

void  ProcessMessage(void);

int   ProcessVoiceMessage(void);
void  SendVoiceInfoMessage( DWORD dwOperation, DWORD dwDestPlayerID );
int   ProcessVoiceInfoMessage(void);

//$REMOVED_DOESNOTHINGbool CreatePlayer(char *name, long mode);
void SetPlayerData(void);

unsigned long GetSendQueueLength(void); //$NOTE: was originally GetSendQueue(DPID id)
void  UpdatePacketInfo(void);
void  CheckAllPlayersReady(void);
void  SendGameStarted(void);
int   ProcessGameStarted(void);
void  SendSyncRequest(void);
int   ProcessSyncRequest(void);
int   ProcessSyncReply(void);
void  SendGameLoaded(void);
int   ProcessGameLoaded(void);
void  SendCountdownStart(void);
int   ProcessCountdownStart(void);
//$REMOVED_UNREACHABLEvoid  SendJoinInfo(DPID id);
//$REMOVED_UNREACHABLEvoid  ProcessJoinInfo(void);
void  SendRaceFinishTime(void);
int   ProcessRaceFinishTime(void);
void  SendPlayerSync(void);
int   ProcessPlayerSync1(void);
int   ProcessPlayerSync2(void);
int   ProcessPlayerSync3(void);
void  SendMultiplayerRestart(void);
int   ProcessMultiplayerRestart(void);
void  ClientMultiplayerRestart(void);
void  SendPosition(void);
int   ProcessPosition(void);
void  SendTransferBomb(GLOBAL_ID objid, unsigned long player1id, unsigned long player2id);
int   ProcessTransferBomb(void);
void  SendTransferFox(unsigned long player1id, unsigned long player2id);
int   ProcessTransferFox(void);
void  SendBombTagClock(void);
int   ProcessBombTagClock(void);
void  SendElectroPulseTheWorld(long slot);
int   ProcessElectroPulseTheWorld(void);
void  SendGotGlobal(void);
int   ProcessGotGlobal(void);
void  SendHonka(void);
int   ProcessHonka(void);

void  RemoteSyncHost(void);
void  RemoteSyncClient(void);
void  ProcessPlayerJoining(void);
void  ProcessPlayerLeaving(void);
  #ifndef XBOX_NOT_YET_IMPLEMENTED  // we don't support host migration yet
void  ProcessBecomeHost(void);
  #endif // ! XBOX_NOT_YET_IMPLEMENTED
//$REMOVED_TENTATIVEunsigned long WINAPI ReceiveThread(void *param);
//$REMOVEDvoid RefreshSessions(void);



// globals
//$TODO -- CLEANUP GLOBAR-VAR DECLARATIONS (ADD MISSING ONES; AND REMOVE NONEXISTENT ONES)
/// ALSO, CONSIDER MOVING SOME OF THESE INTO network_private.h (SINCE DON'T NEED GLOBAL ACCESS).  BUT MAYBE THAT WOULD CAUSE CLUTTER AND DO MORE DAMAGE THAN GOOD.

extern WORD LocalPlayerID;
extern DWORD FromID, ServerID;  //$NOTE: type was DPID originally  //$REVISIT: using FromID and ServerID is kind of ugly (leftover from porting Acclaim DPlay code); I'd like to remove them eventually.

extern char  SendMsgBuffer[];
extern char* RecvMsgBuffer;
extern long MessageQueueSize;  //$TODO: cleanup code so we don't have to expose this

extern NET_PLAYER  PlayerList[];
extern long PlayerCount;

typedef std::vector<NET_PLAYER> PLAYER_LIST;
extern PLAYER_LIST DepartedPlayerList;

extern NET_MACHINE MachineList[];  // NOTE: [0] is always the server machine
extern long MachineCount;  //$NOTE: MachineList/MachineCount were static until we started accessing from online task pump.
extern NET_MACHINE* pMachineLocal;  // makes it easier to skip self when looping through machine list

extern NET_SESSION SessionList[];
char GetSessionCount();  // returns SessionCount ; accessor is to prevent external code from modifying value
extern bool DisableSessionListUpdate;  // to avoid changing list entries (eg, user selects a session but session-discovery message comes back and updates list behind the scenes)

extern NET_SESSION SessionCurr;  //$NOTE: this was originally named Session  //$NOTE: this was originally static (until we needed to access from online task pump)

extern SOCKET soUDP;              //$CMP_NOTE: (might want to un-expose this) only used in SendLocalCarData()

extern XNADDR XnAddrLocal;

extern BOOL bGameStarted;

extern long NextPositionReady, NextPacketReady, NextSyncReady, AllPlayersReady, HostQuit, LocalPlayerReady;
extern REAL NextPacketTimer, NextSyncTimer, NextPositionTimer;
extern REAL AllReadyTimeout;
extern long NextSyncMachine;
extern char SessionPick;

extern DWORD dwLocalPlayerCount;
#define      INVALID_PLAYER_ID  (0)

extern BYTE g_bInvitedByFriend;  // non-zero if joining session due to XOnline Friend invitation; else zero.


//$HACK(Apr02_GameBash) - matchmaking parameters are colliding everywhere
extern LONG g_lMatchmakingLevelNum;


#define IsSameMachine(xnaddr1,xnaddr2)  ( 0 == memcmp(&((xnaddr1).abEnet), &((xnaddr2).abEnet), 6) )

union ZERO_UNION
{
    XNKID xnkid;
    XUID  xuid;
};
extern const ZERO_UNION g_Zero;

#define IsSameXNKID(_xnkid1,_xnkid2)  ( 0 == memcmp(&(_xnkid1), &(_xnkid2), sizeof(XNKID)) )
#define IsZeroXNKID(_xnkid)          ( 0 == memcmp(&(_xnkid), &g_Zero.xnkid, sizeof(XNKID)) )
//#define IsSameXUID(_xuid1,_xuid2)     ( 0 == memcmp(&(_xuid1), &(_xuid2), sizeof(XUID)) )
#define IsSameXUID(_xuid1,_xuid2)     ( XOnlineAreUsersIdentical(&(_xuid1), &(_xuid2)) )
#define IsZeroXUID(_xuid)            ( 0 == memcmp(&(_xuid), &g_Zero.xuid, sizeof( XUID)) )
#define IsInGameSession()           ( ! IsZeroXNKID(SessionCurr.keyID) )
#define IsInWaitingRoom()           ( IsInGameSession() && !bGameStarted )



#endif // NETWORK_H

