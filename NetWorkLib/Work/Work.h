#pragma once
#include "pch.h"
#include "Buffer/SerializeBuf.h"
#include "Buffer/LFreeQ.h"
#include "Session/Session.h"
#include "WorkProtocol.h"
#include "NetworkManager/Network.h"


struct ThreadJob
{
	BYTE jobType;
	ULONG64 id;
};


class Work
{
public:
	LFreeQ<ThreadJob> ThreadMessageQ;
	std::list<Session*> threadSessionList;
	CSessionManager* sessionManager;
	CNetwork* networkManager;

public:
	virtual bool WorkInit() = 0;
	virtual bool HandleMessage(CPacket* message, ULONG64 id) = 0; //Session 대상으로 뽑아냄
	virtual bool FrameLogic() = 0; //Player 대상으로 돌아갈 예정
	virtual void OnCreateSession(ULONG64 ID) = 0; 
	virtual void OnDeleteSession(ULONG64 ID) = 0; 


	bool CreateSession(ULONG64 ID);
	bool DeleteSession(ULONG64 ID);

	bool HandleCreateSessionMsg(ULONG64 ID);
	bool HandleDeleteSessionMsg(ULONG64 ID);

};

struct WorkArg
{
	Work* threadWork;
};

class WorkManager
{
	bool RequestMoveToWork(BYTE toWork, ULONG64 ID);
};

unsigned int ThreadWorkFunc(void* param);
bool HandleThreadJob(Work* myWork, ThreadJob* currentJob);