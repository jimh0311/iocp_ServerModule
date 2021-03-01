#pragma once
#include <windows.h>
#include "Net_Lib\CProtoBuffer.h"
#include "Net_Lib\CStreamRQ.h"

struct stSession
{
	SOCKET		hSocket;
	SOCKADDR_IN addrClient;
	int			addrSize;
	CStreamRQ	RecvQ, SendQ;
	OVERLAPPED	OverlapRecv, OverlapSend;
	volatile INT IOCount;					//	0 이면 모든 IO 완료로 보고, 다 죽인다. 안전빵으로 건다
	BOOL		bSendAble;					//	각 세션의 Send 호출은 단 한번.

	DWORD		recvBytes;
	DWORD		sendBytes;

	CRITICAL_SECTION csQueue;
};
