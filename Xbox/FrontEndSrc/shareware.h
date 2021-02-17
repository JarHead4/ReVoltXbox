//$CMP_NOTE: not yet clear whether we need some of this in Xbox version...

//-----------------------------------------------------------------------------
// File: shareware.h
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef SHAREWARE_H
#define SHAREWARE_H


#define SHAREWARE_MAX_PAGES     10

#define SLIDE_WAIT 0x1
#define SLIDE_SKIP 0x2

typedef struct {
    char *BitmapFilename;
    REAL DisplayTime;
    long Type;
} SLIDE;



extern void ProcessSharewareIntro();
extern void SetupSharewareIntro();


#endif // SHAREWARE_H

