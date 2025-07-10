#include "Auth.h"
#include "CommonProtocol.h"
#include "Network/Network.h"
#include "Player/Player.h"
#include "Game.h"


extern CGameThreadWork* g_GameThreadWork;

bool CAuthThreadWork::HandleMessage(CPacket* message, ULONG64 id)
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
	case en_PACKET_CS_GAME_REQ_LOGIN:
	{
		HandleLoginMessage(message, id);
	}
		break;

	case en_PACKET_CS_GAME_REQ_HEARTBEAT:
		__debugbreak();
		break;

	default:
	{
		std::string error;
		error += "Auth Thread Message Type Error || ID : ";
		error += std::to_string(id);

		networkManager->EnqueLog(error);
		networkManager->DisconnectSession(id);
		return false;
	}
		break;

	}

	return true;
}
bool CAuthThreadWork::FrameLogic()
{
	InterlockedIncrement(&frame);
	return true;
}


bool CAuthThreadWork::HandleLoginMessage(CPacket* message, ULONG64 id)
{
	CPlayer* currentPlayer;
	unsigned short playerIndex;


	INT64 AccountNo;
	char SessionKey[64];
	int Version;

	if (message->GetDataSize() != sizeof(AccountNo) + 64 + sizeof(Version))
	{
		std::string error;
		error = "AuthThread Message Len Error || ";
		error += std::to_string(id);

		networkManager->EnqueLog(error);
		networkManager->DisconnectSession(id);
		return false;
	}
	
	*message >> AccountNo;
	message->PopFrontData(64, SessionKey);
	*message >> Version;

	playerIndex = CWanServer::GetIndex(id);
	currentPlayer = &(*playerManager)[playerIndex];
	currentPlayer->accountNo = AccountNo;
	
	g_GameThreadWork->CreateSession(id);
	//DeleteSession(id);
	HandleDeleteSessionMsg(id);

	return true;
}


bool CAuthThreadWork::WorkInit()
{
	return true;
}

void CAuthThreadWork::OnCreateSession(ULONG64 ID)
{
	CPlayer* currentPlayer;
	unsigned short playerIndex;

	playerIndex = CWanServer::GetIndex(ID);
	currentPlayer = &(*playerManager)[playerIndex];

	playerList.push_back(currentPlayer);
	currentPlayer->work = static_cast<BYTE>(enPlayerWork::en_Auth);
	workPlayerCount++;
}
void CAuthThreadWork::OnDeleteSession(ULONG64 ID)
{
	CPlayer* currentPlayer;
	unsigned short playerIndex;

	playerIndex = CWanServer::GetIndex(ID);
	currentPlayer = &(*playerManager)[playerIndex];

	playerList.remove(currentPlayer);
	workPlayerCount--;
}

CAuthThreadWork::CAuthThreadWork()
{
	frame = 0;
	workPlayerCount = 0;
}