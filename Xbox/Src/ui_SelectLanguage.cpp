//-----------------------------------------------------------------------------
// File: ui_SelectLanguage.cpp
//
// Desc: 
//
// Re-Volt (Generic) Copyright (c) Probe Entertainment 1998
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"
#include "Settings.h"
#include "Text.h"
#include "ui_Menu.h"
#include "ui_menudraw.h"
#include "ui_MenuText.h"
#include "ui_TitleScreen.h"
#include "ui_StateEngine.h"
#include "ui_SelectLanguage.h"


static BOOL HandleLanguageMenu( MENU_HEADER* pMenuHeader, DWORD input );
static VOID DrawLanguageMenu( MENU_HEADER* pMenuHeader, MENU* pMenu );




//-----------------------------------------------------------------------------
// Language choice menu
//-----------------------------------------------------------------------------
extern MENU Menu_Language = 
{
    TEXT_LANGUAGE,
    MENU_DEFAULT | MENU_CENTRE_X | MENU_CENTRE_Y | MENU_NOBOX,   // Menu type
    NULL,                                   // Create menu function
    HandleLanguageMenu,                     // Input handler function
    DrawLanguageMenu,                       // Menu draw function
    0,                                      // X coord
    0,                                      // Y Coord
};


// Menu input handler
BOOL HandleLanguageMenu( MENU_HEADER* pMenuHeader, DWORD dwInput )
{
    switch( dwInput )
    {
        case MENU_INPUT_UP:
            if( gTitleScreenVars.Language > 0 )
                gTitleScreenVars.Language--;
            else
                gTitleScreenVars.Language = NUM_LANGUAGES - 1;
            return TRUE;
        
        case MENU_INPUT_DOWN:
            if( gTitleScreenVars.Language < NUM_LANGUAGES - 1 )
                gTitleScreenVars.Language++;
            else
                gTitleScreenVars.Language = 0;
            return TRUE;

        case MENU_INPUT_BACK:
            pMenuHeader->m_strTitle = NULL;
            g_SelectLanguageStateEngine.Return( STATEENGINE_TERMINATED );
            return TRUE;

        case MENU_INPUT_SELECT:
            LANG_LoadStrings( (LANGUAGE)gTitleScreenVars.Language );
            pMenuHeader->m_strTitle = NULL;
            g_SelectLanguageStateEngine.Return( STATEENGINE_COMPLETED );
            return TRUE;
    }

    return FALSE;
}




//-----------------------------------------------------------------------------
// Name: DrawLanguageMenu()
// Desc: 
//-----------------------------------------------------------------------------
VOID DrawLanguageMenu( MENU_HEADER* pMenuHeader, MENU* pMenu )
{
    FLOAT x = pMenuHeader->m_XPos - (MENU_TEXT_WIDTH*4);
    FLOAT y = pMenuHeader->m_YPos - (MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP) * 2;

    DrawNewSpruBox( (x-26.0f), (y-22.0f), 
                    (MENU_TEXT_WIDTH*8 + 52.0f), ( (MENU_TEXT_HEIGHT+MENU_TEXT_VSKIP) * NUM_LANGUAGES + 48.0f) );

    BeginTextState();

    for( long i = 0; i < NUM_LANGUAGES; i++ )
    {
        DWORD color = ( i == (signed)gTitleScreenVars.Language ) ? MENU_TEXT_RGB_HILITE : MENU_TEXT_RGB_NORMAL; 
        DrawMenuText( x, y + i * (MENU_TEXT_HEIGHT+MENU_TEXT_VSKIP), color, g_strLanguageNames[i] );
    }
}




CSelectLanguageStateEngine g_SelectLanguageStateEngine;

//-----------------------------------------------------------------------------
// Name: Process()
// Desc: Main control function for the state engine
//-----------------------------------------------------------------------------
HRESULT CSelectLanguageStateEngine::Process()
{
    enum
    {
        SELECTLANGUAGE_STATE_BEGIN = STATEENGINE_STATE_BEGIN,
        SELECTLANGUAGE_STATE_MAINLOOP,
    };

    switch( m_State )
    {
        case SELECTLANGUAGE_STATE_BEGIN:
            // Initialize state
            GameSettings.GameType = GAMETYPE_NONE;  //$REVISIT(cprince): I don't think this is supposed to be here.  (It's handled elsewhere.)  Should verify.

            // Set the menu and camera settings
            g_pMenuHeader->SetNextMenu( &Menu_Language );
            g_pTitleScreenCamera->SetNewPos( TITLESCREEN_CAMPOS_INIT );

            m_State = SELECTLANGUAGE_STATE_MAINLOOP;
            break;

        case SELECTLANGUAGE_STATE_MAINLOOP:
            // Nothing to do, as control is in the menus
            break;
    }

    // Handle the menus
    g_pMenuHeader->HandleMenus();

    return S_FALSE;
}



