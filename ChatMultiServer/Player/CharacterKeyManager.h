#pragma once
#include "ContentsResource.h"



class CCharacterKeyManager
{
	//전체 플레이어 아이디를 저장할 자료구조 필요

	//이 자료구조에서 접근할 때 락 필요

	std::unordered_set<ULONG64> _Key_uSet;
	std::mutex _Key_uSetLock;
	DWORD _playerMaxCount;

public:
	CCharacterKeyManager() = delete;

	CCharacterKeyManager(DWORD playerCount)
	{
		_playerMaxCount = playerCount;
		_Key_uSet.reserve(playerCount);
	}
	
	bool InsertID(ULONG64 characterKey);
	bool DeleteID(ULONG64 characterKey);


};