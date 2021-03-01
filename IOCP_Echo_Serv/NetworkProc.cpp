#include "NetworkProc.h"
#include <list>
#include "Net_Lib\__Log.h"
#include <cstdio>

using namespace std;

extern list<stSession*> l_Session;
extern CRITICAL_SECTION csList;
// 일단 에러발생해서 Recv, Send Shutdown 한다.
void ShutdownSession(stSession* pSession)
{
	shutdown(pSession->hSocket, SD_BOTH);
}
// IOCount = 0 이면, Delete Session을 수행한다. 물론 리스트에서 빼버리고.
void ReleaseSession(stSession* pSession)
{
	//종료루틴 밟어!
	if (pSession->IOCount != 0)
	{
		__Log(dfLOG_SHUTDOWN, L"ReleaseSession() IOCount != 0 !");
	}
	
	EnterCriticalSection(&csList);
	list<stSession*>::iterator Iter = l_Session.begin();
	for (Iter; Iter != l_Session.end(); ++Iter)
	{
		if (*Iter == pSession)
		{
			l_Session.erase(Iter);
			break;
		}
	}
	LeaveCriticalSection(&csList);

	closesocket(pSession->hSocket);
	delete pSession;
	__Log(dfLOG_SYSTEM, L"Normal ReleaseSession");
	
	return;
}

//-------------------------------------------------------
//  17.02.10
//	WorkerThread 에서 Recv 일 때
//-------------------------------------------------------
void CompletionRecv(stSession* pSession, DWORD cbTrasferred)
{
	//	일단 IO Count 올려주고
	InterlockedIncrement((LONG*)&pSession->IOCount);

	//	Recv에 바로 꽂았기 때문에, MoveWritePos를 일단 해줘야한다.
	EnterCriticalSection(&pSession->csQueue);
	pSession->RecvQ.MoveWritePos(cbTrasferred);
	LeaveCriticalSection(&pSession->csQueue);
	//	패킷처리한다 시발놈아!
	CProtoBuffer Data;
	int Dequeue;
	while (1)
	{
		Data.Clear();

		// 데이터가 없으면 브레이크
		if (pSession->RecvQ.GetCurrentUsingSize() <= 0)
			break;

		// 데이터가 있으면?
		Dequeue = pSession->RecvQ.Dequeue(Data.GetBufferPtr(), Data.GetBufferSize());
		Data.MoveWritePos(Dequeue);
		// SendQueue FULL
		if (PushSendQueue(pSession, &Data) == FALSE)
		{
			// 줄여주고...
			InterlockedDecrement((LONG*)&pSession->IOCount);
			ShutdownSession(pSession);
			// 종료루틴...
			if (pSession->IOCount == 0)
			{
				ReleaseSession(pSession);
			}
		}
	}
	//WSASend 등록해주러갑니다...
	PostSend(pSession);

	//WSARecv 등록
	int iRetval;
	WSABUF wsaBuf;
	int Error;
	DWORD flag = 0;
	wsaBuf.buf = pSession->RecvQ.GetWriteBufferPtr();
	wsaBuf.len = pSession->RecvQ.GetNotBrokenPutSize();
	if (wsaBuf.len == 0)
	{
		__Log(dfLOG_WARNING, L"WSABUF Length == 0");
		InterlockedDecrement((LONG*)&pSession->IOCount);
		ShutdownSession(pSession);
		// 종료루틴...
		if (pSession->IOCount == 0)
		{
			ReleaseSession(pSession);
		}
		return;
	}

	//WSARecv 호출
	iRetval = WSARecv(pSession->hSocket, &wsaBuf, 1, &pSession->recvBytes, &flag, &pSession->OverlapRecv, NULL);
	if (iRetval == SOCKET_ERROR)
	{
		Error = WSAGetLastError();
		if (Error != ERROR_IO_PENDING)
		{
			__Log(dfLOG_WARNING, L"WSARecv() Error[0x%05d]", Error);
			InterlockedDecrement((LONG*)&pSession->IOCount);
			ShutdownSession(pSession);
			// 종료루틴...
			if (pSession->IOCount == 0)
			{
				ReleaseSession(pSession);
			}
		}
	}
}

//-------------------------------------------------------
//  17.02.10
//	SendQueue 에 넣는 과정
//
//	Return TRUE 면 성공
//	FALSE 면, SendQueue가 꽉 찾다. 에러, IOCount 내리고, 셧다운 건다.
//-------------------------------------------------------
BOOL PushSendQueue(stSession *pSession, CProtoBuffer *pData)
{
	int iSize = pData->GetDataSize();
	int ReturnEnqueue;
	ReturnEnqueue = pSession->SendQ.Enqueue(pData->GetBufferPtr(), iSize);
	if (ReturnEnqueue != iSize)
	{
		return FALSE;
	}

	return TRUE;
}
//-------------------------------------------------------
//  17.02.10
//
//	실제로 WSASend를 호출해줄 녀석.
//-------------------------------------------------------
void PostSend(stSession* pSession)
{
	// 보낼 수 있는 상태 0 이어서 1로 바꿔준다. 근데 0이면 보낼 수 없는 상태이란 의미이므로, Return;
	if (1 == InterlockedCompareExchange((LONG*)&pSession->bSendAble, 1, 0))
		return;

	InterlockedIncrement((LONG*)&pSession->IOCount);

	int retval;
	WSABUF wsaBuf;
	DWORD sendBytes;
	DWORD flag = 0;
	int Error;
	wsaBuf.buf = pSession->SendQ.GetReadBufferPtr();
	wsaBuf.len = pSession->SendQ.GetNotBrokenGetSize();
	retval = WSASend(pSession->hSocket, &wsaBuf, 1, &sendBytes, flag, &pSession->OverlapSend, NULL);
	if (retval == SOCKET_ERROR)
	{
		Error = WSAGetLastError();
		if (Error != ERROR_IO_PENDING)
		{
			__Log(dfLOG_SHUTDOWN, L"WSASend() Error [0x%05d] ", Error);
			InterlockedDecrement((LONG*)&pSession->IOCount);
			ShutdownSession(pSession);
			if (pSession->IOCount == 0)
			{
				ReleaseSession(pSession);
			}
		}
	}
}
//-------------------------------------------------------
//  17.02.10
//	WorkerThread 에서 Send 가 완료되었을 때
//-------------------------------------------------------
void CompletionSend(stSession* pSession, DWORD cbTrasferred)
{
	//	1는 보낼 수 없는 상태였기 때문에, 보낼 수 있는 상태로 바꾼다.
	InterlockedCompareExchange((LONG*)&pSession->bSendAble, 0, 1);

	//	저 만큼 보냈으므로, 지워버린당.
	pSession->SendQ.RemoveData(cbTrasferred);

	//	혹시, 또 보내야할 데이터가 있다면?!
	if (pSession->SendQ.GetCurrentUsingSize() > 0)
	{
		PostSend(pSession);
	}
}