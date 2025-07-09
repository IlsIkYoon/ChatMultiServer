//----------------------------------------------------------------
//ConcurrentThread 1, WorkerThread 1의 Config File을 읽어야함
//로직이 완전한 싱글 스레드를 기반으로 설계됨
//----------------------------------------------------------------
#include "Resource/MonitoringServerResource.h"
#include "MonitoringServer.h"
#include "Network/NetworkManager.h"
#include "Contents/ContentsManager.h"
#include "DBConnector/DBConnector.h"

extern CWanServer* g_NetworkManager;
extern CContentsManager* g_ContentsManager;
CPdhManager g_PDH;
#ifndef __MAC__
CDBManager* g_DBManager;
#endif

bool MonitoringServer()
{
	g_NetworkManager = new CWanServer;

	int portNum;
	int sessionCount;
	int ethernetCount;

	txParser.GetData("MonitorServer_Config.ini");
	txParser.SearchData("portNum", &portNum);
	txParser.SearchData("sessionCount", &sessionCount);
	txParser.SearchData("ethernetCount", &ethernetCount);
	txParser.CloseData();
	
	g_NetworkManager->RegistPortNum(portNum);
	g_NetworkManager->RegistSessionMaxCoiunt(sessionCount);
	g_NetworkManager->RegistWorkerThreadCount(1);
	g_NetworkManager->RegistConcurrentCount(1);
	g_ContentsManager = new CContentsManager(g_NetworkManager);
	g_PDH.RegistEthernetMax(ethernetCount);
	g_PDH.Start();
#ifndef __MAC__
	g_DBManager = new CDBManager;
#endif
	g_NetworkManager->Start();

	CCpuUsage myCpu;

	while (1)
	{
		myCpu.UpdateCpuTime();
		printf("Total Process : %f, Monitor Process : %f\n", myCpu.ProcessorTotal(), myCpu.ProcessTotal());
		Sleep(1000);
	}
	

	Sleep(INFINITE);
	

	return true;
}

