#pragma once


//#ifdef __PROFILE__


#include <windows.h>
#include <iostream>
#include <map>
/*
- 마이크로세컨드 단위의 시간 체크 필요.


# queryperformancecounter 사용
---------------------------------------------------------------- -
large_integer start;
large_integer end;
large_integer freq;
queryperformancefrequency(&freq);	// 1초의 진동주기

queryperformancecounter(&start);
sleep(1000);
queryperformancecounter(&end);

초단위의 시간 = (end.quadpart - start.quadpart) / freq.quadpart;



우리는 이를 사용하여 마이크로세컨드(100만분의 1초 단위로 변경 필요) 또는 100나노 기준
*/


enum class LockStatus
{
	LOCK_UNINITIALIZED = 0, LOCK_INITIALIZING, LOCK_INITIALIZED

};

extern CRITICAL_SECTION g_ProfilerWriteLock;
extern unsigned long g_lockInit;


class ProfilerMap
{

	struct PStruct
	{
		long long lessValue;
		long long maxValue;
		long long count;
		long long total;
		long long average;
	};

	LARGE_INTEGER _freq;
	DWORD _TID;
	std::map<const char*, PStruct> pMap;

public:

	double GetMicroSecond(unsigned long long quadPart)
	{
		double ret;

		ret = (double)quadPart * 1000'000 / _freq.QuadPart;
		return ret;
	}


	double GetMiliSecond(unsigned long long quadPart)
	{
		double ret;

		ret = (double)quadPart * 1000 / _freq.QuadPart;
		return ret;
	}


	//------------------------------------------------------------
	// 종료 시 파일 출력을 위한 락의 초기화 작업. 스핀락 방식으로 구현
	//------------------------------------------------------------
	ProfilerMap()
	{
		unsigned long result;

		while (1)
		{
			result = InterlockedExchange(&g_lockInit, (unsigned long)LockStatus::LOCK_INITIALIZING);

			if (result == (unsigned long)LockStatus::LOCK_UNINITIALIZED)
			{
				InitializeCriticalSection(&g_ProfilerWriteLock);
				InterlockedExchange(&g_lockInit, (unsigned long)LockStatus::LOCK_INITIALIZED);
				break;
			}
			else if (result == (unsigned long)LockStatus::LOCK_INITIALIZING)
			{
				YieldProcessor();
				continue;
			}
			else if (result == (unsigned long)LockStatus::LOCK_INITIALIZED)
			{
				InterlockedExchange(&g_lockInit, (unsigned long)LockStatus::LOCK_INITIALIZED);
				break;
			}
			else
			{
				__debugbreak();
				//enumclass외의 값이 나오는 비정상적인 상황
			}
		}

		QueryPerformanceFrequency(&_freq);
		_TID = GetCurrentThreadId();
	}
	~ProfilerMap()
	{



		FILE* fpWrite;
		std::string fileName;
		fileName = "Profiler_";
		fileName += __DATE__;
		fileName += ".txt";

		//Lock//
		EnterCriticalSection(&g_ProfilerWriteLock);

		fopen_s(&fpWrite, fileName.c_str(), "a");
		if (fpWrite == nullptr)
			__debugbreak();

		fprintf(fpWrite, "---------------------------------\n");
		fprintf(fpWrite, "THREADID,%d\n", _TID);
		fprintf(fpWrite, "FileName||lessValue||maxValue||count||total||average\n");
		for (const auto& it : pMap)
		{
			fprintf(fpWrite, "%s||%fμs||%fμs||%lldμs||%fμs||%fμs\n", it.first, GetMicroSecond(it.second.lessValue),
				GetMicroSecond(it.second.maxValue), it.second.count, GetMicroSecond(it.second.total), GetMicroSecond(it.second.average));
		}
		fprintf(fpWrite, "---------------------------------\n");
		fclose(fpWrite);

		LeaveCriticalSection(&g_ProfilerWriteLock);
		//UnLock//


		printf("Save Complete\n");
	}


	void Insert(const char* TagName, long long Data)
	{
		pMap[TagName].total += Data;

		if (pMap[TagName].lessValue > Data || pMap[TagName].lessValue == 0)
			pMap[TagName].lessValue = Data;
		if (pMap[TagName].maxValue < Data)
			pMap[TagName].maxValue = Data;
		pMap[TagName].count++;
		pMap[TagName].average = pMap[TagName].total / pMap[TagName].count;
	}

	void ResetData()
	{
		//맵 안에 있는 데이터를 리셋하는 함수
		pMap.clear();
	}
};




extern thread_local ProfilerMap g_ProfileMap;


class Profiler
{

	LARGE_INTEGER _start;
	LARGE_INTEGER _end;
	const char* _tagName;
	long long _result;

public:
	Profiler(const char* TagName)
	{
		_result = 0;
		_end.QuadPart = 0;
		_tagName = TagName;
		QueryPerformanceCounter(&_start);
	}
	~Profiler()
	{
		QueryPerformanceCounter(&_end);
		SaveProfileData();
	}


	void SaveProfileData()
	{
		_result = _end.QuadPart - _start.QuadPart;
		if (_result < 0) __debugbreak();
		g_ProfileMap.Insert(_tagName, _result);
	}

};

//#endif