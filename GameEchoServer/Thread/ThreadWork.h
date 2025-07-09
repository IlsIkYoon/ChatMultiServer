#pragma once
#include "GameEchoServerResource.h"


struct ThreadJob
{
	BYTE jobType;
	ULONG64 id;
};


class Work
{
public:
	virtual bool WorkInit() = 0;
	virtual bool HandleMessage(CPacket* message, ULONG64 id) = 0;
	virtual bool FrameLogic() = 0;
};

