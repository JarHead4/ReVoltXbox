#define STRICT
#define D3D_OVERLOADS
#include <d3dframe.h>

#define UNIT_INFO_EXTERN

#include <assert.h>
#include "Render.h"
#include "editorconstants.h"
#include "UnitInfo.h"
#include "states.h"
#include <time.h>

extern int	g_DefaultScreenWidth;
extern int	g_DefaultScreenHeight;

extern MAIN_STATES			MainState;

extern POPUP_MENU_STATES	PopupState;
extern POPUP_MENU_ACTIONS	PopupAction;
extern U16					PopupMenuPosition;
extern REAL					PopupMenuTime;
extern U16					PopupCursorFrame;
extern U16					PopupCursorPosition;
extern REAL					PopupCursorTime;
extern COMPILE_STATES		CompileState;
extern U8					ClockFrame;

extern NEW_TRACK_STATES		NewTrackState;
extern LOAD_TRACK_STATES	LoadTrackState;
extern SAVE_TRACK_STATES	SaveTrackState;

extern PLACEMENT_CAMERA_DIR	PlacementViewpoint;
extern CURSORDESC			UnitCursor;
extern CURSORDESC			ModuleCursor;
extern CURSORDESC			TrackCursor;
extern LPDIRECT3DTEXTURE2	UnitTex[9];
extern LPDIRECT3DTEXTURE2	ButtonTex;
extern LPDIRECT3DTEXTURE2	SprueWireTex;
extern LPDIRECT3DTEXTURE2	SprueFontTex;
extern LPDIRECT3DTEXTURE2	GameFontTex;
extern LPDIRECT3DTEXTURE2	ClockTex;
extern CD3DFramework*		g_pFramework;
extern TRACKDESC			TrackData;
extern DXUNIT**				DXUnits;
extern U16					FileWindowStart;
extern U16					FileWindowOffset;
extern char**				FileList;
extern U16					FileCount;

#define FONT_HEIGHT 17

VOID   AppOutputText( LPDIRECT3DDEVICE3, DWORD, DWORD, CHAR* );

U16 TextureChanges;
U16 CurrentTexturePage;

static const REAL CameraDistance = SMALL_CUBE_SIZE * Real(7.5f);

U16		ModuleWidth(TRACKMODULE* module);
U16		ModuleHeight(TRACKMODULE* module);

static D3DVECTOR		box_color(Real(0.7f), Real(0.2f), ZERO);

static D3DLVERTEX		boxverts[8];
static WORD			boxlines[24] = { 0, 1, 1, 2, 2, 3, 3, 0,
						 4, 5, 5, 6, 6, 7, 7, 4,
						 0, 4, 1, 5, 2, 6, 3, 7
					   };
static D3DVERTEX		unitrootverts[16];
static WORD			unitrootindices[24] = { 8, 1, 0, 8, 9, 1,
										10, 3, 2, 10, 11, 3,
										12, 5, 4, 12, 13, 5,
										14, 7, 6, 14, 15, 7
									 };

static D3DTLVERTEX MenuVerts[4];
static D3DTLVERTEX MenuCursorVerts[4];
static D3DTLVERTEX SpriteVerts[4];
static D3DMATRIX   TrackScaleMatrix;
static LPDIRECT3DTEXTURE2	CurrentTexture = NULL;
static DWORD	   SpriteClipOption = D3DDP_DONOTCLIP;

extern FONTDESCRIPTION GameFont;
extern FONTDESCRIPTION SprueWireFont;
extern FONTDESCRIPTION SprueTextFont;

void SetCurrentTexture(LPDIRECT3DDEVICE3 pd3dDevice, LPDIRECT3DTEXTURE2	newtexture)
{
	if(newtexture != CurrentTexture)
	{
		pd3dDevice->SetTexture( 0, newtexture);
		CurrentTexture = newtexture;
	}
}

void UseFontTexture(LPDIRECT3DDEVICE3 pd3dDevice)
{
	SetCurrentTexture(pd3dDevice, GameFontTex);
}

void UseSprueTexture(LPDIRECT3DDEVICE3 pd3dDevice)
{
	SetCurrentTexture(pd3dDevice, SprueFontTex);
}

void UseButtonTexture(LPDIRECT3DDEVICE3 pd3dDevice)
{
	SetCurrentTexture(pd3dDevice, ButtonTex);
}

void UseClockTexture(LPDIRECT3DDEVICE3 pd3dDevice)
{
	SetCurrentTexture(pd3dDevice, ClockTex);
}

void DrawErrorMessage(LPDIRECT3DDEVICE3 pd3dDevice)
{
	UseSprueTexture(pd3dDevice);
	DrawFrame(pd3dDevice, 32, 144, 12, 6);

	SetCurrentFont(pd3dDevice, &GameFont);

	SetSpriteColor(PURE_RED);
	DrawText( pd3dDevice, FILE_WINDOW_LEFT, FILE_WINDOW_TOP - FONT_HEIGHT, "Warning!!");

	const char* errortext = GetErrorText();
	assert(strlen(errortext) != 0);		//ensure that the user has specified a suitable error message
	SetSpriteColor(PURE_WHITE);
	DrawText( pd3dDevice, FILE_WINDOW_LEFT, FILE_WINDOW_TOP , errortext);
}

void DrawClock(LPDIRECT3DDEVICE3 pd3dDevice)
{
	static const REAL CLOCK_LEFT = 64;
	static const REAL CLOCK_TOP = 272;
	static const REAL CLOCK_RIGHT = CLOCK_LEFT + 127;
	static const REAL CLOCK_BOTTOM = CLOCK_TOP + 127;

	UseClockTexture(pd3dDevice);

	if(CompileState == TCS_RINGING_CLOCK)
	{
		REAL srcleft = ZERO;
		REAL srctop = HALF;
		REAL srcright = HALF;
		REAL srcbottom = ONE;
		if(ClockFrame & 0x04)
		{
			srcleft = HALF;
			srcright = ONE;
		}

		DrawSprite(pd3dDevice, CLOCK_LEFT, CLOCK_TOP, CLOCK_RIGHT, CLOCK_BOTTOM, srcleft, srctop, srcright, srcbottom);
	}
	else
	{
		if(CompileState != TCS_VALIDATING)
		{
			//draw the main body of the clock
			DrawSprite(pd3dDevice, CLOCK_LEFT, CLOCK_TOP, CLOCK_RIGHT, CLOCK_BOTTOM, ZERO, ZERO, HALF, HALF);

			//now draw the spots indicating current processing stage
			REAL srcleft = HALF;
			REAL srctop = ZERO;
			REAL srcright = srcleft + Real(0.0625f);
			REAL srcbottom = srctop + Real(0.0625f);
			REAL destleft;
			REAL desttop;
			REAL destright;
			REAL destbottom;
			static const REAL spotleft[12] = {63, 72, 75, 72, 64, 52, 40, 31, 28, 32, 41, 52};
			static const REAL spottop[12] = {44, 52, 63, 74, 83, 86, 83, 75, 63, 52, 44, 41};

			U16 spot = 12;
			while(spot--)
			{
				if(CompileState == spot)
				{
					srcleft += Real(0.0625);
					srcright += Real(0.0625f);
				}
				destleft = spotleft[spot] + CLOCK_LEFT;
				desttop = spottop[spot] + CLOCK_TOP;
				destright = destleft + 15;
				destbottom = desttop + 15;
				DrawSprite(pd3dDevice, destleft, desttop, destright, destbottom, srcleft, srctop, srcright, srcbottom);
			}
		}
	}
}

void DrawExportingScreen(LPDIRECT3DDEVICE3 pd3dDevice)
{
    CHAR compile_state_text[][40]=
	{
		{"Stage 1"},
		{"Stage 2"},
		{"Stage 3"},
		{"Stage 4"},
		{"Stage 5"},
		{"Stage 6"},
		{"Stage 7"},
		{"Stage 8"},
		{"Stage 9"},
		{"Stage 10"},
		{"Stage 11"},
		{"Stage 12"},
		{"Finished!!"},
	};
	UseSprueTexture(pd3dDevice);
	DrawFrame(pd3dDevice, 32, 144, 6, 3);
	DrawFrame(pd3dDevice, 32, 240, 6, 6);

	SetCurrentFont(pd3dDevice, &GameFont);
	DrawText( pd3dDevice, FILE_WINDOW_LEFT, FILE_WINDOW_TOP - 16, compile_state_text[CompileState]);
	DrawClock(pd3dDevice);
}

void DrawLoadMenu(LPDIRECT3DDEVICE3 pd3dDevice)
{
	UseSprueTexture(pd3dDevice);
	DrawFrame(pd3dDevice, 32, 144, 10, 10);
	DrawFrame(pd3dDevice, 352, 144, 4, 4);

	SetCurrentFont(pd3dDevice, &GameFont);

	CHAR menuentry[80];

    U32 color;
	for(U16 offset = 0; offset < MAX_FILES_IN_WINDOW; offset++)
	{
		color = PURE_WHITE;
		U16 n = FileWindowStart + offset;
		if(n < FileCount)
		{
			strcpy(menuentry, FileList[n]);
			if((offset == FileWindowOffset) && (LoadTrackState == LTS_CHOOSING))
			{
				color = PURE_RED;
			}
			SetSpriteColor(color);
			DrawText( pd3dDevice, FILE_WINDOW_LEFT, FILE_WINDOW_TOP + (offset * FONT_HEIGHT), menuentry );
		}
	}

	color = PURE_WHITE;
	if(LoadTrackState == LTS_CANCEL)
	{
		color = PURE_RED;
	}
	SetSpriteColor(color);
	DrawText( pd3dDevice, CANCEL_WINDOW_LEFT, CANCEL_WINDOW_TOP, "Cancel" );
	SetSpriteColor(PURE_WHITE);
}

void DrawSaveMenu(LPDIRECT3DDEVICE3 pd3dDevice)
{
	UseSprueTexture(pd3dDevice);
	DrawFrame(pd3dDevice, 32, 144, 10, 4);
	DrawFrame(pd3dDevice, 352, 144, 4, 4);

	SetCurrentFont(pd3dDevice, &GameFont);

	CHAR menuentry[80];
	static const long MARK = 600;
	static const long SPACE = 400;

	U32 color = PURE_WHITE;
	strcpy(menuentry, TrackData.Name);
	if(SaveTrackState == STS_ENTERING)
	{
		long time = clock();
		if((time % (MARK + SPACE)) < MARK)
		{
			strcat(menuentry, "_");
		}
		else
		{
			strcat(menuentry, " ");
		}
		color = PURE_RED;
	}
	SetSpriteColor(color);
	DrawText( pd3dDevice, FILE_WINDOW_LEFT, FILE_WINDOW_TOP, menuentry );

	color = PURE_WHITE;
	if(SaveTrackState == STS_SAVE)
	{
		color = PURE_RED;
	}
	SetSpriteColor(color);
	DrawText( pd3dDevice, FILE_WINDOW_LEFT, FILE_WINDOW_TOP + (FONT_HEIGHT * 2), "Save Track");

	color = PURE_WHITE;
	if(SaveTrackState == STS_CANCEL)
	{
		color = PURE_RED;
	}
	SetSpriteColor(color);
	DrawText( pd3dDevice, CANCEL_WINDOW_LEFT, CANCEL_WINDOW_TOP, "Cancel" );
	SetSpriteColor(PURE_WHITE);
}

void DrawNewTrackMenu(LPDIRECT3DDEVICE3 pd3dDevice)
{
	SetCurrentFont(pd3dDevice, &GameFont);

    CHAR theme_menu_text[][40]=
	{
		{"Toy-World"},
		{"Rc-World"},
		{"Cancel"},
	};
    for(int i = 0; i < 3; i++)
	{
		U32 color = PURE_WHITE;
		if(NewTrackState == i)
		{
			color = PURE_RED;
		}
		SetSpriteColor(color);
		DrawText( pd3dDevice, FILE_WINDOW_LEFT, FILE_WINDOW_TOP + (FONT_HEIGHT * i), theme_menu_text[i] );
	}
	SetSpriteColor(PURE_WHITE);
}

void DrawChoosingScreen(LPDIRECT3DDEVICE3 pd3dDevice, TRACKTHEME* theme)
{
	UseSprueTexture(pd3dDevice);
	DrawFrame(pd3dDevice, 32, 144, 7, 10);
	DrawFrame(pd3dDevice, 256, 144, 9, 10);

	SetCurrentFont(pd3dDevice, &GameFont);

	SetSpriteColor(PURE_WHITE);
	U16 groupcount = theme->Lookup->GroupCount;
	while(groupcount--)
	{
		if(groupcount == ModuleCursor.Y)
		{
			SetSpriteColor(PURE_RED);
		}
		DrawText(pd3dDevice, 48, 176 + (groupcount * FONT_HEIGHT), theme->Lookup->Groups[groupcount].Name);
		if(groupcount == ModuleCursor.Y)
		{
			SetSpriteColor(PURE_WHITE);
		}
	}

	HRESULT hResult;

	DWORD fvf = D3DFVF_NORMAL | D3DFVF_XYZ | D3DFVF_TEX1;

	D3DMATRIX			camera;
	static D3DVECTOR	up(ZERO, ONE, ZERO);
	static D3DVECTOR	from(ZERO, -CameraDistance, -CameraDistance);
	static D3DVECTOR	at(ZERO, ZERO, ZERO);

	D3DUtil_SetViewMatrix(camera, from, at, up);
	pd3dDevice->SetTransform(D3DTRANSFORMSTATE_VIEW, &camera);

	D3DMATRIX ObjectMatrix;
	D3DMATRIX TransMatrix;
	D3DMATRIX ScaleMatrix;
	D3DMATRIX RotMatrix;
	static REAL spin = Real(0.0f);

	for(CurrentTexturePage = 0; CurrentTexturePage < theme->TPageCount; CurrentTexturePage++)
	{
		SetCurrentTexture(pd3dDevice, UnitTex[CurrentTexturePage]);
		TextureChanges++;

		INDEX m = theme->Lookup->Groups[ModuleCursor.Y].ModuleID;
		
		TRACKMODULE* module = theme->Modules[m];
		REAL scale = (ONE / max(ModuleWidth(module), ModuleHeight(module))) * Real(2.0f);
		REAL xshift = (SMALL_CUBE_SIZE * (ModuleWidth(module) - 1)) / Real(2.0f);
		REAL yshift = (SMALL_CUBE_SIZE * (ModuleHeight(module) - 1)) / Real(2.0f);
		for(U32 i = 0; i < module->InstanceCount; i++)
		{
			RevoltTrackUnitInstance* instance = module->Instances[i];

			D3DUtil_SetScaleMatrix(ScaleMatrix, scale, scale, scale);
			D3DUtil_SetTranslateMatrix(TransMatrix, (instance->XPos * SMALL_CUBE_SIZE) - xshift, 0, (instance->YPos * SMALL_CUBE_SIZE) + yshift);
			D3DMath_MatrixMultiply(ObjectMatrix, ScaleMatrix, TransMatrix);
			D3DUtil_SetRotateYMatrix(RotMatrix, spin);
			D3DMath_MatrixMultiply(ObjectMatrix, RotMatrix, ObjectMatrix);
			
			hResult = pd3dDevice->SetTransform(D3DTRANSFORMSTATE_WORLD, &ObjectMatrix);

			COMPONENT* pancomponent = DXUnits[instance->UnitID]->Components[PAN_INDEX];	//get pan component
			PRIMITIVE* panprim = pancomponent->Primitives[CurrentTexturePage];

			COMPONENT* pegcomponent = DXUnits[instance->UnitID]->Components[PEG_INDEX];	//get peg component
			PRIMITIVE* pegprim = pegcomponent->Primitives[CurrentTexturePage];

			if( ((panprim->VertexCount != 0) && (panprim->Vertices != NULL)) || ((pegprim->VertexCount != 0) && (pegprim->Vertices != NULL)) )
			{
				if((pegprim->VertexCount != 0) && (pegprim->Vertices != NULL) )
				{
					hResult = pd3dDevice->DrawPrimitive(D3DPT_TRIANGLELIST, fvf, (LPVOID)pegprim->Vertices, pegprim->VertexCount,NULL);
				}
				if((panprim->VertexCount != 0) && (panprim->Vertices != NULL) )
				{
					hResult = pd3dDevice->DrawPrimitive(D3DPT_TRIANGLELIST, fvf, (LPVOID)panprim->Vertices, panprim->VertexCount,NULL);
				}
			}
		}
	}
	spin += 0.01f;
}

void SetTrackScale(REAL scale)
{
	D3DUtil_SetScaleMatrix(TrackScaleMatrix, scale, scale, scale);
}

void DrawTrack(LPDIRECT3DDEVICE3 pd3dDevice, TRACKTHEME* theme)
{
	HRESULT hResult;

	DWORD fvf = D3DFVF_NORMAL | D3DFVF_XYZ | D3DFVF_TEX1;

	D3DMATRIX objectmatrix;

	TextureChanges = 0;
	for(CurrentTexturePage = 0; CurrentTexturePage < theme->TPageCount; CurrentTexturePage++)
	{
		SetCurrentTexture(pd3dDevice, UnitTex[CurrentTexturePage]);
		TextureChanges++;
		for(S32 y = 0; y < TrackData.Height; y++)
		{
			for(S32 x = 0; x < TrackData.Width; x++)
			{
				U32 g = (y * TrackData.Width) + x;
				U32 m = TrackData.Units[g].UnitID;

				COMPONENT* pancomponent = DXUnits[m]->Components[PAN_INDEX];	//get pan component
				PRIMITIVE* panprim = pancomponent->Primitives[CurrentTexturePage];

				COMPONENT* pegcomponent = DXUnits[m]->Components[PEG_INDEX];	//get peg component
				PRIMITIVE* pegprim = pegcomponent->Primitives[CurrentTexturePage];
				if( ((panprim->VertexCount != 0) && (panprim->Vertices != NULL)) || ((pegprim->VertexCount != 0) && (pegprim->Vertices != NULL)) )
				{
					hResult = pd3dDevice->SetTransform(D3DTRANSFORMSTATE_WORLD, &TrackData.Units[g].Matrix);
					if((pegprim->VertexCount != 0) && (pegprim->Vertices != NULL) )
					{
						REAL h = TrackData.Units[g].Elevation * -ElevationStep;
						if(h != ZERO)
						{
							DrawUnitRoot(pd3dDevice, h);
						}
						hResult = pd3dDevice->DrawPrimitive(D3DPT_TRIANGLELIST, fvf, (LPVOID)pegprim->Vertices, pegprim->VertexCount,NULL);
					}
					if((panprim->VertexCount != 0) && (panprim->Vertices != NULL) )
					{
						hResult = pd3dDevice->DrawPrimitive(D3DPT_TRIANGLELIST, fvf, (LPVOID)panprim->Vertices, panprim->VertexCount,NULL);
					}
				}
			}
		}
	}
}

void DrawPlacingScreen(LPDIRECT3DDEVICE3 pd3dDevice, TRACKTHEME* theme)
{
	static D3DVECTOR	facing_north(ZERO, -CameraDistance, -CameraDistance);
	static D3DVECTOR	facing_east (CameraDistance, -CameraDistance, ZERO);
	static D3DVECTOR	facing_south(ZERO, -CameraDistance, CameraDistance);
	static D3DVECTOR	facing_west (-CameraDistance, -CameraDistance, ZERO);
	static D3DVECTOR	up(ZERO, ONE, ZERO);
	D3DMATRIX			camera;
	D3DVECTOR			from;
	D3DVECTOR			at(ZERO, ZERO, ZERO);

	D3DMATRIX TransMatrix;
	D3DMATRIX IdentityMatrix;

	at.x = TrackCursor.X * SMALL_CUBE_SIZE;
	at.y = 0;
	at.z = TrackCursor.Y * SMALL_CUBE_SIZE;
	
	D3DUtil_SetTranslateMatrix(TransMatrix, at.x, 0, at.z);
	
	switch(PlacementViewpoint)
	{
		case PCD_NORTH:
			D3DMath_VectorMatrixMultiply(from, facing_north, TransMatrix);
		break;
		case PCD_EAST:
			D3DMath_VectorMatrixMultiply(from, facing_east, TransMatrix);
		break;
		case PCD_SOUTH:
			D3DMath_VectorMatrixMultiply(from, facing_south, TransMatrix);
		break;
		case PCD_WEST:
			D3DMath_VectorMatrixMultiply(from, facing_west, TransMatrix);
		break;
	}

	D3DUtil_SetViewMatrix(camera, from, at, up);
	pd3dDevice->SetTransform(D3DTRANSFORMSTATE_VIEW, &camera);

///	SetTrackScale(ONE);
	DrawTrack(pd3dDevice, theme);
	
	pd3dDevice->SetRenderState(D3DRENDERSTATE_TEXTUREHANDLE, 0);
	pd3dDevice->SetTransform(D3DTRANSFORMSTATE_WORLD, &TransMatrix);
	pd3dDevice->DrawIndexedPrimitive(D3DPT_LINELIST, D3DFVF_LVERTEX, (LPVOID)boxverts, 8, boxlines, 24, NULL);
}

void DrawAdjustmentScreen(LPDIRECT3DDEVICE3 pd3dDevice, TRACKTHEME* theme)
{
	static const REAL	adjustmentscale = Real(0.5f);
	static D3DVECTOR	up(ZERO, ZERO, -ONE);
	static D3DVECTOR	from(ZERO, -CameraDistance * REAL(6.5f), ZERO);
	static D3DVECTOR	at(ZERO, ZERO, ZERO);
	D3DMATRIX			camera;

	REAL trackwidth = TrackData.Width * SMALL_CUBE_SIZE;
	REAL trackheight = TrackData.Height * SMALL_CUBE_SIZE;
	
	at.x = from.x = (trackwidth / REAL(2.0f)) + (SMALL_CUBE_SIZE * 5);
	at.z = from.z = (trackheight / REAL(2.0f)) + (SMALL_CUBE_SIZE * 5);

	D3DUtil_SetViewMatrix(camera, from, at, up);
	pd3dDevice->SetTransform(D3DTRANSFORMSTATE_VIEW, &camera);

///	SetTrackScale(adjustmentscale);
	DrawTrack(pd3dDevice, theme);

	UseFontTexture(pd3dDevice);	// switch to the font texture as it contains the arrows we need

	if((GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0)
	{
		//plot the icons for shifting the track around
		DrawSprite(pd3dDevice, 290, 136, 353, 199, ZERO, HALF, 0.25f, 0.75f);
		DrawSprite(pd3dDevice, 472, 322, 535, 385, 0.25f, 0.75f, ZERO, ONE);
		DrawSprite(pd3dDevice, 290, 506, 353, 569, ZERO, 0.75f, 0.25f, HALF);
		DrawSprite(pd3dDevice, 102, 322, 165, 385, ZERO, 0.75f, 0.25f, ONE);
	}
	else
	{
		//plot the icons for resizing the track
		DrawSprite(pd3dDevice, 224, 120, 287, 183, HALF, HALF, 0.75f, 0.75f);
		DrawSprite(pd3dDevice, 376, 274, 439, 337, 0.75f, 0.75f, HALF, ONE);

		SetCurrentFont(pd3dDevice, &GameFont);
		char sizeinfo[40];
		sprintf(sizeinfo, "Width  : %d\n\rHeight : %d", TrackData.Width, TrackData.Height);
		DrawText(pd3dDevice, 440, FILE_WINDOW_TOP, sizeinfo);

	}

}

void DrawComponent(LPDIRECT3DDEVICE3 pd3dDevice, const COMPONENT* component)
{
	HRESULT hResult;
	DWORD	  fvf = D3DFVF_NORMAL | D3DFVF_XYZ | D3DFVF_TEX1;
	U32 p = component->PrimitiveCount;
	while(p--)
	{
		if(p == CurrentTexturePage)
		{
			PRIMITIVE* prim = component->Primitives[p];
			if((prim->VertexCount != 0) && (prim->Vertices != NULL) )
			{
				hResult = pd3dDevice->DrawPrimitive(D3DPT_TRIANGLELIST, fvf, (LPVOID)prim->Vertices, prim->VertexCount,NULL);
			}
		}
	}
}

void DrawUnitRoot(LPDIRECT3DDEVICE3 pd3dDevice, REAL h)
{
	HRESULT hResult;
	DWORD	  fvf = D3DFVF_NORMAL | D3DFVF_XYZ | D3DFVF_TEX1;

	D3DVERTEX* vert = &unitrootverts[0];
	vert->y = -h;
	vert++;
	vert->y = -h;
	vert++;
	vert->y = -h;
	vert++;
	vert->y = -h;
	vert++;
	vert->y = -h;
	vert++;
	vert->y = -h;
	vert++;
	vert->y = -h;
	vert++;
	vert->y = -h;

	hResult = pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, fvf, (LPVOID)unitrootverts, 16, unitrootindices, 24, NULL);
}

void InitializeCursorPrimitive(void)
{
	D3DCOLOR	diffuse, specular;

	diffuse = D3DRGB(box_color[0], box_color[1], box_color[2]);
	specular = D3DRGB(ZERO, ZERO, ZERO);
	REAL boxradius = SMALL_CUBE_SIZE / Real(1.6f);

	boxverts[0] =  D3DLVERTEX(D3DVECTOR(-boxradius, boxradius, boxradius), diffuse, specular, ZERO, ZERO);
	boxverts[1] =  D3DLVERTEX(D3DVECTOR(boxradius, boxradius, boxradius), diffuse, specular, ZERO, ZERO);
	boxverts[2] =  D3DLVERTEX(D3DVECTOR(boxradius, boxradius, -boxradius), diffuse, specular, ZERO, ZERO);
	boxverts[3] =  D3DLVERTEX(D3DVECTOR(-boxradius, boxradius, -boxradius), diffuse, specular, ZERO, ZERO);
	boxverts[4] =  D3DLVERTEX(D3DVECTOR(-boxradius, -boxradius, boxradius), diffuse, specular, ZERO, ZERO);
	boxverts[5] =  D3DLVERTEX(D3DVECTOR(boxradius, -boxradius, boxradius), diffuse, specular, ZERO, ZERO);
	boxverts[6] =  D3DLVERTEX(D3DVECTOR(boxradius, -boxradius, -boxradius), diffuse, specular, ZERO, ZERO);
	boxverts[7] =  D3DLVERTEX(D3DVECTOR(-boxradius, -boxradius, -boxradius), diffuse, specular, ZERO, ZERO);
}

void InitializeUnitRootPrimitive(void)
{
	D3DCOLOR	diffuse, specular;

	diffuse = D3DRGB(ONE, ONE, ONE);
	specular = D3DRGB(ZERO, ZERO, ZERO);

	REAL unitradius = SMALL_CUBE_SIZE / Real(2.0f);

	unitrootverts[0] =  D3DVERTEX(D3DVECTOR(-unitradius, unitradius, unitradius), D3DVECTOR(ZERO, ZERO, -ONE), ZERO, ZERO);
	unitrootverts[1] =  D3DVERTEX(D3DVECTOR(unitradius, unitradius, unitradius), D3DVECTOR(ZERO, ZERO, -ONE), ZERO, ZERO);
	unitrootverts[2] =  D3DVERTEX(D3DVECTOR(unitradius, unitradius, unitradius), D3DVECTOR(-ONE, ZERO, ZERO), ZERO, ZERO);
	unitrootverts[3] =  D3DVERTEX(D3DVECTOR(unitradius, unitradius, -unitradius), D3DVECTOR(-ONE, ZERO, ZERO), ZERO, ZERO);
	unitrootverts[4] =  D3DVERTEX(D3DVECTOR(unitradius, unitradius, -unitradius), D3DVECTOR(ZERO, ZERO, ONE), ZERO, ZERO);
	unitrootverts[5] =  D3DVERTEX(D3DVECTOR(-unitradius, unitradius, -unitradius), D3DVECTOR(ZERO, ZERO, ONE), ZERO, ZERO);
	unitrootverts[6] =  D3DVERTEX(D3DVECTOR(-unitradius, unitradius, -unitradius), D3DVECTOR(ONE, ZERO, ZERO), ZERO, ZERO);
	unitrootverts[7] =  D3DVERTEX(D3DVECTOR(-unitradius, unitradius, unitradius), D3DVECTOR(ONE, ZERO, ZERO), ZERO, ZERO);
	unitrootverts[8] =  D3DVERTEX(D3DVECTOR(-unitradius, 0, unitradius), D3DVECTOR(ZERO, ZERO, -ONE), ZERO, ZERO);
	unitrootverts[9] =  D3DVERTEX(D3DVECTOR(unitradius, 0, unitradius), D3DVECTOR(ZERO, ZERO, -ONE), ZERO, ZERO);
	unitrootverts[10] =  D3DVERTEX(D3DVECTOR(unitradius, 0, unitradius), D3DVECTOR(-ONE, ZERO, ZERO), ZERO, ZERO);
	unitrootverts[11] =  D3DVERTEX(D3DVECTOR(unitradius, 0, -unitradius), D3DVECTOR(-ONE, ZERO, ZERO), ZERO, ZERO);
	unitrootverts[12] =  D3DVERTEX(D3DVECTOR(unitradius, 0, -unitradius), D3DVECTOR(ZERO, ZERO, ONE), ZERO, ZERO);
	unitrootverts[13] =  D3DVERTEX(D3DVECTOR(-unitradius, 0, -unitradius), D3DVECTOR(ZERO, ZERO, ONE), ZERO, ZERO);
	unitrootverts[14] =  D3DVERTEX(D3DVECTOR(-unitradius, 0, -unitradius), D3DVECTOR(ONE, ZERO, ZERO), ZERO, ZERO);
	unitrootverts[15] =  D3DVERTEX(D3DVECTOR(-unitradius, 0, unitradius), D3DVECTOR(ONE, ZERO, ZERO), ZERO, ZERO);
}

void DrawMenuSprue(LPDIRECT3DDEVICE3 pd3dDevice, U16 x)
{
	static const float menutexturesize = 0.34375f;
	static const float menupixelsize = 176;

	SpriteClippingOn();
	x -= 22;
	U16 y = 32;
	SetCurrentFont(pd3dDevice, &SprueWireFont);
	DrawText(pd3dDevice, x, y,       "%'''(");
	DrawText(pd3dDevice, x, y + 32,  "- ./0");
	DrawText(pd3dDevice, x, y + 64,  "5 678");
	DrawText(pd3dDevice, x, y + 96,  "= >?@");
	DrawText(pd3dDevice, x, y + 128, "- ./0");
	DrawText(pd3dDevice, x, y + 160, "5 678");
	DrawText(pd3dDevice, x, y + 192,  "= >?@");
	DrawText(pd3dDevice, x, y + 224,  "- ./0");
	DrawText(pd3dDevice, x, y + 256,  "5 678");
	DrawText(pd3dDevice, x, y + 288,  "= >?@");
	DrawText(pd3dDevice, x, y + 320,  "- ./0");
	DrawText(pd3dDevice, x, y + 352,  "5 678");
	DrawText(pd3dDevice, x, y + 384,  "E&FGH");

	x += 80;
	y -= 4;
	SetCurrentFont(pd3dDevice, &SprueTextFont);
	DrawText(pd3dDevice, x, y + 48, "units");
	DrawText(pd3dDevice, x, y + 96, "pickups");
	DrawText(pd3dDevice, x, y + 144, "adjust");
	DrawText(pd3dDevice, x, y + 192, "save");
	DrawText(pd3dDevice, x, y + 240, "new");
	DrawText(pd3dDevice, x, y + 288, "load");
	DrawText(pd3dDevice, x, y + 336, "export");
	DrawText(pd3dDevice, x, y + 384, "quit");
	SpriteClippingOff();
}

void DrawSprite(LPDIRECT3DDEVICE3 pd3dDevice, REAL left, REAL top, REAL right, REAL bottom, REAL u0, REAL v0, REAL u1, REAL v1)
{
    // Get dimensions of the render surface 
    DDSURFACEDESC2 ddsd;
    ddsd.dwSize = sizeof(DDSURFACEDESC2);
    g_pFramework->GetRenderSurface()->GetSurfaceDesc(&ddsd);

    REAL xscale = ((REAL)g_DefaultScreenWidth) / ddsd.dwWidth; 
    REAL yscale = ((REAL)g_DefaultScreenHeight) / ddsd.dwHeight; 

	left /= xscale;
	right /= xscale;
	top /= yscale;
	bottom /= yscale;

	HRESULT hResult;

	SpriteVerts[1].sx = left;
	SpriteVerts[1].sy = top;
	SpriteVerts[1].tu = u0;
	SpriteVerts[1].tv = v0;
	
	SpriteVerts[3].sx = left;
	SpriteVerts[3].sy = bottom;
	SpriteVerts[3].tu = u0;
	SpriteVerts[3].tv = v1;
	
	SpriteVerts[0].sx = right;
	SpriteVerts[0].sy = top;
	SpriteVerts[0].tu = u1;
	SpriteVerts[0].tv = v0;
	
	SpriteVerts[2].sx = right;
	SpriteVerts[2].sy = bottom;
	SpriteVerts[2].tu = u1;
	SpriteVerts[2].tv = v1;
	hResult = pd3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DFVF_TLVERTEX, (LPVOID)SpriteVerts, 4,  D3DDP_DONOTUPDATEEXTENTS | SpriteClipOption);
}

void SpriteClippingOff(void)
{
	SpriteClipOption = D3DDP_DONOTCLIP;
}

void SpriteClippingOn(void)
{
	SpriteClipOption = 0;
}

void SetSpriteColor(U32 color)
{
	D3DCOLOR newcolor = color | 0xff000000l;
	SpriteVerts[0].color = newcolor;
	SpriteVerts[1].color = newcolor;
	SpriteVerts[2].color = newcolor;
	SpriteVerts[3].color = newcolor;
}

void DrawMainSprue(LPDIRECT3DDEVICE3 pd3dDevice)
{
	SetCurrentFont(pd3dDevice, &SprueWireFont);
	
	DrawText( pd3dDevice, 128, 0, "PVVVVVVVVVVVVVVO");
	DrawText( pd3dDevice, 0, 32, ")*VVXVVVVVVV+,");
	DrawText( pd3dDevice, 0, 64, "1            4");
	DrawText( pd3dDevice, 0, 96, "9:VVVVVVVVVV;<");
	DrawText( pd3dDevice, 608, 32, "M\n\rM\n\rM\n\rM\n\rM\n\rM\n\rM\n\rM\n\rM\n\rM\n\rM\n\rM\n\rM");
	DrawText( pd3dDevice, 384, 448, "_VVVVVVW");
	DrawText( pd3dDevice, 0, 416, "QRRRR   UVVVV`");
	DrawText( pd3dDevice, 0, 448, "YZ[\\\\]^VW");
	DrawText( pd3dDevice, 0, 128, "N\n\rM\n\rM\n\rM\n\rM\n\rM\n\rM\n\rM\n\rM");
}

void InitializeMenuPolys(void)
{
	MenuVerts[3] = D3DTLVERTEX(D3DVECTOR(640, 556, 0.1f), 0.5f,0xffffffffL, 0xff000000l, ZERO, 0.3125f);
	MenuVerts[1] = D3DTLVERTEX(D3DVECTOR(640, 44, 0.1f), 0.5f, 0xffffffffL, 0xff000000l, ZERO, ZERO);
	MenuVerts[2] = D3DTLVERTEX(D3DVECTOR(800, 556, 0.1f), 0.5f, 0xffffffffL, 0xff000000l, 0.3125f, 0.3125f);
	MenuVerts[0] = D3DTLVERTEX(D3DVECTOR(800, 44, 0.1f), 0.5f, 0xffffffffL, 0xff000000l, 0.3125f, ZERO);

	MenuCursorVerts[3] = D3DTLVERTEX(D3DVECTOR(400, 32, 0.1f), 0.5f,0xffffffffL, 0xff000000l, ZERO, 0.125f);
	MenuCursorVerts[1] = D3DTLVERTEX(D3DVECTOR(400, 0, 0.1f), 0.5f, 0xffffffffL, 0xff000000l, ZERO, ZERO);
	MenuCursorVerts[2] = D3DTLVERTEX(D3DVECTOR(432, 32, 0.1f), 0.5f, 0xffffffffL, 0xff000000l, 0.125f, 0.125f);
	MenuCursorVerts[0] = D3DTLVERTEX(D3DVECTOR(432, 0, 0.1f), 0.5f, 0xffffffffL, 0xff000000l, 0.125f, ZERO);

	SpriteVerts[3] = D3DTLVERTEX(D3DVECTOR(400, 32, 0.1f), 0.5f,0xffffffffL, 0xff000000l, ZERO, 0.125f);
	SpriteVerts[1] = D3DTLVERTEX(D3DVECTOR(400, 0, 0.1f), 0.5f, 0xffffffffL, 0xff000000l, ZERO, ZERO);
	SpriteVerts[2] = D3DTLVERTEX(D3DVECTOR(432, 32, 0.1f), 0.5f, 0xffffffffL, 0xff000000l, 0.125f, 0.125f);
	SpriteVerts[0] = D3DTLVERTEX(D3DVECTOR(432, 0, 0.1f), 0.5f, 0xffffffffL, 0xff000000l, 0.125f, ZERO);
}

void DrawCursorCoin(LPDIRECT3DDEVICE3 pd3dDevice, U16 x, U16 y, U16 frame)
{
	static const float cointexturesize = 0.1875f;
	static const float coinpixelsize = 48;

	if(x < g_DefaultScreenWidth)
	{
		float coinpixelwidth = coinpixelsize;
		if((x + coinpixelsize) > g_DefaultScreenWidth)
		{
			coinpixelwidth = g_DefaultScreenWidth - x;
		}

		float cointexturewidth = cointexturesize * (coinpixelwidth / coinpixelsize);
		frame = 15 - frame;
		REAL u = (frame % 4) * cointexturesize;
		REAL v = (frame / 4) * cointexturesize;

		DrawSprite(pd3dDevice, x, y, x + coinpixelwidth, y + coinpixelsize, u, v, u + cointexturewidth, v + cointexturesize);
	}
}

void DrawPopupMenu(LPDIRECT3DDEVICE3 pd3dDevice)
{
	static const U16 buttonspacing = 48;
	static const float coinpixelsize = 48;
	float cursorcolumn = (g_DefaultScreenWidth - PopupSlideDistance) + PopupMenuPosition;
	float cursorrow = (g_DefaultScreenHeight - ((7 * buttonspacing) + 48)) / 2;

	U16 frame;
	if((PopupState == PMS_ACTIVE) || (PopupState == PMS_APPEARING) || (PopupState == PMS_DISAPPEARING))
	{
		UseButtonTexture(pd3dDevice);
		for(U16 s = 0; s < ESS_PLACING_MODULE; s++)
		{
			frame = 8;
			if(s == PopupCursorPosition)
			{
				frame = 0;
			}
			DrawCursorCoin(pd3dDevice, cursorcolumn, (s * buttonspacing) + cursorrow, frame);
		}
		DrawMenuSprue(pd3dDevice, cursorcolumn);
	}
	if(PopupState == PMS_MOVING_UP)
	{
		UseButtonTexture(pd3dDevice);
		for(U16 s = 0; s < ESS_PLACING_MODULE; s++)
		{
			frame = 8;
			if(s == PopupCursorPosition + 1)
			{
				frame = PopupCursorFrame;
			}
			else
			{
				if(s == PopupCursorPosition)
				{
					frame = PopupCursorFrame + 8;
				}
			}
			DrawCursorCoin(pd3dDevice, cursorcolumn, (s * buttonspacing) + cursorrow, frame);
		}
		DrawMenuSprue(pd3dDevice, cursorcolumn);
	}
	if(PopupState == PMS_MOVING_DOWN)
	{
		UseButtonTexture(pd3dDevice);
		for(U16 s = 0; s < ESS_PLACING_MODULE; s++)
		{
			frame = 8;
			if(s == PopupCursorPosition - 1)
			{
				frame = 15 - PopupCursorFrame;
			}
			else
			{
				if(s == PopupCursorPosition)
				{
					frame = 7 - PopupCursorFrame;
				}
			}
			DrawCursorCoin(pd3dDevice, cursorcolumn, (s * buttonspacing) + cursorrow, frame);
		}
		DrawMenuSprue(pd3dDevice, cursorcolumn);
	}
}

void DrawFrame(LPDIRECT3DDEVICE3 pd3dDevice, REAL xpix, REAL ypix, U16 cellwidth, U16 cellheight)
{
	static const spritesize = 32;
	DrawSprite(pd3dDevice, xpix, ypix, xpix + spritesize, ypix + spritesize, ZERO, 0.8125f, 0.0625f, 0.875f);		//top left corner
	xpix += 32;
	U16 count = cellwidth - 2;
	while(count--)
	{
		DrawSprite(pd3dDevice, xpix, ypix, xpix + spritesize, ypix + spritesize, 0.0625f, 0.8125f, 0.125f, 0.875f);		//top bar
		xpix += 32;
	}
	DrawSprite(pd3dDevice, xpix, ypix, xpix + spritesize, ypix + spritesize, 0.0625f, 0.8125f, ZERO, 0.875f);		//top right corner
	ypix += 32;

	count = cellheight - 2;
	while(count--)
	{
		DrawSprite(pd3dDevice, xpix, ypix, xpix + spritesize, ypix + spritesize, 0.0625f, 0.875f, ZERO, 0.9375f);		//right bar
		ypix += 32;
	}
	DrawSprite(pd3dDevice, xpix, ypix, xpix + spritesize, ypix + spritesize, 0.0625f, 0.875f, ZERO, 0.8125f);		//bottom right corner
	xpix -= 32;
	count = cellwidth - 2;
	while(count--)
	{
		DrawSprite(pd3dDevice, xpix, ypix, xpix + spritesize, ypix + spritesize, 0.0625f, 0.875f, 0.125f, 0.8125f);		//bottom bar
		xpix -= 32;
	}
	DrawSprite(pd3dDevice, xpix, ypix, xpix + spritesize, ypix + spritesize, ZERO, 0.875f, 0.0625f, 0.8125f);		//bottom left corner
	ypix -= 32;
	count = cellheight - 2;
	while(count--)
	{
		DrawSprite(pd3dDevice, xpix, ypix, xpix + spritesize, ypix + spritesize, ZERO, 0.875f, 0.0625f, 0.9375f);		//left bar
		ypix -= 32;
	}
}

