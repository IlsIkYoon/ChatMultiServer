#include "LoginServer.h"
#include "Network/NetworkManager.h"
#include "Contents/ContentsManager.h"
//-------------------------------------
// 전역변수
//-------------------------------------
extern CWanServer* g_NetworkManager;
extern CContentsManager* g_ContentsManager;


//-------------------------------------

bool LoginServer()
{
	g_NetworkManager = new CWanServer;
	g_ContentsManager = new CContentsManager(g_NetworkManager);
	
	Sleep(INFINITE);


	return true;
}