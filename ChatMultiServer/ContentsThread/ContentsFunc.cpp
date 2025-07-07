#include "ContentsResource.h"
#include "ContentsFunc.h"
#include "Player/Player.h"
#include "Msg/ContentsPacket.h"
#include "Msg/Message.h"
#include "Sector/Sector.h"
#include "ContentsThread/ContentsThreadManager.h"
#include "Msg/CommonProtocol.h"
#include "MonitorManager.h"
//-------------------------------------
// 에러 메세지 종료에 대한 카운트
//-------------------------------------
extern unsigned long long g_ErrorSectorSize;
extern unsigned long long g_ErrorNetworkLen;
extern unsigned long long g_ErrorChatMsgLen;
extern unsigned long long g_ErrorPacketType;

LFreeQ<CPacket*> g_ContentsJobQ[CONTENTS_THREADCOUNT];

extern std::list<Player*> Sector[SECTOR_MAX][SECTOR_MAX];
extern std::recursive_mutex SectorLock[SECTOR_MAX][SECTOR_MAX];

extern long long g_playerCount;

extern CWanServer* networkServer;

extern unsigned long long g_HeartBeatOverCount;

extern CContentsThreadManager* contentsManager;

extern CMonitorManager g_MonitorManager;
extern CPdhManager g_PDH;

//출력용 카운팅 변수들 DEBUG//
unsigned long long g_loginMsgCnt;
unsigned long long g_sectorMoveMsgCnt;
unsigned long long g_chatMsgCnt;

extern unsigned long long g_TotalPlayerCreate;
extern unsigned long long g_PlayerLogInCount;
extern unsigned long long g_PlayerLogOut;

extern unsigned long long g_CPacketAllocCount;

extern CCpuUsage g_CpuUsage;

unsigned long long g_UpdateMsgTps;

void CWanServer::_OnMessage(CPacket* message, ULONG64 sessionID)
{
	unsigned short playerIndex;
	WORD contentsType;
	Player* localPlayerList;

	InterlockedIncrement(&g_UpdateMsgTps);

	if (message->GetDataSize() < sizeof(contentsType))
	{
		std::string error;
		error += "Incomplete Message || Contents Header size Error";
		
		EnqueLog(error);

		DisconnectSession(sessionID);
		return;
	}

	*message >> contentsType;
	playerIndex = GetIndex(sessionID);
	localPlayerList = contentsManager->playerList->playerArr;

	localPlayerList[playerIndex]._timeOut = timeGetTime();

	switch (contentsType)
	{
	case en_PACKET_CS_CHAT_REQ_LOGIN:
	{
		InterlockedIncrement(&g_loginMsgCnt);
		HandleLoginMessage(message, sessionID);
	}
		break;

	case en_PACKET_CS_CHAT_REQ_SECTOR_MOVE:
	{
		InterlockedIncrement(&g_sectorMoveMsgCnt);
		HandleSectorMoveMessage(message, sessionID);
	}
		break;

	case en_PACKET_CS_CHAT_REQ_MESSAGE:
	{
		InterlockedIncrement(&g_chatMsgCnt);
		HandleChatMessage(message, sessionID);
	}
		break;

	case en_PACKET_CS_CHAT_REQ_HEARTBEAT:
		break;

	default:
		InterlockedIncrement(&g_ErrorPacketType);
		networkServer->DisconnectSession(sessionID);
		break;

	}
}



//commit
void ShutDownAllThread()
{
	

}

void CWanServer::_OnAccept(ULONG64 sessionID)
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
void CWanServer::_OnSend(ULONG64 ID)
{
	//할 일 없음
	return;
}
void CWanServer::_OnDisConnect(ULONG64 sessionID)
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

	contentsManager->keyList->DeleteID(localPlayerList[playerIndex].accountNo, sessionID);
	localPlayerList[playerIndex].Clear();

}

CWanServer::CWanServer()
{
}

void TimeOutCheck()
{
	return;


	DWORD deadLine = timeGetTime() - dfNETWORK_PACKET_RECV_TIMEOUT;

	int sessionCount;
	sessionCount = networkServer->GetSessionMaxCount();
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
			networkServer->DisconnectSession(localPlayerList[i].GetID());
			InterlockedIncrement(&g_HeartBeatOverCount);
			
		}
	}

}

bool UpdateMonitorData()
{
	//데이터 정산 후에 모니터 데이터로 보내주는 함수
	int localPlayerCount = g_PlayerLogInCount;
	int localSessionCount = g_LoginSessionCount;
	int localProcessTotal;
	int localUpdateMsgTps;
	double privateMem;
	g_PDH.GetMemoryData(&privateMem, nullptr, nullptr, nullptr);
	g_CpuUsage.UpdateCpuTime();
	localProcessTotal = (int)g_CpuUsage.ProcessTotal();

	localUpdateMsgTps = (int)InterlockedExchange(&g_UpdateMsgTps, 0);

	g_MonitorManager.UpdateMonitor(dfMONITOR_DATA_TYPE_CHAT_SERVER_RUN, 1);
	g_MonitorManager.UpdateMonitor(dfMONITOR_DATA_TYPE_CHAT_SERVER_CPU, localProcessTotal);
	g_MonitorManager.UpdateMonitor(dfMONITOR_DATA_TYPE_CHAT_SESSION, localSessionCount);
	g_MonitorManager.UpdateMonitor(dfMONITOR_DATA_TYPE_CHAT_PLAYER, localPlayerCount);
	g_MonitorManager.UpdateMonitor(dfMONITOR_DATA_TYPE_CHAT_SERVER_MEM, (int)privateMem / 1024 / 1024);
	g_MonitorManager.UpdateMonitor(dfMONITOR_DATA_TYPE_CHAT_UPDATE_TPS, localUpdateMsgTps);
	g_MonitorManager.UpdateMonitor(dfMONITOR_DATA_TYPE_CHAT_PACKET_POOL, (int)g_CPacketAllocCount);



	/*
		dfMONITOR_DATA_TYPE_CHAT_SERVER_RUN = 30,		// 채팅서버 ChatServer 실행 여부 ON / OFF
		dfMONITOR_DATA_TYPE_CHAT_SERVER_CPU = 31,		// 채팅서버 ChatServer CPU 사용률
		dfMONITOR_DATA_TYPE_CHAT_SERVER_MEM = 32,		// 채팅서버 ChatServer 메모리 사용 MByte
		dfMONITOR_DATA_TYPE_CHAT_SESSION = 33,		// 채팅서버 세션 수 (컨넥션 수)
		dfMONITOR_DATA_TYPE_CHAT_PLAYER = 34,		// 채팅서버 인증성공 사용자 수 (실제 접속자)
		dfMONITOR_DATA_TYPE_CHAT_UPDATE_TPS = 35,		// 채팅서버 UPDATE 스레드 초당 초리 횟수
		dfMONITOR_DATA_TYPE_CHAT_PACKET_POOL = 36,		// 채팅서버 패킷풀 사용량
		dfMONITOR_DATA_TYPE_CHAT_UPDATEMSG_POOL = 37,		// 채팅서버 UPDATE MSG 풀 사용량
	*/

	return true;
}