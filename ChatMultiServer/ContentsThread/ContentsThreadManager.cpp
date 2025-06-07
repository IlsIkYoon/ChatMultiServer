#include "ContentsResource.h"
#include "ContentsThreadManager.h"
#include "ContentsFunc.h"
#include "Sector/Sector.h"
#include "TickThread.h"

long CContentsThreadManager::threadIndex;
LFreeQ<CPacket*>* CContentsThreadManager::contentsJobQ;
HANDLE* CContentsThreadManager::hEvent_contentsJobQ;
int CContentsThreadManager::threadCount;
int CContentsThreadManager::playerCount;
CPlayerManager* CContentsThreadManager::playerList;
NetWorkManager* CContentsThreadManager::ntManager;
CCharacterKeyManager* CContentsThreadManager::keyList;

CContentsThreadManager::CContentsThreadManager(NetWorkManager* ntLib)
{
	ntManager = ntLib;
	threadIndex = 0;
	playerCount = PLAYER_MAXCOUNT; //default 값, 실제 값은 Parser로 읽어서 쓸 예정
	threadCount = CONTENTS_THREADCOUNT;
	contentsThreadArr = nullptr;
}

CContentsThreadManager::~CContentsThreadManager()
{
	//컨텐츠 쓰레드 정리 작업
}


bool CContentsThreadManager::Start()
{
	ReadConfig();
	ContentsThreadInit();



	return true;
}

bool CContentsThreadManager::ReadConfig()
{
	bool retval;
	retval = txParser.GetData("ContentsConfig.txt");
	if (retval == false)
	{
		__debugbreak();
	}

	txParser.SearchData("PlayerMaxCount", &playerCount);
	txParser.SearchData("ThreadCount", &threadCount);

	return true;
}

bool CContentsThreadManager::ContentsThreadInit()
{
	contentsJobQ = new LFreeQ<CPacket*>[threadCount];
	hEvent_contentsJobQ = new HANDLE[threadCount];
	contentsThreadArr = new HANDLE[threadCount];
	playerList = new CPlayerManager(playerCount);
	keyList = new CCharacterKeyManager(playerCount);
	
	

	for (int i = 0; i < threadCount; i++)
	{
		hEvent_contentsJobQ[i] = CreateEvent(NULL, false, false, NULL);
		contentsThreadArr[i] = (HANDLE)_beginthreadex(NULL, 0, ContentsThreadFunc, NULL, NULL, NULL);
	}

	tickThread = (HANDLE)_beginthreadex(NULL, 0, TickThread, NULL, NULL, NULL);

	return true;
}


//----------------------------------
// Frame을 위한 변수들, 내부에서 변수 사용할 가능성이 있어서 tls
//----------------------------------
thread_local DWORD t_prevFrameTime;
thread_local DWORD t_fixedDeltaTime;
thread_local DWORD t_frame;
thread_local DWORD t_sec;


unsigned int CContentsThreadManager::ContentsThreadFunc(void*)
{
	long myIndex;
	myIndex = InterlockedIncrement(&threadIndex) - 1;

	while (1)
	{

		

		WaitForSingleObject(hEvent_contentsJobQ[myIndex], INFINITE);


		//이게 아예 없어짐//
		UpdateContentsLogic(myIndex, t_fixedDeltaTime); //멀티 쓰레드로 수정
		


	}





	return true;
}



void CContentsThreadManager::UpdateContentsLogic(long myIndex, DWORD deltaTime)
{

	for (int i = 0; i < playerCount; i++)
	{
		if (i % threadCount != myIndex)
		{
			continue;
		}
		//playerList[i]이런 식의 접근으로 하면 될듯 ?
		if ((*playerList)[i].isAlive() == false)
		{
			continue;
		}

		CheckSector((*playerList)[i].GetID());
		(*playerList)[i].Move(deltaTime);
		CheckSector((*playerList)[i].GetID());
	}

	TimeOutCheck();



}



bool CContentsThreadManager::End()
{

	return true;
}