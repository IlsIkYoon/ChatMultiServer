#include "Log.h"
#include <format>

DWORD sec;

DWORD errorCount = 0;

unsigned int __stdcall LogManager::LogThread()
{
	sec = timeGetTime() / 1000;

	//DWORD eventRet;
	while (1)
	{
		if (sec != timeGetTime() / 1000)
		{

			sec = timeGetTime() / 1000;

			if (_LogQ.size() > 0)
			{
				WriteLogQToFile();
			}

		}

		if (WaitForSingleObject(_LogThreadExitEvent, 0) == WAIT_OBJECT_0)
		{
			_ExitLogThread();
			break;
		}


		Sleep(1000 - (timeGetTime() % 1000));


	}

	return 0;
}

void LogManager::WriteLog()
{
	FILE* fpWrite;
	char fileName[40] = { 0, };
	strcpy_s(fileName, __DATE__);
	strcat_s(fileName, "Log.txt");

	fopen_s(&fpWrite, fileName, "a");

	if (fpWrite == 0)
	{
		EnqueLog("Log File Open",0,  __FILE__, __func__, __LINE__, GetLastError());

		return;
	}

}

void LogManager::WriteLogQToFile()
{
	FILE* fpWrite;
	char fileName[40] = { 0, };
	strcpy_s(fileName, __DATE__);
	strcat_s(fileName, "Log.txt");

	fopen_s(&fpWrite, fileName, "a");
	if (fpWrite == 0)
	{
		EnqueLog("Log File Open" ,0,  __FILE__, __func__, __LINE__, GetLastError());

		return;
	}

	while (1)
	{
		if (_LogQ.size() == 0) break;
		EnterCriticalSection(&logQLock);
		std::string logEntry = _LogQ.front();
		_LogQ.pop_front();
		LeaveCriticalSection(&logQLock);
		fwrite(logEntry.c_str(), 1, logEntry.size(), fpWrite);
	}

	fclose(fpWrite);
}



std::string LogManager::getFileName(const std::string& path) {
	return path.substr(path.find_last_of("/\\") + 1);
}

void LogManager::EnqueLog(const char* name, long long PlayerID, const char* FileName, const char* FuncName, int Line, int errorCode)
{
	//errorCount++;
	
	std::string logEntry = std::format("ID: {} || {} || FILE : {}, Func : {} , Line : {} error : {}\n",
		PlayerID, name, getFileName(__FILE__), __func__, __LINE__, GetLastError());
	EnterCriticalSection(&logQLock);
	_LogQ.push_back(logEntry);
	LeaveCriticalSection(&logQLock);


}
void LogManager::EnqueLog(const char* string)
{
	std::string logEntry = std::format("{}\n", string);
	EnterCriticalSection(&logQLock);
	_LogQ.push_back(logEntry);
	LeaveCriticalSection(&logQLock);


}


bool LogManager::InitLogManager()
{
	//logThread생성
	_logThread = std::thread([this]() {this-> LogThread(); });

	return true;
}


LogManager::LogManager()
{
	sec = 0;
	errorCount = 0;
	InitializeCriticalSection(&logQLock);
	_LogThreadExitEvent = CreateEvent(NULL, true, false, NULL);
}


void LogManager::EnterLogQ()
{
	EnterCriticalSection(&logQLock);
}
void LogManager::LeaveLog()
{
	LeaveCriticalSection(&logQLock);
}

LogManager::~LogManager()
{
	//쓰레드 종료 등의 로직

}

void LogManager::CloseLogManager()
{
	SetEvent(_LogThreadExitEvent);
	WaitForSingleObject(&_logThread, INFINITE);
	
	return;
}

void LogManager::_ExitLogThread()
{
	//로그 리소스 정리
	while (_LogQ.size() > 0)
	{
		WriteLogQToFile();
	}
	

}