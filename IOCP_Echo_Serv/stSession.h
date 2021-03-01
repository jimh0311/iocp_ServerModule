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
	volatile INT IOCount;					//	0 �̸� ��� IO �Ϸ�� ����, �� ���δ�. ���������� �Ǵ�
	BOOL		bSendAble;					//	�� ������ Send ȣ���� �� �ѹ�.

	DWORD		recvBytes;
	DWORD		sendBytes;

	CRITICAL_SECTION csQueue;
};
