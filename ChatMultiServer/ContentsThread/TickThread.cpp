#include "ContentsResource.h"
#include "TickThread.h"
#include "Log/Monitoring.h"
#include "ContentsFunc.h"

extern CLanServer* ntServer;


extern thread_local DWORD t_prevFrameTime;
extern thread_local DWORD t_fixedDeltaTime;
extern thread_local DWORD t_frame;
extern thread_local DWORD t_sec;

unsigned int TickThread(void*)
{
	CMonitor serverMornitor;

	DWORD startTime = timeGetTime();

	DWORD dwUpdateTick = startTime - FrameSec;
	t_sec = startTime / 1000;

	t_prevFrameTime = startTime - FrameSec;// 초기 값 설정

	while (1)
	{
		DWORD currentTime = timeGetTime();
		DWORD deltaTime = currentTime - t_prevFrameTime;
		DWORD deltaCount = deltaTime / FrameSec;
		t_fixedDeltaTime = deltaCount * FrameSec;


		ntServer->EnqueSendRequest();


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
				//todo//모든 쓰레드가 프로파일러를 저장할 수 있게 하기
				//방법 마련..
				break;
			default:
				break;
			}
			
			serverMornitor.ConsolPrint();
			t_frame == 0;
			t_sec++;
		}

		t_prevFrameTime += t_fixedDeltaTime;

	}

	//todo//여기서 종료 절차


	return 0;
}



