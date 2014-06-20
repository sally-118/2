#include "stdafx.h"

#include "GMan.h"
#include "ProcessDBEvent.h"
#include "netComm.h"
#include "XMLParse.h"

const DWORD WAITMAXTC=300000L;

void MyFreeStr(MystrBuf *opBuf)
{
	if (opBuf->m_strBuf) free(opBuf->m_strBuf);
	opBuf->m_strBuf=NULL;
	opBuf->m_allocLen=opBuf->m_strLen=0;
}

static void CheckXMLBuf(MystrBuf *opBuf, int newLen)
{
	if (opBuf->m_strLen+newLen+1>opBuf->m_allocLen) 
	{
		opBuf->m_allocLen=opBuf->m_strLen+newLen+1;
		opBuf->m_allocLen/=0X10000L;
		opBuf->m_allocLen=(opBuf->m_allocLen+1)*0X10000L;
		char *newStr=(char *)malloc(opBuf->m_allocLen);
		memset(newStr, 0, opBuf->m_allocLen);
		if (opBuf->m_strBuf)
		{
			memcpy(newStr, opBuf->m_strBuf, opBuf->m_strLen);
			free(opBuf->m_strBuf);
			opBuf->m_strBuf=NULL;
		}
		opBuf->m_strBuf=newStr;
	}
}
static VOID MyAddBuf(MystrBuf *opBuf, const char *addBuf, int bufSize, BOOL isShort)
{
	int bytes=isShort?sizeof(short):sizeof(DWORD);
	CheckXMLBuf(opBuf, bufSize +bytes);
	char *teBuf=opBuf->m_strBuf+opBuf->m_strLen;
	if (isShort) *(short *)teBuf=bufSize;
	else *(DWORD *)teBuf=bufSize;
	if(bufSize>0) memcpy(teBuf+bytes, addBuf, bufSize);
	opBuf->m_strLen+=bufSize+bytes;
}

static VOID MyAddString(MystrBuf *opBuf, const char *addStr)
{
	int addLen=strlen(addStr);
	CheckXMLBuf(opBuf, addLen);
	memcpy(opBuf->m_strBuf+opBuf->m_strLen, addStr, addLen);
	opBuf->m_strLen+=addLen;
}

static void   convertGBKToUtf8(MystrBuf *opBuf) 
{ 
	if (opBuf->m_strBuf==NULL) return;
	int len=MultiByteToWideChar(CP_ACP, 0, (LPCTSTR)opBuf->m_strBuf, -1, NULL,0); 
	unsigned short * wszUtf8 = new unsigned short[len+1]; 
	memset(wszUtf8, 0, len * 2 + 2); 
	MultiByteToWideChar(CP_ACP, 0, (LPCTSTR)opBuf->m_strBuf, -1, (LPWSTR)wszUtf8, len); 

	len = WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR)wszUtf8, -1, NULL, 0, NULL, NULL); 
	char *szUtf8=new char[len + 1]; 
	memset(szUtf8, 0, len + 1); 
	WideCharToMultiByte (CP_UTF8, 0, (LPCWSTR)wszUtf8, -1, szUtf8, len, NULL,NULL); 

	MyFreeStr(opBuf);
	MyAddString(opBuf, szUtf8);
	delete[] szUtf8; 
	delete[] wszUtf8; 
} 

BOOL SendSA(SOCKET sock, _variant_t avarRecords, BOOL isShort)
{
	long lUbound, ColCount, i;
	HRESULT hr;
	try
	{
		hr = SafeArrayGetUBound(avarRecords.parray, 1,&ColCount);
		hr = SafeArrayGetUBound(avarRecords.parray, 2,&lUbound);
		BOOL OKSend=TRUE;
		if(SUCCEEDED(hr))       
		{
			CString str;
			_bstr_t bstr;
			SuperCMD teCMD;
			int sendNum=0;
			MystrBuf opBuf;
			memset(&opBuf, 0, sizeof(MystrBuf));
			for (int intRecords=0; intRecords < lUbound+1&&OKSend; intRecords++)
			{
				long rgIndices[2];
				for (i=0; i<ColCount+1; i++)
				{
					rgIndices[0] = i; 
					rgIndices[1] = intRecords;
					_variant_t result;
					result.vt = VT_BSTR;
					hr= SafeArrayGetElement(avarRecords.parray, rgIndices, &result);
					if(SUCCEEDED(hr))
					{
						if(result.vt == (VT_ARRAY | VT_UI1))
						{
							char *pBufTemp = NULL;
							long tSize=0;
							hr=SafeArrayGetUBound(result.parray, 1,&tSize);
							if(SUCCEEDED(hr)) tSize++;
							SafeArrayAccessData(result.parray,(void **)&pBufTemp);
							MyAddBuf(&opBuf, pBufTemp, tSize, isShort);
							SafeArrayUnaccessData(result.parray);
						}
						else
						{
							if (result.vt==VT_NULL || result.vt==VT_EMPTY) str.Empty();
							else
							{
								bstr=result; str=(const char *)bstr; 
							}
							MyAddBuf(&opBuf, str, str.GetLength(), isShort);
						}
					}
					else OKSend=FALSE;
				}
				sendNum++;
				if (opBuf.m_strLen>65536L)
				{
					teCMD.cmdID=SCMDDATA;
					teCMD.data=opBuf.m_strLen;
					teCMD.super[0]=sendNum;
					OKSend=SENDALL(sock, &teCMD, sizeof(SuperCMD), 0)==sizeof(SuperCMD);
					OKSend=SENDALL(sock, opBuf.m_strBuf, opBuf.m_strLen, 0)==opBuf.m_strLen;
					MyFreeStr(&opBuf);
					sendNum=0;
				}
			}
			if (opBuf.m_strLen>0)
			{
				teCMD.cmdID=SCMDDATA;
				teCMD.data=opBuf.m_strLen;
				teCMD.super[0]=sendNum;
				OKSend=SENDALL(sock, &teCMD, sizeof(SuperCMD), 0)==sizeof(SuperCMD);
				OKSend=SENDALL(sock, opBuf.m_strBuf, opBuf.m_strLen, 0)==opBuf.m_strLen;
				MyFreeStr(&opBuf);
				sendNum=0;
			}
			MyFreeStr(&opBuf);
			return OKSend;
		}
	}
	catch (...)
	{
		WriteLogFile("SendSA exception!");
	}
	return FALSE;
}

int RunSql2SA(const char *sqlStr, _ConnectionPtr inCon, CStringArray &fieldsNameArr, _variant_t &avarRecords)
{
	_RecordsetPtr retPtr;
	long lUbound=0;
	long ColCount=0;       
	long i;
	try
	{
		BOOL teOK=FALSE;
		retPtr = MyExecuteL(sqlStr, inCon, teOK);
		if (teOK==FALSE) return -1;
		if (retPtr == NULL) return 0;

		Fields *  fields = NULL;
		if (retPtr != NULL  ) //获取表字段名;
		{
			HRESULT   hr;   
			hr = retPtr->get_Fields(&fields);
			if(SUCCEEDED(hr))        
				fields->get_Count(&ColCount);
			for(i = 0;i<ColCount;i++)
			{   
				Field *   field = NULL;   
				field = fields->GetItem((_variant_t)(i));       
				fieldsNameArr.Add(CString((LPCTSTR)field->GetName()));	
			}   
			if(SUCCEEDED(hr)) fields->Release();
			if (ColCount>0)
			{
				if (!(retPtr->adoEOF &&retPtr->adoBOF))
				{
					avarRecords=retPtr->GetRows(-1);
					hr = SafeArrayGetUBound(avarRecords.parray, 2,&lUbound);
					lUbound++;
				}
				retPtr->Close();
			}
		}
		return lUbound;
	}
	catch (...)
	{
		CString telog;
		telog.Format("RunSql2SA Exception! colCount=%d, lUbound=%d FieldsName=", ColCount, lUbound);
		for (i=0;i<fieldsNameArr.GetSize(); i++)
		{
			telog+=fieldsNameArr.GetAt(i);
			telog+=_T("  ");
		}
		WriteLogFile(telog);
	}
	return -1;
}


int RunSqlAndGetXML(MystrBuf &opBuf, const char *sqlStr, _ConnectionPtr inCon)
{
	_RecordsetPtr retPtr;
	try
	{
		BOOL teOK=FALSE;
		retPtr = MyExecuteL(sqlStr, inCon, teOK);
		if (teOK==FALSE) return -1;
		if (retPtr == NULL) return 0;

		CString str;
		_variant_t  vname;
		_bstr_t bstr;
		Fields *  fields = NULL;
		CStringArray fieldsarray;

		if (retPtr != NULL  ) //获取表字段名;
		{
			long i, recCnt=0;
			HRESULT   hr;   
			hr = retPtr->get_Fields(&fields);
			long ColCount=0;       
			if(SUCCEEDED(hr))        
				fields->get_Count(&ColCount);    
			for(i = 0;i<ColCount;i++)
			{   
				Field *   field = NULL;   
				field = fields->GetItem((_variant_t)(i));       
				fieldsarray.Add(CString((LPCTSTR)field->GetName()));	
			}   
			if(SUCCEEDED(hr))       
				fields->Release();
			i = 0;
			int nFieldCount = fieldsarray.GetSize();
			MyAddString(&opBuf, "<?xml version=\"1.0\" standalone=\"yes\"?>\r\n");
			MyAddString(&opBuf, "<DEFAULT>\r\n");

			_variant_t   vNullVal; 
			vNullVal.vt   =   VT_NULL; 
			DWORD dwStart = GetTickCount();
			if (!(retPtr->adoEOF &&retPtr->adoBOF))
			{
				retPtr->MoveFirst();
				while (!retPtr->adoEOF)
				{
					MyAddString(&opBuf, "<RecordSetXML>\r\n");
					for (i = 0; i < nFieldCount;  i++)
					{
						MyAddString(&opBuf, "<");
						MyAddString(&opBuf, fieldsarray.GetAt(i));
						MyAddString(&opBuf, ">");
						//vname=retPtr->GetCollect(COleVariant(fieldsarray.GetAt(i)));
						vname=retPtr->GetCollect(i);
						if (vname.vt==VT_NULL || vname.vt==VT_EMPTY) str.Empty();
						else { bstr=vname; str=(const char *)bstr; }
						MyAddString(&opBuf, str);
						MyAddString(&opBuf, "</");
						MyAddString(&opBuf, fieldsarray.GetAt(i));
						MyAddString(&opBuf, ">\r\n");
					}
					MyAddString(&opBuf, "</RecordSetXML>\r\n");
					retPtr->MoveNext();
					recCnt++;
				}
				DWORD dwEnd = GetTickCount() - dwStart;
				CString strTime;
				strTime.Format(_T("%s execute %d"),sqlStr,  dwEnd);
				WriteLogFile(strTime);
			}
			MyAddString(&opBuf, "</DEFAULT>\r\n");
			convertGBKToUtf8(&opBuf);
			retPtr->Close();
			retPtr = NULL;
			return recCnt;
		}
	}
	catch (_com_error &e)
	{
		if(retPtr) retPtr->Close();
		retPtr = NULL;
		_bstr_t bstrSource(e.Source());
		_bstr_t bstrDescription(e.Description());
		
		// Print Com errors.  
		printf("Error\n");
		printf("\tCode = %08lx\n", e.Error());
		printf("\tCode meaning = %s", e.ErrorMessage());
		printf("\tSource = %s\n", (LPCSTR) bstrSource);
		printf("\tDescription = %s\n", (LPCSTR) bstrDescription);
	}
	return -1;
}

static void WriteParam(const char *fileName, const char *section, const char *strValue)
{
	WritePrivateProfileString("ExchangeFormat", section, strValue, fileName);
}
static int ReadParamPInt(const char *fileName)
{
	if(fileName[0]==0) return 30000;
	return GetPrivateProfileInt("ExchangeFormat", "Progress", 30000, fileName);
}
static void ReadParamStr(const char *fileName, const char *section, MystrBuf *outBuf)
{
	char *teBuf=(char *)malloc(ONE_M);
	teBuf[0]=0;
	GetPrivateProfileString("ExchangeFormat", section, "", teBuf, ONE_M, fileName);
	MyAddString(outBuf, teBuf);
	free(teBuf);
}

int RunExeByparamFile(const char *exeFile, const char *paramFile, HANDLE &outAppProcessH, BOOL waitRet)
{
	CFileStatus teFS;
	BOOL checkFile=CFile::GetStatus(exeFile, teFS);
	if(checkFile==FALSE||teFS.m_size<10) return -1;
	int retV=0;
	SHELLEXECUTEINFO ShExecInfo = {0};
	ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
	ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
	ShExecInfo.hwnd = NULL;
	ShExecInfo.lpVerb = NULL;
	ShExecInfo.lpFile = exeFile;             
	ShExecInfo.lpParameters = paramFile;
	ShExecInfo.lpDirectory = NULL;
	ShExecInfo.nShow = SW_NORMAL;
	ShExecInfo.hInstApp = NULL;       
	BOOL sucExe=ShellExecuteEx(&ShExecInfo);
	if (sucExe)
	{
		CString teLog;
		outAppProcessH=ShExecInfo.hProcess;
		if (waitRet==FALSE)
		{
			teLog.Format("%s(%s) Execute Startup!", exeFile, paramFile);
			WriteLogFile(teLog);
		}
		else
		{
			int tL=ReadParamPInt(paramFile);
			do 
			{
				if (WaitForSingleObject(ShExecInfo.hProcess,WAITMAXTC)==WAIT_OBJECT_0) break;
				if (tL==ReadParamPInt(paramFile))
				{
					TerminateProcess(ShExecInfo.hProcess,0);
					outAppProcessH=NULL;
					teLog.Format("%s(%s) Execute TimeOut!", exeFile, paramFile);
					WriteLogFile(teLog);
					retV=-3;
					break;
				}
				tL=ReadParamPInt(paramFile);
			} while (1);
		}		
	}
	else retV=-1;
	return retV;
}

int RunLExe(int paramNum, char *buf, int *paramSize, MystrBuf *outBuf)
{
	try
	{
		CString localFullPath=GetAppDirPath();
		int retV=0;
		char bufFunName[256];
		memcpy(bufFunName, buf, paramSize[0]);
		bufFunName[paramSize[0]]=0;
		CString exeName;
		exeName.Format("%s\\%s", localFullPath, bufFunName);
		if(strrchr(bufFunName, '.')==NULL) exeName+=_T(".exe");
		int index=paramSize[0];
		::srand(GetTickCount());
		CString paramFileName;
		paramFileName.Format("%s\\%d%d.ini", localFullPath, GetTickCount(), rand());
		::DeleteFile(paramFileName);
		for (int i=1; i<paramNum; i ++)
		{
			CString paramName;
			paramName.Format("param%d", i);
			char bufj[1024];
			memset(bufj, 0, 1024);
			memcpy(bufj, buf+index, paramSize[i]);
			WriteParam(paramFileName, paramName, bufj);
			index+=paramSize[i];
		}		
		HANDLE appProcessH=NULL;
		retV=RunExeByparamFile(exeName, paramFileName, appProcessH, TRUE);
		if(retV==0) ReadParamStr(paramFileName, "Result", outBuf);
		::DeleteFile(paramFileName);
		return retV;
	}
	catch(...)
	{
		WriteLogFile("RunExeFun: Exception!");
	}	
	return -4; 
}

typedef struct tagRunControlP
{
	HANDLE p_processH;
	int p_progress;
	DWORD p_lastSTC,p_lastPTC;
}RunControlP;

void SearchAndRunexe(int way)
{
	static RunControlP s_RCP[17];

	int i;
	char bufname[MAX_PATH];
	CString exeName,paramFileName;
	if (way==1)
	{
		for (i=0; i<17; i++) 
		{
			s_RCP[i].p_lastSTC=0;
			s_RCP[i].p_processH=NULL;
		}
	}
	for (i=1; i<17; i++) 
	{
		bufname[0]=0;
		GetRunParamString(i, "ExecuteName", bufname);
		if(bufname[0]) 
		{
			exeName.Format("%s\\%s", GetAppDirPath(), bufname);
			if(strrchr(bufname, '.')==NULL) exeName+=_T(".exe");
		}
		bufname[0]=0;
		GetRunParamString(i, "ParamName", bufname);
		if(bufname[0])
		{
			paramFileName.Format("%s\\%s", GetAppDirPath(), bufname);
			if(strrchr(bufname, '.')==NULL) paramFileName+=_T(".ini");
		}
		if (s_RCP[i].p_processH&&s_RCP[i].p_processH!=INVALID_HANDLE_VALUE)
		{
			DWORD dwExitC=0;
			GetExitCodeProcess(s_RCP[i].p_processH, &dwExitC);
			if (dwExitC==STILL_ACTIVE)
			{
				if (way!=2)
				{
					DWORD thisTC=GetTickCount();
					int tL=ReadParamPInt(paramFileName);
					if (s_RCP[i].p_lastPTC<1)
					{
						s_RCP[i].p_lastPTC=thisTC;
						s_RCP[i].p_progress=tL;
						continue;
					}
					if (thisTC-s_RCP[i].p_lastPTC<WAITMAXTC) continue;
					if (s_RCP[i].p_progress!=tL)
					{
						s_RCP[i].p_lastPTC=thisTC;
						s_RCP[i].p_progress=tL;
						continue;
					}
				}
				TerminateProcess(s_RCP[i].p_processH, -1);
				CString teLog;
				teLog.Format("%s(%s) Execute TimeOut!", exeName, paramFileName);
				WriteLogFile(teLog);
			}
			s_RCP[i].p_processH=NULL;
			s_RCP[i].p_progress=-1;
			s_RCP[i].p_lastPTC=0;
		}
		else
		{
			if(GetRunParamInt(i, "RunWay", 9)==way)
			{
				int loopTime=GetRunParamInt(i, "LoopTime", -1)*60L;
				DWORD thisTC=GetTickCount();
 				if (loopTime<1) loopTime=thisTC;
 				if (s_RCP[i].p_lastSTC==0||s_RCP[i].p_lastSTC+loopTime<thisTC/1000)
				{
					if(!exeName.IsEmpty()) 
					{
						if (RunExeByparamFile(exeName, paramFileName, s_RCP[i].p_processH, way==2)==0)
						{
							s_RCP[i].p_lastPTC=GetTickCount();
							s_RCP[i].p_lastSTC=s_RCP[i].p_lastPTC/1000;
							s_RCP[i].p_progress=ReadParamPInt(paramFileName);
							//return;
						}
					}
				}
			}
		}
	}
}
