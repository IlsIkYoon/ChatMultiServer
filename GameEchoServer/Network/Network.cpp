#include "Network.h"
#include "Thread/Auth.h"

extern CAuthThreadWork* g_AuthThreadWork;

void CWanServer::_OnMessage(CPacket* SBuf, ULONG64 ID)
{
	InterlockedIncrement(&recvCount);

	return;
}


void CWanServer::_OnAccept(ULONG64 ID)
{
	unsigned short playerIndex;
	playerIndex = GetIndex(ID);
	(*playerManager)[playerIndex].init(ID);

	g_AuthThreadWork->CreateSession(ID);

	InterlockedIncrement(&acceptCount);
}

CWanServer::CWanServer()
{
	acceptCount = 0;
	recvCount = 0;
	sendCount = 0;
	playerManager = nullptr;
}

void CWanServer::_OnDisConnect(ULONG64 ID)
{
	unsigned short playerIndex;
	playerIndex = GetIndex(ID);
	(*playerManager)[playerIndex].clear();
}
void CWanServer::_OnSend(ULONG64 ID)
{
	return;
}