#include "Game.h"
#include "Network/Network.h"
#include "CommonProtocol.h"

bool CGameThreadWork::WorkInit()
{
	return true;
}
bool CGameThreadWork::HandleMessage(CPacket* message, ULONG64 id)
{
	WORD messageType;



	if (message->GetDataSize() < sizeof(messageType))
	{
		std::string error;
		error += "AuthThread InComplete Message Error || ID : ";
		error += std::to_string(id);

		networkManager->EnqueLog(error);
		networkManager->DisconnectSession(id);
		return false;
	}

	*message >> messageType;

	switch (messageType)
	{
	case en_PACKET_CS_GAME_REQ_ECHO:
	{
		HandleReqEchoMessage(message, id);
	}
	break;

	case en_PACKET_CS_GAME_REQ_HEARTBEAT:
		__debugbreak();
		break;

	default:
	{
		std::string error;
		error += "Game Thread Message Type Error || ID : ";
		error += std::to_string(id);

		networkManager->EnqueLog(error);
		networkManager->DisconnectSession(id);
		return false;
	}
	break;

	}

	return true;

}
bool CGameThreadWork::FrameLogic()
{
	InterlockedIncrement(&frame);
	return true;
}
void CGameThreadWork::OnCreateSession(ULONG64 ID)
{
	CPlayer* currentPlayer;
	unsigned short playerIndex;

	playerIndex = CWanServer::GetIndex(ID);
	currentPlayer = &(*playerManager)[playerIndex];

	playerList.push_back(currentPlayer);
	workPlayerCount++;

	currentPlayer->work = static_cast<BYTE>(enPlayerWork::en_Auth);

	//메시지 만들어서 보내기
	SendGameResLogin(ID);
	
	
}
void CGameThreadWork::OnDeleteSession(ULONG64 ID)
{
	CPlayer* currentPlayer;
	unsigned short playerIndex;

	playerIndex = CWanServer::GetIndex(ID);
	currentPlayer = &(*playerManager)[playerIndex];

	playerList.remove(currentPlayer);
	workPlayerCount--;
}

void CGameThreadWork::SendGameResLogin(ULONG64 ID)
{
	WORD Type;
	BYTE Status;
	INT64 AccountNo;
	CPlayer* currentPlayer;
	unsigned short playerIndex;

	CPacket* message;
	
	playerIndex = CWanServer::GetIndex(ID);
	currentPlayer = &(*playerManager)[playerIndex];

	message = CPacket::Alloc();

	Type = en_PACKET_CS_GAME_RES_LOGIN;
	Status = 1;
	AccountNo = currentPlayer->accountNo;

	*message << Type;
	*message << Status;
	*message << AccountNo;

	networkManager->SendPacket(ID, message);
	InterlockedIncrement(&((CWanServer*)networkManager)->sendCount);

	message->DecrementUseCount();

	return;
}


void CGameThreadWork::HandleReqEchoMessage(CPacket* message, ULONG64 ID)
{
	CPacket* sendMsg;
	WORD Type;
	
	if (message->GetDataSize() != sizeof(INT64) + sizeof(LONGLONG))
	{
		__debugbreak();
	}


	sendMsg = CPacket::Alloc();

	Type = en_PACKET_CS_GAME_RES_ECHO;

	*sendMsg << Type;
	sendMsg->PutData(message->GetDataPtr(), message->GetDataSize());
	message->MoveFront(message->GetDataSize());

	networkManager->SendPacket(ID, sendMsg);

	sendMsg->DecrementUseCount();
	InterlockedIncrement(&((CWanServer*)networkManager)->sendCount);
}

CGameThreadWork::CGameThreadWork()
{
	frame = 0;
	workPlayerCount = 0;
	playerManager = nullptr;

}