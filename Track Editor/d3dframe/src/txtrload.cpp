//-----------------------------------------------------------------------
// File: TxtrLoad.cpp
//
// Desc: Support functions for loading textures, used by the texture
//       engine class.
//
//
// Copyright (C) 1997 Microsoft Corporation. All rights reserved
//-----------------------------------------------------------------------

#define STRICT
#include <ddraw.h>
#include <stdio.h>
#include <tchar.h>


//-----------------------------------------------------------------------
// Function prototypes and static variable
//-----------------------------------------------------------------------
inline BOOL FileExists( TCHAR* n ) 
{ FILE* fp=_tfopen(n,TEXT("rb")); return fp?(0==fclose(fp)):FALSE; }

static BOOL LoadPPMHeader( FILE*, DWORD*, DWORD*, DWORD* );

static TCHAR* g_strRegValueName = TEXT("DX6SDK Samples Path");




//-----------------------------------------------------------------------
// Name: FindTextureFile()
// Desc: Looks for the specified file in the current directory, in the path
//       specified in the environment variables, or in the path specified in
//       the registry. After finding it, the function returns the full path.
//-----------------------------------------------------------------------
HRESULT FindTextureFile( TCHAR* strFilename, TCHAR* strTexturePath,
                         TCHAR* strFullPath )
{
    // First, check the current directory
    _tcscpy( strFullPath, strFilename );
    if( FileExists( strFullPath ) )
        return DD_OK;

    // Next, check to see if an environment variable specifies the path
    TCHAR* strPath = _tgetenv( TEXT("D3DPATH") );
    if( NULL != strPath )
    {
        _stprintf( strFullPath, TEXT("%s\\%s"), strPath, strFilename );
        if( FileExists( strFullPath ) )
            return DD_OK;
    }

    // Finally, check the system registry for a path
    HKEY key;
    LONG result = RegOpenKeyEx( HKEY_LOCAL_MACHINE, 
                                TEXT("Software\\Microsoft\\DirectX"), 
                                0, KEY_READ, &key );
    if( ERROR_SUCCESS == result )
    {
        TCHAR  strPath[512];
        DWORD type, size = 512;
        result = RegQueryValueEx( key, g_strRegValueName, NULL, &type, 
                                  (BYTE*)strPath, &size );
                    
        RegCloseKey( key );

        if( ERROR_SUCCESS == result )
        {
            _stprintf( strFullPath, TEXT("%s\\D3DIM\\Media\\%s"), strPath, 
                       strFilename );
            if( FileExists( strFullPath ) )
                return DD_OK;

            _stprintf( strFullPath, TEXT("%s\\%s%s"), strPath, 
                       strTexturePath, strFilename );
            if( FileExists( strFullPath ) )
                return DD_OK;
        }
    }

    return DDERR_NOTFOUND;
}




//-----------------------------------------------------------------------
// Name: PPMGetByte()
// Desc: Function to read in a byte from the texture map .ppm file
//-----------------------------------------------------------------------
BOOL PPMGetByte( FILE* fp, BYTE* newVal )
{
    WORD val;

    if( EOF == (val=getc(fp)) )    // Get a byte, and dump comments
        return FALSE;
    
    if( '#' == val )                     // Skip comments
    {
        while( ('\n'!=val) && ('\r'!=val) )  // Read until just after EOL
        {
            if( EOF == ( val=getc(fp) ) )
                return FALSE;
        }
    }
    *newVal = (BYTE)val;
    return TRUE;
}




//-----------------------------------------------------------------------
// Name: PPMGetByte()
// Desc: Function to read in a byte from the texture map .ppm file
//-----------------------------------------------------------------------
BOOL PPMGetByteResource( BYTE** ppData, BYTE* pVal )
{
    BYTE* pStream = *ppData;
    BYTE  val     = (*pStream++);

    if( EOF == val )    // Get a byte, and dump comments
        return FALSE;
    
    if( '#' == val )                     // Skip comments
    {
        while( ('\n'!=val) && ('\r'!=val) )  // Read until just after EOL
        {
            if( EOF == ( val=(*pStream++) ) )
                return FALSE;
        }
    }
    (*pVal) = val;

    (*ppData) = pStream;
    return TRUE;
}




//-----------------------------------------------------------------------
// Name: PPMGetWord()
// Desc: Function to read in a word from the texture map .ppm file
//-----------------------------------------------------------------------
BOOL PPMGetWord( FILE* fp, DWORD* newVal )
{
    WORD val = 0;
    BYTE ch  = 0;
    
    while( !isdigit(ch) )              // Skip bytes until ASCII 0-9
    {
        if( !PPMGetByte(fp, &ch) )     // Get next byte
            return FALSE;              // File error occured
    }
    

    while( isdigit(ch) )
    {
        val = (val * 10) + (ch - '0'); // Compose digits
        if( !PPMGetByte(fp, &ch) )     // Get next digit
            return FALSE;
    }
    
    *newVal = (DWORD)val;
    return TRUE;
}


    

//-----------------------------------------------------------------------
// Name: PPMGetWord()
// Desc: Function to read in a word from the texture map .ppm file
//-----------------------------------------------------------------------
BOOL PPMGetWordResource( WORD** ppData, WORD* pVal )
{
    WORD val = 0;
    BYTE ch  = 0;
    
    while( !isdigit(ch) )              // Skip bytes until ASCII 0-9
    {
        if( !PPMGetByteResource( (BYTE**)ppData, &ch ) )     // Get next byte
            return FALSE;              // File error occured
    }
    

    while( isdigit(ch) )
    {
        val = (val * 10) + (ch - '0'); // Compose digits
        if( !PPMGetByteResource( (BYTE**)ppData, &ch ) )     // Get next digit
            return FALSE;
    }
    
    (*pVal) = val;
    return TRUE;
}


    

//-----------------------------------------------------------------------
// Name: LoadPPMHeader()
// Desc: Reads in values from the header of a texture map's .ppm file
//-----------------------------------------------------------------------
BOOL LoadPPMHeader( FILE* fp, DWORD* pdwWidth, DWORD* pdwHeight, 
                    DWORD* pdwMaxgrey )
{
    BYTE ch = ' ', magic[2] = "";
    
    fread( magic, 2, 1, fp );       // Read and check the magic bytes
    if( ('P'!=magic[0]) || ('6'!=magic[1]) )
        return FALSE;

    // Read in width, height, and maxgrey
    if( PPMGetWord( fp, pdwWidth ) && PPMGetWord( fp, pdwHeight ) &&
        PPMGetWord( fp, pdwMaxgrey ) )
    {
        // Slurp up rest of white space so we get to actual data
        while( (' '==ch) || ('\t'==ch) || ('\n'==ch) || ('\r'==ch) )
        {
            if( !PPMGetByte( fp, &ch ) )
                return FALSE;
        }   

        // Back up to start of data and return successfully
        fseek( fp, -1, SEEK_CUR );
        return TRUE;
    }

    return FALSE;
}




//-----------------------------------------------------------------------
// Name: LoadPPMHeader()
// Desc: Reads in values from the header of a texture map's .ppm file
//-----------------------------------------------------------------------
BOOL LoadPPMHeaderResource( BYTE** ppData, WORD* pwWidth, WORD* pwHeight, 
                    WORD* pwMaxgrey )
{
    WORD* stream = (WORD*)(*ppData);
    
    BYTE ch = ' ', magic[2] = "";
    
    memcpy( magic, stream, 2 );
    stream++;
    if( ('P'!=magic[0]) || ('6'!=magic[1]) )
        return FALSE;

    // Read in width, height, and maxgrey
    if( PPMGetWordResource( &stream, pwWidth ) &&
        PPMGetWordResource( &stream, pwHeight ) &&
        PPMGetWordResource( &stream, pwMaxgrey ) )
    {
        // Slurp up rest of white space so we get to actual data
        while( (' '==ch) || ('\t'==ch) || ('\n'==ch) || ('\r'==ch) )
        {
            if( !PPMGetByteResource( (BYTE**)&stream, &ch ) )
                return FALSE;
        }   

        // Back up to start of data and return successfully
        (*ppData) = ((BYTE*)stream)-1;
        return TRUE;
    }

    return FALSE;
}




//-----------------------------------------------------------------------
// Name: LoadPPMResource()
// Desc: Given a filename of a PPM file, it loads the image in BITMAP form
//-----------------------------------------------------------------------
HRESULT LoadPPMFileResource( BYTE* pData, HBITMAP* phbm )
{
    // Check params
    if( NULL==pData || NULL==phbm )
        return DDERR_INVALIDPARAMS;

    // Parse the PPM header
    WORD wMaxGrey, wWidth, wHeight;
    if( !LoadPPMHeaderResource( &pData, &wWidth, &wHeight, &wMaxGrey ) )
        return DDERR_GENERIC;

    // Create a bitmap to load the data into
    LONG*      pBits    = NULL;
    HDC        hdcImage = CreateCompatibleDC(NULL);
    BITMAPINFO bmi      = { sizeof(BITMAPINFOHEADER), wWidth, wHeight, 1, 32,
                            BI_RGB, wWidth*wHeight*4, 4000, 4000, 0, 0 };

    if( NULL == ( (*phbm) = CreateDIBSection( hdcImage, &bmi, DIB_RGB_COLORS, 
                                              (VOID**)&pBits, NULL, 0 ) ) )
    {
        DeleteDC( hdcImage );
        return DDERR_GENERIC; 
    }

    // Read in all the pixels for the texture map
    for( WORD y=0; y<wHeight; y++ ) 
    {
        for( WORD x = 0; x<wWidth; x++ ) 
        {
            // Read each pixel value and store it in the bitmap bits
            ((BYTE*)pBits)[3] = 0;
            ((BYTE*)pBits)[2] = *pData++; // red component
            ((BYTE*)pBits)[1] = *pData++; // green component
            ((BYTE*)pBits)[0] = *pData++; // blue component
            pBits++;
        }
    }
    DeleteDC( hdcImage );

    return DD_OK;
}




//-----------------------------------------------------------------------
// Name: LoadPPMFile()
// Desc: Given a filename of a PPM file, it loads the image in BITMAP form
//-----------------------------------------------------------------------
HRESULT LoadPPMFile( TCHAR* strFilename, HBITMAP* phbm )
{
    // Check params
    if( NULL==strFilename || NULL==phbm )
        return DDERR_INVALIDPARAMS;

    // Open the file and read the header
    FILE* file = _tfopen( strFilename, TEXT("rb") );
    if( NULL == file )
        return DDERR_NOTFOUND;

    // Parse the PPM header
    DWORD dwMaxGrey, dwWidth, dwHeight;
    if( !LoadPPMHeader( file, &dwWidth, &dwHeight, &dwMaxGrey ) )
    {
        fclose( file );
        return DDERR_GENERIC;
    }

    // Create a bitmap to load the data into
    LONG*      pBits    = NULL;
    HDC        hdcImage = CreateCompatibleDC(NULL);
    BITMAPINFO bmi      = { sizeof(BITMAPINFOHEADER), dwWidth, dwHeight, 1, 32,
                            BI_RGB, dwWidth*dwHeight*4, 4000, 4000, 0, 0 };

    if( NULL == ( (*phbm) = CreateDIBSection( hdcImage, &bmi, DIB_RGB_COLORS, 
                                              (VOID**)&pBits, NULL, 0 ) ) )
    {
        DeleteDC( hdcImage );
        fclose( file );
        return DDERR_GENERIC; 
    }

    // Read in all the pixels for the texture map
    for( DWORD y=0; y<dwHeight; y++ ) 
    {
        for( DWORD x = 0; x<dwWidth; x++ ) 
        {
            // Read each pixel value and store it in the bitmap bits
            ((BYTE*)pBits)[3] = 0;
            ((BYTE*)pBits)[2] = getc( file ); // red component
            ((BYTE*)pBits)[1] = getc( file ); // green component
            ((BYTE*)pBits)[0] = getc( file ); // blue component
            pBits++;
        }
    }
    fclose( file );
    DeleteDC( hdcImage );

    return DD_OK;
}




//-----------------------------------------------------------------------
// Name: LoadBMPFile()
// Desc: Used as a callback to load textures, this function reads in a BMP
//       file and creates and returns a BITMAP handle for it.
//-----------------------------------------------------------------------
HRESULT LoadBMPFile( TCHAR* strFilename, HBITMAP* phbm )
{ 
    // Check params
    if( NULL==strFilename || NULL==phbm )
        return DDERR_INVALIDPARAMS;

    // Try to load the bitmap as a resource.
    (*phbm) = (HBITMAP)LoadImage( GetModuleHandle(NULL), strFilename, 
                              IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION ); 

    // If the bitmap wasn't a resource, try it as a file.
    if( NULL == (*phbm) ) 
        (*phbm) = (HBITMAP)LoadImage( NULL, strFilename, IMAGE_BITMAP, 0, 0, 
                                  LR_LOADFROMFILE|LR_CREATEDIBSECTION ); 

    return (*phbm) ? DD_OK : DDERR_NOTFOUND;
}




