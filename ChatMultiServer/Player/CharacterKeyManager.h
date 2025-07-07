#pragma once
#include "ContentsResource.h"



class CCharacterKeyManager
{
	//전체 플레이어 아이디를 저장할 자료구조 필요

	//이 자료구조에서 접근할 때 락 필요

	std::unordered_map<ULONG64, ULONG64> _Key_uMap;
	std::recursive_mutex _Key_uMapLock;
	DWORD _playerMaxCount;

public:
	CCharacterKeyManager() = delete;

	CCharacterKeyManager(DWORD playerCount)
	{
		_playerMaxCount = playerCount;
		_Key_uMap.rehash(playerCount);
	}
	
	bool InsertID(ULONG64 characterKey, ULONG64 sessionID);
	bool DeleteID(ULONG64 characterKey, ULONG64 sessionID);


};