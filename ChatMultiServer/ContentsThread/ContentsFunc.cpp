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
extern std::recursive_mutex SectorLock[SECTOR_MAX][SECTOR_MAX];

extern long long g_playerCount;

extern CLanServer* ntServer;

extern unsigned long long g_HeartBeatOverCount;

extern CContentsThreadManager* contentsManager;


//출력용 카운팅 변수들 DEBUG//
unsigned long long g_loginMsgCnt;
unsigned long long g_sectorMoveMsgCnt;
unsigned long long g_chatMsgCnt;

extern unsigned long long g_TotalPlayerCreate;
extern unsigned long long g_PlayerLogInCount;
extern unsigned long long g_PlayerLogOut;




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
		InterlockedIncrement(&g_loginMsgCnt);
		HandleLoginMessage(message, sessionID);
		break;

	case en_PACKET_CS_CHAT_REQ_SECTOR_MOVE:
		InterlockedIncrement(&g_sectorMoveMsgCnt);
		HandleSectorMoveMessage(message, sessionID);
		break;

	case en_PACKET_CS_CHAT_REQ_MESSAGE:
		InterlockedIncrement(&g_chatMsgCnt);
		HandleChatMessage(message, sessionID);
		if (g_chatMsgCnt % 1000 == 0)
		{
		//	__debugbreak();
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

	if (localPlayerList[playerIndex]._status != static_cast<BYTE>(Player::STATUS::IDLE))
	{
		__debugbreak();
	}

	localPlayerList[playerIndex]._status = static_cast<BYTE>(Player::STATUS::SESSION);
	localPlayerList[playerIndex]._sessionID = sessionID;
	localPlayerList[playerIndex]._timeOut = timeGetTime();
	localPlayerList[playerIndex].sectorX = 0;
	localPlayerList[playerIndex].sectorY = 0;
	localPlayerList[playerIndex].accountNo = 0;
	localPlayerList[playerIndex]._move = false;

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


	if (localPlayerList[playerIndex]._sessionID != sessionID)
	{
		__debugbreak();
		return;
	}


	if (localPlayerList[playerIndex]._status < static_cast<BYTE>(Player::STATUS::PLAYER))
	{
		if (localPlayerList[playerIndex]._status == static_cast<BYTE>(Player::STATUS::PENDING_SECTOR))
		{
			InterlockedDecrement(&g_PlayerLogInCount);
			InterlockedIncrement(&g_PlayerLogOut);
		}

		localPlayerList[playerIndex]._status = static_cast<BYTE>(Player::STATUS::IDLE);
		return;
	}

	InterlockedDecrement(&g_PlayerLogInCount);
	InterlockedIncrement(&g_PlayerLogOut);

	CheckSector(sessionID);

	int SectorX = localPlayerList[playerIndex].sectorX;
	int SectorY = localPlayerList[playerIndex].sectorY;

	{
		std::lock_guard guard(SectorLock[SectorX][SectorY]);
		Sector[SectorX][SectorY].remove(&localPlayerList[playerIndex]);
	}

	localPlayerList[playerIndex].Clear();


}

CLanServer::CLanServer()
{
}

void TimeOutCheck()
{
	return;


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