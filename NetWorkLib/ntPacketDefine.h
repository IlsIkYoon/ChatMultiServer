#pragma once

#include "pch.h"

//-------------------------------------------------------
// 네트워크 프로토콜 헤더
//-------------------------------------------------------

#define NETWORK_PROTOCOL_CODE 0x89
#define STICKYKEY 0xa9

#define PAYLOAD_MAX 500


struct ServerHeader
{
#pragma pack(push, 1)
	WORD len;
#pragma pack (pop)
};

struct ClientHeader //5바이트 짜리 헤더
{
#pragma pack(push, 1)
	unsigned char _code;
	unsigned short _len;
	unsigned char _randKey;
	unsigned char _checkSum;
#pragma pack(pop)
};