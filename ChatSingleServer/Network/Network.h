#pragma once
#include "Contents/ContentsResource.h"


class CWanServer : public CWanManager
{

public:
	CWanServer()
	{

	}

	void _OnMessage(CPacket* message, ULONG64 ID) override final;
	void _OnAccept(ULONG64 ID) override final;
	void _OnDisConnect(ULONG64 ID) override final;
	void _OnSend(ULONG64 ID) override final;

};
