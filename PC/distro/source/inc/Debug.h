/////////////////////////////////////////////////////////////////////
// 
// Replacement debug routines to help track down elusive little bugs
//
/////////////////////////////////////////////////////////////////////

#ifndef __DEBUG_H__
#define __DEBUG_H__

#define USE_DEBUG_ROUTINES	(TRUE)
#define DEBUG_USE_MEMGUARD	(FALSE)
#define DEBUG_GUARD_BYTE	(0xE3)

/////////////////////////////////////////////////////////////////////
//
// Debug memory routines:
//
// - keep track of where memory was allocated from
// - keep track of amount of allocated ram
// - keep track of peak amount of allocated ram
//
/////////////////////////////////////////////////////////////////////

#if USE_DEBUG_ROUTINES
#define malloc(x)			DebugMalloc(x, __LINE__, __FILE__)
#define free(x)				DebugFree(x, __LINE__, __FILE__)
#define ReleaseMalloc(x)	((void *)(LocalAlloc(LMEM_FIXED, (x))))
#define ReleaseFree(x)		(LocalFree((HLOCAL)(x)))
#else
#define malloc(x)	((void *)(LocalAlloc(LMEM_FIXED, (x))))
#define free(x)		(LocalFree((HLOCAL)(x)))
#define ReleaseMalloc(x)	malloc(x)
#define ReleaseFree(x)		free(x)
#endif

#if DEBUG_USE_MEMGUARD
#define DEBUG_MEMGUARD_TYPE	long
#define DEBUG_MEMGUARD_SIZE (sizeof(long))
#define DEBUG_MEMGUARD_CONTENTS (0xe3e3e3e3)
#else
#define DEBUG_MEMGUARD_SIZE (0)
#endif

typedef struct MemStorageStruct {
	void	*Ptr;							// pointer to the allocated ram
	size_t	Size;							// amount of ram that was allocated (an extra byte is added as a guard byte)
	char	*File;							// first few characters of file where allocated
	int		Line;							// line number where allocated

	struct MemStorageStruct *Prev;
	struct MemStorageStruct *Next;
} MEMSTORE;

extern void *DebugMalloc(size_t size, int line, char *file);
extern void DebugFree(void *p, int line, char *file);
extern void CheckMemoryAllocation(void);
extern void Error(char *mod, char *func, char *mess, long errno);


extern size_t DBG_AllocatedRAM;
extern char *DBG_LogFile;;
extern bool TellChris;

/////////////////////////////////////////////////////////////////////
//
// Assertion Routines
//
/////////////////////////////////////////////////////////////////////

#ifdef USE_DEBUG_ROUTINES
#define Assert(x)	DebugAssert((x), __LINE__, __FILE__)
#else
#define Assert(x)	(NULL)
#endif

extern void DebugAssert(bool ExpResult, int line, char *file);




/////////////////////////////////////////////////////////////////////
//
// Error log file stuff
//
/////////////////////////////////////////////////////////////////////

extern void InitLogFile();
extern void WriteLogEntry(char *s);


#endif