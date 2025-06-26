#pragma once
#include "Resource/MonitoringServerResource.h"


class CLanServer : public CLanManger
{
	virtual void _OnMessage(CPacket* SBuf, ULONG64 ID) override;
	virtual void _OnAccept(ULONG64 ID) override;
	virtual void _OnDisConnect(ULONG64 ID) override;
	virtual void _OnSend(ULONG64 ID) override;





};