#include "ChatMultiServer.h"
#include "ContentsResource.h"
#include "ContentsThread/ContentsFunc.h"
#include "ContentsThread//ContentsThreadManager.h"
#include "Log/Monitoring.h"
CLanServer* ntServer;
CContentsThreadManager* contentsManager;
bool ChatMultiServer()
{
	CMornitor serverMornitor;
	procademy::CCrashDump dump;
	ntServer = new CLanServer;
	contentsManager = new CContentsThreadManager(ntServer);
	contentsManager->Start();
	//ContentsThreadManager 인스턴스 생성


	//종료 대기
	while (1)
	{
		Sleep(1000);
	}

	//서버 종료 절차
	contentsManager->End();
	

	return true;
}