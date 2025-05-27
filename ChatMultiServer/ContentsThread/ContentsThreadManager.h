#pragma once
#include "Player/PlayerManager.h"



class CContentsThreadManager
{
public:
	static int threadCount;
	static int playerCount;
	HANDLE* contentsThreadArr;

	static CPlayerManager* playerList;

	CContentsThreadanager();
	~CContentsThreadManager();

	bool ReadConfig();
	bool ContentsThreadInit();
	bool Start();


	static long threadIndex;
	static LFreeQ<CPacket*>* contentsJobQ;

	static unsigned int ContentsThreadFunc(void*);


	//----------------------------------------------
	// 매 프레임마다 해야할 일 
	//----------------------------------------------
	static void UpdateContentsLogic(long myIndx, DWORD deltaTime);





};