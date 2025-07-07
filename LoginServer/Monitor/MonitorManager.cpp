#include "MonitorManager.h"
#include "Network/NetworkManager.h"
#include "CommonProtocol.h"


extern CWanServer* g_NetworkManager;
CCpuUsage g_CpuUsage;
CPdhManager g_Pdh;

extern unsigned long long g_LoginSessionCount;
extern unsigned long long g_CPacketAllocCount;

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



bool CMonitor::UpdateAllMonitorData()
{

	int loginServer_Cpu;
	double loginServer_Mem;
	int loginServer_authTps;

	g_CpuUsage.UpdateCpuTime();
	loginServer_Cpu =  (int)g_CpuUsage.ProcessTotal();
	g_Pdh.GetMemoryData(&loginServer_Mem, nullptr, nullptr, nullptr);
	loginServer_authTps = (int)InterlockedExchange(&loginSuccessCount, 0);

	UpdateMonitor(dfMONITOR_DATA_TYPE_LOGIN_SERVER_RUN, 1);
	UpdateMonitor(dfMONITOR_DATA_TYPE_LOGIN_SERVER_CPU, loginServer_Cpu);
	UpdateMonitor(dfMONITOR_DATA_TYPE_LOGIN_SERVER_MEM, (int)loginServer_Mem / 1024 / 1024);
	UpdateMonitor(dfMONITOR_DATA_TYPE_LOGIN_SESSION, g_LoginSessionCount);
	UpdateMonitor(dfMONITOR_DATA_TYPE_LOGIN_AUTH_TPS, loginServer_authTps);
	UpdateMonitor(dfMONITOR_DATA_TYPE_LOGIN_PACKET_POOL, g_CPacketAllocCount);

	return true;
}