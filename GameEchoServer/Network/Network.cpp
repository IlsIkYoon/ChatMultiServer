#include "Network.h"


void CWanServer::_OnMessage(CPacket* SBuf, ULONG64 ID)
{
	//Session이 어디 쓰레드에 있는지를 얻어내고
	//그 쓰레드에 넘겨 줌 
	//user job q 구조로 간다면 ?
	
	//플레이어의 메세지 큐에 메세지를 넣어 준다. (work)
	unsigned short PlayerIndex = GetIndex(ID);
	(*playerManager)[PlayerIndex].messageQ.Enqueue(SBuf);
	SBuf->IncrementUseCount();


	return;
}


void CWanServer::_OnAccept(ULONG64 ID)
{
	//여기서 authThread에 생성 요청.




}