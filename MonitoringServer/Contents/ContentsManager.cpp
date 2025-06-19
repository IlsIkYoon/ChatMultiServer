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

	return true;
}


CContentsManager::CContentsManager(CLanServer* pNetworkManager)
{
	networkManager = pNetworkManager;
	agentManager = new CAgentManager(networkManager->_sessionMaxCount);
	strcpy_s(clientLoginToken, "ajfw@!cv980dSZ[fje#@fdj123948djf");

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
	(*agentManager)[agentIndex].sessionID = ID;

	agentManager->RegistServer(ID);
	return true;
}
bool CContentsManager::HandleDataUpdateMsg(CPacket* message, ULONG64 ID)
{
	BYTE DataType;
	int DataValue;
	int TimeStamp;

	BYTE serverNo;
	unsigned short AgentIndex;

	if (message->GetDataSize() != sizeof(DataType) + sizeof(DataValue) + sizeof(TimeStamp))
	{
		networkManager->DisconnectSession(ID);
		return false;
	}


	AgentIndex = CLanServer::GetIndex(ID);

	*message >> DataType;
	*message >> DataValue;
	*message >> TimeStamp;

	//---------------------------------------------------
	// 메세지 예외처리 //todo//
	//---------------------------------------------------
	if (DataType > 44)
	{
		networkManager->DisconnectSession(ID);
		return false;
	}
	

	//여기까지가 Message Dequeue;

	WORD msgType;
	serverNo = (*agentManager)[AgentIndex].ServerNo;

	CPacket* sendMsg = CPacket::Alloc();

	msgType = en_PACKET_CS_MONITOR_TOOL_DATA_UPDATE;

	*sendMsg << msgType;
	*sendMsg << serverNo;
	*sendMsg << DataType;
	*sendMsg << DataValue;
	*sendMsg << TimeStamp;

	agentManager->SendAllClient(sendMsg);

	message->DecrementUseCount();
	sendMsg->DecrementUseCount();

	return true;
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
	(*agentManager)[agentIndex].sessionID = ID;

	agentManager->RegistClient(ID);
	SendClientLoginResMsg(ID);

	message->DecrementUseCount();

	return true;
}

bool CContentsManager::SendClientLoginResMsg(ULONG64 ID)
{
	CPacket* sendMsg;

	WORD MsgType;
	BYTE LoginResult;

	MsgType = en_PACKET_CS_MONITOR_TOOL_RES_LOGIN;
	LoginResult = dfMONITOR_TOOL_LOGIN_OK;

	sendMsg = CPacket::Alloc();

	*sendMsg << MsgType;
	*sendMsg << LoginResult;

	networkManager->SendPacket(ID, sendMsg);

	sendMsg->DecrementUseCount();

	return true;
}



bool CContentsManager::DeleteAgent(ULONG64 ID)
{
	unsigned short agentIndex = CLanServer::GetIndex(ID);
	(*agentManager)[agentIndex].Clear();
	return true;
}