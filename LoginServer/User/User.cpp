#include "User.h"


User* CUserManager::GetUser(DWORD index)
{
	if (index > userMaxCount)
	{
		return nullptr;
	}

	return &userArr[index];
}

CUserManager::CUserManager(DWORD pUserMaxCount)
{
	userMaxCount = pUserMaxCount;
	userArr = new User[userMaxCount];
	currentUserCount = 0;

}

bool CUserManager::InitUser(unsigned long long sessionID)
{
	unsigned short userIndex;
	userIndex = NetWorkManager::GetIndex(sessionID);

	std::lock_guard guard(userArr[userIndex].userLock);

	

	userArr[userIndex].alive = true;
	userArr[userIndex].sessionID = sessionID;
	userArr[userIndex].timeOut = timeGetTime();

	InterlockedIncrement(&currentUserCount);

	return true;
}


bool CUserManager::DeleteUser(unsigned long long sessionID)
{
	unsigned short userIndex;
	userIndex = NetWorkManager::GetIndex(sessionID);

	std::lock_guard guard(userArr[userIndex].userLock);

	InterlockedDecrement(&currentUserCount);

	if (userArr[userIndex].sessionID != sessionID)
	{
		return false;
	}
	

	userArr[userIndex].alive = false;
	userArr[userIndex].sessionID = 0;
	userArr[userIndex].timeOut = 0;



	return true;
}