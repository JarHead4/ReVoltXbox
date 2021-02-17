/*
**----------------------------------------------------------------------------
**
**  File:       TrackEdit.cpp
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


#include "TrackEdit.h"

/*
**----------------------------------------------------------------------------
** Defines
**----------------------------------------------------------------------------
*/

#define NUM_BOIDS	13

// ground pattern
D3DVECTOR   grid_color(0.0f, 0.3f, 0.5f);
D3DLVERTEX	pattern1[24];
D3DLVERTEX	pattern2[8];
WORD		pat1_indices[25] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 
								13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 0 };
WORD		pat2_indices[9] = { 0, 1, 2, 3, 4, 5, 6, 7, 0 };

D3DVECTOR   box_color(0.7f, 0.2f, 0.0f);
D3DLVERTEX	boxverts[8];
WORD		boxlines[24] = { 0, 1, 1, 2, 2, 3, 3, 0,
							 4, 5, 5, 6, 6, 7, 7, 4,
							 0, 4, 1, 5, 2, 6, 3, 7
						   };

/*
**----------------------------------------------------------------------------
** Local Variables
**----------------------------------------------------------------------------
*/


D3DMATRIX	proj, view, world;

Light		*lpLight1,
			*lpLight2;

Material	*lpBackgroundMat,
			*lpGridMat,
			*lpUnitMat[4];

D3DTexture	UnitTex[3];

/*
**----------------------------------------------------------------------------
** Function definitions
**----------------------------------------------------------------------------
*/


D3DScene* CreateScene(void)
{
	TrackEdit* scene = new TrackEdit;
	return (D3DScene*) scene;
}

HRESULT
DrawPattern(LPDIRECT3DDEVICE2 lpDev)
{
	HRESULT hResult;

	for (int dx= -2; dx<3; dx++) {
		for (int dz= -2; dz<3; dz++) {
			D3DVECTOR	offset(dx*80.0f, 0.0f, dz*80.0f);

			world = TranslateMatrix(offset);
			hResult = lpDev->SetTransform(D3DTRANSFORMSTATE_WORLD, &world);
			if (hResult != D3D_OK) {
				REPORTERR(hResult);
				return hResult;
			}
            hResult = lpDev->DrawIndexedPrimitive(D3DPT_LINESTRIP, D3DVT_LVERTEX, (LPVOID)pattern1, 24, pat1_indices, 25, D3DDP_WAIT);
			if (hResult != D3D_OK) {
				REPORTERR(hResult);
				return hResult;
			}
            hResult = lpDev->DrawIndexedPrimitive(D3DPT_LINESTRIP, D3DVT_LVERTEX, (LPVOID)pattern2, 8, pat2_indices, 9, D3DDP_WAIT);
			if (hResult != D3D_OK) {
				REPORTERR(hResult);
				return hResult;
			}
		}
	}

	return D3D_OK;
}	// end of DrawPattern()



TrackEdit::TrackEdit() : D3DScene() , TrackGrid(0), Edit_Status(CHOOSING)
{
	CurrentMat = 0;
}

HRESULT TrackEdit::DrawComponent(LPDIRECT3DDEVICE2 lpDev, const COMPONENT* component)
{
	HRESULT hResult;
	U32 p = component->PrimitiveCount;
	while(p--)
	{
		PRIMITIVE* prim = component->Primitives[p];
		if((prim->VertexCount != 0) && (prim->Vertices != NULL) )
		{
			lpUnitMat[p]->SetAsCurrent(lpDev);
			hResult = lpDev->DrawPrimitive(D3DPT_TRIANGLELIST,D3DVT_VERTEX,(LPVOID)prim->Vertices, prim->VertexCount,NULL);
		}
	}

	return hResult;
}

HRESULT TrackEdit::DrawUnits(LPDIRECT3DDEVICE2 lpDev)
{
	HRESULT hResult;

	D3DMATRIX ObjectMatrix;

	U32 x, y, width, height;
	float xoffset;
	float yoffset;
	static float spin = 0.0f;
	COMPONENT* component;

	switch(Edit_Status)
	{
		case CHOOSING:
			width = (U32)sqrt(MeshCount);
			height = width;
			if((width * height) < MeshCount)
			{
				width++;
			}
			xoffset = (width-1) * 5;
			yoffset = (height-1) * 5;
			xoffset /= 2;
			yoffset /= 2;

			for(y = 0; y < height; y++)
			{
				for(x = 0; x < width; x++)
				{
					U32 m = (y * width) + x;
					if(m < MeshCount)
					{
						component = DXUnits[m]->Components[PAN_INDEX];	//get pan component
						ObjectMatrix = RotateXMatrix((float)PI);
						ObjectMatrix = MatrixMult(TranslateMatrix((x * 5) - xoffset, 0, (y * 5) - yoffset), ObjectMatrix);
						hResult = lpDev->SetTransform(D3DTRANSFORMSTATE_WORLD, &ObjectMatrix);

						DrawComponent(lpDev, component);
					}
				}
			}

			ObjectMatrix = RotateYMatrix(spin);
			ObjectMatrix = MatrixMult(ScaleMatrix(1, 1, -1), ObjectMatrix);
			ObjectMatrix = MatrixMult(RotateXMatrix((float)PI * 1.75f), ObjectMatrix);
			ObjectMatrix = MatrixMult(TranslateMatrix(0.0f, 30.0f, 0.0f), ObjectMatrix);
			hResult = lpDev->SetTransform(D3DTRANSFORMSTATE_WORLD, &ObjectMatrix);
			spin += 0.04f;
			if(spin > (PI * 2))
			{
				spin = 0.0f;
			}
			
			component = DXUnits[UnitCursor.X + (UnitCursor.Y * UnitCursor.XMax)]->Components[PAN_INDEX];	//get pan component
			DrawComponent(lpDev, component);

			hResult = lpDev->SetRenderState(D3DRENDERSTATE_TEXTUREHANDLE, 0);
			ObjectMatrix = RotateXMatrix((float)PI);
			ObjectMatrix = MatrixMult(TranslateMatrix((UnitCursor.X * 5) - xoffset, 0, (UnitCursor.Y * 5) - yoffset), ObjectMatrix);
			hResult = lpDev->SetTransform(D3DTRANSFORMSTATE_WORLD, &ObjectMatrix);
			hResult = lpDev->DrawIndexedPrimitive(D3DPT_LINELIST, D3DVT_LVERTEX, (LPVOID)boxverts, 8, boxlines, 24, NULL);
		break;
		case PLACING:
			xoffset = (TrackWidth - 1) * 4;
			yoffset = (TrackHeight - 1) * 4;
			xoffset /= 2;
			yoffset /= 2;

			for(y = 0; y < TrackHeight; y++)
			{
				for(x = 0; x < TrackWidth; x++)
				{
					U32 g = (y * TrackWidth) + x;
					U32 m = TrackGrid[g].UnitID;

					component = DXUnits[m]->Components[PAN_INDEX];	//get pan component
					ObjectMatrix = RotateXMatrix((float)PI);
					ObjectMatrix = MatrixMult(TranslateMatrix((x * 4) - xoffset, 0, (y * 4) - yoffset), ObjectMatrix);
					hResult = lpDev->SetTransform(D3DTRANSFORMSTATE_WORLD, &ObjectMatrix);

					DrawComponent(lpDev, component);
				}
			}
			hResult = lpDev->SetRenderState(D3DRENDERSTATE_TEXTUREHANDLE, 0);
			ObjectMatrix = RotateXMatrix((float)PI);
			ObjectMatrix = MatrixMult(TranslateMatrix((TrackCursor.X * 4) - xoffset, 0, (TrackCursor.Y * 4) - yoffset), ObjectMatrix);
			hResult = lpDev->SetTransform(D3DTRANSFORMSTATE_WORLD, &ObjectMatrix);
			hResult = lpDev->DrawIndexedPrimitive(D3DPT_LINELIST, D3DVT_LVERTEX, (LPVOID)boxverts, 8, boxlines, 24, NULL);
		break;
	}

	return D3D_OK;
}	// end of DrawPattern()


/*
**----------------------------------------------------------------------------
** Name:        TrackEdit::Init
** Purpose:		Do all static intialization here
** Notes:		This means all Scene data that isn't dependent on the
**				D3D interface, viewport, or D3D device in some manner
**----------------------------------------------------------------------------
*/

HRESULT
TrackEdit::Init(LPD3DWindow lpd3dWin)
{
	HRESULT result = D3DScene::Init(lpd3dWin);
	if(FAILED(result))
	{
		return result;
	}
	
	result = ParseCommandline();
	if(FAILED(result))
	{
		return result;
	}

	// seed the random number generator
	srand(time(NULL));


	ReadRTUFile(RTUFileName);

	D3DCOLOR	diffuse = D3DRGB(grid_color[0], grid_color[1], grid_color[2]),
				specular = D3DRGB(0.0, 0.0, 0.0);

	pattern1[ 0] = D3DLVERTEX(D3DVECTOR(-25.0f, 0.0f, 35.0f), diffuse, specular, 0.0f, 0.0f);
	pattern1[ 1] = D3DLVERTEX(D3DVECTOR(-15.0f, 0.0f, 35.0f), diffuse, specular, 0.0f, 0.0f);
	pattern1[ 2] = D3DLVERTEX(D3DVECTOR(-5.0f, 0.0f, 25.0f), diffuse, specular, 0.0f, 0.0f);
	pattern1[ 3] = D3DLVERTEX(D3DVECTOR(5.0f, 0.0f, 25.0f), diffuse, specular, 0.0f, 0.0f);
	pattern1[ 4] = D3DLVERTEX(D3DVECTOR(15.0f, 0.0f, 35.0f), diffuse, specular, 0.0f, 0.0f);
	pattern1[ 5] = D3DLVERTEX(D3DVECTOR(25.0f, 0.0f, 35.0f), diffuse, specular, 0.0f, 0.0f);
	pattern1[ 6] = D3DLVERTEX(D3DVECTOR(35.0f, 0.0f, 25.0f), diffuse, specular, 0.0f, 0.0f);
	pattern1[ 7] = D3DLVERTEX(D3DVECTOR(35.0f, 0.0f, 15.0f), diffuse, specular, 0.0f, 0.0f);
	pattern1[ 8] = D3DLVERTEX(D3DVECTOR(25.0f, 0.0f, 5.0f), diffuse, specular, 0.0f, 0.0f);
	pattern1[ 9] = D3DLVERTEX(D3DVECTOR(25.0f, 0.0f, -5.0f), diffuse, specular, 0.0f, 0.0f);
	pattern1[10] = D3DLVERTEX(D3DVECTOR(35.0f, 0.0f, -15.0f), diffuse, specular, 0.0f, 0.0f);
	pattern1[11] = D3DLVERTEX(D3DVECTOR(35.0f, 0.0f, -25.0f), diffuse, specular, 0.0f, 0.0f);
	pattern1[12] = D3DLVERTEX(D3DVECTOR(25.0f, 0.0f, -35.0f), diffuse, specular, 0.0f, 0.0f);
	pattern1[13] = D3DLVERTEX(D3DVECTOR(15.0f, 0.0f,-35.0f), diffuse, specular, 0.0f, 0.0f);
	pattern1[14] = D3DLVERTEX(D3DVECTOR(5.0f, 0.0f, -25.0f), diffuse, specular, 0.0f, 0.0f);
	pattern1[15] = D3DLVERTEX(D3DVECTOR(-5.0f, 0.0f, -25.0f), diffuse, specular, 0.0f, 0.0f);
	pattern1[16] = D3DLVERTEX(D3DVECTOR(-15.0f, 0.0f,-35.0f), diffuse, specular, 0.0f, 0.0f);
	pattern1[17] = D3DLVERTEX(D3DVECTOR(-25.0f, 0.0f,-35.0f), diffuse, specular, 0.0f, 0.0f);
	pattern1[18] = D3DLVERTEX(D3DVECTOR(-35.0f, 0.0f, -25.0f), diffuse, specular, 0.0f, 0.0f);
	pattern1[19] = D3DLVERTEX(D3DVECTOR(-35.0f, 0.0f, -15.0f), diffuse, specular, 0.0f, 0.0f);
	pattern1[20] = D3DLVERTEX(D3DVECTOR(-25.0f, 0.0f, -5.0f), diffuse, specular, 0.0f, 0.0f);
	pattern1[21] = D3DLVERTEX(D3DVECTOR(-25.0f, 0.0f, 5.0f), diffuse, specular, 0.0f, 0.0f);
	pattern1[22] = D3DLVERTEX(D3DVECTOR(-35.0f, 0.0f, 15.0f), diffuse, specular, 0.0f, 0.0f);
	pattern1[23] = D3DLVERTEX(D3DVECTOR(-35.0f, 0.0f, 25.0f), diffuse, specular, 0.0f, 0.0f);

	pattern2[ 0] = D3DLVERTEX(D3DVECTOR(-5.0f, 0.0f, 15.0f), diffuse, specular, 0.0f, 0.0f);
	pattern2[ 1] = D3DLVERTEX(D3DVECTOR(5.0f, 0.0f, 15.0f), diffuse, specular, 0.0f, 0.0f);
	pattern2[ 2] = D3DLVERTEX(D3DVECTOR(15.0f, 0.0f, 5.0f), diffuse, specular, 0.0f, 0.0f);
	pattern2[ 3] = D3DLVERTEX(D3DVECTOR(15.0f, 0.0f, -5.0f), diffuse, specular, 0.0f, 0.0f);
	pattern2[ 4] = D3DLVERTEX(D3DVECTOR(5.0f, 0.0f, -15.0f), diffuse, specular, 0.0f, 0.0f);
	pattern2[ 5] = D3DLVERTEX(D3DVECTOR(-5.0f, 0.0f, -15.0f), diffuse, specular, 0.0f, 0.0f);
	pattern2[ 6] = D3DLVERTEX(D3DVECTOR(-15.0f, 0.0f, -5.0f), diffuse, specular, 0.0f, 0.0f);
	pattern2[ 7] = D3DLVERTEX(D3DVECTOR(-15.0f, 0.0f, 5.0f), diffuse, specular, 0.0f, 0.0f);

	diffuse = D3DRGB(box_color[0], box_color[1], box_color[2]);
	specular = D3DRGB(0.0, 0.0, 0.0);

	boxverts[0] =  D3DLVERTEX(D3DVECTOR(-2.5f, 2.5f, 2.5f), diffuse, specular, 0.0f, 0.0f);
	boxverts[1] =  D3DLVERTEX(D3DVECTOR(2.5f, 2.5f, 2.5f), diffuse, specular, 0.0f, 0.0f);
	boxverts[2] =  D3DLVERTEX(D3DVECTOR(2.5f, 2.5f, -2.5f), diffuse, specular, 0.0f, 0.0f);
	boxverts[3] =  D3DLVERTEX(D3DVECTOR(-2.5f, 2.5f, -2.5f), diffuse, specular, 0.0f, 0.0f);
	boxverts[4] =  D3DLVERTEX(D3DVECTOR(-2.5f, -2.5f, 2.5f), diffuse, specular, 0.0f, 0.0f);
	boxverts[5] =  D3DLVERTEX(D3DVECTOR(2.5f, -2.5f, 2.5f), diffuse, specular, 0.0f, 0.0f);
	boxverts[6] =  D3DLVERTEX(D3DVECTOR(2.5f, -2.5f, -2.5f), diffuse, specular, 0.0f, 0.0f);
	boxverts[7] =  D3DLVERTEX(D3DVECTOR(-2.5f, -2.5f, -2.5f), diffuse, specular, 0.0f, 0.0f);

	UnitCursor.X = 0;
	UnitCursor.Y = 0;
	U32 root = (U32)sqrt(MeshCount);
	UnitCursor.XMax = UnitCursor.YMax = root;

	if((root * root) < MeshCount)
	{
		UnitCursor.XMax++;
	}
	UnitCursor.AbsMax = MeshCount;

	Verts = NULL;
	Polys = NULL;
	Meshes = NULL;
	UVCoords = NULL;

	CreateTrack(TRACK_WIDTH, TRACK_HEIGHT);
	// Success
    return D3D_OK;
} // End TrackEdit::Init


/*
**----------------------------------------------------------------------------
** Name:        TrackEdit::Fini
** Purpose:
**----------------------------------------------------------------------------
*/

HRESULT TrackEdit::Fini(void)
{
	EraseGeometry();
	DestroyTrack();
	return D3DScene::Fini();
} // End TrackEdit::Fini


/*
**----------------------------------------------------------------------------
** Name:        TrackEdit::Render
** Purpose:
**----------------------------------------------------------------------------
*/

HRESULT TrackEdit::Render(void)
{
	LPDIRECT3DDEVICE2	lpDev;
	LPDIRECT3DVIEWPORT2 lpView;
	RECT				rSrc;
	LPD3DRECT			lpExtent = NULL;
	HRESULT				hResult;
	D3DCLIPSTATUS		status;
	D3DRECT				d3dRect;
	D3DVECTOR			offset;
	static D3DVECTOR	from(0.0f, 60.0f, 0.0f);
	static D3DVECTOR	at(0.0f, 0.0f, 0.0f);
	static D3DVECTOR	up(0.0f, 0.0f, 1.0f);
	static float		tic = -200.0f * rnd();

	// Check Initialization
	if ((! lpd3dWindow) || (! lpd3dWindow->isValid ()))	{
		// Error, not properly initialized
		hResult = APPERR_NOTINITIALIZED;
		REPORTERR(hResult);
		return hResult;
	}

	lpDev	= lpd3dWindow->GetD3DDevice();
	lpView  = lpd3dWindow->GetViewport();
	lpd3dWindow->GetSurfaceRect(rSrc);

	// Double Check
#ifdef DEBUG
	if ((! lpDev) || (! lpView))
	{
		// Error, not initialized properly
		hResult = APPERR_NOTINITIALIZED;
		REPORTERR(hResult);
		return hResult;
	}
#endif
	
	
	//
    // Clear both back and z-buffer.
    //
    // NOTE: Its safe to specify the z-buffer clear flag even if we
    // don't have an attached z-buffer. Direct3D will simply discard
    // the flag if no z-buffer is being used.
    //
    // NOTE: For maximum efficiency we only want to clear those
    // regions of the device surface and z-buffer which we actually
    // rendered to in the last frame. This is the purpose of the
    // array of rectangles and count passed to this function. It is
    // possible to query Direct3D for the regions of the device
    // surface that were rendered to by that execute. The application
    // can then accumulate those rectangles and clear only those
    // regions. However this is a very simple sample and so, for
    // simplicity, we will just clear the entire device surface and
    // z-buffer. Probably not something you wan't to do in a real
    // application.
    ///
	d3dRect.lX1 = rSrc.left;
	d3dRect.lX2 = rSrc.right;
	d3dRect.lY1 = rSrc.top;
	d3dRect.lY2 = rSrc.bottom;
	hResult = lpView->Clear(1UL, &d3dRect,
							 D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER);
	if (hResult != D3D_OK) {
		REPORTERR(hResult);
		return hResult;
	}	

	
	if (lpExtent) {
		// Calculate exclude region
		status.dwFlags = D3DCLIPSTATUS_EXTENTS2;
		status.dwStatus = 0;
		status.minx = (float)lpExtent->x1;
		status.maxx = (float)lpExtent->x2;
		status.miny = (float)lpExtent->y1;
		status.maxy = (float)lpExtent->y2;
		status.minz = 0.0f;
		status.maxz = 0.0f;

		hResult = lpDev->SetClipStatus(&status);
		if (hResult != D3D_OK) {
			REPORTERR(hResult);
			goto lblCLEANUP;
		}
	}

	// Begin Scene
	// Note:  This is complicated by the need to
	//		  check for lost surfaces and restore
	hResult = lpDev->BeginScene();
	if (hResult != D3D_OK) {
		while (hResult == DDERR_SURFACELOST) {
			// Restore surface
			while (hResult == DDERR_SURFACELOST) {
				hResult = lpd3dWindow->Restore();
			}

			// Try BeginScene again
			hResult = lpDev->BeginScene();
		}

		if (hResult != D3D_OK) {
			REPORTERR(hResult);
			return hResult;
		}
	}

    // Turn off specular highlights
        hResult = lpDev->SetRenderState(D3DRENDERSTATE_SPECULARENABLE, FALSE);
	if (hResult != D3D_OK) {
        REPORTERR(hResult);
		goto lblCLEANUP;
	}

    // Turn on Z-buffering
	hResult = lpDev->SetRenderState(D3DRENDERSTATE_ZENABLE, 1);
	if (hResult != D3D_OK) {
        REPORTERR(hResult);
		goto lblCLEANUP;
	}

	// null out the texture handle
    hResult = lpDev->SetRenderState(D3DRENDERSTATE_TEXTUREHANDLE, 0);
    if (hResult != D3D_OK) {
        REPORTERR(hResult);
		goto lblCLEANUP;
	}

	// turn on dithering
    hResult = lpDev->SetRenderState(D3DRENDERSTATE_DITHERENABLE, TRUE);
    if (hResult != D3D_OK) {
        REPORTERR(hResult);
		goto lblCLEANUP;
	}

	// turn on some ambient light
    hResult = lpDev->SetLightState(D3DLIGHTSTATE_AMBIENT, RGBA_MAKE(10, 10, 10, 10));
    if (hResult != D3D_OK) {
        REPORTERR(hResult);
		goto lblCLEANUP;
	}

	// set the view and projection matrices

	tic += 0.01f;


	view = ViewMatrix(from, at, up);
	proj = ProjectionMatrix(1.0f, 500.0f, pi/4.0f);

	hResult = lpDev->SetTransform(D3DTRANSFORMSTATE_VIEW, &view);
    if (hResult != D3D_OK) {
        REPORTERR(hResult);
		goto lblCLEANUP;
	}

	hResult = lpDev->SetTransform(D3DTRANSFORMSTATE_PROJECTION, &proj);
    if (hResult != D3D_OK) {
        REPORTERR(hResult);
		goto lblCLEANUP;
	}

	// draw ground grid
	lpGridMat->SetAsCurrent(lpDev);
	hResult = DrawPattern(lpDev);
    if (hResult != D3D_OK) {
        REPORTERR(hResult);
		goto lblCLEANUP;
	}

	// draw ground grid
	hResult = DrawUnits(lpDev);
    if (hResult != D3D_OK) {
        REPORTERR(hResult);
		goto lblCLEANUP;
	}


	// set renderstates back to default
    if (lpDev->SetRenderState(D3DRENDERSTATE_BLENDENABLE, FALSE) != D3D_OK)
        return FALSE;
    if (lpDev->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE) != D3D_OK)
        return FALSE;
    if (lpDev->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ZERO) != D3D_OK)
		return FALSE;
    if (lpDev->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE) != D3D_OK)
		return FALSE;
    if (lpDev->SetRenderState(D3DRENDERSTATE_CULLMODE,D3DCULL_CW) != D3D_OK)
		return FALSE;


lblCLEANUP: 
	// End Scene
	// Note:  This is complicated by the need to restore lost surfaces
    hResult = lpDev->EndScene();
	if (hResult != D3D_OK) {
		while (hResult == DDERR_SURFACELOST) {
			// Restore surface
			while (hResult == DDERR_SURFACELOST) {
				hResult = lpd3dWindow->Restore();
			}

			// Try EndScene again
			hResult = lpDev->EndScene();
		}

		if (hResult != D3D_OK) {
			REPORTERR(hResult);
			return hResult;
		}
	}

	if (lpExtent) {
		hResult = lpDev->GetClipStatus(&status);
		if (hResult != D3D_OK) {
			REPORTERR(hResult);
			return hResult;
		}

		if (status.dwFlags & D3DCLIPSTATUS_EXTENTS2) {
			lpExtent->x1 = (long) floor((double)status.minx);
			lpExtent->x2 = (long) ceil((double)status.maxx);
			lpExtent->y1 = (long) floor((double)status.miny);
			lpExtent->y2 = (long) ceil((double)status.maxy);
		}
	}
		
	return hResult;
} // End TrackEdit::Render


  
/*
**----------------------------------------------------------------------------
** Name:        TrackEdit::AttachViewport
** Purpose:
**----------------------------------------------------------------------------
*/

HRESULT
TrackEdit::AttachViewport(void)
{
	LPDIRECT3D2				lpD3D;
	LPDIRECT3DDEVICE2		lpDev;
	LPDIRECT3DVIEWPORT2   	lpView;

	// Check Initialization
	if ((! lpd3dWindow) || (! lpd3dWindow->isValid())) {
		// Error,
		REPORTERR(DDERR_GENERIC);
		return DDERR_GENERIC;
	}

	lpD3D  = lpd3dWindow->GetD3D();
	lpDev  = lpd3dWindow->GetD3DDevice();
	lpView = lpd3dWindow->GetViewport();

	if ((! lpD3D) || (! lpDev) || (! lpView)) {
		REPORTERR(DDERR_GENERIC);
		return DDERR_GENERIC;
	}

	// Create and set up the background material
	lpBackgroundMat = new Material(lpD3D, lpDev);
	lpBackgroundMat->SetDiffuse(D3DVECTOR(0.3f, 0.6f, 0.1f));
	lpBackgroundMat->SetAsBackground(lpView);

	// Create and set up the grid material, since we're using LVertices we want
	// to make sure that ramp mode can actually get the right colors so we set
	// the emissive value to the color we want and choose a small ramp size
	lpGridMat = new Material(lpD3D, lpDev);
	lpGridMat->SetEmissive(grid_color);
	lpGridMat->SetRampSize(2);

	lpUnitMat[0] = new Material(lpD3D, lpDev);
	lpUnitMat[0]->SetDiffuse(D3DVECTOR(1.0f, 1.0f, 1.0f));
	lpUnitMat[0]->SetRampSize(2);

	lpUnitMat[1] = new Material(lpD3D, lpDev);
	lpUnitMat[1]->SetDiffuse(D3DVECTOR(1.0f, 1.0f, 1.0f));
	lpUnitMat[1]->SetRampSize(2);

	lpUnitMat[2] = new Material(lpD3D, lpDev);
	lpUnitMat[2]->SetDiffuse(D3DVECTOR(1.0f, 1.0f, 1.0f));
	lpUnitMat[2]->SetRampSize(2);

	lpUnitMat[3] = new Material(lpD3D, lpDev);
	lpUnitMat[3]->SetDiffuse(D3DVECTOR(1.0f, 1.0f, 1.0f));
	lpUnitMat[3]->SetRampSize(2);

	// Create the unit textures and attach them to the materials
	UnitTex[0].Load(lpDev, "TPAGE_01");
	lpUnitMat[1]->SetTextureHandle(UnitTex[0].GetHandle());

	UnitTex[1].Load(lpDev, "TPAGE_00");
	lpUnitMat[2]->SetTextureHandle(UnitTex[1].GetHandle());

	UnitTex[2].Load(lpDev, "TPAGE_02");
	lpUnitMat[3]->SetTextureHandle(UnitTex[2].GetHandle());

	// create 2 lights
	D3DVECTOR	color(1.0f, 1.0f, 1.0f);
	D3DVECTOR	direction(-0.5f, -1.0f, -0.3f);

	lpLight1 = new DirectionalLight(lpD3D, color, Normalize(direction));
	if (lpLight1) {
		lpLight1->AddToViewport(lpView);
	}

	lpLight2 = new DirectionalLight(lpD3D, color/2.0f, -Normalize(direction));
	if (lpLight2) {
		lpLight2->AddToViewport(lpView);
	}

	// Success
    return D3D_OK;
} // End TrackEdit::AttachViewport

	

/*
**-----------------------------------------------------------------------------
**  Name:       TrackEdit::DetachViewport
**  Purpose:	Cleanup Viewport
**-----------------------------------------------------------------------------
*/

HRESULT TrackEdit::DetachViewport(void)
{
	LPDIRECT3DVIEWPORT2 lpViewport = NULL;

	if (lpd3dWindow) {
		lpViewport = lpd3dWindow->GetViewport();
	}

	// Cleanup lights
	if (lpLight1) {
		if (lpViewport) {
			lpLight1->RemoveFromViewport(lpViewport);
		}

	    delete lpLight1;
		lpLight1 = NULL;
	}

	if (lpLight2) {
		if (lpViewport) {
			lpLight2->RemoveFromViewport(lpViewport);
		}

		delete lpLight2;
		lpLight2 = NULL;
	}

	// Cleanup Materials
	delete lpBackgroundMat;
	delete lpGridMat;
	delete lpUnitMat[0];
	delete lpUnitMat[1];
	delete lpUnitMat[2];

	lpBackgroundMat = NULL;
	lpGridMat = NULL;
	lpUnitMat[0] = NULL;
	lpUnitMat[1] = NULL;
	lpUnitMat[2] = NULL;

	UnitTex[0].Release();
	UnitTex[1].Release();
	UnitTex[2].Release();

	// Success
	return D3D_OK;
} // End TrackEdit::DetachViewport


/*
**-----------------------------------------------------------------------------
**  Name:       TrackEdit::ReadRTUFile
**  Purpose:	Loads data from RTU (Revolt Track Unit) file into memory
**				and then builds D3D primitives from 
**-----------------------------------------------------------------------------
*/

using namespace std;

void TrackEdit::ReadRTUFile(const string& filename)
{
	EndianInputStream is(filename.c_str());
	D3DVECTOR v;
	if(is.is_open())
	{
		U32 fileid = is.GetU32();
		U16 version = is.GetU16();
		VALID_TARGETS targets = is.GetU16();
		
		ReadVertices(is);
		ReadPolygons(is);
		ReadMeshes(is);
		ReadUVCoords(is);
		ReadUVPolygons(is);
		ReadUnits(is);
	}
	CreatePrimitives();
}// End TrackEdit::ReadRTUFile

/*
**-----------------------------------------------------------------------------
**  Name:       TrackEdit::CreatePrimitives
**  Purpose:	Contructs D3D version of the data which has been loaded into
				memory from an RTU (Revolt Track Unit) file
**-----------------------------------------------------------------------------
*/

void TrackEdit::CreatePrimitives()
{
	DXUnits = new DXUNIT*[UnitCount];
	static float ucoords[] = {0.0, 1.0, 1.0, 0.0};
	static float vcoords[] = {0.0, 0.0, 1.0, 1.0};
	static BASICPOLY blankuvpoly = {FALSE, {0, 0, 0, 0}};
	for(U32 n = 0; n < UnitCount; n++)
	{
		DXUNIT* dxunit = new DXUNIT;
		DXUnits[n] = dxunit;
		TRACKUNIT* unit = Units[n];
		MESH* mesh = &Meshes[unit->MeshID];
		dxunit->ComponentCount = Meshes[n].PolySetCount;
		dxunit->Components = new COMPONENT*[dxunit->ComponentCount];
		for(U16 c = 0; c < dxunit->ComponentCount; c++)
		{
			COMPONENT* component = new COMPONENT;
			dxunit->Components[c] = component;
			component->PrimitiveCount = 4;
			component->Primitives = new PRIMITIVE*[component->PrimitiveCount];

			for(U16 ps = 0; ps < component->PrimitiveCount; ps++)
			{
				PRIMITIVE* primitive = new PRIMITIVE;
				component->Primitives[ps] = primitive;
				BASICPOLY* uvpoly = &blankuvpoly;
				U32 vc = 0;
				if(c == PAN_INDEX)
				{
					for(U16 p = 0; p < mesh->PolySets[c]->PolygonCount; p++)
					{
						uvpoly = &UVPolys[unit->UVPolys[p].PolyID];
						if(unit->UVPolys[p].TPageID == ps)
						{
							vc += 3;
							if(uvpoly->IsTriangle == false)
							{
								vc += 3;
							}
						}
					}
				}
				else
				{
					if(ps == 0)
					{
						vc = mesh->PolySets[c]->VertexCount;
					}
				}
				primitive->VertexCount = vc;
				if(vc == 0)
				{
					primitive->Vertices = NULL;
				}
				else
				{
					primitive->Vertices = new D3DVERTEX[primitive->VertexCount];
					U16 pv = 0;
					for(U16 p = 0; p < mesh->PolySets[c]->PolygonCount; p++)
					{
						if(((c == PAN_INDEX) && (unit->UVPolys[p].TPageID == ps)) || ((c != PAN_INDEX) && (ps == 0)) )
						{
							BASICPOLY* poly = &Polys[mesh->PolySets[c]->Indices[p]];
							uvpoly = &blankuvpoly;
							U32 uvmodvalue = 4;
							U32 uvindex = 0;
							U32 uvstep = 1;
							if(c == PAN_INDEX)
							{
								uvpoly = &UVPolys[unit->UVPolys[p].PolyID];
								uvmodvalue = uvpoly->IsTriangle ? 3 : 4;
								uvindex = unit->UVPolys[p].Rotation;
								uvstep = unit->UVPolys[p].Reversed ? (uvmodvalue - 1) : 1;
							}
							D3DVECTOR v1;
							D3DVECTOR v2;
							D3DVECTOR normal;
							D3DRMVectorSubtract(&v1, &Verts[poly->Vertices[0]], &Verts[poly->Vertices[1]]);
							D3DRMVectorSubtract(&v2, &Verts[poly->Vertices[2]], &Verts[poly->Vertices[1]]);
							D3DRMVectorCrossProduct(&normal, &v1, &v2);
							D3DRMVectorNormalize(&normal);

							for(U16 v = 0; v < 3; v++)
							{
								primitive->Vertices[pv].x = Verts[poly->Vertices[v]].x;
								primitive->Vertices[pv].y = Verts[poly->Vertices[v]].y;
								primitive->Vertices[pv].z = Verts[poly->Vertices[v]].z;
								primitive->Vertices[pv].nx = normal.x;
								primitive->Vertices[pv].ny = normal.y;
								primitive->Vertices[pv].nz = normal.z;
								primitive->Vertices[pv].tu = UVCoords[uvpoly->Vertices[uvindex]].U;
								primitive->Vertices[pv].tv = UVCoords[uvpoly->Vertices[uvindex]].V;
								pv++;
								uvindex += uvstep;
								uvindex %= uvmodvalue;
							}
							if(!poly->IsTriangle)
							{
								primitive->Vertices[pv].x = primitive->Vertices[pv-3].x;
								primitive->Vertices[pv].y = primitive->Vertices[pv-3].y;
								primitive->Vertices[pv].z = primitive->Vertices[pv-3].z;
								primitive->Vertices[pv].nx = normal.x;
								primitive->Vertices[pv].ny = normal.y;
								primitive->Vertices[pv].nz = normal.z;
								primitive->Vertices[pv].tu = primitive->Vertices[pv-3].tu;
								primitive->Vertices[pv].tv = primitive->Vertices[pv-3].tv;
								pv++;
								primitive->Vertices[pv].x = primitive->Vertices[pv-2].x;
								primitive->Vertices[pv].y = primitive->Vertices[pv-2].y;
								primitive->Vertices[pv].z = primitive->Vertices[pv-2].z;
								primitive->Vertices[pv].nx = normal.x;
								primitive->Vertices[pv].ny = normal.y;
								primitive->Vertices[pv].nz = normal.z;
								primitive->Vertices[pv].tu = primitive->Vertices[pv-2].tu;
								primitive->Vertices[pv].tv = primitive->Vertices[pv-2].tv;
								pv++;
								primitive->Vertices[pv].x = Verts[poly->Vertices[3]].x;
								primitive->Vertices[pv].y = Verts[poly->Vertices[3]].y;
								primitive->Vertices[pv].z = Verts[poly->Vertices[3]].z;
								primitive->Vertices[pv].nx = normal.x;
								primitive->Vertices[pv].ny = normal.y;
								primitive->Vertices[pv].nz = normal.z;
								primitive->Vertices[pv].tu = UVCoords[uvpoly->Vertices[uvindex]].U;
								primitive->Vertices[pv].tv = UVCoords[uvpoly->Vertices[uvindex]].V;
								pv++;
							}
						}
					}
				}
			}
		}
	}
}
  
void TrackEdit::OnKeyStroke(int vKeyCode)
{
	switch(vKeyCode)
	{
		case VK_LEFT:
		case VK_RIGHT:
		case VK_DOWN:
		case VK_UP:
			switch(Edit_Status)
			{
				case CHOOSING:
					MoveCursor(&UnitCursor, vKeyCode);
				break;
				case PLACING:
					MoveCursor(&TrackCursor, vKeyCode);
				break;
			}
		break;

		case VK_RETURN:
			PlaceUnit();	//place the currently selected unit onto the track
							//YES - this is supposed to fall through
		case VK_TAB:
			ToggleStatus(); //switch modes
		break;

		case VK_ADD:
			CurrentMat++;
			CurrentMat %= 3;
		break;
	}
}

void TrackEdit::MoveCursor(CURSOR_DESC* cursor, int vKeyCode)
{
	if(cursor != NULL)
	{
		switch(vKeyCode)
		{
			case VK_LEFT:
				cursor->X += (cursor->XMax - 1);
				cursor->X %= cursor->XMax;
				if((cursor->X + (cursor->Y * cursor->XMax)) >= cursor->AbsMax)
				{
					cursor->X = (cursor->AbsMax - ((cursor->YMax - 1) * cursor->XMax)) - 1;
				}
			break;

			case VK_RIGHT:
				cursor->X++;
				cursor->X %= cursor->XMax;
				if((cursor->X + (cursor->Y * cursor->XMax)) >= cursor->AbsMax)
				{
					cursor->X = 0;
				}
			break;

			case VK_DOWN:
				cursor->Y += (cursor->YMax - 1);
				cursor->Y %= cursor->YMax;
				if((cursor->X + (cursor->Y * cursor->XMax)) >= cursor->AbsMax)
				{
					cursor->Y = cursor->YMax - 1;
					if((cursor->X + (cursor->Y * cursor->XMax)) >= cursor->AbsMax)
					{
						cursor->Y--;
					}
				}
			break;

			case VK_UP:
				cursor->Y++;
				if((cursor->X + (cursor->Y * cursor->XMax)) >= cursor->AbsMax)
				{
					cursor->Y = 0;
				}
			break;
		}
	}
}

bool TrackEdit::CreateTrack(U32 width, U32 height)
{
	bool success = false;
	TrackGrid = new RevoltTrackUnitInstance[width * height];
	if(TrackGrid != NULL)
	{
		success = true;
		TrackCursor.XMax = TrackWidth = width;
		TrackCursor.YMax = TrackHeight = height;
		TrackCursor.X = TrackCursor.Y = 0;
		for(U32 g = 0; g < (TrackWidth * TrackHeight); g++)
		{
			TrackGrid[g].UnitID = 0;
		}
	}
	return success;
}

void TrackEdit::DestroyTrack()
{
	if(TrackGrid != NULL)
	{
		delete[] TrackGrid;
	}
	TrackWidth = 0;
	TrackHeight = 0;
}

void TrackEdit::ToggleStatus()
{
	switch(Edit_Status)
	{
		case CHOOSING:
			Edit_Status = PLACING;
		break;
		case PLACING:
			Edit_Status = CHOOSING;
		break;
	}
}

void TrackEdit::PlaceUnit()
{
	U32 m = (UnitCursor.Y * UnitCursor.XMax) + UnitCursor.X;
	U32 g = (TrackCursor.Y * TrackCursor.XMax) + TrackCursor.X;
	TrackGrid[g].UnitID = m;
}

/*
**----------------------------------------------------------------------------
** End of File
**----------------------------------------------------------------------------
*/

HRESULT TrackEdit::ParseCommandline()
{
	if (__argc == 1)
	{
		return E_INVALIDARG;
	}

	for (int i = 1 ; i < __argc ; i++)
	{
		if (*__argv[i] != '-')
		{
			return E_INVALIDARG;
		}

		if (strncmp(__argv[i], "-f", 2) == 0)
		{
			RTUFileName = __argv[i];
			RTUFileName = RTUFileName.substr(2);
		}
	}
	return S_OK;
}

void TrackEdit::ReadUVCoords(EndianInputStream& is)
{
	UVCoordCount = is.GetU32();
	UVCoords = new RevoltUVCoord[UVCoordCount];
	for(U32 v = 0; v < UVCoordCount; v++)
	{
		UVCoords[v].U = is.GetFloat();
		UVCoords[v].V = is.GetFloat();
	}
}

void TrackEdit::ReadUVPolygons(EndianInputStream& is)
{
	UVPolyCount = is.GetU32();
	UVPolys = new BASICPOLY[UVPolyCount];
	BASICPOLY* polyptr = UVPolys;
	for(U32 p = 0; p < UVPolyCount; p++)
	{
		U16 vertexcount = is.GetU16();
		polyptr->Vertices[0] = is.GetU32();
		polyptr->Vertices[1] = is.GetU32();
		polyptr->Vertices[2] = is.GetU32();
		if(vertexcount > 3)
		{
			polyptr->IsTriangle = FALSE;
			do{
				polyptr->Vertices[3] = is.GetU32();
			}while(--vertexcount > 3);
		}
		else
		{
			polyptr->IsTriangle = TRUE;
		}
		polyptr++;			
	}
}

void TrackEdit::ReadVertices(EndianInputStream& is)
{
	VertCount = is.GetU32();
	Verts = new D3DVECTOR[VertCount];
	for(U32 v = 0; v < VertCount; v++)
	{
		Verts[v].x = is.GetFloat();
		Verts[v].y = is.GetFloat();
		Verts[v].z = is.GetFloat();
	}
}

void TrackEdit::ReadPolygons(EndianInputStream& is)
{
	PolyCount = is.GetU32();
	Polys = new BASICPOLY[PolyCount];
	BASICPOLY* polyptr = Polys;
	for(U32 p = 0; p < PolyCount; p++)
	{
		U16 vertexcount = is.GetU16();
		polyptr->Vertices[0] = is.GetU32();
		polyptr->Vertices[1] = is.GetU32();
		polyptr->Vertices[2] = is.GetU32();
		if(vertexcount > 3)
		{
			polyptr->IsTriangle = FALSE;
			do{
				polyptr->Vertices[3] = is.GetU32();
			}while(--vertexcount > 3);
		}
		else
		{
			polyptr->IsTriangle = TRUE;
		}
		polyptr++;			
	}
}

void TrackEdit::ReadMeshes(EndianInputStream& is)
{
	MeshCount = is.GetU32();
	Meshes = new MESH[MeshCount];
	for(U32 m = 0; m < MeshCount; m++)
	{
		U16 polysetcount = Meshes[m].PolySetCount = is.GetU16();
		Meshes[m].PolySets = new POLYSET*[polysetcount];
		POLYSET** psptr = Meshes[m].PolySets;
		for(U16 ps = 0; ps < polysetcount; ps++)
		{
			POLYSET* polyset = new POLYSET;
			*psptr = polyset;
			psptr++;
			polyset->PolygonCount = is.GetU32();
			polyset->VertexCount = 0;
			polyset->Indices = new U32[polyset->PolygonCount];
			for(U32 p = 0; p < polyset->PolygonCount; p++)
			{
				polyset->Indices[p] = is.GetU32();
				polyset->VertexCount += 3;
				if(Polys[polyset->Indices[p]].IsTriangle == FALSE)
				{
					polyset->VertexCount += 3;
				}
			}
		}
	}
}

void TrackEdit::ReadUnits(EndianInputStream& is)
{
	UnitCount = is.GetU32();
	Units = new TRACKUNIT*[UnitCount];
	for(U32 i = 0; i < UnitCount; i++)
	{
		U32 meshid = is.GetU32();
		U32 uvpolycount = is.GetU32();
		U32 memsize = sizeof(TRACKUNIT) + ((uvpolycount -1) * sizeof(UVPOLYINSTANCE));
		TRACKUNIT* unit = (TRACKUNIT*) new U8[memsize];
		Units[i] = unit;
		unit->MeshID = meshid;
		unit->UVPolyCount = uvpolycount;
		for(U32 p = 0; p < uvpolycount; p++)
		{
			unit->UVPolys[p].TPageID = is.GetU32();
			unit->UVPolys[p].PolyID = is.GetU32();
			unit->UVPolys[p].Rotation = is.GetU8();
			unit->UVPolys[p].Reversed = (is.GetU8() != 0);
		}
	}
}

void TrackEdit::EraseGeometry()
{
	delete[] Verts;
	delete[] Polys;
	if(Meshes != NULL)
	{
		for(U32 m = 0; m < MeshCount; m++)
		{
			U16 polysetcount = Meshes[m].PolySetCount;
			POLYSET** psptr = Meshes[m].PolySets;
			for(U16 ps = 0; ps < polysetcount; ps++)
			{
				delete[] (*psptr)->Indices;
				delete (*psptr);
				psptr++;
			}
			delete[] Meshes[m].PolySets;
		}
	}
	delete[] Meshes;
	delete[] UVCoords;
	delete[] UVPolys;

	if(DXUnits != NULL)
	{
		for(U32 u = 0; u < MeshCount; u++)
		{
			DXUNIT* unit = DXUnits[u];
			for(U32 c =0; c < unit->ComponentCount; c++)
			{
				COMPONENT* component = unit->Components[c];
				for(U32 p = 0; p < component->PrimitiveCount; p++)
				{
					PRIMITIVE* primitive = component->Primitives[p];
					delete[] primitive->Vertices;
				}
				delete[] component->Primitives;
			}
			delete[] unit->Components;
		}
		delete[] DXUnits;
	}
}
