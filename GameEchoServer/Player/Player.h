#pragma once

#include "GameEchoServerResource.h"


class CPlayer
{
	ULONG64 _id;
public:
	INT64 accountNo;
	LFreeQ<CPacket*> messageQ;


public:
	CPlayer()
	{
		_id = 0;
	}

	void init();
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


};