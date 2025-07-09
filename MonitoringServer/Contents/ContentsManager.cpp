#include "ContentsManager.h"
#include "Resource/CommonProtocol.h"
#include "DBConnector/DBConnector.h"

CContentsManager* g_ContentsManager;
extern CPdhManager g_PDH;
#ifndef __MAC__
extern CDBManager* g_DBManager; 
#endif

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
	prevTime = 0;
	dbSavetime = 0;
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
	CMonitorAgent* currentAgent = &(*agentManager)[AgentIndex];
	serverNo = currentAgent->ServerNo;

	CPacket* sendMsg = CPacket::Alloc();

	msgType = en_PACKET_CS_MONITOR_TOOL_DATA_UPDATE;

	*sendMsg << msgType;
	*sendMsg << serverNo;
	*sendMsg << DataType;
	*sendMsg << DataValue;
	*sendMsg << TimeStamp;

	//평균값 구해서 등록해놓기
	auto it = currentAgent->datatypeAverage_Map.find(DataType);

	if (it != currentAgent->datatypeAverage_Map.end()) //이미 등록이 되어 있었다면
	{
		auto& dataStat = it->second;

		int currentValue = DataValue;
		if (dataStat.max < DataValue)
		{
			dataStat.max = DataValue;
		}
		else if (dataStat.min > DataValue)
		{
			dataStat.min = DataValue;
		}


		int prevCount = dataStat.count;
		int prevAverage = dataStat.average;
		int prevTotal = prevCount * prevAverage;

		int nextTotal = prevTotal + currentValue;
		int nextCount = prevCount + 1;
		int nextAverage = nextTotal / nextCount;
		
		dataStat.average = nextAverage;
		dataStat.count = nextCount;
	}
	else
	{
		auto& dataStat = currentAgent->datatypeAverage_Map[DataType];

		dataStat.count = 1;
		dataStat.max = DataValue;
		dataStat.min = DataValue;
		dataStat.average = DataValue;

		switch (DataType)
		{
		case dfMONITOR_DATA_TYPE_CHAT_SERVER_RUN : 
			dataStat.name = "Chat_Server_Run";
			break;

		case dfMONITOR_DATA_TYPE_CHAT_SERVER_CPU:
			dataStat.name = "Chat_Server_CPU";
			break;

		case dfMONITOR_DATA_TYPE_CHAT_SERVER_MEM:
			dataStat.name = "Chat_Server_Mem";
			break;

		case dfMONITOR_DATA_TYPE_CHAT_SESSION:
			dataStat.name = "Chat_Server_Session";
			break;
		case dfMONITOR_DATA_TYPE_CHAT_PLAYER:
			dataStat.name = "Chat_Server_Player";
			break;
		case dfMONITOR_DATA_TYPE_CHAT_UPDATE_TPS:
			dataStat.name = "Chat_Server_UPDATE_TPS";
			break;
		case dfMONITOR_DATA_TYPE_CHAT_PACKET_POOL:
			dataStat.name = "Chat_Server_PACKET_POOL";
			break;

		case dfMONITOR_DATA_TYPE_CHAT_UPDATEMSG_POOL:
			dataStat.name = "Chat_Server_MSGQ";
			break;
		default:
			break;

		}




	}


	//메세지 보내기
	agentManager->SendAllClient(sendMsg);

	SendMonitorServerData();
	DBSaveData();
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
	(*agentManager)[agentIndex].Type = static_cast<BYTE>(enAgentType::en_Client);
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
	CMonitorAgent* currentAgent;
	currentAgent = &(*agentManager)[agentIndex];
	if (currentAgent->Type == static_cast<BYTE>(enAgentType::en_Client))
	{
		agentManager->clientList.remove(ID);
	}
	else if (currentAgent->Type == static_cast<BYTE>(enAgentType::en_Server))
	{
		agentManager->serverList.remove(ID);
	}
	else
	{
		return false;
	}
	currentAgent->Clear();
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
	MakeCSUpdateMsg(MSG_networkRecv, dfMONITOR_DATA_TYPE_MONITOR_NETWORK_RECV, (int)networkRecv / 1024);
	MakeCSUpdateMsg(MSG_networkSend, dfMONITOR_DATA_TYPE_MONITOR_NETWORK_SEND, (int)networkSend / 1024);

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


bool CContentsManager::DBSaveData()
{
	unsigned short agentIndex;
	CMonitorAgent* currentAgent;
	BYTE dataType;

	DWORD nowTime = timeGetTime();

	if (nowTime - dbSavetime < 1000 * 60 * 30)
	{
		return false;
	}

	dbSavetime = nowTime;

	for (auto& it: agentManager->serverList)
	{
		agentIndex = CLanManager::GetIndex(it);
		currentAgent = &(*agentManager)[agentIndex];

		for (auto& agentIt : currentAgent->datatypeAverage_Map)
		{
			dataType = agentIt.first;
			DataAverage* data = &agentIt.second;
			CDBManager::MonitorData monitorData;

			time_t now = time(nullptr);                // 현재 시간 (time_t)
			std::tm localTime;
			localtime_s(&localTime, &now);

			std::ostringstream oss;
			oss << std::put_time(&localTime, "%Y-%m-%d %H:%M:%S");
			
			monitorData.aver = data->average;
			monitorData.max = data->max;
			monitorData.min = data->min;


			monitorData.timeStamp = oss.str();
			monitorData.dataType = data->name;
			
			data->average = 0;
			data->count = 0;
			data->max = 0;
			data->min = 0;

#ifndef __MAC__
			g_DBManager->WriteMonitorData(&monitorData);
#endif
		}
	}

	

	return true;
}