#include "Message.h"
#include "Sector/Sector.h"
#include "Contents/ContentsPacket.h"
#include "Player./Player.h"
#include "Contents/ContentsFunc.h"
#include "Network/Network.h"
#include <format>
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

extern CWanServer* pLib;

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
	int playerIndex = pLib->GetIndex(id);
	int x;
	int y;
	BYTE direction;

	CPacket* msg;
	msg = payLoad;


	if (msg->GetDataSize() != sizeof(direction) + sizeof(x) + sizeof(y))
	{
		std::string logString;
		logString = "MoveStart Message - Msg size Error || messageSize : ";
		logString += std::to_string(msg->GetDataSize());

		pLib->EnqueLog(logString);

		pLib->DisconnectSession(id);
		return false;
	}


	*msg >> direction;
	*msg >> x;
	*msg >> y;
	

	if (direction > 7)//direction max값보다 큰 값이 왔음
	{
		std::string logString;
		logString = "MoveStart Message - Direction Error || value : ";
		logString += std::to_string(direction);

		pLib->EnqueLog(logString);

		pLib->DisconnectSession(id);
		return false;
	}

	if (x >= dfRANGE_MOVE_RIGHT || x < dfRANGE_MOVE_LEFT || y >= dfRANGE_MOVE_BOTTOM || y < dfRANGE_MOVE_TOP)
	{
		std::string logString;
		logString = "MoveStart Message - x or y value Error || x : ";
		logString += std::to_string(x);
		logString += " y : ";
		logString += std::to_string(y);

		pLib->EnqueLog(logString);

		pLib->DisconnectSession(id);
		return false;
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
	int playerIndex;
	int x;
	int y;
	BYTE direction;

	CPacket* msg;
	msg = payLoad;

	if (msg->GetDataSize() != sizeof(direction) + sizeof(x) + sizeof(y))
	{
		std::string logString;
		logString = "MoveStop Message - Msg Size Error || messageSize : ";
		logString += std::to_string(msg->GetDataSize());

		pLib->EnqueLog(logString);

		pLib->DisconnectSession(id);
		return false;
	}


	*msg >> direction;
	*msg >> x;
	*msg >> y;


	if (direction > 7)//direction max값보다 큰 값이 왔음
	{
		std::string logString;
		logString = "MoveStop Message - Direction Error || value : ";
		logString += std::to_string(direction);

		pLib->EnqueLog(logString);

		pLib->DisconnectSession(id);
		return false;
	}
	if (x >= dfRANGE_MOVE_RIGHT || x < dfRANGE_MOVE_LEFT || y >= dfRANGE_MOVE_BOTTOM || y < dfRANGE_MOVE_TOP)
	{
		std::string logString;
		logString = "MoveStop Message - x or y value Error || x : ";
		logString += std::to_string(x);
		logString += " y : ";
		logString += std::to_string(y);

		pLib->EnqueLog(logString);

		pLib->DisconnectSession(id);
		return false;
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

	if (msg->GetDataSize() < sizeof(chatMessageLen))
	{
		//로그 남기기 작업
		std::string error;
		error = "Local Chat Error || message Size error";
		
		pLib->EnqueLog(error);

		pLib->DisconnectSession(id);
		return false;
	}

	*msg >> chatMessageLen;
	

	if (msg->GetDataSize() != chatMessageLen)
	{
		//로그 남기기 작업
		std::string error;
		error = "Local Chat Error || message Size error";

		pLib->EnqueLog(error);

		pLib->DisconnectSession(id);
		return false;
	}


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
	int playerIndex = pLib->GetIndex(id);

	if (g_PlayerArr[playerIndex].GetID() != id || g_PlayerArr[playerIndex].isAlive() == false)
	{
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


bool HandleCreatePlayer(CPacket* JobMessage, ULONG64 id)
{

	int playerIndex;
	ULONG64 playerID;

	if (id != SERVER_ID)
	{
		std::string logString;
		logString = "Client sent create Message!!! || ID : ";
		logString += std::to_string(id);

		pLib->EnqueLog(logString);

		pLib->DisconnectSession(id);
		return false;
	}

	*JobMessage >> playerID;

	playerIndex = pLib->GetIndex(playerID);


	g_PlayerArr[playerIndex].Init(playerID);

	ContentsSendCreatePlayerPacket(playerID);

	return true;
}

bool HandleDeletePlayer(CPacket* JobMessage, ULONG64 id)
{
	int playerIndex;
	ULONG64 playerID;

	if (id != SERVER_ID)
	{
		std::string logString;
		logString = "Client sent delete Message!!! || ID : ";
		logString += std::to_string(id);

		pLib->EnqueLog(logString);
		pLib->DisconnectSession(id);
		return false;
	}

	*JobMessage >> playerID;

	playerIndex = pLib->GetIndex(playerID);

	if (g_PlayerArr[playerIndex].GetID() != playerID || g_PlayerArr[playerIndex].isAlive() == false)
	{
		if (g_PlayerArr[playerIndex]._status == static_cast<BYTE>(Player::STATUS::WAIT_CREATE))
		{
			g_PlayerArr[playerIndex]._status = static_cast<BYTE>(Player::STATUS::DELETED);
		}
		else
		{
			std::string logString;
			logString = "Duplicate delete || ID :  ";
			logString += std::to_string(playerID);

			pLib->EnqueLog(logString);
		}
		return false;
	}
	
	CheckSector(playerID);

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