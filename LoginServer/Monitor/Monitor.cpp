#include "Monitor.h"
#include "Contents/ContentsManager.h"

CMonitor_delete g_Monitor;
extern CContentsManager* g_ContentsManager;

void CMonitor_delete::ConsolPrintAll()
{
	printf("aa\n");
	return;
	ConsolPrintLoginCount();
	ConsolPrintUserCount();

}


void CMonitor_delete::ConsolPrintLoginCount()
{
	printf("--------------------------------------------------------\n");
	printf("--------------------------------------------------------\n");
	printf("Login Success : %lld || LoginFailed : %lld\n", loginSuccessCount, loginFailedCount);
}

void CMonitor_delete::ConsolPrintUserCount()
{
	printf("Current User : %d\n", g_ContentsManager->GetCurrentUser());

	return;
}