//-----------------------------------------------------------------------------
// File: VideoSettings.cpp
//
// Desc: VideoSettings.cpp
//
// Hist: 02.08.02 - ported/created
//
// Copyright (c)  Microsoft Corporation.  All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"
#include "menu2.h"
#include "MenuDraw.h"       // DrawSpruBox
#include "settings.h"       // RegistrySettings
#include "RenderSettings.h" // Menu_RenderSettings

// re-volt specific
#include "menutext.h"   // re-volt strings

// temporary includes?
#include "titlescreen.h"  // TITLESCREEN_CAMPOS_ & g_bTitleScreenRunDemo

#define MENU_VIDEOOPTIONS_XPOS          Real(100)
#define MENU_VIDEOOPTIONS_YPOS          Real(150)

static void CreateVideoSettingsMenu(MENU_HEADER *menuHeader, MENU *menu);

static void SelectPrevVideoDevice(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
static void SelectNextVideoDevice(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
static void SelectPrevVideoMode(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
static void SelectNextVideoMode(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
static void DecTextureBpp(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
static void IncTextureBpp(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
static void DecreaseBrightness(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
static void IncreaseBrightness(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
static void DecreaseContrast(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
static void IncreaseContrast(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);

static void DrawVideoDevice(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex);
static void DrawVideoMode(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex);

static void DrawTextureBpp(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex);

// Menu
#ifdef _PC
MENU Menu_VideoSettings = {
    TEXT_VIDEOSETTINGS,
    MENU_IMAGE_OPTIONS,                     // Menu title bar image enum
    TITLESCREEN_CAMPOS_OVERVIEW,                     // Camera pos index
    MENU_DEFAULT | MENU_CENTRE_X,   // Menu type
    SPRU_COL_RANDOM,                        // Spru colour
    CreateVideoSettingsMenu,                // Create menu function
    MENU_VIDEOOPTIONS_XPOS,                 // X coord
    MENU_VIDEOOPTIONS_YPOS,                 // Y Coord
    0,                                      // Current Item index
    NULL,                                   // Parent Menu (OverWritten in Create)
};

// Data
unsigned long gBrightness;
unsigned long gContrast;
SLIDER_DATA_ULONG BrightnessSlider = {
    &gBrightness,
    0, 100, 1,
    TRUE, TRUE,
};
SLIDER_DATA_ULONG ContrastSlider = {
    &gContrast,
    0, 100, 1,
    TRUE, TRUE,
};

// Video Settings - Device
MENU_ITEM MenuItem_VideoDevice = {
    TEXT_DRAWDEVICE,                        // Text label index

    MENU_DATA_WIDTH_TEXT,                   // Space needed to draw item data
    NULL,                                   // Data
    DrawVideoDevice,                        // Draw Function

	NULL,								    // Input handler
	0,                                      // Flags

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    SelectPrevVideoDevice,                  // Left Action
    SelectNextVideoDevice,                  // Right Action
    MenuGoBack,                             // Back Action
    NULL,                                   // Forward Action
};

// Video Settings - Mode
MENU_ITEM MenuItem_VideoMode = {
    TEXT_SCREENRES,                         // Text label index

    MENU_DATA_WIDTH_TEXT,                   // Space needed to draw item data
    NULL,                                   // Data
    DrawVideoMode,                          // Draw Function

	NULL,								    // Input handler
	0,                                      // Flags

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    SelectPrevVideoMode,                    // Left Action
    SelectNextVideoMode,                    // Right Action
    MenuGoBack,                             // Back Action
    NULL,                                   // Forward Action
};

// Video Settings - 16 / 24 bit textures
MENU_ITEM MenuItem_TextureBpp = {
    TEXT_TEXTURE,                           // Text label index

    MENU_DATA_WIDTH_TEXT,                   // Space needed to draw item data
    NULL,                                   // Data
    DrawTextureBpp,                         // Draw Function

	NULL,								    // Input handler
	0,                                      // Flags

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    DecTextureBpp,                          // Left Action
    IncTextureBpp,                          // Right Action
    MenuGoBack,                             // Back Action
    NULL,                                   // Forward Action
};

// Video Settings - Brightness
MENU_ITEM MenuItem_Brightness = {
    TEXT_BRIGHTNESS,                        // Text label index

    MENU_DATA_WIDTH_INT,                    // Space needed to draw item data
    &BrightnessSlider,                      // Data
    DrawSliderDataULong,                    // Draw Function

	NULL,								    // Input handler
	0,                                      // Flags

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    DecreaseBrightness,                     // Left Action
    IncreaseBrightness,                     // Right Action
    MenuGoBack,                             // Back Action
    NULL,                                   // Forward Action
};

// Video Settings - Contrast
MENU_ITEM MenuItem_Contrast = {
    TEXT_CONTRAST,                          // Text label index

    MENU_DATA_WIDTH_INT,                    // Space needed to draw item data
    &ContrastSlider,                        // Data
    DrawSliderDataULong,                    // Draw Function

	NULL,								    // Input handler
	0,                                      // Flags

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    DecreaseContrast,                       // Left Action
    IncreaseContrast,                       // Right Action
    MenuGoBack,                             // Back Action
    NULL,                                   // Forward Action
};

// Video Settings - RenderSettings
MENU_ITEM MenuItem_RenderSettings = {
    TEXT_RENDERSETTINGS,                    // Text label index

    0,                                      // Space needed to draw item data
    &Menu_RenderSettings,                   // Data
    NULL,                                   // Draw Function

	NULL,								    // Input handler
	0,                                      // Flags

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    NULL,                                   // Left Action
    NULL,                                   // Right Action
    MenuGoBack,                             // Back Action
    MenuGoForward,                          // Forward Action
};

// Create

void CreateVideoSettingsMenu(MENU_HEADER *menuHeader, MENU *menu)
{
    AddMenuItem(menuHeader, menu, &MenuItem_VideoDevice);
    AddMenuItem(menuHeader, menu, &MenuItem_VideoMode);
    AddMenuItem(menuHeader, menu, &MenuItem_TextureBpp);

    if ((GammaFlag != GAMMA_UNAVAILABLE) && (GammaFlag != GAMMA_AUTO)) {
        AddMenuItem(menuHeader, menu, &MenuItem_Brightness);
        AddMenuItem(menuHeader, menu, &MenuItem_Contrast);
    }
    AddMenuItem(menuHeader, menu, &MenuItem_RenderSettings);

    gBrightness = ((RegistrySettings.Brightness - 32) * 100) / (512 - 32);
    gContrast = ((RegistrySettings.Contrast - 32) * 100) / (512 - 32);
}


// Utility functions
void SelectPrevVideoDevice(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
#ifndef XBOX_NOT_YET_IMPLEMENTED
    if (RegistrySettings.DrawDevice > 0) {
        RegistrySettings.DrawDevice--;
    }
    DisplayModeCount = DrawDevices[RegistrySettings.DrawDevice].BestDisplayMode;
#endif
}

void SelectNextVideoDevice(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
#ifndef XBOX_NOT_YET_IMPLEMENTED
    if (RegistrySettings.DrawDevice < (DWORD)DrawDeviceNum - 1) {
        RegistrySettings.DrawDevice++;
    }
    DisplayModeCount = DrawDevices[RegistrySettings.DrawDevice].BestDisplayMode;
#endif
}

void SelectPrevVideoMode(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
#ifndef XBOX_NOT_YET_IMPLEMENTED
    DisplayModeCount--;
    if (DisplayModeCount < 0) DisplayModeCount = 0;

    RegistrySettings.ScreenWidth = DrawDevices[RegistrySettings.DrawDevice].DisplayMode[DisplayModeCount].Width;
    RegistrySettings.ScreenHeight = DrawDevices[RegistrySettings.DrawDevice].DisplayMode[DisplayModeCount].Height;
    RegistrySettings.ScreenBpp = DrawDevices[RegistrySettings.DrawDevice].DisplayMode[DisplayModeCount].Bpp;
#else
    //$NOTE: dont remove this function yet; probably want to support more modes eventually!!
    RegistrySettings.ScreenWidth = 640;
    RegistrySettings.ScreenHeight = 480;
    RegistrySettings.ScreenBpp = 32;
#endif
}

void SelectNextVideoMode(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
#ifndef XBOX_NOT_YET_IMPLEMENTED
    DisplayModeCount++;
    if (DisplayModeCount >= DrawDevices[RegistrySettings.DrawDevice].DisplayModeNum) DisplayModeCount = DrawDevices[RegistrySettings.DrawDevice].DisplayModeNum - 1;

    RegistrySettings.ScreenWidth = DrawDevices[RegistrySettings.DrawDevice].DisplayMode[DisplayModeCount].Width;
    RegistrySettings.ScreenHeight = DrawDevices[RegistrySettings.DrawDevice].DisplayMode[DisplayModeCount].Height;
    RegistrySettings.ScreenBpp = DrawDevices[RegistrySettings.DrawDevice].DisplayMode[DisplayModeCount].Bpp;
#else
    //$NOTE: dont remove this function yet; probably want to support more modes eventually!!
    RegistrySettings.ScreenWidth = 640;
    RegistrySettings.ScreenHeight = 480;
    RegistrySettings.ScreenBpp = 32;
#endif
}

void DecTextureBpp(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    RegistrySettings.Texture24 = FALSE;
}

void IncTextureBpp(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    RegistrySettings.Texture24 = TRUE;
}

void DecreaseBrightness(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    DecreaseSliderDataULong(menuHeader, menu, menuItem);
    RegistrySettings.Brightness = 32 + (gBrightness * (512 - 32)) / 100;
    SetGamma(RegistrySettings.Brightness, RegistrySettings.Contrast);
}

void IncreaseBrightness(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    IncreaseSliderDataULong(menuHeader, menu, menuItem);
    RegistrySettings.Brightness = 32 + (gBrightness * (512 - 32)) / 100;
    SetGamma(RegistrySettings.Brightness, RegistrySettings.Contrast);
}

void DecreaseContrast(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    DecreaseSliderDataULong(menuHeader, menu, menuItem);
    RegistrySettings.Contrast = 32 + (gContrast * (512 - 32)) / 100;
    SetGamma(RegistrySettings.Brightness, RegistrySettings.Contrast);
}

void IncreaseContrast(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    IncreaseSliderDataULong(menuHeader, menu, menuItem);
    RegistrySettings.Contrast = 32 + (gContrast * (512 - 32)) / 100;
    SetGamma(RegistrySettings.Brightness, RegistrySettings.Contrast);
}
#endif



////////////////////////////////////////////////////////////////
//
// Draw video device
//
////////////////////////////////////////////////////////////////

#ifdef _PC
void DrawVideoDevice(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex)
{
    REAL xPos, yPos;

    xPos = menuHeader->XPos + menuHeader->ItemTextWidth + MENU_TEXT_GAP;
    yPos = menuHeader->YPos + (MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP) * itemIndex;

//$MODIFIED
//    if (DrawDeviceNum > 1) {
//        DrawMenuText(xPos, yPos, MENU_TEXT_RGB_NORMAL, DrawDevices[RegistrySettings.DrawDevice].Name);
//    } else {
//        DrawMenuText(xPos, yPos, MENU_TEXT_RGB_LOLITE, DrawDevices[RegistrySettings.DrawDevice].Name);
//    }
    DrawMenuText(xPos, yPos, MENU_TEXT_RGB_LOLITE, "<xbox draw device>");
//$END_MODIFICATIONS
}
#endif


////////////////////////////////////////////////////////////////
//
// Draw video mode
//
////////////////////////////////////////////////////////////////

#ifdef _PC
void DrawVideoMode(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex)
{
    REAL xPos, yPos;

    xPos = menuHeader->XPos + menuHeader->ItemTextWidth + MENU_TEXT_GAP;
    yPos = menuHeader->YPos + (MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP) * itemIndex;

//$MODIFIED
//    DrawMenuText(xPos,yPos, MENU_TEXT_RGB_NORMAL, DrawDevices[RegistrySettings.DrawDevice].DisplayMode[DisplayModeCount].DisplayText);
    DrawMenuText(xPos,yPos, MENU_TEXT_RGB_NORMAL, "<xbox video mode goes here>");
//$END_MODIFICATION
}
#endif


//////////////////////
//
// draw texture bpp
//
//////////////////////

#ifdef _PC
void DrawTextureBpp(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex)
{
    REAL xPos, yPos;

    xPos = menuHeader->XPos + menuHeader->ItemTextWidth + MENU_TEXT_GAP;
    yPos = menuHeader->YPos + (MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP) * itemIndex;

    DrawMenuText(xPos,yPos, MENU_TEXT_RGB_NORMAL, RegistrySettings.Texture24 ? "24 Bit" : "16 Bit");
}
#endif

