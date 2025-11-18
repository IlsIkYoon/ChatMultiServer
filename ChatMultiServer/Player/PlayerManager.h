#pragma once

#include "ContentsResource.h"
#include "Player.h"

class CPlayerManager
{
public:
	Player* playerArr;
	CPlayerManager(int playerCount);

	Player& operator[](int iDex)
	{
		return playerArr[iDex];
	}
};