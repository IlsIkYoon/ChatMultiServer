#pragma once
#include "Resource/LoginServerResource.h"


class CRedisConnector
{
	cpp_redis::client redisClient;
	std::string redisIP;
	std::size_t redisPort;

public:
	CRedisConnector();
	CRedisConnector(std::string pIP, std::size_t pPort);

	bool SetToken(char* token, ULONG64 characterKey);
	bool SetToken(std::string token, ULONG64 characterKey);
};