/////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////
#include "revolt.h"
#include "draw.h"
#include "main.h"
#include "text.h"
#include "input.h"
#include "play.h"
#include "model.h"
#include "world.h"
#include "texture.h"
#include "geom.h"
#include "camera.h"
#include "Particle.h"
#include "NewColl.h"
#include "Body.h"
#include "Aerial.h"
#include "Wheel.h"
#include "DrawObj.h"
#include "visibox.h"
#include "level.h"
#include "ctrlread.h"
#include "object.h"
#include "player.h"
#include "gameloop.h"
#include "registry.h"
#include "ghost.h"
#include "ai.h"
#ifdef _PC
#include "spark.h"
#endif
#include "timing.h"

#include "Text.h"
#include "TitleScreen.h"
#include "Menu.h"
#include "MenuData.h"
#include "MenuText.h"


/////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////
extern	char MainMenuAllowed[];
extern	char *MainMenuText[];
extern	char *EditMenuText[];
extern	char *NoYesText[];

t_Menu		*gMenu;
t_Flag		gFlag;

#define	MENU_LIGHT_MAX	3
t_Light		gLight[MENU_LIGHT_MAX];

MODEL		gModel[1];


/////////////////////////////////////////////////////////////////////////////////
// Camera Info
/////////////////////////////////////////////////////////////////////////////////
#define TS_COORD_SCALE		(Real(200))

t_CameraPos	gCameraPositions[] =
{
	{Real(-0.002),Real(-29.79),Real(1.885), Real(-0.002),Real(0.066),Real(4.37)},
	{Real(9.587),Real(7.127),Real(2.395), Real(16.676),Real(12.887),Real(1.849)},
	{Real(11.37),Real(-19.888),Real(5.72), Real(26.234),Real(-12.128),Real(3.72)},
	{Real(12.36),Real(-14.955),Real(5.257), Real(26.913),Real(-21.921),Real(3.951)},
	{Real(-13.194),Real(-1.497),Real(3.862), Real(-22.397),Real(-4.682),Real(6.635)},
	{Real(-15.784),Real(0.415),Real(2.86), Real(-23.098),Real(0.418),Real(4.835)},
	{Real(-14.642),Real(4.748),Real(5.062), Real(-21.258),Real(3.829),Real(5.873)},
	{Real(-14.086),Real(7.223),Real(3.014), Real(-22.498),Real(7.924),Real(4.835)},
	{Real(0.0),Real(0.042),Real(63.661), Real(0.0),Real(-0.361),Real(-0.748)},

//	{Real(9.587),Real(7.127),Real(2.395), Real(16.676),Real(12.887),Real(1.849)},
};

enum TS_CAMPOS
{
	TS_CAMPOS_START,
	TS_CAMPOS_CAR_SELECT,
	TS_CAMPOS_TRACK_SELECT,
	TS_CAMPOS_USER_SELECT,
	TS_CAMPOS_TROPHY_1,
	TS_CAMPOS_TROPHY_2,
	TS_CAMPOS_TROPHY_3,
	TS_CAMPOS_TROPHY_4,
	TS_CAMPOS_OVER_VIEW,
	TS_CAMPOS_NUM
};



/////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////
void GoTitleScreen(void)
{
// init D3D
	if (!InitD3D(DrawDevices[RegistrySettings.DrawDevice].DisplayMode[DisplayModeCount].Width, DrawDevices[RegistrySettings.DrawDevice].DisplayMode[DisplayModeCount].Height, DrawDevices[RegistrySettings.DrawDevice].DisplayMode[DisplayModeCount].Bpp, 0))
	{
		QuitGame = TRUE;
		return;
	}

	GetTextureFormat(RegistrySettings.TextureBpp);
	InitTextures();

// setup states

	SetupDxState();

// set geom vars

	RenderSettings.GeomPers = BaseGeomPers;
//	SetNearFar(48.0f, 4096.0f);
	SetNearFar(48.0f, 60000.0f);
	SetViewport(0, 0, (float)ScreenXsize, (float)ScreenYsize, RenderSettings.GeomPers);

// setup main menu

	LoadMipTexture("gfx\\font1.bmp", TPAGE_FONT, 256, 256, 0, 1);

	LoadBitmap("gfx\\title.bmp", &TitleHbm);

	MenuCount = 0;
	Event = TitleScreen;


// Setup flag
	InitFlag(&gFlag, 32,32, 10,10);

// Setup light #0
	SetLightPos(&gLight[0], 100,-100,-100);
	SetLightDir(&gLight[0], -0.5,1,0.5);
	SetLightPower(&gLight[0], 15);
	SetLightStrength(&gLight[0], 1000);
	SetLightRange(&gLight[0], 250);

// Setup light #1
	SetLightPos(&gLight[1], -200,-100,0);
	SetLightDir(&gLight[1], 1,1,0);
	SetLightPower(&gLight[1], 25);
	SetLightStrength(&gLight[1], 1250);
	SetLightRange(&gLight[1], 300);

// Setup light #2
	SetLightPos(&gLight[2], 0,100,150);
	SetLightDir(&gLight[2], 0,-4,-2);
	SetLightPower(&gLight[2], 30);
	SetLightStrength(&gLight[2], 500);
	SetLightRange(&gLight[2], 500);

//		SetLightStrength(&gLight[0], 0);
//		SetLightStrength(&gLight[2], 0);

// Setup menu options
	gMenu = &gMainOptions_Menu;
	MenuInit(gMenu, NULL);


// Model

	//	LoadModel("models\\spaceman.m", &gModel[0], 0,1, LOADMODEL_OFFSET_TPAGE, 100);
//	LoadModel("models\\beachball.m", &gModel[0], 0,1, LOADMODEL_OFFSET_TPAGE, 100);
	LoadModel("models\\trolley.m", &gModel[0], 0,1, LOADMODEL_OFFSET_TPAGE, 100);


// init level
	GameSettings.Level = GetLevelNum("FRONTEND");
	if (GameSettings.Level < 0)
		GameSettings.Level = 12;
	LEV_InitLevel();

	ts_InitCameraPositions();

//	Camera[CameraCount].WPos.v[0] = 200 * 9.587;	//0;
//	Camera[CameraCount].WPos.v[1] = 200 * -2.395;	//-4000;
//	Camera[CameraCount].WPos.v[2] = 200 * 7.127;	//0;
}

/////////////////////////////////////////////////////////////////////////////////
// ReleaseTitleScreen()
/////////////////////////////////////////////////////////////////////////////////
void ReleaseTitleScreen(void)
{
	ReleaseFlag(&gFlag);
	FreeModel(&gModel[0], 1);
	LEV_EndLevel();
}


/////////////////////////////////////////////////////////////////////////////////
// ts_InitCameraPositions()
/////////////////////////////////////////////////////////////////////////////////
void ts_InitCameraPositions(void)
{
	t_CameraPos*	pCamPos;
	MAT				matrix;
	REAL			y;
	int				i;

	pCamPos = gCameraPositions;
	for (i = 0; i < TS_CAMPOS_NUM; i++)
	{
		y = pCamPos->eye.v[1];
		pCamPos->eye.v[0] = MulScalar(TS_COORD_SCALE, pCamPos->eye.v[0]);
		pCamPos->eye.v[1] = MulScalar(TS_COORD_SCALE, -pCamPos->eye.v[2]);
		pCamPos->eye.v[2] = MulScalar(TS_COORD_SCALE, y);

		y = pCamPos->focus.v[1];
		pCamPos->focus.v[0] = MulScalar(TS_COORD_SCALE, pCamPos->focus.v[0]);
		pCamPos->focus.v[1] = MulScalar(TS_COORD_SCALE, -pCamPos->focus.v[2]);
		pCamPos->focus.v[2] = MulScalar(TS_COORD_SCALE, y);

		BuildLookMatrixForward(&pCamPos->eye, &pCamPos->focus, &matrix);
		MatToQuat(&matrix, &pCamPos->quat);

		pCamPos++;
	}
}


/////////////////////////////////////////////////////////////////////////////////
// TitleScreen()
/////////////////////////////////////////////////////////////////////////////////
void TitleScreen(void)
{

// buffer flip / clear

	CheckSurfaces();
	FlipBuffers();
//	ClearBuffers();

// set and clear viewport
//	SetViewport(Camera[CameraCount].X, Camera[CameraCount].Y, Camera[CameraCount].Xsize, Camera[CameraCount].Ysize, BaseGeomPers + Camera[CameraCount].Lens);
	InitRenderStates();


// reset 3d poly list
	Reset3dPolyList();
	InitPolyBuckets();

// Input
	ReadMouse();
	ReadKeyboard();
	UpdateTimeFactor();


// show menu

	D3Ddevice->BeginScene();

//	BlitBitmap(TitleHbm, &BackBuffer);

	BeginTextState();


// Gazza
	gMenu = MenuProcess(gMenu);



// update camera + set camera view vars
	int CameraCount = 0;

	UpdateCamera(&Camera[CameraCount]);

		static VEC camFocus = {5000,5000,0};
//		static VEC camFocus = {200*16.676, -200*1.849, 200*12.887};

		if (Keys[DIK_NUMPAD4])
			camFocus.v[0] -= 100;
		if (Keys[DIK_NUMPAD6])
			camFocus.v[0] += 100;
		if (Keys[DIK_NUMPAD2])
			camFocus.v[2] -= 100;
		if (Keys[DIK_NUMPAD8])
			camFocus.v[2] += 100;

		if (Keys[DIK_NUMPAD7])
			Camera[CameraCount].WPos.v[1] -= 100;
		if (Keys[DIK_NUMPAD9])
			Camera[CameraCount].WPos.v[1] += 100;

//		BuildLookMatrixForward(&Camera[CameraCount].WPos, &camFocus, &Camera[CameraCount].WMatrix);

			t_CameraPos* pCamPos;
			pCamPos = &gCameraPositions[0];
			Camera[CameraCount].WPos.v[0] = pCamPos->eye.v[0];
			Camera[CameraCount].WPos.v[1] = pCamPos->eye.v[1];
			Camera[CameraCount].WPos.v[2] = pCamPos->eye.v[2];
			camFocus.v[0] = pCamPos->focus.v[0];
			camFocus.v[1] = pCamPos->focus.v[1];
			camFocus.v[2] = pCamPos->focus.v[2];
//			BuildLookMatrixForward(&Camera[CameraCount].WPos, &camFocus, &Camera[CameraCount].WMatrix);
			QuatToMat(&pCamPos->quat, &Camera[CameraCount].WMatrix);

// set and clear viewport

	SetViewport(Camera[CameraCount].X, Camera[CameraCount].Y, Camera[CameraCount].Xsize, Camera[CameraCount].Ysize, BaseGeomPers + Camera[CameraCount].Lens);
	InitRenderStates();
	ClearBuffers();

	SetCameraView(&Camera[CameraCount].WMatrix, &Camera[CameraCount].WPos, Camera[CameraCount].Shake);
	SetCameraVisiMask(&Camera[CameraCount].WPos);

// render opaque polys

	ResetSemiList();

//		if (DrawGridCollSkin)
//		{
//			DrawGridCollPolys(PosToCollGrid(&PLR_LocalPlayer->car.Body->Centre.Pos));
//		}
//		else
	{
		DrawWorld();
		DrawInstances();
	}

//		DrawObjects();
//		DrawAllCars();
//		Draw3dPolyList();



// TEST
//	RenderFlag(&gFlag);


// TEST
	MAT camMat;
//	static VEC camPos = {0,0,-750};
	static VEC camPos = {0,0,-200};
	RotMatrixZYX(&camMat, (REAL)0/360,(REAL)0/360,(REAL)0/360);
	SetCameraView(&camMat, &camPos, 0);

	static VEC modelRot = {0,0,0};
	MAT	modelMat;
	VEC	modelPos = {0,0,0};
	RotMatrixZYX(&modelMat, (REAL)modelRot.v[0]/360,(REAL)modelRot.v[1]/360,(REAL)modelRot.v[2]/360);
//	DrawModel(&gModel[0], &modelMat, &modelPos, MODEL_LIT);

	modelRot.v[0] += 0;
	modelRot.v[1] += 10;
	modelRot.v[2] += 0;


// set eye mat + trans
//	MulMatrix(&ViewMatrixScaled, worldmat, &eyematrix);
//	RotTransVector(&ViewMatrixScaled, &ViewTransScaled, worldpos, &eyetrans);





// Flush poly buckets
	FlushPolyBuckets();
	FlushEnvBuckets();


// Render menu
	if (gMenu)
	{
		BeginTextState();
		MenuRender(gMenu);
	}

// End scene
	D3Ddevice->EndScene();


// Quit ?
	if (!gMenu)
	{
		ReleaseTitleScreen();
		Event = GoFront;
	}
}


/////////////////////////////////////////////////////////////////////////////////
// Light
/////////////////////////////////////////////////////////////////////////////////
void SetLightPos(t_Light* pLight, REAL x, REAL y, REAL z)
{
	pLight->pos.v[0] = x;
	pLight->pos.v[1] = y;
	pLight->pos.v[2] = z;
}

void SetLightDir(t_Light* pLight, REAL x, REAL y, REAL z)
{
	REAL l = MulScalar(x, x) + MulScalar(y, y) + MulScalar(z, z);
	if (l != (ONE*ONE))
	{
		l = ONE / (REAL)sqrt(l);
		x = MulScalar(x, l);
		y = MulScalar(y, l);
		z = MulScalar(z, l);
	}

	pLight->dir.v[0] = x;
	pLight->dir.v[1] = y;
	pLight->dir.v[2] = z;
}

void SetLightPower(t_Light* pLight, REAL power)
{
	pLight->power = power;
}

void SetLightStrength(t_Light* pLight, REAL strength)
{
	pLight->strength = strength;
}

void SetLightRange(t_Light* pLight, REAL range)
{
	pLight->range = range;
	pLight->range2 = MulScalar(range, range);
	pLight->rangeInv = DivScalar(ONE, range);
}


/////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////
void InitFlag(t_Flag *pFlag, int w, int h, REAL sX, REAL sY)
{
	REAL	x, y, z, Dx, Dy, Dz;
	int		cX, cY;

// Setup variables
	pFlag->w = w;
	pFlag->h = h;
	pFlag->pPoints = (t_FlagPoint*)malloc(w * h * sizeof(t_FlagPoint));

// Create points
	Dx = sX;
	Dy = 0;
	Dz = -sY;

	y = 0;
	z = (h >> 1) * -Dz;

	for (cY = 0; cY < h; cY++)
	{
		x = (w >> 1) * -Dx;

		for (cX = 0; cX < w; cX++)
		{
			pFlag->pPoints[cX + (cY * w)].pos.v[0] = x;
			pFlag->pPoints[cX + (cY * w)].pos.v[1] = y;
			pFlag->pPoints[cX + (cY * w)].pos.v[2] = z;

			pFlag->pPoints[cX + (cY * w)].u = 0;
			pFlag->pPoints[cX + (cY * w)].v = 0;

			x += Dx;
		}

		z += Dz;
	}
}

/////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////
void ReleaseFlag(t_Flag *pFlag)
{
	if (pFlag->pPoints)
	{
		free(pFlag->pPoints);
		pFlag->pPoints = NULL;
	}
}


/////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////
void RenderFlag(t_Flag *pFlag)
{
	VERTEX_TEX1	v[64*64*2];
	VERTEX_TEX1 *pDV;
	t_FlagPoint	*pSV;
	int			cX, cY;
	int			cL;
	int			i;
	static		rX = 25;
	static		rY = 45;
	static		rZ = 0;

//	SetCameraView(&Camera[CameraCount].WMatrix, &Camera[CameraCount].WPos, Camera[CameraCount].Shake);
//	SetCameraVisiMask(&Camera[CameraCount].WPos);

	ZBUFFER_ON();
	ZWRITE_ON();
	ZCMP(D3DCMP_LESSEQUAL);
//	FOG_ON();
//	WIREFRAME_ON();
	CULL_OFF();

	MAT	viewMatrix;
	VEC viewPos;
	VEC tempPos;
	VEC tempPos2;

	tempPos.v[0] = 0;
	tempPos.v[1] = -150;
	tempPos.v[2] = -750;

	tempPos2.v[0] = 0;
	tempPos2.v[1] = 0;
	tempPos2.v[2] = 0;

//		if (Keys[DIK_UP] && !LastKeys[DIK_UP])
		if (Keys[DIK_NUMPAD8])
			rX -= 10;
		if (Keys[DIK_NUMPAD2])
			rX += 10;
		if (Keys[DIK_NUMPAD4])
			rY -= 10;
		if (Keys[DIK_NUMPAD6])
			rY += 10;

//		if (Keys[DIK_NUMPADMINUS])
		if (Keys[DIK_COMMA])
			gLight[0].strength -= 2;
//		if (Keys[DIK_NUMPADMINUS])
		if (Keys[DIK_PERIOD])
			gLight[0].strength += 2;

		if (Keys[DIK_LBRACKET])
			gLight[0].power -= 1;
		if (Keys[DIK_RBRACKET])
			gLight[0].power += 1;
		if (gLight[0].power < 1)
			gLight[0].power = 1;

	RotMatrixZYX(&viewMatrix, (REAL)rX/360,(REAL)rY/360,(REAL)rZ/360);
	RotTransVector(&viewMatrix, &tempPos2, &tempPos, &viewPos);
	SetCameraView(&viewMatrix, &viewPos, 0);


// Transform vertices
	pSV = pFlag->pPoints;
	pDV = v;
	for (cY = 0; cY < pFlag->h; cY++)
	{
		for (cX = 0; cX < pFlag->w; cX++)
		{
			static VEC normal = {0,-1,0};
			VEC	lightPullVec[MENU_LIGHT_MAX];

			tempPos.v[0] = 0;
			tempPos.v[1] = 0;
			tempPos.v[2] = 0;
			for (cL = 0; cL < MENU_LIGHT_MAX; cL++)
			{
				ts_ApplyLightPull(&gLight[cL], &normal, &pSV->pos, &lightPullVec[cL]);
				VecPlusEqVec(&tempPos, &lightPullVec[cL]);
			}

			tempPos.v[0] = MulScalar(tempPos.v[0], ONE/MENU_LIGHT_MAX);
			tempPos.v[1] = MulScalar(tempPos.v[1], ONE/MENU_LIGHT_MAX);
			tempPos.v[2] = MulScalar(tempPos.v[2], ONE/MENU_LIGHT_MAX);

			RotTransPersVector(&ViewMatrixScaled, &ViewTransScaled, &tempPos, (float*)&pDV->sx);			

//			RotTransPersVector(&ViewMatrixScaled, &ViewTransScaled, &pSV->pos, (float*)&pDV->sx);			

			pDV->tu			= pSV->u;
			pDV->tv			= pSV->v;
			pDV->color		= 0xFFFFFF;
			pDV->specular	= 0x000000;

	#if 1
#if 1
/*
			int r,g,b;
			if (normal.v[0] > 0)	r = (int)(normal.v[0] * 255);
			else					r = 0;
			if (normal.v[1] > 0)	g = (int)(normal.v[1] * 255);
			else					g = 0;
			if (normal.v[2] > 0)	b = (int)(normal.v[2] * 255);
			else					b = 0;
				g = b = 0;
			pDV->color		= r | (g << 8) | (b << 16);
*/
			int	rgb[3];
			for (i = 0; i < 3; i++)
			{
//				rgb[i] = (int)((((tempPos.v[i] - viewPos.v[i]) / 320) * 255) + 128);
				rgb[i] = (int)((((tempPos.v[i]) / 320) * 255) + 128);

				if (rgb[i] < 0)		rgb[i] = 0;
				if (rgb[i] > 255)	rgb[i] = 255;
			}

				//rgb[0] = rgb[1] = rgb[2];

			pDV->color = rgb[0] | (rgb[1] << 8) | (rgb[2] << 16);
#else
			int f = (cX & 1) + ((cY & 1) << 1);
			switch (f)
			{
				case 0:
					pDV->color = 0x000000;
					break;
				case 1:
					pDV->color = 0x0000FF;
					break;
				case 2:
					pDV->color = 0x00FF00;
					break;
				case 3:
					pDV->color = 0xFF0000;
					break;
			}
#endif
	#endif

			pSV++;
			pDV++;
		}
	}


// Render tris
	SET_TPAGE(-1);

	pDV = v;
	for (cY = 0; cY < pFlag->h-1; cY++)
	{
		for (cX = 0; cX < pFlag->w-1; cX++)
		{
			DrawTriClip(&pDV[0], &pDV[1], &pDV[pFlag->w+1]);
			DrawTriClip(&pDV[0], &pDV[pFlag->w+1], &pDV[pFlag->w]);
			pDV++;
		}

		pDV++;
	}
}


/////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////
void ts_ApplyLightPull(t_Light* pLight, VEC* pN, VEC* pS, VEC* pE)
{
	REAL	dotP;
	VEC		delta;
	REAL	l, lInv;
	REAL	strength;

	delta.v[0] = pS->v[0] - pLight->pos.v[0];
	delta.v[1] = pS->v[1] - pLight->pos.v[1];
	delta.v[2] = pS->v[2] - pLight->pos.v[2];
	l = MulScalar(delta.v[0], delta.v[0]) +
		MulScalar(delta.v[1], delta.v[1]) +
		MulScalar(delta.v[2], delta.v[2]);

	if (l >= pLight->range2)
	{
		pE->v[0] = pS->v[0];
		pE->v[1] = pS->v[1];
		pE->v[2] = pS->v[2];
		return;
	}

	l = (REAL)sqrt(l);
	lInv = ONE / l;
	delta.v[0] = MulScalar(delta.v[0], lInv);
	delta.v[1] = MulScalar(delta.v[1], lInv);
	delta.v[2] = MulScalar(delta.v[2], lInv);

	dotP = MulScalar(pLight->dir.v[0], delta.v[0]) +
		   MulScalar(pLight->dir.v[1], delta.v[1]) +
		   MulScalar(pLight->dir.v[2], delta.v[2]);

	if (dotP > 0)
	{
		strength = MulScalar(pLight->strength, ONE - MulScalar(l, pLight->rangeInv));
		dotP = ts_Power(dotP, pLight->power);
		dotP = MulScalar(dotP, strength);

		pE->v[0] = pS->v[0] - MulScalar(dotP, pLight->dir.v[0]);
		pE->v[1] = pS->v[1] - MulScalar(dotP, pLight->dir.v[1]);
		pE->v[2] = pS->v[2] - MulScalar(dotP, pLight->dir.v[2]);
	}
	else
	{
		pE->v[0] = pS->v[0];
		pE->v[1] = pS->v[1];
		pE->v[2] = pS->v[2];
	}

	CopyVec(&delta, pN);
}


/////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////
REAL ts_Power(REAL value, REAL power)
{
	REAL denominator = power - MulScalar(power, value) + value;
	if (denominator == 0)
		return 0;

	return (value / denominator);
}
