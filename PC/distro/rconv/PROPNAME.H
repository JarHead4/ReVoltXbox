// PropName.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// PropName dialog

class PropName : public CDialog
{
// Construction
public:
	PropName(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(PropName)
	enum { IDD = IDD_PROPNAME };
	CComboBox	m_DefaultType;
	CString	m_Name;
	CString	m_DefaultValue;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(PropName)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(PropName)
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
