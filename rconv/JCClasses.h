//-------------------------------------------------------------------------------------------------------------------------------
//
//                           Habitat classes written by J.Croudy for Probe Entertainment
//
//
//            ************** If you modify this file, record a new version in JCClasses.cpp ***************
//
//-------------------------------------------------------------------------------------------------------------------------------
#ifndef jcclasses_h
#define jcclasses_h
//-------------------------------------------------------------------------------------------------------------------------------
#include <database.h>
#include <strstrea.h>
#define D3D_OVERLOADS
#include <d3drm.h>
#include <d3drmdef.h>
//-------------------------------------------------------------------------------------------------------------------------------
const U16 CURRENT_BITMAP_VERSION = 0x0003;
const U16 CURRENT_PROPNAME_VERSION = 0x0004;
const U16 CURRENT_MODEL_VERSION = 0x0006;
const U16 CURRENT_ENUM_RECORD_VERSION = 0x0002;
//-------------------------------------------------------------------------------------------------------------------------------
class	JC_Pin;
class	JC_Data;
class	JC_Face;
class	JC_List;
class	JC_Vertex;
class	JC_Bitmap;
class	JC_Template;
class	JC_FaceList;
class	JC_DataConcrete;
class	JC_PropertyList;
class	JC_RecordRefList;
class	AD_LightTypeList;
class	PS_Folder;
class   PS_Texture;
class	PS_UniqueRecordList;
class	PS_CompressRecordList;
class	JC_Object;
class	JC_EnumList;
class	JC_PropertyName;
class	JC_UpgradeContext;
class	JC_EnumRecord;
class	JC_DialogTemplateRecord;
//-------------------------------------------------------------------------------------------------------------------------------
#define JC_ABS(a)		((a) < 0 ? -(a) : (a))						// Absolute value
#define JC_SWAP(a, b)	{(a)^=(b);(b)^=(a);(a)^=(b);}				// Swap values
#define JC_MIN(a, b)	((a) < (b) ? (a) : (b))						// Find minimum input
#define JC_MAX(a, b)	((a) > (b) ? (a) : (b))						// Find maximum input
#define JC_CLAMP(v,l,h)	((v)<(l) ? (l) : (v) > (h) ? (h) : (v))		// Clamp the input to the specified range
//-------------------------------------------------------------------------------------------------------------------------
enum SyncReturns { SYNC_OK, SYNC_SAVE_ME, SYNC_DELETE_ME };
//-------------------------------------------------------------------------------------------------------------------------
inline int LastSlashPos(const CString& string) { return string.ReverseFind('\\'); }
//-------------------------------------------------------------------------------------------------------------------------
inline const char * JC_Plural(int num)	// Handy function for "File" or Files" type output.
{
	return (num == 1) ? "" : "s";		// Usage: string.Format("%d file%s", num, JC_Plural(num));
}
//-------------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_UseDLLResources
{
private:
	HINSTANCE m_Instance;

public:
	JC_UseDLLResources();
	virtual ~JC_UseDLLResources();
};
//-------------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_Exception : public CException
{
private:
	CString m_Message;
	DECLARE_DYNAMIC(JC_Exception)

public:
	JC_Exception(const CString& message);

	CString Message(void)	{ return m_Message; }
};
//-------------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_Buffer			// General purpose buffer
{
private:
	enum { BlockSize = 1024 };

private:
	U8* m_AllocBuf;					// Ptr to allocated buffer
	U32	m_AllocLen;					// Length of allocated buffer
	U32	m_Position;					// Buffer position to read or write at
	U32	m_HighWater;				// Used length of buffer (high water mark)

public:
	JC_Buffer();
	virtual ~JC_Buffer();

	void	Insert(U8 x)						{ Insert((U8*)&x, sizeof(U8)); }
	void	Insert(S8 x)						{ Insert((U8*)&x, sizeof(S8)); }
	void	Insert(U16 x)						{ Insert((U8*)&x, sizeof(U16)); }
	void	Insert(S16 x)						{ Insert((U8*)&x, sizeof(S16)); }
	void	Insert(U32 x)						{ Insert((U8*)&x, sizeof(U32)); }
	void	Insert(S32 x)						{ Insert((U8*)&x, sizeof(S32)); }
	void	Insert(BOOL x)						{ Insert((U8*)&x, sizeof(BOOL)); }
	void	Insert(float x)						{ Insert((U8*)&x, sizeof(float)); }
	void	Insert(double x)					{ Insert((U8*)&x, sizeof(double)); }
	void	Insert(const CRect& rect);
	void	Insert(const CString& string);
	void	Insert(const U8 * buf, U32 numbytes);
	void	InsertAlign(int boundary);
	void	InsertUnicode(const CString& s);

	void	Extract(U8& x)						{ Extract((U8*)&x, sizeof(U8)); }
	void	Extract(S8& x)						{ Extract((U8*)&x, sizeof(S8)); }
	void	Extract(U16& x)						{ Extract((U8*)&x, sizeof(U16)); }
	void	Extract(S16& x)						{ Extract((U8*)&x, sizeof(S16)); }
	void	Extract(U32& x)						{ Extract((U8*)&x, sizeof(U32)); }
	void	Extract(S32& x)						{ Extract((U8*)&x, sizeof(S32)); }
	void	Extract(BOOL& x)					{ Extract((U8*)&x, sizeof(BOOL)); }
	void	Extract(float& x)					{ Extract((U8*)&x, sizeof(float)); }
	void	Extract(double& x)					{ Extract((U8*)&x, sizeof(double)); }
	void	Extract(CRect& rect);
	void	Extract(CString& string);
	void	Extract(U8 * buf, U32 numbytes);

	void	Seek(U32 pos);
	U32		Tell(void)			const	{ return m_Position; }
	U32		Length(void)		const	{ return m_HighWater; }
	void	Length(U32 len);
	BOOL	IsEmpty(void)		const	{ return (BOOL)(m_Position == m_HighWater); }
	const U8 * GetBuffer(void)	const	{ return m_AllocBuf; }
	HGLOBAL GetGlobalData(void) const;
	void    PutGlobalData(HGLOBAL data);

	JC_Buffer& operator << (U8  x)		{ Insert(x); return *this; }
	JC_Buffer& operator << (S8  x)		{ Insert(x); return *this; }
	JC_Buffer& operator << (U16 x)		{ Insert(x); return *this; }
	JC_Buffer& operator << (S16 x)		{ Insert(x); return *this; }
	JC_Buffer& operator << (U32 x)		{ Insert(x); return *this; }
	JC_Buffer& operator << (S32 x)		{ Insert(x); return *this; }
	JC_Buffer& operator << (BOOL x)		{ Insert(x); return *this; }
	JC_Buffer& operator << (float x)	{ Insert(x); return *this; }
	JC_Buffer& operator << (double x)	{ Insert(x); return *this; }
	JC_Buffer& operator << (const CRect& x)		{  Insert(x); return *this; }
	JC_Buffer& operator << (const CString& x)	{  Insert(x); return *this; }

	JC_Buffer& operator >> (U8&  x)		{ Extract(x); return *this; }
	JC_Buffer& operator >> (S8&  x)		{ Extract(x); return *this; }
	JC_Buffer& operator >> (U16& x)		{ Extract(x); return *this; }
	JC_Buffer& operator >> (S16& x)		{ Extract(x); return *this; }
	JC_Buffer& operator >> (U32& x)		{ Extract(x); return *this; }
	JC_Buffer& operator >> (S32& x)		{ Extract(x); return *this; }
	JC_Buffer& operator >> (BOOL& x)	{ Extract(x); return *this; }
	JC_Buffer& operator >> (float& x)	{ Extract(x); return *this; }
	JC_Buffer& operator >> (double& x)	{ Extract(x); return *this; }
	JC_Buffer& operator >> (CRect& x)			{ Extract(x); return *this; }
	JC_Buffer& operator >> (CString& x)			{ Extract(x); return *this; }
};
//-------------------------------------------------------------------------------------------------------------------------
// JC_ListElement - Anything that can be inserted in a JC_List must be derived from this
//-------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_ListElement
{
public:
	virtual ~JC_ListElement()	{ }

public:	// Virtual functions
	virtual void Insert(JC_Buffer& buf) const = 0;
	virtual void Extract(JC_Buffer& buf) = 0;
	virtual U8 ListElementType(void)	const	{ return 0; }	// Override this if you have different kinds of
																// derived class for the base list element class
																// For example, see JC_Vertex and JC_Pin

	//. If you need to be able to copy your lists, you must provide a copy function for your derived class.
	virtual void Copy(const JC_ListElement *src);

	// Override if your list element contains record references.
	virtual void ResolveRefs(JC_UpgradeContext&, int /*item*/=-1, JC_List * /*parentlist*/=NULL) { }

	virtual void Dependencies(PS_CompressRecordList * /*list*/) const {}

	friend JC_Buffer& operator << (JC_Buffer& buf, const JC_ListElement& le)	{ le.Insert(buf); return buf; }
	friend JC_Buffer& operator >> (JC_Buffer& buf, JC_ListElement& le)			{ le.Extract(buf); return buf; }
};
//-------------------------------------------------------------------------------------------------------------------------
// JC_List - General purpose pointer list which OWNS it's elements
//-------------------------------------------------------------------------------------------------------------------------
class PS_ElementList : public CTypedPtrList<CPtrList, JC_ListElement*>
{
};

class JCCLASS_DLL JC_List : public PS_ElementList
{
public:
	JC_List() { }
	virtual ~JC_List() { Empty(); }

	void Empty(void);
	void Copy(const JC_List *src);
	void Redirect(JC_ListElement * dest, JC_ListElement * srce);	// Not properly tested
	void ResolveRefs(JC_UpgradeContext& upgrade);

public:	// Virtual functions
	virtual void Insert(JC_Buffer& buf) const;
	virtual void Extract(JC_Buffer& buf);
	virtual void Dependencies(PS_CompressRecordList * list) const;
	virtual JC_ListElement * NewType(U8 kind) = 0;
	virtual POSITION AttachTail(JC_ListElement * le);

	friend JC_Buffer& operator << (JC_Buffer& buf, const JC_List& list)	{ list.Insert(buf); return buf; }
	friend JC_Buffer& operator >> (JC_Buffer& buf, JC_List& list)	{ list.Extract(buf); return buf; }

protected:
	PS_ElementList m_BackupList;

	POSITION AddTail(JC_ListElement * new_element);
public:
	POSITION AddHead(JC_ListElement * new_element);
	POSITION Find(JC_ListElement * search_value, POSITION start_after = NULL) const;
	POSITION FindIndex(int index) const;
	JC_ListElement *& GetAt(POSITION pos);
	JC_ListElement *  GetAt(POSITION pos) const;
	int GetCount() const;
	JC_ListElement *& GetHead();
	JC_ListElement *  GetHead() const;
	POSITION GetHeadPosition() const;
	JC_ListElement *& GetNext(POSITION& pos);
	JC_ListElement *  GetNext(POSITION& pos) const;
	JC_ListElement *& GetPrev(POSITION& pos);
	JC_ListElement *  GetPrev(POSITION& pos) const;
	JC_ListElement *& GetTail();
	JC_ListElement *  GetTail() const;
	POSITION GetTailPosition() const;
	POSITION InsertAfter(POSITION pos, JC_ListElement * new_element);
	POSITION InsertBefore(POSITION pos, JC_ListElement * new_element);
	BOOL IsEmpty() const;
	void RemoveAll();
	void RemoveAt(POSITION pos);
	JC_ListElement * RemoveHead();
	JC_ListElement * RemoveTail();
	void SetAt(POSITION pos, JC_ListElement * new_element);
protected:
	void CheckElement(POSITION pos) const;
	POSITION BackupPos(POSITION pos) const;
};
//-------------------------------------------------------------------------------------------------------------------------------
// JC_RecordMap - Used by Upgrade to map record numbers between old and new databases.
//-------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_U32Map : public CMap<U32, U32, U32, U32>
{
};
//-------------------------------------------------------------------------------------------------------------------------------
enum JC_Status { Info, Success, Warning, Error };	// In order of increasing severity.
//-------------------------------------------------------------------------------------------------------------------------------
// JC_UpgradeContext - Used by JC_Record derived classes to communicate with JC_Upgrade.
//-------------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_UpgradeContext
{
private:
	int m_Phase;					// Phase number of processing.
	BOOL m_Fatal;					// Flag set if fatal error occurred.
	BOOL m_Verbose;					// Flag set if verbose output required.
	CString m_Path;					// Full path to record being processed (for messages).
	CWnd * m_Output;				// Output window for log entries.
	JC_U32Map m_CopyMap;			// Map to keep track of record numbers and their copy's numbers.
	JC_U32Map m_ResolveMap;			// Map to keep track of reference-resolved records.
	CDWordArray m_DeletedNums;		// Record numbers of deleted records found during resolve refs.
	PS_Database * m_DestDatabase;	// Ptr to destination database.
	PS_Database * m_SrceDatabase;	// Ptr to source database (Source Template in Phase 1, Source Level in Phase 2).
	JC_U32Map m_SearchCache;		// A cache for global searches to speed up processing

public:
	JC_UpgradeContext(CWnd * output, int phase=0, BOOL verbose=FALSE, PS_Database * srcedb=NULL, PS_Database * destdb=NULL);

	BOOL AdvanceProgress(void);
	BOOL IsUpdateResolved(U32 recnum);
	void ResolveUpdate(U32 recnum);
	U32  GetCopyRecordNum(U32 recnum) const;
	void SetCopyRecordNum(U32 recnum, U32 copynum);
	void BeginResolveRefs(void) { m_DeletedNums.RemoveAll(); }
	BOOL HasDeletion(void) const;
	void AssertNoDeletion(const CString& caller, int element=-1);
	int  GetPhase(void) const { return m_Phase; }
	void Enter(JC_Record * record);
	void Exit(void);
	CString GetPath(void) const { return m_Path; }
	void LogDivider(void);
	void LogDeletions(void);
	void Log(const CString& text, JC_Status status = Info);
	void LogWhere(int element, JC_Status status = Info);
	void LogIfVerbose(const CString& text, JC_Status status = Info);
	void RecordDeleted(U32 recnum) { m_DeletedNums.Add(recnum); }
	void FatalError(void) { m_Fatal = TRUE; }
	BOOL MustAbort(void) const { return m_Fatal; }
	int  GetTotalCopied(void) const { return m_CopyMap.GetCount(); }
	bool GetSearchCache(const U32 search_record, U32& match_record) { return (m_SearchCache.Lookup(search_record, match_record) != 0); }
	void SetSearchCache(const U32 search_record, const U32 match_record) { m_SearchCache.SetAt(search_record, match_record); }
	PS_Database * GetDestDatabase(void) const { ASSERT(m_DestDatabase != NULL); return m_DestDatabase; }
	PS_Database * GetSourceDatabase(void) const { ASSERT(m_SrceDatabase != NULL); return m_SrceDatabase; }
};
//-------------------------------------------------------------------------------------------------------------------------------
// JC_Record - Base class for all kinds of database records
//-------------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_Record
{
public:
	enum RecType
	{
		TypeRecord = 0, TypePropertyRecord, TypeTemplate, TypeObject, TypePropertyName, TypeTexture, TypeTextureAnimation, TypeBitmap, 
		TypePalette, TypeFolder, TypeProject, TypeObsoleteTextureSet, TypeDialogTemplateRecord, TypeEnumRecord,
		TypeTextureSetFolder, TypeAnimatedTemplate, TypeAnimatedObject, TypeLink, TypeModel, TypeNewTextureSet,

		// ^ Add new ones here ^

		TypeTotal
	};

private:
	CString	m_Name;							// Record name
	U32		m_Number;						// Record number
	bool	m_Modified;
	U16		m_RefCnt;						// Reference count
	JC_RecordRefList * m_Parents;			// Database folders that contain this record (now only allowed one folder)
	U32		m_TimeToLoad;
	U32		m_TimeReleased;					// Time the record was last released in ticks

protected:
	U16		m_Version;						// Version number
	JC_Buffer * m_Buffer;					// Ptr to buffer from database
	PS_Database * m_Database;				// Ptr to database
	bool	m_IsDeleted;					// used to optimise database scans during delete

public:
	JC_Record(PS_Database * db, U32 recnum);
	virtual ~JC_Record();

	void Write(JC_Buffer& buf);
	void Read(JC_Buffer& buf);

	virtual void Name(const CString& name)	{ m_Name = name; } //. Overridden by rv_Model
	virtual CString Name(void)		 const	{ return m_Name; }
	CString Path(void)			 const;
	U32  Number(void)			 const		{ return m_Number; }
	PS_Database * Database(void) const		{ return m_Database; }

	U16	AddRef(void) const;
	U16 Release(void) const;
	U16 GetRefCnt(void) const				{ return m_RefCnt;   }
	bool IsModified(void) const				{ return m_Modified; }
	void SetModified(void)					{ m_Modified = true; }
	void ClearModified(void)				{ m_Modified = false;}
	BOOL IsHidden(void) const;
	bool IsDeleted(void) const				{ return m_IsDeleted;}
	bool CanFreeMemory(void) const;
	void Clear(void);
	BOOL HasFolder() const;					
	PS_Folder* FolderPtr() const;
	void ResolveRefsPut(JC_UpgradeContext& upgrade);
	JC_RecordRefList * ParentFolders(void)	{ return m_Parents; } //. This should now contain only one folder.

protected:
	virtual void CopyBodyAcross(JC_UpgradeContext& upgrade, JC_Record * destrecord);

public: // Virtual functions
	//. Fun with types
	virtual U16 Type(void)		const		{ return TypeRecord; }
	virtual BOOL IsModel()		const		{ return FALSE;		 }
	virtual BOOL IsTemplate()	const		{ return FALSE;		 }
	virtual BOOL IsObject()		const		{ return FALSE;		 }
	virtual BOOL IsFolder()		const		{ return FALSE;		 }
	bool IsCompatibleRecord(const JC_Record const * source) const;

	virtual bool Delete(void);
	virtual void AddToDeleteList(PS_RecordList& delete_list) const;
	virtual bool CopyFromRecord(const JC_Record * source, PS_RecordRemap& copy_map);
	virtual bool CreateCopyRecordList(CDWordArray& list) const;
	virtual void WriteBody(void);
	virtual void ReadBody(void);
	virtual void Dependencies(PS_CompressRecordList * list) const = 0;
	virtual void Save(void) const;							// Write self to database.
	virtual void AddPropertyNameUsage(JC_RecordRefList& list) const;
	virtual U32  Synchronise(void);
	virtual bool Verify(void) { return true; } // override this to check references
	virtual bool Repair(void) { return true; } // override this to repair damage to a record
	virtual bool Initialise(void) { return true; } // Called after ReadBody to perform any required initialisation

	virtual JC_PropertyList * PropertyList(void)	{ return NULL; }
	virtual CString TypeString(void) const = 0;
	virtual U32 NumberOfRefs(const JC_PropertyName * name);
	virtual bool RemovePropertyNameReference(const JC_PropertyName * name);

	// These functions are used by JC_ScratchPad which holds records and displays them as icons.
	virtual void RenderIcon(CDC * dc, const CRect& rect);	// Called to render the icon imagery on the scratch pad.
	virtual void SelectIcon(BOOL sel);						// Called when a scratch pad icon is selected or deselected.

	// These functions are used by the Upgrade Application.

	// Only overridden by PS_Project because it is an exception to normal record creation.
	virtual void CopyAcross(JC_UpgradeContext& upgrade);
	virtual BOOL IsInProject() const;

	// Only overridden by records that contain record references.
	virtual void ResolveRefs(JC_UpgradeContext& upgrade);

public: // Debugging
	virtual void ReportRecordRef(JC_RecordRefList&)	{ }
};
//-------------------------------------------------------------------------------------------------------------------------------
// JC_RecordRef - Controls access to database records
//-------------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_RecordRef : public JC_ListElement
{
private:
	U32	m_RecordNum;				// Record number in database
	PS_Database * m_Database;		// Ptr to database

public:
	JC_RecordRef(PS_Database * db, U32 recnum = PS_Database::NullRecord);
	virtual ~JC_RecordRef()		{ }

	void Null(void);
	void Clear(void);
	U32	 RecordNum(void)			const		{ return m_RecordNum; }
	void RecordNum(U32 num)						{ m_RecordNum = num; }
	void Database(PS_Database * db)				{ m_Database = db; }
	PS_Database * Database(void)	const		{ return m_Database; }
	BOOL IsRecordNum(U32 num)		const		{ return (BOOL)(m_RecordNum == num); }
	BOOL IsInvalid(void)			const		{ return (BOOL)(m_RecordNum == PS_Database::NullRecord); }

	JC_Record * Get(JC_Record::RecType type) const;

protected:
	U32 DoGlobalSearch(JC_UpgradeContext& upgrade);

public:	// Virtual overrides
	virtual void Insert(JC_Buffer& buf) const { buf << m_RecordNum; }
	virtual void Extract(JC_Buffer& buf);
	virtual void Dependencies(PS_CompressRecordList * list) const;
	virtual void Copy(const JC_ListElement *src_item);
	virtual void CopyAcross(JC_UpgradeContext& upgrade);
	virtual void ResolveRefs(JC_UpgradeContext&, int item=-1, JC_List * parentlist=NULL);

	friend JC_Buffer& operator << (JC_Buffer& buf, const JC_RecordRef& rr)	{ rr.Insert(buf); return buf; }
	friend JC_Buffer& operator >> (JC_Buffer& buf, JC_RecordRef& rr)	{ rr.Extract(buf); return buf; }

public:	// Debugging
	void Report(JC_RecordRefList& rrlist);
};
// JC_DataString
//-------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_DataString : public CString
{
public:
	JC_DataString()					: CString()		{ }
	JC_DataString(const CString& s)	: CString(s)	{ }
	virtual ~JC_DataString()						{ }

	void PutWord(WORD x)				{ CString s; s.Format("%d", x); PutString(s); }
	void PutString(const CString& s)	{ ((CString&)*this) += s + "|"; }
	WORD GetWord(void)					{ CString s = GetString(); return (WORD)atoi((LPCSTR)s); }

	CString GetString(void)
	{
		ASSERT(Find('\n') < 0);			// If this happens you are using \n instead of |
		int pos = Find('|');
		if (pos < 0) THROW(new CException);
		CString s = Left(pos);
		((CString&)*this) = Mid(pos+1);
		return s;
	}

	CString SingleString(void)			// Returns a single string describing contents, even if multiple items
	{
		int pos = Find('|');
		if (pos == (GetLength()-1))
		{
			return Left(pos);
		}
		else return CString("Multiple Names");
	}
};
//-------------------------------------------------------------------------------------------------------------------------
// JC_DataIO - Used to insert and extract JC_Data in a JC_Buffer
//-------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_DataIO
{
private:
	JC_Data * m_OutputData;
	const JC_Data * m_InputData;
	PS_Database * m_Database;

public:
	JC_DataIO(const JC_Data * data)	{ m_InputData = data; m_OutputData = NULL; m_Database = NULL; }	// Input constructor
	JC_DataIO(PS_Database * db)		{ m_InputData = NULL; m_OutputData = NULL; m_Database = db; }	// Output constructor

	void Insert(JC_Buffer& buf) const;
	void Extract(JC_Buffer& buf);

	friend JC_Buffer& operator << (JC_Buffer& buf, const JC_DataIO& di)	{ di.Insert(buf); return buf; }
	friend JC_Buffer& operator >> (JC_Buffer& buf, JC_DataIO& di)		{ di.Extract(buf); return buf; }

	JC_Data * Data(void) const { return m_OutputData; }
};
//-------------------------------------------------------------------------------------------------------------------------
// JC_Data - Base class for any kind of data
//-------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_Data
{
public:
	enum Tok	{	TokNone, TokU8, TokS8, TokU16, TokS16, TokU32, TokS32, TokBool,
					TokFloat, TokDouble, TokBinary, TokString, TokObsolete1, TokObsolete2,
					TokNamedDataList, TokNamedDataArray, TokDialogDataOld, TokEnumRef,
					TokDialogData,
				};
public:
	virtual ~JC_Data() { };

	JC_Data * Duplicate(void) const;

	JC_Data * NewInstance(const CString& text="") const;
	static JC_Data * NewInstance(Tok token, PS_Database * db, const CString& text="");

public:		// Virtual functions
	virtual Tok Token(void) const = 0;
	virtual BOOL IsNumeric(void)  const { return FALSE; }
	virtual BOOL IsConcrete(void) const { return FALSE; }
	virtual void Insert(JC_Buffer& buf) const = 0;
	virtual void Extract(JC_Buffer& buf) = 0;
	virtual void Clear(void) { }									// Only overridden by types that can be 'cleared'.
	virtual PS_Database * GetDatabase(void) const { return NULL; }	// Only overridden by types that have a database ptr.
	virtual CString RangeString(void) const { return "Unknown range"; }
	virtual CString TypeString(void)  const { return "Unknown type"; }
	virtual CString ShortTypeString(void) const { return "Unknown"; }
	virtual CString String(void)	  const { return "Not a string"; }

	// Override this in any derived class that manipulates the dreaded JC_RecordRef.
	virtual void ResolveRefs(JC_UpgradeContext&) { }
};
//-------------------------------------------------------------------------------------------------------------------------
// JC_DataConcrete - Base class for any kind of concrete data that has a value
//-------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_DataConcrete : public JC_Data
{
protected:
	static char m_Format;

public:
	virtual ~JC_DataConcrete() { };

	double Validate(double value, Tok token);
	static void SetFormat(char c)				{ m_Format = c; }

	char * CharString(void)	const
	{
		static char buf[256];
		CString s = String();
		strcpy(buf, (LPCSTR)s);
		return buf;
	}

	JC_DataConcrete * Duplicate(void)						const { return (JC_DataConcrete*)JC_Data::Duplicate(); }
	JC_DataConcrete * NewInstance(const CString& text="")	const { return (JC_DataConcrete*)JC_Data::NewInstance(text); }

public:		// Virtual functions
	virtual void	Set(U8 *, U32, BOOL)		{ ASSERT(FALSE); }
	virtual void *	GetData(void)				{ ASSERT(FALSE); return NULL; }
	virtual void	Delete(void)				{ ASSERT(FALSE); }

	virtual operator U8		(void) const = 0;
	virtual operator S8		(void) const = 0;
	virtual operator U16	(void) const = 0;
	virtual operator S16	(void) const = 0;
	virtual operator U32	(void) const = 0;
	virtual operator S32	(void) const = 0;
	virtual operator BOOL	(void) const = 0;
	virtual operator float	(void) const = 0;
	virtual operator double	(void) const = 0;
	virtual operator void * (void) const { return NULL; }
	virtual BOOL IsConcrete (void) const { return TRUE; }

protected:
	double Convert(const CString& str);
};
//-------------------------------------------------------------------------------------------------------------------------------
// JC_Property
//-------------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_Property : public JC_ListElement
{
public:
	enum Switch { Self, Parent, Template };

private:
	JC_RecordRef m_Name;						// Reference to property name record
	JC_Data	*	 m_Value;						// NULL if default is to be used
	Switch		 m_Switch;						// Switch for redirection

public:
	JC_Property(PS_Database * pndb);			// Only used for loading from database
	JC_Property(PS_Database * pndb, U32 pnrecnum, JC_Data * value);
	virtual ~JC_Property();

	void	SetSwitch(Switch s)					{ m_Switch = s; }
	Switch	GetSwitch(void)			 const		{ return m_Switch; }

	BOOL IsNew(void) const;
	void SetValue(JC_Data * v);
	BOOL ValueIsDefault(void)		 const;
	JC_Data * CopyValue(void)		 const;		// Get copy of value
	JC_Data * GetValueExact(void)	 const;		// Get value but Property still owns it
	JC_Data * OwnValue(void);					// Get value and OWN it
	const CString Name(void)		 const;
	U32 GetNameRecordNumber(void) const { return m_Name.RecordNum(); }
	JC_Property * Duplicate(void)	 const;
	const CString SwitchString(void) const;
	void SetName(JC_RecordRef& rr);
	void ReleaseName(void);
	bool IsDialogData(void) const;
	bool IsEnumerated(void) const;
	JC_EnumRecord * GetEnumRecord(void) const;
	JC_DialogTemplateRecord * GetDialogTemplate(void) const;

public:	// Virtual overrides
	void Insert(JC_Buffer& buf)	 const;
	void Extract(JC_Buffer& buf);
	virtual void Dependencies(PS_CompressRecordList * list) const;
	virtual void ResolveRefs(JC_UpgradeContext&, int item=-1, JC_List * parentlist=NULL);

	friend JC_Buffer& operator << (JC_Buffer& buf, const JC_Property& prop)	{ prop.Insert(buf); return buf; }
	friend JC_Buffer& operator >> (JC_Buffer& buf, JC_Property& prop)	{ prop.Extract(buf); return buf; }

public: // Debugging
	void ReportRecordRef(JC_RecordRefList& rrlist);

private:
	void ClearValue(void);
	void Set(PS_Database * pndb, U32 pnrecnum, JC_Data * value);
};
//-------------------------------------------------------------------------------------------------------------------------------
// JC_PropertyRecord - Base class for objects which are database records with properties
//-------------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_PropertyRecord : public JC_Record
{
protected:
	JC_PropertyList * m_PropertyList;

public:
	JC_PropertyRecord(PS_Database * db, U32 recnum);
	virtual ~JC_PropertyRecord();

	JC_Data *	CopyPropertyInfo(const CString& name, JC_Property::Switch& sw);
	void		SetPropertyInfo(const CString& name, JC_Property::Switch& sw, JC_Data * data);
	JC_PropertyList * PropertyList(void)		{ return m_PropertyList; }
	void		CopyPropertyList(const JC_PropertyRecord& source);
	void		Clear(void);

public: // Virtual overrides
	virtual JC_PropertyList * CreateFullPropertyList(void) const;
	virtual JC_PropertyList * CreateBackgroundPropertyList(void) const;
	virtual bool CopyFromRecord(const JC_Record * source, PS_RecordRemap& copy_map);
	virtual void WriteBody(void);
	virtual void ReadBody(void);
	virtual U16 Type(void) const { return TypePropertyRecord; }
	virtual void Dependencies(PS_CompressRecordList * list) const;
	virtual U32 NumberOfRefs(const JC_PropertyName * name);
	virtual bool RemovePropertyNameReference(const JC_PropertyName * name);
	virtual void AddPropertyNameUsage(JC_RecordRefList& list) const;
	virtual void ResolveRefs(JC_UpgradeContext& upgrade);

public: // Debugging
	virtual void ReportRecordRef(JC_RecordRefList&);
};
//-------------------------------------------------------------------------------------------------------------------------------
// JC_PropertyName - Name of a JC_Property and it's default value
//-------------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_PropertyName : public JC_Record
{
private:
	JC_Data * m_Default;	// Default value created by caller, deleted by this
	U32 m_UseCnt;			// Use count to keep track of usage by properties.

	// Numeric Property
	double m_TopNumericValue;
	double m_BottomNumericValue;

	COLORREF m_TopNumericColour;
	COLORREF m_BottomNumericColour;


public:
	JC_PropertyName(PS_Database * db, U32 recnum);
	virtual ~JC_PropertyName();

	virtual bool CopyFromRecord(const JC_Record * source, PS_RecordRemap& copy_map);
	void Default(JC_Data * def);
	JC_Data * CopyDefault(void) const;
	U32 AddUse(void);
	U32 ReleaseUse(void);
	int GetUseCnt(void)		const { return m_UseCnt; }
	BOOL IsInUse(void)		const { return (BOOL)(m_UseCnt > 0); }
	BOOL HasDefault(void)	const { return (BOOL)(m_Default != NULL); }
	JC_Data::Tok DataType() const { return (m_Default ? m_Default->Token() : JC_Data::TokNone); }
	bool IsDialogData(void) const;
	bool IsEnumerated(void) const;
	JC_EnumRecord * GetEnumRecord(void) const;
	U32 GetEnumRecordNumber(void) const;
	void SetEnumRecordNumber(U32 rec_num);
	void SetEnumRecord(JC_EnumRecord * record);
	JC_DialogTemplateRecord * GetDialogTemplate(void) const;

	double GetTopNumericValue(void)		  { return m_TopNumericValue; }
	double GetBottomNumericValue(void)	  { return m_BottomNumericValue; }
	COLORREF GetTopNumericColour(void)	  { return m_TopNumericColour; } 
	COLORREF GetBottomNumericColour(void) {	return m_BottomNumericColour; }

	void SetTopNumericValue(double NewTopNumericValue)		     { m_TopNumericValue = NewTopNumericValue; }
	void SetBottomNumericValue(double NewBottomNumericValue)     { m_BottomNumericValue = NewBottomNumericValue; }
	void SetTopNumericColour(COLORREF NewTopNumericColour)		 { m_TopNumericColour = NewTopNumericColour; } 
	void SetBottomNumericColour(COLORREF NewBottomNumericColour) { m_BottomNumericColour = NewBottomNumericColour; }

public: // Virtual overrides
	virtual bool Delete(void);
	virtual void WriteBody(void);
	virtual void ReadBody(void);
	virtual void Dependencies(PS_CompressRecordList * list) const;
	virtual U16 Type(void)			const	{ return TypePropertyName; }
	virtual CString TypeString(void)const	{ return CString("Property Name"); }
	virtual void ResolveRefs(JC_UpgradeContext& upgrade);

protected:
	void ScanReferenceCount(void);
};
//-------------------------------------------------------------------------------------------------------------------------------
// JC_PropertyList - List of JC_Property
//-------------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_PropertyList : public JC_List
{
private:
	PS_Database * m_Database;				// Ptr to database

public:
	JC_PropertyList(PS_Database * db)		{ m_Database = db; }
	JC_PropertyList(const JC_PropertyList * pl);
	virtual ~JC_PropertyList()				{ }
	
	void AddLayer(const JC_PropertyList * source_list);
	BOOL AllNew(void) const;
	int GetDefaultCount(void) const;
	JC_Property * FindProperty(const CString& name);
	JC_DataConcrete * CopyCommonDataType(void) const;
	PS_Database * Database(void)	const	{ return m_Database; }
	JC_ListElement * NewType(U8)			{ return new JC_Property(m_Database); }	// There is only one kind of JC_Property
	U32 NumberOfRefs(const JC_PropertyName * name);
	bool RemovePropertyNameReference(const JC_PropertyName * name);
	virtual void AddPropertyNameUsage(JC_RecordRefList& list) const;

public: // Debugging
	void ReportRecordRef(JC_RecordRefList& rrlist);
};
//-------------------------------------------------------------------------------------------------------------------------------
// AD_LightTypeList - List of AD_LightType
//-------------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL AD_LightTypeList : public JC_List
{
public:
	AD_LightTypeList(void){ }
	AD_LightTypeList(const AD_LightTypeList& source);
	void CopyFrom(const AD_LightTypeList& source);
	virtual ~AD_LightTypeList()				{ }
	JC_ListElement * NewType(U8);
};
//-------------------------------------------------------------------------------------------------------------------------------
// JC_VertexList - List of JC_Vertex or JC_Pin
//-------------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_VertexList : public JC_List
{
public:
	enum Ident { InvalidIdent = 0, FirstValidIdent };

private:
	U32	m_IdentSeq;		// Next unique identifier for a vertex

public:
	JC_VertexList()						{ m_IdentSeq = FirstValidIdent; }
	virtual ~JC_VertexList()			{ }

public:	// Virtual overrides
	void Insert(JC_Buffer& buf) const;
	void Extract(JC_Buffer& buf);
	void AttachTail(JC_Pin * p)		{ AttachTail((JC_ListElement*)p); }
	void AttachTail(JC_Vertex * v)	{ AttachTail((JC_ListElement*)v); }

	JC_ListElement * NewType(U8 kind);

private:
	POSITION AttachTail(JC_ListElement * le);
};
//-------------------------------------------------------------------------------------------------------------------------------
// JC_Vector - A vector in 3D world space
//-------------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_Vector
{
private:
	double m_X;
	double m_Y;
	double m_Z;
	static double  m_Tolerance; 


public:
	JC_Vector(double x=0.0, double y=0.0, double z=0.0)	{ Set(x, y, z); }
	JC_Vector(const D3DVECTOR& vector) { AssignFromD3D(vector); }
	void AssignFromD3D(const D3DVECTOR& v){ Set(v.x, v.y, v.z); }
	virtual ~JC_Vector()				{ }


	double X(void)		const			{ return m_X; }
	double Y(void)		const			{ return m_Y; }
	double Z(void)		const			{ return m_Z; }

	void X(double x)					{ m_X = x; }
	void Y(double y)					{ m_Y = y; }
	void Z(double z)					{ m_Z = z; }

	void Set(double  x, double  y, double  z)			{ m_X = x; m_Y = y; m_Z = z;		}
	void Get(double& x, double& y, double& z)	const	{ x = m_X; y = m_Y; z = m_Z;		}
	void GetD3DVector(D3DVECTOR& v)	const				{ v.x = (float)m_X; v.y = (float)m_Y; v.z = (float)m_Z;	}

	double		Length()					{ return sqrt(DotProduct(*this)); }
	void		Normalise();
	JC_Vector	CrossProduct(JC_Vector& v);
	double		DotProduct(JC_Vector& v);
	double		DotProduct(D3DVECTOR& v);

	void Null(void)	{ Set(0.0, 0.0, 0.0); }

	void Insert(JC_Buffer& buf)	const	{ buf << m_X << m_Y << m_Z; }
	void Extract(JC_Buffer& buf)		{ buf >> m_X >> m_Y >> m_Z; }

	void	SetTolerance(double val)	{ m_Tolerance = val;	}

	BOOL	EqualsWithinTolerance(const JC_Vector& v) const;

	JC_Vector  operator +  (const JC_Vector& v) const	{ return JC_Vector(m_X + v.m_X, m_Y + v.m_Y, m_Z + v.m_Z); }
	JC_Vector  operator -  (const JC_Vector& v) const	{ return JC_Vector(m_X - v.m_X, m_Y - v.m_Y, m_Z - v.m_Z); }
	JC_Vector& operator += (const JC_Vector& v)			{ m_X += v.m_X; m_Y += v.m_Y; m_Z += v.m_Z; return *this; }
	JC_Vector& operator -= (const JC_Vector& v)			{ m_X -= v.m_X; m_Y -= v.m_Y; m_Z -= v.m_Z; return *this; }
	JC_Vector& operator =  (const D3DVECTOR& vector)	{ AssignFromD3D(vector); return *this; }

	friend JC_Buffer& operator << (JC_Buffer& buf, const JC_Vector& v)	{ v.Insert(buf); return buf; }
	friend JC_Buffer& operator >> (JC_Buffer& buf, JC_Vector& v)		{ v.Extract(buf); return buf; }
};
//-------------------------------------------------------------------------------------------------------------------------------
// JC_Vertex - A vertex in 3D world space
//-------------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_Vertex : public JC_ListElement, public JC_Vector
{
public:
	enum TypeV { NotUsedButDoNotDelete, Vertex, Pin };
private:
	U32	m_Ident;				// Unique identifier

public:
	JC_Vertex(double x=0.0, double y=0.0, double z=0.0) : JC_Vector(x, y, z) { Ident(JC_VertexList::InvalidIdent); }
	JC_Vertex(const D3DVECTOR& vector){ AssignFromD3D(vector); }
	virtual ~JC_Vertex() { }

	U32	 Ident(void)		const	{ return m_Ident; }
	void Ident(U32 id)				{ m_Ident = id; }

	BOOL operator == (const JC_Vertex& v) const;
	JC_Vertex& operator = (const D3DVECTOR& vector){ AssignFromD3D(vector); return *this; }
	void AssignFromD3D(const D3DVECTOR& v){ JC_Vector::AssignFromD3D(v); Ident(JC_VertexList::InvalidIdent); }

public:	// Virtual overrides
	U8 ListElementType(void)	const	{ return (U8)Type(); }

	void Insert(JC_Buffer& buf) const;
	void Extract(JC_Buffer& buf);
	virtual TypeV Type(void) const { return Vertex; }
};
//-------------------------------------------------------------------------------------------------------------------------------
// JC_Pin - A vertex to pin objects to
//-------------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_Pin : public JC_Vertex
{
private:
	U32	m_RefCnt;	// Reference count

public:
	JC_Pin(double x=0.0, double y=0.0, double z=0.0);
	JC_Pin(const D3DVECTOR& vector);
	virtual ~JC_Pin()		{ }
	JC_Pin& operator = (const D3DVECTOR& vector);

	void	Reference(void)				{ m_RefCnt++; }
	void	Dereference(void)			{ m_RefCnt--; }

public:	// Virtual overrides
	void	Insert(JC_Buffer& buf) const;
	void	Extract(JC_Buffer& buf);

	U8 ListElementType(void) const	{ return Pin; }
	TypeV Type(void) const { return Pin; }
};
//-------------------------------------------------------------------------------------------------------------------------------
// JC_Colour - A Red/Green/Blue/Alpha
//-------------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_Colour
{
private:
	U8 m_R;
	U8 m_G;
	U8 m_B;
	U8 m_A;

public:
	#define m 255
	enum // Standard colours
	{
		Black   = RGB(0, 0, 0),
		Red     = RGB(m, 0, 0),
		Green   = RGB(0, m, 0),
		Blue    = RGB(0, 0, m),
		Cyan    = RGB(0, m, m),
		Magenta = RGB(m, 0, m),
		Yellow  = RGB(m, m, 0),
		White   = RGB(m, m, m),
	};
	#undef m

	enum // Default colours
	{
		DataTypeDef = Black,
		DefaultDataDef = Red,
		DateDef = RGB(0, 80, 0),
	};

public:
	JC_Colour()									{ Set(0, 0, 0, 0); }
	JC_Colour(U8 r, U8 g, U8 b, U8 a = 0)		{ Set(r, g, b, a); }
	JC_Colour(const JC_Colour& source)			{ *this = source; }
	JC_Colour(const COLORREF& col)				{ Set(GetRValue(col), GetGValue(col), GetBValue(col)); }
	void Set(U8 r, U8 g, U8 b, U8 a = 0)		{ m_R = r; m_G = g; m_B = b; m_A = a; }
	void Get(U8& r, U8& g, U8& b, U8& a)const	{ r = m_R; g = m_G; b = m_B; a = m_A; }
	void Get(U8& r, U8& g, U8& b)		const	{ r = m_R; g = m_G; b = m_B; }
	COLORREF Get(void)					const	{ return RGB(m_R, m_G, m_B); }
	operator COLORREF (void)			const	{ return Get(); }
	JC_Colour& operator = (const JC_Colour& source);
	JC_Colour Dim(int percent=50) const;
	JC_Colour Bleach(int percent=75) const;
	JC_Colour Interpolate(const JC_Colour& other, int percent) const;

public:	// Static functions
	static JC_Colour Date(void);
	static JC_Colour DefaultData(void);
	static JC_Colour DataType(const JC_DataConcrete * type);

public:	// Virtual overrides
	void Insert(JC_Buffer& buf) const	{ buf << m_R << m_G << m_B << m_A; }
	void Extract(JC_Buffer& buf)		{ buf >> m_R >> m_G >> m_B >> m_A; }

	friend JC_Buffer& operator << (JC_Buffer& buf, const JC_Colour& c)	{ c.Insert(buf); return buf; }
	friend JC_Buffer& operator >> (JC_Buffer& buf, JC_Colour& c)		{ c.Extract(buf); return buf; }
};
//-------------------------------------------------------------------------------------------------------------------------------
// JC_Palette - An array of JC_Colour
//-------------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_Palette : public JC_Record
{
private:
	U16 m_NumColours;
	JC_Colour * m_ColourArray;

public:
	JC_Palette(PS_Database * db, U32 recnum);
	virtual ~JC_Palette();
	virtual void Dependencies(PS_CompressRecordList * list) const;

	void Get(CPalette& pal);
	void Set(const CPalette& pal);
	void Set(const JC_Bitmap * bm);
	void Set(const BITMAPINFO * bi);
	U16	 NumColours(void)	const	{ return m_NumColours; }

public:		// Virtual overrides
	virtual bool Delete(void);
	virtual void AddToDeleteList(PS_RecordList& delete_list) const;
	virtual bool CopyFromRecord(const JC_Record * source, PS_RecordRemap& copy_map);
	void New(U16 numcols);
	U16  Type(void)	const			{ return TypePalette; }
	void WriteBody(void);
	void ReadBody(void);
	JC_Colour Colour(U16 index);
	void Colour(U16 index, const JC_Colour& col);
	virtual CString TypeString(void) const { return CString("Palette"); }

private:
	void Clear(void);
};
//-------------------------------------------------------------------------------------------------------------------------------
// JC_Point - A coloured vertex reference
//-------------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_Point : public JC_ListElement
{
private:
	JC_Colour	 m_Colour;
	JC_Vertex  * m_Vertex;
	static JC_Template * m_Template;		// Temporary for construction from database

public:
	JC_Point(void){ m_Colour.Set(0, 0, 0, 0); m_Vertex = NULL; }
	static void Template(JC_Template * t)	{ m_Template = t; }

	const JC_Colour&	Colour(void) const	{ return m_Colour; }
	const JC_Vertex *	Vertex(void) const	{ return m_Vertex; }

	void	Colour(const JC_Colour& c)		{ m_Colour = c; }
	void	Vertex(JC_Vertex * v)			{ m_Vertex = v; }

	BOOL	EqualsWithinTolerance(const JC_Vector& v) const		{ return m_Vertex->EqualsWithinTolerance(v);	}
	void	CopyData(const JC_Point& p);
	void	DeleteVertex(void);		// Do not call this unless you know what you're doing in fine detail :)

public:	// Virtual overrides
	void	Insert(JC_Buffer& buf) const;
	void	Extract(JC_Buffer& buf);

	friend JC_Buffer& operator << (JC_Buffer& buf, const JC_Point& p)	{ p.Insert(buf); return buf; }
	friend JC_Buffer& operator >> (JC_Buffer& buf, JC_Point& p)			{ p.Extract(buf); return buf; }
};
//-------------------------------------------------------------------------------------------------------------------------------
// JC_PointList - List of JC_Point
//-------------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_PointList : public JC_List
{
public:
	JC_PointList() { }
	JC_PointList(const JC_PointList * pl);
	virtual ~JC_PointList() { }

public:	// Virtual overrides
	JC_ListElement * NewType(U8 /*kind*/)	{ return new JC_Point; }		// There is only one kind of point
};
//-------------------------------------------------------------------------------------------------------------------------------
// JC_RecordRefList - List of JC_RecordRef
//-------------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_RecordRefList : public JC_List
{
private:
	PS_Database * m_Database;			// Ptr to database

public:
	JC_RecordRefList(PS_Database * db)		{ m_Database = db; }
	virtual ~JC_RecordRefList() { }

	void Database(PS_Database * db)			{ m_Database = db; }
	BOOL ContainsRecNum(U32 rec_num) const;

public:	// Virtual overrides
	virtual POSITION AttachTail(JC_ListElement * le);
	virtual JC_ListElement * NewType(U8 /*kind*/)	{ return new JC_RecordRef(m_Database); }	// There is only one kind of record ref
	virtual void CopyAcross(JC_UpgradeContext& upgrade);

public: // Debugging
	virtual void ReportRecordRef(JC_RecordRefList&);
};
//-------------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_FaceArray	: public CTypedPtrArray<CPtrArray, JC_Face*> { };
class JCCLASS_DLL JC_VertexArray: public CTypedPtrArray<CPtrArray, JC_Vertex*> { };
//-------------------------------------------------------------------------------------------------------------------------------
struct JC_BSRec			// Binary search record
{
	U32	 Ident;			// Ident of element
	void * Element;		// Ptr to element
};
//-------------------------------------------------------------------------------------------------------------------------------
// JC_Anchor - Anchors an object to it's parent
//-------------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_Anchor
{
private:
	U32			 m_PinId;			// Id of pin within vertex list
	JC_RecordRef m_Template;		// Reference to template containing pin
	JC_Vector	 m_VectorUp;		// Upward vector
	JC_Vector	 m_VectorFwd;		// Forward vector
	PS_Database* m_Database;		// Ptr to database
	JC_Vector	m_AnchorPos;		// Position offset of anchor within object

public: // Why public?
	U16	m_Version;

public:
	JC_Anchor(PS_Database * db): m_Template(db)	{ Null(); m_Database = db; m_Version = 0x0002; }

	void	PinId(U32 pinid)					{ m_PinId = pinid; }
	U32	PinId(void)				const			{ return m_PinId; }
	void	Null(void);
	void	Template(JC_Template * t);
	JC_Template * TemplatePtr() const;
	const JC_Vector& PinVector(void)	const;
	void	VectorUp(const JC_Vector& v)		{ m_VectorUp = v; m_VectorUp.Normalise();		}
	void	VectorFwd(const JC_Vector& v)		{ m_VectorFwd = v; m_VectorFwd.Normalise();		}
	void	AnchorPos(const JC_Vector& v)		{ m_AnchorPos = v;	 }
	JC_Vector VectorUp(void)	const			{ return m_VectorUp; }
	JC_Vector VectorFwd(void)	const			{ return m_VectorFwd; }
	JC_Vector AnchorPos(void) const				{ return m_AnchorPos; }
	const JC_RecordRef& Template(void)	const	{ return m_Template; }
	void	Database(PS_Database * db)			{ m_Database = db; }

	void	Insert(JC_Buffer& buf) const;
	void	Extract(JC_Buffer& buf);
	void	Copy(const JC_Anchor *src_anchor);

	void ResolveRefs(JC_UpgradeContext& upgrade);

	friend JC_Buffer& operator << (JC_Buffer& buf, const JC_Anchor& a)	{ a.Insert(buf); return buf; }
	friend JC_Buffer& operator >> (JC_Buffer& buf, JC_Anchor& a)		{ a.Extract(buf); return buf; }

public:		// Debugging
	virtual void ReportRecordRef(JC_RecordRefList&);
};
//-------------------------------------------------------------------------------------------------------------------------------
//. rv_Model - Abstract base class for JC_Template and JC_Object
//-------------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL rv_Model : public JC_PropertyRecord
{
protected:
	CString						m_OrigName;		//. Original import name (if the model has been renamed)

	JC_RecordRef				m_Parent;		//. Object/Template parent record
	JC_Anchor					m_Anchor;		//. Anchor for orienation w.r.t parent's pin.
	JC_Vector					m_Scale;		//. X, Y, and Z scales

	JC_RecordRefList			m_ChildList;	//. List of references to child objects
	AD_LightTypeList			m_LightList;	//. List of Lights
	JC_RecordRefList			m_LinkList;		//. List of Links

	//. Light data:  These could use a struct to save space when they are not in use.
	U8							m_ObjectLightingType;
	BOOL						m_LightChosenObjectOnlyFlag;
	BOOL						m_WholeTreeFlag;
	U32							m_ParentAccessNumber;
	U32							m_ChildAccessNumber;

public:
	rv_Model(PS_Database * db, U32 recnum);
	virtual ~rv_Model();

	D3DVECTOR ApplyTransforms(const rv_Model * target_frame, const D3DVECTOR& in_vector) const;
	bool	GetObjectClassType(CString& type) const;
	virtual bool CopyFromRecord(const JC_Record * source, PS_RecordRemap& copy_map);
	virtual bool				CreateCopyRecordList(CDWordArray& list) const;
	virtual bool				Delete(void);
	void						Name(const CString& name);
	CString						Name(void) const				{ return JC_Record::Name();	}
	const CString&				OrigName() const				{ return m_OrigName;		}
	void						OrigName(const CString& name)	{ m_OrigName = name; }

	void						Unpin(bool synchronise = true);
	virtual void				PinTo(rv_Model * parent, U32 pinid, const JC_Vector& up, const JC_Vector& fwd, bool synchronise = true);
	virtual const JC_Anchor&	Anchor(void) const				{ return m_Anchor;			}
	JC_Vector					WorldPinPos(void);
	const JC_RecordRef&			Parent(void)	const			{ return m_Parent;			}
	rv_Model*					ParentPtr(void) const;

	virtual void				Scale(const JC_Vector& v)		{ m_Scale = v;				}
	virtual const JC_Vector&	Scale(void) const				{ return m_Scale;			}

	JC_RecordRefList&			ChildList(void)					{ return m_ChildList;		}
	const JC_RecordRefList&		ChildList(void) const			{ return m_ChildList;		}
	AD_LightTypeList&			LightList(void)					{ return m_LightList;		}
	JC_RecordRefList&			LinkList(void)					{ return m_LinkList;		}
	const JC_RecordRefList&		LinkList(void) const			{ return m_LinkList;		}
	JC_RecordRefList*			GetLinkRecordRefList(void);

	BOOL						HasChildren(void) const			{ return (BOOL)(m_ChildList.GetCount() != 0); }
	BOOL						HasParent(void)	const			{ return !m_Parent.IsInvalid(); }

	BOOL						ContainsModel(U32 recnum) const;
	void						ReferenceChild(rv_Model * object);
	void						DereferenceChild(rv_Model * object);

	void						GetTextureArray(CDWordArray& array) const;
	void						GetVertices(JC_VertexArray& array);
	void						GetFaces(JC_FaceArray& array);

	void						AddLink(U32 LinkRecordNumber);
	void						RemoveLink(U32 LinkRecordNumber);

	void	SetObjectLightType(U8 NewObjectLightType)			{ m_ObjectLightingType = NewObjectLightType;	}
	void	SetLightChosenObjectOnlyFlag(BOOL FlagValue)		{ m_LightChosenObjectOnlyFlag = FlagValue;		}
	void	SetWholeTreeFlag(BOOL NewWholeTreeFlag)				{ m_WholeTreeFlag = NewWholeTreeFlag;			} 
	void	SetParentAccessNumber(U32 NewParentAccessNum)		{ m_ParentAccessNumber = NewParentAccessNum;	}
	void	SetChildAccessNumber(U32 NewChildAccessNum)			{ m_ChildAccessNumber = NewChildAccessNum;		}
	U8		GetObjectLightType(void)							{ return m_ObjectLightingType;					}
	BOOL	GetLightChosenObjectOnlyFlag(void)					{ return m_LightChosenObjectOnlyFlag;			}
	BOOL	GetWholeTreeFlag(void)					 			{ return m_WholeTreeFlag;						}
	U32		GetParentAccessNumber(void)							{ return m_ParentAccessNumber;					}
	U32		GetChildAccessNumber(void)							{ return m_ChildAccessNumber;					}

	void	RemoveLightList(void);

	BOOL	DatabaseIs(PS_Database * db) const					{ return (BOOL)(m_Database == db); }

	void						Copy(rv_Model* src_model);		//. Front end to Template/Object copy
	virtual void				SaveTree(void) const;			//. Recursive save

public:	// Virtual overrides
	virtual void AddToDeleteList(PS_RecordList& delete_list) const;

	virtual U16					Type(void) const				{ return TypeModel;			}
	virtual CString				TypeString(void) const			{ return CString("Model");	}
	virtual BOOL				IsAnimated() const				{ return FALSE;				}
	virtual BOOL				IsModel() const					{ return TRUE;				}
	virtual void				Dependencies(PS_CompressRecordList * list) const;

	virtual void				WriteBody(void);
	virtual void				ReadBody(void);	

	virtual JC_Template*		TemplatePtr()		= 0;
	virtual JC_Template* const  TemplatePtr() const = 0;
	virtual void				GetTextureList(PS_UniqueRecordList * list) const = 0;

	virtual void				ReportRecordRef(JC_RecordRefList&);
	virtual void				ResolveRefs(JC_UpgradeContext& upgrade);
	virtual U32					Synchronise();

	//. For animations:
	virtual U32					CalcFrame(float start, float now, PS_CompressRecordList *dirty_list = NULL);
	virtual const JC_Anchor&	RealAnchor(void) const			{ return m_Anchor;			}
	virtual void				CopyBodyAcross(JC_UpgradeContext& context, JC_Record * dest_record);
};
//-------------------------------------------------------------------------------------------------------------------------------
//. JC_Template - abstract base class for Animated and Static templates
//-------------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_Template : public rv_Model
{
protected:
	JC_RecordRefList  m_InstanceList;					// RecordRef list of instances of this template
public:
	JC_Template(PS_Database * db, U32 recnum);
	virtual ~JC_Template();

	virtual void			AttachFace(JC_Face * f)		= 0;
	virtual void			AttachVertex(JC_Vertex * v)	= 0;

	virtual JC_Pin	 *		FindPin(U32 pinid)		= 0;
	virtual JC_Vertex *		FindVertex(U32 vertid)	= 0;

	virtual JC_Data *		CopyPropertyData(const CString& name)	= 0;
	virtual void			ModifyProperty(const CString& name, JC_Data * data, JC_Property::Switch sw)	= 0;

	virtual JC_FaceList*	FaceList(void) const	= 0;
	virtual JC_VertexList*	VertexList(void) const	= 0;

	virtual void			GetTextureList(PS_UniqueRecordList * list) const			= 0;

	static const JC_Vector	UnitVectorUp;
	static const JC_Vector	UnitVectorForward;
	static const JC_Vector  ORIGIN;

	JC_Template*		TemplatePtr()			{ AddRef(); return (JC_Template*)this;	     }
	JC_Template* const	TemplatePtr() const		{ AddRef(); return (JC_Template*const)this;  }

	void					AddInstance(U32 recnum);
	void					RemoveInstance(U32 recnum);
	const JC_RecordRefList*	InstanceList() const		{ return &m_InstanceList;	}

public:	// Virtual overrides
	virtual bool Delete(void);
	virtual void AddToDeleteList(PS_RecordList& delete_list) const;
	virtual BOOL IsTemplate()	const		{ return TRUE;			}
	virtual void CopyTree(const JC_Template * src_tmp)	= 0;
	virtual void Copy(const JC_Template * src_tmp) = 0;
	virtual U32  Synchronise();
	virtual bool CopyFromRecord(const JC_Record * source, PS_RecordRemap& copy_map);
	virtual BOOL IsInstancedByRecord(U32 record_number) const;
	U32 TotalFaces(void) const;
};
//-------------------------------------------------------------------------------------------------------------------------------
//. rv_StaticTemplate : Non animated template data.
//-------------------------------------------------------------------------------------------------------------------------------
typedef CList<JC_Face*, JC_Face*&> PS_FaceList;
typedef	CMap<U32, U32&, PS_FaceList*, PS_FaceList*&> PS_TempFaceMap;
typedef CMap<U32, U32&, JC_Vertex*, JC_Vertex*&> PS_SearchMap; // used to optimise loading time
class JCCLASS_DLL rv_StaticTemplate : public JC_Template
{
private:
	JC_FaceList		* m_FaceList;						// Ptr to face list
	JC_VertexList	* m_VertexList;						// Ptr to vertex list
	JC_RecordRef	  m_BaseTemplate;					// RecordRef to base template that this is derived from
	PS_SearchMap m_SearchMap; // this replaces the following line
	//	CArray<JC_BSRec, JC_BSRec&> m_BinarySearchArray;	// Binary search array

	PS_TempFaceMap  * m_TempFaceMap; // this map is used when cross-referencing textures and faces
protected:
	void AddFaceToTempMap(const U32 texture, JC_Face * face);
public:
	rv_StaticTemplate(PS_Database * db, U32 recnum);
	virtual ~rv_StaticTemplate();

	virtual bool CopyFromRecord(const JC_Record * source, PS_RecordRemap& copy_map);
	void InvalidateTempFaceMap(void);
	void AttachFace(JC_Face * f);
	void AttachVertex(JC_Vertex * v);

	JC_Pin	 * FindPin(U32 pinid);
	JC_Vertex * FindVertex(U32 vertid);

	JC_Data * CopyPropertyData(const CString& name);
	void	ModifyProperty(const CString& name, JC_Data * data, JC_Property::Switch sw);

	JC_FaceList*			FaceList(void) const		{ return m_FaceList;		}
	JC_VertexList*			VertexList(void) const		{ return m_VertexList;		}

	void GetTextureList(PS_UniqueRecordList * list) const;

	static const JC_Vector	UnitVectorUp;
	static const JC_Vector	UnitVectorForward;

public:	// Virtual overrides
	virtual bool NullFacesUsingTexture(const PS_Texture * texture);
	virtual U16  Type(void)		const		{ return TypeTemplate;	}
	virtual void WriteBody(void);
	virtual void ReadBody(void);
	virtual void Dependencies(PS_CompressRecordList * list) const;
	virtual void Copy(const JC_Template * src_tmp);
	virtual void CopyTree(const JC_Template * src_tmp);
	virtual U32 NumberOfRefs(const JC_PropertyName * name);
	virtual bool RemovePropertyNameReference(const JC_PropertyName * name);
	virtual void AddPropertyNameUsage(JC_RecordRefList& list) const;
	virtual void ResolveRefs(JC_UpgradeContext& upgrade);
	virtual void CopyAcross(JC_UpgradeContext& upgrade);

public:	// Debugging
	virtual void ReportRecordRef(JC_RecordRefList&);
	virtual CString TypeString(void) const { return CString("Template"); }

protected:
	JC_Template * BaseTemplate(void);
	void CreateBinarySearchArray(void);
	void AddToBinarySearchArray(JC_Vertex * v);
	void ScanInstanceList(void);
};
//-------------------------------------------------------------------------------------------------------------------------------
// JC_Object - A 3D world object
//-------------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_Object : public rv_Model
{
private:
	JC_RecordRef m_Template;			// For access to template

public:
	JC_Object(PS_Database * db, U32 recnum);
	virtual ~JC_Object();

	void Template(U32 recnum);
	JC_Data * CopyPropertyData(const CString& name);
	void ModifyProperty(const CString& name, JC_Data * data, JC_Property::Switch sw = JC_Property::Self);
	void GetTextureList(PS_UniqueRecordList * list) const;
	const JC_RecordRef&	Template(void)	const		{ return m_Template; }

	BOOL HasTemplate(void)				const	{ return !m_Template.IsInvalid(); }

	JC_Template * TemplatePtr();
	JC_Template * const TemplatePtr(void) const;

public:	// Virtual overrides
	virtual bool				Delete(void);
	virtual U16  Type(void)				const	{ return TypeObject; }
	virtual BOOL IsObject()	const				{ return TRUE;			}
	virtual bool CopyFromRecord(const JC_Record * source, PS_RecordRemap& copy_map);
	virtual void WriteBody(void);
	virtual void ReadBody(void);
	virtual void Dependencies(PS_CompressRecordList * list) const;
	virtual void Copy(JC_Object* src_obj);
	virtual void CopyTree(JC_Object * src_obj);
	virtual void ResolveRefs(JC_UpgradeContext& upgrade);
	virtual U32  Synchronise();
	virtual JC_PropertyList * CreateBackgroundPropertyList(void) const;

public:	// Debugging
	virtual void ReportRecordRef(JC_RecordRefList&);
	virtual CString TypeString(void) const { return CString("Object"); }
};
//-------------------------------------------------------------------------------------------------------------------------------
// JC_Face - A list of vertices describing one face of an object
//-------------------------------------------------------------------------------------------------------------------------------
struct BA_VertexOrient
{
	U16 FirstFaceVertex;
	U16 FirstTextureVertex;
	BOOL Reversed;
};
//-------------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_Face : public JC_ListElement
{
	friend virtual bool rv_StaticTemplate::NullFacesUsingTexture(const PS_Texture * texture);

private:
	BOOL				m_CopyFlag;				// TRUE if face is a copy of another face
	double				m_Reflectivity;
	double				m_Translucency;
	PS_Database		*	m_Database;
	JC_PointList		m_PointList;
	JC_PropertyList *	m_PropertyList;
	JC_FaceList		*	m_ParentList;			// This is set when the face is inserted in a face list
	JC_RecordRef        m_FrontTexture;			// Texture for the front (clockwise?) of a face
	JC_RecordRef        m_BackTexture;			// Texture for the back of a face (Null record = not set)
	BA_VertexOrient		m_FrontOrient;
	BA_VertexOrient		m_BackOrient;
	U16					m_Version;				// So that JC_Face can be updated in a database

public:
	JC_Face(PS_Database * db);
	JC_Face(JC_Face * face);
	virtual ~JC_Face();
	double GetReflectivity(void) const					{ return m_Reflectivity; }
	void   SetReflectivity(double reflectivity)			{ m_Reflectivity = reflectivity; }
	double GetTranslucency(void) const					{ return m_Translucency; }
	void   SetTranslucency(double translucency)         { m_Translucency = translucency; }

	void ReversePointOrder(void);
	void ParentList(JC_FaceList * list)					{ m_ParentList = list; }
	JC_FaceList * ParentList(void)				const	{ return m_ParentList; }

	POSITION AttachPoint(JC_Point * p)					{ return m_PointList.AttachTail(p); }

	const JC_PointList * PointList(void)		const	{ return &m_PointList; }
	JC_PropertyList * PropertyList(void)		const	{ return m_PropertyList; }
	const PS_Texture *     FrontTexture(void)   const;
	const PS_Texture *     BackTexture (void)   const;
	BA_VertexOrient&	   FrontOrient(void)			{ return m_FrontOrient; }
	BA_VertexOrient&	   BackOrient(void)				{ return m_BackOrient;  }
	void  FrontTexture(PS_Texture * t);
	void  BackTexture (PS_Texture * t);
	void  CopyData(const JC_Face& f, double tolerance = 0.0);
	PS_Database * Database(void)				const	{ return m_Database; }
	U16& Version(void)								{ return m_Version; }
	U32 NumberOfRefs(const JC_PropertyName * name);
	bool RemovePropertyNameReference(const JC_PropertyName * name);
	virtual void AddPropertyNameUsage(JC_RecordRefList& list) const;
	virtual void ResolveRefs(JC_UpgradeContext&, int item=-1, JC_List * parentlist=NULL);

public:	// Virtual overrides
	void	Insert(JC_Buffer& buf) const;
	void	Extract(JC_Buffer& buf);
	virtual void Dependencies(PS_CompressRecordList * list) const;
	virtual void CopyAcross(JC_UpgradeContext& upgrade);

public:	// Debugging
	virtual void ReportRecordRef(JC_RecordRefList&);

private:
	void DeletePointVertices(void);
};
//-------------------------------------------------------------------------------------------------------------------------------
// JC_FaceList - List of JC_Face
//-------------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_FaceList : public JC_List
{
private:
	JC_Template * m_Template;			// Ptr to parent template
	U16 m_Version;						// Version of list

public:
	JC_FaceList(JC_Template * t)			{ m_Template = t; }
	virtual ~JC_FaceList() { }

	JC_Template * Template(void)			{ return m_Template; }
	void InvalidateTempFaceMap(void)		{ ((rv_StaticTemplate*)m_Template)->InvalidateTempFaceMap(); }
	void ReverseFacePointOrder(void);
	void Version(U16 version);
	int GetVertexReferenceCount();
	int GetBackVertexReferenceCount();
	U32 NumberOfRefs(const JC_PropertyName * name);
	bool RemovePropertyNameReference(const JC_PropertyName * name);
	virtual void AddPropertyNameUsage(JC_RecordRefList& list) const;
	virtual void CopyAcross(JC_UpgradeContext& upgrade);

public:	// Virtual overrides
	POSITION AttachTail(JC_ListElement * le);
	JC_ListElement * NewType(U8 /*kind*/)	{ JC_Face * face = new JC_Face(m_Template->Database());
											  face->Version() = m_Version;
											  return face; }		// There is only one kind of face

public:	// Debugging
	virtual void ReportRecordRef(JC_RecordRefList&);
};
//-------------------------------------------------------------------------------------------------------------------------
// JC_Bool - Copies and stores boolean values
//-------------------------------------------------------------------------------------------------------------------------
class JC_Bool : public JC_DataConcrete
{
private:
	BOOL m_Data;				// Data stored here

public:
	JC_Bool(BOOL val=0)			{ m_Data = val; }

	JC_Bool(const CString& str)
	{
		m_Data = FALSE;
		if (str.IsEmpty() == FALSE)
		{
			char s = (char)tolower(str[0]);
			if (s == 't') m_Data = TRUE;
			else
			if (s != 'f') THROW(new JC_Exception("Boolean value must be True or False"));
		}
	}

public:  // Virtual overrides
	virtual void Clear(void) { m_Data = 0; }
	virtual Tok  Token(void)		const		{ return TokBool; }
	virtual void Insert(JC_Buffer& buf) const	{ buf.Insert(m_Data); }
	virtual void Extract(JC_Buffer& buf)		{ buf.Extract(m_Data); }
	virtual operator U8		(void)	const { return (U8)		m_Data; }
	virtual operator S8		(void)	const { return (S8)		m_Data; }
	virtual operator U16	(void)	const { return (U16)	m_Data; }
	virtual operator S16	(void)	const { return (S16)	m_Data; }
	virtual operator U32	(void)	const { return (U32)	m_Data; }
	virtual operator S32	(void)	const { return (S32)	m_Data; }
	virtual operator BOOL	(void)	const { return (BOOL)   m_Data; }
	virtual operator float	(void)	const { return (float)	m_Data; }
	virtual operator double	(void)	const { return (double)	m_Data; }
	virtual CString RangeString(void)const { return "True/False"; }
	virtual CString TypeString(void) const { return "Truth value (" + RangeString() + ") [Bool]"; }
	virtual CString ShortTypeString(void) const { return "Bool"; }

	virtual CString String(void)	const
	{
		CString s;

		if (m_Data) s = "True";
		else		s = "False";

		return s;
	}
};
//-------------------------------------------------------------------------------------------------------------------------
// JC_Int - Copies and stores integer values
//-------------------------------------------------------------------------------------------------------------------------
template <class TYPE, JC_Data::Tok TOK> class JC_Int : public JC_DataConcrete
{
private:
	TYPE m_Data;				// Data stored here

public:
	JC_Int(TYPE val = 0)		{ m_Data = (TYPE)Validate((double)val, TOK); }
	JC_Int(const CString& str)	{ m_Data = (TYPE)Validate(Convert(str), TOK); }

public:	 // Virtual overrides
	virtual void Insert(JC_Buffer& buf)	const	{ buf.Insert(m_Data); }
	virtual void Extract(JC_Buffer& buf)		{ buf.Extract(m_Data); }

	virtual void Clear(void) { m_Data = 0; }
	virtual Tok	 Token(void)		const	{ return TOK; }
	virtual BOOL IsNumeric(void)	const	{ return TRUE; }
	virtual operator U8		(void)	const	{ return (U8)	  m_Data; }
	virtual operator S8		(void)	const	{ return (S8)	  m_Data; }
	virtual operator U16	(void)	const	{ return (U16)	  m_Data; }
	virtual operator S16	(void)	const	{ return (S16)	  m_Data; }
	virtual operator U32	(void)	const	{ return (U32)	  m_Data; }
	virtual operator S32	(void)	const	{ return (S32)	  m_Data; }
	virtual operator BOOL	(void)	const	{ return (BOOL)   m_Data; }
	virtual operator float	(void)	const	{ return (float)  m_Data; }
	virtual operator double	(void)	const	{ return (double) m_Data; }
	virtual CString TypeString(void)const	{ return "Number (" + RangeString() + ") [" + ShortTypeString() + "]"; }

	virtual CString RangeString(void) const
	{
		CString s;
		const char * format = "%d to %d";
		if (TOK == TokS8)	s.Format(format, S8_MIN,  S8_MAX);
		else
		if (TOK == TokS16)	s.Format(format, S16_MIN, S16_MAX);
		else
		if (TOK == TokS32)	s = "-2 billion to +2 billion";
		else
		if (TOK == TokU8)	s.Format(format, U8_MIN,  U8_MAX);
		else
		if (TOK == TokU16)	s.Format(format, U16_MIN, U16_MAX);
		else
		if (TOK == TokU32)	s = "0 to 4 billion";
		return s;
	}

	virtual CString ShortTypeString(void) const
	{
		if (TOK == TokS8)	return "Int8";
		else
		if (TOK == TokS16)	return "Int16";
		else
		if (TOK == TokS32)	return "Int32";
		else
		if (TOK == TokU8)	return "UInt8";
		else
		if (TOK == TokU16)	return "UInt16";
		else
		if (TOK == TokU32)	return "UInt32";
	}

	virtual CString String(void) const
	{
		CString s;

		if (m_Format == 'd')
		{
			if ((TOK == TokS8) || (TOK == TokS16) || (TOK == TokS32))
			{
				s.Format("%d", m_Data);
			}
			else s.Format("%u", m_Data);
		}
		else
		if (m_Format == 'h')
		{
			s.Format("0x%08X", m_Data);
		}
		else
		if (m_Format == 'b')
		{
			s = "%";
			U32 mask = 0x80000000;
			while (mask != 0)
			{
				if (m_Data & mask) s += "1"; else s += "0";
				mask >>= 1;
			}
		}

		return s;
	}
};
//-------------------------------------------------------------------------------------------------------------------------
typedef JC_Int<U8,	JC_Data::TokU8>		JC_U8;
typedef JC_Int<S8,	JC_Data::TokS8>		JC_S8;
typedef JC_Int<U16, JC_Data::TokU16>	JC_U16;
typedef JC_Int<S16, JC_Data::TokS16>	JC_S16;
typedef JC_Int<U32,	JC_Data::TokU32>	JC_U32;
typedef JC_Int<S32,	JC_Data::TokS32>	JC_S32;
//-------------------------------------------------------------------------------------------------------------------------
// JC_Flt - Copies and stores floating-point values
//-------------------------------------------------------------------------------------------------------------------------
template <class TYPE, JC_Data::Tok TOK> class JC_Flt : public JC_DataConcrete
{
private:
	TYPE m_Data;				// Data stored here

private:
	TYPE RoundUp(TYPE x)	const	{ return x < (TYPE)0.0 ? x - (TYPE)0.5 : x + (TYPE)0.5; }

public:
	JC_Flt(TYPE val = (TYPE)0.0)	{ m_Data = (TYPE)val; }

	JC_Flt(const CString& str)
	{
		m_Data = (TYPE)atof((LPCSTR)str);
	}

public: // Virtual overrides
	virtual void Clear(void) { m_Data = 0.0; }
	virtual void Insert(JC_Buffer& buf)	const	{ buf.Insert(m_Data); }
	virtual void Extract(JC_Buffer& buf)		{ buf.Extract(m_Data); }
	virtual Tok	Token(void)			const { return TOK; }
	virtual BOOL IsNumeric(void)	const { return TRUE; }
	virtual operator U8		(void)	const { return (U8)		RoundUp(m_Data); }
	virtual operator S8		(void)	const { return (S8)		RoundUp(m_Data); }
	virtual operator U16	(void)	const { return (U16)	RoundUp(m_Data); }
	virtual operator S16	(void)	const { return (S16)	RoundUp(m_Data); }
	virtual operator U32	(void)	const { return (U32)	RoundUp(m_Data); }
	virtual operator S32	(void)	const { return (S32)	RoundUp(m_Data); }
	virtual operator BOOL	(void)	const { return (BOOL)	RoundUp(m_Data); }
	virtual operator float	(void)	const { return (float)  m_Data;	}
	virtual operator double (void)	const { return (double) m_Data;	}
	virtual CString  String (void)  const { CString s; s.Format("%g", (double)m_Data); return s; }

	virtual CString RangeString(void) const
	{
		if (TOK == TokFloat)	return "Single Precision";
		else
		if (TOK == TokDouble)	return "Double Precision";
	}

	virtual CString TypeString(void) const
	{
		if (TOK == TokFloat)	return "Number with single precision [Float]";
		else
		if (TOK == TokDouble)	return "Number with double precision [Double]";
	}

	virtual CString ShortTypeString(void) const
	{
		if (TOK == TokFloat)	return "Float";
		else
		if (TOK == TokDouble)	return "Double";
	}
};
//-------------------------------------------------------------------------------------------------------------------------
typedef JC_Flt<float,	JC_Data::TokFloat>	JC_Float;
typedef JC_Flt<double,	JC_Data::TokDouble>	JC_Double;
//-------------------------------------------------------------------------------------------------------------------------
// JC_Binary - Copies and stores binary data of any length
//-------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_Binary : public JC_DataConcrete
{
private:
	U8 * m_Data;			// Ptr to buffer containing data
	U32  m_NumBytes;		// Number of bytes in buffer

public:
	JC_Binary();
	JC_Binary(U8 * buf, U32 numbytes);
	virtual ~JC_Binary();

	void Set(U8 * buf, U32 numbytes, BOOL clear = FALSE);
	const U8 * Get(void)	const	{ return m_Data; }
	U32  Length(void)		const	{ return m_NumBytes; }
	void Unset(void);

public:		// Virtual overrides
	void Insert(JC_Buffer& buf) const;
	void Extract(JC_Buffer& buf);

	Tok	Token(void)			const { return TokBinary; }
	operator U8		(void)	const { return (U8)		m_NumBytes;	}
	operator S8		(void)	const { return (S8)		m_NumBytes;	}
	operator U16	(void)	const { return (U16)	m_NumBytes;	}
	operator S16	(void)	const { return (S16)	m_NumBytes;	}
	operator U32	(void)	const { return (U32)	m_NumBytes;	}
	operator S32	(void)	const { return (S32)	m_NumBytes;	}
	operator BOOL	(void)	const { return (BOOL)	m_NumBytes; }
	operator float	(void)	const { return (float)	m_NumBytes;	}
	operator double	(void)	const { return (double)	m_NumBytes;	}
	operator void *	(void)	const { return (void*)	Get();		}

	virtual void Clear(void) { Unset(); }
	virtual CString TypeString(void) const { return "Binary Data [Bin]"; }
	virtual CString ShortTypeString(void) const { return "Bin"; }

private:
	void Initialise(void)	{ m_Data = NULL; m_NumBytes = 0; }
};
//-------------------------------------------------------------------------------------------------------------------------
// JC_String - Handles character strings
//-------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_String : public CString, public JC_DataConcrete
{
public:
	JC_String();
	JC_String(UINT id);
	JC_String(const char * str);
	JC_String(const CString& str);

	void Standardise(void);
	void TrimWhiteSpace(void);

	void Insert(JC_Buffer& buf)	const	{ buf.Insert(*this); }
	void Extract(JC_Buffer& buf)		{ buf.Extract(*this); }

public:		// Virtual overrides
	Tok	Token(void)		const	{ return TokString; }
	operator U8		(void)	const { return (U8)		GetLength();	}
	operator S8		(void)	const { return (S8)		GetLength();	}
	operator U16	(void)	const { return (U16)	GetLength();	}
	operator S16	(void)	const { return (S16)	GetLength();	}
	operator U32	(void)	const { return (U32)	GetLength();	}
	operator S32	(void)	const { return (S32)	GetLength();	}
	operator BOOL	(void)	const { return (BOOL)	GetLength();	}
	operator float	(void)	const { return (float)	GetLength();	}
	operator double	(void)	const { return (double)	GetLength();	}
	virtual CString TypeString(void) const { return "Plain Text (a-z, 0-9, etc) [String]"; }
	virtual CString ShortTypeString(void) const { return "String"; }
	virtual void Clear(void) { Empty(); }
	CString String(void) const { return (CString&)*this; }
};
//-------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_StringArray : public CArray<JC_String, const JC_String&>
{
public:
	BOOL Contains(const JC_String& s) const;
};
//-------------------------------------------------------------------------------------------------------------------------
// JC_NamedData - Historically a JC_Data with a name, but now a JC_Data with a unique data id string.
//-------------------------------------------------------------------------------------------------------------------------
class JC_DialogData;
class JC_NamedDataCollection;
//-------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_NamedData
{
private:
	JC_Data * m_Data;						// Ptr to JC_Data that holds the actual data.
	CString   m_DataId;						// Unique Id of the data.
	JC_NamedDataCollection * m_Collection;	// Ptr to collection that contains me.

public:
	JC_NamedData(const CString& dataid="", JC_Data * data = NULL);
	virtual ~JC_NamedData();

	S32 Value(void) const;
	void Insert(JC_Buffer& buf) const;
	void Extract(JC_Buffer& buf);
	JC_NamedData * Duplicate(void) const;
	JC_Data * GetData(void)		const	{ return m_Data; }
	void SetData(JC_Data * data);
	void SetCollection(JC_NamedDataCollection * c)	{ m_Collection = c; }
	BOOL IsDataId(const CString& id)	const	{ return (BOOL)(m_DataId == id); }
	const CString& GetDataId(void)		const	{ return m_DataId; }
	void ResolveRefs(JC_UpgradeContext& upgrade);

private:
	void DeleteData(void);
};
//-------------------------------------------------------------------------------------------------------------------------
// JC_NamedDataCollection - Collection of pointers to JC_NamedData
//-------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_NamedDataCollection : public JC_Data
{
private:
	PS_Database * m_Database;

public:
	JC_NamedDataCollection(PS_Database * db) { m_Database = db; }
	virtual ~JC_NamedDataCollection() { }

	void AffixNewData(JC_NamedData * nd);
	JC_Data * GetData(const CString& dataid);
	BOOL DeleteNamedData(const CString& dataid);
	JC_NamedData * GetNamedData(const CString& dataid);
	void SetData(const CString& dataid, JC_Data * data);

public: // Virtual functions
	virtual int  AddNamedData(JC_NamedData * nd) = 0;
	virtual void DetachNamedData(JC_NamedData * nd) = 0;
	virtual JC_NamedData * GetNextNamedData(void) = 0;
	virtual JC_NamedData * GetFirstNamedData(void) = 0;
	virtual PS_Database * GetDatabase(void) const { return m_Database; }
	virtual BOOL IsEmpty(void) const = 0;
	virtual void Clear(void) = 0;
	virtual void ResolveRefs(JC_UpgradeContext& upgrade) = 0;
};
//-------------------------------------------------------------------------------------------------------------------------
// JC_NamedDataList - List of pointers to JC_NamedData
//-------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_NamedDataList : public JC_NamedDataCollection, public CTypedPtrList<CPtrList, JC_NamedData*>
{
private:
	POSITION m_Position;

public:
	JC_NamedDataList(PS_Database * db) : JC_NamedDataCollection(db) { }
	virtual ~JC_NamedDataList()	{ Clear(); }

public: // Virtual overrides
	virtual void Insert(JC_Buffer& buf) const;
	virtual void Extract(JC_Buffer& buf);
	virtual JC_NamedData * GetNextNamedData(void);
	virtual JC_NamedData * GetFirstNamedData(void);
	virtual int AddNamedData(JC_NamedData * nd)	{ nd->SetCollection(this); AddTail(nd); return -1; }
	virtual void DetachNamedData(JC_NamedData * nd);
	virtual Tok	Token(void)				const		{ return TokNamedDataList; }
	virtual BOOL IsEmpty(void)			const		{ return GetCount() == 0; }
	virtual void ResolveRefs(JC_UpgradeContext& upgrade);

private:
	virtual void Clear(void);
};
//-------------------------------------------------------------------------------------------------------------------------
// JC_NamedDataArray - Array of pointers to JC_NamedData
//-------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_NamedDataArray : public JC_NamedDataCollection, public CTypedPtrArray<CPtrArray, JC_NamedData*>
{
private:
	int m_Index;			// Index for GetFirst/GetNext.
	int m_Sequence;			// Unique id sequence for new array entries (ie: new rows).

public:
	JC_NamedDataArray(PS_Database * db) : JC_NamedDataCollection(db) { SetSize(0, 16); m_Sequence = 1; }
	virtual ~JC_NamedDataArray()									 { Clear(); }

	int GetSequence(void)				const	{ return m_Sequence; }
	void SetSequence(int seq)					{ m_Sequence = seq; }
	JC_Data * GetData(int index)		const;
	JC_Data * operator [] (int index)	const	{ return GetData(index); }

public: // Virtual overrides
	virtual void Clear(void);
	virtual void Insert(JC_Buffer& buf) const;
	virtual void Extract(JC_Buffer& buf);
	virtual JC_NamedData * GetNextNamedData(void);
	virtual JC_NamedData * GetFirstNamedData(void);
	virtual int AddNamedData(JC_NamedData * nd)	{ nd->SetCollection(this); return Add(nd); }
	virtual void DetachNamedData(JC_NamedData * nd);
	virtual Tok	Token(void)				const		{ return TokNamedDataArray; }
	virtual BOOL IsEmpty(void)			const		{ return GetSize() == 0; }
	virtual void ResolveRefs(JC_UpgradeContext& upgrade);
};
//-------------------------------------------------------------------------------------------------------------------------
// JC_DialogData
//-------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_DialogData : public JC_DataConcrete
{
private:
	JC_RecordRef m_DialogTemplateRecord;		// Reference to associated dialog template record.
	JC_NamedDataCollection * m_Collection;		// Collection of data.

public:
	JC_DialogData(const JC_RecordRef& rr, JC_NamedDataCollection * ndc) : m_DialogTemplateRecord(rr) { m_Collection = ndc; }
	virtual ~JC_DialogData() { if (m_Collection != NULL) delete m_Collection; }

	void AffixNewData(JC_NamedData * nd);
	JC_Record * GetDialogTemplateRecord(void);
	JC_Data * GetData(const CString& dataid) const;
	BOOL DeleteNamedData(const CString& dataid) const;
	void SetData(const CString& dataid, JC_Data * data);
	BOOL IsEmpty(void) const { return m_Collection->IsEmpty(); }
	JC_NamedData * GetNamedData(const CString& dataid) const;
	U32 GetRecordNum(void) const { return m_DialogTemplateRecord.RecordNum(); }

public: // Virtual overrides
	virtual Tok	Token(void) const { return TokDialogData; }
	virtual void Insert(JC_Buffer& buf) const;
	virtual void Extract(JC_Buffer& buf);
	virtual PS_Database * GetDatabase(void)	const	{ return m_Collection->GetDatabase(); }
	virtual void ResolveRefs(JC_UpgradeContext& upgrade);

	virtual CString String(void)	const;
	virtual operator U8		(void)	const { return 0; }
	virtual operator S8		(void)	const { return 0; }
	virtual operator U16	(void)	const { return 0; }
	virtual operator S16	(void)	const { return 0; }
	virtual operator U32	(void)	const { return 0; }
	virtual operator S32	(void)	const { return 0; }
	virtual operator BOOL	(void)	const { return FALSE; }
	virtual operator float	(void)	const { return 0.0; }
	virtual operator double	(void)	const { return 0.0; }
	virtual CString TypeString(void)const { return "Dialog Box Data [Struct]"; }
	virtual CString ShortTypeString(void) const { return "Struct"; }
	virtual void Clear(void) { m_Collection->Clear(); m_DialogTemplateRecord.Clear(); }
};
//-------------------------------------------------------------------------------------------------------------------------
// JC_DialogInfo - A class used to pass a 'Front' and 'Back' dialog data to functions instead of one dialog data.
//-------------------------------------------------------------------------------------------------------------------------
class JC_DialogInfo
{
private:
	JC_DialogData * m_BackData;		// Ptr to 'Back' dialog data	} These data are NOT owned by this class.
	JC_DialogData * m_FrontData;	// Ptr to 'Front' dialog data	}

public:
	JC_DialogInfo(JC_DialogData * front=NULL, JC_DialogData * back=NULL) { m_FrontData = front; m_BackData = back; }
	~JC_DialogInfo(){ DeleteAllData(); }

	JC_DialogData * GetFrontData(void)	const	{ return m_FrontData; }
	JC_DialogData * GetBackData(void)	const	{ return m_BackData; }
	BOOL HasBackData(void)				const	{ return (BOOL)(m_BackData != NULL); }
	BOOL HasFrontData(void)				const	{ return (BOOL)(m_FrontData != NULL); }
	void DeleteBackData(void)					{ if (m_BackData != NULL) { delete m_BackData; m_BackData = NULL; } }
	void DeleteFrontData(void)					{ if (m_FrontData != NULL) { delete m_FrontData; m_FrontData = NULL; } }
	void DeleteAllData(void)					{ DeleteBackData(); DeleteFrontData(); }
	void SetBackData(JC_DialogData * data)		{ DeleteBackData(); m_BackData = data; }
	void SetFrontData(JC_DialogData * data)		{ DeleteFrontData(); m_FrontData = data; }
};
//-------------------------------------------------------------------------------------------------------------------------
// JC_Enum - Enumerated Value
//-------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_Enum : public JC_ListElement
{
private:
	CString	m_Name;			// Name
	U32		m_Value;		// Value
	BOOL	m_ValueIsSet;	// TRUE if value is set
	COLORREF m_Colour;		// colour used by property palette

public:
	JC_Enum();
	JC_Enum(const CString& name);
	JC_Enum(const CString& name, S32 value);
	virtual ~JC_Enum() { }

	void	Set(S32 value);
	void	Set(const CString& name);
	void	Set(const CString& name, S32 value);
	BOOL	ValueIsSet(void)	{ return m_ValueIsSet; }
	CString Equation(void) const;
	void	Insert(JC_Buffer& buf) const;
	void	Extract(JC_Buffer& buf);
	CString	GetName(void)  const { return m_Name; }
	U32		GetValue(void) const { return m_Value; }
	COLORREF GetColour(void) const { return m_Colour; }
	void    SetColour(const COLORREF colour) { m_Colour = colour; }
};
//-------------------------------------------------------------------------------------------------------------------------
// JC_EnumList - List of Enumerated Values
//-------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_EnumList : public JC_List
{
private:
	S32	m_Default;			// Default value for Enums that have not had value set yet

	// I moved the following into JC_Enum so that you would get one colour per enum
	//	COLORREF *ColourArray;  // Used for the 3d property editor - 'AD_3DPropertyView'

public:
	JC_EnumList();
	virtual ~JC_EnumList();

	U32 Count(void)	const				{ return GetCount(); }
	S32 Value(const char * name) const;
	JC_Enum * Enum(const char * name) const;

	void Insert(JC_Buffer& buf) const;
	void Extract(JC_Buffer& buf);
	void ExtractOld(JC_Buffer& buf);

public: // Virtual overrides
	void DeleteEnum(const char * name);		// Deletes all Enums with the specified name from this list
	void AttachTail(JC_Enum * e);
	JC_ListElement * NewType(U8)		{ return new JC_Enum(CString(), 0); }		// There is only one kind of JC_Enum

	COLORREF GetEnumColour(U32 index);
	void SetEnumColour(U32 index, COLORREF colour);
};
//-------------------------------------------------------------------------------------------------------------------------
// JC_EnumRecord - An EnumList which is stored in its own database record
//-------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_EnumRecord : public JC_Record, public JC_EnumList
{
public:
	JC_EnumRecord(PS_Database * db, U32 recnum);  // { }
	virtual ~JC_EnumRecord() { }

public: // Virtual overrides
	virtual bool Delete(void);
	virtual void WriteBody(void);	// { JC_Record::WriteBody(); JC_EnumList::Insert(*m_Buffer); }
	virtual void ReadBody(void);	// { JC_Record::ReadBody();  JC_EnumList::Extract(*m_Buffer); }
	virtual U16 Type(void) const	{ return TypeEnumRecord; }
	virtual bool CopyFromRecord(const JC_Record * source, PS_RecordRemap& copy_map);
	virtual void Dependencies(PS_CompressRecordList * list) const { JC_Record::Dependencies(list); }

public: // Debugging
	virtual CString TypeString(void) const { return CString("Enum Record"); }
	virtual CString ShortTypeString(void) const { return "EnumRec"; }
};
//-------------------------------------------------------------------------------------------------------------------------
// JC_EnumRef - Reference to Enumerated Value
//-------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_EnumRef : public JC_RecordRef, public JC_DataConcrete
{
private:
	CString m_Name;

public:
	JC_EnumRef(PS_Database * db, const CString& text="") : JC_RecordRef(db) { m_Name = text; }
	JC_EnumRef(const CString& name, const JC_RecordRef& rr) : JC_RecordRef(rr) { m_Name = name; }
	virtual ~JC_EnumRef() { }

	JC_Enum * CopyEnum(void) const;
	U32 GetEnumValue(void)  const;
	void SetName(const CString& name) { m_Name = name; }
	CString GetName(void) const { return m_Name; }

public: // Virtual overrides
	virtual void Insert(JC_Buffer& buf)	const;
	virtual void Extract(JC_Buffer& buf);
	Tok	Token(void) const { return TokEnumRef; }
	virtual CString  String (void)  const { return m_Name; }
	virtual operator U8		(void)	const { return (U8)		GetEnumValue();	}
	virtual operator S8		(void)	const { return (S8)		GetEnumValue();	}
	virtual operator U16	(void)	const { return (U16)	GetEnumValue();	}
	virtual operator S16	(void)	const { return (S16)	GetEnumValue();	}
	virtual operator U32	(void)	const { return (U32)	GetEnumValue();	}
	virtual operator S32	(void)	const { return (S32)	GetEnumValue();	}
	virtual operator BOOL	(void)	const { return (BOOL)	GetEnumValue();	}
	virtual operator float	(void)	const { return (float)	GetEnumValue();	}
	virtual operator double	(void)	const { return (double)	GetEnumValue();	}
	virtual PS_Database * GetDatabase(void) const { return JC_RecordRef::Database(); }
	virtual CString TypeString(void) const { return "Enumerated Value [EnumRef]"; }
	virtual CString ShortTypeString(void) const { return "EnumRef"; }
	virtual void Clear(void) { m_Name.Empty(); JC_RecordRef::Clear(); }
	virtual void ResolveRefs(JC_UpgradeContext& context) { JC_RecordRef::ResolveRefs(context); }

};
//-------------------------------------------------------------------------------------------------------------------------
// JC_Bitmap - A bitmap image and default palette
//-------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_Bitmap : public JC_PropertyRecord
{
private:
	JC_Binary * m_Binary;		// Ptr to JC_Binary for BITMAPINFOHEADER, palette and image
	JC_RecordRef m_Palette;		// Record ref of default JC_Palette to use with bitmap if 8 bit
	JC_RecordRefList m_TextureList; // List of textures that use this bitmap

public:
	enum OutputDepth { EXACT_DEPTH = 0, FORCE_16_BIT, FORCE_32_BIT, FORCE_4_BIT_TO_16_BIT };
	enum OutputScale { EXACT_SIZE = 0, ROUND_NEAREST, ROUND_UP, ROUND_DOWN };

public:
	JC_Bitmap(PS_Database * db, U32 recnum);
	virtual ~JC_Bitmap();

	virtual bool CopyFromRecord(const JC_Record * source, PS_RecordRemap& copy_map);
	void	AddTextureRef(const PS_Texture * texture);
	void	ReleaseTextureRef(const PS_Texture * texture);
	void	RemoveInvalidRef(const U32 record_number);
	JC_RecordRefList& TextureList(void) { return m_TextureList; }
	const JC_Binary * GetBinaryData(void) const { return m_Binary; }
	BOOL	IsFlipped(void) const { return (((BITMAPINFOHEADER*)m_Binary->Get())->biHeight > 0); }
	CSize	Size(void) const;
	U32		Depth(void) const;
	void	GetPalette(CPalette& pal) const;
	void	SetPalette(CPalette& pal);
	U32		GetPaletteID(void) const;
	void	SetPaletteID(U32 record);
	void	CreateRecord(const BITMAPINFO * bi);
	void	UpdateRecord(const BITMAPINFO * bi);
	void	Render(CDC * dc, U32 paletterecord = 0);
	void	Render(CDC * dc, const CRect& dest, U32 paletterecord = 0);
	BITMAPINFO * CreateBitmapInfo(OutputScale scale = EXACT_SIZE, OutputDepth depth = EXACT_DEPTH) const;
	// The caller must delete[] the returned pointer
	U32		PaletteNumColours(const BITMAPINFOHEADER * bih) const;
	U32		PaletteNumBytes(const BITMAPINFOHEADER * bih)	const;
	U8*		ImagePtr(const BITMAPINFOHEADER* bih) const;
	void	SetPaletteInBitmap(CPalette& pal);

public:		// Virtual overrides
	virtual bool Delete(void);
	virtual void AddToDeleteList(PS_RecordList& delete_list) const;
	U16  Type(void)	const				{ return TypeBitmap; }
	void WriteBody(void);
	void ReadBody(void);
	virtual void Dependencies(PS_CompressRecordList * list) const;
	virtual void CopyAcross(JC_UpgradeContext& upgrade);
	void GetTextureArray(CDWordArray & array) const;
	virtual void ResolveRefs(JC_UpgradeContext& upgrade);
	virtual U32 Synchronise(void);

public:		// Debugging
	virtual CString TypeString(void) const { return CString("Bitmap"); }

private:
	void Clear(void);
	void ScaleToPower(S32& size, OutputScale scale) const;
protected:
	void ScanTexturesFromDatabase(void);
	void GetPaletteFromBitmap(CPalette& pal) const;
	void CreatePaletteRecord(const BITMAPINFOHEADER * bih);
};
//-------------------------------------------------------------------------------------------------------------------------
// JC_RegularExpression
//-------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_RegularExpression : public CString
{
public:
	JC_RegularExpression(const CString& s);
	virtual ~JC_RegularExpression() { }

	BOOL CheckMatch(const CString& s, BOOL yes = TRUE) const;
};
//-------------------------------------------------------------------------------------------------------------------------
// JC_ListCtrl
//-------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_ListCtrl : public CListCtrl
{
private:
	int m_Idx;
	LV_ITEM m_Lvi;
	char m_Buf[256];

public:
	LV_ITEM * GetNextSelectedItem(void);
	LV_ITEM * GetFirstSelectedItem(void);
	void SelectItem(int item, BOOL select=TRUE);
	void FilterSelection(JC_RegularExpression& re);
	void InvertSelection(void);
};
//-------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_EnumComboBox : public CComboBox
{
// **** You MUST NOT add any data members because it may be cast from CWnd!

public:
	JC_String GetSelText(void);
	void Initialise(const JC_EnumList& el);
	void Reinitialise(const JC_EnumList& el);
};
//-------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_BoolComboBox : public CComboBox
{
// **** You MUST NOT add any data members because it may be cast from CWnd!
public:
	void Initialise(void);
};
//-------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_RecordComboBox : public CComboBox
{
// **** You MUST NOT add any data members because it may be cast from CWnd!
public:
	void Initialise(PS_Database * db, const CDWordArray& rectypes);
};
//-------------------------------------------------------------------------------------------------------------------------
#endif
//-------------------------------------------------------------------------------------------------------------------------
