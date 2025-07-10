#pragma once

#include "GameEchoServerResource.h"


class CPlayer
{
	ULONG64 _id;
public:
	INT64 accountNo;
	LFreeQ<CPacket*> messageQ;
	BYTE work;

public:
	CPlayer()
	{
		_id = 0;
		accountNo = 0;
		work = 0;
	}

	void init(ULONG64 ID);
	void clear();


};



class CPlayerManager
{
	CPlayer* playerArr;
	
public:
	unsigned short playerMaxCount;


public:
	CPlayer& operator[](int iDex)
	{
		return playerArr[iDex];
	}
	CPlayerManager(unsigned int playerMaxCount);

};

enum class enPlayerWork
{
	en_Auth = 10,
	en_Game
};