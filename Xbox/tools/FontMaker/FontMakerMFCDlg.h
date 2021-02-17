// FontMakerMFCDlg.h : header file
//

#if !defined(AFX_FONTMAKERMFCDLG_H__D7C6A93E_7565_4080_BCE6_CCEDADCEB8C7__INCLUDED_)
#define AFX_FONTMAKERMFCDLG_H__D7C6A93E_7565_4080_BCE6_CCEDADCEB8C7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CTextureBitmap : public CWnd
{
// Construction
public:
	CTextureBitmap();

// Attributes
public:
	DWORD m_dwWidth;
	DWORD m_dwHeight;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTextureBitmap)
	//}}AFX_VIRTUAL

// Implementation
public:
	static BOOL RegisterWndClass(HINSTANCE hInstance);
	virtual ~CTextureBitmap();

	// Generated message map functions
protected:
	//{{AFX_MSG(CTextureBitmap)
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////
// CFontMakerMFCDlg dialog
class CFontMakerMFCDlg : public CDialog
{
protected:
	static BOOL RegisterWndClass(HINSTANCE hInstance);
	CTextureBitmap m_TextureBitmap; 

// Construction
public:
	CFontMakerMFCDlg(CWnd* pParent = NULL);	// standard constructor

	CComboBox* m_pFontNameComboBox;
	CComboBox* m_pFontStyleComboBox;
	CComboBox* m_pFontSizeComboBox;
	int        m_iCurrentFont;

	CFont* m_pFont;
	void CreateFont();
	
	CListCtrl* m_pGlyphList;
	VOID DrawGlyphs( CDC* pDC = NULL );
	VOID UpdateGlyphList();

// Dialog Data
	//{{AFX_DATA(CFontMakerMFCDlg)
	enum { IDD = IDD_FONTMAKER };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFontMakerMFCDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	void ExtractValidGlyphsFromRange();
	void ExtractValidGlyphsFromFile();
	
	// Generated message map functions
	//{{AFX_MSG(CFontMakerMFCDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnSelchangeFontname();
	afx_msg void OnEditupdateFontname();
	afx_msg void OnSelchangeFontstyle();
	afx_msg void OnSelchangeFontsize();
	afx_msg void OnKillfocusFontsize();
	afx_msg void OnItemclickGlyphList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnClickGlyphList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemchangedGlyphList(NMHDR* pNMHDR, LRESULT* pResult);
	virtual void OnOK();
	afx_msg void OnFixedRangeRadio();
	afx_msg void OnExtractFromFile();
	afx_msg void OnFileselector();
	afx_msg void OnKillfocusGlyphrangeFrom();
	afx_msg void OnKillfocusGlyphrangeTo();
	afx_msg void OnKillfocusGlyphfilename();
	afx_msg void OnKillfocusBitmapSize();
	afx_msg void OnEffects();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FONTMAKERMFCDLG_H__D7C6A93E_7565_4080_BCE6_CCEDADCEB8C7__INCLUDED_)
