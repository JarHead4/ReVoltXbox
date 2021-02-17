//-----------------------------------------------------------------------------
// File: ui_TrackScreen.cpp
//
// Desc: UI implementation
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

#include "ui_Menu.h"
#include "ui_menudraw.h"
#include "ui_TitleScreen.h"
#include "ui_StateEngine.h"
#include "ui_SelectTrack.h"


///////////////////////////
// object init functions //
///////////////////////////
static long InitTrackScreen(OBJECT *obj, long *flags);
static void LoadTrackSelectTexture(OBJECT *obj);
static void TrackScreenHandler(OBJECT *obj);
static void DrawTrackSelectScreen(OBJECT *obj);

static long InitSmallScreen(OBJECT *obj, long *flags);
static void SmallScreenHandler(OBJECT *obj);


// Register the object init data
REGISTER_OBJECT( OBJECT_TYPE_SMALLSCREEN, InitSmallScreen, sizeof(SMALLSCREEN_OBJ) );
REGISTER_OBJECT( OBJECT_TYPE_TRACKSCREEN, InitTrackScreen, sizeof(OBJECT_TRACKSCREEN_OBJ) );



////////////////////////////////////////////////////////////////
//
// Init Track Screen
//
////////////////////////////////////////////////////////////////
VEC gTrackSelectScreenPos[4] =
{
    {-299.0f, -726.4f, -75.2f},
    { 299.0f, -726.4f, -75.2f},
    { 299.0f, -291.6f,   5.0f},
    {-299.0f, -291.6f,   5.0f},
};

static long InitTrackScreen(OBJECT *obj, long *flags)
{
    int ii;
    PLANE plane;
    OBJECT_TRACKSCREEN_OBJ *trackScreen = (OBJECT_TRACKSCREEN_OBJ*)obj->Data;

    // set default model
    obj->DefaultModel = LoadOneLevelModel(LEVEL_MODEL_TRACKSCREEN, TRUE, obj->renderflag, 0);
    if (obj->DefaultModel == -1) return FALSE;

    obj->renderflag.envmap = FALSE;
    obj->renderflag.envgood = TRUE;
    obj->EnvRGB = 0x101010;
    obj->renderhandler = (RENDER_HANDLER)DrawTrackSelectScreen;

    // handlers
    obj->aihandler = (AI_HANDLER)TrackScreenHandler;

    // data
    trackScreen->State = TRACKSCREEN_STEADY;
    trackScreen->Timer = ZERO;
    trackScreen->CurrentLevel = -1;
    trackScreen->TPage = TPAGE_MISC2;

    // get coords of screen
    for (ii = 0; ii < 4; ii++) {
        VecMulMat(&gTrackSelectScreenPos[ii], &obj->body.Centre.WMatrix, &trackScreen->Poly.Pos[ii]);
        VecPlusEqVec(&trackScreen->Poly.Pos[ii], &obj->body.Centre.Pos);
    }

    // shift screen out a fraction
    BuildPlane(&trackScreen->Poly.Pos[0], &trackScreen->Poly.Pos[1], &trackScreen->Poly.Pos[2], &plane);
    for (ii = 0; ii < 4; ii++) {
        VecMinusEqVec(&trackScreen->Poly.Pos[ii], PlaneNormal(&plane));
    }

    // Set up initial UVs
    trackScreen->Poly.Verts[0].tu = 0.0f;
    trackScreen->Poly.Verts[3].tu = 0.0f;
    trackScreen->Poly.Verts[1].tu = 1.0f;
    trackScreen->Poly.Verts[2].tu = 1.0f;
    trackScreen->Poly.Verts[0].tv = 0.0f;
    trackScreen->Poly.Verts[1].tv = 0.0f;
    trackScreen->Poly.Verts[2].tv = 1.0f;
    trackScreen->Poly.Verts[3].tv = 1.0f;

    // No collision
    obj->CollType = COLL_TYPE_NONE;
    obj->body.CollSkin.AllowObjColls = FALSE;
    obj->body.CollSkin.AllowWorldColls = FALSE;

    return TRUE;
}




/////////////////////////////////////////////////////////////////////////////////
//
// LoadTrackSelectTexture
//
/////////////////////////////////////////////////////////////////////////////////

void LoadTrackSelectTexture(OBJECT *obj)
{
    char tPage;
    char buf[256];
    OBJECT_TRACKSCREEN_OBJ *trackScreen = (OBJECT_TRACKSCREEN_OBJ*)obj->Data;
    LEVELINFO *levelInfo = GetLevelInfo(gTrackScreenLevelNum);

    if (trackScreen->TPage == TPAGE_MISC1) {
        tPage = TPAGE_MISC2;
    } else {
        tPage = TPAGE_MISC1;
    }
    FreeOneTexture(tPage);

    if (levelInfo != NULL) {
        sprintf(buf, "D:\\gfx\\%s.bmp", levelInfo->szName);
    } else {
        sprintf(buf, "D:\\gfx\\Acclaim.bmp");
    }

    if (!LoadTextureClever(buf, tPage, 256, 256, 0, FxTextureSet, 1)) {
        LoadTextureClever("D:\\gfx\\Acclaim.bmp", tPage, 256, 256, 0, FxTextureSet, 1);
    }
}



////////////////////////////////////////////////////////////////
//
// TrackScreenHandler
//
////////////////////////////////////////////////////////////////
#define TRACKSCREEN_WOBBLE_TIME TO_TIME(0.1f)

void TrackScreenHandler(OBJECT *obj)
{
    OBJECT_TRACKSCREEN_OBJ *trackScreen = (OBJECT_TRACKSCREEN_OBJ*)obj->Data;

    switch (trackScreen->State) {

    case TRACKSCREEN_STEADY:

        // reduce wobble time
        trackScreen->Timer -= TimeStep;
        if (trackScreen->Timer < ZERO) {
            trackScreen->Timer = ZERO;
        }

        // Change state if level has changed
        if (gTrackScreenLevelNum != trackScreen->CurrentLevel) {
            trackScreen->CurrentLevel = gTrackScreenLevelNum;
            trackScreen->State = TRACKSCREEN_WOBBLY;
            if (g_pMenuHeader->m_pMenu == &Menu_TrackSelect)
            {
#ifdef OLD_AUDIO
                PlaySfx(SFX_TVSTATIC, SFX_MAX_VOL, SFX_CENTRE_PAN, SFX_SAMPLE_RATE, 0x7fffffff);
#else
                g_SoundEngine.Play2DSound( EFFECT_TVStatic, FALSE );
#endif // OLD_AUDIO
            }
            LoadTrackSelectTexture(obj);
        }
        break;

    case TRACKSCREEN_WOBBLY:

        trackScreen->Timer += TimeStep;
        if (trackScreen->Timer > TRACKSCREEN_WOBBLE_TIME) {
            trackScreen->Timer = TRACKSCREEN_WOBBLE_TIME;
            trackScreen->State = TRACKSCREEN_STEADY;
            if (trackScreen->TPage == TPAGE_MISC1) {
                gCurrentScreenTPage = trackScreen->TPage = TPAGE_MISC2;
            } else {
                gCurrentScreenTPage = trackScreen->TPage = TPAGE_MISC1;
            }
        }
        break;

    default:
        break;
    }

}




/////////////////////////////////////////////////////////////////////////////////
//
// Render Track Select Screen
//
/////////////////////////////////////////////////////////////////////////////////
void DrawTrackSelectScreen(OBJECT *obj)
{
    int j;
    OBJECT_TRACKSCREEN_OBJ *trackScreen = (OBJECT_TRACKSCREEN_OBJ*)obj->Data;
    VERTEX_TEX1 destVerts[4];
    long g;

    // Draw the model
    RenderObjectModel(&obj->body.Centre.WMatrix, &obj->body.Centre.Pos, &LevelModel[obj->DefaultModel].Model, obj->EnvRGB, obj->EnvOffsetX, obj->EnvOffsetY, obj->EnvScale, obj->renderflag, FALSE);

    // Setup screen
    trackScreen->Poly.VertNum = 4;
    trackScreen->Poly.Tpage = trackScreen->TPage;
    trackScreen->Poly.Fog = FALSE;
    trackScreen->Poly.SemiType = -1;

    // Colour
    g = 255 - (long)(trackScreen->Timer * 1280);
    trackScreen->Poly.Verts[0].color = 
    trackScreen->Poly.Verts[1].color = 
    trackScreen->Poly.Verts[2].color = 
    trackScreen->Poly.Verts[3].color = g << 16 | g << 8 | g;//0x00FFFFFF;

    // Calculate UVs
    destVerts[0].tu = 0.0f - trackScreen->Timer * 60;
    destVerts[3].tu = 0.0f + trackScreen->Timer * 60;
    destVerts[1].tu = 1.0f - trackScreen->Timer * 60;
    destVerts[2].tu = 1.0f + trackScreen->Timer * 60;
    destVerts[0].tv = 0.0f - trackScreen->Timer * 3 + trackScreen->Timer * 6;
    destVerts[1].tv = 0.0f - trackScreen->Timer * 3 + trackScreen->Timer * 6;
    destVerts[2].tv = 1.0f + trackScreen->Timer * 3 + trackScreen->Timer * 6;
    destVerts[3].tv = 1.0f + trackScreen->Timer * 3 + trackScreen->Timer * 6;

    if (gTitleScreenVars.mirror) {
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

    if (gTitleScreenVars.reverse) {
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
    if (trackScreen->Timer > ZERO) {
        trackScreen->Poly.Verts[0].tu = destVerts[0].tu;
        trackScreen->Poly.Verts[0].tv = destVerts[0].tv;
        trackScreen->Poly.Verts[1].tu = destVerts[1].tu;
        trackScreen->Poly.Verts[1].tv = destVerts[1].tv;
        trackScreen->Poly.Verts[2].tu = destVerts[2].tu;
        trackScreen->Poly.Verts[2].tv = destVerts[2].tv;
        trackScreen->Poly.Verts[3].tu = destVerts[3].tu;
        trackScreen->Poly.Verts[3].tv = destVerts[3].tv;
    } else {
        FLOAT scale = TimeStep * 10;
        trackScreen->Poly.Verts[0].tu += (destVerts[0].tu - trackScreen->Poly.Verts[0].tu) * scale;
        trackScreen->Poly.Verts[0].tv += (destVerts[0].tv - trackScreen->Poly.Verts[0].tv) * scale;
        trackScreen->Poly.Verts[1].tu += (destVerts[1].tu - trackScreen->Poly.Verts[1].tu) * scale;
        trackScreen->Poly.Verts[1].tv += (destVerts[1].tv - trackScreen->Poly.Verts[1].tv) * scale;
        trackScreen->Poly.Verts[2].tu += (destVerts[2].tu - trackScreen->Poly.Verts[2].tu) * scale;
        trackScreen->Poly.Verts[2].tv += (destVerts[2].tv - trackScreen->Poly.Verts[2].tv) * scale;
        trackScreen->Poly.Verts[3].tu += (destVerts[3].tu - trackScreen->Poly.Verts[3].tu) * scale;
        trackScreen->Poly.Verts[3].tv += (destVerts[3].tv - trackScreen->Poly.Verts[3].tv) * scale;
    }

    for (j = 0 ; j < trackScreen->Poly.VertNum ; j++)
        RotTransPersVector(&ViewMatrixScaled, &ViewTransScaled, &trackScreen->Poly.Pos[j], (FLOAT*)&trackScreen->Poly.Verts[j]);

    SET_TPAGE((short)trackScreen->Poly.Tpage);

    TEXTURE_ADDRESS(D3DTADDRESS_WRAP);
    DRAW_PRIM(D3DPT_TRIANGLEFAN, FVF_TEX1, trackScreen->Poly.Verts, trackScreen->Poly.VertNum, D3DDP_DONOTUPDATEEXTENTS);
    TEXTURE_ADDRESS(D3DTADDRESS_CLAMP);
}




////////////////////////////////////////////////////////////////
//
// InitSmallScreen:
//
////////////////////////////////////////////////////////////////
VEC gSmallScreenPos[4] =
{
    {-129.0f, -326.6f, -23.1f},
    { 132.0f, -326.6f, -23.1f},
    { 132.0f, -133.5f, -23.1f},
    {-129.0f, -133.5f, -23.1f},
};




long InitSmallScreen(OBJECT *obj, long *flags)
{
    
    int ii;
    SMALLSCREEN_OBJ *screen = (SMALLSCREEN_OBJ*)obj->Data;
    PLANE plane;

    obj->DefaultModel = LoadOneLevelModel(LEVEL_MODEL_SMALLSCREEN, TRUE, obj->renderflag, 0);
    if (obj->DefaultModel == -1) return FALSE;

    obj->renderflag.envmap = FALSE;
    obj->renderflag.envgood = TRUE;
    obj->EnvRGB = 0x101010;
    obj->renderhandler = (RENDER_HANDLER)DrawSmallScreen;

    obj->aihandler = (AI_HANDLER)SmallScreenHandler;

    screen->Timer = ZERO;
    screen->TPage = TPAGE_MISC2;
    screen->CurrentLevel = -1;

    // get coords of screen
    for (ii = 0; ii < 4; ii++) {
        VecMulMat(&gSmallScreenPos[ii], &obj->body.Centre.WMatrix, &screen->Poly.Pos[ii]);
        VecPlusEqVec(&screen->Poly.Pos[ii], &obj->body.Centre.Pos);
    }

    // shift screen out a fraction
    BuildPlane(&screen->Poly.Pos[0], &screen->Poly.Pos[1], &screen->Poly.Pos[2], &plane);
    for (ii = 0; ii < 4; ii++) {
        VecMinusEqVec(&screen->Poly.Pos[ii], PlaneNormal(&plane));
    }

    screen->Poly.VertNum = 4;
    screen->Poly.Tpage = screen->TPage;
    screen->Poly.Fog = FALSE;
    screen->Poly.SemiType = -1;

    screen->Poly.Verts[0].color = 
    screen->Poly.Verts[1].color = 
    screen->Poly.Verts[2].color = 
    screen->Poly.Verts[3].color = 0xFFA0A0A0;

    // Set up initial UVs
    screen->Poly.Verts[0].tu = 0.0f;
    screen->Poly.Verts[3].tu = 0.0f;
    screen->Poly.Verts[1].tu = 1.0f;
    screen->Poly.Verts[2].tu = 1.0f;
    screen->Poly.Verts[0].tv = 0.0f;
    screen->Poly.Verts[1].tv = 0.0f;
    screen->Poly.Verts[2].tv = 1.0f;
    screen->Poly.Verts[3].tv = 1.0f;

    // No collision
    obj->CollType = COLL_TYPE_NONE;
    obj->body.CollSkin.AllowObjColls = FALSE;
    obj->body.CollSkin.AllowWorldColls = FALSE;
    
    return TRUE;
}




////////////////////////////////////////////////////////////////
//
// Small Screen Handler
//
////////////////////////////////////////////////////////////////
void SmallScreenHandler(OBJECT *obj)
{
    /*SMALLSCREEN_OBJ *screen = (SMALLSCREEN_OBJ*)obj->Data;
    LEVELINFO *levelInfo;

    if (gTitleScreenVars.iLevelNum != screen->CurrentLevel) {
        char buf[256];
        screen->CurrentLevel = gTitleScreenVars.iLevelNum;

        levelInfo = GetLevelInfo(gTitleScreenVars.iLevelNum);

        if (levelInfo != NULL) {
            sprintf(buf, "gfx\\%s.bmp", levelInfo->Dir);
        } else {
            sprintf(buf, "gfx\\Acclaim.bmp");
        }

        if (!LoadTextureClever(buf, TPAGE_MISC2, 256, 256, 0, FxTextureSet, 1)) {
            LoadTextureClever("gfx\\Acclaim.bmp", TPAGE_MISC2, 256, 256, 0, FxTextureSet, 1);
        }
    }*/
}




