//-----------------------------------------------------------------------------
// File: MainMenu.cpp
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "Revolt.h"
#include "Draw.h"
#include "DX.h"
#include "Main.h"
#include "Geom.h"
#include "Text.h"
#include "input.h"
#include "LevelLoad.h"
#include "settings.h"
#include "timing.h"
#include "initplay.h"
#include "TitleScreen.h"
#include "VoiceCommunicator.h"
#include "XBFont.h"
#include "Menu2.h"
#include "MainMenu.h"

// globals
short MenuCount;

// menu text
char MainMenuAllowed[] = 
{
    TRUE,
    FALSE,
    FALSE,
    FALSE,
    FALSE,
    TRUE,
    TRUE,
    FALSE,
    TRUE,
    TRUE,
    FALSE,
    FALSE,
    TRUE,
    TRUE,
    FALSE,
    FALSE,

    FALSE,
};

char *MainMenuText[] = 
{
    "Start Game",
    "Start AI Test",
    "Start Multi Player",
    "Join Multi Player",
    "Track: ",
    "Screen: ",
    "Textures: ",
    "Device: ",
    "Joystick: ",
    "Car: ",
    "Change Name: ",
    "Edit Mode: ",
    "Brightness: ",
    "Contrast: ",
    "Reversed: ",
    "Mirrored: ",

    "New Front End",
};

char *TextureFilterText[] = 
{
    "Point",
    "Linear",
    "Anisotropic",
};

char *MipMapText[] = 
{
    "None",
    "Point",
    "Linear",
};

char *NoYesText[] = 
{
    "No",
    "Yes",
};




///////////////
// main menu //
///////////////
void MainMenu(void)
{
    short i, line, flag;
    long col;
    unsigned char c;
    char *text;
    char buf[128];
    LEVELINFO *levelInfo;

// buffer flip / clear

//$REMOVED    CheckSurfaces();
    FlipBuffers();
    ClearBuffers();

// update pos

//$REMOVED    ReadMouse();
//$REMOVED    ReadKeyboard();
    ReadJoystick(); //$ADDITION
    UpdateTimeStep();

    if (Keys[DIK_UP] && !LastKeys[DIK_UP])
    {
        flag = TRUE;
        while ((!MainMenuAllowed[MenuCount]) || flag)
        {
            MenuCount--;
            if (MenuCount < 0) MenuCount = MAIN_MENU_NUM - 1;
            flag = FALSE;
        }
    }

    if (Keys[DIK_DOWN] && !LastKeys[DIK_DOWN])
    {
        flag = TRUE;
        while ((!MainMenuAllowed[MenuCount]) || flag)
        {
            MenuCount++;
            if (MenuCount > MAIN_MENU_NUM - 1) MenuCount = 0;
            flag = FALSE;
        }
    }

// show menu

//$REMOVED    D3Ddevice->BeginScene();

//  BlitBitmap(TitleHbm, &BackBuffer);

    BeginTextState();

    line = 8;

    for (i = 0 ; i < MAIN_MENU_NUM ; i++) if (MainMenuAllowed[i])
    {
        if (MenuCount == i)
            col = 0xff0000;
        else
            col = 0x808080;

        DrawWideGameText(224, line * 16, col, MainMenuText[i]);

        if (i == MAIN_MENU_TRACK)
        {
            levelInfo = GetLevelInfo(GameSettings.Level);
            if (MenuCount == i)
            {
                if (Keys[DIK_LEFT] && !LastKeys[DIK_LEFT]) GameSettings.Level--;
                if (Keys[DIK_RIGHT] && !LastKeys[DIK_RIGHT]) GameSettings.Level++;
                if (GameSettings.Level == -1) GameSettings.Level = GameSettings.LevelNum - 1;
                if (GameSettings.Level == GameSettings.LevelNum) GameSettings.Level = 0;

                memcpy(RegistrySettings.LevelDir, levelInfo->Dir, MAX_LEVEL_DIR_NAME);
            }
            DrawWideGameText(224 + strlen(MainMenuText[i]) * 12, line * 16, col, levelInfo->Name);
        }

        if (i == MAIN_MENU_RES)
        {
            if (MenuCount == i)
            {
//$REMOVED
//                if (Keys[DIK_LEFT] && !LastKeys[DIK_LEFT]) DisplayModeCount--;
//                if (Keys[DIK_RIGHT] && !LastKeys[DIK_RIGHT]) DisplayModeCount++;
//
//                if (DisplayModeCount < 0) DisplayModeCount = 0;
//                if (DisplayModeCount >= DrawDevices[RegistrySettings.DrawDevice].DisplayModeNum) DisplayModeCount = DrawDevices[RegistrySettings.DrawDevice].DisplayModeNum - 1;

//$MODIFIED
//                RegistrySettings.ScreenWidth = DrawDevices[RegistrySettings.DrawDevice].DisplayMode[DisplayModeCount].Width;
//                RegistrySettings.ScreenHeight = DrawDevices[RegistrySettings.DrawDevice].DisplayMode[DisplayModeCount].Height;
//                RegistrySettings.ScreenBpp = DrawDevices[RegistrySettings.DrawDevice].DisplayMode[DisplayModeCount].Bpp;
//            }
//            DrawWideGameText(224 + strlen(MainMenuText[i]) * 12, line * 16, col, DrawDevices[RegistrySettings.DrawDevice].DisplayMode[DisplayModeCount].DisplayText);
                RegistrySettings.ScreenWidth  = 640;
                RegistrySettings.ScreenHeight = 480;
                RegistrySettings.ScreenBpp    = 32;
        }
        DrawWideGameText(224 + strlen(MainMenuText[i]) * 12, line * 16, col, "640 x 480");
//$END_MODIFICATIONS
        }

        if (i == MAIN_MENU_TEXBPP)
        {
            if (MenuCount == i)
            {
//              if (Keys[DIK_LEFT] && !LastKeys[DIK_LEFT]) RegistrySettings.TextureBpp = 16;
//              if (Keys[DIK_RIGHT] && !LastKeys[DIK_RIGHT]) RegistrySettings.TextureBpp = 24;
            }
//          DrawWideGameText(224 + strlen(MainMenuText[i]) * 12, line * 16, col, RegistrySettings.TextureBpp == 16 ? "16 bit" : "24 bit");
        }

        if (i == MAIN_MENU_DEVICE)
        {
            if (MenuCount == i)
            {
//$REMOVED
//                if (Keys[DIK_LEFT] && !LastKeys[DIK_LEFT] && RegistrySettings.DrawDevice)
//                {
//                    RegistrySettings.DrawDevice--;
//                    DisplayModeCount = DrawDevices[RegistrySettings.DrawDevice].BestDisplayMode;
//                }
//                if (Keys[DIK_RIGHT] && !LastKeys[DIK_RIGHT] && RegistrySettings.DrawDevice < (DWORD)DrawDeviceNum - 1)
//                {
//                    RegistrySettings.DrawDevice++;
//                    DisplayModeCount = DrawDevices[RegistrySettings.DrawDevice].BestDisplayMode;
//                }
            }
//$MODIFIED
//            DrawWideGameText(224 + strlen(MainMenuText[i]) * 12, line * 16, col, DrawDevices[RegistrySettings.DrawDevice].Name);
            DrawWideGameText(224 + strlen(MainMenuText[i]) * 12, line * 16, col, "Xbox Device");
//$END_MODIFICATIONS
        }

        if (i == MAIN_MENU_JOYSTICK)
        {
#ifndef XBOX_NOT_YET_IMPLEMENTED
            if (JoystickNum && MenuCount == i)
            {
                if (Keys[DIK_LEFT] && !LastKeys[DIK_LEFT] && RegistrySettings.Joystick > -1) RegistrySettings.Joystick--;
                if (Keys[DIK_RIGHT] && !LastKeys[DIK_RIGHT] && RegistrySettings.Joystick < JoystickNum - 1) RegistrySettings.Joystick++;
            }

            if (RegistrySettings.Joystick == -1)
            {
                text = "None";
            }
            else
            {
                text = Joystick[RegistrySettings.Joystick].Name;
            }

            DrawWideGameText(224 + strlen(MainMenuText[i]) * 12, line * 16, col, text);
#endif // ! XBOX_NOT_YET_IMPLEMENTED
        }

        if (i == MAIN_MENU_CAR)
        {
            if (MenuCount == i)
            {
                if (Keys[DIK_LEFT] && !LastKeys[DIK_LEFT])
                {
                    GameSettings.CarType = PrevValidCarType(GameSettings.CarType);
                }
                if (Keys[DIK_RIGHT] && !LastKeys[DIK_RIGHT])
                {
                    GameSettings.CarType = NextValidCarType(GameSettings.CarType);
                }
            }

            text = CarInfo[GameSettings.CarType].Name;
            DrawWideGameText(224 + strlen(MainMenuText[i]) * 12, line * 16, col, text);
        }

        if (i == MAIN_MENU_NAME)
        {
            if (MenuCount == i)
            {
                if ((c = GetKeyPress()))
                {
                    if (c == 8)
                    {
                        if (strlen(RegistrySettings.PlayerName))
                        {
                            RegistrySettings.PlayerName[strlen(RegistrySettings.PlayerName) - 1] = 0;
                        }
                    }
                    else if (c != 13 && c != 27)
                    {
                        if (strlen(RegistrySettings.PlayerName) < MAX_PLAYER_NAME - 1)
                        {
                            RegistrySettings.PlayerName[strlen(RegistrySettings.PlayerName)] = c;
                            RegistrySettings.PlayerName[strlen(RegistrySettings.PlayerName) + 1] = 0;
                        }
                    }
                }
                DrawWideGameText(224 + strlen(MainMenuText[i]) * 12 + strlen(RegistrySettings.PlayerName) * 12, line * 16, col, "_");
            }
            DrawWideGameText(224 + strlen(MainMenuText[i]) * 12, line * 16, col, RegistrySettings.PlayerName);
        }

        if (i == MAIN_MENU_EDIT)
        {
/*
            if (MenuCount == i)
            {
                if (Keys[DIK_LEFT] && !LastKeys[DIK_LEFT]) EditMode--;
                if (Keys[DIK_RIGHT] && !LastKeys[DIK_RIGHT]) EditMode++;
                if (EditMode < 0) EditMode = EDIT_NUM - 1;
                if (EditMode == EDIT_NUM) EditMode = 0;
            }
            DrawWideGameText(224 + strlen(MainMenuText[i]) * 12, line * 16, col, EditMenuText[EditMode]);
*/
        }

        if (i == MAIN_MENU_BRIGHTNESS)
        {
            if (GammaFlag == GAMMA_UNAVAILABLE)
            {
                text = "Unavailable";
            }
            else if (GammaFlag == GAMMA_AUTO)
            {
                text = "Auto";
            }
            else
            {
                if (MenuCount == i)
                {
                    if (Keys[DIK_LEFT]) RegistrySettings.Brightness -= (long)(TimeStep * 100.0f);
                    if (Keys[DIK_RIGHT]) RegistrySettings.Brightness += (long)(TimeStep * 100.0f);

                    if (RegistrySettings.Brightness & 0x80000000) RegistrySettings.Brightness = 0;
                    if (RegistrySettings.Brightness > 512) RegistrySettings.Brightness = 512;

                    SetGamma(RegistrySettings.Brightness, RegistrySettings.Contrast);
                }
                wsprintf(buf, "%d", RegistrySettings.Brightness * 100 / 512);
                text = buf;
            }
            DrawWideGameText(224 + strlen(MainMenuText[i]) * 12, line * 16, col, text);
        }

        if (i == MAIN_MENU_CONTRAST)
        {
            if (GammaFlag == GAMMA_UNAVAILABLE)
            {
                text = "Unavailable";
            }
            else if (GammaFlag == GAMMA_AUTO)
            {
                text = "Auto";
            }
            else
            {
                if (MenuCount == i)
                {
                    if (Keys[DIK_LEFT]) RegistrySettings.Contrast -= (long)(TimeStep * 100.0f);
                    if (Keys[DIK_RIGHT]) RegistrySettings.Contrast += (long)(TimeStep * 100.0f);

                    if (RegistrySettings.Contrast & 0x80000000) RegistrySettings.Contrast = 0;
                    if (RegistrySettings.Contrast > 512) RegistrySettings.Contrast = 512;

                    SetGamma(RegistrySettings.Brightness, RegistrySettings.Contrast);
                }
                wsprintf(buf, "%d", RegistrySettings.Contrast * 100 / 512);
                text = buf;
            }
            DrawWideGameText(224 + strlen(MainMenuText[i]) * 12, line * 16, col, text);
        }

        if (i == MAIN_MENU_REVERSED)
        {
            if (MenuCount == i)
            {
                if (Keys[DIK_LEFT] && !LastKeys[DIK_LEFT]) GameSettings.Reversed = FALSE;
                if (Keys[DIK_RIGHT] && !LastKeys[DIK_RIGHT]) GameSettings.Reversed = TRUE;
            }
            DrawWideGameText(224 + strlen(MainMenuText[i]) * 12, line * 16, col, NoYesText[GameSettings.Reversed]);
        }

        if (i == MAIN_MENU_MIRRORED)
        {
            if (MenuCount == i)
            {
                if (Keys[DIK_LEFT] && !LastKeys[DIK_LEFT]) GameSettings.Mirrored = FALSE;
                if (Keys[DIK_RIGHT] && !LastKeys[DIK_RIGHT]) GameSettings.Mirrored = TRUE;
            }
            DrawWideGameText(224 + strlen(MainMenuText[i]) * 12, line * 16, col, NoYesText[GameSettings.Mirrored]);
        }

        line++;
    }

//$REMOVED    D3Ddevice->EndScene();

// selected?

    if (Keys[DIK_RETURN] && !LastKeys[DIK_RETURN] && MainMenuAllowed[MenuCount])
    {
/*
        AI_Testing = FALSE;
*/
        levelInfo = GetLevelInfo(GameSettings.Level);

        if (MenuCount == MAIN_MENU_SINGLE)
        {
            memcpy(StartData.LevelDir, levelInfo->Dir, MAX_LEVEL_DIR_NAME);
            GameSettings.GameType = GAMETYPE_TRIAL;
            SET_EVENT(SetupGame);
        }

        if (MenuCount == MAIN_MENU_AI_TEST)
        {
/*
            memcpy(StartData.LevelDir, levelInfo->Dir, MAX_LEVEL_DIR_NAME);
            AI_Testing = TRUE;
            GameSettings.GameType = GAMETYPE_SINGLE;
            SET_EVENT(SetupGame);
*/
        }

        if (MenuCount == MAIN_MENU_MULTI)
        {
            memcpy(StartData.LevelDir, levelInfo->Dir, MAX_LEVEL_DIR_NAME);
            GameSettings.MultiType = MULTITYPE_SERVER;
            MenuCount = 0;
            InitNetwork();
            SET_EVENT(EnterSessionName);  //$MODIFIED: was set to ConnectionMenu, but we bypass that now
        }

        if (MenuCount == MAIN_MENU_JOIN)
        {
            memcpy(StartData.LevelDir, levelInfo->Dir, MAX_LEVEL_DIR_NAME);
            GameSettings.MultiType = MULTITYPE_CLIENT;
            MenuCount = 0;
            InitNetwork();
            SessionCount = 0;          //$ADDITION: b/c bypassing work done in ConnectionMenu
            SessionRequestTime = 0.0f; //$ADDITION: b/c bypassing work done in ConnectionMenu
            SET_EVENT(BrowseSessions);  //$MODIFIED: was set to ConnectionMenu, but we bypass that now
        }

        if (MenuCount == MAIN_MENU_NEW_TITLE_SCREEN)
        {
            SET_EVENT(GoTitleScreen);
        }

        // Initialise the in-game menu
        //gMenuHeader.pMenu = &gGamePaws_Menu;
        //MenuInit(&gMenuHeader, gMenuHeader.pMenu, gMenuHeader.pMenu);
    }

// quit?

    if (Keys[DIK_ESCAPE] && !LastKeys[DIK_ESCAPE])
        g_bQuitGame = TRUE;
}
