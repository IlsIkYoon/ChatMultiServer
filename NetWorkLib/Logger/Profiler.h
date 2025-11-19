#pragma once

#include "pch.h"

class CProfilerMap;

#define T_PROFILER_MAP GetThreadProfilerMap()


constexpr int COL_TAG = 24;   // FileName 열 폭
constexpr int COL_NUM = 12;   // 숫자 열 폭

//------------------------------------------
// 모든 쓰레드의 프로파일 맵을 저장할 전역 리스트 (중간 출력을 위해 존재)
//------------------------------------------
extern std::list<CProfilerMap*> g_ProfileMapList;
extern std::mutex g_ProfileMapListLock;


//------------------------------------------
// 모든 쓰레드의 프로파일 데이터 파일 출력
//------------------------------------------
bool WriteAllProfileData();

bool ResetAllProfileDate();

class CProfilerMap
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
	std::mutex profilerMapLock;

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

	~CProfilerMap()
	{
		FILE* fpWrite;
		std::string fileName;

		fileName = "Profiler_";
		fileName += __DATE__;
		fileName += ".txt";

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


		printf("Save Complete\n");
	}


	CProfilerMap()
	{
		QueryPerformanceFrequency(&_freq);
		_TID = GetCurrentThreadId();
		Regist();
	}
	void Regist()
	{
		std::lock_guard guard(g_ProfileMapListLock);
		g_ProfileMapList.push_back(this);
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
		std::lock_guard guard(profilerMapLock);
		pMap.clear();
	}
	bool WriteProfile()
	{
		FILE* fpWrite;
		std::string fileName;

		fileName = "Profiler_";
		fileName += __DATE__;
		fileName += ".txt";

		fopen_s(&fpWrite, fileName.c_str(), "a");
		if (fpWrite == nullptr)
			__debugbreak();
		fprintf(fpWrite, "-----------------------------------------------------------------------\n");
		fprintf(fpWrite, "Exit-WriteProfle || THREADID : %d\n", _TID);
		auto titleStr = std::format("{:<{}}||{:>{}}||{:>{}}||{:>{}}||{:>{}}\n",
			"FileName", COL_TAG,
			"lessValue", COL_NUM,
			"maxValue", COL_NUM,
			"count", COL_NUM,
			"average", COL_NUM);
		fprintf(fpWrite, titleStr.c_str());

		for (const auto& [tag, st] : pMap)
		{
			auto row = std::format("{:<{}}||{:>{}.2f}||{:>{}.2f}||{:>{}}||{:>{}.2f}\n",
				tag, COL_TAG,                        // ← 왼쪽 정렬
				GetMicroSecond(st.lessValue), COL_NUM,
				GetMicroSecond(st.maxValue), COL_NUM,
				st.count, COL_NUM,
				GetMicroSecond(st.average), COL_NUM);
			fprintf(fpWrite, row.c_str());
		}
		fprintf(fpWrite, "-----------------------------------------------------------------------\n");
		fclose(fpWrite);


		printf("Save Complete\n");
		return true;
	}
	bool SaveProfile()
	{
		if (pMap.size() == 0)
		{
			return false;
		}

		FILE* fpWrite;
		std::string fileName;
		fileName = "Profiler_";
		fileName += __DATE__;
		fileName += ".txt";

		fopen_s(&fpWrite, fileName.c_str(), "a");
		if (fpWrite == nullptr)
			__debugbreak();
		fprintf(fpWrite, "-----------------------------------------------------------------------\n");
		fprintf(fpWrite, "Save-WriteProfle || THREADID : %d || Time : %s\n", _TID, __TIME__);
		auto titleStr = std::format("{:<{}}||{:>{}}||{:>{}}||{:>{}}||{:>{}}\n",
			"FileName", COL_TAG,
			"lessValue", COL_NUM,
			"maxValue", COL_NUM,
			"count", COL_NUM,
			"average", COL_NUM);
		fprintf(fpWrite, titleStr.c_str());

		for (const auto& [tag, st] : pMap)
		{
			auto row = std::format("{:<{}}||{:>{}.2f}||{:>{}.2f}||{:>{}}||{:>{}.2f}\n",
				tag, COL_TAG,                        // ← 왼쪽 정렬
				GetMicroSecond(st.lessValue), COL_NUM,
				GetMicroSecond(st.maxValue), COL_NUM,
				st.count, COL_NUM,
				GetMicroSecond(st.average), COL_NUM);
			fprintf(fpWrite, row.c_str());
		}
		fprintf(fpWrite, "-----------------------------------------------------------------------\n");
		fclose(fpWrite);


		printf("Save Complete\n");

		return true;
	}
};

inline CProfilerMap& GetThreadProfilerMap()
{
	static thread_local CProfilerMap instance;
	return instance;
}
class CProfiler
{
	LARGE_INTEGER _start;
	LARGE_INTEGER _end;
	const char* _tagName;
	long long _result;

public:
	CProfiler(const char* TagName)
	{
		_result = 0;
		_end.QuadPart = 0;
		_tagName = TagName;
		QueryPerformanceCounter(&_start);
	}
	~CProfiler()
	{
		QueryPerformanceCounter(&_end);
		SaveProfileData();
	}

	void SaveProfileData()
	{
		_result = _end.QuadPart - _start.QuadPart;	
		{
			std::lock_guard guard(T_PROFILER_MAP.profilerMapLock);
			//이 락에 대한 경합은 프로파일러 출력이 일어날 때만 발생
			T_PROFILER_MAP.Insert(_tagName, _result);
		}
	}
};
