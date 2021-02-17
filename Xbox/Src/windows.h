//-----------------------------------------------------------------------------
// File: Windows.h
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
/* $HACK: for Xbox, replace windows.h with xtl.h, and add some compatibility stuff */

#ifndef __WINDOWS_H__
#define __WINDOWS_H__

#ifdef _XBOX
  #include <xtl.h>

  #define IDirect3DTexture2  IDirect3DTexture8
  #define IDirectDrawSurface4  IDirect3DSurface8
  #define IDirectDrawPalette   IDirect3DPalette8
  #define DDPIXELFORMAT        D3DFORMAT
  #define DDSURFACEDESC2       D3DSURFACE_DESC
  #define IDirect3DDevice3     IDirect3DDevice8
  #define IDirect3D3           IDirect3D8
  #define DDGAMMARAMP          D3DGAMMARAMP
//$REMOVED   #define IDirect3DViewport3   D3DVIEWPORT8
  #define D3DVIEWPORT2         D3DVIEWPORT8

  #define XBOX_UNUSED_PARAM    NULL
  

    /*
     * MessageBox() Flags
     */
    #define MB_OK                       0x00000000L
    #define MB_OKCANCEL                 0x00000001L
    #define MB_ABORTRETRYIGNORE         0x00000002L
    #define MB_YESNOCANCEL              0x00000003L
    #define MB_YESNO                    0x00000004L
    #define MB_RETRYCANCEL              0x00000005L
    
    #define MB_ICONHAND                 0x00000010L
    #define MB_ICONQUESTION             0x00000020L
    #define MB_ICONEXCLAMATION          0x00000030L
    #define MB_ICONASTERISK             0x00000040L
    
    #if(WINVER >= 0x0400)
    #define MB_USERICON                 0x00000080L
    #define MB_ICONWARNING              MB_ICONEXCLAMATION
    #define MB_ICONERROR                MB_ICONHAND
    #endif /* WINVER >= 0x0400 */
    
    #define MB_ICONINFORMATION          MB_ICONASTERISK
    #define MB_ICONSTOP                 MB_ICONHAND
    
    #define MB_DEFBUTTON1               0x00000000L
    #define MB_DEFBUTTON2               0x00000100L
    #define MB_DEFBUTTON3               0x00000200L
    #if(WINVER >= 0x0400)
    #define MB_DEFBUTTON4               0x00000300L
    #endif /* WINVER >= 0x0400 */
    

    /*
     * Dialog Box Command IDs
     */
    #define IDOK                1
    #define IDCANCEL            2
    #define IDABORT             3
    #define IDRETRY             4
    #define IDIGNORE            5
    #define IDYES               6
    #define IDNO                7
    #if(WINVER >= 0x0400)
    #define IDCLOSE         8
    #define IDHELP          9
    #endif /* WINVER >= 0x0400 */

    //
    // Misc DPlay porting crap
    //
    typedef  DWORD  DPID;



#endif // _XBOX

#endif // __WINDOWS_H__