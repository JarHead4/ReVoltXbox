// RecordSelector.h : header file
//
#include "JCClassResource.h"
/////////////////////////////////////////////////////////////////////////////
// PS_RecordSelector dialog

class PS_RecordSelector : public CDialog
{
// Construction
public:
	U32 AddPath(PS_RecordInfo& info) const;
	U32 Selection(void) const;
	PS_RecordSelector(CWnd* pParent, PS_Database * db, CDWordArray * masks);   // standard constructor

// Dialog Data
	//{{AFX_DATA(PS_RecordSelector)
	enum { IDD = IDD_RECORD_SELECTOR };
	CComboBox	m_RecordList;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(PS_RecordSelector)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(PS_RecordSelector)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnDblclkRecordlist();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	S32 m_Selection;
	CDWordArray * m_MaskArray;
	PS_Database * m_DB;
};
