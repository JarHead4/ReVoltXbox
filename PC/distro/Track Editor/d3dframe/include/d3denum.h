//-----------------------------------------------------------------------------
// File: D3DEnum.h
//
// Desc: Functions which enumerate through the DirectDraw drivers, Direct3D
//       devices, and the display modes available to each device.
//
//
// Copyright (C) 1997 Microsoft Corporation. All rights reserved
//-----------------------------------------------------------------------------


#ifndef D3DENUM_H
#define D3DENUM_H

#include <ddraw.h>
#include <d3d.h>




//-----------------------------------------------------------------------------
// Name: ModeInfo
// Desc: Linked-list structure to hold information about a display mode. This
//       info is stored as a width, height, bpp, and pixelformat within a 
//       DDSURFACEDESC2.
//-----------------------------------------------------------------------------
struct ModeInfo
{
    DDSURFACEDESC2 ddsd;
	CHAR           strDesc[40];
    ModeInfo*      pNext;
};




//-----------------------------------------------------------------------------
// Name: DeviceInfo
// Desc: Linked-list structure to hold information about a Direct3D device. The
//       primary information recorded here is the D3DDEVICEDESC and a ptr to a
//       linked-list of valid display modes.
//-----------------------------------------------------------------------------
struct DeviceInfo
{
    GUID            guid;
    GUID*           pGUID;
    CHAR            strName[40];
    D3DDEVICEDESC   ddDesc;
    BOOL            bIsHardware;
	BOOL            bCompatbileWithDesktop;
	BOOL            bWindowed;

    ModeInfo*       pCurrentMode;
    ModeInfo*       pFirstMode;
    DeviceInfo*     pNext;
};




//-----------------------------------------------------------------------------
// Name: DriverInfo
// Desc: Linked-list structure to hold information about a DirectX driver. The
//       info stored is the capability bits for the driver plus a linked-list
//       of valid Direct3D devices for the driver. Note: most systems will only
//       have one driver. The exception are multi-monitor systems, and systems
//       with non-GDI 3D video cards.
//-----------------------------------------------------------------------------
struct DriverInfo
{
    GUID         guid;
    GUID*        pGUID;
    CHAR         strDesc[40];
    CHAR         strName[40];
    DDCAPS       ddDriverCaps;
    DDCAPS       ddHELCaps;
	HANDLE       hMonitor;

    DeviceInfo*  pCurrentDevice;
    DeviceInfo*  pFirstDevice;
    DriverInfo*  pNext;
};




//-----------------------------------------------------------------------------
// Name: D3DEnum_EnumerateDevices()
// Desc: Enumerates all drivers, devices, and modes. The optional app-supplied
//       callback is called for each enumerated device, to confirm that the
//       device supports the feature set required by the app.
//-----------------------------------------------------------------------------
HRESULT D3DEnum_EnumerateDevices( BOOL bEnum8BitModes,
						HRESULT (*AppConfirmFn)(DDCAPS*, D3DDEVICEDESC*) );




//-----------------------------------------------------------------------------
// Name: D3DEnum_FreeResources()
// Desc: Frees all resources used for driver enumeration
//-----------------------------------------------------------------------------
VOID D3DEnum_FreeResources();




//-----------------------------------------------------------------------------
// Name: D3DEnum_SelectDefaultDriver()
// Desc: Picks a driver based on a set of internal criteria.
//-----------------------------------------------------------------------------
HRESULT D3DEnum_SelectDefaultDriver( BOOL bAllowHardware  );




//-----------------------------------------------------------------------------
// Name: D3DEnum_UserDlgSelectDriver()
// Desc: Prompts the user with a dialog box, from which to choose a DD driver,
//       D3D device, and compatible display mode. The function will return 
//       IDOK if a new driver/device/mode was selected, or IDCANCEL if not.
//       Any error will result in a -1 for a return code.
//-----------------------------------------------------------------------------
INT D3DEnum_UserDlgSelectDriver( HWND hwndParent, BOOL bCurrentlyWindowed );




//-----------------------------------------------------------------------------
// Name: D3DEnum_GetSelectedDriver()
// Desc: Returns the currently selected driver, device, and display mode.
//-----------------------------------------------------------------------------
HRESULT D3DEnum_GetSelectedDriver( LPGUID* ppDriverGUID, LPGUID* ppDeviceGuid,
                                   LPDDSURFACEDESC2* pddsdDisplayMode = NULL,
								   BOOL* pbWindowed = NULL );




//-----------------------------------------------------------------------------
// Name: D3DEnum_GetFirstDriver()
// Desc: Returns a ptr to the first DriverInfo structure in the tree holding
//       the device/driver/mode enumeration information.
//-----------------------------------------------------------------------------
DriverInfo* D3DEnum_GetFirstDriver();




#endif // D3DENUM_H
