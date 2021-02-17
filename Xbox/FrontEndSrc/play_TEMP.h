
#ifndef PLAY_H
#define PLAY_H

#include "dx.h"
#include "LevelInfo.h"

// macros

#define USE_GROUPS 1

#define MAX_HOST_NAME 1024
#define MAX_HOST_COMPUTER 33

#define CONNECTION_MAX 16
#define SESSION_MAX 8
#define MAX_SESSION_NAME 128
#define MAX_CONNECTION_NAME 128

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
    DP_CLIENT_PLAYER,
    DP_SERVER_PLAYER,
    DP_SPECTATOR_PLAYER,
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
    MESSAGE_JOIN_INFO,
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
};

typedef struct {
    void *Ptr;
    GUID Guid;
    char Name[MAX_CONNECTION_NAME];
} DP_CONNECTION;

typedef struct {
    GUID Guid;
    DWORD Flags;
    DWORD Started;
    DWORD GameType;
    DWORD Version;
    DWORD RandomCars;
    DWORD RandomTrack;
    DWORD PlayerNum;
    char Name[MAX_SESSION_NAME];
} DP_SESSION;

#define CAR_NAMELEN 20

typedef struct {
    long Ready, Cheating;
    char CarName[CAR_NAMELEN];
} DP_PLAYER_DATA;

typedef struct {
    long Spectator, Host;
    DPID PlayerID;
//  DPCAPS Caps;
    char Name[MAX_PLAYER_NAME];
    DP_PLAYER_DATA Data;
} DP_PLAYER;

typedef struct {
    unsigned short Type, Flag;
} MESSAGE_HEADER;

typedef struct {
    unsigned long RaceTime;
    char LevelDir[MAX_LEVEL_DIR_NAME];
} JOIN_INFO;

typedef struct {
    unsigned char Mask[4], IP[4];
} LEGAL_IP;

typedef struct {
    unsigned long Time;
} RACE_TIME_INFO;

typedef struct {
    long Seed, NewTrack;
    char CarName[CAR_NAMELEN];
    char LevelDir[MAX_LEVEL_DIR_NAME];
} RESTART_DATA;

struct object_def;

// prototypes

extern void LobbyInit(void);
extern void LobbyConnect(void);
extern void LobbyFree(void);
extern bool InitPlay(void);
extern void KillPlay(void);
extern bool InitConnection(char num);
extern bool CreateSession(char *name);
extern bool SetSessionDesc(char *name, char *track, long started, long gametype, long randomcars, long randomtrack);
  #ifndef XBOX_DISABLE_NETWORK
extern BOOL FAR PASCAL EnumGroupCallback(DPID dpId, DWORD dwPlayerType, LPCDPNAME lpName, DWORD dwFlags, LPVOID lpContext);
  #endif // ! XBOX_DISABLE_NETWORK
extern void ListSessions(void);
extern void StopSessionEnum(void);
extern bool JoinSession(char num);
extern bool CreatePlayer(char *name, long mode);
extern void SetPlayerName(char *name);
extern void SetPlayerData(void);
extern void ListPlayers(GUID *guid);
extern void UpdatePacketInfo(void);
extern void CheckAllPlayersReady(void);
extern void QueueMessage(MESSAGE_HEADER *buff, short size);
extern void SendMessage(MESSAGE_HEADER *buff, short size, DPID to);
extern void SendMessageGuaranteed(MESSAGE_HEADER *buff, short size, DPID to);
extern char GetRemoteMessages(void);
extern void ProcessPersonalMessage(void);
extern void ProcessSystemMessage(void);
extern void SendGameStarted(void);
extern void ProcessGameStarted(void);
extern void SendSyncRequest(void);
extern void ProcessSyncRequest(void);
extern void ProcessSyncReply(void);
extern void SendGameLoaded(void);
extern void ProcessGameLoaded(void);
extern void SendCountdownStart(void);
extern void ProcessCountdownStart(void);
extern void SendJoinInfo(DPID id);
extern void ProcessJoinInfo(void);
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
extern unsigned long GetSendQueue(DPID id);
extern unsigned long GetReceiveQueue(DPID id);
  #ifndef XBOX_DISABLE_NETWORK
extern BOOL FAR PASCAL EnumConnectionsCallback(LPCGUID lpguidSP, LPVOID lpConnection, DWORD dwConnectionSize, LPCDPNAME lpName, DWORD dwFlags, LPVOID lpContext);
extern BOOL FAR PASCAL EnumSessionsCallback(LPCDPSESSIONDESC2 lpSessionDesc, LPDWORD lpdwTimeOut, DWORD dwFlags, LPVOID lpContext);
extern BOOL FAR PASCAL EnumPlayersCallback(DPID dpId, DWORD dwPlayerType, LPCDPNAME lpName, DWORD dwFlags, LPVOID lpContext);
  #endif // ! XBOX_DISABLE_NETWORK
extern void ConnectionMenu(void);
extern void GetSessionName(void);
extern void LookForSessions(void);
extern void ClientJoinLate(void);
extern void HostWait(void);
extern void ClientWait(void);
extern char GetHostDetails(void);
//$REMOVEDextern char CheckLegalIP(void);
extern long GetIPString(char *str);
extern void RemoteSyncHost(void);
extern void RemoteSyncClient(void);
extern void ProcessPlayerJoining(void);
extern void ProcessPlayerLeaving(void);
extern void ProcessBecomeHost(void);
extern unsigned long WINAPI ReceiveThread(void *param);
extern void RefreshSessions(void);
extern void UpdateMessages(void);

// globals

  #ifndef XBOX_DISABLE_NETWORK
extern IDirectPlayLobby3 *Lobby;
extern IDirectPlay4A *DP;
  #endif // ! XBOX_DISABLE_NETWORK
extern DPID FromID, ToID, LocalPlayerID, ServerID, GroupID;
extern DP_PLAYER PlayerList[];
extern REAL PacketsPerSecond;
extern REAL NextPacketTimer, NextSyncTimer, NextPositionTimer, AllReadyTimeout;
extern long SessionFlag;
extern long PlayerCount, NextPositionReady, NextPacketReady, NextSyncReady, NextSyncPlayer, AllPlayersReady, GameStarted, HostQuit, LocalPlayerReady;
extern float PlayersRequestTime, SessionRequestTime;
extern MESSAGE_HEADER *SendHeader, *ReceiveHeader;
extern char *MessageQueue;
extern long MessageQueueSize, TotalDataSent;
  #ifndef XBOX_DISABLE_NETWORK
extern DP_CONNECTION Connection[CONNECTION_MAX];
extern char ConnectionCount, SessionCount, SessionPick;
extern DPSESSIONDESC2 Session;
extern DP_SESSION SessionList[SESSION_MAX];
extern DPLCONNECTION *LobbyConnection;
  #endif // ! XBOX_DISABLE_NETWORK

#endif
