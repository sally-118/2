
#include "stdafx.h"
#include "gman.h"
#include "NetComm.h"
#include "XMLParse.h"
#include "ProcessDBEvent.h"

TCHAR szServiceName[] = _T("DBTransfer");

volatile DWORD g_status=0;
char g_DataSharePath[MAX_PATH];
char g_PictSharePath[MAX_PATH];
CStringArray gCodeArr;
char gKindStr[8][4]={"jpg","mo2","tn1","tn2"};
CRITICAL_SECTION gDLLcs;

CString	WINAPI	GetAppDirPath()
{
	static CString g_appPath;
	if (g_appPath.IsEmpty())
	{
		CString	strPath;
		LPTSTR	lpPath = strPath.GetBuffer(MAX_PATH);
		if(::GetModuleFileName(NULL,lpPath,MAX_PATH) > 0)
		{
			strPath.ReleaseBuffer();
			int	nPos = strPath.ReverseFind(_T('\\'));
			strPath.Delete(nPos,strPath.GetLength() - nPos);
		} else {
			strPath.ReleaseBuffer();
			strPath = _T(".");
		}
		g_appPath=strPath;
	}
	return	g_appPath;
}

#define MAXLOGSIZE	(1024L*1024)
void WriteLogFile(const char *string)
{
	static bool enter=false;
	char fileName[FILENAME_MAX];
	HANDLE hFile =INVALID_HANDLE_VALUE;
	char *buf=NULL;
	try{
		if(enter) Sleep(400);
		enter=true;
		DWORD writeByte=strlen(string);
		buf=(char *)malloc(writeByte+MAX_PATH);
		strcpy(buf, ::GetAppDirPath());
		//	GetWindowsDirectory(buf, FILENAME_MAX);
		sprintf(fileName, "%s\\%s.log", buf, szServiceName);
		hFile = ::CreateFile(fileName, GENERIC_READ|GENERIC_WRITE, 0, NULL,
			OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if(hFile!=INVALID_HANDLE_VALUE)
		{
			SYSTEMTIME sysTime;
			GetLocalTime(&sysTime);
			sprintf(buf, "%d-%d-%d %02d:%02d:%02d %s\r\n", sysTime.wYear, sysTime.wMonth, sysTime.wDay,
				sysTime.wHour, sysTime.wMinute, sysTime.wSecond, string);
			writeByte= ::SetFilePointer(hFile, 0, NULL, FILE_END);
			if (writeByte>MAXLOGSIZE*5) {
				BYTE *readBuf= (BYTE *)malloc(MAXLOGSIZE);
				SetFilePointer(hFile, writeByte-MAXLOGSIZE, NULL, FILE_BEGIN);
				ReadFile(hFile, readBuf, MAXLOGSIZE, &writeByte, NULL);
				SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
				WriteFile(hFile, readBuf, MAXLOGSIZE, &writeByte, NULL);
				SetEndOfFile(hFile);
				free(readBuf);
			}
			WriteFile(hFile, buf, strlen(buf), &writeByte, NULL);
			CloseHandle(hFile);
		}
	}
	catch (...) {
		if(hFile!=INVALID_HANDLE_VALUE) CloseHandle(hFile);
	}
	if(buf) free(buf);
	enter=false;
}

void GetServiceParamString(CString strItem, char* outBuf)
{
	CString fileName;
	fileName.Format("%s\\%s.ini", GetAppDirPath(), szServiceName);
	GetPrivateProfileString(BDParamStr, strItem, _T(""), outBuf, MAX_PATH, fileName);
}

void GetRunParamString(int which, CString strItem, char* outBuf)
{
	CString fileName;
	fileName.Format("%s\\%s.ini", GetAppDirPath(), szServiceName);
	CString section;
	section.Format("%s%d", RUNParamStr, which);
	GetPrivateProfileString(section, strItem, _T(""), outBuf, MAX_PATH, fileName);
}

int GetRunParamInt(int which, CString strItem, int defaultV)
{
	CString fileName;
	fileName.Format("%s\\%s.ini", GetAppDirPath(), szServiceName);
	CString section;
	section.Format("%s%d", RUNParamStr, which);
	return GetPrivateProfileInt(section, strItem, defaultV, fileName);
}

DWORD WINAPI ThreadListen(LPVOID lpParam)
{
 	SOCKET sockListen;
	SOCKADDR_IN sin;
	int port=6060;
	CString teStr;
	BOOL listenSuc=FALSE;
	SCODE	sc = ::OleInitialize(NULL);
// 	do 
// 	{
		port ++;
		sockListen= socket(AF_INET, SOCK_STREAM, 0);
		sin.sin_family = AF_INET;
		sin.sin_addr.s_addr = 0;
		sin.sin_port = htons(port);

		if(bind(sockListen, (SOCKADDR*)&sin, sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
		{
			closesocket(sockListen);
			sockListen=INVALID_SOCKET;
		}
		if (sockListen!=INVALID_SOCKET) 
		{			
			if(listen(sockListen, 200) == SOCKET_ERROR)
			{
				closesocket(sockListen);
				sockListen=INVALID_SOCKET;
			}
			else listenSuc=TRUE;
		}
//	} while(port<145 && !listenSuc);

	if (listenSuc==FALSE) g_status|=GEND;
	teStr.Format("Listen %d is %s.", port, listenSuc?"Suc!":"Failed!");
	WriteLogFile(teStr);

	fd_set fdListen;
	timeval seltime;
	seltime.tv_sec = 0;
	seltime.tv_usec = 2;

	DisposeAllConnect(TRUE);
	if(sockListen!=INVALID_SOCKET) 
	{
 		long teTC=GetTickCount();
//		while((g_status&GEND)==0)
		while ( 1 )
		{
			if (GetTickCount()-teTC>600000L)
			{
				DisposeAllConnect(FALSE);
				teTC=GetTickCount();
			}
			FD_ZERO(&fdListen);
			FD_SET(sockListen, &fdListen);
			if(select(0, &fdListen, NULL, NULL, &seltime)>0)
			{
				if (FD_ISSET(sockListen, &fdListen))
				{
					int len = sizeof(SOCKADDR_IN);
					int idleCon=GetFreeConnect();
					if (idleCon>=0)
					{
						SOCKET sock=accept(sockListen, (SOCKADDR*)&sin, &len);
						if (sock!=INVALID_SOCKET)
						{
							DWORD id;
							DWORD *pparam=new DWORD[2];
							pparam[0]=sock;
							pparam[1]=idleCon;
							HANDLE h = CreateThread(NULL, 0, RecvThread, (void *)pparam, 0, NULL);
							CloseHandle(h);
						}
						else FreeConnect(idleCon);
 					}
 				}
 			}
 		}
		DisposeAllConnect(TRUE);
		closesocket(sockListen);
		sockListen=INVALID_SOCKET;
 	}
	::OleUninitialize();

	return 1;
}

DWORD WINAPI RecvThread(LPVOID lpParam)
{
	CString teLog;
	int	nRet = 0;
	TRACE(_T("RecvThread\n"));
	if((g_status&GEND)||lpParam == NULL) return -1;

	DWORD *pte=(DWORD*)lpParam;
	SOCKET sock=pte[0];
	int freeDBID=pte[1];
	delete []pte;

	SCODE	sc = ::OleInitialize(NULL);
	if(FAILED(sc)) {
		WriteLogFile("RecvThread:OLEINITIAL FAILED!");
		return -1;
	}

	int revS;
	SuperCMD psupercmd;
	try{
		fd_set fdRecv;
		timeval seltime;
		seltime.tv_sec = 0;
		seltime.tv_usec = 15000;
		DWORD t1=GetTickCount();
		BOOL selectIt=FALSE;
		do {
			FD_ZERO(&fdRecv);
			FD_SET(sock, &fdRecv);
			selectIt=select(0, &fdRecv, NULL, NULL, &seltime)>0 && FD_ISSET(sock, &fdRecv);
			if(GetTickCount()-t1>5000L||selectIt) break;
			Sleep(50);
		} while(!selectIt);
		if(!selectIt) WriteLogFile("Socket Error 0");

		DWORD val=1;
		nRet= setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char *)&val, sizeof(val) );
		if(nRet<0) WriteLogFile(_T("setscokopt ERROR!"));
		val=20000;
		setsockopt(sock,SOL_SOCKET,SO_RCVTIMEO,(char *)&val,sizeof(val));
		setsockopt(sock,SOL_SOCKET,SO_SNDTIMEO,(char *)&val,sizeof(val));
		memset(&psupercmd, 0, sizeof(SuperCMD));

		int recvSize = sizeof(SuperCMD);
		char *buf=(char *)&psupercmd;
		int remainSize=recvSize;
		int teSize=0;
		do {
			teSize=min(4*1024L, remainSize);
			teSize=recv(sock, buf, teSize, 0);
			if(teSize<=0) { revS = recvSize; break; }
			remainSize-=teSize;
			buf+=teSize;
		} while(remainSize>0);
		revS = recvSize;

		if(revS != SOCKET_ERROR && psupercmd.cmdID !=0)
		{
			int i;
			if (psupercmd.cmdID!=RUNSQL&&psupercmd.cmdID!=RUNSQL2&&UPDATEBINV!=psupercmd.cmdID)
				FreeConnect(freeDBID);
			switch(psupercmd.cmdID)
			{
			case CHECKSRV:
				{
					BOOL teSuc=TRUE;
					for (i=0; i<10&&teSuc; i++) teSuc=psupercmd.super[i]+i==2222;
					psupercmd.cmdID=teSuc?SCMDOK:SCMDCANCEL;

					int sendSize = sizeof(SuperCMD);
					int nLeft, nWritten;
					PBYTE pBuf = (PBYTE)&psupercmd;
					nLeft = sendSize;
					while (nLeft > 0)
					{
						nWritten=min(4*1024L, nLeft);
						nWritten = send(sock, (const char *)pBuf, nWritten, 0);
						if (nWritten == SOCKET_ERROR) break;
						nLeft -= nWritten;
						pBuf += nWritten;
						if(nWritten == 0) break;
					}
				}
				break;
			case RUNSQL2:
				if (psupercmd.data>0 && psupercmd.data<ONE_M)
				{
					BOOL suc2=FALSE;
					char *sqlBuf=(char *)MyAlloc(psupercmd.data);
					revS=RECVALL(sock, sqlBuf, psupercmd.data, 0);
					// 如果获取的字节数和结构体预先声明的一样;
					if (revS==psupercmd.data)
					{
						_ConnectionPtr teCon=NULL;
						if (psupercmd.super[0]>0)
 						{
							FreeConnect(freeDBID);
							char *lianjieStr=(char *)MyAlloc(psupercmd.super[0]);
							revS=RECVALL(sock, lianjieStr, psupercmd.super[0], 0);
							if (revS==psupercmd.super[0])
							{
//								WriteLogFile(lianjieStr);
								CString ConStr=lianjieStr;
								if ( ConStr.Find("XCKYString")==0 ) 
									teCon=ConnectXCKYDatabase();
								else if ( ConStr.Find("CSIFMISString")==0 ) 
									teCon=ConnectCSIFMISDatabase();
								else 
									teCon=ConnectDataBase(lianjieStr);
							}
							MyFree((BYTE *)lianjieStr);
						}
						else teCon=GetDBConnectPtr(freeDBID);
						if (teCon)
						{
							CStringArray fieldsNameArr;
							_variant_t recArr;
							int reV=RunSql2SA(sqlBuf, teCon, fieldsNameArr, recArr);
							if (freeDBID<0) DisconnectDataBase(teCon);
							else FreeConnect(freeDBID);
							psupercmd.cmdID=SCMDNAME;
							psupercmd.data=fieldsNameArr.GetSize();
							psupercmd.super[0]=reV;
							suc2=reV>=0;
							if (suc2&&psupercmd.data>0)
							{
								char fieldName[128];
								SENDALL(sock, &psupercmd, sizeof(SuperCMD), 0);
								for (i=0; i<psupercmd.data; i++)
								{
									memset(fieldName, 0 , 128);
									CString teS=fieldsNameArr.GetAt(i);
									int opS=min(teS.GetLength(), 127);
									memcpy(fieldName, teS, opS);
									SENDALL(sock, fieldName, 128, 0);
								}
								if (reV>0) suc2=SendSA(sock, recArr, psupercmd.super[1]==0);
								else suc2=TRUE;
								teLog+=_T("  SendSA Suc!");
							}
 						}
 					}
					psupercmd.cmdID=suc2?SCMDOK:SCMDCANCEL;
					SENDALL(sock, &psupercmd, sizeof(SuperCMD), 0);
					MyFree((BYTE *)sqlBuf);
				}
				break;
			case RUNSQL:
				if (psupercmd.data>0 && psupercmd.data<ONE_M)
				{
					WriteLogFile("RunSQL!");
					char *sqlBuf=(char *)MyAlloc(psupercmd.data);
					revS=RECVALL(sock, sqlBuf, psupercmd.data, 0);
					if (revS==psupercmd.data)
					{
						_ConnectionPtr teCon=GetDBConnectPtr(freeDBID);
						if (teCon)
						{
							MystrBuf teStrBuf;
							memset(&teStrBuf, 0 ,sizeof(MystrBuf));
							int reV=RunSqlAndGetXML(teStrBuf, sqlBuf, teCon);
							if (reV>=0)
							{
								psupercmd.cmdID=SCMDOK;
								psupercmd.data=teStrBuf.m_strLen;
								SENDALL(sock, &psupercmd, sizeof(SuperCMD), 0);
								if (teStrBuf.m_strLen>0) revS=SENDALL(sock, teStrBuf.m_strBuf, teStrBuf.m_strLen, 0);
							}
							FreeConnect(freeDBID);
							MyFreeStr(&teStrBuf);
						}
					}
					if (psupercmd.cmdID!=SCMDOK)
					{
						psupercmd.cmdID=SCMDCANCEL;
						SENDALL(sock, &psupercmd, sizeof(SuperCMD), 0);
					}
					MyFree((BYTE *)sqlBuf);
				}
				break;
			case UPDATEBINV:
				if (psupercmd.data>0 &&psupercmd.data<ONE_M)
				{
					BOOL suc1= FALSE;
					BOOL suc2=FALSE;
					char *sqlBuf=(char *)MyAlloc(psupercmd.data);
					revS=RECVALL(sock, sqlBuf, psupercmd.data, 0);
					if (revS==psupercmd.data)
					{
						_ConnectionPtr teCon=NULL;
						if (psupercmd.super[1]>0)
						{
							FreeConnect(freeDBID);
							char *lianjieStr=(char *)MyAlloc(psupercmd.super[1]);
							revS=RECVALL(sock, lianjieStr, psupercmd.super[1], 0);
							if (revS==psupercmd.super[1])
							{
								WriteLogFile(lianjieStr);
								CString ConStr=lianjieStr;
								if ( ConStr.Find("XCKYString")==0 ) 
									teCon=ConnectXCKYDatabase();
								else if ( ConStr.Find("CSIFMISString")==0 ) 
									teCon=ConnectCSIFMISDatabase();
								else 
									teCon=ConnectDataBase(lianjieStr);
							}
							MyFree((BYTE *)lianjieStr);
						}
						else teCon=GetDBConnectPtr(freeDBID);

						if (teCon)
						{
							BYTE *binV=NULL;
							if (psupercmd.super[0]>0)
							{
								binV=MyAlloc(psupercmd.super[0]);
								revS=RECVALL(sock, binV, psupercmd.super[0], 0);
								if (revS==psupercmd.super[0])
								{
									suc2=TRUE;
								}
								
							}
							suc2=UpdateBinValue(teCon, sqlBuf, binV, psupercmd.super[0]);
							MyFree(binV);
						}
					}

					FreeConnect(freeDBID);
					MyFree((BYTE *)sqlBuf);
					psupercmd.cmdID=suc2?SCMDOK:SCMDCANCEL;
					SENDALL(sock, &psupercmd, sizeof(SuperCMD), 0);
				}
				break;
			default:
				break;
			}
		}
		else WriteLogFile("Recv CMD error!");
	}
	catch(...)
	{
		teLog+=_T(" Socket Exception!");
		WriteLogFile(teLog);
	}
	FreeConnect(freeDBID);
	closesocket(sock);
	::OleUninitialize();

	return 0;
}
