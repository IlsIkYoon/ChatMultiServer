#pragma once
#include "Resource/LoginServerResource.h"


class CRedisConnector
{
	cpp_redis::client redisClient;


public:
	CRedisConnector();

	bool SetToken(CPacket* message, ULONG64 characterKey);
};