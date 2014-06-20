
#include "stdafx.h"
#include "gman.h"
#include "XMLParse.h"


//定义全局函数变量;
void Init();
BOOL IsInstalled();
BOOL Install();
BOOL Uninstall();
void WINAPI ServiceMain();
void WINAPI ServiceStrl(DWORD dwOpcode);

BOOL bInstall;
SERVICE_STATUS_HANDLE hServiceStatus;
SERVICE_STATUS status;


int APIENTRY WinMain(HINSTANCE hInstance,
			HINSTANCE hPrevInstance,
			LPSTR     lpCmdLine,
			int       nCmdShow)
{

// 	Init();
// 
// 	SERVICE_TABLE_ENTRY st[] =
// 	{
// 		{ szServiceName, (LPSERVICE_MAIN_FUNCTION)ServiceMain },
// 		{ NULL, NULL }
// 	};
// 
// 	if (stricmp(lpCmdLine, "/install") == 0)
// 	{
// 		Install();
// 	}
// 	else if (stricmp(lpCmdLine, "/uninstall") == 0)
// 	{
// 		Uninstall();
// 	}
// 	else
	{
//		if (!::StartServiceCtrlDispatcher(st))
		{
//			WriteLogFile(_T("Register Service Main Function Error!"));
		}
	}

	ServiceMain();

	return 0;
}


void WINAPI ServiceMain()
{

	WriteLogFile(_T("Service Start!"));

	CString teLog;
	WSADATA   wsd;
	int nResult =WSAStartup(MAKEWORD(2,2),&wsd); 
	if (nResult != 0) WriteLogFile(_T("Socket Init Error"));
	else
	{
		SCODE sc = ::OleInitialize(NULL);
		if (sc!=S_OK) WriteLogFile(_T("OLE Init Error"));
		{
			InitializeCriticalSection(&gDLLcs);

			// 创建监听线程;
			HANDLE threadH=NULL;
			threadH=::CreateThread(NULL, 0, ThreadListen, NULL, 0, NULL);
			CloseHandle(threadH);

			DWORD loopCtrl=0;
//			SearchAndRunexe(1);
			do {
				try
				{
					if (g_status&GEND) break;
					Sleep(1000);
//					SearchAndRunexe(3);
					loopCtrl++;
				}
				catch (...)
				{
					WriteLogFile(_T("Main loop Exception!"));
				}
			} while((g_status&GEND)==0);
//			SearchAndRunexe(2);

			DeleteCriticalSection(&gDLLcs);
			::OleUninitialize();
		}
		WSACleanup();
	}

	WriteLogFile(_T("Service Exit!"));

}


void Init()
{
	hServiceStatus = NULL;
	status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	status.dwCurrentState = SERVICE_STOPPED;
	status.dwControlsAccepted = SERVICE_ACCEPT_STOP;
	status.dwWin32ExitCode = 0;
	status.dwServiceSpecificExitCode = 0;
	status.dwCheckPoint = 0;
	status.dwWaitHint = 0;
}

BOOL Install()
{

	if (IsInstalled())
		return TRUE;

	//打开服务控制管理器;
	SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (hSCM == NULL)
	{
		MessageBox(NULL, _T("Couldn't open service manager"), szServiceName, MB_OK);
		return FALSE;
	}

	// Get the executable file path
	TCHAR szFilePath[MAX_PATH];
	::GetModuleFileName(NULL, szFilePath, MAX_PATH);

	//创建服务;
	SC_HANDLE hService = ::CreateService(
		hSCM, szServiceName, szServiceName,
		SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS,
		SERVICE_AUTO_START, SERVICE_ERROR_NORMAL,
		szFilePath, NULL, NULL, _T(""), NULL, NULL);

	if (hService == NULL)
	{
		::CloseServiceHandle(hSCM);
		MessageBox(NULL, _T("Couldn't create service"), szServiceName, MB_OK);
		return FALSE;
	}
	// Set service description
	SERVICE_DESCRIPTION ServiceDesc;
	static TCHAR szDescription[MAX_PATH];
	_tcscpy(szDescription, _T("Database Transfer service"));
	ServiceDesc.lpDescription = szDescription;
	::ChangeServiceConfig2(hService, SERVICE_CONFIG_DESCRIPTION, &ServiceDesc);

	::CloseServiceHandle(hService);
	::CloseServiceHandle(hSCM);
	return TRUE;
}

BOOL IsInstalled()
{
	BOOL bResult = FALSE;

	//打开服务控制管理器;
	SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

	if (hSCM != NULL)
	{
		//打开服务;
		SC_HANDLE hService = ::OpenService(hSCM, szServiceName, SERVICE_QUERY_CONFIG);
		if (hService != NULL)
		{
			bResult = TRUE;
			::CloseServiceHandle(hService);
		}
		::CloseServiceHandle(hSCM);
	}
	return bResult;
}

BOOL Uninstall()
{
	WriteLogFile(_T("Execute Uninstall()"));

	if (!IsInstalled())
		return TRUE;

	SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

	if (hSCM == NULL)
	{
		MessageBox(NULL, _T("Couldn't open service manager"), szServiceName, MB_OK);
		return FALSE;
	}

	SC_HANDLE hService = ::OpenService(hSCM, szServiceName, SERVICE_STOP | DELETE);

	if (hService == NULL)
	{
		::CloseServiceHandle(hSCM);
		MessageBox(NULL, _T("Couldn't open service"), szServiceName, MB_OK);
		return FALSE;
	}
	SERVICE_STATUS status;
	::ControlService(hService, SERVICE_CONTROL_STOP, &status);

	//删除服务;
	BOOL bDelete = ::DeleteService(hService);
	::CloseServiceHandle(hService);
	::CloseServiceHandle(hSCM);

	if (bDelete)
		return TRUE;

	WriteLogFile(_T("Service could not be deleted"));
	return FALSE;
}