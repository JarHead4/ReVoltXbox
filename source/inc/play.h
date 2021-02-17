
#ifndef PLAY_H
#define PLAY_H

#include "dx.h"
#include "main.h"

// macros

#define MAX_HOST_NAME 1024

#define CONNECTION_MAX 16
#define SESSION_MAX 8
#define MAX_SESSION_NAME 34
#define MAX_CONNECTION_NAME 128
#define MAX_LEVEL_DIR_NAME 16

#define MESSAGE_CONTENTS_CAR_TIME 1
#define MESSAGE_CONTENTS_CAR_POS 2
#define MESSAGE_CONTENTS_CAR_QUAT 4
#define MESSAGE_CONTENTS_CAR_VEL 8
#define MESSAGE_CONTENTS_CAR_ANGVEL 16
#define MESSAGE_CONTENTS_CAR_CONTROL 32

#define MESSAGE_PRIORITY_NORMAL 0
#define MESSAGE_PRIORITY_CAR 1

enum {
	MESSAGE_GAME_STARTED,
	MESSAGE_CAR_DATA,
	MESSAGE_PING_REQUEST,
	MESSAGE_PING_RETURN,
	MESSAGE_PLAYER_READY,
	MESSAGE_SYNC_TIMERS1,
	MESSAGE_SYNC_TIMERS2,
	MESSAGE_SYNC_TIMERS3,
};

typedef struct {
	void *Ptr;
	char Name[MAX_CONNECTION_NAME];
} DP_CONNECTION;

typedef struct {
	GUID Guid;
	DWORD Flags;
	char Name[MAX_SESSION_NAME];
} DP_SESSION;

typedef struct {
	long CarID;
} DP_PLAYER_DATA;

typedef struct {
	DPID PlayerID;
	DPCAPS Caps;
	long Ping;
	char Name[MAX_PLAYER_NAME];
	DP_PLAYER_DATA Data;
} DP_PLAYER;

typedef struct {
	unsigned short Type, Contents;
} MESSAGE_HEADER;

typedef struct {
	long GridNum, CarID;
	DPID PlayerID;
	char Name[MAX_PLAYER_NAME];
} PLAYER_START_DATA;

typedef struct {
	long PlayerNum;
	char LevelDir[MAX_LEVEL_DIR_NAME];
	PLAYER_START_DATA PlayerData[MAX_NUM_PLAYERS];
} START_DATA;

typedef struct {
	unsigned char Mask[4], IP[4];
} LEGAL_IP;

// prototypes

extern void LobbyRegister(void);
extern bool InitPlay(void);
extern void KillPlay(void);
extern bool InitConnection(char num);
extern bool CreateSession(char *name);
extern void ListSessions(void);
extern void StopSessionEnum(void);
extern bool JoinSession(char num);
extern bool CreatePlayer(char *name, long server);
extern void ListPlayers(GUID *guid);
extern void DisplayPlayers(void);
extern void TransmitMessage(char *buff, short size, DPID to, long pri);
extern void TransmitMessageGuaranteed(char *buff, short size, DPID to, long pri);
extern void CancelPriority(long pri);
extern char GetRemoteMessages(void);
extern void ProcessCarMessage(void);
extern void ProcessPersonalMessage(void);
extern void ProcessSystemMessage(void);
extern void RequestPings(void);
extern void ProcessPingRequest(void);
extern void ProcessPingReturn(void);
extern void ProcessPlayerReady(void);
extern void ProcessSyncTimers1(void);
extern void ProcessSyncTimers2(void);
extern void ProcessSyncTimers3(void);
extern BOOL FAR PASCAL EnumConnectionsCallback(LPCGUID lpguidSP, LPVOID lpConnection, DWORD dwConnectionSize, LPCDPNAME lpName, DWORD dwFlags, LPVOID lpContext);
extern BOOL FAR PASCAL EnumSessionsCallback(LPCDPSESSIONDESC2 lpSessionDesc, LPDWORD lpdwTimeOut, DWORD dwFlags, LPVOID lpContext);
extern BOOL FAR PASCAL EnumPlayersCallback(DPID dpId, DWORD dwPlayerType, LPCDPNAME lpName, DWORD dwFlags, LPVOID lpContext);
extern void ConnectionMenu(void);
extern void GetSessionName(void);
extern void LookForSessions(void);
extern void HostWait(void);
extern void ClientWait(void);
extern char GetHostDetails(void);
extern char CheckLegalIP(void);
extern void RemoteSync(void);

// globals

extern IDirectPlayLobby3 *Lobby;
extern IDirectPlay4A *DP;
extern DPID FromID, ToID, LocalPlayerID, ServerID;
extern DP_PLAYER PlayerList[];
extern long PlayerCount;
extern char ReceiveBuff[], TransmitBuff[];
extern START_DATA StartData;

#endif
