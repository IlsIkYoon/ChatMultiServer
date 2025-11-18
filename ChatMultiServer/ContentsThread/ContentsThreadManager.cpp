#include "ContentsResource.h"
#include "ContentsThreadManager.h"
#include "ContentsFunc.h"
#include "Sector/Sector.h"
#include "TickThread.h"
#include "MonitorManager.h"

int CContentsThreadManager::playerMaxCount;
CPlayerManager* CContentsThreadManager::playerList;
CWanManager* CContentsThreadManager::ntManager;
CCharacterKeyManager* CContentsThreadManager::keyList;
extern CMonitorManager g_MonitorManager;


CContentsThreadManager::CContentsThreadManager(CWanManager* ntLib)
{
	ntManager = ntLib;
	playerMaxCount = 0;
	tickThread = NULL;
}

CContentsThreadManager::~CContentsThreadManager()
{
	//ÄÁÅÙÃ÷ ¾²·¹µå Á¤¸® ÀÛ¾÷
}


bool CContentsThreadManager::Start()
{
	ReadConfig();
	ntManager->Start();
	ContentsThreadInit();

	g_MonitorManager.RegistMonitor(L"127.0.0.1", monitorPort);

	return true;
}

bool CContentsThreadManager::ReadConfig()
{
	bool retval;

	int concurrentThreadcount;
	int sessionMaxCount;
	int workerThreadCount;
	int portNum;

	retval = txParser.GetData("ChatMultiServer_Config.ini");
	if (retval == false)
	{
		__debugbreak();
	}

	txParser.SearchData("PlayerMaxCount", &playerMaxCount);
	txParser.SearchData("ConcurrentCount", &concurrentThreadcount);
	txParser.SearchData("WorkerThreadCount", &workerThreadCount);
	txParser.SearchData("PortNum", &portNum);
	txParser.SearchData("SessionCount", &sessionMaxCount);
	txParser.SearchData("MonitorPort", &monitorPort);

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


	return true;
}