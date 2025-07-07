#include "CharacterKeyManager.h"
#include "ContentsThread/ContentsFunc.h"

extern CWanServer* networkServer;

bool CCharacterKeyManager::InsertID(ULONG64 characterKey, ULONG64 sessionID)
{
	std::lock_guard guard(_Key_uMapLock);
	if (_Key_uMap.contains(characterKey) == true)
	{
		auto logMsg = std::format("Duplicated Character Key [{}]", characterKey);
		
		CWanManager::_log.EnqueLog(logMsg);

		networkServer->DisconnectSession(_Key_uMap[characterKey]);

		DeleteID(characterKey, _Key_uMap[characterKey]);
	}
	if (_Key_uMap.size() >= _playerMaxCount)
	{
		__debugbreak();
		auto logMsg = std::format("PlayerCount Full [{}]", characterKey);

		CWanManager::_log.EnqueLog(logMsg);
		return false;
	}

	_Key_uMap.insert({ characterKey, sessionID });

	return true;
}

bool CCharacterKeyManager::DeleteID(ULONG64 characterKey, ULONG64 sessionID)
{
	std::lock_guard guard(_Key_uMapLock);

	if (_Key_uMap.contains(characterKey) == false)
	{
		auto logMsg = std::format("Key is Not Exist !!! [{}]", characterKey);

		CWanManager::_log.EnqueLog(logMsg);

		__debugbreak();

		return false;
	}

	_Key_uMap.erase(characterKey);

	return true;
}