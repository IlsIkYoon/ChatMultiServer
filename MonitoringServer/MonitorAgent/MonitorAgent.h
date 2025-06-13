#pragma once
#include "Resource/MonitoringServerResource.h"

#define NO_SERVER 0

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

	CMonitorAgent()
	{
		Type = static_cast<BYTE>(enAgentType::en_IDLE);
		Status = static_cast<BYTE>(enAgentStatus::en_IDLE);
		ServerNo = NO_SERVER;
	}

};


class CAgentManager
{
public:
	CMonitorAgent* AgentArr;
	DWORD MaxAgentCount;
	CAgentManager() = delete;
	CAgentManager(DWORD pMaxAgentCount)
	{
		MaxAgentCount = pMaxAgentCount;
		AgentArr = new CMonitorAgent[MaxAgentCount];
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