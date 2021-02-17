//-----------------------------------------------------------------------------
// Top-level front end menu
//-----------------------------------------------------------------------------

#include "revolt.h"
#include "menu2.h"
#include "MenuDraw.h"   // DrawSpruBox
#include "text.h"       // BeginTextState
#include "settings.h"   // RegistrySettings

// menus
#include "Secrets.h"
#include "Options.h"
#include "XBInput.h"

// re-volt specific
#include "menutext.h"   // re-volt strings

// temporary includes?
#include "titlescreen.h"
#include "StateEngine.h"
#include "LiveSignOn.h"
#include "SelectRace.h"

extern BOOL g_bMenuActive;

// The old top-level menu
extern MENU Menu_PlayLive;
extern MENU Menu_SelectRace;
extern MENU Menu_SelectTrack;
extern MENU Menu_SelectCar;
extern MENU Menu_Overview;
extern MENU Menu_Connection;



#define MENU_TOPLEVEL_XPOS              Real(0)
#define MENU_TOPLEVEL_YPOS              Real(0)


extern BOOL    ArePlayersSignedOn();


static VOID CreateTopLevelMenu(MENU_HEADER *menuHeader, MENU *menu);
static BOOL TopLevelMenuHandler( DWORD input, MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem );

static VOID CreatePlayLiveMenu( MENU_HEADER* pMenuHeader, MENU* pMenu );
static BOOL PlayLiveMenuHandler( DWORD input, MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem );


BOOL g_bUsingNewFrontEnd = FALSE;



//-----------------------------------------------------------------------------
// Play Live state engine
//-----------------------------------------------------------------------------
class CPlayLiveStateEngine : public CStateEngine
{
public:
	virtual HRESULT Process();
} g_PlayLiveStateEngine;




//-----------------------------------------------------------------------------
// Single Player Game state engine
//-----------------------------------------------------------------------------
class CSinglePlayerGameStateEngine : public CStateEngine
{
public:
	virtual HRESULT Process();
} g_SinglePlayerGameStateEngine;




//-----------------------------------------------------------------------------
// System Link Game state engine
//-----------------------------------------------------------------------------
class CSystemLinkGameStateEngine : public CStateEngine
{
public:
	virtual HRESULT Process();
} g_SystemLinkGameStateEngine;




//-----------------------------------------------------------------------------
// Create Match state engine
//-----------------------------------------------------------------------------
class CCreateMatchStateEngine : public CStateEngine
{
public:
	virtual HRESULT Process();
} g_CreateMatchStateEngine;




//-----------------------------------------------------------------------------
// Select Track state engine
//-----------------------------------------------------------------------------
class CSelectTrackStateEngine : public CStateEngine
{
public:
	virtual HRESULT Process();
} g_SelectTrackStateEngine;




//-----------------------------------------------------------------------------
// Select Car state engine
//-----------------------------------------------------------------------------
class CSelectCarStateEngine : public CStateEngine
{
public:
	virtual HRESULT Process();
} g_SelectCarStateEngine;




//-----------------------------------------------------------------------------
// Connect state engine
//-----------------------------------------------------------------------------
class CConnectStateEngine : public CStateEngine
{
public:
	virtual HRESULT Process();
} g_ConnectStateEngine;




//-----------------------------------------------------------------------------
// Top Level Menu
//-----------------------------------------------------------------------------
MENU Menu_TopLevel = 
{
    TEXT_NONE,
    MENU_IMAGE_NONE,                        // Menu title bar image enum
    TITLESCREEN_CAMPOS_START,                        // Camera pos index
    MENU_DEFAULT | MENU_CENTRE_X | MENU_CENTRE_Y,   // Menu type
    SPRU_COL_RANDOM,                        // Spru colour
    CreateTopLevelMenu,                     // Create menu function
    MENU_TOPLEVEL_XPOS,                     // X coord
    MENU_TOPLEVEL_YPOS,                     // Y Coord
    0,                                      // Current Item index
    &Menu_TopLevel,                         // Parent Menu (OverWritten in Create)
};

// Top Level - Play Live!
static MENU_ITEM MenuItem_PlayLive = 
{
    TEXT_PLAYLIVE,                          // Text label index
    0,                                      // Space needed to draw item data
    &Menu_SelectRace,                       // Data
    NULL,                                   // Draw Function
	TopLevelMenuHandler,                    // Input handler
	0,                                      // Flags
};

// Top Level - Single player
static MENU_ITEM MenuItem_SinglePlayer = 
{
    TEXT_SINGLEPLAYER,                      // Text label index
    0,                                      // Space needed to draw item data
    &Menu_SelectRace,                       // Data
    NULL,                                   // Draw Function
	TopLevelMenuHandler,                    // Input handler
	0,                                      // Flags
};

// Top Level - Multi-player
static MENU_ITEM MenuItem_MultiPlayer = 
{
    TEXT_MULTIPLAYER,                       // Text label index
    0,                                      // Space needed to draw item data
    &Menu_Connection,                       // Data
    NULL,                                   // Draw Function
	MenuDefHandler,                         // Input handler
	0,                                      // Flags
};


// Top Level - System link
static MENU_ITEM MenuItem_SystemLink = 
{
    TEXT_SYSTEMLINK,                        // Text label index
    0,                                      // Space needed to draw item data
    &Menu_Connection,                       // Data
    NULL,                                   // Draw Function
	MenuDefHandler,                         // Input handler
	0,                                      // Flags
};

// Top Level - Secrets
MENU_ITEM MenuItem_Secrets = 
{
    TEXT_SECRETS,                           // Text label index
    0,                                      // Space needed to draw item data
    &Menu_Secrets,                          // Data
    NULL,                                   // Draw Function
	MenuDefHandler,                         // Input handler
	0,                                      // Flags
};

// Top Level - Options
static MENU_ITEM MenuItem_Options = 
{
    TEXT_OPTIONS,                           // Text label index
    0,                                      // Space needed to draw item data
    &Menu_Options,                          // Data
    NULL,                                   // Draw Function
	MenuDefHandler,                         // Input handler
	0,                                      // Flags
};




// Create Function
VOID CreateTopLevelMenu( MENU_HEADER* pMenuHeader, MENU* pMenu )
{
	// Re-enable auto demo running
//	g_TitleScreenData.bAllowDemoToRun = TRUE;
    g_TitleScreenData.bAllowDemoToRun = FALSE;

    // Switch off "Go-Back" icon
    pMenuHeader->NavFlags &= ~MENU_FLAG_GOBACK;

    // add menu items
    AddMenuItem( pMenuHeader, pMenu, &MenuItem_PlayLive );
    AddMenuItem( pMenuHeader, pMenu, &MenuItem_SinglePlayer );
    AddMenuItem( pMenuHeader, pMenu, &MenuItem_MultiPlayer );
    AddMenuItem( pMenuHeader, pMenu, &MenuItem_SystemLink );
	AddMenuItem( pMenuHeader, pMenu, &MenuItem_Secrets );
	AddMenuItem( pMenuHeader, pMenu, &MenuItem_Options );

    GameSettings.MultiType = MULTITYPE_NONE;
}




// Menu handler
BOOL TopLevelMenuHandler( DWORD input, MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem )
{
	if( input == MENU_INPUT_SELECT )
	{
		if( pMenuItem->TextIndex == TEXT_PLAYLIVE )
		{
			// Exit the menu system
		    pMenuHeader->pNextMenu = NULL;
		
			// Switch to the Play Live state engine
			g_PlayLiveStateEngine.MakeActive( NULL );

			g_bUsingNewFrontEnd = TRUE;

			return TRUE;
		}

		if( pMenuItem->TextIndex == TEXT_SINGLEPLAYER )
		{
			// Exit the menu system
		    pMenuHeader->pNextMenu = NULL;
		
			// Switch to the Single Player Game state engine
			g_SinglePlayerGameStateEngine.MakeActive( NULL );

			g_bUsingNewFrontEnd = TRUE;

			return TRUE;
		}

/*
		if( pMenuItem->TextIndex == TEXT_SYSTEMLINK )
		{
			// Exit the menu system
		    pMenuHeader->pNextMenu = NULL;
		
			// Switch to the Single Player Game state engine
			g_SystemLinkGameStateEngine.MakeActive( NULL );

			g_bUsingNewFrontEnd = TRUE;

			return TRUE;
		}
*/
	}

	return MenuDefHandler( input, pMenuHeader, pMenu, pMenuItem );
}




//-----------------------------------------------------------------------------
// Play Live Menu
//-----------------------------------------------------------------------------
MENU Menu_PlayLive = 
{
    TEXT_NONE,
    MENU_IMAGE_NONE,                        // Menu title bar image enum
    TITLESCREEN_CAMPOS_START,                        // Camera pos index
    MENU_DEFAULT | MENU_CENTRE_X | MENU_CENTRE_Y,   // Menu type
    SPRU_COL_RANDOM,                        // Spru colour
    CreatePlayLiveMenu,                     // Create menu function
    MENU_TOPLEVEL_XPOS,                     // X coord
    MENU_TOPLEVEL_YPOS,                     // Y Coord
    0,                                      // Current Item index
    &Menu_TopLevel,                         // Parent Menu (OverWritten in Create)
};

// Play Live! - Quick Match
static MENU_ITEM MenuItem_QuickMatch = 
{
    TEXT_QUICKMATCH,                        // Text label index
    0,                                      // Space needed to draw item data
    &Menu_SelectRace,                       // Data
    NULL,                                   // Draw Function
	PlayLiveMenuHandler,                    // Input handler
	0,                                      // Flags
};

// Play Live! - Find a Match
static MENU_ITEM MenuItem_FindMatch = 
{
    TEXT_FINDMATCH,                         // Text label index
    0,                                      // Space needed to draw item data
    &Menu_SelectRace,                       // Data
    NULL,                                   // Draw Function
	PlayLiveMenuHandler,                    // Input handler
	0,                                      // Flags
};

// Play Live! - Friends
static MENU_ITEM MenuItem_Friends = 
{
    TEXT_FRIENDS,                           // Text label index
    0,                                      // Space needed to draw item data
    &Menu_SelectRace,                       // Data
    NULL,                                   // Draw Function
	PlayLiveMenuHandler,                    // Input handler
	0,                                      // Flags
};

// Play Live! - Create a Match
static MENU_ITEM MenuItem_CreateMatch = 
{
    TEXT_CREATEMATCH,                       // Text label index
    0,                                      // Space needed to draw item data
    &Menu_SelectRace,                       // Data
    NULL,                                   // Draw Function
	PlayLiveMenuHandler,                    // Input handler
	0,                                      // Flags
};

// Play Live! - Statistics
static MENU_ITEM MenuItem_Statistics = 
{
    TEXT_STATISTICS,                        // Text label index
    0,                                      // Space needed to draw item data
    &Menu_SelectRace,                       // Data
    NULL,                                   // Draw Function
	PlayLiveMenuHandler,                    // Input handler
	0,                                      // Flags
};




// Create Function
VOID CreatePlayLiveMenu( MENU_HEADER* pMenuHeader, MENU* pMenu )
{
    // add menu items
    AddMenuItem( pMenuHeader, pMenu, &MenuItem_QuickMatch );
    AddMenuItem( pMenuHeader, pMenu, &MenuItem_FindMatch );
    AddMenuItem( pMenuHeader, pMenu, &MenuItem_Friends );
    AddMenuItem( pMenuHeader, pMenu, &MenuItem_CreateMatch );
	AddMenuItem( pMenuHeader, pMenu, &MenuItem_Statistics );
}




// Menu Handler
BOOL PlayLiveMenuHandler( DWORD input, MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem )
{
	if( input == MENU_INPUT_SELECT )
	{
		if( pMenuItem->TextIndex == TEXT_CREATEMATCH )
		{
			// Switch to the Create Match state engine
			g_pActiveStateEngine->Call( &g_CreateMatchStateEngine );
			return TRUE;
		}
	}

	return MenuDefHandler( input, pMenuHeader, pMenu, pMenuItem );
}




//-----------------------------------------------------------------------------
// The Play Live state engine
//-----------------------------------------------------------------------------
HRESULT CPlayLiveStateEngine::Process()
{
	enum
	{
		PLAYLIVE_STATE_BEGIN = STATEENGINE_STATE_BEGIN,
		PLAYLIVE_STATE_LIVESIGNON,
		PLAYLIVE_STATE_MAINLOOP,
		PLAYLIVE_STATE_QUICKMATCH,
		PLAYLIVE_STATE_CREATEMATCH,
	};

	switch( m_State )
	{
		case PLAYLIVE_STATE_BEGIN:
		{
			g_bUsingNewFrontEnd = TRUE;

			if( ArePlayersSignedOn() )
			{
				m_State = PLAYLIVE_STATE_LIVESIGNON;
			}
			else
			{
				// If there's still a menu, wait till it transitions out of view
				if( gMenuHeader.pMenu ) 
					return S_FALSE;

				// Call the Live Sign On state engine
				Call( &g_LiveSignOnStateEngine );

				// Afterwards, advance tothe nextstate
				m_State = PLAYLIVE_STATE_LIVESIGNON;
			}

			break;
		}

		case PLAYLIVE_STATE_LIVESIGNON:
		{
			if( ArePlayersSignedOn() )
			{
				SetNextMenu( &gMenuHeader, &Menu_PlayLive );
				m_State = PLAYLIVE_STATE_MAINLOOP;
			}
			else
			{
				// User hit BACK or there was a problem

				// Go back to the top level menu
				Return();
				SetNextMenu( &gMenuHeader, &Menu_TopLevel );
				g_bUsingNewFrontEnd = FALSE;
				return E_FAIL;
			}
			break;
		}

		case PLAYLIVE_STATE_MAINLOOP:
			// Nothing much to do while the user is in the menu system
			break;
	}

	return S_FALSE;
}




//-----------------------------------------------------------------------------
// The Single Player Game state engine
//-----------------------------------------------------------------------------
HRESULT CSinglePlayerGameStateEngine::Process()
{
	enum
	{
		SINGLEPLAYERGAME_STATE_BEGIN = STATEENGINE_STATE_BEGIN,
		SINGLEPLAYERGAME_STATE_SELECTRACE,
		SINGLEPLAYERGAME_STATE_SELECTTRACK,
		SINGLEPLAYERGAME_STATE_SELECTCAR,
		SINGLEPLAYERGAME_STATE_GAMEOVERVIEW,
		SINGLEPLAYERGAME_STATE_STARTGAME,
	};

	switch( m_State )
	{
		case SINGLEPLAYERGAME_STATE_BEGIN:
			m_State = SINGLEPLAYERGAME_STATE_SELECTRACE;
			break;

		case SINGLEPLAYERGAME_STATE_SELECTRACE:
			// Switch control to the Select Race state engine
			Call( &g_SelectRaceStateEngine );

			// When that state engine returns control, be in the 
			// "Select Track" state
			m_State = SINGLEPLAYERGAME_STATE_SELECTTRACK;

			break;

		case SINGLEPLAYERGAME_STATE_SELECTTRACK:
			// Check whether the Select Race state engine succeeded in
			// selecting a race
			if( g_TitleScreenData.bGameTypeSelected )
			{
				// Advance to the select track state engine
				Call( &g_SelectTrackStateEngine );

				// When that state engine returns control, be in the 
				// "Track Selected" state
				m_State = SINGLEPLAYERGAME_STATE_SELECTCAR;
			}
			else
			{
				// User must have hit the BACK button
				g_bUsingNewFrontEnd = FALSE;
				Return();
				return E_FAIL;
			}
			break;

		case SINGLEPLAYERGAME_STATE_SELECTCAR:
			if( g_TitleScreenData.bTrackSelected )
			{
				// Advance to the select track state engine
				Call( &g_SelectCarStateEngine );

				// When that state engine returns control, be in the 
				// "Track Selected" state
				m_State = SINGLEPLAYERGAME_STATE_GAMEOVERVIEW;
			}
			else
			{
				// User must have hit the BACK button
				m_State = SINGLEPLAYERGAME_STATE_SELECTTRACK;
			}
			break;

		case SINGLEPLAYERGAME_STATE_GAMEOVERVIEW:
			SetNextMenu( &gMenuHeader, &Menu_Overview );
			m_State = SINGLEPLAYERGAME_STATE_STARTGAME;
			break;

		case SINGLEPLAYERGAME_STATE_STARTGAME:
			if( FALSE == g_bMenuActive )
			{
				// Return for now
				Return();
			}
			break;
	}

	return S_FALSE;
}


//-----------------------------------------------------------------------------
// The System Link Game state engine
//-----------------------------------------------------------------------------
HRESULT CSystemLinkGameStateEngine::Process()
{
	enum
	{
		SYSTEMLINKGAME_STATE_BEGIN = STATEENGINE_STATE_BEGIN,
		SYSTEMLINKGAME_STATE_CONNECT,
		SYSTEMLINKGAME_MAINLOOP,
	};

	switch( m_State )
	{
		case SYSTEMLINKGAME_STATE_BEGIN:
			GameSettings.GameType    = GAMETYPE_MULTI;
			GameSettings.RandomCars  = g_TitleScreenData.RandomCars;
			GameSettings.RandomTrack = g_TitleScreenData.RandomTrack;

			m_State = SYSTEMLINKGAME_STATE_CONNECT;
			break;

		case SYSTEMLINKGAME_STATE_CONNECT:
			// Switch control to the Select Race state engine
			Call( &g_ConnectStateEngine );

			// When that state engine returns control, be in the 
			// "Select Track" state
			m_State = SYSTEMLINKGAME_MAINLOOP;

			break;

		case SYSTEMLINKGAME_MAINLOOP:
			if( FALSE == g_bMenuActive )
			{
				// Return for now
				Return();
			}
			break;
	}

	return S_FALSE;
}




//-----------------------------------------------------------------------------
// The Create Match state engine
//-----------------------------------------------------------------------------
HRESULT CCreateMatchStateEngine::Process()
{
	enum
	{
		CREATEMATCH_STATE_BEGIN = STATEENGINE_STATE_BEGIN,
		CREATEMATCH_STATE_SELECTRACE,
		CREATEMATCH_STATE_SELECTTRACK,
		CREATEMATCH_STATE_SELECTCAR,
		CREATEMATCH_STATE_GAMEOVERVIEW,
		CREATEMATCH_STATE_STARTGAME,
	};

	switch( m_State )
	{
		case CREATEMATCH_STATE_BEGIN:
			m_State = CREATEMATCH_STATE_SELECTRACE;
			break;

		case CREATEMATCH_STATE_SELECTRACE:
			// Switch control to the Select Race state engine
			Call( &g_SelectRaceStateEngine );

			// When that state engine returns control, be in the 
			// "Select Track" state
			m_State = CREATEMATCH_STATE_SELECTTRACK;

			break;

		case CREATEMATCH_STATE_SELECTTRACK:
			// Check whether the Select Race state engine succeeded in
			// selecting a race
			if( g_TitleScreenData.bGameTypeSelected )
			{
				// Advance to the select track state engine
				Call( &g_SelectTrackStateEngine );

				// When that state engine returns control, be in the 
				// "Track Selected" state
				m_State = CREATEMATCH_STATE_SELECTCAR;
			}
			else
			{
				// User must have hit the BACK button
				Return();
				return E_FAIL;
			}
			break;

		case CREATEMATCH_STATE_SELECTCAR:
			if( g_TitleScreenData.bTrackSelected )
			{
				// Advance to the select track state engine
				Call( &g_SelectCarStateEngine );

				// When that state engine returns control, be in the 
				// "Track Selected" state
				m_State = CREATEMATCH_STATE_GAMEOVERVIEW;
			}
			else
			{
				// User must have hit the BACK button
				m_State = CREATEMATCH_STATE_SELECTTRACK;
			}
			break;

		case CREATEMATCH_STATE_GAMEOVERVIEW:
			SetNextMenu( &gMenuHeader, &Menu_Overview );
			m_State = CREATEMATCH_STATE_STARTGAME;
			break;

		case CREATEMATCH_STATE_STARTGAME:
			if( FALSE == g_bMenuActive )
			{
				// Return for now
				Return();
			}
			break;
	}

	return S_FALSE;
}




//-----------------------------------------------------------------------------
// The Select Track state engine
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
			g_TitleScreenData.bTrackSelected = FALSE;
			SetNextMenu( &gMenuHeader, &Menu_SelectTrack );
			m_State = SELECTTRACK_STATE_MAINLOOP;
			break;

		case SELECTTRACK_STATE_MAINLOOP:
			// Do nothing until the user exits the menu system
			if( g_bMenuActive == FALSE )
				Return();
			break;
	}

	return S_FALSE;
}




//-----------------------------------------------------------------------------
// The Select Car state engine
//-----------------------------------------------------------------------------
HRESULT CSelectCarStateEngine::Process()
{
	enum
	{
		SELECTCAR_STATE_BEGIN = STATEENGINE_STATE_BEGIN,
		SELECTCAR_STATE_MAINLOOP,
	};

	switch( m_State )
	{
		case SELECTCAR_STATE_BEGIN:
			g_TitleScreenData.bCarsSelected = FALSE;
			SetNextMenu( &gMenuHeader, &Menu_SelectCar );
			m_State = SELECTCAR_STATE_MAINLOOP;
			break;

		case SELECTCAR_STATE_MAINLOOP:
			// Do nothing until the user exits the menu system
			if( g_bMenuActive == FALSE )
				Return();
			break;
	}

	return S_FALSE;
}






//-----------------------------------------------------------------------------
// The Connect state engine
//-----------------------------------------------------------------------------
HRESULT CConnectStateEngine::Process()
{
	enum
	{
		CONNECT_STATE_BEGIN = STATEENGINE_STATE_BEGIN,
		CONNECT_STATE_MAINLOOP,
	};

	switch( m_State )
	{
		case CONNECT_STATE_BEGIN:
			SetNextMenu( &gMenuHeader, &Menu_Connection );
			m_State = CONNECT_STATE_MAINLOOP;
			break;

		case CONNECT_STATE_MAINLOOP:
			// Do nothing until the user exits the menu system
			if( g_bMenuActive == FALSE )
				Return();
			break;
	}

	return S_FALSE;
}






