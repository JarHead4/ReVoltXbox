//-----------------------------------------------------------------------------
// File: Cheats.h
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef CHEATS_H
#define CHEATS_H
#include "Cheats.h"




//-----------------------------------------------------------------------------
// Cheat flags
//-----------------------------------------------------------------------------
extern BOOL AllCars;
extern BOOL AllTracks;
extern BOOL AllWeapons;
extern BOOL AllowEditors;
extern BOOL AllowAllCameras;
extern BOOL SnailMode;
extern BOOL ChangeCars;
extern BOOL AllowUFO;




//-----------------------------------------------------------------------------
// Cheats
//-----------------------------------------------------------------------------
enum 
{
    CHEAT_TABLE_VERSION_DEV = 1,
};




//-----------------------------------------------------------------------------
// Function prototypes
//-----------------------------------------------------------------------------

// Check a name for cheat codes
BOOL CheckNameCheats( CHAR* strName );
LONG CheckCheatKeys();




#endif // CHEATS_H
