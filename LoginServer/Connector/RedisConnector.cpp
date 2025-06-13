#include "RedisConnector.h"


bool CRedisConnector::SetToken(CPacket* message, ULONG64 characterKey)
{
	
	//todo//Redis연동 후엔 여기에 토큰 넣는 과정 필요

	//char Token[64]

	//MakeRandToken

	//RedisConnector->SetToken(chracterkey, Token)

	//SetToken(message, 


	return true;
}

CRedisConnector::CRedisConnector()
{
	redisClient.connect(); // 루프백 연결
}