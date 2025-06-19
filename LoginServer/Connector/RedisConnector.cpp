#include "RedisConnector.h"

std::mutex redisLock;


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
	std::lock_guard guard(redisLock);
	redisClient.connect(); // 루프백 연결
}

CRedisConnector::CRedisConnector(std::string pIP, std::size_t pPort)
{
	redisIP = pIP;
	redisPort = pPort;

	redisClient.connect(redisIP, redisPort, NULL, NULL, NULL, NULL);
}


CRedisConnector& GetRedisConnector()
{
	static thread_local CRedisConnector instance;
	return instance;
}

bool CRedisConnector::CheckToken(std::string Key, std::string Token)
{

	std::promise<bool> retval;
	std::future<bool> future = retval.get_future();

	redisClient.get(Key, [&retval, Key ,Token, this](cpp_redis::reply pReply) {
		if (pReply.is_null() == true || Token != pReply.as_string())
		{
			retval.set_value(false);
			return;
		}

		retval.set_value(true);

		std::vector<std::string> delParam;
		delParam.push_back(Key);

		this->redisClient.del(delParam);

		});
	redisClient.sync_commit();

	return future.get();
}