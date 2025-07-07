#pragma once
#include "Resource/MonitoringServerResource.h"
#include "NetworkManager/NetWorkManager.h"

#define NO_SERVER 0

extern CWanServer* g_NetworkManager;
struct DataAverage
{
	std::string name;
	int count;
	int average;
	int min;
	int max;
};

enum class enAgentType
{
	en_IDLE = 0,
	en_Server,
	en_Client
};

enum class enAgentStatus
{
	en_IDLE = 0,
	en_Alive
};

class CMonitorAgent
{
public:
	BYTE Type;
	BYTE Status;
	int ServerNo;
	ULONG64 sessionID;
	long bResCreate;
	std::map<BYTE, DataAverage> datatypeAverage_Map;


	CMonitorAgent()
	{
		Type = static_cast<BYTE>(enAgentType::en_IDLE);
		Status = static_cast<BYTE>(enAgentStatus::en_IDLE);
		ServerNo = NO_SERVER;
		sessionID = 0;
		bResCreate = 0;
	}

	bool Clear()
	{
		Status = static_cast<BYTE>(enAgentStatus::en_IDLE);
		Type = static_cast<BYTE>(enAgentType::en_IDLE);
		ServerNo = NO_SERVER;
		return true;
	}
	
};


class CAgentManager
{
public:
	CMonitorAgent* AgentArr;
	DWORD MaxAgentCount;

	std::list<ULONG64> clientList;
	std::list<ULONG64> serverList;

	CAgentManager() = delete;
	CAgentManager(DWORD pMaxAgentCount)
	{
		MaxAgentCount = pMaxAgentCount;
		AgentArr = new CMonitorAgent[MaxAgentCount];
	}

	bool RegistClient(ULONG64 ID)
	{
		//싱글로 설계할 예정이라 락을 안 잡음
		clientList.push_back(ID);
		return true;
	}
	bool RegistServer(ULONG64 ID)
	{
		//싱글로 설계할 예정이라 락을 안 잡음
		serverList.push_back(ID);
		return true;
	}

	bool SendAllClient(CPacket* sendMsg)
	{
		for (const auto& it : clientList)
		{
			g_NetworkManager->SendPacket(it, sendMsg);
		}

		g_NetworkManager->EnqueSendRequest();
		return true;
	}



	CMonitorAgent& operator[](unsigned short iDex)
	{
		if (iDex > (unsigned short)MaxAgentCount)
		{
			__debugbreak();
		}

		return AgentArr[iDex];
	}

};