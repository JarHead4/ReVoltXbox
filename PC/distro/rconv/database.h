#ifndef DATABASE_H
#define DATABASE_H

#include <afxdao.h>
#include <afxtempl.h>
#include <jctypes.h>
#include "PS_ProcessingDialog.h"

#ifndef build_jcclass_dll
	#ifdef build_jcclass_lib
		#define JCCLASS_DLL 
	#else
		#define JCCLASS_DLL __declspec(dllimport)
	#endif
#else
	#define JCCLASS_DLL __declspec(dllexport)
#endif

void JCCLASS_DLL HabitatCleanupDatabase(void);
//------------------------------------------------------------------------------------------------------------
//. Rv's database function macros
//------------------------------------------------------------------------------------------------------------
#define GET_RECORD(db, recnum)			((JC_Record *)			(db)->Get((recnum), JC_Record::TypeRecord))
#define GET_OBJ(db, recnum)				((JC_Object *)			(db)->Get((recnum), JC_Record::TypeObject))
#define GET_LINK(db, recnum)			((AD_Link *)			(db)->Get((recnum), JC_Record::TypeLink))
#define GET_TEMPLATE(db, recnum)		((JC_Template *)		(db)->Get((recnum), JC_Record::TypeTemplate))
#define GET_MODEL(db, recnum)			((rv_Model *)			(db)->Get((recnum), JC_Record::TypeModel))
#define GET_ANIM_OBJ(db, recnum)		((rv_AnimatedObject *)	(db)->Get((recnum), JC_Record::TypeAnimatedObject))
#define GET_ANIM_TEMPLATE(db, recnum)	((rv_AnimatedTemplate *)(db)->Get((recnum), JC_Record::TypeAnimatedTemplate))
#define GET_PROP_NAME(db, recnum)	    ((JC_PropertyName *)	(db)->Get((recnum), JC_Record::TypePropertyName))
#define GET_PROP_REC(db, recnum)		((JC_PropertyRecord *)	(db)->Get((recnum), JC_Record::TypePropertyRecord))
#define GET_BITMAP(db, recnum)			((JC_Bitmap *)			(db)->Get((recnum), JC_Record::TypeBitmap))
#define GET_PALETTE(db, recnum)			((JC_Palette *)			(db)->Get((recnum), JC_Record::TypePalette))
#define GET_TEXTURE(db, recnum)			((PS_Texture *)			(db)->Get((recnum), JC_Record::TypeTexture))
#define GET_TEX_ANIM(db, recnum)		((PS_TextureAnimation *)(db)->Get((recnum), JC_Record::TypeTextureAnimation))
#define GET_FOLDER(db, recnum)			((PS_Folder *)			(db)->Get((recnum), JC_Record::TypeFolder))
#define GET_PROJECT(db, recnum)			((PS_Project *)			(db)->Get((recnum), JC_Record::TypeProject))
#define GET_TEX_SET(db, recnum)			((JC_TextureSet *)		(db)->Get((recnum), JC_Record::TypeNewTextureSet))
#define GET_DIAG_TMPL_REC(db, recnum)	((JC_DialogTemplateRecord *) (db)->Get((recnum), JC_Record::TypeDialogTemplateRecord))
#define GET_ENUM_REC(db, recnum)		((JC_EnumRecord *)		(db)->Get((recnum), JC_Record::TypeEnumRecord))
#define GET_TEX_SET_FOLDER(db, recnum)	((PS_TextureSetFolder *)(db)->Get((recnum), JC_Record::TypeTextureSetFolder))

#define NEW_OBJ(db, name)				((JC_Object *)			(db)->New((name), JC_Record::TypeObject))
#define NEW_TEMPLATE(db, name)			((JC_Template *)		(db)->New((name), JC_Record::TypeTemplate))
#define NEW_ANIM_OBJ(db, name)			((rv_AnimatedObject *)	(db)->New((name), JC_Record::TypeAnimatedObject))
#define NEW_ANIM_TEMPLATE(db, name)		((rv_AnimatedTemplate *)(db)->New((name), JC_Record::TypeAnimatedTemplate))
#define NEW_PROP_NAME(db, name)			((JC_PropertyName *)	(db)->New((name), JC_Record::TypePropertyName))
#define NEW_PROP_REC(db, name)			((JC_PropertyRecord *)	(db)->New((name), JC_Record::TypePropertyRecord))
#define NEW_BITMAP(db, name)			((JC_Bitmap *)			(db)->New((name), JC_Record::TypeBitmap))
#define NEW_PALETTE(db, name)			((JC_Palette *)			(db)->New((name), JC_Record::TypePalette))
#define NEW_TEXTURE(db, name)			((PS_Texture *)			(db)->New((name), JC_Record::TypeTexture))
#define NEW_TEX_ANIM(db, name)			((PS_TextureAnimation *)(db)->New((name), JC_Record::TypeTextureAnimation))
#define NEW_FOLDER(db, name)			((PS_Folder *)			(db)->New((name), JC_Record::TypeFolder))
#define NEW_PROJECT(db, name)			((PS_Project *)			(db)->New((name), JC_Record::TypeProject))
#define NEW_TEX_SET(db, name)			((JC_TextureSet *)		(db)->New((name), JC_Record::TypeNewTextureSet))
#define NEW_DIAG_TMPL_REC(db, name)		((JC_DialogTemplateRecord *) (db)->New((name), JC_Record::TypeDialogTemplateRecord))
#define NEW_ENUM_REC(db, name)			((JC_EnumRecord *)		(db)->New((name), JC_Record::TypeEnumRecord))
#define NEW_TEX_SET_FOLDER(db, name)	((PS_TextureSetFolder *)(db)->New((name), JC_Record::TypeTextureSetFolder))
//------------------------------------------------------------------------------------------------------------
// PS_Database : database access and memory cache
//------------------------------------------------------------------------------------------------------------
class JC_Record;
class PS_Record;
class JC_Vector;
class rv_AnimatedTemplate;
class JC_Template;
class JC_Object;
class rv_Model;
class PS_Folder;
class PS_Database;
class PS_CompressRecordList;
//------------------------------------------------------------------------------------------------------------
class PS_CacheEntry
{
	JC_Record*	m_pData;
	U32			m_ID;
public:
	PS_CacheEntry(U32 ID, JC_Record* pData);
	~PS_CacheEntry(void);
	JC_Record*	Data(void) const;
	U32			ID  (void) const;
};
//------------------------------------------------------------------------------------------------------------
class JC_Bitmap;
typedef CMap < U32, U32&, PS_CacheEntry*, PS_CacheEntry*& > PS_CacheMap;
typedef CMap < U32, U32&, U32, U32& > PS_RecordRemap;
typedef CList <U32, U32&> PS_DWordList;
//typedef CList < JC_Record*, JC_Record*& > PS_RecordList;
// PS_RecordList a list of records stored as record numbers
//------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL PS_RecordList
{
	PS_DWordList m_List;
	PS_Database * m_Database;
public:
	PS_RecordList(PS_Database * db) { m_Database = db; }
	POSITION GetHeadPosition(void) { return m_List.GetHeadPosition(); }
	void AddTail(const JC_Record * const record);
	JC_Record * GetNext(POSITION& pos);
	void RemoveAll(void) { m_List.RemoveAll(); }
	void RemoveAt(POSITION pos){ m_List.RemoveAt(pos); }
	U32 GetCount(void) { return m_List.GetCount(); }
	bool Contains(const JC_Record * const record) const;
	PS_Database * Database(void) const { return m_Database; }
};
//------------------------------------------------------------------------------------------------------------
class PS_Cache
{
	PS_CacheMap m_pData;
	PS_DWordList m_ReleaseList;
public:
	void CommitAll(void);
	~PS_Cache(void);
	void Purge(void);
	void FreeUnreferencedRecords(void);
	void ForceFreeUnreferencedRecords(void);
	void Add(const U32 ID, const JC_Record* pData);
	void AddToReleaseList(const U32 id);
	void RemoveFromReleaseList(const U32 id);
	void Remove(const U32 ID);
	JC_Record* Get(const U32 ID) const;
};
//------------------------------------------------------------------------------------------------------------
class PS_ByteArray : public CByteArray
{
public:
	void Init(U8* pdata, U32 size);
};
//------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL PS_RecordInfo
{
	U32		m_ObjectId;
	CString m_Name;
	U32		m_Type;
	U32		m_Lock;
	COleDateTime m_Date;
	CString m_Comment;
public:
	U32	Record(void) const;
	CString RecordNumber(void) const;
	CString Name(void) const;
	U32 Type(void) const;
	CString TypeString(void) const;
	CString Lock(void) const;
	CString Date(void) const;
	CString Comment(void) const;
	BOOL IsModel(void) const;
	void RecordNumber(U32);
	void Name(CString name);
	void Type(U32 type);
	void Lock(U32 lock);
	void Date(COleDateTime date);
	void Comment(CString comment);
};
//------------------------------------------------------------------------------------------------------------
class PS_Record : public CDaoRecordset
{
public:
	PS_Record(CDaoDatabase* pDatabase = NULL);
	DECLARE_DYNAMIC(PS_Record)

// Field/Param Data
	//{{AFX_FIELD(PS_Record, CDaoRecordset)
	long			m_ObjectId;
	CString			m_Name;
	long			m_Type;
	long			m_Lock;
	COleDateTime	m_Date;
	CString			m_Comment;
	PS_ByteArray	m_Blob;
	//}}AFX_FIELD

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(PS_Record)
	public:
	virtual CString GetDefaultDBName();		// Default database name
	virtual CString GetDefaultSQL();		// Default SQL for Recordset
	virtual void DoFieldExchange(CDaoFieldExchange* pFX);  // RFX support
	//}}AFX_VIRTUAL

// Implementation
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
};
//------------------------------------------------------------------------------------------------------------
typedef CMap <U32, U32&, CDWordArray*, CDWordArray*&> PS_RecordListMap;
class JCCLASS_DLL PS_Database : public CDaoDatabase
{
	CDaoTableDef* m_pTable;
	PS_Cache m_Cache;
	PS_Record * m_pRecord;
	BOOL m_ExtraNameInfo;
	PS_ProcessingDialog* m_ProgressDlg;
	PS_RecordListMap * m_TempRecordListMap; // temporary map used to cache calls to CreateRecordList

	bool	m_EnableLostFound;

public:
	void CommitAll(void);
	void SetAsReleaseTarget(const U32 id) { m_Cache.AddToReleaseList(id); }
	void RemoveReleaseTarget(const U32 id) { m_Cache.RemoveFromReleaseList(id); }
	bool CopyRecordList(CDWordArray& input_list, PS_Folder& target_folder);
	bool CopyRecord(const U32 src_rec, U32& new_rec, PS_RecordRemap& copy_map);
	PS_ProcessingDialog * ProgressDlg(void) const { return m_ProgressDlg; }
	void ProgressDlg(PS_ProcessingDialog * dialog) { ASSERT(m_ProgressDlg == NULL); m_ProgressDlg = dialog; }
	void CloseProgressDlg(void) { delete m_ProgressDlg; m_ProgressDlg = NULL; }
	void Clean(void);
	void CreateRecordList(PS_RecordList& bitmap_list, U32 type);
	void RemapDuplicateBitmaps(PS_RecordList& bitmap_list, PS_RecordRemap& remap);
	void RemoveDuplicateBitmaps(PS_RecordRemap& remap);
	void RemapDuplicateTextures(PS_RecordList& texture_list, PS_RecordRemap& remap, double tolerance);
	void RemoveDuplicateTextures(PS_RecordRemap& remap);
	void RemoveTextureList(PS_RecordList& texture_list);
	JC_Object* CreateObjectFromTemplate(JC_Template *tmpl, PS_Folder *folder = NULL);
	JC_Object* CreateObjectFromObject(JC_Object* src_obj, PS_Folder *folder = NULL);
	U32  CloneObjectFromTemplate(U32 sourcerecnum, U32 destrecnum, JC_Vector& position);
	U32  CloneObject(U32 sourcerecnum, U32 destrecnum, const JC_Vector& position, const JC_Vector& up, const JC_Vector& forward);
	void MoveModel(U32 model_num, U32 new_parent_num, bool synchronise = true);
	static	HINSTANCE m_Instance;
	U32 GetRecordPrompt(CDWordArray * masks = NULL, CWnd * parent = NULL);
	enum { NullRecord = 0, ProjectRecord = 1 };
	PS_Database(LPCSTR pFilename);
	PS_Database(CDaoWorkspace * workspace = NULL);
	~PS_Database(void);
	void CreateProjectNode(void);
	BOOL	Open(LPCSTR pFilename);
	BOOL	Create(LPCSTR pFilename);
	void	Close(void);
	U32		RecordCount(void) const;
	BOOL	FirstRecord(PS_RecordInfo& info, U32 record_type = 0);
	BOOL	GetInfo(U32 recnum, PS_RecordInfo& info);
	BOOL	NextRecord(PS_RecordInfo& info, U32 record_type = 0);
	BOOL    NextRecord(U32 recnum, PS_RecordInfo& info, U32 record_type = 0);
	U32		RecordLength(U32 recnum) const;
	CString	RecordName(U32 recnum) const;
	COleDateTime RecordDate(U32 recnum) const;
	U32		RecordNum(const CString& name, U32 type) const;
	U32		FindRecord(const CString& searchpath, const U32 type);
	U32		FindFolder(CString& path);
	void	EnableLostFound(bool enable);
	void	AddToLostAndFoundFolder(U32 lost_rec_num);
	JC_Record * Load(const U32 rec_num, const U32 type = 0);
	JC_Record * Get (const U32 rec_num, const U32 type = 0);
	//--------------------------------------------------------------
	// Fix for bodge up with version numbers
	JC_Record * GetErrorRecord(U32 recnum, U32 type);
	//--------------------------------------------------------------
	JC_Record * Put(U32 recnum);
	JC_Record * Put(U32 recnum, CString& comment);
	void CommitRecord(U32 recnum);
	JC_Record * New(const CString& name, U32 type, U32 parent = ProjectRecord);

	void					AnimateTree(rv_Model *base, rv_Model *new_parent);
	rv_AnimatedTemplate*	CreateAnimation(JC_Template *base);
	rv_AnimatedTemplate*	NewAnimatedTemplate(JC_Template* tmpl);

	void	Write(JC_Record * pData, PS_ByteArray& blob);
	void	Read (JC_Record * pData, PS_ByteArray& blob);
	void	PurgeCache(void);
	void	FreeCache(void);
	void	ForceFreeCache(void);
	void	PurgeRecord(U32 record);
	CString GetName(void) { if (IsOpen()) return CDaoDatabase::GetName(); else return CString(""); }
	BOOL BackupDatabase();
	BOOL RestoreBackup();
	BOOL UpgradeProject(U16 version);
	void Synchronise();
	U32 CountRecords();
protected:
	void DeleteDataNotInList(PS_CompressRecordList& list);
	void DeleteTempRecordMap(void);
	void Init(void);
	void SetRecordInfo(PS_RecordInfo& info);
	void SetIndex(PS_Record* precord, const CString table) const;
	void SearchId(PS_Record* precord, long recnum) const;
	void OpenTable(void);
	void CreateTable(void);
	void CreateRecordIndex(void);
	void CreateNameIndex(void);
	void CreateDateIndex(void);
	void CreateTypeIndex(void);
	void CreateIndex(CDaoIndexInfo& index);
	JC_Record* GetNewObject(int type, U32 recnum);
	void UpgradeProjectV2toV3(void);
	void UpgradePropertyName(U32 record);
	void UpgradeFolder(U32 record);
	void UpgradeBitmap(U32 record);
	void UpgradeTemplate(U32 record);
};

#endif
