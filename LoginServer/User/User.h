#pragma once

#include "Resource/LoginServerResource.h"

class User
{
public:
	bool alive;
	DWORD timeOut;
	unsigned long long sessionID;
	std::mutex userLock;

	User()
	{
		alive = false;
		timeOut = 0;
		sessionID = 0;
	}
};


class CUserManager
{
public:
	User* userArr;
	DWORD userMaxCount;
	DWORD currentUserCount;
	

public:
	CUserManager() = delete;
	CUserManager(DWORD userMaxCount);
	User* GetUser(DWORD index);
	bool InitUser(unsigned long long sessionID);
	bool DeleteUser(unsigned long long sessionID);

};
