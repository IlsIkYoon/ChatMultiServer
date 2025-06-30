#include "ContentsManager.h"
#include "Resource/CommonProtocol.h"

CContentsManager* g_ContentsManager;
extern CPdhManager g_PDH;


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


CContentsManager::CContentsManager(CWanServer* pNetworkManager)
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

	agentIndex = CWanServer::GetIndex(ID);
	(*agentManager)[agentIndex].Status = static_cast<BYTE>(enAgentStatus::en_Alive);
	(*agentManager)[agentIndex].Type = static_cast<BYTE>(enAgentType::en_Server);
	(*agentManager)[agentIndex].ServerNo = serverNo;
	(*agentManager)[agentIndex].sessionID = ID;

	agentManager->RegistServer(ID);

	printf("Server Login! \n");

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

	AgentIndex = CWanServer::GetIndex(ID);

	*message >> DataType;
	*message >> DataValue;
	*message >> TimeStamp;

	//---------------------------------------------------
	// 메세지 예외처리
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

	SendMonitorServerData();
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
		printf("Token Error ! \n");
		networkManager->DisconnectSession(ID);
		return false;
	}

	agentIndex = CWanServer::GetIndex(ID);
	(*agentManager)[agentIndex].Type = static_cast<BYTE>(enAgentType::en_Server);
	(*agentManager)[agentIndex].Status = static_cast<BYTE>(enAgentStatus::en_Alive);
	(*agentManager)[agentIndex].sessionID = ID;
	agentManager->RegistClient(ID);

	SendClientLoginResMsg(ID);
	InterlockedExchange(&(*agentManager)[agentIndex].bResCreate, 1);


	printf("ClientLogin!\n");

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
	networkManager->EnqueSendRequest();

	sendMsg->DecrementUseCount();

	return true;
}

bool CContentsManager::DeleteAgent(ULONG64 ID)
{
	unsigned short agentIndex = CWanServer::GetIndex(ID);
	(*agentManager)[agentIndex].Clear();
	return true;
}

bool CContentsManager::SendMonitorServerData()
{
	DWORD nowTime = timeGetTime();

	if (nowTime - prevTime < 1000)
	{
		return false;
	}

	CPacket* MSG_processorTotal;
	CPacket* MSG_nonPagedTotal;
	CPacket* MSG_availableMem;
	CPacket* MSG_networkRecv;
	CPacket* MSG_networkSend;

	double processorCpuTotal;
	double nonPagedTotal;
	double availableMem;
	double networkRecv;
	double networkSend;

	g_PDH.GetCpuData(&processorCpuTotal, NULL, NULL);
	g_PDH.GetMemoryData(nullptr, nullptr, &nonPagedTotal, &availableMem);
	g_PDH.GetEthernetData(&networkRecv, &networkSend);
	
	MSG_processorTotal = CPacket::Alloc();
	MSG_nonPagedTotal = CPacket::Alloc();
	MSG_availableMem = CPacket::Alloc();
	MSG_networkRecv = CPacket::Alloc();
	MSG_networkSend = CPacket::Alloc();
	
	MakeCSUpdateMsg(MSG_processorTotal, dfMONITOR_DATA_TYPE_MONITOR_CPU_TOTAL, (int)processorCpuTotal);
	MakeCSUpdateMsg(MSG_nonPagedTotal, dfMONITOR_DATA_TYPE_MONITOR_NONPAGED_MEMORY, (int)(nonPagedTotal / 1024 / 1024));
	MakeCSUpdateMsg(MSG_availableMem, dfMONITOR_DATA_TYPE_MONITOR_AVAILABLE_MEMORY, (int)availableMem);
	MakeCSUpdateMsg(MSG_networkRecv, dfMONITOR_DATA_TYPE_MONITOR_NETWORK_RECV, (int)networkRecv);
	MakeCSUpdateMsg(MSG_networkSend, dfMONITOR_DATA_TYPE_MONITOR_NETWORK_SEND, (int)networkSend);

	agentManager->SendAllClient(MSG_processorTotal);
	agentManager->SendAllClient(MSG_nonPagedTotal);
	agentManager->SendAllClient(MSG_availableMem);
	agentManager->SendAllClient(MSG_networkRecv);
	agentManager->SendAllClient(MSG_networkSend);

	MSG_processorTotal->DecrementUseCount();
	MSG_nonPagedTotal->DecrementUseCount();
	MSG_availableMem->DecrementUseCount();
	MSG_networkRecv->DecrementUseCount();
	MSG_networkSend->DecrementUseCount();



	prevTime = nowTime;

	return true;
}

bool CContentsManager::MakeCSUpdateMsg(CPacket* msg, BYTE dataType, int Value)
{
	WORD type;
	int nowTime;
	BYTE serverNo;


	type = en_PACKET_CS_MONITOR_TOOL_DATA_UPDATE;
	nowTime = (int)time(nullptr);
	serverNo = MONITOR_SERVERNO;

	*msg << type;
	*msg << serverNo;
	*msg << dataType;
	*msg << Value;
	*msg << nowTime;

	return true;
}