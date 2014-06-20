#pragma once

_ConnectionPtr ConnectDataBase(CString oracleStr);
_ConnectionPtr ConnectVolumeDatabase();
_ConnectionPtr ConnectXCKYDatabase();
_ConnectionPtr ConnectCSIFMISDatabase();
BOOL DisconnectDataBase(_ConnectionPtr _disConnection);
_RecordsetPtr MyExecuteL(const char *strsql, _ConnectionPtr inCon, BOOL &teOK);

BOOL Update2DB(const char *piName, long srcSize, long tezhSize, long modiTime);
BOOL GetPictBDParamByZjbh(const char *zjbh1, const char *zjbh2, int *outKV1, int *outKV2);
BOOL RUNORAFUN(_ConnectionPtr tePte, int paramNum, char *buf, int *paramSize, char *outBuf);
BOOL UpdateBinValue(_ConnectionPtr inCon, char *sqlBuf, BYTE *binData, int dataSize);

int GetFreeConnect();
void FreeConnect(int &iConnect);
void DisposeConnect(int iConnect);
void DisposeAllConnect(BOOL forceDis);
_ConnectionPtr GetDBConnectPtr(int dbid);