//-----------------------------------------------------------------------------
// File: load.h
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef LOAD_H
#define LOAD_H

// re-define fopen

#define fopen(_f, _m) BKK_fopen(_f, _m)

// prototypes

FILE *BKK_fopen(const char *filename, const char *mode);

#endif // LOAD_H

