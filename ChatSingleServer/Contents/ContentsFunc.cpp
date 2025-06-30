#include "ContentsFunc.h"
#include "Player/Player.h"
#include "ContentsPacket.h"
#include "Message/Message.h"
#include "Sector/Sector.h"
#include "Network/Network.h"

LFreeQ<Job> g_ContentsJobQ;
extern Player* g_PlayerArr;

extern std::list<Player*> Sector[dfRANGE_MOVE_RIGHT / SECTOR_RATIO][dfRANGE_MOVE_BOTTOM / SECTOR_RATIO];

extern long long g_playerCount;

extern CWanServer* pLib;

extern unsigned long long g_HeartBeatOverCount;


void PrintString()
{

}
void ShutDownAllThread()
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

		while(g_ContentsJobQ.GetSize() != 0)
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

		InterlockedIncrement(&g_frame);

		g_prevFrameTime += g_fixedDeltaTime;

	}



	return 0;
}

bool HandleContentJob()
{
	Job currentJob;
	WORD contentsType;
	ULONG64 userId;
	CPacket* JobMessage;

	currentJob = g_ContentsJobQ.Dequeue();

	JobMessage = currentJob.packet;
	userId = currentJob.ID;

	if (JobMessage->GetDataSize() < sizeof(WORD))
	{
		std::string logString;
		logString += "Incomplete Packet Error || ID : ";
		logString += std::to_string(userId);

		pLib->EnqueLog(logString);
		pLib->DisconnectSession(userId);
		JobMessage->DecrementUseCount();
		return false;
	}

		*JobMessage >> contentsType;

		switch (contentsType)
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
			HandleCreatePlayer(JobMessage, userId);
			break;

		case stJob_DeletePlayer:
			HandleDeletePlayer(JobMessage, userId);
			break;

		default:
		{
			std::string logString;
			logString += "Packet Type Error || ID : ";
			logString += std::to_string(userId);

			pLib->EnqueLog(logString);

			pLib->DisconnectSession(userId);
			JobMessage->DecrementUseCount();
			return false;
		}
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