// PS_ProcessingDialog.h : header file
//
#ifndef _PS_PROCESSING_DIALOG_H_
#define _PS_PROCESSING_DIALOG_H_
#include "JCClassResource.h"
/////////////////////////////////////////////////////////////////////////////
// PS_ProcessingDialog dialog

class JCCLASS_DLL PS_ProcessingDialog : public CDialog
{
// Construction
public:
	void SetProcessTrigger(UINT message, WPARAM wparam, LPARAM lparam);
	~PS_ProcessingDialog();
	BOOL UpdateProgress(CString* caption, S32 percentage);
	BOOL HasCancelled(void);
	PS_ProcessingDialog(CWnd* pParent = NULL);   // standard constructor
	void SetCancelTimeout(DWORD timeout) { m_CancelTimeout = timeout; }
// Dialog Data
	//{{AFX_DATA(PS_ProcessingDialog)
	enum { IDD = IDD_PROCESSING };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(PS_ProcessingDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	afx_msg LRESULT OnUpdateProcess(WPARAM wparam, LPARAM lparam);
	afx_msg LRESULT OnCloseProcess(WPARAM wparam, LPARAM lparam);
	UINT m_StartMessage;
	WPARAM m_StartWParam;
	LPARAM m_StartLParam;
	DWORD m_CancelTimeout;
	DWORD m_StartCancelTime;
	BOOL ProcessWindowsMessages(void);
	CProgressCtrl m_Progress;
	CStatic m_Caption;
	BOOL m_HasCancelled;
	BOOL m_NotReady;
	CWnd m_CancelButton;
	// Generated message map functions
	//{{AFX_MSG(PS_ProcessingDialog)
	virtual void OnCancel();
	virtual BOOL OnInitDialog();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif