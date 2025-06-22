#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#include <process.h>
#include <random>
#include <algorithm>
#include <vector>
#include <conio.h>
#include <atomic>

#include "MonitoringDatas.h"
#include "Parser.h"
#include "MakePacket.h"
#include "Network.h"
#include "Protocol.h"
#include "Sector.h"
#include "RingBuffer.h"

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "Winmm.lib")

using namespace std;

bool GetDummySetting(string& clientIP, int& clientPort, int& clientUsers, int& disconnectRandom, int& contentRandom);

HANDLE g_hExitEvent = nullptr;
int disconnectMode;
bool disConnectStop = false;
bool packetSendStop = false;
bool checkHeartBeatStop = true;
int trustMode;

void ProcessNetwork(int startIndex, int endIndex);
void UpdateContent(int startIndex, int endIndex);
void UpdateChat(int startIndex, int endIndex);
void UpdateContentAndChat(int startIndex, int endIndex, bool stopPacket);
int CheckClientCnt();
void CloseSession(Session* session);
void PrintState(int trustMode);


SRWLOCK g_SectorLock[dfSECTOR_MAX_Y][dfSECTOR_MAX_X];

std::vector<Session*> g_Sessions;
//오류 체크용
LONG moveStopCnt;
LONG chatCompleteCnt;
LONG chatPlayerCnt;
LONG moveStopError;
LONG chatCompleteError;
LONG totalMoveStopSend;
LONG totalMoveStopRecv;
LONG MoveStopOverlapError;
LONG waitingForDisconnetCnt;
LONG charOverlappedCnt;

//출력용
LONG totalMessageRecv;
//LONG connectCnt;
LONG totalConnect;
LONG connectTryCnt;
LONG connectFail;
LONG disconnectFail;
LONG stateLoopCnt;
LONG wrongChatCnt;
LONG packetTypeErrorCnt;
LONG packetlenErrorCnt;
LONG fixedCodeErrorCnt;
LONG checkSumErrorCnt;
LONG heartBeatErrorCnt;

LONG forSendCheck;

LONG forDebug;
LONG forFrameCheck;
//LONG forDebugCloseSession;

enum ClientState {STATE_MOVING, STATE_WAIT_STOP, STATE_CHATTING, STATE_WAIT_CHAT, STATE_SYNC};
std::atomic<ClientState> g_GlobalState{ STATE_MOVING };

LONG waitThreadCount;

int THREAD_COUNT;

struct ThreadParam
{
	std::string serverIP;
	int serverPort;
	int startIndex;
	int endIndex;
	int threadnumber;
};

const int MSG_COUNT = 20;
vector<string> g_randomMessages;

DWORD conRandom = 0;
DWORD disRandom = 0;
DWORD conRandomPlay = 0;
DWORD disRandomPlay = 0;

void InitRandomMessage()
{
	g_randomMessages.reserve(MSG_COUNT);
	
	for (int i = 0; i < MSG_COUNT; i++)
	{
		int len = 4 + i;
		string s;
		s.reserve(len);
		for (int j = 0; j < len; j++)
		{
			char c = 'a' + rand() % 26;
			s += c;
		}
		g_randomMessages.emplace_back(s);
	}
}

unsigned __stdcall ClientThread1(void* param)
{
	//cout << "thread start\n";
	auto* tp = static_cast<ThreadParam*>(param);

	int startIndex = tp->startIndex;
	int endIndex = tp->endIndex;

	srand(time(NULL) + tp->threadnumber);

	for (int idx = startIndex; idx <= endIndex; idx++)
	{
		Session* sess = g_Sessions[idx];

		if (!connectSession(sess, tp->serverIP.c_str(), tp->serverPort)) 
		{
			// 실패한 세션은 건너뜀
			continue;
		}
	}
	

	/*DWORD startTime = timeGetTime();
	const DWORD FRAME = 40;*/

	//cout << "프레임시작\n";

	//디버그용 frame체크
	DWORD fpsUpdateTime = timeGetTime();
	int frameCount = 0;
	int FPSframe = 0;

	/*
	ClientState state = STATE_MOVING;
	const DWORD MOVE_TIME = 3000;
	const DWORD CHAT_TIME = 1000;*/

	//ClientState state = STATE_MOVING;
	DWORD       phaseStart = timeGetTime();
	const DWORD MOVE_TIME = 3000; // 3초
	const DWORD CHAT_TIME = 1000; // 1초
	const DWORD FRAME_MS = 40;
	DWORD       nextFrame = timeGetTime() + FRAME_MS;
	DWORD       chatStart = 0;
	DWORD heartBeatTime = timeGetTime();
	DWORD frameTime = timeGetTime();
	bool threadEnd = false;
	ClientState oldState = g_GlobalState.load(std::memory_order_acquire);

	while (WaitForSingleObject(g_hExitEvent,0) == WAIT_TIMEOUT)
	{
		//frameCount++;

		if (timeGetTime() - heartBeatTime >= 20000)
		{
			for (int idx = startIndex; idx <= endIndex; idx++)
			{
				Session* sess = g_Sessions[idx];

				if (!sess->isSession || !sess->isConnected)
				{
					continue;
				}

				PacketBuffer packet;
				mpHeartBeat(&packet);
				SendPacket_Unicast(sess, &packet);
			}
			heartBeatTime += 20000;
		}

		if (timeGetTime() - frameTime >= 1000)
		{
			if (FPSframe < 23)
			{
				InterlockedIncrement(&forFrameCheck);
			}
			frameTime += 1000;
			FPSframe = 0;
		}

		FPSframe++;

		ClientState state = g_GlobalState.load(std::memory_order_acquire);
		DWORD now = timeGetTime();
		if (oldState != state)
		{
			threadEnd = false;
			oldState = state;
			if (state == STATE_CHATTING)
				chatStart = now;
			if (state == STATE_MOVING)
				phaseStart = now;
		}


		switch (state)
		{
		case STATE_MOVING:
			if (now - phaseStart >= MOVE_TIME)
			{
				if (!threadEnd)
				{
					for (int i = startIndex; i <= endIndex; i++)
					{
						Session* session = g_Sessions[i];
						if (!session->isConnected || !session->isSession || !session->isCharacterActive)
							continue;

						if (session->isMoving)
						{

							PacketBuffer buf;
							session->byDirection = rand() % 2 == 0 ? 0 : 4;
							//session->shX = 3500;
							//session->shY = 3500;
							Sector_UpdateSession(session);
							mpMoveStop(&buf, session->byDirection, session->shX, session->shY);
							SendPacket_Unicast(session, &buf);
							session->isMoving = false;
							int ret = InterlockedIncrement(&moveStopCnt);
							InterlockedIncrement(&totalMoveStopSend);
							session->moveStopRecv = true;
							//printf("moveStopCnt : %d\n", ret);
						}
						//else
						//{
						//	session->moveStopRecv = true;
						//}
					}
					threadEnd = true;
					InterlockedIncrement(&waitThreadCount);
				}
				
				if (tp->threadnumber == 0)
				{
					if (InterlockedCompareExchange(&waitThreadCount, 0, THREAD_COUNT) == THREAD_COUNT)
					{
						g_GlobalState.store(STATE_WAIT_STOP, std::memory_order_release);
						//Sleep(1000);
						//Sleep(2000);
						//InterlockedExchange(&waitThreadCount, 0);
					}
				}
				//printf("moveStopCnt : %d", moveStopCnt);
			}
			else
			{
				for (int idx = startIndex; idx <= endIndex; idx++)
				{
					Session* sess = g_Sessions[idx];

					if (sess->isSession)
					{
						continue;
					}
					if (!connectSession(sess, tp->serverIP.c_str(), tp->serverPort))
					{
						// 실패한 세션은 건너뜀
						continue;
					}
				}
				UpdateContent(startIndex, endIndex);
			}
			break;
		case STATE_WAIT_STOP:
			if (InterlockedCompareExchange(&moveStopCnt, 0, 0) == 0)
			{
				//printf("+++++++++++++++++++=====================++++이동 멈춤 완료++++++++++++++++++=========================+++++++++++++\n");
				if (!threadEnd)
				{
					for (int i = startIndex; i <= endIndex; i++)
					{
						Session* session = g_Sessions[i];
						if (!session->isConnected || !session->isSession || !session->isCharacterActive)
							continue;

						if (session->moveStopRecv)
						{
							//printf("session ID : %llu / movestopnotrecved\n", session->id);
							InterlockedIncrement(&moveStopError);
						}

						if (!session->isReadyForChat)
							session->isReadyForChat = true;

					}

					threadEnd = true;
					InterlockedIncrement(&waitThreadCount);
				}
			}
			if (tp->threadnumber == 0)
			{
				if (InterlockedCompareExchange(&waitThreadCount, 0, THREAD_COUNT) == THREAD_COUNT)
				{
					g_GlobalState.store(STATE_CHATTING, std::memory_order_release);

					//Sleep(1000);
					//InterlockedExchange(&waitThreadCount, 0);
				}
			}
			break;
		case STATE_CHATTING:
			if (now - chatStart > CHAT_TIME)
			{
				if (!threadEnd)
				{
					for (int i = startIndex; i <= endIndex; i++)
					{
						Session* session = g_Sessions[i];

						if (!session->isConnected || !session->isSession || !session->isCharacterActive)
							continue;

						if (session->chatSent)
						{
							PacketBuffer packet;
							mpChatEnd(&packet);
							SendPacket_Unicast(session, &packet);
						}
					}
					threadEnd = true;
					InterlockedIncrement(&waitThreadCount);
				}

				if (tp->threadnumber == 0)
				{
					if (InterlockedCompareExchange(&waitThreadCount, 0, THREAD_COUNT) == THREAD_COUNT)
					{
						g_GlobalState.store(STATE_WAIT_CHAT, std::memory_order_release);
						//InterlockedExchange(&waitThreadCount, 0);
					}
				}
				//state = STATE_WAIT_CHAT;
			}
			else
			{
				UpdateChat(startIndex, endIndex);
			}
			break;
		case STATE_WAIT_CHAT:
	/*		int cnt = 0;

			for (int i = startIndex; i <= endIndex; i++)
			{
				Session* session = g_Sessions[i];
				if (session->chatSent)
					cnt++;
			}*/

			//printf("--------cnt : %d----------\n", cnt);

			//printf("chatCompleteCnt : %d / chatPlayerCnt : %d\n", (int)chatCompleteCnt, (int)chatPlayerCnt);
			
			//&& InterlockedCompareExchange(&chatPlayerCnt,0,0)==0
			if (InterlockedCompareExchange(&chatCompleteCnt, 0, 0) == 0 && InterlockedCompareExchange(&chatPlayerCnt, 0, 0) == 0)
			{
				if (!threadEnd)
				{
					for (int i = startIndex; i <= endIndex; i++)
					{
						Session* session = g_Sessions[i];
						if (!session->isSession || !session->isConnected || !session->isCharacterActive)
							continue;

						if (session->chatSent)
						{
							printf("session %llu missed ChatComplete\n", session->id);
							InterlockedIncrement(&chatCompleteError);
						}
					}

					for (int i = startIndex; i <= endIndex; i++)
					{
						Session* session = g_Sessions[i];
						session->chatSent = false;
						session->moveStopRecv = false;
						session->isReadyForChat = false;
					}
					threadEnd = true;
					InterlockedIncrement(&waitThreadCount);
				}

				if (tp->threadnumber == 0)
				{
					if (InterlockedCompareExchange(&waitThreadCount, 0, THREAD_COUNT) == THREAD_COUNT)
					{
						g_GlobalState.store(STATE_MOVING, std::memory_order_release);
						InterlockedIncrement(&stateLoopCnt);
						//InterlockedExchange(&waitThreadCount, 0);
					}
				}
				//state = STATE_MOVING;
				//phaseStart = now;
			}
			break;
		}

		ProcessNetwork(startIndex, endIndex);

		//if (state == STATE_MOVING)
		//	UpdateContent(startIndex, endIndex);
		//else if (state == STATE_CHATTING)
		//	UpdateChat(startIndex, endIndex);


		DWORD sleepTime = nextFrame - timeGetTime();
		if (sleepTime <= FRAME_MS)
			Sleep(sleepTime);

		nextFrame += FRAME_MS;
		//cout << state << '\n';
	}

	return 0;
}


unsigned __stdcall ClientThread2(void* param)
{
	auto* tp = static_cast<ThreadParam*>(param);

	int startIndex = tp->startIndex;
	int endIndex = tp->endIndex;

	srand(time(NULL) + tp->threadnumber);

	for (int idx = startIndex; idx <= endIndex; idx++)
	{
		Session* sess = g_Sessions[idx];

		if (!checkHeartBeatStop)
		{
			int heartBeatCheck = rand() % 150;
			if (heartBeatCheck == 50)
			{
				sess->checkHeartBeat = true;
				InterlockedIncrement(&waitingForDisconnetCnt);
			}
		}

		if (!connectSession(sess, tp->serverIP.c_str(), tp->serverPort))
		{
			// 실패한 세션은 건너뜀
			InterlockedDecrement(&waitingForDisconnetCnt);
			continue;
		}
	}

	//디버그용 frame체크
	DWORD fpsUpdateTime = timeGetTime();
	int frameCount = 0;
	int FPSframe = 0;
	const DWORD FRAME_MS = 40;
	DWORD nextFrame = timeGetTime() + FRAME_MS;
	DWORD heartBeatTime = timeGetTime();
	DWORD frameTime = timeGetTime();

	while (WaitForSingleObject(g_hExitEvent, 0) == WAIT_TIMEOUT)
	{
		/*if (timeGetTime() - heartBeatTime >= 20000)
		{
			for (int idx = startIndex; idx <= endIndex; idx++)
			{
				Session* sess = g_Sessions[idx];

				if (!sess->isSession || !sess->isConnected || sess->checkHeartBeat)
				{
					continue;
				}

				PacketBuffer packet;
				mpHeartBeat(&packet);
				InterlockedIncrement(&forSendCheck);
				SendPacket_Unicast(sess, &packet);
			}
			heartBeatTime += 20000;
		}*/

		if (timeGetTime() - frameTime >= 1000)
		{
			if (FPSframe < 23)
			{
				InterlockedIncrement(&forFrameCheck);
			}
			FPSframe = 0;
			frameTime += 1000;
		}

		FPSframe++;

		bool packetActive = packetSendStop;

		DWORD now = timeGetTime();

		ProcessNetwork(startIndex, endIndex);

		for (int idx = startIndex; idx <= endIndex; idx++)
		{
			Session* sess = g_Sessions[idx];

			if (sess->isSession)
			{
				continue;
			}

			if (!checkHeartBeatStop)
			{
				int heartBeatCheck = rand() % 150;
				if (heartBeatCheck == 50)
				{
					sess->checkHeartBeat = true;
					InterlockedIncrement(&waitingForDisconnetCnt);
				}
			}

			if (!connectSession(sess, tp->serverIP.c_str(), tp->serverPort))
			{
				// 실패한 세션은 건너뜀
				InterlockedDecrement(&waitingForDisconnetCnt);
				continue;
			}
		}

		UpdateContentAndChat(startIndex, endIndex, packetActive);

		DWORD sleepTime = nextFrame - timeGetTime();
		if (sleepTime <= FRAME_MS)
			Sleep(sleepTime);

		nextFrame += FRAME_MS;
	}


	return 0;
}

int main()
{
	srand(time(NULL));
	timeBeginPeriod(1);


	string clientIP;
	int clientPort;
	int clientUsers;
	int disconnectRandom;
	int contentRandom;

	if (GetDummySetting(clientIP, clientPort, clientUsers, disconnectRandom, contentRandom))
	{
		printf("Client Setting Parser Clear!\n");
	}
	else
	{
		printf("Client Setting Parser something wrong\n");
		return 1;
	}


	g_hExitEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);

	for (int y = 0; y < dfSECTOR_MAX_Y; y++)
	{
		for (int x = 0; x < dfSECTOR_MAX_X; x++)
		{
			InitializeSRWLock(&g_SectorLock[y][x]);
		}
	}

	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);

	InitRandomMessage();

	//문자열 출력 검증 
	//for (int i = 0; i < MSG_COUNT; i++)
	//{
	//	cout << g_randomMessages[i] << '\n';
	//}

	std::string serverIP;
	int serverPort, clientCount;
	serverIP = clientIP;
	serverPort = clientPort;
	clientCount = clientUsers;
	conRandom = contentRandom;
	disRandom = disconnectRandom;
	conRandomPlay = conRandom/2;
	disRandomPlay = disRandom/2;
	//std::cout << "Server IP: ";    std::cin >> serverIP;
	//std::cout << "Server Port: ";  std::cin >> serverPort;
	//std::cout << "Client Count: ";  std::cin >> clientCount;
	std::cout << "Disconnect YES = 0 / NO = 1: "; std::cin >> disconnectMode;
	std::cout << "Dummy Mode - Trust / Yes = 0 / NO = 1: "; std::cin >> trustMode;

	g_Sessions.reserve(clientCount);

	for (int i = 0; i < clientCount; i++)
	{
		Session* newSession = new Session();
		g_Sessions.emplace_back(newSession);
	}

	const int perThread = 500;
	int threadCount = (clientCount) / perThread;
	if (clientCount % perThread > 0)
		threadCount++;

	std::vector<HANDLE> threads;
	//threads.reserve(threadCount);
	cout << "threadCount : " << threadCount << '\n';
	THREAD_COUNT = threadCount;

	for (int i = 0; i < threadCount; i++)
	{
		int start = i * perThread;
		int end = min((i + 1) * perThread - 1, clientCount - 1);

		//cout << "startIndex : " << start << '\n';
		//cout << "endIndex : " << end << '\n';

		auto* param = new ThreadParam{ serverIP, serverPort, start, end, i};
		HANDLE h;
		if (trustMode==0)
		{
			h = (HANDLE)_beginthreadex(nullptr, 0, ClientThread1, param, 0, 0);
		}
		else
		{
			h = (HANDLE)_beginthreadex(nullptr, 0, ClientThread2, param, 0, 0);
		}

		threads.push_back(h);
	}

	DWORD startTime = timeGetTime();
	time_t now = time(nullptr);
	struct tm localTime;
	localtime_s(&localTime, &now);
	while (1)
	{
		if (timeGetTime() - startTime >= 1000)
		{
			//tps구하는거 하고 pdh로 processor만이라도 확인해야 되겠는데??
			printf("StartTime : %04d.%02d.%02d %02d:%02d:%02d\n",
				localTime.tm_year + 1900,
				localTime.tm_mon + 1,
				localTime.tm_mday,
				localTime.tm_hour,
				localTime.tm_min,
				localTime.tm_sec);
			printf("─────────────────────────────────────────────\n");
			//system("cls");
			printf("q : quit | s : stop | d : disconnect | h : heartBeat\n");
			printf("===============================\n");
			printf("Client : %d | Thread : %d\n", CheckClientCnt(), THREAD_COUNT);
			PrintState(trustMode);
			printf("===============================\n");
			printf("\n");
			//printf("connect : %lu\n", connectCnt);
			printf("Connect Total : %lu\n", totalConnect);
			printf("Connect Fail  : %lu\n", connectFail);
			printf("Connect Try   : %lu\n", connectTryCnt);
			printf("\n");
			printf("moveStopCnt          : %lu\n", moveStopCnt);
			printf("chatCompleteCnt      : %lu\n", chatCompleteCnt);
			printf("chatPlayerCnt        : %lu\n", chatPlayerCnt);
			printf("waitforDisconnectCnt : %lu\n", waitingForDisconnetCnt);
			printf("\n");
			printf("Error - Disconnect Total     : %lu\n", disconnectFail);
			printf("Error - ChatPacketError      : %lu\n", wrongChatCnt);
			printf("Error - PacketTypeError      : %lu\n", packetTypeErrorCnt);
			printf("Error - PacketLenError       : %lu\n", packetlenErrorCnt);
			printf("Error - FixedCodeError       : %lu\n", fixedCodeErrorCnt);
			printf("Error - checkSumError        : %lu\n", checkSumErrorCnt);
			printf("Error - MoveStopOverlapError : %lu\n", MoveStopOverlapError);
			printf("Error - heartBeatError       : %lu\n", heartBeatErrorCnt);
			printf("Error - charOverlappedCnt    : %lu\n", charOverlappedCnt);
			printf("Error - %lu / sendCheck : %lu\n", forDebug, forSendCheck);
			printf("frame - %lu\n", forFrameCheck);
			printf("--------------------------------\n");
			int rttMax, rttMin, rttAvg;
			RTTData.GetSnapshot(rttMin, rttMax, rttAvg);
			printf("RTT MIN : %d\n", rttMin);
			printf("RTT MAX : %d\n", rttMax);
			printf("RTT AVG : %d\n", rttAvg);

			//printf("Error - %lu\n", forDebugCloseSession);
			//for (auto& session : g_Sessions)
			//{
			//	for (int i = 0; i < MSG_COUNT; ++i)
			//	{
			//		if (session->messageCount[i] != 0)
			//		{
			//			printf("id : %llu / mescountid : %d\n", session->id, session->messageCount[i]);
			//			printf("session alive : %d\n", (int)session->isSession);
			//		}
			//	}
			//}
			/*printf("===============================\n");
			printf("totalRecv : %lu\n", totalMessageRecv);
			printf("totalMoveStopSend : %lu\n", totalMoveStopSend);
			printf("totalMoveStopRecv : %lu\n", totalMoveStopRecv);*/

			startTime += 1000;
		}

		if (_kbhit())
		{
			char c = _getch();
			if (c == 'q' || c == 'Q')
			{
				SetEvent(g_hExitEvent);
				break;
			}
			else if (c == 'd' || c == 'D')
			{
				disConnectStop = !disConnectStop;
			}
			else if (c == 's' || c == 'S')
			{
				packetSendStop = !packetSendStop;
			}
			else if (c == 'h' || c == 'H')
			{
				checkHeartBeatStop = !checkHeartBeatStop;
			}
		}

		Sleep(100);
	}

	WaitForMultipleObjects
	(
		static_cast<DWORD>(threads.size()),
		threads.data(),
		TRUE,      // 모든 스레드를 기다림
		INFINITE   // 무한 대기
	);

	for (auto* sess : g_Sessions)
	{
		if (sess->socket != INVALID_SOCKET)
		{
			closesocket(sess->socket);
			printf("session id : %llu is closing\n", sess->id);
		}
	}

	for (auto h : threads) CloseHandle(h);

	CloseHandle(g_hExitEvent);
	WSACleanup();

	return 0;
}




void ProcessNetwork(int startIndex, int endIndex)
{
	const int maxFDS = FD_SETSIZE;
	timeval tv{ 0,0 };


	for (int base = startIndex; base <= endIndex; base += maxFDS)
	{
		int selectStart = base;
		int selectEnd = min(endIndex, base + maxFDS - 1);

		FD_SET rset, wset, eset;
		FD_ZERO(&rset);
		FD_ZERO(&wset);
		FD_ZERO(&eset);

		int fdCount = 0;
		for (int idx = selectStart; idx <= selectEnd; idx++)
		{
			Session* session = g_Sessions[idx];
			if (!session->isSession || session->socket == INVALID_SOCKET)
				continue;

			if (!session->isConnected)
			{
				FD_SET(session->socket, &wset);
				FD_SET(session->socket, &eset);
				fdCount++;
			}
			else
			{
				FD_SET(session->socket, &rset);
				fdCount++;
				if (session->sendQ.GetUseSize() > 0)
					FD_SET(session->socket, &wset);
			}

		}

		if (fdCount == 0)
			continue;

		int sel = select(0, &rset, &wset, &eset, &tv);
		if (sel == SOCKET_ERROR)
		{
			std::cout << "select error: " << WSAGetLastError() << "\n";
			continue;
		}
		if (sel == 0)
			continue;

		//non-blocking connect를 위해서
		for (int idx = selectStart; idx <= selectEnd; ++idx)
		{
			Session* session = g_Sessions[idx];
			SOCKET sock = session->socket;
			if (!session->isSession || sock == INVALID_SOCKET || session->isConnected)
				continue;

			if (FD_ISSET(sock, &wset)||FD_ISSET(sock, &eset))
			{
				--sel;
				int err = 0, len = sizeof(err);
				getsockopt(sock, SOL_SOCKET, SO_ERROR, (char*)&err, &len);
				if (err == 0) 
				{
					// 연결 성공!
					session->isConnected = true;
					InterlockedIncrement(&totalConnect);
					//InterlockedDecrement(&connectCnt);
				}
				else 
				{
					// 연결 실패
					closesocket(sock);
					session->socket = INVALID_SOCKET;
					session->isSession = false;
					InterlockedIncrement(&connectFail);
				}
			}
		}

		for (int idx = selectStart; idx <= selectEnd && sel > 0; idx++)
		{
			Session* session = g_Sessions[idx];
			SOCKET sock = session->socket;

			if (sock == INVALID_SOCKET)
				continue;

			if (FD_ISSET(sock, &rset))
			{
				--sel;
				if (!netProc_Recv(session))
				{
					//closesocket(session->socket);
					//session->socket = INVALID_SOCKET;
					//session->isSession = false;
					CloseSession(session);
					continue;
				}

				// netProc_Recv(s);
			}
			if (FD_ISSET(sock, &wset))
			{
				--sel;
				if (!netProc_Send(session))
				{
					/*closesocket(session->socket);
					session->socket = INVALID_SOCKET;
					session->isSession = false;*/
					CloseSession(session);
					continue;
				}
				
				// netProc_Send(s);
			}
		}
	}
}

bool CharacterMoveCheck(short x, short y)
{
	if (x < dfRANGE_MOVE_LEFT || x >= dfRANGE_MOVE_RIGHT || y < dfRANGE_MOVE_TOP || y >= dfRANGE_MOVE_BOTTOM)
		return false;

	return true;
}

void UpdateContent(int startIndex, int endIndex)
{
	for (int idx = startIndex; idx <= endIndex; idx++)
	{
		Session* session = g_Sessions[idx];
		if (!session->isSession || !session->isConnected || !session->isCharacterActive)
			continue;

		if (!session->isMoving)
		{
			session->isMoving = true;
			PacketBuffer packet;

			session->byDirection = rand() % 8;
		
			mpMoveStart(&packet, session->byDirection, session->shX, session->shY);
			SendPacket_Unicast(session, &packet);

			//InterlockedIncrement(&moveStopCnt);
		}

		switch (session->byDirection)
		{
		case dfPACKET_MOVE_DIR_LL:
			if (CharacterMoveCheck(session->shX - dfSPEED_PLAYER_X, session->shY))
			{
				session->shX -= dfSPEED_PLAYER_X;
			}
			break;
		case dfPACKET_MOVE_DIR_LU:
			if (CharacterMoveCheck(session->shX - dfSPEED_PLAYER_X, session->shY - dfSPEED_PLAYER_Y))
			{
				session->shX -= dfSPEED_PLAYER_X;
				session->shY -= dfSPEED_PLAYER_Y;
			}
			break;
		case dfPACKET_MOVE_DIR_UU:
			if (CharacterMoveCheck(session->shX, session->shY - dfSPEED_PLAYER_Y))
				session->shY -= dfSPEED_PLAYER_Y;
			break;
		case dfPACKET_MOVE_DIR_RU:
			if (CharacterMoveCheck(session->shX + dfSPEED_PLAYER_X, session->shY - dfSPEED_PLAYER_Y))
			{
				session->shX += dfSPEED_PLAYER_X;
				session->shY -= dfSPEED_PLAYER_Y;
			}
			break;
		case dfPACKET_MOVE_DIR_RR:
			if (CharacterMoveCheck(session->shX + dfSPEED_PLAYER_X, session->shY))
				session->shX += dfSPEED_PLAYER_X;
			break;
		case dfPACKET_MOVE_DIR_RD:
			if (CharacterMoveCheck(session->shX + dfSPEED_PLAYER_X, session->shY + dfSPEED_PLAYER_Y))
			{
				session->shX += dfSPEED_PLAYER_X;
				session->shY += dfSPEED_PLAYER_Y;
			}
			break;
		case dfPACKET_MOVE_DIR_DD:
			if (CharacterMoveCheck(session->shX, session->shY + dfSPEED_PLAYER_Y))
				session->shY += dfSPEED_PLAYER_Y;
			break;
		case dfPACKET_MOVE_DIR_LD:
			if (CharacterMoveCheck(session->shX - dfSPEED_PLAYER_X, session->shY + dfSPEED_PLAYER_Y))
			{
				session->shX -= dfSPEED_PLAYER_X;
				session->shY += dfSPEED_PLAYER_Y;
			}
			break;
		}

		Sector_UpdateSession(session);

		if (disconnectMode == 0 && !disConnectStop)
		{
			int disconnect = rand() % 700;
			if (disconnect == 15)
			{
				CloseSession(session);
			}
		}
	}
}

void UpdateChat(int startIndex, int endIndex)
{
	//static int test = 0;

	for (int idx = startIndex; idx <= endIndex; idx++)
	{
		Session* session = g_Sessions[idx];

		if (!session->isSession || !session->isConnected || !session->isCharacterActive)
			continue;

		if (!session->isReadyForChat)
			continue;

		int willSend = rand() % 10;
		//int willSend = 10;

		if (willSend == 5)
		{
			int randomIdx = rand() % 20;

			st_SECTOR_AROUND aroundSectors;
			GetSectorAround(session->CurSector.iX, session->CurSector.iY, &aroundSectors);

			for (int i = 0; i < aroundSectors.iCount; i++)
			{
				int sectorX = aroundSectors.Around[i].iX;
				int sectorY = aroundSectors.Around[i].iY;


				for (auto* otherSession : g_Sector[sectorY][sectorX])
				{
					if (otherSession == session)
						continue;

					if (!otherSession->isReadyForChat)
						continue;

					if (!otherSession->isSession || !otherSession->isConnected || !otherSession->isCharacterActive)
						continue;

					InterlockedIncrement(&otherSession->messageCount[randomIdx]);
					InterlockedIncrement(&chatCompleteCnt);
					//test++;
					//printf("total localSend : %d\n", test);
					//printf("session id : %llu / x : %d / y : %d\n",session->id, session->shX, session->shY);
				}
			}

			PacketBuffer packet;

			const string& msg = g_randomMessages[randomIdx];
			//cout << "msg : " << msg << '\n';
			mpLocalChat(&packet, (BYTE)msg.size(), msg.c_str());
			SendPacket_Unicast(session, &packet);
			if (!session->chatSent)
			{
				session->chatSent = true;
				InterlockedIncrement(&chatPlayerCnt);
			}
		}
	}
}

int CheckClientCnt()
{
	int cnt = 0;
	for (auto* session : g_Sessions)
	{
		if (session->isCharacterActive)
			cnt++;
	}
	return cnt;
}

void PrintState(int trustMode)
{
	if (trustMode != 0)
		return;
	//enum ClientState { STATE_MOVING, STATE_WAIT_STOP, STATE_CHATTING, STATE_WAIT_CHAT, STATE_SYNC };
	std::string state;
	ClientState State = g_GlobalState.load(std::memory_order_acquire);
	if (State == 0)
		state = "STATE_MOVING";
	else if (State == 1)
		state = "STATE_WAIT_STOP";
	else if (State == 2)
		state = "STATE_CHATTING";
	else if (State == 3)
		state = "STATE_WAIT_CHAT";

	printf("current state : %s | state loop cnt: %lu\n", state.c_str(), stateLoopCnt);
}


void CloseSession(Session* session)
{
	//InterlockedIncrement(&forDebugCloseSession);
	closesocket(session->socket);
	session->socket = INVALID_SOCKET;
	session->isSession = false;
	session->isCharacterActive = false;
	session->isConnected = false;
	session->isReadyForChat = false;
	session->isMoving = false;
	session->recvQ.ClearBuffer();
	session->sendQ.ClearBuffer();
	session->id = 0;

	RemoveSector(session->CurSector, session);
	if (session->checkHeartBeat)
	{
		session->checkHeartBeat = false;
		InterlockedDecrement(&waitingForDisconnetCnt);
	}
	if (session->chatSent)
	{
		session->chatSent = false;
		InterlockedDecrement(&chatPlayerCnt);
	}
	if (session->moveStopRecv)
	{
		session->moveStopRecv = false;
		InterlockedDecrement(&moveStopCnt);
	}
	LONG cnt = 0;
	for (int i = 0; i < MSG_COUNT; ++i)
	{
		// old = 세션이 지금까지 보낸 메시지 개수
		LONG old = InterlockedExchange(&session->messageCount[i], 0);
		cnt += old;
	}
	//int cnt = 0;
	//for (auto& mesCnt : session->messageCount)
	//{
	//	if (mesCnt == 0)
	//		continue;

	//	cnt += mesCnt;
	//	mesCnt = 0;
	//}

	InterlockedAdd(&chatCompleteCnt, -cnt);
}

void UpdateContentAndChat(int startIndex, int endIndex, bool stopPacket)
{
	for (int idx = startIndex; idx <= endIndex; idx++)
	{
		Session* session = g_Sessions[idx];
		if (!session->isSession || !session->isConnected || !session->isCharacterActive)
			continue;

		if (!stopPacket && !session->isMoving && !session->moveStopRecv)
		{
			int startRand = rand() % conRandom;
			if (startRand == conRandomPlay)
			{
				session->isMoving = true;
				PacketBuffer packet;

				session->byDirection = rand() % 8;

				mpMoveStart(&packet, session->byDirection, session->shX, session->shY);
				SendPacket_Unicast(session, &packet);
				InterlockedIncrement(&forSendCheck);
			}
		}

		switch (session->byDirection)
		{
		case dfPACKET_MOVE_DIR_LL:
			if (CharacterMoveCheck(session->shX - dfSPEED_PLAYER_X, session->shY))
			{
				session->shX -= dfSPEED_PLAYER_X;
			}
			break;
		case dfPACKET_MOVE_DIR_LU:
			if (CharacterMoveCheck(session->shX - dfSPEED_PLAYER_X, session->shY - dfSPEED_PLAYER_Y))
			{
				session->shX -= dfSPEED_PLAYER_X;
				session->shY -= dfSPEED_PLAYER_Y;
			}
			break;
		case dfPACKET_MOVE_DIR_UU:
			if (CharacterMoveCheck(session->shX, session->shY - dfSPEED_PLAYER_Y))
				session->shY -= dfSPEED_PLAYER_Y;
			break;
		case dfPACKET_MOVE_DIR_RU:
			if (CharacterMoveCheck(session->shX + dfSPEED_PLAYER_X, session->shY - dfSPEED_PLAYER_Y))
			{
				session->shX += dfSPEED_PLAYER_X;
				session->shY -= dfSPEED_PLAYER_Y;
			}
			break;
		case dfPACKET_MOVE_DIR_RR:
			if (CharacterMoveCheck(session->shX + dfSPEED_PLAYER_X, session->shY))
				session->shX += dfSPEED_PLAYER_X;
			break;
		case dfPACKET_MOVE_DIR_RD:
			if (CharacterMoveCheck(session->shX + dfSPEED_PLAYER_X, session->shY + dfSPEED_PLAYER_Y))
			{
				session->shX += dfSPEED_PLAYER_X;
				session->shY += dfSPEED_PLAYER_Y;
			}
			break;
		case dfPACKET_MOVE_DIR_DD:
			if (CharacterMoveCheck(session->shX, session->shY + dfSPEED_PLAYER_Y))
				session->shY += dfSPEED_PLAYER_Y;
			break;
		case dfPACKET_MOVE_DIR_LD:
			if (CharacterMoveCheck(session->shX - dfSPEED_PLAYER_X, session->shY + dfSPEED_PLAYER_Y))
			{
				session->shX -= dfSPEED_PLAYER_X;
				session->shY += dfSPEED_PLAYER_Y;
			}
			break;
		}

		Sector_UpdateSession(session);

		if (!stopPacket && session->isMoving)
		{
			int stopRand = rand() % conRandom;
			if (stopRand == conRandomPlay)
			{
				PacketBuffer stopPacket;
				session->byDirection = rand() % 2 == 0 ? 0 : 4;

				mpMoveStop(&stopPacket, session->byDirection, session->shX, session->shY);
				SendPacket_Unicast(session, &stopPacket);
				InterlockedIncrement(&forSendCheck);
				session->isMoving = false;
				session->moveStopRecv = true;
				int ret = InterlockedIncrement(&moveStopCnt);
			}
		}

		if (disconnectMode == 0 && !disConnectStop && !session->checkHeartBeat)
		{
			int disconnect = rand() % disRandom;
			if (disconnect == disRandomPlay)
			{
				CloseSession(session);
				continue;
			}
		}

		if (!stopPacket &&!session->chatSent)
		{
			int chatRand = rand() % (conRandom/2);

			if (chatRand == (conRandomPlay/2))
			{
				int randomIndex = rand() % 20;

				PacketBuffer packet;
				const string& msg = g_randomMessages[randomIndex];

				mpLocalChat(&packet, (BYTE)msg.size(), msg.c_str());
				SendPacket_Unicast(session, &packet);
				InterlockedIncrement(&forSendCheck);
				PacketBuffer chatEndPacket;
				mpChatEnd(&packet);
				//session->rtt = timeGetTime();
				SendPacket_Unicast(session, &packet);
				InterlockedIncrement(&forSendCheck);
				session->chatSent = true;
				InterlockedIncrement(&chatPlayerCnt);
			}
		}
	}
}

bool GetDummySetting(string& clientIP, int& clientPort, int& clientUsers, int& disconnectRandom, int& contentRandom)
{
	Parser parser;
	parser.LoadFile("DummySetting.txt");

	parser.GetValue("IP", clientIP);
	parser.GetValue("Port", clientPort);
	parser.GetValue("clientUsers", clientUsers);
	parser.GetValue("disconnectRandom", disconnectRandom);
	parser.GetValue("contentRandom", contentRandom);

	cout << "dummy setting" << '\n';
	cout << "IP : " << clientIP << '\n';
	cout << "Port : " << clientPort << '\n';
	cout << "clientUsers : " << clientUsers << '\n';
	cout << "disconnectRandom : " << disconnectRandom << '\n';
	cout << "contentRandom : " << contentRandom << '\n';

	return parser.CheckLeak();
}
