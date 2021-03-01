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
//	전역변수
//-------------------------------------------------------
HANDLE	g_IOCP;			//	Complete Port 전역 관리변수.
DWORD	g_IOFlag = 0;

list<stSession*>	l_Session;	//	세션관리 리스트.

HANDLE	hAcceptTh, hWorkerTh[THREADMAX];
BOOL	bThExit;		//	bThExit = TRUE 면 모두 종료

SOCKET		hLisnSock;
SOCKADDR_IN addrLisn;

CRITICAL_SECTION csList;
//-------------------------------------------------------
//  17.02.10
//	전역함수
//-------------------------------------------------------

unsigned __stdcall AcceptThreadProc(void *PARAM);	//	Accept 전용 녀석...
unsigned __stdcall WorkerThreadProc(void *PARAM);

//-------------------------------------------------------
//  17.02.10
//	메인함수
//-------------------------------------------------------
int main()
{
	//	Listen 네트워크 생성...
	WSADATA wsaData;
	WSAStartup(0x0202, &wsaData);

	//CompletionPort 생성
	g_IOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, THREADMAX);

	//Critical Section 초기화
	InitializeCriticalSection(&csList);

	
	//
	hLisnSock = socket(AF_INET, SOCK_STREAM, 0);	//	버클리 소켓은 비동기 IO 지원됨.
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
	//	각 Thread 생성
	//	Accept 1
	//	Worker 4
	//	끝
	//-------------------------------------------------------
	bThExit = FALSE;
	hAcceptTh = (HANDLE)_beginthreadex(NULL, 0, AcceptThreadProc, NULL, 0, NULL);	//	Accept Thread;
	
	for (int i = 0; i < THREADMAX; i++)
	{
		hWorkerTh[i] = (HANDLE)_beginthreadex(NULL, 0, WorkerThreadProc, NULL, 0, NULL);	//	4 - Worker Thread;
	}

	//	메인은 아무것도 안하고 여기서 대기하고 있을 예정.
	WaitForSingleObject(hAcceptTh, INFINITE);
	WaitForMultipleObjects(THREADMAX, hWorkerTh, TRUE, INFINITE);

	//	종료루틴
	DeleteCriticalSection(&csList);
	WSACleanup();
	return 0;
}


//-------------------------------------------------------
//  17.02.10
//	
//	Accept만 전담으로 하는 녀석.
//-------------------------------------------------------
unsigned __stdcall AcceptThreadProc(void *PARAM)
{
	stSession *pClient;
	DWORD ReturnRecv;
	WSABUF	wsaBuf;
	DWORD Error;
	while (!bThExit)	//	bThExit = TRUE 면 모든 쓰레드 종료
	{
		// 변수들 초기화
		ReturnRecv = 0;
		wsaBuf.buf = NULL;
		wsaBuf.len = 0;
		Error = 0;
		//	세션 동적할당
		pClient = new stSession;
		pClient->addrSize = sizeof(SOCKADDR_IN);
		//	여기서 블럭이 계속 걸려있을 것이다.
		pClient->hSocket = accept(hLisnSock, (SOCKADDR*)&pClient->addrClient, &pClient->addrSize);

		//	에러뜨면 로그찍고 다시 엑셉트하러.
		if (pClient->hSocket == INVALID_SOCKET)
		{
			__Log(dfLOG_WARNING, L"accept Fail; INVALID_SOCKET");
			delete pClient;
			continue;
		}
		// 디버깅용
		__Log(dfLOG_SYSTEM, L"Connect hSocket = %d ;", pClient->hSocket);

		// 정상적으로 접속이 완료했다.
		// IOCP 핸들에 소켓정보 추가등록.
		CreateIoCompletionPort((HANDLE)pClient->hSocket, g_IOCP, (ULONG_PTR)pClient, THREADMAX);
		//
		pClient->IOCount = 0;
		pClient->bSendAble = 0;
		// 오버랩 초기화
		memset(&pClient->OverlapRecv, 0, sizeof(OVERLAPPED));
		memset(&pClient->OverlapSend, 0, sizeof(OVERLAPPED));
		// WSARecv를 위한 버퍼 셋팅
		wsaBuf.buf = pClient->RecvQ.GetWriteBufferPtr();
		wsaBuf.len = pClient->RecvQ.GetNotBrokenPutSize();
		// Session 별 크리티컬 섹션 등록
		InitializeCriticalSection(&pClient->csQueue);
		// 전역 리스트에 세션 등록
		// 크리티컬 섹션 동기화 들어간다.
		EnterCriticalSection(&csList);
		l_Session.push_back(pClient);
		LeaveCriticalSection(&csList);
		// 안전하게 IOCount 상승
		InterlockedIncrement((unsigned long*)&pClient->IOCount);
		// Recv 들어간다.
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
			// FALSE 라는 것은 현재 뭔가 에러가 있다는 뜻이다!
			if (bIOReturn == FALSE)
			{
				Error = WSAGetLastError();
				__Log(dfLOG_WARNING, L"GQCS() Error[0x%d] !", Error);
			}
			//정상적인 종료이다.
			//실제 메모리 Delete는 IOCount = 0 이 되었을 때 한다.
			shutdown(pSession->hSocket, SD_BOTH);
		}
		else
		{
			//	Recv 상황인가?
			if (pOverlapTemp == &pSession->OverlapRecv)
			{
				CompletionRecv(pSession, cbTransferred);
			}

			//	Send 상황인가?
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