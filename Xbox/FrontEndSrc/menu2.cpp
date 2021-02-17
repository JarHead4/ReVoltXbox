//-----------------------------------------------------------------------------
// File: Menu2.cpp
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"
#include "Menu2.h"
#include "MenuText.h"
#include "MenuDraw.h"
#include "TitleScreen.h"
#include "timing.h"
#include "text.h"

#include "SoundEffectEngine.h"
#include "SoundEffects.h"
#include "ctrlread.h"
#include "XBInput.h"

// menus
#include "Confirm.h"


static void InitMenuHeader(MENU_HEADER *menuHeader);
static void CalcMenuDimensions(MENU_HEADER *menuHeader, MENU *menu);
static void DrawMenuBox(MENU_HEADER *menuHeader);
static void DrawMenuItemText(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex);

MENU_HEADER gMenuHeader = {NULL,};
bool gChangeParentMenu = FALSE;
bool g_bMenuDrawMenuBox = TRUE;




#define NAVICON_DEFAULT_XPOS 518
#define NAVICON_DEFAULT_YPOS 400

#define LANGUAGE_SELECT_STRING "Select Language"

////////////////////////////////////////////////////////////////
//
// Clear Menu Header: call when level entered for first time
//
////////////////////////////////////////////////////////////////

void ClearMenuHeader(MENU_HEADER *menuHeader)
{
    menuHeader->pMenu = NULL;
    menuHeader->pNextMenu = NULL;
    menuHeader->NMenuItems = 0;
    menuHeader->strTitle = NULL;
    
    menuHeader->XPos = menuHeader->YPos = 0;
    menuHeader->DestXPos = menuHeader->DestYPos = 0;
    menuHeader->XSize = menuHeader->YSize = 0;
    menuHeader->DestXSize = menuHeader->DestYSize = 0;

    menuHeader->State = MENU_STATE_LEAVING;
}


/////////////////////////////////////////////////////////////////////
//
// InitMenu:
//
/////////////////////////////////////////////////////////////////////

void InitMenuHeader(MENU_HEADER *menuHeader)
{

    // Reset Confirmation menu return value if not going back from Confirm menu
    if (menuHeader->pMenu != &Menu_ConfirmYesNo) {
        gConfirmMenuReturnVal = CONFIRM_NONE;
    }
    
    // Set the new menu up
    if ((menuHeader->pNextMenu != NULL) && (menuHeader->pMenu != NULL)) 
	{
        if (menuHeader->pMenu->pParentMenu != menuHeader->pNextMenu) 
		{
            if (gChangeParentMenu) 
			{
                menuHeader->pNextMenu->pParentMenu = menuHeader->pMenu;
            } 
			else 
			{
                menuHeader->pNextMenu->pParentMenu = menuHeader->pMenu->pParentMenu;
            }
        }
    }
    menuHeader->NMenuItems = 0;
    menuHeader->pMenu = menuHeader->pNextMenu;
    menuHeader->strTitle = NULL;
    menuHeader->ItemDataWidth = 0;
    menuHeader->ItemTextWidth = 0;

    // If no menu, put the box dimensions to zero
    if (menuHeader->pMenu == NULL) {
        menuHeader->XSize = menuHeader->YSize = 0;
        return;
    }

    // Disable auto demo running by default
    g_TitleScreenData.bAllowDemoToRun = FALSE;

    // Initial menu state
    menuHeader->State = MENU_STATE_ENTERING;
    menuHeader->NavFlags = MENU_FLAG_ADVANCE | MENU_FLAG_GOBACK;
    menuHeader->NavXPos = NAVICON_DEFAULT_XPOS;
    menuHeader->NavYPos = NAVICON_DEFAULT_YPOS;
    gChangeParentMenu = TRUE;
    g_bMenuDrawMenuBox = TRUE;

    // Create the menu structure
    menuHeader->pMenu->CreateFunc(menuHeader, menuHeader->pMenu);

    // If the menu has killed itself, don't try to do anything with it
    if (menuHeader->pMenu == NULL) return;

    // Calculate the dimensions of the menu box
    CalcMenuDimensions(menuHeader, menuHeader->pMenu);
    menuHeader->XSize = menuHeader->DestXSize;
    menuHeader->YSize = menuHeader->DestYSize;

    // Set offscreen position 
    if (menuHeader->pMenu->Type & MENU_ENTER_LEFT) {
        menuHeader->XPos = -menuHeader->XSize - MENU_START_OFFSET_X;
    } else if (menuHeader->pMenu->Type & MENU_ENTER_RIGHT) {
        menuHeader->XPos = Real(MENU_SCREEN_WIDTH) + MENU_START_OFFSET_X;
    } else {
        if (menuHeader->pMenu->Type & MENU_CENTRE_X) {
            menuHeader->XPos = (Real(MENU_SCREEN_WIDTH) - menuHeader->XSize) / 2;
        } else {
            menuHeader->XPos = menuHeader->pMenu->XPos;
        }
    }
    if (menuHeader->pMenu->Type & MENU_ENTER_TOP) {
        menuHeader->YPos = -menuHeader->YSize - MENU_START_OFFSET_Y;
    } else if (menuHeader->pMenu->Type & MENU_ENTER_BOTTOM) {
        menuHeader->YPos = Real(MENU_SCREEN_HEIGHT) + MENU_START_OFFSET_Y;
    } else {
        if (menuHeader->pMenu->Type & MENU_CENTRE_Y) {
            menuHeader->YPos = (Real(MENU_SCREEN_HEIGHT) - menuHeader->YSize) / 2;
        } else {
            menuHeader->YPos = menuHeader->pMenu->YPos;
        }
    }
    // Set destination position
    if (menuHeader->pMenu->Type & MENU_CENTRE_X) {
        menuHeader->DestXPos = (Real(MENU_SCREEN_WIDTH) - menuHeader->XSize) / 2;
    } else {
        menuHeader->DestXPos = menuHeader->pMenu->XPos;
    }
    if (menuHeader->pMenu->Type & MENU_CENTRE_Y) {
        menuHeader->DestYPos = (Real(MENU_SCREEN_HEIGHT) - menuHeader->YSize) / 2;
    } else {
        menuHeader->DestYPos = menuHeader->pMenu->YPos;
    }

    // Setup menu title text
    if (menuHeader->pMenu->TextIndex != TEXT_NONE) {
        if (menuHeader->pMenu->TextIndex == TEXT_LANGUAGE) {
            menuHeader->strTitle = LANGUAGE_SELECT_STRING;
        } else {
            menuHeader->strTitle = TEXT_TABLE(menuHeader->pMenu->TextIndex);
        }
    } else {
        menuHeader->strTitle = NULL;
    }

    // Set menu colour
    if (menuHeader->pMenu->SpruColIndex != SPRU_COL_RANDOM) {
        if (menuHeader->pMenu->SpruColIndex != SPRU_COL_KEEP) {
            menuHeader->SpruColIndex = menuHeader->pMenu->SpruColIndex;
        }
    } else {
        menuHeader->SpruColIndex = rand() % SPRU_COL_RANDOM;
    }

}

void CalcMenuDimensions(MENU_HEADER *menuHeader, MENU *menu)
{
    //int iMenu;
    //REAL t;
    //MENU_ITEM *menuItem;
    REAL width = 0;
    REAL height = 0;


/*  for (iMenu = 0; iMenu < menuHeader->NMenuItems; iMenu++) {
        menuItem = menuHeader->MenuItem[iMenu];
        
        t = 0;
        if (menuItem->TextIndex != TEXT_NONE) {
            t+= Real(2) * MENU_TEXT_HSKIP + strlen(TEXT_TABLE(menuItem->TextIndex)) * Real(MENU_TEXT_WIDTH);
        }
        t += menuItem->DataWidth;

        if (t > width) width = t;
    }
*/

    if (menuHeader->ItemDataWidth == 0 && menuHeader->ItemTextWidth == 0) {
        width = height = 0;
    } else {
        width = menuHeader->ItemDataWidth + menuHeader->ItemTextWidth;
        if ((menuHeader->ItemDataWidth > 0) && (menuHeader->ItemTextWidth > 0)) {
            width += MENU_TEXT_GAP;
        }

        height = menuHeader->NMenuItems * Real(MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP);
    }

    menuHeader->DestXSize = width;
    menuHeader->DestYSize = height;

}


////////////////////////////////////////////////////////////////
//
// Set Next Menu
//
////////////////////////////////////////////////////////////////
extern BOOL g_bMenuActive = FALSE;

void SetNextMenu(MENU_HEADER *menuHeader, MENU *menu)
{
    menuHeader->pNextMenu = menu;

	if( menu )
		g_bMenuActive = TRUE;
	else 
		g_bMenuActive = FALSE;

    // Set off screen destination of menu
    if( menuHeader->pMenu ) 
	{
        // X destination
        if (menuHeader->pMenu->Type & MENU_EXIT_LEFT) {
            menuHeader->DestXPos = -menuHeader->XSize - Real(100);
        } else if (menuHeader->pMenu->Type & MENU_EXIT_RIGHT) {
            menuHeader->DestXPos = Real(MENU_SCREEN_WIDTH) + Real(100);
        } else {
            menuHeader->DestXPos = menuHeader->XPos;//menuHeader->Menu->XPos;
        }
        // Y destination
        if (menuHeader->pMenu->Type & MENU_EXIT_TOP) {
            menuHeader->DestYPos = -menuHeader->YSize - Real(100);
        } else if (menuHeader->pMenu->Type & MENU_EXIT_BOTTOM) {
            menuHeader->DestYPos = Real(MENU_SCREEN_HEIGHT) + Real(100);
        } else {
            menuHeader->DestYPos = menuHeader->YPos;//menuHeader->Menu->YPos;
        }
        // Set initial state
        menuHeader->State = MENU_STATE_LEAVING;
    } 
	else 
	{
        // Set initial state
        menuHeader->State = MENU_STATE_OFFSCREEN;
    }
}


/////////////////////////////////////////////////////////////////////
//
// AddMenuItem:
//
/////////////////////////////////////////////////////////////////////

void AddMenuItem(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    REAL width;
    Assert(menuHeader->NMenuItems < MENU_MAX_ITEMS);

    // Add item to menu list
    menuHeader->pMenuItem[menuHeader->NMenuItems] = menuItem;
    menuHeader->pMenuItem[menuHeader->NMenuItems]->ActiveFlags = MENU_ITEM_SELECTABLE | MENU_ITEM_ACTIVE;

    menuHeader->NMenuItems++;

    // Calculate text width so that data can be placed properly
#ifdef _N64
    if (menuItem->TextIndex != TEXT_NONE) {
        width = GetTextLen(TEXT_TABLE(menuItem->TextIndex));
        if (width > menuHeader->ItemTextWidth) {
            menuHeader->ItemTextWidth = width;
        }
    }
#else
    if (menuItem->TextIndex != TEXT_NONE) {
        width = strlen(TEXT_TABLE(menuItem->TextIndex)) * Real(MENU_TEXT_WIDTH);
        if (width > menuHeader->ItemTextWidth) {
            menuHeader->ItemTextWidth = width;
        }
    }
#endif
    // Set data width
    if (menuItem->DataWidth > menuHeader->ItemDataWidth) {
        menuHeader->ItemDataWidth = menuItem->DataWidth;
    }

}


////////////////////////////////////////////////////////////////
//
// ToggleMenuData:
//
////////////////////////////////////////////////////////////////

void ToggleMenuData(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    bool *data;
    if(!menuItem->ActiveFlags & MENU_ITEM_SELECTABLE) return;

    data = (bool*)menuItem->pData;

    *data = !(*data);
}

void ToggleMenuDataOn(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    bool *data;

    if(!menuItem->ActiveFlags & MENU_ITEM_SELECTABLE) return;

    data = (bool*)menuItem->pData;

    *data = TRUE;
}

void ToggleMenuDataOff(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    bool *data;

    if(!menuItem->ActiveFlags & MENU_ITEM_SELECTABLE) return;

    data = (bool*)menuItem->pData;

    *data = FALSE;
}




/////////////////////////////////////////////////////////////////////
//
// SelectPreviousMenuItem:
//
/////////////////////////////////////////////////////////////////////

void SelectPreviousMenuItem(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    long startIndex = menu->CurrentItemIndex;

    do {
        --menu->CurrentItemIndex;
        if (menu->CurrentItemIndex < 0) {
            menu->CurrentItemIndex = menuHeader->NMenuItems - 1;
        }
    } while ((startIndex != menu->CurrentItemIndex) && !(menuHeader->pMenuItem[menuHeader->pMenu->CurrentItemIndex]->ActiveFlags & MENU_ITEM_SELECTABLE));
}


/////////////////////////////////////////////////////////////////////
//
// SelectNextMenuItem:
//
/////////////////////////////////////////////////////////////////////

void SelectNextMenuItem(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    long startIndex = menu->CurrentItemIndex;

    do {
        ++menu->CurrentItemIndex;
        if (menu->CurrentItemIndex >= menuHeader->NMenuItems) {
            menu->CurrentItemIndex = 0;
        }
    } while ((startIndex != menu->CurrentItemIndex) && !(menuHeader->pMenuItem[menuHeader->pMenu->CurrentItemIndex]->ActiveFlags & MENU_ITEM_SELECTABLE));
}


/////////////////////////////////////////////////////////////////////
//
// MenuGoBack:
//
/////////////////////////////////////////////////////////////////////

void MenuGoBack(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    menuHeader->pNextMenu = menu->pParentMenu;
}


/////////////////////////////////////////////////////////////////////
//
// MenuGoForward:
//
/////////////////////////////////////////////////////////////////////

void MenuGoForward(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    menuHeader->pNextMenu = (MENU *)menuItem->pData;
}


////////////////////////////////////////////////////////////////
//
// Move Menu: move the menu from current pos towards destination
//
////////////////////////////////////////////////////////////////

void MoveResizeMenu(MENU_HEADER *menuHeader)
{
    REAL delta, xVel, yVel;
    bool reachedDest = TRUE;

    if ((menuHeader->pMenu == NULL) && (menuHeader->pNextMenu == NULL)) return;

    // Move menu if neccesary
    if ((menuHeader->State == MENU_STATE_LEAVING) || (menuHeader->State == MENU_STATE_ENTERING)) {

        xVel = MulScalar(MENU_MOVE_VEL, TimeStep);
        yVel = MulScalar(MENU_MOVE_VEL, TimeStep);

        if (menuHeader->pMenu != NULL) {
            // Move along the X axis
            if (menuHeader->XPos != menuHeader->DestXPos) {
                delta = menuHeader->DestXPos - menuHeader->XPos;
                if (abs(delta) < xVel) {
                    menuHeader->XPos = menuHeader->DestXPos;
                } else {
                    if (delta < 0) {
                        menuHeader->XPos -= xVel;
                    } else {
                        menuHeader->XPos += xVel;
                    }
                    reachedDest = FALSE;
                }
            }

            // Move along the Y axis
            if (menuHeader->YPos != menuHeader->DestYPos) {
                delta = menuHeader->DestYPos - menuHeader->YPos;
                if (abs(delta) < yVel) {
                    menuHeader->YPos = menuHeader->DestYPos;
                } else {
                    if (delta < 0) {
                        menuHeader->YPos -= yVel;
                    } else {
                        menuHeader->YPos += yVel;
                    }
                    reachedDest = FALSE;
                }
            }
        }

        // Has menu reached destination?
        if (reachedDest) {
            if (menuHeader->State == MENU_STATE_LEAVING) {
                menuHeader->State = MENU_STATE_OFFSCREEN;
            } else {
                menuHeader->State = MENU_STATE_PROCESSING;
            }
        }
    }

    // If menu offscreen, reset the position for the new menu
    if (menuHeader->State == MENU_STATE_OFFSCREEN) {

        InitMenuHeader(menuHeader);
    
    }

}   




//-----------------------------------------------------------------------------
// Name: ProcessMenu()
// Desc: 
//-----------------------------------------------------------------------------
VOID ProcessMenuInput( MENU_HEADER* pMenuHeader )
{
    if( pMenuHeader->pMenu == NULL )
		return;
    if( pMenuHeader->State == MENU_STATE_LEAVING )
		return;

    MENU_ITEM* pCurrentItem = pMenuHeader->pMenuItem[pMenuHeader->pMenu->CurrentItemIndex];

    // Make sure that the current selected item is valid
    while( (pMenuHeader->pMenu->CurrentItemIndex > 0) && 
		  !(pMenuHeader->pMenuItem[pMenuHeader->pMenu->CurrentItemIndex]->ActiveFlags & MENU_ITEM_SELECTABLE) ) 
	{
		pMenuHeader->pMenu->CurrentItemIndex--;
    }

    // Read Input
	LONG input = MENU_INPUT_NONE;

	// Turn gamepad input into menu input
	switch( g_Controllers[0].dwMenuInput )
	{
		// Directional input
		case XBINPUT_LEFT:  input = MENU_INPUT_LEFT;   break;
		case XBINPUT_RIGHT: input = MENU_INPUT_RIGHT;  break;
		case XBINPUT_UP:    input = MENU_INPUT_UP;     break;
		case XBINPUT_DOWN:  input = MENU_INPUT_DOWN;   break;

		// Use START and A as select buttons, but only if the current item is active
		case XBINPUT_START_BUTTON:  
		case XBINPUT_A_BUTTON:
			if( pCurrentItem->ActiveFlags & MENU_ITEM_ACTIVE )
				input = MENU_INPUT_SELECT; 
			break;

		// Use BACK and B and back buttons
		case XBINPUT_BACK_BUTTON:  
		case XBINPUT_B_BUTTON:     
			input = MENU_INPUT_BACK;   
			break;

		// All other input is thrown away
		default:            
			input = MENU_INPUT_NONE;   
			break;
	}

    // Process Input
	if( input != XBINPUT_NONE )
    {
		// Process input
        if( pCurrentItem->pfnInputHandler ) 
		{
			// New input handler
            pCurrentItem->pfnInputHandler( input, pMenuHeader, pMenuHeader->pMenu, pCurrentItem );
		}
		else if( pCurrentItem->InputAction[input] ) 
		{
            pCurrentItem->InputAction[input]( pMenuHeader, pMenuHeader->pMenu, pCurrentItem );

			// Play sound effects
            if( (input == XBINPUT_LEFT) || (input == XBINPUT_RIGHT) )
				PlaySfx( SFX_MENU_LEFTRIGHT, SFX_MAX_VOL, SFX_CENTRE_PAN, 44100, 0x7fffffff );

            if( (input == XBINPUT_UP) || (input == XBINPUT_DOWN) )
				PlaySfx( SFX_MENU_UPDOWN, SFX_MAX_VOL, SFX_CENTRE_PAN, 44100, 0x7fffffff );
        }

        g_fTitleScreenTimer = 0.0f;
    } 

    // See if its time to change menus
    if( pMenuHeader->pMenu != pMenuHeader->pNextMenu ) 
	{
        if( pMenuHeader->pNextMenu != pMenuHeader->pMenu->pParentMenu ) 
			PlaySfx( SFX_MENU_FORWARD, SFX_MAX_VOL, SFX_CENTRE_PAN, 44100, 0x7fffffff );
		else 
			PlaySfx( SFX_MENU_BACK, SFX_MAX_VOL, SFX_CENTRE_PAN, 44100, 0x7fffffff );

        SetNextMenu( pMenuHeader, pMenuHeader->pNextMenu );
    }
}




BOOL MenuDefHandler( DWORD input, MENU_HEADER* pMenuHeader, MENU* pMenu, 
					 MENU_ITEM* pMenuItem )
{
	switch( input )
	{
		case MENU_INPUT_UP:   
			SelectPreviousMenuItem( pMenuHeader, pMenu, pMenuItem );
			return TRUE;
		
		case MENU_INPUT_DOWN: 
			SelectNextMenuItem( pMenuHeader, pMenu, pMenuItem );
			return TRUE;

		case MENU_INPUT_BACK:
			MenuGoBack( pMenuHeader, pMenu, pMenuItem );
			return TRUE;

		case MENU_INPUT_SELECT:
			MenuGoForward( pMenuHeader, pMenu, pMenuItem );
			return TRUE;
	}

	return FALSE;
}




/////////////////////////////////////////////////////////////////////
//
// DrawMenu:
//
/////////////////////////////////////////////////////////////////////
void DrawMenu(MENU_HEADER *menuHeader)
{
    int iMenu;
    MENU_ITEM *menuItem;
    MENU *menu;

    // Make sure there are some items
    if ((menuHeader->NMenuItems == 0) || (menuHeader->pMenu == NULL)) 
		return;

    // Useful variables
    menu = menuHeader->pMenu;

    // Draw the background box
    if ((menuHeader->XSize > 0) && (menuHeader->YSize > 0))
        DrawMenuBox(menuHeader);

    // Draw all the text for the menu titles
    for (iMenu = 0; iMenu < menuHeader->NMenuItems; iMenu++) 
	{
        menuItem = menuHeader->pMenuItem[iMenu];
        DrawMenuItemText(menuHeader, menu, menuItem, iMenu);
    }

    // Do all the special drawing stuff if there is any
    for (iMenu = 0; iMenu < menuHeader->NMenuItems; iMenu++) 
	{
        menuItem = menuHeader->pMenuItem[iMenu];
        if (menuItem->DrawFunc) 
            menuItem->DrawFunc(menuHeader, menu, menuItem, iMenu);
    }
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
BOOL HandleMenus()
{
	// Menu Stuff
	MoveResizeMenu( &gMenuHeader );
	ProcessMenuInput( &gMenuHeader );

	// Draw menu
	if( gMenuHeader.pMenu == NULL ) 
		return FALSE;

	DrawMenuTitle( &gMenuHeader );
	DrawMenu( &gMenuHeader );
	DrawMenuLogo( &gMenuHeader );
	
	return TRUE;
}





////////////////////////////////////////////////////////////////
//
// Slider manipulation
//
////////////////////////////////////////////////////////////////

// signed long
void IncreaseSliderDataLong(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    SLIDER_DATA_LONG *sliderData;

    sliderData = (SLIDER_DATA_LONG*)menuItem->pData;

    *(sliderData->pData) += sliderData->Step;
    if (*(sliderData->pData) > sliderData->Max) *(sliderData->pData) = sliderData->Max;

}

void DecreaseSliderDataLong(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    SLIDER_DATA_LONG *sliderData;

    sliderData = (SLIDER_DATA_LONG*)menuItem->pData;

    *(sliderData->pData) -= sliderData->Step;
    if (*(sliderData->pData) < sliderData->Min) *(sliderData->pData) = sliderData->Min;

}

// unsigned long
void IncreaseSliderDataULong(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    SLIDER_DATA_ULONG *sliderData;

    sliderData = (SLIDER_DATA_ULONG*)menuItem->pData;

    *(sliderData->pData) += sliderData->Step;
    if (*(sliderData->pData) > sliderData->Max) *(sliderData->pData) = sliderData->Max;

}

void DecreaseSliderDataULong(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    SLIDER_DATA_ULONG *sliderData;

    sliderData = (SLIDER_DATA_ULONG*)menuItem->pData;

    *(sliderData->pData) -= sliderData->Step;
    if (*(sliderData->pData) < sliderData->Min) *(sliderData->pData) = sliderData->Min;
    if (*(sliderData->pData) & 0x80000000) *(sliderData->pData) = 0;

}

// string table
void IncreaseSliderDataStringTable(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    SLIDER_DATA_STRING_TABLE* pSliderData = (SLIDER_DATA_STRING_TABLE*)menuItem->pData;

    if( pSliderData->dwCurrentString < pSliderData->dwNumStrings - 1)
	    pSliderData->dwCurrentString++;
}

void DecreaseSliderDataStringTable(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    SLIDER_DATA_STRING_TABLE* pSliderData = (SLIDER_DATA_STRING_TABLE*)menuItem->pData;

    if( pSliderData->dwCurrentString > 0 )
	    pSliderData->dwCurrentString--;
}





long  InitialMenuMessage = MENU_MESSAGE_NONE;
int   InitialMenuMessageCount = 0;
int   InitialMenuMessageWidth = 0;
char  InitialMenuMessageString[256];
char* InitialMenuMessageLines[10];
FLOAT InitialMenuMessageTimer = ZERO;
FLOAT InitialMenuMessageMaxTime = ZERO;

//-----------------------------------------------------------------------------
// Name: InitMenuMessage()
// Desc: Initialise the initial message to show at the start
//-----------------------------------------------------------------------------
void InitMenuMessage( FLOAT fTimeOut )
{
    InitialMenuMessageTimer   = 0.0f;
    InitialMenuMessageMaxTime = fTimeOut;

    // Count the lines and get max width
    char c;
    int  iChar =  0;
    int  sWidth = 0;
    InitialMenuMessageWidth = 0;
    InitialMenuMessageCount = 0;
    InitialMenuMessageLines[0] = &InitialMenuMessageString[0];

    while( (c = InitialMenuMessageString[iChar++]) != '\0' ) 
	{
        sWidth++;

        // New line
        if( c == '\n' ) 
		{
            // Count lines and store pointer to start of line
            InitialMenuMessageCount++;
            InitialMenuMessageString[iChar-1] = '\0';
            InitialMenuMessageLines[InitialMenuMessageCount] = &InitialMenuMessageString[iChar];

            // Store max width
            if( sWidth > InitialMenuMessageWidth )
                InitialMenuMessageWidth = sWidth;
            sWidth = 0;
            continue;
        }
    }
}




//-----------------------------------------------------------------------------
// Name: SetMenuMessage()
// Desc: 
//-----------------------------------------------------------------------------
void SetMenuMessage(char *message)
{
    strncpy( InitialMenuMessageString, message, 256 );
}





//-----------------------------------------------------------------------------
// Name: SetBonusMenuMessage()
// Desc: Set startup message from championship etc
//-----------------------------------------------------------------------------
void SetBonusMenuMessage()
{
    // Build the message string
    switch( InitialMenuMessage )
	{
		case MENU_MESSAGE_NEWCARS:
			sprintf(InitialMenuMessageString, TEXT_TABLE(TEXT_BONUS_NEWCARS));
			break;

		case MENU_MESSAGE_REVERSE:
			sprintf(InitialMenuMessageString, TEXT_TABLE(TEXT_BONUS_REVERSE), TEXT_TABLE(TEXT_BRONZE_CUP + CupTable.CupType - 1));
			break;

		case MENU_MESSAGE_MIRROR:
			sprintf(InitialMenuMessageString, TEXT_TABLE(TEXT_BONUS_MIRROR), TEXT_TABLE(TEXT_BRONZE_CUP + CupTable.CupType - 1));
			break;

		case MENU_MESSAGE_REVMIR:
			sprintf(InitialMenuMessageString, TEXT_TABLE(TEXT_BONUS_REVMIR), TEXT_TABLE(TEXT_BRONZE_CUP + CupTable.CupType - 1));
			break;

		case MENU_MESSAGE_COCKWORK:
			sprintf(InitialMenuMessageString, TEXT_TABLE(TEXT_BONUS_COCKWORK));
			break;
    }
}




/////////////////////////////////////////////////////////////////////
//
// DrawMenuBox:
//
/////////////////////////////////////////////////////////////////////
void DrawMenuBox(MENU_HEADER *menuHeader)
{
    REAL xPos, yPos, xSize, ySize;

    if (!g_bMenuDrawMenuBox) return;

    xPos = (menuHeader->XPos - MENU_FRAME_WIDTH) * gMenuWidthScale;
    yPos = (menuHeader->YPos - MENU_FRAME_WIDTH - MENU_TEXT_VSKIP) * gMenuHeightScale;
    xSize = (menuHeader->XSize + MENU_FRAME_WIDTH * 2) * gMenuWidthScale;
    ySize = (menuHeader->YSize + MENU_FRAME_WIDTH * 2 + MENU_TEXT_VSKIP) * gMenuHeightScale;

    DrawSpruBox(xPos, yPos, xSize, ySize, menuHeader->SpruColIndex, 0);
}




/////////////////////////////////////////////////////////////////////
//
// DrawMenuItemText:
//
/////////////////////////////////////////////////////////////////////
void DrawMenuItemText( MENU_HEADER* pMenuHeader, MENU* pMenu, 
					   MENU_ITEM* pMenuItem, int itemIndex )
{
    DWORD color = MENU_TEXT_RGB_NORMAL;
    MENU_ITEM* pCurrentItem = pMenuHeader->pMenuItem[pMenu->CurrentItemIndex];

    // If no text, don't draw!
    if( pMenuItem->TextIndex == TEXT_NONE )
        return;

    // Choose text colour
    if( pCurrentItem == pMenuItem )
        color = MENU_TEXT_RGB_HILITE;

    if( !(pMenuHeader->pMenuItem[itemIndex]->ActiveFlags & MENU_ITEM_ACTIVE) ) 
	{
        if (pCurrentItem == pMenuItem) 
            color = MENU_TEXT_RGB_MIDLITE;
        else
            color = MENU_TEXT_RGB_LOLITE;
    }

    // Draw item name text
    FLOAT sx = pMenuHeader->XPos;
    FLOAT sy = pMenuHeader->YPos + (MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP) * itemIndex;
    DrawMenuText( sx, sy, color, gTitleScreen_Text[pMenuItem->TextIndex] );
}


