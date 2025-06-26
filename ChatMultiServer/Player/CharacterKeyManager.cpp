#include "CharacterKeyManager.h"


bool CCharacterKeyManager::InsertID(ULONG64 characterKey)
{
	std::lock_guard guard(_Key_uSetLock);
	if (_Key_uSet.contains(characterKey) == true)
	{
		auto logMsg = std::format("Duplicated Character Key [{}]", characterKey);

		CWanManager::_log.EnqueLog(logMsg.c_str());
		return false;
	}
	if (_Key_uSet.size() >= _playerMaxCount)
	{
		__debugbreak();
		auto logMsg = std::format("PlayerCount Full [{}]", characterKey);

		CWanManager::_log.EnqueLog(logMsg.c_str());
		return false;
	}

	_Key_uSet.insert(characterKey);

	return true;
}

bool CCharacterKeyManager::DeleteID(ULONG64 characterKey)
{
	std::lock_guard guard(_Key_uSetLock);

	if (_Key_uSet.contains(characterKey) == false)
	{
		auto logMsg = std::format("Key is Not Exist !!! [{}]", characterKey);

		CWanManager::_log.EnqueLog(logMsg.c_str());

		__debugbreak();

		return false;
	}

	_Key_uSet.erase(characterKey);

	return true;
}