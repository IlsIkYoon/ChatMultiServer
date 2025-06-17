#pragma once
#include "Resource/LoginServerResource.h"


class CRedisConnector
{
public:
	cpp_redis::client redisClient;
	std::string redisIP;
	std::size_t redisPort;

	CRedisConnector();
	CRedisConnector(std::string pIP, std::size_t pPort);

	bool SetToken(char* token, ULONG64 characterKey);
	bool SetToken(std::string token, ULONG64 characterKey);
	bool CheckToken(std::string Key, std::string Token);
};

CRedisConnector& GetRedisConnector();

#define TLS_REDIS_CONNECTOR GetRedisConnector()