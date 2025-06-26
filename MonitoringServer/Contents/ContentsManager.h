#pragma once
#include "Resource/MonitoringServerResource.h"
#include "Network/NetworkManager.h"
#include "MonitorAgent/MonitorAgent.h"


class CContentsManager
{
	CWanServer* networkManager;
	CAgentManager* agentManager;
	char clientLoginToken[33];


public:
	CContentsManager() = delete;
	CContentsManager(CWanServer* pNetworkManager);


	bool HandleContentsMsg(CPacket* message, ULONG64 ID);

	bool HandleServerLoginMsg(CPacket* message, ULONG64 ID);
	bool HandleDataUpdateMsg(CPacket* message, ULONG64 ID);
	bool HandeClientLoginMsg(CPacket* message, ULONG64 ID);

	bool SendClientLoginResMsg(ULONG64 ID);


	bool DeleteAgent(ULONG64 ID);

};

