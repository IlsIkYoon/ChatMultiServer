#include "ContentsManager.h"
#include "Resource/CommonProtocol.h"

CContentsManager* g_ContentsManager;



bool CContentsManager::HandleContentsMsg(CPacket* message, ULONG64 ID)
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
	case en_PACKET_SS_MONITOR_LOGIN:
		HandleServerLoginMsg(message, ID);
		break;

	case en_PACKET_SS_MONITOR_DATA_UPDATE:
		HandleDataUpdateMsg(message, ID);
		break;

	case en_PACKET_CS_MONITOR_TOOL_REQ_LOGIN:
		HandeClientLoginMsg(message, ID);
		break;

	default:
		networkManager->DisconnectSession(ID);
		break;

	}




}


CContentsManager::CContentsManager(CLanServer* pNetworkManager)
{
	networkManager = pNetworkManager;
	agentManager = new CAgentManager(networkManager->_sessionMaxCount);
	strcpy(clientLoginToken, "ajfw@!cv980dSZ[fje#@fdj123948djf");
}


bool CContentsManager::HandleServerLoginMsg(CPacket* message, ULONG64 ID)
{
	unsigned short agentIndex;
	int serverNo;

	*message >> serverNo;

	agentIndex = CLanServer::GetIndex(ID);
	(*agentManager)[agentIndex].Status = static_cast<BYTE>(enAgentStatus::en_Alive);
	(*agentManager)[agentIndex].Type = static_cast<BYTE>(enAgentType::en_Server);
	(*agentManager)[agentIndex].ServerNo = serverNo;

	return true;
}
bool CContentsManager::HandleDataUpdateMsg(CPacket* message, ULONG64 ID)
{

}
bool CContentsManager::HandeClientLoginMsg(CPacket* message, ULONG64 ID)
{
	//키 값 유효한지 확인 후에 로그인
	unsigned short agentIndex;
	char loginToken[33];

	message->PopFrontData(32, loginToken);
	loginToken[32] = NULL;

	if (strcmp(loginToken, clientLoginToken) != 0)
	{
		networkManager->DisconnectSession(ID);
		return false;
	}

	agentIndex = CLanServer::GetIndex(ID);
	(*agentManager)[agentIndex].Status = static_cast<BYTE>(enAgentStatus::en_Alive);
	(*agentManager)[agentIndex].Type = static_cast<BYTE>(enAgentType::en_Server);


	//todo//클라이언트 접속 응답 보내줘야 함

	return true;
}