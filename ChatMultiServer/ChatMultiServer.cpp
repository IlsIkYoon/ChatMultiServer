#include "ChatMultiServer.h"
#include "ContentsResource.h"
#include "ContentsThread/ContentsFunc.h"
#include "ContentsThread//ContentsThreadManager.h"

CLanServer* ntServer;
CContentsThreadManager* contentsManager;
bool ChatMultiServer()
{
	procademy::CCrashDump dump;
	ntServer = new CLanServer;
	contentsManager = new CContentsThreadManager(ntServer);
	contentsManager->Start();
	//ContentsThreadManager 인스턴스 생성

	Sleep(INFINITE);
	

	//키 입력 대기(입력하면 종료)
	//종료 입력 시에는 비밀번호 입력해야 함

	

	return true;
}