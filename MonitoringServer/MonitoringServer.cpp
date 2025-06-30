//----------------------------------------------------------------
//ConcurrentThread 1, WorkerThread 1의 Config File을 읽어야함
//로직이 완전한 싱글 스레드를 기반으로 설계됨
//----------------------------------------------------------------

#include "MonitoringServer.h"
#include "Network/NetworkManager.h"
#include "Contents/ContentsManager.h"

extern CWanServer* g_NetworkManager;
extern CContentsManager* g_ContentsManager;
CPdhManager g_PDH;


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
	g_ContentsManager = new CContentsManager(g_NetworkManager);
	g_PDH.RegistEthernetMax(ethernetCount);
	g_PDH.Start();

	g_NetworkManager->Start();

	
	

	Sleep(INFINITE);
	

	return true;
}

