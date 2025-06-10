#pragma once

#include "Resource/LoginServerResource.h"
#include "User/User.h"
#include "Network/NetworkManager.h"


class CContentsManager
{
public:
	CUserManager* userManager;
	CWanServer* networkManager;
	std::thread tickThread;

public:
	CContentsManager() = delete;
	CContentsManager(CWanServer* pNetworkManager);


	void tickThreadFunc();
	
	//-------------------------------------------
	// User에 대한 인터페이스
	//-------------------------------------------
	bool InitUser(unsigned long long sessionID);
	bool DeleteUser(unsigned long long sessionID);


	//-------------------------------------------
	// 메세지 처리 인터페이스
	//-------------------------------------------
	bool HandleContentsMessage(CPacket* message, ULONG64 ID);

	bool HandleLoginREQMsg(CPacket* message, ULONG64 ID);



};

