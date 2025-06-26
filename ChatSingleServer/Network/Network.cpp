#include "Network.h"
#include "Contents/ContentsPacket.h"

extern LFreeQ<CPacket*> g_ContentsJobQ;
extern long long g_playerCount;


void CWanServer::_OnMessage(CPacket* message, ULONG64 ID)
{
	unsigned int enqueResult;
	ULONG64 localID = GetID(ID);

	CPacket* EnqueMessage = CPacket::Alloc();

	*EnqueMessage << ID;
	EnqueMessage->PutData(message->GetDataPtr(), message->GetDataSize());
	g_ContentsJobQ.Enqueue(EnqueMessage);
}


void CWanServer::_OnAccept(ULONG64 ID)
{
	unsigned int enqueResult;
	CPacket* CreatePlayerMsg;
	stHeader msgHeader;


	//플레이어 카운트가 맥스인지를 확인하고
	//맥스라면 큐에 넣어줌

	msgHeader.type = stJob_CreatePlayer;

	CreatePlayerMsg = CPacket::Alloc();

	*CreatePlayerMsg << ID;
	CreatePlayerMsg->PutData((char*)&msgHeader, sizeof(msgHeader));
	g_ContentsJobQ.Enqueue(CreatePlayerMsg);

	InterlockedIncrement64(&g_playerCount);
}


void CWanServer::_OnSend(ULONG64 ID)
{
	//할 일 없음
	return;
}
void CWanServer::_OnDisConnect(ULONG64 ID)
{

	CPacket* DeletePlayerMsg;
	stHeader msgHeader;

	DeletePlayerMsg = CPacket::Alloc();
	msgHeader.type = stJob_DeletePlayer;

	*DeletePlayerMsg << ID;
	DeletePlayerMsg->PutData((char*)&msgHeader, sizeof(msgHeader));

	g_ContentsJobQ.Enqueue(DeletePlayerMsg);

	InterlockedDecrement64(&g_playerCount);
}
