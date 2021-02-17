//-----------------------------------------------------------------------------
// File: ControllerOptions.cpp
//
// Desc: ControllerOptions.cpp
//
// Hist: 02.08.02 - ported/created
//
// Copyright (c)  Microsoft Corporation.  All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"
#include "menu2.h"
#include "MenuDraw.h"   // DrawSpruBox
#include "text.h"       // BeginTextState
#include "main.h"       // TimeStep
#include "settings.h"   // RegistrySettings
#include "input.h"      // JoystickNum

// re-volt specific
#include "menutext.h"   // re-volt strings

// temporary includes?
#include "titlescreen.h"

#define MENU_CONTROLLER_XPOS            Real(100)
#define MENU_CONTROLLER_YPOS            Real(150)
#define MENU_CONTROLSELECT_XPOS         Real(100)
#define MENU_CONTROLSELECT_YPOS         Real(150)

static void CreateControllerConfigMenu(MENU_HEADER *menuHeader, MENU *menu);
static void CreateControllerMenu(MENU_HEADER *menuHeader, MENU *menu);
static void MenuControlConfigBack(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
static void MenuSetControlsDefault(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);

static void IncreaseControlMethod(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
static void DecreaseControlMethod(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
static void IncreaseControllerPlayerNum(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
static void DecreaseControllerPlayerNum(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);

static void DrawControllerType(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex);
static void DrawControllerConfigure(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex);
static void DrawControllerPlayerNum(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex);


#ifdef _PC

KEY LastConfigKey;
long ControllerConfigPick;

// Menu
MENU Menu_ControllerConfigure = {
    TEXT_CONTROLLERCONFIG,
    MENU_IMAGE_OPTIONS,                     // Menu title bar image enum
    TITLESCREEN_CAMPOS_OVERVIEW,                     // Camera pos index
    MENU_DEFAULT,                           // Menu type
    SPRU_COL_RANDOM,                        // Spru colour
    CreateControllerConfigMenu,             // Create menu function
    MENU_CONTROLLER_XPOS,                   // X coord
    MENU_CONTROLLER_YPOS,                   // Y Coord
    0,                                      // Current Item index
    NULL,                                   // Parent Menu (OverWritten in Create)
};

// Controller - dummy menu entry
MENU_ITEM MenuItem_ControllerDummy = {
    TEXT_NONE,                              // Text label index

    0,                                      // Space needed to draw item data
    NULL,                                   // Data
    DrawControllerConfigure,                // Draw Function

 	NULL,								    // Input handler
	0,                                      // Flags

    NULL,                                   // Up Action
    NULL,                                   // Down Action
    NULL,                                   // Left Action
    NULL,                                   // Right Action
    NULL,                                   // Back Action
    NULL,                                   // Forward Action
};

// Create
void CreateControllerConfigMenu(MENU_HEADER *menuHeader, MENU *menu)
{
//$REMOVED_USEDBELOW    long i, delta;

    ControllerConfigPick = 0;

    LastConfigKey.Index = DIK_RETURN;
    LastConfigKey.Type = KEY_TYPE_KEYBOARD;

//$REVISIT: do we need whatever junk they're doing here?
//$REMOVED
//    if (RegistrySettings.Joystick != -1)
//    {
//        for (i = 0 ; i < Joystick[RegistrySettings.Joystick].AxisNum ; i++)
//        {
//            delta = ((long*)&JoystickState.lX)[Joystick[RegistrySettings.Joystick].AxisIndex[i]];
//            Joystick[RegistrySettings.Joystick].AxisDisable[i] = (delta < -(CTRL_RANGE_MAX / 2) || delta > (CTRL_RANGE_MAX / 2));
//        }
//    }
//$END_REMOVAL

    AddMenuItem(menuHeader, menu, &MenuItem_ControllerDummy);
}

////////////////////////////////////////////////////////////////
//
// Controller select
//
////////////////////////////////////////////////////////////////

// Data
SLIDER_DATA_LONG ControllerSlider = {
    &RegistrySettings.Joystick,
    -1, -1, 1,
    FALSE, TRUE,
};

SLIDER_DATA_LONG SteeringDeadzoneSlider = {
    &RegistrySettings.SteeringDeadzone,
    0, 100, 1,
    TRUE, TRUE,
};

SLIDER_DATA_LONG SteeringRangeSlider = {
    &RegistrySettings.SteeringRange,
    0, 100, 1,
    TRUE, TRUE,
};
static long gJoyTypeIn = -1;                // Joystick type chosen on entering config screen

// Menu
MENU Menu_ControllerSettings = {
    TEXT_CONTROLLER,
    MENU_IMAGE_OPTIONS,                     // Menu title bar image enum
    TITLESCREEN_CAMPOS_OVERVIEW,                     // Camera pos index
    MENU_DEFAULT,                           // Menu type
    SPRU_COL_RANDOM,                        // Spru colour
    CreateControllerMenu,                   // Create menu function
    MENU_CONTROLSELECT_XPOS,                // X coord
    MENU_CONTROLSELECT_YPOS,                // Y Coord
    0,                                      // Current Item index
    NULL,                                   // Parent Menu (OverWritten in Create)
};

// Controller - select
MENU_ITEM MenuItem_ControllerSelect = {
    TEXT_CONTROLLERSELECT,                  // Text label index

    MENU_DATA_WIDTH_TEXT,                   // Space needed to draw item data
    &ControllerSlider,                      // Data
    DrawControllerType,                     // Draw Function

	NULL,								    // Input handler
	0,                                      // Flags

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    DecreaseSliderDataLong,                 // Left Action
    IncreaseSliderDataLong,                 // Right Action
    MenuControlConfigBack,                              // Back Action
    NULL,                                   // Forward Action
};

// Controller - config
MENU_ITEM MenuItem_ControllerSetup = {
    TEXT_CONTROLLERCONFIG,                  // Text label index

    0,                                      // Space needed to draw item data
    &Menu_ControllerConfigure,              // Data
    NULL,                                   // Draw Function

	NULL,								    // Input handler
	0,                                      // Flags

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    NULL,                                   // Left Action
    NULL,                                   // Right Action
    MenuControlConfigBack,                              // Back Action
    MenuGoForward,                          // Forward Action
};

// Controller - default controls
MENU_ITEM MenuItem_ControllerDefault = {
    TEXT_CONTROLLERDEFAULT,                 // Text label index

    0,                                      // Space needed to draw item data
    NULL,                                   // Data
    NULL,                                   // Draw Function

	NULL,								    // Input handler
	0,                                      // Flags

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    NULL,                                   // Left Action
    NULL,                                   // Right Action
    MenuControlConfigBack,                              // Back Action
    MenuSetControlsDefault,                 // Forward Action
};

// Controller - steering deadzone
MENU_ITEM MenuItem_SteeringDeadZone = {
    TEXT_STEERING_DEADZONE,                 // Text label index

    MENU_DATA_WIDTH_SLIDER + 4 * MENU_TEXT_WIDTH,   // Space needed to draw item data
    &SteeringDeadzoneSlider,                // Data
    DrawSliderDataLong,                         // Draw Function

	NULL,								    // Input handler
	0,                                      // Flags

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    DecreaseSliderDataLong,                 // Left Action
    IncreaseSliderDataLong,                 // Right Action
    MenuControlConfigBack,                              // Back Action
    NULL,                                   // Forward Action
};

// Controller - steering range
MENU_ITEM MenuItem_SteeringRange = {
    TEXT_STEERING_RANGE,                    // Text label index

    MENU_DATA_WIDTH_SLIDER + 4 * MENU_TEXT_WIDTH,   // Space needed to draw item data
    &SteeringRangeSlider,                   // Data
    DrawSliderDataLong,                         // Draw Function

 	NULL,								    // Input handler
	0,                                      // Flags

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    DecreaseSliderDataLong,                 // Left Action
    IncreaseSliderDataLong,                 // Right Action
    MenuControlConfigBack,                              // Back Action
    NULL,                                   // Forward Action
};

// Controller - Non linear steering
MENU_ITEM MenuItem_NonLinearSteer = {
    TEXT_STEERING,                          // Text label index

    MENU_DATA_WIDTH_BOOL,                   // Space needed to draw item data
    &NonLinearSteering,                     // Data
    DrawMenuDataOnOff,                      // Draw Function

 	NULL,								    // Input handler
	0,                                      // Flags

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    ToggleMenuDataOff,                      // Left Action
    ToggleMenuDataOn,                       // Right Action
    MenuControlConfigBack,                              // Back Action
    NULL,                                   // Forward Action
};

// Create
void CreateControllerMenu(MENU_HEADER *menuHeader, MENU *menu)
{
    ControllerSlider.Max = JoystickNum - 1;
    gJoyTypeIn = RegistrySettings.Joystick;
    
    AddMenuItem(menuHeader, menu, &MenuItem_ControllerSelect);
    AddMenuItem(menuHeader, menu, &MenuItem_ControllerSetup);
    AddMenuItem(menuHeader, menu, &MenuItem_ControllerDefault);
    AddMenuItem(menuHeader, menu, &MenuItem_SteeringDeadZone);
    AddMenuItem(menuHeader, menu, &MenuItem_SteeringRange);
    AddMenuItem(menuHeader, menu, &MenuItem_NonLinearSteer);
}

// Utility
void MenuControlConfigBack(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    if (gJoyTypeIn != RegistrySettings.Joystick) {
        MenuSetControlsDefault(menuHeader, menu, menuItem);
    }

    // set steering deadzone / range?
  #ifndef XBOX_NOT_YET_IMPLEMENTED
    if (RegistrySettings.Joystick != -1)
    {
        if (KeyTable[KEY_LEFT].Type == KEY_TYPE_AXISNEG || KeyTable[KEY_LEFT].Type == KEY_TYPE_AXISPOS)
            SetAxisProperties(KeyTable[KEY_LEFT].Index, RegistrySettings.SteeringDeadzone * 100, RegistrySettings.SteeringRange * 100);

        if (KeyTable[KEY_RIGHT].Type == KEY_TYPE_AXISNEG || KeyTable[KEY_RIGHT].Type == KEY_TYPE_AXISPOS)
            SetAxisProperties(KeyTable[KEY_RIGHT].Index, RegistrySettings.SteeringDeadzone * 100, RegistrySettings.SteeringRange * 100);
    }
  #endif // ! XBOX_NOT_YET_IMPLEMENTED

    MenuGoBack(menuHeader, menu, menuItem);
}

void MenuSetControlsDefault(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    CRD_SetDefaultControls();
	PlaySfx( SFX_MENU_FORWARD, SFX_MAX_VOL, SFX_CENTRE_PAN, 44100, 0x7fffffff );
    //SetConsoleMessage("Controls set to default", NULL, 0xffffff, 0xffffff, 10, CONSOLE_MESSAGE_DEFAULT_TIME);
}


#else  // PC

////////////////////////////////////////////////////////////////
//
// Controller option
//
////////////////////////////////////////////////////////////////

// Menu
MENU Menu_ControllerSettings = {
    TEXT_CONTROLLER,
    MENU_IMAGE_OPTIONS,                     // Menu title bar image enum
    MENU_DEFAULT,                           // Menu type
    SPRU_COL_RED,                           // Spru colour
    CreateControllerMenu,                   // Create menu function
    MENU_CONTROLSELECT_XPOS,                // X coord
    MENU_CONTROLSELECT_YPOS,                // Y Coord
    0,                                      // Current Item index
    NULL,                                   // Parent Menu (OverWritten in Create)
};

// Controller - Non linear steering
MENU_ITEM MenuItem_NonLinearSteer = {
    TEXT_STEERING,                          // Text label index
    TITLESCREEN_CAMPOS_OVERVIEW,                     // Camera pos index

    MENU_DATA_WIDTH_BOOL,                   // Space needed to draw item data
    &NonLinearSteering,                     // Data
    DrawMenuDataOnOff,                      // Draw Function

 	NULL,								    // Input handler
	0,                                      // Flags

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    ToggleMenuDataOff,                      // Left Action
    ToggleMenuDataOn,                       // Right Action
    MenuGoBack,                             // Back Action
    NULL,                                   // Forward Action
};

// Player Number
MENU_ITEM MenuItem_ControllerPlayerNum = {
    TEXT_PLAYERNUM,                         // Text label index
    TITLESCREEN_CAMPOS_OVERVIEW,                     // Camera pos index

    MENU_DATA_WIDTH_INT,                    // Space needed to draw item data
    NULL,                                   // Data
    DrawControllerPlayerNum,                // Draw Function

 	NULL,								    // Input handler
	0,                                      // Flags

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    DecreaseControllerPlayerNum,            // Left Action
    IncreaseControllerPlayerNum,            // Right Action
    MenuGoBack,                             // Back Action
    NULL,                                   // Forward Action
};

// Controller config type
MENU_ITEM MenuItem_ControllerConfig = {
    TEXT_CONTROLLERCONFIGNUM,               // Text label index
    TITLESCREEN_CAMPOS_OVERVIEW,                     // Camera pos index

    MENU_DATA_WIDTH_INT,                    // Space needed to draw item data
    NULL,                                   // Data
    DrawControlMethod,                      // Draw Function

	NULL,								    // Input handler
	0,                                      // Flags

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    DecreaseControlMethod,                  // Left Action
    IncreaseControlMethod,                  // Right Action
    MenuGoBack,                             // Back Action
    NULL,                                   // Forward Action
};

#ifndef CONTROL_CONFIG_NTYPES
#define CONTROL_CONFIG_NTYPES 4
#endif

// Create
void CreateControllerMenu(MENU_HEADER *menuHeader, MENU *menu)
{
    //AddMenuItem(menuHeader, menu, &MenuItem_NonLinearSteer);
    AddMenuItem(menuHeader, menu, &MenuItem_ControllerPlayerNum);
    AddMenuItem(menuHeader, menu, &MenuItem_ControllerConfig);

    // Switch off "Go-Forward" icon
    menuHeader->NavFlags &= ~MENU_FLAG_ADVANCE;

}

// Utility
void DecreaseControlMethod(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    if (g_TitleScreenData.iButtonConfig[g_TitleScreenData.iCurrentPlayer] > 0) {
        g_TitleScreenData.iButtonConfig[g_TitleScreenData.iCurrentPlayer]--;
    }
}
void IncreaseControlMethod(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{

    if (g_TitleScreenData.iButtonConfig[g_TitleScreenData.iCurrentPlayer] < (CONTROL_CONFIG_NTYPES-1)) {
        g_TitleScreenData.iButtonConfig[g_TitleScreenData.iCurrentPlayer]++;
    }
}

void DecreaseControllerPlayerNum(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    if (g_TitleScreenData.iCurrentPlayer > 0) {
        g_TitleScreenData.iCurrentPlayer--;
    }
}

void IncreaseControllerPlayerNum(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    if (g_TitleScreenData.iCurrentPlayer < MAX_LOCAL_PLAYERS - 1) {
        g_TitleScreenData.iCurrentPlayer++;
    }
}

#endif


////////////////////////////////////////////////////////////////
//
// DrawControllerType
//
////////////////////////////////////////////////////////////////
#ifdef _PC
void DrawControllerType(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex)
{
    REAL xPos, yPos;
    char *text;

    xPos = menuHeader->XPos + menuHeader->ItemTextWidth + MENU_TEXT_GAP;
    yPos = menuHeader->YPos + (MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP) * itemIndex;

    if (RegistrySettings.Joystick == -1)
    {
        text = "Keyboard";

        // Disable steering stuff
        MenuItem_NonLinearSteer.ActiveFlags = 0;
        MenuItem_SteeringRange.ActiveFlags = 0;
        MenuItem_SteeringDeadZone.ActiveFlags = 0;
    }
    else
    {
//$MODIFIED
//        text = Joystick[RegistrySettings.Joystick].Name;
        text = "<joystick name goes here>";
//$END_MODIFICATIONS

        // Enable steering stuff
        MenuItem_NonLinearSteer.ActiveFlags = MENU_ITEM_ACTIVE | MENU_ITEM_SELECTABLE;
        MenuItem_SteeringRange.ActiveFlags = MENU_ITEM_ACTIVE | MENU_ITEM_SELECTABLE;
        MenuItem_SteeringDeadZone.ActiveFlags = MENU_ITEM_ACTIVE | MENU_ITEM_SELECTABLE;
    }

    DrawMenuText(xPos,yPos, MENU_TEXT_RGB_NORMAL, text );
}
#endif

////////////////////////////////////////////////////////////////
//
// DrawControllerConfig
//
////////////////////////////////////////////////////////////////
#ifdef _PC
extern long ControllerConfigPick;
extern KEY LastConfigKey;

static long ConfigAllowed[] = {
    KEY_LEFT,
    KEY_RIGHT,
    KEY_FWD,
    KEY_BACK,
    KEY_FIRE,
    KEY_RESET,
    KEY_REPOSITION,
    KEY_HONKA,
    KEY_PAUSE,

    -1
};

static long ConfigText[] = {
    TEXT_CONTROL_LEFT,
    TEXT_CONTROL_RIGHT,
    TEXT_CONTROL_ACC,
    TEXT_CONTROL_REV,
    TEXT_CONTROL_FIRE,
    TEXT_CONTROL_FLIP,
    TEXT_CONTROL_REPOSITION,
    TEXT_CONTROL_HORN,
    TEXT_CONTROL_PAUSE,
};

bool KeyAllowed(KEY *key)
{
    long i;

    if ((key->Type == KEY_TYPE_KEYBOARD) && (key->Index == DIK_ESCAPE)) return false;

    for (i = 0 ; i < ControllerConfigPick ; i++)
    {
        if (KeyTable[ConfigAllowed[i]].Type == key->Type && KeyTable[ConfigAllowed[i]].Index == key->Index)
            return false;
    }

    return true;
}

void DrawControllerConfigure(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex)
{
    long i;
    short x, y;
    KEY key;
    char *text;

// display key choices

    x = (short)menuHeader->XPos;
    y = (short)menuHeader->YPos;

    DrawSpruBox(
        gMenuWidthScale * (x - MENU_TEXT_GAP), 
        gMenuHeightScale * (y - MENU_TEXT_GAP),
        gMenuWidthScale * (MENU_TEXT_WIDTH * 30 + MENU_TEXT_GAP * 2),
        gMenuHeightScale * ((MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP) * 8 + MENU_TEXT_GAP * 2), 
        menuHeader->SpruColIndex, 0);   

    BeginTextState();

    for (i = 0 ; i < ControllerConfigPick + 1; i++, y += MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP)
    {
        if (ConfigAllowed[i] == -1)
        {
            MenuGoBack(menuHeader, menu, menuItem);
            return;
        }

        sprintf(MenuBuffer, "%s:", TEXT_TABLE(ConfigText[i]));
        DrawMenuText(x, y, MENU_TEXT_RGB_NORMAL, MenuBuffer);
        if (i != ControllerConfigPick)
        {
            switch (KeyTable[ConfigAllowed[i]].Type)
            {
                case KEY_TYPE_KEYBOARD:
                    text = GetStringFromDik(KeyTable[ConfigAllowed[i]].Index);
                    break;

                case KEY_TYPE_BUTTON:
//$MODIFIED
//                    text = Joystick[RegistrySettings.Joystick].ButtonName[KeyTable[ConfigAllowed[i]].Index];
                    text = "<button name here>";
//$END_MODIFICATIONS
                    break;

                case KEY_TYPE_AXISNEG:
//$MODIFIED
//                    wsprintf(MenuBuffer, "-%s", Joystick[RegistrySettings.Joystick].AxisName[KeyTable[ConfigAllowed[i]].Index]);
                    wsprintf(MenuBuffer, "-%s", "<AxisNameHere>");
//$END_MODIFICATIONS
                    text = MenuBuffer;
                    break;

                case KEY_TYPE_AXISPOS:
//$MODIFIED
//                    wsprintf(MenuBuffer, "+%s", Joystick[RegistrySettings.Joystick].AxisName[KeyTable[ConfigAllowed[i]].Index]);
                    wsprintf(MenuBuffer, "+%s", "<AxisNameHere>");
//$END_MODIFICATIONS
                    text = MenuBuffer;
                    break;
            }
            DrawMenuText(x + MENU_TEXT_WIDTH * 20, y, MENU_TEXT_RGB_NORMAL, text);
        }
    }

// look for a key press

    i = SearchForKeyPress(&key);
    if (i && KeyAllowed(&key))
    {
        if ((key.Type != LastConfigKey.Type || key.Index != LastConfigKey.Index))
        {
            LastConfigKey = key;
            if (i == 2)
            {
                PlaySfx(SFX_MENU_BACK, SFX_MAX_VOL, SFX_CENTRE_PAN, SFX_SAMPLE_RATE, 0x7fffffff);
            }
            else
            {
                KeyTable[ConfigAllowed[ControllerConfigPick]] = key;
                ControllerConfigPick++;
            }
        }
    }
    else
    {
        LastConfigKey.Type = -1;
    }
}
#endif
