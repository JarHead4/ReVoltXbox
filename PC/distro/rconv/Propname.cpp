//-------------------------------------------------------------------------------------------------
#include "stdafx.h"
#include "HabExtract.h"
#include "PropName.h"
//-------------------------------------------------------------------------------------------------
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
//-------------------------------------------------------------------------------------------------
PropName::PropName(CWnd* pParent /*=NULL*/)
	: CDialog(PropName::IDD, pParent)
{
	//{{AFX_DATA_INIT(PropName)
	m_Name = _T("");
	m_DefaultValue = _T("");
	//}}AFX_DATA_INIT
}
//-------------------------------------------------------------------------------------------------
void PropName::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(PropName)
	DDX_Control(pDX, IDC_COMBO1, m_DefaultType);
	DDX_Text(pDX, IDC_EDIT1, m_Name);
	DDX_Text(pDX, IDC_EDIT2, m_DefaultValue);
	//}}AFX_DATA_MAP
}
//-------------------------------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(PropName, CDialog)
	//{{AFX_MSG_MAP(PropName)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()
//-------------------------------------------------------------------------------------------------
void PropName::OnOK(void)
{
	UpdateData();

	if (m_Name.IsEmpty())
	{
		AfxMessageBox("You must provide a name for the property");
	}
	else
	{
		JC_Data * defval = NULL;
		JC_String name = m_Name;
		JC_String val  = m_DefaultValue;

		TRY
		{
			switch (m_DefaultType.GetCurSel())		// The order is set in the dialog editor
			{
				case 0:		defval = new JC_String(val);		break;
				case 1:		defval = new JC_Bool(val);			break;
				case 2:		defval = new JC_S8(val);			break;
				case 3:		defval = new JC_S16(val);			break;
				case 4:		defval = new JC_S32(val);			break;
				case 5:		defval = new JC_U8(val);			break;
				case 6:		defval = new JC_U16(val);			break;
				case 7:		defval = new JC_U32(val);			break;
				case 8:		defval = new JC_Float(val);			break;
				case 9:		defval = new JC_Double(val);		break;

				default:	AfxMessageBox("You must provide a default type for the property");
			}

			if (defval != NULL)
			{
				PS_Database * db = &api.Database();
				JC_PropertyName * pn = (JC_PropertyName*)db->New(name, JC_Record::TypePropertyName);

				pn->Default(defval);
				db->Put(pn->Number());

				EndDialog(IDOK);
			}
		}

		CATCH(JC_Exception, e)
		{
			AfxMessageBox((LPCSTR)e->Message());
		}

		END_CATCH
	}
}
//-------------------------------------------------------------------------------------------------
