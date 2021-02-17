//-----------------------------------------------------------------------------
// File: RatingEquation.h
//
// Desc: Calculate player ratings
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------

#ifndef RATING_EQUATION_H
#define RATING_EQUATION_H

#include <xtl.h>


typedef DOUBLE RATING_TYPE;

// &&& temporary
struct RPlayer
{
    RATING_TYPE Rating;             // Current player rating
    DWORD       dwRaceFinishPos;    // Finish position (0=winner)
    BOOL        bEverLedRace;       // TRUE if ever led the race
    BOOL        bLedMostLaps;       // TRUE if led the most laps
    DWORD       dwRacesCompleted;   // Number of completed races
};


//-----------------------------------------------------------------------------
// Name: RatingConstants
// Desc: The parameters used to determine player ratings. Split out into its
//       own structure so it can be refactored into a global leaderboard or
//       a content package.
//-----------------------------------------------------------------------------
struct RatingConstants
{
    RATING_TYPE Base;               // Base for Elo equaltion, e.g. e or 10
    RATING_TYPE ScaleFactor;        // Sigmoid curve temperature value
    RATING_TYPE MaxDisplayedRating; // Max rating in Re-Volt UI (min is 0.0)
    RATING_TYPE MinBonusLaps;       // Min race laps for MajorityLeadBonus to apply
    RATING_TYPE LeadBonus;          // Bonus awarded for ever leading the race
    RATING_TYPE MajorityLeadBonus;  // Bonus awarded for leading most of the race
    RATING_TYPE MaxWeight;          // Weight is max amount rating can change
    RATING_TYPE MinWeight;          // Max and min specify range of weights
    RATING_TYPE RaceThreshold;      // Races completed; triggers weight change
    RATING_TYPE RatingThreshold;    // Player rating; triggers weight change
};


//-----------------------------------------------------------------------------
// Name: Rating
// Desc: Encapsulates the rating functions used to compute new player ratings
//-----------------------------------------------------------------------------
class Rating
{

    RatingConstants     m_RatingConstants; // Rating equation parameters
    mutable RATING_TYPE m_MinRating;       // Min player ranking in player database
    mutable RATING_TYPE m_MaxRating;       // Max player ranking in player database
    mutable BOOL        m_bIsMinMaxRatingChanged; // TRUE if min or max rating changed

public:

    Rating( const RatingConstants&, LONGLONG Min, LONGLONG Max );
    ~Rating();

    VOID SetRating( RPlayer&, LONGLONG llRating ) const;
    LONGLONG GetStoredRating( const RPlayer& ) const;
    VOID UpdateRatings( RPlayer& Player, const RPlayer* OpponentListStart,
                        const RPlayer* OpponentListEnd ) const;
    RATING_TYPE GetDisplayedRating( const RPlayer& ) const;

    BOOL IsMinMaxRatingChanged() const;
    LONGLONG GetMinRating() const;
    LONGLONG GetMaxRating() const;
    VOID SetClean();


    static LONGLONG    RatingToLongLong( RATING_TYPE );
    static RATING_TYPE RatingFromLongLong( LONGLONG );

private:

    RATING_TYPE GetNewRating( const RPlayer&, const RPlayer& ) const;
    RATING_TYPE GetScore( const RPlayer&, const RPlayer& ) const;
    RATING_TYPE GetBonus( const RPlayer& ) const;
    RATING_TYPE GetExpectedScore( RATING_TYPE ) const;
    RATING_TYPE GetWeightRange() const;
    RATING_TYPE GetWeight( RATING_TYPE, RATING_TYPE ) const;
    RATING_TYPE GetRatingChange( RATING_TYPE, RATING_TYPE, RATING_TYPE, 
                                 RATING_TYPE ) const;
    RATING_TYPE GetRatingRange() const;
    RATING_TYPE GetScaledRating( RATING_TYPE ) const;
    RATING_TYPE GetRating( const RPlayer& ) const;
    VOID        SetRating( RPlayer&, RATING_TYPE ) const;
    RATING_TYPE GetRacesCompleted( const RPlayer& ) const;
    VOID UpdateMinMax( RATING_TYPE ) const;

private:

    // Disabled
    Rating();
    Rating( const Rating& );
    Rating& operator=( const Rating& );

};



#endif // RATING_EQUATION