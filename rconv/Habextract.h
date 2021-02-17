//-------------------------------------------------------------------------------------------------
// The API User must NOT edit this file
//-------------------------------------------------------------------------------------------------
#ifndef habextract_h
#define habextract_h
//-------------------------------------------------------------------------------------------------
#include "math.h"
#include "resource.h"
#include "habextractdlg.h"
#include "AD_Link.h"
#include "rv_ObjTransformation.h"
#include "rv_ObjAnimation.h"
#define PLAY_FWD PS_TextureAnimation::Forward
#define PLAY_BACK PS_TextureAnimation::Backward
#define PLAY_PONG PS_TextureAnimation::PingPong
#define DIALOG_GET_ALL_DATA JC_BucketObject::DialogGetAllData
#define DIALOG_GET_MODIFIED_DATA JC_BucketObject::DialogGetModifiedData
//-------------------------------------------------------------------------------------------------
// Simple structures for the api user
//-------------------------------------------------------------------------------------------------
enum TRANSFORM
{
	UNKNOWN,
	ROTATION,
	TRANSLATION,
	MORPH,
	PROPERTY,
};
//-------------------------------------------------------------------------------------------------
struct QUV
{
	double U;
	double V;
};
//-------------------------------------------------------------------------------------------------
struct QVERTEX
{
	double X;
	double Y;
	double Z;
};
//-------------------------------------------------------------------------------------------------
struct QCOLOUR
{
	U8 R;
	U8 G;
	U8 B;
	U8 A;
};
//-------------------------------------------------------------------------------------------------
struct QPOINT
{
	QVERTEX Vertex;
	QCOLOUR Colour;
};
//-------------------------------------------------------------------------------------------------
struct QPALETTE
{
	U16 Size; // number of colours in the palette
	QCOLOUR Entry[1];
};
//-------------------------------------------------------------------------------------------------
// I didn't call it PS_BucketTexture as it isn't in a bucket
//-------------------------------------------------------------------------------------------------
class JC_TextureSet;
class PS_ExportTexture // Alias QTEXTURE
{
	const PS_Texture * m_pTexture;
	PS_FloatUV m_UV;
	QUV * m_UVArray;
	U32 m_Count;
	U32 m_Iterator;
	BITMAPINFO * m_BitmapInfo;
	const PS_TextureSetFolder * m_pTextureSetGroup;
	static const char * m_NotFound;
	CList < const PS_ExportTexture*, const PS_ExportTexture*& > m_CelList;
	BYTE * m_Palette;
	JC_TextureSet * m_TextureSet;
public:
	PS_ExportTexture(BA_VertexOrient * orientation, const PS_Texture * texture, const PS_TextureSetFolder * texture_set, const U16 number_of_vertices);
	~PS_ExportTexture(void);
	QUV * GetFirstUV(void);
	QUV * GetNextUV(void);
	CString GetTextureName(void) const { return m_pTexture->Name(); }
	JC_Bitmap * Bitmap(void);
	U32 GetNumberOfUVCoords(void) { return m_Count; }
	U32 GetBitmapID(void) { JC_Bitmap * bitmap = Bitmap(); if (bitmap) return bitmap->Number(); else return 0; }
	const char * GetBitmapName(void);
	U32 GetBitmapDataSize(void);
	BITMAPINFO * GetBitmapData(void);
	BOOL IsAnimation(void) const { return m_pTexture->IsAnimation(); }
	U32 GetNumberOfCels(void) const;
	PS_ExportTexture * GetCel(U32 index) const;
	U32 GetNumberOfRepeats(void) const;
	U32 GetRepeatMode(void) const;
	U32 GetCelDuration(U32 index) const;
	U32 GetTextureID() const { return m_pTexture->Number(); }
	U32 GetPaletteID();
	BOOL HasOwnPalette() const { return (m_pTexture->Palette()->RecordNum() != PS_Database::NullRecord); }
	QPALETTE * GetPalette();
};
//-------------------------------------------------------------------------------------------------
class JC_BucketItem
{
private:
	BOOL m_Removed;			// Flags item has been "removed" from it's container

public:
	JC_BucketItem();
	virtual ~JC_BucketItem() { }

	void Report(void);
	void ApplyFilter(char * filter);
	JC_String GetNameString(const CString& s);
	BOOL IsRemoved(void)				const	{ return m_Removed; }

public:	// Virtual functions
	virtual JC_Data * Property(const CString&) const = 0;
	virtual BOOL NameMatches(const JC_RegularExpression&) = 0;
};
//-------------------------------------------------------------------------------------------------
class JC_BucketObject;
class PS_ExportDialog;
//-------------------------------------------------------------------------------------------------
class JC_BucketFace : public JC_BucketItem		// Alias QFACE
{
private:
	QPOINT m_Point;			// Reused each time GetNextPoint is called
	JC_Face * m_Face;		// Ptr to JC_Face that this represents
	POSITION m_PointPos;	// Position for point list
	PS_ExportTexture * m_FrontTexture;
	PS_ExportTexture * m_BackTexture;
	PS_TextureSetFolder * m_Group;
	JC_BucketObject * m_Object; // pointer back to the object that contains this face

public:
	JC_BucketFace(JC_Face * face, JC_BucketObject * object);
	virtual ~JC_BucketFace();

	U32 GetPointCount(void);
	QPOINT GetFirstPoint(void);
	virtual QPOINT GetNextPoint(void);
	PS_TextureSetFolder * GetTextureSetGroup(const char * path = NULL);
	void SetTextureSetGroup(const char * path);
	PS_ExportTexture * GetFrontTexture(const PS_TextureSetFolder * group = NULL); // these return null if no texture 
	PS_ExportTexture * GetBackTexture(const PS_TextureSetFolder * group = NULL);  // is applied to the face
	PS_ExportTexture * GetFrontTexture(const char * path ); // these return null if no texture 
	PS_ExportTexture * GetBackTexture(const char * path);  // is applied to the face
	PS_ExportDialog * GetDialogData(const CString& name);

public:	// Virtual overrides
	JC_Data * Property(const CString& name) const;
	BOOL NameMatches(const JC_RegularExpression&);
};
//-------------------------------------------------------------------------------------------------
class PS_ExportMorphFace: public JC_BucketFace
{
	QVERTEX m_MorphOffset;
public:
	PS_ExportMorphFace(JC_Face * face, JC_BucketObject * object, QVERTEX morph_offset);
	QPOINT GetNextPoint(void);
};
//-------------------------------------------------------------------------------------------------
class PS_ExportStruct
{
	JC_DialogSpreadsheet * m_ArrayItem;
	JC_NamedDataCollection * m_Data;
public:
	PS_ExportStruct(JC_NamedDataCollection * data, JC_DialogSpreadsheet * array_item);
	JC_DataConcrete * ExtractData(const CString& name);
};
//-------------------------------------------------------------------------------------------------
class PS_ExportArray
{
	JC_NamedDataArray * m_DataArray;
	JC_NamedDataArray * m_BackDataArray;
	JC_DialogSpreadsheet * m_ArrayItem;
	CArray <PS_ExportStruct*, PS_ExportStruct*> m_StructArray;
public:
	PS_ExportArray(JC_NamedDataArray * data, JC_NamedDataArray * back_data, JC_DialogSpreadsheet * array_item);
	~PS_ExportArray(void);
	PS_ExportStruct * GetRow(const U32 row);
	U32 GetSize(void);
};
//-------------------------------------------------------------------------------------------------
class PS_ExportDialog
{
	JC_DialogData * m_DialogData;
	JC_DialogData * m_BackDialogData;
	CArray <PS_ExportArray*, PS_ExportArray*&> m_ArrayArray;
public:
	PS_ExportDialog(JC_DialogData * data, JC_DialogData * back_data = NULL);
	~PS_ExportDialog(void);
	JC_DataConcrete * ExtractData(const CString& name);
	PS_ExportArray * ExtractArray(const CString& name);
protected:
	JC_DataConcrete * ExtractData(const CString& name, JC_DialogData * dialog_data);
};
//-------------------------------------------------------------------------------------------------
class JC_BucketObject;
class PS_ExportLink
{
	const AD_Link * m_Link;
	const JC_BucketObject * m_RootBucketObject;
public:
	PS_ExportLink(const AD_Link * link, const JC_BucketObject * root);
	~PS_ExportLink(void);
	U32 GetLinkID(void) const;
	BOOL IsBidirectional(void) const;
	JC_BucketObject * GetSourceObject(void) const;
	JC_BucketObject * GetDestinationObject(void) const;
	JC_Data * Property(const CString& name) const;
protected:
	JC_BucketObject * GetObject(U32 record_number) const;
};
//-------------------------------------------------------------------------------------------------
class JC_BucketFaceArray : public CTypedPtrArray<CPtrArray, JC_BucketFace*>	{ };
class PS_ExportTransform
{
	const rv_ObjTransBase * m_Transform;
	JC_BucketFaceArray m_FaceArray;
	S32 m_FaceArrayIdx;
	JC_BucketObject * m_Object;
public:
	PS_ExportTransform(const rv_ObjTransBase * transform, JC_BucketObject * object);
	~PS_ExportTransform(void);

	CString Name(void) const;
	float Duration(void) const;
	float StartTime(void) const;
	TRANSFORM Type(void) const;
	BOOL ApplyLocally(void) const;
	float RotationAngle(void) const;
	QVERTEX RotationAxis(void) const;
	QVERTEX CentreOfRotation(void) const;
	QVERTEX TranslationVector(void) const;
	JC_Data * GetPropertyValue(void) const;
	LPCSTR GetPropertyName(void) const;
	JC_BucketFace * GetFirstFace(void) const;
	JC_BucketFace * GetNextFace(void) const;
protected:
	void LoadFaces(void);
	void UnloadFaces(void);
};
//-------------------------------------------------------------------------------------------------
class JC_BucketObjectArray : public CTypedPtrArray<CPtrArray, JC_BucketObject*>	{ };
typedef CTypedPtrArray<CPtrArray, PS_ExportDialog*> PS_DialogArray;
typedef CTypedPtrArray<CPtrArray, PS_ExportTransform*> PS_TransformArray;
//-------------------------------------------------------------------------------------------------
class JC_BucketObject : public JC_BucketItem	// Alias QOBJECT
{
	enum GetDataType
	{
		DialogGetAllData,
		DialogGetModifiedData
	};

private:
	int m_Depth;
	S32 m_ChildArrayIdx;
	BOOL m_FacesLoaded;
	BOOL m_VerticesLoaded;
	QVERTEX m_Vertex;			// Reused each time GetNextVertex is called
	U32 m_RecordNum;			// Record number of JC_Object that this represents
	S32 m_FaceArrayIdx;
	S32 m_VertexArrayIdx;
	S32 m_TransformArrayIdx;
	S32 m_LinkIdx;
	JC_Vector m_ParentPos;
	JC_BucketFaceArray m_FaceArray;
	JC_VertexArray m_VertexArray;
	JC_BucketObject * m_Parent;
	JC_BucketObjectArray m_ChildArray;
	PS_DialogArray m_DialogArray;
	PS_TransformArray m_TransformArray;

public:
	QVERTEX GetAnchorPos(void) const;
	QVERTEX GetUpWorld(void) const;
	QVERTEX GetDirWorld(void) const;
	JC_BucketObject(rv_Model * obj, JC_BucketObject * parentbo, int depth);
	virtual ~JC_BucketObject();

	QVERTEX ApplyTransforms(const QVERTEX vertex) const;
	QVERTEX ApplyRotations(const QVERTEX vector) const;
	QVERTEX ApplyScales(const QVERTEX vector) const;
	void RemoveFaces(char * filter);
	void LoadFaces(void);
	void UnloadFaces(void);
	void LoadVertices(void);
	void UnloadVertices(void);
	const char * GetName(void);
	const char * GetComment(void);
	U32  GetFaceCount(void);
	JC_BucketFace * GetFirstFace(void);
	JC_BucketFace * GetNextFace(void);
	U32  GetVertexCount(void);
	QVERTEX GetFirstVertex(void);
	QVERTEX GetNextVertex(void);
	void AddChild(JC_BucketObject * bo);
	QVERTEX GetPosWorld(void) const;
	QVERTEX GetPosParent(void) const;
	BOOL HasParent(void) const { return Object()->HasParent(); }

	U32 GetNumChildren(void);
	JC_BucketObject * GetParent(void);
	JC_BucketObject * GetFirstChild(void);
	JC_BucketObject * GetNextChild(void);

	U32 GetLinkCount(void) const;
	PS_ExportLink * GetFirstLink(void) const;
	PS_ExportLink * GetNextLink(void) const;
	U32 GetObjectID(void) const;
	JC_BucketObject * FindObject(U32 record_number) const;

	BOOL IsAnimatedObject(void) const;
	U32 GetNumberOfTransforms(void) const;
	PS_ExportTransform * GetFirstTransform(void) const;
	PS_ExportTransform * GetNextTransform(void) const;

public:	// Virtual overrides
	void Report(void) const;
	JC_Data * Property(const CString& name) const;
	PS_ExportDialog * GetDialogData(const CString& name, const GetDataType mode = DIALOG_GET_ALL_DATA);
	BOOL NameMatches(const JC_RegularExpression&);
	void AddToDialogArray(PS_ExportDialog * dialog) { m_DialogArray.Add(dialog); }

protected:
	JC_Data * Property(const CString& name, const rv_Model * model) const;
	JC_BucketObject * GetRootBucketObject(void) const;
	QVERTEX ApplyTransforms(const QVERTEX vertex, rv_Model * model) const;
	QVERTEX ApplyRotation(const QVERTEX vector, rv_Model * model) const;
	QVERTEX ApplyScale(const QVERTEX vector, rv_Model * model) const;
private:
	void DeleteFaceArray(void);
	void DeleteVertexArray(void);
	rv_Model * Object(void) const;
};
//-------------------------------------------------------------------------------------------------
class JC_Bucket				// Alias QBUCKET
{
private:
	int m_Indent;
	S32 m_ObjectArrayIdx;
	JC_BucketObjectArray m_ObjectArray;

public:
	JC_Bucket();
	virtual ~JC_Bucket();

	void RemoveObjects(char * filter);
	JC_BucketObject * GetFirstObject(void);
	JC_BucketObject * GetNextObject(void);
	JC_BucketObject * GetFirstModel(void);
	JC_BucketObject * GetNextModel(void);
	void Fill(const char * objname, BOOL recurse);
	void Add(JC_BucketObject * bo)		{ m_ObjectArray.Add(bo); }
	int  GetNumObjects(void)	const;
	int	 GetNumObjectDescendants(void) const { return m_ObjectArray.GetSize(); }

private:
	JC_BucketObject * BuildBucketObject(rv_Model * obj, JC_BucketObject * parentbo, BOOL recurse);
};
//-------------------------------------------------------------------------------------------------
typedef CTypedPtrList<CPtrList, PS_ExportLink*> PS_ExportLinkList;
//-------------------------------------------------------------------------------------------------
class CHabExtractApp : public CWinApp
{
private:
	PS_TextureSetFolder * m_TextureSetGroup;
	int		m_Indent;
	BOOL	m_Abort;
	BOOL	m_Paused;
	BOOL	m_Waiting;
	BOOL	m_AutoRun;
	U32		m_Progress;
	U32		m_ProgressCnt;
	PS_Database * m_Database;
	JC_Bucket * m_Bucket;
	CHabExtractDlg * m_Dialog;
	CDaoWorkspace * m_Workspace;
	PS_ExportLinkList m_LinkList;

public:
	CHabExtractApp();
	~CHabExtractApp();

	BOOL	SetAutoRun(BOOL auto_run)   { m_AutoRun = auto_run; return m_AutoRun; }
	BOOL	GetAutoRun(void) const	    { return m_AutoRun; }
	void	SetProgress(U32 cnt)		{ m_Progress = 0; m_ProgressCnt = cnt; }
	void	AdvanceProgress(void);
	void	Finish(void);
	void	Log(const char * message);
	void	Error(const char * message);
	BOOL	Update(void);
	BOOL	Update(int percent);
	BOOL	Begin(const char * text, const char * database);
	void	End(const char * text);
	JC_Bucket * GetBucket(const char * rootname, BOOL recurse = TRUE);
	void	Go(void)					{ m_Waiting = FALSE; }
	void	Abort(void)					{ m_Abort = TRUE; }
	PS_Database& Database(void)	const	{ return *m_Database; }
	BOOL	IsPaused(void)		const	{ return m_Paused; }
	void	IsPaused(BOOL flag)			{ m_Paused = flag; }
	void	Indent(int x)				{ m_Indent += x; }
	PS_TextureSetFolder * TextureSetGroup(void) const { return m_TextureSetGroup; }
	BOOL SetTextureGroup(const char * path);
	const char * Plural(int n)			{ if (n == 1) return ""; return "s"; }
	void	ShowProgress(const char * format_string, double percent) { if (m_Dialog) m_Dialog->ShowProgress(format_string, percent); }

	void AddLink(PS_ExportLink * link) { m_LinkList.AddTail(link); }

private:
	void	ClearBucket(void);
	void	Display(int id);
	void	Display(const char * buf);
	void	Display(const CString& string);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHabExtractApp)
	public:
	virtual BOOL InitInstance();
	virtual int  ExitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CHabExtractApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
//-------------------------------------------------------------------------------------------------
// For user convenience
//-------------------------------------------------------------------------------------------------
#define	api theApp
extern	CHabExtractApp theApp;
//-------------------------------------------------------------------------------------------------
typedef JC_DataConcrete	 QVALUE;
typedef	JC_Bucket		 QBUCKET;
typedef	JC_BucketFace	 QFACE;
typedef	JC_BucketObject	 QOBJECT;
typedef PS_ExportTexture QTEXTURE;
typedef PS_ExportDialog	 QDIALOG;
typedef JC_Data			 QDATA;
typedef PS_ExportArray	 QARRAY;
typedef PS_ExportStruct	 QSTRUCT;
typedef PS_TextureSetFolder QTEXTUREGROUP;
typedef PS_ExportLink	 QLINK;
typedef PS_ExportTransform QTRANSFORM;
//-------------------------------------------------------------------------------------------------
#endif
//-------------------------------------------------------------------------------------------------
