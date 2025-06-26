#include "ContentsResource.h"
#include "ContentsThreadManager.h"
#include "ContentsFunc.h"
#include "Sector/Sector.h"
#include "TickThread.h"

int CContentsThreadManager::playerMaxCount;
CPlayerManager* CContentsThreadManager::playerList;
CWanManager* CContentsThreadManager::ntManager;
CCharacterKeyManager* CContentsThreadManager::keyList;

CContentsThreadManager::CContentsThreadManager(CWanManager* ntLib)
{
	ntManager = ntLib;
	playerMaxCount = 0;
	tickThread = NULL;
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

	int concurrentThreadcount;
	int sessionMaxCount;
	int workerThreadCount;
	int portNum;

	retval = txParser.GetData("ChatMultiServer_Config.txt");
	if (retval == false)
	{
		__debugbreak();
	}

	txParser.SearchData("PlayerMaxCount", &playerMaxCount);
	txParser.SearchData("ConcurrentCount", &concurrentThreadcount);
	txParser.SearchData("WorkerThreadCount", &workerThreadCount);
	txParser.SearchData("PortNum", &portNum);
	txParser.SearchData("SessionCount", &sessionMaxCount);

	ntManager->RegistConcurrentCount(concurrentThreadcount);
	ntManager->RegistPortNum(portNum);
	ntManager->RegistSessionMaxCoiunt(sessionMaxCount);
	ntManager->RegistWorkerThreadCount(workerThreadCount);


	return true;
}

bool CContentsThreadManager::ContentsThreadInit()
{
	playerList = new CPlayerManager(playerMaxCount);
	keyList = new CCharacterKeyManager(playerMaxCount);
	
	tickThread = (HANDLE)_beginthreadex(NULL, 0, TickThread, NULL, NULL, NULL);

	return true;
}

bool CContentsThreadManager::End()
{
	//todo//서버 종료 절차에 따른 리소스 반환

	return true;
}