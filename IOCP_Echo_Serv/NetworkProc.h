#pragma once
#include <WinSock2.h>
#include "stSession.h"

void CompletionRecv(stSession* pSession, DWORD);
void CompletionSend(stSession* pSession, DWORD);
void PostSend(stSession* pSession);

BOOL PushSendQueue(stSession *pSession, CProtoBuffer *pData);


// 일단 에러발생해서 Recv, Send Shutdown 한다.
void ShutdownSession(stSession* pSession);
// IOCount = 0 이면, Delete Session을 수행한다. 물론 리스트에서 빼버리고.
void ReleaseSession(stSession* pSession);