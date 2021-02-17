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
#include <d3d8.h>
#include "dinput.h"  //$NOTE(cprince): temporary compatibility layer
#include <dsound.h>
//$REMOVED#include <dplobby.h>
#include "windows.h"  //$NOTE(cprince): temporary compatibility layer

#include "Units.h"
#include "Typedefs.h"
#include "Util.h"
//$REMOVED#include "Resource.h"
#include "Debug.h"
#include "load.h"

// macros

#define MAX_NUM_PLAYERS 30
#define MAX_RECORD_TIMES 10
#define MAX_SPLIT_TIMES 10

#define MAX_PLAYER_NAME 16
#define MAX_LEVEL_DIR_NAME 16


#define RELEASE(x) \
{ \
    if (x) \
    { \
        x->Release(); \
        x = NULL; \
    } \
}

#endif // REVOLT_H

