#pragma once

typedef struct tagstrBuf
{
	char *m_strBuf;
	int m_strLen,m_allocLen;
}MystrBuf;

int RunSqlAndGetXML(MystrBuf &tB, const char *sqlStr, _ConnectionPtr inCon);
int RunSql2SA(const char *sqlStr, _ConnectionPtr inCon, CStringArray &fieldsNameArr, _variant_t &avarRecords);
BOOL SendSA(SOCKET sock, _variant_t avarRecords, BOOL isShort);
void MyFreeStr(MystrBuf *opBuf);
int RunLExe(int paramNum, char *buf, int *paramSize, MystrBuf *outBuf);
int RunExeByparamFile(const char *exeFile, const char *paramFile, HANDLE &outAppProcessH, BOOL waitRet);
void SearchAndRunexe(int way);
