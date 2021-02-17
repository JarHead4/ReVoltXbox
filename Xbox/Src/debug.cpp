//-----------------------------------------------------------------------------
// File: debug.cpp
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "ReVolt.h"
#include "Main.h"

size_t      DBG_AllocatedRAM = 0;
size_t      DBG_NAllocations = 0;
char        *DBG_LogFile = NULL;
char        DBG_TempPath[MAX_PATH];  //$TODO(cprince): can merge this with DBG_LogFile
char        DBG_MallocString[MALLOC_STRING_LENGTH] = "";

static MEMSTORE *MemStoreHead = NULL;
static char     ErrorMessage[1024];
static size_t   MaxAllocatedRAM = 0;
static size_t   TotalAllocatedRAM = 0;
static bool     AlreadyWarned = FALSE;
static bool     AlreadyAsserted = FALSE;
bool            TellChris = FALSE;

bool AddMemStore(void);
void DeleteMemStore(MEMSTORE *memStore);
MEMSTORE *NextMemStore(MEMSTORE *memStore);
void WriteLogEntry(char *s);
void InitLogFile();
void Error(char *mod, char *func, char *mess, long errno);

#if USE_DEBUG_ROUTINES

/////////////////////////////////////////////////////////////////////
//
// AddMemStore: add a new element to the memory info list
//
/////////////////////////////////////////////////////////////////////

bool AddMemStore()
{
    MEMSTORE *oldHead;
    
    oldHead = MemStoreHead;

    // create new memory info store
    MemStoreHead = (MEMSTORE *)ReleaseMalloc(sizeof(MEMSTORE));
    if (MemStoreHead == NULL) {
        sprintf(ErrorMessage, "Could not Allocate RAM for memory list");
        DumpMessage("Error", ErrorMessage);
        return FALSE;
    }

    MemStoreHead->Next = oldHead;
    MemStoreHead->Prev = NULL;
    if (oldHead != NULL) {
        oldHead->Prev = MemStoreHead;
    }
    return TRUE;
}

/////////////////////////////////////////////////////////////////////
//
// DeleteMemStore: delete element from the memory info list
//
/////////////////////////////////////////////////////////////////////

void DeleteMemStore(MEMSTORE *memStore) {

    if (memStore == NULL) {
        sprintf(ErrorMessage, "Attempt to free null memory store");
        DumpMessage("Error", ErrorMessage;
        return;
    }

    // Set up the pointers on the adjacent list elements
    if (memStore->Prev != NULL) {
        (memStore->Prev)->Next = memStore->Next;
    } else {
        MemStoreHead = memStore->Next;
    }
    if (memStore->Next != NULL) {
        (memStore->Next)->Prev = memStore->Prev;
    }

    // delete the item
    ReleaseFree(memStore);

}


/////////////////////////////////////////////////////////////////////
//
// NextMemStore: return pointer to next item in list
//
/////////////////////////////////////////////////////////////////////

MEMSTORE *NextMemStore(MEMSTORE *memStore)
{
    if (memStore == NULL) {
        return NULL;
    }

    return memStore->Next;
}


/////////////////////////////////////////////////////////////////////
//
// DebugMalloc: allocate RAM and store info about it
//
/////////////////////////////////////////////////////////////////////

void *DebugMalloc(size_t size, int line, char *file)
{
    unsigned long extra, iGuard;
    unsigned char *guardData;
    void *ptr;
    MEMORYSTATUS status;

    // Write log entry
    //sprintf(ErrorMessage, "Allocating RAM: %s\n", DBG_MallocString);
    //WriteLogEntry(DBG_MallocString);

    // Round size to upper 4 bytes
#if DEBUG_USE_MEMGUARD
    extra = 4 - (size % 4);
    if (extra == 4) extra = 0;
#else
    extra = 0;
#endif

    // Attempt to allocate the RAM as usual
    ptr = ReleaseMalloc(size + extra + DEBUG_MEMGUARD_SIZE);
    if (ptr == NULL) {
        // allocation failed
        sprintf(ErrorMessage, "Could not Allocate RAM\nLine: %d\nFile: %s", line, file);
        if (!AlreadyWarned) {
            DumpMessage("Error", ErrorMessage);
            AlreadyWarned = TRUE;
        }
        WriteLogEntry(ErrorMessage);
        return NULL;
    }

    // Create a new item on the allocation list
    AddMemStore();

    // Fill it with information
    MemStoreHead->Ptr = ptr;
    MemStoreHead->Size = size;
    MemStoreHead->Line = line;
    MemStoreHead->File = file;
    strncpy(MemStoreHead->Message, DBG_MallocString, MALLOC_STRING_LENGTH);
    //DBG_MallocString[0] = '\0';

    // Set the guard byte
#if DEBUG_USE_MEMGUARD
    guardData = (unsigned char*)MemStoreHead->Ptr + MemStoreHead->Size;
    for (iGuard = 0; iGuard < extra + DEBUG_MEMGUARD_SIZE; iGuard++) {
        *guardData = DEBUG_MEMGUARD_CONTENTS;
        guardData++;
    }
#endif

    // Keep track of allocated RAM
    DBG_AllocatedRAM += size;
    DBG_NAllocations++;
    if (DBG_AllocatedRAM > MaxAllocatedRAM) MaxAllocatedRAM = DBG_AllocatedRAM;

    // check total used mem
    GlobalMemoryStatus(&status);
    if ((status.dwTotalPhys - status.dwAvailPhys) > TotalAllocatedRAM) TotalAllocatedRAM = (status.dwTotalPhys - status.dwAvailPhys);

    // return mem ptr
    return ptr;
}


/////////////////////////////////////////////////////////////////////
//
// DebugFree: free the memory associated with the passed pointer.
// 
/////////////////////////////////////////////////////////////////////

void DebugFree(void *ptr, int line, char *file)
{
    bool foundPtr;
    MEMSTORE *ptrStore, *nextMem;

    // do nothing for a null pointer
    if (ptr == NULL) {
        return;
    }

    // locate the pointer in the allocated list
    foundPtr = FALSE;
    for (nextMem = MemStoreHead; nextMem != NULL; nextMem = NextMemStore(nextMem)) {
        if (nextMem->Ptr == ptr) {
            ptrStore = nextMem;
            foundPtr = TRUE;
            break;
        }
    }

    // make sure the memory has already been allocated
    if (!foundPtr) {
        sprintf(ErrorMessage, "Attempt to free non-allocated RAM\nFreed at\n\tLine: %d\n\tFile %s\n", line, file);
        if (!AlreadyWarned) {
            DumpMessage("Error", ErrorMessage);
            AlreadyWarned = TRUE;
        }
        WriteLogEntry(ErrorMessage);
        return;
    }

    // Check to see if the Guard byte has been modified
#if DEBUG_USE_MEMGUARD
    unsigned long extra, iGuard;
    unsigned char *guard;

    extra = 4 - (ptrStore->Size % 4);
    if (extra == 4) extra = 0;

    guard = (unsigned char*)ptrStore->Ptr + ptrStore->Size;
    for (iGuard = 0; iGuard < extra + DEBUG_MEMGUARD_SIZE; iGuard++) {
        if (*guard != DEBUG_MEMGUARD_CONTENTS) {
            sprintf(ErrorMessage, "Guard byte overwritten\nAllocated at\n\tLine: %d\n\tFile %s\nFreed at\n\tLine: %d\n\tFile %s\n", ptrStore->Line, ptrStore->File, line, file);
            if (!AlreadyWarned) {
                DumpMessage("Warning", ErrorMessage);
                AlreadyWarned = TRUE;
            }
            WriteLogEntry(ErrorMessage);
            sprintf(ErrorMessage, "Message: %s\n", ptrStore->Message);
            WriteLogEntry(ErrorMessage);
            break;
        }
        guard++;
    }
#endif

    // free the memory
    ReleaseFree(ptr);

    // Update the allocated RAM amount
    DBG_AllocatedRAM -= ptrStore->Size;
    DBG_NAllocations --;
    if (DBG_AllocatedRAM < 0 || DBG_NAllocations < 0) {
        sprintf(ErrorMessage, "Too much memory is being deallocated\n\tLine: %d\n\tFile %s\n(Should never get this message!!)", line, file);
        if (!AlreadyWarned) {
            DumpMessage("Error", ErrorMessage);
            AlreadyWarned = TRUE;
        }
        WriteLogEntry(ErrorMessage);
        return;
    }


    // free the memory store
    DeleteMemStore(ptrStore);

}


/////////////////////////////////////////////////////////////////////
//
// CheckMemoryAllocation: Make sure that all allocated RAM has been
// freed. If not, show a message and print out a log file
//
/////////////////////////////////////////////////////////////////////
#if USE_DEBUG_ROUTINES
extern REAL DEBUG_MaxImpulseMag;
extern REAL DEBUG_MaxAngImpulseMag;
#endif

void CheckMemoryAllocation()
{
    int iMem;
    MEMSTORE *memStore;
    FILE *fp;

    if (DBG_AllocatedRAM != 0 || DBG_NAllocations != 0) {
        sprintf(ErrorMessage, "Still have RAM allocated\nPrinting log to %s", DBG_LogFile);
        DumpMessage("Error", ErrorMessage);
    }
    if (AlreadyWarned) {
        sprintf(ErrorMessage, "Check Log file \"%s\"", DBG_LogFile);
        DumpMessage("Error", ErrorMessage);
    }
    if (TellChris) {
        sprintf(ErrorMessage, "Mail log file \"%s\" to Chris please!", DBG_LogFile);
        DumpMessage("Mysterious Warning", ErrorMessage);
    }

    fp = fopen(DBG_LogFile, "a");
    if (fp == NULL) {
        sprintf(ErrorMessage, "Could not open log file \"%s\" for writing", DBG_LogFile);
        DumpMessage("Error", ErrorMessage);
        return;
    }

    // Write out error log file
#if USE_DEBUG_ROUTINES
    fprintf(fp, "\nMax Impulse Magnitude:  %f\n", DEBUG_MaxImpulseMag);
    fprintf(fp, "\nMax Ang Imp Magnitude:  %f\n\n", DEBUG_MaxAngImpulseMag);
#endif
    fprintf(fp, "Max RAM  allocated:     %ld bytes\n", MaxAllocatedRAM);
    fprintf(fp, "Total RAM used:         %ld bytes\n", TotalAllocatedRAM);
    fprintf(fp, "RAM still allocated:    %ld bytes\n", DBG_AllocatedRAM);
    fprintf(fp, "Blocks still allocated: %ld\n\n", DBG_NAllocations);

    iMem = 0;
    for (memStore = MemStoreHead; memStore != NULL; memStore = NextMemStore(memStore)) {

        fprintf(fp, "Block %5d\n", iMem++);
        fprintf(fp, "\tSize: %ld bytes\n\tFile: %s\n\tLine: %d\n", 
            memStore->Size, memStore->File, memStore->Line);
        fprintf(fp, "\tMessage: %s\n", memStore->Message);
    }
    fclose(fp);
}
#endif //USE_DEBUG_ROUTINES


void WriteLogEntry(char *s)
{
    FILE *fp;

    fp = fopen(DBG_LogFile, "a");
    if (fp == NULL) {
        return;
    }
    fprintf(fp, "%s", s);
    fclose(fp);
}

void InitLogFile()
{
    FILE *fp;

    fp = fopen(DBG_LogFile, "w");
    if (fp == NULL) {
        return;
    }

    fprintf(fp, "ReVolt Error Log File\n");
    fprintf(fp, "Compilation date %s, %s\n\n", __TIME__, __DATE__);

    fclose(fp);
}

/////////////////////////////////////////////////////////////////////
//
// DebugAssert: check for validity of assertion and show error box
// if failed
//
/////////////////////////////////////////////////////////////////////

void DebugAssert(bool result, int line, char *file)
{
    if (result) return;

    sprintf(ErrorMessage, "Assertion Failed\nLine: %d\nFile: %s\n", line, file);
    if (!AlreadyAsserted) {
        DumpMessage("Error", ErrorMessage);
        AlreadyAsserted = TRUE;
    }
    WriteLogEntry(ErrorMessage);
}


//
// ERROR
//
// Display N64/PSX error message using PC message box
//

void Error(char *mod, char *func, char *mess, long errno)
{
    char buf[256];

    sprintf(buf, "ERROR (%d) in %s - %s", errno, mod, func);
    DumpMessage(buf, mess);
    WriteLogEntry(ErrorMessage);  //$CMP_NOTE: should this be "buf" instead of "ErrorMessage" !?!
    QuitGame();
}


////////////////////////////////////////////////////////////////
//
// SetMallocString:
//
////////////////////////////////////////////////////////////////
#if USE_DEBUG_ROUTINES
void SetMallocString(char *string)
{
    strncpy(DBG_MallocString, string, MALLOC_STRING_LENGTH);
    DBG_MallocString[MALLOC_STRING_LENGTH - 1] = '\0';
}
#endif


//$ADDITION_BEGIN
//-----------------------------------------------------------------------------
// Name: SetupFPUExceptions()
// Desc: Enables or disables the set of interesting floating-point exceptions.
//-----------------------------------------------------------------------------
void SetupFPUExceptions( bool bEnableExceptions )
{
    // Note: the FPU control word tells which exceptions are masked, so an
    // exception is ENABLED by setting its control word bit to ZERO.
    //
    // The Pentium floating-point exception conditions are explained at:
    // http://developer.intel.com/design/intarch/techinfo/pentium/fpu.htm
    
    // specify which exceptions we'll enable/disable
    unsigned int exception_mask = (
          _EM_ZERODIVIDE // divide by zero
        | _EM_INVALID    // invalid operand or stack underflow/overflow
        | _EM_OVERFLOW   // numeric overflow: result too large
        | _EM_UNDERFLOW  // numeric underflow: result too small
//      | _EM_INEXACT    // result was rounded: some precision lost
//      | _EM_DENORMAL   // denormalized (tiny) operand <<x86 only>>
    );

    unsigned int ctrlword = _control87(0,0);  // get old FPU control word value

    if( bEnableExceptions )
        ctrlword &= ~(exception_mask);
    else
        ctrlword |= (exception_mask);

    _control87(ctrlword,MCW_EM);  // set new FPU control word (exception part only)
}
//$ADDITION_END

