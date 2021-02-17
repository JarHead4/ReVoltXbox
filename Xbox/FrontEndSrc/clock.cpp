//-----------------------------------------------------------------------------
// File: Clock.cpp
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "Revolt.h"
#include "obj_init.h"
#include "menu2.h"
#include "menudat2.h"
#include "menudraw.h"
#include "titlescreen.h"
#include "player.h"
#include "timing.h"
#include "drawObj.h"




static long InitClock(OBJECT *obj, long *flags);




// Register the CarBox object init data
REGISTER_OBJECT( OBJECT_TYPE_CLOCK, InitClock, sizeof(OBJECT_CLOCK_OBJ) );


static VEC ClockHandOffset = {-1.7f, -500, 37};
static VEC ClockDiscOffset = {-1.7f, -500, 30};




//-----------------------------------------------------------------------------
// Name: ClockHandler()
// Desc: Best Times Clock
//-----------------------------------------------------------------------------
void ClockHandler(OBJECT *obj)
{
    OBJECT_CLOCK_OBJ *clock = (OBJECT_CLOCK_OBJ*)obj->Data;

    clock->LargeHandAngle += TimeStep;
    clock->SmallHandAngle += TimeStep / 60;
    clock->DiscAngle -= TimeStep / 6;

}




//-----------------------------------------------------------------------------
// Name: RenderClock()
// Desc: 
//-----------------------------------------------------------------------------
void RenderClock(OBJECT *obj)
{
    VEC handPos;
    MAT handMat, rotMat;
    OBJECT_CLOCK_OBJ *clock = (OBJECT_CLOCK_OBJ*)obj->Data;

    // Draw the clock body
    RenderObjectModel(&obj->body.Centre.WMatrix, &obj->body.Centre.Pos, &LevelModel[clock->BodyModel].Model, obj->EnvRGB, obj->EnvOffsetX, obj->EnvOffsetY, obj->EnvScale, obj->renderflag, FALSE);

    // Hand Position
    VecMulMat(&ClockHandOffset, &obj->body.Centre.WMatrix, &handPos);
    VecPlusEqVec(&handPos, &obj->body.Centre.Pos);

    // Small Hand
    RotMatrixZ(&rotMat, clock->SmallHandAngle);
    MatMulMat(&rotMat, &obj->body.Centre.WMatrix, &handMat);
    RenderObjectModel(&handMat, &handPos, &LevelModel[clock->SmallHandModel].Model, obj->EnvRGB, obj->EnvOffsetX, obj->EnvOffsetY, obj->EnvScale, obj->renderflag, FALSE);

    // Big Hand
    RotMatrixZ(&rotMat, clock->LargeHandAngle);
    MatMulMat(&rotMat, &obj->body.Centre.WMatrix, &handMat);
    RenderObjectModel(&handMat, &handPos, &LevelModel[clock->LargeHandModel].Model, obj->EnvRGB, obj->EnvOffsetX, obj->EnvOffsetY, obj->EnvScale, obj->renderflag, FALSE);

    // Disc
    VecMulMat(&ClockDiscOffset, &obj->body.Centre.WMatrix, &handPos);
    VecPlusEqVec(&handPos, &obj->body.Centre.Pos);
    RotMatrixZ(&rotMat, clock->DiscAngle);
    MatMulMat(&rotMat, &obj->body.Centre.WMatrix, &handMat);
    RenderObjectModel(&handMat, &handPos, &LevelModel[clock->DiscModel].Model, obj->EnvRGB, obj->EnvOffsetX, obj->EnvOffsetY, obj->EnvScale, obj->renderflag, FALSE);

}




//-----------------------------------------------------------------------------
// Name: InitClock()
// Desc: 
//-----------------------------------------------------------------------------
long InitClock(OBJECT *obj, long *flags)
{
    OBJECT_CLOCK_OBJ *clock = (OBJECT_CLOCK_OBJ*)obj->Data;

    // set default model
    clock->BodyModel = LoadOneLevelModel(LEVEL_MODEL_CLOCKBODY, TRUE, obj->renderflag, 0);
    clock->SmallHandModel = LoadOneLevelModel(LEVEL_MODEL_CLOCKHANDSMALL, TRUE, obj->renderflag, 0);
    clock->LargeHandModel = LoadOneLevelModel(LEVEL_MODEL_CLOCKHANDLARGE, TRUE, obj->renderflag, 0);
    clock->DiscModel = LoadOneLevelModel(LEVEL_MODEL_CLOCKDISC, TRUE, obj->renderflag, 0);
    obj->renderflag.envmap = FALSE;
    obj->renderflag.envgood = TRUE;
    obj->EnvRGB = 0xA0A0A0;

    // render handler
    obj->renderhandler = (RENDER_HANDLER)RenderClock;

    // ai handler
    obj->aihandler = (AI_HANDLER)ClockHandler;

    // Init data
    clock->LargeHandAngle = ZERO;
    clock->SmallHandAngle = ZERO;
    clock->DiscAngle = ZERO;

    // No collision
    obj->CollType = COLL_TYPE_NONE;
    obj->body.CollSkin.AllowObjColls = FALSE;
    obj->body.CollSkin.AllowWorldColls = FALSE;
    
    return TRUE;
}

