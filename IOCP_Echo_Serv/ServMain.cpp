#pragma comment(lib,"ws2_32")

#include <WinSock2.h>	
#include <WS2tcpip.h>
#include <Windows.h>	
#include <process.h>
#include <list>

#include "stSession.h"
#include "Net_Lib\__Log.h"
#include "Net_Lib\CProtoBuffer.h"
#include "Net_Lib\CStreamRQ.h"
#include "NetworkProc.h"

using namespace std;

#define SERVERPORT	5000
#define THREADMAX	4

//-------------------------------------------------------
//  17.02.10
//	��������
//-------------------------------------------------------
HANDLE	g_IOCP;			//	Complete Port ���� ��������.
DWORD	g_IOFlag = 0;

list<stSession*>	l_Session;	//	���ǰ��� ����Ʈ.

HANDLE	hAcceptTh, hWorkerTh[THREADMAX];
BOOL	bThExit;		//	bThExit = TRUE �� ��� ����

SOCKET		hLisnSock;
SOCKADDR_IN addrLisn;

CRITICAL_SECTION csList;
//-------------------------------------------------------
//  17.02.10
//	�����Լ�
//-------------------------------------------------------

unsigned __stdcall AcceptThreadProc(void *PARAM);	//	Accept ���� �༮...
unsigned __stdcall WorkerThreadProc(void *PARAM);

//-------------------------------------------------------
//  17.02.10
//	�����Լ�
//-------------------------------------------------------
int main()
{
	//	Listen ��Ʈ��ũ ����...
	WSADATA wsaData;
	WSAStartup(0x0202, &wsaData);

	//CompletionPort ����
	g_IOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, THREADMAX);

	//Critical Section �ʱ�ȭ
	InitializeCriticalSection(&csList);

	
	//
	hLisnSock = socket(AF_INET, SOCK_STREAM, 0);	//	��Ŭ�� ������ �񵿱� IO ������.
	memset(&addrLisn, 0, sizeof(SOCKADDR_IN));
	addrLisn.sin_family = AF_INET;
	addrLisn.sin_port = htons(SERVERPORT);
	InetPton(AF_INET, L"0.0.0.0", &addrLisn.sin_addr);
	//
	bind(hLisnSock, (SOCKADDR*)&addrLisn, sizeof(SOCKADDR_IN));
	//
	listen(hLisnSock, SOMAXCONN);
	//

	//-------------------------------------------------------
	//  17.02.10
	//	�� Thread ����
	//	Accept 1
	//	Worker 4
	//	��
	//-------------------------------------------------------
	bThExit = FALSE;
	hAcceptTh = (HANDLE)_beginthreadex(NULL, 0, AcceptThreadProc, NULL, 0, NULL);	//	Accept Thread;
	
	for (int i = 0; i < THREADMAX; i++)
	{
		hWorkerTh[i] = (HANDLE)_beginthreadex(NULL, 0, WorkerThreadProc, NULL, 0, NULL);	//	4 - Worker Thread;
	}

	//	������ �ƹ��͵� ���ϰ� ���⼭ ����ϰ� ���� ����.
	WaitForSingleObject(hAcceptTh, INFINITE);
	WaitForMultipleObjects(THREADMAX, hWorkerTh, TRUE, INFINITE);

	//	�����ƾ
	DeleteCriticalSection(&csList);
	WSACleanup();
	return 0;
}


//-------------------------------------------------------
//  17.02.10
//	
//	Accept�� �������� �ϴ� �༮.
//-------------------------------------------------------
unsigned __stdcall AcceptThreadProc(void *PARAM)
{
	stSession *pClient;
	DWORD ReturnRecv;
	WSABUF	wsaBuf;
	DWORD Error;
	while (!bThExit)	//	bThExit = TRUE �� ��� ������ ����
	{
		// ������ �ʱ�ȭ
		ReturnRecv = 0;
		wsaBuf.buf = NULL;
		wsaBuf.len = 0;
		Error = 0;
		//	���� �����Ҵ�
		pClient = new stSession;
		pClient->addrSize = sizeof(SOCKADDR_IN);
		//	���⼭ ���� ��� �ɷ����� ���̴�.
		pClient->hSocket = accept(hLisnSock, (SOCKADDR*)&pClient->addrClient, &pClient->addrSize);

		//	�����߸� �α���� �ٽ� ����Ʈ�Ϸ�.
		if (pClient->hSocket == INVALID_SOCKET)
		{
			__Log(dfLOG_WARNING, L"accept Fail; INVALID_SOCKET");
			delete pClient;
			continue;
		}
		// ������
		__Log(dfLOG_SYSTEM, L"Connect hSocket = %d ;", pClient->hSocket);

		// ���������� ������ �Ϸ��ߴ�.
		// IOCP �ڵ鿡 �������� �߰����.
		CreateIoCompletionPort((HANDLE)pClient->hSocket, g_IOCP, (ULONG_PTR)pClient, THREADMAX);
		//
		pClient->IOCount = 0;
		pClient->bSendAble = 0;
		// ������ �ʱ�ȭ
		memset(&pClient->OverlapRecv, 0, sizeof(OVERLAPPED));
		memset(&pClient->OverlapSend, 0, sizeof(OVERLAPPED));
		// WSARecv�� ���� ���� ����
		wsaBuf.buf = pClient->RecvQ.GetWriteBufferPtr();
		wsaBuf.len = pClient->RecvQ.GetNotBrokenPutSize();
		// Session �� ũ��Ƽ�� ���� ���
		InitializeCriticalSection(&pClient->csQueue);
		// ���� ����Ʈ�� ���� ���
		// ũ��Ƽ�� ���� ����ȭ ����.
		EnterCriticalSection(&csList);
		l_Session.push_back(pClient);
		LeaveCriticalSection(&csList);
		// �����ϰ� IOCount ���
		InterlockedIncrement((unsigned long*)&pClient->IOCount);
		// Recv ����.
		ReturnRecv = WSARecv(pClient->hSocket, &wsaBuf, 1, &pClient->recvBytes, &g_IOFlag, &pClient->OverlapRecv, NULL);
		if (ReturnRecv == SOCKET_ERROR)
		{
			Error = WSAGetLastError();
			if (Error != ERROR_IO_PENDING)
			{
				__Log(dfLOG_SHUTDOWN, L"WSARecv() Error[0x%x]", Error);
			}
		}
	}

	return 0;
}

//-------------------------------------------------------
//  17.02.10
//
//-------------------------------------------------------
unsigned __stdcall WorkerThreadProc(void *PARAM)
{
	BOOL	bIOReturn;
	stSession *pSession;
	stSession pClient;
	DWORD	cbTransferred;
	OVERLAPPED *pOverlapTemp;
	DWORD	Error;
	while (1)
	{
		//
		
		bIOReturn = 0;
		//cbTransferred = 0;
		//pOverlapTemp = 0;
		Error = 0;
		//
		bIOReturn = GetQueuedCompletionStatus(g_IOCP, &cbTransferred, (PULONG_PTR)&pSession, &pOverlapTemp, INFINITE);

		if (bIOReturn == FALSE || cbTransferred == 0)
		{
			// FALSE ��� ���� ���� ���� ������ �ִٴ� ���̴�!
			if (bIOReturn == FALSE)
			{
				Error = WSAGetLastError();
				__Log(dfLOG_WARNING, L"GQCS() Error[0x%d] !", Error);
			}
			//�������� �����̴�.
			//���� �޸� Delete�� IOCount = 0 �� �Ǿ��� �� �Ѵ�.
			shutdown(pSession->hSocket, SD_BOTH);
		}
		else
		{
			//	Recv ��Ȳ�ΰ�?
			if (pOverlapTemp == &pSession->OverlapRecv)
			{
				CompletionRecv(pSession, cbTransferred);
			}

			//	Send ��Ȳ�ΰ�?
			if (pOverlapTemp == &pSession->OverlapSend)
			{
				CompletionSend(pSession, cbTransferred);
			}
		}


		InterlockedDecrement((unsigned long*)&pSession->IOCount);
		if (pSession->IOCount == 0)
		{
			ReleaseSession(pSession);
		}

	}
}