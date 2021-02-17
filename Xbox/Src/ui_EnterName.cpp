//-----------------------------------------------------------------------------
// File: EnterName.cpp
//
// Desc: 
//
// Re-Volt (Generic) Copyright (c) Probe Entertainment 1998
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "Revolt.h"
#include "Main.h"
#include "Light.h"
#include "Obj_Init.h"
#include "Timing.h"
#include "Text.h"
#include "Cheats.h"
#include "Settings.h"
#include "ui_Menu.h"
#include "ui_TitleScreen.h"
#include "ui_StateEngine.h"
#include "ui_MenuText.h"
#include "ui_menudraw.h"
#include "ui_SelectCar.h"
#include "ui_SelectTrack.h"
#include "ui_ShowMessage.h"
#include "ui_WaitingRoom.h"
#include "ui_RaceOverview.h"
#include "ui_EnterName.h"
#include "net_xonline.h"



#define MENU_NAMESELECT_XPOS            320.0f
#define MENU_NAMESELECT_YPOS            130.0f

static VOID CreatePlayerNameMenu( MENU_HEADER* pMenuHeader, MENU* pMenu );
static BOOL HandlePlayerNameMenu( MENU_HEADER* pMenuHeader, DWORD dwInput );
static BOOL PlayerNamePrevChar();
static BOOL PlayerNameNextChar();
static BOOL PlayerNameSelectChar();
static VOID DrawPlayerName( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem, int itemIndex );

static BOOL g_bNameEntered = FALSE;


// Object init functions
static long InitNameWheel(OBJECT *obj, long *flags);
static void UpdateNameWheel(OBJECT *obj);
static void FreeNameWheel(OBJECT *obj);
static void RenderNameWheel(OBJECT *obj);

extern void InitNameWheelStuff();

static OBJECT* gNameWheel = NULL;
static BYTE    g_NameWheelUpperCaseLetters[MAX_NAMESELECTCHARS+1] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ !\x081\x080";
static BYTE    g_NameWheelLowerCaseLetters[MAX_NAMESELECTCHARS+1] = "abcdefghijklmnopqrstuvwxyz ?\x081\x080";
static BYTE    g_NameWheelSymbols[MAX_NAMESELECTCHARS+1]          = "0123456789()@#$%^&*-+=:;'\"[]\x081\x080";
static BYTE*   g_pNameWheelLetters = g_NameWheelUpperCaseLetters;

static D3DTexture* g_pNameWheelTexture_LowerCase = NULL;
static D3DTexture* g_pNameWheelTexture_UpperCase = NULL;
static D3DTexture* g_pNameWheelTexture_Symbols   = NULL;

// Register the object init data
REGISTER_OBJECT( OBJECT_TYPE_NAMEWHEEL, InitNameWheel, sizeof(NAMEWHEEL_OBJ) );




////////////////////////////////////////////////////////////////
//
// InitNameWheel:
//
////////////////////////////////////////////////////////////////
static long InitNameWheel(OBJECT *obj, long *flags)
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
//    data->Angle = 0;
//    data->DestAngle = 0;
    gTitleScreenVars.pCurrentPlayer->nameWheelAngle     = 0;
    gTitleScreenVars.pCurrentPlayer->nameWheelDestAngle = 0;
    SetVec(&data->WheelPos, 0, -490, -80);

    if (World.m_pXBR != NULL)
    {
     // Get the textures for the namewheel
    g_pNameWheelTexture_UpperCase = World.m_pXBR->GetTexture( "FrontEndd0" );
    g_pNameWheelTexture_LowerCase = World.m_pXBR->GetTexture( "FrontEndd1" );
    g_pNameWheelTexture_Symbols   = World.m_pXBR->GetTexture( "FrontEndd2" );

    // $TODO: there's an unnecessary release on textures when the level
    // is exitted, so we have a temporary AddRef here as a workaround
    g_pNameWheelTexture_UpperCase->AddRef();
    g_pNameWheelTexture_LowerCase->AddRef();
    g_pNameWheelTexture_Symbols->AddRef();
    }

    return TRUE;
}




////////////////////////////////////////////////////////////////
//
// FreeNameWheel
//
////////////////////////////////////////////////////////////////
void FreeNameWheel(OBJECT *obj)
{
    NAMEWHEEL_OBJ *data = (NAMEWHEEL_OBJ*)obj->Data;
    //FreeModel(&data->StandModel, 1);
    //FreeModel(&data->WheelModel, 1);
}




void InitNameWheelStuff()
{
    gNameWheel = NextObjectOfType(OBJ_ObjectHead, OBJECT_TYPE_NAMEWHEEL);
}




////////////////////////////////////////////////////////////////
//
// UpdateNameWheel:
//
////////////////////////////////////////////////////////////////
void UpdateNameWheel(OBJECT *obj)
{
    NAMEWHEEL_OBJ *data = (NAMEWHEEL_OBJ*)obj->Data;

    gTitleScreenVars.pCurrentPlayer->nameWheelDestAngle = (FLOAT)(gTitleScreenVars.pCurrentPlayer->nameSelectPos) / (MAX_NAMESELECTCHARS);

    // Update wheel angle
    FLOAT angle = gTitleScreenVars.pCurrentPlayer->nameWheelDestAngle - gTitleScreenVars.pCurrentPlayer->nameWheelAngle;
    GoodWrap( &angle, -HALF, HALF );
    gTitleScreenVars.pCurrentPlayer->nameWheelAngle += (angle/6);
}




////////////////////////////////////////////////////////////////
//
// RenderNameWheel:
//
////////////////////////////////////////////////////////////////
void RenderNameWheel(OBJECT *obj)
{
    VEC wheelPos;
    MAT matZ, wheelMat;
    NAMEWHEEL_OBJ *data = (NAMEWHEEL_OBJ*)obj->Data;
    BBOX bBox;

    // Set the namewheel's texture page to use the correct case letters
    if( g_pNameWheelLetters == g_NameWheelUpperCaseLetters )
        TexInfo[3].Texture = g_pNameWheelTexture_UpperCase;
    else if( g_pNameWheelLetters == g_NameWheelLowerCaseLetters )
        TexInfo[3].Texture = g_pNameWheelTexture_LowerCase;
    else
        TexInfo[3].Texture = g_pNameWheelTexture_Symbols;
        
    //Render  Stand
    SetEnvStatic(&obj->body.Centre.Pos, &obj->body.Centre.WMatrix, 0xffffff, 0, 0, 2);
    DrawModel(&LevelModel[data->StandModel].Model, &obj->body.Centre.WMatrix, &obj->body.Centre.Pos, MODEL_ENV);

    // Render Wheel Stand
//    RotMatrixZ(&matZ, data->Angle);
    RotMatrixZ(&matZ, gTitleScreenVars.pCurrentPlayer->nameWheelAngle);
    MatMulMat(&matZ, &obj->body.Centre.WMatrix, &wheelMat);
    VecMulMat(&data->WheelPos, &obj->body.Centre.WMatrix, &wheelPos);
    VecPlusEqVec(&wheelPos, &obj->body.Centre.Pos);
    SetEnvGood(0x888888, 0, 0, 1);
    SetBBox(&bBox, -LevelModel[data->WheelModel].Model.Radius,
                    LevelModel[data->WheelModel].Model.Radius,
                   -LevelModel[data->WheelModel].Model.Radius,
                    LevelModel[data->WheelModel].Model.Radius,
                   -LevelModel[data->WheelModel].Model.Radius,
                    LevelModel[data->WheelModel].Model.Radius );
    if( CheckObjectLight(&wheelPos, (BOUNDING_BOX*)&bBox, LevelModel[data->WheelModel].Model.Radius) ) 
    {
        AddModelLight(&LevelModel[data->WheelModel].Model, &wheelPos, &wheelMat);
    }
    DrawModel( &LevelModel[data->WheelModel].Model, &wheelMat, &wheelPos, MODEL_ENVGOOD | MODEL_LIT );
}




/////////////////////////////////////////////////////////////////////
//
// Name Select Menu
//
/////////////////////////////////////////////////////////////////////
extern MENU Menu_EnterName = 
{
    TEXT_ENTERNICKNAME,
    MENU_DEFAULT | MENU_NOBOX,              // Menu type
    CreatePlayerNameMenu,                   // Create menu function
    HandlePlayerNameMenu,                   // Input handler function
    NULL,                                   // Menu draw function
    MENU_NAMESELECT_XPOS,                   // X coord
    MENU_NAMESELECT_YPOS,                   // Y Coord
};

// Name Select - 
MENU_ITEM MenuItem_EnterName = 
{
    TEXT_NONE,                             // Text label index
    MENU_DATA_WIDTH_NAME,                   // Space needed to draw item data
    NULL,                                   // Data
    DrawPlayerName,                         // Draw Function
};


// Create
void CreatePlayerNameMenu(MENU_HEADER *menuHeader, MENU *menu)
{
    // Add the menu items
    menuHeader->AddMenuItem( &MenuItem_EnterName );

}

BOOL HandlePlayerNameMenu( MENU_HEADER* pMenuHeader, DWORD dwInput )
{
    if( g_bNameEntered )
        return FALSE;

    switch( dwInput )
    {
        case MENU_INPUT_LEFT:
            return PlayerNamePrevChar();

        case MENU_INPUT_RIGHT:
            return PlayerNameNextChar();

        case MENU_INPUT_Y:
            // Toggle between lower and upper case letters
            if( g_pNameWheelLetters == g_NameWheelUpperCaseLetters )
                g_pNameWheelLetters = g_NameWheelLowerCaseLetters;
            else if( g_pNameWheelLetters == g_NameWheelLowerCaseLetters )
                g_pNameWheelLetters = g_NameWheelSymbols;
            else
                g_pNameWheelLetters = g_NameWheelUpperCaseLetters;

            return TRUE;

        case MENU_INPUT_BACK:
            gTitleScreenVars.pCurrentPlayer->nameEnter[gTitleScreenVars.pCurrentPlayer->nameEnterPos] = '\0';

            // Return to caller
            g_EnterNameStateEngine.Return( STATEENGINE_TERMINATED );
            return TRUE;

        case MENU_INPUT_SELECT:
            return PlayerNameSelectChar();
    }
    return FALSE;
}



BOOL PlayerNamePrevChar()
{
    // Choose the previous character
    if( --gTitleScreenVars.pCurrentPlayer->nameSelectPos < 0 )
        gTitleScreenVars.pCurrentPlayer->nameSelectPos = MAX_NAMESELECTCHARS-1;

    // Don't let last char be anything other than end or delete
    if( gTitleScreenVars.pCurrentPlayer->nameEnterPos == MAX_PLAYER_NAME - 1 ) 
    {
        if( gTitleScreenVars.pCurrentPlayer->nameSelectPos < NAMESELECT_DELETE ) 
        {
            gTitleScreenVars.pCurrentPlayer->nameSelectPos = NAMESELECT_DELETE;

            g_SoundEngine.Play2DSound( SFX_MENU_LEFTRIGHT, FALSE );
            return FALSE;
        }
    }
    return TRUE;
}

BOOL PlayerNameNextChar()
{
    // Choose the previous character
    if( ++gTitleScreenVars.pCurrentPlayer->nameSelectPos == MAX_NAMESELECTCHARS )
        gTitleScreenVars.pCurrentPlayer->nameSelectPos = 0;

    // Don't let last char be anything other than end or delete
    if( gTitleScreenVars.pCurrentPlayer->nameEnterPos == MAX_PLAYER_NAME - 1 ) 
    {
        if( gTitleScreenVars.pCurrentPlayer->nameSelectPos < NAMESELECT_END )
        {
            gTitleScreenVars.pCurrentPlayer->nameSelectPos = NAMESELECT_END;

            g_SoundEngine.Play2DSound( SFX_MENU_LEFTRIGHT, FALSE );
            return FALSE;
        }
    }
    return TRUE;
}

BOOL PlayerNameSelectChar()
{
    if( gTitleScreenVars.pCurrentPlayer->nameSelectPos == NAMESELECT_END )
    {
        // END

        if( 0 == gTitleScreenVars.pCurrentPlayer->nameEnterPos )
        {
            g_SoundEngine.Play2DSound( SFX_MENU_LEFTRIGHT, FALSE );
            return FALSE;
        }

        // Remove trailing spaces
        while( gTitleScreenVars.pCurrentPlayer->nameEnter[gTitleScreenVars.pCurrentPlayer->nameEnterPos-1] == L' ' )
            gTitleScreenVars.pCurrentPlayer->nameEnterPos--;
        
        gTitleScreenVars.pCurrentPlayer->nameEnter[gTitleScreenVars.pCurrentPlayer->nameEnterPos] = '\0';
        g_bNameEntered = TRUE;

        return TRUE;
    }
    else if( gTitleScreenVars.pCurrentPlayer->nameSelectPos == NAMESELECT_DELETE )
    {
        // DELETE

        if( 0 == gTitleScreenVars.pCurrentPlayer->nameEnterPos )
        {
            g_SoundEngine.Play2DSound( SFX_MENU_LEFTRIGHT, FALSE );
            return FALSE;
        }

        if( --gTitleScreenVars.pCurrentPlayer->nameEnterPos < 0 )
            gTitleScreenVars.pCurrentPlayer->nameEnterPos = 0;

        //gTitleScreenVars.pCurrentPlayer->nameEnter[gTitleScreenVars.pCurrentPlayer->nameEnterPos] = '_';
        gTitleScreenVars.pCurrentPlayer->nameEnter[gTitleScreenVars.pCurrentPlayer->nameEnterPos] = '\0';

        return TRUE;
    }
    else
    {
        // CHARACTER
        if( gTitleScreenVars.pCurrentPlayer->nameEnterPos < MAX_PLAYER_NAME - 1 )
        {
            // Make exceptions for spaces (can't begin with space, or have multiple spaces in a row)
            if( g_pNameWheelLetters[gTitleScreenVars.pCurrentPlayer->nameSelectPos] == L' ' )
            {
                // Don't allow names to begin with a space
                if( 0 == gTitleScreenVars.pCurrentPlayer->nameEnterPos )
                {
                    g_SoundEngine.Play2DSound( SFX_MENU_LEFTRIGHT, FALSE );
                    return FALSE;
                }

                // Don't allow names to have mulitple adjacent spaces
                else if( gTitleScreenVars.pCurrentPlayer->nameEnter[gTitleScreenVars.pCurrentPlayer->nameEnterPos-1] == L' ' )
                {
                    g_SoundEngine.Play2DSound( SFX_MENU_LEFTRIGHT, FALSE );
                    return FALSE;
                }
            }

            gTitleScreenVars.pCurrentPlayer->nameEnter[gTitleScreenVars.pCurrentPlayer->nameEnterPos] = g_pNameWheelLetters[gTitleScreenVars.pCurrentPlayer->nameSelectPos];
            gTitleScreenVars.pCurrentPlayer->nameEnterPos++;
            //gTitleScreenVars.pCurrentPlayer->nameEnter[gTitleScreenVars.pCurrentPlayer->nameEnterPos] = '_';
            gTitleScreenVars.pCurrentPlayer->nameEnter[gTitleScreenVars.pCurrentPlayer->nameEnterPos] = '\0';
            if( gTitleScreenVars.pCurrentPlayer->nameEnterPos == MAX_PLAYER_NAME - 1 ) 
                gTitleScreenVars.pCurrentPlayer->nameSelectPos = NAMESELECT_END;

            return TRUE;
        }
    }

    return FALSE;
}




/////////////////////////////////////////////////////////////////////
//
// Draw Player Name
//
/////////////////////////////////////////////////////////////////////
void DrawPlayerName(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex)
{
    FLOAT fTextWidth;
    
    // Determine max width
    swprintf( MenuBuffer, L"%s 4: MMMMMMMMMMMMMMMM", TEXT_TABLE(TEXT_PLAYER) );  //$REVISIT: should this use a more reliable method?  (Note that other chars -- eg, "W" or Japanese Kanji -- might be larger.)
    fTextWidth = g_pFont->GetTextWidth( MenuBuffer );


    FLOAT xPos = menuHeader->m_XPos - fTextWidth/2;
    FLOAT yPos = menuHeader->m_YPos + (MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP) * itemIndex;


    DrawNewSpruBox( gMenuWidthScale  * (xPos - MENU_LEFT_PAD ),
                    gMenuHeightScale * (yPos - MENU_TOP_PAD ),
                    gMenuWidthScale  * (fTextWidth + MENU_LEFT_PAD + MENU_RIGHT_PAD ),
                    gMenuHeightScale * (MENU_TEXT_HEIGHT + MENU_TOP_PAD + MENU_BOTTOM_PAD ) );

    DrawNewSpruBox( gMenuWidthScale  * (menuHeader->m_XPos - 150 - MENU_LEFT_PAD ),
                    gMenuHeightScale * (yPos+250 - MENU_TOP_PAD ),
                    gMenuWidthScale  * (300 + MENU_LEFT_PAD + MENU_RIGHT_PAD ),
                    gMenuHeightScale * (MENU_TEXT_HEIGHT + MENU_TOP_PAD + MENU_BOTTOM_PAD ) );
    g_pFont->DrawText( menuHeader->m_XPos - 150, menuHeader->m_YPos+250, MENU_TEXT_RGB_NORMAL, L"\203 " );
    g_pFont->DrawText( MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_ENTERNAME_MORELETTERS) );

    if( IsMultiPlayer() && gTitleScreenVars.bUseXOnline && IsLoggedIn(0) && g_bNameEntered )
    {
        DrawNewSpruBox( gMenuWidthScale  * (menuHeader->m_XPos - 150 - MENU_LEFT_PAD ),
                        gMenuHeightScale * (yPos+100 - MENU_TOP_PAD ),
                        gMenuWidthScale  * (300 + MENU_LEFT_PAD + MENU_RIGHT_PAD ),
                        gMenuHeightScale * (MENU_TEXT_HEIGHT + MENU_TOP_PAD + MENU_BOTTOM_PAD ) );
        g_pFont->DrawText( menuHeader->m_XPos - 150, menuHeader->m_YPos+100, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_ENTERNAME_NAMEBEINGVERIFIED) );
    }

    swprintf(MenuBuffer, L"%s %d: ", TEXT_TABLE(TEXT_PLAYER), gTitleScreenVars.iCurrentPlayer + 1);

    DrawMenuText(xPos, yPos, MENU_TEXT_RGB_HILITE, MenuBuffer, menuHeader->m_ItemDataWidth);

    fTextWidth = g_pFont->GetTextWidth( MenuBuffer );
    xPos += fTextWidth;

    // Draw player name with caret
    if( g_bNameEntered || !(TIME2MS(TimerCurrent) & 128) )
        swprintf( MenuBuffer, L"%S", gTitleScreenVars.pCurrentPlayer->nameEnter );
    else
        swprintf( MenuBuffer, L"%S_", gTitleScreenVars.pCurrentPlayer->nameEnter );
    DrawMenuText(xPos, yPos, MENU_TEXT_RGB_CHOICE, MenuBuffer );

    // host started?
    //$WARNING: this code assumes 'bGameStarted' has already been cleared (b/c
    /// assumes this UI screen will only be called *after* client has joined
    /// session, and 'bGameStarted' gets cleared by the UI code when you start
    /// to join a session.  Ugly, ugly...)
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



CEnterNameStateEngine g_EnterNameStateEngine;


//-----------------------------------------------------------------------------
// Name: Process()
// Desc: Main control function for the state engine
//-----------------------------------------------------------------------------
HRESULT CEnterNameStateEngine::Process()
{
    static XONLINETASK_HANDLE hNameVerifyTask = NULL;

    enum
    {
        ENTERNAME_STATE_BEGIN = STATEENGINE_STATE_BEGIN,
        ENTERNAME_STATE_MAINLOOP,
        ENTERNAME_STATE_NAMEENTERED,
        ENTERNAME_STATE_VERIFYNG_NAME,
        ENTERNAME_STATE_NAME_ACCEPTED,
    };

    switch( m_State )
    {
        case ENTERNAME_STATE_BEGIN:
        {
            g_bNameEntered = FALSE;

            //$HACK(Apr02_GameBash) - to default name to gamerID
            if( IsLoggedIn(0) )
            {
                if( !_stricmp( gTitleScreenVars.pCurrentPlayer->nameEnter, "Player" ) )
                {
                    strncpy( gTitleScreenVars.pCurrentPlayer->nameEnter, Players[0].XOnlineInfo.pXOnlineUser->szGamertag, MAX_PLAYER_NAME );
                }
            }
                
            // Get current name entry character position
            gTitleScreenVars.pCurrentPlayer->nameEnterPos = strlen(gTitleScreenVars.pCurrentPlayer->nameEnter);

            // Set initial character choice
            gTitleScreenVars.pCurrentPlayer->nameSelectPos = NAMESELECT_END;
            //gTitleScreenVars.pCurrentPlayer->nameEnter[gTitleScreenVars.pCurrentPlayer->nameEnterPos] = '_';
            gTitleScreenVars.pCurrentPlayer->nameEnter[gTitleScreenVars.pCurrentPlayer->nameEnterPos] = '\0';

            // Set the enter name menu and camera position
            g_pMenuHeader->SetNextMenu( &Menu_EnterName );
            g_pTitleScreenCamera->SetNewPos( TITLESCREEN_CAMPOS_NAME_SELECT );

            m_State = ENTERNAME_STATE_MAINLOOP;
            break;
        }

        case ENTERNAME_STATE_MAINLOOP:
            if( g_bNameEntered )
                m_State = ENTERNAME_STATE_NAMEENTERED;
                // Fall through to next state
            else
                // Nothing to do, as control is in the menus
                break;

        case ENTERNAME_STATE_NAMEENTERED:
            // Check for cheats that are triggered from special player names
            CheckNameCheats( gTitleScreenVars.pCurrentPlayer->nameEnter );

            // Check for inappropriate online names

//$BUGBUG: The IsMultiPlayer() check fails when doing OptiMatch, since the
//         GameSettings.GameType variable is set to GAMETYPE_NONE. Perhaps we
//         should add a GAMETYPE_NETWORK_ANY enum to replace this. For consumer
//         beta, I have little choice but to temporarily remove the
//         IsMultiPlayer() check.
//
//          if( IsMultiPlayer() && gTitleScreenVars.bUseXOnline && IsLoggedIn(0) )
            if( gTitleScreenVars.bUseXOnline && IsLoggedIn(0) )
//$END_BUGBUG
            {
                WCHAR strPlayerName[64];
                swprintf( strPlayerName, L"%S", gTitleScreenVars.pCurrentPlayer->nameEnter );
                HRESULT hr = XOnlineVerifyNickname( strPlayerName, NULL, &hNameVerifyTask );

                m_State = ENTERNAME_STATE_VERIFYNG_NAME;
            }
            else
                m_State = ENTERNAME_STATE_NAME_ACCEPTED;
            break;

        case ENTERNAME_STATE_VERIFYNG_NAME:
        {
            HRESULT hr = XOnlineTaskContinue( hNameVerifyTask );

            if( XONLINETASK_S_RUNNING == hr )
                break;

            XOnlineTaskClose( hNameVerifyTask );

            if( XONLINETASK_S_SUCCESS == hr )
            {
                // The name was accepted so adavance states
                m_State = ENTERNAME_STATE_NAME_ACCEPTED;
            }
            else
            {
                // Tell the user his name was inappropiate
                g_ShowSimpleMessage.Begin( TEXT_TABLE(TEXT_ENTERNAME_NICKNAME),
                                           TEXT_TABLE(TEXT_ENTERNAME_NICKNAMEINAPPROPRIATE),
                                           NULL,
                                           TEXT_TABLE(TEXT_BUTTON_B_BACK) );

                // After the displaying the message, send the player back to
                // entering a name
                NextState( ENTERNAME_STATE_MAINLOOP );
                g_bNameEntered = FALSE;
            }
            break;
        }

        case ENTERNAME_STATE_NAME_ACCEPTED:
            // Copy the name to the registry settings
            // TODO: There should be multiple registry entries for multiple players
            strncpy( RegistrySettings.PlayerName, gTitleScreenVars.pCurrentPlayer->nameEnter, MAX_PLAYER_NAME );

            // Return to caller
            g_EnterNameStateEngine.Return( STATEENGINE_COMPLETED );
            break;
    }

    // Handle the menus
    g_pMenuHeader->HandleMenus();

    return S_FALSE;
}



