
#include "pch.h"
#include "Logger.h"

DWORD sec;

DWORD errorCount = 0;

enum class LogThreadEvent
{
	Exit = 0,
	DataInsert
};

unsigned int __stdcall CLogManager::LogThread()
{
	HANDLE hArr[2];
	DWORD waitRetval;
	DWORD EventIndex;

	hArr[(int)LogThreadEvent::Exit] = _LogThreadExitEvent;
	hArr[(int)LogThreadEvent::DataInsert] = _LogThreadDataEvent;

	while (1)
	{
		waitRetval = WaitForMultipleObjects(2, hArr, false, INFINITE);

		if (waitRetval == WAIT_FAILED)
		{
			__debugbreak();
			break;
		}

		EventIndex = waitRetval - WAIT_OBJECT_0;

		switch (EventIndex)
		{
		case (int)LogThreadEvent::Exit:
			_ExitLogThread();
			return 0;

		case (int)LogThreadEvent::DataInsert:
		{
			while (_LogQ.empty() == false)
			{
				WriteLogQToFile();
			}
		}
		break;

		default:
			__debugbreak();

		}

	}

	return 0;
}

void CLogManager::WriteLogQToFile()
{
	FILE* fpWrite;
	char fileName[40] = { 0, };
	strcpy_s(fileName, __DATE__);
	strcat_s(fileName, myFileName.c_str());
	strcat_s(fileName, "_Log.txt");

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



std::string CLogManager::getFileName(const std::string& path) {
	return path.substr(path.find_last_of("/\\") + 1);
}

void CLogManager::EnqueLog(const char* name, long long PlayerID, const char* FileName, const char* FuncName, int Line, int errorCode)
{
	//errorCount++;
	
	std::string logEntry = std::format("ID: {} || {} || FILE : {}, Func : {} , Line : {} error : {}\n",
		PlayerID, name, getFileName(__FILE__), __func__, __LINE__, GetLastError());
	EnterCriticalSection(&logQLock);
	_LogQ.push_back(logEntry);
	LeaveCriticalSection(&logQLock);

	SetEvent(_LogThreadDataEvent);
}
void CLogManager::EnqueLog(const char* string)
{
	std::string logEntry = std::format("{}\n", string);
	EnterCriticalSection(&logQLock);
	_LogQ.push_back(logEntry);
	LeaveCriticalSection(&logQLock);

	SetEvent(_LogThreadDataEvent);
}
void CLogManager::EnqueLog(std::string& logEntry)
{
	logEntry += "\n";
	EnterCriticalSection(&logQLock);
	_LogQ.push_back(logEntry);
	LeaveCriticalSection(&logQLock);

	SetEvent(_LogThreadDataEvent);
}



bool CLogManager::InitLogManager()
{
	//logThread생성
	_logThread = std::thread([this]() {this-> LogThread(); });

	return true;
}


CLogManager::CLogManager()
{
	sec = 0;
	errorCount = 0;
	InitializeCriticalSection(&logQLock);
	_LogThreadExitEvent = CreateEvent(NULL, true, false, NULL);
	_LogThreadDataEvent = CreateEvent(NULL, false, false, NULL);
}


void CLogManager::EnterLogQ()
{
	EnterCriticalSection(&logQLock);
}
void CLogManager::LeaveLog()
{
	LeaveCriticalSection(&logQLock);
}

CLogManager::~CLogManager()
{
	//쓰레드 종료 등의 로직

}

void CLogManager::CloseLogManager()
{
	SetEvent(_LogThreadExitEvent);
	WaitForSingleObject(&_logThread, INFINITE);
	
	return;
}

void CLogManager::_ExitLogThread()
{
	//로그 리소스 정리
	while (_LogQ.empty() == false)
	{
		WriteLogQToFile();
	}
	

}

void CLogManager::RegistMyFileName(std::string name)
{
	myFileName = name;
}