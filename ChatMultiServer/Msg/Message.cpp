#include "ContentsResource.h"
#include "Message.h"
#include "Sector/Sector.h"
#include "ContentsPacket.h"
#include "Player/Player.h"
#include "ContentsThread/ContentsFunc.h"
#include "CommonProtocol.h"
#include "ContentsThread/ContentsThreadManager.h"
//-----------------------------------------
// 출력을 위한 메세지 카운팅
//-----------------------------------------
unsigned long long g_MoveStopCompleteCount;
unsigned long long g_MoveStartCount;
unsigned long long g_MoveStopCount;
unsigned long long g_LocalChatCount;
unsigned long long g_ChatEndCount;
unsigned long long g_DeleteMsgCount;
unsigned long long g_CreateMsgCount;
unsigned long long g_HeartBeatCount;

extern std::stack<int> g_playerIndexStack;
extern long long g_playerCount;
extern unsigned long long g_PlayerLogOut;

extern std::list<Player*> Sector[dfRANGE_MOVE_RIGHT / SECTOR_RATIO][dfRANGE_MOVE_BOTTOM / SECTOR_RATIO];
extern std::mutex SectorLock[dfRANGE_MOVE_RIGHT / SECTOR_RATIO][dfRANGE_MOVE_BOTTOM / SECTOR_RATIO];
extern int sectorXRange;
extern int sectorYRange;

extern unsigned long long g_PlayerLogInCount;
extern unsigned long long g_TotalPlayerCreate;
extern std::queue<ULONG64> g_WaitingPlayerAcceptQ;

extern CLanServer* ntServer;
extern CContentsThreadManager contentsManager;


void MsgSectorBroadCasting(void (*Func)(ULONG64 srcID, ULONG64 destID, CPacket* Packet), Player* _src, CPacket* Packet, bool SendMe)
{
	Player* pSrc = _src;

	typename std::list<Player*>::iterator pit; // = Sector[0][0].begin();
	int SectorX;
	int SectorY;

	SectorX = pSrc->GetX() / SECTOR_RATIO;
	SectorY = pSrc->GetY() / SECTOR_RATIO;

	//섹터 락을 전부 잡고 나서 시작

	for (int i = -1; i < 2; i++)
	{
		for (int j = -1; j < 2; j++)
		{
			if (SectorX + i < 0 || SectorX + i >= sectorXRange) continue;
			if (SectorY + j < 0 || SectorY + j >= sectorYRange) continue;


			SectorLock[SectorX + i][SectorY + j].lock();
		}
	}



	for (int i = -1; i < 2; i++)
	{
		for (int j = -1; j < 2; j++)
		{
			if (SectorX + i < 0 || SectorX + i >= sectorXRange) continue;
			if (SectorY + j < 0 || SectorY + j >= sectorYRange) continue;

			for (pit = Sector[SectorX + i][SectorY + j].begin(); pit != Sector[SectorX + i][SectorY + j].end(); pit++)
			{
				if ((*pit)->_sessionID == pSrc->_sessionID && SendMe == false)
				{
					continue;
				}
				if ((*pit)->isAlive() == false)
				{
					continue;
				}
				Func(pSrc->_sessionID, (*pit)->_sessionID, Packet);

			}
			/*
			for (auto* it : Sector[SectorX + i][SectorY + j])
			{
				if (it->GetID() == pSrc->GetID() && SendMe == false)
				{
					continue;
				}
				if (it->isAlive() == false) continue;

				Func(pSrc->GetID(), it->GetID(), Packet);

			}
			*/
		}
	}


	for (int i = -1; i < 2; i++)
	{
		for (int j = -1; j < 2; j++)
		{
			if (SectorX + i < 0 || SectorX + i >= sectorXRange) continue;
			if (SectorY + j < 0 || SectorY + j >= sectorYRange) continue;


			SectorLock[SectorX + i][SectorY + j].unlock();
		}
	}


}

bool HandleMoveStartMsg(CPacket* payLoad, ULONG64 id)
{
	InterlockedIncrement(&g_MoveStartCount);
	int playerIndex = ntServer->GetIndex(id);
	int x;
	int y;
	BYTE direction;
	Player* localPlayerList;
	CPacket* msg;

	localPlayerList = contentsManager.playerList->playerArr;
	msg = (CPacket*)payLoad;

	*msg >> direction;
	*msg >> x;
	*msg >> y;
	
	if (direction > 7)//direction max값
	{
		__debugbreak();
	}


	if (localPlayerList[playerIndex].GetID() != id || localPlayerList[playerIndex].isAlive() == false)
	{
		return false;
	}

	playerIndex = ntServer->GetIndex(id);
	
	CheckSector(id);
	localPlayerList[playerIndex].MoveStart(direction, x, y);
	CheckSector(id);

	return true;
}
bool HandleMoveStopMsg(CPacket* payLoad, ULONG64 id)
{
	InterlockedIncrement(&g_MoveStopCount);
	int playerIndex;
	int x;
	int y;
	BYTE direction;

	Player* localPlayerList;
	CPacket* msg;

	localPlayerList = contentsManager.playerList->playerArr;
	msg = (CPacket*)payLoad;

	*msg >> direction;
	*msg >> x;
	*msg >> y;


	if (direction > 7)//direction max값
	{
		__debugbreak();
	}

	playerIndex = ntServer->GetIndex(id);

	

	if (localPlayerList[playerIndex].GetID() != id || localPlayerList[playerIndex].isAlive() == false)
	{
		return false;
	}

	CheckSector(id);
	localPlayerList[playerIndex].MoveStop(direction, x, y);
	CheckSector(id);

	SendMoveStopCompleteMessage(id);

	return true;
}
bool HandleLocalChatMsg(CPacket* payLoad, ULONG64 id)
{
	InterlockedIncrement(&g_LocalChatCount);
	int playerIndex;
	BYTE chatMessageLen;
	stHeader sendChatHeader;
	CPacket* sendMsg;
	Player* localPlayerList;
	
	localPlayerList = contentsManager.playerList->playerArr;
	playerIndex = ntServer->GetIndex(id);

	if (localPlayerList[playerIndex].GetID() != id || localPlayerList[playerIndex].isAlive() == false)
	{
		return false;
	}



	CPacket* msg = (CPacket*)payLoad;
	*msg >> chatMessageLen;
	
	sendChatHeader.type = stPacket_Chat_Client_LocalChat;
	sendChatHeader.size = sizeof(chatMessageLen) + chatMessageLen;

	sendMsg = CPacket::Alloc();

	sendMsg->PutData((char*)&sendChatHeader, sizeof(sendChatHeader));
	*sendMsg << id;
	*sendMsg << chatMessageLen;
	sendMsg->PutData(msg->GetDataPtr(), chatMessageLen);
	msg->MoveFront(chatMessageLen);


	MsgSectorBroadCasting(ContentsSendPacket, &localPlayerList[playerIndex],sendMsg, false);
	sendMsg->DecrementUseCount();

	return true;
}
bool HandleHeartBeatMsg(ULONG64 id)
{
	InterlockedIncrement(&g_HeartBeatCount);
	int playerIndex;
	Player* localPlayerList;

	localPlayerList = contentsManager.playerList->playerArr;

	playerIndex = ntServer->GetIndex(id);

	if (localPlayerList[playerIndex].GetID() != id || localPlayerList[playerIndex].isAlive() == false)
	{
		return false;
	}

	localPlayerList[playerIndex]._timeOut = timeGetTime();

	return true;
}
bool HandleChatEndMsg(ULONG64 id)
{
	Player* localPlayerList;

	localPlayerList = contentsManager.playerList->playerArr;
	InterlockedIncrement(&g_ChatEndCount);

	int playerIndex = ntServer->GetIndex(id);

	if (localPlayerList[playerIndex].GetID() != id || localPlayerList[playerIndex].isAlive() == false)
	{
		return false;
	}

	stHeader packetHeader;
	CPacket* sendMsg = CPacket::Alloc();

	packetHeader.type = stPacket_Chat_Client_ChatComplete;
	packetHeader.size = 0;

	sendMsg->PutData((char*)&packetHeader, sizeof(packetHeader));

	ntServer->SendPacket(id, sendMsg);
	sendMsg->DecrementUseCount();

	return true;
}


bool HandleCreatePlayer(ULONG64 id)
{
	int playerIndex;
	Player* localPlayerList;

	InterlockedIncrement(&g_CreateMsgCount);
	localPlayerList = contentsManager.playerList->playerArr;
	playerIndex = ntServer->GetIndex(id);



	if (g_PlayerLogInCount >= PLAYERMAXCOUNT)
	{
		//todo//이 로직이 아직 정상이 아님
		__debugbreak();
		EnqueueWaitingPlayerQ(id);
		return false;
	}

	localPlayerList[playerIndex].Init(id);

	SendLoginResPacket(id);

	

	return true;
}

bool HandleDeletePlayer(ULONG64 id)
{
	int playerIndex;
	Player* localPlayerList;

	InterlockedIncrement(&g_DeleteMsgCount);
	localPlayerList = contentsManager.playerList->playerArr;
	playerIndex = ntServer->GetIndex(id);

	if (localPlayerList[playerIndex].GetID() != id || localPlayerList[playerIndex].isAlive() == false)
	{
		if (localPlayerList[playerIndex]._status == static_cast<BYTE>(Player::STATUS::SESSION))
		{
			localPlayerList[playerIndex]._status = static_cast<BYTE>(Player::STATUS::IDLE);
		}
		else
		{
			ntServer->EnqueLog("Duplicate delete!");
		}
		return false;
	}
	
	CheckSector(id);

	int SectorX = localPlayerList[playerIndex].GetX() / SECTOR_RATIO;
	int SectorY = localPlayerList[playerIndex].GetY() / SECTOR_RATIO;

	SectorLock[SectorX][SectorY].lock();
	Sector[SectorX][SectorY].remove(&localPlayerList[playerIndex]);
	SectorLock[SectorX][SectorY].unlock();

	localPlayerList[playerIndex].Clear();

	InterlockedIncrement(&g_PlayerLogOut);

	//여기서 대기열 체크하고 
	if (g_WaitingPlayerAcceptQ.size() > 0)
	{
		bool retval = DequeueWaitingPlayerQ();
		if(retval == true)
		{ 
			return true;
		}
	}

	InterlockedDecrement(&g_PlayerLogInCount);

	return true;
}



void ContentsSendPacket(ULONG64 srcID, ULONG64 destID, CPacket* packet)
{

	ntServer->SendPacket(destID, packet);

}

void SendMoveStopCompleteMessage(ULONG64 destID)
{
	InterlockedIncrement(&g_MoveStopCompleteCount);
	stHeader packetHeader;
	CPacket* sendMsg = CPacket::Alloc();
	packetHeader.type = stPacket_Chat_Client_MoveStopComplete;
	packetHeader.size = 0;
	sendMsg->PutData((char*)&packetHeader, sizeof(packetHeader));
	
	ntServer->SendPacket(destID, sendMsg);
	sendMsg->DecrementUseCount();

}



void SendLoginResPacket(ULONG64 sessionID)
{
	int playerIndex;
	Player* localPlayerList;
	CPacket* sendMsg;

	WORD Type;
	BYTE Status;	//0 : 실패 1: 성공
	INT64 AccountNo;

	localPlayerList = contentsManager.playerList->playerArr;
	playerIndex = ntServer->GetIndex(sessionID);

	Type = en_PACKET_CS_CHAT_RES_LOGIN;
	Status = 1;
	AccountNo = localPlayerList[playerIndex].accountNo;

	sendMsg = CPacket::Alloc();
	*sendMsg << Type;
	*sendMsg << Status;
	*sendMsg << AccountNo;

	ntServer->SendPacket(sessionID, sendMsg);

	sendMsg->DecrementUseCount();

	InterlockedIncrement(&g_PlayerLogInCount);
	InterlockedIncrement(&g_TotalPlayerCreate);

}


bool HandleLoginMessage(CPacket* message, ULONG64 sessionID)
{
	//char tokenRetval;
	DWORD playerIndex;
	Player* localPlayerList;

	//------------------------------------
	// 메세지 페이로드
	//------------------------------------
	INT64 AccountNo; //캐릭터 키
	WCHAR ID[20]; //null포함
	WCHAR Nickname[20]; //null포함
	char SessionKey[64]; //인증 토큰
	
	localPlayerList = contentsManager.playerList->playerArr;
	playerIndex = NetWorkManager::GetIndex(sessionID);
	//------------------------------------
	// 오류 체크
	//------------------------------------
	if (localPlayerList[playerIndex]._status != static_cast<BYTE>(Player::STATUS::SESSION))
	{
		ntServer->DisconnectSession(sessionID);
		return false;
	}
	//------------------------------------
	InterlockedIncrement(&g_CreateMsgCount);


	*message >> AccountNo;
	message->PopFrontData(sizeof(WCHAR) * 20, (char*)ID);
	message->PopFrontData(sizeof(WCHAR) * 20, (char*)Nickname);
	message->PopFrontData(sizeof(char) * 64, SessionKey);

	//tokenRetval = CheckLoginToken(AccountNo, SessionKey); -> 로그인 서버에서 보내준 토큰 자료구조를 검색해서 키값에 맞는 토큰인지 확인
	//이후 tokenRetval에 따른 로직분기 

	//LoadCharacterDataOnLogin(AccountNo, userId); 
	//토큰이 유효하다면 DB에서 캐릭터 데이터 긁어오기 요청 후 긁어온 뒤에 결과에 따라 Login_res 메세지 전송

	localPlayerList[playerIndex].Init(sessionID);
	localPlayerList[playerIndex].accountNo = AccountNo;
	wcsncpy_s(localPlayerList[playerIndex].ID, ID, 20);
	wcsncpy_s(localPlayerList[playerIndex].nickname, Nickname, 20);

	SendLoginResPacket(sessionID);

	return true;
}


bool HandleSectorMoveMessage(CPacket* message, ULONG64 sessionID)
{
	//플레이어의 섹터 정보를 변경 시켜주고 
	//섹터 이동에 따른 섹터 처리를 함
	Player* localPlayerList;
	DWORD playerIndex;
	WORD oldSectorX;
	WORD oldSectorY;
	//-------------------------------
	// 메세지 페이로드
	//-------------------------------
	INT64 AccountNo;
	WORD SectorX;
	WORD SectorY;

	localPlayerList = contentsManager.playerList->playerArr;
	playerIndex = NetWorkManager::GetIndex(sessionID);

	*message >> AccountNo;
	*message >> SectorX;
	*message >> SectorY;


	//-----------------------------------------
	// 오류 체크
	//-----------------------------------------
	if (localPlayerList[playerIndex]._status < static_cast<BYTE>(Player::STATUS::PENDING_SECTOR))
	{
		ntServer->DisconnectSession(sessionID);
		return false;
	}
	if (SectorX > SECTOR_MAX || SectorY > SECTOR_MAX)
	{
		ntServer->DisconnectSession(sessionID);
		return false;
	}
	if (AccountNo != localPlayerList[playerIndex].accountNo)
	{
		ntServer->DisconnectSession(sessionID);
		return false;
	}
	//-----------------------------------------

	if (localPlayerList[playerIndex]._status == static_cast<BYTE>(Player::STATUS::PENDING_SECTOR))
	{
		//섹터 첫 배치 작업
		localPlayerList[playerIndex].sectorX = SectorX;
		localPlayerList[playerIndex].sectorY = SectorY;

		SectorLock[SectorX][SectorY].lock();

		Sector[SectorX][SectorY].push_back(&localPlayerList[playerIndex]);
		SectorLock[SectorX][SectorY].unlock();

	}
	else 
	{
		//섹터 옮겨주기 작업

		oldSectorX = localPlayerList[playerIndex].sectorX;
		oldSectorY = localPlayerList[playerIndex].sectorY;

		localPlayerList[playerIndex].sectorX = SectorX;
		localPlayerList[playerIndex].sectorY = SectorY;

		SyncSector(sessionID, oldSectorX, oldSectorY);
	}
	SendSectorMoveResPacket(sessionID);


	return true;
}

bool HandleChatMessage(CPacket* message, ULONG64 sessionID)
{
	DWORD playerIndex;

	INT64 AccountNo;
	WORD MessageLen;
	WCHAR* Message; //null 미포함 // messageLen / 2길이
	Player* localPlayerList;

	localPlayerList = contentsManager.playerList->playerArr;

	*message >> AccountNo;
	*message >> MessageLen;

	Message = new WCHAR[MessageLen / 2];
	message->PopFrontData(MessageLen / 2, (char*)Message);

	playerIndex = NetWorkManager::GetIndex(sessionID);

	WORD Type;
	//INT64 AccountNo;
	WCHAR ID[20];
	WCHAR Nickname[20];
	//WORD MessageLen;
	//WCHAR* Message;

	Type = en_PACKET_CS_CHAT_RES_MESSAGE;
	wcsncpy_s(ID, localPlayerList[playerIndex].ID, 20);
	wcsncpy_s(Nickname, localPlayerList[playerIndex].nickname, 20);

	CPacket* sendMsg;
	sendMsg = CPacket::Alloc();

	*sendMsg << Type;
	*sendMsg << AccountNo;
	sendMsg->PutData((char*)ID, sizeof(WCHAR) * 20);
	sendMsg->PutData((char*)Nickname, sizeof(WCHAR) * 20);
	*sendMsg << MessageLen;
	sendMsg->PutData((char*)Message, MessageLen);


	MsgSectorBroadCasting(ContentsSendPacket, &localPlayerList[playerIndex], sendMsg, true);



	return true;
}


void SendSectorMoveResPacket(ULONG64 sessionID)
{
	int playerIndex;
	Player* localPlayerList;
	CPacket* sendMsg;

	WORD Type;
	INT64 AccountNo;
	WORD SectorX;
	WORD SectorY;

	localPlayerList = contentsManager.playerList->playerArr;
	playerIndex = NetWorkManager::GetIndex(sessionID);

	Type = en_PACKET_CS_CHAT_RES_SECTOR_MOVE;
	AccountNo = localPlayerList[playerIndex].accountNo;
	SectorX = localPlayerList[playerIndex].sectorX;
	SectorY = localPlayerList[playerIndex].sectorY;

	sendMsg = CPacket::Alloc();
	*sendMsg << Type;
	*sendMsg << AccountNo;
	*sendMsg << SectorX;
	*sendMsg << SectorY;


	ntServer->SendPacket(sessionID, sendMsg);

	sendMsg->DecrementUseCount();

	//todo//메세지 카운팅 
}