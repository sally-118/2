// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ�;

#if !defined(AFX_STDAFX_H__46983218_1831_4118_9283_9AFEB84DCDC7__INCLUDED_)
#define AFX_STDAFX_H__46983218_1831_4118_9283_9AFEB84DCDC7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//#include <SDKDDKVer.h>

#define WIN32_LEAN_AND_MEAN             //  �� Windows ͷ�ļ����ų�����ʹ�õ���Ϣ;


// Windows ͷ�ļ�;
// �Ѿ�������MFCͷ�ļ���;
//#include <windows.h>


// MFC ͷ�ļ�;
#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC Automation classes
#include <afxsock.h>
#include <afxmt.h>

#include <math.h>
#include <winsvc.h>     // SERVICE_STATUS_HANDLE
#include <Imm.h>


// C ����ʱͷ�ļ�;
#include <stdio.h>


// ������Ҫ������ͷ�ļ�;
#include "BaseDef.h"

#import "msado15.dll" no_namespace rename("EOF","adoEOF") rename("BOF","adoBOF")


#endif // !defined(AFX_STDAFX_H__46983218_1831_4118_9283_9AFEB84DCDC6__INCLUDED_)