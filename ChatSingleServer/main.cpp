#include "Contents/ContentsResource.h"
#include "Contents/ContentsPacket.h"
#include "Contents/ContentsFunc.h"
#include "Player/Player.h"
#include "Network/Network.h"
#include "Contents/MonitorManager.h"
#include "CommonProtocol.h"

//----------------------------------------
//모니터링 출력을 위한 전역 변수들
//----------------------------------------
extern long long g_playerCount;

extern unsigned long long g_CPacketCreateCount;
extern unsigned long long g_CPacketDeleteCount;
extern unsigned long long g_CPacketReleaseCount;
extern unsigned long long g_CPacketAllocCount;

extern int g_concurrentCount;
extern int g_workerThreadCount;
extern int g_maxSessionCount;

extern unsigned long long g_SessionTotalCreateCount;
extern unsigned long long g_LoginSessionCount;
extern unsigned long long g_LogoutSessionCount;
extern unsigned long long g_HeartBeatOverCount;

extern unsigned long long g_AcceptTps;
extern unsigned long long* g_pRecvTps;
extern unsigned long long* g_pSendTps;

//----------------------------------------
// 메세지 카운트
//----------------------------------------
extern unsigned long long g_MoveStopCompleteCount;
extern unsigned long long g_MoveStartCount;
extern unsigned long long g_MoveStopCount;
extern unsigned long long g_LocalChatCount;
extern unsigned long long g_ChatEndCount;
extern unsigned long long g_DeleteMsgCount;
extern unsigned long long g_CreateMsgCount;
extern unsigned long long g_HeartBeatCount;

//-----------------------------------------
// 플레이어 카운팅을 위한 변수
//-----------------------------------------
extern unsigned long long g_TotalPlayerCreate;
extern unsigned long long g_PlayerLogInCount;
extern unsigned long long g_PlayerLogOut;

extern std::queue<ULONG64> g_WaitingPlayerAcceptQ;


extern DWORD g_frame;


//----------------------------------------
//컨텐츠 쓰레드 핸들
//----------------------------------------
HANDLE g_ContentsThread;

HANDLE g_ExitEvent;


//----------------------------------------
// 출력 함수들
//----------------------------------------
void PrintSendBufferPool();
void PrintSerializePool();
void PrintSessionCount();
void PrintMessageCount();
void PrintConsolMenu();
void PrintfCpuUsage();
void PrintConsolAll();

void UpdateMonitorData();

//----------------------------------------
// 입력 값을 체크. 입력 값에 따라 출력 혹은 종료
//----------------------------------------
void CheckInput();

//----------------------------------------
// ESC를 눌렀을 때 서버 종료 절차가 진행되는 함수
//----------------------------------------
void ExitAllProcess();

CWanServer* pLib;
CCpuUsage CPUUsage;
CMonitorManager g_Monitor;
CPdhManager g_PDH;

int main()
{
	timeBeginPeriod(1);
	
	DWORD startTimeTick;
	DWORD endTimeTick;
	DWORD resultTimeTick;

	procademy::CCrashDump dump;

	int portNum;
	int MonitorPortNum;
	int SessionMaxCount;
	int WorkerThreadCount;
	int ConcurrentCount;

	g_ExitEvent = CreateEvent(NULL, true, false, NULL);

	pLib = new CWanServer;
	g_ContentsThread = (HANDLE)_beginthreadex(NULL, 0, ContentsThreadFunc, NULL, NULL, NULL);

	txParser.GetData("Chat_Single_Config.ini");
	txParser.SearchData("PortNum", &portNum);
	txParser.SearchData("SessionMaxCount", &SessionMaxCount);
	txParser.SearchData("WorkerThreadCount", &WorkerThreadCount);
	txParser.SearchData("ConcurrentCount", &ConcurrentCount);
	txParser.SearchData("MonitorPortNum", &MonitorPortNum);
	txParser.CloseData();

	pLib->RegistPortNum(portNum);
	pLib->RegistSessionMaxCoiunt(SessionMaxCount);
	pLib->RegistWorkerThreadCount(WorkerThreadCount);
	pLib->RegistConcurrentCount(ConcurrentCount);

	pLib->Start();

	g_PDH.RegistEthernetMax(3);
	g_PDH.Start();
	g_Monitor.RegistMonitor(L"127.0.0.1", MonitorPortNum);

	

	while (1)
	{

		startTimeTick = timeGetTime();

		PrintConsolMenu();

		CheckInput();

		UpdateMonitorData();
		
		endTimeTick = timeGetTime();
		resultTimeTick = (endTimeTick - startTimeTick) / 1000;
		Sleep(1000 - resultTimeTick);

	}

	return 0;
}



void PrintSendBufferPool()
{
	

}
void PrintSerializePool()
{
	printf("------------------------------------------\n");
	printf("SerializePool || size : %d\n", CPacket::cLocalPool.GetSize());
	printf("CPacket || CreateCount : %lld || DeleteCount : %lld || AllocCount : %lld || ReleaseCount : %lld\n",
		g_CPacketCreateCount, g_CPacketDeleteCount, g_CPacketAllocCount, g_CPacketReleaseCount);
}
void PrintSessionCount()
{
	printf("------------------------------------------\n");
	printf("Session Create Total : %lld\n", g_SessionTotalCreateCount);
	printf("Session LogIn : %lld\n", g_LoginSessionCount);
	printf("Session LogOut : %lld\n", g_LogoutSessionCount);
	printf("Session HeartBeat Time Over : %lld\n", g_HeartBeatOverCount);
	printf("------------------------------------------\n");
	printf("Player Create Total : %lld\n", g_TotalPlayerCreate);
	printf("Player LogIn : %lld\n", g_PlayerLogInCount);
	printf("Player LogOut : %lld\n", g_PlayerLogOut);
	printf("Waiting Player : %lld\n", g_WaitingPlayerAcceptQ.size());
}


void PrintMessageCount()
{
	printf("------------------------------------------\n");
	printf("MoveStartPacket(Recv) : %lld\n", g_MoveStartCount);
	printf("MoveStopPacket(Recv) : %lld\n", g_MoveStopCount);
	printf("MoveStopCompletePacket(Send) : %lld\n", g_MoveStopCompleteCount);
	printf("LocalChat Msg (Recv) : %lld\n", g_LocalChatCount);
	printf("HeartBeat Msg (Recv) : %lld\n", g_HeartBeatCount);
	printf("ChatEnd Msg (Recv) : %lld\n", g_ChatEndCount);
	printf("CreateMsg (ContentsRecv) : %lld\n", g_CreateMsgCount);
	printf("DeleteMsg (ContentsRecv) : %lld\n", g_DeleteMsgCount);
}


void PrintConsolMenu()
{
	printf("\n");
	printf("Q : DebugBreak() || W : WriteProfile() || E : Exit() \n");
	printf("\n");
	printf("-----------------------------------------------------------------------\n");
	printf("PortNum :%d || MaxSessionCount : %d || WORKERThread Count : %d || ConcurrentCount : %d\n",
		pLib->_portNum, pLib->_sessionMaxCount, pLib->_workerThreadCount, pLib->_concurrentCount);
	printf("-----------------------------------------------------------------------\n");
	return;
}

void CheckInput()
{
	if (_kbhit())
	{
		char c = _getch();
		if (c == 'q' || c == 'Q')
		{
			__debugbreak(); //메모리 Dump를 확인하기 위한 debugBreak();
		}
		if (c == 'w' || c == 'W')
		{
			WriteAllProfileData();
		}
		if (c == 'E' || c == 'e')
		{
			ExitAllProcess();
		}
	}
}

void PrintfCpuUsage()
{
	CPUUsage.UpdateCpuTime();
	printf("-----------------------------------------------------------------------\n");
	printf("Processor Total : %f || Process Total : %f\n", CPUUsage.ProcessorTotal(), CPUUsage.ProcessTotal());
	printf("Processor User : %f || Process User : %f\n", CPUUsage.ProcessorUser(), CPUUsage.ProcessUser());
	printf("Processor Kernel : %f || Process Kernel : %f\n", CPUUsage.ProcessorKernel(), CPUUsage.ProcessKernel());
}


void PrintConsolAll()
{
	PrintMessageCount();
	PrintSerializePool();
	PrintSessionCount();
	PrintfCpuUsage();
}


void ExitAllProcess()
{
	//서버 종료 절차
	pLib->ExitNetWorkManager();



}

void UpdateMonitorData()
{
	long long localRECVTPS = 0;
	long long localSENDTPS = 0;
	int localFrame = 0;
	int localProcessorTotal = 0;
	double PrivateMem = 0;
	double ProcessNonPaged = 0;
	double TotalNonPaged = 0;
	double Available = 0;
	long long localAcceptTPS = InterlockedExchange(&g_AcceptTps, 0);



	for (int i = 0; i < pLib->_workerThreadCount; i++)
	{
		localRECVTPS += InterlockedExchange(&g_pRecvTps[i], 0);
		localSENDTPS += InterlockedExchange(&g_pSendTps[i], 0);
	}
	localFrame = InterlockedExchange(&g_frame, 0);

	g_PDH.GetMemoryData(&PrivateMem, &ProcessNonPaged, &TotalNonPaged, &Available);
	
	CPUUsage.UpdateCpuTime();
	localProcessorTotal = (int)CPUUsage.ProcessorTotal();
	
	g_Monitor.UpdateMonitor(dfMONITOR_DATA_TYPE_CHAT_SERVER_RUN, 1);
	g_Monitor.UpdateMonitor(dfMONITOR_DATA_TYPE_CHAT_SERVER_CPU, localProcessorTotal);
	g_Monitor.UpdateMonitor(dfMONITOR_DATA_TYPE_CHAT_SERVER_MEM, PrivateMem / 1024 / 1024);
	g_Monitor.UpdateMonitor(dfMONITOR_DATA_TYPE_CHAT_SESSION, pLib->_sessionLoginCount);
	g_Monitor.UpdateMonitor(dfMONITOR_DATA_TYPE_CHAT_PLAYER, g_PlayerLogInCount);
	g_Monitor.UpdateMonitor(dfMONITOR_DATA_TYPE_CHAT_UPDATE_TPS, localRECVTPS);
}