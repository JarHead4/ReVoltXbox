//-----------------------------------------------------------------------------
// File: ui_ShowMessage.cpp
//
// Desc: UI implementation
//
// Re-Volt (Generic) Copyright (c) Probe Entertainment 1998
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"
#include "ui_Menu.h"        // MENU_DEFAULT
#include "ui_MenuDraw.h"    // DrawSpruBox
#include "main.h"           // TimeStep
#include "Text.h"           // BeginTextState

// re-volt specific
#include "ui_menutext.h"    // re-volt strings
#include "ui_TitleScreen.h"
#include "ui_StateEngine.h" 
#include "ui_ShowMessage.h" 

// temporary includes?
#include "player.h"         // for ui_TitleScreen.h to work


static VOID CreateInitialMessageMenu( MENU_HEADER* pMenuHeader, MENU* pMenu );
static BOOL HandleInitialMessageMenu( MENU_HEADER *pMenuHeader, DWORD dwInput );
static VOID DrawInitialMessage( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem, int itemIndex );

static VOID CreateMessageMenu( MENU_HEADER *pMenuHeader, MENU* pMenu );
static BOOL HandleMessageMenu( MENU_HEADER *pMenuHeader, DWORD dwInput );
static VOID DrawMessage( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem, int itemIndex );


CShowSimpleMessage g_ShowSimpleMessage;
CControllerRemoved g_ControllerRemoved;
WCHAR g_SimpleMessageBuffer[ 1024 ]; // Global buffer to be used for simple messages

//-----------------------------------------------------------------------------
// Initial Message menu (for bonus messages)
//-----------------------------------------------------------------------------
extern MENU Menu_InitialMessage = 
{
    TEXT_NONE,
    MENU_DEFAULT | MENU_CENTRE_X | MENU_CENTRE_Y,   // Menu type
    CreateInitialMessageMenu,               // Create menu function
    HandleInitialMessageMenu,               // Input handler function
    NULL,                                   // Menu draw function
    0,                                      // X coord
    0,                                      // Y Coord
};

// Menu item
MENU_ITEM MenuItem_InitialMessage = 
{
    TEXT_NONE,                              // Text label index
    0,                                      // Space needed to draw item data
    NULL,                                   // Data
    DrawInitialMessage,                     // Draw Function
};




//-----------------------------------------------------------------------------
// Name: CreateInitialMessageMenu()
// Desc: 
//-----------------------------------------------------------------------------
void CreateInitialMessageMenu(MENU_HEADER *pMenuHeader, MENU *pMenu)
{
    pMenuHeader->AddMenuItem( &MenuItem_InitialMessage );

    // Clear the message flag
    InitialMenuMessage = MENU_MESSAGE_NONE;
    pMenuHeader->m_ItemTextWidth = (FLOAT)(MENU_TEXT_WIDTH) * InitialMenuMessageWidth;

    pMenuHeader->m_pMenu->dwFlags |= MENU_NOBOX;
}




//-----------------------------------------------------------------------------
// Name: HandleInitialMessageMenu()
// Desc: 
//-----------------------------------------------------------------------------
BOOL HandleInitialMessageMenu( MENU_HEADER* pMenuHeader, DWORD dwInput )
{
    switch( dwInput )
    {
        case MENU_INPUT_BACK:
        case MENU_INPUT_SELECT:
            pMenuHeader->SetNextMenu( NULL );
            return TRUE;
    }

    return FALSE;
}




//-----------------------------------------------------------------------------
// Message menu
//-----------------------------------------------------------------------------
extern MENU Menu_Message = 
{
    TEXT_NONE,
    MENU_DEFAULT | MENU_CENTRE_X | MENU_CENTRE_Y,   // Menu type
    CreateMessageMenu,                      // Create menu function
    HandleMessageMenu,                      // Input handler function
    NULL,                                   // Menu draw function
    0,                                      // X coord
    0,                                      // Y Coord
};

// Menu item
MENU_ITEM MenuItem_Message = 
{
    TEXT_NONE,                              // Text label index
    0,                                      // Space needed to draw item data
    NULL,                                   // Data
    DrawMessage,                            // Draw Function
};




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
void CreateMessageMenu(MENU_HEADER *pMenuHeader, MENU *pMenu)
{
    pMenuHeader->AddMenuItem( &MenuItem_Message );

    if( pMenu->ParentMenu ) 
        pMenu->TextIndex = pMenu->ParentMenu->TextIndex;
    else 
        pMenu->TextIndex = TEXT_NONE;

    pMenuHeader->m_ItemTextWidth = (FLOAT)(MENU_TEXT_WIDTH) * InitialMenuMessageWidth;
    pMenuHeader->m_pMenu->dwFlags |= MENU_NOBOX;
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
BOOL HandleMessageMenu( MENU_HEADER* pMenuHeader, DWORD dwInput )
{
    assert( g_pActiveStateEngine == &g_ShowSimpleMessage ||
            g_pActiveStateEngine == &g_ControllerRemoved );

    switch( dwInput )
    {
        case MENU_INPUT_BACK:
            ((CShowSimpleMessage *)g_pActiveStateEngine)->Return( STATEENGINE_TERMINATED );
            return TRUE;
        case MENU_INPUT_SELECT:
            ((CShowSimpleMessage *)g_pActiveStateEngine)->Return( STATEENGINE_COMPLETED );
            return TRUE;
    }

    return FALSE;
}




//-----------------------------------------------------------------------------
// Name: DrawInitialMessage()
// Desc: Draw menu message
//-----------------------------------------------------------------------------
void DrawInitialMessage( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem, int itemIndex)
{
    int iLine, sLen;
    FLOAT xPos, yPos, xSize, ySize;

    xPos = pMenuHeader->m_XPos;
    yPos = pMenuHeader->m_YPos;
    xSize = (FLOAT)(MENU_TEXT_WIDTH) * InitialMenuMessageWidth;
    ySize = (FLOAT)(MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP) * InitialMenuMessageCount;

    DrawSpruBox( gMenuWidthScale  * (xPos - MENU_TEXT_GAP),
                 gMenuHeightScale * (yPos - MENU_TEXT_GAP),
                 gMenuWidthScale  * (xSize + 2*MENU_TEXT_GAP),
                 gMenuHeightScale * (ySize + 2*MENU_TEXT_GAP),
                 SPRU_COL_RANDOM, 0);

    BeginTextState();

    for (iLine = 0; iLine < InitialMenuMessageCount; iLine++) {
        sLen = wcslen(InitialMenuMessageLines[iLine]);

        xPos = (pMenuHeader->m_XPos + (xSize - (FLOAT)(MENU_TEXT_WIDTH) * sLen) / 2);

        DrawMenuText(xPos, yPos, MENU_TEXT_RGB_HILITE, InitialMenuMessageLines[iLine]);
        yPos += MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP;
    }

    // See if its time to move on
    InitialMenuMessageTimer += TimeStep;
    if (InitialMenuMessageTimer > InitialMenuMessageMaxTime) 
    {
        InitialMenuMessageTimer = ZERO;
        MenuGoForward(pMenuHeader, pMenu, pMenuItem);
    }
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
void DrawMessage( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem, int itemIndex)
{
    if( ( g_pActiveStateEngine != &g_ShowSimpleMessage &&
          g_pActiveStateEngine != &g_ControllerRemoved ) ||
        ((CShowSimpleMessage *)g_pActiveStateEngine)->m_strMessage == NULL )
        return;

    if( pMenuHeader->m_strTitle ) 
        pMenuHeader->m_strTitle = (WCHAR*)((CShowSimpleMessage *)g_pActiveStateEngine)->m_strTitle;

    FLOAT xPos  = pMenuHeader->m_XPos;
    FLOAT yPos  = pMenuHeader->m_YPos;
    FLOAT xSize;
    FLOAT ySize;

    FLOAT fWidth1 = g_pFont->GetTextWidth( ((CShowSimpleMessage *)g_pActiveStateEngine)->m_strTitle );
    FLOAT fWidth2 = g_pFont->GetTextWidth( ((CShowSimpleMessage *)g_pActiveStateEngine)->m_strMessage );
    FLOAT fWidth3 = g_pFont->GetTextWidth( ((CShowSimpleMessage *)g_pActiveStateEngine)->m_strForward );
    FLOAT fWidth4 = g_pFont->GetTextWidth( ((CShowSimpleMessage *)g_pActiveStateEngine)->m_strBackward );
    FLOAT fMaxWidth = max( fWidth1, max( fWidth2, max( fWidth3, fWidth4 ) ) );

    FLOAT fMessageWidth;
    FLOAT fMessageHeight;
    g_pFont->GetTextExtent( ((CShowSimpleMessage *)g_pActiveStateEngine)->m_strMessage, &fMessageWidth, &fMessageHeight );

    xSize = fMaxWidth;
    ySize = fMessageHeight;

    if( ((CShowSimpleMessage *)g_pActiveStateEngine)->m_strTitle )
        ySize += 40;
    if( ((CShowSimpleMessage *)g_pActiveStateEngine)->m_strForward || ((CShowSimpleMessage *)g_pActiveStateEngine)->m_strBackward )
        ySize += 20;
    if( ((CShowSimpleMessage *)g_pActiveStateEngine)->m_strForward )
        ySize += 20;
    if( ((CShowSimpleMessage *)g_pActiveStateEngine)->m_strBackward )
        ySize += 20;
    
    xPos  = xPos - xSize/2;
    yPos  = yPos - ySize/2;

    DrawNewSpruBoxWithTabs( xPos - 20,
                            yPos - 10,
                            xSize + 40,
                            ySize + 20 );

    BeginTextState();

    if( ((CShowSimpleMessage *)g_pActiveStateEngine)->m_strTitle )
    {
        g_pFont->DrawText( xPos+xSize/2, yPos, MENU_TEXT_RGB_NORMAL, ((CShowSimpleMessage *)g_pActiveStateEngine)->m_strTitle, XBFONT_CENTER_X );
        yPos += 40;
    }

    g_pFont->DrawText( xPos+xSize/2, yPos, MENU_TEXT_RGB_NORMAL, ((CShowSimpleMessage *)g_pActiveStateEngine)->m_strMessage, XBFONT_CENTER_X );
    yPos += fMessageHeight;
    yPos += 20;
    
    if( ((CShowSimpleMessage *)g_pActiveStateEngine)->m_strForward )
    {
        g_pFont->DrawText( xPos, yPos, MENU_TEXT_RGB_NORMAL, ((CShowSimpleMessage *)g_pActiveStateEngine)->m_strForward );
        yPos += 20;
    }
    
    if( ((CShowSimpleMessage *)g_pActiveStateEngine)->m_strBackward )
    {
        g_pFont->DrawText( xPos, yPos, MENU_TEXT_RGB_NORMAL, ((CShowSimpleMessage *)g_pActiveStateEngine)->m_strBackward );
        yPos += 20;
    }
}



//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
//$HACK(Apr02_GameBash) - don't push a ShowSimpleMessage on top of another one!
VOID CShowSimpleMessage::MakeActive( CStateEngine* pParent )
{
    if( g_pActiveStateEngine == this )
    {
        // Super-duper hack.  If we use g_pActiveStateEngine,
        // then we get CStateEngine::Return, rather than
        // CUIStateEngine::Return( dwStatus )
        ((CShowSimpleMessage *)g_pActiveStateEngine)->Return();
    }

    // Note that g_pActiveStateEngine may have changed 
    // due to the above Return() call, so we want to
    // use the new g_pActiveStateEngine, rather than
    // the passed in parent.
    CUIStateEngine::MakeActive( g_pActiveStateEngine );
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
VOID CShowSimpleMessage::Begin( const WCHAR* strTitle, const WCHAR* strMessage,
                                const WCHAR* strForward, const WCHAR* strBackward )
{
    Assert( strMessage != NULL);
    Assert( (strForward != NULL) || (strBackward != NULL) );

    m_strTitle      = strTitle;
    m_strMessage    = strMessage;
    m_strForward    = strForward;
    m_strBackward   = strBackward;

    MakeActive();
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CShowSimpleMessage::Process()
{
    enum 
    {
        SHOWMESSAGE_STATE_BEGIN = STATEENGINE_STATE_BEGIN,
        SHOWMESSAGE_STATE_MAINLOOP,
    };

    switch( m_State )
    {
        case SHOWMESSAGE_STATE_BEGIN:
            g_pMenuHeader->SetNextMenu( &Menu_Message );

            // The menu is gone, so advance to the next state
            m_State = SHOWMESSAGE_STATE_MAINLOOP;
            break;

        case SHOWMESSAGE_STATE_MAINLOOP:
            // Nothing to do, as control is in the menus
            break;
    }

    // Handle the menus
    g_pMenuHeader->HandleMenus();

    return S_FALSE;
}



//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
VOID CControllerRemoved::Begin( DWORD dwPort )
{
    swprintf( m_strMessage, TEXT_TABLE(TEXT_RECONNECT_CONTROLLER_TO), dwPort + 1 );

    CShowSimpleMessage::Begin( TEXT_TABLE(TEXT_RECONNECT_CONTROLLER),
                               m_strMessage,
                               TEXT_TABLE(TEXT_BUTTON_A_CONTINUE),
                               NULL );
}



