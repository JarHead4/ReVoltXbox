//-------------------------------------------------------------------------------------------------------------------------------
#include "stdafx.h"
#include "HabExtract.h"
#include "LogDialog.h"
//-------------------------------------------------------------------------------------------------------------------------------
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
//-------------------------------------------------------------------------------------------------------------------------------
LogDialog::LogDialog(CRichEditCtrl * edit, CWnd* pParent /*=NULL*/)
	: CDialog(LogDialog::IDD, pParent)
{
	m_Edit = edit;

	//{{AFX_DATA_INIT(LogDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}
//-------------------------------------------------------------------------------------------------------------------------------
void LogDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(LogDialog)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}
//-------------------------------------------------------------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(LogDialog, CDialog)
	//{{AFX_MSG_MAP(LogDialog)
	ON_BN_CLICKED(IDPRINT, OnPrint)
	ON_BN_CLICKED(IDFILE, OnFile)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()
//-------------------------------------------------------------------------------------------------------------------------------
void LogDialog::OnPrint(void)
{
	CPrintDialog d(TRUE);

	if (d.DoModal() == IDOK)
	{
		HDC hDC = d.CreatePrinterDC();
		CDC * pdc = CDC::FromHandle(hDC);
		pdc->m_bPrinting = TRUE;

		DOCINFO di;
		di.lpszOutput = NULL;
		di.cbSize = sizeof(DOCINFO);
		di.lpszDocName = "Habitat Extract";

		BOOL res = pdc->StartDoc(&di);

		if (res < 0)
		{
			AfxMessageBox("Print command failed");
		}
		else
		{
			pdc->StartPage();

			for (int i = 0; i < m_Edit->GetLineCount(); i++)
			{
				char buf[256];
				int len = m_Edit->GetLine(i, buf, 250);
				buf[len++] = 0;
				pdc->TextOut(0, i*64, CString(buf));		// 64????
			}

			pdc->EndPage();
			pdc->EndDoc();
		}
	}
}
//-------------------------------------------------------------------------------------------------------------------------------
void LogDialog::OnFile(void)
{
	CFileDialog d(FALSE, NULL, "HabExtract.log", NULL, "*.log");

	if (d.DoModal() == IDOK)
	{
		TRY
		{
			CString path = d.GetPathName();

			CFile f((LPCSTR)path, CFile::modeCreate|CFile::modeWrite|CFile::shareExclusive);

			for (int i = 0; i < m_Edit->GetLineCount(); i++)
			{
				char buf[256];
				int len = m_Edit->GetLine(i, buf, 250);
				buf[len++] = 0x0D;
				buf[len++] = 0x0A;
				f.Write(buf, len);
			}

			EndDialog(IDOK);
		}

		CATCH(CFileException, e)
		{
			AfxMessageBox("File write operation failed");
		}

		END_CATCH
	}
}
//-------------------------------------------------------------------------------------------------------------------------------
