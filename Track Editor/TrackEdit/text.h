#ifndef _TEXT_H
#define _TEXT_H

typedef struct
{
	U16					PixelWidth;
	U16					PixelHeight;
	U16					UsedWidth;
	U16					CharsPerRow;
	LPDIRECT3DTEXTURE2*	Texture;
}FONTDESCRIPTION;

void DrawText(LPDIRECT3DDEVICE3 pd3dDevice, U16 x, U16 y, const char* text);
void SetCurrentFont(LPDIRECT3DDEVICE3 pd3dDevice, FONTDESCRIPTION* newfont);

#endif
