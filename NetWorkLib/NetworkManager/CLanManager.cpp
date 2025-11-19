#include "pch.h"
#include "CLanManager.h"
#include "Parser/TextParser.h"
#include "ntPacketDefine.h"
#include "Buffer/LFreeQ.h"
#include "NetWorkManager.h"

#define DEFAULT_LAN_SESSIONCOUNT 128
#define DEFAULT_LAN_WORKERTHREAD 1
#define DEFAULT_LAN_PORTNUM 15053


#ifdef __LOGDEBUG__
extern CRITICAL_SECTION g_Lock;
#endif


unsigned long g_LanThreadIndex;

CLogManager CLanManager::_log;

CLanManager::CLanManager()
{
	_exitThreadEvent = CreateEvent(NULL, true, false, NULL);

	_sendInProgress = 0;
	_concurrentCount = DEFAULT_LAN_WORKERTHREAD;
	_hIOCP = NULL;
	_listenSocket = NULL;
	ZeroMemory(&_serverAddr, sizeof(_serverAddr));
	_portNum = DEFAULT_LAN_PORTNUM;
	_sessionLoginCount = 0;
	_workerThreadCount = DEFAULT_LAN_WORKERTHREAD;
	_sessionMaxCount = DEFAULT_LAN_SESSIONCOUNT;

	_workerThreadArr = nullptr;
	_sessionList = nullptr;
}

CLanManager::~CLanManager()
{
	closesocket(_listenSocket); //더이상의 접속을 막는다

	//모든 세션을 내보내고 기다림
	DisconnectAllSessions();

	while (1)
	{
		if (_sessionLoginCount == 0)
		{
			break;
		}
		Sleep(0);
	}

	EnqueLog("All Session Log Out");

	//네트워크 쓰레드를 모두 종료 시킨다.
	SetEvent(_exitThreadEvent);


	//쓰레드 종료에 대한 대기 작업



	//리턴






	//NetWorkClear();
	WSACleanup();

}


bool CLanManager::_NetworkInit()
{

	int wsa_retval;
	int bind_retval;
	int listen_retval;

	WSAData wsa;


	LINGER ling;

	ling.l_linger = 0;
	ling.l_onoff = 1;

	_serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	_serverAddr.sin_family = AF_INET;
	_serverAddr.sin_port = htons(_portNum);


	wsa_retval = WSAStartup(WINSOCK_VERSION, &wsa);
	if (wsa_retval != 0)
	{
		printf("Wsa Startup Error : %d\n", GetLastError());
		_log.EnqueLog("Wsa Startup Error", 0, __FILE__, __func__, __LINE__, GetLastError());
		return false;
	}
	_log.EnqueLog("Wsa Startup ... success");
	_listenSocket = socket(AF_INET, SOCK_STREAM, NULL);
	if (_listenSocket == INVALID_SOCKET)
	{
		printf("Socket Init error : %d\n", GetLastError());
		_log.EnqueLog("Socket Init Error", 0, __FILE__, __func__, __LINE__, GetLastError());
		return false;
	}
	_log.EnqueLog("Socket Init ... success");
	int zero = 0;

	if (setsockopt(_listenSocket, SOL_SOCKET, SO_SNDBUF, (const char*)&zero, sizeof(zero)) == SOCKET_ERROR)
	{
		printf("ZeroBuf Error : %d\n", GetLastError());
		_log.EnqueLog("ZeroBuf option ... failed\n");
	}
	_log.EnqueLog("Send ZeroBuf Option ... success");

	if (setsockopt(_listenSocket, SOL_SOCKET, SO_LINGER, (const char*)&ling, sizeof(ling)) == SOCKET_ERROR)
	{
		printf("Lingeroption Error : %d\n", GetLastError());
		_log.EnqueLog("Linger option ... failed\n");
	}
	_log.EnqueLog("Socket Linger Option ... success");



	bind_retval = bind(_listenSocket, (SOCKADDR*)&_serverAddr, sizeof(_serverAddr));
	if (bind_retval != 0)
	{
		printf("bind Error : %d\n", GetLastError());
		_log.EnqueLog("Bind Error", 0, __FILE__, __func__, __LINE__, GetLastError());
		return false;
	}
	_log.EnqueLog("bind... success");


	listen_retval = listen(_listenSocket, SOMAXCONN_HINT(65536));
	if (listen_retval != 0)
	{
		printf("listen Error : %d\n", GetLastError());
		_log.EnqueLog("Listen Error", 0, __FILE__, __func__, __LINE__, GetLastError());
		return false;
	}

	_log.EnqueLog("Socket Listen...success");

	//소켓 초기화 완료

	_hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, _concurrentCount);

	if (_hIOCP == NULL)
	{
		printf("createIoCompletionPort Failed : %d\n", GetLastError());
		_log.EnqueLog("CreateIoCompletionPort fails to Create", 0, __FILE__, __func__, __LINE__, GetLastError());
		return false;
	}

	_log.EnqueLog("NetWorkInit success");

#ifdef __LOGDEBUG__
	InitializeCriticalSection(&g_Lock);
#endif
	return true;
}



void CLanManager::_MakeNetWorkMainThread()
{
	_acceptThread = std::thread([this]() { this->AcceptThread(); });
	_log.EnqueLog("Accept Thread Made Success");
}

void CLanManager::AcceptThread()
{

	_log.EnqueLog("Accept Thread Wake up");

	_workerThreadArr = new std::thread[_workerThreadCount];

	for (int i = 0; i < _workerThreadCount; i++)
	{
		_workerThreadArr[i] = std::thread(&CLanManager::IOCP_WorkerThread, this);
	}




	Session* currentSession;
	SOCKET newSocket;
	SOCKADDR_IN clientAddr;
	int addrLen = sizeof(clientAddr);
	unsigned short currentIdex;

	while (1)
	{
		newSocket = 0;
		ZeroMemory(&clientAddr, sizeof(clientAddr));

		newSocket = accept(_listenSocket, (SOCKADDR*)&clientAddr, &addrLen);

		if (newSocket == INVALID_SOCKET)
		{
			if (GetLastError() == WSAENOTSOCK)
			{
				_log.EnqueLog("AcceptThread Exit Logic Start");
				break;
			}

			_log.EnqueLog("Accept Error", 0, __FILE__, __func__, __LINE__, GetLastError());
			continue;
		}

		if (_sessionLoginCount == _sessionMaxCount)
		{
			_log.EnqueLog("Session OverFLow\n");
			closesocket(newSocket);
			continue;
		}


		_sessionList->_makeNewSession(&currentIdex, &newSocket, &clientAddr);
		currentSession = &_sessionList->GetSession(currentIdex);

		if (InterlockedIncrement(&_sessionLoginCount) > _sessionMaxCount)
		{
			__debugbreak();
		}

		_OnAccept(_sessionList->GetSession(currentIdex)._ID._ulong64);

		CreateIoCompletionPort((HANDLE)currentSession->_socket, _hIOCP,
			(ULONG_PTR)currentSession, 0);

		_RecvPost(currentSession);

		DecrementSessionIoCount(currentSession);

	}
	//AcceptThread 종료 로직 
	WaitForSingleObject(_exitThreadEvent, INFINITE);
	_log.EnqueLog("exitThreadEvent Signaled !!!");


}



thread_local DWORD t_LanMyIndex;



void CLanManager::IOCP_WorkerThread()
{
	_log.EnqueLog("IOCP_WORKER Thread Made Success");

#
	DWORD recvdBytes;
	ULONG_PTR recvdKey;
	OVERLAPPED* recvdOverLapped;

	bool GQCS_ret;
	bool errorCheck;
	Session* _session;
	bool completionRetval;

	t_LanMyIndex = InterlockedIncrement(&g_LanThreadIndex) - 1;


	while (1)
	{
		GQCS_ret = GetQueuedCompletionStatus(_hIOCP, &recvdBytes, &recvdKey, &recvdOverLapped, INFINITE);

		errorCheck = CheckGQCSError(GQCS_ret, &recvdBytes, recvdKey, recvdOverLapped, GetLastError());

		if (errorCheck == true)
		{
			continue;
		}

		_session = nullptr;
		_session = (Session*)recvdKey;

		if ((ULONG_PTR)recvdOverLapped == (ULONG_PTR)&_session->_recvOverLapped) //Recv에 대한 완료 통지
		{

			_session->_recvBuffer->MoveRear(recvdBytes);

			completionRetval = RecvCompletionRoutine(_session);

			if (completionRetval == false)
			{
				continue;
			}
		}

		else if ((ULONG_PTR)recvdOverLapped == (ULONG_PTR)&_session->_sendOverLapped) //Send에 대한 완료 통지
		{
			SendCompletionRoutine(_session);
		}

		else if (recvdOverLapped == SENDREQUEST)
		{
			SendToAllSessions();
			continue;
		}

		else
		{
			__debugbreak();
		}

		DecrementSessionIoCount(_session);

	}
}



bool CLanManager::_RecvPost(Session* _session)
{
	WSABUF localBuf;

	localBuf.buf = _session->_recvBuffer->GetBufferPtr();
	localBuf.len = _session->_recvBuffer->GetBufferSize();

	int recvRet;
	DWORD flag = 0;


	if (localBuf.len == 0)
		return false;

	IncrementSessionIoCount(_session);
	ZeroMemory(&_session->_recvOverLapped, sizeof(_session->_recvOverLapped));
	recvRet = WSARecv(_session->_socket, &localBuf, 1, NULL, &flag, &_session->_recvOverLapped, NULL);

	if (recvRet != 0 && GetLastError() != WSA_IO_PENDING)
	{

		if (GetLastError() == 10054 || GetLastError() == 10053)
		{
			RequestSessionAbort(_session->_ID._ulong64);
			DecrementSessionIoCount(_session);
		}
		else
		{
			printf("GetLastError : %d\n", GetLastError());
			_log.EnqueLog("WsaRecvError", _session->_ID.GetID(), __FILE__, __func__, __LINE__, GetLastError());
			__debugbreak();
		}
	}

	return true;
}
bool CLanManager::_SendPost(Session* _session)
{
	if (_session->_sendBuffer.GetSize() == 0)
	{
		__debugbreak();
		return false;
	}

	IncrementSessionIoCount(_session);

	LFreeQ<CPacket*>::Node* targetNode;
	CPacket* targetBuf;
	int sendRet;


	unsigned long long sendSize = _session->_sendBuffer.GetSize();

	WSABUF* buf = new WSABUF[sendSize];

	targetNode = _session->_sendBuffer.PeekFront();


	for (int i = 0; i < sendSize; i++)
	{
		targetBuf = (CPacket*)targetNode->_data;
		if (targetNode == nullptr)
			__debugbreak();

		//sendq에서 순회하면서 버퍼에 넣어 줌 
		buf[i].buf = targetBuf->GetDataPtr();
		buf[i].len = targetBuf->GetDataSize();
		targetNode = targetNode->_next;

		if (buf[i].len == 0)
		{
			__debugbreak();
		}


		_session->sendData += buf[i].len;
		_session->sendCount++;
	}

	if (_session->sendCount > 1000)
	{
		__debugbreak();
		DisconnectSession(_session->_ID._ulong64);
		delete[] buf;
		return false;
	}

	ZeroMemory(&_session->_sendOverLapped, sizeof(_session->_sendOverLapped));
	OVERLAPPED localOverlapped = _session->_sendOverLapped;
	SOCKET localSocket = _session->_socket;

	{
		sendRet = WSASend(localSocket, buf, (DWORD)sendSize, NULL, NULL, &_session->_sendOverLapped, NULL);
	}
	DWORD errCode = GetLastError();
	if (sendRet != 0 && errCode != WSA_IO_PENDING)
	{
		if (errCode == 10054 || errCode == 10053 || errCode == 10004)
		{
			std::string error;
			error = "WSA Send Error || ErrorCode : ";
			error += std::to_string(errCode);
			error += "session ID : ";
			error += std::to_string(_session->_ID._ulong64);
			EnqueLog(error);
			RequestSessionAbort(_session->_ID._ulong64);
		}
		else {
			printf("GetLastError : %d\n", GetLastError());
			__debugbreak();
		}
	}

	delete[] buf;

	return true;
}



bool CLanManager::_DequePacket(CPacket* sBuf, Session* _session)
{
	unsigned short LanHeader;
	CPacket* targetBuf = _session->_recvBuffer;



	if (targetBuf->GetDataSize() < sizeof(LanHeader))
	{
		return false;
	}

	targetBuf->PopFrontData(sizeof(LanHeader), (char*)&LanHeader);

	if (targetBuf->GetDataSize() < LanHeader)
	{
		//다시 데이터 넣기 
		_RecvBufRestorePacket(_session, (char*)&LanHeader, sizeof(LanHeader));
		return false;
	}

	targetBuf->PopFrontData(LanHeader, sBuf->GetBufferPtr());
	sBuf->MoveRear(LanHeader);

	return true;
}


void CLanManager::_RecvBufRestorePacket(Session* _session, char* _packet, int _packetSize)
{

	int size = _session->_recvBuffer->GetDataSize();
	char* localBuf = (char*)malloc(size);

	_session->_recvBuffer->PopFrontData(size, localBuf);
	_session->_recvBuffer->PutData(_packet, _packetSize);
	_session->_recvBuffer->PutData(localBuf, size);

	free(localBuf);

	return;
}




bool CLanManager::SendPacket(ULONG64 playerId, CPacket* buf)
{

	CPacket* sendPacket = buf;

	ULONG64 localID = GetID(playerId);
	unsigned short localIndex = GetIndex(playerId);
	Session* _session = &_sessionList->GetSession(localIndex);

	sendPacket->IncrementUseCount(); //패킷을 보낸다는 의미

	//들어오자마자 릴리즈 할 수 없게 ioCount를 올리고 release플래그를 체크해줌 
	IncrementSessionIoCount(_session);
	if (*(_session->_releaseIOFlag.GetReleaseFlagPtr()) == 0x01)
	{
		DecrementSessionIoCount(_session);
		sendPacket->DecrementUseCount();
		return false;
	}

	//ID가 달라졌는지 비교
	ULONG64 CurrentId = _session->_ID._ulong64;
	if (CurrentId != playerId)
	{

		DecrementSessionIoCount(_session);
		sendPacket->DecrementUseCount();
		return false;
	}

	unsigned short msgLen;
	msgLen = sendPacket->GetDataSize();
	sendPacket->InsertLen(msgLen);

	_sessionList->GetSession(localIndex)._sendBuffer.Enqueue(sendPacket);


	DecrementSessionIoCount(_session);


	return true;
}


void CLanManager::_DisconnectSession(ULONG64 sessionID)
{
	unsigned long long currentLoginCount;

	unsigned short localIndex = GetIndex(sessionID);
	ULONG64 localID = GetID(sessionID);

	if (InterlockedCompareExchange64(&_sessionList->GetSession(localIndex)._releaseIOFlag._all, SESSION_DISCONNECTING, SESSION_CLOSABLE)
		!= SESSION_CLOSABLE)
	{
		return;
	}


	_OnDisConnect(sessionID);

	closesocket(_sessionList->GetSession(localIndex)._socket);


	currentLoginCount = InterlockedDecrement(&g_LoginSessionCount);

	_sessionList->Delete(localIndex);


}
void CLanManager::_DisconnectSession(Session* _session)
{
	if (InterlockedCompareExchange64(&_session->_releaseIOFlag._all, SESSION_DISCONNECTING, SESSION_CLOSABLE) != SESSION_CLOSABLE)
	{
		//그 사이에 누가 iocount를 올리고 작업에 들어갔음 ?
		return;
	}

	_OnDisConnect(_session->_ID._ulong64);

	closesocket(_session->_socket);

	_sessionList->Delete(_session->_ID.GetIndex());

}
bool CLanManager::DisconnectSession(ULONG64 playerID)
{
	RequestSessionAbort(playerID);
	return true;
}

bool CLanManager::CheckGQCSError(bool retval, DWORD* recvdbytes, ULONG_PTR recvdkey, OVERLAPPED* overlapped, DWORD errorno)
{
	Session* _session = (Session*)recvdkey;


	if (recvdbytes == 0)
	{
		DecrementSessionIoCount(_session);

		return true;
	}
	if (retval == false && errorno == ERROR_OPERATION_ABORTED) //CancelIoEx 호출됨
	{
		DecrementSessionIoCount(_session);
		return true;
	}
	if (retval == false && errorno == ERROR_SEM_TIMEOUT)
	{
		DecrementSessionIoCount(_session);
		return true;
	}
	if (retval == false && errorno == ERROR_CONNECTION_ABORTED)
	{
		DecrementSessionIoCount(_session);
		return true;
	}

	if (recvdkey == THREAD_EXIT)
	{
		__debugbreak();
	}
	if (recvdkey == NULL)
	{
		__debugbreak();
	}
	if (overlapped == nullptr)
	{
		__debugbreak();
	}

	if (retval == false && errorno != ERROR_NETNAME_DELETED && errorno != ERROR_OPERATION_ABORTED)
	{
		printf("GQCS False : %d\n", errorno);
		_log.EnqueLog("GQCS Error", 0, __FILE__, __func__, __LINE__, errorno);
		__debugbreak();
	}


	return false;
}








ULONG64 CLanManager::GetID(ULONG64 target)
{
	ULONG64 ret;
	ret = target & 0x0000ffffffffffff;

	return ret;
}

unsigned short CLanManager::GetIndex(ULONG64 target)
{
	ULONG64 temp;
	unsigned short ret;

	temp = target & 0xffff000000000000;
	temp = temp >> 48;
	ret = (unsigned short)temp;

	return ret;
}

void CLanManager::EnqueLog(const char* string)
{
	_log.EnqueLog(string);
}


void CLanManager::EnqueLog(std::string& string)
{
	_log.EnqueLog(string);
}


void CLanManager::InitSessionList(int SessionCount)
{
	_sessionList = new CSessionManager(SessionCount);
}


int CLanManager::GetSessionMaxCount()
{
	return _sessionMaxCount;
}


void CLanManager::ExitNetWorkManager()
{
	closesocket(_listenSocket); //더이상의 접속을 막는다

	//모든 세션을 내보내고 기다림
	DisconnectAllSessions();

	while (1)
	{
		if (_sessionLoginCount == 0)
		{
			break;
		}
		Sleep(0);
	}

	//네트워크 쓰레드를 모두 종료 시킨다.
	SetEvent(_exitThreadEvent);

	//쓰레드 종료에 대한 대기 작업

	//리턴



}


bool CLanManager::SendToAllSessions()
{
	Session* targetSession;
	ULONG64 playerId;
	ULONG64 CurrentId;
	long releaseFlag;


	if (InterlockedExchange(&_sendInProgress, static_cast<unsigned long>(SendStatus::InProgress))
		!= static_cast<unsigned long>(SendStatus::Idle))
	{
		return false;
	}


	for (int i = 0; i < _sessionMaxCount; i++)
	{
		targetSession = &_sessionList->GetSession(i);
		playerId = targetSession->_ID._ulong64;

		releaseFlag = targetSession->_releaseIOFlag._struct.releaseFlag;
		if (releaseFlag == 0x01)
		{
			continue;
		}
		//들어오자마자 릴리즈 할 수 없게 ioCount를 올리고 release플래그를 체크해줌 
		IncrementSessionIoCount(targetSession);
		releaseFlag = targetSession->_releaseIOFlag._struct.releaseFlag;
		if (releaseFlag == 0x01)
		{
			DecrementSessionIoCount(targetSession);
			continue;
		}

		//ID가 달라졌는지 비교
		CurrentId = targetSession->_ID._ulong64;
		if (CurrentId != playerId)
		{
			DecrementSessionIoCount(targetSession);
			continue;
		}

		_TrySendPost(targetSession);

		DecrementSessionIoCount(targetSession);

	}


	InterlockedExchange(&_sendInProgress, static_cast<unsigned long>(SendStatus::Idle));

	return true;
}


void CLanManager::EnqueSendRequest()
{
	PostQueuedCompletionStatus(_hIOCP, SENDREQUEST_BYTE, SENDREQUEST_KEY, SENDREQUEST);
}


bool CLanManager::_TrySendPost(Session* _session)
{

	while (1)
	{
		if (_session->_sendBuffer.GetSize() > 0 &&
			InterlockedExchange(&_session->_sendFlag, 1) == 0)
		{
			if (_session->_sendBuffer.GetSize() == 0)
			{
				if (InterlockedExchange(&_session->_sendFlag, 0) != 1)
				{
					__debugbreak();
				}

				continue;
			}

			_SendPost(_session);
		}

		break;
	}

	return true;
}


bool CLanManager::RequestSessionAbort(ULONG64 playerID)
{
	bool CanelIoExResult;
	int localIndex;
	unsigned long long localID;
	Session* _session;

	localIndex = GetIndex(playerID);
	localID = GetID(playerID);
	_session = &_sessionList->GetSession(localIndex);

	IncrementSessionIoCount(_session); //삭제 되지 않게 IOCount올리기
	if (*(_session->_releaseIOFlag.GetReleaseFlagPtr()) == 1)
	{
		DecrementSessionIoCount(_session);
		return false;
	}


	if (_session->_ID._ulong64 != playerID)
	{
		DecrementSessionIoCount(_session);
		return false;
	}

	InterlockedExchange(&_session->_status, static_cast<long>(Session::Status::MarkForDeletion));
	CanelIoExResult = CancelIoEx((HANDLE)_session->_socket, NULL);


	DecrementSessionIoCount(_session);
	return true;
}


void CLanManager::DisconnectAllSessions()
{
	Session* _session;
	for (int i = 0; i < _sessionMaxCount; i++)
	{
		_session = &_sessionList->GetSession(i);

		if (_session->_releaseIOFlag._struct.releaseFlag == 1) // 이때 서버에 남아있을 가능성 ?
		{
			continue;
		}

		RequestSessionAbort(_session->_ID._ulong64);
	}

	EnqueLog("DisConnect All session Clear");

}



bool CLanManager::IncrementSessionIoCount(Session* _session)
{

	InterlockedIncrement(_session->_releaseIOFlag.GetIoCountPtr());


	return true;
}
bool CLanManager::DecrementSessionIoCount(Session* _session)
{

	long retval;

	retval = InterlockedDecrement(_session->_releaseIOFlag.GetIoCountPtr());
	if (retval == 0)
	{
		_DisconnectSession(_session);
		return false;
	}
	//*
	else if (retval < 0)
	{
		__debugbreak();
	}
	//*/

	return true;
}



bool CLanManager::SendCompletionRoutine(Session* _session)
{
	for (unsigned int i = 0; i < _session->sendCount; i++)
	{

		CPacket* retNode;
		retNode = _session->_sendBuffer.Dequeue();

		if (retNode == nullptr) __debugbreak();

		if (retNode->_usageCount == 0) __debugbreak();

		retNode->DecrementUseCount();
	}
	_session->sendCount = 0;
	_session->sendData = 0;

	if (InterlockedExchange(&_session->_sendFlag, 0) == 0)
	{
		__debugbreak();
	}

	if (InterlockedCompareExchange(&_session->_status, static_cast<long>(Session::Status::Active), static_cast<long>(Session::Status::Active))
		== static_cast<long>(Session::Status::MarkForDeletion))
	{
		RequestSessionAbort(_session->_ID._ulong64);
	}


	return true;
}


bool CLanManager::RecvCompletionRoutine(Session* _session)
{
	bool disconnected = false;
	CPacket* SBuf;

	if (_session->_recvBuffer == nullptr)
	{
		__debugbreak();
	}

	while (1)
	{
		SBuf = CPacket::Alloc();

		if (_DequePacket(SBuf, _session) == false)
		{
			if (SBuf->_usageCount != 1)
			{
				__debugbreak();
			}
			SBuf->DecrementUseCount();

			if (_session->_recvBuffer == nullptr)
			{
				__debugbreak();
			}

			break;
		}

		_OnMessage(SBuf, _session->_ID._ulong64);

		if (SBuf->_usageCount != 1)
		{
			__debugbreak();
		}
		SBuf->DecrementUseCount();



	}
	if (disconnected == true)
	{
		return false;
	}

	if (_session->_recvBuffer == nullptr)
	{
		__debugbreak();
	}

	_session->SwapRecvBuffer();

	//리시브 재등록
	if (_session->_status != static_cast<long>(Session::Status::MarkForDeletion))
	{
		_RecvPost(_session);
		if (InterlockedCompareExchange(&_session->_status, static_cast<long>(Session::Status::Active), static_cast<long>(Session::Status::Active))
			== static_cast<long>(Session::Status::MarkForDeletion))
		{
			RequestSessionAbort(_session->_ID._ulong64);
		}
	}


	return true;
}

bool CLanManager::Start()
{
	bool networkInit_retval;

	_log.RegistMyFileName("LanServer");
	_log.InitLogManager();


	InitSessionList(_sessionMaxCount);

	networkInit_retval = _NetworkInit();
	if (networkInit_retval == false)
	{

		_log.EnqueLog("NetWork Init Failed !!!");
		_log.EnqueLog("Server off...");
		__debugbreak();
	}

	_MakeNetWorkMainThread();

	return true;
}


void CLanManager::RegistConcurrentCount(int pCount)
{
	_concurrentCount = pCount;
}
void CLanManager::RegistSessionMaxCoiunt(int pCount)
{
	_sessionMaxCount = pCount;
}
void CLanManager::RegistWorkerThreadCount(int pCount)
{
	_workerThreadCount = pCount;
}
void CLanManager::RegistPortNum(int pPortNum)
{
	_portNum = pPortNum;
}

bool CLanManager::ConnectServer(std::wstring ip, unsigned short portNum, ULONG64* outSessionID)
{
	SOCKET newSocket;
	unsigned short currentIdex;
	SOCKADDR_IN serverAddr;
	Session* currentSession;
	int retval;

	if (InterlockedIncrement(&_sessionLoginCount) >= _sessionMaxCount)
	{
		InterlockedDecrement(&_sessionLoginCount);
		_log.EnqueLog("Session OverFLow\n");
		return false;
	}

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(portNum);
	InetPtonW(AF_INET, ip.c_str(), &serverAddr.sin_addr.S_un.S_addr);
	
	newSocket = socket(AF_INET, SOCK_STREAM, NULL);
	if (newSocket == INVALID_SOCKET)
	{
		__debugbreak();
	}

	retval = connect(newSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
	if (retval != 0)
	{
		__debugbreak();
	}

	_sessionList->_makeNewSession(&currentIdex, &newSocket, &serverAddr);
	(*_sessionList)[currentIdex]._type = static_cast<BYTE>(enSessionType::en_Server);

	currentSession = &_sessionList->GetSession(currentIdex);

	CreateIoCompletionPort((HANDLE)currentSession->_socket, _hIOCP,
		(ULONG_PTR)currentSession, 0);

	_RecvPost(currentSession);

	DecrementSessionIoCount(currentSession);

}