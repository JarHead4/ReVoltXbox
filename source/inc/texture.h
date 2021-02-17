#ifndef TEXTURE_H
#define TEXTURE_H

// macros

#define MAX_MIPMAPS 2
#define MAX_TPAGE_FILENAME 32
#define MAX_TEXTURE_TEST 1024

#define TPAGE_WORLD_NUM 10
#define TPAGE_SCALE_NUM 4
#define TPAGE_FIXED_NUM 5

enum {
	TPAGE_WORLD_START,
	TPAGE_CAR_START = TPAGE_WORLD_NUM,
	TPAGE_FONT = TPAGE_WORLD_NUM + MAX_NUM_PLAYERS,
	TPAGE_BIGFONT,
	TPAGE_ENVSTILL,
	TPAGE_ENVROLL,
	TPAGE_SHADOW,
	TPAGE_FX1,
	TPAGE_FX2,
	TPAGE_FX3,
	TPAGE_MISC1,

	TPAGE_NUM
};

typedef struct {
	long Active;
	long Width;
	long Height;
	long Stage;
	long MipCount;
	char File[MAX_TPAGE_FILENAME];

	IDirect3DTexture2 *Texture;
	IDirectDrawSurface4 *Surface;
	IDirectDrawPalette *Palette;
} TEXINFO;

// prototypes

extern bool CreateTPages(int nPages);
extern void DestroyTPages();
extern void GetTextureFormat(long bpp);
extern void PickTextureSets(long playernum);
extern long MipSize(long size, long set, long count, long mip);
extern HRESULT CALLBACK FindTextureCallback(DDPIXELFORMAT *ddpf, void *lParam);
extern long CountTexturePixels(long needed, long width, long height);
extern long CountMipTexturePixels(long needed, long width, long height);
extern bool LoadTextureClever(char *tex, char tpage, long width, long height, long stage, long set, long mip);
extern bool LoadTexture(char *tex, char tpage, long width, long height, long stage);
extern bool LoadMipTexture(char *tex, char tpage, long width, long height, long stage, long mipcount);
extern void InitTextures(void);
extern void FreeTextures(void);
extern void FreeOneTexture(char tp);

// globals

extern DDPIXELFORMAT TexFormat;
extern char TexturesEnabled, TexturesSquareOnly, TexturesAGP;
extern TEXINFO *TexInfo;
extern int TEX_NTPages;
extern long TexturePixels, WorldTextureSet, CarTextureSet, FxTextureSet;
extern DWORD TextureMinWidth, TextureMaxWidth, TextureMinHeight, TextureMaxHeight;

#endif