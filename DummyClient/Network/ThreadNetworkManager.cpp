#include "ThreadNetworkManager.h"
extern DWORD g_threadCount;
extern DWORD g_ThreadIndex;
extern DWORD g_clientCount;

extern DummySession* g_DummySessionArr;

extern char g_currentAction;
extern long* g_actionCompleteCount;

extern std::list<Session*> g_Sector[dfRANGE_MOVE_RIGHT / SECTOR_RATIO][dfRANGE_MOVE_BOTTOM / SECTOR_RATIO];

extern DWORD tls_myThreadIndex;

CThreadNetworkManager::CThreadNetworkManager()
{
	SetMySessionCount();
	mySessionArr = &g_DummySessionArr[SESSION_PER_THREAD * (dwMyIndex - 1)];

	SetMyFDSET();

	g_actionCompleteCount[tls_myThreadIndex] = THREAD_ACTION_INCOMPLETE;

	for (unsigned int i = 0; i < dwMySessionCount; i++)
	{
		mySessionArr[i].dummyStatus = static_cast<long>(DummySession::DummyStatus::Created);
		mySessionArr[i]._created = true;
	}

}

bool CThreadNetworkManager::SetMySessionCount()
{
	dwMyIndex = InterlockedIncrement(&g_ThreadIndex);
	tls_myThreadIndex = dwMyIndex - 1;
	if (dwMyIndex == g_threadCount)
	{
		bLastThread = true;
		dwMySessionCount = g_clientCount % SESSION_PER_THREAD;
		if (dwMySessionCount == 0)
		{
			dwMySessionCount = SESSION_PER_THREAD;
		}


	}
	else {
		bLastThread = false;
		dwMySessionCount = SESSION_PER_THREAD;
	}

	return bLastThread;
}


bool CThreadNetworkManager::SetMyFDSET()
{
	DWORD myThreadSessionCount;
	if (bLastThread == false)
	{
		fdSetCount = SESSION_PER_THREAD / 64;
		myFdReadSet = new fd_set[fdSetCount];
		myFdWriteSet = new fd_set[fdSetCount];
	}
	else
	{
		//세션 카운트에 맞게 생성
		myThreadSessionCount = g_clientCount % SESSION_PER_THREAD;

		if (myThreadSessionCount == 0)
		{
			fdSetCount = SESSION_PER_THREAD / 64;
		}
		else {
		fdSetCount = myThreadSessionCount / 64;
		}


		if (myThreadSessionCount % 64 != 0)
		{
			fdSetCount++;
		}

		myFdReadSet = new fd_set[fdSetCount];
		myFdWriteSet = new fd_set[fdSetCount];

	}

	return true;
}


bool CThreadNetworkManager::HandleNetworkEvent()
{
	ZeroFdSet();
	RegistFdSet();
	DoSelectFunc();



	return true;
}

bool CThreadNetworkManager::ZeroFdSet()
{
	for (DWORD i = 0; i < fdSetCount; i++)
	{
		FD_ZERO(&myFdReadSet[i]);
		FD_ZERO(&myFdWriteSet[i]);
	}
	return true;
}

bool CThreadNetworkManager::RegistFdSet()
{
	for (DWORD i = 0; i < dwMySessionCount; i++)
	{
		if (mySessionArr[i].dummyStatus < static_cast<long>(DummySession::DummyStatus::Connecting)) //Connecting 이하의 레벨이면 스킵
		{
			continue;
		}

		DWORD setIndex = i / 64;
		if (setIndex >= fdSetCount) {
			__debugbreak(); // 확인용
		}


		FD_SET(mySessionArr[i]._socket, &myFdReadSet[i / 64]);
		FD_SET(mySessionArr[i]._socket, &myFdWriteSet[i / 64]);
	}
	return true;
}

bool CThreadNetworkManager::DoSelectFunc()
{
	int select_retval;
	TIMEVAL timeout = { 0, 0 };

	for (DWORD i = 0; i < fdSetCount; i++)
	{
		select_retval = select(NULL, &myFdReadSet[i], &myFdWriteSet[i], NULL, &timeout);
		if (select_retval == SOCKET_ERROR && GetLastError() != WSAEINVAL)
		{
			//selete 에러에 대한 처리 필요
			__debugbreak();
			continue;
		}
	}

	for (DWORD i = 0; i < dwMySessionCount; i++)
	{
		if (mySessionArr[i].dummyStatus < static_cast<long>(DummySession::DummyStatus::Connecting)) //Connecting 이하의 레벨이면 스킵
		{
			continue;
		}

		if (FD_ISSET(mySessionArr[i]._socket, &myFdReadSet[i / 64]))
		{
			//readSet 처리
			HandleReadSelect(&mySessionArr[i]);
		}

		if (FD_ISSET(mySessionArr[i]._socket, &myFdWriteSet[i / 64]))
		{
			HandleWriteSelect(&mySessionArr[i]);
		}
	}



	return true;
}



bool CThreadNetworkManager::HandleReadSelect(DummySession* _session)
{
	DWORD recv_retval;

	recv_retval = recv(_session->_socket, _session->_recvQ.GetRear(),
		_session->_recvQ.GetDirectEnqueSize(), NULL);

	if (recv_retval == 0)
	{
		//DeleteSession(_session);
	}

	else if (recv_retval == SOCKET_ERROR && GetLastError() != WSAEWOULDBLOCK)
	{

		if (GetLastError() == 10054 || GetLastError() == 10053)
		{

			//DeleteSession 요청
			//DeleteSession(_session);
		}
		else {
			__debugbreak();

		}
	}

	else if (recv_retval != SOCKET_ERROR)
	{
		_session->_recvQ.MoveRear(recv_retval);
	}
	else {
		if (GetLastError() != WSAEWOULDBLOCK)
		{
			__debugbreak();
		}
	}
	return true;

}
bool CThreadNetworkManager::HandleWriteSelect(DummySession* _session)
{
	DWORD send_retval;
	CPacket sendBuf;
	bool retval;
	char* localBuf;
	unsigned int dequeResult;
	//세션 sendQ에 대해 이 쓰레드에서만 접근한다고 가정
	if (_session->_sendQ.IsEmpty() == true)
	{
		return false;
	}


	int sendSize = _session->_sendQ.GetSizeUsed();
	localBuf = (char*)malloc(sizeof(char) * sendSize);
	_session->SendBufferDeque(localBuf, sendSize, &dequeResult);

	retval = sendBuf.PutData(localBuf, sendSize);
	if (retval == false)
	{
		__debugbreak();
	}

	sendBuf._ClientEncodePacket();

	send_retval = send(_session->_socket, sendBuf.GetDataPtr(), sendBuf.GetDataSize(), NULL);


	if (send_retval == SOCKET_ERROR && GetLastError() != WSAEWOULDBLOCK)
	{
		if (GetLastError() != 10054 && GetLastError() != 10053) {
			//delete요청 필요
			__debugbreak();
		}
	}

	free(localBuf);


	return true;

}


unsigned short CThreadNetworkManager::GetIndex(ULONG64 target)
{
	ULONG64 temp;
	unsigned short ret;

	temp = target & 0xffff000000000000;
	temp = temp >> 48;
	ret = (unsigned short)temp;

	return ret;
}

bool CThreadNetworkManager::SetSocketOption(SOCKET* socket)
{
	LINGER ling;

	ling.l_linger = 0;
	ling.l_onoff = 1;


	if (setsockopt(*socket, SOL_SOCKET, SO_LINGER, (const char*)&ling, sizeof(ling)) == SOCKET_ERROR)
	{
		printf("Lingeroption Error : %d\n", GetLastError());
		__debugbreak();
	}




	return true;
}