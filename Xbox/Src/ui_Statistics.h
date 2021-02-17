//-----------------------------------------------------------------------------
// File: ui_Statistics.h
//
// Desc: 
//
// Re-Volt (Generic) Copyright (c) Probe Entertainment 1998
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef UI_STATISTICS_H
#define UI_STATISTICS_H

//$REVISIT: this is ugly!!  User shouldn't have to worry about this dependency.
#ifndef NET_STATISTICS_H
#error "Must include net_Statistics.h before ui_Statistics.h"
#endif 

//$REVISIT: this is ugly!!  User shouldn't have to worry about this dependency.
#ifndef UI_ANIMATION_H
#error "Must include ui_Animation.h before ui_Statistics.h"
#endif 

extern MENU Menu_StatisticsMenu;
extern MENU Menu_StatisticsRacerRankList;
extern MENU Menu_StatisticsRacerDetails;
extern MENU Menu_StatisticsTrackRankList;
extern MENU Menu_StatisticsTrackDetails;



extern MENU Menu_StatsRankList;


class CUIStatsPanelScreen : public CUIScreen
{
public:
    CUIRCStatPanel          m_RC;

    CUIStatsPanelScreen()
    {
        m_RC.Init( this);
    }

    virtual VOID HandleEnterFromParent()    { m_RC.BeginEnterFromSides(); }
    virtual VOID HandleEnterFromChild()     { m_RC.BeginEnterFromBottom(); }

    virtual VOID HandleExitToParent()       
    { 
        CancelPendingRequests();
        m_RC.BeginExitToSides(); 
    }

    virtual VOID HandleExitToChild()        
    { 
        CancelPendingRequests();
        m_RC.BeginExitToBottom(); 
    }

    virtual VOID CancelPendingRequests() = 0;    // terminate any outstanding XONLINE tasks
};

//-----------------------------------------------------------------------------
// The Racer Summary state engine
//-----------------------------------------------------------------------------
class CUIStatsUserSummary : public CUIStatsPanelScreen
{
public:
    XONLINE_USER            m_User;
    DWORD                   m_dwLeaderBoardID;
    bool                    m_bBattleMode;
    CStatRacer*             m_pRaceStats;
    CStatBattler*           m_pBattleStats;

    enum
    {
            STATE_BEGIN = STATEENGINE_STATE_BEGIN,
        STATE_WAIT_FOR_RESULTS,
        STATE_STATS_ERROR,
        STATE_MAINLOOP,
    };


    CUIStatsUserSummary( );
    virtual HRESULT Process();
    virtual WCHAR*  DebugGetName()   { return L"StatsRacerSummary"; }
    virtual bool HandleInput( MenuInputTypeEnum input);
    virtual void Draw();

    HRESULT Init( XONLINE_USER *pUser, DWORD dwLeaderBoardID );

    virtual VOID CancelPendingRequests()
    {
        // terminate any outstanding XONLINE tasks
        if ( m_pRaceStats )
            m_pRaceStats->CancelPendingRequest();

        if ( m_pBattleStats )
            m_pBattleStats->CancelPendingRequest();
    }
};


//-----------------------------------------------------------------------------
// The Stats Racer Rank List state engine
//-----------------------------------------------------------------------------
class CUIStatsRacerRankList : public CUIStatsPanelScreen
{
public:
    CStatPlayerRanks   *m_pDisplayedRanks;
    CStatPlayerRanks    m_RacingRanks;
    CStatPlayerRanks    m_BattleRanks;
    CStatFriendsList    m_StatFriends;

    CUIListHelper       m_List;
    int                 m_nLocalPlayers;

    //CUIStateEngine*     m_pStatsChild;
    CUIStatsUserSummary         m_UIUserSummary;

    enum
    {
        STATE_BEGIN = STATEENGINE_STATE_BEGIN,
        STATE_REQUEST_FRIENDS_LIST,
        STATE_WAIT_FOR_FRIENDS_LIST,
        STATE_REQUEST_STATS,
        STATE_WAIT_FOR_STATS,
        STATE_GOT_STATS,
        STATE_STATS_ERROR,
        STATE_MAINLOOP,
    };


    CUIStatsRacerRankList();
    virtual HRESULT Process();
    virtual WCHAR*  DebugGetName()   { return L"StatsRankList"; }
    virtual bool HandleInput( MenuInputTypeEnum input);
    virtual void Draw();

    HRESULT Init( DWORD dwLeaderBoardID );
    virtual VOID CancelPendingRequests()
    {
        // terminate any outstanding XONLINE tasks
        m_RacingRanks.CancelPendingRequest();
        m_BattleRanks.CancelPendingRequest();
    }

};

//-----------------------------------------------------------------------------
// The Stats World Leader Racer Rank List state engine
//-----------------------------------------------------------------------------
class CUIStatsWorldLeaderRankList : public CUIStatsPanelScreen
{
public:
    CStatWorldLeaderRanks      *m_pDisplayedRanks;
    CStatWorldLeaderRanks       m_RacingRanks;
    CStatWorldLeaderRanks       m_BattleRanks;

    CUIListHelper               m_List;

    //CUIStateEngine             *m_pStatsChild;
    CUIStatsUserSummary         m_UIUserSummary;

    enum
    {
        STATE_BEGIN = STATEENGINE_STATE_BEGIN,
        STATE_REQUEST_STATS,
        STATE_WAIT_FOR_RESULTS,
        STATE_GOT_STATS,
        STATE_STATS_ERROR,
        STATE_MAINLOOP,
    };


    CUIStatsWorldLeaderRankList();
    virtual HRESULT Process();
    virtual WCHAR*  DebugGetName()   { return L"StatsWorldLeaderRankList"; }
    virtual bool HandleInput( MenuInputTypeEnum input);
    virtual void Draw();

    HRESULT Init( DWORD dwLeaderBoardID );

    virtual VOID CancelPendingRequests()
    {
        // terminate any outstanding XONLINE tasks
        m_RacingRanks.CancelPendingRequest();
        m_BattleRanks.CancelPendingRequest();
    }
};




//-----------------------------------------------------------------------------
// The Racer's Track Rank Stats List state engine
//-----------------------------------------------------------------------------
class CUIStatsRacerTrackRankList : public CUIStatsPanelScreen
{
public:
    XONLINE_USER            m_User;
    bool                    m_bBattleMode;
    CStatUserTrackRanks*    m_pStatRanks;
    CUIListHelper           m_List;
    CUIStatsUserSummary     m_UIDetail;

    enum
    {
        STATE_BEGIN = STATEENGINE_STATE_BEGIN,
        STATE_WAIT_FOR_RESULTS,
        STATE_GOT_STATS,
        STATE_STATS_ERROR,
        STATE_MAINLOOP,
    };

    CUIStatsRacerTrackRankList( );
    virtual HRESULT Process();
    virtual WCHAR*  DebugGetName()   { return L"StatsRacerTrackRankList"; }
    virtual bool HandleInput( MenuInputTypeEnum input);
    virtual void Draw();

    HRESULT Init( XONLINE_USER *pUser, bool bBattleMode );

    virtual VOID CancelPendingRequests()
    {
        // terminate any outstanding XONLINE tasks
        if (m_pStatRanks)
        {
            m_pStatRanks->CancelPendingRequest();
        }
    }
};



//-----------------------------------------------------------------------------
// The StatisticsMenu state engine
//-----------------------------------------------------------------------------
class CUIStatisticsMenu : public CUIStateEngine
{
public:
//    CUIStateEngine* m_pStatsChild;
    CUIStatsRacerRankList       m_UIRacerRankList;
    CUIStatsWorldLeaderRankList m_UIWorldLeaderRankList;
    CUIStatsUserSummary         m_UIUserSummary;

    virtual HRESULT Process();
    virtual WCHAR*  DebugGetName()   { return L"StatisticsMenu"; };

    virtual VOID HandleExitToParent();
};

extern CUIStatisticsMenu g_UIStatisticsMenu;



#endif // UI_STATISTICS_H

