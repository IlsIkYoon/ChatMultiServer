#include "ContentsResource.h"
#include "ContentsFunc.h"
#include "Player/Player.h"
#include "Msg/ContentsPacket.h"
#include "Msg/Message.h"
#include "Sector/Sector.h"
#include "ContentsThread/ContentsThreadManager.h"
#include "Msg/CommonProtocol.h"


LFreeQ<CPacket*> g_ContentsJobQ[CONTENTS_THREADCOUNT];

extern Player* g_PlayerArr;

extern std::list<Player*> Sector[dfRANGE_MOVE_RIGHT / SECTOR_RATIO][dfRANGE_MOVE_BOTTOM / SECTOR_RATIO];
extern std::mutex SectorLock[dfRANGE_MOVE_RIGHT / SECTOR_RATIO][dfRANGE_MOVE_BOTTOM / SECTOR_RATIO];

extern long long g_playerCount;

extern CLanServer* ntServer;

extern unsigned long long g_HeartBeatOverCount;


void PrintString()
{

}







void CLanServer::_OnMessage(CPacket* message, ULONG64 sessionID)
{
	unsigned short playerIndex;
	WORD contentsType;
	*message >> contentsType;
	playerIndex = GetIndex(sessionID);

	
	if (g_PlayerArr[playerIndex]._status != static_cast<BYTE>(Player::STATUS::PLAYER)
		&& contentsType != en_PACKET_CS_CHAT_REQ_LOGIN)
	{
		ntServer->DisconnectSession(sessionID);
		return;
	}

	g_PlayerArr[playerIndex]._timeOut = timeGetTime();

	switch (contentsType)
	{
	case en_PACKET_CS_CHAT_REQ_LOGIN:
		HandleLoginMessage(message, sessionID);
		break;

	case en_PACKET_CS_CHAT_REQ_SECTOR_MOVE:
		HandleSectorMoveMessage(message, sessionID);
		break;

	case en_PACKET_CS_CHAT_REQ_MESSAGE:
		HandleChatMessage(message, sessionID);
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

	g_PlayerArr[playerIndex]._status = static_cast<unsigned short>(Player::STATUS::SESSION);
	g_PlayerArr[playerIndex]._timeOut = timeGetTime(); 

}
void CLanServer::_OnSend(ULONG64 ID)
{
	//할 일 없음
	return;
}
void CLanServer::_OnDisConnect(ULONG64 sessionID)
{

	unsigned short playerIndex = GetIndex(sessionID);


	if (g_PlayerArr[playerIndex].GetID() != sessionID)
	{
		return;
	}

	if (g_PlayerArr[playerIndex]._status < static_cast<BYTE>(Player::STATUS::PLAYER))
	{
		g_PlayerArr[playerIndex]._status = static_cast<BYTE>(Player::STATUS::IDLE);
		return;
	}

	CheckSector(sessionID);

	int SectorX = g_PlayerArr[playerIndex].GetX();
	int SectorY = g_PlayerArr[playerIndex].GetY();

	SectorLock[SectorX][SectorY].lock();
	Sector[SectorX][SectorY].remove(&g_PlayerArr[playerIndex]);
	SectorLock[SectorX][SectorY].unlock();

	g_PlayerArr[playerIndex].Clear();
}

CLanServer::CLanServer()
{
}


bool HandleContentJob(long myIndex)
{
	CPacket* JobMessage;
	stHeader contentsHeader;
	CPacket* msgPayload;
	ULONG64 userId;


	{
		Profiler p("g_ContentsJobQ_Deque");

		JobMessage = CContentsThreadManager::contentsJobQ[myIndex].Dequeue();
	}


	if (JobMessage->GetDataSize() < sizeof(userId))
	{
		__debugbreak();
	}

	JobMessage->PopFrontData(sizeof(userId), (char*) & userId);


	while (1)
	{
		if (JobMessage->GetDataSize() == 0)
		{
			break;
		}
		if (JobMessage->GetDataSize() < sizeof(contentsHeader))
		{
			//데이터 크기가 안 맞음
			__debugbreak();
		}

		JobMessage->PopFrontData(sizeof(contentsHeader), (char*)&contentsHeader);

		msgPayload = CPacket::Alloc();

		if (contentsHeader.size > 0)
		{
			JobMessage->PopFrontData(contentsHeader.size, msgPayload->GetBufferPtr());
			msgPayload->MoveRear(contentsHeader.size);
		}



		switch (contentsHeader.type)
		{
		case stPacket_Client_Chat_MoveStart:
			HandleMoveStartMsg(msgPayload, userId);
			break;

		case stPacket_Client_Chat_MoveStop:
			HandleMoveStopMsg(msgPayload, userId);
			break;

		case stPacket_Client_Chat_LocalChat:
			HandleLocalChatMsg(msgPayload, userId);
			break;

		case stPacket_Client_Chat_HeartBeat:
			HandleHeartBeatMsg(userId);
			break;

		case stPacket_Client_Chat_ChatEnd:
			HandleChatEndMsg(userId);
			break;
		case stJob_CreatePlayer:
			HandleCreatePlayer(userId);
			break;

		case stJob_DeletePlayer:
			HandleDeletePlayer(userId);
			break;

		default:
			ntServer->DisconnectSession(userId);
			__debugbreak(); //todo//나중엔 지워야 함//
			break;

		}


		msgPayload->DecrementUseCount();

	}

	JobMessage->DecrementUseCount();

	return true;
}





bool InitContentsResource()
{
	g_PlayerArr = new Player[ntServer->GetSessionCount()];

	return true;
}




void TimeOutCheck()
{
	DWORD deadLine = timeGetTime() - dfNETWORK_PACKET_RECV_TIMEOUT;

	int sessionCount;
	sessionCount = ntServer->GetSessionCount();

	for (int i = 0; i < sessionCount; i++)
	{
		if (g_PlayerArr[i]._status == static_cast<BYTE>(Player::STATUS::IDLE))
		{
			continue;
		}

		if (g_PlayerArr[i]._timeOut < deadLine)
		{
			if (g_PlayerArr[i]._timeOut == 0)
			{
				continue;
			}
			ntServer->DisconnectSession(g_PlayerArr[i].GetID());
			InterlockedIncrement(&g_HeartBeatOverCount);
			
		}
	}

}