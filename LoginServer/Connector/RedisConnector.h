#pragma once
#include "Resource/LoginServerResource.h"


class CRedisConnector
{

public:

	bool SetToken(CPacket* message, ULONG64 characterKey);
};