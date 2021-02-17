#include "ReVolt.h"
#include "Main.h"


size_t		DBG_AllocatedRAM = 0;
size_t		DBG_NAllocations = 0;
char		*DBG_LogFile = NULL;


static MEMSTORE	*MemStoreHead = NULL;
static char		ErrorMessage[1024];
static size_t	MaxAllocatedRAM = 0;
static bool		AlreadyWarned = FALSE;
static bool		AlreadyAsserted = FALSE;
bool			TellChris = FALSE;

bool AddMemStore(void);
void DeleteMemStore(MEMSTORE *memStore);
MEMSTORE *NextMemStore(MEMSTORE *memStore);
void WriteLogEntry(char *s);
void InitLogFile();
void Error(char *mod, char *func, char *mess, long errno);


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
		wsprintf(ErrorMessage, "Could not Allocate RAM for memory list");
		Box("Error", ErrorMessage, MB_OK | MB_ICONERROR);
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
		wsprintf(ErrorMessage, "Attempt to free null memory store");
		Box("Error", ErrorMessage, MB_OK | MB_ICONEXCLAMATION);
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
	void *ptr;

	// Attempt to allocate the RAM as usual
	ptr = ReleaseMalloc(size + DEBUG_MEMGUARD_SIZE);
	if (ptr == NULL) {
		// allocation failed
		wsprintf(ErrorMessage, "Could not Allocate RAM\nLine: %d\nFile: %s", line, file);
		if (!AlreadyWarned) {
			Box("Error", ErrorMessage, MB_OK | MB_ICONERROR);
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

	// Set the guard byte
#if DEBUG_USE_MEMGUARD
	*((char *)MemStoreHead->Ptr + MemStoreHead->Size) = (DEBUG_GUARD_TYPE)DEBUG_MEMGUARD_CONTENTS;
#endif

	// Keep track of allocated RAM
	DBG_AllocatedRAM += size;
	DBG_NAllocations++;
	if (DBG_AllocatedRAM > MaxAllocatedRAM) MaxAllocatedRAM = DBG_AllocatedRAM;

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
		wsprintf(ErrorMessage, "Attempt to free non-allocated RAM\nFreed at\n\tLine: %d\n\tFile %s\n", line, file);
		if (!AlreadyWarned) {
			Box("Error", ErrorMessage, MB_OK | MB_ICONERROR);
			AlreadyWarned = TRUE;
		}
		WriteLogEntry(ErrorMessage);
		return;
	}

	// Check to see if the Guard byte has been modified
#if DEBUG_USE_MEMGUARD
	if (*(DEBUG_MEMGUARD_TYPE *)((char *)ptrStore->Ptr + ptrStore->Size) != DEBUG_MEMGUARD_CONTENTS) {
		wsprintf(ErrorMessage, "Guard byte overwritten\nAllocated at\n\tLine: %d\n\tFile %s\nFreed at\n\tLine: %d\n\tFile %s\n", ptrStore->Line, ptrStore->File, line, file);
		if (!AlreadyWarned) {
			Box("Warning", ErrorMessage, MB_OK | MB_ICONEXCLAMATION);
			AlreadyWarned = TRUE;
		}
		WriteLogEntry(ErrorMessage);
	}
#endif

	// free the memory
	ReleaseFree(ptr);

	// Update the allocated RAM amount
	DBG_AllocatedRAM -= ptrStore->Size;
	DBG_NAllocations --;
	if (DBG_AllocatedRAM < 0 || DBG_NAllocations < 0) {
		wsprintf(ErrorMessage, "Too much memory is being deallocated\n\tLine: %d\n\tFile %s\n(Should never get this message!!)", line, file);
		if (!AlreadyWarned) {
			Box("Error", ErrorMessage, MB_OK | MB_ICONERROR);
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
		wsprintf(ErrorMessage, "Still have RAM allocated\nPrinting log to %s", DBG_LogFile);
		Box("Error", ErrorMessage, MB_OK | MB_ICONERROR);
	}
	if (AlreadyWarned) {
		wsprintf(ErrorMessage, "Check Log file \"%s\"", DBG_LogFile);
		Box("Error", ErrorMessage, MB_OK | MB_ICONEXCLAMATION);
	}
	if (TellChris) {
		wsprintf(ErrorMessage, "Mail log file \"%s\" to Chris please!", DBG_LogFile);
		Box("Mysterious Warning", ErrorMessage, MB_OK | MB_ICONEXCLAMATION);
	}

	fp = fopen(DBG_LogFile, "a");
	if (fp == NULL) {
		wsprintf(ErrorMessage, "Could not open log file \"%s\" for writing", DBG_LogFile);
		Box("Error", ErrorMessage, MB_OK | MB_ICONERROR);
		return;
	}

	// Write out error log file
#if USE_DEBUG_ROUTINES
	fprintf(fp, "\nMax Impulse Magnitude:  %f\n", DEBUG_MaxImpulseMag);
	fprintf(fp, "\nMax Ang Imp Magnitude:  %f\n\n", DEBUG_MaxAngImpulseMag);
#endif
	fprintf(fp, "Max RAM  allocated:     %ld bytes\n", MaxAllocatedRAM);
	fprintf(fp, "RAM still allocated:    %ld bytes\n", DBG_AllocatedRAM);
	fprintf(fp, "Blocks still allocated: %ld\n\n", DBG_NAllocations);

	iMem = 0;
	for (memStore = MemStoreHead; memStore != NULL; memStore = NextMemStore(memStore)) {

		fprintf(fp, "Block %5d\n", iMem++);
		fprintf(fp, "\tSize: %ld bytes\n\tFile: %s\n\tLine: %d\n", 
			memStore->Size, memStore->File, memStore->Line);
	}
	fclose(fp);
}

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

	wsprintf(ErrorMessage, "Assertion Failed\nLine: %d\nFile: %s\n", line, file);
	if (!AlreadyAsserted) {
		Box("Error", ErrorMessage, MB_OK | MB_ICONERROR);
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

	wsprintf(buf, "ERROR (%d) in %s - %s", errno, mod, func);
	Box(buf, mess, MB_OK | MB_ICONERROR);
	WriteLogEntry(ErrorMessage);
	QuitGame = TRUE;
}