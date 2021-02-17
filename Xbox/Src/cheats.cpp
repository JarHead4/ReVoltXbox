//-----------------------------------------------------------------------------
// File: cheats.cpp
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"
#include "input.h"
#include "player.h"
#include "text.h"
#include "SoundEffectEngine.h"
#include "texture.h"
#include "LevelLoad.h"
#include "cheats.h"

#include "ui_menu.h"
#include "ui_TitleScreen.h"
#include "ui_StateEngine.h"
#include "ui_SelectRaceMode.h"
#include "ui_TopLevelMenu.h"


// cheat flags
long AllCars         = FALSE;
long AllTracks       = FALSE;
long AllWeapons      = FALSE;
long AllowEditors    = FALSE;
long AllowAllCameras = FALSE;
long SnailMode       = FALSE;
long ChangeCars      = FALSE;
long AllowUFO        = FALSE;
long AllowCredits    = FALSE;




/////////////////////////
// check cheat strings //
/////////////////////////

#define MAX_CHEAT_STRING_BUFFER 16
#define CHEAT_XOR (char)-1
#define CHEAT(c) (c ^ CHEAT_XOR)

static char CheatStringBuffer[MAX_CHEAT_STRING_BUFFER + 1];

static char CheatStrings[][MAX_CHEAT_STRING_BUFFER + 1] = {
    {CHEAT('b'), CHEAT('a'), CHEAT('r'), CHEAT('g'), CHEAT('a'), CHEAT('r'), CHEAT('a'), CHEAT('m'), CHEAT('a'), 0},
    {CHEAT('j'), CHEAT('o'), CHEAT('k'), CHEAT('e'), CHEAT('r'), 0},
    {CHEAT('g'), CHEAT('i'), CHEAT('m'), CHEAT('m'), CHEAT('e'), CHEAT('c'), CHEAT('r'), CHEAT('e'), CHEAT('d'), CHEAT('i'), CHEAT('t'), CHEAT('s'), 0},

    {0}
};

void CheckCheatStrings()
{
    long flag, i, len;
    unsigned char ch;

// update buffer

    ch = GetKeyPress();
    if (!ch) return;

    memcpy(CheatStringBuffer, CheatStringBuffer + 1, MAX_CHEAT_STRING_BUFFER - 1);
    CheatStringBuffer[MAX_CHEAT_STRING_BUFFER - 1] = ch ^ CHEAT_XOR;
    CheatStringBuffer[MAX_CHEAT_STRING_BUFFER] = 0;

// look for cheat string

    flag = 0;

    while (TRUE)
    {
        len = 0;
        while (CheatStrings[flag][len]) len++;

        if (!len)
            return;

        for (i = 0 ; i < len ; i++)
        {
            if (CheatStrings[flag][i] != CheatStringBuffer[MAX_CHEAT_STRING_BUFFER - len + i])
            {
                break;
            }
        }

        if (i == len)
            break;

        flag++;
    }

// act

    ZeroMemory(CheatStringBuffer, MAX_CHEAT_STRING_BUFFER);
#ifdef OLD_AUDIO
    PlaySfx(SFX_HONK, SFX_MAX_VOL, SFX_CENTRE_PAN, SFX_SAMPLE_RATE, 0);
#else
    g_SoundEngine.Play2DSound( EFFECT_HonkGood, FALSE );
#endif

    switch (flag)
    {
        case 0:
            FreeOneTexture(TPAGE_ENVSTILL);
            LoadMipTexture("D:\\gfx\\font.bmp", TPAGE_ENVSTILL, 128, 128, 0, 1, TRUE);  //$MODIFIED: added "D:\\"
            WheelRenderType = !WheelRenderType;
            break;

        case 1:
            if (IsMultiPlayer())
            {
                SendCarNewCarAll(GameSettings.CarType);
            }
            break;

        case 2:
            AllowCredits = !AllowCredits;
            break;
    }
}




//-----------------------------------------------------------------------------
// Check UDLR for cheats
//-----------------------------------------------------------------------------
#define MAX_CHEAT_KEYS 16

#define CHEAT_KEY_LEFT 1
#define CHEAT_KEY_RIGHT 2
#define CHEAT_KEY_FWD 4
#define CHEAT_KEY_BACK 8

enum 
{
    CHEAT_TABLE_VERSION_DEV,
};

static char CheatKeys[MAX_CHEAT_KEYS];

static VERSION LastVersion = VERSION_DEV;

char CheatKeyTable[][MAX_CHEAT_KEYS] = 
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

void CheckCheatKeys()
{
    long i, j, dx, dy;
    char key = 0;

// never!
return;

    // only in 1st 2 menus
    if (g_pMenuHeader->m_pMenu != &Menu_TopLevel && g_pMenuHeader->m_pMenu != &Menu_StartRace)
        return;

    // get current state
    dx = GetAnaloguePair(&KeyTable[KEY_LEFT], &KeyTable[KEY_RIGHT]);
    dy = GetAnaloguePair(&KeyTable[KEY_FWD], &KeyTable[KEY_BACK]);

    if (dx < 0) key |= CHEAT_KEY_LEFT;
    else if (dx > 0) key |= CHEAT_KEY_RIGHT;
    if (dy < 0) key |= CHEAT_KEY_FWD;
    else if (dy > 0) key |= CHEAT_KEY_BACK;

    // new?
    if (key == CheatKeys[MAX_CHEAT_KEYS - 1])
        return;

    // yep, store
    for (i = 0 ; i  < MAX_CHEAT_KEYS - 1 ; i++)
    {
        CheatKeys[i] = CheatKeys[i + 1];
    }
    CheatKeys[MAX_CHEAT_KEYS - 1] = key;

    // check against table
    i = 0;
    while (CheatKeyTable[i][0] != -1)
    {
        for( j = 0 ; j < MAX_CHEAT_KEYS ; j++ )
        {
            if( CheatKeyTable[i][j] != CheatKeys[j] )
                break;
        }

        // found one, act
        if( j == MAX_CHEAT_KEYS )
        {
#ifdef OLD_AUDIO
            PlaySfx(SFX_HONK, SFX_MAX_VOL, SFX_CENTRE_PAN, SFX_SAMPLE_RATE, 0x7fffffff);
#else
            g_SoundEngine.Play2DSound( EFFECT_HonkGood, FALSE );
#endif // OLD_AUDIO

            switch( i )
            {
                // enable dev mode
                case CHEAT_TABLE_VERSION_DEV:

                    VERSION ver = LastVersion;
                    LastVersion = Version;
                    Version = ver;

                    //InitDefaultLevels();
                    //SetAllCarSelect();
                    
                    break;
            }
        }

        // next
        i++;
    }
}




//-----------------------------------------------------------------------------
// check name cheats
//-----------------------------------------------------------------------------
#define NAME_CHEAT_LEN 12
#define NAME_CHEAT_XOR (char)0xff
#define NC(c) (c ^ NAME_CHEAT_XOR)

struct NAME_CHEAT_DATA
{
    long* Flag;
    char  Cheat[NAME_CHEAT_LEN];
};

NAME_CHEAT_DATA NameCheats[] = 
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

long CheckNameCheats( char* name )
{
    // loop thru all cheats
    for( long i = 0; NameCheats[i].Flag; i++ )
    {
        for( long j = 0; j < NAME_CHEAT_LEN; j++ )
        {
            // check for end of cheat string
            if (!NameCheats[i].Cheat[j])
            {
                *NameCheats[i].Flag = TRUE;
#ifdef OLD_AUDIO
                PlaySfx(SFX_HONK, SFX_MAX_VOL, SFX_CENTRE_PAN, SFX_SAMPLE_RATE, 0x7fffffff);
#else
                g_SoundEngine.Play2DSound( EFFECT_HonkGood, FALSE );
#endif
                return TRUE;
            }

            // check this char
            if ((name[j] ^ NAME_CHEAT_XOR) != NameCheats[i].Cheat[j])
                break;
        }
    }
        
    // none found
    return FALSE;
}
