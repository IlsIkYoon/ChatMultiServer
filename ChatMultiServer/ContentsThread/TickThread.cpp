#include "ContentsResource.h"
#include "TickThread.h"
#include "Log/Monitoring.h"
#include "ContentsFunc.h"

extern CLanServer* networkServer;


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

		if (sendProb % 2 == 0) 
		{
			networkServer->EnqueSendRequest();
		}


		DWORD logicTime = timeGetTime() - currentTime;

		if (logicTime < FrameSec)
		{

			Sleep(FrameSec - logicTime);
		}


		t_frame++;

		if (t_frame >= FrameRate)
		{
			char retval;
			retval = serverMornitor.CheckInput();
			switch (retval)
			{
			case static_cast<char>(en_InputType::en_ProcessExit):
				//todo//서버 종료 요청 실행
				break;
			case static_cast<char>(en_InputType::en_SaveProfiler):
				WriteAllProfileData();
				break;
			default:
				break;
			}
			
			{
				//Profiler p("Consol Print");
				serverMornitor.ConsolPrint();
			}

			TimeOutCheck();

			t_frame = 0;
			t_sec++;
		}

		t_prevFrameTime += t_fixedDeltaTime;

	}

	//todo//여기서 종료 절차


	return 0;
}



