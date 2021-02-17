//-----------------------------------------------------------------------------
// File: ui_Options.cpp
//
// Desc: 
//
// Re-Volt (Generic) Copyright (c) Probe Entertainment 1998
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"
#include "main.h"
#include "geom.h"
#include "particle.h"
#include "model.h"
#include "aerial.h"
#include "newcoll.h"
#include "body.h"
#include "car.h"
#include "ctrlread.h"
#include "object.h"
#include "light.h"
#include "obj_init.h"
#include "player.h"
#include "ai.h"
#include "ai_init.h"
#include "EditObject.h"
#include "drawobj.h"
#include "move.h"
#include "timing.h"
#include "visibox.h"
#include "spark.h"
#include "field.h"
#include "weapon.h"
#include "input.h"
#include "initplay.h"
#include "pickup.h"
#include "SoundEffectEngine.h"
#include "ReadInit.h"
#include "MusicManager.h"
#include "Settings.h"
#include "credits.h"
#include "Gameloop.h"
#include "Panel.h"
#include "Text.h"
#include "VoiceManager.h"

#include "ui_Menu.h"
#include "ui_TitleScreen.h"
#include "ui_StateEngine.h"
#include "ui_menudraw.h"
#include "ui_MenuText.h"
#include "ui_Confirm.h"
//$REMOVED#include "ui_BestTimes.h"
//$REMOVED#include "ui_ProgressTable.h"
#include "ui_Options.h"
#include "ui_SelectRaceMode.h"
#include "net_xonline.h"

extern MENU Menu_Gallery; // This should be in gallery.h


#define MENU_INGAME_XPOS                100
#define MENU_INGAME_YPOS                150
#define MENU_OPTIONS_XPOS               100
#define MENU_OPTIONS_YPOS               150
#define MENU_SETTINGS_XPOS              100
#define MENU_SETTINGS_YPOS              150
#define MENU_RENDERSETTINGS_XPOS        100
#define MENU_RENDERSETTINGS_YPOS        150
#define MENU_VIDEOOPTIONS_XPOS          100
#define MENU_VIDEOOPTIONS_YPOS          150
#define MENU_AUDIOSETTINGS_XPOS         100
#define MENU_AUDIOSETTINGS_YPOS         150
#define MENU_CONTROLLER_XPOS            100
#define MENU_CONTROLLER_YPOS            150
#define MENU_CONTROLSELECT_XPOS         100
#define MENU_CONTROLSELECT_YPOS         150

static VOID CreateOptionsMenu( MENU_HEADER* pMenuHeader, MENU* pMenu );
static BOOL HandleOptionsMenu( MENU_HEADER* pMenuHeader, DWORD dwInput );

static VOID CreateGameSettingsMenu( MENU_HEADER* pMenuHeader, MENU* pMenu );
static BOOL HandleGameSettingsMenu( MENU_HEADER* pMenuHeader, DWORD dwInput );

static void CreateVideoSettingsMenu( MENU_HEADER* pMenuHeader, MENU* pMenu );
static void CreateRenderSettingsMenu( MENU_HEADER* pMenuHeader, MENU* pMenu );
static void CreateAudioSettingsMenu( MENU_HEADER* pMenuHeader, MENU* pMenu );
static void CreateControllerConfigMenu( MENU_HEADER* pMenuHeader, MENU* pMenu );
static void CreateControllerMenu( MENU_HEADER* pMenuHeader, MENU* pMenu );

static BOOL SelectPrevVideoMode( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem );
static BOOL SelectNextVideoMode( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem );
static BOOL DecreaseBrightness( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem );
static BOOL IncreaseBrightness( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem );
static BOOL DecreaseContrast( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem );
static BOOL IncreaseContrast( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem );
static BOOL ToggleAntiAliasOff( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem );
static BOOL ToggleAntiAliasOn( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem );
static BOOL ToggleVsyncOff( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem );
static BOOL ToggleVsyncOn( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem );
static BOOL ToggleShowFPSOff( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem );
static BOOL ToggleShowFPSOn( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem );
static BOOL ToggleWireFrameOff( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem );
static BOOL ToggleWireFrameOn( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem );
static BOOL NextTextureFilter( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem );
static BOOL PrevTextureFilter( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem );
static BOOL PrevMipMapLevel( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem );
static BOOL NextMipMapLevel( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem );
static void DrawSmokeLevel( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem, int itemIndex );
static void DrawMusicVolume( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem, int itemIndex );
static void DrawMusicOnOff( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem, int itemIndex );
static void DrawSFXVolume( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem, int itemIndex );
static void DrawVoiceMaskItem( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem, int itemIndex );
static BOOL PreviousVoiceMask( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem );
static BOOL NextVoiceMask( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem );
static BOOL MenuSetControlsDefault( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem );
static BOOL MenuControlConfigBack( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem );
static BOOL IncreaseControlMethod( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem );
static BOOL DecreaseControlMethod( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem );
static BOOL IncreaseControllerPlayerNum( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem );
static BOOL DecreaseControllerPlayerNum( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem );

extern void DrawVideoMode( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem, int itemIndex );
extern void DrawSpeedUnits( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem, int itemIndex );
extern void DrawDrawDist( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem, int itemIndex );
extern void DrawTextureFilter( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem, int itemIndex );
extern void DrawMipMapLevel( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem, int itemIndex );
extern void DrawWireFrame( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem, int itemIndex );
extern void DrawParticleLevel( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem, int itemIndex );
extern void DrawControllerConfigure( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem, int itemIndex );
extern void DrawGhostType( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem, int itemIndex );

static void DrawControllerType( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem, int itemIndex );


WCHAR* DrawDistStrings[] = 
{
    L"Lowest",
    L"Low",
    L"Medium",
    L"High",
    L"Highest",
};

WCHAR* TextureFilterStrings[] = 
{
    L"Point",
    L"Linear",
    L"Anisotropic",
};

WCHAR* ParticleStrings[] = 
{
    L"None",
    L"Some",
    L"Lots",
};

WCHAR* MipMapStrings[] = 
{
    L"None",
    L"Point",
    L"Linear",
};




////////////////////////////////////////////////////////////////
//
// Options menu
//
////////////////////////////////////////////////////////////////
extern MENU Menu_Options = 
{
    TEXT_OPTIONS,
    MENU_DEFAULT | MENU_CENTRE_X,           // Menu type
    CreateOptionsMenu,                      // Create menu function
    HandleOptionsMenu,                      // Input handler function
    NULL,                                   // Menu draw function
    MENU_OPTIONS_XPOS,                      // X coord
    MENU_OPTIONS_YPOS,                      // Y Coord
};


// Create
void CreateOptionsMenu( MENU_HEADER* pMenuHeader, MENU* pMenu )
{
    // add menu items
//$REMOVED    pMenuHeader->AddMenuItem( TEXT_BESTTRIALTIMES );
//$REMOVED    pMenuHeader->AddMenuItem( TEXT_PROGRESSTABLE );
    pMenuHeader->AddMenuItem( TEXT_GAMESETTINGS );
//$REMOVED    pMenuHeader->AddMenuItem( TEXT_VIDEOSETTINGS );
    pMenuHeader->AddMenuItem( TEXT_AUDIOSETTINGS );
    pMenuHeader->AddMenuItem( TEXT_CONTROLLERSETTINGS, MENU_ITEM_INACTIVE );
//$REMOVED    pMenuHeader->AddMenuItem( TEXT_CREDITS );
//$REMOVED    pMenuHeader->AddMenuItem( TEXT_GALLERY );
}


BOOL HandleOptionsMenu( MENU_HEADER* pMenuHeader, DWORD dwInput )
{
    switch( dwInput )
    {
        case MENU_INPUT_UP:
            return SelectPreviousMenuItem( pMenuHeader, pMenuHeader->m_pMenu, NULL );
        
        case MENU_INPUT_DOWN:
            return SelectNextMenuItem( pMenuHeader, pMenuHeader->m_pMenu, NULL );

        case MENU_INPUT_BACK:
            g_OptionsStateEngine.Return( STATEENGINE_TERMINATED );
            return TRUE;

        case MENU_INPUT_SELECT:
            switch( pMenuHeader->m_pMenuItem[pMenuHeader->m_pMenu->CurrentItemIndex]->TextIndex )
            {
//$REMOVED
//              case TEXT_BESTTRIALTIMES:
//                    g_pActiveStateEngine->Call( &g_BestTimesStateEngine );
//                    return TRUE;

//$REMOVED
//              case TEXT_PROGRESSTABLE:
//                    g_pActiveStateEngine->Call( &g_ProgressTableStateEngine );
//                    return TRUE;

                case TEXT_GAMESETTINGS:
                    pMenuHeader->SetNextMenu( &Menu_GameSettings );
                    return TRUE;
//$REMOVED
//              case TEXT_VIDEOSETTINGS:
//                    pMenuHeader->SetNextMenu( &Menu_VideoSettings );
//                    return TRUE;

                case TEXT_AUDIOSETTINGS:
                    pMenuHeader->SetNextMenu( &Menu_AudioSettings );
                    return TRUE;

                case TEXT_CONTROLLERSETTINGS:
                    pMenuHeader->SetNextMenu( &Menu_ControllerSettings );
                    return TRUE;

//$REMOVED
//                case TEXT_GALLERY:
//                    pMenuHeader->SetNextMenu( &Menu_Gallery );
//                    return TRUE;
                
//$REMOVED
//                case TEXT_CREDITS:
//                    SetRaceCredits();
//                    return TRUE;
            }
    }

    return FALSE;
}



// Utility
void SetRaceCredits()
{
    GameSettings.GameType = GAMETYPE_DEMO;
    g_bTitleScreenRunDemo = TRUE;
    InitCreditStateActive();
    DemoTimeout = 0.0f;
    g_pMenuHeader->SetNextMenu( NULL );
}





////////////////////////////////////////////////////////////////
//
// Game Settings Menu
//
////////////////////////////////////////////////////////////////
extern MENU Menu_GameSettings = 
{
    TEXT_GAMESETTINGS,
    MENU_DEFAULT | MENU_CENTRE_X,           // Menu type
    CreateGameSettingsMenu,                 // Create menu function
    HandleGameSettingsMenu,                 // Input handler function
    NULL,                                   // Menu draw function
    MENU_SETTINGS_XPOS,                     // X coord
    MENU_SETTINGS_YPOS,                     // Y Coord
};

// Data
SLIDER_DATA_LONG NumberOfCarsSlider = 
{
    &gTitleScreenVars.numberOfCars,
    2, MAX_RACE_CARS, 1,
    FALSE, TRUE,
};
SLIDER_DATA_LONG NumberOfLapsSlider = 
{
    &gTitleScreenVars.numberOfLaps,
    MIN_RACE_LAPS, MAX_RACE_LAPS, 1,
    FALSE, TRUE,
};
SLIDER_DATA_LONG PlayModeSlider = 
{
    &gTitleScreenVars.playMode,
    0, MODE_KIDS, 1,
    FALSE, TRUE,
};
SLIDER_DATA_LONG SpeedUnitsSlider = 
{
    &SpeedUnits,
    0, SPEED_NTYPES - 1, 1,
    FALSE, FALSE,
};

// Game Settings - # cars
MENU_ITEM MenuItem_NumberOfCars = 
{
    TEXT_NUMBEROFCARS,                      // Text label index
    MENU_DATA_WIDTH_INT,                    // Space needed to draw item data
    &NumberOfCarsSlider,                    // Data
    DrawSliderDataLong,                     // Draw Function
};

// Game Settings - # laps
MENU_ITEM MenuItem_NumberOfLaps = 
{
    TEXT_NUMBEROFLAPS,                      // Text label index
    MENU_DATA_WIDTH_INT,                    // Space needed to draw item data
    &NumberOfLapsSlider,                    // Data
    DrawSliderDataLong,                     // Draw Function
};



// Game Settings - Random cars
MENU_ITEM MenuItem_RandomCars = 
{
    TEXT_RANDOM_CARS,                       // Text label index
    0,                                      // Space needed to draw item data
    &gTitleScreenVars.RandomCars,           // Data
    DrawMenuDataOnOff,                      // Draw Function
};

// Game Settings - Random track
MENU_ITEM MenuItem_RandomTrack = 
{
    TEXT_RANDOM_TRACK,                      // Text label index
    0,                                      // Space needed to draw item data
    &gTitleScreenVars.RandomTrack,          // Data
    DrawMenuDataOnOff,                      // Draw Function
};

// Game Settings - pickups
MENU_ITEM MenuItem_Pickups = 
{
    TEXT_PICKUPS,                           // Text label index
    MENU_DATA_WIDTH_BOOL,                   // Space needed to draw item data
    &gTitleScreenVars.pickUps,              // Data
    DrawMenuDataOnOff,                      // Draw Function
};

// Game Settings - Speed units
MENU_ITEM MenuItem_SpeedUnits = 
{
    TEXT_UNITS,                             // Text label index
    12.0f * MENU_TEXT_WIDTH,                 // Space needed to draw item data
    &SpeedUnitsSlider,                      // Data
    DrawSpeedUnits,                         // Draw Function
};

// Game Settings - Local Ghost?
MENU_ITEM MenuItem_GhostType = 
{
    TEXT_GHOSTTYPE,                         // Text label index
    12.0f * MENU_TEXT_WIDTH,                // Space needed to draw item data
    &gTitleScreenVars.LocalGhost,           // Data
    DrawGhostType,                          // Draw Function
};

//$REMOVED
//// Game Settings - Reset Progress Table
//MENU_ITEM MenuItem_ResetProgressTable = 
//{
//    TEXT_RESETPROGRESSTABLE,                // Text label index
//    0,                                      // Space needed to draw item data
//    NULL,                                   // Data
//    NULL,                                   // Draw Function
//};

//$REMOVED
//// Game Settings - Save CarInfo (Dev only)
//MENU_ITEM MenuItem_SaveCarInfoSingle = 
//{
//    TEXT_SAVECARINFOSINGLE,                 // Text label index
//    0,                                      // Space needed to draw item data
//    NULL,                                   // Data
//    NULL,                                   // Draw Function
//};
//$END_REMOVAL

//$REMOVED
//MENU_ITEM MenuItem_SaveCarInfoMultiple = 
//{
//    TEXT_SAVECARINFOMULTIPLE,               // Text label index
//    0,                                      // Space needed to draw item data
//    NULL,                                   // Data
//    NULL,                                   // Draw Function
//};
//$END_REMOVAL

// Create Game settings menu
void CreateGameSettingsMenu( MENU_HEADER* pMenuHeader, MENU* pMenu )
{
    // Calc max data width for the menuitems
    MenuItem_RandomCars.DataWidth   = CalcMaxStringWidth( 2, &TEXT_TABLE(TEXT_ON) ) + g_pFont->GetTextWidth( L" >" );;
//$REMOVED  MenuItem_RandomTrack.DataWidth  = CalcMaxStringWidth( 2, &TEXT_TABLE(TEXT_ON) ) + g_pFont->GetTextWidth( L" >" );;
    MenuItem_Pickups.DataWidth      = CalcMaxStringWidth( 2, &TEXT_TABLE(TEXT_ON) ) + g_pFont->GetTextWidth( L" >" );;
    MenuItem_SpeedUnits.DataWidth   = CalcMaxStringWidth( SPEED_NTYPES, &TEXT_TABLE(TEXT_MPH) ) + g_pFont->GetTextWidth( L" >" );;
//$REMOVED  MenuItem_GhostType.DataWidth    = CalcMaxStringWidth( 2, &TEXT_TABLE(TEXT_LOCAL) ) + g_pFont->GetTextWidth( L" >" );;

    // add menu items
    pMenuHeader->AddMenuItem( &MenuItem_NumberOfCars );
    pMenuHeader->AddMenuItem( &MenuItem_NumberOfLaps );
    pMenuHeader->AddMenuItem( &MenuItem_RandomCars );
//$REMOVED  pMenuHeader->AddMenuItem( &MenuItem_RandomTrack );
    pMenuHeader->AddMenuItem( &MenuItem_Pickups );
    pMenuHeader->AddMenuItem( &MenuItem_SpeedUnits );
//$REMOVED  pMenuHeader->AddMenuItem( &MenuItem_GhostType );

    //clear secrets?
    if( gConfirmMenuReturnVal == CONFIRM_YES )
    {
        StarList.NumFound = 0;
        ZeroMemory(&LevelSecrets, sizeof(LevelSecrets));
        //InitDefaultLevels();
        //SetAllCarSelect();
    }

    SetConfirmMenuStrings(TEXT_TABLE(TEXT_MENU_CONFIRM_GENERIC), TEXT_TABLE(TEXT_MENU_CONFIRM_REALLY) , TEXT_TABLE(TEXT_MENU_CONFIRM_PRETEND), CONFIRM_NO);
//$REMOVED  pMenuHeader->AddMenuItem( &MenuItem_ResetProgressTable);

    // save carinfo
    if (Version == VERSION_DEV) 
    {
//$REMOVED  pMenuHeader->AddMenuItem( &MenuItem_SaveCarInfoSingle);
//          pMenuHeader->AddMenuItem( &MenuItem_SaveCarInfoMultiple);   // GAZZA
    }
}

BOOL HandleGameSettingsMenu( MENU_HEADER* pMenuHeader, DWORD dwInput )
{
    switch( pMenuHeader->m_pCurrentItem->TextIndex )
    {
        case TEXT_NUMBEROFCARS:
        case TEXT_NUMBEROFLAPS:
        case TEXT_UNITS:
            if( dwInput == MENU_INPUT_LEFT )  return DecreaseSliderDataLong( pMenuHeader, pMenuHeader->m_pMenu, pMenuHeader->m_pCurrentItem );
            if( dwInput == MENU_INPUT_RIGHT ) return IncreaseSliderDataLong( pMenuHeader, pMenuHeader->m_pMenu, pMenuHeader->m_pCurrentItem );
            break;

        case TEXT_RANDOM_CARS:
        case TEXT_RANDOM_TRACK:
        case TEXT_PICKUPS:
//$REMOVED        case TEXT_GHOSTTYPE:
            if( dwInput == MENU_INPUT_LEFT )  return ToggleMenuDataOff( pMenuHeader, pMenuHeader->m_pMenu, pMenuHeader->m_pCurrentItem );
            if( dwInput == MENU_INPUT_RIGHT ) return ToggleMenuDataOn( pMenuHeader, pMenuHeader->m_pMenu, pMenuHeader->m_pCurrentItem );
            break;

//$REMOVED
//        case TEXT_RESETPROGRESSTABLE:
//            if( dwInput == MENU_INPUT_SELECT )
//            { 
//                pMenuHeader->SetNextMenu( &Menu_ConfirmYesNo );
//                return TRUE; 
//            }
//            break;
        
//$REMOVED
//      case TEXT_SAVECARINFOSINGLE:
//          if( dwInput == MENU_INPUT_SELECT )
//          { 
//              WriteAllCarInfoSingle( CarInfo, NCarTypes, "AllCarInfo.txt" );
//              return TRUE; 
//          }
//          break;

//$REMOVED
//      case TEXT_SAVECARINFOMULTIPLE:
//          if( dwInput == MENU_INPUT_SELECT )
//          { 
//              WriteAllCarInfoMultiple( CarInfo, NCarTypes );
//              return TRUE; 
//          }
//          break;
    }
            
    switch( dwInput )
    {
        case MENU_INPUT_UP:
            return SelectPreviousMenuItem( pMenuHeader, pMenuHeader->m_pMenu, pMenuHeader->m_pCurrentItem );
        
        case MENU_INPUT_DOWN:
            return SelectNextMenuItem( pMenuHeader, pMenuHeader->m_pMenu, pMenuHeader->m_pCurrentItem );

        case MENU_INPUT_BACK:
            return MenuGoBack( pMenuHeader, pMenuHeader->m_pMenu, pMenuHeader->m_pCurrentItem );
    }

    return FALSE;
}




////////////////////////////////////////////////////////////////
//
// Video Options
//
////////////////////////////////////////////////////////////////
extern MENU Menu_VideoSettings = 
{
    TEXT_VIDEOSETTINGS,
    MENU_DEFAULT | MENU_CENTRE_X,           // Menu type
    CreateVideoSettingsMenu,                // Create menu function
    NULL,                                   // Input handler function
    NULL,                                   // Menu draw function
    MENU_VIDEOOPTIONS_XPOS,                 // X coord
    MENU_VIDEOOPTIONS_YPOS,                 // Y Coord
};

// Data
unsigned long gBrightness;
unsigned long gContrast;
SLIDER_DATA_ULONG BrightnessSlider = 
{
    &gBrightness,
    0, 100, 1,
    TRUE, TRUE,
};
SLIDER_DATA_ULONG ContrastSlider = 
{
    &gContrast,
    0, 100, 1,
    TRUE, TRUE,
};

// Video Settings - Mode
MENU_ITEM MenuItem_VideoMode = 
{
    TEXT_SCREENRES,                         // Text label index

    MENU_DATA_WIDTH_TEXT,                   // Space needed to draw item data
    NULL,                                   // Data
    DrawVideoMode,                          // Draw Function

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    SelectPrevVideoMode,                    // Left Action
    SelectNextVideoMode,                    // Right Action
    MenuGoBack,                             // Back Action
    NULL,                                   // Forward Action
};

// Video Settings - Brightness
MENU_ITEM MenuItem_Brightness = 
{
    TEXT_BRIGHTNESS,                        // Text label index

    MENU_DATA_WIDTH_INT,                    // Space needed to draw item data
    &BrightnessSlider,                      // Data
    DrawSliderDataULong,                    // Draw Function

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    DecreaseBrightness,                     // Left Action
    IncreaseBrightness,                     // Right Action
    MenuGoBack,                             // Back Action
    NULL,                                   // Forward Action
};

// Video Settings - Contrast
MENU_ITEM MenuItem_Contrast = 
{
    TEXT_CONTRAST,                          // Text label index

    MENU_DATA_WIDTH_INT,                    // Space needed to draw item data
    &ContrastSlider,                        // Data
    DrawSliderDataULong,                    // Draw Function

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    DecreaseContrast,                       // Left Action
    IncreaseContrast,                       // Right Action
    MenuGoBack,                             // Back Action
    NULL,                                   // Forward Action
};

// Video Settings - RenderSettings
MENU_ITEM MenuItem_RenderSettings = 
{
    TEXT_RENDERSETTINGS,                    // Text label index

    0,                                      // Space needed to draw item data
    &Menu_RenderSettings,                   // Data
    NULL,                                   // Draw Function

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    NULL,                                   // Left Action
    NULL,                                   // Right Action
    MenuGoBack,                             // Back Action
    MenuGoForward,                          // Forward Action
};

// Create

void CreateVideoSettingsMenu( MENU_HEADER* pMenuHeader, MENU* pMenu )
{
    MenuItem_VideoMode.DataWidth = MENU_DATA_WIDTH_SLIDER + g_pFont->GetTextWidth( L" 100%" );

    pMenuHeader->AddMenuItem( &MenuItem_VideoMode, MENU_ITEM_INACTIVE );

    if( (GammaFlag != GAMMA_UNAVAILABLE) && (GammaFlag != GAMMA_AUTO) ) 
    {
        pMenuHeader->AddMenuItem( &MenuItem_Brightness );
        pMenuHeader->AddMenuItem( &MenuItem_Contrast );
    }
    pMenuHeader->AddMenuItem( &MenuItem_RenderSettings );

    gBrightness = ((RegistrySettings.Brightness - 32) * 100) / (512 - 32);
    gContrast   = ((RegistrySettings.Contrast - 32) * 100) / (512 - 32);
}


BOOL SelectPrevVideoMode( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem )
{
    //$NOTE: dont remove this function yet; probably want to support more modes eventually!!
    RegistrySettings.ScreenWidth  = 640;
    RegistrySettings.ScreenHeight = 480;
    RegistrySettings.ScreenBpp    = 32;
    return TRUE;
}

BOOL SelectNextVideoMode( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem )
{
    //$NOTE: dont remove this function yet; probably want to support more modes eventually!!
    RegistrySettings.ScreenWidth  = 640;
    RegistrySettings.ScreenHeight = 480;
    RegistrySettings.ScreenBpp    = 32;
    return TRUE;
}

BOOL DecreaseBrightness( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem )
{
    if( TRUE == DecreaseSliderDataULong(pMenuHeader, pMenu, pMenuItem) )
    {
        RegistrySettings.Brightness = 32 + (gBrightness * (512 - 32)) / 100;
        SetGamma(RegistrySettings.Brightness, RegistrySettings.Contrast);
        return TRUE;
    }
    return FALSE;
}

BOOL IncreaseBrightness( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem )
{
    if( TRUE == IncreaseSliderDataULong(pMenuHeader, pMenu, pMenuItem) )
    {
        RegistrySettings.Brightness = 32 + (gBrightness * (512 - 32)) / 100;
        SetGamma(RegistrySettings.Brightness, RegistrySettings.Contrast);
        return TRUE;
    }
    return FALSE;
}

BOOL DecreaseContrast( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem )
{
    if( TRUE == DecreaseSliderDataULong(pMenuHeader, pMenu, pMenuItem) )
    {
        RegistrySettings.Contrast = 32 + (gContrast * (512 - 32)) / 100;
        SetGamma(RegistrySettings.Brightness, RegistrySettings.Contrast);
        return TRUE;
    }
    return FALSE;
}

BOOL IncreaseContrast( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem )
{
    if( TRUE == IncreaseSliderDataULong(pMenuHeader, pMenu, pMenuItem) )
    {
        RegistrySettings.Contrast = 32 + (gContrast * (512 - 32)) / 100;
        SetGamma(RegistrySettings.Brightness, RegistrySettings.Contrast);
        return TRUE;
    }
    return FALSE;
}


////////////////////////////////////////////////////////////////
//
// RenderSettings
//
////////////////////////////////////////////////////////////////
SLIDER_DATA_ULONG DrawDistSlider = 
{
    &RegistrySettings.DrawDist,
    0, 4, 1,
    FALSE, TRUE,
};

SLIDER_DATA_LONG ParticlesSlider = 
{
    &gTitleScreenVars.sparkLevel,
    0, 2, 1,
    FALSE, TRUE,
};

SLIDER_DATA_LONG TextureFilterSlider = 
{
    &gTitleScreenVars.textureFilter,
    0, 2, 1,
    FALSE, TRUE,
};

SLIDER_DATA_LONG MipMapSlider = 
{
    &gTitleScreenVars.mipLevel,
    0, 2, 1,
    FALSE, TRUE,
};


// Menu
extern MENU Menu_RenderSettings = 
{
    TEXT_RENDERSETTINGS,
    MENU_DEFAULT | MENU_CENTRE_X,           // Menu type
    CreateRenderSettingsMenu,               // Menu create function
    NULL,                                   // Input handler function
    NULL,                                   // Menu draw function
    MENU_RENDERSETTINGS_XPOS,               // X coord
    MENU_RENDERSETTINGS_YPOS,               // Y Coord
};

// Render Settings - Draw Dist
MENU_ITEM MenuItem_DrawDist = 
{
    TEXT_DRAWDIST,                          // Text label index

    MENU_DATA_WIDTH_BOOL,                   // Space needed to draw item data
    &DrawDistSlider,                        // Data
    DrawDrawDist,                           // Draw Function

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    DecreaseSliderDataULong,                // Left Action
    IncreaseSliderDataULong,                // Right Action
    MenuGoBack,                             // Back Action
    NULL,                                   // Forward Action
};

// Render Settings - Reflections
MENU_ITEM MenuItem_Reflections = 
{
    TEXT_REFLECTIONS,                       // Text label index

    MENU_DATA_WIDTH_BOOL,                   // Space needed to draw item data
    &gTitleScreenVars.reflections,          // Data
    DrawMenuDataOnOff,                      // Draw Function

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    ToggleMenuDataOff,                      // Left Action
    ToggleMenuDataOn,                       // Right Action
    MenuGoBack,                             // Back Action
    NULL,                                   // Forward Action
};

// Render Settings - Env Maps
MENU_ITEM MenuItem_Shinyness = 
{
    TEXT_SHINYNESS,                         // Text label index

    MENU_DATA_WIDTH_BOOL,                   // Space needed to draw item data
    &gTitleScreenVars.shinyness,            // Data
    DrawMenuDataOnOff,                      // Draw Function

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    ToggleMenuDataOff,                      // Left Action
    ToggleMenuDataOn,                       // Right Action
    MenuGoBack,                             // Back Action
    NULL,                                   // Forward Action
};

// Render Settings - Lights
MENU_ITEM MenuItem_Lights = 
{
    TEXT_LIGHTS,                            // Text label index

    MENU_DATA_WIDTH_BOOL,                   // Space needed to draw item data
    &gTitleScreenVars.lights,               // Data
    DrawMenuDataOnOff,                      // Draw Function

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    ToggleMenuDataOff,                      // Left Action
    ToggleMenuDataOn,                       // Right Action
    MenuGoBack,                             // Back Action
    NULL,                                   // Forward Action
};

// Render Settings - Instances
MENU_ITEM MenuItem_Instances = 
{
    TEXT_INSTANCES,                         // Text label index

    MENU_DATA_WIDTH_BOOL,                   // Space needed to draw item data
    &gTitleScreenVars.instances,            // Data
    DrawMenuDataOnOff,                      // Draw Function

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    ToggleMenuDataOff,                      // Left Action
    ToggleMenuDataOn,                       // Right Action
    MenuGoBack,                             // Back Action
    NULL,                                   // Forward Action
};

// Render Settings - SkidMarks
MENU_ITEM MenuItem_SkidMarks = 
{
    TEXT_SKIDMARKS,                         // Text label index

    MENU_DATA_WIDTH_BOOL,                   // Space needed to draw item data
    &gTitleScreenVars.skidmarks,            // Data
    DrawMenuDataOnOff,                      // Draw Function

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    ToggleMenuDataOff,                      // Left Action
    ToggleMenuDataOn,                       // Right Action
    MenuGoBack,                             // Back Action
    NULL,                                   // Forward Action
};

// Render Settings - Shadows
MENU_ITEM MenuItem_Shadows = 
{
    TEXT_SHADOWS,                           // Text label index

    MENU_DATA_WIDTH_BOOL,                   // Space needed to draw item data
    &gTitleScreenVars.shadows,              // Data
    DrawMenuDataOnOff,                      // Draw Function

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    ToggleMenuDataOff,                      // Left Action
    ToggleMenuDataOn,                       // Right Action
    MenuGoBack,                             // Back Action
    NULL,                                   // Forward Action
};

// Render Settings - Smoke
MENU_ITEM MenuItem_Smoke = 
{
    TEXT_SMOKE,                         // Text label index

    MENU_DATA_WIDTH_BOOL,                   // Space needed to draw item data
    &gTitleScreenVars.smoke,                // Data
    DrawSmokeLevel,                         // Draw Function

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    ToggleMenuDataOff,                      // Left Action
    ToggleMenuDataOn,                       // Right Action
    MenuGoBack,                             // Back Action
    NULL,                                   // Forward Action
};


// Render Settings - Particles
MENU_ITEM MenuItem_Particles = 
{
    TEXT_PARTICLES,                         // Text label index

    MENU_TEXT_WIDTH * 13,                   // Space needed to draw item data
    &ParticlesSlider,                       // Data
    DrawParticleLevel,                      // Draw Function

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    DecreaseSliderDataLong,                 // Left Action
    IncreaseSliderDataLong,                 // Right Action
    MenuGoBack,                             // Back Action
    NULL,                                   // Forward Action
};


// RenderSettings - AntiAlias
MENU_ITEM MenuItem_AntiAlias = 
{
    TEXT_ANTIALIAS,                         // Text label index

    0,                                      // Space needed to draw item data
    &gTitleScreenVars.antialias,            // Data
    DrawMenuDataOnOff,                      // Draw Function

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    ToggleAntiAliasOff,                     // Left Action
    ToggleAntiAliasOn,                      // Right Action
    MenuGoBack,                             // Back Action
    NULL,                                   // Forward Action
};

// RenderSettings - Texture Filter
MENU_ITEM MenuItem_TextureFilter = 
{
    TEXT_TEXTUREFILTER,                     // Text label index

    0,                                      // Space needed to draw item data
    &TextureFilterSlider,                   // Data
    DrawTextureFilter,                      // Draw Function

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    PrevTextureFilter,                      // Left Action
    NextTextureFilter,                      // Right Action
    MenuGoBack,                             // Back Action
    NULL,                                   // Forward Action
};

// RenderSettings - Mip map
MENU_ITEM MenuItem_MipMap = 
{
    TEXT_MIPMAP,                            // Text label index

    0,                                      // Space needed to draw item data
    &MipMapSlider,                          // Data
    DrawMipMapLevel,                        // Draw Function

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    PrevMipMapLevel,                        // Left Action
    NextMipMapLevel,                        // Right Action
    MenuGoBack,                             // Back Action
    NULL,                                   // Forward Action
};

// RenderSettings - Vsync
MENU_ITEM MenuItem_Vsync = 
{
    TEXT_VSYNC,                             // Text label index

    0,                                      // Space needed to draw item data
    &RegistrySettings.Vsync,                // Data
    DrawMenuDataOnOff,                      // Draw Function

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    ToggleVsyncOff,                         // Left Action
    ToggleVsyncOn,                          // Right Action
    MenuGoBack,                             // Back Action
    NULL,                                   // Forward Action
};

// RenderSettings - ShowFPS
MENU_ITEM MenuItem_ShowFPS = 
{
    TEXT_SHOW_FPS,                          // Text label index

    0,                                      // Space needed to draw item data
    &RegistrySettings.ShowFPS,              // Data
    DrawMenuDataOnOff,                      // Draw Function

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    ToggleShowFPSOff,                       // Left Action
    ToggleShowFPSOn,                        // Right Action
    MenuGoBack,                             // Back Action
    NULL,                                   // Forward Action
};

// In Game Graphics - Wireframe
MENU_ITEM MenuItem_WireFrame = 
{
    TEXT_WIREFRAME,                         // Text label index

    0,                                      // Space needed to draw item data
    NULL,                                   // Data
    DrawWireFrame,                          // Draw Function

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    ToggleWireFrameOff,                     // Left Action
    ToggleWireFrameOn,                      // Right Action
    MenuGoBack,                             // Back Action
    NULL,                                   // Forward Action
};

// Create
void CreateRenderSettingsMenu( MENU_HEADER* pMenuHeader, MENU* pMenu )
{
    //$REVISIT: should we allow FPS counter (and similar options) in shipping versions?  Probably not.

    MenuItem_ShowFPS.DataWidth       = CalcMaxStringWidth( 2, &gTitleScreen_Text[TEXT_ON] );
    MenuItem_Vsync.DataWidth         = CalcMaxStringWidth( 2, &gTitleScreen_Text[TEXT_ON] );
    MenuItem_TextureFilter.DataWidth = CalcMaxStringWidth( 3, &TextureFilterStrings[0] );
    MenuItem_MipMap.DataWidth        = CalcMaxStringWidth( 3, &MipMapStrings[0] );
    MenuItem_AntiAlias.DataWidth     = CalcMaxStringWidth( 2, &gTitleScreen_Text[TEXT_ON] );
    MenuItem_DrawDist.DataWidth      = CalcMaxStringWidth( 5, &DrawDistStrings[0] );
    MenuItem_Reflections.DataWidth   = CalcMaxStringWidth( 2, &gTitleScreen_Text[TEXT_ON] );
    MenuItem_SkidMarks.DataWidth     = CalcMaxStringWidth( 2, &gTitleScreen_Text[TEXT_ON] );
    MenuItem_Particles.DataWidth     = CalcMaxStringWidth( 3, &ParticleStrings[0] );
    MenuItem_Shadows.DataWidth       = CalcMaxStringWidth( 2, &gTitleScreen_Text[TEXT_ON] );
    MenuItem_Shinyness.DataWidth     = CalcMaxStringWidth( 2, &gTitleScreen_Text[TEXT_ON] );
    MenuItem_Lights.DataWidth        = CalcMaxStringWidth( 2, &gTitleScreen_Text[TEXT_ON] );
    MenuItem_Instances.DataWidth     = CalcMaxStringWidth( 2, &gTitleScreen_Text[TEXT_ON] );

    pMenuHeader->AddMenuItem( &MenuItem_ShowFPS );
    pMenuHeader->AddMenuItem( &MenuItem_Vsync );
    pMenuHeader->AddMenuItem( &MenuItem_TextureFilter );
    pMenuHeader->AddMenuItem( &MenuItem_MipMap );
    pMenuHeader->AddMenuItem( &MenuItem_AntiAlias, MENU_ITEM_INACTIVE );
    pMenuHeader->AddMenuItem( &MenuItem_DrawDist );
    pMenuHeader->AddMenuItem( &MenuItem_Reflections );
    pMenuHeader->AddMenuItem( &MenuItem_SkidMarks );
    pMenuHeader->AddMenuItem( &MenuItem_Particles );
    pMenuHeader->AddMenuItem( &MenuItem_Shadows );
    pMenuHeader->AddMenuItem( &MenuItem_Shinyness );
    pMenuHeader->AddMenuItem( &MenuItem_Lights );
    pMenuHeader->AddMenuItem( &MenuItem_Instances );
}

// Utility
BOOL ToggleAntiAliasOff( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem )
{
#ifndef XBOX_NOT_YET_IMPLEMENTED
    gTitleScreenVars.antialias = FALSE;
    DxState.AntiAlias = D3DANTIALIAS_NONE;
    ANTIALIAS_ON();
    return TRUE;
#endif // ! XBOX_NOT_YET_IMPLEMENTED
    return FALSE;
}

BOOL ToggleAntiAliasOn( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem )
{
#ifndef XBOX_NOT_YET_IMPLEMENTED
    gTitleScreenVars.antialias = TRUE;
    DxState.AntiAlias = D3DANTIALIAS_SORTINDEPENDENT;
    ANTIALIAS_ON();
    return TRUE;
#endif // ! XBOX_NOT_YET_IMPLEMENTED
    return FALSE;
}

BOOL ToggleVsyncOff( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem )
{
    if( FALSE != RegistrySettings.Vsync )
    {
        RegistrySettings.Vsync = FALSE;
        return TRUE;
    }
    return FALSE;
}

BOOL ToggleVsyncOn( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem )
{
    if( TRUE != RegistrySettings.Vsync )
    {
        RegistrySettings.Vsync = TRUE;
        return TRUE;
    }
    return FALSE;
}

BOOL ToggleShowFPSOff( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem )
{
    if( FALSE != RegistrySettings.ShowFPS )
    {
        RegistrySettings.ShowFPS = FALSE;
        return TRUE;
    }
    return FALSE;
}

BOOL ToggleShowFPSOn( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem )
{
    if( TRUE != RegistrySettings.ShowFPS )
    {
        RegistrySettings.ShowFPS = TRUE;
        return TRUE;
    }
    return FALSE;
}

BOOL NextTextureFilter( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem )
{
    if( TRUE == IncreaseSliderDataLong(pMenuHeader, pMenu, pMenuItem) )
    {
        DxState.TextureFilter = gTitleScreenVars.textureFilter;
        TEXTUREFILTER_ON();
        return TRUE;
    }
    return FALSE;
}

BOOL PrevTextureFilter( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem )
{
    if( TRUE == DecreaseSliderDataLong(pMenuHeader, pMenu, pMenuItem) )
    {
        DxState.TextureFilter = gTitleScreenVars.textureFilter;
        TEXTUREFILTER_ON();
        return TRUE;
    }
    return FALSE;
}

BOOL NextMipMapLevel( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem )
{
    if( TRUE == IncreaseSliderDataLong(pMenuHeader, pMenu, pMenuItem) )
    {
        DxState.MipMap = gTitleScreenVars.mipLevel;
        MIPMAP_ON();
        return TRUE;
    }
    return FALSE;
}

BOOL PrevMipMapLevel( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem )
{
    if( TRUE == DecreaseSliderDataLong(pMenuHeader, pMenu, pMenuItem) )
    {
        DxState.MipMap = gTitleScreenVars.mipLevel;
        MIPMAP_ON();
        return TRUE;
    }
    return FALSE;
}

BOOL ToggleWireFrameOff( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem )
{
    if( FALSE != Wireframe )
    {
//      DxState.Wireframe = D3DFILL_SOLID;
        Wireframe = FALSE;
        return TRUE;
    }
    return FALSE;
}

BOOL ToggleWireFrameOn( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem )
{
    if( TRUE != Wireframe )
    {
//      DxState.Wireframe = D3DFILL_WIREFRAME;
        Wireframe = TRUE;
        return TRUE;
    }
    return FALSE;
}

// Utility
void DrawSmokeLevel( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem, int itemIndex )
{
    if (gTitleScreenVars.smoke)
        gSmokeDensity = ONE;
    else 
        gSmokeDensity = ZERO;

    DrawMenuDataOnOff(pMenuHeader, pMenu, pMenuItem, itemIndex);
}




////////////////////////////////////////////////////////////////
//
// Audio Settings
//
////////////////////////////////////////////////////////////////
extern MENU Menu_AudioSettings = 
{
    TEXT_AUDIOSETTINGS,
    MENU_DEFAULT | MENU_CENTRE_X,           // Menu type
    CreateAudioSettingsMenu,                // Create menu function
    NULL,                                   // Input handler function
    NULL,                                   // Menu draw function
    MENU_AUDIOSETTINGS_XPOS,                // X coord
    MENU_AUDIOSETTINGS_YPOS,                // Y Coord
};

// Data
SLIDER_DATA_LONG MusicVolumeSlider = 
{
    &gTitleScreenVars.musicVolume,
    0, 100, 10,
    TRUE, TRUE,
};
SLIDER_DATA_LONG SFXVolumeSlider = 
{
    &gTitleScreenVars.sfxVolume,
    0, 100, 10,
    TRUE, TRUE,
};

SLIDER_DATA_LONG SFXChannelsSlider = 
{
    (long*)&RegistrySettings.SfxChannels,
    1, SFX_MAX_SAMPLES, 1,
    FALSE, TRUE,
};

BOOL AudioMenuGoBack( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem );

// Audio - Music on / off
MENU_ITEM MenuItem_MusicOn = 
{
    TEXT_MUSIC_ON,                          // Text label index

    0,                                      // Space needed to draw item data
    &RegistrySettings.MusicOn,              // Data
    DrawMusicOnOff,                         // Draw Function

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    ToggleMenuDataOff,                      // Left Action
    ToggleMenuDataOn,                       // Right Action
    AudioMenuGoBack,                        // Back Action
    NULL,                                   // Forward Action
};

// Audio - Music Vol
MENU_ITEM MenuItem_MusicVolume = 
{
    TEXT_MUSICVOLUME,                       // Text label index

    MENU_DATA_WIDTH_SLIDER + 4 * MENU_TEXT_WIDTH,   // Space needed to draw item data
    &MusicVolumeSlider,                     // Data
    DrawMusicVolume,                        // Draw Function

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    DecreaseSliderDataLong,                 // Left Action
    IncreaseSliderDataLong,                 // Right Action
    AudioMenuGoBack,                        // Back Action
    NULL,                                   // Forward Action
};

// Audio - SFX Vol
MENU_ITEM MenuItem_SFXVolume = 
{
    TEXT_SFXVOLUME,                         // Text label index

    MENU_DATA_WIDTH_SLIDER + 4 * MENU_TEXT_WIDTH,   // Space needed to draw item data
    &SFXVolumeSlider,                       // Data
    DrawSFXVolume,                          // Draw Function

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    DecreaseSliderDataLong,                 // Left Action
    IncreaseSliderDataLong,                 // Right Action
    AudioMenuGoBack,                        // Back Action
    NULL,                                   // Forward Action
};

// Audio - SFX channels
MENU_ITEM MenuItem_SfxChannels = 
{
    TEXT_SFX_CHANNELS,                          // Text label index

    MENU_DATA_WIDTH_INT,                    // Space needed to draw item data
    &SFXChannelsSlider,                         // Data
    DrawSliderDataLong,                     // Draw Function

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    DecreaseSliderDataLong,                 // Left Action
    IncreaseSliderDataLong,                 // Right Action
    AudioMenuGoBack,                        // Back Action
    NULL,                                   // Forward Action
};

VOICE_MASK_PRESET g_VoiceMaskPresets[] =
{
    { TEXT_VOICEMASK_NONE,       XVOICE_MASK_NONE },
    { TEXT_VOICEMASK_DARKMASTER, XVOICE_MASK_DARKMASTER },
    { TEXT_VOICEMASK_CARTOON,    XVOICE_MASK_CARTOON },
    { TEXT_VOICEMASK_BIGGUY,     XVOICE_MASK_BIGGUY },
    { TEXT_VOICEMASK_CHILD,      XVOICE_MASK_CHILD },
    { TEXT_VOICEMASK_ROBOT,      XVOICE_MASK_ROBOT },
    { TEXT_VOICEMASK_WHISPER,    XVOICE_MASK_WHISPER },
};
const DWORD g_dwNumVoiceMaskPresets = sizeof( g_VoiceMaskPresets ) / sizeof( g_VoiceMaskPresets[0] );

MENU_ITEM MenuItem_VoiceMask =
{
    TEXT_VOICEMASK,                         // This should be "Voice Mask"
    200.0f,
    &RegistrySettings.VoiceMaskPreset,      // Preset value
    DrawVoiceMaskItem,                      // Custom draw
    SelectPreviousMenuItem,                 // Up
    SelectNextMenuItem,                     // Down
    PreviousVoiceMask,                      // Left
    NextVoiceMask,                          // Right
    AudioMenuGoBack,                        // Back
    NULL,                                   // Forward    
};


//Create
void CreateAudioSettingsMenu( MENU_HEADER* pMenuHeader, MENU* pMenu )
{
    MenuItem_MusicOn.DataWidth     = CalcMaxStringWidth( 2, &gTitleScreen_Text[TEXT_ON] ) + g_pFont->GetTextWidth( L" >" );
    MenuItem_MusicVolume.DataWidth = MENU_DATA_WIDTH_SLIDER + g_pFont->GetTextWidth( L" 100% >" );
    MenuItem_SFXVolume.DataWidth   = MENU_DATA_WIDTH_SLIDER + g_pFont->GetTextWidth( L" 100% >" );
    MenuItem_VoiceMask.DataWidth   = CalcMaxStringWidth( 7, &gTitleScreen_Text[TEXT_VOICEMASK_NONE] ) + g_pFont->GetTextWidth( L" >" );

    if( GameSettings.Level == LEVEL_FRONTEND )
        pMenuHeader->AddMenuItem( &MenuItem_MusicOn );
    pMenuHeader->AddMenuItem( &MenuItem_MusicVolume );
    pMenuHeader->AddMenuItem( &MenuItem_SFXVolume );
    pMenuHeader->AddMenuItem( &MenuItem_VoiceMask );

    pMenu->CurrentItemIndex = 0;

    for( DWORD i = 0; i < XGetPortCount(); i++ )
    {
        if( g_VoiceManager.IsCommunicatorInserted( i ) )
        {
            g_VoiceManager.SetLoopback( i, TRUE );
        }
    }
//$REMOVED    if( GameSettings.Level == LEVEL_FRONTEND )
//$REMOVED        pMenuHeader->AddMenuItem( &MenuItem_SfxChannels );
}

// Utility
BOOL AudioMenuGoBack( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem )
{
    // Stopping sfx samples
    //----------------------
// NOTE (JHarding): Not supporting sound effect test

    for( DWORD i = 0; i < XGetPortCount(); i++ )
    {
        if( g_VoiceManager.IsCommunicatorInserted( i ) )
        {
            g_VoiceManager.SetLoopback( i, FALSE );
        }
    }

    MenuGoBack( pMenuHeader, pMenu, pMenuItem );
    return TRUE;
}

void DrawMusicOnOff( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem, int itemIndex )
{   
    //$REVISIT - This is "draw" code, so values should be "set" somewhere else

    // Make sure MusicManager state is in sync w/ registry setting
    if( !RegistrySettings.MusicOn && g_MusicManager.GetStatus() == MM_PLAYING )
    {
        g_MusicManager.Pause();
    }
    else if( RegistrySettings.MusicOn && g_MusicManager.GetStatus() != MM_PLAYING )
    {
        g_MusicManager.Play();
    }

    // Draw the music volume slider
    DrawMenuDataOnOff(pMenuHeader, pMenu, pMenuItem, itemIndex);
}

void DrawMusicVolume( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem, int itemIndex )
{   
    //$REVISIT - This is "draw" code, so values should be "set" somewhere else

    // Set the music volume
#ifdef OLD_AUDIO
    UpdateMusicVol(gTitleScreenVars.musicVolume);
#else // !OLD_AUDIO
    g_MusicManager.SetVolume( gTitleScreenVars.musicVolume );
#endif

    // Draw the music volume slider
    DrawSliderDataLong(pMenuHeader, pMenu, pMenuItem, itemIndex);
}

void DrawSFXVolume( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem, int itemIndex )
{   
    //$REVISIT - This is "draw" code, so values should be "set" somewhere else

    // Set the SFX volume
#ifdef OLD_AUDIO
    UpdateSfxVol(gTitleScreenVars.sfxVolume);
#else // !OLD_AUDIO
    g_SoundEngine.SetVolume( gTitleScreenVars.sfxVolume );
#endif // OLD_AUDIO

    // Draw the SFX volume slider
    DrawSliderDataLong(pMenuHeader, pMenu, pMenuItem, itemIndex);
}


void DrawVoiceMaskItem( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem, int itemIndex )
{
    FLOAT xPos = pMenuHeader->m_XPos + pMenuHeader->m_ItemTextWidth + MENU_TEXT_GAP;
    FLOAT yPos = pMenuHeader->m_YPos + (MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP) * itemIndex;

    DrawMenuTextWithArrows( (pMenuItem == pMenuHeader->m_pCurrentItem) ? TRUE : FALSE,
                            xPos, yPos, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(g_VoiceMaskPresets[*(DWORD*)pMenuItem->Data].LabelTextID), 
                            pMenuHeader->m_ItemDataWidth );

}

BOOL PreviousVoiceMask( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem )
{
    DWORD* pPreset = (DWORD *)pMenuItem->Data;
    if( *pPreset > 0 )
        *pPreset -= 1;
    else
		return FALSE;

    for( DWORD i = 0; i < XGetPortCount(); i++ )
    {
        if( g_VoiceManager.IsCommunicatorInserted( i ) )
        {
            g_VoiceManager.SetVoiceMask( g_dwSignedInController, g_VoiceMaskPresets[ *pPreset ].mask );
        }
    }

    return TRUE;
}

BOOL NextVoiceMask( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem )
{
    DWORD *pPreset = (DWORD *)pMenuItem->Data;

    if( *pPreset < g_dwNumVoiceMaskPresets - 1 )
        *pPreset += 1;
    else
		return FALSE;

    for( DWORD i = 0; i < XGetPortCount(); i++ )
    {
        if( g_VoiceManager.IsCommunicatorInserted( i ) )
        {
            g_VoiceManager.SetVoiceMask( g_dwSignedInController, g_VoiceMaskPresets[ *pPreset ].mask );
        }
    }

    return TRUE;
}


////////////////////////////////////////////////////////////////
//
// Controller config
//
////////////////////////////////////////////////////////////////
KEY LastConfigKey;
long ControllerConfigPick;

// Menu
extern MENU Menu_ControllerConfigure = 
{
    TEXT_CONTROLLERCONFIG,
    MENU_DEFAULT,                           // Menu type
    CreateControllerConfigMenu,             // Create menu function
    NULL,                                   // Input handler function
    NULL,                                   // Menu draw function
    MENU_CONTROLLER_XPOS,                   // X coord
    MENU_CONTROLLER_YPOS,                   // Y Coord
};

// Controller - dummy menu entry
MENU_ITEM MenuItem_ControllerDummy = 
{
    TEXT_NONE,                              // Text label index
    0,                                      // Space needed to draw item data
    NULL,                                   // Data
    DrawControllerConfigure,                // Draw Function

    NULL,                                   // Up Action
    NULL,                                   // Down Action
    NULL,                                   // Left Action
    NULL,                                   // Right Action
    NULL,                                   // Back Action
    NULL,                                   // Forward Action
};

// Create
void CreateControllerConfigMenu( MENU_HEADER* pMenuHeader, MENU* pMenu )
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

    pMenuHeader->AddMenuItem( &MenuItem_ControllerDummy);
}




////////////////////////////////////////////////////////////////
//
// Controller select
//
////////////////////////////////////////////////////////////////

// Data
SLIDER_DATA_LONG ControllerSlider = 
{
    &RegistrySettings.Joystick,
    -1, -1, 1,
    FALSE, TRUE,
};

SLIDER_DATA_LONG SteeringDeadzoneSlider = 
{
    &RegistrySettings.SteeringDeadzone,
    0, 100, 1,
    TRUE, TRUE,
};

SLIDER_DATA_LONG SteeringRangeSlider = 
{
    &RegistrySettings.SteeringRange,
    0, 100, 1,
    TRUE, TRUE,
};
static long gJoyTypeIn = -1;                // Joystick type chosen on entering config screen

// Menu
extern MENU Menu_ControllerSettings = 
{
    TEXT_CONTROLLERSETTINGS,
    MENU_DEFAULT,                           // Menu type
    CreateControllerMenu,                   // Create menu function
    NULL,                                   // Input handler function
    NULL,                                   // Menu draw function
    MENU_CONTROLSELECT_XPOS,                // X coord
    MENU_CONTROLSELECT_YPOS,                // Y Coord
};

// Controller - select
MENU_ITEM MenuItem_ControllerSelect =
{
    TEXT_CONTROLLERSELECT,                  // Text label index

    MENU_DATA_WIDTH_TEXT,                   // Space needed to draw item data
    &ControllerSlider,                      // Data
    DrawControllerType,                     // Draw Function

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    DecreaseSliderDataLong,                 // Left Action
    IncreaseSliderDataLong,                 // Right Action
    MenuControlConfigBack,                              // Back Action
    NULL,                                   // Forward Action
};

// Controller - config
MENU_ITEM MenuItem_ControllerSetup = 
{
    TEXT_CONTROLLERCONFIG,                  // Text label index

    0,                                      // Space needed to draw item data
    &Menu_ControllerConfigure,              // Data
    NULL,                                   // Draw Function

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    NULL,                                   // Left Action
    NULL,                                   // Right Action
    MenuControlConfigBack,                              // Back Action
    MenuGoForward,                          // Forward Action
};

// Controller - default controls
MENU_ITEM MenuItem_ControllerDefault = 
{
    TEXT_CONTROLLERDEFAULT,                 // Text label index

    0,                                      // Space needed to draw item data
    NULL,                                   // Data
    NULL,                                   // Draw Function

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    NULL,                                   // Left Action
    NULL,                                   // Right Action
    MenuControlConfigBack,                              // Back Action
    MenuSetControlsDefault,                 // Forward Action
};

// Controller - steering deadzone
MENU_ITEM MenuItem_SteeringDeadZone = 
{
    TEXT_STEERING_DEADZONE,                 // Text label index

    MENU_DATA_WIDTH_SLIDER + 4 * MENU_TEXT_WIDTH,   // Space needed to draw item data
    &SteeringDeadzoneSlider,                // Data
    DrawSliderDataLong,                         // Draw Function

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    DecreaseSliderDataLong,                 // Left Action
    IncreaseSliderDataLong,                 // Right Action
    MenuControlConfigBack,                              // Back Action
    NULL,                                   // Forward Action
};

// Controller - steering range
MENU_ITEM MenuItem_SteeringRange = 
{
    TEXT_STEERING_RANGE,                    // Text label index

    MENU_DATA_WIDTH_SLIDER + 4 * MENU_TEXT_WIDTH,   // Space needed to draw item data
    &SteeringRangeSlider,                   // Data
    DrawSliderDataLong,                         // Draw Function

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    DecreaseSliderDataLong,                 // Left Action
    IncreaseSliderDataLong,                 // Right Action
    MenuControlConfigBack,                              // Back Action
    NULL,                                   // Forward Action
};

// Controller - Non linear steering
MENU_ITEM MenuItem_NonLinearSteer = 
{
    TEXT_STEERING,                          // Text label index

    MENU_DATA_WIDTH_BOOL,                   // Space needed to draw item data
    &NonLinearSteering,                     // Data
    DrawMenuDataOnOff,                      // Draw Function

    SelectPreviousMenuItem,                 // Up Action
    SelectNextMenuItem,                     // Down Action
    ToggleMenuDataOff,                      // Left Action
    ToggleMenuDataOn,                       // Right Action
    MenuControlConfigBack,                              // Back Action
    NULL,                                   // Forward Action
};

// Create
void CreateControllerMenu( MENU_HEADER* pMenuHeader, MENU* pMenu )
{
    ControllerSlider.Max = JoystickNum - 1;
    gJoyTypeIn = RegistrySettings.Joystick;
    
    pMenuHeader->AddMenuItem( &MenuItem_ControllerSelect);
    pMenuHeader->AddMenuItem( &MenuItem_ControllerSetup);
    pMenuHeader->AddMenuItem( &MenuItem_ControllerDefault);
    pMenuHeader->AddMenuItem( &MenuItem_SteeringDeadZone);
    pMenuHeader->AddMenuItem( &MenuItem_SteeringRange);
    pMenuHeader->AddMenuItem( &MenuItem_NonLinearSteer);
}

// Utility
BOOL MenuControlConfigBack( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem )
{
    if (gJoyTypeIn != RegistrySettings.Joystick) 
    {
        MenuSetControlsDefault(pMenuHeader, pMenu, pMenuItem);
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

    MenuGoBack(pMenuHeader, pMenu, pMenuItem);
    return TRUE;
}


BOOL MenuSetControlsDefault( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem )
{
    CRD_SetDefaultControls();
#ifdef OLD_AUDIO
    PlaySfx(gMenuInputSFXIndex[MENU_INPUT_SELECT], SFX_MAX_VOL, SFX_CENTRE_PAN, 44100, 0x7fffffff);
#else
    g_SoundEngine.Play2DSound( EFFECT_MenuNext, FALSE );
#endif // OLD_AUDIO

    //SetConsoleMessage( L"Controls set to default", NULL, MENU_COLOR_WHITE, MENU_COLOR_WHITE, 10, CONSOLE_MESSAGE_DEFAULT_TIME );
    return TRUE;
}




////////////////////////////////////////////////////////////////
//
// Draw video mode
//
////////////////////////////////////////////////////////////////
void DrawVideoMode( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem, int itemIndex )
{
    FLOAT xPos = pMenuHeader->m_XPos + pMenuHeader->m_ItemTextWidth + MENU_TEXT_GAP;
    FLOAT yPos = pMenuHeader->m_YPos + (MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP) * itemIndex;

    DrawMenuTextWithArrows( (pMenuItem == pMenuHeader->m_pCurrentItem) ? TRUE : FALSE, 
                        xPos, yPos, MENU_TEXT_RGB_NORMAL, L"640x480", 
                        pMenuHeader->m_ItemDataWidth );
}




////////////////////////////////////////////////////////////////
//
// Draw Speed Units
//
////////////////////////////////////////////////////////////////
void DrawSpeedUnits( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem, int itemIndex )
{
    FLOAT xPos = pMenuHeader->m_XPos + pMenuHeader->m_ItemTextWidth + MENU_TEXT_GAP;
    FLOAT yPos = pMenuHeader->m_YPos + (MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP) * itemIndex;

    DrawMenuTextWithArrows( (pMenuItem == pMenuHeader->m_pCurrentItem) ? TRUE : FALSE, 
                        xPos, yPos, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_MPH + SpeedUnits), 
                        pMenuHeader->m_ItemDataWidth );
}




////////////////////////////////////////////////////////////////
//
// Draw Draw Dist
//
////////////////////////////////////////////////////////////////
void DrawDrawDist( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem, int itemIndex )
{
    FLOAT xPos = pMenuHeader->m_XPos + pMenuHeader->m_ItemTextWidth + MENU_TEXT_GAP;
    FLOAT yPos = pMenuHeader->m_YPos + (MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP) * itemIndex;

    DrawMenuTextWithArrows( (pMenuItem == pMenuHeader->m_pCurrentItem) ? TRUE : FALSE, 
                            xPos, yPos, MENU_TEXT_RGB_NORMAL, DrawDistStrings[RegistrySettings.DrawDist], 
                            pMenuHeader->m_ItemDataWidth );
}




////////////////////////////////////////////////////////////////
//
// Draw Texture filter
//
////////////////////////////////////////////////////////////////
void DrawTextureFilter( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem, int itemIndex )
{
    long col;

    FLOAT xPos = pMenuHeader->m_XPos + pMenuHeader->m_ItemTextWidth + MENU_TEXT_GAP;
    FLOAT yPos = pMenuHeader->m_YPos + (MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP) * itemIndex;

    if( DxState.TextureFilterFlag & (1 << DxState.TextureFilter) )
        col = MENU_TEXT_RGB_NORMAL;
    else
        col = MENU_TEXT_RGB_LOLITE;

    DrawMenuTextWithArrows( (pMenuItem == pMenuHeader->m_pCurrentItem) ? TRUE : FALSE, 
                            xPos, yPos, col, TextureFilterStrings[gTitleScreenVars.textureFilter], 
                            pMenuHeader->m_ItemDataWidth );
}




////////////////////////////////////////////////////////////////
//
// Draw MipMap Level
//
////////////////////////////////////////////////////////////////
void DrawMipMapLevel( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem, int itemIndex )
{
    long col;

    FLOAT xPos = pMenuHeader->m_XPos + pMenuHeader->m_ItemTextWidth + MENU_TEXT_GAP;
    FLOAT yPos = pMenuHeader->m_YPos + (MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP) * itemIndex;

    if (DxState.MipMapFlag & (1 << DxState.MipMap))
        col = MENU_TEXT_RGB_NORMAL;
    else
        col = MENU_TEXT_RGB_LOLITE;

    DrawMenuTextWithArrows( (pMenuItem == pMenuHeader->m_pCurrentItem) ? TRUE : FALSE, 
                            xPos, yPos, col, MipMapStrings[gTitleScreenVars.mipLevel], 
                            pMenuHeader->m_ItemDataWidth );
}




////////////////////////////////////////////////////////////////
//
// Draw WireFrame
//
////////////////////////////////////////////////////////////////
void DrawWireFrame( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem, int itemIndex )
{
    FLOAT xPos = pMenuHeader->m_XPos + pMenuHeader->m_ItemTextWidth + MENU_TEXT_GAP;
    FLOAT yPos = pMenuHeader->m_YPos + (MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP) * itemIndex;

    DrawMenuTextWithArrows( (pMenuItem == pMenuHeader->m_pCurrentItem) ? TRUE : FALSE, 
                            xPos, yPos, MENU_TEXT_RGB_NORMAL, Wireframe ? TEXT_TABLE(TEXT_ON): TEXT_TABLE(TEXT_OFF), 
                            pMenuHeader->m_ItemDataWidth );
}




////////////////////////////////////////////////////////////////
//
// Draw Particle Level
//
////////////////////////////////////////////////////////////////
void DrawParticleLevel( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem, int itemIndex )
{
    gSparkDensity = gTitleScreenVars.sparkLevel * HALF;

    FLOAT xPos = pMenuHeader->m_XPos + pMenuHeader->m_ItemTextWidth + MENU_TEXT_GAP;
    FLOAT yPos = pMenuHeader->m_YPos + (MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP) * itemIndex;

    DrawMenuTextWithArrows( (pMenuItem == pMenuHeader->m_pCurrentItem) ? TRUE : FALSE, 
                            xPos, yPos, MENU_TEXT_RGB_NORMAL, ParticleStrings[gTitleScreenVars.sparkLevel], 
                            pMenuHeader->m_ItemDataWidth );
}





////////////////////////////////////////////////////////////////
//
// DrawControllerConfig
//
////////////////////////////////////////////////////////////////
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

void DrawControllerConfigure( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem, int itemIndex )
{
    long i;
    short x, y;
    KEY key;
    WCHAR *text;

// display key choices

    x = (short)pMenuHeader->m_XPos;
    y = (short)pMenuHeader->m_YPos;

    DrawSpruBox(
        gMenuWidthScale * (x - MENU_TEXT_GAP), 
        gMenuHeightScale * (y - MENU_TEXT_GAP),
        gMenuWidthScale * (MENU_TEXT_WIDTH * 30 + MENU_TEXT_GAP * 2),
        gMenuHeightScale * ((MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP) * 8 + MENU_TEXT_GAP * 2), 
        SPRU_COL_RANDOM, 0);   

    BeginTextState();

    for (i = 0 ; i < ControllerConfigPick + 1; i++, y += MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP)
    {
        if (ConfigAllowed[i] == -1)
        {
            MenuGoBack(pMenuHeader, pMenu, pMenuItem);
            return;
        }

        swprintf(MenuBuffer, L"%s:", TEXT_TABLE(ConfigText[i]));
        DumpText(x, y, MENU_TEXT_WIDTH, MENU_TEXT_HEIGHT, MENU_TEXT_RGB_NORMAL, MenuBuffer);
        if (i != ControllerConfigPick)
        {
            switch (KeyTable[ConfigAllowed[i]].Type)
            {
                case KEY_TYPE_KEYBOARD:
//$MODIFIED
//                    text = GetStringFromDik(KeyTable[ConfigAllowed[i]].Index);
                    assert(0);  // should never get to here
//$END_MODIFICATIONS
                    break;

                case KEY_TYPE_BUTTON:
//$MODIFIED
//                    text = Joystick[RegistrySettings.Joystick].ButtonName[KeyTable[ConfigAllowed[i]].Index];
                    text = L"<button name here>";
//$END_MODIFICATIONS
                    break;

                case KEY_TYPE_AXISNEG:
//$MODIFIED
//                    sprintf(MenuBuffer, "-%s", Joystick[RegistrySettings.Joystick].AxisName[KeyTable[ConfigAllowed[i]].Index]);
                    swprintf(MenuBuffer, L"-%s", L"<AxisNameHere>");
//$END_MODIFICATIONS
                    text = MenuBuffer;
                    break;

                case KEY_TYPE_AXISPOS:
//$MODIFIED
//                    sprintf(MenuBuffer, "+%s", Joystick[RegistrySettings.Joystick].AxisName[KeyTable[ConfigAllowed[i]].Index]);
                    swprintf(MenuBuffer, L"+%s", L"<AxisNameHere>");
//$END_MODIFICATIONS
                    text = MenuBuffer;
                    break;
            }
            DrawMenuText( (FLOAT)x + MENU_TEXT_WIDTH * 20, (FLOAT)y, MENU_TEXT_RGB_NORMAL, text);
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
#ifdef OLD_AUDIO
                PlaySfx(SFX_MENU_BACK, SFX_MAX_VOL, SFX_CENTRE_PAN, SFX_SAMPLE_RATE, 0x7fffffff);
#else
                g_SoundEngine.Play2DSound( EFFECT_MenuPrev, FALSE );
#endif // OLD_AUDIO
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




////////////////////////////////////////////////////////////////
//
// Draw Ghost Type
//
////////////////////////////////////////////////////////////////
void DrawGhostType( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem, int itemIndex )
{
    FLOAT xPos = pMenuHeader->m_XPos + pMenuHeader->m_ItemTextWidth + MENU_TEXT_GAP;
    FLOAT yPos = pMenuHeader->m_YPos + (MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP) * itemIndex;

    DrawMenuTextWithArrows( (pMenuItem == pMenuHeader->m_pCurrentItem) ? TRUE : FALSE, 
                        xPos, yPos, MENU_TEXT_RGB_NORMAL, (gTitleScreenVars.LocalGhost)? TEXT_TABLE(TEXT_LOCAL) : TEXT_TABLE(TEXT_DOWNLOAD), 
                        pMenuHeader->m_ItemDataWidth );
}




////////////////////////////////////////////////////////////////
//
// DrawControllerType
//
////////////////////////////////////////////////////////////////
void DrawControllerType( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem, int itemIndex)
{
    FLOAT xPos, yPos;
    WCHAR *text;

    xPos = pMenuHeader->m_XPos + pMenuHeader->m_ItemTextWidth + MENU_TEXT_GAP;
    yPos = pMenuHeader->m_YPos + (MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP) * itemIndex;

    if (RegistrySettings.Joystick == -1)
    {
        text = L"Keyboard";

        // Disable steering stuff
        MenuItem_NonLinearSteer.ActiveFlags = 0;
        MenuItem_SteeringRange.ActiveFlags = 0;
        MenuItem_SteeringDeadZone.ActiveFlags = 0;
    }
    else
    {
//$MODIFIED
//        text = Joystick[RegistrySettings.Joystick].Name;
        text = L"<joystick name goes here>";
//$END_MODIFICATIONS

        // Enable steering stuff
        MenuItem_NonLinearSteer.ActiveFlags = MENU_ITEM_ACTIVE | MENU_ITEM_SELECTABLE;
        MenuItem_SteeringRange.ActiveFlags = MENU_ITEM_ACTIVE | MENU_ITEM_SELECTABLE;
        MenuItem_SteeringDeadZone.ActiveFlags = MENU_ITEM_ACTIVE | MENU_ITEM_SELECTABLE;
    }

    DrawMenuText(xPos,yPos, MENU_TEXT_RGB_NORMAL, text, pMenuHeader->m_ItemDataWidth);
}


COptionsStateEngine g_OptionsStateEngine;

//-----------------------------------------------------------------------------
// Name: Process()
// Desc: Main control function for the state engine
//-----------------------------------------------------------------------------
HRESULT COptionsStateEngine::Process()
{
    enum
    {
        OPTIONS_STATE_BEGIN = STATEENGINE_STATE_BEGIN,
        OPTIONS_STATE_MAINLOOP,
    };

    switch( m_State )
    {
        case OPTIONS_STATE_BEGIN:
            g_pMenuHeader->SetNextMenu( &Menu_Options );
            g_pTitleScreenCamera->SetNewPos( TITLESCREEN_CAMPOS_OVERVIEW );

            m_State = OPTIONS_STATE_MAINLOOP;
            break;

        case OPTIONS_STATE_MAINLOOP:
            // Nothing to do, as control is in the menus
            break;
    }

    // Handle the menus
    g_pMenuHeader->HandleMenus();

    return S_FALSE;
}




