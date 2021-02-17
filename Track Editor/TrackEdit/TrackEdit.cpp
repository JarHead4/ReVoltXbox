//-----------------------------------------------------------------------------
// File: TrackEdit.cpp
//
// Desc: Main application logic for the Re-Volt Track Editor
//
//       Note: This code uses the D3D Framework helper library.
//
//
// Copyright (c) 1998 Probe Entertainment. All rights reserved.
//-----------------------------------------------------------------------------

#define STRICT
#define D3D_OVERLOADS
#include <ddraw.h>
#include <d3d.h>
#include <d3drm.h>
#include <dinput.h>
#include <math.h>
#include <string>
#include "D3DTextr.h"
#include "D3DUtil.h"
#include "D3DFrame.h"
#include "D3DMath.h"
#include "D3DClass.h"

#include "fileio.h"
#include "editor.h"
#include "Render.h"
#include "inputs.h"
#include "states.h"
#include "modules.h"

//-----------------------------------------------------------------------------
// Declare the application globals for use by WinMain.cpp
//-----------------------------------------------------------------------------
TCHAR*				g_strAppTitle       = TEXT( "Re-Volt Track Editor" );
BOOL				g_bAppUseZBuffer    = TRUE;    // Create/use a z-buffering
BOOL				g_bAppUseBackBuffer = TRUE;    // Create/use a back buffer
BOOL				g_bAppUseRefRast    = FALSE;   // Enumerate the reference rasterizer
BOOL				g_bWindowed			= FALSE;

int					g_DefaultScreenWidth = 640;
int					g_DefaultScreenHeight = 480;

//-----------------------------------------------------------------------------
// Declare the other application globals
//-----------------------------------------------------------------------------
TRACKTHEME				Theme;
TRACKDESC				TrackData;

LPDIRECTINPUT			DirectInputObject	= NULL;
LPDIRECTINPUTDEVICE		KeyboardDevice		= NULL;

PLACEMENT_CAMERA_DIR	PlacementViewpoint	= PCD_NORTH;
CURSORDESC				UnitCursor;
CURSORDESC				ModuleCursor;
INDEX					CurrentModule;
CURSORDESC				TrackCursor;
DIRECTION				ModuleDirection;
S16						ModuleElevation;
string					RTUPathName;
DXUNIT**				DXUnits				= NULL;
char**					FileList			= NULL;
U16						FileCount;
U16						FileWindowStart		= 0;
U16						FileWindowOffset	= 0;
bool					StatsOn				= true;

//-----------------------------------------------------------------------------
// Defines, constants, and global variables
//-----------------------------------------------------------------------------
extern MODULE_LOOKUP_INFO ToyLookup;
extern HINSTANCE	g_hInstance;
//-----------------------------------------------------------------------------
// Function prototypes and global (or static) variables
//-----------------------------------------------------------------------------
VOID    AppPause( BOOL );
HRESULT App_InitDeviceObjects( LPDIRECT3DDEVICE3, LPDIRECT3DVIEWPORT3 );
VOID    App_DeleteDeviceObjects( LPDIRECT3DDEVICE3, LPDIRECT3DVIEWPORT3 );
void	CreatePrimitives(void);
HRESULT ParseCommandline(void);
void	DestroyTrack(TRACKDESC* track);
void	InitializeTheme(void);
void	DestroyTheme(void);
void	MoveCursor(CURSORDESC* cursor, int vKeyCode);
BOOL	WINAPI DI_Init(void);
void	WINAPI DI_Term(void);
void	WINAPI ProcessKBInput(void);

//-----------------------------------------------------------------------------
// File scope variables
//-----------------------------------------------------------------------------

D3DMATRIX	proj, view, world;

Light		*lpLight1,
			*lpLight2;

Material	*lpBackgroundMat,
			*lpGridMat,
			*lpUnitMat[4];

LPDIRECT3DTEXTURE2 UnitTex[9];
LPDIRECT3DTEXTURE2 ButtonTex;
LPDIRECT3DTEXTURE2 SprueWireTex;
LPDIRECT3DTEXTURE2 SprueFontTex;
LPDIRECT3DTEXTURE2 GameFontTex;
LPDIRECT3DTEXTURE2 ClockTex;

static TCHAR ButtonTextureName[] = "buttons.bmp";
static TCHAR SprueWireTextureName[] = "spruewire.bmp";
static TCHAR SprueFontTextureName[] = "spruefont2.bmp";
static TCHAR GameFontTextureName[] = "gamefont.bmp";
static TCHAR ClockTextureName[] = "clock.bmp";

//-----------------------------------------------------------------------------
// Name: App_InitOptions()
// Desc: Called during initial app startup, this function should setup global
//       variables to indicate the options desired by the app.
//       i.e. g_bWindowed = FALSE etc
//-----------------------------------------------------------------------------
VOID App_InitOptions( )
{
	g_bWindowed = TRUE;
}

//-----------------------------------------------------------------------------
// Name: App_OneTimeSceneInit()
// Desc: Called during initial app startup, this function performs all the
//       permanent initialization.
//-----------------------------------------------------------------------------
HRESULT App_OneTimeSceneInit( HWND hWnd, LPDIRECT3DDEVICE3 pd3dDevice,
                              LPDIRECT3DVIEWPORT3 pvViewport )
{
	char application_name[_MAX_PATH];
	GetModuleFileName(g_hInstance, application_name, sizeof(application_name));
	RTUPathName = application_name;
	RTUPathName = RTUPathName.substr(0, RTUPathName.rfind('\\'));
	InitializeTheme();
	
	if(ReadRTUFile(RTUPathName, &Theme) != false)
	{
		Theme.Lookup = &ToyLookup;
		TrackData.Units = NULL;
		TrackData.Modules = NULL;

		InitializeCursorPrimitive();
		InitializeUnitRootPrimitive();
		InitializeMenuPolys();

		CreatePrimitives();

		char filename[MAX_PATH];
		
		for(U16 i = 0; i < 8; i++)
		{
			sprintf(filename, "%s\\tpage_%02d.bmp", RTUPathName.c_str(), i);
			D3DTextr_CreateTexture(filename);
		}
		D3DTextr_CreateTexture(ButtonTextureName);
		D3DTextr_CreateTexture(SprueWireTextureName);
		D3DTextr_CreateTexture(SprueFontTextureName);
		D3DTextr_CreateTexture(GameFontTextureName);
		D3DTextr_CreateTexture(ClockTextureName);

		UnitCursor.X = 0;
		UnitCursor.Y = 0;
		UnitCursor.XSize = 1;
		UnitCursor.YSize = 1;
		U32 width, height;
		
		width = height = (U32)sqrt(Theme.MeshCount);

		while((width * height) < Theme.MeshCount)
		{
			width++;
			if((width * height) < Theme.MeshCount)
			{
				height++;
			}
		}

		UnitCursor.XMax = width;
		UnitCursor.YMax = height;

		UnitCursor.AbsMax = Theme.MeshCount;

		ModuleCursor.X = 0;
		ModuleCursor.Y = 0;
		ModuleCursor.XSize = 1;
		ModuleCursor.YSize = 1;

		ModuleCursor.XMax = 1;
		ModuleCursor.YMax = Theme.Lookup->GroupCount;
		ModuleCursor.AbsMax = ModuleCursor.YMax;

		ModuleDirection = NORTH;
		ModuleElevation = 0;
		
		//create a default track
		NewTrack(&TrackData);

		// Initialize the device-dependant objects, such as materials and lights.
		// Note: this is in a separate function so that device changes can be
		// handled easily.
		return App_InitDeviceObjects( pd3dDevice, pvViewport );
	}
	else
	{
		return E_FAIL;
	}
}

//-----------------------------------------------------------------------------
// Name: App_FrameMove()
// Desc: Called once per frame, the call is the entry point for animating
//       the scene.
//-----------------------------------------------------------------------------
HRESULT App_FrameMove( LPDIRECT3DDEVICE3 pd3dDevice, FLOAT fTimeKey )
{
	static bool firstpass = true;
	static FLOAT lasttime;
	FLOAT elapsedtime = 0;
	if(firstpass == false)
	{
		elapsedtime = fTimeKey - lasttime;
	}
	lasttime = fTimeKey;
	firstpass = false;

	HeartBeat(elapsedtime);

    return DD_OK;
}

//-----------------------------------------------------------------------------
// Name: App_Render()
// Desc: Called once per frame, the call is the entry point for 3d
//       rendering. This function sets up render states, clears the
//       viewport, and renders the scene.
//-----------------------------------------------------------------------------
HRESULT App_Render( LPDIRECT3DDEVICE3 pd3dDevice, 
                    LPDIRECT3DVIEWPORT3 pvViewport,
                    D3DRECT* prcViewportRect )
{
    //Clear the viewport
    pvViewport->Clear( 1UL, prcViewportRect, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER );

    ProcessKBInput();
	
	// Begin the scene 
    if( SUCCEEDED( pd3dDevice->BeginScene() ) )
    {

		DWORD			dwSurfW, dwSurfH;

		// Get Surface Width and Height
		dwSurfW = abs (prcViewportRect->x2 - prcViewportRect->x1);
		dwSurfH = abs (prcViewportRect->y2 - prcViewportRect->y1);

		REAL inv_aspect = ONE;
		
		if (dwSurfW)
		{
			inv_aspect = (REAL)dwSurfH/(REAL)dwSurfW;
		}

		D3DUtil_SetProjectionMatrix(proj, pi / Real(4.0f), inv_aspect, Real(1000.0f), Real(50000.0f));
		proj._22 = -proj._22;	//adjust projection matrix so that our right handed data looks correct

		pd3dDevice->SetTransform(D3DTRANSFORMSTATE_PROJECTION, &proj);

        if(ErrorExists())
		{
			DrawErrorMessage(pd3dDevice);
		}
		else
		{
			switch(GetScreenState())
			{
				case ESS_PLACING_MODULE:
					DrawPlacingScreen(pd3dDevice, &Theme);
					DrawPopupMenu(pd3dDevice);
				break;
				
				case ESS_CHOOSING_MODULE:
					DrawChoosingScreen(pd3dDevice, &Theme);
				break;

				case ESS_LOADING_TRACK:
					DrawLoadMenu(pd3dDevice);
				break;

				case ESS_SAVING_TRACK:
					DrawSaveMenu(pd3dDevice);
				break;

				case ESS_EXPORTING_TRACK:
					DrawExportingScreen(pd3dDevice);
				break;			
				
				case ESS_CREATING_TRACK:
					DrawNewTrackMenu(pd3dDevice);
				break;
				
				case ESS_ADJUSTING_TRACK:
					DrawAdjustmentScreen(pd3dDevice, &Theme);
				break;
			}
		}
		DrawMainSprue(pd3dDevice);

        // End the scene.
        pd3dDevice->EndScene();
    }

    return DD_OK;
}




//-----------------------------------------------------------------------------
// Name: App_InitDeviceObjects()
// Desc: Initialize scene objects.
//-----------------------------------------------------------------------------
HRESULT App_InitDeviceObjects( LPDIRECT3DDEVICE3 pd3dDevice,
                               LPDIRECT3DVIEWPORT3 pvViewport )
{
    // Check parameters
    if( NULL==pd3dDevice || NULL==pvViewport )
        return DDERR_INVALIDPARAMS;

    LPDIRECT3D3 pD3D;
    if( FAILED( pd3dDevice->GetDirect3D( &pD3D ) ) )
        return E_FAIL;

    pD3D->Release();

	// Create and set up the background material
	lpBackgroundMat = new Material(pD3D, pd3dDevice);
	lpBackgroundMat->SetDiffuse(D3DVECTOR(Real(0.097f), Real(0.195f), Real(0.39f)));
	lpBackgroundMat->SetAsBackground(pvViewport);

	lpUnitMat[0] = new Material(pD3D, pd3dDevice);
	lpUnitMat[0]->SetDiffuse(D3DVECTOR(ONE, ONE, ONE));
	lpUnitMat[0]->SetRampSize(2);

	lpUnitMat[1] = new Material(pD3D, pd3dDevice);
	lpUnitMat[1]->SetDiffuse(D3DVECTOR(ONE, ONE, ONE));
	lpUnitMat[1]->SetRampSize(2);

	lpUnitMat[2] = new Material(pD3D, pd3dDevice);
	lpUnitMat[2]->SetDiffuse(D3DVECTOR(ONE, ONE, ONE));
	lpUnitMat[2]->SetRampSize(2);

	lpUnitMat[3] = new Material(pD3D, pd3dDevice);
	lpUnitMat[3]->SetDiffuse(D3DVECTOR(ONE, ONE, ONE));
	lpUnitMat[3]->SetRampSize(2);

	// Create the unit textures and attach them to the materials
	D3DTextr_RestoreAllTextures(pd3dDevice);
    pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
    pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
    pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );

	char filename[MAX_PATH];
	
	UnitTex[0] = NULL;
	for(U16 i = 0; i < 8; i++)
	{
		sprintf(filename, "%s\\tpage_%02d.bmp", RTUPathName.c_str(), i);
		UnitTex[i+1] = D3DTextr_GetTexture(filename);
	}

	DDCOLORKEY ck;

	ck.dwColorSpaceLowValue = 0;
	ck.dwColorSpaceHighValue = 0;

	ButtonTex = D3DTextr_GetTexture(ButtonTextureName);
	LPDIRECTDRAWSURFACE4 buttonsurf = D3DTextr_GetSurface(ButtonTextureName);
	buttonsurf->SetColorKey(DDCKEY_SRCBLT, &ck);

	SprueWireTex = D3DTextr_GetTexture(SprueWireTextureName);
	LPDIRECTDRAWSURFACE4 spruewiresurf = D3DTextr_GetSurface(SprueWireTextureName);
	spruewiresurf->SetColorKey(DDCKEY_SRCBLT, &ck);
	
	SprueFontTex = D3DTextr_GetTexture(SprueFontTextureName);
	LPDIRECTDRAWSURFACE4 spruefontsurf = D3DTextr_GetSurface(SprueFontTextureName);
	spruefontsurf->SetColorKey(DDCKEY_SRCBLT, &ck);
	
	GameFontTex = D3DTextr_GetTexture(GameFontTextureName);
	LPDIRECTDRAWSURFACE4 gamefontsurf = D3DTextr_GetSurface(GameFontTextureName);
	gamefontsurf->SetColorKey(DDCKEY_SRCBLT, &ck);
	
	ClockTex = D3DTextr_GetTexture(ClockTextureName);
	LPDIRECTDRAWSURFACE4 clocksurf = D3DTextr_GetSurface(ClockTextureName);
	clocksurf->SetColorKey(DDCKEY_SRCBLT, &ck);
	
	// create 2 lights
	D3DVECTOR	color(ONE, ONE, ONE);
	D3DVECTOR	direction(Real(-0.5f), -ONE, Real(-0.3f));

	lpLight1 = new DirectionalLight(pD3D, color, Normalize(direction));
	if (lpLight1) {
		lpLight1->AddToViewport(pvViewport);
	}

	lpLight2 = new DirectionalLight(pD3D, color / Real(2.0f), -Normalize(direction));
	if (lpLight2) {
		lpLight2->AddToViewport(pvViewport);
	}

	lpUnitMat[0]->SetAsCurrent(pd3dDevice);
	
	// set renderstates back to default
    if (pd3dDevice->SetRenderState(D3DRENDERSTATE_BLENDENABLE, FALSE) != D3D_OK)
        return FALSE;
    if (pd3dDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE) != D3D_OK)
        return FALSE;
    if (pd3dDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ZERO) != D3D_OK)
		return FALSE;
    if (pd3dDevice->SetRenderState(D3DRENDERSTATE_ZENABLE, TRUE) != D3D_OK)
		return FALSE;
    if (pd3dDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE) != D3D_OK)
		return FALSE;
    if (pd3dDevice->SetRenderState(D3DRENDERSTATE_CULLMODE,D3DCULL_CW) != D3D_OK)
		return FALSE;
    if (pd3dDevice->SetRenderState(D3DRENDERSTATE_COLORKEYENABLE, TRUE) != D3D_OK)
		return FALSE;

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: App_FinalCleanup()
// Desc: Called before the app exits, this function gives the app the chance
//       to cleanup after itself.
//-----------------------------------------------------------------------------
HRESULT App_FinalCleanup( LPDIRECT3DDEVICE3 pd3dDevice, 
                          LPDIRECT3DVIEWPORT3 pvViewport)
{
    DestroyTheme();
	DestroyTrack(&TrackData);
	App_DeleteDeviceObjects( pd3dDevice, pvViewport );
    
    return DD_OK;
}




//-----------------------------------------------------------------------------
// Name: App_DeleteDeviceObjects()
// Desc: Called when the app is exitting, or the device is being changed,
//       this function deletes any device dependant objects.
//-----------------------------------------------------------------------------
VOID App_DeleteDeviceObjects( LPDIRECT3DDEVICE3 pd3dDevice, 
                              LPDIRECT3DVIEWPORT3 pvViewport)
{
	D3DTextr_InvalidateAllTextures();

	// Cleanup lights
	if (lpLight1) {
		if (pvViewport) {
			lpLight1->RemoveFromViewport(pvViewport);
		}

	    delete lpLight1;
		lpLight1 = NULL;
	}

	if (lpLight2) {
		if (pvViewport) {
			lpLight2->RemoveFromViewport(pvViewport);
		}

		delete lpLight2;
		lpLight2 = NULL;
	}

	// Cleanup Materials
	delete lpBackgroundMat;
	delete lpUnitMat[0];
	delete lpUnitMat[1];
	delete lpUnitMat[2];

	lpBackgroundMat = NULL;
	lpUnitMat[0] = NULL;
	lpUnitMat[1] = NULL;
	lpUnitMat[2] = NULL;


    D3DTextr_InvalidateAllTextures();
}




//----------------------------------------------------------------------------
// Name: App_RestoreSurfaces
// Desc: Restores any previously lost surfaces. Must do this for all surfaces
//       (including textures) that the app created.
//----------------------------------------------------------------------------
HRESULT App_RestoreSurfaces( )
{
    D3DTextr_RestoreAllTextureSurfaces();
    return D3D_OK;
}




//-----------------------------------------------------------------------------
// Name: App_ConfirmDevice()
// Desc: Called during device intialization, this code checks the device
//       for some minimum set of capabilities
//-----------------------------------------------------------------------------
HRESULT App_ConfirmDevice( DDCAPS* pddDriverCaps,
						   D3DDEVICEDESC* pd3dDeviceDesc )
{
    return S_OK;
}

/*
**-----------------------------------------------------------------------------
**  Name:       CreatePrimitives
**  Purpose:	Contructs D3D version of the data which has been loaded into
				memory from an RTU (Revolt Track Unit) file
**-----------------------------------------------------------------------------
*/

void CreatePrimitives(void)
{
	DXUnits = new DXUNIT*[Theme.UnitCount];
	static REAL ucoords[] = {Real(0.0), Real(1.0), Real(1.0), Real(0.0)};
	static REAL vcoords[] = {Real(0.0), Real(0.0), Real(1.0), Real(1.0)};
	static BASICPOLY blankuvpoly = {FALSE, {0, 0, 0, 0}};
	for(U32 n = 0; n < Theme.UnitCount; n++)
	{
		DXUNIT* dxunit = new DXUNIT;
		DXUnits[n] = dxunit;
		TRACKUNIT* unit = Theme.Units[n];
		MESH* mesh = &Theme.Meshes[unit->MeshID];
		dxunit->ComponentCount = Theme.Meshes[n].PolySetCount;
		dxunit->Components = new COMPONENT*[dxunit->ComponentCount];
		for(U16 c = 0; c < dxunit->ComponentCount; c++)
		{
			COMPONENT* component = new COMPONENT;
			dxunit->Components[c] = component;
			component->PrimitiveCount = Theme.TPageCount;
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
						uvpoly = &Theme.UVPolys[unit->UVPolys[p].PolyID];
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
							BASICPOLY* poly = &Theme.Polys[mesh->PolySets[c]->Indices[p]];
							uvpoly = &blankuvpoly;
							U32 uvmodvalue = 4;
							U32 uvindex = 0;
							U32 uvstep = 1;
							if(c == PAN_INDEX)
							{
								uvpoly = &Theme.UVPolys[unit->UVPolys[p].PolyID];
								uvmodvalue = uvpoly->IsTriangle ? 3 : 4;
								uvindex = unit->UVPolys[p].Rotation;
								uvstep = unit->UVPolys[p].Reversed ? (uvmodvalue - 1) : 1;
							}
							D3DVECTOR v1;
							D3DVECTOR v2;
							D3DVECTOR normal;
							D3DRMVectorSubtract(&v1, &Theme.Verts[poly->Vertices[0]], &Theme.Verts[poly->Vertices[1]]);
							D3DRMVectorSubtract(&v2, &Theme.Verts[poly->Vertices[2]], &Theme.Verts[poly->Vertices[1]]);
							D3DRMVectorCrossProduct(&normal, &v1, &v2);
							D3DRMVectorNormalize(&normal);

							for(U16 v = 0; v < 3; v++)
							{
								primitive->Vertices[pv].x = Theme.Verts[poly->Vertices[v]].x;
								primitive->Vertices[pv].y = Theme.Verts[poly->Vertices[v]].y;
								primitive->Vertices[pv].z = Theme.Verts[poly->Vertices[v]].z;
								primitive->Vertices[pv].nx = normal.x;
								primitive->Vertices[pv].ny = normal.y;
								primitive->Vertices[pv].nz = normal.z;
								primitive->Vertices[pv].tu = Theme.UVCoords[uvpoly->Vertices[uvindex]].U;
								primitive->Vertices[pv].tv = Theme.UVCoords[uvpoly->Vertices[uvindex]].V;
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
								primitive->Vertices[pv].x = Theme.Verts[poly->Vertices[3]].x;
								primitive->Vertices[pv].y = Theme.Verts[poly->Vertices[3]].y;
								primitive->Vertices[pv].z = Theme.Verts[poly->Vertices[3]].z;
								primitive->Vertices[pv].nx = normal.x;
								primitive->Vertices[pv].ny = normal.y;
								primitive->Vertices[pv].nz = normal.z;
								primitive->Vertices[pv].tu = Theme.UVCoords[uvpoly->Vertices[uvindex]].U;
								primitive->Vertices[pv].tv = Theme.UVCoords[uvpoly->Vertices[uvindex]].V;
								pv++;
							}
						}
					}
				}
			}
		}
	}
}
  
HRESULT ParseCommandline(void)
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
			RTUPathName = __argv[i];
			RTUPathName = RTUPathName.substr(2);
		}
	}
	return S_OK;
}

LRESULT App_HandleKeystroke(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	int vKeyCode = (int)wParam;
	switch(msg)
	{
		case WM_KEYDOWN:
			HandleKeyDown(hWnd, vKeyCode);
		break;
	}
	return 0;
}

void InitializeTheme(void)
{
	memset(&Theme, 0, sizeof(TRACKTHEME));
}

void DestroyTheme(void)
{
	if(Theme.Modules != NULL)
	{
		TRACKMODULE* module;
		while(Theme.ModuleCount--)
		{
			module = Theme.Modules[Theme.ModuleCount];
			if(module != NULL)
			{
				RevoltTrackUnitInstance* instance;
				while(module->InstanceCount--)
				{
					instance = module->Instances[module->InstanceCount];
					delete instance;
				}
				delete[] module->Instances;
				TRACKZONE* zone;
				while(module->ZoneCount--)
				{
					zone = module->Zones[module->ZoneCount];
					delete zone;
				}
				delete[] module->Zones;
			}
		}
		delete[] Theme.Modules;
		Theme.Modules = 0;
	}

	if(Theme.Units != NULL)
	{
		TRACKUNIT* unit;
		while(Theme.UnitCount--)
		{
			unit = Theme.Units[Theme.UnitCount];
			if(unit != NULL)
			{
				delete[] unit->UVPolys;
				delete[] unit->Surfaces;
				delete[] unit;
			}
		}
		delete[] Theme.Units;
		Theme.Units = 0;
	}

	if(Theme.Meshes != NULL)
	{
		MESH* mesh;
		while(Theme.MeshCount--)
		{
			mesh = &Theme.Meshes[Theme.MeshCount];
			POLYSET* polyset;
			while(mesh->PolySetCount--)
			{
				polyset = mesh->PolySets[mesh->PolySetCount];
				delete[] polyset->Indices;
				delete polyset;
			}
		}
		delete[] Theme.Meshes;
		Theme.Modules = 0;
	}

	delete[] Theme.Polys;
	delete[] Theme.UVPolys;
	delete[] Theme.Verts;
	delete[] Theme.UVCoords;
	
	Theme.UnitCount = 0;
	Theme.UVCoordCount = 0;
	Theme.UVPolyCount = 0;
	Theme.VertCount = 0;
	Theme.PolyCount = 0;
	Theme.MeshCount = 0;
	Theme.ModuleCount = 0;
	Theme.TPageCount = 0;
}

BOOL WINAPI DI_Init(HINSTANCE hInst, HWND hWnd) 
{ 
    HRESULT hr; 
 
    // Create the DirectInput object. 
    hr = DirectInputCreate(hInst, DIRECTINPUT_VERSION, 
                           &DirectInputObject, NULL); 
    if FAILED(hr) return FALSE; 
 
    // Retrieve a pointer to an IDirectInputDevice interface 
    hr = DirectInputObject->CreateDevice(GUID_SysKeyboard, &KeyboardDevice, NULL); 
    if FAILED(hr) 
    { 
        DI_Term(); 
        return FALSE; 
    } 
 
// Now that you have an IDirectInputDevice interface, get 
// it ready to use. 
 
    // Set the data format using the predefined keyboard data 
    // format provided by the DirectInput object for keyboards. 
    hr = KeyboardDevice->SetDataFormat(&c_dfDIKeyboard); 
    if FAILED(hr) 
    { 
        DI_Term(); 
        return FALSE; 
    } 
 
    // Set the cooperative level 
    hr = KeyboardDevice->SetCooperativeLevel(hWnd, 
                       DISCL_FOREGROUND | DISCL_NONEXCLUSIVE); 
    if FAILED(hr) 
    { 
        DI_Term(); 
        return FALSE; 
    } 
 
    // Get access to the input device. 
    hr = KeyboardDevice->Acquire(); 
    if FAILED(hr) 
    { 
        DI_Term(); 
        return FALSE; 
    } 
 
    return TRUE; 
} 
 
 
void WINAPI DI_Term(void) 
{ 
    if (DirectInputObject) 
    { 
        if (KeyboardDevice) 
        { 
            // Always unacquire the device before calling Release(). 
             KeyboardDevice->Unacquire(); 
             KeyboardDevice->Release();
             KeyboardDevice = NULL; 
        } 
        DirectInputObject->Release();
        DirectInputObject = NULL; 
    } 
} 
 
 
BOOL InitializeInputDevices(HINSTANCE hInst, HWND hWnd)
{
	return DI_Init(hInst, hWnd);
}

void WINAPI ProcessKBInput(void) 
{ 
    #define KEYDOWN(name,key) (name[key] & 0x80) 
 
    char     buffer[256]; 
    HRESULT  hr; 
 
    hr = KeyboardDevice->GetDeviceState(sizeof(buffer),(LPVOID)&buffer); 
    if FAILED(hr) 
    { 
         // If it failed, the device has probably been lost. 
         // We should check for (hr == DIERR_INPUTLOST) 
         // and attempt to reacquire it here. 
         return; 
    } 
 
    // Turn the status display on/off
    if (KEYDOWN(buffer, DIK_LSHIFT))
    {
		StatsOn = !StatsOn;
	}
} 
 
