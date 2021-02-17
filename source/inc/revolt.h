
#define WIN32_LEAN_AND_MEAN
#define _PC

// includes

#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <memory.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>
#include <d3d.h>
#include <ddraw.h>
#include <dinput.h>
#include <dsound.h>
#include <dplobby.h>

#include "Units.h"
#include "Typedefs.h"
#include "Util.h"
//#include "Resource.h"
#include "Debug.h"

// macros

#define SCREEN_DEBUG 1
#define SCREEN_TIMES 1

#define MAX_NUM_PLAYERS 12
#define MAX_RECORD_TIMES 10
#define MAX_SPLIT_TIMES 10

#define RELEASE(x) \
{ \
	if (x) \
	{ \
		x->Release(); \
		x = NULL; \
	} \
}
