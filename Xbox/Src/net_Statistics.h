//-----------------------------------------------------------------------------
// File: net_Statistics.h
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------

#ifndef NET_STATISTICS_H
#define NET_STATISTICS_H

#include "RatingEquation.h"
#include "InitPlay.h"

#include "ui_ShowMessage.h" // for displaying XOnline errors to player

// $REVISIT: remove stat code that retrieves min and min leaderboard stats from servers
#define XSTATS_NO_MINMAX_RATING_REQUEST

// leaderboard id's as stopred on server
// changing these id's will invalidate server data
const DWORD STAT_LB_RACING_OVERALL  =  1;
const DWORD STAT_LB_BATTLE_OVERALL  =  2;
const DWORD STAT_LB_FIRST_TRACK     = 1000;

inline LeaderBoardID( int nLevel )
{
    Assert( (nLevel >= 0) && (nLevel < LEVEL_NSHIPPED_LEVELS) );
    return nLevel + STAT_LB_FIRST_TRACK;
}

inline IsTrack( DWORD dwLeaderBoardID )
{
    return (dwLeaderBoardID >= STAT_LB_FIRST_TRACK) && (dwLeaderBoardID < STAT_LB_FIRST_TRACK + LEVEL_NSHIPPED_LEVELS);
}

inline TrackLevel( DWORD dwLeaderBoardID )
{
    Assert( IsTrack(dwLeaderBoardID ) );
    return dwLeaderBoardID - STAT_LB_FIRST_TRACK;
}

inline bool IsBattleStat( DWORD dwLeaderBoardID )
{
    return ( STAT_LB_BATTLE_OVERALL == dwLeaderBoardID ) || 
            ( IsTrack( dwLeaderBoardID ) && (TRACK_TYPE_BATTLE == DefaultLevelInfo[ TrackLevel( dwLeaderBoardID ) ].TrackType ));
}


// stat attribute id's as stored in server, 
// changing these id's will invalidate server data
const WORD STAT_RACES_STARTED           = 1;
const WORD STAT_RACES_COMPLETED         = 2;
const WORD STAT_RACES_WON               = 3;
const WORD STAT_RACES_TOTAL_SECONDS     = 4;
const WORD STAT_RACES_TOTAL_METERS      = 5;
const WORD STAT_RACES_BESTSPLIT_SECONDS = 6;
const WORD STAT_RACES_BESTSPLIT_CAR     = 7;

// stat attribute id's as stored in server, 
// changing these id's will invalidate server data
const WORD STAT_BATTLES_STARTED         = 1;
const WORD STAT_BATTLES_COMPLETED       = 2;
const WORD STAT_BATTLES_WON             = 3;
const WORD STAT_BATTLES_TOTAL_IT_SECONDS= 4;
const WORD STAT_BATTLES_TOTAL_SECONDS   = 5;


inline void ComputeHMS( DWORD dwTotalMSeconds, DWORD &dwHours, DWORD &dwMinutes, DWORD &dwSeconds)
{
    dwSeconds = dwTotalMSeconds / 1000;
    dwHours = dwSeconds / ( 60 * 60 );

    if ( dwHours )
        dwSeconds %= dwHours * (60 * 60);

    dwMinutes = dwSeconds / 60;

    if ( dwMinutes )
        dwSeconds %= dwMinutes * 60;
}

// percentile of i in n = ( n - i) / (n - 1)
inline float ComputeRankPercentile( LONG lNumPlayers, LONG lRank )
{
    if ( 0 == lNumPlayers )
        return 0.0f;

    return (float)(lNumPlayers - lRank) * 100.0f / (lNumPlayers - 1);
}

class CXOnlineTask
{
public:
    XONLINETASK_HANDLE  m_hTask;
    HRESULT             m_hrStatus;

    virtual HRESULT TaskContinue() = 0;

};

typedef std::vector< CXOnlineTask* > XOnlineTasks;

class CXOnlineTasks : public XOnlineTasks
{
public:
    void TaskContinueAll()
    {
        for ( iterator pTask = begin(); pTask < end(); pTask++ )
        {
            (*pTask)->TaskContinue();

            if ( NULL == (*pTask)->m_hTask)
            {
                // remove the task from the list, nothing remain to do.
                pTask = erase( pTask );
            }
        }

    }
};

extern CXOnlineTasks g_XOnlineTasks;

#define XONLINESTATSERROR( hr) ShowXStatsError( __LINE__, hr)
extern void ShowXStatsError( int nLineNumber, HRESULT hr );


class CStatFriendsList : public CXOnlineTask
{
public:
    DWORD           m_dwNumFriends;
    XONLINE_FRIEND  m_Friends[MAX_FRIENDS];

public:
    HRESULT StartRequest()
    {
        Assert( NULL == m_hTask );  // can only process 1 task at a time

        m_dwNumFriends = 0;
        memset( m_Friends, 0, sizeof( m_Friends[MAX_FRIENDS] ) );
#ifdef XONLINE_OFFLINE
        m_hrStatus = S_OK;
        m_hTask = (XONLINETASK_HANDLE)-1;
#else
        m_hrStatus = XOnlineFriendsEnumerate( 0, NULL, &m_hTask );
#endif
        // OnlineTasks_Add( handle );  I want to process my result with using single instance global structures.

        return m_hrStatus;
    }

    HRESULT CancelPendingRequest()
    {
        if ( NULL == m_hTask )
            return S_FALSE;  // none pending
    
        XOnlineFriendsEnumerateFinish( m_hTask );
        do
        {
            m_hrStatus = XOnlineTaskContinue( m_hTask );
            assert( m_hrStatus != XONLINETASK_S_RESULTS_AVAIL );
        } while( XONLINETASK_S_RUNNING == m_hrStatus );

        m_hrStatus = XOnlineTaskClose( m_hTask);
        m_hTask = NULL;

        return m_hrStatus;
    }

    HRESULT TaskContinue()
    {
#ifdef XONLINE_OFFLINE
        Assert( (XONLINETASK_HANDLE)-1 == m_hTask );
        m_hrStatus = S_OK;

        m_hTask = NULL;
#else
        Assert( m_hTask );
        m_hrStatus = XOnlineTaskContinue( m_hTask );

        if (XONLINETASK_S_RESULTS_AVAIL == m_hrStatus)
        {
            // $SINGLEPLAYER need to get friends for all users or just spcified user, hardcaded for now
            m_dwNumFriends = XOnlineFriendsGetLatest( 0, 
                                                      MAX_FRIENDS, 
                                                      m_Friends );

            CancelPendingRequest();
        }
        else if ( FAILED( m_hrStatus ))
        {
            XONLINESTATSERROR( m_hrStatus );
        }

#endif

        return m_hrStatus;
    }


};

// May only contain XONLINE_STAT members as this is used at times as an array
class OverallStatsRecord
{
public:
    XONLINE_STAT Rating;            // writeable
    XONLINE_STAT RacesCompleted;    // writeable
    XONLINE_STAT LeaderBoardSize;   // read-only
    XONLINE_STAT Rank;              // read-only

    enum { NUMBER_OF_WRITEABLE_STATS = 2 };
    enum { NUMBER_OF_STATS = 4 };

    OverallStatsRecord()
    {
        Assert( sizeof(OverallStatsRecord)/sizeof(XONLINE_STAT) == NUMBER_OF_STATS );
        Clear();
    }

    //PXONLINE_STAT operator&() { return (PXONLINE_STAT)this; };

    void Clear()
    {
        // init the field data
        Rating.wID              = XONLINE_STAT_RATING;
        Rating.llValue          = 0;

        RacesCompleted.wID      = STAT_RACES_COMPLETED;
        RacesCompleted.lValue   = 0;

        LeaderBoardSize.wID     = XONLINE_STAT_LEADERBOARD_SIZE;
        LeaderBoardSize.lValue  = 0;

        Rank.wID                = XONLINE_STAT_RANK;
        Rank.lValue             = 0;

        ResetTypes();
    }

    void ResetTypes()
    {
        Rating.type             = XONLINE_STAT_LONGLONG;
        RacesCompleted.type     = XONLINE_STAT_LONG;
        LeaderBoardSize.type    = XONLINE_STAT_LONG;
        Rank.type               = XONLINE_STAT_LONG;
    }

    float ComputeRankPercentile()
    {
        return ::ComputeRankPercentile( LeaderBoardSize.lValue, Rank.lValue);
    }

    void RandomFillData()
    {
            LeaderBoardSize.lValue    = 3591 * rand() / RAND_MAX;
            Rank.lValue               = 876 * rand() / RAND_MAX;
            Rating.llValue            = 234567 * rand() / RAND_MAX;
            RacesCompleted.lValue     = 850 * rand() / RAND_MAX;
    }
};

// May only contain XONLINE_STAT members as this is used at times as an array
class RacerStatsRecord
{
public:
    XONLINE_STAT Rating;            // writeable
    XONLINE_STAT RacesStarted;      // writeable
    XONLINE_STAT RacesCompleted;    // writeable
    XONLINE_STAT RacesWon;          // writeable
    XONLINE_STAT TotalMSeconds;      // writeable
    XONLINE_STAT TotalMeters;       // writeable
    XONLINE_STAT BestSplitTime;     // writeable
    XONLINE_STAT BestSplitCar;      // writeable
    XONLINE_STAT LeaderBoardSize;   // read-only
    XONLINE_STAT Rank;              // read-only

    enum { NUMBER_OF_WRITEABLE_STATS = 8 };
    enum { NUMBER_OF_STATS = 10 };

    RacerStatsRecord()
    {
        Assert( sizeof(RacerStatsRecord)/sizeof(XONLINE_STAT) == NUMBER_OF_STATS );
        Clear();
    }

    //PXONLINE_STAT operator&() { return (PXONLINE_STAT)this; };

    void Clear()
    {
        // init the field data
        Rating.wID              = XONLINE_STAT_RATING;
        Rating.llValue          = 0;

        RacesStarted.wID        = STAT_RACES_STARTED;
        RacesStarted.lValue     = 0;

        RacesCompleted.wID      = STAT_RACES_COMPLETED;
        RacesCompleted.lValue   = 0;

        RacesWon.wID            = STAT_RACES_WON;
        RacesWon.lValue         = 0;

        TotalMSeconds.wID        = STAT_RACES_TOTAL_SECONDS;
        TotalMSeconds.lValue     = 0;

        TotalMeters.wID         = STAT_RACES_TOTAL_METERS;
        TotalMeters.lValue      = 0;

        BestSplitTime.wID       = STAT_RACES_BESTSPLIT_SECONDS;
        BestSplitTime.lValue    = 0;

        BestSplitCar.wID        = STAT_RACES_BESTSPLIT_CAR;
        BestSplitCar.lValue     = 0;

        LeaderBoardSize.wID     = XONLINE_STAT_LEADERBOARD_SIZE;
        LeaderBoardSize.lValue  = 0;

        Rank.wID                = XONLINE_STAT_RANK;
        Rank.lValue             = 0;

        ResetTypes();
    }

    void ResetTypes()
    {
        Rating.type             = XONLINE_STAT_LONGLONG;
        RacesStarted.type       = XONLINE_STAT_LONG;
        RacesCompleted.type     = XONLINE_STAT_LONG;
        RacesWon.type           = XONLINE_STAT_LONG;
        TotalMSeconds.type       = XONLINE_STAT_LONG;
        TotalMeters.type        = XONLINE_STAT_LONG;
        BestSplitTime.type      = XONLINE_STAT_LONG;
        BestSplitCar.type       = XONLINE_STAT_LONG;
        LeaderBoardSize.type    = XONLINE_STAT_LONG;
        Rank.type               = XONLINE_STAT_LONG;
    }

    long ComputeRacesLost()
    {
        return RacesCompleted.lValue - RacesWon.lValue;
    }

    long ComputeRacesAbandoned()
    {
        return RacesStarted.lValue - RacesCompleted.lValue;
    }


    long ComputeWinLossPercentage()
    {
        long lRacesLost = ComputeRacesLost();

        if ( RacesCompleted.lValue )
            return RacesWon.lValue * 100 / RacesCompleted.lValue;

        return -1;
    }

    float ComputeRankPercentile()
    {
        return ::ComputeRankPercentile( LeaderBoardSize.lValue, Rank.lValue);
    }

    double ComputeAverageKPH()
    {
        if ( 0 == TotalMSeconds.lValue )
            return 0.0;

        // (meters per second) * (one hour) / (one kilometer)
        //
        //  M     km
        // --- * ----
        // sec   hour
        //
        // meters * hour / km * seconds
        const double dMSecondsPerHour = 60 * 60 * 1000;
        const double dKilometer = 1000.0;
        return TotalMeters.lValue * dMSecondsPerHour / ( TotalMSeconds.lValue  * dKilometer );
    }

    double ComputeAverageMPH()
    {
        // standard kilometers to miles conversion
        return ComputeAverageKPH() * 0.6124;
    }

    void ComputeTotalRaceTimeHMS( DWORD &dwHours, DWORD &dwMinutes, DWORD &dwSeconds)
    {
        ::ComputeHMS( TotalMSeconds.lValue, dwHours, dwMinutes, dwSeconds );
    }

    void ComputeBestSplitTimeHMS( DWORD &dwHours, DWORD &dwMinutes, DWORD &dwSeconds)
    {
        ::ComputeHMS( BestSplitTime.lValue, dwHours, dwMinutes, dwSeconds );
    }

    char *LookupBestSplitCar()
    {
        if ( 0 == BestSplitCar.lValue )
            return "---";

        Assert( ( BestSplitCar.lValue > 0 ) && ( BestSplitCar.lValue < NCarTypes + 1 ) );
        return CarInfo[ BestSplitCar.lValue - 1 ].Name;
    }

    void RandomFillData()
    {
            LeaderBoardSize.lValue    = 3591 * rand() / RAND_MAX;
            Rank.lValue               = 876 * rand() / RAND_MAX;
            Rating.llValue            = 234567 * rand() / RAND_MAX;
            RacesStarted.lValue       = 900 * rand() / RAND_MAX;
            RacesCompleted.lValue     = 850 * rand() / RAND_MAX;
            RacesWon.lValue           = 34 * rand() / RAND_MAX;
            TotalMSeconds.lValue       = 516195 * rand() / RAND_MAX;
            TotalMeters.lValue        = 20000 * rand() / RAND_MAX;
            BestSplitTime.lValue      = 73 * rand() / RAND_MAX;
            BestSplitCar.lValue       = 2 * rand() / RAND_MAX;
    }
};
//int checksize = 1 / ( sizeof(RacerStatsRecord)/sizeof(XONLINE_STAT) == RacerStatsRecord::NUMBER_OF_STATS  ? 1 : 0);
//#error RacerStatsRecord must only contain the fields fo the record no extra data!
//#endif

class CStatRacer : public CXOnlineTask
{
public:

    XUID                m_xuidUser;
    int                 m_nTracks;
    XONLINE_STAT_SPEC   m_Specs[ LEVEL_NSHIPPED_LEVELS ];
    RacerStatsRecord    m_Stats[ LEVEL_NSHIPPED_LEVELS ];
    RacerStatsRecord    m_Summary;

    std::map< int, int>      m_TrackIndexes;

public:
    CStatRacer()
    {
        m_nTracks = 0;
        m_hTask = NULL;
        memset( m_Specs, 0, sizeof(m_Specs));
        memset( m_Stats, 0, sizeof(m_Stats));
        memset( &m_xuidUser, 0, sizeof(m_xuidUser));
    }

    void Reset( XUID xuidUser )
    {
        Assert( sizeof(RacerStatsRecord)/sizeof(XONLINE_STAT) == RacerStatsRecord::NUMBER_OF_STATS );

        m_xuidUser = xuidUser;
        m_nTracks = 0;
        m_TrackIndexes.clear();
        m_Summary.Clear();

        for ( int nLevel = 0; nLevel < LEVEL_NSHIPPED_LEVELS; nLevel++ )
        {
            DWORD dwLeaderBoardID = LeaderBoardID( nLevel );

            if ( TRACK_TYPE_RACE == DefaultLevelInfo[ nLevel ].TrackType )
            {
                m_Specs[ m_nTracks ].xuidUser = m_xuidUser;
                m_Specs[ m_nTracks ].dwLeaderBoardID = dwLeaderBoardID;
                m_Specs[ m_nTracks ].dwNumStats = RacerStatsRecord::NUMBER_OF_STATS;
                m_Specs[ m_nTracks ].pStats = (PXONLINE_STAT)&m_Stats[ m_nTracks ];

                m_Stats[ m_nTracks ].Clear();

                m_TrackIndexes[ dwLeaderBoardID ] = m_nTracks;

                m_nTracks++;
            }
        }

        // ver overall leaderboard size and rank.
        m_Specs[ m_nTracks ].xuidUser = m_xuidUser;
        m_Specs[ m_nTracks ].dwLeaderBoardID = STAT_LB_RACING_OVERALL;
        m_Specs[ m_nTracks ].dwNumStats = 2;
        m_Specs[ m_nTracks ].pStats = &m_Summary.LeaderBoardSize;

    }

    RacerStatsRecord *GetLeaderBoardStats( DWORD dwLeaderBoardID )
    {
        //Assert( IsTrack( dwLeaderBoardID ) );
        if ( STAT_LB_RACING_OVERALL == dwLeaderBoardID )
            return &m_Summary;
            
        int nTrack = m_TrackIndexes[ dwLeaderBoardID ];

        return &( m_Stats[ nTrack ] );
    }

    // Server used reduces storage, but in exchange costs additional 
    // transfer of data needed to compute racer summary 
    void ComputeCalculatedFields()
    {
        //m_Summary.Clear();

        long lResult = 0;
        for (int nIndex = 0; nIndex < m_nTracks; nIndex++ )
        {
            m_Summary.RacesStarted.lValue     += m_Stats[ nIndex ].RacesStarted.lValue;
            m_Summary.RacesCompleted.lValue   += m_Stats[ nIndex ].RacesCompleted.lValue;
            m_Summary.RacesWon.lValue         += m_Stats[ nIndex ].RacesWon.lValue;
            m_Summary.TotalMSeconds.lValue    += m_Stats[ nIndex ].TotalMSeconds.lValue;
            m_Summary.TotalMeters.lValue      += m_Stats[ nIndex ].TotalMeters.lValue;
            
            if ( m_Summary.BestSplitTime.lValue > m_Stats[ nIndex ].BestSplitTime.lValue )
            {
                m_Summary.BestSplitTime.lValue  = m_Stats[ nIndex ].BestSplitTime.lValue;
                m_Summary.BestSplitCar.lValue   = m_Stats[ nIndex ].BestSplitCar.lValue;
            }
        }
    }

    HRESULT RequestStats()
    {
        Assert( NULL == m_hTask );  // can only process 1 task at a time
#ifdef XONLINE_OFFLINE
        m_hrStatus = S_OK;
        m_hTask = (XONLINETASK_HANDLE)-1;
#else
        m_hrStatus = XOnlineStatRead( m_nTracks + 1, m_Specs, NULL, &m_hTask);
#endif
        // OnlineTasks_Add( handle );  I want to process my result with using single instance global structures.


        return m_hrStatus;
    }

    HRESULT WriteTrackStats( DWORD dwLeaderBoardID )
    {
        Assert( NULL == m_hTask );  // can only process 1 task at a time
#ifdef XONLINE_OFFLINE
        m_hrStatus = S_OK;
        m_hTask = (XONLINETASK_HANDLE)-1;
#else
        Assert( STAT_LB_RACING_OVERALL != dwLeaderBoardID );
        
        int nTrack = m_TrackIndexes[ dwLeaderBoardID ];

        m_hrStatus = XOnlineStatWrite( 1, &(m_Specs[ nTrack ]), NULL, &m_hTask);

#endif


        return m_hrStatus;
    }

    HRESULT CancelPendingRequest()
    {
        if ( NULL == m_hTask )
            return S_FALSE;  // none pending
    
        m_hrStatus = XOnlineTaskClose( m_hTask);
        m_hTask = NULL;

        return m_hrStatus;
    }

    HRESULT TaskContinue()
    {
#ifdef XONLINE_OFFLINE
        Assert( (XONLINETASK_HANDLE)-1 == m_hTask );
        m_hrStatus = S_OK;

        for (int nIndex = 0; nIndex < m_nTracks; nIndex++ )
        {
            m_Stats[ nIndex ].RandomFillData();
        }

        ComputeCalculatedFields();

        m_hTask = NULL;
#else
        Assert( m_hTask );
        m_hrStatus = XOnlineTaskContinue( m_hTask );

        if (XONLINETASK_S_RUNNING != m_hrStatus)
        {
            if ( SUCCEEDED( m_hrStatus ) )
            {
                m_hrStatus = XOnlineStatReadGetResult( m_hTask, m_nTracks + 1, m_Specs, 0, NULL);
                Assert( XONLINETASK_S_SUCCESS == m_hrStatus );  // what possible errors???

                ComputeCalculatedFields();
            }
            else
            {
                XONLINESTATSERROR( m_hrStatus);
            }

            XOnlineTaskClose( m_hTask);
                m_hTask = NULL;
        }
#endif

        return m_hrStatus;
    }
};

// May only contain XONLINE_STAT members as this is used at times as an array
class BattlerStatsRecord
{
public:
    XONLINE_STAT Rating;                // writeable
    XONLINE_STAT BattlesStarted;        // writeable
    XONLINE_STAT BattlesCompleted;      // writeable
    XONLINE_STAT BattlesWon;            // writeable
    XONLINE_STAT TotalItMSeconds;       // writeable
    XONLINE_STAT TotalBattleMSeconds;   // writeable
    XONLINE_STAT LeaderBoardSize;       // read-only
    XONLINE_STAT Rank;                  // read-only

    enum { NUMBER_OF_WRITEABLE_STATS = 6 };
    enum { NUMBER_OF_STATS = 8 };

    BattlerStatsRecord()
    {
        Assert( sizeof(BattlerStatsRecord)/sizeof(XONLINE_STAT) == NUMBER_OF_STATS );
        Clear();
    }

    //PXONLINE_STAT operator&() { return (PXONLINE_STAT)this; };

    void Clear()
    {
        // init the field data
        Rating.wID                  = XONLINE_STAT_RATING;
        Rating.type                 = XONLINE_STAT_LONGLONG;
        Rating.llValue              = 0;

        BattlesStarted.wID          = STAT_BATTLES_COMPLETED;
        BattlesStarted.type         = XONLINE_STAT_LONG;
        BattlesStarted.lValue       = 0;

        BattlesCompleted.wID        = STAT_BATTLES_COMPLETED;
        BattlesCompleted.type       = XONLINE_STAT_LONG;
        BattlesCompleted.lValue     = 0;

        BattlesWon.wID              = STAT_BATTLES_WON;
        BattlesWon.type             = XONLINE_STAT_LONG;
        BattlesWon.lValue           = 0;

        TotalItMSeconds.wID         = STAT_BATTLES_TOTAL_IT_SECONDS;
        TotalItMSeconds.type        = XONLINE_STAT_LONG;
        TotalItMSeconds.lValue      = 0;

        TotalBattleMSeconds.wID     = STAT_BATTLES_TOTAL_SECONDS;
        TotalBattleMSeconds.type    = XONLINE_STAT_LONG;
        TotalBattleMSeconds.dValue  = 0;

        LeaderBoardSize.wID         = XONLINE_STAT_LEADERBOARD_SIZE;
        LeaderBoardSize.type        = XONLINE_STAT_LONG;
        LeaderBoardSize.lValue      = 0;

        Rank.wID                    = XONLINE_STAT_RANK;
        Rank.type                   = XONLINE_STAT_LONG;
        Rank.lValue                 = 0;

    }

    long ComputeBattlesLost()
    {
        return BattlesCompleted.lValue - BattlesWon.lValue;
    }

    long ComputeBattlesAbandoned()
    {
        return BattlesStarted.lValue - BattlesCompleted.lValue;
    }

    long ComputeWinLossPercentage()
    {
        long lBattlesLost = ComputeBattlesLost();

        if (lBattlesLost)
            return BattlesWon.lValue * 100 / lBattlesLost;

        return 0;
    }

    float ComputeRankPercentile()
    {
        return ::ComputeRankPercentile( LeaderBoardSize.lValue, Rank.lValue);
    }

    void ComputeTotalItTimeHMS( DWORD &dwHours, DWORD &dwMinutes, DWORD &dwSeconds)
    {
        ::ComputeHMS( TotalItMSeconds.lValue, dwHours, dwMinutes, dwSeconds );
    }

    void ComputeTotalBattleTimeHMS( DWORD &dwHours, DWORD &dwMinutes, DWORD &dwSeconds)
    {
        ::ComputeHMS( TotalBattleMSeconds.lValue, dwHours, dwMinutes, dwSeconds );
    }

    long ComputeTotalItPercentage()
    {
        if ( TotalBattleMSeconds.lValue )
            return TotalItMSeconds.lValue * 100 / TotalBattleMSeconds.lValue;

        return 0;
    }
};
//int checksize2 = 1 / ( sizeof(BattlerStatsRecord)/sizeof(XONLINE_STAT) == BattlerStatsRecord::NUMBER_OF_STATS  ? 1 : 0);

class CStatBattler
{
public:
    XUID                m_xuidUser;
    int                 m_nTracks;
    XONLINE_STAT_SPEC   m_Specs[ LEVEL_NSHIPPED_LEVELS ];
    BattlerStatsRecord  m_Stats[ LEVEL_NSHIPPED_LEVELS ];
    BattlerStatsRecord  m_Summary;

    XONLINETASK_HANDLE  m_hTask;
    std::map< int, int>      m_TrackIndexes;

public:
    CStatBattler()
    {
        m_nTracks = 0;
        m_hTask = NULL;
        memset( m_Specs, 0, sizeof(m_Specs));
        memset( &m_Stats, 0, sizeof(m_Stats));
        memset( &m_xuidUser, 0, sizeof(m_xuidUser));
    }

    void Reset( XUID xuidUser)
    {
        m_xuidUser = xuidUser;
        m_nTracks = 0;
        m_TrackIndexes.clear();
        m_Summary.Clear();

        for ( int nLevel = 0; nLevel < LEVEL_NSHIPPED_LEVELS; nLevel++ )
        {
            DWORD dwLeaderBoardID = LeaderBoardID( nLevel );

            if ( TRACK_TYPE_RACE == DefaultLevelInfo[ nLevel ].TrackType )
            {
                m_Specs[ m_nTracks ].xuidUser = m_xuidUser;
                m_Specs[ m_nTracks ].dwLeaderBoardID = dwLeaderBoardID;
                m_Specs[ m_nTracks ].dwNumStats = 2;
                m_Specs[ m_nTracks ].pStats = (PXONLINE_STAT)&m_Stats[ m_nTracks ];

                m_Stats[ m_nTracks ].Clear();

                m_TrackIndexes[ dwLeaderBoardID ] = m_nTracks;

                m_nTracks++;
            }
        }

    }

    BattlerStatsRecord *GetLeaderBoardStats( DWORD dwLeaderBoardID )
    {
        //Assert( IsTrack( dwLeaderBoardID ) );
        if ( STAT_LB_BATTLE_OVERALL == dwLeaderBoardID )
            return &m_Summary;
         
        int nTrack = m_TrackIndexes[ dwLeaderBoardID ];

        return &( m_Stats[ nTrack ] );
    }

    void ComputeCalculatedFields()
    {
        m_Summary.Clear();

        long lResult = 0;
        for (int nIndex = 0; nIndex < m_nTracks; nIndex++ )
        {
            m_Summary.BattlesStarted.lValue      += m_Stats[ nIndex ].BattlesStarted.lValue;
            m_Summary.BattlesCompleted.lValue    += m_Stats[ nIndex ].BattlesCompleted.lValue;
            m_Summary.BattlesWon.lValue          += m_Stats[ nIndex ].BattlesWon.lValue;
            m_Summary.TotalItMSeconds.lValue     += m_Stats[ nIndex ].TotalItMSeconds.lValue;
            m_Summary.TotalBattleMSeconds.lValue += m_Stats[ nIndex ].TotalBattleMSeconds.lValue;
        }
    }


    HRESULT RequestStats()
    {
        Assert( NULL == m_hTask );  // can only process 1 task at a time
#ifdef XONLINE_OFFLINE
        HRESULT hr = S_OK;
        m_hTask = (XONLINETASK_HANDLE)-1;
#else
        HRESULT hr = XOnlineStatRead( m_nTracks, m_Specs, NULL, &m_hTask );
#endif
        // OnlineTasks_Add( handle );  I want to process my result with using single instance global structures.


        return hr;
    }

    HRESULT CancelPendingRequest()
    {
        if ( NULL == m_hTask )
            return S_FALSE;  // none pending
    
        HRESULT hr = XOnlineTaskClose( m_hTask);
        m_hTask = NULL;

        return hr;
    }

    HRESULT TaskContinue()
    {
#ifdef XONLINE_OFFLINE
        Assert( (XONLINETASK_HANDLE)-1 == m_hTask );
        HRESULT hr = S_OK;

        for (int nIndex = 0; nIndex < m_nTracks; nIndex++ )
        {
            m_Stats[ nIndex ].LeaderBoardSize.lValue    = 3591 * rand() / RAND_MAX;
            m_Stats[ nIndex ].Rank.lValue               = 876 * rand() / RAND_MAX;
            m_Stats[ nIndex ].Rating.llValue            = 234567 * rand() / RAND_MAX;
            m_Stats[ nIndex ].BattlesStarted.lValue       = 900 * rand() / RAND_MAX;
            m_Stats[ nIndex ].BattlesCompleted.lValue     = 850 * rand() / RAND_MAX;
            m_Stats[ nIndex ].BattlesWon.lValue           = 34 * rand() / RAND_MAX;
            m_Stats[ nIndex ].TotalItSeconds.lValue       = 16195 * rand() / RAND_MAX;
            m_Stats[ nIndex ].TotalBattleSeconds.lValue       = 516195 * rand() / RAND_MAX;
        }

        ComputeCalculatedFields();

        m_hTask = NULL;
#else
        Assert( m_hTask );
        HRESULT hr = XOnlineTaskContinue( m_hTask );

        if (XONLINETASK_S_RUNNING != hr)
        {
            ComputeCalculatedFields();
            XOnlineTaskClose( m_hTask);
            m_hTask = NULL;
        }
        else if ( FAILED( hr) )
        {
            XONLINESTATSERROR( hr );
        }
#endif

        return hr;
    }
};


#define MAX_BOARDSIZE_LOCALRACERS 10
class CStatPlayerRanks
{
public:

    XONLINE_STAT_SPEC   m_Specs[ MAX_BOARDSIZE_LOCALRACERS];
    XONLINE_STAT        m_Stats[ MAX_BOARDSIZE_LOCALRACERS + 1];
    XONLINE_USER        m_Users[ MAX_BOARDSIZE_LOCALRACERS];

    DWORD               m_dwLeaderBoardID;
    int                 m_nUsers;
    XONLINETASK_HANDLE  m_hTask;
    HRESULT             m_hrStatus;

public:
    CStatPlayerRanks()
    {
        m_hTask = NULL;
        Reset( 0 );
        memset( m_Specs, 0, sizeof(m_Specs));
        memset( m_Stats, 0, sizeof(m_Stats));
        memset( m_Users, 0, sizeof(m_Users));
    }

    void Reset( DWORD dwLeaderBoardID )
    {
        m_hrStatus = S_FALSE;
        m_dwLeaderBoardID = dwLeaderBoardID;
        m_nUsers = 0;
    }

    int AddUser( XONLINE_USER *pUser)
    {
        Assert( pUser );
        Assert( m_nUsers < MAX_BOARDSIZE_LOCALRACERS);

        m_Users[ m_nUsers ] = *pUser;

        if (0 == m_nUsers)
        {
            m_Specs[0].xuidUser = pUser->xuid;
            m_Specs[0].dwLeaderBoardID = m_dwLeaderBoardID;
            m_Specs[0].dwNumStats = 2;
            m_Specs[0].pStats = &m_Stats[0];

            m_Stats[0].wID = XONLINE_STAT_LEADERBOARD_SIZE;
            m_Stats[0].type = XONLINE_STAT_LONG;
            m_Stats[0].lValue = 0;

            m_Stats[1].wID = XONLINE_STAT_RANK;
            m_Stats[1].type = XONLINE_STAT_LONG;
            m_Stats[1].lValue = 0;
        }
        else
        {
            m_Specs[m_nUsers].xuidUser = pUser->xuid;
            m_Specs[m_nUsers].dwLeaderBoardID = m_dwLeaderBoardID;
            m_Specs[m_nUsers].dwNumStats = 1;
            m_Specs[m_nUsers].pStats = &m_Stats[m_nUsers + 1];

            m_Stats[m_nUsers + 1].wID = XONLINE_STAT_RANK;
            m_Stats[m_nUsers + 1].type = XONLINE_STAT_LONG;
            m_Stats[m_nUsers + 1].lValue = 0;
        }

        m_nUsers++;

        // return index of this user
        return m_nUsers - 1;
    }

    long GetSize()
    {
        return m_Stats[0].lValue;
    }

    long GetUserRank( int nUserIndex)
    {
        Assert( nUserIndex < m_nUsers);
        return m_Stats[nUserIndex + 1].lValue;
    }


    HRESULT RequestStats()
    {
        Assert( NULL == m_hTask );  // can only process 1 task at a time
#ifdef XONLINE_OFFLINE
        HRESULT hr = S_OK;
        m_hTask = (XONLINETASK_HANDLE)-1;
#else
        HRESULT hr = XOnlineStatRead( m_nUsers, m_Specs, NULL, &m_hTask);
#endif
        // OnlineTasks_Add( handle );  I want to process my result with using single instance global structures.


        return hr;
    }

    HRESULT CancelPendingRequest()
    {
        if ( NULL == m_hTask )
            return S_FALSE;  // none pending
    
        HRESULT hr = XOnlineTaskClose( m_hTask);
        m_hTask = NULL;

        return hr;
    }

    HRESULT TaskContinue()
    {
#ifdef XONLINE_OFFLINE
        Assert( (XONLINETASK_HANDLE)-1 == m_hTask );

        m_Stats[ 0 ].lValue = 1234567;
        for ( int nStat = 1; nStat < m_nUsers + 1; nStat++ )
        {
            m_Stats[ nStat ].lValue = nStat * 111;
        }
        m_hTask = NULL;
        m_hrStatus = S_OK;
#else
        Assert( m_hTask );
        m_hrStatus = XOnlineTaskContinue( m_hTask );

        if (XONLINETASK_S_RUNNING != m_hrStatus)
        {
            if ( SUCCEEDED(m_hrStatus) )
            {
                m_hrStatus = XOnlineStatReadGetResult( m_hTask, m_nUsers, m_Specs, 0, NULL );
                Assert( XONLINETASK_S_SUCCESS == m_hrStatus );  // what possible errors???
            }
            else
            {
                XONLINESTATSERROR(m_hrStatus);
            }

            XOnlineTaskClose(m_hTask);
            m_hTask = NULL;
        }
        else if ( FAILED( m_hrStatus ))
        {
            XONLINESTATSERROR(m_hrStatus);
        }

#endif

        return m_hrStatus;
    }
};

#define MAX_BOARDSIZE_WORLDLEADER 10
class CStatWorldLeaderRanks
{
public:

    XONLINE_STAT_SPEC   m_Specs[ MAX_BOARDSIZE_WORLDLEADER];
    XONLINE_STAT        m_Stats[ MAX_BOARDSIZE_WORLDLEADER + 1];
    XONLINE_STAT_USER   m_Users[ MAX_BOARDSIZE_WORLDLEADER];

    DWORD               m_dwLeaderBoardID;
    int                 m_nUsers;
    XONLINETASK_HANDLE  m_hTask;
    DWORD               m_dwLeaderboardSize;
    DWORD               m_dwReturnedResults;

public:
    CStatWorldLeaderRanks()
    {
        Reset( 0, 0 );
    }

    void Reset( DWORD dwLeaderBoardID, int nNumberOfLeaders )
    {
        m_dwLeaderBoardID = dwLeaderBoardID;
        m_nUsers = nNumberOfLeaders;
        m_dwReturnedResults = 0;
        m_hTask = NULL;
        memset( m_Specs, 0, sizeof(m_Specs));
        memset( m_Stats, 0, sizeof(m_Stats));
        memset( m_Users, 0, sizeof(m_Users));
    }


    long GetSize()
    {
        return m_Stats[0].lValue;
    }

    long GetUserRank( int nUserIndex)
    {
        Assert( nUserIndex < m_nUsers);
        return m_Stats[nUserIndex].lValue;
    }


    HRESULT RequestStats()
    {
#ifdef XONLINE_OFFLINE
        HRESULT hr = S_OK;
        m_hTask = (XONLINETASK_HANDLE)-1;
#else
        WORD wXONLINE_STAT_RANK = XONLINE_STAT_RANK;

        HRESULT hr = XOnlineStatLeaderEnumerate(    NULL,                       // pxuidPagePivot
                                                    0,                          // dwPageStart
                                                    MAX_BOARDSIZE_WORLDLEADER,  // dwPageSize,
                                                    m_dwLeaderBoardID,
                                                    1,                          // dwNumStatsPerUser,
                                                    &wXONLINE_STAT_RANK,        // pStatsPerUser
                                                    NULL,                       // hWorkEvent,
                                                    &m_hTask);
#endif

        return hr;
    }

    HRESULT CancelPendingRequest()
    {
        if ( NULL == m_hTask )
            return S_FALSE;  // none pending
    
        HRESULT hr = XOnlineTaskClose( m_hTask);
        m_hTask = NULL;

        return hr;
    }

    HRESULT TaskContinue()
    {
#ifdef XONLINE_OFFLINE
        Assert( (XONLINETASK_HANDLE)-1 == m_hTask );
        HRESULT hr = S_OK;

        m_Stats[ 0 ].lValue = 1234567;

        for ( int nStat = 1; nStat < m_nUsers + 1; nStat++ )
        {
            m_Stats[ nStat ].lValue = nStat;
            sprintf( m_Users[ nStat - 1].szUsername, "XONLINE_OFFLINE.WorldChamp%d", nStat);
        }

        m_dwReturnedResults = m_nUsers;

        m_hTask = NULL;
#else
        Assert( m_hTask );
        HRESULT hr = XOnlineTaskContinue( m_hTask );

        if (XONLINETASK_S_RUNNING != hr)
        {
            if ( SUCCEEDED( hr ) )
            {
                HRESULT hrResults = XOnlineStatLeaderEnumerateGetResults(
                                            m_hTask,
                                            MAX_BOARDSIZE_WORLDLEADER,  // dwUserCount,
                                            m_Users,
                                            MAX_BOARDSIZE_WORLDLEADER,  // dwStatCount,
                                            m_Stats,
                                            &m_dwLeaderboardSize,
                                            &m_dwReturnedResults,
                                            0,                          // dwExtraBufferSize
                                            NULL );                     // pExtraBuffer
                Assert( XONLINETASK_S_SUCCESS == hrResults );  // what possible errors???
            }
            else
            {
                XONLINESTATSERROR( hr );
                // results failed, better make sure no one tried to use stats
                m_dwReturnedResults = 0;
            }

            XOnlineTaskClose( m_hTask);
            m_hTask = NULL;
        }
#endif
        return hr;
    }
};


//-----------------------------------------------------------------------------
// Name: CStatUserTrackRanks
// Desc: Retrieve the ranks for a single user for all tracks of a given type
//-----------------------------------------------------------------------------
class CStatUserTrackRanks
{
public:

    XONLINE_STAT_SPEC   m_Specs[ LEVEL_NSHIPPED_LEVELS];
    XONLINE_STAT        m_Stats[ LEVEL_NSHIPPED_LEVELS];

    XONLINE_USER        m_User;
    XONLINETASK_HANDLE  m_hTask;

    int                 m_nRaceLevels;

public:
    CStatUserTrackRanks()
    {
        m_hTask = NULL;
        m_nRaceLevels = 0;
        memset( &m_User, 0, sizeof( m_User ) );
        memset( m_Specs, 0, sizeof( m_Specs ) );
        memset( m_Stats, 0, sizeof( m_Stats ) );
    }

    void Reset( XONLINE_USER *pUser, bool bBattleMode )
    {
        Assert( pUser );

        m_User = *pUser;
        m_nRaceLevels = 0;

        TRACK_TYPE eTrackFilter = ( bBattleMode ? TRACK_TYPE_BATTLE : TRACK_TYPE_RACE );


        for ( int nLevel = 0; nLevel < LEVEL_NSHIPPED_LEVELS; nLevel++ )
        {
            if ( eTrackFilter == DefaultLevelInfo[ nLevel ].TrackType )
            {
                m_Specs[ m_nRaceLevels ].xuidUser = m_User.xuid;
                m_Specs[ m_nRaceLevels ].dwLeaderBoardID = LeaderBoardID( nLevel );
                m_Specs[ m_nRaceLevels ].dwNumStats = 1;
                m_Specs[ m_nRaceLevels ].pStats = &m_Stats[m_nRaceLevels];

                m_Stats[ m_nRaceLevels ].wID = XONLINE_STAT_RANK;
                m_Stats[ m_nRaceLevels ].type = XONLINE_STAT_LONG;
                m_Stats[ m_nRaceLevels ].lValue = 0;

                m_nRaceLevels++;
            }
        }
    
    }


    HRESULT RequestStats()
    {
        Assert( NULL == m_hTask );  // can only process 1 task at a time
#ifdef XONLINE_OFFLINE
        HRESULT hr = S_OK;
        m_hTask = (XONLINETASK_HANDLE)-1;
#else
        HRESULT hr = XOnlineStatRead( m_nRaceLevels, m_Specs, NULL, &m_hTask);
#endif

        return hr;
    }

    HRESULT CancelPendingRequest()
    {
        if ( NULL == m_hTask )
            return S_FALSE;  // none pending
    
        HRESULT hr = XOnlineTaskClose( m_hTask);
        m_hTask = NULL;

        return hr;
    }

    HRESULT TaskContinue()
    {
#ifdef XONLINE_OFFLINE
        Assert( (XONLINETASK_HANDLE)-1 == m_hTask );
        HRESULT hr = S_OK;

        for ( int nStat = 1; nStat < m_nRaceLevels; nStat++ )
        {
            m_Stats[ nStat ].lValue = nStat * 101;
        }
        m_hTask = NULL;
#else
        Assert( m_hTask );
        HRESULT hr = XOnlineTaskContinue( m_hTask );

        if (XONLINETASK_S_RUNNING != hr)
        {
            if (SUCCEEDED( hr ) )
            {
                hr = XOnlineStatReadGetResult( m_hTask, m_nRaceLevels, m_Specs, 0, NULL);
                Assert( XONLINETASK_S_SUCCESS == hr );  // what possible errors???
            }
            else
            {
                XONLINESTATSERROR( hr );
            }

            XOnlineTaskClose( m_hTask);
            m_hTask = NULL;
        }
#endif

        return hr;
    }
};

#define MAX_LOCALPLAYERS 4
class CStatAdjustMatchesStarted : public CXOnlineTask
{
public:

    XONLINE_STAT_SPEC   m_Specs[ MAX_LOCALPLAYERS];
    XONLINE_STAT        m_Stats[ MAX_LOCALPLAYERS];
    //XONLINE_USER        m_Users[ MAX_BOARDSIZE_LOCALRACERS];

    DWORD               m_dwLeaderBoardID;
    int                 m_nUsers;
    int                 m_nOffset;
    //XONLINETASK_HANDLE  m_hTask;
    //HRESULT             m_hrStatus;

    enum 
    {
        STATE_INIT,
        STATE_STAT_READ,
        STATE_STAT_WRITE,
        STATE_DONE
    }                   m_eState;

public:
    CStatAdjustMatchesStarted()
    {
        m_hTask = NULL;
        Reset( 0, 0 );
        memset( m_Specs, 0, sizeof(m_Specs));
        memset( m_Stats, 0, sizeof(m_Stats));
        //memset( m_Users, 0, sizeof(m_Users));
    }

    void Reset( DWORD dwLeaderBoardID, int nOffset )
    {
        m_hrStatus = S_FALSE;
        m_dwLeaderBoardID = dwLeaderBoardID;
        m_nUsers = 0;
        m_eState = STATE_INIT;
        m_nOffset = nOffset;
    }

    int AddLocalPlayer( XUID xuid)
    {
        Assert( m_nUsers < MAX_LOCALPLAYERS);

        // implementation relies upon STAT_RACES_STARTED == STAT_BATTLES_STARTED
        Assert( STAT_RACES_STARTED == STAT_BATTLES_STARTED );

        //m_Users[ m_nUsers ] = *pUser;

        m_Specs[ m_nUsers ].xuidUser = xuid;
        m_Specs[ m_nUsers ].dwLeaderBoardID = m_dwLeaderBoardID;
        m_Specs[ m_nUsers ].dwNumStats = 1;
        m_Specs[ m_nUsers ].pStats = &m_Stats[ m_nUsers ];

        m_Stats[ m_nUsers ].wID = STAT_RACES_STARTED;
        m_Stats[ m_nUsers ].type = XONLINE_STAT_LONG;
        m_Stats[ m_nUsers ].lValue = 0;

        m_nUsers++;

        // return index of this user
        return m_nUsers - 1;
    }

    HRESULT BeginStatUpdate()
    {
        Assert( NULL == m_hTask );  // can only process 1 task at a time
#ifdef XONLINE_OFFLINE
        m_hrStatus = S_OK;
        m_hTask = (XONLINETASK_HANDLE)-1;
#else
        m_hrStatus = XOnlineStatRead( m_nUsers, m_Specs, NULL, &m_hTask);
#endif
        // OnlineTasks_Add( handle );  I want to process my result with using single instance global structures.
        m_eState = STATE_STAT_READ;

        return m_hrStatus;
    }

    HRESULT CancelPendingRequest()
    {
        if ( NULL == m_hTask )
            return S_FALSE;  // none pending
    
        m_hrStatus = XOnlineTaskClose( m_hTask);
        m_hTask = NULL;
        m_eState = STATE_DONE;

        return m_hrStatus;
    }

    HRESULT TaskContinue()
    {
#ifdef XONLINE_OFFLINE
        Assert( (XONLINETASK_HANDLE)-1 == m_hTask );

        for ( int nStat = 0; nStat < m_nUsers; nStat++ )
        {
            m_Stats[ nStat ].lValue = nStat * 111;
        }
        m_hTask = NULL;
        m_hrStatus = S_OK;
        m_eState = STATE_DONE;
#else
        Assert( m_hTask );
        m_hrStatus = XOnlineTaskContinue( m_hTask );

        if ( FAILED( m_hrStatus ) )
        {
            XONLINESTATSERROR( m_hrStatus );
            XOnlineTaskClose( m_hTask);
            m_hTask = NULL;   
        }

        if (XONLINETASK_S_SUCCESS == m_hrStatus)
        {
            switch ( m_eState )
            {
                case STATE_STAT_READ:
                {
                    m_hrStatus = XOnlineStatReadGetResult( m_hTask, m_nUsers, m_Specs, 0, NULL);
                    Assert( XONLINETASK_S_SUCCESS == m_hrStatus );  // what possible errors???

                    for ( int nStat = 0; nStat < m_nUsers; nStat++ )
                    {
                        m_Stats[ nStat ].wID = STAT_RACES_STARTED;
                        m_Stats[ nStat ].type = XONLINE_STAT_LONG;
                        m_Stats[ nStat ].lValue += m_nOffset;
                    }
                    XOnlineTaskClose( m_hTask);
                    m_hTask = NULL;

                    m_hrStatus = XOnlineStatWrite( m_nUsers, m_Specs, NULL, &m_hTask);
                    m_eState = STATE_STAT_WRITE;
                    break;
                }

                case STATE_STAT_WRITE:
                    XOnlineTaskClose( m_hTask);
                    m_hTask = NULL;
                    m_eState = STATE_DONE;
                    break;
            }

        }
#endif

        return m_hrStatus;
    }
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#define TASKCONTINUEWITHERROR( hTask, hr) TaskContinueWithError( hTask, hr, __LINE__ )
inline void TaskContinueWithError( XONLINETASK_HANDLE &hTask, HRESULT &hr, int nLineNumber)
{
    hr = XOnlineTaskContinue( hTask );

    if ( FAILED( hr ) )
    {
        // XONLINESTATSERROR( hr );
        Assert( !"Stat Error");
        ShowXStatsError( nLineNumber, hr);
        XOnlineTaskClose( hTask);
        hTask = NULL;   
    }

}


const int BAD_BYTE = 0xfa;
//-----------------------------------------------------------------------------
// Name: CStatRatingSystemPlayerData
// Desc: Retrieve Races completed for each player on each track AND Overall Rating
//
// Process:
//      1) First trip to stats server
//         StatGet
//          retrieve player ratings & races completed for current track LB and overall LB  (users * 2)
//         LeaderEnumerate
//          a) retrieve LB size and Max rating for Overall and track LB     (2 * 2)
//
//      2) Second trip to server
//         LeaderEnumerate
//          a) retrieve LB min rating for Overall and track LB              (2)


// Notes:  
//      1) Leaderboard MinRating is rating of lowest player on LB( player at LB size position)
//-----------------------------------------------------------------------------
class CStatRatingSystemPlayerData
{
public:


    enum 
    {
        STATE_INIT,
        STATE_STAT_READ_PLAYER_DATA,
        STATE_STAT_READ_LEADERBOARD_DATA,
        STATE_DONE
    }                   m_eState;

    XONLINE_USER        m_Users[ MAX_NUM_PLAYERS];
    int                 m_nUsers;

#define NUMSTATS( nUsers )  ( 2 + ( nUsers ) * 2 )  // number fo spec to be read
    inline int Track( int nUser )       { return 2 + (nUser * 2); }       // index to track SPEC fo user N   
    inline int Overall( int nUser )     { return 2 + (nUser * 2) + 1; }   // index to Overall SPEC for user N

    // keeping all specs in one array as that is how xonline API wants it.
    // first 2 specs for leaderboard size trac and overall
    // remaining spec alternate between being specs for tracks and overall stats for user
    XONLINE_STAT_SPEC   m_Specs[ NUMSTATS( MAX_NUM_PLAYERS ) ];

    typedef struct _PLAYERRATING_STATS
    {
        XONLINE_STAT    Rating;
        XONLINE_STAT    RacesCompleted;

        void Clear()
        {
            // init the field data
            Rating.wID              = XONLINE_STAT_RATING;
            Rating.llValue          = 0;

            RacesCompleted.wID      = STAT_RACES_COMPLETED;
            RacesCompleted.lValue   = 0;

            ResetTypes();
        }

        void ResetTypes()
        {
            Rating.type             = XONLINE_STAT_LONGLONG;
            RacesCompleted.type     = XONLINE_STAT_LONG;
        }

    } PLAYERRATING_STATS;

    typedef struct _LEADERBOARD_STATS
    {
        DWORD           dwLeaderBoardID;
        XONLINE_STAT    Size;
        XONLINE_STAT    MinRating;
        XONLINE_STAT    MaxRating;

        void Clear()
        {
            dwLeaderBoardID = -1;

            // init the field data
            Size.wID     = XONLINE_STAT_LEADERBOARD_SIZE;
            Size.lValue  = 0;

            MinRating.wID              = XONLINE_STAT_RATING;
            MinRating.llValue          = Rating::RatingToLongLong( RATING_TYPE( 0.0 ) );

            MaxRating.wID              = XONLINE_STAT_RATING;
            MaxRating.llValue          = Rating::RatingToLongLong( RATING_TYPE( 1.0 ) );

            ResetTypes();
        }

        void ResetTypes()
        {
            Size.type    = XONLINE_STAT_LONG;
            MinRating.type             = XONLINE_STAT_LONGLONG;
            MaxRating.type             = XONLINE_STAT_LONGLONG;
        }

    } LEADERBOARD_STATS;

    // stats have not need to be sequential
    LEADERBOARD_STATS   m_OverallLeaderBoard;
    PLAYERRATING_STATS  m_OverallRatingStats[ MAX_NUM_PLAYERS ];

    LEADERBOARD_STATS   m_TrackLeaderBoard;
    PLAYERRATING_STATS  m_TrackRatingStats[ MAX_NUM_PLAYERS ];

    //XONLINE_STAT        m_RacesCompleted[ MAX_NUM_PLAYERS ] [ LEVEL_NSHIPPED_LEVELS ];
    //XONLINE_STAT        m_PlayerRating[ MAX_NUM_PLAYERS ][ MAX_RATING_LB ];

    // Leader enumeration



    XONLINETASK_HANDLE  m_hTaskStatRead;
    HRESULT             m_hrStatusStatRead;

#ifndef XSTATS_NO_MINMAX_RATING_REQUEST
    XONLINETASK_HANDLE  m_hTaskEnumTrackMin;
    XONLINETASK_HANDLE  m_hTaskEnumTrackMax;
    XONLINETASK_HANDLE  m_hTaskEnumOverallMin;
    XONLINETASK_HANDLE  m_hTaskEnumOverallMax;

    HRESULT             m_hrStatusEnumTrackMin;
    HRESULT             m_hrStatusEnumTrackMax;
    HRESULT             m_hrStatusEnumOverallMin;
    HRESULT             m_hrStatusEnumOverallMax;
#endif XSTATS_NO_MINMAX_RATING_REQUEST



public:
    CStatRatingSystemPlayerData()
    {
        m_hTaskStatRead = NULL;

#ifndef XSTATS_NO_MINMAX_RATING_REQUEST
        m_hTaskEnumTrackMin = NULL;
        m_hTaskEnumTrackMax = NULL;
        m_hTaskEnumOverallMin = NULL;
        m_hTaskEnumOverallMax = NULL;
#endif XSTATS_NO_MINMAX_RATING_REQUEST

        memset( &m_Users, 0, sizeof( m_Users ) );
        memset( m_Specs, BAD_BYTE, sizeof( m_Specs ) );
    }

    void Reset( DWORD dwTrackLeaderBoardID )
    {
        m_TrackLeaderBoard.Clear();
        m_TrackLeaderBoard.dwLeaderBoardID = dwTrackLeaderBoardID;

        // $STATISTICS: hardcoded, BATTLE not yet supported
        m_OverallLeaderBoard.Clear();
        m_OverallLeaderBoard.dwLeaderBoardID = STAT_LB_RACING_OVERALL;

        m_eState = STATE_INIT;
    }

    // Users will be added in the order they finished the race in.
    int AddUser( XUID &xuid)
    {
        Assert( m_nUsers < MAX_NUM_PLAYERS);

        //m_Users[ m_nUsers ] = *pUser;

        if ( 0 == m_nUsers )
        {
            XONLINE_STAT_SPEC &SpecLBOverall = m_Specs[ 0 ];
            SpecLBOverall.xuidUser = xuid;
            SpecLBOverall.dwLeaderBoardID = m_TrackLeaderBoard.dwLeaderBoardID;
            SpecLBOverall.dwNumStats = 1;
            SpecLBOverall.pStats = (XONLINE_STAT*)&m_TrackLeaderBoard.Size;

            XONLINE_STAT_SPEC &SpecLBTrack = m_Specs[ 1 ];
            SpecLBTrack.xuidUser = xuid;
            SpecLBTrack.dwLeaderBoardID = m_OverallLeaderBoard.dwLeaderBoardID;
            SpecLBTrack.dwNumStats = 1;
            SpecLBTrack.pStats = (XONLINE_STAT*)&m_OverallLeaderBoard.Size;

        }

        XONLINE_STAT_SPEC &pSpecTrack = m_Specs[ Track( m_nUsers ) ];
        pSpecTrack.xuidUser = xuid;
        pSpecTrack.dwLeaderBoardID = m_TrackLeaderBoard.dwLeaderBoardID;
        pSpecTrack.dwNumStats = 2;
        pSpecTrack.pStats = (XONLINE_STAT*)&m_TrackRatingStats[ m_nUsers ];

        m_TrackRatingStats[ m_nUsers ].Clear();

        // $STATISTICS Once a real formula for overall ranking has been decided, get other stats
        XONLINE_STAT_SPEC &pSpecOverall = m_Specs[ Overall( m_nUsers ) ];
        pSpecOverall.xuidUser = xuid;
        pSpecOverall.dwLeaderBoardID = m_OverallLeaderBoard.dwLeaderBoardID;
        pSpecOverall.dwNumStats = 2;
        pSpecOverall.pStats = (XONLINE_STAT*)&m_OverallRatingStats[ m_nUsers ];

        m_OverallRatingStats[ m_nUsers ].Clear();

        m_nUsers++;

        // return index of this user
        return m_nUsers - 1;
    }


    HRESULT RequestStats()
    {
        Assert( NULL == m_hTaskStatRead );  // can only process 1 task at a time
#ifdef XONLINE_OFFLINE
        HRESULT hr = S_OK;
        m_hTaskStatRead = (XONLINETASK_HANDLE)-1;
#else
        m_eState = STATE_STAT_READ_PLAYER_DATA;
        m_hrStatusStatRead = XOnlineStatRead( NUMSTATS( m_nUsers ), m_Specs, NULL, &m_hTaskStatRead);
#endif

        return m_hrStatusStatRead;
    }

    // HRESULT CancelPendingRequest() no reason to implement at this time.

    inline void TaskContinueEnumGetData( XONLINETASK_HANDLE &hTask, HRESULT &hr, XONLINE_STAT *pStat, int &nTasksStillRunning )
    {
        if ( hTask )
        {
            TASKCONTINUEWITHERROR( hTask, hr);

            if (XONLINETASK_S_RUNNING == hr)
            {
                // still running
                nTasksStillRunning++;
            }
            else
            {
                if ( SUCCEEDED( hr ) )
                {
                    DWORD dwReturnedResults = 0;
                    DWORD dwLeaderBoardSize = 0;
                    XONLINE_STAT_USER User;  // we do not care about user info.

                    hr = XOnlineStatLeaderEnumerateGetResults(
                                                hTask,
                                                1,  // dwUserCount,
                                                &User,
                                                1,  // dwStatCount,
                                                pStat,
                                                &dwLeaderBoardSize,
                                                &dwReturnedResults,
                                                0,                          // dwExtraBufferSize
                                                NULL );                     // pExtraBuffer
                    Assert( XONLINETASK_S_SUCCESS == hr );  // what possible errors???
                }
                else
                {
                    XONLINESTATSERROR( hr );
                }

                XOnlineTaskClose( hTask);
                hTask = NULL;
            }
        }
    }

    HRESULT CancelPendingRequest()
    {
        if ( m_hTaskStatRead ) 
        {
            m_hrStatusStatRead = XOnlineTaskClose( m_hTaskStatRead);
            m_hTaskStatRead = NULL;
        }

#ifndef XSTATS_NO_MINMAX_RATING_REQUEST
        if ( m_hTaskEnumTrackMin ) 
        {
            m_hrStatusEnumTrackMin = XOnlineTaskClose( m_hTaskEnumTrackMin);
            m_hTaskEnumTrackMin = NULL;
        }

        if ( m_hTaskEnumTrackMax ) 
        {
            m_hrStatusEnumTrackMax = XOnlineTaskClose( m_hTaskEnumTrackMax);
            m_hTaskEnumTrackMax = NULL;
        }

        if ( m_hTaskEnumOverallMin ) 
        {
            m_hrStatusEnumOverallMin = XOnlineTaskClose( m_hTaskEnumOverallMin);
            m_hTaskEnumOverallMin = NULL;
        }

        if ( m_hTaskEnumOverallMax ) 
        {
            m_hrStatusEnumOverallMax = XOnlineTaskClose( m_hTaskEnumOverallMax);
            m_hTaskEnumOverallMax = NULL;
        }
#endif XSTATS_NO_MINMAX_RATING_REQUEST

        m_eState = STATE_DONE;

        return S_OK;
    }

    // $STATISTICS: current API does not allow for retrieval of Min and Max HACKHACK here to get it.
    HRESULT TaskContinue()
    {
#ifdef XONLINE_OFFLINE
        omiting implementation at this time
#else
        switch ( m_eState )
        {
            case STATE_STAT_READ_PLAYER_DATA:
            {
                Assert( m_hTaskStatRead );

                TASKCONTINUEWITHERROR( m_hTaskStatRead, m_hrStatusStatRead);

                if (XONLINETASK_S_SUCCESS == m_hrStatusStatRead)
                {
                    m_hrStatusStatRead = XOnlineStatReadGetResult( m_hTaskStatRead, m_nUsers * 2, m_Specs, 0, NULL);
                    Assert( XONLINETASK_S_SUCCESS == m_hrStatusStatRead );  // what possible errors???
                    XOnlineTaskClose( m_hTaskStatRead);
                    m_hTaskStatRead = NULL;   

#ifndef XSTATS_NO_MINMAX_RATING_REQUEST

                    // we now know how many players are within both leaderboards
                    // lets enumerate ratings for players 1 and MAX
                    WORD wXONLINE_STAT_RATING = XONLINE_STAT_RATING;

                    // get the track leaderboard Max rating
                    m_hrStatusEnumTrackMax = XOnlineStatLeaderEnumerate(    
                                                    NULL,                       // pxuidPagePivot
                                                    1,                          // dwPageStart
                                                    1,                          // dwPageSize,
                                                    m_TrackLeaderBoard.dwLeaderBoardID,
                                                    1,                          // dwNumStatsPerUser,
                                                    &wXONLINE_STAT_RATING,      // pStatsPerUser
                                                    NULL,                       // hWorkEvent,
                                                    &m_hTaskEnumTrackMax);
                    if ( FAILED( m_hrStatusEnumTrackMax ) )
                    {
                        XONLINESTATSERROR( m_hrStatusEnumTrackMax );
                    }

                    // get the track leaderboard Min rating
                    m_hrStatusEnumTrackMin = XOnlineStatLeaderEnumerate(    
                                                    NULL,                       // pxuidPagePivot
                                                    m_TrackLeaderBoard.Size.lValue, // dwPageStart
                                                    1,                          // dwPageSize,
                                                    m_TrackLeaderBoard.dwLeaderBoardID,
                                                    1,                          // dwNumStatsPerUser,
                                                    &wXONLINE_STAT_RATING,      // pStatsPerUser
                                                    NULL,                       // hWorkEvent,
                                                    &m_hTaskEnumTrackMin);
                    if ( FAILED( m_hrStatusEnumTrackMin ) )
                    {
                        XONLINESTATSERROR( m_hrStatusEnumTrackMin );
                    }

                    // get the Overall leaderboard Max rating
                    m_hrStatusEnumOverallMax = XOnlineStatLeaderEnumerate(    
                                                    NULL,                       // pxuidPagePivot
                                                    1,                          // dwPageStart
                                                    1,                          // dwPageSize,
                                                    m_OverallLeaderBoard.dwLeaderBoardID,
                                                    1,                          // dwNumStatsPerUser,
                                                    &wXONLINE_STAT_RATING,      // pStatsPerUser
                                                    NULL,                       // hWorkEvent,
                                                    &m_hTaskEnumOverallMax);
                    if ( FAILED( m_hrStatusEnumOverallMax ) )
                    {
                        XONLINESTATSERROR( m_hrStatusEnumOverallMax );
                    }

                    // get the Overall leaderboard Min rating
                    m_hrStatusEnumOverallMin = XOnlineStatLeaderEnumerate(    
                                                    NULL,                       // pxuidPagePivot
                                                    m_OverallLeaderBoard.Size.lValue, // dwPageStart
                                                    1,  // dwPageSize,
                                                    m_OverallLeaderBoard.dwLeaderBoardID,
                                                    1,                          // dwNumStatsPerUser,
                                                    &wXONLINE_STAT_RATING,      // pStatsPerUser
                                                    NULL,                       // hWorkEvent,
                                                    &m_hTaskEnumOverallMin);
                    if ( FAILED( m_hrStatusEnumOverallMin ) )
                    {
                        XONLINESTATSERROR( m_hrStatusEnumOverallMin );
                    }
#endif // XSTATS_NO_MINMAX_RATING_REQUEST


                }

                m_eState = STATE_STAT_READ_LEADERBOARD_DATA;
                break;
            }

            case STATE_STAT_READ_LEADERBOARD_DATA:
            {
                int nTasksStillRunning = 0;

#ifndef XSTATS_NO_MINMAX_RATING_REQUEST
                TaskContinueEnumGetData( m_hTaskEnumTrackMin, m_hrStatusEnumTrackMin, &m_TrackLeaderBoard.MinRating, nTasksStillRunning );
                TaskContinueEnumGetData( m_hTaskEnumTrackMax, m_hrStatusEnumTrackMax, &m_TrackLeaderBoard.MaxRating, nTasksStillRunning );
                TaskContinueEnumGetData( m_hTaskEnumOverallMin, m_hrStatusEnumOverallMin, &m_OverallLeaderBoard.MinRating, nTasksStillRunning );
                TaskContinueEnumGetData( m_hTaskEnumOverallMax, m_hrStatusEnumOverallMax, &m_OverallLeaderBoard.MaxRating, nTasksStillRunning );
#endif // XSTATS_NO_MINMAX_RATING_REQUEST

                if ( 0 == nTasksStillRunning )
                {
                    m_eState = STATE_DONE;
                }
            }

        }

#endif

        return S_OK;
    }
};

extern RatingConstants g_RatingConstants;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
class CStatUpdateEndgamePlayerStats : public CXOnlineTask
{
public:

    XONLINE_STAT_SPEC   m_Specs[ MAX_LOCALPLAYERS * 2 ];
    RacerStatsRecord    m_RaceStats[ MAX_LOCALPLAYERS ];
    OverallStatsRecord  m_Overall[ MAX_LOCALPLAYERS ];

    DWORD               m_dwLeaderBoardID;
    int                 m_nUsers;
    //XONLINETASK_HANDLE  m_hTask;
    //HRESULT             m_hrStatus;

    // $STATISTICS(PERF): local racer data for ratings and for general stats update is 2 fields of redundancy.
    //                    note a big perf issues so opted to keep redundancy rather than complicate code
    CStatRatingSystemPlayerData     m_Ratings;

    bool                m_bGotLocalPlayerStatData;
    bool                m_bGotOpponentPlayerRatingStatData;

    // RatingEquation data and objects
    DWORD               m_dwNumRacers;
    RPlayer             m_TrackRatings[ MAX_NUM_PLAYERS ];      // same order as FinishTable
    RPlayer             m_OverallRatings[ MAX_NUM_PLAYERS ];    // same order as FinishTable 

    enum 
    {
        STATE_INIT,
        STATE_STAT_GET,
        STATE_STAT_READ,
        STATE_STAT_WRITE,
        STATE_DONE
    }                   m_eState;

public:
    CStatUpdateEndgamePlayerStats()
    {
        m_hTask = NULL;
        memset( m_Specs, 0, sizeof(m_Specs));
        memset( m_RaceStats, 0, sizeof(m_RaceStats));
    }


    HRESULT BeginStatUpdate()
    {
        Assert( sizeof(RacerStatsRecord)/sizeof(XONLINE_STAT) == RacerStatsRecord::NUMBER_OF_STATS );
        Assert( NULL == m_hTask );  // can only process 1 task at a time

        //m_dwLeaderBoardID = LeaderBoardID( gTitleScreenVars.iLevelNum );
        m_dwLeaderBoardID = LeaderBoardID( GetLevelNum(StartData.LevelDir) );
        m_hrStatus = S_FALSE;
        m_nUsers = 0;
        m_eState = STATE_INIT;
        m_dwNumRacers = 0;

        m_bGotLocalPlayerStatData = FALSE;
        m_bGotOpponentPlayerRatingStatData = FALSE;

        // $SINGLEPLAYER gotta get this single player stuff worked out...
        //PLAYER *pPlayer;   
        //for ( pPlayer = PLR_PlayerHead ; pPlayer ; pPlayer = pPlayer->next)
        //{
        //    if ( PLAYER_LOCAL == pPlayer->type  && !pPlayer->XOnlineInfo.bIsGuestOfUser )
        //    {
        //          Assert( m_nUsers < MAX_LOCALPLAYERS);
        //         g_StatIncrementMatchesStarted.AddLocalPlayer( pPlayer->XOnlineInfo.pXOnlineUser->xuid );
        //         m_nUsers++;
        //
        //    }
        //}

        // prep the ratings retrieval class
        m_Ratings.Reset( m_dwLeaderBoardID );

        // add each player to ratings retrieval, same order as FinishTable
        for (int i = 0 ; i < MAX_NUM_PLAYERS ; i++)
        {
            if ( FinishTable[i].Time )
            {
                DWORD dwPlayerID = FinishTable[i].Player->PlayerID;

                LONG lPlayerIndex = PlayerIndexFromPlayerID( dwPlayerID );

                m_Ratings.AddUser( PlayerList[ lPlayerIndex ].xuid );
                m_dwNumRacers++;
            }
        }

        m_Ratings.RequestStats();

        // retrieve all stats for local player
        if( IsLoggedIn( 0 ) )
        {
            PLAYER* pPlayer = &Players[0]; //GetPlayerFromPlayerID( LocalPlayerID );
            m_Specs[ m_nUsers * 2 ].xuidUser = pPlayer->XOnlineInfo.pXOnlineUser->xuid;
            m_Specs[ m_nUsers * 2 ].dwLeaderBoardID = m_dwLeaderBoardID;
            m_Specs[ m_nUsers * 2 ].dwNumStats = RacerStatsRecord::NUMBER_OF_WRITEABLE_STATS;
            m_Specs[ m_nUsers * 2 ].pStats = (XONLINE_STAT*)&m_RaceStats[ m_nUsers ];

            m_RaceStats[ m_nUsers ].Clear();

            // $STATISTICS Once a real formula for overall ranking has been decided, get other stats
            m_Specs[ m_nUsers * 2 + 1 ].xuidUser = pPlayer->XOnlineInfo.pXOnlineUser->xuid;
            m_Specs[ m_nUsers * 2 + 1 ].dwLeaderBoardID = STAT_LB_RACING_OVERALL;
            m_Specs[ m_nUsers * 2 + 1 ].dwNumStats = OverallStatsRecord::NUMBER_OF_WRITEABLE_STATS;
            m_Specs[ m_nUsers * 2 + 1 ].pStats = (XONLINE_STAT*)&m_Overall[ m_nUsers ];

            m_Overall[ m_nUsers ].Clear();

            m_nUsers = 1;

        }


#ifdef XONLINE_OFFLINE
        HRESULT hr = S_OK;
        m_hTask = (XONLINETASK_HANDLE)-1;
#else
        HRESULT hr = XOnlineStatRead( m_nUsers * 2, m_Specs, NULL, &m_hTask);
#endif
        // OnlineTasks_Add( handle );  I want to process my result with using single instance global structures.
        m_eState = STATE_STAT_READ;

        return hr;
    }

    HRESULT CancelPendingRequest()
    {
        if ( NULL == m_hTask )
            return S_FALSE;  // none pending
    
        HRESULT hr = XOnlineTaskClose( m_hTask);
        m_hTask = NULL;

        m_Ratings.CancelPendingRequest();
        
        m_eState = STATE_DONE;

        return hr;
    }

    HRESULT TaskContinue()
    {
#ifdef XONLINE_OFFLINE
        Assert( (XONLINETASK_HANDLE)-1 == m_hTask );

        for ( int nStat = 0; nStat < m_nUsers; nStat++ )
        {
            m_RaceStats[ nStat ].RandomFillData();
        }
        m_hTask = NULL;
        m_hrStatus = S_OK;
        m_eState = STATE_DONE;
#else
        switch ( m_eState )
        {
            case STATE_STAT_READ:
            {
                // pump task until complete
                if (!m_bGotLocalPlayerStatData)
                {
                    Assert( m_hTask != (XONLINETASK_HANDLE)-1);
                    TASKCONTINUEWITHERROR( m_hTask, m_hrStatus);

                    if (XONLINETASK_S_SUCCESS == m_hrStatus)
                    {
                        m_hrStatus = XOnlineStatReadGetResult( m_hTask, m_nUsers * 2, m_Specs, 0, NULL);
                        Assert( XONLINETASK_S_SUCCESS == m_hrStatus );  // what possible errors???

                        m_bGotLocalPlayerStatData = TRUE;

                        XOnlineTaskClose( m_hTask);

                        // HACKHACK  this task has multiple handles, set this handle to -1, so not NULL
                        m_hTask = (XONLINETASK_HANDLE)-1;  
                    }

                    if ( FAILED( m_hrStatus ) )
                    {
                        CancelPendingRequest();
                    }
                }

                if (!m_bGotOpponentPlayerRatingStatData)
                {
                    m_Ratings.TaskContinue();

                    if ( m_Ratings.m_eState == m_Ratings.STATE_DONE )
                    {
                        m_bGotOpponentPlayerRatingStatData = TRUE;

                    }
                }

                // if both retrievals are don, we are ready to calc and write the stats
                if ( m_bGotLocalPlayerStatData && m_bGotOpponentPlayerRatingStatData )
                {
                    // HACKHACK  if this task is -1 then must be reset back to NULL before next step
                    m_hTask = NULL;  
                    CalculateStatsAndBeginWrite();
                }

                break;
            }

            case STATE_STAT_WRITE:
                TASKCONTINUEWITHERROR( m_hTask, m_hrStatus);

                if (XONLINETASK_S_SUCCESS == m_hrStatus)
                {
                    XOnlineTaskClose( m_hTask);
                    m_hTask = NULL;
                    m_eState = STATE_DONE;
                }
                break;
        }

#endif

        return m_hrStatus;
    }

    HRESULT CalculateStatsAndBeginWrite()
    {
        m_hrStatus = E_FAIL;

        // RATING: prepare the list of players for use with ratings calulation

        // note that we can access stats array using same order
        // because we added users to collect in same order as in the FinishTable array
        // though FinishTable "can" have holes in it so nRacePos is contigious index into Ratings arrays
        DWORD dwRacePos = 0;
        for (int i = 0 ; i < MAX_NUM_PLAYERS ; i++)
        {
            if ( FinishTable[i].Time )
            {
//                DWORD dwPlayerID = FinishTable[i].Player->PlayerID;

//                LONG lPlayerIndex = PlayerIndexFromPlayerID( dwPlayerID );

                m_TrackRatings[ i ].Rating = Rating::RatingFromLongLong( m_Ratings.m_TrackRatingStats[dwRacePos].Rating.llValue );
                m_TrackRatings[ i ].dwRaceFinishPos = dwRacePos;
                m_TrackRatings[ i ].bEverLedRace = FALSE;
                m_TrackRatings[ i ].bLedMostLaps = FALSE;
                m_TrackRatings[ i ].dwRacesCompleted = m_Ratings.m_TrackRatingStats[dwRacePos].RacesCompleted.lValue;

                m_OverallRatings[ i ].Rating = Rating::RatingFromLongLong( m_Ratings.m_OverallRatingStats[dwRacePos].Rating.llValue );
                m_OverallRatings[ i ].dwRaceFinishPos = dwRacePos;
                m_OverallRatings[ i ].bEverLedRace = FALSE;
                m_OverallRatings[ i ].bLedMostLaps = FALSE;
                m_OverallRatings[ i ].dwRacesCompleted = m_Ratings.m_OverallRatingStats[dwRacePos].RacesCompleted.lValue;

                dwRacePos++;
            }
        }

        // RATING: Calc new rating for local player.on the given track
        Rating TrackCalc(   g_RatingConstants, 
                            m_Ratings.m_TrackLeaderBoard.MinRating.llValue, 
                            m_Ratings.m_TrackLeaderBoard.MaxRating.llValue);

        TrackCalc.UpdateRatings(    m_TrackRatings[ PLR_LocalPlayer->RaceFinishPos ], 
                                    m_TrackRatings, 
                                    &m_TrackRatings[ m_dwNumRacers] );

        // RATING: Calc new rating for local player.on the overall leaderboard
        Rating OverallCalc( g_RatingConstants, 
                            m_Ratings.m_OverallLeaderBoard.MinRating.llValue, 
                            m_Ratings.m_OverallLeaderBoard.MaxRating.llValue);

        OverallCalc.UpdateRatings(  m_OverallRatings[ PLR_LocalPlayer->RaceFinishPos ], 
                                    m_OverallRatings, 
                                    &m_OverallRatings[ m_dwNumRacers] );

        // $SINGLEPLAYER add code for handling players here.
        PLAYER* pPlayer = GetPlayerFromPlayerID( LocalPlayerID );
        RacerStatsRecord* pRacerStats = &m_RaceStats[0];
        for ( int nStat = 0; nStat < m_nUsers; nStat++ )
        {
            m_Overall[ nStat ].ResetTypes();

            // $STATISTICS need calcuation for overall rating
            m_Overall[ nStat ].Rating.llValue = Rating::RatingToLongLong( m_OverallRatings[ PLR_LocalPlayer->RaceFinishPos ].Rating );
            m_Overall[ nStat ].RacesCompleted.lValue++;

            pRacerStats->ResetTypes();

            pRacerStats->RacesCompleted.lValue++;
            pRacerStats->TotalMSeconds.lValue += pPlayer->RaceFinishTime;
        
        
            LEVELINFO *pLevel = GetLevelInfo(GetLevelNum(StartData.LevelDir)); // GetLevelInfo(GameSettings.Level );
            pRacerStats->TotalMeters.lValue += (long)( pPlayer->car.Laps * pLevel->Length ); 

            // $STATISTICS rating increases with each race completed...
            pRacerStats->Rating.llValue = Rating::RatingToLongLong( m_TrackRatings[ PLR_LocalPlayer->RaceFinishPos ].Rating );

            if ( pPlayer->RaceFinishPos == 0 )
            {
                pRacerStats->RacesWon.lValue++;
            }

            // $HACK - best split is now best lap, cleanup code later.
#ifdef USE_SPLIT_TIMES
            int nSplits = min( pPlayer->car.NextSplit, MAX_SPLIT_TIMES);
            for (int nSplit = 0; nSplit < nSplits; nSplit++)
            {
                if ( pPlayer->car.SplitTime[ nSplit ] < m_RaceStats[0].BestSplitTime.lValue )
                {
                    pRacerStats->BestSplitTime.lValue = pPlayer->car.SplitTime[ nSplit ];
                    pRacerStats->BestSplitCar.lValue = pPlayer->car.CarType + 1;
                }
            }
#else
            if ( ( pPlayer->car.BestLapTime < m_RaceStats[0].BestSplitTime.lValue ) || 
                 ( 0 == pRacerStats->BestSplitCar.lValue ) )  
            {
                pRacerStats->BestSplitTime.lValue = pPlayer->car.BestLapTime;
                pRacerStats->BestSplitCar.lValue = pPlayer->car.CarType + 1;
            }
#endif

        }

        m_hrStatus = XOnlineStatWrite( m_nUsers * 2, m_Specs, NULL, &m_hTask);
        m_eState = STATE_STAT_WRITE;

        return m_hrStatus;
    }
};


// HACKHACK: remove following bool defs when bool is fixed
#pragma warning( disable : 4800 ) // warning C4800: 'long' : forcing value to bool 'true' or 'false' (performance warning)
#undef bool 


struct PlayerModeKey
{
    XUID    xuid;
    bool    bBattleMode;
    bool operator==(const PlayerModeKey& Other)
    {
        return xuid.qwUserID == Other.xuid.qwUserID && bBattleMode == Other.bBattleMode;
    }
};

class compare
{
public:
    bool operator()( const XUID s, const XUID t ) const
    { 
        return ( s.qwUserID < t.qwUserID ); 
    }
    bool operator()( const PlayerModeKey s, const PlayerModeKey t ) const
    { 
        return ( s.xuid.qwUserID < t.xuid.qwUserID ) || ( s.bBattleMode < t.bBattleMode ); 
    }
};


typedef std::map< XUID, CStatRacer*, compare > RacerStats;
typedef std::map< XUID, CStatBattler*, compare > BattlerStats;
typedef std::map< PlayerModeKey, CStatUserTrackRanks*, compare > UserTrackRanks;

class CStatRacerCache : public RacerStats 
{
public:
    void PurgeStats()
    {

        for ( iterator pStat = begin(); pStat != end(); pStat++ )
            delete pStat->second;
        clear();
    }
};

class CStatBattlerCache : public BattlerStats
{
public:
    void PurgeStats()
    {
        for ( iterator pStat = begin(); pStat != end(); pStat++ )
            delete pStat->second;
        clear();
    }
};

class CStatUserTrackRanksCache : public UserTrackRanks
{
public:
    void PurgeStats()
    {
        for ( iterator pStat = begin(); pStat != end(); pStat++ )
            delete pStat->second;
        clear();
    }
};

extern CStatRacerCache          g_RacerStats;
extern CStatBattlerCache        g_BattlerStats;
extern CStatUserTrackRanksCache g_UserTrackRanks;

extern void StatsLocalPlayersStartingMatch();
extern void StatsLocalPlayersExitingMatch();
extern void StatsTestIncrementUpdateMatchesStarted();
extern void StatUpdateEndgamePlayerStats();

//$HACKHACK: remove following bool defs when bool is fixed

#define bool long

#endif // NET_STATISTICS_H
