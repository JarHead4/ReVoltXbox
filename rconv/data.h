#include <afxtempl.h>
#include <BA_3DViewStyle.h>
#include <jcincludes.h>

#ifndef DATA_H
#define DATA_H

#ifndef build_jcclass_dll
	#ifdef build_jcclass_lib
		#define JCCLASS_DLL 
	#else
		#define JCCLASS_DLL __declspec(dllimport)
	#endif
#else
	#define JCCLASS_DLL __declspec(dllexport)
#endif

//------------------------------------------------------------------------------------------------
class JCCLASS_DLL PS_DeltaColour
{
	S16 m_Red;
	S16 m_Green;
	S16 m_Blue;
	S16 m_Alpha;
public:
	PS_DeltaColour(void) { m_Red = 0; m_Green = 0; m_Blue = 0; m_Alpha = 0; }
	PS_DeltaColour(const PS_DeltaColour& src_delta)
	{
		m_Red   = src_delta.m_Red;
		m_Green = src_delta.m_Green;
		m_Blue  = src_delta.m_Blue;
		m_Alpha = src_delta.m_Alpha;
	}
	PS_DeltaColour(const JC_Colour& dst_colour, const JC_Colour& src_colour)
	{
		SetDelta(dst_colour, src_colour);
	}
	void SetDelta(const JC_Colour& dst_colour, const JC_Colour& src_colour);
	JC_Colour ApplyDelta(const JC_Colour& src_colour);
	PS_DeltaColour& operator *= (double percentage)
	{
		m_Red   = (S16)(((double)m_Red  ) * percentage);
		m_Green = (S16)(((double)m_Green) * percentage);
		m_Blue  = (S16)(((double)m_Blue ) * percentage);
		m_Alpha = (S16)(((double)m_Alpha) * percentage);
		return *this;
	}
};
//------------------------------------------------------------------------------------------------
class JCCLASS_DLL PS_FloatUVOrigin
{
public:
	float u;
	float v;
	PS_FloatUVOrigin(float x, float y);
	PS_FloatUVOrigin(void);
	friend JC_Buffer& operator << (JC_Buffer& buf, PS_FloatUVOrigin& uv);
	friend JC_Buffer& operator >> (JC_Buffer& buf, PS_FloatUVOrigin& uv);
	BOOL operator == (PS_FloatUVOrigin& uv) { return ((u == uv.u) && (v == uv.v)); }
};
//------------------------------------------------------------------------------------------------
class PS_Polygon;
//------------------------------------------------------------------------------------------------
class JCCLASS_DLL PS_FloatUV: public PS_FloatUVOrigin
{
private:
	U32 m_Iterator;
	// this object also acts as the iterator for extraction
	// to extract you should create a blank PS_UV and use the >> operator
	// IsNotLast() function will tell you when you've finished iterating the points
	friend class PS_Polygon;
public:
	PS_FloatUV(float x, float y);
	PS_FloatUV(void);
	operator ++ (int);
	BOOL IsNotLast(void) const;
	U32 Pos(void) { return m_Iterator; }
};
//------------------------------------------------------------------------------------------------
struct PS_UV
{
	double u;
	double v;
};
//------------------------------------------------------------------------------------------------
typedef CArray < PS_UV, PS_UV& > PS_UVArray;
//------------------------------------------------------------------------------------------------
typedef CArray < PS_FloatUV, PS_FloatUV& > PS_FloatUVArray;
//------------------------------------------------------------------------------------------------
class JCCLASS_DLL PS_Polygon
{
	PS_FloatUVOrigin m_Origin;
	PS_FloatUVArray  m_Points;
	void SetIterator(PS_FloatUV& uv, U32 i) const;
public:
	void Mask(CDC* pDC, CSize& size, CSize& bmsize) const;
	PS_Polygon(void);
	PS_Polygon(const PS_Polygon& source);
	void CopyFrom(const PS_Polygon& source);
	CRect Rect(CSize size) const;
	CRect PositiveRect(CSize size) const;
	PS_Polygon& operator << (const PS_FloatUVOrigin& uv);
	const PS_Polygon& operator >> (PS_FloatUVOrigin& uv) const;
	PS_Polygon& operator << (const PS_FloatUV& uv);
	const PS_Polygon& operator >> (PS_FloatUV& uv) const;
	PS_Polygon& operator = (const PS_Polygon& x);
	operator BOOL (void) const;
	void InsertAt(U32 i, PS_FloatUV& uv);
	void RemoveAt(U32 i);
	S32  FindUV(PS_FloatUV& uv) const;
	S32  GetNumberOfVertices(void) const { return m_Points.GetSize(); }
	const PS_FloatUV& operator[](U32 position) const { return m_Points[position]; }
	PS_FloatUV& operator[](U32 position) { return m_Points[position]; }
	friend JC_Buffer& operator << (JC_Buffer& buf, PS_Polygon& p);
	friend JC_Buffer& operator >> (JC_Buffer& buf, PS_Polygon& p);
};
//------------------------------------------------------------------------------------------------
class JCCLASS_DLL PS_Texture : public JC_PropertyRecord
{
	JC_RecordRef m_Bitmap;		// database bitmap object pointed to by ref
	JC_RecordRef m_Palette;		// optional palette object(s) used by ref
	PS_Polygon	 m_Shape;		// definition of part texture to use
public:
	PS_Texture(PS_Database * db, U32 recnum);
	PS_Texture(PS_Database * db, U32 recnum, U32 bitmaprec);
	void SetBitmapID(U32 id);
	void ReadFaultyBody(JC_Buffer& buf);
	const PS_Texture& operator >> (PS_Polygon& p) const;
	PS_Texture& operator << (const PS_Polygon& p);
	bool DeleteRecord(void);
public:
	const PS_FloatUV& UV(U16 index) const;
	// virtual overrides
	virtual bool Delete(void);
	virtual void ReadBody (void);
	virtual	void WriteBody(void);
	virtual void Dependencies(PS_CompressRecordList * list) const;
	virtual void CopyAcross(JC_UpgradeContext& upgrade);
	virtual bool CopyFromRecord(const JC_Record * source, PS_RecordRemap& copy_map);
	virtual void GetTextureList(PS_UniqueRecordList * list) const;
	virtual U16 Type(void) const;
	virtual ~PS_Texture(void);
	virtual void RenderIcon(CDC * dc, const CRect& rect);
	virtual void SelectIcon(BOOL sel);
	virtual CString TypeString(void) const		{ return CString("Texture"); }
	virtual void ResolveRefs(JC_UpgradeContext& upgrade);
	// virtual functions
	virtual U16 GetNumberOfVertices(void) const { return static_cast <U16>(m_Shape.GetNumberOfVertices()); }
	virtual JC_RecordRef * Palette(void) const { return &(((PS_Texture*)this)->m_Palette); }
	virtual CBitmap * GetBitmap(CDC * pDC, CSize& size) const;
	virtual void    Render(CDC * pDC, CRect & dest) const;
	virtual U32		GetBitmapID(void) const;
	virtual BOOL IsAnimation(void) const;
	virtual const JC_Bitmap * Bitmap(void) const;
	virtual void GetFloatUVArray(PS_FloatUVArray& array) const;
	virtual void GetPositiveFloatUVArray(PS_FloatUVArray& array) const;
	virtual U32  Synchronise(void);
};
//------------------------------------------------------------------------------------------------
class JCCLASS_DLL PS_TextureAnimationCel
{
public:
	enum PlayMode { Forward, Backward, PingPong };
private:
	PS_Database *	m_pDB;      // Pointer to database : used when loading refs
	JC_RecordRef	m_Texture;	// Pointer to a texture reference object
	U32				m_Cycles;		
	U32				m_Repeats;
	JC_RecordRef	m_Palette;   // ID of a palette reference to select
	PlayMode		m_PlayMode;
public:
	void SetTextureID(const U32 new_texture_number){ m_Texture.RecordNum(new_texture_number); }
	virtual void CopyAcross(JC_UpgradeContext& upgrade);
	void Dependencies(PS_CompressRecordList * list) const;
	void GetTextureList(PS_UniqueRecordList * list) const;
	PlayMode GetPlayMode() const;
	void	SetPlayMode(PlayMode mode);
	PS_Texture * GetTexture() const;
	U32		GetCycles()       const;
	U32		GetPaletteID()    const;
	void	SetCycles(U32 cycles);
	void	SetPaletteID(U32 id);
	void	Repeats(S32 repeats);
	S32		Repeats(void);
	PS_Texture * GetCurrentTexture() const;
	PS_TextureAnimationCel(PS_Database * pDB);
	PS_TextureAnimationCel(PS_Texture * texture, U32 cycles = 50, PlayMode playmode = Forward, 
						   U32 repeats = 0, U32 palrec = NULL);
	~PS_TextureAnimationCel(void);

	void ResolveRefs(JC_UpgradeContext& upgrade);

	friend JC_Buffer& operator << (JC_Buffer& buf, PS_TextureAnimationCel* a);
	friend JC_Buffer& operator >> (JC_Buffer& buf, PS_TextureAnimationCel* a);
};
//------------------------------------------------------------------------------------------------
typedef CTypedPtrArray <CPtrArray, PS_TextureAnimationCel*> PS_TextureAnimationArray;
typedef CTypedPtrArray <CPtrArray, PS_Texture*> PS_TextureArray;
//------------------------------------------------------------------------------------------------
class JCCLASS_DLL PS_TextureAnimation : public PS_Texture
{
public:
	enum PlayMode { Forward, Backward, PingPong };
private:
	PS_TextureAnimationArray	Cels;
	S32							m_Cel; // current cel
	double						m_RemainingDuration;
	PlayMode					m_Direction; // current direction
	PlayMode					m_RepeatMode;
	S32							m_Repeats; // current repeats left
	U16							m_AllRefCount;
public:
	PS_TextureAnimation(PS_Database * db, U32 recnum);
	void ReadFaultyBody(JC_Buffer& buf);
	void Put(void);
	void Clear(void);
	bool RemoveAllCelsUsingTexture(const PS_Texture * texture);
	// record overrides
	virtual bool CopyFromRecord(const JC_Record * source, PS_RecordRemap& copy_map);
	virtual bool CreateCopyRecordList(CDWordArray& list) const;
	virtual BOOL IsAnimation(void) const;
	virtual void ReadBody (void);
	virtual	void WriteBody(void);
	virtual void Dependencies(PS_CompressRecordList * list) const;
	virtual void GetTextureList(PS_UniqueRecordList * list) const;
	virtual U16 Type(void) const;
	virtual ~PS_TextureAnimation(void); 
	virtual void ResolveRefs(JC_UpgradeContext& upgrade);
	// texture overrides
	virtual U16 GetNumberOfVertices(void) const;
	virtual JC_RecordRef * Palette(void) const;
	virtual CBitmap * GetBitmap(CDC * pDC, CSize& size) const;
	virtual void    Render(CDC * pDC, CRect & dest) const;
	virtual U32		GetBitmapID(void) const;
	virtual const JC_Bitmap * Bitmap(void) const;
	virtual void GetFloatUVArray(PS_FloatUVArray& array) const;
	virtual void CopyAcross(JC_UpgradeContext& upgrade);
	virtual bool Initialise(void);
public:
	void ReferenceAll(void) const;
	void ReleaseAll(void) const;
	BOOL IsFinished(void) const;
	void StepOn(double time, BOOL& changed);
	BOOL GetNextCel(void);
	void Reset(PS_TextureAnimationCel* pcel, PlayMode direction);
	void Reset(void);
	S32 Repeats(void) const;
	void Repeats(S32 repeats);
	PlayMode Direction(void) const;
	PlayMode RepeatMode(void) const;
	void RepeatMode(PlayMode mode);
	void Direction(PlayMode direction);
	S32 GetTotalCels(void);
	void SetCurrentCel(S32 cel);
	PS_TextureAnimationCel* GetCurrentCel(void) const;
	PS_TextureAnimationCel* GetCel(S32 index) const;
	PS_Texture * GetCurrentTexture(void) const;
	PS_Texture * GetStaticTexture(void) const;
	void AddCel(S32 index, PS_TextureAnimationCel* pcel);
	void RemoveCel(S32 index);
	void DetachCel(S32 index);
	S32 GetNumberOfCels(void) const;
	virtual U32  Synchronise(void) { return JC_PropertyRecord::Synchronise(); }
};
//------------------------------------------------------------------------------------------------
class JCCLASS_DLL PS_Folder : public JC_PropertyRecord
{
private:
	JC_RecordRefList m_ChildList;
	U32				 m_Status;		//. For Explorer View

public:
	PS_Folder(PS_Database * db, U32 recnum);
	virtual ~PS_Folder(void){}

	static bool ScanForRecordsToDelete(PS_RecordList& initial_delete_list, PS_RecordList& total_delete_list);
	static bool MultipleRecordDelete(PS_RecordList& delete_list);

	//. Binary status flags
	enum StatusBits { NONE = 0,  REBUILD_TREE = 0x0001,  REFRESH_LIST = 0x0002 };

	U32					GetStatus() const				{ return m_Status;		}
	void				AddStatus(U32 flags)			{ m_Status |= flags;	}
	void				ClearStatus()					{ m_Status = NONE;		}

	virtual void		Contain(U32 recnum);
	void				Discard(U32 recnum);
	bool				Delete(U32 record);
	JC_RecordRefList&	ChildList(void);
	const JC_RecordRefList& ChildList(void) const;
	U32					FindRecord(const CString& name, const U32 type) const;
	U32					FindRecordInTree(const CString& name, U32 type) const;
	POSITION			FindFirstRecord(const CString& name, const U32 type) const;
	U32					GetNextRecord(const CString& name, const U32 type, POSITION& pos) const;
	BOOL				ContainsRecord(U32 recnum, BOOL recurse_tree = FALSE) const;	
	
	virtual bool Delete(void);
	virtual void AddToDeleteList(PS_RecordList& delete_list) const;
	virtual bool CreateCopyRecordList(CDWordArray& list) const;
	virtual bool CopyFromRecord(const JC_Record * source, PS_RecordRemap& copy_map);
	virtual void ReportRecordRef(JC_RecordRefList& list);
	virtual void ReadBody (void);
	virtual void Dependencies(PS_CompressRecordList * list) const;
	void ReadFaultyBody(JC_Buffer& buf);
	virtual	void WriteBody(void);
	virtual U16 Type(void) const;
	virtual BOOL IsFolder() const				{ return TRUE; }
	virtual CString TypeString(void) const		{ return CString("Folder"); }
	virtual void CopyBodyAcross(JC_UpgradeContext& upgrade, JC_Record * destrecord);
	virtual void ResolveRefs(JC_UpgradeContext& upgrade);
	virtual U32  Synchronise();
	U32 FindSameNameAndType(JC_Record * record) const;
	bool ContainsSameNameAndType(JC_Record * record) const;
	void ReplaceRecord(JC_Record * record);
};
//------------------------------------------------------------------------------------------------
class JCCLASS_DLL PS_Project : public PS_Folder
{
public:
	BA_3DViewStyleCollection* D3DViewStyles(void);
	PS_Project(PS_Database * db, U32 recnum);
	virtual ~PS_Project(void){}
	//. For use in database upgrades
	U16		GetVersion()				{ return m_Version;		}
	void	SetVersion(U16 version)		{ m_Version = version;	}

	virtual void WriteBody(void);
	virtual void ReadBody(void);
	virtual void Dependencies(PS_CompressRecordList * list) const;
	virtual bool CopyFromRecord(const JC_Record * source, PS_RecordRemap& copy_map);
		
	virtual U16 Type(void) const;
	virtual CString TypeString(void) const		{ return CString("Project Folder"); }
	virtual void CopyAcross(JC_UpgradeContext& upgrade);
	virtual BOOL IsInProject() const			{ return TRUE;	}
	virtual bool RemovePropertyNameReference(const JC_PropertyName * name);

protected:
	BA_3DViewStyleCollection m_D3DViewStyles;
};
//------------------------------------------------------------------------------------------------
class JC_TextureSet;
//------------------------------------------------------------------------------------------------
class JCCLASS_DLL PS_TextureSetFolder : public PS_Folder
{
public:
	PS_TextureSetFolder(PS_Database * db, U32 recnum);
	virtual ~PS_TextureSetFolder(void){}
	virtual U16 Type(void) const;
	virtual void Contain(U32 recnum);
	virtual bool CopyFromRecord(const JC_Record * source, PS_RecordRemap& copy_map);
	JC_TextureSet * TextureSetContaining(const PS_Texture * texture) const;
	JC_TextureSet * TextureSetContaining(U32 record) const;
	virtual CString TypeString(void) const		{ return CString("TextureSet Folder"); }
};

#endif /* DATA.H */