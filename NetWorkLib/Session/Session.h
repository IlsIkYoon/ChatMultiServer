#pragma once
#include "pch.h"
#include "Buffer/LFreeQ.h"
#include "Buffer/RingBuffer.h"
#include "Buffer/SerializeBuf.h"

extern unsigned long long g_SessionTotalCreateCount;
extern unsigned long long g_LoginSessionCount;

//todo// SendFlag 수정하기
enum class SendFlag
{
	OFF_SendFlag = 0x00,
	On_SendFlag = 0x01,
	ON_EnqueFlag = 0x02,


};

struct Session
{

	enum class Status
	{
		Active = 0, MarkForDeletion 
	};


	struct IDByte
	{
#pragma pack(push, 1)
		WORD _word;
		DWORD _dword;
#pragma pack(pop)
	};

	struct IdIndexStruct
	{
#pragma pack(push, 1)
		IDByte _id;
		unsigned short _idex;
#pragma pack(pop)
	};


	union SessionIndexUnion
	{
		IdIndexStruct _struct;
		ULONG64 _ulong64;

		ULONG64 GetID();
		void SetID(ULONG64 ID);
		unsigned short GetIndex();
		void SetIndex(unsigned short index);
	};

	union SessionReleaseIOCount
	{
		struct FlagIostruct
		{
			long releaseFlag;
			long ioCount;

		};

		long* GetReleaseFlagPtr();
		long* GetIoCountPtr();

		long long _all;
		FlagIostruct _struct;
	};



	SOCKET _socket;

	SessionIndexUnion _ID;
	SessionReleaseIOCount _releaseIOFlag;

	SOCKADDR_IN _clientAddr;
	long _sendFlag;

	LFreeQ<CPacket*> _sendBuffer;
	CPacket* _recvBuffer;


	OVERLAPPED _sendOverLapped;
	OVERLAPPED _recvOverLapped;

	long _status;
	unsigned long sendData;
	unsigned long sendCount;


	Session();
	~Session();
	//----------------------------------
	// todo// Session 소멸자에 준하는 함수
	//----------------------------------
	void clear();
	//----------------------------------
	// Session 생성자에 준하는 함수
	//----------------------------------
	void init();
	//----------------------------------
	// 리시브 버퍼를 새로운 직렬화 버퍼로 갈아끼워 주는 함수
	//----------------------------------
	void SwapRecvBuffer();
};

class SessionManager
{
	//매니저에서 할 일 
	// todo//SessionList 배열 개수 , Config파일 읽어서 동적할당 해주는 식으로 변경
	//
	Session* _sessionList;
	int _sessionMaxCount;
	LFreeStack<unsigned short> _indexStack;

	ULONG64 _sessionCount;
	ULONG64 _sessionID;


	//----------------------------------
	// IndexPool에 1~20000까지의 숫자를 넣어주는 함수
	//----------------------------------
	void _InitIndexStack();




public:

	SessionManager(int sessionCount);
	bool _makeNewSession(unsigned short* outIDex, SOCKET* newSocket, SOCKADDR_IN* clientAddr);


	//---------------------------------
	// 배열 접근으로 리스트 접근 가능
	// 현재 포인터로 동적할당 중이라서 불가능 
	//---------------------------------
	Session& operator[](int idex);


	//---------------------------------
	// 지금 포인터로 동적할당 중이라서 배열 오버로딩 접근이 불가능함
	// 그래서 필요한 Getter...
	//---------------------------------
	Session& GetSession(int idex);

	//----------------------------------
	// 세션 delete함수
	//----------------------------------
	void Delete(unsigned short iDex);

	

};

