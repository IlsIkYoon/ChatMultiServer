#include "Monitor.h"
#include "Contents/ContentsManager.h"

CMonitor g_Monitor;
extern CContentsManager* g_ContentsManager;

void CMonitor::ConsolPrintAll()
{
	printf("aa\n");
	return;
	ConsolPrintLoginCount();
	ConsolPrintUserCount();

}


void CMonitor::ConsolPrintLoginCount()
{
	printf("--------------------------------------------------------\n");
	printf("--------------------------------------------------------\n");
	printf("Login Success : %lld || LoginFailed : %lld\n", loginSuccessCount, loginFailedCount);
}

void CMonitor::ConsolPrintUserCount()
{
	printf("Current User : %d\n", g_ContentsManager->GetCurrentUser());

	return;
}