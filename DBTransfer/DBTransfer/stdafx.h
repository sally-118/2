// stdafx.h : 标准系统包含文件的包含文件;

#if !defined(AFX_STDAFX_H__46983218_1831_4118_9283_9AFEB84DCDC7__INCLUDED_)
#define AFX_STDAFX_H__46983218_1831_4118_9283_9AFEB84DCDC7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//#include <SDKDDKVer.h>

#define WIN32_LEAN_AND_MEAN             //  从 Windows 头文件中排除极少使用的信息;


// Windows 头文件;
// 已经包含在MFC头文件中;
//#include <windows.h>


// MFC 头文件;
#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC Automation classes
#include <afxsock.h>
#include <afxmt.h>

#include <math.h>
#include <winsvc.h>     // SERVICE_STATUS_HANDLE
#include <Imm.h>


// C 运行时头文件;
#include <stdio.h>


// 程序需要的其他头文件;
#include "BaseDef.h"

#import "msado15.dll" no_namespace rename("EOF","adoEOF") rename("BOF","adoBOF")


#endif // !defined(AFX_STDAFX_H__46983218_1831_4118_9283_9AFEB84DCDC6__INCLUDED_)