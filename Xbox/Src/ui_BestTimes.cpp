//-----------------------------------------------------------------------------
// File: ui_BestTimes.cpp
//
// Desc: UI implementation
//
// Re-Volt (Generic) Copyright (c) Probe Entertainment 1998
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"
#include "main.h"           // TimeStep
#include "timing.h"         // SaveTrackTimes
#include "LevelLoad.h"      // GAMETYPE_*
#include "player.h"         // for ui_TitleScreen.h to work
#include "Text.h"           // DrawMenuText
#include "ui_Menu.h"        // MENU_DEFAULT
#include "ui_MenuDraw.h"    // DrawSpruBox
#include "ui_MenuText.h"   // re-volt strings
#include "ui_Confirm.h"
#include "ui_TitleScreen.h"
#include "ui_StateEngine.h"
#include "ui_EnterName.h"
#include "ui_SelectTrack.h"
#include "ui_BestTimes.h"


static VOID CreateBestTimesMenu( MENU_HEADER* pMenuHeader, MENU* pMenu );
static BOOL HandleBestTimesMenu( MENU_HEADER* pMenuHeader, DWORD dwInput );
static VOID DrawBestTimes( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem, int itemIndex );

#define MENU_BESTTIMES_XPOS             100
#define MENU_BESTTIMES_YPOS             150




////////////////////////////////////////////////////////////////
//
// Best Times menu
//
////////////////////////////////////////////////////////////////
static MENU Menu_BestTimes = 
{
    TEXT_BESTTRIALTIMES,
    MENU_DEFAULT | MENU_CENTRE_X,           // Menu type
    CreateBestTimesMenu,                    // Create menu function
    HandleBestTimesMenu,                    // Input handler function
    NULL,                                   // Menu draw function
    MENU_BESTTIMES_XPOS,                    // X coord
    MENU_BESTTIMES_YPOS,                    // Y Coord
};

// Best Times - select level
static MENU_ITEM MenuItem_BestTimesLevel = 
{
    TEXT_SELECTTRACK,                       // Text label index
    MENU_DATA_WIDTH_TEXT,                   // Space needed to draw item data
    NULL,                                   // Data (Menu to set up game and then run it)
    DrawBestTimes,                          // Draw Function
};

// Best Times - select level In-Game
static MENU_ITEM MenuItem_BestTimesLevelInGame = 
{
    TEXT_NONE,                              // Text label index
    340,                                    // Space needed to draw item data
    NULL,                                   // Data (Menu to set up game and then run it)
    DrawBestTimes,                          // Draw Function
};


// Create
void CreateBestTimesMenu( MENU_HEADER* pMenuHeader, MENU* pMenu )
{
    if( GameSettings.Level == LEVEL_FRONTEND )
    {
        pMenuHeader->AddMenuItem( &MenuItem_BestTimesLevel );
    }
    else
    {
        pMenuHeader->AddMenuItem( &MenuItem_BestTimesLevelInGame );
    }
}


BOOL HandleBestTimesMenu( MENU_HEADER* pMenuHeader, DWORD dwInput )
{
    if( pMenuHeader->m_pCurrentItem->TextIndex == TEXT_SELECTTRACK )
    {
        switch( dwInput )
        {
            case MENU_INPUT_LEFT:
                TrackSelectPrevTrack();
                return TRUE;
            case MENU_INPUT_RIGHT:
                TrackSelectNextTrack();
                return TRUE;
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
#endif //SHIPPING
        }
    }

    switch( dwInput )
    {
        case MENU_INPUT_BACK:
            g_BestTimesStateEngine.Return( STATEENGINE_TERMINATED );
            return TRUE;
    }

    return FALSE;
}




/////////////////////////////////////////////////////////////////////
//
// Draw Best Times
//
/////////////////////////////////////////////////////////////////////
void DrawBestTimes( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem, int itemIndex )
{
    int j;
    FLOAT xPos, yPos;
    LEVELINFO *levelInfo;
    long col;
    bool avail;

    // Draw the current level for which the times are shown
    levelInfo = GetLevelInfo(gTitleScreenVars.iLevelNum);
    if (levelInfo == NULL) return;
    avail = IsLevelTypeAvailable(gTitleScreenVars.iLevelNum, gTitleScreenVars.mirror, gTitleScreenVars.reverse);

    // Choose text colour
    if( avail )
        col = MENU_TEXT_RGB_CHOICE;
    else
        col = MENU_TEXT_RGB_LOLITE;
    
    xPos = pMenuHeader->m_XPos + pMenuHeader->m_ItemTextWidth + MENU_TEXT_GAP;
    yPos = pMenuHeader->m_YPos + (MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP) * itemIndex;

    if( GameSettings.Level == LEVEL_FRONTEND )
    {
        swprintf( MenuBuffer, L"%s", levelInfo->strName );

        if( gTitleScreenVars.mirror )
        {
            wcscat( MenuBuffer, L" " );
            wcscat( MenuBuffer, TEXT_TABLE(TEXT_MIRROR_ABREV_PARENTHESIS) );
        }
        if( gTitleScreenVars.reverse )
        {
            wcscat( MenuBuffer, L" " );
            wcscat( MenuBuffer, TEXT_TABLE(TEXT_REVERSE_ABREV_PARENTHESIS) );
        }

        DrawMenuText( xPos, yPos, col, MenuBuffer, pMenuHeader->m_ItemDataWidth );
    }

    // Draw the table of best times
    xPos = pMenuHeader->m_XPos + 6;
    //yPos -= (MAX_RECORD_TIMES * (MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP) + 2 * MENU_TEXT_GAP);
    yPos += MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP + 2 * MENU_TEXT_GAP;

    DrawNewSpruBox( gMenuWidthScale * (xPos- MENU_TEXT_GAP) - 10, 
                    gMenuHeightScale * (yPos - MENU_TEXT_GAP) - 10, 
                    gMenuWidthScale * (42 * MENU_TEXT_WIDTH + 2 * MENU_TEXT_GAP) + 20, 
                    gMenuHeightScale * (MAX_RECORD_TIMES * (MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP) + 2 * MENU_TEXT_GAP) + 20 );

    BeginTextState();

    if (avail)
        col = MENU_TEXT_RGB_NORMAL;
    else
        col = MENU_TEXT_RGB_LOLITE;

    for( j = 0 ; j < MAX_RECORD_TIMES; j++ )
    {
        if( TrackRecords.RecordLap[j].Time == MAX_LAP_TIME )
            swprintf(MenuBuffer, L"--------- %16.16S %16.16S", TrackRecords.RecordLap[j].Player, TrackRecords.RecordLap[j].Car);
        else
            swprintf(MenuBuffer, L"%02d:%02d:%03d %16.16S %16.16S", MINUTES(TrackRecords.RecordLap[j].Time), SECONDS(TrackRecords.RecordLap[j].Time), THOUSANDTHS(TrackRecords.RecordLap[j].Time), TrackRecords.RecordLap[j].Player, TrackRecords.RecordLap[j].Car);

        DumpTextReal(xPos, yPos, MENU_TEXT_WIDTH, MENU_TEXT_HEIGHT, col, MenuBuffer);
        yPos += MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP;
    }
}




CBestTimesStateEngine g_BestTimesStateEngine;

//-----------------------------------------------------------------------------
// Name: Process()
// Desc: Main control function for the state engine
//-----------------------------------------------------------------------------
HRESULT CBestTimesStateEngine::Process()
{
    enum
    {
        BESTTIMES_STATE_BEGIN = STATEENGINE_STATE_BEGIN,
        BESTTIMES_STATE_MAINLOOP,
    };

    switch( m_State )
    {
        case BESTTIMES_STATE_BEGIN:
            // Initialise
            gTitleScreenVars.iCurrentPlayer = 0;
            gTitleScreenVars.numberOfPlayers = 1;

            if( GameSettings.Level == LEVEL_FRONTEND )
            {
                GameSettings.GameType = GAMETYPE_TRIAL;
            }
            else
            {
                SaveTrackTimes();
            }

            // Make sure level is valid
            TrackSelectPrevTrack();
            TrackSelectNextTrack();

//$REMOVED -The above track selection will load track times
//          LoadTrackTimes( gTitleScreenVars.iLevelNum, gTitleScreenVars.mirror, gTitleScreenVars.reverse );
//$END_REMOVAL

            // Set the best times menu and camera position
            g_pMenuHeader->SetNextMenu( &Menu_BestTimes );
            g_pTitleScreenCamera->SetNewPos( TITLESCREEN_CAMPOS_BESTTIMES );

            m_State = BESTTIMES_STATE_MAINLOOP;
            break;

        case BESTTIMES_STATE_MAINLOOP:
            // Nothing to do, as control is in menu code
            break;
    }

    // Handle active menus
    g_pMenuHeader->HandleMenus();

    return S_FALSE;
}



