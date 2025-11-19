#include "pch.h"
#include "PDHManager.h"



bool CPdhManager::CEthernetData::GetData(double* outRecvData, double* outSendData)
{
	PdhCollectQueryData(ethernetQuery);

	for (int i = 0; i < ethernetMaxCount; i++)
	{
		if (_EthernetStruct[i]._bUse)
		{
			Status = PdhGetFormattedCounterValue(_EthernetStruct[i]._pdh_Counter_Network_RecvBytes,
				PDH_FMT_DOUBLE, NULL, &CounterValue);
			if (Status == 0)
			{
				_pdh_value_Network_RecvBytes += CounterValue.doubleValue;
			}
			else {
			}

			Status = PdhGetFormattedCounterValue(_EthernetStruct[i]._pdh_Counter_Network_SendBytes,
				PDH_FMT_DOUBLE, NULL, &CounterValue);
			if (Status == 0)
			{
				_pdh_value_Network_SendBytes += CounterValue.doubleValue;
			}
			else
			{
			}
		}
	}

	*outRecvData = _pdh_value_Network_RecvBytes;
	*outSendData = _pdh_value_Network_SendBytes;
	_pdh_value_Network_RecvBytes = 0;
	_pdh_value_Network_SendBytes = 0;

	return true;
}

CPdhManager::CEthernetData::~CEthernetData()
{
	delete[] _EthernetStruct;
	delete[] szCounters;
	delete[] szInterfaces;
}
bool CPdhManager::CEthernetData::InitEthernetData()
{
	ethernetMaxCount = 8;
	_EthernetStruct = new st_ETHERNET[ethernetMaxCount];


	PdhEnumObjectItems(NULL, NULL, L"Network Interface", szCounters, &dwCounterSize, szInterfaces, &dwInterfaceSize,
		PERF_DETAIL_WIZARD, 0);

	szCounters = new WCHAR[dwCounterSize];
	szInterfaces = new WCHAR[dwInterfaceSize];

	//-----------------------------------------------------------------
	// 버퍼의 동적 할당 후 다시 호출
	// 이때 szCounters와 szInterfaces에 문자열이 들어옴
	// 2차원 배열이 아닌 1차원 배열에 null을 구분자로 쭉 들어옴
	//-----------------------------------------------------------------
	if (PdhEnumObjectItems(NULL, NULL, L"Network Interface", szCounters, &dwCounterSize, szInterfaces, &dwInterfaceSize,
		PERF_DETAIL_WIZARD, 0) != ERROR_SUCCESS)
	{
		__debugbreak();
		return false;
	}

	iCnt = 0;
	szCur = szInterfaces;

	//-----------------------------------------------------------------
	// szInterfaces에서 문자열 단위로 끊으면서 이름을 복사 받는다
	//-----------------------------------------------------------------

	ethernetQuery = { 0, };

	PdhOpenQuery(NULL, NULL, &ethernetQuery);


	for (; *szCur != L'\0' && iCnt < ethernetMaxCount; szCur += wcslen(szCur) + 1, iCnt++)
	{
		_EthernetStruct[iCnt]._bUse = true;
		_EthernetStruct[iCnt]._szName[0] = L'\0';

		wcscpy_s(_EthernetStruct[iCnt]._szName, szCur);

		szQuery[0] = L'\0';
		StringCbPrintf(szQuery, sizeof(WCHAR) * 1024, L"\\Network Interface(%s)\\Bytes Received/sec", szCur);
		if (PdhAddCounter(ethernetQuery, szQuery, NULL, &_EthernetStruct[iCnt]._pdh_Counter_Network_RecvBytes)
			!= ERROR_SUCCESS)
		{
			__debugbreak();
		}

		szQuery[0] = L'\0';
		StringCbPrintf(szQuery, sizeof(WCHAR) * 1024, L"\\Network Interface(%s)\\Bytes Sent/sec", szCur);
		if (PdhAddCounter(ethernetQuery, szQuery, NULL, &_EthernetStruct[iCnt]._pdh_Counter_Network_SendBytes)
			!= ERROR_SUCCESS)
		{
			__debugbreak();
		}
	}

	PdhCollectQueryData(ethernetQuery);

	return true;
}

CPdhManager::CEthernetData::CEthernetData()
{
	CounterValue = { 0 };
	Status = { 0 };
	iCnt = 0;
	bErr = false;
	szCur = NULL;
	szCounters = NULL;
	szInterfaces = NULL;
	dwCounterSize = 0;
	dwInterfaceSize = 0;
	ZeroMemory((char*)szQuery, sizeof(WCHAR) * 1024);
	_EthernetStruct = nullptr;
	_pdh_value_Network_RecvBytes = 0;
	_pdh_value_Network_SendBytes = 0;
	ethernetQuery = { 0 };
	ethernetMaxCount = df_PDH_ETHERNET_MAX;
}

bool CPdhManager::CCPUData::InitCpuData()
{
	PdhOpenQuery(NULL, NULL, &cpuQuery);

	PdhAddCounter(cpuQuery, L"\\Processor(_Total)\\% Processor Time", NULL, &cpuTotal);
	PdhAddCounter(cpuQuery, L"\\Processor(_Total)\\% User Time", NULL, &cpuUser);
	PdhAddCounter(cpuQuery, L"\\Processor(_Total)\\% Privileged Time", NULL, &cpuKernel);

	PdhCollectQueryData(cpuQuery);

	return true;
}

bool CPdhManager::CCPUData::GetData(double* pCpuTotal, double* pCpuUser, double* pCpuKernel)
{
	PdhCollectQueryData(cpuQuery);

	if (pCpuTotal != nullptr)
	{
		PdhGetFormattedCounterValue(cpuTotal, PDH_FMT_DOUBLE, NULL, &counterVal);
		*pCpuTotal = counterVal.doubleValue;
	}

	if (pCpuUser != nullptr)
	{
		PdhGetFormattedCounterValue(cpuUser, PDH_FMT_DOUBLE, NULL, &counterVal);
		*pCpuUser = counterVal.doubleValue;
	}

	if (pCpuKernel != nullptr)
	{
		PdhGetFormattedCounterValue(cpuKernel, PDH_FMT_DOUBLE, NULL, &counterVal);
		*pCpuKernel = counterVal.doubleValue;
	}

	return true;
}

CPdhManager::CCPUData::CCPUData()
{
	cpuQuery = 0;
	cpuTotal = 0;
	cpuUser = 0;
	cpuKernel = 0;

	counterVal = { 0 };
}

CPdhManager::CMemoryData::CMemoryData()
{
	memoryQuery = 0;
	PrivateMemory = 0;
	ProcessNonPagedMemory = 0;
	TotalNonPagedMemory = 0;
	AvailableMemory = 0;

	counterVal = { 0, };
}

bool CPdhManager::CMemoryData::InitMemoryData(WCHAR* processName)
{
	szProcessName = processName;

	for (int i = 0; i < 1024; i++)
	{
		if (szProcessName[i] == '.')
		{
			szProcessName[i] = NULL;
			break;
		}
	}

	WCHAR PrivateMemoryCounterPath[1024];
	WCHAR processNonPagedMemoryCounterPath[1024];

	StringCchPrintf(PrivateMemoryCounterPath, 1024, L"\\Process(%s)\\Private Bytes", szProcessName);
	StringCchPrintf(processNonPagedMemoryCounterPath, 1024, L"\\Process(%s)\\Pool Nonpaged Bytes", szProcessName);


	PdhOpenQuery(NULL, NULL, &memoryQuery);

	if (PdhAddCounter(memoryQuery, PrivateMemoryCounterPath, NULL, &PrivateMemory) != ERROR_SUCCESS)
	{
		__debugbreak();
	}
	if (PdhAddCounter(memoryQuery, processNonPagedMemoryCounterPath, NULL, &ProcessNonPagedMemory) != ERROR_SUCCESS)
	{
		__debugbreak();
	}
	PdhAddCounter(memoryQuery, L"\\Memory\\Available MBytes", NULL, &AvailableMemory);
	PdhAddCounter(memoryQuery, L"\\Memory\\Pool Nonpaged Bytes", NULL, &TotalNonPagedMemory);

	PdhCollectQueryData(memoryQuery);

	return true;
}


bool CPdhManager::CMemoryData::GetData(double* Private, double* ProcessNonPaged, double* TotalNonPaged, double* Available)
{
	PdhCollectQueryData(memoryQuery);

	long retval;

	if (Private != nullptr)
	{
		retval = PdhGetFormattedCounterValue(PrivateMemory, PDH_FMT_LARGE, NULL, &counterVal);
		if (retval != ERROR_SUCCESS)
		{
			__debugbreak();
		}
		*Private = (double)counterVal.largeValue;
	}

	if (ProcessNonPaged != nullptr)
	{
		if (PdhGetFormattedCounterValue(ProcessNonPagedMemory, PDH_FMT_LARGE, NULL, &counterVal) != ERROR_SUCCESS)
		{
			__debugbreak();
		}
		*ProcessNonPaged = (double)counterVal.largeValue;
	}

	if (TotalNonPaged != nullptr)
	{
		if (PdhGetFormattedCounterValue(TotalNonPagedMemory, PDH_FMT_LARGE, NULL, &counterVal) != ERROR_SUCCESS)
		{
			__debugbreak();
		}
		*TotalNonPaged = (double)counterVal.largeValue;
	}

	if (Available != nullptr)
	{
		if (PdhGetFormattedCounterValue(AvailableMemory, PDH_FMT_LARGE, NULL, &counterVal) != ERROR_SUCCESS)
		{
			__debugbreak();
		}
		*Available = (double)counterVal.largeValue;
	}
	return true;
}


void CPdhManager::Start()
{
	GetModuleBaseNameW(GetCurrentProcess(), NULL, szProcessName, MAX_PATH);
	EthernetData.InitEthernetData();
	CpuData.InitCpuData();
	MemoryData.InitMemoryData(szProcessName);
}

bool CPdhManager::GetCpuData(double* pCpuTotal, double* pCpuUser, double* pCpuKernel)
{
	return CpuData.GetData(pCpuTotal, pCpuUser, pCpuKernel);
}
bool CPdhManager::GetEthernetData(double* pRecvData, double* pSendData)
{
	return EthernetData.GetData(pRecvData, pSendData);
}
bool CPdhManager::GetMemoryData(double* pPrivate, double* pProcessNonpaged, double* pTotalNonpaged, double* pAvailable)
{
	return MemoryData.GetData(pPrivate, pProcessNonpaged, pTotalNonpaged, pAvailable);
}

void CPdhManager::CEthernetData::RegistEthernetMax(unsigned char count)
{
	ethernetMaxCount = count;
}

void CPdhManager::RegistEthernetMax(unsigned char count)
{
	EthernetData.RegistEthernetMax(count);
}