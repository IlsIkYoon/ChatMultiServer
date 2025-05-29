#pragma once
#include "Player/PlayerManager.h"



class CContentsThreadManager
{
public:
	static int threadCount;
	static int playerCount;
	HANDLE* contentsThreadArr;
	HANDLE tickThread;


	static NetWorkManager* ntManager;


	static CPlayerManager* playerList;

	CContentsThreadManager() = delete;
	CContentsThreadManager(NetWorkManager* ntLib);
	~CContentsThreadManager();

	bool ReadConfig();
	bool ContentsThreadInit();
	bool Start();
	bool End();


	static long threadIndex;
	static LFreeQ<CPacket*>* contentsJobQ;
	static HANDLE* hEvent_contentsJobQ;

	static unsigned int ContentsThreadFunc(void*);


	//----------------------------------------------
	// 매 프레임마다 해야할 일 
	//----------------------------------------------
	static void UpdateContentsLogic(long myIndx, DWORD deltaTime);





};