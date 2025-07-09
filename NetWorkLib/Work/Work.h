#pragma once
#include "pch.h"
#include "Buffer/SerializeBuf.h"
#include "Buffer/LFreeQ.h"
#include "Session/Session.h"



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


public:
	virtual bool WorkInit() = 0;
	virtual bool HandleMessage(CPacket* message, ULONG64 id) = 0; //Session 대상으로 뽑아냄
	virtual bool FrameLogic() = 0; //Player 대상으로 돌아갈 예정
};

struct WorkArg
{
	int threadindex;
	LFreeQ<CPacket*>* threadJobQ;
	Work* threadWork;
};

unsigned int ThreadWorkFunc(void* param);

class WorkManager
{


	bool RequestMoveToWork(BYTE toWork, ULONG64 ID);

};