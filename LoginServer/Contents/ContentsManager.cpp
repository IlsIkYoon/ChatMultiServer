#include "ContentsManager.h"
#include "CommonProtocol.h"
#include "Monitor/Monitor.h"

CContentsManager* g_ContentsManager;



extern CMonitor g_Monitor;


CContentsManager::CContentsManager(CWanServer* pNetworkManager)
{
	networkManager = pNetworkManager;
	userManager = new CUserManager(pNetworkManager->_sessionMaxCount);
	tickThread = std::thread([this]() {tickThreadFunc(); });
	DBConnector = new CDBConnector;
	//RedisConnector = new CRedisConnector;

	TextParser parser;
	parser.GetData("LoginConfig.ini");
	parser.SearchData("ChatServerPort", &chatServerPort);
	parser.SearchData("GameServerPort", &gameServerPort);
	parser.CloseData();

	wcscpy_s(chatServerIP, IP_CHATSERVER);
	wcscpy_s(gameServerIP, IP_GAMESERVER);

}



void CContentsManager::tickThreadFunc()
{
	

	while (1)
	{
		//어떤 틱 쓰레드 로직 진행
		g_Monitor.ConsolPrintAll();
		networkManager->EnqueSendRequest();
		Sleep(1000);

		if (_kbhit())
		{
			char c = _getch();
			ULONG64 key;
			std::string value;
			if (c == 'q' || c == 'Q')
			{
				std::cout << "Set key: ";
				std::cin >> key;
				std::cout << "\nValue : ";
				std::cin >> value;

				g_ContentsManager->SetToken(value, key);

			}
		}
	}


}

bool CContentsManager::InitUser(unsigned long long sessionID)
{
	return userManager->InitUser(sessionID);
}
bool CContentsManager::DeleteUser(unsigned long long sessionID)
{
	return userManager->DeleteUser(sessionID);
}


bool CContentsManager::HandleContentsMessage(CPacket* message, ULONG64 ID)
{
	WORD messageType;

	if (message->GetDataSize() < sizeof(messageType))
	{
		networkManager->DisconnectSession(ID);
		return false;
	}
	*message >> messageType;

	switch (messageType)
	{
	case en_PACKET_CS_LOGIN_REQ_LOGIN:
		HandleLoginREQMsg(message, ID);
		break;

	default:
	{
		networkManager->DisconnectSession(ID);
		return false;
	}
	break;
	}
}



bool CContentsManager::HandleLoginREQMsg(CPacket* message, ULONG64 ID)
{
	ULONG64 accountNo;
	char platformToken[65]; //토큰
	unsigned short userIndex;
	User* user;
	CPacket* sendMsg;

	userIndex = CWanServer::GetIndex(ID);
	user = userManager->GetUser(userIndex);

	if (user->alive == false)
	{
		__debugbreak();
	}
	if (user->sessionID != ID)
	{
		__debugbreak();
	}
		
	if (message->GetDataSize() < sizeof(accountNo) + sizeof(char) * 64)
	{
		networkManager->DisconnectSession(ID);
		return false;
	}

	*message >> accountNo;
	message->PopFrontData(64, platformToken);
	platformToken[64] = NULL;
	//로그인 과정
	WORD sendType;
	ULONG64 sendAccountNo;
	bool DBRetval;

	sendType = en_PACKET_CS_LOGIN_RES_LOGIN;
	sendAccountNo = accountNo;
	sendMsg = CPacket::Alloc();
	*sendMsg << sendType;
	*sendMsg << sendAccountNo;
	
	DBRetval = DBConnector->LoginDataRequest(sendMsg, sendAccountNo);

	if (DBRetval == false)
	{
		__debugbreak();
		//토큰 저장 없이 실패에 대한 메세지 반환해주는 함수
	}
	TLS_REDIS_CONNECTOR.SetToken(platformToken, sendAccountNo);


	MsgSetServerAddr(sendMsg);

	networkManager->SendPacket(ID, sendMsg);

	InterlockedIncrement(&g_Monitor.loginSuccessCount);

	sendMsg->DecrementUseCount();
	return true;
}


bool CContentsManager::MsgSetServerAddr(CPacket* message)
{

	message->PutData((char*)gameServerIP, sizeof(WCHAR) * 16);
	*message << gameServerPort;
	message->PutData((char*)chatServerIP, sizeof(WCHAR) * 16);
	*message << chatServerPort;

	return true;
}

DWORD CContentsManager::GetCurrentUser()
{
	return userManager->currentUserCount;
}
bool CContentsManager::SetToken(std::string Value, ULONG64 key)
{
	TLS_REDIS_CONNECTOR.SetToken(Value, key);
	return true;
}