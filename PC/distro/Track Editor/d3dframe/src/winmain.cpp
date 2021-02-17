//-----------------------------------------------------------------------------
// File: WinMain.cpp
//
// Desc: Windows code for Direct3D samples
//
//       This code uses the CD3DFramework, CD3DTextureEngine, and
//       CD3DEnumerator helper classes.
//
//
// Copyright (c) 1997 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------

#define STRICT
#include <windows.h>
#include <time.h>
#include <stdio.h>
#include "D3DFrame.h"
#include "D3DEnum.h"
#include "D3DUtil.h"
#include "resource.h"


//-----------------------------------------------------------------------------
// Global variables for using the D3D sample framework class
//-----------------------------------------------------------------------------
CD3DFramework* g_pFramework        = NULL;
BOOL           g_bActive           = FALSE;
BOOL           g_bReady            = FALSE;
BOOL           g_bExitting         = FALSE;
BOOL           g_bAppUseFullScreen = FALSE;
BOOL           g_bShowStats        = TRUE;
RECT           g_rcWindow;
extern BOOL    g_bWindowed;
extern BOOL    g_bAppUseZBuffer;
extern BOOL    g_bAppUseBackBuffer;
extern TCHAR*  g_strAppTitle;



//-----------------------------------------------------------------------------
// Local function-prototypes
//-----------------------------------------------------------------------------
HRESULT Initialize3DEnvironment( HWND );
HRESULT Change3DEnvironment( HWND, BOOL );
HRESULT Render3DEnvironment();
VOID    Cleanup3DEnvironment();
VOID    AppOutputText( LPDIRECT3DDEVICE3, DWORD, DWORD, CHAR* );
VOID    AppPause( BOOL );

//-----------------------------------------------------------------------------
// External function-prototypes
//-----------------------------------------------------------------------------
HRESULT App_ConfirmDevice( DDCAPS*, D3DDEVICEDESC* );
HRESULT App_OneTimeSceneInit( HWND, LPDIRECT3DDEVICE3, LPDIRECT3DVIEWPORT3 );
VOID    App_DeleteDeviceObjects( LPDIRECT3DDEVICE3, LPDIRECT3DVIEWPORT3 );
HRESULT App_InitDeviceObjects( LPDIRECT3DDEVICE3, LPDIRECT3DVIEWPORT3 );
HRESULT App_FrameMove( LPDIRECT3DDEVICE3, FLOAT );
HRESULT App_Render( LPDIRECT3DDEVICE3, LPDIRECT3DVIEWPORT3, D3DRECT* );
HRESULT App_RestoreSurfaces();
HRESULT App_FinalCleanup( LPDIRECT3DDEVICE3, LPDIRECT3DVIEWPORT3 );
VOID    App_InitOptions();
LRESULT App_HandleKeystroke(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

//-----------------------------------------------------------------------------
// Name: AboutProc()
// Desc: Minimal message proc function for the about box
//-----------------------------------------------------------------------------
BOOL FAR PASCAL AboutProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM )
{
    if( WM_COMMAND == uMsg )
		if( IDOK == LOWORD(wParam) || IDCANCEL == LOWORD(wParam) )
			EndDialog (hWnd, TRUE);

    return ( WM_INITDIALOG == uMsg ) ? TRUE : FALSE;
}

//-----------------------------------------------------------------------------
// Name: WndProc()
// Desc: This is the basic Windows-programming function that processes
//       Windows messages. We need to handle window movement, painting,
//       and destruction.
//-----------------------------------------------------------------------------
LRESULT CALLBACK WndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    switch( msg )
    {
        case WM_ACTIVATEAPP:
            g_bActive = (BOOL)wParam;
            break;

        case WM_PAINT:
            if( g_pFramework && g_bReady )
                g_pFramework->ShowFrame();
            break;

        case WM_MOVE:
            if( g_bActive && g_bReady && g_bWindowed )
			{
			    GetWindowRect( hWnd, &g_rcWindow );
                g_pFramework->Move( (SHORT)LOWORD(lParam), (SHORT)HIWORD(lParam) );
			}
            break;

        case WM_SIZE:
            // Check to see if we are losing our window...
            if( SIZE_MAXHIDE==wParam || SIZE_MINIMIZED==wParam )
                g_bActive = FALSE;

            // A new window size will require a new viewport and backbuffer
            // size, so the 3D structures must be changed accordingly.
            if( g_bActive && g_bReady && g_bWindowed )
			{
				g_bReady = FALSE;
				GetWindowRect( hWnd, &g_rcWindow );
				Change3DEnvironment( hWnd, FALSE );
				g_bReady = TRUE;
			}
            break;

		case WM_GETMINMAXINFO:
			((MINMAXINFO*)lParam)->ptMinTrackSize.x = 100;
			((MINMAXINFO*)lParam)->ptMinTrackSize.y = 100;
			break;

        case WM_SETCURSOR:
            if( g_bActive && g_bReady && (!g_bWindowed) )
            {
                SetCursor(NULL);
                return TRUE;
            }
            break;

        case WM_CLOSE:
            g_bExitting = TRUE;
            DestroyWindow( hWnd );
            return 0;
        
        case WM_DESTROY:
            Cleanup3DEnvironment();
            PostQuitMessage(0);
            return 0L;

        case WM_COMMAND:
            switch( LOWORD(wParam) )
            {
                case IDM_CHANGEDEVICE:
                    // Display the driver-selection dialog box.
		            if( g_bActive && g_bReady )
					{
						AppPause(TRUE);
						if( IDOK == D3DEnum_UserDlgSelectDriver( hWnd, g_bWindowed ) )
							Change3DEnvironment( hWnd, TRUE );
						AppPause(FALSE);
					}
                    return 0;

                case IDM_TOGGLEFULLSCREEN:
                    // Toggle the fullscreen/window mode
		            if( g_bActive && g_bReady )
					{
						g_bReady = FALSE;
			            g_bWindowed = !g_bWindowed;
						Change3DEnvironment( hWnd, FALSE );
						g_bReady = TRUE;
					}
					return 0;

                case IDM_HELP:
					AppPause(TRUE);
                    DialogBox( (HINSTANCE)GetWindowLong( hWnd, GWL_HINSTANCE ),
                               MAKEINTRESOURCE(IDD_ABOUT), hWnd, AboutProc );
					AppPause(FALSE);
                    return 0;

                case IDM_EXIT:
                    // Recieved key/menu command to exit app
                    SendMessage( hWnd, WM_CLOSE, 0, 0 );
                    return 0;
            }
            break;

			case WM_KEYDOWN:
			case WM_KEYUP:
				return App_HandleKeystroke(hWnd, msg, wParam, lParam);
			break;
    }

    return DefWindowProc( hWnd, msg, wParam, lParam );
}
            
//-----------------------------------------------------------------------------
// Name: InitializeWindowEnvironment()
// Desc: This function initializes our standard Windows' stuff
//-----------------------------------------------------------------------------
HWND InitializeWindowEnvironment( HINSTANCE hInst )
{
    WNDCLASS wndClass = { CS_HREDRAW | CS_VREDRAW, WndProc, 0, 0, hInst,
              LoadIcon( hInst, MAKEINTRESOURCE(IDI_MAIN_ICON)),
              LoadCursor(NULL, IDC_ARROW), 
              (HBRUSH)GetStockObject(WHITE_BRUSH), NULL,
              TEXT("Render Window") };
    RegisterClass( &wndClass );

    HWND hWnd = CreateWindow( TEXT("Render Window"), g_strAppTitle,
                  WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,
                  CW_USEDEFAULT, 300, 300, 0L, 0L, hInst, 0L );

    ShowWindow( hWnd, SW_SHOWNORMAL );
    UpdateWindow( hWnd );

    return hWnd;
}

//-----------------------------------------------------------------------------
// Name: WinMain()
// Desc: Entry point to the program. Initializes everything, and goes
//       into a message-processing loop. 3D-rendering occurs during the
//       idle phase of the loop.
//-----------------------------------------------------------------------------
int PASCAL WinMain( HINSTANCE hInst, HINSTANCE, LPSTR, int )
{
    // Create our main window
    HWND hWnd = InitializeWindowEnvironment( hInst );

    // Load keyboard accelerators
    HACCEL hAccel = LoadAccelerators( hInst, MAKEINTRESOURCE(IDR_MAIN_ACCEL) );

    if( FAILED( Initialize3DEnvironment( hWnd ) ) )
    {
	    Cleanup3DEnvironment();
        return 0;
    }

    // Now we're ready to recieve and process Windows messages.
    while( TRUE )
    {
        MSG msg;
        if( PeekMessage( &msg, NULL, 0U, 0U, PM_REMOVE ) )
        {
            if( WM_QUIT == msg.message )
            return msg.wParam;

            // Check for keyboard accelerators
            if( hAccel )
                if( 0 != TranslateAccelerator( hWnd, hAccel, &msg ) )
                    continue;

            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }
        else
        {   
            // Idle processing goes here. This is where we do our rendering.
            // Alternatively, we may use a timer on a separate thread
            if( g_bActive && g_bReady )
                Render3DEnvironment();
        }
    }
    return 0;
}


//-----------------------------------------------------------------------------
// Note: From this point on, the code is DirectX specific support for the app.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Name: Initialize3DEnvironment()
// Desc: Called when the app window is initially created, this triggers
//       creation of the remaining portion (the 3D stuff) of the app.
//-----------------------------------------------------------------------------
HRESULT Initialize3DEnvironment( HWND hWnd )
{
    GUID*           pDriverGUID;
    GUID*           pDeviceGUID;
    DDSURFACEDESC2* pMode;

	App_InitOptions();
	
	// Make sure the display mode bit-depth can support the samples.
	if( D3DUtil_GetDisplayDepth() < 16 )
	{
        MessageBox( hWnd, TEXT("Direct3D samples require 16-bit or higher "
			        "display modes.\n\nPlease switch your desktop settings "
					"accordingly."), g_strAppTitle, MB_ICONERROR|MB_OK );
        return E_FAIL;
	}

	// Enumerate available devices. The App_ConfirmDevice() function
	// is a callback that allows devices to be accepted/rejected
	// based on what capabilities the app requires.
	if( FAILED( D3DEnum_EnumerateDevices( FALSE, App_ConfirmDevice ) ) )
    {
        MessageBox( hWnd, TEXT("Could not find any compatible Direct3D\n"
			        "devices.\n\nTry enabling the reference rasterizer by\n"
					"executing EnableRefRast.reg."),
                    g_strAppTitle, MB_ICONERROR|MB_OK );
        return E_FAIL;
    }
    D3DEnum_GetSelectedDriver( &pDriverGUID, &pDeviceGUID, &pMode );

    // Create a new CD3DFramework class. This class does all of our D3D
    // initialization and manages the common D3D objects.
    if( NULL == ( g_pFramework = new CD3DFramework() ) )
        return E_FAIL;

    DWORD dwFrameworkFlags = 0L;
    dwFrameworkFlags |= ( !g_bWindowed        ? D3DFW_FULLSCREEN : 0L );
    dwFrameworkFlags |= ( g_bAppUseZBuffer    ? D3DFW_ZBUFFER    : 0L );
    dwFrameworkFlags |= ( g_bAppUseBackBuffer ? D3DFW_BACKBUFFER : 0L );

	// Initialize the D3D framework
    if( SUCCEEDED( g_pFramework->Initialize( hWnd, pDriverGUID, pDeviceGUID,
                pMode, dwFrameworkFlags ) ) )
    {
		// Let the app run its startup code which creates the 3d scene.
		if( SUCCEEDED( App_OneTimeSceneInit( hWnd,
			                                 g_pFramework->GetD3DDevice(),
                                             g_pFramework->GetViewport() ) ) )
		{
		    GetWindowRect( hWnd, &g_rcWindow ); // Save the window for later...
			g_bReady = TRUE;
			return S_OK;
		}
	}

	// Else, an error occurred, display a message and exit
    MessageBox( hWnd, TEXT("Initialization failed\n\nThis app will now exit."),
		        g_strAppTitle, MB_ICONERROR|MB_OK );
    return E_FAIL;
}

    


//-----------------------------------------------------------------------------
// Name: Change3DEnvironment()
// Desc: Handles driver, device, and/or mode changes for the app.
//-----------------------------------------------------------------------------
HRESULT Change3DEnvironment( HWND hWnd, BOOL bFromUserDlgSelection )
{
    GUID*           pDriverGUID;
    GUID*           pDeviceGUID;
    DDSURFACEDESC2* pMode;
	BOOL            bWindowed;
    
    if( NULL == g_pFramework )
        return E_FAIL;

    // Release all objects that need to be re-created for the new device
    App_DeleteDeviceObjects( g_pFramework->GetD3DDevice(), 
                             g_pFramework->GetViewport() );
	
	if( FAILED( g_pFramework->DestroyObjects() ) )
	{
		MessageBox( hWnd, TEXT("DirectDraw object still referenced.\n\n"
			        "Sample app did not properly cleanup objects."),
                    g_strAppTitle, MB_ICONERROR|MB_OK );
        DestroyWindow( hWnd );
		return E_FAIL;
	}

	// Get the newly selected driver, device, and display mode
    if( FAILED( D3DEnum_GetSelectedDriver( &pDriverGUID, &pDeviceGUID,
                                           &pMode, &bWindowed ) ) )
        return E_FAIL;

	// Match the bWindowed flag with the value selected in the user dialog
	if( bFromUserDlgSelection )
		g_bWindowed = bWindowed;

	// If windowed, we need to resize the window first
	if( g_bWindowed )
	{
		SetWindowPos( hWnd, HWND_NOTOPMOST, g_rcWindow.left, g_rcWindow.top,
                      ( g_rcWindow.right - g_rcWindow.left ), 
                      ( g_rcWindow.bottom - g_rcWindow.top ), SWP_SHOWWINDOW );
	}
	else
		GetWindowRect( hWnd, &g_rcWindow );

    // Inform the framework class of the driver change. It will internally
    // re-create valid surfaces, a d3ddevice, and a viewport.
    DWORD dwFrameworkFlags = 0L;
    dwFrameworkFlags |= ( !g_bWindowed         ? D3DFW_FULLSCREEN : 0L );
    dwFrameworkFlags |= (  g_bAppUseZBuffer    ? D3DFW_ZBUFFER    : 0L );
    dwFrameworkFlags |= (  g_bAppUseBackBuffer ? D3DFW_BACKBUFFER : 0L );

    if( FAILED( g_pFramework->Initialize( hWnd, pDriverGUID,
		                           pDeviceGUID, pMode, dwFrameworkFlags ) ) )
    {
		// If that failed, let's switch to a software rasterizer
		MessageBox( hWnd, TEXT("Device could not be created. Possible causes "
			        "are incompatibility\nwith display mode or running out of "
					"video memory.\n\nSwitching to software rasterizer."),
                    g_strAppTitle, MB_ICONWARNING|MB_OK );

		// Pick a non-hardware rasterizer
		D3DEnum_SelectDefaultDriver( FALSE );
		D3DEnum_GetSelectedDriver( &pDriverGUID, &pDeviceGUID );

		// If we are coming from fullscreen, restore the window size
		if( FALSE == g_bWindowed )
		{
			g_bWindowed = TRUE;
			SetWindowPos( hWnd, HWND_NOTOPMOST, g_rcWindow.left, g_rcWindow.top,
				          ( g_rcWindow.right - g_rcWindow.left ), 
					      ( g_rcWindow.bottom - g_rcWindow.top ), SWP_SHOWWINDOW );
		}

		// Try the Initialize() function again. Failure this time is fatal.
		dwFrameworkFlags  = ( g_bAppUseZBuffer    ? D3DFW_ZBUFFER    : 0L );
		dwFrameworkFlags |= ( g_bAppUseBackBuffer ? D3DFW_BACKBUFFER : 0L );
	    if( FAILED( g_pFramework->Initialize( hWnd, pDriverGUID,
			                      pDeviceGUID, pMode, dwFrameworkFlags ) ) )
        {
			MessageBox( hWnd, TEXT("Could not create software rasterizer\n\n"
				        "This app will now exit."), 
						g_strAppTitle, MB_ICONWARNING|MB_OK );
            DestroyWindow( hWnd );
			return E_FAIL;
		}
	}

    // Let the app know the device changed so it can rebuild objects.
    HRESULT hr = App_InitDeviceObjects( g_pFramework->GetD3DDevice(),
                                        g_pFramework->GetViewport() );
	return hr;
}




//-----------------------------------------------------------------------------
// Name: Render3DEnvironment()
// Desc: Draws the scene
//-----------------------------------------------------------------------------
HRESULT Render3DEnvironment()
{
    // FrameMove (animate) the scene
    FLOAT fTime = ((FLOAT)clock())/CLOCKS_PER_SEC;
    App_FrameMove( g_pFramework->GetD3DDevice(), fTime );

    //Render the scene
    App_Render( g_pFramework->GetD3DDevice(), g_pFramework->GetViewport(), 
                (D3DRECT*)g_pFramework->GetViewportRect() );
    
    // Calculate and display frame rate
    if( g_bShowStats )
    {
        static FLOAT fFPS      = 0.0f;
        static FLOAT fLastTime = 0.0f;
        static DWORD dwFrames  = 0L;

        if( 100 == ++dwFrames )
        {
            fFPS      = dwFrames / (fTime - fLastTime);
            fLastTime = fTime;
            dwFrames  = 0;
        }

        // Get dimensions of the render surface 
        DDSURFACEDESC2 ddsd;
        ddsd.dwSize = sizeof(DDSURFACEDESC2);
        g_pFramework->GetRenderSurface()->GetSurfaceDesc(&ddsd);

        // Setup the text buffer to write out
        CHAR buffer[80];
        sprintf( buffer, "%7.02f fps (%dx%dx%d)", fFPS, ddsd.dwWidth,
                 ddsd.dwHeight, ddsd.ddpfPixelFormat.dwRGBBitCount );
        AppOutputText( g_pFramework->GetD3DDevice(), 0, 0, buffer );
    }

    // Show the frame on the primary surface.
    if( DDERR_SURFACELOST == g_pFramework->ShowFrame() )
    {
		g_pFramework->RestoreSurfaces();
        App_RestoreSurfaces();
	}

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: Cleanup3DEnvironment()
// Desc: Cleanup scene objects
//-----------------------------------------------------------------------------
VOID Cleanup3DEnvironment()
{
    if( g_pFramework )
    {
        App_FinalCleanup( g_pFramework->GetD3DDevice(), 
                          g_pFramework->GetViewport() );

        SAFE_DELETE( g_pFramework );
    }
}


  

//-----------------------------------------------------------------------------
// Name: AppPause()
// Desc: Called in to toggle the pause state of the app. This function
//       brings the GDI surface to the front of the display, so drawing
//       output like message boxes and menus may be displayed.
//-----------------------------------------------------------------------------
VOID AppPause( BOOL bPause )
{
    static DWORD dwAppPausedCount = 0L;

    if( bPause && 0 == dwAppPausedCount )
        if( g_pFramework )
            g_pFramework->FlipToGDISurface( TRUE );

    dwAppPausedCount += ( bPause ? +1 : -1 );

    g_bReady = (0==dwAppPausedCount);
}




//-----------------------------------------------------------------------------
// Name: AppOutputText()
// Desc: Draws text on the window.
//-----------------------------------------------------------------------------
VOID AppOutputText( LPDIRECT3DDEVICE3 pd3dDevice, DWORD x, DWORD y, CHAR* str )
{
    LPDIRECTDRAWSURFACE4 pddsRenderSurface;
    if( FAILED( pd3dDevice->GetRenderTarget( &pddsRenderSurface ) ) )
        return;

    // Get a DC for the surface. Then, write out the buffer
    HDC hDC;
    if( SUCCEEDED( pddsRenderSurface->GetDC(&hDC) ) )
    {
        SetTextColor( hDC, RGB(255,255,0) );
        SetBkMode( hDC, TRANSPARENT );
		RECT rect;
		rect.left = x;
		rect.top = y;
		rect.right = 1024;
		rect.bottom = 768;
		DrawText( hDC, str, strlen(str), &rect, DT_LEFT | DT_NOCLIP | DT_NOPREFIX | DT_WORDBREAK);
        pddsRenderSurface->ReleaseDC(hDC);
    }
    pddsRenderSurface->Release();
}




