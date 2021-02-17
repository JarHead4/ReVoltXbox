//-----------------------------------------------------------------------------
// File: RatingEquation.cpp
//
// Desc: Calculate player ratings
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------

#include "RatingEquation.h"
#include "revolt.h"
#include "Player.h"


//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------
const RATING_TYPE One = RATING_TYPE( 1.0 );
const RATING_TYPE Zero = RATING_TYPE( 0.0 );


//-----------------------------------------------------------------------------
// Name: Round
// Desc: &&& Move this function to wherever the math functions reside
//-----------------------------------------------------------------------------
template< typename Input >
DWORD Round( Input f )
{
    f += ( f > Input( 0.0 ) ) ? Input( 0.5 ) : -Input( 0.5 );
    return DWORD( f );
}


//-----------------------------------------------------------------------------
// Name: Ctor
// Desc: Initialize
//-----------------------------------------------------------------------------
Rating::Rating( const RatingConstants& RatingConstants, LONGLONG llMin,
                LONGLONG llMax )
:
    m_RatingConstants( RatingConstants ),
    m_MinRating( RatingFromLongLong( llMin ) ),
    m_MaxRating( RatingFromLongLong( llMax ) ),
    m_bIsMinMaxRatingChanged( FALSE )
{
}


//-----------------------------------------------------------------------------
// Name: Dtor
// Desc: Destroy
//-----------------------------------------------------------------------------
Rating::~Rating()
{
}


//-----------------------------------------------------------------------------
// Name: SetRating
// Desc: Set the rating for a player from the rating database (leaderboard)
//-----------------------------------------------------------------------------
VOID Rating::SetRating( RPlayer& Player, LONGLONG llRating ) const
{
    SetRating( Player, RatingFromLongLong( llRating ) );
}


//-----------------------------------------------------------------------------
// Name: GetStoredRating
// Desc: Extract a player rating in terms of how it must be stored in the
//       rating database (leaderboard)
//-----------------------------------------------------------------------------
LONGLONG Rating::GetStoredRating( const RPlayer& Player ) const
{
    return RatingToLongLong( GetRating( Player ) );
}


//-----------------------------------------------------------------------------
// Name: UpdateRatings
// Desc: Update the ratings for the player based on his post race status
//-----------------------------------------------------------------------------
VOID Rating::UpdateRatings( RPlayer& Player, 
                            const RPlayer* pOpponentListStart,
                            const RPlayer* pOpponentListEnd ) const
{
    // Compute sum of ratings against all opponents
    const RPlayer* pOpponent;
    for( pOpponent = pOpponentListStart; pOpponent != pOpponentListEnd; ++pOpponent )
    {
        // lets reuse the list of players for all local players; skip this Player in list
        if (Player.dwRaceFinishPos != pOpponent->dwRaceFinishPos)
        {
            RATING_TYPE Rating = GetNewRating( Player, *pOpponent );
            SetRating( Player, Rating );
        }
    }
}


//-----------------------------------------------------------------------------
// Name: GetDisplayedRating
// Desc: Get the rating that's displayed in the UI based on actual rating
//       The displayed rating is always in the range:
//               0 - m_RatingConstants.MaxDisplayedRating
//-----------------------------------------------------------------------------
RATING_TYPE Rating::GetDisplayedRating( const RPlayer& Player ) const
{
    RATING_TYPE PlayerRating = GetRating( Player );
    RATING_TYPE ScaledRating = GetScaledRating( PlayerRating );

    return ScaledRating * m_MaxRating;
}


//-----------------------------------------------------------------------------
// Name: IsMinMaxRatingChanged
// Desc: TRUE if either the min or max rating has been adjusted since we
//       constructed the object or called SetClean. If they have been changed,
//       we need to update the player database with the new values.
//-----------------------------------------------------------------------------
BOOL Rating::IsMinMaxRatingChanged() const
{
    return m_bIsMinMaxRatingChanged;
}


//-----------------------------------------------------------------------------
// Name: GetMinRating
// Desc: Get the current minimum rating in LONGLONG format
//-----------------------------------------------------------------------------
LONGLONG Rating::GetMinRating() const
{
    return RatingToLongLong( m_MinRating );
}


//-----------------------------------------------------------------------------
// Name: GetMaxRating
// Desc: Get the current maximum rating in LONGLONG format
//-----------------------------------------------------------------------------
LONGLONG Rating::GetMaxRating() const
{
    return RatingToLongLong( m_MaxRating );
}


//-----------------------------------------------------------------------------
// Name: SetClean
// Desc: Call this function to indicate that new values of either the min
//       or max rating have been written to the player database.
//-----------------------------------------------------------------------------
VOID Rating::SetClean()
{
    m_bIsMinMaxRatingChanged = FALSE;
}


//-----------------------------------------------------------------------------
// Name: GetNewRating
// Desc: Get the new rating for the player.
//-----------------------------------------------------------------------------
RATING_TYPE Rating::GetNewRating( const RPlayer& Player, 
                                  const RPlayer& Opponent ) const // private
{
    // Determine score and bonus
    RATING_TYPE Score = GetScore( Player, Opponent );
    RATING_TYPE Bonus = GetBonus( Player );

    // Grab the existing ratings
    RATING_TYPE RacesCompleted = GetRacesCompleted( Player );
    RATING_TYPE PlayerRating = GetRating( Player );
    RATING_TYPE OpponentRating = GetRating( Opponent );

    // Compute new rating
    RATING_TYPE RatingDifference = OpponentRating - PlayerRating;
    RATING_TYPE ExpectedScore = GetExpectedScore( RatingDifference );
    RATING_TYPE ScaledRating = GetScaledRating( PlayerRating );
    RATING_TYPE Weight = GetWeight( ScaledRating, RacesCompleted );
    RATING_TYPE RatingChange = GetRatingChange( Weight, Score, Bonus, ExpectedScore );
    RATING_TYPE NewRating = PlayerRating + RatingChange;

    // Update cached min and max ratings as necessary
    UpdateMinMax( NewRating );

    return NewRating;
}


//-----------------------------------------------------------------------------
// Name: GetScore
// Desc: Get the score between the players. If Player won, the score is 1.0.
//       If Player lost, the score is 0.0. If it was a tie, the score is 0.5.
//-----------------------------------------------------------------------------
RATING_TYPE Rating::GetScore( const RPlayer& Player, 
                              const RPlayer& Opponent ) const // private
{
    // Player finished ahead of their opponent
    if( Player.dwRaceFinishPos < Opponent.dwRaceFinishPos )
        return One;

    // Player finished behind their opponent
    if( Player.dwRaceFinishPos > Opponent.dwRaceFinishPos )
        return Zero;

    // Tie
    return RATING_TYPE( 0.5 );
}


//-----------------------------------------------------------------------------
// Name: GetBonus
// Desc: Gets any bonus points associated with the race. Bonus points are based
//       on the NASCAR system. There are bonuses for leading the race for the 
//       most laps, and bonuses for ever leading. The winner always receives
//       a bonus for leading.
//-----------------------------------------------------------------------------
RATING_TYPE Rating::GetBonus( const RPlayer& Player ) const // private
{
    RATING_TYPE Bonus = Zero;

    if( Player.bEverLedRace )
        Bonus += m_RatingConstants.LeadBonus;
    if( Player.bLedMostLaps )
        Bonus += m_RatingConstants.MajorityLeadBonus;

    return Bonus;
}


//-----------------------------------------------------------------------------
// Name: GetExpectedScore
// Desc: Returns the expected score given the ratings difference between
//       two players. The resulting number is between 0.0 and 1.0
//-----------------------------------------------------------------------------
RATING_TYPE Rating::GetExpectedScore( RATING_TYPE RatingDifference ) const // private
{
    // This formula is central to the rating system. The Elo Chess rating
    // formula is the same. It uses a Base of 10 and a ScaleFactor of 400.

    assert( m_RatingConstants.Base != Zero );
    assert( m_RatingConstants.ScaleFactor != Zero );

    RATING_TYPE Power = RATING_TYPE( pow( m_RatingConstants.Base, 
                                          RatingDifference / 
                                          m_RatingConstants.ScaleFactor ) );
    RATING_TYPE ExpectedScore = One / ( One + Power );
    assert( ExpectedScore >= Zero );
    assert( ExpectedScore <= One );
    return ExpectedScore;
}


//-----------------------------------------------------------------------------
// Name: GetWeightRange
// Desc: Returns the difference between MinWeight and MaxWeight
//-----------------------------------------------------------------------------
RATING_TYPE Rating::GetWeightRange() const // private
{
    assert( m_RatingConstants.MaxWeight >= m_RatingConstants.MinWeight );  
    return m_RatingConstants.MaxWeight - m_RatingConstants.MinWeight;
}


//-----------------------------------------------------------------------------
// Name: GetWeight
// Desc: Returns the weight of the match given information about the player.
//       Weight is the maximum amount a player's rating can change as the
//       result of a single race between two players.
//-----------------------------------------------------------------------------
RATING_TYPE Rating::GetWeight( RATING_TYPE ScaledRating, 
                               RATING_TYPE RacesCompleted ) const // private
{
    // Players with a low rating get a higher weight. If they win a race,
    // they stand to gain more points. Similarly, new players also have
    // a higher weight. This helps get racers to their correct skill rating
    // quickly.
    if( ScaledRating < m_RatingConstants.RatingThreshold ||
        RacesCompleted < m_RatingConstants.RaceThreshold )
        return m_RatingConstants.MaxWeight;

    // Established players have a smaller weight coefficient, so they
    // experience less rating change. The best players will have a weight
    // of MinWeight.
    RATING_TYPE Weight = m_RatingConstants.MinWeight;
    Weight += ( GetWeightRange() / ( One - m_RatingConstants.RatingThreshold ) ) *
              ( One - ScaledRating );
    return Weight;
}


//-----------------------------------------------------------------------------
// Name: GetRatingChange
// Desc: Returns the change in a player's rating
//-----------------------------------------------------------------------------
RATING_TYPE Rating::GetRatingChange( RATING_TYPE Weight, RATING_TYPE Score, 
                                     RATING_TYPE Bonus, 
                                     RATING_TYPE ExpectedScore ) const // private
{
    // The change in a player's rating is based on the difference between
    // their actual score and their expected score. A player that defeats
    // a player when they are expected to win experiences a smaller rating
    // change than a player who wins when they are expected to lose.
    return Weight * ( ( Score + Bonus ) - ExpectedScore );
}


//-----------------------------------------------------------------------------
// Name: GetRatingRange
// Desc: Returns the range of ratings in the player database
//-----------------------------------------------------------------------------
RATING_TYPE Rating::GetRatingRange() const // private
{
    assert( m_MaxRating >= m_MinRating );
    return m_MaxRating - m_MinRating;
}


//-----------------------------------------------------------------------------
// Name: GetScaledRating
// Desc: Returns a rating in the range 0.0 - 1.0
//-----------------------------------------------------------------------------
RATING_TYPE Rating::GetScaledRating( RATING_TYPE Rating ) const // private
{
    assert( GetRatingRange() > Zero );
    RATING_TYPE ScaledRating = ( Rating - m_MinRating ) / GetRatingRange();
    assert( ScaledRating >= Zero );
    assert( ScaledRating <= One );
    return ScaledRating;
}


//-----------------------------------------------------------------------------
// Name: GetRating
// Desc: Extracts the current rating information from the player record
//-----------------------------------------------------------------------------
RATING_TYPE Rating::GetRating( const RPlayer& Player ) const // private
{
    return Player.Rating;
}


//-----------------------------------------------------------------------------
// Name: SetRating
// Desc: Set the rating in the player record
//-----------------------------------------------------------------------------
VOID Rating::SetRating( RPlayer& Player, RATING_TYPE Rating ) const // private
{
    Player.Rating = Rating;
}


//-----------------------------------------------------------------------------
// Name: RatingToLongLong
// Desc: Convert RATING_TYPE to LONGLONG
//-----------------------------------------------------------------------------
LONGLONG Rating::RatingToLongLong( RATING_TYPE Rating ) // static
{
    // Convert to positive rating
    BOOL bPositive = TRUE;
    if( Rating < Zero )
    {
        bPositive = FALSE;
        Rating = -Rating;
    }

    RATING_TYPE Integer;
    RATING_TYPE Decimal;

    // Divide rating into its fractional and integer parts
    Decimal = modf( Rating, &Integer );

    // Convert to LONGLONG. The upper LONG stores the integer portion and
    // the lower DWORD stores the decimal portion.
    LONGLONG llDecimal = LONGLONG( Round( Decimal * RATING_TYPE( DWORD( 0xFFFFFFFF ) ) ) );
    LONGLONG llRating = LONGLONG( Integer );
    llRating <<= 32;
    llRating |= llDecimal;

    // Back to negative if necessary
    if( !bPositive )
        llRating = -llRating;

    return llRating;
}


//-----------------------------------------------------------------------------
// Name: RatingFromLongLong
// Desc: Convert LONGLONG to RATING_TYPE
//-----------------------------------------------------------------------------
RATING_TYPE Rating::RatingFromLongLong( LONGLONG llRating ) // static
{
    // Convert to positive rating
    BOOL bPositive = TRUE;
    if( llRating < 0 )
    {
        bPositive = FALSE;
        llRating = -llRating;
    }

    // Convert to RATING_TYPE. The upper DWORD stores the integer portion
    // and the lower DWORD stores the decimal portion
    RATING_TYPE Integer = RATING_TYPE( DWORD( llRating >> 32uL ) );
    RATING_TYPE Decimal = RATING_TYPE( DWORD( llRating ) & 0x00000000FFFFFFFF ) /
                          RATING_TYPE( 0xFFFFFFFF );
    RATING_TYPE Rating = Integer + Decimal;

    // Back to negative if necessary
    if( !bPositive )
        Rating = -Rating;

    return Rating;
}


//-----------------------------------------------------------------------------
// Name: GetRacesCompleted
// Desc: Extracts the number of races this player has completed from the 
//       player record
//-----------------------------------------------------------------------------
RATING_TYPE Rating::GetRacesCompleted( const RPlayer& Player ) const // private
{
    return RATING_TYPE( Player.dwRacesCompleted );
}


//-----------------------------------------------------------------------------
// Name: UpdateMinMax
// Desc: Update cached min and max ratings as necessary
//-----------------------------------------------------------------------------
VOID Rating::UpdateMinMax( RATING_TYPE NewRating ) const // private
{
    if( NewRating < m_MinRating )
    {
        m_MinRating = NewRating;
        m_bIsMinMaxRatingChanged = TRUE;
    }
    if( NewRating > m_MaxRating )
    {
        m_MaxRating = NewRating;
        m_bIsMinMaxRatingChanged = TRUE;
    }
}


//-----------------------------------------------------------------------------
