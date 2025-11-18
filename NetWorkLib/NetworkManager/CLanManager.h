#pragma once
#include "pch.h"

#include "Buffer/RingBuffer.h"
#include "Buffer/SerializeBuf.h"
#include "Log/Log.h"
#include "Buffer/LFreeQ.h"
#include "Log/Profiler.h"
#include "Session/Session.h"
#include "Network.h"

//-----------------------------------------------
// 클라이언트 통신 전용 네트워크 매니저
//-----------------------------------------------
class CLanManager : public CNetwork
{
public:


	//--------------------------------------------
	//network Session 관리를 위한 변수
	//--------------------------------------------
	SOCKADDR_IN _serverAddr;

	unsigned short _portNum;

	CSessionManager* _sessionList;
	unsigned long _sendInProgress;

	int _sessionMaxCount;
	long _sessionLoginCount;



	//--------------------------------------------
	// IOCP 변수
	//--------------------------------------------
	int _concurrentCount;
	int _workerThreadCount;
	HANDLE _hIOCP;


	//---------------------------------------------
	//쓰레드 관리를 위한 변수
	//---------------------------------------------
	std::thread* _workerThreadArr;
	std::thread _acceptThread;

	HANDLE _exitThreadEvent;


	//---------------------------------------------
	// 로그 관리를 하는 매니저
	// 내부에서 로그 쓰레드 생성 및 큐에 있는 로그를 파일에 출력하는 역할
	//---------------------------------------------
	static LogManager _log;


	SOCKET _listenSocket;




public:
	CLanManager();
	~CLanManager();

private:

	bool _NetworkInit();
	void _MakeNetWorkMainThread();
	//--------------------------------------------
	// 워커 쓰레드 생성 후 
	// 블로킹 _listensocket을 이용해 accept만을 실행하는 함수
	// accept가 성공하면  _sessionCount++;
	// accept가 성공해도 키 값이 이미 내부에 있었다면 접속 실패로 간주
	// 이후에 OnAccept 핸들러 함수 호출
	//--------------------------------------------
	void AcceptThread();

	void IOCP_WorkerThread();
	//--------------------------------------------
	// WsaRecv함수 래핑. 오류 코드에 대한 예외처리, 분산 버퍼 등의 행동 진행
	//--------------------------------------------
	bool _RecvPost(Session* _session);
	//--------------------------------------------
	// WsaSend함수 래핑. 오류 코드에 대한 예외처리, 분산 버퍼 등의 행동 진행
	// 들어있는 데이터 전부 Send
	//--------------------------------------------
	bool _SendPost(Session* _session);

	bool _TrySendPost(Session* _session);
	//--------------------------------------------
	// 세션리스트의 sendInProgressFlag를 확인하고 Flag가 꺼져 있으면 들어가서 배열을 돌면서 SendPost를 호출해주는 함수
	//--------------------------------------------
	bool SendToAllSessions();
	//--------------------------------------------
	// Session의 recvBuf(rBuf)에서 네트워크 헤더 길이에 맞는 페이로드를 뽑아서
	// sBuf에 삽입하고 리턴.
	//--------------------------------------------
	bool _DequePacket(CPacket* sBuf, Session* _session);
	//--------------------------------------------
	// _session의 recv Buffer에 있는 데이터를 전부 뺀 뒤 _packet을 _packetSize만큼
	// 삽입하고 뺀 데이터를 다시 버퍼에 넣어줌
	//--------------------------------------------
	void _RecvBufRestorePacket(Session* _session, char* _packet, int _packetSize);
	//--------------------------------------------
	// 세션을 실제로 삭제하는 함수
	// close socket, 세션 delete, list에서 erase작업 수행
	//--------------------------------------------
	void _DisconnectSession(ULONG64 sessionID);
	void _DisconnectSession(Session* _session);

	//--------------------------------------------
	// 에러라면 예외 처리를 내부에서 함
	// 에러 발생시 true 반환
	//--------------------------------------------
	bool CheckGQCSError(bool retval, DWORD* recvdbytes, ULONG_PTR recvdkey, OVERLAPPED* overlapped, DWORD errorno);

	void InitSessionList(int SessionCount);
	//--------------------------------------------
	// CancelIo를 호출 시켜서 정리 루틴을 타게 만드는 함수
	//--------------------------------------------
	bool RequestSessionAbort(ULONG64 playerID);

	virtual bool IncrementSessionIoCount(Session* _session) override final;
	//--------------------------------------------
	// IOCount가 0이면 삭제 후 false 반환
	//--------------------------------------------
	virtual bool DecrementSessionIoCount(Session* _session) override final;

	bool SendCompletionRoutine(Session* _session);
	bool RecvCompletionRoutine(Session* _session);

public:
	/////////////컨텐츠 제공 함수들///////////////////////////////
	bool Start();
	int GetSessionMaxCount();
	//---------------------------------------------
	// ULONG64 변수에서 ID값에 해당하는 6바이트만 추출하는 함수
	//---------------------------------------------
	static ULONG64 GetID(ULONG64 target);
	//---------------------------------------------
	// ULONG64 변수에서 Index값에 해당하는 2바이트만 추출하는 함수
	//---------------------------------------------
	static unsigned short GetIndex(ULONG64 target);
	//---------------------------------------------
	// 컨텐츠에서 다른 세션에 메세지를 보낼때 쓸 함수
	//---------------------------------------------
	bool SendPacket(ULONG64 playerId, CPacket* buf);
	//---------------------------------------------
	// 컨텐츠에서 세션 삭제 요청 함수
	//---------------------------------------------
	bool DisconnectSession(ULONG64 sessionID);

	void EnqueLog(const char* string);
	void EnqueLog(std::string& string);
	//--------------------------------------------
	// 핸들러 함수
	// 가상함수로 상속받아서 정의해서 사용
	//--------------------------------------------
	virtual void _OnMessage(CPacket* SBuf, ULONG64 ID) = 0;
	virtual void _OnAccept(ULONG64 ID) = 0;
	virtual void _OnDisConnect(ULONG64 ID) = 0;
	virtual void _OnSend(ULONG64 ID) = 0;

	void ExitNetWorkManager();
	//--------------------------------------------
	// NetWorkLibrary에 PQCS로 SendAllSession을 요청하는 함수
	//--------------------------------------------
	void EnqueSendRequest();
	//--------------------------------------------
	// 모든 세션 연결 종료(CancelIo호출) 시키는 함수
	//--------------------------------------------
	void DisconnectAllSessions();

	void RegistConcurrentCount(int pCount);
	void RegistSessionMaxCoiunt(int pCount);
	void RegistWorkerThreadCount(int pCount);
	void RegistPortNum(int pPortNum);

	bool ConnectServer(std::wstring ip, unsigned short portNum, ULONG64* outSessionID);

};

