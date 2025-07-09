#include "pch.h"
#include "Work.h"


HANDLE WorkThreadArr[128];


unsigned int ThreadWorkFunc(void* param)
{
	WorkArg* workParam;
	Work* myWork;
	unsigned short myIdex;
	CPacket* currentJob;
	LFreeQ<CPacket*>* myThreadQ;

	DWORD prevTime;
	DWORD currentTime;
	DWORD resultTime;


	workParam = (WorkArg*)param;


	myWork = workParam->threadWork;
	myThreadQ = workParam->threadJobQ;
	myIdex = workParam->threadindex;

	myWork->WorkInit();

	prevTime = timeGetTime();

	while (1)
	{
		for (auto& it : myWork->threadSessionList)
		{
			if (it->jobQ.GetSize() == 0)
			{
				continue;
			}

			currentJob = it->jobQ.Dequeue();
			myWork->HandleMessage(currentJob, it->_ID._ulong64);
			currentJob->DecrementUseCount();
		}

		myWork->FrameLogic();
		//프레임 슬립//

		currentTime = timeGetTime();
		resultTime = currentTime - prevTime;
		if (resultTime < 40)
		{
			Sleep(40 - resultTime);
		}

		prevTime += 40;
	}

	return 0;
}



bool WorkManager::RequestMoveToWork(BYTE toWork, ULONG64 ID)
{
	//세션 매니저에게 현재 위치 질의

	


	return true;
}