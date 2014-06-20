#pragma once

#define		CHECKSRV		2222

//500以上或5000以上
#define		RUNSQL			710
#define		RUNSQL2			711
#define		UPDATEBINV		712
//#define		RUNREMOTEEXE	521

#define		SCMDNAME		1120
#define		SCMDDATA		1121

#define		SCMDOK		3333
#define		SCMDCANCEL	3000

typedef struct tagSuperCMD
{
	/*
	int cmdID:
	int data：后续传递数据的字节数;
	int super[30]:
	*/
	int		cmdID,data;
	int		super[30];
} SuperCMD;

inline int RECVALL(SOCKET sock, LPVOID buf1, int recvSize, int flag)
{
	char *buf=(char *)buf1;
	int remainSize=recvSize;
	int teSize=0;
	do {
		teSize=min(4*1024L, remainSize);
		teSize=recv(sock, buf, teSize, flag);
		if(teSize<=0) return teSize;
		remainSize-=teSize;
		buf+=teSize;
	} while(remainSize>0);
	return recvSize;
}
inline int SENDALL(SOCKET sock, LPVOID buf1, int sendSize, int flag)
{
	int nLeft, nWritten;
	PBYTE pBuf = (PBYTE)buf1;
	nLeft = sendSize;
	while (nLeft > 0)
	{
		//		nWritten = SendPACSChunk(pBuf, nLeft, nFlags);
		nWritten=min(4*1024L, nLeft);
		nWritten = send(sock, (const char *)pBuf, nWritten, flag);//SendChunk(pBuf, nLeft, nFlags);
		if (nWritten == SOCKET_ERROR)
			return nWritten;
		nLeft -= nWritten;
		pBuf += nWritten;
		if(nWritten == 0) return SOCKET_ERROR;
	}
	return (sendSize - nLeft);
}


