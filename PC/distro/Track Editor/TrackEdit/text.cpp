#include <assert.h>

#include "Render.h"

extern LPDIRECT3DTEXTURE2 UnitTex[9];
extern LPDIRECT3DTEXTURE2 ButtonTex;
extern LPDIRECT3DTEXTURE2 SprueWireTex;
extern LPDIRECT3DTEXTURE2 SprueFontTex;
extern LPDIRECT3DTEXTURE2 GameFontTex;
extern LPDIRECT3DTEXTURE2 ClockTex;

FONTDESCRIPTION GameFont = {13, 17, 13, 19, &GameFontTex};
FONTDESCRIPTION SprueWireFont = {32, 32, 32, 8, &SprueWireTex};
FONTDESCRIPTION SprueTextFont = {16, 16, 14, 16, &SprueFontTex};

static FONTDESCRIPTION* CurrentFont = NULL;

void SetCurrentFont(LPDIRECT3DDEVICE3 pd3dDevice, FONTDESCRIPTION* newfont)
{
	assert(pd3dDevice != NULL);
	assert(newfont != NULL);
	
	if(newfont != CurrentFont)
	{
		SetCurrentTexture(pd3dDevice, *(newfont->Texture));
		CurrentFont = newfont;
	}
}

void DrawText(LPDIRECT3DDEVICE3 pd3dDevice, U16 x, U16 y, const char* text)
{
	assert(CurrentFont != NULL);
	
	SetCurrentTexture(pd3dDevice, *(CurrentFont->Texture));
	REAL destleft = x;
	REAL desttop = y;
	REAL destright = destleft + CurrentFont->PixelWidth;
	REAL destbottom = desttop + CurrentFont->PixelHeight;
	U16 n = 0;
	while(text[n] != '\0')
	{
		char c = text[n];
		
		if(c == '\n')
		{
			desttop += CurrentFont->PixelHeight;
			destbottom += CurrentFont->PixelHeight;
		}
		else
		{
			if(c == '\r')
			{
				destleft = x;
				destright = destleft + CurrentFont->PixelWidth;
			}
			else
			{
				c -= '!';
				if( c >= 0)
				{
					REAL srcleft = (c % CurrentFont->CharsPerRow) * CurrentFont->PixelWidth;
					REAL srctop = (c / CurrentFont->CharsPerRow) * CurrentFont->PixelHeight;
					REAL srcright = srcleft + CurrentFont->PixelWidth;
					REAL srcbottom = srctop + CurrentFont->PixelHeight;
					srcleft /= 256.0f;
					srctop /= 256.0f;
					srcright /= 256.0f;
					srcbottom /= 256.0f;
					DrawSprite(pd3dDevice, destleft, desttop, destright, destbottom, srcleft, srctop, srcright, srcbottom);
				}
				destleft += CurrentFont->UsedWidth;
				destright += CurrentFont->UsedWidth;
			}
		}
		n++;
	}
}