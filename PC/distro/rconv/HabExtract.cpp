//-------------------------------------------------------------------------------------------------
#include "stdafx.h"
#include "HabExtract.h"
#include "HabExtractDlg.h"
#include "rv_Matrix4x4.H"
#include <afxole.h>
#include "JC_TextureSet.H"
//-------------------------------------------------------------------------------------------------
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
//-------------------------------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(CHabExtractApp, CWinApp)
	//{{AFX_MSG_MAP(CHabExtractApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()
//-------------------------------------------------------------------------------------------------
const char * PS_ExportTexture::m_NotFound = "Bitmap not found\0";
//-------------------------------------------------------------------------------------------------
void CheckMessages(void)
{
	MSG msg;

	while (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		if (msg.message == WM_QUIT)
		{
			api.Abort();
			break;
		}

		if (!api.PreTranslateMessage(&msg))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
	}
}
//-------------------------------------------------------------------------------------------------
CHabExtractApp theApp;
//-------------------------------------------------------------------------------------------------
CHabExtractApp::CHabExtractApp()
{
	m_Indent  = 0;
	m_Progress = 0;
	m_ProgressCnt = 1;
	m_Bucket  = NULL;
	m_Waiting = TRUE;
	m_Abort   = FALSE;
	m_Dialog  = FALSE;
	m_Paused  = FALSE;
	m_AutoRun = FALSE;		// Set from command line?
}
//-------------------------------------------------------------------------------------------------
CHabExtractApp::~CHabExtractApp(void)
{
	POSITION pos = m_LinkList.GetHeadPosition();
	while (pos)
	{
		delete m_LinkList.GetNext(pos);
	}
}
//-------------------------------------------------------------------------------------------------
void CHabExtractApp::AdvanceProgress(void)
{
	int percent = (++m_Progress * 100) / m_ProgressCnt;

	if (percent > 100) percent = 100;

	Update(percent);
}
//-------------------------------------------------------------------------------------------------
BOOL CHabExtractApp::Begin(const char * text, const char * database)
{
	CTime t = CTime::GetCurrentTime();

	Display(IDS_TITLE);
	Display(t.Format(" - %A %d %B %Y\n"));

	char buf[256];
	sprintf(buf, "%s, compiled on %s at %s\n", text, __DATE__, __TIME__);
	Display(buf);

	if (m_AutoRun)
	{
		Display(" - auto-running");
		m_Dialog->StartRun();
	}

	Display("\n");

	m_Database = new PS_Database(m_Workspace);
	m_Database->Open(database);

	while (m_Waiting)
	{
		CheckMessages();

		if (m_Abort) return FALSE;
	}
	m_TextureSetGroup = NULL;
	return TRUE;
}
//-------------------------------------------------------------------------------------------------
BOOL CHabExtractApp::SetTextureGroup(const char * path)
{
	if (m_Database)
	{
		if (path)
		{
			U32 rec = m_Database->FindRecord(path, JC_Record::TypeTextureSetFolder);
			if (rec != PS_Database::NullRecord)
			{
				PS_TextureSetFolder * group = (PS_TextureSetFolder*)m_Database->Get(rec);
				if (!group)
				{
					return FALSE;
				}
				else
				{
					POSITION pos = group->ChildList().GetHeadPosition();
					while (pos)
					{
						JC_RecordRef * ref = static_cast<JC_RecordRef*>(group->ChildList().GetNext(pos));
						JC_Record * record = ref->Get(JC_Record::TypeRecord);
						// don't release these records!
					}
				}
				m_TextureSetGroup = group;
				return TRUE;
			}
		}
		else
		{
			m_TextureSetGroup = NULL;
			return TRUE;
		}
	}
	return FALSE;
}
//-------------------------------------------------------------------------------------------------
void CHabExtractApp::End(const char * text)
{
	ClearBucket();

	Display("\n");
	Display(text);
	Display("\n");
}
//-------------------------------------------------------------------------------------------------
BOOL CHabExtractApp::Update(int percent)
{
	m_Dialog->UpdateProgress(percent);

	return Update();
}
//-------------------------------------------------------------------------------------------------
BOOL CHabExtractApp::Update(void)
{
	do
	{
		CheckMessages();

		if (m_Abort) break;
	}
	while (m_Paused);

	return m_Abort;
}
//-------------------------------------------------------------------------------------------------
void CHabExtractApp::Error(const char * message)
{
	if (!m_Abort)
	{
		::AfxMessageBox(message, MB_ICONSTOP);

		Log(message);
		Display("\n");

		m_Abort = TRUE;
	}
}
//-------------------------------------------------------------------------------------------------
void CHabExtractApp::Log(const char * message)
{
	CTime t = CTime::GetCurrentTime();

	CString s = t.Format("%H:%M:%S - ");

	m_Dialog->Display(s);
  
	for (int i = 0; i < m_Indent; i++)
	{
		m_Dialog->Display(" ");
	}

	m_Dialog->Display(message);
}
//-------------------------------------------------------------------------------------------------
void CHabExtractApp::Display(int id)
{
	m_Dialog->Display(id);
}
//-------------------------------------------------------------------------------------------------
void CHabExtractApp::Display(const CString& string)
{
	m_Dialog->Display(string);
}
//-------------------------------------------------------------------------------------------------
void CHabExtractApp::Display(const char * message)
{
	m_Dialog->Display(message);
}
//-------------------------------------------------------------------------------------------------
JC_Bucket * CHabExtractApp::GetBucket(const char * rootname, BOOL recurse)
{
	ClearBucket();

	m_Bucket = new JC_Bucket;

	TRY
	{
		m_Bucket->Fill(rootname, recurse);
	}
	CATCH(CException, e)
	{
		Error("Unable to fill bucket using specified root object");
		delete m_Bucket;
		m_Bucket = NULL;
	}
	END_CATCH

	return m_Bucket;
}
//-------------------------------------------------------------------------------------------------
void CHabExtractApp::ClearBucket(void)
{
	if (m_Bucket != NULL)
	{
		delete m_Bucket;
		m_Bucket = NULL;
	}
}
//-------------------------------------------------------------------------------------------------
BOOL CHabExtractApp::InitInstance(void)
{
	LoadLibrary("RICHED32.DLL");
	Enable3dControls();			// Call this when using MFC in a shared DLL
	AfxEnableControlContainer(); // to enable the edit window

	m_Dialog = new CHabExtractDlg;
	m_pMainWnd = m_Dialog;
	m_Dialog->Create(IDD_HABEXTRACT_DIALOG);
	m_Workspace = new CDaoWorkspace;
	m_Workspace->Open();
	return m_Dialog->MainProcessing();
}
//-------------------------------------------------------------------------------------------------
int CHabExtractApp::ExitInstance(void)
{
	ASSERT(m_Dialog != NULL);
	delete m_Dialog;
	m_Dialog = NULL;

	ClearBucket();

	ASSERT(m_Database != NULL);
	delete m_Database;
	m_Workspace->Close();
	delete m_Workspace;
	m_Workspace = NULL;

	return CWinApp::ExitInstance();
}
//-------------------------------------------------------------------------------------------------
void CHabExtractApp::Finish(void)
{
	if (m_Dialog != NULL)
	{
		if (m_AutoRun)
		{
			m_Dialog->PostMessage(WM_CLOSE);
		}
		else
		{
			m_Dialog->Final();
		}
	}
}
//-------------------------------------------------------------------------------------------------
// JC_Bucket
//-------------------------------------------------------------------------------------------------
JC_Bucket::JC_Bucket()
{
	m_Indent = 0;
	m_ObjectArrayIdx = -1;
	m_ObjectArray.SetSize(0, 256);
}
//-------------------------------------------------------------------------------------------------
JC_Bucket::~JC_Bucket()
{
	for (int i = 0; i < m_ObjectArray.GetSize(); i++)
	{
		delete m_ObjectArray.GetAt(i);
	}

	m_ObjectArray.RemoveAll();
}
//-------------------------------------------------------------------------------------------------
JC_BucketObject * JC_Bucket::BuildBucketObject(rv_Model * obj, JC_BucketObject * parentbo, BOOL recurse)
{
	JC_BucketObject * bo = new JC_BucketObject(obj, parentbo, m_Indent);

	m_Indent++;

	JC_RecordRefList& list = obj->ChildList();
	POSITION pos = list.GetHeadPosition();
	while (pos != NULL)
	{
		JC_RecordRef * rr = (JC_RecordRef*)list.GetNext(pos);
		rv_Model * childobj = (rv_Model*)rr->Get(JC_Record::TypeModel);
		if (childobj)
		{
			JC_BucketObject * childbo = BuildBucketObject(childobj, bo, recurse);
			if (recurse)
			{
				Add(childbo);
			}
			bo->AddChild(childbo);
		}
	}
	JC_Template * tmpl = obj->TemplatePtr();
	if (tmpl != obj)
	{
		JC_RecordRefList& list2 = tmpl->ChildList();
		POSITION pos2 = list2.GetHeadPosition();
		while (pos2 != NULL)
		{
			JC_RecordRef * rr = (JC_RecordRef*)list2.GetNext(pos2);
			rv_Model * childobj = (rv_Model*)rr->Get(JC_Record::TypeModel);
			if (childobj)
			{
				JC_BucketObject * childbo = BuildBucketObject(childobj, bo, recurse);
				if (recurse)
				{
					Add(childbo);
				}
				bo->AddChild(childbo);
			}
		}
	}
	m_Indent--;

	return bo;
}
//-------------------------------------------------------------------------------------------------
void JC_Bucket::Fill(const char * objname, BOOL recurse)
{
	CWaitCursor busy;
	PS_Database& db = api.Database();

	CString path(objname);
	U32 path_num = db.FindFolder(path);
	PS_Folder * folder = (PS_Folder*)db.Get(path_num);
	if (folder)
	{
		POSITION pos = folder->FindFirstRecord(path, JC_Record::TypeRecord);
		while (pos)
		{
			U32 recnum = folder->GetNextRecord(path, JC_Record::TypeRecord, pos);
			if (recnum)
			{
				JC_Record * rec = db.Get(recnum, JC_Record::TypeRecord);
				if (rec->Type() == JC_Record::TypeObject || 
					rec->Type() == JC_Record::TypeAnimatedObject ||
					rec->Type() == JC_Record::TypeTemplate ||
					rec->Type() == JC_Record::TypeAnimatedTemplate)
				{
					rv_Model * obj = static_cast <rv_Model*>(rec);
					JC_BucketObject * bo = BuildBucketObject(obj, NULL, recurse);
					Add(bo);

					int num = m_ObjectArray.GetSize();

					TRACE("Collected %d object%s %s\n", num, api.Plural(num), recurse ? "recursively" : "");
				}
			}
		}
	}
}
//-------------------------------------------------------------------------------------------------
JC_BucketObject * JC_Bucket::GetFirstObject(void)
{
	m_ObjectArrayIdx = 0;

	return GetNextObject();
}
//-------------------------------------------------------------------------------------------------
JC_BucketObject * JC_Bucket::GetNextObject(void)
{
	if (m_ObjectArrayIdx < 0)
	{
		THROW(new JC_Exception("Called GetNextObject without calling GetFirstObject"));
	}

	if (api.Update()) return NULL;

	JC_BucketObject * bo = NULL;

	do
	{
		if (m_ObjectArrayIdx < m_ObjectArray.GetSize())
		{
			bo = m_ObjectArray[m_ObjectArrayIdx++];
		}
		else return NULL;
	}
	while (bo->IsRemoved() || bo->HasParent()); // only return root objects

 	return bo;
}
//-------------------------------------------------------------------------------------------------
JC_BucketObject * JC_Bucket::GetFirstModel(void)
{
	m_ObjectArrayIdx = 0;
	return GetNextModel();
}
//-------------------------------------------------------------------------------------------------
JC_BucketObject * JC_Bucket::GetNextModel(void)
{
	if (m_ObjectArrayIdx < 0)
	{
		THROW(new JC_Exception("Called GetNextModel without calling GetFirstModel"));
	}

	if (api.Update()) return NULL;

	JC_BucketObject * bo = NULL;

	do
	{
		if (m_ObjectArrayIdx < m_ObjectArray.GetSize())
		{
			bo = m_ObjectArray[m_ObjectArrayIdx++];
		}
		else return NULL;
	}
	while (bo->IsRemoved()); 

 	return bo;
}
//-------------------------------------------------------------------------------------------------
int JC_Bucket::GetNumObjects(void) const
{
	int count = 0;
	for (int i = 0; i < m_ObjectArray.GetSize(); i++)
	{
		if (!m_ObjectArray[i]->HasParent())
		{
			count++;
		}
	}
	return count;
}
//-------------------------------------------------------------------------------------------------
void JC_Bucket::RemoveObjects(char * filter)
{
	TRACE("Removing objects for '%s'\n", filter);

	CWaitCursor busy;

	for (S32 i = 0; i < m_ObjectArray.GetSize(); i++)
	{
		JC_BucketObject * bo = m_ObjectArray[i];

		if (!bo->IsRemoved())
		{
			bo->ApplyFilter(filter);
		}
	}
}
//-------------------------------------------------------------------------------------------------
// JC_BucketObject
//-------------------------------------------------------------------------------------------------
JC_BucketObject::JC_BucketObject(rv_Model * obj, JC_BucketObject * parentbo, int depth)
{
	m_Depth = depth;
	m_FaceArrayIdx = -1;
	m_Parent = parentbo;
	m_ChildArrayIdx = -1;
	m_VertexArrayIdx = -1;
	m_TransformArrayIdx = -1;
	m_LinkIdx = -1;
	m_FacesLoaded = FALSE;
	m_VerticesLoaded = FALSE;
	m_ChildArray.SetSize(0, 16);
	m_FaceArray.SetSize(0, 1024);
	m_VertexArray.SetSize(0, 1024);

	m_RecordNum = obj->Number();
	m_ParentPos = obj->RealAnchor().PinVector();
	
	if (obj->IsAnimated())
	{
		JC_Template * t = obj->TemplatePtr();
		if (t->IsAnimated())
		{
			rv_AnimatedTemplate * anim_temp = static_cast <rv_AnimatedTemplate*>(t);
			POSITION pos = anim_temp->TransList()->GetHeadPosition();
			while (pos)
			{
				PS_ExportTransform * transform;
				transform = new PS_ExportTransform((const rv_ObjTransBase*)anim_temp->TransList()->GetNext(pos), this);
				m_TransformArray.Add(transform);
			}
		}
	}
}
//-------------------------------------------------------------------------------------------------
rv_Model * JC_BucketObject::Object(void) const
{
	return static_cast <rv_Model*>(api.Database().Get(m_RecordNum, JC_Record::TypeRecord));
}
//-------------------------------------------------------------------------------------------------
JC_BucketObject::~JC_BucketObject()
{
	UnloadFaces();
	UnloadVertices();
	{
		U32 items = (U32)m_DialogArray.GetSize();
		while (items--)
		{
			delete m_DialogArray[items];
		}
	}
	{
		U32 items = (U32)m_TransformArray.GetSize();
		while (items --)
		{
			delete m_TransformArray[items];
		}
	}
}
//-------------------------------------------------------------------------------------------------
void JC_BucketObject::Report(void) const
{
	JC_String name = Object()->Name();

	char * str = "                ";

	CString s;
	s.Format("%sProcessing object '%s'\n", str+16-m_Depth, (LPCSTR)name);
	api.Log((LPCSTR)s);
}
//-------------------------------------------------------------------------------------------------
U32 JC_BucketObject::GetVertexCount(void)
{
	return m_VertexArray.GetSize();
}
//-------------------------------------------------------------------------------------------------
void JC_BucketObject::LoadVertices(void)
{
	Object()->GetVertices(m_VertexArray);

	m_VerticesLoaded = TRUE;
}
//-------------------------------------------------------------------------------------------------
QVERTEX JC_BucketObject::GetFirstVertex(void)
{
	if (!m_VerticesLoaded)
	{
		THROW(new JC_Exception("Called GetFirstVertex without calling LoadVertices"));
	}

	m_VertexArrayIdx = 0;

	return GetNextVertex();
}
//-------------------------------------------------------------------------------------------------
QVERTEX JC_BucketObject::GetNextVertex(void)
{
	QVERTEX ret_vert;	
	if (m_VertexArrayIdx < 0)
	{
		THROW(new JC_Exception("Called GetNextVertex without calling GetFirstVertex"));
	}

	if (m_VertexArrayIdx < m_VertexArray.GetSize())
	{
		JC_Vertex * v = m_VertexArray[m_VertexArrayIdx++];
		QVERTEX vert;
		vert.X = v->X();
		vert.Y = v->Y();
		vert.Z = v->Z();
		ret_vert = ApplyTransforms(vert);
	}
	else
	{
		THROW(new JC_Exception("Went passed the end of the vertex list"));
	}
	return ret_vert;
}
//-------------------------------------------------------------------------------------------------
void JC_BucketObject::UnloadVertices(void)
{
	for (int i = 0; i < m_VertexArray.GetSize(); i++)
	{
		delete m_VertexArray.GetAt(i);
	}

	m_VertexArray.RemoveAll();

	m_VerticesLoaded = FALSE;
}
//-------------------------------------------------------------------------------------------------
QVERTEX JC_BucketObject::GetPosWorld(void) const
{
	QVERTEX v = { 0.0, 0.0, 0.0 };

	return ApplyTransforms(v);
}
//-------------------------------------------------------------------------------------------------
QVERTEX JC_BucketObject::GetPosParent(void) const
{
	QVERTEX v;

	v.X = m_ParentPos.X();
	v.Y = m_ParentPos.Y();
	v.Z = m_ParentPos.Z();

	return v;
}
//-------------------------------------------------------------------------------------------------
U32 JC_BucketObject::GetFaceCount(void)
{
	return m_FaceArray.GetSize();
}
//-------------------------------------------------------------------------------------------------
void JC_BucketObject::LoadFaces(void)
{
	m_FaceArray.SetSize(0, 32);
	const JC_Template * temp = Object()->TemplatePtr();
	const JC_FaceList * fl = temp->FaceList();
	POSITION pos = fl->GetHeadPosition();
	while (pos)
	{
		JC_Face * f = (JC_Face*)fl->GetNext(pos);
		m_FaceArray.Add(new JC_BucketFace(f, this));
	}
	m_FacesLoaded = TRUE;
}
//-------------------------------------------------------------------------------------------------
QFACE * JC_BucketObject::GetFirstFace(void)
{
	if (!m_FacesLoaded)
	{
		THROW(new JC_Exception("Called GetFirstFace without calling LoadFaces"));
	}

	m_FaceArrayIdx = 0;

	return GetNextFace();
}
//-------------------------------------------------------------------------------------------------
QFACE * JC_BucketObject::GetNextFace(void)
{
	if (m_FaceArrayIdx < 0)
	{
		THROW(new JC_Exception("Called GetNextFace without calling GetFirstFace"));
	}

	JC_BucketFace * bf = NULL;

	do
	{
		if (m_FaceArrayIdx < m_FaceArray.GetSize())
		{
			bf = m_FaceArray[m_FaceArrayIdx++];
		}
		else return NULL;
	}
	while (bf->IsRemoved());

 	return bf;
}
//-------------------------------------------------------------------------------------------------
void JC_BucketObject::UnloadFaces(void)
{
	for (int i = 0; i < m_FaceArray.GetSize(); i++)
	{
		delete m_FaceArray.GetAt(i);
	}

	m_FaceArray.RemoveAll();

	m_FacesLoaded = FALSE;
}
//-------------------------------------------------------------------------------------------------
void JC_BucketObject::AddChild(JC_BucketObject * bo)
{
	m_ChildArray.Add(bo);
}
//-------------------------------------------------------------------------------------------------
U32 JC_BucketObject::GetNumChildren(void)
{
	return m_ChildArray.GetSize();
}
//-------------------------------------------------------------------------------------------------
JC_BucketObject * JC_BucketObject::GetParent(void)
{
	return m_Parent;
}
//-------------------------------------------------------------------------------------------------
JC_BucketObject * JC_BucketObject::GetFirstChild(void)
{
	m_ChildArrayIdx = 0;

	return GetNextChild();
}
//-------------------------------------------------------------------------------------------------
JC_BucketObject * JC_BucketObject::GetNextChild(void)
{
	if (m_ChildArrayIdx < 0)
	{
		THROW(new JC_Exception("Called GetNextChild without calling GetFirstChild"));
	}

	JC_BucketObject * bo = NULL;

	do
	{
		if (m_ChildArrayIdx < m_ChildArray.GetSize())
		{
			bo = m_ChildArray[m_ChildArrayIdx++];
		}
		else return NULL;
	}
	while (bo->IsRemoved());

 	return bo;
}
//-------------------------------------------------------------------------------------------------
const char * JC_BucketObject::GetName(void)
{
	return (LPCSTR)Object()->Name();
}
//-------------------------------------------------------------------------------------------------
const char * JC_BucketObject::GetComment(void)
{
	PS_RecordInfo ri;

	api.Database().FirstRecord(ri);					// Ahem :)
	api.Database().GetInfo(Object()->Number(), ri);

	return (LPCSTR)ri.Comment();
}
//-------------------------------------------------------------------------------------------------
JC_Data * JC_BucketObject::Property(const CString& name) const
{
	return Property(name, Object());
}
//-------------------------------------------------------------------------------------------------
JC_Data * JC_BucketObject::Property(const CString& name, const rv_Model * model) const
{
	JC_PropertyList * list = model->CreateFullPropertyList();
	JC_Property * property = list->FindProperty(name);
	JC_Data * data = NULL;
	if (property)
	{
		data = property->CopyValue();
	}
	delete list;
	return data;
}
//-------------------------------------------------------------------------------------------------
BOOL JC_BucketObject::NameMatches(const JC_RegularExpression& re)
{
	return re.CheckMatch(Object()->Name());
}
//-------------------------------------------------------------------------------------------------
void JC_BucketObject::RemoveFaces(char * filter)
{
	if (!m_FacesLoaded)
	{
		THROW(new JC_Exception("Called RemoveFaces without calling LoadFaces"));
	}

	CWaitCursor busy;

	TRACE("Removing faces for '%s'\n", filter);

	for (S32 i = 0; i < m_FaceArray.GetSize(); i++)
	{
		JC_BucketFace * bf = m_FaceArray[i];

		if (!bf->IsRemoved())
		{
			bf->ApplyFilter(filter);
		}
	}
}
//-------------------------------------------------------------------------------------------------
// JC_BucketFace
//-------------------------------------------------------------------------------------------------
JC_BucketFace::JC_BucketFace(JC_Face * face, JC_BucketObject * object)
{
	ASSERT(m_Face != NULL);

	m_Face = face;
	m_PointPos = (POSITION)-1;
	m_FrontTexture = NULL;
	m_BackTexture = NULL;
	m_Group = NULL;
	m_Object = object;
}
//-------------------------------------------------------------------------------------------------
JC_BucketFace::~JC_BucketFace()
{
	ASSERT(m_Face != NULL);

//	delete m_Face;
	delete m_FrontTexture;
	delete m_BackTexture;
}
//-------------------------------------------------------------------------------------------------
void JC_BucketFace::SetTextureSetGroup(const char * path)
{
	m_Group = GetTextureSetGroup(path);
}
//-------------------------------------------------------------------------------------------------
PS_TextureSetFolder * JC_BucketFace::GetTextureSetGroup(const char * path)
{
	if (!path && m_Group)
	{
		return m_Group;
	}
	const PS_Texture * texture = m_Face->FrontTexture();
	if (!texture)
	{
		texture = m_Face->BackTexture();
	}
	if (path && texture)
	{
		U32 rec = texture->Database()->FindRecord(path, JC_Record::TypeTextureSetFolder);
		PS_TextureSetFolder * group = (PS_TextureSetFolder*)texture->Database()->Get(rec);
		return group;
	}
	return NULL;
}
//-------------------------------------------------------------------------------------------------
PS_ExportTexture * JC_BucketFace::GetFrontTexture(const PS_TextureSetFolder * group)
{
	if (m_FrontTexture == NULL)
	{
		const PS_Texture * texture = m_Face->FrontTexture();
		if (texture)
		{
//			DT_TextureSet * texture_set = NULL;
			if (!group)
			{
				group = api.TextureSetGroup();
			}
//			if (group)
//			{
//				texture_set = group->TextureSetContaining(texture);
//			}
			JC_PointList * point_list = const_cast <JC_PointList*>(m_Face->PointList());
			const U16 number_of_vertices = (U16)point_list->GetCount();
			m_FrontTexture = new PS_ExportTexture(&(m_Face->FrontOrient()), texture, group, number_of_vertices);
		}
	}
	return m_FrontTexture;
}
//-------------------------------------------------------------------------------------------------
PS_ExportTexture * JC_BucketFace::GetFrontTexture(const char * path)
{
	return GetFrontTexture(GetTextureSetGroup(path));
}
//-------------------------------------------------------------------------------------------------
PS_ExportTexture * JC_BucketFace::GetBackTexture(const PS_TextureSetFolder * group)
{
	if (m_BackTexture == NULL)
	{
		const PS_Texture * texture = m_Face->BackTexture();
		if (texture)
		{
			if (!group)
			{
				group = api.TextureSetGroup();
			}
			JC_PointList * point_list = const_cast <JC_PointList*>(m_Face->PointList());
			const U16 number_of_vertices = (U16)point_list->GetCount();
			m_BackTexture = new PS_ExportTexture(&(m_Face->BackOrient()), texture, group, number_of_vertices);
		}
	}
	return m_BackTexture;
}
//-------------------------------------------------------------------------------------------------
PS_ExportTexture * JC_BucketFace::GetBackTexture(const char * path)
{
	return GetBackTexture(GetTextureSetGroup(path));
}
//-------------------------------------------------------------------------------------------------
PS_ExportDialog * JC_BucketFace::GetDialogData(const CString& name)
{
	PS_ExportDialog * result = NULL;
	JC_DialogData * dialog_data = NULL;

	JC_Data * data = Property(name);
	if (data && data->Token() == JC_Data::TokDialogData)
	{
		dialog_data = (JC_DialogData*)data;
		if (dialog_data)
		{
			result = new PS_ExportDialog(dialog_data);
			m_Object->AddToDialogArray(result);
		}
	}
	return result;
}
//-------------------------------------------------------------------------------------------------
U32 JC_BucketFace::GetPointCount(void)
{
	return m_Face->PointList()->GetCount();
}
//-------------------------------------------------------------------------------------------------
QPOINT JC_BucketFace::GetFirstPoint(void)
{
	m_PointPos = m_Face->PointList()->GetHeadPosition();

	return GetNextPoint();
}
//-------------------------------------------------------------------------------------------------
QPOINT JC_BucketFace::GetNextPoint(void)
{
	QPOINT point;
	if (m_PointPos == (POSITION)-1)
	{
		THROW(new JC_Exception("Called GetNextPoint without calling GetFirstPoint"));
	}

	if (m_PointPos != NULL)
	{
		JC_Point * list_pt = (JC_Point*)m_Face->PointList()->GetNext(m_PointPos);

		QCOLOUR& col = point.Colour;
		list_pt->Colour().Get(col.R, col.G, col.B, col.A);

		QVERTEX vert;
		vert.X = list_pt->Vertex()->X();
		vert.Y = list_pt->Vertex()->Y();
		vert.Z = list_pt->Vertex()->Z();
		point.Vertex = m_Object->ApplyTransforms(vert);
	}
	
	return point;
}
//-------------------------------------------------------------------------------------------------
BOOL JC_BucketFace::NameMatches(const JC_RegularExpression&)
{
	THROW(new JC_Exception("You cannot filter a face by name"));

	return FALSE;
}
//-------------------------------------------------------------------------------------------------
JC_Data * JC_BucketFace::Property(const CString& name) const
{
	JC_Data * data = NULL;
	JC_Property * property = m_Face->PropertyList()->FindProperty(name);
	if (property)
	{
		data = property->CopyValue();
	}
	return data;						///////////////////////////////////////
}
//-------------------------------------------------------------------------------------------------
// PS_ExportMorphFace: overrides JC_BucketFace to add in the morphs offset position
//-------------------------------------------------------------------------------------------------
PS_ExportMorphFace::PS_ExportMorphFace(JC_Face * face, JC_BucketObject * object, QVERTEX offset):
JC_BucketFace(face, object)
{
	m_MorphOffset = offset;
}
//-------------------------------------------------------------------------------------------------
QPOINT PS_ExportMorphFace::GetNextPoint(void)
{
	QPOINT point = JC_BucketFace::GetNextPoint();
	point.Vertex.X += m_MorphOffset.X;
	point.Vertex.Y += m_MorphOffset.Y;
	point.Vertex.Z += m_MorphOffset.Z;
	return point;
}
//-------------------------------------------------------------------------------------------------
// JC_BucketItem
//-------------------------------------------------------------------------------------------------
JC_BucketItem::JC_BucketItem()
{
	m_Removed = FALSE;
}
//-------------------------------------------------------------------------------------------------
JC_String JC_BucketItem::GetNameString(const CString& s)
{
	CString ns = s;
	ns.TrimLeft();

	if (ns[0] == '[')
	{
		ns = ns.Mid(1);
		ns = ns.SpanExcluding("]");
	}

	ns = ns.SpanExcluding("=");
	ns.TrimRight();

	return (JC_String)ns;
}
//-------------------------------------------------------------------------------------------------
void JC_BucketItem::ApplyFilter(char * filter)
{
	CString err;
	JC_String ns;
	JC_String s(filter);		// Ensure string is standardised

	if (s.Left(4) == "Name")
	{
		ns = s.Mid(4);
		ns.TrimLeft();

		if (ns[0] == '=')
		{
			ns = GetNameString(ns.Mid(1));

			if (NameMatches(JC_RegularExpression(ns)))
			{
				m_Removed = TRUE;
			}
		}
		else
		{
			err.Format("Syntax error, missing = in '%s'", (LPCSTR)ns);
			api.Error((LPCSTR)err);
		}
	}
	else
	{
		err.Format("Error in '%s' - You can only filter by 'Name'", filter);
		api.Error((LPCSTR)err);
	}
}
//-------------------------------------------------------------------------------------------------
// PS_ExportTexture
//-------------------------------------------------------------------------------------------------
PS_ExportTexture::PS_ExportTexture(BA_VertexOrient * orientation, const PS_Texture * texture, const PS_TextureSetFolder * group, const U16 number_of_vertices)
{
	m_pTexture = texture;
	m_Palette = NULL;
	m_BitmapInfo = NULL;
	m_Iterator = 0;
	m_pTextureSetGroup = group;
	m_UVArray = NULL;
	m_TextureSet = NULL;
	if (IsAnimation())
	{
		const PS_TextureAnimation * t = static_cast < const PS_TextureAnimation* > (m_pTexture);
		U32 size = t->GetNumberOfCels();
		for (U32 i = 0; i < size; i++)
		{
			const PS_ExportTexture * e = new PS_ExportTexture(orientation, t->GetCel(i)->GetTexture(), group, number_of_vertices);
			m_CelList.AddTail(e);
		}
	}
	else
	{
		JC_TextureSet * texture_set = NULL;

		if (group)
		{
			texture_set = group->TextureSetContaining(texture);
		}
		PS_FloatUVArray array;
		if (texture_set)
		{
			// Fill using texture set

			texture_set->GetFloatUVArray(texture->Number(), array);
			m_TextureSet = texture_set;
		}
		else
		{
			// Fill using source bitmap
			PS_Polygon textureshape;
			*m_pTexture >> textureshape;
			PS_FloatUV uv;
			int count = textureshape.GetNumberOfVertices();
			for (int k = 0; k < count; k++)
			{
				textureshape >> uv;
				array.Add(uv);
			}
		}
		int count = array.GetSize();
		m_UVArray = new QUV[number_of_vertices];
		U16 i = orientation->FirstFaceVertex;
		U16 j = orientation->FirstTextureVertex;
		U16 inc = static_cast <U16>((orientation->Reversed)?number_of_vertices-1:1);
		U16 num = static_cast <U16>(number_of_vertices);
		while (num--)
		{
			if (count > 0)
			{
				m_UVArray[i % number_of_vertices].U = array[j % count].u;
				m_UVArray[i % number_of_vertices].V = array[j % count].v;
			}
			else
			{
				m_UVArray[i % number_of_vertices].U = 0.0;
				m_UVArray[i % number_of_vertices].V = 0.0;
			}
			j++;
			i = static_cast <U16>(i+inc);
		}
		m_Count = count;
	}
}
//-------------------------------------------------------------------------------------------------
PS_ExportTexture::~PS_ExportTexture(void)
{
	delete [] m_UVArray;
	delete [] m_Palette;
	U32 size = m_CelList.GetCount();
	POSITION pos = m_CelList.GetHeadPosition();
	while (pos)
	{
		PS_ExportTexture * t = const_cast < PS_ExportTexture* > (m_CelList.GetNext(pos));
		delete t;
	}
}
//-------------------------------------------------------------------------------------------------
U32 PS_ExportTexture::GetPaletteID(void)
{
	U32 palette_recnum = PS_Database::NullRecord;
	if (m_TextureSet)
	{
		palette_recnum = m_TextureSet->GetTextureGroupIDFromTexture(m_pTexture->Number());
	}
	else
	{
		if (HasOwnPalette())
		{
			palette_recnum = m_pTexture->Palette()->RecordNum();
		}
		else
		{
			const JC_Bitmap * bitmap = m_pTexture->Bitmap();
			if (bitmap)
			{
				palette_recnum = bitmap->GetPaletteID();
			}
		}
	}
	return palette_recnum;
}
//-------------------------------------------------------------------------------------------------
QPALETTE * PS_ExportTexture::GetPalette(void)
{
	if (m_Palette == NULL)
	{
		if (m_TextureSet)
		{
			CDWordArray palette;
			if (m_TextureSet->GetPalette(m_pTexture->Number(), palette))
			{
				int size = palette.GetSize();
				m_Palette = new BYTE[sizeof(U16) + sizeof(QCOLOUR) * size];
				QPALETTE * new_palette = (QPALETTE*)(m_Palette);
				for (U16 i = 0; i < size; i++)
				{
					JC_Pixel colour(palette[i]);
					new_palette->Entry[i].R = colour.R();
					new_palette->Entry[i].G = colour.G();
					new_palette->Entry[i].B = colour.B();
					new_palette->Entry[i].A = 0;
				}
				new_palette->Size = (U16)size;
			}
		}
		else
		{
			U32 palette_recnum = GetPaletteID();
			if (palette_recnum != PS_Database::NullRecord)
			{
				PS_Database * db = m_pTexture->Database();
				JC_Palette * palette = static_cast <JC_Palette*> (db->Get(palette_recnum, JC_Record::TypePalette));
				if (palette != NULL)
				{
					U16 size = static_cast <U16>(palette->NumColours());
					m_Palette = new BYTE[sizeof(U16) + sizeof(QCOLOUR) * size];
					QPALETTE * new_palette = (QPALETTE*) (m_Palette);
					for (U16 i = 0; i < size; i++)
					{
						U8& red	  = new_palette->Entry[i].R;
						U8& green = new_palette->Entry[i].G;
						U8& blue  = new_palette->Entry[i].B;
						U8& alpha = new_palette->Entry[i].A;
						palette->Colour(i).Get(red, green, blue, alpha);
						alpha = 0;
					}
					new_palette->Size = size;
				}
			}
		}
	}
	return (QPALETTE*) m_Palette;
}
//-------------------------------------------------------------------------------------------------
QUV * PS_ExportTexture::GetFirstUV(void)
{
	m_Iterator = 0;
	return GetNextUV();
}
//-------------------------------------------------------------------------------------------------
QUV * PS_ExportTexture::GetNextUV(void)
{
	if (IsAnimation())
	{
		THROW(new JC_Exception("Trying to get UV coords of an animating texture"));
	}
	ASSERT(m_Iterator <= m_Count); // m_Iterator should not be allow to get
								   // get bigger than m_Count; this is an internal
								   // error.
	QUV * rval = NULL;
	if (m_Iterator != m_Count)
	{
		rval = m_UVArray + m_Iterator;
		m_Iterator++;
	}
	return rval;
}
//-------------------------------------------------------------------------------------------------
U32 PS_ExportTexture::GetNumberOfCels(void) const
{
	if (IsAnimation())
	{
		return m_CelList.GetCount();
	}
	else
	{
		return 0;
	}
}
//-------------------------------------------------------------------------------------------------
PS_ExportTexture * PS_ExportTexture::GetCel(U32 index) const
{
	return const_cast <PS_ExportTexture*> (m_CelList.GetAt(m_CelList.FindIndex(index)));
}
//-------------------------------------------------------------------------------------------------
U32 PS_ExportTexture::GetCelDuration(U32 index) const
{
	const PS_TextureAnimation * t = static_cast < const PS_TextureAnimation* > (m_pTexture);
	PS_TextureAnimationCel * cel = t->GetCel(index);
	return cel->GetCycles();
}
//-------------------------------------------------------------------------------------------------
U32 PS_ExportTexture::GetNumberOfRepeats(void) const
{
	const PS_TextureAnimation * t = static_cast < const PS_TextureAnimation* > (m_pTexture);
	return t->Repeats();
}
//-------------------------------------------------------------------------------------------------
U32 PS_ExportTexture::GetRepeatMode(void) const
{
	const PS_TextureAnimation * t = static_cast < const PS_TextureAnimation* > (m_pTexture);
	return t->RepeatMode();
}
//-------------------------------------------------------------------------------------------------
U32 PS_ExportTexture::GetBitmapDataSize(void)
{
	GetBitmapData();
	U32 size = m_BitmapInfo->bmiHeader.biSizeImage + m_BitmapInfo->bmiHeader.biSize;
	U32 num;
	switch (m_BitmapInfo->bmiHeader.biBitCount)
	{
		case 4:	num = 16;		break;
		case 8:	num = 256;		break;
		case 16: num = 0;       break;
		case 24: num = 0;		break;
		case 32: num = 0;		break;
		default: THROW(new JC_Exception("PS_ExportTexture:: - Bad biBitCount"));
	}
	if (m_BitmapInfo->bmiHeader.biClrUsed)
	{
		num = m_BitmapInfo->bmiHeader.biClrUsed;
	}
	size += num * sizeof(RGBQUAD);
	return size;
}
//-------------------------------------------------------------------------------------------------
const char * PS_ExportTexture::GetBitmapName(void)
{
	if (IsAnimation())
	{
		THROW(new JC_Exception("Trying to get bitmap name of an animating texture"));
	}
	JC_Bitmap * bitmap = Bitmap();
	if (bitmap)
	{
		return bitmap->Name();
	}
	else
	{
		return m_NotFound;
	}
}
//-------------------------------------------------------------------------------------------------
JC_Bitmap * PS_ExportTexture::Bitmap(void)
{
	if (IsAnimation())
	{
		THROW(new JC_Exception("Trying to get bitmap from an animating texture"));
	}
	JC_Bitmap * bitmap = NULL;
	JC_TextureSet * texture_set = NULL;
	if (m_pTextureSetGroup)
	{
		texture_set = m_pTextureSetGroup->TextureSetContaining(m_pTexture);
	}

	if (texture_set)
	{
		U32 record = texture_set->GetOutputBitmap();
		if (record)
		{
			bitmap = (JC_Bitmap*)texture_set->Database()->Get(record, JC_Record::TypeBitmap);
		}
		else
		{
			THROW(new JC_Exception("Texture set could not create the output bitmap"));
		}
	}
	else if (m_pTexture)
	{
		bitmap = (JC_Bitmap*)m_pTexture->Bitmap();
	}
	return bitmap;
}
//-------------------------------------------------------------------------------------------------
BITMAPINFO * PS_ExportTexture::GetBitmapData(void)
{
	if (IsAnimation())
	{
		THROW(new JC_Exception("Trying to get bitmap data from an animating texture"));
	}
	if (!m_BitmapInfo)
	{
		m_BitmapInfo = Bitmap()->CreateBitmapInfo();
	}
	return m_BitmapInfo;
}
//-------------------------------------------------------------------------------------------------
QVERTEX JC_BucketObject::ApplyTransforms(const QVERTEX point) const
{
	rv_Model * model = Object();
	QVERTEX new_point = point;
	if (model)
	{
		new_point = ApplyTransforms(new_point, model);
		if (model->Type() == JC_Record::TypeObject || 
			model->Type() == JC_Record::TypeAnimatedObject)
		{
			rv_Model * tmpl = model->TemplatePtr();
			if (tmpl != model)
			{
				new_point = ApplyTransforms(new_point, tmpl);
			}
		}

		if (m_Parent != NULL)
		{
			// apply pin offset
			const JC_Anchor& anchor = model->RealAnchor();
			const JC_Vector& v = anchor.PinVector();
			new_point.X += v.X();
			new_point.Y += v.Y();
			new_point.Z += v.Z();

			new_point = m_Parent->ApplyTransforms(new_point);
		}
	}
	return new_point;
}
//-------------------------------------------------------------------------------------------------
QVERTEX JC_BucketObject::ApplyRotations(const QVERTEX vector) const
{
	rv_Model * model = Object();
	QVERTEX new_vector = vector;
	if (model)
	{
		new_vector = ApplyRotation(new_vector, model);
		if (model->Type() == JC_Record::TypeObject || 
			model->Type() == JC_Record::TypeAnimatedObject)
		{
			rv_Model * tmpl = model->TemplatePtr();
			if (tmpl)
			{
				new_vector = ApplyRotation(vector, tmpl);
			}
		}
		if (m_Parent != NULL)
		{
			new_vector = m_Parent->ApplyRotations(new_vector);
		}
	}
	return new_vector;
}
//-------------------------------------------------------------------------------------------------
QVERTEX JC_BucketObject::ApplyRotation(const QVERTEX vector, rv_Model * model) const
{
	const JC_Anchor& anchor = model->RealAnchor();
	// apply rotation
	rv_Matrix4x4 coord_shift;
	JC_Vector x_axis, y_axis, z_axis;
	z_axis = anchor.VectorFwd();
	y_axis = anchor.VectorUp();
	x_axis = y_axis.CrossProduct(z_axis);
	y_axis = z_axis.CrossProduct(x_axis); //. Just in case
	x_axis.Normalise();
	y_axis.Normalise();
	z_axis.Normalise();

	D3DVECTOR vx, vy, vz;
	x_axis.GetD3DVector(vx);
	y_axis.GetD3DVector(vy);
	z_axis.GetD3DVector(vz);

	//. Shift the rotation details into the Anchor's coordinate system
	coord_shift.BuildFromAxisVectors(&vx, &vy, &vz);
	//				coord_shift.Transpose();
	D3DVECTOR rot_point;
	rot_point.x = (float)vector.X;
	rot_point.y = (float)vector.Y;
	rot_point.z = (float)vector.Z;
	coord_shift.ApplyTo(&rot_point);
	QVERTEX adj_point;
	adj_point.X = rot_point.x;
	adj_point.Y = rot_point.y;
	adj_point.Z = rot_point.z;
	return adj_point;
}
//-------------------------------------------------------------------------------------------------
QVERTEX JC_BucketObject::ApplyScales(const QVERTEX vector) const
{
	rv_Model * model = Object();
	QVERTEX new_vector = vector;
	if (model)
	{
		new_vector = ApplyScale(new_vector, model);
		if (model->Type() == JC_Record::TypeObject || 
			model->Type() == JC_Record::TypeAnimatedObject)
		{
			rv_Model * tmpl = model->TemplatePtr();
			if (tmpl)
			{
				new_vector = ApplyScale(new_vector, tmpl);
			}
		}
		if (m_Parent != NULL)
		{
			new_vector = m_Parent->ApplyScales(new_vector);
		}
	}
	return new_vector;
}
//-------------------------------------------------------------------------------------------------
QVERTEX JC_BucketObject::ApplyScale(const QVERTEX vector, rv_Model * model) const
{
	QVERTEX adj_point = vector;
	const JC_Vector& scale = model->Scale();
	adj_point.X *= scale.X();
	adj_point.Y *= scale.Y();
	adj_point.Z *= scale.Z();
	return adj_point;
}
//-------------------------------------------------------------------------------------------------
QVERTEX JC_BucketObject::ApplyTransforms(const QVERTEX point, rv_Model * model) const
{
	QVERTEX adj_point = point;
	const JC_Anchor& anchor = model->RealAnchor();

	// apply scale
	adj_point = ApplyScale(adj_point, model);

	// apply rotation
	adj_point = ApplyRotation(adj_point, model);

	// apply anchor offset
	JC_Vector vector = anchor.AnchorPos();
	adj_point.X += vector.X();
	adj_point.Y += vector.Y();
	adj_point.Z += vector.Z();
	
	return adj_point;
}
//-------------------------------------------------------------------------------------------------
PS_ExportDialog * JC_BucketObject::GetDialogData(const CString& name, const GetDataType mode)
{
	PS_ExportDialog * result = NULL;
	JC_DialogData * dialog_data = NULL;

	JC_Data * data = Property(name);
	if (data && data->Token() == JC_Data::TokDialogData)
	{
		dialog_data = (JC_DialogData*)data;
		if (dialog_data && (Object()->IsTemplate() || mode == DIALOG_GET_MODIFIED_DATA))
		{
			result = new PS_ExportDialog(dialog_data);
			m_DialogArray.Add(result);
		}
		else if (dialog_data && mode == DIALOG_GET_ALL_DATA)
		{
			JC_DialogData * back_dialog_data = NULL;
			rv_Model * model = Object()->TemplatePtr();
			ASSERT(model);
			JC_Data * data = Property(name, model);
			if (data && data->Token() == JC_Data::TokDialogData)
			{
				back_dialog_data = (JC_DialogData*)data;
			}
			model->Release();
			result = new PS_ExportDialog(dialog_data, back_dialog_data);
			m_DialogArray.Add(result);
		}
	}
	return result;
}
//-------------------------------------------------------------------------------------------------
QVERTEX JC_BucketObject::GetDirWorld(void) const
{
	JC_Vector vector = Object()->RealAnchor().VectorFwd();
	vector.Normalise();
	QVERTEX v;

	v.X = vector.X();
	v.Y = vector.Y();
	v.Z = vector.Z();

	return v;
}
//-------------------------------------------------------------------------------------------------
QVERTEX JC_BucketObject::GetUpWorld(void) const
{
	JC_Vector vector = Object()->RealAnchor().VectorUp();
	vector.Normalise();
	QVERTEX v;

	v.X = vector.X();
	v.Y = vector.Y();
	v.Z = vector.Z();

	return v;
}
//-------------------------------------------------------------------------------------------------
PS_ExportDialog::PS_ExportDialog(JC_DialogData * dialog_data, JC_DialogData * back_data)
{
	m_DialogData = dialog_data;
	m_BackDialogData = back_data;
}
//-------------------------------------------------------------------------------------------------
PS_ExportDialog::~PS_ExportDialog(void)
{
	S32 i = m_ArrayArray.GetSize();
	while (i--)
	{
		delete m_ArrayArray[i];
	}
}
//-------------------------------------------------------------------------------------------------
JC_DataConcrete * PS_ExportDialog::ExtractData(const CString& name)
{
	JC_DataConcrete * result = ExtractData(name, m_DialogData);
	if (result == NULL)
	{
		result = ExtractData(name, m_BackDialogData);
	}
	return result;
}
//-------------------------------------------------------------------------------------------------
JC_DataConcrete * PS_ExportDialog::ExtractData(const CString& name, JC_DialogData * dialog_data)
{
	JC_DataConcrete * result = NULL;
	JC_DialogTemplateRecord * dialog_record = NULL;

	if (dialog_data)
	{
		U32 dialog_record_number = dialog_data->GetRecordNum();
		dialog_record = (JC_DialogTemplateRecord*)api.Database().Get(dialog_record_number, JC_Record::TypeDialogTemplateRecord);
		if (dialog_record)
		{
			JC_DialogTemplate * dialog_template = dialog_record->GetDialogTemplate();
			if (dialog_template)
			{
				CString data_id = dialog_template->GetDataIdFromName(name);
				if (data_id.IsEmpty() == FALSE)
				{
					result = (JC_DataConcrete*)dialog_data->GetData(data_id);
				}
			}
		}
	}
	return result;
}
//-------------------------------------------------------------------------------------------------
PS_ExportArray * PS_ExportDialog::ExtractArray(const CString& name)
{
	JC_Data * data = NULL;
	CString data_id;
	JC_DialogTemplateRecord * dialog_record = NULL;
	JC_DialogTemplate * dialog_template = NULL;
	if (m_DialogData)
	{
		U32 dialog_record_number = m_DialogData->GetRecordNum();
		dialog_record = (JC_DialogTemplateRecord*)api.Database().Get(dialog_record_number, JC_Record::TypeDialogTemplateRecord);
		if (dialog_record)
		{
			dialog_template = dialog_record->GetDialogTemplate();
			if (dialog_template)
			{
				data_id = dialog_template->GetDataIdFromName(name);
				if (data_id.IsEmpty() == FALSE)
				{
					data = m_DialogData->GetData(data_id);
					if (data && data->Token() != JC_Data::TokNamedDataArray)
					{
						data = NULL;
					}
					JC_NamedDataArray * nda = static_cast <JC_NamedDataArray*>(data);
					if (m_BackDialogData)
					{
						data = m_BackDialogData->GetData(data_id);
						if (data && data->Token() != JC_Data::TokNamedDataArray)
						{
							data = NULL;
						}
					}
					else
					{
						data = NULL;
					}
					JC_NamedDataArray * back_nda = static_cast <JC_NamedDataArray*>(data);
					JC_DialogItem * array_item = dialog_template->GetItemFromId(data_id);
					if (array_item != NULL)
					{
						if (!array_item->IsSpreadsheet())
						{
							THROW(new JC_Exception("ExtractArray: item is not an array"));
						}
					}
					JC_DialogSpreadsheet * spread_sheet = static_cast <JC_DialogSpreadsheet*>(array_item);
					PS_ExportArray * array = new PS_ExportArray(nda, back_nda, spread_sheet);
					m_ArrayArray.Add(array);
					return array;
				}
			}
		}
	}
	return NULL;
}
//-------------------------------------------------------------------------------------------------
PS_ExportLink::PS_ExportLink(const AD_Link * link, const JC_BucketObject * root)
{
	ASSERT(link != NULL && root != NULL);
	m_Link = link;
	m_Link->AddRef();
	m_RootBucketObject = root;
}
//-------------------------------------------------------------------------------------------------
PS_ExportLink::~PS_ExportLink(void)
{
	m_Link->Release();
}
//-------------------------------------------------------------------------------------------------
U32 PS_ExportLink::GetLinkID(void) const
{
	return m_Link->Number();
}
//-------------------------------------------------------------------------------------------------
BOOL PS_ExportLink::IsBidirectional(void) const
{
	return (m_Link->GetLinkDirection() == AD_Link::BiDirectional);
}
//-------------------------------------------------------------------------------------------------
JC_BucketObject * PS_ExportLink::GetSourceObject(void) const
{
	U32 record_number = PS_Database::NullRecord;
	if (m_Link->GetLinkDirection() == AD_Link::StartEnd)
	{
		record_number = m_Link->GetStartObjectRecordNumber();
	}
	else
	{
		record_number = m_Link->GetEndObjectRecordNumber();
	}
	return GetObject(record_number);
}
//-------------------------------------------------------------------------------------------------
JC_BucketObject * PS_ExportLink::GetDestinationObject(void) const
{
	U32 record_number = PS_Database::NullRecord;
	if (m_Link->GetLinkDirection() != AD_Link::StartEnd)
	{
		record_number = m_Link->GetStartObjectRecordNumber();
	}
	else
	{
		record_number = m_Link->GetEndObjectRecordNumber();
	}
	return GetObject(record_number);
}
//-------------------------------------------------------------------------------------------------
JC_BucketObject * PS_ExportLink::GetObject(U32 record_number) const
{
	JC_BucketObject * ret_object = m_RootBucketObject->FindObject(record_number);
	if (ret_object == NULL)
	{
		CString message;
		message.Format("LinkID %d could not find ObjectID %d in bucket", m_Link->Number(), record_number);
		THROW(new JC_Exception(message));
	}
	return ret_object;
}
//-------------------------------------------------------------------------------------------------
JC_Data * PS_ExportLink::Property(const CString& name) const
{
	AD_Link * mutable_link = const_cast <AD_Link*>(m_Link);
	JC_PropertyList * list = mutable_link->PropertyList();
	JC_Property * property = list->FindProperty(name);
	JC_Data * data = NULL;
	if (property)
	{
		data = property->CopyValue();
	}
	return data;
}
//-------------------------------------------------------------------------------------------------
U32 JC_BucketObject::GetLinkCount(void) const
{
	U32 count = 0;
	JC_RecordRefList * list = Object()->GetLinkRecordRefList();
	if (list != NULL)
	{
		count = list->GetCount();
	}
	return count;
}
//-------------------------------------------------------------------------------------------------
PS_ExportLink * JC_BucketObject::GetFirstLink(void) const
{
	JC_BucketObject * mutable_this = const_cast <JC_BucketObject *>(this);
	mutable_this->m_LinkIdx = 0;
	return GetNextLink();
}
//-------------------------------------------------------------------------------------------------
PS_ExportLink * JC_BucketObject::GetNextLink(void) const
{
	PS_ExportLink * ret_val = NULL;
	if (m_LinkIdx < 0)
	{
		THROW(new JC_Exception("Called GetNextLink without calling GetFirstLink"));
	}
	if (m_LinkIdx < (S32)GetLinkCount())
	{
		JC_RecordRefList * list = Object()->GetLinkRecordRefList();
		JC_RecordRef * ref = (JC_RecordRef*)list->GetAt(list->FindIndex(m_LinkIdx));
		if (ref != NULL)
		{
			AD_Link * link = (AD_Link*)ref->Get(JC_Record::TypeLink);
			ret_val = new PS_ExportLink(link, GetRootBucketObject());
			api.AddLink(ret_val);
			JC_BucketObject * mutable_this = const_cast <JC_BucketObject *>(this);
			mutable_this->m_LinkIdx++;
		}
	}
	return ret_val;
}
//-------------------------------------------------------------------------------------------------
U32 JC_BucketObject::GetObjectID(void) const
{
	return m_RecordNum;
}
//-------------------------------------------------------------------------------------------------
JC_BucketObject * JC_BucketObject::FindObject(U32 record_number) const
{
	JC_BucketObject * found_object = NULL;
	if (record_number == GetObjectID())
	{
		found_object = const_cast <JC_BucketObject*>(this);
	}
	else
	{
		for (int i = 0; i < m_ChildArray.GetSize() && found_object == NULL; i++)
		{
			JC_BucketObject * child = m_ChildArray[i];
			found_object = child->FindObject(record_number);
		}
	}
	return found_object;
}
//-------------------------------------------------------------------------------------------------
JC_BucketObject * JC_BucketObject::GetRootBucketObject(void) const
{
	JC_BucketObject * root_object = const_cast <JC_BucketObject*>(this);
	while (root_object->GetParent() != NULL)
	{
		root_object = root_object->GetParent();
	}
	return root_object;
}
//-------------------------------------------------------------------------------------------------
BOOL JC_BucketObject::IsAnimatedObject(void) const
{
	if (Object()->Type() == JC_Record::TypeAnimatedObject || Object()->Type() == JC_Record::TypeAnimatedTemplate)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}
//-------------------------------------------------------------------------------------------------
U32 JC_BucketObject::GetNumberOfTransforms(void) const
{
	U32 transforms = 0;
	if (IsAnimatedObject())
	{
		transforms = m_TransformArray.GetSize();
	}
	else
	{
		THROW(new JC_Exception("Trying to call GetNumberOfTransforms on an object that is not animated"));
	}
	return transforms;
}
//-------------------------------------------------------------------------------------------------
PS_ExportTransform * JC_BucketObject::GetFirstTransform(void) const
{
	PS_ExportTransform * transform = NULL;
	if (IsAnimatedObject())
	{
		JC_BucketObject * mutable_this = const_cast <JC_BucketObject*>(this);
		mutable_this->m_TransformArrayIdx = 0;
		transform = GetNextTransform();
	}
	else
	{
		THROW(new JC_Exception("Trying to call GetFirstTransform on an object that is not animated"));
	}
	return transform;
}
//-------------------------------------------------------------------------------------------------
PS_ExportTransform * JC_BucketObject::GetNextTransform(void) const
{
	PS_ExportTransform * transform = NULL;
	if (IsAnimatedObject())
	{
		if (m_TransformArrayIdx < 0)
		{
			THROW(new JC_Exception("Call to GetNextTransform before calling GetFirstTransform"));
		}
		else if (m_TransformArrayIdx < (S32)GetNumberOfTransforms())
		{
			transform = m_TransformArray[m_TransformArrayIdx];
			JC_BucketObject * mutable_this = const_cast <JC_BucketObject*>(this);
			mutable_this->m_TransformArrayIdx++;
		}
	}
	else
	{
		THROW(new JC_Exception("Trying to call GetNextTransform on an object that is not animated"));
	}
	return transform;
}
//-------------------------------------------------------------------------------------------------
PS_ExportTransform::PS_ExportTransform(const rv_ObjTransBase * transform, JC_BucketObject * object)
{
	m_Object = object;
	m_FaceArrayIdx = -1;
	m_Transform = transform;
	if (Type() == MORPH)
	{
		LoadFaces();
	}
}
//-------------------------------------------------------------------------------------------------
CString PS_ExportTransform::Name(void) const
{
	return m_Transform->Name();
}
//-------------------------------------------------------------------------------------------------
float PS_ExportTransform::Duration(void) const
{
	return m_Transform->Duration();
}
//-------------------------------------------------------------------------------------------------
float PS_ExportTransform::StartTime(void) const
{
	return m_Transform->StartTime();
}
//-------------------------------------------------------------------------------------------------
TRANSFORM PS_ExportTransform::Type(void) const
{
	TRANSFORM return_type;
	int type = (int)m_Transform->ListElementType();
	switch (type)
	{
		case rv_ObjTransBase::OAT_ROTATE:	 return_type = ROTATION;    break;
		case rv_ObjTransBase::OAT_MORPH:	 return_type = MORPH;	    break;
		case rv_ObjTransBase::OAT_TRANSLATE: return_type = TRANSLATION; break;
		case rv_ObjTransBase::OAT_TRIGGER:	 return_type = PROPERTY;    break;
		default:							 return_type = UNKNOWN;     break;
	}
	return return_type;
}
//-------------------------------------------------------------------------------------------------
LPCSTR PS_ExportTransform::GetPropertyName(void) const
{
	if (Type() != PROPERTY)
	{
		THROW(new JC_Exception("GetPropertyName not available on this transform"));
	}

	rv_ObjTransBase * trans = const_cast <rv_ObjTransBase*>(m_Transform);
	rv_ObjTransTrigger * trigger = static_cast <rv_ObjTransTrigger*>(trans);
	JC_Property * property = trigger->Property();
	if (property)
	{
		return property->Name();
	}
	else
	{
		return NULL;
	}
}
//-------------------------------------------------------------------------------------------------
JC_Data * PS_ExportTransform::GetPropertyValue(void) const
{
	if (Type() != PROPERTY)
	{
		THROW(new JC_Exception("GetPropertyValue not available on this transform"));
	}

	rv_ObjTransBase * trans = const_cast <rv_ObjTransBase*>(m_Transform);
	rv_ObjTransTrigger * trigger = static_cast <rv_ObjTransTrigger*>(trans);
	JC_Property * property = trigger->Property();
	if (property)
	{
		return property->CopyValue();
	}
	else
	{
		return NULL;
	}
}
//-------------------------------------------------------------------------------------------------
BOOL PS_ExportTransform::ApplyLocally(void) const
{
	BOOL answer;
	if (Type() == ROTATION)
	{
		rv_ObjTransBase * trans = const_cast <rv_ObjTransBase*>(m_Transform);
		rv_ObjTransRotate * rotate = static_cast <rv_ObjTransRotate*>(trans);
		answer = rotate->ApplyLocally();
	}
	else if (Type() == TRANSLATION)
	{
		rv_ObjTransBase * trans = const_cast <rv_ObjTransBase*>(m_Transform);
		rv_ObjTransTranslate * translate = static_cast <rv_ObjTransTranslate*>(trans);
		answer = translate->ApplyLocally();
	}
	else
	{
		THROW(new JC_Exception("ApplyLocally not available on this transform"));
	}
	return answer;
}
//-------------------------------------------------------------------------------------------------
float PS_ExportTransform::RotationAngle(void) const
{
	float angle;
	if (Type() == ROTATION)
	{
		rv_ObjTransBase * trans = const_cast <rv_ObjTransBase*>(m_Transform);
		rv_ObjTransRotate * rotate = static_cast <rv_ObjTransRotate*>(trans);
		angle = rotate->RotationValue();
	}
	else
	{
		THROW(new JC_Exception("RotationAngle not available on this transform"));
	}
	return angle;
}
//-------------------------------------------------------------------------------------------------
QVERTEX PS_ExportTransform::RotationAxis(void) const
{
	QVERTEX answer;
	if (Type() == ROTATION)
	{
		rv_ObjTransBase * trans = const_cast <rv_ObjTransBase*>(m_Transform);
		rv_ObjTransRotate * rotate = static_cast <rv_ObjTransRotate*>(trans);
		D3DVECTOR& vector = rotate->RotationAxis();
		answer.X = vector.x;
		answer.Y = vector.y;
		answer.Z = vector.z;
		if (ApplyLocally())
		{
			answer = m_Object->ApplyRotations(answer);
		}
		else
		{
			if (m_Object->GetParent() != NULL)
			{
				answer = m_Object->GetParent()->ApplyRotations(answer);
			}
		}
		// just in case
		JC_Vector n_answer;
		n_answer.Set(answer.X, answer.Y, answer.Z);
		n_answer.Normalise();
		answer.X = n_answer.X();
		answer.Y = n_answer.Y();
		answer.Z = n_answer.Z();
	}
	else
	{
		THROW(new JC_Exception("RotationAxis not available on this transform"));
	}
	return answer;
}
//-------------------------------------------------------------------------------------------------
QVERTEX JC_BucketObject::GetAnchorPos(void) const
{
	QVERTEX anchor_pos;
	D3DVECTOR d3d_pos;
	Object()->RealAnchor().AnchorPos().GetD3DVector(d3d_pos);
	anchor_pos.X = d3d_pos.x;
	anchor_pos.Y = d3d_pos.y;
	anchor_pos.Z = d3d_pos.z;
	return anchor_pos;
}
//-------------------------------------------------------------------------------------------------
QVERTEX PS_ExportTransform::CentreOfRotation(void) const
{
	QVERTEX answer;
	if (Type() == ROTATION)
	{
		rv_ObjTransBase * trans = const_cast <rv_ObjTransBase*>(m_Transform);
		rv_ObjTransRotate * rotate = static_cast <rv_ObjTransRotate*>(trans);
		D3DVECTOR& rot_point = rotate->RotationPoint();
		QVERTEX anchor_pos = m_Object->GetAnchorPos();
		answer.X = rot_point.x;
		answer.Y = rot_point.y;
		answer.Z = rot_point.z;
		answer = m_Object->ApplyTransforms(answer);
	}
	else
	{
		THROW(new JC_Exception("CentreOfRotation not available on this transform"));
	}
	return answer;
}
//-------------------------------------------------------------------------------------------------
QVERTEX PS_ExportTransform::TranslationVector(void) const
{
	QVERTEX answer;
	if (Type() == TRANSLATION)
	{
		rv_ObjTransBase * trans = const_cast <rv_ObjTransBase*>(m_Transform);
		rv_ObjTransTranslate * translate = static_cast <rv_ObjTransTranslate*>(trans);
		D3DVECTOR& vector = translate->Offset();
		answer.X = vector.x;
		answer.Y = vector.y;
		answer.Z = vector.z;
		if (ApplyLocally())
		{
			answer = m_Object->ApplyRotations(answer);
			answer = m_Object->ApplyScales(answer);
		}
		else
		{
			if (m_Object->GetParent() != NULL)
			{
				answer = m_Object->GetParent()->ApplyRotations(answer);
				answer = m_Object->GetParent()->ApplyScales(answer);
			}
		}
	}
	else
	{
		THROW(new JC_Exception("TranslationVector not available on this transform"));
	}
	return answer;
}
//-------------------------------------------------------------------------------------------------
JC_BucketFace * PS_ExportTransform::GetFirstFace(void) const
{
	QFACE * face = NULL;
	if (Type() == MORPH)
	{
		PS_ExportTransform * mutable_this = const_cast <PS_ExportTransform*>(this);
		mutable_this->m_FaceArrayIdx = 0;
		face = GetNextFace();
	}
	else
	{
		THROW(new JC_Exception("GetFirstFace is only available in morph transforms"));
	}
	return face;
}
//-------------------------------------------------------------------------------------------------
JC_BucketFace * PS_ExportTransform::GetNextFace(void) const
{
	QFACE * face = NULL;
	if (Type() == MORPH)
	{
		if (m_FaceArrayIdx < 0)
		{
			THROW(new JC_Exception("GetNextFace called before GetFirstFace"));
		}
		else if (m_FaceArrayIdx < m_FaceArray.GetSize())
		{
			PS_ExportTransform * mutable_this = const_cast <PS_ExportTransform*>(this);
			face = m_FaceArray[m_FaceArrayIdx];
			mutable_this->m_FaceArrayIdx++;
		}
	}
	else
	{
		THROW(new JC_Exception("GetFirstFace is only available in morph transforms"));
	}
	return face;
}
//-------------------------------------------------------------------------------------------------
PS_ExportTransform::~PS_ExportTransform(void)
{
	UnloadFaces();
}
//-------------------------------------------------------------------------------------------------
void PS_ExportTransform::UnloadFaces(void)
{
	for (int i = 0; i < m_FaceArray.GetSize(); i++)
	{
		delete m_FaceArray.GetAt(i);
	}
}
//-------------------------------------------------------------------------------------------------
void PS_ExportTransform::LoadFaces(void)
{
	rv_ObjTransBase * base = const_cast <rv_ObjTransBase*>(m_Transform);
	rv_ObjTransMorph * morph = static_cast <rv_ObjTransMorph*>(base);
	const JC_Record * record = morph->MorphObject().Get(JC_Record::TypeRecord);
	if (record && (record->Type() == JC_Record::TypeTemplate || record->Type() == JC_Record::TypeAnimatedTemplate))
	{
		const JC_Template * templ = static_cast <const JC_Template*>(record);
		const JC_FaceList * fl = templ->FaceList();
		POSITION pos = fl->GetHeadPosition();
		QVERTEX morph_offset;

		// add in the morphs anchor pos

		const JC_Anchor& anchor = templ->RealAnchor();
		JC_Vector vector = anchor.AnchorPos();

		morph_offset.X = vector.X();
		morph_offset.Y = vector.Y();
		morph_offset.Z = vector.Z();

		morph_offset = m_Object->ApplyTransforms(morph_offset);
		
		// subtract the holder model's anchor pos

		QVERTEX holder_anchor_pos = m_Object->GetAnchorPos();

		holder_anchor_pos = m_Object->ApplyTransforms(holder_anchor_pos);

		morph_offset.X -= holder_anchor_pos.X;
		morph_offset.Y -= holder_anchor_pos.Y;
		morph_offset.Z -= holder_anchor_pos.Z;

		while (pos)
		{
			JC_Face * f = (JC_Face*)fl->GetNext(pos);
			JC_BucketFace * bf = new PS_ExportMorphFace(f, m_Object, morph_offset);
			m_FaceArray.Add(bf);
		}
	}
}
//-------------------------------------------------------------------------------------------------
PS_ExportArray::PS_ExportArray(JC_NamedDataArray * data, JC_NamedDataArray * back_data, JC_DialogSpreadsheet * array_item)
{
	m_ArrayItem = array_item;
	m_DataArray = data;
	m_BackDataArray = back_data;
}
//-------------------------------------------------------------------------------------------------
PS_ExportArray::~PS_ExportArray(void)
{
	S32 i = m_StructArray.GetSize();
	while (i--)
	{
		delete m_StructArray[i];
	}
}
//-------------------------------------------------------------------------------------------------
PS_ExportStruct * PS_ExportArray::GetRow(const U32 row)
{
	PS_ExportStruct * row_struct = NULL;
	JC_NamedDataCollection * data = NULL;
	if (row >= GetSize())
	{
		THROW(new JC_Exception("Array::GetRow out of range"));
	}
	if (m_BackDataArray && row < (U32)m_BackDataArray->GetSize())
	{
		data = static_cast <JC_NamedDataCollection*>(m_BackDataArray->GetData(row));
	}
	if (m_DataArray && row < (U32)m_DataArray->GetSize())
	{
		JC_NamedDataCollection * front_data = static_cast <JC_NamedDataCollection*>(m_DataArray->GetData(row));
		if (front_data && (!front_data->IsEmpty() || !data))
		{
			data = front_data;
		}
	}
	row_struct = new PS_ExportStruct(data, m_ArrayItem);
	m_StructArray.Add(row_struct);
	return row_struct;
}
//-------------------------------------------------------------------------------------------------
U32 PS_ExportArray::GetSize(void)
{
	U32 size = 0;
	if (m_DataArray)
	{
		size = m_DataArray->GetSize();
	}
	if (m_BackDataArray)
	{
		size = max(size, ((U32)m_BackDataArray->GetSize()));
	}
	return size;
}
//-------------------------------------------------------------------------------------------------
PS_ExportStruct::PS_ExportStruct(JC_NamedDataCollection * row_data, JC_DialogSpreadsheet * array_item)
{
	m_ArrayItem = array_item;
	m_Data = row_data;
}
//-------------------------------------------------------------------------------------------------
JC_DataConcrete * PS_ExportStruct::ExtractData(const CString& name)
{
	U32 col_id;
	if (m_ArrayItem->GetColumnIdFromName(name, col_id))
	{
		CString data_id = m_Data->GetFirstNamedData()->GetDataId();

		// Remove the column id.
		const int colpos = data_id.ReverseFind('.');
		ASSERT(colpos > 0);
		CString trunc = data_id.Left(colpos);			// eg: aaaa.bbbb.rowid

		// Isolate the row id.
		const int rowpos = trunc.ReverseFind('.');
		ASSERT(rowpos > 0);
		CString row_id = trunc.Mid(rowpos+1);			// eg: rowid
		trunc = data_id.Left(rowpos);					// eg: aaaa.bbbb

		CString full_id;
		full_id.Format("%s.%s.%i",trunc, row_id, col_id);
		return (JC_DataConcrete*)m_Data->GetData(full_id);
	}
	else
	{
		return NULL;
	}
}