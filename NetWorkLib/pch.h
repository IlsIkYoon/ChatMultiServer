// pch.h: 미리 컴파일된 헤더 파일입니다.
// 아래 나열된 파일은 한 번만 컴파일되었으며, 향후 빌드에 대한 빌드 성능을 향상합니다.
// 코드 컴파일 및 여러 코드 검색 기능을 포함하여 IntelliSense 성능에도 영향을 미칩니다.
// 그러나 여기에 나열된 파일은 빌드 간 업데이트되는 경우 모두 다시 컴파일됩니다.
// 여기에 자주 업데이트할 파일을 추가하지 마세요. 그러면 성능이 저하됩니다.

#ifndef PCH_H
#define PCH_H

#define WIN32_LEAN_AND_MEAN
#define _WINSOCKAPI_   
// 여기에 미리 컴파일하려는 헤더 추가



#include <ws2tcpip.h>
#include <WinSock2.h>
#include <windows.h>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "winmm.lib")

#include <mmsystem.h>
#include <iostream>
#include <process.h>
#include <string>
#include <stdlib.h>
#include <conio.h>
#include "framework.h"


#include <list>
#include <stack>
#include <map>
#include <queue>
#include <thread>
#include <format>
#include <mutex>

//---------------------------------
//빌드 전처리 옵션
//---------------------------------
// --디버그 옵션--
//#define __LOGDEBUG__ // 
#define __DEBUG__ //
//#define __LFDEBUG__ //락프리 메모리 디버그 옵션

//#define __JOBQLOCKFREEQ_

//-----------------------------------------------
// Contents에서 따로 Regist 안했을 시에 동작할 기본 값
//-----------------------------------------------
#define DEFAULT_PORTNUM 12203 
#define DEFAULT_CONCURRENT_COUNT 3
#define DEFAULT_WORKER_THREAD_COUNT 3
#define DEFAULT_SESSION_MAX_COUNT 30000

#define MAX_SESSION 300


#define HANDLER_EVENT_ONACCEPT 0
#define HANDLER_EVENT_ONMESSAGE 1
#define HANDLER_EVENT_ONSEND 2
#define HANDLER_EVENT_ONDISCONNECT 3

#define LOG_MAXNUM 50000

#define SESSION_CLOSABLE 0x00
#define SESSION_DISCONNECTING 0x01

//------------------------------------
// PQCS define값
//------------------------------------
#define SENDREQUEST (LPOVERLAPPED)0x01
#define SENDREQUEST_BYTE 0x05
#define SENDREQUEST_KEY 0x01

#define THREAD_EXIT 0xffffffff


#define CONTINUE 0x01


#define KEY_ESC 27

/*
#ifdef __PROFILE__
#include "Profiler.h"
#endif

*/

#endif //PCH_H
