//-----------------------------------------------------------------------------
// File: network.h
//
// Desc: Low-level networking code for Re-Volt.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef NETWORK_H
#define NETWORK_H


#include "dx.h"
#include "LevelInfo.h"
#include "main.h"


// macros

//$TODO - SEE IF WE NEED TO REMOVE ANYTHING FROM THESE AUG99 #DEFINE ADDITIONS
//$REMOVED#define USE_GROUPS 1
#define MAX_HOST_NAME 1024
#define MAX_HOST_COMPUTER 33

#define DP_PACKETS_PER_SEC 6
#define DP_SEND_TIMEOUT 2000
#define DP_SYNC_TIME 5.0f
#define DP_SYNC_PING_MAX 1000
#define DP_POSITION_TIME 20.0f
#define DP_BUFFER_MAX 2500

#define REMOTE_QUAT_SCALE 100.0f
#define REMOTE_VEL_SCALE 3.0f
#define REMOTE_ANGVEL_SCALE 1000.0f

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
    (GameSettings.GameType == GAMETYPE_MULTI || GameSettings.GameType == GAMETYPE_BATTLE)

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



#define MAX_NUM_MACHINES 4
#define MAX_NUM_SESSIONS 8
#define MAX_SESSION_NAME 128
//$REMOVED#define MAX_CONNECTION_NAME 128
//#define MAX_LEVEL_DIR_NAME 16  DELETED IN AUG99 DROP



enum {
    MESSAGE_NULL,

    MESSAGE_CAR_DATA,
    MESSAGE_CAR_NEWCAR,
    MESSAGE_CAR_NEWCAR_ALL,
    MESSAGE_WEAPON_DATA,
    MESSAGE_OBJECT_DATA,
    MESSAGE_TARGET_STATUS_DATA,
    MESSAGE_GAME_STARTED,
    MESSAGE_SYNC_REQUEST,
    MESSAGE_SYNC_REPLY,
    MESSAGE_GAME_LOADED,
    MESSAGE_COUNTDOWN_START,
//$REMOVED_UNREACHABLE    MESSAGE_JOIN_INFO,
    MESSAGE_RACE_FINISH_TIME,
    MESSAGE_PLAYER_SYNC_REQUEST,
    MESSAGE_PLAYER_SYNC_REPLY2,
    MESSAGE_PLAYER_SYNC_REPLY1,
    MESSAGE_QUEUE,
    MESSAGE_RESTART,
    MESSAGE_POSITION,
    MESSAGE_TRANSFER_BOMB,
    MESSAGE_CHAT,
    MESSAGE_BOMBTAG_CLOCK,
    MESSAGE_TRANSFER_FOX,
    MESSAGE_ELECTROPULSE_THE_WORLD,
    MESSAGE_GOT_GLOBAL,
    MESSAGE_HONKA,
//$HEY -- REMOVED THESE IN AUG99 DROP
//#define MESSAGE_CONTENTS_CAR_TIME    (1<<0) //  1
//#define MESSAGE_CONTENTS_CAR_POS     (1<<1) //  2
//#define MESSAGE_CONTENTS_CAR_QUAT    (1<<2) //  4
//#define MESSAGE_CONTENTS_CAR_VEL     (1<<3) //  8
//#define MESSAGE_CONTENTS_CAR_ANGVEL  (1<<4) // 16
//#define MESSAGE_CONTENTS_CAR_CONTROL (1<<5) // 32

//$HEY -- OLD STUFF THAT WE NEED TO KEEP AROUND (FOR REPLACING DPLAY SESSION SETUP)
    MESSAGE_FIND_SESSION,
    MESSAGE_SESSION_FOUND,
    MESSAGE_REQUEST_ADDPLAYERS,
    MESSAGE_ADDPLAYERS_ACCEPTED,
    MESSAGE_ADDPLAYERS_REJECTED,
    MESSAGE_PLAYER_LIST,
    MESSAGE_VOICE_PACKET,
//$REMOVED_MAYBEDEPRECATED    MESSAGE_PING_REQUEST,
//$REMOVED_MAYBEDEPRECATED    MESSAGE_PING_RETURN,

};

typedef struct {
    unsigned short Type;
    unsigned short Contents;  //$HEY -- THIS WAS RENAMED 'Flag' IN AUG99 DROP (BUT NAME SEEMS SUCKY; MAYBE LEAVE AS 'Contents' OR SOMETHING ELSE)
DWORD PlayerID;  //$BUG: this is ugly; need better way to specify which player struct to update (and note this field isn't even necessary for some message types!)
} MESSAGE_HEADER;



typedef struct {
    SOCKET       Socket;
    SOCKADDR_IN  SoAddr;
    XNADDR       XnAddr;
    //$CMP_NOTE: don't send the following over the network (very large, and not necessary)
    short        RecvMsgCurrBytes;                  // for TCP connection
    short        RecvMsgTotalBytes;                 // for TCP connection
    char         RecvMsgBuffer[PACKET_BUFFER_SIZE]; // for TCP connection (need buffer per TCP connection)
} NET_MACHINE;

typedef struct {
    DWORD        PlayerID;
    SOCKADDR_IN  SoAddr;  //$CMP_NOTE: no need to send this over net.  Options: (a) store this in a parallel array, (b) change net protocol to not send entire struct, (c) don't worry about it (b/c small, and only used during session-init)
    XNADDR       XnAddr;
    long         Ping;    //$CMP_NOTE: no need to send this over net.  Options: (a) store this in a parallel array, (b) change net protocol to not send entire struct, (c) don't worry about it (b/c small, and only used during session-init)
//$HEY: AUG99 DROP REMOVED THE 'Ping' FIELD; CAN WE DO THE SAME?
    char         Name[MAX_PLAYER_NAME];
    long         CarType;
//$HEY: AUG99 DROP ADDED THE FOLLOWING FIELDS
long Spectator;
long Host;
} NET_PLAYER;

typedef struct {
    XNKID   keyID;
    XNKEY   key;  //$REVISIT: check whether we really use/need this guy
    XNADDR  XnAddrServer;
    char    name[MAX_SESSION_NAME];
    //$CMP_NOTE: what about max/curr players?
//$HEY -- THE FOLLOWING WERE NEW FIELDS IN AUG99 DROP
DWORD Started;
DWORD GameType;
DWORD Version;
DWORD RandomCars;
DWORD RandomTrack;
DWORD PlayerNum;
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
    char LevelDir[MAX_LEVEL_DIR_NAME];
} JOIN_INFO;

//$HEY -- NEW IN AUG99 DROP
typedef struct {
    unsigned long Time;
} RACE_TIME_INFO;



//$TODO - NEED TO CHECK CHANGES IN THESE STRUCTS, AND UPDATE MY NET CODE TO USE/SET NEW FIELDS!!
/*
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
    char LevelDir[MAX_LEVEL_DIR_NAME];
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
//$TODO - CLEANUP FUNCTION PROTOTYPES ABOVE (ADD MISSING ONES, AND REMOVE NONEXISTENT ONES)

extern bool  InitNetwork(void);
extern void  KillNetwork(void);

extern bool  CreateSession(char *name);
extern void  DestroySession(void);
extern BOOL  JoinSession(int iSession);
extern void  LeaveSession(void);
extern void  RequestSessionList(void);

extern void  RequestAddPlayers(DWORD dwPlayers);
extern DWORD NextPlayerID(void);

extern int   GetRemoteMessages(void);
extern BOOL  GetSingleMessageTCP( SOCKET socket, char* pRecvBuffer,
                                  short* pMsgCurrBytes, short* pMsgTotalBytes );
extern void  ProcessMessage(void);
extern void  ProcessCarMessage(void); //$HEY -- I THINK THIS FUNC IS DEPRECATED (REPLACED BY MORE GRANULAR ONES)
extern void  ProcessPlayerReady(void);

// NOTE (JHarding): Added for voice processing...
extern void  ProcessVoiceMessage(void);

extern void  RequestPings(void);
extern void  ProcessPingRequest(void);
extern void  ProcessPingReturn(void);

extern void  EnterSessionName(void);
extern void  HostWait(void);
extern void  BrowseSessions(void);
extern void  ClientWait(void);
extern void  LookForClientConnections(void);
extern void CreateLocalServerPlayers(void);




void QueueMessage(MESSAGE_HEADER *buff, short size);
void TransmitMessageQueue(void);

extern bool CreatePlayer(char *name, long mode);
extern void SetPlayerData(void);

extern unsigned long GetSendQueueLength(void); //$NOTE: was originally GetSendQueue(DPID id)
extern void UpdatePacketInfo(void);
extern void CheckAllPlayersReady(void);
extern void SendGameStarted(void);
extern void ProcessGameStarted(void);


extern void ProcessGameStarted(void);
extern void SendSyncRequest(void);
extern void ProcessSyncRequest(void);
extern void ProcessSyncReply(void);
extern void SendGameLoaded(void);
extern void ProcessGameLoaded(void);
extern void SendCountdownStart(void);
extern void ProcessCountdownStart(void);
//$REMOVED_UNREACHABLEextern void SendJoinInfo(DPID id);
//$REMOVED_UNREACHABLEextern void ProcessJoinInfo(void);
extern void SendRaceFinishTime(void);
extern void ProcessRaceFinishTime(void);
extern void SendPlayerSync(void);
extern void ProcessPlayerSync1(void);
extern void ProcessPlayerSync2(void);
extern void ProcessPlayerSync3(void);
extern void SendMultiplayerRestart(void);
extern void ProcessMultiplayerRestart(void);
extern void ClientMultiplayerRestart(void);
extern void SendPosition(void);
extern void ProcessPosition(void);
extern void SendTransferBomb(LARGE_INTEGER objid, unsigned long player1id, unsigned long player2id);
extern void ProcessTransferBomb(void);
extern void SendTransferFox(unsigned long player1id, unsigned long player2id);
extern void ProcessTransferFox(void);
extern void SendChatMessage(char *message, DPID to);
extern void ProcessChatMessage(void);
extern void SendBombTagClock(void);
extern void ProcessBombTagClock(void);
extern void SendElectroPulseTheWorld(long slot);
extern void ProcessElectroPulseTheWorld(void);
extern void SendGotGlobal(void);
extern void ProcessGotGlobal(void);
extern void SendHonka(void);
extern void ProcessHonka(void);

extern void RemoteSyncHost(void);
extern void RemoteSyncClient(void);
extern void ProcessPlayerJoining(void);
extern void ProcessPlayerLeaving(void);
  #ifndef XBOX_NOT_YET_IMPLEMENTED  // we don't support host migration yet
extern void ProcessBecomeHost(void);
  #endif // ! XBOX_NOT_YET_IMPLEMENTED
extern unsigned long WINAPI ReceiveThread(void *param);
extern void RefreshSessions(void);
extern void UpdateMessages(void);




// globals
//$TODO -- CLEANUP GLOBAR-VAR DECLARATIONS (ADD MISSING ONES; AND REMOVE NONEXISTENT ONES)

extern DWORD LocalPlayerID;
extern DWORD FromID, ServerID;  //$NOTE: type was DPID originally  //$REVISIT: using FromID and ServerID is kind of ugly (leftover from porting Acclaim DPlay code); I'd like to remove them eventually.

extern long  PlayerCount;
extern char  SendMsgBuffer[];
extern char* RecvMsgBuffer;
extern MESSAGE_HEADER* SendHeader;
extern MESSAGE_HEADER* ReceiveHeader;
extern long MessageQueueSize;

extern NET_PLAYER  PlayerList[];
extern NET_SESSION SessionList[];

extern char   SessionCount;       //$CMP_NOTE: (might want to un-expose this) removal of ConnectionMenu means we had to access this from MainMenu, in another file
extern float  SessionRequestTime; //$CMP_NOTE: (might want to un-expose this) removal of ConnectionMenu means we had to access this from MainMenu, in another file

extern SOCKET soUDP;              //$CMP_NOTE: (might want to un-expose this) only used in SendLocalCarData()
extern const SOCKADDR_IN SoAddrBroadcast; //$CMP_NOTE: (might want to un-expose this) only used in SendLocalCarData()

extern BOOL bGameStarted;

extern long NextPositionReady, NextPacketReady, NextSyncReady, AllPlayersReady, HostQuit, LocalPlayerReady;
extern REAL NextPacketTimer, NextSyncTimer, NextPositionTimer;
extern REAL AllReadyTimeout;
extern long NextSyncMachine;
extern char SessionPick;

extern DWORD dwLocalPlayerCount;
#define      INVALID_PLAYER_ID  (0)


#endif // NETWORK_H

