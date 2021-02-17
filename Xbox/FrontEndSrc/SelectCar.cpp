//-----------------------------------------------------------------------------
// File: SelectCar.cpp
//
// Desc: 
//
// Re-Volt (Generic) Copyright (c) Probe Entertainment 1998
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"
#include "obj_init.h"
#include "menu2.h"
#include "MenuDraw.h"   // DrawSpruBox
#include "LevelLoad.h"  // GAMETYPE
#include "cheats.h"       // AllCars
#include "GetPlayerName.h"

// re-volt specific
#include "menutext.h"   // re-volt strings

// menus
#include "SelectCup.h"
#include "RaceSummary.h"
#include "WaitingRoom.h"
#include "SelectTrack.h"

// temporary includes?
#include "titlescreen.h"  // TITLESCREEN_CAMPOS_ & g_bTitleScreenRunDemo

#include "player.h"
#include "timing.h"
#include "drawObj.h"

static void SelectCarSelectCar(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);

static long InitCarBox(OBJECT *obj, long *flags);

static void DrawCarName(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex);


extern BOOL g_bUsingNewFrontEnd;


static PLAYER*      g_pCarSelectPlayer = NULL;

static VEC          g_vCarBoxSelectPos = {0,0,0};
static VEC          g_vCarBoxOpenPos   = {0,0,0};
static QUATERNION   g_qCarBoxOpenQuat  = {0,0,0,1};
static MAT          g_matCarDropMatrix;


// Register the CarBox object init data
REGISTER_OBJECT( OBJECT_TYPE_CARBOX, InitCarBox, sizeof(OBJECT_CARBOX_OBJ) );


////////////////////////////////////////////////////////////////
//
// Select Car Menu
//
////////////////////////////////////////////////////////////////

// Data
bool gCarChosen = FALSE;                    // Car has been selected (with return)
bool gCarTaken = FALSE;                     // Car has been dropped, and we are ready to move to next menu

#define MENU_CARSELECT_XPOS             Real(100)
#define MENU_CARSELECT_YPOS             Real(150)

void CreateSelectCarMenu(MENU_HEADER *menuHeader, MENU *menu);
void CreateSelectCarDummyMenu(MENU_HEADER *menuHeader, MENU *menu);

void SelectCarPrevCar(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
void SelectCarNextCar(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
void SelectCarUpCar(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
void SelectCarDownCar(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
void SelectCarSelectCar(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
void SelectCarChooseCar(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);


// Menu
MENU Menu_SelectCar = 
{
    TEXT_CHOOSECAR,
    MENU_IMAGE_CARSELECT,                   // Menu title bar image enum
    TITLESCREEN_CAMPOS_CAR_SELECT,                   // Camera pos index
    MENU_DEFAULT,                           // Menu type
    SPRU_COL_RANDOM,                        // Spru colour
    CreateSelectCarMenu,                    // Create menu function
    MENU_CARSELECT_XPOS,                    // X coord
    MENU_CARSELECT_YPOS,                    // Y Coord
    0,                                      // Current Item index
    NULL,                                   // Parent Menu (OverWritten in Create)
};

MENU Menu_SelectCarDummy = 
{
    TEXT_CHOOSECAR,
    MENU_IMAGE_CARSELECT,                   // Camera pos index
    TITLESCREEN_CAMPOS_CAR_SELECTED,                 // Camera pos index
    MENU_DEFAULT,                           // Menu type
    SPRU_COL_KEEP,                          // Spru colour
    CreateSelectCarDummyMenu,               // Create menu function
    MENU_CARSELECT_XPOS,                    // X coord
    MENU_CARSELECT_YPOS,                    // Y Coord
    0,                                      // Current Item index
    NULL,                                   // Parent Menu (OverWritten in Create)
};


// Car Select Menu Item
MENU_ITEM MenuItem_SelectCar =
 {
    TEXT_NONE,                              // Text label index

    0,                                      // Space needed to draw item data
    NULL,                                   // Data
    DrawCarName,                            // Draw Function

	NULL,								    // Input handler
	0,                                      // Flags

    NULL,//SelectCarUpCar,                          // Up Action
    NULL,//SelectCarDownCar,                        // Down Action
    SelectCarPrevCar,                       // Left Action
    SelectCarNextCar,                       // Right Action
    MenuGoBack,                             // Back Action
    SelectCarChooseCar,
};

// Car Select Menu Item
MENU_ITEM MenuItem_SelectCarDummy = {
    TEXT_NONE,                              // Text label index

    0,                                      // Space needed to draw item data
    NULL,                                   // Data
    NULL,                                   // Draw Function

	NULL,								    // Input handler
	0,                                      // Flags

    NULL,                                   // Up Action
    NULL,                                   // Down Action
    NULL,                                   // Left Action
    NULL,                                   // Right Action
    MenuGoBack,                             // Back Action
    SelectCarSelectCar,                     // Forward Action
};

// Create
void CreateSelectCarMenu(MENU_HEADER *menuHeader, MENU *menu)
{

    gCarChosen = FALSE;

// re-call car select func if 'AllCars' true

    if (AllCars || AllowUFO)
    {
        SetAllCarSelect();
    }

    GameSettings.CarType = g_TitleScreenData.iCarNum[g_TitleScreenData.iCurrentPlayer];
    if (!CarInfo[GameSettings.CarType].Selectable) {
        GameSettings.CarType = g_TitleScreenData.iCarNum[g_TitleScreenData.iCurrentPlayer] = 0;
    }


    AddMenuItem(menuHeader, menu, &MenuItem_SelectCar);

    Menu_SelectCar.CamPosIndex = TITLESCREEN_CAMPOS_CAR_SELECT;
    MenuItem_SelectCar.InputAction[MENU_INPUT_SELECT] = SelectCarChooseCar;
}

void CreateSelectCarDummyMenu(MENU_HEADER *menuHeader, MENU *menu)
{
    gChangeParentMenu = FALSE;
    AddMenuItem(menuHeader, menu, &MenuItem_SelectCarDummy);
}

// Utility functions

void SelectCarPrevCar(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    if (gCarChosen) return;
    // Set the car number for this player
    GameSettings.CarType = g_TitleScreenData.iCarNum[g_TitleScreenData.iCurrentPlayer] = PrevValidCarType(g_TitleScreenData.iCarNum[g_TitleScreenData.iCurrentPlayer]);

}

void SelectCarNextCar(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    if (gCarChosen) return;
    // Set the car number for this player
    GameSettings.CarType = g_TitleScreenData.iCarNum[g_TitleScreenData.iCurrentPlayer] = NextValidCarType(g_TitleScreenData.iCarNum[g_TitleScreenData.iCurrentPlayer]);

}

void SelectCarSelectCar(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    // Move to next player
    g_TitleScreenData.iCurrentPlayer++;

	if( g_bUsingNewFrontEnd )
	{
		// Kill the menu system to return to the caller
		SetNextMenu( menuHeader, NULL );
	}
	else
	{
		// Choose next menu according to game type
		if (g_TitleScreenData.iCurrentPlayer < g_TitleScreenData.numberOfPlayers) 
		{
			// More players to do...
			menuItem->pData = &Menu_PlayerName;
			MenuGoForward(menuHeader, menu, menuItem);
		} 
		else 
		{
			// Set to highest player
			g_TitleScreenData.iCurrentPlayer = g_TitleScreenData.numberOfPlayers - 1;

			// Go to next menu
			switch (GameSettings.GameType) 
			{
				case GAMETYPE_CHAMPIONSHIP:
					menuItem->pData = &Menu_CupSelect;                   // Cup select
					MenuGoForward(menuHeader, menu, menuItem);
					break;

				case GAMETYPE_TRAINING:
					menuItem->pData = &Menu_Overview;                    // Overview
					MenuGoForward(menuHeader, menu, menuItem);
					break;

				case GAMETYPE_MULTI:                                    // waiting room if client
				case GAMETYPE_BATTLE:
					SetPlayerData();

					if (GameSettings.MultiType == MULTITYPE_CLIENT)
					{
						menuItem->pData = &Menu_WaitingRoom;
						MenuGoForward(menuHeader, menu, menuItem);
					}
					else
					{
						if (GameSettings.RandomTrack)
						{
							menuItem->pData = &Menu_WaitingRoom;
						}
						else
						{
							menuItem->pData = &Menu_SelectTrack;
						}

						MenuGoForward(menuHeader, menu, menuItem);
					}
					break;

				default:
					if (GameSettings.RandomTrack)
						menuItem->pData = &Menu_Overview;
					else
						menuItem->pData = &Menu_SelectTrack;

					MenuGoForward(menuHeader, menu, menuItem);
					break;
			}
		}
	}
}

void SelectCarChooseCar(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    gCarChosen = TRUE;
    SetNextMenu(menuHeader, &Menu_SelectCarDummy);
    //MenuItem_SelectCar.InputAction[MENU_INPUT_SELECT] = SelectCarSelectCar;
}



////////////////////////////////////////////////////////////////
//
// Draw Car Box
//
////////////////////////////////////////////////////////////////

VEC CarBoxFaceCorner[4] = {
    {-32.7f, -48.9f, -105.5f},
    {77.3f, -48.9f, -105.5f},
    {77.3f, 61.1f, -105.5f},
    {-32.7f, 61.1f, -105.5f},
};

void DrawCarBoxFace(OBJECT *obj)
{
    REAL xOff, yOff;
    OBJECT_CARBOX_OBJ *box = (OBJECT_CARBOX_OBJ*)obj->Data;

    if (box->CarType == -1)
    {
        // tpage offsets
        xOff = 0.0f /256.0f;
        yOff = (256.0f - 85.0f) /256.0f;

        // TPage
        box->FacePoly->Tpage = TPAGE_WORLD_START + 6;
    }
    else if (box->CarType <= CARID_AMW) 
    {
        // tpage offsets
        xOff = 1.0f /256.0f + (box->CarType % 3) * 85.0f / 256.0f;
        yOff = 1.0f /256.0f + ((box->CarType % 9) / 3) * 85.0f / 256.0f;

        // TPage
        box->FacePoly->Tpage = (short)(TPAGE_FX1 + (box->CarType / 9));
    }
    else if (box->CarType == CARID_PANGA)
    {
        // tpage offsets
        xOff = 171.0f /256.0f;
        yOff = 73.0f /256.0f;

        // TPage
        box->FacePoly->Tpage = TPAGE_WORLD_START + 4;
    }

    // UVs
    box->FacePoly->tu0 = xOff + 84.0f/256.0f;       box->FacePoly->tv0 = yOff + 0.0f/256.0f;
    box->FacePoly->tu1 = xOff + 84.0f/256.0f;       box->FacePoly->tv1 = yOff + 84.0f/256.0f;
    box->FacePoly->tu2 = xOff + 0.0f/256.0f;        box->FacePoly->tv2 = yOff + 84.0f/256.0f;
    box->FacePoly->tu3 = xOff + 0.0f/256.0f;        box->FacePoly->tv3 = yOff + 0.0f/256.0f;

}

void DrawCarBox(OBJECT *obj)
{
    OBJECT_CARBOX_OBJ *box = (OBJECT_CARBOX_OBJ*)obj->Data;

    if ((box->CarType == -1) || CarInfo[box->CarType].Selectable) {
        DrawCarBoxFace(obj);
        RenderObjectModel(&obj->body.Centre.WMatrix, &obj->body.Centre.Pos, &LevelModel[box->BoxModel].Model, obj->EnvRGB, obj->EnvOffsetX, obj->EnvOffsetY, obj->EnvScale, obj->renderflag, FALSE);
    } else {
        RenderObjectModel(&obj->body.Centre.WMatrix, &obj->body.Centre.Pos, &LevelModel[box->PlainBoxModel].Model, obj->EnvRGB, obj->EnvOffsetX, obj->EnvOffsetY, obj->EnvScale, obj->renderflag, FALSE);
    }
}




//-----------------------------------------------------------------------------
// Name: SetCarBoxDestination()
// Desc: 
//-----------------------------------------------------------------------------
void SetCarBoxDestination()
{
	// mwetzel - moved to here 
	// Initialize the car select player
	g_pCarSelectPlayer = NULL;

    MAT mat;

    // Car box close-to-camera (highlighted) position
    QuatToMat(&g_CameraPositions[TITLESCREEN_CAMPOS_CAR_SELECT].Quat, &mat);
    VecPlusScalarVec(&g_CameraPositions[TITLESCREEN_CAMPOS_CAR_SELECT].vEye, 600.0f, &mat.mv[L], &g_vCarBoxSelectPos);
    VecPlusEqScalarVec(&g_vCarBoxSelectPos, 150.0f, &mat.mv[R]);
    VecPlusEqScalarVec(&g_vCarBoxSelectPos, -50.0f, &mat.mv[U]);

    // Carbox opening orientation
    //SetVec(&mat.mv[U], ZERO, ZERO, ONE);
    mat.m[UX] = -mat.m[LX];
    mat.m[UY] = ZERO;
    mat.m[UZ] = -mat.m[LZ];
    NormalizeVec(&mat.mv[U]);
    VecCrossVec(&mat.mv[R], &mat.mv[U], &mat.mv[L]);
    MatToQuat(&mat, &g_qCarBoxOpenQuat);

    // Car's drop matrix
    SwapVecs(&mat.mv[U], &mat.mv[L]);
    NegateVec(&mat.mv[L]);
    SwapVecs(&mat.mv[R], &mat.mv[L]);
    NegateVec(&mat.mv[L]);
    CopyMat(&mat, &g_matCarDropMatrix);

    // Carbox opening position
    //SetVec(&g_vCarBoxOpenPos, 
    //  g_vCarBoxSelectPos.v[X] + MulScalar(230.0f, mat.m[LX]),
    //  ZERO,//g_vCarBoxSelectPos.v[Y],
    //  g_vCarBoxSelectPos.v[Z] + MulScalar(230.0f, mat.m[LZ]));
    QuatToMat(&g_CameraPositions[TITLESCREEN_CAMPOS_CAR_SELECTED].Quat, &mat);
    SetVec(&g_vCarBoxOpenPos, 
        g_CameraPositions[TITLESCREEN_CAMPOS_CAR_SELECTED].vEye.v[X] + MulScalar(230.0f, mat.m[LX]),
        ZERO,//g_vCarBoxSelectPos.v[Y],
        g_CameraPositions[TITLESCREEN_CAMPOS_CAR_SELECTED].vEye.v[Z] + MulScalar(230.0f, mat.m[LZ]));
}




//-----------------------------------------------------------------------------
// Name: InterpCarBoxPos()
// Desc: 
//-----------------------------------------------------------------------------
void InterpCarBoxPos(VEC *pos, VEC *dest, FLOAT xScale, FLOAT yScale, FLOAT zScale)
{
    FLOAT timeScale = TimeStep * 50;

    pos->v[X] += MulScalar3(xScale, timeScale, (dest->v[X] - pos->v[X]));
    pos->v[Y] += MulScalar3(yScale, timeScale, (dest->v[Y] - pos->v[Y]));
    pos->v[Z] += MulScalar3(zScale, timeScale, (dest->v[Z] - pos->v[Z]));
}




//-----------------------------------------------------------------------------
// Name: InterpCarBoxQuat()
// Desc: 
//-----------------------------------------------------------------------------
void InterpCarBoxQuat(QUATERNION *quat, QUATERNION *destQuat, FLOAT scale)
{
    scale = ONE - scale;
    scale = MulScalar3(scale, TimeStep, 30);
    if (scale > ONE) scale = ONE;
    ScalarQuatPlusScalarQuat(ONE - scale, quat, scale, destQuat, quat);
}




//  CARBOX_STACKED = 0,
//  CARBOX_SELECTED,
//  CARBOX_CHOOSABLE,
//  CARBOX_OPENING,
//  CARBOX_OPEN,

//-----------------------------------------------------------------------------
// Name: CarBoxHandler()
// Desc: 
//-----------------------------------------------------------------------------
void CarBoxHandler(OBJECT *obj)
{
    VEC dR;
    OBJECT_CARBOX_OBJ *box = (OBJECT_CARBOX_OBJ*)obj->Data;

    Assert(box->State < CARBOX_NSTATES);
    // Only deal with these when in car select menu
    /*if ((gMenuHeader.Menu != &Menu_SelectCar) && (gMenuHeader.Menu != &Menu_SelectCarDummy)){
        if (!box->AtHome) {
            box->State = CARBOX_STACKED;
            box->Timer = ZERO;
            CopyVec(&box->HomePos, &obj->body.Centre.Pos);
            CopyQuat(&box->HomeQuat, &obj->body.Centre.Quat);
            QuatToMat(&obj->body.Centre.Quat, &obj->body.Centre.WMatrix);
            box->AtHome = TRUE;
        }
        return;
    }*/

    // See if box has become unselected
    if ((gMenuHeader.pMenu != &Menu_SelectCar) && (gMenuHeader.pMenu != &Menu_SelectCarDummy)) {
        box->State = CARBOX_STACKED;
        box->Timer = ZERO;
    } else if ((box->CarType == -1) && (g_TitleScreenData.iCarNum[g_TitleScreenData.iCurrentPlayer] > CARID_PANGA)) {

    } else if (g_TitleScreenData.iCarNum[g_TitleScreenData.iCurrentPlayer] != box->CarType) {
        box->State = CARBOX_STACKED;
        box->Timer = ZERO;
    }

    // Housekeeping
    box->Timer += TimeStep;

    // Handle state
    switch (box->State) {

    case CARBOX_STACKED:
        // Move box back to default position
        if (!box->AtHome) {
            VecMinusVec(&obj->body.Centre.Pos, &box->HomePos, &dR);
            if ((abs(dR.v[X]) < 1) && (abs(dR.v[Y]) < 1) && (abs(dR.v[Z]) < 1)) {
                CopyVec(&box->HomePos, &obj->body.Centre.Pos);
                CopyQuat(&box->HomeQuat, &obj->body.Centre.Quat);
                box->AtHome = TRUE;
            } else if (box->Timer > 1.5f) {
                CopyVec(&box->HomePos, &obj->body.Centre.Pos);
                CopyQuat(&box->HomeQuat, &obj->body.Centre.Quat);
                box->AtHome = TRUE;
            } else {
                InterpCarBoxPos(&obj->body.Centre.Pos, &box->HomePos, 0.2f, 0.3f, 0.2f);
                InterpCarBoxQuat(&obj->body.Centre.Quat, &box->HomeQuat, 0.85f);
            }
        }

        // See if it has become selected
        if (!gCarChosen && 
            ((g_TitleScreenData.iCarNum[g_TitleScreenData.iCurrentPlayer] == box->CarType) || 
             (g_TitleScreenData.iCarNum[g_TitleScreenData.iCurrentPlayer] > 27 && box->CarType == -1)))
        {
            if (gMenuHeader.pMenu == &Menu_SelectCar)
            {
                box->State = CARBOX_SELECTED;
                box->Timer = ZERO;
                box->AtHome = FALSE;
                PlaySfx(SFX_BOXENTRY, SFX_MAX_VOL, SFX_CENTRE_PAN, SFX_SAMPLE_RATE, 0x7fffffff);
#pragma message( "jharding needs to look into porting this." )
            }
        }

        break;

    case CARBOX_SELECTED:
        // Move box in front of camera
        InterpCarBoxPos(&obj->body.Centre.Pos, &g_vCarBoxSelectPos, 0.2f, 0.1f, 0.2f);
        InterpCarBoxQuat(&obj->body.Centre.Quat, &g_pTitleScreenCamera->m_pCamera->Quat, 0.9f);

        // Collide with other boxes
        FLOAT dRLenSq, dRLen, depth;
        OBJECT *otherBoxObj;
        OBJECT_CARBOX_OBJ *otherBox;

        for (otherBoxObj = NextObjectOfType(OBJ_ObjectHead, OBJECT_TYPE_CARBOX); otherBoxObj != NULL; otherBoxObj = NextObjectOfType(otherBoxObj->next, OBJECT_TYPE_CARBOX)) {
            if (otherBoxObj == obj) continue;
            otherBox = (OBJECT_CARBOX_OBJ*)otherBoxObj->Data;
            if (otherBox->AtHome) continue;

            VecMinusVec(&obj->body.Centre.Pos, &otherBoxObj->body.Centre.Pos, &dR);
            dRLenSq = VecDotVec(&dR, &dR);
            if (dRLenSq < 250*250 && dRLenSq > SMALL_REAL) {
                dRLen = sqrtf(dRLenSq);
                depth = (250 - dRLen)/2;
                if (abs(otherBox->CarType - box->CarType) < 2) {
                    dR.v[Y] = ZERO;
                } else {
                    dR.v[Y] /= 5;
                }
                VecPlusEqScalarVec(&otherBoxObj->body.Centre.Pos, -depth/dRLen, &dR);
                VecPlusEqScalarVec(&obj->body.Centre.Pos, depth/dRLen, &dR);
            }
        }

        // Has car been selected?
        if (gCarChosen) {
            box->State = CARBOX_OPENING;
            box->Timer = ZERO;
            ConstrainQuat2(&g_qCarBoxOpenQuat, &obj->body.Centre.Quat);
            Menu_SelectCar.CamPosIndex = TITLESCREEN_CAMPOS_CAR_SELECTED;
        }
        break;

    case CARBOX_OPENING:
        // Move box to drop position
        InterpCarBoxPos(&obj->body.Centre.Pos, &g_vCarBoxOpenPos, 0.2f, 0.1f, 0.2f);
        InterpCarBoxQuat(&obj->body.Centre.Quat, &g_qCarBoxOpenQuat, 0.9f);
        if (g_pCarSelectPlayer) {
            g_pCarSelectPlayer->car.AerialTimer -= TimeStep * 2;
            if (g_pCarSelectPlayer->car.AerialTimer < 0.2f) g_pCarSelectPlayer->car.AerialTimer = 0.2f;
        }
        if (box->Timer > 1.0f) {
            VEC carPos;
            if (g_pCarSelectPlayer != NULL) {
                PLR_KillPlayer(g_pCarSelectPlayer);
                g_pCarSelectPlayer = NULL;
            }
            carPos.v[X] = obj->body.Centre.Pos.v[X] + 20.0f;
            carPos.v[Y] = obj->body.Centre.Pos.v[Y] - 10.0f;
            carPos.v[Z] = obj->body.Centre.Pos.v[Z] + 0.0f;
            g_pCarSelectPlayer = PLR_CreatePlayer(PLAYER_FRONTEND, CTRL_TYPE_NONE, g_TitleScreenData.iCarNum[g_TitleScreenData.iCurrentPlayer], &carPos, &g_matCarDropMatrix);
            box->State = CARBOX_OPEN;
            box->Timer = ZERO;

        }
        break;

    case CARBOX_OPEN:
        // Move box back to default position        
        if (box->Timer > 1.3f) {
            if (gMenuHeader.pMenu == &Menu_SelectCarDummy) {
                SelectCarSelectCar(&gMenuHeader, gMenuHeader.pMenu, gMenuHeader.pMenuItem[gMenuHeader.pMenu->CurrentItemIndex]);
                box->State = CARBOX_STACKED;
            } else {

                box->State = CARBOX_SELECTED;
            }   
        }
        if (box->Timer > 0.3f) {
            // Send back to stack
            InterpCarBoxPos(&obj->body.Centre.Pos, &box->HomePos, 0.1f, 0.2f, 0.1f);
            InterpCarBoxQuat(&obj->body.Centre.Quat, &box->HomeQuat, 0.9f);
        }
        break;

    }

    QuatToMat(&obj->body.Centre.Quat, &obj->body.Centre.WMatrix);
}




//-----------------------------------------------------------------------------
// Name:
// Desc: Init Car Box
//-----------------------------------------------------------------------------
long InitCarBox(OBJECT *obj, long *flags)
{
    MODEL *model;
    int iPoly;
    OBJECT_CARBOX_OBJ *box = (OBJECT_CARBOX_OBJ*)obj->Data;

    // set default model
    box->BoxModel = LoadOneLevelModel(LEVEL_MODEL_CARBOX, TRUE, obj->renderflag, 0);
    box->PlainBoxModel = LoadOneLevelModel(LEVEL_MODEL_PLAINBOX, TRUE, obj->renderflag, 0);
    obj->renderflag.envmap = FALSE;
    obj->renderflag.envgood = FALSE;
    obj->EnvRGB = 0xffffff;

    // find the face poly
    model = &LevelModel[box->BoxModel].Model;   
    for (iPoly = 0; iPoly < model->PolyNum; iPoly++) {
        box->FacePoly = &model->PolyPtr[iPoly];
        if (box->FacePoly->Type & POLY_ENV) {
            break;
        }
    }

    // AI handler
    obj->aihandler = (AI_HANDLER)CarBoxHandler;

    // Render Handler
    obj->renderhandler = (RENDER_HANDLER)DrawCarBox;

    // Collision - does in its handler
    obj->CollType = COLL_TYPE_NONE;
    obj->body.CollSkin.AllowObjColls = FALSE;
    obj->body.CollSkin.AllowWorldColls = FALSE;

    // Init data
    box->CarType = flags[0];
    box->State = CARBOX_STACKED;
    box->Timer = ZERO;
    box->AtHome = TRUE;
    CopyVec(&obj->body.Centre.Pos, &box->HomePos);
    CopyQuat(&obj->body.Centre.Quat, &box->HomeQuat);

    Assert(box->CarType < CARID_NTYPES);

    return TRUE;
}

/////////////////////////////////////////////////////////////////////
//
// Draw Car Name
//
/////////////////////////////////////////////////////////////////////
/*char *CarClassString[] = {
    "Electric",
    "Glow",
    "Special",
};

char *CarRatingString[] = {
    "Rookie",
    "Amateur",
    "Advanced",
    "Semi-Pro",
    "Pro",
};

char *CarTransString[] = {
    "4WD",
    "RWD",
    "FWD",
};*/
    
#define MIN_CAR_TOPEND      MPH2OGU_SPEED * Real(26)
#define MAX_CAR_TOPEND      MPH2OGU_SPEED * Real(42)
#define MIN_CAR_WEIGHT      Real(0.6)
#define MAX_CAR_WEIGHT      Real(2.8)
#define MIN_CAR_ACC         TO_TIME(Real(4))
#define MAX_CAR_ACC         TO_TIME(Real(12))
#define MIN_CAR_HANDLING    Real(0)
#define MAX_CAR_HANDLING    Real(100)

void DrawCarName(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex)
{
    REAL xPos, yPos;
    long carID = g_TitleScreenData.iCarNum[g_TitleScreenData.iCurrentPlayer];
    static REAL speed = MIN_CAR_TOPEND;
    static REAL acc = MAX_CAR_ACC;
    static REAL weight = MIN_CAR_WEIGHT;
    static REAL handling = ZERO;

#ifdef _PC

    
    xPos = menuHeader->XPos;
    yPos = menuHeader->YPos + (MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP) * itemIndex;

    DrawSpruBox(
        gMenuWidthScale * (xPos - MENU_TEXT_GAP), 
        gMenuHeightScale * (yPos - MENU_TEXT_GAP),
        gMenuWidthScale * (MENU_DATA_WIDTH_TEXT + MENU_TEXT_GAP * 2),
        gMenuHeightScale * ((MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP) * 7 + MENU_TEXT_GAP * 2), 
        menuHeader->SpruColIndex, 0);   

    // Car Name
    DrawMenuText(xPos, yPos, MENU_TEXT_RGB_CHOICE, CarInfo[carID].Name);

    if (CarInfo[carID].Modified || CarInfo[carID].Moved || ModifiedCarInfo) {
        DrawMenuText(xPos + 20 + (MENU_TEXT_WIDTH * strlen(CarInfo[carID].Name)), yPos, MENU_TEXT_RGB_LOLITE, "(CHT)");
    }
    yPos += MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP;

    // Car Class
    xPos = menuHeader->XPos;
    yPos += MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP;
    sprintf( MenuBuffer,  "%s", TEXT_TABLE(TEXT_CAR_CLASS) );
    sprintf( MenuBuffer2, "%s", TEXT_TABLE(TEXT_CAR_CLASS_ELECTRIC + CarInfo[carID].Class) );
	DrawMenuText( xPos, yPos, MENU_TEXT_RGB_NORMAL, MenuBuffer );
    DrawMenuText( xPos + 14 * MENU_TEXT_WIDTH, yPos, MENU_TEXT_RGB_NORMAL, MenuBuffer2 );

    // Car Type
    xPos = menuHeader->XPos;
    yPos += MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP;
    sprintf(MenuBuffer,  "%s", TEXT_TABLE(TEXT_CAR_RATING) );
    sprintf(MenuBuffer2, "%s", TEXT_TABLE(TEXT_CAR_RATING_ROOKIE + CarInfo[carID].Rating));
	DrawMenuText( xPos, yPos, MENU_TEXT_RGB_NORMAL, MenuBuffer );
    DrawMenuText( xPos + 14 * MENU_TEXT_WIDTH, yPos, MENU_TEXT_RGB_NORMAL, MenuBuffer2 );

    // TopSpeed
    xPos = menuHeader->XPos;
    yPos += MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP;
    sprintf(MenuBuffer, TEXT_TABLE(TEXT_CAR_SPEED));
    DrawMenuText(xPos, yPos, MENU_TEXT_RGB_NORMAL, MenuBuffer);
    if (carID <= CARID_AMW) {
        speed += (CarInfo[carID].TopEnd - speed)/10;
        DrawScale(100 * (speed - MIN_CAR_TOPEND) / (MAX_CAR_TOPEND - MIN_CAR_TOPEND), xPos + 14 * MENU_TEXT_WIDTH, yPos, 100, MENU_TEXT_HEIGHT);
    } else {
        DrawMenuText(xPos + 14 * MENU_TEXT_WIDTH, yPos, MENU_TEXT_RGB_NORMAL, "???");
    }
    // Acc
    xPos = menuHeader->XPos;
    yPos += MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP;
    sprintf(MenuBuffer, TEXT_TABLE(TEXT_CAR_ACC));
    DrawMenuText(xPos, yPos, MENU_TEXT_RGB_NORMAL, MenuBuffer);
    if (carID <= CARID_AMW) {
        acc += (CarInfo[carID].Acc - acc)/10;
        DrawScale(100 * (MAX_CAR_ACC - acc) / (MAX_CAR_ACC - MIN_CAR_ACC), xPos + 14 * MENU_TEXT_WIDTH, yPos, 100, MENU_TEXT_HEIGHT);
    } else {
        DrawMenuText(xPos + 14 * MENU_TEXT_WIDTH, yPos, MENU_TEXT_RGB_NORMAL, "???");
    }

    // Weight
    xPos = menuHeader->XPos;
    yPos += MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP;
    sprintf(MenuBuffer, TEXT_TABLE(TEXT_CAR_WEIGHT));
    DrawMenuText(xPos, yPos, MENU_TEXT_RGB_NORMAL, MenuBuffer);
    if (carID <= CARID_AMW) {
        weight += (CarInfo[carID].Weight - weight)/10;
        DrawScale(100 * (weight - MIN_CAR_WEIGHT) / (MAX_CAR_WEIGHT - MIN_CAR_WEIGHT), xPos + 14 * MENU_TEXT_WIDTH, yPos, 100, MENU_TEXT_HEIGHT);
    } else {
        DrawMenuText(xPos + 14 * MENU_TEXT_WIDTH, yPos, MENU_TEXT_RGB_NORMAL, "???");
    }

    // Handling
//  xPos = menuHeader->XPos;
//  yPos += MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP;
//  sprintf(MenuBuffer, "Handling ");
//  DrawMenuText(xPos, yPos, MENU_TEXT_RGB_NORMAL, MenuBuffer);
    /*if (carID <= CARID_AMW) {
        handling += (CarInfo[carID].Handling - handling)/10;
        DrawScale(handling, xPos + 14 * MENU_TEXT_WIDTH, yPos, 100, MENU_TEXT_HEIGHT);
    } else {
        DrawMenuText(xPos + 14 * MENU_TEXT_WIDTH, yPos, MENU_TEXT_RGB_NORMAL, "???");
    }*/
//  DrawMenuText(xPos + 14 * MENU_TEXT_WIDTH, yPos, MENU_TEXT_RGB_NORMAL, "Guess!!");

    // Transmission
    xPos = menuHeader->XPos;
    yPos += MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP;
    sprintf(MenuBuffer,  "%s", TEXT_TABLE(TEXT_CAR_TRANS) );
    sprintf(MenuBuffer2, "%s", TEXT_TABLE(TEXT_CAR_TRANS_4WD + CarInfo[carID].Trans) );
	DrawMenuText( xPos, yPos, MENU_TEXT_RGB_NORMAL, MenuBuffer );
    DrawMenuText( xPos + 14 * MENU_TEXT_WIDTH, yPos, MENU_TEXT_RGB_NORMAL, MenuBuffer2 );

    // host started?
    if (IsMultiPlayer())
    {
        GetRemoteMessages();
        if (bGameStarted)
        {
            //SetRaceData(menuHeader, menu, menuItem);
            g_bTitleScreenRunGame = TRUE;
        }
    }
#endif
}

