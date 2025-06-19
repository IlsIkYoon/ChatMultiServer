//----------------------------------------------------------------
//ConcurrentThread 1, WorkerThread 1의 Config File을 읽어야함
//로직이 완전한 싱글 스레드를 기반으로 설계됨
//----------------------------------------------------------------

#include "MonitoringServer.h"
#include "Network/NetworkManager.h"
#include "Contents/ContentsManager.h"

extern CLanServer* g_NetworkManager;
extern CContentsManager* g_ContentsManager;

bool MonitoringServer()
{
	g_NetworkManager = new CLanServer;
	g_ContentsManager = new CContentsManager(g_NetworkManager);
	
	Sleep(INFINITE);
	

	return true;
}

