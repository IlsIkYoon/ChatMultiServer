#include "ContentsResource.h"
#include "Message.h"
#include "Sector/Sector.h"
#include "ContentsPacket.h"
#include "Player/Player.h"
#include "ContentsThread/ContentsFunc.h"
#include "CommonProtocol.h"
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
extern Player* g_PlayerArr;
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
				if ((*pit)->GetID() == pSrc->GetID() && SendMe == false)
				{
					continue;
				}
				if ((*pit)->isAlive() == false)
				{
					continue;
				}
				Func(pSrc->GetID(), (*pit)->GetID(), Packet);

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

	CPacket* msg;
	msg = (CPacket*)payLoad;

	*msg >> direction;
	*msg >> x;
	*msg >> y;
	
	if (direction > 7)//direction max값
	{
		__debugbreak();
	}


	if (g_PlayerArr[playerIndex].GetID() != id || g_PlayerArr[playerIndex].isAlive() == false)
	{
		return false;
	}

	playerIndex = ntServer->GetIndex(id);
	
	CheckSector(id);
	g_PlayerArr[playerIndex].MoveStart(direction, x, y);
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

	CPacket* msg;
	msg = (CPacket*)payLoad;

	*msg >> direction;
	*msg >> x;
	*msg >> y;


	if (direction > 7)//direction max값
	{
		__debugbreak();
	}

	playerIndex = ntServer->GetIndex(id);

	

	if (g_PlayerArr[playerIndex].GetID() != id || g_PlayerArr[playerIndex].isAlive() == false)
	{
		return false;
	}

	CheckSector(id);
	g_PlayerArr[playerIndex].MoveStop(direction, x, y);
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
	
	playerIndex = ntServer->GetIndex(id);

	if (g_PlayerArr[playerIndex].GetID() != id || g_PlayerArr[playerIndex].isAlive() == false)
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


	MsgSectorBroadCasting(ContentsSendPacket, & g_PlayerArr[playerIndex],sendMsg, false);
	sendMsg->DecrementUseCount();

	return true;
}
bool HandleHeartBeatMsg(ULONG64 id)
{
	InterlockedIncrement(&g_HeartBeatCount);
	int playerIndex;

	playerIndex = ntServer->GetIndex(id);

	if (g_PlayerArr[playerIndex].GetID() != id || g_PlayerArr[playerIndex].isAlive() == false)
	{
		return false;
	}

	g_PlayerArr[playerIndex]._timeOut = timeGetTime();

	return true;
}
bool HandleChatEndMsg(ULONG64 id)
{
	InterlockedIncrement(&g_ChatEndCount);

	int playerIndex = ntServer->GetIndex(id);

	if (g_PlayerArr[playerIndex].GetID() != id || g_PlayerArr[playerIndex].isAlive() == false)
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
	InterlockedIncrement(&g_CreateMsgCount);
	int playerIndex;
	playerIndex = ntServer->GetIndex(id);



	if (g_PlayerLogInCount >= PLAYERMAXCOUNT)
	{
		//todo//이 로직이 아직 정상이 아님
		__debugbreak();
		EnqueueWaitingPlayerQ(id);
		return false;
	}

	g_PlayerArr[playerIndex].Init(id);

	SendLoginResPacket(id);

	

	return true;
}

bool HandleDeletePlayer(ULONG64 id)
{
	InterlockedIncrement(&g_DeleteMsgCount);
	int playerIndex;
	playerIndex = ntServer->GetIndex(id);

	if (g_PlayerArr[playerIndex].GetID() != id || g_PlayerArr[playerIndex].isAlive() == false)
	{
		if (g_PlayerArr[playerIndex]._status == static_cast<BYTE>(Player::STATUS::SESSION))
		{
			g_PlayerArr[playerIndex]._status = static_cast<BYTE>(Player::STATUS::IDLE);
		}
		else
		{
			ntServer->EnqueLog("Duplicate delete!");
		}
		return false;
	}
	
	CheckSector(id);

	int SectorX = g_PlayerArr[playerIndex].GetX() / SECTOR_RATIO;
	int SectorY = g_PlayerArr[playerIndex].GetY() / SECTOR_RATIO;

	SectorLock[SectorX][SectorY].lock();
	Sector[SectorX][SectorY].remove(&g_PlayerArr[playerIndex]);
	SectorLock[SectorX][SectorY].unlock();

	g_PlayerArr[playerIndex].Clear();

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
	int playerIndex = ntServer->GetIndex(sessionID);

	CPacket* sendMsg;

	WORD Type;
	BYTE Status;	//0 : 실패 1: 성공
	INT64 AccountNo;
	Type = en_PACKET_CS_CHAT_RES_LOGIN;
	Status = 1;
	AccountNo = g_PlayerArr[playerIndex].accountNo;

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

	//------------------------------------
	// 메세지 페이로드
	//------------------------------------
	INT64 AccountNo; //캐릭터 키
	WCHAR ID[20]; //null포함
	WCHAR Nickname[20]; //null포함
	char SessionKey[64]; //인증 토큰
	
	InterlockedIncrement(&g_CreateMsgCount);

	*message >> AccountNo;
	message->PopFrontData(sizeof(WCHAR) * 20, (char*)ID);
	message->PopFrontData(sizeof(WCHAR) * 20, (char*)Nickname);
	message->PopFrontData(sizeof(char) * 64, SessionKey);

	//tokenRetval = CheckLoginToken(AccountNo, SessionKey); -> 로그인 서버에서 보내준 토큰 자료구조를 검색해서 키값에 맞는 토큰인지 확인
	//이후 tokenRetval에 따른 로직분기 

	//LoadCharacterDataOnLogin(AccountNo, userId); 
	//토큰이 유효하다면 DB에서 캐릭터 데이터 긁어오기 요청 후 긁어온 뒤에 결과에 따라 Login_res 메세지 전송

	playerIndex = NetWorkManager::GetIndex(sessionID);
	g_PlayerArr[playerIndex].Init(sessionID);
	g_PlayerArr->accountNo = AccountNo;
	wcsncpy(g_PlayerArr[playerIndex].ID, ID, 20);
	wcsncpy(g_PlayerArr[playerIndex].nickname, Nickname, 20);

	SendLoginResPacket(sessionID);

	return true;
}


bool HandleSectorMoveMessage(CPacket* message, ULONG64 sessionID)
{
	//플레이어의 섹터 정보를 변경 시켜주고 
	//섹터 이동에 따른 섹터 처리를 함

	DWORD playerIndex;
	WORD oldSectorX;
	WORD oldSectorY;
	//-------------------------------
	// 메세지 페이로드
	//-------------------------------
	INT64 AccountNo;
	WORD SectorX;
	WORD SectorY;

	*message >> AccountNo;
	*message >> SectorX;
	*message >> SectorY;

	playerIndex = NetWorkManager::GetIndex(sessionID);

	oldSectorX = g_PlayerArr[playerIndex].sectorX;
	oldSectorY = g_PlayerArr[playerIndex].sectorY;

	g_PlayerArr[playerIndex].sectorX = SectorX;
	g_PlayerArr[playerIndex].sectorY = SectorY;

	SyncSector(sessionID, oldSectorX, oldSectorY);

	SendSectorMoveResPacket(sessionID);


	return true;
}

bool HandleChatMessage(CPacket* message, ULONG64 sessionID)
{
	DWORD playerIndex;

	INT64 AccountNo;
	WORD MessageLen;
	WCHAR* Message; //null 미포함 // messageLen / 2길이

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
	wcsncpy(ID, g_PlayerArr[playerIndex].ID, 20);
	wcsncpy(Nickname, g_PlayerArr[playerIndex].nickname, 20);

	CPacket* sendMsg;
	sendMsg = CPacket::Alloc();

	*sendMsg << Type;
	*sendMsg << AccountNo;
	sendMsg->PutData((char*)ID, sizeof(WCHAR) * 20);
	sendMsg->PutData((char*)Nickname, sizeof(WCHAR) * 20);
	*sendMsg << MessageLen;
	sendMsg->PutData((char*)Message, MessageLen);


	MsgSectorBroadCasting(ContentsSendPacket, &g_PlayerArr[playerIndex], sendMsg, true);



	return true;
}


void SendSectorMoveResPacket(ULONG64 sessionID)
{
	int playerIndex = NetWorkManager::GetIndex(sessionID);

	CPacket* sendMsg;

	WORD Type;
	INT64 AccountNo;
	WORD SectorX;
	WORD SectorY;

	Type = en_PACKET_CS_CHAT_RES_SECTOR_MOVE;
	AccountNo = g_PlayerArr[playerIndex].accountNo;
	SectorX = g_PlayerArr[playerIndex].sectorX;
	SectorY = g_PlayerArr[playerIndex].sectorY;

	sendMsg = CPacket::Alloc();
	*sendMsg << Type;
	*sendMsg << AccountNo;
	*sendMsg << SectorX;
	*sendMsg << SectorY;


	ntServer->SendPacket(sessionID, sendMsg);

	sendMsg->DecrementUseCount();

	//todo//메세지 카운팅 
}