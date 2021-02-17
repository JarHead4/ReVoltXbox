#ifndef habextractdlg_h
#define habextractdlg_h

class CHabExtractApp;

class CHabExtractDlg : public CDialog
{
private:
	static BOOL m_CloseFlag;
	enum { ModeBegin, ModePause, ModeResume, ModeSaveLog };

private:
	int m_Mode;
	int m_Percent;

public:
	virtual ~CHabExtractDlg();
	CHabExtractDlg(CWnd* pParent = NULL);	// standard constructor

	void Init(void);
	void Final(void);
	void OnCancel(void);
	void StartRun(void);
	void Display(int id);
	void Display(const char * buf);
	void Display(const CString& string);
	void RefreshProgress(void);
	void UpdateProgress(int percent);
	void ShowProgress(const char * format, double percent);
	BOOL MainProcessing(void);

// Dialog Data
	//{{AFX_DATA(CHabExtractDlg)
	enum { IDD = IDD_HABEXTRACT_DIALOG };
	CButton	m_Help;
	CButton	m_Cancel;
	CButton	m_Begin;
	CProgressCtrl	m_Progress;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHabExtractDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CRichEditCtrl m_Edit;
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CHabExtractDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnBegin();
	afx_msg void OnHelp();
	afx_msg void OnNewprop();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
