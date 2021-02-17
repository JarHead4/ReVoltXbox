//-----------------------------------------------------------------------------
// File: ui_SelectTrack.cpp
//
// Desc: 
//
// Re-Volt (Generic) Copyright (c) Probe Entertainment 1998
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"
#include "main.h"
#include "geom.h"
#include "particle.h"
#include "model.h"
#include "aerial.h"
#include "newcoll.h"
#include "body.h"
#include "car.h"
#include "ctrlread.h"
#include "object.h"
#include "light.h"
#include "obj_init.h"
#include "player.h"
#include "ai.h"
#include "ai_init.h"
#include "EditObject.h"
#include "drawobj.h"
#include "move.h"
#include "timing.h"
#include "visibox.h"
#include "spark.h"
#include "field.h"
#include "weapon.h"
#include "input.h"
#include "ui_menudraw.h"
#include "initplay.h"
#include "pickup.h"
#include "SoundEffectEngine.h"
#include "Settings.h"
#include "Text.h"
#include "Cheats.h"

#include "ui_Menu.h"
#include "ui_MenuText.h"
#include "ui_TitleScreen.h"
#include "ui_StateEngine.h"
#include "ui_SelectTrack.h"
#include "ui_RaceOverview.h"
#include "ui_WaitingRoom.h"
#include "content.h"


static void CreateTrackSelectMenu( MENU_HEADER* pMenuHeader, MENU* pMenu );
static BOOL HandleTrackSelectMenu( MENU_HEADER* pMenuHeader, DWORD dwInput );
static void DrawTrackSelectMenu( MENU_HEADER* pMenuHeader, MENU* pMenu );

#define MENU_TRACKSELECT_XPOS           320.0f
#define MENU_TRACKSELECT_YPOS           350.0f





////////////////////////////////////////////////////////////////
//
// Track Select Menu
//
////////////////////////////////////////////////////////////////
extern MENU Menu_TrackSelect = 
{
    TEXT_SELECTTRACK,
    MENU_DEFAULT | MENU_NOBOX,              // Menu type
    CreateTrackSelectMenu,                  // Create menu function
    HandleTrackSelectMenu,                  // Input handler function
    DrawTrackSelectMenu,                    // Menu draw function
    MENU_TRACKSELECT_XPOS,                  // X coord
    MENU_TRACKSELECT_YPOS,                  // Y Coord
};


// Create
void CreateTrackSelectMenu( MENU_HEADER* pMenuHeader, MENU* pMenu )
{
    // Don't draw a default box
    pMenuHeader->m_pMenu->dwFlags |= MENU_NOBOX;
}


BOOL HandleTrackSelectMenu( MENU_HEADER* pMenuHeader, DWORD dwInput )
{
    switch( dwInput )
    {
//$BUGBUG: This #ifdef is a temp workaround for May02_TechBeta (and possibly future beta releases).
/// Eventually, we should support track mirroring and reversing.
#ifdef SHIPPING
        // Don't let user mirror or reverse track
#else
        case MENU_INPUT_UP:
            TrackSelectMirrorTrack();
            return TRUE;

        case MENU_INPUT_DOWN:
            TrackSelectReverseTrack();
            return TRUE;
#endif

        case MENU_INPUT_LEFT:
            TrackSelectPrevTrack();
            return TRUE;

        case MENU_INPUT_RIGHT:
            TrackSelectNextTrack();
            return TRUE;

        case MENU_INPUT_BACK:
            g_SelectTrackStateEngine.Return( STATEENGINE_TERMINATED );
            return TRUE;

        case MENU_INPUT_SELECT:
            if( IsLevelTypeAvailable( gTitleScreenVars.iLevelNum, gTitleScreenVars.mirror, gTitleScreenVars.reverse ) )
            {
                g_SelectTrackStateEngine.Return( STATEENGINE_COMPLETED );
                return TRUE;
            }
            break;
    }

    return FALSE;
}


void TrackSelectMirrorTrack()
{
    if( GetLevelInfo(gTitleScreenVars.iLevelNum)->TrackType == TRACK_TYPE_BATTLE ) return;

    gTitleScreenVars.mirror = !gTitleScreenVars.mirror;

    LoadTrackTimes( gTitleScreenVars.iLevelNum, gTitleScreenVars.mirror, gTitleScreenVars.reverse );
}

void TrackSelectReverseTrack()
{
    if( GetLevelInfo(gTitleScreenVars.iLevelNum)->TrackType == TRACK_TYPE_BATTLE ) return;
    if( GetLevelInfo(gTitleScreenVars.iLevelNum)->TrackType == TRACK_TYPE_USER )   return;

    gTitleScreenVars.reverse = !gTitleScreenVars.reverse;

    LoadTrackTimes( gTitleScreenVars.iLevelNum, gTitleScreenVars.mirror, gTitleScreenVars.reverse );
}


void TrackSelectPrevTrack()
{
    LEVELINFO *pLevelInfo;

    // Select previous allowed level
    do 
    {
        gTitleScreenVars.iLevelNum--;
        if (gTitleScreenVars.iLevelNum < 0) 
            gTitleScreenVars.iLevelNum = GameSettings.LevelNum - 1;

        pLevelInfo = GetLevelInfo(gTitleScreenVars.iLevelNum);
        gTrackScreenLevelNum = gTitleScreenVars.iLevelNum;
    } while (!IsLevelSelectable(gTitleScreenVars.iLevelNum));

    // Setup info 
    strncpy(StartData.LevelDir, pLevelInfo->szDir, MAX_PATH);
    strncpy(RegistrySettings.LevelDir, pLevelInfo->szDir, MAX_PATH);

    // Load level image and best times
    LoadTrackTimes(gTitleScreenVars.iLevelNum, gTitleScreenVars.mirror, gTitleScreenVars.reverse);

    // (Make sure directory exists
    if( !DoesLevelExist(gTitleScreenVars.iLevelNum) )
        SetLevelUnavailable( gTitleScreenVars.iLevelNum );
}

void TrackSelectNextTrack()
{
    LEVELINFO *pLevelInfo;

    do 
    {
        gTitleScreenVars.iLevelNum++;
        if (gTitleScreenVars.iLevelNum >= GameSettings.LevelNum) 
            gTitleScreenVars.iLevelNum = 0;

        pLevelInfo = GetLevelInfo(gTitleScreenVars.iLevelNum);
        gTrackScreenLevelNum = gTitleScreenVars.iLevelNum;
    } while (!IsLevelSelectable(gTitleScreenVars.iLevelNum));

    // Setup Info
    pLevelInfo = GetLevelInfo(gTitleScreenVars.iLevelNum);
    strncpy(StartData.LevelDir, pLevelInfo->szDir, MAX_PATH);
    strncpy(RegistrySettings.LevelDir, pLevelInfo->szDir, MAX_PATH);
        
    // Load level image and Best Times
    LoadTrackTimes( gTitleScreenVars.iLevelNum, gTitleScreenVars.mirror, gTitleScreenVars.reverse );

    // (Make sure directory exists
    if( !DoesLevelExist(gTitleScreenVars.iLevelNum) )
        SetLevelUnavailable( gTitleScreenVars.iLevelNum );
}




/////////////////////////////////////////////////////////////////////
//
// Draw Track Name
//
/////////////////////////////////////////////////////////////////////
void DrawTrackSelectMenu( MENU_HEADER* menuHeader, MENU *menu )
{
    LEVELINFO* pLevelInfo = GetLevelInfo(gTitleScreenVars.iLevelNum);

    
    FLOAT fMaxItemWidth =  max( CalcMaxStringWidth( 1, &gTitleScreen_Text[TEXT_LENGTH] ),
                                CalcMaxStringWidth( 1, &gTitleScreen_Text[TEXT_DIFFICULTY] ) );

    FLOAT fMaxDataWidth = CalcMaxStringWidth( 4, &gTitleScreen_Text[TEXT_EASY] );
    
    FLOAT fTextWidth;
    swprintf( MenuBuffer, L"%d %s", 99999, gTitleScreen_Text[TEXT_METERS] );
    fTextWidth = g_pFont->GetTextWidth( MenuBuffer );
    if( fTextWidth > fMaxDataWidth )
        fMaxDataWidth = fTextWidth;

    FLOAT fMaxWidth = fMaxItemWidth + 20 + fMaxDataWidth;
    for( int i=0; i<GameSettings.LevelNum; i++ )
    {
        if( IsLevelSelectable(i) && GetLevelInfo(i) )
        {
            swprintf( MenuBuffer, L"%s %s%s", GetLevelInfo(i)->strName, TEXT_TABLE(TEXT_MIRROR_ABREV_PARENTHESIS), TEXT_TABLE(TEXT_REVERSE_ABREV_PARENTHESIS) );
            fMaxWidth = max( fMaxWidth, g_pFont->GetTextWidth( MenuBuffer ) );
        }
    }

    FLOAT xPos1 = menuHeader->m_XPos - (fMaxWidth)/2;
    FLOAT yPos  = menuHeader->m_YPos;
    FLOAT xPos2 = xPos1 + fMaxItemWidth + 20;

    DrawNewSpruBox( gMenuWidthScale  * (xPos1 - MENU_LEFT_PAD ),
                    gMenuHeightScale * (yPos - MENU_TOP_PAD ),
                    gMenuWidthScale  * (fMaxWidth + MENU_LEFT_PAD + MENU_RIGHT_PAD ),
                    gMenuHeightScale * (MENU_TEXT_HEIGHT*3 + MENU_TEXT_VSKIP*2 + MENU_TOP_PAD + MENU_BOTTOM_PAD ) );

    BeginTextState();

    // Track Name
    // $MD: $LOCALIZE
    swprintf( MenuBuffer, L"%s %s%s %s", pLevelInfo->strName, 
                                      (gTitleScreenVars.mirror)? TEXT_TABLE(TEXT_MIRROR_ABREV_PARENTHESIS) : L"",
                                      (gTitleScreenVars.reverse)? TEXT_TABLE(TEXT_REVERSE_ABREV_PARENTHESIS) : L"",
                                      (pLevelInfo->ObtainFlags & LEVEL_ONLINEONLY)? L"Online" : L"");

    long color = MENU_TEXT_RGB_LOLITE;

    if( IsLevelTypeAvailable( gTitleScreenVars.iLevelNum, gTitleScreenVars.mirror, gTitleScreenVars.reverse ) ) 
        color = MENU_TEXT_RGB_CHOICE;
    
    g_pFont->DrawText( menuHeader->m_XPos, yPos, color, MenuBuffer, XBFONT_CENTER_X );
    yPos += MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP * 2;

    // Track Length
    DrawMenuText( xPos1, yPos, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_LENGTH) );
    if( (pLevelInfo->TrackType == TRACK_TYPE_BATTLE) || (pLevelInfo->TrackType == TRACK_TYPE_TRAINING) ) 
    {
        DrawMenuText(xPos2, yPos, MENU_TEXT_RGB_NORMAL, L"n/a");
    } 
    else if( pLevelInfo->Length != ZERO ) 
    {
        swprintf(MenuBuffer, L"%d %s", (int)pLevelInfo->Length, gTitleScreen_Text[TEXT_METERS] );
        DrawMenuText(xPos2, yPos, MENU_TEXT_RGB_NORMAL, MenuBuffer );
    } 
    else 
    {
        DrawMenuText(xPos2, yPos, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_TRACK_UNKNOWN) );
    }
    yPos += MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP;

    // Difficulty
    DrawMenuText( xPos1, yPos, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_DIFFICULTY) );
    if( pLevelInfo->LevelClass  < RACE_CLASS_BRONZE )
        DrawMenuText(xPos2, yPos, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_TRACK_UNKNOWN) );
    else
        DrawMenuText(xPos2, yPos, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_EASY + pLevelInfo->LevelClass - 1));

    // If level not available, say so
    if( !IsLevelTypeAvailable( gTitleScreenVars.iLevelNum, gTitleScreenVars.mirror, gTitleScreenVars.reverse ) ) 
    {
        if( !(TIME2MS(TimerCurrent) & 128) ) 
        {
            swprintf( MenuBuffer, TEXT_TABLE(TEXT_TRACK_LOCKED) );
            fTextWidth = g_pFont->GetTextWidth( MenuBuffer );
            FLOAT xPos = (640 - fTextWidth) / 2;
            FLOAT yPos = (480 - (FLOAT)(MENU_TEXT_HEIGHT)) / 2;
            DrawNewSpruBox( gMenuWidthScale  * (xPos - MENU_TEXT_HSKIP) - 10, 
                            gMenuHeightScale * (yPos - MENU_TEXT_VSKIP) - 8,
                            gMenuWidthScale  * (fTextWidth + MENU_TEXT_HSKIP * 2) + 20,
                            gMenuHeightScale * (MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP * 2) + 25 );

            BeginTextState();
            DrawMenuText( xPos, yPos, MENU_TEXT_RGB_HILITE, MenuBuffer );
        }
    }
}

// $MD: added
//-----------------------------------------------------------------------------
// Name: SetHasSubscriptionLevels
// Desc: Allows the use of "premium levels" if we have a subscription
// NOTE: g_bHasSubscription is set when you sign on and cleared when you sign off
//-----------------------------------------------------------------------------
VOID SetHasSubscriptionLevels()
{
    for(int i = 0; i < GameSettings.LevelNum; i++)
    {
        LEVELINFO* pLevel = GetLevelInfo(i);
        if(pLevel->ObtainFlags & LEVEL_ONLINEONLY)
        {
            if(g_bHasSubscription)
                SetLevelSelectable(i);
            else
                SetLevelUnselectable(i);
        }
    }
}

CSelectTrackStateEngine g_SelectTrackStateEngine;

//-----------------------------------------------------------------------------
// Name: Process()
// Desc: Main control function for the state engine
//-----------------------------------------------------------------------------
HRESULT CSelectTrackStateEngine::Process()
{
    enum
    {
        SELECTTRACK_STATE_BEGIN = STATEENGINE_STATE_BEGIN,
        SELECTTRACK_STATE_MAINLOOP,
    };

    switch( m_State )
    {
        case SELECTTRACK_STATE_BEGIN:
        {
            if( GameSettings.RandomTrack )
            {
                // User can't pick a track. Skip ahead.
                Return( STATEENGINE_COMPLETED );
                break;
            }

            // re-call car select func if 'AllTracks' true
            //if( AllTracks )
            //    InitDefaultLevels();

            // $MD: added subscription levels
            SetHasSubscriptionLevels();


            // Load level image and best times
            LoadTrackTimes( gTitleScreenVars.iLevelNum, gTitleScreenVars.mirror, gTitleScreenVars.reverse );

            gTitleScreenVars.iLevelNum = GetLevelNum(RegistrySettings.LevelDir);
            if( gTitleScreenVars.iLevelNum == -1)
                gTitleScreenVars.iLevelNum = 0;
            LEVELINFO* pLevelInfo = GetLevelInfo(gTitleScreenVars.iLevelNum);
            strncpy(StartData.LevelDir, pLevelInfo->szDir, MAX_PATH);

            // Make sure directory exists
            if( !DoesLevelExist(gTitleScreenVars.iLevelNum) )
                SetLevelUnavailable( gTitleScreenVars.iLevelNum );

            // get next then prev track to ensure valid level for game type - BKK 03/05/99
            // PT: Changed to Prev -> Next so it goes to the first ones when playing stunt or battle
            TrackSelectPrevTrack();
            TrackSelectNextTrack();
        
            // Make sure track isn't mirrored or reversed if its a battle track
            if( GetLevelInfo(gTitleScreenVars.iLevelNum)->TrackType == TRACK_TYPE_BATTLE ) 
            {
                gTitleScreenVars.mirror  = FALSE;
                gTitleScreenVars.reverse = FALSE;
            }

            if( GetLevelInfo(gTitleScreenVars.iLevelNum)->TrackType == TRACK_TYPE_USER )
            {
                gTitleScreenVars.reverse = FALSE;
            }

            // Level number shown on the video screens
            gTrackScreenLevelNum = gTitleScreenVars.iLevelNum;

            // Set the track select menu and camera position
            g_pMenuHeader->SetNextMenu( &Menu_TrackSelect );
            g_pTitleScreenCamera->SetNewPos( TITLESCREEN_CAMPOS_TRACK_SELECT );

            m_State = SELECTTRACK_STATE_MAINLOOP;
            break;
        }

        case SELECTTRACK_STATE_MAINLOOP:
            // Nothing to do, as control is in the menus
            break;
    }

    // Handle the menus
    g_pMenuHeader->HandleMenus();

    return S_FALSE;
}



