#pragma once


#include "pch.h"



class LogManager
{
	DWORD sec;

	DWORD errorCount;

	std::thread _logThread;
	std::list<std::string> _LogQ;
	CRITICAL_SECTION logQLock;

	HANDLE _LogThreadExitEvent;

public:
	
	LogManager();
	~LogManager();

	bool InitLogManager();

	//-------------------------------------------//
	//1초에 한 번 모니터링 화면 출력, LogQ에 로그가 들어있다면 로그로 저장
	// Frame이 떨어지면 모니터링 화면 로그로 저장
	//-------------------------------------------//
	unsigned int __stdcall LogThread();
	//-------------------------------------------//
	//모니터링 출력
	//-------------------------------------------//
	//void PrintLog();
	
	//-------------------------------------------//
	//프레임이 FrameError 값 이하로 떨어졌을 때 모니터링 되는 화면을 로그로 저장
	//-------------------------------------------//
	void WriteLog();
	//-------------------------------------------//
	//LogQ에서 뽑아서 로그로 저장
	//-------------------------------------------//
	void WriteLogQToFile();
	
	//-------------------------------------------//
	//파일 이름을 xxx.cpp로 남겨줌
	//-------------------------------------------//
	std::string getFileName(const std::string& path);
	
	
	//-------------------------------------------//
	//로그 형태 문자열로 로그 큐에 Enque
	//-------------------------------------------//
	void EnqueLog(const char* name,long long PlayID,  const char* FileName, const char* FuncName, int Line, int errorCode);
	void EnqueLog(const char* string);


	//------------------------------------------
	// EnterCriticalSection(&LogQLock)매핑 함수
	//------------------------------------------
	void EnterLogQ();
	//------------------------------------------
	// LeaveCriticalSection(&LogQLock)매핑 함수
	//------------------------------------------
	void LeaveLog();

	//------------------------------------------
	// 로그 쓰레드 종료 요청 함수
	//------------------------------------------
	void CloseLogManager();

	//------------------------------------------
	// Logthread 종료 절차 - LogQ를 비운 뒤에 종료
	//------------------------------------------

	void _ExitLogThread();
};





