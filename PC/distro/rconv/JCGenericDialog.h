//-------------------------------------------------------------------------------------------------------------------------
#ifndef jcgenericdialog_h
#define jcgenericdialog_h
//-------------------------------------------------------------------------------------------------------------------------
#include <HabitatMessage.h>
//-------------------------------------------------------------------------------------------------------------------------
class JCCLASS_DLL JC_GenericDialog : public CDialog
{
private:
	CWnd * m_Parent;
	JC_DialogData * m_DialogData;
	JC_NamedDataList * m_NamedDataList;
	JC_DialogTemplateRecord * m_DialogTemplateRecord;

public:
	JC_GenericDialog(CWnd * parent=NULL);
	virtual ~JC_GenericDialog();

	JC_DialogData * GetDialogData(void);
	void Initialise(JC_DialogData * dd);		// Only used to EDIT existing data
	void Initialise(U32 templaterecnum);		// Only used to CREATE new data

public:	// Static functions
	static void Initialise(PS_Database * db);

protected:
	BOOL OnInitDialog(void);
	void DefaultToEdit(void);
	void DeleteStatement(WORD id);
	void InsertStatement(WORD id);
	void SetArrow(WORD gid, BOOL justadded);
	void CheckDanger(CWordArray& array);
	void ReinitialiseAllStatements(U32 reqrecnum);
	JC_DialogTemplate * DialogTemplate() const { return m_DialogTemplateRecord->GetDialogTemplate(); }
	PS_Database * Database(void) const { return (PS_Database*)::AfxGetMainWnd()->SendMessage(HB_GETDATABASE); }

protected: // Virtual overrides
	virtual void OnOK(void);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
};
//-------------------------------------------------------------------------------------------------------------------------
#endif
//-------------------------------------------------------------------------------------------------------------------------
