#include "ContentsManager.h"
#include "CommonProtocol.h"

CContentsManager* g_ContentsManager;




CContentsManager::CContentsManager(CWanServer* pNetworkManager)
{
	networkManager = pNetworkManager;
	userManager = new CUserManager(pNetworkManager->_sessionMaxCount);
	tickThread = std::thread([this]() {tickThreadFunc(); });
}



void CContentsManager::tickThreadFunc()
{

	while (1)
	{
		//어떤 틱 쓰레드 로직 진행


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



bool HandleLoginREQMsg(CPacket* message, ULONG64 ID)
{
	ULONG64 accountNo;
	char sessionKey[64]; //토큰




	return true;
}