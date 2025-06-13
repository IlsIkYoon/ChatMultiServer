#pragma once
#include "Resource/MonitoringServerResource.h"
#include "Network/NetworkManager.h"
#include "MonitorAgent/MonitorAgent.h"


class CContentsManager
{
	CLanServer* networkManager;
	CAgentManager* agentManager;
	char clientLoginToken[33];


public:
	CContentsManager() = delete;
	CContentsManager(CLanServer* pNetworkManager);


	bool HandleContentsMsg(CPacket* message, ULONG64 ID);

	bool HandleServerLoginMsg(CPacket* message, ULONG64 ID);
	bool HandleDataUpdateMsg(CPacket* message, ULONG64 ID);
	bool HandeClientLoginMsg(CPacket* message, ULONG64 ID);

};

