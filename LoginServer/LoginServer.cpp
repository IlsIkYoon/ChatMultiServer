#include "LoginServer.h"
#include "Network/NetworkManager.h"
#include "Contents/ContentsManager.h"
#include "Monitor/MonitorManager.h"
//-------------------------------------
// 전역변수
//-------------------------------------
extern CWanServer* g_NetworkManager;
extern CContentsManager* g_ContentsManager;

CMonitor g_MonitorManager;
CPdhManager g_PDH;


//-------------------------------------

bool LoginServer()
{
	procademy::CCrashDump dump;
	g_PDH.Start();
	
	g_NetworkManager = new CWanServer;
	g_ContentsManager = new CContentsManager(g_NetworkManager);
	
	Sleep(INFINITE);


	return true;
}