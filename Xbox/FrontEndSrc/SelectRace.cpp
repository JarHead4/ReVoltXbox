//-----------------------------------------------------------------------------
// File: SelectRace.cpp
//
// Desc: 
//
// Copyright (c)  Microsoft Corporation.  All rights reserved.
//-----------------------------------------------------------------------------
#include "Revolt.h"
#include "Menu2.h"
#include "MenuDraw.h"   // DrawSpruBox
#include "Settings.h"   // RegistrySettings
#include "XBInput.h"
#include "MenuText.h"   // re-volt strings
#include "TitleScreen.h"
#include "StateEngine.h"
#include "SelectRace.h"

extern BOOL g_bMenuActive;
extern MENU Menu_TopLevel;

#define MENU_SELECTRACE_XPOS            Real(100)
#define MENU_SELECTRACE_YPOS            Real(150)

static VOID CreateSelectRaceMenu( MENU_HEADER* pMenuHeader, MENU* pMenu );
static BOOL SelectRaceHandler( DWORD input, MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem );

CSelectRaceStateEngine g_SelectRaceStateEngine;




//-----------------------------------------------------------------------------
// Select Race Menu
//-----------------------------------------------------------------------------
MENU Menu_SelectRace = 
{
    TEXT_SELECTRACE,
	MENU_IMAGE_SELECTRACE,                  // Menu title bar image enum
    TITLESCREEN_CAMPOS_RACE,                // Camera pos index
    MENU_DEFAULT | MENU_CENTRE_X,           // Menu type
    SPRU_COL_RANDOM,                        // Spru colour
    CreateSelectRaceMenu,                   // Create menu function
    MENU_SELECTRACE_XPOS,                   // X coord
    MENU_SELECTRACE_YPOS,
    0,                                      // Current Item index
    &Menu_TopLevel,                         // Parent Menu (OverWritten in Create)
};

// Select Race - Single Race
MENU_ITEM MenuItem_SingleRace = 
{
    TEXT_SINGLERACE,                        // Text label index
    0,                                      // Space needed to draw item data
    NULL,                                   // Data
    NULL,                                   // Draw Function
 	SelectRaceHandler,					    // Input handler
	0,                                      // Flags
};

// Select Race - Time Trial
MENU_ITEM MenuItem_TimeTrial = 
{
    TEXT_TIMETRIAL,                         // Text label index
    0,                                      // Space needed to draw item data
    NULL,                                   // Data
    NULL,                                   // Draw Function
 	SelectRaceHandler,					    // Input handler
	0,                                      // Flags
};

// Select Race - Practice
MENU_ITEM MenuItem_Practice = 
{
    TEXT_PRACTICE,                          // Text label index
    0,                                      // Space needed to draw item data
    NULL,                                   // Data
    NULL,                                   // Draw Function
	SelectRaceHandler,					    // Input handler
	0,                                      // Flags
};

// Select Race - Battle Tag
MENU_ITEM MenuItem_BattleTag = 
{
    TEXT_BATTLETAG,                         // Text label index
    0,                                      // Space needed to draw item data
    NULL,                                   // Data
    NULL,                                   // Draw Function
	SelectRaceHandler,					    // Input handler
	0,                                      // Flags
};

// Select Race - Stunt Area
MENU_ITEM MenuItem_StuntArea = 
{
    TEXT_STUNTAREA,                         // Text label index
    0,                                      // Space needed to draw item data
    NULL,                                   // Data
    NULL,                                   // Draw Function
 	SelectRaceHandler,					    // Input handler
	0,                                      // Flags
};




// Create Function
VOID CreateSelectRaceMenu( MENU_HEADER* pMenuHeader, MENU* pMenu )
{
    // Initialise variables
    g_TitleScreenData.iCurrentPlayer  = 0;
    g_TitleScreenData.numberOfPlayers = 1;
    GameSettings.RandomCars  = FALSE;
    GameSettings.RandomTrack = FALSE;

    // Add menu items
    AddMenuItem( pMenuHeader, pMenu, &MenuItem_SingleRace );
    AddMenuItem( pMenuHeader, pMenu, &MenuItem_TimeTrial );
    AddMenuItem( pMenuHeader, pMenu, &MenuItem_Practice );
    AddMenuItem( pMenuHeader, pMenu, &MenuItem_BattleTag );
    AddMenuItem( pMenuHeader, pMenu, &MenuItem_StuntArea );
}




// Menu handler
BOOL SelectRaceHandler( DWORD input, MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem )
{
	if( input == MENU_INPUT_SELECT )
	{
		switch( pMenuItem->TextIndex )
		{
			case TEXT_SINGLERACE:
				GameSettings.GameType = GAMETYPE_SINGLE;
				GameSettings.RandomCars = g_TitleScreenData.RandomCars;
				GameSettings.RandomTrack = g_TitleScreenData.RandomTrack;
				break;

			case TEXT_TIMETRIAL:
				GameSettings.GameType = GAMETYPE_TRIAL;
				break;

			case TEXT_PRACTICE:
				GameSettings.GameType = GAMETYPE_PRACTICE;
				break;

			case TEXT_BATTLETAG:
				GameSettings.GameType = GAMETYPE_BATTLE;
				break;

			case TEXT_STUNTAREA:
				GameSettings.GameType = GAMETYPE_TRAINING;
				break;
		}

		g_TitleScreenData.bGameTypeSelected = TRUE;
		SetNextMenu( &gMenuHeader, NULL );
		return TRUE;
	}

	return MenuDefHandler( input, pMenuHeader, pMenu, pMenuItem );
}




//-----------------------------------------------------------------------------
// The Select Race state engine
//-----------------------------------------------------------------------------
HRESULT CSelectRaceStateEngine::Process()
{
	enum
	{
		SELECTRACE_STATE_BEGIN = STATEENGINE_STATE_BEGIN,
		SELECTRACE_STATE_MAINLOOP,
	};

	switch( m_State )
	{
		case SELECTRACE_STATE_BEGIN:
			g_TitleScreenData.bGameTypeSelected = FALSE;
			SetNextMenu( &gMenuHeader, &Menu_SelectRace );
			m_State = SELECTRACE_STATE_MAINLOOP;
			break;

		case SELECTRACE_STATE_MAINLOOP:
			// Do nothing until the user exits the menu system
			if( g_bMenuActive == FALSE )
				Return();
			break;
	}

	return S_FALSE;
}




