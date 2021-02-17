//-------------------------------------------------------------------------------------------------
#include "stdafx.h"
#include "HabExtract.h"
#include "PropName.h"
#include "LogDialog.h"
#include "HelpDialog.h"
#include "HabExtractDlg.h"
//-------------------------------------------------------------------------------------------------
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
//-------------------------------------------------------------------------------------------------
class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
//-------------------------------------------------------------------------------------------------
CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}
//-------------------------------------------------------------------------------------------------
void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}
//-------------------------------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()
//-------------------------------------------------------------------------------------------------
BOOL CHabExtractDlg::m_CloseFlag = FALSE;
//-------------------------------------------------------------------------------------------------
CHabExtractDlg::CHabExtractDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CHabExtractDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CHabExtractDlg)
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_Percent = 0;
	m_CloseFlag = FALSE;
}
//-------------------------------------------------------------------------------------------------
void CHabExtractDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CHabExtractDlg)
	DDX_Control(pDX, IDHELP, m_Help);
	DDX_Control(pDX, IDCANCEL, m_Cancel);
	DDX_Control(pDX, IDBEGIN, m_Begin);
	DDX_Control(pDX, IDC_PROGRESS1, m_Progress);
	//}}AFX_DATA_MAP
}
//-------------------------------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(CHabExtractDlg, CDialog)
	//{{AFX_MSG_MAP(CHabExtractDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDBEGIN, OnBegin)
	ON_BN_CLICKED(IDHELP, OnHelp)
	ON_BN_CLICKED(IDNEWPROP, OnNewprop)
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()
//-------------------------------------------------------------------------------------------------
BOOL CHabExtractDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_Edit.Attach(*GetDlgItem(IDC_EDIT1));
	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	CString strAboutMenu;
	strAboutMenu.LoadString(IDS_ABOUTBOX);
	if (!strAboutMenu.IsEmpty())
	{
		pSysMenu->AppendMenu(MF_SEPARATOR);
		pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	Init();

	return FALSE; // TRUE;  // return TRUE  unless you set the focus to a control
}
//-------------------------------------------------------------------------------------------------
void CHabExtractDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}
//-------------------------------------------------------------------------------------------------
// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.
//-------------------------------------------------------------------------------------------------
void CHabExtractDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}
//-------------------------------------------------------------------------------------------------
// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
//-------------------------------------------------------------------------------------------------
HCURSOR CHabExtractDlg::OnQueryDragIcon(void)
{
	return (HCURSOR)m_hIcon;
}
//-------------------------------------------------------------------------------------------------
void CHabExtractDlg::OnCancel(void)
{
	m_CloseFlag = TRUE;

	DestroyWindow();
}
//-------------------------------------------------------------------------------------------------
void CHabExtractDlg::Display(int id)
{
	CString s;
	s.LoadString(id);
	Display(s);
}
//-------------------------------------------------------------------------------------------------
void CHabExtractDlg::Display(const CString& string)
{
	Display((LPCSTR)string);
}
//-------------------------------------------------------------------------------------------------
void CHabExtractDlg::Display(const char * buf)
{
	if (m_CloseFlag) return;

	char editbuf[256];

	int j = 0;

	for (int i = 0; i < 256; i++)
	{
		char c = buf[i];

		if (c == '\n')
		{
			editbuf[j++] = '\r';
		}

		editbuf[j++] = c;

		if (c == 0) break;
	}

	long length = m_Edit.GetTextLength();
	m_Edit.SetSel(length, length);
	m_Edit.ReplaceSel(editbuf);
	m_Edit.SendMessage(EM_SCROLLCARET, 0, 0);
}
//-------------------------------------------------------------------------------------------------
void CHabExtractDlg::Init(void)
{
	m_Mode = ModeBegin;

	m_Begin.SetWindowText("&Begin");
	m_Cancel.SetWindowText("Close");
}
//-------------------------------------------------------------------------------------------------
void CHabExtractDlg::Final(void)
{
	m_Mode = ModeSaveLog;

	m_Begin.SetWindowText("&Save Log");
	m_Cancel.SetWindowText("Close");
	
	GotoDlgCtrl(&m_Cancel);
}
//-------------------------------------------------------------------------------------------------
void theUserProgram(void);
//-------------------------------------------------------------------------------------------------
BOOL CHabExtractDlg::MainProcessing(void)
{
	TRY
	{
		theUserProgram();
	}

	CATCH(JC_Exception, e)
	{
		api.Log(e->Message() + "\n");

		AfxMessageBox((LPCSTR)e->Message());
	}

	END_CATCH

	if (!m_CloseFlag)
	{
		api.Update(100);
		api.Finish();

		return TRUE;
	}

	return FALSE;
}
//-------------------------------------------------------------------------------------------------
void CHabExtractDlg::RefreshProgress(void)
{
	UpdateProgress(m_Percent);
}
//-------------------------------------------------------------------------------------------------
void CHabExtractDlg::ShowProgress(const char * format, double percent)
{
	CString message;
	message.Format(format, percent);
	SetWindowText(message);
	m_Progress.SetPos((int)percent);
}
//-------------------------------------------------------------------------------------------------
void CHabExtractDlg::UpdateProgress(int percent)
{
	if (m_CloseFlag) return;

	char buf[256];

	char * str = "complete";

	if (api.IsPaused())
	{
		str = "- Paused";
	}

	sprintf(buf, "%3d%% %s", percent, str);
	SetWindowText(buf);

	m_Progress.SetPos(percent);

	m_Percent = percent;
}
//-------------------------------------------------------------------------------------------------
void CHabExtractDlg::StartRun(void)
{
	m_Begin.SetWindowText("&Pause");
	m_Cancel.SetWindowText("Abort");
	m_Help.EnableWindow(FALSE);

	m_Mode = ModePause;

	api.Go();

	m_Percent = 0;
	RefreshProgress();
}
//-------------------------------------------------------------------------------------------------
void CHabExtractDlg::OnBegin(void)
{
	switch(m_Mode)
	{
		case ModeBegin:		StartRun();
							break;

		case ModePause:		m_Begin.SetWindowText("&Resume");
							m_Mode = ModeResume;
							api.IsPaused(TRUE);
							RefreshProgress();
							break;

		case ModeResume:	m_Begin.SetWindowText("&Pause");
							m_Mode = ModePause;
							api.IsPaused(FALSE);
							break;

		case ModeSaveLog:	LogDialog d(&m_Edit);
							d.DoModal();
							break;
	}
}
//-------------------------------------------------------------------------------------------------
void CHabExtractDlg::OnHelp(void)
{
	HelpDialog d;

	d.DoModal();
}
//-------------------------------------------------------------------------------------------------
void CHabExtractDlg::OnNewprop(void)
{
	PropName d;

	d.DoModal();
}
//-------------------------------------------------------------------------------------------------

CHabExtractDlg::~CHabExtractDlg()
{
	m_Edit.Detach();
}

int CHabExtractDlg::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// TODO: Add your specialized creation code here
	
	return 0;
}
