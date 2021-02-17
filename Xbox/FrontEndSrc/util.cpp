//-----------------------------------------------------------------------------
// File: util.cpp
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "ReVolt.h"
#include "util.h"


/////////////////////////////////////////////////////////////////////
//
// GoodWrap: wrap a number within a range without assuming it is
// less than one "range" away from the range
//
/////////////////////////////////////////////////////////////////////
REAL GoodWrap(REAL *var, REAL min, REAL max)
{ 
    int n; 
    REAL diff, range; 
    if (*var < min) { 
        range = max - min; 
        diff = min - *var; 
        n = (int) (diff / range); 
        *var += range * (n + 1); 
    } 
    else if (*var > max) { 
        range = max - min; 
        diff = *var - max; 
        n = (int) (diff / range); 
        *var -= range * (n + 1); 
    } 
    return *var;
}


// r,g,b values are from 0 to 1
// h = [0,360], s = [0,1], v = [0,1]
//              if s == 0, then h = -1 (undefined)

void RGBtoHSV( REAL r, REAL g, REAL b, REAL *h, REAL *s, REAL *v )
{
    REAL min, max, delta;

    min = Min( Min(r, g), b );
    max = Max( Max(r, g), b );
    *v = max;                               // v
    delta = max - min;

    if( max != 0 )
        *s = delta / max;               // s
    else {
        // r = g = b = 0                // s = 0, v is undefined
        *s = 0;
        *h = -1;
        return;
    }

    if( r == max )
        *h = ( g - b ) / delta;         // between yellow & magenta
    else if( g == max )
        *h = 2 + ( b - r ) / delta;     // between cyan & yellow
    else
        *h = 4 + ( r - g ) / delta;     // between magenta & cyan
    *h *= 60;                               // degrees

    if( *h < 0 )
        *h += 360;

}

void HSVtoRGB(REAL h, REAL s, REAL v, REAL *r, REAL *g, REAL *b )
{
    int i;
    REAL f, p, q, t;
    
    if( s == 0 ) {
        // achromatic (grey)
        *r = *g = *b = v;
        return;
    }

    h /= 60;                        // sector 0 to 5
    i = Int( h );
    f = h - i;                      // factorial part of h
    p = v * ( 1 - s );
    q = v * ( 1 - s * f );
    t = v * ( 1 - s * ( 1 - f ) );

    switch( i ) {
        case 0:
            *r = v;
            *g = t;
            *b = p;
            break;
        case 1:
            *r = q;
            *g = v;
            *b = p;
            break;
        case 2:
            *r = p;
            *g = v;
            *b = t;
            break;
        case 3:
            *r = p;
            *g = q;
            *b = v;
            break;
        case 4:
            *r = t;
            *g = p;
            *b = v;
            break;
        default:                // case 5:
            *r = v;
            *g = p;
            *b = q;
            break;
    }
}

//$ADDITION

//$TODO: maybe wrap this stuff in a #ifdef so it gets removed from shipping version.

__forceinline ULONGLONG RDTSC()  { __asm rdtsc }

//****************************
// FRAMERATE CALCULATION CODE
//****************************
double g_fFramesPerSec;

static ULONGLONG s_qwSampleStart;
static DWORD     s_dwSampleFrames;
static DWORD     s_dwCyclesPerSec; // DWORD okay here up to approx 4 GHz

double FrameRate_GetMsecPerFrame(void)  { return (1000.0 / g_fFramesPerSec); }
double FrameRate_GetFPS(void)           { return (g_fFramesPerSec); }

void FrameRate_Init(void)
{
    ULONGLONG qwTemp;
    QueryPerformanceFrequency((LARGE_INTEGER*)&qwTemp);
    s_dwCyclesPerSec = (DWORD)qwTemp;

    s_qwSampleStart = RDTSC();
    s_dwSampleFrames = 0;
    g_fFramesPerSec = 0.0;
}

void FrameRate_NextFrame(void)
{
    s_dwSampleFrames++;
    ULONGLONG qwCurr = RDTSC();
    DWORD dwSampleCycles = (DWORD)(qwCurr - s_qwSampleStart);
      // DWORD okay here if sample period never larger than approx 4 billion cycles

    if( dwSampleCycles >= (s_dwCyclesPerSec/2) )  // have we waited long enough?
    {
        // Compute framerate
        // Note: use doubles b/c floats could drop precision here
        double fSecElapsed = double(dwSampleCycles) / double(s_dwCyclesPerSec);
        g_fFramesPerSec = double(s_dwSampleFrames) / fSecElapsed;
        //double fMsecElapsed = double(dwSampleCycles) * 1000.0 / double(s_dwCyclesPerSec);
        //g_fMsecPerFrame = fMsecElapsed / double(s_dwSampleFrames);

        // Reset for next time
        s_qwSampleStart = qwCurr;
        s_dwSampleFrames = 0;
    }
}

//$END_ADDITION

