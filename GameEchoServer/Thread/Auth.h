#pragma once
#include "GameEchoServerResource.h"
#include "Thread/ThreadWork.h"


class AuthThreadWork : public Work
{
public:
	LFreeQ<ThreadJob> threadMessageQ;
	std::list<CPlayer*> playerList;

	virtual bool WorkInit() override final;
	virtual bool HandleMessage(CPacket* message, ULONG64 id) override final;
	virtual bool FrameLogic() override final;

	bool HandleLoginMessage(CPacket* message, ULONG64 id);
};

