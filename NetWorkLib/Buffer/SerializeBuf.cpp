#include "pch.h"
#include "SerializeBuf.h"


unsigned long long g_CPacketCreateCount;
unsigned long long g_CPacketDeleteCount;
unsigned long long g_CPacketReleaseCount;
unsigned long long g_CPacketAllocCount;

CPacket::CPacket()
{
	_usageCount = 1;
	_front = HEADER_SIZE;
	_rear = HEADER_SIZE;
	_encodeflag = false;
	_bufsize = eBuffer_default;
	_buf = (char*)malloc(eBuffer_default);
	unsigned long long retval;
	retval = InterlockedIncrement(&g_CPacketCreateCount);



	_headerFlag = 0;
#ifdef __LOGDEBUG__
	debug_clearCount = 0;
	debug_initCount = 0;
#endif

}
CPacket::CPacket(int bufsize)
{
	_usageCount = 1;
	_headerFlag = 0;
	_front = HEADER_SIZE;
	_rear = HEADER_SIZE;
	_encodeflag = false;
	_bufsize = bufsize;
	_buf = (char*)malloc(bufsize);
	InterlockedIncrement(&g_CPacketCreateCount);
#ifdef __LOGDEBUG__
	debug_clearCount = 0;
	debug_initCount = 0;
#endif
}
CPacket::~CPacket()
{
	free(_buf);
	InterlockedIncrement(&g_CPacketDeleteCount);
}

void CPacket::Clear()
{
	_front = HEADER_SIZE;
	_rear = HEADER_SIZE;
#ifdef __LOGDEBUG__
	InterlockedIncrement(&debug_clearCount);
#endif
	
}
void CPacket::Init()
{
	_usageCount = 1;

#ifdef __LOGDEBUG__
	InterlockedIncrement(&debug_initCount);
#endif

	_encodeflag = false;
	_headerFlag = 0;


}
int CPacket::GetBufferSize()
{
	return _bufsize - _rear;
}
int CPacket::GetDataSize()
{
	return _rear - _front;
}
char* CPacket::GetBufferPtr()
{
	return &_buf[_rear];
}

char* CPacket::GetDataPtr()
{
	return &_buf[_front];
}

int CPacket::MoveFront(int iSize)
{
	_front += iSize;

	return _front;
}
int CPacket::MoveRear(int iSize)
{
	_rear += iSize;

	return _rear;
}

bool CPacket::PutData(char* data, int size)
{
	if (GetBufferSize() < size) return false;

	if (size == 0) return false;

	memcpy(&_buf[_rear], data, size);
	_rear += size;

	return true;
}


bool CPacket::PopFrontData(int size, char* out)
{

	if (size > GetDataSize())
	{
		__debugbreak();
		return false;
	}
	memcpy(out, &_buf[_front], size);
	MoveFront(size);

	return true;
}

void CPacket::IncrementUseCount()
{
	long retval;
	retval = _InterlockedIncrement(&_usageCount);


}
void CPacket::DecrementUseCount()
{
	DWORD retval = _InterlockedDecrement(&_usageCount);

	if (retval == 0)
	{
		ReleaseCPacket(this);	
	}
	else if (retval < 0)
		__debugbreak(); //완료통지를 두번으로 착각했을 가능성 있음
}

char* CPacket::GetHeaderPtr()
{
	return &_buf[0];
}

bool CPacket::PutHeader(ServerHeader sHeader)
{
	if (InterlockedExchange(&_headerFlag, 1) != 0)
	{
		return false;
	}

	if (_front != 5 || _usageCount == 0)
	{
		__debugbreak();
	}
	_front = _front - sizeof(sHeader);
	if (_front < 0)
	{
		__debugbreak();
		return false;
	}
	memcpy(&_buf[_front], &sHeader, sizeof(sHeader));

	return true;
}


bool CPacket::PutHeader(ClientHeader cHeader)
{
	if (InterlockedExchange(&_headerFlag, 1) != 0)
	{
		return false;
	}
	_front = _front - sizeof(cHeader);
	if (_front < 0)
	{
		__debugbreak();
		return false;
	}
	memcpy(&_buf[_front], &cHeader, sizeof(cHeader));

	return true;
}


//------------------------------------------------------------------------------


bool CPacket::_ClientEncodePacket()
{
	if (_encodeflag == true)
	{
		//이미 이 패킷에 대해 인코딩을 했다면 리턴
		return false;
	}
	//--------------------------------------------------
	//헤더 먼저 만들어주기 작업
	//--------------------------------------------------
	unsigned char localRandkey;

	ClientHeader pHeader;
	unsigned char* dataPtr;
	int sum = 0;
	
	localRandkey = (unsigned char)rand();

	pHeader._code = NETWORK_PROTOCOL_CODE;
	pHeader._randKey = localRandkey;
	pHeader._len = GetDataSize();
	dataPtr = (unsigned char*)GetDataPtr();

	for (int i = 0; i < pHeader._len; i++)
	{
		sum += dataPtr[i];
	}
	pHeader._checkSum = (unsigned char)(sum % 256);
	
	PutHeader(pHeader);

	//--------------------------------------------------
	//패킷 인코딩 작업
	//--------------------------------------------------

	unsigned char* encodePtr;
	int encodeSize = pHeader._len + 1; //CheckSum + Data 

	unsigned char bufSave1 = 0;
	unsigned char bufSave2 = 0;



	encodePtr = &((ClientHeader*)GetDataPtr())->_checkSum;

	for (int i = 0; i < encodeSize; i++)
	{
		encodePtr[i] = encodePtr[i] ^ (localRandkey + i + 1 + bufSave1);
		bufSave1 = encodePtr[i];
		encodePtr[i] = encodePtr[i] ^ (STICKYKEY + i + 1 + bufSave2);
		bufSave2 = encodePtr[i];
	}


	_encodeflag = true;

	return true;
}

int CPacket::_ClientDecodePacket()
{

	//--------------------------------------------------
	//패킷 디코딩 작업
	//--------------------------------------------------

	unsigned char* decodePtr;
	int decodeSize;
	int payloadSize;

	unsigned char localRandkey;
	int localDataSize;

	unsigned char bufSave1 = 0;
	unsigned char bufSave2 = 0;

	unsigned char checkSum;
	long long sum = 0;

	unsigned char bufSaveBefore = 0;
	unsigned char valueBefore = 0;

	localRandkey = ((ClientHeader*)GetDataPtr())->_randKey;
	decodePtr = &((ClientHeader*)GetDataPtr())->_checkSum;
	payloadSize = ((ClientHeader*)GetDataPtr())->_len;
	decodeSize = payloadSize + 1;

	localDataSize = GetDataSize();

	//--------------------------------------------------
	// 헤더에 대한 예외처리 작업
	//--------------------------------------------------

	if (sizeof(ClientHeader) > localDataSize)
	{
		//헤더도 다 안온 상황
		return static_cast<int>(ErrorCode::INCOMPLETE_DATA_PACKET);
	}

	if (payloadSize + sizeof(ClientHeader) > localDataSize)
	{
		//페이로드가 아직 다 안 온 상황
		return static_cast<int>(ErrorCode::INCOMPLETE_DATA_PACKET);
	}
	if (payloadSize > PAYLOAD_MAX)
	{
		return static_cast<int>(ErrorCode::INVALID_DATA_PACKET);
	}



	//--------------------------------------------------
	// 복호화 작업
	//--------------------------------------------------

	for (int i = 0; i < decodeSize; i++)
	{

		bufSave1 = decodePtr[i] ^ (valueBefore + STICKYKEY + i + 1);

		bufSave2 = bufSave1 ^ (bufSaveBefore + localRandkey + i + 1);

		valueBefore = decodePtr[i];
		bufSaveBefore = bufSave1;
		decodePtr[i] = bufSave2;

	}

	checkSum = *decodePtr;

	for (int i = 1; i < payloadSize + 1; i++)
	{
		sum += decodePtr[i];
	}

	sum = sum % 256;

	if (checkSum == sum)
	{
		return static_cast<int>(ErrorCode::SUCCESS);
	}


	//todo//체크섬 오류에 대한 로그 남기고 리턴 (연결 끊게 됨)

	__debugbreak();
	return static_cast<int>(ErrorCode::INVALID_DATA_PACKET);
}

//--------------------------------------------------------------------

thread_local TMemoryPool<CPacket> CPacket::cLocalPool;