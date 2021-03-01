#include "NetworkProc.h"
#include <list>
#include "Net_Lib\__Log.h"
#include <cstdio>

using namespace std;

extern list<stSession*> l_Session;
extern CRITICAL_SECTION csList;
// �ϴ� �����߻��ؼ� Recv, Send Shutdown �Ѵ�.
void ShutdownSession(stSession* pSession)
{
	shutdown(pSession->hSocket, SD_BOTH);
}
// IOCount = 0 �̸�, Delete Session�� �����Ѵ�. ���� ����Ʈ���� ��������.
void ReleaseSession(stSession* pSession)
{
	//�����ƾ ���!
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
//	WorkerThread ���� Recv �� ��
//-------------------------------------------------------
void CompletionRecv(stSession* pSession, DWORD cbTrasferred)
{
	//	�ϴ� IO Count �÷��ְ�
	InterlockedIncrement((LONG*)&pSession->IOCount);

	//	Recv�� �ٷ� �Ⱦұ� ������, MoveWritePos�� �ϴ� ������Ѵ�.
	EnterCriticalSection(&pSession->csQueue);
	pSession->RecvQ.MoveWritePos(cbTrasferred);
	LeaveCriticalSection(&pSession->csQueue);
	//	��Ŷó���Ѵ� �ù߳��!
	CProtoBuffer Data;
	int Dequeue;
	while (1)
	{
		Data.Clear();

		// �����Ͱ� ������ �극��ũ
		if (pSession->RecvQ.GetCurrentUsingSize() <= 0)
			break;

		// �����Ͱ� ������?
		Dequeue = pSession->RecvQ.Dequeue(Data.GetBufferPtr(), Data.GetBufferSize());
		Data.MoveWritePos(Dequeue);
		// SendQueue FULL
		if (PushSendQueue(pSession, &Data) == FALSE)
		{
			// �ٿ��ְ�...
			InterlockedDecrement((LONG*)&pSession->IOCount);
			ShutdownSession(pSession);
			// �����ƾ...
			if (pSession->IOCount == 0)
			{
				ReleaseSession(pSession);
			}
		}
	}
	//WSASend ������ַ����ϴ�...
	PostSend(pSession);

	//WSARecv ���
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
		// �����ƾ...
		if (pSession->IOCount == 0)
		{
			ReleaseSession(pSession);
		}
		return;
	}

	//WSARecv ȣ��
	iRetval = WSARecv(pSession->hSocket, &wsaBuf, 1, &pSession->recvBytes, &flag, &pSession->OverlapRecv, NULL);
	if (iRetval == SOCKET_ERROR)
	{
		Error = WSAGetLastError();
		if (Error != ERROR_IO_PENDING)
		{
			__Log(dfLOG_WARNING, L"WSARecv() Error[0x%05d]", Error);
			InterlockedDecrement((LONG*)&pSession->IOCount);
			ShutdownSession(pSession);
			// �����ƾ...
			if (pSession->IOCount == 0)
			{
				ReleaseSession(pSession);
			}
		}
	}
}

//-------------------------------------------------------
//  17.02.10
//	SendQueue �� �ִ� ����
//
//	Return TRUE �� ����
//	FALSE ��, SendQueue�� �� ã��. ����, IOCount ������, �˴ٿ� �Ǵ�.
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
//	������ WSASend�� ȣ������ �༮.
//-------------------------------------------------------
void PostSend(stSession* pSession)
{
	// ���� �� �ִ� ���� 0 �̾ 1�� �ٲ��ش�. �ٵ� 0�̸� ���� �� ���� �����̶� �ǹ��̹Ƿ�, Return;
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
//	WorkerThread ���� Send �� �Ϸ�Ǿ��� ��
//-------------------------------------------------------
void CompletionSend(stSession* pSession, DWORD cbTrasferred)
{
	//	1�� ���� �� ���� ���¿��� ������, ���� �� �ִ� ���·� �ٲ۴�.
	InterlockedCompareExchange((LONG*)&pSession->bSendAble, 0, 1);

	//	�� ��ŭ �������Ƿ�, ����������.
	pSession->SendQ.RemoveData(cbTrasferred);

	//	Ȥ��, �� �������� �����Ͱ� �ִٸ�?!
	if (pSession->SendQ.GetCurrentUsingSize() > 0)
	{
		PostSend(pSession);
	}
}