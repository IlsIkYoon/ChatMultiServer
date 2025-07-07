#include "MonitorManager.h"
#include "ContentsThread/ContentsFunc.h"
#include "Msg/CommonProtocol.h"

extern CWanServer* networkServer;

CMonitorManager::CMonitorManager()
{
	myServerNo = 1;
	mSessionID = 0;
}

bool CMonitorManager::RegistMonitor(std::wstring ip, unsigned short portNum)
{
	if (networkServer->ConnectServer(ip, portNum, &mSessionID) == false)
	{
		__debugbreak();
	}

	CPacket* sendMsg;
	sendMsg = CPacket::Alloc();

	WORD type;
	type = en_PACKET_SS_MONITOR_LOGIN;

	*sendMsg << type;
	*sendMsg << myServerNo;

	networkServer->SendPacket(mSessionID, sendMsg);

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

	networkServer->SendPacket(mSessionID, sendMsg);

	sendMsg->DecrementUseCount();

	return true;
}