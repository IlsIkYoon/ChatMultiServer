#include "MonitorManager.h"
#include "Network/Network.h"
#include "CommonProtocol.h"

extern CWanServer* g_NetworkManager;


CMonitorManager::CMonitorManager()
{
	myServerNo = 1;
	mSessionID = 0;
}

bool CMonitorManager::RegistMonitor(std::wstring ip, unsigned short portNum)
{
	if (g_NetworkManager->ConnectServer(ip, portNum, &mSessionID) == false)
	{
		__debugbreak();
	}

	CPacket* sendMsg;
	sendMsg = CPacket::Alloc();

	WORD type;
	type = en_PACKET_SS_MONITOR_LOGIN;

	*sendMsg << type;
	*sendMsg << myServerNo;

	g_NetworkManager->SendPacket(mSessionID, sendMsg);

	sendMsg->DecrementUseCount();
	return true;
}

bool CMonitorManager::UpdateMonitor(BYTE dataType, int dataValue)
{
	CPacket* sendMsg;
	sendMsg = CPacket::Alloc();

	WORD type;
	unsigned int nowTime;
	type = en_PACKET_SS_MONITOR_DATA_UPDATE;
	nowTime = (unsigned int)time(nullptr);

	*sendMsg << type;
	*sendMsg << dataType;
	*sendMsg << dataValue;
	*sendMsg << nowTime;

	g_NetworkManager->SendPacket(mSessionID, sendMsg);

	sendMsg->DecrementUseCount();

	return true;
}