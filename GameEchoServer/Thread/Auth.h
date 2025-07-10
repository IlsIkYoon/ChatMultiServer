#pragma once
#include "GameEchoServerResource.h"
#include "Thread/ThreadWork.h"
#include "Player/Player.h"


class CAuthThreadWork : public Work
{
public:
	std::list<CPlayer*> playerList;
	CPlayerManager* playerManager;

	unsigned int workPlayerCount;
	unsigned long frame;

	CAuthThreadWork();

	virtual bool WorkInit() override final;
	virtual bool HandleMessage(CPacket* message, ULONG64 id) override final;
	virtual bool FrameLogic() override final;

	virtual void OnCreateSession(ULONG64 ID) override final;
	virtual void OnDeleteSession(ULONG64 ID) override final;

	bool HandleLoginMessage(CPacket* message, ULONG64 id);
};

