#include "pch.h"
#include "Work.h"
#include "NetworkManager/NetWorkManager.h"


HANDLE WorkThreadArr[128];


unsigned int ThreadWorkFunc(void* param)
{
	WorkArg* workParam;
	Work* myWork;
	CPacket* currentSessionJob;
	ThreadJob currentThreadJob;

	DWORD prevTime;
	DWORD currentTime;
	DWORD resultTime;

	workParam = (WorkArg*)param;


	myWork = workParam->threadWork;

	myWork->WorkInit();

	prevTime = timeGetTime();

	while (1)
	{
		while(myWork->ThreadMessageQ.GetSize() != 0)
		{
			currentThreadJob = myWork->ThreadMessageQ.Dequeue();
			HandleThreadJob(myWork, &currentThreadJob);
		}

		//for (auto& it : myWork->threadSessionList)
		for(std::list<Session*>::iterator it = myWork->threadSessionList.begin(); it != myWork->threadSessionList.end();)
		{
			std::list<Session*>::iterator nextIt;
			nextIt = std::next(it);

			while (1)
			{

				if ((*it)->jobQ.GetSize() == 0)
				{
					break;
				}

				currentSessionJob = (*it)->jobQ.Dequeue();
				myWork->HandleMessage(currentSessionJob, (*it)->_ID._ulong64);
				currentSessionJob->DecrementUseCount();
			}

			it = nextIt;
		}
		
		myWork->FrameLogic();

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


bool Work::CreateSession(ULONG64 ID)
{
	ThreadJob message;
	Session* currentSession;
	unsigned short sessionIndex;

	sessionIndex = CWanManager::GetIndex(ID);
	currentSession = &(*sessionManager)[sessionIndex];

	networkManager->IncrementSessionIoCount(currentSession); // 이 시점에 이미 들어간 것임 
	if (currentSession->_ID._ulong64 != ID)
	{
		networkManager->DecrementSessionIoCount(currentSession);
		return false;
	}
	currentSession->currentWork = this;

	message.id = ID;
	message.jobType = static_cast<BYTE>(enNetworkThreadProtocol::en_CreateSession);

	ThreadMessageQ.Enqueue(message);

	return true;
}
bool Work::DeleteSession(ULONG64 ID)
{
	ThreadJob message;
	message.id = ID;
	message.jobType = static_cast<BYTE>(enNetworkThreadProtocol::en_DeleteSession);
	ThreadMessageQ.Enqueue(message);

	return true;
}


bool Work::HandleCreateSessionMsg(ULONG64 ID)
{
	Session* currentSession;
	unsigned short sessionIndex;
	
	sessionIndex = CWanManager::GetIndex(ID);
	currentSession = &(*sessionManager)[sessionIndex];

	threadSessionList.push_back(currentSession);
	
	OnCreateSession(ID);

	return true;
}
bool Work::HandleDeleteSessionMsg(ULONG64 ID)
{
	Session* currentSession;
	unsigned short sessionIndex;

	sessionIndex = CWanManager::GetIndex(ID);
	currentSession = &(*sessionManager)[sessionIndex];

	int count = std::count(threadSessionList.begin(), threadSessionList.end(), currentSession);

	if (count != 0 && count != 1)
	{
		__debugbreak();
	}

	threadSessionList.remove(currentSession);
	networkManager->DecrementSessionIoCount(currentSession);

	OnDeleteSession(ID);

	return true;
}


bool HandleThreadJob(Work* myWork, ThreadJob* currentJob)
{
	ULONG64 jobID;
	BYTE jobType;
	jobID = currentJob->id;
	jobType = currentJob->jobType;

	switch (jobType)
	{
	case static_cast<BYTE>(enNetworkThreadProtocol::en_CreateSession) : 
		myWork->HandleCreateSessionMsg(jobID);
		break;
	case static_cast<BYTE>(enNetworkThreadProtocol::en_DeleteSession):
		myWork->HandleDeleteSessionMsg(jobID);
		break;
	default:
		__debugbreak();
		return false;
	}

	return true;;
}