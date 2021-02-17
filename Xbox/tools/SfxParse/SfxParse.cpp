#include <windows.h>
#include <stdio.h>
#include <vector>
#include "SoundEffectEngine.h"

// Update this whenever format is changed
const DWORD VERSION_NUMBER = 0;
const CHAR* HEADER_PRE = "#pragma once\r\n\r\ntypedef enum {\r\n";
const CHAR* HEADER_POST = "};";

BOOL GetLine( HANDLE hDefinition, CHAR strLine[] )
{
    DWORD dwCharsRead = 0;

    for( ; ; )
    {
        DWORD dwRead;

        ReadFile( hDefinition, &strLine[ dwCharsRead ], 1, &dwRead, NULL );
        if( dwRead == 0 || strLine[ dwCharsRead ] == '\n' )
        {
            break;
        }

        if( strLine[ dwCharsRead ] != '\r' )
            dwCharsRead++;
    }

    strLine[ dwCharsRead ] = '\0';

    return( dwCharsRead != 0 );
}


BOOL GetNameValue( HANDLE hDefinition, CHAR** pstrName, CHAR** pstrValue )
{
    static CHAR strLine[80];

    if( !GetLine( hDefinition, strLine ) )
        return FALSE;

    *pstrName = strtok( strLine, "= " );
    if( *pstrName == NULL )
        return FALSE;

    *pstrValue = strtok( NULL, "= " );
    if( *pstrValue == NULL )
        return FALSE;

    return TRUE;
}


BOOL GetValue( HANDLE hDefinition, CHAR* strName, CHAR** pstrValue )
{
    CHAR* strCurrentName;

    if( !GetNameValue( hDefinition, &strCurrentName, pstrValue ) )
        return FALSE;

    if( stricmp( strCurrentName, strName ) )
    {
        printf( "Expected %s, got %s.\n", strName, strCurrentName );
        return FALSE;
    }

    return TRUE;
}


VOID ParseSDF( HANDLE hDefinition, HANDLE hOutput, HANDLE hHeader )
{
    DWORD dwNumEffects = 0;
    CSoundEffectDefinition m_aEffects[ MAX_EFFECTS ];
    std::vector<CSoundEffectAsset> vAssets;
    CHAR* strValue;
    DWORD dwWritten;

    ZeroMemory( m_aEffects, MAX_EFFECTS * sizeof( CSoundEffectDefinition ) );

    WriteFile( hOutput, &VERSION_NUMBER, sizeof( VERSION_NUMBER ), &dwWritten, NULL );
    WriteFile( hHeader, HEADER_PRE, strlen( HEADER_PRE ), &dwWritten, NULL );

    // Get an "effect" statement
    while( GetValue( hDefinition, "effect", &strValue ) )
    {
        CHAR str[80];
        sprintf( str, "EFFECT_%s,\r\n", strValue );
        WriteFile( hHeader, str, strlen(str), &dwWritten, NULL );
        printf( "Effect %d: %s.\n", dwNumEffects, strValue );

        // Get the rolloff factor
        GetValue( hDefinition, "RolloffFactor", &strValue );
        m_aEffects[ dwNumEffects ].m_fRolloffFactor = (FLOAT)atof( strValue );
        printf( "Roloff Factor: %f\n", m_aEffects[ dwNumEffects ].m_fRolloffFactor );

        // Get the number of variations
        GetValue( hDefinition, "variations", &strValue );
        m_aEffects[ dwNumEffects ].m_dwNumVariations = (DWORD)atol( strValue );
        printf( "Effect has %d variations\n", m_aEffects[ dwNumEffects ].m_dwNumVariations );

        // For each variation
        for( DWORD i = 0; i < m_aEffects[ dwNumEffects ].m_dwNumVariations; i++ )
        {
            m_aEffects[ dwNumEffects ].m_aVariations[ i ].m_dwNumAssets = 1;
            m_aEffects[ dwNumEffects ].m_aVariations[ i ].m_aAssets[ 0 ] = vAssets.size();

            CSoundEffectAsset asset;

            GetValue( hDefinition, "asset", &strValue );
            printf( "Variation %d: %s\n", i, strValue );
            asset.m_dwSoundBankEntry = (DWORD)atol(strValue);

            GetValue( hDefinition, "volume", &strValue );
            printf( "\tVolume: %s\n", strValue );
            asset.m_lVolume = (LONG)atol(strValue);

            vAssets.push_back( asset );
        }

        ++dwNumEffects;
    }

    // Write our output file

    // Number of effects
    WriteFile( hOutput, &dwNumEffects, sizeof( DWORD ), &dwWritten, NULL );
    WriteFile( hOutput, m_aEffects, dwNumEffects * sizeof( CSoundEffectDefinition ), &dwWritten, NULL );

    // Number of assets
    DWORD dwNumAssets = vAssets.size();
    WriteFile( hOutput, &dwNumAssets, sizeof( DWORD ), &dwWritten, NULL );
    for( std::vector<CSoundEffectAsset>::iterator i = vAssets.begin(); i != vAssets.end(); ++i )
    {
        WriteFile( hOutput, (CSoundEffectAsset *)i, sizeof( CSoundEffectAsset ), &dwWritten, NULL );
    }

    WriteFile( hHeader, HEADER_POST, strlen( HEADER_POST ), &dwWritten, NULL );
}


VOID main( INT argc, CHAR* argv[] )
{
    if( argc != 4 )
    {
        printf( "Usage: %s <filename.sdf> <output.sfx> <output.h>\n", argv[0] );
        exit( 1 );
    }

    HANDLE hDefinitionFile;
    hDefinitionFile = CreateFile( argv[1],
                                  GENERIC_READ,
                                  FILE_SHARE_READ,
                                  NULL,
                                  OPEN_EXISTING,
                                  FILE_ATTRIBUTE_NORMAL,
                                  NULL );
    if( hDefinitionFile == INVALID_HANDLE_VALUE )
    {
        printf( "Can not open %s for reading.\n", argv[1] );
    }

    HANDLE hOutputFile;
    hOutputFile = CreateFile( argv[2],
                              GENERIC_WRITE,
                              0,
                              NULL,
                              CREATE_ALWAYS,
                              FILE_ATTRIBUTE_NORMAL,
                              NULL );
    if( hOutputFile == INVALID_HANDLE_VALUE )
    {
        printf( "Can not open %s for writing.\n", argv[2] );
    }

    HANDLE hHeaderFile;
    hHeaderFile = CreateFile( argv[3],
                              GENERIC_WRITE,
                              0,
                              NULL,
                              CREATE_ALWAYS,
                              FILE_ATTRIBUTE_NORMAL,
                              NULL );
    if( hHeaderFile == INVALID_HANDLE_VALUE )
    {
        printf( "Can't open %s for writing.\n", argv[3] );
    }

    if( hDefinitionFile != INVALID_HANDLE_VALUE &&
        hOutputFile     != INVALID_HANDLE_VALUE &&
        hHeaderFile     != INVALID_HANDLE_VALUE )
    {
        ParseSDF( hDefinitionFile, hOutputFile, hHeaderFile );
    }

    CloseHandle( hDefinitionFile );
    CloseHandle( hOutputFile );
    CloseHandle( hHeaderFile );
}