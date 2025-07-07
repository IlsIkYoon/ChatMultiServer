#pragma once
#include "Player/PlayerManager.h"
#include "Player/CharacterKeyManager.h"


class CContentsThreadManager
{
public:
	static int playerMaxCount;
	HANDLE tickThread;
	unsigned short monitorPort;

	static CWanManager* ntManager;
	static CPlayerManager* playerList;
	static CCharacterKeyManager* keyList;


	CContentsThreadManager() = delete;
	CContentsThreadManager(CWanManager* ntLib);
	~CContentsThreadManager();

	bool ReadConfig();
	bool ContentsThreadInit();
	bool Start();
	bool End();

};