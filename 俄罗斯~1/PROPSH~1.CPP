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


// PropSheetTab.cpp : implementation file
//

#include "stdafx.h"
#include "tetris.h"
#include "PropSheetTab.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPropSheetTab

CPropSheetTab::CPropSheetTab()
{
}

CPropSheetTab::~CPropSheetTab()
{
}


BEGIN_MESSAGE_MAP(CPropSheetTab, CTabCtrl)
	//{{AFX_MSG_MAP(CPropSheetTab)
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPropSheetTab message handlers

BOOL CPropSheetTab::OnEraseBkgnd(CDC* pDC) 
{
	// We only erase the tabs itself. The other areas of the
	// control shall be transparent ...
	const int nCnt = GetItemCount();
	for( register int i = 0 ; i<nCnt ; ++i ) {
		CRect rect;
		VERIFY(GetItemRect(i, rect));
		pDC->FillSolidRect(rect, ::GetSysColor(COLOR_BTNFACE));
	}
	return TRUE;
}
