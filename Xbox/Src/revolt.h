//-----------------------------------------------------------------------------
// File: revolt.h
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef REVOLT_H
#define REVOLT_H

#ifndef WIN32_LEAN_AND_MEAN //$ADDITION
#define WIN32_LEAN_AND_MEAN
#endif //$ADDITION
#define _PC

// includes

//$MODIFIED:
#include <xtl.h>
#include <memory.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>
#include <assert.h>  //$ADDITION(cprince)
#include <d3d8.h>
#include "dinput.h"  //$NOTE(cprince): temporary compatibility layer
#include <dsound.h>
//$REVISIT: do we want STL stuff??
#include <vector>
#include <map>
#include <algorithm>
#include <list>
//$REMOVED#include <dplobby.h>
#include "windows.h"  //$NOTE(cprince): temporary compatibility layer

#include "units.h"
#include "typedefs.h"
#include "util.h"
//$REMOVED#include "Resource.h"
#include "debug.h"
#include "load.h"

// macros

  //$REVISIT:  This #ifdef is a temp hack for May02_TechBeta (and possibly future beta releases).
  /// Eventually, all builds should have same value for MAX_NUM_PLAYERS, which will make this #ifdef unnecessary.
  #ifdef SHIPPING
#define MAX_NUM_PLAYERS 6
  #else
#define MAX_NUM_PLAYERS 24
  #endif
#define MAX_RECORD_TIMES 10
#define MAX_SPLIT_TIMES 10

#define MAX_PLAYER_NAME 16  //$REVISIT: probably should make sure this is >= XONLINE_GAMERTAG_SIZE
#define MAX_LEVEL_INF_NAME 16


#define RELEASE(x) \
{ \
    if (x) \
    { \
        x->Release(); \
        x = NULL; \
    } \
}

// workarounds for typecast warnings
#define fgetc(x)    ((char)fgetc(x))

// prevent debug spew in certain builds (by redefining function as 0 cast to its return type)
#ifdef SHIPPING
#define OutputDebugStringA(s)   ((VOID)0)
#define OutputDebugStringW(s)   ((VOID)0)
#endif //SHIPPING


#endif // REVOLT_H

