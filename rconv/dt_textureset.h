#ifndef __DT_TEXTURESET_H
#define __DT_TEXTURESET_H
 
enum enumOrientation {TEXT_NORMAL = 1, TEXT_ROTATE90 = 2, TEXT_ROTATE180 = 4, TEXT_ROTATE270 = 8,
					  TEXT_XFLIP = 16, TEXT_YFLIP = 32, TEXT_XYFLIP = 64};

enum enumView {DT_TPETSETVIEW = 0, DT_TPESCRATCHVIEW = 1};
enum enumOverlap {DT_TPENOOVERLAP = 0, DT_TPEXOVERLAP = 1, DT_TPEYOVERLAP = 2, DT_TPEXYOVERLAP = 3};

class DT_BooleanArray;
class PS_UniqueRecordList;
class PS_CompressRecordList;

class JCCLASS_DLL DT_BooleanReference
{
private:
	unsigned char* m_pIndex;
	unsigned char m_nMask;
	
	DT_BooleanReference (void);
	void Assign (unsigned char* pIndex, unsigned char nMask);

	friend BOOL operator== (const DT_BooleanReference &a, const DT_BooleanReference &b);

	friend class DT_BooleanArray;
};

class JCCLASS_DLL DT_BooleanArray
{
private:
	unsigned char m_nMask;
	U32 m_nArrayIndex;
	U32 m_nReferenceIndex;

	DT_BooleanReference m_BooleanReference [2];

protected:
	unsigned char* m_pTable;
	U32 m_nLimit;
	U32 m_nSet;

public:
	DT_BooleanArray (U32 nLimit = 0, BOOL bInitial = FALSE);
	~DT_BooleanArray ();

	void RemoveAll (void);
	void SetSize (U32 nLimit = 0, BOOL bInitial = FALSE);
	void SetTrue (U32 nIndex);
	void SetFalse (U32 nIndex);
	void Toggle (U32 nIndex);

	BOOL IsTrue (U32 nIndex);
	BOOL IsFalse (U32 nIndex);

	U32 CountTrue (void) { return m_nSet; }
	U32 CountFalse (void) { return m_nLimit - m_nSet; }

	DT_BooleanReference operator [] (U32 nIndex);

	DT_BooleanArray& operator= (const DT_BooleanArray& BoolArray);

private:
	void CalculateIndex (U32 nIndex);
};

struct JCCLASS_DLL DT_PaletteEntry
{
	U32 m_nIndex;
	U32 m_nRemapTo;
	U32 m_nCount;

	PALETTEENTRY m_Colour;

	DT_PaletteEntry& operator= (const DT_PaletteEntry& pal);
};

class JCCLASS_DLL DT_Bitmap
{
	JC_Binary m_Data;

	U32 m_nPaletteOffset;
	U32 m_nDataOffset;

public:
	DT_Bitmap (void);
	~DT_Bitmap (void);
	BOOL Create (JC_Bitmap& Bitmap, CRect& Rect);
	U8* Header (void);
	U8* Data (void);
	RGBQUAD* Palette (void);
	U32 Width (void);
	S32 Height (void);
	U32 BitDepth (void);
	U32 Stride (void);
	U32 Size (void);
	void Render (CDC* dc, CPoint& point);
	void YFlip (void);
	void XFlip (void);
	void XYFlip (void);
	void RotateClockwise90 (void);
	void RotateAntiClockwise90 (void);
	void Rotate180 (void);

	DT_Bitmap& operator= (const DT_Bitmap& Bitmap);
};

class JCCLASS_DLL DT_UVPoint : public CObject
{
public:
	S32 x;
	S32 y;

	float U;
	float fFractU;
	float V;
	float fFractV;

	DT_UVPoint (void) {}
	DT_UVPoint (const DT_UVPoint& point) { *this = point; }

	DT_UVPoint& operator= (const DT_UVPoint& point);
};

typedef CArray <CPoint, CPoint&> PS_PointArray;
typedef CArray <JC_Colour, JC_Colour&> JC_ColourArray;

class JCCLASS_DLL DT_TextureEntry
{
protected:
	U32 m_nOverlap;

	U32 m_nType;
	U32 m_nOrientation;
	U32 m_nRecord;
	U32 m_nBitDepth;	
	U32 m_nUniqueColours;
	U32 m_nHeaderSize;
	U32 m_nStride;
	U32 m_nStartY;
	U32 m_nEndY;
	U32 m_nGroup;
	U32 m_nParent;

	S32* m_nLeft;
	S32* m_nRight;
	
	BOOL m_bAllowSnap;
	BOOL m_bGroup;

	PS_Texture* m_pTexture;

	CRect m_Rect;
	CRect m_SrcRect;
	
	CPoint m_Pt;
	CPoint m_Position;
	CPoint m_Start;
	
//	CArray <DT_UVPoint, DT_UVPoint&> m_Point; // this not needed any more?

	PS_PointArray m_Point;
	PS_UVArray m_AdjustedUV;

	CDWordArray m_nCount;
	CDWordArray m_nRemap;
	
//	CPalette m_TexturePal;
	JC_ColourArray m_TexturePal;
	
	DT_BooleanArray m_ColourUsed;
	
	DT_PaletteEntry* m_PalEntry;

	DT_Bitmap m_Bitmap;

	double m_Bleed;

public:
	enum {DT_TE1BIT, DT_TE4BIT, DT_TE8BIT, DT_TE15BIT, DT_TE16BIT, DT_TE24BIT, DT_TE32BIT, DT_TEGROUP};
	enum {HIGH_NIBBLE = 0, LOW_NIBBLE = 1};
	enum BOUNDARY_POS { TOP_LEFT = 0, TOP_RIGHT, BOTTOM_LEFT, BOTTOM_RIGHT, TOTAL_POSITIONS };

	DT_TextureEntry (void);
	DT_TextureEntry (DT_TextureEntry& Texture) { *this = Texture; };
	BOOL Init(PS_Texture * texture);
	~DT_TextureEntry (void);
	void Remove ();
	double GetBleedPixels(void) const { return m_Bleed; }
	void   SetBleedPixels(double pixels) { m_Bleed = pixels; }
	void CreatePalette(LOGPALETTE * log_pal);
	U32 RecordNumber (void) { return m_nRecord; }
	U32 RecordNumber (U32 nRecordNumber) { m_nRecord = nRecordNumber; return nRecordNumber; }
	void Orientation (U32 nOrientation) { m_nOrientation = nOrientation; }
	U32 Orientation (void) { return m_nOrientation; }
	PS_Texture* Texture (void) { return m_pTexture; }
	PS_Texture* Texture (PS_Texture* pTexture) { m_pTexture = pTexture; return m_pTexture; }
	void SetRect (CRect Rect) { m_Rect = Rect; }
	CRect& GetRect (void) { return m_Rect; }
	void SetSrcRect (CRect Rect) { m_SrcRect = Rect; }
	CRect& GetSrcRect (void) { return m_SrcRect; }
	CPoint& Position (void) { return m_Position; }
	CPoint& Position (CPoint Pos) { m_Position = Pos; return m_Position; }
	CPoint& StartPosition (void) { return m_Start; }
	CPoint& StartPosition (CPoint Pos) { m_Start = Pos; return m_Start; }
	CPoint& Point (U32 nIndex);
	CPoint& UVPoint (U32 nIndex);
	PS_UVArray& UVArray(void) { return m_AdjustedUV; }
	U32 NumPoints () { return m_Point.GetSize (); }
	DT_Bitmap& Bitmap (void) { return m_Bitmap; }
	JC_Bitmap* GetBitmap (void) { return (JC_Bitmap *) m_pTexture -> Bitmap (); }
	void CalculateBitmapPoints (const CSize& Size);
	POINT CalculateBleedPoint(const PS_FloatUV& uv, const CSize& bitmap_size, const BOUNDARY_POS pos);
	void CalculateOutlineUVs(const PS_FloatUVArray& source_uvs, const CSize& bitmap_size, PS_PointArray& outline_uvs);

	BOOL CreateEdgeTable (void);
	void CountUniqueColours8Bit (U8* pData);
	U32 GetNumberUniqueColours (void) { return m_nUniqueColours; }
	void ReduceTexturePaletteLowCount (U32 nTarget = 16);
	U32 ColoursUsed (void) { return m_nUniqueColours; }
	S32 Vertices (void) { return m_Point.GetSize (); }
	void AddPaletteStrip (RGBQUAD* pColour);
	void Copy16Col (BITMAPINFOHEADER* bmihDest, U8* pDest, U32 nStrip);
	void LockToGrid (CSize& sGrid);
	void YFlip (void);
	void XFlip (void);
	void XYFlip (void);
	void RotateClockwise90 (void);
	void RotateAntiClockwise90 (void);
	void Rotate180 (void);
	S32 Left (U32 nIndex) { ASSERT (m_nLeft != NULL); return m_nLeft [nIndex]; }
	S32 Right (U32 nIndex) { ASSERT (m_nRight != NULL); return m_nRight [nIndex]; }
	U32 Group (U32 nGroup) { m_nGroup = nGroup; return m_nGroup; }
	U32 Group (void) { return m_nGroup; }
	BOOL AllowSnap (BOOL bSnap) { m_bAllowSnap = bSnap; return m_bAllowSnap; }
	BOOL AllowSnap (void) { return m_bAllowSnap; }
	U32 Parent (U32 nParent) { m_nParent = nParent; return m_nParent; }
	U32 Parent (void) { return m_nParent; }
	U32 Type () { return m_nType; }
	U8 GetColourIndex (U8 nSrc);
	U8 GetColourIndex (U8 nRed, U8 nGreen, U8 nBlue);
//	CPalette& Palette () { return m_TexturePal; }
	U32 Overlap (U32 nOverlap) { m_nOverlap = nOverlap; return m_nOverlap; }
	U32 Overlap () { return m_nOverlap; }
	BOOL NoOverlap () { return m_nOverlap == DT_TPENOOVERLAP; }
	BOOL XOverlap () { return m_nOverlap & DT_TPEXOVERLAP; }
	BOOL YOverlap () { return m_nOverlap & DT_TPEYOVERLAP; }
	BOOL XYOverlap () { return m_nOverlap == DT_TPEXYOVERLAP; }

	virtual void CountUniqueColours ();
	virtual void CreateTexturePalette (void);
	virtual void CopyTo1Bit (BITMAPINFOHEADER* /* bmihDest */, U8* /* pDest */) {};
	virtual void CopyTo4Bit (BITMAPINFOHEADER* /* bmihDest */, U8* /* pDest */) {};
	virtual void CopyTo8Bit (BITMAPINFOHEADER* /* bmihDest */, U8* /* pDest */) {};
	virtual void CopyTo15Bit (BITMAPINFOHEADER* /* bmihDest */, U8* /* pDest */) {};
	virtual void CopyTo16Bit (BITMAPINFOHEADER* /* bmihDest */, U8* /* pDest */) {};
	virtual void CopyTo24Bit (BITMAPINFOHEADER* /* bmihDest */, U8* /* pDest */) {};
	virtual void CopyTo32Bit (BITMAPINFOHEADER* /* bmihDest */, U8* /* pDest */) {};
	
	DT_TextureEntry& Equals (DT_TextureEntry& Texture);

	virtual DT_TextureEntry& operator= (DT_TextureEntry& Texture) { return Equals (Texture); }

	friend JC_Buffer& operator << (JC_Buffer& buf, DT_TextureEntry& Texture);
    friend JC_Buffer& operator >> (JC_Buffer& buf, DT_TextureEntry& Texture);

protected:
	U32 ColourDifference (PALETTEENTRY a, PALETTEENTRY b);
};

class JCCLASS_DLL DT_TextureEntryScratchPad : public DT_TextureEntry
{
public:
	DT_TextureEntryScratchPad (void) : DT_TextureEntry () {};
	DT_TextureEntryScratchPad (DT_TextureEntryScratchPad& Texture) { *this = Texture; };

	virtual void CountUniqueColours (JC_Bitmap* /* Bitmap */) {}
	virtual void CreateTexturePalette (void) {}
	virtual void CopyTo1Bit (BITMAPINFOHEADER* /* bmihDest */, U8* /* pDest */) {};
	virtual void CopyTo4Bit (BITMAPINFOHEADER* /* bmihDest */, U8* /* pDest */) {};
	virtual void CopyTo8Bit (BITMAPINFOHEADER* /* bmihDest */, U8* /* pDest */) {};
	virtual void CopyTo15Bit (BITMAPINFOHEADER* /* bmihDest */, U8* /* pDest */) {};
	virtual void CopyTo16Bit (BITMAPINFOHEADER* /* bmihDest */, U8* /* pDest */) {};
	virtual void CopyTo24Bit (BITMAPINFOHEADER* /* bmihDest */, U8* /* pDest */) {};
	virtual void CopyTo32Bit (BITMAPINFOHEADER* /* bmihDest */, U8* /* pDest */) {};

	DT_TextureEntryScratchPad& operator= (DT_TextureEntryScratchPad& Texture) { return (DT_TextureEntryScratchPad&) Equals (Texture); }
};

class JCCLASS_DLL DT_TextureEntry1Bit : public DT_TextureEntry
{
public:
	DT_TextureEntry1Bit (void) : DT_TextureEntry () {m_nBitDepth = 1; m_nType = DT_TE1BIT; }
	DT_TextureEntry1Bit (DT_TextureEntry1Bit& Texture) { *this = Texture; };

	virtual void CountUniqueColours () {}
	virtual void CreateTexturePalette (void) {}
	virtual void CopyTo1Bit (BITMAPINFOHEADER* /* bmihDest */, U8* /* pDest */) {};
	virtual void CopyTo4Bit (BITMAPINFOHEADER* /* bmihDest */, U8* /* pDest */) {};
	virtual void CopyTo8Bit (BITMAPINFOHEADER* /* bmihDest */, U8* /* pDest */) {};
	virtual void CopyTo15Bit (BITMAPINFOHEADER* /* bmihDest */, U8* /* pDest */) {};
	virtual void CopyTo16Bit (BITMAPINFOHEADER* /* bmihDest */, U8* /* pDest */) {};
	virtual void CopyTo24Bit (BITMAPINFOHEADER* /* bmihDest */, U8* /* pDest */) {};
	virtual void CopyTo32Bit (BITMAPINFOHEADER* /* bmihDest */, U8* /* pDest */) {};

	DT_TextureEntry1Bit& operator= (DT_TextureEntry1Bit& Texture) { return (DT_TextureEntry1Bit&) Equals (Texture); }
};

class JCCLASS_DLL DT_TextureEntry4Bit : public DT_TextureEntry
{
public:
	DT_TextureEntry4Bit (void) : DT_TextureEntry () {m_nBitDepth = 4; m_nType = DT_TE4BIT; };
	DT_TextureEntry4Bit (DT_TextureEntry4Bit& Texture) { *this = Texture; };

	virtual void CountUniqueColours ();
	virtual void CreateTexturePalette (void);
	virtual void CopyTo1Bit (BITMAPINFOHEADER* /* bmihDest */, U8* /* pDest */) {}
	virtual void CopyTo4Bit (BITMAPINFOHEADER* bmihDest, U8* pDest);
	virtual void CopyTo8Bit (BITMAPINFOHEADER* bmihDest, U8* pDest);
	virtual void CopyTo15Bit (BITMAPINFOHEADER* bmihDest, U8* pDest);
	virtual void CopyTo16Bit (BITMAPINFOHEADER* bmihDest, U8* pDest);
	virtual void CopyTo24Bit (BITMAPINFOHEADER* bmihDest, U8* pDest);
	virtual void CopyTo32Bit (BITMAPINFOHEADER* bmihDest, U8* pDest);

	DT_TextureEntry4Bit& operator= (DT_TextureEntry4Bit& Texture) { return (DT_TextureEntry4Bit&) Equals (Texture); }
};

class JCCLASS_DLL DT_TextureEntry8Bit : public DT_TextureEntry
{
public:
	DT_TextureEntry8Bit (void) : DT_TextureEntry () {m_nBitDepth = 8; m_nType = DT_TE8BIT; }
	DT_TextureEntry8Bit (DT_TextureEntry8Bit& Texture) { *this = Texture; };

	virtual void CountUniqueColours ();
	virtual void CreateTexturePalette (void);
	virtual void CopyTo1Bit (BITMAPINFOHEADER* /* bmihDest */, U8* /* pDest */) {}
	virtual void CopyTo4Bit (BITMAPINFOHEADER* bmihDest, U8* pDest);
	virtual void CopyTo8Bit (BITMAPINFOHEADER* bmihDest, U8* pDest);
	virtual void CopyTo15Bit (BITMAPINFOHEADER* bmihDest, U8* pDest);
	virtual void CopyTo16Bit (BITMAPINFOHEADER* bmihDest, U8* pDest);
	virtual void CopyTo24Bit (BITMAPINFOHEADER* bmihDest, U8* pDest);
	virtual void CopyTo32Bit (BITMAPINFOHEADER* bmihDest, U8* pDest);

	DT_TextureEntry8Bit& operator= (DT_TextureEntry8Bit& Texture) { return (DT_TextureEntry8Bit&) Equals (Texture); }
};

class JCCLASS_DLL DT_TextureEntry15Bit : public DT_TextureEntry
{
public:
	DT_TextureEntry15Bit (void) : DT_TextureEntry () {m_nBitDepth = 15; m_nType = DT_TE15BIT; };
	DT_TextureEntry15Bit (DT_TextureEntry15Bit& Texture) { *this = Texture; };

	virtual void CountUniqueColours ();
	virtual void CreateTexturePalette (void);
	virtual void CopyTo1Bit (BITMAPINFOHEADER* /* bmihDest */, U8* /* pDest */) {};
	virtual void CopyTo4Bit (BITMAPINFOHEADER* bmihDest, U8* pDest);
	virtual void CopyTo8Bit (BITMAPINFOHEADER* bmihDest, U8* pDest);
	virtual void CopyTo15Bit (BITMAPINFOHEADER* bmihDest, U8* pDest);
	virtual void CopyTo16Bit (BITMAPINFOHEADER* bmihDest, U8* pDest);
	virtual void CopyTo24Bit (BITMAPINFOHEADER* bmihDest, U8* pDest);
	virtual void CopyTo32Bit (BITMAPINFOHEADER* bmihDest, U8* pDest);

	DT_TextureEntry15Bit& operator= (DT_TextureEntry15Bit& Texture) { return (DT_TextureEntry15Bit&) Equals (Texture); }
};

class JCCLASS_DLL DT_TextureEntry16Bit : public DT_TextureEntry
{
public:
	DT_TextureEntry16Bit (void) : DT_TextureEntry () {m_nBitDepth = 16; m_nType = DT_TE16BIT; };
	DT_TextureEntry16Bit (DT_TextureEntry16Bit& Texture) { *this = Texture; };

	virtual void CountUniqueColours ();
	virtual void CreateTexturePalette (void);
	virtual void CopyTo1Bit (BITMAPINFOHEADER* /* bmihDest */, U8* /* pDest */) {};
	virtual void CopyTo4Bit (BITMAPINFOHEADER* bmihDest, U8* pDest);
	virtual void CopyTo8Bit (BITMAPINFOHEADER* bmihDest, U8* pDest);
	virtual void CopyTo15Bit (BITMAPINFOHEADER* bmihDest, U8* pDest);
	virtual void CopyTo16Bit (BITMAPINFOHEADER* bmihDest, U8* pDest);
	virtual void CopyTo24Bit (BITMAPINFOHEADER* bmihDest, U8* pDest);
	virtual void CopyTo32Bit (BITMAPINFOHEADER* bmihDest, U8* pDest);

	DT_TextureEntry16Bit& operator= (DT_TextureEntry16Bit& Texture) { return (DT_TextureEntry16Bit&) Equals (Texture); }
};

class JCCLASS_DLL DT_TextureEntry24Bit : public DT_TextureEntry
{
public:
	DT_TextureEntry24Bit (void) : DT_TextureEntry () {m_nBitDepth = 24; m_nType = DT_TE24BIT; }
	DT_TextureEntry24Bit (DT_TextureEntry24Bit& Texture) { *this = Texture; };

	virtual void CountUniqueColours ();
	virtual void CreateTexturePalette (void);
	virtual void CopyTo1Bit (BITMAPINFOHEADER* /* bmihDest */, U8* /* pDest */) {};
	virtual void CopyTo4Bit (BITMAPINFOHEADER* bmihDest, U8* pDest);
	virtual void CopyTo8Bit (BITMAPINFOHEADER* bmihDest, U8* pDest);
	virtual void CopyTo15Bit (BITMAPINFOHEADER* bmihDest, U8* pDest);
	virtual void CopyTo16Bit (BITMAPINFOHEADER* bmihDest, U8* pDest);
	virtual void CopyTo24Bit (BITMAPINFOHEADER* bmihDest, U8* pDest);
	virtual void CopyTo32Bit (BITMAPINFOHEADER* bmihDest, U8* pDest);

	DT_TextureEntry24Bit& operator= (DT_TextureEntry24Bit& Texture) { return (DT_TextureEntry24Bit&) Equals (Texture); }
};

class JCCLASS_DLL DT_TextureEntry32Bit : public DT_TextureEntry
{
public:
	DT_TextureEntry32Bit (void) : DT_TextureEntry () {m_nBitDepth = 32; m_nType = DT_TE32BIT; }
	DT_TextureEntry32Bit (DT_TextureEntry32Bit& Texture) { *this = Texture; };

	virtual void CountUniqueColours ();
	virtual void CreateTexturePalette (void);
	virtual void CopyTo1Bit (BITMAPINFOHEADER* /* bmihDest */, U8* /* pDest */) {};
	virtual void CopyTo4Bit (BITMAPINFOHEADER* bmihDest, U8* pDest);
	virtual void CopyTo8Bit (BITMAPINFOHEADER* bmihDest, U8* pDest);
	virtual void CopyTo15Bit (BITMAPINFOHEADER* bmihDest, U8* pDest);
	virtual void CopyTo16Bit (BITMAPINFOHEADER* bmihDest, U8* pDest);
	virtual void CopyTo24Bit (BITMAPINFOHEADER* bmihDest, U8* pDest);
	virtual void CopyTo32Bit (BITMAPINFOHEADER* bmihDest, U8* pDest);

	DT_TextureEntry32Bit& operator= (DT_TextureEntry32Bit& Texture) { return (DT_TextureEntry32Bit&) Equals (Texture); }
};

// Change to this struct at a later date

class JCCLASS_DLL DT_TextureEntryGroup : public DT_TextureEntry
{
private:
	CArray <DT_TextureEntry*, DT_TextureEntry*> m_Group;

public:
	DT_TextureEntryGroup (void) : DT_TextureEntry () {m_nBitDepth = 0; m_nType = DT_TEGROUP; }
	DT_TextureEntryGroup (DT_TextureEntryGroup& Texture) { *this = Texture; };

	virtual void CountUniqueColours (JC_Bitmap* /* Bitmap */) {}
	virtual void CreateTexturePalette (void) {}
	virtual void CopyTo1Bit (BITMAPINFOHEADER* /* bmihDest */, U8* /* pDest */) {};
	virtual void CopyTo4Bit (BITMAPINFOHEADER* /* bmihDest */, U8* /* pDest */) {};
	virtual void CopyTo8Bit (BITMAPINFOHEADER* /* bmihDest */, U8* /* pDest */) {};
	virtual void CopyTo15Bit (BITMAPINFOHEADER* /* bmihDest */, U8* /* pDest */) {};
	virtual void CopyTo16Bit (BITMAPINFOHEADER* /* bmihDest */, U8* /* pDest */) {};
	virtual void CopyTo24Bit (BITMAPINFOHEADER* /* bmihDest */, U8* /* pDest */) {};
	virtual void CopyTo32Bit (BITMAPINFOHEADER* /* bmihDest */, U8* /* pDest */) {};

	DT_TextureEntryGroup& operator= (DT_TextureEntryGroup& Texture) { return (DT_TextureEntryGroup&) Equals (Texture); }
};

struct JCCLASS_DLL DT_TextureSetType : public CObject
{
	CArray <DT_TextureEntry*, DT_TextureEntry*> m_Texture;

	CSize m_Size;
	
	U32 m_nTPage;
};

typedef CList <DT_TextureEntry*, DT_TextureEntry*> DT_TextureEntryList;

class JCCLASS_DLL DT_TextureSet : public JC_PropertyRecord
{
private:
	enum {DT_TEXTUREENTRY = 0, DT_TEXTURESCRATCHPAD = 1, DT_TEXTURE1BIT = 2, DT_TEXTURE4BIT = 4, DT_TEXTURE8BIT = 8,
	      DT_TEXTURE15BIT = 16, DT_TEXTURE16BIT = 32, DT_TEXTURE24BIT = 64, DT_TEXTURE32BIT = 128, DT_TEXTUREGROUP = 256};
	
	CArray <DT_TextureEntry*, DT_TextureEntry*> m_Texture;
	CArray <DT_TextureEntry*, DT_TextureEntry*> m_ScratchTexture;

	CSize m_Size;
	U8 m_nBitDepth;
	double m_Bleed;
	
	CDWordArray m_nTPage1Bit;
	CDWordArray m_nTPage4Bit;
	CDWordArray m_nTPage8Bit;
	CDWordArray m_nTPage15Bit;
	CDWordArray m_nTPage16Bit;
	CDWordArray m_nTPage24Bit;
	CDWordArray m_nTPage32Bit;

	U32 m_nNumGroups;

public:
    DT_TextureSet(PS_Database* db, U32 recnum);
    virtual ~DT_TextureSet();
	
	void ResolveRefs(JC_UpgradeContext& upgrade);
	DT_TextureEntry* Texture (U32 nTexture,  U32 nView = DT_TPETSETVIEW);
	U32 NumTextures (U32 nView = DT_TPETSETVIEW)
	{
		if (nView == DT_TPETSETVIEW)
		{
			return m_Texture.GetSize ();
		}
		return m_ScratchTexture.GetSize ();
	}

	BOOL AddTextureEntry (U32 nRecordNumber, U32 nOrientation = 0);
	BOOL AddTextureEntry (U32 nRecordNumber, CPoint& point, U32 nOrientation = 0);
	BOOL DeleteTextureEntry (U32 nRecordNumber);
	BOOL CheckTextureOverlap (U32 nSelected);
	BOOL AddScratchTextureEntry (U32 nRecordNumber, U32 nOrientation = 0);
	BOOL AddScratchTextureEntry (U32 nRecordNumber, CPoint& point, U32 nOrientation = 0);
	BOOL DeleteScratchTextureEntry (U32 nRecordNumber);
	BOOL CheckScratchTextureOverlap (U32 nSelected);
	U32 BitDepth (U32 nBitDepth) { m_nBitDepth = static_cast <U8>(nBitDepth); return nBitDepth; }
	U32 BitDepth () { return m_nBitDepth; }
	CSize& Size (CSize size) { m_Size = size; return m_Size; }
	CSize& Size () { return m_Size; }
	CSize GetMinSize ();
	void CreatePSX8BitStrips (void);
	void SetBleedPixelValue(double bleed) { m_Bleed = bleed; }
	double GetBleedPixelValue(void) const { return m_Bleed;  }

	U32 CreateTPage1Bit () { return 0; }
	U32 CreateTPage4Bit ();
//	U32 CreateTPage4BitMultiplePalettes () { return 0; }
	U32 CreateTPage8Bit ();
//	U32 CreateTPage8BitMultiplePalettes () { return 0; }
	U32 CreateTPage15Bit ();
	U32 CreateTPage16Bit ();
	U32 CreateTPage24Bit ();
	U32 CreateTPage32Bit ();

	BOOL GetFloatUVArray (S32 nRecNum, S32 nTextNum, PS_FloatUVArray& array);
//	CPalette& GetPalette (S32 nRecNum, S32 nTextNum);
	BOOL ContainsTexture (const U32 nTextNum);
	
	U32 GetTPage1Bit (void);
	U32 GetTPage4Bit (void);
	U32 GetTPage8Bit (void);
	U32 GetTPage15Bit (void);
	U32 GetTPage16Bit (void);
	U32 GetTPage24Bit (void);
	U32 GetTPage32Bit (void);

	U32 NumberGroups (U32 nNumGroups) { m_nNumGroups = nNumGroups; return m_nNumGroups; }
	U32 NumberGroups (void) { return m_nNumGroups; }

public: // Virtual overrides
	virtual bool Initialise(void); // Called after ReadBody to perform any required initialisation
    virtual U16  Type()         const           { return TypeTextureSet; }
    virtual void WriteBody();
    virtual void ReadBody();
	virtual void Dependencies(PS_CompressRecordList * list) const;

public: // Debugging
    virtual CString TypeString() const { return CString("Texture Set"); }

private:
	DT_TextureEntryList m_TextureList;
	DT_TextureEntryList m_TextureScratchList;
};

BOOL operator== (const PALETTEENTRY& colour1, const RGBQUAD& colour2);
BOOL operator== (const PALETTEENTRY& colour1, const PALETTEENTRY& colour2);
BOOL operator== (const CPalette& palette1, const CPalette& palette2);
BOOL CanRemapPalette (const CPalette& palette1, const CPalette& palette2);

#endif
