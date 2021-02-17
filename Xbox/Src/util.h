//-----------------------------------------------------------------------------
// File: Util.h
//
// Desc: Useful functions.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef UTIL_H
#define UTIL_H

/////////////////////////////////////////////////////////////////////
//
// abs: return absolute value of passed variable
//
/////////////////////////////////////////////////////////////////////
#define abs(x) ((x) > 0 ? (x) : -(x))

/////////////////////////////////////////////////////////////////////
//
// Wrap: contstrain a variable to remain within the limits passed
// and if it goes past the end, wrap it around to the other end
//
/////////////////////////////////////////////////////////////////////
#define Wrap(var, min, max) \
{ \
    if ((var) < (min)) (var) = (max) + (var) - (min); \
    if ((var) >= (max)) (var) = (min) + (var) - (max); \
}


#define Limit(var, min, max) \
{ \
    if ((var) < (min)) (var) = (min); \
    if ((var) > (max)) (var) = (max); \
} 

        
/////////////////////////////////////////////////////////////////////
//
// Sign: return -1, or +1 depending on the sign of a variable
//
/////////////////////////////////////////////////////////////////////
#define Sign(var) (((var) == 0)? 0: ((var) < 0)? -1: 1)


/////////////////////////////////////////////////////////////////////
//
// Min and Max:
//
/////////////////////////////////////////////////////////////////////
#define Min(a, b) (((a) < (b))? a: b)
#define Max(a, b) (((a) > (b))? a: b)


/////////////////////////////////////////////////////////////////////
//
// ApproxEqual: determine whether to REALs are roughly equal
//
/////////////////////////////////////////////////////////////////////
#define ApproxEqual(a, b) ( (abs((a) - (b)) < SIMILAR_REAL)? TRUE: FALSE)

////////////////////////////////////////
// frand: return a random float 0 - n //
////////////////////////////////////////

#define frand(_n) \
    ((REAL)rand() / RAND_MAX * (_n))


/////////////////////////////////////////////////////////////////////
// FastDistance()   Calculates the distance between 2 points
//                  with an error of +-9%
/////////////////////////////////////////////////////////////////////
#ifdef _PC
#define FastLength(a, dist)                                                         \
{                                                                                   \
    if (abs((a)->v[0]) > abs((a)->v[1]))                                            \
    {                                                                               \
        if (abs((a)->v[0]) > abs((a)->v[2]))                                        \
            *dist = abs((a)->v[0]) + ((abs((a)->v[1]) + abs((a)->v[2])) * Real(0.25));  \
        else                                                                        \
            *dist = abs((a)->v[2]) + ((abs((a)->v[0]) + abs((a)->v[1])) * Real(0.25));  \
    }                                                                               \
    else if (abs((a)->v[1]) > abs((a)->v[2]))                                       \
        *dist = abs((a)->v[1]) + ((abs((a)->v[0]) + abs((a)->v[2])) * Real(0.25));      \
    else                                                                            \
        *dist = abs((a)->v[2]) + ((abs((a)->v[1]) + abs((a)->v[2])) * Real(0.25));      \
}
#endif

#ifdef _N64
#define FastLength(a, dist)                                                         \
{                                                                                   \
    if (abs((a)->v[0]) > abs((a)->v[1]))                                            \
    {                                                                               \
        if (abs((a)->v[0]) > abs((a)->v[2]))                                        \
            *dist = abs((a)->v[0]) + ((abs((a)->v[1]) + abs((a)->v[2])) * Real(0.25));  \
        else                                                                        \
            *dist = abs((a)->v[2]) + ((abs((a)->v[0]) + abs((a)->v[1])) * Real(0.25));  \
    }                                                                               \
    else if (abs((a)->v[1]) > abs((a)->v[2]))                                       \
        *dist = abs((a)->v[1]) + ((abs((a)->v[0]) + abs((a)->v[2])) * Real(0.25));      \
    else                                                                            \
        *dist = abs((a)->v[2]) + ((abs((a)->v[1]) + abs((a)->v[2])) * Real(0.25));      \
}
#endif

#ifdef _PSX
#define FastLength(a, dist)                                                         \
{                                                                                   \
    if (abs((a)->v[0]) > abs((a)->v[1]))                                            \
    {                                                                               \
        if (abs((a)->v[0]) > abs((a)->v[2]))                                        \
            *dist = abs((a)->v[0]) + ((abs((a)->v[1]) + abs((a)->v[2])) >> 2);          \
        else                                                                        \
            *dist = abs((a)->v[2]) + ((abs((a)->v[0]) + abs((a)->v[1])) >> 2);          \
    }                                                                               \
    else if (abs((a)->v[1]) > abs((a)->v[2]))                                       \
        *dist = abs((a)->v[1]) + ((abs((a)->v[0]) + abs((a)->v[2])) >> 2);              \
    else                                                                            \
        *dist = abs((a)->v[2]) + ((abs((a)->v[1]) + abs((a)->v[2])) >> 2);              \
}
#endif

////////////////
// prototypes //
////////////////

extern REAL GoodWrap(REAL *var, REAL min, REAL max);
extern void HSVtoRGB( REAL h, REAL s, REAL v, REAL *r, REAL *g, REAL *b );
extern void RGBtoHSV( REAL r, REAL g, REAL b, REAL *h, REAL *s, REAL *v );
//$ADDITION
__forceinline ULONGLONG RDTSC();
extern double g_fFramesPerSec;
double FrameRate_GetMsecPerFrame(void);
double FrameRate_GetFPS(void);
void   FrameRate_Init(void);
void   FrameRate_NextFrame(void);
//$END_ADDITION


#endif // UTIL_H

