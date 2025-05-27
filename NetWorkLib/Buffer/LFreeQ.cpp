#include "pch.h"
#include "LFreeQ.h"

#ifdef __LFDEBUG__
LogData* logArr;
ULONG64* CASERROR_logArr;
LogData* TailDefer_logArr;
unsigned long long g_SeqNum = 0;
unsigned long long g_CASERROR_SeqNum = 0;
unsigned long long g_TailDefer_SeqNum = 0;
unsigned long long logMaxNum = LOG_MAXNUM;


/*
struct LogData
{
	char funcType;
	DWORD TID;

	unsigned long long seqNum;

	ULONG_PTR firstCasParam2;
	ULONG_PTR firstCasParam3;
	ULONG_PTR SecondCasParam2;
	ULONG_PTR SecondCasParam3;

	ULONG_PTR NewNodeAddress;
	ULONG_PTR DeleteNodeAddress;

	bool CAS1Success;
	bool CAS2Success;
};

*/


void WriteAllLogData()
{

	printf("Memory Log Save Start!!!\n");
	std::string fileName;
	fileName = __DATE__;
	fileName += "Memory_Log.csv";

	FILE* fpWrite;
	fopen_s(&fpWrite, fileName.c_str(), "a");
	if (fpWrite == 0) __debugbreak();
	fprintf(fpWrite, "SeqNum,FuncType,TID,NewNodeAddress,DeleteNodeAddress,firstCasParam2,firstCasParam3,SecondCasParam2,SecondCasParam3,CAS1,CAS2\n");


	for (int i = 0; i < g_SeqNum; i++)
	{

		if ((logArr[i].funcType & DEQUEUE) == DEQUEUE)
		{
			fprintf(fpWrite, "%llu,DEQUEUE, %lu, %16llx,%16llx,%16llx,%16llx,%16llx,%16llx,%d,%d\n", logArr[i].seqNum, logArr[i].TID, logArr[i].NewNodeAddress,
				logArr[i].DeleteNodeAddress, logArr[i].firstCasParam2, logArr[i].firstCasParam3, logArr[i].SecondCasParam2,
				logArr[i].SecondCasParam3, logArr[i].CAS1Success, logArr[i].CAS2Success);
		}
		else
		{
			fprintf(fpWrite, "%llu,ENQUEUE, %lu, %16llx,%16llx,%16llx,%16llx,%16llx,%16llx,%d,%d\n", logArr[i].seqNum, logArr[i].TID, logArr[i].NewNodeAddress,
				logArr[i].DeleteNodeAddress, logArr[i].firstCasParam2, logArr[i].firstCasParam3, logArr[i].SecondCasParam2,
				logArr[i].SecondCasParam3, logArr[i].CAS1Success, logArr[i].CAS2Success);
		}

	}

	fclose(fpWrite);
	printf("Memory Log Save Fin!!!\n");


	return;


}
#endif