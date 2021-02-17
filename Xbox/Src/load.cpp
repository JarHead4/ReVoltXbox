//-----------------------------------------------------------------------------
// File: load.cpp
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef WIN32_LEAN_AND_MEAN  //<-- $ADDITION
#define WIN32_LEAN_AND_MEAN
#endif //WIN32_LEAN_AND_MEAN //<-- $ADDITION
#ifndef _PC   //<-- $ADDITION
#define _PC
#endif //_PC  //<-- $ADDITION

//$MODIFIED
//#include <windows.h>
#include <xtl.h>
//$END_MODIFICATIONS
#include <stdio.h>

extern char LastFile[];
extern void WriteLogEntry(char *s);
extern char *DBG_LogFile;

/////////////////
// open a file //
/////////////////

FILE *BKK_fopen(const char *filename, const char *mode)
{
    FILE *fp = fopen(filename, mode);

    if (_stricmp(filename, DBG_LogFile)) //$MODIFIED: changed stricmp to _stricmp
    {
        char buf[MAX_PATH];
        sprintf(buf, "Loading: %s: %s\n", filename, fp ? "Found" : "Not Found");
        WriteLogEntry(buf);
    }

    strncpy(LastFile, filename, MAX_PATH);

    return fp;
}

