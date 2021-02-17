//-----------------------------------------------------------------------------
// File: debug.h
//
// Desc: Replacement debug routines to help track down elusive little bugs.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef DEBUG_H
#define DEBUG_H

#define USE_DEBUG_ROUTINES  (FALSE)
#define DEBUG_USE_MEMGUARD  (FALSE)
#define DEBUG_GUARD_BYTE    (0xE3)

#include <assert.h>  //$ADDITION

/////////////////////////////////////////////////////////////////////
//
// Debug memory routines:
//
// - keep track of where memory was allocated from
// - keep track of amount of allocated ram
// - keep track of peak amount of allocated ram
//
/////////////////////////////////////////////////////////////////////

#define MALLOC_STRING_LENGTH    16

#if USE_DEBUG_ROUTINES
//$REMOVED
//#define malloc(x)           DebugMalloc(x, __LINE__, __FILE__)
//#define free(x)             DebugFree(x, __LINE__, __FILE__)
//$END_REMOVAL
#define ReleaseMalloc(x)    ((void *)(LocalAlloc(LMEM_FIXED, (x))))
#define ReleaseFree(x)      (LocalFree((HLOCAL)(x)))
extern void SetMallocString(char * string);
#else
//$REMOVED
//#define malloc(x)   ((void *)(LocalAlloc(LMEM_FIXED, (x))))
//#define free(x)     (LocalFree((HLOCAL)(x)))
//$END_REMOVAL
#define ReleaseMalloc(x)    malloc(x)
#define ReleaseFree(x)      free(x)
#define SetMallocString(_s) (NULL)
#endif

#if DEBUG_USE_MEMGUARD
#define DEBUG_MEMGUARD_TYPE long
#define DEBUG_MEMGUARD_SIZE (sizeof(long))
#define DEBUG_MEMGUARD_CONTENTS (0xe3)
#else
#define DEBUG_MEMGUARD_SIZE (0)
#endif

typedef struct MemStorageStruct {
    void    *Ptr;                           // pointer to the allocated ram
    size_t  Size;                           // amount of ram that was allocated (an extra byte is added as a guard byte)
    char    *File;                          // first few characters of file where allocated
    int     Line;                           // line number where allocated
    char    Message[MALLOC_STRING_LENGTH];

    struct MemStorageStruct *Prev;
    struct MemStorageStruct *Next;
} MEMSTORE;

extern void *DebugMalloc(size_t size, int line, char *file);
extern void DebugFree(void *p, int line, char *file);
extern void CheckMemoryAllocation(void);
extern void Error(char *mod, char *func, char *mess, long errno);
//$ADDITION_BEGIN
extern void SetupFPUExceptions( bool bEnableExceptions );
//$ADDITION_END

extern size_t DBG_AllocatedRAM;
extern char *DBG_LogFile;
extern char DBG_TempPath[];
extern bool TellChris;

/////////////////////////////////////////////////////////////////////
//
// Assertion Routines
//
/////////////////////////////////////////////////////////////////////

//$MODIFIED
//#if USE_DEBUG_ROUTINES
//#define Assert(x)   DebugAssert((x), __LINE__, __FILE__)
//#else
//#define Assert(x)   (NULL)
//#endif
//
//extern void DebugAssert(bool ExpResult, int line, char *file);

    //$REVISIT(cprince): should we go back to using USE_DEBUG_ROUTINES ?
    /// (Would allow us to add asserts to retail builds, and remove from debug builds, if desired.)
    #define Assert(x)  assert(x)
//$END_MODIFICATIONS

//$ADDITION(cprince)
#ifdef wsprintf
  #undef wsprintf
#endif
#define wsprintf  DONT_USE_WSPRINTF___USE_SPRINTF_OR_SWPRINTF  // Silly rabbit, wsprintf is for TCHARs (which you aren't using).  Explicitly use CHAR sprintf or WCHAR swprintf.
//$END_ADDITION



/////////////////////////////////////////////////////////////////////
//
// Error log file stuff
//
/////////////////////////////////////////////////////////////////////

extern void InitLogFile();
extern void WriteLogEntry(char *s);


#endif // DEBUG_H

