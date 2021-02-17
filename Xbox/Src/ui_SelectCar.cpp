//-----------------------------------------------------------------------------
// File: ui_SelectCar.cpp
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
#include "ui_menudraw.h"
#include "initplay.h"
#include "pickup.h"
#include "SoundEffectEngine.h"
#include "Text.h"
#include "Cheats.h"
#include "settings.h"	// $TEMPORARY jedl - for now, force car box to be drawn using CPU

#include "ui_Menu.h"
#include "ui_MenuDraw.h"
#include "ui_MenuText.h"
#include "ui_TitleScreen.h"
#include "ui_StateEngine.h"
#include "ui_EnterName.h"
#include "ui_SelectCar.h"
#include "ui_SelectCup.h"
#include "ui_SelectTrack.h"
#include "ui_WaitingRoom.h"
#include "ui_RaceOverview.h"

extern void DrawNewTitleBox( FLOAT, FLOAT, FLOAT, FLOAT, int, int );   

#define MENU_CARSELECT_XPOS              48.0f
#define MENU_CARSELECT_YPOS             100.0f


///////////////////////////
// object init functions //
///////////////////////////

static long InitCarBox(OBJECT *obj, long *flags);
static void CarBoxHandler(OBJECT *obj);
extern void InitCarBoxStuff();
static void InterpCarBoxPos(VEC *pos, VEC *dest, FLOAT xScale, FLOAT yScale, FLOAT zScale);
static void InterpCarBoxQuat(QUATERNION *quat, QUATERNION *destQuat, FLOAT scale);
static void DrawCarBox(OBJECT *obj);

static BOOL HandleCarSelectMenu( MENU_HEADER* pMenuHeader, DWORD dwInput );
static VOID DrawCarSelectMenu( MENU_HEADER* pMenuHeader, MENU* pMenu );

static BOOL HandleCarSelectedMenu( MENU_HEADER* pMenuHeader, DWORD dwInput );

/*
static PLAYER*      gCarSelectPlayer = NULL;
static VEC          gCarBoxSelectPos = {0,0,0};
static VEC          gCarBoxOpenPos   = {0,0,0};
static QUATERNION   gCarBoxOpenQuat  = {0,0,0,ONE};
static MAT          gCarDropMat;

//extern CAMERA*      gCamera;

// Data
bool gCarChosen = FALSE;                    // Car has been selected (with return)
bool gCarTaken = FALSE;                     // Car has been dropped, and we are ready to move to next menu
*/



struct PERPLAYERCARSELECTINFO
{
    PLAYER*      pCarSelectPlayer;
    OBJECT_CARBOX_OBJ* pCarBoxObject;
    VEC          vCarBoxSelectPos;
    VEC          vCarBoxOpenPos;
    QUATERNION   qCarBoxOpenQuat;
    MAT          matCarDropMatrix;
    BOOL         bCarChosen;                    // Whether car has been selected (with return)
    BOOL         bCarTaken;                     // Whether car has been dropped, and we are ready to move to next menu
};

static PERPLAYERCARSELECTINFO g_CarSelectInfo[4];



// Register the object init data
REGISTER_OBJECT( OBJECT_TYPE_CARBOX, InitCarBox, sizeof(OBJECT_CARBOX_OBJ) );

extern MENU Menu_CarSelected;

extern MENU_ITEM MenuItem_CarSelect;
extern MENU_ITEM MenuItem_CarSelected;

////////////////////////////////////////////////////////////////
//
// Init Car Box
//
////////////////////////////////////////////////////////////////
static long InitCarBox(OBJECT *obj, long *flags)
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
//    box->State = CARBOX_STACKED;
//    box->Timer = ZERO;
//    box->AtHome = TRUE;
//    CopyVec(&obj->body.Centre.Pos, &box->HomePos);
//    CopyQuat(&obj->body.Centre.Quat, &box->HomeQuat);

    for( int i=0; i<4; i++ )
    {
        CopyVec(&obj->body.Centre.Pos, &box->HomePos[i]);
        CopyQuat(&obj->body.Centre.Quat, &box->HomeQuat[i]);

        box->State[i]   = CARBOX_STACKED;
        box->Timer[i]   = ZERO;
        box->AtHome[i]  = TRUE;
        CopyVec(&obj->body.Centre.Pos, &box->Pos[i]);
        CopyQuat(&obj->body.Centre.Quat, &box->Quat[i]);
    }


    Assert(box->CarType < CARID_NTYPES);

    return TRUE;
}

////////////////////////////////////////////////////////////////
//
// CarBoxHandler
//
////////////////////////////////////////////////////////////////

//  CARBOX_STACKED = 0,
//  CARBOX_SELECTED,
//  CARBOX_CHOOSABLE,
//  CARBOX_OPENING,
//  CARBOX_OPEN,

void CarBoxHandler(OBJECT *obj)
{
    for( int i=0; i<4; i++ )
    {
        VEC dR;
        OBJECT_CARBOX_OBJ *box = (OBJECT_CARBOX_OBJ*)obj->Data;

        Assert(box->State[i] < CARBOX_NSTATES);
        // Only deal with these when in car select menu
        /*
        if( (g_pMenuHeader->m_pMenu != &Menu_CarSelect) && (g_pMenuHeader->m_pMenu != &Menu_CarSelected) )
        {
            if( !box->AtHome[i] ) 
            {
                box->State[i] = CARBOX_STACKED;
                box->Timer[i] = ZERO;
                CopyVec(&box->HomePos, &box->Pos[i]);
                CopyQuat(&box->HomeQuat, &box->Quat[i]);
                QuatToMat(&box->Quat[i], &box->WMatrix[i]);
                box->AtHome[i] = TRUE;
            }
            return;
        }
        */

        // See if box has become unselected
        if ((g_pMenuHeader->m_pMenu != &Menu_CarSelect) && (g_pMenuHeader->m_pMenu != &Menu_CarSelected))
        {
            box->State[i] = CARBOX_STACKED;
            box->Timer[i] = ZERO;
        } 
        else if ((box->CarType == -1) && (gTitleScreenVars.PlayerData[i].iCarNum > CARID_PANGA)) 
        {

        } 
        else if (gTitleScreenVars.PlayerData[i].iCarNum != box->CarType) 
        {
            box->State[i] = CARBOX_STACKED;
            box->Timer[i] = ZERO;
        }

        // Housekeeping
        box->Timer[i] += TimeStep;

        // Handle state
        switch (box->State[i]) 
        {
            case CARBOX_STACKED:
                // Move box back to default position
                if (!box->AtHome[i]) 
                {
                    VecMinusVec(&box->Pos[i], &box->HomePos[i], &dR);
                    if ((abs(dR.v[X]) < 1) && (abs(dR.v[Y]) < 1) && (abs(dR.v[Z]) < 1)) 
                    {
                        CopyVec(&box->HomePos[i], &box->Pos[i]);
                        CopyQuat(&box->HomeQuat[i], &box->Quat[i]);
                        box->AtHome[i] = TRUE;
                    } 
                    else if (box->Timer[i] > TO_TIME(1.5f)) 
                    {
                        CopyVec(&box->HomePos[i], &box->Pos[i]);
                        CopyQuat(&box->HomeQuat[i], &box->Quat[i]);
                        box->AtHome[i] = TRUE;
                    } 
                    else 
                    {
                        InterpCarBoxPos(&box->Pos[i], &box->HomePos[i], 0.2f, 0.3f, 0.2f);
                        InterpCarBoxQuat(&box->Quat[i], &box->HomeQuat[i], 0.85f);
                    }
                }

                // See if it has become selected
                if (!g_CarSelectInfo[i].bCarChosen && 
                    ((gTitleScreenVars.PlayerData[i].iCarNum == box->CarType) || 
                     (gTitleScreenVars.PlayerData[i].iCarNum > 27 && box->CarType == -1)))
                {
                    if (g_pMenuHeader->m_pMenu == &Menu_CarSelect)
                    {
                        box->State[i] = CARBOX_SELECTED;
                        box->Timer[i] = ZERO;
                        box->AtHome[i] = FALSE;
        #ifdef OLD_AUDIO
                        PlaySfx(SFX_BOXENTRY, SFX_MAX_VOL, SFX_CENTRE_PAN, SFX_SAMPLE_RATE, 0x7fffffff);
        #else
                        g_SoundEngine.Play2DSound( EFFECT_BoxSlide, FALSE );
        #endif // OLD_AUDIO
                    }
                }

                break;

            case CARBOX_SELECTED:
                // Select this box for the player
                g_CarSelectInfo[i].pCarBoxObject = box;

                // Move box in front of camera
                InterpCarBoxPos(&box->Pos[i], &g_CarSelectInfo[i].vCarBoxSelectPos, 0.2f, 0.1f, 0.2f);
                InterpCarBoxQuat(&box->Quat[i], &g_pTitleScreenCamera->m_pCamera->Quat, 0.9f);

                // Collide with other boxes
                FLOAT dRLenSq, dRLen, depth;
                OBJECT *otherBoxObj;
                OBJECT_CARBOX_OBJ *otherBox;

                for (otherBoxObj = NextObjectOfType(OBJ_ObjectHead, OBJECT_TYPE_CARBOX); otherBoxObj != NULL; otherBoxObj = NextObjectOfType(otherBoxObj->next, OBJECT_TYPE_CARBOX)) 
                {
                    if (otherBoxObj == obj) continue;
                    otherBox = (OBJECT_CARBOX_OBJ*)otherBoxObj->Data;
                    if (otherBox->AtHome[i]) continue;

                    VecMinusVec(&box->Pos[i], &otherBox->Pos[i], &dR);
                    dRLenSq = VecDotVec(&dR, &dR);
                    if (dRLenSq < 250*250 && dRLenSq > SMALL_REAL) 
                    {
                        dRLen = (FLOAT)sqrt(dRLenSq);
                        depth = (250 - dRLen)/2;
                        if (abs(otherBox->CarType - box->CarType) < 2) 
                        {
                            dR.v[Y] = ZERO;
                        } 
                        else 
                        {
                            dR.v[Y] /= 5;
                        }
                        VecPlusEqScalarVec(&otherBox->Pos[i], -depth/dRLen, &dR);
                        VecPlusEqScalarVec(&box->Pos[i], depth/dRLen, &dR);
                    }
                }

                // Has car been selected?
                if (g_CarSelectInfo[i].bCarChosen) 
                {
                    box->State[i] = CARBOX_OPENING;
                    box->Timer[i] = ZERO;
                    ConstrainQuat2(&g_CarSelectInfo[i].qCarBoxOpenQuat, &box->Quat[i]);
                }
                break;

            case CARBOX_OPENING:
                // Move box to drop position
                InterpCarBoxPos(&box->Pos[i], &g_CarSelectInfo[i].vCarBoxOpenPos, 0.2f, 0.1f, 0.2f);
                InterpCarBoxQuat(&box->Quat[i], &g_CarSelectInfo[i].qCarBoxOpenQuat, 0.9f);
                if (g_CarSelectInfo[i].pCarSelectPlayer) 
                {
                    g_CarSelectInfo[i].pCarSelectPlayer->car.AerialTimer -= TimeStep * 2;
                    if (g_CarSelectInfo[i].pCarSelectPlayer->car.AerialTimer < TO_TIME(0.2f)) 
                        g_CarSelectInfo[i].pCarSelectPlayer->car.AerialTimer = TO_TIME(0.2f);
                }
                if (box->Timer[i] > TO_TIME(1.0f)) 
                {
                    VEC carPos;
                    if (g_CarSelectInfo[i].pCarSelectPlayer != NULL) 
                    {
                        PLR_KillPlayer(g_CarSelectInfo[i].pCarSelectPlayer);
                        g_CarSelectInfo[i].pCarSelectPlayer = NULL;
                    }
                    carPos.v[X] = box->Pos[i].v[X] + TO_LENGTH(20.0f);
                    carPos.v[Y] = box->Pos[i].v[Y] - TO_LENGTH(10.0f);
                    carPos.v[Z] = box->Pos[i].v[Z] + TO_LENGTH(0.0f);
                    g_CarSelectInfo[i].pCarSelectPlayer = PLR_CreatePlayer(PLAYER_FRONTEND, CTRL_TYPE_NONE, gTitleScreenVars.PlayerData[i].iCarNum, &carPos, &g_CarSelectInfo[i].matCarDropMatrix);
                    box->State[i] = CARBOX_OPEN;
                    box->Timer[i] = ZERO;

                }
                break;

            case CARBOX_OPEN:
                // Move box back to default position        
                if (box->Timer[i] > TO_TIME(1.3f)) 
                {
                    if (g_pMenuHeader->m_pMenu == &Menu_CarSelected) 
                    {
                        // After some time delay, return from the Select Car
                        // state engine.
                        g_SelectCarStateEngine.Return( STATEENGINE_COMPLETED );

                        // After we're through with it, the car box goes back to being stacked
                        box->State[i] = CARBOX_STACKED;
                    } 
                    else 
                    {
                        box->State[i] = CARBOX_SELECTED;
                    }   
                }
                if (box->Timer[i] > TO_TIME(0.3f)) 
                {
                    // Send back to stack
                    InterpCarBoxPos(&box->Pos[i], &box->HomePos[i], 0.1f, 0.2f, 0.1f);
                    InterpCarBoxQuat(&box->Quat[i], &box->HomeQuat[i], 0.9f);
                }
                break;
        }

        QuatToMat(&box->Quat[i], &box->WMatrix[i]);
    }
}

void InitCarBoxStuff()
{
    for( DWORD i=0; i<4; i++ )
    {
        PERPLAYERCARSELECTINFO* pCarSelectInfo = &g_CarSelectInfo[i];

        VEC a = {0,0,0};
        QUATERNION q = {0,0,0,1};
        pCarSelectInfo->pCarSelectPlayer = NULL;
        pCarSelectInfo->pCarBoxObject    = NULL;
        pCarSelectInfo->vCarBoxSelectPos = a;
        pCarSelectInfo->vCarBoxOpenPos   = a;
        pCarSelectInfo->qCarBoxOpenQuat  = q;
        pCarSelectInfo->bCarChosen       = FALSE;
        pCarSelectInfo->bCarTaken        = FALSE;

        // No car selected
        pCarSelectInfo->pCarSelectPlayer = NULL;

        // Set the Car Box destination

        MAT mat;

        // Car box close-to-camera (highlighted) position
        QuatToMat(&g_CameraPositions[TITLESCREEN_CAMPOS_CAR_SELECT].Quat, &mat);
        VecPlusScalarVec(&g_CameraPositions[TITLESCREEN_CAMPOS_CAR_SELECT].vEye, TO_LENGTH(600.0f), &mat.mv[L], &pCarSelectInfo->vCarBoxSelectPos);
        VecPlusEqScalarVec(&pCarSelectInfo->vCarBoxSelectPos, TO_LENGTH(150.0f), &mat.mv[R]);
        VecPlusEqScalarVec(&pCarSelectInfo->vCarBoxSelectPos, -TO_LENGTH(50.0f), &mat.mv[U]);

        // Carbox opening orientation
        //SetVec(&mat.mv[U], ZERO, ZERO, ONE);
        mat.m[UX] = -mat.m[LX];
        mat.m[UY] = ZERO;
        mat.m[UZ] = -mat.m[LZ];
        NormalizeVec(&mat.mv[U]);
        VecCrossVec(&mat.mv[R], &mat.mv[U], &mat.mv[L]);
        MatToQuat(&mat, &pCarSelectInfo->qCarBoxOpenQuat);

        // Car's drop matrix
        SwapVecs(&mat.mv[U], &mat.mv[L]);
        NegateVec(&mat.mv[L]);
        SwapVecs(&mat.mv[R], &mat.mv[L]);
        NegateVec(&mat.mv[L]);
        CopyMat(&mat, &pCarSelectInfo->matCarDropMatrix);

        // Carbox opening position
        //SetVec(&gCarBoxOpenPos, 
        //  pCarSelectInfo->vCarBoxSelectPos.v[X] + MulScalar(TO_LENGTH(230.0f), mat.m[LX]),
        //  0.0f,//pCarSelectInfo->vCarBoxSelectPos.v[Y],
        //  pCarSelectInfo->vCarBoxSelectPos.v[Z] + MulScalar(TO_LENGTH(230.0f), mat.m[LZ]));
        QuatToMat(&g_CameraPositions[TITLESCREEN_CAMPOS_CAR_SELECTED].Quat, &mat);
        SetVec( &pCarSelectInfo->vCarBoxOpenPos, 
                g_CameraPositions[TITLESCREEN_CAMPOS_CAR_SELECTED].vEye.v[X] + MulScalar(TO_LENGTH(230.0f), mat.m[LX]),
                0.0f,//pCarSelectInfo->vCarBoxSelectPos.v[Y],
                g_CameraPositions[TITLESCREEN_CAMPOS_CAR_SELECTED].vEye.v[Z] + MulScalar(TO_LENGTH(230.0f), mat.m[LZ]));
    }

}

void InterpCarBoxPos(VEC *pos, VEC *dest, FLOAT xScale, FLOAT yScale, FLOAT zScale)
{
    FLOAT timeScale = TimeStep * 50;

    pos->v[X] += MulScalar3(xScale, timeScale, (dest->v[X] - pos->v[X]));
    pos->v[Y] += MulScalar3(yScale, timeScale, (dest->v[Y] - pos->v[Y]));
    pos->v[Z] += MulScalar3(zScale, timeScale, (dest->v[Z] - pos->v[Z]));
}

void InterpCarBoxQuat(QUATERNION *quat, QUATERNION *destQuat, FLOAT scale)
{
    scale = ONE - scale;
    scale = MulScalar3(scale, TimeStep, 30);
    if (scale > ONE) scale = ONE;
    ScalarQuatPlusScalarQuat(ONE - scale, quat, scale, destQuat, quat);
}




////////////////////////////////////////////////////////////////
//
// Draw Car Box
//
////////////////////////////////////////////////////////////////
VEC CarBoxFaceCorner[4] = 
{
    {-32.7f, -48.9f, -105.5f },
    { 77.3f, -48.9f, -105.5f },
    { 77.3f,  61.1f, -105.5f },
    {-32.7f,  61.1f, -105.5f },
};

// $MD added so that new cars can show their carboxes
void LoadExtraCarBoxTextures()
{
    for(int i = 0; i < NCarTypes; i++)
    {
        XBResource* pRes = NULL;
            
            // load
        if(CarInfo[i].TCarBoxFile[0] != NULL)
        {
            pRes = new XBResource;
            pRes->Create(CarInfo[i].TCarBoxFile, 1, NULL);
            if(!pRes)
            {
                DumpMessage("Warning", "could not find carbox texture");
                delete pRes;
            }
            else
            {
                CarInfo[i].m_pCarBoxXBR = pRes;
            }
        }
    }
}

// $MD added so that new cars can show their carboxes
void FreeExtraCarBoxTextures()
{
    for(int i = 0; i < NCarTypes; i++)
    {
        delete CarInfo[i].m_pCarBoxXBR;
        CarInfo[i].m_pCarBoxXBR = NULL;
    }
}

            
void DrawCarBoxFace(OBJECT *obj)
{
    FLOAT xOff, yOff;
    OBJECT_CARBOX_OBJ *box = (OBJECT_CARBOX_OBJ*)obj->Data;
    
    if (box->CarType == -1)
    {
        // set new tpage here
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

    if( box->CarType == 16 )
        obj->body.Centre.Pos.v[0] = 0;

    // Update the car box (which is unique) with the state for the current player
    obj->body.Centre.WMatrix = box->WMatrix[gTitleScreenVars.iCurrentPlayer];
    obj->body.Centre.Pos     = box->Pos[gTitleScreenVars.iCurrentPlayer];

    
    
    // Render the car box
    if ((box->CarType == -1) || CarInfo[box->CarType].Selectable) 
    {   
        if (box->CarType == -1 && CarInfo[GameSettings.CarType].m_pCarBoxXBR != NULL)
        {
            WORD tpage = TPAGE_WORLD_START + 6;
            TEXINFO Temp = TexInfo[tpage];

            D3DSURFACE_DESC desc;
            TexInfo[tpage].Texture = CarInfo[GameSettings.CarType].m_pCarBoxXBR->GetTexture(DWORD(0));
            TexInfo[tpage].Texture->GetLevelDesc(0, &desc);
            TexInfo[tpage].Active = TRUE;
            TexInfo[tpage].Width = desc.Width;
            TexInfo[tpage].Height = desc.Height;
            TexInfo[tpage].File[0] = NULL;

            /*
            static LPDIRECT3DTEXTURE8 pRes = NULL;
            if(!pRes)
            {
                D3DXCreateTextureFromFileA(D3Ddevice,"D:\\cars\\ufo\\carbox.bmp", &pRes);
            }
            TexInfo[tpage].Texture = pRes;
            */
            
            // setup face poly
            box->FacePoly->Tpage = tpage;

            
            box->FacePoly->tu0 = 1.0f;       box->FacePoly->tv0 = 0.0f;
            box->FacePoly->tu1 = 1.0f;       box->FacePoly->tv1 = 1.0f;
            box->FacePoly->tu2 = 0.0f;       box->FacePoly->tv2 = 1.0f;
            box->FacePoly->tu3 = 0.0f;       box->FacePoly->tv3 = 0.0f;

            FlushPolyBuckets(); // flush buckets with old tpage
            RenderObjectModel(&obj->body.Centre.WMatrix, &obj->body.Centre.Pos, &LevelModel[box->BoxModel].Model, obj->EnvRGB, obj->EnvOffsetX, obj->EnvOffsetY, obj->EnvScale, obj->renderflag, FALSE);
            FlushPolyBuckets(); // flush bucketes that we just rendered

            TexInfo[tpage] = Temp;
        }
        else
        {
            DrawCarBoxFace(obj);
            RenderObjectModel(&obj->body.Centre.WMatrix, &obj->body.Centre.Pos, &LevelModel[box->BoxModel].Model, obj->EnvRGB, obj->EnvOffsetX, obj->EnvOffsetY, obj->EnvScale, obj->renderflag, FALSE);
        }
    } 
    else 
    {
        RenderObjectModel(&obj->body.Centre.WMatrix, &obj->body.Centre.Pos, &LevelModel[box->PlainBoxModel].Model, obj->EnvRGB, obj->EnvOffsetX, obj->EnvOffsetY, obj->EnvScale, obj->renderflag, FALSE);
    }
}





////////////////////////////////////////////////////////////////
//
// Select Car Menu
//
////////////////////////////////////////////////////////////////
extern MENU Menu_CarSelect = 
{
    TEXT_SELECTCAR,
    MENU_DEFAULT,                           // Menu type
    NULL,                                   // Create menu function
    HandleCarSelectMenu,                    // Input handler function
    DrawCarSelectMenu,                      // Menu draw function
    MENU_CARSELECT_XPOS,                    // X coord
    MENU_CARSELECT_YPOS,                    // Y Coord
};

extern MENU Menu_CarSelected = 
{
    TEXT_SELECTCAR,
    MENU_DEFAULT,                           // Menu type
    NULL,                                   // Create menu function
    HandleCarSelectedMenu,                  // Input handler function
    NULL,                                   // Menu draw function
};


BOOL HandleCarSelectMenu( MENU_HEADER* pMenuHeader, DWORD dwInput )
{
    switch( dwInput )
    {
        case MENU_INPUT_LEFT:
            if( g_CarSelectInfo[gTitleScreenVars.iCurrentPlayer].bCarChosen )
                break;
            // Set the car number for this player
            GameSettings.CarType = gTitleScreenVars.pCurrentPlayer->iCarNum = PrevValidCarType(gTitleScreenVars.pCurrentPlayer->iCarNum);
            return TRUE;

        case MENU_INPUT_RIGHT:
            if( g_CarSelectInfo[gTitleScreenVars.iCurrentPlayer].bCarChosen )
                break;
            // Set the car number for this player
            GameSettings.CarType = gTitleScreenVars.pCurrentPlayer->iCarNum = NextValidCarType(gTitleScreenVars.pCurrentPlayer->iCarNum);
            return TRUE;

        case MENU_INPUT_BACK:
            FreeExtraCarBoxTextures();
            g_SelectCarStateEngine.Return( STATEENGINE_TERMINATED );
            return TRUE;

        case MENU_INPUT_SELECT:
            g_CarSelectInfo[gTitleScreenVars.iCurrentPlayer].bCarChosen = TRUE;

            // Advance to the Car Selected menu
            pMenuHeader->SetNextMenu( &Menu_CarSelected );
            g_pTitleScreenCamera->SetNewPos( TITLESCREEN_CAMPOS_CAR_SELECTED );
            return TRUE;
    }

    return FALSE;
}

BOOL HandleCarSelectedMenu( MENU_HEADER* pMenuHeader, DWORD dwInput )
{
    switch( dwInput )
    {
//$REMOVED - Don't allow this
//      case MENU_INPUT_BACK:
//          // Go back to the Car Select menu
//          pMenuHeader->SetNextMenu( &Menu_CarSelect );
//          g_pTitleScreenCamera->SetNewPos( TITLESCREEN_CAMPOS_CAR_SELECT );
//          return TRUE;
//$END_REMOVAL

        case MENU_INPUT_SELECT:
            // Move the car box back to the stack
            for( DWORD i=0; i<4; i++ )
            {
                if( g_CarSelectInfo[i].pCarBoxObject )
                    g_CarSelectInfo[i].pCarBoxObject->State[i] = CARBOX_STACKED;
            }

            // Return from the Car Select state engine
            FreeExtraCarBoxTextures();
            g_SelectCarStateEngine.Return( STATEENGINE_COMPLETED );

            return TRUE;
    }

    return FALSE;
}




/////////////////////////////////////////////////////////////////////
//
// Draw Car Name
//
/////////////////////////////////////////////////////////////////////
#define MIN_CAR_TOPEND      MPH2OGU_SPEED * 26.0f
#define MAX_CAR_TOPEND      MPH2OGU_SPEED * 42.0f
#define MIN_CAR_WEIGHT      0.6f
#define MAX_CAR_WEIGHT      3.0f
#define MIN_CAR_ACC         TO_TIME(4.0f)
#define MAX_CAR_ACC         TO_TIME(12.0f)
#define MIN_CAR_HANDLING    0.0f
#define MAX_CAR_HANDLING    100.0f

void DrawCarSelectMenu( MENU_HEADER *menuHeader, MENU *menu )
{
    FLOAT xPos1, xPos2, yPos;
    FLOAT fTextWidth;

    long carID = gTitleScreenVars.pCurrentPlayer->iCarNum;
    static FLOAT speed[4]    = { MIN_CAR_TOPEND, MIN_CAR_TOPEND, MIN_CAR_TOPEND, MIN_CAR_TOPEND };
    static FLOAT acc[4]      = { MAX_CAR_ACC, MAX_CAR_ACC, MAX_CAR_ACC, MAX_CAR_ACC };
    static FLOAT weight[4]   = { MIN_CAR_WEIGHT, MIN_CAR_WEIGHT, MIN_CAR_WEIGHT, MIN_CAR_WEIGHT };
    static FLOAT handling[4] = { ZERO, ZERO, ZERO, ZERO };

    FLOAT fMaxItemWidth = CalcMaxStringWidth( 6, &gTitleScreen_Text[TEXT_CAR_CLASS] );

    FLOAT fMaxDataWidth  = 100.0f;
    FLOAT fMaxDataWidth1 = CalcMaxStringWidth( 3, &gTitleScreen_Text[TEXT_CAR_CLASS_ELECTRIC] );
    FLOAT fMaxDataWidth2 = CalcMaxStringWidth( 5, &gTitleScreen_Text[TEXT_CAR_RATING_ROOKIE] );
    FLOAT fMaxDataWidth3 = CalcMaxStringWidth( 3, &gTitleScreen_Text[TEXT_CAR_TRANS_4WD] );
    if( fMaxDataWidth1 > fMaxDataWidth ) fMaxDataWidth = fMaxDataWidth1;
    if( fMaxDataWidth2 > fMaxDataWidth ) fMaxDataWidth = fMaxDataWidth2;
    if( fMaxDataWidth3 > fMaxDataWidth ) fMaxDataWidth = fMaxDataWidth3;

	FLOAT fMaxWidth = fMaxItemWidth + 20 + fMaxDataWidth;
	for( int i=0; i<NCarTypes; i++ )
	{
		swprintf( MenuBuffer, L"%S %s", CarInfo[i].Name, TEXT_TABLE(TEXT_CHEAT_SUFFIX) );
		fMaxWidth = max( fMaxWidth, g_pFont->GetTextWidth( MenuBuffer ) );
	}

    xPos1 = menuHeader->m_XPos;
    yPos  = menuHeader->m_YPos;

    DrawNewSpruBox( gMenuWidthScale  * (xPos1 - MENU_LEFT_PAD ),
                    gMenuHeightScale * (yPos - MENU_TOP_PAD ),
                    gMenuWidthScale  * (fMaxWidth + MENU_LEFT_PAD + MENU_RIGHT_PAD ),
                    gMenuHeightScale * (MENU_TEXT_HEIGHT*8 + MENU_TEXT_VSKIP*7 + MENU_TOP_PAD + MENU_BOTTOM_PAD ) );

    xPos2 = xPos1 + 20 + fMaxItemWidth;

    BeginTextState();

    // Car Name
    swprintf( MenuBuffer, L"%S", CarInfo[carID].Name, 200);
    DrawMenuText( xPos1, yPos, MENU_TEXT_RGB_CHOICE, MenuBuffer );

    if (CarInfo[carID].Modified || CarInfo[carID].Moved || ModifiedCarInfo) 
    {
        fTextWidth = g_pFont->GetTextWidth( MenuBuffer );
        DrawMenuText( xPos1 + 20 + fTextWidth, yPos, MENU_TEXT_RGB_LOLITE, TEXT_TABLE(TEXT_CHEAT_SUFFIX), 200 );
    }
    yPos += MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP;

    // Car Class
    yPos += MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP;
    DrawMenuText( xPos1, yPos, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_CAR_CLASS));
    DrawMenuText( xPos2, yPos, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_CAR_CLASS_ELECTRIC + CarInfo[carID].Class));

    // Transmission
    yPos += MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP;
    DrawMenuText( xPos1, yPos, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_CAR_TRANS));
    DrawMenuText( xPos2, yPos, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_CAR_TRANS_4WD + CarInfo[carID].Trans));

    // Car Rating
    yPos += MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP;
    DrawMenuText( xPos1, yPos, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_CAR_RATING));
    DrawMenuText( xPos2, yPos, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_CAR_RATING_ROOKIE + CarInfo[carID].Rating));

    // TopSpeed
    yPos += MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP;
    DrawMenuText( xPos1, yPos, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_CAR_SPEED), 200 );

    // $MD: these checks prevent new cars from showing acc, weight, and top speed bars
    // $MD: removed the carID checks and fixed some paramters.txt files so that
    //      all cars can show car paramters
    if( carID <= CARID_AMW || TRUE)
    {
        speed[gTitleScreenVars.iCurrentPlayer] += (CarInfo[carID].TopEnd - speed[gTitleScreenVars.iCurrentPlayer])/10;
        DrawScale(100 * (speed[gTitleScreenVars.iCurrentPlayer] - MIN_CAR_TOPEND) / (MAX_CAR_TOPEND - MIN_CAR_TOPEND), xPos2, yPos+3, 100, MENU_TEXT_HEIGHT);
        BeginTextState();
    } 
    else 
    {
        DrawMenuText( xPos2, yPos, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_CAR_QUESTIONMARKS), 200 );
    }

    // Acc
    yPos += MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP;
    DrawMenuText( xPos1, yPos, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_CAR_ACCEL), 200 );
    if( carID <= CARID_AMW || TRUE)
    {
        acc[gTitleScreenVars.iCurrentPlayer] += (CarInfo[carID].Acc - acc[gTitleScreenVars.iCurrentPlayer])/10;
        DrawScale(100 * (MAX_CAR_ACC - acc[gTitleScreenVars.iCurrentPlayer]) / (MAX_CAR_ACC - MIN_CAR_ACC), xPos2, yPos+3, 100, MENU_TEXT_HEIGHT);
        BeginTextState();
    } 
    else 
    {
        DrawMenuText( xPos2, yPos, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_CAR_QUESTIONMARKS), 200 );
    }

    // Weight
    yPos += MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP;
    DrawMenuText( xPos1, yPos, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_CAR_WEIGHT), 200 );
    if( carID <= CARID_AMW || TRUE)
    {
        weight[gTitleScreenVars.iCurrentPlayer] += (CarInfo[carID].Weight - weight[gTitleScreenVars.iCurrentPlayer])/10;
        DrawScale(100 * (weight[gTitleScreenVars.iCurrentPlayer] - MIN_CAR_WEIGHT) / (MAX_CAR_WEIGHT - MIN_CAR_WEIGHT), xPos2, yPos+3, 100, MENU_TEXT_HEIGHT);
        BeginTextState();
    } 
    else 
    {
        DrawMenuText( xPos2, yPos, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_CAR_QUESTIONMARKS), 200 );
    }

    // Handling
//  yPos += MENU_TEXT_HEIGHT + MENU_TEXT_VSKIP;
//  DrawMenuText( xPos1, yPos, MENU_TEXT_RGB_NORMAL, L"Handling ", 200 );
    /*
    if( carID <= CARID_AMW ) 
    {
        handling += (CarInfo[carID].Handling - handling)/10;
        DrawScale(handling, xPos + 8 * MENU_TEXT_WIDTH, yPos, 100, MENU_TEXT_HEIGHT);
        BeginTextState();
    } 
    else 
    {
        DrawMenuText( xPos2, yPos, MENU_TEXT_RGB_NORMAL, TEXT_TABLE(TEXT_CAR_QUESTIONMARKS), 200 );
    }
    */
//  DrawMenuText( xPos2, yPos, MENU_TEXT_RGB_NORMAL, L"Guess!!", 200 );

    // host started?
    //$WARNING: this code assumes 'bGameStarted' has already been cleared (b/c
    /// assumes this UI screen will only be called *after* client has joined
    /// session, and 'bGameStarted' gets cleared by the UI code when you start
    /// to join a session.  Ugly, ugly...)
    if( IsMultiPlayer() )
    {
        GetRemoteMessages();
        if( bGameStarted )
        {
            //SetRaceData(menuHeader, menu, menuItem);
            g_bTitleScreenRunGame = TRUE;
        }
    }
}





CSelectCarStateEngine g_SelectCarStateEngine;

//-----------------------------------------------------------------------------
// Name: Process()
// Desc: Main control function for the state engine
//-----------------------------------------------------------------------------
HRESULT CSelectCarStateEngine::Process()
{
    enum
    {
        SELECTCAR_STATE_BEGIN = STATEENGINE_STATE_BEGIN,
        SELECTCAR_STATE_MAINLOOP,
    };

    switch( m_State )
    {
        case SELECTCAR_STATE_BEGIN:

            // $MD: added
            LoadExtraCarBoxTextures();

            for( DWORD i=0; i<MAX_LOCAL_PLAYERS; i++ )
            {
                if( g_CarSelectInfo[i].pCarSelectPlayer ) 
                {
                    PLR_KillPlayer( g_CarSelectInfo[i].pCarSelectPlayer );
                    g_CarSelectInfo[i].pCarSelectPlayer = NULL;
                }
            }

            if( GameSettings.RandomCars )
            {
                // User can't pick cars. Skip ahead.
                Return( STATEENGINE_COMPLETED );
                break;
            }

            g_CarSelectInfo[gTitleScreenVars.iCurrentPlayer].bCarChosen = FALSE;

            // re-call car select func if 'AllCars' true
            //if( AllCars || AllowUFO )
            //    SetAllCarSelect();

            GameSettings.CarType = gTitleScreenVars.pCurrentPlayer->iCarNum;
            if( !CarInfo[GameSettings.CarType].Selectable ) 
                GameSettings.CarType = gTitleScreenVars.pCurrentPlayer->iCarNum = 0;

            // Set the car select menu and camera position
            g_pMenuHeader->SetNextMenu( &Menu_CarSelect );
            g_pTitleScreenCamera->SetNewPos( TITLESCREEN_CAMPOS_CAR_SELECT );

            m_State = SELECTCAR_STATE_MAINLOOP;
            break;

        case SELECTCAR_STATE_MAINLOOP:
            // Nothing to do, as control is in the menus
            break;
    }

    // Handle the menus
    g_pMenuHeader->HandleMenus();

    return S_FALSE;
}



