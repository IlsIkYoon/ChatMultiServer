#pragma once
#include "GameEchoServerResource.h"
#include "Thread/ThreadWork.h"
#include "Player/Player.h"


class CGameThreadWork :public Work
{
public:
	std::list<CPlayer*> playerList;
	CPlayerManager* playerManager;

	unsigned int workPlayerCount;

	unsigned long frame;

	CGameThreadWork();

	virtual bool WorkInit() override final;
	virtual bool HandleMessage(CPacket* message, ULONG64 id) override final;
	virtual bool FrameLogic() override final;

	virtual void OnCreateSession(ULONG64 ID) override final;
	virtual void OnDeleteSession(ULONG64 ID) override final;

	void SendGameResLogin(ULONG64 ID);
	void HandleReqEchoMessage(CPacket* message, ULONG64 ID);
};