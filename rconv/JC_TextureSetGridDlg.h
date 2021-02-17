//-------------------------------------------------------------------------------------------------------------------------
#pragma once
#include "JCClassResource.h"
//-------------------------------------------------------------------------------------------------------------------------
class JC_TextureSetGridDlg : public CDialog
{
public:
	enum // Grid types.
	{
		FixedNone,
		Fixed8x8,
		Fixed8x16,
		Fixed16x8,
		Fixed16x16,
		Custom,
	};

public:
	JC_TextureSetGridDlg(CSize custom, int type, BOOL snap, BOOL showsnap, BOOL showpixel, CWnd * parent=NULL);

	int m_Type;
	CSize m_Custom;
	//{{AFX_DATA(JC_TextureSetGridDlg)
	enum { IDD = IDD_TSE_GRID };
	BOOL	m_Snap;
	BOOL	m_ShowSnap;
	BOOL	m_ShowPixel;
	CButton m_Radio1;
	CButton m_Radio2;
	CButton m_Radio3;
	CButton m_Radio4;
	CButton m_Radio5;
	CButton m_Radio6;
	CString	m_CustomX;
	CString	m_CustomY;
	//}}AFX_DATA

	//{{AFX_VIRTUAL(JC_TextureSetGridDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	//}}AFX_VIRTUAL

protected:
	void SetType(int type);
	void UpdateEnable(void);
	BOOL OnInitDialog(void);
	//{{AFX_MSG(JC_TextureSetGridDlg)
	afx_msg void OnGrid8x8();
	afx_msg void OnGrid8x16();
	afx_msg void OnGrid16x8();
	afx_msg void OnGrid16x16();
	afx_msg void OnNogrid();
	afx_msg void OnGridCustomsize();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
//-------------------------------------------------------------------------------------------------------------------------
