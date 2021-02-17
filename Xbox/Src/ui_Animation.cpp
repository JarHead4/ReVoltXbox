//-----------------------------------------------------------------------------
// File: ui_Animation.cpp
//
// Desc: 
//
// Re-Volt (Generic) Copyright (c) Probe Entertainment 1998
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"
#include "text.h"
#include "ui_MenuText.h"
#include "ui_MenuDraw.h"
#include "ui_TitleScreen.h"  // for TimeStep
#include "ui_StateEngine.h"
#include "ui_Animation.h"


CUIAnimationQueue g_AnimationQueue;

const float CUIRCSlidingAnimation::c_fXScreenWidth = 640.0;
const float CUIRCSlidingAnimation::c_fYScreenHeight = 480.0;

