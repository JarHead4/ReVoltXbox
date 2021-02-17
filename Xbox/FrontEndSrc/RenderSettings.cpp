//-----------------------------------------------------------------------------
// File: RenderSettings.cpp
//
// Desc: RenderSettings.cpp
//
// Hist: 02.08.02 - ported/created
//
// Copyright (c)  Microsoft Corporation.  All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"
#include "menu2.h"
#include "MenuDraw.h"   // DrawSpruBox
#include "settings.h"   // RegistrySettings
#include "spark.h"      // gSmokeDensity

// re-volt specific
#include "menutext.h"   // re-volt strings

// temporary includes?
#include "titlescreen.h"  // TITLESCREEN_CAMPOS_ & g_bTitleScreenRunDemo
#include "gameloop.h"       // DemoTimeout

#define MENU_RENDERSETTINGS_XPOS        Real(100)
#define MENU_RENDERSETTINGS_YPOS        Real(150)

#define MENU_INGAME_XPOS                Real(100)
#define MENU_INGAME_YPOS                Real(150)

static void CreateVideoSettingsMenu(MENU_HEADER *menuHeader, MENU *menu);
static void CreateRenderSettingsMenu(MENU_HEADER *menuHeader, MENU *menu);
static void CreateInGameGraphicsMenu(MENU_HEADER *menuHeader, MENU *menu);

static void ToggleWireFrameOff(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
static void ToggleWireFrameOn(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
static void ToggleAntiAliasOff(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
static void ToggleAntiAliasOn(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
static void ToggleVsyncOff(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
static void ToggleVsyncOn(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
static void ToggleShowFPSOff(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
static void ToggleShowFPSOn(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
static void NextTextureFilter(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
static void PrevTextureFilter(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
static void PrevMipMapLevel(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
static void NextMipMapLevel(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
static void SetSmokeLevel(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex);

//void ToggleWireFrameOff(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
//void ToggleWireFrameOn(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);

static void DrawDrawDist(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex);

static void DrawTextureFilter(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex);
static void DrawMipMapLevel(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex);
static void DrawWireFrame(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex);
static void DrawParticleLevel(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex);

#ifdef _PC
SLIDER_DATA_ULONG DrawDistSlider = {
    &RegistrySettings.DrawDist,
    0, 4, 1,
    FALSE, TRUE,
};

SLIDER_DATA_LONG ParticlesSlider = {
    &g_TitleScreenData.sparkLevel,
    0, 2, 1,
    FALSE, TRUE,
};

SLIDER_DATA_LONG TextureFilterSlider = {
    &g_TitleScreenData.textureFilter,
    0, 2, 1,
    FALSE, TRUE,
};

SLIDER_DATA_LONG MipMapSlider = {
    &g_TitleScreenData.mipLevel,
    0, 2, 1,
    FALSE, TRUE,
};

// Menu
MENU Menu_RenderSettings = {
    TEXT_RENDERSETTINGS,
    MENU_IMAGE_OPTIONS,                     // Menu title bar image enum
    TITLESCREEN_CAMPOS_OVERVIEW,                     // Camera pos index
    MENU_DEFAULT | MENU_CENTRE_X,           // Menu type
    SPRU_COL_RANDOM,                        // Spru colour
    CreateRenderSettingsMenu,
    MENU_RENDERSETTINGS_XPOS,               // X coord
    MENU_RENDERSETTINGS_YPOS,               // Y Coord
    0,
    NULL,
};

// Render Settings - Draw Dist
MENU_ITEM MenuItem_DrawDist = {
    TEXT_DRAWDIST,                          // Text label index

    MENU_DATA_WIDTH_BOOL,                   // Space needed to draw item data
    &DrawDistSlider,                        // Data
    DrawDrawDist,                           // Draw Function

	NULL,								    // Input handler
	0,                                      // Flags

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    DecreaseSliderDataULong,                // Left Action
    IncreaseSliderDataULong,                // Right Action
    MenuGoBack,                             // Back Action
    NULL,                                   // Forward Action
};

// Render Settings - Reflections
MENU_ITEM MenuItem_Reflections = {
    TEXT_REFLECTIONS,                       // Text label index

    MENU_DATA_WIDTH_BOOL,                   // Space needed to draw item data
    &g_TitleScreenData.reflections,          // Data
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

// Render Settings - Env Maps
MENU_ITEM MenuItem_Shinyness = {
    TEXT_SHINYNESS,                         // Text label index

    MENU_DATA_WIDTH_BOOL,                   // Space needed to draw item data
    &g_TitleScreenData.shinyness,            // Data
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

// Render Settings - Lights
MENU_ITEM MenuItem_Lights = {
    TEXT_LIGHTS,                            // Text label index

    MENU_DATA_WIDTH_BOOL,                   // Space needed to draw item data
    &g_TitleScreenData.lights,               // Data
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

// Render Settings - Instances
MENU_ITEM MenuItem_Instances = {
    TEXT_INSTANCES,                         // Text label index

    MENU_DATA_WIDTH_BOOL,                   // Space needed to draw item data
    &g_TitleScreenData.instances,            // Data
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

// Render Settings - SkidMarks
MENU_ITEM MenuItem_SkidMarks = {
    TEXT_SKIDMARKS,                         // Text label index

    MENU_DATA_WIDTH_BOOL,                   // Space needed to draw item data
    &g_TitleScreenData.skidmarks,            // Data
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

// Render Settings - Shadows
MENU_ITEM MenuItem_Shadows = {
    TEXT_SHADOWS,                           // Text label index

    MENU_DATA_WIDTH_BOOL,                   // Space needed to draw item data
    &g_TitleScreenData.shadows,              // Data
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

// Render Settings - Smoke
MENU_ITEM MenuItem_Smoke = {
    TEXT_SMOKE,                         // Text label index

    MENU_DATA_WIDTH_BOOL,                   // Space needed to draw item data
    &g_TitleScreenData.smoke,                // Data
    SetSmokeLevel,                          // Draw Function

	NULL,								    // Input handler
	0,                                      // Flags

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    ToggleMenuDataOff,                      // Left Action
    ToggleMenuDataOn,                       // Right Action
    MenuGoBack,                             // Back Action
    NULL,                                   // Forward Action
};


// Render Settings - Particles
MENU_ITEM MenuItem_Particles = {
    TEXT_PARTICLES,                         // Text label index

    MENU_TEXT_WIDTH * 13,                   // Space needed to draw item data
    &ParticlesSlider,                       // Data
    DrawParticleLevel,                      // Draw Function

	NULL,								    // Input handler
	0,                                      // Flags

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    DecreaseSliderDataLong,                 // Left Action
    IncreaseSliderDataLong,                 // Right Action
    MenuGoBack,                             // Back Action
    NULL,                                   // Forward Action
};

// RenderSettings - AntiAlias
MENU_ITEM MenuItem_AntiAlias = {
    TEXT_ANTIALIAS,                         // Text label index

    0,                                      // Space needed to draw item data
    &g_TitleScreenData.antialias,            // Data
    DrawMenuDataOnOff,                      // Draw Function

	NULL,								    // Input handler
	0,                                      // Flags

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    ToggleAntiAliasOff,                     // Left Action
    ToggleAntiAliasOn,                      // Right Action
    MenuGoBack,                             // Back Action
    NULL,                                   // Forward Action
};

// RenderSettings - Texture Filter
MENU_ITEM MenuItem_TextureFilter = {
    TEXT_TEXTUREFILTER,                     // Text label index

    0,                                      // Space needed to draw item data
    &TextureFilterSlider,                   // Data
    DrawTextureFilter,                      // Draw Function

	NULL,								    // Input handler
	0,                                      // Flags

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    PrevTextureFilter,                      // Left Action
    NextTextureFilter,                      // Right Action
    MenuGoBack,                             // Back Action
    NULL,                                   // Forward Action
};

// RenderSettings - Mip map
MENU_ITEM MenuItem_MipMap = {
    TEXT_MIPMAP,                            // Text label index

    0,                                      // Space needed to draw item data
    &MipMapSlider,                          // Data
    DrawMipMapLevel,                        // Draw Function

	NULL,								    // Input handler
	0,                                      // Flags

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    PrevMipMapLevel,                        // Left Action
    NextMipMapLevel,                        // Right Action
    MenuGoBack,                             // Back Action
    NULL,                                   // Forward Action
};

// RenderSettings - Vsync
MENU_ITEM MenuItem_Vsync = {
    TEXT_VSYNC,                             // Text label index

    0,                                      // Space needed to draw item data
    &RegistrySettings.Vsync,                // Data
    DrawMenuDataOnOff,                      // Draw Function

	NULL,								    // Input handler
	0,                                      // Flags

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    ToggleVsyncOff,                         // Left Action
    ToggleVsyncOn,                          // Right Action
    MenuGoBack,                             // Back Action
    NULL,                                   // Forward Action
};

// RenderSettings - ShowFPS
MENU_ITEM MenuItem_ShowFPS = {
    TEXT_SHOW_FPS,                          // Text label index

    0,                                      // Space needed to draw item data
    &RegistrySettings.ShowFPS,              // Data
    DrawMenuDataOnOff,                      // Draw Function

	NULL,								    // Input handler
	0,                                      // Flags

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    ToggleShowFPSOff,                       // Left Action
    ToggleShowFPSOn,                        // Right Action
    MenuGoBack,                             // Back Action
    NULL,                                   // Forward Action
};

// Create
void CreateRenderSettingsMenu(MENU_HEADER *menuHeader, MENU *menu)
{
    AddMenuItem(menuHeader, menu, &MenuItem_ShowFPS);
    AddMenuItem(menuHeader, menu, &MenuItem_Vsync);
    AddMenuItem(menuHeader, menu, &MenuItem_TextureFilter);
    AddMenuItem(menuHeader, menu, &MenuItem_MipMap);
    AddMenuItem(menuHeader, menu, &MenuItem_AntiAlias);
    AddMenuItem(menuHeader, menu, &MenuItem_DrawDist);
    AddMenuItem(menuHeader, menu, &MenuItem_Reflections);
    AddMenuItem(menuHeader, menu, &MenuItem_SkidMarks);
    AddMenuItem(menuHeader, menu, &MenuItem_Particles);
    AddMenuItem(menuHeader, menu, &MenuItem_Shadows);
    AddMenuItem(menuHeader, menu, &MenuItem_Shinyness);
    AddMenuItem(menuHeader, menu, &MenuItem_Lights);
    AddMenuItem(menuHeader, menu, &MenuItem_Instances);

#ifndef XBOX_NOT_YET_IMPLEMENTED
    if (!DxState.AntiAliasEnabled) {
        MenuItem_AntiAlias.ActiveFlags = 0;
    }
#endif // ! XBOX_NOT_YET_IMPLEMENTED
}

// Utility
void SetSmokeLevel(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex)
{
    if (g_TitleScreenData.smoke)
        gSmokeDensity = ONE;
    else 
        gSmokeDensity = ZERO;

    DrawMenuDataOnOff(menuHeader, menu, menuItem, itemIndex);
}

////////////////////////////////////////////////////////////////
//
// In Game Graphics Options Menu
//
////////////////////////////////////////////////////////////////

// Menu
MENU Menu_InGameGraphicsOptions = {
    TEXT_VIDEOSETTINGS,
    MENU_IMAGE_NONE,                        // Menu title bar image enum
    TITLESCREEN_CAMPOS_START,                        // Camera pos index
    MENU_DEFAULT | MENU_CENTRE_X | MENU_CENTRE_Y,   // Menu type
    SPRU_COL_RANDOM,                        // Spru colour
    CreateInGameGraphicsMenu,               // Create menu function
    MENU_INGAME_XPOS,                       // X coord
    MENU_INGAME_YPOS,                       // Y Coord
    0,                                      // Current Item index
    NULL,                                   // Parent Menu (OverWritten in Create)
};

SLIDER_DATA_LONG WireFrameSlider = {
    &DxState.WireframeEnabled,
    0, 1, 1,
    FALSE, TRUE,
};

// In Game Graphics - Wireframe
MENU_ITEM MenuItem_WireFrame = {
    TEXT_WIREFRAME,                         // Text label index

    0,                                      // Space needed to draw item data
    NULL,                                   // Data
    DrawWireFrame,                          // Draw Function

	NULL,								    // Input handler
	0,                                      // Flags

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    ToggleWireFrameOff,                     // Left Action
    ToggleWireFrameOn,                      // Right Action
    MenuGoBack,                             // Back Action
    NULL,                                   // Forward Action
};

// In Game Graphics - Collision
MENU_ITEM MenuItem_Collision = {
    TEXT_COLLISION,                         // Text label index

    0,                                      // Space needed to draw item data
    &DrawGridCollSkin,                      // Data
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


// In Game Graphics - Car BBoxes
MENU_ITEM MenuItem_CarBBoxes = {
    TEXT_CARBBOXES,                         // Text label index

    0,                                      // Space needed to draw item data
    &CAR_DrawCarBBoxes,                     // Data
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

#endif // _PC

// Create
void CreateInGameGraphicsMenu(MENU_HEADER *menuHeader, MENU *menu)
{
    menu->CurrentItemIndex = 0;
#ifdef _PC
    AddMenuItem(menuHeader, menu, &MenuItem_ShowFPS);
    AddMenuItem(menuHeader, menu, &MenuItem_Vsync);
    AddMenuItem(menuHeader, menu, &MenuItem_TextureFilter);
    AddMenuItem(menuHeader, menu, &MenuItem_MipMap);
    AddMenuItem(menuHeader, menu, &MenuItem_AntiAlias);
    AddMenuItem(menuHeader, menu, &MenuItem_DrawDist);
    AddMenuItem(menuHeader, menu, &MenuItem_Reflections);
    AddMenuItem(menuHeader, menu, &MenuItem_SkidMarks);
    AddMenuItem(menuHeader, menu, &MenuItem_Particles);
    AddMenuItem(menuHeader, menu, &MenuItem_Shadows);
    AddMenuItem(menuHeader, menu, &MenuItem_Shinyness);
    AddMenuItem(menuHeader, menu, &MenuItem_Lights);
//  AddMenuItem(menuHeader, menu, &MenuItem_Instances);

#ifndef XBOX_NOT_YET_IMPLEMENTED
    if (!DxState.AntiAliasEnabled) {
        MenuItem_AntiAlias.ActiveFlags = 0;
    }
#endif // ! XBOX_NOT_YET_IMPLEMENTED

    if (Version == VERSION_DEV) {
        AddMenuItem(menuHeader, menu, &MenuItem_WireFrame);
        AddMenuItem(menuHeader, menu, &MenuItem_Collision);
        AddMenuItem(menuHeader, menu, &MenuItem_CarBBoxes);

        if (!DxState.WireframeEnabled) {
            MenuItem_WireFrame.ActiveFlags = 0;
        }
    }
#endif // _PC
}

// Utility
#ifdef _PC
void ToggleAntiAliasOff(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
  #ifndef XBOX_NOT_YET_IMPLEMENTED
    g_TitleScreenData.antialias = FALSE;
    DxState.AntiAlias = D3DANTIALIAS_NONE;
    ANTIALIAS_ON();
  #endif // ! XBOX_NOT_YET_IMPLEMENTED
}

void ToggleAntiAliasOn(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
  #ifndef XBOX_NOT_YET_IMPLEMENTED
    g_TitleScreenData.antialias = TRUE;
    DxState.AntiAlias = D3DANTIALIAS_SORTINDEPENDENT;
    ANTIALIAS_ON();
  #endif // ! XBOX_NOT_YET_IMPLEMENTED
}

void ToggleVsyncOff(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    RegistrySettings.Vsync = FALSE;
}

void ToggleVsyncOn(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    RegistrySettings.Vsync = TRUE;
}

void ToggleShowFPSOff(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    RegistrySettings.ShowFPS = FALSE;
}

void ToggleShowFPSOn(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    RegistrySettings.ShowFPS = TRUE;
}

void NextTextureFilter(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    IncreaseSliderDataLong(menuHeader, menu, menuItem);
    DxState.TextureFilter = g_TitleScreenData.textureFilter;
    TEXTUREFILTER_ON();
}

void PrevTextureFilter(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    DecreaseSliderDataLong(menuHeader, menu, menuItem);
    DxState.TextureFilter = g_TitleScreenData.textureFilter;
    TEXTUREFILTER_ON();
}

void NextMipMapLevel(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    IncreaseSliderDataLong(menuHeader, menu, menuItem);
    DxState.MipMap = g_TitleScreenData.mipLevel;
    MIPMAP_ON();
}

void PrevMipMapLevel(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    DecreaseSliderDataLong(menuHeader, menu, menuItem);
    DxState.MipMap = g_TitleScreenData.mipLevel;
    MIPMAP_ON();
}

void ToggleWireFrameOff(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
//  DxState.Wireframe = D3DFILL_SOLID;
    Wireframe = FALSE;
}

void ToggleWireFrameOn(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
//  DxState.Wireframe = D3DFILL_WIREFRAME;
    Wireframe = TRUE;
}

#endif 
////////////////////////////////////////////////////////////////
//
// Draw Draw Dist
//
////////////////////////////////////////////////////////////////

char *DrawDistStrings[] = {
    "Lowest",
    "Low",
    "Medium",
    "High",
    "Highest",
};

void DrawDrawDist(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex)
{
    REAL xPos, yPos;

#ifdef _PC
    xPos = menuHeader->XPos + menuHeader->ItemTextWidth + MENU_TEXT_GAP;
    yPos = menuHeader->YPos + (MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP) * itemIndex;

    sprintf(MenuBuffer, "%s", DrawDistStrings[RegistrySettings.DrawDist]);

    DrawMenuText(xPos, yPos, MENU_TEXT_RGB_NORMAL, MenuBuffer);
#endif
}

////////////////////////////////////////////////////////////////
//
// Draw Texture filter
//
////////////////////////////////////////////////////////////////

char *TextureFilterStrings[] = {
    "Point",
    "Linear",
    "Anisotropic",
};

void DrawTextureFilter(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex)
{
    REAL xPos, yPos;
    long col;

#ifdef _PC
    xPos = menuHeader->XPos + menuHeader->ItemTextWidth + MENU_TEXT_GAP;
    yPos = menuHeader->YPos + (MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP) * itemIndex;

    if (DxState.TextureFilterFlag & (1 << DxState.TextureFilter)) {
        col = MENU_TEXT_RGB_NORMAL;
    } else {
        col = MENU_TEXT_RGB_LOLITE;
    }

    sprintf(MenuBuffer, "%s", TextureFilterStrings[g_TitleScreenData.textureFilter]);

    DrawMenuText(xPos, yPos, col, MenuBuffer);
#endif
}

////////////////////////////////////////////////////////////////
//
// Draw Texture filter
//
////////////////////////////////////////////////////////////////

char *MipMapStrings[] = {
    "None",
    "Point",
    "Linear",
};

void DrawMipMapLevel(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex)
{
    REAL xPos, yPos;
    long col;

#ifdef _PC
    xPos = menuHeader->XPos + menuHeader->ItemTextWidth + MENU_TEXT_GAP;
    yPos = menuHeader->YPos + (MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP) * itemIndex;

    if (DxState.MipMapFlag & (1 << DxState.MipMap)) {
        col = MENU_TEXT_RGB_NORMAL;
    } else {
        col = MENU_TEXT_RGB_LOLITE;
    }

    sprintf(MenuBuffer, "%s", MipMapStrings[g_TitleScreenData.mipLevel]);

    DrawMenuText(xPos, yPos, col, MenuBuffer);
#endif
}

////////////////////////////////////////////////////////////////
//
// Draw WireFrame
//
////////////////////////////////////////////////////////////////

void DrawWireFrame(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex)
{
    REAL xPos, yPos;

#ifdef _PC
    xPos = menuHeader->XPos + menuHeader->ItemTextWidth + MENU_TEXT_GAP;
    yPos = menuHeader->YPos + (MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP) * itemIndex;

    sprintf(MenuBuffer, "%s", Wireframe ? "On": "Off");

    DrawMenuText(xPos, yPos, MENU_TEXT_RGB_NORMAL, MenuBuffer);
#endif
}

////////////////////////////////////////////////////////////////
//
// Draw Particle Level
//
////////////////////////////////////////////////////////////////

char *ParticleStrings[] = {
    "None",
    "Some",
    "Lots",
};

void DrawParticleLevel(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex)
{
    REAL xPos, yPos;

    gSparkDensity = g_TitleScreenData.sparkLevel * HALF;

#ifdef _PC
    xPos = menuHeader->XPos + menuHeader->ItemTextWidth + MENU_TEXT_GAP;
    yPos = menuHeader->YPos + (MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP) * itemIndex;

    sprintf(MenuBuffer, "%s", ParticleStrings[g_TitleScreenData.sparkLevel]);

    DrawMenuText(xPos, yPos, MENU_TEXT_RGB_NORMAL, MenuBuffer);
#endif
}

