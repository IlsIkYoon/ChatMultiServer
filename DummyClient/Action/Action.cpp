#include "resource.h"
#include "Action.h"
#include "Thread/DummyThread.h"
#include "Msg/RandStringManager.h"
#include "Network/ThreadNetworkManager.h"

extern SOCKADDR_IN serverAddr;
extern char g_currentAction;

extern CRandStringManager g_RandStringManager;

extern char g_currentAction;
extern DWORD g_threadCount;
extern long* g_actionCompleteCount;
extern std::mutex actionCompleteLock;

extern thread_local DWORD tls_myThreadIndex;


//--------------------------------------------------
// 출력용 변수들
//--------------------------------------------------
extern unsigned long long g_moveStartPacket;
extern unsigned long long g_moveStopPacket;
extern unsigned long long g_recvdMoveStopPacket;

unsigned long long g_sendchatMsg;
unsigned long long g_recvdChatCompleteMsg;

unsigned long long g_sessionLoginCount;
unsigned long long g_sessionLogoutCount;

bool HandleConnectAction(DummySession* mySessionArr, DWORD mySessionCount)
{
	bool allConnected = true;
	u_long nonBlockingMode = 1;

	for (unsigned int i = 0; i < mySessionCount; i++)
	{
		if (mySessionArr[i].dummyStatus == static_cast<long>(DummySession::DummyStatus::Created))
		{
			DoSessionConnect(&mySessionArr[i]);
		}	

		if (mySessionArr[i].dummyStatus < static_cast<long>(DummySession::DummyStatus::Connected))
		{
			allConnected = false;
		}
	}

	if (allConnected == true)
	{
		

		bool completeRetval;
		completeRetval = ThreadActionComplete();

		if (completeRetval == true)
		{
			
			ChangeAction(static_cast<char>(Action::Status::ACTION_MOVE));
		}

		return true;
	}



	return false;
}


bool HandleMoveAction(DummySession* mySessionArr, DWORD mySessionCount, DWORD fixedDeltaTime)
{
	bool allActionComplete = true;

	for (unsigned int i = 0; i < mySessionCount; i++)
	{
		if (mySessionArr[i].dummyStatus < static_cast<long>(DummySession::DummyStatus::Connected))
		{
			continue;
		}

		switch(mySessionArr[i].dummyStatus)
		{
		case static_cast<char>(DummySession::DummyStatus::Connected):
			SendMoveStartMessage(&mySessionArr[i]);
			mySessionArr[i].dummyStatus = static_cast<char>(DummySession::DummyStatus::SendingMove);
			allActionComplete = false;
			break;

		case static_cast<char>(DummySession::DummyStatus::SendingMove): //moveStart를 보낸 상태
			break;

		default:
			__debugbreak();
			break;
		}
	}

	if (allActionComplete == true)
	{
		bool completeRetval;
		completeRetval = ThreadActionComplete();

		if (completeRetval == true)
		{
			ChangeAction(static_cast<char>(Action::Status::ACTION_MOVESTOP));
		}

		return true;
	}





	return false;
}

bool HandleMoveStopAction(DummySession* mySessionArr, DWORD mySessionCount)
{
	bool allActionComplete = true;

	for (unsigned int i = 0; i < mySessionCount; i++)
	{
		if (mySessionArr[i].dummyStatus < static_cast<long>(DummySession::DummyStatus::Connected))
		{
			continue;
		}

		switch (mySessionArr[i].dummyStatus)
		{
		case static_cast<char>(DummySession::DummyStatus::SendingMove):
		{
			SendMoveStopMessage(&mySessionArr[i]);
			SendHeartBeatMessage(&mySessionArr[i]);
			mySessionArr[i].dummyStatus = static_cast<char>(DummySession::DummyStatus::WaitingMoveEnd);
			allActionComplete = false;
		}
			break;

		case static_cast<char>(DummySession::DummyStatus::WaitingMoveEnd): //moveStart를 보낸 상태
			allActionComplete = false;
			break;

		case static_cast<char>(DummySession::DummyStatus::RecvdMoveEnd):
			break;


		default:
			__debugbreak();
			break;
		}
	}


	if (allActionComplete == true)
	{
		bool completeRetval;
		completeRetval = ThreadActionComplete();

		if (completeRetval == true)
		{
			ChangeAction(static_cast<char>(Action::Status::ACTION_SENDCHATMSG));
		}

		return true;
	}



	return false;
}

bool HandleCheckMsgAction(DummySession* mySessionArr, DWORD mySessionCount)
{
	bool allRecvd = true;;
	for (unsigned int i = 0; i < mySessionCount; i++)
	{
		if (mySessionArr[i].dummyStatus < static_cast<long>(DummySession::DummyStatus::SendingChat))
		{
			continue;
		}

		if (mySessionArr[i].allMsgRecvd == false)
		{
			allRecvd = false;
			break;
		}
	}

	if (allRecvd == true)
	{
		//todo//disconnect 없이 계속 돌게 할 목적으로 넣은 로직
		for (unsigned int i = 0; i < mySessionCount; i++)
		{
			if (mySessionArr[i].dummyStatus < static_cast<long>(DummySession::DummyStatus::Connected))
			{
				continue;
			}

			mySessionArr[i].dummyStatus = static_cast<long>(DummySession::DummyStatus::Connected);

		}


		bool completeRetval;
		completeRetval = ThreadActionComplete();
		if (completeRetval == true)
		{
			ChangeAction(static_cast<char>(Action::Status::ACTION_CHECKDISCONNECT));
		}

		return true;
	}

	return false;
}

bool HandleSendChatMsgAction(DummySession* mySessionArr, DWORD mySessionCount)
{

	bool allActionComplete = true;

	for (unsigned int i = 0; i < mySessionCount; i++)
	{
		if (mySessionArr[i].dummyStatus < static_cast<long>(DummySession::DummyStatus::RecvdMoveEnd))
		{
			continue;
		}

		//todo//확률 체크 로직 필요
		if (mySessionArr[i].dummyStatus == static_cast<long>(DummySession::DummyStatus::RecvdMoveEnd))
		{
			char* sendMsg;
			int sendLen;
			g_RandStringManager.GetRandString(&sendMsg, &sendLen);
			mySessionArr[i].dummyStatus = static_cast<long>(DummySession::DummyStatus::SendingChat);

			SendLocalChatMessage(&mySessionArr[i], sendMsg, sendLen);
			InterlockedIncrement(&g_sendchatMsg);
			CheckReceiversInSector(&mySessionArr[i], sendMsg, sendLen);

		}
		else if (mySessionArr[i].dummyStatus == static_cast<long>(DummySession::DummyStatus::SendingChat))
		{
			mySessionArr[i].dummyStatus = static_cast<long>(DummySession::DummyStatus::WaitingChatEnd);
			SendChatEndMessage(&mySessionArr[i]);
		}


		if (mySessionArr[i].dummyStatus != static_cast<long>(DummySession::DummyStatus::RecvdChatComplete))
		{
			allActionComplete = false;
		}



	}

	if (allActionComplete == true)
	{
		
		bool completeRetval;
		completeRetval = ThreadActionComplete();

		if (completeRetval == true)
		{
			ChangeAction(static_cast<char>(Action::Status::ACTION_CHECKCHATMSG));
		}

		return true;


	}
	

	return false;
}


bool HandleCheckMoveStopAction(DummySession* mySessionArr, DWORD mySessionCount)
{
	bool allActionComplete = true;

	for (unsigned int i = 0; i < mySessionCount; i++)
	{
		if (mySessionArr[i].dummyStatus == static_cast<long>(DummySession::DummyStatus::SendingMove))
		{
			allActionComplete = false;
			break;
		}
		//todo//얼마나 안 왔는지 체크 해야하나 ? 
	}


	return allActionComplete;
}

bool HandleCheckDisconnectAction(DummySession* mySessionArr, DWORD mySessionCount)
{
	for (unsigned int i = 0; i < mySessionCount; i++)
	{
		if (mySessionArr[i].dummyStatus < static_cast<long>(DummySession::DummyStatus::Connected))
		{
			continue;
		}

		if (rand() % 20 == 0)
		{
			mySessionArr[i].Dummy_Clear();
			InterlockedIncrement(&g_sessionLogoutCount);
			InterlockedDecrement(&g_sessionLoginCount);
		}
	}

	bool completeRetval;
	completeRetval = ThreadActionComplete();

	if (completeRetval == true)
	{
		ChangeAction(static_cast<char>(Action::Status::ACTION_CONNECT));
	}

	return true;
}


bool ChangeAction(char action)
{
	g_currentAction = action;

	actionCompleteLock.lock();

	//두개가 순차적으로 같은 것에 대해 들어올 수 있으므로
	//먼저 들어와서 한 애가 있으면 다음 애가 나갈 수 있어야 함 
	if (g_actionCompleteCount[tls_myThreadIndex] != THREAD_ACTION_COMPLETE)
	{
		actionCompleteLock.unlock();
		return false;
	}
	

	for (unsigned int i = 0; i < g_threadCount; i++)
	{
		g_actionCompleteCount[i] = 0;
	}
	actionCompleteLock.unlock();

	return true;
}




bool DoSessionConnect(DummySession* session)
{
	int connectRetval;
	//커넥트 로직 실행
	session->_socket = socket(AF_INET, SOCK_STREAM, NULL);

	connectRetval = connect(session->_socket, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
	if (connectRetval == SOCKET_ERROR)
	{
		//블로킹 소켓이므로 getlastError코드 확인이 필요
		if (GetLastError() == 10061)
		{
			closesocket(session->_socket);
			return false;
		}
		else if (GetLastError() == 10054)
		{
			__debugbreak(); //여기서 이 값이 나오는지 확인이 필요.
		}
		__debugbreak();
	}



	CThreadNetworkManager::SetSocketOption(&session->_socket);

	session->dummyStatus = static_cast<long>(DummySession::DummyStatus::Connecting);


	return true;
}