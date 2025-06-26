#include "NetworkManager.h"
#include "Contents/ContentsManager.h"

CWanServer* g_NetworkManager;
extern CContentsManager* g_ContentsManager;

void CWanServer::_OnMessage(CPacket* SBuf, ULONG64 ID)
{
	g_ContentsManager->HandleContentsMsg(SBuf, ID);
}
void CWanServer::_OnAccept(ULONG64 ID)
{
	//할 일 없음
}
void CWanServer::_OnDisConnect(ULONG64 ID)
{
	g_ContentsManager->DeleteAgent(ID);
}
void CWanServer::_OnSend(ULONG64 ID)
{
	//할 일 없음
}
