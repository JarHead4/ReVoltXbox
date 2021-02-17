//-------------------------------------------------------------------------------------------------------------------------------
#ifndef JCTYPES_HPP
#define JCTYPES_HPP
//-------------------------------------------------------------------------------------------------------------------------------
// Type definitions - (JC) 7th June 1996
//-------------------------------------------------------------------------------------------------------------------------------
typedef	unsigned	char	U8;			// Unsigned  8-bit quantity.
typedef	  signed	char	S8;			// Signed    8-bit quantity.
typedef	unsigned	short	U16;		// Unsigned 16-bit quantity.
typedef	  signed	short	S16;		// Signed   16-bit quantity.
typedef	unsigned	long	U32;		// Unsigned 32-bit quantity.
typedef	  signed	long	S32;		// Signed   32-bit quantity.
//-------------------------------------------------------------------------------------------------------------------------------
#define  U8_MIN           0
#define  U8_MAX         255
#define  S8_MIN        -128
#define  S8_MAX         127
#define U16_MIN           0
#define U16_MAX       65535
#define S16_MIN      -32768
#define S16_MAX       32767
#define U32_MIN           0
#define U32_MAX  4294967295
#define S32_MIN (-2147483647-1)
#define S32_MAX   2147483647
//-------------------------------------------------------------------------------------------------------------------------------
#ifdef _WINDOWS
#else
 #ifdef _CONSOLE
 #else
  typedef unsigned long	BOOL;		// Boolean.
 #endif
#endif
//-------------------------------------------------------------------------------------------------------------------------------
#endif
//-------------------------------------------------------------------------------------------------------------------------------
