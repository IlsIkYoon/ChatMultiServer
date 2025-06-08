#pragma once
#include "Player/PlayerManager.h"
#include "Player/CharacterKeyManager.h"


class CContentsThreadManager
{
public:
	static int playerMaxCount;
	HANDLE tickThread;

	static NetWorkManager* ntManager;
	static CPlayerManager* playerList;
	static CCharacterKeyManager* keyList;


	CContentsThreadManager() = delete;
	CContentsThreadManager(NetWorkManager* ntLib);
	~CContentsThreadManager();

	bool ReadConfig();
	bool ContentsThreadInit();
	bool Start();
	bool End();

};