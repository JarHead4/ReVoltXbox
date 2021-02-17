/*
**----------------------------------------------------------------------------
**
**  File:       d3dscene.cpp
**  Purpose:    
**  Notes:
**
**	Copyright (C) 1995 - 1997 Microsoft Corporation. All Rights Reserved.
**----------------------------------------------------------------------------
*/

/*
**----------------------------------------------------------------------------
** Includes
**----------------------------------------------------------------------------
*/

#include <math.h>
#include <time.h>

#include "D3DScene.h"
#include "Debug.h"
#include "D3DWin.h"
#include "d3dutils.h"
#include "d3dtex.h"

/*
**----------------------------------------------------------------------------
** Defines
**----------------------------------------------------------------------------
*/

/*
**----------------------------------------------------------------------------
** D3DScene Methods
**----------------------------------------------------------------------------
*/

/*
**----------------------------------------------------------------------------
** Name:        D3DScene::D3DScene
** Purpose:		Default Constructor
**----------------------------------------------------------------------------
*/

D3DScene::D3DScene(void)
{
	lpd3dWindow = NULL;
} // End D3DScene::D3DScene


  
/*
**----------------------------------------------------------------------------
** Name:        D3DScene::~D3DScene
** Purpose:		Default Destructor
**----------------------------------------------------------------------------
*/

D3DScene::~D3DScene(void)
{
	Fini();
	lpd3dWindow = NULL;
} // End D3DScene::~D3DScene



/*
**----------------------------------------------------------------------------
** Name:        D3DScene::Init
** Purpose:		Do all static intialization here
** Notes:		This means all Scene data that isn't dependent on the
**				D3D interface, viewport, or D3D device in some manner
**----------------------------------------------------------------------------
*/

HRESULT
D3DScene::Init(LPD3DWindow lpd3dWin)
{
	HRESULT hResult;

	// Check Parameters
	if (! lpd3dWin)	{
		// Need a valid D3D Window object
		hResult = APPERR_INVALIDPARAMS;
		REPORTERR(hResult);
		return hResult;
	}

	// Save assocation with D3D Window
	lpd3dWindow = lpd3dWin;

	// Success
    return D3D_OK;
} // End D3DScene::Init
  
/*
**-----------------------------------------------------------------------------
**  Name:       D3DScene::Fini
**  Purpose:	Cleanup scene objects
**-----------------------------------------------------------------------------
*/

HRESULT D3DScene::Fini(void)
{
	Detach();
	lpd3dWindow = NULL;

	// Success
	return D3D_OK;
} // End D3DScene::Fini



/*
**----------------------------------------------------------------------------
** Name:        D3DScene::Attach
** Purpose:		Attaching to a new D3DWindow object
** Notes:		Need to create and attach all Scene objects dependent upon
**				the D3D interface, viewport, and D3D device here.  
**				For Example:  Textures, Materials, Lights, etc.
**----------------------------------------------------------------------------
*/

HRESULT D3DScene::Attach(void)
{
	HRESULT hResult;

	// Check Initialization
	if ((! lpd3dWindow) || (! lpd3dWindow->isValid()))	{
		// Error, not properly initialized
		hResult = APPERR_NOTINITIALIZED;
		REPORTERR(hResult);
		return hResult;
	}

	hResult = AttachViewport();
	if (hResult != D3D_OK) {
		return hResult;
	}

	// Success
	return D3D_OK;
} // End D3DScene::Attach

  

/*
**-----------------------------------------------------------------------------
**  Name:       D3DScene::Detach
**  Purpose:	Cleanup all scene objects dependent upon the 
**				D3D Interface, viewport, or D3D device 
**-----------------------------------------------------------------------------
*/

HRESULT D3DScene::Detach(void)
{
	// Cleanup Viewport
	DetachViewport();

	// Success
	return D3D_OK;
} // End D3DScene::Fini




/*
**----------------------------------------------------------------------------
** Name:        D3DScene::Render
** Purpose:
**----------------------------------------------------------------------------
*/

HRESULT D3DScene::Render(void)
{
	// Nothing to do for now

	// Success
	return D3D_OK;
} // End D3DScene::Render


  
/*
**----------------------------------------------------------------------------
** Name:        D3DScene::Restore
** Purpose:     Restore any scene specific surfaces that might have been
**				lost on a DDERR_LOSTSURFACE message
**----------------------------------------------------------------------------
*/

HRESULT D3DScene::Restore(void)
{
	// Nothing to do for now

	// Success
	return D3D_OK;
} // End D3DScene::Restore


  
/*
**----------------------------------------------------------------------------
** Name:        D3DScene::AttachViewport
** Purpose:
**----------------------------------------------------------------------------
*/

HRESULT
D3DScene::AttachViewport(void)
{
	// Nothing to do for now

	// Success
	return D3D_OK;
} // End D3DScene::AttachViewport

	

/*
**-----------------------------------------------------------------------------
**  Name:       D3DScene::DetachViewport
**  Purpose:	Cleanup Viewport
**-----------------------------------------------------------------------------
*/

HRESULT D3DScene::DetachViewport(void)
{
	// Nothing to do for now

	// Success
	return D3D_OK;
} // End D3DScene::DetachViewport


void D3DScene::OnKeyStroke(int vKeyCode)
{
}
 
/*
**----------------------------------------------------------------------------
** End of File
**----------------------------------------------------------------------------
*/

