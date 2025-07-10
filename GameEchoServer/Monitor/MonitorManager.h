#pragma once
#include "GameEchoServerResource.h"


class CMonitorManager
{
	// 모니터링 프로토콜에 맞춰서 데이터 넣고 보내줌 
	ULONG64 mSessionID;
	int myServerNo;

public:
	CMonitorManager();

	bool RegistMonitor(std::wstring ip, unsigned short portNum);
	bool UpdateMonitor(BYTE dataType, int dataValue);
};