#include "ChatMultiServer.h"
#include "ContentsResource.h"
#include "ContentsThread/ContentsFunc.h"
#include "ContentsThread//ContentsThreadManager.h"
#include "Log/Monitoring.h"
CLanServer* networkServer;
CContentsThreadManager* contentsManager;
bool ChatMultiServer()
{
	CMonitor serverMornitor;
	procademy::CCrashDump dump;
	networkServer = new CLanServer;
	contentsManager = new CContentsThreadManager(networkServer);
	contentsManager->Start();
	//ContentsThreadManager 인스턴스 생성


	//종료 대기
	while (1)
	{
		Profiler p("Sleep 1000");
		Sleep(1000);
	}

	//서버 종료 절차
	contentsManager->End();
	

	return true;
}