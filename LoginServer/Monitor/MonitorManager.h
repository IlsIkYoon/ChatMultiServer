#pragma once
#include "Resource/LoginServerResource.h"


class CMonitorManager
{
public:
	// 모니터링 프로토콜에 맞춰서 데이터 넣고 보내줌 
	ULONG64 mSessionID;
	int myServerNo;

	CMonitorManager();

	bool RegistMonitor(std::wstring ip, unsigned short portNum);
	bool UpdateMonitor(BYTE dataType, int dataValue);
};

class CMonitor : public CMonitorManager
{
public:

	unsigned long long loginSuccessCount;

	CMonitor()
	{
		loginSuccessCount = 0;
	}

	bool UpdateAllMonitorData();


};