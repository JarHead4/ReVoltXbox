#ifndef PS_UNDO
#define PS_UNDO

//-------------------------------------------------------------------------------------------------
template < class UNDO > class PS_UndoArray: public CArray < UNDO *, UNDO * >
{
	int		m_UndoPos;
public:
	PS_UndoArray(void) { m_UndoPos = -1; }
	~PS_UndoArray(void);
	void SetUndo(UNDO * undo);
	UNDO * GetUndo(void) { return ElementAt(m_UndoPos); }
	UNDO * GetRedo(void) { return ElementAt(m_UndoPos + 1); }
	void Undo(UNDO * redo);
	void Redo(UNDO * undo);
	const BOOL		 CanUndo(void) const { return (m_UndoPos >= 0 && m_UndoPos < GetSize()); }
	const BOOL		 CanRedo(void) const { return (m_UndoPos >= -1 && m_UndoPos < GetSize()-1); }
};
//-------------------------------------------------------------------------------------------------
template <class UNDO> PS_UndoArray<UNDO>::~PS_UndoArray()
{
	U32 count = GetSize();
	while (count--)
	{
		delete ElementAt(count);
	}
}
//-------------------------------------------------------------------------------------------------
template <class UNDO> void PS_UndoArray<UNDO>::SetUndo(UNDO * undo)
{
	ASSERT(m_UndoPos >= -1);
	int max = GetSize();
	while (max > m_UndoPos + 1)
	{
		max --;
		UNDO * undo = ElementAt(max);
		delete undo;
		RemoveAt(max);
	}
	m_UndoPos++;
	Add(undo);
}
//-------------------------------------------------------------------------------------------------
template <class UNDO> void PS_UndoArray<UNDO>::Undo(UNDO * redo)
{
	ASSERT(CanUndo());
	UNDO * undo = ElementAt(m_UndoPos);
	RemoveAt(m_UndoPos);
	InsertAt(m_UndoPos, redo);
	m_UndoPos--;
	delete undo;
}
//-------------------------------------------------------------------------------------------------
template <class UNDO> void PS_UndoArray<UNDO>::Redo(UNDO * undo)
{
	ASSERT(CanRedo());
	m_UndoPos++;
	UNDO * redo = ElementAt(m_UndoPos);
	RemoveAt(m_UndoPos);
	InsertAt(m_UndoPos, undo);
	delete redo;
}
//-------------------------------------------------------------------------------------------------

#endif