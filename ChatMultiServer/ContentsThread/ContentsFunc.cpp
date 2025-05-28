#include "ContentsResource.h"
#include "ContentsFunc.h"
#include "Player/Player.h"
#include "Msg/ContentsPacket.h"
#include "Msg/Message.h"
#include "Sector/Sector.h"
#include "ContentsThread/ContentsThreadManager.h"


LFreeQ<CPacket*> g_ContentsJobQ[CONTENTS_THREADCOUNT];

extern Player* g_PlayerArr;

extern std::list<Player*> Sector[dfRANGE_MOVE_RIGHT / SECTOR_RATIO][dfRANGE_MOVE_BOTTOM / SECTOR_RATIO];

extern long long g_playerCount;

extern CLanServer* ntServer;

extern unsigned long long g_HeartBeatOverCount;


void PrintString()
{

}







void CLanServer::_OnMessage(char* message, ULONG64 ID)
{
	//여기서 이제 JobQ에 넣어주기만 할 것임
	int QIndex;
	ULONG64 localID = GetID(ID);
	QIndex = GetIndex(ID) % CContentsThreadManager::threadCount;
	
	CPacket* localMessage = (CPacket*)message;
	CPacket* EnqueMessage = CPacket::Alloc();

	//ID를 넣고 데이터를 넣는 중
	EnqueMessage->operator<<(ID);
	EnqueMessage->PutData(localMessage->GetDataPtr(), localMessage->GetDataSize());
	{
		Profiler p("g_ContentsJobQ_Enque");

		CContentsThreadManager::contentsJobQ[QIndex].Enqueue(EnqueMessage);
	}
}



//commit
void ShutDownAllThread()
{
	

}

void CLanServer::_OnAccept(ULONG64 ID)
{
	unsigned int enqueResult;
	CPacket* CreatePlayerMsg;
	stHeader msgHeader;
	int QIndex;

	//플레이어 카운트가 맥스인지를 확인하고
	//맥스라면 큐에 넣어줌

	QIndex = NetWorkManager::GetIndex(ID) % CContentsThreadManager::threadCount;

	msgHeader.type = stJob_CreatePlayer;
	msgHeader.size = 0;
	
	CreatePlayerMsg = CPacket::Alloc();

	*CreatePlayerMsg << ID;
	CreatePlayerMsg->PutData((char*)&msgHeader, sizeof(msgHeader));
	{

		Profiler p("g_ContentsJobQ_Enque");
		CContentsThreadManager::contentsJobQ[QIndex].Enqueue(CreatePlayerMsg);
	}

	InterlockedIncrement64(&g_playerCount);

}
void CLanServer::_OnSend(ULONG64 ID)
{
	//할 일 없음
	return;
}
void CLanServer::_OnDisConnect(ULONG64 ID)
{
	
	CPacket* DeletePlayerMsg;
	stHeader msgHeader;
	int QIndex;
	QIndex = GetIndex(ID) % CContentsThreadManager::threadCount;

	DeletePlayerMsg = CPacket::Alloc();
	msgHeader.type = stJob_DeletePlayer;
	msgHeader.size = 0;

	*DeletePlayerMsg << ID;
	DeletePlayerMsg->PutData((char*)&msgHeader, sizeof(msgHeader));

	{
		Profiler p("g_ContentsJobQ_Enque");

		CContentsThreadManager::contentsJobQ[QIndex].Enqueue(DeletePlayerMsg);
	}


	//todo//여기서 대기 큐에 있던 애가 있으면 생성에 대한 절차를 해줌




	InterlockedDecrement64(&g_playerCount);
}

CLanServer::CLanServer()
{
}


bool HandleContentJob(long myIndex)
{
	unsigned int dequeResult;
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
			HandleMoveStartMsg((char*)msgPayload, userId);
			break;

		case stPacket_Client_Chat_MoveStop:
			HandleMoveStopMsg((char*)msgPayload, userId);
			break;

		case stPacket_Client_Chat_LocalChat:
			HandleLocalChatMsg((char*)msgPayload, userId);
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
		if (g_PlayerArr[i].isAlive() == false)
		{
			continue;
		}

		if (g_PlayerArr[i]._timeOut < deadLine)
		{
			ntServer->DisconnectSession(g_PlayerArr[i].GetID());
			InterlockedIncrement(&g_HeartBeatOverCount);
			
		}
	}

}