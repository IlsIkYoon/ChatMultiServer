#include "resource.h"
#include "DummyClient.h"
#include "Thread/DummyThread.h"
#include "Player/DummySession.h"
#include "Action/Action.h"
#include "Thread/TrustDummyThread.h"
//--------------------------------
// InputData
//--------------------------------
WCHAR g_serverIp[20];
WCHAR g_portNum[10];
DWORD g_clientCount;
DWORD g_disconnectOption;
DWORD g_trustMode;
extern unsigned long long g_trustMsgProbability;

SOCKADDR_IN serverAddr;


DWORD g_threadCount;
DWORD g_ThreadIndex;
HANDLE* g_thread;

extern DWORD g_clientCount;
extern char g_currentAction;
extern DummySession* g_DummySessionArr;

extern long* g_actionCompleteCount;




//---------------------------------------------
// 출력용 변수들
//---------------------------------------------
extern unsigned long long g_moveStartPacket;
extern unsigned long long g_moveStopPacket;
extern unsigned long long g_recvdMoveStopPacket;

extern unsigned long long g_sendchatMsg;
extern unsigned long long g_recvdChatCompleteMsg;
extern unsigned long long g_sendLocalChatMsgTotal;
extern unsigned long long g_recvLocalChatMsgTotal;

extern unsigned long long g_sessionLoginCount;
extern unsigned long long g_sessionLogoutCount;

bool DummyClient()
{
	bool bExitFlag;
	DWORD dwStartTime;
	DWORD dwEndTime;
	DWORD dwElapsedTime;

	NetWorkInit();

	CheckInPut();

	DummyInit();

	while (1)
	{
		dwStartTime = timeGetTime();

		bExitFlag = MonitoringServerStatus();
		if (bExitFlag == true)
		{
			break;
		}
		dwEndTime = timeGetTime();
		dwElapsedTime = dwEndTime - dwStartTime;
		if (dwElapsedTime < 1000)
		{
			Sleep(1000 - dwElapsedTime);
		}
	}

	DummyClear();

	NetWorkClear();

	return true;
}


bool NetWorkInit()
{
	WSADATA wsa;
	int retval;


	retval = WSAStartup(WINSOCK_VERSION, &wsa);
	if (retval == false)
	{
		return false;
	}

	return true;
}

bool NetWorkClear()
{

	WSACleanup();

	return true;
}

bool CheckInPut()
{
	int InetPtonWresult;
	serverAddr.sin_family = AF_INET;

	wprintf(L"Server IP : ");
	wscanf_s(L"%s", g_serverIp, 19);
	InetPtonWresult = InetPtonW(AF_INET, g_serverIp, &serverAddr.sin_addr);
	if (InetPtonWresult != 1)
	{
		printf("ip 입력 오류\n");
		__debugbreak();
	}

	wprintf(L"Port Num : ");
	wscanf_s(L"%s", g_portNum, 9);
	int portNum;
	portNum = _wtoi(g_portNum);
	serverAddr.sin_port = htons(portNum);

	wprintf(L"Client Count : ");
	wscanf_s(L"%d", &g_clientCount);

	wprintf(L"Disconnect Option (0 = NoDisconnect / 1 = Disconnect) : \n");
	wscanf_s(L"%d", &g_disconnectOption);

	wprintf(L"Trust Option (0 = No Trust / 1 = Trust) : \n");
	wscanf_s(L"%d", &g_trustMode);

	if (g_trustMode != TRUST_MODE)
	{
		wprintf(L"SendMessage Probabillity (1 : the Least Probability) : \n");
		wscanf_s(L"%lld", &g_trustMsgProbability);
		if (g_trustMsgProbability == 0) {
			__debugbreak();
		}
	}



	return true;
}



bool DummyInit()
{
	InitThread();

	//쓰레드 생성
	//




	return true;
}


bool InitThread()
{
	DWORD threadCount;

	g_DummySessionArr = new DummySession[g_clientCount];

	threadCount = g_clientCount / 640;
	if (g_clientCount % 640 != 0)
	{
		threadCount++;
	}
	
	g_threadCount = threadCount;

	g_actionCompleteCount = new long[g_threadCount];

	g_thread = new HANDLE[g_threadCount];

	if (g_trustMode == TRUST_MODE)
	{
		for (unsigned int i = 0; i < g_threadCount; i++)
		{
			g_thread[i] = (HANDLE)_beginthreadex(NULL, 0, DummyClientThreadFunc, NULL, NULL, NULL);
		}
	}
	else
	{
		for (unsigned int i = 0; i < g_threadCount; i++)
		{
			g_thread[i] = (HANDLE)_beginthreadex(NULL, 0, DummyTrustThreadFunc, NULL, NULL, NULL);
		}
	}


	return true;
}



bool DummyClear()
{
	//더미 삭제 로직



	return true;
}


bool MonitoringServerStatus()
{
	if (_kbhit())
	{
		char input;
		input = _getch();
		if (input == 'q' || input == 'Q')
		{
			return true;
		}
	}

	PrintCreatedClient();
	PrintCurrentAction();
	PrintMsgCount();
	


	return false;
}


bool PrintCreatedClient()
{
	int createdCount = 0;
	for (unsigned int i = 0; i < g_clientCount; i++)
	{
		if (g_DummySessionArr[i]._created == true)
		{
			createdCount++;
		}
	}
	printf("--------------------------------------------------------------------------\n");
	printf("Input Session Count : %d || Login Session Count : %lld || Logout Session Count : %lld\n", 
		g_clientCount, g_sessionLoginCount, g_sessionLogoutCount);
	printf("--------------------------------------------------------------------------\n");
	return true;
}

bool PrintCurrentAction()
{
	printf("Current Action : ");
	switch (g_currentAction)
	{
	case static_cast<char>(Action::Status::ACTION_CONNECT):
		printf("ACTION_CONNECT\n");
		break;

	case static_cast<char>(Action::Status::ACTION_SENDCHATMSG):
		printf("ACTION_SENDCHATMSG\n");
		break;
	case static_cast<char>(Action::Status::ACTION_CHECKCHATMSG):
		printf("ACTION_CHECKCHATMSG\n");
		break;
	case static_cast<char>(Action::Status::ACTION_MOVE):
		printf("ACTION_MOVE\n");
		break;
	case static_cast<char>(Action::Status::ACTION_MOVESTOP):
		printf("ACTION_MOVESTOP\n");
		break;
	case static_cast<char>(Action::Status::ACTION_CHECKMOVESTOPMSG):
		printf("ACTION_CHECKMOVESTOPMSG\n");
		break;
	case static_cast<char>(Action::Status::ACTION_CHECKDISCONNECT):
		printf("ACTION_CHECKDISCONNECT\n");
		break;

	default:
		__debugbreak();
		break;
	}

	printf("--------------------------------------------------------------------------\n");
	return true;
}


bool PrintMsgCount()
{
	printf("MoveStart : %lld || MoveStop : %lld || MoveStopComplete : %lld\n", g_moveStartPacket, g_moveStopPacket, g_recvdMoveStopPacket);
	printf("SendChatMsg : %lld, RecvdChatComplete : %lld\n", g_sendchatMsg, g_recvdChatCompleteMsg);
	printf("SendLocalChatMsgTotal : %lld || RecvLocalChatMsgTotal : %lld\n", g_sendLocalChatMsgTotal, g_recvLocalChatMsgTotal);
	printf("--------------------------------------------------------------------------\n");

	return true;
}