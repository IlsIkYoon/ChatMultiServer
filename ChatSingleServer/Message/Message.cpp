#include "Message.h"
#include "Sector/Sector.h"
#include "Contents/ContentsPacket.h"
#include "Player./Player.h"
#include "Contents/ContentsFunc.h"
#include <format>
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
extern int sectorXRange;
extern int sectorYRange;

extern unsigned long long g_PlayerLogInCount;
extern unsigned long long g_TotalPlayerCreate;
extern std::queue<ULONG64> g_WaitingPlayerAcceptQ;

extern CLanServer* pLib;

void MsgSectorBroadCasting(void (*Func)(ULONG64 srcID, ULONG64 destID, CPacket* Packet), char* _src, CPacket* Packet, bool SendMe)
{
	Player* pSrc = (Player*)_src;
	
	typename std::list<Player*>::iterator pit = Sector[0][0].begin();

	int SectorX;
	int SectorY;

	SectorX = pSrc->GetX() / SECTOR_RATIO;
	SectorY = pSrc->GetY() / SECTOR_RATIO;


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

}

bool HandleMoveStartMsg(CPacket* payLoad, ULONG64 id)
{
	InterlockedIncrement(&g_MoveStartCount);
	int playerIndex = pLib->GetIndex(id);
	int x;
	int y;
	BYTE direction;

	CPacket* msg;
	msg = payLoad;


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

	playerIndex = pLib->GetIndex(id);
	
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
	msg = payLoad;

	*msg >> direction;
	*msg >> x;
	*msg >> y;


	if (direction > 7)//direction max값
	{
		__debugbreak();
	}

	playerIndex = pLib->GetIndex(id);

	

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
	
	playerIndex = pLib->GetIndex(id);

	if (g_PlayerArr[playerIndex].GetID() != id || g_PlayerArr[playerIndex].isAlive() == false)
	{
		return false;
	}

	CPacket* msg = payLoad;
	*msg >> chatMessageLen;
	
	sendChatHeader.type = stPacket_Chat_Client_LocalChat;

	sendMsg = CPacket::Alloc();

	sendMsg->PutData((char*)&sendChatHeader, sizeof(sendChatHeader));
	*sendMsg << id;
	*sendMsg << chatMessageLen;
	sendMsg->PutData(msg->GetDataPtr(), chatMessageLen);
	msg->MoveFront(chatMessageLen);


	MsgSectorBroadCasting(ContentsSendPacket, (char*) & g_PlayerArr[playerIndex],sendMsg, false);
	sendMsg->DecrementUseCount();

	return true;
}
bool HandleHeartBeatMsg(ULONG64 id)
{
	InterlockedIncrement(&g_HeartBeatCount);
	int playerIndex;

	playerIndex = pLib->GetIndex(id);

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



	int playerIndex = pLib->GetIndex(id);

	if (g_PlayerArr[playerIndex].GetID() != id || g_PlayerArr[playerIndex].isAlive() == false)
	{
		__debugbreak();
		return false;
	}

	stHeader packetHeader;
	CPacket* sendMsg = CPacket::Alloc();

	packetHeader.type = stPacket_Chat_Client_ChatComplete;

	sendMsg->PutData((char*)&packetHeader, sizeof(packetHeader));

	pLib->SendPacket(id, sendMsg);
	sendMsg->DecrementUseCount();



	return true;
}


bool HandleCreatePlayer(ULONG64 id)
{
	InterlockedIncrement(&g_CreateMsgCount);
	int playerIndex;
	playerIndex = pLib->GetIndex(id);


	/*
	if (g_PlayerLogInCount >= PLAYERMAXCOUNT)
	{
		EnqueueWaitingPlayerQ(id);
		return false;
	}

	*/
	g_PlayerArr[playerIndex].Init(id);

	ContentsSendCreatePlayerPacket(id);

	return true;
}

bool HandleDeletePlayer(ULONG64 id)
{
	InterlockedIncrement(&g_DeleteMsgCount);
	int playerIndex;
	playerIndex = pLib->GetIndex(id);

	if (g_PlayerArr[playerIndex].GetID() != id || g_PlayerArr[playerIndex].isAlive() == false)
	{
		if (g_PlayerArr[playerIndex]._status == static_cast<BYTE>(Player::STATUS::WAIT_CREATE))
		{
			g_PlayerArr[playerIndex]._status = static_cast<BYTE>(Player::STATUS::DELETED);
		}
		else
		{
			pLib->EnqueLog("Duplicate delete!");
		}
		return false;
	}
	
	CheckSector(id);

	int SectorX = g_PlayerArr[playerIndex].GetX() / SECTOR_RATIO;
	int SectorY = g_PlayerArr[playerIndex].GetY() / SECTOR_RATIO;

	Sector[SectorX][SectorY].remove(&g_PlayerArr[playerIndex]);

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

	pLib->SendPacket(destID, packet);

}

void SendMoveStopCompleteMessage(ULONG64 destID)
{
	InterlockedIncrement(&g_MoveStopCompleteCount);
	stHeader packetHeader;
	CPacket* sendMsg = CPacket::Alloc();
	packetHeader.type = stPacket_Chat_Client_MoveStopComplete;
	sendMsg->PutData((char*)&packetHeader, sizeof(packetHeader));
	
	pLib->SendPacket(destID, sendMsg);
	sendMsg->DecrementUseCount();

}



void ContentsSendCreatePlayerPacket(ULONG64 id)
{
	int playerIndex = pLib->GetIndex(id);

	stHeader PacketHeader;
	CPacket* sendMsg;
	unsigned long long characterKey;
	int x;
	int y;

	characterKey = id;
	x = g_PlayerArr[playerIndex].GetX();
	y = g_PlayerArr[playerIndex].GetY();

	PacketHeader.type = stPacket_Chat_Client_CreateCharacter;

	sendMsg = CPacket::Alloc();

	sendMsg->PutData((char*)&PacketHeader, sizeof(PacketHeader));
	*sendMsg << characterKey;
	*sendMsg << x;
	*sendMsg << y;

	pLib->SendPacket(id, sendMsg);
	sendMsg->DecrementUseCount();

	InterlockedIncrement(&g_PlayerLogInCount);
	InterlockedIncrement(&g_TotalPlayerCreate);

}