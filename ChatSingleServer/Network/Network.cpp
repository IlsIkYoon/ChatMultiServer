#include "Network.h"
#include "Contents/ContentsPacket.h"

extern LFreeQ<Job> g_ContentsJobQ;
extern long long g_playerCount;


void CWanServer::_OnMessage(CPacket* message, ULONG64 ID)
{
	Job enqueMessage;
	enqueMessage.ID = ID;
	enqueMessage.packet = message;
	message->IncrementUseCount(); // 넣었다는 의미
	g_ContentsJobQ.Enqueue(enqueMessage);
}


void CWanServer::_OnAccept(ULONG64 ID)
{
	Job enqueMessage;
	CPacket* CreatePlayerMsg;

	WORD type = stJob_CreatePlayer;

	CreatePlayerMsg = CPacket::Alloc();
	*CreatePlayerMsg << type;
	*CreatePlayerMsg << ID;

	enqueMessage.ID = SERVER_ID;
	enqueMessage.packet = CreatePlayerMsg;

	g_ContentsJobQ.Enqueue(enqueMessage);

	InterlockedIncrement64(&g_playerCount);
}


void CWanServer::_OnSend(ULONG64 ID)
{
	//할 일 없음
	return;
}
void CWanServer::_OnDisConnect(ULONG64 ID)
{
	Job enqueMessage;
	CPacket* DeletePlayerMsg;

	WORD type = stJob_DeletePlayer;

	DeletePlayerMsg = CPacket::Alloc();

	*DeletePlayerMsg << type;
	*DeletePlayerMsg << ID;

	enqueMessage.ID = SERVER_ID;
	enqueMessage.packet = DeletePlayerMsg;

	g_ContentsJobQ.Enqueue(enqueMessage);

	InterlockedDecrement64(&g_playerCount);
}
