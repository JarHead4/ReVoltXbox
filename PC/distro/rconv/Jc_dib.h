//-------------------------------------------------------------------------------------------------------------------------
#pragma once
//-------------------------------------------------------------------------------------------------------------------------
//typedef U32 JC_Pixel; // Simple 32-bit pixel type.
class JC_Pixel
{
	union colour_word
	{
		U32 word;
		struct 
		{
			U8 b;
			U8 g;
			U8 r;
			U8 a;
		} bytes;
	};
	colour_word m_Value;
public:
	const JC_Pixel& operator = (U32 value) { m_Value.word = value; return *this; }
	operator U32 (void) const { return m_Value.word; }
	JC_Pixel(const JC_Pixel& pixel) { m_Value.word = pixel; }
	JC_Pixel(const U32& value) { m_Value.word = value; }
	JC_Pixel(void) { m_Value.word = 0; }
	operator ^= (const JC_Pixel& pixel) { m_Value.word ^= pixel.m_Value.word; return *this; }
	BYTE R(void) const { return m_Value.bytes.r; }
	BYTE G(void) const { return m_Value.bytes.g; }
	BYTE B(void) const { return m_Value.bytes.b; }
	BYTE A(void) const { return m_Value.bytes.a; }
	U16 Get16Bit(void) const;
};
//-------------------------------------------------------------------------------------------------------------------------
inline JC_Pixel JC_RGB(int r, int g, int b) { return (JC_Pixel)((r << 16) | (g << 8) | b); }
inline JC_Pixel JC_RGBA(int r, int g, int b, int a) { return (JC_Pixel)(JC_RGB(r, g, b) | (a << 24)); }
//-------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_Dib : public CObject	// A hi-colour device-independant bitmap (dib) for general use.
{
	DECLARE_DYNAMIC(JC_Dib)

public:
	enum { Depth = 32 };

	enum // Draw modes. Only supported for Rectangle().
	{
		Solid,
		Xor,
	};

protected:
	CSize m_Size;						// Dimensions of bitmap in pixels.
	int m_ImageSize;					// Size of the bitmap image data in bytes.
	int m_PixelsNextLine;				// Offset to next line in pixels.
	JC_Pixel m_Background;				// Background colour.
	JC_Pixel * m_ImageData;				// Ptr to the start of the actual image data in memory.
	HBITMAP m_BitmapHandle;				// Handle which can be used to access the bitmap using GDI.
	mutable HBITMAP m_OldBitmapHandle;	// Handle saved from dc when using Begin/EndGDI.

public:
	JC_Dib(COLORREF background, CSize = CSize(0, 0));
	~JC_Dib();

	BOOL Update(void);
	void SetSize(CSize size);
	void Clear(void) { DeleteBitmap(); }
	void Blit(CDC * dc, CPoint pos) const;
	CDC* BeginGDI(void) const;
	void EndGDI(CDC * dc) const;
	void WriteBMP(const CString& path) const;
	JC_Pixel GetBackground(void) const { return m_Background; }
	void Rectangle(const CRect& rect, JC_Pixel col, int drawmode=Solid) const;
	COLORREF COLORREFFromPixel(JC_Pixel p) const { return RGB((p >> 16) & 0xFF, (p >> 8) & 0xFF, p & 0xFF); }
	static JC_Pixel PixelFromCOLORREF(COLORREF c) { return JC_RGB(GetRValue(c), GetGValue(c), GetBValue(c)); }
	bool CreateBitmapInfo(BITMAPINFO*& bitmap_info);

protected:
	void DeleteBitmap(void);
};
//-------------------------------------------------------------------------------------------------------------------------
