////////////////////////////////////////////////////////////////
//
// Track Select Menu
//
////////////////////////////////////////////////////////////////

#include "revolt.h"
#include "menu2.h"
#include "MenuDraw.h"   // DrawSpruBox
#include "settings.h"   // RegistrySettings
#include "LevelLoad.h"  // GAMETYPE, GameSettings
#include "cheats.h"       // AllCars
#include "timing.h"       // LoadTrackTimes
#include "InitPlay.h"     // StartData
#include "RaceSummary.h"  // Menu_Overview
#include "WaitingRoom.h"  // Menu_WaitingRoom
#include "Object.h"
#include "Obj_init.h"
#include "drawobj.h"
#include "menutext.h"   // re-volt strings
#include "titlescreen.h"
#include "SelectTrack.h"


extern BOOL g_bUsingNewFrontEnd;


#define MENU_SELECTTRACK_XPOS           100.0f
#define MENU_SELECTTRACK_YPOS           380.0f

#define TRACKSCREEN_WOBBLE_TIME 0.1f


static void CreateSelectTrackMenu( MENU_HEADER* pMenuHeader, MENU* pMenu );
static void SelectTrackSelectTrack( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem );

static void LoadSelectTrackTexture( object_def* obj );
static void DrawTrackName( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem, int itemIndex );

static void DrawSelectTrackScreen( OBJECT* obj );
static long InitTrackScreen( OBJECT* obj, long* flags );
static void TrackScreenHandler( OBJECT* obj );

static void DrawSmallScreen( OBJECT* obj );
static long InitSmallScreen( OBJECT* obj, long* flags );
static void SmallScreenHandler( OBJECT* obj );


// Register the object init data
REGISTER_OBJECT( OBJECT_TYPE_TRACKSCREEN, InitTrackScreen, sizeof(OBJECT_TRACKSCREEN_OBJ) );
REGISTER_OBJECT( OBJECT_TYPE_SMALLSCREEN, InitSmallScreen, sizeof(OBJECT_SMALLSCREEN_OBJ) );





static long g_lSmallScreenTPage  = TPAGE_FX1;


// Track Select Screen position
VEC g_SelectTrackScreenPos[4] =
{
    {-299.0f, -726.4f, -75.2f },
    { 299.0f, -726.4f, -75.2f },
    { 299.0f, -291.6f,   5.0f },
    {-299.0f, -291.6f,   5.0f },
};

VEC g_SmallScreenPos[4] =
{
    {-129.0f, -326.6f, -23.1f },
    { 132.0f, -326.6f, -23.1f },
    { 132.0f, -133.5f, -23.1f },
    {-129.0f, -133.5f, -23.1f },
};




// Menu
MENU Menu_SelectTrack = 
{
    TEXT_SELECTTRACK,
    MENU_IMAGE_SELECTTRACK,                     // Menu title bar image enum
    TITLESCREEN_CAMPOS_TRACK_SELECT,                 // Camera pos index
    MENU_DEFAULT | MENU_CENTRE_X,       // Menu type
    SPRU_COL_RANDOM,                        // Spru colour
    CreateSelectTrackMenu,                  // Create menu function
    MENU_SELECTTRACK_XPOS,                  // X coord
    MENU_SELECTTRACK_YPOS,                  // Y Coord
    0,                                      // Current Item index
    NULL,                                   // Parent Menu (OverWritten in Create)
};

// Car Select Menu Item
MENU_ITEM MenuItem_TrackSelect = 
{
    TEXT_NONE,                              // Text label index

    MENU_DATA_WIDTH_TEXT,                   // Space needed to draw item data
    NULL, // &Menu_Overview,                // Data
    DrawTrackName,                          // Draw Function

 	NULL,								    // Input handler
	0,                                      // Flags

    SelectTrackMirrorTrack,                 // Up Action
    SelectTrackReverseTrack,                // Down Action
    SelectTrackPrevTrack,                   // Left Action
    SelectTrackNextTrack,                   // Right Action
    MenuGoBack,                             // Back Action
    SelectTrackSelectTrack,                 // Forward Action
};




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
VOID CreateSelectTrackMenu( MENU_HEADER* pMenuHeader, MENU* pMenu )
{
    LEVELINFO *pLevelInfo;

	// re-call car select func if 'AllTracks' true
    if( AllTracks )
    {
        InitDefaultLevels();
    }

    // Set the next menu item depending on game type
	if( g_bUsingNewFrontEnd )
	{
		MenuItem_TrackSelect.pData = NULL;
	}
	else
	{
		if( IsMultiPlayer() ) 
		{
			MenuItem_TrackSelect.pData = &Menu_WaitingRoom;
		} 
		else 
		{
			MenuItem_TrackSelect.pData = &Menu_Overview;
		}
	}

    // Load level image and best times
    LoadTrackTimes(g_TitleScreenData.iLevelNum, g_TitleScreenData.mirror, g_TitleScreenData.reverse);

    g_TitleScreenData.iLevelNum = GetLevelNum(RegistrySettings.LevelDir);
    if (g_TitleScreenData.iLevelNum == -1) 
	{
        g_TitleScreenData.iLevelNum = 0;
    }
    pLevelInfo = GetLevelInfo( g_TitleScreenData.iLevelNum );
    strncpy( StartData.LevelDir, pLevelInfo->Dir, MAX_LEVEL_DIR_NAME );

    // Make sure directory exists
    if (!DoesLevelExist(g_TitleScreenData.iLevelNum)) 
	{
        SetLevelUnavailable(g_TitleScreenData.iLevelNum);
    }

    // Add menu items
    AddMenuItem( pMenuHeader, pMenu, &MenuItem_TrackSelect );

    // get next then prev track to ensure valid level for game type - BKK 03/05/99
    // PT: Changed to Prev -> Next so it goes to the first ones when playing stunt or battle
    SelectTrackPrevTrack(pMenuHeader, pMenu, NULL);
    SelectTrackNextTrack(pMenuHeader, pMenu, NULL);
        
    // Make sure track isn't mirrored or reversed if its a battle track
    if (GetLevelInfo(g_TitleScreenData.iLevelNum)->TrackType == TRACK_TYPE_BATTLE) 
	{
        g_TitleScreenData.mirror = FALSE;
        g_TitleScreenData.reverse = FALSE;
    }

    if (GetLevelInfo(g_TitleScreenData.iLevelNum)->TrackType == TRACK_TYPE_USER) 
	{
        g_TitleScreenData.reverse = FALSE;
    }

    g_iTrackScreenLevelNum = g_TitleScreenData.iLevelNum;          // Level number shown on the video screens
    g_bMenuDrawMenuBox = FALSE;                                   // Don't draw a default box

}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
void SelectTrackSelectTrack( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem )
{
    if( IsLevelTypeAvailable( g_TitleScreenData.iLevelNum, g_TitleScreenData.mirror, 
		                      g_TitleScreenData.reverse ) )
    {
		g_TitleScreenData.bTrackSelected = TRUE;

		// mwetzel - this line will start the game
        MenuGoForward( pMenuHeader, pMenu, pMenuItem );
    }
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
void SelectTrackMirrorTrack( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem )
{
    if (GetLevelInfo(g_TitleScreenData.iLevelNum)->TrackType == TRACK_TYPE_BATTLE) return;

    g_TitleScreenData.mirror = !g_TitleScreenData.mirror;

    LoadTrackTimes(g_TitleScreenData.iLevelNum, g_TitleScreenData.mirror, g_TitleScreenData.reverse);
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
void SelectTrackReverseTrack( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem )
{
    if (GetLevelInfo(g_TitleScreenData.iLevelNum)->TrackType == TRACK_TYPE_BATTLE) return;
    if (GetLevelInfo(g_TitleScreenData.iLevelNum)->TrackType == TRACK_TYPE_USER) return;

    g_TitleScreenData.reverse = !g_TitleScreenData.reverse;

    LoadTrackTimes(g_TitleScreenData.iLevelNum, g_TitleScreenData.mirror, g_TitleScreenData.reverse);
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
void SelectTrackPrevTrack( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem )
{
    LEVELINFO *pLevelInfo;

    // Select previous allowed level
    do 
	{
        g_TitleScreenData.iLevelNum--;
        if (g_TitleScreenData.iLevelNum < 0) 
            g_TitleScreenData.iLevelNum = GameSettings.LevelNum - 1;

        pLevelInfo = GetLevelInfo(g_TitleScreenData.iLevelNum);
        g_iTrackScreenLevelNum = g_TitleScreenData.iLevelNum;
    } 
	while (!IsLevelSelectable(g_TitleScreenData.iLevelNum));

    // Setup info 
    strncpy(StartData.LevelDir, pLevelInfo->Dir, MAX_LEVEL_DIR_NAME);
    strncpy(RegistrySettings.LevelDir, pLevelInfo->Dir, MAX_LEVEL_DIR_NAME);

    // Load level image and best times
    LoadTrackTimes(g_TitleScreenData.iLevelNum, g_TitleScreenData.mirror, g_TitleScreenData.reverse);

    // Make sure directory exists
    if( !DoesLevelExist( g_TitleScreenData.iLevelNum ) ) 
        SetLevelUnavailable( g_TitleScreenData.iLevelNum );
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
void SelectTrackNextTrack( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem )
{
    LEVELINFO *pLevelInfo;

    do 
	{
        g_TitleScreenData.iLevelNum++;
        if (g_TitleScreenData.iLevelNum >= GameSettings.LevelNum) 
            g_TitleScreenData.iLevelNum = 0;

        pLevelInfo = GetLevelInfo(g_TitleScreenData.iLevelNum);
        g_iTrackScreenLevelNum = g_TitleScreenData.iLevelNum;
    } 
	while (!IsLevelSelectable(g_TitleScreenData.iLevelNum));

    // Setup Info
    pLevelInfo = GetLevelInfo(g_TitleScreenData.iLevelNum);
    strncpy(StartData.LevelDir, pLevelInfo->Dir, MAX_LEVEL_DIR_NAME);
    strncpy(RegistrySettings.LevelDir, pLevelInfo->Dir, MAX_LEVEL_DIR_NAME);
        
    // Load level image and Best Times
    LoadTrackTimes(g_TitleScreenData.iLevelNum, g_TitleScreenData.mirror, g_TitleScreenData.reverse);


    // Make sure directory exists
    if( !DoesLevelExist( g_TitleScreenData.iLevelNum ) )
        SetLevelUnavailable( g_TitleScreenData.iLevelNum );
}




//-----------------------------------------------------------------------------
// Name: TrackScreenHandler()
// Desc: 
//-----------------------------------------------------------------------------
void TrackScreenHandler( OBJECT* obj )
{
    OBJECT_TRACKSCREEN_OBJ *pTrackScreen = (OBJECT_TRACKSCREEN_OBJ*)obj->Data;

    switch (pTrackScreen->State) {

    case TRACKSCREEN_STEADY:

        // reduce wobble time
        pTrackScreen->Timer -= TimeStep;
        if (pTrackScreen->Timer < 0.0f)
            pTrackScreen->Timer = 0.0f;

        // Change state if level has changed
        if (g_iTrackScreenLevelNum != pTrackScreen->CurrentLevel) {
            pTrackScreen->CurrentLevel = g_iTrackScreenLevelNum;
            pTrackScreen->State = TRACKSCREEN_WOBBLY;
            if (gMenuHeader.pMenu == &Menu_SelectTrack)
            {
                PlaySfx(SFX_TVSTATIC, SFX_MAX_VOL, SFX_CENTRE_PAN, SFX_SAMPLE_RATE, 0x7fffffff);
#pragma message( "jharding needs to look into porting this." )
            }
            LoadSelectTrackTexture(obj);
        }
        break;

    case TRACKSCREEN_WOBBLY:

        pTrackScreen->Timer += TimeStep;
        if (pTrackScreen->Timer > TRACKSCREEN_WOBBLE_TIME) {
            pTrackScreen->Timer = TRACKSCREEN_WOBBLE_TIME;
            pTrackScreen->State = TRACKSCREEN_STEADY;
            if (pTrackScreen->TPage == TPAGE_MISC1) 
			{
                g_lSmallScreenTPage = pTrackScreen->TPage = TPAGE_MISC2;
            } 
			else 
			{
                g_lSmallScreenTPage = pTrackScreen->TPage = TPAGE_MISC1;
            }
        }
        break;

    default:
        break;
    }

}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
long InitTrackScreen( OBJECT* obj, long* flags )
{
    OBJECT_TRACKSCREEN_OBJ* pTrackScreen = (OBJECT_TRACKSCREEN_OBJ*)obj->Data;

    // set default model
    obj->DefaultModel = LoadOneLevelModel(LEVEL_MODEL_TRACKSCREEN, TRUE, obj->renderflag, 0);
    if (obj->DefaultModel == -1) return FALSE;

    obj->EnvRGB             = 0x101010;
    obj->renderflag.envmap  = FALSE;
    obj->renderflag.envgood = TRUE;
    obj->renderhandler      = (RENDER_HANDLER)DrawSelectTrackScreen;

    // handlers
    obj->aihandler = (AI_HANDLER)TrackScreenHandler;

    // data
    pTrackScreen->State        = TRACKSCREEN_STEADY;
    pTrackScreen->Timer        = 0.0f;
    pTrackScreen->CurrentLevel = -1;
    pTrackScreen->TPage        = TPAGE_MISC2;

    // get coords of screen
    for( int ii = 0; ii < 4; ii++ ) 
	{
        VecMulMat(&g_SelectTrackScreenPos[ii], &obj->body.Centre.WMatrix, &pTrackScreen->Poly.Pos[ii]);
        VecPlusEqVec(&pTrackScreen->Poly.Pos[ii], &obj->body.Centre.Pos);
    }

    // shift screen out a fraction
    PLANE plane;
    BuildPlane(&pTrackScreen->Poly.Pos[0], &pTrackScreen->Poly.Pos[1], &pTrackScreen->Poly.Pos[2], &plane);
    for( ii = 0; ii < 4; ii++ ) 
	{
        VecMinusEqVec(&pTrackScreen->Poly.Pos[ii], PlaneNormal(&plane));
    }

    // Set up initial UVs
    pTrackScreen->Poly.Verts[0].tu = 0.0f;
    pTrackScreen->Poly.Verts[3].tu = 0.0f;
    pTrackScreen->Poly.Verts[1].tu = 1.0f;
    pTrackScreen->Poly.Verts[2].tu = 1.0f;
    pTrackScreen->Poly.Verts[0].tv = 0.0f;
    pTrackScreen->Poly.Verts[1].tv = 0.0f;
    pTrackScreen->Poly.Verts[2].tv = 1.0f;
    pTrackScreen->Poly.Verts[3].tv = 1.0f;

    // No collision
    obj->CollType = COLL_TYPE_NONE;
    obj->body.CollSkin.AllowObjColls = FALSE;
    obj->body.CollSkin.AllowWorldColls = FALSE;
    
    return TRUE;
}




//-----------------------------------------------------------------------------
// Name: DrawSelectTrackScreen()
// Desc: Render Track Select Screen
//-----------------------------------------------------------------------------
void DrawSelectTrackScreen( OBJECT* obj )
{
    int j;
    OBJECT_TRACKSCREEN_OBJ *pTrackScreen = (OBJECT_TRACKSCREEN_OBJ*)obj->Data;
    VERTEX_TEX1 destVerts[4];
    long g;

    // Draw the model
    RenderObjectModel(&obj->body.Centre.WMatrix, &obj->body.Centre.Pos, &LevelModel[obj->DefaultModel].Model, obj->EnvRGB, obj->EnvOffsetX, obj->EnvOffsetY, obj->EnvScale, obj->renderflag, FALSE);

    // Setup screen
    pTrackScreen->Poly.VertNum = 4;
    pTrackScreen->Poly.Tpage = pTrackScreen->TPage;
    pTrackScreen->Poly.Fog = FALSE;
    pTrackScreen->Poly.SemiType = -1;

    // Colour
    g = 255 - (long)(pTrackScreen->Timer * 1280);
    pTrackScreen->Poly.Verts[0].color = 
    pTrackScreen->Poly.Verts[1].color = 
    pTrackScreen->Poly.Verts[2].color = 
    pTrackScreen->Poly.Verts[3].color = g << 16 | g << 8 | g;//0x00FFFFFF;

    // Calculate UVs
    destVerts[0].tu = 0.0f - pTrackScreen->Timer * 60;
    destVerts[3].tu = 0.0f + pTrackScreen->Timer * 60;
    destVerts[1].tu = 1.0f - pTrackScreen->Timer * 60;
    destVerts[2].tu = 1.0f + pTrackScreen->Timer * 60;
    destVerts[0].tv = 0.0f - pTrackScreen->Timer * 3 + pTrackScreen->Timer * 6;
    destVerts[1].tv = 0.0f - pTrackScreen->Timer * 3 + pTrackScreen->Timer * 6;
    destVerts[2].tv = 1.0f + pTrackScreen->Timer * 3 + pTrackScreen->Timer * 6;
    destVerts[3].tv = 1.0f + pTrackScreen->Timer * 3 + pTrackScreen->Timer * 6;

    if (g_TitleScreenData.mirror) 
	{
        FLOAT tu, tv;

        tu = destVerts[0].tu;
        tv = destVerts[0].tv;
        destVerts[0].tu = destVerts[1].tu;
        destVerts[0].tv = destVerts[1].tv;
        destVerts[1].tu = tu;
        destVerts[1].tv = tv;

        tu = destVerts[2].tu;
        tv = destVerts[2].tv;
        destVerts[2].tu = destVerts[3].tu;
        destVerts[2].tv = destVerts[3].tv;
        destVerts[3].tu = tu;
        destVerts[3].tv = tv;
    }

    if (g_TitleScreenData.reverse) 
	{
        FLOAT tu, tv;

        tu = destVerts[0].tu;
        tv = destVerts[0].tv;
        destVerts[0].tu = destVerts[3].tu;
        destVerts[0].tv = destVerts[3].tv;
        destVerts[3].tu = tu;
        destVerts[3].tv = tv;

        tu = destVerts[1].tu;
        tv = destVerts[1].tv;
        destVerts[1].tu = destVerts[2].tu;
        destVerts[1].tv = destVerts[2].tv;
        destVerts[2].tu = tu;
        destVerts[2].tv = tv;
    }

    // Set UVs
    if( pTrackScreen->Timer > 0.0f ) 
	{
        pTrackScreen->Poly.Verts[0].tu = destVerts[0].tu;
        pTrackScreen->Poly.Verts[0].tv = destVerts[0].tv;
        pTrackScreen->Poly.Verts[1].tu = destVerts[1].tu;
        pTrackScreen->Poly.Verts[1].tv = destVerts[1].tv;
        pTrackScreen->Poly.Verts[2].tu = destVerts[2].tu;
        pTrackScreen->Poly.Verts[2].tv = destVerts[2].tv;
        pTrackScreen->Poly.Verts[3].tu = destVerts[3].tu;
        pTrackScreen->Poly.Verts[3].tv = destVerts[3].tv;
    } 
	else 
	{
        FLOAT scale = TimeStep * 10;
        pTrackScreen->Poly.Verts[0].tu += (destVerts[0].tu - pTrackScreen->Poly.Verts[0].tu) * scale;
        pTrackScreen->Poly.Verts[0].tv += (destVerts[0].tv - pTrackScreen->Poly.Verts[0].tv) * scale;
        pTrackScreen->Poly.Verts[1].tu += (destVerts[1].tu - pTrackScreen->Poly.Verts[1].tu) * scale;
        pTrackScreen->Poly.Verts[1].tv += (destVerts[1].tv - pTrackScreen->Poly.Verts[1].tv) * scale;
        pTrackScreen->Poly.Verts[2].tu += (destVerts[2].tu - pTrackScreen->Poly.Verts[2].tu) * scale;
        pTrackScreen->Poly.Verts[2].tv += (destVerts[2].tv - pTrackScreen->Poly.Verts[2].tv) * scale;
        pTrackScreen->Poly.Verts[3].tu += (destVerts[3].tu - pTrackScreen->Poly.Verts[3].tu) * scale;
        pTrackScreen->Poly.Verts[3].tv += (destVerts[3].tv - pTrackScreen->Poly.Verts[3].tv) * scale;
    }

    for (j = 0 ; j < pTrackScreen->Poly.VertNum ; j++)
        RotTransPersVector(&ViewMatrixScaled, &ViewTransScaled, &pTrackScreen->Poly.Pos[j], (FLOAT*)&pTrackScreen->Poly.Verts[j]);

    SET_TPAGE((short)pTrackScreen->Poly.Tpage);

    TEXTURE_ADDRESS(D3DTADDRESS_WRAP);
    DRAW_PRIM(D3DPT_TRIANGLEFAN, FVF_TEX1, pTrackScreen->Poly.Verts, pTrackScreen->Poly.VertNum, D3DDP_DONOTUPDATEEXTENTS);
    TEXTURE_ADDRESS(D3DTADDRESS_CLAMP);

}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
VOID LoadSelectTrackTexture( OBJECT* obj )
{
    char tPage;
    char buf[256];
    OBJECT_TRACKSCREEN_OBJ *pTrackScreen = (OBJECT_TRACKSCREEN_OBJ*)obj->Data;
    LEVELINFO *pLevelInfo = GetLevelInfo(g_iTrackScreenLevelNum);

    if (pTrackScreen->TPage == TPAGE_MISC1)
        tPage = TPAGE_MISC2;
    else
        tPage = TPAGE_MISC1;
    FreeOneTexture(tPage);

    if( pLevelInfo != NULL )
        sprintf(buf, "D:\\gfx\\%s.bmp", pLevelInfo->Dir);
    else
        sprintf(buf, "D:\\gfx\\Acclaim.bmp");

    if (!LoadTextureClever(buf, tPage, 256, 256, 0, FxTextureSet, 1))
	{
        LoadTextureClever("D:\\gfx\\Acclaim.bmp", tPage, 256, 256, 0, FxTextureSet, 1);
    }
}




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
void DrawTrackName( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem, int itemIndex )
{
    LEVELINFO* pLevelInfo = GetLevelInfo( g_TitleScreenData.iLevelNum );

    FLOAT xPos = pMenuHeader->XPos;
    FLOAT yPos = pMenuHeader->YPos;

    DrawSpruBox( gMenuWidthScale * (xPos - MENU_TEXT_GAP), 
                 gMenuHeightScale * (yPos - MENU_TEXT_GAP),
                 gMenuWidthScale * (MENU_DATA_WIDTH_TEXT + MENU_TEXT_GAP * 2),
                 gMenuHeightScale * ((MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP) * 3 + MENU_TEXT_GAP * 2), 
                 pMenuHeader->SpruColIndex, 0 );

    // Track Name
    sprintf(MenuBuffer, "%s %s%s", pLevelInfo->Name, 
                                   (g_TitleScreenData.mirror)? TEXT_TABLE(TEXT_MIRROR_ABREV_PARENTHESIS) : "",
                                   (g_TitleScreenData.reverse)? TEXT_TABLE(TEXT_REVERSE_ABREV_PARENTHESIS) : "");

    long color;
    if( IsLevelTypeAvailable(g_TitleScreenData.iLevelNum, g_TitleScreenData.mirror, g_TitleScreenData.reverse) )
        color = MENU_TEXT_RGB_CHOICE;
    else
        color = MENU_TEXT_RGB_LOLITE;

    DrawMenuText(xPos, yPos, color, MenuBuffer);
    yPos += MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP * 2;

    // Track Length
    DrawMenuText(xPos, yPos, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_LENGTH));
    if ((pLevelInfo->TrackType == TRACK_TYPE_BATTLE) || (pLevelInfo->TrackType == TRACK_TYPE_TRAINING))
        sprintf(MenuBuffer, "n/a" );
    else if (pLevelInfo->Length != 0.0f)
        sprintf(MenuBuffer, "%d metres", (int)pLevelInfo->Length);
    else
        sprintf(MenuBuffer, "Unknown");
    DrawMenuText(xPos+15*MENU_TEXT_WIDTH, yPos, MENU_TEXT_RGB_NORMAL, MenuBuffer);
    yPos += MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP;

    // Difficulty
    DrawMenuText(xPos, yPos, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_DIFFICULTY));
    sprintf(MenuBuffer, "%s", pLevelInfo->LevelClass  < RACE_CLASS_BRONZE ? "Unknown" : TEXT_TABLE(TEXT_EASY + pLevelInfo->LevelClass - 1));
    DrawMenuText(xPos+15*MENU_TEXT_WIDTH, yPos, MENU_TEXT_RGB_NORMAL, MenuBuffer);

    {
        if (!IsLevelTypeAvailable(g_TitleScreenData.iLevelNum, g_TitleScreenData.mirror, g_TitleScreenData.reverse))
		{
            if (!(TIME2MS(TimerCurrent) & 128)) 
			{
                sprintf(MenuBuffer, "Locked");
                xPos = (640 - strlen(MenuBuffer) * (FLOAT)(MENU_TEXT_WIDTH)) / 2;
                yPos = (480 - (FLOAT)(MENU_TEXT_HEIGHT)) / 2;
                DrawSpruBox( gMenuWidthScale * (xPos - MENU_TEXT_HSKIP), 
                             gMenuHeightScale * (yPos - MENU_TEXT_VSKIP),
                             gMenuWidthScale * (strlen(MenuBuffer) * MENU_TEXT_WIDTH + MENU_TEXT_HSKIP * 2),
                             gMenuHeightScale * (MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP * 2),
                             pMenuHeader->SpruColIndex, 0 );

                DrawMenuText( xPos, yPos, MENU_TEXT_RGB_HILITE, MenuBuffer );
            }
        }
    }
}




//-----------------------------------------------------------------------------
// Name: InitSmallScreen()
// Desc: Init Small Screen
//-----------------------------------------------------------------------------
LONG InitSmallScreen( OBJECT* obj, long* flags )
{
    OBJECT_SMALLSCREEN_OBJ* pSmallScreen = (OBJECT_SMALLSCREEN_OBJ*)obj->Data;

    obj->DefaultModel = LoadOneLevelModel(LEVEL_MODEL_SMALLSCREEN, TRUE, obj->renderflag, 0);
    if( obj->DefaultModel == -1 )
		return FALSE;

    obj->EnvRGB             = 0x101010;
    obj->renderflag.envmap  = FALSE;
    obj->renderflag.envgood = TRUE;
    obj->renderhandler      = (RENDER_HANDLER)DrawSmallScreen;

    obj->aihandler = (AI_HANDLER)SmallScreenHandler;

    pSmallScreen->Timer = 0.0f;
    pSmallScreen->TPage = TPAGE_MISC2;
    pSmallScreen->CurrentLevel = -1;

    // get coords of screen
    for( int ii = 0; ii < 4; ii++ )
	{
        VecMulMat( &g_SmallScreenPos[ii], &obj->body.Centre.WMatrix, &pSmallScreen->Poly.Pos[ii] );
        VecPlusEqVec( &pSmallScreen->Poly.Pos[ii], &obj->body.Centre.Pos );
    }

    // Shift screen out a fraction
    PLANE plane;
    BuildPlane( &pSmallScreen->Poly.Pos[0], &pSmallScreen->Poly.Pos[1], &pSmallScreen->Poly.Pos[2], &plane );
    for( ii = 0; ii < 4; ii++ ) 
	{
        VecMinusEqVec( &pSmallScreen->Poly.Pos[ii], PlaneNormal(&plane) );
    }

    pSmallScreen->Poly.VertNum  = 4;
    pSmallScreen->Poly.Tpage    = pSmallScreen->TPage;
    pSmallScreen->Poly.Fog      = FALSE;
    pSmallScreen->Poly.SemiType = -1;

    pSmallScreen->Poly.Verts[0].color = 0xffa0a0a0; 
    pSmallScreen->Poly.Verts[1].color = 0xffa0a0a0;
    pSmallScreen->Poly.Verts[2].color = 0xffa0a0a0;
    pSmallScreen->Poly.Verts[3].color = 0xffa0a0a0;

    // Set up initial UVs
    pSmallScreen->Poly.Verts[0].tu = 0.0f;
    pSmallScreen->Poly.Verts[3].tu = 0.0f;
    pSmallScreen->Poly.Verts[1].tu = 1.0f;
    pSmallScreen->Poly.Verts[2].tu = 1.0f;
    pSmallScreen->Poly.Verts[0].tv = 0.0f;
    pSmallScreen->Poly.Verts[1].tv = 0.0f;
    pSmallScreen->Poly.Verts[2].tv = 1.0f;
    pSmallScreen->Poly.Verts[3].tv = 1.0f;

    // No collision
    obj->CollType = COLL_TYPE_NONE;
    obj->body.CollSkin.AllowObjColls   = FALSE;
    obj->body.CollSkin.AllowWorldColls = FALSE;
    
    return TRUE;
}




//-----------------------------------------------------------------------------
// Name: SmallScreenHandler()
// Desc: Small Screen Handler
//-----------------------------------------------------------------------------
VOID SmallScreenHandler( OBJECT* obj )
{
    /*
	OBJECT_SMALLSCREEN_OBJ* pSmallScreen = (OBJECT_SMALLSCREEN_OBJ*)obj->Data;

    if( g_TitleScreenData.iLevelNum != pSmallScreen->CurrentLevel ) 
	{
        char buf[256];
        pSmallScreen->CurrentLevel = g_TitleScreenData.iLevelNum;

	    LEVELINFO* pLevelInfo = GetLevelInfo( g_TitleScreenData.iLevelNum );

        if( pLevelInfo != NULL ) 
            sprintf( buf, "gfx\\%s.bmp", pLevelInfo->Dir );
        else
            sprintf( buf, "gfx\\Acclaim.bmp" );

        if( !LoadTextureClever(buf, TPAGE_MISC2, 256, 256, 0, FxTextureSet, 1) ) 
            LoadTextureClever( "gfx\\Acclaim.bmp", TPAGE_MISC2, 256, 256, 0, FxTextureSet, 1 );
    }
	*/
}




//-----------------------------------------------------------------------------
// Name: DrawSmallScreen()
// Desc: Draw Small Screen
//-----------------------------------------------------------------------------
VOID DrawSmallScreen( OBJECT* obj )
{
    OBJECT_SMALLSCREEN_OBJ* pSmallScreen = (OBJECT_SMALLSCREEN_OBJ*)obj->Data;

    // Draw the model
    RenderObjectModel( &obj->body.Centre.WMatrix, &obj->body.Centre.Pos, 
		               &LevelModel[obj->DefaultModel].Model, obj->EnvRGB, 
					   obj->EnvOffsetX, obj->EnvOffsetY, obj->EnvScale, 
					   obj->renderflag, FALSE );

    // Draw the small screen poly
    for( int j = 0; j < pSmallScreen->Poly.VertNum; j++ )
	{
        RotTransPersVector( &ViewMatrixScaled, &ViewTransScaled, 
		                    &pSmallScreen->Poly.Pos[j],
							(FLOAT*)&pSmallScreen->Poly.Verts[j] );
	}

    pSmallScreen->Poly.Tpage = g_lSmallScreenTPage;

    SET_TPAGE( (short)pSmallScreen->Poly.Tpage );

    DRAW_PRIM( D3DPT_TRIANGLEFAN, FVF_TEX1, pSmallScreen->Poly.Verts,  
		       pSmallScreen->Poly.VertNum, D3DDP_DONOTUPDATEEXTENTS );
}




