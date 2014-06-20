#include "StdAfx.h"
#include "GMan.h"
#include "ProcessDBEvent.h"

_ConnectionPtr ConnectDataBase(CString oracleParam)
{
	CString errormessage; 
	try //建立数据库连接 
	{ 
		//		CString OracleParam = "Provider=OraOLEDB.Oracle.1;Password=footmark;User ID=footmark;Data Source=chancel;Persist Security Info=True";
		HRESULT hr=S_OK;
		_ConnectionPtr _teConnection=NULL;
		if (strlen(oracleParam)>5)
		{
			hr=_teConnection.CreateInstance(__uuidof(Connection)); 
			if (SUCCEEDED(hr))
			{
				hr=_teConnection->Open(_bstr_t(oracleParam),"","",adModeUnknown);
				if(SUCCEEDED(hr)) return _teConnection;
				errormessage+=_T("Failed: ");
				errormessage+="OracleString=";
				errormessage+=oracleParam;
				WriteLogFile(errormessage);
			}
			DisconnectDataBase(_teConnection);
		}
		return NULL;
	} 
	catch(...) 
	{
		errormessage+=_T("Exception: ");
		errormessage+="OracleString=";
		errormessage+=oracleParam;
		WriteLogFile(errormessage);
		return NULL; 
	} 
}

_ConnectionPtr ConnectVolumeDatabase()
{
	char oracleParam[256];
	memset(oracleParam, 0, 256);
	GetServiceParamString("VolumeString", oracleParam);
	return ConnectDataBase(oracleParam); 
}

_ConnectionPtr ConnectXCKYDatabase()
{
	char oracleParam[256];
	memset(oracleParam, 0, 256);
	GetServiceParamString("XCKYString", oracleParam);
	return ConnectDataBase(oracleParam); 
}

_ConnectionPtr ConnectCSIFMISDatabase()
{
	char oracleParam[256];
	memset(oracleParam, 0, 256);
	GetServiceParamString("CSIFMISString", oracleParam);
	return ConnectDataBase(oracleParam); 
}

BOOL DisconnectDataBase(_ConnectionPtr _disConnection)
{
	try
	{
		if (_disConnection!=NULL)
		{
			_disConnection->Close();
			_disConnection=NULL;
		}
		return TRUE;
	}
	catch(_com_error e)
	{
		CString errormessage; 
		errormessage.Format("DisconnectDataBase: 关闭数据库失败！\r\n错误信息：%s", e.ErrorMessage());
		WriteLogFile(errormessage);
		return FALSE; 
	}
}

_RecordsetPtr MyExecuteL(const char *strsql, _ConnectionPtr inCon, BOOL &teOK)
{
	try
	{
		teOK=FALSE;
		if (inCon==NULL) return NULL;
		_RecordsetPtr retPtr=inCon->Execute(_bstr_t(strsql),NULL,adCmdText);
		teOK=TRUE;
		return retPtr;
	}
	catch(...)
	{
		CString errormessage; 
		errormessage.Format("MyExecuteL Exception: strSQL=%s", strsql); 
		WriteLogFile(errormessage);
		return NULL; 
	}
}

BOOL UpdateBinValue(_ConnectionPtr inCon, char *sqlBuf, BYTE *binData, int dataSize)
{
	try
	{
		_bstr_t bstrQuery(sqlBuf);
		_variant_t vRecsAffected(0L);
		_RecordsetPtr pRecordSet;
		HRESULT hr=S_OK;
		pRecordSet.CreateInstance("ADODB.Recordset");
		hr=pRecordSet->Open(bstrQuery, inCon.GetInterfacePtr(), adOpenStatic/*adOpenDynamic*/,adLockPessimistic,adCmdText);
		if (SUCCEEDED(hr) &&!pRecordSet->adoBOF)
		{
			VARIANT			varBLOB;
			SAFEARRAY FAR *psa;
			SAFEARRAYBOUND rgsabound[1];
			rgsabound[0].lLbound = 0;
			rgsabound[0].cElements = dataSize;
			//Create a safe array to store the array of BYTES
			psa = SafeArrayCreate(VT_UI1,1,rgsabound);
			for (long i = 0; i < (long)dataSize; i++)
				SafeArrayPutElement (psa, &i, binData++);
			varBLOB.vt = VT_ARRAY | VT_UI1;
			varBLOB.parray = psa;
			pRecordSet->MoveFirst();
			pRecordSet->Fields->GetItem((_variant_t)(0L))->AppendChunk(varBLOB);
			pRecordSet->Update();

			pRecordSet->Close(); pRecordSet=NULL;
			::SafeArrayDestroy(psa);
			return TRUE;
		}
	}
	catch(...)
	{
		CString teLog;
		teLog.Format("UpdateBinValue Exception: sql=%s, dataSize =%d.", sqlBuf, dataSize);
		WriteLogFile(teLog);
	}
	return FALSE;
}

BOOL RUNORAFUN(_ConnectionPtr pDBConnect, int paramNum, char *buf, int *paramSize, char *outBuf)
{
	CString teLog;
	try
	{
		if (pDBConnect)
		{
			_CommandPtr m_pCmd; 
			_ParameterPtr m_rPara,m_para[100]; 
			int retV=0;
			char retBuf[1025];
			m_pCmd.CreateInstance(__uuidof(Command)); 
			m_rPara.CreateInstance(__uuidof(Parameter)); 
			for (int i=0; i<paramNum; i ++)
				m_para[i].CreateInstance(__uuidof(Parameter));
			char bufFunName[256];
			memcpy(bufFunName, buf, paramSize[0]);
			bufFunName[paramSize[0]]=0;
			m_pCmd->CommandText = bufFunName; 
			m_pCmd->ActiveConnection = pDBConnect; 
			m_pCmd->CommandType = adCmdStoredProc; 
			teLog+=bufFunName;
			teLog+=_T("  ");
			m_rPara = m_pCmd->CreateParameter(_bstr_t("Result"),adVarChar,adParamReturnValue,1024,_variant_t(retBuf)); 
			m_pCmd->Parameters->Append(m_rPara);
			int index=paramSize[0];
			for (int i=1; i<paramNum; i ++)
			{
				CString paramName;
				paramName.Format("paramName%d", i);
				char bufj[1024];
				memset(bufj, 0, 1024);
				memcpy(bufj, buf+index, paramSize[i]);
				teLog+=bufj;
				teLog+=_T("  ");
				index+=paramSize[i];
				m_para[i] = m_pCmd->CreateParameter(_bstr_t(paramName),adVarChar,adParamInput, strlen(bufj)+1,_variant_t(bufj));
				m_pCmd->Parameters->Append(m_para[i]); 
			}			
			m_pCmd->Execute(NULL,NULL,adCmdStoredProc);
			teLog+=_T(" End ExeFun ");
			_ParameterPtr resPPtr=m_pCmd->Parameters->GetItem("Result");
			if(resPPtr==NULL) return TRUE;
			_variant_t vret=resPPtr->GetValue();
			if(vret.vt == VT_NULL||vret.vt==VT_EMPTY) return TRUE;
			_bstr_t bstr=vret;
			teLog+=_T(" End GetValue ");
			CString retStr=(const char *)bstr;
			int size1=min(1023, retStr.GetLength());
			memcpy(outBuf, retStr, size1);
			teLog+=_T(" End Copy Result ");
			outBuf[size1]=0;
			return TRUE;
		}
	}
	catch(...)
	{
		WriteLogFile("RunStandardFun: Exception!");
		WriteLogFile(teLog);
	}	
	return FALSE; 
}

BOOL Update2DB(const char *piName, long srcSize, long tezhSize, long modiTime)
{
	try
	{
		_ConnectionPtr pDBConnect=ConnectVolumeDatabase();
		if (pDBConnect)
		{
			_CommandPtr m_pCmd; 
			_ParameterPtr m_rPara,m_para1,m_para2,m_para3,m_para4; 
			char zjbh[32];
			int retV=0;
			strcpy(zjbh, piName);
			m_pCmd.CreateInstance(__uuidof(Command)); 
			m_rPara.CreateInstance(__uuidof(Parameter)); 
			m_para1.CreateInstance(__uuidof(Parameter));
			m_para2.CreateInstance(__uuidof(Parameter));
			m_para3.CreateInstance(__uuidof(Parameter));
			m_para4.CreateInstance(__uuidof(Parameter));
			
			m_pCmd->CommandText = "FUN_UPDATEPICTINFO"; 
			m_pCmd->ActiveConnection = pDBConnect; 
			m_pCmd->CommandType = adCmdStoredProc; 
			
			m_rPara = m_pCmd->CreateParameter(_bstr_t("Result"),adInteger,adParamReturnValue,sizeof(long),_variant_t((long)retV)); 
			m_pCmd->Parameters->Append(m_rPara); 
			m_para1 = m_pCmd->CreateParameter(_bstr_t("pv_zjbh"),adVarChar,adParamInput, strlen(zjbh)+1,_variant_t(zjbh));
			m_pCmd->Parameters->Append(m_para1); 

			m_para2 = m_pCmd->CreateParameter(_bstr_t("pv_pictsize"),adVarNumeric,adParamInput, 4,_variant_t(srcSize));
			m_pCmd->Parameters->Append(m_para2); 
			m_para3 = m_pCmd->CreateParameter(_bstr_t("pv_tezhsize"),adVarNumeric,adParamInput, 4,_variant_t(tezhSize));
			m_pCmd->Parameters->Append(m_para3); 
			m_para4 = m_pCmd->CreateParameter(_bstr_t("pv_modiTime"),adVarNumeric,adParamInput, 4, _variant_t(modiTime));
			m_pCmd->Parameters->Append(m_para4); 
			
			m_pCmd->Execute(NULL,NULL,adCmdStoredProc); 	

			DisconnectDataBase(pDBConnect);
			return TRUE;
		}
	}
	catch(...)
	{
		WriteLogFile("Update2DB: Exception!");
	}	
	return FALSE; 
}

BOOL GetPictBDParamByZjbh(const char *zjbh1, const char *zjbh2, int *outKV1, int *outKV2)
{
	try
	{
		_ConnectionPtr pDBConnect=ConnectVolumeDatabase();
		if (pDBConnect)
		{
			char retStr[128];
			_CommandPtr m_pCmd; 
			_ParameterPtr m_rPara,m_para1,m_para2,m_para3; 
			m_pCmd.CreateInstance(__uuidof(Command)); 
			m_rPara.CreateInstance(__uuidof(Parameter)); 
			m_para1.CreateInstance(__uuidof(Parameter));
			m_para2.CreateInstance(__uuidof(Parameter));
			
			m_pCmd->CommandText = "FUN_GETRESULT_KV"; 
			m_pCmd->ActiveConnection = pDBConnect; 
			m_pCmd->CommandType = adCmdStoredProc; 
			
			m_rPara = m_pCmd->CreateParameter(_bstr_t("Result"),adVarChar,adParamReturnValue,128,_variant_t(retStr)); 
			m_pCmd->Parameters->Append(m_rPara); 
			m_para1 = m_pCmd->CreateParameter(_bstr_t("pv_sbzjbh"),adVarChar,adParamInput, strlen(zjbh1)+1,_variant_t(zjbh1));
			m_pCmd->Parameters->Append(m_para1); 
			m_para2 = m_pCmd->CreateParameter(_bstr_t("pv_zjbh"),adVarChar,adParamInput, strlen(zjbh2)+1,_variant_t(zjbh2));
			m_pCmd->Parameters->Append(m_para2); 

			m_pCmd->Execute(NULL,NULL,adCmdStoredProc); 
			_ParameterPtr resPPtr=m_pCmd->Parameters->GetItem("Result");
			if(resPPtr==NULL) return TRUE;
			_variant_t vret=resPPtr->GetValue();
			if(vret.vt == VT_NULL||vret.vt==VT_EMPTY) return TRUE;
			_bstr_t bstr=vret;
			//_bstr_t bstr=m_pCmd->Parameters->GetItem("Result")->GetValue();
			DisconnectDataBase(pDBConnect);
			CString teOpStr,tStr1;
			teOpStr=(const char *)bstr;
			int pos=teOpStr.Find(',');
			if (pos<1) return FALSE;
			tStr1=teOpStr.Left(pos);
			*outKV1=atoi((const char *)tStr1);
			tStr1=teOpStr.Right(teOpStr.GetLength()-pos-1);
			*outKV2=atoi((const char *)tStr1);

// 			tStr1.Format("%s %s %d %d", zjbh1,zjbh2, *outKV1, *outKV2);
// 			WriteLogFile(tStr1);
			return TRUE;
		}
	}
	catch(...)
	{
		WriteLogFile("GetPictBDParamByZjbh: Exception!");
	}	
	return FALSE; 	
}
const int MAXCONNECT=64;
static _ConnectionPtr g_stConnect[MAXCONNECT];
static int g_howConnect=0;
static int g_ConnectStatus[MAXCONNECT];
int GetFreeConnect()
{
	int i, tryTime=0;
	for(i=0; i<g_howConnect; i ++)
	{
		if(g_stConnect[i]!=NULL && g_ConnectStatus[i]==0X11)
		{
			g_ConnectStatus[i]=0X22;
			return i;
		}
	}
	for(i=0; i<g_howConnect; i ++)
		if(g_ConnectStatus[i]==0) break;
	if(i<MAXCONNECT)
	{
		g_stConnect[i]=ConnectVolumeDatabase();
		if(g_stConnect[i]!=NULL) 
		{
			g_ConnectStatus[i]=0X11;
			if(i>=g_howConnect) g_howConnect=i+1;
			CString teLog;
			teLog.Format("New ConnectDB %d", i);
			WriteLogFile(teLog);
			return i;
		}
	}
	
	tryTime=0;
	while(tryTime<3&&g_howConnect>0)
	{
		for(i=0; i<g_howConnect; i ++)
		{
			if(g_stConnect[i]!=NULL && g_ConnectStatus[i]==0X11)
			{
				g_ConnectStatus[i]=0X22;
				return i;
			}
		}
		Sleep(100*(tryTime+1));
		tryTime++;
	};
	WriteLogFile(_T("Cannot Get Free DB Connect!"));
	return -1;
}
void FreeConnect(int &iConnect)
{
	if(iConnect<g_howConnect&&iConnect>=0) 
	{
		g_ConnectStatus[iConnect]=0X11;
		iConnect=-1;
	}
}
void DisposeConnect(int iConnect)
{
	if(g_stConnect[iConnect]!=NULL)
	{
		try{
			if(g_stConnect[iConnect])
			{
				g_stConnect[iConnect]->Close();
				g_stConnect[iConnect]=NULL;
				g_ConnectStatus[iConnect]=0;
				CString teLog;
				teLog.Format("Dispose %d DBConnect Suc!", iConnect);
				WriteLogFile(teLog);
			}
		}
		catch (...) {
			g_stConnect[iConnect]=NULL;
			WriteLogFile(_T("Close DataBase Error Exception!"));
		}
	}
	g_ConnectStatus[iConnect]=0;
}
void DisposeAllConnect(BOOL forcedis)
{
	int i;
	for(i=0; i<g_howConnect; i ++)
	{
		if (forcedis||g_ConnectStatus[i]!=0X22) DisposeConnect(i);
	}
	if (forcedis)
	{
		g_howConnect=0;
		for(i=0; i<MAXCONNECT; i ++)
		{
			g_stConnect[i]=NULL;
			g_ConnectStatus[i]=0;
		}
	}
	else
	{
		for(i=g_howConnect-1; i>=0; i --)
		{
			if (g_ConnectStatus[i]!=0) break; 
		}
		g_howConnect=i+1;
	}
//	WriteLogFile(_T("Dispose All DB Connect!"));
}

_ConnectionPtr GetDBConnectPtr(int dbid)
{
	return g_stConnect[dbid];
}