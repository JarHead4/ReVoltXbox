#ifndef PS_UNIQUERECORDLIST_H
#define PS_UNIQUERECORDLIST_H

class JCCLASS_DLL PS_UniqueRecordList: public CObject
{
	CList<JC_Record *, JC_Record *&> m_List;
	CList<U32, U32&> m_Filters;
public:
	~PS_UniqueRecordList();
	BOOL Add(const JC_Record * record);

	BOOL Merge(const PS_UniqueRecordList * list);
	BOOL Remove(const JC_Record * record);

	BOOL AddFilter(JC_Record::RecType filter, BOOL update = FALSE);
	BOOL HasFilter(JC_Record::RecType filter) const { U32 f = filter; return (m_Filters.Find(f) != NULL); }
	BOOL RemoveFilter(JC_Record::RecType filter);
	void RemoveAllFilters() { m_Filters.RemoveAll(); }
	// The functions of m_List in the public interface of PS_UniqueRecordList
	void RemoveAll(void);
	POSITION GetHeadPosition() const { return m_List.GetHeadPosition(); }
	POSITION GetTailPosition() const { return m_List.GetTailPosition(); }
	JC_Record * GetNext(POSITION& pos) const; 
	JC_Record * GetPrev(POSITION& pos) const; 
	POSITION Find(const JC_Record * const & record, POSITION start_after = NULL) const;
	POSITION FindIndex(int index) const { return m_List.FindIndex(index); }
	int GetCount() const { return m_List.GetCount(); }
	BOOL IsEmpty() const { return m_List.IsEmpty(); }
};

class JCCLASS_DLL PS_CompressRecordList: public CObject
{
	CList<U32, U32&> m_List;
public:
	DECLARE_DYNAMIC(PS_CompressRecordList)
	BOOL Add(U32 record);
	BOOL Merge(const PS_CompressRecordList * list);
	BOOL Remove(U32 record);
	// The functions of m_List in the public interface of PS_UniqueRecordList
	void RemoveAll(void) { m_List.RemoveAll(); }
	POSITION GetHeadPosition() const { return m_List.GetHeadPosition(); }
	POSITION GetTailPosition() const { return m_List.GetTailPosition(); }
	U32 GetNext(POSITION& pos) const { return m_List.GetNext(pos); }
	U32 GetPrev(POSITION& pos) const { return m_List.GetPrev(pos); }
	U32	RemoveHead()				 { return m_List.RemoveHead(); }
	POSITION Find(U32 record, POSITION start_after = NULL) const { return m_List.Find(record, start_after); }
	POSITION FindIndex(int index) const { return m_List.FindIndex(index); }
	int GetCount() const { return m_List.GetCount(); }
	BOOL IsEmpty() const { return m_List.IsEmpty(); }
};

#endif