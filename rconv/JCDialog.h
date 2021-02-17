//-------------------------------------------------------------------------------------------------------------------------
#pragma once
//-------------------------------------------------------------------------------------------------------------------------
#include "JCClassResource.h"
#include "JCTracker.h"
//-------------------------------------------------------------------------------------------------------------------------
class JC_CustomCWnd;
class JC_DialogItemList;
class JC_DialogSpreadsheet;
class PS_CompressRecordList;
//-------------------------------------------------------------------------------------------------------------------------
class JC_DlgWnd					// Associates a dialog control with a CWnd-derived class.
{
public:
	int ctrlid;					// Control id of dialog control HWND.
	CWnd * wnd;					// Ptr to CWnd-derived class to wrap this HWND.
};
//-------------------------------------------------------------------------------------------------------------------------
class JC_DlgWndArray : public CArray<JC_DlgWnd, JC_DlgWnd&> // Array of JC_DlgWnd, build by CreateDlgItem calls.
{
public:
	void BeginDialog(CWnd * parent, JC_DialogInfo * info);
	void EndDialog(JC_DialogInfo * info);
};
//-------------------------------------------------------------------------------------------------------------------------
class JC_CreateDlgContext		// Context for CreateDlgItem calls.
{
public:
	JC_Buffer buf;				// Buffer that the Windows DlgTemplate is built in.
	JC_DlgWndArray wndarray;	// Array of JC_DlgWnd that associates each HWND with a CWnd.

private:
	int m_ObsoleteCnt;			// Counter for obsolete controls.

public:
	void BeginObsoleteCheck(void);
	void EndObsoleteCheck(void);
	void CountObsolete(void);
	void BeginDialog(CWnd * parent, JC_DialogInfo * info)	{ wndarray.BeginDialog(parent, info); }
	void EndDialog(JC_DialogInfo * info)					{ wndarray.EndDialog(info); }
};
//-------------------------------------------------------------------------------------------------------------------------
// JC_DialogTracker
//-------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_DialogTracker : public JC_Tracker
{
protected: // Static
	static CSize m_GridSize;			//'Snap to grid' size.
	static BOOL  m_GridSnap;			// Flags 'Snap to grid' is on.
	static BOOL  m_GridShow;			// Flags grid is to be shown.
	static CSize m_DialogBaseUnits;		// Base units for coordinate conversion.

public:
	JC_DialogTracker() { }
	virtual ~JC_DialogTracker() { }

	void SnapToGrid(CRect& durect);

public: // Static functions
	static void   SetGrid(CSize size, BOOL snap, BOOL show)	{ m_GridSize = size; m_GridSnap = snap; m_GridShow = show; }
	static CSize  GetGrid(BOOL& snap, BOOL& show)			{ snap = m_GridSnap; show = m_GridShow; return m_GridSize; }
	static void   SetDialogBaseUnits(CDC * dc);
	static void	  ConvertCoords(CPoint& pos, BOOL topixels);
	static void	  ConvertCoords(CSize& size, BOOL topixels);
	static void	  ConvertCoords(CRect& rect, BOOL topixels);
	static CRect  DialogUnitsToPixels(const CRect& rect) { CRect r(rect); ConvertCoords(r, TRUE);  return r; }
	static CSize  DialogUnitsToPixels(const CSize& size) { CSize s(size); ConvertCoords(s, TRUE);  return s; }
	static CPoint DialogUnitsToPixels(const CPoint& pos) { CPoint p(pos); ConvertCoords(p, TRUE);  return p; }
	static CRect  PixelsToDialogUnits(const CRect& rect) { CRect r(rect); ConvertCoords(r, FALSE); return r; }
	static CSize  PixelsToDialogUnits(const CSize& size) { CSize s(size); ConvertCoords(s, FALSE); return s; }
	static CPoint PixelsToDialogUnits(const CPoint& pos) { CPoint p(pos); ConvertCoords(p, FALSE); return p; }

public: // Virtual functions
	virtual void MoveToOrigin(CRect& rect) const;
	virtual void MoveFromOrigin(CRect& rect) const;

public: // Virtual overrides
	virtual void DrawTrackerRect(const CRect& rect, CDC * dc, CWnd * wnd);
};
//-------------------------------------------------------------------------------------------------------------------------
// JC_DialogRect
//-------------------------------------------------------------------------------------------------------------------------
class JC_DialogItemList;
//-------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_DialogRect : public JC_DialogTracker
{
protected:
	CRect m_Rect;						// Position and size of rect in dialog units
	CRect m_SaveRect;					// Position and size of rect saved by BeginRefresh
	CSize m_MinSize;					// Minimum size of rect in dialog units
	BOOL  m_Selected;					// Flags rect is selected (when editing)
	JC_DialogItemList * m_ParentList;	// Ptr to parent list.

protected: // Static
	static BOOL m_TabOrderMode;			// Flags "Tab Order" mode.

public:
	JC_DialogRect(const CRect& rect = CRect(0, 0, 0, 0));
	virtual ~JC_DialogRect();

	void SetTracker(void);
	BOOL IsSelected(void)					const	{ return m_Selected; }
	void SetParentList(JC_DialogItemList * list)	{ m_ParentList = list; }

public: // Static functions
	static BOOL IsTabOrderMode(void)		{ return m_TabOrderMode; }
	static void SetTabOrderMode(BOOL flag)	{ m_TabOrderMode = flag; }

public: // Virtual functions
	virtual void Size(CRect delta);
	virtual void Move(CSize delta);
	virtual void Render(CDC * dc);
	virtual BOOL Select(CRect bound, BOOL toggle);
	virtual void Select(BOOL flag);
	virtual BOOL TabOrder(CPoint pt);
	virtual void Delete(JC_DialogItemList&, BOOL always);
	virtual int  HitTest(CPoint pt) const;
	virtual CRect GetDuRect(void)		const	{ return m_Rect; }
	virtual void SetDuRect(const CRect& rect)	{ m_Rect = rect; }
	virtual BOOL SetCursor(CWnd * wnd, UINT hittest) const;
	virtual void BeginRefresh(CDC * dc);
	virtual void EndRefresh(CDC * dc, CWnd * wnd, BOOL always);
	virtual void TrackBegin(CWnd * wnd, CPoint pt, CDC * dc);
	virtual void TrackMove(CSize delta, CDC * dc);
	virtual void TrackEnd(CDC * dc);
	virtual int  CountSelection(void)	const { return m_Selected ? 1 : 0; }
	virtual JC_DialogItemList * FindTargetList(CRect) { return NULL; }
	virtual void DrawDialogItem(CRect, CDC * dc) = 0;
	virtual void DrawDialogItemName(CRect, CDC*)  const { };
	virtual CString GetDescription(void) const = 0;
	virtual CString GetSelectedDescription(void)  const { if (m_Selected) return GetDescription(); return ""; }
	virtual void Insert(JC_Buffer& buf) const;
	virtual void Extract(JC_Buffer& buf);
	virtual BOOL CanCopy(void)			const { return m_Selected; }
	virtual CRect InflateTrackRect(void) const { return CRect(0, 0, 0, 0); }
	virtual CRect GetRefreshRect(CDC * dc) const;
	virtual BOOL CanSetTabOrder(void) const { return TRUE; }
	virtual void MoveToOrigin(CRect& rect) const;
	virtual void MoveFromOrigin(CRect& rect) const;
	virtual CRect BoundItems(BOOL justsel=FALSE) const;

protected:
	CRect GetPixelRect(void) const;
};
//-------------------------------------------------------------------------------------------------------------------------
// JC_DialogItem
//-------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_DialogItem : public JC_ListElement, public JC_DialogRect
{
public:
	enum TypeDI
	{
		None = -1,
		Group = 0,
		Caption,
			OBSOLETE_Choice,
			OBSOLETE_Statement,
		Button,
			OBSOLETE_EventBox,
			OBSOLETE_EventGroup,
			OBSOLETE_String,
			OBSOLETE_ES16,
			OBSOLETE_Bitmap,
			OBSOLETE_AddEditEvent,
			OBSOLETE_EventDel,
			OBSOLETE_DelStatement,
			OBSOLETE_UpArrow,
			OBSOLETE_DnArrow,
		OkButton,
		CancelButton,
		NewStatement,
		Spreadsheet,
		NewChoice,
		NewEdit,
	};

protected:
	CString	m_Name;		// Name of dialog item
	CString	m_DataId;	// Data Id of dialog item... given to named data so that it can be found again.
	U16 m_UserLevel;	// If the user level in the registry is lower than this value, the control is hidden.
	static U16 m_Version;

public:
	JC_DialogItem(const CRect& rect, const CString& name);
	virtual ~JC_DialogItem() { }

	CString GetName(void)	  const		{ return m_Name; }		// (eg: "Choice")
	CString GetFullName(void) const;							// (eg: "DoorGroup.OpenWhen.Choice")
	void SetDataId(const CString& did)	{ m_DataId = did; }
	CString GetDataId(void)		const	{ return m_DataId; }
	BOOL IsNamed(const CString& name) const;
	BOOL DataIdIsSet(void)		const	{ return (BOOL)(m_DataId != ""); }
	U16  GetUserLevel(void)		const	{ return m_UserLevel; }
	void SetUserLevel(U16 lev)			{ m_UserLevel = lev; }
	static void SetLoadVersion(const U16 version) { m_Version = version; }

public: // Virtual functions
	virtual CString GetWindowClassName(void) const = 0;
	virtual CWnd * NewWindowInstance(void) const = 0;
	virtual TypeDI GetType(void) const = 0;
	virtual DWORD GetStyle(void) const { return 0; }
	virtual int GetCtrlId(void)  const { return 0xFFFF; }
	virtual void SetDataIds(void);
	virtual void Clear(void) const { }
	virtual void SetName(const CString& name) { m_Name = name; }
	virtual BOOL NeedsName(void)	const	{ return FALSE; }
	virtual BOOL NeedsCaption(void)	const	{ return FALSE; }
	virtual CString GetCaption(void) const	{ return ""; }
	virtual void SetCaption(const CString&)	{ }
	virtual void CreateDlgTemplate(JC_CreateDlgContext& cd);
	virtual int  GetTotalCount(void)const	{ return 1; }
	virtual int  GetSelection(void)	const	 { return -1; }
	virtual CString GetItemNames(void) const { return GetFullName(); }
	virtual void Copy(JC_Buffer& buf) const;
	virtual void Paste(JC_Buffer& buf, JC_DialogItemList * list);
	virtual CRect GetRefreshRect(CDC * dc)  const;
	virtual JC_DialogItem * GetSelectedItem(void)	{ if (m_Selected) return this; return NULL; }
	virtual JC_RecordRef * GetRecordRef(void)		{ return NULL; }
	virtual void SetRecordRef(const JC_RecordRef&)	{ }
	virtual JC_DataConcrete * GetDataType(void)		{ return NULL; }
	virtual void SetDataType(JC_DataConcrete*)		{ }
	virtual int CountIllegalChoices(void)	const 	{ return 0; }
	virtual void SelectIllegalChoices(void)			{ }
	virtual CString GetDataIdFromName(const CString & name) const;
	virtual BOOL PregnantGroupIsSelected(void) const { return FALSE; }
	virtual void EraseDataId(void)					{ m_DataId = ""; }
	virtual int  UpdateControls(void)				{ return 0; }
	virtual void AddToSpreadsheet(JC_DialogSpreadsheet * dss);
	virtual void ResolveRefs(JC_UpgradeContext&) { }

	virtual BOOL IsEdit(void)			const		{ return FALSE; }
	virtual BOOL IsGroup(void)			const		{ return FALSE; }
	virtual BOOL IsChoice(void)			const		{ return FALSE; }
	virtual BOOL IsStatement(void)		const		{ return FALSE; }
	virtual BOOL IsSpreadsheet(void)	const		{ return FALSE; }
	virtual BOOL IsCustomisable(void)	const		{ return FALSE; }

public: // Virtual overrides
	virtual void Insert(JC_Buffer& buf) const;
	virtual void Extract(JC_Buffer& buf);
	virtual void DrawDialogItemName(CRect rect, CDC * dc) const;
	virtual U8 ListElementType(void) const	{ return (U8)GetType(); }
	virtual JC_DialogItem * GetItemFromId(const CString & id) const;
	virtual bool UsesEnumeratedList(U32 /*rec_num*/) const { return false; }
	virtual void ReplaceEnumeratedList(U32 /*source*/, U32 /*target*/) {}
};
//-------------------------------------------------------------------------------------------------------------------------
// JC_DialogCaption - A simple static text window.
//-------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_DialogCaption : public JC_DialogItem
{
private:
	CString m_Caption;	// Text of caption.

public:
	JC_DialogCaption(const CRect& rect, const CString& name, const CString& text)
						: JC_DialogItem(rect, name) { m_Caption = text; }
	virtual ~JC_DialogCaption() { }

public: // Virtual overrides
	virtual CString GetWindowClassName(void) const { return "STATIC"; }
	virtual CWnd * NewWindowInstance(void)const { return new CStatic; }
	virtual BOOL NeedsCaption(void)		const	{ return TRUE; }
	virtual CString GetCaption(void)	const	{ return m_Caption; }
	virtual void SetCaption(const CString& cap)	{ m_Caption = cap; }
	virtual TypeDI GetType(void)		const	{ return Caption; }
	virtual void Insert(JC_Buffer& buf) const;
	virtual void Extract(JC_Buffer& buf);
	virtual void DrawDialogItem(CRect, CDC * dc);
	virtual CString GetDescription(void) const { return "Caption"; }
};
//-------------------------------------------------------------------------------------------------------------------------
// JC_DialogNewCustom - Base class for the new custom controls.
//-------------------------------------------------------------------------------------------------------------------------
#define JC_CUSTOM_DIALOG_CONTROL_CLASSNAME "HB Custom Dialog Control"
//-------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_DialogNewCustom : public JC_DialogItem
{
public:
	JC_DialogNewCustom(const CRect& rect, const CString& name) : JC_DialogItem(rect, name) { }
	virtual ~JC_DialogNewCustom() { }

public: // Virtual overrides
	virtual CString GetWindowClassName(void) const { return JC_CUSTOM_DIALOG_CONTROL_CLASSNAME; }
	virtual DWORD GetStyle(void) const { return WS_TABSTOP; }
};
//-------------------------------------------------------------------------------------------------------------------------
// JC_DialogNewChoice - A choice of items from an EnumRecord which is fixed.
//-------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_DialogNewChoice : public JC_DialogNewCustom
{
protected:
	JC_RecordRef m_EnumRecord;	// Enum record containing choice of items.

public:
	JC_DialogNewChoice(const CRect& rect, const CString& name, const JC_RecordRef& rr)
						: JC_DialogNewCustom(rect, name), m_EnumRecord(rr) { }
	virtual ~JC_DialogNewChoice() { }

public: // Virtual overrides
	virtual void Insert(JC_Buffer& buf) const;
	virtual void Extract(JC_Buffer& buf);
	virtual void SelectIllegalChoices(void);
	virtual CWnd * NewWindowInstance(void)const;
	virtual int CountIllegalChoices(void) const;
	virtual TypeDI GetType(void) const { return NewChoice; }
	virtual BOOL IsChoice(void)  const { return TRUE; }
	virtual BOOL NeedsName(void) const { return TRUE; }
	virtual void DrawDialogItem(CRect rect, CDC * dc);
	virtual CString GetDescription(void) const { return "Choice"; }
	virtual JC_RecordRef * GetRecordRef(void)  { return &m_EnumRecord; }
	virtual void SetRecordRef(const JC_RecordRef& rr) { m_EnumRecord = rr; }
	virtual void ResolveRefs(JC_UpgradeContext& upgrade);
	virtual bool UsesEnumeratedList(U32 rec_num) const;
	virtual void ReplaceEnumeratedList(U32 source, U32 target);
};
//-------------------------------------------------------------------------------------------------------------------------
// JC_DialogNewStatement - A choice of items from an EnumRecord which is editable by the user of the dialog.
//-------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_DialogNewStatement : public JC_DialogNewChoice
{
public:
	JC_DialogNewStatement(const CRect& rect, const CString& name, const JC_RecordRef& rr)
						: JC_DialogNewChoice(rect, name, rr) { }
	virtual ~JC_DialogNewStatement() { }

public: // Virtual overrides
	virtual CWnd * NewWindowInstance(void) const;
	virtual TypeDI GetType(void) const { return NewStatement; }
	virtual BOOL IsStatement(void) const { return TRUE; }
	virtual void DrawDialogItem(CRect rect, CDC * dc);
	virtual CString GetDescription(void) const { return "Statement"; }
};
//-------------------------------------------------------------------------------------------------------------------------
// JC_DialogNewEdit
//-------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_DialogNewEdit : public JC_DialogNewCustom
{
private:
	JC_DataConcrete * m_DataType;	// Used as an indication of the associated data type.

public:
	JC_DialogNewEdit(const CRect& rect, const CString& name, JC_DataConcrete * datatype);
	virtual ~JC_DialogNewEdit();

public: // Virtual overrides
	virtual void Insert(JC_Buffer& buf) const;
	virtual void Extract(JC_Buffer& buf);
	virtual CString GetDescription(void) const;
	virtual CWnd * NewWindowInstance(void) const;
	virtual void DrawDialogItem(CRect rect, CDC * dc);
	virtual BOOL NeedsName(void)	const { return TRUE; }
	virtual BOOL IsEdit(void)		const { return TRUE; }
	virtual TypeDI GetType(void)	const { return NewEdit; }
	virtual JC_DataConcrete * GetDataType(void)		{ return m_DataType; }
	virtual void SetDataType(JC_DataConcrete * dt);
};
//-------------------------------------------------------------------------------------------------------------------------
// JC_DialogSpreadsheet - A spreadsheet-like control for entering events made up of several data fields.
//-------------------------------------------------------------------------------------------------------------------------
class JC_SpreadsheetColumnArray;
//-------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_DialogSpreadsheet : public JC_DialogNewCustom
{
protected:
	CString m_Caption;						// Text of caption.
	JC_SpreadsheetColumnArray * m_Columns;	// Ptr to column array.

public:
	JC_DialogSpreadsheet(const CRect& rect, const CString& name, const CString& dataid, PS_Database * db);
	virtual ~JC_DialogSpreadsheet();

	void AddColumn(int width, JC_CustomCWnd * control, const CString& name);

public: // Virtual overrides
	virtual void Insert(JC_Buffer& buf) const;
	virtual void Extract(JC_Buffer& buf);
	virtual CWnd * NewWindowInstance(void) const;
	virtual TypeDI GetType(void) const { return Spreadsheet; }
	virtual BOOL IsCustomisable(void)	 const { return TRUE; }
	virtual BOOL IsSpreadsheet(void)	 const { return TRUE; }
	virtual BOOL NeedsCaption(void)		 const { return TRUE; }
	virtual CString GetCaption(void)	 const { return m_Caption; }
	virtual void SetCaption(const CString& cap){ m_Caption = cap; }
	virtual BOOL NeedsName(void)		 const { return TRUE; }
	virtual DWORD GetStyle(void) const;
	virtual void DrawDialogItem(CRect rect, CDC * dc);
	virtual CString GetDescription(void) const { return "Spreadsheet"; }
	virtual void ResolveRefs(JC_UpgradeContext& upgrade);
	bool GetColumnIdFromName(const CString& column_name, U32& id) const;
	virtual bool UsesEnumeratedList(U32 rec_num) const;
	virtual void ReplaceEnumeratedList(U32 source, U32 target);
};
//-------------------------------------------------------------------------------------------------------------------------
// JC_DialogButton, JC_DialogOkButton, JC_DialogCancelButton
//-------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_DialogButton : public JC_DialogItem
{
protected:
	CString m_Caption;		// Caption text.

public:
	JC_DialogButton(const CRect& rect, const CString& name, const CString& caption)
						: JC_DialogItem(rect, name) { m_Caption = caption; }
	virtual ~JC_DialogButton() { }

public: // Virtual overrides
	virtual CString GetWindowClassName(void) const { return "BUTTON"; }
	virtual CWnd * NewWindowInstance(void)const { return new CButton; }
	virtual TypeDI GetType(void)		const	{ return Button; }
	virtual BOOL NeedsCaption(void)		const	{ return TRUE; }
	virtual CString GetCaption(void)	const	{ return m_Caption; }
	virtual void SetCaption(const CString& cap)	{ m_Caption = cap; }
	virtual DWORD GetStyle(void)		const	{ return BS_PUSHBUTTON | WS_TABSTOP; }
	virtual void DrawDialogItem(CRect rect, CDC * dc);
	virtual int UpdateControls(void);
	virtual void Insert(JC_Buffer& buf) const;
	virtual void Extract(JC_Buffer& buf);
	virtual CString GetDescription(void) const { return "Button"; }
};
//-------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_DialogOkButton : public JC_DialogButton
{
public:
	JC_DialogOkButton(const CRect& rect) : JC_DialogButton(rect, "", "OK") { }
	virtual ~JC_DialogOkButton() { }

	virtual int GetCtrlId(void)				const { return IDOK; }
	virtual TypeDI GetType(void)			const { return OkButton; }
	virtual CString GetDescription(void)	const { return "Ok Button"; }
};
//-------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_DialogCancelButton : public JC_DialogButton
{
public:
	JC_DialogCancelButton(const CRect& rect) : JC_DialogButton(rect, "", "Cancel") { }
	virtual ~JC_DialogCancelButton() { }

	virtual int GetCtrlId(void)				const { return IDCANCEL; }
	virtual TypeDI GetType(void)			const { return CancelButton; }
	virtual CString GetDescription(void)	const { return "Cancel Button"; }
};
//-------------------------------------------------------------------------------------------------------------------------
// JC_DialogItemList
//-------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_DialogItemList : public JC_List
{
protected:
	U32 m_Sequence;
	PS_Database * m_Database;

private: // Static
	static int m_TabCnt;

public:
	JC_DialogItemList(PS_Database * db) { m_Database = db; m_Sequence = 1; }
	virtual ~JC_DialogItemList()		{ }

	void Render(CDC * dc);
	void Clear(void) const;
	void CreateDlgTemplate(JC_CreateDlgContext& cd);
	void AddItem(JC_DialogItem * di);
	void AttachItemList(JC_DialogItemList * dil);
	int  GetTotalCount(void) const;
	void Size(CRect delta);
	void Move(CSize delta);
	void Select(BOOL flag);
	BOOL TabOrder(CPoint pt);
	void Delete(BOOL always);
	BOOL Select(CRect bound, BOOL toggle);
	BOOL SetCursor(CWnd * wnd, UINT hittest) const;
	void BeginRefresh(CDC * dc);
	void EndRefresh(CDC * dc, CWnd * wnd, BOOL always);
	int  CountSelection(void) const;
	void TrackBegin(CWnd * wnd, CPoint pt, CDC * dc);
	void TrackMove(CSize delta, CDC * dc);
	void TrackEnd(CDC * dc);
	CString GetItemNames(void) const;
	CString GetSelectedDescription(void) const;
	JC_DialogItem * GetSelectedItem(void);
	void Copy(JC_Buffer& buf) const;
	void Paste(JC_Buffer& buf);
	JC_DialogItemList * FindTargetList(CRect bound);
	BOOL NameExists(const CString& name) const;
	JC_DialogItemList * MakeItemList(JC_DialogItem::TypeDI type, const CRect& rect, CString& desc);
	void SetDataIds(void);
	CString GetUniqueDataId(void);
	int  CountIllegalChoices(void) const;
	void SelectIllegalChoices(void);
	void SetTabOrder(JC_DialogRect * dr);
	int  GetTabOrder(JC_DialogRect * dr) const;
	CRect BoundItems(BOOL justsel=FALSE) const;
	BOOL PregnantGroupIsSelected(void) const;
	void EraseDataId(void);
	int  UpdateControls(void);
	void ResolveRefs(JC_UpgradeContext& upgrade);
	void AddToSpreadsheet(JC_DialogSpreadsheet * dss);
	PS_Database * GetDatabase(void) const { return m_Database; }
	CString GetDataIdFromName(const CString & name) const;
	bool UsesEnumeratedList(U32 rec_num) const;
	void ReplaceEnumeratedList(U32 source, U32 target);

public:    // Static
	static void ResetTabCnt(void)	{ m_TabCnt = 0; }

public:
	virtual int HitTest(CPoint pt) const;
	virtual JC_ListElement * NewType(U8 type);
	virtual void Insert(JC_Buffer& buf)		const	{ JC_List::Insert(buf);  buf << m_Sequence; }
	virtual void Extract(JC_Buffer& buf)			{ JC_List::Extract(buf); buf >> m_Sequence; }
	virtual POSITION AttachTail(JC_ListElement * le);
	virtual CString GetDataIdPrefix(void)	const	{ return ""; }
	virtual CString GetDataId(void)			const	{ ASSERT(FALSE); return ""; }			// Should never get here
	virtual JC_DialogItemList * GetParentList(void)	const { ASSERT(FALSE); return NULL; }	// Should never get here
	virtual BOOL IsListSelected(void)		const	{ ASSERT(FALSE); return FALSE; }		// Should never get here
	virtual CString GetNameRoot(void)		const	{ ASSERT(FALSE); return ""; }			// Should never get here
	JC_DialogItem * GetItemFromId(const CString & id) const; 
};
//-------------------------------------------------------------------------------------------------------------------------
// JC_DialogTemplateList - The one and only top-level item list
//-------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_DialogTemplateList : public JC_DialogItemList
{
private:
	BOOL m_ListSelected;	// Set by the dialog template when it becomes selected.

public:
	JC_DialogTemplateList(PS_Database * db) : JC_DialogItemList(db) { m_ListSelected = FALSE; }
	virtual ~JC_DialogTemplateList() { }

	void SetListSelected(BOOL flag)			{ m_ListSelected = flag; }

public: // Virtual overrides
	virtual CString GetDataIdPrefix(void)	const { return ""; }
	virtual CString GetDataId(void)			const { return ""; }
	virtual BOOL IsListSelected(void)		const { return m_ListSelected; }
	virtual CString GetNameRoot(void)		const { return ""; }
	virtual JC_DialogItemList * GetParentList(void)	const { return NULL; }
};
//-------------------------------------------------------------------------------------------------------------------------
// JC_DialogGroup
//-------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_DialogGroup : public JC_DialogButton, public JC_DialogItemList
{
private:
	short m_TopMargin;	// Allows for the fact that Windows offsets the top of a group box by half the text height.

public:
	JC_DialogGroup(PS_Database * db, const CRect& rect, const CString& name, const CString& caption)
				: JC_DialogButton(rect, name, caption), JC_DialogItemList(db) { m_TopMargin = 0; }
	virtual ~JC_DialogGroup() { }

public: // Virtual overrides
	virtual TypeDI GetType(void)		const	{ return Group; }
	virtual DWORD GetStyle(void)		const	{ return BS_GROUPBOX; }
	virtual int  GetTotalCount(void)	const	{ return JC_DialogButton::GetTotalCount() + JC_DialogItemList::GetTotalCount(); }
	virtual void DrawDialogItem(CRect rect, CDC * dc);
	virtual void Size(CRect d)					{ JC_DialogButton::Size(d);          JC_DialogItemList::Size(d); }
	virtual void Move(CSize d)					{ JC_DialogButton::Move(d);          JC_DialogItemList::Move(d); }
	virtual void Render(CDC * dc)				{ JC_DialogButton::Render(dc);       JC_DialogItemList::Render(dc); }
	virtual void Select(BOOL flag)				{ JC_DialogButton::Select(flag);     JC_DialogItemList::Select(flag); }
	virtual void BeginRefresh(CDC * dc)			{ JC_DialogButton::BeginRefresh(dc); JC_DialogItemList::BeginRefresh(dc); }
	virtual void EraseDataId(void)				{ JC_DialogButton::EraseDataId();    JC_DialogItemList::EraseDataId(); }
	virtual BOOL TabOrder(CPoint pt);
	virtual void Delete(JC_DialogItemList&, BOOL);
	virtual void Insert(JC_Buffer& buf) const;
	virtual void Extract(JC_Buffer& buf);
	virtual BOOL HitTest(CPoint pt) const;
	virtual void EndRefresh(CDC * d, CWnd * w, BOOL a)	{ JC_DialogButton::EndRefresh(d, w, a); JC_DialogItemList::EndRefresh(d, w, a); }
	virtual void TrackBegin(CWnd * wnd, CPoint pt, CDC * dc);
	virtual void TrackMove(CSize delta, CDC * dc);
	virtual void TrackEnd(CDC * dc);
	virtual int  CountIllegalChoices(void)const { return JC_DialogItemList::CountIllegalChoices(); }
	virtual void SelectIllegalChoices(void)		{ JC_DialogItemList::SelectIllegalChoices(); }
	virtual void Copy(JC_Buffer& buf) const;
	virtual void Paste(JC_Buffer& buf, JC_DialogItemList * list);
	virtual BOOL CanCopy(void)			const	{ return TRUE; }
	virtual BOOL SetCursor(CWnd * wnd, UINT hittest) const;
	virtual BOOL Select(CRect bound, BOOL toggle);
	virtual CRect InflateTrackRect(void) const { return CRect(4, 0, 4, 4); }
	virtual CString GetDescription(void) const { return "Group Box"; }
	virtual JC_DialogItemList * FindTargetList(CRect bound);
	virtual CString GetSelectedDescription(void) const;
	virtual JC_DialogItem * GetSelectedItem(void);
	virtual CString GetItemNames(void) const;
	virtual int CountSelection(void) const;
	virtual void CreateDlgTemplate(JC_CreateDlgContext& cd);
	virtual void DrawDialogItemName(CRect rect, CDC * dc) const;
	virtual BOOL IsGroup(void)	  const { return TRUE; }
	virtual BOOL NeedsName(void)  const { return TRUE; }
	virtual void SetDataIds(void);
	virtual CString GetDataIdPrefix(void)	const { return m_DataId + "."; }
	virtual CString GetDataId(void)			const { return m_DataId; }
	virtual BOOL IsListSelected(void)		const { return IsSelected(); }
	virtual JC_DialogItemList * GetParentList(void)	const { return m_ParentList; }
	virtual CRect BoundItems(BOOL justsel=FALSE) const;
	virtual BOOL PregnantGroupIsSelected(void) const;
	virtual CString GetDataIdFromName(const CString & name) const;
	virtual int UpdateControls(void);
	virtual void ResolveRefs(JC_UpgradeContext& upgrade);
	virtual CString GetNameRoot(void) const { return JC_DialogButton::GetFullName(); }
	virtual JC_DialogItem * GetItemFromId(const CString& id) const;
	virtual bool UsesEnumeratedList(U32 rec_num) const;
	virtual void ReplaceEnumeratedList(U32 source, U32 target);

protected:
	BOOL ExcludeHit(CPoint pt) const;
};
//-------------------------------------------------------------------------------------------------------------------------
// JC_DialogTemplate
//-------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_DialogTemplate : public JC_DialogRect, public JC_GenericUndoData
{
private:
	CString m_Title;
	PS_Database * m_Database;
	JC_DialogTemplateList * m_ItemList;
	JC_CreateDlgContext m_CreateContext;

public:
	JC_DialogTemplate(PS_Database * db);
	virtual ~JC_DialogTemplate();

	LPCDLGTEMPLATE CreateDlgTemplate(void);
	void SetTitle(const CString& str)						{ m_Title = str; }
	CString GetTitle(void) const							{ return m_Title; }
	void BeginDialog(CWnd * parent, JC_DialogInfo * info);
	void EndDialog(JC_DialogInfo * info);
	JC_DlgWndArray& GetDlgWndArray(void)					{ return m_CreateContext.wndarray; }
	void AddItem(JC_DialogItem * di)						{ m_ItemList->AddItem(di); }
	void ResolveRefs(JC_UpgradeContext& upgrade)			{ m_ItemList->ResolveRefs(upgrade); }
	int  UpdateControls(void);
	void DoSelectIllegalChoices(void);
	int  DoCountSelection(void)			const				{ return m_ItemList->CountSelection(); }
	int  DoCountIllegalChoices(void)	const				{ return m_ItemList->CountIllegalChoices(); }
	CString GetDataIdFromName(const CString & name) const	{ return m_ItemList->GetDataIdFromName(name); }
	BOOL DoTabOrder(CPoint pt);
	BOOL DoTabOrderReset(CPoint pt);
	void DoCopy(JC_Buffer& buf) const;
	void DoPaste(JC_Buffer& buf, CPoint pixpoint, BOOL erasedataid=FALSE);
	void DoDelete(void);
	void TrackBegin(CWnd * wnd, CPoint pt, CDC * dc);
	void TrackMove(CSize delta, CDC * dc);
	void TrackEnd(CDC * dc);
	void BeginRefresh(CDC * dc);
	void EndRefresh(CDC * dc, CWnd * wnd, BOOL always);
	void Render(CDC * dc);
	CSize GetSize(void) const;
	void DoSize(CRect delta);
	void DoMove(CSize delta);
	void DoSelect(CRect bound, int flags);
	int  DoHitTest(CPoint pt) const;
	JC_DialogTemplate * Duplicate(void) const;
	void Insert(JC_Buffer& buf) const;
	void Extract(JC_Buffer& buf);
	void SelectDialogBox(void);
	void SelectionRules(void);
	JC_DialogItemList * MakeItemList(JC_DialogItem::TypeDI type, CRect pixrect, CString& desc);
	const JC_DialogItemList* GetItemList() const		{ return m_ItemList;	}
	void PlaceItemList(JC_DialogItemList * dil);
	void SetDataIds(void);
	bool UsesEnumeratedList(U32 rec_num) const;
	void ReplaceEnumeratedList(U32 source, U32 target);

public: // Static functions
	static BOOL IsTabOrderMode(void)			{ return JC_DialogItem::IsTabOrderMode(); }
	static void DoSetTabOrderMode(BOOL flag);

public: // Virtual overrides
	virtual void DrawDialogItem(CRect rect, CDC * dc);
	virtual BOOL SetCursor(CWnd * wnd, UINT hittest) const;
	virtual CString GetDescription(void) const { return "Dialog Box"; }
	virtual CString GetSelectedDescription(void) const;
	virtual JC_DialogItem * GetSelectedItem(void);
	virtual CRect InflateTrackRect(void) const;
	JC_DialogItem * GetItemFromId(const CString& id) const  { return m_ItemList->GetItemFromId(id); }
};
//-------------------------------------------------------------------------------------------------------------------------
// JC_DialogTemplateRecord
//-------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_DialogTemplateRecord : public JC_Record
{
private:
	JC_DialogTemplate * m_DialogTemplate;	// Ptr to dialog template

public:
	JC_DialogTemplateRecord(PS_Database * db, U32 recnum);
	virtual ~JC_DialogTemplateRecord();

	JC_DialogTemplate * GetDialogTemplate(void)			{ return m_DialogTemplate; }
	void SetDialogTemplate(JC_DialogTemplate * dt)		{ m_DialogTemplate = dt; }
	JC_DialogData * CreateDialogData(void) const;
	bool UsesEnumeratedList(U32 rec_num) const;
	void ReplaceEnumeratedList(U32 source, U32 target);

public: // Virtual overrides
	virtual bool Delete(void);
	virtual bool CopyFromRecord(const JC_Record * source, PS_RecordRemap& copy_map);
	virtual void WriteBody(void);
	virtual void ReadBody(void);
	virtual U16 Type(void)	const		{ return TypeDialogTemplateRecord; }
	virtual void Dependencies(PS_CompressRecordList * list) const;
	virtual void ResolveRefs(JC_UpgradeContext& upgrade) { m_DialogTemplate->ResolveRefs(upgrade); }

public: // Debugging
	virtual CString TypeString(void) const { return CString("Dialog Template"); }
};
//-------------------------------------------------------------------------------------------------------------------------
//
//                                      O b s o l e t e   C l a s s e s
//                                      -------------------------------
//
// The following obsolete classes have been retained to keep old databases working and to convert/delete themselves.
//
//-------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_DialogObsolete : public JC_DialogItem
{
public:
	JC_DialogObsolete() : JC_DialogItem(CRect(0, 0, 0, 0), "") { }
	virtual int UpdateControls(void) { return 0; }
	virtual void DrawDialogItem(CRect, CDC*) { }
	virtual CWnd * NewWindowInstance(void) const { return NULL; }
	virtual CString GetDescription(void) const { return "Obsolete"; }
	virtual CString GetWindowClassName(void) const { return "Obsolete"; }
};
//-------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_DialogChoice : public JC_DialogObsolete
{
private:
	JC_RecordRef m_RecordRef;

public:
	JC_DialogChoice() : m_RecordRef(NULL) { }
	void Insert(JC_Buffer& buf) const;
	void Extract(JC_Buffer& buf);
	int UpdateControls(void);
	TypeDI GetType(void) const { return OBSOLETE_Choice; }
};
//-------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_DialogStatement : public JC_DialogChoice
{
public:
	TypeDI GetType(void) const { return OBSOLETE_Statement; }
};
//-------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_DialogString : public JC_DialogObsolete
{
public:
	int UpdateControls(void);
	TypeDI GetType(void) const { return OBSOLETE_String; }
};
//-------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_DialogES16 : public JC_DialogObsolete
{
public:
	int UpdateControls(void);
	TypeDI GetType(void) const { return OBSOLETE_ES16; }
};
//-------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_DialogBitmap : public JC_DialogObsolete
{
public:
	void Insert(JC_Buffer& buf) const;
	void Extract(JC_Buffer& buf);
	TypeDI GetType(void) const { return OBSOLETE_Bitmap; }
};
//-------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_DialogDelStatement : public JC_DialogButton
{
public:
	JC_DialogDelStatement() : JC_DialogButton(CRect(0, 0, 0, 0), "", "") { }
	int UpdateControls(void);
	virtual void DrawDialogItem(CRect, CDC*) { }
	virtual CWnd * NewWindowInstance(void) const { return NULL; }
	virtual CString GetDescription(void) const { return "Obsolete"; }
	virtual CString GetWindowClassName(void) const { return "Obsolete"; }
	TypeDI GetType(void) const { return OBSOLETE_DelStatement; }
};
//-------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_DialogEventBox : public JC_DialogObsolete
{
public:
	TypeDI GetType(void) const { return OBSOLETE_EventBox; }
};
//-------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_DialogEventGroup : public JC_DialogGroup
{
public:
	JC_DialogEventGroup(PS_Database * db, const CRect& rect, const CString& name, const CString& caption)
						: JC_DialogGroup(db, rect, name, caption) { }
	virtual int UpdateControls(void);
	virtual void DrawDialogItem(CRect, CDC*) { }
	virtual CWnd * NewWindowInstance(void) const { return NULL; }
	virtual CString GetDescription(void) const { return "Obsolete"; }
	virtual CString GetWindowClassName(void) const { return "Obsolete"; }
	virtual TypeDI GetType(void) const { return OBSOLETE_EventGroup; }
};
//-------------------------------------------------------------------------------------------------------------------------
