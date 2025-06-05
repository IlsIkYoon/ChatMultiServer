#include "ContentsResource.h"
#include "ContentsFunc.h"
#include "Player/Player.h"
#include "Msg/ContentsPacket.h"
#include "Msg/Message.h"
#include "Sector/Sector.h"
#include "ContentsThread/ContentsThreadManager.h"
#include "Msg/CommonProtocol.h"


LFreeQ<CPacket*> g_ContentsJobQ[CONTENTS_THREADCOUNT];

extern std::list<Player*> Sector[SECTOR_MAX][SECTOR_MAX];
extern std::mutex SectorLock[SECTOR_MAX][SECTOR_MAX];

extern long long g_playerCount;

extern CLanServer* ntServer;

extern unsigned long long g_HeartBeatOverCount;

extern CContentsThreadManager* contentsManager;


//출력용 카운팅 변수들 DEBUG//
unsigned long long g_loginMsgCnt;
unsigned long long g_sectorMoveMsgCnt;
unsigned long long g_chatMsgCnt;




void PrintString()
{

}







void CLanServer::_OnMessage(CPacket* message, ULONG64 sessionID)
{
	unsigned short playerIndex;
	WORD contentsType;
	Player* localPlayerList;

	*message >> contentsType;
	playerIndex = GetIndex(sessionID);
	localPlayerList = contentsManager->playerList->playerArr;

	localPlayerList[playerIndex]._timeOut = timeGetTime();

	switch (contentsType)
	{
	case en_PACKET_CS_CHAT_REQ_LOGIN:
		HandleLoginMessage(message, sessionID);
		InterlockedIncrement(&g_loginMsgCnt);
		break;

	case en_PACKET_CS_CHAT_REQ_SECTOR_MOVE:
		HandleSectorMoveMessage(message, sessionID);
		InterlockedIncrement(&g_sectorMoveMsgCnt);
		break;

	case en_PACKET_CS_CHAT_REQ_MESSAGE:
		InterlockedIncrement(&g_chatMsgCnt);
		HandleChatMessage(message, sessionID);
		if (g_chatMsgCnt % 1000 == 0)
		{
			__debugbreak();
		}
		break;

	case en_PACKET_CS_CHAT_REQ_HEARTBEAT:
		break;

	default:
		ntServer->DisconnectSession(sessionID);
		__debugbreak(); 
		break;

	}
}



//commit
void ShutDownAllThread()
{
	

}

void CLanServer::_OnAccept(ULONG64 sessionID)
{
	unsigned short playerIndex = GetIndex(sessionID);

	Player* localPlayerList = contentsManager->playerList->playerArr;

	localPlayerList[playerIndex]._status = static_cast<unsigned short>(Player::STATUS::SESSION);
	localPlayerList[playerIndex]._sessionID = sessionID;
	localPlayerList[playerIndex]._timeOut = timeGetTime();

}
void CLanServer::_OnSend(ULONG64 ID)
{
	//할 일 없음
	return;
}
void CLanServer::_OnDisConnect(ULONG64 sessionID)
{

	unsigned short playerIndex = GetIndex(sessionID);

	Player* localPlayerList = contentsManager->playerList->playerArr;


	if (localPlayerList[playerIndex].GetID() != sessionID)
	{
		return;
	}

	if (localPlayerList[playerIndex]._status < static_cast<BYTE>(Player::STATUS::PLAYER))
	{
		localPlayerList[playerIndex]._status = static_cast<BYTE>(Player::STATUS::IDLE);
		return;
	}

	CheckSector(sessionID);

	int SectorX = localPlayerList[playerIndex].GetX();
	int SectorY = localPlayerList[playerIndex].GetY();

	SectorLock[SectorX][SectorY].lock();
	Sector[SectorX][SectorY].remove(&localPlayerList[playerIndex]);
	SectorLock[SectorX][SectorY].unlock();

	localPlayerList[playerIndex].Clear();
}

CLanServer::CLanServer()
{
}

void TimeOutCheck()
{
	DWORD deadLine = timeGetTime() - dfNETWORK_PACKET_RECV_TIMEOUT;

	int sessionCount;
	sessionCount = ntServer->GetSessionCount();
	Player* localPlayerList;

	localPlayerList = contentsManager->playerList->playerArr;

	

	for (int i = 0; i < sessionCount; i++)
	{
		if (localPlayerList[i]._status == static_cast<BYTE>(Player::STATUS::IDLE))
		{
			continue;
		}

		if (localPlayerList[i]._timeOut < deadLine)
		{
			if (localPlayerList[i]._timeOut == 0)
			{
				continue;
			}
			ntServer->DisconnectSession(localPlayerList[i].GetID());
			InterlockedIncrement(&g_HeartBeatOverCount);
			
		}
	}

}