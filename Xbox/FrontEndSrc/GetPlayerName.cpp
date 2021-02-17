//-----------------------------------------------------------------------------
// File: GetPlayerName.cpp
//
// Desc: 
//
// Re-Volt (Generic) Copyright (c) Probe Entertainment 1998
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "Revolt.h"
#include "Object.h"
#include "Obj_Init.h"
#include "Light.h"
#include "TitleScreen.h"
#include "Timing.h"
#include "Cheats.h"       // CheckNameCheats
#include "Menu2.h"
#include "MenuDraw.h"   // DrawSpruBox
#include "MenuText.h"   // re-volt strings

#include "RaceSummary.h"
#include "SelectTrack.h"
#include "WaitingRoom.h"
#include "SelectCar.h"



//-----------------------------------------------------------------------------
// Local function prototypes
//-----------------------------------------------------------------------------
static VOID CreatePlayerNameMenu(MENU_HEADER *menuHeader, MENU *menu);
static VOID DrawPlayerName(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex);
static BOOL PlayerNameHandler( DWORD input, MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem );
static LONG InitNameWheel(OBJECT *obj, long *flags);
void MenuGoBackCloseSession(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);

REGISTER_OBJECT( OBJECT_TYPE_NAMEWHEEL, InitNameWheel, sizeof(NAMEWHEEL_OBJ) );




//-----------------------------------------------------------------------------
// Local data
//-----------------------------------------------------------------------------

// Name selection
#define MAX_NAMESELECTCHARS (26+4)
#define NAMESELECT_END      (MAX_NAMESELECTCHARS-1)
#define NAMESELECT_DELETE   (MAX_NAMESELECTCHARS-2)

static BYTE g_NameSelectLetters[MAX_NAMESELECTCHARS+1] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ !\x081\x080";

// Menu defines
#define MENU_NAMESELECT_XPOS            Real(100)
#define MENU_NAMESELECT_YPOS            Real(120)




//-----------------------------------------------------------------------------
// Name: RenderNameWheel()
// Desc: 
//-----------------------------------------------------------------------------
void RenderNameWheel(OBJECT *obj)
{
    VEC wheelPos;
    MAT matZ, wheelMat;
    NAMEWHEEL_OBJ *data = (NAMEWHEEL_OBJ*)obj->Data;
    BBOX bBox;

    //Render  Stand
    SetEnvStatic(&obj->body.Centre.Pos, &obj->body.Centre.WMatrix, 0xffffff, 0, 0, 2);
    DrawModel(&LevelModel[data->StandModel].Model, &obj->body.Centre.WMatrix, &obj->body.Centre.Pos, MODEL_ENV);

    // Render Wheel Stand
    RotMatrixZ(&matZ, data->Angle);
    MatMulMat(&matZ, &obj->body.Centre.WMatrix, &wheelMat);
    VecMulMat(&data->WheelPos, &obj->body.Centre.WMatrix, &wheelPos);
    VecPlusEqVec(&wheelPos, &obj->body.Centre.Pos);
    SetEnvGood(0x888888, 0, 0, 1);
    SetBBox( &bBox, -LevelModel[data->WheelModel].Model.Radius,
                    +LevelModel[data->WheelModel].Model.Radius,
                    -LevelModel[data->WheelModel].Model.Radius,
                    +LevelModel[data->WheelModel].Model.Radius,
                    -LevelModel[data->WheelModel].Model.Radius,
                    +LevelModel[data->WheelModel].Model.Radius );
    if (CheckObjectLight(&wheelPos, (BOUNDING_BOX*)&bBox, LevelModel[data->WheelModel].Model.Radius)) 
	{
        AddModelLight(&LevelModel[data->WheelModel].Model, &wheelPos, &wheelMat);
    }
    DrawModel(&LevelModel[data->WheelModel].Model, &wheelMat, &wheelPos, MODEL_ENVGOOD | MODEL_LIT);
}




//-----------------------------------------------------------------------------
// Name: UpdateNameWheel()
// Desc: 
//-----------------------------------------------------------------------------
void UpdateNameWheel(OBJECT *obj)
{
    REAL ang;
    NAMEWHEEL_OBJ *data = (NAMEWHEEL_OBJ*)obj->Data;

    data->DestAngle = Real(g_TitleScreenData.nameSelectPos) / (MAX_NAMESELECTCHARS);

    // Update wheel angle
    ang = data->DestAngle - data->Angle;
    GoodWrap(&ang, -HALF, HALF);
    data->Angle += (ang / 6);
    
}




//-----------------------------------------------------------------------------
// Name FreeNameWheel()
// Desc: 
//-----------------------------------------------------------------------------
void FreeNameWheel(OBJECT *obj)
{
    NAMEWHEEL_OBJ *data = (NAMEWHEEL_OBJ*)obj->Data;

    //FreeModel(&data->StandModel, 1);
    //FreeModel(&data->WheelModel, 1);
}




//-----------------------------------------------------------------------------
// Name: InitNameWheel()
// Desc: 
//-----------------------------------------------------------------------------
long InitNameWheel(OBJECT *obj, long *flags)
{
    NAMEWHEEL_OBJ *data = (NAMEWHEEL_OBJ*)obj->Data;

    // Models
    data->StandModel = LoadOneLevelModel(LEVEL_MODEL_NAMESTAND, TRUE, obj->renderflag, 0);
    data->WheelModel = LoadOneLevelModel(LEVEL_MODEL_NAMEWHEEL, TRUE, obj->renderflag, 0);

    // No collision
    obj->CollType = COLL_TYPE_NONE;
    obj->body.CollSkin.AllowObjColls = FALSE;
    obj->body.CollSkin.AllowWorldColls = FALSE;
    

    // handlers
    obj->renderhandler = (RENDER_HANDLER)RenderNameWheel;
    obj->aihandler = (AI_HANDLER)UpdateNameWheel;
    obj->freehandler = (FREE_HANDLER)FreeNameWheel;

    // Init Data
    data->Angle = 0;
    data->DestAngle = 0;
    SetVec(&data->WheelPos, 0, -490, -80);

    return TRUE;
}
















// Player Name Menu
MENU Menu_PlayerName = 
{
    TEXT_ENTERNAME,
    MENU_IMAGE_NAMEENTER,                   // Menu title bar image enum
    TITLESCREEN_CAMPOS_NAME_SELECT,                  // Camera pos index
    MENU_DEFAULT | MENU_CENTRE_X,               // Menu type
    SPRU_COL_RANDOM,                        // Spru colour
    CreatePlayerNameMenu,                   // Create menu function
    MENU_NAMESELECT_XPOS,                   // X coord
    MENU_NAMESELECT_YPOS,                   // Y Coord
    0,                                      // Current Item index
    NULL,                                   // Parent Menu (OverWritten in Create)
};

// Name Select - 
MENU_ITEM MenuItem_PlayerName = 
{
    TEXT_PLAYER_SINGULAR,                   // Text label index

    MENU_DATA_WIDTH_NAME,                   // Space needed to draw item data
    &Menu_SelectCar,                        // Data
    DrawPlayerName,                         // Draw Function

	PlayerNameHandler,					    // Input handler
	0,                                      // Flags
};




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
VOID CreatePlayerNameMenu( MENU_HEADER* pMenuHeader, MENU* pMenu )
{
    // Make current name into upper case
    CHAR* pc = g_TitleScreenData.nameEnter[g_TitleScreenData.iCurrentPlayer];
    while( *pc != '\0' ) 
        *pc++ = toupper(*pc);

    // Get current name entry character position
    g_TitleScreenData.nameEnterPos = strlen( g_TitleScreenData.nameEnter[g_TitleScreenData.iCurrentPlayer] );

    // Set initial character choice
    g_TitleScreenData.nameSelectPos = NAMESELECT_END;
    //g_TitleScreenData.nameEnter[g_TitleScreenData.iCurrentPlayer][g_TitleScreenData.nameEnterPos] = '_';
    g_TitleScreenData.nameEnter[g_TitleScreenData.iCurrentPlayer][g_TitleScreenData.nameEnterPos] = '\0';

    // Add the menu items
    AddMenuItem( pMenuHeader, pMenu, &MenuItem_PlayerName );

}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
BOOL PlayerNameHandler( DWORD input, MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem )
{
	if( input == MENU_INPUT_LEFT || input == MENU_INPUT_UP )
	{
		// Choose the previous character
		g_TitleScreenData.nameSelectPos--;
		if( g_TitleScreenData.nameSelectPos < 0 )
			g_TitleScreenData.nameSelectPos = MAX_NAMESELECTCHARS-1;

		// Don't let last char be anything other than end or delete
		if( g_TitleScreenData.nameEnterPos == MAX_INPUT_NAME - 2 ) 
		{
			if( g_TitleScreenData.nameSelectPos < NAMESELECT_DELETE ) 
				g_TitleScreenData.nameSelectPos = NAMESELECT_DELETE;
		}
		return TRUE;
	}

	if( input == MENU_INPUT_RIGHT || input == MENU_INPUT_DOWN )
	{
		// Choose the previous character
		g_TitleScreenData.nameSelectPos++;
		if( g_TitleScreenData.nameSelectPos == MAX_NAMESELECTCHARS )
			g_TitleScreenData.nameSelectPos = 0;

		// Don't let last char be anything other than end or delete
		if( g_TitleScreenData.nameEnterPos == MAX_INPUT_NAME - 2 ) 
		{
			if( g_TitleScreenData.nameSelectPos < NAMESELECT_END ) 
				g_TitleScreenData.nameSelectPos = NAMESELECT_END;
		}
		return TRUE;
	}

	if( input == MENU_INPUT_BACK )
	{
		g_TitleScreenData.nameEnter[g_TitleScreenData.iCurrentPlayer][g_TitleScreenData.nameEnterPos] = '\0';

#ifndef XBOX_DISABLE_NETWORK
//$MODIFIED  
//		  if (IsMultiPlayer() && GameSettings.MultiType == MULTITYPE_CLIENT && !Lobby)
	    if( IsMultiPlayer() && IsClient() ) //$HEY: do we want to check for valid net connection here (kind of like what !Lobby was checking)??
//$END_MODIFICATIONS
		{
			MenuGoBackCloseSession(menuHeader, menu, menuItem);
		}
#endif // ! XBOX_DISABLE_NETWORK

		MenuGoBack( pMenuHeader, pMenu, pMenuItem );
		return TRUE;
	}

	if( input == MENU_INPUT_SELECT )
	{
		PlaySfx(SFX_MENU_LEFTRIGHT, SFX_MAX_VOL, SFX_CENTRE_PAN, 44100, 0x7fffffff);

		if( g_TitleScreenData.nameSelectPos == NAMESELECT_END )
		{
			// END
			g_TitleScreenData.nameEnter[g_TitleScreenData.iCurrentPlayer][g_TitleScreenData.nameEnterPos] = '\0';
			CheckNameCheats(g_TitleScreenData.nameEnter[g_TitleScreenData.iCurrentPlayer]);

			if( IsMultiPlayer() )
			{
//$REMOVED (tentative!!) - don't want to store names via DPlay, but need to store them somewhere...
//	            SetPlayerName(g_TitleScreenData.nameEnter[0]);
//$END_REMOVAL
				if (IsServer())
				{
#ifndef XBOX_DISABLE_NETWORK //$REVISIT: Probably can remove; I don't think we need to call this (level name, etc gets propagated to clients manually, not via DPlay)
					LEVELINFO *levinf = GetLevelInfo(g_TitleScreenData.iLevelNum);
					SetSessionDesc(g_TitleScreenData.nameEnter[0], levinf->Dir, FALSE, GAMETYPE_MULTI, GameSettings.RandomCars, GameSettings.RandomTrack);
#endif // !XBOX_DISABLE_NETWORK
				}
			}

			// set next menu
			if( GameSettings.GameType == GAMETYPE_CLOCKWORK )
			{
				if( GameSettings.RandomTrack )
					pMenuItem->pData = &Menu_Overview;
				else
					pMenuItem->pData = &Menu_SelectTrack;
			}
			else if( GameSettings.RandomCars )
			{
				if( GameSettings.MultiType == MULTITYPE_CLIENT )
				{
					pMenuItem->pData = &Menu_WaitingRoom;
				}
				else
				{
					if( GameSettings.RandomTrack )
					{
						if( IsMultiPlayer() )
							pMenuItem->pData = &Menu_WaitingRoom;
						else
							pMenuItem->pData = &Menu_Overview;
					}
					else
						pMenuItem->pData = &Menu_SelectTrack;
				}
			}
			else
			{
				pMenuItem->pData = &Menu_SelectCar;
			}

			MenuGoForward( pMenuHeader, pMenu, pMenuItem);
		}
		else if( g_TitleScreenData.nameSelectPos == NAMESELECT_DELETE )
		{
			// DELETE
			g_TitleScreenData.nameEnterPos--;
			if( g_TitleScreenData.nameEnterPos < 0 )
				g_TitleScreenData.nameEnterPos = 0;

			//g_TitleScreenData.nameEnter[g_TitleScreenData.iCurrentPlayer][g_TitleScreenData.nameEnterPos] = '_';
			g_TitleScreenData.nameEnter[g_TitleScreenData.iCurrentPlayer][g_TitleScreenData.nameEnterPos] = '\0';
		}
		else
		{
			// CHARACTER
			if( g_TitleScreenData.nameEnterPos < MAX_INPUT_NAME - 2 )
			{
				g_TitleScreenData.nameEnter[g_TitleScreenData.iCurrentPlayer][g_TitleScreenData.nameEnterPos] = g_NameSelectLetters[g_TitleScreenData.nameSelectPos];
				g_TitleScreenData.nameEnterPos++;
				//g_TitleScreenData.nameEnter[g_TitleScreenData.iCurrentPlayer][g_TitleScreenData.nameEnterPos] = '_';
				g_TitleScreenData.nameEnter[g_TitleScreenData.iCurrentPlayer][g_TitleScreenData.nameEnterPos] = '\0';
				if( g_TitleScreenData.nameEnterPos == MAX_INPUT_NAME - 2 ) 
					g_TitleScreenData.nameSelectPos = NAMESELECT_END;
			}
		}
		return TRUE;
	}

	return FALSE;
}




//-----------------------------------------------------------------------------
// Name: DrawPlayerName()
// Desc: Draw Player Name
//-----------------------------------------------------------------------------
void DrawPlayerName(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex)
{
    REAL xPos, yPos;

    // display name
    xPos = menuHeader->XPos;
    yPos = menuHeader->YPos + (MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP) * itemIndex;

    sprintf(MenuBuffer, " %d: ", g_TitleScreenData.iCurrentPlayer + 1);
    DrawMenuText(xPos + strlen(TEXT_TABLE(TEXT_PLAYER_SINGULAR)) * MENU_TEXT_WIDTH, yPos, MENU_TEXT_RGB_HILITE, MenuBuffer );

    xPos = menuHeader->XPos + menuHeader->ItemTextWidth + MENU_TEXT_GAP;
    DrawMenuText(xPos, yPos, MENU_TEXT_RGB_CHOICE, g_TitleScreenData.nameEnter[g_TitleScreenData.iCurrentPlayer] );
 
    // flash selected letter
    if (!(TIME2MS(TimerCurrent) & 128))
    {
        long len = strlen(g_TitleScreenData.nameEnter[g_TitleScreenData.iCurrentPlayer]);
        /*
            sprintf(MenuBuffer, "%c", g_NameSelectLetters[g_TitleScreenData.nameSelectPos]);
            DrawMenuText(xPos + (len) * MENU_TEXT_WIDTH, yPos, MENU_TEXT_RGB_LOLITE, MenuBuffer );
        }*/
        DrawMenuText(xPos + (len) * MENU_TEXT_WIDTH, yPos, MENU_TEXT_RGB_NOTCHOICE, "_" );
    }
    else 
    {
        long len = strlen(g_TitleScreenData.nameEnter[g_TitleScreenData.iCurrentPlayer]);
        /*
            sprintf(MenuBuffer, "%c", g_NameSelectLetters[g_TitleScreenData.nameSelectPos]);
            DrawMenuText(xPos + (len) * MENU_TEXT_WIDTH, yPos, MENU_TEXT_RGB_LOLITE, MenuBuffer );
        }*/
        DrawMenuText(xPos + (len) * MENU_TEXT_WIDTH, yPos, MENU_TEXT_RGB_CHOICE, "_" );
    }

    //DrawPlayerNumber();

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
}

////////////////////////////////////////////////////////////////
//
// Draw Player Number
//
////////////////////////////////////////////////////////////////

void DrawPlayerNumber()
{
    sprintf(MenuBuffer, "%s %d", TEXT_TABLE(TEXT_PLAYER_SINGULAR), g_TitleScreenData.iCurrentPlayer + 1);
    DrawMenuText(100, 16, MENU_TEXT_RGB_NORMAL, MenuBuffer );
}




