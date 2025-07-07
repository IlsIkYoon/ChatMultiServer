#pragma once

#include "pch.h"
#include "TLSPool.h"
#include "ntPacketDefine.h"

#define HEADER_SIZE 5

extern unsigned long long g_CPacketReleaseCount;
extern unsigned long long g_CPacketAllocCount;

class CPacket
{
	enum en_packet
	{
		eBuffer_default = 1400
	};

public:
	enum class ErrorCode
	{
		SUCCESS = 0, 
		INCOMPLETE_DATA_PACKET,
		INVALID_DATA_PACKET
	};

public:
	int _front;
	int _rear;
	int _bufsize;
	long _usageCount;
	bool _encodeflag;
	long _headerFlag;
	char* _buf;

#ifdef __LOGDEBUG__
	long debug_clearCount;
	long debug_initCount;
#endif


	char* GetHeaderPtr();


public:
	static thread_local TMemoryPool<CPacket> cLocalPool;

	CPacket();
	CPacket(int bufize);
	~CPacket();

	static CPacket* Alloc()
	{
		CPacket* retNode = (CPacket*)cLocalPool.Alloc();
		retNode->Init();

		InterlockedIncrement(&g_CPacketAllocCount);

		return retNode;
	}

	static void ReleaseCPacket(CPacket* target)
	{

		target->Clear();
		cLocalPool.Delete(target);
		InterlockedDecrement(&g_CPacketAllocCount);

	}
	
	void IncrementUseCount();
	void DecrementUseCount();
	//------------------------------------------
	//소멸자 대신 호출되는 함수 
	//------------------------------------------
	void Clear();
	//------------------------------------------
	//생성자 대신 호출되는 함수 
	//------------------------------------------
	void Init();

	//------------------------------------------
	//남아있는 버퍼의 공간을 반환 
	//------------------------------------------
	int GetBufferSize();
	
	//------------------------------------------
	//들어있는 데이터의 양을 반환
	//------------------------------------------
	int GetDataSize();

	//------------------------------------------
	// 데이터를 넣을 곳을 반환해줌 
	//------------------------------------------
	char* GetBufferPtr();
	//------------------------------------------
	// 데이터가 들어있는 부분을 반환해줌
	//------------------------------------------
	char* GetDataPtr();
	int MoveFront(int iSize);
	int MoveRear(int iSize);

	
	bool PutData(char* data, int size);

	//-----------------------------------------
	// front를 0으로 옮기고 거기에 헤더를 넣어주는 함수
	//-----------------------------------------
	bool PutHeader(ClientHeader cHeader);

	//-----------------------------------------
	// front를 3으로 옮기고 거기에 헤더를 넣어주는 함수
	//-----------------------------------------
	bool PutHeader(ServerHeader sHeader);

	//-----------------------------------------
	// Front에서 데이터 뽑아주는 함수
	//-----------------------------------------
	bool PopFrontData(int size, char* out);


	//--------------------------------------------
	// 직렬화 버퍼에 헤더를 넣고 암호화 해주는 함수.
	// 클라이언트 대상이라 5바이트 규격 헤더
	// 이미 Encode 되었다면 return false
	//--------------------------------------------
	bool _ClientEncodePacket();



	//--------------------------------------------
	// 암호화 된 직렬화 버퍼를 복호화 해주는 함수.
	// 클라이언트 대상이라 5바이트 규격 헤더
	// 체크섬 체크 후 값이 다르면 return false -> 연결 종료 로직으로 유도
	//--------------------------------------------
	int _ClientDecodePacket();

	//--------------------------------------------
	// Lan통신에서는 2바이트 길이로만 통신하기 때문에 길이를 넣어주는 함수
	//--------------------------------------------
	bool InsertLen(unsigned short pLen);


	//연산자 오버로딩//---------------------------------------
	template<typename type>
		requires std::is_arithmetic_v<type>
	CPacket& operator<<(type iValue) {
		
		(type&)_buf[_rear] = iValue;
		_rear += sizeof(iValue);

		return *this;
	}

	template<typename type>
		requires std::is_arithmetic_v<type>
	CPacket& operator>>(type& iValue) {

		iValue = (type&)_buf[_front];
		_front += sizeof(iValue);

		return *this;
	}

	/*
	CPacket& operator<<(int iValue)
	{
		memcpy(&_buf[_rear], &iValue, sizeof(iValue));
		_rear += sizeof(iValue);

		return *this;
	}
	CPacket& operator<<(float fValue)
	{
		memcpy(&_buf[_rear], &fValue, sizeof(fValue));
		_rear += sizeof(fValue);

		return *this;
	}
	CPacket& operator<<(char cValue)
	{
		memcpy(&_buf[_rear], &cValue, sizeof(cValue));
		_rear += sizeof(cValue);

		return *this;
	}
	CPacket& operator<<(double iValue)
	{
		memcpy(&_buf[_rear], &iValue, sizeof(iValue));
		_rear += sizeof(iValue);

		return *this;
	}
	CPacket& operator<<(__int64 iValue)
	{
		memcpy(&_buf[_rear], &iValue, sizeof(iValue));
		_rear += sizeof(iValue);

		return *this;
	}
	CPacket& operator<<(unsigned char iValue)
	{
		memcpy(&_buf[_rear], &iValue, sizeof(iValue));
		_rear += sizeof(iValue);

		return *this;
	}

	CPacket& operator<<(short iValue)
	{
		memcpy(&_buf[_rear], &iValue, sizeof(iValue));
		_rear += sizeof(iValue);

		return *this;
	}
	CPacket& operator<<(unsigned short iValue)
	{
		memcpy(&_buf[_rear], &iValue, sizeof(iValue));
		_rear += sizeof(iValue);

		return *this;
	}
	CPacket& operator<<(long iValue)
	{
		memcpy(&_buf[_rear], &iValue, sizeof(iValue));
		_rear += sizeof(iValue);

		return *this;
	}

	CPacket& operator>>(int& Value)
	{
		memcpy(&Value, &_buf[_front], sizeof(Value));
		_front += sizeof(Value);

		return *this;
	}
	CPacket& operator>>(double& Value)
	{
		memcpy(&Value, &_buf[_front], sizeof(Value));
		_front += sizeof(Value);

		return *this;
	}
	CPacket& operator>>(char& Value)
	{
		memcpy(&Value, &_buf[_front], sizeof(Value));
		_front += sizeof(Value);

		return *this;
	}
	CPacket& operator>>(float& Value)
	{
		memcpy(&Value, &_buf[_front], sizeof(Value));
		_front += sizeof(Value);

		return *this;
	}
	CPacket& operator>>(unsigned char& Value)
	{
		memcpy(&Value, &_buf[_front], sizeof(Value));
		_front += sizeof(Value);

		return *this;
	}
	CPacket& operator>>(short& Value)
	{
		memcpy(&Value, &_buf[_front], sizeof(Value));
		_front += sizeof(Value);

		return *this;
	}
	CPacket& operator>>(unsigned short& Value)
	{
		memcpy(&Value, &_buf[_front], sizeof(Value));
		_front += sizeof(Value);

		return *this;
	}
	CPacket& operator>>(unsigned int& Value)
	{
		memcpy(&Value, &_buf[_front], sizeof(Value));
		_front += sizeof(Value);

		return *this;
	}
	CPacket& operator>>(__int64& Value)
	{
		memcpy(&Value, &_buf[_front], sizeof(Value));
		_front += sizeof(Value);

		return *this;
	}
	*/

};

