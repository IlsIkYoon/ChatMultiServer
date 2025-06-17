#include "RedisConnector.h"


bool CRedisConnector::SetToken(char* token, ULONG64 characterKey)
{

	std::string redisKey;
	std::string redisToken;
	
	redisKey = std::to_string(characterKey);
	redisToken = token;

	redisClient.set(redisKey, redisToken);

	redisClient.sync_commit();

	return true;
}

bool CRedisConnector::SetToken(std::string token, ULONG64 characterKey)
{

	std::string redisKey;
	std::string redisToken;

	redisKey = std::to_string(characterKey);
	redisToken = token;

	redisClient.set(redisKey, redisToken);

	redisClient.sync_commit();

	return true;
}

CRedisConnector::CRedisConnector()
{
	redisClient.connect(); // 루프백 연결
}

CRedisConnector::CRedisConnector(std::string pIP, std::size_t pPort)
{
	redisIP = pIP;
	redisPort = pPort;

	redisClient.connect(redisIP, redisPort, NULL, NULL, NULL, NULL);
}