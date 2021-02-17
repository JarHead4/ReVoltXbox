#ifndef logdialog_h
#define logdialog_h

// LogDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// LogDialog dialog

class LogDialog : public CDialog
{
private:
	CRichEditCtrl * m_Edit;

// Construction
public:
	LogDialog(CRichEditCtrl * edit, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(LogDialog)
	enum { IDD = IDD_LOGDIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(LogDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(LogDialog)
	afx_msg void OnPrint();
	afx_msg void OnFile();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
