
#ifndef DX_H
#define DX_H

// macros

#define MAX_DISPLAY_MODES 32
#define MAX_DISPLAY_MODE_TEXT 32
#define MAX_DRAW_DEVICES 3
#define MAX_DRAW_DEVICE_NAME 128

enum {
	GAMMA_UNAVAILABLE,
	GAMMA_AVAILABLE,
	GAMMA_AUTO,
};

// render state macros

#if SCREEN_DEBUG

#define SET_RENDER_STATE(_s, _v) \
{ \
	D3Ddevice->SetRenderState((_s), (_v)); \
	RenderStateChange++; \
}

#define SET_STAGE_STATE(_t, _s, _v) \
{ \
	D3Ddevice->SetTextureStageState((_t), (_s), (_v)); \
	RenderStateChange++; \
}

#define SET_TEXTURE(_t, _tex) \
{ \
	D3Ddevice->SetTexture((_t), (_tex)); \
	TextureStateChange++; \
}

#else

#define SET_RENDER_STATE(_s, _v) \
{ \
	D3Ddevice->SetRenderState(_s, _v); \
}

#define SET_STAGE_STATE(_t, _s, _v) \
{ \
	D3Ddevice->SetTextureStageState((_t), (_s), (_v)); \
}

#define SET_TEXTURE(_t, _tex) \
{ \
	D3Ddevice->SetTexture((_t), (_tex)) \
}

#endif

#define MIPMAP_LODBIAS(_n) \
{ \
	float _f = _n; \
	SET_STAGE_STATE(0, D3DTSS_MIPMAPLODBIAS, *(DWORD*)&_f); \
	SET_STAGE_STATE(1, D3DTSS_MIPMAPLODBIAS, *(DWORD*)&_f); \
}

#define TEXTURE_ADDRESS(_w) \
{ \
	SET_STAGE_STATE(0, D3DTSS_ADDRESS, _w); \
	SET_STAGE_STATE(1, D3DTSS_ADDRESS, _w); \
}

#define FOG_ON() \
{ \
	if (!RenderFog) SET_RENDER_STATE(D3DRENDERSTATE_FOGENABLE, (RenderFog = TRUE)); \
}

#define FOG_OFF() \
{ \
	if (RenderFog) SET_RENDER_STATE(D3DRENDERSTATE_FOGENABLE, (RenderFog = FALSE)); \
}

#define ALPHA_ON() \
{ \
	if (!RenderAlpha) SET_RENDER_STATE(D3DRENDERSTATE_ALPHABLENDENABLE, (RenderAlpha = TRUE)); \
}

#define ALPHA_OFF() \
{ \
	if (RenderAlpha) SET_RENDER_STATE(D3DRENDERSTATE_ALPHABLENDENABLE, (RenderAlpha = FALSE)); \
}

#define ALPHA_SRC(_a) \
{ \
	if (RenderAlphaSrc != _a) SET_RENDER_STATE(D3DRENDERSTATE_SRCBLEND, (RenderAlphaSrc = _a)); \
}

#define ALPHA_DEST(_a) \
{ \
	if (RenderAlphaDest != _a) SET_RENDER_STATE(D3DRENDERSTATE_DESTBLEND, (RenderAlphaDest = _a)); \
}

#define ZWRITE_ON() \
{ \
	if (!RenderZwrite) SET_RENDER_STATE(D3DRENDERSTATE_ZWRITEENABLE, (RenderZwrite = TRUE)); \
}

#define ZWRITE_OFF() \
{ \
	if (RenderZwrite) SET_RENDER_STATE(D3DRENDERSTATE_ZWRITEENABLE, (RenderZwrite = FALSE)); \
}

#define ZBUFFER_ON() \
{ \
	if (RenderZbuffer != D3DZB_TRUE) SET_RENDER_STATE(D3DRENDERSTATE_ZENABLE, (RenderZbuffer = D3DZB_TRUE)); \
}

#define ZBUFFER_OFF() \
{ \
	if (RenderZbuffer != D3DZB_FALSE) SET_RENDER_STATE(D3DRENDERSTATE_ZENABLE, (RenderZbuffer = D3DZB_FALSE)); \
}

#define ZCMP(_c) \
{ \
	if (RenderZcmp != _c) SET_RENDER_STATE(D3DRENDERSTATE_ZFUNC, (RenderZcmp = _c)); \
}

#define PERSPECTIVE_ON() \
	SET_RENDER_STATE(D3DRENDERSTATE_TEXTUREPERSPECTIVE, DxState.Perspective)

#define PERSPECTIVE_OFF() \
	SET_RENDER_STATE(D3DRENDERSTATE_TEXTUREPERSPECTIVE, FALSE)

#define DITHER_ON() \
	SET_RENDER_STATE(D3DRENDERSTATE_DITHERENABLE, DxState.Dither)

#define DITHER_OFF() \
	SET_RENDER_STATE(D3DRENDERSTATE_DITHERENABLE, FALSE)

#define COLORKEY_ON() \
	SET_RENDER_STATE(D3DRENDERSTATE_COLORKEYENABLE, DxState.ColorKey)

#define COLORKEY_OFF() \
	SET_RENDER_STATE(D3DRENDERSTATE_COLORKEYENABLE, FALSE)

#define ANTIALIAS_ON() \
	SET_RENDER_STATE(D3DRENDERSTATE_ANTIALIAS, DxState.AntiAlias)

#define ANTIALIAS_OFF() \
	SET_RENDER_STATE(D3DRENDERSTATE_ANTIALIAS, D3DANTIALIAS_NONE)

#define CULL_ON() \
	SET_RENDER_STATE(D3DRENDERSTATE_CULLMODE, D3DCULL_CCW)

#define CULL_OFF() \
	SET_RENDER_STATE(D3DRENDERSTATE_CULLMODE, D3DCULL_NONE)

#define SPECULAR_ON() \
	SET_RENDER_STATE(D3DRENDERSTATE_SPECULARENABLE, TRUE)

#define SPECULAR_OFF() \
	SET_RENDER_STATE(D3DRENDERSTATE_SPECULARENABLE, FALSE)

#define WIREFRAME_ON() \
	SET_RENDER_STATE(D3DRENDERSTATE_FILLMODE, DxState.Wireframe)

#define WIREFRAME_OFF() \
	SET_RENDER_STATE(D3DRENDERSTATE_FILLMODE, D3DFILL_SOLID)

#define TEXTUREFILTER_OFF() \
{ \
	SET_STAGE_STATE(0, D3DTSS_MINFILTER, D3DTFN_POINT); \
	SET_STAGE_STATE(0, D3DTSS_MAGFILTER, D3DTFG_POINT); \
	SET_STAGE_STATE(1, D3DTSS_MINFILTER, D3DTFN_POINT); \
	SET_STAGE_STATE(1, D3DTSS_MAGFILTER, D3DTFG_POINT); \
}

#define TEXTUREFILTER_ON() \
{ \
	switch (DxState.TextureFilter) \
	{ \
		case 0: \
			SET_STAGE_STATE(0, D3DTSS_MINFILTER, D3DTFN_POINT); \
			SET_STAGE_STATE(0, D3DTSS_MAGFILTER, D3DTFG_POINT); \
			SET_STAGE_STATE(1, D3DTSS_MINFILTER, D3DTFN_POINT); \
			SET_STAGE_STATE(1, D3DTSS_MAGFILTER, D3DTFG_POINT); \
			break; \
		case 1: \
			SET_STAGE_STATE(0, D3DTSS_MINFILTER, D3DTFN_LINEAR); \
			SET_STAGE_STATE(0, D3DTSS_MAGFILTER, D3DTFG_LINEAR); \
			SET_STAGE_STATE(1, D3DTSS_MINFILTER, D3DTFN_LINEAR); \
			SET_STAGE_STATE(1, D3DTSS_MAGFILTER, D3DTFG_LINEAR); \
			break; \
		case 2: \
			SET_STAGE_STATE(0, D3DTSS_MINFILTER, D3DTFN_ANISOTROPIC); \
			SET_STAGE_STATE(0, D3DTSS_MAGFILTER, D3DTFG_ANISOTROPIC); \
			SET_STAGE_STATE(1, D3DTSS_MINFILTER, D3DTFN_ANISOTROPIC); \
			SET_STAGE_STATE(1, D3DTSS_MAGFILTER, D3DTFG_ANISOTROPIC); \
			break; \
	} \
}

#define MIPMAP_OFF() \
{ \
	SET_STAGE_STATE(0, D3DTSS_MIPFILTER, D3DTFP_NONE); \
	SET_STAGE_STATE(1, D3DTSS_MIPFILTER, D3DTFP_NONE); \
}

#define MIPMAP_ON() \
{ \
	SET_STAGE_STATE(0, D3DTSS_MIPFILTER, DxState.MipMap + 1); \
	SET_STAGE_STATE(1, D3DTSS_MIPFILTER, DxState.MipMap + 1); \
}

#define FOG_COLOR(_c) \
	SET_RENDER_STATE(D3DRENDERSTATE_FOGCOLOR, _c)

#define FOG_COLOR(_c) \
	SET_RENDER_STATE(D3DRENDERSTATE_FOGCOLOR, _c)

#define SORT_INDEPENDENT_ON() \
	SET_RENDER_STATE(D3DRENDERSTATE_TRANSLUCENTSORTINDEPENDENT, TRUE)

#define SORT_INDEPENDENT_OFF() \
	SET_RENDER_STATE(D3DRENDERSTATE_TRANSLUCENTSORTINDEPENDENT, FALSE)

#define SET_TPAGE(_tp) \
{ \
	if (RenderTP != _tp) \
	{ \
		if ((RenderTP = _tp) == -1) \
		{ \
			SET_TEXTURE(0, NULL); \
		} \
		else \
		{ \
			SET_TEXTURE(0, TexInfo[RenderTP].Texture); \
		} \
	} \
}

#define SET_TPAGE2(_tp) \
{ \
	if (RenderTP2 != _tp) \
	{ \
		if ((RenderTP2 = _tp) == -1) \
		{ \
			SET_TEXTURE(1, NULL); \
		} \
		else \
		{ \
			SET_TEXTURE(1, TexInfo[RenderTP].Texture); \
		} \
	} \
}

// structs

typedef struct {
	long WireframeEnabled, Wireframe;
	long PerspectiveEnabled, Perspective;
	long TextureFilterFlag, TextureFilter;
	long MipMapFlag, MipMap;
	long FogEnabled, Fog;
	long DitherEnabled, Dither;
	long ColorKeyEnabled, ColorKey;
	long AntiAliasEnabled, AntiAlias;
} DX_STATE;

typedef struct {
	DWORD Width, Height, Bpp, Refresh;
	char DisplayText[MAX_DISPLAY_MODE_TEXT];
} DISPLAY_MODE;

typedef struct {
	char Name[MAX_DRAW_DEVICE_NAME];
	long DisplayModeNum, BestDisplayMode;
	DISPLAY_MODE DisplayMode[MAX_DISPLAY_MODES];
} DRAW_DEVICE;

// prototypes

extern BOOL InitDD(void);
extern BOOL InitD3D(DWORD width, DWORD height, DWORD bpp, DWORD refresh);
extern void ReleaseDX(void);
extern void ReleaseD3D(void);
extern void SetGamma(long brightness, long contrast);
extern void ErrorDX(HRESULT r, char *mess);
extern void SetBackgroundColor(long col);
extern HRESULT CALLBACK EnumZedBufferCallback(DDPIXELFORMAT *ddpf, void *user);
extern void CheckSurfaces(void);
extern void FlipBuffers(void);
extern void ClearBuffers(void);
extern void SetFrontBufferRGB(long rgb);
extern void SetupDxState(void);
extern void GetDrawDevices(void);
extern BOOL CALLBACK GetDrawDeviceCallback(GUID *lpGUID, LPSTR szName, LPSTR szDevice, LPVOID lParam);
extern BOOL CALLBACK CreateDrawDeviceCallback(GUID *lpGUID, LPSTR szName, LPSTR szDevice, LPVOID lParam);
extern HRESULT CALLBACK DisplayModesCallback(DDSURFACEDESC2 *Mode, void *UserArg);

// globals

extern DX_STATE DxState;
extern IDirectDraw4 *DD;
extern IDirectDrawSurface4 *FrontBuffer;
extern IDirectDrawSurface4 *BackBuffer;
extern IDirectDrawSurface4 *ZedBuffer;
extern IDirectDrawGammaControl *GammaControl;
extern IDirect3D3 *D3D;
extern IDirect3DDevice3 *D3Ddevice;
extern IDirect3DViewport3 *D3Dviewport;
extern D3DDEVICEDESC D3Dcaps;
extern DDPIXELFORMAT ZedBufferFormat;
extern DWORD ScreenXsize;
extern DWORD ScreenYsize;
extern DWORD ScreenBpp;
extern DWORD ScreenRefresh;
extern long GammaFlag;
extern long NoColorKey;
extern long DrawDeviceNum, CurrentDrawDevice;
extern long DisplayModeCount, RenderStateChange, TextureStateChange;
extern DRAW_DEVICE DrawDevices[];
extern short RenderTP, RenderTP2, RenderFog, RenderAlpha, RenderAlphaSrc, RenderAlphaDest;
extern short RenderZcmp, RenderZwrite, RenderZbuffer;

#endif
