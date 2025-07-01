#include "MonitoringServer.h"



int main()
{
	procademy::CCrashDump dump;

	if (MonitoringServer() == false)
	{
		printf("Server Failed!!!!\n");
	}


	return 0;
}