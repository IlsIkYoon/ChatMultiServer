#include "NetworkManager.h"
#include "Contents/ContentsManager.h"

CLanServer* g_NetworkManager;
extern CContentsManager* g_ContentsManager;

void CLanServer::_OnMessage(CPacket* SBuf, ULONG64 ID)
{
	g_ContentsManager->HandleContentsMsg(SBuf, ID);
}
void CLanServer::_OnAccept(ULONG64 ID)
{
	//할 일 없음
}
void CLanServer::_OnDisConnect(ULONG64 ID)
{
	g_ContentsManager->DeleteAgent(ID);
}
void CLanServer::_OnSend(ULONG64 ID)
{
	//할 일 없음
}
