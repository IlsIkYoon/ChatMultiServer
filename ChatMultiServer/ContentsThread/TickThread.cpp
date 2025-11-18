#include "ContentsResource.h"
#include "TickThread.h"
#include "Log/Monitoring.h"
#include "ContentsFunc.h"

extern CWanServer* networkServer;


thread_local DWORD t_prevFrameTime;
thread_local DWORD t_fixedDeltaTime;
thread_local DWORD t_frame;
thread_local DWORD t_sec;

unsigned int TickThread(void*)
{
	CMonitor serverMornitor;

	DWORD startTime = timeGetTime();

	DWORD dwUpdateTick = startTime - FrameSec;
	t_sec = startTime / 1000;

	t_prevFrameTime = startTime - FrameSec;// 초기 값 설정

	t_sec = 0;
	unsigned long long sendProb = 0;

	while (1)
	{
		
		DWORD currentTime = timeGetTime();
		DWORD deltaTime = currentTime - t_prevFrameTime;
		DWORD deltaCount = deltaTime / FrameSec;
		t_fixedDeltaTime = deltaCount * FrameSec;


		sendProb++;

		if (sendProb % 4 == 0) 
		{
			networkServer->EnqueSendRequest();
		}


		DWORD logicTime = timeGetTime() - currentTime;

		if (logicTime < FrameSec)
		{

			Sleep(FrameSec - logicTime);
		}

		t_frame++;

		//여기서 이거 때문에 밀릴듯

		if (t_frame >= FrameRate)
		{
			char retval;
			retval = serverMornitor.CheckInput();
			switch (retval)
			{
			case static_cast<char>(en_InputType::en_ProcessExit):
				break;
			case static_cast<char>(en_InputType::en_SaveProfiler):
				WriteAllProfileData();
				break;
			default:
				break;
			}
			
			serverMornitor.ConsolPrint();

			TimeOutCheck();

			t_frame = 0;
			t_sec++;
		}

		t_prevFrameTime += t_fixedDeltaTime;

	}


	return 0;
}



