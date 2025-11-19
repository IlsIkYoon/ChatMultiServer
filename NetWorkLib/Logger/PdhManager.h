#pragma once
#include "pch.h"
#include <pdh.h>
#include <pdhmsg.h> 
#include <strsafe.h>
#include <psapi.h>

#pragma comment(lib, "pdh.lib")
#pragma comment(lib, "psapi.lib")

#define df_PDH_ETHERNET_MAX 3

class CPdhManager
{
	struct st_ETHERNET
	{
		bool _bUse;
		WCHAR _szName[128];

		PDH_HCOUNTER _pdh_Counter_Network_RecvBytes;
		PDH_HCOUNTER _pdh_Counter_Network_SendBytes;
	};

	class CEthernetData
	{
		st_ETHERNET* _EthernetStruct; //랜카드 별 pdh정보
		double _pdh_value_Network_RecvBytes; //총 RecvBytes 모든 랜카드 합산
		double _pdh_value_Network_SendBytes; //총 SendBytes 모든 랜카드 합산

		unsigned char ethernetMaxCount;

		int iCnt;
		bool bErr;
		WCHAR* szCur;
		WCHAR* szCounters;
		WCHAR* szInterfaces;
		DWORD dwCounterSize;
		DWORD dwInterfaceSize;
		WCHAR szQuery[1024];
		PDH_FMT_COUNTERVALUE CounterValue;
		PDH_STATUS Status;
		PDH_HQUERY ethernetQuery;

	public:
		CEthernetData();
		~CEthernetData();
		void RegistEthernetMax(unsigned char count);
		bool InitEthernetData();
		bool GetData(double* outRecvData, double* outSendData);
	};

	class CCPUData
	{
		PDH_HQUERY cpuQuery;
		PDH_HCOUNTER cpuTotal;
		PDH_HCOUNTER cpuUser;
		PDH_HCOUNTER cpuKernel;

		PDH_FMT_COUNTERVALUE counterVal;
	public:
		CCPUData();
		bool InitCpuData();
		bool GetData(double* cpuTotal, double* cpuUser, double* cpuKernel);

	};

	class CMemoryData
	{
		PDH_HQUERY memoryQuery;
		PDH_HCOUNTER PrivateMemory;
		PDH_HCOUNTER ProcessNonPagedMemory;
		PDH_HCOUNTER TotalNonPagedMemory;
		PDH_HCOUNTER AvailableMemory;

		PDH_FMT_COUNTERVALUE counterVal;

		WCHAR* szProcessName;
	public:

		CMemoryData();
		bool InitMemoryData(WCHAR* processName);
		bool GetData(double* Private, double* ProcessNonPaged, double* TotalNonPaged, double* Available);

	};

	CEthernetData EthernetData;
	CCPUData CpuData;
	CMemoryData MemoryData;

	WCHAR szProcessName[1024];


public:
	void Start();
	void RegistEthernetMax(unsigned char count);
	bool GetCpuData(double* pCpuTotal, double* pCpuUser, double* pCpuKernel);
	bool GetEthernetData(double* pRecvData, double* pSendData);
	bool GetMemoryData(double* pPrivate, double* pProcessNonpaged, double* pTotalNonpaged, double* pAvailable);
};
