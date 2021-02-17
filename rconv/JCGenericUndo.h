//-------------------------------------------------------------------------------------------------------------------------
#ifndef jcgenericundo_h
#define jcgenericundo_h
//-------------------------------------------------------------------------------------------------------------------------
class JC_GenericUndo;
//-------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_GenericUndoData	// Any class can use undo/redo if it is derived from this and has a Duplicate()
{
public:
	virtual ~JC_GenericUndoData() { }
};
//-------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_GenericUndoArray : public CTypedPtrArray<CPtrArray, JC_GenericUndo*>
{
private:
	int m_Index;

public:
	JC_GenericUndoArray();
	virtual ~JC_GenericUndoArray();

	void Clear(void);
	void CancelUndo(JC_GenericUndoData * ud);
	void AttachUndo(JC_GenericUndoData * ud, const CString& action);
	JC_GenericUndoData * GetUndoData(JC_GenericUndoData * redodata);
	JC_GenericUndoData * GetRedoData(JC_GenericUndoData * undodata);
	JC_GenericUndo * GetUndo(void)	const	{ if (m_Index >= 0) return GetAt(m_Index); return NULL; }
	JC_GenericUndo * GetRedo(void)	const	{ if (m_Index < GetUpperBound()) return GetAt(m_Index+1); return NULL; }
};
//-------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_GenericUndo
{
private:
	CString m_Action;					// Action that caused object to be added to undo array
	JC_GenericUndoData * m_UndoData;	// Ptr to duplicate of object just before it was changed

public:
	JC_GenericUndo(JC_GenericUndoData * ud, const CString& action);
	virtual ~JC_GenericUndo();

	CString GetAction(void)		const			{ return m_Action; }
//	JC_GenericUndoData * GetUndoData(void)		{ return m_UndoData; }
	JC_GenericUndoData * ReplaceUndoData(JC_GenericUndoData * newdata);
};
//-------------------------------------------------------------------------------------------------------------------------
#endif
//-------------------------------------------------------------------------------------------------------------------------
