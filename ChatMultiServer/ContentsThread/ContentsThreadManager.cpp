#include "ContentsResource.h"
#include "ContentsThreadManager.h"
#include "ContentsFunc.h"
#include "Sector/Sector.h"


long CContentsThreadManager::threadIndex;
LFreeQ<CPacket*>* CContentsThreadManager::contentsJobQ;
int CContentsThreadManager::threadCount;
int CContentsThreadManager::playerCount;
CPlayerManager* CContentsThreadManager::playerList;


CContentsThreadManager::CContentsThreadManager(NetWorkManager* ntLib)
{
	ntManager = ntLib;
	threadIndex = 0;
	playerCount = PLAYER_MAXCOUNT; //default 값, 실제 값은 Parser로 읽어서 쓸 예정
	threadCount = CONTENTS_THREADCOUNT;


	contentsJobQ = new LFreeQ<CPacket*>[threadCount];
	contentsThreadArr = new HANDLE[threadCount];
	playerList = new CPlayerManager(playerCount);
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
	for (int i = 0; i < threadCount; i++)
	{
		contentsThreadArr[i] = (HANDLE)_beginthreadex(NULL, 0, ContentsThreadFunc, NULL, NULL, NULL);
	}

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

	InitContentsResource(); //todo//멀티쓰레드에 맞게 수정해야함

	DWORD startTime = timeGetTime();

	DWORD dwUpdateTick = startTime - FrameSec;
	t_sec = startTime / 1000;

	t_prevFrameTime = startTime - FrameSec;// 초기 값 설정

	printf("Thread 생성 : %d\n", GetCurrentThreadId());
	while (1)
	{
		DWORD currentTime = timeGetTime();
		DWORD deltaTime = currentTime - t_prevFrameTime;
		DWORD deltaCount = deltaTime / FrameSec;
		t_fixedDeltaTime = deltaCount * FrameSec;

		while (contentsJobQ[myIndex].GetSize() != 0)
		{
			HandleContentJob(myIndex);//todo//멀티쓰레드로 수정
		}

		UpdateContentsLogic(myIndex, t_fixedDeltaTime); //멀티 쓰레드로 수정

		//send에 대한 고민은 좀 필요

		DWORD logicTime = timeGetTime() - currentTime;

		if (logicTime < FrameSec)
		{

			Sleep(FrameSec - logicTime);
		}

		t_frame++;

		t_prevFrameTime += t_fixedDeltaTime;
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
