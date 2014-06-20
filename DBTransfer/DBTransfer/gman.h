#pragma once


#define  GEND			0x800000
#define  GJOBCANCEL		0x400000
#define  GREBUILDSRC	0x80

#define BDParamStr _T("General")
#define RUNParamStr _T("RunParam")

extern TCHAR szServiceName[];

extern volatile DWORD g_status;
extern char g_DataSharePath[MAX_PATH];
extern char g_PictSharePath[MAX_PATH];
extern CRITICAL_SECTION gDLLcs;


CString	WINAPI	GetAppDirPath();
void WriteLogFile(const char *string);

DWORD WINAPI ThreadListen(LPVOID lpParam);
DWORD WINAPI RecvThread(LPVOID lpParam);

void GetServiceParamString(CString strItem, char* outBuf);
void GetRunParamString(int which, CString strItem, char* outBuf);
int GetRunParamInt(int which, CString strItem, int defaultV);


inline BYTE* MyAlloc(DWORD size)
{
	size+=64;
	BYTE* te=(BYTE*)malloc(size);
	if(te)memset(te, 0, size);
	return te;
}

inline void MyFree(BYTE* ptr)
{
	if (ptr) free(ptr);
}