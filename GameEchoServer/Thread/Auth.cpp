#include "Auth.h"
#include "CommonProtocol.h"
#include "Network/Network.h"
#include "Player/Player.h"

CWanServer* g_WanServer;

bool AuthThreadWork::HandleMessage(CPacket* message, ULONG64 id)
{
	//메세지 분기 타는 함수
	WORD messageType;

	if (message->GetDataSize() < messageType)
	{
		std::string error;
		error += "AuthThread InComplete Message Error || ID : ";
		error += std::to_string(id);

		g_WanServer->EnqueLog(error);
		g_WanServer->DisconnectSession(id);
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

		g_WanServer->EnqueLog(error);
		g_WanServer->DisconnectSession(id);
		return false;
	}
		break;

	}

	return true;
}
bool AuthThreadWork::FrameLogic()
{
	//messageQ에서 메세지 꺼내서 처리하기



	return true;
}


bool AuthThreadWork::HandleLoginMessage(CPacket* message, ULONG64 id)
{
//		INT64	AccountNo
//		char	SessionKey[64]
//
//		int		Version			// 1 
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

		g_WanServer->EnqueLog(error);
		g_WanServer->DisconnectSession(id);
		return false;
	}
	
	*message >> AccountNo;
	message->PopFrontData(64, SessionKey);
	*message >> Version;

	playerIndex = g_WanServer->GetIndex(id);
	currentPlayer = &(*g_WanServer->playerManager)[playerIndex];
	currentPlayer->accountNo = AccountNo;
	
	//todo//게임 쓰레드에 캐릭터 생성 요청 보내기

	return true;
}