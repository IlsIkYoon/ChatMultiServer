#include "resource.h"
#include "DummyThread.h"
#include "Player/DummySession.h"
#include "Action/Action.h"
#include "Msg/ContentsPacket.h"
#include "Network/ThreadNetworkManager.h"
#include "Msg/RandStringManager.h"

extern DWORD g_threadCount;
extern DWORD g_ThreadIndex;
extern DWORD g_clientCount;
extern DWORD g_trustMode;

extern DummySession* g_DummySessionArr;

char g_currentAction = static_cast<char>(Action::Status::ACTION_CONNECT);
long* g_actionCompleteCount;
std::mutex actionCompleteLock;

extern std::list<DummySession*> g_Sector[dfRANGE_MOVE_RIGHT / SECTOR_RATIO][dfRANGE_MOVE_BOTTOM / SECTOR_RATIO];
extern std::mutex g_SectorLock[dfRANGE_MOVE_RIGHT / SECTOR_RATIO][dfRANGE_MOVE_BOTTOM / SECTOR_RATIO];

extern CRandStringManager g_RandStringManager;


thread_local DWORD g_prevFrameTime;
thread_local DWORD g_fixedDeltaTime;
thread_local DWORD g_frame = 0;
thread_local DWORD g_sec;

thread_local DWORD tls_myThreadIndex;

//--------------------------------------------
// 출력용 변수
//--------------------------------------------
unsigned long long g_moveStartPacket;
unsigned long long g_moveStopPacket;
unsigned long long g_recvdMoveStopPacket;

extern unsigned long long g_sendchatMsg;
extern unsigned long long g_recvdChatCompleteMsg;
unsigned long long g_sendLocalChatMsgTotal;
unsigned long long g_recvLocalChatMsgTotal;

extern unsigned long long g_sessionLoginCount;
extern unsigned long long g_sessionLogoutCount;

unsigned int DummyClientThreadFunc(void*)
{
	CThreadNetworkManager ThreadNetworkManager;

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


		ThreadNetworkManager.HandleNetworkEvent();

		HandleRecvData_All(ThreadNetworkManager.mySessionArr, ThreadNetworkManager.dwMySessionCount);

		ContentsWork(ThreadNetworkManager.mySessionArr, ThreadNetworkManager.dwMySessionCount, g_fixedDeltaTime); 

		//Sleep
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

bool ContentsWork(DummySession* mySessionArr, DWORD mySessionCount, DWORD fixedDeltaTime)
{
	//더미 쓰레드에서 돌아가는 컨텐츠 로직 함수
	//이동이 있다면 이동 로직 처리
	//페이즈를 나눠서 동작하게 됨
	//페이즈 자체는 전역에 두고 그걸 기반으로 모든 로직이 돌아감

	for(unsigned int i =0; i < mySessionCount; i++)
	{
		mySessionArr[i].Move(fixedDeltaTime);

	}


	if (CheckMyActionComplete() == true)
	{
		return false;
	}

	switch (g_currentAction)
	{
	case static_cast<char>(Action::Status::ACTION_CONNECT):
		HandleConnectAction(mySessionArr, mySessionCount);
		break;
	case static_cast<char>(Action::Status::ACTION_CHECKCHATMSG):
		HandleCheckMsgAction(mySessionArr, mySessionCount);
		break;

	case static_cast<char>(Action::Status::ACTION_MOVE):
		HandleMoveAction(mySessionArr, mySessionCount, fixedDeltaTime);
		break;
		
	case static_cast<char>(Action::Status::ACTION_MOVESTOP):
		HandleMoveStopAction(mySessionArr, mySessionCount);
		break;

	case static_cast<char>(Action::Status::ACTION_CHECKMOVESTOPMSG):
		HandleCheckMoveStopAction(mySessionArr, mySessionCount);
		break;

	case static_cast<char>(Action::Status::ACTION_SENDCHATMSG):
		HandleSendChatMsgAction(mySessionArr, mySessionCount);
		break;

	case static_cast<char>(Action::Status::ACTION_CHECKDISCONNECT):
		HandleCheckDisconnectAction(mySessionArr, mySessionCount);
		break;

	default:
		__debugbreak();
		break;
	}


	return true;
}



bool ThreadActionComplete()
{
	bool allActionComplete = true;
	bool retval = false;

	actionCompleteLock.lock();
	g_actionCompleteCount[tls_myThreadIndex] = THREAD_ACTION_COMPLETE;
	actionCompleteLock.unlock();

	for (unsigned int i = 0; i < g_threadCount; i++)
	{
		if (g_actionCompleteCount[i] == THREAD_ACTION_INCOMPLETE)
		{
			allActionComplete = false;
			break;
		}
	}

	if (allActionComplete == true)
	{
		retval = true;
	}

	return retval;
}


bool SendMoveStartMessage(DummySession* _session)
{
	unsigned int enqueResult;
	
	stHeader packetHeader;
	CPacket moveStartMsg;
	packetHeader.type = stPacket_Client_Chat_MoveStart;
	moveStartMsg << _session->_direction;
	moveStartMsg << _session->_x;
	moveStartMsg << _session->_y;
	packetHeader.size = moveStartMsg.GetDataSize();

	_session->SendBufferEnque((char*)&packetHeader, sizeof(packetHeader), &enqueResult);
	_session->SendBufferEnque(moveStartMsg.GetDataPtr(), moveStartMsg.GetDataSize(), &enqueResult);
	

	InterlockedIncrement(&g_moveStartPacket);
	_session->_move = true;

	return true;
}

bool SendMoveStopMessage(DummySession* _session)
{
	unsigned int enqueResult;
	stHeader packetHeader;
	CPacket moveStopMsg;
	packetHeader.type = stPacket_Client_Chat_MoveStop;
	moveStopMsg << _session->_direction;
	moveStopMsg << _session->_x;
	moveStopMsg << _session->_y;
	packetHeader.size = moveStopMsg.GetDataSize();

	_session->SendBufferEnque((char*)&packetHeader, sizeof(packetHeader), &enqueResult);
	_session->SendBufferEnque(moveStopMsg.GetDataPtr(), moveStopMsg.GetDataSize(), &enqueResult);

	_session->_move = false;
	InterlockedIncrement(&g_moveStopPacket);


	return true;
}

bool SendHeartBeatMessage(DummySession* _session)
{
	if (_session->dummyStatus < static_cast<long>(DummySession::DummyStatus::Connected))
	{
		return false;
	}

	unsigned int enqueResult;
	stHeader packetHeader;
	packetHeader.type = stPacket_Client_Chat_HeartBeat;
	packetHeader.size = 0;

	
	_session->SendBufferEnque((char*)&packetHeader, sizeof(packetHeader), &enqueResult);
	

	return true;
}

bool SendLocalChatMessage(DummySession* _session, char* chatMessage, int msgLen)
{
	unsigned int enqueResult;
	stHeader packetHeader;
	CPacket ChatMsg;
	char localLen;
	localLen = static_cast<char>(msgLen);

	packetHeader.type = stPacket_Client_Chat_LocalChat;
	ChatMsg << localLen;
	ChatMsg.PutData(chatMessage, localLen);
	packetHeader.size = sizeof(localLen) + localLen;


	_session->SendBufferEnque((char*)&packetHeader, sizeof(packetHeader), &enqueResult);
	_session->SendBufferEnque(ChatMsg.GetDataPtr(), ChatMsg.GetDataSize(), &enqueResult);
	
	
	return true;
}

bool SendChatEndMessage(DummySession* _session)
{
	unsigned int enqueResult;
	stHeader packetHeader;

	packetHeader.type = stPacket_Client_Chat_ChatEnd;
	packetHeader.size = 0;
	_session->SendBufferEnque((char*)&packetHeader, sizeof(packetHeader), &enqueResult);
	


	return true;
}



bool HandleRecvData_All(DummySession* mySessionArr, DWORD dwMySessionCount)
{
	for (unsigned int i = 0; i < dwMySessionCount; i++)
	{
		if (mySessionArr[i]._recvQ.IsEmpty() == true)
		{
			continue;
		}
		HandleRecvData_Session(&mySessionArr[i]);
	}

	return true;
}


bool HandleRecvData_Session(DummySession* _session)
{
	//들어온 데이터 패킷 뜯어서 로직 타는 함수
	CPacket* contentsPacket;
	int decodeRetval;

	while (1)
	{
		contentsPacket = CPacket::Alloc();
		decodeRetval = Decode_NetworkHeader(_session, contentsPacket);
		if (decodeRetval == false)
		{
			contentsPacket->DecrementUseCount();
			break;
		}

		while (1)
		{
			decodeRetval = Decode_ContentsPacket(_session, contentsPacket);
			if (decodeRetval == false)
			{
				break;
			}
		}


		contentsPacket->DecrementUseCount();
	}
	

	return true;
}


bool Decode_NetworkHeader(DummySession* _session,CPacket* ContentsPacket)
{
	
	ClientHeader ntHeader;
	unsigned int dequeResult;
	
	int decodeRetval;
	
	if (_session->_recvQ.GetSizeUsed() < sizeof(ntHeader)) {
		return false;
	}

	_session->RecvBufferDeque((char*)&ntHeader, sizeof(ntHeader), &dequeResult);

	if (_session->_recvQ.GetSizeUsed() < ntHeader._len)
	{
		RestoreNetworkHeader(ntHeader, _session);
		return false;
	}

	ContentsPacket->PutData((char*)&ntHeader, sizeof(ntHeader));
	_session->RecvBufferDeque(ContentsPacket->GetBufferPtr(), ntHeader._len, &dequeResult);
	ContentsPacket->MoveRear(ntHeader._len);
	decodeRetval = ContentsPacket->_ClientDecodePacket();

	if (decodeRetval == static_cast<int>(CPacket::ErrorCode::INVALID_DATA_PACKET))
	{
		__debugbreak();
		return false;
	}

	ContentsPacket->MoveFront(sizeof(ntHeader)); //네트워크 헤더를 제거해줌

	return true;
}


bool Decode_ContentsPacket(DummySession* _session, CPacket* ContentsPacket)
{
	//헤더 뜯고
	//헤더에 따라 switch문 타주는 함수
	stHeader contentsHeader;
	if (ContentsPacket->GetDataSize() == 0)
	{
		return false;
	}


	ContentsPacket->PopFrontData(sizeof(contentsHeader), (char*)&contentsHeader);

	if (ContentsPacket->GetDataSize() < contentsHeader.size)
	{
		__debugbreak();
	}

	switch (contentsHeader.type)
	{
	case stPacket_Chat_Client_CreateCharacter:
		CreateCharacter(_session, ContentsPacket);
		break;

	case stPacket_Chat_Client_LocalChat:
		HandleLocalChatMsg(_session, ContentsPacket);
		break;

	case stPacket_Chat_Client_MoveStopComplete:
		HandleMoveStopCompleteMsg(_session, ContentsPacket);
		break;

	case stPacket_Chat_Client_ChatComplete:
		HandleChatCompleteMsg(_session, ContentsPacket);
		break;

	default:
		__debugbreak();
		break;
	}

	return true;
}


bool CreateCharacter(DummySession* _session, CPacket* ContentsPacket)
{
	_session->_sessionLock.lock();
	*ContentsPacket >> _session->_ID._ulong64;
	*ContentsPacket >> _session->_x;
	*ContentsPacket >> _session->_y;

	int sectorX;
	int sectorY;

	sectorX = _session->_x / SECTOR_RATIO;
	sectorY = _session->_y / SECTOR_RATIO;

	g_SectorLock[sectorX][sectorY].lock();
	g_Sector[sectorX][sectorY].push_back(_session);
	g_SectorLock[sectorX][sectorY].unlock();

	_session->dummyStatus = static_cast<long>(DummySession::DummyStatus::Connected);
	_session->_sessionLock.unlock();

	InterlockedIncrement(&g_sessionLoginCount);

	return true;
}

bool HandleLocalChatMsg(DummySession* _session, CPacket* ContentsPacket)
{
	//localChat메세지가 오면 Session 맵에서 카운트 조절해주는 함수
	unsigned long long packetID;
	bool allRecvd = true;
	char packetLen;
	char* packetStr;
	*ContentsPacket >> packetID;
	*ContentsPacket >> packetLen;



	packetStr = (char*)malloc(sizeof(char) * (packetLen+1));

	ContentsPacket->PopFrontData(static_cast<int>(packetLen), packetStr);
	packetStr[packetLen] = NULL;


	if (g_trustMode != TRUST_MODE)
	{
		InterlockedIncrement(&g_recvLocalChatMsgTotal);
		free(packetStr);
		return true;
	}

	
	_session->_sessionLock.lock(); //SessionLock

	if (_session->stringCounter.count(packetStr) == 0)
	{
		__debugbreak();
	}
	if (InterlockedDecrement(&_session->stringCounter[packetStr]) < 0)
	{
		__debugbreak();
	}
	InterlockedIncrement(&g_recvLocalChatMsgTotal);

	for (auto& it : _session->stringCounter)
	{
		if (it.second != 0)
		{
			allRecvd = false;
			break;
		}
	}

	if (allRecvd == true)
	{
		_session->allMsgRecvd = true;
	}

	_session->_sessionLock.unlock(); //SessionUnlock



	free(packetStr);

	return true;
}

bool HandleMoveStopCompleteMsg(DummySession* _session, CPacket* ContentsPacket)
{
	
	_session->_sessionLock.lock();
	_session->dummyStatus = static_cast<long>(DummySession::DummyStatus::RecvdMoveEnd);
	_session->_sessionLock.unlock();
	InterlockedIncrement(&g_recvdMoveStopPacket);

	return true;
}

bool HandleChatCompleteMsg(DummySession* _session, CPacket* ContentsPacket)
{
	_session->_sessionLock.lock();
	_session->dummyStatus = static_cast<long>(DummySession::DummyStatus::RecvdChatComplete);
	_session->_sessionLock.unlock();
	InterlockedIncrement(&g_recvdChatCompleteMsg);

	return true;
}

bool CheckReceiversInSector(DummySession* _session, char* chatMsg, int msgLen)
{
	int sectorX;
	int sectorY;

	sectorX = _session->_x / SECTOR_RATIO;
	sectorY = _session->_y / SECTOR_RATIO;
	
	//섹터 기준으로 9방향 섹터 다 검사해서 메세지 카운트 올려주기
	for (int i = -1; i < 2; i++)
	{
		if (sectorX + i < 0 || sectorX + i >= MAX_SECTOR_X)
		{
			continue;
		}

		for (int j = -1; j < 2; j++)
		{
			if (sectorY + j < 0 || sectorY + j >= MAX_SECTOR_Y)
			{
				continue;
			}
			//섹터 내 리스트 돌면서 문자열 카운트 올려주기
			g_SectorLock[sectorX + i][sectorY + j].lock();
			for (auto& it : g_Sector[sectorX + i][sectorY + j])
			{
				if (it->_ID._ulong64 == _session->_ID._ulong64)
				{
					continue;
				}


				it->_sessionLock.lock();
				if (it->stringCounter.count(chatMsg) == 0)
				{
					__debugbreak();
				}

				it->allMsgRecvd = false;

				InterlockedIncrement(&it->stringCounter[chatMsg]);
				it->_sessionLock.unlock();
				InterlockedIncrement(&g_sendLocalChatMsgTotal);
				
			}
			g_SectorLock[sectorX + i][sectorY + j].unlock();
		}
	}
	
	return true;
}


bool CheckMyActionComplete()
{
	bool retval = true;
	actionCompleteLock.lock();
	if (g_actionCompleteCount[tls_myThreadIndex] == THREAD_ACTION_INCOMPLETE)
	{
		retval = false;
	}
	actionCompleteLock.unlock();

	return retval;
}


bool RestoreNetworkHeader(ClientHeader header, DummySession* _session)
{
	unsigned int dequeResult;
	unsigned int enqueResult;
	int size = _session->_recvQ.GetSizeUsed();
	char* localBuf = (char*)malloc(size);

	
	_session->RecvBufferDeque(localBuf, size, &dequeResult);
	_session->RecvBufferEnque((char*)&header, sizeof(header), &enqueResult);
	_session->RecvBufferEnque(localBuf, size, &enqueResult);
	free(localBuf);

	return true;
}