// FontMakerMFC.h : main header file for the FONTMAKERMFC application
//

#if !defined(AFX_FONTMAKERMFC_H__D8CFDAC3_9C21_4916_919B_2AC2870DF20D__INCLUDED_)
#define AFX_FONTMAKERMFC_H__D8CFDAC3_9C21_4916_919B_2AC2870DF20D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CFontMakerMFCApp:
// See FontMakerMFC.cpp for the implementation of this class
//

class CFontMakerMFCApp : public CWinApp
{
public:
	CFontMakerMFCApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFontMakerMFCApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CFontMakerMFCApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FONTMAKERMFC_H__D8CFDAC3_9C21_4916_919B_2AC2870DF20D__INCLUDED_)
