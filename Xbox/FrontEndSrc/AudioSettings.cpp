//-----------------------------------------------------------------------------
// File: AudioSettings.cpp
//
// Desc: AudioSettings.cpp
//
// Hist: 02.08.02 - ported/created
//
// Copyright (c)  Microsoft Corporation.  All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"
#include "menu2.h"
#include "MenuDraw.h"   // DrawSpruBox
#include "main.h"       // TimeStep
#include "settings.h"   // RegistrySettings

// re-volt specific
#include "menutext.h"   // re-volt strings

// temporary includes?
#include "titlescreen.h"

#define MENU_AUDIOSETTINGS_XPOS         Real(100)
#define MENU_AUDIOSETTINGS_YPOS         Real(150)

static void CreateAudioSettingsMenu(MENU_HEADER *menuHeader, MENU *menu);

static void SetMusicVolume(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex);
static void StartMusicTest(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
static void StartSFXTest(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
static void SetSFXVolume(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex);


// Menu
MENU Menu_AudioSettings = {
    TEXT_AUDIOSETTINGS,
    MENU_IMAGE_OPTIONS,                     // Menu title bar image enum
    TITLESCREEN_CAMPOS_OVERVIEW,                     // Camera pos index
    MENU_DEFAULT | MENU_CENTRE_X,           // Menu type
    SPRU_COL_RANDOM,                        // Spru colour
    CreateAudioSettingsMenu,                // Create menu function
    MENU_AUDIOSETTINGS_XPOS,                // X coord
    MENU_AUDIOSETTINGS_YPOS,                // Y Coord
    0,                                      // Current Item index
    NULL,                                   // Parent Menu (OverWritten in Create)
};

// Data
SLIDER_DATA_LONG MusicVolumeSlider = {
    &g_TitleScreenData.musicVolume,
    0, 100, 10,
    TRUE, TRUE,
};
SLIDER_DATA_LONG SFXVolumeSlider = {
    &g_TitleScreenData.sfxVolume,
    0, 100, 10,
    TRUE, TRUE,
};
SLIDER_DATA_LONG MusicTestSlider = {
    &g_TitleScreenData.musicTest,
    0, 11, 1,
    FALSE, TRUE,
};
SLIDER_DATA_LONG SFXTestSlider = {
    &g_TitleScreenData.sfxTest,
    0, 31, 1,
    FALSE, TRUE,
};

SLIDER_DATA_LONG SFXChannelsSlider = {
    (long*)&RegistrySettings.SfxChannels,
    1, SFX_MAX_SAMPLES, 1,
    FALSE, TRUE,
};

void AudioMenuGoBack(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);

// Audio - Music on / off
#ifdef _PC
MENU_ITEM MenuItem_MusicOn = {
    TEXT_MUSIC_ON,                          // Text label index

    0,                                      // Space needed to draw item data
    &RegistrySettings.MusicOn,              // Data
    DrawMenuDataOnOff,                      // Draw Function

	NULL,								    // Input handler
	0,                                      // Flags

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    ToggleMenuDataOff,                      // Left Action
    ToggleMenuDataOn,                       // Right Action
    AudioMenuGoBack,                        // Back Action
    NULL,                                   // Forward Action
};
#endif

// Audio - Music Vol
MENU_ITEM MenuItem_MusicVolume = {
    TEXT_MUSICVOLUME,                       // Text label index

    MENU_DATA_WIDTH_SLIDER + 4 * MENU_TEXT_WIDTH,   // Space needed to draw item data
    &MusicVolumeSlider,                     // Data
    SetMusicVolume,                         // Draw Function

	NULL,								    // Input handler
	0,                                      // Flags

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    DecreaseSliderDataLong,                 // Left Action
    IncreaseSliderDataLong,                 // Right Action
    AudioMenuGoBack,                        // Back Action
    NULL,                                   // Forward Action
};

// Audio - SFX Vol
MENU_ITEM MenuItem_SFXVolume = {
    TEXT_SFXVOLUME,                         // Text label index

    MENU_DATA_WIDTH_SLIDER + 4 * MENU_TEXT_WIDTH,   // Space needed to draw item data
    &SFXVolumeSlider,                       // Data
    SetSFXVolume,                           // Draw Function

	NULL,								    // Input handler
	0,                                      // Flags

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    DecreaseSliderDataLong,                 // Left Action
    IncreaseSliderDataLong,                 // Right Action
    AudioMenuGoBack,                        // Back Action
    NULL,                                   // Forward Action
};

// Audio - Test Music
MENU_ITEM MenuItem_TestMusic = {
    TEXT_MUSICTEST,                         // Text label index

    MENU_DATA_WIDTH_INT,                    // Space needed to draw item data
    &MusicTestSlider,                       // Data
    DrawSliderDataLong,                         // Draw Function

	NULL,								    // Input handler
	0,                                      // Flags

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    DecreaseSliderDataLong,                 // Left Action
    IncreaseSliderDataLong,                 // Right Action
    AudioMenuGoBack,                        // Back Action
    StartMusicTest,                         // Forward Action
};

// Audio - Test SFX
MENU_ITEM MenuItem_TestSFX = {
    TEXT_SFXTEST,                           // Text label index

    MENU_DATA_WIDTH_INT,                    // Space needed to draw item data
    &SFXTestSlider,                         // Data
    DrawSliderDataLong,                     // Draw Function

 	NULL,								    // Input handler
	0,                                      // Flags

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    DecreaseSliderDataLong,                 // Left Action
    IncreaseSliderDataLong,                 // Right Action
    AudioMenuGoBack,                                // Back Action
    StartSFXTest,                           // Forward Action
};

// Audio - SFX channels
MENU_ITEM MenuItem_SfxChannels = {
    TEXT_SFX_CHANNELS,                          // Text label index

    MENU_DATA_WIDTH_INT,                    // Space needed to draw item data
    &SFXChannelsSlider,                         // Data
    DrawSliderDataLong,                     // Draw Function

 	NULL,								    // Input handler
	0,                                      // Flags

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    DecreaseSliderDataLong,                 // Left Action
    IncreaseSliderDataLong,                 // Right Action
    AudioMenuGoBack,                        // Back Action
    NULL,                                   // Forward Action
};

//Create
void CreateAudioSettingsMenu(MENU_HEADER *menuHeader, MENU *menu)
{
    SFXTestSlider.Max = SfxLoadNum - 1;

    if (GameSettings.Level == LEVEL_FRONTEND)
        AddMenuItem(menuHeader, menu, &MenuItem_MusicOn);

    AddMenuItem(menuHeader, menu, &MenuItem_MusicVolume);
    AddMenuItem(menuHeader, menu, &MenuItem_SFXVolume);

    // Switch off "Go-Forward" icon
    menuHeader->NavFlags &= ~MENU_FLAG_ADVANCE;

    if (GameSettings.Level == LEVEL_FRONTEND){
        AddMenuItem(menuHeader, menu, &MenuItem_TestSFX);
        AddMenuItem(menuHeader, menu, &MenuItem_SfxChannels);
    }

}

// Utility
void AudioMenuGoBack(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    // Stopping sfx samples
    //----------------------

    MenuGoBack(menuHeader,menu,menuItem);
}

void StartMusicTest(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
}

void StartSFXTest(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    PlaySfx(g_TitleScreenData.sfxTest, g_TitleScreenData.sfxVolume, SFX_CENTRE_PAN, SFX_SAMPLE_RATE, 0x7fffffff);
}


void SetMusicVolume(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex)
{   
    // Set the music volume
    UpdateMusicVol(g_TitleScreenData.musicVolume);

    // Draw the music volume slider
    DrawSliderDataLong(menuHeader, menu, menuItem, itemIndex);
}
void SetSFXVolume(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex)
{   
    // Set the SFX volume
    UpdateSfxVol(g_TitleScreenData.sfxVolume);

    // Draw the SFX volume slider
    DrawSliderDataLong(menuHeader, menu, menuItem, itemIndex);
}

