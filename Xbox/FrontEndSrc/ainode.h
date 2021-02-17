//-----------------------------------------------------------------------------
// File: AINode.h
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef AINODE_H
#define AINODE_H

//
// Defines and macros
//

#define MAX_AINODES 1024
#define MAX_AINODE_LINKS 2
#define MAX_AINODE_SPEED 100
#define MAX_AINODE_PRIORITY 1


//
// AI Node Types
//
enum
{
    AIN_TYPE_RACINGLINE = 0,
    AIN_TYPE_PICKUP,
    AIN_TYPE_STAIRS,
    AIN_TYPE_BUMPY,
    AIN_TYPE_SLOWDOWN_25,
    AIN_TYPE_SOFTSUSPENSION,
    AIN_TYPE_JUMPWALL,
    AIN_TYPE_TITLESCR_SLOWDOWN,
    AIN_TYPE_TURBOLINE,
    AIN_TYPE_LONGPICKUP,
    AIN_TYPE_SHORTCUT,
    AIN_TYPE_LONGCUT,
    AIN_TYPE_BARRELBLOCK,
    AIN_TYPE_OFFTHROTTLE,
    AIN_TYPE_OFFTHROTTLEPETROL,
    AIN_TYPE_WILDERNESS,
    AIN_TYPE_SLOWDOWN_15,
    AIN_TYPE_SLOWDOWN_20,
    AIN_TYPE_SLOWDOWN_30,

    AIN_TYPE_NUM,
};

// AI node Link Flags
enum
{
    AIN_LF_WALL_LEFT        = 0x01,
    AIN_LF_WALL_RIGHT       = 0x02,
};


//
// Typedefs and structures
//

// Used for left & right points of node
typedef struct
{
    long    Speed;
    VEC     Pos;                                    // Position on node edge

} FILE_ONE_AINODE;

typedef struct {
    char Priority, StartNode;
    char flags[2];
    REAL RacingLine, FinishDist, OvertakingLine, fpad;
    long RacingLineSpeed, CentreSpeed;
    long Prev[MAX_AINODE_LINKS];
    long Next[MAX_AINODE_LINKS];
    FILE_ONE_AINODE Node[2];
} FILE_AINODE;

// Used for left & right points of node
typedef struct
{
    VEC     Pos;                                    // Position on node edge
#ifndef _PSX
    long    Speed;
#endif

} ONE_AINODE;


//
typedef struct _AINODE_LINKINFO
{
    VEC     forwardVec;                             // Forward vector
    VEC     rightVec;                               // Right vector
    REAL    dist;
    REAL    invDist;                                    // Dist to the link node
    REAL    speed;                                  // Speed rating of this racing line's node turning onto linked nodes
    char    dir;                                    // Direction angle of racing line along x/z plane (0-255 degrees)
    char    incline;                                // Incline angle of slope (0-255)
    char    flags;                                  // Flags
    char    pad3;
    REAL    boundsMin[2];                           // Bounds min
    REAL    boundsMax[2];                           // Bounds max
    PLANE   planeEdge[4];                           // THIS IS TEMPORARY !!!!!!!!! USED FOR EDITOR

} AINODE_LINKINFO;


// AI Node
typedef struct _AINODE
{
    char    Priority, StartNode;
    char    CheckNext[MAX_AINODE_LINKS];            // !MT! Caution: If MAX_AINODE_LINKS becomes > 2 then these need to be relocated to account for long word alignment
    REAL    RacingLine, OvertakingLine;
    REAL    FinishDist;
    long    RacingLineSpeed, CentreSpeed;
    struct _AINODE  *Prev[MAX_AINODE_LINKS];
    struct _AINODE  *Next[MAX_AINODE_LINKS];
    ONE_AINODE      Node[2];

    long    ZoneID, ZoneBBox;                       // Zone ID, and BBOX of that zone
    VEC     Centre;
    REAL    trackWidth;                             // Width of track at node on left & right sides
    REAL    trackWidthL;                            // Width of track on left of racing line
    REAL    trackWidthR;                            // Width of track on right of racing line
    REAL    trackHotDist[2];                        // 'hot zone' left and right distances to racing line

    struct _AINODE *ZonePrev;
    struct _AINODE *ZoneNext;

//  AINODE_LINKINFO linkInfo[MAX_AINODE_LINKS];     // Link info
    AINODE_LINKINFO link;                           // Link info to next node

} AINODE;

typedef struct _AINODE_ZONE 
{
    long Count;
    AINODE *FirstNode;
} AINODE_ZONE;

//
// External variables
//

extern AINODE *AiNode;
extern AINODE_ZONE *AiNodeZone;
extern short AiNodeNum;
extern short AiLinkNodeNum;
extern long AiStartNode;
extern REAL AiNodeTotalDist;

//
// External function prototypes
//

#ifdef _PC
extern void     LoadAiNodes(char *file);
#endif
extern void     FreeAiNodes(void);
extern void ZoneAiNodes(void);

struct PlayerStruct;

extern AINODE *AIN_NearestNode(struct PlayerStruct *Player, REAL *Dist);
extern AINODE *AIN_GetForwardNode(struct PlayerStruct *Player, REAL MinDist, REAL *Dist);
extern AINODE *AIN_GetForwardNodeInRange(struct PlayerStruct *Player, REAL MaxDist, REAL *Dist);
extern AINODE *AIN_IsPlayerInNodeRecurseN(struct PlayerStruct *pPlayer, AINODE *pNode, long iRoute, REAL *pDist);
extern AINODE *AIN_IsPlayerInNodeRecurseP(struct PlayerStruct *pPlayer, AINODE *pNode, long iRoute, REAL *pDist);

extern REAL CAI_GetAngleToRacingLine(struct PlayerStruct *pPlayer, VEC *pFVec, VEC *pRVec, REAL* pSide);
extern AINODE *AIN_FindNodePlayerIsIn(struct PlayerStruct *pPlayer);
extern AINODE *AIN_FindZoneNodePlayerIsIn(struct PlayerStruct *pPlayer);
extern AINODE *AIN_FindNodeAhead(struct PlayerStruct *pPlayer, REAL dist, VEC *pPos);
extern AINODE *AIN_FindNodeAheadNoWall(struct PlayerStruct *pPlayer, REAL dist, VEC *pPos);

extern int AIN_IsPointInNodeBounds(VEC *pPos, AINODE *pNode, int iRoute);
extern AINODE *AIN_FindFirstValidNodeIDN(AINODE *pNode);
extern AINODE *AIN_FindFirstValidNodeIDP(AINODE *pNode);

#endif // AINODE_H

