/////////////////////////////////////////////////////////////////////////////
// This file is part of the completely free tetris clone "CGTetris".
//
// This is free software.
// You may redistribute it by any means providing it is not sold for profit
// without the authors written consent.
//
// No warrantee of any kind, expressed or implied, is included with this
// software; use at your own risk, responsibility for damages (if any) to
// anyone resulting from the use of this software rests entirely with the
// user.
/////////////////////////////////////////////////////////////////////////////


#if !defined(AFX_PROPSHEETTAB_H__48800E43_19AF_11D2_A741_000000000000__INCLUDED_)
#define AFX_PROPSHEETTAB_H__48800E43_19AF_11D2_A741_000000000000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// PropSheetTab.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPropSheetTab window

class CPropSheetTab : public CTabCtrl
{
// Construction
public:
	CPropSheetTab();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPropSheetTab)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CPropSheetTab();

	// Generated message map functions
protected:
	//{{AFX_MSG(CPropSheetTab)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROPSHEETTAB_H__48800E43_19AF_11D2_A741_000000000000__INCLUDED_)
