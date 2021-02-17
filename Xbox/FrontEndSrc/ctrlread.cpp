//-----------------------------------------------------------------------------
// File: ctrlread.cpp
//
// Desc: Controller reading code (for mouse, keyboard, etc)
//
//       Moved over code from control.cpp. control.cpp now used for processing
//       controller inputs into game controls. Controller inputs are generated
//       in this file. Local key handling is done here without being passed to
//       the game code itself (ie. for car model changes, etc).
//
// Re-Volt (PC) Copyright (c) Probe Entertainment 1998
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"
#include "main.h"
#include "dx.h"
#include "geom.h"
#include "particle.h"
#include "texture.h"
#include "model.h"
#include "aerial.h"
#include "network.h"
#include "NewColl.h"
#include "body.h"
#include "car.h"
#include "input.h"
#include "camera.h"
#include "light.h"
#include "visibox.h"
#include "LevelLoad.h"
#include "ctrlread.h"
#include "object.h"
#include "obj_init.h"
#include "control.h"
#include "player.h"
#include "aizone.h"
#include "timing.h"
#include "ghost.h"
#include "settings.h"
#include "panel.h"
#include "move.h"
#include "readinit.h"

#include "TitleScreen.h"
#include "Cheats.h"

extern char *CarInfoFile;
extern bool GHO_ShowGhost;

//
// Static variable declarations
//

static long FunctionKeyNum;
static FUNCTION_KEY FunctionKey[FUNCTION_KEY_MAX];

static REAL impPos = 10.0f;
static REAL impMag = 100.0f;

static char DikString[MAX_PATH];
//
// Static function prototypes
//

static void ToggleGhostRender(void);
static void SetPrevCarType(void);
static void SetNextCarType(void);
static void ReadCarInfoAgain(void);
static void SetNextCameraFollow(void);
static void SetNextLocalCamera(void);
static void ToggleRearView(void);
static void ToggleFollowView(void);
static void CycleFollowView(void);
static void SetNextCameraAttached(void);
static void SetNextCameraFreedom(void);
static void SetCameraEdit(void);
static void SetNextCameraRail(void);
static void ToggleAutobrake(void);
static void ClearGhostCar(void);
static void RestartGhostCar(void);
static void AddNewCar(void);
static void ControlNextCar(void);
static void CycleSpeedUnits(void);
static void ToggleCarAxisVisible(void);
static void DecCarFrontToeIn(void);
static void IncCarFrontToeIn(void);
static void DecCarRearToeIn(void);
static void IncCarRearToeIn(void);
static void StopCarMotion(void);
static void ApplyImpXneg(void);
static void ApplyImpXpos(void);
static void ApplyImpYneg(void);
static void ApplyImpYpos(void);
static void ApplyImpZneg(void);
static void ApplyImpZpos(void);
static void ApplyAngImpXneg(void);
static void ApplyAngImpXpos(void);
static void ApplyAngImpYneg(void);
static void ApplyAngImpYpos(void);
static void ApplyAngImpZneg(void);
static void ApplyAngImpZpos(void);
static void ResetLocalCar(void);
static void GrabScreen(void);
static void ToggleCarVisi(void);

//
// Global function prototypes
//

void CRD_CheckLocalKeys(void);
void CRD_InitPlayerControl(PLAYER *player, CTRL_TYPE CtrlType);
void CRD_LocalInput(PLAYER *player);
void CRD_RemoteInput(PLAYER *player);
void CRD_ReplayInput(PLAYER *player);
void CRD_TitleScreenTraining(PLAYER *player);

// key table

KEY KeyTable[KEY_NUM];

// function key list - sys key, NOT sys key, key, new only, function

static FUNCTION_KEY FuncKeys[] = {

    0, 0, DIK_F10, TRUE, ResetLocalCar,
    0, 0, DIK_PGUP, TRUE, SetPrevCarType,
    0, 0, DIK_PGDN, TRUE, SetNextCarType,

    0, 0, DIK_F1, TRUE, SetNextLocalCamera,
    0, 0, DIK_F2, TRUE, ToggleRearView,
    0, 0, DIK_F3, TRUE, ToggleFollowView,
    0, 0, DIK_F4, TRUE, CycleFollowView,
    0, 0, DIK_F4, FALSE, SetCameraEdit,
    0, 0, DIK_F5, TRUE, SetNextCameraRail,
    0, 0, DIK_F6, TRUE, SetNextCameraFreedom,
    0, 0, DIK_F7, TRUE, ToggleCarVisi,
    0, 0, DIK_F8, TRUE, GrabScreen,


#if WANKY_KEYS
    0, 0, DIK_8, TRUE, ToggleGhostRender,
    0, 0, DIK_4, TRUE, ToggleAutobrake,
    DIK_LSHIFT, 0, DIK_5, TRUE, ClearGhostCar,
    0, DIK_LSHIFT, DIK_5, TRUE, RestartGhostCar,
    0, 0, DIK_9, TRUE, AddNewCar,
    0, 0, DIK_0, TRUE, ControlNextCar,
    0, 0, DIK_1, TRUE, CycleSpeedUnits,
    DIK_RSHIFT, 0, DIK_P, TRUE, DecCarFrontToeIn,
    0, DIK_RSHIFT, DIK_P, TRUE, IncCarFrontToeIn,
    DIK_RSHIFT, 0, DIK_L, TRUE, DecCarRearToeIn,
    0, DIK_RSHIFT, DIK_L, TRUE, IncCarRearToeIn,
    0, 0, DIK_T, TRUE, StopCarMotion,
#endif

#ifdef CHRIS_EXTRAS
    DIK_LSHIFT, 0, DIK_F1, TRUE, ReadCarInfoAgain,
    DIK_LSHIFT, 0, DIK_X, FALSE, ApplyImpXneg,
    0, DIK_LSHIFT, DIK_X, FALSE, ApplyImpXpos,
    DIK_LSHIFT, 0, DIK_Y, FALSE, ApplyImpYneg,
    0, DIK_LSHIFT, DIK_Y, FALSE, ApplyImpYpos,
    DIK_LSHIFT, 0, DIK_Z, FALSE, ApplyImpZneg,
    0, DIK_LSHIFT, DIK_Z, FALSE, ApplyImpZpos,
    DIK_LSHIFT, 0, DIK_B, FALSE, ApplyAngImpXneg,
    0, DIK_LSHIFT, DIK_B, FALSE, ApplyAngImpXpos,
    DIK_LSHIFT, 0, DIK_N, FALSE, ApplyAngImpYneg,
    0, DIK_LSHIFT, DIK_N, FALSE, ApplyAngImpYpos,
    DIK_LSHIFT, 0, DIK_M, FALSE, ApplyAngImpZneg,
    0, DIK_LSHIFT, DIK_M, FALSE, ApplyAngImpZpos,
    0, 0, DIK_2, TRUE, ToggleCarAxisVisible,
#endif

    0, 0, 0, 0, 0,

};

//--------------------------------------------------------------------------------------------------------------------------

//
// CRD_CheckLocalKeys
//
// Checks local keypresses
//

void CRD_CheckLocalKeys(void)
{
    long i;

// loop through all function keys

    for (i = 0 ; i < FunctionKeyNum ; i++)
    {
        if (Keys[FunctionKey[i].SysKey] && !Keys[FunctionKey[i].NotSysKey] && Keys[FunctionKey[i].Key] && !(LastKeys[FunctionKey[i].Key] && FunctionKey[i].NewOnly))
        {
            if (FunctionKey[i].Func)
                FunctionKey[i].Func();
        }
    }
}

//--------------------------------------------------------------------------------------------------------------------------

void CRD_InitPlayerControl(PLAYER *player, CTRL_TYPE CtrlType)
{
    switch(CtrlType)
    {
        case CTRL_TYPE_NONE:
        player->ctrlhandler = NULL;
        break;

        case CTRL_TYPE_LOCAL:
        player->ctrlhandler = (CTRL_HANDLER)CRD_LocalInput;
        break;

        case CTRL_TYPE_REMOTE:
        player->ctrlhandler = (CTRL_HANDLER)CRD_RemoteInput;
        break;

        case CTRL_TYPE_REPLAY:
        player->ctrlhandler = (CTRL_HANDLER)CRD_ReplayInput;
        break;

        case CTRL_TYPE_TS_TRAINING:
        player->ctrlhandler = (CTRL_HANDLER)CRD_TitleScreenTraining;
        break;

        case CTRL_TYPE_CPU_AI:
        player->ctrlhandler = NULL;
        break;

        //CTRL_TYPE_KBDJOY:
        //CTRL_TYPE_MOUSE:
        //return(0);                                        // Not supported yet
        //break;

        default:
        break;
    };

    CON_InitControls(&player->controls);
}

//////////////////////////////////////////////////////////////
// search for a key press - fill in key struct if one found //
//////////////////////////////////////////////////////////////

long SearchForKeyPress(KEY *key)
{
    long i, delta, gaykey;

// check keyboard

    gaykey = 0;

    for (i = 0 ; i < 255 ; i++)
    {
        if (Keys[i])
        {
            if (i == 197)   // pause / break
            {
                gaykey = 2;
                continue;
            }

            if (i >= DIK_F1 && i <= DIK_F12)    // function keys
            {
                gaykey = 2;
                continue;
            }

            key->Index = (short)i;
            key->Type = KEY_TYPE_KEYBOARD;
            return 1;
        }
    }

// quit now if no joystick

    if (RegistrySettings.Joystick == -1)
        return gaykey;

// check buttons

//$MODIFIED
//    for (i = 0 ; i < Joystick[RegistrySettings.Joystick].ButtonNum ; i++)
    for (i = 0 ; i < 8 ; i++)  //$HACK: assume 8 joystick buttons for now.  Need to $REVISIT this.
//$END_MODIFICATIONS
    {
        if (JoystickState.rgbButtons[i])
        {
            key->Index = (short)i;
            key->Type = KEY_TYPE_BUTTON;
            return 1;
        }
    }

// check axis

//$MODIFIED
//    for (i = 0 ; i < Joystick[RegistrySettings.Joystick].AxisNum ; i++) if (!Joystick[RegistrySettings.Joystick].AxisDisable[i])
    for (i = 0 ; i < 2 ; i++) //$HACK: assume 2 axes for now, none disabled.  Need to $REVISIT this.
//$END_MODIFICATIONS
    {
//$MODIFIED
//        delta = ((long*)&JoystickState.lX)[Joystick[RegistrySettings.Joystick].AxisIndex[i]];
        delta = ((long*)&JoystickState.lX)[i];  //$HACK: assume AxisIndex[x] == x.  Need to $REVISIT this.
//$END_MODIFICATIONS

        if (delta < -(CTRL_RANGE_MAX / 2))
        {
            key->Index = (short)i;
            key->Type = KEY_TYPE_AXISNEG;
            return 1;
        }
        if (delta > (CTRL_RANGE_MAX / 2))
        {
            key->Index = (short)i;
            key->Type = KEY_TYPE_AXISPOS;
            return 1;
        }
    }


// no press

    return gaykey;
}

///////////////////////////////////////////////////////
// return true if any fresh key or button is pressed //
///////////////////////////////////////////////////////

bool AnyKeyOrButton(void)
{
    long i;

// check keys

    for (i = 0 ; i < 255 ; i++)
    {
        if (Keys[i] && !LastKeys[i])
            return TRUE;
    }

// check buttons

//$MODIFIED
//    if (RegistrySettings.Joystick != -1) for (i = 0 ; i < Joystick[RegistrySettings.Joystick].ButtonNum ; i++)
    for (i = 0 ; i < 8 ; i++)  //$HACK: assume 8 joystick buttons for now.  Need to $REVISIT this.
//$END_MODIFICATIONS
    {
        if (JoystickState.rgbButtons[i] && !LastJoystickState.rgbButtons[i])
            return true;
    }

// nope

    return FALSE;
}

////////////////////////////////////////////////
// get a digital state for a KEY_TABLE entry //
////////////////////////////////////////////////

bool GetDigitalState(KEY *key)
{

// check state

    switch (key->Type)
    {
        case KEY_TYPE_KEYBOARD:
            if (Keys[key->Index]) return true;
            break;

        case KEY_TYPE_BUTTON:
            if (JoystickState.rgbButtons[key->Index]) return true;
            break;
    }

// not pressed

    return false;
}

//////////////////////////////////////////////////////////////////
// get a digital state for a KEY_TABLE entry - only if joystick //
//////////////////////////////////////////////////////////////////

bool GetDigitalStateJoy(KEY *key)
{

// check state

    switch (key->Type)
    {
        case KEY_TYPE_KEYBOARD:
            return false;

        case KEY_TYPE_BUTTON:
            if (JoystickState.rgbButtons[key->Index]) return true;
            break;
    }

// not pressed

    return false;
}

////////////////////////////////////////////////
// get a digital state for a KEY_TABLE entry //
////////////////////////////////////////////////

char *GetStringFromDik(long index)
{
    unsigned long vk, ch;

    DikString[0] = 0;

    switch (index)
    {
        case DIK_LEFT:
            wsprintf(DikString, "LEFT");
            break;

        case DIK_RIGHT:
            wsprintf(DikString, "RIGHT");
            break;

        case DIK_UP:
            wsprintf(DikString, "UP");
            break;

        case DIK_DOWN:
            wsprintf(DikString, "DOWN");
            break;

        case DIK_LSHIFT:
        case DIK_RSHIFT:
            wsprintf(DikString, "SHIFT");
            break;

        case DIK_LCONTROL:
        case DIK_RCONTROL:
            wsprintf(DikString, "CTRL");
            break;

        case DIK_LALT:
        case DIK_RALT:
            wsprintf(DikString, "ALT");
            break;

        case DIK_SPACE:
            wsprintf(DikString, "SPACE");
            break;

        case DIK_RETURN:
        case DIK_NUMPADENTER:
            wsprintf(DikString, "ENTER");
            break;

        case DIK_ESCAPE:
            wsprintf(DikString, "ESCAPE");
            break;

        case DIK_TAB:
            wsprintf(DikString, "TAB");
            break;

        case DIK_CAPSLOCK:
            wsprintf(DikString, "CAPS LOCK");
            break;

        case DIK_INSERT:
            wsprintf(DikString, "INSERT");
            break;

        case DIK_HOME:
            wsprintf(DikString, "HOME");
            break;

        case DIK_PGUP:
            wsprintf(DikString, "PAGE UP");
            break;

        case DIK_PGDN:
            wsprintf(DikString, "PAGE DOWN");
            break;

        case DIK_DELETE:
            wsprintf(DikString, "DELETE");
            break;

        case DIK_BACKSPACE:
            wsprintf(DikString, "BACKSPACE");
            break;

        case DIK_END:
            wsprintf(DikString, "END");
            break;

        case DIK_NUMLOCK:
            wsprintf(DikString, "NUMLOCK");
            break;

        case DIK_SCROLL:
            wsprintf(DikString, "SCROLL LOCK");
            break;

        case 183:
            wsprintf(DikString, "PRINT SCREEN");
            break;

        case 197:
            wsprintf(DikString, "PAUSE");
            break;

        default:
#ifndef XBOX_NOT_YET_IMPLEMENTED
            vk = MapVirtualKey(index, 1);
            if (!vk) break;
            ch = MapVirtualKey(vk, 2);
            if (!ch) break;
            wsprintf(DikString, "%c", ch);
#endif // ! XBOX_NOT_YET_IMPLEMENTED
            break;
    }

// return string

    return DikString;
}
    
/////////////////////////////////////////////////
// get an analogue pair for 2 KEY_TABLE entrys //
/////////////////////////////////////////////////

char GetAnaloguePair(KEY *key1, KEY *key2)
{
    char a = 0;
    long delta;
               
// key 1

    switch (key1->Type)
    {
        case KEY_TYPE_KEYBOARD:
            if (Keys[key1->Index]) a -= CTRL_RANGE_MAX;
            break;

        case KEY_TYPE_BUTTON:
            if (JoystickState.rgbButtons[key1->Index]) a -= CTRL_RANGE_MAX;
            break;

        case KEY_TYPE_AXISNEG:
//$MODIFIED
//            delta = ((long*)&JoystickState.lX)[Joystick[RegistrySettings.Joystick].AxisIndex[key1->Index]];
            delta = ((long*)&JoystickState.lX)[key1->Index]; //$HACK: assume AxisIndex[x] == x.  Need to $REVISIT this.
//$END_MODIFICATIONS
            if (delta < 0)
            {
                a += (char)delta;
                if (a < -CTRL_RANGE_MAX) a = -CTRL_RANGE_MAX;
            }
            break;

        case KEY_TYPE_AXISPOS:
//$MODIFIED
//            delta = ((long*)&JoystickState.lX)[Joystick[RegistrySettings.Joystick].AxisIndex[key1->Index]];
            delta = ((long*)&JoystickState.lX)[key1->Index]; //$HACK: assume AxisIndex[x] == x.  Need to $REVISIT this.
//$END_MODIFICATIONS
            if (delta > 0)
            {
                a -= (char)delta;
                if (a < -CTRL_RANGE_MAX) a = -CTRL_RANGE_MAX;
            }
            break;
    }

// key 2

    switch (key2->Type)
    {
        case KEY_TYPE_KEYBOARD:
            if (Keys[key2->Index]) a += CTRL_RANGE_MAX;
            break;

        case KEY_TYPE_BUTTON:
            if (JoystickState.rgbButtons[key2->Index]) a += CTRL_RANGE_MAX;
            break;

        case KEY_TYPE_AXISNEG:
//$MODIFIED
//            delta = ((long*)&JoystickState.lX)[Joystick[RegistrySettings.Joystick].AxisIndex[key2->Index]];
            delta = ((long*)&JoystickState.lX)[key2->Index]; //$HACK: assume AxisIndex[x] == x.  Need to $REVISIT this.
//$END_MODIFICATIONS
            if (delta < 0)
            {
                a -= (char)delta;
                if (a > CTRL_RANGE_MAX) a = CTRL_RANGE_MAX;
            }
            break;

        case KEY_TYPE_AXISPOS:
//$MODIFIED
//            delta = ((long*)&JoystickState.lX)[Joystick[RegistrySettings.Joystick].AxisIndex[key2->Index]];
            delta = ((long*)&JoystickState.lX)[key2->Index]; //$HACK: assume AxisIndex[x] == x.  Need to $REVISIT this.
//$END_MODIFICATIONS
            if (delta > 0)
            {
                a += (char)delta;
                if (a > CTRL_RANGE_MAX) a = CTRL_RANGE_MAX;
            }
            break;
    }

// return analogue input

    return a;
}

//--------------------------------------------------------------------------------------------------------------------------

void CRD_LocalInput(PLAYER *player)
{

// left / right

    player->controls.dx = GetAnaloguePair(&KeyTable[KEY_LEFT], &KeyTable[KEY_RIGHT]);
    if (GameSettings.Mirrored) player->controls.dx = -player->controls.dx;

// forward / back

    player->controls.dy = GetAnaloguePair(&KeyTable[KEY_FWD], &KeyTable[KEY_BACK]);

// fire

    if (GetDigitalState(&KeyTable[KEY_FIRE]))
    {
        player->controls.digital |= CTRL_FIRE;
    }

// reset

    if (GetDigitalState(&KeyTable[KEY_RESET]))
    {
        player->controls.digital |= CTRL_RESET;
    }

// reposition

    if (GetDigitalState(&KeyTable[KEY_REPOSITION]))
    {
        player->controls.digital |= CTRL_REPOSITION;
    }

// honka

    if (GetDigitalState(&KeyTable[KEY_HONKA]))
    {
        player->controls.digital |= CTRL_HONKA;
    }

// select weapon

    if (GetDigitalState(&KeyTable[KEY_SELWEAPON]))
    {
        player->controls.digital |= CTRL_SELWEAPON;
    }

// pause

    if (GetDigitalState(&KeyTable[KEY_PAUSE]))
    {
        player->controls.digital |= CTRL_PAUSE;
    }

// gari's shite...
#if 0
    if ((Version == VERSION_DEV) && gGazzasAICar)
    {
//      if (!(player->controls.digital | player->controls.dx | player->controls.dy))
        if (!Keys[DIK_RCONTROL])
        {
             player->controls.dx = player->CarAI.dx;
             player->controls.dy = player->CarAI.dy;
        }
    }
#endif
}

//-------------------------------------------------------------------------------------------------------------------------


void CRD_TitleScreenTraining(PLAYER *player)
{
    if (player->car.Body->BangMag >= Real(400))
    {
//      if ((int)(TimeStep * 1000) & 1)
        //player->DINKY_DX = -player->DINKY_DX;
        //player->DINKY_DY = -player->DINKY_DY;
        player->controls.dx = -player->controls.dx;
        player->controls.dy = -player->controls.dy;

        player->DinkyTimer = Real(1*3);
    }
    else if (player->DinkyTimer <= 0)
    {

        if (player->controls.dx == 0) {
            if (frand(ONE) > HALF) {
                player->controls.dx = CTRL_RANGE_MAX;
            } else {
                player->controls.dx = -CTRL_RANGE_MAX;
            }
        } else {
            player->controls.dx = -player->controls.dx;
        }

        if (player->controls.dy == 0) {
            if (frand(ONE) > HALF) {
                player->controls.dy = CTRL_RANGE_MAX;
            } else {
                player->controls.dy = -CTRL_RANGE_MAX;
            }
        } else {
            player->controls.dy = -player->controls.dy;
        }

        player->DinkyTimer = Real(1*3);
    }

    //player->controls.dx = (signed char)player->controls.dx;
    //player->controls.dy = (signed char)player->controls.dy;
    //player->controls.digital = 0;

    player->DinkyTimer -= TimeStep;
}

/////////////////////////
// init functions keys //
/////////////////////////

void InitFunctionKeys(void)
{
    FunctionKeyNum = 0;
}

////////////////////////////////////////////////////
// add a functions key - return TRUE if sucessful //
////////////////////////////////////////////////////

long AddFunctionKey(long syskey, long notsyskey, long key, void (*func)(void), long newonly)
{

// full?

    if (FunctionKeyNum >= FUNCTION_KEY_MAX)
        return FALSE;

// add

    if (syskey) FunctionKey[FunctionKeyNum].SysKey = syskey;
    else FunctionKey[FunctionKeyNum].SysKey = key;
    FunctionKey[FunctionKeyNum].NotSysKey = notsyskey;
    FunctionKey[FunctionKeyNum].Key = key;
    FunctionKey[FunctionKeyNum].Func = func;
    FunctionKey[FunctionKeyNum].NewOnly = newonly;

// return OK

    FunctionKeyNum++;
    return TRUE;
}

/////////////////////////
// toggle ghost render //
/////////////////////////

static void ToggleGhostRender(void)
{
    if (GHO_GhostPlayer->car.RenderFlag == CAR_RENDER_GHOST)
        GHO_GhostPlayer->car.RenderFlag = CAR_RENDER_NORMAL;
    else if (GHO_GhostPlayer->car.RenderFlag == CAR_RENDER_NORMAL)
        GHO_GhostPlayer->car.RenderFlag = CAR_RENDER_GHOST;
}

/////////////////////////
// set previous car ID //
/////////////////////////

static void SetPrevCarType(void)
{
    VEC pos;
    MAT mat;

    CopyVec(&PLR_LocalPlayer->car.Body->Centre.Pos, &pos);
    CopyMat(&PLR_LocalPlayer->car.Body->Centre.WMatrix, &mat);

    if (ReplayMode || !ChangeCars)
        return;

    GameSettings.CarType = PrevValidCarType(GameSettings.CarType);
    SetupCar(PLR_LocalPlayer, GameSettings.CarType);
    SetCarPos(&PLR_LocalPlayer->car, &pos, &mat);
    SetCarAerialPos(&PLR_LocalPlayer->car);

    CAI_InitCarAI(PLR_LocalPlayer, PLR_LocalPlayer->car.CarType);

    if (IsMultiPlayer())
    {
        SendCarNewCar(GameSettings.CarType);
    }
}

/////////////////////
// set next car ID //
/////////////////////

static void SetNextCarType(void)
{
    VEC pos;
    MAT mat;

    CopyVec(&PLR_LocalPlayer->car.Body->Centre.Pos, &pos);
    CopyMat(&PLR_LocalPlayer->car.Body->Centre.WMatrix, &mat);

    if (ReplayMode || !ChangeCars)
        return;

    GameSettings.CarType = NextValidCarType(GameSettings.CarType);
    SetupCar(PLR_LocalPlayer, GameSettings.CarType);
    SetCarPos(&PLR_LocalPlayer->car, &pos, &mat);
    SetCarAerialPos(&PLR_LocalPlayer->car);

    CAI_InitCarAI(PLR_LocalPlayer, PLR_LocalPlayer->car.CarType);

    if (IsMultiPlayer())
    {
        SendCarNewCar(GameSettings.CarType);
    }
}

/////////////////////////////
// read carinfo file again //
/////////////////////////////

static void ReadCarInfoAgain(void)
{
    //if (!ReadAllCarInfo(CarInfoFile)) {
    if (!ReadAllCarInfoMultiple()) {
        g_bQuitGame = TRUE;
        return;
    }
    SetAllCarCoMs();
    CalcCarStats();
    SetAllCarSelect();
}

////////////////////////////
// set next follow camera //
////////////////////////////

static void SetNextCameraFollow(void)
{
    long subType;
    if (CAM_MainCamera->Type != CAM_FOLLOW) {
        subType = 0;
    } else {
        subType = ++(CAM_MainCamera->SubType);
        if (subType >= CAM_FOLLOW_NTYPES) subType = 0;
    }
    SetCameraFollow(CAM_MainCamera, PLR_LocalPlayer->ownobj, subType);
}

//////////////////////////////
// set next attached camera //
//////////////////////////////

static void SetNextCameraAttached(void)
{
    long subType;
    if (CAM_MainCamera->Type != CAM_ATTACHED) {
        subType = 0;
    } else {
        subType = CAM_MainCamera->SubType + 1;
        if (subType >= CAM_ATTACHED_NTYPES) subType = 0;
    }
    SetCameraAttached(CAM_MainCamera, PLR_LocalPlayer->ownobj, subType);
}

/////////////////////////////
// set next freedom camera //
/////////////////////////////

static void SetNextCameraFreedom(void)
{
    if (Version != VERSION_DEV && !(AllowAllCameras))
        return;

    if (GameSettings.GameType == GAMETYPE_BATTLE)
        return;

    if (CAM_MainCamera->Type != CAM_FREEDOM) {
        SetCameraFreedom(CAM_MainCamera, NULL, 0);
    } else {
        if (CAM_MainCamera->Object == NULL) {
            SetCameraFreedom(CAM_MainCamera, PLR_LocalPlayer->ownobj, 0);
        } else {
            SetCameraFreedom(CAM_MainCamera, NULL, 0);
        }
    }
}

///////////////////////////
// set next local camera //
///////////////////////////

static void SetNextLocalCamera(void)
{
    if (!gAllowCameraChange || PLR_LocalPlayer->RaceFinishTime)
        return;

    if (CAM_MainCamera->Type == CAM_FOLLOW)
    {
        CAM_MainCamera->SubType++;
        if (CAM_MainCamera->SubType <= CAM_FOLLOW_CLOSE)
            SetCameraFollow(CAM_MainCamera, PLR_LocalPlayer->ownobj, CAM_MainCamera->SubType);
        else
            SetCameraAttached(CAM_MainCamera, PLR_LocalPlayer->ownobj, CAM_ATTACHED_INCAR);
    }
    else
    {
        SetCameraFollow(CAM_MainCamera, PLR_LocalPlayer->ownobj, CAM_FOLLOW_BEHIND);
    }
}

//////////////////////
// toggle rear view //
//////////////////////

static void ToggleRearView(void)
{

// toggle

    GameSettings.DrawRearView = !GameSettings.DrawRearView;

    if (GameSettings.DrawRearView)
    {
        CAM_RearCamera = AddCamera(96, 32, 144, 108, CAMERA_FLAG_SECONDARY);
        SetCameraFollow(CAM_RearCamera, PLR_LocalPlayer->ownobj, CAM_FOLLOW_FRONT);
    }
    else
    {
        if (CAM_RearCamera != NULL)
        {
            RemoveCamera(CAM_RearCamera);
            CAM_RearCamera = NULL;
        }
    }
}

////////////////////////
// toggle follow view //
////////////////////////

static void ToggleFollowView(void)
{

// not if battle tag

    if (GameSettings.GameType == GAMETYPE_BATTLE && !PLR_LocalPlayer->RaceFinishTime)
        return;

// toggle

    GameSettings.DrawFollowView = !GameSettings.DrawFollowView;

    if (GameSettings.DrawFollowView)
    {
        CAM_OtherCamera = AddCamera(396, 32, 144, 108, CAMERA_FLAG_SECONDARY);
        CAM_OtherCamera->Object = PLR_LocalPlayer->ownobj;
        CycleFollowView();
//      SetCameraFollow(CAM_OtherCamera, PLR_LocalPlayer->ownobj, CAM_FOLLOW_CLOSE);
    }
    else
    {
        if (CAM_OtherCamera != NULL)
        {
            RemoveCamera(CAM_OtherCamera);
            CAM_OtherCamera = NULL;
        }
    }
}

///////////////////////
// cycle follow view //
///////////////////////

static void CycleFollowView(void)
{
    PLAYER *player;

    if (!CAM_OtherCamera)
        return;

    for (player = CAM_OtherCamera->Object->player->next ; player ; player = player->next)
    {
        if (player->type != PLAYER_LOCAL && player->type != PLAYER_NONE)
            break;
    }

    if (!player)
    {
        for (player = PLR_PlayerHead ; player ; player = player->next)
        {
            if (player->type != PLAYER_LOCAL && player->type != PLAYER_NONE)
                break;
        }
    }

    if ((player != NULL) && ((player->type != PLAYER_GHOST) || GHO_ShowGhost))
    {
        SetCameraRail(CAM_OtherCamera, player->ownobj, CAM_RAIL_DYNAMIC_MONO);
    }
    else
    {
        RemoveCamera(CAM_OtherCamera);
        CAM_OtherCamera = NULL;
    }
}

/////////////////////////////
// set camera to edit mode //
/////////////////////////////

static void SetCameraEdit(void)
{
}

//////////////////////////
// set next rail camera //
//////////////////////////

static void SetNextCameraRail(void)
{
    //if (Version != VERSION_DEV)
    //  return;
    if ((!AllowAllCameras) && (Version != VERSION_DEV))
        return;

    long subType;
    if (CAM_MainCamera->Type != CAM_RAIL) {
        subType = 0;
    } else {
        subType = CAM_MainCamera->SubType + 1;
        if (subType == CAM_RAIL_NTYPES) subType = 0;
    }
    SetCameraRail(CAM_MainCamera, PLR_LocalPlayer->ownobj, subType);
}

//////////////////////
// toggle autobrake //
//////////////////////

static void ToggleAutobrake(void)
{
    GameSettings.AutoBrake = !GameSettings.AutoBrake;
}

/////////////////////
// clear ghost car //
/////////////////////

static void ClearGhostCar(void)
{
    ClearBestGhostData();
    //InitBestGhostData();
}

///////////////////////
// restart ghost car //
///////////////////////

static void RestartGhostCar(void)
{
    InitBestGhostData();
}

/////////////////
// add new car //
/////////////////

static void AddNewCar(void)
{
    PLR_CreatePlayer(PLAYER_CPU, CTRL_TYPE_CPU_AI, 0, &CAM_MainCamera->WPos, &IdentityMatrix);
}

//////////////////////////////
// take control of next car //
//////////////////////////////

static void ControlNextCar(void)
{
    PLAYER *newPlayer;

    for (newPlayer = PLR_LocalPlayer->next; newPlayer != NULL; newPlayer = newPlayer->next) {
        if (newPlayer->type == PLAYER_CPU || newPlayer->type == PLAYER_LOCAL) {
            break;
        }
    }
    if (newPlayer == NULL) {
        for (newPlayer = PLR_PlayerHead; newPlayer != NULL; newPlayer = newPlayer->next) {
            if (newPlayer->type == PLAYER_CPU || newPlayer->type == PLAYER_LOCAL) {
                break;
            }
        }
    }
    if (newPlayer != NULL) {
        if (Keys[DIK_LSHIFT]) {
            PLR_KillPlayer(PLR_LocalPlayer);
        } else {
            PLR_SetPlayerType(PLR_LocalPlayer, newPlayer->type);
        }
        PLR_SetPlayerType(newPlayer, PLAYER_LOCAL);
        PLR_LocalPlayer = newPlayer;
        CAM_MainCamera->Object = PLR_LocalPlayer->ownobj;
    }
}

///////////////////////
// cycle speed units //
///////////////////////

static void CycleSpeedUnits(void)
{
    SpeedUnits++;
    if (SpeedUnits == SPEED_NTYPES) SpeedUnits = 0;
}

/////////////////////
// toggle car axis //
/////////////////////

static void ToggleCarAxisVisible(void)
{
    CAR_DrawCarAxes = !CAR_DrawCarAxes;
}

////////////////////
// dec car toe in //
////////////////////

static void DecCarFrontToeIn(void)
{
    PLR_LocalPlayer->car.Wheel[0].ToeIn -= 0.005f;
    PLR_LocalPlayer->car.Wheel[1].ToeIn += 0.005f;
}

static void DecCarRearToeIn(void)
{
    PLR_LocalPlayer->car.Wheel[2].ToeIn -= 0.005f;
    PLR_LocalPlayer->car.Wheel[3].ToeIn += 0.005f;
}

////////////////////
// inc car toe in //
////////////////////

static void IncCarFrontToeIn(void)
{
    PLR_LocalPlayer->car.Wheel[0].ToeIn += 0.005f;
    PLR_LocalPlayer->car.Wheel[1].ToeIn -= 0.005f;
}

static void IncCarRearToeIn(void)
{
    PLR_LocalPlayer->car.Wheel[2].ToeIn += 0.005f;
    PLR_LocalPlayer->car.Wheel[3].ToeIn -= 0.005f;
}

/////////////////////
// stop car motion //
/////////////////////

static void StopCarMotion(void)
{
    SetVecZero(&PLR_LocalPlayer->car.Body->AngVel);
    SetVecZero(&PLR_LocalPlayer->car.Body->Centre.Vel);
}

////////////////////////
// apply body impulse //
////////////////////////

static void ApplyImpXneg(void)
{
    VEC vImp;

    SetVec(&vImp, -impMag, 0.0f, 0.0f);
    ApplyBodyImpulse(PLR_LocalPlayer->car.Body, &vImp, &ZeroVector);
}

////////////////////////
// apply body impulse //
////////////////////////

static void ApplyImpXpos(void)
{
    VEC vImp;

    SetVec(&vImp, impMag, 0.0f, 0.0f);
    ApplyBodyImpulse(PLR_LocalPlayer->car.Body, &vImp, &ZeroVector);
}

////////////////////////
// apply body impulse //
////////////////////////

static void ApplyImpYneg(void)
{
    VEC vImp;

    SetVec(&vImp, 0.0f, -impMag, 0.0f);
    ApplyBodyImpulse(PLR_LocalPlayer->car.Body, &vImp, &ZeroVector);
}

////////////////////////
// apply body impulse //
////////////////////////

static void ApplyImpYpos(void)
{
    VEC vImp;

    SetVec(&vImp, 0.0f, impMag, 0.0f);
    ApplyBodyImpulse(PLR_LocalPlayer->car.Body, &vImp, &ZeroVector);
}

////////////////////////
// apply body impulse //
////////////////////////

static void ApplyImpZneg(void)
{
    VEC vImp;

    SetVec(&vImp, 0.0f, 0.0f, -impMag);
    ApplyBodyImpulse(PLR_LocalPlayer->car.Body, &vImp, &ZeroVector);
}

////////////////////////
// apply body impulse //
////////////////////////

static void ApplyImpZpos(void)
{
    VEC vImp;

    SetVec(&vImp, 0.0f, 0.0f, impMag);
    ApplyBodyImpulse(PLR_LocalPlayer->car.Body, &vImp, &ZeroVector);
}

///////////////////////////
// apply angular impulse //
///////////////////////////

static void ApplyAngImpXneg(void)
{
    VEC vImp, vPos;

    SetVec(&vPos, 0.0f, 0.0f, impPos);
    SetVec(&vImp, 0.0f, -impMag, 0.0f);
    ApplyBodyImpulse(PLR_LocalPlayer->car.Body, &vImp, &vPos);
    SetVec(&vPos, 0.0f, 0.0f, -impPos);
    SetVec(&vImp, 0.0f, impMag, 0.0f);
    ApplyBodyImpulse(PLR_LocalPlayer->car.Body, &vImp, &vPos);
}

///////////////////////////
// apply angular impulse //
///////////////////////////

static void ApplyAngImpXpos(void)
{
    VEC vImp, vPos;

    SetVec(&vPos, 0.0f, 0.0f, impPos);
    SetVec(&vImp, 0.0f, impMag, 0.0f);
    ApplyBodyImpulse(PLR_LocalPlayer->car.Body, &vImp, &vPos);
    SetVec(&vPos, 0.0f, 0.0f, -impPos);
    SetVec(&vImp, 0.0f, -impMag, 0.0f);
    ApplyBodyImpulse(PLR_LocalPlayer->car.Body, &vImp, &vPos);
}

///////////////////////////
// apply angular impulse //
///////////////////////////

static void ApplyAngImpYneg(void)
{
    VEC vImp, vPos;

    SetVec(&vPos, 0.0f, 0.0f, impPos);
    SetVec(&vImp, impMag, 0.0f, 0.0f);
    ApplyBodyImpulse(PLR_LocalPlayer->car.Body, &vImp, &vPos);
    SetVec(&vPos, 0.0f, 0.0f, -impPos);
    SetVec(&vImp, -impMag, 0.0f, 0.0f);
    ApplyBodyImpulse(PLR_LocalPlayer->car.Body, &vImp, &vPos);
}

///////////////////////////
// apply angular impulse //
///////////////////////////

static void ApplyAngImpYpos(void)
{
    VEC vImp, vPos;

    SetVec(&vPos, 0.0f, 0.0f, -impPos);
    SetVec(&vImp, impMag, 0.0f, 0.0f);
    ApplyBodyImpulse(PLR_LocalPlayer->car.Body, &vImp, &vPos);
    SetVec(&vPos, 0.0f, 0.0f, impPos);
    SetVec(&vImp, -impMag, 0.0f, 0.0f);
    ApplyBodyImpulse(PLR_LocalPlayer->car.Body, &vImp, &vPos);
}

///////////////////////////
// apply angular impulse //
///////////////////////////

static void ApplyAngImpZneg(void)
{
    VEC vImp, vPos;

    SetVec(&vPos, 0.0f, impPos, 0.0f);
    SetVec(&vImp, impMag, 0.0f, 0.0f);
    ApplyBodyImpulse(PLR_LocalPlayer->car.Body, &vImp, &vPos);
    SetVec(&vPos, 0.0f, -impPos, 0.0f);
    SetVec(&vImp, -impMag, 0.0f, 0.0f);
    ApplyBodyImpulse(PLR_LocalPlayer->car.Body, &vImp, &vPos);
}

///////////////////////////
// apply angular impulse //
///////////////////////////

static void ApplyAngImpZpos(void)
{
    VEC vImp, vPos;

    SetVec(&vPos, 0.0f, -impPos, 0.0f);
    SetVec(&vImp, impMag, 0.0f, 0.0f);
    ApplyBodyImpulse(PLR_LocalPlayer->car.Body, &vImp, &vPos);
    SetVec(&vPos, 0.0f, impPos, 0.0f);
    SetVec(&vImp, -impMag, 0.0f, 0.0f);
    ApplyBodyImpulse(PLR_LocalPlayer->car.Body, &vImp, &vPos);
}

/////////////////////
// reset local car //
/////////////////////

static void ResetLocalCar(void)
{
    VEC vec, vec2;
    MAT mat;

    if (Version != VERSION_DEV)
        return;

    SetVector(&vec2, 0, 0, 256);
    RotTransVector(&CAM_MainCamera->WMatrix, &CAM_MainCamera->WPos, &vec2, &vec);

    CopyVec(&CAM_MainCamera->WMatrix.mv[R], &mat.mv[R]);
    SetVector(&mat.mv[U], 0, 1, 0);
    CrossProduct(&mat.mv[R], &mat.mv[U], &mat.mv[L]);
    SetCarPos(&PLR_LocalPlayer->car, &vec, &mat);
}

////////////////////////
// grab screen as bmp //
////////////////////////

static void GrabScreen(void)
{
    static long num = 0;
    char buf[128];

    if (Version != VERSION_DEV)
        return;

    sprintf(buf, "grab%4.4ld.bmp", num++);
    SaveFrontBuffer(buf);
}

/////////////////////
// toggle car visi //
/////////////////////

static void ToggleCarVisi(void)
{
    if (Version != VERSION_DEV)
        return;

    CarVisi = !CarVisi;
}

///////////////////////////
// add all function keys //
///////////////////////////

void AddAllFunctionKeys(void)
{
    FUNCTION_KEY *f = FuncKeys;

    while (f->Key)
    {
        AddFunctionKey(f->SysKey, f->NotSysKey, f->Key, f->Func, f->NewOnly);
        f++;
    }
}

////////////////////////////////////////////////////////////////
//
// CRD_ReplayCtrlHandler:
//
////////////////////////////////////////////////////////////////

void CRD_ReplayInput(PLAYER *player)
{
    CAR_REMOTE_DATA *rem;

    Assert(player->type == PLAYER_REPLAY);

    rem = &player->car.RemoteData[player->car.NewDat];

    player->controls.dx = rem->dx;
    player->controls.dy = rem->dy;
    player->controls.digital = rem->digital;
}


////////////////////////////////////////////////////////////////
//
// CRD_RemoteCtrlHandler:
//
////////////////////////////////////////////////////////////////

void CRD_RemoteInput(PLAYER *player)
{
    CAR_REMOTE_DATA *rem;

    if (player->type == PLAYER_NONE)
    {
        player->ctrlhandler = NULL;
        return;
    }

    Assert(player->type == PLAYER_REMOTE);

    rem = &player->car.RemoteData[player->car.NewDat];

    player->controls.dx = rem->dx;
    player->controls.dy = rem->dy;
    player->controls.digital = rem->digital;
}

////////////////////////////////////////////////////////////////
//
// CRD_SetDefaultControls
//
////////////////////////////////////////////////////////////////

void CRD_SetDefaultControls()
{
    KEY key;
    if (RegistrySettings.Joystick == -1) {
        RegistrySettings.KeyLeft = DIK_LEFT;
        RegistrySettings.KeyRight = DIK_RIGHT;
        RegistrySettings.KeyFwd = DIK_UP;
        RegistrySettings.KeyBack = DIK_DOWN;
        RegistrySettings.KeyFire = DIK_LCONTROL;
        RegistrySettings.KeyReset = DIK_END;
        RegistrySettings.KeyReposition = DIK_HOME;
        RegistrySettings.KeyPause = DIK_ESCAPE;
    } else {
        key.Index = 0;
        key.Type = KEY_TYPE_AXISNEG;
        RegistrySettings.KeyLeft = *((DWORD*)&key);
        
        key.Index = 0;
        key.Type = KEY_TYPE_AXISPOS;
        RegistrySettings.KeyRight = *((DWORD*)&key);

        key.Index = 1;
        key.Type = KEY_TYPE_AXISNEG;
        RegistrySettings.KeyFwd = *((DWORD*)&key);

        key.Index = 1;
        key.Type = KEY_TYPE_AXISPOS;
        RegistrySettings.KeyBack = *((DWORD*)&key);

        key.Index = 0;
        key.Type = KEY_TYPE_BUTTON;
        RegistrySettings.KeyFire = *((DWORD*)&key);

        key.Index = 1;
        key.Type = KEY_TYPE_BUTTON;
        RegistrySettings.KeyReset = *((DWORD*)&key);

        key.Index = 2;
        key.Type = KEY_TYPE_BUTTON;
        RegistrySettings.KeyReposition = *((DWORD*)&key);

        key.Index = 3;
        key.Type = KEY_TYPE_BUTTON;
        RegistrySettings.KeyPause = *((DWORD*)&key);
    }

    RegistrySettings.KeyHonka = DIK_LSHIFT;
    RegistrySettings.KeyHandbrake = DIK_SPACE;
    RegistrySettings.KeySelWeapon = DIK_RSHIFT;
    RegistrySettings.KeySelCamera = 0xffffffff;
    RegistrySettings.KeyFullBrake = 0xffffffff;

    KeyTable[KEY_LEFT] = *(KEY*)&RegistrySettings.KeyLeft;
    KeyTable[KEY_RIGHT] = *(KEY*)&RegistrySettings.KeyRight;
    KeyTable[KEY_FWD] = *(KEY*)&RegistrySettings.KeyFwd;
    KeyTable[KEY_BACK] = *(KEY*)&RegistrySettings.KeyBack;
    KeyTable[KEY_FIRE] = *(KEY*)&RegistrySettings.KeyFire;
    KeyTable[KEY_RESET] = *(KEY*)&RegistrySettings.KeyReset;
    KeyTable[KEY_REPOSITION] = *(KEY*)&RegistrySettings.KeyReposition;
    KeyTable[KEY_HONKA] = *(KEY*)&RegistrySettings.KeyHonka;
//  KeyTable[KEY_HANDBRAKE] = *(KEY*)&RegistrySettings.KeyHandbrake;
    KeyTable[KEY_REARVIEW] = *(KEY*)&RegistrySettings.KeyHandbrake;
    KeyTable[KEY_SELWEAPON] = *(KEY*)&RegistrySettings.KeySelWeapon;
    KeyTable[KEY_SELCAMERAMODE] = *(KEY*)&RegistrySettings.KeySelCamera;
    KeyTable[KEY_FULLBRAKE] = *(KEY*)&RegistrySettings.KeyFullBrake;
    KeyTable[KEY_PAUSE] = *(KEY*)&RegistrySettings.KeyPause;

}


void CRD_SetControlsFromRegistry()
{
    KeyTable[KEY_LEFT] = *(KEY*)&RegistrySettings.KeyLeft;
    KeyTable[KEY_RIGHT] = *(KEY*)&RegistrySettings.KeyRight;
    KeyTable[KEY_FWD] = *(KEY*)&RegistrySettings.KeyFwd;
    KeyTable[KEY_BACK] = *(KEY*)&RegistrySettings.KeyBack;
    KeyTable[KEY_FIRE] = *(KEY*)&RegistrySettings.KeyFire;
    KeyTable[KEY_RESET] = *(KEY*)&RegistrySettings.KeyReset;
    KeyTable[KEY_REPOSITION] = *(KEY*)&RegistrySettings.KeyReposition;
    KeyTable[KEY_HONKA] = *(KEY*)&RegistrySettings.KeyHonka;
//  KeyTable[KEY_HANDBRAKE] = *(KEY*)&RegistrySettings.KeyHandbrake;
    KeyTable[KEY_REARVIEW] = *(KEY*)&RegistrySettings.KeyHandbrake;
    KeyTable[KEY_SELWEAPON] = *(KEY*)&RegistrySettings.KeySelWeapon;
    KeyTable[KEY_SELCAMERAMODE] = *(KEY*)&RegistrySettings.KeySelCamera;
    KeyTable[KEY_FULLBRAKE] = *(KEY*)&RegistrySettings.KeyFullBrake;
    KeyTable[KEY_PAUSE] = *(KEY*)&RegistrySettings.KeyPause;
}