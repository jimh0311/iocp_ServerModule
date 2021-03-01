#pragma once
#include <WinSock2.h>
#include "stSession.h"

void CompletionRecv(stSession* pSession, DWORD);
void CompletionSend(stSession* pSession, DWORD);
void PostSend(stSession* pSession);

BOOL PushSendQueue(stSession *pSession, CProtoBuffer *pData);


// �ϴ� �����߻��ؼ� Recv, Send Shutdown �Ѵ�.
void ShutdownSession(stSession* pSession);
// IOCount = 0 �̸�, Delete Session�� �����Ѵ�. ���� ����Ʈ���� ��������.
void ReleaseSession(stSession* pSession);