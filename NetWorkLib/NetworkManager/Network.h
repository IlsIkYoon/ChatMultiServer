#pragma once
#include "pch.h"
#include "Session/Session.h"

class CNetwork
{
public:
	virtual bool IncrementSessionIoCount(Session* _session) = 0;
	virtual bool DecrementSessionIoCount(Session* _session) = 0;

	virtual bool SendPacket(ULONG64 playerId, CPacket* buf) = 0;
	virtual bool DisconnectSession(ULONG64 sessionID) = 0;
	virtual void EnqueLog(std::string& string) = 0;
};