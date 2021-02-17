//-----------------------------------------------------------------------------
// File: D3DEnum.cpp
//
// Desc: Class enumerate through the DirectDraw drivers, Direct3D devices,
//       and the display modes available to each device.
//
//
// Copyright (C) 1997 Microsoft Corporation. All rights reserved
//-----------------------------------------------------------------------------

#define STRICT
#include <stdio.h>

// HACK!!!!!
#include <windows.h>
#ifndef SM_CMONITORS
#define SM_CMONITORS
DECLARE_HANDLE(HMONITOR);
#endif

#include "D3DEnum.h"
#include "D3DUtil.h"




//-----------------------------------------------------------------------------
// Constants and function prototypes for the user select driver dialog
//-----------------------------------------------------------------------------
DLGTEMPLATE*  BuildDriverSelectTemplate();
BOOL CALLBACK DriverSelectProc( HWND, UINT, WPARAM, LPARAM );




//-----------------------------------------------------------------------------
// Global data for the enumerator functions
//-----------------------------------------------------------------------------
LPDIRECTDRAW4  g_pDD                = NULL;  // Used for callbacks
DWORD          g_dwDisplayDepth     = 0L;    // Used for matching drivers
BOOL           g_bEnum8BitModes     = TRUE;  // Used during enumeration

DriverInfo*    g_pFirstDriver       = NULL;  // List of enumerated DD drivers
DriverInfo*    g_pDefaultDriver     = NULL;  // Default driver if none chosen
DriverInfo*    g_pCurrentDriver     = NULL;  // The selected DD driver

HRESULT (*g_fnAppConfirmFn)(DDCAPS*, D3DDEVICEDESC*) = NULL;




//-----------------------------------------------------------------------------
// Local callback functions used during enumeration
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Name: EnumDisplayModesCallback()
// Desc: Callback function called for each display mode. Each available
//       display mode is added to a list for further choosing from the app.
//-----------------------------------------------------------------------------
static HRESULT WINAPI EnumDisplayModesCallback( DDSURFACEDESC2* pddsd,
                                                VOID* pvContext )
{
    // Check parameters
    if( NULL==pddsd || NULL==pvContext )
        return DDENUMRET_CANCEL;

    DeviceInfo* pDevice     = (DeviceInfo*)pvContext;
    DWORD       dwBitDepth  = pDevice->ddDesc.dwDeviceRenderBitDepth;
    DWORD       dwModeDepth = pddsd->ddpfPixelFormat.dwRGBBitCount;
    ModeInfo*   pNewMode;

	// Check mode for compatability with device
    if( ( 8==dwModeDepth) && (0==(dwBitDepth&DDBD_8 )) ) return DDENUMRET_OK;
    if( (16==dwModeDepth) && (0==(dwBitDepth&DDBD_16)) ) return DDENUMRET_OK;
    if( (24==dwModeDepth) && (0==(dwBitDepth&DDBD_24)) ) return DDENUMRET_OK;
    if( (32==dwModeDepth) && (0==(dwBitDepth&DDBD_32)) ) return DDENUMRET_OK;

	// Skip 8-bit modes, unless requested by app
	if( ( 8==dwModeDepth) && (FALSE==g_bEnum8BitModes) ) return DDENUMRET_OK;

	if( dwModeDepth == g_dwDisplayDepth )
		pDevice->bCompatbileWithDesktop = TRUE;

    // Create a new mode structure
    if( NULL == ( pNewMode = new ModeInfo ) )
        return DDENUMRET_CANCEL;

    // Initialize the new mode structure
    ZeroMemory( pNewMode, sizeof(ModeInfo) );
    memcpy( &pNewMode->ddsd, pddsd, sizeof(DDSURFACEDESC2) );
    sprintf( pNewMode->strDesc, TEXT("%ld x %ld x %ld"), pddsd->dwWidth,
                                pddsd->dwHeight, dwModeDepth );

    // Link the new mode struct in the list of other display modes
	ModeInfo** pMode = &pDevice->pFirstMode;
	while( *pMode )
		pMode = &((*pMode)->pNext);
	(*pMode) = pNewMode;
	
    // If this is a 640x480x16 mode, save it as the default mode
    if( ( 800 == pddsd->dwWidth ) && ( 600 == pddsd->dwHeight ) &&
        ( 16 == pddsd->ddpfPixelFormat.dwRGBBitCount ) )
        pDevice->pCurrentMode = pNewMode;
	
	if( NULL == pDevice->pCurrentMode )
        pDevice->pCurrentMode = pNewMode;

    return DDENUMRET_OK;
}




//-----------------------------------------------------------------------------
// Name: Enum3DDevicesCallback()
// Desc: Callback function called for each DirectX 3D device. The driver's
//       attributes are added to the list of available drivers.
//-----------------------------------------------------------------------------
static HRESULT WINAPI Enum3DDevicesCallback( GUID* pGUID, LPSTR strDesc, 
                                LPSTR strName, LPD3DDEVICEDESC pHALDesc, 
                                LPD3DDEVICEDESC pHELDesc, LPVOID pvContext )
{
    DriverInfo* pDriver = (DriverInfo*)pvContext;
    DeviceInfo* pNewDevice;

    // Check params
    if( NULL==pGUID || NULL==pHALDesc || NULL==pHELDesc || NULL==pDriver )
        return D3DENUMRET_CANCEL;

    // Handle specific device GUIDs. NullDevice renders nothing
    if( IsEqualGUID( *pGUID, IID_IDirect3DNullDevice ) )
        return D3DENUMRET_OK;

	// Skip any non-HW devices, if we have a HW secondary-driver
    BOOL bIsHardware = ( 0 != pHALDesc->dwFlags );
	if( ( FALSE == bIsHardware ) && ( pDriver->pGUID != NULL ) )
		return D3DENUMRET_OK;

	D3DDEVICEDESC* pDesc = bIsHardware ? pHALDesc : pHELDesc;

	// Give the app a chance to accept or reject this device, based on
	// what feature set it supports
	if( g_fnAppConfirmFn )
		if( FAILED( g_fnAppConfirmFn( &pDriver->ddDriverCaps, pDesc ) ) )
			return D3DENUMRET_OK;

    // Create a new D3D Driver struct
    if( NULL == ( pNewDevice = new DeviceInfo ) )
        return D3DENUMRET_CANCEL;
    ZeroMemory( pNewDevice, sizeof(DeviceInfo) );

    // Copy remaining device attributes
    memcpy( &pNewDevice->guid, pGUID, sizeof(GUID) );
    pNewDevice->pGUID = &pNewDevice->guid;
    strncpy( pNewDevice->strName, strName, 39 );
    memcpy( &pNewDevice->ddDesc, pDesc, sizeof(D3DDEVICEDESC) );
    pNewDevice->bIsHardware = bIsHardware;

    if( pNewDevice->bIsHardware )
        pDriver->pCurrentDevice = pNewDevice;
    else
	{
		if( NULL == pDriver->pCurrentDevice )
			if( D3DCOLOR_RGB & pHELDesc->dcmColorModel )
				pDriver->pCurrentDevice = pNewDevice;
	}

    // Enumerate the display modes
    g_pDD->EnumDisplayModes( 0, NULL, pNewDevice, EnumDisplayModesCallback );

	if( pDriver->ddDriverCaps.dwCaps2 & DDCAPS2_CANRENDERWINDOWED )
		if( pNewDevice->bCompatbileWithDesktop )
			pNewDevice->bWindowed = TRUE;

	if( pNewDevice->pFirstMode )
    {
		// Link it with the other D3D drivers in the DD Driver struct
		DeviceInfo** pDevice = &pDriver->pFirstDevice;
		while( *pDevice )
			pDevice = &((*pDevice)->pNext);
		(*pDevice) = pNewDevice;
	}
	else
		// Device has no modes, so delete it
		delete pNewDevice;


    return D3DENUMRET_OK;
}




//-----------------------------------------------------------------------------
// Name: DirectDrawEnumCallbackEx()
// Desc: Callback function called for each DirectDraw driver. Unless we have
//       multimon or a type of card which uses a separate 2D card in
//       conjunction with the 3D card, this is only done once.
//-----------------------------------------------------------------------------
static BOOL WINAPI DirectDrawEnumCallbackEx( GUID FAR* pGUID, LPSTR strDesc,
											 LPSTR strName, VOID*,
											 HMONITOR hMonitor )
{
    // Use the GUID to create the DirectDraw object, so that information
    // can be extracted from it.
	LPDIRECTDRAW pDD;
    if( FAILED( DirectDrawCreate( pGUID, &pDD, 0L ) ) )
    {
		DEBUG_MSG( TEXT("Can't create DDraw during enumeration!") );
        return D3DENUMRET_OK;
	}

    // Query the DirectDraw driver for access to Direct3D.
    if( FAILED( pDD->QueryInterface( IID_IDirectDraw4, (VOID**)&g_pDD ) ) )
	{
        DEBUG_MSG( TEXT("Can't query IDirectDraw4 during enumeration!") );
		pDD->Release();
		return D3DENUMRET_OK;
	}
	pDD->Release();

    // Query the DirectDraw driver for access to Direct3D.
    LPDIRECT3D3 pD3D;
    if( FAILED( g_pDD->QueryInterface( IID_IDirect3D3, (VOID**)&pD3D ) ) )
	{
        DEBUG_MSG( TEXT("Can't query IDirect3D3 during enumeration!") );
		g_pDD->Release();
		return D3DENUMRET_OK;
	}

    // Copy the DDDriver info into a new DriverInfo struct
    DriverInfo* pNewDriver = new DriverInfo;
    if( NULL == pNewDriver )
		return D3DENUMRET_CANCEL;

    ZeroMemory( pNewDriver, sizeof(DriverInfo) );

    // Copy the GUID (if specified) and the driver name
    if( NULL != pGUID  )
    {
        memcpy( &pNewDriver->guid, pGUID, sizeof(GUID) );
        pNewDriver->pGUID = &pNewDriver->guid;
    }
    strncpy( pNewDriver->strDesc, strDesc, 39 );
    strncpy( pNewDriver->strName, strName, 39 );
	pNewDriver->hMonitor = hMonitor;

    // Get the caps bits for the driver
    pNewDriver->ddDriverCaps.dwSize = sizeof(DDCAPS);
    pNewDriver->ddHELCaps.dwSize    = sizeof(DDCAPS);
    g_pDD->GetCaps( &pNewDriver->ddDriverCaps, &pNewDriver->ddHELCaps );

	// Now, enumerate all the 3D devices
    pD3D->EnumDevices( Enum3DDevicesCallback, pNewDriver );

	if( pNewDriver->pFirstDevice )
	{
	    // Link the new DDDriver with the global list
		DriverInfo** pDriver = &g_pFirstDriver;
		while( *pDriver )
			pDriver = &((*pDriver)->pNext);
		(*pDriver) = pNewDriver;

		// Decide if this is a good default driver
		if( NULL == pGUID )
			g_pCurrentDriver = pNewDriver;
	}
	else
		// Driver has no devices, so delete it
		delete pNewDriver;

    pD3D->Release();
    g_pDD->Release();
    return DDENUMRET_OK;
}




//-----------------------------------------------------------------------------
// Name: DirectDrawEnumCallback()
// Desc: Non-mulitmon version of the ddraw enumeration callback
//-----------------------------------------------------------------------------
static BOOL WINAPI DirectDrawEnumCallback( GUID FAR* pGUID, LPSTR strDesc,
                                           LPSTR strName, VOID* )
{
	return DirectDrawEnumCallbackEx( pGUID, strDesc, strName, NULL, NULL );
}




//-----------------------------------------------------------------------------
// Name: D3DEnum_FreeResources()
// Desc: Frees all resources used for driver enumeration
//-----------------------------------------------------------------------------
VOID D3DEnum_FreeResources()
{
    // Loop through each driver, and delete everything
    while( g_pFirstDriver )
    {
        DriverInfo* pDriverVictim = g_pFirstDriver;
        g_pFirstDriver            = g_pFirstDriver->pNext;

        while( pDriverVictim->pFirstDevice )
        {
            DeviceInfo* pDeviceVictim   = pDriverVictim->pFirstDevice;
            pDriverVictim->pFirstDevice = pDeviceVictim->pNext;
   
            while( pDeviceVictim->pFirstMode )
            {
                ModeInfo* pModeVictim     = pDeviceVictim->pFirstMode;
                pDeviceVictim->pFirstMode = pModeVictim->pNext;
                delete pModeVictim;
            }
            delete pDeviceVictim;
        }
        delete pDriverVictim;
    }
}




//-----------------------------------------------------------------------------
// Name: D3DEnum_EnumerateDevices()
// Desc: Enumerates all drivers, devices, and modes. The optional app-supplied
//       callback is called for each enumerated device, to confirm that the
//       device supports the feature set required by the app.
//-----------------------------------------------------------------------------
HRESULT D3DEnum_EnumerateDevices( BOOL bEnum8BitModes,
						HRESULT (*AppConfirmFn)(DDCAPS*, D3DDEVICEDESC*) )
{
	g_bEnum8BitModes     = bEnum8BitModes;
	g_fnAppConfirmFn     = AppConfirmFn;

	// We need to manually get the procedure address for the DDrawEnumEx()
	// function.
	HMODULE hDDrawDLL = GetModuleHandle("DDRAW.DLL");
	if( NULL == hDDrawDLL )
	{
		DEBUG_MSG( TEXT("Can't load DDRAW.DLL!") );
		return E_FAIL;
	}

	// Get the display mode depth, used for comparison during enumeration
	if( 0L == ( g_dwDisplayDepth = D3DUtil_GetDisplayDepth() ) )
		return E_FAIL;

	// Find the DDraw enumeration function, and call it
	LPDIRECTDRAWENUMERATEEX pDDrawEnumFn = (LPDIRECTDRAWENUMERATEEX)
		             GetProcAddress( hDDrawDLL, "DirectDrawEnumerateExA" );

	if( pDDrawEnumFn )
		pDDrawEnumFn( DirectDrawEnumCallbackEx, NULL,
					  DDENUM_ATTACHEDSECONDARYDEVICES |
					  DDENUM_DETACHEDSECONDARYDEVICES |
					  DDENUM_NONDISPLAYDEVICES );
	else
        DirectDrawEnumerate( DirectDrawEnumCallback, NULL );

	// Select a driver. Ask for a hardware device that renders in a window
	return D3DEnum_SelectDefaultDriver( TRUE );
}




//-----------------------------------------------------------------------------
// Name: D3DEnum_SelectDefaultDriver()
// Desc: Picks a default driver that can render at least in a window
//-----------------------------------------------------------------------------
HRESULT D3DEnum_SelectDefaultDriver( BOOL bAllowHardware )
{
	// Do two passes, looping through drivers, devices and modes. The 1st pass
	// searches for hardware. The 2nd pass looks for software devices.
	for( WORD pass=0; pass<2; pass++ )
	{
		// Skip the hardware pass, if we aren't allowing hardware
		if( (0==pass) && (FALSE==bAllowHardware) )
			continue;

		BOOL bSeekHardware = (0==pass) ? TRUE : FALSE;

		for( DriverInfo* pDriver = g_pFirstDriver; pDriver;
		     pDriver = pDriver->pNext )
		{
			DDCAPS* pCaps = &pDriver->ddDriverCaps;

			if( 0 == ( pCaps->dwCaps2 & DDCAPS2_CANRENDERWINDOWED ) )
				continue;

			for( DeviceInfo* pDevice = pDriver->pFirstDevice; pDevice;
			     pDevice = pDevice->pNext )
			{
				if( FALSE == pDevice->bCompatbileWithDesktop )
					continue;
				if( bSeekHardware != pDevice->bIsHardware )
					continue;

				pDevice->bWindowed = TRUE;
				g_pCurrentDriver   = pDriver;
				g_pCurrentDriver->pCurrentDevice = pDevice;

				return S_OK;
			}
		}
	}

	return E_FAIL;
}




//-----------------------------------------------------------------------------
// Name: D3DEnum_UserDlgSelectDriver()
// Desc: Displays a dialog box for the user to select a driver/device/mode.
//       The return values are akin to the Windows DialogBox() function.
//-----------------------------------------------------------------------------
INT D3DEnum_UserDlgSelectDriver( HWND hwndParent, BOOL bCurrentlyWindowed )
{
	INT nResult = -1;

    // Check in case drivers weren't properly enumerated beforehand.
    if( NULL == g_pCurrentDriver )
        return -1;

	// MAtch the current windowed-vs-fullscreen state
	g_pCurrentDriver->pCurrentDevice->bWindowed = bCurrentlyWindowed;

    // Pop up a dialog box for the user's choice of driver/device/mode
    HINSTANCE hInstance = (HINSTANCE)GetWindowLong( hwndParent, 
                                                    GWL_HINSTANCE );

    // Create dynamic dialog template
    DLGTEMPLATE* pDlgSelect = BuildDriverSelectTemplate();
    if( pDlgSelect )
	{
	    // Create dialog box from template
		nResult = DialogBoxIndirectParam( hInstance, pDlgSelect, hwndParent,
                                          (DLGPROC)DriverSelectProc, 0L );
	    delete pDlgSelect;
	}

    return nResult;
}




//-----------------------------------------------------------------------------
// Name: D3DEnum_GetSelectedDriver()
// Desc: Returns the currently selected driver, device, and display mode
//-----------------------------------------------------------------------------
HRESULT D3DEnum_GetSelectedDriver( LPGUID* ppDriverGUID, LPGUID* ppDeviceGUID, 
                                   LPDDSURFACEDESC2* ppddsd, BOOL* pbWindowed )
{
    // Check parans
    if( (!ppDriverGUID) || (!ppDeviceGUID) )
        return E_INVALIDARG;

    // Abort if things weren't setup correctly
    if( NULL == g_pCurrentDriver )
        return E_FAIL;

    // Copy the driver and device GUID ptrs
    (*ppDriverGUID) = g_pCurrentDriver->pGUID;
    (*ppDeviceGUID) = g_pCurrentDriver->pCurrentDevice->pGUID;
	
	if( ppddsd )
		(*ppddsd) = &g_pCurrentDriver->pCurrentDevice->pCurrentMode->ddsd;
	if( pbWindowed )
		(*pbWindowed) = g_pCurrentDriver->pCurrentDevice->bWindowed;

	return DD_OK;
}




//-----------------------------------------------------------------------------
// Name: D3DEnum_GetFirstDriver()
// Desc: Returns a ptr to the first DriverInfo structure in the list.
//-----------------------------------------------------------------------------
DriverInfo* D3DEnum_GetFirstDriver()
{
    return g_pFirstDriver;
}




