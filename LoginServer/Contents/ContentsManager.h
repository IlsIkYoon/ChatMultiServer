#pragma once

#include "Resource/LoginServerResource.h"
#include "User/User.h"
#include "Network/NetworkManager.h"
#include "Connector/DBConnector.h"
#include "Connector/RedisConnector.h"

class CContentsManager
{
public:
	CUserManager* userManager;
	CWanServer* networkManager;
	CDBConnector* DBConnector;
	CRedisConnector* RedisConnector;


	std::thread tickThread;

	WCHAR gameServerIP[16];
	unsigned short gameServerPort;
	WCHAR chatServerIP[16];
	unsigned short chatServerPort;

	std::string redisIP;
	std::size_t redisPort;

public:
	CContentsManager() = delete;
	CContentsManager(CWanServer* pNetworkManager);


	void tickThreadFunc();
	
	//-------------------------------------------
	// User에 대한 인터페이스
	//-------------------------------------------
	bool InitUser(unsigned long long sessionID);
	bool DeleteUser(unsigned long long sessionID);

	DWORD GetCurrentUser();

	//-------------------------------------------
	// 메세지 처리 인터페이스
	//-------------------------------------------
	bool HandleContentsMessage(CPacket* message, ULONG64 ID);

	//메세지 처리부
	bool HandleLoginREQMsg(CPacket* message, ULONG64 ID);
	
	//메세지 전송부
	bool MsgSetServerAddr(CPacket* message);


	bool SetToken(std::string Value, ULONG64 key);

};

