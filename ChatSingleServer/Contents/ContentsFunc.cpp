#include "ContentsFunc.h"
#include "Player/Player.h"
#include "ContentsPacket.h"
#include "Message/Message.h"
#include "Sector/Sector.h"

#ifdef __JOBQLOCKFREEQ_
LFreeQ<CPacket*> g_ContentsJobQ;

#else
RingBuffer g_ContentsJobQ;
std::mutex g_ContentsJobQ_Lock;
#endif
extern Player* g_PlayerArr;

extern std::list<Player*> Sector[dfRANGE_MOVE_RIGHT / SECTOR_RATIO][dfRANGE_MOVE_BOTTOM / SECTOR_RATIO];

extern long long g_playerCount;

extern CLanServer* pLib;

extern unsigned long long g_HeartBeatOverCount;


void PrintString()
{

}


void CLanServer::_OnMessage(CPacket* message, ULONG64 ID)
{
	//여기서 이제 JobQ에 넣어주기만 할 것임
	unsigned int enqueResult;
	ULONG64 localID = GetID(ID);
	
	CPacket* localMessage = (CPacket*)message;
	CPacket* EnqueMessage = CPacket::Alloc();

	*EnqueMessage << ID;
	EnqueMessage->PutData(localMessage->GetDataPtr(), localMessage->GetDataSize());
	{
		Profiler p("g_ContentsJobQ_Enque");

#ifdef __JOBQLOCKFREEQ_
		g_ContentsJobQ.Enqueue(EnqueMessage);
#else
		g_ContentsJobQ_Lock.lock();
		g_ContentsJobQ.Enqueue((char*)&EnqueMessage, sizeof(EnqueMessage), &enqueResult);
		g_ContentsJobQ_Lock.unlock();

	if (enqueResult < sizeof(EnqueMessage))
	{
		__debugbreak(); //링버퍼에 공간이 없을 수 있음
	}
#endif
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


	//플레이어 카운트가 맥스인지를 확인하고
	//맥스라면 큐에 넣어줌

	msgHeader.type = stJob_CreatePlayer;
	
	CreatePlayerMsg = CPacket::Alloc();

	*CreatePlayerMsg << ID;
	CreatePlayerMsg->PutData((char*)&msgHeader, sizeof(msgHeader));
	{

		Profiler p("g_ContentsJobQ_Enque");
#ifdef __JOBQLOCKFREEQ_
		g_ContentsJobQ.Enqueue(CreatePlayerMsg);
#else
		g_ContentsJobQ_Lock.lock();
		g_ContentsJobQ.Enqueue((char*)&CreatePlayerMsg, sizeof(CreatePlayerMsg), &enqueResult);
		g_ContentsJobQ_Lock.unlock();
#endif
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
	unsigned int dequeResult;

	DeletePlayerMsg = CPacket::Alloc();
	msgHeader.type = stJob_DeletePlayer;

	*DeletePlayerMsg << ID;
	DeletePlayerMsg->PutData((char*)&msgHeader, sizeof(msgHeader));

	{
		Profiler p("g_ContentsJobQ_Enque");

#ifdef __JOBQLOCKFREEQ_
		g_ContentsJobQ.Enqueue(DeletePlayerMsg);
#else
		g_ContentsJobQ_Lock.lock();
		g_ContentsJobQ.Enqueue((char*)&DeletePlayerMsg, sizeof(DeletePlayerMsg), &dequeResult);
		g_ContentsJobQ_Lock.unlock();
#endif
	}


	//todo//여기서 대기 큐에 있던 애가 있으면 생성에 대한 절차를 해줌




	InterlockedDecrement64(&g_playerCount);
}

CLanServer::CLanServer()
{
}

DWORD g_prevFrameTime;
DWORD g_fixedDeltaTime;
DWORD g_frame = 0;
DWORD g_sec;

unsigned int ContentsThreadFunc(void*)
{

	InitContentsResource();

	DWORD startTime = timeGetTime();

	DWORD dwUpdateTick = startTime - FrameSec;
	g_sec = startTime / 1000;

	g_prevFrameTime = startTime - FrameSec;// 초기 값 설정

	while (1)
	{
		DWORD currentTime = timeGetTime();
		DWORD deltaTime = currentTime - g_prevFrameTime;
		DWORD deltaCount = deltaTime / FrameSec;
		g_fixedDeltaTime = deltaCount * FrameSec;

#ifdef __JOBQLOCKFREEQ_
		while(g_ContentsJobQ.GetSize() != 0)
#else
		while(g_ContentsJobQ.IsEmpty() == false)
#endif
		{
			HandleContentJob();
		}

		UpdateContentsLogic(g_fixedDeltaTime);

		pLib->EnqueSendRequest();

		DWORD logicTime = timeGetTime() - currentTime;

		if (logicTime < FrameSec)
		{

			Sleep(FrameSec - logicTime);
		}

		g_frame++;

		g_prevFrameTime += g_fixedDeltaTime;

		


	}



	return 0;
}

bool HandleContentJob()
{
	unsigned int dequeResult;
	CPacket* JobMessage;
	stHeader contentsHeader;
	CPacket* msgPayload;
	ULONG64 userId;

	ULONG_PTR CPacketPtr;

	//Todo // 락프리큐랑 링버퍼 성능 비교해보기
	{
		Profiler p("g_ContentsJobQ_Deque");

#ifdef __JOBQLOCKFREEQ_
		JobMessage = g_ContentsJobQ.Dequeue();
#else
		g_ContentsJobQ.Dequeue((char*)&CPacketPtr, sizeof(CPacketPtr), &dequeResult);
		if (dequeResult < sizeof(CPacketPtr))
		{
			__debugbreak();
			return false;
		}
		JobMessage = (CPacket*)CPacketPtr;
#endif
	}


	if (JobMessage->GetDataSize() < sizeof(userId))
	{
		__debugbreak();
	}

	JobMessage->PopFrontData(sizeof(userId), (char*) & userId);


		if (JobMessage->GetDataSize() < sizeof(contentsHeader))
		{
			//데이터 크기가 안 맞음 //Len만큼도 안 왔다
			__debugbreak();
		}

		JobMessage->PopFrontData(sizeof(contentsHeader), (char*)&contentsHeader);

		switch (contentsHeader.type)
		{
		case stPacket_Client_Chat_MoveStart:
			HandleMoveStartMsg(JobMessage, userId);
			break;

		case stPacket_Client_Chat_MoveStop:
			HandleMoveStopMsg(JobMessage, userId);
			break;

		case stPacket_Client_Chat_LocalChat:
			HandleLocalChatMsg(JobMessage, userId);
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
			pLib->DisconnectSession(userId);
			__debugbreak(); //todo//나중엔 지워야 함//
			break;

		}

	JobMessage->DecrementUseCount();

	return true;
}





bool InitContentsResource()
{
	g_PlayerArr = new Player[pLib->_sessionMaxCount];

	return true;
}



void UpdateContentsLogic(DWORD deltaTime)
{
	int sessionCount;
	sessionCount = pLib->_sessionMaxCount;

	for (int i = 0; i < sessionCount; i++)
	{
		if (g_PlayerArr[i].isAlive() == false)
		{
			continue;
		}

		CheckSector(g_PlayerArr[i].GetID());
		g_PlayerArr[i].Move(deltaTime);
		CheckSector(g_PlayerArr[i].GetID());

	}

	TimeOutCheck();



}


void TimeOutCheck()
{
	DWORD deadLine = timeGetTime() - dfNETWORK_PACKET_RECV_TIMEOUT;

	int sessionCount;
	sessionCount = pLib->_sessionMaxCount;

	for (int i = 0; i < sessionCount; i++)
	{
		if (g_PlayerArr[i].isAlive() == false)
		{
			continue;
		}

		/*
		if (g_PlayerArr[i]._timeOut < deadLine)
		{
			pLib->DisconnectSession(g_PlayerArr[i].GetID());
			InterlockedIncrement(&g_HeartBeatOverCount);
			
		}
		*/
	}

}