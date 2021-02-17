//-----------------------------------------------------------------------------
// File: Cheats.cpp
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "Revolt.h"
#include "SFX.h"
#include "Cheats.h"




// Cheat flags
BOOL AllCars         = FALSE;
BOOL AllTracks       = FALSE;
BOOL AllWeapons      = FALSE;
BOOL AllowEditors    = FALSE;
BOOL AllowAllCameras = FALSE;
BOOL SnailMode       = FALSE;
BOOL ChangeCars      = FALSE;
BOOL AllowUFO        = FALSE;




//-----------------------------------------------------------------------------
// Name cheats
//-----------------------------------------------------------------------------
#define NAME_CHEAT_LEN 12
#define NAME_CHEAT_XOR (char)0xff
#define NC(c) (c ^ NAME_CHEAT_XOR)

struct NAME_CHEAT_DATA
{
    BOOL* pbFlag;
    CHAR  strCheat[NAME_CHEAT_LEN];
};

static NAME_CHEAT_DATA g_NameCheats[] = 
{
    {
        &AllCars,
        {NC('C'), NC('A'), NC('R'), NC('N'), NC('I'), NC('V'), NC('A'), NC('L'), 0},
    },
    {
        &AllTracks,
        {NC('T'), NC('R'), NC('A'), NC('C'), NC('K'), NC('E'), NC('R'), 0},
    },
    {
        &AllWeapons,
        {NC('S'), NC('A'), NC('D'), NC('I'), NC('S'), NC('T'), 0},
    },
    {
        &AllowEditors,
        {NC('M'), NC('A'), NC('K'), NC('E'), NC('I'), NC('T'), NC('G'), NC('O'), NC('O'), NC('D'), 0},
    },
    {
        &AllowAllCameras,
        {NC('T'), NC('V'), NC('T'), NC('I'), NC('M'), NC('E'), 0},
    },
    {
        &SnailMode,
        {NC('D'), NC('R'), NC('I'), NC('N'), NC('K'), NC('M'), NC('E'), 0},
    },
    {
        &ChangeCars,
        {NC('C'), NC('H'), NC('A'), NC('N'), NC('G'), NC('E'), NC('L'), NC('I'), NC('N'), NC('G'), 0},
    },
    {
        &AllowUFO,
        {NC('U'), NC('R'), NC('C'), NC('O'), 0},
    },
    {
        NULL
    }
};




//-----------------------------------------------------------------------------
// Name: CheckNameCheats()
// Desc: 
//-----------------------------------------------------------------------------
BOOL CheckNameCheats( CHAR* strName )
{
	// loop thru all cheats
    for( int i = 0; g_NameCheats[i].pbFlag; i++ )
    {
        for( int j = 0 ; j < NAME_CHEAT_LEN ; j++ )
        {
			// Check for end of cheat string
            if( !g_NameCheats[i].strCheat[j] )
            {
                (*g_NameCheats[i].pbFlag) = TRUE;
                PlaySfx( SFX_HONK, SFX_MAX_VOL, SFX_CENTRE_PAN, SFX_SAMPLE_RATE, 0x7fffffff) ;
#pragma message( "jharding needs to look into porting this." )
                return TRUE;
            }

			// Check this char
            if( (strName[j] ^ NAME_CHEAT_XOR) != g_NameCheats[i].strCheat[j] )
                break;
        }
    }
        
	// No cheats found
    return FALSE;
}




#include "main.h"
#include "ctrlread.h"

//-----------------------------------------------------------------------------
// Check UDLR for cheats
//-----------------------------------------------------------------------------
#define MAX_CHEAT_KEYS 16

#define CHEAT_KEY_LEFT  1
#define CHEAT_KEY_RIGHT 2
#define CHEAT_KEY_FWD   4
#define CHEAT_KEY_BACK  8

static char g_CheatKeys[MAX_CHEAT_KEYS];
static char g_CheatKeyTable[][MAX_CHEAT_KEYS] = 
{
    {
        0,
        CHEAT_KEY_FWD,
        0,
        CHEAT_KEY_RIGHT,
        0,
        CHEAT_KEY_BACK,
        0,
        CHEAT_KEY_LEFT,
        0,
        CHEAT_KEY_FWD,
        0,
        CHEAT_KEY_LEFT,
        0,
        CHEAT_KEY_BACK,
        0,
        CHEAT_KEY_RIGHT,
    },
    {
        -1,
    }
};




//-----------------------------------------------------------------------------
// Name: CheckCheatKeys()
// Desc: 
//-----------------------------------------------------------------------------
LONG CheckCheatKeys()
{
    char key = 0;

	// never!
    return 0;

	// get current state
    long dx = GetAnaloguePair(&KeyTable[KEY_LEFT], &KeyTable[KEY_RIGHT]);
    long dy = GetAnaloguePair(&KeyTable[KEY_FWD], &KeyTable[KEY_BACK]);

    if (dx < 0)   key |= CHEAT_KEY_LEFT;
    if (dx > 0)   key |= CHEAT_KEY_RIGHT;
    if (dy < 0)   key |= CHEAT_KEY_FWD;
    if (dy > 0)   key |= CHEAT_KEY_BACK;

	// new?
    if( key == g_CheatKeys[MAX_CHEAT_KEYS - 1] )
        return 0L;

	// yep, store
    for( long i = 0; i  < MAX_CHEAT_KEYS - 1; i++ )
    {
        g_CheatKeys[i] = g_CheatKeys[i + 1];
    }
    g_CheatKeys[MAX_CHEAT_KEYS - 1] = key;

	// check against table
    for( long i = 0; g_CheatKeyTable[i][0] != -1; i++ )
    {
        for( long j = 0; j < MAX_CHEAT_KEYS; j++ )
        {
            if( g_CheatKeyTable[i][j] != g_CheatKeys[j] )
                break;
        }

		// Found one, act
        if( j == MAX_CHEAT_KEYS )
        {
            PlaySfx( SFX_HONK, SFX_MAX_VOL, SFX_CENTRE_PAN, SFX_SAMPLE_RATE, 0x7fffffff) ;
#pragma message( "jharding needs to look into porting this." )

			// Return this cheat
			return i;
		}
	}
}
