//-----------------------------------------------------------------------------
// File: ui_Menu.cpp
//
// Desc: UI implementation
//
// Re-Volt (Generic) Copyright (c) Probe Entertainment 1998
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"
#include "text.h"
#include "input.h"
#include "geom.h"
#include "camera.h"
#include "player.h"
#include "settings.h"
#include "SoundEffectEngine.h"
#include "ctrlread.h"
#include "XBInput.h"
#include "DrawObj.h"

#include "ui_Menu.h"
#include "ui_TitleScreen.h"
#include "ui_StateEngine.h"
#include "ui_MenuText.h"
#include "ui_MenuDraw.h"
#include "ui_Confirm.h"

extern BOOL g_bSplitScreen;

extern BOOL g_bShowMenuLogo = FALSE;


MENU_HEADER  g_MenuHeader;
MENU_HEADER* g_pMenuHeader = &g_MenuHeader;

MENU_HEADER::MENU_HEADER()
{
    ZeroMemory( this, sizeof(this) );
}

static BOOL IsInteger( FLOAT f )
{
    return( ( f - floorf(f) ) < 0.0001 ||
            ( ceilf(f) - f  ) < 0.0001 );
}



#ifdef OLD_AUDIO
long gMenuInputSFXIndex[MENU_INPUT_NTYPES] = 
{
    SFX_MENU_UPDOWN,
    SFX_MENU_UPDOWN,
    SFX_MENU_LEFTRIGHT,
    SFX_MENU_LEFTRIGHT,
    SFX_MENU_BACK,
    SFX_MENU_FORWARD,
    SFX_MENU_FORWARD, // X button
    SFX_MENU_FORWARD, // Y Button
};
#else
long gMenuInputSFXIndex[MENU_INPUT_NTYPES] = 
{
    EFFECT_MenuUpDown,
    EFFECT_MenuUpDown,
    EFFECT_MenuLeftRight,
    EFFECT_MenuLeftRight,
    EFFECT_MenuPrev,
    EFFECT_MenuNext,
    EFFECT_MenuNext,    // X button
    EFFECT_MenuNext,    // Y button
};
#endif
#define NAVICON_DEFAULT_XPOS 518
#define NAVICON_DEFAULT_YPOS 400

#define MENU_LOGO_WIDTH     160
#define MENU_LOGO_HEIGHT    160
#define MENU_LOGO_YPOS      20
#define MENU_LOGO_FADE_TIME TO_TIME(0.5f)


////////////////////////////////////////////////////////////////
//
// Clear Menu Header: call when level entered for first time
//
////////////////////////////////////////////////////////////////
void MENU_HEADER::ClearMenuHeader()
{
    m_pMenu          = NULL;
    m_pNextMenu      = NULL;
    m_dwNumMenuItems = 0;
    m_strTitle       = NULL;
    
    m_XPos       = m_YPos      = 0;
    m_DestXPos   = m_DestYPos  = 0;
    m_XSize      = m_YSize     = 0;
    m_DestXSize  = m_DestYSize = 0;

    m_State      = MENU_STATE_LEAVING;
}




/////////////////////////////////////////////////////////////////////
//
// InitMenu:
//
/////////////////////////////////////////////////////////////////////
void MENU_HEADER::InitMenuHeader()
{
    // Reset Confirmation menu return value if not going back from Confirm menu
    if( m_pMenu != &Menu_ConfirmYesNo )
        gConfirmMenuReturnVal = CONFIRM_NONE;
    
    // Set the new menu up
    if( (m_pNextMenu != NULL) && (m_pMenu != NULL) ) 
    {
        if( m_pMenu->ParentMenu != m_pNextMenu ) 
        {
            m_pNextMenu->ParentMenu = m_pMenu;
        }
    }
    m_dwNumMenuItems = 0;
    m_pMenu          = m_pNextMenu;
    m_strTitle       = NULL;
    m_ItemDataWidth  = 0;
    m_ItemTextWidth  = 0;

    // If no menu, put the box dimensions to zero
    if( m_pMenu == NULL ) 
    {
        m_XSize = m_YSize = 0;
        return;
    }

    // Disable auto demo running by default
    gTitleScreenVars.bAllowDemoToRun = FALSE;

    // Initial menu state
    m_State = MENU_STATE_ENTERING;

    // Create the menu structure
    if( m_pMenu->CreateFunc )
        m_pMenu->CreateFunc( this, m_pMenu );

    // If the menu has killed itself, don't try to do anything with it
    if( m_pMenu == NULL )
        return;

    // Calculate the dimensions of the menu box
    CalcMenuDimensions(m_pMenu);
    m_XSize = m_DestXSize;
    m_YSize = m_DestYSize;

    // Set offscreen position 
    if( m_pMenu->dwFlags & MENU_ENTER_LEFT ) 
    {
        m_XPos = -m_XSize - MENU_START_OFFSET_X;
    } 
    else if( m_pMenu->dwFlags & MENU_ENTER_RIGHT ) 
    {
        m_XPos = (FLOAT)(MENU_SCREEN_WIDTH) + MENU_START_OFFSET_X;
    } 
    else 
    {
        if( m_pMenu->dwFlags & MENU_CENTRE_X ) 
            m_XPos = ((FLOAT)(MENU_SCREEN_WIDTH) - m_XSize) / 2;
        else 
            m_XPos = m_pMenu->XPos;
    }

    if( m_pMenu->dwFlags & MENU_ENTER_TOP ) 
    {
        m_YPos = -m_YSize - MENU_START_OFFSET_Y;
    } 
    else if( m_pMenu->dwFlags & MENU_ENTER_BOTTOM ) 
    {
        m_YPos = (FLOAT)(MENU_SCREEN_HEIGHT) + MENU_START_OFFSET_Y;
    } 
    else 
    {
        if( m_pMenu->dwFlags & MENU_CENTRE_Y ) 
            m_YPos = ((FLOAT)(MENU_SCREEN_HEIGHT) - m_YSize) / 2;
        else 
            m_YPos = m_pMenu->YPos;
    }

    // Set destination position
    if( m_pMenu->dwFlags & MENU_CENTRE_X ) 
        m_DestXPos = floorf( ((FLOAT)(MENU_SCREEN_WIDTH) - m_XSize) / 2.0f );
    else 
        m_DestXPos = m_pMenu->XPos;
    
    if( m_pMenu->dwFlags & MENU_CENTRE_Y ) 
        m_DestYPos = floorf( ((FLOAT)(MENU_SCREEN_HEIGHT) - m_YSize) / 2.0f );
    else 
        m_DestYPos = m_pMenu->YPos;

    // Ensure destination position is aligned correctly (x,y should be integral
    // values, so that half pixel offset added later will work correctly)
    assert( IsInteger( m_DestXPos ) );
    assert( IsInteger( m_DestYPos ) );

    // Setup menu title text
//$ADDITION(RBailey)
    if( ( -1 != m_pMenu->TextIndex ) && HIWORD(m_pMenu->TextIndex)) //$REVISIT
    {
        // TextIndex is actually a hardcoded string and should be migrated to language file.
        m_strTitle = (WCHAR*)m_pMenu->TextIndex;
    } 
    else
//$END_ADDITION
    if( m_pMenu->TextIndex != TEXT_NONE ) 
    {
        m_strTitle = TEXT_TABLE(m_pMenu->TextIndex);
    } 
    else 
    {
        m_strTitle = NULL;
    }
}




void MENU_HEADER::CalcMenuDimensions( MENU* pMenu )
{
/*  
    for( int iMenu = 0; iMenu < m_dwNumMenuItems; iMenu++ ) 
    {
        MENU_ITEM* menuItem = m_pMenuItem[iMenu];
        
        FLOAT t = 0;
        if (menuItem->TextIndex != TEXT_NONE)
        {
            t+= 2.0f * MENU_TEXT_HSKIP + wcslen(TEXT_TABLE(menuItem->TextIndex)) * (FLOAT)(MENU_TEXT_WIDTH);
        }
        t += menuItem->DataWidth;

        if( t > width )
            width = t;
    }
*/

    if( pMenu->fWidth != 0 && pMenu->fHeight != 0 ) 
    {
        m_DestXSize = pMenu->fWidth;
        m_DestYSize = pMenu->fHeight;
    }
    else if( m_ItemDataWidth == 0 && m_ItemTextWidth == 0 ) 
    {
        m_DestXSize = 0;
        m_DestYSize = 0;
    } 
    else 
    {
        FLOAT width = m_ItemDataWidth + m_ItemTextWidth;

        if( (m_ItemDataWidth > 0) && (m_ItemTextWidth > 0) ) 
            width += MENU_TEXT_GAP;

        FLOAT height = 0;

        if( m_dwNumMenuItems > 0 )
        {
            height += m_dwNumMenuItems * MENU_TEXT_HEIGHT;
            height += (m_dwNumMenuItems-1) * MENU_TEXT_VSKIP;
        }

        if( pMenu->dwFlags & MENU_PAD_FOR_ARROWS )
            width += g_pFont->GetTextWidth( L" >" );

        m_DestXSize = width;
        m_DestYSize = height;
    }
}




////////////////////////////////////////////////////////////////
//
// Set Next Menu
//
////////////////////////////////////////////////////////////////
void MENU_HEADER::SetNextMenu( MENU* pMenu )
{
    m_pNextMenu = pMenu;

    // Set off screen destination of menu
    if( m_pMenu != NULL ) 
    {
        // X destination
        if (m_pMenu->dwFlags & MENU_EXIT_LEFT) 
        {
            m_DestXPos = -m_XSize - (FLOAT)(100);
        } 
        else if (m_pMenu->dwFlags & MENU_EXIT_RIGHT) 
        {
            m_DestXPos = (FLOAT)(MENU_SCREEN_WIDTH) + (FLOAT)(100);
        } 
        else 
        {
            m_DestXPos = floorf( m_XPos );//m_pMenu->XPos;
        }
        
        // Y destination
        if (m_pMenu->dwFlags & MENU_EXIT_TOP) 
        {
            m_DestYPos = -m_YSize - (FLOAT)(100);
        } 
        else if (m_pMenu->dwFlags & MENU_EXIT_BOTTOM) 
        {
            m_DestYPos = (FLOAT)(MENU_SCREEN_HEIGHT) + (FLOAT)(100);
        } 
        else 
        {
            m_DestYPos = floorf( m_YPos );//m_pMenu->YPos;
        }

        // Ensure destination position is aligned correctly (x,y should be int
        // values, so that half pixel offset added later will work correctly)
        assert( IsInteger( m_DestXPos ) );
        assert( IsInteger( m_DestYPos ) );

        // Set initial state
        m_State = MENU_STATE_LEAVING;
    } 
    else 
    {
        // Set initial state
        m_State = MENU_STATE_OFFSCREEN;
    }
}




/////////////////////////////////////////////////////////////////////
//
// AddMenuItem:
//
/////////////////////////////////////////////////////////////////////
void MENU_HEADER::AddMenuItem( MENU_ITEM* pMenuItem, DWORD dwFlags )
{
    Assert( m_dwNumMenuItems < MENU_MAX_ITEMS );

    // Set up the new menuitem
    pMenuItem->ActiveFlags = dwFlags;

    // Add item to menu list
    m_pMenuItem[m_dwNumMenuItems] = pMenuItem;
    m_dwNumMenuItems++;

    // Calculate text width so that data can be placed properly
    if( pMenuItem->TextIndex != TEXT_NONE ) 
    {
        FLOAT width = g_pFont->GetTextWidth( TEXT_TABLE(pMenuItem->TextIndex) );
        if( width > m_ItemTextWidth ) 
            m_ItemTextWidth = width;
    }

    // Set data width
    if( pMenuItem->DataWidth > m_ItemDataWidth )
        m_ItemDataWidth = pMenuItem->DataWidth;
}




/////////////////////////////////////////////////////////////////////
//
// AddMenuItem:
//
/////////////////////////////////////////////////////////////////////
void MENU_HEADER::AddMenuItem( DWORD TextIndex, DWORD dwFlags )
{
    Assert( m_dwNumMenuItems < MENU_MAX_ITEMS );

    // Set up a new menuitem from the static list of menuitems
    static MENU_ITEM m_MenuItems[MENU_MAX_ITEMS];
    MENU_ITEM* pMenuItem = &m_MenuItems[m_dwNumMenuItems];
    ZeroMemory( pMenuItem, sizeof(MENU_ITEM) );
    pMenuItem->TextIndex = (TITLE_TEXT_STRING)TextIndex;
    pMenuItem->ActiveFlags = dwFlags;

    // Add item to menu list
    m_pMenuItem[m_dwNumMenuItems] = pMenuItem;
    m_dwNumMenuItems++;

    // Calculate text width so that data can be placed properly
    if( pMenuItem->TextIndex != TEXT_NONE ) 
    {
        FLOAT width = g_pFont->GetTextWidth( TEXT_TABLE(pMenuItem->TextIndex) );
        if( width > m_ItemTextWidth ) 
            m_ItemTextWidth = width;
    }

    // Set data width
    if( pMenuItem->DataWidth > m_ItemDataWidth )
        m_ItemDataWidth = pMenuItem->DataWidth;
}




/////////////////////////////////////////////////////////////////////
//
// AddMenuItem:
//
/////////////////////////////////////////////////////////////////////
void MENU_HEADER::AddMenuItem( const WCHAR* strText, DWORD dwFlags )
{
    Assert( m_dwNumMenuItems < MENU_MAX_ITEMS );

    // Set up a new menuitem from the static list of menuitems
    static MENU_ITEM m_MenuItems[MENU_MAX_ITEMS];
    MENU_ITEM* pMenuItem = &m_MenuItems[m_dwNumMenuItems];
    ZeroMemory( pMenuItem, sizeof(MENU_ITEM) );
    pMenuItem->TextIndex   = TEXT_NONE;
    pMenuItem->Data        = (void*)strText;
    pMenuItem->ActiveFlags = dwFlags | MENU_ITEM_HARDCODEDTEXT;

    // Add item to menu list
    m_pMenuItem[m_dwNumMenuItems] = pMenuItem;
    m_dwNumMenuItems++;

    // Calculate text width so that data can be placed properly
    if( strText != NULL ) 
    {
        FLOAT width = g_pFont->GetTextWidth( strText );
        if( width > m_ItemTextWidth ) 
            m_ItemTextWidth = width;
    }

    // Set data width
    if( pMenuItem->DataWidth > m_ItemDataWidth )
        m_ItemDataWidth = pMenuItem->DataWidth;
}




////////////////////////////////////////////////////////////////
//
// ToggleMenuData:
//
////////////////////////////////////////////////////////////////
BOOL ToggleMenuData(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    if(!menuItem->ActiveFlags & MENU_ITEM_SELECTABLE) return FALSE;

    bool* data = (bool*)menuItem->Data;

    *data = !(*data);

    return TRUE;
}

BOOL ToggleMenuDataOn(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    if(!menuItem->ActiveFlags & MENU_ITEM_SELECTABLE) return FALSE;

    bool* data = (bool*)menuItem->Data;

    if( *data != TRUE )
    {
        *data = TRUE;
        return TRUE;
    }

    return FALSE;
}

BOOL ToggleMenuDataOff(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    if(!menuItem->ActiveFlags & MENU_ITEM_SELECTABLE) return FALSE;

    bool* data = (bool*)menuItem->Data;

    if( *data != FALSE )
    {
        *data = FALSE;
        return TRUE;
    }

    return FALSE;
}




/////////////////////////////////////////////////////////////////////
//
// SelectPrevious/NextMenuItem:
//
/////////////////////////////////////////////////////////////////////
BOOL SelectPreviousMenuItem( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* )
{
    long startIndex = pMenu->CurrentItemIndex;

    do 
    {
        pMenu->CurrentItemIndex--;
        if( pMenu->CurrentItemIndex < 0 ) 
            pMenu->CurrentItemIndex = pMenuHeader->m_dwNumMenuItems ? pMenuHeader->m_dwNumMenuItems - 1 : 0;
    } 
    while( (startIndex != pMenu->CurrentItemIndex) && !(pMenuHeader->m_pMenuItem[pMenuHeader->m_pMenu->CurrentItemIndex]->ActiveFlags & MENU_ITEM_SELECTABLE) );

    // Return TRUE if a new menu item was actually selected
    return ( startIndex != pMenu->CurrentItemIndex ) ? TRUE : FALSE;
}

BOOL SelectNextMenuItem( MENU_HEADER *pMenuHeader, MENU* pMenu, MENU_ITEM* )
{
    long startIndex = pMenu->CurrentItemIndex;

    do 
    {
        pMenu->CurrentItemIndex++;
        if( pMenu->CurrentItemIndex >= pMenuHeader->m_dwNumMenuItems )
            pMenu->CurrentItemIndex = 0;
    } 
    while ((startIndex != pMenu->CurrentItemIndex) && !(pMenuHeader->m_pMenuItem[pMenuHeader->m_pMenu->CurrentItemIndex]->ActiveFlags & MENU_ITEM_SELECTABLE));

    // Return TRUE if a new menu item was actually selected
    return ( startIndex != pMenu->CurrentItemIndex ) ? TRUE : FALSE;
}




/////////////////////////////////////////////////////////////////////
//
// MenuGoBack/Forward:
//
/////////////////////////////////////////////////////////////////////
BOOL MenuGoBack( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* )
{
    pMenuHeader->m_pNextMenu = pMenu->ParentMenu;
    return TRUE;
}

BOOL MenuGoForward(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    menuHeader->m_pNextMenu = (MENU *)menuItem->Data;
    return TRUE;
}




////////////////////////////////////////////////////////////////
//
// Move Menu: move the menu from current pos towards destination
//
////////////////////////////////////////////////////////////////
void MENU_HEADER::MoveResizeMenu()
{
    if( (m_pMenu == NULL) && (m_pNextMenu == NULL) )
        return;

    // Move menu if neccesary
    if( (m_State == MENU_STATE_LEAVING) || (m_State == MENU_STATE_ENTERING) )
    {
        BOOL  bReachedDest = TRUE;

		// Keep timestep a 60Hz for debugging
		FLOAT fTimeStep = min( 1.0f/60.0f, TimeStep );
        FLOAT xVel = MENU_MOVE_VEL * fTimeStep;
        FLOAT yVel = MENU_MOVE_VEL * fTimeStep;

        if( m_pMenu != NULL ) 
        {
            // Move along the X axis
            if( m_XPos != m_DestXPos ) 
            {
                FLOAT delta = m_DestXPos - m_XPos;
                if( fabsf(delta) < xVel ) 
                {
                    m_XPos = m_DestXPos;
                } 
                else 
                {
                    if( delta < 0 ) 
                        m_XPos -= xVel;
                    else 
                        m_XPos += xVel;
                    bReachedDest = FALSE;
                }
            }

            // Move along the Y axis
            if( m_YPos != m_DestYPos ) 
            {
                FLOAT delta = m_DestYPos - m_YPos;
                if( fabsf(delta) < yVel ) 
                {
                    m_YPos = m_DestYPos;
                } 
                else 
                {
                    if( delta < 0 ) 
                        m_YPos -= yVel;
                    else
                        m_YPos += yVel;
                    bReachedDest = FALSE;
                }
            }
        }

        // Has menu reached destination?
        if( bReachedDest ) 
        {
            if( m_State == MENU_STATE_LEAVING )
                m_State = MENU_STATE_OFFSCREEN;
            else
                m_State = MENU_STATE_PROCESSING;
        }
    }

    // If menu offscreen, reset the position for the new menu
    if( m_State == MENU_STATE_OFFSCREEN )
        InitMenuHeader();
}   






//+----------------------------------------------------------------------------
//
// Function:  GetFullScreenMenuInput
//
// Synopsis:  consider input from all XB controllers, listen to lower 
//            controllers first, BACK==B, START==A, return simple result
//
// Arguments: bMergeButtons: TRUE if B==BACK and A==START
//            pnController: received index of controller making selection 
//
// Returns:   MenuInputTypeEnum - The simplified fullscreen menu 
//            interaction to be processed.
//
//+----------------------------------------------------------------------------
MenuInputTypeEnum GetFullScreenMenuInput( BOOL bMergeButtons, int *pnController )
{
    MenuInputTypeEnum input = MENU_INPUT_NONE;
    DWORD dwMenuInput = XBINPUT_NONE;

    if( g_bSplitScreen )
    {
        dwMenuInput = g_Controllers[gTitleScreenVars.iCurrentPlayer].dwMenuInput;
    }
    else
    {
        if ( pnController )
            *pnController = -1;

        // Concat input from all controllers
        for ( int i = 3; i >= 0; i-- )
        {
            if( g_Controllers[ i ].dwMenuInput != XBINPUT_NONE )
            {
                // if caller wants to know who made selection...
                if ( pnController )
                    *pnController = i;

                dwMenuInput = g_Controllers[ i ].dwMenuInput;

#ifdef _DEBUG
                //debug code to catch hidden UI
                if ( ( g_Controllers[ i ].bAnalogButtons[ XINPUT_GAMEPAD_LEFT_TRIGGER ] > XINPUT_GAMEPAD_MAX_CROSSTALK ) &&
                     ( g_Controllers[ i ].bAnalogButtons[ XINPUT_GAMEPAD_RIGHT_TRIGGER ] > XINPUT_GAMEPAD_MAX_CROSSTALK ) )
                {
                    if ( g_Controllers[ i ].bAnalogButtons[XINPUT_GAMEPAD_WHITE] > XINPUT_GAMEPAD_MAX_CROSSTALK )
                        input = MENU_INPUT_DEBUG_WHITE;
                    if( g_Controllers[ i ].bAnalogButtons[XINPUT_GAMEPAD_BLACK] > XINPUT_GAMEPAD_MAX_CROSSTALK )
                        input = MENU_INPUT_DEBUG_BLACK;

                }

#endif 
            }
        }

    }

    switch( dwMenuInput )
    {
        case XBINPUT_LEFT:         input = MENU_INPUT_LEFT;   break;
        case XBINPUT_RIGHT:        input = MENU_INPUT_RIGHT;  break;
        case XBINPUT_UP:           input = MENU_INPUT_UP;     break;
        case XBINPUT_DOWN:         input = MENU_INPUT_DOWN;   break;
        case XBINPUT_BACK_BUTTON:  input = MENU_INPUT_BACK;   break;
        case XBINPUT_B_BUTTON:     input = (bMergeButtons ? MENU_INPUT_BACK : MENU_INPUT_NONE);   break;
        case XBINPUT_START_BUTTON: input = MENU_INPUT_SELECT; break;
        case XBINPUT_A_BUTTON:     input = (bMergeButtons ? MENU_INPUT_SELECT : MENU_INPUT_NONE);   break;
        case XBINPUT_X_BUTTON:     input = MENU_INPUT_X;      break;
        case XBINPUT_Y_BUTTON:     input = MENU_INPUT_Y;      break;
    }

    return input;
}

void MENU_HEADER::ProcessMenuInput()
{
    if( m_pMenu == NULL ) 
        return;
    if( m_State == MENU_STATE_LEAVING ) 
        return;

    m_pCurrentItem = m_pMenuItem[m_pMenu->CurrentItemIndex];

    // Make sure that the current selected item is valid
    while( (m_pMenu->CurrentItemIndex > 0) && !(m_pMenuItem[m_pMenu->CurrentItemIndex]->ActiveFlags & MENU_ITEM_SELECTABLE) ) 
    {
        m_pMenu->CurrentItemIndex--;
    }

    // Read Input
    MenuInputTypeEnum input = GetFullScreenMenuInput( TRUE, &m_nLastControllerInput );

    // Process Input
    if( input != MENU_INPUT_NONE )
    {
        BOOL bMenuInputProcessed = FALSE;

        // Process input 

        // Try the global menu input handler first
        if( m_pMenu->InputFunc != NULL ) 
        {
            if( m_pMenu->InputFunc( this, input ) )
            {
                bMenuInputProcessed = TRUE;
            }
        }
        else if( m_pCurrentItem->InputAction[input] != NULL ) 
        {
            // Else, try the menuitem's input handler functions
            bMenuInputProcessed = m_pCurrentItem->InputAction[input]( this, m_pMenu, m_pCurrentItem );
        }

        if (bMenuInputProcessed)
        {
#ifdef OLD_AUDIO
                PlaySfx(gMenuInputSFXIndex[input], SFX_MAX_VOL, SFX_CENTRE_PAN, 44100, 0x7fffffff);
#else
                g_SoundEngine.Play2DSound( gMenuInputSFXIndex[input], FALSE );
#endif // OLD_AUDIO
        }

        g_fTitleScreenTimer = 0.0f;
    }

    // See if its time to change menus
    if( m_pMenu != m_pNextMenu ) 
    {
/*
        if( m_pNextMenu != m_pMenu->ParentMenu ) 
        {
#ifdef OLD_AUDIO
            PlaySfx( gMenuInputSFXIndex[MENU_INPUT_SELECT], SFX_MAX_VOL, SFX_CENTRE_PAN, 44100, 0x7fffffff );
#else
            g_SoundEngine.Play2DSound( gMenuInputSFXIndex[MENU_INPUT_SELECT], FALSE );
#endif // OLD_AUDIO
        } 
        else 
        {
#ifdef OLD_AUDIO
            PlaySfx( gMenuInputSFXIndex[MENU_INPUT_BACK], SFX_MAX_VOL, SFX_CENTRE_PAN, 44100, 0x7fffffff );
#else
            g_SoundEngine.Play2DSound( gMenuInputSFXIndex[MENU_INPUT_BACK], FALSE );
#endif // OLD_AUDIO
        }
*/
        SetNextMenu( m_pNextMenu );
    }
}




/////////////////////////////////////////////////////////////////////
//
// DrawMenu:
//
/////////////////////////////////////////////////////////////////////
void MENU_HEADER::DrawMenu()
{
    if( m_pMenu == NULL )
        return;

    // Draw the background box
    if( 0 == ( m_pMenu->dwFlags & MENU_NOBOX ) )
    {
        if( m_XSize > 0 && m_YSize > 0 ) 
            DrawMenuBox();
    }

    BeginTextState();

    // Do all the special drawing stuff if there is any
    if( m_pMenu->DrawFunc ) 
        m_pMenu->DrawFunc( this, m_pMenu );

    for( int iMenu = 0; iMenu < m_dwNumMenuItems; iMenu++ ) 
    {
        MENU_ITEM* pMenuItem = m_pMenuItem[iMenu];
        if( pMenuItem->DrawFunc ) 
            pMenuItem->DrawFunc( this, m_pMenu, pMenuItem, iMenu );
    }

    // Draw all the text for the menu titles
    BeginTextState();
    for( int iMenu = 0; iMenu < m_dwNumMenuItems; iMenu++ ) 
    {
        MENU_ITEM* pMenuItem = m_pMenuItem[iMenu];
        DrawMenuItemText( m_pMenu, pMenuItem, iMenu );
    }
}




////////////////////////////////////////////////////////////////
//
// Slider manipulation
//
////////////////////////////////////////////////////////////////
FLOAT CalcMaxStringWidth( DWORD dwNumStrings, WCHAR** pstrStrings )
{
    FLOAT fMaxWidth = 0.0f;
    for( DWORD i=0; i<dwNumStrings; i++ )
    {
        FLOAT fWidth = g_pFont->GetTextWidth( pstrStrings[i] );
        if( fWidth > fMaxWidth )
            fMaxWidth = fWidth;
    }
    return fMaxWidth;
}


// signed long
BOOL IncreaseSliderDataLong(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    SLIDER_DATA_LONG* sliderData = (SLIDER_DATA_LONG*)menuItem->Data;

    if (*(sliderData->Data) == sliderData->Max)
        return FALSE;

    *(sliderData->Data) += sliderData->Step;
    if (*(sliderData->Data) > sliderData->Max) 
        *(sliderData->Data) = sliderData->Max;

    return TRUE;
}

BOOL DecreaseSliderDataLong(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    SLIDER_DATA_LONG* sliderData = (SLIDER_DATA_LONG*)menuItem->Data;

    if (*(sliderData->Data) == sliderData->Min)
        return FALSE;

    *(sliderData->Data) -= sliderData->Step;
    if (*(sliderData->Data) < sliderData->Min) 
        *(sliderData->Data) = sliderData->Min;

    return TRUE;
}

// unsigned long
BOOL IncreaseSliderDataULong(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    SLIDER_DATA_ULONG* sliderData = (SLIDER_DATA_ULONG*)menuItem->Data;

    if (*(sliderData->Data) == sliderData->Max)
        return FALSE;

    *(sliderData->Data) += sliderData->Step;
    if (*(sliderData->Data) > sliderData->Max) 
        *(sliderData->Data) = sliderData->Max;

    return TRUE;
}

BOOL DecreaseSliderDataULong(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    SLIDER_DATA_ULONG* sliderData = (SLIDER_DATA_ULONG*)menuItem->Data;

    if (*(sliderData->Data) == sliderData->Min)
        return FALSE;

    *(sliderData->Data) -= sliderData->Step;
    if (*(sliderData->Data) < sliderData->Min)
        *(sliderData->Data) = sliderData->Min;
    if (*(sliderData->Data) & 0x80000000)
        *(sliderData->Data) = 0;

    return TRUE;
}




void MENU_HEADER::HandleMenus()
{
    // Update the camera
    g_pTitleScreenCamera->Update();

    // Animate menus and handle input
    MoveResizeMenu();
    ProcessMenuInput();

    // Draw menu
    if( m_pMenu != NULL ) 
    {
        DrawMenuTitle();
        DrawMenu();
    }

    DrawMenuLogo();
}




//-----------------------------------------------------------------------------
// Name: DrawMenuLogo()
// Desc: Draws the Re-volt logo
//-----------------------------------------------------------------------------
void MENU_HEADER::DrawMenuLogo()
{
    static FLOAT fLogoFadeTimer = 0.0f;

    // Fade timing
    fLogoFadeTimer += g_bShowMenuLogo ? TimeStep : -TimeStep;

    if( fLogoFadeTimer < 0.0f ) 
        fLogoFadeTimer = 0.0f;
    else if( fLogoFadeTimer > MENU_LOGO_FADE_TIME ) 
        fLogoFadeTimer = MENU_LOGO_FADE_TIME;

    // Draw the logo
    if( fLogoFadeTimer > 0.0f ) 
        DrawDemoLogo( fLogoFadeTimer / MENU_LOGO_FADE_TIME, 1);
}




