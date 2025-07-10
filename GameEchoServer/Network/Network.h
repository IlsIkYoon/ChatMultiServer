#pragma once
#include "GameEchoServerResource.h"
#include "Player/Player.h"

class CWanServer : public CWanManager
{
public:
	CPlayerManager* playerManager;

	unsigned int acceptCount;
	unsigned int recvCount;
	unsigned int sendCount;
	
	CWanServer();

	virtual void _OnMessage(CPacket* message, ULONG64 ID) override final;
	virtual void _OnAccept(ULONG64 ID) override final;
	virtual void _OnDisConnect(ULONG64 ID) override final;
	virtual void _OnSend(ULONG64 ID) override final;


	
};

struct ContentsMessage
{
	ULONG64 ID;
	CPacket* message;
};


