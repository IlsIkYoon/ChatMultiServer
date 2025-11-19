#pragma once

#include "pch.h"

#pragma comment(lib, "DbgHelp.lib")
#pragma comment(lib, "Psapi.lib")


#include <psapi.h>
#include <DbgHelp.h>
#include <crtdbg.h>


namespace procademy
{
	class CCrashDump
	{
	public:

		static long _DumpCount;

		CCrashDump()
		{
			_DumpCount = 0;
			_invalid_parameter_handler oldHandler, newHandler;
			newHandler = myInvalidParameterHandler;

			oldHandler = _set_invalid_parameter_handler(newHandler);
			_CrtSetReportMode(_CRT_WARN, 0);
			_CrtSetReportMode(_CRT_ASSERT, 0);
			_CrtSetReportMode(_CRT_ERROR, 0);

			//----------------------------------------------------
			// pure virtual function called 에러 핸들러를 사용자 정의 함수로 우회시킨다.
			//----------------------------------------------------
			_set_purecall_handler(myPurecallHandler);

			SetHandlerDump();
		}

		static void Crash(void)
		{
			int* p = nullptr;
			*p = 0;
		}

		static LONG WINAPI MyExceptionFilter(__in PEXCEPTION_POINTERS pExceptionPointer)
		{
			wprintf(L"핸들러 호출 \n");

			int iWorkingMemory = 0;
			SYSTEMTIME stNowTime;

			long DumpCount = InterlockedIncrement(&_DumpCount);

			//----------------------------------------------------
			// 현재 프로세스의 메모리를 얻어온다.
			//----------------------------------------------------
			HANDLE hProcess = 0;
			PROCESS_MEMORY_COUNTERS pmc;

			hProcess = GetCurrentProcess();

			if (hProcess == NULL)
				return 0;

			if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc)))
			{
				iWorkingMemory = (int)(pmc.WorkingSetSize / 1024 / 1024);
			}
			CloseHandle(hProcess);

			//----------------------------------------------------
			// 현재 날짜와 시간을 알아온다.
			//----------------------------------------------------
			WCHAR filename[MAX_PATH];

			GetLocalTime(&stNowTime);
			wsprintf(filename, L"Dump_%d%02d%02d_%02d.%02d.%02d_%d%dMB.dmp",
				stNowTime.wYear, stNowTime.wMonth, stNowTime.wDay, stNowTime.wHour, stNowTime.wMinute, stNowTime.wSecond, DumpCount, iWorkingMemory);

			wprintf(L"\n\n\n!!!Crash Error!!! %d.%d.%d/%d:%d:%d\n",
				stNowTime.wYear, stNowTime.wMonth, stNowTime.wDay, stNowTime.wHour, stNowTime.wMinute, stNowTime.wSecond);
			wprintf(L"Now Save dump File...\n");

			HANDLE hDumpFile = ::CreateFile(filename,
				GENERIC_WRITE,
				FILE_SHARE_WRITE,
				NULL,
				CREATE_ALWAYS,
				FILE_ATTRIBUTE_NORMAL, NULL);

			if (hDumpFile != INVALID_HANDLE_VALUE)
			{
				MINIDUMP_EXCEPTION_INFORMATION MinidumpExceptionInformantion;

				MinidumpExceptionInformantion.ThreadId = ::GetCurrentThreadId();
				MinidumpExceptionInformantion.ExceptionPointers = pExceptionPointer;
				MinidumpExceptionInformantion.ClientPointers = TRUE;

				MiniDumpWriteDump(GetCurrentProcess(),
					GetCurrentProcessId(),
					hDumpFile,
					MiniDumpWithFullMemory,
					&MinidumpExceptionInformantion,
					NULL,
					NULL);

				CloseHandle(hDumpFile);
				wprintf(L"CrashDump Save Finish!");
			}


			return EXCEPTION_EXECUTE_HANDLER;
		}

		static void SetHandlerDump()
		{
			SetUnhandledExceptionFilter(MyExceptionFilter);
		}

		static void myInvalidParameterHandler(const wchar_t* expression, const wchar_t* function, const wchar_t* file, unsigned int line, uintptr_t pReserved)
		{
			Crash();
		}

		static int _custom_Report_hook(int ireposttype, char* message, int* returnvalue)
		{
			Crash();
			return true;
		}

		static void myPurecallHandler(void)
		{
			Crash();

		}


	};

}