//-----------------------------------------------------------------------------
// File: MakeStringsBinFromInf.cpp
//
// Desc: Turns .inf unicode text files that contain localized text strings for
//       Re-volt into .bin binary files that are loaded at runtime on the Xbox.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include <windows.h>




//-----------------------------------------------------------------------------
// Name: ReadString()
// Desc: Reads a line of text from a file
//-----------------------------------------------------------------------------
WCHAR* ReadString( WCHAR* strBuffer, int maxsize, FILE* file )
{
    WCHAR* pReturnString = strBuffer;
    
    while( TRUE )
    {
        WCHAR c = fgetwc( file );

        if( c == WEOF )
            return NULL;

        if( c == L'\r' )
        {
            *strBuffer++ = 0;
            return pReturnString;
        }

        *strBuffer++ = c;
    }
}




//-----------------------------------------------------------------------------
// Name: main()
// Desc: Entry point to the app
//-----------------------------------------------------------------------------
int _cdecl main( int argc, char* argv[] )
{
    // Make sure we have enough arguments, else show the usage
    if( argc < 3 )
    {
        wprintf( L"USAGE: MakeStringsBinFromInf infile.inf outfile.bin\n" );
        return(-1);
    }

    // Open the input .inf file for reading
    WCHAR strInFileName[MAX_PATH];
    swprintf( strInFileName,  L"%S", argv[1] );
    FILE* infile  = _wfopen( strInFileName,  L"rb" );
    if( NULL==infile )
    {
        wprintf( L"Error: Could not open %s for reading!\n", strInFileName );
        return(-1);
    }

    // Open the output .bin file for writing
    WCHAR strOutFileName[MAX_PATH];
    swprintf( strOutFileName, L"%S", argv[2] );
    FILE* outfile = _wfopen( strOutFileName, L"wb" );
    if( NULL==outfile )
    {
        fclose( infile );
        wprintf( L"Error: Could not open %s for writing!\n", strOutFileName );
        return(-1);
    }

    // Skip the unicode marker
    WORD w = fgetwc( infile );

    // Skip past [Strings] and blank line
    WCHAR strBuffer[512];
    fgetws( strBuffer, 512, infile );
    fgetws( strBuffer, 512, infile );

    // Loop through all strings in the infile, and write them to the outfile
    DWORD dwNumStringsProcessed = 0L;
    while( TRUE )
    {
        // Get the next line
        if( NULL == fgetws( strBuffer, 512, infile ) )
            break;

        // Skip blank lines and comments
        if( strBuffer[1] == 0 )
            continue;
        if( strBuffer[1] == L' ' )
            continue;
        if( strBuffer[1] == L'\t' )
            continue;
        if( strBuffer[0] == L'\r' )
            continue;
        if( strBuffer[0] == L'\n' )
            continue;
        if( strBuffer[1] == L';' )
            continue;

        DWORD  dwLength  = wcslen( strBuffer );
        WCHAR* pstr_head = &strBuffer[0];
        WCHAR* pstr_tail = &strBuffer[dwLength-1];

        // Advance the string head just past the opening quote
        while( *pstr_head++ != L'\"' )
        {}

        pstr_head = pstr_head;

        // Truncate the string tail to just before the closing quote
        while( *pstr_tail != L'\"' )
        {
            pstr_tail--;
        }
        *pstr_tail = 0;

        dwLength = wcslen( pstr_head );

        // Check for bogus newline
        for( DWORD i=0; i<dwLength; i++ )
        {
            if( pstr_head[i] == 0x00a0 )
                pstr_head[i] = 0x0020;
        }

        // Look for special characters
        for( i=0; i<dwLength; i++ )
        {
            // Check for ""
            if( pstr_head[i] == L'\"' && pstr_head[i+1] == L'\"' )
            {
                // Remove redundant quote
                wcscpy( &pstr_head[i+1], &pstr_head[i+2] );
                dwLength--;
                continue;
            }

            if( pstr_head[i] == L'\\' )
            {
                // Check for \n
                if( pstr_head[i+1] == L'n' )
                {
                    // Replace \n with its binary equivalent
                    pstr_head[i] = L'\n';
                    // Remove extra char
                    wcscpy( &pstr_head[i+1], &pstr_head[i+2] );
                    dwLength--;
                    continue;
                }

                // Check for \000
                if( iswdigit( pstr_head[i+1] ) )
                {
                    DWORD dwNumber = pstr_head[i+1] - L'0';
                    DWORD dwNumDigits = 1;

                    if( iswdigit( pstr_head[i+2] ) )
                    {
                        dwNumber = (dwNumber*8) + pstr_head[i+2] - L'0';
                        dwNumDigits++;

                        if( iswdigit( pstr_head[i+3] ) )
                        {
                            dwNumber = (dwNumber*8) + pstr_head[i+3] - L'0';
                            dwNumDigits++;
                        }
                    }

                    pstr_head[i] = (WCHAR)dwNumber;
                    wcscpy( &pstr_head[i+1], &pstr_head[i+1+dwNumDigits] );
                    dwLength -= dwNumDigits;
                }
            }
        }

        fwrite( (VOID*)pstr_head, sizeof(WCHAR), wcslen(pstr_head)+1, outfile );

        wprintf( L"%s\n", pstr_head );
        dwNumStringsProcessed++;
    }

    fclose( infile );
    fclose( outfile );

    wprintf( L"\n%d strings processed.\n", dwNumStringsProcessed );

    return 0;
}

